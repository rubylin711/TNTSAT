include ${SDK_DIR}/build/script/base.mk
SRC_DIR	:= ./bluetooth_usb_driver
#MDL_DIR	:= $(MONTAGE_ROOTFS_STRIPED)/lib/modules/
DRV_DIR	:= $(MODULE_DIR)

##########################################
install:
	make -C $(SRC_DIR)
	cp -af $(SRC_DIR)/rtk_btusb.ko $(DRV_DIR)/rtk_btusb.ko
	echo "install rtk_btusb success!"

##########################################

uninstall:
	rm -f $(DRV_DIR)/rtk_btusb.ko
	echo "uninstall rtk_btusb success!"
##########################################

clean:
	make -C $(SRC_DIR) clean

