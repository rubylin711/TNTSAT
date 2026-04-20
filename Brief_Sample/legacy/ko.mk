include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -C kprobe-uprobe/kprobe
	#$(MAKE) -C kprobe-uprobe/raw-kprobe
	$(MAKE) -C jill/drv/jill/
	$(MAKE) -C jill/drv/jill-dev0
	$(MAKE) -C jill/drv/jill-dev1

clean:
	$(MAKE) -C kprobe-uprobe/kprobe clean
	#$(MAKE) -C kprobe-uprobe/raw-kprobe clean
	$(MAKE) -C jill/drv/jill/ clean
	$(MAKE) -C jill/drv/jill-dev0 clean
	$(MAKE) -C jill/drv/jill-dev1 clean

install:
	$(MAKE) -C kprobe-uprobe/kprobe install
	#$(MAKE) -C kprobe-uprobe/raw-kprobe install
	$(MAKE) -C jill/drv/jill/ install
	$(MAKE) -C jill/drv/jill-dev0 install
	$(MAKE) -C jill/drv/jill-dev1 install

uninstall:
	$(MAKE) -C kprobe-uprobe/kprobe uninstall
	#$(MAKE) -C kprobe-uprobe/raw-kprobe uninstall
	$(MAKE) -C jill/drv/jill/ uninstall
	$(MAKE) -C jill/drv/jill-dev0 uninstall
	$(MAKE) -C jill/drv/jill-dev1 uninstall

distclean:
	$(MAKE) -C kprobe-uprobe/kprobe distclean
	#$(MAKE) -C kprobe-uprobe/raw-kprobe distclean
	$(MAKE) -C jill/drv/jill/ distclean
	$(MAKE) -C jill/drv/jill-dev0 distclean
	$(MAKE) -C jill/drv/jill-dev1 distclean
