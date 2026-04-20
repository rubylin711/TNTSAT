/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AO_AEF_H__
#define __DRV_AO_AEF_H__

#include "mt_drv_ao.h"
#include "hal_aoe_func.h"
#include "hal_aoe.h"
#include "drv_ao_private.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/***************************** Struct Definition ******************************/
/*audio effect proc attr*/
typedef struct
{
    mmz_buffer_s        stProcMMz;
    AO_AEF_PROC_ITEM_S  *pstProcItem;
} SND_AEF_PROC_ATTR_S;

/******************************AEF process FUNC*************************************/

mt_s32 AEF_AttachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId, ulong *pu32AefProcAddr);
mt_s32 AEF_DetachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId);
mt_s32 AEF_GetSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings);
mt_s32 AEF_RestoreSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings);
mt_s32 SND_ReadAefProc(struct seq_file* p, SND_CARD_STATE_S *pCard);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __DRV_AO_AEF_H__