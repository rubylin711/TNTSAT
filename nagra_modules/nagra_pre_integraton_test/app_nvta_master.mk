include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

EXTRA_CFLAGS += -D_MT_WITH_CAK_TEST

		  
INCLUDE_PATH = -I$(SDK_DIR)/nagra_modules/nagra_pre_integraton_test/include	\
		  -I$(SDK_DIR)/nagra_modules/nagra_pre_integraton_test/test_inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/inc \
		  -I$(SDK_DIR)/nagra_modules/MT_NOCSAPIs/NOCSAPIs/NOCSAPIs_PKG_EXT-04.13.01/inc		\
		  -I$(SDK_DIR)/common/inc \
		  -I$(SDK_DIR)/msp/inc		

OBJS := nv_spr.o
OBJS += nv_debug.o
OBJS += nv_time.o
#OBJS += ca_dmx.o
#OBJS += ca_icc.o
#OBJS += ca_os.o
OBJS += ngwm_ree.o
OBJS += nvta_tflts.o
OBJS += mt_hdmi.o
#OBJS += mt_record.o
OBJS += ca_icc_s.o
OBJS += ott_reEnc_teset.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lssl -lcrypto  -lteec  -lmt_nocsapi_impl -lsmpc
#DEPEND_LIBS += -lngwm_ree

#DEPEND_LIBS += -lsimbad_nta_214.mtg-symp6.12.3.rel1.64bits.openssl-111_20250407
#DEPEND_LIBS += -lsimbad_nta_214.mtg-symp6.12.3.rel1.64bits.openssl-111_20250908_fix-recording-saving-path-issue
DEPEND_LIBS += -lsimbad

##Debug libs
#DEPEND_LIBS += -lcak8.dual_posix.debug.aarch64-none-linux-gnu-gcc-NAK
#DEPEND_LIBS += -lcak8.dual_posix.debug.aarch64-none-linux-gnu-gcc
#DEPEND_LIBS += -lcma.dual.cma.debug.aarch64-none-linux-gnu-gcc
#DEPEND_LIBS += -ldma.dual.dma_cert.debug.aarch64-none-linux-gnu-gcc
##Release libs
DEPEND_LIBS += -lcak8.dual_posix.release.aarch64-none-linux-gnu-gcc-NAK
DEPEND_LIBS += -lcak8.dual_posix.release.aarch64-none-linux-gnu-gcc
DEPEND_LIBS += -lcma.dual.cma.release.aarch64-none-linux-gnu-gcc
DEPEND_LIBS += -ldma.dual.dma_cert.release.aarch64-none-linux-gnu-gcc


ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
DEPEND_LIBS_PATH += -L$(MT_STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR_STRIPED)
endif
DEPEND_LIBS_PATH += -L$(SDK_DIR)/tee_bsp/arm64/usr/lib
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/nagra_pre_integraton_test/prebuilt_libs/libs_caks
DEPEND_LIBS_PATH += -L$(SDK_DIR)/nagra_modules/nagra_pre_integraton_test/prebuilt_libs/libs_simbad


APP = nagra_simbad_master

include ${SDK_DIR}/build/script/Makefile-app.rule
