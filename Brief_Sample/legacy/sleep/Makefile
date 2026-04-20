include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

OBJS = test_sleep.o

APP = test_sleep


DEPEND_LIBS = -lmt_common -lpthread

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif


include ${SDK_DIR}/build/script/Makefile-app.rule
