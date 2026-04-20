#CFG_MT_STATIC_LINK = y
include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

OBJS = test_sanitize_main.o test_sanitize_main_multi_file.o

APP = test_sanitize

DEPEND_LIBS = -lmt_common -ltest_sanitize -lpthread

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
