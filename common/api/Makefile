include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(MSP_DIR)/api/inc
INCLUDE_PATH += -I$(COMMON_DIR)/inc
INCLUDE_PATH += -I$(COMMON_DIR)/api/inc
INCLUDE_PATH += -I$(COMMON_DIR)/drv/inc
INCLUDE_PATH += -I$(COMMON_DIR)/api/stat
INCLUDE_PATH += -I$(COMMON_DIR)/api/sys
INCLUDE_PATH += -I$(COMMON_DIR)/api/log
INCLUDE_PATH += -I$(COMMON_DIR)/api/mem
INCLUDE_PATH += -I$(COMMON_DIR)/api/mmz
INCLUDE_PATH += -I$(COMMON_DIR)/api/module
INCLUDE_PATH += -I$(COMMON_DIR)/drv/module
INCLUDE_PATH += -I$(COMMON_DIR)/drv/mem
INCLUDE_PATH += -I$(INCLUDE_DIR)
INCLUDE_PATH += -I$(MSP_DIR)/inc
INCLUDE_PATH += -I$(MSP_DIR)/drv/inc

ifeq (y,$(CFG_MT_TEST_SUPPORT))
INCLUDE_PATH += -I$(COMMON_DIR)/drv/test
endif

# mpi_mmz.c include sys/timeb.h
# android has no sys/timeb.h
EXTRA_CFLAGS = -DHAVE_FTIME -Werror


LIB := libmt_common


OBJS := ./sys/sys_common.o        \
        ./sys/mt_ftrace.o         \
        ./mmz/mpi_mmz.o           \
        ./mem/mpi_mem.o           \
        ./mem/mpi_mutils.o        \
        ./mem/mpi_memmap.o        \
        ./mem/mpi_mmgr.o          \
        ./module/mpi_module.o     \
        ./userproc/mpi_userproc.o \
        ./osal/mt_osal.o          \
        ./memdev/mpi_memdev.o     \
        ./log/mpi_log.o	          \
        ./stat/mpi_stat.o

OBJS += ./sys/mt_unf_timerfd.o
OBJS += ./sys/mt_unf_sys_monitor.o

MTFLASH_DIR := ./flash
INCLUDE_PATH += -I$(MTFLASH_DIR)/include
OBJS += $(MTFLASH_DIR)/src/mt_flash.o	\
	$(MTFLASH_DIR)/src/nand.o	\
	$(MTFLASH_DIR)/src/nand_raw.o	\
	$(MTFLASH_DIR)/src/spi_raw.o	\
	$(MTFLASH_DIR)/src/emmc_raw.o	\
	$(MTFLASH_DIR)/src/cmdline_parts.o

ifeq (y,$(CFG_MT_TEST_SUPPORT))
EXTRA_CFLAGS += -DCMN_TEST_SUPPORTED
OBJS   += ./test/mpi_test.o
endif

HEADER_FILES := $(COMMON_DIR)/inc/mt_common.h   \
                $(COMMON_DIR)/inc/mt_debug.h    \
                $(COMMON_DIR)/inc/mt_module.h   \
                $(COMMON_DIR)/inc/mt_type.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
