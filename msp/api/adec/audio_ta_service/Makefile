include ${SDK_DIR}/build/script/base.mk


INCLUDE_PATH = -I$(COMMON_DIR)/inc \
			   -I$(MSP_DIR)/inc \
			   -I$(MSP_DIR)/api/inc \
			   -I$(MSP_DIR)/api/inc/tee/ \
			   -I$(MSP_DIR)/drv/inc \
			   -I${SDK_DIR}/pub/inc 


OBJS := audio_ta_service.o


DEPEND_LIBS =  -lpthread -lmt_common
ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
DEPEND_LIBS_PATH += -L$(TEE_LIB_DIR)
DEPEND_LIBS += -lteec
endif

APP = audio_ta_service

include ${SDK_DIR}/build/script/Makefile-app.rule
