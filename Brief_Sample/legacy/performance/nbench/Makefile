CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

#-j1: avoid multi thread compile error!

all:
	$(MAKE) -C nbench-byte-2.2.3 -j1
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean:
	$(MAKE) -C nbench-byte-2.2.3 mrproper

install:
	$(AT)mkdir -p $(BIN_DIR) $(RESOURCE_DIR)
	cp -af nbench-byte-2.2.3/nbench $(BIN_DIR)
	cp -af nbench-byte-2.2.3/NNET.DAT $(RESOURCE_DIR)
	cp -af nbench-byte-2.2.3/COM.DAT $(RESOURCE_DIR)
	cp -af nbench-byte-2.2.3/debugbit.good.gz $(RESOURCE_DIR)

uninstall:
	rm -f $(BIN_DIR)/nbench
	rm -f $(RESOURCE_DIR)/NNET.DAT
	rm -f $(RESOURCE_DIR)/COM.DAT
	rm -f $(RESOURCE_DIR)/debugbit.good.gz

distclean:

