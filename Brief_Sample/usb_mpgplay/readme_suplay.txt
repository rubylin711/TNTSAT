1. 整体介绍
该sample包含多个基本文件
sample_suplay.c
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_suplay 到u盘，并将你需要播放的视频放在U盘的目录下，u盘接上单板
执行以下指令：
将linux/pub目录下的lib文件夹拷贝到U盘
#mount /dev/sda1 /mnt/
#export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
#cd /mnt
#./sample_suplay -f ./test.mp4

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) MTADP_HDMI_Init()  //HDMI初始化
  3) MTADP_Disp_Init()  //display的初始化
  4) MTADP_Snd_Init()  //音频设备初始化
  5) MTADP_VO_Init()  //vo设备的初始化
  6) MT_Suplay_AvplayInit()  //音视频播放初始化
	(1) MT_UNF_AVPLAY_Init()  //AVPLAY初始化
	(2) MT_UNF_AVPLAY_Create()  //创建播放器
	(3) MT_UNF_AVPLAY_ChnOpen()  //打开音视频通道
	(4) MT_UNF_SND_CreateTrack()  //创建一个声道track
	(5) MT_UNF_SND_Attach()  //将track attach到播放器
	(6) MTADP_VO_CreatWin()  //创建一个窗口
	(7) MT_UNF_VO_AttachWindow()  //将窗口attach到播放器
	(8) MT_UNF_VO_SetWindowEnable()  //设置窗口使能
   7) MT_Suplay_Avplay_Start()  //开始播放
   8) MT_Suplay_AvplayDeinit()  //音视频播放去初始化
   9) MTADP_VO_DeInit()  //vo设备的去初始化
  10) MTADP_Snd_DeInit()  //音频设备去初始化
  11) MTADP_Disp_DeInit()  //display的去初始化
  12) MTADP_HDMI_DeInit()  //HDMI的去初始化
  13) mt_sys_deinit()  //系统去初始化
