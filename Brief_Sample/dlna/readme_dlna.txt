1. 整体介绍
该sample包含两个基本文件
sample_dlna.c
该文件主要实现dlna推送的功能
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
配置好STB的网络
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_dlna到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
检查网络是否连通(udhcpc配置网络)
./sample_dlna  (然后使用腾讯视屏推送视屏，默认名称为MT_dlna_test)


4.流程介绍
1.mt_sys_init()  //系统初始化
2.MTADP_HDMI_Init()  //HDMI初始化
3.MTADP_Disp_Init()  //显示初始化
4.MTADP_Snd_Init()  //声音设备初始化
5.MTADP_VO_Init()  //vo设备的初始化
6.MT_DLNA_Create_RootDev()  //创建root设备
7.MT_DLNA_Create_DMR()  //创建DMR
8.MT_DLNA_Start() //启动dlna
9.MT_DLNA_AvPlayInit()  //av播放器初始化
10.MT_DLNA_Avplay_Start() //启动播放器
11.MT_DLNA_Stop()  //停止dlna
12.MT_SVR_PLAYER_Deinit()  //svr播放器去初始化
13.MT_SVR_PLAYER_Destroy()  //svr播放器销毁
14.MT_DLNA_Delete_DMR()  //删除DMR
15.MT_DLNA_Delete_RootDev()  //删除root设备
16.MTADP_VO_DeInit()  //vo设备的去初始化
17.MTADP_Snd_DeInit()  //音响设备去初始化
18.MTADP_Disp_DeInit()  //显示去初始化
19.MTADP_HDMI_DeInit()  //HDMI去初始化
20.mt_sys_deinit()  //系统去初始化

