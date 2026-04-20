1. 整体介绍
该sample包含两个基本文件
sample_box.c
该文件主要实现播放USB中的TS流文件并且对不同宽高比的视频进行黑边或裁剪
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_box到u盘，u盘接上单板
拷贝677-CC-12.ts文件到u盘中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_box -f ./677-CC-12.ts

4.流程介绍
  1)  mt_sys_init()             //系统初始化
  2)  MTADP_HDMI_Init()         //HDMI初始化
  3)  MTADP_Disp_Init()         //显示初始化
  4)  MTADP_VO_Init()           //vo设备的初始化
  5)  MTADP_Snd_Init()          //声音设备初始化
  6)  BOX_DmxInit()             //Demux初始化
  7)  InjectTsTask()            //接收文件流数据的线程
  8)  MTADP_Search_GetAllPmt()  //获取PMT表
  9)  BOX_AvplayInit()          //音视频播器放初始化
  10) BOX_StarToPlay()          //开始播放
  11) BOX_StopToPlay()          //停止播放
  12) BOX_SetAspectRatio()      //切换宽高比
  13) BOX_AvplayDeInit()        //音视频播放器去初始化
  14) MTADP_Search_FreeAllPmt() //释放PMT表
  15) BOX_DmxDeInit()           //Demux去初始化
  16) MTADP_Snd_DeInit()        //音响设备去初始化
  17) MTADP_VO_DeInit()         //vo设备的去初始化
  18) MTADP_Disp_DeInit()       //显示去初始化
  19) MTADP_HDMI_DeInit()       //HDMI去初始化
  20) mt_sys_deinit()           //系统去初始化
