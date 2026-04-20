#!/bin/bash

./build/script/release/compile-check-config.sh
RESULT=$?
if [ "${RESULT}" != 0 ]; then
	echo -e "\e[0;31m check config \e[m ERROR !!!"
	exit 1
fi

#mfrs_cfg_all="symphony6.cfg symphony6_tee.cfg symphony6_tee_gst.cfg"
#mfrs_cfg_all="symphony6_gst.cfg symphony6_tee_gst.cfg"
mfrs_cfg_all="symphony6_tee_gst.cfg"
cfg_path="product/configs"

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

	make
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

#	make -C loader
#	RESULT=$?
#	if [ "${RESULT}" != 0 ]; then
#		echo -e "\e[0;31m make -C loader \e[m for ${mfrs_cfg} ERROR !!!"
#		exit 1
#	fi

	echo "compile msp as ko"
	MT_KERNEL_ARCH="`grep CONFIG_MT_KERNEL_ARCH_ ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $1}'`"
	if [ "${MT_KERNEL_ARCH}" = "CONFIG_MT_KERNEL_ARCH_AARCH64" ]; then
		kernel_cfg_path1="kernel/linux-x.y.z/arch/arm64/configs"
	elif [ "${MT_KERNEL_ARCH}" = "CONFIG_MT_KERNEL_ARCH_ARM" ]; then
		kernel_cfg_path1="kernel/linux-x.y.z/arch/arm/configs"
	else
		kernel_cfg_path1="kernel/linux-x.y.z/arch/mips/configs"
	fi
	kernel_cfg_path2="product/configs/kernel_configs"
	kernel_cfg_name="`grep CONFIG_MT_KERNEL_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`"
	if [ -f "${kernel_cfg_path1}/${kernel_cfg_name}" ]; then
		kernel_cfg_path="${kernel_cfg_path1}"
	else
		kernel_cfg_path="${kernel_cfg_path2}"
	fi
	echo "kernel config: ${kernel_cfg_path}/${kernel_cfg_name}"
	cp -af ${kernel_cfg_path}/${kernel_cfg_name} ${kernel_cfg_path}/${kernel_cfg_name}-bak
	sed -i 's|CONFIG_MT_MSP=y|CONFIG_MT_MSP=m|g' ${kernel_cfg_path}/${kernel_cfg_name}
	make kernel
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m make msp as ko \e[m for ${mfrs_cfg} ERROR !!!"
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

	mv ${kernel_cfg_path}/${kernel_cfg_name}-bak ${kernel_cfg_path}/${kernel_cfg_name}

	if [ -n "`repo forall -c "git status --ignored" | grep 'Untracked files'`" ]; then
		repo forall -c "git status --ignored"
		echo ""
		echo "${mfrs_cfg} found garbage: Untracked files"
		exit 1
	fi

	if [ -n "`repo forall -c "git status --ignored" | grep 'Ignored files'`" ]; then
		repo forall -c "git status --ignored"
		echo ""
		echo "${mfrs_cfg} found garbage: Ignored files"
		exit 1
	fi

	if [ -n "`repo forall -c "git status --ignored" | grep 'Changes'`" ]; then
		repo forall -c "git status --ignored"
		echo ""
		echo "${mfrs_cfg} some file be modified"
		exit 1
	fi

	if [ -d "pub" ]; then
		tree pub
		echo ""
		echo "${mfrs_cfg} pub not clean completely"
		exit 1
	fi

	if [ -d "out" ]; then
		tree out
		echo ""
		echo "${mfrs_cfg} out not clean completely"
		exit 1
	fi

	if [ -d "image" ]; then
		tree image
		echo ""
		echo "${mfrs_cfg} image not clean completely"
		exit 1
	fi
done
