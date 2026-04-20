/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "pthread.h"
#include "sample_audio.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"
#include "mt_common.h"
#include "mt_unf_dma.h"

#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"
#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_AUDIO_DEBUG
#define MT_AUDIO_PRINT   printf
#else
#define MT_AUDIO_PRINT
#endif

#define SAMPLE_AUDIO_FUNCTION_ENTER()           MT_AUDIO_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_AUDIO_FUNCTION_EXIT()            MT_AUDIO_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_AUDIO_FATAL_PRINT(fmt...)        MT_AUDIO_PRINT(" [FATAL] " fmt)
#define SAMPLE_AUDIO_ERR_PRINT(fmt...)          MT_AUDIO_PRINT(" [ERROR] " fmt)
#define SAMPLE_AUDIO_WARN_PRINT(fmt...)         MT_AUDIO_PRINT(" [WARN] "  fmt)
#define SAMPLE_AUDIO_INFO_PRINT(fmt...)         MT_AUDIO_PRINT(" [INFO] "  fmt)
#define SAMPLE_AUDIO_DBG_PRINT(fmt...)          MT_AUDIO_PRINT(" [DEBUG] " fmt)

#define DEBUG_AUDIO_CRC_BUF_SIZE    (4*30*10)
#define TEST_STR_NUB                4096
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
#define FILENUM                64
/*************************** Structure Definition ****************************/

typedef struct
{
    MT_S32 decType;
    MT_S32 decMode;
    MT_CHAR file_name[256];
    MT_U8 path[256];
    MT_BOOL iscycle;
}mt_audio_para_t;

typedef struct
{
    MT_HANDLE   hAvPlay;
    MT_CHAR     file_name[256];
}mt_audio_thread_para_t;


typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    pthread_t          hEsThd;
} MT_AUDIO_RUN_INFO;

/********************** Global Variable declaration **************************/
static MT_U8        u8DecOpenBuf[1024];
static MT_BOOL      g_bTaskQuit = MT_TRUE;
static MT_AUDIO_RUN_INFO    g_stAudioRunInfo = {MT_INVALID_HANDLE};
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_AudioModeMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_BOOL isDirectoryExists(const char *path)
{
    struct stat info;

    if(stat(path, &info) != 0)
    {
        //The folder does not exist
        return MT_FALSE;
    }
    else if
    (info.st_mode & S_IFDIR)
    {
        //It's a folder
        return MT_TRUE;
    }
    else
    {
        //Not a folder
        return MT_FALSE;
    }
}

/*!
@brief audio playback initialization
@param[in]  hAvplay           Handle to soundtrack
@param[in]  hSoundTrack       Handle to AV player
@return::MT_SUCCESS           Success.
@return::MT_FAILURE           Fail
@return::ret                  Fail
@*/
static MT_S32 MT_AudioModeInit(MT_HANDLE *hAvplay, MT_HANDLE *hSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    MT_HANDLE                avplay = MT_INVALID_HANDLE;
    MT_HANDLE                track = MT_INVALID_HANDLE;
    MT_UNF_SYNC_ATTR_S       AvSyncAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    if(NULL == hAvplay || NULL == hSoundTrack)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input address is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return ret;
    }

    MTADP_AVPlay_RegADecLib();
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    memset(&AvplayAttr, 0, sizeof(MT_UNF_AVPLAY_ATTR_S));
    ret  = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);

    /** Enable Audio Debug CRC Buffer, for Autotest Only */
    AvplayAttr.stStreamAttr.u32DebugAudCrcBufSize = DEBUG_AUDIO_CRC_BUF_SIZE;
    ret |= MT_UNF_AVPLAY_Create(&AvplayAttr, &avplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Create failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR1;
    }

    memset(&AvSyncAttr, 0, sizeof(MT_UNF_SYNC_ATTR_S));
    ret = MT_UNF_AVPLAY_GetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);

    AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    ret |= MT_UNF_AVPLAY_SetAttr(avplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_ChnOpen failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR2;
    }

    memset(&stTrackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_SND_GetDefaultTrackAttr failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR3;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &track);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_SND_CreateTrack failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR3;
    }

    ret = MT_UNF_SND_Attach(track, avplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_SND_Attach failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR4;
    }

    ret = MT_UNF_SND_SetTrackMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, MT_UNF_TRACK_MODE_STEREO);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_SND_SetTrackMode failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        goto ERR5;
    }

    *hAvplay = avplay;
    *hSoundTrack = track;
    return MT_SUCCESS;

ERR5:
    (MT_VOID)MT_UNF_SND_Detach(track, avplay);
ERR4:
    (MT_VOID)MT_UNF_SND_DestroyTrack(track);
ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(avplay);
ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}


/*!
@brief audio playback deinitialization
@param[in]  track             Handle to soundtrack
@param[in]  avplay            Handle to AV player
@return::MT_SUCCESS           Success.
@return::MT_FAILURE           Fail
@*/
static MT_VOID MT_AudioModeDeinit(MT_HANDLE hTrack, MT_HANDLE hAvplay)
{
    if(MT_INVALID_HANDLE == hTrack || MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    (MT_VOID)MT_UNF_SND_Detach(hTrack, hAvplay);

    (MT_VOID)MT_UNF_SND_DestroyTrack(hTrack);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    (MT_VOID)MT_UNF_AVPLAY_DeInit();
}

static void aud_dmacpy_data(void *dst, const void *src, size_t size)
{
#define AUD_DMA_CHANNEL_ID  6
    int ret = -1;
    phys_addr_t psrc = 0;
    phys_addr_t pdst = 0;
    unsigned char *vsrc_mmz = NULL;
    const unsigned int ch = AUD_DMA_CHANNEL_ID;

    if((NULL == dst) || (NULL == src)) {
        printf("Audio dam copy para error\n");
        return NULL;
    }

    while (MT_EDMA_STATUS_FREE != mt_unf_dma_check((MT_EDMA_CH_E)ch)) {
        usleep(10*1000);
    }
    
    mt_unf_dma_release_channel((MT_EDMA_CH_E)ch);
    ret = mt_unf_dma_request_channel((MT_EDMA_CH_E)ch);
    if (ret != MT_SUCCESS) {
        printf("DMA copy channel is busy!\n");
        ret = MT_FAILURE;
        goto exit;
    }

    psrc = mt_mmz_new(size, 0, "ddr",  "aud_mmz_buf");
    if(0 != psrc){
        /* Copy not continuous address to mmz zone */
        vsrc_mmz = (unsigned char *) mt_mmz_map(psrc, 0);
        if (NULL == vsrc_mmz) {
            ret = MT_FAILURE;
            printf("MMZ Map ERROR\n");
            goto exit;
        }
        memcpy(vsrc_mmz, (const void *)src, size);
    } else {
        ret = MT_FAILURE;
        printf("New MMZ ERROR\n");
        goto exit;
    }

    ret = mt_mem_get_phyaddr((void *)dst, &pdst);
    if(ret != MT_SUCCESS){
        printf("Get phyaddr ERROR\n");
        goto exit;
    }

    ret = mt_unf_dma_memcpy((MT_EDMA_CH_E)ch, psrc, pdst, size);
    if (ret != MT_SUCCESS) {
        printf("dma-%d copy from %p to %p with size(0x%x) failed\n", ch, src, dst, (int) size);
        ret = MT_FAILURE;
        goto exit;
    }

    ret = MT_SUCCESS;
exit:
    //printf("[%s] ch%d: dst = 0x%lx, src = 0x%lx \n", __func__, ch, dst, src);
    mt_unf_dma_release_channel((MT_EDMA_CH_E)ch);
    if (NULL != vsrc_mmz) {
        mt_mmz_unmap(vsrc_mmz);
        mt_mmz_delete(psrc);
    }

    if (ret != MT_SUCCESS){
        return NULL;
    }
    return dst;
}

/*!
@brief The thread that parses the data stream for the audio file
@param[in]  hAvplay         Handle to AV player
@return::void
@*/
static MT_VOID MT_AudioModeInjectEsTask(MT_VOID *args)
{
    MT_U32               Readlen = 0;
    MT_S32               ret = 0;
    MT_UNF_STREAM_BUF_S  StreamBuf = { 0 };
    FILE *pTsFile = NULL;
    MT_HANDLE            hAvplay;
	unsigned int es_data[4096];

    mt_audio_thread_para_t *pThreadPara = (mt_audio_thread_para_t *)(args);
    hAvplay = pThreadPara->hAvPlay;
    SAMPLE_AUDIO_INFO_PRINT(">>>open file : %s  >>>> \n", pThreadPara->file_name);
    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen(pThreadPara->file_name, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_AUDIO_ERR_PRINT( "file %s open error!!\n", pThreadPara->file_name);
        g_bTaskQuit = MT_TRUE;
        return;
    }

    while(!g_bTaskQuit)
    {
        ret = MT_UNF_AVPLAY_GetBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, 0x1000, &StreamBuf, 0);
        if(MT_SUCCESS != ret)
        {
            mt_msleep(10);
        }
        else
        {
            if(StreamBuf.u32Size >= 0x1000)
            {
                Readlen = fread(es_data, 1, 0x1000, pTsFile);
            }
            else if(StreamBuf.u32Size == 0)
            {
                continue;
            }
            else
            {
                Readlen = fread(es_data, 1, StreamBuf.u32Size, pTsFile);
            }
            if(Readlen > 0)
            {
            #ifdef CONFIG_MT_EXT_VVID_SUPPORT
				memcpy(StreamBuf.pu8Data, es_data, Readlen);
			#else
            	aud_dmacpy_data(StreamBuf.pu8Data, es_data, Readlen);
			#endif
                ret = MT_UNF_AVPLAY_PutBuf(hAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD, Readlen, 10);
                if(ret != MT_SUCCESS)
                {
                    SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_PutBuf failed, ret = 0x%x \n", ret);
                }
            }
            else if(Readlen <= 0)
            {
                SAMPLE_AUDIO_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER...............!\n");
                rewind(pTsFile);
            }
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    return;
}


/*!
@brief Set the HA format
@param[in]  mtatype         HA format
@return::atype              HA format definition
@*/
static MT_U32 MT_AudioModeParseType(MT_S32 mtatype)
{
    MT_U32 atype = 0;
    switch (mtatype)
    {
        case 0:
            atype = HA_AUDIO_ID_PCM;
            break;
        case 1:
            atype = HA_AUDIO_ID_MP2;
            break;
        case 2:
            atype = HA_AUDIO_ID_MP3;
            break;
        case 3:
            atype = HA_AUDIO_ID_DOLBY_TRUEHD;
            break;
        case 4:
            atype = HA_AUDIO_ID_AAC;
            break;
        case 5:
            atype = HA_AUDIO_ID_DRA;
            break;
        case 6:
            atype = HA_AUDIO_ID_VORBIS;
            break;
        case 7:
            atype = HA_AUDIO_ID_OPUS;
            break;
        case 8:
            atype = HA_AUDIO_ID_FLAC;
            break;
        case 9:
            atype = HA_AUDIO_ID_APE;
            break;
        case 10:
            atype = HA_AUDIO_ID_AC3PASSTHROUGH;
            break;
        case 11:
            atype = HA_AUDIO_ID_DOLBY_CONVERT;
            break;
        case 12:
            atype = HA_AUDIO_ID_DTSPASSTHROUGH;
            break;
        case 13:
            atype = HA_AUDIO_ID_OGG;
            break;
		case 14:
			atype = HA_AUDIO_ID_VVID;
			break;
		case 15:
			atype = HA_AUDIO_ID_DOLBY_AC4;
			break;
        default:
            SAMPLE_AUDIO_ERR_PRINT("wrong params or unsupport audio type\n");
            break;
    }
    return atype;
}


/*!
@brief Set the decoding properties of AVplay
@param[in]  avplay          handle to AV player
@param[in]  enADecType      HA format
@param[in]  enMode          The decoding mode of the HA decoder
@param[in]  isCoreOnly      NULL
@param[in]  samp_rate       PCM sample rates
@return::MT_SUCCESS         Success.
@return::MT_FAILURE         Fail.
@return::ret                Fail
@*/
static MT_S32 MT_AudioModeSetAdecAttr(MT_HANDLE hAvplay, MT_U32 enADecType, MT_HA_DECODEMODE_E enMode, MT_S32 isCoreOnly, MT_U32 samp_rate)
{
    MT_S32               ret = MT_FAILURE;
    WAV_FORMAT_S         stWavFormat = { 0 };
    MT_UNF_ACODEC_ATTR_S AdecAttr = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
    }

    AdecAttr.enType = enADecType;

    if (HA_AUDIO_ID_PCM == AdecAttr.enType)
    {
        /* set pcm wav format here base on pcm file 48k.raw */
        stWavFormat.nChannels = 2;
        stWavFormat.wBitsPerSample = 16;
        stWavFormat.cbExtWord[0] = 0;
        stWavFormat.cbExtWord[1] = 1;
        if(AUDIO_SAMPLE_48 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 48000;
        }
        else if(AUDIO_SAMPLE_44 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 44000;
        }
        else if(AUDIO_SAMPLE_32 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 32000;
        }
        else if(AUDIO_SAMPLE_24 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 24000;
        }
        else if(AUDIO_SAMPLE_22 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 22000;
        }
        else if(AUDIO_SAMPLE_16 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 16000;
        }
        else if(AUDIO_SAMPLE_96 == samp_rate)
        {
            stWavFormat.nSamplesPerSec = 96000;
        }
        else
        {
            stWavFormat.nSamplesPerSec = 48000;
        }

        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), &stWavFormat);
        SAMPLE_AUDIO_INFO_PRINT("please make sure the attributes of PCM stream is tme same as defined in function of \"MTADP_AVPlay_SetAdecAttr\"? \n");
        SAMPLE_AUDIO_INFO_PRINT("(nChannels = 1, wBitsPerSample = 16, nSamplesPerSec = 48000, isBigEndian = MT_FALSE) \n");
    }
    else if(HA_AUDIO_ID_MP2 == AdecAttr.enType)
    {
         HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_AAC == AdecAttr.enType)
    {
         HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_MP3 == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_VORBIS == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_OGG == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_OPUS == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_FLAC == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_APE == AdecAttr.enType)
    {
         HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_AC3PASSTHROUGH== AdecAttr.enType)
    {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_DTSPASSTHROUGH ==  AdecAttr.enType)
    {
        HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_TRUEHD == AdecAttr.enType)
    {
        HA_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        if(HD_DEC_MODE_THRU != enMode)
        {
            SAMPLE_AUDIO_ERR_PRINT(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return MT_FAILURE;
        }

        /* truehd just support pass-through */
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
        SAMPLE_AUDIO_INFO_PRINT(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if(HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if(HA_AUDIO_ID_DOLBY_CONVERT == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_CONVERT_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if(HA_AUDIO_ID_DTSHD == AdecAttr.enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if(HA_AUDIO_ID_DTSM6 == AdecAttr.enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if(HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#endif
    else if(HA_AUDIO_ID_DRA == AdecAttr.enType)
    {
        HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB ==AdecAttr.enType
        || HA_AUDIO_ID_AMRWB == AdecAttr.enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        SAMPLE_AUDIO_INFO_PRINT("cook dec set ffmpeg dec param \n");
    }


    ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_GetAttr failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;
}


/*!
@brief Open the file and start playing the audio file
@param[in]  param           Parameters for playing audio files
@param[in]  avplay          handle to AV player
@return::MT_SUCCESS         Success.
@return::MT_FAILURE         Fail
@return::ret                Fail
@*/
static MT_S32 MT_AudioModeStartEs(mt_audio_para_t param, MT_HANDLE hAvplay)
{
    MT_S32                   ret = MT_FAILURE;
    MT_S32                   s32DtsDtsCoreOnly = 0;
    HA_CODEC_ID_E            AdecType = { 0 };
    MT_HA_DECODEMODE_E       enAudioDecMode = HD_DEC_MODE_RAWPCM;
    audio_sample_rate_t      AudioSampleRate = AUDIO_SAMPLE_48;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    enAudioDecMode = param.decMode;
    AdecType = MT_AudioModeParseType(param.decType);

    ret = MT_AudioModeSetAdecAttr(hAvplay, AdecType, enAudioDecMode, s32DtsDtsCoreOnly, AudioSampleRate);
    ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Start failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;

}


/*!
@brief Stop playing the audio file and close the file
@param[in]  avplay          handle to AV player
@return::MT_SUCCESS         Success.
@return::MT_FAILURE         Fail
@*/
static MT_S32 MT_AudioModeStopEs(MT_HANDLE hAvplay)
{
    MT_S32                   ret = MT_FAILURE;
    MT_UNF_AVPLAY_STOP_OPT_S stop = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stop.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stop);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Stop failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;
}


/*!
@brief pause playback
@param[in]  avplay            handle to AV player
@return::MT_SUCCESS           Success.
@return::ret                  Fail
@*/
static MT_S32 MT_AudioModePauseEs(MT_HANDLE hAvplay)
{
    MT_S32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_PAUSE_OPT_S pstPauseOpt = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_Pause(hAvplay, &pstPauseOpt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Pause failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;
}


/*!
@brief resume playback
@param[in]  avplay            handle to AV player
@return::MT_SUCCESS           Success.
@return::ret                  Fail
@*/
static MT_S32 MT_AudioModeResumeEs(MT_HANDLE hAvplay)
{
    MT_S32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_RESUME_OPT_S pstResumeOpt = { 0 };

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_Resume(hAvplay, &pstResumeOpt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MT_UNF_AVPLAY_Resume failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }

    return MT_SUCCESS;
}

static MT_VOID MT_AudioModeExit(void)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)MT_AudioModeStopEs(g_stAudioRunInfo.hAvPlay);

    (MT_VOID)pthread_join(g_stAudioRunInfo.hEsThd, MT_NULL);

    (MT_VOID)MT_AudioModeDeinit(g_stAudioRunInfo.hSoundTrack, g_stAudioRunInfo.hAvPlay);

    (MT_VOID)MTADP_Snd_DeInit();
}

static MT_S32 MT_AudioModeReadFilename(MT_CHAR *path, MT_S32 *num, MT_CHAR folderName[][256])
{
    MT_S32 i = 0;

    if(isDirectoryExists(path))
    {
        MT_AUDIO_PRINT("The folder exists\n");
    }
    else
    {
        SAMPLE_AUDIO_ERR_PRINT("The folder does not exist\n");
        return MT_FAILURE;
    }

    DIR *dir = opendir(path);//打开目录文件
    struct dirent *entry;
    while((entry = readdir(dir))!=0)
    {
        if(strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0)
        {
            continue;
        }
        sprintf(folderName[i], "%s", entry->d_name);
        i++;
    }

    *num = i;

    (MT_VOID)closedir(dir);

    return MT_SUCCESS;
}

static MT_CHAR *MT_AudioModeGetFileExtension(const MT_CHAR *filePath)
{
    size_t length = strlen(filePath);

    for(int i = length - 1; i >= 0; i--)
    {
        if(filePath[i] == '.')
        {
            return &filePath[i + 1];
        }

        if(filePath[i] == '/')
        {
            SAMPLE_AUDIO_ERR_PRINT("The file is unplayable\n");
            break;
        }
    }

    return "error";
}

static MT_S32 MT_AudioModeType(MT_CHAR *fileExtension)
{
    if(strcmp(fileExtension, "pcm") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play pcm files\n");
        return 0;
    }
    else if(strcmp(fileExtension, "mp2") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play mp2 files\n");
        return 1;
    }
    else if(strcmp(fileExtension, "mp3") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play mp3 files\n");
        return 2;
    }
    else if(strcmp(fileExtension, "aac") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play acc files\n");
        return 4;
    }
    else if(strcmp(fileExtension, "dra") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play dra files\n");
        return 5;
    }
    else if(strcmp(fileExtension, "vorbis") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play vorbis files\n");
        return 6;
    }
    else if(strcmp(fileExtension, "opus") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play opus files\n");
        return 7;
    }
    else if(strcmp(fileExtension, "flac") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play flac files\n");
        return 8;
    }
    else if(strcmp(fileExtension, "ape") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play ape files\n");
        return 9;
    }
    else if(strcmp(fileExtension, "ac3") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play ac3 files\n");
        return 10;
    }
    else if(strcmp(fileExtension, "ogg") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play ogg files\n");
        return 13;
    }
    else if(strcmp(fileExtension, "av3a") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play vvid files\n");
        return 14;
    }
    else if(strcmp(fileExtension, "ac4") == 0)
    {
        SAMPLE_AUDIO_INFO_PRINT("Play ac4 files\n");
        return 15;
    }
    else if(strcmp(fileExtension, "error") == 0)
    {
        SAMPLE_AUDIO_ERR_PRINT("This file cannot be played\n");
        return MT_FAILURE;
    }
    else
    {
        SAMPLE_AUDIO_ERR_PRINT("This file cannot be played\n");
        return MT_FAILURE;
    }
}

static MT_VOID MT_AudioModePrintMenu(MT_VOID)
{
    MT_AUDIO_PRINT("commond: \n");
    MT_AUDIO_PRINT("     p: pause playback\n");
    MT_AUDIO_PRINT("     r: resume playback \n");
#ifdef MT_SAMPLE_APP
    MT_AUDIO_PRINT("     b: background run \n");
#endif
    MT_AUDIO_PRINT("     q: quit \n");
    MT_AUDIO_PRINT("     h: help \n");
    MT_AUDIO_PRINT("=============================\n");
    MT_AUDIO_PRINT("AUDIO>> ");
}

/*!
@brief Task action commands
@param[in]  avplay     handle to AV player
@return::void
@*/
static MT_VOID MT_AudioModeCmdTask(MT_S32 fileNum, mt_audio_para_t param, MT_CHAR folderName[][256])
{

    MT_CHAR     inPutCmd[32] = { 0 };
    MT_CHAR     fileName[256] = { 0 };
    MT_CHAR     *fileExtension = MT_NULL;
    MT_S32      ret = MT_FAILURE;
    MT_S32      pauseStatus = MT_FAILURE;
    MT_S32      time = 0;
    mt_audio_thread_para_t thread_para = { 0 };

    if(MT_INVALID_HANDLE == g_stAudioRunInfo.hAvPlay)
    {
        SAMPLE_AUDIO_ERR_PRINT("The input handle is empty!-------<%s> line: %d\n", __FUNCTION__, __LINE__);
        return;
    }

    while(g_bTaskQuit != MT_TRUE || param.iscycle == MT_TRUE)
    {
        (MT_VOID)MT_AudioModePrintMenu();

        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);

        if('q' == inPutCmd[0])
        {
            SAMPLE_AUDIO_INFO_PRINT("<exit>\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if('b' == inPutCmd[0])
        {
            SAMPLE_AUDIO_INFO_PRINT("audio play in back!\n");

            return;
        }
    #endif
        else if('p' == inPutCmd[0])
        {
            if(MT_FAILURE == pauseStatus)
            {
                ret = MT_AudioModePauseEs(g_stAudioRunInfo.hAvPlay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_AUDIO_ERR_PRINT("MT_AudioModePauseEs failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
                    break;
                }

                pauseStatus = MT_SUCCESS;
                SAMPLE_AUDIO_INFO_PRINT("Pause success!\n");
            }
            else
            {
                SAMPLE_AUDIO_INFO_PRINT("Already in a paused state\n");
            }
        }
        else if('r' == inPutCmd[0])
        {
            if(pauseStatus == MT_SUCCESS)
            {
                ret = MT_AudioModeResumeEs(g_stAudioRunInfo.hAvPlay);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_AUDIO_ERR_PRINT("MT_AudioModeResumeEs failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
                    break;
                }

                pauseStatus = MT_FAILURE;
                SAMPLE_AUDIO_INFO_PRINT("Resume success!\n");
            }
            else
            {
                SAMPLE_AUDIO_INFO_PRINT("It is not currently in a paused state and does not need to be resumed\n");
            }
        }
        else if('t' == inPutCmd[0])
        {
            (MT_VOID)MT_AudioModeStopEs(g_stAudioRunInfo.hAvPlay);

            g_bTaskQuit = MT_TRUE;
            (MT_VOID)pthread_join(g_stAudioRunInfo.hEsThd, MT_NULL);
        }
        else if('y' == inPutCmd[0])
        {
            time++;
            if(fileNum == time)
            {
                time = 0;
            }
            fileExtension = MT_AudioModeGetFileExtension(folderName[time]);
            if(MT_NULL == fileExtension)
            {
                continue;
            }

            param.decType = MT_AudioModeType(fileExtension);
            if (MT_FAILURE == param.decType)
            {
                continue;
            }
            g_bTaskQuit = MT_FALSE;
            snprintf(fileName, sizeof(fileName), "%s/%s", param.path, folderName[time]);
            SAMPLE_AUDIO_INFO_PRINT("File name: %s\n", fileName);
            ret = MT_AudioModeStartEs(param, g_stAudioRunInfo.hAvPlay);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_AUDIO_ERR_PRINT("MT_Start_Audio failed, ret = 0x%x \n", ret);
                continue;
            }

            thread_para.hAvPlay = g_stAudioRunInfo.hAvPlay;
            MTADP_Strncpy(thread_para.file_name, fileName, sizeof(thread_para.file_name));

            pthread_create(&g_stAudioRunInfo.hEsThd, MT_NULL, (MT_VOID * (*)(MT_VOID *))MT_AudioModeInjectEsTask, &thread_para);
            sleep(1);
        }
        else if('h' == inPutCmd[0])
        {
            SAMPLE_AUDIO_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}



/*!
@brief Print help information
@param[in]  name         Program name
@return::MT_VOID
@*/
static MT_VOID MT_AudioModePrint_Help(MT_CHAR *name)
{
    MT_AUDIO_PRINT("Lack of parameters\n");
    MT_AUDIO_PRINT("\nUsage:\n");
    MT_AUDIO_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MT_AUDIO_PRINT("    -q: Exit the background\n");
#endif
    MT_AUDIO_PRINT("    -f: File path\n");
    MT_AUDIO_PRINT("    -t: Audio format\n");
    MT_AUDIO_PRINT("example:\n");
    MT_AUDIO_PRINT("    %s -f ./1.mp3\n", name);
    MT_AUDIO_PRINT("    %s -t ./audio_file\n", name);
}


/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[in]  param           Gets the external input parameters
@return::MT_VOID
@*/
static MT_S32 MT_AudioModeParase_args(MT_S32 argc, MT_CHAR *argv[], mt_audio_para_t *param)
{
    MT_S32 opt = 0;

    SAMPLE_AUDIO_FUNCTION_ENTER();

    if(argc < 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_AudioModePrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHq:f:t")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_AudioModePrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                (MT_VOID)MT_AudioModeExit();
                return MT_TASK_EXIT;
            case 'f':
                if(argc != 3 && g_bTaskQuit == MT_TRUE)
                {
                    (MT_VOID)MT_AudioModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                MTADP_Strncpy(param->file_name, argv[2], sizeof(param->file_name));
                param->iscycle = MT_FAILURE;
                return MT_SUCCESS;
            case 't':
                if(argc != 3 && g_bTaskQuit == MT_TRUE)
                {
                    (MT_VOID)MT_AudioModePrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                MTADP_Strncpy((mt_char*)param->path, argv[2], sizeof(param->path));
                param->iscycle = MT_TRUE;
                return MT_SUCCESS;
            default:
                (MT_VOID)MT_AudioModePrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_AUDIO_FUNCTION_EXIT();
    return MT_SUCCESS;
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_AudioModeMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32      ret = MT_FAILURE;
    MT_CHAR     folderName[FILENUM][256] = { 0 };
    MT_CHAR     *fileExtension = MT_NULL;
    MT_S32      filenum = 0;
    mt_audio_para_t param = { 0 };
    mt_audio_thread_para_t thread_para = { 0 };

    SAMPLE_AUDIO_FUNCTION_ENTER();

    ret = MT_AudioModeParase_args(argc, argv, &param);
    if (MT_FAILURE == ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(MT_TRUE == param.iscycle)
    {
        ret = MT_AudioModeReadFilename((mt_char*)param.path, &filenum, folderName);
        if(MT_FAILURE == ret)
        {
            return MT_FAILURE;
        }

        for(MT_S32 i = 0; i < filenum; i++)
        {
            memset(param.file_name, 0, sizeof(param.file_name));
            snprintf(param.file_name, sizeof(param.file_name), "%s/%s", param.path, folderName[i]);
            fileExtension = MT_AudioModeGetFileExtension(param.file_name);
            param.decType = MT_AudioModeType(fileExtension);
            if(MT_FAILURE == param.decType)
            {
                continue;
            }
        }
    }
    else
    {
        fileExtension = MT_AudioModeGetFileExtension(param.file_name);
        param.decType = MT_AudioModeType(fileExtension);
    }

    if (MT_FAILURE == param.decType)
    {
        SAMPLE_AUDIO_ERR_PRINT("No playable files!\n");
        return MT_FAILURE;
    }

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
        return MT_FAILURE;
    }

    /** HDMI initialization */
    ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
        goto ERR0;
    }

    /** Display initialization */
    ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_AUDIO_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
        goto ERR1;
    }
#endif

    if(MT_TRUE == g_bTaskQuit)
    {
        g_bTaskQuit = MT_FALSE;
        /** The type and mode of the playback file */
        param.decMode = 0;

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AUDIO_ERR_PRINT("MT_UNF_SND_Init failed, ret = 0x%x\n", ret);
            goto ERR2;
        }

        ret = MT_AudioModeInit(&g_stAudioRunInfo.hAvPlay, &g_stAudioRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AUDIO_ERR_PRINT("MT_AudioInit failed, ret = 0x%x \n", ret);
            goto ERR3;
        }

        ret = MT_AudioModeStartEs(param, g_stAudioRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_AUDIO_ERR_PRINT("MT_Start_Audio failed, ret = 0x%x \n", ret);
            goto ERR4;
        }
        thread_para.hAvPlay = g_stAudioRunInfo.hAvPlay;
        MTADP_Strncpy(thread_para.file_name, param.file_name, sizeof(thread_para.file_name));

        pthread_create(&g_stAudioRunInfo.hEsThd, MT_NULL, (MT_VOID * (*)(MT_VOID *))MT_AudioModeInjectEsTask, &thread_para);
        sleep(1);
    }

    (MT_VOID)MT_AudioModeCmdTask(filenum, param, folderName);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)MT_AudioModeStopEs(g_stAudioRunInfo.hAvPlay);

    if(MT_FALSE == g_bTaskQuit)
    {
        g_bTaskQuit = MT_TRUE;
        (MT_VOID)pthread_join(g_stAudioRunInfo.hEsThd, MT_NULL);
    }

ERR4:
    (MT_VOID)MT_AudioModeDeinit(g_stAudioRunInfo.hSoundTrack, g_stAudioRunInfo.hAvPlay);
ERR3:
    (MT_VOID)MTADP_Snd_DeInit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR0:
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;
    memset(&g_stAudioRunInfo, 0xff, sizeof(g_stAudioRunInfo));
    SAMPLE_AUDIO_FUNCTION_EXIT();

    return ret;
}

