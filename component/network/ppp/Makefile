include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

install:
	$(AT)mkdir -p $(BUILDROOT_SYSROOT_DIR)/etc/ppp
	$(AT)cp -arf scripts/* $(BUILDROOT_SYSROOT_DIR)/etc/ppp/
	$(AT)cp -arf peers/* $(BIN_DIR)/

uninstall:
	$(AT)rm -f $(BUILDROOT_SYSROOT_DIR)/etc/ppp/*
	$(AT)rm -f $(BIN_DIR)/gprs-*dial

clean:

distclean:
