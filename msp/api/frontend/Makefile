include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)      \
            -I$(COMMON_API_INCLUDE)         \
            -I$(COMMON_DRV_INCLUDE)         \
            -I$(MSP_UNF_INCLUDE)            \
            -I$(MSP_API_INCLUDE)            \
            -I$(MSP_DRV_INCLUDE)            \
            -I$(INCLUDE_DIR)

OBJS := mt_unf_frontend.o

ifeq ($(CONFIG_MT_DISEQC_SUPPORT),y)
OBJS += mt_unf_diseqc.o
OBJS += mt_unf_unicable.o
endif

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_frontend.h

include $(MSP_DIR)/api/Makefile.rule
