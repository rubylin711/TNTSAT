objects :=

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
objects += lxc_ipc
objects += lxc_ipcfs
endif

objects += i2c
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
objects += misc

endif
objects += frontend
objects += flash
objects += aenc
objects += demux

objects += mtgo
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),etude2 symphony6))
objects += hdmi20
else
objects += hdmi
endif
objects += sci
objects += jpeg
objects += tde
#########################################################################################
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony6))

objects += jtag
objects += sci
ifeq ($(CFG_MT_CHIP),aria)
objects += venc
endif

objects += adec
ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
objects += adec/audio_ta_service
endif

ifeq ($(CONFIG_MT_DOLBY_AC4_SUPPORT),y)
objects += adec/ext_decoder
#objects += adec/ext_decoder/ac3
objects += adec/ext_decoder/ac4
endif

ifeq ($(CONFIG_MT_EXT_VVID_SUPPORT),y)
objects += adec/ext_decoder
objects += adec/ext_decoder/vvid
endif

objects += eaa
objects += vdec
objects += avplay
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
objects += png
endif

ifeq ($(CFG_MT_CHIP),aria)
objects += jpge
endif

ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
objects += jpeg
endif

ifeq ($(CFG_MT_CHIP),aria)
objects += ai
endif

objects += ao
objects += wrap
objects += demux
objects += vo
objects += tde
#objects += gfx2d
objects += sync

ifeq ($(CFG_MT_CHIP),aria)
objects += vpu_enc
endif

objects += wdg
objects += ir
objects += pq
objects += gpio
objects += power
objects += keyled
objects += otp
objects += timer

ifeq ($(CONFIG_MT_PVR_SUPPORT),y)
objects += pvr
endif

########Cipher,HDCP,MSS start############
ifeq ($(CONFIG_MT_CIPHER_SUPPORT),y)

ifeq ($(CFG_MT_CHIP),symphony1)
objects += cipher
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony2 symphony4))
objects += mss
endif

endif

ifeq ($(CONFIG_MT_HDCPKEY_ENC_SUPPORT),y)
objects += hdcp
endif

objects += suplayer
endif   #########################################################################################

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
objects += ampshm
objects += dma
endif
objects += ciplus

########Cipher,HDCP,MSS start############
ifeq ($(CONFIG_MT_CIPHER_SUPPORT),y)

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony2 symphony4 symphony6))
objects += mss
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
objects += vss
endif

endif
########Cipher,HDCP,MSS end############

