#include <iostream>
#include <fstream>
#include <nvcuvid.h>
#include <cuda_runtime.h>

// 错误检查宏
#define CHECK_CUDA(call) \
    do { \
        CUresult err = call; \
        if (err != CUDA_SUCCESS) { \
            const char *errStr; \
            cuGetErrorString(err, &errStr); \
            std::cerr << "CUDA Error: " << errStr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// 回调函数，用于处理解码后的帧
int CUDAAPI HandlePictureDecode(void *pUserData, CUVIDPICPARAMS *pPicParams) {
    std::cout << "Decoded frame: " << pPicParams->CurrPicIdx << std::endl;
    return 1;
}

int CUDAAPI HandlePictureDisplay(void *pUserData, CUVIDPARSERDISPINFO *pDispInfo) {
    std::cout << "Displaying frame: " << pDispInfo->picture_index << std::endl;
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
    parserParams.CodecType = cudaVideoCodec_H264;
    parserParams.ulMaxNumDecodeSurfaces = 1;
    parserParams.ulMaxDisplayDelay = 1;
    parserParams.pUserData = nullptr;
    parserParams.pfnSequenceCallback = nullptr;
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

    // 读取并解析视频数据
    char buffer[4096];
    while (inputFile.read(buffer, sizeof(buffer)) || inputFile.gcount() > 0) {
        CUVIDSOURCEDATAPACKET packet = {};
        packet.payload_size = inputFile.gcount();
        packet.payload = reinterpret_cast<unsigned char *>(buffer);
        packet.flags = CUVID_PKT_TIMESTAMP;
        CHECK_CUDA(cuvidParseVideoData(videoParser, &packet));
    }

    // 清理资源
    CHECK_CUDA(cuvidDestroyVideoParser(videoParser));
    CHECK_CUDA(cuCtxDestroy(cuContext));

    std::cout << "Decoding completed." << std::endl;
    return EXIT_SUCCESS;
}