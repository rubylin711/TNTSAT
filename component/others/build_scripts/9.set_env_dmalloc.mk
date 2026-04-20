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

CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(INCLUDE_DIR)
CFLAGS += $(CPPFLAGS)
LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
CXXFLAGS = $(CFLAGS)

export CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LD_LIBRARY_PATH PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP


all:
ifeq ($(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done,$(wildcard $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done))
	echo "$(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done";
else
	echo "generate makefile for $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2";
	mkdir -p $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2;
	cd $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2; $(MT_SRC_DIR)/dmalloc/dmalloc-5.5.2/configure --host=${HOST} --prefix=/usr --enable-cxx --enable-threads;
	touch $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done;
endif
	cd $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2; make -j$(j) V=1
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make install DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install DESTDIR=$(BUILDROOT_TARGET_DIR)

clean:
ifeq ($(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done,$(wildcard $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done))
	cd $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2; make uninstall; make clean;
else
	echo "$(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2 have not makefile, do nothing";
endif

install:

uninstall:

distclean:
ifeq ($(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done,$(wildcard $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done))
	cd $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2; make distclean;
endif
	@rm -rf $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2/makefile-done
	@rm -rf $(MT_BUILD_DIR)/dmalloc/dmalloc-5.5.2
