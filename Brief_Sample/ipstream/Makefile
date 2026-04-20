include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

ifeq ($(CFG_MT_CI_DEV_CIMAX),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAX
endif
ifeq ($(CFG_MT_CI_DEV_CIMAXPLUS),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAXPLUS
endif
ifeq ($(CFG_MT_CI_DEV_HICI),y)
EXTRA_CFLAGS += -DMT_CI_DEV_HICI
endif
ifeq ($(CFG_MT_SAMPLE_DEBUG),y)
EXTRA_CFLAGS += -DMT_SAMPLE_IPSTREAM_DEBUG
endif
INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(INCLUDE_DIR) \
          -I$(MSP_DIR)/drv/inc \
          -I${SDK_DIR}/pub/inc \
          -I${SDK_DIR}/pub/inc/suplayer \
          -I$(KWARE_DIR)/libmonplayer/include \
          -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
          -I$(KWARE_DIR)/libmonplayer/suplayer \
          -I$(KWARE_DIR)/libmonplayer/suplayer/include


OBJS = sample_ipstream.o

ifeq ($(CFG_MT_SAMPLE), y)

EXTRA_CFLAGS += -DMT_SAMPLE_APP
LIB = libmt_sample_ipstream

include ${SDK_DIR}/build/script/Makefile-lib.rule

else

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lsuplayer -losal -lmutils -lmonplayer_demux -lTsSeq -lPlaybackSeq -lmt_msp -lBento4 -lmt_tde -lavcodec -lavformat -lavutil -lswresample -lswscale -lexpat -lcrypto -lssl

ifeq ($(CONFIG_MT_CHIP_SYMPHONY4),y)
DEPEND_LIBS += -lhttpc -lneon
endif

ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
DEPEND_LIBS += -lcurl -lMTDrmTEE -lcares -lteec
else
DEPEND_LIBS += -lcurl -lMTDrm -lcares
endif
endif
endif
ifeq ($(CONFIG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_LIB_DIR)
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
DEPEND_LIBS_PATH += -Wl,-rpath-link,$(OPENSOURCE_2ND_LIB_DIR)
DEPEND_LIBS_PATH += -L$(OPENSOURCE_SHARED_LIB_DIR) -Wl,-rpath-link,$(OPENSOURCE_SHARED_LIB_DIR)

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
DEPEND_LIBS_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm
endif
endif

ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)


endif
APP = sample_ipstream

include ${SDK_DIR}/build/script/Makefile-app.rule
endif
