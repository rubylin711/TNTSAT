SL_HDR_DIR := $(GENERAL_OUTPUT)/msp/drv/vo_fw/sl_hdr/
obj_sl_hdr :=  $(SL_HDR_DIR)sl_hdr.o $(SL_HDR_DIR)src/SL_api.o $(SL_HDR_DIR)src/SL_compute.o $(SL_HDR_DIR)src/SL_LUT_fill.o \
			$(SL_HDR_DIR)src/SL_math.o $(SL_HDR_DIR)src/SL_OEOTF_convert.o $(SL_HDR_DIR)src/SL_static_LUT_fill.o

VO_FW_DIR := $(GENERAL_OUTPUT)/msp/drv/vo_fw/
obj_vo_fw := $(VO_FW_DIR)src/MT_DF_4k_reg.o \
	$(VO_FW_DIR)src/MT_DF_DCE.o \
	$(VO_FW_DIR)src/MT_DF_disp.o \
	$(VO_FW_DIR)src/MT_DF_FRC.o \
	$(VO_FW_DIR)src/MT_DF_Freeze.o \
	$(VO_FW_DIR)src/MT_DF_hdr10p_ootf.o \
	$(VO_FW_DIR)src/MT_DF_HdrVivid_ootf.o \
	$(VO_FW_DIR)src/MT_DF_Osd_Gra_Scaler_4K.o \
	$(VO_FW_DIR)src/MT_DF_Osd_Gra_Scaler.o \
	$(VO_FW_DIR)src/MT_DF_PreScale.o \
	$(VO_FW_DIR)src/MT_DF_PullDown.o \
	$(VO_FW_DIR)src/MT_DF_ScalarPara.o \
	$(VO_FW_DIR)vo_fw.o\
	$(VO_FW_DIR)/src/MT_DF_4K_Disp_Set.o                                                                                                                                                                    
	
VF_MW_DIR := $(GENERAL_OUTPUT)/msp/drv/vfmw/video_lib_1.0/
obj_vf_mw := $(VF_MW_DIR)api/src/fw_api.o \
	$(VF_MW_DIR)common/src/bitstream.o \
	$(VF_MW_DIR)common/src/DPB_AVS_MPEG.o \
	$(VF_MW_DIR)common/src/fw_clock.o \
	$(VF_MW_DIR)common/src/fw_dpb_output.o \
	$(VF_MW_DIR)common/src/fw_es_descriptor.o \
	$(VF_MW_DIR)common/src/fw_hw_reset.o \
	$(VF_MW_DIR)common/src/fw_timeout.o \
	$(VF_MW_DIR)common/src/fw_utility.o \
	$(VF_MW_DIR)decoder/src/avs2_dec.o \
	$(VF_MW_DIR)decoder/src/avs.o \
	$(VF_MW_DIR)decoder/src/h264_dec.o \
	$(VF_MW_DIR)decoder/src/h264_dpb.o \
	$(VF_MW_DIR)decoder/src/h264_header.o \
	$(VF_MW_DIR)decoder/src/h264_image.o \
	$(VF_MW_DIR)decoder/src/h264_mbuffer.o \
	$(VF_MW_DIR)decoder/src/h264_output.o \
	$(VF_MW_DIR)decoder/src/hevc_comdpb.o \
	$(VF_MW_DIR)decoder/src/hevc_compic.o \
	$(VF_MW_DIR)decoder/src/hevc_comrom.o \
	$(VF_MW_DIR)decoder/src/hevc_comslice.o \
	$(VF_MW_DIR)decoder/src/hevc_deccavlc.o \
	$(VF_MW_DIR)decoder/src/hevc_decsei.o \
	$(VF_MW_DIR)decoder/src/hevc_dectop.o \
	$(VF_MW_DIR)decoder/src/mpeg4.o \
	$(VF_MW_DIR)decoder/src/mpeg.o \
	$(VF_MW_DIR)decoder/src/rv.o \
	$(VF_MW_DIR)decoder/src/vc1_dec.o \
	$(VF_MW_DIR)decoder/src/vc1_dpb.o \
	$(VF_MW_DIR)decoder/src/vdec_common.o \
	$(VF_MW_DIR)decoder/src/vp8.o \
	$(VF_MW_DIR)decoder/src/vp9_entropymode.o \
	$(VF_MW_DIR)decoder/src/vp9_entropymv.o \
	$(VF_MW_DIR)decoder/src/vp9_entropy.o \
	$(VF_MW_DIR)decoder/src/vp9.o \
	$(VF_MW_DIR)decoder/src/vp9_prob.o \
	$(VF_MW_DIR)decoder/src/vp9_quant_common.o \
	$(VF_MW_DIR)decoder/src/vp9_seg_common.o \
	$(VF_MW_DIR)middle/src/fw_middle.o \
	$(VF_MW_DIR)middle/src/fw_scheduler.o \
	$(VF_MW_DIR)vfmw_intf.o
