include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(INCLUDE_DIR)


OBJS := listener3.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -llxc_ipc -lmt_common -lpthread

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

APP := listener3

include ${SDK_DIR}/build/script/Makefile-app.rule
