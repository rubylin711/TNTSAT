/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/times.h>

#include "mt_mpi_sync.h"
#include "mt_module.h"
#include "mt_mpi_mem.h"
#include "mt_drv_struct.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


//static mt_u32 u32SynMutexCount = 0;
static void SYNC_Mutex_Lock(pthread_mutex_t *ss)
{
    //u32SynMutexCount ++;
    //MT_INFO_SYNC("lock u32SynMutexCount:%d\n", u32SynMutexCount);
    pthread_mutex_lock(ss);
}

static void SYNC_Mutex_UnLock(pthread_mutex_t *ss)
{
    //u32SynMutexCount --;
    //MT_INFO_SYNC("unlock u32SynMutexCount:%d\n", u32SynMutexCount);
    pthread_mutex_unlock(ss);
}

static const mt_u8 s_szSyncVersion[] __attribute__((used)) = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";

static mt_s32                 g_SyncDevFd    = -1;
static const mt_char     g_SyncDevName[] ="/dev/"UMAP_DEVNAME_SYNC;
static pthread_mutex_t   g_SyncMutex = PTHREAD_MUTEX_INITIALIZER;

#define MT_SYNC_LOCK()       (void)pthread_mutex_lock(&g_SyncMutex);
#define MT_SYNC_UNLOCK()     (void)pthread_mutex_unlock(&g_SyncMutex);

#define CHECK_SYNC_INIT()\
do{\
    MT_SYNC_LOCK();\
    if (g_SyncDevFd < 0)\
    {\
        MT_ERR_SYNC("SYNC is not init.\n");\
        MT_SYNC_UNLOCK();\
        return MT_ERR_SYNC_DEV_NO_INIT;\
    }\
    MT_SYNC_UNLOCK();\
}while(0)

static mt_s32 SYNC_CheckHandle(mt_handle hSync, SYNC_USR_ADDR_S  *pSyncUsrAddr)
{
    pSyncUsrAddr->SyncId = hSync & 0xff;

    return ioctl(g_SyncDevFd, CMD_SYNC_CHECK_ID, pSyncUsrAddr);
}

static mt_s32 SYNC_GetNum(mt_u32 *pSyncyNum)
{
    return ioctl(g_SyncDevFd, CMD_SYNC_CHECK_NUM, pSyncyNum);
}

static mt_void SYNC_ResetStatInfo(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidFirstCome = MT_FALSE;
        pSync->VidFirstSysTime = MT_INVALID_TIME_U64;
        pSync->VidFirstPts = MT_INVALID_PTS_U64;
        pSync->VidLastPts = MT_INVALID_PTS_U64;
        pSync->VidPreSyncTargetInit = MT_FALSE;
        pSync->VidPreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->VidLocalTimeFlag = MT_FALSE;
        pSync->VidLastSysTime = MT_INVALID_TIME_U64;
        pSync->VidLastLocalTime = MT_INVALID_TIME_U64;
        pSync->VidPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->VidPtsSeriesCnt = 0;

        pSync->CrtBufStatus.VidBufState = SYNC_BUF_STATE_NORMAL;
        pSync->CrtBufStatus.VidBufPercent = 0;
        pSync->CrtBufStatus.bOverflowDiscFrm = MT_FALSE;
        pSync->VidSyndAdjust = MT_FALSE;
        pSync->VidDisPlayCnt = 0;
        pSync->VidDiscardCnt = 0;
        pSync->VidRepPlayCnt = 0;
        pSync->VidRepeatCnt = 0;
        pSync->VidFirstPlay = MT_FALSE;
        pSync->VidFirstPlayTime = MT_INVALID_TIME_U64;
        pSync->VidInfo.SrcPts = MT_INVALID_PTS_U64;
        pSync->VidInfo.Pts = MT_INVALID_PTS_U64;
#if MT_PTS_USE_64_US
        pSync->VidInfo.FrameTime = 40*1000;
        pSync->VidInfo.DelayTime = 20*1000;
        pSync->VidInfo.DispTime = 1*1000;
#else
        pSync->VidInfo.FrameTime = 40;
        pSync->VidInfo.DelayTime = 20;
        pSync->VidInfo.DispTime = 1;
#endif

        pSync->VidFirstValidCome = MT_FALSE;
        pSync->VidFirstValidPts = MT_INVALID_PTS_U64;
        pSync->VidLastSrcPts = MT_INVALID_PTS_U64;
        pSync->VidPtsLoopBack = MT_FALSE;

        pSync->VidFirstDecPts = MT_INVALID_PTS_U64;
        pSync->VidSecondDecPts = MT_INVALID_PTS_U64;
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudFirstCome = MT_FALSE;
        pSync->AudFirstSysTime = MT_INVALID_TIME_U64;
        pSync->AudFirstPts = MT_INVALID_PTS_U64;
        pSync->AudLastPts = MT_INVALID_PTS_U64;
        pSync->AudLastBufTime = 0;
        pSync->AudPreSyncTargetInit = MT_FALSE;
        pSync->AudPreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->AudLocalTimeFlag = MT_FALSE;
        pSync->AudLastSysTime = MT_INVALID_TIME_U64;
        pSync->AudLastLocalTime = MT_INVALID_TIME_U64;
        pSync->AudPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->AudPtsSeriesCnt = 0;

        pSync->CrtBufStatus.AudBufState = SYNC_BUF_STATE_NORMAL;
        pSync->CrtBufStatus.AudBufPercent = 0;

        pSync->AudReSync = MT_TRUE;
        pSync->AudReBufFund = MT_TRUE;
        pSync->AudPlayCnt = 0;
        pSync->AudDiscardCnt = 0;
        pSync->AudRepeatCnt = 0;
        pSync->AudInfo.SrcPts = MT_INVALID_PTS_U64;
        pSync->AudInfo.Pts = MT_INVALID_PTS_U64;
#if MT_PTS_USE_64_US
        pSync->AudInfo.FrameTime = 24*1000;
        pSync->AudInfo.BufTime = 0;
#else
        pSync->AudInfo.FrameTime = 24;
        pSync->AudInfo.BufTime = 0;
#endif
        pSync->AudInfo.FrameNum = 0;
        pSync->AudFirstPlay = MT_FALSE;
        pSync->AudFirstPlayTime = MT_INVALID_TIME_U64;

        pSync->AudFirstValidCome = MT_FALSE;
        pSync->AudFirstValidPts = MT_INVALID_PTS_U64;
        pSync->AudLastSrcPts = MT_INVALID_PTS_U64;
        pSync->AudPtsLoopBack = MT_FALSE;
    }

    if ((!pSync->VidEnable)
      &&(!pSync->AudEnable)
       )
    {
        pSync->CrtStatus = SYNC_STATUS_STOP;
        pSync->PreSyncStartSysTime = MT_INVALID_TIME_U64;
        pSync->PreSyncEndSysTime = MT_INVALID_TIME_U64;
        pSync->PreSyncFinish = MT_FALSE;
        pSync->BufFundEndSysTime = MT_INVALID_TIME_U64;
        pSync->BufFundFinish = MT_FALSE;
        pSync->PreSyncTarget = SYNC_CHAN_VID;
        pSync->PreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->PreSyncTargetInit = MT_FALSE;
        pSync->ExtPreSyncTagetTime = MT_INVALID_TIME_U64;
        pSync->UseExtPreSyncTaget = MT_FALSE;

        pSync->PcrSyncInfo.PcrFirstCome = MT_FALSE;
        pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_TRUE;
        pSync->PcrSyncInfo.PcrFirstSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrFirst = MT_INVALID_PTS_U64;
        pSync->PcrSyncInfo.PcrLast = MT_INVALID_PTS_U64;
        pSync->PcrSyncInfo.PcrLastSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrLastLocalTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrLocalTimeFlag = MT_FALSE;
        pSync->PcrSyncInfo.PcrSeriesCnt = 0;
        pSync->PcrSyncInfo.PcrSyncStartSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrDelta = 0;
        pSync->PcrSyncInfo.enPcrAdjust = SYNC_SCR_ADJUST_BUTT;
        pSync->PcrSyncInfo.PcrLoopBack = MT_FALSE;

        pSync->ScrInitFlag = MT_FALSE;
        pSync->ScrFirstLocalTime = MT_INVALID_TIME_U64;
        pSync->ScrFirstSysTime = MT_INVALID_TIME_U64;
        pSync->ScrLastLocalTime = MT_INVALID_TIME_U64;
        pSync->ScrLastSysTime = MT_INVALID_TIME_U64;

        pSync->AudReSync = MT_FALSE;
        pSync->AudReBufFund = MT_FALSE;

        pSync->SyncEvent.bAudPtsJump = MT_FALSE;
        pSync->SyncEvent.bVidPtsJump = MT_FALSE;
        pSync->SyncEvent.bStatChange = MT_FALSE;
		pSync->SyncEvent.bEos_back = MT_FALSE;
        pSync->LoopBackTime = MT_INVALID_TIME_U64;
        pSync->LoopBackFlag = MT_FALSE;
    }

    return;
}


mt_s32 MT_MPI_SYNC_Init(mt_void)
{
    MT_SYNC_LOCK();

    /* already opened in this process */
    if (g_SyncDevFd > 0)
    {
        MT_SYNC_UNLOCK();
        return MT_SUCCESS;
    }

    g_SyncDevFd = open(g_SyncDevName, O_RDWR|O_NONBLOCK | O_CLOEXEC, 0);

    if (g_SyncDevFd < 0)
    {
        MT_FATAL_SYNC("open SYNC err.\n");
        MT_SYNC_UNLOCK();
        return MT_ERR_SYNC_DEV_OPEN_ERR;
    }

    MT_SYNC_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_DeInit(mt_void)
{
    mt_s32  Ret;
    mt_u32  SyncNum = 0;

    MT_SYNC_LOCK();

    if (g_SyncDevFd < 0)
    {
        MT_SYNC_UNLOCK();
        return MT_SUCCESS;
    }

    SYNC_GetNum(&SyncNum);

    if (SyncNum)
    {
        MT_FATAL_SYNC("there are %d SYNC not been destroied.\n", SyncNum);
        MT_SYNC_UNLOCK();
        return MT_ERR_SYNC_INVALID_OPT;
    }

    Ret = close(g_SyncDevFd);

    if(MT_SUCCESS != Ret)
    {
        MT_FATAL_SYNC("DeInit AVPLAY err.\n");
        MT_SYNC_UNLOCK();
        return MT_ERR_SYNC_DEV_CLOSE_ERR;
    }

    g_SyncDevFd = -1;

    MT_SYNC_UNLOCK();

    return MT_SUCCESS;
}

mt_void MT_MPI_SYNC_GetDefaultAttr(MT_UNF_SYNC_ATTR_S *pstSyncAttr)
{
    pstSyncAttr->enSyncRef = MT_UNF_SYNC_REF_AUDIO;
    pstSyncAttr->enStreamType = MT_UNF_AVPLAY_STREAM_TYPE_TS;
#if (MT_PTS_USE_64_US == 0)
    pstSyncAttr->stSyncStartRegion.s32VidPlusTime = 100;
    pstSyncAttr->stSyncStartRegion.s32VidNegativeTime = -100;
    pstSyncAttr->stSyncStartRegion.bSmoothPlay = MT_FALSE;

    pstSyncAttr->stSyncNovelRegion.s32VidPlusTime = 3000;
    pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime = -3000;
    pstSyncAttr->stSyncNovelRegion.bSmoothPlay = MT_FALSE;

    pstSyncAttr->s32VidPtsAdjust = 0;
    pstSyncAttr->s32AudPtsAdjust = 0;
    pstSyncAttr->u32PreSyncTimeoutMs = 0;
    pstSyncAttr->bQuickOutput = MT_TRUE;
#else
    pstSyncAttr->stSyncStartRegion.s32VidPlusTime = 100 * 1000;
    pstSyncAttr->stSyncStartRegion.s32VidNegativeTime = -100 * 1000;
    pstSyncAttr->stSyncStartRegion.bSmoothPlay = MT_FALSE;

    pstSyncAttr->stSyncNovelRegion.s32VidPlusTime = 3000 * 1000;
    pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime = -3000 * 1000;
    pstSyncAttr->stSyncNovelRegion.bSmoothPlay = MT_FALSE;

    pstSyncAttr->s32VidPtsAdjust = 0;
    pstSyncAttr->s32AudPtsAdjust = 0;
    pstSyncAttr->u32PreSyncTimeoutMs = 0;
    pstSyncAttr->bQuickOutput = MT_TRUE;
#endif
    return;
}

mt_s32 MT_MPI_SYNC_Create(MT_UNF_SYNC_ATTR_S *pstSyncAttr, mt_handle *phSync)
{
    SYNC_S               *pSync = MT_NULL;
    SYNC_CREATE_S        SyncCreate;
    SYNC_USR_ADDR_S      SyncUsrAddr = {0};
    mt_s32                 Ret = 0;

    if (!phSync)
    {
        MT_ERR_SYNC("para phSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pstSyncAttr)
    {
        MT_ERR_SYNC("para pstSyncAttr is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    /* create SYNC */
    Ret = ioctl(g_SyncDevFd, CMD_SYNC_CREATE, &SyncCreate);
    if (Ret != MT_SUCCESS)
    {
        goto RET;
    }

    /* map the memories allocated in kernel space to user space */
    pSync = (SYNC_S *)(mt_mmap(SyncCreate.SyncPhyAddr, 0x1000));
    if (!pSync)
    {
         Ret = MT_ERR_AVPLAY_CREATE_ERR;
         goto SYNC_DESTROY;
    }

    SyncUsrAddr.SyncId = SyncCreate.SyncId;
    SyncUsrAddr.SyncUsrAddr = (ulong)pSync;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_SET_USRADDR, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
         Ret = MT_ERR_SYNC_CREATE_ERR;
         goto SYNC_UNMAP;
    }
    pSync->pSyncMutex = (pthread_mutex_t *)mt_malloc(MT_ID_SYNC, sizeof(pthread_mutex_t));
    if (pSync->pSyncMutex == MT_NULL)
    {
         Ret = MT_ERR_SYNC_CREATE_ERR;
         goto SYNC_UNMAP;
    }

    (mt_void)pthread_mutex_init(pSync->pSyncMutex, NULL);

    SYNC_Mutex_Lock(pSync->pSyncMutex);

    pSync->SyncAttr = *pstSyncAttr;

    pSync->VidEnable = MT_FALSE;
    pSync->AudEnable = MT_FALSE;
    pSync->CrtStatus = SYNC_STATUS_STOP;

    pSync->AudDDPMode = MT_FALSE;

    pSync->bUseStopRegion = MT_TRUE;

    pSync->DebugRender = 0;
    pSync->DebugParam  = 0;

    if (0 == SyncCreate.SyncId)
    {
        pSync->bPrint = MT_TRUE;
    }
    else
    {
        pSync->bPrint = MT_FALSE;
    }

    SYNC_ResetStatInfo(pSync, SYNC_CHAN_VID);
    SYNC_ResetStatInfo(pSync, SYNC_CHAN_AUD);

    *phSync = (MT_ID_SYNC << 16) | SyncCreate.SyncId;

    SYNC_Mutex_UnLock(pSync->pSyncMutex);

    return     MT_SUCCESS;

SYNC_UNMAP:
    (mt_void)mt_munmap((mt_void *)SyncUsrAddr.SyncUsrAddr);

SYNC_DESTROY:
    (mt_void)ioctl(g_SyncDevFd, CMD_SYNC_DESTROY, &(SyncCreate.SyncId));

RET:
    return Ret;
}

mt_s32 MT_MPI_SYNC_Destroy(mt_handle hSync)
{
    SYNC_S               *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr = {0};
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    (mt_void)pthread_mutex_destroy(pSync->pSyncMutex);

    mt_free(MT_ID_SYNC, (mt_void*)(pSync->pSyncMutex));
    
    (mt_void)mt_munmap((mt_void *)SyncUsrAddr.SyncUsrAddr);

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_DESTROY, &SyncUsrAddr.SyncId);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    return MT_SUCCESS ;
}

mt_s32 MT_MPI_SYNC_SetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_SET_AVSYNC_MODE_S    sync_ref_mode;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pstSyncAttr)
    {
        MT_ERR_SYNC("para pstSyncAttr is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (pstSyncAttr->enSyncRef >= MT_UNF_AVPLAY_SYNC_REF_BUTT)
    {
        MT_ERR_SYNC("para pstSyncAttr->enSyncRef is invalid.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS != pstSyncAttr->enStreamType
    	&& MT_UNF_AVPLAY_STREAM_TYPE_ES != pstSyncAttr->enStreamType)
    {
        MT_ERR_SYNC("para pstSyncAttr->enStreamType is no-support.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

	if (MT_UNF_SYNC_REF_NONE != pstSyncAttr->enSyncRef)
	{
	    if (pstSyncAttr->stSyncNovelRegion.s32VidPlusTime < 0)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncNovelRegion.s32VidPlusTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime > 0)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncNovelRegion.s32VidPlusTime > VID_LEAD_DISCARD_THRESHOLD)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncNovelRegion.s32VidPlusTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime < (-VID_LAG_DISCARD_THRESHOLD))
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if ((pstSyncAttr->stSyncNovelRegion.bSmoothPlay!= MT_TRUE)
	      &&(pstSyncAttr->stSyncNovelRegion.bSmoothPlay!= MT_FALSE)
	       )
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncNovelRegion.bSmoothPlay is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncStartRegion.s32VidPlusTime < 0)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncStartRegion.s32VidPlusTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncStartRegion.s32VidNegativeTime > 0)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncStartRegion.s32VidNegativeTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncStartRegion.s32VidPlusTime > VID_LEAD_DISCARD_THRESHOLD)
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncStartRegion.s32VidPlusTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncStartRegion.s32VidNegativeTime < (-VID_LAG_DISCARD_THRESHOLD))
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncStartRegion.s32VidNegativeTime is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if ((pstSyncAttr->stSyncStartRegion.bSmoothPlay!= MT_TRUE)
	      &&(pstSyncAttr->stSyncStartRegion.bSmoothPlay!= MT_FALSE)
	       )
	    {
	        MT_ERR_SYNC("para pstSyncAttr->stSyncStartRegion.bSmoothPlay is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncNovelRegion.s32VidPlusTime <= pstSyncAttr->stSyncStartRegion.s32VidPlusTime)
	    {
	        MT_ERR_SYNC("para stSyncNovelRegion.s32VidPlusTime <= stSyncStartRegion.s32VidPlusTime.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if (pstSyncAttr->stSyncNovelRegion.s32VidNegativeTime >=  pstSyncAttr->stSyncStartRegion.s32VidNegativeTime)
	    {
	        MT_ERR_SYNC("para stSyncNovelRegion.s32VidNegativeTime >= stSyncStartRegion.s32VidNegativeTime.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if ((pstSyncAttr->bQuickOutput != MT_TRUE)
	      &&(pstSyncAttr->bQuickOutput != MT_FALSE)
	       )
	    {
	        MT_ERR_SYNC("para pstSyncAttr->bQuickOutput is invalid.\n");
	        return MT_ERR_SYNC_INVALID_PARA;
	    }

	    if ((pstSyncAttr->u32PreSyncTimeoutMs)
	      &&(pstSyncAttr->u32PreSyncTimeoutMs < PRE_SYNC_MIN_TIME)
	       )
	    {
	        MT_WARN_SYNC("para pstSyncAttr->u32PreSyncTimeoutMs is invalid, modify it to 300.\n");
	        pstSyncAttr->u32PreSyncTimeoutMs = PRE_SYNC_MIN_TIME;
	    }
	}

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

	//check enSyncRef while START
	if (pSync->AudEnable && pSync->VidEnable)
	{
		if (pstSyncAttr->enSyncRef != pSync->SyncAttr.enSyncRef)
		{
			MT_ERR_SYNC("[NOTE]enSyncRef: %d -> %d while START.\n",
				pSync->SyncAttr.enSyncRef,pstSyncAttr->enSyncRef);
		}
	}

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    memcpy(&pSync->SyncAttr, pstSyncAttr, sizeof(MT_UNF_SYNC_ATTR_S));

    sync_ref_mode.SyncId = hSync & 0xff;
    sync_ref_mode.sync_ref_mode = pSync->SyncAttr.enSyncRef;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_SET_AVSYNC_REF_MODE, &sync_ref_mode);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("CMD_SYNC_SET_AVSYNC_MODE failed, Ret = %x\n", Ret);
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_GetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pstSyncAttr)
    {
        MT_ERR_SYNC("para pstSyncAttr is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    memcpy(pstSyncAttr, &pSync->SyncAttr, sizeof(MT_UNF_SYNC_ATTR_S));

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Start(mt_handle hSync, SYNC_CHAN_E enChn)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_u32             SyncId;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (enChn >= SYNC_CHAN_BUTT)
    {
        MT_ERR_SYNC("para enChn is invalid.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncId = hSync & 0xff;

    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidEnable = MT_TRUE;

        Ret = ioctl(g_SyncDevFd, CMD_SYNC_START_SYNC, &SyncId);
        if (Ret != MT_SUCCESS)
        {
            SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
            return Ret;
        }
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudEnable = MT_TRUE;

        Ret = ioctl(g_SyncDevFd, CMD_SYNC_START_SYNC, &SyncId);
        if (Ret != MT_SUCCESS)
        {
            SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
            return Ret;
        }
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Stop(mt_handle hSync, SYNC_CHAN_E enChn)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidEnable = MT_FALSE;
        SYNC_ResetStatInfo(pSync, SYNC_CHAN_VID);
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudEnable = MT_FALSE;
        SYNC_ResetStatInfo(pSync, SYNC_CHAN_AUD);
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Play(mt_handle hSync)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    pSync->CrtStatus = SYNC_STATUS_PLAY;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Seek(mt_handle hSync, mt_u64 u64SeekPts)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    pSync->ExtPreSyncTagetTime = u64SeekPts;
    pSync->UseExtPreSyncTaget = MT_TRUE;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Pause(mt_handle hSync)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_u32             SyncId;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncId = hSync & 0xff;

    pSync->CrtStatus = SYNC_STATUS_PAUSE;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_PAUSE_SYNC, &SyncId);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Tplay(mt_handle hSync )
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    pSync->CrtStatus = SYNC_STATUS_TPLAY;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Resume(mt_handle hSync)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_u32             SyncId;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncId = hSync & 0xff;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_RESUME_SYNC, &SyncId);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    pSync->CrtStatus = SYNC_STATUS_PLAY;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_SetBufState(mt_handle hSync, SYNC_BUF_STATUS_S stBufStatus)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (stBufStatus.VidBufState >= SYNC_BUF_STATE_BUTT)
    {
        MT_ERR_SYNC("VidBufState is invalid.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    if (stBufStatus.AudBufState >= SYNC_BUF_STATE_BUTT)
    {
        MT_ERR_SYNC("AudBufState is invalid.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    memcpy(&(pSync->CrtBufStatus), &stBufStatus, sizeof(SYNC_BUF_STATUS_S));

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_AudJudge(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_AUD_JUDGE_S   SyncAudJudge;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pAudInfo)
    {
        MT_ERR_SYNC("para pAudInfo is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pAudOpt)
    {
        MT_ERR_SYNC("para pAudOpt is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncAudJudge.hSync = hSync;
    SyncAudJudge.AudInfo = *pAudInfo;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_AUD_JUDGE, &SyncAudJudge);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    *pAudOpt = SyncAudJudge.AudOpt;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

//decrease range: 0 to 0x7fff; increase range: 0x8000 to 0x8000+0x7fff;  unit:ms
mt_s32 MT_MPI_SYNC_AptsAdjust(mt_handle hSync, mt_u32 adjust)
{
    SYNC_S  *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_u32  apt_adjust = 0;
    mt_s32  Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    apt_adjust = adjust;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_APTS_ADJUST, &apt_adjust);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_VidJudge(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_VID_JUDGE_S   SyncVidJudge;
    mt_s32             Ret;

    if (MT_INVALID_HANDLE == hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pVidInfo)
    {
        MT_ERR_SYNC("para pAudInfo is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pVidOpt)
    {
        MT_ERR_SYNC("para pAudOpt is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncVidJudge.hSync = hSync;
    SyncVidJudge.VidInfo = *pVidInfo;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_VID_JUDGE, &SyncVidJudge);
    if (MT_SUCCESS != Ret)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    *pVidOpt = SyncVidJudge.VidOpt;

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_GetVFrm_count(mt_handle hSync ,mt_s32 *pcnt)
{

    mt_s32             Ret;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_S             *pSync = MT_NULL;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_GET_VFRM_CNT, pcnt);
    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_GetStatus(mt_handle hSync, MT_UNF_SYNC_STATUS_S *pSyncStatus)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_GET_TIME_INFO_S    SyncTime;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pSyncStatus)
    {
        MT_ERR_SYNC("para pSyncStatus is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    SyncTime.SyncId = hSync & 0xff;
    Ret = ioctl(g_SyncDevFd, CMD_SYNC_GET_TIME_INFO, &SyncTime);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

#if 0
    pSyncStatus->u64FirstAudPts = pSync->AudFirstPts;
    pSyncStatus->u64FirstVidPts = pSync->VidFirstPts;
    pSyncStatus->u64LastAudPts = pSync->AudLastPts;
    pSyncStatus->u64LastVidPts = pSync->VidLastPts;
    pSyncStatus->s64DiffAvPlayTime = pSync->VidAudDiff;
#endif

    pSyncStatus->u64PlayTime = SyncTime.PlayTime;
    pSyncStatus->u64LocalTime = SyncTime.LocalTime;

     pSyncStatus->u64FirstAudPts = SyncTime.AudFirstPts;
    pSyncStatus->u64FirstVidPts = SyncTime.VidFirstPts;
    pSyncStatus->u64LastAudPts = SyncTime.AudLastPts;
    pSyncStatus->u64LastVidPts = SyncTime.VidLastPts;
    pSyncStatus->s64DiffAvPlayTime = SyncTime.VidAudDiff;
    
    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    //printf("TM: [%x] [%llx][%llx] [%llx][%llx] [%llx] [%llx][%llx]\n", (mt_u32)hSync, pSyncStatus->u64FirstAudPts, pSyncStatus->u64FirstVidPts,  pSyncStatus->u64LastAudPts, pSyncStatus->u64LastVidPts,  pSyncStatus->s64DiffAvPlayTime , pSyncStatus->u64PlayTime, pSyncStatus->u64LocalTime);
    
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_SetDDPTestMode(mt_handle hSync, MT_BOOL bEnable)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    (mt_void)SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    pSync->AudDDPMode = bEnable;

    (mt_void)SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_CheckNewEvent(mt_handle hSync, SYNC_EVENT_S *pstEvent)
{
    mt_s32             Ret;
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;

    if (MT_INVALID_HANDLE == hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (MT_NULL == pstEvent)
    {
        MT_ERR_SYNC("para pstEvent is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    (mt_void)SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    memcpy(pstEvent, &(pSync->SyncEvent), sizeof(SYNC_EVENT_S));

    if (pSync->SyncEvent.bVidPtsJump)
    {
        pSync->SyncEvent.bVidPtsJump = MT_FALSE;
    }

    if (pSync->SyncEvent.bAudPtsJump)
    {
        pSync->SyncEvent.bAudPtsJump = MT_FALSE;
    }

    if (pSync->SyncEvent.bStatChange)
    {
        pSync->SyncEvent.bStatChange = MT_FALSE;
    }

    if (pSync->SyncEvent.bEos_back)
    {
        pSync->SyncEvent.bEos_back = MT_FALSE;
    }

    if (pSync->SyncEvent.AudPtsJumpParam.bLoopback)
    {
        pSync->SyncEvent.AudPtsJumpParam.bLoopback = MT_FALSE;
    }

    if (pSync->SyncEvent.VidPtsJumpParam.bLoopback)
    {
        pSync->SyncEvent.VidPtsJumpParam.bLoopback = MT_FALSE;
    }

    (mt_void)SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_SetExtInfo(mt_handle hSync, SYNC_EXT_INFO_E enExtInfo, mt_void *pData)
{
    mt_s32              Ret;

    SYNC_S              *pSync = MT_NULL;
    SYNC_USR_ADDR_S     SyncUsrAddr;

    if (MT_INVALID_HANDLE == hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    (mt_void)SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    switch(enExtInfo)
    {
    case SYNC_EXT_INFO_FIRST_PTS:
        pSync->VidFirstDecPts = (ulong)pData;
        break;
    case SYNC_EXT_INFO_SECOND_PTS:
        pSync->VidSecondDecPts = (ulong)pData;
        break;
    case SYNC_EXT_INFO_STOP_REGION:
        pSync->bUseStopRegion = (ulong)pData;
        break;
    default:
        break;
    }

    (mt_void)SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    return MT_SUCCESS;

}

mt_s32 MT_MPI_SYNC_Push_Apts(mt_handle hSync, SYNC_PUSH_APTS_S *SyncApts)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!SyncApts)
    {
        MT_ERR_SYNC("para SyncApts is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    //SyncTime.SyncId = hSync & 0xff;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_PUSH_APTS, SyncApts);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}



mt_s32 MT_MPI_SYNC_Push_Adec_Apts(mt_handle hSync, SYNC_PUSH_APTS_S *SyncApts)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!SyncApts)
    {
        MT_ERR_SYNC("para SyncApts is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    //SyncTime.SyncId = hSync & 0xff;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_ADEC_PTS, SyncApts);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_Vid_Init(mt_handle hSync, mt_u32 vdec_fmt)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    //SyncTime.SyncId = hSync & 0xff;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_INIT_VID, &vdec_fmt);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_Vid_DeInit(mt_handle hSync)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;
    mt_u32             param = 0;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);


    Ret = ioctl(g_SyncDevFd, CMD_SYNC_DEINIT_VID, &param);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_Aud_Init(mt_handle hSync, mt_u32 adec_fmt)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);


    Ret = ioctl(g_SyncDevFd, CMD_SYNC_INIT_AUD, &adec_fmt);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_Aud_Track(mt_handle hSync, mt_u32 *ptrack)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync || !ptrack)
    {
        MT_ERR_SYNC("%s input para is null.\n", __func__);
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_AUD_TRACK, ptrack);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("CMD_SYNC_AUD_TRACK failed, Ret = %x\n", Ret);
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_SetAVsyncMode(mt_handle hSync, MT_UNF_SYNC_REF_E enAVsyncMode)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    SYNC_SET_AVSYNC_MODE_S    sync_ref_mode;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("%s input para is null.\n", __func__);
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);

    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    sync_ref_mode.SyncId = hSync & 0xff;
    sync_ref_mode.sync_ref_mode = enAVsyncMode;

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_SET_AVSYNC_REF_MODE, &sync_ref_mode);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("CMD_SYNC_SET_AVSYNC_MODE failed, Ret = %x\n", Ret);
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_Aud_DeInit(mt_handle hSync)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;
    mt_u32             param = 0;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);


    Ret = ioctl(g_SyncDevFd, CMD_SYNC_DEINIT_AUD, &param);

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

// @brief Debug SYNC AV Render
// @param[in]  debug                          param
//             0: none                        x
//             1: audio render play           x
//             2: audio render pause          x
//             3: audio render discard        discard frames
//             4: video render play           x
//             5: video render repeat         x
//             6: video render discard        discard frames
mt_s32 MT_MPI_SYNC_Debug(mt_handle hSync, mt_u32 debug, mt_u32 param)
{
	SYNC_S			   *pSync = MT_NULL;
	SYNC_USR_ADDR_S    SyncUsrAddr;
	mt_s32			   Ret;

	if (!hSync)
	{
		MT_ERR_SYNC("para hSync is null.\n");
		return MT_ERR_SYNC_NULL_PTR;
	}

	if (debug > 6)
	{
		MT_ERR_SYNC("para debug is invalid.\n");
		return MT_ERR_SYNC_INVALID_PARA;
	}

	CHECK_SYNC_INIT();

	Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
	if (Ret != MT_SUCCESS)
	{
		return Ret;
	}

	pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

	SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

	pSync->DebugRender = debug;
	pSync->DebugParam  = param;

	SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

	return MT_SUCCESS;
}


mt_s32 MT_MPI_SYNC_GetAVSyncInfo(mt_handle hSync, MT_UNF_SYNC_AV_INFO_S *pSyncAVInfo)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    MT_UNF_SYNC_AV_INFO_S    SyncInfo;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pSyncAVInfo)
    {
        MT_ERR_SYNC("para pSyncStatus is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_GET_AVSYNC_INFO, &SyncInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("%s, CMD_SYNC_GET_AVSYNC_INFO failed, Ret = %x\n", __func__, Ret);
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    pSyncAVInfo->cur_apts = SyncInfo.cur_apts;
    pSyncAVInfo->cur_vpts = SyncInfo.cur_vpts;
	
    MT_INFO_SYNC("%s: [%x][%x] \n", __func__, pSyncAVInfo->cur_vpts, pSyncAVInfo->cur_apts);
	
    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);   
    
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Play_Info_Cfg(mt_handle hSync, mt_u32 *pargs)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pargs) 
    {
        MT_ERR_SYNC("para pargs is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);
    Ret = ioctl(g_SyncDevFd, CMD_SYNC_CFG_PLAY_INFO, pargs);
    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_VFRM_INFO_CAP_Enable(mt_handle hSync, mt_u32 enable)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        fprintf(stderr, "check handle failed %d\n", Ret);
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);
    fprintf(stderr, "do ioctl for vfrm info cap enable\n");
    Ret = ioctl(g_SyncDevFd, CMD_SYNC_VFRM_INFO_CAP_ENABLE, &enable);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        fprintf(stderr, "ioctl CMD_SYNC_VFRM_INFO_CAP_ENABLE failed %d\n", Ret);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_VFRM_INFO_CAP_Read(mt_handle hSync, SYNC_VOUT_FRAME_INFO *info)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;

    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);

    Ret = ioctl(g_SyncDevFd, CMD_SYNC_VFRM_INFO_CAP_READ, info);
    if (Ret != MT_SUCCESS)
    {
        SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
        return Ret;
    }

    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_SYNC_Data_Source_Cfg(mt_handle hSync, MT_SYNC_DATA_SOURCE_E *pargs)
{
    SYNC_S             *pSync = MT_NULL;
    SYNC_USR_ADDR_S    SyncUsrAddr;
    mt_s32             Ret;

    if (!hSync)
    {
        MT_ERR_SYNC("para hSync is null.\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    if (!pargs)
    {
        MT_ERR_SYNC("pargs is null\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    CHECK_SYNC_INIT();

    Ret = SYNC_CheckHandle(hSync, &SyncUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        return Ret;
    }

    pSync = (SYNC_S *)SyncUsrAddr.SyncUsrAddr;
    SYNC_Mutex_Lock((pthread_mutex_t *)pSync->pSyncMutex);
    Ret = ioctl(g_SyncDevFd, CMD_SYNC_SET_AVSYNC_DATA_SOURCE, pargs);
    SYNC_Mutex_UnLock((pthread_mutex_t *)pSync->pSyncMutex);

    return MT_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
