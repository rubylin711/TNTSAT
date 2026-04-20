include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

EXTRA_CFLAGS += -D_MT_WITH_TALTS_

INCLUDE_PATH = -I$(SDK_DIR)/mt_inc \
          	-I$(SDK_DIR)/mt_inc/drv \
          	-Iinclude \
          	-I$(SDK_DIR)/mt_nocsapi/inc \
		-I../MT_NOCSAPIs/inc

OBJS := nv_spr.o
OBJS += nv_debug.o
OBJS += ca_dmx.o
#OBJS += ca_icc.o
OBJS += ca_icc_s.o
OBJS += ca_os.o
OBJS += ngwm_ree.o
OBJS += nvta_tflts.o
OBJS += mt_hdmi.o

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lssl -lcrypto -lteec  -lsimbad_sprts_210.mtg-symp6.12.3.rel1.64bits.openssl-111_20240417 -lmt_nocsapi_impl -lsmpc

#DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lssl -lcrypto -lteec  -lsimbad_sprts_210.mtg-symp6.12.3.rel1.64bits.openssl-111_20240417 -lmt_nocsapi_impl 

ifeq ($(CFG_MT_STATIC_LINK),y)
adfa
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
DEPEND_LIBS_PATH += -L$(MT_STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
DEPEND_LIBS_PATH += -L$(MT_SHARED_LIB_DIR)
DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR_STRIPED)
endif

DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_lib/prebuilt_libs
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_lib/prebuilt_libs/talts_s6_xxx
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_lib/prebuilt_libs/libs_simbad

APP = nagra_simbad_sprts

include ${SDK_DIR}/build/script/Makefile-app.rule
