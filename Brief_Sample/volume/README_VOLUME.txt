1. 整体介绍
该sample包含多个基本文件
sample_volume.c
该文件是关于ts播放以及音量加减和静音的主要步骤以及涉及函数。
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_volume 到u盘，并将你需要播放的视频放在U盘的目录下，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_volume -f 你的视频路径名称
./sample_volume -c [频率] [速率] [幅度调制]
./sample_volume -s [频率] [速率] [22K]
比如 ./sample_volume -f ./subtitle.ts
     ./sample_volume -c 654 6875 64
     ./sample_volume -s 3840 27500 0

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init() //锁频初始化
  3) MTADP_Fe_Connect_Dvbc() //DVBC锁频
  4) MTADP_Fe_Connect_Dvbs()//DVBS锁频
  5) MTADP_HDMI_Init()  //HDMI初始化
  6) MTADP_Disp_Init()  //display的初始化
  7) MTADP_Snd_Init()  //音频设备初始化
  8) MTADP_VO_Init()  //vo设备的初始化
  9) MT_DmxInit  //dmx初始化	
  10) InjectTsTask()  //线程函数，读取U盘视频文件信息
  11) MTADP_Search_Init()  //搜索设备初始化
  12) MTADP_Search_GetAllPmt()  //获取pmt 
  13) MT_AvplayInit()  //音视频播放初始化
	(1) MT_UNF_AVPLAY_Init()  //AVPLAY初始化
	(2) MT_UNF_AVPLAY_Create()  //创建播放器
	(3) MT_UNF_AVPLAY_ChnOpen()  //打开音视频通道
	(4) MT_UNF_SND_CreateTrack()  //创建一个声道track
	(5) MT_UNF_SND_Attach()  //将track attach到播放器
	(6) MTADP_VO_CreatWin()  //创建一个窗口
	(7) MT_UNF_VO_AttachWindow()  //将窗口attach到播放器
	(8) MT_UNF_VO_SetWindowEnable()  //设置窗口使能
  14) MT_UNF_SND_GetVolume()  //获取当前音量
  15) MT_AVPlay_Start()  //开始播放
  16) MT_CmdTask()       //实现音量加减和静音
  17) MT_AvplayDeInit()  //音视频播放去初始化
  18) MT_DmxDeInit()  //dmx去初始化
  19) MTADP_VO_DeInit()  //vo设备的去初始化
  20) MTADP_Snd_DeInit()  //音频设备去初始化
  21) MTADP_Disp_DeInit()  //display的去初始化
  22) MTADP_HDMI_DeInit()  //HDMI的去初始化
  23) MT_SYS_DeInit()  //系统去初始化
==END==