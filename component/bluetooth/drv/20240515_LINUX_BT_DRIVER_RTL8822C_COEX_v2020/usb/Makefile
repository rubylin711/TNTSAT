include ${SDK_DIR}/build/script/base.mk

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BUILD_DIR  := $(shell pwd)
ifneq ($(ANDROID_BUILD),y)
INSTALL_DIR := $(MODULE_DIR)
else
INSTALL_DIR := $(ANDROID_PRODUCT_OUT)/system/lib/modules
endif

MODULE_SRC := bluetooth_usb_driver
MODULE := rtk_btusb.ko
MODULE_PATH := $(BUILD_DIR)/$(MODULE_SRC)/rtk_btusb.ko

BUILD_MODULES = $(patsubst %,%_build,${MODULE_SRC})
UNINSTALL_MODULES = $(patsubst %,%_uninstall,${MODULE})

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean install uninstall
all: $(BUILD_MODULES)
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	#@rm -rf $(BUILD_DIR)
	$(MAKE) clean -C $(MODULE_SRC)

install:
	$(AT)mkdir -p $(MODULE_DIR)
	make ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KSRC=$(MT_KERNEL_DIR) -C $(MODULE_SRC) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install

uninstall: $(UNINSTALL_MODULES)
	$(MAKE) uninstall -C $(MODULE_SRC)
#==================================================================
#                          modules
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	make ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KSRC=$(MT_KERNEL_DIR) -C $(BUILD_DIR)/$(patsubst %_build,%,$@) -j$(j)

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	rm -f $(MODULE_DIR)/lib/modules/$(CFG_MT_KERNEL_VERSION)/updates/rtk_btusb.ko


