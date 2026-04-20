#!/bin/bash

mfrs_cfg_all="symphony1.cfg"

for mfrs_cfg in ${mfrs_cfg_all}; do

	git clean -dxf
	git reset --hard

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

	make
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	make -C loader
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make -C loader \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

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

	if [ -n "`git status --ignored | grep 'Untracked files'`" ]; then
		echo "found garbage: Untracked files"
		git status --ignored
		exit 1
	fi

	if [ -n "`git status --ignored | grep 'Ignored files'`" ]; then
		echo "found garbage: Ignored files"
		git status --ignored
		exit 1
	fi

	if [ -n "`git status --ignored | grep 'Changes'`" ]; then
		echo "some file be modified"
		git status --ignored
		exit 1
	fi
done
