1. 整体介绍
该sample包含多个基本文件
sample_super_blindscan.c
该文件是关于super blindscan的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_super_blindscan 到u盘，linux\out\general\breif_sample\super_blindscanblindscan
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
u盘接上单板，
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_super_blindscan -b 1 -s 950 -t 1000 -k 1 -p 0

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3）MT_Blindscan_Dvbs()  //盲扫
  4) DVB_DmxInitAndSearch()  //dmx初始化和搜台
	(1) MT_UNF_DMX_Init()  //dmx初始化
	(2) MT_UNF_DMX_AttachTSPort()  //dmx端口连接tuner端口
	(3) MTADP_Search_Init()  //搜索设备初始化
	(4) MTADP_Search_GetAllPmt()  //获取pmt 
  5) DVB_DmxDeInit()  //dmx去初始化
  6) MTADP_Fe_Deinit()  //去初始化tuner
  7) MT_SYS_DeInit()  //系统去初始化
