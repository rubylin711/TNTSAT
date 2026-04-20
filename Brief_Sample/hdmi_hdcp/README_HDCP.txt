1. 整体介绍
该sample包含多个基本文件
sample_hdcp.c
该文件是关于HDCP的打开和关闭操作
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_hdcp到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_hdcp 

4.流程介绍
(1) mt_sys_init()  //系统初始化
(2) MTADP_HDMI_Init()  //hdmi初始化
(3) MTADP_Disp_Init()  //display初始化
(4) MT_Hdcpkey_clear() //清除key
(5) MT_Hdcpkey_load()  //加载key
(6) MT_HdcpOn()  // 打开hdcp
(7) MT_HdcpOff()  //关闭hdcp
(8) MTADP_Disp_DeInit()  //display去初始化
(9) MTADP_HDMI_DeInit()  //hdmi去初始化
(10) mt_sys_deinit()  //系统去初始化