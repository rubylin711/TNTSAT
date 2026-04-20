/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __PTS_RECV_H__
#define __PTS_RECV_H__

/******************************* Include Files *******************************/

/* add include here */
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "vfmw.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
mt_s32 PTSREC_Init(mt_void);
mt_s32 PTSREC_DeInit(mt_void);
mt_s32 PTSREC_Alloc(mt_handle hHandle);
mt_s32 PTSREC_Free(mt_handle hHandle);
mt_s32 PTSREC_Start(mt_handle hHandle);
mt_s32 PTSREC_Stop(mt_handle hHandle);
mt_s32 PTSREC_Reset(mt_handle hHandle);
mt_s32 PTSREC_SetFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_s32 PTSREC_GetFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate);
mt_u32 PTSREC_CalcStamp(mt_handle hHandle, MT_UNF_VCODEC_TYPE_E enVdecType, IMAGE *pstImage);
mt_u32 PTSREC_GetInterPtsDelta(mt_handle hHandle);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __PTS_RECV_H__ */
