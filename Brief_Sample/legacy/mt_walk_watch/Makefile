include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_API_INCLUDE) -I$(COMMON_UNF_INCLUDE)

OBJS = mt_walk_watch.o

DEPEND_LIBS := -lmt_common

APP = mt_walk_watch

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
