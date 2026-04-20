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
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_demux.h"

#define PAT_TABLEID     0
#define PMT_TABLEID     2
#define TDT_TABLEID     0x70
#define EIT_TABLEID0    0x4e
#define EIT_TABLEID1    0x50
#define EIT_TABLEID2    0x51

#define TUNER_PORT      0
#define DMX_ID          0

#ifndef PAT_TSPID
#define PAT_TSPID       0
#endif
#ifndef TDT_TSPID
#define TDT_TSPID       20
#endif
#ifndef EIT_TSPID
#define EIT_TSPID       18
#endif

mt_u32 g_TunerFreq;
mt_u32 g_TunerSrate;
mt_u32 g_ThirdParam;
static mt_u8 g_match[DMX_FILTER_MAX_DEPTH];
static mt_u8 g_mask[DMX_FILTER_MAX_DEPTH];
static mt_u8 g_Negate[DMX_FILTER_MAX_DEPTH];

#define  MAX_FILTER_NUM     64

/*ts file: 7daysEPG.ts*/
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
#define BUFFER_SIZE     (32*1024)
#define  MAX_CHANNL_NUM     2
static mt_u32 section_pid[MAX_CHANNL_NUM] = {EIT_TSPID,EIT_TSPID};     
#else
#define  MAX_CHANNL_NUM     3
#define BUFFER_SIZE     (16*1024)
static mt_u32 section_pid[MAX_CHANNL_NUM] = {PAT_TSPID,TDT_TSPID,EIT_TSPID};     
#endif

static mt_handle MtChannel[MAX_CHANNL_NUM];
static mt_handle MtFilter[MAX_CHANNL_NUM][MAX_FILTER_NUM];

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

static mt_u32 CRC32(mt_u8 *buffer, mt_u32 size)
{
    mt_u32 Result = 0xFFFFFFFF;

    while (size--){
        Result = (Result << 8) ^ m_Table[(Result >> 24) ^ *buffer ++];
    }

    return Result;
}

static void dmx_data_dump(mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    for(i=0;i<lens;i++){
        printf("%02x ",head[i]);
        if((i&7)==7){
            printf("\n");
        }
    }
    printf("\n");
}

/*
mt_void TsTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S   StreamBuf;
    mt_u32                Readlen;
    mt_s32                Ret;

    while (!g_bStopTsThread)
    {
        pthread_mutex_lock(&g_TsMutex);
        Ret = MT_UNF_DMX_GetTSBuffer(g_TsBuf, 188*1024, &StreamBuf, 1000);
        if (Ret != MT_SUCCESS )
        {
            pthread_mutex_unlock(&g_TsMutex);
            continue;
        }
        //printf("fread ..........%x...........%x.....\n", g_pTsFile, StreamBuf.pu8Data);
        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), 188*1024, g_pTsFile);
        printf("fread .......................Readlen=%d\n",Readlen);
        if(Readlen <= 0)
        {
            printf("read ts file end and rewind!\n");
            rewind(g_pTsFile);
            pthread_mutex_unlock(&g_TsMutex);
            continue;
        }
        //printf("fread .......................line=%d\n",__LINE__);
        Ret = MT_UNF_DMX_PutTSBuffer(g_TsBuf, Readlen);
        if (Ret != MT_SUCCESS )
        {
            printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
        }
        //printf("fread .......................line=%d\n",__LINE__);        
        //MT_USLEEP(1000*100);
        pthread_mutex_unlock(&g_TsMutex);
    }

    Ret = MT_UNF_DMX_ResetTSBuffer(g_TsBuf);
    if (Ret != MT_SUCCESS )
    {
        printf("call MT_UNF_DMX_ResetTSBuffer failed.\n");
    }

    return;
}


mt_void dvbplayThread(mt_void *args)
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
    mt_u32                      VPid=201;
    mt_u32                      APid=202;
    MT_BOOL                     bAudPlay = MT_TRUE;
    MT_BOOL                     bVidPlay = MT_TRUE;
    PMT_COMPACT_TBL             pProgTbl;
    PMT_COMPACT_PROG            proginfo;


    Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, enFormat);
    Ret = MTADP_Snd_Init();
    if (MT_SUCCESS != Ret)
    {
        printf("call SndInit failed.\n");
        return;
    }

    Ret = MTADP_Disp_Init(enFormat);
    if (MT_SUCCESS != Ret)
    {
        printf("call MTADP_Disp_Init failed.\n");
        goto SND_DEINIT;
    }

    Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
    Ret |= MTADP_VO_CreatWin(MT_NULL,&g_hWin);
    if (MT_SUCCESS != Ret)
    {
        printf("call VoInit failed.\n");
        MTADP_VO_DeInit();
        goto DISP_DEINIT;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_0);
#endif    
    Ret = MTADP_AVPlay_RegADecLib();
    if (MT_SUCCESS != Ret)
    {
        printf("call RegADecLib failed.\n");
        goto VO_DEINIT;
    }
     
    Ret = MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_AVPLAY_Init failed.\n");
        return;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
	AvplayAttr.u32DemuxId = PLAY_DMX_ID;
    AvplayAttr.stStreamAttr.u32VidBufSize = (3*1024*1024);
    Ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_AVPLAY_Create failed.\n");
        goto AVPLAY_DEINIT;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto AVPLAY_DESTROY;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto VCHN_CLOSE;
    }

    Ret = MT_UNF_VO_AttachWindow(g_hWin, hAvplay);
    if (MT_SUCCESS != Ret)
    {
        printf("call MT_UNF_VO_AttachWindow failed:%#x.\n",Ret);
    }
    Ret = MT_UNF_VO_SetWindowEnable(g_hWin, MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto WIN_DETACH;
    }

    //stTrackAttr.enTrackType = MT_UNF_SND_TRACK_TYPE_MASTER;
    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto WIN_DETACH;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hTrack); 
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_SND_Attach failed.\n");
        goto WIN_DETACH;
    }

    Ret = MT_UNF_SND_Attach(hTrack, hAvplay);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_SND_Attach failed.\n");
        goto TRACK_DESTROY;
    }
    printf("MTADP_AVPlay_PlayProg ..........................0\n");
    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
        goto SND_DETACH;
    }

  
#if 0
    printf("select channel number to start play\n");
    SAMPLE_GET_INPUTCMD(InputCmd);
    ProgNum = atoi(InputCmd);
#endif
   printf("MTADP_AVPlay_PlayProg ..........................1\n");
   if (bVidPlay)
    {
         proginfo.VElementNum = 1;
         proginfo.VElementPid = VPid;
         proginfo.VideoType = MT_UNF_VCODEC_TYPE_MPEG2;
    }
    if (bAudPlay)
    {
         proginfo.AElementNum = 1;
         proginfo.AElementPid = APid;
         proginfo.AudioType = HA_AUDIO_ID_MP3;
    }

    pProgTbl.prog_num = 1;
    pProgTbl.proginfo = &proginfo;
    g_pProgTbl = &pProgTbl;
   
    ProgNum = 0;

    Ret = MTADP_AVPlay_PlayProg(hAvplay,g_pProgTbl,ProgNum,MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        printf("call SwitchProg failed.\n");
        goto AVPLAY_STOP;
    }


    printf("MTADP_AVPlay_PlayProg ..........................2\n");
    while(1)
    {
   
        //SAMPLE_GET_INPUTCMD(InputCmd);
	 printf("please input 'h' to get help or 'q' to quit!\n");
        printf("hdmi_cmd >");
        memset(InputCmd, 0, 128);
        SAMPLE_GET_INPUTCMD(InputCmd);
        if ('q' == InputCmd[0])
        {
            printf("prepare to quit!\n");
            break;
        }
        printf("MTADP_AVPlay_PlayProg ..........................3\n");
		//HDMI_Test_CMD(InputCmd);
    }
    printf("MTADP_AVPlay_PlayProg ..........................end\n");
AVPLAY_STOP:
    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;
    MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);

    g_pProgTbl = MT_NULL;
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
    return Ret;
}
*/

/*
mt_s32 DMX_DataRead(mt_handle MtChannel, mt_u32 u32TimeOutms, mt_u8 *pBuf, mt_u32* pAcquiredNum , mt_u32 * pBuffSize)
{
    mt_u8  u8tableid;
    MT_UNF_DMX_DATA_S sSection[32];
    mt_u32 num, i = 0;
    mt_u32 count = 0 ;
    mt_u32 u32Times = 0;
    mt_u32 u32SecTotalNum=0, u32SecNum = 0;
    mt_u32 RequestNum = 32;
    mt_u8 u8SecGotFlag[MAX_SECTION_NUM];

    u32Times = u32TimeOutms / 10;

    memset(u8SecGotFlag, 0, sizeof(u8SecGotFlag[MAX_SECTION_NUM]));

    while (--u32Times)
    {
        num = 0;
	 printf("u32Times = %d\n",u32Times);
        if ((MT_SUCCESS == MT_UNF_DMX_AcquireBuf(MtChannel, RequestNum, &num, sSection, u32TimeOutms)) && (num > 0))
        {
	     mt_u32 x=0;
            printf("num=======================%d\n",num);
            for (i = 0; i < num; i++)
            {
                printf("sSection[i].enDataType = %d\n",sSection[i].enDataType);
                if (sSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
                {
                    u8tableid = sSection[i].pu8Data[0];
                    if( ((EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH))
                        ||((EIT_TABLE_ID_SCHEDULE_OTHER_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_OTHER_HIGH)))
                    {
                        u32SecNum = sSection[i].pu8Data[6]>>3;
                        u32SecTotalNum = (sSection[i].pu8Data[7]>>3) +1;
			   printf("===========data:%d============\n",sSection[i].u32Size);
			   for(x=0;x<sSection[i].u32Size;x++)    printf("%x,",sSection[i].pu8Data[x]);
			   printf("\n");
                    }
                    else if ( TDT_TABLE_ID == u8tableid ||TOT_TABLE_ID == u8tableid)
                    {
                        u32SecNum = 0;
                        u32SecTotalNum = 1;
                    }
                    else
                    {
                        u32SecNum = sSection[i].pu8Data[6];
                        u32SecTotalNum = sSection[i].pu8Data[7] + 1;
                    }
		      printf("u32SecNum=%d, u32SecTotalNum=%d,u8tableid=%d\n",u32SecNum,u32SecTotalNum,u8tableid);
		      {
		      	   printf("===========i:%d, size:%d============\n",i,sSection[i].u32Size);
			   for(x=0;x<sSection[i].u32Size;x++)    printf("%x,",sSection[i].pu8Data[x]);
			   printf("\n");
		      	}
		      if(u8SecGotFlag[u32SecNum] == 0)
                    {
                        memcpy((void *)(pBuf + u32SecNum * MAX_SECTION_LEN), sSection[i].pu8Data, sSection[i].u32Size);
                        u8SecGotFlag[u32SecNum] = 1;
                        pBuffSize[  u32SecNum] = sSection[i].u32Size;
                        count++;
                    }
                }
            }
	     printf("count=%d\n",count);
            MT_UNF_DMX_ReleaseBuf(MtChannel, num, sSection);

            // to check if all sections are received
            if(u32SecTotalNum == count)
                break;
        }
        else
        {
            sample_common_printf("MT_UNF_DMX_AcquireBuf time out\n");
            return MT_FAILURE;
        }
    }

    if (u32Times == 0)
    {
        sample_common_printf("MT_UNF_DMX_AcquireBuf time out\n");
        return MT_FAILURE;
    }

    if(pAcquiredNum != MT_NULL)
        *pAcquiredNum = u32SecTotalNum;
    printf("download section u8tableid = 0x%x  success !!!\n",u8tableid);
	
    return MT_SUCCESS;
}

mt_s32 MT_DMX_SectionStartDataFilter(mt_u32 u32DmxId, DMX_DATA_FILTER_S * pstDataFilter)
{
    mt_u32 i;
    mt_u8 *p;
    MT_UNF_DMX_CHAN_ATTR_S stChanAttr;
    mt_handle hChan, MtFilter = MT_NULL;
    mt_s32 s32Ret;
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr;
    mt_u32 u32AquiredNum = 0;
    mt_u8 u8DataBuf[MAX_SECTION_LEN * MAX_SECTION_NUM];
    mt_u32 u32BufSize[MAX_SECTION_NUM];

    stChanAttr.u32BufSize = 16 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN_RETURN(MT_UNF_DMX_CreateChannel(u32DmxId, &stChanAttr, &hChan));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN_RETURN(MT_UNF_DMX_SetChannelPID(hChan, pstDataFilter->u32TSPID));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    stFilterAttr.u32FilterDepth = pstDataFilter->u16FilterDepth;
    memcpy(stFilterAttr.au8Match, pstDataFilter->u8Match, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Mask, pstDataFilter->u8Mask, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate, pstDataFilter->u8Negate, DMX_FILTER_MAX_DEPTH);

    MTAPI_RUN_RETURN(MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &MtFilter));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN_RETURN(MT_UNF_DMX_SetFilterAttr(MtFilter, &stFilterAttr));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN_RETURN(MT_UNF_DMX_AttachFilter(MtFilter, hChan));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN_RETURN(MT_UNF_DMX_OpenChannel(hChan));
    //printf("============%s  %d\n",__FILE__,__LINE__);

    memset(u8DataBuf, 0, sizeof(u8DataBuf));
    memset(u32BufSize, 0, sizeof(u32BufSize));
    //printf("============%s  %d\n",__FILE__,__LINE__);
    while(1)
    {
        s32Ret = DMX_DataRead(hChan, pstDataFilter->u32TimeOut, u8DataBuf, &u32AquiredNum , u32BufSize);
        //printf("============%s  %d\n",__FILE__,__LINE__);
        if (MT_SUCCESS == s32Ret)
        {
            //multi-SECTION parse
            p = u8DataBuf;
    	    //printf("============%s  u32AquiredNum=%d\n",__FILE__,u32AquiredNum);
            for(i=0;i<u32AquiredNum;i++)
            {
                pstDataFilter->funSectionFunCallback(p, u32BufSize[i], pstDataFilter->pSectionStruct);
                p = p+MAX_SECTION_LEN;
            }
        }
        sleep(1);
    }
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN(MT_UNF_DMX_CloseChannel(hChan),s32Ret);
    //printf("============%s  %d\n",__FILE__,__LINE__);
    MTAPI_RUN(MT_UNF_DMX_DetachFilter(MtFilter, hChan),s32Ret);
    //printf("============%s  %d\n",__FILE__,__LINE__);	
    MTAPI_RUN(MT_UNF_DMX_DestroyFilter(MtFilter),s32Ret);
    //printf("============%s  %d\n",__FILE__,__LINE__);	
    MTAPI_RUN(MT_UNF_DMX_DestroyChannel(hChan),s32Ret);
    printf("============%s  %d\n",__FILE__,__LINE__);
    return s32Ret;
}

static mt_s32 DVB_PATRequest(mt_u32 u32DmxID, PAT_TB *pat_tb)
{
    mt_s32 s32Ret;

    DMX_DATA_FILTER_S stDataFilter;

    //printf("\n ++++ PAT Request dmxid = %d \n",u32DmxID);

    memset(stDataFilter.u8Match, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Mask, 0xff, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));
    memset(stDataFilter.u8Negate, 0, DMX_FILTER_MAX_DEPTH * sizeof(mt_u8));

    stDataFilter.u32TSPID   = PAT_TSPID;
    stDataFilter.u32TimeOut = PGPAT_TIMEOUT;
    stDataFilter.u16FilterDepth = 1;
    stDataFilter.u8Crcflag = 0;

    stDataFilter.u8Match[0] = PAT_TABLE_ID;
    stDataFilter.u8Mask[0] = 0;

    // set call back func
    stDataFilter.funSectionFunCallback = &SRH_ParsePAT;
    stDataFilter.pSectionStruct = (mt_u8 *)pat_tb;
    printf("============%s  %d\n",__FILE__,__LINE__);
    // start data filter
    s32Ret = MT_DMX_SectionStartDataFilter(u32DmxID, &stDataFilter);
    if (s32Ret != MT_SUCCESS)
    {
        sample_common_printf("\n No PAT received \n");
    }
    printf("============%s  %d\n",__FILE__,__LINE__);
    return s32Ret;
}
mt_s32 MT_DVB_SearchStart(mt_u32 u32DmxID)
{
    PAT_TB pat_tb;
    PMT_TB pmt_tb;
    SDT_TB sdt_tb;

    mt_u32 i, j;
    mt_s32 s32Ret;

    memset(&pat_tb, 0, sizeof(pat_tb));
    memset(&pmt_tb, 0, sizeof(pmt_tb));
    memset(&sdt_tb, 0, sizeof(sdt_tb));

    s32Ret = DVB_PATRequest(u32DmxID, &pat_tb);
    if (s32Ret == MT_FAILURE)
    {
        return MT_FAILURE;
    }
    
    return 0;
}
*/
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
static mt_s32 SectionRecvFunc_handle0(void)
{
    mt_u32 dumpsz=0;
    mt_s32 gettimes=300;
    mt_u32 i,j,k;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_u32 u32HandleNum = 32;
    mt_handle u32ChHandle[32];
    mt_handle CheckHandle[128];
    mt_s32 ret;

    while(gettimes>0)
    {
        memset((void *)u32ChHandle, 0, sizeof(mt_handle) * u32HandleNum);
        MT_UNF_DMX_ResetCheckChanBuf(CheckHandle,128);
        u32HandleNum = 32;
        ret = MT_UNF_DMX_GetDataHandle((mt_handle *)u32ChHandle, (mt_u32 *)(&u32HandleNum), (mt_u32)10000);
        if ((MT_SUCCESS != ret) || (u32HandleNum == 0)){
            printf("MT_UNF_DMX_GetDataHandle failed\n\n");
            continue;
        }
        for(j=0; j<u32HandleNum; j++){
            if(MtChannel[1]==u32ChHandle[j]){   //only for chan:0
                continue;
            }
            if(MT_TRUE==MT_UNF_DMX_CheckChanHandle(CheckHandle,128,u32ChHandle[j])){    //dealed.re-select
                mt_handle phWatchChannel[1]={u32ChHandle[j]};
                mt_u32 u32WatchNum=1;
                mt_handle phDataChannel[1];
                mt_u32 pu32ChNum=1;
                ret=MT_UNF_DMX_SelectDataHandle(phWatchChannel,u32WatchNum,phDataChannel,&pu32ChNum,(mt_u32)0);
                if(MT_SUCCESS!=ret){
                    printf("reselect ,there is no data!\n");
                    continue;
                }
            }
            gettimes--;
            ret = MT_UNF_DMX_AcquireBuf(u32ChHandle[j],u32AcquireNum,(mt_u32 *)(&pu32AcquiredNum),pstBuf,(mt_u32)5000);
            if(MT_SUCCESS != ret) {
                printf("MT_UNF_DMX_AcquireBuf failed!\n");
                MT_USLEEP(10 * 1000);
                continue;
            }   

            for(i=0;i<pu32AcquiredNum;i++)
            {
                if(pstBuf[i].u32Size>8){
                    dumpsz=8;
                }else{
                    dumpsz=pstBuf[i].u32Size;
                }
                printf("handlea:%x,filtid:%x,size=%x,data8:",u32ChHandle[j],pstBuf[i].filthandle,pstBuf[i].u32Size);
                dmx_data_dump(pstBuf[i].pu8Data,dumpsz);

                k = pstBuf[i].u32Size;
                crcSoftware = CRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
                crcStream=pstBuf[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    printf("err.CRC,don't care TDT!!!\n");
                }
            }
            ret = MT_UNF_DMX_ReleaseBuf(u32ChHandle[j],pu32AcquiredNum,pstBuf);
            if (MT_SUCCESS != ret){
                printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
            }
        }
    }
    printf("receive over0!\n");
    return MT_SUCCESS;
}

static mt_s32 SectionRecvFunc_handle1_select(void)
{
    mt_u32 dumpsz=0;
    mt_s32 gettimes=300;
    mt_u32 i,j,k;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_s32 ret;
    mt_handle phWatchChannel[2]={MtChannel[0],MtChannel[1]};
    mt_u32 u32WatchNum=2;
    mt_handle phDataChannel[8];
    mt_u32 pu32ChNum=8;

    while(gettimes>0)
    {
        pu32ChNum=8;
        phWatchChannel[0]=MtChannel[0];
        phWatchChannel[1]=MtChannel[1];
        ret=MT_UNF_DMX_SelectDataHandle(phWatchChannel,u32WatchNum,phDataChannel,&pu32ChNum,(mt_u32)10000);
        if ((MT_SUCCESS != ret) || (pu32ChNum == 0)){
            printf("MT_UNF_DMX_SelectDataHandle failed\n\n");
            continue;
        }
        for(j=0; j<pu32ChNum; j++){
            if(MtChannel[1]!=phDataChannel[j]){   //only for channel:1
                continue;
            }
            gettimes--;
            ret = MT_UNF_DMX_AcquireBuf(phDataChannel[j],u32AcquireNum,(mt_u32 *)(&pu32AcquiredNum),pstBuf,(mt_u32)5000);
            if(MT_SUCCESS != ret) {
                printf("MT_UNF_DMX_AcquireBuf failed!\n");
                MT_USLEEP(10 * 1000);
                continue;
            }   

            for(i=0;i<pu32AcquiredNum;i++)
            {
                if(pstBuf[i].u32Size>8){
                    dumpsz=8;
                }else{
                    dumpsz=pstBuf[i].u32Size;
                }
                printf("handleb:%x,filtid:%x,size=%x,data8:",phDataChannel[j],pstBuf[i].filthandle,pstBuf[i].u32Size);
                dmx_data_dump(pstBuf[i].pu8Data,dumpsz);

                k = pstBuf[i].u32Size;
                crcSoftware = CRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
                crcStream=pstBuf[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    printf("err.CRC,don't care TDT!!!\n");
                }
            }
            ret = MT_UNF_DMX_ReleaseBuf(phDataChannel[j],pu32AcquiredNum,pstBuf);
            if (MT_SUCCESS != ret){
                printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
            }
        }
    }
    printf("receive over1!\n");
    return MT_SUCCESS;
}
static mt_s32 SectionRecvFunc_handle1(void)
{
    mt_u32 dumpsz=0;
    mt_s32 gettimes=300;
    mt_u32 i,j,k;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_u32 u32HandleNum = 32;
    mt_handle u32ChHandle[32];
    mt_handle CheckHandle[128];
    mt_u32 checknum=0;
    mt_s32 ret;

    while(gettimes>0)
    {
        memset((void *)u32ChHandle, 0, sizeof(mt_handle) * u32HandleNum);
        MT_UNF_DMX_ResetCheckChanBuf(CheckHandle,128);
        u32HandleNum = 32;
        ret = MT_UNF_DMX_GetDataHandle((mt_handle *)u32ChHandle, (mt_u32 *)(&u32HandleNum), (mt_u32)10000);
        if ((MT_SUCCESS != ret) || (u32HandleNum == 0)){
            printf("MT_UNF_DMX_GetDataHandle failed\n\n");
            continue;
        }
        for(j=0; j<u32HandleNum; j++){
            if(MtChannel[1]!=u32ChHandle[j]){   //only for channel:1
                continue;
            }
            if(MT_TRUE==MT_UNF_DMX_CheckChanHandle(CheckHandle,128,u32ChHandle[j])){    //dealed.re-select
                mt_handle phWatchChannel[1]={u32ChHandle[j]};
                mt_u32 u32WatchNum=1;
                mt_handle phDataChannel[1];
                mt_u32 pu32ChNum=1;
                ret=MT_UNF_DMX_SelectDataHandle(phWatchChannel,u32WatchNum,phDataChannel,&pu32ChNum,(mt_u32)0);
                if(MT_SUCCESS!=ret){
                    printf("reselect ,there is no data!\n");
                    continue;
                }
            }
            gettimes--;
            ret = MT_UNF_DMX_AcquireBuf(u32ChHandle[j],u32AcquireNum,(mt_u32 *)(&pu32AcquiredNum),pstBuf,(mt_u32)5000);
            if(MT_SUCCESS != ret) {
                printf("MT_UNF_DMX_AcquireBuf failed!\n");
                MT_USLEEP(10 * 1000);
                continue;
            }   

            for(i=0;i<pu32AcquiredNum;i++)
            {
                if(pstBuf[i].u32Size>8){
                    dumpsz=8;
                }else{
                    dumpsz=pstBuf[i].u32Size;
                }
                printf("handletc:%x,filtid:%x,size=%x,data8:",u32ChHandle[j],pstBuf[i].filthandle,pstBuf[i].u32Size);
                dmx_data_dump(pstBuf[i].pu8Data,dumpsz);

                k = pstBuf[i].u32Size;
                crcSoftware = CRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
                crcStream=pstBuf[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    printf("err.CRC,don't care TDT!!!\n");
                }
            }
            ret = MT_UNF_DMX_ReleaseBuf(u32ChHandle[j],pu32AcquiredNum,pstBuf);
            if (MT_SUCCESS != ret){
                printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
            }
        }
    }
    printf("receive over1!\n");
    return MT_SUCCESS;
}

mt_s32 main(mt_s32 argc,mt_char *argv[])
{//./sample_sectionRecv 538 6875 64
    pthread_t pthread_id0;
    pthread_t pthread_id1;
    mt_s32                      repeat=0;
    mt_s32                      Ret;
    MT_UNF_DMX_CHAN_ATTR_S      stChnAttr;
    MT_UNF_DMX_FILTER_ATTR_S    stFilterAttr;
    mt_s32 i=0,j=0;
    mt_u32 freq, symbol_rate, qam_mode;
    mt_sys_version_s stSysChipInfo;

    memset(MtChannel,0x00,sizeof(MtChannel));
    memset(MtFilter,0x00,sizeof(MtFilter));
    mt_sys_init();
    MTADP_MCE_Exit();
    Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_Init failed.\n");
        return -1;
    }

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

#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);
    if (0<qam_mode){                                                //dvbc
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.C
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }else{                                                      //sym1.C
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }
    }else{
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }else{                                                      //sym1.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }
    }
#endif  
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return -1;
    }

    for(i=0; i<MAX_CHANNL_NUM; i++)
    {
        printf("create channel:%x,pid=%x\n",i,section_pid[i]);
        stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
        stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD;
        stChnAttr.u32BufSize = BUFFER_SIZE;
        stChnAttr.enOutputMode=MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
        if(TDT_TSPID == section_pid[i]){        //tdt forbid crc!
            stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
        }
        Ret = MT_UNF_DMX_CreateChannel(DMX_ID,&stChnAttr,&MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_CreateChannel failed!\n");
            goto DMX_DEINIT;
        }
        Ret = MT_UNF_DMX_SetChannelPID(MtChannel[i],section_pid[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_SetChannelPID failed!\n");
            goto DMX_DEINIT;
        }
        /* set filter attr */
        memset(&g_mask,0xff,DMX_FILTER_MAX_DEPTH);
        memset(&g_match,0,DMX_FILTER_MAX_DEPTH);
        memset(&g_Negate,0,DMX_FILTER_MAX_DEPTH);
        stFilterAttr.u32FilterDepth = DMX_FILTER_MAX_DEPTH;

        if((PAT_TSPID== section_pid[i]) ||(TDT_TSPID == section_pid[i])){
            if(PAT_TSPID== section_pid[i]){   
                g_match[0] =  PAT_TABLEID;
                g_mask[0] = 0;
            }else if(20 == section_pid[i]){
                g_match[0] = TDT_TABLEID;
                g_mask[0] = 0;
            }
            memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
            Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][0]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CreateFilter failed!\n");
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][0], &stFilterAttr);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][0],MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
        }else if(EIT_TSPID == section_pid[i]){ //eit for multi-filter
            if(0==repeat){
                g_match[0] = EIT_TABLEID0;
                g_mask[0] = 0;
                memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
                Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][0]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_CreateFilter failed!\n");
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][0], &stFilterAttr);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][0],MtChannel[i]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                    goto CHN_DESTROY;
                }
                
                g_match[0] = EIT_TABLEID1;
                g_mask[0] = 0;
                memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
                Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][1]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_CreateFilter failed!\n");
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][1], &stFilterAttr);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][1]);
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][1],MtChannel[i]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][1]);
                    goto CHN_DESTROY;
                }
            }else{
                g_match[0] = EIT_TABLEID2;
                g_mask[0] = 0x01;
                memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
                memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
                Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][0]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_CreateFilter failed!\n");
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][0], &stFilterAttr);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                    goto CHN_DESTROY;
                }
                Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][0],MtChannel[i]);
                if (MT_SUCCESS != Ret){
                    printf("call MT_UNF_DMX_AttachFilter failed!\n");
                    MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                    goto CHN_DESTROY;
                }
            }
            repeat++;
        }
        Ret |= MT_UNF_DMX_OpenChannel(MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_OpenChannel failed!\n");
            goto FILTER1_DESTROY;
        }
    }

    /* start to recv section data, and meanwhile print received section */
    pthread_create(&pthread_id0, NULL, (void *)SectionRecvFunc_handle0, NULL);
    pthread_create(&pthread_id1, NULL, (void *)SectionRecvFunc_handle1_select, NULL);     //select
    //pthread_create(&pthread_id1, NULL, (void *)SectionRecvFunc_handle1, NULL);              //blind
    pthread_join(pthread_id0,NULL);
    pthread_join(pthread_id1,NULL);
    for(i=0; i<MAX_CHANNL_NUM; i++){
        Ret = MT_UNF_DMX_CloseChannel(MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_CloseChannel failed!\n");
        }
    }
    FILTER1_DESTROY:
        for(i=0; i<MAX_CHANNL_NUM; i++)
        for(j=0; j<MAX_FILTER_NUM; j++){
            if(0!=MtFilter[i][j]){
                MT_UNF_DMX_DetachFilter(MtFilter[i][j],MtChannel[i]);
                MT_UNF_DMX_DestroyFilter(MtFilter[i][j]);
            }
        }
    CHN_DESTROY:
        for(i=0; i<MAX_CHANNL_NUM; i++){
            Ret = MT_UNF_DMX_DestroyChannel(MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CloseChannel failed!\n");
            }
        }
    DMX_DEINIT:
        MT_UNF_DMX_DetachTSPort(DMX_ID);
        MT_UNF_DMX_DeInit();
    mtadp_fe_deinit();
    mt_sys_deinit();
    return Ret;
}
#else
static mt_s32 SectionRecvFunc(void)
{
    mt_u32 dumpsz=0,gettimes=300;
    mt_u32 i,j,k;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 crcSoftware = 0;
    mt_u32 crcStream = 0;
    mt_u32 u32HandleNum = 32;
    mt_handle u32ChHandle[32];
    mt_s32 ret;

    while(gettimes--)
    {
        memset((void *)u32ChHandle, 0, sizeof(mt_handle) * u32HandleNum);
        u32HandleNum = 32;
        ret = MT_UNF_DMX_GetDataHandle((mt_handle *)u32ChHandle, (mt_u32 *)(&u32HandleNum), (mt_u32)10000);
        if ((MT_SUCCESS != ret) || (u32HandleNum == 0)){
            printf("MT_UNF_DMX_GetDataHandle failed\n\n");
            continue;
        }
        for(j=0; j<u32HandleNum; j++){
            ret = MT_UNF_DMX_AcquireBuf(u32ChHandle[j],u32AcquireNum,(mt_u32 *)(&pu32AcquiredNum),pstBuf,(mt_u32)5000);
            if(MT_SUCCESS != ret) {
                printf("MT_UNF_DMX_AcquireBuf failed!\n");
                MT_USLEEP(10 * 1000);
                continue;
            }   

            for(i=0;i<pu32AcquiredNum;i++)
            {
                if(pstBuf[i].u32Size>16){
                    dumpsz=16;
                }else{
                    dumpsz=pstBuf[i].u32Size;
                }
                printf("handle:%x,filtid:%x,size=%x,data16:\n",u32ChHandle[j],pstBuf[i].filthandle,pstBuf[i].u32Size);
                dmx_data_dump(pstBuf[i].pu8Data,dumpsz);

                k = pstBuf[i].u32Size;
                crcSoftware = CRC32(&(pstBuf[i].pu8Data[0]), pstBuf[i].u32Size - 4);
                crcStream=pstBuf[i].pu8Data[k-4];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-3];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-2];
                crcStream=(crcStream<<8)|pstBuf[i].pu8Data[k-1];
                if(crcSoftware != crcStream){
                    printf("err.CRC,don't care TDT!!!\n");
                }
            }
            ret = MT_UNF_DMX_ReleaseBuf(u32ChHandle[j],pu32AcquiredNum,pstBuf);
            if (MT_SUCCESS != ret){
                printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
            }
        }
    }
    printf("receive over!\n");
    return MT_SUCCESS;
}

mt_s32 main(mt_s32 argc,mt_char *argv[])
{//./sample_sectionRecv 538 6875 64
    mt_s32                      repeat=0;
    mt_s32                      Ret;
    MT_UNF_DMX_CHAN_ATTR_S      stChnAttr;
    MT_UNF_DMX_FILTER_ATTR_S    stFilterAttr;
    mt_s32 i=0,j=0;
    mt_u32 freq, symbol_rate, qam_mode;
    mt_sys_version_s stSysChipInfo;

    memset(MtChannel,0x00,sizeof(MtChannel));
    memset(MtFilter,0x00,sizeof(MtFilter));
    mt_sys_init();
    MTADP_MCE_Exit();
    Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_Init failed.\n");
        return -1;
    }

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

#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);
    if (0<qam_mode){                                                //dvbc
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.C
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }else{                                                      //sym1.C
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }
    }else{
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }else{                                                      //sym1.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }
    }
#endif  
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return -1;
    }

    for(i=0; i<MAX_CHANNL_NUM; i++)
    {
        printf("create channel:%x,pid=%x\n",i,section_pid[i]);
        stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
        stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD;
        stChnAttr.u32BufSize = BUFFER_SIZE;
        stChnAttr.enOutputMode=MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
        if(TDT_TSPID == section_pid[i]){        //tdt forbid crc!
            stChnAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
        }
        Ret = MT_UNF_DMX_CreateChannel(DMX_ID,&stChnAttr,&MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_CreateChannel failed!\n");
            goto DMX_DEINIT;
        }
        Ret = MT_UNF_DMX_SetChannelPID(MtChannel[i],section_pid[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_SetChannelPID failed!\n");
            goto DMX_DEINIT;
        }
        /* set filter attr */
        memset(&g_mask,0xff,DMX_FILTER_MAX_DEPTH);
        memset(&g_match,0,DMX_FILTER_MAX_DEPTH);
        memset(&g_Negate,0,DMX_FILTER_MAX_DEPTH);
        stFilterAttr.u32FilterDepth = DMX_FILTER_MAX_DEPTH;

        if((PAT_TSPID== section_pid[i]) ||(TDT_TSPID == section_pid[i])){
            if(PAT_TSPID== section_pid[i]){   
                g_match[0] =  PAT_TABLEID;
                g_mask[0] = 0;
            }else if(20 == section_pid[i]){
                g_match[0] = TDT_TABLEID;
                g_mask[0] = 0;
            }
            memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
            Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][0]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CreateFilter failed!\n");
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][0], &stFilterAttr);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][0],MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
        }else if(EIT_TSPID == section_pid[i]){ //eit for multi-filter
            g_match[0] = EIT_TABLEID0;
            g_mask[0] = 0;
            memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
            Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][0]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CreateFilter failed!\n");
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][0], &stFilterAttr);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][0],MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][0]);
                goto CHN_DESTROY;
            }
            
            g_match[0] = EIT_TABLEID1;
            g_mask[0] = 0;
            memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
            Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][1]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CreateFilter failed!\n");
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][1], &stFilterAttr);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][1]);
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][1],MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][1]);
                goto CHN_DESTROY;
            }
            
            g_match[0] = EIT_TABLEID2;
            g_mask[0] = 0x00;
            memcpy(stFilterAttr.au8Mask,g_mask,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Match,g_match,DMX_FILTER_MAX_DEPTH);
            memcpy(stFilterAttr.au8Negate,g_Negate,DMX_FILTER_MAX_DEPTH);
            Ret = MT_UNF_DMX_CreateFilter(DMX_ID,&stFilterAttr,&MtFilter[i][2]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CreateFilter failed!\n");
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_SetFilterAttr(MtFilter[i][2], &stFilterAttr);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][2]);
                goto CHN_DESTROY;
            }
            Ret = MT_UNF_DMX_AttachFilter(MtFilter[i][2],MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_AttachFilter failed!\n");
                MT_UNF_DMX_DestroyFilter(MtFilter[i][2]);
                goto CHN_DESTROY;
            }
        }
        Ret |= MT_UNF_DMX_OpenChannel(MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_OpenChannel failed!\n");
            goto FILTER1_DESTROY;
        }
    }

    /* start to recv section data, and meanwhile print received section */
    SectionRecvFunc();
    for(i=0; i<MAX_CHANNL_NUM; i++){
        Ret = MT_UNF_DMX_CloseChannel(MtChannel[i]);
        if (MT_SUCCESS != Ret){
            printf("call MT_UNF_DMX_CloseChannel failed!\n");
        }
    }
    FILTER1_DESTROY:
        for(i=0; i<MAX_CHANNL_NUM; i++)
        for(j=0; j<MAX_FILTER_NUM; j++){
            if(0!=MtFilter[i][j]){
                MT_UNF_DMX_DetachFilter(MtFilter[i][j],MtChannel[i]);
                MT_UNF_DMX_DestroyFilter(MtFilter[i][j]);
            }
        }
    CHN_DESTROY:
        for(i=0; i<MAX_CHANNL_NUM; i++){
            Ret = MT_UNF_DMX_DestroyChannel(MtChannel[i]);
            if (MT_SUCCESS != Ret){
                printf("call MT_UNF_DMX_CloseChannel failed!\n");
            }
        }
    DMX_DEINIT:
        MT_UNF_DMX_DetachTSPort(DMX_ID);
        MT_UNF_DMX_DeInit();
    mtadp_fe_deinit();
    mt_sys_deinit();
    return Ret;
}

#endif
