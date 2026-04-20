1. 整体介绍
该sample包含多个基本文件
sample_pvr_rec.c
该文件是关于用于录制一个频点中的所有节目的涉及函数。
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_pvr_rec 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的shared_lib_striped文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_pvr_rec -f ./pvr -c 314 6875 64
./sample_pvr_rec -f ./pvr -s 3840 27500 1 0 0

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_Dvbc()  //dvbc锁频
  4) MTADP_Fe_Connect_Dvbs()  //dvbs锁频
  5) MTADP_HDMI_Init()  //HDMI初始化
  6) MTADP_Disp_Init()  //显示初始化
  7) MTADP_VO_Init()  //vo设备的初始化
  8) MTADP_Snd_Init()  //声音设备初始化
  9) MT_DmxInit()  //Demux初始化
  10) MTADP_Search_Init()  //检索初始化
  11) MTADP_Search_GetAllPmt  //检索TS中的PMT和PAT表
  12) MT_AVplayInit()  //音视频播器放初始化
  13) MT_UNF_PVR_RecInit()  //录制功能初始化
  14) MT_UNF_PVR_PlayInit()  //初始化PVR播放模块
  15) PVR_RegisterCallBacks()  //注册pvr事件的回调函数
  16) MT_AVPlay_Start()  //开始播放
  17) MT_PVRRecCmdtask()  //功能函数
  18) MT_Stopplay()  //停止播放
  19) MT_AvplayDeInit()  //音视频播放器去初始化
  20) MTADP_Search_FreeAllPmt()  //释放TS中的PMT和PAT表
  21) MTADP_Search_DeInit()  //检索去初始化
  22) MT_DmxDeInit()  //dmx去初始化
  23) MTADP_Snd_DeInit()  //音响设备去初始化
  24) MTADP_VO_DeInit()  //vo设备的去初始化
  25) MTADP_Disp_DeInit()  //显示去初始化
  26) MTADP_HDMI_DeInit()  //HDMI去初始化
  27) MTADP_Fe_Deinit()  //Tuner去初始化
  28) mt_sys_deinit()  //系统去初始化
