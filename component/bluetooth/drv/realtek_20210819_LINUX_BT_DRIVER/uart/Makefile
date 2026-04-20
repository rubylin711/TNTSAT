include ${SDK_DIR}/build/script/base.mk
DRV_SRC_DIR	:= bluetooth_uart_driver
DRV_DIR	:= $(MODULE_DIR)
TOOL_SRC_DIR := rtk_hciattach

install:
	-mkdir -p $(DRV_DIR)
	make -C $(DRV_SRC_DIR)
	#mv -n $(DRV_DIR)/hci_uart.ko $(FW_DIR)/hci_uart.bk
	cp -af $(DRV_SRC_DIR)/hci_uart.ko $(DRV_DIR)/hci_uart.ko
	echo "make hci_uart_driver success!"
	make -C $(TOOL_SRC_DIR)
	cp -af $(TOOL_SRC_DIR)/rtk_hciattach $(BIN_DIR)/rtk_hciattach
	echo "make rtk_hciattach success!"

uninstall:
	rm -rf $(DRV_DIR)/hci_uart.ko
	echo "uninstall hci_uart_driver success!"
	rm -f $(BIN_DIR)/rtk_hciattach
	echo "uninstall rtk_hciattach success!"

clean:
	make -C $(DRV_SRC_DIR) clean
	make -C $(TOOL_SRC_DIR) clean
