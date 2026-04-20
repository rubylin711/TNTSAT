/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#if 0
//unused
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/times.h>
#include "mt_type.h"
#include "mt_unf_sound.h"
#include "mtsu_svr_player.h"
#include "suplayer_internal.h"
#include "suplayer_hw.h"
#include "mt_type.h"
#include "mpcommon.h"
#include "demux_mp.h"
#include "libmpdemux/stheader.h"

#include "mt_unf_avplay.h"
#include "mt_unf_vo.h"
#include "mt_error_mpi.h"
#include "mt_adp_mpi.h"


#define mon_error printf
#define mon_debug printf
#define mon_keylog printf

static mt_handle hWin;
static mt_handle hTrack;

/*******************************  Video Related *****************************/


static int Get_AVtype(void *hPlayer)
{
    int biComp = 0;
    char *p_str = NULL;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hPlayer;
    demux_stream_t *ds_v = p_MonPlayer->p_demuxer_video;
    demux_stream_t *ds_a = p_MonPlayer->p_demuxer_audio;
    sh_audio_t *p_sh_audio = ds_a->sh;
    sh_video_t *p_sh_video = ds_v->sh;

    if (p_sh_video && p_sh_video->bih) {
	biComp = le2me_32(p_sh_video->bih->biCompression);
	p_str = (char *)(&biComp);

	if (strstr(((char *)&biComp), "mpg2") || strstr(((char *)&biComp), "mpg1")) {
	    p_MonPlayer->vdec_type = MT_UNF_VCODEC_TYPE_MPEG2;
	} else if (p_sh_video->bih->biCompression == 1) {
	    p_MonPlayer->vdec_type = MT_UNF_VCODEC_TYPE_MPEG2;
	} else if (strstr(((char *)&biComp), "MPG2")) {
	    p_MonPlayer->vdec_type = MT_UNF_VCODEC_TYPE_MPEG2;
	} else if ((strstr(((char *)&biComp), "H264")) || (strstr(((char *)&biComp), "avc1")) || (strstr(((char *)&biComp), "h264"))) {
	    p_MonPlayer->vdec_type = MT_UNF_VCODEC_TYPE_H264;
	} else if (strstr(((char *)&biComp), "RV30")) {
	} else if (strstr(((char *)&biComp), "RV40")) {
	} else if (strstr(((char *)&biComp), "VP80")) {
	} else {
	    if ((strstr(((char *)&biComp), "WVC1")) || (strstr(((char *)&biComp), "WMV"))) {
	    } else if ((strstr(((char *)&biComp), "MP4V")) || (strstr(((char *)&biComp), "XVID")) ||
	               (strstr(((char *)&biComp), "DIV") && !strstr(((char *)&biComp), "DIV3")) ||
	               (strstr(((char *)&biComp), "DX")) || (strstr(((char *)&biComp), "s263"))) {
		p_MonPlayer->vdec_type = MT_UNF_VCODEC_TYPE_MPEG4;
	    }
	}
    }

    if (p_sh_audio) {
	if (p_sh_audio->format == 0x55 || p_sh_audio->format == 0x50) {
	    p_MonPlayer->adec_type = HA_AUDIO_ID_MP3;
	}

	if (p_sh_audio->format == ((int)MKTAG('M', 'P', '4', 'A')) ||
	    p_sh_audio->format == MKTAG('M', 'P', '4', 'A') ||
	    p_sh_audio->format == MKTAG('m', 'p', '4', 'l') ||
	    p_sh_audio->format == MKTAG('M', 'P', '4', 'L')) {
	    p_MonPlayer->adec_type = HA_AUDIO_ID_AAC;
	}
    }

    return 0;
}

MO_S32 Mon_Decoder_Deinit(void *hPlayer)
{
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hPlayer;
    mt_handle hAvplay = p_MonPlayer->hAvplay;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;
    Stop.u32TimeoutMs = 0;
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);
    MT_UNF_SND_Detach(hTrack, hAvplay);
    MT_UNF_SND_DestroyTrack(hTrack);
    MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &Stop);
    MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    MT_UNF_VO_DetachWindow(hWin, hAvplay);
    MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    MT_UNF_AVPLAY_Destroy(hAvplay);
    MT_UNF_AVPLAY_DeInit();
    MTADP_Snd_DeInit();
    MT_UNF_VO_DestroyWindow(hWin);
    MTADP_VO_DeInit();
    mt_sys_deinit();
    return 0;
}
MO_S32 Mon_Decoder_Init(void *hPlayer)
{
    mt_s32 Ret;
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;
    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_BUTT;
    mt_s32 AdecType = 0;
    mt_handle hAvplay;
    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;
    MT_HA_DECODEMODE_E enAudioDecMode = HD_DEC_MODE_RAWPCM;
    mt_s32 s32DtsDtsCoreOnly = 0;
    MT_UNF_ENC_FMT_E g_enDefaultFmt = MT_UNF_ENC_FMT_1080i_50;//MT_UNF_ENC_FMT_720P_50;
    MT_BOOL bAdvancedProfil = 1;
    mt_s32 u32CodecVersion = 8;
    mt_s32 frame_rate = 0;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hPlayer;
    Get_AVtype(hPlayer);
    VdecType = p_MonPlayer->vdec_type;
    AdecType = p_MonPlayer->adec_type;
    mt_sys_init();
    MTADP_MCE_Exit();
    Ret = MT_UNF_AVPLAY_Init();

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_Init not success %s %d\n", __FUNCTION__, __LINE__);
	 goto MON_DEC_INIT_ERR;
    }

    Ret = MTADP_Snd_Init();

    if (Ret != MT_SUCCESS) {
	mon_error("\n call SndInit not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MTADP_AVPlay_RegADecLib();

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MOHW_AVPlay_RegADecLib not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_Create not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

    if (MT_SUCCESS != Ret) {
	mon_error("\n MT_UNF_AVPLAY_SetAttr not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_ChnOpen not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_SND_GetDefaultTrackAttr not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_SND_CreateTrack not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_SND_Attach(hTrack, hAvplay);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_SND_Attach not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MTADP_AVPlay_SetAdecAttr(hAvplay, AdecType, enAudioDecMode, s32DtsDtsCoreOnly);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MOHW_AVPlay_SetAdecAttr not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_Start not success %s %d\n", __FUNCTION__, __LINE__);

    }

    Ret = MTADP_Disp_Init(g_enDefaultFmt);

    if (Ret != MT_SUCCESS)
    {
        mon_error("\n call MTADP_Disp_Init not success %s %d\n", __FUNCTION__, __LINE__);
        goto MON_DEC_INIT_ERR;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL, &hWin);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MOHW_VO_Init not success %s %d\n", __FUNCTION__, __LINE__);
	MTADP_VO_DeInit();
	goto MON_DEC_INIT_ERR;
    }

    MT_UNF_AVPLAY_OPEN_OPT_S *pMaxCapbility = MT_NULL;
    MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;

    if (MT_UNF_VCODEC_TYPE_MVC == VdecType) {
	stMaxCapbility.enCapLevel = MT_UNF_VCODEC_CAP_LEVEL_FULLHD;
	stMaxCapbility.enDecType = MT_UNF_VCODEC_DEC_TYPE_BUTT;
	stMaxCapbility.enProtocolLevel = MT_UNF_VCODEC_PRTCL_LEVEL_MVC;
	pMaxCapbility = &stMaxCapbility;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, pMaxCapbility);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_ChnOpen not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    /*set compress attr*/
    MT_UNF_VCODEC_ATTR_S VcodecAttr;
    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

    if (MT_UNF_VCODEC_TYPE_VC1 == VdecType) {
	VcodecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = bAdvancedProfil;
	VcodecAttr.unExtAttr.stVC1Attr.u32CodecVersion = u32CodecVersion;
    }

    if (MT_UNF_VCODEC_TYPE_VP6 == VdecType) {
	VcodecAttr.unExtAttr.stVP6Attr.bReversed = 0;
    }

    VcodecAttr.enType = VdecType;
    Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);

    if (MT_SUCCESS != Ret) {
	mon_error("\n call MT_UNF_AVPLAY_SetAttr not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_VO_AttachWindow not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_VO_SetWindowEnable not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    Ret = MTADP_AVPlay_SetVdecAttr(hAvplay, VdecType, MT_UNF_VCODEC_MODE_NORMAL);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MOHW_AVPlay_SetVdecAttr not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }
    mon_debug("\n  %d\n", __FUNCTION__, __LINE__);

    if (0 != frame_rate) {
	MT_UNF_AVPLAY_FRMRATE_PARAM_S stFramerate;
	stFramerate.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_USER;
	stFramerate.stSetFrmRate.u32fpsInteger = frame_rate;
	stFramerate.stSetFrmRate.u32fpsDecimal = 0;
	MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFramerate);
    }
    mon_debug("\n  %d\n", __FUNCTION__, __LINE__);

    Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    mon_debug("\n [%s]  %d\n", __FUNCTION__, __LINE__);

    if (Ret != MT_SUCCESS) {
	mon_error("\n call MT_UNF_AVPLAY_Start not success %s %d\n", __FUNCTION__, __LINE__);
	goto MON_DEC_INIT_ERR;
    }

    p_MonPlayer->hAvplay = hAvplay;
    return 0;


MON_DEC_INIT_ERR:

    return -1;


}
int Is_Decoder_End(void *hMonplay)
{
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hMonplay;
    void *hAvplay = p_MonPlayer->hAvplay;
    MT_BOOL bIsEmpty = MT_FALSE;
    MO_S32 Ret;
    int is_end = 0;
    Ret = MT_UNF_AVPLAY_IsBuffEmpty(hAvplay, &bIsEmpty);

    if (Ret == MT_SUCCESS) {
	is_end = bIsEmpty;
    }

    return is_end;
}
int Push_Audio_ES(void *hMonplay)
{
    MO_S32 Ret;
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hMonplay;
    int audio_size = p_MonPlayer->audio_es_size;

    if (audio_size > 0) {
	void *hAvplay = p_MonPlayer->hAvplay;
	unsigned int *p_extra_aud_buf = p_MonPlayer->p_extra_aud_buf;
	unsigned int extra_audio_size = p_MonPlayer->extra_audio_size;
	int *audio_start = p_MonPlayer->audio_es_start;
	MO_U32 Readlen;
	MT_UNF_STREAM_BUF_S AudioBuf;
	Readlen = audio_size + extra_audio_size;
	Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, Readlen, &AudioBuf, 0);

	if (MT_SUCCESS == Ret) {
	    if (extra_audio_size > 0) {
		memcpy(AudioBuf.pu8Data, p_extra_aud_buf, sizeof(mt_s8) * extra_audio_size);
	    }

	    memcpy(AudioBuf.pu8Data + extra_audio_size, audio_start, sizeof(mt_s8) * audio_size);
	    Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, Readlen, 0);

	    if (Ret == MT_SUCCESS) {
		p_MonPlayer->audio_es_size = 0;
	    } else {
		mon_error("\n not success %s %d\n", __FUNCTION__, __LINE__);
	    }
	}
    }

    return 0;
}
int Push_Video_ES(void *hMonplay)
{
    MT_PLAYBACK_INTERNAL_T *p_MonPlayer = hMonplay;
    MO_S32 Ret;
    int video_size = p_MonPlayer->video_es_size;
    void *hAvplay = p_MonPlayer->hAvplay;

    if (video_size > 0) {
	MT_UNF_STREAM_BUF_S VideoBuf;
	Ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, video_size, &VideoBuf, 0);

	if (Ret == MT_ERR_VDEC_BUFFER_FULL) {
	    MT_USLEEP(1000);
	} else if (MT_SUCCESS == Ret) {
	    memcpy(VideoBuf.pu8Data, p_MonPlayer->video_es_start, sizeof(mt_s8) * video_size);
	    Ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID, video_size, 0);

	    if (MT_SUCCESS == Ret) {
		p_MonPlayer->video_es_size = 0;
	    } else {
		mon_error("\n%s %d\n", __FUNCTION__, __LINE__);
	    }
	}
    }

    return 0;
}
#endif
