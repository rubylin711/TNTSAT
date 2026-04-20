#!/bin/sh

log_file="$1"

poweron_lines="`grep -nr "Welcome to Montage-tech SOC" ${log_file} | awk -F':' '{print $1}'`"
echo poweron_lines=${poweron_lines}
echo ""

test_finish_lines="`grep -nr "test finish " ${log_file} | awk -F':' '{print $1}'`"
echo test_finish_lines=${test_finish_lines}
echo ""



for poweron_line_x in ${poweron_lines}; do
	last_finish_every_poweron_x=""
	for test_finish_line_x in ${test_finish_lines}; do
		if [ "${test_finish_line_x}" -lt "${poweron_line_x}" ]; then
			last_finish_every_poweron_x=${test_finish_line_x}
		fi
	done

	if [ -n "${last_finish_every_poweron_x}" ]; then
		last_ringo_every_finish_x1="`expr ${last_finish_every_poweron_x} - 1`"
		last_ringo_every_finish_x2="`expr ${last_finish_every_poweron_x} - 2`"
		echo "${last_finish_every_poweron_x}"
		last_ringo_every_finish_string_x1="`sed -n "${last_ringo_every_finish_x1}p" ${log_file}`"
		last_ringo_every_finish_true_x1="`echo "${last_ringo_every_finish_string_x1}" | grep "\]..[0-9]"`"
		if [ -n "${last_ringo_every_finish_true_x1}" ]; then
			#echo "-1"
			sed -n "${last_ringo_every_finish_x1}p" ${log_file}
		else
			#echo "-2"
			sed -n "${last_ringo_every_finish_x2}p" ${log_file}
		fi
		echo ""
	fi
done
