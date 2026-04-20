1. 整体介绍
该sample包含两个基本文件
sample_dvbt.c 
该文件用于DVBT/T2的锁频，以及音视频初始化，解码播放等函数。
MT_Adp_Frontend.c
该文件包含的是锁频相关的配置等
以MTADP打头的文件不建议客户直接使用，如需使用，copy此文件到自己app目录
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile.mk进行编译

3. 使用
在以下路径拷贝:\linux\pub\bin\sample_dvbt
拷贝到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
./sample_dvbt -f 565 -p 8
使用T/T2需要接码流机，而-f后的频率可以用户自己咋在码流机设置。
T/T2不需要额外设置，只需要在码流机上面修改调制方式即可。

4. 流程介绍（函数功能简单描述）
（1）.MT_SYS_Init() --> 初始化
（2）.mtadp_fe_init() -->初始化
（3）.mtadp_fe_connect_dvbtauto() --> 根据ID，符号率和带宽锁频
（4）.MTADP_HDMI_Init() --> 初始化
（5）.MTADP_Disp_Init() --> 初始化
（6）.MTADP_VO_Init() -->初始化
（7）.DVB_DmxInitAndSearch() -->搜台
（8）.DVB_AvplayInit() -->播放器初始化
（9）.DVB_StarToPlay() -->开始播放器
（10）.pthread_create（）——>创建线程运行按“q”退出命令
（11）.MT_UNF_AVPLAY_Stop（）——>停止播放
（12）.DVB_AvplayDeInit（） ->去初始化
（13）.MT_UNF_VO_DestroyWindow（） ->Win去初始化
（14）.MTADP_VO_DeInit（） ->去初始化
（15）.MTADP_Disp_DeInit（） ->显示函数去初始化
（16）.MTADP_Snd_DeInit（） ->音频函数去初始化
（17）.DVB_DmxDeInit（） ->去初始化
==END==
