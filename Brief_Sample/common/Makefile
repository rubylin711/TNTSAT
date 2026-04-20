include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

ifeq ($(CFG_MT_DSS),y)
EXTRA_CFLAGS += -DMT_SYM4_DSS
endif
ifeq ($(CFG_MT_SAMPLE_DEBUG),y)
EXTRA_CFLAGS += -DMTADP_HDMI_DEBUG
EXTRA_CFLAGS += -DMTADP_SEARCH_DEBUG
EXTRA_CFLAGS += -DMTADP_DATA_DEBUG
EXTRA_CFLAGS += -DMTADP_MPI_DEBUG
EXTRA_CFLAGS += -DMTADP_CONFIG_DEBUG
EXTRA_CFLAGS += -DMTADP_FRONT_DEBUG
EXTRA_CFLAGS += -DMTADP_STR_DEBUG
endif
INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include
INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(COMMON_DIR)/api/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(INCLUDE_DIR)

OBJS := mt_cmd.o

OBJS += mt_adp_hdmi.o

OBJS += mt_adp_search.o
OBJS += mt_adp_data.o
OBJS += mt_adp_demux.o
OBJS += mt_adp_mpi.o
OBJS += mt_adp_config.o
OBJS += mt_adp_str.o

ifeq ($(CONFIG_MT_FRONTEND_SUPPORT),y)
OBJS += mt_adp_frontend.o
endif

ifeq ($(CONFIG_MT_PVR_SUPPORT),y)
OBJS += mt_adp_pvr.o
endif


LIB := libmt_sample_common

include ${SDK_DIR}/build/script/Makefile-lib.rule
