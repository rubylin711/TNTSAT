include ${SDK_DIR}/build/script/base.mk

CURRENT_DIR :=  $(shell pwd)
EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)
EXTRA_CFLAGS += -D__LINUX__

INCLUDE_PATH = 

ifeq ($(CONFIG_MT_DRM_SUPPORT), y)
EXTRA_CFLAGS += -DMT_DRM_SUPPORT
INCLUDE_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
EXTRA_CFLAGS +=  -DDRM_SMP_ENABLE
endif


INCLUDE_PATH += -I$(INCLUDE_DIR) -I$(MSP_DIR)/inc -I$(CURRENT_DIR) -I$(CURRENT_DIR)/stream_filter

LIB  := libmtlzavfilter
OBJS := mtlz_avfilter.o mtlz_avfilter_stream.o         \
		platform/stream_filter_os_support.o            \
		stream_filter/stream_filter_h264.o             \
		stream_filter/stream_filter_h265.o             \
		stream_filter/stream_filter_vc1.o              \
		stream_filter/stream_filter_vpx.o

ifneq ($(findstring -DMT_DRM_SUPPORT,$(EXTRA_CFLAGS)),)
	OBJS += stream_filter/stream_filter_drm.o
endif

HEADER_FILES = $(CURRENT_DIR)/mtlz_avfilter.h $(CURRENT_DIR)/mtlz_avfilter_stream.h
HEADER_FILES_INSTALL_SUBDIR = media/mtlz_avfilter
include ${SDK_DIR}/build/script/Makefile-lib.rule
