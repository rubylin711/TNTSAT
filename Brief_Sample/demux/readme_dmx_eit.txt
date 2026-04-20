1. 整体介绍
该sample包含两个基本文件
sample_dmx_eit.c
该文件主要实现DVB锁频，对eit的section数据包读取和打印
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_dmx_eit到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_dmx_eit -f 314 -s 6875 -q 64  //-f 频率 -s 符号率 -q QAM

4.流程介绍
  1）MT_SYS_Init()  //系统初始化
  2）MTADP_Fe_Init()  //初始化tuner
  3）MTADP_Fe_Connect()  //锁频
  4）MTADP_HDMI_Init()  //HDMI初始化
  5）DVB_DmxInit()  //Demux初始化 
  6）MT_AcquirePATSection() //获取PAT的section数据包
  7）MT_AcquireEITSection（）//获取EIT的section数据包
  8）DVB_DmxDeInit()  //dmx去初始化
  9）MTADP_HDMI_DeInit()  //HDMI去初始化
  10）MTADP_Fe_Deinit()  //Tuner去初始化
  11）MT_SYS_DeInit()  //系统去初始化
==END==