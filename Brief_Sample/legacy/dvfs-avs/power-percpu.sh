#!/bin/sh

if [ -n "`cat /proc/cmdline | grep "dvfs_test_percpu"`" ]; then
	cpu="`cat /proc/cmdline | awk -F'dvfs_test_percpu=' '{print $2}' | awk '{print $1}'`"
	if [ "${cpu}" = "0" ]; then
		cpu_mask="1"
	elif  [ "${cpu}" = "1" ]; then
		cpu_mask="2"
	elif  [ "${cpu}" = "2" ]; then
		cpu_mask="4"
		echo "core number not right"
		exit 0
	elif  [ "${cpu}" = "3" ]; then
		cpu_mask="8"
		echo "core number not right"
		exit 0
	else
		echo "core number not right"
		exit 0
	fi
	all_smp_affinity="`ls /proc/irq/*/smp_affinity`"
	for x in ${all_smp_affinity}; do echo ${x}; echo ${cpu_mask} > ${x}; done
fi

taskset -p ${cpu_mask} `pidof power-percpu.sh`

taskset ${cpu_mask} max-power-consumption-percpu.sh
