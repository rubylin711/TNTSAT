include ${SDK_DIR}/build/script/base.mk

CONFIG_BTUSB_AUTOSUSPEND = n
CONFIG_BTUSB_WAKEUP_HOST = n
CONFIG_BTCOEX = y

ifeq ($(CONFIG_BTUSB_AUTOSUSPEND), y)
	EXTRA_CFLAGS += -DCONFIG_BTUSB_AUTOSUSPEND
endif

ifeq ($(CONFIG_BTUSB_WAKEUP_HOST), y)
	EXTRA_CFLAGS += -DCONFIG_BTUSB_WAKEUP_HOST
endif

ifeq ($(CONFIG_BTCOEX), y)
	EXTRA_CFLAGS += -DCONFIG_BTCOEX
endif
ifneq ($(KERNELRELEASE),)
	obj-m := rtk_btusb.o
	rtk_btusb-y = rtk_coex.o rtk_misc.o rtk_bt.o
else
	PWD := $(shell pwd)

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" -C $(MT_KERNEL_DIR) M=$(shell pwd)  $(MT_KERNEL_OUTPUT_OPT) modules -j$(j)

clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a .*.cmd
	rm -rf .tmp_versions
	
endif
