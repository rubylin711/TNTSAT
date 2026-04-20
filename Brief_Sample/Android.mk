LOCAL_PATH := $(call my-dir)

SAMPLE_MODULES := common android demux esplay hdmi_tsplay frontend mtest

include $(call all-named-subdir-makefiles,$(SAMPLE_MODULES))
