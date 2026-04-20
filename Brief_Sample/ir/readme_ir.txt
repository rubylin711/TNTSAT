1. 整体介绍
该sample包含多个基本文件
sample_ir.c
该文件是配置获取遥控器键值的主要步骤以及涉及函数。
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
拷贝 sample_ir 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的shared_lib_striped文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_ir

4.流程介绍

1.MT_UNF_IR_Init()  //ir的初始化
2.MT_UNF_IR_SetRepKeyTimeoutAttr()  // 设置按键重复按下的响应时间
3.MT_UNF_IR_EnableKeyUp()  //按键抬起状态不识别
4.MT_UNF_IR_EnableRepKey()  //设置不报告重复按键
5.MT_UNF_IR_SetFetchMode()  //设置键获取模式或符号模式
6.MT_UNF_IR_GetValueWithProtocol()  //获取按键值
7.MT_UNF_IR_Enable()  //启用ir设备
8.MT_UNF_IR_DeInit()  //ir去初始化