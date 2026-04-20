#!/bin/sh

cd /root
cp -af COM.DAT debugbit.good.gz NNET.DAT /tmp
cd /tmp
mkdir -p /tmp/0 /tmp/1

echo 1 > /sys/kernel/debug/regulator/vcc_cpu/high

while true; do
	if [ "$1" = "high" ]; then
		cd /tmp/0; pi_css5 21000 > /dev/null &
		cd /tmp/1; pi_css5 21000 > /dev/null
		ps > 1.log
		until [ -z "`grep pi_css5 1.log`" ]; do
			ps > 1.log
		done
		sync
		if [ -n "`diff /tmp/0/pi32768.txt /root/pi32768-x86.txt`" ]; then
			echo "/tmp/0/pi32768.txt diff with /root/pi32768-x86.txt"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/0/pi32768.txt
		if [ -n "`diff /tmp/1/pi32768.txt /root/pi32768-x86.txt`" ]; then
			echo "/tmp/1/pi32768.txt diff with /root/pi32768-x86.txt"
			echo "c" > /proc/sysrq-trigger
		fi
		rm /tmp/1/pi32768.txt

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

		echo "FREQ	LOAD	TMP	VCODE	COR0	COR1	COR2	COR3	COH0	COH1	COH2	COH3	CR0	CR1	CR2	CH0	CH1	CH2"
		if [ "$2" = "one" ]; then
			ringo_now="`cat /sys/kernel/debug/regulator/vcc_cpu/ringo`"
			freq_now="`echo ${ringo_now} | awk '{print $1}'`"
			load_now="`echo ${ringo_now} | awk '{print $2}'`"
			tmp_now="`echo ${ringo_now} | awk '{print $3}'`"
			vcode_now="`echo ${ringo_now} | awk '{print $4}'`"
			cor0_now="`echo ${ringo_now} | awk '{print $5}'`"
			cor1_now="`echo ${ringo_now} | awk '{print $6}'`"
			cor2_now="`echo ${ringo_now} | awk '{print $7}'`"
			cor3_now="`echo ${ringo_now} | awk '{print $8}'`"
			coh0_now="`echo ${ringo_now} | awk '{print $9}'`"
			coh1_now="`echo ${ringo_now} | awk '{print $10}'`"
			coh2_now="`echo ${ringo_now} | awk '{print $11}'`"
			coh3_now="`echo ${ringo_now} | awk '{print $12}'`"
			cr0_now="`echo ${ringo_now} | awk '{print $13}'`"
			cr1_now="`echo ${ringo_now} | awk '{print $14}'`"
			cr2_now="`echo ${ringo_now} | awk '{print $15}'`"
			ch0_now="`echo ${ringo_now} | awk '{print $16}'`"
			ch1_now="`echo ${ringo_now} | awk '{print $17}'`"
			ch2_now="`echo ${ringo_now} | awk '{print $18}'`"
			echo "${ringo_now}"
			#echo "${freq_now}	${load_now}	${tmp_now}	${vcode_now}	${cor0_now}	${cor1_now}	${cor2_now}	${cor3_now}	${coh0_now}	${coh1_now}	${coh2_now}	${coh3_now}	${cr0_now}	${cr1_now}	${cr2_now}	${ch0_now}	${ch1_now}	${ch2_now}"
			#echo "success, vcode=${vcode_now}, ringo0=${cr0_now}, ringo1=${cr1_now}, ringo2=${cr2_now}"
			exit
		else
			cat /sys/kernel/debug/regulator/vcc_cpu/ringo
		fi
	else
		sleep 5
		echo "FREQ	LOAD	TMP	VCODE	COR0	COR1	COR2	COR3	COH0	COH1	COH2	COH3	CR0	CR1	CR2	CH0	CH1	CH2"
		cat /sys/kernel/debug/regulator/vcc_cpu/ringo
	fi
done
