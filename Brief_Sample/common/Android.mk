LOCAL_PATH := $(call my-dir)

##################################
# define header files include path
##################################
#ifeq ($(COMMON_UNF_INCLUDE),)
#COMMON_UNF_INCLUDE := $(LOCAL_PATH)/../../common/inc
#endif

#ifeq ($(COMMON_DRV_INCLUDE),)
#COMMON_DRV_INCLUDE := $(LOCAL_PATH)/../../common/drv/inc
#endif

#ifeq ($(COMMON_API_INCLUDE),)
#COMMON_API_INCLUDE := $(LOCAL_PATH)/../../common/api/inc
#endif

#ifeq ($(MSP_UNF_INCLUDE),)
#MSP_UNF_INCLUDE := $(LOCAL_PATH)/../../msp/inc
#endif

#ifeq ($(MSP_DRV_INCLUDE),)
#MSP_DRV_INCLUDE := $(LOCAL_PATH)/../../msp/drv/inc
#endif

#ifeq ($(MSP_API_INCLUDE),)
#MSP_API_INCLUDE := $(LOCAL_PATH)/../../msp/api/inc
#endif

MSP_API := $(LOCAL_PATH)/../../msp/api

include $(CLEAR_VARS)

include $(SDK_LINUX_DIR)/Android_inner.def

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libmt_msp_adaptor

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"$(LOCAL_MODULE)\"
LOCAL_CFLAGS += -DANDROID

LOCAL_SRC_FILES :=  \
		android.c			\
        mt_adp_data.c       \
		mt_adp_demux.c      \
		mt_adp_frontend.c	\
		mt_adp_hdmi.c       \
        mt_adp_mpi.c        \
	mt_adp_config.c        \
        mt_adp_search.c     \
        mt_filter.c         \
        mt_psi_si.c         \
		search.c

### not support by now:
#		mt_adp_audio.c		\
#		mt_adp_pvr.c		\

#LOCAL_CFLAGS += -DMT_HDMI_RX_INSIDE
#		mt_adp_hdmi_rx.c    \

LOCAL_C_INCLUDES := $(COMMON_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_API_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_API)/mtgo/include

##################################
# osd & tde
##################################
LOCAL_SRC_FILES +=  \
        mt_adp_osd.cpp      \
        OsdManager.cpp

### graphics
#		mt_adp_mtgo.c		\

LOCAL_C_INCLUDES += $(MSP_API)/tde/include
LOCAL_C_INCLUDES += $(TOP)/external/skia/include/core
LOCAL_C_INCLUDES += $(TOP)/external/skia/include/images
LOCAL_C_INCLUDES += $(TOP)/vendor/montage/generic/hardware/arm/gralloc

# skia config
#LOCAL_CFLAGS += -DSK_SUPPORT_LEGACY_SETCONFIG
LOCAL_CFLAGS += -DSK_SUPPORT_LEGACY_BITMAP_CONFIG

LOCAL_SHARED_LIBRARIES := libcutils libutils libskia libui libgui
LOCAL_SHARED_LIBRARIES += libmt_common libmt_msp

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
