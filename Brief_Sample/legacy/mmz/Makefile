include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE) -I$(COMMON_API_INCLUDE)

OBJS = sample_mmz.o



DEPEND_LIBS := $(SYS_LIBS) -lmt_common

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)



APP = sample_mmz

include ${SDK_DIR}/build/script/Makefile-app.rule
