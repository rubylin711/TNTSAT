/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_frontend.h"

//#include "mt_unf_advca.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#if 0
#include "mt_adp.h"
#include "mt_adp_audio.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_tuner.h"
#include "sample_ca_common.h"
#else
#include "../common/mt_adp.h"
#include "../common/mt_adp_audio.h"
#include "../common/mt_adp_hdmi.h"
#include "../common/mt_adp_boardcfg.h"
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_frontend.h"
#endif

#ifndef DEFAULT_DVB_PORT
#define DEFAULT_DVB_PORT 1
#endif

#define INVALID_HANDLE 0xffffffff

PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;
static mt_handle g_hAvplay;
static mt_u32 g_avsync_mode = 1;
static mt_s32 g_tuner_id = 0;
mt_handle hWin;
mt_handle hTrack;

static mt_s32 set_avsync_mode(void)
{
    mt_s32 Ret = MT_SUCCESS;
    MT_UNF_SYNC_ATTR_S SyncAttr;

    Ret = MT_UNF_AVPLAY_GetAttr(g_hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    switch (g_avsync_mode) {
    case 0:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
	break;
    case 1:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
	break;
    case 2:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_VIDEO;
	break;
    case 3:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_PCR;
	break;
    case 4:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_SCR;
	break;
    default:
	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
	break;
    }

    Ret = MT_UNF_AVPLAY_SetAttr(g_hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    return Ret;
}

mt_s32 play_bypid(mt_handle hAvplay, MT_UNF_VCODEC_TYPE_E VdecType, mt_u32 VidPid, mt_u32 AdecType, mt_u32 AudPid, mt_u32 PcrPid, mt_u32 avsync_flag)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STOP_OPT_S StopOpt;
    StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    StopOpt.u32TimeoutMs = 0;

    if ((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID)) {

	printf("avsync_flagg=================  avsync_flag = %d \n", avsync_flag);
	MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync;
	DmxAvsync.VdecType = VdecType;
	DmxAvsync.AdecType = AdecType;
	DmxAvsync.AvsyncFlage = avsync_flag;
	MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
    }
    if (VidPid != INVALID_TSPID) {
	Ret = MTADP_AVPlay_SetVdecAttr(hAvplay, VdecType, MT_UNF_VCODEC_MODE_NORMAL);
	Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &VidPid);
	if (Ret != MT_SUCCESS) {
	    printf("call MTADP_AVPlay_SetVdecAttr failed.\n");
	    return Ret;
	}

	Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
	if (Ret != MT_SUCCESS) {
	    printf("call MTADP_AVPlay_SetVdecAttr PcrPid failed.\n");
	    return Ret;
	}

	Ret = set_avsync_mode();

	Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (Ret != MT_SUCCESS) {
	    printf("call MT_UNF_AVPLAY_Start failed.\n");
	    return Ret;
	}
    }
    if (AudPid != INVALID_TSPID) {
	Ret = MTADP_AVPlay_SetAdecAttr(hAvplay, AdecType, HD_DEC_MODE_RAWPCM, 1);
	Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
	if (MT_SUCCESS != Ret) {
	    printf("MTADP_AVPlay_SetAdecAttr failed:%#x\n", Ret);
	    return Ret;
	}

	Ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	if (Ret != MT_SUCCESS) {
	    printf("call MT_UNF_AVPLAY_Start failed.\n");
	    return Ret;
	}
    }
    return MT_SUCCESS;
}
mt_s32 fe_init()
{
    mt_s32 ret = 0;
    mt_unf_fe_attr_t fe_attr;
    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret) {
	return ret;
    }

    /* open Tuner*/
    ret = mt_unf_fe_open(g_tuner_id);
    if (MT_SUCCESS != ret) {
	mt_unf_fe_deinit();
	return ret;
    }

    /* get default attribute */
    ret = mt_unf_fe_get_default_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
	mt_unf_fe_close(g_tuner_id);
	mt_unf_fe_deinit();
	return ret;
    }

    MTADP_FE_GET_CONFIG(g_tuner_id, fe_attr);
    ret = mt_unf_fe_set_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
	return ret;
    }
    return ret;
}
mt_s32 fe_deinit()
{
    mt_s32 ret = 0;
    ret = mt_unf_fe_close(g_tuner_id);
    if (MT_SUCCESS != ret) {
	return ret;
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret) {
	return ret;
    }
    return ret;
}
mt_s32 mt_fe_connect(mt_u32 freq, mt_u32 sym_rate, mt_u32 qam_type, mt_u32 band_width)
{
    mt_s32 ret = MT_FAILURE;
    mt_unf_fe_status_t stTunerStatus;
    mt_u32 u32Loop = 0;
    mt_u32 u32LoopTimes = 100;
    mt_u32 u32Freq = 0;
    mt_u32 u32SymbolRate = 0;
    mt_unf_fe_connect_para_t s_stConnectPara;

    s_stConnectPara.connect_param.cab.freq = freq * 1000;
    s_stConnectPara.connect_param.cab.sym_rate = sym_rate * 1000;
    s_stConnectPara.connect_param.cab.mod_type = qam_type;
    s_stConnectPara.connect_param.cab.band_width = band_width;

    ret = mt_unf_fe_connect(g_tuner_id, &s_stConnectPara, 0);
    u32Freq = s_stConnectPara.connect_param.cab.freq;
    u32SymbolRate = s_stConnectPara.connect_param.cab.sym_rate;

    if (MT_SUCCESS == ret) {
	for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++) {
	    ret = mt_unf_fe_get_status(g_tuner_id, &stTunerStatus);
	    if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status) {
		printf("Tuner Lock freq %d symb %d qam%d Success!\n", freq, sym_rate, qam_type);
		/*automatically play the first program after locked successfully*/
		printf("SUCCESS end\n");
		return MT_SUCCESS;
	    } else {
		MT_USLEEP(10000);
	    }
	}
    } else {
	printf("Tuner Lock freq %d symb %d qam%d Fail!, ret = 0x%x\n", freq, sym_rate, qam_type, ret);
    }

    if (u32Loop == u32LoopTimes) {
	printf("Tuner Lock freq %d symb %d  qam%d Fail!\n", freq, sym_rate, qam_type);
    }

    printf("FAIL end\n");
    return MT_FAILURE;
}

mt_s32 avplay_open()
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;

    Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != Ret) {
	goto ERROR_RETURN;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL, &hWin);
    if (MT_SUCCESS != Ret) {
	printf("call MTADP_VO_Init failed.\n");
	(mt_void) MTADP_VO_DeInit();
	goto SND_DEINIT;
    }

    Ret = MTADP_AVPlay_RegADecLib();
    Ret |= MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_AVPLAY_Init failed.\n");
	goto VO_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_hAvplay);
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_AVPLAY_Create failed.\n");
	goto AVPLAY_DEINIT;
    }

    printf("=========================%s LINE = %d\n", __func__, __LINE__);
    Ret = MT_UNF_AVPLAY_ChnOpen(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_AVPLAY_ChnOpen MT_UNF_AVPLAY_MEDIA_CHAN_VID failed.\n");
	goto CHN_CLOSE;
    }

    Ret |= MT_UNF_AVPLAY_ChnOpen(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_AVPLAY_ChnOpen MT_UNF_AVPLAY_MEDIA_CHAN_AUD failed. Ret = 0x%x\n", Ret);
	goto CHN_CLOSE;
    }
    Ret = MT_UNF_VO_AttachWindow(hWin, g_hAvplay);
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_VO_AttachWindow failed.\n");
	goto CHN_CLOSE;
    }
    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_VO_SetWindowEnable failed.\n");
	goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
	goto WIN_DETACH;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_SND_CreateTrack failed.\n");
	goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_Attach(hTrack, g_hAvplay);
    if (Ret != MT_SUCCESS) {
	printf("call MT_SND_Attach failed.\n");
	goto TRACK_DESTROY;
    }

    return Ret;
SND_DETACH:
    (mt_void) MT_UNF_SND_Detach(MT_UNF_SND_0, g_hAvplay);

TRACK_DESTROY:
    MT_UNF_SND_DestroyTrack(hTrack);

WIN_DETACH:
    (mt_void) MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    (mt_void) MT_UNF_VO_DetachWindow(hWin, g_hAvplay);
CHN_CLOSE:
    (mt_void) MT_UNF_AVPLAY_ChnClose(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    (mt_void) MT_UNF_AVPLAY_Destroy(g_hAvplay);

AVPLAY_DEINIT:
    (mt_void) MT_UNF_AVPLAY_DeInit();

VO_DEINIT:
    (mt_void) MT_UNF_VO_DestroyWindow(hWin);
    (mt_void) MTADP_VO_DeInit();

SND_DEINIT:
    (mt_void) MTADP_Snd_DeInit();
ERROR_RETURN:
    return Ret;
}
#define CHECK_RESULT(Ret, INFO) \
    if (Ret != MT_SUCCESS)      \
    printf("===== %s ====== 0x%x\n", INFO, Ret)
mt_s32 avplay_close()
{
    mt_s32 Ret;
    Ret = MT_UNF_SND_Detach(hTrack, g_hAvplay);
    CHECK_RESULT(Ret, "MT_UNF_SND_Detach");
    Ret = MT_UNF_SND_DestroyTrack(hTrack);
    CHECK_RESULT(Ret, "MT_UNF_SND_DestroyTrack");
    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
    CHECK_RESULT(Ret, "MT_UNF_VO_SetWindowEnable");
    Ret = MT_UNF_VO_DetachWindow(hWin, g_hAvplay);
    CHECK_RESULT(Ret, "MT_UNF_VO_DetachWindow");

    Ret = MT_UNF_AVPLAY_ChnClose(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    CHECK_RESULT(Ret, "MT_UNF_AVPLAY_ChnClose");
    Ret = MT_UNF_AVPLAY_Destroy(g_hAvplay);
    CHECK_RESULT(Ret, "MT_UNF_AVPLAY_Destroy");
    Ret = MT_UNF_AVPLAY_DeInit();
    CHECK_RESULT(Ret, "MT_UNF_AVPLAY_DeInit");

    Ret = MT_UNF_VO_DestroyWindow(hWin);
    CHECK_RESULT(Ret, "MT_UNF_VO_DestroyWindow");
    Ret = MTADP_VO_DeInit();
    CHECK_RESULT(Ret, "MTADP_VO_DeInit");
    Ret = MTADP_Snd_DeInit();
    CHECK_RESULT(Ret, "MTADP_Snd_DeInit");
    return Ret;
}
mt_s32 main(mt_s32 argc, mt_char *argv[])
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;
    mt_u32 freq = 770;
    mt_u32 symbol_rate = 6875;
    mt_u32 qam_type = 0;
    mt_u32 qam_mode = 64;
    mt_u32 band_width = 8;
    int i, j;
    j = 2000;
    if (argc > 1)
	freq = strtol(argv[1], NULL, 0);
    if (argc > 2)
	symbol_rate = strtol(argv[2], NULL, 0);
    if (argc > 3)
	qam_mode = strtol(argv[3], NULL, 0);
    if (argc > 4)
	band_width = strtol(argv[4], NULL, 0);
    if (argc > 5)
	j = strtol(argv[5], NULL, 0);

    if (qam_mode == 16)
	qam_type = MT_UNF_MOD_TYPE_QAM_16;
    else if (qam_mode == 32)
	qam_type = MT_UNF_MOD_TYPE_QAM_32;
    else if (qam_mode == 64)
	qam_type = MT_UNF_MOD_TYPE_QAM_64;
    else if (qam_mode == 128)
	qam_type = MT_UNF_MOD_TYPE_QAM_128;
    else if (qam_mode == 256)
	qam_type = MT_UNF_MOD_TYPE_QAM_256;
    else if (qam_mode == 512)
	qam_type = MT_UNF_MOD_TYPE_QAM_512;

    mt_sys_init();
    Ret = fe_init();
    if (MT_SUCCESS != Ret) {
	printf("mt_fe_connect %d MHZ Fail\n", freq);
	return 0;
    }

    Ret = mt_fe_connect(freq, symbol_rate, qam_type, band_width);
    if (MT_SUCCESS != Ret) {
	printf("mt_fe_connect %d MHZ Fail\n", freq);
	return 0;
    }

    MTADP_MCE_Exit();
    MTADP_MCE_Exit();
    // Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_50);
    Ret = MT_UNF_DISP_Init();
    if (MT_SUCCESS != Ret) {
	printf("call MTADP_Disp_DeInit failed.\n");
    }

    Ret = MT_UNF_DMX_Init();
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_0);
    if (MT_SUCCESS != Ret) {
	return 0;
    }

    MTADP_Search_Init();
    Ret = MTADP_Search_GetAllPmt(0, &g_pProgTbl);
    if (g_pProgTbl->prog_num <= 0) {
	printf("Can not Search Program\n");
	return 0;
    }
    while (j-- >= 0) {
	i = j % g_pProgTbl->prog_num;
	Ret = avplay_open();
	if (Ret != MT_SUCCESS) {
	    break;
	}
	play_bypid(g_hAvplay,
	           g_pProgTbl->proginfo[i].VideoType,
	           g_pProgTbl->proginfo[i].VElementPid,
	           g_pProgTbl->proginfo[i].AudioType,
	           g_pProgTbl->proginfo[i].AElementPid,
	           g_pProgTbl->proginfo[i].PcrPid,
	           1);
	sleep(5);
	printf("=========================%s LINE = %d j ==== %d\n", __func__, __LINE__, j);

	Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
	Stop.u32TimeoutMs = 0;
	Ret = MT_UNF_AVPLAY_Stop(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);
	printf("MT_UNF_AVPLAY_Stop Ret = 0x%x\n", Ret);
	avplay_close();
    }
    (mt_void) MTADP_Search_FreeAllPmt(g_pProgTbl);
    MTADP_Search_DeInit();
    fe_deinit();
    //MTADP_Disp_DeInit();
    return Ret;
}
