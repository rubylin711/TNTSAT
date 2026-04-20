include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(COMMON_UNF_INCLUDE)

OBJS = usdt.o

APP = usdt

DEPEND_LIBS = -lusdt

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
