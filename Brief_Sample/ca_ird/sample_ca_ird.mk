CONFIG_MT_STATIC_LINK=y
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
ifeq ($(CFG_MT_DSS),y)
EXTRA_CFLAGS += -DMT_SYM4_DSS
endif
ifeq ($(CFG_MT_SAMPLE_DEBUG),y)
EXTRA_CFLAGS += -DMT_SAMPLE_CA_IRD_DEBUG
endif

INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(INCLUDE_DIR) \
          -I$(MSP_DIR)/drv/inc \
		  -I./adv_ca_irdeto_cak/inc \
		  -I./adv_ca_irdeto

OBJS = sample_ca_ird.o 

ifeq ($(CFG_MT_SAMPLE), y)

EXTRA_CFLAGS += -DMT_SAMPLE_APP
LIB = libmt_sample_ca_ird

include ${SDK_DIR}/build/script/Makefile-lib.rule

else
DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lmt_ird_cak -lmt_ird_test -lmt_mtgo -lmt_tde -lmt_jpeg -lssl -lcrypto

DEPEND_LIBS += -L${STATIC_LIB_DIR} -lIrdetoclCloakedCAAgent -lIrdetoclWMAgent
DEPEND_LIBS += -lteec -lsmpc 

ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CONFIG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)

DEPEND_LIBS_PATH += -L$(TEE_USRFS_DIR)/lib

endif

APP = sample_ca_ird

include ${SDK_DIR}/build/script/Makefile-app.rule
endif
