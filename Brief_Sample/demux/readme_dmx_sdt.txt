1. 整体介绍
该sample包含两个基本文件
sample_dmx_sdt.c
该文件主要实现抓取SDT的section包并进行数据解析
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
接上天线
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_dmx_sdt到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_dmx_sdt -f 314 -s 6875 -p 64 //-f 频率 -s 符号率 -p QAM

4.流程介绍
  1)  MT_SdtModeParase_args()        //输入参数解析         
  2)  mt_sys_init()                  //系统初始化
  3)  MTADP_Fe_Init()                //Tuner初始化
  4)  MT_SdtModeCheckDvbcParam()     //检查Dvbc的参数是否错误
  5)  MT_SdtModeCheckDvbsParam()     //检查Dvbs的参数是否错误
  6)  MTADP_Fe_Connect_Dvbc()        //连接Dvbc
  7)  MTADP_Fe_Connect_Dvbs()        //连接Dvbs
  8)  MT_SdtModeDmxInit()            //Demux初始化
  9)  pthread_create()               //读取文件流数据的线程
  10) MT_SdtModeAcquireSection()     //创建和设置filter通道并解析数据
     1) MT_SdtModeSectionRead()      //获取数据
  11) MT_SdtModeDmxDeinit()          //Demux去初始化
  12) MTADP_Fe_Deinit()              //Tuner去初始化
  13) mt_sys_deinit()                //系统去初始化
