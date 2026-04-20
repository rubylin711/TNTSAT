include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -f 24.set_env_systemtap.mk SRC_PATH=systemtap/systemtap-4.5 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -f 24.set_env_systemtap.mk clean SRC_PATH=systemtap/systemtap-4.5 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -f 24.set_env_systemtap.mk distclean SRC_PATH=systemtap/systemtap-4.5 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
