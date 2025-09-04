#include "video_decoder.h"
#include <fstream>
#include <stdexcept>
#include <chrono>

#define CHECK_CUDA(call)                                                             \
    do {                                                                             \
        CUresult err = call;                                                         \
        if (err != CUDA_SUCCESS) {                                                   \
            const char* error_str = nullptr;                                         \
            const char* error_name = nullptr;                                        \
            cuGetErrorString(err, &error_str);                                       \
            cuGetErrorName(err, &error_name);                                        \
            std::cerr << "CUDA Error: " << (error_name ? error_name : "Unknown error") \
                      << " - " << (error_str ? error_str : "No description available") \
                      << " (Error Code: " << err << ")"                              \
                      << " at " << __FILE__ << ":" << __LINE__                       \
                      << " in function " << __func__ << std::endl;                   \
            throw std::runtime_error("CUDA Error occurred.");                        \
        }                                                                            \
    } while (0)

// 获取色度平面高度系数
static float GetChromaHeightFactor(cudaVideoChromaFormat chroma_format) {
    switch (chroma_format) {
        case cudaVideoChromaFormat_420: return 0.5f;
        case cudaVideoChromaFormat_422: return 1.0f;
        case cudaVideoChromaFormat_444: return 1.0f;
        case cudaVideoChromaFormat_Monochrome: return 0.0f;
        default: return 0.5f;
    }
}

// 获取色度平面数量
static int GetChromaPlaneCount(cudaVideoChromaFormat chroma_format) {
    switch (chroma_format) {
        case cudaVideoChromaFormat_420: return 1;
        case cudaVideoChromaFormat_444: return 2;
        default: return 0;
    }
}

// 获取解码表面数量
static unsigned long GetNumDecodeSurfaces(cudaVideoCodec codec, unsigned int width, unsigned int height) {
    if (codec == cudaVideoCodec_VP9) {
        return 12;
    } else if (codec == cudaVideoCodec_H264 || codec == cudaVideoCodec_H264_SVC || codec == cudaVideoCodec_H264_MVC) {
        // H.264的最坏情况
        return 20;
    } else if (codec == cudaVideoCodec_HEVC) {
        // HEVC规范：A.4.1 通用级别和层级限制
        auto MaxLumaPS = 35651584U;
        int MaxDpbPicBuf = 6;
        int PicSizeInSamplesY = width * height;
        int MaxDpbSize;

        if (PicSizeInSamplesY <= (MaxLumaPS >> 2U)) {
            MaxDpbSize = MaxDpbPicBuf * 4;
        } else if (PicSizeInSamplesY <= (MaxLumaPS >> 1U)) {
            MaxDpbSize = MaxDpbPicBuf * 2;
        } else if (PicSizeInSamplesY <= ((3U * MaxLumaPS) >> 2U)) {
            MaxDpbSize = (MaxDpbPicBuf * 4) / 3;
        } else {
            MaxDpbSize = MaxDpbPicBuf;
        }

        return std::min(MaxDpbSize, 16) + 4;
    }
    
    return 8;  // 默认
}

VideoDecoder::VideoDecoder(bool low_latency, int max_width, int max_height) {
    // 初始化CUDA上下文
    if (!InitCudaContext()) {
        throw std::runtime_error("Failed to initialize CUDA context");
    }
    
    // 创建CUDA流
    CHECK_CUDA(cuStreamCreate(&cuda_stream_, 0));
    
    // 创建上下文锁
    // CHECK_CUDA(cuvidCtxLockCreate(&ctx_lock_, cuda_context_));
    
    // 创建视频解析器
    CUVIDPARSERPARAMS parser_params = {};
    parser_params.CodecType = codec_;
    parser_params.ulMaxNumDecodeSurfaces = 20;  // 初始值，会在HandleVideoSequence中更新
    parser_params.ulMaxDisplayDelay = low_latency ? 0 : 1;
    parser_params.pUserData = this;
    parser_params.pfnSequenceCallback = HandleVideoSequence;
    parser_params.pfnDecodePicture = HandlePictureDecode;
    parser_params.pfnDisplayPicture = HandlePictureDisplay;
    
    CHECK_CUDA(cuvidCreateVideoParser(&video_parser_, &parser_params));
    
    // 启动解码线程
    decoding_thread_ = std::thread(&VideoDecoder::DecodingThread, this);
}

VideoDecoder::~VideoDecoder() {
    // 发送结束标记并等待解码线程结束
    stop_flag_ = true;
    
    // 发送流结束包
    CUVIDSOURCEDATAPACKET end_packet = {};
    end_packet.flags = CUVID_PKT_ENDOFSTREAM;
    input_queue_.Push(end_packet);
    
    // 等待解码线程结束
    if (decoding_thread_.joinable()) {
        decoding_thread_.join();
    }
    
    // 释放已分配的GPU内存
    for (auto ptr : allocated_ptrs_) {
        cuMemFree(ptr);
    }
    allocated_ptrs_.clear();
    
    // 清理解码器资源
    if (video_decoder_) {
        cuvidDestroyDecoder(video_decoder_);
        video_decoder_ = nullptr;
    }
    
    // 清理解析器资源
    if (video_parser_) {
        cuvidDestroyVideoParser(video_parser_);
        video_parser_ = nullptr;
    }
    
    // 清理上下文锁
    if (ctx_lock_) {
        cuvidCtxLockDestroy(ctx_lock_);
        ctx_lock_ = nullptr;
    }
    
    // 清理CUDA流
    if (cuda_stream_) {
        cuStreamDestroy(cuda_stream_);
        cuda_stream_ = nullptr;
    }
    
    // 清理CUDA上下文
    DestroyCudaContext();
}

bool VideoDecoder::InitCudaContext() {
    CHECK_CUDA(cuInit(0));
    
    CUdevice cuda_device;
    CHECK_CUDA(cuDeviceGet(&cuda_device, 0));
    
    CHECK_CUDA(cuCtxCreate(&cuda_context_, 0, cuda_device));
    
    return true;
}

void VideoDecoder::DestroyCudaContext() {
    if (cuda_context_) {
        cuCtxDestroy(cuda_context_);
        cuda_context_ = nullptr;
    }
}

bool VideoDecoder::OpenFile(const std::string& filename) {
    // 读取文件内容
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open video file: " << filename << std::endl;
        return false;
    }
    
    // 获取文件大小
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 读取文件数据
    file_data_.resize(size);
    if (!file.read(reinterpret_cast<char*>(file_data_.data()), size)) {
        std::cerr << "Failed to read video file" << std::endl;
        return false;
    }
    
    // 重置解码状态
    data_pos_ = 0;
    
    // 设置初始数据包大小 (可调整)
    const size_t packet_size = 256 * 1024;
    
    // 发送数据包进行解码
    while (data_pos_ < file_data_.size()) {
        // 计算当前数据包大小
        size_t current_size = std::min(packet_size, file_data_.size() - data_pos_);
        
        // 创建数据包
        CUVIDSOURCEDATAPACKET packet = {};
        packet.payload = file_data_.data() + data_pos_;
        packet.payload_size = current_size;
        packet.flags = CUVID_PKT_TIMESTAMP;
        packet.timestamp = data_pos_;  // 使用文件位置作为时间戳
        
        // 将数据包加入队列
        input_queue_.Push(packet);
        
        // 更新位置
        data_pos_ += current_size;
    }
    
    // 发送结束标记
    CUVIDSOURCEDATAPACKET end_packet = {};
    end_packet.flags = CUVID_PKT_ENDOFSTREAM;
    input_queue_.Push(end_packet);
    
    return true;
}

bool VideoDecoder::GetFrame(DecodedFrame& frame) {
    // 检查解码错误
    if (decode_error_.load() || parser_error_.load()) {
        std::cerr << "Decoder error detected" << std::endl;
        return false;
    }
    
    // 等待并获取一帧
    if (output_queue_.Empty()) {
        return false;
    }
    
    frame = output_queue_.Pop();
    return true;
}

bool VideoDecoder::SeekToTime(int64_t timestamp_ms) {
    if (timestamp_ms == 0) {
        data_pos_ = 0;

        // 清空队列
        while (!output_queue_.Empty()) {
            DecodedFrame frame = output_queue_.Pop();
            if (allocated_ptrs_.find(frame.data) != allocated_ptrs_.end()) {
                reuse_ptr_queue_.Push(frame.data);
            }
        }

        // 修改为将 file_data_ 转换为 std::string
        return OpenFile(std::string(file_data_.begin(), file_data_.end()));
    }

    std::cerr << "Seeking to arbitrary positions not implemented" << std::endl;
    return false;
}

void VideoDecoder::DecodingThread() {
    try {
        while (!stop_flag_) {
            // 等待输入队列不为空
            if (input_queue_.Empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            
            // 获取数据包
            CUVIDSOURCEDATAPACKET packet = input_queue_.Pop();
            
            // 处理数据包
            CUresult result = cuvidParseVideoData(video_parser_, &packet);
            if (result != CUDA_SUCCESS) {
                const char* error_str;
                cuGetErrorString(result, &error_str);
                std::cerr << "Error parsing video data: " << error_str << std::endl;
                parser_error_.store(1);
                break;
            }
            
            // 如果是流结束包，且没有停止标志，则退出
            if ((packet.flags & CUVID_PKT_ENDOFSTREAM) && !stop_flag_) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in decoding thread: " << e.what() << std::endl;
        parser_error_.store(1);
    }
}

int CUDAAPI VideoDecoder::HandleVideoSequence(void* user_data, CUVIDEOFORMAT* video_format) {
    VideoDecoder* decoder = static_cast<VideoDecoder*>(user_data);
    return decoder->OnVideoSequence(video_format);
}

int CUDAAPI VideoDecoder::HandlePictureDecode(void* user_data, CUVIDPICPARAMS* pic_params) {
    VideoDecoder* decoder = static_cast<VideoDecoder*>(user_data);
    return decoder->OnPictureDecode(pic_params);
}

int CUDAAPI VideoDecoder::HandlePictureDisplay(void* user_data, CUVIDPARSERDISPINFO* disp_info) {
    VideoDecoder* decoder = static_cast<VideoDecoder*>(user_data);
    return decoder->OnPictureDisplay(disp_info);
}

int VideoDecoder::OnVideoSequence(CUVIDEOFORMAT* video_format) {
    // 检查解码器能力
    CUVIDDECODECAPS decode_caps = {};
    decode_caps.eCodecType = video_format->codec;
    decode_caps.eChromaFormat = video_format->chroma_format;
    decode_caps.nBitDepthMinus8 = video_format->bit_depth_luma_minus8;

    CHECK_CUDA(cuvidGetDecoderCaps(&decode_caps));

    if (!decode_caps.bIsSupported) {
        std::cerr << "Codec not supported on this GPU" << std::endl;
        return 0;
    }

    // 检查分辨率
    if (video_format->coded_width > decode_caps.nMaxWidth ||
        video_format->coded_height > decode_caps.nMaxHeight) {
        std::cerr << "Resolution not supported: " 
                  << video_format->coded_width << "x" << video_format->coded_height
                  << " (max: " << decode_caps.nMaxWidth << "x" << decode_caps.nMaxHeight << ")"
                  << std::endl;
        return 0;
    }

    // 保存视频参数
    width_ = video_format->display_area.right - video_format->display_area.left;
    height_ = video_format->display_area.bottom - video_format->display_area.top;
    codec_ = video_format->codec;
    chroma_format_ = video_format->chroma_format;
    bit_depth_minus_8_ = video_format->bit_depth_luma_minus8;
    bpp_ = bit_depth_minus_8_ > 0 ? 2 : 1;
    
    // 设置表面格式
    if (chroma_format_ == cudaVideoChromaFormat_420) {
        surface_format_ = bit_depth_minus_8_ > 0 ? cudaVideoSurfaceFormat_P016 : cudaVideoSurfaceFormat_NV12;
    } else if (chroma_format_ == cudaVideoChromaFormat_444) {
        surface_format_ = bit_depth_minus_8_ > 0 ? cudaVideoSurfaceFormat_YUV444_16Bit : cudaVideoSurfaceFormat_YUV444;
    }
    
    // 计算色度平面参数
    chroma_height_ = static_cast<int>(height_ * GetChromaHeightFactor(chroma_format_));
    num_chroma_planes_ = GetChromaPlaneCount(chroma_format_);
    
    // 设置其他参数
    surface_width_ = video_format->coded_width;
    surface_height_ = video_format->coded_height;
    
    // 计算帧率
    if (video_format->frame_rate.denominator) {
        frame_rate_ = static_cast<double>(video_format->frame_rate.numerator) / 
                      static_cast<double>(video_format->frame_rate.denominator);
    }
    
    // 创建解码器
    int num_decode_surfaces = video_format->min_num_decode_surfaces;
    
    // 如果解码器已存在则销毁
    if (video_decoder_) {
        cuvidDestroyDecoder(video_decoder_);
        video_decoder_ = nullptr;
    }
    
    // 创建解码器
    CUVIDDECODECREATEINFO create_info = {};
    create_info.CodecType = codec_;
    create_info.ChromaFormat = chroma_format_;
    create_info.OutputFormat = surface_format_;
    create_info.bitDepthMinus8 = bit_depth_minus_8_;
    create_info.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;
    create_info.ulNumOutputSurfaces = 2;
    create_info.ulCreationFlags = cudaVideoCreate_PreferCUVID;
    create_info.ulNumDecodeSurfaces = num_decode_surfaces;
    create_info.vidLock = ctx_lock_;
    create_info.ulWidth = video_format->coded_width;
    create_info.ulHeight = video_format->coded_height;
    create_info.ulMaxWidth = video_format->coded_width;
    create_info.ulMaxHeight = video_format->coded_height;
    create_info.ulTargetWidth = video_format->coded_width;
    create_info.ulTargetHeight = video_format->coded_height;
    
    // 设置显示区域
    create_info.display_area.left = video_format->display_area.left;
    create_info.display_area.top = video_format->display_area.top;
    create_info.display_area.right = video_format->display_area.right;
    create_info.display_area.bottom = video_format->display_area.bottom;
    
    CHECK_CUDA(cuvidCreateDecoder(&video_decoder_, &create_info));
    
    std::cout << "Created decoder: " << width_ << "x" << height_ 
              << ", codec: " << codec_
              << ", chroma format: " << chroma_format_
              << ", bit depth: " << (bit_depth_minus_8_ + 8)
              << std::endl;
    
    return num_decode_surfaces;
}

int VideoDecoder::OnPictureDecode(CUVIDPICPARAMS* pic_params) {
    if (!video_decoder_) {
        std::cerr << "Decoder not initialized" << std::endl;
        return 0;
    }
    
    CHECK_CUDA(cuvidDecodePicture(video_decoder_, pic_params));
    return 1;
}

int VideoDecoder::OnPictureDisplay(CUVIDPARSERDISPINFO* disp_info) {
    if (!video_decoder_ || !disp_info) {
        return 1;  // 跳过无效显示
    }
    
    // 映射解码后的帧
    CUVIDPROCPARAMS proc_params = {};
    proc_params.progressive_frame = disp_info->progressive_frame;
    proc_params.second_field = disp_info->repeat_first_field + 1;
    proc_params.top_field_first = disp_info->top_field_first;
    proc_params.unpaired_field = disp_info->repeat_first_field < 0;
    proc_params.output_stream = cuda_stream_;
    
    CUdeviceptr source_frame = 0;
    unsigned int source_pitch = 0;
    
    CHECK_CUDA(cuvidMapVideoFrame(video_decoder_, disp_info->picture_index, 
                                &source_frame, &source_pitch, &proc_params));
    
    // 获取解码状态
    CUVIDGETDECODESTATUS decode_status = {};
    CUresult status_result = cuvidGetDecodeStatus(video_decoder_, disp_info->picture_index, &decode_status);
    
    bool is_error = status_result == CUDA_SUCCESS && 
                   (decode_status.decodeStatus == cuvidDecodeStatus_Error || 
                    decode_status.decodeStatus == cuvidDecodeStatus_Error_Concealed);
    
    if (is_error) {
        std::cerr << "Decode error for picture " << disp_info->picture_index << std::endl;
        decode_error_.store(1);
        cuvidUnmapVideoFrame(video_decoder_, source_frame);
        return 0;
    }
    
    // 分配或复用输出帧内存
    CUdeviceptr output_frame = 0;
    if (!reuse_ptr_queue_.Empty()) {
        output_frame = reuse_ptr_queue_.Pop();
    } else {
        int total_height = height_ + chroma_height_ * num_chroma_planes_;
        int width = width_ * bpp_;
        
        CHECK_CUDA(cuMemAllocPitch(&output_frame, &frame_pitch_, width, total_height, 16));
        allocated_ptrs_.insert(output_frame);
    }
    
    // 复制亮度平面
    CUDA_MEMCPY2D copy_params = {};
    copy_params.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    copy_params.srcDevice = source_frame;
    copy_params.srcPitch = source_pitch;
    copy_params.dstMemoryType = CU_MEMORYTYPE_DEVICE;
    copy_params.dstDevice = output_frame;
    copy_params.dstPitch = frame_pitch_ ? frame_pitch_ : width_ * bpp_;
    copy_params.WidthInBytes = width_ * bpp_;
    copy_params.Height = height_;
    
    CHECK_CUDA(cuMemcpy2DAsync(&copy_params, cuda_stream_));
    
    // 复制色度平面
    copy_params.srcDevice = source_frame + source_pitch * surface_height_;
    copy_params.dstDevice = output_frame + copy_params.dstPitch * height_;
    copy_params.Height = chroma_height_;
    
    CHECK_CUDA(cuMemcpy2DAsync(&copy_params, cuda_stream_));
    
    // 如果有第二个色度平面，也复制它
    if (num_chroma_planes_ == 2) {
        copy_params.srcDevice = source_frame + source_pitch * surface_height_ * 2;
        copy_params.dstDevice = output_frame + copy_params.dstPitch * height_ * 2;
        copy_params.Height = chroma_height_;
        
        CHECK_CUDA(cuMemcpy2DAsync(&copy_params, cuda_stream_));
    }
    
    // 解除映射
    CHECK_CUDA(cuvidUnmapVideoFrame(video_decoder_, source_frame));
    
    // 同步流
    CHECK_CUDA(cuStreamSynchronize(cuda_stream_));
    
    // 创建输出帧结构
    DecodedFrame frame;
    frame.data = output_frame;
    frame.timestamp = disp_info->timestamp;
    frame.width = width_;
    frame.height = height_;
    frame.pitch = copy_params.dstPitch;
    frame.is_key_frame = disp_info->progressive_frame; // 简化处理
    
    // 将帧放入输出队列
    output_queue_.Push(frame);
    
    return 1;
}