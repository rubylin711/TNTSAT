1. 整体介绍
该sample包含两个基本文件
sample_cipher_r2r.c
该文件主要实现对数据的加解密
Makefile
该文件为编译所用到的makefile

2. 编译
在文件所在目录下执行make进行编译
make

3. 使用
将我们要加密的encrypt.txt文件拷贝到u盘
拷贝库/linux/pub/shared_lib_striped到u盘
拷贝/linux/pub/bin/sample_cipher_r2r到u盘，u盘接上单板
执行以下指令：
mount /dev/sda1 /mnt/
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt/
./sample_cipher_r2r  //需要先加密，再解密，并且加密和解密方式需一致

4.流程介绍
  1)  mt_sys_init()                        //系统初始化
  2)  mt_unf_cipher_keyslot_request()      //申请密钥槽
  3)  mt_unf_cipher_keyslot_set()          //设置密钥
  4)  mt_unf_cipher_crypto_create()        //获取用于加密或解密的句柄
  5)  mt_unf_cipher_crypto_config()        //配置加解密信息
  6)  mt_unf_cipher_crypto_process()       //执行加密或解密
  7)  mt_unf_cipher_crypto_destroy()       //销毁用于加密或解密的句柄
  8)  mt_unf_cipher_keyslot_release()      //释放密钥槽
  9 ) mt_sys_deinit()                      //系统去初始化