include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

ifeq ($(CFG_MT_SAMPLE_DEBUG),y)
EXTRA_CFLAGS += -DMT_SAMPLE_WDG_DEBUG
endif
INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(INCLUDE_DIR)/testframe \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc


OBJS = sample_wdg.o

ifeq ($(CFG_MT_SAMPLE), y)

EXTRA_CFLAGS += -DMT_SAMPLE_APP
LIB := libmt_sample_wdg

include ${SDK_DIR}/build/script/Makefile-lib.rule

else

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)

endif

APP = sample_wdg

include ${SDK_DIR}/build/script/Makefile-app.rule

endif
