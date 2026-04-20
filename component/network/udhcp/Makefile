include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

install:
	$(AT)mkdir -p $(BIN_DIR)
	$(AT)cp -arf udhcp.script $(BIN_DIR)

uninstall:
	$(AT)rm -f $(BIN_DIR)/udhcp.script

clean:

distclean:
