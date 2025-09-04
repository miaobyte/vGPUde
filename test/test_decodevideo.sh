clear
unset LD_PRELOAD
# 使用指定的测试视频
cd test
/usr/local/cuda-12.4/bin/nvcc -o decode_test.app decode_test.cpp \
    -I/usr/local/cuda/include -L/usr/local/cuda/lib64 \
    -lcuda -lnvcuvid -gencode arch=compute_80,code=sm_80 \
    -rdc=true --compiler-options '-std=c++11'

export LD_PRELOAD=/root/github.com/miaobyte/vGPUde/build/libvGPUde.so
./decode_test.app data/istockphoto-2157626194-640_adpp_is.mp4