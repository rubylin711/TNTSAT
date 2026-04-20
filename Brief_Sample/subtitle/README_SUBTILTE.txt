1. 整体介绍
该sample包含多个基本文件
sample_subtitle.c
该文件是关于subtitle播放的主要步骤以及涉及函数。
sample_subtitle_date.c
该文件主要是关于流中的节目信息过滤的函数
sample_subtitle_out.c
该文件主要是关于将subtitle输出到屏幕的函数
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_subtitle 到u盘，并将你需要播放的视频放在U盘的目录下，视频资源需要支持subtitle，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘[视频资源需要支持subtitle]
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_subtitle -f 你的视频路径名称
./sample_subtitle -c [频率] [速率] [幅度调制]
./sample_subtitle -s [频率] [速率] [22K]
比如 ./sample_subtitle -f ./subtitle.ts
     ./sample_subtitle -c 654 6875 64
     ./sample_subtitle -s 3840 27500 0

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) MTADP_MCE_Exit() //初始化
  3) MTADP_Fe_Init() //锁频初始化
  4) MTADP_Fe_Connect_Dvbc() //DVBC锁频
  5) MTADP_Fe_Connect_Dvbs()//DVBS锁频
  6) MTADP_HDMI_Init()  //HDMI初始化
  7) MTADP_Snd_Init()  //音频设备初始化
  8) MTADP_Disp_Init()  //display的初始化
  9) MTADP_VO_Init()  //vo设备的初始化
  10) MT_Subt_DmxInit  //dmx初始化
  11）pthread_create() //线程函数，读取U盘视频文件信息
  12) MTADP_Search_Init()  //搜索设备初始化
  13) MTADP_Search_GetAllPmt()  //获取pmt 
  14) MT_AvplayInit()  //音视频播放初始化
  15) MT_AVPlay_Start()  //开始播放
  16) MT_Subt_StartSubtitle //开始输出字幕
  17) MT_Subt_StartDataFilter //开始从流中获取数据
  18) MT_Subt_CmdTask  //获取按键信息，根据不同按键值做出反应
  19) MT_AvplayDeInit()  //播放去初始化
  20) MTADP_Search_FreeAllPmt()  //释放pmt
  21) MTADP_Search_DeInit()  //Search去初始化
  22) MT_Subt_DmxDeInit()  //DMX去初始化
  23) MTADP_Disp_DeInit()  //display的去初始化
  24) MTADP_Snd_DeInit  //声道去初始化
  25) MTADP_Fe_DeInit() //锁频去初始化
  26) MTADP_HDMI_DeInit()  //HDMI的去初始化
  27) MT_SYS_DeInit()  //系统去初始化
==END==