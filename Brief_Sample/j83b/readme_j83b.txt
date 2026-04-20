1. 整体介绍
该sample包含两个基本文件
sample_j83b.c
该文件主要实现tuner锁频搜台播放切换台
MakeFile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_dvbc到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_j83b -f 314 -s 6875 -p 64  //-f 频率 -s 符号率 -p QAM

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_J83B()  //J83B锁频
  4) MTADP_HDMI_Init()  //HDMI初始化
  5) MTADP_Disp_Init()  //显示初始化
  6) MTADP_VO_Init()  //VO设备的初始化
  7) MTADP_Snd_Init()  //声音设备初始化
  8) DVB_DmxInitAndSearch()  //Demux初始化并检索TS中的PMT和PAT表
  9) DVB_AvplayInit()  //音视频播器放初始化
  10) DVB_StarToPlay()  //开始播放
  11) DVB_StopToPlay()  //停止播放
  12) DVB_AvplayDeInit()  //音视频播放器去初始化
  13) DVB_DmxDeInit()  //Demux去初始化
  14) MTADP_Snd_DeInit()  //音响设备去初始化
  15) MTADP_VO_DeInit()  //VO设备去初始化
  16) MTADP_Disp_DeInit()  //显示去初始化
  17) MTADP_HDMI_DeInit() //HDMI去初始化
  18) MTADP_Fe_Deinit()  //去初始化tuner
  19) MT_SYS_DeInit()  //系统去初始化