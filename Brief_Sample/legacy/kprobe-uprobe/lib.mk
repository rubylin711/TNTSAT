include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

OBJS = libuprobe.o

LIB = libuprobe

include ${SDK_DIR}/build/script/Makefile-lib.rule
