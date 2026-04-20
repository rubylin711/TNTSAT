include ${SDK_DIR}/build/script/base.mk

CURR_DIR := $(shell pwd)
SUBDIRS := utils

ifeq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
SUBDIRS += mtlz_avfilter
endif
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
ifeq ($(CURR_DIR)/gstreamer, $(wildcard $(CURR_DIR)/gstreamer))
SUBDIRS += gstreamer
endif
endif

include ${SDK_DIR}/build/script/Makefile-subdirs.rule
