1. 整体介绍
该sample包含多个基本文件
sample_diseqc2.c
该文件是关于diseqc2.0 gpio read data的主要步骤以及涉及函数。的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 sample_diseqc2 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_diseqc2  

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MT_Diseqc2SetParameter()
  4) MT_Diseqc2LNB_Test()
  5) MT_Diseqc2Sendrecv
  6) MTADP_Fe_Deinit()  //去初始化tuner
  7) MT_SYS_DeInit()  //系统去初始化
