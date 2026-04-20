1. 整体介绍
该sample包含两个基本文件
sample_net_dump.c
该文件主要实现获取网口的mac以及ip
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_net_dump到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_net_dump


4.流程介绍

1.socket()  //创建原始socket
2.MT_Dump_setPromisc()  //设置网卡的混杂模式
3.setsockopt()  //设置SOCKET选项
4.bind()  //绑定物理网卡
5.MT_Dump_startCapture()  //捕获网卡数据帧
  1) recvfrom()  //接收数据帧
  2) MT_Dump_parseFrame()  //解析数据帧