include ${SDK_DIR}/build/script/base.mk



MOD_NAME := mt_msp

obj-$(CONFIG_MT_MSP) += $(MOD_NAME).o

ifeq ($(CFG_MT_CHIP),aria)
obj-$(CONFIG_MT_MSP) += gpu/
endif

obj-$(CONFIG_MT_PCMCIA_CI) += ciplus/pcmcia/mtci/
obj-$(CONFIG_MT_SMIT_USB_CAM) += ciplus/usb/smit/

#TODO

obj-$(CONFIG_MT_VO_FW) += vfmw/
obj-$(CONFIG_MT_VO_FW) += vo_fw/
ifeq ($(CFG_MT_SDK_RELEASE),n)
obj-$(CONFIG_MT_VO_SL_HDR) += vo_fw/sl_hdr/
endif

EXTRA_CFLAGS += $(CFG_MT_KMOD_CFLAGS) -Wno-date-time

EXTRA_CFLAGS += -I$(COMMON_UNF_INCLUDE) \
                -I$(COMMON_API_INCLUDE) \
                -I$(COMMON_DRV_INCLUDE) \
                -I$(MSP_UNF_INCLUDE) \
                -I$(MSP_API_INCLUDE) \
                -I$(MSP_DRV_INCLUDE) \
                -I$(COMMON_DIR)/api/log \
                -I$(COMMON_DIR)/api/stat \
                -I$(COMMON_DIR)/api/sys \
                -I$(COMMON_DIR)/api/mmz \
                -I$(COMMON_DIR)/api/mem \
                -I$(COMMON_DIR)/drv/module \
                -I$(COMMON_DIR)/drv/mem \
                -I$(COMMON_DIR)/drv/log \
                -I$(COMMON_DIR)/drv/mmz \
                -I$(COMMON_DIR)/drv/inc/mt_symphony \
                -I$(MSP_DIR)/api/mtgo/include \
                -I$(MSP_DIR)/api/tde/include \
                -I$(MSP_DIR)/api/jpge/include \
                -I$(MSP_DIR)/api/mtgo/include \
                -I$(MSP_DIR)/drv/vo/vdp_v3_0 \
                -I$(MSP_DIR)/drv/vo/vdp_v3_0/drv \
                -I$(MSP_DIR)/drv/vo/vdp_v3_0/com \
                -I$(MSP_DIR)/drv/vo/vdp_v3_0/hal/common \
                -I$(MSP_DIR)/drv/osd \
                -I$(MSP_DIR)/drv/vo_fw \
                -I$(MSP_DIR)/drv/vo_fw/inc \
                -I$(MSP_DIR)/drv/vo_fw/sl_hdr \
                -I$(MSP_DIR)/drv/vo_fw/sl_hdr/inc \
                -I$(MSP_DIR)/drv/vo_fw/sl_hdr/lut \
                -I$(MSP_DIR)/drv/vfmw/video_lib_1.0 \
                -I$(MSP_DIR)/drv/pq/pq_v3_0/include \
                -I$(MSP_DIR)/drv/pq/pq_v3_0/hal \
                -I$(MSP_DIR)/drv/jpge/include \
                -I$(MSP_DIR)/drv/adsp/adsp_v1_1/include \
                -I$(MSP_DIR)/drv/aiao/aiao_v1_1/common/include \
                -I$(MSP_DIR)/drv/aiao/aiao_v1_1/ao \
                -I$(MSP_DIR)/drv/aiao/aiao_v1_1/ai \
                -I$(MSP_DIR)/drv/snd/mt_alsa \
                -I$(MSP_DIR)/drv/mtfb/src \
                -I$(MSP_DIR)/drv/mtfb/adp/platform/montage \
                -I$(MSP_DIR)/drv/jpeg/include \
                -I$(MSP_DIR)/drv/pvr/ \
                -I$(MSP_DIR)/drv/keytable \
                -I$(MSP_DIR)/drv/keyladder \
                -I$(MSP_DIR)/drv/crypto_engine \
                -I$(MSP_DIR)/drv/vss \
                -I$(MSP_DIR)/drv/chip_verification/inc \
                -I$(INCLUDE_DIR)

ifeq ($(CONFIG_MT_HDMI_VER20),y)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/hdmi20 \
                -I$(MSP_DIR)/drv/hdmi20/hdmi20_drv \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_PCApp/platform/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_PCApp/driver/cra/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/application/cec/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/application/tx/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tx/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tx/videopath/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tx/tx_hdmi/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tx/hdcp/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tx/scdc/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/cpi/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/pll/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/tpg/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/driver/vtg/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/seq/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/video/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/log/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/time/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv//Tx_Common/library/obj/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/malloc/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/seq/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/edid/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/mtx/ \
				-I$(MSP_DIR)/drv/hdmi20/hdmi20_drv/Tx_Common/library/video/
else
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/hdmi \
                -I$(MSP_DIR)/drv/hdmi/mta_hdmi
endif

$(MOD_NAME)-y := base/msp_init.o

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
$(MOD_NAME)-y += misc/drv_sys_misc.o
$(MOD_NAME)-y += misc/mt_clk_core.o
$(MOD_NAME)-y += misc/mt_clk_apl.o
$(MOD_NAME)-y += misc/mt_analog.o
#$(MOD_NAME)-y += misc/mt_clk_testsuite.o
endif

ifeq ($(CFG_MT_CHIP),symphony4)
$(MOD_NAME)-y += misc/drv_mss_rsa.o
$(MOD_NAME)-y += misc/drv_sym4_reset.o
$(MOD_NAME)-y += misc/mt_avcpu_decompress.o
$(MOD_NAME)-y += misc/mt_avcpu_decrypt.o
$(MOD_NAME)-y += misc/mssimg/mt_rsa.o
$(MOD_NAME)-y += misc/mssimg/mt_aes.o
endif

ifeq ($(CFG_MT_CHIP),symphony6)
$(MOD_NAME)-y += misc/drv_sys_misc.o
$(MOD_NAME)-y += misc/avcpu/drv_avcpu.o
$(MOD_NAME)-y += misc/avcpu/mt_avcpu_decompress.o
$(MOD_NAME)-y += misc/avcpu/mt_avcpu_decrypt.o
$(MOD_NAME)-y += misc/mssimg/drv_mss_rsa.o
$(MOD_NAME)-y += misc/mssimg/mt_rsa.o
$(MOD_NAME)-y += misc/mssimg/mt_aes.o
endif

ifeq ($(CFG_MT_CHIP),symphony6)
$(MOD_NAME)-y += analog/drv_analog_intf.o
$(MOD_NAME)-y += analog/mt_analog.o
$(MOD_NAME)-y += analog/mt_analog_parameter.o
$(MOD_NAME)-y += analog/mt_analog_app_hdmi.o
$(MOD_NAME)-y += analog/mt_analog_app_adc.o
$(MOD_NAME)-y += analog/mt_analog_sym6.o
$(MOD_NAME)-y += analog/mt_analog_tsensor.o
$(MOD_NAME)-y += analog/board_analog_cfg.o
endif

$(MOD_NAME)-y += crm/mt_clock_common.o
#$(MOD_NAME)-y += crm/mt_clock_cfg.o
$(MOD_NAME)-y += crm/mt_mod_reset.o
$(MOD_NAME)-y += crm/mt_hal_mod_composite.o
$(MOD_NAME)-y += crm/drv_crm_intf.o
$(MOD_NAME)-y += crm/parse_cmdline.o
ifeq ($(CFG_MT_CHIP),symphony6)
$(MOD_NAME)-y += crm/mt_clock_sym6.o
$(MOD_NAME)-y += crm/mt_clock_analog_sym6.o
$(MOD_NAME)-y += crm/mt_clock_top_sym6.o
$(MOD_NAME)-y += crm/mt_mod_reset_sym6.o
else
$(MOD_NAME)-y += crm/mt_crm_stub.o
endif

ifeq ($(CONFIG_MT_HDMI_VER20),y)
$(MOD_NAME)-y += hdmi20/drv_global.o
$(MOD_NAME)-y += hdmi20/drv_compatibility.o
$(MOD_NAME)-y += hdmi20/hdmi_intf_k.o
$(MOD_NAME)-y += hdmi20/drv_hdmi.o
$(MOD_NAME)-y += hdmi20/drv_reg_proc.o
$(MOD_NAME)-y += hdmi20/hdmi20_drv.o
$(MOD_NAME)-y += hdmi20/hdmi20_drv/si_hdmitx.o
$(MOD_NAME)-y += hdmi20/hdmi20_drv/si_misc.o
$(MOD_NAME)-y += hdmi20/drv_abs_debug.o
$(MOD_NAME)-y += hdmi20/drv_hdmi_debug.o

$(MOD_NAME)-y +=  \
					hdmi20/hdmi20_drv/Tx_Common/application/cec/si_app_cec.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/cpi/si_drv_cpi.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/pll/si_drv_pll_vo.o \
					hdmi20/hdmi20_drv/Tx_Common/driver/pll/si_drv_pll_vo_cfg.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tpg/si_drv_tpg.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tx/si_drv_tx.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tx/hdcp/si_mod_tx_hdcp.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tx/scdc/si_mod_tx_scdc.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tx/tx_hdmi/si_mod_tx_hdmi.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/tx/videopath/si_mod_tx_videopath.o \
					hdmi20/hdmi20_drv/Tx_Common/driver/tx/videopath/si_mod_tx_videopath_cra.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/driver/vtg/si_drv_vtg.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/edid/si_lib_edid.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/log/si_lib_log.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/malloc/si_lib_malloc.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/mtx/si_lib_mtx.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/obj/si_lib_obj.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/seq/si_lib_seq.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/time/si_lib_time.o
$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_Common/library/video/si_lib_video.o

$(MOD_NAME)-y +=	hdmi20/hdmi20_drv/Tx_PCApp/platform/platform.o \
					hdmi20/hdmi20_drv/Tx_PCApp/platform/sii_time.o
else ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
$(MOD_NAME)-y += hdmi/hdmi_mt.o
$(MOD_NAME)-y += hdmi/hdmi_mta_hdmi.o
$(MOD_NAME)-y += hdmi/mta_hdmi/mta_hdmi_priv.o
$(MOD_NAME)-y += hdmi/drv_abs_debug.o
$(MOD_NAME)-y += hdmi/drv_global.o
$(MOD_NAME)-y += hdmi/drv_hdmi.o
$(MOD_NAME)-y += hdmi/drv_hdmi_debug.o
#$(MOD_NAME)-y += hdmi/drv_reg_proc.o
$(MOD_NAME)-y += hdmi/hdmi_intf.o
$(MOD_NAME)-y += hdmi/hdmi_intf_k.o
endif

$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_intf.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_func.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/hal_demux.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_osal.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_scd.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_bitstream.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_index.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/drv_demux_sw.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/hal_demux_regs.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/descrambler/drv_descrambler.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/descrambler/drv_descrambler_func.o
$(MOD_NAME)-$(CONFIG_MT_DEMUX) += demux/descrambler/hal_descrambler.o

ifeq ($(CONFIG_MT_VO),y)
EXTRA_CFLAGS += -DUSE_OS
#EXTRA_CFLAGS += -DDYNAMIC_RES_SUPPORT
endif
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv_disp.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv_disp_intf.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv_disp_adp2unf.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_display.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_isr.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_cast.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_buffer.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/com/drv_disp_osal.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/com/drv_disp_debug.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv_win.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv_win_intf.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_window.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_virtual.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_win_buffer.o
ifeq ($(CFG_MT_CHIP),symphony1)
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_Symphony1_reg.o
endif
ifeq ($(CFG_MT_CHIP),symphony2)
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_Symphony2_reg.o
endif
ifeq ($(CFG_MT_CHIP),symphony4)
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_Symphony4_reg.o
endif
ifeq ($(CFG_MT_CHIP),symphony6)
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_Symphony6_reg.o
endif
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/sd_enc_aria_reg.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/hd_enc_aria_reg.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_disp_bufcore.o
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/drv/drv_win_policy.o

ifneq ($(CONFIG_MT_VO_FW),y)
$(MOD_NAME)-$(CONFIG_MT_VO) += vo/vdp_v3_0/firmware/drv_disp_fw_bridge.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
EXTRA_CFLAGS += -DVIDEO_DISP_2K  -DVFW_SYMPHONY -DDISP_ON_AP_LINUX
endif
ifeq ($(CFG_MT_CHIP),symphony6)
EXTRA_CFLAGS += -DVIDEO_DISP_4K  -DVFW_SYMPHONY -DDISP_ON_AP_LINUX
endif

$(MOD_NAME)-y += pq/pq_v3_0/drv_pq.o
$(MOD_NAME)-y += pq/pq_v3_0/drv_pq_intf.o
$(MOD_NAME)-y += pq/pq_v3_0/hal/pq_hal.o

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-y += otp/drv_otp_symphony.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony6))
$(MOD_NAME)-y += otp/mt_otp_object.o otp/mt_otp_object_sym6.o otp/mt_otp_object_sym6_a0.o
endif

$(MOD_NAME)-$(CONFIG_MT_AVPLAY) += avplay/drv_avplay_intf.o

$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_alg_api_frd.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_alg_frd.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_buf_mng.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_intf.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_intf_k.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_pts_recv.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_usrdata.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_debug.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_fw_helper.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/tile_conversion.o

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2))
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/queue.o
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_fw_rpc_bridge.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 aria symphony6))
#ifeq ($(CONFIG_MT_VFMW),m)
$(MOD_NAME)-$(CONFIG_MT_VDEC) += vdec/drv_vdec_fw_bridge.o
#endif
endif

ifeq ($(CFG_MT_CHIP),aria)
EXTRA_CFLAGS += -I$(MSP_DIR)/api/vpu_enc
$(MOD_NAME)-y += venc/drv_venc_osal.o
$(MOD_NAME)-y += venc/drv_venc_intf.o
$(MOD_NAME)-y += venc/drv_venc.o
$(MOD_NAME)-y += venc/drv_venc_efl.o
$(MOD_NAME)-y += venc/drv_omxvenc.o
$(MOD_NAME)-y += venc/drv_omxvenc_efl.o
$(MOD_NAME)-y += venc/drv_venc_buf_mng.o
$(MOD_NAME)-y += venc/drv_venc_queue_mng.o
$(MOD_NAME)-y += venc/drv_venc_proc.o
$(MOD_NAME)-y += venc/hal_venc.o
endif

ifeq ($(CONFIG_MT_HAS_HW_VPASS),y)
$(MOD_NAME)-y += vpass/vpss_v3_0/drv_vpss_intf_k.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_alg_ratio.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_alg_rwzb.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_alg_scd.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_ctrl.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_fb.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_img.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_in.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_info.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_instance.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_mt.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_osal.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_rwzb.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_src.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_sttinf.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_wbc.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_reg_3798m.o
$(MOD_NAME)-y += vpass/vpss_v3_0/vpss_hal_3798m.o
else
$(MOD_NAME)-y += vpass/vpss_fake/drv_vpss_intf_k.o
endif

ifeq ($(CFG_MT_CHIP),aria)
EXTRA_CFLAGS += -DARIA_LINUX
else
EXTRA_CFLAGS += -DSYMPHONY_LINUX
endif

$(MOD_NAME)-$(CONFIG_MT_SYNC) += sync/drv_sync.o
$(MOD_NAME)-$(CONFIG_MT_SYNC) += sync/drv_sync_intf.o
$(MOD_NAME)-$(CONFIG_MT_SYNC) += sync/drv_sync_intf_k.o
$(MOD_NAME)-$(CONFIG_MT_SYNC) += sync/drv_sync_reg.o

EXTRA_CFLAGS += -DMT_SND_CAST_SUPPORT
EXTRA_CFLAGS += -DMT_I2S0_SUPPORT
ifeq ($(CFG_MT_SND_ALSA_AO_SUPPORT),y)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/aiao/aiao_v1_1/$(ALSA_DIR)
EXTRA_CFLAGS += -DMT_ALSA_AO_SUPPORT
endif
ifeq ($(CFG_MT_SND_ALSA_I2S_ONLY_SUPPORT),y)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/aiao/aiao_v1_1/$(ALSA_DIR)
endif
ifeq ($(CFG_MT_SND_ALSA_HDMI_ONLY_SUPPORT),y)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/aiao/aiao_v1_1/$(ALSA_DIR)
endif
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/drv_aiao_module.o
#AO SUPPORT (default)
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao_op.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao_track.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao_aef.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao_cast.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/hal_aoe_func.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/hal_aoe.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/hal_cast.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/ao/drv_ao_intf.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/common/src/audio_util.o
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/common/src/hal_aiao.o
$(MOD_NAME)-$(CONFIG_MT_AIN) += aiao/aiao_v1_1/common/src/aud_in_aria_reg.o
ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/common/src/aud_out_aria_reg.o
else
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_AIAO) += aiao/aiao_v1_1/common/src/aud_out_symphony_reg.o
endif
endif
#AI SUPPORT (default)
$(MOD_NAME)-$(CONFIG_MT_AIN) += aiao/aiao_v1_1/ai/drv_ai_intf.o
$(MOD_NAME)-$(CONFIG_MT_AIN) += aiao/aiao_v1_1/ai/drv_ai.o

ifeq ($(CONFIG_MT_AIN),y)
EXTRA_CFLAGS += -DMT_AUDIO_AI_SUPPORT
endif

$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/adec_k.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/buf/adec_buf_k.o
ifeq ($(CFG_MT_MCE_SUPPORT),y)
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/ha_mp3dec_wrap.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/mp3_decoder.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/bits_op.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/dct_core.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/memory_op.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/dequant.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/huffman.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/huffman_rom.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/dec_layer12_sample.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/imdct.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/mp3_rom.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/data_decode.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/stereo.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/synthesis.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/trig_rom.o
$(MOD_NAME)-$(CONFIG_MT_ADEC) += adec/mp3dec/armgcc/asmpoly_gcc.o
endif

ifeq ($(CFG_ANDROID_PRODUCT),y)
EXTRA_CFLAGS += -I$(MT_KERNEL_DIR)/drivers/staging/android
EXTRA_CFLAGS += -DCFG_MTFB_FENCE_SUPPORT
endif
$(MOD_NAME)-$(CONFIG_MT_OPTM) += mtfb/adp/platform/montage/optm_mtfb.o
ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-$(CONFIG_MT_OPTM) += mtfb/adp/platform/montage/optm_hal.o
endif
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
$(MOD_NAME)-$(CONFIG_MT_OPTM) += mtfb/adp/platform/montage/optm_hal_sym.o
endif
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_dmabuf.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_fence.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_main.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_main_k.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_scrolltext.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_snapshot.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/src/mtfb_comm.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/adp/src/mtfb_blit.o
$(MOD_NAME)-$(CONFIG_MT_MTFB) += mtfb/adp/src/mtfb_drv_adpt.o

ifeq ($(CONFIG_MT_MTFB),y)
EXTRA_CFLAGS += -DCFG_MTFB_SUPPORT
endif

$(MOD_NAME)-$(CONFIG_MT_OSD) += osd/drv_osd_intf.o

#EXTRA_CFLAGS += -DCFG_MT_TDE_CSCTMPBUFFER_SIZE=$(CFG_MT_TDE_CSCTMPBUFFER_SIZE)
EXTRA_CFLAGS += -DCFG_MT_TDE_CSCTMPBUFFER_SIZE=0x20000
EXTRA_CFLAGS += -DMT_TDE_MEMCOUNT_SUPPORT=0
EXTRA_CFLAGS += -DMT_TDE_BUFFER=0x20000
EXTRA_CFLAGS += -DCONFIG_GFX_STB_SDK
EXTRA_CFLAGS += -DCONFIG_TDE_VERSION_DISABLE
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_osr.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_osr_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_handle_k.o
#$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_hal_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_hal_aria_vsb.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_hal_aria_tool.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_osilist_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_osictl_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_proc_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/wmalloc_k.o
$(MOD_NAME)-$(CONFIG_MT_TDE) += tde/tde_adp.o

$(MOD_NAME)-$(CONFIG_MT_PNG) += png/png_osr.o

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_WDG_SUPPORT) += wdg/drv_sym4_wdg.o
else
$(MOD_NAME)-$(CONFIG_MT_WDG_SUPPORT) += wdg/drv_wdg.o
endif

ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-y += timer/drv_timer.o
$(MOD_NAME)-y += timer/drv_sys_timer.o
else
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
$(MOD_NAME)-y += timer/drv_sym4_timer.o
$(MOD_NAME)-y += timer/drv_sym4_timer_intf.o
else
$(MOD_NAME)-y += timer/drv_symp_timer.o
endif
endif
$(MOD_NAME)-y += timer/drv_kn_timer.o

$(MOD_NAME)-y += dma/dma_aria.o
$(MOD_NAME)-y += dma/hal_dma.o

$(MOD_NAME)-$(CONFIG_MT_SPDMA) += spdma/hal_spdma.o

$(MOD_NAME)-y += jpeg/src/jpg_osr.o
$(MOD_NAME)-y += jpeg/src/jpg_proc.o
$(MOD_NAME)-y += jpeg/src/jpg_hal.o
$(MOD_NAME)-y += jpeg/src/jpg_suspend.o

ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-y += jpge/src/jpge_osr.o
$(MOD_NAME)-y += jpge/src/jpge_osal.o
$(MOD_NAME)-y += jpge/src/jpge_hal.o
endif

#no need, use linux standard ir now
#$(MOD_NAME)-y += ir/drv_ir_intf.o

$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/drv_frontend.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/frontend_intf.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/public/drv_gpio.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/public/gpio_i2c_concerto.o
ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/ca8k/port_m88ca8k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/ca8k/mt_fe_dmd_ca8k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/ca8k/mt_fe_dmd_ca8k_tuner.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/ca8k/mt_fe_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND) += frontend/ca8k/mt_fe_tn_tc6800.o
else
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE) += frontend/cs8000_cab/port_m88cs8k_cab.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE) += frontend/cs8000_cab/mt_fe_dmd_cs8000_cab.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE) += frontend/cs8000_cab/mt_fe_dmd_cs8000_cab_tuner.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE) += frontend/cs8000_cab/mt_fe_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC2800) += frontend/cs8000_cab/mt_fe_cab_tn_tc2800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC3800) += frontend/cs8000_cab/mt_fe_cab_tn_tc3800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC6800) += frontend/cs8000_cab/mt_fe_cab_tn_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TDA18250) += frontend/cs8000_cab/mt_fe_cab_tn_TDA18250.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_R836) += frontend/cs8000_cab/mt_fe_cab_tn_R836.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TDA18250) += frontend/cs8000_cab/TDA18250/tmddTDA18250.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TDA18250) += frontend/cs8000_cab/TDA18250A/tmbslTDA18250A/tmbslTDA18250A.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TDA18250) += frontend/cs8000_cab/TDA18250A/tmbslTDA18250A/tmbslTDA18250A_Advanced.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL203) += frontend/cs8000_cab/mt_fe_cab_tn_MxL203.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL608) += frontend/cs8000_cab/MXL608/MxL608_OEM_Drv.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL608) += frontend/cs8000_cab/MXL608/MxL608_TunerApi.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL608) += frontend/cs8000_cab/MXL608/MxL608_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL608) += frontend/cs8000_cab/MXL608/MxL608_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL608) += frontend/cs8000_cab/mt_fe_cab_tn_MxL608.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL203) += frontend/cs8000_cab/MxL203/MxL203RF.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL203) += frontend/cs8000_cab/MxL203/MxL203RF_API.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_MXL203) += frontend/cs8000_cab/MxL203/MxL_User_Define.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE) += frontend/cs8000_sat/port_m88cs8k_sat.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE) += frontend/cs8000_sat/mt_fe_dmd_cs8000_sat.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE) += frontend/cs8000_sat/mt_fe_cs8000_sat_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022) += frontend/cs8000_sat/mt_fe_sat_tn_montage_ts2022.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS6011) += frontend/cs8000_sat/mt_fe_sat_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE) += frontend/cs8000_sat/port_m88cs8k_sat.o

ifeq ($(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC3800),y)
EXTRA_CFLAGS += -DCFG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC3800_SUPPORT
endif

ifeq ($(CONFIG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC6800),y)
EXTRA_CFLAGS += -DCFG_MT_FRONTEND_DMD_CS8K_CABLE_TN_TC6800_SUPPORT
endif

ifeq ($(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022),y)
EXTRA_CFLAGS += -DCFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022_SUPPORT
endif

ifeq ($(CONFIG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS6011),y)
EXTRA_CFLAGS += -DCFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS6011_SUPPORT
endif

endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K) += frontend/m88dd3k/mt_fe_dmd_dd3k.o
#$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K) += frontend/m88dd3k/mt_fe_dmd_dd3k_tuner.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K) += frontend/m88dd3k/mt_fe_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_TC6800) += frontend/m88dd3k/mt_fe_tn_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_TC3800) += frontend/m88dd3k/mt_fe_tn_tc3800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL603) += frontend/m88dd3k/mt_fe_tn_MxL603.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL603) += frontend/m88dd3k/MxL6/MxL601_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL603) += frontend/m88dd3k/MxL6/MxL601_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL603) += frontend/m88dd3k/MxL6/MxL603_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL603) += frontend/m88dd3k/MxL6/MxL603_TunerSpurTable.o

$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL608) += frontend/m88dd3k/mt_fe_tn_MxL608.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL608) += frontend/m88dd3k/MxL6/MxL608_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K_TN_MXL608) += frontend/m88dd3k/MxL6/MxL608_TunerSpurTable.o

$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DD3K) += frontend/m88dd3k/port_dd3k.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TENOR) += frontend/tc6930/port_tc6930.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TENOR) += frontend/tc6930/mt_fe_dmd_tc6930.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TENOR) += frontend/tc6930/mt_fe_i2c_tc6930.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TENOR) += frontend/tc6930/mt_fe_tn_tc6930.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/port_sony_cxd2856.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sony_common.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sony_demod.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sony_integ.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sony_stopwatch_port.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_demod_dvbc2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_demod_dvbc2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_demod_dvbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_demod_dvbc_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_integ_dvbc2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_cable/sony_integ_dvbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/sony_demod_dvbs_s2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/sony_demod_dvbs_s2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/sony_integ_dvbs_s2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_algo.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_bt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_cs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_fs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_pm.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_ss.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_utility.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_dvbs_s2_tune_srs_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_sat_device_ctrl_detect_cw_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_sat/blindscan/sony_demod_sat_device_ctrl_find1ub_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_demod_dvbt2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_demod_dvbt2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_demod_dvbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_demod_dvbt_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_integ_dvbt2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_integ_dvbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/dvb_terr/sony_integ_dvbt_t2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/i2c/sony_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_cable/sony_demod_isdbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_cable/sony_demod_isdbc_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_cable/sony_integ_isdbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_demod_isdbs3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_demod_isdbs3_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_demod_isdbs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_demod_isdbs_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_integ_isdbs3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_sat/sony_integ_isdbs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_terr/sony_demod_isdbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_terr/sony_demod_isdbt_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/isdb_terr/sony_integ_isdbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/j83b/sony_demod_j83b.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/j83b/sony_demod_j83b_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/j83b/sony_integ_j83b.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/lib/sony_math.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_demod_sat_device_ctrl.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_diseqc_command_1_0.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_diseqc_command_1_1.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_diseqc_command_1_2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_diseqc_command_2_0.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_diseqc_command_2_2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_integ_sat_device_ctrl.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_singlecable2_command.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/sat_device_ctrl/sony_singlecable_command.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/sat_horus3a/sony_tuner_horus3a.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/sat_horus3a/refcode/sony_horus3a.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_ascot3/sony_tuner_ascot3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_ascot3/refcode/sony_ascot3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_sat_helene/sony_tuner_helene.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_sat_helene/refcode/sony_helene.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_MXL608) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_MxL608/sony_tuner_MxL608.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_MXL608) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_MxL608/MxL608_OEM_Drv.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_MXL608) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_MxL608/MxL608_TunerApi.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_MXL608) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_MxL608/MxL608_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_MXL608) += frontend/sony_cxd2856/sony_cxd2856/tuner/terr_cable_MxL608/MxL608_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R850) += frontend/sony_cxd2856/sony_cxd2856/tuner/R850/sony_tuner_r850.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R850) += frontend/sony_cxd2856/sony_cxd2856/tuner/R850/R850.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R858C) += frontend/sony_cxd2856/sony_cxd2856/tuner/R858C/sony_tuner_r858.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R858C) += frontend/sony_cxd2856/sony_cxd2856/tuner/R858C/R858.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R836) += frontend/sony_cxd2856/sony_cxd2856/tuner/R836/sony_tuner_r836.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_R836) += frontend/sony_cxd2856/sony_cxd2856/tuner/R836/R836.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_TC6800) += frontend/sony_cxd2856/sony_cxd2856/tuner/tc6800/sony_tuner_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD2856_TN_TC6800) += frontend/sony_cxd2856/sony_cxd2856/tuner/tc6800/mt_fe_tn_tc6800.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/port_sony_cxd.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sony_common.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sony_demod.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sony_integ.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sony_stopwatch_port.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_demod_dvbc2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_demod_dvbc2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_demod_dvbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_demod_dvbc_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_integ_dvbc2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_cable/sony_integ_dvbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/sony_demod_dvbs_s2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/sony_demod_dvbs_s2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/sony_integ_dvbs_s2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_algo.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_bt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_cs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_fs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_pm.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_subseq_ss.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_blindscan_utility.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_dvbs_s2_tune_srs_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_sat_device_ctrl_detect_cw_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_sat/blindscan/sony_demod_sat_device_ctrl_find1ub_seq.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_demod_dvbt2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_demod_dvbt2_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_demod_dvbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_demod_dvbt_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_integ_dvbt2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_integ_dvbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/dvb_terr/sony_integ_dvbt_t2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/i2c/sony_i2c.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_cable/sony_demod_isdbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_cable/sony_demod_isdbc_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_cable/sony_integ_isdbc.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_demod_isdbs3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_demod_isdbs3_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_demod_isdbs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_demod_isdbs_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_integ_isdbs3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_sat/sony_integ_isdbs.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_terr/sony_demod_isdbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_terr/sony_demod_isdbt_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/isdb_terr/sony_integ_isdbt.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/j83b/sony_demod_j83b.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/j83b/sony_demod_j83b_monitor.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/j83b/sony_integ_j83b.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/lib/sony_math.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_demod_sat_device_ctrl.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_diseqc_command_1_0.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_diseqc_command_1_1.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_diseqc_command_1_2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_diseqc_command_2_0.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_diseqc_command_2_2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_integ_sat_device_ctrl.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_singlecable2_command.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/sat_device_ctrl/sony_singlecable_command.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/sat_horus3a/sony_tuner_horus3a.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/sat_horus3a/refcode/sony_horus3a.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/terr_cable_ascot3/sony_tuner_ascot3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/terr_cable_ascot3/refcode/sony_ascot3.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/terr_cable_sat_helene/sony_tuner_helene.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY) += frontend/sony_cxd_family/family_source/tuner/terr_cable_sat_helene/refcode/sony_helene.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_MXL608) += frontend/sony_cxd_family/family_source/tuner/terr_cable_MxL608/sony_tuner_MxL608.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_MXL608) += frontend/sony_cxd_family/family_source/tuner/terr_cable_MxL608/MxL608_OEM_Drv.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_MXL608) += frontend/sony_cxd_family/family_source/tuner/terr_cable_MxL608/MxL608_TunerApi.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_MXL608) += frontend/sony_cxd_family/family_source/tuner/terr_cable_MxL608/MxL608_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_MXL608) += frontend/sony_cxd_family/family_source/tuner/terr_cable_MxL608/MxL608_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R850) += frontend/sony_cxd_family/family_source/tuner/R850/sony_tuner_r850.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R850) += frontend/sony_cxd_family/family_source/tuner/R850/R850.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R858C) += frontend/sony_cxd_family/family_source/tuner/R858C/sony_tuner_r858.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R858C) += frontend/sony_cxd_family/family_source/tuner/R858C/R858.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R836) += frontend/sony_cxd_family/family_source/tuner/R836/sony_tuner_r836.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_R836) += frontend/sony_cxd_family/family_source/tuner/R836/R836.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_TC6800) += frontend/sony_cxd_family/family_source/tuner/tc6800/sony_tuner_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_SONY_CXD_FAMILY_TN_TC6800) += frontend/sony_cxd_family/family_source/tuner/tc6800/mt_fe_tn_tc6800.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_dmd_dm6k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_dmd_dm6k_C.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_dmd_dm6k_S_S2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_dmd_dm6k_T2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_dmd_dm6k_T.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/mt_fe_i2c_dm6k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_TS2022) += frontend/dm6000/mt_fe_tn_montage_ts2022.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_TS6011) += frontend/dm6000/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_MXL603) += frontend/dm6000/mt_fe_tn_MxL603.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_RDA5815S) += frontend/dm6000/mt_fe_tn_rda5815s.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_TC3800) += frontend/dm6000/mt_fe_tn_tc3800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_TC6800) += frontend/dm6000/mt_fe_tn_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_MXL601) += frontend/dm6000/MxL6/MxL601_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_MXL601) += frontend/dm6000/MxL6/MxL601_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_MXL603) += frontend/dm6000/MxL6/MxL603_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K_TN_MXL603) += frontend/dm6000/MxL6/MxL603_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DM6K) += frontend/dm6000/port_dm6k.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_RS6060) += frontend/rs6060/mt_fe_dmd_rs6060.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_RS6060) += frontend/rs6060/mt_fe_i2c_rs6060.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_RS6060) += frontend/rs6060/mt_fe_tn_montage_rs6060.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_RS6060) += frontend/rs6060/port_rs6060.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103) += frontend/ds6103/mt_fe_dmd_ds6103.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103) += frontend/ds6103/mt_fe_it_ds6103.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103) += frontend/ds6103/mt_fe_i2c_ds6103.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103) += frontend/ds6103/port_ds6103.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103_TN_TS6011) += frontend/ds6103/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103_TN_RDA5815M) += frontend/ds6103/mt_fe_tn_rda_5815m.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103_TN_RDA5815S) += frontend/ds6103/mt_fe_tn_rda_5815s.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103_TN_AV2012) += frontend/ds6103/mt_fe_tn_av2012.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6103_TN_AV2018) += frontend/ds6103/mt_fe_tn_av2018.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113) += frontend/ds6113/mt_fe_dmd_ds6113.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113) += frontend/ds6113/mt_fe_it_ds6113.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113) += frontend/ds6113/mt_fe_i2c_ds6113.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113) += frontend/ds6113/port_ds6113.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113_TN_TS6011) += frontend/ds6113/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113_TN_RDA5815M) += frontend/ds6113/mt_fe_tn_rda_5815m.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113_TN_RDA5815S) += frontend/ds6113/mt_fe_tn_rda_5815s.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113_TN_AV2012) += frontend/ds6113/mt_fe_tn_av2012.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_DS6113_TN_AV2018) += frontend/ds6113/mt_fe_tn_av2018.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/av2020.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/QM1D1C0045.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/raon_rf_mtv600.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/RDA5812.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/RDA5815.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/RDA5815M.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/S305.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/sharp6306.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/sharp6903.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/TP5001.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_TP5001) += frontend/tp5001/port_tp5001.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/RDA5812.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/RDA5815M.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/ISharpTuner.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/HDIC2501.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_HD2502) += frontend/hd2502/port_hd2502.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony2))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k_B.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k_C.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k_S_S2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k_T2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_dmd_ct8k_T.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/mt_fe_i2c_ct8k.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_TS2022) += frontend/ct8k/mt_fe_tn_montage_ts2022.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_TS6011) += frontend/ct8k/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_RDA5815M) += frontend/ct8k/mt_fe_tn_rda5815m.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL603) += frontend/ct8k/mt_fe_tn_MxL603.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_RDA5815S) += frontend/ct8k/mt_fe_tn_rda5815s.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_TC3800) += frontend/ct8k/mt_fe_tn_tc3800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_TC6800) += frontend/ct8k/mt_fe_tn_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL601) += frontend/ct8k/MxL6/MxL601_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL601) += frontend/ct8k/MxL6/MxL601_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL603) += frontend/ct8k/MxL6/MxL603_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL603) += frontend/ct8k/MxL6/MxL603_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CT8K) += frontend/ct8k/port_ct8k.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony4 symphony6))
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/mt_fe_dmd_cs8800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/mt_fe_dmd_cs8800_B.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/mt_fe_dmd_cs8800_C.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/mt_fe_dmd_cs8800_S_S2.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/mt_fe_i2c_cs8800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_TS6011) += frontend/cs8800/mt_fe_tn_montage_ts6011.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL603) += frontend/cs8800/mt_fe_tn_MxL603.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_RDA5815S) += frontend/cs8800/mt_fe_tn_rda5815s.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_TC6800) += frontend/cs8800/mt_fe_tn_tc6800.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL601) += frontend/cs8800/MxL6/MxL601_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL601) += frontend/cs8800/MxL6/MxL601_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL603) += frontend/cs8800/MxL6/MxL603_TunerCfg.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL603) += frontend/cs8800/MxL6/MxL603_TunerSpurTable.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800_TN_RT720) += frontend/cs8800/mt_fe_tn_RT710.o
$(MOD_NAME)-$(CONFIG_MT_FRONTEND_DMD_CS8800) += frontend/cs8800/port_cs8800.o
endif

$(MOD_NAME)-y += gpio/drv_gpio_intf.o
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-y += power/drv_standby_intf.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-y += ipc/ipc_g.o
$(MOD_NAME)-y += ipc/ipc_symphony.o
$(MOD_NAME)-y += ipc/ipc_secure_plugin.o
endif

ifeq ($(CONFIG_MT_MTEST),y)
$(MOD_NAME)-y += mtest/drv_mtest.o
endif

# symphony4/6: no rpc
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2))
$(MOD_NAME)-y += rpc/rpc_k.o
$(MOD_NAME)-y += rpc/rpc_proxy.o
endif

ifeq ($(CONFIG_MT_PVR_SUPPORT),y)
$(MOD_NAME)-$(CONFIG_MT_PVR) += pvr/drv_pvr_intf.o
$(MOD_NAME)-$(CONFIG_MT_PVR) += pvr/fw_api_pvr.o
endif

ifeq ($(CONFIG_MT_SECURITY),y)
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE) += crypto_engine/drv_ce_common.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE) += crypto_engine/drv_ce_intf.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/drv_ades.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/drv_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/drv_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/drv_sha.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/drv_ts.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/hw_ades.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/hw_ce_common.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/hw_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/hw_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM2) += crypto_engine/sym2/hw_sha.o
ifeq ($(CONFIG_MT_CRYPTOENGINE_SYM4),y)
EXTRA_CFLAGS += -DLINUX
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/crypto_engine/sym4/lib \
		-I$(MSP_DIR)/drv/crypto_engine/sym4/m2m \
		-I$(MSP_DIR)/drv/crypto_engine/sym4/pka
endif
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/drv_m2m.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/drv_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/drv_mod.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/drv_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/hw_m2m.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_cipher.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_ghash.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_mac.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_ts.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_bgc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_gcm.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_hash.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_mgr.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/m2m/m2m_desc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/pka/pka_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/pka/pka_mod.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/pka/pka_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/pka/pka_sem.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM4) += crypto_engine/sym4/lib/mt_utils.o
ifeq ($(CONFIG_MT_CRYPTOENGINE_SYM6),y)
EXTRA_CFLAGS += -DLINUX
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/crypto_engine/sym6/lib \
		-I$(MSP_DIR)/drv/crypto_engine/sym6/m2m \
		-I$(MSP_DIR)/drv/crypto_engine/sym6/pka
endif
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/drv_m2m.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/drv_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/drv_mod.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/drv_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/hw_m2m.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_cipher.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_ghash.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_mac.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_ts.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_bgc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_gcm.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_hash.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_mgr.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/m2m/m2m_desc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/pka/pka_ecc.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/pka/pka_mod.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/pka/pka_rsa.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/pka/pka_sem.o
$(MOD_NAME)-$(CONFIG_MT_CRYPTOENGINE_SYM6) += crypto_engine/sym6/lib/mt_utils.o
$(MOD_NAME)-$(CONFIG_MT_KEYLADDER) += keyladder/drv_keyladder.o
$(MOD_NAME)-$(CONFIG_MT_KEYLADDER) += keyladder/drv_keyladder_intf.o
ifeq ($(CONFIG_MT_CHIP_SYMPHONY2),y)
$(MOD_NAME)-$(CONFIG_MT_KEYLADDER) += keyladder/sym2/hw_keyladder.o
else ifeq ($(CONFIG_MT_CHIP_SYMPHONY4),y)
$(MOD_NAME)-$(CONFIG_MT_KEYLADDER) += keyladder/sym4/hw_keyladder.o
else ifeq ($(CONFIG_MT_CHIP_SYMPHONY6),y)
$(MOD_NAME)-$(CONFIG_MT_KEYLADDER) += keyladder/sym6/hw_keyladder.o
endif
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE) += keytable/drv_kt.o
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE) += keytable/drv_kt_common.o
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE) += keytable/drv_kt_intf.o
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE_SYM2) += keytable/sym2/hw_kt.o
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE_SYM4) += keytable/sym4/hw_kt.o
$(MOD_NAME)-$(CONFIG_MT_KEYTABLE_SYM6) += keytable/sym6/hw_kt.o
$(MOD_NAME)-$(CONFIG_MT_IPCS) += ipcs/drv_ipcs_io.o
$(MOD_NAME)-$(CONFIG_MT_IPCS) += ipcs/ipcs_symphony.o
$(MOD_NAME)-$(CONFIG_MT_CERT) += cert/certip.o
$(MOD_NAME)-$(CONFIG_MT_CERT) += cert/drv_cert_intf.o
$(MOD_NAME)-$(CONFIG_MT_VSS) += vss/drv_vss_intf.o
$(MOD_NAME)-$(CONFIG_MT_VSS) += vss/drv_vss_base_intf.o
$(MOD_NAME)-$(CONFIG_MT_VSS_MB) += vss/drv_vss_mb_intf.o
$(MOD_NAME)-$(CONFIG_MT_VSS_MB) += vss/hw_mailbox.o
$(MOD_NAME)-$(CONFIG_MT_OPC) += opc/drv_opc_intf.o
$(MOD_NAME)-$(CONFIG_MT_OPC) += opc/hw_opc.o
#symphony1 only
$(MOD_NAME)-$(CONFIG_MT_CIPHER_SCPU) += cipher_scpu/drv_cipher_io.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4 symphony6))
$(MOD_NAME)-y += ampshm/mt_drv_ampshm.o
$(MOD_NAME)-y += ampshm/drv_av_secure_shm.o
endif

$(MOD_NAME)-$(CONFIG_MT_LXC_IPC) += lxc_ipc/lxc_ipc.o
$(MOD_NAME)-y += suplayer/drv_suplayer_intf.o

ifeq ($(CONFIG_SYMPHONY_SMART_SPEAKER),y)
#$(MOD_NAME)-y += dai/purin-aloop.o
#$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-asp-aloop.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-audio-dai.o dai/purin-dma.o dai/purin-proc.o dai/purin-sram-resource.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-dac-dai.o dai/codec/dac.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-adc-dai.o dai/codec/adc.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-pdm-dai.o dai/codec/pdm.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_AD83586) += dai/codec/ad83586.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_DUMMY) += dai/codec/dummy.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES7149) += dai/codec/es7144_48_49.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES7210) += dai/codec/es7210.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_PCM0_ES7210) += dai/codec/es7210_7243.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES7243) += dai/codec/es7243.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES8311) += dai/codec/es8311.o codec/es7134.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES8388) += dai/codec/es8388.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ES9023) += dai/codec/es9023.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_ESS9018Q2C) += dai/codec/es9018k2m.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_MP320) += dai/codec/mp320.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_NAU8822) += dai/codec/nau8822.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_PCM1681) += dai/codec/pcm1681.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_TAS5711) += dai/codec/tas5711.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_TLV320AIC3111) += dai/codec/tlv320aic31xx.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_WM8737) += dai/codec/wm8737.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_WM8750) += dai/codec/wm8750.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND_CODEC_WM8988) += dai/codec/wm8988.o
$(MOD_NAME)-$(CONFIG_PANTHER_SND) += dai/purin-internal-card.o dai/purin-external-card.o
#$(MOD_NAME)-$(CONFIG_SND_ALOOP) += dai/snd-aloop.o
$(MOD_NAME)-$(CONFIG_PANTHER_MICARRAY_POWERSAVING) += dai/purin-suspend-micarray-kthread.o
endif

ifeq ($(CFG_MT_SDK_RELEASE),n)
$(MOD_NAME)-$(CONFIG_MT_CHIP_VERIFICATION) += chip_verification/mss/chipverify_mss.o chip_verification/mss/mss_descrambler.o chip_verification/mss/kt/hwtest_kt.o chip_verification/mss/kl/hwtest_kl.o chip_verification/mss/cpc/hwtest_cpc.o
endif

ifeq ($(CFG_MT_CHIP),symphony6)
$(MOD_NAME)-y += pinctrl/drv_pinctrl_intf.o
$(MOD_NAME)-y += pinctrl/pinctrl.o
$(MOD_NAME)-y += pinctrl/pinmux_sym6.o
#$(MOD_NAME)-y += pinctrl/pinmux_stub.o

#TEST_JY
EXTRA_CFLAGS += '-Wno-implicit-fallthrough'
$(MOD_NAME)-y += snd/ao/drv_snd_ao.o
$(MOD_NAME)-y += snd/ao/drv_snd_reg.o
$(MOD_NAME)-y += snd/ao/snd_fpga_test.o
endif

$(MOD_NAME)-$(CONFIG_SOUND) += snd/mt_alsa/mt_snd_card.o
$(MOD_NAME)-$(CONFIG_SOUND) += snd/mt_alsa/mt_snd_codec.o
$(MOD_NAME)-$(CONFIG_SOUND) += snd/mt_alsa/mt_snd_dma.o
$(MOD_NAME)-$(CONFIG_SOUND) += snd/mt_alsa/mt_snd_dai.o

EXTRA_CFLAGS += '-Wno-int-to-pointer-cast' '-Wno-pointer-to-int-cast'
$(MOD_NAME)-y += sci/drv_sci_intf.o
$(MOD_NAME)-y += i2c/drv_i2c_intf.o

$(MOD_NAME)-$(CONFIG_MT_KEYLED) += keyled/drv_keyled.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_FD650) += keyled/fd650/drv_fd650.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_CT1642) += keyled/ct1642/drv_ct1642.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_ADC) += keyled/keyadc/drv_keyadc.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_TT1629B) += keyled/tt1629b/drv_tt1629b.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_PT6393) += keyled/vfd_pt6393/drv_pt6393.o
$(MOD_NAME)-$(CONFIG_MT_KEYLED_FD612) += keyled/fd612/drv_fd612.o

ifeq ($(CONFIG_MT_KEYLED_FD650),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_FD650_SUPPORT
endif

ifeq ($(CONFIG_MT_KEYLED_CT1642),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_CT1642_SUPPORT
endif

ifeq ($(CONFIG_MT_KEYLED_ADC),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_ADC_SUPPORT
endif

ifeq ($(CONFIG_MT_KEYLED_TT1629B),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_TT1629B_SUPPORT
endif

ifeq ($(CONFIG_MT_KEYLED_PT6393),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_PT6393_SUPPORT
endif

ifeq ($(CONFIG_MT_KEYLED_FD650_KADC),y)
EXTRA_CFLAGS += -DCFG_MT_KEYLED_FD650_KADC_SUPPORT
endif