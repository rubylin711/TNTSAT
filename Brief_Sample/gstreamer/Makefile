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
EXTRA_CFLAGS += -DMT_SAMPLE_GSTP_DEBUG
endif
INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(SAMPLE_DIR)/mtgo \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(INCLUDE_DIR) \
          -I$(MSP_DIR)/drv/inc \
          -I${SDK_DIR}/pub/inc \
          -I${SDK_DIR}/pub/inc/mlzplayer \
          -I${SDK_DIR}/pub/inc/suplayer \
          -I$(KWARE_DIR)/libmonplayer/include \
          -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
          -I$(KWARE_DIR)/libmonplayer/suplayer \
          -I$(KWARE_DIR)/libmonplayer/utils \
          -I$(KWARE_DIR)/media/utils        \
          -I$(BUILDROOT_SYSROOT_USR_INC_MT_DIR)/mlzplayer \
          -I$(OPENSOURCE_2ND_PREFIX_DIR)/gstreamer/gstreamer/subprojects/glib/glib \
          -I$(KWARE_DIR)/libmonplayer/suplayer/include


OBJS = sample_gstp.o

ifeq ($(CFG_MT_SAMPLE), y)

EXTRA_CFLAGS += -DMT_SAMPLE_APP
LIB = libmt_sample_gstp

include ${SDK_DIR}/build/script/Makefile-lib.rule

else

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lsuplayer -lmutils -lmonplayer_demux -lTsSeq -lPlaybackSeq -lmt_msp -lBento4 -lmt_tde -lavcodec -lavformat -lavutil -lswresample -lswscale -lcrypto -lssl -lneon -lhttpc -lsmhd  -losal -lmt_mtgo -lmt_jpeg -lcupnp -lmt_ttx -lmt_fastplayer -lmt_os -lmt_subtitle -lmt_subtoutput
DEPEND_LIBS += -lgstmlzplayer
ifneq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
DEPEND_LIBS += -lmd_expat -lgavplay -lgstmtdec
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libffi.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libsoup-2.4.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libglib-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgobject-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgobject-2.0.so.0
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgmodule-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgstreamer-1.0.so
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_PREFIX_DIR)/lib
else
DEPEND_LIBS += -lpsl -lxml2 -lcurl -lssl -lcrypto -lffi -lsoup-2.4 -lgio-2.0 -lsqlite3
DEPEND_LIBS += -lgstbase-1.0 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgstreamer-1.0
endif
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
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
ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
DEPEND_LIBS += -lcurl -lcares -lteec -L${EXTERNAL_BINARY_PREFIX_DIR}/drm -lMTDrmTEE
else
DEPEND_LIBS += -lcurl -lcares -L${EXTERNAL_BINARY_PREFIX_DIR}/drm -lMTDrm
endif
endif
endif

ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
endif
APP = sample_gstp

include ${SDK_DIR}/build/script/Makefile-app.rule
endif
