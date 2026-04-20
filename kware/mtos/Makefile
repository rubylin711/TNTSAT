include ${SDK_DIR}/build/script/base.mk

EXTRA_CFLAGS = -DMT_LINUX_KERNEL_VERSION=\"linux-$(CFG_MT_KERNEL_VERSION)\"

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include

INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc

INCLUDE_PATH += -I../include
INCLUDE_PATH += -I../include/mtos

OBJS := drv_os.o
OBJS += mtos_debug.o
OBJS += mtos_event.o
OBJS += mtos_fifo.o
OBJS += mtos_int.o
OBJS += mtos_mem.o
OBJS += mtos_misc.o
OBJS += mtos_msg.o
OBJS += mtos_mutex.o
OBJS += mtos_printk.o
OBJS += mtos_sem.o
OBJS += mtos_task.o
OBJS += mtos_timer.o

LIB := libmt_os

HEADER_FILES=$(KWARE_DIR)/include/mtos

include ${SDK_DIR}/build/script/Makefile-lib.rule
