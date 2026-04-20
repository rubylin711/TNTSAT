include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)
EXTRA_CFLAGS += -D__LINUX__





INCLUDE_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm


ifeq ($(CONFIG_MT_VMX_OTT_SVP_SUPPORT),y)
EXTRA_CFLAGS +=  -DVMX_OTT_SVP
endif
INCLUDE_PATH +=                             \
        -I$(MSP_DIR)/inc                    \
        -I$(INCLUDE_DIR)                    \
        -I$(KWARE_DIR)/include/mtos         \
        -I$(MSP_DIR)/drv/inc                \
        -I$(MSP_DIR)/api/vdec               \
        -I$(MSP_DIR)/api/inc                \
        -I$(KWARE_DIR)/libmonplayer/include \
        -I$(KWARE_DIR)/libmonplayer/file_playback \
        -I$(KWARE_DIR)/../external-binary/output/aarch64_output/drm
INCLUDE_PATH +=  -I$(KWARE_DIR)/libmonplayer/osal
INCLUDE_PATH +=  -I$(KWARE_DIR)/media/mtlz_avfilter

LIB := libosal
OBJS := drv_adp.o

ifneq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/mtlz_avfilter.o
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/mtlz_avfilter_stream.o
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/stream_filter/stream_filter_vc1.o
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/stream_filter/stream_filter_vpx.o
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/stream_filter/stream_filter_h264.o
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/stream_filter/stream_filter_h265.o
ifeq ($(CONFIG_MT_DRM_SUPPORT), y)
OBJS += $(KWARE_DIR)/media/mtlz_avfilter/stream_filter/stream_filter_drm.o
endif
endif

include ${SDK_DIR}/build/script/Makefile-lib.rule
