1. 整体介绍
该sample包含多个基本文件
sample_wdg.c
该文件是关于使用看门狗的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
拷贝 sample_wdg 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_wdg


4.流程介绍
1.mt_unf_wdg_init()  //wdg设备的初始化
2.mt_unf_wdg_set_timeout()  //设置wdg超时时间
3.mt_unf_wdg_enable()  //wdg设备使能
4.mt_unf_wdg_clear()  //喂食操作
5.mt_unf_wdg_disable()  //wdg去使能
6.mt_unf_wdg_set_timeout()  //重新设置超时时间
7.mt_unf_wdg_enable()  //wdg设备使能
8.mt_unf_wdg_deinit()  //wdg设备的去初始化