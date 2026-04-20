/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_AI_EXT_H__
#define __DRV_AI_EXT_H__

#include "mt_type.h"
#include "mt_drv_ai.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


typedef mt_s32 (*FN_AI_DrvResume)(basedev_s *);
typedef mt_s32 (*FN_AI_DrvSuspend)(basedev_s * , pm_message_t);

typedef struct 
{       
	FN_AI_DrvResume  pfnAI_DrvResume;
	FN_AI_DrvSuspend pfnAI_DrvSuspend;
} AI_EXPORT_FUNC_S;

mt_s32  AI_DRV_ModInit(mt_void);
mt_void AI_DRV_ModExit(mt_void);

mt_s32 MT_DRV_AI_Init(mt_void);
mt_void MT_DRV_AI_DeInit(mt_void);
mt_s32 MT_DRV_AI_Drv_Release(mt_void);
mt_s32 MT_DRV_AI_SND_GetDefaultOpenAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr);
mt_s32 MT_DRV_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phandle);
mt_s32 MT_DRV_AI_Destroy(mt_handle hAi);
mt_s32 MT_DRV_AI_SetEnable(mt_handle hAi, MT_BOOL bEnable);
mt_s32 MT_DRV_AI_GetEnable(mt_handle hAi, MT_BOOL *pbEnable);
mt_s32 MT_DRV_AI_GetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pstAttr);
mt_s32 MT_DRV_AI_GetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pstAttr);
mt_s32 MT_DRV_AI_AcquireFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame);
mt_s32 MT_DRV_AI_ReleaseFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif//__DRV_AI_EXT_H__
