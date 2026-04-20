1. 整体介绍
该sample包含两个基本文件
sample_frontpanel.c
该文件主要前面板的配置
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_frontpanel到u盘，u盘接上单板， 
由于SYM4工板没有前面板，因此，板子需要外接一个前面板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_frontpanel -t 0  //0: fd650, 1: CT1642

4.流程介绍
  1）mt_sys_init()                 //系统初始化
  2）MT_UNF_KEYLED_Init()          //初始化KEYLED
  3）MT_UNF_LED_Open()             //开起LED器件
  4）MT_UNF_KEYLED_SelectType()    //key类型
  6）MT_UNF_KEY_RepKeyTimeoutVal() //超时时间
  7）MT_UNF_KEY_IsRepKey()         //设置是否使能重复按键
  8）MT_UNF_KEY_IsKeyUp()          //设置是否上报按键抬起
  9）MT_UNF_LED_Display()          //数码管显示的数
  10）pthread_create()             //监视按键的状态
  11）MT_UNF_LED_Close()           //关闭
  12）MT_UNF_KEYLED_DeInit()       //去初始化
  13）mt_sys_deinit()              //系统去初始化
END