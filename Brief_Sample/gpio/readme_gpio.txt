1. 整体介绍
该sample包含两个基本文件
sample_gpio.c
该文件主要实现通过GPIO控制LED灯点亮或熄灭
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_gpio到u盘，u盘接上单板
由于SYM4工板没有可控制的LED灯，因此，此sample在工板上看不到效果
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_gpio Pin0 down  //[Pin0|Pin1]是灯的引脚，[down|up]是把电压拉低或者拉高，实例是将灯的Pin0引脚拉低，即亮绿灯，如果把Pin1拉低则亮红灯

4.流程介绍
  1) MT_SYS_Init()        //系统初始化
  2) MT_Check_Pinmux()    //检查引脚复用
  3) MT_UNF_GPIO_Init()   //GPIO初始化
  4) MT_Gpio_SetPara()    //设置GPIO的参数
  5) MT_UNF_GPIO_Deinit() //GPIO去初始化
  6) MT_SYS_DeInit()      //系统去初始化