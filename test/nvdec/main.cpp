#include "video_decoder.h"
#include <iostream>
#include <opencv2/opencv.hpp>

// 将NV12转换为BGR格式
cv::Mat ConvertToBGR(CUdeviceptr device_ptr, int width, int height, int pitch) {
    // 分配主机内存
    cv::Mat nv12(height * 3 / 2, width, CV_8UC1);
    
    // 从设备内存复制到主机内存
    CUDA_MEMCPY2D copy_params = {};
    copy_params.srcMemoryType = CU_MEMORYTYPE_DEVICE;
    copy_params.srcDevice = device_ptr;
    copy_params.srcPitch = pitch;
    copy_params.dstMemoryType = CU_MEMORYTYPE_HOST;
    copy_params.dstHost = nv12.data;
    copy_params.dstPitch = width;
    copy_params.WidthInBytes = width;
    copy_params.Height = height * 3 / 2;
    
    cuMemcpy2D(&copy_params);
    
    // 将NV12转换为BGR
    cv::Mat bgr;
    cv::cvtColor(nv12, bgr, cv::COLOR_YUV2BGR_NV12);
    return bgr;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return -1;
    }
    
    try {
        // 创建解码器
        VideoDecoder decoder;
        
        // 打开视频文件
        if (!decoder.OpenFile(argv[1])) {
            std::cerr << "Failed to open video file" << std::endl;
            return -1;
        }
        
        // 输出视频信息
        std::cout << "Video size: " << decoder.GetWidth() << "x" << decoder.GetHeight() 
                  << ", framerate: " << decoder.GetFrameRate() << std::endl;
        
        // 创建窗口
        cv::namedWindow("Video", cv::WINDOW_NORMAL);
        
        // 处理解码帧
        DecodedFrame frame;
        while (decoder.GetFrame(frame)) {
            // 将设备内存转换为BGR格式
            cv::Mat bgr = ConvertToBGR(frame.data, frame.width, frame.height, frame.pitch);
            
            // 显示帧
            cv::imshow("Video", bgr);
            
            // 等待按键
            int key = cv::waitKey(30);
            if (key == 27) // ESC键
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}