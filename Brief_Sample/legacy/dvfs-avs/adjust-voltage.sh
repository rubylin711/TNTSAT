#!/bin/sh

if [ -f "/usr/local/stbdata/high_low" ]; then
	high_low="`cat /usr/local/stbdata/high_low`"
else
	high_low="$1"
fi

if [ -f "/usr/local/stbdata/start_vcode" ]; then
	start_vcode="`cat /usr/local/stbdata/start_vcode`"
else
	start_vcode="$2"
fi

diff="$3"

if [ "${start_vcode}" -gt "255" ]; then
	start_vcode=255
fi
if [ "${start_vcode}" -lt "1" ]; then
	start_vcode=1
fi

echo "high or low: ${high_low}"
echo "start vcode: ${start_vcode}"
echo "diff: ${diff}"

x=${start_vcode}

if [ "${high_low}" = "high" ]; then
	echo 1 > /sys/kernel/debug/regulator/vcc_cpu/high
else
	echo 0 > /sys/kernel/debug/regulator/vcc_cpu/high
fi

cd /root
cp -af COM.DAT debugbit.good.gz NNET.DAT /tmp
cd /tmp
mkdir -p /tmp/0 /tmp/1


read_avs_data 0 &

if [ -f "/usr/local/stbdata/current_index" ]; then
	current_index="`cat /usr/local/stbdata/current_index`"
else
	current_index="`cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep cur_index | awk '{print $2}'`"
fi
cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep "index: $current_index,"

while true; do
	echo 3 > /sys/kernel/debug/clk/cpu/cpu_clk_index	#480
	sleep 0.1
	echo ${x} > /sys/kernel/debug/regulator/vcc_cpu/vcode
	sleep 0.1
	echo ${current_index} > /sys/kernel/debug/clk/cpu/cpu_clk_index
	sleep 0.1

	if [ "${high_low}" = "high" ]; then
		cd /tmp/0; pi_css5 2100000 > /dev/null &
		cd /tmp/1; pi_css5 2100000 > /dev/null
		ps > 1.log
		until [ -z "`grep pi_css5 1.log`" ]; do
			ps > 1.log
		done
		sync
		if [ -n "`diff /tmp/0/pi4194304.txt /root/pi4194304-x86.txt`" ]; then
			echo "/tmp/0/pi4194304.txt diff with /root/pi4194304-x86.txt"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/0/pi4194304.txt
		if [ -n "`diff /tmp/1/pi4194304.txt /root/pi4194304-x86.txt`" ]; then
			echo "/tmp/1/pi4194304.txt diff with /root/pi4194304-x86.txt"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/1/pi4194304.txt

		cd /tmp

		stream_c.exe > /dev/null &
		stream_c.exe > /dev/null
		ps > 1.log
		until [ -z "`grep stream_c.exe 1.log`" ]; do
			ps > 1.log
		done

		djpeg /root/1.jpg > /tmp/1.jpg.result &
		djpeg /root/2.jpg > /tmp/2.jpg.result
		ps > 1.log
		until [ -z "`grep djpeg 1.log`" ]; do
			ps > 1.log
		done
		sync
		if [ -n "`diff /tmp/1.jpg.result /root/1.jpg-x86.result`" ]; then
			echo "/tmp/1.jpg.result diff with /root/1.jpg-x86.result"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/1.jpg.result
		if [ -n "`diff /tmp/2.jpg.result /root/2.jpg-x86.result`" ]; then
			echo "/tmp/2.jpg.result diff with /root/2.jpg-x86.result"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/2.jpg.result

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
		echo "test finish high"

		x="`expr $x - $diff`"
		if [ "${x}" -lt "1" ]; then
			x=1
			echo "vcode=1 finished high, stop test"
			exit 0
		fi

	else
		sleep 120
		echo "test finish low"

		x="`expr $x - $diff`"
		if [ "${x}" -lt "1" ]; then
			x=1
			echo "vcode=1 finished low, stop test"
			exit 0
		fi
	fi

done

