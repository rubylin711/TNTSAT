include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(INCLUDE_DIR)
INCLUDE_PATH += -I$(MSP_DIR)/inc

OBJS := listener_mpfs.o


DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -llxc_ipc -lpthread -lmt_common

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

APP := listener_mpfs
INSTALL_EXTRA_SCRIPT = listener_mpfs.sh

include ${SDK_DIR}/build/script/Makefile-app.rule
