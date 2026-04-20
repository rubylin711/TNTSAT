/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
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
#include "mt_unf_venc.h"
#include "../common/mt_adp_audio.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"

#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"

FILE *g_pTsFile = MT_NULL;
static pthread_t g_TsThd;
static pthread_t g_SaveThd;
static pthread_t g_SaveEsThd;
static pthread_mutex_t g_TsMutex;
static volatile MT_BOOL g_bStopTsThread = MT_FALSE;
static mt_handle hAvplay = 0;
mt_handle g_TsBuf = 0;
static PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;
static mt_u8 ts_file_name[256] = { 0 };

static MT_BOOL g_bStopSaveThread = MT_TRUE;
static MT_BOOL g_bSaveStream = MT_FALSE;

static mt_handle g_hWin;

#define PLAY_DMX_ID 0
#define TSPLAY_DEBUG 0

mt_void *TsTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 Readlen;
    mt_s32 Ret;
    mt_u32 count = 0;

    printf("%s: Enter\n", __FUNCTION__);
    while (!g_bStopTsThread) {
	pthread_mutex_lock(&g_TsMutex);
	//Ret = MT_UNF_DMX_GetTSBuffer(g_TsBuf, 188*1024, &StreamBuf, 1000);
	Ret = MT_UNF_DMX_GetTSBuffer(g_TsBuf, 188 * 512, &StreamBuf, 1000);
	if (Ret != MT_SUCCESS) {
	    pthread_mutex_unlock(&g_TsMutex);
	    MT_USLEEP(1000);
	    continue;
	}
	//printf("fread ..........%x...........%x.....\n", g_pTsFile, StreamBuf.pu8Data);
	//memset(StreamBuf.pu8Data, 0, 188*1024);
	memset(StreamBuf.pu8Data, 0, 188 * 512);
#if TSPLAY_DEBUG
	count++;
	if (count > 512)
	    count = 1;
	Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), 188 * count, g_pTsFile);
#else
	Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), 188 * 512, g_pTsFile);
//printf("fread .......................Readlen=%d\n",Readlen);
#endif
	if (Readlen <= 0) {
	    printf("read ts file end and rewind! Readlen=%d\n", Readlen);
	    //rewind(g_pTsFile);
	    fclose(g_pTsFile);
	    MT_USLEEP(1000);
	    g_pTsFile = fopen(ts_file_name, "rb");
	    pthread_mutex_unlock(&g_TsMutex);
	    MT_USLEEP(1000);
	    continue;
	}
	//printf("fread .......................Readlen=%d\n",Readlen);
	Ret = MT_UNF_DMX_PutTSBuffer(g_TsBuf, Readlen);
	if (Ret != MT_SUCCESS) {
	    printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
	}
	//printf("putTsBuff.......................Readlen=%d\n",Readlen);
	pthread_mutex_unlock(&g_TsMutex);

	MT_USLEEP(1000);
    }

    Ret = MT_UNF_DMX_ResetTSBuffer(g_TsBuf);
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_DMX_ResetTSBuffer failed.\n");
    }

    printf("%s: Leave ================\n", __FUNCTION__);

    return NULL;
}

typedef struct
{
    mt_handle hVenc;
    mt_handle hAenc;
} THREAD_ARGS_S;

#define VID_FILENAME_PATH "./vir_venc.h264"

mt_void *SaveThread(mt_void *pArgs)
{
    THREAD_ARGS_S *pThreadArgs = (THREAD_ARGS_S *)pArgs;
    mt_handle hVencChn, hAencChn;
    FILE *pVidFile, *pAudFile;
    mt_s32 Ret;

    hVencChn = pThreadArgs->hVenc;
    hAencChn = pThreadArgs->hAenc;
    pVidFile = fopen(VID_FILENAME_PATH, "w");
    if (MT_NULL == pVidFile) {
	printf("open file %s failed\n", VID_FILENAME_PATH);
	return MT_NULL;
    }
/*
    pAudFile = fopen(AUD_FILENAME_PATH, "w");
    if (MT_NULL == pVidFile) {
	printf("open file %s failed\n", AUD_FILENAME_PATH);
	fclose(pVidFile);
	return MT_NULL;
    }
*/
#if defined(CONFIG_MT_CHIP_ARIA)
    while (!g_bStopSaveThread) {
	MT_BOOL bGotStream = MT_FALSE;

	if (g_bSaveStream) {
	    MT_UNF_VENC_STREAM_S stVencStream;
	    MT_UNF_ES_BUF_S stAencStream;
#ifdef FILE_LEN_LIMIT
	    if (ftell(pVidFile) >= VID_FILE_MAX_LEN) {
		fclose(pVidFile);
		//fclose(pAudFile);
		pVidFile = fopen(VID_FILENAME_PATH, "w");
		//pAudFile = fopen(AUD_FILENAME_PATH, "w");
		printf("stream files are truncated to zero\n");
	    }
#endif

	    /*save video stream*/
	    Ret = MT_UNF_VENC_AcquireStream(hVencChn, &stVencStream, 0);
	    if (MT_SUCCESS == Ret) {
		printf("APP dvbcast write 0x%x byte to file, addr: 0x%x\n",
		       stVencStream.u32SlcLen, stVencStream.pu8Addr);
		fwrite(stVencStream.pu8Addr, 1, stVencStream.u32SlcLen, pVidFile);
		MT_UNF_VENC_ReleaseStream(hVencChn, &stVencStream);
		fflush(pVidFile);
		bGotStream = MT_TRUE;
	    } else if (MT_ERR_VENC_BUF_EMPTY != Ret) {
		printf("MT_UNF_VENC_AcquireStream failed:%#x\n", Ret);
	    }
	    /*
	    //save audio stream
	    Ret = MT_UNF_AENC_AcquireStream(hAencChn, &stAencStream, 0);
	    if (MT_SUCCESS == Ret) {
		fwrite(stAencStream.pu8Buf, 1, stAencStream.u32BufLen, pAudFile);
		fflush(pAudFile);
		MT_UNF_AENC_ReleaseStream(hAencChn, &stAencStream);
		bGotStream = MT_TRUE;
	    } else if (MT_ERR_AENC_OUT_BUF_EMPTY != Ret) {
		printf("MT_UNF_AENC_AcquireStream failed:%#x\n", Ret);
	    }
*/
	}

	if (MT_FALSE == bGotStream) {
	    MT_USLEEP(10 * 1000);
	}
    }
#endif
    fclose(pVidFile);
    //fclose(pAudFile);
    return MT_NULL;
}

extern mt_u32 HDMI_Test_CMD(mt_char *u8String);

mt_void *SaveEsBuffDataTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 Readlen;
    mt_s32 Ret;
    ulong es_buffer = 0;
    ulong es_size = 0;
    FILE *RecFile = MT_NULL;
    mt_u32 wp = 0;
    mt_u32 rp = 0;
    mt_u32 len = 0;

    MT_MPI_DMX_GetEsBuffAddr(hAvplay, &es_buffer, &es_size, NULL, NULL, NULL);
    RecFile = fopen("/mnt/hdmi_esdata.ts", "wb+");
    if (!RecFile) {
	perror("fopen error");
        return NULL;
    }

    while (!g_bStopTsThread) {
	wp = MT_MPI_DMX_GetCurrentEsBufferWritePoint(hAvplay);
	//printf("wp = 0x%x, es_buff_addr = 0x%x, es_buff_size=0x%x\n",wp,es_buffer,es_size);
	if (wp == rp) {
	    MT_USLEEP(1000);
	    continue;
	} else if (wp > rp) {
	    len = wp - rp;
	    Ret = fwrite((void *)(es_buffer + rp), 1, len, RecFile);
	    //printf("1 Ret = %x\n",Ret);
	    if (Ret != len) {
		printf("Ret=%x\n", Ret);
		perror("[SaveRecData] fwrite error");
		break;
	    }

	    rp = wp;
	} else {
	    len = es_size - rp;
	    Ret = fwrite((void *)(es_buffer + rp), 1, len, RecFile);
	    //printf("2 Ret = %x\n",Ret);
	    if (Ret != len) {
		printf("Ret=%x\n", Ret);
		perror("[SaveRecData] fwrite error");
		break;
	    }
	    len = wp;
	    Ret = fwrite((void *)(es_buffer + 0), 1, len, RecFile);
	    //printf("3 Ret = %x\n",Ret);
	    if (Ret != len) {
		printf("Ret=%x\n", Ret);
		perror("[SaveRecData] fwrite error");
		break;
	    }

	    rp = wp;
	}
	MT_USLEEP(1000);
    }
    fclose(RecFile);
    printf("Save file  .......................ok\n");

    return NULL;
}

typedef struct
{
    mt_handle RecHandle;
    mt_char FileName[256];
    MT_BOOL ThreadRunFlag;
} TsFileInfo;

mt_void *SaveRecDataThread(mt_void *arg)
{
    mt_s32 ret;
    TsFileInfo *Ts = (TsFileInfo *)arg;
    FILE *RecFile = MT_NULL;
    mt_s32 len = 0;
    printf("[%s] open file-------------------- %s\n", __FUNCTION__, Ts->FileName);
    RecFile = fopen(Ts->FileName, "wb");
    if (!RecFile) {
	perror("fopen error");

	Ts->ThreadRunFlag = MT_FALSE;

	return MT_NULL;
    }

    printf("[%s] oTs->ThreadRunFlag %d\n", __FUNCTION__, Ts->ThreadRunFlag);

    while (Ts->ThreadRunFlag) {
#ifndef USE_ORIGINAL_SCD
	MT_UNF_DMX_REC_DATA_S RecData;

	ret = MT_UNF_DMX_AcquireRecData(Ts->RecHandle, &RecData, 100);
#else
	DMX_DATA_S RecData;
	ret = MT_MPI_DMX_AcquireRecScdBuf(DEMUX_ID, &RecData, 1000);
#endif
	if (MT_SUCCESS != ret) {
	    if (MT_ERR_DMX_TIMEOUT == ret) {
		continue;
	    }

	    if (MT_ERR_DMX_NOAVAILABLE_DATA == ret) {
		continue;
	    }

	    printf("[%s] MT_UNF_DMX_AcquireRecData failed 0x%x\n", __FUNCTION__, ret);

	    break;
	}
	len = RecData.u32Len;
	if (len > 0)
	    printf("SaveRecDataThread >>>>>  RecData.pDataAddr=%x, RecData.u32Len=%x\n", RecData.pDataAddr, len);
	ret = fwrite(RecData.pDataAddr, 1, RecData.u32Len, RecFile);
	if (ret != len) {
	    printf("ret=%x\n", ret);
	    perror("[SaveRecDataThread] fwrite error");
	    break;
	}
#ifndef USE_ORIGINAL_SCD
	ret = MT_UNF_DMX_ReleaseRecData(Ts->RecHandle, &RecData);
#else
	ret = MT_MPI_DMX_ReleaseRecScdBuf(DEMUX_ID, &RecData);
#endif
	if (MT_SUCCESS != ret) {
	    printf("[%s] MT_UNF_DMX_ReleaseRecData failed 0x%x\n", __FUNCTION__, ret);

	    break;
	}
    }

    fclose(RecFile);
    printf("fclose >>>>>  \n");
    Ts->ThreadRunFlag = MT_FALSE;

    return MT_NULL;
}

#define INVALID_PID 0x1FFF
mt_s32 DmxStartRecord(mt_char *Path, PMT_COMPACT_PROG *ProgInfo)
{
    mt_s32 ret = 0;
    MT_UNF_DMX_REC_ATTR_S RecAttr;
    mt_handle RecHandle;
    mt_handle ChanHandle[8];
    mt_u32 ChanCount = 0;
    MT_BOOL RecordStatus = MT_FALSE;
    pthread_t RecThreadId = -1;
    pthread_t IndexThreadId = -1;
    TsFileInfo TsRecInfo;
    TsFileInfo TsIndexInfo;
    mt_char FileName[256];
    mt_u32 i;

    ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_RAM_0);
    RecAttr.u32DmxId = 0;
    RecAttr.u32RecBufSize = 4 * 1024 * 1024;
    RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_SELECT_PID;
    RecAttr.bDescramed = MT_TRUE;

    if (ProgInfo->VElementPid < INVALID_PID) {
	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_VIDEO;
	RecAttr.u32IndexSrcPid = ProgInfo->VElementPid;
	RecAttr.enVCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
    } else if (ProgInfo->AElementPid < INVALID_PID) {
	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_AUDIO;
	RecAttr.u32IndexSrcPid = ProgInfo->AElementPid;
    } else {
	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    }
    printf("DmxStartRecord >>>>  ProgInfo->VElementPid=0x%x,ProgInfo->AElementPid=0x%x\n ", ProgInfo->VElementPid, ProgInfo->AElementPid);
    ret = MT_UNF_DMX_CreateRecChn(&RecAttr, &RecHandle);
    if (MT_SUCCESS != ret) {
	printf("[%s - %u] MT_UNF_DMX_CreateRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);

	return ret;
	;
    }

    sprintf(FileName, "%s/rec_20180102", Path);

    if (ProgInfo->VElementPid < INVALID_PID) {
	ret = MT_UNF_DMX_AddRecPid(RecHandle, ProgInfo->VElementPid, &ChanHandle[ChanCount]);
	if (MT_SUCCESS != ret) {
	    printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);

	    goto exit;
	}

	sprintf(FileName, "%s_v%u", FileName, ProgInfo->VElementPid);

	++ChanCount;
    }

    ret = MT_UNF_DMX_StartRecChn(RecHandle);
    if (MT_SUCCESS != ret) {
	printf("[%s - %u] MT_UNF_DMX_StartRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);

	goto exit;
    }

    RecordStatus = MT_TRUE;

    TsRecInfo.RecHandle = RecHandle;
    TsRecInfo.ThreadRunFlag = MT_TRUE;
    sprintf(TsRecInfo.FileName, "%s.ts", FileName);

    ret = pthread_create(&RecThreadId, MT_NULL, SaveRecDataThread, (mt_void *)&TsRecInfo);
    if (0 != ret) {
	perror("[DmxStartRecord] pthread_create record error");

	goto exit;
    }

    TsIndexInfo.RecHandle = RecHandle;
    TsIndexInfo.ThreadRunFlag = MT_TRUE;
    sprintf(TsIndexInfo.FileName, "%s.ts.idx", FileName);
    /*
    ret = pthread_create(&IndexThreadId, MT_NULL, SaveIndexDataThread, (mt_void*)&TsIndexInfo);
    if (0 != ret)
    {
        perror("[DmxStartRecord] pthread_create index error");

        goto exit;
    }

    sleep(1);
    */
    while (1) {
	mt_char InputCmd[256] = { 0 };

	printf("please input the qqqqqqqqqqqqqqqqqqqqqqqqqqq to quit !\n");

	SAMPLE_GET_INPUTCMD(InputCmd);
	if ('q' == InputCmd[0]) {
	    break;
	}
    }

exit:
    if (-1 != RecThreadId) {
	TsRecInfo.ThreadRunFlag = MT_FALSE;
	pthread_join(RecThreadId, MT_NULL);
    }

    if (-1 != IndexThreadId) {
	TsIndexInfo.ThreadRunFlag = MT_FALSE;
	pthread_join(IndexThreadId, MT_NULL);
    }
    printf("======================== stop rec =================================1\n");
    if (RecordStatus) {
	ret = MT_UNF_DMX_StopRecChn(RecHandle);
	if (MT_SUCCESS != ret) {
	    printf("[%s - %u] MT_UNF_DMX_StopRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
    }
    printf("======================== stop rec =================================ChanCount=%d\n", ChanCount);
    for (i = 0; i < ChanCount; i++) {
	ret = MT_UNF_DMX_DelRecPid(RecHandle, ChanHandle[i]);
	if (MT_SUCCESS != ret) {
	    printf("[%s - %u] MT_UNF_DMX_DelRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
    }
    printf("======================== stop rec =================================3\n");
    ret = MT_UNF_DMX_DestroyRecChn(RecHandle);
    if (MT_SUCCESS != ret) {
	printf("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    }

    return ret;
}

mt_s32 main(mt_s32 argc, mt_char *argv[])
{
    mt_s32 Ret;
    mt_u32 ProgNum;
    mt_char InputCmd[256];


    if (6 <= argc) {

    } else {
	printf("\n\n");
	printf("Usage: tsplay file vtype vpid venc[vo_format] atype apid [avsync]\n\n"
	       "       vtype    :mpeg2 | mpeg4 | h264 | h265\n"
	       "       vo_format:2160P_30|2160P_24|1080P_60|1080P_50|1080i_60|[1080i_50]|720P_60|720P_50\n"
	       "                 |480p_60|576P_50|PAL|NTSC \n\n");
	printf("Example:./sample_hdmi_tsplay ./test.ts mpeg2 201\n");
	printf("\n\n");
	return 0;
    }
    memset(ts_file_name, 0, 256);
    strcpy(ts_file_name, argv[1]);
    g_pTsFile = fopen(argv[1], "rb");
    if (!g_pTsFile) {
	printf("open file %s error!\n", argv[1]);
	return -1;
    }

    mt_sys_init();


    Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_DMX_Init failed.\n");
	return 0;
    }

    Ret = MT_UNF_DMX_AttachTSPort(PLAY_DMX_ID, MT_UNF_DMX_PORT_RAM_0);
    if (MT_SUCCESS != Ret) {
	printf("call MT_UNF_DMX_AttachTSPort failed.\n");
	return 0;
    }

    Ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &g_TsBuf);
    if (Ret != MT_SUCCESS) {
	printf("call MT_UNF_DMX_CreateTSBuffer failed.\n");
	return 0;
    }

    pthread_mutex_init(&g_TsMutex, NULL);
    g_bStopTsThread = MT_FALSE;
    pthread_create(&g_TsThd, MT_NULL, (mt_void *)TsTthread, MT_NULL);


    if (Ret != MT_SUCCESS) {
	printf("call MTADP_Search_GetAllPmt failed........\n");
	while (1) {
	    if ('q' == InputCmd[0]) {
		printf("prepare to quit!\n");
		break;
	    }
	    sleep(10);
	    printf("----------sleep ........\n");
	}
	return 0;
    }

    ProgNum = 0;

    pthread_mutex_lock(&g_TsMutex);
    rewind(g_pTsFile);
    MT_UNF_DMX_ResetTSBuffer(g_TsBuf);
    pthread_mutex_unlock(&g_TsMutex);


    while (1) {

	//SAMPLE_GET_INPUTCMD(InputCmd);
	printf("please input 'h' to get help or 'q' to quit!\n");
	printf("hdmi_cmd >....");
	memset(InputCmd, 0, 128);
	SAMPLE_GET_INPUTCMD(InputCmd);
	if ('q' == InputCmd[0]) {
	    printf("prepare to quit 666666666!\n");
	    break;
	}
    }

SYS_DEINIT:
    //MT_SYS_DeInit();
    mt_sys_deinit();
    fclose(g_pTsFile);
    g_pTsFile = MT_NULL;

    return 0;
}
