CFG_MT_STATIC_LINK=y
include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS) -DCI_SAS_RES -DHAVE_CISTACK

ifeq ($(CFG_MT_CI_DEV_CIMAX),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAX
endif
ifeq ($(CFG_MT_CI_DEV_CIMAXPLUS),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAXPLUS
endif
ifeq ($(CFG_MT_CI_DEV_HICI),y)
EXTRA_CFLAGS += -DMT_CI_DEV_HICI
endif

EXTRA_CFLAGS += -DMT_SAMPLE_APP
ifeq ($(CFG_MT_DSS),y)
EXTRA_CFLAGS += -DMT_SYM4_DSS
endif

#CFG_LIBC_STATIC_LINK=y


INCLUDE_PATH =	-I$(SAMPLE_DIR)/common \
		-I$(SAMPLE_DIR)/mtgo \
		-I$(COMMON_DIR)/inc \
		-I$(COMMON_DIR)/drv/inc \
		-I$(MSP_DIR)/inc \
		-I$(MSP_DIR)/api/inc \
		-I${SDK_DIR}/pub/inc/http_file \
		-I$(INCLUDE_DIR) \
		-I${SDK_DIR}/msp/inc \
		-I$(MSP_DIR)/drv/inc \
		-I${SDK_DIR}/pub/inc \
		-I${SDK_DIR}/pub/inc/suplayer \
		-I${SDK_DIR}/pub/inc/dlna \
		-I$(KWARE_DIR)/libmonplayer/include \
		-I$(KWARE_DIR)/libmonplayer/include/ts_seq \
		-I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
		-I$(KWARE_DIR)/libmonplayer/suplayer \
		-I$(KWARE_DIR)/libmonplayer/suplayer/include \
		-I$(KWARE_DIR)/ci_route_ts/include \
		-I$(SAMPLE_DIR)/ciplus \
		-I$(INCLUDE_DIR)/mtos \
		-I$(EXTERNAL_BINARY_PREFIX_DIR)/include

ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
INCLUDE_PATH += -I${SDK_DIR}/pub/inc/mlzplayer
INCLUDE_PATH += -I$(KWARE_DIR)/libmonplayer/utils
INCLUDE_PATH += -I$(OPENSOURCE_2ND_PREFIX_DIR)/gstreamer/gstreamer/subprojects/glib/glib
endif

ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)
OBJS = main_loader.o
else
OBJS = main.o
endif

DEPEND_LIBS := $(SYS_LIBS) $(MT_LIBS) -lmt_sample_common -lxml2

#################
# for maincode begin
#################
ifneq ($(CONFIG_MT_PRODUCT_LOADER),y)

DEPEND_LIBS += -lmt_sample_dvbc
DEPEND_LIBS += -lmt_sample_dvb_range
DEPEND_LIBS += -lmt_sample_dvbs
DEPEND_LIBS += -lmt_sample_nim_play
DEPEND_LIBS += -lmt_sample_pip_play
DEPEND_LIBS += -lmt_sample_dvbt
DEPEND_LIBS += -lmt_sample_j83b
DEPEND_LIBS += -lmt_sample_tsplay
DEPEND_LIBS += -lmt_sample_audio
ifeq ($(CFG_MT_DSS),y)
DEPEND_LIBS += -lmt_sample_dssplay
endif

DEPEND_LIBS += -lmt_sample_suplay -lsuplayer  -losal  -lcupnp -lmutils -lmtlzavfilter -lmonplayer_demux -lTsSeq -lPlaybackSeq -lmt_msp -lBento4 -lmt_tde -lavcodec -lavformat -lavutil -lswresample -lswscale -lexpat -lcrypto 

ifeq ($(CONFIG_MT_CHIP_SYMPHONY4),y)
DEPEND_LIBS += -lneon -lhttpc -lsmhd
endif
DEPEND_LIBS += -lmt_sample_showlogo
DEPEND_LIBS += -lmt_sample_frontpanel
DEPEND_LIBS += -lmt_sample_gpio
DEPEND_LIBS += -lmt_sample_ir
DEPEND_LIBS += -lmt_sample_i2c
DEPEND_LIBS += -lmt_sample_kadc
DEPEND_LIBS += -lmt_sample_standby_frontpanel
DEPEND_LIBS += -lmt_sample_standby_ir
DEPEND_LIBS += -lmt_sample_standby_timer
DEPEND_LIBS += -lmt_sample_power_frontpanel
DEPEND_LIBS += -lmt_sample_power_ir
DEPEND_LIBS += -lmt_sample_power_timer
DEPEND_LIBS += -lmt_sample_str_standby
DEPEND_LIBS += -lmt_sample_layerperformance
DEPEND_LIBS += -lmt_sample_text -lmt_sample_rotate_mirror -lmt_sample_loop_pic -lmt_sample_decpic -lmt_sample_scrolltext -lmt_sample_draw -lmt_sample_optlayer -lmt_sample_opacity -lmt_mtgo -lmt_tde -lmt_sample_bit
DEPEND_LIBS += -lmt_sample_dmx_cat -lmt_sample_dmx_eit -lmt_sample_dmx_nit -lmt_sample_dmx_pat -lmt_sample_dmx_pmt -lmt_sample_dmx_sdt -lmt_sample_dmx_section
DEPEND_LIBS += -lmt_sample_sound_track
DEPEND_LIBS += -lmt_sample_video_unblank
DEPEND_LIBS += -lmt_sample_split_tuner
DEPEND_LIBS += -lmt_sample_unicable
DEPEND_LIBS += -lmt_sample_disp_ratio
DEPEND_LIBS += -lmt_sample_afd
DEPEND_LIBS += -lmt_sample_disp_zoom
DEPEND_LIBS += -lmt_sample_disp_format
DEPEND_LIBS += -lmt_sample_downmix
DEPEND_LIBS += -lmt_sample_dolby_mode
DEPEND_LIBS += -lmt_sample_hdmi_cec
DEPEND_LIBS += -lmt_sample_hdcp
DEPEND_LIBS += -lmt_sample_hdmi_hdr
DEPEND_LIBS += -lmt_sample_save_config
DEPEND_LIBS += -lmt_sample_flash
DEPEND_LIBS += -lmt_sample_flash_avl
DEPEND_LIBS += -lmt_sample_wdg
DEPEND_LIBS += -lmt_sample_volume
DEPEND_LIBS += -lmt_sample_switch_track
DEPEND_LIBS += -lmt_sample_udplay
DEPEND_LIBS += -lmt_sample_cgms_a
DEPEND_LIBS += -lmt_sample_ad
DEPEND_LIBS += -lmt_sample_net
DEPEND_LIBS += -lmt_sample_version
DEPEND_LIBS += -lmt_sample_net_dump
DEPEND_LIBS += -lmt_sample_cc -lmt_cc -lmt_os
DEPEND_LIBS += -lmt_sample_tplay
DEPEND_LIBS += -lmt_sample_teletext -lmt_ttx
DEPEND_LIBS += -lmt_sample_subtitle -lmt_subtoutput -lmt_subtitle 
DEPEND_LIBS += -lmt_sample_pvr_play -lmt_sample_pvr_timeshift
#DEPEND_LIBS += -lmt_sample_pvr_rec
DEPEND_LIBS += -lmt_sample_playready
DEPEND_LIBS += -lmt_sample_ipstream
DEPEND_LIBS += -lmt_sample_gseiperf
DEPEND_LIBS += -lmt_sample_system_info
DEPEND_LIBS += -lmt_sample_heaac
DEPEND_LIBS += -lmt_sample_str_standby_timer
#DEPEND_LIBS += -lmt_sample_download
ifneq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
DEPEND_LIBS += -lssl -lcrypto
endif

ifeq ($(CONFIG_MT_DOLBY_AC4_SUPPORT),y)
#DEPEND_LIBS += -laudiodec
DEPEND_LIBS += -lmt_ac4codec
DEPEND_LIBS += -lms12_components_lib_armv8float_neon_eval
DEPEND_LIBS_PATH += -L$(EXTERNAL_BINARY_PREFIX_DIR)/lib
endif

ifeq ($(CONFIG_MT_EXT_VVID_SUPPORT),y)
DEPEND_LIBS += -lvvid
endif

DEPEND_LIBS_PATH += -L$(EXTERNAL_BINARY_PREFIX_DIR)/lib

ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
EXTRA_CFLAGS += -DMT_GSTPLAYER_ENABLE
DEPEND_LIBS += -lgstmlzplayer
DEPEND_LIBS += -lmt_sample_gstp -lmt_fastplayer -llzma -lmt_os
ifneq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
DEPEND_LIBS += -lmd_expat -lgavplay -lgstmtdec
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libcurl.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libssl.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libcrypto.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libffi.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libsoup-2.4.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgio-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libsqlite3.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libxml2.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libpsl.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgstbase-1.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libglib-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgobject-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgmodule-2.0.so
DEPEND_LIBS += $(OPENSOURCE_2ND_PREFIX_DIR)/lib/libgstreamer-1.0.so
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_PREFIX_DIR)/lib
else
DEPEND_LIBS += -lpsl -lxml2 -lcurl -lssl -lcrypto -lffi -lsoup-2.4 -lgio-2.0 -lsqlite3
DEPEND_LIBS += -lgstbase-1.0 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgstreamer-1.0
endif
endif

ifeq ($(CONFIG_MT_WIFI_SUPPORT),y)
DEPEND_LIBS += -lmt_sample_wifi -lmtwlan -lwpa_client
endif

DEPEND_LIBS += -lmt_sample_dvb_playbypid
DEPEND_LIBS += -lmt_sample_smart_card
DEPEND_LIBS += -lmt_sample_blindscan
DEPEND_LIBS += -lmt_sample_dvbs_diseqc
DEPEND_LIBS += -lmt_sample_split_search
DEPEND_LIBS += -lmt_sample_dlna
DEPEND_LIBS += -lmt_sample_satip  -lsatip

DEPEND_LIBS += -lmt_sample_cipher_rsa
DEPEND_LIBS += -lmt_sample_cipher_hash
DEPEND_LIBS += -lmt_sample_cipher_hmac
DEPEND_LIBS += -lmt_sample_cipher_hmac_sha1
DEPEND_LIBS += -lmt_sample_cipher_r2r

DEPEND_LIBS += -lmt_sample_otp
DEPEND_LIBS += -lmt_sample_usbdevice
DEPEND_LIBS += -lmt_sample_macrovision

DEPEND_LIBS += -lmt_sample_dvbs2_t2mi
DEPEND_LIBS += -lmt_sample_super_blindscan
DEPEND_LIBS += -lmt_sample_diseqc2
DEPEND_LIBS += -lmt_sample_motor
DEPEND_LIBS += -lmt_sample_temperature

DEPEND_LIBS += -lmt_sample_ac4
DEPEND_LIBS += -lmt_sample_eaa

ifeq ($(CONFIG_MT_EXT_VVID_SUPPORT),y)
EXTRA_CFLAGS += -DMT_HDMI_HDR_VIVID
DEPEND_LIBS += -lmt_sample_hdmi_hdr_vivid
DEPEND_LIBS += -lmt_sample_audio_vivid
endif

ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
DEPEND_LIBS += -lmt_jpeg
endif
ifeq ($(CONFIG_MT_DOLBY_SUPPORT),y)
DEPEND_LIBS += -ldummy
endif

ifeq ($(CONFIG_MT_MIRACAST_SUPPORT),y)
EXTRA_CFLAGS += -DMT_MIRACAST_SUPPORT
DEPEND_LIBS += -lmt_sample_miracast
endif

ifeq ($(CFG_MT_CA_IRD_ENABLE),y)
EXTRA_CFLAGS += -DMT_CA_IRD_ENABLE
DEPEND_LIBS += -lmt_sample_ca_ird -lmt_ird_cak -lmt_ird_test
DEPEND_LIBS += -lIrdetoclCloakedCAAgent -lIrdetoclWMAgent -lirdsample
DEPEND_LIBS += -L$(TEE_LIB_DIR) -lteec -lsmpc
DEPEND_LIBS += -lciplus
else

DEPEND_LIBS += -lciplus_usb
DEPEND_LIBS += -lusbcam_route_ts
DEPEND_LIBS += -lsample_usbcam

endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
DEPEND_LIBS += -lcurl -lcares -L$(TEE_LIB_DIR) -lteec -L$(EXTERNAL_BINARY_PREFIX_DIR)/drm -lMTDrmTEE
else
DEPEND_LIBS += -lcurl -lcares -L$(EXTERNAL_BINARY_PREFIX_DIR)/drm -lMTDrm
endif
endif
endif

endif
#################
# for maincode end
#################

################
# for loader begin
################
ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)

DEPEND_LIBS += -lmt_sample_frontpanel
DEPEND_LIBS += -lmt_sample_gpio
DEPEND_LIBS += -lmt_sample_ir
DEPEND_LIBS += -lmt_sample_i2c
DEPEND_LIBS += -lmt_sample_kadc

DEPEND_LIBS += -lmt_sample_text -lmt_sample_rotate_mirror -lmt_sample_loop_pic -lmt_sample_decpic -lmt_sample_scrolltext -lmt_sample_draw -lmt_sample_optlayer -lmt_sample_opacity -lmt_mtgo -lmt_tde -lmt_sample_bit
DEPEND_LIBS += -lmt_sample_dmx_cat -lmt_sample_dmx_eit -lmt_sample_dmx_nit -lmt_sample_dmx_pat -lmt_sample_dmx_pmt -lmt_sample_dmx_sdt -lmt_sample_dmx_section

DEPEND_LIBS += -lmt_sample_flash
DEPEND_LIBS += -lmt_sample_wdg
DEPEND_LIBS += -lmt_sample_net
DEPEND_LIBS += -lmt_sample_version
DEPEND_LIBS += -lmt_sample_net_dump
DEPEND_LIBS += -lmt_sample_system_info

DEPEND_LIBS += -lmt_sample_smart_card

DEPEND_LIBS += -lmt_sample_cipher_rsa
DEPEND_LIBS += -lmt_sample_cipher_hash
DEPEND_LIBS += -lmt_sample_cipher_hmac
DEPEND_LIBS += -lmt_sample_cipher_hmac_sha1
DEPEND_LIBS += -lmt_sample_cipher_r2r

DEPEND_LIBS += -lmt_sample_otp

DEPEND_LIBS += -lmt_sample_diseqc2
DEPEND_LIBS += -lmt_sample_motor
DEPEND_LIBS += -lmt_sample_temperature

ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
DEPEND_LIBS += -lmt_jpeg
endif

endif
################
# for loader end
################

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH += -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR)
endif
DEPEND_LIBS_PATH += -L$(OPENSOURCE_LIB_DIR)
DEPEND_LIBS_PATH += -L$(OPENSOURCE_2ND_LIB_DIR)

DEPEND_LIBS_PATH += -L$(OPENSOURCE_SHARED_LIB_DIR) -Wl,-rpath-link,$(OPENSOURCE_SHARED_LIB_DIR)
ifeq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
DEPEND_LIBS_PATH += -L$(BUILDROOT_SYSROOT_USR_LIB_DIR) -Wl,-rpath-link,$(BUILDROOT_SYSROOT_USR_LIB_DIR)
endif

APP := mt_sample

include ${SDK_DIR}/build/script/Makefile-app.rule
