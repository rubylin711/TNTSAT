include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include
INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(KWARE_DIR)/include/lzma \
          -I${SDK_DIR}/opensource/zlib-1.2.11/contrib/minizip \
          -I$(BUILDROOT_SYSROOT_USR_INC_DIR)

INCLUDE_PATH += -I../include
INCLUDE_PATH += -I../include/mtos


OBJS := LzmaDecode.o
OBJS += LzmaIf.o
OBJS += encode/LzFind.o
OBJS += encode/lzma.o
OBJS += encode/LzmaEnc.o


LIB := liblzma


include ${SDK_DIR}/build/script/Makefile-lib.rule
