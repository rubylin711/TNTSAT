include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(COMMON_UNF_INCLUDE)

OBJS = libusdt.o

LIB = libusdt

include ${SDK_DIR}/build/script/Makefile-lib.rule
