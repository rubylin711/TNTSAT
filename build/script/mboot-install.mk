include ${SDK_DIR}/build/script/base.mk

mboot_install:
	mkdir -p ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF
	cp -arf $(MBOOT_DIR)/pub/* ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF

mboot_uninstall:
	rm -rf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF

