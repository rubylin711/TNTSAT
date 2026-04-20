/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_DF_REG_SET_H
#define MT_DF_REG_SET_H

#include "MT_DF_video.h"

#ifdef __cplusplus
extern "C" {
#endif

//FILTER table
#define VIDEO_SINGLE_SCALE_COEFF_TABLE_SIZE 0x100

#ifdef VIDEO_DISP_2K
#include "MT_DF_Symphony_reg.h"

typedef struct tagRegSetInfo
{
    MT_U32 stNoDIProgressive;
    MT_U32 stNoDIFieldFlag;

    reg_symphony_disp_display_ctrl_t stVideoCtl1;
    //reg_aria1_disp_video_ctrl_2_t stVideoCtl2;
    reg_symphony_disp_vid_input_size_t stInVideoSize;
    reg_symphony_disp_vid_crop_mode_en_t stVideoCropEn;
    reg_symphony_disp_vid_crop_hori_t stVideoCropPixel;
    reg_symphony_disp_vid_crop_vert_t stVideoCropLine;
    reg_symphony_disp_vscaler_ratio_hd_t stVideoScaleHDRatio;
    reg_symphony_disp_vscaler_ratio_init_hd_t stVideoScaleHDInitRatio;
    reg_symphony_disp_vid_window_x_hd_t stVideoHDWindowX;
    reg_symphony_disp_vid_window_y_hd_t stVideoHDWindowY;
    reg_symphony_disp_vid_window_cut_hd_t stVideoHDWindowCut;
    reg_symphony_disp_di_enable_t stDICtl;
    //reg_aria1_disp_di_ctrl_t stDICtl;
    reg_symphony_disp_di_pause_en_t stDIPauseEnable;
    reg_symphony_disp_video_display_info_t  stDIFieldFlag;
    reg_symphony_disp_di_hevc_flag_t stDIHEVCFlag;
    //reg_symphony_disp_none_di_progressive_flag_t stNoDIProgressive;
    //reg_symphony_disp_none_di_fields_flag_t stNoDIFieldFlag;
    reg_symphony_disp_none_di_hevc_flag_t stNoDIHEVC;
    //reg_aria1_disp_none_di_progressive_flag_2nd_t stNoDIProgressive_2nd;
    //reg_aria1_disp_none_di_fields_flag_2nd_t stNoDIFieldFlag_2nd;
    //reg_aria1_disp_none_di_hevc_flag_2nd_t stNoDIHEVC_2nd;
    reg_symphony_disp_luma_pre_addr_t stLumaPre;
    reg_symphony_disp_luma_cur_addr_t stLumaCurTop;
    reg_symphony_disp_luma_bot_cur_addr_t stLumaCurBot;
    reg_symphony_disp_luma_cur_addr2_t stLumaCurTop_2nd;
    reg_symphony_disp_luma_bot_cur_addr2_t stLumaCurBot_2nd;

    reg_symphony_disp_luma_next_addr_t stLumaNxt;
    reg_symphony_disp_chroma_ppre_addr_t stChromaPPre;
    reg_symphony_disp_chroma_pre_addr_t stChromaPre;
    reg_symphony_disp_chroma_cur_addr_t stChromaCurTop;
    reg_symphony_disp_chroma_bot_cur_addr_t stChromaCurBot;
    reg_symphony_disp_chroma_cur_addr2_t stChromaCurTop_2nd;
    reg_symphony_disp_chroma_bot_cur_addr2_t stChromaCurBot_2nd;

    reg_symphony_disp_chroma_next_addr_t stChromaNxt;

    //mfbc
    reg_symphony_disp_luma_lut_addr_t stLumaLutCurTop;
    reg_symphony_disp_luma_lut_addr_t stLumaLutCurBot;
    reg_symphony_disp_luma_lut_addr_t stLumaLutPre;
    reg_symphony_disp_luma_lut_addr_t stLumaLutNxt;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutCurTop;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutCurBot;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutPPre;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutPre;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutNxt;

    reg_symphony_disp_luma_lut_addr_t stLumaLutCurTop_2;
    reg_symphony_disp_luma_lut_addr_t stLumaLutCurBot_2;
    reg_symphony_disp_luma_lut_addr_t stLumaLutPre_2;
    reg_symphony_disp_luma_lut_addr_t stLumaLutNxt_2;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutCurTop_2;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutCurBot_2;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutPPre_2;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutPre_2;
    reg_symphony_disp_chroma_lut_addr_t stChromaLutNxt_2;

    reg_symphony_disp_decomp_ctrl_0_t stDecompCtrl;
    reg_symphony_disp_decomp_1bgs_str_t stDecomp1BgsStr;
    reg_symphony_disp_decomp_pic_resl_t stPicReslution;

    //reg_aria1_disp_tile_para_t stTilePara;
    reg_symphony_disp_row_jump_00_t stTileRowjump00;
    reg_symphony_disp_row_jump_01_t stTileRowjump01;
    reg_symphony_disp_row_jump_10_t stTileRowjump10;
    reg_symphony_disp_row_jump_11_t stTileRowjump11;
    reg_symphony_disp_hd_size_out_t stHDScreenOutSize;
    reg_symphony_disp_sd_size_out_t stSDScreenOutSize;
}DF_REG_SET_INFO;
#elif defined(VIDEO_DISP_4K)
#include "MT_DF_4k_reg.h"

typedef struct tagRegSetInfo
{
    reg_4k_disp_video_input_frame_size_t stInVideoSize;
    //reg_4k_disp_video_crop_en_t stVideoCropEn;
    //reg_4k_disp_video_crop_pixel_t stVideoCropPixel;
    //reg_4k_disp_video_crop_line_t stVideoCropLine;
    reg_4k_disp_video_scale_hd_ratio_t stVideoScaleHDRatio;
    reg_4k_disp_video_scale_hd_init_ratio_t stVideoScaleHDInitRatio;
    //reg_symphony_disp_vid_window_x_hd_t stVideoHDWindowX;
    //reg_symphony_disp_vid_window_y_hd_t stVideoHDWindowY;
    //reg_symphony_disp_vid_window_cut_hd_t stVideoHDWindowCut;
    //reg_symphony_disp_di_enable_t stDICtl;
    //reg_symphony_disp_di_pause_en_t stDIPauseEnable;
    //reg_symphony_disp_video_display_info_t  stDIFieldFlag;
    reg_4k_disp_di_storage_flag_t stDIStorageFlag;
    reg_4k_disp_none_di_storage_flag_t stNoDIStorageFlag;
    reg_4k_disp_luma_pre_addr_0_t stLumaPre;
    reg_4k_disp_luma_top_cur_addr_0_t stLumaCurTop;
    reg_4k_disp_luma_bot_cur_addr_0_t stLumaCurBot;
    reg_4k_disp_luma_top_cur_addr_2_t stLumaCurTop_2nd;
    reg_4k_disp_luma_bot_cur_addr_2_t stLumaCurBot_2nd;

    reg_4k_disp_luma_nxt_addr_0_t stLumaNxt;
    reg_4k_disp_chroma_ppre_addr_0_t stChromaPPre;
    reg_4k_disp_chroma_pre_addr_0_t stChromaPre;
    reg_4k_disp_chroma_top_cur_addr_0_t stChromaCurTop;
    reg_4k_disp_chroma_bot_cur_addr_0_t stChromaCurBot;
    reg_4k_disp_chroma_top_cur_addr_2_t stChromaCurTop_2nd;
    reg_4k_disp_chroma_bot_cur_addr_2_t stChromaCurBot_2nd;
    reg_4k_disp_chroma_nxt_addr_0_t stChromaNxt;

    //mfbc
    reg_4k_disp_vid_decomp_ltop_lut_base_t stLumaLutCurTop;
    reg_4k_disp_vid_decomp_lbot_lut_base_t stLumaLutCurBot;
    reg_4k_disp_vid_decomp_lpre_lut_base_t stLumaLutPre;
    reg_4k_disp_vid_decomp_lnxt_lut_base_t stLumaLutNxt;
    reg_4k_disp_vid_decomp_ctop_lut_base_t stChromaLutCurTop;
    reg_4k_disp_vid_decomp_cbot_lut_base_t stChromaLutCurBot;
    reg_4k_disp_vid_decomp_cppr_lut_base_t stChromaLutPPre;
    reg_4k_disp_vid_decomp_cpre_lut_base_t stChromaLutPre;
    reg_4k_disp_vid_decomp_cnxt_lut_base_t stChromaLutNxt;

   /* reg_4k_disp_vid_decomp_ltop_lut_base_2_t stLumaLutCurTop_2;
    reg_4k_disp_vid_decomp_lbot_lut_base_2_t stLumaLutCurBot_2;
    reg_4k_disp_vid_decomp_lpre_lut_base_2_t stLumaLutPre_2;
    reg_4k_disp_vid_decomp_lnxt_lut_base_2_t stLumaLutNxt_2;
    reg_4k_disp_vid_decomp_ctop_lut_base_2_t stChromaLutCurTop_2;
    reg_4k_disp_vid_decomp_cbot_lut_base_2_t stChromaLutCurBot_2;
    reg_4k_disp_vid_decomp_cppr_lut_base_2_t stChromaLutPPre_2;
    reg_4k_disp_vid_decomp_cpre_lut_base_2_t stChromaLutPre_2;
    reg_4k_disp_vid_decomp_cnxt_lut_base_2_t stChromaLutNxt_2;*/

    reg_4k_disp_vid_decomp_ltop_lut_base_t stLumaLutCurTop_2nd;
    reg_4k_disp_vid_decomp_lbot_lut_base_t stLumaLutCurBot_2nd;
    reg_4k_disp_vid_decomp_ctop_lut_base_t stChromaLutCurTop_2nd;
    reg_4k_disp_vid_decomp_cbot_lut_base_t stChromaLutCurBot_2nd;

    //reg_symphony_disp_decomp_ctrl_0_t stDecompCtrl;
    //reg_symphony_disp_decomp_1bgs_str_t stDecomp1BgsStr;
    //reg_symphony_disp_decomp_pic_resl_t stPicReslution;

    //reg_aria1_disp_tile_para_t stTilePara;
    //reg_symphony_disp_row_jump_00_t stTileRowjump00;
    //reg_symphony_disp_row_jump_01_t stTileRowjump01;
    //reg_symphony_disp_row_jump_10_t stTileRowjump10;
    //reg_symphony_disp_row_jump_11_t stTileRowjump11;
    //reg_symphony_disp_hd_size_out_t stHDScreenOutSize;
    //reg_symphony_disp_sd_size_out_t stSDScreenOutSize;
}DF_REG_SET_INFO;

#endif

MT_U32 DF_Get_Chip_Id(void);
ulong DF_Get_RegBase(void);

MT_RET DF_InitVideoCoeffTable(void);
MT_U32 DF_GetVideoCoeffTableAddr(VIDEO_SCALE_COEFF_TABLE_E table);
MT_RET DF_FinVideoCoeffTable(void);

MT_U32 DF_RegSet_Get_Vid_Status(void);
void DF_RegSet_Set_Vid_OnOff(MT_DF_BOOL bOn);

MT_U32 DF_RegSet_Get_SubVid_Status(void);
void DF_RegSet_Set_SubVid_OnOff(MT_DF_BOOL bOn);

void DF_RegSet_Set_ClockGate(void);

MT_RET DF_RegSet_DI_Close(DISP_POOL_S * pstDispBP, DISP_CFG_DI_ENABLE* pstCfgDIDisable, MT_FIELDPOLARITY eFieldPolarity_CurNeed, DF_CONTROL_INFO_S *pstInfo);
MT_RET DF_RegSet_DI(DISP_POOL_S * pstDispBP, DISP_CFG_DI_ENABLE * pstCfgDIEnable, DF_CONTROL_INFO_S *pstInfo);

MT_FIELDPOLARITY DF_Check_3scaler_Polarity(void);

void DF_RegSet_Show_Black(DF_CONTROL_INFO_S *pstInfo, MT_DF_BOOL bBlack);

MT_U32 DF_Get_Luma_Top_Cur_Addr(mt_void);

mt_void DF_RegSet_OSD_CSC(mt_u32 dst_tv_mode);
mt_void DF_RegSet_SD_CSC(mt_u32 dst_tv_mode);
mt_void DF_RegSet_CSC(MT_DF_CSC_INFO_T *p_cur, mt_u32 dst_tv_mode, mt_u32 dst_max_luminance);
mt_void DF_Hdr10p_Reg_Cfg(MT_DF_VIDEO_FRAME_S * pstFrame, DF_CONTROL_INFO_S *pstInfo, MT_DF_CSC_INFO_T *pCscInfo);

MT_VOID DF_Regset_OsdScaler(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height);
mt_void DF_RegSet_GraScaler(  disp_sys_t eHdVidSys, disp_sys_t eSdVidSys);
MT_VOID DF_Regset_StillScaler(DF_CONTROL_INFO_S *pstInfo, MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive);
MT_VOID DF_RegSet_DumpScaler(MT_U32 in_width, MT_U32 in_height, MT_U32 out_width, MT_U32 out_height);
MT_VOID DF_RegSet_DumpScaler_Start(MT_U32 luma_addr, MT_U32 chroma_addr, MT_U32 stride);
MT_VOID DF_RegSet_DumpScaler_Stop(MT_VOID);
mt_void DF_RegSet_Wr_Motion(MT_U8 enable_value);
mt_void DF_RegSet_Denoise_Deband(MT_U8 denoise_enable, MT_U8 deband_enable);
mt_void DF_RegSet_Dce(MT_U8 dce_enable);
mt_void DF_Display_Init(void);

#ifdef VIDEO_DISP_NEW_ALGO
MT_VOID DF_RegSet_New_DI(DISP_POOL_S* pstDispBP, MT_DF_VIDEO_FRAME_S * pstFrame,MT_DF_VIDEO_FRAME_S * pPpreFrame,MT_DF_VIDEO_FRAME_S * pPreFrame,MT_DF_VIDEO_FRAME_S * pNxtFrame, MT_BOOL bCurTop, MT_U32 uDiBufferAddr);
MT_VOID DF_LoadSRVideoFilter(MT_DF_VIDEO_FRAME_S * pstFrameCur,MT_VOID* stCalcScalarPara, DF_DRV_DISP_PPMODE_E stDispMode, DF_CONTROL_INFO_S *pstInfo, MT_U8 bDisableSR);
#endif

MT_VOID DF_RegSet_Set_Pdd(MT_DF_BOOL status);
MT_U32 DF_RegSet_Get_Pdd_Status(void);
MT_VOID DF_RegSet_Get_Pdd_Diffs(void* pPddDiffs);
MT_VOID DF_RegSet_Set_MovieMode(MT_DF_BOOL bMovieMode, MT_U32 u32WeaveFlag);

#ifdef __cplusplus
}
#endif
#endif

