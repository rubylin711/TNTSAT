CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

all:
ifneq (conf-cc,$(wildcard conf-cc))
	cp -af conf-cc-orig conf-cc
	sed -i 's|^cc|${CC}|g' conf-cc
endif
ifneq (conf-ld,$(wildcard conf-ld))
	cp -af conf-ld-orig conf-ld
	sed -i 's|^cc|${CC}|g' conf-ld
endif
	$(MAKE)

clean:
	$(MAKE) clean
	rm -rf conf-cc conf-ld

install:
	$(AT)mkdir -p $(BIN_DIR)
	cp -af memtester $(BIN_DIR)

uninstall:
	rm -rf $(BIN_DIR)/memtester

distclean:

