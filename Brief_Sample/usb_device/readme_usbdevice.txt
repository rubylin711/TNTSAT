1. 整体介绍
该sample包含两个基本文件
sample_usbdevice.c
该文件主要实现dlna推送的功能
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用

拷贝库/linux/pub/shared_lib_striped到u盘
拷贝usbmode脚本到U盘
拷贝/linux/pub/bin/sample_usbdevice到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_usbdevice

4.流程介绍
1.system()

