include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -f 8.set_env_dfb_example.mk

install:
	mkdir -p ${RESOURCE_DIR}
	cp -af .directfbrc ${RESOURCE_DIR}
	cp -arf $(BUILDROOT_SYSROOT_USR_DIR)/share/directfb-examples ${RESOURCE_DIR}

uninstall:
	rm -f ${RESOURCE_DIR}/.directfbrc
	rm -rf ${RESOURCE_DIR}/directfb-examples

clean:
	-$(MAKE) -f 8.set_env_dfb_example.mk clean

distclean:
	-$(MAKE) -f 8.set_env_dfb_example.mk distclean
