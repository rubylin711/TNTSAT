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

MODULE_SRC := ATBM_LINUX_WIFI4_BLE_SVN3407_AsmLite19040_ARESM18826_Meru19963_V1.1_20240527
MODULE := atbm_usb_wifi.ko
MODULE_PATH := $(BUILD_DIR)/atbm_usb_wifi.ko

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
	@cp $(MODULE_SRC)/defconfig $(MODULE_SRC)/.config
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) clean -C $(MODULE_SRC)
	@rm -rf $(MODULE_SRC)/.config

install:
	$(AT)mkdir -p $(MODULE_DIR)
	make ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KSRC=$(MT_KERNEL_DIR) -C $(MODULE_SRC) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install

uninstall: $(UNINSTALL_MODULES)

#==================================================================
#                          modules
#==================================================================
%_build: $(BUILD_DIR)
	@cp $(MODULE_SRC)/defconfig $(MODULE_SRC)/.config
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	make ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KSRC=$(MT_KERNEL_DIR) -C $(BUILD_DIR)/$(patsubst %_build,%,$@)

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	rm -f $(MODULE_DIR)/lib/modules/$(CFG_MT_KERNEL_VERSION)/extra/atbm6x3x.ko

