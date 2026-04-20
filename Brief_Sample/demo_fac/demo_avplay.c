/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mt_type.h"
#include "mt_unf_sound.h"
#include "mt_unf_vo.h"
#include "mt_unf_disp.h"
#include "mt_unf_demux.h"
#include "mt_unf_avplay.h"
#include "HA.AUDIO.MP3.decode.h"
#include "demo.h"

mt_handle g_sound_track = 0;
mt_handle g_vo_window = 0;
mt_handle g_av_play = 0;

mt_s32 demo_get_vo_windows(void)
{
	mt_s32 ret;
	mt_s32 retval = MT_SUCCESS;
	MT_UNF_WINDOW_ATTR_S pWinAttr;

	ret = MT_UNF_VO_GetWindowAttr(g_vo_window, &pWinAttr);
	if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_VO_GetWindowAttr failed.\n");
		retval = MT_FAILURE;
    }

	printf("input info:\n");
	printf("    x: %d, y: %d, w: %d, h: %d\n", pWinAttr.stInputRect.s32X, pWinAttr.stInputRect.s32Y, pWinAttr.stInputRect.s32Width, pWinAttr.stInputRect.s32Height);
	printf("output info:\n");
	printf("    x: %d, y: %d, w: %d, h: %d\n", pWinAttr.stOutputRect.s32X, pWinAttr.stOutputRect.s32Y, pWinAttr.stOutputRect.s32Width, pWinAttr.stOutputRect.s32Height);

	return MT_SUCCESS;
}


mt_s32 demo_reg_adec_libs(void)
{
    mt_s32 ret = MT_SUCCESS;

#if 1
	ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
#else
    ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYTRUEHD.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSHD.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSM6.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
    ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");
#endif
    if (ret != MT_SUCCESS)
	{
		printf("\n\n!!! some audio codec NOT found. you may NOT able to decode some audio type.\n\n");
    }

    return MT_SUCCESS;
}


mt_s32 demo_sound_init(mt_handle *p_track)
{
	mt_s32 ret;
	mt_u32 retval = MT_SUCCESS;	
	MT_UNF_SND_ATTR_S stAttr;
	MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;

	ret = MT_UNF_SND_Init();
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_SND_Init fail\n");
		retval = MT_FAILURE;
    }

	memset(&stAttr, 0, sizeof(MT_UNF_SND_ATTR_S));
    ret = MT_UNF_SND_GetDefaultOpenAttr(MT_UNF_SND_0, &stAttr);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_SND_GetDefaultOpenAttr fail\n");
		retval = MT_FAILURE;
    }

    ret = MT_UNF_SND_Open(MT_UNF_SND_0, &stAttr);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_SND_Open fail\n");
		retval = MT_FAILURE;
    }

    memset(&stTrackAttr, 0, sizeof(stTrackAttr));
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (ret != MT_SUCCESS)
	{
		printf("Create Track Fail");
		retval = MT_FAILURE;
    }

	ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, p_track);
    if (ret != MT_SUCCESS) {
		printf("Create Track Fail");
		retval = MT_FAILURE;
    }

	return ret;
}

mt_s32 demo_vo_init(mt_handle *phWin)
{
	mt_s32 ret;
	mt_u32 retval = MT_SUCCESS;	
	MT_UNF_WINDOW_ATTR_S WinAttr;

	ret = MT_UNF_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    if (ret != MT_SUCCESS)
	{
		printf("MT_Av_VO_Init fail %d\n", ret);
		retval = MT_FAILURE;
    }

    memset(&WinAttr, 0, sizeof(MT_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = MT_UNF_DISPLAY1;

    ret = MT_UNF_VO_CreateWindow(&WinAttr, phWin);
    if (ret != MT_SUCCESS)
	{
		printf("call MT_UNF_VO_CreateWindow failed.\n");
		return ret;
    }
	
    return MT_SUCCESS;
}

mt_s32 demo_avplay_init(mt_handle *p_Avplay)
{
	mt_s32 ret;
	mt_u32 retval = MT_SUCCESS;	
	MT_UNF_AVPLAY_ATTR_S AvplayAttr;
	MT_UNF_AVPLAY_OPEN_OPT_S stMaxCapbility;

	ret = MT_UNF_AVPLAY_Init();
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_Init fail\n");
		retval = MT_FAILURE;
	}

	ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_GetDefaultConfig fail\n");
		retval = MT_FAILURE;
	}

#ifdef CONFIG_MT_DDR_SIZE_128
	AvplayAttr.stStreamAttr.u32VidBufSize = 0x400000;
#else
	AvplayAttr.stStreamAttr.u32VidBufSize = 0x800000;
#endif
	printf("MT_Av_AVPLAY_Create,%0x \n", AvplayAttr.stStreamAttr.u32VidBufSize);

	ret = MT_UNF_AVPLAY_Create(&AvplayAttr, p_Avplay);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_GetDefaultConfig fail\n");
		retval = MT_FAILURE;
	}

	ret = MT_UNF_AVPLAY_ChnOpen(*p_Avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &stMaxCapbility);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_ChnOpen fail\n");
		retval = MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(*p_Avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_ChnOpen fail\n");
		retval = MT_FAILURE;
    }

	return ret;
}

mt_s32 demo_av_play(void)
{
	mt_u32 ret;
	mt_u32 retval = MT_SUCCESS;	
	MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync;
	MT_UNF_SYNC_ATTR_S SyncAttr;
	MT_UNF_VCODEC_ATTR_S VdecAttr;
	MT_UNF_ACODEC_ATTR_S AdecAttr;
	mt_handle hAvplay = g_av_play;
	prog_info_t prog = {0x12c, MT_UNF_VCODEC_TYPE_MPEG2, 0x18f, HA_AUDIO_ID_MP3, 0x12c};	//videp mpeg 2, audio mpeg 1

	DmxAvsync.VdecType = prog.v_type;
	DmxAvsync.AdecType = prog.a_type;
	DmxAvsync.AvsyncFlage = 1; // 1--insert pts 0--do not insert pts
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_SetAttr fail\n");
		retval = MT_FAILURE;
    }
	//set pcr pid
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &prog.pcr_pid);
	if (MT_SUCCESS != ret)
	{
	    printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
	    retval = MT_FAILURE;
	}
	//process video
	ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (ret != MT_SUCCESS)
		retval = MT_FAILURE;

    VdecAttr.enType = prog.v_type;
    VdecAttr.enUnBlank=MT_UNF_VCODEC_UNBLANK_STABLE;
    VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
    VdecAttr.u32ErrCover = 100;
    VdecAttr.s32CtrlOptions = 0;
    VdecAttr.u32Priority = 3;

	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
	if (ret != MT_SUCCESS)
	{
	    printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
	    retval = MT_FAILURE;
	}
	
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &prog.v_pid);
	if (ret != MT_SUCCESS)
	{
	    printf("call HIADP_AVPlay_SetVdecAttr failed.\n");
	    retval = MT_FAILURE;
	}
	//set av sync mode
	ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
	if (ret != MT_SUCCESS)
	{
	    printf("call HIADP_AVPlay_SetVdecAttr failed.\n");
	    retval = MT_FAILURE;
	}
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (ret != MT_SUCCESS)
	{
	    printf("call HIADP_AVPlay_SetVdecAttr failed.\n");
	    retval = MT_FAILURE;
	}
	//process audio 
	ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
	if (ret != MT_SUCCESS)
	{
	    printf("MT_UNF_AVPLAY_SetAttr failed\n");
	    retval = MT_FAILURE;
	}
	AdecAttr.enType = prog.a_type;
	HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
//	HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
	if (ret != MT_SUCCESS)
	{
	    printf("MT_UNF_AVPLAY_SetAttr failed\n");
	    retval = MT_FAILURE;
	}
	
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &prog.a_pid);
	if (ret != MT_SUCCESS)
	{
	    printf("MT_UNF_AVPLAY_SetAttr failed\n");
	    retval = MT_FAILURE;
	}
	//start play av
	ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_Start failed.\n");
		retval = MT_FAILURE;
    }

	return retval;
}

mt_s32 demo_playback_init(mt_u8 port)
{
	mt_s32 ret;
	mt_s32 retval = MT_SUCCESS;
	MT_UNF_SYNC_ATTR_S SyncAttr;
	mt_rect_s rect = {490*1920/1280, 200*1080/720, 630*1920/1280, 420*1080/720};

	ret = MT_UNF_DMX_Init();
	ret = MT_UNF_DMX_AttachTSPort(0, port);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_DMX_AttachTSPort failed! return 0x%x.\n", ret);
		return ret;
	}

	ret = demo_sound_init(&g_sound_track);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_DMX_AttachTSPort failed! return 0x%x.\n", ret);
		return ret;
	}

	ret = demo_vo_init(&g_vo_window);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_DMX_AttachTSPort failed! return 0x%x.\n", ret);
		return ret;
	}
	
	(void)demo_reg_adec_libs();
	ret = demo_avplay_init(&g_av_play);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_DMX_AttachTSPort failed! return 0x%x.\n", ret);
		return ret;
	}
	
	ret = MT_UNF_VO_AttachWindow(g_vo_window, g_av_play);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_VO_AttachWindow fail\n");
		retval = MT_FAILURE;
    }
	
    ret = MT_UNF_VO_SetWindowEnable(g_vo_window, MT_TRUE);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_VO_SetWindowEnable fail\n");
		retval = MT_FAILURE;
    }

	demo_set_vo_window(&rect);
	
    ret = MT_UNF_SND_Attach(g_sound_track, g_av_play);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_SND_Attach fail\n");
		retval = MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_GetAttr(g_av_play, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
	if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_GetAttr fail\n");
		retval = MT_FAILURE;
    }
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    ret = MT_UNF_AVPLAY_SetAttr(g_av_play, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (ret != MT_SUCCESS)
	{
		printf("MT_UNF_AVPLAY_SetAttr fail\n");
		retval = MT_FAILURE;
    }
	
	return retval;
}

mt_s32 demo_playback_destory(void)
{
    mt_s32 ret = 0;
    mt_s32 retval = MT_SUCCESS;
    MT_UNF_AVPLAY_STOP_OPT_S stop;

	//stop avplay
    stop.u32TimeoutMs = 0;
	stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    ret = MT_UNF_AVPLAY_Stop(g_av_play, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stop);
    //MT_USLEEP(50*1000);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_AVPLAY_Stop failed.\n");
		retval = MT_FAILURE;
    }
    ret = MT_UNF_SND_Detach(g_sound_track, g_av_play);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_SND_Detach failed.\n");
		retval = MT_FAILURE;
    }

    ret = MT_UNF_SND_DestroyTrack(g_sound_track);
    if (ret != MT_SUCCESS) {
		printf(" MT_UNF_SND_Detach failed.\n");
		retval = MT_FAILURE;
    }

    //win detach
    ret = MT_UNF_VO_SetWindowEnable(g_vo_window, MT_FALSE);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_VO_SetWindowEnable failed.\n");
		retval = MT_FAILURE;
    }
    ret = MT_UNF_VO_DetachWindow(g_vo_window, g_av_play);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_VO_DetachWindow failed.\n");
		retval = MT_FAILURE;
    }
    ret = MT_UNF_AVPLAY_ChnClose(g_av_play, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_AVPLAY_ChnClose failed.\n");
		retval = MT_FAILURE;
    }
    //close wchannel
    ret = MT_UNF_AVPLAY_ChnClose(g_av_play, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_AVPLAY_ChnClose failed.\n");
		retval = MT_FAILURE;
    }
    //destroy avplay
    ret = MT_UNF_AVPLAY_Destroy(g_av_play);
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    ret = MT_UNF_AVPLAY_DeInit();
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    //close demux
    ret = MT_UNF_DMX_DeInit();
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    //video out
    ret = MT_UNF_VO_DestroyWindow(g_vo_window);
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    ret = MT_UNF_VO_DeInit();
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    //sound deinit
    ret = MT_UNF_SND_Close(MT_UNF_SND_0);
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    ret = MT_UNF_SND_DeInit();
    if (ret != MT_SUCCESS)
	{
		printf(" ==>%s ==%d\n", __FUNCTION__, __LINE__);
		retval = MT_FAILURE;
    }
    
    g_sound_track = 0;
	g_vo_window = 0;
	g_av_play = 0;
	
    return retval;
}

mt_s32 demo_set_vo_window(void *p_rect)
{
	mt_s32 ret;
	mt_s32 retval = MT_SUCCESS;
	MT_UNF_WINDOW_ATTR_S pWinAttr;
	mt_rect_s *vo_rect = (mt_rect_s *)p_rect;

	printf("%s(line: %d), entry\n", __FUNCTION__, __LINE__);
	ret = MT_UNF_VO_GetWindowAttr(g_vo_window, &pWinAttr);
	if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_VO_GetWindowAttr failed.\n");
		return MT_FAILURE;
    }
	pWinAttr.stOutputRect.s32X = vo_rect->s32X;
	pWinAttr.stOutputRect.s32Y = vo_rect->s32Y;
	pWinAttr.stOutputRect.s32Width = vo_rect->s32Width;
	pWinAttr.stOutputRect.s32Height = vo_rect->s32Height;
	ret = MT_UNF_VO_SetWindowAttr(g_vo_window, &pWinAttr);
	if (ret != MT_SUCCESS)
	{
		printf(" MT_UNF_VO_SetWindowAttr failed.\n");
		retval = MT_FAILURE;
    }
	printf("%s(line: %d), set window x=%d, y=%d, w=%d, h=%d, ret=%d\n", __FUNCTION__, __LINE__, pWinAttr.stOutputRect.s32X, pWinAttr.stOutputRect.s32Y, pWinAttr.stOutputRect.s32Width, pWinAttr.stOutputRect.s32Height, ret);

	return MT_SUCCESS;
}

mt_s32 demo_av_set_mute(MT_BOOL mute)
{
	mt_s32 ret;
	
	ret = MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, mute);
    ret |= MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, mute);
    ret |= MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, mute);

	return ret;
}

mt_s32 demo_rebuild_playback(mt_u8 port)
{
	demo_playback_destory();
	demo_playback_init(port);
	demo_av_play();
}


