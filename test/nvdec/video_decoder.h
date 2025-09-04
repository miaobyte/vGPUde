#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <memory>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <nvcuvid.h>
#include <cuda_runtime.h>

// 表示解码后的帧结构
struct DecodedFrame {
    CUdeviceptr data;       // 解码后的帧数据在GPU上的指针
    int64_t timestamp;      // 时间戳
    int width;              // 帧宽度
    int height;             // 帧高度
    int pitch;              // 行间距(pitch)
    bool is_key_frame;      // 是否是关键帧
};

// 线程安全的队列模板
template <typename T>
class SafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;

public:
    void Push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        T item = queue_.front();
        queue_.pop();
        return item;
    }

    bool Empty() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t Size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};

class VideoDecoder {
public:
    // 构造函数: 初始化解码器
    VideoDecoder(bool low_latency = false, int max_width = 4096, int max_height = 4096);
    
    // 析构函数: 清理资源
    ~VideoDecoder();
    
    // 打开视频文件
    bool OpenFile(const std::string& filename);
    
    // 获取一帧解码后的图像
    bool GetFrame(DecodedFrame& frame);
    
    // 跳转到指定时间点(毫秒)
    bool SeekToTime(int64_t timestamp_ms);
    
    // 获取视频信息
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    double GetFrameRate() const { return frame_rate_; }
    int64_t GetDuration() const { return duration_; } // 以毫秒为单位
    
private:
    // CUVID回调函数
    static int CUDAAPI HandleVideoSequence(void* user_data, CUVIDEOFORMAT* video_format);
    static int CUDAAPI HandlePictureDecode(void* user_data, CUVIDPICPARAMS* pic_params);
    static int CUDAAPI HandlePictureDisplay(void* user_data, CUVIDPARSERDISPINFO* disp_info);
    
    // 内部回调实现
    int OnVideoSequence(CUVIDEOFORMAT* video_format);
    int OnPictureDecode(CUVIDPICPARAMS* pic_params);
    int OnPictureDisplay(CUVIDPARSERDISPINFO* disp_info);
    
    // 创建解码器
    bool CreateDecoder(CUVIDEOFORMAT* video_format);
    
    // 解码线程函数
    void DecodingThread();
    
    // CUDA上下文管理
    bool InitCudaContext();
    void DestroyCudaContext();
    
    // 视频解析和解码相关成员
    CUcontext cuda_context_ = nullptr;
    CUvideoctxlock ctx_lock_ = nullptr;
    CUvideoparser video_parser_ = nullptr;
    CUvideodecoder video_decoder_ = nullptr;
    CUstream cuda_stream_ = nullptr;
    
    // 视频参数
    int width_ = 0;
    int height_ = 0;
    int surface_height_ = 0;
    int surface_width_ = 0;
    int chroma_height_ = 0;
    int num_chroma_planes_ = 0;
    int bpp_ = 1;  // bytes per pixel
    size_t frame_pitch_ = 0;
    double frame_rate_ = 0.0;
    int64_t duration_ = 0;
    
    // 解码状态和数据
    cudaVideoCodec codec_ = cudaVideoCodec_H264;
    cudaVideoChromaFormat chroma_format_ = cudaVideoChromaFormat_420;
    cudaVideoSurfaceFormat surface_format_ = cudaVideoSurfaceFormat_NV12;
    int bit_depth_minus_8_ = 0;
    
    // 解码队列和状态
    SafeQueue<CUVIDSOURCEDATAPACKET> input_queue_;
    SafeQueue<DecodedFrame> output_queue_;
    std::thread decoding_thread_;
    std::atomic<bool> stop_flag_{false};
    std::atomic<int> decode_error_{0};
    std::atomic<int> parser_error_{0};
    
    // 已分配的GPU内存管理
    std::set<CUdeviceptr> allocated_ptrs_;
    SafeQueue<CUdeviceptr> reuse_ptr_queue_;
    
    // 视频文件数据
    std::vector<uint8_t> file_data_;
    size_t data_pos_ = 0;
};