CFG_MT_FAC_CFLAGS :=

CFG_MT_FAC_CFLAGS += ${CFG_MT_BOARD_CONFIGS}

ifeq ($(CFG_MT_HDMI_RX_SUPPORT),y)
CFG_MT_FAC_CFLAGS += -DMT_HDMI_RX_INSIDE
endif

SYS_LIBS := -lpthread -lrt -lm -ldl

MT_LIBS := -lmt_common

MT_LIBS += -lmt_msp

MT_LIBS += -lmt_sci

ifeq ($(CONFIG_MT_CIPHER_SUPPORT),y)

ifeq ($(CFG_MT_CHIP),symphony1)
MT_LIBS += -lmt_cipher
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony2 symphony4))
MT_LIBS += -lmt_mss
endif

endif

MT_LIBS += -lmt_testfm

ifeq ($(CFG_MT_HAEFFECT_BASE_SUPPORT),y)
MT_LIBS += -lmt_aef_base
endif

ifeq ($(CFG_MT_HAEFFECT_SRS_SUPPORT),y)
MT_LIBS += -lmt_aef_srs
endif

ifeq ($(CONFIG_MT_ZLIB_SUPPORT),y)
MT_LIBS += -lz
endif

MT_LIBS += -lfreetype

ifeq ($(CFG_MT_PES_SUPPORT),y)
MT_LIBS += -lmt_pes
endif

ifeq ($(CFG_MT_CAPTION_SUBT_SUPPORT),y)
MT_LIBS += -lmt_subtitle
endif

ifeq ($(CFG_MT_CAPTION_SO_SUPPORT),y)
MT_LIBS += -lmt_subtoutput
endif

ifeq ($(CFG_MT_CAPTION_TTX_SUPPORT),y)
MT_LIBS += -lmt_ttx
endif

ifeq ($(CFG_MT_VP_SUPPORT),y)
MT_LIBS += -lmt_vp -lrtp
endif

MT_LIBS += -lpng
ifeq ($(CFG_MT_CHIP),symphony4)
MT_LIBS += -lmt_png
endif

MT_LIBS += -lmt_os

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
MT_LIBS += -lmt_unf_ipc_fs
MT_LIBS += -llxc_ipc
endif
