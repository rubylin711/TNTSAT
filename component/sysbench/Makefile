include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

SRC_PATH = sysbench/sysbench-1.0.20

all:
	$(MAKE) -f 19.set_env_sysbench.mk SRC_PATH=$(SRC_PATH) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -f 19.set_env_sysbench.mk clean SRC_PATH=$(SRC_PATH) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -f 19.set_env_sysbench.mk distclean SRC_PATH=$(SRC_PATH) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
