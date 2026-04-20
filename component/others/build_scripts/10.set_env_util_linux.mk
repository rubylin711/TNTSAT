include ${SDK_DIR}/build/script/base.mk

ifeq ($(CONFIG_MT_DEBUG),y)
CFLAGS = -g
else
CFLAGS =
endif

ifeq ($(CONFIG_MT_OPTIMIZE_OS),y)
CFLAGS += -Os
else ifeq ($(CONFIG_MT_OPTIMIZE_O2),y)
CFLAGS += -O2
endif

.PHONY: all install uninstall clean distclean

CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
CFLAGS += $(CPPFLAGS)
LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
LIBS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
CXXFLAGS = $(CFLAGS)

export CPPFLAGS CFLAGS LDFLAGS CXXFLAGS LIBS LD_LIBRARY_PATH PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP


all:
ifeq ($(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done))
	echo "$(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done";
else
	echo "generate makefile for $(MT_BUILD_DIR)/$(SRC_PATH)";
	mkdir -p $(MT_BUILD_DIR)/$(SRC_PATH);
ifeq ($(COPY_SOURCECODE),y)
	cp -arf $(MT_SRC_DIR)/$(SRC_PATH) `dirname $(MT_BUILD_DIR)/$(SRC_PATH)`;
	cd $(MT_BUILD_DIR)/$(SRC_PATH); aclocal; autoconf; autoheader; automake --add-missing; ./configure --host=${HOST} --prefix=/usr $(CONFIGURE_ARGS_EXTRA);
else
	cd $(MT_BUILD_DIR)/$(SRC_PATH); aclocal; autoconf; autoheader; automake --add-missing; $(MT_SRC_DIR)/$(SRC_PATH)/configure --host=${HOST} --prefix=/usr $(CONFIGURE_ARGS_EXTRA);
endif
	touch $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done;
endif
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make -j$(j) V=1
	-cd $(MT_BUILD_DIR)/$(SRC_PATH); make install
	echo -e "\033[31m can not make install, need sudo make install, this will destroy the build machine environment \033[0m"

install:

uninstall:

clean:
ifeq ($(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make clean;
else
	echo "$(MT_BUILD_DIR)/$(SRC_PATH) have not makefile, do nothing";
endif

distclean:
ifeq ($(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make distclean;
endif
	@rm -rf $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done
	@rm -rf $(MT_BUILD_DIR)/$(SRC_PATH)
