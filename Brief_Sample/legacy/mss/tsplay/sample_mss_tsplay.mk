include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)


INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(INCLUDE_DIR)


OBJS := tsplay/sample_mss_tsplay.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
DEPEND_LIBS_PATH += -L$(EXTERNAL_BINARY_PREFIX_DIR)/lib

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
DEPEND_LIBS_PATH += -L$(TEE_LIB_DIR)
DEPEND_LIBS += -lteec
endif

APP := sample_mss_tsplay

include ${SDK_DIR}/build/script/Makefile-app.rule
