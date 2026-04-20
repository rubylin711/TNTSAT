include ${SDK_DIR}/build/script/base.mk

OBJS = pthread.o

DEPEND_LIBS := -pthread

APP = pthread

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
