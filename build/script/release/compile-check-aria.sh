#!/bin/bash

mfrs_cfg_all="aria.cfg"

#for mfrs_cfg in ${mfrs_cfg_all}; do
#
#	git clean -dxf
#	git reset --hard
#
#	source envsetup ${mfrs_cfg} ccache jenkins
#
#	make clean
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make clean \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi
#
#	make uninstall
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make uninstall \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi
#
#	#make mall
#	#RESULT=$?
#	#if [ "${RESULT}" != 0 ]; then
#	#	echo -e "\e[0;31m make mall \e[m for ${mfrs_cfg} ERROR !!!"
#	#	exit 1
#	#fi
#
#	make clean
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make clean \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi
#
#	make uninstall
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make uninstall \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi
#done
