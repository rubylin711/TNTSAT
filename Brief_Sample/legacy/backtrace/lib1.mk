include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

OBJS = lib1.o

LIB = lib1

include ${SDK_DIR}/build/script/Makefile-lib.rule
