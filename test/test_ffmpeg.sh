clear
export LD_PRELOAD=build/libvGPUde.so
# 使用指定的测试视频
./test/decode_test test/data/BigBuckBunny_320x180.mp4