#include "hook.h"

// 拦截 cuvidCreateDecoder
CUresult cuvidCreateDecoder(CUvideodecoder *pDecoder, CUVIDDECODECREATEINFO *pCreateInfo) {
    GET_ORIGINAL_FUNC(cuvidCreateDecoder, CUresult, (CUvideodecoder *, CUVIDDECODECREATEINFO *));
    printf("[vGPUde] Intercepting cuvidCreateDecoder...\n");
    CUresult result = original_cuvidCreateDecoder(pDecoder, pCreateInfo);
    printf("[vGPUde] cuvidCreateDecoder intercepted successfully.\n");
    return result;
}

// 拦截 cuvidDestroyDecoder
CUresult cuvidDestroyDecoder(CUvideodecoder hDecoder) {
    GET_ORIGINAL_FUNC(cuvidDestroyDecoder, CUresult, (CUvideodecoder));
    printf("[vGPUde] Intercepting cuvidDestroyDecoder...\n");
    CUresult result = original_cuvidDestroyDecoder(hDecoder);
    printf("[vGPUde] cuvidDestroyDecoder intercepted successfully.\n");
    return result;
}

// 拦截 cuvidCreateVideoParser
CUresult cuvidCreateVideoParser(CUvideoparser *pObj, CUVIDPARSERPARAMS *pParams) {
    GET_ORIGINAL_FUNC(cuvidCreateVideoParser, CUresult, (CUvideoparser *, CUVIDPARSERPARAMS *));
    printf("[vGPUde] Intercepting cuvidCreateVideoParser...\n");
    CUresult result = original_cuvidCreateVideoParser(pObj, pParams);
    printf("[vGPUde] cuvidCreateVideoParser intercepted successfully.\n");
    return result;
}

// 拦截 cuvidDestroyVideoParser
CUresult cuvidDestroyVideoParser(CUvideoparser hParser) {
    GET_ORIGINAL_FUNC(cuvidDestroyVideoParser, CUresult, (CUvideoparser));
    printf("[vGPUde] Intercepting cuvidDestroyVideoParser...\n");
    CUresult result = original_cuvidDestroyVideoParser(hParser);
    printf("[vGPUde] cuvidDestroyVideoParser intercepted successfully.\n");
    return result;
}

// 拦截 cuvidDecodePicture
CUresult cuvidDecodePicture(CUvideodecoder hDecoder, CUVIDPICPARAMS *pPicParams) {
    GET_ORIGINAL_FUNC(cuvidDecodePicture, CUresult, (CUvideodecoder, CUVIDPICPARAMS *));
    printf("[vGPUde] Intercepting cuvidDecodePicture...\n");
    CUresult result = original_cuvidDecodePicture(hDecoder, pPicParams);
    printf("[vGPUde] cuvidDecodePicture intercepted successfully.\n");
    return result;
}

// 拦截 cuvidParseVideoData
CUresult cuvidParseVideoData(CUvideoparser hParser, CUVIDSOURCEDATAPACKET *pPacket) {
    GET_ORIGINAL_FUNC(cuvidParseVideoData, CUresult, (CUvideoparser, CUVIDSOURCEDATAPACKET *));
    printf("[vGPUde] Intercepting cuvidParseVideoData...\n");
    CUresult result = original_cuvidParseVideoData(hParser, pPacket);
    printf("[vGPUde] cuvidParseVideoData intercepted successfully.\n");
    return result;
}

// 拦截 cuvidMapVideoFrame
CUresult cuvidMapVideoFrame(CUvideodecoder hDecoder, int nPicIdx, CUdeviceptr *pDevPtr, unsigned int *pPitch, CUVIDPROCPARAMS *pVPP) {
    GET_ORIGINAL_FUNC(cuvidMapVideoFrame, CUresult, (CUvideodecoder, int, CUdeviceptr *, unsigned int *, CUVIDPROCPARAMS *));
    printf("[vGPUde] Intercepting cuvidMapVideoFrame...\n");
    CUresult result = original_cuvidMapVideoFrame(hDecoder, nPicIdx, pDevPtr, pPitch, pVPP);
    printf("[vGPUde] cuvidMapVideoFrame intercepted successfully.\n");
    return result;
}



// 拦截 cuvidUnmapVideoFrame
CUresult cuvidUnmapVideoFrame(CUvideodecoder hDecoder, CUdeviceptr devPtr) {
    GET_ORIGINAL_FUNC(cuvidUnmapVideoFrame, CUresult, (CUvideodecoder, CUdeviceptr));
    printf("[vGPUde] Intercepting cuvidUnmapVideoFrame...\n");
    CUresult result = original_cuvidUnmapVideoFrame(hDecoder, devPtr);
    printf("[vGPUde] cuvidUnmapVideoFrame intercepted successfully.\n");
    return result;
}

// 拦截 cuvidGetDecodeStatus
CUresult cuvidGetDecodeStatus(CUvideodecoder hDecoder, int nPicIdx, CUVIDGETDECODESTATUS *pDecodeStatus) {
    GET_ORIGINAL_FUNC(cuvidGetDecodeStatus, CUresult, (CUvideodecoder, int, CUVIDGETDECODESTATUS *));
    printf("[vGPUde] Intercepting cuvidGetDecodeStatus...\n");
    CUresult result = original_cuvidGetDecodeStatus(hDecoder, nPicIdx, pDecodeStatus);
    printf("[vGPUde] cuvidGetDecodeStatus intercepted successfully.\n");
    return result;
}

// 拦截 cuvidGetDecoderCaps
CUresult cuvidGetDecoderCaps(CUVIDDECODECAPS *pCaps) {
    GET_ORIGINAL_FUNC(cuvidGetDecoderCaps, CUresult, (CUVIDDECODECAPS *));
    printf("[vGPUde] Intercepting cuvidGetDecoderCaps...\n");
    CUresult result = original_cuvidGetDecoderCaps(pCaps);
    printf("[vGPUde] cuvidGetDecoderCaps intercepted successfully.\n");
    return result;
}