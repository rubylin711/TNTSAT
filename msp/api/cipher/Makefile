include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -Iinclude                            \
	    -I$(COMMON_UNF_INCLUDE)                 \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(INCLUDE_DIR)

LIB := libmt_cipher

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

HEADER_FILES := $(MSP_DIR)/inc/mt_unf_cipher.h

#start_address=$(shell grep "T _stext" ${SDK_DIR}/out/general/kernel/linux-x.y.z/System.map |cut -c9-16)
#end_address=$(shell grep "T _etext" ${SDK_DIR}/out/general/kernel/linux-x.y.z/System.map |cut -c9-16)
#CFLAGS_EXTRA += -DRUNTIME_CHECK_START_ADDRESS=0x$(start_address)
#CFLAGS_EXTRA += -DRUNTIME_CHECK_END_ADDRESS=0x$(end_address)

#include $(MSP_DIR)/api/Makefile.rule
include ${SDK_DIR}/build/script/Makefile-lib.rule
