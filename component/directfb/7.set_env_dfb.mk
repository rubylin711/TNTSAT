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

DEPEND_LIBS = -lmt_jpeg -lmt_tde -lmt_mtgo
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
DEPEND_LIBS += -lmt_png
CFLAGS += -DCONFIG_MT_TDE_CMD_FIFO_SUPPORT
CFLAGS += -DCONFIG_MT_HW_PNG_SUPPORT
endif
DEPEND_LIBS += -lmt_common -lpng -lfreetype -lz -lrt -lm -lstdc++ -lpthread -ldl

ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
CFLAGS += -DCONFIG_MT_MTGO_JPEG_SUPPORT
endif

.PHONY: all install uninstall clean distclean

CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(MSP_DIR)/drv/inc -I$(MSP_DIR)/api/inc -I$(MSP_DIR)/inc -I$(COMMON_DIR)/drv/inc -I$(INCLUDE_DIR) -include $(INCLUDE_DIR)/autoconf-old.h
CFLAGS += $(CPPFLAGS)
LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -L$(SHARED_LIB_DIR)
LDFLAGS += -Wl,-rpath-link,$(SHARED_LIB_DIR)
LIBS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) $(DEPEND_LIBS)
LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
CXXFLAGS = $(CFLAGS)

ifeq ($(KERNELRELEASE),)
	KERNELRELEASE = $(shell cat $(KERNEL_OUTPUT)/include/config/kernel.release 2> /dev/null)
endif

export CPPFLAGS CFLAGS LDFLAGS CXXFLAGS LIBS LD_LIBRARY_PATH PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR
export LIBPNG_CFLAGS=-I$(BUILDROOT_SYSROOT_USR_INC_DIR)/libpng16
export LIBPNG_LIBS=-L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -lpng16
export FREETYPE_CFLAGS=-I$(BUILDROOT_SYSROOT_USR_INC_DIR)/freetype2 -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
export FREETYPE_LIBS=-L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -lfreetype
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP
export KERNELRELEASE

DFB_SRC_DIR=$(COMPONENT_DIR)/directfb/DirectFB-1.7.7
DFB_BUILD_DIR=$(COMPONENT_BUILD_DIR)/directfb/DirectFB-1.7.7


DFB_ENABLE_DEBUG_OPT := $(if $(DFB_DEBUG),--enable-debug --enable-trace, )
MULTI_FUSION_OPT := $(if $(CONFIG_MT_LINUX_FUSION),--enable-multi --enable-multi-kernel, )

RE_CONFIG := y
ifeq ($(DFB_BUILD_DIR)/makefile-done,$(wildcard $(DFB_BUILD_DIR)/makefile-done))
	ifeq ($(FORCE_CONFIG), )
		RE_CONFIG := n
	endif
endif


all:
	@echo -e "\033[32;5m""===== now is build DFB =====""\033[00m"
ifeq ($(RE_CONFIG), n)
	@echo "Already configured, ignore configure"
else
	@echo "generate makefile for ${DFB_BUILD_DIR}"
	mkdir -p ${DFB_BUILD_DIR};
	cd ${DFB_BUILD_DIR}; \
	${DFB_SRC_DIR}/configure --prefix=/usr --host=${HOST} \
		--enable-gif --enable-png --enable-jpeg --disable-x11 --disable-video4linux \
		--disable-linotype --with-gfxdrivers=tde --with-inputdrivers=linuxinput --without-setsockopt \
		--with-tests  --enable-freetype=yes  --disable-zlib --without-tools have_linux=yes --disable-video4linux \
		--disable-video4linux2 --disable-voodoo --disable-mmx --disable-sdl \
		--enable-sawman \
		--disable-vnc \
		${DFB_ENABLE_DEBUG_OPT} ${MULTI_FUSION_OPT} ; \
	touch $(DFB_BUILD_DIR)/makefile-done;
endif
	cd ${DFB_BUILD_DIR}; make -j$(j) V=1
	cd ${DFB_BUILD_DIR}; make install V=1 DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install V=1 DESTDIR=$(BUILDROOT_TARGET_DIR)
	unset KERNELRELEASE; $(MAKE) -C ${SDK_DIR} copy_lib_to_static



clean:
ifeq ($(DFB_BUILD_DIR)/makefile-done,$(wildcard $(DFB_BUILD_DIR)/makefile-done))
	@rm -f $(DFB_BUILD_DIR)/makefile-done;
endif
	cd ${DFB_BUILD_DIR}; make uninstall; make clean

install:

uninstall:

distclean:
	cd ${DFB_BUILD_DIR}; make distclean
	rm -rf $(DFB_BUILD_DIR)
