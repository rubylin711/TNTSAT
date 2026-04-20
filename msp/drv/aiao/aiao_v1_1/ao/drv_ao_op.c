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

#include "drv_ao_op.h"
#include "audio_util.h"
//#include "mt_audsp_aflt.h"  //TODO for AFLT_MAX_CHAN_NUM

extern void reg_sym_linux_04_reg_pcm_chan_spdif_bit(mt_u8 data);
extern void reg_sym_linux_pp_reg_chan_mod_bit(mt_u8 data);
extern mt_s32 HAL_SET_stCB(SND_CARD_STATE_S *pCard,AudioBufTypeAria CBType,ulong reg_base,ulong vaddr,mt_u32 sz);

static AIAO_PORT_ID_E SndOpGetPort(MT_UNF_SND_OUTPUTPORT_E enOutPort,SND_AOP_TYPE_E enType)
{
    AIAO_PORT_ID_E enPortId;

    switch(enOutPort)
    {
        case MT_UNF_SND_OUTPUTPORT_DAC0:
            enPortId = AIAO_PORT_TX2;
            break;
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC1:
            enPortId = AIAO_PORT_TX3;
            break;
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC2:
            enPortId = AIAO_PORT_TX4;
            break;
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC3:
            enPortId = AIAO_PORT_TX5;
            break;
        case MT_UNF_SND_OUTPUTPORT_SPDIF0:
            enPortId = AIAO_PORT_SPDIF_TX1;
            break;
        case MT_UNF_SND_OUTPUTPORT_HDMI0:
            if(SND_AOP_TYPE_I2S == enType)
            {
                 if(AUTIL_CMTP_PLATFORM_S40 == AUTIL_GetChipPlatform())
                 {
                    enPortId = AIAO_PORT_TX3;
                 }
                 else
                 {
                     enPortId = AIAO_PORT_TX6;
                 }
            }
            else
            {
                enPortId = AIAO_PORT_SPDIF_TX0;
            }
          break;
      default:
          MT_ERR_AO("Outport %d is invalid!\n ",(mt_u32)enOutPort);
          enPortId = AIAO_PORT_BUTT;
    }

    return enPortId;
}

MT_UNF_SND_OUTPUTPORT_E SndOpGetOutport(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enOutPort;
}

SND_OUTPUT_TYPE_E SndOpGetOutType(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enOutType;
}

mt_handle SNDGetOpHandleByOutPort(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort)
{
    mt_s32 u32PortNum;
    mt_handle hSndOp;

    for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
    {
        hSndOp = pCard->hSndOp[u32PortNum];
        if (hSndOp)
        {
            if (enOutPort == SndOpGetOutport(hSndOp))
            {
                return hSndOp;
            }
        }
    }

    return MT_NULL;
}

SND_ENGINE_TYPE_E SND_GetOpGetOutType(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enEngineType[state->ActiveId];
}

mt_handle SND_GetOpHandlebyOutType(SND_CARD_STATE_S *pCard, SND_OUTPUT_TYPE_E enOutType)
{
    mt_s32 u32PortNum;
    mt_handle hSndOp;

    for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
    {
        hSndOp = pCard->hSndOp[u32PortNum];
        if (hSndOp)
        {
            if (enOutType == SndOpGetOutType(hSndOp))
            {
                return hSndOp;
            }
        }
    }

    return MT_NULL;
}

mt_void SND_GetDelayMs(SND_CARD_STATE_S *pCard, mt_u32 *pdelayms)
{
    mt_handle hSndOp;
    SND_OP_STATE_S *state;

    hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_DAC);
    if(hSndOp)
    {
        state = (SND_OP_STATE_S *)hSndOp;
        HAL_AIAO_P_GetDelayMs(state->enPortID[state->ActiveId], pdelayms);
        return;
    }

    hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
    if(hSndOp)
    {
        state = (SND_OP_STATE_S *)hSndOp;
        HAL_AIAO_P_GetDelayMs(state->enPortID[state->ActiveId], pdelayms);
        return;
    }

    hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_SPDIF);
    if(hSndOp)
    {
        state = (SND_OP_STATE_S *)hSndOp;
        HAL_AIAO_P_GetDelayMs(state->enPortID[state->ActiveId], pdelayms);
        return;
    }

    hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_HDMI);
    if(hSndOp)
    {
        state = (SND_OP_STATE_S *)hSndOp;
        HAL_AIAO_P_GetDelayMs(state->enPortID[state->ActiveId], pdelayms);
        return;
    }

    *pdelayms = 0;

    return ;
}
//enSampleRate for support slic 8k
static mt_s32 SndOpCreateAop(SND_OP_STATE_S *state,MT_UNF_SND_OUTPORT_S *pstAttr,
	                         SND_AOP_TYPE_E enType,AO_ALSA_I2S_Param_S* pstAoI2sParam,
	                         MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    mt_s32 Ret;
    AIAO_PORT_USER_CFG_S stHwPortAttr;
    AIAO_PORT_ID_E enPort;
    AOE_AOP_ID_E enAOP;
    AOE_AOP_CHN_ATTR_S stAopAttr;
    mmz_buffer_s stRbfMmz;
    mt_u32 u32BufSize;
    AIAO_RBUF_ATTR_S stRbfAttr;
    mt_u32 u32AopId;
    MT_UNF_SND_OUTPUTPORT_E enOutPort;
   MT_BOOL bAlsaI2sUse = MT_FALSE;//MT_ALSA_I2S_ONLY_SUPPORT

#ifdef MT_ADAC_SLIC_SUPPORT
    mt_u32 u32AlignInBytes,u32FrameSize;
	u32AlignInBytes = AIAO_BUFFER_ADDR_ALIGN * 2 / 8;	 //translate bit to byte
#endif

    if(pstAoI2sParam != NULL)
      bAlsaI2sUse  = pstAoI2sParam->bAlsaI2sUse;//MT_ALSA_I2S_ONLY_SUPPORT

    if(enType >= SND_AOP_TYPE_CAST)
    {
        goto SndReturn_ERR_EXIT;
    }

    enOutPort = pstAttr->enOutPort;
    switch(enOutPort)
    {
        case MT_UNF_SND_OUTPUTPORT_DAC0:
            enPort = SndOpGetPort(enOutPort,enType);
            HAL_AIAO_P_GetTxI2SDfAttr(enPort, &stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_Adac0", MMZ_OTHERS, AO_DAC_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC1:
            enPort = SndOpGetPort(enOutPort,enType);
            HAL_AIAO_P_GetTxI2SDfAttr(enPort, &stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            stHwPortAttr.stIfAttr.eCrgSource = AIAO_TX_CRG2;
            MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_Adac1", MMZ_OTHERS, AO_DAC_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;
       case MT_UNF_SND_OUTPUTPORT_EXT_DAC2:
            enPort = SndOpGetPort(enOutPort,enType);
            HAL_AIAO_P_GetTxI2SDfAttr(enPort, &stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            stHwPortAttr.stIfAttr.eCrgSource = AIAO_TX_CRG2;
            MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_Adac2", MMZ_OTHERS, AO_DAC_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC3:
            enPort = SndOpGetPort(enOutPort,enType);
            HAL_AIAO_P_GetTxI2SDfAttr(enPort, &stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            stHwPortAttr.stIfAttr.eCrgSource = AIAO_TX_CRG2;
            MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_Adac3", MMZ_OTHERS, AO_DAC_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;
#if defined (MT_I2S0_SUPPORT)
        case MT_UNF_SND_OUTPUTPORT_I2S0:
            MT_ASSERT(pstAttr->unAttr.stI2sAttr.stAttr.enChannel==MT_UNF_I2S_CHNUM_2 ||
                          pstAttr->unAttr.stI2sAttr.stAttr.enChannel==MT_UNF_I2S_CHNUM_1);	//support slic mono
            MT_ASSERT(pstAttr->unAttr.stI2sAttr.stAttr.enBitDepth==MT_UNF_I2S_BIT_DEPTH_16);
            HAL_AIAO_P_GetBorardTxI2SDfAttr(0,&pstAttr->unAttr.stI2sAttr.stAttr,&enPort,&stHwPortAttr);
           if(bAlsaI2sUse == MT_TRUE)
            {
                 stRbfMmz.size                          = pstAoI2sParam->stBuf.u32BufSize;
                 u32BufSize                                = pstAoI2sParam->stBuf.u32BufSize;
                 MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
                 stRbfMmz.startPhyAddr                  = pstAoI2sParam->stBuf.u32BufPhyAddr;
                 stRbfMmz.startVirAddr                  = (void *)pstAoI2sParam->stBuf.u32BufVirAddr;
                 stHwPortAttr.pIsrFunc                     = (AIAO_IsrFunc *)pstAoI2sParam->IsrFunc;
                 stHwPortAttr.substream                    = pstAoI2sParam->substream;
				 stHwPortAttr.stIfAttr.enRate              = (AIAO_SAMPLE_RATE_E)pstAoI2sParam->enRate;
                 stHwPortAttr.stBufConfig.u32PeriodBufSize = pstAoI2sParam->stBuf.u32PeriodByteSize;
                 stHwPortAttr.stBufConfig.u32PeriodNumber  = pstAoI2sParam->stBuf.u32Periods;
                 Ret = MT_SUCCESS;
            }
            else
            {
#ifdef MT_ADAC_SLIC_SUPPORT
                if (MT_UNF_SAMPLE_RATE_8K == enSampleRate)
                {
                    //slic 8k 1ch , the sizeof aop size should > 512, so the smallest size  > 40ms
    			    u32FrameSize = AUTIL_CalcFrameSize((mt_u32)stHwPortAttr.stIfAttr.enChNum, (mt_u32)stHwPortAttr.stIfAttr.enBitDepth);
    			    stHwPortAttr.stBufConfig.u32PeriodBufSize = 40 * enSampleRate * u32FrameSize / 1000;	//8k 40ms
    			    stHwPortAttr.stBufConfig.u32PeriodBufSize = (stHwPortAttr.stBufConfig.u32PeriodBufSize / u32AlignInBytes) * u32AlignInBytes; //Align 256bit
                    MT_INFO_AO("Slic device aop buffersize force to 40ms !\n");
                }
#endif
                u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
                MT_ASSERT(u32BufSize < AO_I2S_MMZSIZE_MAX);
                Ret = mt_drv_mmz_alloc_and_map("AO_I2s0", MMZ_OTHERS, AO_I2S_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            }
            break;
#endif

#if defined (MT_I2S1_SUPPORT)
        case MT_UNF_SND_OUTPUTPORT_I2S1:
            MT_ASSERT(pstAttr->unAttr.stI2sAttr.stAttr.enChannel==MT_UNF_I2S_CHNUM_2);
            MT_ASSERT(pstAttr->unAttr.stI2sAttr.stAttr.enBitDepth==MT_UNF_I2S_BIT_DEPTH_16);
            HAL_AIAO_P_GetBorardTxI2SDfAttr(1,&pstAttr->unAttr.stI2sAttr.stAttr,&enPort,&stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            MT_ASSERT(u32BufSize < AO_I2S_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_I2s1", MMZ_OTHERS, AO_I2S_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;
#endif

        case MT_UNF_SND_OUTPUTPORT_SPDIF0:
            enPort = SndOpGetPort(enOutPort,enType);
            HAL_AIAO_P_GetTxSpdDfAttr(enPort, &stHwPortAttr);
            u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber;
            MT_ASSERT(u32BufSize < AO_SPDIF_MMZSIZE_MAX);
            Ret = mt_drv_mmz_alloc_and_map("AO_Spidf", MMZ_OTHERS, AO_SPDIF_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            break;

        case MT_UNF_SND_OUTPUTPORT_HDMI0:
            if(SND_AOP_TYPE_I2S == enType)
            {
                enPort = SndOpGetPort(enOutPort,enType);
                HAL_AIAO_P_GetHdmiI2SDfAttr(enPort, &stHwPortAttr);
#if 1 //def MT_ALSA_HDMI_ONLY_SUPPORT
                if(bAlsaI2sUse == MT_TRUE)
                 {
                      stRbfMmz.size                          = pstAoI2sParam->stBuf.u32BufSize;
                      u32BufSize                                = pstAoI2sParam->stBuf.u32BufSize;
                      MT_ASSERT(u32BufSize < AO_DAC_MMZSIZE_MAX);
                      stRbfMmz.startPhyAddr                  = pstAoI2sParam->stBuf.u32BufPhyAddr;
                      stRbfMmz.startVirAddr                  = (void *)pstAoI2sParam->stBuf.u32BufVirAddr;
                      stHwPortAttr.pIsrFunc                     = (AIAO_IsrFunc *)pstAoI2sParam->IsrFunc;
                      stHwPortAttr.substream                    = pstAoI2sParam->substream;
                      stHwPortAttr.stIfAttr.enRate              = (int)pstAoI2sParam->enRate;
                      stHwPortAttr.stBufConfig.u32PeriodBufSize = pstAoI2sParam->stBuf.u32PeriodByteSize;
                      stHwPortAttr.stBufConfig.u32PeriodNumber  = pstAoI2sParam->stBuf.u32Periods;
                      //stHwPortAttr.u32VolumedB                  = AUTIL_VolumeLinear2RegdB((mt_u32)g_stUserGain.s32Gain);
                      Ret = MT_SUCCESS;
                 }
                 else
#endif
                 {
                u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber * 32;  //verify considerate from 48k 2ch 16bit to 192k 8ch 32bit for example 24bit LPCM
                MT_ASSERT(u32BufSize < AO_HDMI_MMZSIZE_MAX);
                Ret = mt_drv_mmz_alloc_and_map("AO_HdmiI2s", MMZ_OTHERS, AO_HDMI_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
                  }
            }
            else
            {
                enPort = SndOpGetPort(enOutPort,enType);
                HAL_AIAO_P_GetTxSpdDfAttr(enPort, &stHwPortAttr);
                u32BufSize = stHwPortAttr.stBufConfig.u32PeriodBufSize * stHwPortAttr.stBufConfig.u32PeriodNumber * 4; //verify considerate frome 48k to 192k  for example ddp
                MT_ASSERT(u32BufSize < AO_SPDIF_MMZSIZE_MAX);
                Ret = mt_drv_mmz_alloc_and_map("AO_HdmiSpidf", MMZ_OTHERS, AO_SPDIF_MMZSIZE_MAX, AIAO_BUFFER_ADDR_ALIGN, &stRbfMmz);
            }
            break;

        default:
            MT_ERR_AO("Outport is invalid!\n");
            goto SndReturn_ERR_EXIT;
    }
    if (MT_SUCCESS != Ret)
    {
        goto SndReturn_ERR_EXIT;
    }

    stHwPortAttr.bExtDmaMem = MT_TRUE;
    stHwPortAttr.stExtMem.u32BufPhyAddr = stRbfMmz.startPhyAddr;
    stHwPortAttr.stExtMem.u32BufVirAddr = (ulong)stRbfMmz.startVirAddr;
    stHwPortAttr.stExtMem.u32BufSize = u32BufSize;
	stHwPortAttr.stIfAttr.enRate = (int)enSampleRate;
    Ret = HAL_AIAO_P_Open(enPort, &stHwPortAttr);
    if (MT_SUCCESS != Ret)
    {
        goto SndMMZRelease_ERR_EXIT;
    }

    memset(&stRbfAttr, 0, sizeof(AIAO_RBUF_ATTR_S));
    Ret = HAL_AIAO_P_GetRbfAttr(enPort, &stRbfAttr);
    if(MT_SUCCESS != Ret)
    {
        goto SndClosePort_ERR_EXIT;
    }

    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyAddr = stRbfAttr.u32BufPhyAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirAddr = stRbfAttr.u32BufVirAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyWptr = stRbfAttr.u32BufPhyWptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirWptr = stRbfAttr.u32BufVirWptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyRptr = stRbfAttr.u32BufPhyRptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirRptr = stRbfAttr.u32BufVirRptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufWptrRptrFlag = 1;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufSize = stRbfAttr.u32BufSize;
    stAopAttr.stRbfOutAttr.u32BufBitPerSample = stHwPortAttr.stIfAttr.enBitDepth;
    stAopAttr.stRbfOutAttr.u32BufChannels   = stHwPortAttr.stIfAttr.enChNum;
    stAopAttr.stRbfOutAttr.u32BufSampleRate = stHwPortAttr.stIfAttr.enRate;
    stAopAttr.stRbfOutAttr.u32BufDataFormat = 0;
    stAopAttr.stRbfOutAttr.bRbfHwPriority = MT_TRUE;
    stAopAttr.stRbfOutAttr.u32BufLatencyThdMs = AOE_AOP_BUFF_LATENCYMS_DF;
    Ret = HAL_AOE_AOP_Create(&enAOP, &stAopAttr);
    if (MT_SUCCESS != Ret)
    {
       goto SndClosePort_ERR_EXIT;
    }

    if(MT_UNF_SND_OUTPUTPORT_HDMI0 != enOutPort || SND_AOP_TYPE_SPDIF != enType)  //spdif interface of hdmi don't start aop
    {
        Ret = HAL_AIAO_P_Start(enPort);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AIAO_P_Start(%d) failed\n", enPort);
            goto SndDestroyAOP_ERR_EXIT;
        }

        Ret = HAL_AOE_AOP_Start(enAOP);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AOP_Start(%d) failed\n", enAOP);
            goto SndStopPort_ERR_EXIT;
        }
    }

    u32AopId = (mt_u32)enType;
    state->u32OpMask |= 1 << u32AopId;
    if(MT_UNF_SND_OUTPUTPORT_HDMI0 != enOutPort || SND_AOP_TYPE_SPDIF != enType)  //spdif interface of hdmi is not active
    {
        state->ActiveId  = u32AopId;
    }
    state->enPortID[u32AopId] = enPort;
    state->enAOP[u32AopId] = enAOP;
    state->stRbfMmz[u32AopId]   = stRbfMmz;
    state->stPortUserAttr[u32AopId] = stHwPortAttr;
    state->enEngineType[u32AopId] = SND_ENGINE_TYPE_PCM;
    return MT_SUCCESS;

SndStopPort_ERR_EXIT:
    (mt_void)HAL_AIAO_P_Stop(enPort, AIAO_STOP_IMMEDIATE);
SndDestroyAOP_ERR_EXIT:
    HAL_AOE_AOP_Destroy(enAOP);
SndClosePort_ERR_EXIT:
    HAL_AIAO_P_Close(enPort);
SndMMZRelease_ERR_EXIT:
    mt_drv_mmz_unmap_and_release(&stRbfMmz);
SndReturn_ERR_EXIT:
    return MT_FAILURE;
}

SND_ENGINE_TYPE_E SND_OpGetEngineType(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enEngineType[state->ActiveId];
}
#ifdef MT_SND_CAST_SUPPORT
static mt_s32 SndOpCreateCast(mt_handle *phSndOp, mt_handle *phCast, MT_UNF_SND_CAST_ATTR_S *pstUserCastAttr,
                              mmz_buffer_s *pstMMz)
{
    mt_s32 Ret;
    AOE_AOP_ID_E enAOP;
    AOE_AOP_CHN_ATTR_S stAopAttr;
    mt_u32 uBufSize, uFrameSize;
    AIAO_CAST_ATTR_S stCastAttr;
    AIAO_CAST_ID_E enCast;
    SND_OP_STATE_S *state = MT_NULL;
    MT_UNF_SND_CAST_ATTR_S stUserCastAttr;

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_OP_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AIAO("malloc SndOpCreate failed\n");
        return MT_FAILURE;
    }

    memset(state, 0, sizeof(SND_OP_STATE_S));
    memcpy(&stUserCastAttr, pstUserCastAttr, sizeof(MT_UNF_SND_CAST_ATTR_S));
    stCastAttr.u32BufChannels = 2;
    stCastAttr.u32BufBitPerSample = 16;
    stCastAttr.u32BufSampleRate = 48000;
    stCastAttr.u32BufDataFormat = 0;

    uFrameSize = AUTIL_CalcFrameSize(stCastAttr.u32BufChannels, stCastAttr.u32BufBitPerSample);
    uBufSize = stUserCastAttr.u32PcmFrameMaxNum * stUserCastAttr.u32PcmSamplesPerFrame * uFrameSize;
    stCastAttr.u32BufLatencyThdMs = AUTIL_ByteSize2LatencyMs(uBufSize, uFrameSize, stCastAttr.u32BufSampleRate);

    MT_ASSERT((uBufSize) < AO_CAST_MMZSIZE_MAX);

    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyAddr = pstMMz->startPhyAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirAddr = (ulong)pstMMz->startVirAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufSize = uBufSize;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufWptrRptrFlag = 0;  /* cast use aop Wptr&Rptr avoid dsp cache problem */
    stAopAttr.stRbfOutAttr.u32BufBitPerSample = stCastAttr.u32BufBitPerSample;
    stAopAttr.stRbfOutAttr.u32BufChannels   = stCastAttr.u32BufChannels;
    stAopAttr.stRbfOutAttr.u32BufSampleRate = stCastAttr.u32BufSampleRate;
    stAopAttr.stRbfOutAttr.u32BufDataFormat = 0;
    stAopAttr.stRbfOutAttr.bRbfHwPriority = MT_FALSE;
    stAopAttr.stRbfOutAttr.u32BufLatencyThdMs = AOE_AOP_BUFF_LATENCYMS_DF;
    Ret = HAL_AOE_AOP_Create(&enAOP, &stAopAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AIAO("AOP_Create failed\n");
        AUTIL_AO_FREE(MT_ID_AO, (mt_void *)state);
        return MT_FAILURE;
    }

    stCastAttr.extDmaMem.u32BufPhyAddr = pstMMz->startPhyAddr;
    stCastAttr.extDmaMem.u32BufVirAddr = (ulong)pstMMz->startVirAddr;
    stCastAttr.extDmaMem.u32BufSize = uBufSize;
    iHAL_AOE_AOP_GetRptrAndWptrRegAddr(enAOP, &stCastAttr.extDmaMem.u32WptrAddr, &stCastAttr.extDmaMem.u32RptrAddr);
    Ret = HAL_CAST_Create(&enCast, &stCastAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AIAO("Cast_Create failed\n");
	    HAL_AOE_AOP_Destroy(enAOP);
        AUTIL_AO_FREE(MT_ID_AO, (mt_void *)state);
        return MT_FAILURE;
    }

    state->u32OpMask |= 1<<SND_AOP_TYPE_CAST;
    state->ActiveId = 0;
    state->enAOP[0] = enAOP;

    memcpy(&state->stCastAttr, &stCastAttr, sizeof(AIAO_CAST_ATTR_S));

    state->CastId = enCast;
    state->enEngineType[0] = SND_ENGINE_TYPE_PCM;
    state->enCurnStatus = SND_OP_STATUS_STOP;
    state->enOutType = SND_OUTPUT_TYPE_CAST;

    *phCast  = (mt_handle )enCast;
    *phSndOp = (mt_handle )state;

    return MT_SUCCESS;
}

#endif

static mt_void SndOpGetSubFormatAttr(SND_OP_STATE_S *state, SND_OP_ATTR_S *pstSndPortAttr)
{
    AOE_AOP_CHN_ATTR_S stAopAttr;
    AOE_AOP_OUTBUF_ATTR_S *pstAttr;
    AIAO_BufAttr_S *pstAiAOBufAttr;

    HAL_AOE_AOP_GetAttr(state->enAOP[state->ActiveId], &stAopAttr);
    pstAttr = &stAopAttr.stRbfOutAttr;
    pstAiAOBufAttr = &state->stPortUserAttr[state->ActiveId].stBufConfig;

    pstSndPortAttr->u32Channels     = pstAttr->u32BufChannels;
    pstSndPortAttr->u32SampleRate   = pstAttr->u32BufSampleRate;
    pstSndPortAttr->u32BitPerSample = pstAttr->u32BufBitPerSample;
    pstSndPortAttr->u32DataFormat   = pstAttr->u32BufDataFormat;
    pstSndPortAttr->u32LatencyThdMs = pstAttr->u32BufLatencyThdMs;
    if (SND_OUTPUT_TYPE_CAST != state->enOutType)
    {
        pstSndPortAttr->u32PeriodBufSize = pstAiAOBufAttr->u32PeriodBufSize;
        pstSndPortAttr->u32PeriodNumber  = pstAiAOBufAttr->u32PeriodNumber;
    }
    return;
}

/*
//zgjiere, unf proc 中断，自动注册 AIAO_IsrFunc      *pIsrFunc;
//zgjiere, alsa 中断如何注册，alsa中断在track确定，aiao中断在port打开确定?
1) alsa能否采用DSP2ARM中断?
2) 运行过程，修改中断服务程序? */
mt_s32 SndOpStart(mt_handle hSndOp, mt_void *pstParams)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];
    AOE_AOP_ID_E enAOP = state->enAOP[state->ActiveId];
    mt_s32 Ret;

    if (SND_OP_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
#ifdef MT_SND_CAST_SUPPORT
        Ret = HAL_CAST_Start(state->CastId);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_CAST_Start(%d) failed\n", state->CastId);
            return MT_FAILURE;
        }

        Ret = HAL_AOE_AOP_Start(enAOP);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AOP_Start(%d) failed\n", enAOP);
            return MT_FAILURE;
        }
#endif
    }
    else
    {
        Ret = HAL_AIAO_P_Start(enPort);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AIAO_P_Start(%d) failed\n", enPort);
            return MT_FAILURE;
        }

        Ret = HAL_AOE_AOP_Start(enAOP);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AOP_Start(%d) failed\n", enAOP);
            return MT_FAILURE;
        }
    }

    state->enCurnStatus = SND_OP_STATUS_START;

    return MT_SUCCESS;
}

mt_s32 SndOpStop(mt_handle hSndOp, mt_void *pstParams)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];
    AOE_AOP_ID_E enAOP = state->enAOP[state->ActiveId];
    mt_s32 Ret;

    if (SND_OP_STATUS_STOP == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
#ifdef MT_SND_CAST_SUPPORT
        Ret = HAL_CAST_Stop(state->CastId);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_CAST_Stop(%d) failed\n", state->CastId);
            return MT_FAILURE;
        }

        Ret = HAL_AOE_AOP_Stop(enAOP);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AOP_Stop(%d) failed\n", enAOP);
            return MT_FAILURE;
        }
#endif
    }
    else
    {
        Ret = HAL_AIAO_P_Stop(enPort, AIAO_STOP_IMMEDIATE);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AIAO_P_Stop(%d) failed\n", enPort);
            return MT_FAILURE;
        }

        Ret = HAL_AOE_AOP_Stop(enAOP);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AOP_Stop(%d) failed\n", enAOP);
            return MT_FAILURE;
        }
    }
    state->enCurnStatus = SND_OP_STATUS_STOP;
    return MT_SUCCESS;
}



static mt_void SndOpDestroyAop(SND_OP_STATE_S *state, SND_AOP_TYPE_E enType)
{
    HAL_AOE_AOP_Destroy(state->enAOP[enType]);
    HAL_AIAO_P_Close(state->enPortID[enType]);
    mt_drv_mmz_unmap_and_release(&state->stRbfMmz[enType]);
    return;
}

static mt_void SndOpDestroy(mt_handle hSndOp, MT_BOOL bSuspend)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    SND_AOP_TYPE_E type;

    if(0 == state->u32OpMask)
    {
        return;
    }

    SndOpStop(hSndOp, MT_NULL);



    if(state->enOutType == SND_OUTPUT_TYPE_CAST)
    {
#ifdef MT_SND_CAST_SUPPORT
        HAL_AOE_AOP_Destroy(state->enAOP[0]);
        HAL_CAST_Destroy(state->CastId);
#endif
    }
    else
    {
        for(type = SND_AOP_TYPE_I2S; type < SND_AOP_TYPE_CAST; type++)
        {
            if (state->u32OpMask & (1 << type))
            {
                SndOpDestroyAop(state, type);
            }
        }
        if(state->enOutType == SND_OUTPUT_TYPE_DAC)
        {
        	#ifdef MT_TIANLAI_V500
		       	ADAC_TIANLAI_DeInit(bSuspend);
			#endif
        }
    }

    memset(state, 0, sizeof(SND_OP_STATE_S));
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return;
}

static mt_void SndOpInitState(SND_OP_STATE_S *state)
{
    mt_u32 idx;

    memset(state, 0, sizeof(SND_OP_STATE_S));
    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        state->enPortID[idx] = AIAO_PORT_BUTT;
        state->enAOP[idx] = AOE_AOP_BUTT;
        state->enEngineType[idx] = SND_ENGINE_TYPE_BUTT;
    }
#ifdef MT_SND_AMP_SUPPORT
    state->pstAmpFunc = MT_NULL;
#endif
    return;
}
static mt_s32 SndOpSetSPDIFCategoryCode(mt_handle hSndOp, MT_UNF_SND_SPDIF_CATEGORYCODE_E enCategoryCode)
{
    mt_u32 idx;
    AIAO_PORT_ID_E enPort;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    mt_s32 Ret = MT_FAILURE;
    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enPort = state->enPortID[idx];
        if(enPort < AIAO_PORT_BUTT)
        {
            Ret = HAL_AIAO_P_SetSpdifCategoryCode(enPort, (AIAO_SPDIF_CATEGORYCODE_E)enCategoryCode);
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AIAO("HAL_AIAO_P_SetSpdifCategoryCode port:%d, Category Code: %d\n", (mt_u32)enPort, (mt_u32)enCategoryCode);
                return MT_FAILURE;
            }
        }
    }
    state->enUserSPDIFCategoryCode= enCategoryCode;
    return MT_SUCCESS;
}
static mt_s32 SndOpSetSPDIFSCMSMode(mt_handle hSndOp, MT_UNF_SND_SPDIF_SCMSMODE_E enSCMSMode)
{
    mt_u32 idx;
    AIAO_PORT_ID_E enPort;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    mt_s32 Ret = MT_FAILURE;
    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enPort = state->enPortID[idx];
        if(enPort < AIAO_PORT_BUTT)
        {
            Ret = HAL_AIAO_P_SetSpdifSCMSMode(enPort, (AIAO_SPDIF_SCMS_MODE_E)enSCMSMode);
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AIAO("HAL_AIAO_P_SetSpdifSCMSMode port:%d, SCMSMode: %d\n", (mt_u32)enPort, (mt_u32)enSCMSMode);
                return MT_FAILURE;
            }
        }
    }
    state->enUserSPDIFSCMSMode = enSCMSMode;
    return MT_SUCCESS;
}
#if 1//def MT_ALSA_I2S_ONLY_SUPPORT
mt_s32 AlsaHwSndOpStart(mt_handle hSndOp, mt_void *pstParams)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];
    mt_s32 Ret;
    if (SND_OP_STATUS_START == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

     if((SND_OUTPUT_TYPE_I2S == state->enOutType)||(SND_OUTPUT_TYPE_HDMI == state->enOutType))
       {
           Ret = HAL_AIAO_P_Start(enPort);
           if (MT_SUCCESS != Ret)
           {
               MT_ERR_AO("HAL_AIAO_P_Start(%d) failed\n", enPort);
               return MT_FAILURE;
           }
       }
     state->enCurnStatus = SND_OP_STATUS_START;
     return MT_SUCCESS;
}
mt_s32 AlsaHwSndOpStop(mt_handle hSndOp, mt_void *pstParams)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];
    mt_s32 Ret;
    if (SND_OP_STATUS_STOP == state->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    if((SND_OUTPUT_TYPE_I2S == state->enOutType)||(SND_OUTPUT_TYPE_HDMI == state->enOutType))
    {
        Ret = HAL_AIAO_P_Stop(enPort, AIAO_STOP_IMMEDIATE);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AIAO_P_Stop(%d) failed\n", enPort);
            return MT_FAILURE;
        }
    }
    state->enCurnStatus = SND_OP_STATUS_STOP;
    return MT_SUCCESS;
}
#endif

static mt_s32 SndOpCreate(mt_handle *phSndOp,MT_UNF_SND_OUTPORT_S *pstAttr,
                          AO_ALSA_I2S_Param_S *pstAoI2sParam, MT_BOOL bResume,
                          MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    SND_OP_STATE_S *state = MT_NULL;
    mt_s32 Ret = MT_FAILURE;
    SND_AOP_TYPE_E AopType;

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_OP_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc SndOpCreate failed\n");
        return MT_FAILURE;
    }

    SndOpInitState(state);

    for(AopType = SND_AOP_TYPE_I2S; AopType < SND_AOP_TYPE_CAST; AopType++)
    {
        if(SND_AOP_TYPE_I2S == AopType && MT_UNF_SND_OUTPUTPORT_SPDIF0 == pstAttr->enOutPort)
        {
            continue;
        }

        if(SND_AOP_TYPE_SPDIF == AopType && MT_UNF_SND_OUTPUTPORT_SPDIF0 != pstAttr->enOutPort && MT_UNF_SND_OUTPUTPORT_HDMI0 != pstAttr->enOutPort)
        {
            continue;
        }

        if(MT_SUCCESS != SndOpCreateAop(state, pstAttr, AopType, pstAoI2sParam, enSampleRate))
        {
            goto SndCreatePort_ERR_EXIT;
        }
    }

    SndOpGetSubFormatAttr(state, &state->stSndPortAttr);
    switch (pstAttr->enOutPort)
    {
    case MT_UNF_SND_OUTPUTPORT_DAC0:
    case MT_UNF_SND_OUTPUTPORT_EXT_DAC1:
    case MT_UNF_SND_OUTPUTPORT_EXT_DAC2:
    case MT_UNF_SND_OUTPUTPORT_EXT_DAC3:
        memcpy(&state->stSndPortAttr.unAttr, &pstAttr->unAttr, sizeof(MT_UNF_SND_DAC_ATTR_S));
        state->enOutType = SND_OUTPUT_TYPE_DAC;
        //init tianlai

#if defined (MT_TIANLAI_V500)
            //init tianlai
            ADAC_TIANLAI_Init(state->stSndPortAttr.u32SampleRate, bResume);
#endif
        break;

    case MT_UNF_SND_OUTPUTPORT_I2S0:
    case MT_UNF_SND_OUTPUTPORT_I2S1:
        memcpy(&state->stSndPortAttr.unAttr, &pstAttr->unAttr, sizeof(MT_UNF_SND_I2S_ATTR_S));
        state->enOutType = SND_OUTPUT_TYPE_I2S;
        break;

    case MT_UNF_SND_OUTPUTPORT_SPDIF0:
        memcpy(&state->stSndPortAttr.unAttr, &pstAttr->unAttr, sizeof(MT_UNF_SND_SPDIF_ATTR_S));
        state->enUserSPDIFSCMSMode = MT_UNF_SND_SPDIF_SCMSMODE_COPYPROHIBITED;
        state->enUserSPDIFCategoryCode = MT_UNF_SND_SPDIF_CATEGORY_GENERAL;
        SndOpSetSPDIFSCMSMode((mt_handle)state, state->enUserSPDIFSCMSMode);
        SndOpSetSPDIFCategoryCode((mt_handle)state, state->enUserSPDIFCategoryCode);
        state->enOutType = SND_OUTPUT_TYPE_SPDIF;
        break;

    case MT_UNF_SND_OUTPUTPORT_HDMI0:
        memcpy(&state->stSndPortAttr.unAttr, &pstAttr->unAttr, sizeof(MT_UNF_SND_HDMI_ATTR_S));
        state->enOutType = SND_OUTPUT_TYPE_HDMI;
        break;

    default:
        MT_ERR_AO("Err OutPort Type!\n");
        goto SndCreatePort_ERR_EXIT;
    }



    state->enOutPort = pstAttr->enOutPort;
    state->enCurnStatus = SND_OP_STATUS_START;
    state->u32UserMute = 0;
    state->enUserTrackMode = MT_UNF_TRACK_MODE_STEREO;
    state->stUserGain.bLinearMode = MT_FALSE;
    state->stUserGain.s32Gain = 0;
    *phSndOp = (mt_handle)state;

    return MT_SUCCESS;

SndCreatePort_ERR_EXIT:
    *phSndOp = (mt_handle)MT_NULL;
    SndOpDestroy((mt_handle)state, MT_FALSE);
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return Ret;
}

mt_s32 SndOpSetAttr(mt_handle hSndOp, SND_OP_ATTR_S *pstSndPortAttr)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    mt_s32 Ret = MT_FAILURE;
    AIAO_PORT_ID_E enPortID;
    AOE_AOP_ID_E enAOP;
    AIAO_PORT_ATTR_S stAiaoAttr;
    AOE_AOP_CHN_ATTR_S stAopAttr;
    AIAO_RBUF_ATTR_S stRbfAttr;

    if (SND_OP_STATUS_STOP != state->enCurnStatus)
    {
        return MT_FAILURE;
    }

#if defined(SND_CAST_SUPPORT)
    /* note: noly spdif & hdmi support set attr as pass-through switch */
    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
        return MT_FAILURE;
    }
#endif
    if (SND_OUTPUT_TYPE_DAC == state->enOutType)
    {
        return MT_FAILURE;
    }

    if (SND_OUTPUT_TYPE_I2S == state->enOutType)
    {
        return MT_FAILURE;
    }

    /* note: hdmi tx use spdif or i2s interface at diffrerent data format */
    if (SND_OUTPUT_TYPE_HDMI == state->enOutType)
    {
        if(!pstSndPortAttr->u32DataFormat)
        {
            state->ActiveId = SND_AOP_TYPE_I2S;  //2.0 pcm
        }
        else if (AUTIL_isIEC61937Hbr(pstSndPortAttr->u32DataFormat, pstSndPortAttr->u32SampleRate))
        {
            state->ActiveId = SND_AOP_TYPE_I2S;       // hbr
            if (IEC61937_DATATYPE_DOLBY_DIGITAL_PLUS == pstSndPortAttr->u32DataFormat)
            {
                state->ActiveId = SND_AOP_TYPE_SPDIF; // lbr or hbr(ddp)
            }
        }
        else if(IEC61937_DATATYPE_71_LPCM == pstSndPortAttr->u32DataFormat)
        {
            state->ActiveId = SND_AOP_TYPE_I2S;   //7.1 lpcm
        }
        else
        {
            state->ActiveId = SND_AOP_TYPE_SPDIF;     // lbr or hbr(ddp)
        }
    }

    enPortID = state->enPortID[state->ActiveId];
    enAOP = state->enAOP[state->ActiveId];

    HAL_AIAO_P_GetAttr(enPortID, &stAiaoAttr);
    HAL_AOE_AOP_GetAttr(enAOP, &stAopAttr);

    stAiaoAttr.stIfAttr.enBitDepth = (AIAO_BITDEPTH_E)pstSndPortAttr->u32BitPerSample;
    stAiaoAttr.stIfAttr.enChNum = (AIAO_I2S_CHNUM_E)pstSndPortAttr->u32Channels;
    stAiaoAttr.stIfAttr.enRate = (AIAO_SAMPLE_RATE_E)pstSndPortAttr->u32SampleRate;
    if (pstSndPortAttr->u32DataFormat)
    {
        AIAO_HAL_P_SetBypass(enPortID, MT_TRUE);
        if (SND_OUTPUT_TYPE_HDMI == state->enOutType)
        {
            state->enEngineType[state->ActiveId] = SND_ENGINE_TYPE_HDMI_RAW;
        }
        else if (SND_OUTPUT_TYPE_SPDIF == state->enOutType)
        {
            state->enEngineType[state->ActiveId] = SND_ENGINE_TYPE_SPDIF_RAW;
        }
    }
    else
    {
        AIAO_HAL_P_SetBypass(enPortID, MT_FALSE);
        state->enEngineType[state->ActiveId] = SND_ENGINE_TYPE_PCM;
    }

    //todo, optimize u32PeriodBufSize & u32PeriodNumber
    stAiaoAttr.stBufConfig.u32PeriodBufSize = pstSndPortAttr->u32PeriodBufSize;
    stAiaoAttr.stBufConfig.u32PeriodNumber = pstSndPortAttr->u32PeriodNumber;
    Ret = HAL_AIAO_P_SetAttr(enPortID, &stAiaoAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AO("HAL_AIAO_P_SetAttr port:0x%x\n", enPortID);
        return MT_FAILURE;
    }

    memset(&stRbfAttr, 0, sizeof(AIAO_RBUF_ATTR_S));
    Ret= HAL_AIAO_P_GetRbfAttr(enPortID, &stRbfAttr);
    if(MT_SUCCESS != Ret)
    {
        return Ret;
    }
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyAddr = stRbfAttr.u32BufPhyAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirAddr = stRbfAttr.u32BufVirAddr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyWptr = stRbfAttr.u32BufPhyWptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirWptr = stRbfAttr.u32BufVirWptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufPhyRptr = stRbfAttr.u32BufPhyRptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufVirRptr = stRbfAttr.u32BufVirRptr;
    stAopAttr.stRbfOutAttr.stRbfAttr.u32BufSize    = stRbfAttr.u32BufSize;

    stAopAttr.stRbfOutAttr.u32BufBitPerSample = pstSndPortAttr->u32BitPerSample;
    stAopAttr.stRbfOutAttr.u32BufChannels   = pstSndPortAttr->u32Channels;
    stAopAttr.stRbfOutAttr.u32BufSampleRate = pstSndPortAttr->u32SampleRate;
    stAopAttr.stRbfOutAttr.u32BufDataFormat = pstSndPortAttr->u32DataFormat;
    Ret = HAL_AOE_AOP_SetAttr(enAOP, &stAopAttr);
    if (MT_SUCCESS != Ret)
    {
        return MT_FAILURE;
    }

    state->stPortUserAttr[state->ActiveId].stIfAttr = stAiaoAttr.stIfAttr;
    state->stPortUserAttr[state->ActiveId].stBufConfig = stAiaoAttr.stBufConfig;
    memcpy(&state->stSndPortAttr, pstSndPortAttr, sizeof(SND_OP_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 SndOpGetAttr(mt_handle hSndOp, SND_OP_ATTR_S *pstSndPortAttr)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    memcpy(pstSndPortAttr, &state->stSndPortAttr, sizeof(SND_OP_ATTR_S));
    return MT_SUCCESS;
}

mt_s32 SndOpGetStatus(mt_handle hSndOp, AIAO_PORT_STAUTS_S *pstPortStatus)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];
    mt_s32 Ret;

    Ret = HAL_AIAO_P_GetStatus(enPort, pstPortStatus);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AIAO("HAL_AIAO_P_GetStatus (port:%d) failed\n", (mt_u32)enPort);
        return Ret;
    }

    return MT_SUCCESS;

}


AOE_AOP_ID_E SND_OpGetAopId(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enAOP[state->ActiveId];
}

MT_UNF_SND_OUTPUTPORT_E SND_GetOpOutputport(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;

    return state->enOutPort;
}

mt_s32 SndOpSetSampleRate(mt_handle hSndOp, MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AIAO_PORT_ID_E enPort = state->enPortID[state->ActiveId];

    mt_s32 Ret = MT_FAILURE;

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
        // todo, cast SampleRate
        return MT_SUCCESS;
    }

    Ret = HAL_AIAO_P_SetSampleRate(enPort, enSampleRate);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AIAO("HAL_AIAO_P_SetSampleRate port:%d \n", enPort);
        return MT_FAILURE;
    }

    state->enSampleRate = enSampleRate;

    return MT_SUCCESS;
}

mt_s32 SndOpSetVolume(mt_handle hSndOp, MT_UNF_SND_GAIN_ATTR_S stGain)
{
    mt_u32 idx;
    AIAO_PORT_ID_E enPort;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    mt_s32 Ret = MT_FAILURE;
    mt_u32 u32dBReg;

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
        // todo, cast Volume
        return MT_SUCCESS;
    }

    if (MT_TRUE == stGain.bLinearMode)
    {
        u32dBReg = AUTIL_VolumeLinear2RegdB((mt_u32)stGain.s32Gain);
    }
    else
    {
        u32dBReg = AUTIL_VolumedB2RegdB(stGain.s32Gain);
    }

    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enPort = state->enPortID[idx];
        if(enPort < AIAO_PORT_BUTT)
        {
            Ret = HAL_AIAO_P_SetVolume(enPort, u32dBReg);
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AIAO("HAL_AIAO_P_SetVolume port:%d, VolunedB: %d\n", (mt_u32)enPort, stGain.s32Gain);
                return MT_FAILURE;
            }
        }
    }

    state->stUserGain.bLinearMode = stGain.bLinearMode;
    state->stUserGain.s32Gain = stGain.s32Gain;

    return MT_SUCCESS;
}

mt_s32 SndOpSetTrackMode(mt_handle hSndOp, MT_UNF_TRACK_MODE_E enMode)
{
    mt_u32 idx;
    AIAO_PORT_ID_E enPort;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    mt_s32 Ret = MT_FAILURE;

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
        // todo, cast TrackMode
        return MT_SUCCESS;
    }

    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enPort = state->enPortID[idx];
        if(enPort < AIAO_PORT_BUTT)
        {
            Ret = HAL_AIAO_P_SetTrackMode(enPort, AUTIL_TrackModeTransform(enMode));
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AIAO("HAL_AIAO_P_SetTrackMode port:%d\n", (mt_u32)enPort);
                return MT_FAILURE;
            }
        }
    }

    state->enUserTrackMode = enMode;

    return MT_SUCCESS;
}

mt_s32 SndOpSetAefBypass(mt_handle hSndOp, MT_BOOL bBypass)
{
    mt_s32 Ret;
    mt_u32 idx;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    AOE_AOP_ID_E enAOP;

    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enAOP = state->enAOP[idx];
        if(enAOP < AOE_AOP_BUTT)
        {
            Ret = HAL_AOE_AOP_SetAefBypass(enAOP, bBypass);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AO("HAL_AOE_AOP_SetAefBypass(%d) failed\n", enAOP);
                return MT_FAILURE;
            }
        }
    }

    state->bBypass = bBypass;

    return MT_SUCCESS;
}
mt_s32 SndOpSetMute(mt_handle hSndOp, mt_u32 u32Mute)
{
    mt_u32 idx;
    AIAO_PORT_ID_E enPort;
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    MT_BOOL bMute = (u32Mute == 0) ? MT_FALSE : MT_TRUE;
    mt_s32 Ret = MT_FAILURE;

    if (SND_OUTPUT_TYPE_CAST == state->enOutType)
    {
        // todo, cast mute
        return MT_SUCCESS;
    }

    for(idx = 0; idx < AO_SNDOP_MAX_AOP_NUM; idx++)
    {
        enPort = state->enPortID[idx];
        if(enPort < AIAO_PORT_BUTT)
        {
            Ret = HAL_AIAO_P_Mute(enPort, bMute);
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AO("HAL_AIAO_P_Mute port:%d, Mute: %d\n", (mt_u32)enPort, (mt_u32)bMute);
                return MT_FAILURE;
            }
        }
    }

#ifdef MT_SND_AMP_SUPPORT
    if(state->pstAmpFunc)
    {
        if(state->pstAmpFunc->pfnAMP_SetMute)
        {
            Ret = (state->pstAmpFunc->pfnAMP_SetMute)(bMute);
            if (MT_SUCCESS != Ret)
            {
                MT_FATAL_AO("Amp mute failed\n");
                return MT_FAILURE;
            }
        }
    }
#endif

    state->u32UserMute = u32Mute;

    return MT_SUCCESS;
}

mt_s32 SndOpGetMute(mt_handle hSndOp)
{
    SND_OP_STATE_S *state = (SND_OP_STATE_S *)hSndOp;
    return state->u32UserMute;
}

#ifdef MT_SND_MUTECTL_SUPPORT
static mt_void SndOpDisableMuteCtrl(mt_u32 u32Card)
{
    SND_CARD_STATE_S *pCard = (SND_CARD_STATE_S *)u32Card;
    if (pCard->pstGpioFunc && pCard->pstGpioFunc->pfnGpioDirSetBit)
    {
        (pCard->pstGpioFunc->pfnGpioDirSetBit)(MT_SND_MUTECTL_GPIO, 0); //output
    }
    if (pCard->pstGpioFunc && pCard->pstGpioFunc->pfnGpioWriteBit)
    {
        (pCard->pstGpioFunc->pfnGpioWriteBit)(MT_SND_MUTECTL_GPIO, ((0 == MT_SND_MUTECTL_LEVEL) ? 1 : 0));
    }
	ADAC_FastPowerEnable(MT_FALSE);    //diable fast power up
    return;
}

static mt_void SndOpEnableMuteCtrl(mt_u32 u32Card)
{
    SND_CARD_STATE_S *pCard = (SND_CARD_STATE_S *)u32Card;
    if (pCard->pstGpioFunc && pCard->pstGpioFunc->pfnGpioDirSetBit)
    {
        (pCard->pstGpioFunc->pfnGpioDirSetBit)(MT_SND_MUTECTL_GPIO, 0); //output
    }
    if (pCard->pstGpioFunc && pCard->pstGpioFunc->pfnGpioWriteBit)
    {
        (pCard->pstGpioFunc->pfnGpioWriteBit)(MT_SND_MUTECTL_GPIO, ((0 == MT_SND_MUTECTL_LEVEL) ? 0 : 1));
    }
	ADAC_FastPowerEnable(MT_TRUE);     //enable fast power up
    return;
}

#endif


mt_void SND_DestroyOp(SND_CARD_STATE_S *pCard, MT_BOOL bSuspend)
{
    mt_s32 u32PortNum;

    for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
    {
        if (pCard->hSndOp[u32PortNum])
        {
#ifdef MT_SND_MUTECTL_SUPPORT
            if(MT_UNF_SND_OUTPUTPORT_DAC0 == ((SND_OP_STATE_S *)pCard->hSndOp[u32PortNum])->enOutPort)
            {
                //if(MT_TRUE == bSuspend)
                {
                    SndOpEnableMuteCtrl((mt_u32)pCard);
                }
                del_timer(&pCard->stMuteDisableTimer);
            }
#endif
            SndOpDestroy(pCard->hSndOp[u32PortNum], bSuspend);
            pCard->hSndOp[u32PortNum] = MT_NULL;
        }
    }

    return;
}

mt_s32 SND_CreateOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_ATTR_S *pstAttr,
                          AO_ALSA_I2S_Param_S* pstAoI2sParam, MT_BOOL bResume)
{
    mt_u32 u32PortNum;
    mt_handle hSndOp;

    for (u32PortNum = 0; u32PortNum < pstAttr->u32PortNum; u32PortNum++)
    {
        if (MT_SUCCESS != SndOpCreate(&hSndOp,&pstAttr->stOutport[u32PortNum],pstAoI2sParam, bResume,pstAttr->enSampleRate))
        {
            goto SND_CreateOp_ERR_EXIT;
        }
		pCard->hSndOp[u32PortNum] = hSndOp;
#ifdef MT_SND_MUTECTL_SUPPORT
        if(MT_UNF_SND_OUTPUTPORT_DAC0 == pstAttr->stOutport[u32PortNum].enOutPort)
        {
            /* Get gpio functions */
            if (MT_SUCCESS != mt_drv_module_getfunction(MT_ID_GPIO, (mt_void**)&pCard->pstGpioFunc))
            {
                MT_ERR_AO("Get gpio function err\n");
                goto SND_CreateOp_ERR_EXIT;
            }

            init_timer(&pCard->stMuteDisableTimer);
            pCard->stMuteDisableTimer.function = (void *)SndOpDisableMuteCtrl;
            pCard->stMuteDisableTimer.data = (mt_u32)pCard;
            /* Tscancode: duplicate branches for 'if' and 'else'. */
            //if(MT_TRUE == bResume)
            {
                pCard->stMuteDisableTimer.expires = (jiffies + msecs_to_jiffies(AO_SND_MUTE_RESUME_DISABLE_TIMEMS));
            }
            //else
            //{
            //    pCard->stMuteDisableTimer.expires = (jiffies + msecs_to_jiffies(AO_SND_MUTE_RESUME_DISABLE_TIMEMS));
            //}
            add_timer(&pCard->stMuteDisableTimer);
        }
#endif
    }

    if(pstAoI2sParam != NULL)//for i2s only card resume MT_ALSA_I2S_ONLY_SUPPORT
    {
        if(pstAoI2sParam->bAlsaI2sUse == MT_TRUE)
        {
            memcpy(&pCard->stUserOpenParamI2s, pstAoI2sParam, sizeof(AO_ALSA_I2S_Param_S));
        }
        else
        {
            memset(&pCard->stUserOpenParamI2s,0,sizeof(AO_ALSA_I2S_Param_S));
        }
    }
    else
    {
     memset(&pCard->stUserOpenParamI2s,0,sizeof(AO_ALSA_I2S_Param_S));
    }

    memcpy(&pCard->stUserOpenParam, pstAttr, sizeof(MT_UNF_SND_ATTR_S));
    pCard->enUserSampleRate = pstAttr->enSampleRate;
    pCard->enUserHdmiMode = MT_UNF_SND_HDMI_MODE_LPCM;
    pCard->enUserSpdifMode = MT_UNF_SND_SPDIF_MODE_LPCM;
    pCard->u32HdmiDataFormat = 0;
    pCard->u32SpdifDataFormat = 0;

    return MT_SUCCESS;

SND_CreateOp_ERR_EXIT:
    SND_DestroyOp(pCard, MT_FALSE);
    return MT_FAILURE;
}

mt_s32 SND_SetOpMute(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute)
{
    mt_handle hSndOp;
    mt_u32    u32Mute = 0;
    mt_s32 Ret = MT_SUCCESS;

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        mt_s32 u32PortNum;

        for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
        {
            hSndOp = pCard->hSndOp[u32PortNum];
            if (hSndOp)
            {
                u32Mute = SndOpGetMute(hSndOp);
                u32Mute &= (~(1L << AO_SNDOP_GLOBAL_MUTE_BIT));
                u32Mute |= (mt_u32)bMute << AO_SNDOP_GLOBAL_MUTE_BIT;
                Ret |= SndOpSetMute(hSndOp, u32Mute);
            }
        }
        return Ret;
    }
    else
    {
        hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
        if (hSndOp)
        {
            u32Mute = SndOpGetMute(hSndOp);
            u32Mute &= (~(1L << AO_SNDOP_LOCAL_MUTE_BIT));
            u32Mute |= (mt_u32)bMute << AO_SNDOP_LOCAL_MUTE_BIT;
            return SndOpSetMute(hSndOp, u32Mute);
        }
        else
        {
            return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
        }
    }
}

mt_s32 SND_GetOpMute(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute)
{
    mt_handle hSndOp;
    mt_u32    u32Mute;
    mt_u32    u32MuteBit;

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        enOutPort = pCard->stUserOpenParam.stOutport[0].enOutPort;
        u32MuteBit = AO_SNDOP_GLOBAL_MUTE_BIT;
    }
    else
    {
        u32MuteBit = AO_SNDOP_LOCAL_MUTE_BIT;
    }

    hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        u32Mute = SndOpGetMute(hSndOp);
        *pbMute = (MT_BOOL)((u32Mute >> u32MuteBit) & 1) ;
        return MT_SUCCESS;
    }
    return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
}

mt_s32 SND_SetOpHdmiMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E enMode)
{
    mt_s32 Ret = MT_SUCCESS;

    if(enMode != pCard->enUserHdmiMode)
    {
        printk("\n  SND_SetOpHdmiMode  HDMI mode changed from %d to %d   \n", pCard->enUserHdmiMode, enMode);
        pCard->enUserHdmiMode = enMode;
        pCard->hdmi_acfg_usrchg = 1;
    }
    else
        printk("\n  SND_SetOpHdmiMode  HDMI mode not changed ,nothing has been done!  \n");


    #if 0
    #define PP_BUF_CONFIG_LEN (96 * 1024) // 96K  8K * 8CHAN * 1.5
    mt_u32 pp_len    = PP_BUF_CONFIG_LEN >> 3;
    mt_u32 pcm_len   = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].u32Size - PP_BUF_CONFIG_LEN;
    #if 0
    if(!SNDGetOpHandleByOutPort(pCard, enOutPort))
    {
        MT_ERR_AO("OutPort(%d) not attatch this card\n", (mt_u32)enOutPort);
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
    #endif
    //printk("+++++set hdmi drv in\n");
    switch (enOutPort)
    {
    case MT_UNF_SND_OUTPUTPORT_HDMI0:
        pCard->enUserHdmiMode = enMode;
        if((MT_UNF_SND_HDMI_MODE_RAW==enMode)||(MT_UNF_SND_HDMI_MODE_LPCM==enMode)){
            if(MT_UNF_SND_HDMI_MODE_RAW==enMode){
                reg_sym_linux_04_reg_pcm_chan_spdif_bit(1); //spdif data
                reg_sym_linux_pp_reg_chan_mod_bit(1);
                iHAL_AOE_AIP_reset_aout_buffer();

                HAL_SET_stCB(pCard,AUDIO_BUF_TYPE_PCM,g_audio_reg_base,
                    pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].u32StartVirAddr+PP_BUF_CONFIG_LEN,pcm_len);
            }else{
                reg_sym_linux_04_reg_pcm_chan_spdif_bit(0); //pcm data
                reg_sym_linux_pp_reg_chan_mod_bit(0);
                iHAL_AOE_AIP_reset_aout_buffer();

                HAL_SET_stCB(pCard,AUDIO_BUF_TYPE_PP,g_audio_reg_base,
                    pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].u32StartVirAddr,pp_len);
            }
            {
                HDMI_AUDIO_ATTR_S stHDMIAttr;

                if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAoAttr)
                {
                    (pCard->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
                }
                if(MT_UNF_SND_HDMI_MODE_RAW==enMode)//
                {
                  stHDMIAttr.enSoundIntf  = HDMI_AUDIO_INTERFACE_SPDIF;
                  MT_INFO_AIAO("\n\n set hdmi to SPDIF mode\n\n\n");
                }
                else
                {
                  stHDMIAttr.enSoundIntf  = HDMI_AUDIO_INTERFACE_I2S;
                  MT_INFO_AIAO("\n\n set hdmi to I2S mode\n\n\n");
                }
                stHDMIAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
                /*get the capability of the max pcm channels of the output device*/
                if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiAudioChange)
                {
                    (pCard->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0,&stHDMIAttr);
                    MT_INFO_AIAO("pfnHdmiAudioChange be called\n");
                }
            }
        }
        break;
    default:
       MT_ERR_AO("Hdmi mode don't support OutPort(%d)\n", (mt_u32)enOutPort);
       return MT_FAILURE;
    }
    #endif
    return Ret;
}

mt_s32 SND_GetOpHdmiMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E *penMode)
{
    mt_s32 Ret = MT_SUCCESS;
    #if 0
    if(!SNDGetOpHandleByOutPort(pCard, enOutPort))
    {
        MT_ERR_AO("OutPort(%d) not attatch this card\n", (mt_u32)enOutPort);
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
    #endif
    switch (enOutPort)
    {
    case MT_UNF_SND_OUTPUTPORT_HDMI0:
        *penMode = pCard->enUserHdmiMode;
        break;
    default:
       MT_ERR_AO("Get hdmi mode don't support OutPort(%d)\n", (mt_u32)enOutPort);
       return MT_FAILURE;
    }

    return Ret;
}


mt_s32 SND_SetOpSpdifMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E enMode)
{
    mt_s32 Ret = MT_SUCCESS;
  
  if(enMode != pCard->enUserSpdifMode)
  {
      MT_INFO_AO("\n  SND_SetOpSpdifMode  SPDIF mode changed from %d to %d   \n", pCard->enUserSpdifMode, enMode);
      pCard->enUserSpdifMode = enMode;
      pCard->hdmi_acfg_usrchg = 1;
  }
  else
  {
      MT_INFO_AO("\n  SND_SetOpSpdifMode  SPDIF mode not changed ,nothing has been done!  \n");
  }
  
/*
    mt_s32 Ret = MT_SUCCESS;
    if(!SNDGetOpHandleByOutPort(pCard, enOutPort))
    {
        MT_ERR_AO("OutPort(%d) not attatch this card\n", (mt_u32)enOutPort);
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }

    switch (enOutPort)
    {
    case MT_UNF_SND_OUTPUTPORT_SPDIF0:
        pCard->enUserSpdifMode = enMode;
        break;
    default:
       MT_ERR_AO("Spdif mode don't support OutPort(%d)\n", (mt_u32)enOutPort);
       return MT_FAILURE;
    }
*/
    return Ret;
}

mt_s32 SND_GetOpSpdifMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_MODE_E *penMode)
{
    mt_s32 Ret = MT_SUCCESS;
    #if 0
    if(!SNDGetOpHandleByOutPort(pCard, enOutPort))
    {
        MT_ERR_AO("OutPort(%d) not attatch this card\n", (mt_u32)enOutPort);
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
    #endif
    switch (enOutPort)
    {
    case MT_UNF_SND_OUTPUTPORT_SPDIF0:
        *penMode = pCard->enUserSpdifMode;
        break;
    default:
       MT_ERR_AO("Get spdif mode don't support OutPort(%d)\n", (mt_u32)enOutPort);
       return MT_FAILURE;
    }

    return Ret;
}


mt_s32 SND_SetOpVolume(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S stGain)
{
    mt_handle hSndOp;
    mt_s32 Ret = MT_SUCCESS;
    if(MT_UNF_SND_OUTPUTPORT_ALL==enOutPort)
    {
        mt_s32 u32PortNum;
        for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
        {
            hSndOp = pCard->hSndOp[u32PortNum];
            if (hSndOp)
            {
                Ret |= SndOpSetVolume(hSndOp, stGain);
            }
        }
        return Ret;
    }
    else
    {
        hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
        if (hSndOp)
        {
            return SndOpSetVolume(hSndOp, stGain);
        }
        else
        {
            return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
        }
    }

    return Ret;
}

mt_s32 SND_GetOpVolume(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S *pstGain)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        MT_ERR_AO("Don't support get volume of allport!\n");
        return MT_ERR_AO_INVALID_PARA;
    }

    if (hSndOp)
    {
        pstGain->bLinearMode = ((SND_OP_STATE_S *)hSndOp)->stUserGain.bLinearMode;
        pstGain->s32Gain = ((SND_OP_STATE_S *)hSndOp)->stUserGain.s32Gain;
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_s32 SND_SetOpSpdifCategoryCode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                  MT_UNF_SND_SPDIF_CATEGORYCODE_E enCategoryCode)
{
    mt_handle hSndOp;
    if(MT_UNF_SND_OUTPUTPORT_SPDIF0 != enOutPort)
    {
        MT_ERR_AO("Set spdif Category code don't support OutPort(%d)", (mt_u32)enOutPort);
        return MT_ERR_AO_INVALID_PARA;
    }
    hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        return SndOpSetSPDIFCategoryCode(hSndOp, enCategoryCode);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}
mt_s32 SND_GetOpSpdifCategoryCode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                  MT_UNF_SND_SPDIF_CATEGORYCODE_E *penCategoryCode)
{
    mt_handle hSndOp;
    if(MT_UNF_SND_OUTPUTPORT_SPDIF0 != enOutPort)
    {
        MT_ERR_AO("Get spdif Category code don't support OutPort(%d)", (mt_u32)enOutPort);
        return MT_ERR_AO_INVALID_PARA;
    }
    hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        *penCategoryCode = ((SND_OP_STATE_S *)hSndOp)->enUserSPDIFCategoryCode;
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}
mt_s32 SND_SetOpSpdifSCMSMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_SCMSMODE_E enSCMSMode)
{
    mt_handle hSndOp;

    if(MT_UNF_SND_OUTPUTPORT_SPDIF0 != enOutPort)
    {
        MT_ERR_AO("Set spdif SCMS mode don't support OutPort(%d)", (mt_u32)enOutPort);
        return MT_ERR_AO_INVALID_PARA;
    }

    hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        return SndOpSetSPDIFSCMSMode(hSndOp, enSCMSMode);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_s32 SND_GetOpSpdifSCMSMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_SPDIF_SCMSMODE_E *penSCMSMode)
{
    mt_handle hSndOp;

    if(MT_UNF_SND_OUTPUTPORT_SPDIF0 != enOutPort)
    {
        MT_ERR_AO("Get spdif SCMS mode don't support OutPort(%d)", (mt_u32)enOutPort);
        return MT_ERR_AO_INVALID_PARA;
    }

    hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        *penSCMSMode = ((SND_OP_STATE_S *)hSndOp)->enUserSPDIFSCMSMode;
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}


mt_s32 SND_SetOpSampleRate(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SAMPLE_RATE_E enSampleRate)
{
#if 1
    if(!pCard)
    {
        MT_ERR_AO( "Card is not open!\n");
        return MT_FAILURE;
    }
    if(pCard->enUserSampleRate == enSampleRate)
    {
        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_AO( "SetSampleRate(%d) failed, can't change samplerate at running!\n",enSampleRate);
        return MT_FAILURE;
    }
#else
    mt_handle hSndOp = SNDGetOpHandleByOutPort(enSound, enOutPort);
    mt_s32 Ret = MT_FAILURE;
    if (hSndOp)
    {
        return SndOpSetSampleRate(hSndOp, enSampleRate);
    }
    return Ret;
#endif
}

mt_s32 SND_GetOpSampleRate(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SAMPLE_RATE_E *penSampleRate)
{
    *penSampleRate = pCard->enUserSampleRate;
    return MT_SUCCESS;
}

mt_s32 SND_SetOpTrackMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode)
{
    mt_handle hSndOp;
    mt_s32 Ret = MT_SUCCESS;

    if(MT_UNF_SND_OUTPUTPORT_ALL==enOutPort)
    {
        mt_s32 u32PortNum;
        for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
        {
            hSndOp = pCard->hSndOp[u32PortNum];
            if (hSndOp)
            {
                Ret |= SndOpSetTrackMode(hSndOp, enMode);
            }
        }
        return Ret;
    }
    else
    {
        hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
        if (hSndOp)
        {
            return SndOpSetTrackMode(hSndOp, enMode);
        }
        else
        {
            return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
        }
    }

    return Ret;
}

mt_s32 SND_GetOpTrackMode(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E *penMode)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        MT_ERR_AO("Don't support get trackmode of allport!\n");
        return MT_ERR_AO_INVALID_PARA;
    }

    if (hSndOp)
    {
        *penMode = ((SND_OP_STATE_S *)hSndOp)->enUserTrackMode;
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_s32 SND_SetOpAefBypass(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass)
{
    mt_handle hSndOp;
    mt_s32 Ret = MT_SUCCESS;

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        mt_s32 u32PortNum;
        for (u32PortNum = 0; u32PortNum < MT_UNF_SND_OUTPUTPORT_MAX; u32PortNum++)
        {
            hSndOp = pCard->hSndOp[u32PortNum];
            if (hSndOp)
            {
                Ret |= SndOpSetAefBypass(hSndOp, bBypass);
            }
        }
        return Ret;
    }
    else
    {
        hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
        if (hSndOp)
        {
            return SndOpSetAefBypass(hSndOp, bBypass);
        }
        else
        {
            return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
        }
    }
    }

mt_s32 SND_GetOpAefBypass(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if(MT_UNF_SND_OUTPUTPORT_ALL == enOutPort)
    {
        MT_ERR_AO("Don't support get aef bypass status of allport!\n");
        return MT_ERR_AO_INVALID_PARA;
    }

    if (hSndOp)
    {
        *pbBypass = ((SND_OP_STATE_S *)hSndOp)->bBypass;
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_s32 SND_SetOpAttr(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, SND_OP_ATTR_S *pstSndPortAttr)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if (hSndOp)
    {
        return SndOpSetAttr(hSndOp, pstSndPortAttr);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }

}

mt_s32 SND_GetOpAttr(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, SND_OP_ATTR_S *pstSndPortAttr)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    if (hSndOp)
    {
        return SndOpGetAttr(hSndOp, pstSndPortAttr);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

#if 0
mt_s32 SND_GetOpStatus(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort, AIAO_PORT_STAUTS_S *pstPortStatus)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);
    mt_s32 Ret = MT_FAILURE;

    if (hSndOp)
    {
        return SndOpGetStatus(hSndOp, pstPortStatus);
    }

    return Ret;
}
#endif

mt_s32 SND_StopOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if (hSndOp)
    {
        return SndOpStop(hSndOp, MT_NULL);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_s32 SND_StartOp(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enOutPort)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enOutPort);

    if (hSndOp)
    {
        return SndOpStart(hSndOp, MT_NULL);
    }
    else
    {
        return MT_ERR_AO_OUTPORT_NOT_ATTATCH;
    }
}

mt_void SND_GetXRunCount(SND_CARD_STATE_S *pCard, mt_u32 *pu32Count)
{
    AIAO_PORT_STAUTS_S stStatus;
    mt_handle hSndOp;
    MT_UNF_SND_ATTR_S* pstSndAttr;
    mt_u32 u32EmptyCnt = 0;
    mt_u32 i = 0;
    MT_UNF_SND_OUTPUTPORT_E enPort = MT_UNF_SND_OUTPUTPORT_HDMI0;

    pstSndAttr = &pCard->stUserOpenParam;

    for (i = 0; i < pstSndAttr->u32PortNum; i++)
    {
       enPort = pstSndAttr->stOutport[i].enOutPort;
       hSndOp = SNDGetOpHandleByOutPort(pCard, enPort);
       if (MT_NULL == hSndOp)
       {
           return ;
       }
       SndOpGetStatus(hSndOp, &stStatus);
       u32EmptyCnt += stStatus.stProcStatus.uInfFiFoEmptyCnt;
    }
    *pu32Count = u32EmptyCnt;

}

#ifdef MT_SND_CAST_SUPPORT

static mt_u32 Cast_GetSysTime(mt_void)
{
    mt_u64   SysTime = 0;
    ///TODO:
    //SysTime = sched_clock();

    //do_div(SysTime, 1000000);

    return (mt_u32)SysTime;
}

mt_s32 SND_StopCastOp(SND_CARD_STATE_S *pCard, mt_s32 s32CastID)
{
    mt_handle hSndOp = pCard->hCastOp[s32CastID];
    mt_s32 Ret = MT_FAILURE;

    if (hSndOp)
    {
        return SndOpStop(hSndOp, MT_NULL);
    }
    return Ret;
}
mt_s32 SND_StartCastOp(SND_CARD_STATE_S *pCard, mt_s32 s32CastID)
{
    mt_handle hSndOp = pCard->hCastOp[s32CastID];
    mt_s32 Ret = MT_FAILURE;

    if (hSndOp)
    {
        return SndOpStart(hSndOp, MT_NULL);
    }
    return Ret;
}

mt_s32 SND_CreateCastOp(SND_CARD_STATE_S *pCard,  mt_handle *ps32CastId, MT_UNF_SND_CAST_ATTR_S *pstAttr, mmz_buffer_s *pstMMz)
{
    mt_handle hSndOp;

    if (MT_SUCCESS != SndOpCreateCast(&hSndOp,  ps32CastId, pstAttr, pstMMz))
    {
        MT_ERR_AIAO("SndOpCreateCast Failed\n");
        return MT_FAILURE;
    }

    pCard->hCastOp[*ps32CastId] = hSndOp;

    return MT_SUCCESS;
}

mt_s32 SND_DestoryCastOp(SND_CARD_STATE_S *pCard,  mt_u32 CastId)
{

    if (pCard->hCastOp[CastId])
    {
        SndOpDestroy(pCard->hCastOp[CastId], MT_FALSE);
        pCard->hCastOp[CastId] = MT_NULL;
    }
    else
    {
        MT_ERR_AIAO("SND_DestoryCastOp Null pointer Failed\n");
    }

    return MT_SUCCESS;
}

mt_u32 SND_ReadCastData(SND_CARD_STATE_S *pCard, mt_s32 s32CastId, AO_Cast_Data_Param_S *pstCastData)
{
    SND_OP_STATE_S *state;
    mt_u32 u32ReadBytes = 0;
    mt_u32 u32NeedBytes = 0;

    state = (SND_OP_STATE_S *)pCard->hCastOp[s32CastId];
    if (state)
    {
        if (SND_OUTPUT_TYPE_CAST != state->enOutType)
        {
            return MT_FAILURE;
        }

        u32NeedBytes = pstCastData->u32FrameBytes;
        //MT_ERR_AO( "u32NeedBytes  0x%x  !\n",u32NeedBytes);

        u32ReadBytes = HAL_CAST_ReadData(state->CastId, &pstCastData->u32DataOffset, u32NeedBytes);
#if 0
        if (u32ReadBytes)
        {
            MT_ERR_AO( "u32NeedBytes  0x%x  , u32ReadBytes 0x%x !\n",u32NeedBytes, u32ReadBytes);
        }
#endif

#if 1   //add  pts to cast frame
        pstCastData->stAOFrame.u64PtsMs = Cast_GetSysTime();
#endif

        pstCastData->stAOFrame.u32PcmSamplesPerFrame = u32ReadBytes  / pstCastData->u32SampleBytes;
        //MT_ERR_AO( "pstCastData->u32DataOffset=0x%x ,stAOFrame.u32PcmSamplesPerFrame   0x%x\n",pstCastData->u32DataOffset ,(int)(pstCastData->stAOFrame.u32PcmSamplesPerFrame));
    }

    return u32ReadBytes;
}

mt_u32 SND_ReleaseCastData(SND_CARD_STATE_S *pCard, mt_s32 s32CastId, AO_Cast_Data_Param_S *pstCastData)
{
    SND_OP_STATE_S *state;
    mt_u32 u32RleaseBytes = 0;

    //*state = ( SND_OP_STATE_S *)SNDGetOpHandleByOutPort(pCard, MT_UNF_SND_OUTPUTPORT_CAST);
    state = (SND_OP_STATE_S *)pCard->hCastOp[s32CastId];
    if (state)
    {
        if (SND_OUTPUT_TYPE_CAST != state->enOutType)
        {
            return MT_FAILURE;
        }

        u32RleaseBytes = pstCastData->u32FrameBytes;
        u32RleaseBytes = HAL_CAST_ReleaseData(state->CastId, u32RleaseBytes);
        //MT_ERR_AO( "u32RleaseBytes  0x%x  !\n",u32RleaseBytes);

        return u32RleaseBytes;
    }

    return MT_FAILURE;
}

#endif


#if defined (MT_SND_DRV_SUSPEND_SUPPORT)
mt_s32 SndOpGetSetting(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enPort, SND_OUTPORT_ATTR_S *pstPortAttr)
{
    SND_OP_STATE_S *state;

    state = (SND_OP_STATE_S *)SNDGetOpHandleByOutPort(pCard, enPort);
    if(!state)
    {
        return MT_FAILURE;
    }
    pstPortAttr->enCurnStatus = state->enCurnStatus;
    pstPortAttr->u32UserMute = state->u32UserMute;
    pstPortAttr->enUserTrackMode = state->enUserTrackMode;
    pstPortAttr->bBypass = state->bBypass;
    memcpy(&pstPortAttr->stUserGain, &state->stUserGain, sizeof(MT_UNF_SND_GAIN_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 SND_GetOpSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings)
{
    mt_u32 u32Port;

    pstSndSettings->enUserHdmiMode = pCard->enUserHdmiMode;
    pstSndSettings->enUserSpdifMode = pCard->enUserSpdifMode;
    memcpy(&pstSndSettings->stUserOpenParam, &pCard->stUserOpenParam, sizeof(MT_UNF_SND_ATTR_S));
    if(&pCard->stUserOpenParamI2s != MT_NULL )//for i2s only card resume MT_ALSA_I2S_ONLY_SUPPORT
    {
       memcpy(&pstSndSettings->stUserOpenParamI2s, &pCard->stUserOpenParamI2s, sizeof(AO_ALSA_I2S_Param_S));
    }
    for(u32Port = 0; u32Port < pCard->stUserOpenParam.u32PortNum; u32Port++)
    {
        SndOpGetSetting(pCard, pCard->stUserOpenParam.stOutport[u32Port].enOutPort, &pstSndSettings->stPortAttr[u32Port]);
    }

    return MT_SUCCESS;
}

mt_s32 SNDOpRestoreSetting(SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enPort, SND_OUTPORT_ATTR_S *pstPortAttr)
{
    mt_handle hSndOp = SNDGetOpHandleByOutPort(pCard, enPort);
    if(!hSndOp)
    {
        return MT_FAILURE;
    }
    if(SND_OP_STATUS_START == pstPortAttr->enCurnStatus)
    {
        SndOpStart(hSndOp, MT_NULL);
    }
    else if(SND_OP_STATUS_STOP == pstPortAttr->enCurnStatus)
    {
        SndOpStop(hSndOp, MT_NULL);
    }

    SndOpSetMute(hSndOp, pstPortAttr->u32UserMute);
    SndOpSetTrackMode(hSndOp, pstPortAttr->enUserTrackMode);
    SndOpSetVolume(hSndOp, pstPortAttr->stUserGain);
    SndOpSetAefBypass(hSndOp, pstPortAttr->bBypass);

    return MT_SUCCESS;
}

mt_s32 SND_RestoreOpSetting(SND_CARD_STATE_S *pCard, SND_CARD_SETTINGS_S* pstSndSettings)
{
    mt_u32 u32Port;

    pCard->enUserHdmiMode = pstSndSettings->enUserHdmiMode;
    pCard->enUserSpdifMode = pstSndSettings->enUserSpdifMode;

    if(&pstSndSettings->stUserOpenParam != MT_NULL ) //for i2s only card resume MT_ALSA_I2S_ONLY_SUPPORT
    {
       memcpy(&pCard->stUserOpenParamI2s, &pstSndSettings->stUserOpenParamI2s, sizeof(AO_ALSA_I2S_Param_S));
    }
    memcpy(&pCard->stUserOpenParam, &pstSndSettings->stUserOpenParam, sizeof(MT_UNF_SND_ATTR_S));

    for(u32Port = 0; u32Port < pstSndSettings->stUserOpenParam.u32PortNum; u32Port++)
    {
        SNDOpRestoreSetting(pCard, pstSndSettings->stUserOpenParam.stOutport[u32Port].enOutPort, &pstSndSettings->stPortAttr[u32Port]);
    }

    return MT_SUCCESS;
}
#endif

static inline const mt_char *AOPort2Name(MT_UNF_SND_OUTPUTPORT_E enPort)
{
    switch(enPort)
    {
        case MT_UNF_SND_OUTPUTPORT_DAC0:
            return "DAC0";
        case MT_UNF_SND_OUTPUTPORT_I2S0:
            return "I2S0";
        case MT_UNF_SND_OUTPUTPORT_I2S1:
            return "I2S1";
        case MT_UNF_SND_OUTPUTPORT_SPDIF0:
            return "SPDIF0";
        case MT_UNF_SND_OUTPUTPORT_HDMI0:
            return "HDMI0";
        case MT_UNF_SND_OUTPUTPORT_ARC0:
            return "ARC0";
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC1:
            return "DAC1";
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC2:
            return "DAC2";
        case MT_UNF_SND_OUTPUTPORT_EXT_DAC3:
            return "DAC3";
        default:
            return "UnknownPort";
    }
}




mt_s32 SND_ReadOpProc(struct seq_file* p, SND_CARD_STATE_S *pCard, MT_UNF_SND_OUTPUTPORT_E enPort)
{
    AIAO_PORT_STAUTS_S stStatus;
    mt_handle hSndOp;
    SND_OP_STATE_S *state;
    //SND_OP_ATTR_S stOpAttr;

    hSndOp = SNDGetOpHandleByOutPort(pCard, enPort);

    if (MT_NULL == hSndOp)
    {
        return MT_FAILURE;
    }

    memset(&stStatus, 0, sizeof(AIAO_PORT_STAUTS_S));
    if(MT_SUCCESS != SndOpGetStatus(hSndOp, &stStatus))
	{
		return MT_FAILURE;
    }
    state = (SND_OP_STATE_S *)hSndOp;
#if 0
    if(MT_SUCCESS != SND_GetOpAttr(enSnd, enPort, &stOpAttr))
    {
        return MT_FAILURE;
    }
#endif
    PROC_PRINT(p,
               "%s: Status(%s), Mute(%s), Vol(%d%s), TrackMode(%s), AefBypass(%s)\n",
                AUTIL_Port2Name(enPort),
               (mt_char*)((AIAO_PORT_STATUS_START == stStatus.enStatus) ? "start" : ((AIAO_PORT_STATUS_STOP == stStatus.enStatus) ? "stop" : "stopping")),
               (0 == state->u32UserMute)?"off":"on",
                state->stUserGain.s32Gain,
                (MT_TRUE == state->stUserGain.bLinearMode)?"":"dB",
                AUTIL_TrackMode2Name(state->enUserTrackMode),
                (MT_TRUE == state->bBypass)?"on":"off");

    if(MT_UNF_SND_OUTPUTPORT_SPDIF0 == enPort)
    {
        PROC_PRINT(p,
                   "      CategoryCode(%s), ScmsMode(%s)\n",
                   AUTIL_CategoryCode2Name(state->enUserSPDIFCategoryCode),
                   AUTIL_ScmsMode2Name(state->enUserSPDIFSCMSMode));
    }
    PROC_PRINT(p,
               "      SampleRate(%.6d), Channel(%.2d), BitWidth(%2d), *Engine(%s), *AOP(0x%x), *PortID(0x%x)\n",
                   stStatus.stUserConfig.stIfAttr.enRate,
                   stStatus.stUserConfig.stIfAttr.enChNum,
                   stStatus.stUserConfig.stIfAttr.enBitDepth,
                   AUTIL_Engine2Name(state->enEngineType[state->ActiveId]),
                   (mt_u32)state->enAOP[state->ActiveId],
                   (mt_u32)state->enPortID[state->ActiveId]);

    PROC_PRINT(p,
               "      DmaCnt(%.6u), BufEmptyCnt(%.6u), FiFoEmptyCnt(%.6u)\n\n",
               stStatus.stProcStatus.uDMACnt,
               stStatus.stProcStatus.uBufEmptyCnt,
               stStatus.stProcStatus.uInfFiFoEmptyCnt);

    return MT_SUCCESS;
}


