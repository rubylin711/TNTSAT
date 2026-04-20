include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean prepare

all: prepare
	$(MAKE) -f lib1.mk
	$(MAKE) -f lib2.mk

clean:
	$(MAKE) -f lib1.mk clean
	$(MAKE) -f lib2.mk clean

install:
	$(MAKE) -f lib1.mk install
	$(MAKE) -f lib2.mk install

uninstall:
	$(MAKE) -f lib1.mk uninstall
	$(MAKE) -f lib2.mk uninstall

distclean:
	$(MAKE) -f lib1.mk distclean
	$(MAKE) -f lib2.mk distclean
