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

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"




/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_SHOWLOGO_DEBUG
#define MT_SHOWLOGO_PRINT   printf
#else
#define MT_SHOWLOGO_PRINT
#endif

#define SAMPLE_SHOWLOGO_FUNCTION_ENTER()        MT_SHOWLOGO_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_SHOWLOGO_FUNCTION_EXIT()         MT_SHOWLOGO_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_SHOWLOGO_FATAL_PRINT(fmt...)         MT_SHOWLOGO_PRINT(" [FATAL] " fmt)
#define SAMPLE_SHOWLOGO_ERR_PRINT(fmt...)           MT_SHOWLOGO_PRINT(" [ERROR] " fmt)
#define SAMPLE_SHOWLOGO_WARN_PRINT(fmt...)          MT_SHOWLOGO_PRINT(" [WARN] "  fmt)
#define SAMPLE_SHOWLOGO_INFO_PRINT(fmt...)          MT_SHOWLOGO_PRINT(" [INFO] "  fmt)
#define SAMPLE_SHOWLOGO_DBG_PRINT(fmt...)           MT_SHOWLOGO_PRINT(" [DEBUG] " fmt)


#define SAMPLE_SHOWLOGO_PRINT printf

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
    MT_HANDLE          hWin;
    MT_HANDLE          hSoundTrack;
    pthread_t          htsThd;
} MT_ShowLogo_RUN_INFO;


/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;


static MT_ShowLogo_RUN_INFO    g_stShowLogoRunInfo = {MT_INVALID_HANDLE};

#ifdef MT_SAMPLE_APP
MT_S32 MT_ShowLogoMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


/*
 @brief Audio and video playback init
 @param[out] phSoundTrack,Pointer to the outgoing SoundTrack handle
 @param[out] hWin, Pointer to the outgoing Window handle
 @param[out] phAvplay,Pointer to the outgoing AVPLAY handle
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_ShowLogoAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };

    if(NULL == p_hAvplay)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }


    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }

    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_ES);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.enStreamType = MT_UNF_AVPLAY_STREAM_TYPE_ES;

    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }



    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR3;
    }

    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR4;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR5;
    }

    *p_hAvplay = hAvplay;
    *P_hWin = hWin;
    *p_hSoundTrack = hsoundTrack;

    return MT_SUCCESS;


ERR5:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

ERR4:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

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
static void  MT_ShowLogoAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

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
static mt_s32 MT_ShowLogoAVPlay_Start(mt_handle hAvplay)
{
    mt_u32 VidPid = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }


    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
            return ret;
        }
        /* The type of code stream supported by the decoder*/
        VcodecAttr.enType = MT_UNF_VCODEC_TYPE_MPEG2;

        VcodecAttr.u32UseDescInfoFlag = 0;

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }



    }

    (MT_VOID)MTADP_AVPlay_SetVdecAttr(hAvplay, MT_UNF_VCODEC_TYPE_MPEG2, MT_UNF_VCODEC_MODE_NORMAL);

    ret = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
        return ret;
    }




    return MT_SUCCESS;
}


/*
@brief stop to play
@param[in] avplay, Player handle
@return void
*/
static mt_s32 MT_ShowLogoStopplay(MT_HANDLE avplay)
{
    mt_s32     ret = MT_SUCCESS;

    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    stopopt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("failed to MT_UNF_AVPLAY_Stop\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
static mt_s32 MT_ShowLogoEvent(mt_handle avplay, MT_UNF_AVPLAY_EVENT_E enEvent, mt_u32 u32Para)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
static mt_s32 MT_ShowLogoEvent(mt_handle avplay, MT_UNF_AVPLAY_EVENT_E enEvent, ulong u32Para)
#endif
{
    if(avplay == MT_INVALID_HANDLE)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("An invalid avplay handle");
        return MT_FAILURE;
    }

    switch(enEvent)
    {
        case MT_UNF_AVPLAY_EVENT_EOS:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_EOS\n");

            (MT_VOID)MT_ShowLogoStopplay(g_stShowLogoRunInfo.hAvPlay);
            break;
        case MT_UNF_AVPLAY_EVENT_STOP:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_STOP\n");
            break;
        case MT_UNF_AVPLAY_EVENT_NORM_SWITCH:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_NORM_SWITCH\n");
            break;
        case MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME\n");
            break;
        case MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME\n");
            break;
        case MT_UNF_AVPLAY_EVENT_VID_BUF_STATE:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_VID_BUF_STATE\n");
            break;
        case MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE\n");
            break;
        case MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT\n");
            break;
        case MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO\n");
            break;
        case MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE\n");
            break;
        case MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT\n");
            break;
        case MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR\n");
            break;
        case MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME\n");
            break;
        case MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE:
            break;
        case MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP\n");
            break;
        case MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED:
            printf("avplay_event: MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED\n");
            break;

        default:
             printf("%s: avplay %x - avent(%d) not support!\n", __FUNCTION__, (mt_u32)avplay, enEvent);
            break;
    }

    return MT_SUCCESS;
}

/*
@brief Read path file contents into g_hTsBuffer
@param[in] args, Structure of file
@return ::MT_SUCCESS
@return ::MT_FAILURE
*/
static mt_s32 MT_ShowLogoInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;
    MT_UNF_AVPLAY_PUTBUFEX_OPT_S PutOpt = {0};
    MT_BOOL pbIsEmpty = {0};

    source_param_t *pstParam = (source_param_t *)(args);

    SAMPLE_SHOWLOGO_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);

    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->FileName, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT( "file %s open error!!\n", pstParam->FileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }



    /* loop in inject data */
    while(g_bTaskQuit == MT_FALSE)
    {

        ret = MT_UNF_AVPLAY_GetBuf(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_BUF_ID_ES_VID, 188*1000, &StreamBuf, 0);
        if(MT_SUCCESS != ret)
        {
            continue;
        }

        Readlen = fread(StreamBuf.pu8Data, 1, 188*1000, pTsFile);

        if(Readlen > 0)
        {

            ret = MT_UNF_AVPLAY_PutBuf(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen, 0xFFFFFFFF);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SHOWLOGO_ERR_PRINT("MT_UNF_AVPLAY_PutBuf failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            }

            PutOpt.u32EosFlag = 1;
            PutOpt.bEndOfFrm = 1;
            ret = MT_UNF_AVPLAY_PutBuf64(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_BUF_ID_ES_VID, Readlen, 0xFFFFFFFF, &PutOpt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_SHOWLOGO_ERR_PRINT("MT_UNF_AVPLAY_PutBuf64 failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
            }

       }

       else if(Readlen <= 0)
       {

            ret = MT_UNF_AVPLAY_FlushStream(g_stShowLogoRunInfo.hAvPlay, NULL);
            ret |= MT_UNF_AVPLAY_IsBuffEmpty(g_stShowLogoRunInfo.hAvPlay, &pbIsEmpty);
            if(MT_SUCCESS == ret)
            {
                break;
            }

       }


    }


    if(pTsFile)
    {
        fclose(pTsFile);
        pTsFile = NULL;
    }


    return MT_SUCCESS;
}

/*
@brief help
@return void
*/
static void MT_ShowLogoPrint_Help(char *name)
{
    SAMPLE_SHOWLOGO_ERR_PRINT("Lack of parameters\n");
    SAMPLE_SHOWLOGO_ERR_PRINT("such as: %s -f ./1.m2v\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_SHOWLOGO_ERR_PRINT("         %s -q  <exit> \n", name);
#endif
}


static MT_VOID MT_ShowLogoPrintMenu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_SHOWLOGO_PRINT("     b : background run \n");
#endif
    SAMPLE_SHOWLOGO_PRINT("     h : help \n");
    SAMPLE_SHOWLOGO_PRINT("     q : quit \n");
    SAMPLE_SHOWLOGO_PRINT("SHOWLOGO>> ");

}

static MT_VOID MT_ShowLogoExit(void)
{
    SAMPLE_SHOWLOGO_FUNCTION_ENTER();

    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stShowLogoRunInfo.htsThd, NULL);

    (MT_VOID)MT_UNF_AVPLAY_UnRegisterEvent(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_EVENT_EOS);

    (MT_VOID)MT_ShowLogoAvplayDeInit(g_stShowLogoRunInfo.hAvPlay, g_stShowLogoRunInfo.hWin, g_stShowLogoRunInfo.hSoundTrack);

    (MT_VOID)MTADP_VO_DeInit();

    memset(&g_stShowLogoRunInfo, 0xff, sizeof(MT_ShowLogo_RUN_INFO));

    SAMPLE_SHOWLOGO_FUNCTION_EXIT();
}


/*
@brief quit and function
@return void
*/
static void MT_ShowLogoCmdTask(mt_handle hAvplay)
{
    MT_CHAR    inputCmd[32] = { 0 };


    while (1)
    {
        (MT_VOID)MT_ShowLogoPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        /* quit*/
        if('q' == inputCmd[0])
        {
            SAMPLE_SHOWLOGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
           SAMPLE_SHOWLOGO_INFO_PRINT("Showlogo play in back!\n");
           break;
        }
#endif

        else if('h' == inputCmd[0])
        {
            SAMPLE_SHOWLOGO_INFO_PRINT("print help info \n");
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
static MT_S32 MT_ShowLogoParase_args(MT_S32 argc, MT_CHAR *argv[], source_param_t *pInutParam)
{
    int opt = 0;

    SAMPLE_SHOWLOGO_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_ShowLogoPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_ShowLogoExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                MTADP_Strncpy((mt_char*)pInutParam->FileName, mt_optarg, sizeof(source_param_t));
                break;

            default:
                (MT_VOID)MT_ShowLogoPrint_Help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    SAMPLE_SHOWLOGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_ShowLogoMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    mt_s32     ret = MT_SUCCESS;
    source_param_t stParam = { 0 };


    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_ShowLogoPrint_Help(argv[0]);
        return MT_SUCCESS;
    }

    /** Get the parameters */
    ret = MT_ShowLogoParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_SHOWLOGO_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MT_SYS_Init\n");
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MTADP_HDMI_Init\n");
            goto ERR1;
        }

        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MTADP_Disp_Init\n");
            goto ERR2;
        }
#endif


        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR3;
        }


        ret = MT_ShowLogoAVplayInit(&g_stShowLogoRunInfo.hAvPlay, &g_stShowLogoRunInfo.hWin, &g_stShowLogoRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MT_ShowLogoAVplayInit\n");
            goto ERR4;
        }


        ret = MT_UNF_AVPLAY_RegisterEvent(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_EVENT_EOS, MT_ShowLogoEvent);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MT_UNF_AVPLAY_RegisterEvent\n");
            goto ERR5;
        }

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stShowLogoRunInfo.htsThd, NULL, (void * (*)(void *))MT_ShowLogoInjectTsTask, &stParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to pthread_create\n");
            goto ERR6;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR6;
        }

        ret = MT_ShowLogoAVPlay_Start(g_stShowLogoRunInfo.hAvPlay);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_SHOWLOGO_ERR_PRINT("failed to MT_ShowLogoAVPlay_Start\n");
            goto ERR7;
        }




    }

    (void)MT_ShowLogoCmdTask(g_stShowLogoRunInfo.hAvPlay);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR7:
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stShowLogoRunInfo.htsThd, NULL);

ERR6:

    (MT_VOID)MT_UNF_AVPLAY_UnRegisterEvent(g_stShowLogoRunInfo.hAvPlay, MT_UNF_AVPLAY_EVENT_EOS);

ERR5:

    (MT_VOID)MT_ShowLogoAvplayDeInit(g_stShowLogoRunInfo.hAvPlay, g_stShowLogoRunInfo.hWin, g_stShowLogoRunInfo.hSoundTrack);

ERR4:
    (MT_VOID)MTADP_VO_DeInit();

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stShowLogoRunInfo, 0xff, sizeof(MT_ShowLogo_RUN_INFO));
    return MT_SUCCESS;
}
