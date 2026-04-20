include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(MSP_UNF_INCLUDE) -I$(MSP_DRV_INCLUDE) -I$(MSP_API_INCLUDE) -I$(COMMON_UNF_INCLUDE) -I$(COMMON_DRV_INCLUDE) -I$(COMMON_API_INCLUDE)

OBJS := lxc_ipc.o

LIB := liblxc_ipc

HEADER_FILES := $(MSP_API_INCLUDE)/lxc_ipc.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
