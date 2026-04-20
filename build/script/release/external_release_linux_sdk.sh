#!/bin/bash

################################################################################
#
# Usage:
#	external_release_linux_sdk.sh
#	参数：	可以不设任何参数，或者是从下面几组参数中任选一个或多个或不选
#		symphony1 symphony2 symphony4 symphony6
#		128 256 512
#		tee_nor_gst nor_gst tee_gst tee_nor tee nor gst default
#		loader
#
#	symphony1 symphony2一般只需要128 256 512 nor default loader
#	symphony4一般只需要128 256 512 tee_nor_gst nor_gst tee_gst tee_nor tee nor gst default loader
#	symphony6一般只需要tee_nor_gst nor_gst tee_gst tee_nor tee nor gst default 32user 32user_32kernel 64user loader
#
################################################################################

# add for local evaluation test
evaluation="0"

local_test="0"

if [ "${evaluation}" = "0" ]; then
	repo forall -c "git checkout ."
	rm -rf *; repo sync -l
	if [ "${local_test}" = "1" ]; then
		repo checkout master
	fi
fi


CHIP_ALL="symphony1 symphony2 symphony4 symphony6"
SIZE_ALL="128 256 512"
TYPE_ALL="tee_nor_gst nor_gst tee_gst tee_nor tee nor gst default"
BIT_ALL="32user 32user_32kernel 64user"
LOADER_ALL="loader"



MATCH="1"
MATCH_CHIP="0"
MATCH_SIZE="0"
MATCH_TYPE="0"
MATCH_BIT="0"
MATCH_LOADER="0"
mt_chip_all=""
size_all=""
type_all=""
bit_all=""
while [ $# -ge "1" -a "${MATCH}" != "0" ]; do
	MATCH="0"
	for x in ${CHIP_ALL}; do
		if [ "$1" = "${x}" ]; then
			MATCH="1"
			MATCH_CHIP="1"
			mt_chip_all="${mt_chip_all} $1"
		fi
	done
	for x in ${SIZE_ALL}; do
		if [ "$1" = "${x}" ]; then
			MATCH="2"
			MATCH_SIZE="1"
			size_all="${size_all} $1"
		fi
	done
	for x in ${TYPE_ALL}; do
		if [ "$1" = "${x}" ]; then
			MATCH="3"
			MATCH_TYPE="1"
			type_all="${type_all} $1"
		fi
	done
	for x in ${BIT_ALL}; do
		if [ "$1" = "${x}" ]; then
			MATCH="4"
			MATCH_BIT="1"
			bit_all="${bit_all} $1"
		fi
	done
	for x in ${LOADER_ALL}; do
		if [ "$1" = "${x}" ]; then
			MATCH="5"
			MATCH_LOADER="1"
			build_loader="yes"
		fi
	done

	if [ "${MATCH}" != 0 ]; then
		shift
	else
		echo -e "\033[31m parameter error! only support these: \033[0m"
		echo -e "\033[32m ${CHIP_ALL} \033[0m"
		echo -e "\033[32m ${SIZE_ALL} \033[0m"
		echo -e "\033[32m ${TYPE_ALL} \033[0m"
		echo -e "\033[32m ${BIT_ALL} \033[0m"
		echo -e "\033[32m ${LOADER_ALL} \033[0m"
		echo -e "not support: \033[31m $1 \033[0m"
		exit 1
	fi
done

if [ ${MATCH_CHIP} = "0" ]; then
	mt_chip_all="${CHIP_ALL}"
fi
if [ ${MATCH_SIZE} = "0" ]; then
	size_all="${SIZE_ALL}"
fi
if [ ${MATCH_TYPE} = "0" ]; then
	type_all="${TYPE_ALL}"
fi
if [ ${MATCH_BIT} = "0" ]; then
	bit_all="${BIT_ALL}"
fi
if [ ${MATCH_LOADER} = "0" ]; then
	build_loader="no"
fi

echo "Release Chip: ${mt_chip_all}"
echo "Release Size: ${size_all}"
echo "Release Type: ${type_all}"
echo "Release Bit: ${bit_all}"
echo "Release Loader: ${build_loader}"



for mt_chip in ${mt_chip_all}; do

	mfrs_cfg_all=""
	if [ "${mt_chip}" = "symphony1" -o "${mt_chip}" = "symphony2" ]; then
		for size in ${size_all}; do
			for typee in ${type_all}; do
				if [ "${typee}" = "default" -a "${size}" = "256" ]; then
					#symphony1.cfg symphony2.cfg
					mfrs_cfg="${mt_chip}.cfg"
				elif [ "${typee}" = "default" ]; then
					#symphony1_128.cfg symphony2_128.cfg symphony1_512.cfg symphony2_512.cfg
					mfrs_cfg="${mt_chip}_${size}.cfg"
				elif [ "${size}" = "256" ]; then
					#symphony1_nor.cfg symphony2_nor.cfg
					mfrs_cfg="${mt_chip}_${typee}.cfg"
				else
					#symphony1_128_nor.cfg symphony2_128_nor.cfg symphony1_512_nor.cfg symphony2_512_nor.cfg
					mfrs_cfg="${mt_chip}_${size}_${typee}.cfg"
				fi
				mfrs_cfg_all="${mfrs_cfg_all} ${mfrs_cfg}"
			done
		done
	elif [ "${mt_chip}" = "symphony4" ]; then
		for size in ${size_all}; do
			for typee in ${type_all}; do
				if [ "${typee}" = "default" -a "${size}" = "256" ]; then
					#symphony4.cfg
					mfrs_cfg="${mt_chip}.cfg"
				elif [ "${typee}" = "default" ]; then
					#symphony4_128.cfg symphony4_512.cfg
					mfrs_cfg="${mt_chip}_${size}.cfg"
				elif [ "${size}" = "256" ]; then
					#symphony4_tee_nor_gst.cfg symphony4_nor_gst.cfg symphony4_tee_gst.cfg symphony4_tee_nor.cfg symphony4_tee.cfg symphony4_nor.cfg symphony4_gst.cfg
					mfrs_cfg="${mt_chip}_${typee}.cfg"
				else
					#symphony4_128_tee_nor_gst.cfg symphony4_128_nor_gst.cfg symphony4_128_tee_gst.cfg symphony4_128_tee_nor.cfg symphony4_128_tee.cfg symphony4_128_nor.cfg symphony4_128_gst.cfg
					#symphony4_512_tee_nor_gst.cfg symphony4_512_nor_gst.cfg symphony4_512_tee_gst.cfg symphony4_512_tee_nor.cfg symphony4_512_tee.cfg symphony4_512_nor.cfg symphony4_512_gst.cfg
					mfrs_cfg="${mt_chip}_${size}_${typee}.cfg"
				fi
				mfrs_cfg_all="${mfrs_cfg_all} ${mfrs_cfg}"
			done
		done
	elif [ "${mt_chip}" = "symphony6" ]; then
		for bit in ${bit_all}; do
			for typee in ${type_all}; do
				if [ "${typee}" = "default" ]; then
					if [ "${bit}" = "64user" ]; then
						#symphony6.cfg
						mfrs_cfg="${mt_chip}.cfg"
					else
						#symphony6_32user.cfg symphony6_32user_32kernel.cfg
						mfrs_cfg="${mt_chip}_${bit}.cfg"
					fi
				else
					if [ "${bit}" = "64user" ]; then
						#symphony6_tee_nor_gst.cfg symphony6_nor_gst.cfg symphony6_tee_gst.cfg symphony6_tee_nor.cfg symphony6_tee.cfg symphony6_nor.cfg symphony6_gst.cfg
						mfrs_cfg="${mt_chip}_${typee}.cfg"
					else
						#symphony6_32user_tee_nor_gst.cfg symphony6_32user_nor_gst.cfg symphony6_32user_tee_gst.cfg symphony6_32user_tee_nor.cfg symphony6_32user_tee.cfg symphony6_32user_nor.cfg symphony6_32user_gst.cfg
						#symphony6_32user_32kernel_tee_nor_gst.cfg symphony6_32user_32kernel_nor_gst.cfg symphony6_32user_32kernel_tee_gst.cfg symphony6_32user_32kernel_tee_nor.cfg symphony6_32user_32kernel_tee.cfg symphony6_32user_32kernel_nor.cfg symphony6_32user_32kernel_gst.cfg
						mfrs_cfg="${mt_chip}_${bit}_${typee}.cfg"
					fi
				fi
				mfrs_cfg_all="${mfrs_cfg_all} ${mfrs_cfg}"
			done
		done
	fi
	echo mfrs_cfg_all="${mfrs_cfg_all}"

	mfrs_cfg_exist_all=""
	for mfrs_cfg in ${mfrs_cfg_all}; do
		if [ -f "product/configs/${mfrs_cfg}" ]; then
			mfrs_cfg_exist_all="${mfrs_cfg_exist_all} ${mfrs_cfg}"
		fi
	done
	echo mfrs_cfg_exist_all="${mfrs_cfg_exist_all}"


	for mfrs_cfg in ${mfrs_cfg_exist_all}; do

		source envsetup ${mfrs_cfg} ccache jenkins

		sed -i 's|CONFIG_REGULATOR_SYMPHONY4=y|CONFIG_REGULATOR_SYMPHONY4=m|g' kernel/linux-x.y.z/arch/arm/configs/*_defconfig kernel/linux-x.y.z/arch/mips/configs/*_defconfig kernel/linux-x.y.z/arch/arm64/configs/*_defconfig product/configs/kernel_configs/*_defconfig

		make uninstall
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make uninstall failed for ${mfrs_cfg}!"
			exit 1
		fi

		make clean
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make clean failed for ${mfrs_cfg}!"
			exit 1
		fi

		make -C loader uninstall
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make -C loader uninstall failed for ${mfrs_cfg}!"
			exit 1
		fi

		make -C loader clean
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make -C loader clean failed for ${mfrs_cfg}!"
			exit 1
		fi

		make
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make failed for ${mfrs_cfg}!"
			exit 1
		fi

		make install DOING_MT_SDK_RELEASE=1
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make install DOING_MT_SDK_RELEASE=1 failed for ${mfrs_cfg}!"
			exit 1
		fi

#		make -C loader
#		RESULT=$?
#		if [ "${RESULT}" != "0" ]; then
#			echo "make -C loader failed for ${mfrs_cfg}!"
#			exit 1
#		fi
#
#		make -C loader install DOING_MT_SDK_RELEASE=1
#		RESULT=$?
#		if [ "${RESULT}" != "0" ]; then
#			echo "make -C loader install DOING_MT_SDK_RELEASE=1 failed for ${mfrs_cfg}!"
#			exit 1
#		fi

		make clean
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make clean failed for ${mfrs_cfg}!"
			exit 1
		fi

		make -C loader clean
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make -C loader clean failed for ${mfrs_cfg}!"
			exit 1
		fi

		make uninstall
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make uninstall failed for ${mfrs_cfg}!"
			exit 1
		fi

		make -C loader uninstall
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make -C loader uninstall failed for ${mfrs_cfg}!"
			exit 1
		fi

		sed -i 's|CFG_MT_SDK_RELEASE=n|CFG_MT_SDK_RELEASE=y|' build/script/env.mk

		make uninstall DOING_MT_SDK_RELEASE=1
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make uninstall DOING_MT_SDK_RELEASE=1 failed for ${mfrs_cfg}!"
			exit 1
		fi

		make -C loader uninstall DOING_MT_SDK_RELEASE=1
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "make -C loader uninstall DOING_MT_SDK_RELEASE=1 failed for ${mfrs_cfg}!"
			exit 1
		fi

		sed -i 's|CFG_MT_SDK_RELEASE=y|CFG_MT_SDK_RELEASE=n|' build/script/env.mk
	done
done



mkdir -p bak
cp -arf --parents msp/drv/vfmw/video_lib_1.0/vfmw.h msp/drv/vo_fw/vo_fw.h msp/drv/vo_fw/inc bak
cp -arf --parents docs/ForRelease bak/
rm -rf sample/gpu_mali sample/gpu_mali_11 sample/gpu_mali_openvg sample/cdmi
rm -rf chip-verification msp/drv/chip_verification
sed -i '/path=\"ddk\/chip-verification\"/'d ../.repo/manifests/*.xml
rm -rf msp/api/gpu msp/api/jpge msp/api/venc msp/api/vpu_enc
rm -rf msp/drv/gpu msp/drv/jpge msp/drv/venc msp/drv/vpu_enc msp/drv/vfmw msp/drv/vo_fw
rm -rf component/fs
sed -i '/path=\"ddk\/component\/fs\"/'d ../.repo/manifests/*.xml
rm -rf kware/netapp
rm -rf product/alaska_rearch product/dongle product/intek product/se_alaska product/suresoft
rm -rf product/alaska product/alaska_128 product/boston product/boston_128 product/boston_tee
rm -rf product/chicago
rm -rf product/lotus product/lotus_sym4
rm -f product/configs/*symphony1*.cfg product/configs/*symphony2*.cfg product/configs/*symphony4*.cfg
rm -f product/configs/symphony6.cfg product/configs/symphony6_32user.cfg product/configs/symphony6_32user_32kernel.cfg product/configs/symphony6_32user_32kernel_gst.cfg product/configs/symphony6_32user_gst.cfg product/configs/symphony6_fpga.cfg product/configs/symphony6_gst.cfg product/configs/symphony6_otpmini.cfg product/configs/symphony6_otpmini_256.cfg product/configs/symphony6_release.cfg product/configs/symphony6_tee_fpga.cfg
rm -f product/configs/kernel_configs/*symphony1* product/configs/kernel_configs/*symphony2* product/configs/kernel_configs/*symphony4*
rm -rf tools/cpu-usage tools/crash tools/ftrace/*.c tools/perf/FlameGraph tools/gperftools tools/memory-usage tools/toolchain/crosstool-ng tools/toolchain/gdb-mt-jtag tools/toolchain/gcc-arm-src-snapshot-8.3-2019.03.tar.xz tools/toolchain/arm-gnu-toolchain-src-snapshot-12.3.rel1.tar.xz tools/toolchain/glibc-arm-src-snapshot-2.28-2019.03.tar.bz2 tools/sanitizer tools/commit_template
rm -rf docs/*
cp -arf bak/* .
rm -rf bak

rm -rf av_rtos
sed -i '/path=\"ddk\/av_rtos\"/'d ../.repo/manifests/*.xml
rm -rf wb
sed -i '/path=\"ddk\/wb\"/'d ../.repo/manifests/*.xml

rm -rf mboot/doc/*.docx mboot/bootinit
sed -i 's|CFG_MT_SDK_RELEASE=n|CFG_MT_SDK_RELEASE=y|g' mboot/Makefile.new

# no release "gstmtlzplayer"
sed -i '/gstmtlzplayer/'d component/media/Makefile

rm -f kernel/linux-x.y.z/drivers/regulator/symphony4-regulator.c
sed -i '/symphony4-regulator/'d kernel/linux-x.y.z/drivers/regulator/Makefile
sed -i '/config REGULATOR_SYMPHONY4/,/Montage LZ Symphony4 SOC/d' kernel/linux-x.y.z/drivers/regulator/Kconfig
sed -i '/CONFIG_REGULATOR_SYMPHONY4/'d kernel/linux-x.y.z/arch/arm/configs/*_defconfig kernel/linux-x.y.z/arch/mips/configs/*_defconfig kernel/linux-x.y.z/arch/arm64/configs/*_defconfig product/configs/kernel_configs/*_defconfig
sed -i 's|CONFIG_RUN_IN_SECURE_WORLD=y|# CONFIG_RUN_IN_SECURE_WORLD is not set|g' kernel/linux-x.y.z/arch/arm/configs/*_defconfig kernel/linux-x.y.z/arch/mips/configs/*_defconfig kernel/linux-x.y.z/arch/arm64/configs/*_defconfig product/configs/kernel_configs/*_defconfig
sed -i 's|CONFIG_MT_CHIP_VERIFICATION=y|# CONFIG_MT_CHIP_VERIFICATION is not set|g' kernel/linux-x.y.z/arch/arm/configs/*_defconfig kernel/linux-x.y.z/arch/mips/configs/*_defconfig kernel/linux-x.y.z/arch/arm64/configs/*_defconfig product/configs/kernel_configs/*_defconfig

rm -rf buildroot/package/mt-av
sed -i 's| mt-av| |g' buildroot/package/mt-*/mt-*.mk
sed -i 's|mt-av | |g' buildroot/package/mt-*/mt-*.mk
sed -i '/mt-av/'d buildroot/package/Config.in
sed -i '/BR2_PACKAGE_MT_AV/'d buildroot/configs/*_defconfig
sed -i '/BR2_PACKAGE_HAS_MT_AV/'d buildroot/configs/*_defconfig
rm -rf buildroot/package/mt-wb
sed -i 's| mt-wb| |g' buildroot/package/mt-*/mt-*.mk
sed -i 's|mt-wb | |g' buildroot/package/mt-*/mt-*.mk
sed -i '/mt-wb/'d buildroot/package/Config.in
sed -i '/BR2_PACKAGE_MT_WB/'d buildroot/configs/*_defconfig
sed -i '/BR2_PACKAGE_HAS_MT_WB/'d buildroot/configs/*_defconfig
sed -i 's|socsw-gitserver.montage-lz.com:29418|socguestgerrit.montage-lz.com:29418|g' buildroot/package/mt-*/mt-*.mk buildroot/package/neon/neon.mk buildroot/configs/*_defconfig
sed -i 's|socsw-gitserver.montage-lz.com:8081|socguestgerrit.montage-lz.com:8081|g' ../.repo/manifests/*.xml

sed -i 's|CFG_MT_SDK_RELEASE=n|CFG_MT_SDK_RELEASE=y|' build/script/env.mk



exit 0
