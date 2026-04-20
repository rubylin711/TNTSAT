CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

all:
	$(MAKE) -C lmbench3 OS=$(HOST) CC="$(CC)" AR=$(AR)
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) -C lmbench3 clean

install:
	$(AT)mkdir -p $(BIN_DIR)
	cp -af lmbench3/bin/$(HOST) $(BIN_DIR)/lmbench
	rm -f $(BIN_DIR)/lmbench/*.o $(BIN_DIR)/lmbench/*.a

uninstall:
	rm -rf $(BIN_DIR)/lmbench

distclean:

