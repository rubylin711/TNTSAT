include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f app.mk
	$(MAKE) -f ko.mk

clean:
	$(MAKE) -f app.mk clean
	$(MAKE) -f ko.mk clean

install:
	$(MAKE) -f app.mk install
	$(MAKE) -f ko.mk install

uninstall:
	$(MAKE) -f app.mk uninstall
	$(MAKE) -f ko.mk uninstall

distclean:
	$(MAKE) -f app.mk distclean
	$(MAKE) -f ko.mk distclean
