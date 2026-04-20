/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : audio_decoder.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/06/17
 * Description    : Montage audio decoder for OMX IL component.
 * History        :
 * 1.Date         : 2017/06/17
 *   Author       : 100613
 *   Modification : Created file
 *
 *****************************************************************************/
/*
 * 20170617:
 */
#include <pthread.h>
#include "mt_audio_codec.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "libavcodec/avcodec.h"
#include "HA.AUDIO.G711.codec.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.AMRWB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"
#include "avplayal.h"

#undef NDEBUG

#ifdef ANDROID
#include <utils/Log.h>
#else
#define ALOGI printf
#define ALOGD printf
#define ALOGE printf
#endif

#undef LOG_TAG
#define LOG_TAG "audio_decoder"

#define TIME_OUT_MS_ES_PUSH       0//10
#define PUSH_DATA_TIME_COUNT 300

#define ADEC_DEBUG 				ALOGD
#define ADEC_INFO 				ALOGI
#define ADEC_ERROR 				ALOGE

#define INVALID_HANDLE			0
#define NORMAL_PCM_EXTWORD    1
#define WIFIDSP_LPCM_EXTWORD  2

static FLUSH_OUTPUT_E flush_status = FLUSH_NO;


/* audio buffer flags */
enum {
	AUDIO_BUFFER_FLAG_EOS 	= 0x80000000UL,		// end of stream
	//TODO...
	// add your buffer flags here:
};



typedef struct {
	unsigned long present_frames;
	int64_t present_timestamp;		// in ms
	//TODO...
	// add your parameters here:
} audio_decoder_status_t;

enum {
	AUD_ESBUF_GETBUF_FAILED = -2,
	AUD_ESBUF_PUTBUF_FAILED = -3,

};

#define CHECK_ARG(expr, retval)		do {	\
										if ((expr)) {	\
											ADEC_ERROR("%s @%d: Error, Invalid arguments!\n",__FUNCTION__,__LINE__);	\
											return retval;	\
										}	\
									} while(0)


//----------------------------------------------------------------------------//

static int g_adec_opened 	     = 0;
static MT_HANDLE g_adec_handle   = INVALID_HANDLE;
static MT_HANDLE g_avplay_handle = INVALID_HANDLE;
//static ADEC_STATUS_T g_audio_decoder_status = DECODE_OTHER;
static short g_stop_flag = 0;
static AUD_DEC_HANDLE_T g_adecoder_h;
mt_u8 u8DecOpenBuf[1024];
//mt_u8 u8EncOpenBuf[1024];
extern int g_check_task_stop;



#if 0
static pthread_mutex_t g_adec_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_LOCK()				pthread_mutex_lock(&g_adec_mutex)
#define MUTEX_UNLOCK()				pthread_mutex_unlock(&g_adec_mutex)
#else
/*
 * audio & video decoders both use this mutex for
 * they have conflict calling AVPlay APIs.
 */
extern pthread_mutex_t g_avdec_mutex;
#define MUTEX_LOCK()				pthread_mutex_lock(&g_avdec_mutex)
#define MUTEX_UNLOCK()				pthread_mutex_unlock(&g_avdec_mutex)
#endif


static int Get_Atype(enum AVCodecID audio_codec_id, u32 *p_ffmpeg_codec_id)//for ffmpeg
{
    HA_CODEC_ID_E AdecType = HA_AUDIO_ID_CUSTOM_1;

    ADEC_INFO("[%s]  acodec_type:0x%x\n", __FUNCTION__, audio_codec_id);

    switch(audio_codec_id)
    	{
		case AV_CODEC_ID_MP3:
			AdecType = HA_AUDIO_ID_MP3;
			ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_MP3\n", __FUNCTION__);
			break;

		case AV_CODEC_ID_AAC:
			AdecType = HA_AUDIO_ID_AAC;
			ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_AAC\n", __FUNCTION__);
			break;

		case AV_CODEC_ID_AC3:
		case AV_CODEC_ID_EAC3:
			AdecType = HA_AUDIO_ID_DOLBY_PLUS;
			ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_DOLBY_PLUS\n", __FUNCTION__);
			break;

		case AV_CODEC_ID_AMR_NB:
			AdecType = HA_AUDIO_ID_AMRNB;
			ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_AMRNB\n", __FUNCTION__);
			break;

		/*case AV_CODEC_ID_AMR_WB:
			AdecType = HA_AUDIO_ID_AMRWB;
			DEBUG_PRINT_STATE("\n[%s]  AdecType = HA_AUDIO_ID_AMRWB\n", __FUNCTION__);
			break;*/

        case AV_CODEC_ID_MP2:
			AdecType = HA_AUDIO_ID_MP2;
			ADEC_INFO("\n[%s]  AdecType = AV_CODEC_ID_MP2\n", __FUNCTION__);
			break;

		/*case OMX_AUDIO_CodingPCM:
			AdecType = HA_AUDIO_ID_PCM;
			ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_PCM\n", __FUNCTION__);
			break;*/

        case AV_CODEC_ID_PCM_S16LE:
        case AV_CODEC_ID_PCM_S16BE:
        case AV_CODEC_ID_PCM_U16LE:
        case AV_CODEC_ID_PCM_U16BE:
        case AV_CODEC_ID_PCM_S8:
        case AV_CODEC_ID_PCM_U8:
        case AV_CODEC_ID_PCM_MULAW:
        case AV_CODEC_ID_PCM_ALAW:
        case AV_CODEC_ID_PCM_S32LE:
        case AV_CODEC_ID_PCM_S32BE:
        case AV_CODEC_ID_PCM_U32LE:
        case AV_CODEC_ID_PCM_U32BE:
        case AV_CODEC_ID_PCM_S24LE:
        case AV_CODEC_ID_PCM_S24BE:
        case AV_CODEC_ID_PCM_U24LE:
        case AV_CODEC_ID_PCM_U24BE:
        case AV_CODEC_ID_PCM_S24DAUD:
        case AV_CODEC_ID_PCM_ZORK:
        case AV_CODEC_ID_PCM_S16LE_PLANAR:
        case AV_CODEC_ID_PCM_DVD:
        /*case AV_CODEC_ID_PCM_F32BE:
        case AV_CODEC_ID_PCM_F32LE:
        case AV_CODEC_ID_PCM_F64BE:
        case AV_CODEC_ID_PCM_F64LE:
        case AV_CODEC_ID_PCM_BLURAY:
        case AV_CODEC_ID_PCM_LXF:
        case AV_CODEC_ID_S302M:
        case AV_CODEC_ID_PCM_S8_PLANAR:
        case AV_CODEC_ID_PCM_S24LE_PLANAR:
        case AV_CODEC_ID_PCM_S32LE_PLANAR:
        case AV_CODEC_ID_PCM_S16BE_PLANAR:*/
            AdecType = HA_AUDIO_ID_PCM;
            break;

		//case OMX_AUDIO_CodingFLAC:
		//case OMX_AUDIO_CodingRA:

    	default:
			AdecType = HA_AUDIO_ID_CUSTOM_1;
			ADEC_ERROR("\n[%s]  unknow type: %d\n", __FUNCTION__, audio_codec_id);
			break;
    	}
	ADEC_INFO("\n[%s]  AdecType:%d, p_ffmpeg_codec_id: 0x%x\n", __FUNCTION__, AdecType,*p_ffmpeg_codec_id);
    return AdecType;
}

mt_s32 MT_AVPlay_SetAdecAttr(AUD_DEC_HANDLE_T *p_adec_handle, 
			mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr;
    WAV_FORMAT_S stWavFormat;
    mt_handle hAvplay = p_adec_handle->avplay_handle;

    if(MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr) != MT_SUCCESS)
    {
        ADEC_ERROR("MT_UNF_AVPLAY_GetAttr error \n");
        return MT_FAILURE;
    }
    AdecAttr.enType = enADecType;

    if (HA_AUDIO_ID_PCM == AdecAttr.enType)
    {
        ADEC_INFO("\n[%s]  AdecType = HA_AUDIO_ID_PCM\n", __FUNCTION__);
        if(MT_TRUE == p_adec_handle->is_big_endian)
        {
            stWavFormat.cbSize = 4;
            stWavFormat.cbExtWord[0] = NORMAL_PCM_EXTWORD; //choose normal pcm decoder
            //stWavFormat.cbExtWord[0] = WIFIDSP_LPCM_EXTWORD; //choose wifi_dsp_lpcm decoder
        }
        stWavFormat.nSamplesPerSec = p_adec_handle->sample_rate;
        stWavFormat.nChannels = p_adec_handle->channels;
        stWavFormat.wBitsPerSample = p_adec_handle->bits;
	    stWavFormat.cbExtWord[0] = p_adec_handle->is_big_endian;
	    stWavFormat.cbExtWord[1] = p_adec_handle->channels >> 1;

	    stWavFormat.wFormatTag = 0xffee;
        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam),&stWavFormat);
        ADEC_INFO("pcm_header:is_big %d channels %d bits %d, samplerate %d \n",
            p_adec_handle->is_big_endian,p_adec_handle->channels, p_adec_handle->bits, p_adec_handle->sample_rate);
    }
#if 0
    else if (HA_AUDIO_ID_G711 == AdecAttr.enType)
    {
         HA_G711_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#endif
    else if (HA_AUDIO_ID_MP2 == AdecAttr.enType)
    {
         HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_AAC == AdecAttr.enType)
    {
         ADEC_INFO("zx HA_AAC_DecGetDefalutOpenParam");
         HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_MP3 == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#if 0
    else if (HA_AUDIO_ID_AMRNB== AdecAttr.enType)
    {
        AMRNB_DECODE_OPENCONFIG_S *pstConfig = (AMRNB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRNB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRNB_MIME;
    }
    else if (HA_AUDIO_ID_AMRWB== AdecAttr.enType)
    {
        AMRWB_DECODE_OPENCONFIG_S *pstConfig = (AMRWB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRWB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRWB_FORMAT_MIME;
    }
#endif
    else if (HA_AUDIO_ID_AC3PASSTHROUGH== AdecAttr.enType)
    {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_DTSPASSTHROUGH ==  AdecAttr.enType)
    {
                HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
             AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if (HA_AUDIO_ID_TRUEHD == AdecAttr.enType)
    {
        HA_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        if (HD_DEC_MODE_THRU != enMode)
        {
            ADEC_ERROR(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }

        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        ADEC_DEBUG(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if (HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DTSHD == AdecAttr.enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if (HA_AUDIO_ID_DTSM6 == AdecAttr.enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
#endif
    else if(HA_AUDIO_ID_DRA == AdecAttr.enType)
    {
//       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
         HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB ==AdecAttr.enType
        || HA_AUDIO_ID_AMRWB == AdecAttr.enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        ADEC_DEBUG("cook dec set ffmpeg dec param \n");
    }


    if(MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr) != MT_SUCCESS)
    {
        ADEC_ERROR("MT_UNF_AVPLAY_SetAttr error \n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/**
 * open audio decoder
 *
 * @param[out] pAvplay AVPlay AL handle
 * @param[out] pTrack audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_open0(MT_HANDLE *pAvplay, MT_HANDLE *pTrack)
{
	MT_S32 Ret;
//    MT_UNF_SND_ATTR_S stAttr;
	MT_HANDLE hAvplay;
    MT_HANDLE hTrack = 0;
//    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;

	ADEC_INFO("Enter %s\n",__FUNCTION__);
	CHECK_ARG((pAvplay == NULL || pTrack == NULL), MT_FAILURE);
	MUTEX_LOCK();

	if (g_adec_opened == 0)
	{
		Ret = avplayal_open(&hAvplay);
		if (Ret != MT_SUCCESS)
		{
			MUTEX_UNLOCK();
			return MT_FAILURE;
		}

#if 0
		ADEC_DEBUG("A OPEN[+1]: %s\n","MT_UNF_SND_Init");
	    Ret = MT_UNF_SND_Init();
	    if (Ret != MT_SUCCESS)
	    {
	        ADEC_ERROR("call MT_UNF_SND_Init failed.\n");
			MUTEX_UNLOCK();
			return MT_FAILURE;
	    }

		ADEC_DEBUG("A OPEN[+2]: %s\n","MT_UNF_SND_GetDefaultOpenAttr");
	    Ret = MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_0, &stAttr);
	    if (Ret != MT_SUCCESS)
	    {
	        ADEC_ERROR("call MT_UNF_SND_GetDefaultOpenAttr failed.\n");
            goto SND_DEINIT;
	    }

		ADEC_DEBUG("A OPEN[+3]: %s\n","MT_UNF_SND_Open");
	    Ret = MT_UNF_SND_Open(MT_UNF_SND_0, &stAttr);
	    if (Ret != MT_SUCCESS)
	    {
	        ADEC_ERROR("call MT_UNF_SND_Open failed.\n");
            goto SND_DEINIT;
	    }

		ADEC_DEBUG("A OPEN[+4]: %s\n","MT_UNF_AVPLAY_RegisterAcodecLib");
	    Ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYTRUEHD.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
#ifdef DOLBYPLUS_HACODEC_SUPPORT
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYPLUS.decode.so");
#endif
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSHD.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSM6.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
	    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");
        if (Ret != MT_SUCCESS)
        {
            ADEC_ERROR("call MT_UNF_AVPLAY_RegisterAcodecLib failed.\n");
            goto SND_CLOSE;
        }
#endif
		ADEC_DEBUG("A OPEN[+5]: %s\n","MT_UNF_AVPLAY_ChnOpen");
	    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	    if (Ret != MT_SUCCESS) {
			ADEC_ERROR("call MT_UNF_AVPLAY_ChnOpen failed!\n");
			goto SND_CLOSE;
	    }

#if 0
		ADEC_DEBUG("A OPEN[+6]: %s\n","MT_UNF_SND_GetDefaultTrackAttr");
	    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
	    if (Ret != MT_SUCCESS) {
			ADEC_ERROR("call MT_UNF_SND_GetDefaultTrackAttr failed!\n");
			goto ACHN_CLOSE;
	    }

		ADEC_DEBUG("A OPEN[+7]: %s\n","MT_UNF_SND_CreateTrack");
	    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);
	    if (Ret != MT_SUCCESS) {
			ADEC_ERROR("call MT_UNF_SND_CreateTrack failed!\n");
			goto ACHN_CLOSE;
	    }
		ADEC_INFO("%s: adec handle=0x%x\n",__FUNCTION__,hTrack);
#endif
		g_avplay_handle = hAvplay;
		g_adec_handle   = hTrack;
	}

	*pAvplay = g_avplay_handle;
	*pTrack  = g_adec_handle;

    g_adec_opened ++;
	ADEC_INFO("Leave %s: opened=%d\n",__FUNCTION__,g_adec_opened);
	MUTEX_UNLOCK();
	return MT_SUCCESS;

//ACHN_CLOSE:
	ADEC_DEBUG("A OPEN[-1]: %s\n","MT_UNF_AVPLAY_ChnClose");
	MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

SND_CLOSE:
	ADEC_DEBUG("A OPEN[-2]: %s\n","MT_UNF_SND_Close");
	MT_UNF_SND_Close(MT_UNF_SND_0);

//SND_DEINIT:
	ADEC_DEBUG("A OPEN[-3]: %s\n","MT_UNF_SND_DeInit");
    MT_UNF_SND_DeInit();

	MUTEX_UNLOCK();
	return MT_FAILURE;
}


/**
 * open audio decoder
 *
 * @param[out] pAvplay AVPlay AL handle
 * @param[out] pTrack audio decoder handle
 *
 * @retval
 *    audio_decoder_handle
 */
AUDIO_DECODER_HANDLE audio_decoder_open(void)
{
	MT_S32 Ret;
    MT_UNF_SND_ATTR_S stAttr;
    MT_HANDLE hAvplay = (MT_HANDLE)NULL;
    MT_HANDLE hTrack = (MT_HANDLE)NULL;
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;

	ADEC_INFO("Enter %s\n",__FUNCTION__);
	//CHECK_ARG((cfg == NULL ), NULL);

	//if(cfg->flags == AUDIO_FLAG_PASS_THROUGH ||cfg->flags == AUDIO_FLAG_TUNNEL_PLAYBACK)
		g_adecoder_h.flag_sound_init = 1;
	//else
	//	g_adecoder_h.flag_sound_init = 0;

	MUTEX_LOCK();

	if (g_adec_opened == 0)
	{
		Ret = avplayal_open(&hAvplay);
		if (Ret != MT_SUCCESS)
		{
			MUTEX_UNLOCK();
			return NULL;
		}

	    if(g_adecoder_h.flag_sound_init){
		    ADEC_DEBUG("A OPEN[+1]: %s\n","MT_UNF_SND_Init");
		    Ret = MT_UNF_SND_Init();
		    if (Ret != MT_SUCCESS)
		    {
		        ADEC_ERROR("call MT_UNF_SND_Init failed.\n");
				MUTEX_UNLOCK();
				return (AUDIO_DECODER_HANDLE)MT_FAILURE;
		    }

			ADEC_DEBUG("A OPEN[+2]: %s\n","MT_UNF_SND_GetDefaultOpenAttr");
		    Ret = MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_0, &stAttr);
		    if (Ret != MT_SUCCESS)
		    {
		        ADEC_ERROR("call MT_UNF_SND_GetDefaultOpenAttr failed.\n");
	            goto SND_DEINIT;
		    }

			ADEC_DEBUG("A OPEN[+3]: %s\n","MT_UNF_SND_Open");
		    Ret = MT_UNF_SND_Open(MT_UNF_SND_0, &stAttr);
		    if (Ret != MT_SUCCESS)
		    {
		        ADEC_ERROR("call MT_UNF_SND_Open failed.\n");
	            goto SND_DEINIT;
		    }

			ADEC_DEBUG("A OPEN[+4]: %s\n","MT_UNF_AVPLAY_RegisterAcodecLib");
		    Ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYTRUEHD.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
#ifdef DOLBYPLUS_HACODEC_SUPPORT
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYPLUS.decode.so");
#endif
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSHD.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSM6.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
		    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");
	          if (Ret != MT_SUCCESS)
	          {
	              ADEC_ERROR("call MT_UNF_AVPLAY_RegisterAcodecLib failed.\n");
	              goto SND_CLOSE;
	          }
	    }

	    ADEC_DEBUG("A OPEN[+5]: %s\n","MT_UNF_AVPLAY_ChnOpen");
	    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	    if (Ret != MT_SUCCESS) {
			ADEC_ERROR("call MT_UNF_AVPLAY_ChnOpen failed!\n");
			goto SND_CLOSE;
	    }

	    if(g_adecoder_h.flag_sound_init){
		    ADEC_DEBUG("A OPEN[+6]: %s\n","MT_UNF_SND_GetDefaultTrackAttr");
		    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
		    if (Ret != MT_SUCCESS) {
				ADEC_ERROR("call MT_UNF_SND_GetDefaultTrackAttr failed!\n");
				goto ACHN_CLOSE;
		    }

			ADEC_DEBUG("A OPEN[+7]: %s\n","MT_UNF_SND_CreateTrack");
            stTrackAttr.b_spdif_mod = MT_FALSE;
			if (0)
			{
				stTrackAttr.b_spdif_mod = MT_TRUE;
				ADEC_DEBUG("Pass-through: set SPDIF mode %d\n",stTrackAttr.b_spdif_mod);
			}
		    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);
		    if (Ret != MT_SUCCESS) {
				ADEC_ERROR("call MT_UNF_SND_CreateTrack failed!\n");
				goto ACHN_CLOSE;
		    }
			ADEC_INFO("%s: adec handle=0x%x\n",__FUNCTION__,hTrack);
	    }
		g_avplay_handle = hAvplay;
		g_adec_handle   = hTrack;
		g_adecoder_h.avplay_handle = g_avplay_handle;
		g_adecoder_h.track_handle = g_adec_handle;
	}


    g_adec_opened ++;
	ADEC_INFO("Leave %s end end: opened=%d, zx g_avplay_handle:0x%x, track_handle:0x%x\n",
        __FUNCTION__,g_adec_opened, g_avplay_handle, g_adec_handle);
	MUTEX_UNLOCK();
	return &g_adecoder_h;

ACHN_CLOSE:
	ADEC_DEBUG("A OPEN[-1]: %s\n","MT_UNF_AVPLAY_ChnClose");
	MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

SND_CLOSE:
	if(g_adecoder_h.flag_sound_init){
	    ADEC_DEBUG("A OPEN[-2]: %s\n","MT_UNF_SND_Close");
	    MT_UNF_SND_Close(MT_UNF_SND_0);
	}

SND_DEINIT:
	if(g_adecoder_h.flag_sound_init){
	    ADEC_DEBUG("A OPEN[-3]: %s\n","MT_UNF_SND_DeInit");
          MT_UNF_SND_DeInit();
	}

	MUTEX_UNLOCK();
	return NULL;
}

AUDIO_DECODER_HANDLE audio_decoder_get_handle(void)
{
    //ADEC_DEBUG("audio_decoder_get_handle, avplay_handle:0x%x\n", g_adecoder_h.avplay_handle);
    return &g_adecoder_h;
}

/**
 * close audio decoder
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_close(AUDIO_DECODER_HANDLE handle)
{
	ADEC_INFO("Enter %s\n",__FUNCTION__);
	CHECK_ARG((handle == INVALID_HANDLE), MT_FAILURE);
	AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
//	mt_handle hAvplay = p_adec_handle->avplay_handle;
	MT_HANDLE hTrack = p_adec_handle->track_handle;
	MUTEX_LOCK();

	if (g_adec_opened > 0 && hTrack == g_adec_handle)
	{
		g_adec_opened --;

		if (g_adec_opened == 0)
		{
			/*ADEC_DEBUG("A CLOSE[+1]: %s\n","MT_UNF_SND_DestroyTrack");
			MT_UNF_SND_DestroyTrack(handle);*/

			//FIXME: audio decoder handle -> avplay al handle
			ADEC_DEBUG("A CLOSE[+2]: %s\n","MT_UNF_AVPLAY_ChnClose");
			MT_UNF_AVPLAY_ChnClose(g_avplay_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

			if(g_adecoder_h.flag_sound_init){
				ADEC_DEBUG("A CLOSE[+3]: %s\n","MT_UNF_SND_Close");
				MT_UNF_SND_Close(MT_UNF_SND_0);

				ADEC_DEBUG("A CLOSE[+4]: %s\n","MT_UNF_SND_DeInit");
				MT_UNF_SND_DeInit();

				ADEC_INFO("%s: close handle(0x%x) success.\n",__FUNCTION__,hTrack);
			}

			avplayal_close(g_avplay_handle);

			g_adec_handle   = INVALID_HANDLE;
			g_avplay_handle = INVALID_HANDLE;
			memset(&g_adecoder_h, 0, sizeof(AUD_DEC_HANDLE_T));
		}

		ADEC_INFO("%s: opened=%d\n",__FUNCTION__,g_adec_opened);
	}

	MUTEX_UNLOCK();
	ADEC_INFO("Leave %s\n",__FUNCTION__);
	return MT_SUCCESS;
}

/**
 * start audio decoder
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_start(AUDIO_DECODER_HANDLE handle)
{
	  mt_s32 ret = MT_SUCCESS;
	  AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	  mt_handle hAvplay = p_adec_handle->avplay_handle;
	  MT_HANDLE hTrack = p_adec_handle->track_handle;
	  //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;
	  mt_u32 AdecType = Get_Atype(p_adec_handle->adec_type, &(p_adec_handle->adec_ffmpeg_type));
	  mt_u32 FfmpegType = p_adec_handle->adec_ffmpeg_type;

	  mt_s32 s32DtsDtsCoreOnly = 0;
	  MT_HA_DECODEMODE_E enAudioDecMode = HD_DEC_MODE_RAWPCM;
	  MT_UNF_ACODEC_ATTR_S    AdecAttr;
	  HA_FFMPEG_DECODE_OPENCONFIG_S ffmpeg_dec_opencfg;

	  //pthread_mutex_lock(pVMutex);
	  MUTEX_LOCK();

	  if(g_adecoder_h.flag_sound_init){
	      ADEC_INFO("%s: %s, hAvplay:0x%x, hTrack:0x%x\n", __FUNCTION__,"MT_UNF_SND_Attach", hAvplay, hTrack);
		  ret = MT_UNF_SND_Attach(hTrack, hAvplay);
		  if (ret != MT_SUCCESS)
		  {
		    ADEC_ERROR("MT_UNF_SND_Attach fail\n");
	        goto A_START_FAILED;
		  }
	  }

          ADEC_INFO("%s: %s\n", __FUNCTION__,"MT_AVPlay_SetAdecAttr");
	   ret = MT_AVPlay_SetAdecAttr(p_adec_handle, AdecType, enAudioDecMode, s32DtsDtsCoreOnly);
	    if (ret != MT_SUCCESS) {
		ADEC_ERROR("MT_AVPlay_SetAdecAttr failed\n");
              goto A_START_FAILED;
	    }

#if 1// for ffmpeg decoders
	 if(AdecType == HA_AUDIO_ID_COOK){
         ADEC_INFO("HA_AUDIO_ID_COOK MT_UNF_AVPLAY_SetAttr \n");
		 ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

			memset(&ffmpeg_dec_opencfg, 0, sizeof(HA_FFMPEG_DECODE_OPENCONFIG_S));
	        HA_FFMPEG_DecGetDefalutOpenConfig(&ffmpeg_dec_opencfg);
	        ffmpeg_dec_opencfg.ffmpeg_info.codec_id = FfmpegType;
	        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), &ffmpeg_dec_opencfg);
	        ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
	        if (MT_SUCCESS != ret)
	        {
	            ADEC_ERROR("call MT_UNF_AVPLAY_SetAttr failed.\n");
	            goto A_START_FAILED;
	        }
	        ADEC_INFO("finish MT_UNF_AVPLAY_SetAttr success, codec_id: 0x%x\n", FfmpegType);
	 	}
#endif

         ADEC_INFO("%s: %s\n", __FUNCTION__,"MT_UNF_AVPLAY_Start");
	  ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	  if (ret != MT_SUCCESS)
	  {
	    ADEC_ERROR(" MT_UNF_AVPLAY_Start failed.\n");
           goto A_START_FAILED;
	  }

	   //pthread_mutex_unlock(pVMutex);
	   MUTEX_UNLOCK();

       MT_UNF_AVPLAY_STATUS_INFO_S status;
       MT_UNF_AVPLAY_GetStatusInfo(hAvplay,&status);
	   ADEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);

       mon_sync_init(OMX_OnlyAudio);
       g_pts_info.type = OMX_OnlyAudio;
       ADEC_INFO("Leave %s end end.\n",__FUNCTION__);
       g_stop_flag = 0;
	return MT_SUCCESS;

A_START_FAILED:
    //pthread_mutex_unlock(pVMutex);
    MUTEX_UNLOCK();
	return MT_FAILURE;

}

/**
 * stop audio decoder
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_stop(AUDIO_DECODER_HANDLE handle)
{
	   mt_s32 ret = MT_SUCCESS;
	  AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	  mt_handle hAvplay = p_adec_handle->avplay_handle;
	  MT_HANDLE hTrack = p_adec_handle->track_handle;
	  //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;
      g_stop_flag = 1;
      g_check_task_stop = 1;

	  MT_UNF_AVPLAY_STOP_OPT_S stop;

	  //pthread_mutex_lock(pVMutex);
	  mon_sync_deinit(OMX_OnlyAudio);

	  MUTEX_LOCK();

	  //close avplay
         ADEC_INFO("%s: %s\n", __FUNCTION__,"MT_UNF_AVPLAY_Stop");
	  stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;//�������ò���
	  stop.u32TimeoutMs = 0;
	  ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD,&stop);
	  if (ret != MT_SUCCESS)
	  {
	    ADEC_ERROR(" MT_UNF_AVPLAY_Stop failed.\n");
	    goto A_STOP_FAILED;
	  }

	 if(g_adecoder_h.flag_sound_init){
	        ADEC_INFO("%s: %s\n", __FUNCTION__,"MT_UNF_SND_Detach");
		  ret = MT_UNF_SND_Detach(hTrack, hAvplay);
		  if (ret != MT_SUCCESS)
		  {
		    ADEC_ERROR("MT_UNF_SND_Detach fail\n");
		    goto A_STOP_FAILED;
		  }
 	  }

	   //pthread_mutex_unlock(pVMutex);
	   MUTEX_UNLOCK();

	   return MT_SUCCESS;

A_STOP_FAILED:

	     //pthread_mutex_unlock(pVMutex);
	     MUTEX_UNLOCK();
	     return MT_FAILURE;
}


/**
 * audio_decoder_pause
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_pause(AUDIO_DECODER_HANDLE handle)
{
	    mt_s32 ret = MT_SUCCESS;
	    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	    mt_handle hAvplay = p_adec_handle->avplay_handle;
//	    MT_HANDLE hTrack = p_adec_handle->track_handle;
	    //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	    MT_UNF_AVPLAY_STATUS_INFO_S status;

	    MT_UNF_AVPLAY_GetStatusInfo(hAvplay,&status);
	    ADEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);
	    if(status.enRunStatus != MT_UNF_AVPLAY_STATUS_PAUSE)
	    {
	    	 //ADEC_INFO("%s,%d\n", __func__,__LINE__);
	        //pthread_mutex_lock(pVMutex);
	        MUTEX_LOCK();
		 //ADEC_INFO("%s,%d\n", __func__,__LINE__);
	        ret =  MT_UNF_AVPLAY_Pause(hAvplay, MT_NULL);
		 if (ret != MT_SUCCESS)
		  {
		    ADEC_ERROR("MT_UNF_AVPLAY_Pause fail\n");
		  }

            ADEC_INFO("%s,%d, call MT_UNF_AVPLAY_Pause, g_stop_flag:%d\n", __func__,__LINE__,g_stop_flag);
	        //pthread_mutex_unlock(pVMutex);
	        MUTEX_UNLOCK();
	    }
        else if(status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE)
        {

        }

	return ret;
}


/**
 * audio_decoder_resume
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_resume(AUDIO_DECODER_HANDLE handle)
{
	    mt_s32 ret = MT_SUCCESS;
	    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	    mt_handle hAvplay = p_adec_handle->avplay_handle;
//	    MT_HANDLE hTrack = p_adec_handle->track_handle;
	    //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	    MT_UNF_AVPLAY_STATUS_INFO_S status;

	    MT_UNF_AVPLAY_GetStatusInfo(hAvplay,&status);
	    ADEC_INFO("%s,%d status.enRunStatus:%d\n", __func__,__LINE__,status.enRunStatus);
	    if(status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE)
	    {
	        //pthread_mutex_lock(pVMutex);
	        MUTEX_LOCK();
	        ret =  MT_UNF_AVPLAY_Resume(hAvplay, MT_NULL);
		  if (ret != MT_SUCCESS)
		  {
		    	ADEC_ERROR("MT_UNF_AVPLAY_Resume fail\n");
	       }

	        //pthread_mutex_unlock(pVMutex);
	        MUTEX_UNLOCK();
	    }
	    return ret;
}

/**
 * audio_decoder_flush
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_flush(AUDIO_DECODER_HANDLE handle, MT_S32 flush_dir)
{
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    mt_s32 s32Ret = -1;
    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	mt_handle hAvplay = p_adec_handle->avplay_handle;
    MT_UNF_AVPLAY_GetStatusInfo(hAvplay, &status);
    ADEC_DEBUG("[%s]  status.enRunStatus:%d, use Flush_Audio, avhandle[0x%x]\n", __FUNCTION__, status.enRunStatus, hAvplay);
    if((status.enRunStatus == MT_UNF_AVPLAY_STATUS_PLAY)||(status.enRunStatus == MT_UNF_AVPLAY_STATUS_PAUSE))
    {
        flush_status = FLUSH_START;
        //MT_USLEEP(3000);
        //s32Ret = MT_UNF_AVPLAY_Flush(hAvplay);
        s32Ret = MT_UNF_AVPLAY_Flush_Audio(hAvplay);//audio only flush auido self
    }
    mon_sync_init(OMX_OnlyVideo);
    ADEC_DEBUG("[%s]  end end\n", __FUNCTION__);
    return s32Ret;
}

MT_S32 audio_decoder_flush_stop(void)
{
    flush_status = FLUSH_END;
    return MT_SUCCESS;
}
#if 0
/**
 * audio_decoder_push_es
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_push_es1(AUDIO_DECODER_HANDLE handle, const audio_decoder_buffer_t *buffer)//(MT_HANDLE handle, mt_u8 *p_data, mt_s32 len, mt_u32 pts)
{
	    mt_s32 ret = MT_SUCCESS;
	    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	    mt_handle hAvplay = p_adec_handle->avplay_handle;
	    MT_HANDLE hTrack = p_adec_handle->track_handle;
	    //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	    mt_u8 *p_addr = buffer->addr;
	    int audioFrameLen = buffer->len;
	    mt_u32 apts = buffer->pts/1000;

	    MT_UNF_STREAM_BUF_S   StreamBuf;
	    mt_s32 timecount = 0;


         //ADEC_INFO("%s,%d \n", __func__,__LINE__);
	  //ADEC_INFO("%s,%d  pVMutex: %p, drv_ctx->pVMutex: %p\n", __func__,__LINE__,pVMutex, drv_ctx->pVMutex);
	  //pthread_mutex_lock(pVMutex);
	  //ADEC_INFO("%s,%d\n", __func__,__LINE__);
	  while (audioFrameLen > 0)
	  {
		   if(g_stop_flag == 1)
		   		break;

		    ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, audioFrameLen, &StreamBuf, TIME_OUT_MS_ES_PUSH);
		    if(MT_SUCCESS != ret || StreamBuf.u32Size <= 0)
		    {
			      //ADEC_ERROR("mem buf get failed:reqlen=%d ret = %d@%s,line%d\n",audioFrameLen,ret ,__func__, __LINE__);

			     if( ++timecount > PUSH_DATA_TIME_COUNT){
				 	ADEC_ERROR("[%s] get buf failed %d times -a-\n",__func__, timecount);
					timecount = 0;
					ADEC_ERROR("[%s] return\n",__func__, timecount);
					//pthread_mutex_unlock(pVMutex);
			             return AUD_ESBUF_GETBUF_FAILED;
			     	}

			        //MT_USLEEP(1000);
		      		continue;
		    }
		    //ADEC_INFO("[%s] -dd- timecount:%d, StreamBuf.u32Size:%d\n",__func__, timecount, StreamBuf.u32Size);
		    timecount = 0;
		    /*push es data*/
		    if(StreamBuf.u32Size >= audioFrameLen)
		    {
		      		StreamBuf.u32Size = audioFrameLen;
		    }
		    memcpy(StreamBuf.pu8Data,p_addr, StreamBuf.u32Size);
		    //ADEC_INFO("push to adec: StreamBuf.pu8Data = 0x%x,StreamBuf.u32Size = 0%d, apts=%d\n",StreamBuf.pu8Data,StreamBuf.u32Size, apts);
		    ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, StreamBuf.u32Size, apts);//
		    if(MT_SUCCESS != ret)
		    {
		      		ADEC_ERROR("[%s] ERROR: audio es input failed: ret = %d\n",__FUNCTION__, ret );
                    //pthread_mutex_unlock(pVMutex);
				    return AUD_ESBUF_PUTBUF_FAILED;
		    }
		    audioFrameLen -= StreamBuf.u32Size;
		    p_addr += StreamBuf.u32Size;
	  }
	 // pthread_mutex_unlock(pVMutex);


	return MT_SUCCESS;
}
#endif

void audio_get_pts(AUDIO_DECODER_HANDLE handle, int64_t *out_pts)
{
    mt_s32 ret = MT_SUCCESS;
    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	mt_handle hAvplay = p_adec_handle->avplay_handle;
    MT_UNF_AVPLAY_STATUS_INFO_S statusInfo;
    ret = MT_UNF_AVPLAY_GetStatusInfo(hAvplay, &statusInfo);
    if(ret != MT_SUCCESS)
    {
        ADEC_ERROR("@@E: get video info error @line %d,fun %s",__LINE__,__func__);
    }

    *out_pts = (int64_t)statusInfo.stSyncStatus.u64LastAudPts * 1000;
    //ADEC_DEBUG("%s: u64LastAudPts[%lld] u64LastVidPts[%lld]\n",__func__, (long long)*out_pts, (long long)statusInfo.stSyncStatus.u64LastVidPts);
    return;
}



/**
 * audio_decoder_push_es
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_push_es(AUDIO_DECODER_HANDLE handle, void *p1)//(MT_HANDLE handle, mt_u8 *p_data, mt_s32 len, mt_u32 pts)
{
	    mt_s32 ret = MT_SUCCESS;
	    AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
        audio_decoder_buffer_t *buffer = (audio_decoder_buffer_t *)p1;
	    mt_handle hAvplay = p_adec_handle->avplay_handle;
//	    MT_HANDLE hTrack = p_adec_handle->track_handle;
	    //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;
	    mt_u32 eos = p_adec_handle->eos_flag;
	    mt_u8 *p_addr = buffer->addr;
	    int audioFrameLen = buffer->len;
	    MT_UNF_AVPLAY_PUTBUFEX_OPT_S ext_opt;

	    MT_UNF_STREAM_BUF_S   StreamBuf;
	    mt_s32 timecount = 0;
	    mt_u32 fake_eos_data = 0;//4//4byte
//	    mt_u32 count = 0;


         //ADEC_INFO("%s,%d \n", __func__,__LINE__);
	  //ADEC_INFO("%s,%d  pVMutex: %p, drv_ctx->pVMutex: %p\n", __func__,__LINE__,pVMutex, drv_ctx->pVMutex);
	  //pthread_mutex_lock(pVMutex);
	  //ADEC_INFO("%s,%d\n", __func__,__LINE__);
	   if(audioFrameLen==0 && eos==1){
			audioFrameLen = 4;
			p_addr = (mt_u8 *)&fake_eos_data;
			ADEC_INFO("[%s]  eos:%d \n", __FUNCTION__,eos);
	   }

        /*while(mon_handle_audio(buffer->pts) == 0)
    	{
    		count++;
    		if(count >= 3000){	//3s
    			ADEC_ERROR("%s: ERROR wait 3s video_pts[%lld] audio_pts[%lld]\n",
                    __func__, g_pts_info.video_pts, g_pts_info.audio_pts);
    			count = 0;
    		}
            if(flush_status == FLUSH_START || g_stop_flag == 1)
                return MT_SUCCESS;
    		MT_USLEEP(1000);	//10ms
    	}*/

      //ADEC_INFO("%s: %s, hAvplay:0x%x, hTrack:0x%x\n", __FUNCTION__,"MT_UNF_SND_Attach", hAvplay, hTrack);
	  while (audioFrameLen >0)
	  {
		   if(g_stop_flag == 1)
		   		break;

           if(flush_status == FLUSH_START)
                return MT_SUCCESS;

		    ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, audioFrameLen, &StreamBuf, TIME_OUT_MS_ES_PUSH);
		    if(MT_SUCCESS != ret || StreamBuf.u32Size <= 0)
		    {
			      //ADEC_ERROR("mem buf get failed:reqlen=%d ret = %d@%s,line%d\n",audioFrameLen,ret ,__func__, __LINE__);

			     if( ++timecount > PUSH_DATA_TIME_COUNT){
				 	ADEC_ERROR("[%s] get buf failed %d times -a-\n",__func__, timecount);
					timecount = 0;
					ADEC_ERROR("[%s] return, timecount = %d\n",__func__, timecount);
					//pthread_mutex_unlock(pVMutex);
			             return AUD_ESBUF_GETBUF_FAILED;
			     	}

			        MT_USLEEP(1000);
		      		continue;
		    }
		    //ADEC_INFO("[%s] -dd- timecount:%d, StreamBuf.u32Size:%d\n",__func__, timecount, StreamBuf.u32Size);
		    timecount = 0;
		    /*push es data*/
		    if(StreamBuf.u32Size >= audioFrameLen)
		    {
		      		StreamBuf.u32Size = audioFrameLen;
		    }
		    memcpy(StreamBuf.pu8Data,p_addr, StreamBuf.u32Size);


		    //ADEC_INFO("push to adec: StreamBuf.pu8Data = 0x%x,StreamBuf.u32Size = 0%d, apts=%lld\n",StreamBuf.pu8Data,StreamBuf.u32Size, buffer->pts);
		    //ret = MT_UNF_AVPLAY_PutBuf_A(hAvplay, StreamBuf.u32Size, apts, eos);
		    memset(&ext_opt, 0, sizeof(ext_opt));
		    ext_opt.bContinue = MT_TRUE;
		    ext_opt.bEndOfFrm = MT_TRUE;
		    ext_opt.u32PtsValide = 0;
		    ext_opt.u32FrameFinsh = 1;
		    ext_opt.u32EosFlag = eos;
            //pts/1000  gst ns, need us
		    ret = MT_UNF_AVPLAY_64BitPTS_PutBufEx(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, StreamBuf.u32Size, buffer->pts/1000, &ext_opt);
		    if(MT_SUCCESS != ret)
		    {
		      		ADEC_ERROR("[%s] ERROR: audio es input failed: ret = %d\n",__FUNCTION__, ret );
                    //pthread_mutex_unlock(pVMutex);
				    return AUD_ESBUF_PUTBUF_FAILED;
		    }
		    audioFrameLen -= StreamBuf.u32Size;
		    p_addr += StreamBuf.u32Size;
	  }
	 // pthread_mutex_unlock(pVMutex);

    //mon_update_apts(buffer->pts);
	return MT_SUCCESS;
}


/**
 * audio_decoder_get_pcm
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_get_pcm(AUDIO_DECODER_HANDLE handle, 
					MT_UNF_AO_FRAMEINFO_S *p_ao_fram)
{
	 mt_s32 ret = MT_SUCCESS;
	 AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	 mt_handle hAvplay = p_adec_handle->avplay_handle;
	 //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	 //ADEC_INFO(" [%s] get lock", __FUNCTION__);
	 //pthread_mutex_lock(pVMutex);
	 //ADEC_INFO(" [%s] call MT_UNF_AVPLAY_GetFrame", __FUNCTION__);
	 ret = MT_UNF_AVPLAY_GetFrame(hAvplay, p_ao_fram);
	 //ADEC_INFO("[%s] out of  MT_UNF_AVPLAY_GetFrame", __FUNCTION__);
	 //pthread_mutex_unlock(pVMutex);

	 if(ret != MT_SUCCESS)
	 	return MT_FAILURE;

	return MT_SUCCESS;
}


/**
 * audio_decoder_release_pcm
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_release_pcm(AUDIO_DECODER_HANDLE handle, 
						MT_UNF_AO_FRAMEINFO_S *p_ao_fram)
{
	 AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	 mt_handle hAvplay = p_adec_handle->avplay_handle;
	 //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	 //ADEC_INFO(" [%s] get lock", __FUNCTION__);
	 //pthread_mutex_lock(pVMutex);
	 //ADEC_INFO(" [%s] call releaseFrame", __FUNCTION__);
	 MT_UNF_AVPLAY_ReleaseFrame(hAvplay, p_ao_fram);
	 //ADEC_INFO(" [%s] out of releaseFrame", __FUNCTION__);
	// pthread_mutex_unlock(pVMutex);
	 return MT_SUCCESS;
}




/**
 * audio_decoder_check_buffer_empty
 *
 * @param[in] handle audio decoder handle
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_check_buffer_empty(AUDIO_DECODER_HANDLE handle, MT_BOOL *p_empty)
{
	 AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	 mt_handle hAvplay = p_adec_handle->avplay_handle;
	 //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;

	MT_UNF_AVPLAY_IsBuffEmpty(hAvplay, p_empty);
	return MT_SUCCESS;

}


/**
 * audio_decoder_reset_buffer
 *
 * @param[in] handle: audio decoder handle
 * @param[in] reset_flag: reset direction, 0:buf_in, 1:buf_out
 *
 * @retval
 *    MT_SUCCESS: success
 *    MT_FAILURE: failed
 */
MT_S32 audio_decoder_reset_buffer(AUDIO_DECODER_HANDLE handle, mt_s32 reset_flag)
{
	 AUD_DEC_HANDLE_T *p_adec_handle = (AUD_DEC_HANDLE_T *)handle;
	 mt_handle hAvplay = p_adec_handle->avplay_handle;
	 //pthread_mutex_t* pVMutex = p_adec_handle->pVMutex;
	 MT_UNF_AVPLAY_MEDIA_BUF_E buf_type;
	 if(reset_flag == 0)
	 	buf_type = MT_UNF_AVPLAY_MEDIA_BUF_IN;
	 else if(reset_flag == 1)
	 	buf_type = MT_UNF_AVPLAY_MEDIA_BUF_OUT;
	 else{
	 	ADEC_ERROR("[%s] bad param, return\n", __FUNCTION__);
		return MT_FAILURE;
	 }

	 ADEC_INFO(" [%s] start, reset_flag: %d", __FUNCTION__, reset_flag);
	 //pthread_mutex_lock(pVMutex);
	 MUTEX_LOCK();
	 MT_UNF_AVPLAY_Reset_Buffer( hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD , buf_type, NULL);
	 //pthread_mutex_unlock(pVMutex);
	 MUTEX_UNLOCK();
	// ADEC_INFO(" [%s] end", __FUNCTION__);

	 return MT_SUCCESS;

}

