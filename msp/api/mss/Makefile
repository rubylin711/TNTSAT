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
            -I$(INCLUDE_DIR)					\
            -I$(MSP_DIR)/api/mss/bn					\
            -I$(MSP_DIR)/api/mss/pkcs1					\
            -I$(MSP_DIR)/api/mss/rsa

LIB := libmt_mss

OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += $(patsubst %.c, %.o, $(wildcard pkcs1/*.c))
OBJS += $(patsubst %.c, %.o, $(wildcard rsa/*.c))
OBJS += $(patsubst %.c, %.o, $(wildcard ecc/*.c))

HEADER_FILES := $(MSP_DIR)/inc/mt_unf_cipher_v2.h
HEADER_FILES += $(MSP_DIR)/inc/mt_unf_mss.h
HEADER_FILES += $(MSP_DIR)/inc/mt_unf_rsa.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
