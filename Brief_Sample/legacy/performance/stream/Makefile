CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

all:
	$(MAKE) -C src
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) -C src clean

install:
	$(AT)mkdir -p $(BIN_DIR)
	cp -af src/stream_c.exe $(BIN_DIR)

uninstall:
	rm -f $(BIN_DIR)/stream_c.exe

distclean:

