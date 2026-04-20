#!/bin/sh

echo 305 > /sys/kernel/debug/regulator/vcc_cpu/1.003v-cpu-ringo-low
echo 305 > /sys/kernel/debug/regulator/vcc_cpu/1.003v-cpu-ringo-high

echo 1200000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed

x="0"

while [ "$x" -lt "6" ]; do
	max-power-consumption > /dev/null &
	max-power-consumption > /dev/null

	echo "FREQ	LOAD	TMP	VCODE	COR0	COR1	COR2	COR3	COH0	COH1	COH2	COH3	CR0	CR1	CR2	CH0	CH1	CH2"
	cat /sys/kernel/debug/regulator/vcc_cpu/ringo

	echo "test done ${x}"
	x="`expr $x + 1`"
done
