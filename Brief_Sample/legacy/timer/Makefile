include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

INCLUDE_PATH = -I$(COMMON_DIR)/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc

OBJS = sample_timer.o

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS)
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
DEPEND_LIBS += -L$(TEE_LIB_DIR) -lteec
endif

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

DEPEND_LIBS_PATH += -L$(EXTERNAL_BINARY_PREFIX_DIR)/lib

APP = sample_timer

include ${SDK_DIR}/build/script/Makefile-app.rule
