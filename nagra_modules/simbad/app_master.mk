include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

EXTRA_CFLAGS += -D_MT_WITH_TALTS_

INCLUDE_PATH = -I$(SDK_DIR)/nagra_modules/simbad/include	\
		  -I$(SDK_DIR)/nagra_modules/simbad/test_inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/NOCSAPIs/NOCSAPIs_PKG_EXT-04.13.01/inc		\
		  -I$(SDK_DIR)/common/inc \
		  -I$(SDK_DIR)/msp/inc	

OBJS := nv_spr.o
OBJS += nv_debug.o
OBJS += nv_time.o
OBJS += ca_dmx.o
#OBJS += ca_icc.o
OBJS += ca_icc_s.o
OBJS += ca_os.o
OBJS += ngwm_ree.o
OBJS += nvta_tflts.o
OBJS += mt_hdmi.o
#OBJS += mt_record.o
OBJS += ott_reEnc_teset.o

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lssl -lcrypto -lMaster -lteec -ltalts.nagrata-mps-ree.release.aarch64-none-linux-gnu-gcc -lmt_nocsapi_impl -lsmpc

																						
ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
DEPEND_LIBS_PATH += -L$(MT_STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR_STRIPED)
endif

DEPEND_LIBS_PATH += -L$(SDK_DIR)/tee_bsp/arm64/usr/lib
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/simbad/prebuilt_libs/libs_talts
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/simbad/prebuilt_libs/libs_simbad

APP = nagra_talts_master

include ${SDK_DIR}/build/script/Makefile-app.rule
