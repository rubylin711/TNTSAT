include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -Iinclude                            \
	    -I$(COMMON_UNF_INCLUDE)                 \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(KWARE_DIR)/include/mtos             \
            -I${SDK_DIR}/pub/inc

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
INCLUDE_PATH+=-Iinclude/lxc_ipc
else
INCLUDE_PATH+=-Iinclude/lnx_fs
endif

EXTRA_CFLAGS = -DMT_OS_LINUX=1 
EXTRA_CFLAGS += -DMT_OS_TYPE=MT_OS_LINUX 
#EXTRA_CFLAGS += -Wno-sign-conversion 
#EXTRA_CFLAGS += -Wno-sign-compare 
#EXTRA_CFLAGS += -Wno-conversion 
#EXTRA_CFLAGS += -Wno-discarded-qualifiers 				#const be discard
#EXTRA_CFLAGS += -Wno-strict-aliasing             #force conversoin. ie.   (struc x*)&a[0];

OBJS := mt_unf_pvr.o           \
        mt_pvr_fifo.o       \
        mt_pvr_index.o      \
        mt_pvr_intf.o       \
        mt_pvr_play_ctrl.o  \
        mt_pvr_rec_ctrl.o  \
        mt_pvr_addon.o

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
OBJS += mt_pvr_lxcipc.o
endif

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_pvr.h $(MSP_API_INCLUDE)/mt_pvr_addon_api.h $(MSP_API_INCLUDE)/mt_pvr_addon_common.h $(MSP_API_INCLUDE)/mt_pvr_types.h

include $(MSP_DIR)/api/Makefile.rule
