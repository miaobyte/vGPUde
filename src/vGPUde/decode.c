#include <stdio.h>
#include <dlfcn.h>
#include "nvcuvid.h"
#include "cuda.h"

// 定义所有需要拦截的函数的指针类型
typedef CUresult (*cuvidCreateVideoParser_func)(CUvideoparser *pObj, CUVIDPARSERPARAMS *pParams);
typedef CUresult (*cuvidCreateDecoder_func)(CUvideodecoder *pDecoder, CUVIDDECODECREATEINFO *pCreateInfo);
typedef CUresult (*cuvidDecodePicture_func)(CUvideodecoder hDecoder, CUVIDPICPARAMS *pPicParams);

// 拦截 cuvidCreateVideoParser
CUresult cuvidCreateVideoParser(CUvideoparser *pObj, CUVIDPARSERPARAMS *pParams) {
    static cuvidCreateVideoParser_func original_func = NULL;
    
    if (!original_func) {
        void *handle = dlopen("libnvcuvid.so.1", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "Error loading libnvcuvid.so.1: %s\n", dlerror());
            return CUDA_ERROR_NOT_FOUND;
        }
        
        original_func = (cuvidCreateVideoParser_func)dlsym(handle, "cuvidCreateVideoParser");
        if (!original_func) {
            fprintf(stderr, "Error finding original cuvidCreateVideoParser: %s\n", dlerror());
            return CUDA_ERROR_NOT_FOUND;
        }
    }
    
    printf("[vGPUde] Intercepted cuvidCreateVideoParser call\n");
    
    // 这里可以添加资源限制、计数等逻辑
    
    CUresult result = original_func(pObj, pParams);
    printf("[vGPUde] Created video parser: %p, result: %d\n", *pObj, result);
    
    return result;
}

// 拦截 cuvidCreateDecoder
CUresult cuvidCreateDecoder(CUvideodecoder *pDecoder, CUVIDDECODECREATEINFO *pCreateInfo) {
    static cuvidCreateDecoder_func original_func = NULL;
    
    if (!original_func) {
        void *handle = dlopen("libnvcuvid.so.1", RTLD_LAZY);
        original_func = (cuvidCreateDecoder_func)dlsym(handle, "cuvidCreateDecoder");
    }
    
    printf("[vGPUde] Intercepted cuvidCreateDecoder call\n");
    printf("[vGPUde] Codec: %d, Width: %lu, Height: %lu\n", 
           pCreateInfo->CodecType, pCreateInfo->ulWidth, pCreateInfo->ulHeight);
    
    // 这里可以添加解码器数量限制、资源分配等逻辑
    
    CUresult result = original_func(pDecoder, pCreateInfo);
    printf("[vGPUde] Created decoder: %p, result: %d\n", *pDecoder, result);
    
    return result;
}

// 拦截 cuvidDecodePicture
CUresult cuvidDecodePicture(CUvideodecoder hDecoder, CUVIDPICPARAMS *pPicParams) {
    static cuvidDecodePicture_func original_func = NULL;
    
    if (!original_func) {
        void *handle = dlopen("libnvcuvid.so.1", RTLD_LAZY);
        original_func = (cuvidDecodePicture_func)dlsym(handle, "cuvidDecodePicture");
    }
    
    printf("[vGPUde] Decoding picture with decoder: %p, PicIdx: %d\n", 
           hDecoder, pPicParams->CurrPicIdx);
    
    // 这里可以添加解码速率限制、性能监控等逻辑
    
    return original_func(hDecoder, pPicParams);
}