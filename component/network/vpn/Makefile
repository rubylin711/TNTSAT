include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean
AT=
all:
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

install:
	@echo $(BUILDROOT_SYSROOT_DIR)
	$(AT)cp -arf vpn.sh $(BIN_DIR)/
ifeq ($(CONFIG_MT_VPN_SUPPORT_OPENVPN),y)
	$(AT)mkdir -p $(BUILDROOT_SYSROOT_DIR)/etc/openvpn
	$(AT)mkdir -p $(BUILDROOT_SYSROOT_DIR)/etc/init.d
	$(AT)cp -arf client.ovpn $(BUILDROOT_SYSROOT_DIR)/etc/openvpn
	$(AT)cp -arf openvpn-client $(BUILDROOT_SYSROOT_DIR)/etc/init.d
else
ifeq ($(CONFIG_MT_VPN_IPSEC_ENABLE),y)
	$(AT)cp -arf strongswan.conf $(BUILDROOT_SYSROOT_DIR)/etc/
endif
endif

uninstall:
ifeq ($(CONFIG_MT_VPN_SUPPORT_OPENVPN),y)
	$(AT)rm -rf $(BUILDROOT_SYSROOT_DIR)/etc/openvpn
	$(AT)rm -f $(BUILDROOT_SYSROOT_DIR)/etc/init.d/openvpn-client
else
	$(AT)rm -rf $(BUILDROOT_SYSROOT_DIR)/etc/strongswan.conf
endif
	$(AT)rm -rf $(BIN_DIR)/vpn.sh

clean:

distclean:
