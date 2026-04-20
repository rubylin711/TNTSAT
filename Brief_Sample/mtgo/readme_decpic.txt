1. 整体介绍
该sample包含两个基本文件
sample_decjpg.c
该文件mtgo显示矢量文本的主要步骤以及涉及函数。
sample_mtgo_common.c
该文件包含与mtgo显示矢量文本无关的设置，如hdmi的初始化等
以MTADP打头的文件不建议客户直接使用，如需使用，copy此文件到自己app目录

sample_decpic.mk
该文件为编译所用到的makefile

2. 编译
执行sample_decpic.mk进行编译
make -f sample_decpic.mk
3. 使用
将你要测试的图片放在U盘目录下
拷贝 sample_decPicture 到u盘，linux\out\general\breif_sample\mtgo\sample_decPicture
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_decpic -f 1.jpg 

4.流程介绍
 1.Sample_MTGO_Display_Init()  //显示init
 2.MT_GO_Init()  //组件init
 3.MT_GO_GetLayerDefaultParam() //根据图层ID获取默认参数，可以自己设置
 4.MT_GO_CreateLayer()  //创建一个layer
 5.MT_GO_GetLayerSurface()  //获取layer上的surface
 6.MT_GO_SetLayerAlpha()  //设置图层surface的alpha值
 8.MT_GO_FillRect()  //填充surface
 9.MT_GO_RefreshLayer()  //刷新
 10.sampe_dec_file_pic() //解码
     1.MT_GO_CreateDecoder //创建一个解码句柄
     2.MT_GO_DecImgInfo //获取图片信息
     3.设置解码后图片格式信息
     3.MT_GO_DecImgData 获取单个图片的数据
     4.MT_GO_DestroyDecoder 释放解码句柄
 11.MT_GO_Blit() //将解码数据移到显示surface
 12.MT_GO_RefreshLayer() //刷新显示surface
 13.MT_GO_FreeSurface()  //释放
 14.sample_Create_GIF() //解码GIF函数
 	MT_GO_CreateDecoder()  //创建一个解码句柄
	MT_GO_DecCommInfo()  //获取解析到信息，创建句柄
 	MT_GO_CreateSurface()  //根据图片信息创建surface，宽和高，像素格式，句柄
	MT_GO_FreeSurface()  //释放surface
	MT_GO_DestroyDecoder() //销毁解码句柄
 15.sample_RenderFrame_GIF()
	MT_GO_DecCommInfo()  //获取surface上的主要信息
	MT_GO_DecImgInfo()  //获取指定图片的信息
	MT_GO_FillRect()  //
	MT_GO_DecImgData()  //创建数据surface句柄
	MT_GO_Blit()  //搬移到显示surface
	MT_GO_FreeSurface()  //释放数据surface
 16.MT_GO_RefreshLayer()  //刷新layer
 17.sample_Destroy_GIF()
	MT_GO_DestroyDecoder()  //释放解码句柄
	MT_GO_FreeSurface()  //释放surface
 18.MT_GO_DestroyLayer()  //释放layer
 19.MT_GO_Deinit();  //组件去初始化
