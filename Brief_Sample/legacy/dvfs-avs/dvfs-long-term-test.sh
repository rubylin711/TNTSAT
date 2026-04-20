#!/bin/sh

ALL_FREQ="`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies`"

while true; do
	for x in ${ALL_FREQ}; do
		echo ${x} > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
		cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq
		cur_index="`cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep cur_index | awk '{print $2}'`"
		cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep "index: ${cur_index},"

		power-emu 50 > /dev/null &
		power-emu 50 > /dev/null


		date
	done
done
