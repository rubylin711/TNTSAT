include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

OBJS = lib2.o

LIB = lib2

include ${SDK_DIR}/build/script/Makefile-lib.rule
