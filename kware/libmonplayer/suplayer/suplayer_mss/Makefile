include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)

INCLUDE_PATH += \
        -I$(MSP_DIR)/inc \
        -I$(MSP_DIR)/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/osal \
        -I$(KWARE_DIR)/libmonplayer/include \
        -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/libmpdemux \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/ffmpeg \
        -I$(KWARE_DIR)/include/mtos \
        -I${SDK_DIR}/common/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/suplayer/include \
        -I$(KWARE_DIR)/libmonplayer/suplayer/ \
        -I$(KWARE_DIR)/libmonplayer/ts_seq/ \
        -I$(KWARE_DIR)/libmonplayer/include/ts_seq/ \
        -I${SDK_DIR}/sample/common \
        -I$(INCLUDE_DIR)/

INCLUDE_PATH += -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/ \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/../sspk/inc \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/../sspk/Platform/inc \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/../sspk/Platform/LinuxSample/inc \
        -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/glib-2.0

LIB := libmsssuplayer

#OBJS := suplayer.o suplayer_hw.o suplayer_internal.o
OBJS += mss_suplayer.o

include ${SDK_DIR}/build/script/Makefile-lib.rule
