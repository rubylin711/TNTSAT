LOCAL_PATH := $(call my-dir)

##################################
# share lib
##################################
include $(CLEAR_VARS)

include $(SDK_LINUX_DIR)/Android_inner.def

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libmt_common

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"$(LOCAL_MODULE)\" -DANDROID_VERSION=$(PLATFORM_VERSION)

ifdef CFG_MT_LOG_LEVEL
LOCAL_CFLAGS += -DCFG_MT_LOG_LEVEL=$(CFG_MT_LOG_LEVEL)
else
LOCAL_CFLAGS += -DCFG_MT_LOG_LEVEL=1
endif

ifeq (y,$(CFG_MT_LOG_NETWORK_SUPPORT))
LOCAL_CFLAGS += -DLOG_NETWORK_SUPPORT
endif

ifeq (y,$(CFG_MT_LOG_UDISK_SUPPORT))
LOCAL_CFLAGS += -DLOG_UDISK_SUPPORT
endif

ifeq (y,$(CFG_MT_MEMMGR_SUPPORT))
LOCAL_CFLAGS += -DCMN_MMGR_SUPPORT
endif

LOCAL_SRC_FILES :=	\
        log/mpi_log.c			\
        memdev/mpi_memdev.c		\
        mmz/mpi_mmz.c			\
        module/mpi_module.c		\
        osal/mt_osal.c			\
        stat/mpi_stat.c			\
		sys/sys_common.c		\
        userproc/mpi_userproc.c

# mem
LOCAL_SRC_FILES +=	\
        mem/mpi_mem.c		\
        mem/mpi_memmap.c	\
        mem/mpi_mmgr.c		\
        mem/mpi_mutils.c

# flash
LOCAL_SRC_FILES +=	\
        flash/src/cmdline_parts.c	\
        flash/src/emmc_raw.c		\
		flash/src/mt_flash.c		\
        flash/src/nand.c			\
        flash/src/nand_raw.c		\
        flash/src/spi_raw.c

# android
LOCAL_SRC_FILES +=	\
        android/android.c	\

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../drv/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/log/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mem/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mmz/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/flash/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../pub/inc

LOCAL_SHARED_LIBRARIES := libcutils libutils

include $(BUILD_SHARED_LIBRARY)
