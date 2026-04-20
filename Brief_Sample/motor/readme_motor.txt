1. 整体介绍
该sample包含多个基本文件
sample_motor.c
该文件是关于motor的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 sample_motor 到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_motor

4.流程介绍
  1) MT_SYS_Init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MT_MotorSetParameter()  //
  4) mt_unf_fe_set_lnb_power()  //
  5) mt_unf_fe_diseqc_move()  //
  6) MTADP_Fe_Deinit()  //去初始化tuner
  7) MT_SYS_DeInit()  //系统去初始化
