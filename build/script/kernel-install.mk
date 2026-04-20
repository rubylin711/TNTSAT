include ${SDK_DIR}/build/script/base.mk

kernel_install:
	make -C $(MT_KERNEL_DIR) $(MT_KERNEL_OUTPUT_OPT) ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" INSTALL_MOD_PATH=$(MODULE_DIR) modules_install

kernel_uninstall:
ifeq (${DOING_MT_SDK_RELEASE},1)
	mkdir -p $(MODULE_DIR)/lib/modules-bak
	-cd $(MODULE_DIR)/lib/modules; cp -af --parents `find -name "symphony4-regulator.ko"` $(MODULE_DIR)/lib/modules-bak
	-cd $(MODULE_DIR)/lib/modules; cp -af --parents `find -name "mt_vofw.ko"` $(MODULE_DIR)/lib/modules-bak
	-cd $(MODULE_DIR)/lib/modules; cp -af --parents `find -name "mt_vfmw.ko"` $(MODULE_DIR)/lib/modules-bak
	rm -rf $(MODULE_DIR)/lib/modules
	mv $(MODULE_DIR)/lib/modules-bak $(MODULE_DIR)/lib/modules
else
	rm -rf $(MODULE_DIR)/lib/modules
endif

uImage_install:
	mkdir -p $(BIN_DIR)
	cp -af $(KERNEL_OUTPUT)/arch/$(KERNEL_ARCH)/boot/uImage* $(BIN_DIR)
	cp -af $(KERNEL_OUTPUT)/vmlinux $(BIN_DIR)
	-find -L $(KERNEL_OUTPUT)/arch/$(KERNEL_ARCH)/boot/dts -name "*.dtb" -exec cp -af {} $(BIN_DIR) \;

uImage_uninstall:
	rm -f $(BIN_DIR)/uImage
	rm -f $(BIN_DIR)/uImage-dtb
	rm -f $(BIN_DIR)/vmlinux
	rm -f $(BIN_DIR)/*.dtb
