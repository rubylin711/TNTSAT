include ${SDK_DIR}/build/script/base.mk
FW_USB_DIR := $(MODULE_DIR)/lib/firmware
FW_UART_DIR := $(MODULE_DIR)/lib/firmware
FW_SDIO_DIR := $(MODULE_DIR)/lib/firmware
RTKBT_FIRMWARE_DIR := rtkbt-firmware/lib/firmware
MDL_DIR	:= $(MODULE_DIR)/lib/modules/

ifeq ($(CONFIG_BT_INTERFACE_USB),y)
INTERFACE := usb
endif
ifeq ($(CONFIG_BT_INTERFACE_SDIO),y)
INTERFACE := sdio
endif
ifeq ($(CONFIG_BT_INTERFACE_UART),y)
INTERFACE := uart
endif

all: install

install: envset
	-mkdir -p $(FW_DIR)
	echo "Copy $(RTKBT_FIRMWARE_DIR)/$(FW_FILE) to $(FW_DIR)"
	cd $(RTKBT_FIRMWARE_DIR); cp -af --parents $(FW_FILE) $(FW_DIR)
	echo "Copy $(RTKBT_FIRMWARE_DIR)/$(CONFIG_FILE) $(FW_DIR)"
	cd $(RTKBT_FIRMWARE_DIR); cp -af --parents $(CONFIG_FILE) $(FW_DIR)
	make -C $(SRC_ROOT_DIR) install

uninstall: envset
	rm -rf $(FW_DIR)/$(FW_FILE)
	rm -rf $(FW_DIR)/$(CONFIG_FILE)
	make -C $(SRC_ROOT_DIR) uninstall

clean: envset
	make -C $(SRC_ROOT_DIR) clean
	
envset:
ifneq (,$(filter usb all,$(INTERFACE)))
	$(eval FW_FILE := rtl*_fw)
	$(eval CONFIG_FILE := rtl*_config)
	$(eval FW_DIR := $(FW_USB_DIR))
	$(eval SRC_ROOT_DIR := usb)
endif

ifneq (,$(filter uart all,$(INTERFACE)))
	$(eval FW_FILE := rtlbt/rtl*_fw)
	$(eval CONFIG_FILE := rtlbt/rtl*_config)
	$(eval FW_DIR := $(FW_UART_DIR))
	$(eval SRC_ROOT_DIR := uart)
endif
		
ifneq (,$(filter sdio all,$(INTERFACE)))
	$(eval FW_FILE := rtl_bt/rtl*_fw)
	$(eval CONFIG_FILE := rtl_bt/rtl*_config)
	$(eval FW_DIR := $(FW_SDIO_DIR))
	$(eval SRC_ROOT_DIR := sdio)
endif
	
