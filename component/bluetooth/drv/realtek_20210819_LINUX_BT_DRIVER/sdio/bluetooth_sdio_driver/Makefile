include ${SDK_DIR}/build/script/base.mk
# the share variable is exported from wifi, the following is required
ifeq ($(CONFIG_BT_WIFI_COMBO),y)
ccflags-y += -DCONFIG_COMBO_MULTISDIO_EXPORT_FROM_RTW
KBUILD_EXTRA_SYMBOLS=$(MONTAGE_ROOTFS_STRIPED)/lib/firmware/Module.symvers
endif
# please mark the above two lines when testing btrtksdio driver without wifi driver

ifneq ($(KERNELRELEASE),)
	obj-m += btrtksdio.o
	btrtksdio-y := btrtl.o btrtk_sdio.o rtk_coex.o
else
	PWD := $(shell pwd)
all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" -C $(MT_KERNEL_DIR) M=$(shell pwd)  $(MT_KERNEL_OUTPUT_OPT) modules -j$(j)
	
clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a .*.cmd
	rm -rf .tmp_versions
endif
