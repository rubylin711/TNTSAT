#!/bin/bash

kernel_cfg_all=`ls product/configs/kernel_configs/*_defconfig kernel/linux-x.y.z/arch/arm/configs/*_defconfig kernel/linux-x.y.z/arch/arm64/configs/*_defconfig kernel/linux-x.y.z/arch/mips/configs/*_defconfig`;
for kernel_cfg in $kernel_cfg_all;
do
	cd ${SDK_DIR}
	if [ -n "`grep -w "CONFIG_MT_CHIP_COMMON=y" ${kernel_cfg}`" ]; then
		if [ -n "`grep -w "CONFIG_MIPS=y" ${kernel_cfg}`" ]; then
			user_cfg="symphony1.cfg"
		elif [ -n "`grep -w "CONFIG_ARM=y" ${kernel_cfg}`" ]; then
			user_cfg="symphony6_32user_32kernel.cfg"
		elif [ -n "`grep -w "CONFIG_ARM64=y" ${kernel_cfg}`" ]; then
			user_cfg="symphony6.cfg"
		else
			echo -e "not support \033[31m ${kernel_cfg} \033[0m, only support CONFIG_MIPS CONFIG_ARM CONFIG_ARM64 now";
			exit 1
		fi

		echo -e "use \033[32m ${kernel_cfg} \033[0m";
		source envsetup ${user_cfg}
		cp -af ${kernel_cfg} ${KERNEL_OUTPUT}/.config;
		cd ${KERNEL_OUTPUT}
		make -f ${SDK_DIR}/build/script/kernel_menuconfig_all_helper.mk;
		cd ${SDK_DIR}
		cp -af ${KERNEL_OUTPUT}/.config ${kernel_cfg}
	fi
done

exit 0
