1. 整体介绍
该sample包含两个基本文件
sample_otp.c
该文件主要实现对OTP进行读数据
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_otp到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_otp

4.流程介绍
  1)  mt_sys_init()                        //系统初始化
  2)  MT_UNF_OTP_Init()                    //opt设备初始化
  3)  MT_UNF_OTP_get_pid()                 //获取pid
  4)  MT_UNF_OTP_get_package_info()        //获取包消息类型
  5)  MT_UNF_OTP_get_product()             //获取产品类型
  6)  MT_UNF_OTP_get_display_resolution()  //获取显示分辨率
  7)  MT_UNF_OTP_get_ca_vendor()           //获取CA厂商信息
  8)  MT_UNF_OTP_get_ca_version()          //获取CA版本
  9)  MT_UNF_OTP_get_chipid()              //获取芯片pid
  10) MT_UNF_OTP_Deinit()                  //opt设备去初始化
  11) mt_sys_deinit()                      //系统去初始化