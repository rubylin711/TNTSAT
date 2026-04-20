1. 整体介绍
该sample包含两个基本文件
sample_keyled.c
该文件主要前面板的配置
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_keyled到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_keyled [keyled_type] [type] [strlen]  //"keyled_type = 0: fd650" "keyled_type = 1: CT1642" "keyled_type = 4: PT6393"
					       //只有在前面板类型是PT6393时输入相应的后两位参数，fd650和CT1642时后两位参数任意输入就行

4.流程介绍
  1）MT_SYS_Init()  //系统初始化
  2）MT_UNF_KEYLED_Init()  //初始化KEYLED
  3）MT_UNF_LED_Open()  //开起LED器件
  4）MT_UNF_KEYLED_SelectType()  //key类型
  5）test_keyled_pt6393()  //如果前面板类型是pt6393就调用这个 
  6）MT_UNF_KEY_RepKeyTimeoutVal() //超时时间
  7）MT_UNF_KEY_IsRepKey()  
  8）MT_UNF_KEY_IsKeyUp()  
  9）MT_UNF_LED_Display()  //数码管显示的数
  10）pthread_create()  //监视按键的状态
  11）MT_UNF_LED_Close() //关闭
  12）MT_UNF_KEYLED_DeInit（）//去初始化
  13）MT_SYS_DeInit（） //去初始化
END