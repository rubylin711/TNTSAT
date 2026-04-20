include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f lib.mk
	$(MAKE) -f app.mk

clean:
	$(MAKE) -f lib.mk clean
	$(MAKE) -f app.mk clean

install:
	$(MAKE) -f lib.mk install
	$(MAKE) -f app.mk install

uninstall:
	$(MAKE) -f lib.mk uninstall
	$(MAKE) -f app.mk uninstall

distclean:
	$(MAKE) -f lib.mk distclean
	$(MAKE) -f app.mk distclean
