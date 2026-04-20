#!/bin/bash
set -x

if [ "${mt_chip}" = "symphony1" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony1_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony1.cfg
	else
		mfrs_cfg=symphony1_512.cfg
	fi
elif [ "${mt_chip}" = "symphony2" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony2_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony2.cfg
	else
		mfrs_cfg=symphony2_512.cfg
	fi
elif [ "${mt_chip}" = "symphony4" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony4_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony4.cfg
	else
		mfrs_cfg=symphony4_512.cfg
	fi
elif [ "${mt_chip}" = "symphony6" ]; then
	if [ "${size}" = "256" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "512" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "1024" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "2048" ]; then
		mfrs_cfg=symphony6.cfg
	else
		mfrs_cfg=symphony6.cfg
	fi
fi
source envsetup ${mfrs_cfg} ccache

make -C loader clean
make -C loader uninstall
make clean
make uninstall

#make j=1
make
RESULT=$?
if [ "${RESULT}" = "0" ]; then
	#make -C loader j=1
	make -C loader
	RESULT=$?
	if [ "${RESULT}" = "0" ]; then
		make -C loader clean
		make -C loader uninstall
		make clean
		make uninstall
		exit 0
	else
		exit 1
	fi
else
	exit 1
fi
