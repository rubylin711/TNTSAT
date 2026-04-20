modprobe cfg80211
if [ "$1" = "sdio" ]; then
	insmod m88wi6700s.ko
fi
if [ "$1" = "usb" ]; then
	insmod m88wi6700u.ko
fi
