1. 整体介绍
该sample包含两个基本文件
sample_smart_card.c
该文件主要实现测试conax卡和irdeto卡与板子的通信
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
需使用有卡槽的单板
将conax卡或irdeto卡插入卡槽中
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_smart_card到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_smart_card

4.流程介绍
  1)  mt_sys_init()              //系统初始化
  2)  mt_unf_sci_init()          //sci设备初始化
  3)  MT_SmcCmdTask()            //任务列表
  4)  MT_SmcStart()              //配置并打开sci设备
  5)  MT_SmcConaxATRData()       //激活conax卡并获取ATR
  6)  MT_SmcIrdetoATRData()      //激活irdeto卡并获取ATR
  7)  MT_SmcCommunication()      //板子与卡通信
  8)  MT_SmcStop()               //关闭sci设备
  9)  mt_unf_sci_deinit()        //sci设备去初始化
  10) mt_sys_deinit()            //系统去初始化