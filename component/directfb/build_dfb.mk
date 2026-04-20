include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -f 7.set_env_dfb.mk

install:

uninstall:

clean:
	-$(MAKE) -f 7.set_env_dfb.mk clean

distclean:
	-$(MAKE) -f 7.set_env_dfb.mk distclean
