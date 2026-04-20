include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(KWARE_DIR)/cc/include

INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc

INCLUDE_PATH += -I$(MSP_DIR)/api/mtgo/include
INCLUDE_PATH += -I$(KWARE_DIR)/include
INCLUDE_PATH += -I$(KWARE_DIR)/include/mtos

INCLUDE_PATH += -I$(CURDIR)/src/708/cc608/include \
	          -I$(CURDIR)/src/708/cc708/include \
            -I$(CURDIR)/src/708/ccdisp/include \
            -I$(CURDIR)/src/708/common/include

OBJS := src/mt_cc_fifo.o src/mt_cc_parse.o src/mt_cc_arib.o src/mt_unf_cc.o

OBJS += src/708/common/src/cc_queue.o \
        src/708/common/src/cc_debug.o \
        src/708/common/src/cc_timer.o

OBJS += src/708/cc608/src/cc608_obj.o \
        src/708/cc608/src/cc608_xds.o \
        src/708/cc608/src/cc608.o \
        src/708/cc608/src/cc608_data.o \
        src/708/cc608/src/cc608_dec.o
                
OBJS += src/708/cc708/src/cc708_obj.o \
        src/708/cc708/src/cc708.o \
        src/708/cc708/src/cc708_dec.o \
        src/708/cc708/src/cc708_osd.o \
        src/708/cc708/src/cc708_pen.o \
        src/708/cc708/src/cc708_cursor.o
        
OBJS += src/708/ccdisp/src/ccdisp_api.o

OBJS += src/com_cc708.o        
                
LIB := libmt_cc

HEADER_FILES=$(wildcard $(KWARE_DIR)/cc/include/*.h)
HEADER_FILES_INSTALL_SUBDIR=cc

include ${SDK_DIR}/build/script/Makefile-lib.rule
