/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_avplay.c
  Version       : Initial Draft
  Author        : Montage software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
    Author      :
    Modification: Created file

********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_mpi_avplay.h"
#include "mt_mpi_vdec.h"
#include "mt_module_debug.h"

#if 0
#define ENTER_FUNCTION			do{printf("%s: Enter\n",__FUNCTION__);}while(0)
#define LEAVE_FUNCTION			do{printf("%s: Leave@%d\n",__FUNCTION__,__LINE__);}while(0)
#else
#define ENTER_FUNCTION			do{}while(0)
#define LEAVE_FUNCTION			do{}while(0)
#endif

/** @deprecated */
mt_s32 MT_UNF_AVPLAY_PutBuf_A(mt_handle hAvplay, mt_u32 u32ValidDataLen, mt_u32 u32Pts, MT_BOOL bEos);

mt_s32 MT_UNF_AVPLAY_Init(mt_void)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Init();
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_DeInit(mt_void)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_DeInit();
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetDefaultConfig(MT_UNF_AVPLAY_ATTR_S *pstAvAttr, MT_UNF_AVPLAY_STREAM_TYPE_E enCfg)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetDefaultConfig(pstAvAttr, enCfg);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Create(const MT_UNF_AVPLAY_ATTR_S *pstAvAttr, mt_handle *phAvplay)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Create(pstAvAttr, phAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Destroy(mt_handle hAvplay)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Destroy(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetDDPTestMode(mt_handle hAvplay, MT_BOOL bEnable)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SetDDPTestMode(hAvplay, bEnable);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_ChnOpen(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_ChnOpen(hAvplay, enChn, pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_ChnClose(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_ChnClose(hAvplay, enChn);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Reset_Buffer(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, MT_UNF_AVPLAY_MEDIA_BUF_E enBuf,const mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Reset_Buffer(hAvplay, enChn, enBuf, pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SetAttr(hAvplay, enAttrID, pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetAttr(hAvplay, enAttrID, pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetDecodeMode(mt_handle hAvplay, MT_UNF_VCODEC_MODE_E enDecodeMode)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;

#ifndef JQW
	return MT_SUCCESS;
#endif
    s32Ret = MT_MPI_AVPLAY_SetDecodeMode(hAvplay, enDecodeMode);

	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_RegisterEvent(mt_handle      hAvplay,
                                   MT_UNF_AVPLAY_EVENT_E     enEvent,
                                   MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_RegisterEvent(hAvplay, enEvent, pfnEventCB);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_UnRegisterEvent(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_UnRegisterEvent(hAvplay, enEvent);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_RegisterAcodecLib(const mt_char *pFileName)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_RegisterAcodecLib(pFileName);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_RegisterVcodecLib(const mt_char *pFileName)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_VDEC_RegisterVcodecLib(pFileName);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_ConfigAcodec(const mt_u32 enDstCodecID, mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_ConfigAcodec(enDstCodecID,pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_FoundSupportDeoder(const HA_FORMAT_E enFormat, mt_u32 * penDstCodecID)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_FoundSupportDeoder(enFormat,penDstCodecID);
	LEAVE_FUNCTION;

    return s32Ret;
}

#if 0
mt_s32 MT_UNF_AVPLAY_ControlAcodec( mt_handle hAvplay, mt_void *pPara)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AVPLAY_ControlAcodec(hAvplay, pPara);

    return s32Ret;
}
#endif


mt_s32 MT_UNF_AVPLAY_PreStart(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_PRESTART_OPT_S *pstPreStartOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_PreStart(hAvplay, enChn);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Start(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_START_OPT_S *pstStartOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Start(hAvplay, enChn);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_PreStop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_PRESTOP_OPT_S *pstPreStopOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_PreStop(hAvplay, enChn,pstPreStopOpt);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Stop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Stop(hAvplay, enChn, pstStopOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Pause(mt_handle hAvplay, const MT_UNF_AVPLAY_PAUSE_OPT_S *pstPauseOpt)
{
    mt_s32 s32Ret;
    MT_UNF_AVPLAY_PAUSE_OPT_S opt;

	ENTER_FUNCTION;
    /* set default to compatible with previous */
    if(pstPauseOpt == NULL) {
        opt.avPushPause = MT_TRUE;
        opt.avDecPause = MT_TRUE;
        opt.avSyncPause = MT_TRUE;
        opt.avRenderPause = MT_TRUE;
        pstPauseOpt = &opt;
    }
    
    s32Ret = MT_MPI_AVPLAY_Pause(hAvplay, pstPauseOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Tplay(mt_handle hAvplay, const MT_UNF_AVPLAY_TPLAY_OPT_S *pstTplayOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;

#if 0
	MT_UNF_AVPLAY_FRMRATE_PARAM_S framerate = {0};


printf("\r\n ********MT_PVR_PlayDispTPlay u32fpsInteger:%d, u32SpeedDecimal:%d", pstTplayOpt->u32SpeedInteger,pstTplayOpt->u32SpeedDecimal );
	framerate.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_USER;
	if((pstTplayOpt->u32SpeedInteger == 0) && (pstTplayOpt->u32SpeedDecimal != 0))
		framerate.stSetFrmRate.u32fpsInteger = pstTplayOpt->u32SpeedDecimal * 25 / 1000;
	else if((pstTplayOpt->u32SpeedInteger != 0) && (pstTplayOpt->u32SpeedDecimal == 0))
		framerate.stSetFrmRate.u32fpsInteger = pstTplayOpt->u32SpeedInteger * 25;
	else
		return MT_FAILURE;

	s32Ret = MT_MPI_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &framerate);
#else
    s32Ret = MT_MPI_AVPLAY_Tplay(hAvplay, pstTplayOpt);
#endif

	LEAVE_FUNCTION;
    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetTrickCfg(mt_handle hAvplay, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
	s32Ret = MT_MPI_AVPLAY_SetTrickCfg(hAvplay, pstTrickParam);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_DecFrmType(mt_handle hAvplay, MT_UNF_DEC_FRM_TYPE_E eDecFrmType)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
	s32Ret = MT_MPI_AVPLAY_DecFrmType(hAvplay, eDecFrmType);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Resume(mt_handle hAvplay, const MT_UNF_AVPLAY_RESUME_OPT_S *pstResumeOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Resume(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Freeze(mt_handle hAvplay, const MT_UNF_AVPLAY_FREEZE_OPT_S *pstFreezeOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Freeze(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Reset(hAvplay, pstResetOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_HRSeek(mt_handle hAvplay, const MT_UNF_AVPLAY_HRSEEK_OPT_S *pstSeekOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_HRSeek(hAvplay, pstSeekOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Flush(mt_handle hAvplay)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Flush(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Flush_Audio(mt_handle hAvplay)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Flush_Audio(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetFrame(hAvplay, p_ao_frame);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_ReleaseFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_ReleaseFrame(hAvplay, p_ao_frame);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetBuf(mt_handle  hAvplay,
                            MT_UNF_AVPLAY_BUFID_E enBufId,
                            mt_u32                u32ReqLen,
                            MT_UNF_STREAM_BUF_S  *pstData,
                            mt_u32                u32TimeOutMs
                            )
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetBuf(hAvplay, enBufId, u32ReqLen, pstData, u32TimeOutMs);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_PutBuf64(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                              mt_u32 u32ValidDataLen, mt_u64 u64TimestampUs, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pPutOpt)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;

    if (pPutOpt == MT_NULL)
    {
        s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u64TimestampUs, MT_NULL, 0, 1, 0);
    }
    else
    {
		if(pPutOpt->u32EosFlag == MT_TRUE && enBufId == MT_UNF_AVPLAY_BUF_ID_ES_VID)
        {

			if(MT_MPI_AVPLAY_IsNormaPlay(hAvplay) == MT_SUCCESS)
			{
	            mt_u32 align_len = 1024;
	            MT_UNF_STREAM_BUF_S dst;

            /* EOS Handle: video decoder hardware read a block of data at a time, to handle
             * this align issue, we need to feed a block of zero data(more than 1k) to ES
             * buffer so that the video decoder hardware can finished the decoding.
             */
            do
            {
                s32Ret = MT_MPI_AVPLAY_GetBuf(hAvplay, enBufId, align_len, &dst, 0);
                if(s32Ret == MT_SUCCESS)
                {
                    memset(dst.pu8Data, 0, dst.u32Size);
                }
                else
                {
                    // should never be reached.
                    MT_ERR_AVPLAY("Handle video EOS failed: can not get buf %d\n", s32Ret);
                    break;
                }
                
                MT_ALWAYS_PRINT("Handle the eos flag %u\n", dst.u32Size);
                if(dst.u32Size >= align_len)
                {
                    pPutOpt->u32FrameFinsh = MT_TRUE;
                }
                else
                {
                    pPutOpt->u32FrameFinsh = MT_FALSE;
                }
                s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, dst.u32Size, u64TimestampUs, pPutOpt,
	    							pPutOpt->u32PtsValide,
	    							pPutOpt->u32FrameFinsh,
	    							pPutOpt->u32EosFlag);
				
                if(dst.u32Size >= align_len)
					align_len = 0;
				else
	                align_len -= dst.u32Size;
				
            } while(align_len);
		}
		else
		{                
				s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u64TimestampUs, pPutOpt,
	    							pPutOpt->u32PtsValide,
	    							pPutOpt->u32FrameFinsh,
	    							pPutOpt->u32EosFlag);
			}
        }
	else
	{
	    s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u64TimestampUs, pPutOpt,
	                                pPutOpt->u32PtsValide,
	                                pPutOpt->u32FrameFinsh,
	                                pPutOpt->u32EosFlag);
	}
    }

    LEAVE_FUNCTION;

    return s32Ret;
}

/* @DEPRECATED please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_PutBuf(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                                       mt_u32 u32ValidDataLen, mt_u32 u32TimestampMs)
{
    mt_s32 s32Ret;

    MT_UNF_AVPLAY_PUTBUFEX_OPT_S    stExOpt;

	ENTER_FUNCTION;

    stExOpt.bContinue = MT_TRUE;
    stExOpt.bEndOfFrm = MT_TRUE;

    s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u32TimestampMs*1000, &stExOpt, 1, 1, 0);

	LEAVE_FUNCTION;

    return s32Ret;
}

/* @DEPRECATED please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_PutBuf_A(mt_handle hAvplay, mt_u32 u32ValidDataLen, mt_u32 u32Pts, MT_BOOL bEos)
{
    mt_s32 s32Ret;

    MT_UNF_AVPLAY_PUTBUFEX_OPT_S    stExOpt;

	ENTER_FUNCTION;

    stExOpt.bContinue = MT_TRUE;
    stExOpt.bEndOfFrm = MT_TRUE;

    s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, u32ValidDataLen, u32Pts, &stExOpt, 1, 1, bEos);

	LEAVE_FUNCTION;

    return s32Ret;
}

/* @DEPRECATED please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_PutBuf_V1(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                               mt_u32 u32ValidDataLen, mt_u64 u64Pts, mt_u32 PtsValide, mt_u32 FrameFinsh, mt_u32 EosFlag)
{
    mt_s32 s32Ret;

    MT_UNF_AVPLAY_PUTBUFEX_OPT_S    stExOpt;

	ENTER_FUNCTION;

    stExOpt.bContinue = MT_TRUE;
    stExOpt.bEndOfFrm = MT_TRUE;

    s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u64Pts, &stExOpt, PtsValide, FrameFinsh, EosFlag);

	LEAVE_FUNCTION;

    return s32Ret;
}

/* @DEPRECATED please use MT_UNF_AVPLAY_PutBuf64 instead */
#if 0
mt_s32 MT_UNF_AVPLAY_PutBufEx(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                                       mt_u32 u32ValidDataLen, mt_u32 u32Pts, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pPutOpt)
{
    mt_s32 s32Ret;

    s32Ret = MT_MPI_AVPLAY_PutBuf(hAvplay, enBufId, u32ValidDataLen, u32Pts, pPutOpt, 0, 1, 0);

    return s32Ret;
}
#endif

/* @DEPRECATED please use MT_UNF_AVPLAY_PutBuf64 instead */
mt_s32 MT_UNF_AVPLAY_64BitPTS_PutBufEx(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId,
                              	  	   mt_u32 u32ValidDataLen, mt_u64 u64PtsUs, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pPutOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
	s32Ret = MT_UNF_AVPLAY_PutBuf64(hAvplay, enBufId, u32ValidDataLen, u64PtsUs, pPutOpt);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetDmxAudChnHandle(mt_handle hAvplay, mt_handle *phDmxAudChn)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetDmxAudChnHandle(hAvplay, phDmxAudChn);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetDmxVidChnHandle(mt_handle hAvplay, mt_handle *phDmxVidChn)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetDmxVidChnHandle(hAvplay, phDmxVidChn);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetStatusInfo(hAvplay, pstStatusInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetCiTestInfo(mt_handle hAvplay, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstCiTestInfo)
{
	mt_s32 s32Ret;

	ENTER_FUNCTION;
	s32Ret = MT_MPI_AVPLAY_GetCiTestInfo(hAvplay, pstCiTestInfo);
	LEAVE_FUNCTION;

	return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetStreamInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetStreamInfo(hAvplay, pstStreamInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetAudioSpectrum(mt_handle hAvplay, mt_u16 *pSpectrum, mt_u32 u32BandNum)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetAudioSpectrum(hAvplay, pSpectrum, u32BandNum);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_IsBuffEmpty(mt_handle hAvplay, MT_BOOL * pbIsEmpty)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_IsBuffEmpty(hAvplay, pbIsEmpty);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SwitchDmxAudChn(mt_handle hAvplay, mt_handle hNewDmxAud, mt_handle *phOldDmxAud)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SwitchDmxAudChn(hAvplay, hNewDmxAud, phOldDmxAud);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_FlushStream(mt_handle hAvplay, MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S *pstFlushOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_FlushStream(hAvplay, pstFlushOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Step(mt_handle hAvplay, const MT_UNF_AVPLAY_STEP_OPT_S *pstStepOpt)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Step(hAvplay, pstStepOpt);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Invoke(mt_handle hAvplay, MT_UNF_AVPLAY_INVOKE_E enInvokeType, mt_void *pPara)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Invoke(hAvplay, enInvokeType, pPara);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_AcqUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S *pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E *penType)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_AcqUserData(hAvplay, pstUserData, penType);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_RlsUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S* pstUserData)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_RlsUserData(hAvplay, pstUserData);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_RstUserDataBuffer(mt_handle hAvplay)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_RstUserDataBuffer(hAvplay);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetAudioStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetAudioStatusInfo(hAvplay, pstStatusInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetVideoStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetVideoStatusInfo(hAvplay, pstStatusInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetSyncStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetSyncStatusInfo(hAvplay, pstStatusInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetAVsyncMode(mt_handle hAvplay, MT_UNF_SYNC_REF_E enAVsyncMode)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SetAVsyncMode(hAvplay, enAVsyncMode);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_EnableAVOutInfo(mt_handle hAvplay, mt_u32 enable)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_EnableAVOutInfo(hAvplay, enable);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetAVOutInfo(mt_handle          hAvplay,
                                MT_UNF_AVPLAY_AVOUT_DEBUG_INFO_S *pstOutInfo)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetAVOutInfo(hAvplay, pstOutInfo);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_StopAudDec(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_StopAudDec(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_StartAudDec(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_StartAudDec(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_AudioTrack(mt_handle hAvplay)
{
    mt_s32 s32Ret;
    mt_u32 track = 0;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_AudioTrack(hAvplay, &track);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_Enable_AudioHEAAC(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_Enable_AudHEAAC(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_set_downmix_enable(mt_handle hAvplay, u32 enable)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_set_downmix_enable(hAvplay, enable);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_TrickSeekIn(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_TrickSeekIn(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_TrickSeekOut(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_TrickSeekOut(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}


mt_s32 MT_UNF_AVPLAY_SW_ADEC_ENABLE(mt_handle hAvplay)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SW_ADEC_ENABLE(hAvplay);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetVideoESPhyAddr(mt_handle hAvplay, phys_addr_t *esBuffPhyAddr, mt_u32 *esBuffSize)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetVideoESPhyAddrr(hAvplay, esBuffPhyAddr, esBuffSize);
    LEAVE_FUNCTION;

    return s32Ret;
}


mt_s32 MT_UNF_AVPLAY_ListAllPlayer(MT_UNF_AVPLAY_PLAYERINFO_S * info)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_ListAllPlayer(info);
    LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_GetMetaInfo(mt_handle hAvplay, MT_UNF_AVPLAY_METARINFO_S *pMetaInfo)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_GetMetaInfo(hAvplay, pMetaInfo);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SetPos(mt_handle hAvplay, float x, float y, float z)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SetPos(hAvplay, x, y, z);
	LEAVE_FUNCTION;

    return s32Ret;
}

mt_s32 MT_UNF_AVPLAY_SelObj(mt_handle hAvplay, mt_u16 id, mt_u16 on)
{
    mt_s32 s32Ret;

	ENTER_FUNCTION;
    s32Ret = MT_MPI_AVPLAY_SelObj(hAvplay, id, on);
	LEAVE_FUNCTION;

    return s32Ret;
}
