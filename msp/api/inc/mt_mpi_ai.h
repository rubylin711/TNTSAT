/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  __MPI_AI_H__
#define  __MPI_AI_H__

#include "mt_type.h"
#include "mt_unf_ai.h"
#include "mt_drv_ai.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */


/******************************* MPI for UNF Sound Init *****************************/
mt_s32 MT_MPI_AI_Init(mt_void);

mt_s32 MT_MPI_AI_DeInit(mt_void);

mt_s32 MT_MPI_AI_GetDefaultAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr);

mt_s32 MT_MPI_AI_SetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr);

mt_s32 MT_MPI_AI_GetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr);

mt_s32 MT_MPI_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phandle);

mt_s32 MT_MPI_AI_Destroy(mt_handle hAI);

mt_s32 MT_MPI_AI_SetEnable(mt_handle hAI, MT_BOOL bEnable);

mt_s32 MT_MPI_AI_GetEnable(mt_handle hAI, MT_BOOL *pbEnable);

mt_s32 MT_MPI_AI_AcquireFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame);

mt_s32 MT_MPI_AI_ReleaseFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame);

mt_s32 MT_MPI_AI_Attach(mt_handle hAI, mt_handle hDst);

mt_s32 MT_MPI_AI_Detach(mt_handle hAI, mt_handle hDst);

mt_s32 MT_MPI_AI_SetDelay(mt_handle hAI, const MT_UNF_AI_DELAY_S *pstDelay);

mt_s32 MT_MPI_AI_GetDelay(mt_handle hAI, MT_UNF_AI_DELAY_S *pstDelay);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif //__MPI_AI_H__
