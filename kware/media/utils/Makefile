include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)
EXTRA_CFLAGS += -D__LINUX__

ifeq ($(CONFIG_MT_DRM_SUPPORT), y)
EXTRA_CFLAGS += -DMT_DRM_SUPPORT
INCLUDE_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
EXTRA_CFLAGS += -DDRM_SMP_ENABLE
endif

INCLUDE_PATH +=                             \
                -I$(MSP_DIR)/inc


LIB := libmutils

OBJS := mutil.o mlog.o mdrv.o

HEADER_FILES                = mutil.h mlog.h
HEADER_FILES_INSTALL_SUBDIR = media/utils
include ${SDK_DIR}/build/script/Makefile-lib.rule
