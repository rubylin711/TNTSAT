include ${SDK_DIR}/build/script/base.mk

RTKBT_FIRMWARE_DIR := $(shell pwd)/../../rtkbt-firmware/lib/firmware
FW_USB_DIR := $(MODULE_DIR)/lib/firmware

EXTRA_CFLAGS += -I$(MSP_DIR)/drv/inc -I$(SDK_DIR)/pub/inc

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

MODULE_NAME := rtk_btusb
obj-m		:= $(MODULE_NAME).o 
$(MODULE_NAME)-y := rtk_coex.o rtk_misc.o rtk_bt.o


all: modules

modules:
	$(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" -C $(MT_KERNEL_DIR) M=$(shell pwd) $(MT_KERNEL_OUTPUT_OPT) modules -j$(j)
	
install:
modules_install:
	$(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" -C $(MT_KERNEL_DIR) M=$(shell pwd) $(MT_KERNEL_OUTPUT_OPT) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install
	mkdir -p $(FW_USB_DIR)
	@echo "Copy $(RTKBT_FIRMWARE_DIR)/rtl*_fw to $(FW_USB_DIR)"
	cp -a $(RTKBT_FIRMWARE_DIR)/rtl*_fw $(FW_USB_DIR)
	@echo "Copy $(RTKBT_FIRMWARE_DIR)/rtl*_config $(FW_USB_DIR)"
	cp -a $(RTKBT_FIRMWARE_DIR)/rtl*_config $(FW_USB_DIR)
	
uninstall:
	rm -f $(MODULE_DIR)/lib/modules/$(CFG_MT_KERNEL_VERSION)/updates/$(MODULE_NAME).ko
	rm -f $(FW_USB_DIR)/rtl*_fw
	rm -f $(FW_USB_DIR)/rtl*_config

.PHONY: modules clean
	
clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a .*.cmd *.mod
	rm -rf .tmp_versions 
