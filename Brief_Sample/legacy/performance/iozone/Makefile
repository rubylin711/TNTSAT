CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

all:
	$(MAKE) -C iozone3_483/src/current linux
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) -C iozone3_483/src/current clean

install:
	$(AT)mkdir -p $(BIN_DIR)
	cp -af iozone3_483/src/current/iozone iozone3_483/src/current/fileop iozone3_483/src/current/pit_server $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/iozone $(BIN_DIR)/fileop $(BIN_DIR)/pit_server

distclean:

