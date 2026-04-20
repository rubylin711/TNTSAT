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

ifeq (linux-$(CFG_MT_KERNEL_VERSION),linux-3.10.y)
MODULE_SRC = cfg80211_3.10.y
else
MODULE_SRC = cfg80211
endif
MODULE = cfg80211.ko
MODULE_PATH = $(BUILD_DIR)/$(MODULE_SRC)/cfg80211.ko

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
	@rm -rf $(BUILD_DIR)

install:
	@test -d $(INSTALL_DIR) || mkdir -p $(INSTALL_DIR)
	@cp -v $(MODULE_PATH) $(INSTALL_DIR)/

uninstall: $(UNINSTALL_MODULES)

#==================================================================
#                          modules
#==================================================================
%_build: $(BUILD_DIR)
	@echo -e "\033[31m""Build $(patsubst %_build,%,$@) ...""\033[00m"
	make ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CROSS_COMPILE)" -C $(MT_KERNEL_DIR) \
		$(MT_KERNEL_OUTPUT_OPT) M=$(BUILD_DIR)/$(patsubst %_build,%,$@) modules

$(BUILD_DIR):
	@test -d $(BUILD_DIR) || mkdir -p $(BUILD_DIR)

%_uninstall:
	@rm -f $(INSTALL_DIR)/$(patsubst %_uninstall,%,$@)
