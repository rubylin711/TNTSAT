1. 整体介绍
该sample包含多个基本文件
sample_dmxpat.c
该文件是关于解析pat的主要步骤以及涉及函数。

sample_dmx_pat.mk
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make -f sample_dmx_pat.mk

3. 使用
拷贝 sample_dmxpat 到u盘，并将你需要解析的视频放在U盘的目录下，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的shared_lib_striped文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_dmxpat -f ./1.ts

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MT_PATSampleDmxInit() //dmx初始化
	(1) MT_UNF_DMX_Init()  //dmx初始化
	(2) MT_UNF_DMX_AttachTSPort()  //dmx端口连接RAM端口	
  3) MT_PATSampleInjectTsTask()  //线程函数，读取U盘视频文件信息
  4）MT_PATSampleAcquireSection()  //获取pat
  5) MT_PATSampleSectionRecvFunc()
  6) MT_PATSampleParsePAT()
  7）MT_PATSampleDmxDeInit()  //dmx去初始化
	(1) MT_UNF_DMX_DeInit()  //dmx去初始化
	(2) MT_UNF_DMX_DetachTSPort()  //dmx端口断开RAM端口
  8) MT_SYS_DeInit()  //系统去初始化
