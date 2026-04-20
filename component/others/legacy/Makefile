include ${SDK_DIR}/build/script/base.mk

CALL_TOP_MAKEFILE = pkg_la_path_release pkg_la_path_build

.PHONY: all clean install uninstall distclean $(CALL_TOP_MAKEFILE)

all:
	$(MAKE) -C bento4
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

install:
	$(MAKE) -C bento4 install

uninstall:
	$(MAKE) -C bento4 uninstall

clean:
	$(MAKE) -C bento4 clean

distclean:
	$(MAKE) -C bento4 distclean

$(CALL_TOP_MAKEFILE):
	$(MAKE) -C ${SDK_DIR} $@
