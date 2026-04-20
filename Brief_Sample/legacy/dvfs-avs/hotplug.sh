#!/bin/sh

cd /root
cp -af COM.DAT debugbit.good.gz NNET.DAT /tmp
cd /tmp

echo 1200000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

while true; do
	pi_css5 2100000 > /dev/null &
	sleep 2
	ps > 1.log
	pid=`grep "pi_css5" 1.log | awk '{print $1}'`
	taskset -p 0x2 $pid
	sleep 180
	sync
	if [ -n "`diff /tmp/pi4194304.txt /root/pi4194304-x86.txt`" ]; then
		echo "/tmp/pi4194304.txt diff with /root/pi4194304-x86.txt"
		echo "c" > /proc/sysrq-trigger
	fi

	stream_c.exe > /dev/null &
	sleep 0.1
	ps > 1.log
	pid=`grep "stream_c.exe" 1.log | awk '{print $1}'`
	taskset -p 0x2 $pid
	sleep 10

	djpeg /root/1.jpg > /tmp/1.jpg.result &
	sleep 0.1
	ps > 1.log
	pid=`grep "djpeg" 1.log | awk '{print $1}'`
	taskset -p 0x2 $pid
	sleep 4
	sync
	if [ -n "`diff /tmp/1.jpg.result /root/1.jpg-x86.result`" ]; then
		echo "/tmp/1.jpg.result diff with /root/1.jpg-x86.result"
		echo "c" > /proc/sysrq-trigger
	fi

	linpack > /tmp/1.linpack.result &
	linpack > /tmp/2.linpack.result
	ps > 1.log
	until [ -z "`grep linpack 1.log`" ]; do
		ps > 1.log
	done
	sync
	if [ -n "`grep "calculate result wrong" /tmp/1.linpack.result`" ]; then
		echo "linpack 1 calculate wrong"
		echo "c" > /proc/sysrq-trigger
	fi
	if [ -n "`grep "calculate result wrong" /tmp/2.linpack.result`" ]; then
		echo "linpack 2 calculate wrong"
		echo "c" > /proc/sysrq-trigger
	fi
	rm /tmp/1.linpack.result /tmp/2.linpack.result

	echo 1 > /sys/kernel/debug/clk/cpu/max_power_consumption
	max-power-consumption > /dev/null &
	max-power-consumption > /dev/null
	ps > 1.log
	until [ -z "`grep max-power-consumption 1.log`" ]; do
		ps > 1.log
	done

	echo 0x2 > /sys/kernel/debug/cache_and_bus_test

	echo ""

	echo 0 > /sys/devices/system/cpu/cpu1/online
	echo ""

	if [ "$1" = "delay" ]; then
		sleep 30
	fi

	echo 1 > /sys/devices/system/cpu/cpu1/online
	echo ""
done

