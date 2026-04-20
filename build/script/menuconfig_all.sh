#!/bin/bash

all_files=`ls ${SDK_DIR}/product/configs | grep ".*\.cfg"`;
for x in $all_files ;
do
	source envsetup $x;
	make menuconfig;
done
