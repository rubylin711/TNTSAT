/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_module.h"
#include "mt_module_debug.h"
#include "mt_mpi_mem.h"
#include "mt_drv_struct.h"
#include "mt_error_mpi.h"

#include "mt_mpi_ao.h"
#include "mt_drv_ao.h"
#include "drv_ao_ioctl.h"
#include "mt_mpi_ai.h"

#include "mt_mpi_vir.h"
#include "snd/snd_api.h"

static mt_s32 g_fd_snd = -1;
//static const mt_char g_acAODevName[] = "/dev/" UMAP_DEVNAME_AO;
//static pthread_mutex_t g_AOMutex = PTHREAD_MUTEX_INITIALIZER;

#if defined (MT_SMARTVOLUME_SUPPORT)
#include "smartvol.h"

/* smart sound information */
typedef struct hiAO_CHN_SMARTVOL_S
{
	HSmartVol          hSmartVol;      /* Smart volume Handle */
	mt_s32             s32LastStatus;
	MT_BOOL            bTrackOpen;
	MT_BOOL            bSwitchProgram;
	MT_BOOL            bEnable;
	SamrtVol_In_Para_S stInParam;
} AO_CHN_SMARTVOL_S;

static AO_CHN_SMARTVOL_S g_stSmartVol[AO_MAX_TOTAL_TRACK_NUM] =
{
	{
		MT_NULL, MT_SUCCESS, MT_FALSE, MT_FALSE, MT_FALSE,
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	}
};

static mt_void    SmartVolumeProcess(MT_UNF_AO_FRAMEINFO_S *pstAOFrame, AO_CHN_SMARTVOL_S *stSmartVol)
{
	if (NULL == stSmartVol)
	{
		MT_ERR_AO("Null pointer stSmartVol!\n");
		return;
	}

	if (pstAOFrame->u32PcmSamplesPerFrame == 0)
	{
		MT_ERR_AO("pcm samples per frame is zero!\n");
		return;
	}

	if (pstAOFrame->u32Channels > 2)
	{
		MT_ERR_AO("Smartvolume don't support multichannel(%d)\n", pstAOFrame->u32Channels);
		return;
	}

	if (MT_FALSE == pstAOFrame->bInterleaved)
	{
		MT_ERR_AO("Smartvolume don't support none bInterleaved pcm format\n");
		return;
	}

	SamrtVol_In_Para_P pstInParam = &(stSmartVol->stInParam);
	if (MT_NULL == stSmartVol->hSmartVol)
	{
		HSmartVol tmp;
		(mt_void)MT_SmartGetDefaultConfig(pstInParam);
		pstInParam->framelen = (mt_s32)pstAOFrame->u32PcmSamplesPerFrame;
		pstInParam->samprate = (mt_s32)pstAOFrame->u32SampleRate;
		pstInParam->nchans   = (mt_s32)pstAOFrame->u32Channels;
		pstInParam->bitdepth = (pstAOFrame->s32BitPerSample == 16) ? 16 : 32;
		pstInParam->Interleaved = 1;
		tmp = MT_SmartVolOpen(pstInParam);
		if (MT_NULL != tmp)
		{
			stSmartVol->hSmartVol = tmp;
			(mt_void)MT_SmartVolEnable(tmp);
		}
	}

	if (MT_NULL != stSmartVol->hSmartVol)
	{
		mt_s32 bReset = 0;
		/* check if need to reset samrtvol state */
		//bReset |= (pstAOFrame->u32PcmSamplesPerFrame != pstInParam->framelen);
		//bReset |= (((mt_s32)(pstAOFrame->u32SampleRate)) != pstInParam->samprate);
		//bReset |= (pstAOFrame->u32Channels != pstInParam->nchans);
		//bReset |= (pstAOFrame->bInterleaved != pstInParam->Interleaved);
		if ((pstAOFrame->u32PcmSamplesPerFrame != (mt_u32)pstInParam->framelen) || (pstAOFrame->u32SampleRate != (mt_u32)pstInParam->samprate) || (pstAOFrame->u32Channels != (mt_u32)pstInParam->nchans) || ((mt_s32)pstAOFrame->bInterleaved != pstInParam->Interleaved))
		{
			bReset = 1;
		}

		if ((0 != bReset) || (stSmartVol->bSwitchProgram))
		{
			pstInParam->framelen = (mt_s32)pstAOFrame->u32PcmSamplesPerFrame;
			pstInParam->samprate = (mt_s32)(pstAOFrame->u32SampleRate);
			pstInParam->nchans   = (mt_s32)pstAOFrame->u32Channels;
			pstInParam->bitdepth = (pstAOFrame->s32BitPerSample == 16) ? 16 : 32;
			pstInParam->Interleaved = 1;
			(mt_void)MT_SmartVolClear(stSmartVol->hSmartVol, pstInParam);
			MT_INFO_AO("audio swtich, smartvol trigger\n");
		}

		if (MT_TRUE == stSmartVol->bSwitchProgram)
		{
			stSmartVol->bSwitchProgram = MT_FALSE;
		}

		/* SmartVolume Process */
		(mt_void)MT_SmartVolPro(stSmartVol->hSmartVol, (mt_s32*)pstAOFrame->ps32PcmBuffer,
		                        NULL, 0, (mt_s32)pstAOFrame->u32PcmSamplesPerFrame);
	}
}

mt_s32   MT_MPI_AO_Track_SetSmartVolume(mt_handle hTrack, MT_BOOL bEnable)
{
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);
	mt_s32 s32Ret = MT_FAILURE;

	AO_CHN_SMARTVOL_S *pstSmartVolume;
	pstSmartVolume = &(g_stSmartVol[hTrack & AO_TRACK_CHNID_MASK]);
	if (MT_TRUE == pstSmartVolume->bTrackOpen)
	{
		pstSmartVolume->bEnable = bEnable;
		s32Ret = MT_SUCCESS;
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_Track_GetSmartVolume(mt_handle hTrack, MT_BOOL *pbEnable)
{
	CHECK_AO_NULL_PTR(pbEnable);
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);

	mt_s32 s32Ret = MT_FAILURE;
	AO_CHN_SMARTVOL_S *pstSmartVolume;
	pstSmartVolume = &(g_stSmartVol[hTrack & AO_TRACK_CHNID_MASK]);
	if (MT_TRUE == pstSmartVolume->bTrackOpen)
	{
		*pbEnable = pstSmartVolume->bEnable;
		s32Ret = MT_SUCCESS;
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_Track_TriggerSmartVolume(mt_handle hTrack)
{
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);
	mt_s32 s32Ret = MT_FAILURE;

	AO_CHN_SMARTVOL_S *pstSmartVolume;
	pstSmartVolume = &(g_stSmartVol[hTrack & AO_TRACK_CHNID_MASK]);
	if (MT_TRUE == pstSmartVolume->bTrackOpen)
	{
		pstSmartVolume->bSwitchProgram = MT_TRUE;
		pstSmartVolume->s32LastStatus  = MT_SUCCESS;
		s32Ret = MT_SUCCESS;
	}

	return s32Ret;
}
#endif

mt_s32 MT_MPI_AO_Init(mt_void)
{
	mt_s32 ret;
	AUD_SND_INIT_PARAM param;
    if (g_fd_snd < 0)
    {
		g_fd_snd = open(/*DRV_SND_DEV_NAME*/"/dev/mt_snd", O_RDWR | O_CLOEXEC, 0);
        if (g_fd_snd < 0)
        {
            MT_FATAL_AO("OpenAODevice err\n");
            g_fd_snd = -1;
            return MT_ERR_AO_CREATE_FAIL;
        }
    }
	//snd init, default as master type
	memset((void *)&param, 0, sizeof(AUD_SND_INIT_PARAM));
	param.type = SND_CARD_TYPE_MASTER;
	ret = ioctl(g_fd_snd, SND_INIT, &param);

	printf("<RR>%s:L%d: ret[%x]g_fd_snd[%x]\n", __FUNCTION__, __LINE__, ret, g_fd_snd);
	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_DeInit(mt_void)
{
	mt_s32 ret;

	printf("<RR>%s:L%d: g_fd_snd[%x]\n", __FUNCTION__, __LINE__, g_fd_snd);
	if (0 < g_fd_snd)
	{
		//snd deinit and close
		ret = ioctl(g_fd_snd, SND_UNINIT);
		printf("<RR>%s:L%d: ret[%x]\n", __FUNCTION__, __LINE__, ret);
	}

	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr)
{
	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_SND_Open(MT_UNF_SND_E enSound, const MT_UNF_SND_ATTR_S *pstAttr)
{
	if (g_fd_snd > 0)
	{
	//T_FATAL_AO("<%s:%d> snd already opened!\n");
		return MT_SUCCESS;
	}
	
	g_fd_snd = open(/*DRV_SND_DEV_NAME*/"/dev/mt_snd", O_RDWR | O_CLOEXEC, 0);
	if (g_fd_snd < 0)
	{
		MT_FATAL_AO("open snd device FAIL\n");
		g_fd_snd = -1;
		return MT_ERR_AO_CREATE_FAIL;
	}

	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_SND_Close(MT_UNF_SND_E enSound)
{
	if (g_fd_snd > 0)
	{
		close(g_fd_snd);
		g_fd_snd = -1;
	}

	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_SND_SetAdacOnOff(MT_UNF_SND_E enSound, MT_BOOL bOnOff)
{
	return ioctl(g_fd_snd, SND_ADAC_ONOFF, bOnOff);
}

mt_s32 MT_MPI_AO_SND_SetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute)
{
	snd_mute_parame_t mute_param;

	mute_param.type = enSound;
	mute_param.port = enOutPort;
	mute_param.mute = bMute;

	return ioctl(g_fd_snd, SND_SET_MUTE, &mute_param);
}

mt_s32 MT_MPI_AO_SND_GetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute)
{
	int ret;
	snd_mute_parame_t mute_param;

	mute_param.type = enSound;
	mute_param.port = enOutPort;

	ret = ioctl(g_fd_snd, SND_GET_MUTE, &mute_param);
	if (MT_SUCCESS == ret)
	{
		*pbMute = mute_param.mute;
	}

	return ret;
}

mt_s32 MT_MPI_AO_SND_SetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
								MT_UNF_SND_HDMI_MODE_E enHdmiMode)
{
	return ioctl(g_fd_snd, SND_SET_HDMI_MODE, enHdmiMode);
}

mt_s32 MT_MPI_AO_SND_GetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
								MT_UNF_SND_HDMI_MODE_E *penHdmiMode)
{
	int ret;
	ulong hdmi_mode;

	ret = ioctl(g_fd_snd, SND_GET_HDMI_MODE, &hdmi_mode);
	if (MT_SUCCESS == ret)
	{
		*penHdmiMode = hdmi_mode;
	}

	return ret;
}

mt_s32 MT_MPI_AO_SND_SetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
									MT_UNF_SND_SPDIF_MODE_E enSpdifMode)
{
	return ioctl(g_fd_snd, SND_SET_SPDIF_MODE, enSpdifMode);
}

mt_s32 MT_MPI_AO_SND_GetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
									MT_UNF_SND_SPDIF_MODE_E *penSpdifMode)
{
	int ret;
	ulong spdif_mode;

	ret = ioctl(g_fd_snd, SND_GET_SPDIF_MODE, &spdif_mode);
	if (MT_SUCCESS == ret)
	{
		*penSpdifMode = spdif_mode;
	}

	return ret;
}

mt_s32 MT_MPI_AO_SND_SetVolume(ulong volume)
{
	return ioctl(g_fd_snd, SND_SET_VOL, volume);
}

mt_s32 MT_MPI_AO_SND_GetVolume(ulong *volume)
{
	int ret;

	ret = ioctl(g_fd_snd, SND_GET_VOL, volume);

	return ret;
}

mt_s32 MT_MPI_AO_SND_SetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
											MT_UNF_SND_SPDIF_CATEGORYCODE_E enSpdifCategoryCode)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_GetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
											MT_UNF_SND_SPDIF_CATEGORYCODE_E *penSpdifCategoryCode)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_SetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
										MT_UNF_SND_SPDIF_SCMSMODE_E enSpdifSCMSMode)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_GetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
										MT_UNF_SND_SPDIF_SCMSMODE_E *penSpdifSCMSMode)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_SetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E enSampleRate)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_GetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E *penSampleRate)
{
	return 0;
}

mt_s32 MT_MPI_AO_SND_SetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode)
{
	return ioctl(g_fd_snd, SND_SET_TRACK_MODE, enMode);
}

mt_s32 MT_MPI_AO_SND_GetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E *penMode)
{
	if (NULL == penMode)
	{
		MT_ERR_AO("penMode is NULL\n");
		return -1;
	}
	
	return ioctl(g_fd_snd, SND_GET_TRACK_MODE, penMode);
}

mt_s32 MT_MPI_AO_SND_GetXrunCount(MT_UNF_SND_E enSound, mt_u32 *pu32Count)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_SND_Get_Xrun_Param_S stXunParam;

	CHECK_AO_NULL_PTR(pu32Count);
	stXunParam.enSound = enSound;

	s32Ret = ioctl(g_fd_snd, CMD_AO_SND_GETXRUNCOUNT, &stXunParam);
	if (MT_SUCCESS == s32Ret)
	{
		*pu32Count = stXunParam.u32Count;
	}

	return s32Ret;
}

mt_s32 MT_MPI_AO_AllTrack_SetMute(MT_UNF_SND_E enSound, MT_BOOL bMute)
{
	return MT_MPI_AO_SND_SetMute(enSound, MT_UNF_SND_OUTPUTPORT_ALL, bMute);
}

mt_s32 MT_MPI_AO_AllTrack_GetMute(MT_UNF_SND_E enSound, MT_BOOL *pbMute)
{
	return MT_MPI_AO_SND_GetMute(enSound, MT_UNF_SND_OUTPUTPORT_ALL, pbMute);
}

/******************************* MPI Track for UNF_SND*****************************/
mt_s32 MT_MPI_AO_Track_GetDefaultOpenAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
	if (NULL != pstAttr)
	{
		memset(pstAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
	}

	return 0;
}

mt_s32 MT_MPI_AO_Track_GetAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
	if (NULL == pstAttr)
	{
		MT_ERR_AO("invalid parameter!\n");
		return -1;
	}

	return ioctl(g_fd_snd, SND_GET_ATTR, pstAttr);
}

mt_s32 MT_MPI_AO_Track_GetCiTestInfo(mt_handle hTrack, MT_UNF_AVPLAY_CI_TEST_INFO_S *pInfo)
{
	if (NULL == pInfo)
	{
		MT_ERR_AO("invalid parameter!\n");
		return -1;
	}

	ioctl(g_fd_snd, SND_GET_CITEST_INFO, pInfo);
	return 0;
}

mt_s32 MT_MPI_AO_Track_SetAttr(mt_handle hTrack, const MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
	return 0;
}

mt_s32 MT_MPI_AO_Track_Create(MT_UNF_SND_E enSound, const MT_UNF_AUDIOTRACK_ATTR_S *pstAttr, mt_handle *phTrack)
{
	mt_s32 ret = 0;
	
	if (NULL == pstAttr)
	{
		MT_ERR_AO("invalid parameter!\n");
		return -1;
	}

	if (1 == pstAttr->b_spdif_mod)
	{
		ret = ioctl(g_fd_snd, SND_SET_HDMI_MODE, MT_UNF_SND_HDMI_MODE_RAW);
		ret = ioctl(g_fd_snd, SND_SET_SPDIF_MODE, MT_UNF_SND_SPDIF_MODE_RAW);
	}

	if (pstAttr->u32DebugCrcSize > 0)
	{
		ret = ioctl(g_fd_snd, SND_SET_CRC_DBG_BUF_SIZE, pstAttr->u32DebugCrcSize);
	}

	return ret;
}

mt_s32 MT_MPI_AO_Track_Destroy(mt_handle hTrack)
{
	int ret = 0;

	return ret;
}

mt_s32 MT_MPI_AO_Track_Start(mt_handle hTrack)
{
	return ioctl(g_fd_snd, SND_START);
}

mt_s32 MT_MPI_AO_Track_Stop(mt_handle hTrack)
{
	return ioctl(g_fd_snd, SND_STOP);
}

mt_s32 MT_MPI_AO_Track_Pause(mt_handle hTrack)
{
	return ioctl(g_fd_snd, SND_PAUSE);
}

mt_s32 MT_MPI_AO_Track_Resume(mt_handle hTrack)
{
	return ioctl(g_fd_snd, SND_RESUME);
}

mt_s32 MT_MPI_AO_Track_Flush(mt_handle hTrack)
{
	return ioctl(g_fd_snd, SND_FLUSH);
}

mt_s32 MT_MPI_AO_Track_SendData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
}

mt_s32 MT_MPI_AO_Track_SetWeight(mt_handle hTrack, const MT_UNF_SND_GAIN_ATTR_S *pstTrackGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
}

mt_s32 MT_MPI_AO_Track_GetWeight(mt_handle hTrack, MT_UNF_SND_GAIN_ATTR_S* pstTrackGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
}

mt_s32 MT_MPI_AO_Track_SetAbsWeight(mt_handle hTrack, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
}

mt_s32 MT_MPI_AO_Track_GetAbsWeight(mt_handle hTrack, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
}

mt_s32 MT_MPI_AO_Track_SetMute(mt_handle hTrack, MT_BOOL bMute)
{
	// not use
	return 0;
}

mt_s32 MT_MPI_AO_Track_GetMute(mt_handle hTrack, MT_BOOL *pbMute)
{
	// not use
	return 0;
}

mt_s32   MT_MPI_AO_Track_SetChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E enMode)
{
	// not use
	return 0;
}

mt_s32   MT_MPI_AO_Track_GetChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E *penMode)
{
	// not use
	return 0;
}

// MT_UNF_SND_TRACK_TYPE_VIRTUAL only
mt_s32 MT_MPI_AO_Track_AcquireFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, mt_u32 u32TimeoutMs)
{
	return 0;
}

mt_s32 MT_MPI_AO_Track_ReleaseFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
	return 0;
}

/******************************* MPI Track for MPI_AVPlay only **********************/
mt_s32   MT_MPI_AO_Track_SetEosFlag(mt_handle hTrack, MT_BOOL bEosFlag)
{
	return ioctl(g_fd_snd, SND_SET_EOS_FLAG, bEosFlag);
}

mt_s32   MT_MPI_AO_Track_SetSpeedAdjust(mt_handle hTrack, mt_s32 s32Speed, MT_MPI_SND_SPEEDADJUST_TYPE_E enType)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);

	AO_Track_SpeedAdjust_Param_S stSpeedAdjust;
	stSpeedAdjust.hTrack = hTrack;
	stSpeedAdjust.enType = (AO_SND_SPEEDADJUST_TYPE_E)enType;
	stSpeedAdjust.s32Speed = s32Speed;

	return ioctl(g_fd_snd, CMD_AO_TRACK_SETSPEEDADJUST, &stSpeedAdjust);
}

mt_s32   MT_MPI_AO_Track_GetDelayMs(const mt_handle hTrack, mt_u32 *pDelayMs)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);

	mt_s32 s32Ret;
	AO_Track_DelayMs_Param_S stDelayMs;
	CHECK_AO_NULL_PTR(pDelayMs);
	stDelayMs.hTrack = hTrack;

	s32Ret = ioctl(g_fd_snd, CMD_AO_TRACK_GETDELAYMS, &stDelayMs);
	if (MT_SUCCESS == s32Ret)
	{
		*pDelayMs = stDelayMs.u32DelayMs;
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_Track_IsBufEmpty(const mt_handle hTrack, MT_BOOL *pbEmpty)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_TRACK_ID(hTrack);
	CHECK_VIRTUAL_Track(hTrack);

	mt_s32 s32Ret;
	AO_Track_BufEmpty_Param_S stEmpty;
	CHECK_AO_NULL_PTR(pbEmpty);
	stEmpty.hTrack = hTrack;

	s32Ret = ioctl(g_fd_snd, CMD_AO_TRACK_ISBUFEMPTY, &stEmpty);
	if (MT_SUCCESS == s32Ret)
	{
		*pbEmpty = stEmpty.bEmpty;
	}

	return s32Ret;
}

mt_s32 MT_MPI_AO_Track_AttachAi(const mt_handle hAi, const mt_handle hTrack)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	AO_Track_AttAi_Param_S stTrackAttAi;

	CHECK_AO_TRACK_ID(hTrack);

	stTrackAttAi.hTrack = hTrack;
	stTrackAttAi.hAi = hAi;

	return ioctl(g_fd_snd, CMD_AO_TRACK_ATTACHAI, &stTrackAttAi);
}

mt_s32 MT_MPI_AO_Track_DetachAi(const mt_handle hAi, const mt_handle hTrack)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	AO_Track_AttAi_Param_S stTrackAttAi;

	CHECK_AO_TRACK_ID(hTrack);

	stTrackAttAi.hTrack = hTrack;
	stTrackAttAi.hAi = hAi;

	return ioctl(g_fd_snd, CMD_AO_TRACK_DETACHAI, &stTrackAttAi);
}

/******************************* MPI Snd Cast for UNF  **********************/
mt_s32   MT_MPI_AO_SND_GetCastDefaultOpenAttr(MT_UNF_SND_CAST_ATTR_S *pstAttr)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_NULL_PTR(pstAttr);

	return ioctl(g_fd_snd, CMD_AO_CAST_GETDEFATTR, pstAttr);
}

mt_s32   MT_MPI_AO_SND_CreateCast(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstCastAttr, mt_handle *phCast)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_Cast_Create_Param_S stCastParam;
	AO_Cast_Info_Param_S stCastInfo;

	CHECK_AO_NULL_PTR(phCast);
	CHECK_AO_NULL_PTR(pstCastAttr);

	stCastParam.enSound = enSound;
	memcpy(&stCastParam.stCastAttr, pstCastAttr, sizeof(MT_UNF_SND_CAST_ATTR_S));

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_CREATE, &stCastParam);
	if (MT_SUCCESS == s32Ret)
	{
		*phCast = stCastParam.hCast;
	}
	else
	{
		return s32Ret;
	}
	stCastInfo.hCast = stCastParam.hCast;

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETINFO, &stCastInfo);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("\n GET CAST INFO s32Ret=0x%x Failed \n", s32Ret);
		goto ERR_CREAT;
	}
	if (!stCastInfo.u32PhyAddr)
	{
		MT_ERR_AO("ERROE Phy addr =0x%x \n", stCastInfo.u32PhyAddr);
		goto ERR_CREAT;
	}

	//stCastInfo.u32UserVirtAddr = (mt_s32)MT_MEM_Map(stCastInfo.u32PhyAddr, stCastParam.u32ReqSize);
	if (!stCastInfo.u32UserVirtAddr)
	{
		MT_ERR_AO("\n u32PhyAddr(0x%x) MT_MEM_Map Failed \n", stCastInfo.u32PhyAddr);
		goto ERR_CREAT;
	}

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_SETINFO, &stCastInfo);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("\n  SET CAST INFO Failed 0x%x\n", s32Ret);
		goto ERR_MMAP;
	}

	return s32Ret;
ERR_MMAP:
	//MT_MEM_Unmap((mt_void *)stCastInfo.u32UserVirtAddr);
ERR_CREAT:
	ioctl(g_fd_snd, CMD_AO_CAST_DESTROY, &stCastInfo.hCast);

	return MT_FAILURE;
}
mt_s32   MT_MPI_AO_SND_DestroyCast(mt_handle hCast)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_Cast_Info_Param_S stCastInfo;

	stCastInfo.hCast = hCast;
	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETINFO, &stCastInfo);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("\n GET CAST INFO s32Ret=0x%x Failed \n", s32Ret);
	}
	else
	{
		//MT_INFO_AO("\n stCastInfo.u32UserVirtAddr(0x%x) TO ummap \n", stCastInfo.u32UserVirtAddr);
		//MT_MEM_Unmap((mt_void *)stCastInfo.u32UserVirtAddr);
	}

	return ioctl(g_fd_snd, CMD_AO_CAST_DESTROY, &hCast);
}
mt_s32   MT_MPI_AO_SND_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_Cast_Enable_Param_S stEnableAttr;

	stEnableAttr.hCast = hCast;
	stEnableAttr.bCastEnable = bEnable;

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_SETENABLE, &stEnableAttr);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("ENABLE CAST Failed 0x%x \n", s32Ret);
	}

	return s32Ret;
}
mt_s32   MT_MPI_AO_SND_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_Cast_Enable_Param_S stEnableAttr;

	stEnableAttr.hCast = hCast;

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETENABLE, &stEnableAttr);
	if (MT_SUCCESS == s32Ret)
	{
		*pbEnable = stEnableAttr.bCastEnable;
	}

	return s32Ret;
}

mt_s32 MT_MPI_AO_Cast_SetMute(mt_handle hCast, MT_BOOL bMute)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	

	AO_Cast_Mute_Param_S stMute;
	stMute.hCast = hCast;
	stMute.bMute  = bMute;
	return ioctl(g_fd_snd, CMD_AO_CAST_SETMUTE, &stMute);
}

mt_s32   MT_MPI_AO_Cast_GetMute(mt_handle hCast, MT_BOOL *pbMute)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_NULL_PTR(pbMute);


	mt_s32 s32Ret;
	AO_Cast_Mute_Param_S stMute;

	stMute.hCast = hCast;
	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETMUTE, &stMute);
	if (MT_SUCCESS == s32Ret)
	{
		*pbMute = stMute.bMute;
	}

	return s32Ret;
}


mt_s32   MT_MPI_AO_SND_SetCastAbsWeight(mt_handle hCast, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_NULL_PTR(pstAbsWeightGain);

	AO_Cast_AbsGain_Param_S stAbsGain;
	stAbsGain.hCast = hCast;
	memcpy(&stAbsGain.stCastAbsGain, pstAbsWeightGain, sizeof(MT_UNF_SND_ABSGAIN_ATTR_S));

	return ioctl(g_fd_snd, CMD_AO_CAST_SETABSGAIN, &stAbsGain);
}

mt_s32   MT_MPI_AO_SND_GetCastAbsWeight(mt_handle hCast, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	CHECK_AO_NULL_PTR(pstAbsWeightGain);

	mt_s32 s32Ret;
	AO_Cast_AbsGain_Param_S stAbsGain;

	stAbsGain.hCast = hCast;

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETABSGAIN, &stAbsGain);

	if (MT_SUCCESS == s32Ret)
	{
		memcpy(pstAbsWeightGain, &stAbsGain.stCastAbsGain, sizeof(MT_UNF_SND_ABSGAIN_ATTR_S));
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_SND_AcquireCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret = 0;
	AO_Cast_Info_Param_S stCastInfo;
	AO_Cast_Data_Param_S stCastData;

	CHECK_AO_NULL_PTR(pstCastFrame);

	stCastInfo.hCast = hCast;
	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETINFO, &stCastInfo);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("\n GET CAST INFO Failed Failed   s32Ret=0x%x \n", s32Ret);
		return MT_FAILURE;
	}

	stCastData.hCast = hCast;
	stCastData.u32FrameBytes = stCastInfo.u32FrameBytes;
	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_ACQUIREFRAME, &stCastData);
	if (MT_SUCCESS != s32Ret)
	{
		pstCastFrame->u32PcmSamplesPerFrame = 0;
		//MT_ERR_AO(" CAST ACQUIREFRAME Failed \n");
		return MT_FAILURE;
	}

	memcpy(pstCastFrame, &stCastData.stAOFrame, sizeof(MT_UNF_AO_FRAMEINFO_S));
	pstCastFrame->ps32PcmBuffer = (ulong *)(stCastInfo.u32UserVirtAddr + stCastData.u32DataOffset);
	if (0 == pstCastFrame->u32PcmSamplesPerFrame)
	{
		return MT_ERR_AO_CAST_TIMEOUT;
	}
	return MT_SUCCESS;
}
mt_s32   MT_MPI_AO_SND_ReleaseCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_Cast_Info_Param_S stCastInfo;
	AO_Cast_Data_Param_S stCastData;

	CHECK_AO_NULL_PTR(pstCastFrame);
	if (pstCastFrame->u32PcmSamplesPerFrame == 0)
	{
		//MT_INFO_AO("\nRelease CastID=0x%x, u32PcmSamplesPerFrame=0x%x\n", hCast, stCastData.stAOFrame.u32PcmSamplesPerFrame);
		return  MT_SUCCESS;
	}

	stCastInfo.hCast = hCast;
	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_GETINFO, &stCastInfo);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("GET CAST INFO Failed Failed \n");
		return MT_FAILURE;
	}

	if ((pstCastFrame->u32PcmSamplesPerFrame != 0 && pstCastFrame->u32PcmSamplesPerFrame != stCastInfo.u32FrameSamples)  ||
	        pstCastFrame->u32Channels != stCastInfo.u32Channels || pstCastFrame->s32BitPerSample != stCastInfo.s32BitPerSample)
	{
		MT_ERR_AO("Release Err Cast Frame Sample 0x%x  u32Channels 0x%x, u32SampleRate 0x%x \n", pstCastFrame->u32PcmSamplesPerFrame, pstCastFrame->u32Channels, pstCastFrame->u32SampleRate);
		return MT_FAILURE;
	}

	stCastData.hCast = hCast;
	stCastData.u32FrameBytes = stCastInfo.u32FrameBytes;
	memcpy(&stCastData.stAOFrame, pstCastFrame, sizeof(MT_UNF_AO_FRAMEINFO_S));

	s32Ret = ioctl(g_fd_snd, CMD_AO_CAST_RELEASEFRAME, &stCastData);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("CAST RELEASEFRAME Failed \n");
		return MT_FAILURE;
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_SND_AttachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId, mt_u32 *pu32AefProcAddr)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_SND_AttAef_Param_S stSndAttAef;

	CHECK_AO_NULL_PTR(pu32AefProcAddr);
	stSndAttAef.enSound = enSound;
	stSndAttAef.u32AefId = u32AefId;

	s32Ret = ioctl(g_fd_snd, CMD_AO_SND_ATTACHAEF, &stSndAttAef);
	if (MT_SUCCESS == s32Ret)
	{
		*pu32AefProcAddr = stSndAttAef.u32AefProcAddr;
	}

	return s32Ret;
}

mt_s32   MT_MPI_AO_SND_DetachAef(MT_UNF_SND_E enSound, mt_u32 u32AefId)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	AO_SND_AttAef_Param_S stSndAttAef;

	stSndAttAef.enSound = enSound;
	stSndAttAef.u32AefId = u32AefId;

	return ioctl(g_fd_snd, CMD_AO_SND_DETACHAEF, &stSndAttAef);
}

mt_s32  MT_MPI_AO_SND_SetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	AO_SND_AefBypass_Param_S stAefBypass;

	stAefBypass.enSound = enSound;
	stAefBypass.enOutPort = enOutPort;
	stAefBypass.bBypass = bBypass;

	return ioctl(g_fd_snd, CMD_AO_SND_SETAEFBYPASS, &stAefBypass);

}

mt_s32  MT_MPI_AO_SND_GetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass)
{
	printf("<ah> <%s:%d> todo!!!!\n", __func__, __LINE__);
	return 0;
	
	mt_s32 s32Ret;
	AO_SND_AefBypass_Param_S stAefBypass;

	CHECK_AO_NULL_PTR(pbBypass);
	stAefBypass.enSound = enSound;
	stAefBypass.enOutPort = enOutPort;

	s32Ret = ioctl(g_fd_snd, CMD_AO_SND_GETAEFBYPASS, &stAefBypass);
	if (MT_SUCCESS == s32Ret)
	{
		*pbBypass = stAefBypass.bBypass;
	}

	return s32Ret;
}

mt_s32 MT_MPI_AO_SND_SetFaderAttr(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_FADER_ATTR_S *pstFader)
{
	return ioctl(g_fd_snd, SND_SET_FADER, pstFader);
}

mt_s32 MT_MPI_AO_SND_GetBufInfo(MT_UNF_SND_E enSound, MT_UNF_SND_BUF_INFO_S *pstBufInfo)
{
	return ioctl(g_fd_snd, SND_GET_BUF_INFO, pstBufInfo);
}

mt_s32 MT_MPI_AO_SND_StartTTS(MT_UNF_SND_E enSound)
{
	mt_s32 s32Ret;
	
    s32Ret = ioctl(g_fd_snd, SND_START_TTS);
    return s32Ret;
}

mt_s32 MT_MPI_AO_SND_SendTTS(MT_UNF_SND_E enSound, mt_u8 *data, mt_u32 size)
{
    if(size != write(g_fd_snd, data, size))
		return MT_FAILURE;
	
	return MT_SUCCESS;
}

mt_s32 MT_MPI_AO_SND_StopTTS(MT_UNF_SND_E enSound)
{
	mt_s32 s32Ret;
	
    s32Ret = ioctl(g_fd_snd, SND_STOP_TTS);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
