include ${SDK_DIR}/build/script/base.mk
SRC_DIR	:= ./bluetooth_sdio_driver
#MDL_DIR	:= $(MONTAGE_ROOTFS_STRIPED)/lib/modules
DRV_DIR	:= $(MODULE_DIR)
install:
	make -C $(SRC_DIR)
	cp -af $(SRC_DIR)/btrtksdio.ko $(DRV_DIR)/btrtksdio.ko
	echo "install rtk_btsdio success!"

uninstall:
	rm -f $(DRV_DIR)/btrtksdio.ko
	echo "uninstall rtk_btsdio success!"

clean:
	make -C $(SRC_DIR) clean
