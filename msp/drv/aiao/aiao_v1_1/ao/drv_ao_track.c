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
#include "mt_drv_file.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "mt_error_mpi.h"
#include "mt_module_debug.h"
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_unf_misc.h"
#endif

#include "mt_cache.h"
#include "mt_drv_hdmi.h"
#include "drv_hdmi_ext.h"
#include "mt_drv_dump.h"

#include "audio_util.h"
#include "drv_ao_op.h"
#include "drv_ao_track.h"
#include "hal_aoe_func.h"
#include "hal_aiao_func.h"
#include "hal_aiao_common.h"
#include "hal_aoe_common.h"
#include "hal_aoe.h"
#include "hal_cast.h"
#include "hal_aiao.h"
//#include "hal_tianlai_adac_v500.h"
#if defined (MT_AUDIO_AI_SUPPORT)
#include "drv_ai_private.h"
#endif
//#include "../../../sync/avsync_fw_vsb2.h"

/* test hdmi pass-through autio without hdmi device , only work at MT_UNF_SND_HDMI_MODE_RAW*/
#define HDMI_AUDIO_PASSTHROUGH_DEBUG


/******************************Track Engine process FUNC*************************************/
extern void reg_sym_linux_04_reg_pcm_chan_spdif_bit(mt_u8 data);
extern void reg_sym_linux_pp_reg_chan_mod_bit(mt_u8 data);
extern mt_u32 reg_sym_linux_04_reg_get_all(void);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
extern int crm_module_clk_set(mt_u32 m_id, mt_u32 type);
#endif

static mt_void *g_pcm_dump_handle;

static mt_void TrackDestroyEngine(mt_handle hSndEngine)
{
    SND_ENGINE_STATE_S *state = (SND_ENGINE_STATE_S *)hSndEngine;

    if (!state)
    {
        return;
    }

    HAL_AOE_ENGINE_Stop(state->enEngine);
    HAL_AOE_ENGINE_Destroy(state->enEngine);
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
}

mt_void TRACK_DestroyEngine(SND_CARD_STATE_S *pCard)
{
    mt_u32 Id;

    for (Id = 0; Id < SND_ENGINE_TYPE_BUTT; Id++)
    {
        if (pCard->hSndEngine[Id])
        {
            TrackDestroyEngine(pCard->hSndEngine[Id]);
            pCard->hSndEngine[Id] = MT_NULL;
        }
    }
}

mt_s32 TrackCreateEngine(mt_handle *phSndEngine, AOE_ENGINE_CHN_ATTR_S *pstAttr, SND_ENGINE_TYPE_E enType)
{
    SND_ENGINE_STATE_S *state = MT_NULL;
    mt_s32 Ret = MT_FAILURE;
    AOE_ENGINE_ID_E enEngine;

    *phSndEngine = MT_NULL;

    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_ENGINE_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AIAO("malloc CreateEngine failed\n");
        goto CreateEngine_ERR_EXIT;
    }

    memset(state, 0, sizeof(SND_ENGINE_STATE_S));

    if (MT_SUCCESS != HAL_AOE_ENGINE_Create(&enEngine, pstAttr))
    {
        MT_ERR_AO("Create engine failed!\n");
        goto CreateEngine_ERR_EXIT;
    }

    state->stUserEngineAttr = *pstAttr;
    state->enEngine = enEngine;
    state->enEngineType = enType;
    *phSndEngine = (mt_handle)state;

    return MT_SUCCESS;

CreateEngine_ERR_EXIT:
    *phSndEngine = (mt_handle)MT_NULL;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return Ret;
}

SND_ENGINE_TYPE_E  TrackGetEngineType(mt_handle hSndEngine)
{
    SND_ENGINE_STATE_S *state = (SND_ENGINE_STATE_S *)hSndEngine;

    return (state->enEngineType);
}

mt_handle  TrackGetEngineHandlebyType(SND_CARD_STATE_S *pCard, SND_ENGINE_TYPE_E enType)
{
    mt_handle hSndEngine;

    hSndEngine = pCard->hSndEngine[enType];
    if (hSndEngine)
    {
        return hSndEngine;
    }

    return MT_NULL;
}


/******************************Track process FUNC*************************************/
MT_BOOL  TrackCheckIsPcmOutput(SND_CARD_STATE_S *pCard)
{
    if(SND_PCM_OUTPUT_CERTAIN == pCard->enPcmOutput)
    {
        return MT_TRUE;
    }
    else if(SND_PCM_OUTPUT_VIR_SPDIFORHDMI == pCard->enPcmOutput)
    {
        if(SND_SPDIF_MODE_PCM == pCard->enSpdifPassthrough || SND_HDMI_MODE_PCM == pCard->enHdmiPassthrough)
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}


mt_u32 TrackGetPcmSize(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    mt_u32 channels=pstAOFrame->u32Channels;
    mt_u32 bytes_per_frame = 0;

    if(pstAOFrame->u32Channels > AO_TRACK_NORMAL_CHANNELNUM)
    {
        channels = AO_TRACK_NORMAL_CHANNELNUM;
    }
bytes_per_frame = pstAOFrame->u32PcmSamplesPerFrame*AUTIL_CalcFrameSize(channels, pstAOFrame->s32BitPerSample);
    return bytes_per_frame;
}

mt_u32 TrackGetMultiPcmSize(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    if(pstAOFrame->u32Channels > AO_TRACK_NORMAL_CHANNELNUM)
    {
        /* allways 8 ch */
        return pstAOFrame->u32PcmSamplesPerFrame*AUTIL_CalcFrameSize(AO_TRACK_MUTILPCM_CHANNELNUM, pstAOFrame->s32BitPerSample);
    }
    else
    {
        return 0;
    }
}


mt_u32 TrackGetLbrSize(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    mt_u32 LbrRawBytes = 0;

    LbrRawBytes = (pstAOFrame->u32BitsBytesPerFrame & 0xffff);
    return LbrRawBytes;
}

mt_u32 TrackGetHbrSize(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    mt_u32 HbrRawBytes = 0;

    if (pstAOFrame->u32BitsBytesPerFrame & 0xffff0000)
    {
        HbrRawBytes = (pstAOFrame->u32BitsBytesPerFrame >> 16);
    }
    else
    {
        HbrRawBytes = (pstAOFrame->u32BitsBytesPerFrame & 0xffff);
    }

    return HbrRawBytes;
}

mt_u32 TrackGetPcmChannels(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    if(AO_TRACK_NORMAL_CHANNELNUM == pstAOFrame->u32Channels|| 1 == pstAOFrame->u32Channels)
    {
        return (mt_u32)pstAOFrame->u32Channels;
    }
    else if(pstAOFrame->u32Channels> AO_TRACK_NORMAL_CHANNELNUM)
    {
        return (mt_u32)(AO_TRACK_NORMAL_CHANNELNUM);
    }
    else
    {
        return 0;
    }
}

mt_u32 TrackGetMultiPcmChannels(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    if(pstAOFrame->u32Channels<=AO_TRACK_NORMAL_CHANNELNUM)
    {
        return 0;
    }

    return pstAOFrame->u32Channels-AO_TRACK_NORMAL_CHANNELNUM;
}

ulong TrackGetPcmBufAddr(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    return (ulong)pstAOFrame->ps32PcmBuffer;
}
/*
  |----Interleaved dmx 2.0 frame----|--Interleaved multi 7.1 frame--|
  |----Interleaved 7.1-----------------------------|
  |----Interleaved 5.1----------------- padding 0/0|

*/
ulong TrackGetMultiPcmAddr(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    ulong u32base = (ulong)pstAOFrame->ps32PcmBuffer;
    if(pstAOFrame->u32Channels <= AO_TRACK_NORMAL_CHANNELNUM)
    {
        return 0;
    }
    else
    {
        /* dmx allways 2 ch */
        return u32base + pstAOFrame->u32PcmSamplesPerFrame*AUTIL_CalcFrameSize(AO_TRACK_NORMAL_CHANNELNUM, pstAOFrame->s32BitPerSample);
    }
    return 0;
}

ulong TrackGetLbrBufAddr(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    if (pstAOFrame->u32BitsBytesPerFrame & 0xffff)
    {
        return (ulong)pstAOFrame->ps32BitsBuffer;
    }

    return 0;
}

ulong TrackGetHbrBufAddr(MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    ulong Addr;

    Addr = (ulong)pstAOFrame->ps32BitsBuffer;
    if (pstAOFrame->u32BitsBytesPerFrame & 0xffff0000)
    {
        Addr += (pstAOFrame->u32BitsBytesPerFrame & 0xffff);
    }

    return Addr;
}

//for both passthrough-only(no pcm output) and simul mode
mt_void TrackBuildPcmAttr(MT_UNF_AO_FRAMEINFO_S *pstAOFrame, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
    if(pstAOFrame->u32PcmSamplesPerFrame)
    {
        pstStreamAttr->u32PcmSampleRate = pstAOFrame->u32SampleRate;
        pstStreamAttr->u32PcmBitDepth = pstAOFrame->s32BitPerSample;
        pstStreamAttr->u32PcmSamplesPerFrame = pstAOFrame->u32PcmSamplesPerFrame;
        pstStreamAttr->u32PcmBytesPerFrame = TrackGetPcmSize(pstAOFrame);
        pstStreamAttr->u32PcmChannels = TrackGetPcmChannels(pstAOFrame);
        pstStreamAttr->pPcmDataBuf = (mt_void*)TrackGetPcmBufAddr(pstAOFrame);
    }
    else
    {
        mt_u32 u32BitWidth;
        if(16 == pstAOFrame->s32BitPerSample)
        {
            u32BitWidth = sizeof(mt_u16);
        }
        else
        {
            u32BitWidth = sizeof(mt_u32);
        }
        if(pstStreamAttr->pLbrDataBuf)
        {
            pstStreamAttr->u32PcmSampleRate = pstAOFrame->u32SampleRate;
            pstStreamAttr->u32PcmBitDepth = pstAOFrame->s32BitPerSample;
            pstStreamAttr->u32PcmBytesPerFrame = pstStreamAttr->u32LbrBytesPerFrame;
            pstStreamAttr->u32PcmChannels = AO_TRACK_NORMAL_CHANNELNUM;
            pstStreamAttr->u32PcmSamplesPerFrame = pstStreamAttr->u32PcmBytesPerFrame/pstStreamAttr->u32PcmChannels/u32BitWidth;
            pstStreamAttr->pPcmDataBuf = (mt_void*)MT_NULL;
        }
        else if(pstStreamAttr->pHbrDataBuf)
        {
            mt_u32 u32HbrSamplesPerFrame = pstStreamAttr->u32HbrBytesPerFrame/pstStreamAttr->u32HbrChannels/u32BitWidth;
            pstStreamAttr->u32PcmSamplesPerFrame = u32HbrSamplesPerFrame >> 2;
            pstStreamAttr->u32PcmSampleRate = pstAOFrame->u32SampleRate;
            pstStreamAttr->u32PcmBitDepth = pstAOFrame->s32BitPerSample;
            pstStreamAttr->u32PcmChannels = AO_TRACK_NORMAL_CHANNELNUM;
            pstStreamAttr->u32PcmBytesPerFrame = pstStreamAttr->u32PcmSamplesPerFrame*pstStreamAttr->u32PcmChannels*u32BitWidth;
            pstStreamAttr->pPcmDataBuf = (mt_void*)MT_NULL;
        }
        else
        {
            pstStreamAttr->u32PcmSampleRate = MT_UNF_SAMPLE_RATE_48K;
            pstStreamAttr->u32PcmBitDepth = AO_TRACK_BITDEPTH_LOW;
            pstStreamAttr->u32PcmChannels = AO_TRACK_NORMAL_CHANNELNUM;
            pstStreamAttr->u32PcmBytesPerFrame = 0;
            pstStreamAttr->u32PcmSamplesPerFrame = 0;
            pstStreamAttr->pPcmDataBuf = (mt_void*)MT_NULL;
        }
    }

    //pstStreamAttr->u32PcmBytesPerFrame *= 2;
}
#if 0 //unuse code
static mt_void TRACKDbgCountTrySendData(SND_TRACK_STATE_S *pTrack)
{
    pTrack->u32SendTryCnt++;
}
#endif
static mt_void TRACKDbgCountSendData(SND_TRACK_STATE_S *pTrack)
{
    pTrack->u32SendCnt++;
}

mt_void TRACKBuildStreamAttr(SND_CARD_STATE_S *pCard, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{

    pstStreamAttr->u32PcmSampleRate = pstAOFrame->u32SampleRate;
    pstStreamAttr->u32PcmBytesPerFrame = pstAOFrame->u32BitsBytesPerFrame;
    pstStreamAttr->pPcmDataBuf = pstAOFrame->ps32PcmBuffer;

    #if 0
    mt_u32 u32IEC61937DataType;
    mt_s32 s32Ret = MT_FAILURE;
    memset(pstStreamAttr, 0, sizeof(SND_TRACK_STREAM_ATTR_S));

    // lbr
    if(pstAOFrame->u32IEC61937DataType & 0xff)
    {
		u32IEC61937DataType = pstAOFrame->u32IEC61937DataType & 0xff;/*0~8bit : datatype*/
	}
	else
	{
		u32IEC61937DataType = AUTIL_IEC61937DataType((mt_u16*)TrackGetLbrBufAddr(pstAOFrame), TrackGetLbrSize(pstAOFrame));
	}
    pstStreamAttr->u32LbrFormat = IEC61937_DATATYPE_NULL;
    if (u32IEC61937DataType && !(AUTIL_isIEC61937Hbr(u32IEC61937DataType, pstAOFrame->u32SampleRate)))
    {
        pstStreamAttr->u32LbrSampleRate = pstAOFrame->u32SampleRate;
        pstStreamAttr->u32LbrBitDepth = AO_TRACK_BITDEPTH_LOW;
        pstStreamAttr->u32LbrChannels = AO_TRACK_NORMAL_CHANNELNUM;
        pstStreamAttr->u32LbrFormat = u32IEC61937DataType;
        pstStreamAttr->u32LbrBytesPerFrame = TrackGetLbrSize(pstAOFrame);
        pstStreamAttr->pLbrDataBuf = (mt_void*)TrackGetLbrBufAddr(pstAOFrame);
    }

    // hbr
	if(pstAOFrame->u32IEC61937DataType & 0xff)
	{
		u32IEC61937DataType = pstAOFrame->u32IEC61937DataType & 0xff;/*0~8bit : datatype*/
	}
	else
	{
		u32IEC61937DataType = AUTIL_IEC61937DataType((mt_u16*)TrackGetHbrBufAddr(pstAOFrame), TrackGetHbrSize(pstAOFrame));
	}

    pstStreamAttr->u32HbrFormat = IEC61937_DATATYPE_NULL;
    if ((AUTIL_isIEC61937Hbr(u32IEC61937DataType, pstAOFrame->u32SampleRate)))
    {
        pstStreamAttr->u32HbrBitDepth = AO_TRACK_BITDEPTH_LOW;
        pstStreamAttr->u32HbrChannels = ((IEC61937_DATATYPE_DOLBY_DIGITAL_PLUS==u32IEC61937DataType)?2:8);
        pstStreamAttr->u32HbrFormat = u32IEC61937DataType;
		if(pstAOFrame->u32SampleRate <= MT_UNF_SAMPLE_RATE_48K)
		{
        pstStreamAttr->u32HbrSampleRate = pstAOFrame->u32SampleRate * 4;  /* hbr 4*samplerate */
		}
		else if(pstAOFrame->u32SampleRate == MT_UNF_SAMPLE_RATE_88K || pstAOFrame->u32SampleRate == MT_UNF_SAMPLE_RATE_96K)
		{
			pstStreamAttr->u32HbrSampleRate = pstAOFrame->u32SampleRate * 2;
		}
		else
		{
			pstStreamAttr->u32HbrSampleRate = pstAOFrame->u32SampleRate;  /* hbr samplerate */
		}
        pstStreamAttr->u32HbrBytesPerFrame = TrackGetHbrSize(pstAOFrame);
        pstStreamAttr->pHbrDataBuf = (mt_void*)TrackGetHbrBufAddr(pstAOFrame);
    }
    else if(TrackGetMultiPcmChannels(pstAOFrame))
    {
        MT_DRV_HDMI_AUDIO_CAPABILITY_S stSinkCap;
        /*get the capability of the max pcm channels of the output device*/
        if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAudioCapability)
        {
            s32Ret = (pCard->pstHdmiFunc->pfnHdmiGetAudioCapability)(MT_UNF_HDMI_ID_0, &stSinkCap);
        }
#if defined(HDMI_AUDIO_PASSTHROUGH_DEBUG)
        if(MT_TRUE == pCard->bHdmiDebug)
        {
            if(MT_UNF_SND_HDMI_MODE_RAW == pCard->enUserHdmiMode)
            {
                s32Ret = MT_SUCCESS; /* cheat SND work at hdmi plun-in status without hdmi device */
                stSinkCap.u32MaxPcmChannels = AO_TRACK_MUTILPCM_CHANNELNUM;
            }
        }
#endif
        if(MT_SUCCESS == s32Ret)
        {
            if (stSinkCap.u32MaxPcmChannels > AO_TRACK_NORMAL_CHANNELNUM)
            {
                pstStreamAttr->u32HbrBitDepth = pstAOFrame->s32BitPerSample;
                pstStreamAttr->u32HbrSampleRate = pstAOFrame->u32SampleRate;
                pstStreamAttr->u32HbrFormat = IEC61937_DATATYPE_71_LPCM;
                pstStreamAttr->u32HbrChannels = AO_TRACK_MUTILPCM_CHANNELNUM;
                pstStreamAttr->u32OrgMultiPcmChannels = TrackGetMultiPcmChannels(pstAOFrame);
                pstStreamAttr->u32HbrBytesPerFrame = TrackGetMultiPcmSize(pstAOFrame);
                pstStreamAttr->pHbrDataBuf = (mt_void*)TrackGetMultiPcmAddr(pstAOFrame);
            }
        }
    }

    #endif
    // pcm
    //TrackBuildPcmAttr(pstAOFrame,pstStreamAttr);
}

static SND_HDMI_MODE_E TRACKHdmiEdidChange(MT_DRV_HDMI_AUDIO_CAPABILITY_S *pstSinkCap, mt_u32 u32Format)
{
    switch(u32Format)
    {
        case IEC61937_DATATYPE_NULL:
            return SND_HDMI_MODE_PCM;

        case IEC61937_DATATYPE_DOLBY_DIGITAL:
            if(MT_TRUE == pstSinkCap->bAudioFmtSupported[AO_HDMI_CAPABILITY_AC3])
            {
                return SND_HDMI_MODE_LBR;
            }
            else
            {
                return SND_HDMI_MODE_PCM;
            }

        case IEC61937_DATATYPE_DTS_TYPE_I:
        case IEC61937_DATATYPE_DTS_TYPE_II:
        case IEC61937_DATATYPE_DTS_TYPE_III:
        case IEC61937_DATATYPE_DTSCD:
            if(MT_TRUE == pstSinkCap->bAudioFmtSupported[AO_HDMI_CAPABILITY_DTS])
            {
                return SND_HDMI_MODE_LBR;
            }
            else
            {
                return SND_HDMI_MODE_PCM;
            }

        case IEC61937_DATATYPE_DOLBY_DIGITAL_PLUS:
            if(MT_TRUE == pstSinkCap->bAudioFmtSupported[AO_HDMI_CAPABILITY_DDP])
            {
                return SND_HDMI_MODE_HBR;
            }
            else
            {
                return SND_HDMI_MODE_LBR;
            }

        case IEC61937_DATATYPE_DTS_TYPE_IV:
            if(MT_TRUE == pstSinkCap->bAudioFmtSupported[AO_HDMI_CAPABILITY_DTSHD])
            {
                return SND_HDMI_MODE_HBR;
            }
            else
            {
                return SND_HDMI_MODE_LBR;
            }

        case IEC61937_DATATYPE_DOLBY_TRUE_HD:
            if(MT_TRUE == pstSinkCap->bAudioFmtSupported[AO_HDMI_CAPABILITY_MAT])
            {
                return SND_HDMI_MODE_HBR;
            }
            else
            {
                return SND_HDMI_MODE_LBR;
            }

        case IEC61937_DATATYPE_71_LPCM:
            return SND_HDMI_MODE_HBR;

        default:
            MT_WARN_AO("Failed to judge edid cabability of format %d\n", u32Format);
            return SND_HDMI_MODE_PCM;
    }
}

TRACK_STREAMMODE_CHANGE_E GetHdmiChangeMode(SND_CARD_STATE_S *pCard, SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    SND_OP_ATTR_S stSndPortAttr;
    mt_handle hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_HDMI);
    TRACK_STREAMMODE_CHANGE_E enChange = TRACK_STREAMMODE_CHANGE_NONE;
    SND_HDMI_MODE_E mode  = SND_HDMI_MODE_PCM;
    MT_BOOL bdisPassThrough = MT_FALSE;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Status = MT_FALSE;

    s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stSndPortAttr);

    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("SND_GetOpAttr Fail\n");
        return s32Ret;
    }

    /* ui */
    if (MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode)
    {
        bdisPassThrough = MT_TRUE;
    }

    if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetPlayStus)
    {
        (pCard->pstHdmiFunc->pfnHdmiGetPlayStus)(MT_UNF_HDMI_ID_0, &u32Status);
#if defined(HDMI_AUDIO_PASSTHROUGH_DEBUG)
        if(MT_TRUE == pCard->bHdmiDebug)
        {
             u32Status = MT_TRUE; /* cheat SND work at hdmi plun-in status without hdmi device */
        }
#endif
	}

	/*HDMI disconnect just PCM no LBR/HBR output */
	if(u32Status != MT_TRUE)
	{
		bdisPassThrough = MT_TRUE;
	}
    /* no raw data at stream */
    if (!pstAttr->u32LbrFormat && !pstAttr->u32HbrFormat)
    {
        bdisPassThrough = MT_TRUE;
    }

    if (bdisPassThrough)
    {
        /* disable hdmi pass-through, switch to pcm */
        mode = SND_HDMI_MODE_PCM;
    }
    else
    {
        MT_DRV_HDMI_AUDIO_CAPABILITY_S stSinkCap;
        mode = SND_HDMI_MODE_LBR;
        if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAudioCapability)
        {
            s32Ret = (pCard->pstHdmiFunc->pfnHdmiGetAudioCapability)(MT_UNF_HDMI_ID_0, &stSinkCap);
        }
		else
		{
			MT_ERR_AO("pfnHdmiGetAudioCapability Fail\n");
			s32Ret = MT_FAILURE;
		}

        if (pstAttr->u32HbrFormat)
        {
            mode = SND_HDMI_MODE_HBR;
            if (MT_UNF_SND_HDMI_MODE_HBR2LBR == pCard->enUserHdmiMode)
            {
                /* hbr2lbr */
                if(pstAttr->u32LbrFormat)
                {
                    mode = SND_HDMI_MODE_LBR;
                }
                else
                {
                    mode = SND_HDMI_MODE_PCM;
                }
            }
            if((MT_UNF_SND_HDMI_MODE_AUTO == pCard->enUserHdmiMode) && (MT_SUCCESS == s32Ret))
            {
                mode = TRACKHdmiEdidChange(&stSinkCap, pstAttr->u32HbrFormat);
            }
        }
        if(SND_HDMI_MODE_LBR == mode)
        {
            if((MT_UNF_SND_HDMI_MODE_AUTO == pCard->enUserHdmiMode) && (MT_SUCCESS == s32Ret))
            {
                mode = TRACKHdmiEdidChange(&stSinkCap, pstAttr->u32LbrFormat);
            }
        }

    }

    if (SND_HDMI_MODE_PCM == mode)
    {
        if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_LBR2PCM;
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_HBR2PCM;
        }
    }
    else if (SND_HDMI_MODE_LBR == mode)
    {
        if (SND_HDMI_MODE_PCM == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_PCM2LBR;
        }
        else if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            if (pstAttr->u32LbrSampleRate != stSndPortAttr.u32SampleRate)
            {
                enChange = TRACK_STREAMMODE_CHANGE_LBR2LBR;
            }
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_HBR2LBR;
        }
    }
    else if (SND_HDMI_MODE_HBR == mode)
    {
        if (SND_HDMI_MODE_PCM == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_PCM2HBR;
        }
        else if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_LBR2HBR;
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            if (pstAttr->u32HbrSampleRate != stSndPortAttr.u32SampleRate)
            {
                enChange = TRACK_STREAMMODE_CHANGE_HBR2HBR;
            }
        }
    }

    return enChange;
}

TRACK_STREAMMODE_CHANGE_E GetSpdifChangeMode(SND_CARD_STATE_S *pCard, SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    SND_OP_ATTR_S stSndPortAttr;
    mt_handle hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_SPDIF);
    MT_BOOL bdisPassThrough = MT_FALSE;
    TRACK_STREAMMODE_CHANGE_E enChange = TRACK_STREAMMODE_CHANGE_NONE;

    s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stSndPortAttr);
    if(MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("SND_GetOpAttr Fail\n");
        return s32Ret;
    }

    /* ui or stream */
    if ((MT_UNF_SND_SPDIF_MODE_LPCM == pCard->enUserSpdifMode) || (IEC61937_DATATYPE_NULL == pstAttr->u32LbrFormat))
    {
        bdisPassThrough = MT_TRUE;
    }

    if (bdisPassThrough)
    {
        /* disable spdif pass-through */
        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            enChange = TRACK_STREAMMODE_CHANGE_LBR2PCM;
        }
    }
    else
    {
        /* enable spdif pass-through */
        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            if (pstAttr->u32LbrSampleRate != stSndPortAttr.u32SampleRate)
            {
                enChange = TRACK_STREAMMODE_CHANGE_LBR2LBR;  /* shtream change samplerate */
            }
        }
        else
        {
            enChange = TRACK_STREAMMODE_CHANGE_PCM2LBR;     /* atcive pass-through */
        }
    }

    return enChange;
}

mt_void DetectTrueHDModeChange(SND_CARD_STATE_S *pCard, SND_TRACK_STREAM_ATTR_S *pstAttr)
{
	if(IEC61937_DATATYPE_DOLBY_TRUE_HD == pstAttr->u32HbrFormat)
	{
		if(SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
		{
			mt_u32 u32PcmBitWidth,u32SampleRateWidth,u32HbrBitWidth;
			mt_u32 u32HbrSamplesPerFrame;
	        if(16 == pstAttr->u32PcmBitDepth)
	        {
	            u32PcmBitWidth = sizeof(mt_u16);
	        }
	        else
	        {
	            u32PcmBitWidth = sizeof(mt_u32);
	        }

	        if(16 == pstAttr->u32HbrBitDepth)
	        {
	            u32HbrBitWidth = sizeof(mt_u16);
	        }
	        else
	        {
	            u32HbrBitWidth = sizeof(mt_u32);
	        }

			if(pstAttr->u32PcmSampleRate <= MT_UNF_SAMPLE_RATE_48K)
			{
				u32SampleRateWidth = 2;
			}
			else if((MT_UNF_SAMPLE_RATE_88K == pstAttr->u32PcmSampleRate) || (MT_UNF_SAMPLE_RATE_96K == pstAttr->u32PcmSampleRate))
			{
				u32SampleRateWidth = 1;
			}
			else
			{
				u32SampleRateWidth = 0;
			}

			u32HbrSamplesPerFrame = pstAttr->u32HbrBytesPerFrame/pstAttr->u32HbrChannels/u32HbrBitWidth;
            pstAttr->u32PcmSamplesPerFrame = u32HbrSamplesPerFrame >> u32SampleRateWidth;
            pstAttr->u32PcmChannels = AO_TRACK_NORMAL_CHANNELNUM;
            pstAttr->u32PcmBytesPerFrame = pstAttr->u32PcmSamplesPerFrame*pstAttr->u32PcmChannels * u32PcmBitWidth;
            pstAttr->pPcmDataBuf = (mt_void*)MT_NULL;
		}
	}
}


mt_void DetectStreamModeChange(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S *pstAttr,
                               STREAMMODE_CHANGE_ATTR_S *pstChange)
{
    SND_TRACK_STREAM_ATTR_S *pstAttr_old = &pTrack->stStreamAttr;

    pstChange->enPcmChange   = TRACK_STREAMMODE_CHANGE_NONE;
    pstChange->enSpdifChange = TRACK_STREAMMODE_CHANGE_NONE;
    pstChange->enHdmiChnage  = TRACK_STREAMMODE_CHANGE_NONE;

    // pcm stream attr
    if (pstAttr_old->u32PcmBitDepth != pstAttr->u32PcmBitDepth)
    {
        pstChange->enPcmChange = TRACK_STREAMMODE_CHANGE_PCM2PCM;
    }

    if (pstAttr_old->u32PcmChannels != pstAttr->u32PcmChannels)
    {
        pstChange->enPcmChange = TRACK_STREAMMODE_CHANGE_PCM2PCM;
    }

    if (pstAttr_old->u32PcmSampleRate != pstAttr->u32PcmSampleRate)
    {
        pstChange->enPcmChange = TRACK_STREAMMODE_CHANGE_PCM2PCM;
    }

    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if (SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
        {
            pstChange->enSpdifChange = GetSpdifChangeMode(pCard, pstAttr);
        }

        if (SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
        {
            pstChange->enHdmiChnage  = GetHdmiChangeMode(pCard, pstAttr);
        }
    }
}

mt_void SndProcPcmRoute(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, TRACK_STREAMMODE_CHANGE_E enMode,
                        SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    AOE_AIP_CHN_ATTR_S stAipAttr;

    if (TRACK_STREAMMODE_CHANGE_NONE == enMode)
    {
        return;
    }

    HAL_AOE_AIP_GetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);
    stAipAttr.stBufInAttr.u32BufBitPerSample = pstAttr->u32PcmBitDepth;
    stAipAttr.stBufInAttr.u32BufSampleRate = pstAttr->u32PcmSampleRate;
    stAipAttr.stBufInAttr.u32BufChannels   = pstAttr->u32PcmChannels;
    stAipAttr.stBufInAttr.u32BufDataFormat = 0;
    HAL_AOE_AIP_SetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);

    memcpy(&pTrack->stStreamAttr, pstAttr, sizeof(SND_TRACK_STREAM_ATTR_S));
    return;
}


static mt_void TranslateOpAttr(SND_OP_ATTR_S *pstOpAttr,  TRACK_STREAMMODE_CHANGE_E enMode, SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    mt_u32 u32PeriondMs;
    mt_u32 u32FrameSize;
    mt_u32 BitDepth,Channels,Format,Rate;


    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode)
        || (TRACK_STREAMMODE_CHANGE_HBR2LBR == enMode))
    {
        BitDepth = pstAttr->u32LbrBitDepth;
        Channels = pstAttr->u32LbrChannels;
        Format = pstAttr->u32LbrFormat;
        Rate = pstAttr->u32LbrSampleRate;
    }
    else if ((TRACK_STREAMMODE_CHANGE_PCM2HBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2HBR == enMode)
             || (TRACK_STREAMMODE_CHANGE_HBR2HBR == enMode))
    {
        BitDepth = pstAttr->u32HbrBitDepth;
        Channels = pstAttr->u32HbrChannels;
        Format = pstAttr->u32HbrFormat;
        Rate = pstAttr->u32HbrSampleRate;
    }
    else
    {
        BitDepth = AO_TRACK_BITDEPTH_LOW;
        Channels   = AO_TRACK_NORMAL_CHANNELNUM;
        Rate = MT_UNF_SAMPLE_RATE_48K;
        Format = 0;
    }
#if 1
    /* recaculate PeriodBufSize */
    u32FrameSize = AUTIL_CalcFrameSize(pstOpAttr->u32Channels, pstOpAttr->u32BitPerSample);
    u32PeriondMs = AUTIL_ByteSize2LatencyMs(pstOpAttr->u32PeriodBufSize, u32FrameSize, pstOpAttr->u32SampleRate);
    u32FrameSize = AUTIL_CalcFrameSize(Channels, BitDepth);
    pstOpAttr->u32PeriodBufSize = AUTIL_LatencyMs2ByteSize(u32PeriondMs, u32FrameSize, Rate);
#endif
    pstOpAttr->u32BitPerSample = BitDepth;
    pstOpAttr->u32SampleRate = Rate;
    pstOpAttr->u32Channels   = Channels;
    pstOpAttr->u32DataFormat = Format;
}


//proprocess hdmi output
static mt_void  HDMISetAudioMute(SND_CARD_STATE_S *pCard)
{
    if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiSetAudioMute)
    {
        (pCard->pstHdmiFunc->pfnHdmiSetAudioMute)(MT_UNF_HDMI_ID_0);
    }
	//comment for future open
    //else
        //MT_WARN_AO(" pstHdmiFunc->pfnHdmiPreFormat Not Found !\n");
}

static mt_void  HDMISetAudioUnMute(SND_CARD_STATE_S *pCard)
{
    if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiSetAudioMute)
    {
        (pCard->pstHdmiFunc->pfnHdmiSetAudioUnMute)(MT_UNF_HDMI_ID_0);
    }
    //comment for future open
	//else
        //MT_WARN_AO(" pstHdmiFunc->pfnHdmiPreFormat Not Found !\n");
}


//verify should simple , can change hdmi attr according to op attr
static mt_void  HDMIAudioChange(SND_CARD_STATE_S *pCard, TRACK_STREAMMODE_CHANGE_E enMode,
                          SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    HDMI_AUDIOINTERFACE_E enHdmiSoundIntf;
    HDMI_AUDIO_ATTR_S stHDMIAtr;
    mt_u32 Channels,Rate;
    MT_UNF_EDID_AUDIO_FORMAT_CODE_E enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_RESERVED;

    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode)
        || (TRACK_STREAMMODE_CHANGE_HBR2LBR == enMode))
    {
        Channels = pstAttr->u32LbrChannels;
        Rate = pstAttr->u32LbrSampleRate;
        enHdmiSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
        /* set hdmi_audio_interface_spdif audio codec type  */
        if((IEC61937_DATATYPE_DOLBY_DIGITAL == pstAttr->u32LbrFormat))
        {
            enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3;
        }
        else if((IEC61937_DATATYPE_DTS_TYPE_I == pstAttr->u32LbrFormat) ||
            (IEC61937_DATATYPE_DTS_TYPE_II== pstAttr->u32LbrFormat)     ||
            (IEC61937_DATATYPE_DTS_TYPE_III== pstAttr->u32LbrFormat))
        {
            enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS;
        }
    }
    else if ((TRACK_STREAMMODE_CHANGE_PCM2HBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2HBR == enMode)
             || (TRACK_STREAMMODE_CHANGE_HBR2HBR == enMode))
    {
        Rate = pstAttr->u32HbrSampleRate;
        Channels = pstAttr->u32HbrChannels;
        enHdmiSoundIntf = HDMI_AUDIO_INTERFACE_HBR;
        if(IEC61937_DATATYPE_71_LPCM==pstAttr->u32HbrFormat)
        {
            Channels = pstAttr->u32OrgMultiPcmChannels;
            enHdmiSoundIntf = HDMI_AUDIO_INTERFACE_I2S;   //verify
            enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
        }
        else if (IEC61937_DATATYPE_DOLBY_DIGITAL_PLUS == pstAttr->u32HbrFormat)
        {
            enHdmiSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
            enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP;  //set hdmi_audio_interface_spdif audio codec type
        }
        else if(IEC61937_DATATYPE_DTS_TYPE_IV == pstAttr->u32HbrFormat)
        {
            enAudioFormat = MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD;
        }
    }
    else if ((TRACK_STREAMMODE_CHANGE_LBR2PCM == enMode) || (TRACK_STREAMMODE_CHANGE_HBR2PCM == enMode))
    {
        mt_handle hSndPcmOnlyOp;
        SND_OP_ATTR_S stPcmOnlyOpAttr;
        hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_DAC);
        if (!hSndPcmOnlyOp)
        {
            hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
        }

        if (!hSndPcmOnlyOp)
        {
            Channels   = AO_TRACK_NORMAL_CHANNELNUM;
            Rate = MT_UNF_SAMPLE_RATE_48K;
        }
        else
        {
            s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndPcmOnlyOp), &stPcmOnlyOpAttr);
            if(MT_SUCCESS != s32Ret)
            {
                MT_ERR_AO("SND_GetOpAttr Fail\n");
                return ;
            }
            Channels = stPcmOnlyOpAttr.u32Channels;
            Rate = stPcmOnlyOpAttr.u32SampleRate;
        }
        enHdmiSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
    }
    else
    {
        return;
    }

    /*if the channels of the frame have changed , set the attribute of HDMI*/
    if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAoAttr)
    {
        (pCard->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAtr);
    }

    stHDMIAtr.enSoundIntf  = enHdmiSoundIntf;
    stHDMIAtr.enSampleRate = (MT_UNF_SAMPLE_RATE_E)Rate;
    stHDMIAtr.u32Channels  = Channels;
    stHDMIAtr.enAudioCode  = enAudioFormat;
    MT_WARN_AO("HDMI Audio format ->  %d\n", enAudioFormat);
    /*get the capability of the max pcm channels of the output device*/
    if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiAudioChange)
    {
        (pCard->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0,&stHDMIAtr);
    }
}

mt_void SndProcHdmifRoute(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, TRACK_STREAMMODE_CHANGE_E enMode,
                          SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mt_handle hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_HDMI);
    AOE_AOP_ID_E Aop;
    SND_OP_ATTR_S stOpAttr;
    AOE_ENGINE_CHN_ATTR_S stEngineAttr;
    SND_ENGINE_TYPE_E enEngineNew;
    SND_ENGINE_TYPE_E enEngineOld;
    mt_handle hEngineNew;
    mt_handle hEngineOld;
	SND_ENGINE_STATE_S *state;
    if (TRACK_STREAMMODE_CHANGE_NONE == enMode)
    {
        return;
    }
    //before resetting audio attr, proprocess hdmi audio output


    Aop = SND_OpGetAopId(hSndOp);
    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode)
        || (TRACK_STREAMMODE_CHANGE_HBR2LBR == enMode))
    {
        //set op
        if (TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode)
        {
            enEngineOld = SND_ENGINE_TYPE_PCM;
        }
        else
        {
            enEngineOld = SND_ENGINE_TYPE_HDMI_RAW;
        }

        enEngineNew = SND_ENGINE_TYPE_HDMI_RAW;
        hEngineOld = TrackGetEngineHandlebyType(pCard, enEngineOld);
        hEngineNew = TrackGetEngineHandlebyType(pCard, enEngineNew);
        s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        if(MT_SUCCESS != s32Ret)
        {
            MT_ERR_AO("SND_GetOpAttr Fail\n");
            return ;
        }

        SND_StopOp(pCard, SND_GetOpOutputport(hSndOp));
		state = (SND_ENGINE_STATE_S *)hEngineOld;
		HAL_AOE_ENGINE_DetachAop(state->enEngine, Aop);
        TranslateOpAttr(&stOpAttr,enMode,pstAttr);
        SND_SetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        Aop = SND_OpGetAopId(hSndOp);  //verify
        state = (SND_ENGINE_STATE_S *)hEngineNew;//verify
        HAL_AOE_ENGINE_Stop(state->enEngine);
		HAL_AOE_ENGINE_AttachAop(state->enEngine, Aop);
        SND_StartOp(pCard, SND_GetOpOutputport(hSndOp));

        //set aip
        HAL_AOE_AIP_GetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
        stAipAttr.stBufInAttr.u32BufBitPerSample = pstAttr->u32LbrBitDepth;
        stAipAttr.stBufInAttr.u32BufSampleRate = pstAttr->u32LbrSampleRate;
        stAipAttr.stBufInAttr.u32BufChannels   = pstAttr->u32LbrChannels;
        stAipAttr.stBufInAttr.u32BufDataFormat = pstAttr->u32LbrFormat;

        stAipAttr.stFifoOutAttr.u32FifoBitPerSample = pstAttr->u32LbrBitDepth;
        stAipAttr.stFifoOutAttr.u32FifoSampleRate = pstAttr->u32LbrSampleRate;
        stAipAttr.stFifoOutAttr.u32FifoChannels   = pstAttr->u32LbrChannels;
        stAipAttr.stFifoOutAttr.u32FifoDataFormat = pstAttr->u32LbrFormat;
        HAL_AOE_AIP_SetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
		HAL_AOE_ENGINE_AttachAip(state->enEngine, pTrack->enAIP[enEngineNew]);

        pCard->u32HdmiDataFormat = stAipAttr.stBufInAttr.u32BufDataFormat;
        //set engine

        stEngineAttr.u32BitPerSample = pstAttr->u32LbrBitDepth;
        stEngineAttr.u32Channels   = pstAttr->u32LbrChannels;
        stEngineAttr.u32SampleRate = pstAttr->u32LbrSampleRate;
        stEngineAttr.u32DataFormat = pstAttr->u32LbrFormat;
		HAL_AOE_ENGINE_SetAttr(state->enEngine, &stEngineAttr);
		HAL_AOE_ENGINE_Start(state->enEngine);
    }
    else if ((TRACK_STREAMMODE_CHANGE_PCM2HBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2HBR == enMode)
             || (TRACK_STREAMMODE_CHANGE_HBR2HBR == enMode))
    {
        //set op
        if (TRACK_STREAMMODE_CHANGE_PCM2HBR == enMode)
        {
            enEngineOld = SND_ENGINE_TYPE_PCM;
        }
        else
        {
            enEngineOld = SND_ENGINE_TYPE_HDMI_RAW;
        }

        enEngineNew = SND_ENGINE_TYPE_HDMI_RAW;
        hEngineOld = TrackGetEngineHandlebyType(pCard, enEngineOld);
        hEngineNew = TrackGetEngineHandlebyType(pCard, enEngineNew);
        s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        if(MT_SUCCESS != s32Ret)
        {
            MT_ERR_AO("SND_GetOpAttr Fail\n");
            return ;
        }

        SND_StopOp(pCard, SND_GetOpOutputport(hSndOp));
		state = (SND_ENGINE_STATE_S *)hEngineOld;//verify
		HAL_AOE_ENGINE_DetachAop(state->enEngine, Aop);
        TranslateOpAttr(&stOpAttr,enMode,pstAttr);
        SND_SetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        Aop = SND_OpGetAopId(hSndOp); //verify
        state = (SND_ENGINE_STATE_S *)hEngineNew;//verify
        HAL_AOE_ENGINE_Stop(state->enEngine);
		HAL_AOE_ENGINE_AttachAop(state->enEngine, Aop);
        SND_StartOp(pCard, SND_GetOpOutputport(hSndOp));

        //set aip
        HAL_AOE_AIP_GetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
        stAipAttr.stBufInAttr.u32BufBitPerSample = pstAttr->u32HbrBitDepth;
        stAipAttr.stBufInAttr.u32BufSampleRate = pstAttr->u32HbrSampleRate;
        stAipAttr.stBufInAttr.u32BufChannels   = pstAttr->u32HbrChannels;
        stAipAttr.stBufInAttr.u32BufDataFormat = pstAttr->u32HbrFormat;

        stAipAttr.stFifoOutAttr.u32FifoBitPerSample = pstAttr->u32HbrBitDepth;
        stAipAttr.stFifoOutAttr.u32FifoSampleRate = pstAttr->u32HbrSampleRate;
        stAipAttr.stFifoOutAttr.u32FifoChannels   = pstAttr->u32HbrChannels;
        stAipAttr.stFifoOutAttr.u32FifoDataFormat = pstAttr->u32HbrFormat;
        HAL_AOE_AIP_SetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
		HAL_AOE_ENGINE_AttachAip(state->enEngine, pTrack->enAIP[enEngineNew]);

        pCard->u32HdmiDataFormat = stAipAttr.stBufInAttr.u32BufDataFormat;
        //set engine
        stEngineAttr.u32BitPerSample = pstAttr->u32HbrBitDepth;
        stEngineAttr.u32Channels   = pstAttr->u32HbrChannels;
        stEngineAttr.u32SampleRate = pstAttr->u32HbrSampleRate;
        stEngineAttr.u32DataFormat = pstAttr->u32HbrFormat;
		HAL_AOE_ENGINE_SetAttr(state->enEngine, &stEngineAttr);
		HAL_AOE_ENGINE_Start(state->enEngine);
    }
    else if ((TRACK_STREAMMODE_CHANGE_LBR2PCM == enMode) || (TRACK_STREAMMODE_CHANGE_HBR2PCM == enMode))
    {
        mt_handle hSndPcmOnlyOp;
        SND_OP_ATTR_S stPcmOnlyOpAttr;
        enEngineOld = SND_ENGINE_TYPE_HDMI_RAW;
        enEngineNew = SND_ENGINE_TYPE_PCM;
        hEngineOld = TrackGetEngineHandlebyType(pCard, enEngineOld);
        hEngineNew = TrackGetEngineHandlebyType(pCard, enEngineNew);

        //set op
        s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        if(MT_SUCCESS != s32Ret)
        {
            MT_ERR_AO("SND_GetOpAttr Fail\n");
            return ;
        }

        SND_StopOp(pCard, SND_GetOpOutputport(hSndOp));
		state = (SND_ENGINE_STATE_S *)hEngineOld; //verify
		HAL_AOE_ENGINE_DetachAop(state->enEngine, Aop);
		HAL_AOE_ENGINE_DetachAip(state->enEngine, pTrack->enAIP[enEngineOld]);
		HAL_AOE_ENGINE_Stop(state->enEngine);

        // reset attr
        hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_DAC);
        if (!hSndPcmOnlyOp)
        {
            hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
        }

        if (!hSndPcmOnlyOp)
        {
            TranslateOpAttr(&stOpAttr,enMode,pstAttr);
        }
        else
        {
            s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndPcmOnlyOp), &stPcmOnlyOpAttr);  //verify
            if(MT_SUCCESS != s32Ret)
            {
                MT_ERR_AO("SND_GetOpAttr Fail\n");
                return ;
            }
            stOpAttr.u32BitPerSample = stPcmOnlyOpAttr.u32BitPerSample;
            stOpAttr.u32Channels   = stPcmOnlyOpAttr.u32Channels;
            stOpAttr.u32SampleRate = stPcmOnlyOpAttr.u32SampleRate;
            stOpAttr.u32PeriodBufSize = stPcmOnlyOpAttr.u32PeriodBufSize;
            stOpAttr.u32LatencyThdMs = stPcmOnlyOpAttr.u32LatencyThdMs;
            stOpAttr.u32DataFormat = 0;
        }

        SND_SetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        Aop = SND_OpGetAopId(hSndOp); //verify
        state = (SND_ENGINE_STATE_S *)hEngineNew;//verify
		HAL_AOE_ENGINE_AttachAop(state->enEngine, Aop);
        SND_StartOp(pCard, SND_GetOpOutputport(hSndOp));
        pCard->u32HdmiDataFormat = 0;
    }

    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode)
        || (TRACK_STREAMMODE_CHANGE_HBR2LBR == enMode))
    {
        pCard->enHdmiPassthrough = SND_HDMI_MODE_LBR;
    }
    else if ((TRACK_STREAMMODE_CHANGE_PCM2HBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2HBR == enMode)
             || (TRACK_STREAMMODE_CHANGE_HBR2HBR == enMode))
    {
        pCard->enHdmiPassthrough = SND_HDMI_MODE_HBR;
    }
    else
    {
        pCard->enHdmiPassthrough = SND_HDMI_MODE_PCM;
    }
    HDMIAudioChange(pCard,enMode,pstAttr);

    memcpy(&pTrack->stStreamAttr, pstAttr, sizeof(SND_TRACK_STREAM_ATTR_S));
}

mt_void SndProcSpidfRoute(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, TRACK_STREAMMODE_CHANGE_E enMode,
                          SND_TRACK_STREAM_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mt_handle hSndOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_SPDIF);
    AOE_AOP_ID_E Aop;
    SND_OP_ATTR_S stOpAttr;
    AOE_ENGINE_CHN_ATTR_S stEngineAttr;
    SND_ENGINE_TYPE_E enEngineNew;
    SND_ENGINE_TYPE_E enEngineOld;
    mt_handle hEngineNew;
    mt_handle hEngineOld;
	SND_ENGINE_STATE_S *state;

    if (TRACK_STREAMMODE_CHANGE_NONE == enMode)
    {
        return;
    }

    Aop = SND_OpGetAopId(hSndOp);
    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode))
    {
        /* enable pass-through */

        //set op
        if (TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode)
        {
            enEngineOld = SND_ENGINE_TYPE_PCM;
        }
        else
        {
            enEngineOld = SND_ENGINE_TYPE_SPDIF_RAW;
        }

        enEngineNew = SND_ENGINE_TYPE_SPDIF_RAW;
        hEngineOld = TrackGetEngineHandlebyType(pCard, enEngineOld);
        hEngineNew = TrackGetEngineHandlebyType(pCard, enEngineNew);

        s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        if(MT_SUCCESS != s32Ret)
        {
            MT_ERR_AO("SND_GetOpAttr Fail\n");
            return ;
        }
        SND_StopOp(pCard, SND_GetOpOutputport(hSndOp));
		state = (SND_ENGINE_STATE_S *)hEngineOld;
		HAL_AOE_ENGINE_DetachAop(state->enEngine, Aop);
        TranslateOpAttr(&stOpAttr,enMode,pstAttr);
        SND_SetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
		state = (SND_ENGINE_STATE_S *)hEngineNew;//verify
		HAL_AOE_ENGINE_Stop(state->enEngine);
		HAL_AOE_ENGINE_AttachAop(state->enEngine, Aop);
        SND_StartOp(pCard, SND_GetOpOutputport(hSndOp));

        //set aip
        HAL_AOE_AIP_GetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
        stAipAttr.stBufInAttr.u32BufBitPerSample = pstAttr->u32LbrBitDepth;
        stAipAttr.stBufInAttr.u32BufSampleRate = pstAttr->u32LbrSampleRate;
        stAipAttr.stBufInAttr.u32BufChannels   = pstAttr->u32LbrChannels;
        stAipAttr.stBufInAttr.u32BufDataFormat = pstAttr->u32LbrFormat;

        stAipAttr.stFifoOutAttr.u32FifoBitPerSample = pstAttr->u32LbrBitDepth;
        stAipAttr.stFifoOutAttr.u32FifoSampleRate = pstAttr->u32LbrSampleRate;
        stAipAttr.stFifoOutAttr.u32FifoChannels   = pstAttr->u32LbrChannels;
        stAipAttr.stFifoOutAttr.u32FifoDataFormat = pstAttr->u32LbrFormat;
        HAL_AOE_AIP_SetAttr(pTrack->enAIP[enEngineNew], &stAipAttr);
		HAL_AOE_ENGINE_AttachAip(state->enEngine, pTrack->enAIP[enEngineNew]);

        pCard->u32SpdifDataFormat = stAipAttr.stBufInAttr.u32BufDataFormat;

        //set engine
        stEngineAttr.u32BitPerSample = pstAttr->u32LbrBitDepth;
        stEngineAttr.u32Channels   = pstAttr->u32LbrChannels;
        stEngineAttr.u32SampleRate = pstAttr->u32LbrSampleRate;
        stEngineAttr.u32DataFormat = pstAttr->u32LbrFormat;
		HAL_AOE_ENGINE_SetAttr(state->enEngine, &stEngineAttr);
		HAL_AOE_ENGINE_Start(state->enEngine);
    }
    else if (TRACK_STREAMMODE_CHANGE_LBR2PCM == enMode)
    {
        /* disable pass-through */
        mt_handle hSndPcmOnlyOp;
        SND_OP_ATTR_S stPcmOnlyOpAttr;
        enEngineOld = SND_ENGINE_TYPE_SPDIF_RAW;
        enEngineNew = SND_ENGINE_TYPE_PCM;
        hEngineOld = TrackGetEngineHandlebyType(pCard, enEngineOld);
        hEngineNew = TrackGetEngineHandlebyType(pCard, enEngineNew);

        //set op
        s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
        if(MT_SUCCESS != s32Ret)
        {
            MT_ERR_AO("SND_GetOpAttr Fail\n");
            return ;
        }

        SND_StopOp(pCard, SND_GetOpOutputport(hSndOp));
		state = (SND_ENGINE_STATE_S *)hEngineOld;
		HAL_AOE_ENGINE_DetachAop(state->enEngine, Aop);
		HAL_AOE_ENGINE_DetachAip(state->enEngine, pTrack->enAIP[enEngineOld]);
		HAL_AOE_ENGINE_Stop(state->enEngine);

        // reset attr
        hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_DAC);
        if (!hSndPcmOnlyOp)
        {
            hSndPcmOnlyOp = SND_GetOpHandlebyOutType(pCard, SND_OUTPUT_TYPE_I2S);
        }

        if (!hSndPcmOnlyOp)
        {
            TranslateOpAttr(&stOpAttr,enMode,pstAttr);
        }
        else
        {
            s32Ret = SND_GetOpAttr(pCard, SND_GetOpOutputport(hSndPcmOnlyOp), &stPcmOnlyOpAttr);
            if(MT_SUCCESS != s32Ret)
            {
                MT_ERR_AO("SND_GetOpAttr Fail\n");
                return ;
            }
            stOpAttr.u32BitPerSample = stPcmOnlyOpAttr.u32BitPerSample;
            stOpAttr.u32Channels   = stPcmOnlyOpAttr.u32Channels;
            stOpAttr.u32SampleRate = stPcmOnlyOpAttr.u32SampleRate;
            stOpAttr.u32PeriodBufSize = stPcmOnlyOpAttr.u32PeriodBufSize;
            stOpAttr.u32LatencyThdMs = stPcmOnlyOpAttr.u32LatencyThdMs;
            stOpAttr.u32DataFormat = 0;
        }

        SND_SetOpAttr(pCard, SND_GetOpOutputport(hSndOp), &stOpAttr);
		state = (SND_ENGINE_STATE_S *)hEngineNew;//verify
		HAL_AOE_ENGINE_AttachAop(state->enEngine, Aop);
        SND_StartOp(pCard, SND_GetOpOutputport(hSndOp));

        pCard->u32SpdifDataFormat = 0;
    }

    if ((TRACK_STREAMMODE_CHANGE_PCM2LBR == enMode) || (TRACK_STREAMMODE_CHANGE_LBR2LBR == enMode))
    {
        pCard->enSpdifPassthrough = SND_SPDIF_MODE_LBR;
    }
    else
    {
        pCard->enSpdifPassthrough = SND_SPDIF_MODE_PCM;
    }

    memcpy(&pTrack->stStreamAttr, pstAttr, sizeof(SND_TRACK_STREAM_ATTR_S));
    return;
}

//zgjiere; u32BufLevelMs需要细心检查异常情况，避免堵塞，u32BufLevelMs异常时，认为无流控
MT_BOOL TrackisBufFree_old(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
    mt_u32 Free = 0;
    mt_u32 DelayMs = 0;
    mt_u32 FrameSize = 0;
    mt_u32 FrameMs = 0;
    mt_u32 PcmFrameBytes = 0;
    mt_u32 SpdifRawBytes = 0;
    mt_u32 HdmiRawBytes = 0;

    PcmFrameBytes = pstStreamAttr->u32PcmBytesPerFrame;
    if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
    {
        Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
        if (Free <= PcmFrameBytes)
        {
            return MT_FALSE;
        }

        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &DelayMs);
        FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32PcmChannels, pstStreamAttr->u32PcmBitDepth);
        FrameMs = AUTIL_ByteSize2LatencyMs(PcmFrameBytes, FrameSize, pstStreamAttr->u32PcmSampleRate);
        if (DelayMs + FrameMs >= pTrack->stUserTrackAttr.u32BufLevelMs)
        {
            return MT_FALSE;
        }
    }

    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            SpdifRawBytes = pstStreamAttr->u32LbrBytesPerFrame;
            Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
            if (Free <= SpdifRawBytes)
            {
                return MT_FALSE;
            }

            HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &DelayMs);  //verify pcm controled , passthrough need control
            FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32LbrChannels, pstStreamAttr->u32LbrBitDepth);
            FrameMs = AUTIL_ByteSize2LatencyMs(SpdifRawBytes, FrameSize, pstStreamAttr->u32LbrSampleRate);
            if (DelayMs + FrameMs >= pTrack->stUserTrackAttr.u32BufLevelMs)
            {
                return MT_FALSE;
            }
        }

        if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            HdmiRawBytes = pstStreamAttr->u32LbrBytesPerFrame;
            Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            if (Free <= HdmiRawBytes)
            {
                return MT_FALSE;
            }

            HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &DelayMs);
            FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32LbrChannels, pstStreamAttr->u32LbrBitDepth);
            FrameMs = AUTIL_ByteSize2LatencyMs(HdmiRawBytes, FrameSize, pstStreamAttr->u32LbrSampleRate);
            if (DelayMs + FrameMs >= pTrack->stUserTrackAttr.u32BufLevelMs)
            {
                return MT_FALSE;
            }
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)

        {
            HdmiRawBytes = pstStreamAttr->u32HbrBytesPerFrame;
            Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            if (Free <= HdmiRawBytes)
            {
                return MT_FALSE;
            }

            HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &DelayMs);
            FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32HbrChannels, pstStreamAttr->u32HbrBitDepth);
            FrameMs = AUTIL_ByteSize2LatencyMs(HdmiRawBytes, FrameSize, pstStreamAttr->u32HbrSampleRate);
            if (DelayMs + FrameMs >= pTrack->stUserTrackAttr.u32BufLevelMs)
            {
                return MT_FALSE;
            }
        }
    }

    return MT_TRUE;
}

//zgjiere; u32BufLevelMs需要细心检查异常情况，避免堵塞，u32BufLevelMs异常时，认为无流控
MT_BOOL TrackisBufFree(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
    mt_u32 Free = 0;
    //mt_u32 DelayMs = 0;
    //mt_u32 FrameSize = 0;
    //mt_u32 FrameMs = 0;
    mt_u32 PcmFrameBytes = 0;
    //mt_u32 SpdifRawBytes = 0;
    //mt_u32 HdmiRawBytes = 0;

    PcmFrameBytes = pstStreamAttr->u32PcmBytesPerFrame;
    //if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
    {
        //Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
        Free = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[pstStreamAttr->u32chan]);

        //MT_INFO_AO("\n TrackisBufFree1  free 0x%x  send 0x%x \n", Free, PcmFrameBytes);
        if (Free <= PcmFrameBytes)
        {
            return MT_FALSE;
        }
    }

    return MT_TRUE;
}

mt_void TRACKStartAip(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack)
{
    mt_u32 u32DelayMs = 0;

    if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &u32DelayMs);
    }
    else if(SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &u32DelayMs);
    }
    else if(SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough || SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &u32DelayMs);
    }

    if(SND_TRACK_STATUS_START == pTrack->enCurnStatus)
    {
        if((MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType && u32DelayMs >= AO_TRACK_AIP_START_LATENCYMS) //for master, 50ms start
            || (MT_UNF_SND_TRACK_TYPE_SLAVE == pTrack->stUserTrackAttr.enTrackType) //for slave, immediately start
            || (MT_TRUE == pTrack->bEosFlag)   //if set eosflag, immediately start
            || (MT_TRUE == pTrack->bAlsaTrack)  //alsa track, immediately start
            || (MT_TRUE == pTrack->bAttAi))     //if track attaches ai, immediately start
        {
            HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
            if(MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
            {
                if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
                    HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
                if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
                    HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            }
             HDMISetAudioUnMute(pCard);
        }
    }

    return;
}

mt_void TRACKPcmUnifyProcess(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
    // todo
    // step 1 , note Interleaved to Interleaved

    // step 2,  7.1+2.0 PCM
}
#if 0 //unuse code
static mt_void TRACKSavePcmData(SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
	mt_s32 s32Len;
    if(SND_DEBUG_CMD_CTRL_START == pTrack->enSaveState)
    {
        if(pstStreamAttr->pPcmDataBuf)
        {
            if(16 == pstStreamAttr->u32PcmBitDepth)
            {
                if(pTrack->fileHandle)
                {
                    mt_u32 u32FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32PcmChannels, pstStreamAttr->u32PcmBitDepth);;
                    s32Len = mt_drv_file_write(pTrack->fileHandle, pstStreamAttr->pPcmDataBuf, pstStreamAttr->u32PcmSamplesPerFrame * u32FrameSize);
                    //pTrack->fileHandle->f_op->write(pTrack->fileHandle, pstStreamAttr->pPcmDataBuf, pstStreamAttr->u32PcmSamplesPerFrame * u32FrameSize, &pTrack->fileHandle->f_pos);
                    if (s32Len != pstStreamAttr->u32PcmSamplesPerFrame * u32FrameSize)
                    {
                        MT_ERR_AO("mt_drv_file_write failed!\n");
                        pTrack->enSaveState = SND_DEBUG_CMD_CTRL_STOP;
                        mt_drv_file_close(pTrack->fileHandle);
                        pTrack->fileHandle = MT_NULL;
                    }
                }
            }
            else if(24 == pstStreamAttr->u32PcmBitDepth)
            {
                mt_u32 i;
                mt_u32 u32TotalSample = pstStreamAttr->u32PcmSamplesPerFrame * pstStreamAttr->u32PcmChannels;
                mt_void *ps8Src = pstStreamAttr->pPcmDataBuf;
                for(i = 0; i < u32TotalSample; i++)
                {
                    if(pTrack->fileHandle)
                    {
                        s32Len = mt_drv_file_write(pTrack->fileHandle, ps8Src + i * 4 + 1, 3);
                        if (s32Len != 3)
                        {
                            MT_ERR_AO("mt_drv_file_write failed!\n");
                            pTrack->enSaveState = SND_DEBUG_CMD_CTRL_STOP;
                            mt_drv_file_close(pTrack->fileHandle);
                            pTrack->fileHandle = MT_NULL;
                        }
                    }
                }
            }
        }
    }

    return;
}
#endif
static mt_u32 UpdateSRCSampleNum(mt_u32 SampleNum_frame, mt_u32 sample_rate, mt_u8 src_mode)
{
  mt_u32 SampleNum_frame_src = 0;

  if(src_mode)
  {
    /*old src mode */
    SampleNum_frame_src = SampleNum_frame * 48000/sample_rate;
  }
  else
  {
    switch(sample_rate)
    {
      case 8000:
      case 11025:
      case 12000:
      {
        SampleNum_frame_src = SampleNum_frame << 2;
      }
      break;
       case 16000:
      case 22050:
      case 24000:
      {
        SampleNum_frame_src = SampleNum_frame << 1;
      }
      break;
      default:
      {
        SampleNum_frame_src = SampleNum_frame;
      }
      break;
    }
  }
return SampleNum_frame_src;
}

static mt_u32 UpdateSRCSamplerate(mt_u32 sample_rate, mt_u8 src_mode)
{
  mt_u32 incnum = 0;
  if(src_mode)
  {
    /*old src mode */
    incnum = 480;
  }
  else
  {
    switch(sample_rate)
    {
      case 8000:
      case 16000:
        incnum = 320;
        break;

      case 11025:
      case 22050:
        incnum = 441;
        break;

      case 12000:
      case 24000:
      default:
        incnum = 480;
        break;
    }
  }
  return incnum;
}

mt_u8 samplerate_2_index(mt_u32 sample)
{
    mt_u8 index = 0;

    if(sample <= 8000)
      index = 0xa;
    else if(sample <= 11025)
      index = 0x8;
    else if(sample <= 12000)
      index = 0x9;
    else if(sample <= 16000)
      index = 0x2;
    else if(sample <= 22050)
      index = 0x0;
    else if(sample <= 24000)
      index = 0x1;
    else if(sample <= 32000)
      index = 0x6;
    else if(sample <= 44100)
      index = 0x4;
    else if(sample <= 48000)
      index = 0x5;
    else if(sample <= 64000)
      index = 0xe;
    else if(sample <= 88200)
      index = 0xc;
    else
      index = 0xd;
   return index;
}

/*
1,set sample rate
2,set sample num reg
*/
void HAL_AOE_AIP_SetPcmSampleRate2(SND_CARD_STATE_S *pCard,SND_TRACK_STREAM_ATTR_S * pstStreamAttr,MT_U32 adecpassthr)
{
    mt_u32 samplerate = pstStreamAttr->u32PcmSampleRate;
    MT_BOOL b_x4 = pstStreamAttr->bEac4TimeSampleRate;
    mt_u32 samplenum = 0;
    mt_u8 index;
    mt_u8 src_mode;
    mt_u32 val;

    MT_INFO_AO("++adectype =%x\n",adecpassthr);
    MT_ALWAYS_PRINT("Cfg-JY[%d/%d/%d/%d]\n", pstStreamAttr->u32chan, samplerate, adecpassthr, b_x4);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    if(0xff == pstStreamAttr->u32ChannelExist){//7.1 channel
        crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_288M);
    }else{//5.1 channel and others
        crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_262M);
    }
#endif    
    
    if(SND_ENGINE_TYPE_PCM == pstStreamAttr->u32chan)
    {
            index = samplerate_2_index(samplerate);
            src_mode = 0;  // 1: old src  0: new src

            //set index
            reg_sym_linux_00_reg_sample_rate_bit(index); //p_sam->bitc.sample_rate = index;

            if((32000 <= samplerate) && (samplerate != 64000))
            {
                reg_sym_linux_pp_reg_src_bit(0); //p_pp_enable->bitc.src_en = 0;
                val = samplerate / 100; //for 44.1 88.2
                if(b_x4){
                    val *= 4;
                    MT_INFO_AO("eac3, set to 4 times val = %d\n", val);
                }else{
                    MT_INFO_AO("not eac3, set to normal val = %d\n", val);
                }

                reg_sym_linux_clk1_reg_div_bit((AUD_SAMPLE_EF * val) / 10);
                samplenum = pstStreamAttr->u32PcmSamplesPerFrame;
            }
            else  //enable src
            {
                // new src do not support large than 48k
                if(64000 == samplerate)
                src_mode = 1;

                reg_sym_linux_pp_reg_src_bit(1); //p_pp_enable->bitc.src_en = 1;
                reg_sym_linux_pp_reg_src_mod_bit(src_mode);
                //p_pp_enable->bitc.old_src_flag = src_mode;
                //p_pcm_sample_clk->bitc.clk_divider_factor = AUD_SAMPLE_EF * 48;

                reg_sym_linux_clk1_reg_div_bit(AUD_SAMPLE_EF * UpdateSRCSamplerate(samplerate, src_mode) / 10);
                samplenum = UpdateSRCSampleNum(pstStreamAttr->u32PcmSamplesPerFrame, samplerate, src_mode);

                MT_INFO_AO("     oooooosrc     \n");
            }

            {
                reg_sym_linux_sample_num_reg_sample_num_bit(samplenum);
                MT_INFO_AO("samplenum is 0x%x\n", samplenum);
            }
        if(0==adecpassthr){     //aout link to
            MT_INFO_AO("++aout.pass pcm\n");
            reg_sym_linux_pp_reg_chan_mod_bit(0);
            reg_sym_linux_04_reg_pcm_chan_spdif_bit(0);
            #ifdef CONFIG_MT_CHIP_SYMPHONY4
            reg_sym_linux_04_reg_spdif_chan_sel_hdmi_bit(0);
            #endif
            if(MT_UNF_SND_SPDIF_MODE_LPCM==pCard->enUserSpdifMode){
                reg_sym_linux_04_reg_spdif_chan_sel_bit(0);     //choose aout channel
                MT_INFO_AO("++spd.pass pcm \n");
            }else{
                #ifdef CONFIG_MT_CHIP_SYMPHONY4
                reg_sym_linux_04_reg_spdif_chan_sel_bit(1);           //choose spdf channel
                MT_INFO_AO("++spd.pass raw \n");
                #endif
            }
        }else{    //aout paketed
            MT_INFO_AO("++aout.pass pack\n");
            val = samplerate / 100; //for 44.1 88.2
            if(b_x4){
                val *= 4;
                MT_INFO_AO("eac3, set to 4 times val = %d samplerate %d\n", val, samplerate);
            }else{
                MT_INFO_AO("not eac3, set to normal val = %d samplerate %d\n", val, samplerate);
            }

            reg_sym_linux_clk1_reg_div_bit((AUD_SAMPLE_EF * val) / 10);

            reg_sym_linux_04_reg_pcm_chan_spdif_bit(1);     //spdif data
            reg_sym_linux_pp_reg_chan_mod_bit(1);
            reg_sym_linux_04_reg_spdif_chan_spdif_bit(0);   //spdif format
            #ifdef CONFIG_MT_CHIP_SYMPHONY4
                reg_sym_linux_04_reg_spdif_chan_sel_hdmi_bit(0);  //hdmi force aout
            #endif
            reg_sym_linux_04_reg_spdif_chan_sel_bit(0);       //spdf force aout
        }
    }
    else if(SND_ENGINE_TYPE_SPDIF_RAW == pstStreamAttr->u32chan)
    {
        MT_INFO_AO("++spd.pass pack\n");
        val = samplerate / 100; //for 44.1 88.2
        if(b_x4){
            val *= 4;
            MT_INFO_AO("eac3, set to 4 times val = %d samplerate %d\n", val, samplerate);
        }else{
            MT_INFO_AO("not eac3, set to normal val = %d samplerate %d\n", val, samplerate);
        }

        reg_sym_linux_clk1_reg_div_bit2((AUD_SAMPLE_EF * val) / 10); 
        if(b_x4){//for eac3 data format
            reg_sym_linux_clk1_reg_div_bit((AUD_SAMPLE_EF * val) / 10 / 4);
#ifndef CONFIG_MT_CHIP_SYMPHONY4
            reg_sym_linux_volume_reg_spdif_mute_coax_bit(1);//spdif port can't allow to output eac3 data
#endif
        }else{//for ac3 data format
            reg_sym_linux_clk1_reg_div_bit((AUD_SAMPLE_EF * val) / 10); 
#ifndef CONFIG_MT_CHIP_SYMPHONY4
            reg_sym_linux_volume_reg_spdif_mute_coax_bit(0);
#endif
        }    
        if((32000 <= samplerate) && (samplerate != 64000)){
            reg_sym_linux_pp_reg_src_bit(0);
        }else{
            reg_sym_linux_pp_reg_src_bit(1);
        }    

        reg_sym_linux_04_reg_spdif_chan_spdif_bit(1);  //spdif format
        #ifdef CONFIG_MT_CHIP_SYMPHONY4
            reg_sym_linux_04_reg_spdif_chan_sel_hdmi_bit(1);
            if(MT_UNF_SND_SPDIF_MODE_RAW==pCard->enUserSpdifMode){
                reg_sym_linux_04_reg_spdif_chan_sel_bit(1);       //spdif choose spdbuf
                MT_INFO_AO("++spd.pass raw \n");
            }else{
                reg_sym_linux_04_reg_spdif_chan_sel_bit(0);       //spdif choose aoubuf
                MT_INFO_AO("++spd.pass pcm \n");
            }
        #else
            reg_sym_linux_04_reg_spdif_chan_sel_bit(1);           //spdif choose spdbuf
        #endif
    }

}
mt_u32 TrackGetEnaip(SND_CARD_STATE_S *pCard)
{
    SND_TRACK_STATE_S *trackerstate=(SND_TRACK_STATE_S *)pCard->hSndTrack[0];
    return trackerstate->enAIP[SND_ENGINE_TYPE_PCM];
}

static mt_void TRACKSetHDMI(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
     //set hdmi

    {
        unsigned long flags;
              HDMI_AUDIO_ATTR_S stHDMIAttr;

                if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAoAttr)
                {
                    (pCard->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
                }

                if(MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode)//((MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode) && !pTrack->b_spdif_mod)
                {
                    stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
                    MT_INFO_AO("TRACKSetHDMI I2SSSSS,%x,%x\n",pCard->enUserHdmiMode,pTrack->b_spdif_mod);
                }

                else
                {
                    stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
                    MT_INFO_AO("TRACKSetHDMI SPDIFFFFFF,%x,%x\n",pCard->enUserHdmiMode,pTrack->b_spdif_mod);
                }


                if((32000 <= pstStreamAttr->u32PcmSampleRate) && (pstStreamAttr->u32PcmSampleRate != 64000)) //hdmi supports
                {
                  if(pstStreamAttr->bEac4TimeSampleRate)
                  {

                     stHDMIAttr.enSampleRate = pstStreamAttr->u32PcmSampleRate * 4;
                     MT_INFO_AO("sample rate set hdmi * 4 %d\n", stHDMIAttr.enSampleRate);
                  }

                  else
                    stHDMIAttr.enSampleRate = pstStreamAttr->u32PcmSampleRate;
                }
                else //hdmi not support ,enable src
                  stHDMIAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;

                MT_INFO_AO("TRACKSetHDMI stHDMIAttr.enSampleRate %d\n", stHDMIAttr.enSampleRate);
#if 1

				{
					mt_u32  is_cfg_ok = FALSE;
					mt_u32  is_cfg_faild = FALSE;
				    //mt_u32  hdmi_inner_state = 0;

					//audio start will cfg spdif or i2s
					local_irq_save(flags);
#if 0
					if (pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetStateMachine)
					{
						pCard->pstHdmiFunc->pfnHdmiGetStateMachine(MT_UNF_HDMI_ID_0, &hdmi_inner_state);
						printk("audio get hdmi sm %d\n", hdmi_inner_state);
					}
#endif

					//if(pCard->hdmi_acfg_flag && hdmi_inner_state == HDMI_SM_CFG_AUDIO)
					if(pCard->hdmi_acfg_flag)
					{
						/*get the capability of the max pcm channels of the output device*/
						if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiAudioChange)
						{
							//p_hdmi_attr = &stHDMIAttr;
							//printk("aud p_hdmi_attr %p\n", p_hdmi_attr);
							MT_INFO_AO("++++drv.tracke.set hdmi...\n");
							if (MT_SUCCESS == (pCard->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0, &stHDMIAttr))
							{
								pCard->hdmi_acfg_flag = 0;
								is_cfg_ok = TRUE;
							}
							else
							{
								pCard->hdmi_acfg_flag++;
								if(pCard->hdmi_acfg_flag > HDMI_AUDIO_CONFIG_TIMES)
								{
									is_cfg_faild = TRUE;
									pCard->hdmi_acfg_flag = 0;
								}
							}
						//printk("\n\n  cccccccccccccccccccccc       pfnHdmiAudioChange be called\n");
						}
					}

					local_irq_restore(flags);

					if(is_cfg_ok)
					{
						MT_INFO_AO("audio symphony HDMI cfg success!!!!\n");
					}

					if(is_cfg_faild)
					{
						MT_INFO_AO("audio symphony HDMI cfg failed %d times!!!! so do not to config ti again!!\n", HDMI_AUDIO_CONFIG_TIMES);
					}
				}
#endif
            }
}

static mt_u32 Getchans_bylayout(int layout)
{//10111111
    int i=0,cnt=0;
    for(i=0;i<8;i++){
        if(layout&(1<<i)){
            cnt++;
        }
    }
    return cnt;
}
static mt_void Config_mix_bychans(int channel,MT_UNF_TRACK_MODE_E output_fmt,int advolume)
{
    MT_ERR_AO("+++cmix,ch=%x,fmt=%x,adv=%x\n",channel,output_fmt,advolume);
    reg_sym_linux_pp_reg_downmix_bit(1);      //enable downmix
    reg_sym_linux_pp_reg_downmix_hdmi_bit(1); //enable hdmi downmix

    if(1 == channel)
    {
        reg_sym_linux_downmix_coef_left_2_all_bit(0x7ff07ff0);
        reg_sym_linux_downmix_coef_right_2_all_bit(0); //r
        reg_sym_linux_downmix_coef_bass_2_all_bit(0); //lfe
        reg_sym_linux_downmix_coef_center_2_all_bit(0); //c
        reg_sym_linux_downmix_coef_sl_2_all_bit(0); //sl
        reg_sym_linux_downmix_coef_sr_2_all_bit(0); //sr
        reg_sym_linux_downmix_coef_rsl_2_all_bit(0); //rsl
        reg_sym_linux_downmix_coef_rsr_2_all_bit(0); //rsr
        if(advolume) //ad volume
        {
            u32 ad_value = 0x8000*advolume/100;
            ad_value|=(ad_value<<16);
            reg_sym_linux_downmix_coef_rsl_2_all_bit(ad_value);
        }
    }
    else if(2 == channel)
    {
        if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt)
        {
          reg_sym_linux_downmix_coef_left_2_all_bit(0x3ff03ff0);
          reg_sym_linux_downmix_coef_right_2_all_bit(0x3ff03ff0);
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == output_fmt)
        {
          reg_sym_linux_downmix_coef_left_2_all_bit(0x7ff07ff0);
          reg_sym_linux_downmix_coef_right_2_all_bit(0);
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == output_fmt)
        {
          reg_sym_linux_downmix_coef_left_2_all_bit(0);
          reg_sym_linux_downmix_coef_right_2_all_bit(0x7ff07ff0);
        }
        else if(MT_UNF_TRACK_MODE_STEREO == output_fmt)
        {
          reg_sym_linux_downmix_coef_left_2_all_bit(0x00007ff0);
          reg_sym_linux_downmix_coef_right_2_all_bit(0X7ff00000);
        }
        reg_sym_linux_downmix_coef_bass_2_all_bit(0); //lfe
        reg_sym_linux_downmix_coef_center_2_all_bit(0); //c
        reg_sym_linux_downmix_coef_sl_2_all_bit(0); //sl
        reg_sym_linux_downmix_coef_sr_2_all_bit(0); //sr
        reg_sym_linux_downmix_coef_rsl_2_all_bit(0); //rsl
        reg_sym_linux_downmix_coef_rsr_2_all_bit(0); //rsr
        if(advolume) //ad volume
        {
            u32 ad_value = 0x8000*advolume/100;
            if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt){
                ad_value=ad_value/2;
            }
            ad_value|=(ad_value<<16);
            reg_sym_linux_downmix_coef_rsl_2_all_bit(ad_value);
        }
    }
    else if((3 <= channel) && (channel <= 6))
    {
        if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x15501550); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x15501550); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x15501550); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x15501550); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x15501550); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x15501550); //sr
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x20002000); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x20002000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x20002000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x20002000); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0); //sr
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x20002000); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x20002000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x20002000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x20002000); //sr
        }
        else if(MT_UNF_TRACK_MODE_STEREO == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x2000); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x20000000); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x20002000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x20002000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x2000); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x20000000); //sr
        }
        reg_sym_linux_downmix_coef_rsl_2_all_bit(0); //rsl
        reg_sym_linux_downmix_coef_rsr_2_all_bit(0); //rsr
        if(advolume) //ad volume
        {
            u32 ad_value = 0x2000*advolume/100;
            if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt){
                ad_value = 0x1550*advolume/100;
            }
            ad_value|=(ad_value<<16);
            reg_sym_linux_downmix_coef_rsl_2_all_bit(ad_value);
        }
    }
    else if(6 < channel)
    {
        if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x10001000); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x10001000); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x10001000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x10001000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x10001000); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x10001000); //sr
            reg_sym_linux_downmix_coef_rsl_2_all_bit(0x10001000); //rsl
            reg_sym_linux_downmix_coef_rsr_2_all_bit(0x10001000); //rsr
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x20002000); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x10001000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x10001000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x20002000); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0); //sr
            reg_sym_linux_downmix_coef_rsl_2_all_bit(0x20002000); //rsl
            reg_sym_linux_downmix_coef_rsr_2_all_bit(0); //rsr
        }
        else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x20002000); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x10001000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x10001000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x20002000); //sr
            reg_sym_linux_downmix_coef_rsl_2_all_bit(0); //rsl
            reg_sym_linux_downmix_coef_rsr_2_all_bit(0x20002000); //rsr
        }
        else if(MT_UNF_TRACK_MODE_STEREO == output_fmt)
        {
            reg_sym_linux_downmix_coef_left_2_all_bit(0x2000); //l
            reg_sym_linux_downmix_coef_right_2_all_bit(0x20000000); //r
            reg_sym_linux_downmix_coef_bass_2_all_bit(0x10001000); //lfe
            reg_sym_linux_downmix_coef_center_2_all_bit(0x10001000); //c
            reg_sym_linux_downmix_coef_sl_2_all_bit(0x2000); //sl
            reg_sym_linux_downmix_coef_sr_2_all_bit(0x20000000); //sr
            reg_sym_linux_downmix_coef_rsl_2_all_bit(0); //rsl
            reg_sym_linux_downmix_coef_rsr_2_all_bit(0x20000000); //rsr
        }
        if(advolume)
        {//ad volume
            u32 ad_value = 0x2000*advolume/100;
            if(MT_UNF_TRACK_MODE_DOUBLE_MONO == output_fmt){
                ad_value = 0x1000*advolume/100;
            }
            ad_value|=(ad_value<<16);
            reg_sym_linux_downmix_coef_rsl_2_all_bit(ad_value);
        }
    }
}
static mt_void TRACKWriteFrame(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr,MT_U32 adectype)
{
    mt_u32 chans=0;
    mt_u32 Write = 0;
    mt_u32 PcmFrameBytes = 0;
    mt_u32 adectype_passthr = 0;
    static mt_u32 mix_cfg_flag = 0;
    //mt_u32 usr_confighdmi_aout=0;

    //mt_u32 SpdifRawBytes = 0;
    //mt_u32 HdmiRawBytes = 0;
    //mt_s32 ret = 0;

    //TRACKSavePcmData(pTrack, pstStreamAttr);
    //TRACKPcmUnifyProcess(pCard,pTrack, pstStreamAttr);
    if((HA_AUDIO_ID_AC3PASSTHROUGH==adectype) ||
      (HA_AUDIO_ID_EAC3PASSTHROUGH==adectype) ||
      (HA_AUDIO_ID_DTSPASSTHROUGH==adectype)){
        adectype_passthr=1;
    }
    
    if((SND_ENGINE_TYPE_PCM == pstStreamAttr->u32chan) && (0==adectype_passthr)){
        int flag_adchange=0;
#ifdef CONFIG_MT_AUDIO_AD
        if(pCard->ao_record.volume_ad_now!=pCard->ao_record.volume_ad){
            flag_adchange=1;
        }
#endif
        if((pCard->ao_record.trackmode!=pCard->ao_record.trackmode_now) || 
         ((pstStreamAttr->u32ChannelExist&0xbf)!=(pCard->ao_record.channles_now&0xbf)) || flag_adchange){//config mix
          mt_u32 advolume=0;
          MT_ERR_AO("+++configmix:oldmode=%x,new=%x;oldchs=%x,new=%x\n",
            pCard->ao_record.trackmode,pCard->ao_record.trackmode_now,pstStreamAttr->u32ChannelExist,pCard->ao_record.channles_now);
          pCard->ao_record.trackmode_now=pCard->ao_record.trackmode;
          pCard->ao_record.channles_now=pstStreamAttr->u32ChannelExist;
#ifdef CONFIG_MT_AUDIO_AD
          pCard->ao_record.volume_ad_now=pCard->ao_record.volume_ad;
          advolume=pCard->ao_record.volume_ad;
#endif
          chans=Getchans_bylayout((pstStreamAttr->u32ChannelExist&0xbf));
          Config_mix_bychans(chans,pCard->ao_record.trackmode_now,((pstStreamAttr->u32ChannelExist&0x40)?advolume:0));
        }
    }
    
    {
        PcmFrameBytes = pstStreamAttr->u32PcmBytesPerFrame;

        if((SND_ENGINE_TYPE_PCM == pstStreamAttr->u32chan) &&
            (((0==adectype_passthr) && (MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode))||
             ((1==adectype_passthr) && (MT_UNF_SND_HDMI_MODE_LPCM != pCard->enUserHdmiMode))))
        {//aout-channel , (adec not pass && user choose pcm || adec pass && user choose spd)
            //printk("+++pcm.%x,%x,%x\n",adectype_passthr,pCard->enUserHdmiMode,adectype);
            if((pstStreamAttr->u32PcmSampleRate != pTrack->u32PcmSampleRate) ||
               (pstStreamAttr->bEac4TimeSampleRate != pTrack->bEac4TimeSampleRate) ||
               pCard->hdmi_acfg_flag || pCard->hdmi_acfg_usrchg)
            {//(pCard->hdmi_acfg_over)
                MT_INFO_AO("+++aout.data,%x,%d\n",adectype_passthr,pTrack->b_spdif_mod);
                MT_INFO_AO("+++samplerate_changed.pcm,before %d %u, now %d %u hdmimode %d,%p,%x,%x\n",
                		  pTrack->bEac4TimeSampleRate, pTrack->u32PcmSampleRate,
                		  pstStreamAttr->bEac4TimeSampleRate, pstStreamAttr->u32PcmSampleRate,
                		  pCard->enUserHdmiMode,
                          pTrack,pCard->hdmi_acfg_flag,pCard->hdmi_acfg_usrchg );

                pTrack->u32PcmSampleRate = pstStreamAttr->u32PcmSampleRate;
                pTrack->bEac4TimeSampleRate = pstStreamAttr->bEac4TimeSampleRate;
                if(0==pCard->hdmi_acfg_flag){  //maybe reloop as 2,1,2,1,2,1.....
                    pCard->hdmi_acfg_flag = 1;
                }

                //set audio hardware
                //HAL_AOE_AIP_SetPcmSampleRate(pstStreamAttr->u32PcmSampleRate, pstStreamAttr->bEac4TimeSampleRate);
                HAL_AOE_AIP_SetPcmSampleRate2(pCard,pstStreamAttr,adectype_passthr);
                mix_cfg_flag = 1;

                //set hdmi
                //if(MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode)
                {
                    TRACKSetHDMI(pCard, pTrack, pstStreamAttr);
                }
                pCard->hdmi_acfg_usrchg=0;
            }
        }
        else if(SND_ENGINE_TYPE_SPDIF_RAW == pstStreamAttr->u32chan)
        {//spd-channel , adec not pass && user choose pcm && call MT_UNF_SND_SetHdmiMode
            //printk("+++spd.%x,%x,%x\n",adectype_passthr,pCard->enUserHdmiMode,adectype);
            if((0==adectype_passthr) && (MT_UNF_SND_HDMI_MODE_LPCM != pCard->enUserHdmiMode)){
                if((pstStreamAttr->u32PcmSampleRate != pTrack->u32PcmSampleRate2) ||
                   (pstStreamAttr->bEac4TimeSampleRate != pTrack->bEac4TimeSampleRate2)||
                   pCard->hdmi_acfg_flag ||pCard->hdmi_acfg_usrchg)
                {
                    MT_INFO_AO("+++spd.data %x,%x\n",adectype_passthr,pTrack->b_spdif_mod);
                    MT_INFO_AO("+++samplerate_changed.spd,before %d %u, now %d %u hdmimode %d,%p,%x,%x\n",
                    	pTrack->bEac4TimeSampleRate, pTrack->u32PcmSampleRate,
                    	pstStreamAttr->bEac4TimeSampleRate, pstStreamAttr->u32PcmSampleRate,
                    	pCard->enUserHdmiMode,
                    	pTrack,pCard->hdmi_acfg_flag,pCard->hdmi_acfg_usrchg);

                    pTrack->u32PcmSampleRate2 = pstStreamAttr->u32PcmSampleRate;
                    pTrack->bEac4TimeSampleRate2 = pstStreamAttr->bEac4TimeSampleRate;
                    if(0==pCard->hdmi_acfg_flag){  //maybe reloop as 2,1,2,1,2,1.....
                        pCard->hdmi_acfg_flag = 1;
                    }
                    //set audio hardware
                    //HAL_AOE_AIP_SetPcmSampleRate(pstStreamAttr->u32PcmSampleRate, pstStreamAttr->bEac4TimeSampleRate);
                    HAL_AOE_AIP_SetPcmSampleRate2(pCard,pstStreamAttr,adectype_passthr);
                    mix_cfg_flag = 2;

                    //set hdmi
                    if(MT_UNF_SND_HDMI_MODE_RAW == pCard->enUserHdmiMode){
                        TRACKSetHDMI(pCard, pTrack, pstStreamAttr);
                    }
                    pCard->hdmi_acfg_usrchg=0;
                }
            }else{//configure clk.divd.Bug 117177
                mt_u32 valv,regv;
                //mt_u32 samplerate = pstStreamAttr->u32PcmSampleRate;
                MT_BOOL b_x4 = pstStreamAttr->bEac4TimeSampleRate;

                valv = (pstStreamAttr->u32PcmSampleRate / 100);
                if(b_x4){
                    valv *= 4;
                }
                valv=(AUD_SAMPLE_EF * valv) / 10;
                regv=reg_sym_linux_get_clk1_reg_div_bit2();
                if(valv != regv){
                    reg_sym_linux_clk1_reg_div_bit2(valv);
                    MT_INFO_AO("+++ao.chn2.only.clk.set %x\n",valv);
                }
                #ifndef CONFIG_MT_CHIP_SYMPHONY4
                    if(!b_x4){  //dd+ can't config
                        if(MT_UNF_SND_SPDIF_MODE_LPCM != pCard->enUserSpdifMode){
                            regv=reg_sym_linux_04_reg_get_all();
                            if((regv&0x600)!=0x600){
                                reg_sym_linux_04_reg_spdif_chan_spdif_bit(1);     //spdif format
                                reg_sym_linux_04_reg_spdif_chan_sel_bit(1);       //choose spdif channel
                                MT_INFO_AO("+++ao.chn2.path.spdif\n");
                            }
                        }
                    }else{      //dd+ set pcm path  //fixbug119857                      
                        regv=reg_sym_linux_04_reg_get_all();                      
                        if(0x00!=(regv&0x500)){//101.0x00                          
                            reg_sym_linux_04_reg_pcm_chan_spdif_bit(0);         //aout pcm                          
                            reg_sym_linux_04_reg_spdif_chan_sel_bit(0);         //choose aout channel                          
                            MT_INFO_AO("+++spd.pass force pcm \n");                      
                        }
                    }
                #else
                    regv=reg_sym_linux_04_reg_get_all();
                    if(MT_UNF_SND_SPDIF_MODE_LPCM != pCard->enUserSpdifMode){
                        if((regv&0x600)!=0x600){
                            reg_sym_linux_04_reg_spdif_chan_spdif_bit(1);       //spdif format
                            reg_sym_linux_04_reg_spdif_chan_sel_bit(1);         //choose spdif channel
                            MT_INFO_AO("+++ao.chn2.path.spdif\n");
                        }
                    }else{
                        if(0x00!=(regv&0x500)){//101.0x00
                            reg_sym_linux_04_reg_pcm_chan_spdif_bit(0);         //aout pcm                          
                            reg_sym_linux_04_reg_spdif_chan_sel_bit(0);         //choose aout channel                          
                            MT_INFO_AO("+++spd.pass force pcm \n");                      
                        } 
                    }
                #endif
            }
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
		else if(SND_ENGINE_TYPE_MIXBUF_DD == pstStreamAttr->u32chan){            
            if(mix_cfg_flag){
                MT_INFO_AO("+++mix.data %x,%x\n",pCard->enUserHdmiMode,pTrack->b_spdif_mod);
                //reg_sym_linux_mix_func_cfg_aud_res_reg0_bit(0x82);
                //reg_sym_linux_mix_func_cfg_aud_res_reg3_bit(0x2);                
                if(MT_UNF_SND_HDMI_MODE_LPCM == pCard->enUserHdmiMode){
                    reg_sym_linux_aud_vol_cfg_spdif_pcm_vol_ctrl_bit(0x8000);//keep pcm_adac, pcm_hdmi as 0x8000, TODO...
                    reg_sym_linux_aud_vol_cfg_spdif_mix_spdif_alpha_bit(0);
                }else{
                    reg_sym_linux_aud_vol_cfg_spdif_pcm_vol_ctrl_bit(0);
                    reg_sym_linux_aud_vol_cfg_spdif_mix_spdif_alpha_bit(0x8000);
                }
                reg_sym_linux_mix_func_alpha_mix_adac_bit(0);
                reg_sym_linux_mix_func_alpha_mix_hdmi_bit(0);
                reg_sym_linux_04_reg_spdif_chan_spdif_bit(1);
                reg_sym_linux_04_reg_spdif_chan_sel_bit(0);
                reg_sym_linux_mix_func_cfg_mix_en_bit(1);
            }
            mix_cfg_flag = 0;
        }
#endif		
        //Write = HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_PCM], (mt_u8 *)pstStreamAttr->pPcmDataBuf,
        Write = HAL_AOE_AIP_WriteBufData(pTrack->enAIP[pstStreamAttr->u32chan], (mt_u8 *)pstStreamAttr->pPcmDataBuf,
        PcmFrameBytes, pstStreamAttr->u32ChannelExist);
        if (Write != PcmFrameBytes)
        {
            MT_ERR_AO("HAL_AOE_AIP_WriteBufData fail write(%u) actual(%u)\n", PcmFrameBytes, Write);
        }
    }


    #if 0
    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            SpdifRawBytes = pstStreamAttr->u32LbrBytesPerFrame;
            Write = HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW],
                                             (mt_u8 *)pstStreamAttr->pLbrDataBuf, SpdifRawBytes);
            if (Write != SpdifRawBytes)
            {
                MT_ERR_AO("HAL_AOE_AIP_WriteBufData fail write(%d) actual(%d)\n", SpdifRawBytes, Write);
            }
        }

        if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            HdmiRawBytes = pstStreamAttr->u32LbrBytesPerFrame;
            Write = HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW],
                                             (mt_u8 *)pstStreamAttr->pLbrDataBuf, HdmiRawBytes);
            if (Write != HdmiRawBytes)
            {
                MT_ERR_AO("HAL_AOE_AIP_WriteBufData fail write(%d) actual(%d)\n", HdmiRawBytes, Write);
            }
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            HdmiRawBytes = pstStreamAttr->u32HbrBytesPerFrame;
            Write = HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW],
                                             (mt_u8 *)pstStreamAttr->pHbrDataBuf, HdmiRawBytes);
            if (Write != HdmiRawBytes)
            {
                MT_ERR_AO("HAL_AOE_AIP_WriteBufData(%d) fail\n", HdmiRawBytes);
            }
        }
    }
    #endif
    return;
}

static mt_void TRACKWriteMuteFrame(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, SND_TRACK_STREAM_ATTR_S * pstStreamAttr)
{
    mt_u32 SpdifRawFree = 0, HdmiRawFree = 0;
    mt_u32 SpdifRawBusy = 0, HdmiRawBusy = 0;
    mt_u32 SpdifRawData = 0, HdmiRawData = 0;
    mt_u32 PcmDelayMs = 0, SpdifDelayMs = 0, HdmiDelayMs = 0;
    mt_u32 FrameSize = 0;

    if(MT_UNF_SND_TRACK_TYPE_MASTER != pTrack->stUserTrackAttr.enTrackType)
    {
        return;
    }

    if (SND_SPDIF_MODE_PCM >= pCard->enSpdifPassthrough && SND_HDMI_MODE_PCM >= pCard->enHdmiPassthrough)
    {
        return;
    }

    SpdifRawBusy = HAL_AOE_AIP_QueryBufData(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
    HdmiRawBusy = HAL_AOE_AIP_QueryBufData(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);

    //When the data of raw AIP less than two frame, send mute frame
    if ((SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough && SpdifRawBusy < 2 * pstStreamAttr->u32LbrBytesPerFrame)
           || (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough && HdmiRawBusy < 2 * pstStreamAttr->u32LbrBytesPerFrame)
           || (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough && HdmiRawBusy < 2 * pstStreamAttr->u32HbrBytesPerFrame))
    {
        if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
        {
            HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &PcmDelayMs);
            if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
            {
                HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &SpdifDelayMs);
                if(PcmDelayMs > SpdifDelayMs)
                {
                    FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32LbrChannels, pstStreamAttr->u32LbrBitDepth);
                    SpdifRawData = AUTIL_LatencyMs2ByteSize(PcmDelayMs - SpdifDelayMs, FrameSize, pstStreamAttr->u32LbrSampleRate); //mute frame size is difference value between delayms
                    if(SpdifRawData < pstStreamAttr->u32LbrBytesPerFrame)
                    {
                        SpdifRawData = pstStreamAttr->u32LbrBytesPerFrame;   //if  it is less than a frame, send a frame
                    }
                    SpdifRawFree = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
                    if(SpdifRawData > SpdifRawFree)
                    {
                        return;
                    }
                }
            }
            if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
            {

                HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &HdmiDelayMs);
                if(PcmDelayMs > HdmiDelayMs)
                {
                    FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32LbrChannels, pstStreamAttr->u32LbrBitDepth);
                    HdmiRawData = AUTIL_LatencyMs2ByteSize(PcmDelayMs - HdmiDelayMs, FrameSize, pstStreamAttr->u32LbrSampleRate);
                    if(HdmiRawData < pstStreamAttr->u32LbrBytesPerFrame)
                    {
                        HdmiRawData = pstStreamAttr->u32LbrBytesPerFrame;
                    }
                    HdmiRawFree = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
                    if(HdmiRawData > HdmiRawFree)
                    {
                        return;
                    }
                }
            }
            else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
            {
                HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &HdmiDelayMs);
                if(PcmDelayMs > HdmiDelayMs)
                {
                    FrameSize = AUTIL_CalcFrameSize(pstStreamAttr->u32HbrChannels, pstStreamAttr->u32HbrBitDepth);
                    HdmiRawData = AUTIL_LatencyMs2ByteSize(PcmDelayMs - HdmiDelayMs, FrameSize, pstStreamAttr->u32HbrSampleRate);
                    if(HdmiRawData < pstStreamAttr->u32HbrBytesPerFrame)
                    {
                        HdmiRawData = pstStreamAttr->u32HbrBytesPerFrame;
                    }
                    HdmiRawFree = HAL_AOE_AIP_QueryBufFree(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
                    if(HdmiRawData > HdmiRawFree)
                    {
                        return;
                    }
                }
            }
        }
        else
        {
            if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
            {
                SpdifRawData = 2 * pstStreamAttr->u32LbrBytesPerFrame;
            }
            if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
            {
                HdmiRawData = 2 * pstStreamAttr->u32LbrBytesPerFrame;
            }
            else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
            {
                HdmiRawData = 2 * pstStreamAttr->u32HbrBytesPerFrame;
            }
        }

        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            //HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], MT_NULL, SpdifRawData);
        }
        if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            //HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], MT_NULL, HdmiRawData);
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            //HAL_AOE_AIP_WriteBufData(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], MT_NULL, HdmiRawData);
        }

        pTrack->u32AddMuteFrameNum++;
    }

    return;
}

mt_void TrackSetAipRbfAttr(AOE_RBUF_ATTR_S *pRbfAttr, mmz_buffer_s *pstRbfMmz)
{
    pRbfAttr->u32BufPhyAddr = pstRbfMmz->startPhyAddr;
    pRbfAttr->u32BufVirAddr = (ulong)pstRbfMmz->startVirAddr;
    pRbfAttr->u32BufSize = pstRbfMmz->size;
    pRbfAttr->u32BufWptrRptrFlag = 0;  /* cpu write */
}

mt_void TrackGetAipPcmDfAttr(AOE_AIP_CHN_ATTR_S *pstAipAttr, mmz_buffer_s *pstRbfMmz,
                             MT_UNF_AUDIOTRACK_ATTR_S *pstUnfAttr)
{
  pstAipAttr->sound_type = SND_ENGINE_TYPE_PCM;
    TrackSetAipRbfAttr(&pstAipAttr->stBufInAttr.stRbfAttr, pstRbfMmz);

    pstAipAttr->stBufInAttr.stRbfAttr.b_spdif_mode = pstUnfAttr->b_spdif_mod;

    //MT_INFO_AO("\n\n\n\n   spdif modessssssssssssssss  %d   \n\n\n\n", pstAipAttr->stBufInAttr.stRbfAttr.b_spdif_mode);

    pstAipAttr->stBufInAttr.u32BufBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stBufInAttr.u32BufSampleRate = MT_UNF_SAMPLE_RATE_48K;
    pstAipAttr->stBufInAttr.u32BufChannels     = AO_TRACK_NORMAL_CHANNELNUM;
    pstAipAttr->stBufInAttr.u32BufDataFormat   = 0;
    pstAipAttr->stBufInAttr.u32BufLatencyThdMs = pstUnfAttr->u32BufLevelMs;
    pstAipAttr->stBufInAttr.u32FadeinMs  = pstUnfAttr->u32FadeinMs;
    pstAipAttr->stBufInAttr.u32FadeoutMs = pstUnfAttr->u32FadeoutMs;
    pstAipAttr->stBufInAttr.bFadeEnable = MT_FALSE;
    pstAipAttr->stBufInAttr.bAlsaEnable = MT_FALSE;
    pstAipAttr->stBufInAttr.bMixPriority = MT_FALSE;
    if (pstUnfAttr->u32FadeinMs | pstUnfAttr->u32FadeoutMs)
    {
        pstAipAttr->stBufInAttr.bFadeEnable = MT_TRUE;
    }

    pstAipAttr->stFifoOutAttr.u32FifoBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stFifoOutAttr.u32FifoSampleRate = MT_UNF_SAMPLE_RATE_48K;
    pstAipAttr->stFifoOutAttr.u32FifoChannels     = AO_TRACK_NORMAL_CHANNELNUM;
    pstAipAttr->stFifoOutAttr.u32FifoDataFormat   = 0;
    //pstAipAttr->stFifoOutAttr.u32FiFoLatencyThdMs = AIP_FIFO_LATENCYMS_DEFAULT;
}

mt_void TrackGetAipLbrDfAttr(AOE_AIP_CHN_ATTR_S *pstAipAttr, mmz_buffer_s *pstRbfMmz,
                             MT_UNF_AUDIOTRACK_ATTR_S *pstUnfAttr)
{
  pstAipAttr->sound_type = SND_ENGINE_TYPE_SPDIF_RAW;
    TrackSetAipRbfAttr(&pstAipAttr->stBufInAttr.stRbfAttr, pstRbfMmz);
    pstAipAttr->stBufInAttr.u32BufBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stBufInAttr.u32BufSampleRate = MT_UNF_SAMPLE_RATE_48K;
    pstAipAttr->stBufInAttr.u32BufChannels     = AO_TRACK_NORMAL_CHANNELNUM;
    pstAipAttr->stBufInAttr.u32BufDataFormat   = 1;
    pstAipAttr->stBufInAttr.u32BufLatencyThdMs = pstUnfAttr->u32BufLevelMs;
    pstAipAttr->stBufInAttr.bFadeEnable = MT_FALSE;
    pstAipAttr->stBufInAttr.bMixPriority = MT_FALSE;

    pstAipAttr->stFifoOutAttr.u32FifoBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stFifoOutAttr.u32FifoSampleRate = MT_UNF_SAMPLE_RATE_48K;
    pstAipAttr->stFifoOutAttr.u32FifoChannels     = AO_TRACK_NORMAL_CHANNELNUM;
    pstAipAttr->stFifoOutAttr.u32FifoDataFormat   = 1;
    //pstAipAttr->stFifoOutAttr.u32FiFoLatencyThdMs = AIP_FIFO_LATENCYMS_DEFAULT;
}

mt_void TrackGetAipHbrDfAttr(AOE_AIP_CHN_ATTR_S *pstAipAttr, mmz_buffer_s *pstRbfMmz,
                             MT_UNF_AUDIOTRACK_ATTR_S *pstUnfAttr)
{
  pstAipAttr->sound_type = SND_ENGINE_TYPE_HDMI_RAW;
    TrackSetAipRbfAttr(&pstAipAttr->stBufInAttr.stRbfAttr, pstRbfMmz);
    pstAipAttr->stBufInAttr.u32BufBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stBufInAttr.u32BufSampleRate = MT_UNF_SAMPLE_RATE_192K;
    pstAipAttr->stBufInAttr.u32BufChannels     = AO_TRACK_MUTILPCM_CHANNELNUM;
    pstAipAttr->stBufInAttr.u32BufDataFormat   = 1;
    pstAipAttr->stBufInAttr.u32BufLatencyThdMs = pstUnfAttr->u32BufLevelMs;;
    pstAipAttr->stBufInAttr.bFadeEnable = MT_FALSE;
    pstAipAttr->stBufInAttr.bMixPriority = MT_FALSE;

    pstAipAttr->stFifoOutAttr.u32FifoBitPerSample = AO_TRACK_BITDEPTH_LOW;
    pstAipAttr->stFifoOutAttr.u32FifoSampleRate = MT_UNF_SAMPLE_RATE_192K;
    pstAipAttr->stFifoOutAttr.u32FifoChannels     = AO_TRACK_MUTILPCM_CHANNELNUM;
    pstAipAttr->stFifoOutAttr.u32FifoDataFormat   = 1;
    //pstAipAttr->stFifoOutAttr.u32FiFoLatencyThdMs = AIP_FIFO_LATENCYMS_DEFAULT;
}

mt_s32 TrackCreateMaster_old(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *state, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
    mt_s32 Ret;
#ifdef MT_ADAC_SLIC_SUPPORT
    mt_u32 i;
#endif
    AOE_AIP_ID_E enAIP;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mmz_buffer_s stRbfMmz;

    stRbfMmz = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM];
    TrackGetAipPcmDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
	stAipAttr.stFifoOutAttr.u32FifoSampleRate = pCard->enUserSampleRate;
    MT_INFO_AO("Aip FifoOutAttr FifoSampleRate: %d\n", stAipAttr.stFifoOutAttr.u32FifoSampleRate);

#ifdef MT_ADAC_SLIC_SUPPORT
	//for support slic 8K 1ch, avoid default value here, update fifo samplereate and channel
    MT_INFO_AO("Aip FifoOutAttr Default FifoChannels: %d\n", stAipAttr.stFifoOutAttr.u32FifoChannels);
	for (i = 0; i < pCard->stUserOpenParam.u32PortNum; i++)
	{
	    if(MT_UNF_SND_OUTPUTPORT_I2S0 == pCard->stUserOpenParam.stOutport[i].enOutPort)
        {
	        stAipAttr.stFifoOutAttr.u32FifoChannels = (mt_u32)(pCard->stUserOpenParam.stOutport[i].unAttr.stI2sAttr.stAttr.enChannel);
            MT_INFO_AO("Update MT_UNF_SND_OUTPUTPORT_I2S0 FifoChannels: %d\n", stAipAttr.stFifoOutAttr.u32FifoChannels);
       }
	}
#endif
    Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
        goto CREATE_PCM_ERR_EXIT;
    }

    state->enAIP[SND_ENGINE_TYPE_PCM] = enAIP;
    state->stAipRbfMmz[SND_ENGINE_TYPE_PCM] = stRbfMmz;
    state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_PCM] = MT_FALSE;

    if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
    {
        stRbfMmz = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW];
        TrackGetAipLbrDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
        Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
            goto CREATE_SPDIF_ERR_EXIT;
        }

        state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW] = enAIP;
        state->stAipRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW] = stRbfMmz;
        state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_SPDIF_RAW] = MT_FALSE;
    }

    if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
    {
        stRbfMmz = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_HDMI_RAW];
        TrackGetAipHbrDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
        Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
            goto CREATE_HDMI_ERR_EXIT;
        }

        state->enAIP[SND_ENGINE_TYPE_HDMI_RAW] = enAIP;
        state->stAipRbfMmz[SND_ENGINE_TYPE_HDMI_RAW] = stRbfMmz;
        state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_HDMI_RAW] = MT_FALSE;
    }

    state->enCurnStatus = SND_TRACK_STATUS_STOP;
    return MT_SUCCESS;

CREATE_HDMI_ERR_EXIT:
    if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
    {
        HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
    }
CREATE_SPDIF_ERR_EXIT:
    HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_PCM]);
CREATE_PCM_ERR_EXIT:
    return MT_FAILURE;
}


mt_s32 TrackCreateMaster(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *state, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
    mt_s32 Ret;
#ifdef MT_ADAC_SLIC_SUPPORT
    mt_u32 i;
#endif
    AOE_AIP_ID_E enAIP;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mmz_buffer_s stRbfMmz;

    stRbfMmz = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM];
    TrackGetAipPcmDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
    stAipAttr.stFifoOutAttr.u32FifoSampleRate = pCard->enUserSampleRate;
    MT_INFO_AO("Aip FifoOutAttr FifoSampleRate: %d\n", stAipAttr.stFifoOutAttr.u32FifoSampleRate);


    Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
        goto CREATE_PCM_ERR_EXIT;
    }

    state->enAIP[SND_ENGINE_TYPE_PCM] = enAIP;
    state->stAipRbfMmz[SND_ENGINE_TYPE_PCM] = stRbfMmz;

    return MT_SUCCESS;


CREATE_PCM_ERR_EXIT:
    return MT_FAILURE;
}


mt_s32 TrackCreateMasterNew(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *state, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
    mt_s32 Ret;
#ifdef MT_ADAC_SLIC_SUPPORT
    mt_u32 i;
#endif
    AOE_AIP_ID_E enAIP;
    AOE_AIP_CHN_ATTR_NEW_S stAipAttr;
    AOE_AIP_CHN_ATTR_NEW_S stAipAttr2;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    AOE_AIP_CHN_ATTR_NEW_S stAipAttr3;
#endif	

    state->b_spdif_mod = pstAttr->b_spdif_mod;

    stAipAttr.u32StartVirAddr = (ulong)pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].startVirAddr;
    stAipAttr.u32StartPhyAddr = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].startPhyAddr;
    stAipAttr.u32Size = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_PCM].size;

    stAipAttr2.u32StartVirAddr = (ulong)pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW].startVirAddr;
    stAipAttr2.u32StartPhyAddr = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW].startPhyAddr;
    stAipAttr2.u32Size = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_SPDIF_RAW].size;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    stAipAttr3.u32StartVirAddr = (ulong)pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD].startVirAddr;
    stAipAttr3.u32StartPhyAddr = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD].startPhyAddr;
    stAipAttr3.u32Size = pCard->stTrackRbfMmz[SND_ENGINE_TYPE_MIXBUF_DD].size;
#endif

	MT_INFO_AO("spdif buf vir 0x%x phy 0x%x len 0x%x\n",
		stAipAttr2.u32StartVirAddr ,stAipAttr2.u32StartPhyAddr ,stAipAttr2.u32Size);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    iHAL_Init_Out_Buf_Reg(&stAipAttr, &stAipAttr2, &stAipAttr3, pstAttr->b_spdif_mod);
#else	
    iHAL_Init_Out_Buf_Reg(&stAipAttr, &stAipAttr2, pstAttr->b_spdif_mod);
#endif


    // dec channel  1
    if(state->b_spdif_mod){
        stAipAttr.CBType = AUDIO_BUF_TYPE_PCM;
        pCard->enUserHdmiMode = MT_UNF_SND_HDMI_MODE_RAW;
    }else{
        stAipAttr.CBType = AUDIO_BUF_TYPE_PP;
        pCard->enUserHdmiMode = MT_UNF_SND_HDMI_MODE_LPCM;
    }


    Ret = HAL_AOE_AIP_CreateNew(&enAIP, &stAipAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
        goto CREATE_PCM_ERR_EXIT;
    }

    state->enAIP[SND_ENGINE_TYPE_PCM] = enAIP;
    //state->stAipRbfMmz[SND_ENGINE_TYPE_PCM] = stRbfMmz;


    //dec channel 2   spdif
    stAipAttr2.CBType = AUDIO_BUF_TYPE_SPDIF;

    Ret = HAL_AOE_AIP_CreateNew(&enAIP, &stAipAttr2);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
        goto CREATE_PCM_ERR_EXIT;
    }

    state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW] = enAIP;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
    //dec channel 3   mix
    stAipAttr3.CBType = AUDIO_BUF_TYPE_MIX;

    Ret = HAL_AOE_AIP_CreateNew(&enAIP, &stAipAttr3);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AO("HAL_AOE_AIP_Create failed\n");
        goto CREATE_PCM_ERR_EXIT;
    }

    state->enAIP[SND_ENGINE_TYPE_MIXBUF_DD] = enAIP;
#endif

    return MT_SUCCESS;


CREATE_PCM_ERR_EXIT:
    return MT_FAILURE;
}


mt_s32 TrackCreateSlave(SND_TRACK_STATE_S *state, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                        MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf,
                        MT_UNF_SAMPLE_RATE_E enUserSampleRate)
{
    mt_s32 Ret;
    AOE_AIP_ID_E enAIP;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mmz_buffer_s stRbfMmz;
#ifdef AIAO_ALSA_DRV_V2
    if (1)
#else
    if (MT_FALSE == bAlsaTrack)
#endif
    {
        Ret = mt_drv_mmz_alloc_and_map("AO_SAipPcm", MMZ_OTHERS, AO_TRACK_PCM_BUFSIZE_BYTE_MAX, AIAO_BUFFER_ADDR_ALIGN,
                                     &stRbfMmz);
        if (MT_SUCCESS != Ret)
        {
            return MT_FAILURE;
        }

        TrackGetAipPcmDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
		stAipAttr.stFifoOutAttr.u32FifoSampleRate = enUserSampleRate;
        Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
        if (MT_SUCCESS != Ret)
        {
            mt_drv_mmz_unmap_and_release(&stRbfMmz);
            return MT_FAILURE;
        }

        state->enAIP[SND_ENGINE_TYPE_PCM] = enAIP;
        state->stAipRbfMmz[SND_ENGINE_TYPE_PCM] = stRbfMmz;
        state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_PCM] = MT_FALSE;
        state->enCurnStatus = SND_TRACK_STATUS_STOP;
    }
    else
    {
        stRbfMmz.startPhyAddr = pstBuf->u32BufPhyAddr;
        stRbfMmz.startVirAddr = (void *)pstBuf->u32BufVirAddr;
        stRbfMmz.size = pstBuf->u32BufSize;
#if  0
        TRP(stRbfMmz.u32StartPhyAddr);
        TRP(stRbfMmz.u32StartVirAddr);
        TRP(stRbfMmz.u32Size);
#endif
        TrackGetAipPcmDfAttr(&stAipAttr, &stRbfMmz, pstAttr);
        stAipAttr.stFifoOutAttr.u32FifoSampleRate = enUserSampleRate;
        stAipAttr.stBufInAttr.bAlsaEnable = MT_TRUE;
        Ret = HAL_AOE_AIP_Create(&enAIP, &stAipAttr);
        if (MT_SUCCESS != Ret)
        {
            return MT_FAILURE;
        }

        state->enAIP[SND_ENGINE_TYPE_PCM] = enAIP;
        state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_PCM] = MT_TRUE;
        state->enCurnStatus = SND_TRACK_STATUS_STOP;
    }

    return MT_SUCCESS;
}

mt_void SndOpBing2Engine(SND_CARD_STATE_S *pCard, mt_handle hEngine)
{
    mt_handle hSndOp;
    mt_u32 op;
    AOE_AOP_ID_E enAOP;
    SND_ENGINE_TYPE_E enEngineType = TrackGetEngineType(hEngine);
	SND_ENGINE_STATE_S *state = (SND_ENGINE_STATE_S *)hEngine;

    for (op = 0; op < MT_UNF_SND_OUTPUTPORT_MAX; op++)
    {
        if(pCard->hSndOp[op])
        {
            hSndOp = pCard->hSndOp[op];
            if (enEngineType == SND_GetOpGetOutType(hSndOp))
            {
                enAOP = SND_OpGetAopId(hSndOp);
				HAL_AOE_ENGINE_AttachAop(state->enEngine, enAOP);
            }
        }
    }
}

mt_void TrackSetMute(SND_CARD_STATE_S *pCard, SND_TRACK_STATE_S *pTrack, MT_BOOL bMute)
{
    HAL_AOE_AIP_SetMute(pTrack->enAIP[SND_ENGINE_TYPE_PCM], bMute);
    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
            HAL_AOE_AIP_SetMute(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], bMute);
        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
            HAL_AOE_AIP_SetMute(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], bMute);
    }
}

mt_void TrackBing2Engine(SND_CARD_STATE_S *pCard, mt_handle hSndTrack)
{
    mt_u32 u32AefId;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    AOE_ENGINE_CHN_ATTR_S stEngineAttr;
    mt_handle hEngine;
    SND_TRACK_STATE_S *state = (SND_TRACK_STATE_S *)hSndTrack;
    SND_ENGINE_STATE_S *pstEnginestate;

    if (pCard->hSndEngine[SND_ENGINE_TYPE_PCM])
    {
        hEngine = pCard->hSndEngine[SND_ENGINE_TYPE_PCM];
        pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;//verify
        HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_PCM]);
    }
    else
    {
        HAL_AOE_AIP_GetAttr(state->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);
        stEngineAttr.u32BitPerSample = stAipAttr.stFifoOutAttr.u32FifoBitPerSample;
        stEngineAttr.u32Channels   = stAipAttr.stFifoOutAttr.u32FifoChannels;
        stEngineAttr.u32SampleRate = stAipAttr.stFifoOutAttr.u32FifoSampleRate;
        stEngineAttr.u32DataFormat = stAipAttr.stFifoOutAttr.u32FifoDataFormat;

        TrackCreateEngine(&hEngine, &stEngineAttr, SND_ENGINE_TYPE_PCM);
        pCard->hSndEngine[SND_ENGINE_TYPE_PCM] = hEngine;
        pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;

        //because engine is created after creating track
        for(u32AefId = 0; u32AefId < AFLT_MAX_CHAN_NUM; u32AefId++)
        {
            if(pCard->u32AttAef & ((mt_u32)1L << u32AefId))
            {
                HAL_AOE_ENGINE_AttachAef(pstEnginestate->enEngine, u32AefId);
            }
        }

        HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_PCM]);
        HAL_AOE_ENGINE_Start(pstEnginestate->enEngine);
        SndOpBing2Engine(pCard, hEngine);
    }

    if (MT_UNF_SND_TRACK_TYPE_MASTER == state->stUserTrackAttr.enTrackType)
    {
        if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
        {
            if (pCard->hSndEngine[SND_ENGINE_TYPE_SPDIF_RAW])
            {
                hEngine = pCard->hSndEngine[SND_ENGINE_TYPE_SPDIF_RAW];
                pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;//verify
                HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
            }
            else
            {
                HAL_AOE_AIP_GetAttr(state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &stAipAttr);
                stEngineAttr.u32BitPerSample = stAipAttr.stFifoOutAttr.u32FifoBitPerSample;
                stEngineAttr.u32Channels   = stAipAttr.stFifoOutAttr.u32FifoChannels;
                stEngineAttr.u32SampleRate = stAipAttr.stFifoOutAttr.u32FifoSampleRate;
                stEngineAttr.u32DataFormat = stAipAttr.stFifoOutAttr.u32FifoDataFormat;
                TrackCreateEngine(&hEngine, &stEngineAttr, SND_ENGINE_TYPE_SPDIF_RAW);
                pCard->hSndEngine[SND_ENGINE_TYPE_SPDIF_RAW] = hEngine;
                pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;
                HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
                HAL_AOE_ENGINE_Start(pstEnginestate->enEngine);
                SndOpBing2Engine(pCard, hEngine);
            }
        }

        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
        {
            if (pCard->hSndEngine[SND_ENGINE_TYPE_HDMI_RAW])
            {
                hEngine = pCard->hSndEngine[SND_ENGINE_TYPE_HDMI_RAW];
                pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;
                HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            }
            else
            {
                HAL_AOE_AIP_GetAttr(state->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &stAipAttr);
                stEngineAttr.u32BitPerSample = stAipAttr.stFifoOutAttr.u32FifoBitPerSample;
                stEngineAttr.u32Channels   = stAipAttr.stFifoOutAttr.u32FifoChannels;
                stEngineAttr.u32SampleRate = stAipAttr.stFifoOutAttr.u32FifoSampleRate;
                stEngineAttr.u32DataFormat = stAipAttr.stFifoOutAttr.u32FifoDataFormat;
                TrackCreateEngine(&hEngine, &stEngineAttr, SND_ENGINE_TYPE_HDMI_RAW);
                pCard->hSndEngine[SND_ENGINE_TYPE_HDMI_RAW] = hEngine;
                pstEnginestate = (SND_ENGINE_STATE_S *)hEngine;
                HAL_AOE_ENGINE_AttachAip(pstEnginestate->enEngine, state->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
                HAL_AOE_ENGINE_Start(pstEnginestate->enEngine);
                SndOpBing2Engine(pCard, hEngine);
            }
        }
    }
}

/******************************AO Track FUNC*************************************/
mt_u32 TRACK_GetMasterId(SND_CARD_STATE_S *pCard)
{
    mt_u32 TrackId;
    SND_TRACK_STATE_S *state;

    for (TrackId = 0; TrackId < AO_MAX_TOTAL_TRACK_NUM; TrackId++)
    {
        if (pCard->uSndTrackInitFlag & ((mt_u32)1L << TrackId))
        {
            state = (SND_TRACK_STATE_S *)pCard->hSndTrack[TrackId];
            if (MT_UNF_SND_TRACK_TYPE_MASTER == state->stUserTrackAttr.enTrackType)
            {
                return TrackId;
            }
        }
    }

    return AO_MAX_TOTAL_TRACK_NUM;
}


mt_s32 TRACK_CreateNew(SND_CARD_STATE_S *pCard, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr,
                    MT_BOOL bAlsaTrack, AO_BUF_ATTR_S *pstBuf, mt_u32 TrackId)
{
    SND_TRACK_STATE_S *state = MT_NULL;
    mt_s32 Ret = MT_FAILURE;
    mt_u32 TrackId2;
	mmz_buffer_s debugcrc_mmz;

    if(pstAttr->enTrackType >= MT_UNF_SND_TRACK_TYPE_BUTT)
    {
        MT_ERR_AO("dont support tracktype(%d)\n",pstAttr->enTrackType);
        return MT_FAILURE;
    }

    if(pstAttr->u32BufLevelMs < AO_TRACK_MASTER_MIN_BUFLEVELMS || pstAttr->u32BufLevelMs > AO_TRACK_MASTER_MAX_BUFLEVELMS)
    {
        MT_ERR_AO("Invalid u32BufLevelMs(%d), Min(%d), Max(%d)\n",pstAttr->u32BufLevelMs,
            AO_TRACK_MASTER_MIN_BUFLEVELMS, AO_TRACK_MASTER_MAX_BUFLEVELMS);
        return MT_FAILURE;
    }

    if(TrackId)
    {
      MT_INFO_AO("\n now create track slave %d \n\n", TrackId);
      TrackId2 = 0;
    }

    else
      TrackId2 = 1;


    if(pCard->hSndTrack[TrackId2])// already created
    {
      MT_INFO_AO("\n track %d exists ,so track %d do noting  addr is 0x%x \n\n", TrackId2, TrackId,pCard->hSndTrack[TrackId2] );
      pCard->hSndTrack[TrackId] = pCard->hSndTrack[TrackId2];
      return MT_SUCCESS;
    }


    state = AUTIL_AO_MALLOC(MT_ID_AO, sizeof(SND_TRACK_STATE_S), GFP_KERNEL);
    if (state == MT_NULL)
    {
        MT_FATAL_AO("malloc TRACK_Create failed\n");
        goto SndTrackCreate_ERR_EXIT;
    }

    memset(state, 0, sizeof(SND_TRACK_STATE_S));

     if (MT_SUCCESS != TrackCreateMasterNew(pCard, state, pstAttr))
    //if (MT_SUCCESS != TrackCreateMaster(pCard, state, pstAttr))
        {
            goto SndTrackCreate_ERR_EXIT;
        }

    state->TrackId = TrackId;
    memcpy(&state->stUserTrackAttr, pstAttr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));

	if(state->stUserTrackAttr.u32DebugCrcSize){
		debugcrc_mmz.size = state->stUserTrackAttr.u32DebugCrcSize;
		Ret = mt_drv_mmz_alloc_and_map("AO_SAipPcm", MMZ_OTHERS, state->stUserTrackAttr.u32DebugCrcSize, 0, &debugcrc_mmz);
		if (MT_SUCCESS == Ret) {
			state->stUserTrackAttr.u32DebugCrc_phy = debugcrc_mmz.startPhyAddr;
			state->stUserTrackAttr.u32DebugCrc_vir = (ulong)debugcrc_mmz.startVirAddr;
		}else{
			state->stUserTrackAttr.u32DebugCrc_phy = 0;
			state->stUserTrackAttr.u32DebugCrc_vir = 0;
		}
	}

    state->stTrackAbsGain.bLinearMode = MT_TRUE;
    state->stTrackAbsGain.s32GainL = AO_MAX_LINEARVOLUME;
    state->stTrackAbsGain.s32GainR = AO_MAX_LINEARVOLUME;
    state->bMute = MT_FALSE;
	state->enChannelMode= MT_UNF_TRACK_MODE_STEREO;
    state->u32SendTryCnt = 0;
    state->u32PcmSampleRate = 0;
    state->u32SendCnt = 0;
    state->u32AddMuteFrameNum = 0;
    state->bEosFlag = MT_FALSE;
    state->bAlsaTrack = bAlsaTrack;
    state->enSaveState = SND_DEBUG_CMD_CTRL_STOP;
    state->u32SaveCnt = 0;
    state->fileHandle = MT_NULL;
    pCard->hSndTrack[TrackId] = (mt_handle)state;

    //avsync_audio_init(3);
    //MT_INFO_AO("\n  creat track %d suc  addr is 0x%x \n\n", TrackId,  pCard->hSndTrack[TrackId]);

    //hdmi
    {
        HDMI_AUDIO_ATTR_S stHDMIAttr;

          if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiGetAoAttr)
          {
              (pCard->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
          }


          if(pstAttr->b_spdif_mod)//
          {
            stHDMIAttr.enSoundIntf  = HDMI_AUDIO_INTERFACE_SPDIF;
            MT_INFO_AO("set hdmi to SPDIFmode\n");
          }
          else
          {
            stHDMIAttr.enSoundIntf  = HDMI_AUDIO_INTERFACE_I2S;
            MT_INFO_AO("set hdmi to I2Smode\n");
          }
          stHDMIAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
          /*get the capability of the max pcm channels of the output device*/
          if(pCard->pstHdmiFunc && pCard->pstHdmiFunc->pfnHdmiAudioChange)
          {
              (pCard->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0,&stHDMIAttr);
              MT_INFO_AO("pfnHdmiAudioChange be called\n");
          }
    }

    pCard->uSndTrackInitFlag |= ((mt_u32)1L << (state->TrackId));

    return MT_SUCCESS;

SndTrackCreate_ERR_EXIT:
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return Ret;
}


mt_s32 TRACK_Destroy(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *state;
    //SND_ENGINE_STATE_S *pstEngineState;
    mt_u32 u32TrackID2;
	mmz_buffer_s debugcrc_mmz;

    state = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (MT_NULL == state)
    {
        return MT_FAILURE;
    }

    if(u32TrackID)
      u32TrackID2 = 0;
    else
      u32TrackID2 = 1;

    if(pCard->hSndTrack[u32TrackID2]) //other track exists,do not destroy
    {
      MT_INFO_AO("\n track destroy   if %d ex   so  %d be null\n", u32TrackID2, u32TrackID);
      pCard->hSndTrack[u32TrackID] = MT_NULL;
      return MT_SUCCESS;
    }


    HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_PCM]);
    HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_MIXBUF_DD]);
#endif	
    pCard->uSndTrackInitFlag &= ~((mt_u32)1L << state->TrackId);
    pCard->hSndTrack[u32TrackID] = MT_NULL;

	if(state->stUserTrackAttr.u32DebugCrc_vir){
		debugcrc_mmz.size = state->stUserTrackAttr.u32DebugCrcSize;
		debugcrc_mmz.startPhyAddr = (phys_addr_t)state->stUserTrackAttr.u32DebugCrc_phy;
		debugcrc_mmz.startVirAddr = (void *)state->stUserTrackAttr.u32DebugCrc_vir;
		mt_drv_mmz_unmap_and_release(&debugcrc_mmz);
	}

    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return MT_SUCCESS;
}

#if 0
mt_s32 TRACK_Destroy2(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *state;
    SND_ENGINE_STATE_S *pstEngineState;

    state = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (MT_NULL == state)
    {
        return MT_FAILURE;
    }

    switch (state->stUserTrackAttr.enTrackType)
    {
    case MT_UNF_SND_TRACK_TYPE_MASTER:
        pstEngineState = (SND_ENGINE_STATE_S *)TrackGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_PCM);
        HAL_AOE_ENGINE_DetachAip(pstEngineState->enEngine, state->enAIP[SND_ENGINE_TYPE_PCM]);
        HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_PCM]);

        if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
        {
            pstEngineState = (SND_ENGINE_STATE_S *)TrackGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_SPDIF_RAW);
            HAL_AOE_ENGINE_DetachAip(pstEngineState->enEngine, state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
            HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
        }

        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
        {
            pstEngineState = (SND_ENGINE_STATE_S *)TrackGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_HDMI_RAW);
            HAL_AOE_ENGINE_DetachAip(pstEngineState->enEngine, state->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
        }
        break;

    case MT_UNF_SND_TRACK_TYPE_SLAVE:
        pstEngineState = (SND_ENGINE_STATE_S *)TrackGetEngineHandlebyType(pCard, SND_ENGINE_TYPE_PCM);
        HAL_AOE_ENGINE_DetachAip(pstEngineState->enEngine, state->enAIP[SND_ENGINE_TYPE_PCM]);
        HAL_AOE_AIP_Destroy(state->enAIP[SND_ENGINE_TYPE_PCM]);
        if (MT_TRUE != state->bAipRbfExtDmaMem[SND_ENGINE_TYPE_PCM])
        {
            mt_drv_mmz_unmap_and_release(&state->stAipRbfMmz[SND_ENGINE_TYPE_PCM]);
        }

        break;

    default:
        break;
    }

    pCard->uSndTrackInitFlag &= ~((mt_u32)1L << state->TrackId);
    pCard->hSndTrack[state->TrackId] = MT_NULL;
    AUTIL_AO_FREE(MT_ID_AO, (mt_void*)state);
    return MT_SUCCESS;
}


mt_s32 TRACK_SendData_old(SND_CARD_STATE_S *pCard,mt_u32 u32TrackID, MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    SND_TRACK_STATE_S *pTrack;
    SND_TRACK_STREAM_ATTR_S stStreamAttr;
    STREAMMODE_CHANGE_ATTR_S stChange;
    AOE_AIP_STATUS_E eAipStatus = AOE_AIP_STATUS_STOP;

    if (MT_NULL == pstAOFrame)
    {
        return MT_FAILURE;
    }

    //todo , check attr

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    //Ao track state check
    if(SND_TRACK_STATUS_BUTT <= pTrack->enCurnStatus)
    {
        MT_ERR_AO("Invalid ao track status\n");
        return MT_FAILURE;
    }
    if (SND_TRACK_STATUS_PAUSE == pTrack->enCurnStatus)
    {
        return MT_ERR_AO_PAUSE_STATE;
    }
    if (SND_TRACK_STATUS_STOP == pTrack->enCurnStatus)
    {
        MT_ERR_AO("Ao track stop status, can't send data\n");
        return MT_FAILURE;
    }

    //if it is invaild samplerate, discard audio frame and return MT_SUCCESS(avoid printing).
    CHECK_AO_FRAME_NOSTANDART_SAMPLERATE(pstAOFrame->u32SampleRate);
	//CHECK_AO_FRAME_BITDEPTH(pstAOFrame->s32BitPerSample);
    TRACKDbgCountTrySendData(pTrack);

    //TRACKBuildStreamAttr(pCard, (MT_UNF_AO_FRAMEINFO_S *)pstAOFrame, &stStreamAttr);

    stStreamAttr.u32PcmSampleRate = pstAOFrame->u32SampleRate;
    stStreamAttr.u32PcmBytesPerFrame = pstAOFrame->u32PcmSamplesPerFrame * 4;
    stStreamAttr.pPcmDataBuf = pstAOFrame->ps32PcmBuffer;
    stStreamAttr.u32PcmSamplesPerFrame = pstAOFrame->u32PcmSamplesPerFrame;

    #if 0
    MT_INFO_AO("\n\n\n\n\n\n\n\n\n\n\n\n");

    MT_INFO_AO(" u32Channels %d \n", pstAOFrame->u32Channels);
    MT_INFO_AO(" s32BitPerSample %d\n", pstAOFrame->s32BitPerSample);
    MT_INFO_AO(" u32SampleRate %d\n", pstAOFrame->u32SampleRate);
    MT_INFO_AO(" u32IEC61937DataType %d\n", pstAOFrame->u32IEC61937DataType);
    MT_INFO_AO(" u32BitsBytesPerFrame %d\n", pstAOFrame->u32BitsBytesPerFrame);
    MT_INFO_AO(" u32PcmSamplesPerFrame %d\n", pstAOFrame->u32PcmSamplesPerFrame);

    MT_INFO_AO("=====================================================\n");

    MT_INFO_AO(" u32PcmBitDepth %d \n", stStreamAttr.u32PcmBitDepth);
    MT_INFO_AO(" u32PcmChannels %d \n", stStreamAttr.u32PcmChannels);
    MT_INFO_AO(" u32PcmSamplesPerFrame %d \n", stStreamAttr.u32PcmSamplesPerFrame);
    MT_INFO_AO(" u32PcmSampleRate %d \n", stStreamAttr.u32PcmSampleRate);
    MT_INFO_AO(" u32PcmBytesPerFrame %d \n", stStreamAttr.u32PcmBytesPerFrame);
    //printf(" u32PcmBitDepth %d \n", stStreamAttr.u32p);



    MT_INFO_AO("\n\n\n\n\n\n\n\n\n\n\n\n");
    #endif

    #if 0
    DetectStreamModeChange(pCard, pTrack, &stStreamAttr, &stChange);

    if(MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if (stChange.enPcmChange || stChange.enSpdifChange || stChange.enHdmiChnage)
        {
            HAL_AOE_AIP_GetStatus(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &eAipStatus); //master don't immediately start, so get start/stop status firstly
            if(AOE_AIP_STATUS_START == eAipStatus || AOE_AIP_STATUS_PAUSE == eAipStatus)
            {
                HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
                if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
                    HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
                if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
                    HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            }

            SndProcPcmRoute(pCard, pTrack, stChange.enPcmChange, &stStreamAttr);
            if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
                SndProcSpidfRoute(pCard, pTrack, stChange.enSpdifChange, &stStreamAttr);
            if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
            {
                if((SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough) || (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough))
                {
                    HDMISetAudioMute(pCard);
                }
                SndProcHdmifRoute(pCard, pTrack, stChange.enHdmiChnage, &stStreamAttr);
            }
            if(AOE_AIP_STATUS_START == eAipStatus)
            {
                HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
                if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
                    HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
                if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
                    HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            }
        }
        DetectTrueHDModeChange(pCard,&stStreamAttr);

    }
    else if (MT_UNF_SND_TRACK_TYPE_SLAVE == pTrack->stUserTrackAttr.enTrackType)
    {
        if (stChange.enPcmChange)
        {
            HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);   //slave immediately start, so we don't need get start/stop status
            SndProcPcmRoute(pCard, pTrack, stChange.enPcmChange, &stStreamAttr);
            HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
        }
    }
    else
    {
        //verify virtual
    }
    #endif

    if (MT_FALSE == TrackisBufFree(pCard, pTrack, &stStreamAttr))
    {
        TRACKStartAip(pCard, pTrack);
        return MT_ERR_AO_OUT_BUF_FULL;
    }

    TRACKWriteFrame(pCard, pTrack, &stStreamAttr);

    if(AO_SND_SPEEDADJUST_SRC == pTrack->enUserSpeedType && 0 > pTrack->s32UserSpeedRate)
    {
        TRACKWriteMuteFrame(pCard, pTrack, &stStreamAttr);
    }

    //avsync_push_audio_pts(pstAOFrame->u64PtsMs, 1, pstAOFrame->u32FrameIndex, 1024*1000);

    TRACKStartAip(pCard, pTrack);

    TRACKDbgCountSendData(pTrack);

    return MT_SUCCESS;
}
#endif

static mt_void TRACK_Dump(ulong src, mt_s32 size)
{
    mt_s32 res = 0;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    src = (ulong)__va(src & 0x1fffffff);
    mt_dcache_invalid((void*)src, size);
#endif

    res = mt_drv_dump_do(g_pcm_dump_handle, (const mt_u8 *)src, size);
    if(res != size)
    {
        /* end of dump */
        mt_drv_dump_destroy(g_pcm_dump_handle);
        g_pcm_dump_handle = NULL;
    }
}

mt_s32 TRACK_SendData(SND_CARD_STATE_S *pCard,mt_u32 u32TrackID, MT_UNF_AO_FRAMEINFO_S * pstAOFrame)
{
    SND_TRACK_STATE_S *pTrack;
    SND_TRACK_STREAM_ATTR_S stStreamAttr;
    //STREAMMODE_CHANGE_ATTR_S stChange;
    //AOE_AIP_STATUS_E eAipStatus = AOE_AIP_STATUS_STOP;

    //todo , check attr

    if(1 == u32TrackID)// slave, check if master exists.
    {
      if(pCard->hSndTrack[0])// if exists, do not out put
        return MT_SUCCESS;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if(NULL == pTrack){
        return MT_FAILURE;
    }

    //TRACKBuildStreamAttr(pCard, (MT_UNF_AO_FRAMEINFO_S *)pstAOFrame, &stStreamAttr);

    stStreamAttr.u32ChannelExist = pstAOFrame->u32ChannelsExist;

    stStreamAttr.u32PcmSampleRate = pstAOFrame->u32SampleRate;
    stStreamAttr.u32PcmBytesPerFrame = pstAOFrame->u32PcmSamplesPerFrame * 4;
    stStreamAttr.pPcmDataBuf = pstAOFrame->ps32PcmBuffer;
    stStreamAttr.u32PcmSamplesPerFrame = pstAOFrame->u32PcmSamplesPerFrame;
    stStreamAttr.bEac4TimeSampleRate = pstAOFrame->bEac4TimeSampleRate;
    stStreamAttr.u32chan = pstAOFrame->u32chan;
	if((SND_ENGINE_TYPE_PCM == stStreamAttr.u32chan) && pTrack->stUserTrackAttr.u32DebugCrc_vir != 0 &&
		pstAOFrame->u32FrameIndex <= (pTrack->stUserTrackAttr.u32DebugCrcSize >> 2)){	
//		pTrack->stUserTrackAttr.u32DebugCrc_vir[pstAOFrame->u32FrameIndex-1] = pstAOFrame->u32DebugCrc;
	}

    if (MT_FALSE == TrackisBufFree(pCard, pTrack, &stStreamAttr))
    {
        //TRACKStartAip(pCard, pTrack);
        return MT_ERR_AO_OUT_BUF_FULL;
    }

    TRACKWriteFrame(pCard, pTrack, &stStreamAttr,pstAOFrame->adectype);    

    if(SND_ENGINE_TYPE_PCM == stStreamAttr.u32chan) {
        TRACK_Dump((ulong)stStreamAttr.pPcmDataBuf,stStreamAttr.u32PcmBytesPerFrame);
    }

    if(AO_SND_SPEEDADJUST_SRC == pTrack->enUserSpeedType && 0 > pTrack->s32UserSpeedRate)
    {
        TRACKWriteMuteFrame(pCard, pTrack, &stStreamAttr);
    }

    //avsync_push_audio_pts(pstAOFrame->u64PtsMs, 1, pstAOFrame->u32FrameIndex, 1024*1000);


    TRACKDbgCountSendData(pTrack);

    return MT_SUCCESS;
}

#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
#define MREAD(A) (*((volatile unsigned int *)(A)))
#define MWRITE(A, V) (*((volatile unsigned int *)(A)) = (V))
#endif
mt_s32 TRACK_Start(SND_CARD_STATE_S *pCard,mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *pTrack;



    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (MT_NULL == pTrack)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (SND_TRACK_STATUS_START == pTrack->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    pTrack->enCurnStatus = SND_TRACK_STATUS_START;
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
#ifdef CONFIG_MT_CHIP_SYMPHONY4
	void __iomem * reg_dcH = ioremap(0xbf4900dc, 4);
	 MWRITE(reg_dcH, (~(1<<0)) & MREAD(reg_dcH));
#else
	 MWRITE(0xbf4900dc, (~(1<<0)) & MREAD(0xbf4900dc));
#endif   

#endif
    //avsync_audio_init(3);
    pTrack->u32PcmSampleRate=0;   //reset samplingrate aout   for avplay_start\switch pg
    pTrack->u32PcmSampleRate2=0;  //reset samplingrate spdif  for avplay_start\switch pg

    TRACKStartAip(pCard, pTrack);

    return MT_SUCCESS;
}

mt_s32 TRACK_Stop(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *pTrack;



    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (MT_NULL == pTrack)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (SND_TRACK_STATUS_STOP == pTrack->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    if(MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        mt_u32 u32AIPStopMask = 0;
        AOE_AIP_STATUS_E curstatus;

        HAL_AOE_AIP_GetStatus(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &curstatus);
        if(curstatus != AOE_AIP_STATUS_STOP)
        {
            u32AIPStopMask |= (1 << (mt_u32)pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
        }

        if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
        {
            HAL_AOE_AIP_GetStatus(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &curstatus);
            if(curstatus != AOE_AIP_STATUS_STOP)
            {
                u32AIPStopMask |= (1 << (mt_u32)pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
            }
        }
        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
        {
            HAL_AOE_AIP_GetStatus(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &curstatus);
            if(curstatus != AOE_AIP_STATUS_STOP)
            {
                u32AIPStopMask |= (1 << (mt_u32)pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
            }
        }

        //the interface is only used for master track type to stop
        if(u32AIPStopMask != 0)
        {
            HAL_AOE_AIP_Group_Stop(u32AIPStopMask);
        }

        if (SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
        {
            SndProcSpidfRoute(pCard, pTrack, TRACK_STREAMMODE_CHANGE_LBR2PCM, &pTrack->stStreamAttr);
        }

        if (SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough)
        {
            HDMISetAudioMute(pCard);
            SndProcHdmifRoute(pCard, pTrack, TRACK_STREAMMODE_CHANGE_LBR2PCM, &pTrack->stStreamAttr);
        }
        else if (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
        {
            HDMISetAudioMute(pCard);
            SndProcHdmifRoute(pCard, pTrack, TRACK_STREAMMODE_CHANGE_HBR2PCM, &pTrack->stStreamAttr);
        }
    }
    else
    {
        HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
    }

    pTrack->enCurnStatus = SND_TRACK_STATUS_STOP;

    return MT_SUCCESS;
}

mt_s32 TRACK_Pause(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *pTrack;



    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (MT_NULL == pTrack)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if (SND_TRACK_STATUS_PAUSE == pTrack->enCurnStatus)
    {
        return MT_SUCCESS;
    }

    HAL_AOE_AIP_Pause(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if(SND_SPDIF_MODE_NONE != pCard->enSpdifPassthrough)
            HAL_AOE_AIP_Pause(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW]);
        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough)
        {
            if((SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough) || (SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough))
            {
                HDMISetAudioMute(pCard);
            }
            HAL_AOE_AIP_Pause(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW]);
        }
    }

    pTrack->enCurnStatus = SND_TRACK_STATUS_PAUSE;
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
    
#ifdef CONFIG_MT_CHIP_SYMPHONY4
	void __iomem * reg_dcH = ioremap(0xbf4900dc, 4);
	MWRITE(reg_dcH, 0x1 | MREAD(reg_dcH));
#else
	 MWRITE(0xbf4900dc, 0x1 | MREAD(0xbf4900dc));
#endif   


#endif
    return MT_SUCCESS;
}

mt_s32 TRACK_Flush(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    SND_TRACK_STATE_S *pTrack;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    
    HAL_AOE_AIP_Flush(SND_ENGINE_TYPE_PCM);

    if (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType)
    {
        if(SND_HDMI_MODE_NONE != pCard->enHdmiPassthrough) {
            HAL_AOE_AIP_Flush(SND_ENGINE_TYPE_SPDIF_RAW);
            HAL_AOE_AIP_Flush(SND_ENGINE_TYPE_HDMI_RAW);
        }
    }
    
    return MT_SUCCESS;
}

mt_s32 TRACK_DetectAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr, SND_TRACK_ATTR_SETTING_E *penAttrSetting)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pstTrackAttr)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];


    //check input parameter
    if(pstTrackAttr->u32BufLevelMs < AO_TRACK_MASTER_MIN_BUFLEVELMS || pstTrackAttr->u32BufLevelMs > AO_TRACK_MASTER_MAX_BUFLEVELMS)
    {
        MT_ERR_AO("SND_TRACK_ATTR_SETTING_E u32BufLevelMs Bad parameter !\n");
        return MT_ERR_AO_INVALID_PARA;
    }

    if (pTrack->stUserTrackAttr.enTrackType == pstTrackAttr->enTrackType)
    {
        if(pTrack->stUserTrackAttr.u32BufLevelMs != pstTrackAttr->u32BufLevelMs  ||
            pTrack->stUserTrackAttr.u32FadeinMs != pstTrackAttr->u32FadeinMs ||
            pTrack->stUserTrackAttr.u32FadeoutMs != pstTrackAttr->u32FadeoutMs ||
            pTrack->stUserTrackAttr.u32OutputBufSize != pstTrackAttr->u32OutputBufSize)
        {
            //TODO   not support parameter  u32FadeinMs   u32FadeoutMs  u32OutputBufSize
            *penAttrSetting = SND_TRACK_ATTR_MODIFY;
        }
        else
		{
            *penAttrSetting = SND_TRACK_ATTR_RETAIN;
		}
    }
    else if(MT_UNF_SND_TRACK_TYPE_MASTER == pstTrackAttr->enTrackType)
    {
        *penAttrSetting = SND_TRACK_ATTR_SLAVE2MASTER;
    }
    else if(MT_UNF_SND_TRACK_TYPE_SLAVE == pstTrackAttr->enTrackType)
    {
        *penAttrSetting = SND_TRACK_ATTR_MASTER2SLAVE;
    }
    else
    {
        MT_ERR_AO("virttrack Not support \n");
        return MT_ERR_AO_INVALID_PARA;
    }

    return MT_SUCCESS;
}


mt_s32 TRACK_CheckAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{

    if (MT_NULL == pstTrackAttr)
    {
        return MT_FAILURE;
    }

    if(pstTrackAttr->enTrackType >= MT_UNF_SND_TRACK_TYPE_BUTT)
    {
        MT_ERR_AO("dont support tracktype(%d)\n",pstTrackAttr->enTrackType);
        return MT_FAILURE;
    }

    if(pstTrackAttr->u32BufLevelMs < AO_TRACK_MASTER_MIN_BUFLEVELMS || pstTrackAttr->u32BufLevelMs > AO_TRACK_MASTER_MAX_BUFLEVELMS)
    {
        MT_ERR_AO("Invalid u32BufLevelMs(%d), Min(%d), Max(%d)\n",pstTrackAttr->u32BufLevelMs,
            AO_TRACK_MASTER_MIN_BUFLEVELMS, AO_TRACK_MASTER_MAX_BUFLEVELMS);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 TRACK_SetAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pstTrackAttr)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    pTrack->stUserTrackAttr.u32BufLevelMs = pstTrackAttr->u32BufLevelMs;

    pTrack->stUserTrackAttr.u32FadeinMs = pstTrackAttr->u32FadeinMs;
    pTrack->stUserTrackAttr.u32FadeoutMs = pstTrackAttr->u32FadeoutMs;
    pTrack->stUserTrackAttr.u32OutputBufSize = pstTrackAttr->u32OutputBufSize;

    return MT_SUCCESS;
}


mt_s32 TRACK_SetWeight(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S *pstTrackGain)
{
    SND_TRACK_STATE_S *pTrack;
    mt_u32 u32dBReg;

    if (MT_NULL == pstTrackGain)
    {
        return MT_FAILURE;
    }

    if(MT_TRUE == pstTrackGain->bLinearMode)
    {
        CHECK_AO_LINEARVOLUME(pstTrackGain->s32Gain);
        u32dBReg = AUTIL_VolumeLinear2RegdB((mt_u32)pstTrackGain->s32Gain);
    }
    else
    {
        CHECK_AO_ABSLUTEVOLUME(pstTrackGain->s32Gain);
        u32dBReg = AUTIL_VolumedB2RegdB(pstTrackGain->s32Gain);
    }


    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    HAL_AOE_AIP_SetLRVolume(pTrack->enAIP[SND_ENGINE_TYPE_PCM], u32dBReg, u32dBReg);
    pTrack->stTrackAbsGain.bLinearMode = pstTrackGain->bLinearMode;
    pTrack->stTrackAbsGain.s32GainL = pstTrackGain->s32Gain;
    pTrack->stTrackAbsGain.s32GainR = pstTrackGain->s32Gain;

    return MT_SUCCESS;
}

mt_s32 TRACK_GetWeight(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_GAIN_ATTR_S *pstTrackGain)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pstTrackGain)
    {
        return MT_FAILURE;
    }



    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    pstTrackGain->bLinearMode = pTrack->stTrackAbsGain.bLinearMode;
    pstTrackGain->s32Gain = pTrack->stTrackAbsGain.s32GainL;    //Just give L Gain

    return MT_SUCCESS;
}

mt_s32 TRACK_SetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain)
{
    SND_TRACK_STATE_S *pTrack;
    mt_u32 u32dBLReg;
    mt_u32 u32dBRReg;

    if (MT_NULL == pstTrackAbsGain)
    {
        return MT_FAILURE;
    }
    if(MT_TRUE == pstTrackAbsGain->bLinearMode)
    {
        CHECK_AO_LINEARVOLUME(pstTrackAbsGain->s32GainL);
        CHECK_AO_LINEARVOLUME(pstTrackAbsGain->s32GainR);

        u32dBLReg = AUTIL_VolumeLinear2RegdB((mt_u32)pstTrackAbsGain->s32GainL);
        u32dBRReg = AUTIL_VolumeLinear2RegdB((mt_u32)pstTrackAbsGain->s32GainR);
    }
    else
    {
        CHECK_AO_ABSLUTEVOLUMEEXT(pstTrackAbsGain->s32GainL);
        CHECK_AO_ABSLUTEVOLUMEEXT(pstTrackAbsGain->s32GainR);

        u32dBLReg = AUTIL_VolumedB2RegdB(pstTrackAbsGain->s32GainL);
        u32dBRReg = AUTIL_VolumedB2RegdB(pstTrackAbsGain->s32GainR);
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    HAL_AOE_AIP_SetLRVolume(pTrack->enAIP[SND_ENGINE_TYPE_PCM], u32dBLReg, u32dBRReg);

    pTrack->stTrackAbsGain.bLinearMode = pstTrackAbsGain->bLinearMode;
    pTrack->stTrackAbsGain.s32GainL = pstTrackAbsGain->s32GainL;
    pTrack->stTrackAbsGain.s32GainR = pstTrackAbsGain->s32GainR;

    return MT_SUCCESS;
}

mt_s32 TRACK_GetAbsGain(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_SND_ABSGAIN_ATTR_S *pstTrackAbsGain)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pstTrackAbsGain)
    {
        return MT_FAILURE;
    }
    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    pstTrackAbsGain->bLinearMode = pTrack->stTrackAbsGain.bLinearMode;
    pstTrackAbsGain->s32GainL = pTrack->stTrackAbsGain.s32GainL;
    pstTrackAbsGain->s32GainR = pTrack->stTrackAbsGain.s32GainR;

    return MT_SUCCESS;
}

mt_s32 TRACK_SetMute(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL bMute)
{
    SND_TRACK_STATE_S *pTrack;
    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    //TODO
    if(MT_TRUE == pTrack->bAlsaTrack)
    {
        MT_WARN_AO("Alsa track don't support mute function!\n");
        return MT_FAILURE;
    }

    if(MT_FALSE == pCard->bAllTrackMute && MT_FALSE == bMute)
    {
        MT_INFO_AO("Track Set unMute Id %d\n",u32TrackID);
        TrackSetMute(pCard, pTrack, MT_FALSE);
    }
    else
    {
        MT_INFO_AO("Track Set Mute Id %d\n",u32TrackID);
        TrackSetMute(pCard, pTrack, MT_TRUE);
    }

    pTrack->bMute = bMute;

    return MT_SUCCESS;
}
mt_s32 TRACK_GetMute(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL *pbMute)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pbMute)
    {
        return MT_FAILURE;
    }
    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    //TODO
    if(MT_TRUE == pTrack->bAlsaTrack)
    {
        MT_ERR_AO("Alsa track don't support mute function!\n");
        return MT_FAILURE;
    }

    *pbMute = pTrack->bMute;
     MT_INFO_AO("Track Id %d get Mute Staues %d\n",u32TrackID,*pbMute);
    return MT_SUCCESS;
}

mt_s32 TRACK_SetAllMute(SND_CARD_STATE_S *pCard, MT_BOOL bMute)
{
    mt_u32 i;
    SND_TRACK_STATE_S *pTrack;

    for(i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
    {
        if(pCard->hSndTrack[i])
        {
            pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[i];
            if(MT_TRUE == pTrack->bAlsaTrack)
            {
                MT_WARN_AO("Alsa track don't support mute function!\n");
                continue;
            }

            if(MT_FALSE == pTrack->bMute && MT_FALSE == bMute)
            {
                MT_INFO_AO("Set Track All unmute Id %d\n",i);
                TrackSetMute(pCard, pTrack, MT_FALSE);
            }
            else
            {
                MT_INFO_AO("Set Track All mute Id %d\n",i);
                TrackSetMute(pCard, pTrack, MT_TRUE);
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 TRACK_SetChannelMode(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E *penMode)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == penMode || MT_UNF_TRACK_MODE_BUTT == *penMode)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    HAL_AOE_AIP_SetChannelMode(pTrack->enAIP[SND_ENGINE_TYPE_PCM], *(mt_u32 *)penMode);

    pTrack->enChannelMode = *penMode;

    return MT_SUCCESS;
}


mt_s32 TRACK_GetChannelMode(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_TRACK_MODE_E *penMode)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == penMode)
    {
        return MT_FAILURE;
    }
    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

	*penMode = pTrack->enChannelMode;

    return MT_SUCCESS;
}

mt_s32 TRACK_SetSpeedAdjust(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, AO_SND_SPEEDADJUST_TYPE_E enType, mt_s32 s32Speed)
{
    SND_TRACK_STATE_S *pTrack;

    CHECK_AO_SPEEDADJUST(s32Speed);

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    if(AO_SND_SPEEDADJUST_SRC == enType)
    {
        HAL_AOE_AIP_SetSpeed(pTrack->enAIP[SND_ENGINE_TYPE_PCM], s32Speed); //verify no pcm need speedadjust?
    }
    else if(AO_SND_SPEEDADJUST_MUTE == enType)
    {
        //verify  avplay not use
    }
    pTrack->enUserSpeedType  = enType;
    pTrack->s32UserSpeedRate = s32Speed;

    return MT_SUCCESS;
}
#if defined (MT_AUDIO_AI_SUPPORT)

mt_s32 TRACK_SetPcmAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_handle hAi)
{
    mt_s32 Ret;
    SND_TRACK_STATE_S *pTrack;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    AIAO_RBUF_ATTR_S stAiaoBuf;
    AIAO_PORT_ATTR_S stPortAttr;

    Ret = AI_GetPortAttr(hAi, &stPortAttr);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AO("call AI_GetPortAttr failed!\n");
        return Ret;
    }

    memset(&stAiaoBuf, 0, sizeof(AIAO_RBUF_ATTR_S));
    Ret = AI_GetPortBuf(hAi,&stAiaoBuf);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AO("call AI_GetPortBuf failed!\n");
        return Ret;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    HAL_AOE_AIP_GetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr );

    stAipAttr.stBufInAttr.u32BufSampleRate = (mt_u32)(stPortAttr.stIfAttr.enRate);
    stAipAttr.stBufInAttr.u32BufBitPerSample = (mt_u32)(stPortAttr.stIfAttr.enBitDepth);
    stAipAttr.stBufInAttr.u32BufChannels = (mt_u32)(stPortAttr.stIfAttr.enChNum);

    stAipAttr.stBufInAttr.stRbfAttr.u32BufPhyAddr = stAiaoBuf.u32BufPhyAddr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufPhyRptr = stAiaoBuf.u32BufPhyRptr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufPhyWptr = stAiaoBuf.u32BufPhyWptr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufVirAddr = stAiaoBuf.u32BufVirAddr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufVirRptr = stAiaoBuf.u32BufVirRptr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufVirWptr = stAiaoBuf.u32BufVirWptr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufSize = stAiaoBuf.u32BufSize;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufWptrRptrFlag = 1;
    stAipAttr.stBufInAttr.bMixPriority = MT_TRUE;
    HAL_AOE_AIP_SetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);

    return MT_SUCCESS;
}


mt_s32 TRACK_AttachAi(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_handle hAi)
{
    SND_TRACK_STATE_S *pTrack;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    pTrack->bAttAi = MT_TRUE;
    pTrack->hAi = hAi;
    return MT_SUCCESS;
}

mt_s32 TRACK_DetachAi(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)
{
    mt_s32 Ret;
    SND_TRACK_STATE_S *pTrack;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mmz_buffer_s stMmzBuf;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    stMmzBuf = pTrack->stAipRbfMmz[SND_ENGINE_TYPE_PCM];

    Ret = HAL_AOE_AIP_GetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);
    stAipAttr.stBufInAttr.stRbfAttr.u32BufPhyAddr = stMmzBuf.startPhyAddr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufVirAddr = (ulong)stMmzBuf.startVirAddr;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufSize = stMmzBuf.size;
    stAipAttr.stBufInAttr.stRbfAttr.u32BufWptrRptrFlag = 0;
    stAipAttr.stBufInAttr.bMixPriority = MT_FALSE;

    HAL_AOE_AIP_Stop(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);
    Ret = HAL_AOE_AIP_SetAttr(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &stAipAttr);
    HAL_AOE_AIP_Start(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);

    pTrack->bAttAi = MT_FALSE;
    pTrack->hAi = MT_INVALID_HANDLE;
    return Ret;
}
#endif
mt_s32 TRACK_GetDelayMs(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32DelayMs)
{
    SND_TRACK_STATE_S *pTrack;
    mt_u32 u32TrackDelayMs=0;
    mt_u32 u32TrackFiFoDelayMs=0;
    mt_u32 u32SndFiFoDelayMs=0;

    if (MT_NULL == pu32DelayMs)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &u32TrackDelayMs);
        HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &u32TrackFiFoDelayMs); // for aip fifo delay
    }
    else if(SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &u32TrackDelayMs);
        HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &u32TrackFiFoDelayMs); // for aip fifo delay
    }
    else if(SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough || SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
    {
        HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &u32TrackDelayMs);
        HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &u32TrackFiFoDelayMs); // for aip fifo delay
    }

    SND_GetDelayMs(pCard,&u32SndFiFoDelayMs);
    *pu32DelayMs = u32TrackDelayMs + u32TrackFiFoDelayMs + u32SndFiFoDelayMs;

    return MT_SUCCESS;
}

mt_s32 TRACK_IsBufEmpty(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL *pbBufEmpty)
{
    SND_TRACK_STATE_S *pTrack;
    mt_u32 u32TrackDelayMs=0;
    mt_u32 u32TrackFiFoDelayMs=0;

    if (MT_NULL == pbBufEmpty)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    if(MT_TRUE == TrackCheckIsPcmOutput(pCard))
    {
       HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &u32TrackDelayMs);
       HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_PCM], &u32TrackFiFoDelayMs); // for aip fifo delay
    }
    else if(SND_SPDIF_MODE_LBR == pCard->enSpdifPassthrough)
    {
       HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &u32TrackDelayMs);
       HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_SPDIF_RAW], &u32TrackFiFoDelayMs); // for aip fifo delay
    }
    else if(SND_HDMI_MODE_LBR == pCard->enHdmiPassthrough || SND_HDMI_MODE_HBR == pCard->enHdmiPassthrough)
    {
       HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &u32TrackDelayMs);
       HAL_AOE_AIP_GetFiFoDelayMs(pTrack->enAIP[SND_ENGINE_TYPE_HDMI_RAW], &u32TrackFiFoDelayMs); // for aip fifo delay
    }

    if(u32TrackDelayMs + u32TrackFiFoDelayMs <= AO_TRACK_BUF_EMPTY_THRESHOLD_MS)
    {
        *pbBufEmpty = MT_TRUE;
    }
    else
    {
        *pbBufEmpty = MT_FALSE;
    }

    return MT_SUCCESS;
}
mt_s32 TRACK_SetEosFlag(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_BOOL bEosFlag)
{
    SND_TRACK_STATE_S *pTrack;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    pTrack->bEosFlag = bEosFlag;

    return MT_SUCCESS;
}

mt_s32 TRACK_GetStatus(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_void *pstParam)
{
    //TO DO
    //s1:HAL_AOE_AOP_GetStatus
    return MT_SUCCESS;
}

mt_s32 TRACK_GetDefAttr(MT_UNF_AUDIOTRACK_ATTR_S * pstDefAttr)
{
    MT_INFO_AO("\n  TRACK_GetDefAttr   type %d    \n", pstDefAttr->enTrackType);
    //zgjiere;u32OutputBufSize 目前被忽略，AIP按照最大能力创建，避免频繁内存操作

	pstDefAttr->u32DebugCrcSize = 0;
	pstDefAttr->u32DebugCrc_phy = 0;
	pstDefAttr->u32DebugCrc_vir = 0;
    switch (pstDefAttr->enTrackType)
    {
    case MT_UNF_SND_TRACK_TYPE_MASTER:
    {
        pstDefAttr->u32BufLevelMs = AO_TRACK_MASTER_DEFATTR_BUFLEVELMS;
        pstDefAttr->u32OutputBufSize = AO_TRACK_MASTER_DEFATTR_BUFSIZE;        //verify
        pstDefAttr->u32FadeinMs  = AO_TRACK_MASTER_DEFATTR_FADEINMS;
        pstDefAttr->u32FadeoutMs = AO_TRACK_MASTER_DEFATTR_FADEOUTMS;
        pstDefAttr->b_spdif_mod = MT_FALSE;
        break;
    }

    case MT_UNF_SND_TRACK_TYPE_SLAVE:
    {
        pstDefAttr->u32BufLevelMs = AO_TRACK_SLAVE_DEFATTR_BUFLEVELMS;
        pstDefAttr->u32OutputBufSize = AO_TRACK_SLAVE_DEFATTR_BUFSIZE;        //verify
        pstDefAttr->u32FadeinMs  = AO_TRACK_SLAVE_DEFATTR_FADEINMS;
        pstDefAttr->u32FadeoutMs = AO_TRACK_SLAVE_DEFATTR_FADEOUTMS;
        pstDefAttr->b_spdif_mod = MT_FALSE;
        break;
    }

    case MT_UNF_SND_TRACK_TYPE_VIRTUAL:
    {
        pstDefAttr->u32OutputBufSize = AO_TRACK_VIRTUAL_DEFATTR_BUFSIZE;
        break;
    }

    default:

        //todo
        MT_ERR_AO("Get DefaultTrackAttr failed!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 TRACK_GetAttr(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, MT_UNF_AUDIOTRACK_ATTR_S * pstTrackAttr)
{
    SND_TRACK_STATE_S *pTrack;

    if (MT_NULL == pstTrackAttr)
    {
        return MT_FAILURE;
    }

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    memcpy(pstTrackAttr, &pTrack->stUserTrackAttr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));//verify  TrackType

    return MT_SUCCESS;
}

mt_s32 TRACK_GetSetting(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_TRACK_SETTINGS_S* pstSndSettings)
{
    SND_TRACK_STATE_S *pTrack;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    pstSndSettings->enCurnStatus = pTrack->enCurnStatus;
    pstSndSettings->enType = pTrack->enUserSpeedType;
    pstSndSettings->s32Speed = pTrack->s32UserSpeedRate;
    memcpy(&pstSndSettings->stTrackAbsGain, &pTrack->stTrackAbsGain, sizeof(MT_UNF_SND_ABSGAIN_ATTR_S));
    memcpy(&pstSndSettings->enChannelMode, &pTrack->enChannelMode, sizeof(MT_UNF_TRACK_MODE_E));
    memcpy(&pstSndSettings->stTrackAttr, &pTrack->stUserTrackAttr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
    pstSndSettings->bMute = pTrack->bMute;
    pstSndSettings->bAttAi = pTrack->bAttAi;
    pstSndSettings->hAi = pTrack->hAi;

    return MT_SUCCESS;
}

mt_s32 TRACK_RestoreSetting(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_TRACK_SETTINGS_S* pstSndSettings)
{
    SND_TRACK_STATE_S *pTrack;
    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if(SND_TRACK_STATUS_START == pstSndSettings->enCurnStatus)
    {
        TRACK_Start(pCard, u32TrackID);
    }
    else if(SND_TRACK_STATUS_STOP == pstSndSettings->enCurnStatus)
    {
        TRACK_Stop(pCard, u32TrackID);
    }
    else if(SND_TRACK_STATUS_PAUSE == pstSndSettings->enCurnStatus)
    {
        TRACK_Pause(pCard, u32TrackID);
    }

    TRACK_SetSpeedAdjust(pCard, u32TrackID, pstSndSettings->enType, pstSndSettings->s32Speed);
    TRACK_SetMute(pCard, u32TrackID, pstSndSettings->bMute);
    TRACK_SetAbsGain(pCard, u32TrackID, &pstSndSettings->stTrackAbsGain);

	TRACK_SetChannelMode(pCard, u32TrackID, &pstSndSettings->enChannelMode);
    return MT_SUCCESS;
}

mt_s32 Track_ReadProc( struct seq_file* p, SND_CARD_STATE_S *pCard )
{
    SND_TRACK_STATE_S *pTrack;
    SND_ENGINE_TYPE_E enEngine;
    AOE_AIP_CHN_ATTR_S stAipAttr;
    mt_u32 i;
    //mt_u32 u32DelayMs;

/*    PROC_PRINT( p,
            "All Mute: %s\n",((MT_FALSE == pCard->bAllTrackMute) ? "off" : "on"));*/

    for(i = 0; i < AO_MAX_TOTAL_TRACK_NUM; i++)
    {
        if(pCard->uSndTrackInitFlag & ((mt_u32)1L << i))
        {
            pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[i];
            PROC_PRINT(p,
               "Track(%d):    Type(%s), Status(%s), Weight(%.3d/%.3d%s)",
                pTrack->TrackId,
               (mt_char*)((MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType) ? "master" : ((MT_UNF_SND_TRACK_TYPE_SLAVE == pTrack->stUserTrackAttr.enTrackType) ? "slave" : "virtual")),
               (mt_char*)((SND_TRACK_STATUS_START == pTrack->enCurnStatus) ? "start" : ((SND_TRACK_STATUS_STOP == pTrack->enCurnStatus) ? "stop" : "pause")),
                pTrack->stTrackAbsGain.s32GainL,
                pTrack->stTrackAbsGain.s32GainR,
               (MT_TRUE == pTrack->stTrackAbsGain.bLinearMode)?"":"dB");

            if(MT_FALSE == pTrack->bAttAi)
            {
                PROC_PRINT(p, "\n");
            }
            else
            {
                PROC_PRINT(p, ", AttachAi(0x%lx)\n", pTrack->hAi);
            }

            /*PROC_PRINT(p,
               "             SpeedRate(%.2d), AddMuteFrames(%.4d), SendCnt(OK)(%.6u)\n",
                pTrack->s32UserSpeedRate,
                pTrack->u32AddMuteFrameNum,
                pTrack->u32SendCnt);*/

            for(enEngine = SND_ENGINE_TYPE_PCM; enEngine < SND_ENGINE_TYPE_BUTT; enEngine++)
            {
                if((SND_ENGINE_TYPE_PCM == enEngine) || (MT_UNF_SND_TRACK_TYPE_MASTER == pTrack->stUserTrackAttr.enTrackType))
                {
                    if((SND_ENGINE_TYPE_PCM == enEngine && (MT_FALSE == TrackCheckIsPcmOutput(pCard)))
                        || (SND_ENGINE_TYPE_SPDIF_RAW == enEngine && SND_SPDIF_MODE_PCM >= pCard->enSpdifPassthrough)
                        || (SND_ENGINE_TYPE_HDMI_RAW == enEngine && SND_HDMI_MODE_PCM >= pCard->enHdmiPassthrough))
                    {
                        continue;
                    }
                    HAL_AOE_AIP_GetAttr(pTrack->enAIP[enEngine], &stAipAttr);
                    PROC_PRINT(p,
                       "AIP(%x):      Engine(%s), SampleRate(%.6d), Channel(%.2d), BitWidth(%2d), DataFormat(%s)\n",
                       (mt_u32)pTrack->enAIP[enEngine],
                        AUTIL_Engine2Name(enEngine),
                        stAipAttr.stBufInAttr.u32BufSampleRate,
                        stAipAttr.stBufInAttr.u32BufChannels,
                        stAipAttr.stBufInAttr.u32BufBitPerSample,
                        AUTIL_Format2Name(stAipAttr.stBufInAttr.u32BufDataFormat));

                    PROC_PRINT(p,
                       "             buffsize(%08x),spdif(%s)\n",
                        (stAipAttr.stBufInAttr.stRbfAttr.u32BufSize),
                        (stAipAttr.stBufInAttr.stRbfAttr.b_spdif_mode)?"yes":"no");

                    /*HAL_AOE_AIP_GetBufDelayMs(pTrack->enAIP[enEngine], &u32DelayMs);
                    PROC_PRINT(p,
                       "             EmptyCnt(%.6u), EmptyWarningCnt(%.6u), Latency/Threshold(%.3dms/%.3dms)\n",
                        0,  //verify
                        0,  //verify
                        u32DelayMs,
                        stAipAttr.stBufInAttr.u32BufLatencyThdMs);*/
                 }
            }
            PROC_PRINT(p,"\n");
         }
    }

    return MT_SUCCESS;
}

mt_s32 TRACK_WriteProc(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, SND_DEBUG_CMD_CTRL_E enCmd)
{
    SND_TRACK_STATE_S *pTrack;
    mt_char szPath[AO_TRACK_PATH_NAME_MAXLEN + AO_TRACK_FILE_NAME_MAXLEN] = {0};
    struct tm now;

    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    if(!pTrack)
    {
        MT_ERR_AO("Track %d don't attach this sound\n");
        return MT_FAILURE;
    }

    if(MT_TRUE == pTrack->bAlsaTrack)
    {
        MT_ERR_AO("ALSA Track not support Echo WriteFile \n");
        return MT_FAILURE;
    }

    if(SND_DEBUG_CMD_CTRL_START == enCmd && SND_DEBUG_CMD_CTRL_STOP == pTrack->enSaveState)
    {
        if(MT_SUCCESS != mt_drv_file_get_storepath(szPath, AO_TRACK_PATH_NAME_MAXLEN))
        {
            MT_ERR_AO("get store path failed\n");
            return MT_FAILURE;
        }
        time64_to_tm(ktime_get_seconds(), 0, &now);
        snprintf(szPath, sizeof(szPath), "%s/track%d_%02u_%02u_%02u.pcm", szPath, u32TrackID, now.tm_hour, now.tm_min, now.tm_sec);
        pTrack->fileHandle = mt_drv_file_open(szPath, 1);
        if (!pTrack->fileHandle)
        {
            MT_ERR_AO("open %s error\n", szPath);
            return MT_FAILURE;
        }
        pTrack->u32SaveCnt++;
    }
    if(SND_DEBUG_CMD_CTRL_STOP == enCmd && SND_DEBUG_CMD_CTRL_START == pTrack->enSaveState)
    {
        if(pTrack->fileHandle)
        {
            mt_drv_file_close(pTrack->fileHandle);
            pTrack->fileHandle = MT_NULL;
        }
    }

    pTrack->enSaveState = enCmd;

    return MT_SUCCESS;
}


mt_s32 TRACK_UpdateWptrPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32WptrLen)    //for alsa
{
    SND_TRACK_STATE_S *pTrack;
    /*
    SND_TRACK_STREAM_ATTR_S stStreamAttr;
    STREAMMODE_CHANGE_ATTR_S stChange;
    mt_u32 Write = 0;
    */
    if (MT_NULL == pu32WptrLen)
    {
        return MT_FAILURE;
    }

    //todo , check attr


    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    /*
    if (SND_TRACK_STATUS_START != pTrack->enCurnStatus)
    {
        MT_ERR_AO("Ao track status(%d) should be SND_TRACK_STATUS_START  \n", pTrack->enCurnStatus);
        //return MT_FAILURE;
    }
    */
    HAL_AOE_AIP_UpdateWritePos(pTrack->enAIP[SND_ENGINE_TYPE_PCM], pu32WptrLen);

    return  MT_SUCCESS;
}

mt_s32 TRACK_UpdateRptrPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32RptrLen)    //for alsa
{
	SND_TRACK_STATE_S *pTrack;
	/*
	SND_TRACK_STREAM_ATTR_S stStreamAttr;
	STREAMMODE_CHANGE_ATTR_S stChange;
	mt_u32 Write = 0;
	*/
	if (MT_NULL == pu32RptrLen)
	{
		return MT_FAILURE;
	}

	//todo , check attr


	pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
	/*
	if (SND_TRACK_STATUS_START != pTrack->enCurnStatus)
	{
		MT_ERR_AO("Ao track status(%d) should be SND_TRACK_STATUS_START  \n", pTrack->enCurnStatus);
		//return MT_FAILURE;
	}
	*/
	HAL_AOE_AIP_UpdateReadPos(pTrack->enAIP[SND_ENGINE_TYPE_PCM], pu32RptrLen);

	return	MT_SUCCESS;
}
mt_s32 TRACK_GetReadPos(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID, mt_u32 *pu32ReadPos)    //for alsa
{
    SND_TRACK_STATE_S *pTrack;
    /*
    SND_TRACK_STREAM_ATTR_S stStreamAttr;
    STREAMMODE_CHANGE_ATTR_S stChange;
    mt_u32 Write = 0;
    */
    if (MT_NULL == pu32ReadPos)
    {
        return MT_FAILURE;
    }

    //todo , check attr


    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];
    /*
    if (SND_TRACK_STATUS_START != pTrack->enCurnStatus)
    {
        MT_ERR_AO("Ao track status(%d) should be SND_TRACK_STATUS_START  \n", pTrack->enCurnStatus);
        //return MT_FAILURE;
    }
    */
    HAL_AOE_AIP_GetReadPos(pTrack->enAIP[SND_ENGINE_TYPE_PCM], pu32ReadPos);

    return  MT_SUCCESS;
}


mt_s32 TRACK_FlushBuf(SND_CARD_STATE_S *pCard, mt_u32 u32TrackID)    //for alsa
{
    SND_TRACK_STATE_S *pTrack;
    //todo , check attr



    pTrack = (SND_TRACK_STATE_S *)pCard->hSndTrack[u32TrackID];

    HAL_AOE_AIP_FlushBuf(pTrack->enAIP[SND_ENGINE_TYPE_PCM]);

    return  MT_SUCCESS;
}
