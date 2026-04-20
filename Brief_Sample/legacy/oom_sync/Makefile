include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_DIR)/inc

EXTRA_CFLAGS = -fno-builtin

OBJS = oom_sync.o

DEPEND_LIBS = -lpthread -lrt

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

APP = oom_sync

include ${SDK_DIR}/build/script/Makefile-app.rule
