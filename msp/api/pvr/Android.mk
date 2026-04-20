ifeq ($(CONFIG_MT_PVR_SUPPORT),y)
PVR_DIR := pvr
LOCAL_SRC_FILES += $(PVR_DIR)/unf_pvr.c
LOCAL_SRC_FILES += $(PVR_DIR)/mt_pvr_fifo.c
LOCAL_SRC_FILES += $(PVR_DIR)/mt_pvr_index.c
LOCAL_SRC_FILES += $(PVR_DIR)/mt_pvr_intf.c
LOCAL_SRC_FILES += $(PVR_DIR)/mt_pvr_play_ctrl.c
LOCAL_SRC_FILES += $(PVR_DIR)/mt_pvr_rec_ctrl.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(PVR_DIR)/include
LOCAL_C_INCLUDES += $(MSP_DIR)/api/jpeg/include
LOCAL_C_INCLUDES += $(MSP_DIR)/api/jpge/include
LOCAL_C_INCLUDES += $(MSP_DIR)/api/jpegfmw/include
LOCAL_C_INCLUDES += $(MSP_DIR)/drv/vdec
LOCAL_C_INCLUDES += $(MSP_DIR)/api/jpeg/include
endif
