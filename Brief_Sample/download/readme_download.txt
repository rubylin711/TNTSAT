1. 整体介绍
该sample包含多个基本文件
sample_download.c
该文件是用于下载网络文件
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
运行前需要将板子的网线与公司内网连接
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_download到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_download -f http://192.168.32.83:8080/download/test.mp3

4.流程介绍
  1) MT_DownloadParase_args()      //输入参数解析
  2) mt_sys_init()                 //系统初始化
  3) MT_DownloadFileName()         //输出下载的文件名字
  4) Nw_DownloadURLTimeout_Gzip()  //下载文件
  5) mt_sys_deinit()               //系统去初始化