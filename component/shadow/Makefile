include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others.mk SRC_PATH=shadow/shadow-4.13 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others.mk clean SRC_PATH=shadow/shadow-4.13 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others.mk distclean SRC_PATH=shadow/shadow-4.13 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
