1. 整体介绍
该sample包含两个基本文件
sample_miracast.c
该文件主要实现miracast功能
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
手机连接wifi，使其与平台在同一局域网中
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./miracast -n miracast_name -m mode -c channel (ex: ./miracast -n Montage-STB -m 1 -c 6)

Usage:
miracast
    -q: Exit the background
    -n: input mircast name(ex: Montage-STB)
    -m: input mircast mode(ex: 0:go 1:gc)
    -c: input mircast channel(ex: 1/6/11)
example:
    miracast -n Montage-STB -m 0 -c 6

4.流程介绍
  1)  MT_MiracastInit()          //申请投屏使用的资源
  2)  MT_MiracastStart()        //开始投屏
  3)  MT_MiracastStop()         //结束投屏
  4)  MT_MiracastDeinit()      //释放投屏申请的资源

