include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)
EXTRA_CFLAGS += -D__LINUX__
ifeq ($(WITH_TCPIP_PROTOCOL),y)
EXTRA_CFLAGS += -DWITH_TCPIP_PROTOCOL
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
        -I$(KWARE_DIR)/libmonplayer/ts_seq \
        -I${SDK_DIR}/common/drv/inc \
        -I$(MSP_DIR)/api/ffmpeg \
        -I$(MSP_DIR)/api/ffmpeg/libavcodec \
        -I${SDK_DIR}/pub/inc \
        -I$(INCLUDE_DIR)

LIB := libAvPlayInstance

DEPEND_LIBS = -losal -lmt_msp -lmt_common -lmutils
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)

OBJS := mt_adp_hdmi.o audio_decoder.o video_decoder.o avplayal.o

include ${SDK_DIR}/build/script/Makefile-lib.rule
