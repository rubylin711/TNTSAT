include ${SDK_DIR}/build/script/base.mk

all:
	make menuconfig ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)";
