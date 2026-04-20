include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

ifeq ($(CONFIG_MT_ARCH_ARM),y)
#ARCH_CFLAGS = $(ARM_COMPILE_OPTION)
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
#ARCH_CFLAGS = $(AARCH64_COMPILE_OPTION)
else ifeq ($(CONFIG_MT_ARCH_MIPS),y)
ARCH_CFLAGS = $(MUCLIBC) $(MFLOAT) $(EL)
endif

CPPFLAGS = $(FIXED_SYSROOT_USR_LIB_PATH) -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
CFLAGS = $(CPPFLAGS)
#don't use -static
LDFLAGS = $(FIXED_SYSROOT_USR_LIB_PATH) -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -Wl,-rpath-link=$(BUILDROOT_SYSROOT_USR_LIB_DIR) ${EL}
EXTRA_CFLAGS = $(CFLAGS) $(ARCH_CFLAGS)
DESTDIR = $(BUILDROOT_SYSROOT_USR_DIR)

export EXTRA_CFLAGS LDFLAGS DESTDIR


all:
	cd ${MT_KERNEL_DIR}/tools/vm; $(MAKE) ARCH=${KERNEL_ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" page-types slabinfo page_owner_sort WERROR=0 V=1
	mkdir -p $(BUILDROOT_SYSROOT_USR_DIR)/bin
	cp -af ${MT_KERNEL_DIR}/tools/vm/page_owner_sort $(BUILDROOT_SYSROOT_USR_DIR)/bin
	cp -af ${MT_KERNEL_DIR}/tools/vm/page-types $(BUILDROOT_SYSROOT_USR_DIR)/bin
	cp -af ${MT_KERNEL_DIR}/tools/vm/slabinfo $(BUILDROOT_SYSROOT_USR_DIR)/bin
	cp -af ${MT_KERNEL_DIR}/tools/vm/slabinfo-gnuplot.sh $(BUILDROOT_SYSROOT_USR_DIR)/bin

install:
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/page-types
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/slabinfo
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/page_owner_sort
	#$(BUILDROOT_SYSROOT_USR_DIR)/bin/slabinfo-gnuplot.sh

uninstall:

clean:
	cd ${MT_KERNEL_DIR}/tools/vm; $(MAKE) ARCH=${KERNEL_ARCH} CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" clean
	rm -f $(BUILDROOT_SYSROOT_USR_DIR)/bin/page_owner_sort $(BUILDROOT_SYSROOT_USR_DIR)/bin/page-types $(BUILDROOT_SYSROOT_USR_DIR)/bin/slabinfo $(BUILDROOT_SYSROOT_USR_DIR)/bin/slabinfo-gnuplot.sh

distclean:
