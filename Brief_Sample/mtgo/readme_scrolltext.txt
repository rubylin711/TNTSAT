1. 整体介绍
该sample包含两个基本文件
sample_scrolltext.c
该文件mtgo显示滚动字幕的主要步骤以及涉及函数。
sample_mtgo_common.c
该文件包含与mtgo显示矢量文本无关的设置，如hdmi的初始化等
以MTADP打头的文件不建议客户直接使用，如需使用，copy此文件到自己app目录
sample_scrolltext.mk
该文件为编译所用到的makefile

2. 编译
执行sample_scrolltext.mk进行编译

3. 使用
拷贝 \sample_scrolltext到u盘，linux\out\general\breif_sample\mtgo\\sample_scrolltext
拷贝 lib库文件到U盘， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 breif_sample\mtgo\res\DroidSansFallbackLegacy.ttf 到U盘目录res
u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
./sample_scrolltext scrolltext


4. 流程介绍（函数功能简单描述）
（1）.MT_GO_Init() --> mtgo初始化
（2）.MT_GO_InitText() -->初始化
（3）.MT_GO_CreateLayer() --> 创建layer
（4）.MT_GO_GetLayerSurface() --> 将layer和surface绑定
（5）.MT_GO_FillRect() --> 填充背景色为紫色
（6）.MT_GO_RefreshLayer() -->刷新将其显示到屏幕
（7）.MT_GO_CreateText() -->创建字
（8）.MT_GO_SetTextColor() -->设置字的颜色为白色
（9）.MT_GO_SetTextStyle() -->设置字的风格
（10）.MT_GO_GetLayerSurface() -->将layer和surface绑定
（11）.MT_GO_TextOutEx() -->将字输出到指定的surface上
（12）.pthread_create（）——>创建线程运行按“q”退出命令
（13）.while(1) ->通过两个while（1）的循环实现字幕滚动
==END==
