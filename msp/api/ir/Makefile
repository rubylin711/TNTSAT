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

#ifeq ($(CFG_MT_IR_TYPE_STD), y)
OBJS := unf_ir_std.o
#endif

ifeq ($(CFG_MT_IR_TYPE_S2), y)
OBJS := unf_ir.o
endif

ifeq ($(CFG_MT_IR_TYPE_LIRC), y)
OBJS := unf_ir_lirc.o
endif

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_ir.h

include $(MSP_DIR)/api/Makefile.rule
