#include <iostream>
#include <fstream>
#include <vector>
#include <nvcuvid.h>
#include <cuda_runtime.h>
#include <cstring>
#include <stdexcept>

// 错误检查宏
#define CHECK_CUDA(call)                                                                                 \
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

// 全局变量
CUvideodecoder decoder = nullptr;
CUVIDDECODECAPS decodeCaps = {};

// 检查解码器能力
void CheckDecoderCapabilities(cudaVideoCodec codec, cudaVideoChromaFormat chromaFormat, int bitDepth) {
    memset(&decodeCaps, 0, sizeof(decodeCaps));
    decodeCaps.eCodecType = codec;
    decodeCaps.eChromaFormat = chromaFormat;
    decodeCaps.nBitDepthMinus8 = bitDepth;

    CHECK_CUDA(cuvidGetDecoderCaps(&decodeCaps));

    if (!decodeCaps.bIsSupported) {
        throw std::runtime_error("Codec not supported on this GPU.");
    }
}

// 回调函数：处理视频序列
int CUDAAPI HandleSequence(void *pUserData, CUVIDEOFORMAT *pFormat) {
    std::cout << "Sequence callback: Width: " << pFormat->coded_width
              << ", Height: " << pFormat->coded_height
              << ", Codec: " << pFormat->codec << std::endl;

    // 检查解码器能力
    try {
        CheckDecoderCapabilities(pFormat->codec, pFormat->chroma_format, pFormat->bit_depth_luma_minus8);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    // 创建解码器
    CUVIDDECODECREATEINFO createInfo = {};
    createInfo.CodecType = pFormat->codec;
    createInfo.ChromaFormat = pFormat->chroma_format;
    createInfo.ulWidth = pFormat->coded_width;
    createInfo.ulHeight = pFormat->coded_height;
    createInfo.ulNumDecodeSurfaces = pFormat->min_num_decode_surfaces;
    createInfo.vidLock = nullptr;
    createInfo.ulCreationFlags = cudaVideoCreate_PreferCUVID;
    createInfo.ulTargetWidth = pFormat->coded_width;
    createInfo.ulTargetHeight = pFormat->coded_height;
    createInfo.ulNumOutputSurfaces = 2;
    createInfo.OutputFormat = (pFormat->chroma_format == cudaVideoChromaFormat_420 && pFormat->bit_depth_luma_minus8 > 0)
                                  ? cudaVideoSurfaceFormat_P016
                                  : cudaVideoSurfaceFormat_NV12;
    createInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;

    // 如果已存在解码器则销毁
    if (decoder != nullptr) {
        cuvidDestroyDecoder(decoder);
    }

    CUresult result = cuvidCreateDecoder(&decoder, &createInfo);
    if (result != CUDA_SUCCESS) {
        const char *errStr;
        cuGetErrorString(result, &errStr);
        std::cerr << "解码器创建失败: " << errStr << std::endl;
        return 0;
    }

    std::cout << "成功创建解码器，视频尺寸: " << pFormat->coded_width << "x" << pFormat->coded_height << std::endl;
    return 1;
}

// 回调函数：处理解码后的帧
int CUDAAPI HandlePictureDecode(void *pUserData, CUVIDPICPARAMS *pPicParams) {
    std::cout << "Decoded frame: " << pPicParams->CurrPicIdx
              << ", Width: " << pPicParams->PicWidthInMbs * 16
              << ", Height: " << pPicParams->FrameHeightInMbs * 16 << std::endl;

    CHECK_CUDA(cuvidDecodePicture(decoder, pPicParams));
    return 1;
}

// 回调函数：显示解码后的帧
int CUDAAPI HandlePictureDisplay(void *pUserData, CUVIDPARSERDISPINFO *pDispInfo) {
    std::cout << "Displaying frame: " << pDispInfo->picture_index
              << ", Timestamp: " << pDispInfo->timestamp << std::endl;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *videoFile = argv[1];

    // 初始化 CUDA
    CHECK_CUDA(cuInit(0));
    CUcontext cuContext;
    CHECK_CUDA(cuCtxCreate(&cuContext, 0, 0));

    // 创建视频解析器
    CUVIDPARSERPARAMS parserParams = {};
    parserParams.CodecType = cudaVideoCodec_H264; // 假设输入是 H.264 编码的视频
    parserParams.ulMaxNumDecodeSurfaces = 8;
    parserParams.ulMaxDisplayDelay = 1;
    parserParams.pUserData = nullptr;
    parserParams.pfnSequenceCallback = HandleSequence;
    parserParams.pfnDecodePicture = HandlePictureDecode;
    parserParams.pfnDisplayPicture = HandlePictureDisplay;

    CUvideoparser videoParser;
    CHECK_CUDA(cuvidCreateVideoParser(&videoParser, &parserParams));

    // 打开视频文件
    std::ifstream inputFile(videoFile, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Failed to open video file: " << videoFile << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> buffer(256 * 1024); // 初始缓冲区大小
    while (inputFile.read(buffer.data(), buffer.size()) || inputFile.gcount() > 0) {
        CUVIDSOURCEDATAPACKET packet = {};
        packet.payload_size = inputFile.gcount();
        packet.payload = reinterpret_cast<unsigned char *>(buffer.data());
        packet.flags = CUVID_PKT_TIMESTAMP;

        CHECK_CUDA(cuvidParseVideoData(videoParser, &packet));
    }

    // 发送结束标记
    CUVIDSOURCEDATAPACKET endPacket = {};
    endPacket.flags = CUVID_PKT_ENDOFSTREAM;
    CHECK_CUDA(cuvidParseVideoData(videoParser, &endPacket));

    // 清理资源
    if (decoder != nullptr) {
        CHECK_CUDA(cuvidDestroyDecoder(decoder));
    }
    CHECK_CUDA(cuvidDestroyVideoParser(videoParser));
    CHECK_CUDA(cuCtxDestroy(cuContext));

    std::cout << "Decoding completed." << std::endl;
    return EXIT_SUCCESS;
}