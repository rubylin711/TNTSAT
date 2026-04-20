1. 整体介绍
sample_optlayer.c文件
该文件对图层操作常用接口的使用说明
sample_optlayer.mk文件
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行以下命令
make -f sample_optlayer.mk

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_optlayer到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_optlayer

4. 流程介绍（函数功能简单描述）
Sample_MTGO_HDMI_Init() -->            初始化HDMI
Sample_MTGO_Display_Init() -->         初始化Display
MT_GO_Init() -->                       初始化MtGo插件
MT_GO_GetLayerDefaultParam() -->       根据图层ID获取图层默认参数
MT_GO_CreateLayer() -->                创建图层
MT_GO_SetLayerPalette() -->            设置Layer的调色板
MT_GO_GetLayerSurface() -->            创建Layer的Surface
MT_GO_FillRect() -->                   在Surface上绘制矩形
MT_GO_SetLayerAlpha() -->              设置layer_alpha的值
MT_GO_GetLayerAlpha() -->              获取layer_alpha的值
MT_GO_SetRegionAlpha() -->             设置region_alpha的值
MT_GO_GetRegionAlpha() -->             获取region_alpha的值
MT_GO_SetLayerColorkey() -->           设置Layer的colorkey，等同于colorkey的颜色在屏幕上不显示
MT_GO_GetLayerColorkey() -->           获取layer colorkey的值
MT_GO_CreateSurface() -->              创建Surface
MT_GO_SetSurfaceAlpha -->              设置Surface的Alpha值
MT_GO_DrawRect() -->                   在Surface上绘制矩形
MT_GO_Blit() -->                       实现Blit操作
MT_GO_RefreshLayer() -->               刷新图层
MT_GO_ShowLayer() -->                  显示或隐藏图层
MT_GO_FreeSurface() -->                释放Surface
MT_GO_DestroyLayer() -->               销毁图层
MT_GO_Deinit() -->                     去初始化MtGo插件
Sample_MTGO_Display_DeInit() -->       去初始化Display
Sample_MTGO_HDMI_DeInit() -->          去初始化HDMI
==END==
