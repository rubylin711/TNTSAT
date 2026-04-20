/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//#include <asm/setup.h>
#include <linux/interrupt.h>

#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"

#include "mt_module.h"
#include "mt_drv_mmz.h"
#include "mt_drv_stat.h"
#include "mt_drv_sys.h"
#include "mt_drv_proc.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "mt_error_mpi.h"

#include "audio_util.h"
#include "drv_ao_aef.h"
//#include "mt_audsp_aflt.h"  //TODO for AFLT_MAX_CHAN_NUM

static mt_s32 AEFAttachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId)
{
    mt_handle hSndEngine;
    SND_ENGINE_STATE_S *pstEngineState;
    
    hSndEngine = pCard->hSndEngine[SND_ENGINE_TYPE_PCM];
    if(hSndEngine)
    {
        pstEngineState = (SND_ENGINE_STATE_S *)hSndEngine;
        if(MT_SUCCESS != HAL_AOE_ENGINE_AttachAef(pstEngineState->enEngine, u32AefId))
        {
            MT_ERR_AO("HAL_AOE_ENGINE_AttachAef failed!\n");
            return MT_FAILURE;
        }
    }
   
    return MT_SUCCESS;
}

static mt_s32 AEFDetachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId)
{
    mt_handle hSndEngine;
    SND_ENGINE_STATE_S *pstEngineState;
    
    hSndEngine = pCard->hSndEngine[SND_ENGINE_TYPE_PCM];
    if(!hSndEngine)
    {
        MT_ERR_AO("no have pcm engine!\n");
        return MT_FAILURE;
    }

    pstEngineState = (SND_ENGINE_STATE_S *)hSndEngine;
    
    if(MT_SUCCESS != HAL_AOE_ENGINE_DetachAef(pstEngineState->enEngine, u32AefId))
    {
        MT_ERR_AO("HAL_AOE_ENGINE_DetachAef failed!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 AEF_AttachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId, ulong *pu32AefProcAddr)
{
    mt_s32 Ret;
    mt_char szProcMmzName[32];
    SND_AEF_PROC_ATTR_S *pstAefProc = MT_NULL; 

    Ret = AEFAttachSnd(pCard, u32AefId);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AO("AEF_AttachSnd failed\n");
        goto err0;
    }

    pstAefProc = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_AEF_PROC_ATTR_S), GFP_KERNEL);
    if(MT_NULL == pstAefProc)
    {
        MT_ERR_AO("malloc SND_AEF_PROC_ATTR_S failed\n");
        goto err1;
    }

    snprintf(szProcMmzName, sizeof(szProcMmzName), "AO_AefProcItem");
    Ret = mt_drv_mmz_alloc_and_map(szProcMmzName, MMZ_OTHERS, sizeof(AO_AEF_PROC_ITEM_S), 0, &pstAefProc->stProcMMz);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AO("mt_drv_mmz_alloc_and_map %s failed\n", szProcMmzName);
        goto err2;
    }

    pstAefProc->pstProcItem = (AO_AEF_PROC_ITEM_S *)(pstAefProc->stProcMMz.startVirAddr);
    pCard->hAefProc[u32AefId] = (mt_handle)pstAefProc;
    pCard->u32AttAef |= ((mt_u32)1L << u32AefId);
    *pu32AefProcAddr = pstAefProc->stProcMMz.startPhyAddr;

    return MT_SUCCESS;

err2:
    AUTIL_AO_FREE(MT_ID_AO, pstAefProc);
    
err1:
    (mt_void)AEFDetachSnd(pCard, u32AefId);
    
err0:
    return MT_FAILURE;   
}

mt_s32 AEF_DetachSnd(SND_CARD_STATE_S *pCard, mt_u32 u32AefId)
{
    SND_AEF_PROC_ATTR_S *pstAefProc = MT_NULL;

    pstAefProc = (SND_AEF_PROC_ATTR_S *)pCard->hAefProc[u32AefId];

    if(pstAefProc)
    {
        mt_drv_mmz_unmap_and_release(&pstAefProc->stProcMMz);
        AUTIL_AO_FREE(MT_ID_AO, pstAefProc);
        pCard->hAefProc[u32AefId] = (mt_handle)MT_NULL;
    }

    if(MT_SUCCESS != AEFDetachSnd(pCard, u32AefId))
    {
        MT_ERR_AO("AEFDetachSnd failed\n");
        return MT_FAILURE;
    }

    pCard->u32AttAef &= ~((mt_u32)1L << u32AefId);

    return MT_SUCCESS;
}

mt_s32 AEF_GetSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings)
{
    mt_u32 u32AefId;
    
    pstSndSettings->u32AttAef = pCard->u32AttAef;
    for(u32AefId = 0; u32AefId < AFLT_MAX_CHAN_NUM; u32AefId++)
    {
        pstSndSettings->hAefProc[u32AefId]  = pCard->hAefProc[u32AefId];
    }
    
    return MT_SUCCESS; 
}

mt_s32 AEF_RestoreSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings)
{
    mt_u32 u32AefId;
        
    for(u32AefId = 0; u32AefId < AFLT_MAX_CHAN_NUM; u32AefId++)
    {
        if(pstSndSettings->u32AttAef & ((mt_u32)1L << u32AefId))
        {
            if(MT_SUCCESS != AEFAttachSnd(pCard, u32AefId))
            {
                return MT_FAILURE;
            }
        }
        pCard->hAefProc[u32AefId] = pstSndSettings->hAefProc[u32AefId];
    }
    
    pCard->u32AttAef = pstSndSettings->u32AttAef;
    return MT_SUCCESS; 
}

mt_s32 SND_ReadAefProc( struct seq_file* p, SND_CARD_STATE_S *pCard )
{
    mt_u32 i;
    mt_handle hAefProc;
    AO_AEF_PROC_ITEM_S  *pstProcItem;
    
    for(i = 0; i < AFLT_MAX_CHAN_NUM; i++)
    {
        if(pCard->u32AttAef & ((mt_u32)1L << i))
        {
            hAefProc = pCard->hAefProc[i];
            if(MT_NULL == hAefProc)            
            {
                return MT_FAILURE;
            }
            pstProcItem = ((SND_AEF_PROC_ATTR_S *)hAefProc)->pstProcItem;
            PROC_PRINT(p,
               "Aef(%d): Type(%s), AuthDescription(%s), Status(%s)\n",
                pstProcItem->u32AefId,
                pstProcItem->szName,
                pstProcItem->szDescription,
               (mt_char*)((MT_TRUE == pstProcItem->bEnable) ? "start" : "stop"));
        }
    }

    PROC_PRINT(p,"\n");

    return MT_SUCCESS;
}