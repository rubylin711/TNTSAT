1. 整体介绍
该sample包含两个基本文件
sample_udplay.c
该文件主要实现通过UDP来播放视频
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
板子和电脑连接网线
使用软件（推荐使用WinSend_1.20）打开视频文件，设置Interface（电脑IP），设置端口（8080）组播IP（224.1.1.1）和码率（勾选上Bitrate，码率太低视频会卡顿，推荐50000000）
点击send发送数据
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_udpplay到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_udpplay -m 224.1.1.1 -p 8080//-m 组播IP -p IP端口

4.流程介绍
  1)  MT_UdPlayParase_args()     //输入参数解析
  2)  mt_sys_init()              //系统初始化
  3)  MTADP_HDMI_Init()          //HDMI初始化
  4)  MTADP_Disp_Init()          //显示初始化
  5)  MTADP_VO_Init()            //vo设备的初始化
  6)  MTADP_Snd_Init()           //声音设备初始化
  7)  MT_UdPlayDmxInit()         //Demux初始化
  8)  pthread_create()           //接收UDP广播数据的线程
  9)  MTADP_Search_GetAllPmt()   //获取PMT表
  10) MT_UdPlayAvplayInit()      //音视频播器放初始化
  11) MT_UdPlayStarToPlay()      //开始播放
  12) MT_UdPlayStopToPlay()      //停止播放
  13) MT_UdPlayAvplayDeInit()    //音视频播放器去初始化
  14) MTADP_Search_FreeAllPmt()  //释放PMT表
  15) MT_UdPlayDmxDeInit()       //Demux去初始化
  16) MTADP_Snd_DeInit()         //音响设备去初始化
  17) MTADP_VO_DeInit()          //vo设备的去初始化
  18) MTADP_Disp_DeInit()        //显示去初始化
  19) MTADP_HDMI_DeInit()        //HDMI去初始化
  20) mt_sys_deinit()            //系统去初始化
