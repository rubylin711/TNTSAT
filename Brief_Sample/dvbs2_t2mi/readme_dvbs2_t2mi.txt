1. 整体介绍
该sample包含多个基本文件
sample_dvbs_t2mi.c
该文件是关于dvbs2 to t2mi 的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 sample_dvbs 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_dvbs_t2mi –f 3840 –s 27500 –k 1 –p 0   //4个参数分别是tunerFreq, tunerSrate, onoff_22k, polar

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_Dvbs()  //dvbs锁频
  4) MT_UNF_DMX_T2MIEnable()
  5) MT_UNF_DMX_T2MISetInCh()
  6) MT_UNF_DMX_T2MISetOutCh()
  7) MT_UNF_DMX_T2MISetPid()
  8) MT_UNF_DMX_T2MISetPlpid()
  9) MTADP_HDMI_Init()  //HDMI初始化
  10) MTADP_Disp_Init()  //display的初始化
  11) MTADP_Snd_Init()  //音频设备初始化
  12) MTADP_VO_Init()  //vo设备的初始化
  13) DVB_DmxInitAndSearch()  //dmx初始化和搜台
	(1) MT_UNF_DMX_Init()  //dmx初始化
	(2) MT_UNF_DMX_AttachTSPort()  //dmx端口连接tuner端口
	(3) MTADP_Search_Init()  //搜索设备初始化
	(4) MTADP_Search_GetAllPmt()  //获取pmt 
  14) DVB_AvplayInit()  //音视频播放初始化
	(1) MT_UNF_AVPLAY_Init()  //AVPLAY初始化
	(2) MT_UNF_AVPLAY_Create()  //创建播放器
	(3) MT_UNF_AVPLAY_ChnOpen()  //打开音视频通道
	(4) MT_UNF_SND_CreateTrack()  //创建一个声道track
	(5) MT_UNF_SND_Attach()  //将track attach到播放器
	(6) MTADP_VO_CreatWin()  //创建一个窗口
	(7) MT_UNF_VO_AttachWindow()  //将窗口attach到播放器
	(8) MT_UNF_VO_SetWindowEnable()  //设置窗口使能
  15) DVB_StarToPlay()  //开始播放
  16) DVB_StopToPlay()  //停止播放
  17) DVB_AvplayDeInit()  //音视频播放去初始化
  18) DVB_DmxDeInit()  //dmx去初始化
  19) MTADP_VO_DeInit()  //vo设备的去初始化
  20) MTADP_Snd_DeInit()  //音频设备去初始化
  21) MTADP_Disp_DeInit()  //display的去初始化
  22) MTADP_HDMI_DeInit()  //HDMI的去初始化
  23) MTADP_Fe_Deinit()  //去初始化tuner
  24) MT_SYS_DeInit()  //系统去初始化
