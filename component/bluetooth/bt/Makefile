include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean

all:
	$(MAKE) -f lib.mk
	$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) -f lib.mk clean

install:
	mkdir -p $(BIN_DIR)
	$(MAKE) -f lib.mk install
	cp -af script/bluetooth.sh $(BIN_DIR)/bluetooth.sh
	cp -af script/system.conf $(BIN_DIR)/system.conf
	cp -af script/bluez.conf $(BIN_DIR)/bluez.conf

uninstall:
	$(MAKE) -f lib.mk uninstall
	rm -rf $(BIN_DIR)/bluetooth.sh
	rm -rf $(BIN_DIR)/system.conf
	rm -rf $(BIN_DIR)/bluez.conf

distclean:
	$(MAKE) -f lib.mk distclean
