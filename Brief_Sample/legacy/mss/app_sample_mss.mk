include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

# linux only
#EXTRA_CFLAGS += -Db_inclue_graphic

INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(INCLUDE_DIR)/testframe \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(INCLUDE_DIR)

INCLUDE_PATH += -I./tmss

OBJS := sample_mss.o
OBJS += mss_cmd_utils.o

OBJS += mss_kt.o
OBJS += mss_hash.o
OBJS += mss_hmac.o
OBJS += mss_crypto.o
OBJS += mss_kl.o
OBJS += mss_pka.o
OBJS += mss_pka_rsa.o
OBJS += mss_pka_ecc.o

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)
DEPEND_LIBS_PATH += -L$(EXTERNAL_BINARY_PREFIX_DIR)/lib

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
OBJS += tmss/mss_tee_client.o
DEPEND_LIBS_PATH += -L$(TEE_LIB_DIR)
DEPEND_LIBS += -lteec
endif

APP = sample_mss

include ${SDK_DIR}/build/script/Makefile-app.rule
