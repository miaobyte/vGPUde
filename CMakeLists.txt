cmake_minimum_required(VERSION 3.10)
project(HAMi_vGPUde)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

find_package(CUDAToolkit REQUIRED)
include_directories(${CUDAToolkit_INCLUDE_DIRS})

include_directories(src)
file(GLOB_RECURSE  SOURCES "src/*.c")
add_library(vGPUde SHARED 
${SOURCES}
)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(vGPUde PRIVATE ENABLE_LOG) # 在 Debug 模式下定义 ENABLE_LOG
endif()
 

# add_subdirectory(test)

