/******************************************************************************

  Copyright (C), 2001-2011, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mplayer.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2010/01/26
  Description   :
  History       :
  1.Date        : 2010/01/26
    Author      : w58735
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_ecs.h"

#include "../common/mt_adp.h"
#include "../common/mt_adp_audio.h"
#include "../common/mt_adp_hdmi.h"
#include "../common/mt_adp_boardcfg.h"
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_frontend.h"

#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#if defined(DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.DTSHD.decode.h"

#ifdef CONFIG_SUPPORT_CA_RELEASE
#define sample_printf
#else
#define sample_printf printf
#endif

PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;

#ifndef DEFAULT_DVB_PORT
#define DEFAULT_DVB_PORT MT_UNF_DMX_PORT_TSI_2
#endif

static mt_handle hAvplay;
static MT_BOOL saveEsDatatoFile = MT_FALSE;
static mt_u8 fileName[64];
static MT_BOOL saveFile = MT_FALSE;

mt_void *saveEsDatathread(mt_void *args)
{
    MT_UNF_ES_BUF_S pEsBuf;
    mt_handle hDmxVidChn, hDmxAudChn;
    FILE *esFile = NULL;
    mt_s32 ret = 0;

    esFile = fopen(fileName, "wb");
    if (!esFile) 
    {
    	perror("fopen error");
    	saveEsDatatoFile = MT_FALSE;
    	return MT_NULL;
    }

    printf("[%s] open file %s\n", __FUNCTION__, fileName);

    MT_UNF_AVPLAY_GetDmxVidChnHandle(hAvplay, &hDmxVidChn);
    while (saveEsDatatoFile) 
    {
    	if (MT_SUCCESS != MT_UNF_DMX_AcquireEs(hDmxVidChn, &pEsBuf)) {
    	    printf("call MT_UNF_DMX_AcquireEs failed!\n");
    	    MT_USLEEP(10 * 1000);
    	    continue;
    	}
    	ret = fwrite(pEsBuf.pu8Buf, 1, pEsBuf.u32BufLen, esFile);
    	printf("ret=0x%x, pEsBuf.pu8Buf = 0x%x, pEsBuf.u32BufLen = 0x%x\n", ret, pEsBuf.pu8Buf, pEsBuf.u32BufLen);
    	if (ret != pEsBuf.u32BufLen) {
    	    printf("ret=%x\n", ret);
    	    perror("[SaveRecDataThread] fwrite error");
    	    break;
    	}
    }
    fclose(esFile);
    printf("[%s] close file %s\n", __FUNCTION__, fileName);
    saveFile = MT_TRUE;
    return MT_NULL;
}

mt_s32 main(mt_s32 argc, mt_char *argv[])
{
    mt_s32 Ret;
    mt_handle hWin;

    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S SyncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;
    mt_char InputCmd[32];
    MT_UNF_ENC_FMT_E enFormat = MT_UNF_ENC_FMT_1080i_50;
    MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_BUTT;
    mt_u32 AdecType = 0;
    mt_u32 VPid = 0, APid = 0;
    MT_BOOL bAudPlay = MT_FALSE;
    MT_BOOL bVidPlay = MT_FALSE;
    mt_u32 ProgNum;
    mt_u32 u32TunerFreq;
    mt_u32 u32TunerSrate;
    mt_u32 u32ThirdParam;
    PMT_COMPACT_TBL pProgTbl;
    PMT_COMPACT_PROG proginfo;

    mt_handle hTrack;
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;
    pthread_t g_TsThd;

    if (argc < 6 || argc > 10) {
	printf("Usage: %s vpid vtype apid atype freq [srate] [qamtype or polarization] [vo_format]\n"
	       "       qamtype or polarization: \n"
	       "           For cable, used as qamtype, value can be 16|32|[64]|128|256|512 defaut[64] \n"
	       "           For satellite, used as polarization, value can be [0] horizontal | 1 vertical defaut[0] \n"
	       "       vo_format:2160P_30|2160P_24|1080P_60|1080P_50|1080i_60|[1080i_50]|720P_60|720P_50  default MT_UNF_ENC_FMT_1080i_50\n"
	       "       vtype: mpeg2/mpeg4/h263/sor/vp6/vp6f/vp6a/h264/avs/real8/real9/vc1\n"
	       "       atype: aac/mp3/dts/dra/mlp/pcm/ddp(ac3/eac3)\n",
	       argv[0]);
	printf(" examples: \n");
	printf("   %s vpid h264 \"null\" \"null\" 610 6875 64 1080i_50\n", argv[0]);
	printf("   %s \"null\" \"null\" apid mp3 610 6875 64 1080i_50\n", argv[0]);
	printf("   %s vpid h264 apid mp3 610 6875 64 1080i_50\n", argv[0]);
	return MT_FAILURE;
    }

    if (10 == argc) {
	u32TunerFreq = strtol(argv[5], NULL, 0);
	u32TunerSrate = strtol(argv[6], NULL, 0);
	u32ThirdParam = strtol(argv[7], NULL, 0);
	enFormat = MTADP_Disp_StrToFmt(argv[8]);
	saveEsDatatoFile = MT_TRUE;
	memset(fileName, 0, 64);
	if (strcasecmp("null", argv[9])) {
	    strcpy(fileName, argv[9]);
	} else {
	    strcpy(fileName, "/mnt/videoEsData.es");
	}
	printf("fileName = %s\n", fileName);
    } else if (9 == argc) {
	u32TunerFreq = strtol(argv[5], NULL, 0);
	u32TunerSrate = strtol(argv[6], NULL, 0);
	u32ThirdParam = strtol(argv[7], NULL, 0);
	enFormat = MTADP_Disp_StrToFmt(argv[8]);
    } else if (8 == argc) {
	u32TunerFreq = strtol(argv[5], NULL, 0);
	u32TunerSrate = strtol(argv[6], NULL, 0);
	u32ThirdParam = strtol(argv[7], NULL, 0);
	enFormat = MT_UNF_ENC_FMT_1080i_50;
    } else if (7 == argc) {
	u32TunerFreq = strtol(argv[5], NULL, 0);
	u32TunerSrate = strtol(argv[6], NULL, 0);
	u32ThirdParam = (u32TunerFreq > 1000) ? 0 : 64;
	enFormat = MT_UNF_ENC_FMT_1080i_50;
    } else if (6 == argc) {
	u32TunerFreq = strtol(argv[5], NULL, 0);
	u32TunerSrate = (u32TunerFreq > 1000) ? 27500 : 6875;
	u32ThirdParam = (u32TunerFreq > 1000) ? 0 : 64;
	enFormat = MT_UNF_ENC_FMT_1080i_50;
    }

    if (strcasecmp("null", argv[1])) {
	bVidPlay = MT_TRUE;
	VPid = strtol(argv[1], NULL, 0);
	if (!strcasecmp("mpeg2", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
	} else if (!strcasecmp("mpeg4", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_MPEG4;
	} else if (!strcasecmp("h263", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_H263;
	} else if (!strcasecmp("sor", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_SORENSON;
	} else if (!strcasecmp("vp6", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_VP6;
	} else if (!strcasecmp("vp6f", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_VP6F;
	} else if (!strcasecmp("vp6a", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_VP6A;
	} else if (!strcasecmp("h264", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_H264;
	} else if (!strcasecmp("avs", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_AVS;
	} else if (!strcasecmp("real8", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_REAL8;
	} else if (!strcasecmp("real9", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_REAL9;
	} else if (!strcasecmp("vc1", argv[2])) {
	    VdecType = MT_UNF_VCODEC_TYPE_VC1;
	} else {
	    sample_printf("unsupport vid codec type!\n");
	    return -1;
	}
    }

    if (strcasecmp("null", argv[3])) {
	bAudPlay = MT_TRUE;
	APid = strtol(argv[3], NULL, 0);
	if (!strcasecmp("aac", argv[4])) {
	    AdecType = HA_AUDIO_ID_AAC;
	} else if (!strcasecmp("mp3", argv[4])) {
	    AdecType = HA_AUDIO_ID_MP3;
	}
#if defined(DOLBYPLUS_HACODEC_SUPPORT)
	else if (!strcasecmp("ddp", argv[4])) {
	    AdecType = HA_AUDIO_ID_DOLBY_PLUS;
	}
#endif
	else if (!strcasecmp("ac3", argv[4])) {
	    AdecType = HA_AUDIO_ID_AC3PASSTHROUGH;
	} else if (!strcasecmp("dts", argv[4])) {
	    AdecType = HA_AUDIO_ID_DTSHD;
	} else if (!strcasecmp("dra", argv[4])) {
	    AdecType = HA_AUDIO_ID_DRA;
	} else if (!strcasecmp("pcm", argv[4])) {
	    AdecType = HA_AUDIO_ID_PCM;
	} else if (!strcasecmp("mlp", argv[4])) {
	    AdecType = HA_AUDIO_ID_TRUEHD;
	} else if (!strcasecmp("amr", argv[4])) {
	    AdecType = HA_AUDIO_ID_AMRNB;
	} else {
	    sample_printf("unsupport aud codec type!\n");
	    return -1;
	}
    }
    if (!(bVidPlay || bAudPlay)) {
	return 0;
    }

    mt_sys_init();

    MTADP_MCE_Exit();
    Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, enFormat);

#if 1
    Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != Ret) {
	sample_printf("call MTADP_Snd_Init failed.\n");
	goto SYS_DEINIT;
    }
    /*MT_SYS_GetPlayTime(& u32Playtime);
    sample_printf("u32Playtime = %d\n",u32Playtime);*/
    Ret = MTADP_Disp_Init(enFormat);
    if (MT_SUCCESS != Ret) {
	sample_printf("call MTADP_Disp_DeInit failed.\n");
	goto SND_DEINIT;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL, &hWin);
    if (MT_SUCCESS != Ret) {
	sample_printf("call MTADP_VO_Init failed.\n");
	MTADP_VO_DeInit();
	goto DISP_DEINIT;
    }
#endif
    Ret = MT_UNF_DMX_Init();
#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_0);
#endif
    if (MT_SUCCESS != Ret) {
	sample_printf("call MTADP_Demux_Init failed.\n");
	//goto VO_DEINIT;
    }
    /*
    Ret = MTADP_Tuner_Init();
    if (MT_SUCCESS != Ret)
    {
        sample_printf("call MTADP_Demux_Init failed.\n");
        goto DMX_DEINIT;
    }
     
    Ret = MTADP_Tuner_Connect(TUNER_USE, u32TunerFreq, u32TunerSrate, u32ThirdParam);
    if (MT_SUCCESS != Ret)
    {
        sample_printf("call MTADP_Tuner_Connect failed.\n");
        goto TUNER_DEINIT;
    }
    */
    MTADP_Search_Init();

    //Ret = MTADP_AVPlay_RegADecLib();
    Ret |= MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_AVPLAY_Init failed.\n");
	goto PSISI_FREE;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    AvplayAttr.stStreamAttr.u32VidBufSize = 0x1000000;
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_AVPLAY_Create failed.\n");
	goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    Ret |= MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != Ret) {
	sample_printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
	goto CHN_CLOSE;
    }
#if 1
    Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if (MT_SUCCESS != Ret) {
	sample_printf("call MT_UNF_VO_AttachWindow failed.\n");
	goto CHN_CLOSE;
    }
    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if (MT_SUCCESS != Ret) {
	sample_printf("call MT_UNF_VO_SetWindowEnable failed.\n");
	goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
	goto WIN_DETACH;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_SND_CreateTrack failed.\n");
	goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_Attach(hTrack, hAvplay);
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_SND_Attach failed.\n");
	goto TRACK_DESTROY;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    SyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
    SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
    SyncAttr.bQuickOutput = MT_FALSE;
    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS) {
	sample_printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
	goto SND_DETACH;
    }
#endif
    memset(&proginfo, 0, sizeof(PMT_COMPACT_PROG));

    if (bVidPlay) {
	proginfo.VElementNum = 1;
	proginfo.VElementPid = VPid;
	proginfo.VideoType = VdecType;
    }
    if (bAudPlay) {
	proginfo.AElementNum = 1;
	proginfo.AElementPid = APid;
	proginfo.AudioType = AdecType;
    }

    pProgTbl.prog_num = 1;
    pProgTbl.proginfo = &proginfo;
    g_pProgTbl = &pProgTbl;

    ProgNum = 0;

    Ret = MTADP_AVPlay_PlayProg(hAvplay, g_pProgTbl, ProgNum, MT_TRUE);
    if (Ret != MT_SUCCESS) {
	sample_printf("call SwitchProg failed.\n");
	goto AVPLAY_STOP;
    }

    //MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_LPCM);
    if (saveEsDatatoFile) {
	    pthread_create(&g_TsThd, MT_NULL, (mt_void *)saveEsDatathread, MT_NULL);
    }

    while (1) 
    {
    	printf("please input the q to quit!, s to toggle spdif pass-through, h to toggle hdmi pass-through,v20(set volume 20),vxx(set volume xx)\n");
    	SAMPLE_GET_INPUTCMD(InputCmd);
    	if ('q' == InputCmd[0]) {
    	    sample_printf("prepare to quit!\n");
    	    saveEsDatatoFile = MT_FALSE;
    	    break;
    	}
    	if ('v' == InputCmd[0]) {
    	    MT_UNF_SND_GAIN_ATTR_S stGain;
    	    stGain.bLinearMode = MT_TRUE;

    	    stGain.s32Gain = atoi(InputCmd + 1);
    	    if (stGain.s32Gain > 100)
    		stGain.s32Gain = 100;

    	    printf("Volume=%d\n", stGain.s32Gain);
    	    MT_UNF_SND_SetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, &stGain);
    	}
    	if ('s' == InputCmd[0] || 'S' == InputCmd[0]) {
    	    static int spdif_toggle = 0;
    	    spdif_toggle++;
    	    if (spdif_toggle & 1) {
    		MT_UNF_SND_SetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, MT_UNF_SND_SPDIF_MODE_RAW);
    		printf("spdif pass-through on!\n");
    	    } else {
    		MT_UNF_SND_SetSpdifMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_SPDIF0, MT_UNF_SND_SPDIF_MODE_LPCM);
    		printf("spdif pass-through off!\n");
    	    }
    	    continue;
    	}
    	if ('h' == InputCmd[0] || 'H' == InputCmd[0]) {
    	    static int hdmi_toggle = 0;
    	    hdmi_toggle++;
    	    if (hdmi_toggle & 1) {
    		MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_RAW);
    		printf("hmdi pass-through on!\n");
    	    } else {
    		MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_LPCM);
    		printf("hmdi pass-through off!\n");
    	    }
    	    continue;
    	}
    }

    while (1) {
    	MT_USLEEP(10 * 1000);
    	if (saveFile == MT_TRUE)
	        break;
    }
AVPLAY_STOP:
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);

    pthread_join(g_TsThd, MT_NULL);

SND_DETACH:
    MT_UNF_SND_Detach(hTrack, hAvplay);

TRACK_DESTROY:
    MT_UNF_SND_DestroyTrack(hTrack);

WIN_DETACH:
    MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    MT_UNF_VO_DetachWindow(hWin, hAvplay);

CHN_CLOSE:
    MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    MT_UNF_AVPLAY_Destroy(hAvplay);

AVPLAY_DEINIT:
    MT_UNF_AVPLAY_DeInit();

PSISI_FREE:
    MTADP_Search_DeInit();
#if 0
TUNER_DEINIT:
    MTADP_Tuner_DeInit();
#endif
DMX_DEINIT:
    MT_UNF_DMX_DetachTSPort(0);
    MT_UNF_DMX_DeInit();

VO_DEINIT:
    MT_UNF_VO_DestroyWindow(hWin);
    MTADP_VO_DeInit();

DISP_DEINIT:
    MTADP_Disp_DeInit();

SND_DEINIT:
    MTADP_Snd_DeInit();

SYS_DEINIT:
    //MT_SYS_DeInit();
    mt_sys_deinit();
    return Ret;
}
