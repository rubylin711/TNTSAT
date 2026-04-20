1. 整体介绍
该sample包含多个基本文件
sample_macrovision.c
该文件是关于视频播放以及macrovision的四个模式切换的主要步骤以及涉及函数。
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_macrovision 到u盘， u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./ sample_macrovision  -f  你的视频路径名称
./ sample_macrovision  -c  [频率] [速率] [幅度调制]
./ sample_macrovision -s  [频率] [速率] [22K] [极性] [类型]
比如 ./ sample_macrovision  -f  ./subtitle.ts
     ./ sample_macrovision  -c  314 6875 64
     ./ sample_macrovision -s  3840 27500 1 0 0
4.流程介绍
  1) mt_sys_init ()  //系统初始化
  2) MTADP_Fe_Init() //锁频初始化
  3) MTADP_Fe_Connect_Dvbc() //DVBC锁频
  4) MTADP_Fe_Connect_Dvbs()//DVBS锁频
  5) MTADP_HDMI_Init()  //HDMI初始化
  6) MTADP_Disp_Init()  //display的初始化
  7) MTADP_VO_Init()  //vo设备的初始化
  8) MTADP_Snd_Init()  //音频设备初始化
  9) MT_MacroVisionDmxInit  //dmx初始化
 10  InjectTsTask()  //线程函数，读取U盘视频文件信息
 11) MTADP_Search_GetAllPmt()   //获取PMT表
 12) MT_MacroVisionAvplayInit ()  //AVpaly设备初始化
 13) MT_UNF_DISP_Setmacrovision ()  //开启macrovision-A模式 
 14) MT_MacroVisionStarToPlay ()  //开始播放
 15) MT_MacroVisionCmdTask ()       //切台，退出和macrovision-A模式的转换
 16) MT_MacroVisionStopToPlay ()     //停止播放
 17) MT_MacroVisionAvplayDeInit  //音视频播放去初始化
 18) MTADP_Search_FreeAllPmt //释放PMT表
 19) MT_MacroVisionDmxDeInit ()  //dmx去初始化
 20) MTADP_Snd_DeInit()  //音频设备去初始化
 21) MTADP_VO_DeInit()  //vo设备的去初始化
 22) MTADP_Disp_DeInit()  //display的去初始化
 23) MTADP_HDMI_DeInit()  //HDMI的去初始化
 24) MTADP_Fe_DeInit()  //锁频去初始化
 25) mt_sys_deinit ()  //系统去初始化
