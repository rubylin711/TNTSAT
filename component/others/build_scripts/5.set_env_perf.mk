include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

CPPFLAGS = $(FIXED_SYSROOT_USR_LIB_PATH) -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(COMMON_UNF_INCLUDE)
CFLAGS = $(CPPFLAGS)
CFLAGS += -Wno-stringop-overflow -Wno-stringop-truncation -Wno-packed-not-aligned -Wno-format-truncation -Wno-restrict -Wno-format-overflow
ifeq ($(CONFIG_MT_ARCH_ARM),y)
#ARCH_CFLAGS = $(ARM_COMPILE_OPTION)
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
#ARCH_CFLAGS = $(AARCH64_COMPILE_OPTION)
else ifeq ($(CONFIG_MT_ARCH_MIPS),y)
ARCH_CFLAGS = $(MUCLIBC) $(MFLOAT) $(EL)
endif
#don't use -static, because the perf call dlopen
LDFLAGS = $(FIXED_SYSROOT_USR_LIB_PATH) -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -Wl,-rpath-link=$(BUILDROOT_SYSROOT_USR_LIB_DIR) ${EL}
EXTRA_CFLAGS = $(CFLAGS) $(ARCH_CFLAGS)
PKG_CONFIG = pkg-config
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)

export EXTRA_CFLAGS LDFLAGS DESTDIR PKG_CONFIG PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR


all:
	@echo -e "\033[32m must compile perf first, then compile gstreamer, otherwise perf will compile fail\033[0m"
	cd ${MT_KERNEL_DIR}/tools/perf; $(MAKE) ARCH=${KERNEL_ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" WERROR=0 V=1 NO_JEVENTS=1
	cd ${MT_KERNEL_DIR}/tools/perf; $(MAKE) ARCH=${KERNEL_ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" WERROR=0 V=1 NO_JEVENTS=1 install DESTDIR=$(BUILDROOT_SYSROOT_USR_DIR)
	cd ${MT_KERNEL_DIR}/tools/perf; $(MAKE) ARCH=${KERNEL_ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" WERROR=0 V=1 NO_JEVENTS=1 install DESTDIR=$(BUILDROOT_TARGET_USR_DIR)

install:
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/perf
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/trace
	#$(BUILDROOT_SYSROOT_USR_DIR)/etc/bash_completion.d
	#$(BUILDROOT_SYSROOT_USR_DIR)/lib/traceevent
	#$(BUILDROOT_SYSROOT_USR_DIR)/libexec/perf-core
	#should we must use bash to support libexec/perf-core/perf-with-kcore and libexec/perf-core/perf-archive ?
	#$(BUILDROOT_SYSROOT_USR_DIR)/lib/libunwind*.so*
	#$(BUILDROOT_SYSROOT_USR_DIR)/lib/elfutils/*$(CFG_MT_ARCH)*


uninstall:

clean:
	cd ${MT_KERNEL_DIR}/tools/perf; $(MAKE) ARCH=${ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" clean

distclean:
