include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE) -I$(COMMON_API_INCLUDE)

OBJS = test_sanitize_lib.o test_sanitize_lib_multi_file.o

LIB = libtest_sanitize

include ${SDK_DIR}/build/script/Makefile-lib.rule
