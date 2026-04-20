CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

FORCE_CFLAGS = -g -Wall -O2 -pedantic -fomit-frame-pointer -funroll-loops

OBJS = pi_fftcs.o fftsg_h.o

APP = pi_css5
INSTALL_EXTRA_RESOURCE = pi32768-x86.txt

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
