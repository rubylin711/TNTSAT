include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)
EXTRA_CFLAGS += -D__LINUX__

ifeq ($(WITH_TCPIP_PROTOCOL),y)
EXTRA_CFLAGS += -DWITH_TCPIP_PROTOCOL
endif

ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
EXTRA_CFLAGS +=  -DMT_DRM_SUPPORT
INCLUDE_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
EXTRA_CFLAGS +=  -DDRM_SMP_ENABLE
endif

ifeq ($(CFG_FFMPEG_VER_NUM),422)
INCLUDE_PATH += -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
endif

INCLUDE_PATH += \
          -I$(MSP_DIR)/inc \
          -I$(INCLUDE_DIR) \
          -I$(MSP_DIR)/drv/inc \
          -I$(KWARE_DIR)/libmonplayer/include \
          -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
          -I$(KWARE_DIR)/libmonplayer/include/ts_seq \
          -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
          -I$(KWARE_DIR)/libmonplayer/osal \
          -I$(KWARE_DIR)/include/mtos \
          -I$(KWARE_DIR)/libmonplayer/ts_seq \
          -I$(KWARE_DIR)/libdownload/include \
          -I${SDK_DIR}/common/drv/inc \
          -I$(INCLUDE_DIR)

LIB := libPlaybackSeq
OBJS := file_playback_sequence.o \
file_seq_misc.o                  \
file_seq_internal.o              \
process_event.o                  \
process_command.o                \
fp_mem_trans.o                   \
file_fake_mpd.o                  

HEADER_FILES := $(KWARE_DIR)/libmonplayer/include/file_playback_sequence.h
HEADER_FILES_INSTALL_SUBDIR=file_playback

include ${SDK_DIR}/build/script/Makefile-lib.rule
