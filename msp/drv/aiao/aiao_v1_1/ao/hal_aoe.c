/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"
#include "mt_module.h"
#include <linux/string.h>

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#include "mt_drv_mem.h"
#include "hal_aoe.h"
#include "hal_aoe_func.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_mem.h"
#include "mt_drv_module.h"
#include "drv_ao_private.h"

#include "circ_buf.h"  //todo drv_aiao_debug_common
#include "audio_util.h"
#include "mt_cache.h"
#include "mt_drv_dma.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

typedef struct
{
    mt_handle hAip[AOE_AIP_BUTT];
    mt_handle hAop[AOE_AOP_BUTT];
    mt_handle hMix[AOE_ENGINE_BUTT];
} AOE_GLOBAL_SOURCE_S;

typedef struct
{
    mt_u32 uTotalByteWrite;
    mt_u32 uTryWriteCnt;
} AOE_AIP_PROC_STATUS_S;

typedef struct
{
    AOE_AIP_CHN_ATTR_S stUserAttr;

    /* internal state */
    AOE_AIP_ID_E aip;
    mt_u32                u32BufFrameSize;
    mt_u32                u32FiFoFrameSize;
    mt_s32                s32AdjSpeed;
    mt_u32                u32LVolumedB;
    mt_u32                u32RVolumedB;
    MT_BOOL               bMute;
	mt_u32				  u32ChannelMode;
    AOE_AIP_STATUS_E      enCurnStatus;
    CIRC_BUF_S            stCB;
    AOE_AIP_PROC_STATUS_S stProc;
} AOE_AIP_CHN_STATE_S;

/* private state */
static AOE_GLOBAL_SOURCE_S g_AoeRm;

#define CHECK_AIP_OPEN(AIP) \
    do {\
        if (MT_NULL == g_AoeRm.hAip[AIP])\
        {\
            MT_ERR_AO("aip (%d) is not create.\n", AIP); \
            return MT_FAILURE; \
        } \
    } while (0)

static mt_void AOEAIPFlushState(AOE_AIP_CHN_STATE_S *state)
{
    state->s32AdjSpeed = 0;
    memset(&state->stProc,0, sizeof(AOE_AIP_PROC_STATUS_S));
    iHAL_AOE_AIP_SetSpeed(state->aip, state->s32AdjSpeed);
    //todo flush detail
}

extern mt_u32 TrackGetEnaip(SND_CARD_STATE_S *pCard);
mt_s32 HAL_SET_stCB(SND_CARD_STATE_S *pCard,AudioBufTypeAria CBType,ulong reg_base,ulong vaddr,mt_u32 sz)
{
    AOE_AIP_ID_E enAIP=TrackGetEnaip(pCard);
    AOE_AIP_CHN_STATE_S *state=(AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];
    if(AUDIO_BUF_TYPE_PCM==CBType){
        state->stUserAttr.stBufInAttr.stRbfAttr.b_spdif_mode=MT_TRUE;
    }else{
        state->stUserAttr.stBufInAttr.stRbfAttr.b_spdif_mode=MT_FALSE;
    }
    state->stCB.CBType = CBType;
    CIRC_BUF_Init(&state->stCB, reg_base,vaddr,sz);

	return MT_SUCCESS;
}

mt_u8 g_PPBuf_Reseted = 0;
mt_u8 g_PCMBuf_Reseted = 0;
mt_u8 g_SPDBuf_Reseted = 0;
mt_u8 g_MIXBuf_Reseted = 0;

/* global function */
mt_s32 HAL_AOE_Init(MT_BOOL bSwAoeFlag)
{
    AOE_AIP_ID_E aip;
    AOE_AOP_ID_E aop;
    AOE_ENGINE_ID_E engine;

    /* init rm */
    for (aip = AOE_AIP0; aip < AOE_AIP_BUTT; aip++)
    {
        g_AoeRm.hAip[aip] = MT_NULL;
    }

    for (aop = AOE_AOP0; aop < AOE_AOP_BUTT; aop++)
    {
        g_AoeRm.hAop[aop] = MT_NULL;
    }

    for (engine = AOE_ENGINE0; engine < AOE_ENGINE_BUTT; engine++)
    {
        g_AoeRm.hMix[engine] = MT_NULL;
    }

    return iHAL_AOE_Init(bSwAoeFlag);
}

mt_void                 HAL_AOE_DeInit(mt_void)
{
    AOE_AIP_ID_E aip;
    AOE_AOP_ID_E aop;
    AOE_ENGINE_ID_E engine;

    /* init rm */
    for (aip = AOE_AIP0; aip < AOE_AIP_BUTT; aip++)
    {
        if (g_AoeRm.hAip[aip])
        {
            iHAL_AOE_AIP_Destroy(aip);
        }
        g_AoeRm.hAip[aip] = MT_NULL;
    }

    for (aop = AOE_AOP0; aop < AOE_AOP_BUTT; aop++)
    {
        if (g_AoeRm.hAop[aop])
        {
            iHAL_AOE_AOP_Destroy(aop);
        }
        g_AoeRm.hAop[aop] = MT_NULL;
    }

    for (engine = AOE_ENGINE0; engine < AOE_ENGINE_BUTT; engine++)
    {
        if (g_AoeRm.hMix[engine])
        {
            iHAL_AOE_ENGINE_Destroy(engine);
        }
        g_AoeRm.hMix[engine] = MT_NULL;
    }

    iHAL_AOE_DeInit();
}

AOE_AIP_ID_E  AOEGetFreeAIP(mt_void)
{
    AOE_AIP_ID_E enFreeAip;

    for (enFreeAip = AOE_AIP0; enFreeAip < AOE_AIP_BUTT; enFreeAip++)
    {
        if (!g_AoeRm.hAip[enFreeAip])
        {
            return enFreeAip;
        }
    }
    return AOE_AIP_BUTT;
}

AOE_AOP_ID_E  AOEGetFreeAOP(mt_void)
{
    AOE_AOP_ID_E enFreeAop;

    for (enFreeAop = AOE_AOP0; enFreeAop < AOE_AOP_BUTT; enFreeAop++)
    {
        if (!g_AoeRm.hAop[enFreeAop])
        {
            return enFreeAop;
        }
    }
    return AOE_AOP_BUTT;
}

#if 0 //unuse code
static mt_u32 UTIL_CalcFrameSize(mt_u32 uCh, mt_u32 uBitDepth)
{
    mt_u32 uFrameSize = 0;

    switch (uBitDepth)
    {
    case 16:
        uFrameSize = ((mt_u32)uCh) * sizeof(mt_u16);
        break;
    case 24:
        uFrameSize = ((mt_u32)uCh) * sizeof(mt_u32);
        break;
    default:
        break;
    }

    return uFrameSize;
}
#endif

mt_s32  HAL_AOE_AIP_Create(AOE_AIP_ID_E *penAIP, AOE_AIP_CHN_ATTR_S *pstAttr)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;
    AOE_AIP_ID_E enAIP;
    mt_s32 Ret = MT_FAILURE;

    // todo , check attr
    enAIP = AOEGetFreeAIP();
    if (AOE_AIP_BUTT == enAIP)
    {
        MT_ERR_AO("Get free Aip failed!\n");
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(AOE_AIP_CHN_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc AOE_AIP_CHN_STATE_S failed\n");
        goto AIP_Create_ERR_EXIT;
    }

    memset(state, 0, sizeof(AOE_AIP_CHN_STATE_S));
    g_AoeRm.hAip[enAIP] = (mt_handle)state;

    if (MT_SUCCESS != (Ret = HAL_AOE_AIP_SetAttr(enAIP, pstAttr)))
    {
        MT_ERR_AO("HAL_AOE_AIP_SetAttr failed!\n");
        goto AIP_Create_ERR_EXIT;
    }

    //state->enCurnStatus = AOE_AIP_STATUS_STOP;
    state->aip = enAIP;

    memcpy(&state->stUserAttr,pstAttr,sizeof(AOE_AIP_CHN_ATTR_S));      //record inout infor

    *penAIP = enAIP;
    return MT_SUCCESS;

AIP_Create_ERR_EXIT:
    *penAIP = AOE_AIP_BUTT;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hAip[enAIP] = MT_NULL;
    return Ret;
}

mt_s32  HAL_AOE_AIP_CreateNew(AOE_AIP_ID_E *penAIP, AOE_AIP_CHN_ATTR_NEW_S *pstAttr)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;
    AOE_AIP_ID_E enAIP;
    mt_s32 Ret = MT_FAILURE;

    // todo , check attr
    enAIP = AOEGetFreeAIP();
    if (AOE_AIP_BUTT == enAIP)
    {
        MT_ERR_AO("Get free Aip failed!\n");
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(AOE_AIP_CHN_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc AOE_AIP_CHN_STATE_S failed\n");
        goto AIP_Create_ERR_EXIT;
    }

    memset(state, 0, sizeof(AOE_AIP_CHN_STATE_S));
    g_AoeRm.hAip[enAIP] = (mt_handle)state;


    state->stCB.CBType = pstAttr->CBType;

    if(AUDIO_BUF_TYPE_PP == pstAttr->CBType)
    {
        pstAttr->u32Size = PP_BUF_CONFIG_LEN >> 3;
    }
    else if(AUDIO_BUF_TYPE_PCM == pstAttr->CBType)
    {
        pstAttr->u32StartVirAddr += PP_BUF_CONFIG_LEN;
        pstAttr->u32Size -= PP_BUF_CONFIG_LEN;
    }

    CIRC_BUF_Init(&state->stCB, g_audio_reg_base,
                      pstAttr->u32StartVirAddr,
                      pstAttr->u32Size);


    //state->enCurnStatus = AOE_AIP_STATUS_STOP;
    state->aip = enAIP;


    *penAIP = enAIP;
    return MT_SUCCESS;

AIP_Create_ERR_EXIT:
    *penAIP = AOE_AIP_BUTT;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hAip[enAIP] = MT_NULL;
    return Ret;
}


mt_void     HAL_AOE_AIP_Destroy(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        return;
    }

    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_STOP != state->enCurnStatus)
    {
        HAL_AOE_AIP_Stop(enAIP);
    }

    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hAip[enAIP] = MT_NULL;
    return;
}


mt_s32  HAL_AOE_AIP_SetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr)
{
    mt_s32 Ret;
    AOE_AIP_CHN_STATE_S *state = MT_NULL;
    //mt_u32 u32WptrAddr, u32RptrAddr;
    AudioBufTypeAria  CBType;
    AOE_AIP_INBUF_ATTR_S   *pInAttr;

    CHECK_AIP_OPEN(enAIP);

    // check attr
    if (MT_NULL == pstAttr)
    {
        MT_FATAL_AO("pstAttr is null\n");
        return MT_FAILURE;
    }
    pInAttr  = &pstAttr->stBufInAttr;

    MT_FATAL_AO("minnan debug : BufPhyAddr(0x%x) BufVirAddr(0x%x) AIP %d invalid\n",
      pInAttr->stRbfAttr.u32BufPhyAddr,pInAttr->stRbfAttr.u32BufVirAddr, enAIP);
    if (!pInAttr->stRbfAttr.u32BufPhyAddr || !pInAttr->stRbfAttr.u32BufVirAddr)
    {
        MT_FATAL_AO("BufPhyAddr(0x%x) BufVirAddr(0x%x) invalid\n",pInAttr->stRbfAttr.u32BufPhyAddr,pInAttr->stRbfAttr.u32BufVirAddr);
        return MT_FAILURE;
    }
    if (!pInAttr->stRbfAttr.u32BufSize)
    {
        MT_FATAL_AO("BufSize(0x%x) invalid\n",pInAttr->stRbfAttr.u32BufSize);
        return MT_FAILURE;
    }

    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    Ret = iHAL_AOE_AIP_SetAttr(enAIP, pstAttr);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    if (!pstAttr->stBufInAttr.stRbfAttr.u32BufWptrRptrFlag)
    {
        //if(SND_ENGINE_TYPE_PCM == pstAttr->sound_type)
        if(pstAttr->stBufInAttr.stRbfAttr.b_spdif_mode)// spdif mode  set to pcm buffer
        {
          CBType = AUDIO_BUF_TYPE_PCM;
        }
        else // pcm  mode  set to pp buffer
        {
          CBType = AUDIO_BUF_TYPE_PP;
        }

          state->stCB.CBType = CBType;
        //CIRC_BUF_Init(&state->stCB, CBType , g_audio_reg_base,
        CIRC_BUF_Init(&state->stCB, g_audio_reg_base,
                      pstAttr->stBufInAttr.stRbfAttr.u32BufVirAddr,
                      (pstAttr->stBufInAttr.stRbfAttr.u32BufSize ));
    }


    return MT_SUCCESS;
}


mt_s32  HAL_AOE_AIP_GetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);

    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];
    /*MT_FATAL_AO("minnan debug: HAL_AOE_AIP_GetAttr %d 0x%x 0x%x\n",
      enAIP, state->stUserAttr.stBufInAttr.stRbfAttr.u32BufPhyAddr, state->stUserAttr.stBufInAttr.stRbfAttr.u32BufVirAddr);*/
    memcpy(pstAttr, &state->stUserAttr, sizeof(AOE_AIP_CHN_ATTR_S));
    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_Start(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    state->enCurnStatus = AOE_AIP_STATUS_START;
    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_Stop(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    state->enCurnStatus = AOE_AIP_STATUS_STOP;
    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_Group_Stop(mt_u32 u32StopMask)
{
    mt_u32 u32AipId = 0;
    mt_u32 u32CmdDone = 0;
    AOE_AIP_CHN_STATE_S *state[AOE_MAX_AIP_NUM] = {MT_NULL};

    for(u32AipId = 0; u32AipId < AOE_MAX_AIP_NUM; u32AipId++)
    {
        if((1 << u32AipId) & u32StopMask)
        {
            if((1 << u32AipId) & u32CmdDone)
            {
                state[u32AipId] = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[u32AipId];
                AOEAIPFlushState(state[u32AipId]);
                state[u32AipId]->enCurnStatus = AOE_AIP_STATUS_STOP;
            }
            else
            {
                MT_ERR_AO("Err: AIP(%d) track stop failed!\n", u32AipId);
            }
        }
    }

    if(u32CmdDone == u32StopMask)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}

mt_s32  HAL_AOE_AIP_Pause(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_PAUSE == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    state->enCurnStatus = AOE_AIP_STATUS_PAUSE;
    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_Flush(mt_u32 Type)
{
    if(SND_ENGINE_TYPE_PCM == Type) 
    {
        iHAL_AOE_AIP_Reset_PCMBuf();
    }
    else if(SND_ENGINE_TYPE_SPDIF_RAW == Type)
    {
        iHAL_AOE_AIP_Reset_SPDBuf();
    }    
#ifdef CONFIG_MT_CHIP_SYMPHONY4	
    else if(SND_ENGINE_TYPE_HDMI_RAW == Type)
    {
        iHAL_AOE_AIP_Reset_MIXBuf();
    }
#endif    

    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_SetLRVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    iHAL_AOE_AIP_SetLRVolume(enAIP, u32VolumeLdB, u32VolumeRdB);


    state->u32LVolumedB = u32VolumeLdB;
    state->u32RVolumedB = u32VolumeRdB;

    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_SetMute(AOE_AIP_ID_E enAIP, MT_BOOL bMute)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    iHAL_AOE_AIP_SetMute(enAIP, bMute);

    state->bMute = bMute;

    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AIP_SetChannelMode(AOE_AIP_ID_E enAIP, mt_u32 u32ChannelMode)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    iHAL_AOE_AIP_SetChannelMode(enAIP, u32ChannelMode);


    state->u32ChannelMode = u32ChannelMode;

    return MT_SUCCESS;
}


mt_s32  HAL_AOE_AIP_SetSpeed(AOE_AIP_ID_E enAIP, mt_s32 s32AdjSpeed)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    CHECK_AIP_OPEN(enAIP);
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    iHAL_AOE_AIP_SetSpeed(enAIP, s32AdjSpeed);
    state->s32AdjSpeed = s32AdjSpeed;
    return MT_SUCCESS;
}

mt_u32  HAL_AOE_AIP_QueryBufData(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];



    return CIRC_BUF_QueryBusy(&state->stCB);
}

mt_u32                  HAL_AOE_AIP_QueryBufFree(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

#if 0
    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }
#endif

    return CIRC_BUF_QueryFree(&state->stCB);
}






void HAL_AOE_AIP_SetPcmSampleRate(mt_u32 samplerate, MT_BOOL b_x4)
{
  mt_u32 val;
  reg_aud_snt_aud_res_reg0_t *p_sample_use;
  reg_aud_snt_pp_en_t *p_pp_enable;
  reg_aud_snt_clk_div_t *p_pcm_sample_clk;
  //mt_u8 index = samplerate_2_index(samplerate);

  //set index
  //reg_aud_snt_ch_srt_t *p_sam = g_audio_reg_base + REG_AUD_SNT_CH_SRT;
  //p_sam->bitc.sample_rate = index;

  p_sample_use = (reg_aud_snt_aud_res_reg0_t*)(g_audio_reg_base + REG_AUD_SNT_AUD_RES_REG0);

  if(p_sample_use->bitc.res)
    return;

  p_pp_enable = (reg_aud_snt_pp_en_t*)(g_audio_reg_base + REG_AUD_SNT_PP_EN);
  p_pcm_sample_clk = (reg_aud_snt_clk_div_t*)(g_audio_reg_base + REG_AUD_SNT_CLK_DIV);

  if((32000 <= samplerate) && (samplerate != 64000))
  {
    p_pp_enable->bitc.src_en = 0;
    val = samplerate / 100; //for 44.1 88.2
    if(b_x4)
    {
      val *= 4;
      MT_ERR_AO("eac3, set to 4 times val = %d\n", val);
    }
    else
    {
      MT_ERR_AO("not eac3, set to normal val = %d\n", val);
    }

    p_pcm_sample_clk->bitc.clk_divider_factor = (AUD_SAMPLE_EF * val) / 10;

  }
  else  //enable src
  {
    p_pp_enable->bitc.src_en = 1;
    p_pcm_sample_clk->bitc.clk_divider_factor = AUD_SAMPLE_EF * 48;
  }
}

//#define __cpuc_flush_dcache_area
//extern void __cpuc_flush_dcache_area(void *, size_t);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
DEFINE_SEMAPHORE(g_AudioMutex, 1);
/*
 * @param[in] to/from physical address
 */
void *audio_dma_memcpy(void *to, const void *from, size_t n, unsigned int ch_id)
{
	mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;

	MT_INFO_AO("dma_memcpy: from %p to %p, size %u\n",from,to,n);

    down_interruptible(&g_AudioMutex);
	if(n == 0)  
	{
		MT_ERR_AO("Param error: from %p to %p, size %u\n",from,to,n);
        up(&g_AudioMutex);
		return NULL;
	}
	
	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.param.len = (mt_u32)n;
	dma_param.param.phy_src_addr = (phys_addr_t)(ulong)from;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = (phys_addr_t)(ulong)to;
	dma_param.param.vir_dst_addr = 0;

	//align 128
	if ((dma_param.param.phy_src_addr & (128-1)) == 0
		&& (dma_param.param.phy_dst_addr & (128-1)) == 0
		&& (dma_param.param.len & (128-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM16;
	}
	//align 64
	else if ((dma_param.param.phy_src_addr & (64-1)) == 0
		&& (dma_param.param.phy_dst_addr & (64-1)) == 0
		&& (dma_param.param.len & (64-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM8;
	}
	//align 32
	else if ((dma_param.param.phy_src_addr & (32-1)) == 0
		&& (dma_param.param.phy_dst_addr & (32-1)) == 0
		&& (dma_param.param.len & (32-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM4;
	}
	//align 16
	else if ((dma_param.param.phy_src_addr & (16-1)) == 0
		&& (dma_param.param.phy_dst_addr & (16-1)) == 0
		&& (dma_param.param.len & (16-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM2;
	}
	//align 8
	else if ((dma_param.param.phy_src_addr & (8-1)) == 0
		&& (dma_param.param.phy_dst_addr & (8-1)) == 0
		&& (dma_param.param.len & (8-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 4
	else if ((dma_param.param.phy_src_addr & (4-1)) == 0
		&& (dma_param.param.phy_dst_addr & (4-1)) == 0
		&& (dma_param.param.len & (4-1)) == 0)
	{
		usize = DMA_USIZE_32BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 2
	else if ((dma_param.param.phy_src_addr & (2-1)) == 0
		&& (dma_param.param.phy_dst_addr & (2-1)) == 0
		&& (dma_param.param.len & (2-1)) == 0)
	{
		usize = DMA_USIZE_16BIT;
		bnum = DMA_BURST_NUM1;
	}
	else
	{
		usize = DMA_USIZE_8BIT;
		bnum = DMA_BURST_NUM1;
	}

	dma_param.param.config.dst_peripheral = 0xf; // memory
	dma_param.param.config.src_peripheral = 0xf; // memory
	dma_param.param.config.dst_endian = 0; // big endian
	dma_param.param.config.src_endian = 0; // big endian
	dma_param.param.config.dst_clk = 0; // AXI clock
	dma_param.param.config.src_clk = 0; // AXI clock
	dma_param.param.config.dst_i = DMA_ADDR_INC;
	dma_param.param.config.src_i = DMA_ADDR_INC;
	dma_param.param.config.dst_usize = usize;
	dma_param.param.config.src_usize = usize;
	dma_param.param.config.dst_bsize = bnum;
	dma_param.param.config.src_bsize = bnum;
	dma_param.param.control.int_link_en = 1;
	dma_param.param.control.int_node_en = 1;
	dma_param.param.control.chn_param_reg_en = 0;

	dma_param.chn_id = ch_id;
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_AO("dma get free channel failed!\n");
        up(&g_AudioMutex);
		return NULL;
	}

	ret = hal_dma_start(dma_param.chn_id, (mt_void *)&dma_param.param, NULL);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_AO("dma channel(%d) start failed!\n",dma_param.chn_id);
        up(&g_AudioMutex);
		return NULL;
	}

	while (DMA_STATUS_STOP != hal_dma_check(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//usleep_range(100, 100);
		usleep_range(100, 1000);
		tmo --;
		if (tmo <= 0)
		{
			MT_ERR_AO("dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = hal_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_AO("dma channel(%d) stop failed!\n",dma_param.chn_id);
        up(&g_AudioMutex);
		return NULL;
	}

    up(&g_AudioMutex);
	return to;
}
EXPORT_SYMBOL(audio_dma_memcpy);
extern mt_s32 hal_dma_active_channel(mt_s32 id);
extern mt_s32 hal_dma_deactive_channel(mt_s32 id);
void *AO_MEMCPY(void *dest, const void *src, size_t n)
{
	while(DMA_CHANNEL_SECURE_AUDIO_1 != hal_dma_active_channel(DMA_CHANNEL_SECURE_AUDIO_1)){
		usleep_range(100, 1000);
	};
	
	audio_dma_memcpy(dest, src, n, DMA_CHANNEL_SECURE_AUDIO_1);

	while(MT_SUCCESS != hal_dma_deactive_channel(DMA_CHANNEL_SECURE_AUDIO_1)){
		usleep_range(100, 1000);
	};
	return NULL;
}

EXPORT_SYMBOL(AO_MEMCPY);

#else
static inline void *AO_MEMCPY(void *dest, const void *src, size_t n)
{
	if(n){//avoid zero length cause kernel crash
		memcpy(dest, src, n);
		mt_dcache_clean(dest, n);
	}
	return dest;
}
#endif

mt_void CIRC_BUF_Init(CIRC_BUF_S *pstCb,
                        //AudioBufTypeAria CBType,
                        ulong u32RegBase,
                        ulong pu8Data,
                        mt_u32  u32Len)
{
    //TODO++++++++++++++
    //MT_ASSERT(NULL != pstCb);


    pstCb->pu32Write  = 0;
    pstCb->pu32Read   = 0;
    pstCb->pu8Data    = (mt_u8 *)pu8Data;
    pstCb->u32Lenght  = u32Len;
    pstCb->u32RegBase = u32RegBase;
    //pstCb->CBType = CBType;
    pstCb->info = 0;

    MT_ALWAYS_PRINT("CIRC BUF len 0x%x\n", u32Len);

    if(AUDIO_BUF_TYPE_PP == pstCb->CBType)
    {
      pstCb->cmd_reg = u32RegBase + REG_AUD_SNT_PP_BUF_WRCMD;
      pstCb->pu32Write_cnt_reg = u32RegBase + REG_AUD_SNT_AUD_PP_BUF0_CNT;
    }
    else if(AUDIO_BUF_TYPE_PCM == pstCb->CBType)
    {
      pstCb->cmd_reg = u32RegBase + REG_AUD_SNT_PCM_BUF_WRCMD;
      pstCb->pu32Write_cnt_reg = u32RegBase + REG_AUD_SNT_PCM_BUF_CNT;
    }
    else if(AUDIO_BUF_TYPE_SPDIF == pstCb->CBType)
    {
      pstCb->cmd_reg = u32RegBase + REG_AUD_SNT_SPD_BUF_WRCMD;
      pstCb->pu32Write_cnt_reg = u32RegBase + REG_AUD_SNT_SPDIF_BUF_CNT;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4	
    else if(AUDIO_BUF_TYPE_MIX == pstCb->CBType)
    {
      pstCb->cmd_reg = u32RegBase + REG_AUD_SNT_MIX_BUF_WRCMD;
      pstCb->pu32Write_cnt_reg = u32RegBase + REG_AUD_SNT_MIX_BUF_CNT;
    }
#endif


//	MT_ALWAYS_PRINT("cmd reg 0x%px, cnt reg 0x%px pcm addr 0x%px pcm len 0x%x\n", pstCb->cmd_reg,
//  		pstCb->pu32Write_cnt_reg, pstCb->pu8Data, pstCb->u32Lenght);
    //*pstCb->pu32Write     = 0;
    //*pstCb->pu32Read      = 0;
}

mt_u32 CIRC_BUF_Write(CIRC_BUF_S *pstCb, mt_u8 *pDest, mt_u32 u32Len, mt_u32 ChanExist)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#define MIPS_PHYS(a)	((a)&0x1fffffff)
    pDest = (mt_u8*)MIPS_PHYS((ulong)pDest);
    pDest = (mt_u8*)__va((ulong)pDest);
#endif
    ulong write_pnt = (ulong)pstCb->pu32Write;
    mt_u32 cpy1,cpy2;
    
    if(AUDIO_BUF_TYPE_PP == pstCb->CBType && g_PPBuf_Reseted){
        write_pnt = 0;
        g_PPBuf_Reseted = 0;
    }

    if(AUDIO_BUF_TYPE_PCM == pstCb->CBType && g_PCMBuf_Reseted){
        write_pnt = 0;
        g_PCMBuf_Reseted = 0;
    }

    if(AUDIO_BUF_TYPE_SPDIF == pstCb->CBType && g_SPDBuf_Reseted){
        write_pnt = 0;
        g_SPDBuf_Reseted = 0;
    }

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    if(AUDIO_BUF_TYPE_MIX == pstCb->CBType && g_MIXBuf_Reseted){
        write_pnt = 0;
        g_MIXBuf_Reseted = 0;
    }
#endif

    if((write_pnt + u32Len) > pstCb->u32Lenght){
        cpy1 = pstCb->u32Lenght - write_pnt;
        cpy2 = u32Len - cpy1;
    }else{
        cpy1 = u32Len;
        cpy2 = 0;
    }

    if(AUDIO_BUF_TYPE_PP == pstCb->CBType){
        int i = 0;
        mt_u8 chansoft = 0;
        mt_u32 cfg_chan = ChanExist;
        mt_u32 reg_table[8] = {
            REG_AUD_SNT_AUD_PP_BUF0_CNT, REG_AUD_SNT_AUD_PP_BUF1_CNT,
            REG_AUD_SNT_AUD_PP_BUF2_CNT, REG_AUD_SNT_AUD_PP_BUF3_CNT,
            REG_AUD_SNT_AUD_PP_BUF4_CNT, REG_AUD_SNT_AUD_PP_BUF5_CNT,
            REG_AUD_SNT_AUD_PP_BUF6_CNT, REG_AUD_SNT_AUD_PP_BUF7_CNT
        };
        
        if(cfg_chan != pstCb->info){
            pstCb->info = cfg_chan;
            MT_ALWAYS_PRINT("channel changed from %d to %d\n", pstCb->info , ChanExist);

            for(i = 7;i >= 0;i--)
            {
                if(ChanExist & (1 << i))    break;
            }
            if(0 <= i){
                pstCb->pu32Write_cnt_reg = pstCb->u32RegBase + reg_table[i];
                iHAL_AOE_AIP_reset_aout_buffer();
                aria_set_audio_reg_0_channel(pstCb->u32RegBase, cfg_chan);
                return u32Len;
            }
        }

        for(i = 0;i < 8;i++)
        {
            if(ChanExist & (1 << i)){
                ulong dest = (ulong)(pstCb->pu8Data + write_pnt + (i * pstCb->u32Lenght));
                ulong src = (ulong)(pDest  + (chansoft * u32Len));

                if(cpy2){
                    AO_MEMCPY((void*)dest, (void*)src, (size_t)cpy1);
                    dest = (ulong)(pstCb->pu8Data + (i * pstCb->u32Lenght));
                    src += cpy1;
                    AO_MEMCPY((void*)dest, (void*)src, (size_t)cpy2);
                }else{
                    AO_MEMCPY((void*)dest, (void*)src, (size_t)u32Len);
                }
                chansoft++;
            }
        }
        for(i = 0;i < 8;i++)
        {
            if(cfg_chan & (1 << i)){
                HAL_PUT_U32((volatile u32 *)pstCb->cmd_reg, (u32Len >> 3) | 0x80000000 | (i << 24));
            }
        }
  }
  else if((AUDIO_BUF_TYPE_PCM == pstCb->CBType) || (AUDIO_BUF_TYPE_SPDIF == pstCb->CBType)
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        || (AUDIO_BUF_TYPE_MIX == pstCb->CBType)
#endif		
            ){
            ulong dest = (ulong)(pstCb->pu8Data + write_pnt);
            ulong src = (ulong)pDest;

            if(cpy2){
                AO_MEMCPY((void*)dest, (void*)src, (size_t)cpy1);
                dest = (ulong)pstCb->pu8Data;
                src = (ulong)(pDest + cpy1);
                AO_MEMCPY((void*)dest, (void*)src, (size_t)cpy2);
            }else{
                AO_MEMCPY((void*)dest, (void*)src, (size_t)u32Len);
            }
            HAL_PUT_U32((volatile u32 *)pstCb->cmd_reg, (u32Len >> 3) | 0x80000000);
    }

    if(cpy2){
        write_pnt = cpy2;
    }else{
        write_pnt += u32Len;
        write_pnt %= pstCb->u32Lenght;
    }
    write_pnt >>= 5;
    write_pnt <<= 5; //32 Bytes align
    pstCb->pu32Write = (ulong*)write_pnt;

    return (u32Len );
}

mt_u32 HAL_AOE_AIP_WriteBufData(AOE_AIP_ID_E enAIP, mt_u8 * pu32Src, mt_u32 u32SrcBytes, mt_u32 ChanExist)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;
    mt_u32 Bytes;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

#if 0
    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        return 0;
    }
#endif

    state->stProc.uTryWriteCnt++;

    Bytes = CIRC_BUF_Write(&state->stCB, pu32Src, u32SrcBytes,  ChanExist);
    state->stProc.uTotalByteWrite += Bytes;
    return Bytes;
}

//for ALSA
mt_u32 HAL_AOE_AIP_UpdateWritePos(AOE_AIP_ID_E enAIP, mt_u32 *pu32WptrLen)
{
    mt_u32 ret = 0;
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        //MT_WARN_AO("\naip (%d) AOE_AIP_STATUS_STOP.\n", enAIP);
        //return 0;
    }

    //TRP(*pu32WptrLen);
    //TRP(*(state->stCB.pu32Write));

    ret = CIRC_BUF_ALSA_UpdateWptr(&state->stCB, *pu32WptrLen);
    //TRP(ret);

    return 0;
}

//for ALSA
mt_u32 HAL_AOE_AIP_UpdateReadPos(AOE_AIP_ID_E enAIP, mt_u32 *pu32RptrLen)
{
	mt_u32 ret = 0;
	AOE_AIP_CHN_STATE_S *state = MT_NULL;

	if (MT_NULL == g_AoeRm.hAip[enAIP])
	{
		MT_WARN_AO("aip (%d) is not create.\n", enAIP);
		return 0;
	}
	state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

	if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
	{
		//MT_WARN_AO("\naip (%d) AOE_AIP_STATUS_STOP.\n", enAIP);
		//return 0;
	}

	//TRP(*pu32WptrLen);
	//TRP(*(state->stCB.pu32Write));

	ret = CIRC_BUF_ALSA_UpdateRptr(&state->stCB, *pu32RptrLen);
	//TRP(ret);

	return 0;
}

//for ALSA
mt_u32 HAL_AOE_AIP_GetReadPos(AOE_AIP_ID_E enAIP, mt_u32 *pu32ReadPos)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        //MT_WARN_AO("\naip (%d) AOE_AIP_STATUS_STOP.\n", enAIP);
        //return 0;
    }

    *pu32ReadPos = CIRC_BUF_QueryReadPos(&state->stCB);

    return 0;
}

//for ALSA
mt_u32 HAL_AOE_AIP_FlushBuf(AOE_AIP_ID_E enAIP)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        MT_WARN_AO("aip (%d) is not create.\n", enAIP);
        return 0;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    CIRC_BUF_Flush(&state->stCB);

    return 0;
}

mt_void                 HAL_AOE_AIP_GetBufDelayMs(AOE_AIP_ID_E enAIP, mt_u32 *pDelayms)
{
#if 0
    AOE_AIP_CHN_STATE_S *state = MT_NULL;
    mt_u32 FreeBytes = 0;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        *pDelayms = 0;
        return;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        *pDelayms = 0;
        return;
    }

    if (state->stUserAttr.stBufInAttr.stRbfAttr.u32BufWptrRptrFlag)
    {
        MT_WARN_AO("dont support AIP_GetBufDelayMs whent u32BufWptrRptrFlag(1)\n");
        *pDelayms = 0;
        return;
    }

    FreeBytes = CIRC_BUF_QueryBusy(&state->stCB);
    *pDelayms = CALC_LATENCY_MS(state->stUserAttr.stBufInAttr.u32BufSampleRate, state->u32BufFrameSize, FreeBytes);
#endif
    return;
}

mt_void                 HAL_AOE_AIP_GetFiFoDelayMs(AOE_AIP_ID_E enAIP, mt_u32 *pDelayms)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        *pDelayms = 0;
        return;
    }
    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];

    if (AOE_AIP_STATUS_STOP == state->enCurnStatus)
    {
        *pDelayms = 0;
        return;
    }


    *pDelayms = iHAL_AOE_AIP_GetFiFoDelayMs(enAIP);
    return;
}

mt_void HAL_AOE_AIP_GetStatus(AOE_AIP_ID_E enAIP, AOE_AIP_STATUS_E *peStatus)
{
    AOE_AIP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAip[enAIP])
    {
        *peStatus = AOE_AIP_STATUS_STOP;
        return;
    }

    state = (AOE_AIP_CHN_STATE_S*)g_AoeRm.hAip[enAIP];
    *peStatus = state->enCurnStatus;
    return;
}


/*aop func*/
typedef struct
{
    AOE_AOP_CHN_ATTR_S stUserAttr;
    mt_u32                u32LVolumedB;
    mt_u32                u32RVolumedB;
    MT_BOOL               bMute;

    /* internal state */
    AOE_AOP_ID_E enAop;
    AOE_AOP_STATUS_E      enCurnStatus;
    MT_BOOL               bBypass;//TODO
} AOE_AOP_CHN_STATE_S;

mt_s32 AOECheckFreeAOP(AOE_AOP_ID_E enAOP)
{
        if (g_AoeRm.hAop[enAOP])
        {
            return MT_FAILURE;
        }
    return MT_SUCCESS;
}

#define CHECK_AOP_OPEN(AOP) \
    do {\
        if (MT_NULL == g_AoeRm.hAop[AOP])\
        {\
            MT_ERR_AO("aop (%d) is not create.\n", AOP); \
            return MT_FAILURE; \
        } \
    } while (0)

mt_s32  HAL_AOE_AOP_SetMute(AOE_AOP_ID_E enAOP, MT_BOOL bMute)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    iHAL_AOE_AOP_SetMute(enAOP, bMute);

    state->bMute = bMute;

    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AOP_SetLRVolume(AOE_AOP_ID_E enAOP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    iHAL_AOE_AOP_SetLRVolume(enAOP, u32VolumeLdB, u32VolumeRdB);

    state->u32LVolumedB = u32VolumeLdB;
    state->u32RVolumedB = u32VolumeRdB;
    return MT_SUCCESS;
}

mt_s32  HAL_AOE_AOP_SetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr)
{
    mt_s32 Ret;
    AOE_AOP_CHN_STATE_S *state = MT_NULL;
    //MT_U32 u32WptrAddr, u32RptrAddr;
    AOE_AOP_OUTBUF_ATTR_S   *pOuAttr;

    CHECK_AOP_OPEN(enAOP);

    // check attr
    if (MT_NULL == pstAttr)
    {
        MT_FATAL_AO("pstAttr is null\n");
        return MT_FAILURE;
    }
    state = (AOE_AOP_CHN_STATE_S * )g_AoeRm.hAop[enAOP];
    pOuAttr  = &pstAttr->stRbfOutAttr;
    if (!pOuAttr->stRbfAttr.u32BufPhyAddr || !pOuAttr->stRbfAttr.u32BufVirAddr)
    {
        MT_FATAL_AO("BufPhyAddr(0x%x) BufVirAddr(0x%x) invalid\n",pOuAttr->stRbfAttr.u32BufPhyAddr,pOuAttr->stRbfAttr.u32BufVirAddr);
        return MT_FAILURE;
    }
    if (!pOuAttr->stRbfAttr.u32BufSize)
    {
        MT_FATAL_AO("BufSize(0x%x) invalid\n",pOuAttr->stRbfAttr.u32BufSize);
        return MT_FAILURE;
    }

    if (pOuAttr->u32BufLatencyThdMs < AOE_AOP_BUFF_LATENCYMS_MIN)
    {
        MT_FATAL_AO("FiFoLatencyThdMs(%d) is less than min(%d)\n",pOuAttr->u32BufLatencyThdMs, AOE_AOP_BUFF_LATENCYMS_MIN);
        return MT_FAILURE;
    }
    #if 0
    //check port id +++  attach Port to set
    if(enAOP)
        pstAttr->stRbfOutAttr.bRbfHwPriority = MT_TRUE;
    #endif

    Ret = iHAL_AOE_AOP_SetAttr(enAOP, pstAttr);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }
    memcpy(&state->stUserAttr, pstAttr, sizeof(AOE_AOP_CHN_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 HAL_AOE_AOP_Create(AOE_AOP_ID_E *penAOP, AOE_AOP_CHN_ATTR_S *pstAttr)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;
    AOE_AOP_ID_E enAOP;
    mt_s32 Ret = MT_FAILURE;

    // todo , check attr

    enAOP = AOEGetFreeAOP();
    if (AOE_AOP_BUTT == enAOP)
    {
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(AOE_AOP_CHN_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc AOE_AOP_CHN_ATTR_S failed\n");
        goto AOP_Create_ERR_EXIT;
    }

    memset(state, 0, sizeof(AOE_AOP_CHN_STATE_S));
    g_AoeRm.hAop[enAOP] = (mt_handle)state;

    if (MT_SUCCESS != (Ret = HAL_AOE_AOP_SetAttr(enAOP, pstAttr)))
    {
        goto AOP_Create_ERR_EXIT;
    }

    //TODO state->bBypass
    state->bBypass = 0;
    state->enCurnStatus = AOE_AOP_STATUS_STOP;
    state->enAop = enAOP;

    state->bMute = MT_FALSE;//to check
    iHAL_AOE_AOP_SetMute(enAOP,MT_FALSE);

    *penAOP = enAOP;
    return MT_SUCCESS;

AOP_Create_ERR_EXIT:
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hAop[enAOP] = MT_NULL;
    *penAOP = AOE_AOP_BUTT;
    return Ret;

}

mt_void HAL_AOE_AOP_Destroy(AOE_AOP_ID_E enAOP)
{
     AOE_AOP_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hAop[enAOP])
    {
        return;
    }
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hAop[enAOP] = MT_NULL;
    return;
}

mt_s32 HAL_AOE_AOP_GetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);

    state = (AOE_AOP_CHN_STATE_S * )g_AoeRm.hAop[enAOP];

    memcpy(pstAttr, &state->stUserAttr, sizeof(AOE_AOP_CHN_ATTR_S));
    return MT_SUCCESS;
}

mt_s32 HAL_AOE_AOP_Start(AOE_AOP_ID_E enAOP)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    if (AOE_AOP_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }
    state->enCurnStatus = AOE_AOP_STATUS_START;
    return MT_SUCCESS;

}

static mt_void AOEAOPFlushState(AOE_AOP_CHN_STATE_S *state)
{
    //todo flush detail
}


mt_s32 HAL_AOE_AOP_Stop(AOE_AOP_ID_E enAOP)
{
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    if (AOE_AOP_STATUS_STOP == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    //TODO add+++
    AOEAOPFlushState(state);

    state->enCurnStatus = AOE_AOP_STATUS_STOP;
    return MT_SUCCESS;
}

mt_s32 HAL_AOE_AOP_SetAefBypass(AOE_AOP_ID_E enAOP, MT_BOOL bBypass)
{
    mt_s32 Ret;
    AOE_AOP_CHN_STATE_S *state = MT_NULL;

    CHECK_AOP_OPEN(enAOP);
    state = (AOE_AOP_CHN_STATE_S*)g_AoeRm.hAop[enAOP];

    Ret = iHAL_AOE_AOP_SetAefBypass(enAOP, bBypass);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    state->bBypass = bBypass;

    return MT_SUCCESS;
}

mt_s32 HAL_AOE_AOP_GetStatus(AOE_AOP_ID_E enAOP, mt_void *pstStatus)
{
    //todo
    return MT_SUCCESS;
}

/* ENGINE function */

typedef struct
{
    AOE_ENGINE_CHN_ATTR_S stUserAttr;

    /* internal state */
    AOE_ENGINE_ID_E enMix;
    AOE_ENGINE_STATUS_E      enCurnStatus;
} AOE_ENGINE_CHN_STATE_S;

AOE_ENGINE_ID_E  AOEGetFreeEngine(mt_void)
{
    AOE_ENGINE_ID_E enFreeEngine;

    for (enFreeEngine = AOE_ENGINE0; enFreeEngine < AOE_ENGINE_BUTT; enFreeEngine++)
    {
        if (!g_AoeRm.hMix[enFreeEngine])
        {
            return enFreeEngine;
        }
    }
    return AOE_ENGINE_BUTT;
}

#define CHECK_ENGINE_OPEN(ENGINE) \
    do {\
        if (MT_NULL == g_AoeRm.hMix[ENGINE])\
        {\
            MT_ERR_AO("engine (%d) is not create.\n", ENGINE); \
            return MT_FAILURE; \
        } \
    } while (0)

mt_s32 HAL_AOE_ENGINE_Create(AOE_ENGINE_ID_E *penENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr)
{
    AOE_ENGINE_CHN_STATE_S *state = MT_NULL;
    AOE_ENGINE_ID_E enEngine;
    mt_s32 Ret = MT_FAILURE;

    // todo , check attr

    enEngine = AOEGetFreeEngine();
    if (AOE_ENGINE_BUTT == enEngine)
    {
        MT_ERR_AO("Get free engine failed!\n");
        return MT_FAILURE;
    }

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(AOE_ENGINE_CHN_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc AOE_ENGINE_CHN_STATE_S failed\n");
        goto Engine_Create_ERR_EXIT;
    }

    memset(state, 0, sizeof(AOE_ENGINE_CHN_STATE_S));
    g_AoeRm.hMix[enEngine] = (mt_handle)state;
    if (MT_SUCCESS != (Ret = HAL_AOE_ENGINE_SetAttr(enEngine, pstAttr)))
    {
        goto Engine_Create_ERR_EXIT;
    }

    state->enCurnStatus = AOE_ENGINE_STATUS_STOP;
    //TODO +++
    state->enMix = enEngine;

    *penENGINE = enEngine;
    return MT_SUCCESS;

Engine_Create_ERR_EXIT:
    *penENGINE = AOE_ENGINE_BUTT;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hMix[enEngine] = MT_NULL;
    return Ret;

}

mt_void HAL_AOE_ENGINE_Destroy(AOE_ENGINE_ID_E enENGINE)
{
     AOE_ENGINE_CHN_STATE_S *state = MT_NULL;

    if (MT_NULL == g_AoeRm.hMix[enENGINE])
    {
        return;
    }
    state = (AOE_ENGINE_CHN_STATE_S*)g_AoeRm.hMix[enENGINE];

    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    g_AoeRm.hMix[enENGINE] = MT_NULL;
    return;

}

mt_s32 HAL_AOE_ENGINE_SetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr)
{
    AOE_ENGINE_CHN_STATE_S *state = MT_NULL;
    mt_s32 Ret = MT_FAILURE;

    CHECK_ENGINE_OPEN(enENGINE);
    // check attr
    if (MT_NULL == pstAttr)
    {
        MT_FATAL_AO("pstAttr is null\n");
        return MT_FAILURE;
    }
    //TODO +++ check samplerate/channel/depth

    state = (AOE_ENGINE_CHN_STATE_S * )g_AoeRm.hMix[enENGINE];
    Ret = iHAL_AOE_ENGINE_SetAttr(enENGINE, *pstAttr);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }
    memcpy(&state->stUserAttr, pstAttr, sizeof(AOE_ENGINE_CHN_ATTR_S));
    return MT_SUCCESS;
}

mt_s32 HAL_AOE_ENGINE_GetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr)
{
    AOE_ENGINE_CHN_STATE_S *state = MT_NULL;

    CHECK_ENGINE_OPEN(enENGINE);

    state = (AOE_ENGINE_CHN_STATE_S * )g_AoeRm.hMix[enENGINE];

    memcpy(pstAttr, &state->stUserAttr, sizeof(AOE_ENGINE_CHN_ATTR_S));
    return MT_SUCCESS;
}

mt_s32 HAL_AOE_ENGINE_Start(AOE_ENGINE_ID_E enENGINE)
{
    mt_s32 Ret = MT_FAILURE;
    AOE_ENGINE_CHN_STATE_S *state = MT_NULL;

    CHECK_ENGINE_OPEN(enENGINE);
    state = (AOE_ENGINE_CHN_STATE_S * )g_AoeRm.hMix[enENGINE];

    if (AOE_ENGINE_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    //Ret = iHAL_AOE_ENGINE_SetCmd(enENGINE, AOE_ENGINE_CMD_START);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    state->enCurnStatus = AOE_ENGINE_STATUS_START;

    return MT_SUCCESS;
}

mt_s32 HAL_AOE_ENGINE_Stop(AOE_ENGINE_ID_E enENGINE)
{
    mt_s32 Ret = MT_FAILURE;
    AOE_ENGINE_CHN_STATE_S *state = MT_NULL;

    CHECK_ENGINE_OPEN(enENGINE);
    state = (AOE_ENGINE_CHN_STATE_S * )g_AoeRm.hMix[enENGINE];

    if (AOE_ENGINE_STATUS_STOP == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    //Ret = iHAL_AOE_ENGINE_SetCmd(enENGINE, AOE_ENGINE_CMD_STOP);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    state->enCurnStatus = AOE_ENGINE_STATUS_STOP;
    return MT_SUCCESS;
}

mt_s32 HAL_AOE_ENGINE_AttachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP)
{
    CHECK_ENGINE_OPEN(enENGINE);
    CHECK_AIP_OPEN(enAIP);

    return iHAL_AOE_ENGINE_AttachAip(enENGINE, enAIP);
}

mt_s32 HAL_AOE_ENGINE_DetachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP)
{
    CHECK_ENGINE_OPEN(enENGINE);
    CHECK_AIP_OPEN(enAIP);

    return iHAL_AOE_ENGINE_DetachAip(enENGINE, enAIP);
}
mt_s32 HAL_AOE_ENGINE_AttachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP)
{
    CHECK_ENGINE_OPEN(enENGINE);
    CHECK_AOP_OPEN(enAOP);

    return iHAL_AOE_ENGINE_AttachAop(enENGINE, enAOP);
}

mt_s32 HAL_AOE_ENGINE_DetachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP)
{
    CHECK_ENGINE_OPEN(enENGINE);
    CHECK_AOP_OPEN(enAOP);

    return iHAL_AOE_ENGINE_DetachAop(enENGINE, enAOP);
}

mt_s32 HAL_AOE_ENGINE_AttachAef(AOE_ENGINE_ID_E enENGINE, mt_u32 u32AefId)
{
    CHECK_ENGINE_OPEN(enENGINE);

    return iHAL_AOE_ENGINE_AttachAef(enENGINE, u32AefId);
}

mt_s32 HAL_AOE_ENGINE_DetachAef(AOE_ENGINE_ID_E enENGINE, mt_u32 u32AefId)
{
    CHECK_ENGINE_OPEN(enENGINE);

    return iHAL_AOE_ENGINE_DetachAef(enENGINE, u32AefId);
}

mt_s32 HAL_AOE_ENGINE_GetStatus(AOE_ENGINE_ID_E enENGINE, mt_void *pstStatus)
{
    //TODO
    return MT_SUCCESS;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
