include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean prepare

all: prepare
	$(MAKE) -f memtester.mk -C memtester-4.5.1
	$(MAKE) -f memtester.mk -C memtester-4.5.1 install

clean:
	$(MAKE) -f memtester.mk -C memtester-4.5.1 clean

install:
	$(MAKE) -f memtester.mk -C memtester-4.5.1 install

uninstall:
	$(MAKE) -f memtester.mk -C memtester-4.5.1 uninstall

distclean:
	$(MAKE) -f memtester.mk -C memtester-4.5.1 distclean
