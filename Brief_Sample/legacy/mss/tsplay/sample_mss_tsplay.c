/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"
#include "mt_unf_cipher_v2.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
//#include "mt_cmdline.h"


//#define MT_SAMPLE_APP

/***************************** Macro Definition ******************************/

#define MT_SAMPLE_TSPLAY_DEBUG

#ifdef MT_SAMPLE_TSPLAY_DEBUG

#define MT_TSPLAY_PRINT   printf
#else

#define MT_TSPLAY_PRINT

#endif

#define SAMPLE_TSPLAY_FUNCTION_ENTER()	    MT_TSPLAY_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TSPLAY_FUNCTION_EXIT()		MT_TSPLAY_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TSPLAY_FATAL_PRINT(fmt...) 		MT_TSPLAY_PRINT(" [FATAL] " fmt)
#define SAMPLE_TSPLAY_ERR_PRINT(fmt...)		    MT_TSPLAY_PRINT(" [ERROR] " fmt)
#define SAMPLE_TSPLAY_WARN_PRINT(fmt...)		MT_TSPLAY_PRINT(" [WARN] "  fmt)
#define SAMPLE_TSPLAY_INFO_PRINT(fmt...)		MT_TSPLAY_PRINT(" [INFO] "  fmt)
#define SAMPLE_TSPLAY_DBG_PRINT(fmt...)			MT_TSPLAY_PRINT(" [DEBUG] " fmt)

#define SAMPLE_TSPLAY_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
#define DMX_ID_0 0
#define INVALID_TSPID (0x1fff)


/*************************** Structure Definition ****************************/
typedef struct tagSource_Param_T
{
    mt_u8 FileName[256];
}source_param_t;
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    pthread_t          htsThd;
    PMT_COMPACT_TBL *pProgTbl;
} MT_TsPlay_RUN_INFO;


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_TsPlay_RUN_INFO    g_stTsPlayRunInfo = {MT_INVALID_HANDLE};
#ifdef MT_SAMPLE_APP
MT_AVPLAY_INFO avplayHandle;
#endif



/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_TsPlayDmxInit(MT_VOID)
{
    mt_s32  ret = MT_SUCCESS;

    SAMPLE_TSPLAY_FUNCTION_ENTER();

    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (MT_VOID)MT_UNF_DMX_DeInit();
        return MT_FAILURE;
    }

	SAMPLE_TSPLAY_FUNCTION_EXIT();

	return MT_SUCCESS;
}

/*
 @brief DmxDeinit and detachTSPort
 @return void
*/
static void MT_TsPlayDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}



/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_TsPlayAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_TSPLAY_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_TSPLAY_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_TSPLAY_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }

    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }

	ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

    return MT_SUCCESS;


ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

ERR6:
    (MT_VOID)MT_UNF_SND_Detach(hsoundTrack, hAvplay);

ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hsoundTrack);

ERR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();


    return MT_FAILURE;

}



/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void

*/
static void  MT_TsPlayAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    (MT_VOID)MT_UNF_AVPLAY_DeInit();

}

/*
@brief Audio and video decoding , synchronous
@param[in] phAvplay,A pointer to the Avplay handle passed in
@param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_TsPlayAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    mt_u32 VidPid = 0;
    mt_u32 AudPid = 0;
    mt_u32 PcrPid = 0;
    mt_u32 u32AudType = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_TSPLAY_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_TSPLAY_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }

    if(p_ProgInfo->VElementNum > 0 )
    {
        VidPid = p_ProgInfo->VElementPid;
        enVidType = p_ProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(p_ProgInfo->AElementNum > 0)
    {
        AudPid  = p_ProgInfo->AElementPid;
        u32AudType = p_ProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    PcrPid = p_ProgInfo->PcrPid;
    SAMPLE_TSPLAY_INFO_PRINT("%s ====%d  vidpid = %d AudPid=%d \n",__FILE__,__LINE__, VidPid, AudPid);

    if(INVALID_TSPID != PcrPid)
    {
		ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
		if (MT_SUCCESS != ret)
        {
		    SAMPLE_TSPLAY_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
		    return MT_FAILURE;
		}
    }

    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
            return ret;
        }
        /* The type of code stream supported by the decoder*/
        VcodecAttr.enType = enVidType;
        VcodecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VcodecAttr.u32ErrCover = 100;
        VcodecAttr.u32Priority = 3;
        VcodecAttr.u32UseDescInfoFlag = 1;

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_TSPLAY_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }

    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_TSPLAY_INFO_PRINT("u32AudType = %#x\n",u32AudType);

        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }

        SAMPLE_TSPLAY_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_TSPLAY_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }


    }


    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }


    //dmx sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync = { 0 };
        memset(&DmxAvsync, 0, sizeof(DmxAvsync));
        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay,MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC,(mt_void *)&DmxAvsync);
        if(ret != MT_SUCCESS)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
            return ret;
        }
    }


    //av sync
    if((INVALID_TSPID != VidPid) || (INVALID_TSPID != AudPid))
    {
        MT_UNF_SYNC_ATTR_S   SyncAttr = { 0 };
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
            return ret;
        }
        SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        SyncAttr.stSyncStartRegion.s32VidPlusTime = 20;
        SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        SyncAttr.bQuickOutput = MT_TRUE;
        SyncAttr.stSyncStartRegion.bSmoothPlay  = MT_TRUE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
            return ret;
        }
    }

    return MT_SUCCESS;
}

/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_TsPlayInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = 0;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;


    source_param_t *pstParam = (source_param_t *)(args);

    SAMPLE_TSPLAY_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->FileName, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_TSPLAY_ERR_PRINT( "\nfile %s open error!!\n", pstParam->FileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_UNF_DMX_GetTSBuffer(hTsBuffer, 188*1000, &StreamBuf, 1000);
        if(MT_SUCCESS != ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, sizeof(mt_s8), StreamBuf.u32Size, pTsFile);
        if(Readlen <= 0)
        {
            SAMPLE_TSPLAY_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");

            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }


        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_TSPLAY_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
        }
    }

    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }

    MT_UNF_DMX_DestroyTSBuffer(hTsBuffer);

    return MT_SUCCESS;
}

/*
@brief resume play
@param[in] avplay, Player handle
@return void
*/
static void MT_TsPlayRemuplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_RESUME_OPT_S opt1 = { 0 };
    (void)MT_UNF_AVPLAY_Resume(avplay, &opt1);
}

/*
@brief Pause play
@param[in] avplay, Player handle
@return void
*/
static void MT_TsPlayPauseplay(MT_HANDLE avplay)
{
    MT_UNF_AVPLAY_PAUSE_OPT_S opt;
    (void)MT_UNF_AVPLAY_Pause(avplay, &opt);
}

/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static mt_s32 MT_TsPlayStopplay(MT_HANDLE avplay)
{
    mt_s32     ret = MT_SUCCESS;

    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("failed to MT_UNF_AVPLAY_Stop\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*
@brief help
@return void
*/
static void MT_TsPlayPrint_Help(char *name)
{
    SAMPLE_TSPLAY_ERR_PRINT("Lack of parameters\n");
    SAMPLE_TSPLAY_INFO_PRINT("such as: %s -f ./1.ts\n", name);
    SAMPLE_TSPLAY_INFO_PRINT("         %s -q  <exit> \n", name);
}


static MT_VOID MT_TsPlayPrintProg(PMT_COMPACT_TBL *pProgTbl)
{
    mt_u32 i;
    PMT_COMPACT_PROG *p_node;

    SAMPLE_TSPLAY_PRINT("\n 1 - %d : select the program \n", pProgTbl->prog_num);
    SAMPLE_TSPLAY_PRINT("[index] [service id] [v_pid] [v_type] [a_pid] [a_type] [pcr pid]\n");
    for (i = 0; i < pProgTbl->prog_num; i++) {
        p_node = &pProgTbl->proginfo[i];
        SAMPLE_TSPLAY_PRINT("%6d%11d%8d%8d%9d   0x%x%6d\n", i + 1,
                p_node->ProgID,
                p_node->VElementPid, p_node->VideoType,
                p_node->AElementPid,  p_node->AudioType,
                p_node->PcrPid);
    }

    SAMPLE_TSPLAY_PRINT("     p : pause \n");
    SAMPLE_TSPLAY_PRINT("     c : continue \n");
    SAMPLE_TSPLAY_PRINT("     t : stop \n");
    SAMPLE_TSPLAY_PRINT("     s : start \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_TSPLAY_PRINT("     b : background run \n");
#endif
    SAMPLE_TSPLAY_PRINT("     d : descrambler \n");
    SAMPLE_TSPLAY_PRINT("     h : help \n");
    SAMPLE_TSPLAY_PRINT("     q : quit \n");
}

static MT_VOID MT_TsPlayPrintMenu(void)
{
    SAMPLE_TSPLAY_PRINT("TsPlay>> ");
}

static MT_VOID MT_TsPlayExit(void)
{
    SAMPLE_TSPLAY_FUNCTION_ENTER();

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_TsPlayStopplay(g_stTsPlayRunInfo.hAvPlay);

	(MT_VOID)MT_TsPlayAvplayDeInit(g_stTsPlayRunInfo.hAvPlay, g_stTsPlayRunInfo.hWin, g_stTsPlayRunInfo.hSoundTrack);

	(MT_VOID)MTADP_Search_FreeAllPmt(g_stTsPlayRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stTsPlayRunInfo.htsThd, NULL);

	(MT_VOID)MT_TsPlayDmxDeInit();

	(MT_VOID)MTADP_Snd_DeInit();

	(MT_VOID)MTADP_VO_DeInit();


    memset(&g_stTsPlayRunInfo, 0xff, sizeof(g_stTsPlayRunInfo));

    SAMPLE_TSPLAY_FUNCTION_EXIT();
}

static mt_s32 mss_create_descrambler(mt_u32 isAdvance, mt_u32 descramblerType,
    MT_UNF_DMX_CA_DEC_ENC_MODE_E enDecOrEncMode, mt_handle *phCaDesc)
{
    mt_s32 Ret;
    MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S DescAttr;

    if (!phCaDesc)
        return -1;
    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    memset(&DescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S));
    if (isAdvance)
        DescAttr.enCaType = MT_UNF_DMX_CA_ADVANCE;
    else
        DescAttr.enCaType = MT_UNF_DMX_CA_NORMAL;

    DescAttr.enDescramblerType = descramblerType;
    DescAttr.ivMode = 0;

    DescAttr.enDecOrEncMode = enDecOrEncMode;
    if (enDecOrEncMode != MT_UNF_DMX_CA_KEY_ATTR_DEC_OPEN) {
        if (enDecOrEncMode == MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_EVEN)
            DescAttr.encMode.encOddOrEven = 0;
        else
            DescAttr.encMode.encOddOrEven = 1;
        DescAttr.encMode.encForce = 0;
        DescAttr.encMode.encScrTagClr = 1;
        DescAttr.encMode.encTsScrClrRange = 1;
    }
    DescAttr.enEntropyReduction = MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN;

    DescAttr.tsCfg.tscfgDsMode = MT_UNF_DMX_CA_AUTO_DS_MODE;
    DescAttr.tsCfg.tscfgCwopt1Mode = MT_UNF_DMX_ACTIVE_PES_LOWBIT_DSC;
    DescAttr.tsCfg.multi2_key_msb64 = 0;

    Ret = MT_UNF_DMX_CreateDescramblerPro(DMX_ID_0, &DescAttr, phCaDesc);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_DMX_CreateDescramblerPro failed:%x\n", Ret);
	return Ret;
    }

    return Ret;
}

static mt_u8 g_u8ClearOddKey[8] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
static mt_u8 g_u8ClearEvenKey[8] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};

static mt_u32 evenSlot = MT_CIPHER_KEYSLOT_INVALID;
static mt_u32 oddSlot = MT_CIPHER_KEYSLOT_INVALID;

static mt_u32 descrambler_flag = 0;

static mt_s32 mss_descrambler_init(mt_handle hAvplay)  __attribute__((unused));
static mt_s32 mss_descrambler_init(mt_handle hAvplay)
{
    mt_s32 Ret;
    mt_handle hDmxVidChn, hDmxAudChn;
    mt_handle hDescrambler;
    mt_handle hCaDesc;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    mt_unf_cipher_keyslot_request(&evenSlot);
    mt_unf_cipher_keyslot_request(&oddSlot);

#if 1
    MT_KT_CTRL_S ktctrl;

    memset(&ktctrl, 0, sizeof(ktctrl));
    ktctrl.purpose = MT_KT_PURPOSE_TS;
    ktctrl.operation = MT_KT_OPERATION_DECRYPT;
    ktctrl.algorithm = MT_KT_ALG_CSA2;
    ktctrl.keysize = MT_KT_KEYSIZE_64;

    mt_unf_cipher_keyslot_set_ext(evenSlot, &ktctrl, g_u8ClearEvenKey, NULL);
    mt_unf_cipher_keyslot_set_ext(oddSlot, &ktctrl, g_u8ClearOddKey, NULL);
#else
    MT_CIPHER_CTRL_S ktctrl;
    memset(&ktctrl, 0, sizeof(ktctrl));
    ktctrl.core = MT_CIPHER_CORE_DSC_TS;
    ktctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
    ktctrl.algorithm = MT_CIPHER_ALG_CSA2;

    mt_unf_cipher_keyslot_set(evenSlot, &ktctrl, g_u8ClearEvenKey, NULL);
    mt_unf_cipher_keyslot_set(oddSlot, &ktctrl, g_u8ClearOddKey, NULL);
#endif

    mss_create_descrambler(1, MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2,
        MT_UNF_DMX_CA_KEY_ATTR_DEC_OPEN, &hCaDesc);

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

    Ret = MT_UNF_DMX_SetDescramblerEvenKeySlot(hCaDesc, (mt_u8)evenSlot);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_DMX_SetDescramblerEvenKeySlot failed:%x\n", Ret);
	return Ret;
    }

    Ret = MT_UNF_DMX_SetDescramblerOddKeySlot(hCaDesc, (mt_u8)oddSlot);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_DMX_SetDescramblerOddKeySlot failed:%x\n", Ret);
	return Ret;
    }
    printf("%s(%d) evenSlot:%d oddSlot:%d\n", __FUNCTION__, __LINE__, evenSlot, oddSlot);
    mt_unf_cipher_keyslot_info(evenSlot);
    mt_unf_cipher_keyslot_info(oddSlot);

    descrambler_flag = 1;
    return MT_SUCCESS;
}

static mt_s32 mss_descrambler_init_normal(mt_handle hAvplay) __attribute__((unused));

static mt_s32 mss_descrambler_init_normal(mt_handle hAvplay)
{
    mt_s32 Ret;
    mt_handle hDmxVidChn, hDmxAudChn;
    mt_handle hDescrambler;
    mt_handle hCaDesc;

    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    mss_create_descrambler(0, MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2,
        MT_UNF_DMX_CA_KEY_ATTR_DEC_OPEN, &hCaDesc);

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

    Ret = MT_UNF_DMX_SetDescramblerEvenKey(hCaDesc, g_u8ClearEvenKey);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_DMX_SetDescramblerEvenKey failed:%x\n", Ret);
	return Ret;
    }

    Ret = MT_UNF_DMX_SetDescramblerOddKey(hCaDesc, g_u8ClearOddKey);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_DMX_SetDescramblerOddKey failed:%x\n", Ret);
	return Ret;
    }

    descrambler_flag = 1;

    return MT_SUCCESS;
}

static mt_s32 mss_descrambler_deinit(mt_handle hAvplay)
{
    mt_s32 Ret;
    mt_handle hDmxVidChn, hDmxAudChn;
    mt_handle hDescrambler;

    if (descrambler_flag == 0)
        return 0;
    printf("%s(%d)\n", __FUNCTION__, __LINE__);

    Ret = MT_UNF_AVPLAY_GetDmxVidChnHandle(hAvplay, &hDmxVidChn);
    Ret |= MT_UNF_AVPLAY_GetDmxAudChnHandle(hAvplay, &hDmxAudChn);
    if (MT_SUCCESS != Ret) {
        printf("MT_UNF_AVPLAY_GetDmxChnHandle failed:%x\n", Ret);
        return Ret;
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxVidChn, &hDescrambler);
    if (MT_SUCCESS == Ret) {
        MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxVidChn);
        MT_UNF_DMX_DestroyDescrambler(hDescrambler);
    }

    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxAudChn, &hDescrambler);
    if (MT_SUCCESS == Ret) {
        MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxAudChn);
        MT_UNF_DMX_DestroyDescrambler(hDescrambler);
    }

    if (oddSlot != MT_CIPHER_KEYSLOT_INVALID) {
        mt_unf_cipher_keyslot_release(oddSlot);
        oddSlot = MT_CIPHER_KEYSLOT_INVALID;
    }
    if (evenSlot != MT_CIPHER_KEYSLOT_INVALID) {
        mt_unf_cipher_keyslot_release(evenSlot);
        evenSlot = MT_CIPHER_KEYSLOT_INVALID;
    }

    descrambler_flag = 0;
    return MT_SUCCESS;
}


/*
@brief quit and function
@return void
*/
static void MT_TsPlayCmdTask(mt_handle hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    mt_s32     ret = MT_SUCCESS;
    mt_u32     u32ProgNum = 0;
    MT_CHAR    inputCmd[32] = { 0 };
    PMT_COMPACT_PROG *stCurrentProgInfo = { 0 };
    struct timespec start, end;
    double elapsed_seconds;

    (void)MT_TsPlayPrintProg(pProgTbl);

    while (1)
    {
        MT_TsPlayPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_TSPLAY_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_TSPLAY_INFO_PRINT("tsplay play in back!\n");
            break;
        }
#endif
        /* pause*/
        else if('p' == inputCmd[0])
        {
            (void)MT_TsPlayPauseplay(hAvplay);
            continue;
        }
        /* continue*/
        else if('c' == inputCmd[0])
        {
            (void)MT_TsPlayRemuplay(hAvplay);
            continue;
        }

        /* start*/
        else if('s' == inputCmd[0])
        {
            (void)MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
            continue;
        }
         /* stop*/
        else if('t' == inputCmd[0])
        {
            (void)MT_TsPlayStopplay(hAvplay);
            mss_descrambler_deinit(hAvplay);
            continue;
        }

         /* descrambler*/
        else if('d' == inputCmd[0])
        {
            //(void)mss_descrambler_init(hAvplay);
            (void)mss_descrambler_init_normal(hAvplay);
            continue;
        }

        /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                (void)MT_TsPlayStopplay(hAvplay);

                SAMPLE_TSPLAY_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret  = MT_TsPlayAVPlay_Start(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_TSPLAY_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_TSPLAY_INFO_PRINT("prog_num the biggest is %d\n", pProgTbl->prog_num);
                SAMPLE_TSPLAY_INFO_PRINT("q: quit\n");
                continue;
            }
            clock_gettime(CLOCK_MONOTONIC, &end);
            elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            MT_TSPLAY_PRINT("Switching time = %lf s\n", elapsed_seconds);
#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_TSPLAY_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
	else if('h' == inputCmd[0])
        {
            (void)MT_TsPlayPrintProg(pProgTbl);
            continue;
        }

    }
}

/*!
@brief gets the external input parameters
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::MT_VOID
@*/
static MT_S32 MT_TsPlayParase_args(MT_S32 argc, MT_CHAR *argv[], source_param_t *pInutParam)
{
    int opt = 0;

    SAMPLE_TSPLAY_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_TsPlayPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_TsPlayExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                MTADP_Strncpy((mt_char*)pInutParam->FileName, mt_optarg, sizeof(source_param_t));
            	break;

            default:
                (MT_VOID)MT_TsPlayPrint_Help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    SAMPLE_TSPLAY_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_TsPlayMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    mt_s32     ret = MT_SUCCESS;
    source_param_t stParam = { 0 };
    struct timespec start, end;
    double elapsed_seconds;

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_TsPlayPrint_Help(argv[0]);
        return MT_SUCCESS;
    }

    /** Get the parameters */
    ret = MT_TsPlayParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif
        clock_gettime(CLOCK_MONOTONIC, &start);

        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR3;
        }

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR4;
        }

        ret = MT_TsPlayDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT( "failed to StartDmx\n");
            goto ERR5;
        }

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stTsPlayRunInfo.htsThd, NULL, (void * (*)(void *))MT_TsPlayInjectTsTask, &stParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR6;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR6;
        }
        (void)MTADP_Search_Init();

        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stTsPlayRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR8;
        }

        ret = MT_TsPlayAVplayInit(&g_stTsPlayRunInfo.hAvPlay, &g_stTsPlayRunInfo.hWin, &g_stTsPlayRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_TSPLAY_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR9;
        }
#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stTsPlayRunInfo.hAvPlay;
#endif

        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        MT_TSPLAY_PRINT("Search time = %lf s\n", elapsed_seconds);
    }

    (void)MT_TsPlayCmdTask(g_stTsPlayRunInfo.hAvPlay, g_stTsPlayRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif

    ret = MT_TsPlayStopplay(g_stTsPlayRunInfo.hAvPlay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_TSPLAY_ERR_PRINT("failed to stop\n");
        goto ERR10;
    }
    SAMPLE_TSPLAY_INFO_PRINT("stop to play\n");

ERR10:
    (MT_VOID)MT_TsPlayAvplayDeInit(g_stTsPlayRunInfo.hAvPlay, g_stTsPlayRunInfo.hWin, g_stTsPlayRunInfo.hSoundTrack);

ERR9:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stTsPlayRunInfo.pProgTbl);

ERR8:
    (MT_VOID)MTADP_Search_DeInit();

//ERR7:
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stTsPlayRunInfo.htsThd, NULL);

ERR6:
    (MT_VOID)MT_TsPlayDmxDeInit();

ERR5:
    (MT_VOID)MTADP_VO_DeInit();

ERR4:
    (MT_VOID)MTADP_Snd_DeInit();

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stTsPlayRunInfo, 0xff, sizeof(g_stTsPlayRunInfo));
    return MT_SUCCESS;
}

