1. 整体介绍
该sample包含两个基本文件
sample_pvr_play.c
该文件主要实现播放录制的直播节目并进行一些特殊播放方式
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_pvr_play到u盘，u盘接上单板
录制直播节目，并将录制下来的文件（包括录制的索引文件）拷贝到u盘中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_pvr_play -f test//-f 录制的节目文件名

4.流程介绍
  1)  mt_sys_init()                  //系统初始化
  2)  MTADP_HDMI_Init()              //HDMI初始化
  3)  MTADP_Disp_Init()              //显示初始化
  4)  MTADP_VO_Init()                //vo设备的初始化
  5)  MTADP_Snd_Init()               //声音设备初始化
  6)  PVRPLAY_DmxInit()              //Demux初始化
  7)  PVRPLAY_AvplayInit()           //音视频播器放初始化
  8)  MT_UNF_PVR_PlayInit()          //播放模块初始化
  9)  PVR_RegisterCallBacks()        //绑定回调函数
  10) PVRPLAY_StartPlayBack()        //开始播放
  11) PVRPLAY_FastForwardTPlay()     //快进
  12) PVRPLAY_FastBackwardTPlay()    //快退
  13) PVRPLAY_SlowForwardTPlay()     //慢进
  14) PVRPLAY_SlowBackwardTPlay()    //慢退
  15) MT_UNF_PVR_PlayResumeChn()     //恢复正常播放状态
  16) MT_UNF_PVR_PlayPauseChn()      //暂停播放
  17) PVRPLAY_SeekToStart()          //跳到开始位置播放
  18) PVRPLAY_SeekToEnd()            //跳到结尾播放位置播放
  19) PVRPLAY_SeekForward()          //向前跳5秒钟
  20) PVRPLAY_SeekBackward()         //向后跳5秒钟
  21) PVRPLAY_PlayProgress()         //打印当前播放进度
  22) PVRPLAY_StopPlayBack()         //停止播放
  23) MT_UNF_PVR_PlayDeInit()        //播放模块去初始化
  24) PVRPLAY_AvplayDeInit()         //音视频播放器去初始化
  25) PVRPLAY_DmxDeInit()            //Demux去初始化
  26) MTADP_Snd_DeInit()             //音响设备去初始化
  27) MTADP_VO_DeInit()              //vo设备的去初始化
  28) MTADP_Disp_DeInit()            //显示去初始化
  29) MTADP_HDMI_DeInit()            //HDMI去初始化
  30) mt_sys_deinit()                //系统去初始化
