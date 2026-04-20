include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean prepare

all: prepare
	$(MAKE) -f dlna.mk
	$(MAKE) -C demo
	$(MAKE) -C daemon
	$(MAKE) -C satip

clean:
	$(MAKE) -f dlna.mk clean
	$(MAKE) -C demo clean
	$(MAKE) -C daemon clean
	$(MAKE) -C satip clean

install:
	$(MAKE) -f dlna.mk install
	$(MAKE) -C demo install
	$(MAKE) -C daemon install
	$(MAKE) -C satip install

uninstall:
	$(MAKE) -f dlna.mk uninstall
	$(MAKE) -C demo uninstall
	$(MAKE) -C daemon uninstall
	$(MAKE) -C satip uninstall

distclean:
	$(MAKE) -f dlna.mk distclean
	$(MAKE) -C demo distclean
	$(MAKE) -C daemon distclean
	$(MAKE) -C satip distclean
