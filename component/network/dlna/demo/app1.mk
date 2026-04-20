include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I../include -I../av/include/ -I../av/include/ -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/suplayer -I${SDK_DIR}/msp/inc -I$(COMMON_UNF_INCLUDE) -I$(BUILDROOT_SYSROOT_USR_INC_DIR)

ifeq ($(CONFIG_MT_GST_SUPLAYER_SUPPORT),y)
EXTRA_CFLAGS = -D__GST_PLAYER__

GSTLIB= \
          -lgstaudio-1.0 -lgstvideo-1.0 -lgsttag-1.0 -lgstbase-1.0 -lgstreamer-1.0 \
          -lglib-2.0 -lgobject-2.0 -lgio-2.0 -lgmodule-2.0 \
          -lgsttypefindfunctions -lgstcoreelements -lgstplayback  -lgstriff-1.0 -lgstpbutils-1.0 \
          -lgstlibav -lavutil -lavformat -lavcodec -lavfilter \
          -lgsthls -lgstadaptivedemux-1.0 -lgsturidownloader-1.0 -lgstmonmulti_queue \
          -lgstrtp-1.0 -lgstvideoparsersbad -lgstaudioparsers -lgstcodecparsers-1.0 \
          -lgstvdec -lgstadec -lgstmontvsink -lgstmontasink -lgstmontssink \
          -lgstneonhttpsrc -lmt_mp4demux -lBento4 -lAvPlayInstance \
          -lz \
          -lffi -lcrypto -lssl \
          -losal -lmutils -lmonplayer_demux -lmt_common -lmt_msp

ifeq (${CONFIG_MT_ENABLE_GST_DASH_DEMUX},y)
GSTLIB += -lgstdashdemux -lxml2 -lgstnet-1.0 -lresolv
endif


endif

OBJS := mt_dmr_demo.o

COMMON_DEPEND_LIBS = -lcupnp -lexpat
ifeq ($(CONFIG_MT_GST_SUPLAYER_SUPPORT),y)
ifneq ($(CONFIG_MT_FILEPLAY_NONETWORK),y)
COMMON_DEPEND_LIBS += -lcrypto -lssl
endif
endif
COMMON_DEPEND_LIBS += -lpthread

ifeq ($(CONFIG_MT_GST_SUPLAYER_SUPPORT),y)
DEPEND_LIBS = $(COMMON_DEPEND_LIBS) -lrt -lm -ldl -lgstsuplayer $(GSTLIB)
ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L${STATIC_LIB_DIR}
else
DEPEND_LIBS_PATH = -L${SHARED_LIB_DIR}
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)/gstreamer-1.0
else
DEPEND_LIBS = $(COMMON_DEPEND_LIBS)
ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L${STATIC_LIB_DIR}
else
DEPEND_LIBS_PATH = -L${SHARED_LIB_DIR}
endif
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)



APP = dlna_dmr

include ${SDK_DIR}/build/script/Makefile-app.rule
