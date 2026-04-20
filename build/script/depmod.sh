#!/bin/bash

#move files, we want to place msp and common ko into usrfs
#wifi and bt keep in updates
mkdir -p ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_common.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_msp.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_vofw.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_vfmw.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_slhdr.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "mt_ciplus.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
mv -f `find ${MODULE_DIR_STRIPED}/lib/modules/ -name "smit.ko"` ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates

#depmod
echo "depmod -ae -b ${MODULE_DIR_STRIPED} -F ${KERNEL_OUTPUT}/System.map ${CFG_MT_KERNEL_VERSION}"
depmod -ae -b ${MODULE_DIR_STRIPED} -F ${KERNEL_OUTPUT}/System.map ${CFG_MT_KERNEL_VERSION}

#prepare for usrfs
mkdir -p ${MODULE_DIR_STRIPED}/external-ko
cp -arf ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates/* ${MODULE_DIR_STRIPED}/external-ko

#ln
cd ${MODULE_DIR_STRIPED}/lib/modules/${CFG_MT_KERNEL_VERSION}/updates
all_files=`find -type f | sed 's|^\./||g'`
echo ${all_files}
for x in ${all_files}
do
	rm -f ${x}
	ln -sf /usr/local/stb/external-ko/${x} ${x}
done
