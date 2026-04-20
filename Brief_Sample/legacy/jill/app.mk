include ${SDK_DIR}/build/script/base.mk

OBJS = test_jill.o


ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)

APP = test_jill

include ${SDK_DIR}/build/script/Makefile-app.rule
