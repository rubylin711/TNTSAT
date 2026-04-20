include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

INCLUDE_PATH = -I$(SAMPLE_DIR)/brief_sample/common \
          -I$(COMMON_DIR)/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I${SDK_DIR}/pub/inc \
          -I$(MSP_DIR)/api/mtgo/include


OBJS = sample_box.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lmt_cc -lmt_mtgo -lmt_tde -lmt_os
ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
DEPEND_LIBS += -lmt_jpeg
endif
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)


APP = sample_box

include ${SDK_DIR}/build/script/Makefile-app.rule
