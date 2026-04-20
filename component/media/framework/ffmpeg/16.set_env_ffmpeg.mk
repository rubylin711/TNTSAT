include ${SDK_DIR}/build/script/base.mk

COPY_SOURCECODE=n

ifeq ($(CFG_FFMPEG_VER_NUM),422)
FFMPEG_FOLDER_NAME=ffmpeg-4.2.2
else
FFMPEG_FOLDER_NAME=ffmpeg-$(CFG_FFMPEG_VER_NUM)
endif
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
CFLAGS += -Werror=incompatible-pointer-types -Werror=uninitialized -Werror-implicit-function-declaration -I$(COMPONENT_DIR)/bento4/Source/C++/Core  -I$(COMPONENT_DIR)/bento4/Source/C++/Codecs  -I$(COMPONENT_DIR)/bento4/Source/C++/MetaData -include $(INCLUDE_DIR)/autoconf-old.h

.PHONY: all install uninstall clean distclean

#CPPFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
#CFLAGS += $(CPPFLAGS) -U__mips_nan2008
ECFLAGS = -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
ELDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
CFLAGS += $(CPPFLAGS)

ifeq ($(CONFIG_MT_DRM_SUPPORT), y)
CFLAGS += -DMT_DRM_SUPPORT
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT), y)
CFLAGS += -DDRM_SMP_ENABLE
endif

ifeq ($(CONFIG_MT_VMX_OTT_SVP_SUPPORT),y)
#CFLAGS += -DVMX_OTT_SVP
endif

#LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
#LIBS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
#LDFLAGS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
#LIBS = -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
#LD_LIBRARY_PATH = $(BUILDROOT_SYSROOT_USR_LIB_DIR)
PKG_CONFIG_PATH = $(BUILDROOT_SYSROOT_USR_LIB_PKGCONFIG_DIR)
PKG_CONFIG_LIBDIR = $(PKG_CONFIG_PATH)
PKG_CONFIG_SYSROOT_DIR = $(BUILDROOT_SYSROOT_DIR)
CXXFLAGS = $(CFLAGS)

export CPPFLAGS CFLAGS LDFLAGS CXXFLAGS LIBS LD_LIBRARY_PATH PKG_CONFIG_PATH PKG_CONFIG_LIBDIR PKG_CONFIG_SYSROOT_DIR
export AR AS CPP LD CXX CC RANLIB NM STRIP OBJCOPY READELF OBJDUMP

CONFIGURE_ARGS = \
  --enable-static \
  --disable-programs \
  --disable-doc \
  --disable-avdevice \
  --disable-avfilter \
  --disable-avresample \
  --disable-postproc \
  --disable-w32threads \
  --disable-os2threads \
  --disable-dct \
  --disable-dwt \
  --disable-lsp \
  --disable-lzo \
  --disable-mdct \
  --disable-rdft \
  --disable-fft \
  --disable-faan \
  --disable-pixelutils \
  --disable-everything \
  --disable-asm \
  --disable-audiotoolbox \
  --enable-demuxer=avi \
  --enable-demuxer=matroska \
  --enable-demuxer=asf \
  --enable-demuxer=flv \
  --enable-demuxer=mov \
  --enable-demuxer=wav \
  --enable-demuxer=flac \
  --enable-demuxer=mpegps \
  --enable-demuxer=mpegts \
  --enable-demuxer=mpegvideo \
  --enable-demuxer=mpegtsraw \
  --enable-demuxer=mgsts \
  --enable-demuxer=m4v \
  --enable-demuxer=ogg \
  --enable-demuxer=vobsub \
  --enable-demuxer=ac3 \
  --enable-demuxer=aac \
  --enable-demuxer=dts \
  --enable-demuxer=eac3 \
  --enable-demuxer=rtp \
  --enable-demuxer=rtsp \
  --enable-demuxer=ape \
  --enable-demuxer=rm \
  --enable-demuxer=mtsurl \
  --enable-parser=h264 \
  --enable-parser=cavsvideo \
  --enable-parser=hevc \
  --enable-parser=pcm_dvd \
  --enable-parser=aac \
  --enable-parser=aac-latm \
  --enable-bsf=h264_mp4toannexb \
  --enable-bsf=hevc_mp4toannexb \
  --enable-bsf=dump_extradata \
  --enable-protocol=file \
  --enable-cross-compile \
  --enable-debug=3 \
  --disable-stripping \
  --target-os=linux \
  --cross-prefix="$(CONFIG_CROSS_COMPILE)" \
  --cc="$(CC)" \
  --ar=$(AR) \
  --as=$(AS) \
  --nm=$(NM) \
  --arch=${ARCH} \
  --enable-shared \
  --enable-pic \
  --prefix=/usr \
  --pkg-config=pkg-config

ifneq ($(CONFIG_MT_SANITIZE),y)
CONFIGURE_ARGS += \
  --enable-parser=mpegvideo \
  --enable-parser=mpeg4video
endif

ifeq ($(CONFIG_MT_ARCH_MIPS),y)
CONFIGURE_ARGS += --enable-mips32r2 --disable-mips32r5 --disable-mips32r6 --disable-mips64r2 --disable-mips64r6 --disable-mipsdsp --disable-mipsdspr2 --disable-mipsfpu --disable-msa --disable-msa2
else ifeq ($(CONFIG_MT_ARCH_ARM),y)
CONFIGURE_ARGS += --disable-neon --disable-armv5te --disable-armv6 --disable-armv6t2 --disable-armv8 --disable-vfp --disable-vfpv3 --disable-setend
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
CONFIGURE_ARGS +=
endif

ifneq ($(CONFIG_MT_FILEPLAY_NONETWORK),y)
CONFIGURE_ARGS += --enable-network \
  --enable-protocol=http           \
  --enable-protocol=https          \
  --enable-protocol=tcp            \
  --enable-protocol=tls            \
  --enable-demuxer=dash            \
  --enable-demuxer=hls_simple      \
  --enable-demuxer=hls_mont        \
  --enable-demuxer=hls             \
  --enable-protocol=udp            \
  --enable-protocol=rtp            \
  --enable-protocol=rtmp
CONFIGURE_ARGS += --enable-openssl \
  --enable-protocol=tls_openssl    \
  --enable-protocol=crypto         \
  --enable-libxml2                 \
  --extra-cflags=$(ECFLAGS)        \
  --extra-ldflags="$(ELDFLAGS)"
else
CONFIGURE_ARGS += --enable-network \
  --enable-protocol=http           \
  --enable-protocol=tcp            \
  --enable-demuxer=hls

CONFIGURE_ARGS += --extra-cflags=$(ECFLAGS) --extra-ldflags=$(ELDFLAGS)  
endif

CONFIGURE_ARGS += --disable-optimizations

all:
	@echo "generate makefile for $(MT_BUILD_DIR)/$(CODE_PATH)";
ifeq ($(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done))
	@echo "$(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done";
else
	@echo "generate makefile for $(MT_BUILD_DIR)/$(CODE_PATH)";
	mkdir -p $(MT_BUILD_DIR)/$(CODE_PATH);
ifeq ($(COPY_SOURCECODE),y)
	cp -arf $(MT_SRC_DIR)/$(CODE_PATH) `dirname $(MT_BUILD_DIR)/$(CODE_PATH)`;
	cd $(MT_BUILD_DIR)/$(CODE_PATH); ./configure $(CONFIGURE_ARGS);
else
	cd $(MT_BUILD_DIR)/$(CODE_PATH); $(MT_SRC_DIR)/$(CODE_PATH)/configure $(CONFIGURE_ARGS);
endif
	sed -i 's/HAVE_LRINT 0/HAVE_LRINT 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_LRINTF 0/HAVE_LRINTF 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_ROUND 0/HAVE_ROUND 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_ROUNDF 0/HAVE_ROUNDF 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_TRUNC 0/HAVE_TRUNC 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_TRUNCF 0/HAVE_TRUNCF 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_CBRT 0/HAVE_CBRT 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_RINT 0/HAVE_RINT 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_HYPOT 0/HAVE_HYPOT 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_CBRTF 0/HAVE_CBRTF 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_COPYSIGN 0/HAVE_COPYSIGN 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_ERF 0/HAVE_ERF 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i '/getenv(x)/d' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_STRUCT_SOCKADDR_STORAGE 0/HAVE_STRUCT_SOCKADDR_STORAGE 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_STRUCT_ADDRINFO 0/HAVE_STRUCT_ADDRINFO 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_STRUCT_POLLFD 1/HAVE_STRUCT_POLLFD 0/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_POLL_H 1/HAVE_POLL_H 0/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/CONFIG_NETWORK 0/CONFIG_NETWORK 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_SOCKLEN_T 0/HAVE_SOCKLEN_T 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_GETADDRINFO 0/HAVE_GETADDRINFO 1/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_INET_ATON 1/HAVE_INET_ATON 0/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_POSIX_MEMALIGN 1/HAVE_POSIX_MEMALIGN 0/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h
	sed -i 's/HAVE_MEMALIGN 1/HAVE_MEMALIGN 0/g' $(MT_BUILD_DIR)/$(CODE_PATH)/config.h

	touch $(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done;
endif
	cd $(MT_BUILD_DIR)/$(CODE_PATH); make -j$(j) V=1
	cd $(MT_BUILD_DIR)/$(CODE_PATH); make install DESTDIR=$(BUILDROOT_SYSROOT_DIR); make install DESTDIR=$(BUILDROOT_TARGET_DIR)
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install
	$(MAKE) -C ${SDK_DIR} copy_lib_to_static

install:

uninstall:

clean:
ifeq ($(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(CODE_PATH); make uninstall; make clean;
else
	echo "$(MT_BUILD_DIR)/$(CODE_PATH) have not makefile, do nothing";
endif

distclean:
ifeq ($(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done,$(wildcard $(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done))
	cd $(MT_BUILD_DIR)/$(CODE_PATH); make distclean;
endif
	@rm -rf $(MT_BUILD_DIR)/$(CODE_PATH)/makefile-done
	@rm -rf $(MT_BUILD_DIR)/$(CODE_PATH)
