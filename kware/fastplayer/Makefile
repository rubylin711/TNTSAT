include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include

INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \


INCLUDE_PATH += -I../include
INCLUDE_PATH += -I$(KWARE_DIR)/include/fastplayer
INCLUDE_PATH += -I$(KWARE_DIR)/libmonplayer/include/ts_seq
INCLUDE_PATH += -I../include/mtos

OBJS := fast_dvb_player.o
#OBJS += fast_dvb_player_drv_sdk.o
#OBJS += fast_dvb_player_priv.o

LIB := libmt_fastplayer

HEADER_FILES=$(KWARE_DIR)/include/fastplayer
#HEADER_FILES_INSTALL_SUBDIR=vfs

include ${SDK_DIR}/build/script/Makefile-lib.rule
