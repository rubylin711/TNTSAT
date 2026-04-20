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
#include "mt_unf_ecs.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "../common/mt_adp_audio.h"
#include "../common/mt_adp_hdmi.h"
#include "../common/mt_adp_mpi.h"

#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"

#ifndef PAT_TABLEID
#define PAT_TABLEID 0
#endif

#ifndef PAT_TSPID
#define PAT_TSPID   0
#endif

mt_u32 g_TunerFreq;
mt_u32 g_TunerSrate;
mt_u32 g_ThirdParam;
static mt_u8 g_match[DMX_FILTER_MAX_DEPTH];
static mt_u8 g_mask[DMX_FILTER_MAX_DEPTH];
static mt_u8 g_Negate[DMX_FILTER_MAX_DEPTH];

FILE               *g_pTsFile = MT_NULL;
static pthread_t   g_TsThd1;
static pthread_t   g_TsThd2;

static pthread_mutex_t g_TsMutex;
static MT_BOOL     g_bStopTsThread = MT_FALSE;
mt_handle          g_TsBuf;

static mt_handle               g_hWin;

#define  PLAY_DMX_ID  0

static mt_u32 m_Table[256] =
{
  0x00000000, 0x04C11DB7, 0x9823B6E, 0xD4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2,
  0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3,
  0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75,  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E,
  0x95609039,0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,  0xBE2B5B58, 0xBAEA46EF,
  0xB7A96036,0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,  0xD4326D90,
  0xD0F37027,0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028,0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A,
  0xEC7DD02D,0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C,
  0x2E003DC5,0x2AC12072,  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,
  0x54BF6A4,0x808D07D, 0xCC9CDCA,  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB,
  0x6F52C06C, 0x6211E6B5, 0x66D0FB02,  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,  0xACA5C697, 0xA864DB20, 0xA527FDF9,
  0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,  0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,  0xE0B41DE7,
  0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED,
  0xD8FBA05A,  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85,
  0x738AAD5C, 0x774BB0EB,  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53,  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,  0x315D626, 0x7D4CB91, 0xA97ED48,
  0xE56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,  0xF12F560E, 0xF5EE4BB9,
  0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,  0xD727BBB6,
  0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC,
  0xA379DD7B,  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD,
  0x81B02D74, 0x857130C3,  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645,
  0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,  0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x29F3D35, 0x65E2082, 0xB1D065B, 0xFDC1BEC,  0x3793A651, 0x3352BBE6,
  0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,  0xC5A92679,
  0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673,
  0xFDE69BC4,  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C,  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,0xBCB4666D,
  0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};

static mt_u32 g_vpid=0x1fff,g_apid=0x1fff;
static mt_u32 g_vtype=MT_UNF_VCODEC_TYPE_MPEG2,g_atype=HA_AUDIO_ID_MP3;

static mt_u32 CRC32(mt_u8 *buffer, mt_u32 size)
{
    mt_u32 Result = 0xFFFFFFFF;

    while (size--)
    {
        Result = (Result << 8) ^ m_Table[(Result >> 24) ^ *buffer ++];
    }

    return Result;
}

static MT_UNF_VCODEC_TYPE_E Get_atype(char *atype)
{
    HA_CODEC_ID_E AdecType;

    if (!strcasecmp("aac", atype)) {
        AdecType = HA_AUDIO_ID_AAC;
    } else if (!strcasecmp("mp3", atype)) {
        AdecType = HA_AUDIO_ID_MP3;
    } else if (!strcasecmp("truehd", atype)) {
        AdecType = HA_AUDIO_ID_DOLBY_TRUEHD;
    } else if (!strcasecmp("ac3raw", atype)) {
        AdecType = HA_AUDIO_ID_AC3PASSTHROUGH;
    } else if (!strcasecmp("dtsraw", atype)) {
        AdecType = HA_AUDIO_ID_DTSPASSTHROUGH;
    }
#if defined(DOLBYPLUS_HACODEC_SUPPORT)
    else if (!strcasecmp("ddp", atype)) {
        AdecType = HA_AUDIO_ID_DOLBY_PLUS;
    }
#endif
    else if (!strcasecmp("dts", atype)) {
        AdecType = HA_AUDIO_ID_DTSHD;
    } else if (!strcasecmp("dtsm6", atype)) {
        AdecType = HA_AUDIO_ID_DTSM6;
    } else if (!strcasecmp("dra", atype)) {
        AdecType = HA_AUDIO_ID_DRA;
    } else if (!strcasecmp("pcm", atype)) {
        AdecType = HA_AUDIO_ID_PCM;
    } else if (!strcasecmp("mlp", atype)) {
        AdecType = HA_AUDIO_ID_TRUEHD;
    } else if (!strcasecmp("amr", atype)) {
        AdecType = HA_AUDIO_ID_AMRNB;
    } else if (!strcasecmp("amrwb", atype)) {
        AdecType = HA_AUDIO_ID_AMRWB;
    } else {
        printf("unsupport aud codec type!\n");
        return -1;
    }
    return AdecType;
}

static MT_UNF_VCODEC_TYPE_E Get_vtype(char *vtype)
{
    MT_UNF_VCODEC_TYPE_E VdecType;
    if (!strcasecmp("mpeg2", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
    } else if (!strcasecmp("mpeg4", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_MPEG4;
    } else if (!strcasecmp("h263", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_H263;
    } else if (!strcasecmp("sor", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_SORENSON;
    } else if (!strcasecmp("vp6", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VP6;
    } else if (!strcasecmp("vp6f", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VP6F;
    } else if (!strcasecmp("vp6a", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VP6A;
    } else if (!strcasecmp("h264", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_H264;
    } else if (!strcasecmp("h265", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_HEVC;
    } else if (!strcasecmp("mvc", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_MVC;
    } else if (!strcasecmp("avs", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_AVS;
    } else if (!strcasecmp("real8", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_REAL8;
    } else if (!strcasecmp("real9", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_REAL9;
    } else if (!strcasecmp("vc1ap", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VC1;
    } else if (!strcasecmp("vc1smp5", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VC1;
    } else if (!strcasecmp("vc1smp8", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VC1;
    } else if (!strcasecmp("vp8", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_VP8;
    } else if (!strcasecmp("divx3", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_DIVX3;
    } else if (!strcasecmp("mjpeg", vtype)) {
        VdecType = MT_UNF_VCODEC_TYPE_MJPEG;
    } else {
        printf("unsupport vid codec type!\n");
        return -1;
    }
    return VdecType;
}

static mt_void TsTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S     StreamBuf;
    mt_u32                  Readlen;
    mt_s32                  Ret;
    mt_u32                  PushLen=188*1024;
    
    while (!g_bStopTsThread)
    {
        pthread_mutex_lock(&g_TsMutex);
        Ret = MT_UNF_DMX_GetTSBuffer(g_TsBuf, PushLen, &StreamBuf, 1000);
        if (Ret != MT_SUCCESS ){
            pthread_mutex_unlock(&g_TsMutex);
            continue;
        }
        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), PushLen, g_pTsFile);
        if(Readlen <= 0){
            printf("read ts file end and rewind!\n");
            rewind(g_pTsFile);
            pthread_mutex_unlock(&g_TsMutex);
            continue;
        }
        Ret = MT_UNF_DMX_PutTSBuffer(g_TsBuf, Readlen);
        if (Ret != MT_SUCCESS ){
            printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
        }

        pthread_mutex_unlock(&g_TsMutex);
    }

    Ret = MT_UNF_DMX_ResetTSBuffer(g_TsBuf);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_ResetTSBuffer failed.\n");
    }

    return;
}


static mt_void dvbplayThread(mt_void *args)
{
    mt_s32                  Ret;
    mt_handle               hAvplay;
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr;
    MT_UNF_SYNC_ATTR_S          SyncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S    Stop;
	
	//when use cec cmd, we need longer cmd len
    mt_char                 InputCmd[128];
    MT_UNF_ENC_FMT_E   enFormat = MT_UNF_ENC_FMT_1080i_50;
    mt_u32             ProgNum;

    mt_handle                   hTrack;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr;
    MT_BOOL                     bAudPlay = MT_TRUE;
    MT_BOOL                     bVidPlay = MT_TRUE;
    PMT_COMPACT_TBL             pProgTbl;
    PMT_COMPACT_PROG            proginfo;
    PMT_COMPACT_TBL             *l_pProgTbl = MT_NULL;
    
    MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, enFormat);
    Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != Ret){
        printf("call SndInit failed.\n");
        return;
    }

    Ret = MTADP_Disp_Init(enFormat);
    if (MT_SUCCESS != Ret){
        printf("call MTADP_Disp_Init failed.\n");
        goto SND_DEINIT;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL,&g_hWin);
    if (MT_SUCCESS != Ret){
        printf("call VoInit failed.\n");
        MTADP_VO_DeInit();
        goto DISP_DEINIT;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(PLAY_DMX_ID, MT_UNF_DMX_PORT_TSI_2);
#else
    Ret |= MT_UNF_DMX_AttachTSPort(PLAY_DMX_ID, MT_UNF_DMX_PORT_RAM_0);
#endif  	    
    Ret = MTADP_AVPlay_RegADecLib();
    if (MT_SUCCESS != Ret){
        printf("call RegADecLib failed.\n");
        goto VO_DEINIT;
    }
     
    Ret = MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_Init failed.\n");
        return;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
	AvplayAttr.u32DemuxId = PLAY_DMX_ID;
    AvplayAttr.stStreamAttr.u32VidBufSize = (16*1024*1024);
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_Create failed.\n");
        goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto AVPLAY_DESTROY;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto VCHN_CLOSE;
    }

    Ret = MT_UNF_VO_AttachWindow(g_hWin, hAvplay);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_VO_AttachWindow failed:%#x.\n",Ret);
    }
    Ret = MT_UNF_VO_SetWindowEnable(g_hWin, MT_TRUE);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto WIN_DETACH;
    }

    //stTrackAttr.enTrackType = MT_UNF_SND_TRACK_TYPE_MASTER;
    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto WIN_DETACH;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack); 
    if (Ret != MT_SUCCESS){
        printf("call MT_SND_Attach failed.\n");
        goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_Attach(hTrack, hAvplay);
    if (Ret != MT_SUCCESS){
        printf("call MT_SND_Attach failed.\n");
        goto TRACK_DESTROY;
    }
    printf("MTADP_AVPlay_PlayProg ..........................0\n");
    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
        goto SND_DETACH;
    }

    printf("MTADP_AVPlay_PlayProg ..........................1\n");
    if (bVidPlay){
         proginfo.VElementNum = 1;
         proginfo.VElementPid = (MT_U16)g_vpid;
         proginfo.VideoType = g_vtype;
    }
    if (bAudPlay){
         proginfo.AElementNum = 1;
         proginfo.AElementPid = (MT_U16)g_apid;
         proginfo.AudioType = g_atype;
    }

    pProgTbl.prog_num = 1;
    pProgTbl.proginfo = &proginfo;
    l_pProgTbl = &pProgTbl;
   
    ProgNum = 0;

    Ret = MTADP_AVPlay_PlayProg(hAvplay,l_pProgTbl,ProgNum,MT_TRUE);
    if (Ret != MT_SUCCESS){
        printf("call SwitchProg failed.\n");
        goto AVPLAY_STOP;
    }

    while(1){
        printf("please input 'h' to get help or 'q' to quit!\n");
        memset(InputCmd, 0, 128);
        SAMPLE_GET_INPUTCMD(InputCmd);
        if ('q' == InputCmd[0])
        {
            printf("prepare to quit!\n");
            break;
        }
    }

AVPLAY_STOP:
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);

    l_pProgTbl = MT_NULL;
    g_bStopTsThread = MT_TRUE;

SND_DETACH:
    MT_UNF_SND_Detach(hTrack, hAvplay);

TRACK_DESTROY:
    MT_UNF_SND_DestroyTrack(hTrack); 
    
WIN_DETACH:
    MT_UNF_VO_SetWindowEnable(g_hWin,MT_FALSE);
    MT_UNF_VO_DetachWindow(g_hWin, hAvplay);

    MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

VCHN_CLOSE:
    MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

AVPLAY_DESTROY:
    MT_UNF_AVPLAY_Destroy(hAvplay);

AVPLAY_DEINIT:
    MT_UNF_AVPLAY_DeInit();

VO_DEINIT:
    MT_UNF_VO_DestroyWindow(g_hWin);
    MTADP_VO_DeInit();

DISP_DEINIT:
	MTADP_Disp_DeInit();

SND_DEINIT:
    MTADP_Snd_DeInit();
}

static mt_s32 SectionRecvFunc(mt_handle Chandle)
{
    mt_s32 ret;
    mt_u32 i,j,k;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    printf("receive start!\n");
    while(!g_bStopTsThread){
        if(MT_SUCCESS != MT_UNF_DMX_AcquireBuf(Chandle,u32AcquireNum,&pu32AcquiredNum,pstBuf,5000)){
            printf("call MT_UNF_DMX_AcquireBuf failed!\n");
            MT_USLEEP(100 * 1000);
            continue;
        }
        for(i=0;i<pu32AcquiredNum;i++){
            printf("section:\n");
            for(j=0;j<pstBuf[i].u32Size;j++){
                printf("%02x ",pstBuf[i].pu8Data[j]);
            }
            printf("\n");
            k = pstBuf[i].u32Size;
            crcSoftware = CRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
            crcStream = pstBuf[i].pu8Data[k-4];
            crcStream = (crcStream<<8)|pstBuf[i].pu8Data[k-3];
            crcStream = (crcStream<<8)|pstBuf[i].pu8Data[k-2];
            crcStream = (crcStream<<8)|pstBuf[i].pu8Data[k-1];
            if(crcSoftware != crcStream){
                printf("CRC Check Failed !!!\n");
            }
        }
        ret = MT_UNF_DMX_ReleaseBuf(Chandle,pu32AcquiredNum,pstBuf);
        if (MT_SUCCESS != ret){
            printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
        }
    }
    printf("receive over!\n");
    return MT_SUCCESS;
}

mt_s32 main(mt_s32 argc,mt_char *argv[])
{//./testcase_id1 ./epg.ts 34 mpeg2 35 mp3
    mt_s32             Ret;
    MT_UNF_DMX_CHAN_ATTR_S stChnAttr;
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr;
    mt_handle hChannel;
    mt_handle hFilter1;

    if (6 != argc){
        printf("Usage: sample_tsplay file vpid vtype apid atype\n");
        printf("Example:./testcase_id1 ./test.ts 201 mpeg2 202 mp3\n");
        return -1;
    }
    g_vpid = (mt_u32)strtol(argv[2], NULL, 0);
    g_vtype= (mt_u32)Get_vtype(argv[3]);
    g_apid = (mt_u32)strtol(argv[4], NULL, 0);
    g_atype= (mt_u32)Get_atype(argv[5]);

    g_pTsFile = fopen(argv[1], "rb");
    if (!g_pTsFile){
        printf("open file %s error!\n", argv[1]);
        return -1;
    }

    mt_sys_init();

    MTADP_MCE_Exit();    

    Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_Init failed.\n");
        goto SYS_DEINIT;
    }  

#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_0);
#endif  
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        goto DMX_DEINIT;
    }

    Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &g_TsBuf);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_DMX_CreateTSBuffer failed.\n");
        goto TSBUF_FREE;
    }

    Ret = MT_UNF_DMX_GetChannelDefaultAttr(&stChnAttr);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_GetChannelDefaultAttr failed!\n");
        goto DMX_DEINIT;
    }
    stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    stChnAttr.u32BufSize = 16 * 1024;
    Ret = MT_UNF_DMX_CreateChannel(0,&stChnAttr,&hChannel);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_CreateChannel failed!\n");
        goto DMX_DEINIT;
    }

    /* set filter attr */
    memset(&g_mask,0,DMX_FILTER_MAX_DEPTH);
    memset(&g_match,0,DMX_FILTER_MAX_DEPTH);
    memset(&g_Negate,0,DMX_FILTER_MAX_DEPTH);
    stFilterAttr.u32FilterDepth = 1;
    g_match[0] = PAT_TABLEID;
    memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
    Ret = MT_UNF_DMX_CreateFilter(0,&stFilterAttr,&hFilter1);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_CreateFilter failed!\n");
        goto CHN_DESTROY;
    }
    Ret = MT_UNF_DMX_AttachFilter(hFilter1,hChannel);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_AttachFilter failed!\n");
        MT_UNF_DMX_DestroyFilter(hFilter1);
        goto CHN_DESTROY;
    }
    pthread_create(&g_TsThd1, MT_NULL, (mt_void *)TsTthread, MT_NULL);
    pthread_create(&g_TsThd2, MT_NULL, (mt_void *)dvbplayThread, MT_NULL);

    Ret = MT_UNF_DMX_SetChannelPID(hChannel,PAT_TSPID);
    Ret |= MT_UNF_DMX_OpenChannel(hChannel);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_OpenChannel failed!\n");
        goto FILTER1_DESTROY;
    }
    /* start to recv section data, and meanwhile print received section */
    SectionRecvFunc(hChannel);

    Ret = MT_UNF_DMX_CloseChannel(hChannel);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_CloseChannel failed!\n");
    }	

    FILTER1_DESTROY:
    MT_UNF_DMX_DetachFilter(hFilter1,hChannel);
    MT_UNF_DMX_DestroyFilter(hFilter1);
    CHN_DESTROY:
    MT_UNF_DMX_DestroyChannel(hChannel);

    TSBUF_FREE:
    MT_UNF_DMX_DestroyTSBuffer(g_TsBuf);

    DMX_DEINIT:
    MT_UNF_DMX_DetachTSPort(0);
    MT_UNF_DMX_DeInit();

    pthread_join(g_TsThd1, MT_NULL);
    //pthread_join(g_TsThd2, MT_NULL);
    pthread_mutex_destroy(&g_TsMutex);

    //MTADP_Tuner_DeInit();
    //MT_SYS_DeInit();
    SYS_DEINIT:
    //MT_SYS_DeInit();
    mt_sys_deinit();
    fclose(g_pTsFile);
    g_pTsFile = MT_NULL;

    return Ret;
}


