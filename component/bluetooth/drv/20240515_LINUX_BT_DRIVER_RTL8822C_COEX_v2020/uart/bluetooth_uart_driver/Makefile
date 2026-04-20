include ${SDK_DIR}/build/script/base.mk

RTKBT_FIRMWARE_DIR := $(shell pwd)/../../rtkbt-firmware/lib/firmware
FW_UART_DIR := $(MODULE_DIR)/lib/firmware/rtlbt

EXTRA_CFLAGS += -I$(MSP_DIR)/drv/inc -I$(SDK_DIR)/pub/inc

MODULE_NAME := hci_uart
obj-m		:= $(MODULE_NAME).o 
$(MODULE_NAME)-y := hci_ldisc.o hci_h4.o hci_rtk_h5.o rtk_coex.o


all: modules
modules:
	$(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" -C $(MT_KERNEL_DIR) M=$(shell pwd) $(MT_KERNEL_OUTPUT_OPT) modules -j$(j)
	
install:
modules_install:
	$(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" -C $(MT_KERNEL_DIR) M=$(shell pwd) $(MT_KERNEL_OUTPUT_OPT) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install
	mkdir -p $(FW_UART_DIR)
	@echo "Copy $(RTKBT_FIRMWARE_DIR)/rtlbt/rtl*_fw to $(FW_UART_DIR)"
	cp -a $(RTKBT_FIRMWARE_DIR)/rtlbt/rtl*_fw $(FW_UART_DIR)
	@echo "Copy $(RTKBT_FIRMWARE_DIR)/rtlbt/rtl*_config $(FW_UART_DIR)"
	cp -a $(RTKBT_FIRMWARE_DIR)/rtlbt/rtl*_config $(FW_UART_DIR)
	
uninstall:
	rm -f $(MODULE_DIR)/lib/modules/$(CFG_MT_KERNEL_VERSION)/updates/$(MODULE_NAME).ko
	rm -rf $(FW_UART_DIR)

.PHONY: modules clean
	
clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a .*.cmd *.mod
	rm -rf .tmp_versions 
