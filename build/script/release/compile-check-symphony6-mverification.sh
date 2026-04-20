#!/bin/bash

./build/script/release/compile-check-config.sh
RESULT=$?
if [ "${RESULT}" != 0 ]; then
	echo -e "\e[0;31m check config \e[m ERROR !!!"
	exit 1
fi

mfrs_cfg_all="symphony6_fpga.cfg"

for mfrs_cfg in ${mfrs_cfg_all}; do

	source envsetup ${mfrs_cfg} ccache jenkins

	make clean
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make clean \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make uninstall
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make uninstall \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make mverification
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make mverification \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

#	make -C loader
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make -C loader \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi

	make clean
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make clean \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make uninstall
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make uninstall \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make -C loader clean
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make -C loader clean \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make -C loader uninstall
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make -C loader uninstall \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi
done
