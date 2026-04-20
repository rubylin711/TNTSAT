1. 整体介绍
该sample包含两个基本文件
sample_audio.c
该文件主要实现播放USB中的mp3文件
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_audio到u盘，u盘接上单板
拷贝qddfd.mp3文件到u盘中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_audio -f ./1.mp3

4.流程介绍
  1)  mt_sys_init()         //系统初始化
  2)  MTADP_HDMI_Init()     //HDMI初始化
  3)  MTADP_Disp_Init()     //Display初始化
  4)  MTADP_Snd_Init()      //声音设备初始化
  5)  MT_Audio_Init()       //音频播放初始化
  6)  MT_Audio_Start_Es()   //开始播放音频
  7)  MT_Audio_Stop_Es()    //停止播放音频
  8)  MT_Audio_Pause_Es()   //暂停播放音频
  9)  MT_Audio_Resume_Es()  //恢复音频播放
  10) MT_Audio_Deinit()     //音频播放去初始化
  11) MTADP_Snd_DeInit()    //声音设备去初始化
  12) MTADP_Disp_DeInit()   //Display去初始化
  13) MTADP_HDMI_DeInit()   //HDMI去初始化
  14) mt_sys_deinit()       //系统去初始化
