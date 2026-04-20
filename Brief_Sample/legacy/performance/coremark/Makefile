CFG_MT_BACKTRACE_ENABLE = n
CFG_MT_SSP_ALL_ENABLE = n
CFG_MT_SSP_STRONG_ENABLE = n
CFG_MT_SSP_NONE_ENABLE = y
include ${SDK_DIR}/build/script/base.mk

FLAGS_STR = "-O2 -DPERFORMANCE_RUN=1 -DITERATIONS=1000"

FORCE_CFLAGS = -Wall -O2 -DPERFORMANCE_RUN=1 -DITERATIONS=1000 -DFLAGS_STR=\"$(FLAGS_STR)\"
ifeq ($(CONFIG_MT_LONG_64),y)
FORCE_CFLAGS += -I./linux64
else
FORCE_CFLAGS += -I./linux
endif
FORCE_CFLAGS += -I.

OBJS = core_list_join.o core_main.o core_matrix.o core_state.o core_util.o ./linux/core_portme.o

APP = coremark.exe

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
