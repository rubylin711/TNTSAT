include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include -I$(BUILDROOT_SYSROOT_USR_INC_DIR) -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/libxml2

LIB := libdrm_adapter


OBJS := src/drm_adapter.o 

#HEADER_FILES=$(KWARE_DIR)/teletext/include/mt_unf_ttx.h 
#HEADER_FILES_INSTALL_SUBDIR=teletext

include ${SDK_DIR}/build/script/Makefile-lib.rule
