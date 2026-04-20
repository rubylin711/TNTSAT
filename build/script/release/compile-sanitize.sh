#!/bin/bash

mfrs_cfg=$1

#generic, tagged
mode=$2
#asan, tsan, msan
san_type=$3
#user, kernel, kernel_user
address_space=$4

cfg_path="product/configs"

MT_KERNEL_ARCH="`grep CONFIG_MT_KERNEL_ARCH_ ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $1}'`"
if [ "${MT_KERNEL_ARCH}" = "CONFIG_MT_KERNEL_ARCH_AARCH64" ]; then
	kernel_cfg_path1="kernel/linux-x.y.z/arch/arm64/configs"
	ASAN_PREINIT_O="/usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libasan_preinit.o"
	TSAN_PREINIT_O="/usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/lib64/libtsan_preinit.o"
elif [ "${MT_KERNEL_ARCH}" = "CONFIG_MT_KERNEL_ARCH_ARM" ]; then
	kernel_cfg_path1="kernel/linux-x.y.z/arch/arm/configs"
	ASAN_PREINIT_O="/usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/arm-linux-gnueabihf/lib/libasan_preinit.o"
else
	kernel_cfg_path1="kernel/linux-x.y.z/arch/mips/configs"
	ASAN_PREINIT_O="/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/lib/libasan_preinit.o"
fi
kernel_cfg_path2="product/configs/kernel_configs"
kernel_cfg_name="`grep CONFIG_MT_KERNEL_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`"
if [ -f "${kernel_cfg_path1}/${kernel_cfg_name}" ]; then
	kernel_cfg_path="${kernel_cfg_path1}"
else
	kernel_cfg_path="${kernel_cfg_path2}"
fi
echo "kernel config: ${kernel_cfg_path}/${kernel_cfg_name}"
mfrs="`grep CONFIG_MT_MFRS ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`"
mfrs_board_config="product/${mfrs}/board_config.mk"
mfrs_bootargs_dir="product/${mfrs}/bootargs"

user_generic_asan()
{
	sed -i 's|CONFIG_MT_INSTALL_JEMALLOC=y||g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_SANITIZE is not set|CONFIG_MT_SANITIZE=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|CONFIG_MT_SANITIZE_NONE=y|# CONFIG_MT_SANITIZE_NONE is not set|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_DEBUG is not set|CONFIG_MT_DEBUG=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_BACKTRACE is not set|CONFIG_MT_BACKTRACE=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_INSTALL_GDBSERVER is not set|CONFIG_MT_INSTALL_GDBSERVER=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i '/BR2_TARGET_OPTIMIZATION/ s|"$| -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize=alignment"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|-ftree-vectorize|-fno-tree-vectorize|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i '/BR2_MT_TARGET_LDFLAGS/ s|"$| -lasan -lubsan '${ASAN_PREINIT_O}'"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|# BR2_FORTIFY_SOURCE_NONE is not set|BR2_FORTIFY_SOURCE_NONE=y|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_1=y|# BR2_FORTIFY_SOURCE_1 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_2=y|# BR2_FORTIFY_SOURCE_2 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_LTRACE=y|# BR2_PACKAGE_LTRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_STRACE=y|# BR2_PACKAGE_STRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
}

user_generic_tsan()
{
	sed -i 's|CONFIG_MT_INSTALL_JEMALLOC=y||g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_SANITIZE_THREAD_RACE is not set|CONFIG_MT_SANITIZE_THREAD_RACE=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|CONFIG_MT_SANITIZE_NONE=y|# CONFIG_MT_SANITIZE_NONE is not set|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_DEBUG is not set|CONFIG_MT_DEBUG=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_BACKTRACE is not set|CONFIG_MT_BACKTRACE=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_INSTALL_GDBSERVER is not set|CONFIG_MT_INSTALL_GDBSERVER=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i '/BR2_TARGET_OPTIMIZATION/ s|"$| -fsanitize=thread"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|-ftree-vectorize|-fno-tree-vectorize|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i '/BR2_MT_TARGET_LDFLAGS/ s|"$| -ltsan '${TSAN_PREINIT_O}'"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|# BR2_FORTIFY_SOURCE_NONE is not set|BR2_FORTIFY_SOURCE_NONE=y|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_1=y|# BR2_FORTIFY_SOURCE_1 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_2=y|# BR2_FORTIFY_SOURCE_2 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_LTRACE=y|# BR2_PACKAGE_LTRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_STRACE=y|# BR2_PACKAGE_STRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
}

user_generic_msan()
{
	echo "not support user_generic_msan"
}

user_tagged_asan()
{
	sed -i 's|CONFIG_MT_INSTALL_JEMALLOC=y||g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_SANITIZE_TAG is not set|CONFIG_MT_SANITIZE_TAG=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|CONFIG_MT_SANITIZE_NONE=y|# CONFIG_MT_SANITIZE_NONE is not set|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_DEBUG is not set|CONFIG_MT_DEBUG=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_BACKTRACE is not set|CONFIG_MT_BACKTRACE=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i 's|# CONFIG_MT_INSTALL_GDBSERVER is not set|CONFIG_MT_INSTALL_GDBSERVER=y|g' ${cfg_path}/${mfrs_cfg}
	sed -i '/BR2_TARGET_OPTIMIZATION/ s|"$| -fsanitize=hwaddress -fsanitize-address-use-after-scope -fno-sanitize=alignment --param hwasan-instrument-allocas=0"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|-ftree-vectorize|-fno-tree-vectorize|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i '/BR2_MT_TARGET_LDFLAGS/ s|"$| -lhwasan"|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|# BR2_FORTIFY_SOURCE_NONE is not set|BR2_FORTIFY_SOURCE_NONE=y|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_1=y|# BR2_FORTIFY_SOURCE_1 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_FORTIFY_SOURCE_2=y|# BR2_FORTIFY_SOURCE_2 is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_LTRACE=y|# BR2_PACKAGE_LTRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
	sed -i 's|BR2_PACKAGE_STRACE=y|# BR2_PACKAGE_STRACE is not set|g' buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
}

kernel_generic_asan()
{
	sed -i 's|# CONFIG_SLUB_DEBUG is not set|CONFIG_SLUB_DEBUG=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|CONFIG_FRAME_WARN=.*|CONFIG_FRAME_WARN=4096|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_KASAN is not set|CONFIG_KASAN=y\nCONFIG_KASAN_GENERIC=y\n# CONFIG_KASAN_SW_TAGS is not set\nCONFIG_KASAN_OUTLINE=y\n# CONFIG_KASAN_INLINE is not set\nCONFIG_KASAN_STACK=y\nCONFIG_KASAN_VMALLOC=y\nCONFIG_KASAN_MODULE_TEST=m|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_LOCK_STAT is not set|CONFIG_LOCK_STAT=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_DEBUG_LOCK_ALLOC is not set|CONFIG_DEBUG_LOCK_ALLOC=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
}

kernel_generic_tsan()
{
	echo "not support kernel_generic_tsan"
}

kernel_generic_msan()
{
	echo "not support kernel_generic_msan"
}

kernel_tagged_asan()
{
	sed -i 's|# CONFIG_SLUB_DEBUG is not set|CONFIG_SLUB_DEBUG=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|CONFIG_FRAME_WARN=.*|CONFIG_FRAME_WARN=4096|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_KASAN is not set|CONFIG_KASAN=y\n# CONFIG_KASAN_GENERIC is not set\nCONFIG_KASAN_SW_TAGS=y\nCONFIG_KASAN_OUTLINE=y\n# CONFIG_KASAN_INLINE is not set\nCONFIG_KASAN_STACK=y\nCONFIG_KASAN_VMALLOC=y\nCONFIG_KASAN_MODULE_TEST=m|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_LOCK_STAT is not set|CONFIG_LOCK_STAT=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
	sed -i 's|# CONFIG_DEBUG_LOCK_ALLOC is not set|CONFIG_DEBUG_LOCK_ALLOC=y|g' ${kernel_cfg_path}/${kernel_cfg_name}
}

copy_config()
{
	cp -af out/general/menuconfig/.config ${cfg_path}/${mfrs_cfg}
	cp -af out/general/kernel/linux-x.y.z/.config ${kernel_cfg_path}/${kernel_cfg_name}
	cp -af buildroot/.config buildroot/configs/`grep CONFIG_MT_BUILDROOT_DEFCONFIG ${cfg_path}/${mfrs_cfg} | awk -F'=' '{print $2}' | awk -F'"' '{print $2}'`
}

if [ "${address_space}" = "user" -a "${mode}" = "generic" -a "${san_type}" = "asan" ]; then
	user_generic_asan
elif [ "${address_space}" = "user" -a "${mode}" = "generic" -a "${san_type}" = "tsan" ]; then
	user_generic_tsan
elif [ "${address_space}" = "user" -a "${mode}" = "generic" -a "${san_type}" = "msan" ]; then
	user_generic_msan
elif [ "${address_space}" = "user" -a "${mode}" = "tagged" -a "${san_type}" = "asan" ]; then
	user_tagged_asan
elif [ "${address_space}" = "kernel" -a "${mode}" = "generic" -a "${san_type}" = "asan" ]; then
	kernel_generic_asan
elif [ "${address_space}" = "kernel" -a "${mode}" = "generic" -a "${san_type}" = "tsan" ]; then
	kernel_generic_tsan
elif [ "${address_space}" = "kernel" -a "${mode}" = "generic" -a "${san_type}" = "msan" ]; then
	kernel_generic_msan
elif [ "${address_space}" = "kernel" -a "${mode}" = "tagged" -a "${san_type}" = "asan" ]; then
	kernel_tagged_asan
elif [ "${address_space}" = "kernel_user" -a "${mode}" = "generic" -a "${san_type}" = "asan" ]; then
	kernel_generic_asan
	user_generic_asan
elif [ "${address_space}" = "kernel_user" -a "${mode}" = "generic" -a "${san_type}" = "tsan" ]; then
	kernel_generic_tsan
	user_generic_tsan
elif [ "${address_space}" = "kernel_user" -a "${mode}" = "generic" -a "${san_type}" = "msan" ]; then
	kernel_generic_msan
	user_generic_msan
elif [ "${address_space}" = "kernel_user" -a "${mode}" = "tagged" -a "${san_type}" = "asan" ]; then
	kernel_tagged_asan
	user_tagged_asan
else
	echo -e "\e[0;31m not support this group: ${mode}, ${san_type}, ${address_space} \e[m for ${mfrs_cfg} ERROR !!!"
	exit 1
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

sed -i 's|^USERFS_TYPE:=cramfs|#USERFS_TYPE:=cramfs|g' ${mfrs_board_config}
sed -i 's|^USERFS_TYPE:=jffs2|#USERFS_TYPE:=jffs2|g' ${mfrs_board_config}
sed -i 's|^USERFS_TYPE:=ubifs|#USERFS_TYPE:=ubifs|g' ${mfrs_board_config}
sed -i 's|^#USERFS_TYPE:=squashfs|USERFS_TYPE:=squashfs|g' ${mfrs_board_config}
sed -i 's|nand_usrfs_rw=1|#nand_usrfs_rw=1|g' ${mfrs_bootargs_dir}/*

make
RESULT=$?
copy_config
if [ "${RESULT}" != 0 ]; then
	echo -e "\e[0;31m make \e[m for ${mfrs_cfg} ERROR !!!"
	exit 1
fi
