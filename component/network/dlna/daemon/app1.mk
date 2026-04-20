include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I../include -I../av/include/ -I$(COMMON_UNF_INCLUDE) -I$(BUILDROOT_SYSROOT_USR_INC_DIR)


OBJS := main.o


DEPEND_LIBS = -lcupnp -lexpat -lpthread
ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR)


APP = dlna_daemon

include ${SDK_DIR}/build/script/Makefile-app.rule
