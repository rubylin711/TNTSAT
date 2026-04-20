1. 整体介绍
该sample包含多个基本文件
sample_wifi.c
该文件是用于配置wifi
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
运行前需要将usb_wifi模块插在板子上
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_wifi到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_wifi

4.流程介绍
  1) mt_sys_init()          //系统初始化
  2) MT_WiFiStart()         //wifi模块开始运行
  3) MT_WiFiCmdTask()       //命令列表
  4) MT_WiFiStop()          //wifi模块停止运行
  5) mt_sys_deinit()        //系统去初始化