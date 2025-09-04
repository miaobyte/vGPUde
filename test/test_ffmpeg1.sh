clear
unset LD_PRELOAD
export LD_PRELOAD=/root/github.com/miaobyte/HAMi/vGPUde/build/libvGPUde.so
video=/root/github.com/miaobyte/HAMi/vGPUde/test/data/istockphoto-2157626194-640_adpp_is.mp4
outvideo=/root/github.com/miaobyte/HAMi/vGPUde/test/data/istockphoto-out.mp4
cd /root/github.com/miaobyte/HAMi/vGPUde/ffmpegsrc/ffmpeg/
./ffmpeg -hwaccel cuda -i ${video} -c:v copy -c:a copy  ${outvideo}