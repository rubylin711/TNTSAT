#!/bin/bash

cfg_mt_oss_ssp_all="ssp"
cfg_mt_oss_backtrace_all="bt"
cfg_mt_oss_backtrace_enable="n"
cfg_mt_oss_ssp_enable="n"

unset CFG_MT_OSS_BACKTRACE CFG_MT_OSS_SSP CFG_MT_OSS_SANITIZE CFG_MT_OSS_SANITIZE_LD CFG_MT_OSS_SANITIZE_DIR CFG_MT_OSS_BASE_CFLAGS CFG_MT_OSS_BASE_LDFLAGS CFG_MT_OSS_ARCH_CFLAGS CFG_MT_OSS_ARCH_LDFLAGS



while [ $# -ge 1 -a "${MATCH}" != 0 ]; do
	for x in ${cfg_mt_oss_backtrace_all}; do
		if [ "$1" = "${x}" ]; then
			cfg_mt_oss_backtrace_enable="y"
			echo "bt"
		fi
	done
	for x in ${cfg_mt_oss_ssp_all}; do
		if [ "$1" = "${x}" ]; then
			cfg_mt_oss_ssp_enable="y"
			echo "ssp"
		fi
	done

	shift
done



if [ "${cfg_mt_oss_backtrace_enable}" = "y" ]; then
	CFG_MT_OSS_BACKTRACE_BASE="-rdynamic -funwind-tables -fno-omit-frame-pointer -fno-optimize-sibling-calls"

	if [ -n "`grep "CONFIG_MT_ARCH_ARM=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
		if [ -n "`grep "CONFIG_MT_ARM_MODE=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
			CFG_MT_OSS_BACKTRACE="${CFG_MT_OSS_BACKTRACE_BASE} -mapcs-frame"
		else
			CFG_MT_OSS_BACKTRACE="${CFG_MT_OSS_BACKTRACE_BASE} -mtpcs-frame -mtpcs-leaf-frame"
		fi
	elif [ -n "`grep "CONFIG_MT_ARCH_AARCH64=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
		CFG_MT_OSS_BACKTRACE="${CFG_MT_OSS_BACKTRACE_BASE} -mno-omit-leaf-frame-pointer"
	else
		CFG_MT_OSS_BACKTRACE="${CFG_MT_OSS_BACKTRACE_BASE}"
	fi

	unset CFG_MT_OSS_BACKTRACE_BASE
fi
export CFG_MT_OSS_BACKTRACE



if [ ${cfg_mt_oss_ssp_enable} = "y" ]; then
	CFG_MT_OSS_SSP="-fstack-protector-strong"
	export CFG_MT_OSS_SSP
fi



if [ -n "`grep "CONFIG_MT_SANITIZE=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_OSS_SANITIZE="-D_FORTIFY_SOURCE=0 -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize=alignment"
	if [ -n "`grep "CONFIG_MT_ARCH_ARM=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
		ASAN_PREINIT_O="/usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/libasan_preinit.o"
	elif [ -n "`grep "CONFIG_MT_ARCH_AARCH64=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
		ASAN_PREINIT_O="/usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libasan_preinit.o"
	else
		ASAN_PREINIT_O="/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/lib/libasan_preinit.o"
	fi
	CFG_MT_OSS_SANITIZE_LD="-lasan -lubsan ${ASAN_PREINIT_O}"
	CFG_MT_OSS_SANITIZE_DIR="sanitize/generic/asan"
elif [ -n "`grep "CONFIG_MT_SANITIZE_TAG=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_OSS_SANITIZE="-D_FORTIFY_SOURCE=0 -fsanitize=hwaddress -fsanitize-address-use-after-scope -fno-sanitize=alignment"
	CFG_MT_OSS_SANITIZE_LD="-lhwasan"
	CFG_MT_OSS_SANITIZE_DIR="sanitize/tagged/asan"
elif [ -n "`grep "CONFIG_MT_SANITIZE_THREAD_RACE=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_OSS_SANITIZE="-D_FORTIFY_SOURCE=0 -fsanitize=thread"
	CFG_MT_OSS_SANITIZE_LD="-ltsan /usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libtsan_preinit.o"
	CFG_MT_OSS_SANITIZE_DIR="sanitize/generic/tsan"
else
	echo "not enable any sanitize option"
fi
export CFG_MT_OSS_SANITIZE
export CFG_MT_OSS_SANITIZE_LD
export CFG_MT_OSS_SANITIZE_DIR



if [ -n "`grep "CONFIG_MT_SANITIZE_NONE=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_OSS_BASE_CFLAGS="-Wl,--build-id -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wl,-z,now -Wl,-z,relro -ftree-vectorize"
else
	CFG_MT_OSS_BASE_CFLAGS="-Wl,--build-id -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -D_FORTIFY_SOURCE=0 -Wl,-z,now -Wl,-z,relro -fno-tree-vectorize"
fi
CFG_MT_OSS_BASE_LDFLAGS="--build-id -z,relro -z,now"
export CFG_MT_OSS_BASE_CFLAGS
export CFG_MT_OSS_BASE_LDFLAGS



TARGET_VENDOR="buildroot"
if [ -n "`grep "CONFIG_MT_ARCH_ARM=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_FIXED_SYSROOT_USR_LIB_PATH="--sysroot=${SDK_DIR}/buildroot/output/host/arm-${TARGET_VENDOR}-linux-gnueabihf/sysroot"
	CFG_MT_OSS_ARCH_CFLAGS_BASE="-march=armv7-a -mtune=cortex-a7 -mabi=aapcs-linux -mfloat-abi=hard -mfpu=neon-vfpv4 -mthumb-interwork ${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
	if [ -n "`grep "CONFIG_MT_ARM_MODE=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
		CFG_MT_OSS_ARCH_CFLAGS="${CFG_MT_OSS_ARCH_CFLAGS_BASE} -marm"
	else
		CFG_MT_OSS_ARCH_CFLAGS="${CFG_MT_OSS_ARCH_CFLAGS_BASE} -mthumb"
	fi
	CFG_MT_OSS_ARCH_LDFLAGS="${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
	unset CFG_MT_OSS_ARCH_CFLAGS_BASE
elif [ -n "`grep "CONFIG_MT_ARCH_AARCH64=y" ${SDK_DIR}/product/configs/${MFRS_CFG}`" ]; then
	CFG_MT_FIXED_SYSROOT_USR_LIB_PATH="--sysroot=${SDK_DIR}/buildroot/output/host/aarch64-${TARGET_VENDOR}-linux-gnu/sysroot"
	CFG_MT_OSS_ARCH_CFLAGS="-march=armv8-a -mtune=cortex-a53 -mabi=lp64 -mfix-cortex-a53-835769 -mfix-cortex-a53-843419 -Wl,-Ttext-segment=0x100400000 ${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
	CFG_MT_OSS_ARCH_LDFLAGS="-Ttext-segment=0x100400000 ${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
else
	CFG_MT_FIXED_SYSROOT_USR_LIB_PATH="--sysroot=${SDK_DIR}/buildroot/output/host/mipsel-${TARGET_VENDOR}-linux-gnu/sysroot"
	CFG_MT_OSS_ARCH_CFLAGS="-EL -msoft-float ${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
	CFG_MT_OSS_ARCH_LDFLAGS="-EL ${CFG_MT_FIXED_SYSROOT_USR_LIB_PATH}"
fi
unset CFG_MT_FIXED_SYSROOT_USR_LIB_PATH
export CFG_MT_OSS_ARCH_CFLAGS
export CFG_MT_OSS_ARCH_LDFLAGS



echo "export CFG_MT_OSS_SSP=\"${CFG_MT_OSS_SSP}\"" | tee .env
echo "export CFG_MT_OSS_BACKTRACE=\"${CFG_MT_OSS_BACKTRACE}\"" | tee -a .env
echo "export CFG_MT_OSS_SANITIZE=\"${CFG_MT_OSS_SANITIZE}\"" | tee -a .env
echo "export CFG_MT_OSS_SANITIZE_LD=\"${CFG_MT_OSS_SANITIZE_LD}\"" | tee -a .env
echo "export CFG_MT_OSS_SANITIZE_DIR=\"${CFG_MT_OSS_SANITIZE_DIR}\"" | tee -a .env
echo "export CFG_MT_OSS_BASE_CFLAGS=\"${CFG_MT_OSS_BASE_CFLAGS}\"" | tee -a .env
echo "export CFG_MT_OSS_BASE_LDFLAGS=\"${CFG_MT_OSS_BASE_LDFLAGS}\"" | tee -a .env
echo "export CFG_MT_OSS_ARCH_CFLAGS=\"${CFG_MT_OSS_ARCH_CFLAGS}\"" | tee -a .env
echo "export CFG_MT_OSS_ARCH_LDFLAGS=\"${CFG_MT_OSS_ARCH_LDFLAGS}\"" | tee -a .env
