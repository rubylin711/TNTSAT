include ${SDK_DIR}/build/script/base.mk

TARGET := png

INCLUDE_PATH = -I$(COMMON_API_INCLUDE)                \
            -I$(COMMON_DRV_INCLUDE)                   \
            -I$(MSP_API_INCLUDE)                      \
            -I$(MSP_DRV_INCLUDE)                      \
            -I$(COMMON_DIR)/inc                       \
            -I$(MSP_DIR)/inc                          \
            -I$(MSP_DIR)/api/$(TARGET) 								\
            -I$(INCLUDE_DIR)

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
            -I$(MSP_DIR)/api/$(TARGET)				      \
            -I$(INCLUDE_DIR)                            \
            -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
            
LIB := libmt_$(TARGET)

OBJS := mt_png_api.o
OBJS += png_hw.o

HEADER_FILES := $(MSP_DIR)/api/$(TARGET)/mt_png_api.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/png_hw.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
