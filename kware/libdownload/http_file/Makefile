include ${SDK_DIR}/build/script/base.mk

ifeq ($(MUCLIBC),)
UCLIBC=
else
UCLIBC=uclibc
endif

EXTRA_CFLAGS = -D__LINUX__

INCLUDE_PATH = -I$(MSP_DIR)/inc \
               -I$(MSP_DIR)/drv/inc \
               -I$(KWARE_DIR)/libdownload/include \
               -I$(KWARE_DIR)/libdownload/include/http_file \
               -I$(KWARE_DIR)/libmonplayer/osal \
               -I$(KWARE_DIR)/include/mtos \
               -I$(BUILDROOT_SYSROOT_USR_INC_DIR) \
               -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/neon \
               -I${SDK_DIR}/common/drv/inc \
               -I$(KWARE_DIR)/include/mtos \
               -I$(INCLUDE_DIR) \
               -I./

LIB := libhttpc

OBJS :=  \
        download_api.o \
        httpc.o \
        http_download_mini.o \
        download_manager.o \
        md5.o

HEADER_FILES=$(KWARE_DIR)/libdownload/include/http_file

include ${SDK_DIR}/build/script/Makefile-lib.rule
