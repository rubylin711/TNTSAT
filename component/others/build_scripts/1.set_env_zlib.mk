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

CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -DUSE_MMAP
CFLAGS += $(CPPFLAGS)
CXXFLAGS = $(CFLAGS)

export CPPFLAGS CFLAGS CXXFLAGS
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP


all:
ifeq ($(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done,$(wildcard $(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done))
	echo "$(MT_BUILD_DIR)/zlib-1.2.11/zlib/makefile-done";
else
	echo "generate makefile for $(MT_BUILD_DIR)/zlib/zlib-1.2.11";
	mkdir -p $(MT_BUILD_DIR)/zlib/zlib-1.2.11;
	cp -arf $(MT_SRC_DIR)/zlib/zlib-1.2.11 $(MT_BUILD_DIR)/zlib;
	cd $(MT_BUILD_DIR)/zlib/zlib-1.2.11; ./configure --prefix=/usr --enable-shared;
	touch $(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done;
endif
	cd $(MT_BUILD_DIR)/zlib/zlib-1.2.11; make -j$(j) V=1
	cd $(MT_BUILD_DIR)/zlib/zlib-1.2.11; make install DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install DESTDIR=$(BUILDROOT_TARGET_DIR)
	$(MAKE) -C ${SDK_DIR} copy_lib_to_static

install:

uninstall:

clean:
ifeq ($(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done,$(wildcard $(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done))
	cd $(MT_BUILD_DIR)/zlib/zlib-1.2.11; make uninstall; make clean;
else
	echo "$(MT_BUILD_DIR)/zlib/zlib-1.2.11 have not makefile, do nothing";
endif

distclean:
ifeq ($(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done,$(wildcard $(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done))
	cd $(MT_BUILD_DIR)/zlib/zlib-1.2.11; make distclean;
endif
	@rm -rf $(MT_BUILD_DIR)/zlib/zlib-1.2.11/makefile-done
	@rm -rf $(MT_BUILD_DIR)/zlib/zlib-1.2.11
