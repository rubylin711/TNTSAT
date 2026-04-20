1. 整体介绍
该sample包含三个基本文件
sample_showlogo.c 
是实现播放视频中的一帧
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile.mk进行编译

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_showlogo到u盘，u盘接上单板
拷贝logo3.m2v文件到u盘中
拷贝到u盘，u盘接上单板
执行以下指令：
#mount /dev/sda1 /mnt/
#export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
#cd /mnt
#./sample_showlogo -f ./logo3.m2v //我们能播出的图片是通过Eagle工具转换出来

4. 流程介绍（函数功能简单描述）
  1)  MT_ShowLogoModeParase_args()   //输入参数解析
  2)  mt_sys_init()                  //系统初始化
  3)  MTADP_HDMI_Init()              //HDMI初始化
  4)  MTADP_Disp_Init()              //显示初始化
  5)  MTADP_VO_Init()                //vo设备的初始化
  6)  MT_ShowLogoModeInit()          //视频初始化
  7)  MT_ShowLogoModeStartEs()       //播放文件的一帧图片
  8)  MT_ShowLogoModeCmdTask()       //任务列表
  9)  MT_ShowLogoModeStopEs()        //清除图片
  10) MT_ShowLogoModeDeinit()        //视频去初始化
  11) MTADP_VO_DeInit()              //vo设备的去初始化
  12) MTADP_Disp_DeInit()            //显示去初始化
  13) MTADP_HDMI_DeInit()            //HDMI去初始化
  14) mt_sys_deinit()                //系统去初始化