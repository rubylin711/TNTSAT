1. 整体介绍
该sample包含两个基本文件
sample_dmx_section.c
该文件主要实现对490.ts文件section数据包检查完整性
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
将490.ts文件拷贝到u盘
拷贝/linux/pub/bin/sample_dmx_section到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_dmx_section -f ./490.ts

4.流程介绍
  1) MT_SectionModeParase_args()          //输入参数解析
  2) MT_SectionModeCheckFile()            //检查文件是否能打开
  3) mt_sys_init()                        //系统初始化
  4) MT_SectionModeDmxInit()              //Demux初始化
  5) pthread_create()                     //创建读取文件数据的线程
  6) MT_SectionModeAcquireSection()       //创建和设置filter通道
     1) MT_SectionModeRecvSection()       //获取和检查数据完整性
  7) MT_SectionModeDmxDeInit()            //Demux去初始化
  8) mt_sys_deinit()                      //系统去初始化
