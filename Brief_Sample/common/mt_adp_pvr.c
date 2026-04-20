/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include "mt_adp_pvr.h"
#include "mt_adp_mpi.h"
#include "mt_adp_demux.h"

//#define PVR_ADVCA_MODE /*if use ADVCA mode,define PVR_ADVCA_MODE*/
/***************************** Macro Definition ******************************/
#define sample_common_printf  printf

/*************************** Structure Definition ****************************/
typedef struct hiTS_SEND_ARGS_S
{
    mt_u32 u32DmxId;
    mt_u32 u32PortId;
    FILE   *pTsFile;
} TS_SEND_ARGS_S;

typedef struct tagPVREventType
{
    mt_u8 szEventTypeName[128];
    MT_UNF_PVR_EVENT_E eEventID;
}PVR_EVENT_TYPE_ST;

typedef struct hiCODEC_VIDEO_CMD_S
{
    mt_u32      u32CmdID;   /**<Commond ID*/ /**<CNcomment: 命令ID*/
    mt_void     *pPara;     /**<Control parameter*/ /**<CNcomment: 命令携带参数*/
}MT_CODEC_VIDEO_CMD_S;

/********************** Global Variable declaration **************************/
static mt_s32       g_s32PvrRecChnNum[DMX_COUNT]; /* number of record channel for each demux */
static mt_handle    g_hPvrRecChns[DMX_COUNT][8];  /* handle of record channel for each demux */

MT_BOOL             g_bIsRecStop = MT_FALSE;
//static MT_BOOL      g_bStopTsThread = MT_FALSE;
mt_handle           g_hTsBufForPlayBack;
mt_handle           g_hPvrPlayChn = MT_INVALID_HANDLE;


static PVR_EVENT_TYPE_ST g_stEventType[] = {
    {"MT_UNF_PVR_EVENT_PLAY_EOF",       MT_UNF_PVR_EVENT_PLAY_EOF},
    {"MT_UNF_PVR_EVENT_PLAY_SOF",       MT_UNF_PVR_EVENT_PLAY_SOF},
    {"MT_UNF_PVR_EVENT_PLAY_ERROR",     MT_UNF_PVR_EVENT_PLAY_ERROR},
    {"MT_UNF_PVR_EVENT_PLAY_REACH_REC", MT_UNF_PVR_EVENT_PLAY_REACH_REC},
    {"MT_UNF_PVR_EVENT_PLAY_RESV",      MT_UNF_PVR_EVENT_PLAY_RESV},
    {"MT_UNF_PVR_EVENT_REC_DISKFULL",   MT_UNF_PVR_EVENT_REC_DISKFULL},
    {"MT_UNF_PVR_EVENT_REC_ERROR",      MT_UNF_PVR_EVENT_REC_ERROR},
    {"MT_UNF_PVR_EVENT_REC_OVER_FIX",   MT_UNF_PVR_EVENT_REC_OVER_FIX},
    {"MT_UNF_PVR_EVENT_REC_REACH_PLAY", MT_UNF_PVR_EVENT_REC_REACH_PLAY},
    {"MT_UNF_PVR_EVENT_REC_DISK_SLOW",  MT_UNF_PVR_EVENT_REC_DISK_SLOW},
    {"MT_UNF_PVR_EVENT_REC_DMX_CREATE",  MT_UNF_PVR_EVENT_REC_DMX_CREATE},
    {"MT_UNF_PVR_EVENT_REC_RESV",       MT_UNF_PVR_EVENT_REC_RESV},
    {"MT_UNF_PVR_EVENT_BUTT",           MT_UNF_PVR_EVENT_BUTT}
};

#if 0 //not used
static void dmx_spes_dumpts(mt_u8 *head,mt_u32 lens)
{
    int i;
    printf("key:\n");
    for(i=0;i<lens;i++){
        if(!(i&7)){
            printf("\n");
        }
        printf("%02x ",head[i]);
    }
    printf("\n");
}
#endif

static pthread_mutex_t g_pvrcrypto_sample=PTHREAD_MUTEX_INITIALIZER;
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
static MT_S32 CipherRecCreate(mt_u32 *o_slot_id, mt_handle *o_p_cipher,MT_UNF_PVR_CIPHER_S *stEncryptCfg,MT_CIPHER_CRYPTO_CH_E chid,mt_u32 enordec)
{//enordec,0:en 1:dec
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_cipher;
    MT_CIPHER_CTRL_S info;
    mt_u32 slot_id = 0;
    {
#ifndef CONFIG_MT_CHIP_SYMPHONY1
        slot_id = MT_CIPHER_KEYSLOT_INVALID;
        mt_unf_cipher_keyslot_request(&slot_id);

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));

        info.operation = enordec;
        info.algorithm = stEncryptCfg->enType;
        info.work_mode = MT_CIPHER_WORK_MODE_ECB;
        //printf("+c.%x,%x\n",enordec,stEncryptCfg->enType);
        //dmx_spes_dumpts(stEncryptCfg->au8Key,16);
        mt_unf_cipher_keyslot_set(slot_id, &info, stEncryptCfg->au8Key, NULL);    //clear-text key, no IV

        ret = mt_unf_cipher_crypto_create(chid, &p_cipher);

        ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
        //printf("ret=%x,%x\n",ret,p_cipher);

        if(MT_SUCCESS==ret){
            *o_p_cipher=p_cipher;
            *o_slot_id=slot_id;
        }else{
            *o_p_cipher=MT_INVALID_HANDLE;
            *o_slot_id=MT_CIPHER_KEYSLOT_INVALID;
        }
#endif
    }
    return ret;
}
static MT_S32 CipherRecDestroy(mt_u32 i_slot_id, mt_handle i_p_cipher)
{
    {
#ifndef CONFIG_MT_CHIP_SYMPHONY1
        if (i_p_cipher != MT_INVALID_HANDLE) {
            mt_unf_cipher_crypto_destroy(i_p_cipher);
        }
        if (i_slot_id != MT_CIPHER_KEYSLOT_INVALID) {
            mt_unf_cipher_keyslot_release(i_slot_id);
        }
#endif
    }
    return MT_SUCCESS;
}
#endif
mt_s32 MTADP_PVR_Normal_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    //printf("++norml.w %x\n",u32DataSize);
    return 0;
}

mt_s32 MTADP_PVR_Normal_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    //printf("++norml.r %x\n",u32DataSize);
    return 0;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 MTADP_PVR_Crypto_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    MT_S32 ret=0,RET=0;
    mt_u32 o_slot_id;
    mt_handle o_p_cipher;
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    pthread_mutex_lock(&g_pvrcrypto_sample);
    ret=CipherRecCreate(&o_slot_id,&o_p_cipher,&pstDataAttr->usercfg,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
    RET|=ret;
    ret= mt_unf_cipher_crypto_process_phy(o_p_cipher,u32SrcDataPhyAddr+u32Offset,u32DestPhyAddr+u32Offset,*u32DataSize);
    RET|=ret;
    if(MT_INVALID_HANDLE==o_p_cipher){
    }
    CipherRecDestroy(o_slot_id,o_p_cipher);
    pthread_mutex_unlock(&g_pvrcrypto_sample);
#else
    memcpy(pu8DestVirAddr,pu8SrcDataVirAddr,*u32DataSize);

#endif
    return RET;
}

mt_s32 MTADP_PVR_Crypto_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  mt_u32 u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, mt_u32 u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    MT_S32 ret=0,RET=0;
    mt_u32 o_slot_id;
    mt_handle o_p_cipher;
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    pthread_mutex_lock(&g_pvrcrypto_sample);
    ret=CipherRecCreate(&o_slot_id,&o_p_cipher,&pstDataAttr->usercfg,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
    RET|=ret;
    ret= mt_unf_cipher_crypto_process_phy(o_p_cipher,u32SrcDataPhyAddr+u32Offset,u32DestPhyAddr+u32Offset,*u32DataSize);
    RET|=ret;
    if(MT_INVALID_HANDLE==o_p_cipher){
    }
    CipherRecDestroy(o_slot_id,o_p_cipher);
    pthread_mutex_unlock(&g_pvrcrypto_sample);
#else
    memcpy(pu8DestVirAddr,pu8SrcDataVirAddr,*u32DataSize);
#endif
    return RET;
}

#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 MTADP_PVR_Crypto_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  ulong u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, ulong u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    MT_S32 ret=0,RET=0;
    mt_u32 o_slot_id;
    mt_handle o_p_cipher;
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    pthread_mutex_lock(&g_pvrcrypto_sample);
    ret=CipherRecCreate(&o_slot_id,&o_p_cipher,&pstDataAttr->usercfg,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
    RET|=ret;
    ret= mt_unf_cipher_crypto_process_phy(o_p_cipher, u32SrcDataPhyAddr+u32Offset, u32DestPhyAddr+u32Offset,*u32DataSize);
    RET|=ret;
    if(MT_INVALID_HANDLE==o_p_cipher){
    }
    CipherRecDestroy(o_slot_id,o_p_cipher);
    pthread_mutex_unlock(&g_pvrcrypto_sample);
#else
    memcpy(pu8DestVirAddr,pu8SrcDataVirAddr,*u32DataSize);

#endif
    return RET;
}

mt_s32 MTADP_PVR_Crypto_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
                                        mt_u8 *pu8DestVirAddr,  ulong u32DestPhyAddr,
                                        mt_u8 *pu8SrcDataVirAddr, ulong u32SrcDataPhyAddr,
                                        mt_u32 u32Offset,
                                        mt_u32 *u32DataSize)
{
    MT_S32 ret=0,RET=0;
    mt_u32 o_slot_id;
    mt_handle o_p_cipher;
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
    pthread_mutex_lock(&g_pvrcrypto_sample);
    ret=CipherRecCreate(&o_slot_id,&o_p_cipher,&pstDataAttr->usercfg,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
    RET|=ret;
    ret= mt_unf_cipher_crypto_process_phy(o_p_cipher, u32SrcDataPhyAddr+u32Offset, u32DestPhyAddr+u32Offset,*u32DataSize);
    RET|=ret;
    if(MT_INVALID_HANDLE==o_p_cipher){
    }
    CipherRecDestroy(o_slot_id,o_p_cipher);
    pthread_mutex_unlock(&g_pvrcrypto_sample);
#else
    memcpy(pu8DestVirAddr,pu8SrcDataVirAddr,*u32DataSize);
#endif
    return RET;
}
#endif
/******************************* API declaration *****************************/

#if 0 //not used
static mt_void SearchFileTsSendThread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S   StreamBuf;
    mt_u32            Readlen;
    mt_s32            Ret;
    mt_handle         g_TsBuf;
    TS_SEND_ARGS_S    *pstPara = args;

    Ret = MT_UNF_DMX_AttachTSPort(pstPara->u32DmxId, pstPara->u32PortId);
    if (MT_SUCCESS != Ret)
    {
        sample_common_printf("call VoInit failed.\n");
        return;
    }

    Ret = MT_UNF_DMX_CreateTSBuffer(pstPara->u32PortId, 0x200000, &g_TsBuf);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_DMX_CreateTSBuffer failed.\n");
        return;
    }

    while (!g_bStopTsThread)
    {
        Ret = MT_UNF_DMX_GetTSBuffer(g_TsBuf, 188*50, &StreamBuf, 0);
        if (Ret != MT_SUCCESS )
        {
            MT_USLEEP(10 * 1000) ;
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), 188*50, pstPara->pTsFile);
        if(Readlen <= 0)
        {
            sample_common_printf("read ts file error!\n");
            rewind(pstPara->pTsFile);
            continue;
        }

        Ret = MT_UNF_DMX_PutTSBuffer(g_TsBuf, Readlen);
        if (Ret != MT_SUCCESS )
        {
            sample_common_printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
        }
    }

    Ret = MT_UNF_DMX_DestroyTSBuffer(g_TsBuf);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_DMX_DestroyTSBuffer failed.\n");
    }
    MT_UNF_DMX_DetachTSPort(pstPara->u32DmxId);

    return;
}

static mt_s32 PVR_SearchFile(mt_u32 u32DmxId, mt_u32 u32PortId, const mt_char *pszFileName, PMT_COMPACT_TBL **ppProgTbl)
{
    mt_s32 Ret;
    pthread_t   TsThd;
    TS_SEND_ARGS_S    stPara;
    FILE *pTsFile;

    pTsFile = fopen(pszFileName, "rb");

    stPara.u32DmxId = u32DmxId;
    stPara.u32PortId = u32PortId;
    stPara.pTsFile = (FILE *)pTsFile;

    g_bStopTsThread = MT_FALSE;
    pthread_create(&TsThd, MT_NULL, (mt_void *)SearchFileTsSendThread, &stPara);

    sleep(1);

    MTADP_Search_Init();
    Ret = MTADP_Search_GetAllPmt(u32DmxId, ppProgTbl);
    if (Ret != MT_SUCCESS){
        sample_common_printf("call MTADP_Search_GetAllPmt failed.\n");
    }
    MTADP_Search_DeInit();

    g_bStopTsThread =MT_TRUE;
    pthread_join(TsThd, MT_NULL);
    fclose(pTsFile);

    return Ret;
}
#endif

static mt_s32 MTADP_PVR_RecAddPid(mt_u32 recChnId, mt_u32 u32DmxId, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType)
{
    mt_s32 chnIdx   = 0;
    mt_handle       hPidChn = 0;

    sample_common_printf("DMX:%d add PID start, pid:%d/%#x, Type:%d\n", u32DmxId, pid, pid, chnType);
    if(u32DmxId >= DMX_COUNT)
    {
        sample_common_printf("IN MTADP_PVR_RecAddPid the u32DmxId:%d is over than DMX_CNT:(%d)!!!\n", u32DmxId, DMX_COUNT);
        return MT_FAILURE;
    }

    MT_UNF_PVR_RecSetPid(recChnId, u32DmxId, pid);
    chnIdx %= 8; //by g_hPvrRecChns definition, the max column index should be less than 8

    g_hPvrRecChns[u32DmxId][chnIdx] = hPidChn;
    g_s32PvrRecChnNum[u32DmxId]++;

    sample_common_printf("DMX:%d add PID OK, pid:%d/%#x, Type:%d\n", u32DmxId, pid, pid, chnType);

    return MT_SUCCESS;
}

#if 0 // not used
static mt_s32 PVR_RecDelAllPid(mt_u32 u32DmxId)
{
    mt_s32 i;
    mt_s32 ret = MT_SUCCESS;
    mt_handle hPidChn;

    for (i = 0; i < g_s32PvrRecChnNum[u32DmxId]; i++)
    {
        hPidChn = g_hPvrRecChns[u32DmxId][i];
        ret = MT_UNF_DMX_CloseChannel(hPidChn);
        ret = MT_UNF_DMX_DestroyChannel(hPidChn);
        sample_common_printf("DMX_DestroyChannel: %lu.\n", hPidChn);
    }

    g_s32PvrRecChnNum[u32DmxId]= 0;

    return ret;
}
#endif

mt_s32 MTADP_PVR_SavePorgInfo(PVR_PROG_INFO_S *pstProgInfo, mt_char *pszPvrRecFile)
{
    MT_S32 ret;
    PVR_PROG_INFO_S userData;
    mt_char attrfilepath[128]={0};

    sprintf(attrfilepath,"%s.attr",pszPvrRecFile);
    memcpy(&userData, pstProgInfo, sizeof(PVR_PROG_INFO_S));
    userData.u32MagicNumber = PVR_PROG_INFO_MAGIC;

    ret = MT_UNF_PVR_SetUsrDataInfoByFileName(attrfilepath, (MT_U8*)&userData, sizeof(PVR_PROG_INFO_S));
    if (MT_SUCCESS != ret){
        sample_common_printf("PVR_SetUsrDataInfoByFileName ERR:%#x.\n", ret);
        return ret;
    }

    MT_USLEEP(10*1000);
    sample_common_printf("\n------------------\n");
    sample_common_printf("Save File Info:\n");
    sample_common_printf("Pid:  A=%d/%#x, V=%d/%#x.\n",pstProgInfo->stProgInfo.AElementPid,
                                         pstProgInfo->stProgInfo.AElementPid,
                                         pstProgInfo->stProgInfo.VElementPid,
                                         pstProgInfo->stProgInfo.VElementPid);
    switch (pstProgInfo->stProgInfo.AudioType)
    {
    case HA_AUDIO_ID_PCM:
        sample_common_printf("AudioType= PCM, ");
        break;
    case HA_AUDIO_ID_MP2:
        sample_common_printf("AudioType= MP2, ");
        break;
    case HA_AUDIO_ID_MP3:
        sample_common_printf("AudioType= MP3, ");
        break;
    case HA_AUDIO_ID_AAC:
        sample_common_printf("AudioType= AAC, ");
        break;
    case HA_AUDIO_ID_DRA:
        sample_common_printf("AudioType= DRA, ");
        break;
    case HA_AUDIO_ID_WMA9STD:
        sample_common_printf("AudioType= WMA9STD, ");
        break;
    case HA_AUDIO_ID_DOLBY_PLUS:
        sample_common_printf("AudioType= DOLBY_PLUS, ");
        break;
    case HA_AUDIO_ID_DOLBY_TRUEHD:
        sample_common_printf("AudioType= DOLBY_TRUEHD, ");
        break;
    case HA_AUDIO_ID_DOLBY_CONVERT:
        sample_common_printf("AudioType= DOLBY_CONVERT, ");
        break;
    case HA_AUDIO_ID_DTSHD:
        sample_common_printf("AudioType= DTSHD, ");
        break;
    case HA_AUDIO_ID_AC3PASSTHROUGH:
        sample_common_printf("AudioType= AC3PASSTHROUGH, ");
        break;
    case HA_AUDIO_ID_EAC3PASSTHROUGH:
        sample_common_printf("AudioType= EAC3PASSTHROUGH,, ");
        break;
    case HA_AUDIO_ID_DTSPASSTHROUGH:
        sample_common_printf("AudioType= DTSPASSTHROUGH, ");
        break;
    default:
        sample_common_printf("AudioType= ERROR, ");
    }
    switch (pstProgInfo->stProgInfo.VideoType)
        {
        case MT_UNF_VCODEC_TYPE_AVS:
            sample_common_printf("VideoType= AVS\n");
            break;
        case MT_UNF_VCODEC_TYPE_H264:
            sample_common_printf("VideoType= H264\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG2:
            sample_common_printf("VideoType= MPEG2\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG4:
            sample_common_printf("VideoType= MPEG4\n");
            break;
        case MT_UNF_VCODEC_TYPE_HEVC:
            sample_common_printf("VideoType= HEVC\n");
            break;
        case MT_UNF_VCODEC_TYPE_VC1:
            sample_common_printf("VideoType= VC1\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP8:
            sample_common_printf("VideoType= VP8\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP9:
            sample_common_printf("VideoType= VP9\n");
            break;
        default:
            sample_common_printf("VideoType= ERROR\n");
        }
    sample_common_printf("isClearStream: %d, isEncrypt: %d.\n", pstProgInfo->stRecAttr.bIsClearStream, pstProgInfo->stRecAttr.stEncryptCfg.bDoCipher);
    sample_common_printf("------------------\n\n");
    MT_USLEEP(10*1000);

    return MT_SUCCESS;
}


mt_s32 MTADP_PVR_GetPorgInfo(PVR_PROG_INFO_S *pstProgInfo, const mt_char *pszPvrRecFile)
{
    MT_S32 ret;
    MT_U32 dataRead;
    PVR_PROG_INFO_S userData;
    mt_char attrfilepath[128]={0};

    sprintf(attrfilepath,"%s.attr",pszPvrRecFile);

    #if 1
    ret = MT_UNF_PVR_GetUsrDataInfoByFileName(attrfilepath, (MT_U8*)&userData,sizeof(PVR_PROG_INFO_S), &dataRead);
    sample_common_printf("file = %s\n",attrfilepath);
    if (MT_SUCCESS != ret){
        sample_common_printf("GetUsrDataInfoByFileName ERR:%#x.\n", ret);
        return ret;
    }
    memcpy(pstProgInfo, &(userData),  sizeof(PVR_PROG_INFO_S));
    #else
    pstProgInfo->stProgInfo.AElementNum=1;
    pstProgInfo->stProgInfo.AElementPid=771;
    pstProgInfo->stProgInfo.AudioType=HA_AUDIO_ID_AAC;
    pstProgInfo->stProgInfo.VElementNum=1;
    pstProgInfo->stProgInfo.VElementPid=774;
    pstProgInfo->stProgInfo.VideoType=MT_UNF_VCODEC_TYPE_MPEG2;
    pstProgInfo->stRecAttr.bIsClearStream=1;
    pstProgInfo->stRecAttr.stEncryptCfg.bDoCipher=0;
    #endif

    sample_common_printf("\n------------------\n");
    sample_common_printf("Get File Info:\n");
    if (pstProgInfo->stProgInfo.AElementNum > 0){
        sample_common_printf("Audio:\n");
        sample_common_printf("   PID = %#x\n",pstProgInfo->stProgInfo.AElementPid);
        switch (pstProgInfo->stProgInfo.AudioType)
        {
        case HA_AUDIO_ID_PCM:
            sample_common_printf("   Type= PCM\n");
            break;
        case HA_AUDIO_ID_MP2:
            sample_common_printf("   Type= MP2\n");
            break;
        case HA_AUDIO_ID_MP3:
            sample_common_printf("   Type= MP3\n");
            break;
        case HA_AUDIO_ID_AAC:
            sample_common_printf("   Type= AAC\n");
            break;
        case HA_AUDIO_ID_DRA:
            sample_common_printf("   Type= DRA\n");
            break;
        case HA_AUDIO_ID_WMA9STD:
            sample_common_printf("   Type= WMA9STD\n");
            break;
        case HA_AUDIO_ID_DOLBY_PLUS:
            sample_common_printf("   Type= DOLBY_PLUS\n");
            break;
        case HA_AUDIO_ID_DOLBY_TRUEHD:
            sample_common_printf("   Type= DOLBY_TRUEHD\n");
            break;
        case HA_AUDIO_ID_DOLBY_CONVERT:
            sample_common_printf("   Type= DOLBY_CONVERT\n");
            break;
        case HA_AUDIO_ID_DTSHD:
            sample_common_printf("   Type= DTSHD\n");
            break;
        case HA_AUDIO_ID_AC3PASSTHROUGH:
            sample_common_printf("   Type= AC3PASSTHROUGH\n");
            break;
        case HA_AUDIO_ID_EAC3PASSTHROUGH:
            sample_common_printf("   Type= EAC3PASSTHROUGH\n");
            break;
        case HA_AUDIO_ID_DTSPASSTHROUGH:
            sample_common_printf("   Type= DTSPASSTHROUGH\n");
            break;
        default:
            sample_common_printf("   Type= ERROR\n");
        }
    }else{
        sample_common_printf("Audio: none\n");
    }
    if (pstProgInfo->stProgInfo.VElementNum > 0){
        sample_common_printf("Video:\n");
        sample_common_printf("   PID = %#x\n",pstProgInfo->stProgInfo.VElementPid);
        switch (pstProgInfo->stProgInfo.VideoType)
        {
        case MT_UNF_VCODEC_TYPE_AVS:
            sample_common_printf("   Type= AVS\n");
            break;
        case MT_UNF_VCODEC_TYPE_H264:
            sample_common_printf("   Type= H264\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG2:
            sample_common_printf("   Type= MPEG2\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG4:
            sample_common_printf("   Type= MPEG4\n");
            break;
        case MT_UNF_VCODEC_TYPE_HEVC:
            sample_common_printf("   Type= HEVC\n");
            break;
        case MT_UNF_VCODEC_TYPE_VC1:
            sample_common_printf("   Type= VC1\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP8:
            sample_common_printf("   Type= VP8\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP9:
            sample_common_printf("   Type= VP9\n");
            break;
        default:
            sample_common_printf("   Type= ERROR\n");
        }
    }else{
        sample_common_printf("Video: none\n\n");
    }

    sample_common_printf("isClearStream: %d, isEncrypt: %d.\n", pstProgInfo->stRecAttr.bIsClearStream, pstProgInfo->stRecAttr.stEncryptCfg.bDoCipher);
    sample_common_printf("------------------\n");

    return MT_SUCCESS;
}

mt_s32 MTADP_PVR_checkIdx(char *pfileName)
{
#if 0
    int  i;
    PVR_INDEX_ENTRY_S entry;
    mt_u64 lastOffset = 0;
    mt_u32 sizeCalc;
    mt_u32 scbuf[10];
    mt_u32 LastSize=0;
#endif
    int ret = 0;
    int pos;
    int readNum;
    PVR_IDX_HEADER_INFO_S headInfo;
    FILE *fpIndex;
    int fileTs;

    char indexName[256];
    sprintf(indexName, "%s.idx", pfileName);

    fpIndex = fopen(indexName, "rb");
    if (NULL == fpIndex)
    {
        sample_common_printf("can't open file %s to read!\n", indexName);
        return 2;
    }
//    fileTs = open(pfileName, O_RDONLY | O_LARGEFILE);
    fileTs = open(pfileName, O_RDONLY);
    if (fileTs == -1)
    {
        sample_common_printf("can't open ts file %s to read!\n", indexName);
        fclose(fpIndex);
        return 2;
    }
    fseek(fpIndex, 0, SEEK_END);
    pos = ftell(fpIndex);
    rewind(fpIndex);

    readNum = fread(&headInfo, 1, sizeof(PVR_IDX_HEADER_INFO_S), fpIndex);
    if (readNum != sizeof(PVR_IDX_HEADER_INFO_S))
    {
            perror("read failed:");
            sample_common_printf("read head failed: want%ld, get:%d\n", sizeof(PVR_IDX_HEADER_INFO_S), readNum);
            fclose(fpIndex);
            return 3;
    }
    else
    {
        if (headInfo.u32StartCode == 0x5A5A5A5A)
        {
            sample_common_printf("This index file has head info: head size=%u, fileSize=%llu\n", headInfo.u32HeaderLen, headInfo.u64ValidSize);
            sample_common_printf("IndexEntrySize: %ld, index file size:%d, headifo len:%d\n",
                     sizeof(PVR_INDEX_ENTRY_S), pos, headInfo.u32HeaderLen);
            fseek(fpIndex, headInfo.u32HeaderLen, SEEK_SET);
        }
        else
        {
            sample_common_printf("This index file has NO head info\n");
            fclose(fpIndex);
            return 3;
        }
    }
    sample_common_printf("\nframe info:\n");
    sample_common_printf("====frame start:%d\n", headInfo.stCycInfo.u32StartFrame);
    sample_common_printf("====frame end:  %d\n", headInfo.stCycInfo.u32EndFrame);
    sample_common_printf("====frame last: %d\n", headInfo.stCycInfo.u32LastFrame);

    close(fileTs);
    fclose(fpIndex);
    if (ret)
    {
        sample_common_printf("\n------------End of file. index check failed, err:%d-------\n", ret);
    }
    else
    {
        sample_common_printf("\n------------End of file. index check ok-------\n");
    }
    return ret;
}
mt_s32 MTADP_PVR_RecCopy(char *path, PMT_COMPACT_PROG *pstProgInfo, mt_u32 u32DemuxID,
            MT_BOOL bRewind, MT_BOOL bDoCipher, mt_u64 maxSize, mt_u32 *pRecChn,mt_u32 pRecChnSrc,MT_BOOL bDIO)
{
    mt_u32 recChn;
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_PVR_REC_ATTR_S   attr;
    mt_u32                  VidPid;
    mt_u32                  AudPid = 0;
    PVR_PROG_INFO_S         fileInfo;
    mt_char                 szFileName[PVR_MAX_FILENAME_LEN];
    memset(&attr, 0 , sizeof(MT_UNF_PVR_REC_ATTR_S));
    attr.u32DemuxID    = u32DemuxID;
#if 1 //add by yuwu
    sprintf(szFileName, "tms_v%d_a%d.ts",
                        pstProgInfo->VElementPid,
                        pstProgInfo->AElementPid);

    sprintf(attr.szFileName, "%s/", path);
    /*ret= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
    if (ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return ret;
    }*/
    sample_common_printf("MTADP_PVR_RecStart  >>>>-->>> pstProgInfo->PmtPid=%x, pstProgInfo->AElementPid=%x,pstProgInfo->VElementPid=%x\n",
                pstProgInfo->PmtPid, pstProgInfo->AElementPid, pstProgInfo->VElementPid);
    strcat(attr.szFileName, szFileName);
    sample_common_printf("attr.szFileName = %s\n",attr.szFileName);
    if((pstProgInfo->VElementPid==0) || (pstProgInfo->VElementPid==0x1fff)){
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_AUDIO;
        attr.enIndexVidType = MT_UNF_VCODEC_TYPE_BUTT;
        attr.u32IndexPid = pstProgInfo->AElementPid;
        printf("+++++audio\n");
    }else{
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_VIDEO;
        attr.enIndexVidType = pstProgInfo->VideoType;
        attr.u32IndexPid = pstProgInfo->VElementPid;
        printf("+++++video\n");
    }
    attr.u32IdxBufSize= PVR_STUB_IDX_SHM_SIZE;
    attr.u32FileNameLen = strlen(attr.szFileName);
    attr.u32ScdBufSize = PVR_STUB_SC_BUF_SZIE;
    attr.u32DavBufSize = PVR_STUB_TSDATA_SIZE;
    attr.enStreamType  = MT_UNF_PVR_STREAM_TYPE_TS;
    attr.bRewind = bRewind;
    attr.u64MaxFileSize= maxSize;//maxSize;//source;
    attr.u64MaxTimeInMs= 0;
    attr.bIsClearStream = MT_TRUE;
    attr.u32UsrDataInfoSize = 0;//sizeof(PVR_PROG_INFO_S) + 100;/*the one in index file is a multipleit of 40 bytes*//*CNcomment:索引文件里是40个字节对齐的*/
    //attr.stEncryptCfg.bDoCipher = bDoCipher;
    attr.u32DIO=bDIO;
    #ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
        attr.stEncryptCfg.bDoCipher = (bDoCipher?MT_TRUE:MT_FALSE);
        if(attr.stEncryptCfg.bDoCipher){    //enable aes crypto
            int i=0;
            attr.stEncryptCfg.u32KeyLen = 16;
            attr.stEncryptCfg.enType = MT_CIPHER_ALG_AES;
            for(i=0;i<attr.stEncryptCfg.u32KeyLen;i++){
                attr.stEncryptCfg.au8Key[i]=10+i;
            }
            attr.bSupportAdvCa=MT_UNF_PVR_REC_DEFALT;
        }
    #else
        attr.stEncryptCfg.bDoCipher = MT_FALSE;
        if(bDoCipher){
            sample_common_printf("error: config crypto but CONFIG_MT_PVR_CIPHER_SUPPORT==0!\n");
        }
    #endif

    sample_common_printf("DMX: MT_UNF_PVR_RecCreateCh........................................................\n");
    ret = MT_UNF_PVR_RecCopyChn(&recChn, pRecChnSrc,&attr);
    if (MT_SUCCESS != ret)
    {
        sample_common_printf("DMX: MT_UNF_PVR_RecCreateCh........................................................ret=%x\n",ret);
        return ret;
    }
    sample_common_printf("MT_UNF_PVR_RecCreateChn, recChn:%d\n", recChn);
    if(bDoCipher){
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Crypto_WriteCallback, NULL);
    }else{
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Normal_WriteCallback, NULL);
    }
#endif
    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, 0, MT_UNF_DMX_CHAN_TYPE_SEC);
    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->PmtPid, MT_UNF_DMX_CHAN_TYPE_SEC);

    if (pstProgInfo->AElementNum > 0)
    {
        AudPid  = pstProgInfo->AElementPid;
        MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, AudPid, MT_UNF_DMX_CHAN_TYPE_AUD);
    }

    if (pstProgInfo->VElementNum > 0 )
    {
        VidPid = pstProgInfo->VElementPid;
        MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, VidPid, MT_UNF_DMX_CHAN_TYPE_VID);
        attr.u32IndexPid   = VidPid;
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_VIDEO;
        attr.enIndexVidType = pstProgInfo->VideoType;
    }
    else
    {
        attr.u32IndexPid   = AudPid;
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_AUDIO;
        attr.enIndexVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }
#if 0 //add by yuwu
    sprintf(szFileName, "rec_v%d_a%d.ts",
                        pstProgInfo->VElementPid,
                        pstProgInfo->AElementPid);

    sprintf(attr.szFileName, "%s/", path);

    strcat(attr.szFileName, szFileName);

    attr.u32FileNameLen = strlen(attr.szFileName);
    attr.u32ScdBufSize = PVR_STUB_SC_BUF_SZIE;
    attr.u32DavBufSize = PVR_STUB_TSDATA_SIZE;
    attr.enStreamType  = MT_UNF_PVR_STREAM_TYPE_TS;
    attr.bRewind = bRewind;
    attr.u64MaxFileSize= maxSize;//source;
    attr.u64MaxTimeInMs= 0;
    attr.bIsClearStream = MT_TRUE;
    attr.u32UsrDataInfoSize = sizeof(PVR_PROG_INFO_S) + 100;/*the one in index file is a multipleit of 40 bytes*//*CNcomment:索引文件里是40个字节对齐的*/

    attr.stEncryptCfg.bDoCipher = bDoCipher;
//    attr.stEncryptCfg.enType = MT_UNF_CIPHER_ALG_AES;
//    attr.stEncryptCfg.u32KeyLen = 16;          /*strlen(PVR_CIPHER_KEY)*/
    SAMPLE_RUN(MT_UNF_PVR_RecCreateChn(&recChn, &attr), ret);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }
#endif
    ret = MT_UNF_PVR_RecStartChn(recChn);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_RecDestroyChn(recChn);
        return ret;
    }

    fileInfo.u32MagicNumber=PVR_PROG_INFO_MAGIC;
    memcpy(&(fileInfo.stProgInfo), pstProgInfo, sizeof(PMT_COMPACT_PROG));
    memcpy(&(fileInfo.stRecAttr), &attr, sizeof(fileInfo.stRecAttr));
#if 1 //add by yuwu

    ret = MTADP_PVR_SavePorgInfo(&fileInfo, attr.szFileName);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_RecStopChn(recChn);
        MT_UNF_PVR_RecDestroyChn(recChn);
        return ret;
    }
#endif
    *pRecChn = recChn;

    return MT_SUCCESS;
}
mt_s32 MTADP_PVR_RecStart(char *path, PMT_COMPACT_PROG *pstProgInfo, mt_u32 u32DemuxID,
            MT_BOOL bRewind, MT_BOOL bDoCipher, mt_u64 maxSize, mt_u32 *pRecChn, MT_BOOL bDIO, mt_pvr_rec_cipher_t rec_cipher, MT_BOOL bInfo)
{
    mt_u32 recChn;
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_PVR_REC_ATTR_S   attr;
    mt_u32                  VidPid;
    mt_u32                  AudPid = 0;
    PVR_PROG_INFO_S         fileInfo;
    mt_char                 szFileName[PVR_MAX_FILENAME_LEN];
    int i = 0;
    int j = 0;

    memset(&attr, 0 , sizeof(MT_UNF_PVR_REC_ATTR_S));
    attr.u32DemuxID    = u32DemuxID;
#if 1 //add by yuwu
    if(0 == maxSize)
    {
        sprintf(szFileName, "rec_v%d_a%d.ts", pstProgInfo->VElementPid, pstProgInfo->AElementPid);
    }
    else
    {
        sprintf(szFileName, "tms_v%d_a%d.ts", pstProgInfo->VElementPid, pstProgInfo->AElementPid);
    }
    sprintf(attr.szFileName, "%s/", path);
    /*ret= MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
    if (ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return ret;
    }*/
    sample_common_printf("MTADP_PVR_RecStart  >>>>-->>> pstProgInfo->PmtPid=%x, pstProgInfo->AElementPid=%x,pstProgInfo->VElementPid=%x\n",
                pstProgInfo->PmtPid, pstProgInfo->AElementPid, pstProgInfo->VElementPid);
    strcat(attr.szFileName, szFileName);
    sample_common_printf("attr.szFileName = %s\n",attr.szFileName);
    if((pstProgInfo->VElementPid==0) || (pstProgInfo->VElementPid==0x1fff)){
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_AUDIO;
        attr.enIndexVidType = MT_UNF_VCODEC_TYPE_BUTT;
        attr.u32IndexPid = pstProgInfo->AElementPid;
        printf("+++++audio\n");
    }else{
        attr.enIndexType   = MT_UNF_PVR_REC_INDEX_TYPE_VIDEO;
        attr.enIndexVidType = pstProgInfo->VideoType;
        attr.u32IndexPid = pstProgInfo->VElementPid;
        printf("+++++video\n");
    }
    attr.u32IdxBufSize= PVR_STUB_IDX_SHM_SIZE;
    attr.u32FileNameLen = strlen(attr.szFileName);
    attr.u32ScdBufSize = PVR_STUB_SC_BUF_SZIE;
    attr.u32DavBufSize = (PVR_STUB_TSDATA_SIZE*2);
    attr.enStreamType  = MT_UNF_PVR_STREAM_TYPE_TS;
    attr.bRewind = bRewind;
    attr.u64MaxFileSize= maxSize;//maxSize;//source;
    attr.u64MaxTimeInMs= 0;
    attr.bIsClearStream = MT_TRUE;
    attr.u32UsrDataInfoSize = 0;//sizeof(PVR_PROG_INFO_S) + 100;/*the one in index file is a multipleit of 40 bytes*//*CNcomment:索引文件里是40个字节对齐的*/
    //attr.stEncryptCfg.bDoCipher = bDoCipher;
    attr.u32DIO=bDIO;
    if(attr.enIndexType == MT_UNF_PVR_REC_INDEX_TYPE_AUDIO)
    {
        attr.u32DIO = MT_FALSE;
    }

#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
        attr.stEncryptCfg.bDoCipher = (bDoCipher?MT_TRUE:MT_FALSE);
        if(attr.stEncryptCfg.bDoCipher)
        {    //enable aes crypto
            if(rec_cipher.bkey)
            {
                sample_common_printf("Use user key \n");
                attr.stEncryptCfg.u32KeyLen = rec_cipher.u32KeyLen;
                attr.stEncryptCfg.enType = rec_cipher.enType;
                for(i=0;i<attr.stEncryptCfg.u32KeyLen;i++)
                {
                    attr.stEncryptCfg.au8Key[i]= rec_cipher.au8Key[i];
                    sample_common_printf("%d ", attr.stEncryptCfg.au8Key[i]);
                }
                sample_common_printf("\n");
                attr.bSupportAdvCa = MT_UNF_PVR_REC_DEFALT;
            }
            else
            {
                sample_common_printf("Use default key \n");
                attr.stEncryptCfg.u32KeyLen = 16;
                attr.stEncryptCfg.enType = MT_CIPHER_ALG_AES;
                for(i=0;i<attr.stEncryptCfg.u32KeyLen;i++)
                {
                    attr.stEncryptCfg.au8Key[i]=10+i;
                    sample_common_printf("%d ", attr.stEncryptCfg.au8Key[i]);
                }
                sample_common_printf("\n");
                attr.bSupportAdvCa = MT_UNF_PVR_REC_DEFALT;

            }

        }
        else
        {
            sample_common_printf("bDoCipher is MT_FALSE! \n");
        }
#else
        attr.stEncryptCfg.bDoCipher = MT_FALSE;
        if(bDoCipher){
            sample_common_printf("error: config crypto but CONFIG_MT_PVR_CIPHER_SUPPORT==0!\n");
        }
#endif

    sample_common_printf("DMX: MT_UNF_PVR_RecCreateCh........................................................\n");
    ret = MT_UNF_PVR_RecCreateChn(&recChn, &attr);
    if (MT_SUCCESS != ret)
    {
        sample_common_printf("DMX: MT_UNF_PVR_RecCreateCh........................................................ret=%x\n",ret);
        return ret;
    }
    sample_common_printf("MT_UNF_PVR_RecCreateChn, recChn:%d\n", recChn);
    if(bDoCipher){
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Crypto_WriteCallback, NULL);
    }else{
        MT_UNF_PVR_RegisterExtraCallback(recChn, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, (ExtraCallBack)MTADP_PVR_Normal_WriteCallback, NULL);
    }
#endif
    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, 0, MT_UNF_DMX_CHAN_TYPE_SEC);
    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->PmtPid, MT_UNF_DMX_CHAN_TYPE_SEC);

    if(MT_TRUE == bInfo)
    {
        if (pstProgInfo->AElementNum > 0)
        {
            for(j = 0; j < pstProgInfo->AElementNum; j++)
            {
                AudPid  = pstProgInfo->Audioinfo[j].u16AudioPid;
                MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, AudPid, MT_UNF_DMX_CHAN_TYPE_AUD);
            }

        }

        if (pstProgInfo->VElementNum > 0 )
        {
            VidPid = pstProgInfo->VElementPid;
            MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, VidPid, MT_UNF_DMX_CHAN_TYPE_VID);

        }

        if (pstProgInfo->u16TtxNum > 0 )
        {
            for(j = 0; j < pstProgInfo->u16TtxNum; j++)
            {
                MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->stTtxInfo[j].u16TtxPID, MT_UNF_DMX_CHAN_TYPE_PES);
            }
        }

        /* 1: dvb subtitle 2: stc subtitle from mt_unf_subt.h*/
        if (1 == pstProgInfo->SubtType)
        {
            if (pstProgInfo->u16SubtitlingNum > 0 )
            {
                for(j = 0; j < pstProgInfo->u16SubtitlingNum; j++)
                {
                    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->SubtitingInfo[j].u16SubtitlingPID, MT_UNF_DMX_CHAN_TYPE_PES);
                }
            }
        }
        else if (2 == pstProgInfo->SubtType)
        {
            if (pstProgInfo->u16SCTESubtNum > 0 )
            {
                for(j = 0; j < pstProgInfo->u16SCTESubtNum; j++)
                {
                    MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->stSCTESubtInfo[j].u16SCTESubtPID, MT_UNF_DMX_CHAN_TYPE_PES);
                }
            }
        }

        if (pstProgInfo->u16ClosedCaptionNum > 0 )
        {
            for(j = 0; j < pstProgInfo->u16ClosedCaptionNum; j++)
            {

                MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, pstProgInfo->u16ARIBCCPid, MT_UNF_DMX_CHAN_TYPE_PES);
            }
        }
        sample_common_printf("Record all program information, bInfo = %d \n", bInfo);
    }
    else
    {
        if (pstProgInfo->AElementNum > 0)
        {
             AudPid  = pstProgInfo->AElementPid;
             MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, AudPid, MT_UNF_DMX_CHAN_TYPE_AUD);

        }

        if (pstProgInfo->VElementNum > 0 )
        {
            VidPid = pstProgInfo->VElementPid;
            MTADP_PVR_RecAddPid(recChn, attr.u32DemuxID, VidPid, MT_UNF_DMX_CHAN_TYPE_VID);

        }
        sample_common_printf("only record audio/vidio info, bInfo = %d \n", bInfo);
    }

    ret = MT_UNF_PVR_RecStartChn(recChn);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_RecDestroyChn(recChn);
        return ret;
    }

    fileInfo.u32MagicNumber=PVR_PROG_INFO_MAGIC;
    memcpy(&(fileInfo.stProgInfo), pstProgInfo, sizeof(PMT_COMPACT_PROG));
    memcpy(&(fileInfo.stRecAttr), &attr, sizeof(fileInfo.stRecAttr));
#if 1 //add by yuwu

    ret = MTADP_PVR_SavePorgInfo(&fileInfo, attr.szFileName);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_RecStopChn(recChn);
        MT_UNF_PVR_RecDestroyChn(recChn);
        return ret;
    }
#endif
    *pRecChn = recChn;

    return MT_SUCCESS;
}

mt_s32 MTADP_PVR_RecStop(mt_u32 u32RecChnID)
{
    mt_s32 ret = MT_FAILURE;
    mt_s32 ret2 = MT_FAILURE;
    MT_UNF_PVR_REC_ATTR_S recAttr;


    ret2 = MT_UNF_PVR_RecGetChn(u32RecChnID, &recAttr) ;
    MT_UNF_PVR_UnRegisterExtraCallBack(u32RecChnID,MT_UNF_PVR_EXTRA_WRITE_CALLBACK);
    ret = MT_UNF_PVR_RecStopChn(u32RecChnID) ;
    ret = MT_UNF_PVR_RecDestroyChn(u32RecChnID) ;
    if (MT_SUCCESS == ret2)
    {
        if (!recAttr.stEncryptCfg.bDoCipher)
        {
            //SAMPLE_RUN(MTADP_PVR_checkIdx(recAttr.szFileName), ret);
        }
    }

    //PVR_RecDelAllPid(PVR_DMX_ID_REC) ;

    return ret;
}


mt_s32 MTADP_PVR_SwitchDmxSource(mt_u32 dmxId, mt_u32 protId)
{
    mt_s32 ret = MT_FAILURE;

    MT_UNF_DMX_DetachTSPort(dmxId);

    ret = MT_UNF_DMX_AttachTSPort(dmxId, protId);

    return ret;
}

static mt_s32 MTADP_PVR_SetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_u32                  VidPid;
    mt_u32                  AudPid;

    MT_UNF_VCODEC_ATTR_S        VdecAttr;
    MT_UNF_ACODEC_ATTR_S        AdecAttr;
    mt_s32                  Ret = MT_FAILURE;
    MT_UNF_VCODEC_TYPE_E    enVidType;
    mt_u32                  u32AudType;


    MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    if(pProgInfo == NULL)
    {
        sample_common_printf("=====pProgInfo == NULL=====\n");
        return Ret;
    }
    if (pProgInfo->VElementNum > 0 )
    {
        VidPid = pProgInfo->VElementPid;
        enVidType = pProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if (pProgInfo->AElementNum > 0)
    {
        AudPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    sample_common_printf("VidPid=%x, AudPid=%x\n",VidPid,AudPid);
    if (VidPid != INVALID_TSPID)
    {
        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            VdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            VdecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            VdecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = MT_UNF_VCODEC_UNBLANK_STABLE;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;
        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if (Ret != MT_SUCCESS)
        {
            sample_common_printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
            return Ret;
        }

        Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if (Ret != MT_SUCCESS)
        {
            sample_common_printf("call MTADP_AVPlay_SetVdecAttr failed.\n");
            return Ret;
        }
    }

    if (AudPid != INVALID_TSPID)
    {
        Ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if (MT_SUCCESS != Ret)
        {
            sample_common_printf("MTADP_AVPlay_SetAdecAttr failed:%#x\n",Ret);
            return Ret;
        }
    }
    if(1){//insert pts
        if ((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID)) {
            MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync;
            DmxAvsync.VdecType = enVidType;
            DmxAvsync.AdecType = u32AudType;
            DmxAvsync.AvsyncFlage = 1; // 1--insert pts 0--do not insert pts
            MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        }
    }

    return MT_SUCCESS;
}



/*set audio and video PID attribution,start to play*//*CNcomment:设置音视频PID属性,开始播放*/
mt_s32 MTADP_PVR_StartLivePlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    mt_u32 ret = MT_SUCCESS;
    mt_u32 pid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = {0};
    MT_UNF_SYNC_ATTR_S stSyncAttr = {0};
    MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
    MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts = {0};

    MTADP_PVR_SetAvplayPidAndCodecType(hAvplay, pProgInfo);

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        sample_common_printf("has no audio stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        sample_common_printf("has no video stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }

    if ((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /*enable vo frame rate detect*//*CNcomment:使能VO自动帧率检测*/
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr))
        {
            sample_common_printf("set frame to VO fail.\n");
            return MT_FAILURE;
        }

        /*enable avplay A/V sync*//*CNcomment:使能avplay音视频同步*/
        if (MT_SUCCESS != MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
        {
            sample_common_printf("get avplay sync attr fail!\n");
            return MT_FAILURE;
        }

        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;

        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
        {
            sample_common_printf("set avplay sync attr fail!\n");
            return MT_FAILURE;
        }
    }

    /*start to play audio and video*//*CNcomment:开始音视频播放*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    /*set avplay trick mode to normal*//*CNcomment:设置avplay特技模式为正常*/
    if (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID)
    {
        stTplayOpts.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
        stTplayOpts.u32SpeedInteger = 1;
        stTplayOpts.u32SpeedDecimal = 0;
        stVdecCmdPara.u32CmdID = MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD;
        stVdecCmdPara.pPara = &stTplayOpts;
        ret = MT_UNF_AVPLAY_Invoke(hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&stVdecCmdPara);
        if (MT_SUCCESS != ret)
        {
            sample_common_printf("Resume Avplay trick mode to normal fail.\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_PVR_StopLivePlay(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option;

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    option.u32TimeoutMs = 0;

    sample_common_printf("stop live play ...\n");

    /*stop playing audio and video*//*CNcomment:停止音视频设备*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}


/*start playback*//*CNcomment:开始回放*/
mt_s32 MTADP_PVR_StartPlayBack(const mt_char *pszFileName, mt_u32 *pu32PlayChn, mt_handle hAvplay)
{
    mt_s32 ret = MT_FAILURE;
    mt_u32 playChn;
    PVR_PROG_INFO_S        fileInfo;
    MT_UNF_PVR_PLAY_ATTR_S attr;
    mt_u32 apid = 0;
    mt_u32 vpid = 0;
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0;
//    SAMPLE_CheckNullPTR(pszFileName);
//    SAMPLE_CheckNullPTR(pu32PlayChn);
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = {0};
    MT_UNF_SYNC_ATTR_S stSyncAttr = {0};
    MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts = {0};
    MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
    PMT_COMPACT_PROG stProgInfoTmp;

    ret=MTADP_PVR_GetPorgInfo(&fileInfo, pszFileName);
    if (MT_SUCCESS != ret){
        sample_common_printf("Can NOT get prog INFO, can't play.\n");
        return -1;
    }else{
        memcpy(&stProgInfoTmp,&fileInfo.stProgInfo,sizeof(PMT_COMPACT_PROG));
        sample_common_printf("play:vnum=0x%x\n",stProgInfoTmp.VElementNum);
        sample_common_printf("play:vpid=0x%x\n",stProgInfoTmp.VElementPid);
        sample_common_printf("play:vtpy=0x%x\n",stProgInfoTmp.VideoType);
        sample_common_printf("play:anum=0x%x\n",stProgInfoTmp.AElementNum);
        sample_common_printf("play:apid=0x%x\n",stProgInfoTmp.AElementPid);
        sample_common_printf("play:atype=0x%x\n",stProgInfoTmp.AudioType);
        sample_common_printf("play:file=%s\n",pszFileName);

        MTADP_PVR_SetAvplayPidAndCodecType(hAvplay, &(stProgInfoTmp));
        
        memset(&attr,0,sizeof(MT_UNF_PVR_PLAY_ATTR_S));//fixed u32StartTimeOffset is  random value, playback from end
        memcpy(attr.szFileName, pszFileName, strlen(pszFileName) + 1);
            attr.u32FileNameLen = strlen(pszFileName);
        attr.enStreamType = fileInfo.stRecAttr.enStreamType;
        attr.bIsClearStream = fileInfo.stRecAttr.bIsClearStream;
        attr.bSupportAdvCa = fileInfo.stRecAttr.bSupportAdvCa;
        sample_common_printf("StreamType:%x\n",attr.enStreamType);
        sample_common_printf("IsClear:%x\n",attr.bIsClearStream);
        if ((1==fileInfo.stRecAttr.stEncryptCfg.bDoCipher) && (ret==MT_SUCCESS)){
            memcpy(&(attr.stDecryptCfg), &(fileInfo.stRecAttr.stEncryptCfg), sizeof(MT_UNF_PVR_CIPHER_S));
            sample_common_printf("cipher info:\n");
            sample_common_printf("enType:%d\n", attr.stDecryptCfg.enType);
        }else{
            sample_common_printf("cipher info: not encrypt\n");
            attr.stDecryptCfg.bDoCipher = MT_FALSE;
        }
    }
    sample_common_printf("u32StartTimeOffset=%d\n",attr.u32StartTimeOffset);
    
    ret = MT_UNF_DMX_CreateTSBuffer(PVR_DMX_PORT_ID_PLAYBACK, 0x200000, &g_hTsBufForPlayBack);
    if (ret != MT_SUCCESS)
    {
        MT_UNF_DMX_DetachTSPort(PVR_DMX_PORT_ID_PLAYBACK);
        MT_UNF_DMX_DeInit();
        return ret;
    }
    /*create new play channel*//*CNcomment:申请新的播放通道*/
    ret = MT_UNF_PVR_PlayCreateChn(&playChn, &attr, hAvplay, g_hTsBufForPlayBack);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }
    if(attr.stDecryptCfg.bDoCipher){
        MT_UNF_PVR_RegisterExtraCallback(playChn, MT_UNF_PVR_EXTRA_READ_CALLBACK, (ExtraCallBack)MTADP_PVR_Crypto_ReadCallback, NULL);
    }else{
        MT_UNF_PVR_RegisterExtraCallback(playChn, MT_UNF_PVR_EXTRA_READ_CALLBACK, (ExtraCallBack)MTADP_PVR_Normal_ReadCallback, NULL);
    }
    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &apid);
    if ((MT_SUCCESS != ret) || (0x1fff == apid))
    {
        sample_common_printf("has no audio stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &vpid);
    if ((MT_SUCCESS != ret) || (0x1fff == vpid))
    {
        sample_common_printf("has no video stream!\n");
    }
    else
    {
        enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }

    /*enable avplay A/V sync*//*CNcomment:使能avplay音视频同步*/
    if (MT_SUCCESS != MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
    {
        sample_common_printf("get avplay sync attr fail!\n");
        return MT_FAILURE;
    }
    stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
    stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
    stSyncAttr.u32PreSyncTimeoutMs = 1000;
    stSyncAttr.bQuickOutput = MT_FALSE;

    if ((0x1fff != apid) && (0x1fff != vpid))
    {
        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
        {
            sample_common_printf("set avplay sync attr fail!\n");
            return MT_FAILURE;
        }
    }

    ret = MT_UNF_PVR_PlayStartChn(playChn);
    if (MT_SUCCESS != ret)
    {
        MT_UNF_PVR_PlayDestroyChn(playChn);
        return ret;
    }

    if (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID)
    {
        /*enable vo frame rate detect*//*CNcomment:使能VO自动帧率检测*/
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr))
        {
            sample_common_printf("set frame to VO fail.\n");
            return MT_FAILURE;
        }
    }


    /*set avplay trick mode to normal*//*CNcomment:设置avplay特技模式为正常*/
    if (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID)
    {
        stTplayOpts.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
        stTplayOpts.u32SpeedInteger = 1;
        stTplayOpts.u32SpeedDecimal = 0;
        stVdecCmdPara.u32CmdID = MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD;
        stVdecCmdPara.pPara = &stTplayOpts;
        ret = MT_UNF_AVPLAY_Invoke(hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&stVdecCmdPara);
        if (MT_SUCCESS != ret)
        {
            sample_common_printf("Resume Avplay trick mode to normal fail.\n");
            return MT_FAILURE;
        }
    }

    *pu32PlayChn = playChn;
    g_hPvrPlayChn = playChn;

    return MT_SUCCESS;
}

mt_s32 MTADP_PVR_StopPlayLive(mt_handle hAvplay)
{
    MT_UNF_AVPLAY_STOP_OPT_S option;

    option.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    option.u32TimeoutMs = 0;

    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

/*stop playback*//*CNcomment: 停止回放 */
mt_void MTADP_PVR_StopPlayBack(mt_u32 playChn)
{
    MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
    MT_UNF_PVR_PLAY_ATTR_S PlayAttr;

    stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopOpt.u32TimeoutMs = 0;
    MT_UNF_PVR_PlayGetChn(playChn,&PlayAttr);
    MT_UNF_PVR_PlayStopChn(playChn, &stopOpt);
    MT_UNF_PVR_UnRegisterExtraCallBack(playChn,MT_UNF_PVR_EXTRA_READ_CALLBACK);
    MT_UNF_PVR_PlayDestroyChn(playChn);
    g_hPvrPlayChn = MT_INVALID_HANDLE;
    MT_UNF_DMX_DestroyTSBuffer(g_hTsBufForPlayBack);
}

mt_s32 MTADP_PVR_AvplayInit(mt_handle hWin, mt_handle *phAvplay, mt_handle* phSoundTrack)
{
    mt_s32                  Ret = MT_FAILURE;
    mt_handle               hAvplay;
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr;
    MT_UNF_SYNC_ATTR_S          SyncAttr;
    MT_UNF_AUDIOTRACK_ATTR_S  stTrackAttr;

    if (phSoundTrack == NULL)
    {
        return MT_FAILURE;
    }

    Ret = MTADP_AVPlay_RegADecLib();
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MTADP_AVPlay_RegADecLib failed.\n");
        return Ret;
    }


    Ret = MT_UNF_AVPLAY_Init();
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_Init failed.\n");
        return Ret;
    }

    Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    AvplayAttr.u32DemuxId = PVR_DMX_ID_LIVE;
/************************************************************************ add by yuwu  ****/
//    Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
    Ret = MT_UNF_DMX_AttachTSPort(PVR_DMX_ID_LIVE, MT_UNF_DMX_PORT_RAM_0);

    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }
 /************************************************************************ **************/
//#ifdef S_SUPPORT_4PLUS64
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
//#endif

    Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_Create failed.\n");
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_GetAttr failed.\n");
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    SyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
    SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
    SyncAttr.u32PreSyncTimeoutMs = 1000;
    SyncAttr.bQuickOutput = MT_FALSE;

    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        MT_UNF_AVPLAY_Destroy(hAvplay);
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
        MT_UNF_AVPLAY_Destroy(hAvplay);
        return Ret;
    }

    Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_VO_AttachWindow failed.\n");
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
        MT_UNF_AVPLAY_Destroy(hAvplay);
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_VO_SetWindowEnable failed.\n");
        MT_UNF_VO_DetachWindow(hWin, hAvplay);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
        MT_UNF_AVPLAY_Destroy(hAvplay);
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        return Ret;
    }
    Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, phSoundTrack);
    if (Ret != MT_SUCCESS)
    {
        MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
        MT_UNF_VO_DetachWindow(hWin, hAvplay);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
        MT_UNF_AVPLAY_Destroy(hAvplay);
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    Ret = MT_UNF_SND_Attach(*phSoundTrack, hAvplay);
    if (Ret != MT_SUCCESS)
    {
        MT_UNF_SND_DestroyTrack(*phSoundTrack);
        sample_common_printf("call MT_SND_Attach failed.\n");
        MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
        MT_UNF_VO_DetachWindow(hWin, hAvplay);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
        MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
        MT_UNF_AVPLAY_Destroy(hAvplay);
        MT_UNF_AVPLAY_DeInit();
        return Ret;
    }

    *phAvplay = hAvplay;
    return MT_SUCCESS;
}

mt_s32  MTADP_PVR_AvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{
    mt_s32                      Ret = MT_FAILURE;
    MT_UNF_AVPLAY_STOP_OPT_S    Stop;

    Stop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    Stop.u32TimeoutMs = 0;

    Ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &Stop);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_AVPLAY_Stop failed.\n");
    }

    MT_UNF_VO_SetWindowEnable(hWin,MT_FALSE);

    Ret = MT_UNF_SND_Detach(hSoundTrack, hAvplay);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_SND_Detach failed.\n");
    }

    Ret = MT_UNF_SND_DestroyTrack(hSoundTrack);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_SND_DestroyTrack failed.\n");
    }

    Ret = MT_UNF_VO_DetachWindow(hWin, hAvplay);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_VO_DetachWindow failed.\n");
    }

    Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    Ret = MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_AVPLAY_ChnClose failed.\n");
    }

    Ret = MT_UNF_AVPLAY_Destroy(hAvplay);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_AVPLAY_Destroy failed.\n");
    }

    Ret = MT_UNF_AVPLAY_DeInit();
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_AVPLAY_DeInit failed.\n");
    }

    Ret = MT_UNF_DMX_DetachTSPort(PVR_DMX_ID_LIVE);
    if (Ret != MT_SUCCESS )
    {
        sample_common_printf("call MT_UNF_DMX_DetachTSPort failed.\n");
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_PVR_RestoreAc4PlayAttrInfo(mt_handle hAvplay)
{
    mt_s32    ret = MT_SUCCESS;
    play_ac4_attr_info play_ac4_info;


    MTADP_AUD_GetAc4PlayAttrInfo(&play_ac4_info);
    if (!play_ac4_info.ac4_attr_enable)
    {
        sample_common_printf("no restore ac4 play attr info \n");
        return ret;
    }

    // restore ac4    attr info.
    ret = MT_UNF_AVPLAY_StopAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_StopAudDec failed.\n");
        return ret;
    }

    ret = MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);
    if (ret != MT_SUCCESS)
    {
        sample_common_printf("call MTADP_AUD_RestoreAc4PlayAttrInfo failed.\n");
    }

    ret = MT_UNF_AVPLAY_StartAudDec(hAvplay);
    if (ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_AVPLAY_StartAudDec failed.\n");
        return ret;
    }

    return ret;
}

mt_void MTADP_PVR_CallBack(mt_u32 u32ChnID, MT_UNF_PVR_EVENT_E EventType, mt_s32 s32EventValue, mt_void *args)
{
    MT_UNF_PVR_PLAY_MODE_S mode;
    MT_UNF_PVR_PLAY_POSITION_S stPos = { 0 };
    MT_HANDLE *hAvPlay = (MT_HANDLE *)args;

    sample_common_printf("==============call back================\n");

    if (EventType > MT_UNF_PVR_EVENT_REC_RESV)
    {
        sample_common_printf("====callback error!!!\n");
        return;
    }

    sample_common_printf("====channel     %d\n", u32ChnID);
    sample_common_printf("====event:%s    %d\n", MTADP_PVR_GetEventTypeStringByID(EventType), EventType);
    sample_common_printf("====event value %d\n", s32EventValue);

    if(EventType == MT_UNF_PVR_EVENT_REC_DMX_CREATE)
    {
        //as ca app could get the rec demux channel(s32EventValue) by this event
        sample_common_printf("==========rec dmx create channel=0x%lx======\n",(MT_HANDLE)s32EventValue);
    }

    if (EventType == MT_UNF_PVR_EVENT_PLAY_EOF)
    {
        sample_common_printf("==========play to end of file======\n");

        stPos.enPositionType = MT_UNF_PVR_PLAY_POS_TYPE_TIME;
        stPos.s64Offset = 0;
        stPos.s32Whence = SEEK_SET;
        MT_UNF_PVR_PlaySeek(u32ChnID, &stPos);
        mode.enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
        MT_UNF_PVR_PlayTPlay(u32ChnID, &mode);
        MTADP_PVR_RestoreAc4PlayAttrInfo(*hAvPlay);
    }
    if (EventType == MT_UNF_PVR_EVENT_PLAY_SOF)
    {
        sample_common_printf("==========play to start of file======\n");
        mode.enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
        MT_UNF_PVR_PlayTPlay(u32ChnID, &mode);
        MTADP_PVR_RestoreAc4PlayAttrInfo(*hAvPlay);
    }
    if (EventType == MT_UNF_PVR_EVENT_PLAY_ERROR)
    {
        sample_common_printf("==========play internal error, check if the disk is insert to the box======\n");
    }
    if (EventType == MT_UNF_PVR_EVENT_PLAY_REACH_REC)
    {
        sample_common_printf("==========play reach to record ======\n");
        mode.enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
        MT_UNF_PVR_PlayTPlay(u32ChnID, &mode);
        MTADP_PVR_RestoreAc4PlayAttrInfo(*hAvPlay);
    }

    if (EventType == MT_UNF_PVR_EVENT_REC_DISKFULL)
    {
        sample_common_printf("\n====disk full,  stop record=====\n\n");

        MT_UNF_PVR_RecStopChn(u32ChnID);
        g_bIsRecStop = MT_TRUE;
    }
    if (EventType == MT_UNF_PVR_EVENT_REC_ERROR)
    {
        sample_common_printf("======disk write error, please check if the disk is insert to the box.====\n");
    }
    if (EventType == MT_UNF_PVR_EVENT_REC_OVER_FIX)
    {
        sample_common_printf("\n======reach the fixed size.==========\n\n");
    }
    if (EventType == MT_UNF_PVR_EVENT_REC_REACH_PLAY)
    {
        sample_common_printf("\n======record reach to play.==========\n\n");
    }
    if (EventType == MT_UNF_PVR_EVENT_REC_DISK_SLOW)
    {
        sample_common_printf("======disk is too slow, the stream record would be error.====\n");
    }

    sample_common_printf("=======================================\n\n");

    return;
}

mt_s32 MTADP_PVR_RegisterCallBacks(MT_HANDLE         *hAvPlay)
{
    mt_s32 Ret = MT_FAILURE;

    Ret = MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_PLAY_EOF, MTADP_PVR_CallBack, hAvPlay);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_PLAY_SOF, MTADP_PVR_CallBack, hAvPlay);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_PLAY_ERROR, MTADP_PVR_CallBack, hAvPlay);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_PLAY_REACH_REC, MTADP_PVR_CallBack, hAvPlay);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_DISKFULL, MTADP_PVR_CallBack, NULL);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_OVER_FIX, MTADP_PVR_CallBack, NULL);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_DISK_SLOW, MTADP_PVR_CallBack, NULL);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_REACH_PLAY, MTADP_PVR_CallBack, NULL);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_ERROR, MTADP_PVR_CallBack, NULL);
    Ret |= MT_UNF_PVR_RegisterEvent(MT_UNF_PVR_EVENT_REC_DMX_CREATE, MTADP_PVR_CallBack, NULL);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_PVR_RegisterEvent failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}


mt_s32 MTADP_PVR_UnRegisterCallBacks(mt_void)
{
    mt_s32 Ret = MT_FAILURE;

    Ret = MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_PLAY_EOF);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_PLAY_SOF);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_PLAY_ERROR);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_PLAY_REACH_REC);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_DISKFULL);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_OVER_FIX);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_DISK_SLOW);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_REACH_PLAY);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_ERROR);
    Ret |= MT_UNF_PVR_UnRegisterEvent(MT_UNF_PVR_EVENT_REC_DMX_CREATE);
    if (Ret != MT_SUCCESS)
    {
        sample_common_printf("call MT_UNF_PVR_UnRegisterEvent failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_u8* MTADP_PVR_GetEventTypeStringByID(MT_UNF_PVR_EVENT_E eEventID)
{
    mt_u32 nNum = sizeof(g_stEventType)/sizeof(g_stEventType[0]);
    mt_u32 nIndex = 0;
    mt_u8* pszRet = MT_NULL;

    for(nIndex=0; nIndex<nNum; nIndex++)
    {
        if( eEventID == g_stEventType[nIndex].eEventID)
        {
            pszRet = g_stEventType[nIndex].szEventTypeName;
            break;
        }
    }

    if(nIndex == nNum)
    {
       pszRet = g_stEventType[nNum-1].szEventTypeName;
    }

    return pszRet;
}


typedef struct tagNET_STREAM_PARAM
{
    mt_u16   u16Port;
    mt_char  szIgmpAddr[16];
    pthread_t hNetStreamThread;
    MT_BOOL   bStopNetStreamThread;
}NET_STREAM_PARAM_S;

NET_STREAM_PARAM_S g_stNetThreadParam = {0};

// not used
#if 0
static mt_void * NetStream(mt_void *args)
{
    mt_s32              SocketFd = -1;
    struct sockaddr_in  ServerAddr;
    in_addr_t           IpAddr;
    struct ip_mreq      Mreq;
    mt_u32              AddrLen;

    MT_UNF_STREAM_BUF_S     StreamBuf;
    mt_u32              ReadLen;
    mt_u32              GetBufCount=0;
    mt_u32              ReceiveCount=0;
    mt_s32              Ret;
    mt_handle           hTsBuf = 0;

    NET_STREAM_PARAM_S *pstThreadParam = (NET_STREAM_PARAM_S*)args;

    if(pstThreadParam == NULL)
    {
        return NULL;
    }

    do
    {
        Ret = MT_UNF_DMX_CreateTSBuffer(PVR_DMX_PORT_ID_IP, 0x200000, &hTsBuf);
        if (Ret != MT_SUCCESS)
        {
            break;
        }

        SocketFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if (SocketFd < 0)
        {
            sample_common_printf("wait send TS to port %d.\n", PVR_DMX_PORT_ID_IP);
            break;
        }

        ServerAddr.sin_family = AF_INET;
        ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        ServerAddr.sin_port = htons(pstThreadParam->u16Port);

        if (bind(SocketFd,(struct sockaddr *)(&ServerAddr),sizeof(struct sockaddr_in)) < 0)
        {
            sample_common_printf("socket bind error [%d].\n", errno);
            break;;
        }

        IpAddr = inet_addr(pstThreadParam->szIgmpAddr);

        sample_common_printf("========================g_pszMultiAddr = %s, g_u16UdpPort=%u\n", pstThreadParam->szIgmpAddr, pstThreadParam->u16Port);
        if (IpAddr)
        {
            Mreq.imr_multiaddr.s_addr = IpAddr;
            Mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            if (setsockopt(SocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &Mreq, sizeof(struct ip_mreq)))
            {
                sample_common_printf("wait send TS to port %d.\n", PVR_DMX_PORT_ID_IP);
                break;
            }
        }

        AddrLen = sizeof(ServerAddr);

        while ( pstThreadParam->bStopNetStreamThread == MT_FALSE)
        {
            Ret = MT_UNF_DMX_GetTSBuffer(hTsBuf, 188*50, &StreamBuf, 0);
            if (Ret != MT_SUCCESS)
            {
                GetBufCount++;
                if(GetBufCount >= 10)
                {
                    sample_common_printf("########## TS come too fast! #########, Ret=%d\n", Ret);
                    GetBufCount=0;
                }

                MT_USLEEP(10000) ;
                continue;
            }
            GetBufCount=0;

            ReadLen = recvfrom(SocketFd, StreamBuf.pu8Data, StreamBuf.u32Size, 0,
                               (struct sockaddr *)&ServerAddr, &AddrLen);
            if (ReadLen <= 0)
            {
                ReceiveCount++;
                if (ReceiveCount >= 50)
                {
                    sample_common_printf("########## TS come too slow or net error! #########\n");
                    ReceiveCount = 0;
                }
            }
            else
            {
                ReceiveCount = 0;
                Ret = MT_UNF_DMX_PutTSBuffer(hTsBuf, ReadLen);
                if (Ret != MT_SUCCESS )
                {
                    sample_common_printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
                }
            }
        }
    }while(0);

    if (SocketFd != -1)
    {
        close(SocketFd);
        SocketFd = -1;
    }

    if (hTsBuf != 0)
    {
        MT_UNF_DMX_DestroyTSBuffer(hTsBuf);
        hTsBuf = 0;
    }

    return NULL;
}

static mt_u32 MTADP_PVR_StartNetStream(mt_char* pszIgmpAddr, mt_u16 u16Port)
{
    if (pszIgmpAddr == NULL)
    {
        return MT_FAILURE;
    }

    if (g_stNetThreadParam.hNetStreamThread == 0)
    {
        g_stNetThreadParam.bStopNetStreamThread = MT_FALSE;
        memset(g_stNetThreadParam.szIgmpAddr, 0, sizeof(g_stNetThreadParam.szIgmpAddr));
        memcpy(g_stNetThreadParam.szIgmpAddr, pszIgmpAddr, sizeof(g_stNetThreadParam.szIgmpAddr));

        g_stNetThreadParam.u16Port     = u16Port;
        pthread_create(&g_stNetThreadParam.hNetStreamThread, MT_NULL, NetStream, &g_stNetThreadParam);
    }

    return MT_SUCCESS;
}

static mt_u32 MTADP_PVR_StopNetStream(void)
{
    if (g_stNetThreadParam.hNetStreamThread != 0)
    {
        g_stNetThreadParam.bStopNetStreamThread = MT_TRUE;

        pthread_join(g_stNetThreadParam.hNetStreamThread, MT_NULL);
    }

    return MT_FAILURE;
}

#endif
