include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include
INCLUDE_PATH += -I$(INCLUDE_DIR)

LIB := libusbcam_route_ts

OBJS := src/usbcam_route_ts.o

HEADER_FILES=$(KWARE_DIR)/ci_route_ts/include/usbcam_route_ts.h
HEADER_FILES_INSTALL_SUBDIR=ci_route_ts

include ${SDK_DIR}/build/script/Makefile-lib.rule
