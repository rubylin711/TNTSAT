#!/bin/sh

start_index="`cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep cur_index | awk '{print $2}'`"
x="${start_index}"

while true; do

	cur_index="`cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep cur_index | awk '{print $2}'`"
	echo "now testing"
	cat /sys/kernel/debug/clk/cpu/cpu_clk_index | grep "index: ${cur_index},"
	if [ "$1" = "low" ]; then
		sleep 2
	else
		benchmark.sh high one
	fi
	echo "index ${cur_index} done"
	temp="`cat /sys/kernel/debug/regulator/vcc_cpu/ringo | awk '{print $3}'`"
	echo "temperature: ${temp}"
	x="`expr $x + 1`"
	echo ${x} > /sys/kernel/debug/clk/cpu/cpu_clk_index

done
