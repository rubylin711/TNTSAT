include ${SDK_DIR}/build/script/base.mk

#SUBDIRS = libmonplayer subtitle subtoutput cc vbi_inserte lzma teletext drm_adapter fastplayer vfs mtos mtlzswplayer
SUBDIRS = libmonplayer subtitle subtoutput cc vbi_inserte lzma teletext drm_adapter fastplayer mtos ci_route_ts


ifeq ($(CONFIG_MT_LIBDOWNLOAD),y)
SUBDIRS += libdownload
ifeq ($(CFG_MT_SDK_RELEASE),n)
SUBDIRS += netapp
endif
endif

ifeq ($(CONFIG_MT_INSTRUMENT_FUNCTIONS),y)
SUBDIRS += instrument-functions
endif

include ${SDK_DIR}/build/script/Makefile-subdirs.rule
