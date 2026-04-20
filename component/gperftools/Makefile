include ${SDK_DIR}/build/script/base.mk

ifneq ($(CONFIG_MT_ARCH_MIPS),y)
CONFIGURE_ARGS_EXTRA = --enable-frame-pointers
export CONFIGURE_ARGS_EXTRA
endif
COPY_SOURCECODE = y
export COPY_SOURCECODE

.PHONY: all install uninstall clean distclean

all:
	$(MAKE) -f 14.set_env_gperftools.mk SRC_PATH=gperftools/gperftools-2.15 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -f 14.set_env_gperftools.mk clean SRC_PATH=gperftools/gperftools-2.15 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -f 14.set_env_gperftools.mk distclean SRC_PATH=gperftools/gperftools-2.15 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
