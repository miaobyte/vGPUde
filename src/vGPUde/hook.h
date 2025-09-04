#ifndef HOOK_H
#define HOOK_H

#include <stdio.h>
#include <dlfcn.h>
#include "nvcuvid.h"
#include "cuda.h"

// 定义一个宏，用于检测并获取原始函数
#define GET_ORIGINAL_FUNC(func_name, ret_type, args)                              \
    static ret_type (*original_##func_name) args = NULL;                          \
    if (!original_##func_name) {                                                  \
        void *handle = dlopen("libnvcuvid.so", RTLD_LAZY);                        \
        if (!handle) {                                                            \
            fprintf(stderr, "Error loading libnvcuvid.so: %s\n", dlerror());      \
            return CUDA_ERROR_NOT_FOUND;                                          \
        }                                                                         \
        original_##func_name = (ret_type (*) args)dlsym(handle, #func_name);      \
        if (!original_##func_name) {                                              \
            fprintf(stderr, "Error finding original " #func_name ": %s\n", dlerror()); \
            return CUDA_ERROR_NOT_FOUND;                                          \
        }                                                                         \
        printf("[vGPUde] Original function " #func_name " loaded successfully.\n"); \
    }

#endif // HOOK_H