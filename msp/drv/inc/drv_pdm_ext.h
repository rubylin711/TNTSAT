/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_PDM_EXT_H__
#define __DRV_PDM_EXT_H__

#include "mt_type.h"
#include "mt_drv_pdm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef mt_s32 (*FN_PDM_GetDispParam)(MT_UNF_DISP_E enDisp, MT_DISP_PARAM_S *pstDispParam);
typedef mt_s32 (*FN_PDM_GetMceParam)(MT_MCE_PARAM_S *pMceParam);
typedef mt_s32 (*FN_PDM_GetMceData)(mt_u32 u32Size, mt_u32 *pAddr);
typedef mt_s32 (*FN_PDM_ReleaseReserveMem)(const mt_char *BufName);
typedef mt_s32 (*FN_PDM_GetData)(const mt_char *BufName, mt_u32 *pu32DataAddr, mt_u32 *pu32DataLen);
typedef mt_s32 (*FN_PDM_GetSoundParam)(MT_UNF_SND_E enSound, MT_UNF_PDM_SOUND_PARAM_S * pstSoundParam);

typedef struct tagPDM_EXPORT_FUNC_S
{
    FN_PDM_GetDispParam             pfnPDM_GetDispParam;
    FN_PDM_GetMceParam              pfnPDM_GetMceParam;
    FN_PDM_GetMceData               pfnPDM_GetMceData;
    FN_PDM_ReleaseReserveMem        pfnPDM_ReleaseReserveMem;
	FN_PDM_GetData                  pfnPDM_GetData; 
	FN_PDM_GetSoundParam			pfnGetSoundParam;
}PDM_EXPORT_FUNC_S;

mt_s32 PDM_DRV_ModInit(mt_void);
mt_void PDM_DRV_ModExit(mt_void);


#ifdef __cplusplus
}
#endif

#endif

