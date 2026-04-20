include ${SDK_DIR}/build/script/base.mk

TARGET := tde

#EXTRA_CFLAGS = -DHI_TDE_BITMAPMASK_SUPPORT=0
INCLUDE_PATH = -I$(COMMON_API_INCLUDE)                \
            -I$(COMMON_DRV_INCLUDE)                   \
            -I$(MSP_API_INCLUDE)                      \
            -I$(MSP_DRV_INCLUDE)                      \
            -I$(COMMON_DIR)/inc                       \
            -I$(MSP_DIR)/inc                          \
            -I$(MSP_DIR)/api/$(TARGET)/include        \
            -I$(MSP_DIR)/drv/$(TARGET)/include        \
            -I$(MSP_DIR)/api/$(TARGET)/src/gpe/driver \
            -I$(MSP_DIR)/api/$(TARGET)/src/gpe/symphony_soft \
            -I$(MSP_DIR)/drv/tde          \
            -I$(INCLUDE_DIR)

LIB := libmt_$(TARGET)

#ifeq ($(CONFIG_MT_HAS_MT_TDE_DRV),y)
OBJS := src/mt_unf_tde.o
#else
#OBJS := src/mt_unf_tde_imp_gpe.o
#endif

OBJS += src/gpe/driver/gpe.o
OBJS += src/gpe/symphony_soft/gpe_hw.o
OBJS += src/gpe/symphony_soft/scale.o

HEADER_FILES := $(MSP_DIR)/api/$(TARGET)/include/mt_tde_api.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/include/mt_tde_type.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/include/mt_tde_errcode.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
