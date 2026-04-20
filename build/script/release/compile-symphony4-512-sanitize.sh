#!/bin/bash

mfrs_cfg_all=$1

#generic, tagged
mode=$2
#asan, tsan
san_type=$3
#user, kernel
address_space=$4
#config, compile
build=$5

cfg_path="product/configs"

for mfrs_cfg in ${mfrs_cfg_all}; do

	if [ "${mode}" != "generic" ]; then
		echo -e "\e[0;31m arm32 only support generic mode \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	if [ "${san_type}" != "asan" ]; then
		echo -e "\e[0;31m arm32 only support asan type \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	if [ "${address_space}" = "user" ]; then
		sed -i 's|# CONFIG_MT_SANITIZE is not set|CONFIG_MT_SANITIZE=y|g' ${cfg_path}/${mfrs_cfg}
		sed -i 's|CONFIG_MT_SANITIZE_NONE=y|# CONFIG_MT_SANITIZE_NONE is not set|g' ${cfg_path}/${mfrs_cfg}
		sed -i 's|# CONFIG_MT_DEBUG is not set|CONFIG_MT_DEBUG=y|g' ${cfg_path}/${mfrs_cfg}
		sed -i 's|# CONFIG_MT_BACKTRACE is not set|CONFIG_MT_BACKTRACE=y|g' ${cfg_path}/${mfrs_cfg}
		sed -i 's|# CONFIG_MT_INSTALL_GDBSERVER is not set|CONFIG_MT_INSTALL_GDBSERVER=y|g' ${cfg_path}/${mfrs_cfg}
		sed -i '/BR2_TARGET_OPTIMIZATION/ s|"$| -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize=alignment"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
		sed -i '/BR2_MT_TARGET_LDFLAGS/ s|"$| -lasan -lubsan /usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/libasan_preinit.o"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
		sed -i 's|# BR2_FORTIFY_SOURCE_NONE is not set|BR2_FORTIFY_SOURCE_NONE=y|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
		sed -i 's|BR2_FORTIFY_SOURCE_1=y|# BR2_FORTIFY_SOURCE_1 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
		sed -i 's|BR2_FORTIFY_SOURCE_2=y|# BR2_FORTIFY_SOURCE_2 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	elif [ "${address_space}" = "kernel" ]; then
		kernel_cfg_path="kernel/linux-x.y.z/arch/arm/configs"
		kernel_cfg_name="`grep CONFIG_MT_KERNEL_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`"
		sed -i 's|# CONFIG_SLUB_DEBUG is not set|CONFIG_SLUB_DEBUG=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
		sed -i 's|CONFIG_FRAME_WARN=2048|CONFIG_FRAME_WARN=4096|g' ${kernel_cfg_path}/${kernel_cfg_name}
		sed -i 's|# CONFIG_KASAN is not set|CONFIG_KASAN=y\nCONFIG_KASAN_GENERIC=y\nCONFIG_KASAN_OUTLINE=y\n# CONFIG_KASAN_INLINE is not set\nCONFIG_KASAN_STACK=y\nCONFIG_KASAN_MODULE_TEST=m|g' ${kernel_cfg_path}/${kernel_cfg_name}
	else
		echo -e "\e[0;31m address_space only support user or kernel \e[m for ${mfrs_cfg} ERROR !!!"
		exit 1
	fi

	if [ "${build}" = "config" ]; then
		echo -e "\e[0;32m config success \e[m for ${mfrs_cfg} ${mode} ${san_type} ${address_space} ${build}"
		exit 0
	fi

	source envsetup ${mfrs_cfg}

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
