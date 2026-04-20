include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)

INCLUDE_PATH += \
        -I$(MSP_DIR)/inc -I$(INCLUDE_DIR) \
        -I$(MSP_DIR)/drv/inc \
        -I$(KWARE_DIR)/libmonplayer/include/ts_seq \
        -I$(KWARE_DIR)/libmonplayer/osal \
        -I$(KWARE_DIR)/include/mtos

OBJS := ts_sequence.o fifo_kw.o h264_parser.o mpeg_parser.o avs_parser.o

LIB := libTsSeq
HEADER_FILES += $(KWARE_DIR)/libmonplayer/include/ts_seq/ts_sequence.h
HEADER_FILES_INSTALL_SUBDIR=ts_seq

include ${SDK_DIR}/build/script/Makefile-lib.rule
