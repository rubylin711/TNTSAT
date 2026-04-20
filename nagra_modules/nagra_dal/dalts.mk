include $(SDK_DIR)/build/script/base.mk

#include $(WB_DIR)/base.mk
include ./base.mk

EXTRA_CFLAGS = $(CFG_NAGRA_DAL_CFLAGS)

INCLUDE_PATH := -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/api/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/api/mtgo/include \
          -I$(MSP_DIR)/drv/inc \
          -I$(MSP_DIR)/drv/mtfb/include \
          -I$(MSP_DIR)/api/ffmpeg \
          -I$(MSP_DIR)/api/ffmpeg \
          -I$(MSP_DIR)/api/mtgo \
          -I$(INCLUDE_DIR)/testframe \
          -I$(KWARE_DIR)/libmonplayer/include \
          -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
          -I$(KWARE_DIR)/libmonplayer/include/ts_seq \
          -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
          -I$(SDK_DIR)/pub/inc	\
          -I$(SDK_DIR)/pub/inc/dlna \
          -I$(SDK_DIR)/pub/inc/mt_wlan \
          -I$(SDK_DIR)/pub/inc/http_file \
          -I$(SDK_DIR)/pub/inc/suplayer \
          -I$(SDK_DIR)/pub/inc/teletext \
          -I$(KWARE_DIR)/libmonplayer/suplayer \
          -I$(KWARE_DIR)/libmonplayer/suplayer/include \
          -I$(OPENSOURCE_INC_DIR) \
          -I$(KWARE_DIR)/libdownload/include/http_file \
          -I$(INCLUDE_DIR)/fastplayer \
          -I$(SDK_DIR)/pub/inc/RSSClient \
          -I$(KWARE_DIR)/teletext/include \
          -I$(INCLUDE_DIR)/subtoutput \
          -I$(INCLUDE_DIR)/subtitle \
          -I$(SAMPLE_DIR)/common \
          -I$(SDK_DIR)/nagra_modules/nagra_dal/inc \
          -I$(SDK_DIR)/nagra_modules/nagra_dal \
          -I$(SDK_DIR)/nagra_modules/nagra_dal/test_inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/NOCSAPIs/NOCSAPIs_PKG_EXT-04.13.01/inc
OBJS := dalts.o mt_hdmi.o mtgo.o

DEPEND_LIBS := $(SYS_LIBS) -lmt_nocsapi_impl -lmt_common -lmt_msp -lmt_mss -lmt_sci -lnagra_dal -lnagra_icc -lmt_sample_common -lmt_mtgo -lmt_tde -lmt_jpeg -lmt_png -lfreetype -lmt_os -lteec -lsmpc -lpng -lz
DEPEND_LIBS += $(DALTS_LIB)

DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/build
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/nagra_dal/build
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/nagra_dal
DEPEND_LIBS_PATH += -L$(SDK_DIR)/tee_bsp/arm64/usr/lib
DEPEND_LIBS_PATH += -L$(OPENSOURCE_LIB_DIR)
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
DEPEND_LIBS_PATH += -Wl,-rpath-link,$(OPENSOURCE_2ND_LIB_DIR)

ifeq ($(CONFIG_MT_STATIC_LINK),y)
  DEPEND_LIBS_PATH += -L$(STATIC_LIB_DIR)
else
  DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR)
endif

APP := dalts
INSTALL_EXTRA := 

include $(SDK_DIR)/build/script/Makefile-app.rule
