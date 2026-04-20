include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(MSP_DIR)/api/inc
INCLUDE_PATH += -I$(COMMON_DIR)/inc
INCLUDE_PATH += -I$(COMMON_DIR)/api/inc
INCLUDE_PATH += -I$(COMMON_DIR)/drv/inc
INCLUDE_PATH += -I$(COMMON_DIR)/api/stat
INCLUDE_PATH += -I$(COMMON_DIR)/api/sys
INCLUDE_PATH += -I$(COMMON_DIR)/api/log
INCLUDE_PATH += -I$(COMMON_DIR)/api/mem
INCLUDE_PATH += -I$(COMMON_DIR)/api/mmz
INCLUDE_PATH += -I$(COMMON_DIR)/api/module
INCLUDE_PATH += -I$(COMMON_DIR)/drv/module
INCLUDE_PATH += -I$(COMMON_DIR)/drv/mem
INCLUDE_PATH += -I$(SAMPLE_DIR)/testfm/inc

LIB := libmt_testfm

OBJS := mt_frame_work.o

HEADER_FILES := $(SAMPLE_DIR)/testfm/inc/mt_frame_work.h
HEADER_FILES_INSTALL_SUBDIR := testframe

include ${SDK_DIR}/build/script/Makefile-lib.rule
