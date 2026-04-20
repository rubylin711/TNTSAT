1. 整体介绍
该sample包含两个基本文件
sample_str_standby
该文件主要遥控器的配置和str待机
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_str_standby到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_str_standby

4.流程介绍
  1）MT_UNF_IR_Init()  //ir初始化
  2）MT_UNF_IR_SetRepKeyTimeoutAttr()  //设置长按按钮时的上报间隔。
  3）MT_UNF_IR_EnableKeyUp()  //配置是否报告密钥释放状态
  4）MT_UNF_IR_EnableRepKey()  //配置是否报告重复键
  5）MT_UNF_IR_SetFetchMode()  //设置键获取模式或符号模式。
  6）MT_UNF_IR_Enable()  //启用ir设备
  7）pthread_create() //检测按键的状态
==END==