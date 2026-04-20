include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

CONFIGURE_ARGS_EXTRA = --with-malloc-conf=background_thread:true
COPY_SOURCECODE = y
export COPY_SOURCECODE CONFIGURE_ARGS_EXTRA

all:
	$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others_autogen.mk SRC_PATH=jemalloc/jemalloc-5.3.0 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others_autogen.mk clean SRC_PATH=jemalloc/jemalloc-5.3.0 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -C ${COMPONENT_DIR}/others/build_scripts -f 2.set_env_others_autogen.mk distclean SRC_PATH=jemalloc/jemalloc-5.3.0 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
