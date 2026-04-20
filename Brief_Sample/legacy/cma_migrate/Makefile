include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_DIR)/inc \

OBJS = cma_migrate.o

DEPEND_LIBS = -lmt_common -lpthread -lrt

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

APP = cma_migrate

include ${SDK_DIR}/build/script/Makefile-app.rule
