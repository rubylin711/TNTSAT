#!/bin/sh

all_sdXY="`ls /dev/sd*`"

for i in ${all_sdXY}; do
	ACTION="add"
	MDEV="`basename ${i}`"
	export ACTION MDEV
	/etc/mdev/automountusb.sh
done

/usr/local/bin/usb-storage-daemon &
