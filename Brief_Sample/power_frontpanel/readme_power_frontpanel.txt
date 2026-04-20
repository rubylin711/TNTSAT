1. 整体介绍
该sample包含两个基本文件
sample_power_frontpanel.c
该文件主要实现通过前面板按键进入真待机状态并且唤醒
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_power_frontpanel到u盘，u盘接上单板
由于SYM4板没有前面板，因此，板子需要外接一个前面板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_power_frontpanel //前面板key[0]进入真待机状态，key[1]唤醒，key[2]退出程序，其他key无效

4.流程介绍
  1)  mt_sys_init()                   //系统初始化
  2)  MTADP_HDMI_Init()               //HDMI初始化
  3)  MTADP_Disp_Init()               //Display初始化
  4)  MT_UNF_KEYLED_Init()            //初始化前面板
  5)  MT_UNF_LED_Open()               //打开LED设备
  6)  MT_UNF_KEYLED_SelectType()      //设置前面板类型
  7)  MT_UNF_LED_Display()            //LED灯显示
  8)  MT_UNF_KEY_RepKeyTimeoutVal()   //重复按键时间
  9)  MT_UNF_KEY_IsRepKey()           //设置是否支持重复按键
  10) MT_UNF_KEY_IsKeyUp()            //设置是否上报按键抬起
  11) MT_UNF_PMOC_Init()              //PMOC初始化
  12) MT_UNF_PMOC_SetDevType()        //设置前面板类型
  13) MT_UNF_PMOC_SetWakeUpAttr()     //设置唤醒键
  14) MT_UNF_PMOC_SetGpenPin()        //将 GPEN 引脚电压拉低
  15) MT_PowerFrontpanelModeCmdTask() //根据输入键进入真待机或唤醒
  16) MT_UNF_PMOC_DeInit()            //PMOC去初始化
  17) MT_UNF_LED_Close()              //关闭LED设备
  18) MT_UNF_KEYLED_DeInit()          //前面板去初始化
  19) MTADP_Disp_DeInit()             //Display去初始化
  20) MTADP_HDMI_DeInit()             //HDMI去初始化
  21) mt_sys_deinit()                 //系统去初始化

