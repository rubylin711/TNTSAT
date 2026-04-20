1. 整体介绍
该sample包含两个基本文件
sample_cc.c
该文件主要实现播放具有CC字幕的视频并显示CC字幕
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_cc到u盘，u盘接上单板
拷贝677-CC-12.ts文件到u盘中
拷贝字体库DroidSansMono.ttf文件到u盘下的res目录下
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_cc -f ./677-CC-12.ts

4.流程介绍
  1)  MT_CCModeParase_args        //输入参数解析       
  2)  MT_CCModeCheckFile          //检查文件是否能打开
  3)  mt_sys_init()               //系统初始化
  4)  MTADP_Fe_Init               //Tuner初始化
  5)  MT_CCModeCheckDvbcParam     //检查Dvbc的QAM是否错误
  6)  MTADP_Fe_Connect_Dvbc()     //连接Dvbc
  7)  MTADP_Fe_Connect_Dvbs()     //连接Dvbs
  8)  MTADP_HDMI_Init()           //HDMI初始化
  9)  MTADP_Disp_Init()           //显示初始化
  10) MTADP_VO_Init()             //vo设备的初始化
  11) MTADP_Snd_Init()            //声音设备初始化
  12) MT_CCModeDmxInit()          //Demux初始化
  13) pthread_create()            //读取文件流数据的线程
  14) MTADP_Search_GetAllPmt()    //获取PMT表
  15) MT_CCModeAvplayInit()       //音视频播器放初始化
  16) MT_CCModeStarToPlay()       //开始播放
  17) MT_UNF_CC_Init()            //CC模块初始化
  18) CC_Output_Init()            //CC模块输出初始化
  19) MT_CCModeCCStart()          //设置CC参数并启动CC
     1) MT_UNF_CC_Create()        //创建CC实例
     2) MT_UNF_CC_Start()         //开始CC模块
  20) MT_CCModeStartDataFilter()  //开始过滤ARIB数据
     1) CC_Data_Init()            //创建线程以读取ES流数据
     2) CC_Data_Install()         //创建和设置Demux通道
  21) MT_UNF_DISP_CreateVBI()     //创建VBI数据通道
  22) pthread_create()            //CC数据注入线程
  23) MT_CCModeCmdTask()          //任务列表
  24) CC_Data_Uninstall()         //关闭相关Demux通道
  25) CC_Data_DeInit()            //CC数据采集模块去初始化
  26) MT_UNF_DISP_DestroyVBI()    //销毁VBI数据通道
  27) MT_CCModeCCStop()           //停止CC模块
  28) CC_Output_DeInit()          //CC模块输出去初始化
  29) MT_UNF_CC_DeInit()          //CC模块去初始化
  30) MT_CCModeStopToPlay()       //停止播放
  31) MT_CCModeAvplayDeInit()     //音视频播放器去初始化
  32) MTADP_Search_FreeAllPmt()   //释放PMT表
  33) MT_CCModeDmxDeinit()        //Demux去初始化
  34) MTADP_Snd_DeInit()          //音响设备去初始化
  35) MTADP_VO_DeInit()           //vo设备的去初始化
  36) MTADP_Disp_DeInit()         //显示去初始化
  37) MTADP_HDMI_DeInit()         //HDMI去初始化
  38) MTADP_Fe_DeInit             //Tuner去初始化
  39) mt_sys_deinit()             //系统去初始化
