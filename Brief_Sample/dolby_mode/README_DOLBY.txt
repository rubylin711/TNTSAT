1. 整体介绍
该sample包含两个基本文件
sample_dolby_mode.c
该文件主要实现Dolby声音模式的切换
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_dolby_mode到u盘，u盘接上单板
拷贝6ch_voices_id_7_dd_DVB_h264_25fps.trp文件到u盘中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_dolby_mode -f ./6ch_voices_id_7_dd_DVB_h264_25fps.trp

4.流程介绍
  1)  MT_DolbyModeParase_args()     //输入参数解析       
  2)  mt_sys_init()                 //系统初始化
  3)  MTADP_Fe_Init()               //Tuner初始化
  4)  MT_DolbyModeCheckDvbcParam()  //检查Dvbc的参数是否正常 
  5)  MT_DolbyModeCheckDvbsParam()  //检查Dvbc的参数是否正常 
  6)  MTADP_HDMI_Init()             //HDMI初始化
  7)  MTADP_Disp_Init()             //显示初始化
  8)  MTADP_VO_Init()               //vo设备的初始化
  9)  MTADP_Snd_Init()              //声音设备初始化
  10)  MT_DolbyModeDmxInit()        //Demux初始化
  11)  InjectTsTask()               //接收文件流数据的线程
  12)  MTADP_Search_GetAllPmt()     //获取PMT表
  13)  MT_DolbyModeAvplayInit()     //音视频播器放初始化
  14) MT_DolbyModeStarToPlay()      //开始播放
  15)  MT_DolbyModeCmdTask()        //任务列表
    1) MT_DolbyModeSNDMode()        //改变声音播放模式
  16) MT_DolbyModeStopToPlay()      //停止播放
  17) MT_DolbyModeAvplayDeInit()    //音视频播放器去初始化
  18) MTADP_Search_FreeAllPmt()     //释放PMT表
  19) MT_DolbyModeDmxDeInit()       //Demux去初始化
  20) MTADP_Snd_DeInit()            //音响设备去初始化
  21) MTADP_VO_DeInit()             //vo设备的去初始化
  22) MTADP_Disp_DeInit()           //显示去初始化
  23) MTADP_HDMI_DeInit()           //HDMI去初始化
  24) mt_sys_deinit()               //系统去初始化
