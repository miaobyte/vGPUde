# vgpude —— 虚拟化 GPU 编解码器

vgpude 是一个面向容器环境的 GPU 编解码虚拟化库，设计理念借鉴自 HAMi-core。  
它通过劫持和虚拟化 NVIDIA 编解码相关 API（如 cuvid、nvidia-encoder），实现对 GPU 视频解码资源的隔离与共享。

## 主要特性

- 支持多容器共享 GPU 编解码能力
- 可限制每个容器的解码器数量、算力
- 支持主流 NVIDIA 驱动和硬件平台

## 设计原理

vgpude 通过 LD_PRELOAD 劫持 libnvcuvid.so、libnvidia-encode 等库的关键 API（如 cuvidCreateDecoder、cuvidDecodePicture），实现资源隔离和虚拟化，类似 HAMi-core 对 CUDA 的处理方式。

## 快速开始

```bash
# 编译
mkdir build &&cd build &&cmake .. &&make

# 使用 LD_PRELOAD 挂载 vgpude
export LD_PRELOAD=/path/to/libvgpude.so
# 启动你的编解码应用
./your_decoder_app
```

## 参考

- [HAMi-core 项目](https://github.com/Project-HAMi/HAMi-core)

