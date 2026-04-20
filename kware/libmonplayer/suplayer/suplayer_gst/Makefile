include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS) -Wno-error -Wno-implicit-function-declaration

INCLUDE_PATH += \
        -I$(MSP_DIR)/inc \
        -I$(MSP_DIR)/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/include \
	-I$(KWARE_DIR)/libmonplayer/avplay_instance \
        -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/libmpdemux \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/ffmpeg \
        -I${SDK_DIR}/common/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/suplayer/include \
        -I$(KWARE_DIR)/libmonplayer/suplayer/ \
        -I${SDK_DIR}/sample/common \
        -I$(KWARE_DIR)/include/mtos \
        -I$(INCLUDE_DIR)/

INCLUDE_PATH += -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/ \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/gstreamer-1.0 \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/glib-2.0 \
        -I$(BUILDROOT_SYSROOT_USR_LIB_DIR)/glib-2.0/include \
        -I$(BUILDROOT_SYSROOT_USR_LIB_DIR)/gstreamer-1.0/include

LIB := libgstsuplayer

#OBJS := suplayer.o suplayer_hw.o suplayer_internal.o
OBJS += gst_suplayer.o

include ${SDK_DIR}/build/script/Makefile-lib.rule
