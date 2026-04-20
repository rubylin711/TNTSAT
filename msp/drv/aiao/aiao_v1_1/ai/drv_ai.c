/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/interrupt.h>
#include <linux/clk.h>

#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_kernel_adapt.h"
#include "mt_module.h"
#include "mt_drv_mmz.h"
#include "mt_drv_sys.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "mt_error_mpi.h"

#include "hal_aiao.h"
#include "audio_util.h"

#include "mt_drv_ai.h"

#include <sound/pcm.h>

#include "drv_ai_private.h"
#include "drv_ai_ioctl.h"
//#include "hal_tianlai_adac_v510.h"
#include "aud_in_aria_reg.h"
#include "mt_drv_file.h"

mt_s32 AO_Track_GetDelayMs(mt_u32 u32TrackID, mt_u32 *pu32DelayMs);

static mt_s32 AI_RegProc(mt_u32 u32Ai);
static mt_void AI_UnRegProc(mt_u32 u32Ai);

#define MT_AI_DRV_SUSPEND_SUPPORT

MT_DECLARE_MUTEX(g_AIMutex);

static atomic_t g_AIOpenCnt = ATOMIC_INIT(0);

struct file g_file;

//AI Resource
static AI_GLOBAL_RESOURCE_S g_pstGlobalAIRS =
    {
        .pstProcParam = MT_NULL,
        .stExtFunc =
            {
	        .pfnAI_DrvResume = AI_DRV_Resume,
	        .pfnAI_DrvSuspend = AI_DRV_Suspend,
	    }
    };

#ifdef MT_ALSA_AI_SUPPORT
AI_ALSA_Param_S g_stAlsaAttr;
#endif

static MT_BOOL AICheckPortValid(MT_UNF_AI_E enAiPort)
{

    if ((MT_UNF_AI_I2S0 != enAiPort) && (MT_UNF_AI_I2S1 != enAiPort) && (MT_UNF_AI_SIF0 != enAiPort) && (MT_UNF_AI_ADC0 != enAiPort) && (MT_UNF_AI_ADC1 != enAiPort) && (MT_UNF_AI_ADC2 != enAiPort) && (MT_UNF_AI_ADC3 != enAiPort) && (MT_UNF_AI_ADC4 != enAiPort) && (MT_UNF_AI_HDMI3 != enAiPort) && (MT_UNF_AI_HDMI0 != enAiPort) && (MT_UNF_AI_HDMI1 != enAiPort) && (MT_UNF_AI_HDMI2 != enAiPort)) {
	///TODO:cooper should fill
	MT_ERR_AI("just support I2S0, I2S1 , SIF, HDMI, ADC Port!\n");
	return MT_FALSE;
    }

    return MT_TRUE;
}

static MT_BOOL AICheckPortUsed(MT_UNF_AI_E enAiPort)
{
    mt_u32 i;

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if (g_pstGlobalAIRS.pstAI_ATTR_S[i]) {
	    if (g_pstGlobalAIRS.pstAI_ATTR_S[i]->enAiPort == enAiPort) {
		MT_ERR_AI("This port has been occupied!\n");
		return MT_FALSE;
	    }
	}
    }

    return MT_TRUE;
}

static mt_void AIGetChannelsAndBitDepth(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAiAttr,
                                        mt_u32 *pu32Channel, mt_u32 *pu32BitDepth)
{
    switch (enAiPort) {
    case MT_UNF_AI_I2S0:
    case MT_UNF_AI_I2S1:
	*pu32Channel = pstAiAttr->unAttr.stI2sAttr.stAttr.enChannel;   // 2
	*pu32BitDepth = pstAiAttr->unAttr.stI2sAttr.stAttr.enBitDepth; // 16
	return;

    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
	*pu32Channel = pstAiAttr->unAttr.stHDMIAttr.enChannel;
	*pu32BitDepth = pstAiAttr->unAttr.stHDMIAttr.enBitDepth;
	return;

    case MT_UNF_AI_ADC0:
    case MT_UNF_AI_ADC1:
    case MT_UNF_AI_ADC2:
    case MT_UNF_AI_ADC3:
    case MT_UNF_AI_ADC4:
    case MT_UNF_AI_SIF0:
	*pu32Channel = MT_UNF_I2S_CHNUM_2;
	*pu32BitDepth = MT_UNF_I2S_BIT_DEPTH_16;
	return;
    default:
	return;
    }
}

mt_s32 AI_GetDefaultAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAiAttr)
{
    if (MT_FALSE == AICheckPortValid(enAiPort)) {
	return MT_FAILURE;
    }

    pstAiAttr->enSampleRate = MT_UNF_SAMPLE_RATE_48K;
    pstAiAttr->u32PcmFrameMaxNum = AI_BUFF_FRAME_NUM_DF;
    pstAiAttr->u32PcmSamplesPerFrame = AI_SAMPLE_PERFRAME_DF;
    if (MT_UNF_AI_I2S0 == enAiPort || MT_UNF_AI_I2S1 == enAiPort) {
	pstAiAttr->unAttr.stI2sAttr.stAttr.bMaster = MT_TRUE;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enI2sMode = MT_UNF_I2S_STD_MODE;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enMclkSel = MT_UNF_I2S_MCLK_256_FS;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enBclkSel = MT_UNF_I2S_BCLK_4_DIV;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enChannel = MT_UNF_I2S_CHNUM_2;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enBitDepth = MT_UNF_I2S_BIT_DEPTH_16;
	pstAiAttr->unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge = MT_TRUE;
	pstAiAttr->unAttr.stI2sAttr.stAttr.enPcmDelayCycle = MT_UNF_I2S_PCM_0_DELAY;
    } else if (MT_UNF_AI_ADC0 == enAiPort || MT_UNF_AI_ADC1 == enAiPort ||
               MT_UNF_AI_ADC2 == enAiPort || MT_UNF_AI_ADC3 == enAiPort || MT_UNF_AI_ADC4 == enAiPort) {
	pstAiAttr->unAttr.stAdcAttr.bByPass = MT_FALSE;
    } else if (MT_UNF_AI_HDMI0 == enAiPort || MT_UNF_AI_HDMI1 == enAiPort ||
               MT_UNF_AI_HDMI2 == enAiPort || MT_UNF_AI_HDMI3 == enAiPort) {
	pstAiAttr->unAttr.stHDMIAttr.enSampleRate = MT_UNF_SAMPLE_RATE_48K;
	pstAiAttr->unAttr.stHDMIAttr.enBitDepth = MT_UNF_I2S_BIT_DEPTH_16;
	pstAiAttr->unAttr.stHDMIAttr.enChannel = MT_UNF_I2S_CHNUM_2;
	pstAiAttr->unAttr.stHDMIAttr.enHdmiAudioDataFormat = MT_UNF_AI_HDMI_FORMAT_LPCM;
    }

    return MT_SUCCESS;
}

#ifdef MT_ALSA_AI_SUPPORT
mt_s32 AIGetProcStatistics(AIAO_IsrFunc **pFunc) //For ALSA
{
    AIAO_PORT_USER_CFG_S pAttr;

    HAL_AIAO_P_GetTxI2SDfAttr(AIAO_PORT_TX0, &pAttr); //pIsrFunc is the same for all ports

    *pFunc = pAttr.pIsrFunc;

    return MT_SUCCESS;
}

mt_s32 AIGetEnport(mt_handle hAi, AIAO_PORT_ID_E *enPort) //For ALSA
{
    AI_CHANNEL_STATE_S *state = MT_NULL;
    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];
    if (state)
	*enPort = state->enPort;
    return MT_SUCCESS;
}
#endif

static mt_s32 AICheckI2sAttr(MT_UNF_AI_I2S_ATTR_S *pstI2sAttr)
{
    CHECK_AI_MCLKDIV(pstI2sAttr->stAttr.enMclkSel);
    CHECK_AI_BCLKDIV(pstI2sAttr->stAttr.enBclkSel);
    CHECK_AI_CHN(pstI2sAttr->stAttr.enChannel);
    CHECK_AI_BITDEPTH(pstI2sAttr->stAttr.enBitDepth);
    CHECK_AI_PCMDELAY(pstI2sAttr->stAttr.enPcmDelayCycle);

    if (MT_UNF_I2S_MODE_BUTT <= pstI2sAttr->stAttr.enI2sMode) {
	MT_ERR_AI("dont support I2sMode(%d)\n", pstI2sAttr->stAttr.enI2sMode);
	return MT_ERR_AI_INVALID_PARA;
    }

    if (MT_UNF_I2S_MCLK_BUTT <= pstI2sAttr->stAttr.enMclkSel) {
	MT_ERR_AI("dont support I2S MclkSel(%d)\n", pstI2sAttr->stAttr.enBclkSel);
	return MT_ERR_AI_INVALID_PARA;
    }

    return MT_SUCCESS;
}

static mt_s32 AICheckHdmiAttr(MT_UNF_AI_HDMI_ATTR_S *pstHdmiAttr)
{
    CHECK_AI_CHN(pstHdmiAttr->enChannel);
    CHECK_AI_BITDEPTH(pstHdmiAttr->enBitDepth);
    CHECK_AI_HdmiDataFormat(pstHdmiAttr->enHdmiAudioDataFormat);

    return MT_SUCCESS;
}

static mt_s32 AICheckAdcAttr(MT_UNF_AI_ATTR_S *pstAiAttr)
{
    if (MT_UNF_SAMPLE_RATE_48K != pstAiAttr->enSampleRate) {
	MT_ERR_AI("ADC port only support 48k samplerate!\n");
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AICheckAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAiAttr)
{
    mt_s32 Ret = MT_SUCCESS;
    CHECK_AI_SAMPLERATE(pstAiAttr->enSampleRate);

    switch (enAiPort) {
    case MT_UNF_AI_I2S0:
    case MT_UNF_AI_I2S1:
	Ret = AICheckI2sAttr(&pstAiAttr->unAttr.stI2sAttr);
	break;
    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
	Ret = AICheckHdmiAttr(&pstAiAttr->unAttr.stHDMIAttr);
	break;
    case MT_UNF_AI_ADC0:
    case MT_UNF_AI_ADC1:
    case MT_UNF_AI_ADC2:
    case MT_UNF_AI_ADC3:
    case MT_UNF_AI_ADC4:
	Ret = AICheckAdcAttr(pstAiAttr);
	break;
    default:
	break;
    }

    return Ret;
}

static mt_void AISetPortIfAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAiAttr, AIAO_IfAttr_S *pstIfAttr)
{
    pstIfAttr->enRate = (AIAO_SAMPLE_RATE_E)(pstAiAttr->enSampleRate);

    if (MT_UNF_AI_I2S0 == enAiPort || MT_UNF_AI_I2S1 == enAiPort) {
	if (pstAiAttr->unAttr.stI2sAttr.stAttr.bMaster) {
	    pstIfAttr->enCrgMode = AIAO_CRG_MODE_DUPLICATE;
	    if (MT_UNF_AI_I2S0 == enAiPort) {
		pstIfAttr->eCrgSource = AIAO_TX_CRG0;
	    }
	    if (MT_UNF_AI_I2S1 == enAiPort) {
		pstIfAttr->eCrgSource = AIAO_TX_CRG1;
	    }

	    pstIfAttr->u32BCLK_DIV = pstAiAttr->unAttr.stI2sAttr.stAttr.enBclkSel;
	    pstIfAttr->u32FCLK_DIV = AUTIL_BclkFclkDiv(pstAiAttr->unAttr.stI2sAttr.stAttr.enMclkSel, pstAiAttr->unAttr.stI2sAttr.stAttr.enBclkSel);
	} else {
	    pstIfAttr->enCrgMode = AIAO_CRG_MODE_SLAVE;
	}
	pstIfAttr->enI2SMode = (AIAO_I2S_MODE_E)(pstAiAttr->unAttr.stI2sAttr.stAttr.enI2sMode);
	pstIfAttr->enChNum = (AIAO_I2S_CHNUM_E)(pstAiAttr->unAttr.stI2sAttr.stAttr.enChannel);
	pstIfAttr->enBitDepth = (AIAO_BITDEPTH_E)(pstAiAttr->unAttr.stI2sAttr.stAttr.enBitDepth);
	pstIfAttr->u32PcmDelayCycles = pstAiAttr->unAttr.stI2sAttr.stAttr.enPcmDelayCycle;
	if (pstAiAttr->unAttr.stI2sAttr.stAttr.bPcmSampleRiseEdge == MT_TRUE) {
	    pstIfAttr->enRiseEdge = AIAO_MODE_EDGE_RISE;
	} else {
	    pstIfAttr->enRiseEdge = AIAO_MODE_EDGE_FALL;
	}
    } else if (MT_UNF_AI_HDMI0 == enAiPort || MT_UNF_AI_HDMI1 == enAiPort || MT_UNF_AI_HDMI2 == enAiPort || MT_UNF_AI_HDMI3 == enAiPort) {
	pstIfAttr->enChNum = (AIAO_I2S_CHNUM_E)(pstAiAttr->unAttr.stHDMIAttr.enChannel);
	pstIfAttr->enBitDepth = (AIAO_BITDEPTH_E)(pstAiAttr->unAttr.stHDMIAttr.enBitDepth);
    } else if (MT_UNF_AI_ADC0 == enAiPort || MT_UNF_AI_ADC1 == enAiPort || MT_UNF_AI_ADC2 == enAiPort || MT_UNF_AI_ADC3 == enAiPort || MT_UNF_AI_ADC4 == enAiPort) {
    }

    return;
}

static mt_s32 AICreateChn(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, MT_BOOL bAlsa, mt_void *pAlsaPara, AI_CHANNEL_STATE_S *state)
{
    mt_s32 Ret;
    AIAO_PORT_USER_CFG_S stHwPortAttr;

    AIAO_PORT_ID_E enPort = AIAO_PORT_BUTT;

#ifdef MT_ALSA_AI_SUPPORT
    AI_ALSA_Param_S *pstAlsaAttr = (AI_ALSA_Param_S *)pAlsaPara;
#endif

    switch (enAiPort) {
#if defined(MT_I2S0_SUPPORT)
    case MT_UNF_AI_I2S0:
	enPort = AIAO_PORT_RX0;
	HAL_AIAO_P_GetBorardRxI2SDfAttr(0, &enPort, &stHwPortAttr);
	break;
#endif

#if defined(MT_I2S1_SUPPORT)
    case MT_UNF_AI_I2S1:
	enPort = AIAO_PORT_RX1;
	HAL_AIAO_P_GetBorardRxI2SDfAttr(1, &enPort, &stHwPortAttr);
	break;
#endif
    case MT_UNF_AI_ADC0:
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_MIC, pstAttr->unAttr.stAdcAttr.bByPass, MT_TRUE);
#endif
	enPort = AIAO_PORT_RX2;
	HAL_AIAO_P_GetRxAdcDfAttr(enPort, &stHwPortAttr);

	break;
    case MT_UNF_AI_ADC1:
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_L1, pstAttr->unAttr.stAdcAttr.bByPass, MT_TRUE);
#endif
	enPort = AIAO_PORT_RX2;
	HAL_AIAO_P_GetRxAdcDfAttr(enPort, &stHwPortAttr);
	break;
    case MT_UNF_AI_ADC2:
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_L2, pstAttr->unAttr.stAdcAttr.bByPass, MT_TRUE);
#endif
	enPort = AIAO_PORT_RX2;
	HAL_AIAO_P_GetRxAdcDfAttr(enPort, &stHwPortAttr);
	break;
    case MT_UNF_AI_ADC3:
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_L3, pstAttr->unAttr.stAdcAttr.bByPass, MT_TRUE);
#endif
	enPort = AIAO_PORT_RX2;
	HAL_AIAO_P_GetRxAdcDfAttr(enPort, &stHwPortAttr);
	break;
    case MT_UNF_AI_ADC4:
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_L4, pstAttr->unAttr.stAdcAttr.bByPass, MT_TRUE);
#endif
	enPort = AIAO_PORT_RX2;
	HAL_AIAO_P_GetRxAdcDfAttr(enPort, &stHwPortAttr);
	break;
    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
#if defined(CMTP_TYPE_hi3798mv100_a)
	enPort = AIAO_PORT_RX1;
#else
	enPort = AIAO_PORT_RX3;
#endif
	HAL_AIAO_P_GetRxHdmiDfAttr(enPort, &stHwPortAttr);
	break;
    case MT_UNF_AI_SIF0:
	enPort = AIAO_PORT_RX1;
	HAL_AIAO_P_GetRxSifDfAttr(enPort, &stHwPortAttr);
	break;
    default:
	MT_ERR_AI("Aiport is invalid!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

//stHwPortAttr.stBufConfig.u32PeriodNumber = state->stRbfMmz.u32Size/stHwPortAttr.stBufConfig.u32PeriodBufSize;
#ifdef MT_ALSA_AI_SUPPORT
    if (bAlsa == MT_TRUE) {
	stHwPortAttr.bExtDmaMem = MT_TRUE;
	stHwPortAttr.stExtMem.u32BufPhyAddr = pstAlsaAttr->stBuf.u32BufPhyAddr;
	stHwPortAttr.stExtMem.u32BufVirAddr = pstAlsaAttr->stBuf.u32BufVirAddr;
	stHwPortAttr.stExtMem.u32BufSize = pstAlsaAttr->stBuf.u32BufSize;
	state->stRbfMmz.size = pstAlsaAttr->stBuf.u32BufSize;
	state->stRbfMmz.startPhyAddr = pstAlsaAttr->stBuf.u32BufPhyAddr;
	state->stRbfMmz.startVirAddr = pstAlsaAttr->stBuf.u32BufVirAddr;
	stHwPortAttr.pIsrFunc = pstAlsaAttr->IsrFunc;
	stHwPortAttr.substream = pstAlsaAttr->substream;

	stHwPortAttr.stBufConfig.u32PeriodBufSize = pstAlsaAttr->stBuf.u32PeriodByteSize;
	stHwPortAttr.stBufConfig.u32PeriodNumber = pstAlsaAttr->stBuf.u32Periods;
    } else
#endif
    {
	stHwPortAttr.bExtDmaMem = MT_TRUE;
	stHwPortAttr.stExtMem.u32BufPhyAddr = state->stRbfMmz.startPhyAddr;
	stHwPortAttr.stExtMem.u32BufVirAddr = (ulong)state->stRbfMmz.startVirAddr;
	stHwPortAttr.stExtMem.u32BufSize = state->stRbfMmz.size; //ppbuf size + pcm buf size
	                                                            //ykang
	stHwPortAttr.stExtMem.u32bufszperch =
	    pstAttr->u32PcmSamplesPerFrame * (pstAttr->unAttr.stI2sAttr.stAttr.enBitDepth / 8) * pstAttr->u32PcmFrameMaxNum; //one circle buf size
	stHwPortAttr.stExtMem.u32samplecntoneframe = pstAttr->u32PcmSamplesPerFrame;                                         //state->stAiBuf.u32Read;
	stHwPortAttr.stExtMem.u32bitdepth = pstAttr->unAttr.stI2sAttr.stAttr.enBitDepth;
   stHwPortAttr.stExtMem.bInterlaceMod = pstAttr->bInterlaceMod;

	if (stHwPortAttr.stExtMem.u32BufSize < (stHwPortAttr.stExtMem.u32bufszperch * 8 * 2)) {
	    MT_ASSERT(0);
	}
    }

    AISetPortIfAttr(enAiPort, pstAttr, &stHwPortAttr.stIfAttr);
    Ret = HAL_AIAO_P_Open_Vsb(enPort, &stHwPortAttr);
    if (MT_SUCCESS != Ret) {
	MT_ERR_AI("HAL_AIAO_P_Open failed\n");
	return MT_FAILURE;
    }

    //2.set state
    state->enCurnStatus = AI_CHANNEL_STATUS_STOP;
    state->enAiPort = enAiPort;
    state->enPort = enPort;

    state->stAiProc.u32AqcCnt = 0; //init proc info
    state->stAiProc.u32AqcTryCnt = 0;
    state->stAiProc.u32RelCnt = 0;
    state->stAiProc.u32RelTryCnt = 0;
    state->u32Rptr = 0;
    state->u32Wptr = 0;
    memcpy(&state->stSndPortAttr, pstAttr, sizeof(MT_UNF_AI_ATTR_S));

    return MT_SUCCESS;
}

static mt_s32 AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, MT_BOOL bAlsa, mt_void *pAlsaPara, mt_handle hAi)
{
    mt_s32 Ret = MT_SUCCESS;
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    if ((pstAttr->u32PcmFrameMaxNum <= 0) || (pstAttr->u32PcmSamplesPerFrame <= 0)) {
	MT_ERR_AI("PcmFrameMaxNum(%d) is invalid!\n", pstAttr->u32PcmFrameMaxNum);
	return MT_ERR_AI_INVALID_PARA;
    }

    if (MT_NULL == g_pstGlobalAIRS.pstAI_ATTR_S[hAi]) {
	printk("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }
    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    Ret = AICreateChn(enAiPort, pstAttr, bAlsa, pAlsaPara, state);
    if (MT_SUCCESS != Ret) {
	printk("AICreateChn failed 0x%x\n", Ret);
	return MT_FAILURE;
    }

    state->bAlsa = bAlsa;
    state->pAlsaPara = pAlsaPara;
    state->bAttach = MT_FALSE;

    state->enSaveState = AI_CMD_CTRL_STOP;
    state->u32SaveCnt = 0;
    state->fileHandle = MT_NULL;

#if defined(MT_ALSA_AI_SUPPORT)
    if (state->bAlsa == MT_TRUE) {
	memcpy(&g_stAlsaAttr, (AI_ALSA_Param_S *)(pAlsaPara), sizeof(AI_ALSA_Param_S));
    }
#endif
    //4.set global variable

    switch (enAiPort) {
    case MT_UNF_AI_I2S0:
	g_pstGlobalAIRS.u32BitFlag_AI = (mt_u32)1 << AI_I2S0_MSK;
	break;
    case MT_UNF_AI_I2S1:
	g_pstGlobalAIRS.u32BitFlag_AI = (mt_u32)1 << AI_I2S1_MSK;
	break;

    case MT_UNF_AI_ADC0:
    case MT_UNF_AI_ADC1:
    case MT_UNF_AI_ADC2:
    case MT_UNF_AI_ADC3:
    case MT_UNF_AI_ADC4:
	g_pstGlobalAIRS.u32BitFlag_AI = (mt_u32)1 << AI_ADAC_MSK;
	break;
    case MT_UNF_AI_SIF0:
	g_pstGlobalAIRS.u32BitFlag_AI = (mt_u32)1 << AI_I2S1_MSK;
	break;
    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
	g_pstGlobalAIRS.u32BitFlag_AI = (mt_u32)1 << AI_HDMI_MSK;
	break;

    default:
	printk("Aiport is invalid!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    AI_RegProc(hAi);

    return MT_SUCCESS;
}

static mt_s32 AIChnDestory(AI_CHANNEL_STATE_S *state)
{
    HAL_AIAO_P_Close(state->enPort);
    return MT_SUCCESS;
}

static mt_s32 AI_Destory(mt_handle hAi)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];
    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    AI_UnRegProc(hAi);
    AIChnDestory(state);

    switch (state->enAiPort) {
#if defined(MT_I2S0_SUPPORT)
    case MT_UNF_AI_I2S0:
	g_pstGlobalAIRS.u32BitFlag_AI &= (mt_u32)(~(1 << AI_I2S0_MSK));
	break;
#endif
#if defined(MT_I2S1_SUPPORT)
    case MT_UNF_AI_I2S1:
	g_pstGlobalAIRS.u32BitFlag_AI &= (mt_u32)(~(1 << AI_I2S1_MSK));
	break;
#endif
    case MT_UNF_AI_ADC0:
    case MT_UNF_AI_ADC1:
    case MT_UNF_AI_ADC2:
    case MT_UNF_AI_ADC3:
    case MT_UNF_AI_ADC4:
	g_pstGlobalAIRS.u32BitFlag_AI &= (mt_u32)(~(1 << AI_ADAC_MSK));
#ifdef MT_TIANLAI_V510
	HAL_TIANLAI_V510_SetLineInSuorce(S5_TIANLAI_LINEIN_SEL_MIC, MT_FALSE, MT_FALSE);
#endif
	break;
    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
	g_pstGlobalAIRS.u32BitFlag_AI &= (mt_u32)(~(1 << AI_HDMI_MSK));
	break;
    case MT_UNF_AI_SIF0:
	g_pstGlobalAIRS.u32BitFlag_AI &= (mt_u32)(~(1 << AI_I2S1_MSK));
	break;
    default:
	MT_ERR_AI("Aiport is invalid!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    return MT_SUCCESS;
}

mt_s32 AI_SetEnable(mt_handle hAi, MT_BOOL bEnable, MT_BOOL bTrackResume)
{
    mt_s32 Ret = MT_SUCCESS;
    //mt_u32 u32AiDelayMs = 0;
    //mt_u32 u32AoDelayMs = 0;
    //mt_u32 u32BytesSize = 0;
    mt_u32 u32FrameSize = 0;
    AI_CHANNEL_STATE_S *state = MT_NULL;
    AIAO_PORT_ATTR_S stPortAttr;

    memset(&stPortAttr, 0, sizeof(AIAO_PORT_ATTR_S));

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_WARN_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    if (bEnable) {

	Ret = HAL_AIAO_P_GetAttr(state->enPort, &stPortAttr);
	if (MT_SUCCESS != Ret) {
	    MT_WARN_AI("HAL_AIAO_P_GetAttr(%d) failed\n", state->enPort);
	    return Ret;
	}
	u32FrameSize = AUTIL_CalcFrameSize(stPortAttr.stIfAttr.enChNum, stPortAttr.stIfAttr.enBitDepth);

	printk("%s, %d, AUTIL_CalcFrameSize, u32FrameSize %d, %d %d\n", __FUNCTION__, __LINE__,
	       u32FrameSize,
	       stPortAttr.stIfAttr.enChNum,
	       stPortAttr.stIfAttr.enBitDepth);

#if 0
        if(bTrackResume)
        {
            u32BytesSize = AUTIL_LatencyMs2ByteSize(20,  u32FrameSize, stPortAttr.stIfAttr.enRate);
            ///TODO:minnan remove
            HAL_AIAO_P_WriteData(state->enPort, MT_NULL, u32BytesSize);
	     printk("%s, %d, calling HAL_AIAO_P_WriteData\n",__FUNCTION__,__LINE__);

        }

        HAL_AIAO_P_GetDelayMs(state->enPort, &u32AiDelayMs);
	  printk("%s, %d, calling HAL_AIAO_P_WriteData\n",__FUNCTION__,__LINE__);

        if(MT_TRUE == state->bAttach)//false
        {
            Ret = AO_Track_GetDelayMs(state->hTrack, &u32AoDelayMs);
	     printk("%s, %d, calling AO_Track_GetDelayMs\n",__FUNCTION__,__LINE__);

            if (MT_SUCCESS != Ret)
            {
                printk("AO_Track_GetDelayMs(%d) failed\n", state->hTrack);
                return Ret;
            }
        }

        if(u32AiDelayMs + u32AoDelayMs < state->stDelayComps.u32DelayMs)
        {
            u32BytesSize = AUTIL_LatencyMs2ByteSize(state->stDelayComps.u32DelayMs - u32AiDelayMs - u32AoDelayMs,
                                                        u32FrameSize, stPortAttr.stIfAttr.enRate);
            ///TODO:minnan remove
             HAL_AIAO_P_WriteData(state->enPort, MT_NULL, u32BytesSize);
	      printk("%s, %d, calling HAL_AIAO_P_WriteData\n",__FUNCTION__,__LINE__);

        }

        Ret = HAL_AIAO_P_Start(state->enPort);
	 printk("%s, %d, calling HAL_AIAO_P_Start\n",__FUNCTION__,__LINE__);
#endif

	HAL_AO_Hw_Config(state->enPort);
	HAL_AI_Hw_Reset();
	HAL_AI_IO_config();
	HAL_AI_Clk_Srcsel();
	//HAL_AI_Inter_En();//control by debugi2c
	HAL_AI_Hw_Config(state->enPort);
    printk("n\n %s,%d \n", __FUNCTION__, __LINE__);

	HAL_AI_Hw_Start();
    printk("n\n %s,%d \n", __FUNCTION__, __LINE__);

	HAL_AO_Hw_Start();
    printk("n\n %s,%d \n", __FUNCTION__, __LINE__);


	if (MT_SUCCESS != Ret) {
	    MT_WARN_AI("HAL_AIAO_P_Start(%d) failed\n", state->enPort);
	} else {
	    state->enCurnStatus = AI_CHANNEL_STATUS_START;
	}
    } else {
	HAL_AI_Hw_stop();
	if (MT_SUCCESS != Ret) {
	    MT_WARN_AI("HAL_AIAO_P_Stop(%d) failed\n", state->enPort);
	} else {
	    state->enCurnStatus = AI_CHANNEL_STATUS_STOP;
	}
    }

    return Ret;
}

mt_s32 AI_GetEnable(mt_handle hAi, MT_BOOL *pbEnable)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    if (AI_CHANNEL_STATUS_START == state->enCurnStatus) {
	*pbEnable = MT_TRUE;
    } else if (AI_CHANNEL_STATUS_STOP == state->enCurnStatus) {
	*pbEnable = MT_FALSE;
    } else {
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AI_SetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pAiAttr)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;
    AIAO_PORT_ATTR_S stPortAttr;
    mt_s32 Ret;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    Ret = AICheckAttr(state->enAiPort, pAiAttr);
    if (MT_SUCCESS != Ret) {
	MT_ERR_AI("invalid ai attr!\n");
	return Ret;
    }

    if (AI_CHANNEL_STATUS_STOP != state->enCurnStatus) {
	MT_ERR_AI("current state is not stop,can not set attr!\n");
	return MT_FAILURE;
    }

    HAL_AIAO_P_GetAttr(state->enPort, &stPortAttr);

    AISetPortIfAttr(state->enAiPort, pAiAttr, &stPortAttr.stIfAttr);

    Ret = HAL_AIAO_P_SetAttr(state->enPort, &stPortAttr);
    if (MT_SUCCESS != Ret) {
	return Ret;
    }
    memcpy(&state->stSndPortAttr, pAiAttr, sizeof(MT_UNF_AI_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 AI_GetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pAiAttr)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];
    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }
    memcpy(pAiAttr, &state->stSndPortAttr, sizeof(MT_UNF_AI_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 AI_GetPortAttr(mt_handle hAi, AIAO_PORT_ATTR_S *pstPortAttr)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];
    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    if (MT_SUCCESS != HAL_AIAO_P_GetAttr(state->enPort, pstPortAttr)) {
	MT_ERR_AI("HAL_AIAO_P_GetAttr failed!\n");
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AICheckDelayComps(AI_CHANNEL_STATE_S *state, mt_u32 u32CompensationMs)
{
    mt_u32 u32BufDelayMs = 0;

    if (state->stSndPortAttr.enSampleRate) {
	u32BufDelayMs = state->stSndPortAttr.u32PcmSamplesPerFrame * state->stSndPortAttr.u32PcmFrameMaxNum * 1000 / state->stSndPortAttr.enSampleRate;
    }

    if (u32CompensationMs > u32BufDelayMs) {
	MT_ERR_AI("u32CompensationMs(%d) exceed u32BufDelayMs(%d)!\n", u32CompensationMs, u32BufDelayMs);
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AI_SetDelayComps(mt_handle hAi, MT_UNF_AI_DELAY_S *pstDelayComps)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
	;
    }

    if (MT_SUCCESS != AICheckDelayComps(state, pstDelayComps->u32DelayMs)) {
	return MT_FAILURE;
    }

    if (AI_CHANNEL_STATUS_STOP != state->enCurnStatus) {
	MT_ERR_AI("current state is not stop,can not set delay compensation!\n");
	return MT_FAILURE;
    }

    memcpy(&state->stDelayComps, pstDelayComps, sizeof(MT_UNF_AI_DELAY_S));

    return MT_SUCCESS;
}

static mt_s32 AI_GetDelayComps(mt_handle hAi, MT_UNF_AI_DELAY_S *pstDelayComps)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_ERR_AI_INVALID_PARA;
	;
    }

    memcpy(pstDelayComps, &state->stDelayComps, sizeof(MT_UNF_AI_DELAY_S));

    return MT_SUCCESS;
}

static mt_void AISavePcmData(AI_CHANNEL_STATE_S *pAistate, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_u32 u32FrameSize;
    mt_s32 s32Len;

    if (AI_CMD_CTRL_START == pAistate->enSaveState) {
	if (16 == pstFrame->s32BitPerSample || 32 == pstFrame->s32BitPerSample) {
	    if (pAistate->fileHandle) {

		u32FrameSize = AUTIL_CalcFrameSize(pstFrame->u32Channels, pstFrame->s32BitPerSample);
		;
		s32Len = mt_drv_file_write(pAistate->fileHandle, (mt_u8 *)pAistate->stAiBuf.u32KernelVirBaseAddr, pstFrame->u32PcmSamplesPerFrame * u32FrameSize);
		printk("%s, %d\n", __FUNCTION__, __LINE__);
		if (s32Len != pstFrame->u32PcmSamplesPerFrame * u32FrameSize) {
		    MT_ERR_AI("mt_drv_file_write failed!\n");
		    pAistate->enSaveState = AI_CMD_CTRL_STOP;
		    mt_drv_file_close(pAistate->fileHandle);
		    pAistate->fileHandle = MT_NULL;
		}
	    }
	}
    }
    return;
}

static mt_s32 AI_AcquireFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_u32 u32ReadBytes, u32NeedBytes, u32DataBytes, u32FrameSize, u32QurBufDataCnt = 0;
    mt_u32 u32Chn = 0, u32Bit = 0;
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("AI chn is not open,can not get frame!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    state->stAiProc.u32AqcTryCnt++;

    if (AI_CHANNEL_STATUS_STOP == state->enCurnStatus) {
	MT_WARN_AI("current state is stop,can not get frame!\n");
	return MT_FAILURE;
    }

    AIGetChannelsAndBitDepth(state->enAiPort, &state->stSndPortAttr, &u32Chn, &u32Bit);

    if (!state->stAiBuf.u32Read) //do not need read data from port buff
    {
	pstFrame->bInterleaved = MT_TRUE;

	if (MT_UNF_I2S_BIT_DEPTH_16 == u32Bit) {
	    pstFrame->s32BitPerSample = 16;
	} else {
	    pstFrame->s32BitPerSample = 32;
	}
	pstFrame->u32Channels = u32Chn;
	pstFrame->u32PcmSamplesPerFrame = state->stSndPortAttr.u32PcmSamplesPerFrame;
	pstFrame->u32SampleRate = state->stSndPortAttr.enSampleRate;

	AISavePcmData(state, pstFrame);
	//printk("  %s, %d, AISavePcmData\n",__FUNCTION__,__LINE__);

	return MT_SUCCESS;
    }

    u32FrameSize = AUTIL_CalcFrameSize(u32Chn, u32Bit);

    if(0 == reg_audin_aria_get_buf_mode_interlace_mode())
       u32NeedBytes = state->stSndPortAttr.u32PcmSamplesPerFrame * (u32Bit / 8);
    else
       u32NeedBytes = state->stSndPortAttr.u32PcmSamplesPerFrame * (u32Bit / 8) * 2;


    while (1) {
	u32QurBufDataCnt++;

	if (MT_TRUE == state->bAttach) {
	    u32DataBytes = HAL_AIAO_P_QueryBufData_ProvideRptr(state->enPort, &state->u32Rptr);
	    printk("%s, %d, this way need debug more\n", __FUNCTION__, __LINE__);
	} else { //this way
	    u32DataBytes = HAL_AIAO_P_QueryBufData(state->enPort);
	}

	if (u32DataBytes > u32NeedBytes) {
	    break;
	}

	if (u32QurBufDataCnt > AI_QUERY_BUF_CNT_MAX) {
	    MT_WARN_AI("Query BufData time out! %d %d\n", u32DataBytes, u32NeedBytes);
	    return MT_FAILURE;
	}
	msleep(1);
    }

    //printk("AI ACQUIRE FRAME OK %d %d state->bAttach %d\n", u32DataBytes, u32NeedBytes, state->bAttach);

    if (MT_TRUE == state->bAttach) {
	u32ReadBytes = HAL_AIAO_P_ReadData_NotUpRptr(state->enPort, (mt_u8 *)state->stAiBuf.u32KernelVirBaseAddr, u32NeedBytes, &state->u32Rptr, &state->u32Wptr);
    } else {
	//this way
	u32ReadBytes = HAL_AIAO_P_ReadData_Vsb(state->enPort, (mt_u8 *)state->stAiBuf.u32KernelVirBaseAddr/*2ch*/, u32NeedBytes);
	//printk("func %s, line %d  dstbuf 0x%x\n",__FUNCTION__,__LINE__,state->stAiBuf.u32KernelVirBaseAddr);
    }

    if (u32ReadBytes != u32NeedBytes) {
	printk("Read Port Data Error!\n");
	return MT_FAILURE;
    }

    pstFrame->bInterleaved = MT_TRUE;
    if (MT_UNF_I2S_BIT_DEPTH_16 == u32Bit) {
	pstFrame->s32BitPerSample = 16;
    } else {
	pstFrame->s32BitPerSample = 32;
    }
    pstFrame->u32Channels = u32Chn;
    pstFrame->u32PcmSamplesPerFrame = state->stSndPortAttr.u32PcmSamplesPerFrame;
    pstFrame->u32SampleRate = state->stSndPortAttr.enSampleRate;

    state->stAiBuf.u32Write = u32ReadBytes;
    state->stAiBuf.u32Read = 0;
    state->stAiProc.u32AqcCnt++;

    AISavePcmData(state, pstFrame);

    return MT_SUCCESS;
}

static mt_s32 AI_ReleaseFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    //mt_u32 u32DataBytes, u32UpRptrBytes, u32FrameSize;

    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];
    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_FAILURE;
    }

    if (state->stAiBuf.u32Read) {
	MT_ERR_AI("have not acquired frame,can not release frame!\n");
	return MT_FAILURE;
    }

    state->stAiProc.u32RelTryCnt++;
    state->stAiBuf.u32Read = state->stAiBuf.u32Read + pstFrame->u32PcmSamplesPerFrame * pstFrame->s32BitPerSample / 8; //*pstFrame->u32Channels
    //printk("%s, %d :update stAiBUf.u32Read 0x%x\n",__FUNCTION__,__LINE__,state->stAiBuf.u32Read);

    state->stAiProc.u32RelCnt++;
    return MT_SUCCESS;
}

static mt_s32 AI_GetAiBufInfo(mt_handle hAi, AI_BUF_ATTR_S *pstAiBuf)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	printk("AI chn is not open,can not get frame!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    pstAiBuf->u32PhyBaseAddr = state->stAiBuf.u32PhyBaseAddr;
    pstAiBuf->u32Size = state->stAiBuf.u32Size;
    pstAiBuf->u32KernelVirBaseAddr = state->stAiBuf.u32KernelVirBaseAddr;
    pstAiBuf->u32Read = state->stAiBuf.u32Read;
    pstAiBuf->u32Write = state->stAiBuf.u32Write;
    pstAiBuf->u32UserVirBaseAddr = state->stAiBuf.u32UserVirBaseAddr;

    //printk("\n\n%s %d : phyaddr 0x%x,size 0x%x ,viraddr 0x%x,read 0x%x,write 0x%x, baseaddr 0x%x\n",__FUNCTION__,__LINE__,
    //	pstAiBuf->u32PhyBaseAddr,pstAiBuf->u32Size,pstAiBuf->u32KernelVirBaseAddr, pstAiBuf->u32Read,pstAiBuf->u32Write,
    //	 pstAiBuf->u32UserVirBaseAddr);

    return MT_SUCCESS;
}

static mt_s32 AI_SetAiBufInfo(mt_handle hAi, AI_BUF_ATTR_S *pstAiBuf)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("AI chn is not open,can not get frame!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    MT_ERR_AI("minnan debug AI_SetAiBufInfo 0x%x!\n", pstAiBuf->u32UserVirBaseAddr);
    state->stAiBuf.u32UserVirBaseAddr = pstAiBuf->u32UserVirBaseAddr;

    return MT_SUCCESS;
}

mt_s32 AI_GetPortBuf(mt_handle hAi, AIAO_RBUF_ATTR_S *pstAiaoBuf)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    CHECK_AI_CHN_OPEN(hAi);
    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("AI chn is not open,can not get frame!\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    return HAL_AIAO_P_GetRbfAttr(state->enPort, pstAiaoBuf);
}

mt_s32 AI_SetAttachFlag(mt_handle hAi, mt_handle hTrack, MT_BOOL bAttachFlag)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;

    CHECK_AI_CHN_OPEN(hAi);
    hAi &= AI_CHNID_MASK;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hAi];

    if (MT_NULL == state) {
	MT_ERR_AI("AI chn is not open,can not be attached!\n");
	return MT_ERR_AI_INVALID_PARA;
    }
    if (MT_TRUE == bAttachFlag) {

	if ((MT_TRUE == state->bAttach) && (state->hTrack != hTrack)) {
	    MT_ERR_AI("AI is attached ,can not be attached again!\n");
	    return MT_FAILURE;
	}
	state->bAttach = MT_TRUE;
	state->hTrack = hTrack;
    } else {
	if ((MT_TRUE != state->bAttach) || (state->hTrack != hTrack)) {
	    MT_ERR_AI("track(0x%x) is not attach this AI channel, can not detach!\n", hTrack);
	    return MT_FAILURE;
	}
	state->bAttach = MT_FALSE;
	state->hTrack = MT_INVALID_HANDLE;
    }

    return MT_SUCCESS;
}

static mt_s32 AI_AllocHandle(mt_handle *phHandle, struct file *pstFile, MT_UNF_AI_E enAiPort,
                             MT_BOOL bAlseUse, MT_UNF_AI_ATTR_S *pstAiAttr)
{
    mt_u32 i;
    mt_s32 Ret;
    mt_char szName[16];
    AI_CHANNEL_STATE_S *state;
    mt_u32 u32Channel = MT_UNF_I2S_CHNUM_2;
    mt_u32 u32BitDepth = MT_UNF_I2S_BIT_DEPTH_16;
    mt_u32 u32FrameSize = 0, u32AiBufSize = 0;

    mt_u32 u32framesz_p_ch = 0, u32len_audbuf = 0;

    if (!phHandle || !pstAiAttr) {
	MT_ERR_AI("Bad param!\n");
	goto err0;
    }

    if (MT_FALSE == AICheckPortValid(enAiPort)) {
	goto err0;
    }

    if (MT_FALSE == AICheckPortUsed(enAiPort)) {
	goto err0;
    }

    Ret = AICheckAttr(enAiPort, pstAiAttr);
    if (MT_SUCCESS != Ret) {
	MT_ERR_AI("invalid ai attr!\n");
	goto err0;
    }

    /* Allocate new channel */
    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if (NULL == g_pstGlobalAIRS.pstAI_ATTR_S[i]) {
	    break;
	}
    }

    if (i >= AI_MAX_TOTAL_NUM) {
	MT_ERR_AI("Too many Ai channel!\n");
	goto err0;
    }

    state = MT_KMALLOC(MT_ID_AI, sizeof(AI_CHANNEL_STATE_S), GFP_KERNEL);
    if (state == MT_NULL) {
	MT_FATAL_AI("MT_KMALLOC AI_Create failed\n");
	goto err0;
    }
    memset(state, 0, sizeof(AI_CHANNEL_STATE_S));

    if (MT_FALSE == bAlseUse) {
	AIGetChannelsAndBitDepth(enAiPort, pstAiAttr, &u32Channel, &u32BitDepth);

	u32FrameSize = AUTIL_CalcFrameSize(u32Channel, u32BitDepth);
	u32AiBufSize = pstAiAttr->u32PcmSamplesPerFrame * u32FrameSize; //960*4 = 3840

	///////ppbuf and pcm buf

	snprintf(szName, sizeof(szName), "AI_PP_ChnBuf%d", i);
	printk("%s %d u32BitDepth %d  u32FrameSize %d \n", __FUNCTION__, __LINE__, u32BitDepth, u32FrameSize);

	u32framesz_p_ch = pstAiAttr->u32PcmSamplesPerFrame * (u32BitDepth / 8); //frame_size * 2(16bit)
	u32len_audbuf = u32framesz_p_ch * pstAiAttr->u32PcmFrameMaxNum * 8 * 2; //frame_size * 2(16bit) * 6(frame) * 8 (channel) * 2(ppbuf + pcmbuf)

	Ret = mt_drv_mmz_alloc_and_map(szName, MMZ_OTHERS, u32framesz_p_ch * u32Channel /*u32AiBufSize*/, AIAO_BUFFER_ADDR_ALIGN, &state->stAiRbfMmz);
	if (MT_SUCCESS != Ret) {
	    MT_FATAL_AI("MT_MMZ AI_BUF failed, AllocSize(%d)\n", u32AiBufSize);
	    goto err1;
	}
	state->stAiBuf.u32PhyBaseAddr = state->stAiRbfMmz.startPhyAddr;
	state->stAiBuf.u32Size = state->stAiRbfMmz.size; //one frame data of two channel
	state->stAiBuf.u32KernelVirBaseAddr = (ulong)state->stAiRbfMmz.startVirAddr;

	state->stAiBuf.u32UserVirBaseAddr = 0;
	state->stAiBuf.u32Read = u32framesz_p_ch; //samplecnt * bitdepth  //state->stAiRbfMmz.u32Size;  //verify standby
	state->stAiBuf.u32Write = 0;

	printk("\n\n%s %d : phyaddr 0x%llx,size 0x%x ,viraddr 0x%lx,read 0x%x,write 0x%x, baseaddr 0x%lx\n", __FUNCTION__, __LINE__,
	       state->stAiBuf.u32PhyBaseAddr, state->stAiBuf.u32Size, state->stAiBuf.u32KernelVirBaseAddr, state->stAiBuf.u32Read,
	       state->stAiBuf.u32Write,
	       state->stAiBuf.u32UserVirBaseAddr);
#if 1
	// u32BufSize = pstAiAttr->u32PcmSamplesPerFrame * u32FrameSize * pstAiAttr->u32PcmFrameMaxNum;
	snprintf(szName, sizeof(szName), "AI_I2sBuf%d", i);
	Ret = mt_drv_mmz_alloc_and_map(szName, MMZ_OTHERS, u32len_audbuf, AIAO_BUFFER_ADDR_ALIGN, &state->stRbfMmz);
	if (MT_SUCCESS != Ret) {
	    MT_FATAL_AI("MT_MMZ AI_PORT_BUF failed, AllocSize(%d)\n", u32len_audbuf);
	    goto err2;
	}
#endif
    }

    state->u32File = (ulong)pstFile;
    g_pstGlobalAIRS.pstAI_ATTR_S[i] = state;

    *phHandle = (MT_ID_AI << 16) | i;

    return MT_SUCCESS;

err2:
    mt_drv_mmz_unmap_and_release(&state->stAiRbfMmz);
err1:
    MT_KFREE(MT_ID_AI, state);
err0:
    return MT_FAILURE;
}

static mt_void AI_FreeHandle(mt_handle hHandle)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;
    hHandle &= AI_CHNID_MASK;

    if (MT_NULL == g_pstGlobalAIRS.pstAI_ATTR_S[hHandle]) {
	return;
    }

    state = g_pstGlobalAIRS.pstAI_ATTR_S[hHandle];

    if (MT_FALSE == state->bAlsa) {
	     if (state->stRbfMmz.startPhyAddr)
	        mt_drv_mmz_unmap_and_release(&state->stRbfMmz);

			 mt_drv_mmz_unmap_and_release(&state->stAiRbfMmz);
    }
    MT_KFREE(MT_ID_AI, state);
    g_pstGlobalAIRS.pstAI_ATTR_S[hHandle] = NULL;

    return;
}

static mt_s32 AI_OpenDev(mt_void)
{
    mt_u32 i;

    g_pstGlobalAIRS.u32BitFlag_AI = 0;

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	g_pstGlobalAIRS.pstAI_ATTR_S[i] = NULL;
    }

    HAL_AIAO_Init();

    return MT_SUCCESS;
}

static mt_s32 AI_CloseDev(mt_void)
{
    mt_u32 i;

    g_pstGlobalAIRS.u32BitFlag_AI = 0;

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	g_pstGlobalAIRS.pstAI_ATTR_S[i] = NULL;
    }

    HAL_AIAO_DeInit();

    return MT_SUCCESS;
}

/************************************************************************/
static mt_s32 AI_ProcessCmd(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 Ret = MT_SUCCESS;
    mt_handle hHandle = MT_INVALID_HANDLE;

    switch (cmd) {
    case CMD_AI_GEtDEFAULTATTR: {
	AI_GetDfAttr_Param_S_PTR pstAiDfAttr = (AI_GetDfAttr_Param_S_PTR)arg;
	Ret = AI_GetDefaultAttr(pstAiDfAttr->enAiPort, &pstAiDfAttr->stAttr);
	break;
    }
    case CMD_AI_CREATE: {
	AI_Create_Param_S_PTR pstAi = (AI_Create_Param_S_PTR)arg;
	printk("CMD_AI_CREATE %s %d\n", __FUNCTION__, __LINE__);

	if (MT_SUCCESS == AI_AllocHandle(&hHandle, file, pstAi->enAiPort, pstAi->bAlsaUse, &pstAi->stAttr)) {
	    Ret = AI_Create(pstAi->enAiPort, &pstAi->stAttr, pstAi->bAlsaUse, pstAi->pAlsaPara, hHandle);
	    printk("CMD_AI_CREATE %s %d result %d \n", __FUNCTION__, __LINE__, Ret);

	    if (MT_SUCCESS != Ret) {
		AI_FreeHandle(hHandle);
		break;
	    }

	    pstAi->hAi = hHandle;
	} else {
	    Ret = MT_FAILURE;
	}
	break;
    }
    case CMD_AI_DESTROY: {
	mt_handle hAi = *(mt_handle *)arg;
	CHECK_AI_CHN_OPEN(hAi);

	Ret = AI_Destory(hAi);
	if (MT_SUCCESS != Ret) {
	    break;
	}

	AI_FreeHandle(hAi);
	break;
    }

    case CMD_AI_SETENABLE: {
	AI_Enable_Param_S_PTR pstAiEnable = (AI_Enable_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiEnable->hAi);

	Ret = AI_SetEnable(pstAiEnable->hAi, pstAiEnable->bAiEnable, MT_FALSE);
	break;
    }

    case CMD_AI_GETENABLE: {
	AI_Enable_Param_S_PTR pstAiEnable = (AI_Enable_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiEnable->hAi);

	Ret = AI_GetEnable(pstAiEnable->hAi, &pstAiEnable->bAiEnable);
	break;
    }

    case CMD_AI_ACQUIREFRAME: {
	AI_Frame_Param_S_PTR pstAiFrame = (AI_Frame_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiFrame->hAi);

	Ret = AI_AcquireFrame(pstAiFrame->hAi, &pstAiFrame->stAiFrame);
	break;
    }

    case CMD_AI_RELEASEFRAME: {
	AI_Frame_Param_S_PTR pstAiFrame = (AI_Frame_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiFrame->hAi);

	Ret = AI_ReleaseFrame(pstAiFrame->hAi, &pstAiFrame->stAiFrame);
	break;
    }

    case CMD_AI_SETATTR: {
	AI_Attr_Param_S_PTR pstAiAttr = (AI_Attr_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiAttr->hAi);

	Ret = AI_SetAttr(pstAiAttr->hAi, &pstAiAttr->stAttr);
	break;
    }

    case CMD_AI_GETATTR: {
	AI_Attr_Param_S_PTR pstAiAttr = (AI_Attr_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiAttr->hAi);

	Ret = AI_GetAttr(pstAiAttr->hAi, &pstAiAttr->stAttr);
	break;
    }

    case CMD_AI_GETBUFINFO: {
	AI_Buf_Param_S_PTR pstAiBufInfo = (AI_Buf_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiBufInfo->hAi);

	//printk("\n\n%s %d \n",__FUNCTION__,__LINE__);

	Ret = AI_GetAiBufInfo(pstAiBufInfo->hAi, &pstAiBufInfo->stAiBuf);
	//printk("\n\n%s %d \n",__FUNCTION__,__LINE__);

	break;
    }

    case CMD_AI_SETBUFINFO: {
	AI_Buf_Param_S_PTR pstAiBufInfo = (AI_Buf_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstAiBufInfo->hAi);

	Ret = AI_SetAiBufInfo(pstAiBufInfo->hAi, &pstAiBufInfo->stAiBuf);
	break;
    }

    case CMD_AI_SETDELAYCOMPS: {
	AI_DelayComps_Param_S_PTR pstDelayComps = (AI_DelayComps_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstDelayComps->hAi);

	Ret = AI_SetDelayComps(pstDelayComps->hAi, &pstDelayComps->stDelayComps);
	break;
    }

    case CMD_AI_GETDELAYCOMPS: {
	AI_DelayComps_Param_S_PTR pstDelayComps = (AI_DelayComps_Param_S_PTR)arg;

	CHECK_AI_CHN_OPEN(pstDelayComps->hAi);

	Ret = AI_GetDelayComps(pstDelayComps->hAi, &pstDelayComps->stDelayComps);
	break;
    }

    default: {
	Ret = MT_ERR_AI_INVALID_PARA;
	MT_WARN_AI("unknown cmd: 0x%x\n", cmd);
	break;
    }
    }

    return Ret;
}

long AI_DRV_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg)
{
    long Ret;

    Ret = down_interruptible(&g_AIMutex);

    //cmd process
    Ret = (long)AI_ProcessCmd(file, cmd, (mt_void *)arg);

    up(&g_AIMutex);
    return Ret;
}

mt_s32 AI_DRV_Open(struct inode *inode, struct file *filp)
{
    mt_s32 Ret;
    MT_S32 mt_idx = iminor(inode);
    AI_PRIV_DATA_S *ai_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(ai_priv_data->audinclk))
        clk_prepare_enable(ai_priv_data->audinclk);
    if (!IS_ERR_OR_NULL(ai_priv_data->audinaxiclk))
        clk_prepare_enable(ai_priv_data->audinaxiclk);

    Ret = down_interruptible(&g_AIMutex);

    if (atomic_inc_return(&g_AIOpenCnt) == 1) {
	if (MT_SUCCESS != AI_OpenDev()) {
	    MT_FATAL_AI("AI_OpenDev err!\n");
	    up(&g_AIMutex);
	    return MT_FAILURE;
	}
    }

    up(&g_AIMutex);
    return MT_SUCCESS;
}

mt_s32 AI_DRV_Release(struct inode *inode, struct file *filp)
{
    mt_s32 Ret, i;
    mt_handle hAi;
    MT_S32 mt_idx = iminor(inode);
    AI_PRIV_DATA_S *ai_priv_data = get_mt_priv(mt_idx);

    Ret = down_interruptible(&g_AIMutex);

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((NULL != g_pstGlobalAIRS.pstAI_ATTR_S[i]) && (((ulong)filp)) == g_pstGlobalAIRS.pstAI_ATTR_S[i]->u32File) {
	    hAi = (mt_handle)i;

	    if (MT_SUCCESS != AI_Destory(hAi)) {
		MT_ERR_AI("AI_Destory err! hAi is %d\n", hAi);
	    }

	    AI_FreeHandle(hAi);
	}
    }

    if (atomic_dec_and_test(&g_AIOpenCnt)) {
	if (MT_SUCCESS != AI_CloseDev()) {
	    MT_FATAL_AI("AI_CloseDev err!\n");
	}
    }

    up(&g_AIMutex);

    if (!IS_ERR_OR_NULL(ai_priv_data->audinclk))
        clk_disable_unprepare(ai_priv_data->audinclk);
    if (!IS_ERR_OR_NULL(ai_priv_data->audinaxiclk))
        clk_disable_unprepare(ai_priv_data->audinaxiclk);

    return MT_SUCCESS;
}

mt_s32 AI_WriteProc(mt_u32 u32Ai, AI_CMD_CTRL_E enCmd)
{
    AI_CHANNEL_STATE_S *state = MT_NULL;
    mt_char szPath[AI_PATH_NAME_MAXLEN + AI_FILE_NAME_MAXLEN] = { 0 };
    struct tm now;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[u32Ai];
    if (!state) {
	MT_ERR_AI("this AI chn is not open!\n");
	return MT_FAILURE;
    }
    if (AI_CMD_CTRL_START == enCmd && AI_CMD_CTRL_STOP == state->enSaveState) {

	if (MT_SUCCESS != mt_drv_file_get_storepath(szPath, AI_PATH_NAME_MAXLEN)) {
	    MT_ERR_AI("get store path failed\n");
	    return MT_FAILURE;
	}
	time64_to_tm(ktime_get_seconds(), 0, &now);

	snprintf(szPath, sizeof(szPath), "%s/ai%d_%02u_%02u_%02u.pcm", szPath, u32Ai, now.tm_hour, now.tm_min, now.tm_sec);
	state->fileHandle = mt_drv_file_open(szPath, 1);
	if (!state->fileHandle) {
	    MT_ERR_AI("open %s error\n", szPath);
	    return MT_FAILURE;
	}
	state->u32SaveCnt++;
    }

    if (AI_CMD_CTRL_STOP == enCmd && AI_CMD_CTRL_START == state->enSaveState) {
	if (state->fileHandle) {
	    mt_drv_file_close(state->fileHandle);
	    state->fileHandle = MT_NULL;
	}
    }

    state->enSaveState = enCmd;

    return MT_SUCCESS;
}

static mt_void AIShowSpecAttr(struct seq_file *p, AI_CHANNEL_STATE_S *state)
{
    MT_UNF_AI_ADC_ATTR_S stAdcAttr;
    MT_UNF_AI_I2S_ATTR_S stI2sAttr;
    MT_UNF_AI_HDMI_ATTR_S stHDMIAttr;

    switch (state->enAiPort) {
    case MT_UNF_AI_I2S0:
    case MT_UNF_AI_I2S1:
	stI2sAttr = state->stSndPortAttr.unAttr.stI2sAttr;
	PROC_PRINT(p,
	           "Channel                              :%d\n"
	           "BitWidth                             :%d\n"
	           "ClkMode                              :%s\n"
	           "I2sMode                              :%s\n"
	           "Mclk/Fs                              :%d\n"
	           "Mclk/Bclk                            :%d\n"
	           "SampleEdge                           :%s\n"
	           "DelayCycle                           :%d\n",
	           stI2sAttr.stAttr.enChannel,
	           stI2sAttr.stAttr.enBitDepth,
	           (MT_TRUE == stI2sAttr.stAttr.bMaster) ? "Master" : "Slave",
	           (MT_UNF_I2S_STD_MODE == stI2sAttr.stAttr.enI2sMode) ? "Standard" : "Pcm",
	           AUTIL_MclkFclkDiv(stI2sAttr.stAttr.enMclkSel),
	           stI2sAttr.stAttr.enBclkSel,
	           (MT_TRUE == stI2sAttr.stAttr.bPcmSampleRiseEdge) ? "Positive" : "Negative",
	           stI2sAttr.stAttr.enPcmDelayCycle);
	break;
    case MT_UNF_AI_HDMI0:
    case MT_UNF_AI_HDMI1:
    case MT_UNF_AI_HDMI2:
    case MT_UNF_AI_HDMI3:
	stHDMIAttr = state->stSndPortAttr.unAttr.stHDMIAttr;
	PROC_PRINT(p,
	           "Channel                              :%d\n"
	           "BitWidth                             :%d\n"
	           "Format                               :%s\n",
	           stHDMIAttr.enChannel,
	           stHDMIAttr.enBitDepth,
	           (MT_UNF_AI_HDMI_FORMAT_LPCM == stHDMIAttr.enHdmiAudioDataFormat) ? "Pcm" : ((MT_UNF_AI_HDMI_FORMAT_LBR == stHDMIAttr.enHdmiAudioDataFormat) ? "LBR" : "HBR"));
	break;
    case MT_UNF_AI_ADC0:
    case MT_UNF_AI_ADC1:
    case MT_UNF_AI_ADC2:
    case MT_UNF_AI_ADC3:
    case MT_UNF_AI_ADC4:
	stAdcAttr = state->stSndPortAttr.unAttr.stAdcAttr;
	PROC_PRINT(p,
	           "Bypass                               :%s\n",
	           (MT_TRUE == stAdcAttr.bByPass) ? "On" : "Off");
	break;
    default:
	break;
    }

    return;
}

static mt_s32 AI_ShowChnProc(struct seq_file *p, mt_u32 u32Chn)
{
    mt_s32 Ret;
    mt_u32 u32BufSizeUsed, u32BufPerCentUsed;
    AIAO_PORT_ID_E enPort;
    AIAO_PORT_STAUTS_S pstPortStatus;
    AI_CHANNEL_STATE_S *state = MT_NULL;

    state = g_pstGlobalAIRS.pstAI_ATTR_S[u32Chn];

    enPort = state->enPort;

    memset(&pstPortStatus, 0, sizeof(AIAO_PORT_STAUTS_S));
    Ret = HAL_AIAO_P_GetStatus(enPort, &pstPortStatus);
    if (MT_SUCCESS != Ret) {
	return Ret;
    }

    if (*(pstPortStatus.stCircBuf.pu32Write) >= *(pstPortStatus.stCircBuf.pu32Read)) {
	u32BufSizeUsed = *(pstPortStatus.stCircBuf.pu32Write) - *(pstPortStatus.stCircBuf.pu32Read);
    } else {
	u32BufSizeUsed = pstPortStatus.stCircBuf.u32Lenght - (*(pstPortStatus.stCircBuf.pu32Read) - *(pstPortStatus.stCircBuf.pu32Write));
    }

    u32BufPerCentUsed = u32BufSizeUsed * 100 / pstPortStatus.stCircBuf.u32Lenght;

    PROC_PRINT(p,
               "\n--------------------- AI%d[%s] Status ---------------------\n",
               u32Chn,
               AUTIL_AiPort2Name(state->enAiPort));
    PROC_PRINT(p,
               "Status                               :%s\n",
               (mt_char *)((AIAO_PORT_STATUS_START == pstPortStatus.enStatus) ? "start" : ((AIAO_PORT_STATUS_STOP == pstPortStatus.enStatus) ? "stop" : "stopping")));
    PROC_PRINT(p,
               "SampleRate                           :%d\n",
               state->stSndPortAttr.enSampleRate);
    PROC_PRINT(p,
               "PcmFrameMaxNum                       :%d\n",
               state->stSndPortAttr.u32PcmFrameMaxNum);
    PROC_PRINT(p,
               "PcmSamplesPerFrame                   :%d\n",
               state->stSndPortAttr.u32PcmSamplesPerFrame);

    AIShowSpecAttr(p, state);

    PROC_PRINT(p,
               "*AiPort                              :0x%.2x\n",
               enPort);
    PROC_PRINT(p,
               "*Alsa                                :%s\n\n",
               (state->bAlsa == MT_TRUE) ? "Yes" : "No");
    PROC_PRINT(p,
               "DmaCnt                               :%d\n",
               pstPortStatus.stProcStatus.uDMACnt);
    PROC_PRINT(p,
               "BufFullCnt                           :%d\n",
               pstPortStatus.stProcStatus.uBufFullCnt);
    PROC_PRINT(p,
               "FiFoFullCnt                          :%d\n",
               pstPortStatus.stProcStatus.uInfFiFoFullCnt);
    PROC_PRINT(p,
               "FrameBuf(Total/Use/Percent)(Bytes)   :%d/%d/%d%%\n",
               pstPortStatus.stBuf.u32BUFF_SIZE, u32BufSizeUsed, u32BufPerCentUsed);
    PROC_PRINT(p,
               "AcquireFrame(Try/OK)                 :%d/%d\n",
               state->stAiProc.u32AqcTryCnt, state->stAiProc.u32AqcCnt);
    PROC_PRINT(p,
               "ReleaseFrame(Try/OK)                 :%d/%d\n\n",
               state->stAiProc.u32RelTryCnt, state->stAiProc.u32RelCnt);

    return MT_SUCCESS;
}

mt_s32 AI_DRV_ReadProc(struct seq_file *p, mt_void *v)
{
    mt_s32 i;

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if (g_pstGlobalAIRS.pstAI_ATTR_S[i]) {
	    AI_ShowChnProc(p, i);
	}
    }

    return MT_SUCCESS;
}

mt_s32 AI_DRV_WriteProc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    mt_s32 s32Ret;
    mt_u32 u32Ai;

    AI_CMD_PROC_E enProcCmd;
    AI_CMD_CTRL_E enCtrlCmd;

    mt_char szBuf[48];
    mt_char *pcBuf = szBuf;
    mt_char *pcStartCmd = "start";
    mt_char *pcStopCmd = "stop";
    mt_char *pcSaveAICmd = "save";

    mt_char *pcHelpCmd = "help";
    AI_CHANNEL_STATE_S *state = MT_NULL;

    struct seq_file *p = file->private_data;
    mt_proc_entry_t *pstProcItem = p->private;

    s32Ret = down_interruptible(&g_AIMutex);
    if (copy_from_user(szBuf, buf, count)) {
	MT_ERR_AI("copy from user failed\n");
	up(&g_AIMutex);
	return MT_FAILURE;
    }

    (mt_void) sscanf(pstProcItem->entry_name, "ai%1d", &u32Ai);
    if (u32Ai >= AI_MAX_TOTAL_NUM) {
	MT_ERR_AI("Invalid Ai ID:%d.\n", u32Ai);
	goto SAVE_CMD_FAULT;
    }

    state = g_pstGlobalAIRS.pstAI_ATTR_S[u32Ai];
    if (MT_NULL == state) {
	MT_ERR_AI("this AI chn is not open!\n");
	goto SAVE_CMD_FAULT;
    }

    AI_STRING_SKIP_BLANK(pcBuf);
    if (strstr(pcBuf, pcSaveAICmd)) {
	enProcCmd = AI_CMD_PROC_SAVE_AI;
	pcBuf += strlen(pcSaveAICmd);
    } else if (strstr(pcBuf, pcHelpCmd)) {
	AI_PROC_SHOW_HELP(u32Ai);
	up(&g_AIMutex);
	return count;
    } else {
	goto SAVE_CMD_FAULT;
    }
    AI_STRING_SKIP_BLANK(pcBuf);

    if (AI_CMD_PROC_SAVE_AI == enProcCmd) {
	if (strstr(pcBuf, pcStartCmd)) {
	    enCtrlCmd = AI_CMD_CTRL_START;
	} else if (strstr(pcBuf, pcStopCmd)) {
	    enCtrlCmd = AI_CMD_CTRL_STOP;
	} else {
	    goto SAVE_CMD_FAULT;
	}
	s32Ret = AI_WriteProc(u32Ai, enCtrlCmd);
	if (s32Ret != MT_SUCCESS) {
	    goto SAVE_CMD_FAULT;
	}
    }
    up(&g_AIMutex);
    return count;

SAVE_CMD_FAULT:

    MT_ERR_AI("proc cmd is fault\n");
    AI_PROC_SHOW_HELP(u32Ai);
    up(&g_AIMutex);

    return MT_FAILURE;
}

static mt_s32 AI_RegProc(mt_u32 u32Ai)
{
    mt_char aszBuf[16];
    mt_proc_entry_t *pProcItem;

    /* Check parameters */
    if (MT_NULL == g_pstGlobalAIRS.pstProcParam) {
	return MT_FAILURE;
    }

    /* Create proc */
    snprintf(aszBuf, sizeof(aszBuf), "ai%d", u32Ai);
    pProcItem = mt_drv_proc_add_module(aszBuf, MT_NULL, MT_NULL);
    if (!pProcItem) {
	MT_FATAL_AO("Create ai proc entry fail!\n");
	return MT_FAILURE;
    }

    /* Set functions */
    pProcItem->read = g_pstGlobalAIRS.pstProcParam->pfnReadProc;
    pProcItem->write = g_pstGlobalAIRS.pstProcParam->pfnWriteProc;

    MT_INFO_AO("Create Ai proc entry for OK!\n");
    return MT_SUCCESS;
}

static mt_void AI_UnRegProc(mt_u32 u32Ai)
{
    mt_char aszBuf[16];
    snprintf(aszBuf, sizeof(aszBuf), "ai%d", u32Ai);

    mt_drv_proc_rm_module(aszBuf);
}

mt_s32 AI_DRV_RegisterProc(AI_REGISTER_PARAM_S *pstParam)
{
    /* Check parameters */
    if (MT_NULL == pstParam) {
	return MT_FAILURE;
    }

    g_pstGlobalAIRS.pstProcParam = pstParam;

    /* Create proc when use*/

    return MT_SUCCESS;
}

mt_void AI_DRV_UnregisterProc(mt_void)
{

    /* Clear param */
    g_pstGlobalAIRS.pstProcParam = MT_NULL;
    return;
}

mt_s32 AI_DRV_Suspend(basedev_s *pdev,
                      pm_message_t state)
{
    AI_PRIV_DATA_S *ai_priv_data = dev_get_platdata(&pdev->dev);

#if defined(MT_AI_DRV_SUSPEND_SUPPORT)
    mt_u32 i;
    mt_s32 s32Ret;
    AI_CHANNEL_STATE_S *pAistate = MT_NULL;

    s32Ret = down_interruptible(&g_AIMutex);
    if (0 != atomic_read(&g_AIOpenCnt)) {
	for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	    if (g_pstGlobalAIRS.pstAI_ATTR_S[i]) {
		pAistate = g_pstGlobalAIRS.pstAI_ATTR_S[i];
		s32Ret = AI_Destory(i);
		if (MT_SUCCESS != s32Ret) {
		    MT_FATAL_AI("AI Destory fail\n");
		    up(&g_AIMutex);
		    return MT_FAILURE;
		}
	    }
	}

	s32Ret = HAL_AIAO_Suspend();
	if (MT_SUCCESS != s32Ret) {
	    MT_FATAL_AI("AIAO Suspend fail\n");
	    up(&g_AIMutex);
	    return MT_FAILURE;
	}
    }
    MT_PRINT("AI suspend OK\n");
#endif
    up(&g_AIMutex);

    if (!IS_ERR_OR_NULL(ai_priv_data->audinclk))
        clk_disable_unprepare(ai_priv_data->audinclk);
    if (!IS_ERR_OR_NULL(ai_priv_data->audinaxiclk))
        clk_disable_unprepare(ai_priv_data->audinaxiclk);

    return MT_SUCCESS;
}

mt_s32 AI_DRV_Resume(basedev_s *pdev)
{
#if defined(MT_AI_DRV_SUSPEND_SUPPORT)
	mt_s32 s32Ret;
	mt_u32 i;
	MT_UNF_AI_E enAiPort;
	MT_UNF_AI_ATTR_S stAiAttr;
	MT_BOOL bAlsa;
	mt_void *pAlsaPara;
	AI_CHANNEL_STATE_S *state = MT_NULL;
	AI_CHANNEL_STATUS_E enAiStatus;
#endif
    AI_PRIV_DATA_S *ai_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(ai_priv_data->audinclk))
        clk_prepare_enable(ai_priv_data->audinclk);
    if (!IS_ERR_OR_NULL(ai_priv_data->audinaxiclk))
        clk_prepare_enable(ai_priv_data->audinaxiclk);

#if defined(MT_AI_DRV_SUSPEND_SUPPORT)

    s32Ret = down_interruptible(&g_AIMutex);

    if (0 != atomic_read(&g_AIOpenCnt)) {
	s32Ret = HAL_AIAO_Resume();
	if (MT_SUCCESS != s32Ret) {
	    MT_FATAL_AI("AIAO Resume fail\n");
	    up(&g_AIMutex);
	    return MT_FAILURE;
	}

	for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	    if (g_pstGlobalAIRS.pstAI_ATTR_S[i]) {
		state = g_pstGlobalAIRS.pstAI_ATTR_S[i];
		enAiPort = state->enAiPort;
		stAiAttr = state->stSndPortAttr;
		bAlsa = state->bAlsa;
		pAlsaPara = state->pAlsaPara;
		enAiStatus = state->enCurnStatus;
#ifdef MT_ALSA_AI_SUPPORT
		if (bAlsa == MT_TRUE) {
		    pAlsaPara = (void *)&g_stAlsaAttr;
		}
#endif
		s32Ret = AI_Create(enAiPort, &stAiAttr, bAlsa, pAlsaPara, i);

		if (MT_SUCCESS != s32Ret) {
		    MT_FATAL_AI("AICreateChn failed\n");
		    up(&g_AIMutex);
		    return MT_FAILURE;
		}

		if ((AI_CHANNEL_STATUS_START == enAiStatus) && (!(state->bAttach))) {
		    s32Ret = AI_SetEnable(i, MT_TRUE, MT_FALSE);
		    if (MT_SUCCESS != s32Ret) {
			MT_ERR_AI("Set AI Enable failed\n");
			up(&g_AIMutex);
			return MT_FAILURE;
		    }
		}
	    }
	}
    }
    up(&g_AIMutex);
    MT_PRINT("AI resume OK\n");
#endif
    return MT_SUCCESS;
}

mt_s32 AI_DRV_Init(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = down_interruptible(&g_AIMutex);
    s32Ret = mt_drv_module_register(MT_ID_AI, AI_NAME, (mt_void *)&g_pstGlobalAIRS.stExtFunc);
    if (MT_SUCCESS != s32Ret) {
	MT_FATAL_AI("Reg Ai module fail:%#x!\n", s32Ret);
	up(&g_AIMutex);
	return s32Ret;
    }

    up(&g_AIMutex);
    return MT_SUCCESS;
}

mt_void AI_DRV_Exit(mt_void)
{
    mt_s32 s32Ret;

    s32Ret = down_interruptible(&g_AIMutex);

    mt_drv_module_unregister(MT_ID_AI);

    up(&g_AIMutex);
    return;
}

mt_s32 MT_DRV_AI_Init(mt_void)
{
    return AI_DRV_Init();
}

mt_void MT_DRV_AI_DeInit(mt_void)
{
    AI_DRV_Exit();
}

mt_s32 MT_DRV_AI_Drv_Open(mt_void)
{
    return AI_DRV_Open(NULL, &g_file);
}

mt_s32 MT_DRV_AI_Drv_Release(mt_void)
{
    return AI_DRV_Release(NULL, &g_file);
}

mt_s32 MT_DRV_AI_SND_GetDefaultOpenAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr)
{
    return AI_GetDefaultAttr(enAiPort, pstAttr);
}

mt_s32 MT_DRV_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phandle)
{
    mt_s32 Ret;
    AI_Create_Param_S stAiParam;
    mt_handle hAi;

    stAiParam.enAiPort = enAiPort;
    stAiParam.bAlsaUse = MT_FALSE;
    stAiParam.pAlsaPara = NULL;

    memcpy(&stAiParam.stAttr, pstAttr, sizeof(MT_UNF_AI_ATTR_S));

    Ret = AI_AllocHandle(&hAi, &g_file, stAiParam.enAiPort, stAiParam.bAlsaUse, &stAiParam.stAttr);
    if (MT_SUCCESS != Ret) {
	MT_ERR_AI("Alloc Ai Handle failed!");
	return Ret;
    }

    Ret = AI_Create(stAiParam.enAiPort, &stAiParam.stAttr, stAiParam.bAlsaUse, stAiParam.pAlsaPara, hAi);
    if (MT_SUCCESS != Ret) {
	MT_ERR_AI("Ai Create failed 0x%x!", Ret);
	return Ret;
    }
    return Ret;
}

mt_s32 MT_DRV_AI_Destroy(mt_handle hAi)
{
    mt_s32 Ret;

    CHECK_AI_CHN_OPEN(hAi);

    Ret = AI_Destory(hAi);
    if (MT_SUCCESS != Ret) {
	return Ret;
    }

    AI_FreeHandle(hAi);

    return Ret;
}

mt_s32 MT_DRV_AI_SetEnable(mt_handle hAi, MT_BOOL bEnable)
{
    CHECK_AI_CHN_OPEN(hAi);
    return AI_SetEnable(hAi, bEnable, MT_FALSE);
}

mt_s32 MT_DRV_AI_GetEnable(mt_handle hAi, MT_BOOL *pbEnable)
{
    CHECK_AI_CHN_OPEN(hAi);
    return AI_GetEnable(hAi, pbEnable);
}

mt_s32 MT_DRV_AI_GetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pstAttr)
{
    CHECK_AI_CHN_OPEN(hAi);
    return AI_GetAttr(hAi, pstAttr);
}

mt_s32 MT_DRV_AI_SetAttr(mt_handle hAi, MT_UNF_AI_ATTR_S *pstAttr)
{
    CHECK_AI_CHN_OPEN(hAi);
    return AI_SetAttr(hAi, pstAttr);
}

mt_s32 MT_DRV_AI_AcquireFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_s32 Ret;
    AI_BUF_ATTR_S stAiBuf;

    CHECK_AI_CHN_OPEN(hAi);

    Ret = AI_GetAiBufInfo(hAi, &stAiBuf);
    if (MT_SUCCESS != Ret) {
        MT_ERR_AI("Alloc Ai Get BufInfo failed!");
        return Ret;
    }
    printk("%s, %d\n", __FUNCTION__, __LINE__);

    Ret = AI_AcquireFrame(hAi, pstFrame);
    if (MT_SUCCESS != Ret) {
	     MT_ERR_AI("Alloc Ai Get AcquireFrame failed!");
	     return Ret;
    }
    pstFrame->ps32PcmBuffer = (ulong *)(stAiBuf.u32KernelVirBaseAddr);
    return Ret;
}

mt_s32 MT_DRV_AI_ReleaseFrame(mt_handle hAi, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    CHECK_AI_CHN_OPEN(hAi);
    return AI_ReleaseFrame(hAi, pstFrame);
}
