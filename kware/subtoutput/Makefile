include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
#INCLUDE_PATH += -I${SDK_DIR}/source/common/include
INCLUDE_PATH += -I$(KWARE_DIR)/subtoutput/include

LIB := libmt_subtoutput

OBJS := src/mt_unf_so.o src/so_queue.o

HEADER_FILES=$(KWARE_DIR)/subtoutput/include/mt_unf_subtouput.h
HEADER_FILES_INSTALL_SUBDIR=subtoutput

include ${SDK_DIR}/build/script/Makefile-lib.rule
