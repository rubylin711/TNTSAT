include ${SDK_DIR}/build/script/base.mk

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BUILD_DIR  := $(shell pwd)/host

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean install uninstall
all: 
	cd $(BUILD_DIR) && $(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KLIB_BUILD=$(MT_KERNEL_DIR) -j$(j)
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	@cd $(BUILD_DIR) && $(MAKE) ARCH=$(KERNEL_ARCH) KLIB_BUILD=$(MT_KERNEL_DIR) clean

install:
	$(AT)mkdir -p $(MODULE_DIR)
	cd $(BUILD_DIR) && $(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KLIB_BUILD=$(MT_KERNEL_DIR) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install

uninstall:
	rm -f $(MODULE_DIR)/lib/modules/$(CFG_MT_KERNEL_VERSION)/extra/m88wi6700u.ko

#                          modules
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	cd $(BUILD_DIR)/$(patsubst %_build,%,$@) && $(MAKE) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KLIB_BUILD=$(MT_KERNEL_DIR) -j$(j)
