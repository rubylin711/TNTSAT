include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -f 13.set_env_kexec.mk SRC_PATH=kexec/kexec-tools-2.0.17 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -f 13.set_env_kexec.mk clean SRC_PATH=kexec/kexec-tools-2.0.17 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -f 13.set_env_kexec.mk distclean SRC_PATH=kexec/kexec-tools-2.0.17 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
