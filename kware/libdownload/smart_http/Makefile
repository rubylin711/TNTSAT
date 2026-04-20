include ${SDK_DIR}/build/script/base.mk

ifeq ($(MUCLIBC),)
UCLIBC=
else
UCLIBC=uclibc
endif

EXTRA_CFLAGS = -D__LINUX__

INCLUDE_PATH = -I$(KWARE_DIR)/include \
               -I$(KWARE_DIR)/libdownload/include \
               -I$(KWARE_DIR)/libdownload/include/smart_http \
               -I$(KWARE_DIR)/libdownload/include/http_file \
               -I$(KWARE_DIR)/libmonplayer/osal \
               -I$(BUILDROOT_SYSROOT_USR_INC_DIR) \
               -I${SDK_DIR}/common/drv/inc \
               -I$(KWARE_DIR)/include/mtos \
               -I$(INCLUDE_DIR) \
               -I$(COMMON_UNF_INCLUDE)

LIB := libsmhd

OBJS :=  \
        download_group.o \
        jobs.o \
        multiqueue.o \
        smart_http_download.o 

HEADER_FILES=$(KWARE_DIR)/libdownload/include/smart_http

include ${SDK_DIR}/build/script/Makefile-lib.rule
