
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DF_HDR10P_OOTF_H__
#define __MT_DF_HDR10P_OOTF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mt_drv_video.h"

typedef struct HDR10PLUS_OOTF_INFO
{
  MT_U32 dest_max_luminace_disp;
  MT_U32 target_max_luminace;
  MT_U32 knee_point_x;
  MT_U32 knee_point_y;
  MT_U32 num_bezier_curve_anchors;//0~9
  MT_U32 bezier_curve_anchors[11];
  MT_U32 maxscl[3];
  MT_U32 average_maxrgb;
  MT_U32 distribution_value[9];
  MT_U32 hdr2sdr_mode; //0:hdr2sdr 1:hdr2hdr 2:hdr2hlg
}HDR10PLUS_OOTF_INFO_T;

void ootf_process_fixed (sei_dynamic_hdr_metadata_t* p_hdr10p_metadata,  MT_U16 *ootf_table, HDR10PLUS_OOTF_INFO_T * ootf_info_disp_out,
    MT_U32* p_hdr10p_eotf_normal_value, MT_U32 dst_tv_mode, MT_U32 dest_max_luminace_disp);


void sdr2hdr_ootf_process3(MT_U32 *hist_tab_org , MT_U32 width, MT_U32 height, MT_U32 *degamma_table, MT_U32 hist_limit_th1, MT_U32 hist_limit_th2,
    MT_U32 hist_limit_en , MT_U32 sdr2hdr_ootf_mode ,  MT_U32 max_anchor_sdr2hdr, MT_U16 *ootf_table,
    HDR10PLUS_OOTF_INFO_T * ootf_info_disp_out, MT_U32 dst_tv_mode);

void DF_Hlg_Ootf_process2(MT_U16 *ootf_table, MT_U32 hdr10_plus_mode, HDR10PLUS_OOTF_INFO_T * ootf_info_disp_out, MT_U32 dest_max_luminance);

#ifdef __cplusplus
}
#endif

#endif
