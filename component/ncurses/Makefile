include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

ifeq ($(CONFIG_MT_ARCH_MIPS),y)
$(error please use ncurses of toolchian)
endif

all:
	$(MAKE) -f 4.set_env_ncurses.mk SRC_PATH=ncurses/ncurses-5.9 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

install:

uninstall:

clean:
	-$(MAKE) -f 4.set_env_ncurses.mk clean SRC_PATH=ncurses/ncurses-5.9 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)

distclean:
	-$(MAKE) -f 4.set_env_ncurses.mk distclean SRC_PATH=ncurses/ncurses-5.9 MT_BUILD_DIR=$(COMPONENT_BUILD_DIR) MT_SRC_DIR=$(COMPONENT_DIR)
