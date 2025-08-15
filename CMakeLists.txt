cmake_minimum_required(VERSION 3.10)
project(HAMi_vGPUde)

set(CMAKE_C_STANDARD 11)

include_directories(src)
file(GLOB_RECURSE stdutil_SOURCES "src/stdutil/*.c")
add_library(stdutil SHARED 
${stdutil_SOURCES}
)

add_subdirectory(test)

