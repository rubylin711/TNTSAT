/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MPI_DISP_TRAN_H__
#define __MPI_DISP_TRAN_H__

#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_common.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"

#include "mt_drv_video.h"
#include "mt_drv_disp.h"
#include "mt_drv_win.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 Transfer_DispID(MT_UNF_DISP_E *pU, MT_DRV_DISPLAY_E *pM, MT_BOOL bu2m);
mt_s32 Transfer_DispOffset(MT_UNF_DISP_OFFSET_S *pU, MT_DRV_DISP_OFFSET_S *pM, MT_BOOL bu2m);
mt_s32 Transfer_LayerID(MT_UNF_DISP_LAYER_E *pU, MT_DRV_DISP_LAYER_E *pM, MT_BOOL bu2m);
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 Convert_DrvEncFmt(MT_UNF_ENC_FMT_E *pU, MT_DRV_DISP_FMT_E *pM, MT_BOOL bu2m);
#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 Transfer_AspectRatio(MT_UNF_DISP_ASPECT_RATIO_S *pU, mt_u32 *pH, mt_u32 *pV, MT_BOOL bu2m);
mt_s32 Transfer_Timing(MT_UNF_DISP_TIMING_S *pU, MT_DRV_DISP_TIMING_S *pM, MT_BOOL bu2m);
mt_s32 Transfer_BGColor(MT_UNF_DISP_BG_COLOR_S *pU, MT_DRV_DISP_COLOR_S *pM, MT_BOOL bu2m);
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 Transfer_VideoFormat(MT_UNF_VIDEO_FORMAT_E  *pU, MT_DRV_PIX_FORMAT_E *pM, MT_BOOL bu2m);
mt_s32 Transfe_ARConvert(MT_UNF_VO_ASPECT_CVRS_E  *pU, MT_DRV_ASP_RAT_MODE_E *pM, MT_BOOL bu2m);
mt_s32 Transfe_ZOrder(MT_LAYER_ZORDER_E *pU, MT_DRV_DISP_ZORDER_E *pM, MT_BOOL bu2m);
mt_s32 Transfe_SwitchMode(MT_UNF_WINDOW_FREEZE_MODE_E *pU, MT_DRV_WIN_SWITCH_E *pM, MT_BOOL bu2m);
mt_s32 Transfer_Disp3DMode(MT_UNF_DISP_3D_E *pU, MT_DRV_DISP_STEREO_MODE_E *pM, MT_BOOL bu2m);
mt_s32 Transfe_Rotate(MT_UNF_VO_ROTATION_E *pU, MT_DRV_ROT_ANGLE_E *pM, MT_BOOL bu2m);
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

#if 0  /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
mt_s32 Transfer_Frame(MT_UNF_VIDEO_FRAME_INFO_S  *pU, MT_DRV_VIDEO_FRAME_S *pM, MT_BOOL bu2m);
mt_s32 Transfer_BufferPool(MT_UNF_BUFFER_ATTR_S *pU, MT_DRV_VIDEO_BUFFER_POOL_S*pM, MT_BOOL bu2m);
mt_s32 Transfer_CastCfg(MT_UNF_DISP_CAST_ATTR_S  *pU, MT_DRV_DISP_CAST_CFG_S *pM, MT_BOOL bu2m);

mt_s32 Transfer_Intf(MT_UNF_DISP_INTF_S *pU, MT_DRV_DISP_INTF_S *pM, MT_BOOL bu2m);
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
