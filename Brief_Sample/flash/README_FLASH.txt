1. 整体介绍
该sample包含两个基本文件
sample_flash.c
本文将主要包含flash的读，写，擦除
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝sample_flash到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_flash

4.流程介绍
  1) mt_unf_flash_init()     //flash初始化
  2) mt_unf_flash_open()     //打开flash
  3) mt_unf_flash_id()       //获取flash ID
  4) MT_FlashModeCmdTask()   //任务列表
    1) mt_unf_flash_info()   //获取flash的信息
    2) mt_unf_flash_erase()  //擦除flash分区的数据
    3) mt_unf_flash_read()   //读取flash分区的数据
    4) mt_unf_flash_write()  //写入flash分区的数据
  5) mt_unf_flash_close()    //关闭flash
  6) mt_unf_flash_deinit()   //flash去初始化