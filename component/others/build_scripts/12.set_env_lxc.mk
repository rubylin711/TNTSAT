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

CC := $(subst -D_GNU_SOURCE,,$(CC))
CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
CFLAGS += $(CPPFLAGS) -DMT_LXCROOTFSMOUNT -Wno-stringop-overflow -Wno-stringop-truncation -Wno-format-truncation
LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
LIBS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
INSTALL_ARGS = bashcompdir=$(BUILDROOT_SYSROOT_USR_DIR)/share/bash-completion/completions --bindir=$(BUILDROOT_SYSROOT_USR_DIR)/bin --sbindir=$(BUILDROOT_SYSROOT_USR_DIR)/sbin --libexecdir=$(BUILDROOT_SYSROOT_USR_LIB_DIR)exec --libdir=$(BUILDROOT_SYSROOT_USR_LIB_DIR) --includedir=$(BUILDROOT_SYSROOT_USR_INC_DIR) --sysconfdir=$(BUILDROOT_SYSROOT_DIR)/etc --with-systemdsystemunitdir=$(BUILDROOT_SYSROOT_USR_LIB_DIR)/systemd/system --datarootdir=$(BUILDROOT_SYSROOT_USR_DIR)/share --localstatedir=$(BUILDROOT_SYSROOT_DIR)/var
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
	cd $(MT_BUILD_DIR)/$(SRC_PATH); autoreconf -f -i; ./configure --host=${HOST} --prefix=/ $(INSTALL_ARGS);
else
	cd $(MT_BUILD_DIR)/$(SRC_PATH); autoreconf -f -i; $(MT_SRC_DIR)/$(SRC_PATH)/configure --host=${HOST} --prefix=/ $(INSTALL_ARGS);
endif
	touch $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done;
endif
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make -j$(j) V=1
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make install DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install DESTDIR=$(BUILDROOT_TARGET_DIR)

install:

uninstall:

clean:
ifeq ($(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make uninstall; make clean;
else
	echo "$(MT_BUILD_DIR)/$(SRC_PATH) have not makefile, do nothing";
endif

distclean:
ifeq ($(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(SRC_PATH); make distclean;
endif
	@rm -rf $(MT_BUILD_DIR)/$(SRC_PATH)/makefile-done
	@rm -rf $(MT_BUILD_DIR)/$(SRC_PATH)
