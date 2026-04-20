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
#include "mt_drv_sys.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "mt_error_mpi.h"

#include "audio_util.h"
#include "drv_ao_op.h"
#include "hal_aoe_func.h"
#include "hal_aiao_func.h"
#include "hal_aiao_common.h"
#include "hal_aoe_common.h"
#include "hal_aoe.h"
#include "hal_cast.h"

#include "drv_ao_track.h"
 
#include "drv_ao_cast.h"

/******************************Cast process FUNC*************************************/
 mt_handle  CastGetEngineHandlebyType(SND_CARD_STATE_S *pCard, SND_ENGINE_TYPE_E enType)
{
    mt_handle hSndEngine;

    hSndEngine = pCard->hSndEngine[enType];
    if (hSndEngine)
    {
        return hSndEngine;
    }

    return MT_NULL;
}

 mt_s32 CastGetIDbyHandle(SND_CARD_STATE_S *pCard, mt_handle  handle)
{
    mt_u32 ID;
    SND_CAST_STATE_S *state;

    state = (SND_CAST_STATE_S *)(pCard->hCast[handle]);
    if(!state)
    {
        MT_ERR_AIAO("SndProcCastRoute  pCard->hCast[%d] NULL\n", handle);
        return MT_FAILURE;
    }

    ID = state->CastId;
    return ID;
}




static mt_s32 CastCreate(SND_CARD_STATE_S *pCard, mt_handle *ps32CastId, MT_UNF_SND_CAST_ATTR_S *pstCastAttr,
                         mmz_buffer_s *pstMMz)
{
    mt_s32 Ret;

    Ret = SND_CreateCastOp(pCard, ps32CastId, pstCastAttr, pstMMz);
    if (MT_SUCCESS != Ret)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 CastDestory(SND_CARD_STATE_S *pCard, mt_u32 u32CastID)
{
    mt_s32 Ret;

    Ret = SND_DestoryCastOp(pCard,  u32CastID);
    if (MT_SUCCESS != Ret)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 CastEnable(SND_CARD_STATE_S *pCard, SND_CAST_STATE_S *state, mt_s32 s32CastID, MT_BOOL bEnable)
{
    mt_handle hSndOp;
    AOE_AOP_ID_E Aop;
    mt_handle hEngine;
    SND_ENGINE_STATE_S *pEnginestate;

    hSndOp = pCard->hCastOp[s32CastID];
    if(hSndOp == MT_NULL)
    {
        MT_ERR_AIAO("SndProcCastRoute  hSndOp=%p\n", hSndOp);
        return MT_FAILURE;
    }
    Aop = SND_OpGetAopId(hSndOp);
    
#if 0
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32BitPerSample=%d\n", stOpAttr.u32BitPerSample);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32Channels=%d\n", stOpAttr.u32Channels);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32DataFormat=%d\n", stOpAttr.u32DataFormat);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32LatencyThdMs=%d\n", stOpAttr.u32LatencyThdMs);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32PeriodBufSize=%d\n", stOpAttr.u32PeriodBufSize);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32PeriodNumber=%d\n", stOpAttr.u32PeriodNumber);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32SampleRate=%d\n", stOpAttr.u32SampleRate);
#endif

    SND_StopCastOp(pCard, s32CastID);
    MT_INFO_AO("SND_StopOp Over\n");

    hEngine = CastGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_PCM);
    if(!hEngine)
    {
        state->enCurnStatus = SND_CAST_STATUS_STOP;
        if(bEnable)    
        {
            return MT_SUCCESS;
        }    
        else
        {
            MT_ERR_AIAO("Disable Cast But No Engine Found !\n");
            return MT_FAILURE;
        }
    }
    pEnginestate = (SND_ENGINE_STATE_S *)hEngine;

   if(bEnable)
   {
        HAL_AOE_ENGINE_AttachAop(pEnginestate->enEngine, Aop);
        SND_StartCastOp(pCard, s32CastID);
        state->enCurnStatus = SND_CAST_STATUS_START;
   }
   else
   {
        SND_StopCastOp(pCard, s32CastID);
        HAL_AOE_ENGINE_DetachAop(pEnginestate->enEngine, Aop);
        state->enCurnStatus = SND_CAST_STATUS_STOP;
   }

    return MT_SUCCESS;
}





/******************************AO Cast FUNC*************************************/


static mt_void  CastSetMute(mt_handle hSndOp,MT_BOOL bMute)
{
    AOE_AOP_ID_E Aop;

    if(hSndOp == MT_NULL)
    {
        MT_ERR_AIAO("CastSetVolume  hSndOp=%p\n", hSndOp);
        return ;
    }
    Aop = SND_OpGetAopId(hSndOp);

    HAL_AOE_AOP_SetMute(Aop, bMute);

}

mt_s32 CAST_SetMute(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_BOOL bMute)
{
    SND_CAST_STATE_S *pCast;
    
    pCast = (SND_CAST_STATE_S *)pCard->hCast[u32CastID];
    CastSetMute(pCard->hCastOp[u32CastID], bMute);

    pCast->bMute = bMute;
    
    return MT_SUCCESS;
}


mt_s32 CAST_GetMute(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_BOOL *pbMute)
{
    SND_CAST_STATE_S *pCast;

    if (MT_NULL == pbMute)
    {
        return MT_FAILURE;
    }
    pCast = (SND_CAST_STATE_S *)pCard->hCast[u32CastID];

    
    *pbMute = pCast->bMute;
    
    return MT_SUCCESS;
}

static mt_void  CastSetVolume(mt_handle hSndOp,mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB)
{
    AOE_AOP_ID_E Aop;

    if(hSndOp == MT_NULL)
    {
        MT_ERR_AIAO("CastSetVolume  hSndOp=%p\n", hSndOp);
        return ;
    }
    Aop = SND_OpGetAopId(hSndOp);

    HAL_AOE_AOP_SetLRVolume(Aop, u32VolumeLdB, u32VolumeRdB);

}

mt_s32 CAST_SetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S *pstCastAbsGain)
{
    SND_CAST_STATE_S *pCast;
    mt_u32 u32dBLReg;
    mt_u32 u32dBRReg;
    
    if (MT_NULL == pstCastAbsGain)
    {
        return MT_FAILURE;
    }
    if(MT_TRUE == pstCastAbsGain->bLinearMode)
    {
        CHECK_AO_LINEARVOLUME(pstCastAbsGain->s32GainL);
        CHECK_AO_LINEARVOLUME(pstCastAbsGain->s32GainR);
        
        u32dBLReg = AUTIL_VolumeLinear2RegdB((mt_u32)pstCastAbsGain->s32GainL);
        u32dBRReg = AUTIL_VolumeLinear2RegdB((mt_u32)pstCastAbsGain->s32GainR);
    }
    else
    {
        CHECK_AO_ABSLUTEVOLUME(pstCastAbsGain->s32GainL);
        CHECK_AO_ABSLUTEVOLUME(pstCastAbsGain->s32GainR);
        
        u32dBLReg = AUTIL_VolumedB2RegdB(pstCastAbsGain->s32GainL);
        u32dBRReg = AUTIL_VolumedB2RegdB(pstCastAbsGain->s32GainR);
    }

    pCast = (SND_CAST_STATE_S *)pCard->hCast[u32CastID];

    CastSetVolume(pCard->hCastOp[u32CastID], u32dBLReg, u32dBRReg);

    pCast->stCastAbsGain.bLinearMode = pstCastAbsGain->bLinearMode;
    pCast->stCastAbsGain.s32GainL = pstCastAbsGain->s32GainL;
    pCast->stCastAbsGain.s32GainR = pstCastAbsGain->s32GainR;
    return MT_SUCCESS;
}

mt_s32 CAST_GetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32CastID, MT_UNF_SND_ABSGAIN_ATTR_S *pstCastAbsGain)
{
    SND_CAST_STATE_S *pCast;

    if (MT_NULL == pstCastAbsGain)
    {
        return MT_FAILURE;
    }
    pCast = (SND_CAST_STATE_S *)pCard->hCast[u32CastID];
    
    pstCastAbsGain->bLinearMode = pCast->stCastAbsGain.bLinearMode;
    pstCastAbsGain->s32GainL = pCast->stCastAbsGain.s32GainL;
    pstCastAbsGain->s32GainR = pCast->stCastAbsGain.s32GainR;
    return MT_SUCCESS;
}



mt_void CAST_ReadProc(struct seq_file* p, SND_CARD_STATE_S *pCard)
{
    SND_CAST_STATE_S * pCast;
    mt_u32 i;
    mt_handle hSndOp;
    AOE_AOP_ID_E Aop;
    
    for(i = 0; i < AO_MAX_CAST_NUM; i++)
    {
        if(pCard->uSndCastInitFlag & ((mt_u32)1L << i))
        {
            pCast = (SND_CAST_STATE_S *)pCard->hCast[i];
            hSndOp = pCard->hCastOp[pCast->CastId];
            Aop = SND_OpGetAopId(hSndOp);            
            PROC_PRINT(p,
               "Cast(%d): *Aop(0x%x), Status(%s), UserEnable(%s), Weight(%.3d/%.3d%s), Mute(%s)\n",
                pCast->CastId,
                (mt_u32)Aop,
               (mt_char*)((SND_CAST_STATUS_START == pCast->enCurnStatus) ? "start" : ((SND_CAST_STATUS_STOP == pCast->enCurnStatus) ? "stop" : "pause")),
               (mt_char*)((pCast->bUserEnableSetting == MT_TRUE)?"On":"Off"),
                pCast->stCastAbsGain.s32GainL,  
                pCast->stCastAbsGain.s32GainR,  
               (MT_TRUE == pCast->stCastAbsGain.bLinearMode)?"":"dB",
               (MT_FALSE == pCast->bMute)?"off":"on"
               );          

            PROC_PRINT(p,
               "         SampleRate(%.6d), Channel(%.2d), BitWidth(%2d)\n",
                pCast->u32SampleRate,
                pCast->u32Channels,
                pCast->s32BitPerSample
                );
            PROC_PRINT(p,
               "         MaxFrameNum(%.2d), SamplePerFrame(%.5d), AcquireFrame(%s)\n",
                pCast->stUserCastAttr.u32PcmFrameMaxNum,  //verify
                pCast->stUserCastAttr.u32PcmSamplesPerFrame,  //verify
               (mt_char*)((pCast->bAcquireCastFrameFlag == MT_TRUE)?"On":"Off")
                );                   
        }
   }
}






mt_s32 CAST_GetDefAttr(MT_UNF_SND_CAST_ATTR_S * pstDefAttr)
{
    pstDefAttr->u32PcmFrameMaxNum = AO_CAST_DEFATTR_FRAMEMAXNUM;
    pstDefAttr->u32PcmSamplesPerFrame = AO_CAST_DEFATTR_SAMPLESPERFRAME;
    /*
    pstDefAttr->s32BitPerSample = AO_CAST_DEFATTR_BITSPERSAMPLE;
    pstDefAttr->u32Channels = AO_CAST_DEFATTR_CHANNEL;
    pstDefAttr->u32SampleRate = AO_CAST_DEFATTR_SAMPLERATE;
    pstDefAttr->u32DataFormat = 0;
    */
    return MT_SUCCESS;
}

mt_s32 CAST_CreateNew(SND_CARD_STATE_S *pCard, MT_UNF_SND_CAST_ATTR_S *pstCastAttr, mmz_buffer_s *pstMMz, mt_u32 hCast)
{
    SND_CAST_STATE_S *state = MT_NULL;
    mt_handle CastId;
    mt_s32 Ret = MT_FAILURE;

    if (!pCard)
    {
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_CAST_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc CAST_Create failed\n");
        goto CastCreate_ERR_EXIT;
    }

    memset(state, 0, sizeof(SND_CAST_STATE_S));

    memcpy(&state->stUserCastAttr, pstCastAttr, sizeof(MT_UNF_SND_CAST_ATTR_S));

    if (MT_SUCCESS != CastCreate(pCard, &CastId, &state->stUserCastAttr, pstMMz))
    {
        goto CastCreate_ERR_EXIT;
    }

    state->u32PhyAddr = pstMMz->startPhyAddr;
  	state->u32KernelVirtAddr = (ulong)pstMMz->startVirAddr;
    state->hCast  = hCast;
    state->CastId = CastId;

    state->u32Channels = 2;
    state->u32SampleRate = 48000;
    state->s32BitPerSample = 16;
    
    state->u32SampleBytes = AUTIL_CalcFrameSize(state->u32Channels , (mt_u32)state->s32BitPerSample);
    state->u32FrameBytes = pstCastAttr->u32PcmSamplesPerFrame *  state->u32SampleBytes; 
    state->u32FrameSamples = pstCastAttr->u32PcmSamplesPerFrame;

    state->bUserEnableSetting = MT_FALSE;               
    state->enCurnStatus = SND_CAST_STATUS_STOP;
    state->bAcquireCastFrameFlag = MT_FALSE;
    state->stCastAbsGain.bLinearMode = MT_TRUE;
    state->stCastAbsGain.s32GainL = AO_MAX_LINEARVOLUME;
    state->stCastAbsGain.s32GainR = AO_MAX_LINEARVOLUME;
    state->bMute = MT_FALSE;

    //pstCastAttr->u32PhyAddr = state->stUserCastAttr.u32PhyAddr;
    //pstCastAttr->u32Channels = state->stUserCastAttr.u32Channels;
    //pstCastAttr->u32SampleRate = state->stUserCastAttr.u32SampleRate;
    //pstCastAttr->s32BitPerSample = state->stUserCastAttr.s32BitPerSample;

#if 0
    MT_ERR_AO("state->hCast = 0x%x\n", state->hCast);
    MT_ERR_AO("state->CastId = 0x%x\n", state->CastId);
    MT_ERR_AO("state->stUserCastAttr.u32PhyAddr = 0x%x\n", state->stUserCastAttr.u32PhyAddr);
    MT_ERR_AO("state->u32SampleBytes = 0x%x\n", state->u32SampleBytes);
    MT_ERR_AO("state->u32FrameBytes = 0x%x\n", state->u32FrameBytes);
#endif

    pCard->hCast[hCast] = (mt_handle)state;
    pCard->uSndCastInitFlag |= ((mt_u32)1L << hCast);

    return MT_SUCCESS;
    
CastCreate_ERR_EXIT:
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return Ret;
}


mt_s32 CAST_DestroyCast(SND_CARD_STATE_S *pCard, mt_u32 hCast)
{
    SND_ENGINE_STATE_S *pEnginestate;
    SND_CAST_STATE_S *state;
    mt_s32 s32CastID;
    mt_handle hSndOp;
    mt_handle hEngine;
    AOE_AOP_ID_E Aop;

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        return MT_FAILURE;
    }

    s32CastID = state->CastId;

    if(SND_CAST_STATUS_STOP != state->enCurnStatus)
    {
        hSndOp = pCard->hCastOp[s32CastID];
        if(hSndOp == MT_NULL)
        {
            MT_ERR_AIAO("SndProcCastRoute  hSndOp=%p\n", hSndOp);
            return MT_FAILURE;
        }
        Aop = SND_OpGetAopId(hSndOp);
        hEngine = CastGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_PCM);
        if(!hEngine)
        {
            MT_ERR_AIAO("No Engine Found !\n");
            return MT_FAILURE;
        }
            
        pEnginestate = (SND_ENGINE_STATE_S *)hEngine;

        SND_StopCastOp(pCard, s32CastID);
        HAL_AOE_ENGINE_DetachAop(pEnginestate->enEngine, Aop);
        state->enCurnStatus = SND_CAST_STATUS_STOP;
    }


    CastDestory(pCard, s32CastID);

    pCard->uSndCastInitFlag &= ~((mt_u32)1L << state->hCast);
    pCard->hCast[state->hCast] = MT_NULL;
    
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return MT_SUCCESS;
}


mt_s32 CAST_SetInfo(SND_CARD_STATE_S *pCard, mt_u32 hCast, mt_u32 u32UserVirtAddr)
{
    SND_CAST_STATE_S *state;

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        return MT_FAILURE;
    }
    
    state->u32UserVirtAddr = u32UserVirtAddr;

    //MT_ERR_AO("u32UserVirtAddr = 0x%x\n", u32UserVirtAddr);

    return MT_SUCCESS;
}

mt_s32 CAST_GetInfo(SND_CARD_STATE_S *pCard, mt_u32 hCast, AO_Cast_Info_Param_S *pstInfo)
{
    SND_CAST_STATE_S *state;

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        return MT_FAILURE;
    }
    
    pstInfo->u32UserVirtAddr =state->u32UserVirtAddr;
	pstInfo->u32KernelVirtAddr = state->u32KernelVirtAddr;
    pstInfo->u32PhyAddr = state->u32PhyAddr;
    pstInfo->u32FrameBytes =state->u32FrameBytes;
    pstInfo->u32FrameSamples =state->u32FrameSamples;

    pstInfo->u32Channels = state->u32Channels;
    pstInfo->s32BitPerSample =state->s32BitPerSample;
    //MT_ERR_AO("u32UserVirtAddr = 0x%x\n", *pu32UserVirtAddr);

    return MT_SUCCESS;
}

mt_void CAST_GetSettings(SND_CARD_STATE_S *pCard, mt_handle hCast, SND_CAST_SETTINGS_S* pstCastSettings)
{
    SND_CAST_STATE_S *state;

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        return;
    }
    pstCastSettings->bMute = state->bMute;
    memcpy(&pstCastSettings->stCastAbsGain, &state->stCastAbsGain, sizeof(MT_UNF_SND_ABSGAIN_ATTR_S));
    pstCastSettings->u32UserVirtAddr = state->u32UserVirtAddr;
    pstCastSettings->bUserEnableSetting = state->bUserEnableSetting;
    return;
}

mt_void CAST_RestoreSettings(SND_CARD_STATE_S *pCard, mt_handle hCast, SND_CAST_SETTINGS_S* pstCastSettings)
{
    SND_CAST_STATE_S *state;

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        MT_ERR_AO("Cast(%d) don't attach card!\n", hCast);
        return;
    }


    state->u32UserVirtAddr = pstCastSettings->u32UserVirtAddr;

    /* fource discard ReleaseCastFrame after resume */
    state->bAcquireCastFrameFlag = MT_FALSE;
    if (pstCastSettings->bUserEnableSetting != state->bUserEnableSetting)
    {
        CAST_SetEnable(pCard, hCast, pstCastSettings->bUserEnableSetting);
        CAST_SetAbsGain(pCard, hCast, &pstCastSettings->stCastAbsGain);
        CAST_SetMute(pCard, hCast, pstCastSettings->bMute);
    }

    return;
}

mt_s32 CAST_SetEnable(SND_CARD_STATE_S *pCard, mt_u32 hCast, MT_BOOL bEnable)
{
    SND_CAST_STATE_S *state;
    mt_s32 s32CastID;

    //TOCHECK CURRENT state
    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        MT_ERR_AIAO("SND_CAST_STATE_S pointer  NULL\n");
        return MT_FAILURE;
    }

    s32CastID = CastGetIDbyHandle(pCard, hCast);
    if(MT_FAILURE == s32CastID)
    {
        MT_ERR_AIAO("CastGetIDbyHandle  Failed\n");
        return MT_FAILURE;
    }

#if 1
    state->bUserEnableSetting = bEnable;
    return CastEnable(pCard, state, s32CastID, bEnable);
    
#else
    
    //hSndOp = SND_GetOpHandlebyOutType(pCard, MT_UNF_SND_OUTPUTTYPE_CAST);
    hSndOp = pCard->hCastOp[s32CastID];
    if(hSndOp == MT_NULL)
    {
        MT_ERR_AIAO("SndProcCastRoute  hSndOp=%p\n", hSndOp);
        return MT_FAILURE;
    }
    Aop = SND_OpGetAopId(hSndOp);
    
    //MT_ERR_AIAO("hSndOp=%p Aop=0x%x\n", hSndOp, Aop);

#if 0
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32BitPerSample=%d\n", stOpAttr.u32BitPerSample);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32Channels=%d\n", stOpAttr.u32Channels);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32DataFormat=%d\n", stOpAttr.u32DataFormat);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32LatencyThdMs=%d\n", stOpAttr.u32LatencyThdMs);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32PeriodBufSize=%d\n", stOpAttr.u32PeriodBufSize);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32PeriodNumber=%d\n", stOpAttr.u32PeriodNumber);
        MT_ERR_AIAO("SndProcCastRoute  SND_GetOpAttr u32SampleRate=%d\n", stOpAttr.u32SampleRate);
#endif

    SND_StopCastOp(pCard, s32CastID);
    MT_INFO_AO("SND_StopOp Over\n");

    pCaststate->bUserEnableSetting = bEnable;

    hEngine = CastGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_PCM);
    if(!hEngine)
    {
        if (bEnable)
        {
            return MT_SUCCESS;
        }
        else
        {
            MT_ERR_AIAO("Disable Cast But No Engine Found !\n");
            return MT_FAILURE;
        }
    }
    pEnginestate = (SND_ENGINE_STATE_S *)hEngine;

   if(bEnable)
   {
        HAL_AOE_ENGINE_AttachAop(pEnginestate->enEngine, Aop);
        SND_StartCastOp(pCard, s32CastID);
        pCaststate->enCurnStatus = SND_CAST_STATUS_START;
   }
   else
   {
        SND_StopCastOp(pCard, s32CastID);
        HAL_AOE_ENGINE_DetachAop(pEnginestate->enEngine, Aop);
        pCaststate->enCurnStatus = SND_CAST_STATUS_STOP;
   }

    return MT_SUCCESS;
#endif    
}


mt_s32 CAST_GetEnable(SND_CARD_STATE_S *pCard, mt_u32 hCast, MT_BOOL *pbEnable)
{
    SND_CAST_STATE_S *state;

    //TOCHECK CURRENT state
    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if (MT_NULL == state)
    {
        MT_ERR_AIAO("SND_CAST_STATE_S pointer  NULL\n");
        return MT_FAILURE;
    }
	
    *pbEnable = state->bUserEnableSetting;
    return MT_SUCCESS;
}

mt_s32 CAST_ReadData(SND_CARD_STATE_S *pCard, mt_u32 hCast, 
                        AO_Cast_Data_Param_S *pstCastData)
{
    SND_CAST_STATE_S *state = MT_NULL;
    mt_s32 Ret;
    mt_s32 s32CastID;

    //TOCHECK CURRENT state 
    
    s32CastID = CastGetIDbyHandle(pCard, hCast);
    if(MT_FAILURE == s32CastID)
    {
        MT_ERR_AIAO("CastGetIDbyHandle  Failed\n");
        return MT_FAILURE;
    }

    if(MT_NULL == pCard->hCastOp[s32CastID])
    {
        MT_ERR_AIAO("  hSndOp=%p\n", pCard->hCastOp[s32CastID]);
        return MT_FAILURE;
    }

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if(!state)
    {
        MT_ERR_AIAO("SND_CAST_STATE_S  state =%p\n", state);
        return MT_FAILURE;
    }

#if 1
    pstCastData->stAOFrame.u32PcmSamplesPerFrame = 0;       //clear ao frame sample size
    
    if(MT_FALSE == state->bUserEnableSetting)
    {
        //MT_ERR_AIAO("Cast is not Enable!\n");
        return MT_FAILURE;
    }
    else            //user enable cast , but cast is not activity
    {
        if(SND_CAST_STATUS_STOP == state->enCurnStatus)     //to active cast
        {
            Ret = CastEnable(pCard, state, s32CastID, state->bUserEnableSetting);
            if(MT_FAILURE == Ret)
            {
                MT_ERR_AIAO("Enable Cast Failed when read data !\n");
                return MT_FAILURE;
            }
        }
        if(SND_CAST_STATUS_STOP == state->enCurnStatus)
        {
            return MT_SUCCESS;
        }
        
    }
#endif

    pstCastData->u32FrameBytes = state->u32FrameBytes;
    pstCastData->u32SampleBytes = state->u32SampleBytes;
    
#if 0
    MT_ERR_AIAO("state hCast=0x%x\n", state->hCast);
    MT_ERR_AIAO("state CastId=0x%x\n", state->CastId);
    MT_ERR_AIAO("stUserCastAttr.s32BitPerSample=0x%x\n", state->stUserCastAttr.s32BitPerSample);
    MT_ERR_AIAO("stUserCastAttr.u32Channels=0x%x\n", state->stUserCastAttr.u32Channels);
    MT_ERR_AIAO("stUserCastAttr.u32SampleRate=0x%x\n", state->stUserCastAttr.u32SampleRate);
    //MT_ERR_AIAO("stUserCastAttr.u32DataFormat=0x%x\n", state->stUserCastAttr.u32DataFormat);
    //MT_ERR_AIAO("stUserCastAttr.u32LatencyThdMs=0x%x\n", state->stUserCastAttr.u32LatencyThdMs);

    MT_ERR_AIAO("stUserCastAttr.u32PcmFrameMaxNum=0x%x\n", state->stUserCastAttr.u32PcmFrameMaxNum);
    MT_ERR_AIAO("stUserCastAttr.u32PcmSamplesPerFrame=0x%x\n", state->stUserCastAttr.u32PcmSamplesPerFrame);

    MT_ERR_AIAO("pstCastData->u32FrameBytes=0x%x\n", pstCastData->u32FrameBytes);
    MT_ERR_AIAO("pstCastData->u32SampleBytes=0x%x\n", pstCastData->u32SampleBytes);
#endif

    pstCastData->stAOFrame.s32BitPerSample = state->s32BitPerSample;
    pstCastData->stAOFrame.u32Channels = state->u32Channels;
    pstCastData->stAOFrame.u32SampleRate = state->u32SampleRate;
    pstCastData->stAOFrame.u32PcmSamplesPerFrame = state->stUserCastAttr.u32PcmSamplesPerFrame;
    pstCastData->stAOFrame.bInterleaved = MT_TRUE;

    Ret = SND_ReadCastData(pCard,  s32CastID, pstCastData);
    if (MT_FAILURE == Ret)
    {
        return MT_FAILURE;
    }

    state->bAcquireCastFrameFlag = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 CAST_ReleaseData(SND_CARD_STATE_S *pCard, mt_u32 hCast, 
                        AO_Cast_Data_Param_S *pstCastData)
{
    SND_CAST_STATE_S *state = MT_NULL;
    //mt_handle hSndOp;
    mt_s32 Ret;
    mt_s32 s32CastID;

    //TOCHECK CURRENT state     //TODO   Start state
    
    s32CastID = CastGetIDbyHandle(pCard, hCast);
    if(MT_FAILURE == s32CastID)
    {
        MT_ERR_AIAO("CastGetIDbyHandle  Failed\n");
        return MT_FAILURE;
    }

    if(MT_NULL == pCard->hCastOp[s32CastID])
    {
        MT_ERR_AIAO("  hSndOp=%p\n", pCard->hCastOp[s32CastID]);
        return MT_FAILURE;
    }

    state = (SND_CAST_STATE_S *)pCard->hCast[hCast];
    if(!state)
    {
        MT_ERR_AIAO("SND_CAST_STATE_S  state =%p\n", state);
        return MT_FAILURE;
    }

    /* discard ReleaseCastFrame before call AcquireCastFrame */
    if (MT_FALSE == state->bAcquireCastFrameFlag)
    {
        return MT_SUCCESS;
    }

    pstCastData->u32FrameBytes  = state->u32FrameBytes;
    pstCastData->u32SampleBytes = state->u32SampleBytes;
    
#if 0
    MT_ERR_AIAO("state hCast=0x%x\n", state->hCast);
    MT_ERR_AIAO("state CastId=0x%x\n", state->CastId);
    MT_ERR_AIAO("pstCastData->u32FrameBytes=0x%x\n", pstCastData->u32FrameBytes);
    MT_ERR_AIAO("pstCastData->u32SampleBytes=0x%x\n", pstCastData->u32SampleBytes);
#endif


    Ret = SND_ReleaseCastData(pCard,  s32CastID, pstCastData);
    if (MT_FAILURE == Ret)
    {
        return MT_FAILURE;
    }

    state->bAcquireCastFrameFlag = MT_FALSE;

    return MT_SUCCESS;

}

