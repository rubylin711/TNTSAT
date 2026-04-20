#!/bin/sh
bt_chip="${1}"
bt_en_pin=${2}
rtl8723ds_start ()
{
	echo "insmod hci_uart.ko"
	for j in {1..2}
	do
		modprobe -r hci_uart
		modprobe hci_uart bt_en_gpio=${bt_en_pin}
		rtk_hciattach -n -s 115200 ttyS1 rtk_h5>/tmp/hci.log 2>&1 &
		for i in {1..10}
		do
			device=`ls /sys/class/bluetooth|grep hci`
			if [ "${device}" != "" ]; then
				return 0
			fi
			echo "i:${i}"
			sleep 1
		done
		sleep 1
	done
}

aic8800_start()
{
	echo "insmod aic8800 driver"
	modprobe -r aic_btusb
	modprobe aic_btusb
	for i in {1..10}
	do
		device=`ls /sys/class/bluetooth|grep hci`
		if [ "${device}" != "" ]; then
			return 0
		fi
		echo "i:${i}"
		sleep 1
	done
}

dbus_start ()
{
	if [ ! -f /var/run/dbus ]; then
		mkdir -p /var/run/dbus
	fi
	if [  -f /var/run/dbus/system_bus_socket ]; then
		rm -rf /var/run/dbus/system_bus_socket
	fi
	dbus-daemon --print-pid --print-address --config-file=/usr/local/share/dbus-1/system.conf
	echo "dbus-daemon start successful"
}
bluetoothd_start ()
{
	bluetoothd --configfile=/usr/local/stb/bluez.conf &
}
echo "bt chip:${bt_chip},bt enable pin:${bt_en_pin}"

if [ "${bt_chip}" = "rtl8723ds" ] || [ "${bt_chip}" = "rtl88x2cs" ]
then
	rtl8723ds_start
fi

if [ "${bt_chip}" = "aic8800" ]; then
	aic8800_start
fi
export DBUS_SYSTEM_BUS_ADDRESS="unix:path=/var/run/dbus/system_bus_socket"
dbus_start
bluetoothd_start
