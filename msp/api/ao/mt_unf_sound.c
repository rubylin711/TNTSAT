/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//#include "mt_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "mt_error_mpi.h"
#include "mt_unf_sound.h"
//#include "mt_mpi_avplay.h"

#include "mt_mpi_ao.h"

#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_module_debug.h"
#include "mt_drv_ao.h"
#include "mt_mpi_avplay.h"
#include "mt_mpi_aef.h"

#if defined (MT_SOUND_AI_SUPPORT)
#include "mt_mpi_ai.h"
#endif

/***************************** Macro Definition ******************************/
#define MT_UNF_SND_PORT_MAST  (0)
#define MT_UNF_SND_PORT_SLAVE (1)


#define API_SND_CheckNULLPtr(ptr) do{\
        if (NULL == ptr)\
        {\
            MT_ERR_AO("PTR is NULL!\n");\
            return MT_ERR_AO_NULL_PTR;\
        }\
    }while(0)

#define API_SND_CheckId(u32SndId) do{\
        if (MT_UNF_SND_BUTT <= u32SndId)\
        {\
            MT_ERR_AO("Sound ID(%#x) is Invalid!\n", u32SndId);\
            return MT_ERR_AO_INVALID_ID;\
        }\
    }while(0)

#define API_SND_CheckInterface(enInterface) do{\
        if (enInterface >= MT_UNF_SND_INTERFACE_BUTT) \
    	{ \
    		MT_ERR_AO("intf(%d) is invalid\n", enInterface); \
    		return MT_ERR_AO_INVALID_PARA; \
    	}\
	}while(0)


/*************************** Structure Definition ****************************/
typedef struct hiAPI_SND_COMM_S
{
    MT_BOOL              bCreate;
} API_SND_COMM_S;

/******************************* API declaration *****************************/
mt_s32 MT_UNF_SND_Init(mt_void)
{
  MT_INFO_AO(" \n\n\n  unf layer MT_UNF_SND_Init \n\n\n");

    return MT_MPI_AO_Init();
}

mt_s32 MT_UNF_SND_DeInit(mt_void)
{
    return MT_MPI_AO_DeInit();
}

mt_s32   MT_UNF_SND_SendTrackData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
#if 0	// ahren comment
    mt_s32 Ret = MT_SUCCESS;

    API_SND_CheckNULLPtr(pstAOFrame);

    //Ret = MT_MPI_AO_Track_Start(hTrack);
    //if (Ret != MT_SUCCESS)
    //{
        //return Ret;
    //}

    Ret = MT_MPI_AO_Track_SendData(hTrack, pstAOFrame);
    if(Ret == MT_ERR_AO_OUT_BUF_FULL)
    {
    	return Ret;
    }
    else if (Ret != MT_SUCCESS)
    {
        return MT_ERR_AO_INVALID_PARA;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetTrackDelayMs(const mt_handle hTrack, mt_u32 *pDelayMs)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_GetDelayMs(hTrack, pDelayMs);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) DelayMs failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_E enSound, MT_UNF_SND_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    API_SND_CheckId(enSound);
    s32Ret = MT_MPI_AO_SND_GetDefaultOpenAttr(enSound, pstAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_AO("MT_UNF_SND_GetDefaultOpenAttr Failed:%#x.\n", s32Ret);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_Open(MT_UNF_SND_E enSound, const MT_UNF_SND_ATTR_S *pstAttr)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_Open(enSound, pstAttr);
    if (MT_SUCCESS != ret)
    {
        MT_WARN_AO("AO_OPEN Failed:%#x.\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_Close(MT_UNF_SND_E enSound)
{
    mt_s32 ret;
    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_Close(enSound);

    return ret;
}

mt_s32 MT_UNF_SND_SetAdacOnOff(MT_UNF_SND_E enSound, MT_BOOL bOnOff)
{
    mt_s32 ret;    
    API_SND_CheckId(enSound);
    
    ret = MT_MPI_AO_SND_SetAdacOnOff(enSound, bOnOff);

    return ret;
}

mt_s32  MT_UNF_SND_SetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bMute)

{
    mt_s32 ret;

    API_SND_CheckId(enSound);

	ret = MT_MPI_AO_SND_SetMute(enSound, enOutPort, bMute);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("set AO mute failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetMute(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbMute)
{

    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_GetMute(enSound, enOutPort, pbMute);
}


mt_s32 MT_UNF_SND_SetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, const MT_UNF_SND_GAIN_ATTR_S *pstGain)

{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_SetVolume(pstGain->s32Gain);		// ahren
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("set AO volume failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_GAIN_ATTR_S *pstGain)
{
	int ret;
	ulong volume;
	
	API_SND_CheckId(enSound);
	
	ret = MT_MPI_AO_SND_GetVolume(&volume);		// ahren

	pstGain->s32Gain = (mt_s32)volume;

	return ret;
}


mt_s32   MT_UNF_SND_SetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_CATEGORYCODE_E enSpdifCategoryCode)
{
    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_SetSpdifCategoryCode(enSound, enOutPort, enSpdifCategoryCode);
}
mt_s32   MT_UNF_SND_GetSpdifCategoryCode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_CATEGORYCODE_E *penSpdifCategoryCode)
{
    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_GetSpdifCategoryCode(enSound, enOutPort, penSpdifCategoryCode);
}
mt_s32   MT_UNF_SND_SetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_SCMSMODE_E enSpdifSCMSMode)

{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_SetSpdifSCMSMode(enSound, enOutPort, enSpdifSCMSMode);
}


mt_s32   MT_UNF_SND_GetSpdifSCMSMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_SCMSMODE_E *penSpdifSCMSMode)

{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_GetSpdifSCMSMode(enSound, enOutPort, penSpdifSCMSMode);
}


mt_s32 MT_UNF_SND_SetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    if (enSampleRate < MT_UNF_SAMPLE_RATE_48K)
    {
        /*lc change enSampleRate = MT_UNF_SAMPLE_RATE_48K;*/
    }

    //TODO enOutPort useless
    ret = MT_MPI_AO_SND_SetSampleRate(enSound, enSampleRate);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("set AO sampleRate to %d failed, ERR:%#x\n", enSampleRate, ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetSampleRate(MT_UNF_SND_E enSound, MT_UNF_SAMPLE_RATE_E *penSampleRate)
{
    API_SND_CheckId(enSound);

    //TODO enOutPort useless
    return MT_MPI_AO_SND_GetSampleRate(enSound, penSampleRate);
}

mt_s32 MT_UNF_SND_SetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bSmartVolume)
{
    API_SND_CheckId(enSound);
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_GetSmartVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbSmartVolume)
{
    API_SND_CheckId(enSound);
    return MT_ERR_AO_NOTSUPPORT;;
}

mt_s32 MT_UNF_SND_SetTrackSmartVolume(mt_handle hTrack, MT_BOOL bEnable)
{
#if defined (MT_SMARTVOLUME_SUPPORT)
    mt_s32 s32Ret;
    s32Ret = MT_MPI_AO_Track_SetSmartVolume(hTrack, bEnable);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Track(%d) Smart volume failed.\n", hTrack);
        return s32Ret;
    }

    return MT_SUCCESS;
#else
    MT_ERR_AO("Do not Support, should enable 'SND Smart Volume Support' at make menuconfig.\n");
    return MT_ERR_AO_NOTSUPPORT;
#endif
}

mt_s32 MT_UNF_SND_GetTrackSmartVolume(mt_handle hTrack, MT_BOOL *pbEnable)
{
#if defined (MT_SMARTVOLUME_SUPPORT)
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_GetSmartVolume(hTrack, pbEnable);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) Smart volume failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
#else
    MT_ERR_AO("Do not Support, should enable 'SND Smart Volume Support' at make menuconfig.\n");
    return MT_ERR_AO_NOTSUPPORT;
#endif
}

mt_s32 MT_UNF_SND_SetTrackMode(MT_UNF_SND_E enSound,MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E enMode)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_SetTrackMode(enSound, enOutPort, enMode);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("set AO TrackMode to %d failed, ERR:%#x\n", enMode, ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetTrackMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_TRACK_MODE_E *penMode)
{
    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_GetTrackMode(enSound, enOutPort, penMode);
}


mt_s32  MT_UNF_SND_SetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL bMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_AllTrack_SetMute(enSound, bMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set AllTrack Mute failed.\n");
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_GetAllTrackMute(MT_UNF_SND_E enSound, MT_BOOL *pbMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_AllTrack_GetMute(enSound, pbMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get AllTrack Mute Status failed.\n");
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_SetPrecisionVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, const MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_GetPrecisionVolume(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_SetBalance(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, mt_s32 s32Balance)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_GetBalance(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, mt_s32 *ps32Balance)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32   MT_UNF_SND_GetDefaultCastAttr(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstAttr)
{
    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_GetCastDefaultOpenAttr(pstAttr);
}

mt_s32 MT_UNF_SND_CreateCast(MT_UNF_SND_E enSound, MT_UNF_SND_CAST_ATTR_S *pstAttr, mt_handle *phCast)
{
    API_SND_CheckId(enSound);
    return MT_MPI_AO_SND_CreateCast(enSound, pstAttr, phCast);
}

mt_s32 MT_UNF_SND_DestroyCast(mt_handle hCast)
{
    return MT_MPI_AO_SND_DestroyCast(hCast);
}

mt_s32 MT_UNF_SND_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
    return MT_MPI_AO_SND_SetCastEnable(hCast,bEnable);
}

mt_s32 MT_UNF_SND_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    return MT_MPI_AO_SND_GetCastEnable(hCast,pbEnable);
}

mt_s32 MT_UNF_SND_AcquireCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame, mt_u32 u32TimeoutMs)
{
    mt_s32 s32Ret;
    mt_u32 u32SleepCnt;

	s32Ret = MT_MPI_AO_SND_AcquireCastFrame(hCast,pstCastFrame);
	if(MT_ERR_AO_CAST_TIMEOUT != s32Ret)
	{
		return s32Ret;
	}
	for(u32SleepCnt = 0; u32SleepCnt < u32TimeoutMs; u32SleepCnt++)
	{
		(mt_void)MT_USLEEP(1 * 1000);
		s32Ret = MT_MPI_AO_SND_AcquireCastFrame(hCast,pstCastFrame);
		if(MT_ERR_AO_CAST_TIMEOUT != s32Ret)
		{
			return s32Ret;
		}
	}
    return s32Ret;
}

mt_s32 MT_UNF_SND_ReleaseCastFrame(mt_handle hCast, MT_UNF_AO_FRAMEINFO_S *pstCastFrame)
{
    return MT_MPI_AO_SND_ReleaseCastFrame(hCast,pstCastFrame);
}

mt_s32   MT_UNF_SND_SetCastMute(mt_handle hCast, MT_BOOL bMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Cast_SetMute(hCast, bMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Cast(%d) Mute failed.\n", hCast);
        return s32Ret;
    }
    return MT_SUCCESS;

}

mt_s32   MT_UNF_SND_GetCastMute(mt_handle hCast, MT_BOOL *pbMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Cast_GetMute(hCast, pbMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Cast(%d) Mute failed.\n", hCast);
        return s32Ret;
    }
    return MT_SUCCESS;

}

mt_s32 MT_UNF_SND_SetCastAbsWeight(mt_handle hCast, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_SND_SetCastAbsWeight(hCast, pstAbsWeightGain);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Cast(%d) AbsWeight failed.\n", hCast);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_GetCastAbsWeight(mt_handle hCast,  MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_SND_GetCastAbsWeight(hCast, pstAbsWeightGain);

    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get CAST(%d) AbsWeight failed.\n", hCast);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_Attach(mt_handle hTrack, mt_handle hSource)
{
#if 0	// ahren comment
    mt_s32 s32Ret;

    CHECK_AO_TRACK_ID(hTrack);


    if(MT_ID_AVPLAY == (hSource>>16))
    {

        s32Ret = MT_MPI_AVPLAY_AttachSnd(hSource, hTrack);
        if (s32Ret != MT_SUCCESS)
        {
            MT_ERR_AO("call MT_MPI_AVPLAY_AttachSnd failed.\n");
            return s32Ret;
        }
	    s32Ret = MT_MPI_AO_Track_Start(hTrack);
	    if (s32Ret != MT_SUCCESS)
	    {
	        MT_ERR_AO("call MT_MPI_AO_Track_Start failed.\n");
	        return s32Ret;
	    }
    }

#if defined (MT_SOUND_AI_SUPPORT)
    else if(MT_ID_AI == (hSource>>16))
    {
        s32Ret = MT_MPI_AI_Attach(hSource, hTrack);
        if (s32Ret != MT_SUCCESS)
        {
            MT_ERR_AO("call MT_MPI_AI_Attach failed.\n");
            return s32Ret;
        }
    }
#endif
	else
	{
		MT_ERR_AO("Invalid hsoure(0x%x)\n", hSource);
		return MT_FAILURE;
	}
#endif

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_Detach(mt_handle hTrack, mt_handle hSource)
{
    mt_s32 ret;

#if 0	// ahren comment
    CHECK_AO_TRACK_ID(hTrack);

    if(MT_ID_AVPLAY == (hSource>>16))
    {
        ret = MT_MPI_AVPLAY_DetachSnd(hSource, hTrack);
        if (ret != MT_SUCCESS)
        {
            MT_ERR_AO("call MT_MPI_AVPLAY_DetachSnd failed.\n");
            return ret;
        }
    }

#if defined (MT_SOUND_AI_SUPPORT)
    else if(MT_ID_AI == (hSource>>16))
    {
        ret = MT_MPI_AI_Detach(hSource, hTrack);
        if (ret != MT_SUCCESS)
        {
            MT_ERR_AO("call MT_MPI_AI_DetachSnd failed.\n");
            return ret;
        }
    }
#endif
	else
	{
		MT_ERR_AO("Invalid hsoure(0x%x)\n", hSource);
		return MT_FAILURE;
	}
#endif


    ret = MT_MPI_AO_Track_Stop(hTrack);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_AO("call MT_MPI_AO_Track_Stop failed.\n");
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32	 MT_UNF_SND_SetTrackWeight(mt_handle hTrack, const MT_UNF_SND_GAIN_ATTR_S *pstMixWeightGain)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_SetWeight(hTrack, pstMixWeightGain);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Track(%d) Weight failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32   MT_UNF_SND_GetTrackWeight(mt_handle hTrack, MT_UNF_SND_GAIN_ATTR_S *pstMixWeightGain)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_GetWeight(hTrack, pstMixWeightGain);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) Weight failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}


mt_s32  MT_UNF_SND_SetTrackAbsWeight(mt_handle hTrack, const MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_SetAbsWeight(hTrack, pstAbsWeightGain);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Track(%d) AbsWeight failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_GetTrackAbsWeight(mt_handle hTrack, MT_UNF_SND_ABSGAIN_ATTR_S *pstAbsWeightGain)
{
    mt_s32 s32Ret;


    s32Ret = MT_MPI_AO_Track_GetAbsWeight(hTrack, pstAbsWeightGain);

    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) AbsWeight failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

// not support
#if 0
mt_s32  MT_UNF_SND_SetTrackPrescale(mt_handle hTrack, const MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32  MT_UNF_SND_GetTrackPrescale(mt_handle hTrack, MT_UNF_SND_PRECIGAIN_ATTR_S *pstPreciGain)
{
    return MT_ERR_AO_NOTSUPPORT;
}
#endif

mt_s32  MT_UNF_SND_SetTrackMute(mt_handle hTrack, MT_BOOL bMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_SetMute(hTrack, bMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Track(%d) Mute failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_GetTrackMute(mt_handle hTrack, MT_BOOL *pbMute)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_GetMute(hTrack, pbMute);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) Mute failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_SetTrackChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E enMode)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_SetChannelMode(hTrack, enMode);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Set Track(%d) SoftTrackMode failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}


mt_s32 MT_UNF_SND_GetTrackChannelMode(mt_handle hTrack, MT_UNF_TRACK_MODE_E *penMode)
{
    mt_s32 s32Ret;


    s32Ret = MT_MPI_AO_Track_GetChannelMode(hTrack, penMode);

    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_AO("Get Track(%d) SoftTrackMode failed.\n", hTrack);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32   MT_UNF_SND_SetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_HDMI_MODE_E enHdmiMode)

{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_SetHdmiMode(enSound, enOutPort, enHdmiMode);
}

mt_s32   MT_UNF_SND_GetHdmiMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_HDMI_MODE_E *penHdmiMode)
{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_GetHdmiMode(enSound, enOutPort, penHdmiMode);
}

mt_s32   MT_UNF_SND_SetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_MODE_E enSpdifMode)

{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_SetSpdifMode(enSound, enOutPort, enSpdifMode);
}

mt_s32   MT_UNF_SND_GetSpdifMode(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort,
                                           MT_UNF_SND_SPDIF_MODE_E *penSpdifMode)

{
    API_SND_CheckId(enSound);

    return MT_MPI_AO_SND_GetSpdifMode(enSound, enOutPort, penSpdifMode);
}


mt_s32   MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_E enTrackType, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
	return MT_MPI_AO_Track_GetDefaultOpenAttr(enTrackType, pstAttr);
}

mt_s32   MT_UNF_SND_CreateTrack(MT_UNF_SND_E enSound,const MT_UNF_AUDIOTRACK_ATTR_S *pTrackAttr,mt_handle *phTrack)
{
	mt_s32 s32Ret;

	//TODO More Simpler
	*phTrack = 1;
	s32Ret = MT_MPI_AO_Track_Create(enSound, pTrackAttr, phTrack);
	if (MT_SUCCESS != s32Ret)
	{
		MT_ERR_AO("Create Track failed, ERR:%#x\n", s32Ret);
		return s32Ret;
	}

	return s32Ret;
}

mt_s32   MT_UNF_SND_DestroyTrack(mt_handle hTrack)
{
    mt_s32 s32Ret;

    //TODO  1.Detach Track 2. Destory
    //MT_MPI_AO_SND_DetachTrack(MT_UNF_SND_E enSound, mt_handle hTrack)

    s32Ret = MT_MPI_AO_Track_Destroy(hTrack);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("Destroy Track failed, ERR:%#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

mt_s32   MT_UNF_SND_SetTrackAttr(mt_handle hTrack, const MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_SetAttr(hTrack, pstTrackAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("Set Track Attr failed, ERR:%#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

mt_s32   MT_UNF_SND_GetTrackAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AO_Track_GetAttr(hTrack, pstTrackAttr);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("Get Track Attr failed, ERR:%#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

mt_s32  MT_UNF_SND_AcquireTrackFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, mt_u32 u32TimeoutMs)
{
    mt_s32 s32Ret;
    s32Ret = MT_MPI_AO_Track_AcquireFrame(hTrack, pstAOFrame, u32TimeoutMs);
    if (MT_SUCCESS != s32Ret)
    {
        MT_WARN_AO("MT_UNF_SND_AcquireFrame failed, ERR:%#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

mt_s32  MT_UNF_SND_ReleaseTrackFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
    mt_s32 s32Ret;
    s32Ret = MT_MPI_AO_Track_ReleaseFrame(hTrack, pstAOFrame);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_AO("Release Frame failed, ERR:%#x\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

mt_s32  MT_UNF_SND_StartTTS(MT_UNF_SND_E enSound)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

	ret = MT_MPI_AO_SND_StartTTS(enSound);
    if (MT_SUCCESS != ret) {
        MT_ERR_AO("start tts failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_SendTTS(MT_UNF_SND_E enSound, mt_u8 *data, mt_u32 size)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

	ret = MT_MPI_AO_SND_SendTTS(enSound, data, size);
    if (MT_SUCCESS != ret) {
        MT_ERR_AO("send tts failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32  MT_UNF_SND_StopTTS(MT_UNF_SND_E enSound)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

	ret = MT_MPI_AO_SND_StopTTS(enSound);
    if (MT_SUCCESS != ret) {
        MT_ERR_AO("stop tts failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_SetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL bBypass)
{
    return MT_ERR_AO_NOTSUPPORT;
}
mt_s32 MT_UNF_SND_GetAefBypass(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_BOOL *pbBypass)
{
    return MT_ERR_AO_NOTSUPPORT;
}

/******************* Advance authorize golbal audio effect API ************************/
 /** Registers Audio effect authorize library*/
mt_s32 MT_UNF_SND_RegisterAefAuthLib(const mt_char *pAefLibFileName)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_CreateAef(MT_UNF_SND_E enSound, MT_UNF_SND_AEF_TYPE_E enAefType, mt_void *pstAdvAttr, mt_handle *phAef)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_DestroyAef(mt_handle hAef)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_SetAefEnable(mt_handle hAef, MT_BOOL bEnable)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_SetAefParams(mt_handle hAef, mt_u32 u32ParamType, const mt_void *pstParms)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_GetAefParams(mt_handle hAef, mt_u32 u32ParamType, mt_void *pstParms)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_SetAefConfig(mt_handle hAef, mt_u32 u32CfgType, const mt_void *pstConfig)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_GetAefConfig(mt_handle hAef, mt_u32 u32CfgType, mt_void *pstConfig)
{
    return MT_ERR_AO_NOTSUPPORT;
}

mt_s32 MT_UNF_SND_SetFaderAttr(MT_UNF_SND_E enSound, MT_UNF_SND_OUTPUTPORT_E enOutPort, MT_UNF_SND_FADER_ATTR_S *pstFader)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_SetFaderAttr(enSound, enOutPort, pstFader);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("set AO fader failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_UNF_SND_GetBufInfo(MT_UNF_SND_E enSound, MT_UNF_SND_BUF_INFO_S *pstBufInfo)
{
    mt_s32 ret;

    API_SND_CheckId(enSound);

    ret = MT_MPI_AO_SND_GetBufInfo(enSound, pstBufInfo);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AO("Get AO buffer info failed, ERR:%#x\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

