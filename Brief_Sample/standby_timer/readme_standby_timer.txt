1. 整体介绍
该sample包含两个基本文件
sample_standby_timer.c
该文件主要实现进入假待机状态定时唤醒
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_standby_timer到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_standby_timer

4.流程介绍
  1)  mt_sys_init()              //系统初始化
  2)  MTADP_HDMI_Init()          //HDMI初始化
  3)  MTADP_Disp_Init()          //Display初始化
  4)  mt_unf_timer_init()        //定时器初始化
  5)  MT_Fake_Standby_Timer()    //设置定时器并进入假待机状态
      1)mt_unf_timer_request()   //获取定时器
      2)timer_default_callback() //定时器中断服务函数，定时唤醒
  6)  MT_Fake_Wakeup()           //唤醒假待机状态
  7)  mt_unf_timer_release()     //释放获取的定时器
  8)  mt_unf_timer_deinit()      //定时器去初始化
  9)  MTADP_Disp_DeInit()        //Display去初始化
  10) MTADP_HDMI_DeInit()        //HDMI去初始化
  11) mt_sys_deinit()            //系统去初始化