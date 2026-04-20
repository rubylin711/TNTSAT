include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f lib.mk
	$(MAKE) -f app1.mk
	$(MAKE) -f app2.mk

clean:
	$(MAKE) -f lib.mk clean
	$(MAKE) -f app1.mk clean
	$(MAKE) -f app2.mk clean

install:
	$(MAKE) -f lib.mk install
	$(MAKE) -f app1.mk install
	$(MAKE) -f app2.mk install

uninstall:
	$(MAKE) -f lib.mk uninstall
	$(MAKE) -f app1.mk uninstall
	$(MAKE) -f app2.mk uninstall

distclean:
	$(MAKE) -f lib.mk distclean
	$(MAKE) -f app1.mk distclean
	$(MAKE) -f app2.mk distclean
