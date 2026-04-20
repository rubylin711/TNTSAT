LOCAL_PATH := $(call my-dir)

##################################
# define header files include path
##################################
ifeq ($(COMMON_UNF_INCLUDE),)
COMMON_UNF_INCLUDE := $(LOCAL_PATH)/../../common/inc
endif

ifeq ($(COMMON_DRV_INCLUDE),)
COMMON_DRV_INCLUDE := $(LOCAL_PATH)/../../common/drv/inc
endif

ifeq ($(COMMON_API_INCLUDE),)
COMMON_API_INCLUDE := $(LOCAL_PATH)/../../common/api/inc
endif

ifeq ($(MSP_UNF_INCLUDE),)
MSP_UNF_INCLUDE := $(LOCAL_PATH)/../../msp/inc
endif

ifeq ($(MSP_DRV_INCLUDE),)
MSP_DRV_INCLUDE := $(LOCAL_PATH)/../../msp/drv/inc
endif

ifeq ($(MSP_API_INCLUDE),)
MSP_API_INCLUDE := $(LOCAL_PATH)/../../msp/api/inc
endif

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := sample_ca_dvbplay

#ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := sample_ca_dvbplay.c

LOCAL_C_INCLUDES := $(COMMON_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_API_INCLUDE)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common

LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_SHARED_LIBRARIES += libmt_common libmt_msp libmt_msp_adaptor

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := dvbplay_robust_test

#ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := dvbplay_robust_test.c

LOCAL_C_INCLUDES := $(COMMON_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MSP_API_INCLUDE)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common

LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_SHARED_LIBRARIES += libmt_common libmt_msp libmt_msp_adaptor

include $(BUILD_EXECUTABLE)
