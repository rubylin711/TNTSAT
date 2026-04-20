include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f lib1.mk
	$(MAKE) -f lib2.mk
	$(MAKE) -f app.mk

clean:
	$(MAKE) -f lib1.mk clean
	$(MAKE) -f lib2.mk clean
	$(MAKE) -f app.mk clean

install:
	$(MAKE) -f lib1.mk install
	$(MAKE) -f lib2.mk install
	$(MAKE) -f app.mk install

uninstall:
	$(MAKE) -f lib1.mk uninstall
	$(MAKE) -f lib2.mk uninstall
	$(MAKE) -f app.mk uninstall

distclean:
	$(MAKE) -f lib1.mk distclean
	$(MAKE) -f lib2.mk distclean
	$(MAKE) -f app.mk distclean
