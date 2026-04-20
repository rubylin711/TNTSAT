1. 整体介绍
该sample包含两个基本文件
sample_text.c
该文件mtgo显示矢量文本的主要步骤以及涉及函数。
sample_mtgo_common.c
该文件包含与mtgo显示矢量文本无关的设置，如hdmi的初始化等
以MTADP打头的文件不建议客户直接使用，如需使用，copy此文件到自己app目录
sample_text.mk
该文件为编译所用到的makefile

2. 编译
执行sample_text.mk进行编译

3. 使用
拷贝 sample_text 到u盘，linux\out\general\breif_sample\mtgo\sample_text 
拷贝 lib库文件到U盘， linux\pub\shared_lib_striped\ 目录下的所有文件
拷贝 breif_sample\mtgo\res\DroidSansFallbackLegacy.ttf 到U盘目录res
u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
./sample_text TEXT

4. 流程介绍（函数功能简单描述）
mt_sys_init() -->
MT_GO_Init() --> 
MT_GO_InitText() -->
MT_GO_CreateLayer() --> 
MT_GO_GetLayerSurface() --> 
MT_GO_FillRect() --> 
MT_GO_CreateText() --> 
MT_GO_SetTextColor()--> 
MT_GO_SetTextStyle() --> 
MT_GO_DrawRect() --> 
MT_GO_TextOutEx() --> 
MT_GO_RefreshLayer()
==END==
