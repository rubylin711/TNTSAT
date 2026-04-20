include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f app-parent.mk
	$(MAKE) -f app-client.mk
	$(MAKE) -f app-give-back.mk
	$(MAKE) -f app-hole.mk

clean:
	$(MAKE) -f app-parent.mk clean
	$(MAKE) -f app-client.mk clean
	$(MAKE) -f app-give-back.mk clean
	$(MAKE) -f app-hole.mk clean

install:
	$(MAKE) -f app-parent.mk install
	$(MAKE) -f app-client.mk install
	$(MAKE) -f app-give-back.mk install
	$(MAKE) -f app-hole.mk install

uninstall:
	$(MAKE) -f app-parent.mk uninstall
	$(MAKE) -f app-client.mk uninstall
	$(MAKE) -f app-give-back.mk uninstall
	$(MAKE) -f app-hole.mk uninstall

distclean:
	$(MAKE) -f app-parent.mk distclean
	$(MAKE) -f app-client.mk distclean
	$(MAKE) -f app-give-back.mk distclean
	$(MAKE) -f app-hole.mk distclean
