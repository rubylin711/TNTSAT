include ${SDK_DIR}/build/script/base.mk

TARGET := gfx2d

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
            -I$(MSP_DIR)/api/$(TARGET)/include      \
            -I$(MSP_DIR)/drv/$(TARGET)/include

OBJS := src/mt_gfx2d_api.o

HEADER_FILES := $(MSP_DIR)/api/$(TARGET)/include/mt_gfx2d_api.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/include/mt_gfx2d_type.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/include/mt_gfx2d_errcode.h

include $(MSP_DIR)/api/Makefile.rule
