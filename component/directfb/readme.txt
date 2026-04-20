ÔÚ¶¥²ãÄ¿Â¼ÏÂ
make directfb_base
make directfb_base_install
make directfb
make directfb_install




ifconfig eth0 down
ifconfig eth0 hw ether 86:21:38:41:80:0F
ifconfig eth0 up
udhcpc
mount -t nfs -o nolock 192.168.52.15:/home/qjiang/nfs /mnt

export PREFIX=/mnt/dfb_sdk
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PREFIX/lib
cd /mnt/dfb_sdk/lib/directfb-1.6-0
export DFBARGS=module-dir=$PWD
cp -rf /mnt/dfb_sdk/.directfbrc /root
cd /mnt/dfb_sdk/bin
./df_dok
