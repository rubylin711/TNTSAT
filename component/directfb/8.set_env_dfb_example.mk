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
CFLAGS += $(CPPFLAGS) -fno-builtin-cos -fno-builtin-sin -fno-builtin-cosf -fno-builtin-sinf
LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -L$(SHARED_LIB_DIR)
LDFLAGS += -Wl,-rpath-link,$(BUILDROOT_SYSROOT_USR_LIB_DIR),-rpath-link,$(SHARED_LIB_DIR)
LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
CXXFLAGS = $(CFLAGS)

export CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LD_LIBRARY_PATH PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP


DFB_EXAMPLE_SRC_DIR=$(COMPONENT_DIR)/directfb/DirectFB-examples-1.6.0
DFB_EXAMPLE_BUILD_DIR=$(COMPONENT_BUILD_DIR)/directfb/DirectFB-examples-1.6.0

RE_CONFIG := y
ifeq ($(DFB_BUILD_DIR)/makefile-done,$(wildcard $(DFB_BUILD_DIR)/makefile-done))
	ifeq ($(FORCE_CONFIG), )
		RE_CONFIG := n
	endif
endif

all:
	@echo -e "\033[32;5m""===== now is build DFB example =====""\033[00m"
	mkdir -p ${DFB_EXAMPLE_BUILD_DIR};

ifeq ($(RE_CONFIG), n)
	@echo "Already configured, ignore configure"
else
	cd ${DFB_EXAMPLE_BUILD_DIR}; ${DFB_EXAMPLE_SRC_DIR}/configure --host=${HOST} --prefix=/usr;
	@touch $(DFB_EXAMPLE_BUILD_DIR)/makefile-done
endif
	cd ${DFB_EXAMPLE_BUILD_DIR}; make -j$(j) V=1
	cd ${DFB_EXAMPLE_BUILD_DIR}; make install DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install DESTDIR=$(BUILDROOT_TARGET_DIR)
	$(MAKE) -C ${SDK_DIR} copy_lib_to_static


clean:
ifeq ($(DFB_EXAMPLE_BUILD_DIR)/makefile-done,$(wildcard $(DFB_EXAMPLE_BUILD_DIR)/makefile-done))
	@rm -f $(DFB_EXAMPLE_BUILD_DIR)/makefile-done
endif
	cd ${DFB_EXAMPLE_BUILD_DIR}; make uninstall; make clean

install:


uninstall:


distclean:
	cd ${DFB_EXAMPLE_BUILD_DIR}; make distclean
	rm -rf $(DFB_EXAMPLE_BUILD_DIR)
