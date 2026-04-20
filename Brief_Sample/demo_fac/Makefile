include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

ifeq ($(CFG_MT_CI_DEV_CIMAX),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAX
endif
ifeq ($(CFG_MT_CI_DEV_CIMAXPLUS),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAXPLUS
endif
ifeq ($(CFG_MT_CI_DEV_HICI),y)
EXTRA_CFLAGS += -DMT_CI_DEV_HICI
endif

INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(INCLUDE_DIR)/ \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/api/mtgo/include \
          -I$(MSP_DIR)/api/mtgo/src \
          -I$(INCLUDE_DIR)/testframe \
          -I./ui \
		  -I./



OBJS := ./main.o
OBJS += ./demo_usb.o
OBJS += ./demo_tuner.o
OBJS += ./demo_avplay.o
OBJS += ./demo_disp.o
OBJS += ./demo_ir.o
OBJS += ./demo_net.o
OBJS += ./ui/ui_basic.o
OBJS += ./ui/ui_avplay.o
OBJS += ./ui/ui_dvbc.o
OBJS += ./ui/ui_dvbs.o
OBJS += ./ui/ui_net.o
OBJS += ./ui/ui_front.o
OBJS += ./ui/ui_manager.o
OBJS += ./ui/ui_main.o

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_mtgo -lmt_tde -lmt_sample_common
ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
DEPEND_LIBS += -lmt_jpeg
endif
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CONFIG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)


APP = sample_fac

include ${SDK_DIR}/build/script/Makefile-app.rule
