#!/bin/sh

# startup all hw breakpoint and watchpoint
echo 1 > /sys/kernel/debug/clk/cpu/max_power_consumption

# how many cores, you should run how many app
# more app not any helpful, contrarily, schedule will lead power dissipation down
while true; do
	max-power-consumption > /dev/null &
	max-power-consumption > /dev/null
	echo "FREQ	LOAD	TMP	VCODE	COR0	COR1	COR2	COR3	COH0	COH1	COH2	COH3	CR0	CR1	CR2	CH0	CH1	CH2"
	cat /sys/kernel/debug/regulator/vcc_cpu/ringo
done
