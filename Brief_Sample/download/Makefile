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

INCLUDE_PATH = -I$(COMMON_DIR)/inc \
          -I$(SAMPLE_DIR)/common \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(KWARE_DIR)/include/mtos \
          -I${SDK_DIR}/pub/inc \
          -I${SDK_DIR}/common/inc \
          -I$(BUILDROOT_SYSROOT_USR_INC_DIR) \
          -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/neon \
          -I${KWARE_DIR}/libdownload/include/http_file \
          -I$(KWARE_DIR)/libdownload/include \
          -I${SDK_DIR}/msp/inc 



EXTRA_CFLAGS += -D__MT_PLAYER__ \
          -I$(OPENSOURCE_INC_DIR)


OBJS = sample_download.o

ifeq ($(CFG_MT_SAMPLE), y)

EXTRA_CFLAGS += -DMT_SAMPLE_APP
LIB = libmt_sample_download

include ${SDK_DIR}/build/script/Makefile-lib.rule

else

DEPEND_LIBS = $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lmt_msp -lcrypto -lssl -lneon -lz -lhttpc

ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_LIB_DIR)
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
DEPEND_LIBS_PATH += -Wl,-rpath-link,$(OPENSOURCE_2ND_LIB_DIR)
ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
DEPEND_LIBS_PATH += -L$(TEE_LIB_DIR)
DEPEND_LIBS += -lteec
endif

endif
APP = sample_download

include ${SDK_DIR}/build/script/Makefile-app.rule
endif