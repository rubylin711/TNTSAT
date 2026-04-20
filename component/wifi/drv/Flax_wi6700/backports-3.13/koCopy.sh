#1:dir name , 2: device type
rm -rf ../$1
mkdir ../$1
cp ./compat/compat.ko ../$1/
cp ./net/wireless/cfg80211.ko ../$1/

if [ $2 == "usb" ];then
	cp ./drivers/net/wireless/lynx/m88wi6700u.ko ../$1/
elif [ $2 == "sdio" ];then
	cp ./drivers/net/wireless/lynx/m88wi6700s.ko ../$1/
elif [ $2 == "cust1_sdio" ];then
	cp ./drivers/net/wireless/lynx/m88wi6700s.ko ../$1/
	cp ./drivers/net/wireless/lynx/os_dep/linux/wifi_ctrl2.ko ../$1/
fi
