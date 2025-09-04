#!/bin/bash
clear
unset LD_PRELOAD

# 确保在正确的目录
cd "$(dirname "$0")"

rm -f nvdec_decoder
echo "正在编译 NVDEC 视频解码器代码..."

# 使用 NVCC 编译
/usr/local/cuda/bin/nvcc -o nvdec_decoder main.cpp video_decocer.cpp \
    -I/usr/local/cuda/include -L/usr/local/cuda/lib64 \
    -lcuda -lnvcuvid  \
    $(pkg-config --cflags --libs opencv4) \
    -gencode arch=compute_80,code=sm_80 \
    -rdc=true --compiler-options '-std=c++11'

if [ $? -eq 0 ]; then
    echo "编译成功!"
    
    # 直接运行
    echo "运行 NVDEC 解码器示例:"
    
    # 可选: 使用 vGPUde 库
    # export LD_PRELOAD=/root/github.com/miaobyte/vGPUde/build/libvGPUde.so
    
    # 假设测试视频位于 ../data 目录
    VIDEO_FILE="../data/istockphoto-2157626194-640_adpp_is.mp4"
    
    if [ -f "$VIDEO_FILE" ]; then
        ./nvdec_decoder "$VIDEO_FILE"
    else
        echo "未找到测试视频: $VIDEO_FILE"
        echo "请指定视频文件路径运行:"
        echo "./nvdec_decoder <视频文件路径>"
    fi
else
    echo "编译失败!"
fi