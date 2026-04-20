CFG_MT_SAMPLE_CFLAGS :=

CFG_MT_SAMPLE_CFLAGS += ${CFG_MT_BOARD_CONFIGS}

ifeq ($(CFG_MT_HDMI_RX_SUPPORT),y)
CFG_MT_SAMPLE_CFLAGS += -DMT_HDMI_RX_INSIDE
endif

#CFG_MT_DSS=y
CFG_MT_SAMPLE=y
CFG_MT_SAMPLE_DEBUG=y
CFG_MT_CA_IRD_ENABLE=n
CFG_MT_CA_NAGRA_ENABLE=n

ifeq ($(CFG_MT_CA_NAGRA_ENABLE),y)
CFG_MT_SAMPLE_CFLAGS += -DCFG_MT_SAMPLE_NAGRA
endif

SYS_LIBS := -lpthread -lrt -lm -ldl

MT_LIBS := -lmt_common

MT_LIBS += -lmt_msp

ifeq ($(CFG_MT_CA_NAGRA_ENABLE),y)
MT_LIBS += -L$(TEE_LIB_DIR) -lteec
endif

ifeq ($(CONFIG_MT_DOLBY_AC4_SUPPORT),y)
#MY_LIBS += -laudiodec
MT_LIBS += -lms12_components_lib_armv8float_neon_eval
MT_LIBS += -lmt_ac4codec
endif

ifeq ($(CONFIG_MT_EXT_VVID_SUPPORT),y)
MT_LIBS += -lvvid
endif

MT_LIBS += -lmt_sci

ifeq ($(CONFIG_MT_CIPHER_SUPPORT),y)

ifeq ($(CFG_MT_CHIP),symphony1)
MT_LIBS += -lmt_cipher
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
MT_LIBS += -lmt_mss
endif

endif

#MT_LIBS += -lmt_testfm

ifeq ($(CONFIG_MT_WIFI_SUPPORT),y)
MT_LIBS += -lmtwlan -lmtusbdev -lwpa_client
endif

ifeq ($(CONFIG_MT_MIRACAST_SUPPORT),y)
MT_LIBS += -lmtwfd
endif

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
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
MT_LIBS += -lmt_png
endif

ifneq ($(CONFIG_MT_PRODUCT_LOADER),y)
MT_LIBS += -lmt_os
endif

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
MT_LIBS += -lmt_unf_ipc_fs
MT_LIBS += -llxc_ipc
endif

ifeq ($(CONFIG_MT_VDEC_LCEVC_SUPPORT),y)
MT_LIBS += -ldvp_residuals
endif
