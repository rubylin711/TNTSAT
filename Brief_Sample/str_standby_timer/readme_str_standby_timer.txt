1. 整体介绍
该sample包含两个基本文件
sample_str_standby_timer
该文件主要遥控器的配置和str待机
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_str_standby_timer到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_str_standby_timer

4.流程介绍
  1）MT_UNF_PMOC_Init()  //pmoc初始化
  2）MT_UNF_PMOC_SetWakeUpAttr()  //设置待机时长。
  3）close hdmi/disp/sys等模块
  4）MT_UNF_PMOC_DeInit() //pmoc去初始化
==END==