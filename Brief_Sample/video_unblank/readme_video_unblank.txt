1. 整体介绍
该sample包含两个基本文件
sample_disp_zoom.c
该文件主要实现DVB的Tuner锁频搜台播放或者文件播放的切换台以及设置窗口大小
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_disp_zoom到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_disp_zoom -f ./ttx.ts(文件播放)
./sample_disp_zoom -c 314  6875  64 -> frequency SymbolRate QAM(DVBC)
./sample_disp_zoom -s 3840 27500 1 0 0 -> frequency SymbolRate 22k(0:off,1:on)(DVBS)

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_Dvbc()  //dvbc锁频
  4) MTADP_Fe_Connect_Dvbs()  //dvbs锁频
  5) MTADP_HDMI_Init()  //HDMI初始化
  6) MTADP_Disp_Init()  //显示初始化
  7) MT_UNF_DISP_SetSmallWindow()  //设置窗口显示大小
  8) MTADP_VO_Init()  //vo设备的初始化
  9) MTADP_Snd_Init()  //声音设备初始化
  10) MT_DmxInit()  //Demux初始化
  11) MTADP_Search_Init()  //检索初始化
  11) MTADP_Search_GetAllPmt  //检索TS中的PMT和PAT表
  12) MT_AVplayInit()  //音视频播器放初始化
  13) MT_AVPlay_Start()  //开始播放
  14) MT_UNF_DISP_SetSmallWindow()  //设置窗口大小
  15) MT_Stopplay()  //停止播放
  16) MT_AvplayDeInit()  //音视频播放器去初始化
  17) MTADP_Search_FreeAllPmt()  //释放TS中的PMT和PAT表
  18) MTADP_Search_DeInit()  //检索去初始化
  19) MT_DmxDeInit()  //dmx去初始化
  20) MTADP_Snd_DeInit()  //音响设备去初始化
  21) MTADP_VO_DeInit()  //vo设备的去初始化
  22) MTADP_Disp_DeInit()  //显示去初始化
  23) MTADP_HDMI_DeInit()  //HDMI去初始化
  24) MTADP_Fe_Deinit()  //Tuner去初始化
  25) mt_sys_deinit()  //系统去初始化

