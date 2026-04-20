##################################
# ffmpeg share lib!!!
##################################
MSP_LOCAL_PATH := $(call my-dir)
include $(MSP_LOCAL_PATH)/ffmpeg/Android.mk

##################################
LOCAL_PATH := $(MSP_LOCAL_PATH)

##################################
# define header files include path
##################################
include $(SDK_LINUX_DIR)/Android_inner.def

#ifeq ($(COMMON_UNF_INCLUDE),)
#COMMON_UNF_INCLUDE := $(LOCAL_PATH)/../../common/inc
#endif

#ifeq ($(COMMON_DRV_INCLUDE),)
#COMMON_DRV_INCLUDE := $(LOCAL_PATH)/../../common/drv/inc
#endif

#ifeq ($(COMMON_API_INCLUDE),)
#COMMON_API_INCLUDE := $(LOCAL_PATH)/../../common/api/inc
#endif

##################################
# vpu static lib!!!
##################################
include $(LOCAL_PATH)/vpu_enc/vpu_enc.mk

##################################
# libmt_msp
##################################
include $(CLEAR_VARS)

MSP_CFLAGS :=
MSP_INC    :=

##################################
# modules
##################################
# adec
include $(LOCAL_PATH)/adec/adec.mk

# aenc
include $(LOCAL_PATH)/aenc/aenc.mk

# ai
include $(LOCAL_PATH)/ai/ai.mk

# ao
include $(LOCAL_PATH)/ao/ao.mk

# avplay
include $(LOCAL_PATH)/avplay/avplay.mk

# demux
include $(LOCAL_PATH)/demux/demux.mk

# frontend
include $(LOCAL_PATH)/frontend/frontend.mk

# gfx2d
include $(LOCAL_PATH)/gfx2d/gfx2d.mk

# gpu
src_gpu :=

# hdmi
include $(LOCAL_PATH)/hdmi/hdmi.mk

# i2c
include $(LOCAL_PATH)/i2c/i2c.mk

# ir
include $(LOCAL_PATH)/ir/ir.mk

# jpeg
include $(LOCAL_PATH)/jpeg/jpeg.mk

# jpge
include $(LOCAL_PATH)/jpge/jpge.mk

# mtgo
include $(LOCAL_PATH)/mtgo/mtgo.mk

# pq
include $(LOCAL_PATH)/pq/pq.mk

# sci
include $(LOCAL_PATH)/sci/sci.mk

# sync
include $(LOCAL_PATH)/sync/sync.mk

# tde
include $(LOCAL_PATH)/tde/tde.mk

# vdec
include $(LOCAL_PATH)/vdec/vdec.mk

# venc
include $(LOCAL_PATH)/venc/venc.mk

# vo
include $(LOCAL_PATH)/vo/vo.mk

# wdg
include $(LOCAL_PATH)/wdg/wdg.mk
# gpio
include $(LOCAL_PATH)/gpio/gpio.mk

# power
include $(LOCAL_PATH)/power/power.mk

# keyled
include $(LOCAL_PATH)/keyled/keyled.mk

# timer
include $(LOCAL_PATH)/timer/timer.mk

##################################
# header files
##################################
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../drv/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../pub/inc

LOCAL_C_INCLUDES += $(COMMON_UNF_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)

LOCAL_C_INCLUDES += $(MSP_INC)

#vpu!!!
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/vpu/vpuapi
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/vpu/vdi
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../drv/vfmw/video_lib_1.0
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/vpu_enc/include

##################################
# share lib
##################################
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libmt_msp

ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"$(LOCAL_MODULE)\" -DANDROID_VERSION=$(PLATFORM_VERSION)
LOCAL_CFLAGS += -DANDROID
LOCAL_CFLAGS += $(MSP_CFLAGS)

LOCAL_SRC_FILES :=	\
		$(src_adec)		\
		$(src_aenc)		\
		$(src_ai)		\
		$(src_ao)		\
		$(src_avplay)	\
		$(src_demux)	\
		$(src_frontend)	\
		$(src_gfx2d)	\
		$(src_gpu)		\
		$(src_hdmi)		\
		$(src_i2c)		\
		$(src_ir)		\
		$(src_jpeg)		\
		$(src_jpge)		\
		$(src_pq)		\
		$(src_sci)		\
		$(src_sync)		\
		$(src_tde)		\
		$(src_vdec)		\
		$(src_venc)		\
		$(src_vo)		\
		$(src_wdg)		\
		$(src_gpio)		\
		$(src_power)	\
		$(src_keyled)	\
		$(src_timer)	\

# android stub
LOCAL_SRC_FILES +=	\
		android/android.c

LOCAL_SHARED_LIBRARIES := libcutils libdl libutils
LOCAL_SHARED_LIBRARIES += libmt_common
LOCAL_SHARED_LIBRARIES += libmtffmpeg

LOCAL_STATIC_LIBRARIES := libmt_vpu_enc

include $(BUILD_SHARED_LIBRARY)
