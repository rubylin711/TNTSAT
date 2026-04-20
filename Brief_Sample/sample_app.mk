include ${SDK_DIR}/build/script/base.mk
include $(SAMPLE_DIR)/base.mk

#################
# for maincode begin
#################
ifneq ($(CONFIG_MT_PRODUCT_LOADER),y)

SUBDIRS := dvbc
SUBDIRS += dvb_range
SUBDIRS += dvbs
SUBDIRS += nim_play
SUBDIRS += pip_play
SUBDIRS += dvbt
SUBDIRS += j83b
SUBDIRS += dvb_playpid
#SUBDIRS += download
SUBDIRS += usb_tsplay
SUBDIRS += afd
ifeq ($(CFG_MT_DSS),y)
SUBDIRS += dssplay
endif
SUBDIRS += usb_audio
SUBDIRS += usb_mpgplay
SUBDIRS += usb_showlogo
SUBDIRS += frontpanel
SUBDIRS += gpio
SUBDIRS += ir
SUBDIRS += i2c
SUBDIRS += standby_frontpanel
SUBDIRS += standby_ir
SUBDIRS += standby_timer
SUBDIRS += power_frontpanel
SUBDIRS += power_ir
SUBDIRS += power_timer
SUBDIRS += mtgo
SUBDIRS += demux
#SUBDIRS += demo_fac
SUBDIRS += str_standby
SUBDIRS += kadc
SUBDIRS += gseiperf
SUBDIRS += disp_ratio
SUBDIRS += disp_zoom
SUBDIRS += disp_format
SUBDIRS += downmix
SUBDIRS += dolby_mode
SUBDIRS += hdmi_cec
SUBDIRS += hdmi_hdcp
SUBDIRS += hdmi_hdr
SUBDIRS += save_config
SUBDIRS += flash
SUBDIRS += flash_avl
SUBDIRS += watchdog
SUBDIRS += volume
SUBDIRS += switch_track
SUBDIRS += udp_play
SUBDIRS += cgms_a
SUBDIRS += net
SUBDIRS += net_dump
SUBDIRS += cc
SUBDIRS += tplay
SUBDIRS += teletext
SUBDIRS += subtitle
SUBDIRS += pvr
SUBDIRS += wifi
SUBDIRS += system_info
SUBDIRS += heaac

SUBDIRS += smart_card
SUBDIRS += blindscan
SUBDIRS += super_blindscan
SUBDIRS += dvbs_diseqc
SUBDIRS += dvbs_diseqc2
SUBDIRS += split_freq_search
SUBDIRS += split_tuner
SUBDIRS += dlna
SUBDIRS += unicable
SUBDIRS += ad
SUBDIRS += sound_track
SUBDIRS += video_unblank
SUBDIRS += cipher_rsa
SUBDIRS += cipher_hash
SUBDIRS += cipher_hmac
SUBDIRS += cipher_hmac_sha1
SUBDIRS += cipher_r2r

SUBDIRS += otp
SUBDIRS += usb_device
SUBDIRS += temperature
SUBDIRS += dvbs2_t2mi
SUBDIRS += version

SUBDIRS += motor
SUBDIRS += playready
SUBDIRS += satip
SUBDIRS += ipstream
SUBDIRS += macrovision
ifeq ($(CFG_MT_CA_IRD_ENABLE),y)
SUBDIRS += ca_ird
else
SUBDIRS += ciplus
endif
SUBDIRS += str_standby_timer
ifeq ($(CONFIG_MT_EXT_VVID_SUPPORT),y)
SUBDIRS += hdmi_hdr_vivid
SUBDIRS += audio_vivid
endif
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
SUBDIRS += gstreamer
endif
ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
SUBDIRS += legacy
endif
ifeq ($(CONFIG_MT_MIRACAST_SUPPORT),y)
SUBDIRS += miracast
endif
SUBDIRS += ac4
SUBDIRS += eaa

endif
#################
# for maincode end
#################

################
# for loader begin
################
ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)

SUBDIRS := frontpanel
SUBDIRS += gpio
SUBDIRS += ir
SUBDIRS += i2c

SUBDIRS += mtgo
SUBDIRS += demux

SUBDIRS += kadc

SUBDIRS += flash

SUBDIRS += watchdog

SUBDIRS += net
SUBDIRS += net_dump

SUBDIRS += system_info

SUBDIRS += smart_card

SUBDIRS += dvbs_diseqc2

SUBDIRS += cipher_rsa
SUBDIRS += cipher_hash
SUBDIRS += cipher_hmac
SUBDIRS += cipher_hmac_sha1
SUBDIRS += cipher_r2r

SUBDIRS += otp

SUBDIRS += temperature

SUBDIRS += version

SUBDIRS += motor

endif
################
# for loader end
################

include ${SDK_DIR}/build/script/Makefile-subdirs.rule
