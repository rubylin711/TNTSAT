#!/bin/bash

#ln
if [ -d "${MODULE_DIR_STRIPED}/lib/firmware" ]; then
	cp -arf ${MODULE_DIR_STRIPED}/lib/firmware ${MODULE_DIR_STRIPED}
	#keep some fw bin stay in /lib/firmware of rootfs
	#if you hope your fw in usrfs, no need do any thing here
	#if you hope your fw in rootfs, please add your fw into KEEP_STAY_FW
	KEEP_STAY_FW="regulatory.db regulatory.db.p7s"
	cd ${MODULE_DIR_STRIPED}/lib/firmware
	all_files=`find -type f | sed 's|^\./||g'`
	echo ""
	echo ${all_files}
	for x in ${all_files}
	do
	        stay=0
	        for y in ${KEEP_STAY_FW}
	        do
	                if [ "${x}" = "${y}" ]; then
	                        stay=1
	                fi
	        done
	        if [ "${stay}" = "0" ]; then
	                rm -f ${x}
	                ln -sf /usr/local/stb/fw/${x} ${x}
	        fi
	done
fi
