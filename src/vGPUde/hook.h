#ifndef HOOK_H
#define HOOK_H

#include <stdio.h>
#include <dlfcn.h>
#include "nvcuvid.h"
#include "cuda.h"

// 定义一个通用的宏，用于拦截函数
#define DEFINE_HOOK_FUNC(ret_type, func_name, args, arg_names)                \
    typedef ret_type (*func_name##_func) args;                               \
    ret_type func_name args {                                                \
        static func_name##_func original_func = NULL;                        \
        if (!original_func) {                                                \
            void *handle = dlopen("libnvcuvid.so.1", RTLD_LAZY);             \
            if (!handle) {                                                   \
                fprintf(stderr, "Error loading libnvcuvid.so.1: %s\n", dlerror()); \
                return CUDA_ERROR_NOT_FOUND;                                 \
            }                                                                \
            original_func = (func_name##_func)dlsym(handle, #func_name);     \
            if (!original_func) {                                            \
                fprintf(stderr, "Error finding original " #func_name ": %s\n", dlerror()); \
                return CUDA_ERROR_NOT_FOUND;                                 \
            }                                                                \
            printf("[vGPUde] srcfunc " #func_name " loaded\n");             \
        }                                                                    \
        printf("[vGPUde] Intercepted " #func_name " call\n");                \
        return original_func arg_names;                                      \
    }

#endif // HOOK_H