include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -Iinclude -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/glib-2.0 -I$(BUILDROOT_SYSROOT_USR_LIB_DIR)/glib-2.0/include


OBJS := src/mt_bluez_api.o


LIB = libmtbt


HEADER_FILES := include/mt_bluez_api.h
HEADER_FILES_INSTALL_SUBDIR=mt_bt

include ${SDK_DIR}/build/script/Makefile-lib.rule
