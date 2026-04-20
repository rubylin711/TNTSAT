1. 整体介绍
该sample包含多个基本文件
sample_net.c
该文件是用于配置静态网络和动态网络
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
运行前需要将板子的网线连接
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_net到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_net -i eth0 -t static -s 10.48.144.165 -m 255.255.255.0 -g 10.48.144.1 -d 180.76.76.76 -M 00:1B:11:17:00:16

4.流程介绍
  1) MT_ETH_Open();         //打开网口
  2) MT_NetParase_args();   //输入参数解析
  3) MT_ETH_GetLinkStatus();//网口连接状态
  4) MT_NetSetDhcp()        //DHCP动态配置
  5) MT_NetSetStatic()      //Static静态配置配置IP,Mask,GW,DNS,MAC