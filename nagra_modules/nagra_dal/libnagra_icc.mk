include $(SDK_DIR)/build/script/base.mk

EXTRA_CFLAGS = $(CFG_NAGRA_DAL_CFLAGS)

INCLUDE_PATH := -I$(WB_DIR)/testframework \
          -I$(WB_DIR)/common \
          -I$(COMMON_DIR)/inc \
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
          -I$(WB_DIR) \
	  -I$(SDK_DIR)/nagra_modules/nagra_dal/inc

OBJS := src/ca_icc_s.o
LIB := libnagra_icc

INSTALL_EXTRA := 

include $(SDK_DIR)/build/script/Makefile-lib.rule
