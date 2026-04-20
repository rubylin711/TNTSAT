1. 整体介绍
该sample包含两个基本文件
sample_tplay.c
该文件主要实现播放USB中的TS流文件并且进行快进慢进
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_tplay到u盘，u盘接上单板
拷贝677-CC-12.ts文件到u盘中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_tplay -f ./677-CC-12.ts

4.流程介绍
  1)  MT_TPlayParase_args       //输入参数解析
  2)  mt_sys_init()             //系统初始化
  3)  MTADP_HDMI_Init()         //HDMI初始化
  4)  MTADP_Disp_Init()         //显示初始化
  5)  MTADP_VO_Init()           //vo设备的初始化
  6)  MTADP_Snd_Init()          //声音设备初始化
  7)  MT_TPlayDmxInit()         //Demux初始化
  8)  pthread_create()          //读取文件流数据的线程
  9)  MTADP_Search_GetAllPmt()  //获取PMT表
  10) MT_TPlayAvplayInit()      //音视频播器放初始化
  11) MT_TPlayStarToPlay()      //开始播放
  12) MT_TPlayStopToPlay()      //停止播放
  13) MT_TPlayFastForward()     //快进
  14) MT_TPlaySlowForward()     //慢进
  15) MT_TPlayPausePlay()       //暂停
  16) MT_TPlayResumePlay()      //恢复正常播放
  17) MT_TPlayAvplayDeInit()    //音视频播放器去初始化
  18) MTADP_Search_FreeAllPmt() //释放PMT表
  19) MT_TPlayDmxDeInit()       //Demux去初始化
  20) MTADP_Snd_DeInit()        //音响设备去初始化
  21) MTADP_VO_DeInit()         //vo设备的去初始化
  22) MTADP_Disp_DeInit()       //显示去初始化
  23) MTADP_HDMI_DeInit()       //HDMI去初始化
  24) mt_sys_deinit()           //系统去初始化
