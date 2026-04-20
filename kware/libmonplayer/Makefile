include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean prepare

all: prepare
	$(MAKE) -f libmonplayer_osal.mk
	$(MAKE) -f libmonplayer_other.mk

clean:
	$(MAKE) -f libmonplayer_osal.mk clean
	$(MAKE) -f libmonplayer_other.mk clean

install:
	$(MAKE) -f libmonplayer_osal.mk install
	$(MAKE) -f libmonplayer_other.mk install

uninstall:
	$(MAKE) -f libmonplayer_osal.mk uninstall
	$(MAKE) -f libmonplayer_other.mk uninstall

distclean:
	$(MAKE) -f libmonplayer_osal.mk distclean
	$(MAKE) -f libmonplayer_other.mk distclean
