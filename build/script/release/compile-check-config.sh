#!/bin/bash



source envsetup
KERNEL_OUTPUT="${SDK_DIR}/out/general/kernel/linux-x.y.z"
user_cfg_all="`ls ${SDK_DIR}/product/configs | grep ".*\.cfg"`"
echo "${user_cfg_all}"
for user_cfg in ${user_cfg_all}; do
	echo "check ${user_cfg}"
	source envsetup ${user_cfg}
	make syncconfig
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m ${user_cfg} \e[m not mach Kconfig !!!"
		echo "please run make menuconfig_all"
		exit 1
	fi
	diff out/general/menuconfig/.config ${SDK_DIR}/product/configs/${user_cfg}
	RESULT=$?
	if [ "${RESULT}" != 0 ]; then
		echo -e "\e[0;31m ${user_cfg} \e[m not same with .config !!!"
		if [ ${MT_UPDATE_CONFIG} = "true" ]; then
			cp -af out/general/menuconfig/.config ${SDK_DIR}/product/configs/${user_cfg}
		else
			exit 1
		fi
	fi
done



cd ${SDK_DIR}
source envsetup symphony6.cfg
make syncconfig
make buildroot_step0
make create_config_header
make initramfs
make kernel
RESULT=$?
if [ "${RESULT}" != 0 ]; then
	echo -e "\e[0;31m symphony6.cfg \e[m make kernel failed !!!"
	exit 1
fi

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
			echo "not support \033[31m ${kernel_cfg} \033[0m, only support CONFIG_MIPS CONFIG_ARM CONFIG_ARM64 now"
			continue
		fi

		echo "check ${kernel_cfg}"
		source envsetup ${user_cfg}
		cp -af ${kernel_cfg} ${KERNEL_OUTPUT}/.config;
		cd ${KERNEL_OUTPUT}
		make -f ${SDK_DIR}/build/script/release/compile-check-config_helper.mk kernel_config_check;
		RESULT=$?
		if [ "${RESULT}" != 0 ]; then
			echo -e "\e[0;31m ${kernel_cfg} \e[m not mach Kconfig !!!"
			echo "please run make kernel_menuconfig_all"
			exit 1
		fi
		cd ${SDK_DIR}
		diff ${KERNEL_OUTPUT}/.config ${kernel_cfg}
		RESULT=$?
		if [ "${RESULT}" != 0 ]; then
			echo -e "\e[0;31m ${kernel_cfg} \e[m not same with .config !!!"
			if [ ${MT_UPDATE_CONFIG} = "true" ]; then
				cp -af ${KERNEL_OUTPUT}/.config ${kernel_cfg}
			else
				exit 1
			fi
		fi
	fi
done

source envsetup symphony6.cfg
make clean
make uninstall

exit 0
