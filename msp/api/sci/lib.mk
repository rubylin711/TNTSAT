include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)              \
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

OBJS := mt_unf_sci.o
OBJS += protocol/atr.o
OBJS += protocol/icc_async.o
OBJS += protocol/ifd_aria.o
OBJS += protocol/protocol_t0.o
OBJS += protocol/protocol_t1.o
OBJS += protocol/string.o

LIB = libmt_sci

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_sci.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
