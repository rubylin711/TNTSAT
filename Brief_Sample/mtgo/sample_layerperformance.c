/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

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

#include "mt_go.h"

#include "sample_mtgo_common.h"

#ifdef MT_SAMPLE_PERFORMANCE_DEBUG 

#define MT_PERFORMANCE_PRINT   printf
#else

#define MT_PERFORMANCE_PRINT 

#endif

#define SAMPLE_PERFORMANCE_FUNCTION_ENTER()	    MT_PERFORMANCE_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_PERFORMANCE_FUNCTION_EXIT()		MT_PERFORMANCE_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_PERFORMANCE_FATAL_PRINT(fmt...) 	    MT_PERFORMANCE_PRINT(" [FATAL] " fmt)
#define SAMPLE_PERFORMANCE_ERR_PRINT(fmt...)		MT_PERFORMANCE_PRINT(" [ERROR] " fmt)
#define SAMPLE_PERFORMANCE_WARN_PRINT(fmt...)		MT_PERFORMANCE_PRINT(" [WARN] "  fmt)
#define SAMPLE_PERFORMANCE_INFO_PRINT(fmt...)		MT_PERFORMANCE_PRINT(" [INFO] "  fmt)
#define SAMPLE_PERFORMANCE_DBG_PRINT(fmt...)		MT_PERFORMANCE_PRINT(" [DEBUG] " fmt)

#define SAMPLE_PERFORMANCE_PRINT   printf


#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
#define DMX_ID_0 0
#define INVALID_TSPID (0x1fff)


static MT_U32 g_rgb_clut_palette[6] =
{
    0xFF0000FF,  //blue
    0xFF00FF00,  //green
    0xFFFF0000,  //red
    0xFFFFFFFF,  //white
    0xFFFFFF00,  //yellow
    0xFF000000   //black
};
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
    MT_HANDLE hLayer_osd0;
    MT_HANDLE hLayer_osd1;
    MT_HANDLE hLayer_sub;
    MT_HANDLE hLayer_still;
} MT_GO_performance_RUN_INFO;

static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_GO_performance_RUN_INFO    g_stperformanceRunInfo;


#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoLayerpmMain(mt_s32 argc, mt_char *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif


/*
 @brief Dmxinit and attachTSPort
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static mt_s32 MT_GO_PerformanceDmxInit(MT_VOID)
{
    mt_s32  ret = MT_SUCCESS;
    
    SAMPLE_PERFORMANCE_FUNCTION_ENTER();
    
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_DMX_Init\n");
        return MT_FAILURE;
    }
    
    ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_RAM_0);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_DMX_AttachTSPort\n");
        (MT_VOID)MT_UNF_DMX_DeInit();
        return MT_FAILURE;
    }
	
	SAMPLE_PERFORMANCE_FUNCTION_EXIT();

	return MT_SUCCESS;
}

static void MT_GO_PerformanceDmxDeInit(MT_VOID)
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
static mt_s32 MT_GO_PerformanceAVplayInit(mt_handle *p_hAvplay, mt_handle *P_hWin, mt_handle *p_hSoundTrack)
{
    mt_s32      ret = MT_SUCCESS;
    mt_handle   hAvplay = 0;
    mt_handle   hWin = 0;
    mt_handle   hsoundTrack = 0;
    MT_UNF_AUDIOTRACK_ATTR_S    stTrackAttr = { 0 };	
    MT_UNF_AVPLAY_ATTR_S        AvplayAttr = { 0 };
    	
    if(NULL == p_hAvplay)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("p_hAvplay is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == P_hWin)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("P_hWin is NULL!\n");
        return MT_FAILURE;
    }
    if(NULL == p_hSoundTrack)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("p_hSoundTrack is NULL!\n");
        return MT_FAILURE;
    }
    
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_Init failed.\n");
        return ret;
    }
    
    /* brief Obtains the default configuration of an AVPLAY*/
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;
    
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }
    
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2;
    }

    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }
    
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_SND_GetDefaultTrackAttr failed.\n"); 
        goto ERR4;
    }
    
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, &hsoundTrack);
    if(MT_SUCCESS != ret)
    { 
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_SND_CreateTrack failed.\n"); 
        goto ERR4;
    }
    
    ret = MT_UNF_SND_Attach(hsoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_SND_Attach failed.\n");
        goto ERR5;
    }
      
	ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MTADP_VO_Init failed.\n");
        goto ERR6;
    }
    
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_VO_SetWindowEnable failed.\n");
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
static void  MT_GO_PerformanceAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
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
static mt_s32 MT_GO_PerformanceAVPlay_Start(mt_handle hAvplay, PMT_COMPACT_PROG *p_ProgInfo)
{
    mt_u32 VidPid = 0;
    mt_u32 AudPid = 0;
    mt_u32 u32AudType = 0;
    mt_s32 ret = 0;
    MT_UNF_VCODEC_TYPE_E enVidType = { 0 };
    MT_UNF_VCODEC_ATTR_S VcodecAttr = { 0 };
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0X00;

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == p_ProgInfo)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("p_ProgInfo is NULL!\n");
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

    SAMPLE_PERFORMANCE_INFO_PRINT("%s ====%d  vidpid = %x AudPid=%x \n",__FILE__,__LINE__, VidPid, AudPid);
    
    if(INVALID_TSPID != VidPid)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VcodecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.ret = %#x\n",ret);
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
            SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.ret = %#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
        else
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("has no video stream!ret = %#x\n",ret);
        }
        
    }

    if(INVALID_TSPID != AudPid)
    {
        SAMPLE_PERFORMANCE_INFO_PRINT("u32AudType = %#x\n",u32AudType);
        
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay, u32AudType, HD_DEC_MODE_RAWPCM, 1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT(" MTADP_AVPlay_SetAdecAttr failed.\n");
            return ret;
        }
        
        SAMPLE_PERFORMANCE_INFO_PRINT("%s ====%d audiopid %d \n",__FILE__,__LINE__,AudPid);
        
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &AudPid);
        if(MT_SUCCESS == ret)
        {
            enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
        else
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("has no audio stream!ret = %#x\n",ret);
        }
  
        
    }

    
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT(" MT_UNF_AVPLAY_Start failed.\n");
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
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC\n");
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
            SAMPLE_PERFORMANCE_ERR_PRINT("MT_UNF_AVPLAY_GetAttr for sync failed:%#x\n",ret);
            return ret;
        }
        SyncAttr.enSyncRef = MT_UNF_SYNC_REF_VIDEO;
        SyncAttr.stSyncStartRegion.s32VidPlusTime = 20;
        SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        SyncAttr.bQuickOutput = MT_TRUE;
        SyncAttr.stSyncStartRegion.bSmoothPlay  = MT_TRUE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_AVPLAY_SetAttr MT_UNF_AVPLAY_ATTR_ID_SYNC\n");
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
static mt_s32 MT_GO_PerformanceInjectTsTask(mt_void *args)
{
    mt_s32  ret = MT_SUCCESS;
    mt_u32  Readlen = 0;
    MT_HANDLE hTsBuffer = 0;
    MT_UNF_STREAM_BUF_S StreamBuf = { 0 };
    FILE *pTsFile = NULL;
      
     
    source_param_t *pstParam = (source_param_t *)(args);

    SAMPLE_PERFORMANCE_INFO_PRINT(">>>open file : %s  >>>> \n", pstParam->FileName);
    
    /* Open a binary file. The file must exist. Read only*/
    pTsFile = fopen((char*)pstParam->FileName, "rb");
    if(pTsFile == NULL)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT( "\nfile %s open error!!\n", pstParam->FileName);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_RAM_0, 0x200000, &hTsBuffer);
    if(ret != MT_SUCCESS)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_DMX_CreateTSBuffer\n");
        (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    ret = MT_UNF_DMX_ResetTSBuffer(hTsBuffer);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT( "failed to MT_UNF_DMX_ResetTSBuffer\n");
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
            SAMPLE_PERFORMANCE_INFO_PRINT("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER..!\n");
            
            /* Set the file location to the beginning of the file for the given stream*/
            rewind(pTsFile);
            continue;
        }
     

        ret = MT_UNF_DMX_PutTSBuffer(hTsBuffer, Readlen);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_PERFORMANCE_ERR_PRINT( "failed to MT_UNF_DMX_PutTSBuffer\n");
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

static mt_s32 MT_GO_PerformanceStopplay(MT_HANDLE avplay)
{    
    mt_s32     ret = MT_SUCCESS;
    
    MT_UNF_AVPLAY_STOP_OPT_S stopopt = { 0 };
    stopopt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    stopopt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(avplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopopt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_AVPLAY_Stop\n");
        return MT_FAILURE;   
    }

    return MT_SUCCESS;
}


static MT_VOID MT_GO_PerformanceCreatlayer(MTGO_LAYER_E LayerID, MT_HANDLE *hlayer)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };

    s32Ret = MT_GO_GetLayerDefaultParam(LayerID, &stLayerInfo);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_GetLayerDefaultParam osd0\n");
    }
    stLayerInfo.PixelFormat = MTGO_PF_8888;
    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_DOUBBUFER;
    
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, hlayer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_CreateLayer\n");
    }
}



static MT_VOID MT_GO_PerformanceFilllayer(MT_HANDLE hlayer , mt_s32 color_number)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_HANDLE hLayerSurface = MTGO_INVALID_HANDLE;

    MT_RECT   stRect = { 0 };


    stRect.x = 0;
    stRect.y = 0;
    stRect.w = 1280;
    stRect.h = 720;

    s32Ret = MT_GO_GetLayerSurface(hlayer, &hLayerSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_GetLayerSurface\n");
    }


    s32Ret = MT_GO_FillRect(hLayerSurface, &stRect, g_rgb_clut_palette[color_number], MTGO_COMPOPT_NONE);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_FillRect\n");
    }
    s32Ret = MT_GO_SetLayerAlpha(hlayer, 100);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_SetLayerAlpha\n");
    }
    s32Ret = MT_GO_RefreshLayer(hlayer, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_RefreshLayer1\n");
    }

}

static void MT_GO_PerformancePrint_Help(char *name)
{
    SAMPLE_PERFORMANCE_PRINT("Lack of parameters\n");
    SAMPLE_PERFORMANCE_PRINT("such as: %s -f ./1.ts\n", name);
#ifdef MT_SAMPLE_APP    
    SAMPLE_PERFORMANCE_PRINT("         %s -q  <exit> \n", name);
#endif

}


static MT_VOID MT_GO_PerformancePrintMenu(MT_U32 prog_num)
{
    SAMPLE_PERFORMANCE_PRINT("\n 1 - %d : select the program \n", prog_num);
    SAMPLE_PERFORMANCE_PRINT("       s0 : fill still \n");
    SAMPLE_PERFORMANCE_PRINT("       s1 : fill osd0 \n");
    SAMPLE_PERFORMANCE_PRINT("       s2 : fill osd1 \n");
    SAMPLE_PERFORMANCE_PRINT("       s3 : fill sub \n");
    SAMPLE_PERFORMANCE_PRINT("       t0 : destroy still \n");
    SAMPLE_PERFORMANCE_PRINT("       t1 : destroy osd0 \n");
    SAMPLE_PERFORMANCE_PRINT("       t2 : destroy osd1 \n");
    SAMPLE_PERFORMANCE_PRINT("       t3 : destroy sub \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_PERFORMANCE_PRINT("        b : background run \n");
#endif
    SAMPLE_PERFORMANCE_PRINT("        h : help info \n");
    SAMPLE_PERFORMANCE_PRINT("        q : quit \n");
    SAMPLE_PERFORMANCE_PRINT("Performance>> ");
}


static  mt_s32 MT_GO_PerformanceCmdTask(mt_handle hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    mt_s32     ret = MT_SUCCESS;
    MT_CHAR   *fgetret = NULL;
    MT_CHAR   inputCmd[32] = { 0 };
    mt_u32    u32ProgNum = 0;   
    PMT_COMPACT_PROG *stCurrentProgInfo = { 0 };

    while(1)
    {
        (MT_VOID)MT_GO_PerformancePrintMenu(pProgTbl->prog_num);
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;

        if('q' == inputCmd[0])
        {            
            g_bTaskQuit = MT_TRUE;            
            SAMPLE_PERFORMANCE_INFO_PRINT("prepare to exit!\n");
            break;
           
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_PERFORMANCE_INFO_PRINT("layer performance in back!\n");
            break;
        }
#endif   

        else if('s' == inputCmd[0])
        {

            if('1' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE == g_stperformanceRunInfo.hLayer_osd0)
                {
                    (MT_VOID)MT_GO_PerformanceCreatlayer(MTGO_LAYER_OSD0, &g_stperformanceRunInfo.hLayer_osd0);
                }
                
                SAMPLE_PERFORMANCE_INFO_PRINT("fill layer_osd0 \n");
                 
                (MT_VOID)MT_GO_PerformanceFilllayer(g_stperformanceRunInfo.hLayer_osd0, 0);

            }
            else if('2' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE == g_stperformanceRunInfo.hLayer_osd1)
                {
                    (MT_VOID)MT_GO_PerformanceCreatlayer(MTGO_LAYER_OSD1, &g_stperformanceRunInfo.hLayer_osd1);
                }

                SAMPLE_PERFORMANCE_INFO_PRINT("fill layer_osd1 \n");
                (MT_VOID)MT_GO_PerformanceFilllayer(g_stperformanceRunInfo.hLayer_osd1, 1);
            }         
            else if('3' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE == g_stperformanceRunInfo.hLayer_sub)
                {
                    (MT_VOID)MT_GO_PerformanceCreatlayer(MTGO_LAYER_SUB, &g_stperformanceRunInfo.hLayer_sub);
                }

                SAMPLE_PERFORMANCE_INFO_PRINT("fill layer_sub \n");
                (MT_VOID)MT_GO_PerformanceFilllayer(g_stperformanceRunInfo.hLayer_sub, 2);
            }
            else if('0' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE == g_stperformanceRunInfo.hLayer_still)
                {
                    SAMPLE_PERFORMANCE_INFO_PRINT("creat layer still\n");
                    (MT_VOID)MT_GO_PerformanceCreatlayer(MTGO_LAYER_STILL, &g_stperformanceRunInfo.hLayer_still);
                }
                SAMPLE_PERFORMANCE_INFO_PRINT("fill layer_still \n");
                (MT_VOID)MT_GO_PerformanceFilllayer(g_stperformanceRunInfo.hLayer_still, 4);
            }            
            
        }
        else if('t' == inputCmd[0])
        {
            if('0' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_still)
                {
                    (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_still);
                }
                
                g_stperformanceRunInfo.hLayer_still = MTGO_INVALID_HANDLE;
            }
            if('1' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd0)
                {
                    (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd0);
                }
                g_stperformanceRunInfo.hLayer_osd0 = MTGO_INVALID_HANDLE;
            }
            if('2' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd1)
                {
                    (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd1);
                }
                g_stperformanceRunInfo.hLayer_osd1 = MTGO_INVALID_HANDLE;
            }
            if('3' == inputCmd[1])
            {
                if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_sub)
                {
                    (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_sub);
                }
                g_stperformanceRunInfo.hLayer_sub = MTGO_INVALID_HANDLE;
            }
            
        }
                /* Switch between programs*/
        else if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {            
            u32ProgNum = atoi(inputCmd);
            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                (void)MT_GO_PerformanceStopplay(hAvplay);
              
                SAMPLE_PERFORMANCE_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);

                ret  = MT_GO_PerformanceAVPlay_Start(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_PERFORMANCE_ERR_PRINT(" SwitchProg failed.\n");
                }
            }
            else
            {
                SAMPLE_PERFORMANCE_INFO_PRINT("prog_num the biggest is %d\n", pProgTbl->prog_num);
                SAMPLE_PERFORMANCE_INFO_PRINT("q: quit\n");
                continue;
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_PERFORMANCE_INFO_PRINT("printf help info \n");
        }
    }

    return MT_SUCCESS;
}

static MT_VOID MT_GO_PerformanceExit(void)
{
    (MT_VOID)MT_GO_PerformanceStopplay(g_stperformanceRunInfo.hAvPlay);

	(MT_VOID)MT_GO_PerformanceAvplayDeInit(g_stperformanceRunInfo.hAvPlay, g_stperformanceRunInfo.hWin, g_stperformanceRunInfo.hSoundTrack);

	(MT_VOID)MTADP_Search_FreeAllPmt(g_stperformanceRunInfo.pProgTbl);
    
    (MT_VOID)MTADP_Search_DeInit();
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stperformanceRunInfo.htsThd, NULL);
    
	(MT_VOID)MT_GO_PerformanceDmxDeInit();
    
	(MT_VOID)MTADP_Snd_DeInit();
    
	(MT_VOID)MTADP_VO_DeInit();
    
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd0)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd0);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd1)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd1);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_sub)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_sub);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_still)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_still);
    }

    (MT_VOID)MT_GO_Deinit();
    memset(&g_stperformanceRunInfo, 0, sizeof(g_stperformanceRunInfo));

}

static mt_s32 MT_GO_PerformanceParase_args(int argc, char *argv[], source_param_t *pInutParam)
{
	int opt = 0;
	    
    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
	{
		switch(opt)
		{
			case 'h':
			case '?':
			case 'H':
                (MT_VOID)MT_GO_PerformancePrint_Help(argv[0]);
				return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                   (MT_VOID)MT_GO_PerformanceExit();
                }
                return MT_TASK_EXIT;
            case 'f':                
                MTADP_Strncpy((mt_char*)pInutParam->FileName, mt_optarg, sizeof(source_param_t));
            	break;           
	
			default:
                (MT_VOID)MT_GO_PerformancePrint_Help(argv[0]);
				return MT_FAILURE;
			break;
		}
	}
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoLayerpmMain(mt_s32 argc, mt_char *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif

{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_S32 ret = MT_SUCCESS;
    PMT_COMPACT_PROG *p_stCurrentProgInfo = { 0 };
    source_param_t stParam = { 0 };
    
    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_GO_PerformancePrint_Help(argv[0]);
        return MT_SUCCESS;
    }
	s32Ret = MT_GO_PerformanceParase_args(argc, argv, &stParam);
	if (MT_FAILURE == s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("Parase args err. stop window.\n");
                
        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_PERFORMANCE_ERR_PRINT("Recv stop command. stop window.\n");
        
        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        g_stperformanceRunInfo.hLayer_osd0 = MTGO_INVALID_HANDLE;
        g_stperformanceRunInfo.hLayer_osd1 = MTGO_INVALID_HANDLE;
        g_stperformanceRunInfo.hLayer_sub = MTGO_INVALID_HANDLE;
        g_stperformanceRunInfo.hLayer_still = MTGO_INVALID_HANDLE;
        g_bTaskQuit = MT_FALSE;

#ifndef MT_SAMPLE_APP
        /** Display initialization */
        s32Ret = Sample_MTGO_Display_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("<%s>line [%d]: Display init failed!\n", __FUNCTION__, __LINE__);
            return MT_FAILURE;
        }
#endif    


        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MTADP_Snd_Init\n");
            goto ERR0;    
        }    

        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MTADP_VO_Init\n");
            goto ERR1;    
        }
    
        ret = MT_GO_PerformanceDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT( "failed to StartDmx\n");
            goto ERR2;
        }

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stperformanceRunInfo.htsThd, NULL, (void * (*)(void *))MT_GO_PerformanceInjectTsTask, &stParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR3;
        }
        sleep(1);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR3;
        }
        (void)MTADP_Search_Init();
   
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stperformanceRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MTADP_Search_GetAllPmt\n");
            goto ERR5;
        } 

        ret = MT_GO_PerformanceAVplayInit(&g_stperformanceRunInfo.hAvPlay, &g_stperformanceRunInfo.hWin, &g_stperformanceRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to StartAVPlay\n");
            goto ERR6;
        }
    
        /* Play the first program on the program list*/
        p_stCurrentProgInfo = g_stperformanceRunInfo.pProgTbl->proginfo;
    
        ret = MT_GO_PerformanceAVPlay_Start(g_stperformanceRunInfo.hAvPlay, p_stCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MTADP_AVPlay_PlayProg\n");
            goto ERR7;
        }
        ret = MT_UNF_DISP_SetSmallWindow(MT_UNF_DISPLAY1, 320, 180, 640, 360);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_UNF_DISP_SetSmallWindow\n");
            goto ERR7;
        }
        s32Ret = MT_GO_Init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_PERFORMANCE_ERR_PRINT("failed to MT_GO_Init\n");
            goto ERR7;
        }
    }

    (MT_VOID)MT_GO_PerformanceCmdTask(g_stperformanceRunInfo.hAvPlay, g_stperformanceRunInfo.pProgTbl);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)MT_GO_PerformanceStopplay(g_stperformanceRunInfo.hAvPlay);

    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd0)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd0);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_osd1)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_osd1);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_sub)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_sub);
    }
    if(MTGO_INVALID_HANDLE != g_stperformanceRunInfo.hLayer_still)
    {
        (MT_VOID)MT_GO_DestroyLayer(g_stperformanceRunInfo.hLayer_still);
    }
    
    (MT_VOID)MT_GO_Deinit();
ERR7:
    (MT_VOID)MT_GO_PerformanceAvplayDeInit(g_stperformanceRunInfo.hAvPlay, g_stperformanceRunInfo.hWin, g_stperformanceRunInfo.hSoundTrack);
        
ERR6:
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stperformanceRunInfo.pProgTbl);
         
ERR5:    
    (MT_VOID)MTADP_Search_DeInit();
    
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stperformanceRunInfo.htsThd, NULL); 

ERR3:
    (MT_VOID)MT_GO_PerformanceDmxDeInit();
        
ERR2:    
    (MT_VOID)MTADP_VO_DeInit();
      
ERR1:
    (MT_VOID)MTADP_Snd_DeInit();

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)Sample_MTGO_Display_DeInit();
#endif
    memset(&g_stperformanceRunInfo, 0, sizeof(g_stperformanceRunInfo));
    return MT_SUCCESS;
}
