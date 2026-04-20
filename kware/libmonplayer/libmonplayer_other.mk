include ${SDK_DIR}/build/script/base.mk

SUBDIRS := demux_mp
ifeq ($(ENABLE_LIVE555_STREAM),y)
ifeq ($(WITH_TCPIP_PROTOCOL),y)
SUBDIRS += demux_mp/live555
endif
endif
SUBDIRS += ts_seq file_playback suplayer
SUBDIRS += avplay_instance
ifeq ($(CONFIG_MT_ENABLE_MSS_PLAYER),y)
SUBDIRS += suplayer/suplayer_mss
endif
ifeq ($(CONFIG_MT_GST_SUPLAYER_SUPPORT),y)
SUBDIRS += suplayer/suplayer_gst
endif


include ${SDK_DIR}/build/script/Makefile-subdirs.rule
