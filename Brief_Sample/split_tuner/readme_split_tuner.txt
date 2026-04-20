1. 整体介绍
该sample包含多个基本文件
sample_split_tuner.c
该文件是关于dvbs的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 sample_split_tuner 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_split_tuner -c 314 6875 64 -s 3840 27500 1 0 0   

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_Dvbs()  //dvbs锁频
  4) MTADP_HDMI_Init()  //HDMI初始化
  5) MTADP_Disp_Init()  //display的初始化
  6) MTADP_Snd_Init()  //音频设备初始化
  7) MTADP_VO_Init()  //vo设备的初始化
  8) DVB_DmxInitAndSearch()  //dmx初始化和搜台
	(1) MT_UNF_DMX_Init()  //dmx初始化
	(2) MT_UNF_DMX_AttachTSPort()  //dmx端口连接tuner端口
	(3) MTADP_Search_Init()  //搜索设备初始化
	(4) MTADP_Search_GetAllPmt()  //获取pmt 
   9) DVB_AvplayInit()  //音视频播放初始化
	(1) MT_UNF_AVPLAY_Init()  //AVPLAY初始化
	(2) MT_UNF_AVPLAY_Create()  //创建播放器
	(3) MT_UNF_AVPLAY_ChnOpen()  //打开音视频通道
	(4) MT_UNF_SND_CreateTrack()  //创建一个声道track
	(5) MT_UNF_SND_Attach()  //将track attach到播放器
	(6) MTADP_VO_CreatWin()  //创建一个窗口
	(7) MT_UNF_VO_AttachWindow()  //将窗口attach到播放器
	(8) MT_UNF_VO_SetWindowEnable()  //设置窗口使能
  10) DVB_StarToPlay()  //开始播放
  11) DVB_StopToPlay()  //停止播放
  12) DVB_AvplayDeInit()  //音视频播放去初始化
  13) DVB_DmxDeInit()  //dmx去初始化
  14) MTADP_VO_DeInit()  //vo设备的去初始化
  15) MTADP_Snd_DeInit()  //音频设备去初始化
  16) MTADP_Disp_DeInit()  //display的去初始化
  17) MTADP_HDMI_DeInit()  //HDMI的去初始化
  18) MTADP_Fe_Deinit()  //去初始化tuner
  19) MT_SYS_DeInit()  //系统去初始化
