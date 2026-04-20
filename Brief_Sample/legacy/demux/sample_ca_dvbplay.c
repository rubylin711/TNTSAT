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
#include "mt_unf_descrambler.h"
#include "mt_unf_frontend.h"

//#include "mt_unf_advca.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"

#include "../common/mt_adp.h"
#include "../common/mt_adp_audio.h"
#include "../common/mt_adp_hdmi.h"
#include "../common/mt_adp_boardcfg.h"
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_frontend.h"

#ifndef DEFAULT_DVB_PORT
#define DEFAULT_DVB_PORT 1
#endif

#define INVALID_HANDLE 0xffffffff

PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;
static mt_handle g_hAvplay;
static mt_handle g_hNorcaDesc, g_hAdvcaDesc;
static mt_handle g_hNorca_CSA3_Desc, g_hAdvca_CSA3_Desc;

/* intergration and sample stream */
//"(stream: \\mfsclient01\mfsdisk\mdata01\CodeStreamSOC\stream\FunctionTest\Biss\4115_V_6990_Biss.ts, program:PTV-K FEED, pid=512   000000000007868D)"
static mt_u8 g_u8ClearOddKey[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x86, 0x8D};
static mt_u8 g_u8ClearEvenKey[8] =  {0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x86, 0x8D};

//static mt_u8 g_u8EncrytedOddKey[8] = { 0x44, 0x2C, 0xF6, 0x2E, 0x00, 0xD0, 0x77, 0x96 };
//static mt_u8 g_u8EncrytedEvenKey[8] = { 0xD8, 0xDC, 0x8E, 0x22, 0x04, 0x1A, 0xC4, 0x9B };


#if 1
//rootkey: 0xff, 0x00, 0x00, ....
static mt_u8 g_u8EncrptedCwpk[16] =
    { 0x3E, 0xEC, 0x20, 0xC8, 0xB2, 0x13, 0x27, 0xAB,
      0xF8, 0xD7, 0xEE, 0xFB, 0xF8, 0x42, 0x68, 0x0C };
#endif

//csa3_root_key:04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
//g_u8ClearCwpk 01 23 45 67 89 ab cd ef 01 01 01 01 01 01 01 01
//g_u8_CSA3_EncrptedCwpk: 82 18 65 d2 11 2b b7 0e b3 e7 34 05 4a 08 25 ea
static mt_u8 g_u8_CSA3_EncrptedCwpk[16] =
    { 0x82, 0x18, 0x65, 0xd2, 0x11, 0x2b, 0xb7, 0x0e, 0xb3, 0xe7, 0x34, 0x05, 0x4a, 0x08, 0x25, 0xea };
//Clear Odd key:d9 ec 13 d8 fa fd d9 d0 38 08 8f cf a0 df 69 e8
/*************************************************************************************************
*  g_u8_CSA3_ClearOddKey
*  g_u8_CSA3_ClearEvenKey  match ts stream: \\192.168.8.32\upload\temp_release\yuwu\teststream_for_qa\7daysEPG_201_csa3_mux_modify.ts
**************************************************************************************************/
static mt_u8 g_u8_CSA3_ClearOddKey[16] = { 0xc5, 0x82, 0xbf, 0x06, 0x7a, 0xa4, 0xed, 0x0b, 0xc5, 0x82, 0xbf, 0x06, 0x7a, 0xa4, 0xed, 0x0b };
static mt_u8 g_u8_CSA3_ClearEvenKey[16] = { 0xc5, 0x82, 0xbf, 0x06, 0x7a, 0xa4, 0xed, 0x0b, 0xc5, 0x82, 0xbf, 0x06, 0x7a, 0xa4, 0xed, 0x0b };

//Encrypted odd key: 68 DF 44 3D 24 02 0C 58 2E 9C 66 76 33 1F 33 07
static mt_u8 g_u8_CSA3_EncrytedOddKey[16] = { 0x68, 0xDF, 0x44, 0x3D, 0x24, 0x02, 0x0C, 0x58, 0x2E, 0x9C, 0x66, 0x76, 0x33, 0x1F, 0x33, 0x07 };
//Encrypted even key: 8C 3A BD 1E F4 29 BF 35 83 BB 6A 7B 09 F7 86 F1
static mt_u8 g_u8_CSA3_EncrytedEvenKey[16] = { 0x8C, 0x3A, 0xBD, 0x1E, 0xF4, 0x29, 0xBF, 0x35, 0x83, 0xBB, 0x6A, 0x7B, 0x09, 0xF7, 0x86, 0xF1 };

#define MAX_CMDLINE_LEN 1280
#define MAX_ARGS_COUNT 10
#define MAX_ARGS_LEN 128
static mt_char g_CmdLine[MAX_CMDLINE_LEN];
static mt_s32 g_Argc;
static mt_char g_Argv[MAX_ARGS_COUNT][MAX_ARGS_LEN];
static MT_BOOL g_bRun = MT_TRUE;
static mt_u32 g_avsync_mode = 0;

static mt_s32 tysys_set(MT_UNF_ENC_FMT_E enFormat)
{
    mt_s32 Ret = 0;
    Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY1, enFormat);
    if (Ret != MT_SUCCESS) {
        printf("call MT_UNF_DISP_SetFormat failed, hd Ret=%#x.\n", Ret);
        MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
        MT_UNF_DISP_DeInit();
        return Ret;
    }
    /* set sd format*/
    if ((MT_UNF_ENC_FMT_1080P_60 == enFormat) || (MT_UNF_ENC_FMT_1080i_60 == enFormat) || (MT_UNF_ENC_FMT_720P_60 == enFormat) || (MT_UNF_ENC_FMT_480P_60 == enFormat) || (MT_UNF_ENC_FMT_NTSC == enFormat) || (MT_UNF_ENC_FMT_3840X2160_30 == enFormat) || (MT_UNF_ENC_FMT_3840X2160_24 == enFormat)) {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_NTSC);
        if (MT_SUCCESS != Ret) {
            printf("call MT_UNF_DISP_SetFormat failed, sd ntsc Ret=%#x.\n", Ret);
            return Ret;
        }
    }

    if ((MT_UNF_ENC_FMT_1080P_50 == enFormat) || (MT_UNF_ENC_FMT_1080i_50 == enFormat) || (MT_UNF_ENC_FMT_720P_50 == enFormat) || (MT_UNF_ENC_FMT_576P_50 == enFormat) || (MT_UNF_ENC_FMT_PAL == enFormat) || (MT_UNF_ENC_FMT_3840X2160_25 == enFormat)) {
        Ret = MT_UNF_DISP_SetFormat(MT_UNF_DISPLAY0, MT_UNF_ENC_FMT_PAL);
        if (MT_SUCCESS != Ret) {
            printf("call MT_UNF_DISP_SetFormat failed, sd pal Ret=%#x.\n", Ret);
            return Ret;
        }
    }
    return Ret;
}

static mt_s32 parse_cmdline(mt_char *pCmdLine, mt_s32 *pArgc, mt_char Argv[MAX_ARGS_COUNT][MAX_ARGS_LEN])
{
    mt_char *ptr = pCmdLine;
    int i;
    while ((*ptr == ' ') && (*ptr++ != '\0'))
    {
        ;
    }

    for (i = (int)strlen(ptr); i > 0; i--) {
        if ((*(ptr + i - 1) == 0x0a) || (*(ptr + i - 1) == ' ')) {
            *(ptr + i - 1) = '\0';
        } else {
            break;
        }
    }

    for (i = 0; i < MAX_ARGS_COUNT; i++) {
        int j = 0;
        while ((*ptr == ' ') && (*(++ptr) != '\0')) {
        ;
        }

        while ((*ptr != ' ') && (*ptr != '\0') && (j < MAX_ARGS_LEN)) {
            Argv[i][j++] = *ptr++;
        }

        Argv[i][j] = '\0';
        if ('\0' == *ptr) {
            i++;
            break;
        }
    }
    *pArgc = i;

    return MT_SUCCESS;
}

static mt_void show_usage(mt_void)
{
/*
direct play test:
playbypid mpeg2 512 mp3 630 1
loadcw 0
stopplay

set 3des raw cw test:
playbypid mpeg2 512 mp3 630 1
loadcw 0
stopplay

set cas3 raw cw test:
playbypid mpeg2 201 mp3 202 1
loadcsa3 0
stopplay

*/
    printf("main cmd:./sample_ca_dvbplay 306 6875 64\n"
           "sub cmds:\n"
           "getchipid (Get ChipId)\n"
           "scan (Search And GetAllPmt)\n"
           "loadcw type(ClearOddKey g_u8_CSA3_ClearEvenKey)\n"
           "loadcsa3 type\n"
           "playbynum prognum[1-num]\n"
           "playloop loops\n"
           "playbypid vType vpid aType apid avsync_flage(playbypid mpeg2 512 mp3 630 1)\n"
           "stopplay\n"
           "dumpReg (dump all demux register)\n"
           "quit\n"
           "tvsys:1080i50,720p50,720p60,1080p30,1080p24,1080p25,720p60,480p60,576p50,576i50,480i60\n"
           "3840p30,3840p25,3840p24,4096p24\n"
           "vtype:mpeg2,mpeg4,h263,h264,h265,avs\n"
           "atype:mp3,aac,truehd,dts\n"
           "(stream: \\\\mfsclient01\\mfsdisk\\mdata01\\CodeStreamSOC\\stream\\FunctionTest\\Biss\\4115_V_6990_Biss.ts, program:PTV-K FEED, pid=512)\n");
}

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

static mt_s32 play_bypid(mt_handle hAvplay, MT_UNF_VCODEC_TYPE_E VdecType, mt_u32 VidPid, mt_u32 AdecType, mt_u32 AudPid, mt_u32 PcrPid, mt_u32 avsync_flag)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STOP_OPT_S StopOpt;
    static MT_UNF_AVPLAY_STOP_MODE_E enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    //StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    //StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    StopOpt.enMode = enMode;
    StopOpt.u32TimeoutMs = 0;

    printf("call MT_UNF_AVPLAY_Stop(%d).\n", enMode);
    Ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &StopOpt);

    /*
    // switch stop mode between Still & Black
    if (enMode == MT_UNF_AVPLAY_STOP_MODE_STILL)
    enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    else
    enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    */

    if (MT_SUCCESS != Ret) {
        printf("call MT_UNF_AVPLAY_Stop failed.\n");
        return MT_FAILURE;
    }
    if ((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {

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


static mt_s32 switch_audio(mt_handle hAvplay, mt_u32 AdecType, mt_u32 AudPid)
{
    mt_s32 Ret;
    MT_UNF_AVPLAY_STOP_OPT_S StopOpt;
    static MT_UNF_AVPLAY_STOP_MODE_E enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    //StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    //StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    StopOpt.enMode = enMode;
    StopOpt.u32TimeoutMs = 0;

    printf("call MT_UNF_AVPLAY_Stop(%d).\n", enMode);
    Ret = MT_UNF_AVPLAY_Stop(hAvplay,  MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &StopOpt);

    /*
    // switch stop mode between Still & Black
    if (enMode == MT_UNF_AVPLAY_STOP_MODE_STILL)
    enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    else
    enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    */

    if (MT_SUCCESS != Ret) {
        printf("call MT_UNF_AVPLAY_Stop failed.\n");
        return MT_FAILURE;
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

static mt_s32 load_csa3(mt_handle hAvplay, mt_u32 CwType)
{
    mt_s32 Ret;
    mt_handle hDmxVidChn, hDmxAudChn;
    mt_handle hDescrambler, hCaDesc;
    {
        /*
        open cas3
        reg = 0xbf30f0d0;
        start = 2;
        val = on?1:0;
        */
        mt_u32 readv;
        mt_u32 HAL_DS_CSA3=MT_UNF_DMX_GetRegister(0x10f0d0);
        printf("origin val=%x\n",HAL_DS_CSA3);
        MT_UNF_DMX_SetRegister(0x10f0d0,HAL_DS_CSA3|0x04);
        readv=MT_UNF_DMX_GetRegister(0x10f0d0);
        printf("seted val=%x\n",readv);
    }
    /* attach the same descrambler to audio and video channel*/
    hCaDesc = (0 == CwType) ? (g_hNorca_CSA3_Desc) : (g_hAdvca_CSA3_Desc);
    if (INVALID_HANDLE == hCaDesc) {
        printf("Unsupported CwType\n");
        return MT_FAILURE;
    }

    Ret = MT_UNF_AVPLAY_GetDmxVidChnHandle(hAvplay, &hDmxVidChn);
    Ret |= MT_UNF_AVPLAY_GetDmxAudChnHandle(hAvplay, &hDmxAudChn);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_AVPLAY_GetDmxChnHandle failed:%x\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxVidChn, &hDescrambler);
    if (MT_SUCCESS != Ret) {
        if (MT_ERR_DMX_NOATTACH_KEY == Ret) {
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxVidChn);
        } else {
            printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
            return Ret;
        }
    } else {
        if ((hDescrambler != hCaDesc)) {
            (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxVidChn);
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxVidChn);
        }
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxAudChn, &hDescrambler);
    if (MT_SUCCESS != Ret) {
        if (MT_ERR_DMX_NOATTACH_KEY == Ret) {
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxAudChn);
        } else {
            printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
            return Ret;
        }
    } else {
        if ((hDescrambler != hCaDesc)) {
            (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxAudChn);
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxAudChn);
        }
    }

    /* load cw*/
    if (0 == CwType) {
        Ret = MT_UNF_DMX_SetDescramblerOddKey(hCaDesc, g_u8_CSA3_ClearOddKey);
        Ret |= MT_UNF_DMX_SetDescramblerEvenKey(hCaDesc, g_u8_CSA3_ClearEvenKey);
        if (MT_SUCCESS != Ret) {
            printf("call MT_UNF_DMX_SetDescramblerKey failed\n");
            return MT_FAILURE;
        }
    } else {

        printf("call MT_UNF_ADVCA_SetDVBSessionKey failed, do not support !!!\n");
        #if 0
        Ret = MT_UNF_ADVCA_SetDVBAlg(MT_UNF_ADVCA_ALG_TYPE_TDES);
        /* first level */
        Ret |= MT_UNF_ADVCA_SetCSA3SessionKey(MT_UNF_ADVCA_KEYLADDER_LEV1,g_u8_CSA3_EncrptedCwpk);
        if (MT_SUCCESS != Ret)
        {
        printf("call MT_UNF_ADVCA_SetDVBSessionKey failed\n");
        return MT_FAILURE;
        }
        Ret = MT_UNF_DMX_SetDescramblerOddKey(hCaDesc,g_u8_CSA3_EncrytedOddKey);
        Ret |= MT_UNF_DMX_SetDescramblerEvenKey(hCaDesc,g_u8_CSA3_EncrytedEvenKey);
        if (MT_SUCCESS != Ret)
        {
        printf("call MT_UNF_DMX_SetDescramblerEvenKey failed\n");
        return MT_FAILURE;
        }
        #endif
    }

    return MT_SUCCESS;
}

static mt_s32 load_cw(mt_handle hAvplay, mt_u32 CwType)
{
    mt_s32 Ret;
    mt_handle hDmxVidChn, hDmxAudChn;
    mt_handle hDescrambler, hCaDesc;
    /* attach the same descrambler to audio and video channel*/
    // hCaDesc = (0 == CwType) ? (g_hNorcaDesc) : (g_hAdvcaDesc);
    hCaDesc = g_hNorcaDesc;
    if (INVALID_HANDLE == hCaDesc) {
        printf("Unsupported CwType\n");
        return MT_FAILURE;
    }

    Ret = MT_UNF_AVPLAY_GetDmxVidChnHandle(hAvplay, &hDmxVidChn);
    Ret |= MT_UNF_AVPLAY_GetDmxAudChnHandle(hAvplay, &hDmxAudChn);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_AVPLAY_GetDmxChnHandle failed:%x\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxVidChn, &hDescrambler);
    if (MT_SUCCESS != Ret) {
        if (MT_ERR_DMX_NOATTACH_KEY == Ret) {
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxVidChn);
        } else {
            printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
            return Ret;
        }
    } else {
        if ((hDescrambler != hCaDesc)) {
            (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxVidChn);
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxVidChn);
        }
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxAudChn, &hDescrambler);
    if (MT_SUCCESS != Ret) {
        if (MT_ERR_DMX_NOATTACH_KEY == Ret) {
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxAudChn);
        } else {
            printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
            return Ret;
        }
    } else {
        if ((hDescrambler != hCaDesc)) {
            (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxAudChn);
            (mt_void) MT_UNF_DMX_AttachDescrambler(hCaDesc, hDmxAudChn);
        }
    }
    /* load cw*/
    if (0 == CwType)
    {
        printf("MT_UNF_DMX_SetDescramblerOddKey   g_u8ClearOddKey\n"); 
        Ret = MT_UNF_DMX_SetDescramblerOddKey(hCaDesc, g_u8ClearOddKey);
        Ret |= MT_UNF_DMX_SetDescramblerEvenKey(hCaDesc, g_u8ClearEvenKey);
        if (MT_SUCCESS != Ret) {
            printf("call MT_UNF_DMX_SetDescramblerKey failed\n");
            return MT_FAILURE;
        }
    }
    else
    {
        printf("call MT_UNF_ADVCA_SetDVBSessionKey failed, do not support !!!\n");
        #if 0
        Ret = MT_UNF_ADVCA_SetDVBAlg(MT_UNF_ADVCA_ALG_TYPE_TDES);
        /* first level */
        Ret |= MT_UNF_ADVCA_SetDVBSessionKey(MT_UNF_ADVCA_KEYLADDER_LEV1,g_u8EncrptedCwpk);
        if (MT_SUCCESS != Ret)
        {
            printf("call MT_UNF_ADVCA_SetDVBSessionKey failed\n");
            return MT_FAILURE;
        }
        Ret = MT_UNF_DMX_SetDescramblerOddKey(hCaDesc,g_u8EncrytedOddKey);
        Ret |= MT_UNF_DMX_SetDescramblerEvenKey(hCaDesc,g_u8EncrytedEvenKey);
        if (MT_SUCCESS != Ret)
        {
            printf("call MT_UNF_DMX_SetDescramblerEvenKey failed\n");
            return MT_FAILURE;
        }
        #endif
    }

    return MT_SUCCESS;
}

static mt_s32 run_cmdline(mt_s32 argc, mt_char argv[MAX_ARGS_COUNT][MAX_ARGS_LEN])
{
    mt_s32 Ret = MT_FAILURE;

    printf("argv[0] = %s\n", argv[0]);
    if (!strcmp(argv[0], "help")) 
    {
        show_usage();
    }
    else if (!strcmp(argv[0], "getchipid")) 
    {
        mt_sys_version_s stSysChipInfo;
        memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
        mt_sys_get_version(&stSysChipInfo);
        printf("ChipVersion:0x%x\n", stSysChipInfo.enChipVersion);
    }
    else if (!strcmp(argv[0], "scan")) 
    {
        MTADP_Search_Init();
        Ret = MTADP_Search_GetAllPmt(0, &g_pProgTbl);
        if (MT_SUCCESS != Ret) {
            printf("call MTADP_Search_GetAllPmt failed\n");
            return MT_FAILURE;
        }
        {
            int i;
            printf("All prog_num = %d:\n",  g_pProgTbl->prog_num);
            for(i=0; i<(int)g_pProgTbl->prog_num; i++){
                printf("vpid = %x , videoType = %x \n",
                g_pProgTbl->proginfo[i].VElementPid, g_pProgTbl->proginfo[i].VideoType);
                printf("apid = %x , audioType = %x \n",
                g_pProgTbl->proginfo[i].AElementPid, g_pProgTbl->proginfo[i].AudioType);
            }
        }
    }else if (!strcmp(argv[0], "loadcw")){
        mt_u32 CwType;
        if (argc < 2) {
            printf("invalid arguments\n");
            return MT_FAILURE;
        }
        CwType = (mt_u32)strtol(argv[1], NULL, 0);
        Ret = load_cw(g_hAvplay, CwType);
        if (MT_SUCCESS != Ret) {
            printf("load cw failed\n");
        }
    }else if (!strcmp(argv[0], "loadcsa3")){
        mt_u32 CwType;
        if (argc < 2) {
            printf("invalid arguments\n");
            return MT_FAILURE;
        }
        CwType = (mt_u32)strtol(argv[1], NULL, 0);
        Ret = load_csa3(g_hAvplay, CwType);
        if (MT_SUCCESS != Ret) {
            printf("load cw failed\n");
        }
    }else if (!strcmp(argv[0], "playbypid")){
        mt_u32 VidPid, AudPid, pcrPid;
        MT_UNF_VCODEC_TYPE_E VdecType = MT_UNF_VCODEC_TYPE_BUTT;
        mt_u32 AdecType = 0;
        mt_u32 avsync_flage = 0;

        if (argc < 5) {
            printf("invalid arguments\n");
            return MT_FAILURE;
        }

        VidPid = (mt_u32)strtol(argv[2], NULL, 0);
        AudPid = (mt_u32)strtol(argv[4], NULL, 0);
        g_avsync_mode = (mt_u32)strtol(argv[5], NULL, 0);
        if (g_avsync_mode == 0) {
            avsync_flage = 0;
        } else {
            avsync_flage = 1;
        }

        if (argc > 5) {
            pcrPid = (mt_u32)strtol(argv[6], NULL, 0);
        } else {
            pcrPid = VidPid;
        }

        if (!strcasecmp("mpeg2", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_MPEG2;
        } else if (!strcasecmp("mpeg4", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_MPEG4;
        } else if (!strcasecmp("h263", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_H263;
        } else if (!strcasecmp("sor", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_SORENSON;
        } else if (!strcasecmp("vp6", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VP6;
        } else if (!strcasecmp("vp6f", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VP6F;
        } else if (!strcasecmp("vp6a", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VP6A;
        } else if (!strcasecmp("h264", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_H264;
        } else if (!strcasecmp("h265", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_HEVC;
        } else if (!strcasecmp("mvc", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_MVC;
        } else if (!strcasecmp("avs", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_AVS;
        } else if (!strcasecmp("real8", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_REAL8;
        } else if (!strcasecmp("real9", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_REAL9;
        } else if (!strcasecmp("vc1ap", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VC1;
        } else if (!strcasecmp("vc1smp5", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VC1;
        } else if (!strcasecmp("vc1smp8", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VC1;
        } else if (!strcasecmp("vp8", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_VP8;
        } else if (!strcasecmp("divx3", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_DIVX3;
        } else if (!strcasecmp("mjpeg", argv[1])) {
            VdecType = MT_UNF_VCODEC_TYPE_MJPEG;
        } else {
            printf("unsupport vid codec type!\n");
            return -1;
        }

        if (!strcasecmp("aac", argv[3])) {
            AdecType = HA_AUDIO_ID_AAC;
        } else if (!strcasecmp("mp3", argv[3])) {
            AdecType = HA_AUDIO_ID_MP3;
        } else if (!strcasecmp("truehd", argv[3])) {
            AdecType = HA_AUDIO_ID_DOLBY_TRUEHD;
        } else if (!strcasecmp("ac3raw", argv[3])) {
            AdecType = HA_AUDIO_ID_AC3PASSTHROUGH;
        } else if (!strcasecmp("dtsraw", argv[3])) {
            AdecType = HA_AUDIO_ID_DTSPASSTHROUGH;
        }
#if defined(DOLBYPLUS_HACODEC_SUPPORT)
        else if (!strcasecmp("ddp", argv[3])) {
            AdecType = HA_AUDIO_ID_DOLBY_PLUS;
        }
#endif
        else if (!strcasecmp("dts", argv[3])) {
            AdecType = HA_AUDIO_ID_DTSHD;
        } else if (!strcasecmp("dtsm6", argv[3])) {
            AdecType = HA_AUDIO_ID_DTSM6;
        } else if (!strcasecmp("dra", argv[3])) {
            AdecType = HA_AUDIO_ID_DRA;
        } else if (!strcasecmp("pcm", argv[3])) {
            AdecType = HA_AUDIO_ID_PCM;
        } else if (!strcasecmp("mlp", argv[3])) {
            AdecType = HA_AUDIO_ID_TRUEHD;
        } else if (!strcasecmp("amr", argv[3])) {
            AdecType = HA_AUDIO_ID_AMRNB;
        } else if (!strcasecmp("amrwb", argv[3])) {
            AdecType = HA_AUDIO_ID_AMRWB;
        } else {
            printf("unsupport aud codec type!\n");
            return -1;
        }

        Ret = play_bypid(g_hAvplay, VdecType, VidPid, AdecType, AudPid, pcrPid, avsync_flage);
        if (MT_SUCCESS != Ret) {
            printf("play prog failed:%x\n", Ret);
            return MT_FAILURE;
        }
    }
    else if (!strcmp(argv[0], "switchAudio")){
        mt_u32  AudPid;
        mt_u32 AdecType = 0;
        AudPid = (mt_u32)strtol(argv[2], NULL, 0);
        
        if (!strcasecmp("aac", argv[1])) {
            AdecType = HA_AUDIO_ID_AAC;
        } else if (!strcasecmp("mp3", argv[1])) {
            AdecType = HA_AUDIO_ID_MP3;
        } else if (!strcasecmp("truehd", argv[1])) {
            AdecType = HA_AUDIO_ID_DOLBY_TRUEHD;
        } else if (!strcasecmp("ac3raw", argv[1])) {
            AdecType = HA_AUDIO_ID_AC3PASSTHROUGH;
        } else if (!strcasecmp("dtsraw", argv[1])) {
            AdecType = HA_AUDIO_ID_DTSPASSTHROUGH;
        }
#if defined(DOLBYPLUS_HACODEC_SUPPORT)
        else if (!strcasecmp("ddp", argv[1])) {
            AdecType = HA_AUDIO_ID_DOLBY_PLUS;
        }
#endif
        else if (!strcasecmp("dts", argv[1])) {
            AdecType = HA_AUDIO_ID_DTSHD;
        } else if (!strcasecmp("dtsm6", argv[1])) {
            AdecType = HA_AUDIO_ID_DTSM6;
        } else if (!strcasecmp("dra", argv[1])) {
            AdecType = HA_AUDIO_ID_DRA;
        } else if (!strcasecmp("pcm", argv[1])) {
            AdecType = HA_AUDIO_ID_PCM;
        } else if (!strcasecmp("mlp", argv[1])) {
            AdecType = HA_AUDIO_ID_TRUEHD;
        } else if (!strcasecmp("amr", argv[1])) {
            AdecType = HA_AUDIO_ID_AMRNB;
        } else if (!strcasecmp("amrwb", argv[1])) {
            AdecType = HA_AUDIO_ID_AMRWB;
        } else {
            printf("unsupport aud codec type!\n");
            return -1;
        }

        Ret = switch_audio(g_hAvplay,  AdecType, AudPid);
        if (MT_SUCCESS != Ret) {
            printf("play prog failed:%x\n", Ret);
            return MT_FAILURE;
        }
    }
    else if (!strcmp(argv[0], "playbynum")) 
    {
        mt_u32 ProgNum;

        if (MT_NULL == g_pProgTbl || 0 == g_pProgTbl->prog_num) {
            printf("run tunerlock first, Search_GetAllPmt\n");
            MTADP_Search_Init();
            Ret = MTADP_Search_GetAllPmt(0, &g_pProgTbl);
            if (MT_SUCCESS != Ret) {
                printf("call MTADP_Search_GetAllPmt failed\n");
                return MT_FAILURE;
            }
            printf("All prog_num = %d:\n",  g_pProgTbl->prog_num);
        }

        if (argc < 2) {
            printf("invalid arguments\n");
            return MT_FAILURE;
        }
        ProgNum = (mt_u32)strtol(argv[1], NULL, 0);
        Ret = MTADP_AVPlay_PlayProg(g_hAvplay, g_pProgTbl, (ProgNum-1)%g_pProgTbl->prog_num, MT_TRUE);
        if (MT_SUCCESS != Ret) {
            printf("play prog failed:%x\n", Ret);
            return MT_FAILURE;
        }
    }
    else if (!strcmp(argv[0], "playloop")) 
    {
        /* int pInfo[3][4] = {{MT_UNF_VCODEC_TYPE_H264, 47, HA_AUDIO_ID_MP3, 48},
                   {MT_UNF_VCODEC_TYPE_MPEG2, 40, HA_AUDIO_ID_MP3, 41},
                   {MT_UNF_VCODEC_TYPE_MPEG2, 34, HA_AUDIO_ID_MP3, 35}};*/
        int i = 0;
        int j = 0;
        int loopNum = 200;
        if (MT_NULL == g_pProgTbl || 0 == g_pProgTbl->prog_num) {
            printf("run tunerlock first, Search_GetAllPmt\n");
            MTADP_Search_Init();
            Ret = MTADP_Search_GetAllPmt(0, &g_pProgTbl);
            if (MT_SUCCESS != Ret) {
                printf("call MTADP_Search_GetAllPmt failed\n");
                return MT_FAILURE;
            }
        }

        if (argc >= 2) {
            loopNum = (int)strtol(argv[1], NULL, 0);
        }
        while (j++ < loopNum) {
            for (i = 0; i < (int)g_pProgTbl->prog_num; i++) {
                if (g_pProgTbl->proginfo[i].VElementPid <= 0){
                    continue;
                }
                play_bypid(g_hAvplay, g_pProgTbl->proginfo[i].VideoType, g_pProgTbl->proginfo[i].VElementPid,
                   g_pProgTbl->proginfo[i].AudioType, g_pProgTbl->proginfo[i].AElementPid, g_pProgTbl->proginfo[i].PcrPid, 1);
                MT_USLEEP(1000000);
                printf("jjjjjjjjj = %d iii = %d \n", j, i);
                MT_USLEEP(1000000);
            }
        }
    } 
    else if (!strcmp(argv[0], "pause")) 
    {
        MT_UNF_AVPLAY_Pause(g_hAvplay, MT_NULL);
    } 
    else if (!strcmp(argv[0], "freeze")) 
    {
        MT_UNF_AVPLAY_FREEZE_OPT_S stFreezeOpt;
        MT_UNF_AVPLAY_Freeze(g_hAvplay, &stFreezeOpt);
    } 
    else if (!strcmp(argv[0], "resume")) 
    {
        MT_UNF_AVPLAY_RESUME_OPT_S stResumeOpt;
        MT_UNF_AVPLAY_Resume(g_hAvplay, &stResumeOpt);
    } 
    else if (!strcmp(argv[0], "stopplay")) 
    {
        MT_UNF_AVPLAY_STOP_OPT_S StopOpt;
        mt_u32 disp_mode = 0;

        StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
        disp_mode = (mt_u32)strtol(argv[1], NULL, 0);

        if (1 == disp_mode) {
            StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
        }

        StopOpt.u32TimeoutMs = 0;
        Ret = MT_UNF_AVPLAY_Stop(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &StopOpt);
        if (MT_SUCCESS != Ret) {
            printf("call MT_UNF_AVPLAY_Stop failed.\n");
            return Ret;
        }
    } 
    else if (!strcmp(argv[0], "tvsys")) 
    {
        MT_UNF_ENC_FMT_E enFormat;
        if (!strcmp("1080i50", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080i_50;
        } else if (!strcmp("1080i60", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080i_60;
        } else if (!strcmp("1080p50", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080P_50;
        } else if (!strcmp("1080p60", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080P_60;
        } else if (!strcmp("720p50", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_720P_50;
        } else if (!strcmp("720p60", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_720P_60;
        } else if (!strcmp("480p60", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_480P_60;
        } else if (!strcmp("576p50", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_576P_50;
        } else if (!strcmp("1080p25", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080P_25;
        } else if (!strcmp("1080p30", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080P_30;
        } else if (!strcmp("1080p24", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_1080P_24;
        } else if (!strcmp("480i60", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_NTSC;
        } else if (!strcmp("576i50", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_PAL;
        } else if (!strcmp("3840p30", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_3840X2160_30;
        } else if (!strcmp("3840p25", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_3840X2160_25;
        } else if (!strcmp("3840p24", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_3840X2160_24;
        } else if (!strcmp("4096p24", argv[1])) {
            enFormat = MT_UNF_ENC_FMT_4096X2160_24;
        } else {
            enFormat = MT_UNF_ENC_FMT_1080i_50;
        }
        printf("tvsys %s %d\n", argv[1], enFormat);
        tysys_set(enFormat);
    }
    else if (!strcmp(argv[0], "rr")) 
    {
        /*int offset = (int)strtol(argv[1], NULL, 0);
        int count = (int)strtol(argv[2], NULL, 0);
        int i=0;
        for(i=0; i<count; i++)
        {
            printf("reg = %x, vaule = %x\n", 0xbf200000 + offset + 4*i, MT_UNF_DMX_GetRegister(offset+ 4*i));
        }*/
    } 
    else if (!strcmp(argv[0], "wr")) 
    {
        /*int offset = (int)strtol(argv[1], NULL, 0);
        int value = (int)strtol(argv[2], NULL, 0);
        MT_UNF_DMX_SetRegister(0xbf200000 + offset,  value);
        printf("reg = %x, vaule = %x\n", 0xbf200000 + offset, MT_UNF_DMX_GetRegister(offset));*/       
    }
    else if(!strcmp(argv[0],  "dumpReg"))
    {
        printf("dump demux all register:\n");
        MT_UNF_DMX_DumpAllRegister();
    }
    else if (!strcmp(argv[0], "quit")) 
    {
        g_bRun = MT_FALSE;
    }
    else 
    {
        printf("invalid command\n");
        show_usage();
    }

    return MT_SUCCESS;
}

mt_s32 main(mt_s32 argc, mt_char *argv[])
{//./sample_ca_dvbplay 306 6875 64
    mt_s32 Ret;
    mt_handle hWin;
    MT_UNF_AVPLAY_ATTR_S AvplayAttr;
    MT_UNF_SYNC_ATTR_S SyncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S Stop;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S stNorcaDescAttr, stAdvcaDescAttr;

    mt_handle hTrack;                     //++
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr; //++

    mt_u32 freq = 0;
    mt_u32 symbol_rate = 0;
    mt_u32 qam_mode = 0;
    mt_u32 u32DmxId = 0;
    mt_sys_version_s stSysChipInfo;

    mt_sys_init();
    Ret = MT_UNF_DMX_Init();

    freq = (mt_u32)strtol(argv[1], NULL, 0);
    symbol_rate = (mt_u32)strtol(argv[2], NULL, 0);
    qam_mode = (mt_u32)strtol(argv[3], NULL, 0);
    mtadp_fe_init();
    if ((16<=qam_mode) && (256>=qam_mode)){     //dvbc
        mtadp_fe_connect(0, freq, symbol_rate, qam_mode);
    }else if(0<qam_mode){                       //dvbt.t2
        mtadp_fe_connect_dvbtauto(0,freq,qam_mode);
    }else{
        mtadp_fe_connect_dvbs(0,freq, symbol_rate,0, 0, 0);
    }

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);  
    printf("Function : %s  argc=%d, stSysChipInfo.enChipVersion=0x%x\n",__FUNCTION__,argc, stSysChipInfo.enChipVersion);

    if (0<qam_mode){ //dvbc
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.C
            Ret |= MT_UNF_DMX_AttachTSPort(u32DmxId, MT_UNF_DMX_PORT_TSI_1);
        }else{                                                      //sym1.C
            Ret |= MT_UNF_DMX_AttachTSPort(u32DmxId, MT_UNF_DMX_PORT_TSI_0);
        }
    }else{
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.S
            Ret |= MT_UNF_DMX_AttachTSPort(u32DmxId, MT_UNF_DMX_PORT_TSI_0);
        }else{                                                      //sym1.S
            Ret |= MT_UNF_DMX_AttachTSPort(u32DmxId, MT_UNF_DMX_PORT_TSI_1);
        }
    }

    if (MT_SUCCESS != Ret){
        printf("call mtadp_fe_connect failed.\n");
        goto DES_DESTROY;
    }

    MTADP_MCE_Exit();
    Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_50);
    if (MT_SUCCESS != Ret){
        printf("call MTADP_Snd_Init failed.\n");
        goto CA_DEINIT;
    }

    Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != Ret){
        printf("call MTADP_Snd_Init failed.\n");
        goto CA_DEINIT;
    }

    Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_1080i_50);
    if (MT_SUCCESS != Ret){
        printf("call MTADP_Disp_DeInit failed.\n");
        goto SND_DEINIT;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL, &hWin);
    if (MT_SUCCESS != Ret){
        printf("call MTADP_VO_Init failed.\n");
        (mt_void) MTADP_VO_DeInit();
        goto DISP_DEINIT;
    }

    memset(&stNorcaDescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));
    stNorcaDescAttr.enCaType = MT_UNF_DMX_CA_NORMAL;
    stNorcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2;
    Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stNorcaDescAttr, &g_hNorcaDesc);
    if (MT_SUCCESS != Ret){
        g_hNorcaDesc = INVALID_HANDLE;
    }

    memset(&stAdvcaDescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));
    stAdvcaDescAttr.enCaType = MT_UNF_DMX_CA_ADVANCE;
    stAdvcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2;
    Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stAdvcaDescAttr, &g_hAdvcaDesc);
    if (MT_SUCCESS != Ret){
        g_hAdvcaDesc = INVALID_HANDLE;
    }

    //CSA3.0
    memset(&stNorcaDescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));
    stNorcaDescAttr.enCaType = MT_UNF_DMX_CA_NORMAL;
    stNorcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3;
    Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stNorcaDescAttr, &g_hNorca_CSA3_Desc);
    if (MT_SUCCESS != Ret){
        g_hNorca_CSA3_Desc = INVALID_HANDLE;
    }
    //CSA3.0
    memset(&stAdvcaDescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));
    stAdvcaDescAttr.enCaType = MT_UNF_DMX_CA_ADVANCE;
    stAdvcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3;
    Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stAdvcaDescAttr, &g_hAdvca_CSA3_Desc);
    if (MT_SUCCESS != Ret){
        g_hAdvca_CSA3_Desc = INVALID_HANDLE;
    }

    Ret = MTADP_AVPlay_RegADecLib();
    Ret |= MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_Init failed.\n");
        goto PSISI_FREE;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    AvplayAttr.stStreamAttr.u32AudBufSize = (192 * 1024);
#endif

    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &g_hAvplay);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_Create failed.\n");
        goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    Ret |= MT_UNF_AVPLAY_ChnOpen(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto CHN_CLOSE;
    }

    Ret = MT_UNF_VO_AttachWindow(hWin, g_hAvplay);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_VO_AttachWindow failed.\n");
        goto CHN_CLOSE;
    }
    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto WIN_DETACH;
    }

    memset(&stTrackAttr, 0, sizeof(stTrackAttr));
    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto WIN_DETACH;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_SND_CreateTrack failed.\n");
        goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_Attach(hTrack, g_hAvplay);
    if (Ret != MT_SUCCESS){
        printf("call MT_SND_Attach failed.\n");
        goto TRACK_DESTROY;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(g_hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    Ret = MT_UNF_AVPLAY_SetAttr(g_hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS){
        printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
        goto SND_DETACH;
    }
    /*  
    **  add autoplay 
    */
    if ((argc == 5 || argc == 6) && (!strcmp(argv[4], "autoplay"))){
        mt_u8 ProgNum = 0;
        if (MT_NULL == g_pProgTbl || 0 == g_pProgTbl->prog_num) {
            printf("run tunerlock first, Search_GetAllPmt\n");

            MTADP_Search_Init();
            Ret = MTADP_Search_GetAllPmt(0, &g_pProgTbl);
            if (MT_SUCCESS != Ret) {
                printf("call MTADP_Search_GetAllPmt failed\n");
                return MT_FAILURE;
            }
        }
        ProgNum = 0;
        if (argc == 6){
            ProgNum = (mt_u8)strtol(argv[5], NULL, 0);
        }
        Ret = MTADP_AVPlay_PlayProg(g_hAvplay, g_pProgTbl, ProgNum, MT_TRUE);
        if (MT_SUCCESS != Ret) {
            printf("play prog failed:%x\n", Ret);
            return MT_FAILURE;
        }
        while (1) {
            mt_char InputCmd[32];
            printf("please input the q to quit!\n");
            fgets(InputCmd, 30, stdin);
            if ('q' == InputCmd[0]) {
                printf("prepare to quit!\n");
                g_bRun = MT_FALSE;
                break;
            }
        }
    }

    show_usage();

    while (g_bRun) {
        mt_char *pCmdLine=NULL;
        mt_s32 chRet;
        printf("\n>");
        pCmdLine = fgets(g_CmdLine, MAX_CMDLINE_LEN, stdin);
        if (MT_NULL == pCmdLine) {
            show_usage();
            continue;
        }
        printf("pCmdLine = %s\n", pCmdLine);
        chRet = parse_cmdline(pCmdLine, &g_Argc, g_Argv);
        if (MT_SUCCESS != chRet || 0 == g_Argc) {
            show_usage();
            continue;
        }
        (mt_void) run_cmdline(g_Argc, g_Argv);
    }

    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    (mt_void) MT_UNF_AVPLAY_Stop(g_hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);

    SND_DETACH:
    (mt_void) MT_UNF_SND_Detach(hTrack, g_hAvplay);

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

    PSISI_FREE:
    (mt_void) MTADP_Search_FreeAllPmt(g_pProgTbl);
    MTADP_Search_DeInit();

    DES_DESTROY:
    (mt_void) MT_UNF_DMX_DestroyDescrambler(g_hNorcaDesc);
    (mt_void) MT_UNF_DMX_DestroyDescrambler(g_hAdvcaDesc);

    (mt_void) MT_UNF_DMX_DetachTSPort(u32DmxId);
    (mt_void) MT_UNF_DMX_DeInit();
    
    (mt_void) MT_UNF_VO_DestroyWindow(hWin);
    (mt_void) MTADP_VO_DeInit();

    DISP_DEINIT:
    (mt_void) MTADP_Disp_DeInit();

    SND_DEINIT:
    (mt_void) MTADP_Snd_DeInit();
    CA_DEINIT:
    //(mt_void)MT_UNF_ADVCA_DeInit();
    mtadp_fe_deinit();
    mt_sys_deinit();

    return Ret;
}

