#!/bin/bash

mfrs_cfg_all="symphony1_128.cfg"

for mfrs_cfg in ${mfrs_cfg_all}; do

	source envsetup ${mfrs_cfg} ccache

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

	make
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

done
