include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

EXTRA_CFLAGS += -D_MT_WITH_TALTS_

INCLUDE_PATH = -I$(SDK_DIR)/mt_inc \
          	-I$(SDK_DIR)/mt_inc/drv \
          	-Iinclude \
          	-I$(SDK_DIR)/mt_nocsapi/inc \
		-I../MT_NOCSAPIs/inc


OBJS += app_debug.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS)   -lteec  
#DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lssl -lcrypto -lMaster -lteec -ltalts.nagrata-mps-ree.release.aarch64-none-linux-gnu-gcc -lmt_nocsapi_impl 
																						
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


APP = nagra_app_debug

include ${SDK_DIR}/build/script/Makefile-app.rule
