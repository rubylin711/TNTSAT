1. 整体介绍
该sample包含多个基本文件
sample_hash.c
该文件是关于hash的主要步骤以及涉及函数。

Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make
3. 使用
拷贝 sample_hash 到u盘
拷贝 lib库文件到U盘lib目录， linux\pub\shared_lib_striped\ 目录下的所有文件
u盘接上单板，
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
cd /mnt
./sample_hash -f data_file

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) mt_unf_cipher_init()  //cipher初始化
  3) mt_unf_cipher_malloc()  //malloc cipher空间
  4) mt_unf_cipher_hash_create()  //创建一个hash句柄
  5) mt_unf_cipher_hash_update()  //计算哈希值
  6) mt_unf_cipher_hash_final()  //获取最终的哈希值
  7) mt_unf_cipher_free()  //释放 cipher空间
  8) mt_unf_cipher_deinit()  //cipher去初始化
  7) mt_sys_deinit()  //系统去初始化
