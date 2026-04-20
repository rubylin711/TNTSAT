include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall distclean

all:
	$(MAKE) -C rzsz-3.48 posix

clean:
	$(MAKE) -C rzsz-3.48 clean

install:
	$(AT)mkdir -p ${BIN_DIR}
	cp -af rzsz-3.48/rb rzsz-3.48/rc rzsz-3.48/rx rzsz-3.48/rz rzsz-3.48/sb rzsz-3.48/sx rzsz-3.48/sz rzsz-3.48/zcommand rzsz-3.48/zcommandi ${BIN_DIR}

uninstall:
	rm -f ${BIN_DIR}/rb ${BIN_DIR}/rc ${BIN_DIR}/rx ${BIN_DIR}/rz ${BIN_DIR}/sb ${BIN_DIR}/sx ${BIN_DIR}/sz ${BIN_DIR}/zcommand ${BIN_DIR}/zcommandi

distclean:
