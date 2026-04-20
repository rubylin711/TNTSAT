include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)

INCLUDE_PATH += -I$(MSP_DIR)/inc \
        -I$(INCLUDE_DIR) \
        -I$(MSP_DIR)/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/osal \
        -I$(KWARE_DIR)/libmonplayer/include \
        -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/libmpdemux \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/ffmpeg \
        -I${SDK_DIR}/common/drv/inc \
        -I$(KWARE_DIR)/include/mtos \
        -I$(KWARE_DIR)/libmonplayer/suplayer/include \
        -I${SDK_DIR}/sample/common

#ifeq ($(CONFIG_MT_GST_SUPLAYER_SUPPORT),y)
INCLUDE_PATH += -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/ \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/gstreamer-1.0 \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/glib-2.0 \
        -I$(BUILDROOT_SYSROOT_USR_LIB_DIR)/glib-2.0/include \
        -I$(BUILDROOT_SYSROOT_USR_LIB_DIR)/gstreamer-1.0/include
#endif

LIB := libsuplayer
#OBJS := suplayer.o suplayer_hw.o suplayer_internal.o
OBJS += suplayer.o suplayer_internal.o

HEADER_FILES := $(wildcard $(KWARE_DIR)/libmonplayer/suplayer/include/*.h)
HEADER_FILES += $(KWARE_DIR)/libmonplayer/suplayer/suplayer_internal.h
HEADER_FILES_INSTALL_SUBDIR=suplayer

include ${SDK_DIR}/build/script/Makefile-lib.rule
