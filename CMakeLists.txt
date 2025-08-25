cmake_minimum_required(VERSION 3.10)
project(HAMi_vGPUde)

set(CMAKE_C_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)
option(ENABLE_LOG "Enable logging" ON)

# 将节点数量传递为编译器定义

include_directories(src)
include_directories(src/stdutil)
file(GLOB_RECURSE stdutil_SOURCES "src/stdutil/*.c")
add_library(stdutil SHARED 
${stdutil_SOURCES}
)

set(NODE_INT int32_t) # 根据需要设置节点id的整数类型
add_compile_definitions(NODE_INT=${NODE_INT})

# 如果 ENABLE_LOG 被设置为 ON，则定义 ENABLE_LOG 宏
if(ENABLE_LOG)
    target_compile_definitions(stdutil PRIVATE ENABLE_LOG)
endif()

add_subdirectory(test)

