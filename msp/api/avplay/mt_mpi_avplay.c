/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_avplay.c
  Version       : Initial Draft
  Author        : Montage software group
  Created       : 2015/11/25
  Description   : Common definitions of MT_CODEC(video).
                  The codec wants to register to MT_CODEC need to adapt to MT_CODEC_S.
  History       :
  1.Date        : 2015/11/25
  Author      :
  Modification: Created file

*******************************************************************************************/
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
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>

#include "mt_common.h"
#include "mt_mpi_avplay.h"
#include "mt_error_mpi.h"
#include "mt_mpi_mem.h"
#include "mt_module.h"
#include "mt_drv_struct.h"
#include "mt_avplay_frc.h"
#include "../vdec/mt_mpi_vdec_adapter.h"
#include <sys/syscall.h>
#include "mt_mpi_disp.h"
#include "mt_module_debug.h"
#include "mt_drv_demux.h"
#include "mt_drv_adec.h"
#include "mt_common.h"
#include "drv_sync_ioctl.h"
#include "adec/adec_api.h"
#include "mt_mpi_ao.h"
#include "mt_mpi_demux.h"
#include "mt_mpi_hdmi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#ifndef AVPLAY_BUG
#define AVPLAY_BUG()	do { \
							MT_ERR_AVPLAY("[*BUG] %s: AVPlay Bug @%s:%d\n",__FUNCTION__,__FILE__,__LINE__); \
						} while (0)
#endif

#define AVPLAY_AUD_SPEED_ADJUST_SUPPORT

static mt_s32            g_AvplayDevFd    = -1;
static const mt_char     g_AvplayDevName[] ="/dev/"UMAP_DEVNAME_AVPLAY;
static pthread_mutex_t   g_AvplayMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t   g_AvplayResMutex[AVPLAY_MAX_NUM] = {PTHREAD_MUTEX_INITIALIZER};
MT_UNF_AUDIOTRACK_ATTR_S    stTrackInfo;

static const mt_u8 s_szAVPLAYVersion[] __attribute__((used)) = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";

static mt_void AVPLAY_DRV2UNF_VidFrm(MT_DRV_VIDEO_FRAME_S *pstDRVFrm, MT_UNF_VIDEO_FRAME_INFO_S *pstUNFFrm);

void AVPLAY_ThreadMutex_Lock(pthread_mutex_t *ss);
void AVPLAY_ThreadMutex_UnLock(pthread_mutex_t *ss);
void AVPLAY_Mutex_Lock(pthread_mutex_t *ss);
void AVPLAY_Mutex_UnLock(pthread_mutex_t *ss);
mt_u32 AVPLAY_GetSysTime(mt_void);
mt_void AVPLAY_Notify(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_EVENT_E EvtMsg, ulong EvtPara);
MT_BOOL AVPLAY_IsBufEmpty(AVPLAY_S *pAvplay);
MT_UNF_AVPLAY_BUF_STATE_E AVPLAY_CaclBufState(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, mt_u32 UsedBufPercent);
mt_void AVPLAY_ProcAdecToAo(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcAdecToAo2(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_GetWindowByPort(const AVPLAY_S *pAvplay, mt_handle hPort, mt_handle *phWindow);
mt_void AVPLAY_ProcFrmToVirWin(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_ProcVidFrc(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidSync(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidSync_mt(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidPlay(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidQuickOutput(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidRepeat(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidDiscard(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVdecToVo(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_AcquireEs(AVPLAY_S *pAvplay, mt_u32 num,MT_UNF_ES_BUF_S *pEsBuf);
mt_void AVPLAY_ProcDmxToAdec(AVPLAY_S *pAvplay);
mt_void AVPLAY_Eos(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcEos(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcDmxBuf(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcCheckBuf(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcVidEvent(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcAudEvent(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcSyncEvent(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcCheckStandBy(AVPLAY_S *pAvplay);
mt_void AVPLAY_ProcUnloadTime(AVPLAY_S *pAvplay);
mt_void *AVPLAY_StatThread(mt_void *Arg);
mt_void *AVPLAY_DataThread(mt_void *Arg);
mt_void *AVPLAY_VidDataThread(mt_void *Arg);
mt_void AVPLAY_ResetProcFlag(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_CreateThread(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_MallocVdec(AVPLAY_S *pAvplay, const mt_void *pPara);
mt_s32 AVPLAY_FreeVdec(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_MallocAdec(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_FreeAdec(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_MallocDmxChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_BUFID_E BufId);
mt_s32 AVPLAY_FreeDmxChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_BUFID_E BufId);
mt_s32 AVPLAY_MallocVidChn(AVPLAY_S *pAvplay, const mt_void *pPara);
mt_s32 AVPLAY_FreeVidChn(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_MallocAudChn(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_FreeAudChn(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_SetStreamMode(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_S *pAvplayAttr);
mt_s32 AVPLAY_GetStreamMode(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_S *pAvplayAttr);
mt_s32 AVPLAY_SetAdecAttr(AVPLAY_S *pAvplay, const MT_UNF_ACODEC_ATTR_S *pAdecAttr);
mt_s32 AVPLAY_GetAdecAttr(const AVPLAY_S *pAvplay, MT_UNF_ACODEC_ATTR_S *pAdecAttr);
mt_s32 AVPLAY_CheckHandle(mt_handle hAvplay, AVPLAY_USR_ADDR_S  *pAvplayUsrAddr);
mt_s32 AVPLAY_SetVdecAttr(AVPLAY_S *pAvplay, MT_UNF_VCODEC_ATTR_S *pVdecAttr);
mt_s32 AVPLAY_GetVdecAttr(const AVPLAY_S *pAvplay, MT_UNF_VCODEC_ATTR_S *pVdecAttr);
mt_s32 AVPLAY_SetPid(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, const mt_u32 *pPid);
mt_s32 AVPLAY_GetPid(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_u32 *pPid);
mt_s32 AVPLAY_SetSyncAttr(AVPLAY_S *pAvplay, MT_UNF_SYNC_ATTR_S *pSyncAttr);
mt_s32 AVPLAY_GetSyncAttr(AVPLAY_S *pAvplay, MT_UNF_SYNC_ATTR_S *pSyncAttr);
mt_s32 AVPLAY_SetOverflowProc(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_OVERFLOW_E *pOverflowProc);
mt_s32 AVPLAY_GetOverflowProc(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_OVERFLOW_E *pOverflowProc);
mt_s32 AVPLAY_SetLowDelay(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr);
mt_s32 AVPLAY_SetDmxAvsync(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *pDmxAvsync);
mt_s32 AVPLAY_SetDmxAudiosync(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *pDmxAvsync);
mt_s32 AVPLAY_SetWatermarkFilter(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_WATERMARK_FILTER_ATTR_S *pAttr);
mt_s32 AVPLAY_GetLowDelay(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr);
mt_s32 AVPLAY_RelSpecialFrame(AVPLAY_S *pAvplay, mt_handle hWin);
mt_s32 AVPLAY_RelAllVirChnFrame(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_RelAllChnFrame(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_StartVidChn(const AVPLAY_S *pAvplay);
mt_s32 AVPLAY_ResetWindow(const AVPLAY_S *pAvplay, MT_DRV_WIN_SWITCH_E SwitchType);
mt_s32 AVPLAY_FlushWindow(const AVPLAY_S *pAvplay, MT_DRV_WIN_FLUSH_TYPE_E eType);
mt_s32 AVPLAY_StopVidChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_STOP_MODE_E enMode);
mt_s32 AVPLAY_StartAudChn(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_StopAudChn(AVPLAY_S *pAvplay, mt_u32 reset);
mt_void AVPLAY_PrePlay(AVPLAY_S *pAvplay);
mt_void AVPLAY_Play(AVPLAY_S *pAvplay);
mt_void AVPLAY_Stop(AVPLAY_S *pAvplay);
mt_void AVPLAY_Pause(AVPLAY_S *pAvplay);
mt_void AVPLAY_Freeze(AVPLAY_S *pAvplay);
mt_void AVPLAY_Tplay(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_ResetAudChn(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_Seek(AVPLAY_S *pAvplay, mt_u64 u64SeekPts);
mt_s32 AVPLAY_Reset(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_Flush(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_GetNum(mt_u32 *pAvplayNum);
mt_s32 AVPLAY_SetMultiAud(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MULTIAUD_ATTR_S *pAttr);
mt_s32 AVPLAY_GetMultiAud(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MULTIAUD_ATTR_S *pAttr);
mt_s32 AVPLAY_SetEosFlag(AVPLAY_S *pAvplay);
mt_s32 AVPLAY_SetPortAttr(AVPLAY_S *pAvplay, mt_handle hPort, VDEC_PORT_TYPE_E enType);
mt_s32 AVPLAY_CreatePort(AVPLAY_S *pAvplay, mt_handle hWin, VDEC_PORT_ABILITY_E enAbility, mt_handle *phPort);
mt_s32 AVPLAY_DestroyPort(AVPLAY_S *pAvplay, mt_handle hWin, mt_handle hPort);
mt_s32 AVPLAY_SetFrmPackingType(AVPLAY_S *pAvplay, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType);
mt_s32 AVPLAY_GetFrmPackingType(AVPLAY_S *pAvplay, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType);
mt_s32 AVPLAY_SetVdecFrmRateParam(AVPLAY_S *pAvplay,  MT_UNF_AVPLAY_FRMRATE_PARAM_S *pFrmRate);
mt_s32 AVPLAY_GetVdecFrmRateParam(AVPLAY_S *pAvplay,  MT_UNF_AVPLAY_FRMRATE_PARAM_S *pFrmRate);
mt_void AVPLAY_VideoInfo_log(MT_DRV_VIDEO_FRAME_S *pFrm,  mt_u32  line);
mt_s32 AVPLAY_SetDmxBufFullCare(AVPLAY_S *pAvplay, MT_UNF_DMX_BUF_FULL_CARE_S *pDmxBufFullCare);
mt_s32 AVPLAY_GetAVSyncInfo(AVPLAY_S *pAvplay, mt_void *pAVSynInfo);

mt_u32 sys_get_time_stamp_ms(mt_u32 *ms) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // timespec to milliseconds
   	*ms =  (mt_u32)(ts.tv_sec) * 1000 + (uint64_t)(ts.tv_nsec) / 1000000;
	return 0;
}

#define AVPLAY_VID_THREAD 1
//static mt_u32 u32ThreadMutexCount = 0, u32AvplayMutexCount = 0;
void AVPLAY_ThreadMutex_Lock(pthread_mutex_t *ss)
{
    //u32ThreadMutexCount ++;
    //MT_INFO_AVPLAY("lock u32ThreadMutexCount:%d\n", u32ThreadMutexCount);
    pthread_mutex_lock(ss);
}

void AVPLAY_ThreadMutex_UnLock(pthread_mutex_t *ss)
{
    //u32ThreadMutexCount --;
    //MT_INFO_AVPLAY("unlock u32ThreadMutexCount:%d\n", u32ThreadMutexCount);
    pthread_mutex_unlock(ss);
}

void AVPLAY_Mutex_Lock(pthread_mutex_t *ss)
{
    //u32AvplayMutexCount ++;
    //MT_INFO_AVPLAY("lock u32AvplayMutexCount:%d\n", u32AvplayMutexCount);
    pthread_mutex_lock(ss);
}

void AVPLAY_Mutex_UnLock(pthread_mutex_t *ss)
{
    //u32AvplayMutexCount --;
    //MT_INFO_AVPLAY("unlock u32AvplayMutexCount:%d\n", u32AvplayMutexCount);
    pthread_mutex_unlock(ss);
}

#define MT_AVPLAY_LOCK()        (void)pthread_mutex_lock(&g_AvplayMutex);
#define MT_AVPLAY_UNLOCK()      (void)pthread_mutex_unlock(&g_AvplayMutex);

#define MT_AVPLAY_INST_LOCK()        \
do{\
    if((hAvplay & 0xff) >=AVPLAY_MAX_NUM)\
    {\
        MT_ERR_AVPLAY("avplay support %d instance, but this para:%d is illegal\n", AVPLAY_MAX_NUM, (hAvplay & 0xff));\
        return MT_ERR_AVPLAY_INVALID_PARA;\
    }\
    (void)pthread_mutex_lock(&g_AvplayResMutex[(hAvplay & 0xff)]);\
}while(0)

#define MT_AVPLAY_INST_UNLOCK()        \
do{\
    if((hAvplay & 0xff)>=AVPLAY_MAX_NUM)\
    {\
        MT_ERR_AVPLAY("avplay support %d instance, but this para:%d is illegal\n", AVPLAY_MAX_NUM, (hAvplay & 0xff));\
        return MT_ERR_AVPLAY_INVALID_PARA;\
    }\
    (void)pthread_mutex_unlock(&g_AvplayResMutex[(hAvplay & 0xff)]);\
}while(0)

#define AVPLAY_GET_INST_AND_LOCK()\
do{\
    MT_AVPLAY_LOCK();\
    if (g_AvplayDevFd < 0)\
    {\
        MT_ERR_AVPLAY("AVPLAY is not init.\n");\
        MT_AVPLAY_UNLOCK();\
        return MT_ERR_AVPLAY_DEV_NO_INIT;\
    }\
    MT_AVPLAY_UNLOCK();\
    MT_AVPLAY_INST_LOCK(); \
    memset(&AvplayUsrAddr, 0, sizeof(AvplayUsrAddr)); \
    Ret = AVPLAY_CheckHandle(hAvplay, &AvplayUsrAddr);\
    if (Ret != MT_SUCCESS)\
    {\
        MT_AVPLAY_INST_UNLOCK();\
        return MT_ERR_AVPLAY_INVALID_PARA;\
    }\
    pAvplay = (AVPLAY_S *)AvplayUsrAddr.AvplayUsrAddr;\
}while(0)


mt_u32 AVPLAY_GetSysTime(mt_void)
{
    mt_u32      Ticks;
    struct tms  buf;

    /* a non-NULL value is required here */
    Ticks = (mt_u32)times(&buf);

    return Ticks * 10;
}

mt_void AVPLAY_Notify(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_EVENT_E EvtMsg, ulong EvtPara)
{
    if (pAvplay->EvtCbFunc[EvtMsg])
    {
        (mt_void)(pAvplay->EvtCbFunc[EvtMsg](pAvplay->hAvplay, EvtMsg, EvtPara));
    }

    return;
}

MT_BOOL AVPLAY_IsBufEmpty(AVPLAY_S *pAvplay)
{
    ADEC_BUFSTATUS_S            AdecBuf = {0};
    VDEC_STATUSINFO_S           VdecBuf = {0};
    mt_u32                      AudEsBuf = 0;
    mt_u32                      VidEsBuf = 0;
    mt_u32                      VidEsBufWptr = 0;
    mt_u32                      AudEsBufWptr = 0;

    MT_BOOL                     bEmpty = MT_TRUE;
    mt_u32                      Systime = 0;
    mt_u32                      u32TsCnt = 0;
    MT_DRV_WIN_PLAY_INFO_S      WinPlayInfo = {0};
    MT_MPI_DMX_BUF_STATUS_S     VidPesBuf = {0};
    mt_s32                      Ret = MT_SUCCESS;

    WinPlayInfo.u32DelayTime = 0;

    if (pAvplay->AudEnable)
    {
        Ret = MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_BUFFERSTATUS, &AdecBuf);
        if (MT_SUCCESS == Ret)
        {
            AudEsBuf = AdecBuf.u32BufferUsed;
            AudEsBufWptr = AdecBuf.u32BufWritePos;
        }

        if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
        {
            (mt_void)MT_MPI_AO_Track_IsBufEmpty(pAvplay->hSyncTrack, &bEmpty);
        }
    }

    if (pAvplay->VidEnable)
    {
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            (mt_void)MT_MPI_DMX_GetChannelTsCount(pAvplay->hDmxVid, &u32TsCnt);
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidPesBuf);
            if (MT_SUCCESS == Ret)
            {
                VidEsBuf = VidPesBuf.u32UsedSize;
            }
        }
        else
        {
            Ret = MT_MPI_VDEC_GetChanStatusInfo(pAvplay->hVdec, &VdecBuf);
            if (MT_SUCCESS == Ret)
            {
                VidEsBuf = VdecBuf.u32BufferUsed;
            }
        }

        if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
        {
            (mt_void)MT_MPI_WIN_GetPlayInfo(pAvplay->MasterFrmChn.hWindow, &WinPlayInfo);
        }
    }

	//FIXME: MT_MPI_WIN_GetPlayInfo -> WinPlayInfo.u32FrameNumInBufQn is not implement!
    if ((WinPlayInfo.u32FrameNumInBufQn != 0) || (bEmpty != MT_TRUE))
    //if (bEmpty != MT_TRUE)
    {
        pAvplay->CurBufferEmptyState = MT_FALSE;
        return MT_FALSE;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
          if ((AudEsBuf < APPLAY_EOS_BUF_MIN_LEN)
            &&(VidEsBuf < APPLAY_EOS_BUF_MIN_LEN)
            &&(u32TsCnt == pAvplay->PreTscnt )
            &&(AudEsBufWptr == pAvplay->PreAudEsBufWPtr)
             )
          {
              pAvplay->PreAudEsBuf = AudEsBuf;
              pAvplay->PreTscnt = u32TsCnt;
              pAvplay->CurBufferEmptyState = MT_TRUE;
              pAvplay->PreSystime = 0;
              return MT_TRUE;
          }
          else
          {
              if ((AudEsBuf == pAvplay->PreAudEsBuf)
                &&(VidEsBuf == pAvplay->PreVidEsBuf)
                &&(u32TsCnt == pAvplay->PreTscnt)
                &&(AudEsBufWptr == pAvplay->PreAudEsBufWPtr)
                 )
              {
                  Systime = AVPLAY_GetSysTime();

                  if (((Systime > pAvplay->PreSystime) && ((Systime-pAvplay->PreSystime) > AVPLAY_EOS_TIMEOUT))
                    ||((Systime < pAvplay->PreSystime) && (((SYS_TIME_MAX-pAvplay->PreSystime)+Systime) > AVPLAY_EOS_TIMEOUT))
                     )
                  {
                      pAvplay->PreAudEsBuf = AudEsBuf;
                      pAvplay->PreVidEsBuf = VidEsBuf;
                      pAvplay->CurBufferEmptyState = MT_TRUE;
                      pAvplay->PreSystime = 0;
                      return MT_TRUE;
                  }
              }
              else
              {
                  pAvplay->PreAudEsBuf = AudEsBuf;
                  pAvplay->PreVidEsBuf = VidEsBuf;
                  pAvplay->PreTscnt = u32TsCnt;
                  pAvplay->PreAudEsBufWPtr = AudEsBufWptr;
                  pAvplay->PreSystime = AVPLAY_GetSysTime();
              }
          }
    }
    else
    {
        if ((AudEsBuf < APPLAY_EOS_BUF_MIN_LEN)
            &&(VidEsBuf < APPLAY_EOS_BUF_MIN_LEN)
            &&(VidEsBufWptr == pAvplay->PreVidEsBufWPtr)
            &&(AudEsBufWptr == pAvplay->PreAudEsBufWPtr))
        {
            pAvplay->PreAudEsBuf = AudEsBuf;
            pAvplay->PreVidEsBuf = VidEsBuf;
            pAvplay->CurBufferEmptyState = MT_TRUE;
            pAvplay->PreSystime = 0;
            return MT_TRUE;
        }
        else
        {
            if ((AudEsBuf == pAvplay->PreAudEsBuf)
                &&(VidEsBuf == pAvplay->PreVidEsBuf)
                &&(VidEsBufWptr == pAvplay->PreVidEsBufWPtr)
                &&(AudEsBufWptr == pAvplay->PreAudEsBufWPtr)
             )
            {
                Systime = AVPLAY_GetSysTime();

                if (((Systime > pAvplay->PreSystime) && ((Systime-pAvplay->PreSystime) > AVPLAY_EOS_TIMEOUT))
                    ||((Systime < pAvplay->PreSystime) && (((SYS_TIME_MAX-pAvplay->PreSystime)+Systime) > AVPLAY_EOS_TIMEOUT))
                    )
                {
                    pAvplay->PreAudEsBuf = AudEsBuf;
                    pAvplay->PreVidEsBuf = VidEsBuf;
                    pAvplay->CurBufferEmptyState = MT_TRUE;
                    pAvplay->PreSystime = 0;
                    return MT_TRUE;
                }
            }
            else
            {
                pAvplay->PreAudEsBuf = AudEsBuf;
                pAvplay->PreVidEsBuf = VidEsBuf;
                pAvplay->PreVidEsBufWPtr = VidEsBufWptr;
                pAvplay->PreAudEsBufWPtr = AudEsBufWptr;
                pAvplay->PreSystime = AVPLAY_GetSysTime();
            }
        }
    }

    pAvplay->CurBufferEmptyState = MT_FALSE;

    return MT_FALSE;
}

MT_UNF_AVPLAY_BUF_STATE_E AVPLAY_CaclBufState(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, mt_u32 UsedBufPercent)
{
    MT_UNF_AVPLAY_BUF_STATE_E          CurBufState = MT_UNF_AVPLAY_BUF_STATE_NORMAL;
//    VDEC_FRMSTATUSINFO_S               VdecFrmBuf = {0};
    mt_u32                             u32FrmTime = 0;
    MT_DRV_WIN_PLAY_INFO_S             WinInfo;
    mt_s32                             Ret = MT_SUCCESS;

    memset(&WinInfo, 0x0, sizeof(MT_DRV_WIN_PLAY_INFO_S));

    if (MT_UNF_AVPLAY_MEDIA_CHAN_AUD == enChn)
    {
        if (UsedBufPercent >= AVPLAY_ES_AUD_FULL_PERCENT)
        {
            CurBufState = MT_UNF_AVPLAY_BUF_STATE_FULL;
        }
        else if ((UsedBufPercent >= AVPLAY_ES_AUD_HIGH_PERCENT) && (UsedBufPercent < AVPLAY_ES_AUD_FULL_PERCENT))
        {
            CurBufState = MT_UNF_AVPLAY_BUF_STATE_HIGH;
        }
        else if (UsedBufPercent < AVPLAY_ES_AUD_LOW_PERCENT)
        {
           u32FrmTime = pAvplay->AudInfo.FrameNum * pAvplay->AudInfo.FrameTime;

           if (UsedBufPercent < AVPLAY_ES_AUD_EMPTY_PERCENT)
           {
               if (pAvplay->AudInfo.BufTime + u32FrmTime <= 150)
               {
                   CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
               }
               else
               {
                   CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
               }
           }
           else
           {
               if (pAvplay->AudInfo.BufTime + u32FrmTime <= 150)
               {
                   CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
               }
               else if (pAvplay->AudInfo.BufTime + u32FrmTime <= 240)
               {
                   CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
               }
           }
        }
    }
    else
    {
        if (UsedBufPercent >= AVPLAY_ES_VID_FULL_PERCENT)
        {
            CurBufState = MT_UNF_AVPLAY_BUF_STATE_FULL;
        }
        else if ((UsedBufPercent >= AVPLAY_ES_VID_HIGH_PERCENT) && (UsedBufPercent < AVPLAY_ES_VID_FULL_PERCENT))
        {
            CurBufState = MT_UNF_AVPLAY_BUF_STATE_HIGH;
        }
        else if (UsedBufPercent < AVPLAY_ES_VID_LOW_PERCENT)
        {
            if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
            {
                return MT_UNF_AVPLAY_BUF_STATE_LOW;
            }
            Ret = MT_MPI_WIN_GetPlayInfo(pAvplay->MasterFrmChn.hWindow, &WinInfo);
            if (Ret != MT_SUCCESS)
            {
                return MT_UNF_AVPLAY_BUF_STATE_LOW;
            }

            if (WinInfo.u32FrameNumInBufQn <= 1)
            {
                return MT_UNF_AVPLAY_BUF_STATE_EMPTY;
            }

			if((pAvplay->DebugInfo.NoNewFrameCnt > 10) && (UsedBufPercent <= 1))
			{
                return MT_UNF_AVPLAY_BUF_STATE_LOW;
			}

#if 0
            /*InBps is too small*/
            if(VdecFrmBuf.u32StrmInBps < 100)
            {
                u32StrmTime = 0;
            }
            else
            {
                u32StrmTime = (VdecFrmBuf.u32StrmSize * 1000)/(VdecFrmBuf.u32StrmInBps);
            }

            else if (WinInfo.u32FrameNumInBufQn + VdecFrmBuf.u32DecodedFrmNum <= 5)
            {
                if (u32StrmTime <= 80)
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
                }
                else
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
                }
            }
            else if (WinInfo.u32FrameNumInBufQn + VdecFrmBuf.u32DecodedFrmNum <= 10)
            {
                if (u32StrmTime <= 40)
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
                }
                else if (u32StrmTime <= 80)
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
                }
                else
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_NORMAL;
                }
            }
            else
            {
                CurBufState = MT_UNF_AVPLAY_BUF_STATE_NORMAL;
            }

            Ret = MT_MPI_VDEC_GetChanFrmStatusInfo(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, &VdecFrmBuf);
            if (MT_SUCCESS == Ret)
            {
                if (MT_TRUE == pAvplay->VidInfo.bProgressive)
                {
                    u32FrmTime = (VdecFrmBuf.u32DecodedFrmNum + VdecFrmBuf.u32OutBufFrmNum) * pAvplay->VidInfo.FrameTime;
                }
                else
                {
                    u32FrmTime = (2*VdecFrmBuf.u32DecodedFrmNum + VdecFrmBuf.u32OutBufFrmNum) * (pAvplay->VidInfo.FrameTime/2);
                }

                /*InBps is too small*/
                if(VdecFrmBuf.u32StrmInBps < 100)
                {
                    u32StrmTime = 0;
                }
                else
                {
                    u32StrmTime = (VdecFrmBuf.u32StrmSize * 1000)/(VdecFrmBuf.u32StrmInBps);
                }
            }

            if (UsedBufPercent < AVPLAY_ES_VID_EMPTY_PERCENT)
            {
                if((pAvplay->VidInfo.DelayTime < 40) || (pAvplay->VidInfo.DelayTime + u32FrmTime <= 160)
                    || (pAvplay->VidInfo.DelayTime + u32FrmTime + u32StrmTime < 240))
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
                }
                else
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
                }
            }
            else
            {
                if((pAvplay->VidInfo.DelayTime < 40) || (pAvplay->VidInfo.DelayTime + u32FrmTime <= 160)
                     || (pAvplay->VidInfo.DelayTime + u32FrmTime + u32StrmTime < 240))
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
                }
                else if ((pAvplay->VidInfo.DelayTime < 60 || (pAvplay->VidInfo.DelayTime + u32FrmTime) <= 200)
                         || (pAvplay->VidInfo.DelayTime + u32FrmTime + u32StrmTime < 300))
                {
                    CurBufState = MT_UNF_AVPLAY_BUF_STATE_LOW;
                }
            }
#endif
        }
    }

    return CurBufState;
}


static mt_void AVPLAY_AO_FramInfo_log(MT_UNF_AO_FRAMEINFO_S *ptsFrmInfo, mt_u32 line)
{
#if 0
    mt_u32  apts = 0;
    mt_u32  apts_id = 0;
    mt_u32  step = 0;
    mt_u32  index = 0;

     apts = (mt_u32)(ptsFrmInfo->u64PtsMs / 1000 * 45);
     apts_id = ptsFrmInfo->u32FrameCounter;
     index = ptsFrmInfo->u32FrameIndex;

    printf("A: [%d] [%x] [%d][%d] [%d][%d]\n", line, apts, apts_id, index, ptsFrmInfo->u32SampleRate, ptsFrmInfo->u32PcmSamplesPerFrame);

#endif

    return;
}


mt_void AVPLAY_ProcAdecToAo(AVPLAY_S *pAvplay)
{
    mt_s32                  Ret = MT_SUCCESS;
    ADEC_EXTFRAMEINFO_S     AdecExtInfo = {0};
    mt_u32                  AoBufTime = 0;
    ADEC_STATUSINFO_S       AdecStatusinfo;
    mt_u32                  i;
    SYNC_PUSH_APTS_S SyncApts;

    if (!pAvplay->AudEnable)
    {
        return;
    }

    if(MT_INVALID_HANDLE == pAvplay->hSyncTrack)
    {
      return;
    }

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        return;
    }

    memset(&AdecStatusinfo, 0x0, sizeof(ADEC_STATUSINFO_S));
    memset(&stTrackInfo, 0x0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));

    if (!pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO])
    {
        pAvplay->DebugInfo.AcquireAudFrameNum++;

        Ret = MT_MPI_ADEC_ReceiveFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm, &AdecExtInfo);
        //MT_FATAL_AVPLAY("minnan adec to aout receive frame %d!!!\n", Ret);
        if (MT_SUCCESS == Ret)
        {
         // printf("\n  rcv frm %d pts 0x%x\n", pAvplay->AvplayAudFrm.u32FrameCounter, pAvplay->AvplayAudFrm.u32PtsMs);


            pAvplay->AudInfo.SrcPts = AdecExtInfo.u64OrgPtsMs;
            pAvplay->AudInfo.Pts = pAvplay->AvplayAudFrm.u64PtsMs;
            pAvplay->AudInfo.FrameTime = AdecExtInfo.u32FrameDurationMs;

            pAvplay->DebugInfo.AcquiredAudFrameNum++;

			if (pAvplay->DebugInfoExt.pu32AudCrcBuf != NULL)
			{
				if (pAvplay->DebugInfoExt.u32AudCrcBufWrPtr < pAvplay->DebugInfoExt.u32AudCrcBufSize)
				{
					MT_INFO_AVPLAY("Audio PCM CRC: wr %u, crc 0x%08x\n",
						pAvplay->DebugInfoExt.u32AudCrcBufWrPtr,
						pAvplay->AvplayAudFrm.u32DebugCrc);

					pAvplay->DebugInfoExt.pu32AudCrcBuf[pAvplay->DebugInfoExt.u32AudCrcBufWrPtr/sizeof(mt_u32)]
						= pAvplay->AvplayAudFrm.u32DebugCrc;

					pAvplay->DebugInfoExt.u32AudCrcBufWrPtr += sizeof(mt_u32);
				}
				else
				{
					//overflow!
					MT_WARN_AVPLAY("Audio PCM CRC buffer overflow!\n");
				}
			}

            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_TRUE;
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME, (ulong)(&pAvplay->AvplayAudFrm));
            AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
        }
        else
        {
        }
    }

    if (pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO])
    {
        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
        {
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
            pAvplay->AvplayProcContinue = MT_TRUE;
            (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm);
        }
        else
        {
            if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
            {
                (mt_void)MT_MPI_AO_Track_GetDelayMs(pAvplay->hSyncTrack, &AoBufTime);

                (mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_STATUSINFO, &AdecStatusinfo);

                pAvplay->AudInfo.BufTime = AoBufTime;
                pAvplay->AudInfo.FrameNum = AdecStatusinfo.u32UsedBufNum;

#if 0
                Ret = MT_MPI_SYNC_AudJudge(pAvplay->hSync, &pAvplay->AudInfo, &pAvplay->AudOpt);

                if (MT_SUCCESS == Ret)
                {
                    if (SYNC_PROC_DISCARD == pAvplay->AudOpt.SyncProc)
                    {
                        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
                        pAvplay->AvplayProcContinue = MT_TRUE;
                        (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm);
                        return;
                    }
                    else if (SYNC_PROC_REPEAT == pAvplay->AudOpt.SyncProc)
                    {
                        return;
                    }

                    if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
                    {
                        Ret = MT_MPI_AO_Track_GetAttr(pAvplay->hSyncTrack, &stTrackInfo);
                        if ((MT_SUCCESS == Ret) && (MT_UNF_SND_TRACK_TYPE_MASTER == stTrackInfo.enTrackType)
                            && (MT_FALSE == pAvplay->AudDDPMode))  /*do not use speed adjust when ddp test*/
                        {
#ifdef AVPLAY_AUD_SPEED_ADJUST_SUPPORT
                            if (SYNC_AUD_SPEED_ADJUST_NORMAL == pAvplay->AudOpt.SpeedAdjust)
                            {
                                (mt_void)MT_MPI_AO_Track_SetSpeedAdjust(pAvplay->hSyncTrack, 0, MT_MPI_AO_SND_SPEEDADJUST_MUTE);
                                (mt_void)MT_MPI_AO_Track_SetSpeedAdjust(pAvplay->hSyncTrack, 0, MT_MPI_AO_SND_SPEEDADJUST_SRC);
                            }
                            else if (SYNC_AUD_SPEED_ADJUST_UP == pAvplay->AudOpt.SpeedAdjust)
                            {
                                (mt_void)MT_MPI_AO_Track_SetSpeedAdjust(pAvplay->hSyncTrack, 0, MT_MPI_AO_SND_SPEEDADJUST_SRC);
                            }
                            else if (SYNC_AUD_SPEED_ADJUST_DOWN == pAvplay->AudOpt.SpeedAdjust)
                            {
                                (mt_void)MT_MPI_AO_Track_SetSpeedAdjust(pAvplay->hSyncTrack, -10, MT_MPI_AO_SND_SPEEDADJUST_SRC);
                            }
                            else if (SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT == pAvplay->AudOpt.SpeedAdjust)
                            {
                                (mt_void)MT_MPI_AO_Track_SetSpeedAdjust(pAvplay->hSyncTrack, -100, MT_MPI_AO_SND_SPEEDADJUST_MUTE);
                            }
#endif
                        }
                    }
                }
#endif

            }

            pAvplay->DebugInfo.SendAudFrameNum++;

            if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
            {
                /*send frame to main track*/
                pAvplay->AvplayAudFrm.adectype=pAvplay->AdecType;
                Ret = MT_MPI_AO_Track_SendData(pAvplay->hSyncTrack, &pAvplay->AvplayAudFrm);
                if (MT_SUCCESS == Ret)
                {
                    SyncApts.valid = 1;
                    SyncApts.step_u32  = 45000 * pAvplay->AvplayAudFrm.u32PcmSamplesPerFrame /pAvplay->AvplayAudFrm.u32SampleRate;
                    SyncApts.id = pAvplay->AvplayAudFrm.u32FrameCounter;
                    if (pAvplay->AvplayAudFrm.u64PtsMs == 0xffffffffffffffffULL)
                    {
						SyncApts.pts = 0;
						SyncApts.pts_u32 = 0;
                    }
                    else
                    {
						SyncApts.pts = pAvplay->AvplayAudFrm.u64PtsMs;
                        SyncApts.pts_u32 = msecs_to_mt_dvb_pts32((mt_u32)(pAvplay->AvplayAudFrm.u64PtsMs / 1000));
                    }

                    AVPLAY_AO_FramInfo_log(&pAvplay->AvplayAudFrm, __LINE__);

					//debug
					if (0)
					{
						mt_u32 u32Tick;
						sys_get_time_stamp_ms(&u32Tick);
						MT_ALWAYS_PRINT("[%u]A1: [%llx] [%x][%x] [%u]\n", u32Tick,SyncApts.pts, SyncApts.step_u32, SyncApts.pts_u32, SyncApts.id);
					}

					/* EAC3AudTest_new_v1.ts: 0xF0 */
					MT_ASSERT(SyncApts.step_u32 >= 0xB4);//0xB4 to support 96KHz sample rate
                    MT_ASSERT(SyncApts.step_u32 < 0x2000);

                    MT_MPI_SYNC_Push_Apts(pAvplay->hSync, &SyncApts);

                    /*send frame to other track*/
                    pAvplay->AvplayAudFrm.adectype=pAvplay->AdecType;
                    for(i=0; i<pAvplay->TrackNum; i++)
                    {
                        if (pAvplay->hSyncTrack != pAvplay->hTrack[i])
                        {
                            (mt_void)MT_MPI_AO_Track_SendData(pAvplay->hTrack[i], &pAvplay->AvplayAudFrm);
                        }
                    }
                }
            }
            else
            {
                pAvplay->AvplayAudFrm.adectype=pAvplay->AdecType;
                for(i=0; i<pAvplay->TrackNum; i++)
                {
                    Ret = MT_MPI_AO_Track_SendData(pAvplay->hTrack[i], &pAvplay->AvplayAudFrm);
                    if (MT_SUCCESS != Ret)
                    {
                        MT_WARN_AVPLAY("track num %d send data failed\n", i);
                    }
                    else
                    {
                        SyncApts.valid = 1;
                        SyncApts.pts = pAvplay->AvplayAudFrm.u64PtsMs;
                        SyncApts.step_u32  = 45000 *  pAvplay->AvplayAudFrm.u32PcmSamplesPerFrame /pAvplay->AvplayAudFrm.u32SampleRate;
                        SyncApts.id = pAvplay->AvplayAudFrm.u32FrameCounter;
                        SyncApts.pts_u32 = (mt_u32)(pAvplay->AvplayAudFrm.u64PtsMs / 1000 * 45);

                        AVPLAY_AO_FramInfo_log(&pAvplay->AvplayAudFrm, __LINE__);

                       //printf("A2: [%xll] [%x][%x] [%d]\n", SyncApts.pts, SyncApts.step_u32, SyncApts.pts_u32, SyncApts.id);

                       MT_ASSERT(SyncApts.step_u32 > 0x100);
                       MT_ASSERT(SyncApts.step_u32 < 0x2000);

                        MT_MPI_SYNC_Push_Apts(pAvplay->hSync, &SyncApts);



                    }
                }
            }

            if (MT_SUCCESS == Ret)
            {
                pAvplay->DebugInfo.SendedAudFrameNum++;
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
                pAvplay->AvplayProcContinue = MT_TRUE;

                (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm);
            }
            else
            {
                if (MT_ERR_AO_OUT_BUF_FULL != Ret
                    && MT_ERR_AO_SENDMUTE != Ret
                    && MT_ERR_AO_PAUSE_STATE != Ret) /* Error drop this frame */
                {
                    MT_ERR_AVPLAY("Send AudFrame to AO failed:%x, drop a frame.\n", Ret);
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
                    pAvplay->AvplayProcContinue = MT_TRUE;
                    (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm);
                }
            }
        }
    }

    return;
}

#ifdef CONFIG_MT_USE_DATAPIPE_FLOW
mt_void AVPLAY_ProcAdecToAo2(AVPLAY_S *pAvplay)
{
    mt_s32                  Ret = MT_SUCCESS;
    //ADEC_EXTFRAMEINFO_S     AdecExtInfo = {0};
    //mt_u32                  AoBufTime = 0;
    //mt_u32                  i;
    //SYNC_PUSH_APTS_S SyncApts;

    if (!pAvplay->AudEnable)
    {
        return;
    }

    if(MT_INVALID_HANDLE == pAvplay->hSyncTrack)
    {
      return;
    }

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        return;
    }


    if (!pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2])
    {
        //pAvplay->DebugInfo.AcquireAudFrameNum++;

        Ret = MT_MPI_ADEC_ReceiveFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm2, MT_NULL_PTR);
        //MT_FATAL_AVPLAY("minnan adec to aout receive frame %d!!!\n", Ret);
        if (MT_SUCCESS == Ret)
        {
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_TRUE;
        }
    }

    if (pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2])
    {
        // (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm2);
       //  pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
       //  return;
        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
        {
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
            //pAvplay->AvplayProcContinue = MT_TRUE;
            (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm2);
        }
        else
        {
            if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
            {
                /*send frame to main track*/
                pAvplay->AvplayAudFrm2.adectype=pAvplay->AdecType;
                Ret = MT_MPI_AO_Track_SendData(pAvplay->hSyncTrack, &pAvplay->AvplayAudFrm2);
                if (MT_SUCCESS == Ret)
                {
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
                    (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm2);
                }
                else
                {
                    if (MT_ERR_AO_OUT_BUF_FULL != Ret
                        && MT_ERR_AO_SENDMUTE != Ret
                        && MT_ERR_AO_PAUSE_STATE != Ret) /* Error drop this frame */
                    {
                        MT_ERR_AVPLAY("Send AudFrame to AO failed:%x, drop a frame AO2.\n", Ret);
                        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
                        (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm2);
                    }
                }
            }
        }
    }

    return;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_void AVPLAY_ProcAdecToAo3(AVPLAY_S *pAvplay)
{
    mt_s32                  Ret = MT_SUCCESS;

    if (!pAvplay->AudEnable)
    {
        return;
    }

    if(MT_INVALID_HANDLE == pAvplay->hSyncTrack)
    {
      return;
    }

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        return;
    }

    if (!pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3])
    {
        //pAvplay->DebugInfo.AcquireAudFrameNum++;

        Ret = MT_MPI_ADEC_ReceiveFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm3, MT_NULL_PTR);
        //MT_FATAL_AVPLAY("minnan adec to aout receive frame %d!!!\n", Ret);
        if (MT_SUCCESS == Ret)
        {
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_TRUE;
        }
    }

    if (pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3])
    {
        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
        {

            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
            (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm3);
        }
        else
        {
            if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
            {
                /*send frame to main track*/
                pAvplay->AvplayAudFrm3.adectype=pAvplay->AdecType;
                Ret = MT_MPI_AO_Track_SendData(pAvplay->hSyncTrack, &pAvplay->AvplayAudFrm3);
                if (MT_SUCCESS == Ret)
                {
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
                    (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm3);
                }
                else
                {
                    if (MT_ERR_AO_OUT_BUF_FULL != Ret
                        && MT_ERR_AO_SENDMUTE != Ret
                        && MT_ERR_AO_PAUSE_STATE != Ret) /* Error drop this frame */
                    {
                        MT_ERR_AVPLAY("Send AudFrame to AO failed:%x, drop a frame AO3.\n", Ret);
                        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
                        (mt_void)MT_MPI_ADEC_ReleaseFrame(pAvplay->hAdec, &pAvplay->AvplayAudFrm3);
                    }
                }
            }
        }
    }

    return;
}
#endif
#endif

mt_s32 AVPLAY_GetWindowByPort(const AVPLAY_S *pAvplay, mt_handle hPort, mt_handle *phWindow)
{
    mt_u32          i;

    if (pAvplay->MasterFrmChn.hPort == hPort)
    {
        *phWindow = pAvplay->MasterFrmChn.hWindow;
        return MT_SUCCESS;
    }

    for (i=0; i<pAvplay->SlaveChnNum; i++)
    {
        if (pAvplay->SlaveFrmChn[i].hPort == hPort)
        {
            *phWindow = pAvplay->SlaveFrmChn[i].hWindow;
            return MT_SUCCESS;
        }
    }

    for (i=0; i<pAvplay->VirChnNum; i++)
    {
        if (pAvplay->VirFrmChn[i].hPort == hPort)
        {
            *phWindow = pAvplay->VirFrmChn[i].hWindow;
            return MT_SUCCESS;
        }
    }

    *phWindow = MT_INVALID_HANDLE;

    return MT_FAILURE;
}

mt_void AVPLAY_ProcFrmToVirWin(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    pAvplay->bSendedFrmToVirWin = MT_TRUE;

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        for (i = 0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

            for (j=0; j<pAvplay->VirChnNum; j++)
            {
                if (hWindow == pAvplay->VirFrmChn[j].hWindow)
                {
                    pAvplay->DebugInfo.VirVidStat[j].SendNum++;
                    Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                    if (MT_SUCCESS != Ret)
                    {
                        (mt_void)MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                        pAvplay->DebugInfo.VirVidStat[j].DiscardNum++;
                    }
                    else
                    {
                        pAvplay->DebugInfo.VirVidStat[j].PlayNum++;
                    }
                }
            }
        }
    }
    else
    {
        for (i = 0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

            for (j=0; j<pAvplay->VirChnNum; j++)
            {
                if (hWindow == pAvplay->VirFrmChn[j].hWindow)
                {
                    pAvplay->DebugInfo.VirVidStat[j].SendNum++;
                    Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                    if (MT_SUCCESS == Ret)
                    {
#ifdef AVPLAY_VID_THREAD
                        pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
                        pAvplay->AvplayProcContinue = MT_TRUE;
#endif
                        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
                        pAvplay->DebugInfo.VirVidStat[j].PlayNum++;
                    }
                    else if (MT_ERR_VO_BUFQUE_FULL != Ret)
                    {
                       (mt_void)MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
#ifdef AVPLAY_VID_THREAD
                       pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
                       pAvplay->AvplayProcContinue = MT_TRUE;
#endif
                       pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
                       pAvplay->DebugInfo.VirVidStat[j].DiscardNum++;
                    }
                    else
                    {
                        pAvplay->bSendedFrmToVirWin = MT_FALSE;
                    }
                }
            }
        }
    }

    return;
}

mt_s32 AVPLAY_ProcVidFrc(AVPLAY_S *pAvplay)
{
    mt_u32                              i;
    mt_handle                           hWindow = MT_INVALID_HANDLE;
    MT_DRV_WIN_PLAY_INFO_S              WinInfo;
    MT_DRV_VIDEO_PRIVATE_S              *pstVideoPriv = MT_NULL;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S       stFrameRate;

    memset(&WinInfo, 0x0, sizeof(MT_DRV_WIN_PLAY_INFO_S));

    pAvplay->FrcNeedPlayCnt = 1;
    pAvplay->FrcCurPlayCnt = 0;
    pAvplay->FrcCtrlInfo.s32FrmState = 0;

    /* do not do frc in low delay mode */
    if (pAvplay->LowDelayAttr.bEnable)
    {
        return 0;
    }

    /* find the master chan */
    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
        return 0;
    }

    pstVideoPriv = (MT_DRV_VIDEO_PRIVATE_S*)(pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32Priv);

    (mt_void)MT_MPI_WIN_GetPlayInfo(hWindow, &WinInfo);

    memset(&stFrameRate, 0, sizeof(MT_UNF_AVPLAY_FRMRATE_PARAM_S));
    MT_MPI_VDEC_GetChanFrmRate(pAvplay->hVdec, &stFrameRate);

    if (stFrameRate.enFrmRateType == MT_UNF_AVPLAY_FRMRATE_TYPE_USER)
    {
        if ((MT_DRV_FIELD_TOP == pstVideoPriv->eOriginField)
            || (MT_DRV_FIELD_BOTTOM == pstVideoPriv->eOriginField))
        {
            pAvplay->FrcParamCfg.u32InRate = (stFrameRate.stSetFrmRate.u32fpsInteger * 100
                                         + stFrameRate.stSetFrmRate.u32fpsDecimal / 10) * 2;
        }
        else
        {
            pAvplay->FrcParamCfg.u32InRate = stFrameRate.stSetFrmRate.u32fpsInteger * 100
                                         + stFrameRate.stSetFrmRate.u32fpsDecimal / 10;
        }
    }
    else
    {
        pAvplay->FrcParamCfg.u32InRate = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameRate/10;
    }

    pAvplay->FrcParamCfg.u32OutRate = WinInfo.u32DispRate;

#if 0
    if (MT_TRUE == pAvplay->bFrcEnable)
    {
        /*do frc for every new frame*/
        (mt_void)AVPLAY_FrcCalculate(&pAvplay->FrcCalAlg, &pAvplay->FrcParamCfg, &pAvplay->FrcCtrlInfo);

        /* sometimes(such as pvr smooth tplay), vdec set u32PlayTime, means this frame must repeat */
        pAvplay->FrcNeedPlayCnt = (1 + pAvplay->FrcCtrlInfo.s32FrmState) * (1 + pstVideoPriv->u32PlayTime);
    }
#endif //yihua and Rock_hu

    if(!pAvplay->bTargetVideoRendered && pAvplay->TargetTime != MT_INVALID_TIME64) {
        //printf("receiveds pts %llu, target times %u\n",
        //        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts, pAvplay->TargetTime);

        /* Discard any frame in previous of the target frame */
        if(pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts < pAvplay->TargetTime) {
                MT_DBG_AVPLAY("Discard Video pts %llu, target seek time %llu\n",
                            pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts, pAvplay->TargetTime);
                (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                return -1;
        }

        /* find target frame */
        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.repeat_frm_num = 2;
        pAvplay->FrcNeedPlayCnt = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.repeat_frm_num;
    } else {
        //printf("received pts %llu, repeat %u\n",
        //        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts/1000, pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.repeat_frm_num);
    }

	//FIX: Bug 106447
	if (pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.repeat_frm_num != 0)
	{
	    pAvplay->FrcNeedPlayCnt = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.repeat_frm_num;
		if (pAvplay->FrcNeedPlayCnt != 1)
		{
			MT_DBG_AVPLAY("frame repeat num %u\n",pAvplay->FrcNeedPlayCnt);
		}
	}

    return 0;
}

mt_void AVPLAY_ProcVidSync(AVPLAY_S *pAvplay)
{
    mt_u32                              i;
    mt_handle                           hWindow = MT_INVALID_HANDLE;
    MT_DRV_WIN_PLAY_INFO_S              WinInfo;
    MT_DRV_VIDEO_PRIVATE_S              *pstFrmPriv = MT_NULL;

    memset(&WinInfo, 0x0, sizeof(MT_DRV_WIN_PLAY_INFO_S));

    pAvplay->VidOpt.SyncProc = SYNC_PROC_PLAY;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
    }

    /* do not do sync in low delay mode */
    if (pAvplay->LowDelayAttr.bEnable)
    {
        return;
    }

    /* find the master chan */
    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
        return;
    }

    pAvplay->VidInfo.SrcPts = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32SrcPts;
    pAvplay->VidInfo.Pts = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32Pts;

    if (0 != pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameRate)
    {
        pAvplay->VidInfo.FrameTime = 1000000/pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameRate;
    }
    else
    {
        pAvplay->VidInfo.FrameTime = 40;
    }

    pstFrmPriv = (MT_DRV_VIDEO_PRIVATE_S *)(pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32Priv);
    pstFrmPriv->u32PlayTime = 1;

    /* obtain original stream info, judge whether Progressive*/
    if (MT_DRV_FIELD_ALL == pstFrmPriv->eOriginField)
    {
        pAvplay->VidInfo.bProgressive = MT_TRUE;
    }
    else
    {
        pAvplay->VidInfo.bProgressive = MT_FALSE;
    }

    pAvplay->VidInfo.DispTime = pAvplay->FrcNeedPlayCnt;

    /* need to obtain real-time delaytime */
    (mt_void)MT_MPI_WIN_GetPlayInfo(hWindow, &WinInfo);
    pAvplay->VidInfo.DelayTime = WinInfo.u32DelayTime;
    pAvplay->VidInfo.DispRate = WinInfo.u32DispRate;

    (mt_void)MT_MPI_SYNC_VidJudge(pAvplay->hSync, &pAvplay->VidInfo, &pAvplay->VidOpt);

    //printf("V0: [%x][%x][%x] [%x][%x] [%x][%x]\n", pAvplay->AudInfo.SrcPts, pAvplay->AudInfo.Pts, pAvplay->AudInfo.FrameNum, pAvplay->AudInfo.FrameTime, pAvplay->AudInfo.BufTime, pAvplay->AudOpt.SyncProc, pAvplay->AudOpt.SpeedAdjust);
    //printf("V1: [%x][%x][%x] [%x][%x] [%x] [%x]\n", pAvplay->VidInfo.SrcPts, pAvplay->VidInfo.Pts, pAvplay->VidInfo.FrameTime, pAvplay->VidInfo.DispTime, pAvplay->VidInfo.DispRate, pAvplay->VidInfo.DelayTime, pAvplay->VidInfo.bProgressive);
    //printf("V2: [%x] [%x][%x][%x] [%x]\n", pAvplay->VidOpt.SyncProc, pAvplay->VidOpt.Repeat, pAvplay->VidOpt.Discard, pAvplay->VidOpt.VdecDiscardTime, pAvplay->VidOpt.enTBAdjust);

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = pAvplay->VidOpt.enTBAdjust;

        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
        {
            pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
        }
    }

    return;
}

mt_void AVPLAY_ProcVidSync_mt(AVPLAY_S *pAvplay)
{
    mt_u32                              i;
    mt_handle                           hWindow = MT_INVALID_HANDLE;
    MT_DRV_WIN_PLAY_INFO_S              WinInfo;
    MT_DRV_VIDEO_PRIVATE_S              *pstFrmPriv = MT_NULL;

    memset(&WinInfo, 0x0, sizeof(MT_DRV_WIN_PLAY_INFO_S));

    pAvplay->VidOpt.SyncProc = SYNC_PROC_PLAY;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
    }

    /* do not do sync in low delay mode */
    if (pAvplay->LowDelayAttr.bEnable)
    {
        return;
    }

    /* find the master chan */
    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
        return;
    }

    pAvplay->VidInfo.SrcPts = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32SrcPts;
    pAvplay->VidInfo.Pts = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32Pts;

    if (0 != pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameRate)
    {
        pAvplay->VidInfo.FrameTime = 1000000/pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameRate;
    }
    else
    {
        pAvplay->VidInfo.FrameTime = 40;
    }

    pstFrmPriv = (MT_DRV_VIDEO_PRIVATE_S *)(pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32Priv);
    pstFrmPriv->u32PlayTime = 1;

    /* obtain original stream info, judge whether Progressive*/
    if (MT_DRV_FIELD_ALL == pstFrmPriv->eOriginField)
    {
        pAvplay->VidInfo.bProgressive = MT_TRUE;
    }
    else
    {
        pAvplay->VidInfo.bProgressive = MT_FALSE;
    }

    pAvplay->VidInfo.DispTime = pAvplay->FrcNeedPlayCnt;

    /* need to obtain real-time delaytime */
    (mt_void)MT_MPI_WIN_GetPlayInfo(hWindow, &WinInfo);
    pAvplay->VidInfo.DelayTime = WinInfo.u32DelayTime;
    pAvplay->VidInfo.DispRate = WinInfo.u32DispRate;

    //(mt_void)MT_MPI_SYNC_VidJudge(pAvplay->hSync, &pAvplay->VidInfo, &pAvplay->VidOpt);

    //printf("[ztq]V0: [%x][%x][%x] [%x][%x] [%x][%x]\n", pAvplay->AudInfo.SrcPts, pAvplay->AudInfo.Pts, pAvplay->AudInfo.FrameNum, pAvplay->AudInfo.FrameTime, pAvplay->AudInfo.BufTime, pAvplay->AudOpt.SyncProc, pAvplay->AudOpt.SpeedAdjust);
    //printf("[ztq]V1: [%x][%x][%x] [%x][%x] [%x] [%x]\n", pAvplay->VidInfo.SrcPts, pAvplay->VidInfo.Pts, pAvplay->VidInfo.FrameTime, pAvplay->VidInfo.DispTime, pAvplay->VidInfo.DispRate, pAvplay->VidInfo.DelayTime, pAvplay->VidInfo.bProgressive);
    //printf("[ztq]V2: [%x] [%x][%x][%x] [%x]\n", pAvplay->VidOpt.SyncProc, pAvplay->VidOpt.Repeat, pAvplay->VidOpt.Discard, pAvplay->VidOpt.VdecDiscardTime, pAvplay->VidOpt.enTBAdjust);

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = pAvplay->VidOpt.enTBAdjust;

        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
        {
            pAvplay->CurFrmPack.stFrame[i].stFrameVideo.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
        }
    }

    return;
}


static mt_u64 av_get_ms(void)
{
    mt_u64 os_ticks;
    struct  timeval  tv;
    gettimeofday(&tv,NULL);
    os_ticks = tv.tv_sec * 1000 + tv.tv_usec/1000 + 1;
    return os_ticks;
}

mt_void AVPLAY_ProcVidPlay(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;
    mt_ld_event_s                       LdEvent;
	MT_DRV_WIN_SWITCH_E freezeMode;
	MT_BOOL freezEnable;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)//yihua
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
		//FIXME: shall release this frame!
		MT_ERR_AVPLAY("Error, frame not used, shall not happen!\n");
		AVPLAY_BUG();
        return;
    }

    pAvplay->DebugInfo.MasterVidStat.SendNum++;

    //printf("CCCC 2222 frm_cnt = %d, i = %d, with = %d, addr = %x,input_rate = %x \n", pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.frm_cnt, i,
    //  pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pic_width, pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.filedInfoTop.addrLuma,
    //  pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.input_rate);

    //printf("CCCC 2222 slot_idx = %d \n", pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
	  //printf("CCCC 2222 slot_idx = %d \n", pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts);
//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("[%s](%lu): frm_cnt %u, slot %u, pts %llu, eos %u, fps %u\n",__FUNCTION__,gettid(),
			pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.frm_cnt,
			pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.filedInfoTop.slot_idx,
			pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts,
			pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.end_of_stream_flag,
			pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.input_rate);
#endif

	if((pAvplay->trickmode == TM_FREV) &&  (pAvplay->AvplayAttr.stStreamAttr.enStreamType == MT_UNF_AVPLAY_STREAM_TYPE_ES)
		&&(!pAvplay->AudEnable))
	{
		if(500 == pAvplay->u32SpeedDecimal)
		{
			mt_u64								pts_diff = 0;
			mt_u64								ms_time = 0;
			pts_diff = abs(pAvplay->CurFrmPack.stFrame[i].stFrameVideo.slotInfo.pts - pAvplay->LstFrmPack.stFrame[i].stFrameVideo.slotInfo.pts);
			pts_diff = (pts_diff/1000);
			if(pts_diff > 2000)
				pts_diff = 2000;

			ms_time = av_get_ms();
			if(abs(ms_time - pAvplay->LastPlayFrameMs) < pts_diff)
			{
				pAvplay->AvplayVidProcContinue = MT_FALSE;
				return ;
			}
			else
			{
				pAvplay->LastPlayFrameMs = ms_time;
			}
		}
	}

	if(pAvplay->AudEnable)
	{
		Ret= MT_MPI_WIN_GetFreezeStat(hWindow, &freezEnable, &freezeMode);
	    if ((MT_SUCCESS == Ret) && (freezEnable == MT_TRUE))
	    {
	    	MT_UNF_SYNC_AV_INFO_S  AVSynInfo;
			MT_BOOL bWinEnable = 0;
			pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
	    	Ret = MT_MPI_SYNC_GetAVSyncInfo(pAvplay->hSync, (MT_UNF_SYNC_AV_INFO_S *)&AVSynInfo);
			MT_MPI_WIN_GetEnable( hWindow, &bWinEnable);
			if((AVSynInfo.cur_apts != 0) && (AVSynInfo.cur_vpts > AVSynInfo.cur_apts) && bWinEnable )
			{
				pAvplay->AvplayVidProcContinue = MT_FALSE;
				return ;
			}
	    }
	}

    Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
    if (MT_SUCCESS == Ret)
    {
        /* record low delay event */
        LdEvent.evt_id = EVENT_AVPLAY_FRM_OUT;
        LdEvent.frame = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.u32FrameIndex;
        LdEvent.handle = pAvplay->CurFrmPack.stFrame[i].stFrameVideo.hTunnelSrc;
        (mt_void)sys_get_time_stamp_ms(&(LdEvent.time));
        (mt_void)mt_mpi_stat_notify_low_delay_event(&LdEvent);

        /* record program switch event */
        if (pAvplay->CurFrmPack.stFrame[i].stFrameVideo.bIsFirstIFrame)
        {
            mt_mpi_stat_event(STAT_EVENT_VOGETFRM, 0);
        }

         MT_INFO_AVPLAY("Play: queue frame(%u) to master win success!\n",pAvplay->DebugInfo.MasterVidStat.SendNum);

        memcpy(&pAvplay->LstFrmPack, &pAvplay->CurFrmPack, sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));
#ifdef AVPLAY_VID_THREAD
        pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
        pAvplay->AvplayProcContinue = MT_TRUE;
#endif
        pAvplay->FrcCurPlayCnt++;
        pAvplay->DebugInfo.MasterVidStat.PlayNum++;
    }
    else if (MT_ERR_VO_BUFQUE_FULL != Ret)
    {
      //  MT_ERR_AVPLAY("Play: queue frame to master win failed, Ret=%x!\n", Ret);
        if (0 == pAvplay->FrcCurPlayCnt)
        {
            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
        }

#ifdef AVPLAY_VID_THREAD
        pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
        pAvplay->AvplayProcContinue = MT_TRUE;
#endif
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;

        pAvplay->DebugInfo.MasterVidStat.DiscardNum++;
    }
    else
    {
        /* master window is full, do not send to slave window */
        //MT_ERR_AVPLAY("Play: queue frame to master win, master win full!\n");
		/* next loop to send to window again */
        return;
    }

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        for (j=0; j<pAvplay->SlaveChnNum; j++)
        {
            if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
            {
                pAvplay->DebugInfo.SlaveVidStat[j].SendNum++;
                Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                if (MT_SUCCESS != Ret)
                {
                    MT_WARN_AVPLAY("Master queue ok, slave queue failed, Ret=%x!\n", Ret);

                    /*FrcCurPlayCnt maybe has add to 1, because master window send success!*/
                    if (0 == pAvplay->FrcCurPlayCnt || 1 == pAvplay->FrcCurPlayCnt)
                    {
                        Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                        if (MT_SUCCESS != Ret)
                        {
                            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                        }
                    }

                    pAvplay->DebugInfo.SlaveVidStat[j].DiscardNum++;
                }
                else
                {
                    pAvplay->DebugInfo.SlaveVidStat[j].PlayNum++;
                }
            }
        }
    }

    return;
}


mt_void AVPLAY_ProcVidQuickOutput(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
		//FIXME: shall release this frame!
		MT_ERR_AVPLAY("Error, frame not used, shall not happen!\n");
		AVPLAY_BUG();
        return;
    }

    pAvplay->DebugInfo.MasterVidStat.SendNum++;

    Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
    if (MT_SUCCESS == Ret)
    {
#ifdef AVPLAY_VID_THREAD
        pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
        pAvplay->AvplayProcContinue = MT_TRUE;
#endif
        pAvplay->DebugInfo.MasterVidStat.PlayNum++;
        pAvplay->FrcCurPlayCnt++;
    }
    else if (MT_ERR_VO_BUFQUE_FULL != Ret)
    {
        MT_ERR_AVPLAY("Queue frame to master win failed, Ret=%x!\n", Ret);

        if (0 == pAvplay->FrcCurPlayCnt)
        {
            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
        }

#ifdef AVPLAY_VID_THREAD
        pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
        pAvplay->AvplayProcContinue = MT_TRUE;
#endif
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
        pAvplay->DebugInfo.MasterVidStat.DiscardNum++;
    }
    else
    {
        /* master window is full, do not send to slave window */
        //MT_ERR_AVPLAY("Play: queue frame to master win, master win full!\n");
		/* next loop to send to window again */
        return;
    }

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        for (j=0; j<pAvplay->SlaveChnNum; j++)
        {
            if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
            {
                pAvplay->DebugInfo.SlaveVidStat[j].SendNum++;

                Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                if (MT_SUCCESS != Ret)
                {
                    MT_ERR_AVPLAY("Master queue ok, slave queue failed, Ret=%x!\n", Ret);

                    if (0 == pAvplay->FrcCurPlayCnt)
                    {
                        Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                        if (MT_SUCCESS != Ret)
                        {
                            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                        }
                    }

                    pAvplay->DebugInfo.SlaveVidStat[j].DiscardNum++;
                }
                else
                {
                    pAvplay->DebugInfo.SlaveVidStat[j].PlayNum++;
                }
            }
        }
    }

    return;
}



mt_void AVPLAY_ProcVidRepeat(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
        return;
    }

    if (0 == pAvplay->LstFrmPack.u32FrmNum)
    {
        return;
    }

    if (pAvplay->CurFrmPack.stFrame[i].hport != pAvplay->LstFrmPack.stFrame[i].hport)
    {
        return;
    }

    pAvplay->DebugInfo.MasterVidStat.SendNum++;

    Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->LstFrmPack.stFrame[i].stFrameVideo);
    if (MT_SUCCESS != Ret)
    {
        MT_INFO_AVPLAY("Repeat, queue last frame to master win failed, Ret=%x!\n", Ret);
        return;
    }

    pAvplay->DebugInfo.MasterVidStat.RepeatNum++;

    MT_INFO_AVPLAY("Repeat: Queue frame to master win success!\n");

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        for (j=0; j<pAvplay->SlaveChnNum; j++)
        {
            if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
            {
                if (pAvplay->CurFrmPack.stFrame[i].hport != pAvplay->LstFrmPack.stFrame[i].hport)
                {
                    continue;
                }

                pAvplay->DebugInfo.SlaveVidStat[j].SendNum++;

                Ret = MT_MPI_WIN_QueueFrame(hWindow, &pAvplay->LstFrmPack.stFrame[i].stFrameVideo);
                if (MT_SUCCESS != Ret)
                {
                    MT_INFO_AVPLAY("Sync repeat, queue last frame to slave win failed, Ret=%x!\n", Ret);
                }
                else
                {
                    pAvplay->DebugInfo.SlaveVidStat[j].RepeatNum++;
                }
            }
        }
    }

    return;

}

mt_s32 MT_MPI_AVPLAY_SetVOBufferClearnComplete(AVPLAY_S *pAvplay, MT_BOOL bVOClearnFlag)
{
    mt_s32                Ret;

    Ret = MT_MPI_VDEC_SetBuffClearnComp(pAvplay->hVdec, bVOClearnFlag);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_AVPLAY_SetVOBufferClearnComplete failed.\n");
    }

    return Ret;
}

mt_void AVPLAY_ProcVidDiscard(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            break;
        }
    }

    if (i == pAvplay->CurFrmPack.u32FrmNum)
    {
        return;
    }

    pAvplay->DebugInfo.MasterVidStat.SendNum++;

    Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
    if (MT_SUCCESS != Ret)
    {
        MT_INFO_AVPLAY("Discard, queue useless frame to master win failed, Ret=%x!\n", Ret);

        if (MT_ERR_VO_BUFQUE_FULL != Ret)
        {
            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
        }
        else
        {
            return;
        }
    }


    MT_INFO_AVPLAY("Discard, queue useless frame to master win success!\n");

#ifdef AVPLAY_VID_THREAD
    pAvplay->AvplayVidProcContinue = MT_TRUE;
#else
    pAvplay->AvplayProcContinue = MT_TRUE;
#endif
    pAvplay->FrcCurPlayCnt = pAvplay->FrcNeedPlayCnt;
    pAvplay->DebugInfo.MasterVidStat.DiscardNum++;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        for (j=0; j<pAvplay->SlaveChnNum; j++)
        {
            if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
            {
                pAvplay->DebugInfo.SlaveVidStat[j].SendNum++;

                Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                if (MT_SUCCESS != Ret)
                {
                    (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                }

                pAvplay->DebugInfo.SlaveVidStat[j].DiscardNum++;
            }
        }
    }

    return;
}





mt_void AVPLAY_VideoInfo_log(MT_DRV_VIDEO_FRAME_S *pFrm,  mt_u32  line)
{
   //printf("VO: [%d] [%x][%x] [%x][%x] [%llx] [%d] [%x][%x] [%d][%d]\n", line, pFrm->u32FrameNo, pFrm->u32FrameIndex, pFrm->u32Pts, pFrm->u32SrcPts, pFrm->u64Pts,  pFrm->u32FrameRate, pFrm->bProgressive, pFrm->bTopFieldFirst, pFrm->u32Width, pFrm->u32Height);
}



mt_void AVPLAY_ProcVdecToVo(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_ld_event_s                       LdEvent;


    //printf("V3: [%x] [%x][%x][%x] [%x] [%x][%x]\n", pAvplay->VidOpt.SyncProc, pAvplay->VidOpt.Repeat, pAvplay->VidOpt.Discard, pAvplay->VidOpt.VdecDiscardTime, pAvplay->VidOpt.enTBAdjust, pAvplay->FrcCurPlayCnt, pAvplay->FrcNeedPlayCnt, pAvplay->VidEnable, pAvplay->CurStatus);
    if (!pAvplay->VidEnable)//Rock_hu yihua
    {
        return;
    }

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus && pAvplay->AvplayDataPushPause)
    {
        return;
    }

    if (!pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
    {
        pAvplay->DebugInfo.AcquireVidFrameNum++;
#if 0
        if(1 == pAvplay->u32imgChangeFlag)
        {
          mt_handle    hWindow = MT_INVALID_HANDLE;
          mt_u32 Ret;
          int i = 0;
          int found_flag = 0;

          pAvplay->u32imgChangeFlag = 0;
          Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);

          if(pAvplay->CurFrmPack.u32FrmNum < 0)
          {
            pAvplay->CurFrmPack.u32FrmNum = 0;
          }

          if(pAvplay->CurFrmPack.u32FrmNum > 3)
          {
            pAvplay->CurFrmPack.u32FrmNum = 3;
          }

          for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
          {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

            if (hWindow == pAvplay->MasterFrmChn.hWindow)
            {
                found_flag = 1;
                break;
            }
          }

          if(found_flag)
          {
          MT_MPI_WIN_CleanAllFrm(hWindow);
          }
          //MT_MPI_WIN_CleanAllFrm(hWindow);

          i = 0;
          while(1)
          {
            //MT_USLEEP(1000);
            Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);
            //printf("XXXX 8888 Ret = %x \n", Ret);
            if (MT_SUCCESS != Ret)
            {
              if(i < 5)
              {
                i++;
                continue;
              }
              else
              {
                break;
              }
            }
            printf("XXXX 9999 slot_idx = %d \n", pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
            MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[0].hport, &pAvplay->CurFrmPack.stFrame[0].stFrameVideo);
          }

          MT_MPI_AVPLAY_SetVOBufferClearnComplete(pAvplay, MT_TRUE);
          //pAvplay->u32imgChangeFlag = 0;

          return;
        }
#endif

        Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);
        if (MT_SUCCESS != Ret)
        {
            //printf("\n @@@@ 0000 AVPLAY_ProcVdecToVo Ret = %x \n", Ret);
            return;
        }

		if (sem_post(pAvplay->m_vidsem) == -1)
		{
			MT_ERR_AVPLAY("Error, sem_post pAvplay->m_vidsem shall not happen!\n");
		}

        MT_INFO_AVPLAY("%s: [%u, %u] picture_coding_type=%d\n",__FUNCTION__,
		        	pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameNo,
		        	pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameIndex,
		        	pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.picture_coding_type);

        /* record low delay event */
        LdEvent.evt_id = EVENT_AVPLAY_FRM_IN;
        LdEvent.frame = pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameIndex;
        LdEvent.handle = pAvplay->CurFrmPack.stFrame[0].stFrameVideo.hTunnelSrc;
        (mt_void)sys_get_time_stamp_ms(&(LdEvent.time));

        (mt_void)mt_mpi_stat_notify_low_delay_event(&LdEvent);

        /* record program switch event */
        if (pAvplay->CurFrmPack.stFrame[0].stFrameVideo.bIsFirstIFrame)
        {
            mt_mpi_stat_event(STAT_EVENT_AVPLAYGETFRM, 0);
        }


        AVPLAY_VideoInfo_log(&pAvplay->CurFrmPack.stFrame[0].stFrameVideo, __LINE__);


        //printf("=====Receive a new frame, sys=%u, id=%u, pts=%u frameno %d=====\n",
        //    AVPLAY_GetSysTime(), pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameIndex,
        //    pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32Pts, pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameNo);
//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
		VTRACE("[%s](%lu): frm_cnt %u, slot %u, pts %llu, eos %u\n",__FUNCTION__,gettid(),
				pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.frm_cnt,
				pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx,
				pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.pts,
				pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.end_of_stream_flag);
#endif


        Ret = AVPLAY_ProcVidFrc(pAvplay);
        if(Ret == -1)
            return;

        pAvplay->DebugInfo.AcquiredVidFrameNum++;
		if ((pAvplay->DebugInfo.AcquiredVidFrameNum % 250) == 0)
		{
			MT_INFO_AVPLAY("AcquiredVidFrameNum: %u.\n",pAvplay->DebugInfo.AcquiredVidFrameNum);
		}
#if 1
		if (pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME])
		{
			MT_UNF_VIDEO_FRAME_INFO_S			VdecUnfFrm;

			AVPLAY_DRV2UNF_VidFrm(&(pAvplay->LstFrmPack.stFrame[0].stFrameVideo), &VdecUnfFrm);
			if ((pAvplay->DebugInfo.AcquiredVidFrameNum % 250) == 1)
			{
				MT_INFO_AVPLAY("AVPLAY_Notify: Acquired %u Video Frames.\n",pAvplay->DebugInfo.AcquiredVidFrameNum);
			}
			if(pAvplay->DebugInfo.AcquiredVidFrameNum > 0)
			{
				AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME, (ulong)(&VdecUnfFrm));
			}
		}
#endif
        pAvplay->bSendedFrmToVirWin = MT_FALSE;
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_TRUE;
    }

    if (!pAvplay->bSendedFrmToVirWin)
    {
        AVPLAY_ProcFrmToVirWin(pAvplay);
    }

    if (pAvplay->bStepMode)
    {
        if (pAvplay->bStepPlay)
        {
            AVPLAY_ProcVidPlay(pAvplay);

			if (pAvplay->FrcCurPlayCnt >= pAvplay->FrcNeedPlayCnt)
            {
	            pAvplay->bStepPlay = MT_FALSE;
	            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
            }
        }

        return;
    }

    if (0 == pAvplay->FrcCurPlayCnt && 0)
    {
        AVPLAY_ProcVidSync(pAvplay);
    }

    AVPLAY_ProcVidSync_mt(pAvplay);

    /*if ((MT_UNF_VCODEC_TYPE_HEVC == pAvplay->VdecAttr.enType) ||
        ((pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32Width >= 3840) && (pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32Height >= 2160)))
    {
        pAvplay->VidOpt.SyncProc = SYNC_PROC_PLAY;
    }*/

    MT_INFO_AVPLAY("sys:%u, frm:%d, need:%u, cur:%u, sync:%u, delay:%u\n",
        AVPLAY_GetSysTime(),pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u32FrameIndex, pAvplay->FrcNeedPlayCnt,
        pAvplay->FrcCurPlayCnt, pAvplay->VidOpt.SyncProc, pAvplay->VidInfo.DelayTime);


    //printf("V3: [%x] [%x][%x][%x] [%x]\n", pAvplay->VidOpt.SyncProc, pAvplay->VidOpt.Repeat, pAvplay->VidOpt.Discard, pAvplay->VidOpt.VdecDiscardTime, pAvplay->VidOpt.enTBAdjust, pAvplay->FrcCurPlayCnt, pAvplay->FrcNeedPlayCnt);
    if ((pAvplay->FrcCurPlayCnt < pAvplay->FrcNeedPlayCnt)
        || (0 == pAvplay->FrcNeedPlayCnt)
        )
    {
        if (SYNC_PROC_PLAY == pAvplay->VidOpt.SyncProc)
        {
            AVPLAY_ProcVidPlay(pAvplay);
        }
    	    #if 0
        else if (SYNC_PROC_REPEAT == pAvplay->VidOpt.SyncProc)
        {
            AVPLAY_ProcVidRepeat(pAvplay);
        }
        else if (SYNC_PROC_DISCARD == pAvplay->VidOpt.SyncProc)
        {
            AVPLAY_ProcVidDiscard(pAvplay);
        }
		#endif
        else if (SYNC_PROC_QUICKOUTPUT == pAvplay->VidOpt.SyncProc)
        {
            // TODO: remove this to presync to control
            AVPLAY_ProcVidQuickOutput(pAvplay);
        }
		else
		{
			/* next loop to play it. */
		}
    }
	else
	{
		//FIXME: shall release this frame!
		MT_ERR_AVPLAY("Error, frame not used, shall not happen!\n");
		AVPLAY_BUG();
	}

	//FIX: Bug 106447
    if (pAvplay->FrcCurPlayCnt >= pAvplay->FrcNeedPlayCnt)
    {
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
        if(!pAvplay->bTargetVideoRendered && pAvplay->TargetTime != MT_INVALID_TIME64) {
            pAvplay->bTargetVideoRendered = MT_TRUE;
            MT_DBG_AVPLAY("Target video frame %llu rendered\n", pAvplay->CurFrmPack.stFrame[0].stFrameVideo.u64Pts);
        }
    }

    return;
}

FILE               *g_pDmxOutFile = MT_NULL;
int   cooper_file_dmx_open = 0;
#define OPEN_MAGIC_DMX 0x7842561

FILE               *g_pDmxOutFile2 = MT_NULL;
int   cooper_file_dmx_open2 = 0;
#define OPEN_MAGIC_DMX2 0x7842561

#define CHANGE_BETWEEN_LIT_AND_BIG_END(X) ((X >> 24) | \
    (((X >> 16) & 0XFF) << 8) | (((X >> 8) & 0XFF) << 16) | (X << 24))

static FILE *g_debug_aes=NULL;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
/*
 * get PTS Descriptor & packet size by ES Physical Address
 */
static void *avplay_get_aud_pts_desc_by_es_addr(
											    AVPLAY_S *pAvplay,
											    mt_handle dmx_ch,
											    phys_addr_t phy_es_addr,
											    mt_u32 *size)
{
	void *descriptor1 = pAvplay->AudPtsDesc[0];
	void *descriptor2 = pAvplay->AudPtsDesc[1];
	mt_u32 esAddr1;
	mt_u32 esAddr2;
	mt_s32 ret;

	if (descriptor1 == NULL)
	{
		descriptor1 = mpi_demux_desc_next(dmx_ch);
		if (descriptor1 == NULL)
		{
			MT_WARN_AVPLAY("no pts descriptor!\n");
			return NULL;
		}
	}

	do
	{
		if (descriptor2 == NULL)
		{
			descriptor2 = mpi_demux_desc_next(dmx_ch);
		}

		if (descriptor2 == NULL)
			break;

		ret = mpi_demux_desc_get_es_start_addr(descriptor1, &esAddr1);
		ret |= mpi_demux_desc_get_es_start_addr(descriptor2, &esAddr2);

		if (ret == MT_SUCCESS)
		{
			if (esAddr1 < esAddr2)
			{
				if (phy_es_addr >= esAddr1 && phy_es_addr < esAddr2)
				{
					//debug packet size
					//printf("%s: esAddr2(0x%x)-phy_es_addr(0x%x)=0x%x\n",__FUNCTION__,
					//	esAddr2,phy_es_addr,esAddr2-phy_es_addr);

					/* packet size */
					if (size != NULL)
						*size = esAddr2 - phy_es_addr;

					break;
				}
			}
			//rewind
			else if (esAddr2 < esAddr1)
			{
				if (phy_es_addr >= esAddr1 || phy_es_addr < esAddr2)
				{
					/* packet size */
					if (phy_es_addr < esAddr2 && size != NULL)
						*size = esAddr2 - phy_es_addr;

					break;
				}
			}
		}

		mpi_demux_desc_destroy(descriptor1);
		descriptor1 = descriptor2;
		descriptor2 = NULL;

	} while (1);

	pAvplay->AudPtsDesc[0] = descriptor1;
	pAvplay->AudPtsDesc[1] = descriptor2;

	return descriptor1;
}

/*
 * get PTS & packet size from Demux PTS Descriptor by ES Physical Address
 */
static mt_s32 avplay_get_aud_pts_by_es_addr(
											AVPLAY_S *pAvplay,
											mt_handle dmx_ch,
											phys_addr_t phy_es_addr,
											mt_u32 *pts32,
											mt_u64 *pts64,
											mt_u32 *size)
{
	void *descriptor;
	mt_u64 origPTS = 0;
	mt_s32 ret;

	descriptor = avplay_get_aud_pts_desc_by_es_addr(pAvplay, dmx_ch, phy_es_addr, size);
	if (descriptor == NULL)
	{
		return MT_FAILURE;
	}

	ret = mpi_demux_desc_get_orig_pts(descriptor, &origPTS);
	if (ret == MT_SUCCESS)
		*pts32 = (mt_u32)(origPTS >> 1);	/* 33bit -> 32bit */

	ret |= mpi_demux_desc_get_pts64(descriptor, pts64);
#if 0
	mt_u32 esAddr;
	//debug
	if (0)
	{
		mpi_demux_desc_get_es_start_addr(descriptor, &esAddr);

		MT_ALWAYS_PRINT("%s: found ES(0x%08x) - DESC(0x%08x, PTS33 0x%llx, PTS64 0x%llx)\n",__FUNCTION__,
			phy_es_addr,
			esAddr,
			origPTS,
			*pts64);
	}
#endif
	return ret;
}
#endif

/* get ts packet count till pts != u32pts */
static mt_u32 get_pkt_cnt(mt_u8 *data, mt_u32 len, mt_u32 u32pts)
{
    mt_u32 u32pts_tmp;
    mt_u32 offset = DMX_1_PACK_LEN_PTS;

    while (offset < len)
    {
    	u32pts_tmp = *(mt_u32 *)(data + offset + 56);

    	if (u32pts_tmp != u32pts)
    		break;

    	offset += DMX_1_PACK_LEN_PTS;
    }

	//printf("%s: data %p, len %u, offset %u\n",__FUNCTION__,
	//	data, len, offset);

    return (offset / DMX_1_PACK_LEN_PTS);
}

mt_s32 AVPLAY_AcquireEs(AVPLAY_S *pAvplay, mt_u32 num,MT_UNF_ES_BUF_S *pEsBuf)
{
  ulong  dmx_chan_write_pnt = 0x00;
  ulong  dmx_chan_read_pnt = 0x00;
  mt_u32 dmx_chan_free = 0;
  SYNC_PUSH_APTS_S      SyncApts = {0};
  mt_s32 Ret;
  mt_u32 pkt_cnt = 1;

    if (pAvplay->PreDmxAudChn != pAvplay->CurDmxAudChn)
    {
        if (0xffffffff == pAvplay->PreDmxAudChn)
        {
            pAvplay->PreDmxAudChn = pAvplay->CurDmxAudChn;
        }
        else
        {
            mt_u32 pts = 0;
            mt_u32 len = 0;
            mt_u32 diff = 0;
            mt_u32 min_diff = 0xffffffff;
            mt_u32 pre_pts = 0;
            mt_u32 es_rd = 0;
            mt_u32 min_pts = 0;

            Ret = AVPLAY_GetAVSyncInfo(pAvplay, &pAvplay->AVSyncInfo);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("avplay get apts fail\n");
            }

            pEsBuf->pu8Buf = (mt_u8 *)(pAvplay->DmxChanInfoAud[num].dmx_buf_addr);
            MT_DBG_AVPLAY("dmx_chan_read_pnt = %d, play_apts = %#x\n", dmx_chan_read_pnt, pAvplay->AVSyncInfo.cur_vpts);
            len = DMX_1_PACK_LEN_PTS;
            while(1)
            {
                /*
                because apts is inserted in audio es data, the default configuration is inserting a apts every 64 bytes
                so next logic is not perfect, it should read the configuration in dmx registor to get how many bytes
                inserting a apts.
                */
                pts = *((mt_u32 *)(&pEsBuf->pu8Buf[len - 8]));    //the pts at DMX_1_PACK_LEN_PTS*n +56
                pts = CHANGE_BETWEEN_LIT_AND_BIG_END(pts);
                pts = pts << 1;     //the  32bit is  valid-flag

                if (pre_pts != pts)
                {
                    if (pts > (pAvplay->AVSyncInfo.cur_vpts + 200*45))
                    {
                        diff = pts - pAvplay->AVSyncInfo.cur_vpts;
                        MT_DBG_AVPLAY("pts = %#x, dif: %d\n", pts, abs(pts - pAvplay->AVSyncInfo.cur_vpts)/45);
                        if (0xffffffff == min_diff)
                        {
                            min_diff = diff;
                            es_rd = len;
                            min_pts = pts;
                            MT_DBG_AVPLAY("first pts = %#x, dif: %d\n", pts, abs(pts - pAvplay->AVSyncInfo.cur_vpts)/45);
                        }
                        else
                        {
                            if (diff < min_diff)
                            {
                                min_diff = diff;
                                es_rd = len;
                                min_pts = pts;
                            }
                            else
                            {
                                //do nothing
                            }
                        }
                    }
                }
                else
                {
                   //do nothing, there are many same apts in audio es data
                }

                pre_pts = pts;

                if (len < (pAvplay->DmxChanInfoAud[num].dmx_buf_len - DMX_1_PACK_LEN_PTS))
                {
                    len += DMX_1_PACK_LEN_PTS;
                }
                else
                {
                    	MT_ALWAYS_PRINT("find min pts, wr: %#x, es_rd: %#x, pts: %#x, dif: %d\n",
                        MT_MPI_DMX_GetCurrentEsBufferWritePoint(pAvplay->hDmxAud[num]), es_rd, min_pts,
                        abs(min_pts - pAvplay->AVSyncInfo.cur_vpts)/45);
                    break;
                }
            }

            pAvplay->DmxChanInfoAud[num].dmx_buf_read_pnt = es_rd;
            pAvplay->PreDmxAudChn = pAvplay->CurDmxAudChn;

        }
    }

  	dmx_chan_write_pnt = MT_MPI_DMX_GetCurrentEsBufferWritePoint(pAvplay->hDmxAud[num]);
  	dmx_chan_read_pnt = pAvplay->DmxChanInfoAud[num].dmx_buf_read_pnt;

  	MT_DBG_AVPLAY("dmx_buf_addr=0x%x, dmx_buf_len=0x%x, rd=0x%x, wr=0x%x\n",
  	pAvplay->DmxChanInfoAud[num].dmx_buf_addr,
  	pAvplay->DmxChanInfoAud[num].dmx_buf_len,
  	dmx_chan_read_pnt,dmx_chan_write_pnt);

	if(dmx_chan_write_pnt >= dmx_chan_read_pnt)
	{
		dmx_chan_free = dmx_chan_write_pnt - dmx_chan_read_pnt;
	}
	else
	{
		//dmx_chan_free = pAvplay->DmxChanInfoAud[num].dmx_buf_len + dmx_chan_write_pnt - dmx_chan_read_pnt;
		dmx_chan_free = pAvplay->DmxChanInfoAud[num].dmx_buf_len - dmx_chan_read_pnt;
	}

	//align DMX_1_PACK_LEN_PTS
	dmx_chan_free = dmx_chan_free & ((mt_u32)(~(DMX_1_PACK_LEN_PTS-1)));

	if(dmx_chan_free < DMX_1_PACK_LEN_PTS)
		return MT_ERR_DMX_EMPTY_BUFFER;

	pEsBuf->pu8Buf = (mt_u8 *)(pAvplay->DmxChanInfoAud[num].dmx_buf_addr + dmx_chan_read_pnt);
	pEsBuf->es_data_phy_addr = (phys_addr_t)(pAvplay->DmxChanInfoAud[num].dmx_buf_phy_addr+ dmx_chan_read_pnt);


	//pEsBuf->u32BufLen = DMX_1_PACK_LEN_PTS;

	//printf("pEsBuf->pu8Buf 0x%08x\n", pEsBuf->pu8Buf);

	if (MT_MPI_DMX_GetAVsync(pAvplay->hDmxAud[num]))
	{
		// attention  56~59 is pts, so can not use u64 here
		static  mt_u32  g_debug_last_pts_u32 = 0;

		mt_u32 u32pts;
		mt_u64 u64pts_us;

		// get che pts data form es buffer
		u32pts = *((mt_u32 *)(&pEsBuf->pu8Buf[56]));

		pkt_cnt = get_pkt_cnt(pEsBuf->pu8Buf, dmx_chan_free, u32pts);

		u32pts = CHANGE_BETWEEN_LIT_AND_BIG_END(u32pts);
		//printf("%s: u32pts=0x%x\n",__FUNCTION__,u32pts);
		u32pts = u32pts << 1;     //the  32bit is  valid-flag

		// change to 64 bit pts
		u64pts_us = u32pts;
		u64pts_us *= 1000;// change unit to us
		u64pts_us /= 45;

		pEsBuf->u64PtsMs = u64pts_us;

		SyncApts.pts = u64pts_us;
		SyncApts.valid = u64pts_us;
		SyncApts.step = 0x00;
		SyncApts.pts_u32 = u32pts;
		SyncApts.step_u32 = 0x00;

		if(g_debug_last_pts_u32 != u32pts)
		{
		    //printf("D: apts [%llx] [%x][%x]\n", u64pts_us, g_debug_last_pts_u32, u32pts);
		    g_debug_last_pts_u32 = u32pts;
		}

		MT_MPI_SYNC_Push_Adec_Apts(pAvplay->hSync, &SyncApts);

	}
#if defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6)
	/* symphony4 use pts descriptor, no insert pts into es */
	else
	{
		mt_u32 u32pts = 0;
		mt_u64 u64pts_us;
		mt_u32 pkt_size = dmx_chan_free;
		mt_s32 ret;

		/* symphony4 secure can not read es buffer */
		ret = avplay_get_aud_pts_by_es_addr(pAvplay,
										  pAvplay->hDmxAud[num],
										  (phys_addr_t)(ulong)pEsBuf->es_data_phy_addr,
										  &u32pts,
										  &u64pts_us,
										  &pkt_size);
		if (ret == MT_SUCCESS)
		{
			SyncApts.pts = u64pts_us;
			SyncApts.valid = u64pts_us;
			SyncApts.step = 0x00;
			SyncApts.pts_u32 = u32pts;
			SyncApts.step_u32 = 0x00;

			MT_MPI_SYNC_Push_Adec_Apts(pAvplay->hSync, &SyncApts);
		}
		else
		{
		  	//FIXME: ?
			u64pts_us = MT_INVALID_PTS_U64;
		}

		pEsBuf->u64PtsMs = u64pts_us;

		if (pkt_size >= DMX_1_PACK_LEN_PTS)
			pkt_cnt = pkt_size / DMX_1_PACK_LEN_PTS;
	}
#else
	else
	{
		pEsBuf->u64PtsMs = MT_INVALID_PTS_U64;

		pkt_cnt = dmx_chan_free / DMX_1_PACK_LEN_PTS;
	}
#endif

	MT_DBG_AVPLAY("pkt_cnt=%u, pts=%llx\n",pkt_cnt,pEsBuf->u64PtsMs);

	/* max TS Packet count limit */
	if (pkt_cnt > 96)
	  pkt_cnt = 96;

	pEsBuf->u32BufLen = (DMX_1_PACK_LEN_PTS * pkt_cnt);

	pAvplay->DmxChanInfoAud[num].dmx_buf_read_pnt += (DMX_1_PACK_LEN_PTS * pkt_cnt);
	pAvplay->DmxChanInfoAud[num].dmx_buf_read_pnt %= pAvplay->DmxChanInfoAud[num].dmx_buf_len;

	//set dmx read pointer reg
	//for bug 125513
	//  MT_MPI_DMX_SetCurrentEsBufferReadPoint(pAvplay->hDmxAud[num], pAvplay->DmxChanInfoAud[num].dmx_buf_read_pnt);

	return MT_SUCCESS;
}

#ifdef CONFIG_MT_AUDIO_AD
#ifdef CONFIG_MT_USE_DATAPIPE_FLOW

static mt_void AVPLAY_Proc_AD_DmxToAdec(AVPLAY_S *pAvplay)
{
    #define DMX_1_PACK_LEN_PTS 64
    #define DMX_1_PACK_LEN_NOPTS 56

    mt_u32 dmx_chan_write_pnt,dmx_chan_read_pnt;
    mt_u32 dmx_esraw_len_align=0;
    mt_u8 *dmx_esbuff=NULL;
    mt_u32 dmx_esbuffsize=0;
    mt_u32 pcm_freespace = 0;

    if (!pAvplay->AudEnable) {
        return;
    }

    if(MT_INVALID_HANDLE==pAvplay->hDmxAud_AD) {
        return;
    }

    dmx_chan_write_pnt = MT_MPI_DMX_GetCurrentEsBufferWritePoint(pAvplay->hDmxAud_AD);
    dmx_chan_read_pnt  = pAvplay->DmxChanInfoAud_AD.dmx_buf_read_pnt;

    pcm_freespace = ADEC_AD_PCM_GETFREE();
    if(pcm_freespace <= 0) {
        return;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(dmx_chan_write_pnt > dmx_chan_read_pnt) {
        dmx_esraw_len_align = dmx_chan_write_pnt - dmx_chan_read_pnt;
    } else if(dmx_chan_write_pnt < dmx_chan_read_pnt){
        dmx_esraw_len_align = pAvplay->DmxChanInfoAud_AD.dmx_buf_len - dmx_chan_read_pnt;
    } else {
        return;
    }

    if((HA_AUDIO_ID_MP2==pAvplay->adectype_AD) || (HA_AUDIO_ID_MP3==pAvplay->adectype_AD)
       || (HA_AUDIO_ID_AAC==pAvplay->adectype_AD) || (HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)
       || (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD)) {
        /* get phy addr for dma copy */
        MT_MPI_DMX_GetEsBuffPhyAddr(pAvplay->hDmxAud_AD,(ulong*)&dmx_esbuff,&dmx_esbuffsize);

        //dmx_esbuff = (mt_u8 *)(pAvplay->DmxChanInfoAud_AD.dmx_buf_addr);

        /* do align */
        dmx_esraw_len_align = (dmx_esraw_len_align / DMX_1_PACK_LEN_PTS*DMX_1_PACK_LEN_PTS);


        if(dmx_esraw_len_align > pcm_freespace) {
            dmx_esraw_len_align = pcm_freespace / DMX_1_PACK_LEN_PTS*DMX_1_PACK_LEN_PTS;
        } else if(dmx_esraw_len_align == pcm_freespace) {
            /* wr = rd means buffer is empty, so the data feeder should avoid update wr equal to the rd. */
             dmx_esraw_len_align -= DMX_1_PACK_LEN_PTS;
        }

        if(dmx_esraw_len_align > 0) {
            ADEC_AD_PUT_PCM(dmx_esbuff + dmx_chan_read_pnt, dmx_esraw_len_align);
            dmx_chan_read_pnt = ((dmx_chan_read_pnt + dmx_esraw_len_align) % pAvplay->DmxChanInfoAud_AD.dmx_buf_len);
        } else {
            return;
        }
    }
#else
    if(dmx_chan_write_pnt > dmx_chan_read_pnt) {
        dmx_esraw_len_align = dmx_chan_write_pnt - dmx_chan_read_pnt;
    } else if(dmx_chan_write_pnt < dmx_chan_read_pnt){
        dmx_esraw_len_align = pAvplay->DmxChanInfoAud_AD.dmx_buf_len - dmx_chan_read_pnt + dmx_chan_write_pnt;
    } else {
        return;
    }

    //printf("+++ad proc2.es=%x\n",dmx_esraw_len_align);
    if((HA_AUDIO_ID_MP2==pAvplay->adectype_AD)||
       (HA_AUDIO_ID_MP3==pAvplay->adectype_AD)||
       (HA_AUDIO_ID_AAC==pAvplay->adectype_AD)||
       (HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)||
       (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD)) {     //push es to avcpu
        mt_u32 copylen = DMX_1_PACK_LEN_PTS;

        if(MT_MPI_DMX_GetAVsync(pAvplay->hDmxAud_AD))
            copylen = DMX_1_PACK_LEN_NOPTS;

        dmx_esbuff = (mt_u8 *)(pAvplay->DmxChanInfoAud_AD.dmx_buf_addr);
        dmx_esraw_len_align = (dmx_esraw_len_align / DMX_1_PACK_LEN_PTS*DMX_1_PACK_LEN_PTS);

        while(dmx_esraw_len_align) {
            if(copylen < pcm_freespace) {
                ADEC_AD_PUT_PCM(dmx_esbuff+dmx_chan_read_pnt, copylen);
                pcm_freespace -= copylen;
                dmx_chan_read_pnt = ((dmx_chan_read_pnt + DMX_1_PACK_LEN_PTS) % pAvplay->DmxChanInfoAud_AD.dmx_buf_len);
                dmx_esraw_len_align -= DMX_1_PACK_LEN_PTS;
            } else {
                break;
            }
        }
    }
#endif
    else {                                              //free es
        dmx_chan_read_pnt=dmx_esraw_len_align;
    }
    pAvplay->DmxChanInfoAud_AD.dmx_buf_read_pnt=dmx_chan_read_pnt;
    MT_MPI_DMX_SetCurrentEsBufferReadPoint(pAvplay->hDmxAud_AD, dmx_chan_read_pnt);
}
#endif
#endif

mt_void AVPLAY_ProcDmxToAdec(AVPLAY_S *pAvplay)
{
    MT_UNF_STREAM_BUF_S             AdecEsBuf = {0};
    mt_s32                          Ret = MT_FAILURE;
    mt_u32                          i = 0;
    MT_UNF_ES_BUF_S         AudDmxEsBuf = {0};


    /* AVPLAY_Start: pAvplay->AudEnable = MT_TRUE */
    if (!pAvplay->AudEnable)
    {
        return;
    }

    Ret = MT_MPI_ADEC_GetDelayMs(pAvplay->hAdec, &pAvplay->AdecDelayMs);
    if (MT_SUCCESS == Ret && pAvplay->AdecDelayMs > AVPLAY_ADEC_MAX_DELAY)
    {
        return;
    }

    if (!pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC])
    {
        for(i=0; i<pAvplay->DmxAudChnNum; i++)
        {
            if(i == pAvplay->CurDmxAudChn)
            {
                pAvplay->DebugInfo.AcquireAudEsNum++;

                Ret = AVPLAY_AcquireEs(pAvplay, i, &(pAvplay->AvplayDmxEsBuf));
                //Ret = MT_ERR_DMX_EMPTY_BUFFER;
                if (MT_SUCCESS == Ret)
                {
                    pAvplay->DebugInfo.AcquiredAudEsNum++;
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC] = MT_TRUE;
                }
                else
                {
                    /*if is eos and there is no data in demux channel, set eos to adec and ao*/
                    if (MT_ERR_DMX_EMPTY_BUFFER == Ret
                        && pAvplay->bSetEosFlag && !pAvplay->bSetAudEos)
                    {
                        Ret = MT_MPI_ADEC_SetEosFlag(pAvplay->hAdec);
                        if (MT_SUCCESS != Ret)
                        {
                            MT_ERR_AVPLAY("ERR: MT_MPI_ADEC_SetEosFlag, Ret = %x! \n", Ret);
                            return;
                        }

                        if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
                        {
                            Ret = MT_MPI_AO_Track_SetEosFlag(pAvplay->hSyncTrack, MT_TRUE);
                            if (MT_SUCCESS != Ret)
                            {
                                MT_ERR_AVPLAY("ERR: MT_MPI_HIAO_SetEosFlag, Ret = %x! \n", Ret);
                                return;
                            }
                        }

                        pAvplay->bSetAudEos = MT_TRUE;
                    }
                }
            }
            else
            {
                Ret = MT_MPI_DMX_AcquireEs(pAvplay->hDmxAud[i], &AudDmxEsBuf);
                if (MT_SUCCESS == Ret)
                {
                    (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[i], &AudDmxEsBuf);
                }
            }
        }
    }

    if (pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC])
    {
        AdecEsBuf.pu8Data = pAvplay->AvplayDmxEsBuf.pu8Buf;
        AdecEsBuf.u32PhyData = pAvplay->AvplayDmxEsBuf.es_data_phy_addr;
        AdecEsBuf.u32Size = pAvplay->AvplayDmxEsBuf.u32BufLen;

        pAvplay->DebugInfo.SendAudEsNum++;

        /* for DDP test only, when ts stream revers(this pts < last pts),
            reset audChn, and buffer 600ms audio stream  */
        if (pAvplay->AudDDPMode)
        {
            static mt_u32 s_u32LastPtsTime = 0;

            mt_u64 thisPts = pAvplay->AvplayDmxEsBuf.u64PtsMs;
            mt_u32 thisPtsTime = AVPLAY_GetSysTime();
            mt_s32 ptsDiff = 0;

            if ((thisPts < pAvplay->LastAudPts) && (pAvplay->LastAudPts != MT_INVALID_PTS_U64)
                && (thisPts != MT_INVALID_PTS_U64)
                )
            {
                MT_ERR_AVPLAY("PTS:%u -> %u, PtsLess.\n ", pAvplay->LastAudPts, thisPts);
                //(mt_void)MT_MPI_SYNC_Aud_Init(pAvplay->hSync, 3);
                (mt_void)AVPLAY_ResetAudChn(pAvplay);
                MT_USLEEP(1200*1000);
                MT_ERR_AVPLAY("Rest OK.\n");
            }
            else
            {
                if ( thisPtsTime >  s_u32LastPtsTime)
                {
                    ptsDiff = (mt_s32)(thisPtsTime - s_u32LastPtsTime);
                }
                else
                {
                    ptsDiff = 0;
                }
                if ( ptsDiff > 1000 )
                {
                    MT_ERR_AVPLAY("PtsTime:%u -> %u, Diff:%d.\n ", s_u32LastPtsTime, thisPtsTime, ptsDiff);
                    //(mt_void)MT_MPI_SYNC_Aud_Init(pAvplay->hSync, 3);
                    (mt_void)AVPLAY_ResetAudChn(pAvplay);
                    MT_USLEEP(1200*1000);
                    MT_ERR_AVPLAY("Rest OK.\n");
                    s_u32LastPtsTime = MT_INVALID_PTS;
                    pAvplay->LastAudPts = MT_INVALID_PTS_U64;

                }
            }

            if (thisPts != MT_INVALID_PTS_U64)
            {
                pAvplay->LastAudPts = thisPts;
                s_u32LastPtsTime = thisPtsTime;
            }
        }

        Ret = MT_MPI_ADEC_SendStream(pAvplay->hAdec, &AdecEsBuf, pAvplay->AvplayDmxEsBuf.u64PtsMs);
        if (MT_SUCCESS == Ret)
        {
        	//for bug 125513
        	MT_MPI_DMX_SetCurrentEsBufferReadPoint(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], pAvplay->DmxChanInfoAud[pAvplay->CurDmxAudChn].dmx_buf_read_pnt);
            pAvplay->DebugInfo.SendedAudEsNum++;
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC] = MT_FALSE;
            pAvplay->AvplayProcContinue = MT_TRUE;

             if(0)
              {
                if(OPEN_MAGIC_DMX != cooper_file_dmx_open)
                {
                  cooper_file_dmx_open = OPEN_MAGIC_DMX;
                  g_pDmxOutFile = fopen("1dmx_test_dmx_write.mp3", "wb");
                  if (!g_pDmxOutFile)
                  {
                      MT_FATAL_ADEC("open file %s error!\n", "1dmx_test_dmx_write.mp3");
                  }
                }

                if(g_pDmxOutFile && AdecEsBuf.u32Size)
                {
                  //printf(" get es data 0x%x, %d \n", AdecEsBuf.pu8Data, AdecEsBuf.u32Size);
                  fwrite(AdecEsBuf.pu8Data, 1, DMX_1_PACK_LEN_NOPTS, g_pDmxOutFile);
                }
              }
            //AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
            //AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_GET_AUD_ES, (mt_u32)(&pAvplay->AvplayDmxEsBuf));
            //AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
            //(mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &pAvplay->AvplayDmxEsBuf);
        }
        else
        {
            if ((Ret != MT_ERR_ADEC_IN_BUF_FULL) && (Ret != MT_ERR_ADEC_IN_PTSBUF_FULL)) /* drop this pkg */
            {
                MT_ERR_AVPLAY("Send AudEs buf to ADEC fail:%x, drop a pkg.\n", Ret);
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC] = MT_FALSE;
                pAvplay->AvplayProcContinue = MT_TRUE;
                (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &pAvplay->AvplayDmxEsBuf);
            }
        }
    }
    return;
}

mt_void AVPLAY_Eos(AVPLAY_S *pAvplay)
{
    pAvplay->PreAudEsBuf = 0;
    pAvplay->PreVidEsBuf = 0;
    pAvplay->PreSystime = 0;
    pAvplay->PreVidEsBufWPtr= 0;
    pAvplay->PreAudEsBufWPtr= 0;
    pAvplay->CurBufferEmptyState = MT_TRUE;
    pAvplay->LstStatus = pAvplay->CurStatus;
    pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_EOS;

    return;
}

mt_void AVPLAY_ProcEos(AVPLAY_S *pAvplay)
{
    MT_BOOL                 bEmpty = MT_TRUE;
#if 0
    ADEC_BUFSTATUS_S        AdecBuf;
    ADEC_STATUSINFO_S       AdecStatus = {0};
    VDEC_STATUSINFO_S       VdecStatus= {0};
    MT_BOOL                 bVidEos = MT_TRUE;
    MT_BOOL                 bAudEos = MT_TRUE;
    MT_DRV_WIN_PLAY_INFO_S  WinPlayInfo = {0};

    if (pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_EOS)
    {
        return;
    }

    memset(&AdecBuf, 0x0, sizeof(ADEC_BUFSTATUS_S));

    if (pAvplay->AudEnable)
    {
        bAudEos = MT_FALSE;

        //(mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_BUFFERSTATUS, &AdecBuf);//Rocck_hu
        //(mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_STATUSINFO, &AdecStatus);

        if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
        {
            (mt_void)MT_MPI_AO_Track_IsBufEmpty(pAvplay->hSyncTrack, &bEmpty);
        }

        if (AdecBuf.bEndOfFrame && (AdecStatus.u32UsedBufNum == 0))
        {
            if (MT_TRUE == bEmpty)
            {
                bAudEos = MT_TRUE;
            }
        }
    }

    if (pAvplay->VidEnable)
    {
        bVidEos = MT_FALSE;

        (mt_void)MT_MPI_VDEC_GetChanStatusInfo(pAvplay->hVdec,  &VdecStatus);

        if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
        {
            (mt_void)MT_MPI_WIN_GetPlayInfo(pAvplay->MasterFrmChn.hWindow, &WinPlayInfo);
        }

		MT_INFO_AVPLAY("Video, EOS %d, Port Complete %d, Win Frame Num %d\n",
				VdecStatus.bEndOfStream, VdecStatus.bAllPortCompleteFrm,
				WinPlayInfo.u32FrameNumInBufQn);

        if (VdecStatus.bEndOfStream && VdecStatus.bAllPortCompleteFrm)
        {
//FIXME: WinPlayInfo.u32FrameNumInBufQn is not implemented!
#if 0
            if (0 == WinPlayInfo.u32FrameNumInBufQn)
#endif
            {
                bVidEos = MT_TRUE;
            }
        }
    }
#endif

	if((pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_PAUSE) && (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType))
	{
		return;
	}

    bEmpty = AVPLAY_IsBufEmpty(pAvplay);
// for bug 130532 audio only case
    if(bEmpty )
    {
		do
		{
			if(pAvplay->VidEnable && (pAvplay->bVideoLastFrameDecoded != MT_TRUE)
				&& (pAvplay->DebugInfo.LastAcquiredVidFrameNum != pAvplay->DebugInfo.AcquiredVidFrameNum))
				break;

	        AVPLAY_Eos(pAvplay);
	        AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_EOS, MT_NULL);
			pAvplay->bSetEosFlag = MT_FALSE;
	        pAvplay->bVideoLastFrameDecoded = 0;

		}while(0);


    }

#if 0
    MT_ERR_AVPLAY("bVidEos %d, bAudEos %d, AdecEnd %d, AoEmpty %d, Vdec %d, VoFrmNum:%d\n",
        bVidEos, bAudEos, AdecBuf.bEndOfFrame, bEmpty, VdecStatus.bEndOfStream, WinPlayInfo.u32FrameNumInBufQn);
#endif

    return;
}

static mt_void AVPLAY_CalPreBufThreshhold(AVPLAY_S *pAvplay)
{
    mt_u32                                          VidBufPercent = 0;
    mt_u32                                          AudBufPercent = 0;
    MT_MPI_DMX_BUF_STATUS_S                         VidChnBuf = {0};
    MT_MPI_DMX_BUF_STATUS_S                         AudChnBuf = {0};
    mt_u32                                          u32SysTime = 0;
    mt_s32                                          Ret;

    if (0 == pAvplay->AudPreBufThreshhold)
    {
        sys_get_time_stamp_ms(&u32SysTime);
        if (MT_INVALID_TIME == pAvplay->AudPreSysTime)
        {
            pAvplay->AudPreSysTime = u32SysTime;
        }
        else
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudChnBuf);
            if ( MT_SUCCESS == Ret )
            {
                if ( AudChnBuf.u32BufSize == 0 )
                {
                    AudBufPercent = 0;
                    MT_ERR_AVPLAY("AudChnBuf.u32BufSize == 0\n");
                }
                else
                {
                    AudBufPercent = AudChnBuf.u32UsedSize * 100 / AudChnBuf.u32BufSize;
                }
            }
            else
            {
                AudBufPercent = 0;
                MT_ERR_AVPLAY("MT_MPI_DMX_GetPESBufferStatus failed:%x\n",Ret);
            }

            if (((u32SysTime - pAvplay->AudPreSysTime > 1000) && (AudBufPercent > 0))
                || (AudBufPercent >= 60))
            {
                pAvplay->AudPreBufThreshhold = AudBufPercent;
                MT_INFO_AVPLAY("Audio Es buffer shreshhold is :%d\n", pAvplay->AudPreBufThreshhold);
            }
        }
    }

    if (0 == pAvplay->VidPreBufThreshhold)
    {
        sys_get_time_stamp_ms(&u32SysTime);
        if (MT_INVALID_TIME == pAvplay->VidPreSysTime)
        {
            pAvplay->VidPreSysTime = u32SysTime;
        }
        else
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidChnBuf);
            if ( MT_SUCCESS == Ret )
            {
                if ( VidChnBuf.u32BufSize == 0 )
                {
                    VidBufPercent = 0;
                    MT_ERR_AVPLAY("VidChnBuf.u32BufSize == 0\n");
                }
                else
                {
                    VidBufPercent = VidChnBuf.u32UsedSize * 100 / VidChnBuf.u32BufSize;
                }
            }
            else
            {
                VidBufPercent = 0;
                MT_ERR_AVPLAY("MT_MPI_DMX_GetPESBufferStatus failed:%x\n",Ret);
            }

            if (((u32SysTime - pAvplay->VidPreSysTime > 1000) && (VidBufPercent > 0))
                || (VidBufPercent >= 60))
            {
                pAvplay->VidPreBufThreshhold = VidBufPercent;
                MT_INFO_AVPLAY("Video Es buffer shreshhold is :%d\n", pAvplay->VidPreBufThreshhold);
            }
        }
    }
}

mt_void AVPLAY_ProcDmxBuf(AVPLAY_S *pAvplay)
{
    mt_s32 Ret;
    MT_MPI_DMX_BUF_STATUS_S            VidChnBuf = {0};
    MT_MPI_DMX_BUF_STATUS_S            AudChnBuf = {0};
    mt_u32                                          VidBufPercent = 0;
    mt_u32                                          AudBufPercent = 0;
    MT_UNF_ES_BUF_S                           AudDmxEsBuf;
    MT_UNF_ES_BUF_S                           VidDmxEsBuf;

    if (MT_UNF_AVPLAY_STATUS_PREPLAY != pAvplay->CurStatus)
    {
        return;
    }

    if ( !pAvplay->AudEnable && pAvplay->bAudPreEnable )
    {
        AVPLAY_CalPreBufThreshhold(pAvplay);
        if (pAvplay->AudPreBufThreshhold != 0)
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudChnBuf);
            if ( MT_SUCCESS != Ret )
            {
                MT_ERR_AVPLAY("MT_MPI_DMX_GetPESBufferStatus failed:%x\n",Ret);
            }

            if ( AudChnBuf.u32BufSize == 0 )
            {
                MT_ERR_AVPLAY("AudChnBuf.u32BufSize == 0\n");
            }
            else
            {
                AudBufPercent = AudChnBuf.u32UsedSize * 100 / AudChnBuf.u32BufSize;

                if ( AudBufPercent > pAvplay->AudPreBufThreshhold )
                {
                    Ret = MT_MPI_DMX_AcquireEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudDmxEsBuf);
                    if ( MT_SUCCESS != Ret  )
                    {
                        MT_ERR_AVPLAY("MT_MPI_DMX_AcquireEs failed:%x\n",Ret);
                    }
                    else
                    {
                        (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudDmxEsBuf);
                        pAvplay->AvplayProcContinue = MT_TRUE;
                    }
                }
            }
        }
    }

    if ( !pAvplay->VidEnable && pAvplay->bVidPreEnable)
    {
        AVPLAY_CalPreBufThreshhold(pAvplay);
        if (pAvplay->VidPreBufThreshhold != 0)
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidChnBuf);
            if ( MT_SUCCESS != Ret )
            {
                MT_ERR_AVPLAY("MT_MPI_DMX_GetPESBufferStatus failed:%x\n",Ret);
            }

            if ( VidChnBuf.u32BufSize == 0 )
            {
                MT_ERR_AVPLAY("VidChnBuf.u32BufSize == 0\n");
            }
            else
            {
                VidBufPercent = VidChnBuf.u32UsedSize * 100 / VidChnBuf.u32BufSize;

                if ( VidBufPercent > pAvplay->VidPreBufThreshhold )
                {
                    Ret = MT_MPI_DMX_AcquireEs(pAvplay->hDmxVid, &VidDmxEsBuf);
                    if ( MT_SUCCESS != Ret  )
                    {
                        MT_ERR_AVPLAY("MT_MPI_DMX_AcquireEs failed:%x\n",Ret);
                    }
                    else
                    {
                        (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxVid, &VidDmxEsBuf);
                        pAvplay->AvplayProcContinue = MT_TRUE;
                    }
                }
            }
        }
    }

    return;
}

mt_void AVPLAY_ProcCheckBuf(AVPLAY_S *pAvplay)
{
    ADEC_BUFSTATUS_S                   AdecBuf = {0};
    VDEC_STATUSINFO_S                  VdecBuf = {0};
    MT_MPI_DMX_BUF_STATUS_S            VidChnBuf = {0};
    MT_MPI_DMX_BUF_STATUS_S            AudChnBuf = {0};
    static ADEC_BUFSTATUS_S            LastAdecBuf  = {0};
	static MT_MPI_DMX_BUF_STATUS_S 	   LastAudChnBuf = {0};
	static mt_u32 					   AudioNotMove = 0;

    MT_UNF_AVPLAY_BUF_STATE_E          CurVidBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
    MT_UNF_AVPLAY_BUF_STATE_E          CurAudBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
    mt_u32                             VidBufPercent = 0;
    mt_u32                             AudBufPercent = 0;

    MT_UNF_DMX_PORT_MODE_E             PortMode = MT_UNF_DMX_PORT_MODE_BUTT;

    MT_BOOL                            RealModeFlag = MT_FALSE;
    MT_BOOL                            ResetProc = MT_FALSE;

    SYNC_BUF_STATUS_S                  SyncBufStatus = {0};
    SYNC_BUF_STATE_E                   SyncAudBufState = SYNC_BUF_STATE_NORMAL;
    SYNC_BUF_STATE_E                   SyncVidBufState = SYNC_BUF_STATE_NORMAL;
    mt_s32                             Ret;
    if (pAvplay->AudEnable)
    {
        Ret = MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_BUFFERSTATUS, &AdecBuf);
        if (MT_SUCCESS == Ret)
        {
	            if(AdecBuf.bForceBuffReset)
				{
	                CurAudBufState = MT_UNF_AVPLAY_BUF_STATE_FULL;
				}
				else
				{
	                if(MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
	                {
	                    Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudChnBuf);
	                    if (MT_SUCCESS == Ret)
	                    {
	                        if ((AudChnBuf.u32BufSize + AdecBuf.u32BufferSize) > 0)
	                        {
	                            AudBufPercent = (AudChnBuf.u32UsedSize + AdecBuf.u32BufferUsed) * 100 / (AudChnBuf.u32BufSize + AdecBuf.u32BufferSize);
	                        }
	                        else
	                        {
	                            AudBufPercent = 0;
	                        }

    					//fix bug 124167
    					if(LastAdecBuf.s32BufReadPos == AdecBuf.s32BufReadPos)
    					{
    						if(AudChnBuf.u32BufRptr != LastAudChnBuf.u32BufRptr)
    							AudioNotMove++;
    					}
    					else
    					{
    						AudioNotMove = 0;
    					}

    					memcpy(&LastAdecBuf, &AdecBuf, sizeof(AdecBuf));
    					memcpy(&LastAudChnBuf, &AudChnBuf, sizeof(AudChnBuf));

                        CurAudBufState = AVPLAY_CaclBufState(pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, AudBufPercent);
                        SyncAudBufState = (SYNC_BUF_STATE_E)CurAudBufState;

    					if(AudioNotMove > 500)
    					{
    						if(AudBufPercent > 20)
    						{
    							MT_ERR_AVPLAY("Demux chan move but audio ES  not move need reset .\n");
    							CurAudBufState = MT_UNF_AVPLAY_BUF_STATE_FULL;
    							AudioNotMove = 0;
    						}
    					}
                    }
                }
                else
                {
                    if (AdecBuf.u32BufferSize > 0)
                    {
                        AudBufPercent = AdecBuf.u32BufferUsed * 100 / AdecBuf.u32BufferSize;
                    }
                    else
                    {
                        AudBufPercent = 0;
                    }

                    CurAudBufState = AVPLAY_CaclBufState(pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, AudBufPercent);
                    SyncAudBufState = (SYNC_BUF_STATE_E)CurAudBufState;
                }
            }

            if (CurAudBufState != pAvplay->PreAudBufState)
            {
                if (!pAvplay->VidEnable)
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_RNG_BUF_STATE, (ulong) CurAudBufState);
            }

            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_AUD_BUF_STATE, (ulong)CurAudBufState);
            pAvplay->PreAudBufState = CurAudBufState;
            }
        }
    }

    if (pAvplay->VidEnable)
    {
    // for bug 24413, when freeze don't check buffer
		MT_BOOL bEnable = MT_FALSE;
		MT_DRV_WIN_SWITCH_E enWinFreezeMode = MT_DRV_WIN_SWITCH_LAST;
		MT_MPI_WIN_GetFreezeStat(pAvplay->MasterFrmChn.hWindow, &bEnable, &enWinFreezeMode);
		if(bEnable)
			return ;
	// for bug 24413 end.

        Ret = MT_MPI_VDEC_GetChanStatusInfo(pAvplay->hVdec, &VdecBuf);
        if(MT_SUCCESS == Ret)
        {
            if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
            {
                Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidChnBuf);
                if (MT_SUCCESS == Ret)
                {
					MT_INFO_AVPLAY("Video Ts Buffer[DMX]: Used %u, Size %u\n",VidChnBuf.u32UsedSize,VidChnBuf.u32BufSize);
                    if (VidChnBuf.u32BufSize > 0)
                    {
						MT_INFO_AVPLAY("Video Ts Buffer[DMX]: RD %u, WR %u\n",VidChnBuf.u32BufRptr,VidChnBuf.u32BufWptr);
                        VidBufPercent = VidChnBuf.u32UsedSize * 100 / VidChnBuf.u32BufSize;
                    }
                    else
                    {
                        VidBufPercent = 0;
                    }

                    CurVidBufState = AVPLAY_CaclBufState(pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, VidBufPercent);
                    SyncVidBufState = (SYNC_BUF_STATE_E)CurVidBufState;


					if (MT_UNF_AVPLAY_BUF_STATE_FULL == CurVidBufState)
					{
						if(VidChnBuf.u32BufSize < VidChnBuf.u32UsedSize)
						{
							CurVidBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
						}
					}
                }
            }
            else
            {
                if (VdecBuf.u32BufferSize > 0)
                {
                    VidBufPercent = VdecBuf.u32BufferUsed * 100 / VdecBuf.u32BufferSize;
                }
                else
                {
                    VidBufPercent = 0;
                }

                CurVidBufState = AVPLAY_CaclBufState(pAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, VidBufPercent);
                SyncVidBufState = (SYNC_BUF_STATE_E)CurVidBufState;
            }
        }

        if (CurVidBufState != pAvplay->PreVidBufState)
        {
            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_RNG_BUF_STATE, (ulong)CurVidBufState);

            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_VID_BUF_STATE, (ulong)CurVidBufState);
            pAvplay->PreVidBufState = CurVidBufState;
        }
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        (mt_void)MT_MPI_DMX_GetPortMode(pAvplay->AvplayAttr.u32DemuxId, &PortMode);
		MT_INFO_AVPLAY("DMX Port Mode: %d\n",PortMode);

        if (MT_UNF_DMX_PORT_MODE_RAM == PortMode)
        {
            RealModeFlag = MT_FALSE;
        }
        else
        {
            RealModeFlag = MT_TRUE;
        }
    }

    /*  real mode */
    if (RealModeFlag)
    {
        if (MT_UNF_AVPLAY_BUF_STATE_FULL == CurAudBufState)
        {
            ResetProc = MT_TRUE;
            pAvplay->DebugInfo.AudOverflowNum++;
            if(pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PAUSE)
            {
               MT_ERR_AVPLAY("Aud Dmx Buf overflow, reset.\n");
            }
        }
        else if (MT_UNF_AVPLAY_BUF_STATE_EMPTY == CurAudBufState)
        {
            if(pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_PLAY)
            {
                pAvplay->DebugInfo.AudUnderflowNum++;
            }
            MT_DBG_AVPLAY("Aud Dmx Buf underflow, status %d.\n", pAvplay->CurStatus);
        }

        if (pAvplay->VidDiscard)
        {
            if (VidBufPercent <= 60)
            {
                pAvplay->VidDiscard = MT_FALSE;
            }
        }
        else
        {
            if (MT_UNF_AVPLAY_BUF_STATE_FULL == CurVidBufState)
            {
                if (MT_UNF_AVPLAY_OVERFLOW_RESET == pAvplay->OverflowProc)
                {
                    ResetProc = MT_TRUE;
                    pAvplay->DebugInfo.VidOverflowNum++;
                    if(pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PAUSE)
                    {
					   MT_ERR_AVPLAY("Vid Dmx Buf overflow, reset. u32BufferSize %d \n", VidChnBuf.u32BufSize);

                    }
                }
                else
                {
                    pAvplay->VidDiscard = MT_TRUE;
                    pAvplay->DebugInfo.VidOverflowNum++;

                    //(mt_void)MT_MPI_SYNC_Aud_Init(pAvplay->hSync, 3);
                    (mt_void)AVPLAY_ResetAudChn(pAvplay);
                    if(pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PAUSE)
                    {
                        MT_ERR_AVPLAY("Vid Dmx Buf overflow, discard.\n");
                    }
                }
            }
            else if (MT_UNF_AVPLAY_BUF_STATE_EMPTY == CurVidBufState)
            {
                if(pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_PLAY)
                {
                    pAvplay->DebugInfo.VidUnderflowNum++;
                }
                MT_DBG_AVPLAY("Vid Dmx Buf underflow, status %d.\n", pAvplay->CurStatus);
            }
        }

        if (ResetProc
        	&& pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PAUSE
            && pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_FREEZE
        	&& !pAvplay->bStepMode)
        {
		#ifndef CONFIG_MT_FPGA
            (mt_void)AVPLAY_Reset(pAvplay);
		#endif
            pAvplay->VidDiscard = MT_FALSE;
        }
        else
        {
            SyncBufStatus.AudBufPercent = AudBufPercent;
            SyncBufStatus.AudBufState = SyncAudBufState;
            SyncBufStatus.VidBufPercent = VidBufPercent;
            SyncBufStatus.VidBufState = SyncVidBufState;
            SyncBufStatus.bOverflowDiscFrm = pAvplay->VidDiscard;
            (mt_void)MT_MPI_SYNC_SetBufState(pAvplay->hSync,SyncBufStatus);
        }
    }
    else
    {
        if (SyncAudBufState == SYNC_BUF_STATE_LOW || SyncAudBufState == SYNC_BUF_STATE_EMPTY)
        {
            SyncBufStatus.AudBufState = SyncAudBufState;
        }
        else
        {
            SyncBufStatus.AudBufState = SYNC_BUF_STATE_NORMAL;
        }

        SyncBufStatus.AudBufPercent = AudBufPercent;

        if (SyncVidBufState == SYNC_BUF_STATE_LOW || SyncVidBufState == SYNC_BUF_STATE_EMPTY)
        {
            SyncBufStatus.VidBufState = SyncVidBufState;
        }
        else
        {
            SyncBufStatus.VidBufState = SYNC_BUF_STATE_NORMAL;
        }

        SyncBufStatus.VidBufPercent = VidBufPercent;

        SyncBufStatus.bOverflowDiscFrm = pAvplay->VidDiscard;

        (mt_void)MT_MPI_SYNC_SetBufState(pAvplay->hSync,SyncBufStatus);
    }

    return;
}

static mt_void AVPLAY_DRV2UNF_VidFrm(MT_DRV_VIDEO_FRAME_S *pstDRVFrm, MT_UNF_VIDEO_FRAME_INFO_S *pstUNFFrm)
{
    pstUNFFrm->u32FrameIndex = pstDRVFrm->u32FrameIndex;
    pstUNFFrm->stVideoFrameAddr[0].u32YAddr = pstDRVFrm->stBufAddr[0].u32PhyAddr_Y;
    pstUNFFrm->stVideoFrameAddr[0].u32CAddr = pstDRVFrm->stBufAddr[0].u32PhyAddr_C;
    pstUNFFrm->stVideoFrameAddr[0].u32CrAddr = pstDRVFrm->stBufAddr[0].u32PhyAddr_Cr;
    pstUNFFrm->stVideoFrameAddr[0].u32YStride = pstDRVFrm->stBufAddr[0].u32Stride_Y;
    pstUNFFrm->stVideoFrameAddr[0].u32CStride = pstDRVFrm->stBufAddr[0].u32Stride_C;
    pstUNFFrm->stVideoFrameAddr[0].u32CrStride = pstDRVFrm->stBufAddr[0].u32Stride_Cr;
    pstUNFFrm->stVideoFrameAddr[1].u32YAddr = pstDRVFrm->stBufAddr[1].u32PhyAddr_Y;
    pstUNFFrm->stVideoFrameAddr[1].u32CAddr = pstDRVFrm->stBufAddr[1].u32PhyAddr_C;
    pstUNFFrm->stVideoFrameAddr[1].u32CrAddr = pstDRVFrm->stBufAddr[1].u32PhyAddr_Cr;
    pstUNFFrm->stVideoFrameAddr[1].u32YStride = pstDRVFrm->stBufAddr[1].u32Stride_Y;
    pstUNFFrm->stVideoFrameAddr[1].u32CStride = pstDRVFrm->stBufAddr[1].u32Stride_C;
    pstUNFFrm->stVideoFrameAddr[1].u32CrStride = pstDRVFrm->stBufAddr[1].u32Stride_Cr;
    pstUNFFrm->u32Width = pstDRVFrm->u32Width;
    pstUNFFrm->u32Height = pstDRVFrm->u32Height;
    pstUNFFrm->u32SrcPts = pstDRVFrm->u32SrcPts;
    pstUNFFrm->u32Pts = pstDRVFrm->u32Pts;
    pstUNFFrm->u64Pts = pstDRVFrm->u64Pts;
    pstUNFFrm->u32AspectWidth = pstDRVFrm->u32AspectWidth;
    pstUNFFrm->u32AspectHeight = pstDRVFrm->u32AspectHeight;
    pstUNFFrm->stFrameRate.u32fpsInteger = pstDRVFrm->u32FrameRate/1000;
    pstUNFFrm->stFrameRate.u32fpsDecimal = pstDRVFrm->u32FrameRate % 1000;

	if(pstUNFFrm->stFrameRate.u32fpsInteger > 60)
	{
		pstUNFFrm->stFrameRate.u32fpsInteger = pstUNFFrm->stFrameRate.u32fpsInteger/100;
	}
	
    pstUNFFrm->bProgressive = pstDRVFrm->bProgressive;

    switch (pstDRVFrm->ePixFormat)
    {
        case MT_DRV_PIX_FMT_NV61_2X1:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_422;
            break;
        case MT_DRV_PIX_FMT_NV21:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_420;
            break;
        case MT_DRV_PIX_FMT_NV80:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_400;
            break;
        case MT_DRV_PIX_FMT_NV12_411:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_411;
            break;
        case MT_DRV_PIX_FMT_NV61:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2;
            break;
        case MT_DRV_PIX_FMT_NV42:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_SEMIPLANAR_444;
            break;
        case MT_DRV_PIX_FMT_UYVY:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PACKAGE_UYVY;
            break;
        case MT_DRV_PIX_FMT_YUYV:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PACKAGE_YUYV;
            break;
        case MT_DRV_PIX_FMT_YVYU:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PACKAGE_YVYU;
            break;

        case MT_DRV_PIX_FMT_YUV400:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_400;
            break;
        case MT_DRV_PIX_FMT_YUV411:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_411;
            break;
        case MT_DRV_PIX_FMT_YUV420p:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_420;
            break;
        case MT_DRV_PIX_FMT_YUV422_1X2:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_422_1X2;
            break;
        case MT_DRV_PIX_FMT_YUV422_2X1:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_422_2X1;
            break;
        case MT_DRV_PIX_FMT_YUV_444:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_444;
            break;
        case MT_DRV_PIX_FMT_YUV410p:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_PLANAR_410;
            break;
        default:
            pstUNFFrm->enVideoFormat = MT_UNF_FORMAT_YUV_BUTT;
            break;
    }

    switch (pstDRVFrm->enFieldMode)
    {
        case MT_DRV_FIELD_TOP:
        {
            pstUNFFrm->enFieldMode = MT_UNF_VIDEO_FIELD_TOP;
            break;
        }
        case MT_DRV_FIELD_BOTTOM:
        {
            pstUNFFrm->enFieldMode = MT_UNF_VIDEO_FIELD_BOTTOM;
            break;
        }
        case MT_DRV_FIELD_ALL:
        {
            pstUNFFrm->enFieldMode = MT_UNF_VIDEO_FIELD_ALL;
            break;
        }
        default:
        {
            pstUNFFrm->enFieldMode = MT_UNF_VIDEO_FIELD_BUTT;
            break;
        }
    }

    pstUNFFrm->bTopFieldFirst = pstDRVFrm->bTopFieldFirst;

    switch (pstDRVFrm->eFrmType)
    {
        case MT_DRV_FT_NOT_STEREO:
        {
            pstUNFFrm->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_NONE;
            break;
        }
        case MT_DRV_FT_SBS:
        {
            pstUNFFrm->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE;
            break;
        }
        case MT_DRV_FT_TAB:
        {
            pstUNFFrm->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM;
            break;
        }
        case MT_DRV_FT_FPK:
        {
            pstUNFFrm->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED;
            break;
        }
        default:
        {
            pstUNFFrm->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_BUTT;
            break;
        }
    }

    pstUNFFrm->u32Circumrotate = pstDRVFrm->u32Circumrotate;
    pstUNFFrm->bVerticalMirror = pstDRVFrm->bToFlip_V;
    pstUNFFrm->bHorizontalMirror = pstDRVFrm->bToFlip_H;
    pstUNFFrm->u32DisplayWidth = (mt_u32)pstDRVFrm->stDispRect.s32Width;
    pstUNFFrm->u32DisplayHeight = (mt_u32)pstDRVFrm->stDispRect.s32Height;
    pstUNFFrm->u32DisplayCenterX = (mt_u32)pstDRVFrm->stDispRect.s32X;
    pstUNFFrm->u32DisplayCenterY = (mt_u32)pstDRVFrm->stDispRect.s32Y;
    pstUNFFrm->u32ErrorLevel = pstDRVFrm->u32ErrorLevel;

	//extended fields
    pstUNFFrm->picture_coding_type = pstDRVFrm->slotInfo.picture_coding_type;

	//active_format description
	if (pstDRVFrm->slotInfo.active_format_flag)
	{
		pstUNFFrm->active_format = pstDRVFrm->slotInfo.active_format;
	}
	else
	{
		pstUNFFrm->active_format = 0;	//undefined
	}

    if(720 >= pstUNFFrm->u32Width)
    {
        switch(pstUNFFrm->stFrameRate.u32fpsInteger)
        {
            case 25:
                pstUNFFrm->vid_format = VID_SYS_PAL;
                break;

            case 24:
            case 15:
            case 29:
            case 30:
                pstUNFFrm->vid_format = VID_SYS_NTSC_M;
                break;

            case 50:
                pstUNFFrm->vid_format = VID_SYS_576P_50HZ;
                break;

            case 59:
            case 60:
                pstUNFFrm->vid_format = VID_SYS_480P;
                break;

            default:
                pstUNFFrm->vid_format = VID_SYS_PAL;
                break;
        }
    }
    else if(30 >= pstUNFFrm->stFrameRate.u32fpsInteger)  // fxied bug 102846 , frame rate  30  should be 1080 i
    {
        switch(pstUNFFrm->u32Width)
        {
            case 1280:
                if(25 == pstUNFFrm->stFrameRate.u32fpsInteger)
                {
                  pstUNFFrm->vid_format = VID_SYS_720P_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_720P;
                }
                break;

            case 1440:
            case 1920:
                if(25 == pstUNFFrm->stFrameRate.u32fpsInteger)
                {
                  pstUNFFrm->vid_format = VID_SYS_1080I_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_1080I;
                }
                break;

            default:
                if(25 == pstUNFFrm->stFrameRate.u32fpsInteger)
                {
                  pstUNFFrm->vid_format = VID_SYS_720P_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_720P;
                }
                break;
        }
    }
    else
    {
        switch(pstUNFFrm->u32Width)
        {
            case 1280:
                if(50 == pstUNFFrm->stFrameRate.u32fpsInteger)
                {
                  pstUNFFrm->vid_format = VID_SYS_720P_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_720P;
                }
                break;

            case 1440:
            case 1920:
                if(50 == pstUNFFrm->stFrameRate.u32fpsInteger)
                {
                  pstUNFFrm->vid_format = VID_SYS_1080P_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_1080P;
                }
                break;

            default:
                //fix bug 103642
                if((25 == pstUNFFrm->stFrameRate.u32fpsInteger) || (50 == pstUNFFrm->stFrameRate.u32fpsInteger))
                {
                  pstUNFFrm->vid_format = VID_SYS_720P_50HZ;
                }
                else
                {
                  pstUNFFrm->vid_format = VID_SYS_720P;
                }
                break;
        }
    }

    return;
}

/* clean all VPSS buffer frames */
#ifdef CONFIG_MT_HAS_HW_VPASS
static mt_s32 AVPLAY_CleanAllVpssFrame(AVPLAY_S *pAvplay, mt_s32 retryTimes)
{
	mt_s32 Ret = MT_SUCCESS;

	while(1)
	{
		//MT_USLEEP(1000);
		Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);
		//printf("XXXX 8888 Ret = %x \n", Ret);
		if (MT_SUCCESS != Ret)
		{
			if (retryTimes-- > 0)
			{
				//MT_USLEEP(1000);
				continue;
			}
			else
			{
				break;
			}
		}
		MT_INFO_AVPLAY("rls slot_idx = %d \n",pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
		Ret |= MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[0].hport, &pAvplay->CurFrmPack.stFrame[0].stFrameVideo);
	}

	return Ret;
}
#endif

mt_void AVPLAY_ProcVidEvent(AVPLAY_S *pAvplay)
{
    VDEC_EVENT_S                        VdecEvent;
    MT_UNF_VIDEO_USERDATA_S             VdecUsrData;
    mt_s32                              Ret;

	//MT_INFO_AVPLAY("%s: %d, VidEnable=%d\n",__FUNCTION__,__LINE__,pAvplay->VidEnable);
    if (pAvplay->VidEnable)
    {
        Ret = MT_MPI_VDEC_CheckNewEvent(pAvplay->hVdec, &VdecEvent);
        if (MT_SUCCESS == Ret)
        {
            if (VdecEvent.bNewUserData && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_NEW_USER_DATA])
            {
                Ret = MT_MPI_VDEC_ChanRecvUsrData(pAvplay->hVdec, &VdecUsrData);
                if (MT_SUCCESS == Ret)
                {
                    AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NEW_USER_DATA, (ulong)(&VdecUsrData));
                }
                else
                {
                    MT_ERR_AVPLAY("call MT_MPI_VDEC_ReadNewFrame failed.\n");
                }
            }

            if (VdecEvent.bNormChange && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_NORM_SWITCH])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NORM_SWITCH, (ulong)(&(VdecEvent.stNormChangeParam)));
            }

            if (VdecEvent.bFramePackingChange && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_FRAMEPACKING_CHANGE])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_FRAMEPACKING_CHANGE, (ulong)(VdecEvent.enFramePackingType));
            }

            if (VdecEvent.bIFrameErr && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_IFRAME_ERR])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_IFRAME_ERR, MT_NULL);
            }

            if (VdecEvent.bUnSupportStream && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_VID_UNSUPPORT, MT_NULL);
            }

            if (0 != VdecEvent.u32ErrRatio && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_VID_ERR_RATIO, (ulong)(VdecEvent.u32ErrRatio));
            }
            if (0 != VdecEvent.bLastFrameShowed && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED, MT_NULL);
            }

            if (pAvplay->DebugInfo.AcquiredVidFrameNum > 0 && 0 != VdecEvent.bSlHdrEnableChange && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SL_HDR_EN_CHANGE])
            {
                printf("Report HDR Enabled State %d\n", VdecEvent.bSlHdrEnable);
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SL_HDR_EN_CHANGE, (ulong)(VdecEvent.bSlHdrEnable));
            }

            if (0 != VdecEvent.bLastFrameDecoded)
            {
                pAvplay->bVideoLastFrameDecoded = VdecEvent.bLastFrameDecoded;
				     if(pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED])
               			AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED, MT_NULL);
				MT_ALWAYS_PRINT("call MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED\n");
            }
            /*if (VdecEvent.stProbeStreamInfo.bProbeCodecTypeChangeFlag)
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_VID_ERR_TYPE, (ulong)(VdecEvent.stProbeStreamInfo.enCodecType));
            }*/

            if(VdecEvent.bImageSizeChange)
            {
              //Yihua and binxuan
                //mt_handle                           hWindow = MT_INVALID_HANDLE;
                //int i = 0;
                /*for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
                {
                    (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

                    if (hWindow == pAvplay->MasterFrmChn.hWindow)
                    {
                        break;
                    }
                }*/

                //printf("XXXX 7777 VdecEvent.bImageSizeChange = %x hWindow = %x,  hVdec = %x\n", VdecEvent.bImageSizeChange, hWindow, pAvplay->hVdec);
                //printf("%s: VdecEvent - bImageSizeChange = %x \n", __FUNCTION__,VdecEvent.bImageSizeChange);
			    MT_INFO_AVPLAY("EVNT_IMG_SIZE_CHANGE, flush window.\n");
                //pAvplay->u32imgChangeFlag = 1;

				//20181203
				//Bug: WARNING! Video Resolution Change IPC Event maybe dead lock with Audio IPC!
				if (pAvplay->DebugInfo.AcquiredVidFrameNum > 0)
                {
#if 0
                  mt_handle    hWindow = MT_INVALID_HANDLE;
                  mt_u32 Ret;
                  int i = 0;
                  int found_flag = 0;

                  //pAvplay->u32imgChangeFlag = 0;
                  Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);

                  if(pAvplay->CurFrmPack.u32FrmNum < 0)
                  {
                    pAvplay->CurFrmPack.u32FrmNum = 0;
                  }

                  if(pAvplay->CurFrmPack.u32FrmNum > 3)
                  {
                    pAvplay->CurFrmPack.u32FrmNum = 3;
                  }

                  for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
                  {
                    (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

                    if (hWindow == pAvplay->MasterFrmChn.hWindow)
                    {
                        found_flag = 1;
                        break;
                    }
                  }

				  //FIXME: how about pAvplay->CurFrmPack.u32FrmNum=0
				  //       but window has frames?
				  //???????????????
                  if(found_flag)
                  {
                    MT_MPI_WIN_CleanAllFrm(hWindow);
                  }
#else
				  //20180322: clean all frame????MT_MPI_WIN_QueueFrame ???,???????Mutex????
				  AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);

				  //20180208
				  //FIX: Resolution Change Event, Display must release all
				  //frames synchronously, includes the displaying frames!
				  //This may cause screen mess up issue!
				  //[CN Comment]Resolution Change Event: Display?????????????????,
				  //??????????????,???????????????????Bug.
				  //Reason: Resolution Change???????, VFMW??????????Buffer??????,
				  //VFMW????????ЩBuffer????.
				  MT_INFO_AVPLAY("EVNT_IMG_SIZE_CHANGE: v frame %u\n",pAvplay->DebugInfo.AcquiredVidFrameNum);
				  //for 1st frame, no need flush window(flush window use too much time!)
				  //if (pAvplay->DebugInfo.AcquiredVidFrameNum > 0)
				  {
                  	AVPLAY_FlushWindow(pAvplay, MT_DRV_WIN_FLUSH_BOTH);
                  }
#endif

				  //??????н????????
#if 0
                  i = 0;
                  while(1)
                  {
                    //MT_USLEEP(1000);
                    Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);
                    //printf("XXXX 8888 Ret = %x \n", Ret);
                    if (MT_SUCCESS != Ret)
                    {
                      if(i < 5)
                      {
                        i++;
                        continue;
                      }
                      else
                      {
                        break;
                      }
                    }
                    printf("XXXX 9999 slot_idx = %d \n", pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
                    MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[0].hport, &pAvplay->CurFrmPack.stFrame[0].stFrameVideo);
                  }
#else
					//H264 need retry more times!
					//20180324, do clean all Vpss frames in drv.
					//printf("%s: EVNT_IMG_SIZE_CHANGE, clean vpss.\n",__FUNCTION__);
					//AVPLAY_CleanAllVpssFrame(pAvplay, 1000);
#endif

                    //MT_MPI_AVPLAY_SetVOBufferClearnComplete(pAvplay, MT_TRUE);

					//20180322: clean all frame????MT_MPI_WIN_QueueFrame ???,???????Mutex????
					AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                }
                else
                {
                	//do nothing
                }

				MT_MPI_AVPLAY_SetVOBufferClearnComplete(pAvplay, MT_TRUE);
#if 0
                MT_MPI_WIN_CleanAllFrm(hWindow);

                /*{
                  mt_u32 Ret;
                  Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &pAvplay->CurFrmPack);
                  printf("XXXX 1010 slot_idx = %d, Ret = %x \n", pAvplay->CurFrmPack.stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx, Ret);
                  if (MT_SUCCESS != Ret)
                  {
                    //printf("\n @@@@ 0000 AVPLAY_ProcVdecToVo Ret = %x \n", Ret);
                    //return;
                  }
                }*/
//#else
                VPSS_Control(pAvplay->hVdec, VPSS_CMD_RESETVPSS, NULL);
                MT_MPI_AVPLAY_SetVOBufferClearnComplete(pAvplay, MT_TRUE);
#endif
            }

            if (VdecEvent.bFirstValidPts)
            {
                (mt_void)MT_MPI_SYNC_SetExtInfo(pAvplay->hSync, SYNC_EXT_INFO_FIRST_PTS, (mt_void *)VdecEvent.u32FirstValidPts);
            }

            if (VdecEvent.bSecondValidPts)
            {
                (mt_void)MT_MPI_SYNC_SetExtInfo(pAvplay->hSync, SYNC_EXT_INFO_SECOND_PTS, (mt_void *)VdecEvent.u32SecondValidPts);
            }
        }
        else
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_CheckNewEvent failed.\n");
        }
    }

	if(pAvplay->TargetTime != MT_INVALID_TIME64)
	{
		if(pAvplay->VidEnable && pAvplay->AudEnable)
		{
			if(pAvplay->bTargetVideoRendered && pAvplay->bTargetAudioRendered)
			{
				pAvplay->TargetTime = MT_INVALID_TIME64;
				if(pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED])
				{
					AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED, MT_NULL);
				}
				MT_ALWAYS_PRINT("reset av target time\n");
			}
		}
		else if(pAvplay->VidEnable && pAvplay->bTargetVideoRendered)
		{
			pAvplay->TargetTime = MT_INVALID_TIME64;
			if(pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED])
			{
				AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED, MT_NULL);
			}
			MT_ALWAYS_PRINT("reset video target time\n");
		}
		else if(pAvplay->AudEnable && pAvplay->bTargetAudioRendered)
		{
			pAvplay->TargetTime = MT_INVALID_TIME64;
			if(pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED])
			{
				AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SEEK_COMPLETED, MT_NULL);
			}
			MT_ALWAYS_PRINT("reset audio target time\n");
		}
	}

	if( pAvplay->DebugInfo.AcquiredVidFrameNum > pAvplay->DebugInfo.LastAcquiredVidFrameNum)
	{
		pAvplay->DebugInfo.LastAcquiredVidFrameNum += 1;
		pAvplay->DebugInfo.NoNewFrameCnt = 0;

#if 0
		if (pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME])
		{
			MT_UNF_VIDEO_FRAME_INFO_S			VdecUnfFrm;

			AVPLAY_DRV2UNF_VidFrm(&(pAvplay->LstFrmPack.stFrame[0].stFrameVideo), &VdecUnfFrm);
			if ((pAvplay->DebugInfo.AcquiredVidFrameNum % 250) == 1)
			{
				MT_INFO_AVPLAY("AVPLAY_Notify: Acquired %u Video Frames.\n",pAvplay->DebugInfo.AcquiredVidFrameNum);
			}
			if(pAvplay->DebugInfo.AcquiredVidFrameNum > 0)
			{
				AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NEW_VID_FRAME, (ulong)(&VdecUnfFrm));
			}
		}
#endif
		if (pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME])
		{
			MT_UNF_VIDEO_FRAME_INFO_S			VdecUnfFrm;
			AVPLAY_DRV2UNF_VidFrm(&(pAvplay->LstFrmPack.stFrame[0].stFrameVideo), &VdecUnfFrm);

			mt_s32 cnt	= 0;
			Ret = MT_MPI_SYNC_GetVFrm_count(pAvplay->hSync, &cnt);
			if((cnt >= 1) && (0 == pAvplay->DebugInfo.FirstVidShowd))
			{

				MT_ALWAYS_PRINT("AVPLAY_Notify MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME \n");
				pAvplay->DebugInfo.FirstVidShowd = 1;
				AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_FIRST_VID_FRAME, (ulong)(&VdecUnfFrm));
			}
		}
	}
	else
	{
		pAvplay->DebugInfo.NoNewFrameCnt++;
	}
    return;
}

mt_void AVPLAY_ProcAudEvent(AVPLAY_S *pAvplay)
{
    ADEC_EVENT_S AdecEvent;
    mt_s32 Ret;
	memset(&AdecEvent, 0, sizeof(AdecEvent));
    if (pAvplay->AudEnable)
    {
        Ret = MT_MPI_ADEC_CheckNewEvent(pAvplay->hAdec, &AdecEvent);
        if (MT_SUCCESS == Ret)
        {
            if (AdecEvent.bFrameInfoChange && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_AUD_INFO_CHANGE, (ulong)(&(AdecEvent.stStreamInfo)));
            }

            if (AdecEvent.bUnSupportFormat && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_AUD_UNSUPPORT, 0);
            }

            if (AdecEvent.bStreamCorrupt && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR])
            {
                AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_AUD_FRAME_ERR, 0);
            }

#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
			if(AdecEvent.bnewAudioFrame)
			{
				if(pAvplay->TargetTime != MT_INVALID_TIME64)
				{
                    if(AdecEvent.AvplayAudFrm.u32FrameIndex >= 1)
					{
                        /* task maybe switched, here we need to check the target time again */
                        if(!pAvplay->bTargetAudioRendered && pAvplay->TargetTime != MT_INVALID_TIME64)
                        {
                            if(AdecEvent.AvplayAudFrm.u64PtsMs > pAvplay->TargetTime)
                            {
                                pAvplay->bTargetAudioRendered = MT_TRUE;
                                MT_ALWAYS_PRINT("current audio frame pts %llu, target pts %llu, rendered %u\n", AdecEvent.AvplayAudFrm.u64PtsMs, pAvplay->TargetTime, AdecEvent.AvplayAudFrm.u32FrameIndex);
                            }
                        }
					}
				}
				else
				{
					pAvplay->DebugInfo.AcquiredAudFrameNum= AdecEvent.AvplayAudFrm.u32FrameIndex;
					if(AdecEvent.AvplayAudFrm.u32FrameIndex > 1)
					{
                        MT_INFO_AVPLAY("new audio frame %llu rendered, index %u\n", AdecEvent.AvplayAudFrm.u64PtsMs, AdecEvent.AvplayAudFrm.u32FrameIndex);
                        AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_NEW_AUD_FRAME, (ulong)(&AdecEvent.AvplayAudFrm));
					}
				}
			}
#endif
        }
        else
        {
            MT_INFO_AVPLAY("call MT_MPI_ADEC_CheckNewEvent failed.\n");
        }
    }

    return;
}

mt_void AVPLAY_ProcSyncEvent(AVPLAY_S *pAvplay)
{
    mt_s32              Ret;
    SYNC_EVENT_S        SyncEvent;

    Ret = MT_MPI_SYNC_CheckNewEvent(pAvplay->hSync, &SyncEvent);
    if (MT_SUCCESS == Ret)
    {
        if (SyncEvent.bVidPtsJump &&  pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP])
        {
            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP, (ulong)(&(SyncEvent.VidPtsJumpParam)));
        }

        if (SyncEvent.bAudPtsJump &&  pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP])
        {
            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SYNC_PTS_JUMP, (ulong)(&(SyncEvent.AudPtsJumpParam)));
        }

        if (SyncEvent.bStatChange && pAvplay->EvtCbFunc[MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE])
        {
            AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_SYNC_STAT_CHANGE, (ulong)(SyncEvent.bStatChange));
        }
		//for bug 129238 back play issue
        if (SyncEvent.bEos_back)
        {
			if(pAvplay->trickmode == TM_FREV)
			{
				Ret = AVPLAY_SetEosFlag(pAvplay);
				if (MT_SUCCESS != Ret)
				{
					MT_ERR_AVPLAY("ERR: AVPLAY_SetEosFlag, Ret = %x\n", Ret);
				}
			}
        }

    }
    else
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_CheckNewEvent failed.\n");
    }

    return;
}

mt_void AVPLAY_ProcCheckStandBy(AVPLAY_S *pAvplay)
{
    /*ts mode, we need reset avplay when system standby*/
    if (pAvplay->bStandBy && MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        pAvplay->bStandBy = MT_FALSE;
        (mt_void)AVPLAY_Reset(pAvplay);
        MT_WARN_AVPLAY("System standby, now reset the AVPLAY!\n");
    }

    return;
}

mt_void AVPLAY_ProcUnloadTime(AVPLAY_S *pAvplay)
{
#if 0	// ahren comment
	mt_u32              u32AoUnloadTime = 0;
    mt_u32              u32WinUnloadTime = 0;
    mt_u32              u32ThreadScheTimeOutCnt = 0;
    mt_s32              Ret = MT_SUCCESS;

    u32ThreadScheTimeOutCnt = pAvplay->DebugInfo.ThreadScheTimeOutCnt;

    if (pAvplay->MasterFrmChn.hWindow != MT_INVALID_HANDLE)
    {
        Ret = MT_MPI_WIN_GetUnloadTimes(pAvplay->MasterFrmChn.hWindow, &u32WinUnloadTime);
    }

    Ret |= MT_MPI_AO_SND_GetXrunCount(MT_UNF_SND_0, &u32AoUnloadTime);
    if (Ret == MT_SUCCESS)
    {
        if (((pAvplay->u32AoUnloadTime != u32AoUnloadTime) || (pAvplay->u32WinUnloadTime != u32WinUnloadTime)
            || (pAvplay->u32ThreadScheTimeOutCnt != u32ThreadScheTimeOutCnt)))
        {

            if ((pAvplay->PreVidBufState == MT_UNF_AVPLAY_BUF_STATE_EMPTY) && (pAvplay->PreAudBufState == MT_UNF_AVPLAY_BUF_STATE_EMPTY))
            {
                return;
            }

#ifdef MT_MSP_BUILDIN
            Ret = ioctl(g_AvplayDevFd, CMD_AVPLAY_SET_CPUFREQ, MT_NULL);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("AVPLAY CMD_AVPLAY_SET_CPUFREQ failed.\n");
                return;
            }
#endif
            pAvplay->DebugInfo.CpuFreqScheTimeCnt++;
            pAvplay->u32AoUnloadTime = u32AoUnloadTime;
            pAvplay->u32WinUnloadTime = u32WinUnloadTime;
            pAvplay->u32ThreadScheTimeOutCnt = u32ThreadScheTimeOutCnt;
        }
     }
#endif
     return;
}

/*
 for some unknown reason(stack overflow?), after pthread_create,
 the thread might not run, it's better to wait the thread run after pthread_create.
 */
static volatile mt_u32 avplayer_pthread_flag = 0;
/* signal thread run */
static void thread_signal(void)
{
    avplayer_pthread_flag = 1;
}

/* wait thread run */
static void thread_wait(void)
{
    mt_u32 i = 0;
    for(i = 0; i < 100; i++)
    {
        if(1 == avplayer_pthread_flag)
        {
            break;
        }
        MT_USLEEP(10 * 1000);
    }
    avplayer_pthread_flag = 0;
}

/* set thread sched nice(SCHED_OTHER) */
static int set_sched_nice(pid_t tid, int sched_nice)
{
	struct sched_attr 				attr;
	int 							ret = 0;

	if (sched_nice < -20 || sched_nice > 19)
	{
		MT_ERR_AVPLAY("%s: invalid nice(%d)!\n",__FUNCTION__,sched_nice);
		return (-1);
	}

	if (tid == 0)
		tid = gettid();

	ret = sched_getattr(tid, &attr, sizeof(attr), 0);
	if (ret == 0)
	{
		MT_INFO_AVPLAY("\n%s: tid=%u, sched_policy=%u, SCHED_OTHER(%u)\n",__FUNCTION__,
						tid,attr.sched_policy,SCHED_OTHER);

		if (attr.sched_policy == SCHED_OTHER)
		{
			attr.sched_nice = sched_nice;

			ret = sched_setattr(tid, &attr, 0);
			if (ret != 0)
			{
				MT_ERR_AVPLAY("%s: sched_setattr failed!\n",__FUNCTION__);
			}
		}
	}
	else
	{
		MT_ERR_AVPLAY("%s: sched_getattr failed!\n",__FUNCTION__);
	}

	return ret;
}

static void avplay_delay_ms(U32 DelayTime/*ms*/)
{
#ifdef CONFIG_MT_USE_MSLEEP
   (void)MT_USLEEP(1000 * DelayTime);
#else
   struct timespec req = {0};

    req.tv_sec  =  DelayTime / 1000;
    req.tv_nsec = (DelayTime % 1000) * 1000000;

    while (nanosleep(&req, &req) < 0 && errno == EINTR){
	  ;//printf("SYS_TaskDelay --EINTR----------------------\n");
    }
#endif
}

/* tp2 - tp1 */
static long avplay_diff_time_ms(struct timespec tp1, struct timespec tp2)
{
	if ((tp2.tv_sec > tp1.tv_sec)
		|| ((tp2.tv_sec == tp1.tv_sec) && (tp2.tv_nsec >= tp1.tv_nsec)))
	{
		return (long)(((tp2.tv_sec - tp1.tv_sec) * 1000)
					  + (tp2.tv_nsec/1000000)
					  - (tp1.tv_nsec/1000000));
	}
	else
	{
		//FIXME: overflow! treat tp1 as zero!
		MT_INFO_AVPLAY("[W]%s: t1(%lu, %ld), t2(%lu, %ld) overflow!\n",__FUNCTION__,
			tp1.tv_sec, tp1.tv_nsec,
			tp2.tv_sec, tp2.tv_nsec);
		return (long)((tp2.tv_sec * 1000)
					  + (tp2.tv_nsec/1000000));
	}
}


static int avplay_sem_wait(sem_t *p_sem,u32 mSec)
{
  int ret = -1;
  u32 timeCnt = 0;
  struct timespec ts = {0};
  struct timespec tp = {0};
  int ret_ts;

  timeCnt = 0;

  ret_ts = clock_gettime(CLOCK_MONOTONIC, &ts);

  do {
    /* abs_timeout might has problem for sem_timedwait while time changed */
    ret = sem_trywait(p_sem);
    if (ret != 0)
    {
      avplay_delay_ms(1);

	  /* SYS_TaskDelay is not precise */
	  if (ret_ts == 0
	      && (clock_gettime(CLOCK_MONOTONIC, &tp) == 0))
	  {
		if (avplay_diff_time_ms(ts, tp) >= mSec)
		{
		  MT_INFO_AVPLAY("%s: clock timeout(%u)\n",__FUNCTION__,mSec);
		  return -1;
		}
	  }

      timeCnt ++;
      if(timeCnt >= mSec)
      {
        MT_INFO_AVPLAY("%s: timeout(%u)\n",__FUNCTION__,mSec);
        return -1;
      }
    }
  }while(ret != 0);

  return 0;
}
mt_void *AVPLAY_StatThread(mt_void *Arg)
{
  AVPLAY_S        *pAvplay;

	mt_set_pthread_name("AVPlay_Stat");
  pAvplay = (AVPLAY_S *)Arg;

  //setpriority(PRIO_PROCESS, gettid(), -10);
  MT_INFO_AVPLAY("%s: AVPLAY_StatThread run. tid=%d\n",__FUNCTION__, (mt_u32)syscall(__NR_gettid));
  thread_signal();

  while (pAvplay->AvplayThreadRun)
  {
	if(pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_STOP)
			goto st_sleep;

    if (pAvplay->bSetEosFlag)
    {
      AVPLAY_ProcEos(pAvplay);
    }

    AVPLAY_ProcVidEvent(pAvplay);

	AVPLAY_ProcAudEvent(pAvplay);

    AVPLAY_ProcSyncEvent(pAvplay);

    AVPLAY_ProcUnloadTime(pAvplay);

	st_sleep:
		avplay_sem_wait(pAvplay->m_vidsem, 10);
  }

  MT_INFO_AVPLAY("%s: AVPLAY_StatThread exit.\n",__FUNCTION__);
  return MT_NULL;
}

mt_void *AVPLAY_DataThread(mt_void *Arg)
{
    AVPLAY_S                        *pAvplay;
	int 							sched_nice = 0;

	mt_set_pthread_name("AVPlay_Pipe");
    pAvplay = (AVPLAY_S *)Arg;

    pAvplay->ThreadID = gettid();

	MT_INFO_AVPLAY("%s: AVPLAY_DataThread run. tid=%d\n",__FUNCTION__, pAvplay->ThreadID);
	thread_signal();

#if 0
	setpriority(PRIO_PROCESS, pAvplay->ThreadID, -20);

#else
	//set nice
	if (pAvplay->AvplayThreadPrio == THREAD_PRIO_HIGH)
	{
		sched_nice = THREAD_NICE_HIGH;
		set_sched_nice(pAvplay->ThreadID, sched_nice);
	}
	else if (pAvplay->AvplayThreadPrio == THREAD_PRIO_LOW)
	{
		sched_nice = THREAD_NICE_LOW;
		set_sched_nice(pAvplay->ThreadID, sched_nice);
	}
	else
	{
		//THREAD_PRIO_MID
	}
#endif

    while (pAvplay->AvplayThreadRun)
    {
		if(pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_STOP)
				goto data_sleep;
        sys_get_time_stamp_ms(&pAvplay->DebugInfo.ThreadBeginTime);

		//printf("\n%s: begin %u\n\n",__FUNCTION__,pAvplay->DebugInfo.ThreadBeginTime);

        if ((pAvplay->DebugInfo.ThreadBeginTime - pAvplay->DebugInfo.ThreadEndTime > AVPLAY_THREAD_TIMEOUT)
            && (0 != pAvplay->DebugInfo.ThreadEndTime)
            )
        {
            pAvplay->DebugInfo.ThreadScheTimeOutCnt++;

#ifndef CONFIG_MT_FPGA
			if (pAvplay->DebugInfo.ThreadBeginTime - pAvplay->DebugInfo.ThreadEndTime > (5*AVPLAY_THREAD_TIMEOUT))
			{
				MT_INFO_AVPLAY("Critical - end %u, begin %u, sch timeout used %u(ms)!!!\n",
					pAvplay->DebugInfo.ThreadEndTime,pAvplay->DebugInfo.ThreadBeginTime,
					pAvplay->DebugInfo.ThreadBeginTime - pAvplay->DebugInfo.ThreadEndTime);
			}
			else
			{
				if (pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_PLAY || pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_TPLAY || pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_SEEK)
                   			 MT_INFO_AVPLAY("WARNING - end %u, begin %u, sch timeout used %u(ms)\n",
					    pAvplay->DebugInfo.ThreadEndTime,pAvplay->DebugInfo.ThreadBeginTime,
					    pAvplay->DebugInfo.ThreadBeginTime - pAvplay->DebugInfo.ThreadEndTime);
			}
#endif
        }

        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);

        pAvplay->AvplayProcContinue = MT_FALSE;

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
		if(pAvplay->enable_sw_adec)
	        AVPLAY_ProcDmxToAdec(pAvplay);
#ifdef CONFIG_MT_USE_DATAPIPE_FLOW
            AVPLAY_ProcDmxToAdec(pAvplay);
#ifdef CONFIG_MT_AUDIO_AD
            AVPLAY_Proc_AD_DmxToAdec(pAvplay);
#endif
#endif
        }

		//if(pAvplay->enable_sw_adec)
			//AVPLAY_ProcAdecToAo(pAvplay);

#ifdef CONFIG_MT_USE_DATAPIPE_FLOW
        AVPLAY_ProcAdecToAo2(pAvplay);

#ifdef CONFIG_MT_CHIP_SYMPHONY4
        AVPLAY_ProcAdecToAo3(pAvplay);
#endif
#endif

#ifndef AVPLAY_VID_THREAD
        AVPLAY_ProcVdecToVo(pAvplay);
#endif

        AVPLAY_ProcDmxBuf(pAvplay);
#ifdef AVPLAY_VID_THREAD
		AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif
        AVPLAY_ProcCheckBuf(pAvplay);
#ifdef AVPLAY_VID_THREAD
		AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif

        AVPLAY_ProcCheckStandBy(pAvplay);

        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);

        sys_get_time_stamp_ms(&pAvplay->DebugInfo.ThreadEndTime);
		//printf("[ztq]App aes_level=%d, ves_level=%d\n",pAvplay->PreAudBufState,pAvplay->PreVidBufState);
		//printf("\n%s: end %u\n\n",__FUNCTION__,pAvplay->DebugInfo.ThreadEndTime);

        if ((pAvplay->DebugInfo.ThreadEndTime - pAvplay->DebugInfo.ThreadBeginTime > AVPLAY_THREAD_TIMEOUT)
			&& (pAvplay->DebugInfo.ThreadBeginTime != 0))
        {
            pAvplay->DebugInfo.ThreadExeTimeOutCnt++;

#ifndef CONFIG_MT_FPGA
			MT_INFO_AVPLAY("WARNING - begin %u, end %u, exe timeout used %u(ms)\n",
				pAvplay->DebugInfo.ThreadBeginTime,pAvplay->DebugInfo.ThreadEndTime,
				pAvplay->DebugInfo.ThreadEndTime - pAvplay->DebugInfo.ThreadBeginTime);
#endif
        }

        if (pAvplay->AvplayProcContinue)
        {
            continue;
        }
data_sleep:
        (mt_void)MT_USLEEP(AVPLAY_SYS_SLEEP_TIME*1000);
    }

	MT_INFO_AVPLAY("%s: AVPLAY_DataThread exit.\n",__FUNCTION__);
    return    MT_NULL ;
}

mt_void *AVPLAY_VidDataThread(mt_void *Arg)
{
    AVPLAY_S                        *pAvplay;
	mt_set_pthread_name("AVPlay_VPipe");
    pAvplay = (AVPLAY_S *)Arg;

    //setpriority(PRIO_PROCESS, gettid(), -20);

	MT_INFO_AVPLAY("%s: AVPLAY_VidDataThread run. tid=%d\n",__FUNCTION__, (mt_u32)syscall(__NR_gettid));
	thread_signal();

    while (pAvplay->AvplayThreadRun)
    {

		if(pAvplay->CurStatus == MT_UNF_AVPLAY_STATUS_STOP)
			goto vsleep;

		AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);

        pAvplay->AvplayVidProcContinue = MT_FALSE;

        AVPLAY_ProcVdecToVo(pAvplay);

        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
		if(MT_MPI_VDEC_GetLcevcStat(pAvplay->hVdec) == 0)
		{
			printf("LcevcStat = 0 sleep \n");
			(mt_void)MT_USLEEP(8*1000);
		}
#endif

		if (pAvplay->AvplayVidProcContinue)
		{
			continue;
		}

		vsleep:
        (mt_void)MT_USLEEP(8*1000);
    }

	MT_INFO_AVPLAY("%s: AVPLAY_VidDataThread exit.\n",__FUNCTION__);
    return    MT_NULL ;
}


mt_void AVPLAY_ResetProcFlag(AVPLAY_S *pAvplay)
{
    mt_u32 i;

    pAvplay->AvplayProcContinue = MT_FALSE;
    pAvplay->AvplayVidProcContinue = MT_FALSE;

    for (i=0; i<AVPLAY_PROC_BUTT; i++)
    {
        pAvplay->AvplayProcDataFlag[i] = MT_FALSE;
    }

    pAvplay->bSendedFrmToVirWin = MT_FALSE;

    pAvplay->PreVidBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
    pAvplay->PreAudBufState = MT_UNF_AVPLAY_BUF_STATE_EMPTY;
    pAvplay->VidDiscard = MT_FALSE;

    pAvplay->bSetEosFlag = MT_FALSE;
    pAvplay->bSetAudEos = MT_FALSE;
    pAvplay->bStandBy = MT_FALSE;

    pAvplay->AdecDelayMs = 0;

    pAvplay->u32DispOptimizeFlag = 0;

    if (MT_TRUE == pAvplay->CurBufferEmptyState)
    {
       pAvplay->PreTscnt =0;
       pAvplay->PreAudEsBuf = 0;
       pAvplay->PreAudEsBufWPtr = 0;
       pAvplay->PreVidEsBuf = 0;
       pAvplay->PreVidEsBufWPtr = 0;
       pAvplay->CurBufferEmptyState = MT_FALSE;
    }
    else
    {
       pAvplay->PreTscnt = 0xFFFFFFFF;
       pAvplay->PreAudEsBuf = 0xFFFFFFFF;
       pAvplay->PreAudEsBufWPtr = 0xFFFFFFFF;
       pAvplay->PreVidEsBuf = 0xFFFFFFFF;
       pAvplay->PreVidEsBufWPtr = 0xFFFFFFFF;
    }

    memset(&pAvplay->DebugInfo, 0, sizeof(AVPLAY_DEBUG_INFO_S));
    memset(&pAvplay->LstFrmPack, 0, sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));

    pAvplay->stIFrame.hport = MT_INVALID_HANDLE;
    memset(&pAvplay->stIFrame.stFrameVideo, 0x0, sizeof(MT_DRV_VIDEO_FRAME_S));

	//FIX: if no create/destroy,
	//	   bug only stop/start, reset, flush -> u32ScrapSize != 0 bug!
    pAvplay->AvplayVidEsBuf.u32ScrapSize = 0;
    pAvplay->u32ScrapSize = 0;

    return;
}
mt_s32 AVPLAY_CreateThread(AVPLAY_S *pAvplay)
{
#ifdef CONFIG_MT_HIGH_PERF_AVPLAY
    struct sched_param   SchedParam;
#endif
    mt_s32                 Ret = 0;

    (mt_void)pthread_attr_init(&pAvplay->AvplayThreadAttr);

#ifdef CONFIG_MT_HIGH_PERF_AVPLAY
    if (THREAD_PRIO_REALTIME == pAvplay->AvplayThreadPrio)
    {
		//FIX: AV not smooth issue.
        //(mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_FIFO);
        (mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_RR);
		(mt_void)pthread_attr_setinheritsched(&pAvplay->AvplayThreadAttr, PTHREAD_EXPLICIT_SCHED);
        (mt_void)pthread_attr_getschedparam(&pAvplay->AvplayThreadAttr, &SchedParam);
//Add define from product config
        SchedParam.sched_priority = CONFIG_MT_HIGH_PERF_AVPLAY_PRIORITYSET_PAYLOAD;
        (mt_void)pthread_attr_setschedparam(&pAvplay->AvplayThreadAttr, &SchedParam);
    }
    else
#endif
    {
        (mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_OTHER);
        (mt_void)pthread_attr_setinheritsched(&pAvplay->AvplayThreadAttr, PTHREAD_EXPLICIT_SCHED);
    }
    /* create avplay data process thread */
    Ret = pthread_create(&pAvplay->AvplayDataThdInst, &pAvplay->AvplayThreadAttr, AVPLAY_DataThread, pAvplay);
    if (MT_SUCCESS != Ret)
    {
        pthread_attr_destroy(&pAvplay->AvplayThreadAttr);
        return MT_FAILURE;
    }
    MT_INFO_AVPLAY("%s: create AVPLAY_DataThread success.\n",__FUNCTION__);
    thread_wait();

#ifdef AVPLAY_VID_THREAD
    /* create avplay data process thread */
#ifdef CONFIG_MT_HIGH_PERF_AVPLAY
	struct sched_param	 SchedParam_av;
	(mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_RR);
	(mt_void)pthread_attr_setinheritsched(&pAvplay->AvplayThreadAttr, PTHREAD_EXPLICIT_SCHED);
	(mt_void)pthread_attr_getschedparam(&pAvplay->AvplayThreadAttr, &SchedParam_av);
	//Add define from product config
	SchedParam_av.sched_priority = 99;
	(mt_void)pthread_attr_setschedparam(&pAvplay->AvplayThreadAttr, &SchedParam_av);
#else
        (mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_OTHER);
        (mt_void)pthread_attr_setinheritsched(&pAvplay->AvplayThreadAttr, PTHREAD_EXPLICIT_SCHED);
#endif
    Ret = pthread_create(&pAvplay->AvplayVidDataThdInst, &pAvplay->AvplayThreadAttr, AVPLAY_VidDataThread, pAvplay);
    if (MT_SUCCESS != Ret)
    {
        pAvplay->AvplayThreadRun = MT_FALSE;
        (mt_void)pthread_join(pAvplay->AvplayDataThdInst, MT_NULL);
        pthread_attr_destroy(&pAvplay->AvplayThreadAttr);
        return MT_FAILURE;
    }
    MT_INFO_AVPLAY("%s: create AVPLAY_VidDataThread success.\n",__FUNCTION__);
    thread_wait();
#endif

#ifdef CONFIG_MT_HIGH_PERF_AVPLAY
	struct sched_param	 SchedParam_st;
	(mt_void)pthread_attr_setschedpolicy(&pAvplay->AvplayThreadAttr, SCHED_RR);
	(mt_void)pthread_attr_setinheritsched(&pAvplay->AvplayThreadAttr, PTHREAD_EXPLICIT_SCHED);
	(mt_void)pthread_attr_getschedparam(&pAvplay->AvplayThreadAttr, &SchedParam_st);
	//Add define from product config
	SchedParam_st.sched_priority = 99;
	(mt_void)pthread_attr_setschedparam(&pAvplay->AvplayThreadAttr, &SchedParam_st);
#endif

    /* create avplay status check thread */
    Ret = pthread_create(&pAvplay->AvplayStatThdInst, &pAvplay->AvplayThreadAttr, AVPLAY_StatThread, pAvplay);
    if (MT_SUCCESS != Ret)
    {
        pAvplay->AvplayThreadRun = MT_FALSE;
#ifdef AVPLAY_VID_THREAD
        (mt_void)pthread_join(pAvplay->AvplayVidDataThdInst, MT_NULL);
#endif
        (mt_void)pthread_join(pAvplay->AvplayDataThdInst, MT_NULL);
        pthread_attr_destroy(&pAvplay->AvplayThreadAttr);
        return MT_FAILURE;
    }
    MT_INFO_AVPLAY("%s: create AVPLAY_StatThread success.\n",__FUNCTION__);
    thread_wait();

    return    MT_SUCCESS ;
}

mt_s32 AVPLAY_MallocVdec(AVPLAY_S *pAvplay, const mt_void *pPara)
{
    mt_s32  Ret;
    mt_handle hDemux = MT_INVALID_HANDLE;
    mt_u32  u32VideBufSize = pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize;

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        hDemux = pAvplay->hDmxVid;
        u32VideBufSize = pAvplay->u32VidBufSize;
        if(hDemux == MT_INVALID_HANDLE) {
            MT_ERR_AVPLAY("Error: demux channel should be open before opening the video decoder channel in ts mode\n");
            return MT_ERR_AVPLAY_DEV_OPEN_ERR;
        }
    }

    Ret = MT_MPI_VDEC_AllocChan(&pAvplay->hVdec, (const MT_UNF_AVPLAY_OPEN_OPT_S*)pPara);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_AllocChan failed 0x%x\n", Ret);
        return Ret;
    }

    Ret = MT_MPI_VDEC_ChanBufferInit(pAvplay->hVdec, u32VideBufSize, hDemux, pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanBufferInit failed %x.\n", Ret);
        (mt_void)AVPLAY_FreeVdec(pAvplay);
        return Ret;
    }

    return Ret;
}

mt_s32 AVPLAY_FreeVdec(AVPLAY_S *pAvplay)
{
    mt_s32           Ret;

    Ret = MT_MPI_VDEC_ChanBufferDeInit(pAvplay->hVdec);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanBufferDeInit failed.\n");
        return Ret;
    }

    Ret = MT_MPI_VDEC_FreeChan(pAvplay->hVdec);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_freeChan failed 0x%x\n", Ret);
        return Ret;
    }

    pAvplay->hVdec = MT_INVALID_HANDLE;

    return Ret;
}

mt_s32 AVPLAY_MallocAdec(AVPLAY_S *pAvplay)
{
    mt_s32           Ret = MT_SUCCESS;

    Ret = MT_MPI_ADEC_Open(&pAvplay->hAdec);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("MT_MPI_ADEC_Open failed 0x%x\n", Ret);
    }

    return Ret;
}

mt_s32 AVPLAY_FreeAdec(AVPLAY_S *pAvplay)
{
    mt_s32           Ret = MT_SUCCESS;

    #ifdef CONFIG_MT_AUDIO_AD
    /*if(MT_INVALID_HANDLE!=pAvplay->hAdec_AD){
        //printf("+++detach ad.decoder 0\n");
        Ret = ADEC_AD_DETTACH(pAvplay->hAdec_AD);
        pAvplay->hAdec_AD=MT_INVALID_HANDLE;
        //printf("+++detach ad.decoder 1\n");
    }*/
    #endif
    Ret = MT_MPI_ADEC_Close(pAvplay->hAdec);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("MT_MPI_ADEC_Close failed 0x%x\n", Ret);
        return Ret;
    }

    pAvplay->hAdec = MT_INVALID_HANDLE;

    return Ret;
}

mt_s32 AVPLAY_MallocDmxChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_BUFID_E BufId)
{
    mt_s32                      Ret = 0;
    MT_UNF_DMX_CHAN_ATTR_S      DmxChnAttr;
    mt_u8 esBuffId1 = 0xfe;
    memset(&DmxChnAttr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    DmxChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;

    if (MT_UNF_AVPLAY_BUF_ID_ES_VID == BufId)
    {
        DmxChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_VID;
        DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize;
        DmxChnAttr.esBuffId1 = &esBuffId1;
        DmxChnAttr.pip_en =  pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan;

        Ret = MT_MPI_DMX_CreateChannel(pAvplay->AvplayAttr.u32DemuxId, &DmxChnAttr, &pAvplay->hDmxVid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_CreateChannel failed.\n");
        }
        pAvplay->ves_buffer_channel_id = *(DmxChnAttr.esBuffId1);
        MT_INFO_AVPLAY("DmxChnAttr.esBuffId ============= %x\n", *(DmxChnAttr.esBuffId1));
    }
    else if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == BufId)
    {
        DmxChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD;
        DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize ;/// 3;
        Ret = MT_MPI_DMX_CreateChannel(pAvplay->AvplayAttr.u32DemuxId, &DmxChnAttr, &pAvplay->hDmxAud[0]);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_CreateChannel failed.\n");
            return MT_FAILURE;
        }

        pAvplay->DmxAudChnNum = 1;
    }

    return Ret;
}

mt_s32 AVPLAY_FreeDmxChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_BUFID_E BufId)
{
    mt_s32              Ret = 0;
    mt_u32              i;

    if ((MT_UNF_AVPLAY_BUF_ID_ES_VID == BufId) && (pAvplay->hDmxVid != MT_INVALID_HANDLE))
    {
        Ret = MT_MPI_DMX_DestroyChannel(pAvplay->hDmxVid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_DestroyChannel failed.\n");
            return Ret;
        }

        pAvplay->hDmxVid = MT_INVALID_HANDLE;
    }
    else if ((MT_UNF_AVPLAY_BUF_ID_ES_AUD == BufId))
    {
        #ifdef CONFIG_MT_AUDIO_AD
        if(MT_INVALID_HANDLE!=pAvplay->hDmxAud_AD){
            //printf("++destroy ad dmx 0\n");
            MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
            //printf("++destroy ad dmx 1\n");
            pAvplay->hDmxAud_AD=MT_INVALID_HANDLE;
            if(g_debug_aes){
                fclose(g_debug_aes);
                g_debug_aes=NULL;
            }
        }
        #endif

        for(i = 0; i < pAvplay->DmxAudChnNum; i++)
        {
            if(pAvplay->hDmxAud[i] != MT_INVALID_HANDLE)
            {
                Ret = MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud[i]);
                if (Ret != MT_SUCCESS)
                {
                    MT_ERR_AVPLAY("call MT_MPI_DMX_DestroyChannel failed.\n");
                    return Ret;
                }

                pAvplay->hDmxAud[i] = MT_INVALID_HANDLE;
            }
        }

        pAvplay->DmxAudChnNum = 0;
        pAvplay->CurDmxAudChn = 0;
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_MallocVidChn(AVPLAY_S *pAvplay, const mt_void *pPara)
{
    mt_s32             Ret = 0;

    MT_INFO_AVPLAY("AVPLAY_MallocVidChn \n");

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = AVPLAY_MallocDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay malloc vid dmx chn failed.\n");
            return Ret;
        }

        MT_MPI_DMX_GetEsBuffAddr(pAvplay->hDmxVid, &(pAvplay->u32VidBufUsrVirAddr), &(pAvplay->u32VidBufSize),
          &(pAvplay->u32VidBufKerVirAddr), &(pAvplay->u32VidBufKerVirDescAddr), &(pAvplay->u32DescBuffSize));

        MT_INFO_AVPLAY("YYYY 1111 ves_buffer_channel_id = %x, hVdec = %x \n", pAvplay->ves_buffer_channel_id, pAvplay->hVdec);
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_FreeVidChn(AVPLAY_S *pAvplay)
{
    mt_s32  Ret;

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = AVPLAY_FreeDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay free dmx vid chn failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_MallocAudChn(AVPLAY_S *pAvplay)
{
    mt_s32             Ret = 0;

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = AVPLAY_MallocDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay malloc aud dmx chn failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_FreeAudChn(AVPLAY_S *pAvplay)
{
    mt_s32  Ret;

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = AVPLAY_FreeDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay free dmx aud chn failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetStreamMode(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_S *pAvplayAttr)
{
    mt_s32    Ret;
    mt_sys_mem_config_s stMemConfig = {0, 0};

#if 0
    if ((pAvplayAttr->u32DemuxId != 0)
      &&(pAvplayAttr->u32DemuxId != 4)
       )
    {
        MT_ERR_AVPLAY("para pAvplayAttr->u32DemuxId is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }
#endif

    if (pAvplayAttr->stStreamAttr.enStreamType >= MT_UNF_AVPLAY_STREAM_TYPE_BUTT)
    {
        MT_ERR_AVPLAY("para pAvplayAttr->stStreamAttr.enStreamType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if ((pAvplayAttr->stStreamAttr.u32VidBufSize > AVPLAY_MAX_VID_SIZE)
      ||(pAvplayAttr->stStreamAttr.u32VidBufSize < AVPLAY_MIN_VID_SIZE)
       )
    {
        MT_ERR_AVPLAY("para pAvplayAttr->stStreamAttr.u32VidBufSize is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if ((pAvplayAttr->stStreamAttr.u32AudBufSize > AVPLAY_MAX_AUD_SIZE)
      ||(pAvplayAttr->stStreamAttr.u32AudBufSize < AVPLAY_MIN_AUD_SIZE)
       )
    {
        MT_ERR_AVPLAY("para pAvplayAttr->stStreamAttr.u32AudBufSize is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("vid chn is enable, can not set stream mode.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (pAvplay->AudEnable)
    {
        MT_ERR_AVPLAY("aud chn is enable, can not set stream mode.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (pAvplay->hVdec != MT_INVALID_HANDLE)
    {
        Ret = MT_MPI_VDEC_ChanBufferDeInit(pAvplay->hVdec);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanBufferDeInit failed.\n");
            return Ret;
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = AVPLAY_FreeDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free dmx vid chn failed.\n");
                return Ret;
            }
        }
    }

    if (pAvplay->hAdec != MT_INVALID_HANDLE)
    {
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = AVPLAY_FreeDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free dmx aud chn failed.\n");
                return Ret;
            }
        }
    }

    if (pAvplay->hDmxPcr != MT_INVALID_HANDLE)
    {
        Ret = MT_MPI_DMX_DestroyPcrChannel(pAvplay->hDmxPcr);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay free pcr chn failed.\n");
            return Ret;
        }

        pAvplay->hDmxPcr = MT_INVALID_HANDLE;
    }

    /* record stream attributes */
    memcpy(&pAvplay->AvplayAttr, pAvplayAttr, sizeof(MT_UNF_AVPLAY_ATTR_S));

    // for temp use, will delete soon, 20140717
    Ret = mt_sys_get_mem_config(&stMemConfig);
    if (MT_SUCCESS == Ret && stMemConfig.u32TotalSize == 512)
    {
        if ((pAvplay->AvplayAttr.stStreamAttr.enStreamType == MT_UNF_AVPLAY_STREAM_TYPE_ES)
            && (pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize > 32*1024*1024)
            )
        {
            pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize = 32*1024*1024;
        }
    }

    if (pAvplay->hVdec != MT_INVALID_HANDLE)
    {
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = AVPLAY_MallocDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc vid dmx chn failed.\n");
                return Ret;
            }

            Ret = MT_MPI_VDEC_ChanBufferInit(pAvplay->hVdec, 0, pAvplay->hDmxVid, pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanBufferInit failed.\n");
                (mt_void)AVPLAY_FreeDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_VID);
                return Ret;
            }
        }
    else if (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_VDEC_ChanBufferInit(pAvplay->hVdec, pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize,
              MT_INVALID_HANDLE, pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanBufferInit failed.\n");
                return Ret;
            }
        }
    }

    if (pAvplay->hAdec != MT_INVALID_HANDLE)
    {
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = AVPLAY_MallocDmxChn(pAvplay, MT_UNF_AVPLAY_BUF_ID_ES_AUD);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc aud dmx chn failed.\n");
                return Ret;
            }
        }
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_CreatePcrChannel(pAvplay->AvplayAttr.u32DemuxId, &pAvplay->hDmxPcr);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Avplay malloc pcr chn failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetStreamMode(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_S *pAvplayAttr)
{
    memcpy(pAvplayAttr, &pAvplay->AvplayAttr, sizeof(MT_UNF_AVPLAY_ATTR_S));

    return MT_SUCCESS;
}

static mt_s32 AVPLAY_SetDolbyDownmixMode(AVPLAY_S *pAvplay, const mt_u32 *pDolbyDownmixMode)
{
    mt_s32       Ret = 0;

    Ret = MT_MPI_ADEC_SetDolbyDownmixMode(pAvplay->hAdec, pDolbyDownmixMode);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_ADEC_SetDolbyDownmixMode failed.\n");
    }

    return Ret;
}

mt_s32 AVPLAY_SetAdecAttr(AVPLAY_S *pAvplay, const MT_UNF_ACODEC_ATTR_S *pAdecAttr)
{
    ADEC_ATTR_S  AdecAttr;
    mt_s32       Ret;

    if (MT_INVALID_HANDLE == pAvplay->hAdec)
    {
        MT_ERR_AVPLAY("aud chn is close, can not set adec attr.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    if (pAvplay->AudEnable)
    {
        MT_ERR_AVPLAY("aud chn is running, can not set adec attr.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
	pAvplay->AdecType = pAdecAttr->enType;

    AdecAttr.bEnable = MT_FALSE;
    AdecAttr.bEosState = MT_FALSE;
    AdecAttr.u32CodecID = (mt_u32)pAdecAttr->enType;

/* make tscancode happy */
#if 0
    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        AdecAttr.u32InBufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize;
    }
    else
#endif
    {
        AdecAttr.u32InBufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize;
    }
    AdecAttr.AudErrorCheckContinuationNum  = pAvplay->AvplayAttr.stStreamAttr.AudErrorCheckContinuationNum ;
    AdecAttr.u32OutBufNum = AVPLAY_ADEC_FRAME_NUM;
    AdecAttr.sOpenPram = pAdecAttr->stDecodeParam;
    #ifdef CONFIG_MT_AUDIO_AD
        if((MT_INVALID_HANDLE!=pAvplay->hDmxAud_AD) &&
           ((HA_AUDIO_ID_DOLBY_PLUS==AdecAttr.u32CodecID)||
            (HA_AUDIO_ID_DOLBY_TRUEHD==AdecAttr.u32CodecID)||
            (HA_AUDIO_ID_DOLBY_CONVERT==AdecAttr.u32CodecID))&&
           ((HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)||
            (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD))){   //main=dolby , ad=dolby
            AdecAttr.sOpenPram.enDecMode |= HD_DEC_MODE_PCMPCM;
            //printf("++++hack set dual dolby\n");
        }
    #endif
    Ret = MT_MPI_ADEC_SetAllAttr(pAvplay->hAdec, &AdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_ADEC_SetAllAttr failed.\n");
        return Ret;
    }
#if 0	// ahren comment
	if (pAvplay->DebugInfoExt.u32AudCrcBufSize > 0)
	{
		MT_INFO_AVPLAY("Enable Audio PCM CRC!\n");
		ADEC_Hacker_Set_PcmCrc_onlyonlyforTest(1);
	}
	else
	{
		MT_INFO_AVPLAY("Disable Audio PCM CRC!\n");
		ADEC_Hacker_Set_PcmCrc_onlyonlyforTest(0);
	}
#endif
    pAvplay->AdecType = AdecAttr.u32CodecID;

    return Ret;
}

mt_s32 AVPLAY_GetAdecAttr(const AVPLAY_S *pAvplay, MT_UNF_ACODEC_ATTR_S *pAdecAttr)
{
    ADEC_ATTR_S  AdecAttr;
    mt_s32       Ret = MT_SUCCESS;

    memset(&AdecAttr, 0x0, sizeof(ADEC_ATTR_S));

    if (MT_INVALID_HANDLE == pAvplay->hAdec)
    {
        MT_ERR_AVPLAY("aud chn is close, can not set adec attr.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_ADEC_GetAllAttr(pAvplay->hAdec, &AdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_ADEC_GetAllAttr failed.\n");
    }

    pAdecAttr->enType = (HA_CODEC_ID_E)AdecAttr.u32CodecID;
    pAdecAttr->stDecodeParam = AdecAttr.sOpenPram;

    return Ret;
}

mt_s32 AVPLAY_CheckHandle(mt_handle hAvplay, AVPLAY_USR_ADDR_S  *pAvplayUsrAddr)
{
    if ((hAvplay & 0xffff0000) != (MT_ID_AVPLAY << 16))
    {
        MT_WARN_AVPLAY("this is invalid handle.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    pAvplayUsrAddr->AvplayId = hAvplay & 0xff;

    /* check if the handle is valid */
    return ioctl(g_AvplayDevFd, CMD_AVPLAY_CHECK_ID, pAvplayUsrAddr);
}


mt_s32 AVPLAY_SetVdecAttr(AVPLAY_S *pAvplay, MT_UNF_VCODEC_ATTR_S *pVdecAttr)
{
    MT_UNF_VCODEC_ATTR_S  VdecAttr;
    mt_s32                Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec attr.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetChanAttr(pAvplay->hVdec, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanAttr failed.\n");
        return Ret;
    }

    if (pAvplay->VidEnable)
    {
        if (VdecAttr.enType != pVdecAttr->enType)
        {
            MT_ERR_AVPLAY("vid chn is running, can not set vdec type.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if (MT_UNF_VCODEC_TYPE_VC1 == VdecAttr.enType
         && (VdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile != pVdecAttr->unExtAttr.stVC1Attr.bAdvancedProfile
            || VdecAttr.unExtAttr.stVC1Attr.u32CodecVersion != pVdecAttr->unExtAttr.stVC1Attr.u32CodecVersion))
        {
            MT_ERR_AVPLAY("vid chn is running, can not set vdec type.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
    }

	//check fw parameter
	if (VdecAttr.bForceDisableTimeout
		&& pAvplay->AvplayAttr.stStreamAttr.enStreamType == MT_UNF_AVPLAY_STREAM_TYPE_TS)
	{
		MT_ERR_AVPLAY("TS play should not disable fw timeout!\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
	}

	pVdecAttr->pip_en = pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan;
    Ret = MT_MPI_VDEC_SetChanAttr(pAvplay->hVdec,pVdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_SetChanAttr failed.\n");
    }

    memcpy(&pAvplay->VdecAttr, pVdecAttr, sizeof(MT_UNF_VCODEC_ATTR_S));

    return Ret;
}


mt_s32 AVPLAY_GetVdecAttr(const AVPLAY_S *pAvplay, MT_UNF_VCODEC_ATTR_S *pVdecAttr)
{
    mt_s32                Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec attr.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetChanAttr(pAvplay->hVdec, pVdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanAttr failed.\n");
    }
	pVdecAttr->pip_en = pAvplay->AvplayAttr.stStreamAttr.vdec_pip_chan;

    return Ret;
}
#ifdef CONFIG_MT_AUDIO_AD
static mt_s32 AVPLAY_Set_AD_ATTR(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_AD_ATTR_S *adattr)
{
    mt_s32       Ret=MT_SUCCESS;
    MT_UNF_DMX_CHAN_ATTR_S DmxChnAttr={0};
	MT_UNF_DMX_CHAN_ATTR_S ChAttr;

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS != pAvplay->AvplayAttr.stStreamAttr.enStreamType){
        MT_ERR_AVPLAY("avplay is not ts mode.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* if pid is invalid, we should only disable or enable the ad */
    if(adattr->u32AdPid == 8191 || adattr->u32AdPid == 0) {
        printf("do ad on/off %u\n", adattr->bAdEnble);
        ADEC_AD_SET_DATATYPE(pAvplay->hAdec, adattr->bAdEnble);
        pAvplay->adectype_AD=adattr->adectype;
        return MT_SUCCESS;
    }

    if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD) {
        MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
        MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
        pAvplay->hDmxAud_AD=MT_INVALID_HANDLE;
    }
    /*if(MT_INVALID_HANDLE != pAvplay->hAdec_AD){
        ADEC_AD_DETTACH(pAvplay->hAdec_AD);
        pAvplay->hAdec_AD=MT_INVALID_HANDLE;
    }*/
    if (adattr->bAdEnble)
    {
        DmxChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
        DmxChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD_AD;
        DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize;
        Ret = MT_MPI_DMX_CreateChannel(pAvplay->AvplayAttr.u32DemuxId, &DmxChnAttr, &pAvplay->hDmxAud_AD);
        if(MT_SUCCESS != Ret){
            MT_ERR_AVPLAY("call MT_MPI_DMX_CreateChannel failed\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

		MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxAud_AD, &ChAttr);
	    ChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD_AD;
	    ChAttr.AVSyncFlag = 1;
	    Ret = MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxAud_AD, &ChAttr);
		if(MT_SUCCESS != Ret){
            MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelAttr failed\n");
            MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
            MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
            pAvplay->hDmxAud_AD=MT_INVALID_HANDLE;
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_SetChannelPID(pAvplay->hDmxAud_AD, adattr->u32AdPid);
        if(MT_SUCCESS != Ret){
            MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelPID failed\n");
            MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
            MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
            pAvplay->hDmxAud_AD=MT_INVALID_HANDLE;
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
        /*if(0){//attach ap decoder.
            pAvplay->hAdec_AD=ADEC_AD_ATTACH(adattr->adectype);
            if(0==pAvplay->hAdec_AD){
                MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
                MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
                pAvplay->hDmxAud_AD=MT_INVALID_HANDLE;
                pAvplay->hAdec_AD=MT_INVALID_HANDLE;
                MT_ERR_AVPLAY("call adec attach failed\n");
                return MT_ERR_AVPLAY_INVALID_OPT;
            }
            ADEC_AD_SET_DATATYPE(1);
        }else */ /*if((HA_AUDIO_ID_DOLBY_PLUS==adattr->adectype)||
                 (HA_AUDIO_ID_DOLBY_TRUEHD==adattr->adectype)||
                 (HA_AUDIO_ID_MP2==adattr->adectype)||
                 (HA_AUDIO_ID_MP3==adattr->adectype)||
                 (HA_AUDIO_ID_AAC==adattr->adectype)){
            ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 1);
        }else{
            ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
        }*/
        pAvplay->adectype_AD=adattr->adectype;
    }
    if(MT_INVALID_HANDLE!=pAvplay->hDmxAud_AD){
        ;//g_debug_aes=fopen("/media/casetest/debug_aes","wb");
    }
    //printf("++st ad attr7 %x,%x\n",pAvplay->hDmxAud_AD,pAvplay->hAdec_AD);
    return Ret;
}

static mt_s32 AVPLAY_Set_AD_vol_weight(AVPLAY_S *pAvplay, mt_u32 *vol_weight)
{
	return ADEC_AD_SET_VOL_WEIGHT(vol_weight);
}

static mt_s32 AVPLAY_Get_AD_vol_weight(AVPLAY_S *pAvplay, mt_u32 *vol_weight)
{
	return ADEC_AD_GET_VOL_WEIGHT(vol_weight);
}

#endif

mt_s32 AVPLAY_SetPid(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, const mt_u32 *pPid)
{
    mt_s32       Ret;
    mt_u32       i;
    mt_u32 pid = (mt_u32)(*pPid);

    if (pAvplay->AvplayAttr.stStreamAttr.enStreamType != MT_UNF_AVPLAY_STREAM_TYPE_TS)
    {
        MT_ERR_AVPLAY("avplay is not ts mode.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (MT_UNF_AVPLAY_ATTR_ID_AUD_PID == enAttrID)
    {
        if (MT_INVALID_HANDLE == pAvplay->hDmxAud[0])
        {
            MT_ERR_AVPLAY("aud chn is close, can not set aud pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if(pAvplay->DmxAudChnNum == 1)
        {
            if (pAvplay->AudEnable)
            {
                MT_ERR_AVPLAY("aud chn is running, can not set aud pid.\n");
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            Ret = MT_MPI_DMX_SetChannelPID(pAvplay->hDmxAud[0], pid);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelPID failed.\n");
            }

            pAvplay->DmxAudPid[0] = pid;

            pAvplay->CurDmxAudChn = 0;
        }
        /*multi audio*/
        else
        {
            MT_BOOL old_AudEnable = MT_FALSE;
            mt_u32 track = 0;

            AVPLAY_Mutex_Lock(pAvplay->pAvplayThreadMutex);

            /*
            patch for bug129915, when do audio track, avplay should check wether
            the new pid is the same with current pid
            */
            if (pid == pAvplay->DmxAudPid[pAvplay->CurDmxAudChn])
            {
                Ret = MT_SUCCESS;
                MT_ERR_AVPLAY("set a same aud pid again\n");
                AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
                return Ret;
            }

            for (i = 0; i < pAvplay->DmxAudChnNum; i++)
            {
                if(pAvplay->DmxAudPid[i] == pid)
                {
                    break;
                }
            }

            /* Do demux stop */
            MT_MPI_DMX_DataPushStop(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);

            if(i < pAvplay->DmxAudChnNum)
            {
                /* if the es buf has not been released */
                if (pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC])
                {
                    (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &pAvplay->AvplayDmxEsBuf);
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC] = MT_FALSE;
                }

                pAvplay->CurDmxAudChn = i;
            }

            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
#endif

            (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);

			(mt_void)MT_MPI_ADEC_Stop(pAvplay->hAdec, 1);		//stop adec first
            (mt_void)MT_MPI_AO_Track_Stop((mt_handle)NULL);	//then stop snd

            //(mt_void)MT_MPI_AO_Track_Flush((mt_handle)NULL);

            if (MT_NULL != pAvplay->pstAcodecAttr)
            {
                old_AudEnable=pAvplay->AudEnable;
                pAvplay->AudEnable=MT_FALSE;
                (mt_void)AVPLAY_SetAdecAttr(pAvplay, (MT_UNF_ACODEC_ATTR_S *)(pAvplay->pstAcodecAttr + pAvplay->CurDmxAudChn));
                pAvplay->AudEnable=old_AudEnable;
            }

            /*
            after AVPLAY_SetAdecAttr(), all the aud buf be reseted, and then audio pts fifo should be reseted too
            if not, after audio trick, there are some apts which belonges to pre audio will be pushed to avsync
            */
            Ret = MT_MPI_SYNC_Aud_Init(pAvplay->hSync, (mt_u32)pAvplay->AdecType);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("MT_MPI_SYNC_Aud_Init failed.\n");
            }

            track = 1;
            Ret = MT_MPI_SYNC_Aud_Track(pAvplay->hSync, &track);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("MT_MPI_SYNC_Aud_Track failed.\n");
            }

            (mt_void)MT_MPI_ADEC_Start(pAvplay->hAdec, pAvplay->AdecType);

            (mt_void)MT_MPI_AO_Track_Start((mt_handle)NULL);

            (mt_void)MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_AUD);

            /* Do demux start */
            MT_MPI_DMX_DataPushStart(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);

            AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);

            Ret = MT_SUCCESS;
        }
    }
    else if (MT_UNF_AVPLAY_ATTR_ID_VID_PID == enAttrID)
    {
        if (pAvplay->VidEnable)
        {
            MT_ERR_AVPLAY("vid chn is running, can not set vid pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if (MT_INVALID_HANDLE == pAvplay->hDmxVid)
        {
            MT_ERR_AVPLAY("vid chn is close, can not set vid pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_SetChannelPID(pAvplay->hDmxVid, pid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelPID failed.\n");
        }

        pAvplay->DmxVidPid = pid;
    }
    else
    {
        /*if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_STOP)
        {
            MT_ERR_AVPLAY("AVPLAY is not stopped, can not set pcr pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }*/

        if (MT_INVALID_HANDLE == pAvplay->hDmxPcr)
        {
            MT_ERR_AVPLAY("pcr chn is close, can not set pcr pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, pid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
        }

        pAvplay->DmxPcrPid = pid;
    }

    return Ret;
}

mt_s32 AVPLAY_GetPid(const AVPLAY_S *pAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_u32 *pPid)
{
    mt_s32       Ret;

    if (pAvplay->AvplayAttr.stStreamAttr.enStreamType != MT_UNF_AVPLAY_STREAM_TYPE_TS)
    {
        MT_ERR_AVPLAY("avplay is not ts mode.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (MT_UNF_AVPLAY_ATTR_ID_AUD_PID == enAttrID)
    {
        if (MT_INVALID_HANDLE == pAvplay->hAdec)
        {
            MT_ERR_AVPLAY("aud chn is close, can not get aud pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_GetChannelPID(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], pPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_GetChannelPID failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_ATTR_ID_VID_PID == enAttrID)
    {
        if (MT_INVALID_HANDLE == pAvplay->hVdec)
        {
            MT_ERR_AVPLAY("vid chn is close, can not get vid pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_GetChannelPID(pAvplay->hDmxVid, pPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_GetChannelPID failed.\n");
        }
    }
    else
    {
        if (MT_INVALID_HANDLE == pAvplay->hDmxPcr)
        {
            MT_ERR_AVPLAY("pcr chn is close, can not get pcr pid.\n");
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_DMX_PcrPidGet(pAvplay->hDmxPcr, pPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidGet failed.\n");
        }
    }

    return Ret;
}

mt_s32 AVPLAY_SetSyncAttr(AVPLAY_S *pAvplay, MT_UNF_SYNC_ATTR_S *pSyncAttr)
{
    mt_s32                Ret;

    Ret = MT_MPI_SYNC_SetAttr(pAvplay->hSync, pSyncAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_SetAttr failed.\n");
    }

    return Ret;
}

mt_s32 AVPLAY_GetSyncAttr(AVPLAY_S *pAvplay, MT_UNF_SYNC_ATTR_S *pSyncAttr)
{
    mt_s32 Ret;

    Ret = MT_MPI_SYNC_GetAttr(pAvplay->hSync, pSyncAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_GetAttr failed.\n");
    }

    return Ret;
}

mt_s32 AVPLAY_SetOverflowProc(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_OVERFLOW_E *pOverflowProc)
{
    if (*pOverflowProc >= MT_UNF_AVPLAY_OVERFLOW_BUTT)
    {
        MT_ERR_AVPLAY("para OverflowProc is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    pAvplay->OverflowProc = *pOverflowProc;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetOverflowProc(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_OVERFLOW_E *pOverflowProc)
{
    *pOverflowProc = pAvplay->OverflowProc;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetLowDelay(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr)
{
    mt_s32                  Ret;
    mt_u32                  i;
    MT_UNF_SYNC_ATTR_S      stSyncAttr;
    MT_CODEC_VIDEO_CMD_S    stVdecCmd;
    MT_BOOL   bProgressive;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chan is closed!\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("vid chan is running!\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if ((MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
        && (0 == pAvplay->SlaveChnNum)
        && (0 == pAvplay->VirChnNum)
        )
    {
        MT_ERR_AVPLAY("there is now window attached, can not set low delay!\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* set window to quickoutput mode */
    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        Ret = MT_MPI_WIN_SetQuickOutput(pAvplay->MasterFrmChn.hWindow, pstAttr->bEnable);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_WIN_SetQuickOutput ERR, Ret=%x\n", Ret);
            return Ret;
        }
    }

    for (i = 0; i < pAvplay->SlaveChnNum; i++)
    {
        Ret = MT_MPI_WIN_SetQuickOutput(pAvplay->SlaveFrmChn[i].hWindow, pstAttr->bEnable);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_WIN_SetQuickOutput ERR, Ret=%x\n", Ret);
        }
    }

    /* set vdec to lowdelay mode */
    Ret = MT_MPI_VDEC_SetLowDelay(pAvplay->hVdec, pstAttr);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_SetLowDelay ERR, Ret=%x\n", Ret);
        return Ret;
    }

    memset(&stSyncAttr, 0, sizeof(stSyncAttr));
    if (pstAttr->bEnable)
    {
        /* set sync to none */
        Ret = MT_MPI_SYNC_GetAttr(pAvplay->hSync, &stSyncAttr);
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
        Ret |= MT_MPI_SYNC_SetAttr(pAvplay->hSync, &stSyncAttr);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_SYNC_SetAttr ERR, Ret=%x\n", Ret);
            return Ret;
        }

        bProgressive = MT_TRUE;
        stVdecCmd.u32CmdID = MT_UNF_AVPLAY_SET_PROGRESSIVE_CMD;
        stVdecCmd.pPara = (mt_void *)&bProgressive;
        Ret = MT_MPI_VDEC_Invoke(pAvplay->hVdec, &stVdecCmd);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_VDEC_Invoke ERR, Ret=%x\n", Ret);
            return Ret;
        }
    }
    else
    {
        /* set sync to audio */
        Ret = MT_MPI_SYNC_GetAttr(pAvplay->hSync, &stSyncAttr);
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        Ret |= MT_MPI_SYNC_SetAttr(pAvplay->hSync, &stSyncAttr);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_SYNC_SetAttr ERR, Ret=%x\n", Ret);
            return Ret;
        }

        bProgressive = MT_FALSE;
        stVdecCmd.u32CmdID = MT_UNF_AVPLAY_SET_PROGRESSIVE_CMD;
        stVdecCmd.pPara = (mt_void *)&bProgressive;
        Ret = MT_MPI_VDEC_Invoke(pAvplay->hVdec, &stVdecCmd);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_VDEC_Invoke ERR, Ret=%x\n", Ret);
            return Ret;
        }
    }

    pAvplay->LowDelayAttr = *pstAttr;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetDmxAvsync(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *pDmxAvsync)
{
    MT_UNF_DMX_CHAN_ATTR_S ChAttr;
    int i = 0;

    MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxVid, &ChAttr);
    ChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_VID;
    ChAttr.AVSyncFlag = pDmxAvsync->AvsyncFlage;
    ChAttr.vCodecType = pDmxAvsync->VdecType;
    MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxVid, &ChAttr);
    MT_INFO_AVPLAY("\n  pAvplay->hDmxAud[pAvplay->CurDmxAudChn] 0x%x  CurDmxAudChn %d\n\n",
      pAvplay->hDmxAud[pAvplay->CurDmxAudChn], pAvplay->CurDmxAudChn);


    MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &ChAttr);
    ChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD;
    ChAttr.AVSyncFlag = pDmxAvsync->AvsyncFlage;

    for(i = 0; i < pAvplay->DmxAudChnNum; i++) {
        MT_ALWAYS_PRINT("=> set av sync for audio channel %d, flag %u\n", i, pDmxAvsync->AvsyncFlage);
        MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxAud[i], &ChAttr);
    }

    return MT_SUCCESS;
}

static mt_s32 AVPLAY_GetDmxAvsync(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *pDmxAvsync)
{
    MT_UNF_DMX_CHAN_ATTR_S VidChAttr;
    mt_s32 ret;

    /* always insert pts in driver */
    pDmxAvsync->AvsyncFlage = MT_TRUE;

    ret = MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxVid, &VidChAttr);
    if(ret == MT_SUCCESS)
        pDmxAvsync->VdecType = VidChAttr.vCodecType;

    /* no need to get audio channel attribute because audio codec type not exist in attribute */

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetDmxAudiosync(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *pDmxAvsync)
{
    MT_UNF_DMX_CHAN_ATTR_S ChAttr;
    mt_s32  ret = MT_SUCCESS;
    mt_u32 i = 0;

    for (i = 0; i < pAvplay->DmxAudChnNum; i++)
    {
        ret = MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxAud[i], &ChAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_AVPLAY("%s, %d, GetChannelAttr failed, ret:%d, Achn:%#x\n", ret, pAvplay->hDmxAud[i]);
            return ret;
        }

        ChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD;
        ChAttr.AVSyncFlag = pDmxAvsync->AvsyncFlage;

        ret = MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxAud[i], &ChAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_AVPLAY("%s, %d, SetChannelAttr failed, ret:%d, Achn:%#x\n", ret, pAvplay->hDmxAud[i]);
            return ret;
        }

    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetWatermarkFilter(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_WATERMARK_FILTER_ATTR_S *pAttr)
{
    MT_UNF_DMX_CHAN_ATTR_S ChAttr;
    mt_u32 bAudioWatermarkFiltering = 0;
    mt_u32 bVideoWatermarkFiltering = 0;
    mt_s32  ret = MT_SUCCESS;
    mt_u32 i = 0;

    if(pAttr == NULL || pAvplay == NULL) {
        printf("%s: invalid params\n", __func__);
        return MT_FAILURE;
    }

    bAudioWatermarkFiltering = pAttr->bAudioEnable;
    for (i = 0; i < pAvplay->DmxAudChnNum; i++)
    {
        ret = MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxAud[i], &ChAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_AVPLAY("GetChannelAttr failed, ret:%d, Audio Chn:%#x\n", ret, pAvplay->hDmxAud[i]);
            return ret;
        }

        ChAttr.enDiscardErrorPUSIPacket = bAudioWatermarkFiltering;
        ret = MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxAud[i], &ChAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_AVPLAY("SetChannelAttr failed, ret:%d, Audio Chn:%#x\n", ret, pAvplay->hDmxAud[i]);
            return ret;
        }
    }

    bVideoWatermarkFiltering = pAttr->bVideoEnable;
    ret = MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxVid, &ChAttr);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AVPLAY("GetChannelAttr failed, ret:%d, Video Chn:%#x\n", ret, pAvplay->hDmxVid);
        return ret;
    }

    ChAttr.enDiscardErrorPUSIPacket = bVideoWatermarkFiltering;
    ret = MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxVid, &ChAttr);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_AVPLAY("SetChannelAttr failed, ret:%d, Video Chn:%#x\n", ret, pAvplay->hDmxVid);
        return ret;
    }

    return MT_SUCCESS;
}

static mt_s32 AVPLAY_AvcConfig(AVPLAY_S *pAvplay, MT_HADECODE_AVC_PARAM_S * pAvcParam)
{
    mt_s32 ret;

    if (MT_INVALID_HANDLE == pAvplay->hAdec)
    {
        MT_ERR_AVPLAY("aud chn is close, can not set aud pid.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    ret = MT_MPI_ADEC_AvcConfig(pAvplay->hAdec, pAvcParam);

    return ret;
}

/* apts adjust for audio fast/slow*/
static mt_s32 AVPLAY_AptsAdjust(AVPLAY_S *pAvplay, mt_u32 u32AptsAdjust)
{
    mt_s32 ret;

    if (MT_INVALID_HANDLE == pAvplay->hSync)
    {
        MT_ERR_AVPLAY("avplay handle err.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    ret = MT_MPI_SYNC_AptsAdjust(pAvplay->hSync, u32AptsAdjust);

    return ret;
}


mt_s32 AVPLAY_GetLowDelay(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *pstAttr)
{
    *pstAttr = pAvplay->LowDelayAttr;

    return MT_SUCCESS;
}


mt_s32 AVPLAY_RelSpecialFrame(AVPLAY_S *pAvplay, mt_handle hWin)
{
    mt_u32                              i;
    mt_handle                           hWindow = MT_INVALID_HANDLE;
    mt_s32                              Ret;

    for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);
        if (hWindow == hWin)
        {
            Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
            if (MT_SUCCESS != Ret)
            {
                (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_RelAllVirChnFrame(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    for (i = 0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
    {
        (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

        for (j=0; j<pAvplay->VirChnNum; j++)
        {
            if (hWindow == pAvplay->VirFrmChn[j].hWindow)
            {
                Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                if (MT_SUCCESS != Ret)
                {
                    (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                }
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_RelAllChnFrame(AVPLAY_S *pAvplay)
{
    mt_s32                              Ret;
    mt_u32                              i, j;
    mt_handle                           hWindow = MT_INVALID_HANDLE;

    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        AVPLAY_RelAllVirChnFrame(pAvplay);
    }
    else
    {
        for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

             /* may be CurFrmPack has not master frame */
             if (hWindow == pAvplay->MasterFrmChn.hWindow)
             {
                 Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                 if (MT_SUCCESS != Ret)
                 {
                    (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                 }
                 break;
             }
        }

        for (i=0; i<pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, pAvplay->CurFrmPack.stFrame[i].hport, &hWindow);

            for (j=0; j<pAvplay->SlaveChnNum; j++)
            {
                if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
                {
                     Ret = MT_MPI_WIN_QueueUselessFrame(hWindow, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                     if (MT_SUCCESS != Ret)
                     {
                        (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &pAvplay->CurFrmPack.stFrame[i].stFrameVideo);
                     }
                }
            }
        }

        memset(&pAvplay->LstFrmPack, 0, sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_StartVidChn(const AVPLAY_S *pAvplay)
{
	mt_s32         Ret;
	mt_u32       i = 0;

       MT_MPI_SYNC_Vid_Init(pAvplay->hSync, (mt_u32)pAvplay->VdecAttr.enType);

	Ret = MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_VID);
	if (Ret != MT_SUCCESS)
	{
	    MT_ERR_AVPLAY("call MT_MPI_SYNC_Start Vid failed.\n");
	    return Ret;
	}

	if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
	{
	    Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxVid);
	    if (Ret != MT_SUCCESS)
	    {
	        MT_ERR_AVPLAY("call MT_MPI_DMX_OpenChannel failed, Ret=%x.\n", Ret);
	        (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);
	        return Ret;
	    }
	}

	Ret = MT_MPI_VDEC_ChanStart(pAvplay->hVdec);
	if (Ret != MT_SUCCESS)
	{
	    MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanStart failed, Ret=%x.\n", Ret);

	    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
	    {
	        (mt_void)MT_MPI_DMX_CloseChannel(pAvplay->hDmxVid);
	    }

	    (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);

	    return Ret;
	}

	//could control by application
	if (pAvplay->VdecAttr.enUnBlank != MT_UNF_VCODEC_UNBLANK_USER)
	{
		MT_MPI_DISP_VidLayerShow(MT_TRUE);
	}

	//ykang,fixbug 100828, according comment5.
	if (1)//pAvplay->VidEnable
	{
	    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
	    {
	          MT_INFO_AVPLAY("\n\n\n MT_MPI_WIN_Freeze called in MasterFrmChn\n");

	        (mt_void)MT_MPI_WIN_Freeze(pAvplay->MasterFrmChn.hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
	    }

	    for (i=0; i<pAvplay->SlaveChnNum; i++)
	    {
	    	   MT_INFO_AVPLAY("\n\n\n MT_MPI_WIN_Freeze called in SlaveChnNum %d\n",i);

	        (mt_void)MT_MPI_WIN_Freeze(pAvplay->SlaveFrmChn[i].hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
	    }

	    for (i=0; i<pAvplay->VirChnNum; i++)
	    {
	    	   MT_INFO_AVPLAY("\n\n\n MT_MPI_WIN_Freeze called in VirChnNum %d\n",i);

	        (mt_void)MT_MPI_WIN_Freeze(pAvplay->VirFrmChn[i].hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
	    }
	}


    return MT_SUCCESS;
}

mt_s32 AVPLAY_ResetWindow(const AVPLAY_S *pAvplay, MT_DRV_WIN_SWITCH_E SwitchType)
{
    mt_u32                  i;
    MT_DRV_WIN_INFO_S       stWinInfo;

    memset(&stWinInfo, 0, sizeof(MT_DRV_WIN_INFO_S));

    if (MT_INVALID_HANDLE != pAvplay->hSharedOrgWin)
    {
        (mt_void)MT_MPI_WIN_GetInfo(pAvplay->hSharedOrgWin, &stWinInfo);

        (mt_void)MT_MPI_WIN_Reset(pAvplay->hSharedOrgWin, SwitchType);
    }
    else
    {
        stWinInfo.hPrim = MT_INVALID_HANDLE;
        stWinInfo.hSec = MT_INVALID_HANDLE;
    }

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        if (pAvplay->MasterFrmChn.hWindow != stWinInfo.hPrim)
        {
            (mt_void)MT_MPI_WIN_Reset(pAvplay->MasterFrmChn.hWindow, SwitchType);
        }
    }

    for (i=0; i<pAvplay->SlaveChnNum; i++)
    {
        if (pAvplay->SlaveFrmChn[i].hWindow != stWinInfo.hSec)
        {
            (mt_void)MT_MPI_WIN_Reset(pAvplay->SlaveFrmChn[i].hWindow, SwitchType);
        }
    }

    for (i=0; i<pAvplay->VirChnNum; i++)
    {
        (mt_void)MT_MPI_WIN_Reset(pAvplay->VirFrmChn[i].hWindow, SwitchType);
    }

    return MT_SUCCESS;
}

/* flush all windows in playing state */
mt_s32 AVPLAY_FlushWindow(const AVPLAY_S *pAvplay, MT_DRV_WIN_FLUSH_TYPE_E eType)
{
    mt_u32                  i;
    MT_DRV_WIN_INFO_S       stWinInfo;

    memset(&stWinInfo, 0, sizeof(MT_DRV_WIN_INFO_S));

    if (MT_INVALID_HANDLE != pAvplay->hSharedOrgWin)
    {
        (mt_void)MT_MPI_WIN_GetInfo(pAvplay->hSharedOrgWin, &stWinInfo);

        //(mt_void)MT_MPI_WIN_Reset(pAvplay->hSharedOrgWin, SwitchType);
         MT_MPI_WIN_CleanAllFrm(pAvplay->MasterFrmChn.hWindow, eType);
    }
    else
    {
        stWinInfo.hPrim = MT_INVALID_HANDLE;
        stWinInfo.hSec = MT_INVALID_HANDLE;
    }

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        if (pAvplay->MasterFrmChn.hWindow != stWinInfo.hPrim)
        {
            //(mt_void)MT_MPI_WIN_Reset(pAvplay->MasterFrmChn.hWindow, SwitchType);
            MT_MPI_WIN_CleanAllFrm(pAvplay->MasterFrmChn.hWindow, eType);
        }
    }

    for (i=0; i<pAvplay->SlaveChnNum; i++)
    {
        if (pAvplay->SlaveFrmChn[i].hWindow != stWinInfo.hSec)
        {
            //(mt_void)MT_MPI_WIN_Reset(pAvplay->SlaveFrmChn[i].hWindow, SwitchType);
            MT_MPI_WIN_CleanAllFrm(pAvplay->SlaveFrmChn[i].hWindow, eType);
        }
    }

    for (i=0; i<pAvplay->VirChnNum; i++)
    {
        //(mt_void)MT_MPI_WIN_Reset(pAvplay->VirFrmChn[i].hWindow, SwitchType);
        MT_MPI_WIN_CleanAllFrm(pAvplay->VirFrmChn[i].hWindow, eType);
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_StopVidChn(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_STOP_MODE_E enMode)
{
    mt_s32                  Ret;
    MT_DRV_WIN_SWITCH_E     SwitchType = MT_DRV_WIN_SWITCH_BUTT;
	pAvplay->DebugInfo.FirstVidShowd = 0;
	//FIXED: must reset win first, for it may acquire freeze frame buffer from vfmw.
	//if stop first, it'll crash while acquire freeze frame buffer from vfmw.
    if (MT_UNF_AVPLAY_STOP_MODE_STILL == enMode)
    {
        SwitchType = MT_DRV_WIN_SWITCH_LAST;
		//for bug 128198, when no frame
		MT_UNF_DMX_PORT_MODE_E  PortMode = MT_UNF_DMX_PORT_MODE_BUTT;
		(mt_void)MT_MPI_DMX_GetPortMode(pAvplay->AvplayAttr.u32DemuxId, &PortMode);
        if (PortMode <MT_UNF_DMX_PORT_MODE_RAM)
		{
			if(pAvplay->DebugInfo.MasterVidStat.SendNum > 1)
				SwitchType = MT_DRV_WIN_SWITCH_LAST;
			else
				SwitchType = MT_DRV_WIN_SWITCH_BLACK;
		}
		//end bug 128198
    }
    else
    {
        SwitchType = MT_DRV_WIN_SWITCH_BLACK;
    }

    Ret = MT_MPI_VDEC_ChanStop(pAvplay->hVdec);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanStop failed.\n");
        return Ret;
    }

    Ret = AVPLAY_ResetWindow(pAvplay, SwitchType);

    Ret = MT_MPI_VDEC_ResetChan(pAvplay->hVdec, NULL);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ResetChan failed.\n");
        return Ret;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxVid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_CloseChannel failed.\n");
            return Ret;
        }
    }

    MT_MPI_SYNC_Vid_DeInit(pAvplay->hSync);

    Ret = MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Stop Vid failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_StartAudChn(AVPLAY_S *pAvplay)
{
    mt_s32         Ret;
    mt_u32         i, j;

	if (pAvplay->DebugInfoExt.pu32AudCrcBuf != NULL)
	{
		memset(pAvplay->DebugInfoExt.pu32AudCrcBuf, 0, pAvplay->DebugInfoExt.u32AudCrcBufSize);
		pAvplay->DebugInfoExt.u32AudCrcBufWrPtr = 0;
	}

    MT_MPI_SYNC_Aud_Init(pAvplay->hSync, (mt_u32)pAvplay->AdecType);

    Ret = MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_AUD);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Start Aud failed.\n");
        return Ret;
    }

#ifdef CONFIG_MT_AUDIO_AD
	if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD){
		if((HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)||
			 (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD)||
			 (HA_AUDIO_ID_MP2==pAvplay->adectype_AD)||
			 (HA_AUDIO_ID_MP3==pAvplay->adectype_AD)||
			 (HA_AUDIO_ID_AAC==pAvplay->adectype_AD)){
			ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 1);
		}else{
			ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
		}
	}
#endif

    Ret = MT_MPI_ADEC_Start(pAvplay->hAdec, pAvplay->AdecType);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_ADEC_Start failed.\n");
        (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);
        return Ret;
    }

    /* get the string of adec type */
    (mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_HaSzNameInfo, &(pAvplay->AdecNameInfo));

    Ret = MT_MPI_AO_Track_Start(0);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("call MT_MPI_AO_Track_Start failed.\n");
        return Ret;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
      MT_INFO_AVPLAY("\n\n\n  AVPLAY_StartAudChn22222    pAvplay->DmxAudChnNum %d \n\n\n", pAvplay->DmxAudChnNum);

        {
            #ifdef CONFIG_MT_AUDIO_AD
            //MT_UNF_SND_GAIN_ATTR_S gain={MT_TRUE,(mt_s32)0};
			ulong kerVirEsBuffAddr = 0;
            //printf("+++start ad chanenl begin\n");
            if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD){
                Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxAud_AD);
                if(MT_SUCCESS == Ret){
                    //printf("\n\n\n\n\n MT_MPI_DMX_OpenChannel ad good,break   \n\n\n\n\n", i);
                    memset(&pAvplay->DmxChanInfoAud_AD, 0, sizeof(DMX_CHAN_INFO_S));
                    MT_MPI_DMX_GetEsBuffAddr(pAvplay->hDmxAud_AD,
                        &(pAvplay->DmxChanInfoAud_AD.dmx_buf_addr),
                        &(pAvplay->DmxChanInfoAud_AD.dmx_buf_len),
                        &kerVirEsBuffAddr,
                        NULL, NULL);
                }
                //gain.s32Gain=50;
				#if 0	// ahren comment
                ret=MT_MPI_AO_SND_SetVolume(MT_UNF_SND_0,MT_UNF_SND_OUTPUTPORT_AD,&gain);
				#endif
                //printf("+++start ad chanenl over %x %x\n",MT_UNF_SND_OUTPUTPORT_AD,ret);
            }else{  //close ad volume
                ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
                //printf("+++type3.set 0\n");
                //gain.s32Gain=0;
				#if 0	// ahren comment
                ret=MT_MPI_AO_SND_SetVolume(MT_UNF_SND_0,MT_UNF_SND_OUTPUTPORT_AD,&gain);
				#endif
                //printf("+++start no ad chanenl over %x %x\n",MT_UNF_SND_OUTPUTPORT_AD,ret);
            }
            #endif
        }
        for(i= 0; i < pAvplay->DmxAudChnNum; i++)
        {
            ulong kerVirEsBuffAddr = 0;
            mt_u32 esBuffSize = 0;
            Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxAud[i]);
            if(MT_SUCCESS != Ret)
            {
                MT_INFO_AVPLAY("\n\n\n\n\n MT_MPI_DMX_OpenChannel %d error,break   \n\n\n\n\n", i);
                break;
            }

            MT_INFO_AVPLAY("\n\n\n\n\n\n  AVPLAY_StartAudChn22222  before MT_MPI_DMX_GetEsBuffAddr\n\n\n\n\n\n\n");

			//FIX Bug: dmx_buf_read_pnt be memset to zero only at AVPlay Create,
			//         but if APP do not Destroy/Create AVPlay, but Stop/Start,
			//		   the 'dmx_buf_read_pnt' is not correct.
			//pAvplay->DmxChanInfoAud[i].dmx_buf_read_pnt = 0;
			memset(&pAvplay->DmxChanInfoAud[i], 0, sizeof(DMX_CHAN_INFO_S));

            //cooperadd  get dmx addr and size
                MT_MPI_DMX_GetEsBuffAddr(pAvplay->hDmxAud[i],
                    &(pAvplay->DmxChanInfoAud[i].dmx_buf_addr),
                    &(pAvplay->DmxChanInfoAud[i].dmx_buf_len),
                    &kerVirEsBuffAddr,
                    NULL, NULL);

				MT_MPI_DMX_GetEsBuffPhyAddr(pAvplay->hDmxAud[i],
					&(pAvplay->DmxChanInfoAud[i].dmx_buf_phy_addr),&esBuffSize);

                MT_INFO_AVPLAY("\n\n\n\n\n\n\n\n AVPLAY_StartAudChn333 get es buf addr 0x%x len 0x%x\n\n\n\n\n\n\n\n\n",
                  pAvplay->DmxChanInfoAud[i].dmx_buf_addr, pAvplay->DmxChanInfoAud[i].dmx_buf_len);

        }

        if(i < pAvplay->DmxAudChnNum)
        {
            #ifdef CONFIG_MT_AUDIO_AD
            if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD){
                MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
                pAvplay->hDmxAud_AD = MT_INVALID_HANDLE;
            }
            #endif
            for(j = 0; j < i; j++)
            {
                (mt_void)MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud[j]);
            }

            MT_ERR_AVPLAY("call MT_MPI_DMX_OpenChannel failed.\n");
            (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_StartAudDec(mt_handle hAvplay)
{
    AVPLAY_S   *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32         Ret;

    AVPLAY_GET_INST_AND_LOCK();

    if (!pAvplay->AudEnable)
    {
        Ret = MT_MPI_ADEC_Start(pAvplay->hAdec, pAvplay->AdecType);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("ADEC Start failed\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pAvplay->AudEnable = MT_TRUE;
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_AVPLAY("ADEC Started\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_FAILURE;
    }
}

mt_s32 AVPLAY_StopAudChn(AVPLAY_S *pAvplay, mt_u32 reset)
{
    mt_s32         Ret;
    mt_u32         i;

	pAvplay->enable_sw_adec = 0;

    Ret = MT_MPI_ADEC_Stop(pAvplay->hAdec, reset);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_ADEC_Stop failed.\n");
        return Ret;
    }

	Ret = MT_MPI_AO_Track_Stop(0);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_AO_Track_Stop failed.\n");
        return Ret;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        #ifdef CONFIG_MT_AUDIO_AD
        if(MT_INVALID_HANDLE!=pAvplay->hDmxAud_AD){
            //printf("+++close ad chanenl begin\n");
            MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
            //printf("+++close ad chanenl over\n");
            if((HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_MP2==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_MP3==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_AAC==pAvplay->adectype_AD)){
				ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
			}
        }
        #endif
        for(i = 0; i < pAvplay->DmxAudChnNum; i++)
        {
            Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud[i]);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_DMX_CloseChannel failed.\n");
                return Ret;
            }
        }

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
		if (pAvplay->AudPtsDesc[0] != NULL)
		{
			mpi_demux_desc_destroy(pAvplay->AudPtsDesc[0]);
			pAvplay->AudPtsDesc[0] = NULL;
		}
		if (pAvplay->AudPtsDesc[1] != NULL)
		{
			mpi_demux_desc_destroy(pAvplay->AudPtsDesc[1]);
			pAvplay->AudPtsDesc[1] = NULL;
		}
#endif
    }


    MT_MPI_SYNC_Aud_DeInit(pAvplay->hSync);
    Ret = MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Stop Aud failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_ResetAudDemux(mt_handle hAvplay);
mt_s32 AVPLAY_ResetAudDemux(AVPLAY_S *pAvplay);

mt_s32 AVPLAY_ResetAudDemux(AVPLAY_S *pAvplay)
{
    mt_s32         Ret;
    mt_u32         i;

	pAvplay->enable_sw_adec = 0;

	if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
	{
    #ifdef CONFIG_MT_AUDIO_AD
		if(MT_INVALID_HANDLE!=pAvplay->hDmxAud_AD){
			//printf("+++close ad chanenl begin\n");
			MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud_AD);
			//printf("+++close ad chanenl over\n");
			if((HA_AUDIO_ID_DOLBY_PLUS==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_DOLBY_TRUEHD==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_MP2==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_MP3==pAvplay->adectype_AD)||
				 (HA_AUDIO_ID_AAC==pAvplay->adectype_AD)){
				ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
			}
		}
    #endif
		for(i = 0; i < pAvplay->DmxAudChnNum; i++)
		{
			Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud[i]);
			if (Ret != MT_SUCCESS)
			{
				MT_ERR_AVPLAY("call MT_MPI_DMX_CloseChannel failed.\n");
				return Ret;
			}
		}
	}

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
      MT_INFO_AVPLAY("\n\n\n  AVPLAY_StartAudChn22222    pAvplay->DmxAudChnNum %d \n\n\n", pAvplay->DmxAudChnNum);

        {
		#ifdef CONFIG_MT_AUDIO_AD
            //MT_UNF_SND_GAIN_ATTR_S gain={MT_TRUE,(mt_s32)0};
			ulong kerVirEsBuffAddr = 0;
            //printf("+++start ad chanenl begin\n");
            if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD){
                Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxAud_AD);
                if(MT_SUCCESS == Ret){
                    //printf("\n\n\n\n\n MT_MPI_DMX_OpenChannel ad good,break   \n\n\n\n\n", i);
                    memset(&pAvplay->DmxChanInfoAud_AD, 0, sizeof(DMX_CHAN_INFO_S));
                    MT_MPI_DMX_GetEsBuffAddr(pAvplay->hDmxAud_AD,
                        &(pAvplay->DmxChanInfoAud_AD.dmx_buf_addr),
                        &(pAvplay->DmxChanInfoAud_AD.dmx_buf_len),
                        &kerVirEsBuffAddr,
                        NULL, NULL);
                }
                //gain.s32Gain=50;
			#if 0	// ahren comment
                ret=MT_MPI_AO_SND_SetVolume(MT_UNF_SND_0,MT_UNF_SND_OUTPUTPORT_AD,&gain);
			#endif
                //printf("+++start ad chanenl over %x %x\n",MT_UNF_SND_OUTPUTPORT_AD,ret);
            }else{  //close ad volume
                ADEC_AD_SET_DATATYPE(pAvplay->hAdec, 0);
                //printf("+++type3.set 0\n");
                //gain.s32Gain=0;
			#if 0	// ahren comment
                ret=MT_MPI_AO_SND_SetVolume(MT_UNF_SND_0,MT_UNF_SND_OUTPUTPORT_AD,&gain);
			#endif
                //printf("+++start no ad chanenl over %x %x\n",MT_UNF_SND_OUTPUTPORT_AD,ret);
            }
		#endif
        }
        for(i= 0; i < pAvplay->DmxAudChnNum; i++)
        {
            ulong kerVirEsBuffAddr = 0;
            mt_u32 esBuffSize = 0;
            Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxAud[i]);
            if(MT_SUCCESS != Ret)
            {
                MT_INFO_AVPLAY("\n\n\n\n\n MT_MPI_DMX_OpenChannel %d error,break   \n\n\n\n\n", i);
                break;
            }

            MT_INFO_AVPLAY("\n\n\n\n\n\n  AVPLAY_StartAudChn22222  before MT_MPI_DMX_GetEsBuffAddr\n\n\n\n\n\n\n");

			//FIX Bug: dmx_buf_read_pnt be memset to zero only at AVPlay Create,
			//         but if APP do not Destroy/Create AVPlay, but Stop/Start,
			//		   the 'dmx_buf_read_pnt' is not correct.
			//pAvplay->DmxChanInfoAud[i].dmx_buf_read_pnt = 0;
			memset(&pAvplay->DmxChanInfoAud[i], 0, sizeof(DMX_CHAN_INFO_S));

            //cooperadd  get dmx addr and size
                MT_MPI_DMX_GetEsBuffAddr(pAvplay->hDmxAud[i],
                    &(pAvplay->DmxChanInfoAud[i].dmx_buf_addr),
                    &(pAvplay->DmxChanInfoAud[i].dmx_buf_len),
                    &kerVirEsBuffAddr,
                    NULL, NULL);

				MT_MPI_DMX_GetEsBuffPhyAddr(pAvplay->hDmxAud[i],
					&(pAvplay->DmxChanInfoAud[i].dmx_buf_phy_addr),&esBuffSize);

                MT_INFO_AVPLAY("\n\n\n\n\n\n\n\n AVPLAY_StartAudChn333 get es buf addr 0x%x len 0x%x\n\n\n\n\n\n\n\n\n",
                  pAvplay->DmxChanInfoAud[i].dmx_buf_addr, pAvplay->DmxChanInfoAud[i].dmx_buf_len);

        }

        if(i < pAvplay->DmxAudChnNum)
        {
			int j = 0;
		#ifdef CONFIG_MT_AUDIO_AD
            if(MT_INVALID_HANDLE != pAvplay->hDmxAud_AD){
                MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud_AD);
                pAvplay->hDmxAud_AD = MT_INVALID_HANDLE;
            }
		#endif
            for(j = 0; j < i; j++)
            {
                (mt_void)MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud[j]);
            }

            MT_ERR_AVPLAY("call MT_MPI_DMX_OpenChannel failed.\n");
            (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);
            return Ret;
        }
    }
	return 0;
}

mt_s32 MT_MPI_ResetAudDemux(mt_handle hAvplay)
{
    AVPLAY_S                   *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S          AvplayUsrAddr;
    mt_s32                     Ret;
    AVPLAY_GET_INST_AND_LOCK();
	Ret = AVPLAY_ResetAudDemux(pAvplay);

	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}
mt_s32 MT_MPI_AVPLAY_StopAudDec(mt_handle hAvplay)
{
    AVPLAY_S   *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32     Ret;

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->AudEnable)
    {
        Ret = MT_MPI_ADEC_Stop(pAvplay->hAdec, 0);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("ADEC Stop failed\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pAvplay->AudEnable = MT_FALSE;
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }
    else
    {
        MT_ERR_AVPLAY("ADEC Not Start\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_FAILURE;
    }
}

mt_void AVPLAY_PrePlay(AVPLAY_S *pAvplay)
{
    if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PREPLAY)
    {
        pAvplay->LstStatus = pAvplay->CurStatus;
        pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_PREPLAY;
    }
    return;
}

mt_void AVPLAY_Play(AVPLAY_S *pAvplay)
{
    if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_PLAY)
    {
        pAvplay->AvplayDataPushPause = MT_FALSE;
        pAvplay->LstStatus = pAvplay->CurStatus;
        pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_PLAY;
		MT_MPI_ADEC_Set_Pause(pAvplay->hAdec, 0);
    }

    return;
}

mt_void AVPLAY_Stop(AVPLAY_S *pAvplay)
{
    if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_STOP)
    {
        pAvplay->LstStatus = pAvplay->CurStatus;
        pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_STOP;
    }

    /* may be only stop vidchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
    if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
    {
        /*Release vpss frame*/
        (mt_void)AVPLAY_RelAllChnFrame(pAvplay);
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
    }

    AVPLAY_ResetProcFlag(pAvplay);
    return;
}

mt_void AVPLAY_Pause(AVPLAY_S *pAvplay)
{
    pAvplay->LstStatus = pAvplay->CurStatus;
    pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_PAUSE;
	MT_MPI_ADEC_Set_Pause(pAvplay->hAdec, 1);
    return;
}

mt_void AVPLAY_Freeze(AVPLAY_S *pAvplay)
{
    pAvplay->LstStatus = pAvplay->CurStatus;
    pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_FREEZE;

    return;
}

mt_void AVPLAY_Tplay(AVPLAY_S *pAvplay)
{
    pAvplay->AvplayDataPushPause = MT_FALSE;
    pAvplay->LstStatus = pAvplay->CurStatus;
    pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_TPLAY;

    return;
}

mt_s32 AVPLAY_ResetAudChn(AVPLAY_S *pAvplay)
{
    mt_s32  Ret;

    if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StopAudChn(pAvplay, 1);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("stop aud chn failed.\n");
            return Ret;
        }
    }

    if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StartAudChn(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("start aud chn failed.\n");
            return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_Seek(AVPLAY_S *pAvplay, mt_u64 u64SeekPts)
{
    mt_s32 Ret = MT_SUCCESS;
    MT_UNF_SYNC_STATUS_S SyncStatus;
    mt_u64 u64FindObjectPts = u64SeekPts;  //u32FindObjectPts  as  input param and output param

    MT_INFO_AVPLAY("seekpts is %d\n", u64SeekPts);

    if (pAvplay->AudEnable)
    {
        Ret = MT_MPI_SYNC_GetStatus(pAvplay->hSync, &SyncStatus);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_GetStatus failed.\n");
            return MT_FAILURE;
        }
        else
        {
            if (u64SeekPts < SyncStatus.u64LastAudPts)
            {
                MT_INFO_AVPLAY("find pts in ao buf ok quit\n");
                return MT_SUCCESS;
            }
        }
    }

    //1. pause vid, aud only 800ms, don't need pause
    Ret = MT_MPI_SYNC_Pause(pAvplay->hSync);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Pause failed, Ret=0x%x.\n", Ret);
        return MT_FAILURE;
    }

    //2. discard stream
    if (pAvplay->VidEnable)
    {
        Ret = MT_MPI_VDEC_ChanDropStream(pAvplay->hVdec, &u64FindObjectPts, AVPLAY_VDEC_SEEKPTS_THRESHOLD);
        if (Ret != MT_SUCCESS)
        {
            MT_INFO_AVPLAY("call MT_MPI_VDEC_ChanDropStream NO FIND SEEKPTS. \n");
            MT_INFO_AVPLAY("return vid pts is %d\n", u64FindObjectPts);
            return MT_FAILURE;
        }
        else
        {
            MT_INFO_AVPLAY("call MT_MPI_VDEC_ChanDropStream FIND SEEKPTS OK.\n");
            MT_INFO_AVPLAY("return vid pts is %d\n", u64FindObjectPts);

            (mt_void)AVPLAY_ResetWindow(pAvplay, MT_DRV_WIN_SWITCH_LAST);

            MT_INFO_AVPLAY("reset window\n");

            if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
            {
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
            }
        }
    }

    if (pAvplay->AudEnable)
    {
        u64SeekPts = (u64FindObjectPts > u64SeekPts) ? u64FindObjectPts : u64SeekPts;
        Ret = MT_MPI_ADEC_DropStream(pAvplay->hAdec, u64SeekPts);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("no find aud pts quit\n");
            return MT_FAILURE;
        }
        else
        {
            MT_INFO_AVPLAY("find aud pts ok\n");
            (mt_void)MT_MPI_AO_Track_Flush((mt_handle)NULL);
            MT_INFO_AVPLAY("reset ao\n");

            MT_INFO_AVPLAY("set AVPLAY_PROC_ADEC_AO false\n");

            if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO])
            {
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
            }
            if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2])
            {
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
            }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3])
            {
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
            }
#endif
        }
    }

    if ((pAvplay->AudEnable) && (pAvplay->VidEnable))
    {
        Ret  = MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_AUD);
        Ret |= MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);
        Ret |= MT_MPI_SYNC_Seek(pAvplay->hSync, u64SeekPts);
        Ret |= MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_AUD);
        Ret |= MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_VID);
    }

    Ret |= MT_MPI_SYNC_Play(pAvplay->hSync);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed, Ret=0x%x.\n", Ret);
    }

    return Ret;
}

mt_s32 AVPLAY_Reset(AVPLAY_S *pAvplay)
{
    mt_s32  Ret;
    mt_u32  enable_sw_adec = 0;

    if (pAvplay->VidEnable)
    {
        Ret = AVPLAY_StopVidChn(pAvplay, MT_UNF_AVPLAY_STOP_MODE_STILL);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("stop vid chn failed.\n");
            return Ret;
        }

        /* may be only stop vidchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
        if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
        {
            /*Release vpss frame*/
            (mt_void)AVPLAY_RelAllChnFrame(pAvplay);
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
        }
    }

    if (pAvplay->AudEnable)
    {
        enable_sw_adec = pAvplay->enable_sw_adec;
        Ret = AVPLAY_StopAudChn(pAvplay, 1);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("stop aud chn failed.\n");
            //return Ret;
        }
       (mt_void)MT_MPI_AO_Track_Flush((mt_handle)NULL);
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, 0x1fff);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
            return Ret;
        }
    }

    if (pAvplay->VidEnable)
    {
        Ret = AVPLAY_StartVidChn(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("start vid chn failed.\n");
            return Ret;
        }
    }

    //Ret = MT_MPI_SYNC_Aud_Init(pAvplay->hSync, 3);

    if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StartAudChn(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("start aud chn failed.\n");
            return Ret;
        }

        pAvplay->enable_sw_adec = enable_sw_adec;
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, pAvplay->DmxPcrPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
            return Ret;
        }
    }

    if (MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus)
    {

        Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
    {

        Ret = MT_MPI_SYNC_Tplay(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
    {

        /*Current status change to Last status while call Reset after EOS, scene: free TS->scramble TS->free TS*/
        pAvplay->CurStatus = pAvplay->LstStatus;

        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->LstStatus)
        {
            Ret = MT_MPI_SYNC_Tplay(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
            }
        }
        else
        {
            Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
            }
        }
    }
    else
    {
        Ret = MT_MPI_SYNC_Pause(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Pause failed.\n");
        }
    }

    AVPLAY_ResetProcFlag(pAvplay);

    return MT_SUCCESS;
}

/* flush audio/video input/output buffers while still playing */
mt_s32 AVPLAY_Flush(AVPLAY_S *pAvplay)
{
    mt_s32  Ret;
    //MT_DRV_WIN_SWITCH_E     SwitchType = MT_DRV_WIN_SWITCH_BUTT;

    if (pAvplay->VidEnable)
    {
		{
//clean all win frame first!
#if 0
		    if (MT_UNF_AVPLAY_STOP_MODE_STILL == enMode)
		    {
		        SwitchType = MT_DRV_WIN_SWITCH_LAST;
		    }
		    else
		    {
		        SwitchType = MT_DRV_WIN_SWITCH_BLACK;
		    }

			//FIXME: issue - no display after reset!
		    Ret = AVPLAY_ResetWindow(pAvplay, SwitchType);
#else
			//20180126: it's better to flush window after VDEC reset
			//Ret = AVPLAY_FlushWindow(pAvplay);
#endif
            MT_CODEC_RESETPARAM_S param;

            memset(&param, 0, sizeof(MT_CODEC_RESETPARAM_S));
            param.resetDQ = MT_TRUE;
		    Ret = MT_MPI_VDEC_ResetChan(pAvplay->hVdec, &param);
		    if (Ret != MT_SUCCESS)
		    {
		        MT_ERR_AVPLAY("call MT_MPI_VDEC_ResetChan failed.\n");
		        return Ret;
		    }

		    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
		    {
		        Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxVid);
		        if (Ret != MT_SUCCESS)
		        {
		            MT_ERR_AVPLAY("call MT_MPI_DMX_CloseChannel failed.\n");
		            return Ret;
		        }
		    }

		    Ret = MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);
		    if (Ret != MT_SUCCESS)
		    {
		        MT_ERR_AVPLAY("call MT_MPI_SYNC_Stop Vid failed.\n");
		        return Ret;
		    }

			//20180126: it's better to flush window after VDEC reset
		    Ret = AVPLAY_FlushWindow(pAvplay, MT_DRV_WIN_FLUSH_FIFO_QUEUE);
		}

        /* may be only stop vidchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
        if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
        {
            /*Release vpss frame*/
            (mt_void)AVPLAY_RelAllChnFrame(pAvplay);
            pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
        }
    }

    if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StopAudChn(pAvplay, 1);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("stop aud chn failed.\n");
            return Ret;
        }
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, 0x1fff);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
            return Ret;
        }
    }

    if (pAvplay->VidEnable)
    {
		{
		    Ret = MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_VID);
		    if (Ret != MT_SUCCESS)
		    {
		        MT_ERR_AVPLAY("call MT_MPI_SYNC_Start Vid failed.\n");
		        return Ret;
		    }

		    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
		    {
		        Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxVid);
		        if (Ret != MT_SUCCESS)
		        {
		            MT_ERR_AVPLAY("call MT_MPI_DMX_OpenChannel failed, Ret=%x.\n", Ret);
		            (mt_void)MT_MPI_SYNC_Stop(pAvplay->hSync, SYNC_CHAN_VID);
		            return Ret;
		        }
		    }

			if (pAvplay->VdecAttr.enUnBlank != MT_UNF_VCODEC_UNBLANK_USER)
			{
				//could control by application
			    MT_MPI_DISP_VidLayerShow(MT_TRUE);
			}
		}
	}

#if 1
    if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StartAudChn(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("start aud chn failed.\n");
            return Ret;
        }
    }
#endif

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, pAvplay->DmxPcrPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
            return Ret;
        }
    }

    if (MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus)
    {
        Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus)
    {
        Ret = MT_MPI_SYNC_Tplay(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
    {

        /*Current status change to Last status while call Reset after EOS, scene: free TS->scramble TS->free TS*/
        pAvplay->CurStatus = pAvplay->LstStatus;

        if (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->LstStatus)
        {
            Ret = MT_MPI_SYNC_Tplay(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
            }
        }
        else
        {
            Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play failed.\n");
            }
        }
    }
    else
    {
        Ret = MT_MPI_SYNC_Pause(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Pause failed.\n");
        }
    }

    AVPLAY_ResetProcFlag(pAvplay);

    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetNum(mt_u32 *pAvplayNum)
{
    /* get the number of avplay created by this process */
    return ioctl(g_AvplayDevFd, CMD_AVPLAY_CHECK_NUM, pAvplayNum);
}


mt_s32 AVPLAY_SetMultiAud(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MULTIAUD_ATTR_S *pAttr)
{
    mt_s32                      Ret;
    MT_UNF_DMX_CHAN_ATTR_S      DmxChnAttr;
    mt_u32                      i, j;

    if(MT_NULL == pAttr || MT_NULL == pAttr->pu32AudPid || MT_NULL == pAttr->pstAcodecAttr)
    {
        MT_ERR_AVPLAY("multi aud attr is null!\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if(pAttr->u32PidNum > AVPLAY_MAX_DMX_AUD_CHAN_NUM)
    {
        MT_ERR_AVPLAY("pidnum is too large\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if(pAttr->u32AudStartIdx >= pAttr->u32PidNum)
    {
        MT_ERR_AVPLAY("audio start idx=%d should less than pidnum=%d.\n",pAttr->u32AudStartIdx,pAttr->u32PidNum);
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (pAvplay->AudEnable)
    {
        MT_ERR_AVPLAY("aud chn is running, can not set aud pid.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (MT_INVALID_HANDLE == pAvplay->hAdec)
    {
        MT_ERR_AVPLAY("aud chn is close, can not set aud pid.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    memset(&DmxChnAttr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    DmxChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    DmxChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_AUD;
    //DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize / 3;
    if(0 != pAvplay->AvplayAttr.stStreamAttr.u32MultiAudBufSize)
    {
    	DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32MultiAudBufSize ;
    }
	else
	{
    	DmxChnAttr.u32BufSize = pAvplay->AvplayAttr.stStreamAttr.u32AudBufSize ;
	}

    /* destroy the old resource */
    for (i = 1; i < pAvplay->DmxAudChnNum; i++)
    {
        (mt_void)MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud[i]);
    }

    if (MT_NULL != pAvplay->pstAcodecAttr)
    {
        mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pstAcodecAttr));
        pAvplay->pstAcodecAttr = MT_NULL;
    }

    /* create new resource */
    for (i = 1; i < pAttr->u32PidNum; i++)
    {
        Ret = MT_MPI_DMX_CreateChannel(pAvplay->AvplayAttr.u32DemuxId, &DmxChnAttr, &(pAvplay->hDmxAud[i]));
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_CreateChannel failed.\n");
            break;
        }

		DmxChnAttr.AVSyncFlag = 1;
	    Ret = MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxAud[i], &DmxChnAttr);
		if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelAttr failed.\n");
            break;
        }
    }

    if(i != pAttr->u32PidNum)
    {
        for(j = 1; j < i; j++)
        {
            (mt_void)MT_MPI_DMX_DestroyChannel(pAvplay->hDmxAud[j]);
        }

        return MT_FAILURE;
    }

    for(i = 0; i < pAttr->u32PidNum; i++)
    {
        Ret = MT_MPI_DMX_SetChannelPID(pAvplay->hDmxAud[i], *(pAttr->pu32AudPid + i));
        if(MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_SetChannelPID failed.\n");
            return Ret;
        }
        else
        {
            pAvplay->DmxAudPid[i] = *(pAttr->pu32AudPid + i);
        }
    }

    pAvplay->DmxAudChnNum = pAttr->u32PidNum;

    pAvplay->pstAcodecAttr = (MT_UNF_ACODEC_ATTR_S *)mt_malloc(MT_ID_AVPLAY, sizeof(MT_UNF_ACODEC_ATTR_S) * pAttr->u32PidNum);
    if (MT_NULL == pAvplay->pstAcodecAttr)
    {
        MT_ERR_AVPLAY("malloc pstAcodecAttr error.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    memcpy(pAvplay->pstAcodecAttr, pAttr->pstAcodecAttr, sizeof(MT_UNF_ACODEC_ATTR_S)*pAttr->u32PidNum);

    /*for select which audio track start play*/
    pAvplay->CurDmxAudChn = pAttr->u32AudStartIdx;
    MT_MPI_DMX_DataPushStart(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]); // call this api for config main_select_index

    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetMultiAud(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_MULTIAUD_ATTR_S *pAttr)
{
    if (MT_NULL == pAttr || MT_NULL == pAttr->pu32AudPid || MT_NULL == pAttr->pstAcodecAttr)
    {
        MT_ERR_AVPLAY("ERR: invalid para\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    /* only get the real pid num */
    if (pAttr->u32PidNum > pAvplay->DmxAudChnNum)
    {
        pAttr->u32PidNum = pAvplay->DmxAudChnNum;
    }

    if (pAttr->u32PidNum > AVPLAY_MAX_DMX_AUD_CHAN_NUM)
    {
        MT_ERR_AVPLAY("u32PidNum is larger than %d\n", AVPLAY_MAX_DMX_AUD_CHAN_NUM);
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    memcpy(pAttr->pu32AudPid, pAvplay->DmxAudPid, sizeof(mt_u32) * pAttr->u32PidNum);

    memcpy(pAttr->pstAcodecAttr, pAvplay->pstAcodecAttr, sizeof(MT_UNF_ACODEC_ATTR_S) * pAttr->u32PidNum);

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetEosFlag(AVPLAY_S *pAvplay)
{
    mt_s32          Ret;

    if (!pAvplay->AudEnable && !pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("ERR: vid and aud both disable, can not set eos!\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (pAvplay->AudEnable)
    {
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_DMX_SetChannelEosFlag(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("ERR: MT_MPI_DMX_SetChannelEosFlag, Ret = %x! \n", Ret);
                return MT_ERR_AVPLAY_INVALID_OPT;
            }
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_ADEC_SetEosFlag(pAvplay->hAdec);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("ERR: MT_MPI_ADEC_SetEosFlag, Ret = %x! \n", Ret);
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            if (MT_INVALID_HANDLE != pAvplay->hSyncTrack)
            {
                Ret = MT_MPI_AO_Track_SetEosFlag(pAvplay->hSyncTrack, MT_TRUE);
                if (MT_SUCCESS != Ret)
                {
                    MT_ERR_AVPLAY("ERR: MT_MPI_HIAO_SetEosFlag, Ret = %x! \n", Ret);
                    return MT_ERR_AVPLAY_INVALID_OPT;
                }
            }
        }
    }

    if (pAvplay->VidEnable)
    {
        Ret = MT_MPI_VDEC_SetEosFlag(pAvplay->hVdec);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_SetEosFlag, Ret = %x! \n", Ret);
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_DMX_SetChannelEosFlag(pAvplay->hDmxVid);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("ERR: MT_MPI_DMX_SetChannelEosFlag, Ret = %x! \n", Ret);
                return MT_ERR_AVPLAY_INVALID_OPT;
            }
        }
    }

    pAvplay->bSetEosFlag = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetPortAttr(AVPLAY_S *pAvplay, mt_handle hPort, VDEC_PORT_TYPE_E enType)
{
    mt_s32                      Ret;

    Ret = MT_MPI_VDEC_SetPortType(pAvplay->hVdec, hPort, enType);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_SetPortType, Ret=%x.\n", Ret);
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_EnablePort(pAvplay->hVdec, hPort);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_EnablePort, Ret=%x.\n", Ret);
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    return MT_SUCCESS;
}


mt_s32 AVPLAY_CreatePort(AVPLAY_S *pAvplay, mt_handle hWin, VDEC_PORT_ABILITY_E enAbility, mt_handle *phPort)
{
    mt_s32                      Ret;
    VDEC_PORT_PARAM_S           stPortPara;
    MT_DRV_WIN_SRC_INFO_S       stSrcInfo;
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	mt_sys_mem_config_s 		stMemConfig;
#endif
    memset(&stSrcInfo, 0x0, sizeof(MT_DRV_WIN_SRC_INFO_S));

    Ret = MT_MPI_VDEC_CreatePort(pAvplay->hVdec, phPort, enAbility);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_CreatePort.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetPortParam(pAvplay->hVdec, *phPort, &stPortPara);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_GetPortParam.\n");
        (mt_void)MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, *phPort);
        *phPort = MT_INVALID_HANDLE;
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    stSrcInfo.hSrc = *phPort;
    stSrcInfo.pfAcqFrame = (PFN_GET_FRAME_CALLBACK)MT_NULL;
    stSrcInfo.pfRlsFrame = (PFN_PUT_FRAME_CALLBACK)stPortPara.pfVORlsFrame;
    stSrcInfo.pfSendWinInfo = (PFN_GET_WIN_INFO_CALLBACK)stPortPara.pfVOSendWinInfo;

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	//Extend Freeze interfaces
	Ret = mt_sys_get_mem_config(&stMemConfig);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: mt_sys_get_mem_config.\n");
        (mt_void)MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, *phPort);
        *phPort = MT_INVALID_HANDLE;
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
	MT_INFO_AVPLAY("\n%s: total mem size=%u(MB)\n",__FUNCTION__,stMemConfig.u32TotalSize);

	if (stMemConfig.u32TotalSize > 128)
	{
		//>=256M
		//freeze buffer: not shared with VFMW
	    stSrcInfo.pfAcqFreezeFrame = (PFN_GET_FRAME_CALLBACK)MT_NULL;
	    stSrcInfo.pfRlsFreezeFrame = (PFN_PUT_FRAME_CALLBACK)MT_NULL;
	}
	else
	{
		//<=128M
		//freeze buffer: shared with VFMW
	    stSrcInfo.pfAcqFreezeFrame = (PFN_GET_FRAME_CALLBACK)stPortPara.pfAcqFreezeFrame;
	    stSrcInfo.pfRlsFreezeFrame = (PFN_PUT_FRAME_CALLBACK)stPortPara.pfRlsFreezeFrame;
	}
#endif

    Ret = MT_MPI_WIN_SetSource(hWin, &stSrcInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_WIN_SetSource.\n");
        (mt_void)MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, *phPort);
        *phPort = MT_INVALID_HANDLE;
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_DestroyPort(AVPLAY_S *pAvplay, mt_handle hWin, mt_handle hPort)
{
    mt_s32                      Ret = MT_SUCCESS;
    MT_DRV_WIN_SRC_INFO_S       stSrcInfo;

    memset(&stSrcInfo, 0x0, sizeof(MT_DRV_WIN_SRC_INFO_S));

    stSrcInfo.hSrc = MT_INVALID_HANDLE;
    stSrcInfo.pfAcqFrame = (PFN_GET_FRAME_CALLBACK)MT_NULL;
    stSrcInfo.pfRlsFrame = (PFN_PUT_FRAME_CALLBACK)MT_NULL;
    stSrcInfo.pfAcqFreezeFrame = (PFN_GET_FRAME_CALLBACK)MT_NULL;
    stSrcInfo.pfRlsFreezeFrame = (PFN_PUT_FRAME_CALLBACK)MT_NULL;
    stSrcInfo.pfSendWinInfo = (PFN_GET_WIN_INFO_CALLBACK)MT_NULL;

    Ret = MT_MPI_WIN_SetSource(hWin, &stSrcInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_WIN_SetSource.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, hPort);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VDEC_DestroyPort.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Init(mt_void)
{
    mt_s32 Ret;

    MT_AVPLAY_LOCK();

    // already opened in this process
    if (g_AvplayDevFd > 0)
    {
        MT_AVPLAY_UNLOCK();

        return MT_SUCCESS;
    }

    g_AvplayDevFd = open(g_AvplayDevName, O_RDWR | O_NONBLOCK | O_CLOEXEC, 0);

    if (g_AvplayDevFd < 0)
    {
        MT_AVPLAY_UNLOCK();

        MT_FATAL_AVPLAY("open %s error\n", g_AvplayDevName);

        return MT_ERR_AVPLAY_DEV_OPEN_ERR;
    }

    MT_AVPLAY_UNLOCK();

    Ret = MT_MPI_ADEC_Init();
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_ADEC_Init failed 0x%x\n", Ret);
        goto AVPLAY_CLOSE;
    }
    Ret = MT_MPI_VDEC_Init();
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_Init failed 0x%x\n", Ret);
        goto ADEC_DEINIT;
    }

    Ret = MT_MPI_SYNC_Init();
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AVPLAY("MT_MPI_SYNC_Init failed 0x%x\n", Ret);
        goto VDEC_DEINIT;
    }

    return MT_SUCCESS;

VDEC_DEINIT:
    MT_MPI_VDEC_DeInit();

ADEC_DEINIT:
    MT_MPI_ADEC_deInit();
AVPLAY_CLOSE:
    MT_AVPLAY_LOCK();
    close(g_AvplayDevFd);
    g_AvplayDevFd = -1;
    MT_AVPLAY_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_DeInit(mt_void)
{
    mt_s32  Ret;
    mt_u32  AvplayNum = 0;

    MT_AVPLAY_LOCK();

    if (g_AvplayDevFd < 0)
    {
        MT_AVPLAY_UNLOCK();
        return MT_SUCCESS;
    }

    Ret = AVPLAY_GetNum(&AvplayNum);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("call AVPLAY_GetNum failed.\n");
        MT_AVPLAY_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (AvplayNum)
    {
        MT_ERR_AVPLAY("there are %d AVPLAY not been destroied.\n", AvplayNum);
        MT_AVPLAY_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_SYNC_DeInit();
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AVPLAY("MT_MPI_SYNC_DeInit failed 0x%x\n", Ret);
    }

    Ret = MT_MPI_VDEC_DeInit();
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_DeInit failed 0x%x\n", Ret);
    }

    Ret = MT_MPI_ADEC_deInit();
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_ADEC_deInit failed 0x%x\n", Ret);
    }

    Ret = close(g_AvplayDevFd);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AVPLAY("DeInit AVPLAY err.\n");
        MT_AVPLAY_UNLOCK();
        return MT_ERR_AVPLAY_DEV_CLOSE_ERR;
    }

    g_AvplayDevFd = -1;

    MT_AVPLAY_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetDefaultConfig(MT_UNF_AVPLAY_ATTR_S *pstAvAttr, MT_UNF_AVPLAY_STREAM_TYPE_E enCfg)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	mt_sys_mem_config_s stMemConfig;
	int Ret;
#endif

    if (!pstAvAttr)
    {
        MT_ERR_AVPLAY("para pstAvAttr is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    if ((MT_UNF_AVPLAY_STREAM_TYPE_TS != enCfg) && (MT_UNF_AVPLAY_STREAM_TYPE_ES != enCfg))
    {
        MT_ERR_AVPLAY("para enCfg is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    pstAvAttr->u32DemuxId = 0;
    pstAvAttr->stStreamAttr.enStreamType = enCfg;
#if defined(CONFIG_MT_CHIP_ARIA)
    pstAvAttr->stStreamAttr.u32VidBufSize = 16 * 1024 * 1024;
    pstAvAttr->stStreamAttr.u32AudBufSize = 1024 * 1024;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	Ret = mt_sys_get_mem_config(&stMemConfig);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: mt_sys_get_mem_config.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }
	MT_INFO_AVPLAY("%s: Total mem=%d(MB).\n",__FUNCTION__,stMemConfig.u32TotalSize);

//Memory is quite short!
#if 0
	if (stMemConfig.u32TotalSize > 128)
	{
    	pstAvAttr->stStreamAttr.u32VidBufSize = 8 * 1024 * 1024;
	}
	else	//<=128M
#endif
	{
#ifdef CONFIG_MT_VDEC_4K
    	pstAvAttr->stStreamAttr.u32VidBufSize = 16 * 1024 * 1024;
#else
		pstAvAttr->stStreamAttr.u32VidBufSize = 6 * 1024 * 1024;
#endif
	}

    pstAvAttr->stStreamAttr.u32AudBufSize = 384 * 1024;
    pstAvAttr->stStreamAttr.u32MultiAudBufSize = 192 * 1024;

   pstAvAttr->stStreamAttr.AudErrorCheckContinuationNum = 8;
#endif
	pstAvAttr->stStreamAttr.u32DebugAudCrcBufSize = 0;
	pstAvAttr->stStreamAttr.vdec_pip_chan = 0;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Create(const MT_UNF_AVPLAY_ATTR_S *pstAvAttr, mt_handle *phAvplay)
{
    AVPLAY_S               *pAvplay = MT_NULL;
    AVPLAY_CREATE_S        AvplayCreate;
    AVPLAY_USR_ADDR_S      AvplayUsrAddr;
    MT_UNF_SYNC_ATTR_S     SyncAttr;
    mt_u32                 i;
    mt_s32                 Ret = 0;
    //mt_sys_mem_config_s    stMemConfig = {0, 0};	//not used

    if (!pstAvAttr)
    {
        MT_ERR_AVPLAY("para pstAvAttr is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    if (!phAvplay)
    {
        MT_ERR_AVPLAY("para phAvplay is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    if (pstAvAttr->stStreamAttr.enStreamType >= MT_UNF_AVPLAY_STREAM_TYPE_BUTT)
    {
        MT_ERR_AVPLAY("para pstAvAttr->stStreamAttr.enStreamType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if ((pstAvAttr->stStreamAttr.u32VidBufSize > AVPLAY_MAX_VID_SIZE)
      ||(pstAvAttr->stStreamAttr.u32VidBufSize < AVPLAY_MIN_VID_SIZE)
       )
    {
        MT_ERR_AVPLAY("para pstAvAttr->stStreamAttr.u32VidBufSize is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if ((pstAvAttr->stStreamAttr.u32AudBufSize > AVPLAY_MAX_AUD_SIZE)
      ||(pstAvAttr->stStreamAttr.u32AudBufSize < AVPLAY_MIN_AUD_SIZE)
       )
    {
        MT_ERR_AVPLAY("para pstAvAttr->stStreamAttr.u32AudBufSize is invalid.   stStreamAttr.u32AudBufSize = %x \n", pstAvAttr->stStreamAttr.u32AudBufSize);
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    MT_AVPLAY_LOCK();

    if (g_AvplayDevFd < 0)
    {
        MT_ERR_AVPLAY("AVPLAY is not init.\n");
        MT_AVPLAY_UNLOCK();
        return MT_ERR_AVPLAY_DEV_NO_INIT;
    }

    MT_AVPLAY_UNLOCK();

    /* create avplay */
    AvplayCreate.AvplayStreamtype = pstAvAttr->stStreamAttr.enStreamType;
    Ret = ioctl(g_AvplayDevFd, CMD_AVPLAY_CREATE, &AvplayCreate);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY CMD_AVPLAY_CREATE failed.\n");
        goto RET;
    }

    /* remap the memories allocated in kernel space to user space */
    //Bug: 0x2000 < sizeof(AVPLAY_S)
    //pAvplay = (AVPLAY_S *)(mt_mmap(AvplayCreate.AvplayPhyAddr, 0x2000));
    pAvplay = (AVPLAY_S *)(mt_mmap(AvplayCreate.AvplayPhyAddr, sizeof(AVPLAY_S)));
    MT_INFO_AVPLAY("AvplayCreate.AvplayPhyAddr:0x%x, sizeof(AVPLAY_S):0x%x, sizeof(pthread_mutex_t):0x%x\n", AvplayCreate.AvplayPhyAddr, sizeof(AVPLAY_S), sizeof(pthread_mutex_t));
    if (!pAvplay)
    {
        MT_ERR_AVPLAY("AVPLAY memmap failed.\n");
        Ret = MT_ERR_AVPLAY_CREATE_ERR;
        goto AVPLAY_DESTROY;
    }

    pAvplay->pAvplayThreadMutex = (pthread_mutex_t *)mt_malloc(MT_ID_AVPLAY, sizeof(pthread_mutex_t));
    if (pAvplay->pAvplayThreadMutex == MT_NULL)
    {
        Ret = MT_ERR_AVPLAY_CREATE_ERR;
        goto AVPLAY_UNMAP;
    }
    (mt_void)pthread_mutex_init(pAvplay->pAvplayThreadMutex, NULL);


    pAvplay->m_vidsem = (sem_t *)mt_malloc(MT_ID_AVPLAY, sizeof(sem_t));
    if (pAvplay->m_vidsem == MT_NULL)
    {
        Ret = MT_ERR_AVPLAY_CREATE_ERR;
        goto AVPLAY_UNMAP;
    }
	if (sem_init(pAvplay->m_vidsem, 0, 1) == -1)
	{
		MT_ERR_AVPLAY("AVPLAY create m_vidsem failed.\n");
		goto AVPLAY_UNLOCK;
	}

#ifdef AVPLAY_VID_THREAD
    pAvplay->pAvplayVidThreadMutex = (pthread_mutex_t *)mt_malloc(MT_ID_AVPLAY, sizeof(pthread_mutex_t));
    if (pAvplay->pAvplayVidThreadMutex == MT_NULL)
    {
        Ret = MT_ERR_AVPLAY_CREATE_ERR;
        goto DESTROY_THREAD_MUTEX;
    }
    (mt_void)pthread_mutex_init(pAvplay->pAvplayVidThreadMutex, NULL);
#endif



    AvplayUsrAddr.AvplayId = AvplayCreate.AvplayId;
    AvplayUsrAddr.AvplayUsrAddr = (ulong)pAvplay;

    //printf("AvplayUsrAddr.AvplayUsrAddr = %x ROCK \n",  &AvplayUsrAddr);

    Ret = ioctl(g_AvplayDevFd, CMD_AVPLAY_SET_USRADDR, &AvplayUsrAddr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY set user addr failed.\n");
        goto DESTROY_THREAD_MUTEX;
    }

    AVPLAY_Mutex_Lock(&g_AvplayResMutex[AvplayCreate.AvplayId]);
    AVPLAY_Mutex_Lock(pAvplay->pAvplayThreadMutex);

    /* record stream attributes */
    memcpy(&pAvplay->AvplayAttr, pstAvAttr, sizeof(MT_UNF_AVPLAY_ATTR_S));

    if ((pAvplay->AvplayAttr.stStreamAttr.enStreamType == MT_UNF_AVPLAY_STREAM_TYPE_ES)
        && (pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize > 32*1024*1024)
        )
    {
        pAvplay->AvplayAttr.stStreamAttr.u32VidBufSize = 32*1024*1024;
    }


    /* initialize resource handle */
    pAvplay->hVdec = MT_INVALID_HANDLE;
    pAvplay->VdecAttr.enType = MT_UNF_VCODEC_TYPE_BUTT;
    pAvplay->VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
    pAvplay->VdecAttr.u32ErrCover = 0;
    pAvplay->VdecAttr.u32Priority = 0;
    pAvplay->hDmxVid = MT_INVALID_HANDLE;
    pAvplay->DmxVidPid = 0x1fff;
	pAvplay->trickmode = TM_NORMAL;

    pAvplay->hAdec = MT_INVALID_HANDLE;
    pAvplay->AdecType = 0xffffffff;
    #ifdef CONFIG_MT_AUDIO_AD
    pAvplay->hAdec_AD = MT_INVALID_HANDLE;
    pAvplay->hDmxAud_AD = MT_INVALID_HANDLE;
    #endif
    memset(&(pAvplay->AdecNameInfo), 0x0, sizeof(ADEC_SzNameINFO_S));

    for(i=0; i<AVPLAY_MAX_DMX_AUD_CHAN_NUM; i++)
    {
        pAvplay->hDmxAud[i] = MT_INVALID_HANDLE;
        pAvplay->DmxAudPid[i] = 0x1fff;
        memset(&pAvplay->DmxChanInfoAud[i], 0, sizeof(DMX_CHAN_INFO_S));
    }

    pAvplay->DmxAudChnNum = 0;
    pAvplay->CurDmxAudChn = 0;
    pAvplay->u32imgChangeFlag = 0;

    pAvplay->pstAcodecAttr = MT_NULL;

    pAvplay->hDmxPcr = MT_INVALID_HANDLE;
    pAvplay->DmxPcrPid = 0x1fff;

    pAvplay->bStepMode = MT_FALSE;
    pAvplay->bStepPlay = MT_FALSE;
    pAvplay->TargetTime  = MT_INVALID_TIME64;
    pAvplay->bTargetVideoRendered = MT_FALSE;
    pAvplay->bTargetAudioRendered = MT_FALSE;

    pAvplay->hSharedOrgWin = MT_INVALID_HANDLE;

    /*init window and port handle*/
    pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;
    pAvplay->MasterFrmChn.hWindow = MT_INVALID_HANDLE;

    for (i=0; i<AVPLAY_MAX_SLAVE_FRMCHAN; i++)
    {
        pAvplay->SlaveFrmChn[i].hPort = MT_INVALID_HANDLE;
        pAvplay->SlaveFrmChn[i].hWindow = MT_INVALID_HANDLE;
    }

    for (i=0; i<AVPLAY_MAX_VIR_FRMCHAN; i++)
    {
        pAvplay->VirFrmChn[i].hPort = MT_INVALID_HANDLE;
        pAvplay->VirFrmChn[i].hWindow = MT_INVALID_HANDLE;
    }

    pAvplay->SlaveChnNum = 0;
    pAvplay->VirChnNum = 0;

    pAvplay->LowDelayAttr.bEnable = MT_FALSE;

    Ret = AVPLAY_FrcCreate(pAvplay);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY create frc failed.\n");
        goto AVPLAY_UNLOCK;
    }

    //pAvplay->bFrcEnable = MT_TRUE;
    pAvplay->bFrcEnable = MT_FALSE;//yihua

    for (i=0; i<AVPLAY_MAX_TRACK; i++)
    {
        pAvplay->hTrack[i] = MT_INVALID_HANDLE;
    }

    pAvplay->AvplayVidEsBuf.u32PreFrameFinsh = 1;
    pAvplay->AvplayVidEsBuf.u32ScrapSize = 0;
    pAvplay->u32PreFrameFinsh = 1;
    pAvplay->u32ScrapSize = 0;

    pAvplay->TrackNum = 0;
    pAvplay->hSyncTrack = MT_INVALID_HANDLE;

    pAvplay->AudDDPMode = MT_FALSE; /* for DDP test only */
    pAvplay->LastAudPts = 0;        /* for DDP test only */

    pAvplay->VidEnable = MT_FALSE;
    pAvplay->AudEnable = MT_FALSE;
    pAvplay->bVidPreEnable = MT_FALSE;
    pAvplay->bAudPreEnable = MT_FALSE;

    pAvplay->LstStatus = MT_UNF_AVPLAY_STATUS_STOP;
    pAvplay->CurStatus = MT_UNF_AVPLAY_STATUS_STOP;
    pAvplay->OverflowProc = MT_UNF_AVPLAY_OVERFLOW_RESET;
    pAvplay->AvplayDataPushPause = MT_FALSE;
    pAvplay->AvplayAudFrm.u32chan = SND_ENGINE_TYPE_PCM;
    pAvplay->AvplayAudFrm2.u32chan = SND_ENGINE_TYPE_SPDIF_RAW;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    pAvplay->AvplayAudFrm3.u32chan = SND_ENGINE_TYPE_MIXBUF_DD;
#endif

    /* initialize related parameters of the avplay thread */
    pAvplay->AvplayThreadRun = MT_TRUE;

#ifdef CONFIG_MT_HIGH_PERF_AVPLAY
//#ifdef CONFIG_MT_DONT_ALLOW_RT
	//But: ADVCA not allow SCHED_RR!
    pAvplay->AvplayThreadPrio = THREAD_PRIO_HIGH;
//#else
//	pAvplay->AvplayThreadPrio = THREAD_PRIO_REALTIME;
//#endif
#else
	pAvplay->AvplayThreadPrio = THREAD_PRIO_MID;
#endif
    pAvplay->CurBufferEmptyState = MT_FALSE;

    AVPLAY_ResetProcFlag(pAvplay);

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	pAvplay->AudPtsDesc[0] = NULL;
	pAvplay->AudPtsDesc[1] = NULL;
#endif

	if (pAvplay->AvplayAttr.stStreamAttr.u32DebugAudCrcBufSize > 0)
	{
		pAvplay->DebugInfoExt.u32AudCrcBufSize =
			pAvplay->AvplayAttr.stStreamAttr.u32DebugAudCrcBufSize;
		pAvplay->DebugInfoExt.pu32AudCrcBuf =
			(ulong*)mt_malloc(MT_ID_AVPLAY, pAvplay->DebugInfoExt.u32AudCrcBufSize);
		if (pAvplay->DebugInfoExt.pu32AudCrcBuf == NULL)
		{
			pAvplay->DebugInfoExt.u32AudCrcBufSize = 0;
			MT_ERR_AVPLAY("AVPLAY allocate audio debug crc buffer failed.\n");
		}
		else
		{
			memset(pAvplay->DebugInfoExt.pu32AudCrcBuf, 0, pAvplay->DebugInfoExt.u32AudCrcBufSize);
		}
		pAvplay->DebugInfoExt.u32AudCrcBufWrPtr = 0;
		MT_INFO_AVPLAY("Audio PCM CRC Enabled! CRC Buffer Size: %u\n",pAvplay->DebugInfoExt.u32AudCrcBufSize);
	}
	else
	{
		pAvplay->DebugInfoExt.u32AudCrcBufSize = 0;
		pAvplay->DebugInfoExt.pu32AudCrcBuf = NULL;
		pAvplay->DebugInfoExt.u32AudCrcBufWrPtr = 0;
		MT_INFO_AVPLAY("Audio PCM CRC NOT Enabled!\n");
	}

    /* initialize events callback function*/
    for (i=0; i<MT_UNF_AVPLAY_EVENT_BUTT; i++)
    {
        pAvplay->EvtCbFunc[i] = MT_NULL;
    }

    MT_MPI_SYNC_GetDefaultAttr(&SyncAttr);

    Ret = MT_MPI_SYNC_Create(&SyncAttr, &pAvplay->hSync);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY create sync failed.\n");
        goto FRC_DESTROY;
    }

    /* create thread */
    Ret = AVPLAY_CreateThread(pAvplay);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY create thread failed:%x\n",Ret);
        goto SYNC_DESTROY;
    }

    pAvplay->hAvplay = (MT_ID_AVPLAY << 16) | AvplayCreate.AvplayId;

    *phAvplay = (MT_ID_AVPLAY << 16) | AvplayCreate.AvplayId;

    AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
    AVPLAY_Mutex_UnLock(&g_AvplayResMutex[AvplayCreate.AvplayId]);

    return     MT_SUCCESS;

SYNC_DESTROY:
    (mt_void)MT_MPI_SYNC_Destroy(pAvplay->hSync);

FRC_DESTROY:
    (mt_void)AVPLAY_FrcDestroy(pAvplay);
     pAvplay->bFrcEnable = MT_FALSE;

AVPLAY_UNLOCK:
    AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
    AVPLAY_Mutex_UnLock(&g_AvplayResMutex[AvplayCreate.AvplayId]);


DESTROY_THREAD_MUTEX:

	sem_destroy(pAvplay->m_vidsem);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->m_vidsem));

    (mt_void)pthread_mutex_destroy(pAvplay->pAvplayThreadMutex);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pAvplayThreadMutex));
#ifdef AVPLAY_VID_THREAD
    (mt_void)pthread_mutex_destroy(pAvplay->pAvplayVidThreadMutex);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pAvplayVidThreadMutex));
#endif

AVPLAY_UNMAP:
    (mt_void)mt_munmap(pAvplay);

AVPLAY_DESTROY:
    (mt_void)ioctl(g_AvplayDevFd, CMD_AVPLAY_DESTROY, &(AvplayCreate.AvplayId));

RET:
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Destroy(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32             Ret;

    AVPLAY_GET_INST_AND_LOCK();


    if ((pAvplay->hVdec != MT_INVALID_HANDLE)
      ||(pAvplay->hAdec != MT_INVALID_HANDLE)
       )
    {
        MT_ERR_AVPLAY("vid or aud chn is not closed.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if ((MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
        || (0 != pAvplay->SlaveChnNum) || (0 != pAvplay->VirChnNum)
        )
    {
        MT_ERR_AVPLAY("win is not detach.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* stop thread */
    pAvplay->AvplayThreadRun = MT_FALSE;
    (mt_void)pthread_join(pAvplay->AvplayDataThdInst, MT_NULL);
#ifdef AVPLAY_VID_THREAD
    (mt_void)pthread_join(pAvplay->AvplayVidDataThdInst, MT_NULL);
#endif
	if (sem_post(pAvplay->m_vidsem) == -1)
	{
		MT_ERR_AVPLAY("Error, sem_post pAvplay->m_vidsem shall not happen!\n");
	}
    (mt_void)pthread_join(pAvplay->AvplayStatThdInst, MT_NULL);
    pthread_attr_destroy(&pAvplay->AvplayThreadAttr);

    (mt_void)MT_MPI_SYNC_Destroy(pAvplay->hSync);

    (mt_void)AVPLAY_FrcDestroy(pAvplay);
    pAvplay->bFrcEnable = MT_FALSE;
	sem_destroy(pAvplay->m_vidsem);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->m_vidsem));

	if (pAvplay->DebugInfoExt.pu32AudCrcBuf != NULL)
	{
        mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->DebugInfoExt.pu32AudCrcBuf));
		pAvplay->DebugInfoExt.pu32AudCrcBuf = NULL;
	}

    (mt_void)pthread_mutex_destroy(pAvplay->pAvplayThreadMutex);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pAvplayThreadMutex));

#ifdef AVPLAY_VID_THREAD
    (mt_void)pthread_mutex_destroy(pAvplay->pAvplayVidThreadMutex);
    mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pAvplayVidThreadMutex));
#endif

    if (MT_NULL != pAvplay->pstAcodecAttr)
    {
        mt_free(MT_ID_AVPLAY, (mt_void*)(pAvplay->pstAcodecAttr));
        pAvplay->pstAcodecAttr = MT_NULL;
    }

    (mt_void)mt_munmap((mt_void *)AvplayUsrAddr.AvplayUsrAddr);

    Ret = ioctl(g_AvplayDevFd, CMD_AVPLAY_DESTROY, &AvplayUsrAddr.AvplayId);
    if (Ret != MT_SUCCESS)
    {
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS ;
}

mt_s32 MT_MPI_AVPLAY_ChnOpen(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const mt_void *pPara)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    MT_BOOL            AudDmxDecTogether = MT_FALSE;
    MT_BOOL            VidDmxDecTogether = MT_FALSE;
    mt_s32             Ret;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        VidDmxDecTogether = MT_TRUE;
    }

    if (VidDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDMX)))
    {
        if(MT_INVALID_HANDLE == pAvplay->hDmxVid)
        {
            Ret = AVPLAY_MallocVidChn(pAvplay, pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc Vdmx chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (VidDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDEC)))
    {
        if(MT_INVALID_HANDLE == pAvplay->hVdec)
        {
            Ret = AVPLAY_MallocVdec(pAvplay, pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc Vdec chn failed.\n");
                if(pAvplay->hDmxVid != MT_INVALID_HANDLE)
                    AVPLAY_FreeVidChn(pAvplay);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
    {
        AudDmxDecTogether = MT_TRUE;
    }

    if (AudDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_ADMX)))
    {
        if(MT_INVALID_HANDLE == pAvplay->hDmxAud[0])
        {
            Ret = AVPLAY_MallocAudChn(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc Admx chn failed.\n");
                if(pAvplay->hVdec != MT_INVALID_HANDLE)
                    AVPLAY_FreeVdec(pAvplay);
                if(pAvplay->hDmxVid != MT_INVALID_HANDLE)
                    AVPLAY_FreeVidChn(pAvplay);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (AudDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_ADEC)))
    {
        if(MT_INVALID_HANDLE == pAvplay->hAdec)
        {
            Ret = AVPLAY_MallocAdec(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc Adec chn failed.\n");
                if(pAvplay->hVdec != MT_INVALID_HANDLE)
                    AVPLAY_FreeVdec(pAvplay);
                if(pAvplay->hDmxVid != MT_INVALID_HANDLE)
                    AVPLAY_FreeVidChn(pAvplay);
                if(pAvplay->hDmxAud[0] != MT_INVALID_HANDLE)
                    AVPLAY_FreeAudChn(pAvplay);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        if (MT_INVALID_HANDLE == pAvplay->hDmxPcr)
        {
            Ret = MT_MPI_DMX_CreatePcrChannel(pAvplay->AvplayAttr.u32DemuxId, &pAvplay->hDmxPcr);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay malloc pcr chn failed.\n");
                if(pAvplay->hVdec != MT_INVALID_HANDLE)
                {
                    (mt_void)AVPLAY_FreeVdec(pAvplay);
                }

                if(pAvplay->hDmxVid != MT_INVALID_HANDLE)
                {
                    (mt_void)AVPLAY_FreeVidChn(pAvplay);
                }

                if(MT_INVALID_HANDLE != pAvplay->hAdec)
                {
                    (mt_void)AVPLAY_FreeAdec(pAvplay);
                }

                if(pAvplay->hDmxAud[0] != MT_INVALID_HANDLE)
                {
                    (mt_void)AVPLAY_FreeAudChn(pAvplay);
                }

                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
            (mt_void)MT_MPI_DMX_PcrSyncAttach(pAvplay->hDmxPcr,pAvplay->hSync);
        }
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_ChnClose(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    MT_BOOL            AudDmxDecTogether = MT_FALSE;
    MT_BOOL            VidDmxDecTogether = MT_FALSE;
    mt_s32             Ret;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD || enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if(enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID) ||
        enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDMX) ||
        enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDEC))
    {
        if (pAvplay->VidEnable)
        {
            MT_ERR_AVPLAY("vid chn is enable, can not colsed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if ((MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            || (0 != pAvplay->SlaveChnNum) || (0 != pAvplay->VirChnNum))
        {
            MT_ERR_AVPLAY("window is attach to vdec, can not colsed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
    }

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        VidDmxDecTogether = MT_TRUE;
    }

    if (VidDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDEC)))
    {
        if (pAvplay->hVdec != MT_INVALID_HANDLE)
        {
            Ret = AVPLAY_FreeVdec(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free vdec chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (VidDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VDMX)))
    {
        if (pAvplay->hDmxVid != MT_INVALID_HANDLE)
        {
            Ret = AVPLAY_FreeVidChn(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free vdmx chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
    {
        AudDmxDecTogether = MT_TRUE;
    }

    if (AudDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_ADEC)))
    {
        if (pAvplay->hAdec != MT_INVALID_HANDLE)
        {
            Ret = AVPLAY_FreeAdec(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free adec chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (AudDmxDecTogether || (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_ADMX)))
    {
        if (pAvplay->hDmxAud[0] != MT_INVALID_HANDLE)
        {
            Ret = AVPLAY_FreeAudChn(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free aud chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if ((MT_INVALID_HANDLE == pAvplay->hDmxVid) && (MT_INVALID_HANDLE == pAvplay->hDmxAud[0]))
    {
        if (pAvplay->hDmxPcr != MT_INVALID_HANDLE)
        {
            (mt_void)MT_MPI_DMX_PcrSyncDetach(pAvplay->hDmxPcr);
            Ret = MT_MPI_DMX_DestroyPcrChannel(pAvplay->hDmxPcr);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avplay free pcr chn failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->hDmxPcr = MT_INVALID_HANDLE;
        }
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Reset_Buffer(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, MT_UNF_AVPLAY_MEDIA_BUF_E enBuf,const mt_void *pPara)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enBuf < MT_UNF_AVPLAY_MEDIA_BUF_IN)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enBuf >= MT_UNF_AVPLAY_MEDIA_BUF_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
    {
        if (MT_INVALID_HANDLE == pAvplay->hAdec)
        {
            MT_ERR_AVPLAY("aud chn is close, can not start.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_ADEC_DEV_NOT_OPEN;
        }

        MT_INFO_AVPLAY("\n in MT_MPI_AVPLAY_Reset_Buffer  hadec 0x%x   buf 0x%x  \n",  pAvplay->hAdec, enBuf);

        Ret = MT_ERR_ADEC_INVALID_PARA;

        if (enBuf & ((mt_u32)MT_UNF_AVPLAY_MEDIA_BUF_IN))
        {
           Ret = MT_MPI_ADEC_Reset_In_Buf(pAvplay->hAdec);
        }

        if (enBuf & ((mt_u32)MT_UNF_AVPLAY_MEDIA_BUF_OUT))
        {
           Ret = MT_MPI_ADEC_Reset_Out_Buf(pAvplay->hAdec);
        }
     }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}


static mt_s32 AVPLAY_Reset_Buffer(AVPLAY_S *pAvplay)
{
    mt_s32               Ret;
	if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StopAudChn(pAvplay, 1);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("stop aud chn failed.\n");
            return Ret;
        }
        (mt_void)MT_MPI_AO_Track_Flush((mt_handle)NULL);
    }

    if (MT_TRUE == pAvplay->VidEnable)
    {
        if (MT_INVALID_HANDLE != pAvplay->hVdec)
       {
	    	Ret = MT_ERR_VDEC_INVALID_PARA;
			MT_CODEC_RESETPARAM_S param;
			memset(&param, 0, sizeof(MT_CODEC_RESETPARAM_S));
			param.resetDQ = MT_TRUE;
			Ret = MT_MPI_VDEC_ResetChan(pAvplay->hVdec, &param);
			if (Ret != MT_SUCCESS)
			{
				MT_ERR_AVPLAY("call MT_MPI_VDEC_ResetChan failed.\n");
				return Ret;
			}
        }
    }

	if (pAvplay->AudEnable)
    {
        Ret = AVPLAY_StartAudChn(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("start aud chn failed.\n");
            return Ret;
        }
    }
    return Ret;
}

mt_s32 AVPLAY_SetFrmPackingType(AVPLAY_S *pAvplay, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType)
{
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set frm packing type.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* input param check */
    if (*pFrmPackingType >= MT_UNF_FRAME_PACKING_TYPE_BUTT)
    {
        MT_ERR_AVPLAY("FrmPackingType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    Ret = MT_MPI_VDEC_SetChanFrmPackType(pAvplay->hVdec, pFrmPackingType);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_SetChanFrmPackType failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetFrmPackingType(AVPLAY_S *pAvplay, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *pFrmPackingType)
{
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not get frm packing type.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetChanFrmPackType(pAvplay->hVdec, pFrmPackingType);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanFrmPackType failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetVdecFrmRateParam(AVPLAY_S *pAvplay,  MT_UNF_AVPLAY_FRMRATE_PARAM_S *pFrmRate)
{
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec frm rate.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* input param check */
    if (pFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT)
    {
        MT_ERR_AVPLAY("enFrmRateType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

#ifndef JQW
	if(MT_UNF_AVPLAY_FRMRATE_TYPE_PTS == pFrmRate->enFrmRateType)
	{
		MT_WARN_AVPLAY("AVPLAY_FRMRATE_TYPE_PTS not support!\n");
		return MT_SUCCESS;
	}
#endif

	if(MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM == pFrmRate->enFrmRateType)
	{
        if ((pFrmRate->stSetFrmRate.u32fpsInteger != 0)
            || (pFrmRate->stSetFrmRate.u32fpsDecimal != 0)
            )
        {
            MT_ERR_AVPLAY("AVPLAY_FRMRATE_TYPE_STREAM - stSetFrmRate is invalid.\n");
            return MT_ERR_AVPLAY_INVALID_PARA;
        }
	}

    /* input param check */
    if ((MT_UNF_AVPLAY_FRMRATE_TYPE_USER == pFrmRate->enFrmRateType)
        || (MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS == pFrmRate->enFrmRateType)
        )
    {
        if ((pFrmRate->stSetFrmRate.u32fpsInteger == 0)
            && (pFrmRate->stSetFrmRate.u32fpsDecimal == 0)
            )
        {
            MT_ERR_AVPLAY("stSetFrmRate is invalid.\n");
            return MT_ERR_AVPLAY_INVALID_PARA;
        }
    }

    Ret = MT_MPI_VDEC_SetChanFrmRate(pAvplay->hVdec, pFrmRate);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_SetChanFrmRate failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

static mt_s32 AVPLAY_SetVdecFrmRateParamDeclear(AVPLAY_S *pAvplay,  MT_UNF_AVPLAY_FRMRATE_PARAM_S *pFrmRate)
{
    mt_s32 Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec frm rate.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* input param check */
    if (pFrmRate->enFrmRateType >= MT_UNF_AVPLAY_FRMRATE_TYPE_BUTT)
    {
        MT_ERR_AVPLAY("enFrmRateType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

#ifndef JQW
	if(MT_UNF_AVPLAY_FRMRATE_TYPE_PTS == pFrmRate->enFrmRateType)
	{
		MT_WARN_AVPLAY("AVPLAY_FRMRATE_TYPE_PTS not support!\n");
		return MT_SUCCESS;
	}
#endif

	if(MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM == pFrmRate->enFrmRateType)
	{
        if ((pFrmRate->stSetFrmRate.u32fpsInteger != 0)
            || (pFrmRate->stSetFrmRate.u32fpsDecimal != 0)
            )
        {
            MT_ERR_AVPLAY("AVPLAY_FRMRATE_TYPE_STREAM - stSetFrmRate is invalid.\n");
            return MT_ERR_AVPLAY_INVALID_PARA;
        }
	}

    /* input param check */
    if ((MT_UNF_AVPLAY_FRMRATE_TYPE_USER == pFrmRate->enFrmRateType)
        || (MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS == pFrmRate->enFrmRateType)
        )
    {
        if ((pFrmRate->stSetFrmRate.u32fpsInteger == 0)
            && (pFrmRate->stSetFrmRate.u32fpsDecimal == 0)
            )
        {
            MT_ERR_AVPLAY("stSetFrmRate is invalid.\n");
            return MT_ERR_AVPLAY_INVALID_PARA;
        }
    }

    Ret = MT_MPI_VDEC_SetChanFrmRateDeclear(pAvplay->hVdec, pFrmRate);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_SetChanFrmRate failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetVdecFrmRateParam(AVPLAY_S *pAvplay,  MT_UNF_AVPLAY_FRMRATE_PARAM_S *pFrmRate)
{
    mt_s32  Ret;

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec frm rate.\n");
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    Ret = MT_MPI_VDEC_GetChanFrmRate(pAvplay->hVdec, pFrmRate);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanFrmRate failed.\n");
        return Ret;
    }
    return MT_SUCCESS;
}

mt_s32 AVPLAY_Set_AC4_Downmix_Mode(AVPLAY_S *pAvplay, u32 *type)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Set_AC4_Downmix_Mode(type);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Downmix_Mode failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_Dialogue_Enhancement(AVPLAY_S *pAvplay, u32 *val)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Set_AC4_Dialogue_Enhancement(val);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Dialogue_Enhancement failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_Encode_DD_DDP(AVPLAY_S *pAvplay, u32 *encode_type)
{
	mt_s32				 Ret;

	if(MT_TRUE == pAvplay->AudEnable) {
		MT_ERR_AVPLAY("error! set ac4 encode dd/ddp should be called before start audio.\n");
		return MT_FAILURE;
	}

	Ret = MT_MPI_ADEC_Set_AC4_Encode_DD_DDP(encode_type);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Encode_DD_DDP failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_Encode_MAT(AVPLAY_S *pAvplay, u32 *enable)
{
	mt_s32				 Ret;

	if(MT_TRUE == pAvplay->AudEnable) {
		MT_ERR_AVPLAY("error! set ac4 encode MAT should be called before start audio.\n");
		return MT_FAILURE;
	}

	Ret = MT_MPI_ADEC_Set_AC4_Encode_MAT(enable);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Encode_MAT failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_Encode_DAP(AVPLAY_S *pAvplay, u32 *encode_type)
{
	mt_s32				 Ret;

	if(MT_TRUE == pAvplay->AudEnable) {
		MT_ERR_AVPLAY("error! set ac4 encode DAP should be called before start audio.\n");
		return MT_FAILURE;
	}

	Ret = MT_MPI_ADEC_Set_AC4_Encode_DAP(encode_type);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Encode_DAP failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_Pres_ID(AVPLAY_S *pAvplay, s32 *val)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Set_AC4_Pres_ID(val);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Pres_ID failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_AD_OnOff(AVPLAY_S *pAvplay, u32 *enable)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Set_AC4_AD_OnOff(enable);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Encode_MAT failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_AC4_AD_VOL_Weight(AVPLAY_S *pAvplay, s32 *val)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Set_AC4_AD_Weight(val);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_AC4_Encode_MAT failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Sel_AC4_AD_Type(AVPLAY_S *pAvplay, u32 *val)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Sel_AC4_AD_Type(val);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Sel_AC4_AD_Type failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Sel_AC4_AD_Type_Over_Lang(AVPLAY_S *pAvplay)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Sel_AC4_AD_Type_Over_Lang();
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Sel_AC4_AD_Type_Over_Lang failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Sel_AC4_Lang(AVPLAY_S *pAvplay, MT_UNF_AVPLAY_AC4_LANG_S *ac4_lang)
{
	mt_s32				 Ret;

	Ret = MT_MPI_ADEC_Sel_AC4_Lang(ac4_lang);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Sel_AC4_LANG failed.\n");
	}

	return Ret;
}

mt_s32 AVPLAY_Set_Dolby_Force_Ms12_Dec(AVPLAY_S *pAvplay, mt_u32 *enable)
{
	mt_s32				 Ret;

	if(MT_TRUE == pAvplay->AudEnable) {
		MT_ERR_AVPLAY("error! it's should be called before start audio.\n");
		return MT_FAILURE;
	}

	Ret = MT_MPI_ADEC_Set_Dolby_Force_MS12_Dec(enable);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Set_Dolby_Force_MS12_Dec failed.\n");
	}

	return Ret;
}

mt_s32 MT_MPI_AVPLAY_SetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara)
{
    AVPLAY_S                        *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S               AvplayUsrAddr;
    mt_s32                          Ret;

    if (enAttrID >= MT_UNF_AVPLAY_ATTR_ID_BUTT)
    {
        MT_ERR_AVPLAY("para enAttrID is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (!pPara)
    {
        MT_ERR_AVPLAY("para pPara is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    switch (enAttrID)
    {
        case MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE:
            Ret = AVPLAY_SetStreamMode(pAvplay, (MT_UNF_AVPLAY_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set stream mode failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_ADEC:
            if (pAvplay->AudEnable)
            {
                MT_ERR_AVPLAY("aud chn is running, can not set adec attr.\n");
                Ret = MT_ERR_AVPLAY_INVALID_OPT;
                break;
            }
            Ret = AVPLAY_SetAdecAttr(pAvplay, (MT_UNF_ACODEC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set adec attr failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_VDEC:
            {
                MT_UNF_DMX_CHAN_ATTR_S ChAttr;
                MT_UNF_VCODEC_ATTR_S *vdec_attr = (MT_UNF_VCODEC_ATTR_S *)pPara;
                MT_DRV_DISP_UNBLANK_MODE_E disp_unblank_mode = DISP_UNBLANK_MODE_STABLE;
	            /*
                ** set vdecType for dmx
                */
				if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
                	MT_MPI_DMX_GetChannelAttr(pAvplay->hDmxVid, &ChAttr);
                ChAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_VID;
                ChAttr.vCodecType = vdec_attr->enType;
				if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
                	MT_MPI_DMX_SetChannelAttr(pAvplay->hDmxVid, &ChAttr);


                /*
                ** set unblankmode for vo_fw
                */
                switch (vdec_attr->enUnBlank)
                {
                    case MT_UNF_VCODEC_UNBLANK_FAST:
                        disp_unblank_mode = DISP_UNBLANK_MODE_FAST;
                        break;
                    case MT_UNF_VCODEC_UNBLANK_STABLE:
                        disp_unblank_mode = DISP_UNBLANK_MODE_STABLE;
                        break;
                    case MT_UNF_VCODEC_UNBLANK_USER:
                        disp_unblank_mode = DISP_UNBLANK_MODE_USER;
                        break;
                    case MT_UNF_VCODEC_UNBLANK_SYNC:
                        disp_unblank_mode = DISP_UNBLANK_MODE_SYNC;
                        break;
                    default:
                        MT_ERR_AVPLAY("unspported unblankmode !!!!.\n");
                }
                Ret = MT_MPI_DISP_SetUnblankMode(disp_unblank_mode);
                if (Ret != MT_SUCCESS)
                {
                    MT_ERR_AVPLAY("set unblankmode attr failed.\n");
                }
            }

            Ret = AVPLAY_SetVdecAttr(pAvplay, (MT_UNF_VCODEC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set vdec attr failed.\n");
            }
            break;
        #ifdef CONFIG_MT_AUDIO_AD
        case MT_UNF_AVPLAY_ATTR_ID_AD:
            {
                MT_UNF_AVPLAY_AD_ATTR_S *tmp=(MT_UNF_AVPLAY_AD_ATTR_S *)pPara;
                Ret = AVPLAY_Set_AD_ATTR(pAvplay, tmp);
            }
            break;
		case MT_UNF_AVPLAY_ATTR_ID_AD_VOL_WEIGHT:
		{
#ifdef CONFIG_MT_AUDIO_AD
			Ret = AVPLAY_Set_AD_vol_weight(pAvplay, (mt_u32 *)pPara);
			if (Ret != MT_SUCCESS)
			{
				MT_ERR_AVPLAY("set AD vol weight failed.\n");
			}
#endif
		}
			break;
        #endif
        case MT_UNF_AVPLAY_ATTR_ID_AUD_PID:
            Ret = AVPLAY_SetPid(pAvplay, enAttrID, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set aud pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_VID_PID:
            Ret = AVPLAY_SetPid(pAvplay, enAttrID, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set vid pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_PCR_PID:
            Ret = AVPLAY_SetPid(pAvplay, enAttrID, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set pcr pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_SYNC:
        {
        	MT_UNF_SYNC_ATTR_S *pSyncAttr = (MT_UNF_SYNC_ATTR_S *)pPara;

        	pSyncAttr->enStreamType = pAvplay->AvplayAttr.stStreamAttr.enStreamType;

            Ret = AVPLAY_SetSyncAttr(pAvplay, pSyncAttr);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set sync attr failed.\n");
            }
            break;
        }

        case MT_UNF_AVPLAY_ATTR_ID_OVERFLOW:
            Ret = AVPLAY_SetOverflowProc(pAvplay, (MT_UNF_AVPLAY_OVERFLOW_E *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set overflow proc failed.\n");
            }
            break;

       case MT_UNF_AVPLAY_ATTR_ID_MULTIAUD:
            Ret = AVPLAY_SetMultiAud(pAvplay, (MT_UNF_AVPLAY_MULTIAUD_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("set multi aud failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM:
            Ret = AVPLAY_SetVdecFrmRateParam(pAvplay, (MT_UNF_AVPLAY_FRMRATE_PARAM_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Set frm rate failed.\n");
            }
            break;
		case MT_UNF_AVPLAY_ATTR_ID_DECLEAR_FRMRATE_PARAM:
			Ret = AVPLAY_SetVdecFrmRateParamDeclear(pAvplay, (MT_UNF_AVPLAY_FRMRATE_PARAM_S *)pPara);
			if (Ret != MT_SUCCESS)
			{
				MT_ERR_AVPLAY("Set frm rate failed.\n");
			}
			break;
        case MT_UNF_AVPLAY_ATTR_ID_FRMPACK_TYPE:
            Ret = AVPLAY_SetFrmPackingType(pAvplay, (MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Set frm packing type failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_LOW_DELAY:
            Ret = AVPLAY_SetLowDelay(pAvplay, (MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Set Low Delay failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC:
            Ret = AVPLAY_SetDmxAvsync(pAvplay, (MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Set Dmx Avsync failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_AVC_CFG:
            Ret = AVPLAY_AvcConfig(pAvplay, (MT_HADECODE_AVC_PARAM_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avc config failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_APTS_ADJUST:
            Ret = AVPLAY_AptsAdjust(pAvplay, (ulong)((*(mt_u32 *)pPara)));
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Avc config failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_DOLBY_DOWNMIX_MODE:
            Ret = AVPLAY_SetDolbyDownmixMode(pAvplay, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Set audio set Dolby Downmix Mode failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_DMX_MULTIAUDSYNC:
            Ret = AVPLAY_SetDmxAudiosync(pAvplay, (MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set Dmx Audiosync failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_DMX_BUF_FULL_CARE:
            Ret = AVPLAY_SetDmxBufFullCare(pAvplay, (MT_UNF_DMX_BUF_FULL_CARE_S *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set Dmx BufFullCare failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_WATERMARK_FILTER:
            Ret = AVPLAY_SetWatermarkFilter(pAvplay, (MT_UNF_AVPLAY_WATERMARK_FILTER_ATTR_S *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set watermark failed.\n");
            }
            break;

		case MT_UNF_AVPLAY_ATTR_ID_AC4_AD_ONOFF:
			Ret = AVPLAY_Set_AC4_AD_OnOff(pAvplay, (mt_u32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 AD on/off failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_AD_VOL_WEIGHT:
			Ret = AVPLAY_Set_AC4_AD_VOL_Weight(pAvplay, (mt_s32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 AD vol weight failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE:
			Ret = AVPLAY_Sel_AC4_AD_Type(pAvplay, (mt_u32 *)pPara);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Sel AC4 AD content type failed.\n");
			}
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_AD_TYPE_OVER_LANG:
			Ret = AVPLAY_Sel_AC4_AD_Type_Over_Lang(pAvplay);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Sel AC4 AD type over lang failed.\n");
			}
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_LANG:
			Ret = AVPLAY_Sel_AC4_Lang(pAvplay, (MT_UNF_AVPLAY_AC4_LANG_S *)pPara);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Sel AC4 language failed.\n");
			}
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_DOWNMIX_MODE:
			Ret = AVPLAY_Set_AC4_Downmix_Mode(pAvplay, (mt_u32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 downmix mode failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_DIALOGUE_ENHANCEMENT:
			Ret = AVPLAY_Set_AC4_Dialogue_Enhancement(pAvplay, (mt_u32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 dialogue enhancement failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_DD_DDP:
			Ret = AVPLAY_Set_AC4_Encode_DD_DDP(pAvplay, (mt_u32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 encode dd/ddp failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_ENCODE_MAT:
			Ret = AVPLAY_Set_AC4_Encode_MAT(pAvplay, (mt_u32 *)pPara);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("Set AC4 encode MAT failed.\n");
            }
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_SET_PRES_ID:
			Ret = AVPLAY_Set_AC4_Pres_ID(pAvplay, (mt_s32 *)pPara);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Set AC4 Presentation ID failed.\n");
			}
			break;
		case MT_UNF_AVPLAY_ATTR_ID_AC4_SET_ENCODE_DAP:
			Ret = AVPLAY_Set_AC4_Encode_DAP(pAvplay, (mt_u32 *)pPara);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Set AC4 Encode DAP failed.\n");
			}
			break;
		case MT_UNF_AVPLAY_ATTR_ID_DOLBY_FORCE_MS12_DEC:
			Ret = AVPLAY_Set_Dolby_Force_Ms12_Dec(pAvplay, (mt_u32 *)pPara);
			if (MT_SUCCESS != Ret)
			{
				MT_ERR_AVPLAY("Set Dolby force MS12 failed.\n");
			}
			break;

        default:
            MT_AVPLAY_INST_UNLOCK();
            return MT_SUCCESS;

    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetAttr(mt_handle hAvplay, MT_UNF_AVPLAY_ATTR_ID_E enAttrID, mt_void *pPara)
{
    AVPLAY_S                        *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S               AvplayUsrAddr;
    mt_s32                          Ret;

    if (enAttrID >= MT_UNF_AVPLAY_ATTR_ID_BUTT)
    {
        MT_ERR_AVPLAY("para enAttrID is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (!pPara)
    {
        MT_ERR_AVPLAY("para pPara is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    switch (enAttrID)
    {
        case MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE:
            Ret = AVPLAY_GetStreamMode(pAvplay, (MT_UNF_AVPLAY_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get stream mode failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_ADEC:
            Ret = AVPLAY_GetAdecAttr(pAvplay, (MT_UNF_ACODEC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get adec attr failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_VDEC:
            Ret = AVPLAY_GetVdecAttr(pAvplay, (MT_UNF_VCODEC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get vdec attr failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_AUD_PID:
            Ret = AVPLAY_GetPid(pAvplay, enAttrID, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get aud pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_VID_PID:
            Ret = AVPLAY_GetPid(pAvplay, enAttrID, (mt_u32*)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get vid pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_PCR_PID:
            Ret = AVPLAY_GetPid(pAvplay, enAttrID, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get pcr pid failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_SYNC:
            Ret = AVPLAY_GetSyncAttr(pAvplay, (MT_UNF_SYNC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get sync attr failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_OVERFLOW:
            Ret = AVPLAY_GetOverflowProc(pAvplay, (MT_UNF_AVPLAY_OVERFLOW_E *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("get overflow proc failed.\n");
            }
            break;

        case MT_UNF_AVPLAY_ATTR_ID_MULTIAUD:
            Ret = AVPLAY_GetMultiAud(pAvplay, (MT_UNF_AVPLAY_MULTIAUD_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get multi audio failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM:
            Ret = AVPLAY_GetVdecFrmRateParam(pAvplay, (MT_UNF_AVPLAY_FRMRATE_PARAM_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get frm rate failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_FRMPACK_TYPE:
            Ret = AVPLAY_GetFrmPackingType(pAvplay, (MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get frm packing type  failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_LOW_DELAY:
            Ret = AVPLAY_GetLowDelay(pAvplay, (MT_UNF_AVPLAY_LOW_DELAY_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get Low Delay failed.\n");
            }
            break;
        case MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC:
            Ret = AVPLAY_GetDmxAvsync(pAvplay, (MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get Dmx Avsync failed.\n");
            }
            break;
        #ifdef CONFIG_MT_AUDIO_AD
		case MT_UNF_AVPLAY_ATTR_ID_AD_VOL_WEIGHT:
			Ret = AVPLAY_Get_AD_vol_weight(pAvplay, (mt_u32 *)pPara);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("Get AD vol weight failed.\n");
            }
			break;
        #endif

        default:
            MT_AVPLAY_INST_UNLOCK();
            return MT_SUCCESS;
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_DecodeIFrame(mt_handle hAvplay, const MT_UNF_AVPLAY_I_FRAME_S *pstIframe,
                                              MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture)
{
    mt_s32                      Ret;
    AVPLAY_S                    *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S           AvplayUsrAddr;
    MT_UNF_VIDEO_FRAME_INFO_S   stVidFrameInfo;
    MT_DRV_VIDEO_FRAME_S        stDrvFrm;
    mt_u32                      i, j;
    mt_handle                   hWindow = MT_INVALID_HANDLE;
    MT_DRV_VIDEO_FRAME_PACKAGE_S    stFrmPack;
    MT_BOOL                     bCapture = MT_FALSE;
    MT_DRV_VPSS_PORT_CFG_S      stOldCfg, stNewCfg;

    if (MT_NULL == pstIframe)
    {
        MT_ERR_AVPLAY("para pstIframe is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    memset(&stFrmPack, 0x0, sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("hVdec is invalid.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (MT_TRUE == pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("vid chn is opened.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (pAvplay->stIFrame.hport != MT_INVALID_HANDLE)
    {
        MT_ERR_AVPLAY("please release I frame first.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* if there is no window exist, we need create vpss source */
    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        if (MT_NULL == pstCapPicture)
        {
            MT_ERR_AVPLAY("there is no window.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_VDEC_CreatePort(pAvplay->hVdec, &pAvplay->MasterFrmChn.hPort, VDEC_PORT_HD);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_VDEC_CreatePort ERR, Ret=%x\n", Ret);
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_VDEC_SetPortType(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, VDEC_PORT_TYPE_MASTER);
        Ret |= MT_MPI_VDEC_EnablePort(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_VDEC_EnablePort ERR, Ret=%x\n", Ret);
            MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort);
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
    }

    if (MT_NULL != pstCapPicture)
    {
        bCapture = MT_TRUE;
    }

    Ret = MT_MPI_VDEC_GetPortAttr(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, &stOldCfg);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_GetPortAttr ERR, Ret=%x\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    stNewCfg = stOldCfg;
    // need capture frame, config vpss to do not zoom
    if (MT_TRUE == bCapture)
    {
        stNewCfg.s32OutputWidth = 0;
        stNewCfg.s32OutputHeight = 0;
    }

    /*set vpss attr, do not do zoom*/
    Ret = MT_MPI_VDEC_SetPortAttr(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, &stNewCfg);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_SetPortAttr ERR, Ret=%x\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    memset(&stVidFrameInfo, 0x0, sizeof(MT_UNF_VIDEO_FRAME_INFO_S));
    Ret = MT_MPI_VDEC_ChanIFrameDecode(pAvplay->hVdec, (MT_UNF_AVPLAY_I_FRAME_S *)pstIframe, &stDrvFrm, bCapture);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanIFrameDecode failed.\n");
        (mt_void)MT_MPI_VDEC_SetPortAttr(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, &stOldCfg);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    /*wait for vpss process complete*/
    for (i=0; i<20; i++)
    {
        Ret = MT_MPI_VDEC_ReceiveFrame(pAvplay->hVdec, &stFrmPack);
        if (Ret == MT_SUCCESS)
        {
            break;
        }

        MT_USLEEP(10 * 1000);
    }

    if (i >= 20)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_ReceiveFrame failed, Ret=%x.\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    /*resume vpss attr*/
    Ret = MT_MPI_VDEC_SetPortAttr(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort, &stOldCfg);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_SetPortAttr ERR, Ret=%x\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* display on vo */
    if (MT_FALSE == bCapture)
    {
        for (i=0; i<stFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, stFrmPack.stFrame[i].hport, &hWindow);

            if (hWindow == pAvplay->MasterFrmChn.hWindow)
            {
                break;
            }
        }

        if (i == stFrmPack.u32FrmNum)
        {
            MT_ERR_AVPLAY("I Frame Dec: No master window\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_WIN_QueueFrame(hWindow, &stFrmPack.stFrame[i].stFrameVideo);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("I Frame Dec: Queue frame to master win err, Ret=%x\n", Ret);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        for (i=0; i<stFrmPack.u32FrmNum; i++)
        {
            (mt_void)AVPLAY_GetWindowByPort(pAvplay, stFrmPack.stFrame[i].hport, &hWindow);

            for (j=0; j<pAvplay->SlaveChnNum; j++)
            {
                if (hWindow == pAvplay->SlaveFrmChn[j].hWindow)
                {
                    Ret = MT_MPI_WIN_QueueFrame(hWindow, &stFrmPack.stFrame[i].stFrameVideo);
                    if (MT_SUCCESS != Ret)
                    {
                        MT_ERR_AVPLAY("I Frame Dec: Queue frame to slave win err, Ret=%x\n", Ret);
                        MT_AVPLAY_INST_UNLOCK();
                        return Ret;
                    }
                }
            }
        }
    }
    else
    {
        /*use frame of port0, release others*/
        memcpy(&pAvplay->stIFrame, &stFrmPack.stFrame[0], sizeof(MT_DRV_VDEC_FRAME_S));

        for (i = 1; i < stFrmPack.u32FrmNum; i++)
        {
            (mt_void)MT_MPI_VDEC_ReleaseFrame(stFrmPack.stFrame[i].hport, &stFrmPack.stFrame[i].stFrameVideo);
        }

        AVPLAY_DRV2UNF_VidFrm(&(pAvplay->stIFrame.stFrameVideo), pstCapPicture);
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_ReleaseIFrame(mt_handle hAvplay, MT_UNF_VIDEO_FRAME_INFO_S *pstCapPicture)
{
    mt_s32                      Ret;
    AVPLAY_S                    *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S           AvplayUsrAddr;

    if (MT_NULL == pstCapPicture)
    {
        MT_ERR_AVPLAY("para pstCapPicture is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("hVdec is invalid.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    AVPLAY_Mutex_Lock(pAvplay->pAvplayThreadMutex);

    /* destroy vpss source */
    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        Ret = MT_MPI_VDEC_DisablePort(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort);
        Ret |= MT_MPI_VDEC_DestroyPort(pAvplay->hVdec, pAvplay->MasterFrmChn.hPort);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("MT_MPI_VDEC_DestroyPort ERR, Ret=%x\n", Ret);

            AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
            MT_AVPLAY_INST_UNLOCK();

            return Ret;
        }

         pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;
    }
    else
    {
        if (pAvplay->stIFrame.hport != MT_INVALID_HANDLE)
        {
            (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->stIFrame.hport, &pAvplay->stIFrame.stFrameVideo);

            memset(&pAvplay->stIFrame.stFrameVideo, 0x0, sizeof(MT_DRV_VIDEO_FRAME_S));
        }
    }

    pAvplay->stIFrame.hport = MT_INVALID_HANDLE;

    AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}


mt_s32 MT_MPI_AVPLAY_SetAVsyncMode(mt_handle hAvplay, MT_UNF_SYNC_REF_E enAVsyncMode)
{
    AVPLAY_S              *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S      AvplayUsrAddr;

    mt_s32                   Ret;

    if (enAVsyncMode >= MT_UNF_AVPLAY_SYNC_REF_BUTT)
    {
        MT_ERR_AVPLAY("para enAVsyncMode is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_SYNC_SetAVsyncMode(pAvplay->hSync, enAVsyncMode);

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}


mt_s32 MT_MPI_AVPLAY_SetDecodeMode(mt_handle hAvplay, MT_UNF_VCODEC_MODE_E enDecodeMode)
{
    AVPLAY_S              *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S      AvplayUsrAddr;
    MT_UNF_VCODEC_ATTR_S   VdecAttr;
    mt_s32                   Ret;

    if (enDecodeMode >= MT_UNF_VCODEC_MODE_BUTT)
    {
        MT_ERR_AVPLAY("para enDecodeMode is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close, can not set vdec attr.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetChanAttr(pAvplay->hVdec, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanAttr failed.\n");
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    VdecAttr.enMode = enDecodeMode;

    MT_INFO_AVPLAY("yyyyyyyyyyyyyyyyyyyyyyyyyyyyy \n");

    Ret = MT_MPI_VDEC_SetChanAttr(pAvplay->hVdec, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_VDEC_SetChanAttr failed.\n");
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_RegisterEvent(mt_handle      hAvplay,
                                   MT_UNF_AVPLAY_EVENT_E     enEvent,
                                   MT_UNF_AVPLAY_EVENT_CB_FN pfnEventCB)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (enEvent >= MT_UNF_AVPLAY_EVENT_BUTT)
    {
        MT_ERR_AVPLAY("para enEvent is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (!pfnEventCB)
    {
        MT_ERR_AVPLAY("para pfnEventCB is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    /*
    if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_STOP)
    {
        MT_ERR_AVPLAY("can not register when avplay is not stopped.\n");
        (mt_void)AVPLAY_Mutex_UnLock(pAvplay->pAvplayMutex);
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    */

    if (pAvplay->EvtCbFunc[enEvent])
    {
        MT_ERR_AVPLAY("this event has been registered.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    pAvplay->EvtCbFunc[enEvent] = pfnEventCB;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_UnRegisterEvent(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (enEvent >= MT_UNF_AVPLAY_EVENT_BUTT)
    {
        MT_ERR_AVPLAY("para enEvent is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

#if 0  /* remove this limit, you can UnRegisterEvent at any time after init.  */
    if (pAvplay->CurStatus != MT_UNF_AVPLAY_STATUS_STOP)
    {
        MT_ERR_AVPLAY("can not unregister when avplay is not stopped.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
#endif

    pAvplay->EvtCbFunc[enEvent] = MT_NULL;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_RegisterAcodecLib(const mt_char *pFileName)
{
    mt_s32    Ret = MT_SUCCESS;

    if (!pFileName)
    {
        MT_ERR_AVPLAY("para pFileName is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    Ret = MT_MPI_ADEC_RegisterDeoder(pFileName);
    if (Ret != MT_SUCCESS)
    {
        MT_INFO_AVPLAY("call MT_MPI_ADEC_RegisterDeoderLib failed.\n");
    }

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_FoundSupportDeoder(const HA_FORMAT_E enFormat,mt_u32 * penDstCodecID)
{
    mt_s32    Ret = MT_SUCCESS;

    Ret = MT_MPI_ADEC_FoundSupportDeoder(enFormat,penDstCodecID);
    if (Ret != MT_SUCCESS)
    {
        MT_INFO_AVPLAY("call MT_MPI_ADEC_FoundSupportDeoder failed.\n");
    }

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_ConfigAcodec( const mt_u32 enDstCodecID, mt_void *pstConfigStructure)
{
    mt_s32 Ret = MT_SUCCESS;

    Ret = MT_MPI_ADEC_SetConfigDeoder(enDstCodecID, pstConfigStructure);
    if (Ret != MT_SUCCESS)
    {
        MT_INFO_AVPLAY("call MT_MPI_ADEC_SetConfigDeoder failed.\n");
    }

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_PreStart(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS != pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        MT_ERR_AVPLAY("MT_MPI_AVPLAY_PreStart is Not supported in es mode\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_NOT_SUPPORT;
    }

#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif
        if (!pAvplay->VidEnable && !pAvplay->bVidPreEnable)
        {
            Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxVid);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("MT_MPI_DMX_OpenChannel failed:%x\n",Ret);
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->VidPreBufThreshhold = 0;
            pAvplay->VidPreSysTime = MT_INVALID_TIME;
            pAvplay->bVidPreEnable = MT_TRUE;
            AVPLAY_PrePlay(pAvplay);
        }
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    }

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
    {
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif
        if (!pAvplay->AudEnable && !pAvplay->bAudPreEnable)
        {
            mt_u32 i;
            for(i= 0; i < pAvplay->DmxAudChnNum; i++)
            {
                Ret = MT_MPI_DMX_OpenChannel(pAvplay->hDmxAud[i]);
                if(MT_SUCCESS != Ret)
                {
                    MT_ERR_AVPLAY("MT_MPI_DMX_OpenChannel %d failed: %x\n",i,Ret);
                    break;
                }
            }

            if ( i <  pAvplay->DmxAudChnNum)
            {
                mt_u32 j;
                for ( j = 0 ; j < i ; j++ )
                {
                    (mt_void)MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud[j]);
                }
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->AudPreBufThreshhold = 0;
            pAvplay->AudPreSysTime = MT_INVALID_TIME;
            pAvplay->bAudPreEnable = MT_TRUE;
            AVPLAY_PrePlay(pAvplay);

        }
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, pAvplay->DmxPcrPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
#ifndef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}


mt_s32 MT_MPI_AVPLAY_Start(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    MT_SYNC_DATA_SOURCE_E data_source = SYNC_DATA_SOURCE_TUNER;
    MT_UNF_DMX_PORT_MODE_E dmx_port_mode;
    mt_s32               Ret;
	MT_UNF_HDMI_STATUS_S hdmi_status;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

#ifdef CONFIG_MT_DOLBY_AC4_SUPPORT
	AVPLAY_GET_INST_AND_LOCK();
	MT_AVPLAY_INST_UNLOCK();
	if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD)) {
		if(HA_AUDIO_ID_DOLBY_AC4 == pAvplay->AdecType) {// ||
			//HA_AUDIO_ID_DOLBY_PLUS == pAvplay->AdecType) {
			MT_MPI_AVPLAY_SW_ADEC_ENABLE(hAvplay);
		}
	}
#endif

    AVPLAY_GET_INST_AND_LOCK();

#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif
        if (!pAvplay->VidEnable)
        {
            (mt_void)mt_mpi_stat_event(STAT_EVENT_VSTART_IN, 0);
            if (MT_INVALID_HANDLE == pAvplay->hVdec)
            {
                MT_ERR_AVPLAY("vid chn is close, can not start.\n");

#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            if ((MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
                && (0 == pAvplay->SlaveChnNum) && (0 == pAvplay->VirChnNum)
                )
            {
                MT_ERR_AVPLAY("window is not attached, can not start.\n");
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            Ret = AVPLAY_StartVidChn(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("start vid chn failed.\n");
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
            /*
            Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play Vid failed.\n");
            }
            */

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
		    MT_MPI_VDEC_LcevcStart(pAvplay->hVdec);
#endif
            pAvplay->VidEnable = MT_TRUE;
            AVPLAY_Play(pAvplay);

            (mt_void)mt_mpi_stat_event(STAT_EVENT_VSTART, 0);
        }
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    }

    if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
    {
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

        pAvplay->PreDmxAudChn = 0xffffffff;

        if (!pAvplay->AudEnable)
        {
            (mt_void)mt_mpi_stat_event(STAT_EVENT_ASTART_IN, 0);
            if (MT_INVALID_HANDLE == pAvplay->hAdec)
            {
                MT_ERR_AVPLAY("aud chn is close, can not start.\n");
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            Ret = AVPLAY_StartAudChn(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("start aud chn failed.\n");
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            Ret = MT_MPI_SYNC_Play(pAvplay->hSync);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_SYNC_Play Aud failed.\n");
            }

            pAvplay->AudEnable = MT_TRUE;
            AVPLAY_Play(pAvplay);

            (mt_void)mt_mpi_stat_event(STAT_EVENT_ASTART, 0);
        }
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
    }

#ifdef JQW
    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, pAvplay->DmxPcrPid);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
#ifndef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }
#endif
#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif

    if (pAvplay->VidEnable)
    {
        MT_UNF_AVPLAY_PLAY_INFO_S play_info;
        MT_DRV_DISP_STEREO_MODE_E dis_x = 0;
        MT_DRV_DISP_FMT_E dis_format = MT_DRV_DISP_FMT_BUTT;
        memset(&play_info, 0, sizeof(play_info));
        play_info.a_pid = pAvplay->DmxAudPid[0];
        play_info.v_pid = pAvplay->DmxVidPid;
        play_info.a_type = pAvplay->AdecType;
        play_info.v_type = pAvplay->VdecAttr.enType;

        MT_INFO_AVPLAY("%s, %d, apid: %d, vpid: %d, atype: %x, vtype: %d, TVformat: %d\n", __func__, __LINE__,
            play_info.a_pid, play_info.v_pid, play_info.a_type, play_info.v_type);

        Ret = MT_MPI_DISP_GetFormat(MT_DRV_DISPLAY_1, &dis_x, &dis_format);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("%s call MT_MPI_DISP_GetFormat failed\n", __func__);
        }

        play_info.tvformat = dis_format;

		Ret = MT_MPI_HDMI_GetStatus(MT_UNF_HDMI_ID_0, &hdmi_status);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("[%s %d]call MT_MPI_HDMI_GetStatus failed\n", __func__, __LINE__);
        }
		play_info.is_hdmi_connected = hdmi_status.bConnected;

		if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
		{
			if(play_info.a_pid != 8191)
			{
	        	Ret = MT_MPI_SYNC_Play_Info_Cfg(pAvplay->hSync, (mt_u32 *)&play_info);
			}
		}
		else
		{
			Ret = MT_MPI_SYNC_Play_Info_Cfg(pAvplay->hSync, (mt_u32 *)&play_info);
		}
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("%s call MT_MPI_SYNC_Play_Info_Cfg failed\n", __func__);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        Ret = MT_MPI_DMX_GetPortMode(pAvplay->AvplayAttr.u32DemuxId, &dmx_port_mode);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("%s call MT_MPI_DMX_GetTSPortId failed\n", __func__);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        if (dmx_port_mode >= MT_UNF_DMX_PORT_MODE_BUTT)
        {
            MT_ERR_AVPLAY("%s err: dmx prot mode: %d\n", __func__, dmx_port_mode);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        if (MT_UNF_DMX_PORT_MODE_RAM != dmx_port_mode)
        {
            data_source = SYNC_DATA_SOURCE_TUNER;
        }
        else
        {
            data_source = SYNC_DATA_SOURCE_PVR;
        }
    }
    else
    {
        data_source = SYNC_DATA_SOURCE_FILE;
    }

    Ret = MT_MPI_SYNC_Data_Source_Cfg(pAvplay->hSync, &data_source);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("%s call MT_MPI_SYNC_Data_Source_Cfg failed\n", __func__);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}


mt_s32 MT_MPI_AVPLAY_PreStop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn,const MT_UNF_AVPLAY_PRESTOP_OPT_S *pPreStopOpt)
{
    MT_ERR_AVPLAY("MT_MPI_AVPLAY_PreStop is not supported\n");
    return MT_ERR_AVPLAY_NOT_SUPPORT;
}


mt_s32 MT_MPI_AVPLAY_Stop(mt_handle hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_E enChn, const MT_UNF_AVPLAY_STOP_OPT_S *pStop)
{
    AVPLAY_S                   *pAvplay = MT_NULL;
    MT_UNF_AVPLAY_STOP_OPT_S   StopOpt;
    AVPLAY_USR_ADDR_S          AvplayUsrAddr;
    mt_u32                     SysTime;
    MT_BOOL                    Block;
    mt_s32                     Ret;
    MT_BOOL                    bStopNotify = MT_FALSE;
    mt_u32                     i;

    if (enChn < MT_UNF_AVPLAY_MEDIA_CHAN_AUD)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (enChn >= MT_UNF_AVPLAY_MEDIA_CHAN_BUTT)
    {
        MT_ERR_AVPLAY("para enChn is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (pStop)
    {
        if (pStop->enMode >= MT_UNF_AVPLAY_STOP_MODE_BUTT)
        {
            MT_ERR_AVPLAY("para pStop->enMode is invalid.\n");
            return MT_ERR_AVPLAY_INVALID_PARA;
        }

        StopOpt.u32TimeoutMs = pStop->u32TimeoutMs;
        StopOpt.enMode = pStop->enMode;
    }
    else
    {
        StopOpt.u32TimeoutMs = 0;
        StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    }

    AVPLAY_GET_INST_AND_LOCK();

    /*The relevant channel is already stopped*/
    if ( ((MT_UNF_AVPLAY_MEDIA_CHAN_AUD == enChn) && (!pAvplay->AudEnable)  && (!pAvplay->bAudPreEnable) )
       || ((MT_UNF_AVPLAY_MEDIA_CHAN_VID == enChn) && (!pAvplay->VidEnable) && (!pAvplay->bVidPreEnable))
       || ((((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD | (mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID) == enChn)
           && (!pAvplay->AudEnable)  && (!pAvplay->bAudPreEnable) && (!pAvplay->VidEnable) && (!pAvplay->bVidPreEnable)))
    {
        MT_INFO_AVPLAY("The chn is already stoped\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }

#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    /*Non Block invoke*/
    if (0 == StopOpt.u32TimeoutMs)
    {
        if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID))
        {

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
			MT_MPI_VDEC_LcevcStop(pAvplay->hVdec);
#endif

#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif
            if (pAvplay->VidEnable)
            {
				(mt_void)mt_mpi_stat_event(STAT_EVENT_VSTOP_IN, 0);
                Ret = AVPLAY_StopVidChn(pAvplay, StopOpt.enMode);
                if (Ret != MT_SUCCESS)
                {
                    MT_ERR_AVPLAY("stop vid chn failed.\n");
#ifdef AVPLAY_VID_THREAD
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                    MT_AVPLAY_INST_UNLOCK();
                    return Ret;
                }

                if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
                {
                    /* resume the frc and window ratio */
                    pAvplay->bFrcEnable = MT_TRUE;
                    pAvplay->FrcParamCfg.u32PlayRate = AVPLAY_ALG_FRC_BASE_PLAY_RATIO;

					/* display resume */
					MT_MPI_DISP_SetTrickMode(TM_NORMAL);
                }

                pAvplay->VidEnable = MT_FALSE;
                pAvplay->bVidPreEnable = MT_FALSE;

                /* may be only stop vidchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
                if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
                {
                    /*Release vpss frame*/
                    (mt_void)AVPLAY_RelAllChnFrame(pAvplay);
                    pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
                }

                (mt_void)mt_mpi_stat_event(STAT_EVENT_VSTOP, 0);
                /* in asynchronous mode, we should do a little
                 * delay to bring out the switch effect */
                //if(StopOpt.enMode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
                //    usleep(10000);
            }
            else if ( pAvplay->bVidPreEnable )
            {
                Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxVid);
                if(MT_SUCCESS != Ret)
                {
                    MT_ERR_AVPLAY("MT_MPI_DMX_CloseChannel failed:%x.\n",Ret);
#ifdef AVPLAY_VID_THREAD
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                    MT_AVPLAY_INST_UNLOCK();
                    return Ret;
                }

                pAvplay->bVidPreEnable = MT_FALSE;
            }
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        }

        if (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD))
        {
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif
            if (pAvplay->AudEnable)
            {
				(mt_void)mt_mpi_stat_event(STAT_EVENT_ASTOP_IN, 0);
                Ret = AVPLAY_StopAudChn(pAvplay, 0);
                if (Ret != MT_SUCCESS)
                {
                    MT_ERR_AVPLAY("stop aud chn failed.\n");
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                    MT_AVPLAY_INST_UNLOCK();
                    return Ret;
                }
                pAvplay->AudEnable = MT_FALSE;
                pAvplay->bAudPreEnable = MT_FALSE;

                /* may be only stop audchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO] = MT_FALSE;
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO2] = MT_FALSE;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
                pAvplay->AvplayProcDataFlag[AVPLAY_PROC_ADEC_AO3] = MT_FALSE;
#endif

                (mt_void)mt_mpi_stat_event(STAT_EVENT_ASTOP, 0);
            }
            else if (pAvplay->bAudPreEnable)
            {
                    for(i = 0; i < pAvplay->DmxAudChnNum; i++)
                    {
                        Ret = MT_MPI_DMX_CloseChannel(pAvplay->hDmxAud[i]);
                        if (Ret != MT_SUCCESS)
                        {
                            MT_ERR_AVPLAY("MT_MPI_DMX_CloseChannel failed:%x.\n",Ret);
                            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                            MT_AVPLAY_INST_UNLOCK();
                            return Ret;
                        }
                    }

                pAvplay->bAudPreEnable = MT_FALSE;
            }
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            if ((!pAvplay->VidEnable) && (!pAvplay->bVidPreEnable)
                &&(!pAvplay->AudEnable) && (!pAvplay->bAudPreEnable) )
            {
                Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, 0x1fff);
                if (Ret != MT_SUCCESS)
                {
                    MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
#ifndef AVPLAY_VID_THREAD
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                    MT_AVPLAY_INST_UNLOCK();
                    return Ret;
                }
            }
        }

        if ((!pAvplay->VidEnable) && (!pAvplay->bVidPreEnable)
            &&(!pAvplay->AudEnable) && (!pAvplay->bAudPreEnable) )
        {
            AVPLAY_Stop(pAvplay);
            bStopNotify = MT_TRUE;
        }
    }
    else
    {
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif
        if ((pAvplay->VidEnable && pAvplay->AudEnable)
          &&(enChn <= MT_UNF_AVPLAY_MEDIA_CHAN_VID)
           )
        {
            MT_ERR_AVPLAY("must control vid and aud chn together.\n");
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_PARA;
        }

        if (( (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_VID)) && (!pAvplay->VidEnable) && (!pAvplay->bVidPreEnable) )
            || ( (enChn & ((mt_u32)MT_UNF_AVPLAY_MEDIA_CHAN_AUD)) && (!pAvplay->AudEnable) && (!pAvplay->bAudPreEnable) ))
        {
            MT_ERR_AVPLAY("not support this mode.\n");
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_PARA;
        }

        if (StopOpt.u32TimeoutMs != (mt_u32)SYS_TIME_MAX)
        {
            Block = MT_FALSE;
        }
        else
        {
            Block = MT_TRUE;
        }

        Ret = AVPLAY_SetEosFlag(pAvplay);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("ERR: AVPLAY_SetEosFlag, Ret = %x.\n", Ret);
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);

        pAvplay->EosStartTime = AVPLAY_GetSysTime();
        while (1)
        {
            if(MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
            {
                break;
            }

            if (!Block)
            {
                SysTime = AVPLAY_GetSysTime();

                if (SysTime > pAvplay->EosStartTime)
                {
                    pAvplay->EosDurationTime = SysTime - pAvplay->EosStartTime;
                }
                else
                {
                    pAvplay->EosDurationTime = (0xFFFFFFFFU - pAvplay->EosStartTime) + 1 + SysTime;
                }

                if (pAvplay->EosDurationTime >= StopOpt.u32TimeoutMs)
                {
                    MT_ERR_AVPLAY("eos proc timeout.\n");
                    break;
                }
            }

            (mt_void)MT_USLEEP(AVPLAY_SYS_SLEEP_TIME*1000);
        }

#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

        if (pAvplay->VidEnable)
        {
            Ret = AVPLAY_StopVidChn(pAvplay, StopOpt.enMode);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("stop vid chn failed.\n");

#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            {
                /* resume the frc and window ratio */
                pAvplay->bFrcEnable = MT_TRUE;
                pAvplay->FrcParamCfg.u32PlayRate = AVPLAY_ALG_FRC_BASE_PLAY_RATIO;

				/* display resume */
				MT_MPI_DISP_SetTrickMode(TM_NORMAL);
            }

            pAvplay->VidEnable = MT_FALSE;
            pAvplay->bVidPreEnable = MT_FALSE;

            (mt_void)mt_mpi_stat_event(STAT_EVENT_VSTOP, 0);
        }

#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif

#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif
        if (pAvplay->AudEnable)
        {
            Ret = AVPLAY_StopAudChn(pAvplay, 0);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("stop aud chn failed.\n");
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->AudEnable = MT_FALSE;
            pAvplay->bAudPreEnable = MT_FALSE;

            (mt_void)mt_mpi_stat_event(STAT_EVENT_ASTOP, 0);
        }

#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_DMX_PcrPidSet(pAvplay->hDmxPcr, 0x1fff);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_DMX_PcrPidSet failed.\n");
#ifndef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }

        AVPLAY_Stop(pAvplay);
        bStopNotify = MT_TRUE;
    }

#ifndef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif

    MT_AVPLAY_INST_UNLOCK();

    if (MT_TRUE == bStopNotify)
    {
        AVPLAY_Notify(pAvplay, MT_UNF_AVPLAY_EVENT_STOP, MT_NULL);
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Pause(mt_handle hAvplay, const MT_UNF_AVPLAY_PAUSE_OPT_S *pstPauseOpt)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;
    mt_u32              i;

    AVPLAY_GET_INST_AND_LOCK();
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	MT_MPI_VDEC_LcevcPause(pAvplay->hVdec, 1);
#endif

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if ((!pAvplay->VidEnable)
      &&(!pAvplay->AudEnable)
       )
    {
        MT_ERR_AVPLAY("vid and aud chn is stopped.\n");
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if(pstPauseOpt->avSyncPause) {
        Ret = MT_MPI_SYNC_Pause(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Pause failed, Ret=0x%x.\n", Ret);
        }
    }

    pAvplay->AvplayDataPushPause = (MT_BOOL)pstPauseOpt->avPushPause;

    AVPLAY_Pause(pAvplay);

    if (pAvplay->VidEnable)
    {
        if(pstPauseOpt->avDecPause)
            MT_MPI_VDEC_ChanPauseDecode(pAvplay->hVdec, 1);

        if(pstPauseOpt->avRenderPause) {
            if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->MasterFrmChn.hWindow, MT_TRUE);
            }

            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->SlaveFrmChn[i].hWindow, MT_TRUE);
            }

            for (i=0; i<pAvplay->VirChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->VirFrmChn[i].hWindow, MT_TRUE);
            }
        }
    }

    if (pAvplay->AudEnable)
    {
        if(pstPauseOpt->avRenderPause) {
	        Ret |= MT_MPI_AO_Track_Pause((mt_handle)NULL);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_HIAO_SetPause failed, Ret=0x%x.\n", Ret);
            }
        }
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Freeze(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;
    mt_u32              i;

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_UNF_AVPLAY_STATUS_FREEZE == pAvplay->CurStatus)
    {
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if ((!pAvplay->VidEnable)
      &&(!pAvplay->AudEnable)
       )
    {
        MT_ERR_AVPLAY("vid and aud chn is stopped.\n");
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_SYNC_Pause(pAvplay->hSync);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Pause failed, Ret=0x%x.\n", Ret);
    }

    AVPLAY_Freeze(pAvplay);

    if (pAvplay->VidEnable)
    {
        if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
        {
            (mt_void)MT_MPI_WIN_Freeze(pAvplay->MasterFrmChn.hWindow, MT_TRUE, MT_DRV_WIN_SWITCH_LAST);
        }

        for (i=0; i<pAvplay->SlaveChnNum; i++)
        {
            (mt_void)MT_MPI_WIN_Freeze(pAvplay->SlaveFrmChn[i].hWindow, MT_TRUE, MT_DRV_WIN_SWITCH_LAST);
        }

        for (i=0; i<pAvplay->VirChnNum; i++)
        {
            (mt_void)MT_MPI_WIN_Freeze(pAvplay->VirFrmChn[i].hWindow, MT_TRUE, MT_DRV_WIN_SWITCH_LAST);
        }
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Tplay(mt_handle hAvplay, const MT_UNF_AVPLAY_TPLAY_OPT_S *pstTplayOpt)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;
    mt_u32                  i;
    MT_BOOL                 bSetEosFlag;
	DISP_TRICK_MODE_E       trickmode;

	if(NULL == pstTplayOpt)
		return MT_FAILURE;

	if(pstTplayOpt->enTplayDirect >= MT_UNF_AVPLAY_TPLAY_DIRECT_BUTT)
		return MT_FAILURE;

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if (!pAvplay->VidEnable && !pAvplay->AudEnable)
    {
        MT_ERR_AVPLAY("vid and aud chn is stopped.\n");
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        MT_ERR_AVPLAY("AVPLAY has not attach master window.\n");
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    MT_ERR_AVPLAY("last %d, cur %d, \n", pAvplay->LstStatus, pAvplay->CurStatus);
    if (((MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->LstStatus || MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->LstStatus) && (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus))
      ||(MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus)
       )
    {
        bSetEosFlag = pAvplay->bSetEosFlag;
        if (bSetEosFlag)
        {
            Ret = AVPLAY_SetEosFlag(pAvplay);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("ERR: AVPLAY_SetEosFlag, Ret = %x.\n", Ret);
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }
    }

    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        if (pAvplay->VidEnable)
        {
            if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->MasterFrmChn.hWindow, MT_FALSE);
            }

            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->SlaveFrmChn[i].hWindow, MT_FALSE);
            }

            for (i=0; i<pAvplay->VirChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->VirFrmChn[i].hWindow, MT_FALSE);
            }

            MT_MPI_VDEC_ChanPauseDecode(pAvplay->hVdec, 0);
        }

        /* pause->tplay, resume audio out */
        if (pAvplay->AudEnable)
        {
            Ret |= MT_MPI_AO_Track_Resume((mt_handle)NULL);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_HIAO_SetPause failed, Ret=0x%x.\n", Ret);
            }
        }

        /* pause->tplay, resume sync */
        Ret = MT_MPI_SYNC_Resume(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Resume failed, Ret=0x%x.\n", Ret);
        }
    }

	trickmode = TM_NORMAL;
	if(pstTplayOpt->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD &&
		pstTplayOpt->u32SpeedInteger > 1)
	{
        if(pstTplayOpt->u32SpeedInteger == 2)
		{
			trickmode = TM_FFWD_X2;
		}
		else
		{
			trickmode = TM_FFWD_X2_MORE;
		}
	}
	else if(pstTplayOpt->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD &&
		pstTplayOpt->u32SpeedInteger == 0) // slow forward
    {
		if(pstTplayOpt->u32SpeedDecimal == 500)
		{
        	trickmode = TM_SFWD_X2;
		}
		else
		{
			MT_ERR_AVPLAY("Only support 0.5x play(%u/%u)\n", pstTplayOpt->u32SpeedInteger, pstTplayOpt->u32SpeedDecimal);
		}
    }
	else if(pstTplayOpt->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD)
	{
		trickmode = TM_FREV;
	}
    pAvplay->trickmode = trickmode;
    pAvplay->u32SpeedDecimal = pstTplayOpt->u32SpeedDecimal;

	pAvplay->LastPlayFrameMs = 0;

	if(trickmode != TM_NORMAL)
		MT_MPI_ADEC_set_trickmode(pAvplay->hAdec, 1);
	else
		MT_MPI_ADEC_set_trickmode(pAvplay->hAdec, 0);

    MT_ERR_AVPLAY("trick mode %d, %u, %u\n", pstTplayOpt->enTplayDirect, pstTplayOpt->u32SpeedInteger, pstTplayOpt->u32SpeedDecimal);
#ifndef CFG_DISP_TRICK_BY_USER
	MT_MPI_DISP_SetTrickMode(trickmode);
#endif

	//20200806 slow forward: trickmode = TM_NORMAL, but is Tplay
	if (pstTplayOpt->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD
		&& pstTplayOpt->u32SpeedInteger == 1
		&& pstTplayOpt->u32SpeedDecimal == 0)
	{
		(mt_void)MT_MPI_SYNC_Play(pAvplay->hSync);
		AVPLAY_Play(pAvplay);
	}
	else
	{
		(mt_void)MT_MPI_SYNC_Tplay(pAvplay->hSync);
		AVPLAY_Tplay(pAvplay);
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_SetTrickCfg(mt_handle hAvplay, MT_UNF_DEC_TRICK_PARAM_S *pstTrickParam)
{
	AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;

	AVPLAY_GET_INST_AND_LOCK();
	Ret = MT_MPI_VDEC_SetTrickCfg(pAvplay->hVdec, pstTrickParam);
	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}

mt_s32 MT_MPI_AVPLAY_DecFrmType(mt_handle hAvplay, MT_UNF_DEC_FRM_TYPE_E eDecFrmType)
{
	AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;

	AVPLAY_GET_INST_AND_LOCK();
	Ret = MT_MPI_VDEC_DecFrmType(pAvplay->hVdec, eDecFrmType);
	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}

mt_s32 MT_MPI_AVPLAY_Resume(mt_handle hAvplay)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;
    mt_u32                  i;

    AVPLAY_GET_INST_AND_LOCK();
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	MT_MPI_VDEC_LcevcPause(pAvplay->hVdec, 0);
#endif

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    pAvplay->bStepMode = MT_FALSE;
    pAvplay->bStepPlay = MT_FALSE;


    if (MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus)
    {
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }

    if ((!pAvplay->VidEnable)
      &&(!pAvplay->AudEnable)
       )
    {
        MT_ERR_AVPLAY("vid and aud chn is stopped.\n");
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if(MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus ||
        (MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->LstStatus && MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus))
    {
		Ret = AVPLAY_Reset_Buffer(pAvplay);
      //  Ret = AVPLAY_Reset(pAvplay);
        if (MT_SUCCESS != Ret)
        {
            MT_ERR_AVPLAY("AVPLAY_Reset_Buffer, Ret=%x.\n", Ret);
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        (mt_void)MT_MPI_SYNC_Play(pAvplay->hSync);

	   /* display resume */
       MT_MPI_DISP_SetTrickMode(TM_NORMAL);
    }

    /* resume hiao and sync if curstatus is pause */
    if (MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus)
    {
        if (pAvplay->VidEnable)
        {
            if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->MasterFrmChn.hWindow, MT_FALSE);
            }

            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->SlaveFrmChn[i].hWindow, MT_FALSE);
            }

            for (i=0; i<pAvplay->VirChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Pause(pAvplay->VirFrmChn[i].hWindow, MT_FALSE);
            }

            MT_MPI_VDEC_ChanPauseDecode(pAvplay->hVdec, 0);
        }

        if (pAvplay->AudEnable)
        {
            Ret |= MT_MPI_AO_Track_Resume((mt_handle)NULL);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_HIAO_SetPause failed, Ret=0x%x.\n", Ret);
            }
        }

        Ret = MT_MPI_SYNC_Resume(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Resume failed.\n");
        }
    }

        /* resume audio out and sync if curstatus is pause */
    if (MT_UNF_AVPLAY_STATUS_FREEZE == pAvplay->CurStatus)
    {
        if (pAvplay->VidEnable)
        {
            if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
            {
                (mt_void)MT_MPI_WIN_Freeze(pAvplay->MasterFrmChn.hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
            }

            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Freeze(pAvplay->SlaveFrmChn[i].hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
            }

            for (i=0; i<pAvplay->VirChnNum; i++)
            {
                (mt_void)MT_MPI_WIN_Freeze(pAvplay->VirFrmChn[i].hWindow, MT_FALSE, MT_DRV_WIN_SWITCH_LAST);
            }
        }

        if (pAvplay->AudEnable)
        {
            Ret |= MT_MPI_AO_Track_Resume((mt_handle)NULL);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_HIAO_SetPause failed, Ret=0x%x.\n", Ret);
            }
        }

        Ret = MT_MPI_SYNC_Resume(pAvplay->hSync);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_SYNC_Resume failed.\n");
        }
    }

    AVPLAY_Play(pAvplay);

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if ((MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus)
        && (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        && (MT_NULL != pstResetOpt)
        && (MT_INVALID_PTS_U64 != pstResetOpt->u64SeekPtsMs)
    )
    {
        MT_INFO_AVPLAY("sdk buf seek enter\n");

        Ret = AVPLAY_Seek(pAvplay, pstResetOpt->u64SeekPtsMs);
        if (Ret != MT_SUCCESS)
        {
            MT_INFO_AVPLAY("not in sdk buf\n");

            Ret = AVPLAY_Reset(pAvplay);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call AVPLAY_Reset failed.\n");
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
        }

        MT_INFO_AVPLAY("sdk buf seek quit\n");
    }
    else
    {
        Ret = AVPLAY_Reset(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call AVPLAY_Reset failed.\n");
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

    if (pAvplay->DmxAudChnNum > 1)
    {
        Ret = MT_MPI_DMX_ChannelIndexResume(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);

        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("Channel Index Resume failed, %#x\n", Ret);
        }
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_HRSeek(mt_handle hAvplay, const MT_UNF_AVPLAY_HRSEEK_OPT_S *pstSeekOpt)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S   AvplayUsrAddr;
    mt_s32              Ret = MT_SUCCESS;

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if (((MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus || MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus))
        && (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        && (MT_NULL != pstSeekOpt)
        && (MT_INVALID_TIME != pstSeekOpt->u32SeekTimeMs)
    )
    {
        pAvplay->TargetTime = (mt_u64)pstSeekOpt->u32SeekTimeMs *1000;
        pAvplay->bTargetAudioRendered = MT_FALSE;
        pAvplay->bTargetVideoRendered = MT_FALSE;
        MT_ALWAYS_PRINT("high resolution seek to %llu\n", pAvplay->TargetTime);
    }
    else
    {
        Ret = MT_FAILURE;
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Flush(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S   AvplayUsrAddr;
    mt_s32              Ret;

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if (MT_UNF_AVPLAY_STATUS_PREPLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_SEEK == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_FREEZE == pAvplay->CurStatus
		)
    {
        Ret = AVPLAY_Flush(pAvplay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call AVPLAY_Flush failed.\n");
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_Flush_Audio(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);


    if (MT_UNF_AVPLAY_STATUS_PREPLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_PLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_TPLAY == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_PAUSE == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_SEEK == pAvplay->CurStatus
		|| MT_UNF_AVPLAY_STATUS_FREEZE == pAvplay->CurStatus
		)
    {
        Ret = MT_MPI_ADEC_Flush_Buf(pAvplay->hAdec);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call AVPLAY_Flush failed.\n");
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);

            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}


mt_s32 MT_MPI_AVPLAY_GetFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame)
{
  return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_ReleaseFrame(mt_handle  hAvplay,
                            MT_UNF_AO_FRAMEINFO_S * p_ao_frame)
{
  return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetBuf(mt_handle  hAvplay,
                            MT_UNF_AVPLAY_BUFID_E enBufId,
                            mt_u32                u32ReqLen,
                            MT_UNF_STREAM_BUF_S  *pstData,
                            mt_u32                u32TimeOutMs)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (enBufId >= MT_UNF_AVPLAY_BUF_ID_BUTT)
    {
        MT_ERR_AVPLAY("para enBufId is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (!pstData)
    {
        MT_ERR_AVPLAY("para pstData is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    if (u32TimeOutMs != 0)
    {
        MT_ERR_AVPLAY("enBufId=%d NOT support block mode, please set 'u32TimeOutMs' to 0.\n", enBufId);
        return MT_ERR_AVPLAY_NOT_SUPPORT;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        MT_ERR_AVPLAY("avplay is ts stream mode.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /*if user contine getting or putting buffer after EOS, Current status change to Last status*/
    if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
    {
        MT_WARN_AVPLAY("avplay curstatus is eos.\n");
        pAvplay->CurStatus = pAvplay->LstStatus;

    }

    pAvplay->bSetEosFlag = MT_FALSE;

    if (MT_UNF_AVPLAY_BUF_ID_ES_VID == enBufId)
    {
        if (!pAvplay->VidEnable)
        {
            MT_WARN_AVPLAY("vid chn is stopped.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        //printf("HHHHHHHHHHHHHHHH MT_MPI_AVPLAY_GetBuf \n");
        pAvplay->AvplayVidEsBuf.u32ScrapSize = pAvplay->u32ScrapSize;
        Ret = MT_MPI_VDEC_ChanGetBuffer(pAvplay->hVdec, u32ReqLen, &pAvplay->AvplayVidEsBuf);//Rock_hu
        if (Ret != MT_SUCCESS)
        {
            if (Ret != MT_ERR_VDEC_BUFFER_FULL)
            {
                MT_WARN_AVPLAY("call MT_MPI_VDEC_ChanGetBuffer failed, Ret=0x%x.\n", Ret);
            }

            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pstData->pu8Data = pAvplay->AvplayVidEsBuf.pu8Addr;
        pstData->u32Size = pAvplay->AvplayVidEsBuf.u32BufSize;
        pstData->u32PhyData = pAvplay->AvplayVidEsBuf.u32PhyAddr;
    }

    if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == enBufId)
    {
        if (!pAvplay->AudEnable)
        {
            MT_WARN_AVPLAY("aud chn is stopped.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
#if 0
        Ret = MT_MPI_ADEC_GetDelayMs(pAvplay->hAdec, &pAvplay->AdecDelayMs);
        if (MT_SUCCESS == Ret && pAvplay->AdecDelayMs > AVPLAY_ADEC_MAX_DELAY)
        {
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
#endif
        Ret = MT_MPI_ADEC_GetBuffer(pAvplay->hAdec, u32ReqLen, &pAvplay->AvplayAudEsBuf);
        if (Ret != MT_SUCCESS)
        {
            if ((Ret != MT_ERR_ADEC_IN_BUF_FULL) && (Ret != MT_ERR_ADEC_IN_PTSBUF_FULL) )
            {
                MT_ERR_AVPLAY("call MT_MPI_ADEC_GetBuffer failed, Ret=0x%x.\n", Ret);
            }

            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pstData->pu8Data = pAvplay->AvplayAudEsBuf.pu8Data;
        pstData->u32Size = pAvplay->AvplayAudEsBuf.u32Size;
        pstData->u32PhyData = pAvplay->AvplayAudEsBuf.u32PhyData;
        pstData->pu8Data2 = pAvplay->AvplayAudEsBuf.pu8Data2;
        pstData->u32Size2 = pAvplay->AvplayAudEsBuf.u32Size2;
        pstData->u32PhyData2 = pAvplay->AvplayAudEsBuf.u32PhyData2;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_IsNormaPlay(mt_handle hAvplay)
{
	AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

	if(pAvplay->trickmode != TM_FREV)
		Ret = MT_SUCCESS;
	else
		Ret = MT_FAILURE;

	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}

mt_s32 MT_MPI_AVPLAY_PutBuf(mt_handle hAvplay, MT_UNF_AVPLAY_BUFID_E enBufId, mt_u32 u32ValidDataLen,
                            mt_u64 u64Pts, MT_UNF_AVPLAY_PUTBUFEX_OPT_S *pstExOpt, mt_u32 PtsValide,
                            mt_u32 FrameFinsh, mt_u32 u32EosFlag)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;
    mt_u32               u32TimeMs;

    if (enBufId >= MT_UNF_AVPLAY_BUF_ID_BUTT)
    {
        MT_ERR_AVPLAY("para enBufId is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        MT_ERR_AVPLAY("avplay is ts stream mode.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /*if user contine getting or putting buffer after EOS, Current status change to Last status*/
    if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
    {
        MT_WARN_AVPLAY("avplay curstatus is eos.\n");
        pAvplay->CurStatus = pAvplay->LstStatus;

    }

    pAvplay->bSetEosFlag = MT_FALSE;
    pAvplay->bVideoLastFrameDecoded = 0;

    if (MT_UNF_AVPLAY_BUF_ID_ES_VID == enBufId)
    {
        if (!pAvplay->VidEnable)
        {
            MT_ERR_AVPLAY("vid chn is stopped.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }
        if (u32ValidDataLen <= 0)
        {
            MT_ERR_AVPLAY("invalid data len(%u)!\n",u32ValidDataLen);
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_PARA;
        }
        if (pstExOpt == MT_NULL)
        {
            MT_ERR_AVPLAY("invalid ext option(%p)!\n",pstExOpt);
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_PARA;
        }

		if (PtsValide && u64Pts != 0 && u64Pts == pAvplay->AvplayVidEsBuf.u64Pts)
		{
            MT_ERR_AVPLAY("[WARNING] same VPTS(%llu)!\n",u64Pts);
		}

		sys_get_time_stamp_ms(&u32TimeMs);
		if (pAvplay->AvplayVidEsBuf.u32PutTime != 0
			&& u32TimeMs > pAvplay->AvplayVidEsBuf.u32PutTime
			&& u32TimeMs > pAvplay->AvplayVidEsBuf.u32PutTime + 100)
		{
            MT_ERR_AVPLAY("[WARNING] put ves buffer discontinuous {%u-%u=%u(ms)}!\n",
            		pAvplay->AvplayVidEsBuf.u32PutTime,
            		u32TimeMs,
            		u32TimeMs-pAvplay->AvplayVidEsBuf.u32PutTime);
		}

		pAvplay->AvplayVidEsBuf.u32PutTime = u32TimeMs;
        pAvplay->AvplayVidEsBuf.u32BufSize = u32ValidDataLen;
        pAvplay->AvplayVidEsBuf.u64Pts = u64Pts;
        pAvplay->AvplayVidEsBuf.u32PtsValide = PtsValide;
        pAvplay->AvplayVidEsBuf.u32FrameFinsh = FrameFinsh;
        pAvplay->AvplayVidEsBuf.bEndOfFrame = pstExOpt->bEndOfFrm;
        pAvplay->AvplayVidEsBuf.u32PreFrameFinsh = pAvplay->u32PreFrameFinsh;
        pAvplay->AvplayVidEsBuf.u32ScrapSize = pAvplay->u32ScrapSize;
        pAvplay->AvplayVidEsBuf.u32EosFlag = u32EosFlag;

        if (pstExOpt->bContinue)
        {
            pAvplay->AvplayVidEsBuf.bDiscontinuous = MT_FALSE;
        }
        else
        {
            pAvplay->AvplayVidEsBuf.bDiscontinuous = MT_TRUE;
        }

        Ret = MT_MPI_VDEC_ChanPutBuffer(pAvplay->hVdec, &pAvplay->AvplayVidEsBuf);
        pAvplay->u32PreFrameFinsh = FrameFinsh;
        pAvplay->u32ScrapSize = pAvplay->AvplayVidEsBuf.u32ScrapSize;
        //printf("3333 u32ScrapSize = %x, %x \n", pAvplay->AvplayVidEsBuf.u32ScrapSize, pAvplay->u32ScrapSize);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_ChanPutBuffer failed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }
    }

    if (MT_UNF_AVPLAY_BUF_ID_ES_AUD == enBufId)
    {
        mt_u64 diff = (64*1000);

        if (!pAvplay->AudEnable)
        {
            MT_ERR_AVPLAY("aud chn is stopped.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if(!pAvplay->bTargetAudioRendered)
        {
    		if((pAvplay->TargetTime != MT_INVALID_TIME64) && (pAvplay->TargetTime > diff))
            {
    			if(u64Pts < (pAvplay->TargetTime - diff))
    			{
                    MT_DBG_AVPLAY("***> Discard Audio pts %llu, target seek time %llu\n", u64Pts, pAvplay->TargetTime);
    				MT_AVPLAY_INST_UNLOCK();
    				return MT_SUCCESS;
    			}
    		}
        }

		if(pAvplay->trickmode == TM_NORMAL) {
        pAvplay->AvplayAudEsBuf.u32Size = u32ValidDataLen;
        Ret = MT_MPI_ADEC_PutBuffer(pAvplay->hAdec, &pAvplay->AvplayAudEsBuf, u64Pts, PtsValide, u32EosFlag);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_ADEC_PutBuffer failed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
	        }
        }
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}


mt_s32 MT_MPI_AVPLAY_GetSyncVdecHandle(mt_handle hAvplay, mt_handle *phVdec, mt_handle *phSync)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;

    if (!phVdec)
    {
        MT_ERR_AVPLAY("para phVdec is invalid.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    if (!phSync)
    {
        MT_ERR_AVPLAY("para phSync is invalid.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("Avplay have not vdec.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    *phVdec = pAvplay->hVdec;
    *phSync = pAvplay->hSync;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetSndHandle(mt_handle hAvplay, mt_handle *phTrack)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;

    if (!phTrack)
    {
        MT_ERR_AVPLAY("para phTrack is invalid.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->hSyncTrack)
    {
        MT_ERR_AVPLAY("Avplay have not main track.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    *phTrack = pAvplay->hSyncTrack;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

// TODO: ???y????????
mt_s32 MT_MPI_AVPLAY_GetWindowHandle(mt_handle hAvplay, mt_handle *phWindow)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (!phWindow)
    {
        MT_ERR_AVPLAY("para phWindow is invalid.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        MT_ERR_AVPLAY("AVPLAY has not attach master window.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    *phWindow = pAvplay->MasterFrmChn.hWindow;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_AttachWindow(mt_handle hAvplay, mt_handle hWindow)
{
    AVPLAY_S                    *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S           AvplayUsrAddr;
    mt_u32                      i;
    mt_s32                      Ret;
    MT_DRV_WIN_INFO_S           stWinInfo;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_AVPLAY("para hWindow is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    /*free frame which avplay hold*/
    if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
    {
        for (i = 0; i < pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            if (MT_INVALID_HANDLE != pAvplay->CurFrmPack.stFrame[i].hport)
            {
                (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &(pAvplay->CurFrmPack.stFrame[i].stFrameVideo));
            }
        }

        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
    }

    Ret = MT_MPI_WIN_GetInfo(hWindow, &stWinInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_WIN_GetPrivnfo.\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }
    /* homologous window*/
    if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == stWinInfo.eType)
    {
        if (pAvplay->MasterFrmChn.hWindow == stWinInfo.hPrim)
        {
            MT_ERR_AVPLAY("this window is already attached.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_SUCCESS;
        }

        /* if attach homologous window, homologous window must be master window*/
        if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
        {
            MT_ERR_AVPLAY("avplay can only attach one master handle.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        if (pAvplay->SlaveChnNum >= AVPLAY_MAX_SLAVE_FRMCHAN)
        {
            MT_ERR_AVPLAY("avplay has attached max slave window.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = AVPLAY_CreatePort(pAvplay, stWinInfo.hPrim, VDEC_PORT_HD, &(pAvplay->MasterFrmChn.hPort));
        if(MT_SUCCESS != Ret)
        {
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->MasterFrmChn.hPort, VDEC_PORT_TYPE_MASTER);
        if(MT_SUCCESS != Ret)
        {
            (mt_void)AVPLAY_DestroyPort(pAvplay, stWinInfo.hPrim, pAvplay->MasterFrmChn.hPort);
            pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        Ret = AVPLAY_CreatePort(pAvplay, stWinInfo.hSec, VDEC_PORT_SD, &(pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort));
        if(MT_SUCCESS != Ret)
        {
            (mt_void)AVPLAY_DestroyPort(pAvplay, stWinInfo.hPrim, pAvplay->MasterFrmChn.hPort);
            pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort,VDEC_PORT_TYPE_SLAVE);
        if(MT_SUCCESS != Ret)
        {
            (mt_void)AVPLAY_DestroyPort(pAvplay, stWinInfo.hPrim, pAvplay->MasterFrmChn.hPort);
            pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;
            (mt_void)AVPLAY_DestroyPort(pAvplay, stWinInfo.hSec, pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort);
            pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pAvplay->MasterFrmChn.hWindow = stWinInfo.hPrim;
        pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hWindow = stWinInfo.hSec;

        pAvplay->SlaveChnNum++;

        pAvplay->hSharedOrgWin = hWindow;
    }
    /*  analogous master window*/
    else if (MT_DRV_WIN_ACTIVE_SINGLE == stWinInfo.eType)
    {
        if (hWindow == pAvplay->MasterFrmChn.hWindow)
        {
            MT_ERR_AVPLAY("this window is alreay attached!\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_SUCCESS;
        }

        if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
        {
            Ret = AVPLAY_CreatePort(pAvplay, hWindow, VDEC_PORT_HD, &(pAvplay->MasterFrmChn.hPort));
            if (MT_SUCCESS != Ret)
            {
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->MasterFrmChn.hPort, VDEC_PORT_TYPE_MASTER);
            if(MT_SUCCESS != Ret)
            {
                (mt_void)AVPLAY_DestroyPort(pAvplay, hWindow, pAvplay->MasterFrmChn.hPort);
                pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->MasterFrmChn.hWindow = hWindow;
        }
        else
        {
            //another master window, save it as slave window , for example: ktv scene
            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                if (pAvplay->SlaveFrmChn[i].hWindow == hWindow)
                {
                    MT_ERR_AVPLAY("this window is already attached!\n");
#ifdef AVPLAY_VID_THREAD
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                    MT_AVPLAY_INST_UNLOCK();
                    return MT_SUCCESS;
                }
            }

            if (pAvplay->SlaveChnNum >= AVPLAY_MAX_SLAVE_FRMCHAN)
            {
                MT_ERR_AVPLAY("avplay has attached max slave window.\n");
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            Ret = AVPLAY_CreatePort(pAvplay, hWindow, VDEC_PORT_SD, &(pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort));
            if(MT_SUCCESS != Ret)
            {
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort,VDEC_PORT_TYPE_SLAVE);
            if(MT_SUCCESS != Ret)
            {
                (mt_void)AVPLAY_DestroyPort(pAvplay, hWindow, pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort);
                pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum].hWindow = hWindow;
            pAvplay->SlaveChnNum++;
        }
    }
    /*  analogous virtual window*/
    else
    {
        for (i=0; i<pAvplay->VirChnNum; i++)
        {
            if (pAvplay->VirFrmChn[i].hWindow == hWindow)
            {
                MT_ERR_AVPLAY("this window is already attached!\n");
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_SUCCESS;
            }
        }

        if (pAvplay->VirChnNum >= AVPLAY_MAX_VIR_FRMCHAN)
        {
            MT_ERR_AVPLAY("the avplay has attached max window!\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret= AVPLAY_CreatePort(pAvplay, hWindow, VDEC_PORT_STR, &pAvplay->VirFrmChn[pAvplay->VirChnNum].hPort);
        if (MT_SUCCESS != Ret)
        {
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->VirFrmChn[pAvplay->VirChnNum].hPort, VDEC_PORT_TYPE_VIRTUAL);
        if(MT_SUCCESS != Ret)
        {
            (mt_void)AVPLAY_DestroyPort(pAvplay, hWindow, pAvplay->VirFrmChn[pAvplay->VirChnNum].hPort);
            pAvplay->VirFrmChn[pAvplay->VirChnNum].hPort = MT_INVALID_HANDLE;

#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pAvplay->VirFrmChn[pAvplay->VirChnNum].hWindow = hWindow;
        pAvplay->VirChnNum++;
    }

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif

    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_DetachWindow(mt_handle hAvplay, mt_handle hWindow)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_u32                  i;
    mt_s32                  Ret;
    MT_DRV_WIN_INFO_S       WinInfo;

    if (MT_INVALID_HANDLE == hWindow)
    {
        MT_ERR_AVPLAY("para hWindow is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    /*free frame which avplay hold*/
    if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
    {
        for (i = 0; i < pAvplay->CurFrmPack.u32FrmNum; i++)
        {
            if (MT_INVALID_HANDLE != pAvplay->CurFrmPack.stFrame[i].hport)
            {
                (mt_void)MT_MPI_VDEC_ReleaseFrame(pAvplay->CurFrmPack.stFrame[i].hport, &(pAvplay->CurFrmPack.stFrame[i].stFrameVideo));
            }
        }

        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
    }

    Ret = MT_MPI_WIN_GetInfo(hWindow, &WinInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_MPI_VO_GetWindowInfo.\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    /* homologous window*/ /* ?????? */
    if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinInfo.eType)
    {
        if (pAvplay->MasterFrmChn.hWindow != WinInfo.hPrim)
        {
            MT_ERR_AVPLAY("ERR: this is not a attached window.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        for (i=0; i<pAvplay->SlaveChnNum; i++)
        {
            if (pAvplay->SlaveFrmChn[i].hWindow == WinInfo.hSec)
            {
                break;
            }
        }

        if (i == pAvplay->SlaveChnNum)
        {
            MT_ERR_AVPLAY("ERR: this is not a attached window.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = AVPLAY_DestroyPort(pAvplay, pAvplay->MasterFrmChn.hWindow, pAvplay->MasterFrmChn.hPort);
        Ret |= AVPLAY_DestroyPort(pAvplay, pAvplay->SlaveFrmChn[i].hWindow, pAvplay->SlaveFrmChn[i].hPort);
        if (MT_SUCCESS != Ret)
        {
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        pAvplay->MasterFrmChn.hWindow = MT_INVALID_HANDLE;
        pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;

        pAvplay->SlaveFrmChn[i].hWindow = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow;
        pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow = MT_INVALID_HANDLE;

        pAvplay->SlaveFrmChn[i].hPort = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort;
        pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort = MT_INVALID_HANDLE;

        pAvplay->SlaveChnNum--;

        //look up another master window
        for (i=0; i<pAvplay->SlaveChnNum; i++)
        {
            Ret = MT_MPI_WIN_GetInfo(pAvplay->SlaveFrmChn[i].hWindow, &WinInfo);
            if (MT_SUCCESS == Ret)
            {
                if (MT_DRV_WIN_ACTIVE_SINGLE == WinInfo.eType)
                {
                    break;
                }
            }
        }

        //find it
        if (i<pAvplay->SlaveChnNum)
        {
            pAvplay->MasterFrmChn.hWindow = pAvplay->SlaveFrmChn[i].hWindow;
            pAvplay->MasterFrmChn.hPort = pAvplay->SlaveFrmChn[i].hPort;

            Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->MasterFrmChn.hPort, VDEC_PORT_TYPE_MASTER);
            if(MT_SUCCESS != Ret)
            {
                MT_ERR_AVPLAY("ERR: set main port failed.\n");
            }

            pAvplay->SlaveFrmChn[i].hWindow = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow;
            pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow = MT_INVALID_HANDLE;
            pAvplay->SlaveFrmChn[i].hPort = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort;
            pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort = MT_INVALID_HANDLE;

            pAvplay->SlaveChnNum--;
        }

        pAvplay->hSharedOrgWin = MT_INVALID_HANDLE;
    }
    /*  analogous master window*/ /* ???? ???????????? */
    else if (MT_DRV_WIN_ACTIVE_SINGLE == WinInfo.eType)
    {
        if (pAvplay->MasterFrmChn.hWindow == hWindow)
        {
            Ret = AVPLAY_DestroyPort(pAvplay, pAvplay->MasterFrmChn.hWindow, pAvplay->MasterFrmChn.hPort);
            if (MT_SUCCESS != Ret)
            {
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            pAvplay->MasterFrmChn.hWindow = MT_INVALID_HANDLE;
            pAvplay->MasterFrmChn.hPort = MT_INVALID_HANDLE;

            //look up another master window
            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                Ret = MT_MPI_WIN_GetInfo(pAvplay->SlaveFrmChn[i].hWindow, &WinInfo);
                if (MT_SUCCESS == Ret)
                {
                    if (MT_DRV_WIN_ACTIVE_SINGLE == WinInfo.eType)
                    {
                        break;
                    }
                }
            }

            //find it
            if (i<pAvplay->SlaveChnNum)
            {
                pAvplay->MasterFrmChn.hWindow = pAvplay->SlaveFrmChn[i].hWindow;
                pAvplay->MasterFrmChn.hPort = pAvplay->SlaveFrmChn[i].hPort;

                Ret = AVPLAY_SetPortAttr(pAvplay,pAvplay->MasterFrmChn.hPort, VDEC_PORT_TYPE_MASTER);
                if(MT_SUCCESS != Ret)
                {
                    MT_ERR_AVPLAY("ERR: set main port failed.\n");
                }

                pAvplay->SlaveFrmChn[i].hWindow = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow;
                pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow = MT_INVALID_HANDLE;
                pAvplay->SlaveFrmChn[i].hPort = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort;
                pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort = MT_INVALID_HANDLE;

                pAvplay->SlaveChnNum--;
            }
        }
        else
        {
            //look up another master window
            for (i=0; i<pAvplay->SlaveChnNum; i++)
            {
                if (pAvplay->SlaveFrmChn[i].hWindow == hWindow)
                {
                    break;
                }
            }

            if (i == pAvplay->SlaveChnNum)
            {
                MT_ERR_AVPLAY("ERR: this is not a attached master window.\n");
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            /*FATAL: after AVPLAY_DettachWinRelFrame, but AVPLAY_DestroyPort Failed*/
            Ret = AVPLAY_DestroyPort(pAvplay, hWindow, pAvplay->SlaveFrmChn[i].hPort);
            if (MT_SUCCESS != Ret)
            {
#ifdef AVPLAY_VID_THREAD
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
                AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
                MT_AVPLAY_INST_UNLOCK();
                return MT_ERR_AVPLAY_INVALID_OPT;
            }

            //find it
            if (i<pAvplay->SlaveChnNum)
            {
                pAvplay->SlaveFrmChn[i].hWindow = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow;
                pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hWindow = MT_INVALID_HANDLE;
                pAvplay->SlaveFrmChn[i].hPort = pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort;
                pAvplay->SlaveFrmChn[pAvplay->SlaveChnNum - 1].hPort = MT_INVALID_HANDLE;

                pAvplay->SlaveChnNum--;
            }
        }
    }
    /* analogous virtual window*/ /* ???? ??????*/
    else
    {
        for (i=0; i<pAvplay->VirChnNum; i++)
        {
            if (pAvplay->VirFrmChn[i].hWindow == hWindow)
            {
                break;
            }
        }

        if (i == pAvplay->VirChnNum)
        {
            MT_ERR_AVPLAY("ERR: this is not a attached master window.\n");
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = AVPLAY_DestroyPort(pAvplay, hWindow, pAvplay->VirFrmChn[i].hPort);
        if (MT_SUCCESS != Ret)
        {
#ifdef AVPLAY_VID_THREAD
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
            AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        pAvplay->VirFrmChn[i].hWindow = pAvplay->VirFrmChn[pAvplay->VirChnNum - 1].hWindow;
        pAvplay->VirFrmChn[pAvplay->VirChnNum - 1].hWindow = MT_INVALID_HANDLE;

        pAvplay->VirFrmChn[i].hPort = pAvplay->VirFrmChn[pAvplay->VirChnNum - 1].hPort;
        pAvplay->VirFrmChn[pAvplay->VirChnNum - 1].hPort = MT_INVALID_HANDLE;

        pAvplay->VirChnNum--;
    }


#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}


// TODO: ???ù?????????
mt_s32 MT_MPI_AVPLAY_SetWindowRepeat(mt_handle hAvplay, mt_u32 u32Repeat)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_u32               AvplayRatio;
    mt_s32               Ret;

    if (0 == u32Repeat)
    {
        MT_ERR_AVPLAY("para u32Repeat is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    AVPLAY_GET_INST_AND_LOCK();

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        MT_ERR_AVPLAY("AVPLAY has not attach master window.\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    AvplayRatio = 256/u32Repeat;

    if ((AvplayRatio > AVPLAY_ALG_FRC_MAX_PLAY_RATIO)
        || (AvplayRatio < AVPLAY_ALG_FRC_MIN_PLAY_RATIO))
    {
        MT_ERR_AVPLAY("Set repeat invalid!\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    pAvplay->FrcParamCfg.u32PlayRate = AvplayRatio;

	//FIXME: ineffectiveness in fact!

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_AttachSnd(mt_handle hAvplay, mt_handle hTrack)
{
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_DetachSnd(mt_handle hAvplay, mt_handle hTrack)
{
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetDmxAudChnHandle(mt_handle hAvplay, mt_handle *phDmxAudChn)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (!phDmxAudChn)
    {
        MT_ERR_AVPLAY("para phDmxAudChn is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->AvplayAttr.stStreamAttr.enStreamType != MT_UNF_AVPLAY_STREAM_TYPE_TS)
    {
        MT_ERR_AVPLAY("avplay is not ts stream mode.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (!pAvplay->hAdec)
    {
        MT_ERR_AVPLAY("aud chn is close.\n");
        MT_AVPLAY_INST_UNLOCK();
         return MT_ERR_AVPLAY_INVALID_OPT;
    }

    *phDmxAudChn = pAvplay->hDmxAud[pAvplay->CurDmxAudChn];

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetDmxVidChnHandle(mt_handle hAvplay, mt_handle *phDmxVidChn)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    if (!phDmxVidChn)
    {
        MT_ERR_AVPLAY("para phDmxVidChn is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->AvplayAttr.stStreamAttr.enStreamType != MT_UNF_AVPLAY_STREAM_TYPE_TS)
    {
        MT_ERR_AVPLAY("avplay is not ts stream mode.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    if (!pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("vid chn is close.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    *phDmxVidChn = pAvplay->hDmxVid;

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32                           Ret;
    ADEC_BUFSTATUS_S               AdecBufStatus = {0};
    VDEC_STATUSINFO_S              VdecBufStatus = {0};
    MT_MPI_DMX_BUF_STATUS_S        VidChnBuf = {0};
    mt_u32                         SndDelay = 0;
    MT_DRV_WIN_PLAY_INFO_S         WinPlayInfo = {0};
    MT_BOOL                        WinEnable;

    if (!pstStatusInfo)
    {
        MT_ERR_AVPLAY("para pstStatusInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    pstStatusInfo->enRunStatus = pAvplay->CurStatus;

    if (pAvplay->hAdec != MT_INVALID_HANDLE)
    {
        memset(&AdecBufStatus, 0, sizeof(AdecBufStatus));

        (mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_BUFFERSTATUS, &AdecBufStatus);
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize = AdecBufStatus.u32BufferSize;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize = AdecBufStatus.u32BufferUsed;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr = AdecBufStatus.u32BufWritePos;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr = (mt_u32)AdecBufStatus.s32BufReadPos;
		/**<CNcomment: ???????? ??VIDEO??Ч */
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].bEndOfStream = AdecBufStatus.bEndOfFrame;
        pstStatusInfo->u32AuddFrameCount = AdecBufStatus.u32TotDecodeFrame;
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize  = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr  = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr  = 0;
		/**<CNcomment: ???????? ??VIDEO??Ч */
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].bEndOfStream = MT_TRUE;
        pstStatusInfo->u32AuddFrameCount = 0;
    }

    if (pAvplay->hSyncTrack != MT_INVALID_HANDLE)
    {

        Ret = MT_MPI_AO_Track_GetDelayMs(pAvplay->hSyncTrack, &SndDelay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_HIAO_GetDelayMs failed:%x.\n",Ret);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime = SndDelay;

    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime = 0;
    }

    if (pAvplay->VidEnable)
    {
        Ret = MT_MPI_VDEC_GetChanStatusInfo(pAvplay->hVdec, &VdecBufStatus);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanStatusInfo failed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidChnBuf);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_DMX_GetPESBufferStatus failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = VidChnBuf.u32BufSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = VidChnBuf.u32UsedSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = VidChnBuf.u32BufWptr;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = VidChnBuf.u32BufRptr;
        }
        else
        {
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = VdecBufStatus.u32BufferSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = VdecBufStatus.u32BufferUsed;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = 0;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = 0;
        }

        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_VID;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufNum = VdecBufStatus.u32FrameBufNum;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = VdecBufStatus.bEndOfStream;

        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32VideoESFrameNumber= VdecBufStatus.u32VideoESFrameNumber;
        pstStatusInfo->u32VidFrameCount = VdecBufStatus.u32TotalDecFrmNum;
        pstStatusInfo->u32VidErrorFrameCount = VdecBufStatus.u32TotalErrFrmNum;
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_VID;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = MT_TRUE;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32VideoESFrameNumber= 0;
        pstStatusInfo->u32VidFrameCount = 0;
        pstStatusInfo->u32VidErrorFrameCount = 0;
    }

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        Ret = MT_MPI_WIN_GetEnable(pAvplay->MasterFrmChn.hWindow, &WinEnable);
        if ((Ret == MT_SUCCESS) && WinEnable)
        {
            Ret = MT_MPI_WIN_GetPlayInfo(pAvplay->MasterFrmChn.hWindow, &WinPlayInfo);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_WIN_GetPlayInfo failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
            else
            {
                pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = WinPlayInfo.u32DelayTime;
                if(0==WinPlayInfo.u32FrameNumInBufQn){
                    pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = MT_TRUE;
                }
            }
        }
        else
        {
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = 0;
        }
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = 0;
    }

    Ret = MT_MPI_SYNC_GetStatus(pAvplay->hSync, &pstStatusInfo->stSyncStatus);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_GetStatus failed.\n");
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

#if 0
    if(MT_INVALID_PTS !=  pstStatusInfo->stSyncStatus.u32LastAudPts)
    {
        if (pstStatusInfo->stSyncStatus.u32LastAudPts > pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime)
        {
            pstStatusInfo->stSyncStatus.u32LastAudPts -= pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime;
        }
        else
        {
            pstStatusInfo->stSyncStatus.u32LastAudPts = 0;
        }
    }

    if(MT_INVALID_PTS !=  pstStatusInfo->stSyncStatus.u32LastVidPts)
    {
        if (pstStatusInfo->stSyncStatus.u32LastVidPts > pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime)
        {
            pstStatusInfo->stSyncStatus.u32LastVidPts -= pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime;
        }
        else
        {
            pstStatusInfo->stSyncStatus.u32LastVidPts = 0;
        }
    }
#endif

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetCiTestInfo(mt_handle hAvplay, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo)
{
	mt_s32	Ret = MT_SUCCESS;
	AVPLAY_S	*pAvplay = MT_NULL;
	AVPLAY_USR_ADDR_S	AvplayUsrAddr;

	if (!pstInfo){
		MT_ERR_AVPLAY("para pstCiInfo is null.\n");
		return MT_ERR_AVPLAY_NULL_PTR;
	}

	AVPLAY_GET_INST_AND_LOCK();
	memset(pstInfo, 0, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S));

	if (MT_INVALID_HANDLE != pAvplay->hVdec){
		Ret = MT_MPI_VDEC_GetCiTestInfo(pAvplay->hVdec, pstInfo);
		if (MT_SUCCESS != Ret){
			MT_ERR_AVPLAY("call MT_MPI_VDEC_GetCiTestInfo failed.\n");
			MT_AVPLAY_INST_UNLOCK();
			return Ret;
		}
	}
	Ret = MT_MPI_AO_Track_GetCiTestInfo(pAvplay->hTrack[0], pstInfo);
	if (MT_SUCCESS != Ret){
		MT_ERR_AVPLAY("call MT_MPI_AO_Track_GetCiTestInfo failed.\n");
		MT_AVPLAY_INST_UNLOCK();
		return Ret;
	}

	MT_AVPLAY_INST_UNLOCK();
	return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetStreamInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STREAM_INFO_S *pstStreamInfo)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret = MT_SUCCESS;
    ADEC_STREAMINFO_S       AdecStreaminfo = {0};

    if (!pstStreamInfo)
    {
        MT_ERR_AVPLAY("para pstStreamInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->hAdec != MT_INVALID_HANDLE)
    {
        Ret = MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_STREAMINFO, &AdecStreaminfo);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_ADEC_GetInfo failed.\n");
        }
        else
        {
            pstStreamInfo->stAudStreamInfo.enACodecType = AdecStreaminfo.u32CodecID;
            pstStreamInfo->stAudStreamInfo.enSampleRate = AdecStreaminfo.enSampleRate;
            pstStreamInfo->stAudStreamInfo.enBitDepth = MT_UNF_BIT_DEPTH_16;
            pstStreamInfo->stAudStreamInfo.u32Channel = 0;	//unknown
        }
    }

    if (pAvplay->hVdec != MT_INVALID_HANDLE)
    {
        Ret = MT_MPI_VDEC_GetChanStreamInfo(pAvplay->hVdec, &(pstStreamInfo->stVidStreamInfo));
		if(pAvplay->AvplayAttr.stStreamAttr.enStreamType ==  MT_UNF_AVPLAY_STREAM_TYPE_TS)
		{
			if(pAvplay->DebugInfo.AcquiredVidFrameNum < 1)
			{
				memset( &(pstStreamInfo->stVidStreamInfo), 0, sizeof(MT_UNF_VCODEC_STREAMINFO_S));
				Ret = MT_FAILURE;
			}
		}
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanStreamInfo failed.\n");
        }
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetAudioSpectrum(mt_handle hAvplay, mt_u16 *pSpectrum, mt_u32 u32BandNum)
{

    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret = MT_SUCCESS;

    if (!pSpectrum)
    {
        MT_ERR_AVPLAY("para pSpectrum is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (!pAvplay->AudEnable)
    {
        MT_ERR_AVPLAY("aud chn is stopped.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_ADEC_GetAudSpectrum(pAvplay->hAdec,  pSpectrum , u32BandNum);
    if(MT_SUCCESS != Ret)
    {
        MT_WARN_AVPLAY("WARN: MT_MPI_ADEC_GetAudSpectrum.\n");
    }

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

/* add for user to get buffer state, user may want to check if buffer is empty,
    but NOT want to block the user's thread. then user can use this API to check the buffer state
    by q46153 */
mt_s32 MT_MPI_AVPLAY_IsBuffEmpty(mt_handle hAvplay, MT_BOOL *pbIsEmpty)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret;

    if (!pbIsEmpty)
    {
        MT_ERR_AVPLAY("para pbIsEmpty is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    *pbIsEmpty = MT_FALSE;
    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->bSetEosFlag)
    {
        if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
        {
            *pbIsEmpty = MT_TRUE;
            pAvplay->CurBufferEmptyState = MT_TRUE;
        }
        else
        {
            *pbIsEmpty = MT_FALSE;
            pAvplay->CurBufferEmptyState = MT_FALSE;
        }
    }
    else
    {
        *pbIsEmpty = AVPLAY_IsBufEmpty(pAvplay);
    }

    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}


/* for DDP test only! call this before MT_UNF_AVPLAY_ChnOpen */
mt_s32 MT_MPI_AVPLAY_SetDDPTestMode(mt_handle hAvplay, MT_BOOL bEnable)
{
    AVPLAY_S              *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S     AvplayUsrAddr;
    mt_s32                Ret;

    AVPLAY_GET_INST_AND_LOCK();

    pAvplay->AudDDPMode = bEnable;
    pAvplay->LastAudPts = 0;

    Ret = MT_MPI_SYNC_SetDDPTestMode(pAvplay->hSync, pAvplay->AudDDPMode);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("Set SYNC in ddp test mode error:%x.\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    Ret = MT_MPI_ADEC_SetDDPTestMode(pAvplay->hAdec, pAvplay->AudDDPMode);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("Set ADEC in ddp test mode error:%x.\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_SwitchDmxAudChn(mt_handle hAvplay, mt_handle hNewDmxAud, mt_handle *phOldDmxAud)
{
    AVPLAY_S              *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S     AvplayUsrAddr;
    mt_s32                Ret;

    if ((!hAvplay) || (!hNewDmxAud) || (MT_NULL == phOldDmxAud))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_Mutex_Lock(pAvplay->pAvplayThreadMutex);

    /* if the es buf has not been released */
    if(pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC])
    {
        (mt_void)MT_MPI_DMX_ReleaseEs(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &pAvplay->AvplayDmxEsBuf);
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_DMX_ADEC] = MT_FALSE;
    }

    *phOldDmxAud = pAvplay->hDmxAud[pAvplay->CurDmxAudChn];
    pAvplay->hDmxAud[pAvplay->CurDmxAudChn] = hNewDmxAud;

    AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}

/* add for Flashplayer adjust pts */
mt_s32 MT_MPI_AVPLAY_PutAudPts(mt_handle hAvplay, mt_u32 u32AudPts)
{
    AVPLAY_S              *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S     AvplayUsrAddr;
    mt_s32                Ret;

    if ((!hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    pAvplay->AudInfo.SrcPts = u32AudPts;
    pAvplay->AudInfo.Pts = u32AudPts;

    pAvplay->AudInfo.BufTime = 0;
    pAvplay->AudInfo.FrameNum = 0;
    pAvplay->AudInfo.FrameTime = 5000;

   MT_ERR_AVPLAY("Jack %s: %d\n", __FUNCTION__, __LINE__);
    //AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
   //  Ret = MT_MPI_SYNC_AudJudge(pAvplay->hSync, &pAvplay->AudInfo, &pAvplay->AudOpt);
    //AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);

    MT_AVPLAY_INST_UNLOCK();

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_FlushStream(mt_handle hAvplay, MT_UNF_AVPLAY_FLUSH_STREAM_OPT_S *pstFlushOpt)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;

    if ((!hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    AVPLAY_Mutex_Lock(pAvplay->pAvplayThreadMutex);
#ifdef AVPLAY_VID_THREAD
    AVPLAY_Mutex_Lock(pAvplay->pAvplayVidThreadMutex);
#endif

    if (MT_UNF_AVPLAY_STATUS_EOS == pAvplay->CurStatus)
    {
        #if 0
        MT_INFO_AVPLAY("current status is eos!\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_Mutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
        #else
        //should can call AVPLAY_SetEosFlag again for Bug #23853 #27343
        MT_INFO_AVPLAY("current status already eos!\n");
        #endif
    }

    if (pAvplay->bSetEosFlag)
    {
        MT_INFO_AVPLAY("Eos Flag has been set!\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_Mutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
        AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
        MT_AVPLAY_INST_UNLOCK();
        return MT_SUCCESS;
    }

	if(pAvplay->trickmode != TM_FREV)
	{
	    Ret = AVPLAY_SetEosFlag(pAvplay);
	    if (MT_SUCCESS != Ret)
	    {
	        MT_ERR_AVPLAY("ERR: AVPLAY_SetEosFlag, Ret = %x\n", Ret);
#ifdef AVPLAY_VID_THREAD
	        AVPLAY_Mutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
	        AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
	        MT_AVPLAY_INST_UNLOCK();
	        return Ret;
	    }
	}


#ifdef AVPLAY_VID_THREAD
    AVPLAY_Mutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#endif
    AVPLAY_Mutex_UnLock(pAvplay->pAvplayThreadMutex);
    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Step(mt_handle hAvplay, const MT_UNF_AVPLAY_STEP_OPT_S *pstStepOpt)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;

    if ((!hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);
#endif

    if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
    {
        MT_ERR_AVPLAY("AVPLAY has not attach master window.\n");
#ifdef AVPLAY_VID_THREAD
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
        AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    pAvplay->bStepMode = MT_TRUE;
    pAvplay->bStepPlay = MT_TRUE;

#ifdef AVPLAY_VID_THREAD
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayVidThreadMutex);
#else
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
#endif

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Invoke(mt_handle hAvplay, MT_UNF_AVPLAY_INVOKE_E enInvokeType, mt_void *pPara)
{
    mt_s32                                  Ret;
    AVPLAY_S                                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S                       AvplayUsrAddr;
    MT_DRV_VIDEO_FRAME_S                    stVidFrame;
    MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S     stPlayInfo;
    MT_DRV_VIDEO_PRIVATE_S                  stVidPrivate;
    ulong                                  bUseStopRegion = MT_TRUE;

    if (enInvokeType >= MT_UNF_AVPLAY_INVOKE_BUTT)
    {
        MT_ERR_AVPLAY("para enInvokeType is invalid.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (!pPara)
    {
        MT_ERR_AVPLAY("para pPara is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    memset(&stVidFrame, 0x0, sizeof(MT_DRV_VIDEO_FRAME_S));
    memset(&stPlayInfo, 0x0, sizeof(MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S));
    memset(&stVidPrivate, 0x0, sizeof(MT_DRV_VIDEO_PRIVATE_S));

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_UNF_AVPLAY_INVOKE_VCODEC == enInvokeType)
    {
        if (MT_INVALID_HANDLE == pAvplay->hVdec)
        {
            MT_ERR_AVPLAY("vid chn is close, can not set vcodec cmd.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_VDEC_Invoke(pAvplay->hVdec, pPara);
        if (Ret != MT_SUCCESS)
        {
            MT_WARN_AVPLAY("MT_MPI_VDEC_Invoke failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_INVOKE_ACODEC == enInvokeType)
    {
        if (MT_INVALID_HANDLE == pAvplay->hAdec)
        {
            MT_ERR_AVPLAY("aud chn is close, can not set acodec cmd.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_ADEC_SetCodecCmd(pAvplay->hAdec, pPara);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("ADEC_SetCodecCmd failed.\n");
        }
    }
    else if (MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO == enInvokeType)
    {
    	//Feature is not implement for WinBuf_GetDisplayedFrame not implement!
    	MT_WARN_AVPLAY("%s: Warning, INVOKE_GET_PRIV_PLAYINFO - feature not implement!\n",__FUNCTION__);
        if (MT_INVALID_HANDLE == pAvplay->MasterFrmChn.hWindow)
        {
            MT_ERR_AVPLAY("AVPLAY has not attach master window.\n");
            MT_AVPLAY_INST_UNLOCK();
            return MT_ERR_AVPLAY_INVALID_OPT;
        }

        Ret = MT_MPI_WIN_GetLatestFrameInfo(pAvplay->MasterFrmChn.hWindow, &stVidFrame);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("MT_MPI_WIN_GetLatestFrameInfo failed.\n");
        }

        stPlayInfo.u64LastPts = stVidFrame.slotInfo.pts;

        memcpy(&stVidPrivate, (MT_DRV_VIDEO_PRIVATE_S *)(stVidFrame.u32Priv), sizeof(MT_DRV_VIDEO_PRIVATE_S));

        stPlayInfo.u32LastPlayTime = stVidPrivate.u32PrivDispTime;

        stPlayInfo.u32DispOptimizeFlag = pAvplay->u32DispOptimizeFlag;

        memcpy((MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S *)pPara, &stPlayInfo, sizeof(MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S));
    }
    else if (MT_UNF_AVPLAY_INVOKE_SET_DISP_OPTIMIZE_FLAG == enInvokeType)
    {
        pAvplay->u32DispOptimizeFlag = *(mt_u32 *)pPara;
    }
    else if (MT_UNF_AVPLAY_INVOKE_SET_SYNC_MODE == enInvokeType)
    {
        if (*(mt_u32 *)pPara == 1)
        {
            bUseStopRegion = MT_FALSE;
            (mt_void)MT_MPI_SYNC_SetExtInfo(pAvplay->hSync, SYNC_EXT_INFO_STOP_REGION, (mt_void *)bUseStopRegion);
        }
    }

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}


mt_s32 MT_MPI_AVPLAY_AcqUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S *pstUserData, MT_UNF_VIDEO_USERDATA_TYPE_E *penType)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;

    if ((MT_INVALID_HANDLE == hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (!pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("Vid chan is not start.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_AcqUserData(pAvplay->hVdec, pstUserData, penType);
    if (MT_SUCCESS != Ret)
    {
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_RlsUserData(mt_handle hAvplay, MT_UNF_VIDEO_USERDATA_S* pstUserData)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;

    if ((MT_INVALID_HANDLE == hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (!pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("Vid chan is not start.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_RlsUserData(pAvplay->hVdec, pstUserData);
    if (MT_SUCCESS != Ret)
    {
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_RstUserDataBuffer(mt_handle hAvplay)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;

    if ((MT_INVALID_HANDLE == hAvplay))
    {
        MT_ERR_AVPLAY("para is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (!pAvplay->VidEnable)
    {
        MT_ERR_AVPLAY("Vid chan is not start.\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_RstUserDataBuffer(pAvplay->hVdec);
    if (MT_SUCCESS != Ret)
    {
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetVidChnOpenParam(mt_handle hAvplay, MT_UNF_AVPLAY_OPEN_OPT_S *pstOpenPara)
{
    mt_s32                  Ret;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr = {0, 0};

    if (MT_NULL == pstOpenPara)
    {
        MT_ERR_AVPLAY("pstOpenPara is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (MT_INVALID_HANDLE == pAvplay->hVdec)
    {
        MT_ERR_AVPLAY("Vid Chan is not open!\n");
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    Ret = MT_MPI_VDEC_GetChanOpenParam(pAvplay->hVdec, pstOpenPara);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("MT_MPI_VDEC_GetChanOpenParam ERR, Ret=%x\n", Ret);
        MT_AVPLAY_INST_UNLOCK();
        return MT_ERR_AVPLAY_INVALID_OPT;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_UseExternalBuffer(mt_handle hAvplay, mt_handle* phBuffers, mt_u32 u32Cnt, mt_u32 u32Size)
{
    mt_s32                  Ret;
    mt_u32                  i;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr = {0, 0};
    VDEC_BUFFER_ATTR_S      stVdecAttr;

    if (MT_NULL == phBuffers || 0 == u32Cnt)
    {
        MT_ERR_AVPLAY("invalid external buffer\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);

    stVdecAttr.u32BufNum = u32Cnt;
    stVdecAttr.u32Stride = 0;
    stVdecAttr.u32BufSize = u32Size;

    for (i = 0; i < u32Cnt; i++)
    {
        stVdecAttr.u32UsrVirAddr[i] = 0;
        stVdecAttr.u32PhyAddr[i] = phBuffers[i];
    }

    Ret = MT_MPI_VDEC_SetExternBufferState(pAvplay->hVdec, VDEC_EXTBUFFER_STATE_STOP);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("stop external buffer manager failed");
    }
    Ret = MT_MPI_VDEC_SetExternBuffer(pAvplay->hVdec, &stVdecAttr);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("Set vdec external buffer failed");
        goto out;
    }

    Ret = MT_MPI_VDEC_SetExternBufferState(pAvplay->hVdec, VDEC_EXTBUFFER_STATE_START);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("stop external buffer manager failed");
    }
out:
    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
    AVPLAY_Mutex_UnLock(pAvplay->pAvplayMutex);
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_DeleteExternalBuffer(mt_handle hAvplay,mt_handle* phBuffers, mt_u32 u32Cnt)
{
    mt_s32                  Ret;
    mt_u32                  i;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr = {0, 0};
    VDEC_FRAMEBUFFER_STATE_E state = VDEC_BUF_STATE_BUTT;
    mt_s32                  WaitTime = 0;

    if (MT_NULL == phBuffers || 0 == u32Cnt)
    {
        MT_ERR_AVPLAY("invalid external buffer\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();
    AVPLAY_ThreadMutex_Lock(pAvplay->pAvplayThreadMutex);

    Ret = MT_MPI_VDEC_SetExternBufferState(pAvplay->hVdec, VDEC_EXTBUFFER_STATE_STOP);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("stop external buffer manager failed");
    }
    /* may be only stop vidchannel,avoid there is frame at avplay, when stop avplay, we drop this frame*/
    if (MT_TRUE == pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO])
    {
        /*Release vpss frame*/
        (mt_void)AVPLAY_RelAllChnFrame(pAvplay);
        pAvplay->AvplayProcDataFlag[AVPLAY_PROC_VDEC_VO] = MT_FALSE;
        MT_INFO_AVPLAY("release avplay frame success");
    }

    /* release frame in virtual window */
    for (i=0; i<pAvplay->VirChnNum; i++)
    {
        Ret = MT_MPI_WIN_Reset(pAvplay->VirFrmChn[i].hWindow, MT_DRV_WIN_SWITCH_BLACK);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("reset window failed");
        }
    }

    for (i = 0; i < u32Cnt; i++)
    {
        WaitTime = 0;
        while (WaitTime < 50)
        {
            Ret = MT_MPI_VDEC_CheckAndDeleteExtBuffer(pAvplay->hVdec, phBuffers[i], &state);
            if (Ret != MT_SUCCESS || (state == VDEC_BUF_STATE_IN_USE))
            {
                MT_ERR_AVPLAY("delete buffer %x from vdec failed, state:%d, Ret:%d\n", phBuffers[i], state, Ret);
                MT_USLEEP(1000*10);
                WaitTime++;
                continue;
            }
            MT_INFO_AVPLAY("delete buffer %x from vdec successed, state:%d, Ret:%d\n", phBuffers[i], state, Ret);
            break;
        }
    }
    Ret = MT_MPI_VDEC_SetExternBufferState(pAvplay->hVdec, VDEC_EXTBUFFER_STATE_START);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("start external buffer manager failed");
    }

    AVPLAY_ThreadMutex_UnLock(pAvplay->pAvplayThreadMutex);
    AVPLAY_Mutex_UnLock(pAvplay->pAvplayMutex);

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_CalculateFRC(mt_handle hAvplay, MT_UNF_VIDEO_FRAME_INFO_S* pstFrame,
        mt_u32 u32RefreshRate, mt_s32* ps32RepeatCnt)
{
    mt_s32                  Ret = MT_SUCCESS;
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr = {0, 0};
    MT_UNF_AVPLAY_FRMRATE_PARAM_S       stFrameRate;

    if (MT_NULL == pstFrame || NULL == ps32RepeatCnt)
    {
        MT_ERR_AVPLAY("invalid parameter \n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    memset(&stFrameRate, 0, sizeof(MT_UNF_AVPLAY_FRMRATE_PARAM_S));
    MT_MPI_VDEC_GetChanFrmRate(pAvplay->hVdec, &stFrameRate);

    if (stFrameRate.enFrmRateType == MT_UNF_AVPLAY_FRMRATE_TYPE_USER)
    {
        if ((MT_UNF_VIDEO_FIELD_TOP == pstFrame->enFieldMode)
            || (MT_UNF_VIDEO_FIELD_BOTTOM == pstFrame->enFieldMode))
        {
            pAvplay->FrcParamCfg.u32InRate = (stFrameRate.stSetFrmRate.u32fpsInteger * 100
                                         + stFrameRate.stSetFrmRate.u32fpsDecimal / 10) * 2;
        }
        else
        {
            pAvplay->FrcParamCfg.u32InRate = stFrameRate.stSetFrmRate.u32fpsInteger * 100
                                         + stFrameRate.stSetFrmRate.u32fpsDecimal / 10;
        }
    }
    else
    {
        pAvplay->FrcParamCfg.u32InRate = pstFrame->stFrameRate.u32fpsInteger * 100
                                        + pstFrame->stFrameRate.u32fpsDecimal /10;
    }

    pAvplay->FrcParamCfg.u32OutRate = u32RefreshRate;

    /*do frc for every the frame*/
    (mt_void)AVPLAY_FrcCalculate(&pAvplay->FrcCalAlg, &pAvplay->FrcParamCfg, &pAvplay->FrcCtrlInfo);

    /* sometimes(such as pvr smooth tplay), vdec set u32PlayTime, means this frame must repeat */
    *ps32RepeatCnt = (1 + pAvplay->FrcCtrlInfo.s32FrmState);// * (1 + pstVideoPriv->u32PlayTime);

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetAudioStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32                           Ret;
    ADEC_BUFSTATUS_S               AdecBufStatus = {0};
    mt_u32                         SndDelay = 0;

    if (!pstStatusInfo)
    {
        MT_ERR_AVPLAY("para pstStatusInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    pstStatusInfo->enRunStatus = pAvplay->CurStatus;

    if (pAvplay->hAdec != MT_INVALID_HANDLE)
    {
        memset(&AdecBufStatus, 0, sizeof(AdecBufStatus));

        (mt_void)MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_BUFFERSTATUS, &AdecBufStatus);
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize = AdecBufStatus.u32BufferSize;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize = AdecBufStatus.u32BufferUsed;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr = AdecBufStatus.u32BufWritePos;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr = (mt_u32)AdecBufStatus.s32BufReadPos;
		/**<CNcomment: ???????? ??VIDEO??Ч */
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].bEndOfStream = AdecBufStatus.bEndOfFrame;
        pstStatusInfo->u32AuddFrameCount = AdecBufStatus.u32TotDecodeFrame;

		if(MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
		{
			MT_MPI_DMX_BUF_STATUS_S 		   AudChnBuf = {0};
			MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxAud[pAvplay->CurDmxAudChn], &AudChnBuf);
			pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize += AudChnBuf.u32BufSize;
			pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize += AudChnBuf.u32UsedSize;

		}
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize  = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr  = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr  = 0;
		/**<CNcomment: ???????? ??VIDEO??Ч */
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].bEndOfStream = MT_TRUE;
        pstStatusInfo->u32AuddFrameCount = 0;
    }

    if (pAvplay->hSyncTrack != MT_INVALID_HANDLE)
    {

        Ret = MT_MPI_AO_Track_GetDelayMs(pAvplay->hSyncTrack, &SndDelay);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_HIAO_GetDelayMs failed:%x.\n",Ret);
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime = SndDelay;

    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32FrameBufTime = 0;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetVideoStatusInfo(mt_handle hAvplay, MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32                           Ret;
    VDEC_STATUSINFO_S              VdecBufStatus = {0};
    MT_MPI_DMX_BUF_STATUS_S        VidChnBuf = {0};
    MT_DRV_WIN_PLAY_INFO_S         WinPlayInfo = {0};
    MT_BOOL                        WinEnable;

    if (!pstStatusInfo)
    {
        MT_ERR_AVPLAY("para pstStatusInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    pstStatusInfo->enRunStatus = pAvplay->CurStatus;

    if (pAvplay->VidEnable)
    {
         Ret = MT_MPI_VDEC_GetChanStatusInfo(pAvplay->hVdec, &VdecBufStatus);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_AVPLAY("call MT_MPI_VDEC_GetChanStatusInfo failed.\n");
            MT_AVPLAY_INST_UNLOCK();
            return Ret;
        }

        if (MT_UNF_AVPLAY_STREAM_TYPE_TS == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
        {
            Ret = MT_MPI_DMX_GetPESBufferStatus(pAvplay->hDmxVid, &VidChnBuf);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_DMX_GetPESBufferStatus failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }

            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = VidChnBuf.u32BufSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = VidChnBuf.u32UsedSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = VidChnBuf.u32BufWptr;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = VidChnBuf.u32BufRptr;
        }
        else
        {
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = VdecBufStatus.u32BufferSize;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = VdecBufStatus.u32BufferUsed;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = 0;
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = 0;
        }

        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_VID;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufNum = VdecBufStatus.u32FrameBufNum;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = VdecBufStatus.bEndOfStream;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32VideoESFrameNumber= VdecBufStatus.u32VideoESFrameNumber;
        pstStatusInfo->u32VidFrameCount = VdecBufStatus.u32TotalDecFrmNum;
        pstStatusInfo->u32VidErrorFrameCount = VdecBufStatus.u32TotalErrFrmNum;
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufId = MT_UNF_AVPLAY_BUF_ID_ES_VID;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufNum = 0;
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = MT_TRUE;
        pstStatusInfo->u32VidFrameCount = 0;
        pstStatusInfo->u32VidErrorFrameCount = 0;
    }

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        Ret = MT_MPI_WIN_GetEnable(pAvplay->MasterFrmChn.hWindow, &WinEnable);
        if ((Ret == MT_SUCCESS) && WinEnable)
        {
            Ret = MT_MPI_WIN_GetPlayInfo(pAvplay->MasterFrmChn.hWindow, &WinPlayInfo);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_AVPLAY("call MT_MPI_WIN_GetPlayInfo failed.\n");
                MT_AVPLAY_INST_UNLOCK();
                return Ret;
            }
            else
            {
                pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = WinPlayInfo.u32DelayTime;
                if(0==WinPlayInfo.u32FrameNumInBufQn){
                    pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream = MT_TRUE;
                }
            }
        }
        else
        {
            pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = 0;
        }
    }
    else
    {
        pstStatusInfo->stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32FrameBufTime = 0;
    }

    MT_AVPLAY_INST_UNLOCK();
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_GetSyncStatusInfo(mt_handle          hAvplay,
                                 MT_UNF_AVPLAY_STATUS_INFO_S *pstStatusInfo)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32                         Ret;

    if (!pstStatusInfo)
    {
        MT_ERR_AVPLAY("para pstStatusInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_SYNC_GetStatus(pAvplay->hSync, &pstStatusInfo->stSyncStatus);

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetDebugInfo(mt_handle hAvplay, MT_UNF_AVPLAY_DEBUG_INFO_S *pstDebugInfo)
{
	AVPLAY_S						*pAvplay = MT_NULL;
	AVPLAY_USR_ADDR_S				AvplayUsrAddr;
	mt_s32							Ret;

	if (!pstDebugInfo)
	{
		MT_ERR_AVPLAY("para pstDebugInfo is null.\n");
		return MT_ERR_AVPLAY_NULL_PTR;
	}

	AVPLAY_GET_INST_AND_LOCK();

	if (pAvplay->hAdec != MT_INVALID_HANDLE)
	{
	    memset(&stTrackInfo, 0x0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
	    Ret = MT_MPI_AO_Track_GetAttr(pAvplay->hTrack[0], &stTrackInfo);
	    if (MT_SUCCESS != Ret){
	        MT_AVPLAY_INST_UNLOCK();
	        return MT_FAILURE;
	    }
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32Size = stTrackInfo.u32DebugCrcSize;//pAvplay->DebugInfoExt.u32AudCrcBufSize;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32PhyAddr = stTrackInfo.u32DebugCrc_phy;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsrVirAddr = stTrackInfo.u32DebugCrc_vir;//(mt_u32)pAvplay->DebugInfoExt.pu32AudCrcBuf;
	}
	else
	{
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32Size = 0;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32PhyAddr = 0;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsrVirAddr = 0;
	}

    if (pAvplay->hVdec != MT_INVALID_HANDLE)
	{
		MT_CODEC_VIDEO_CMD_S stVdecCmd;
		MT_UNF_AVPLAY_CRC_BUF_S stVdecCrcBuf;

		stVdecCmd.u32CmdID = VFMW_CMD_GET_CRCBUF;
		stVdecCmd.pPara = (mt_void*)&stVdecCrcBuf;

        Ret = MT_MPI_VDEC_Invoke(pAvplay->hVdec, &stVdecCmd);

		if (Ret == MT_SUCCESS)
		{
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32Size = stVdecCrcBuf.u32Size;
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32PhyAddr = stVdecCrcBuf.u32PhyAddr;
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsrVirAddr = stVdecCrcBuf.u32UsrVirAddr;
		}
		else
		{
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32Size = 0;
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32PhyAddr = 0;
			pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsrVirAddr = 0;
		}
	}
	else
	{
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32Size = 0;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32PhyAddr = 0;
		pstDebugInfo->stCrcBuf[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsrVirAddr = 0;
	}

	MT_AVPLAY_INST_UNLOCK();
	return MT_SUCCESS;
}

mt_s32 MT_MPI_AVPLAY_AudioTrack(mt_handle hAvplay, mt_void *ptrack)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32                         Ret = MT_SUCCESS;

    if (!ptrack)
    {
        MT_ERR_AVPLAY("para ptrack is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_SYNC_Aud_Track(pAvplay->hSync, ptrack);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("%s failed. Ret = %x\n", __func__, Ret);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_EnableAVOutInfo(mt_handle hAvplay, MT_BOOL bEnable)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S               AvplayUsrAddr;
    mt_s32                          Ret = MT_SUCCESS;

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_SYNC_VFRM_INFO_CAP_Enable(pAvplay->hSync, bEnable);
    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("%s failed. Ret = %x\n", __func__, Ret);
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetAVOutInfo(mt_handle hAvplay, mt_void *pAVOutInfo)
{
    AVPLAY_S                       *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S              AvplayUsrAddr;
    mt_s32  Ret = MT_SUCCESS;

    if (!pAVOutInfo)
    {
        MT_ERR_AVPLAY("%s para is null.\n", __func__);
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_SYNC_VFRM_INFO_CAP_Read(pAvplay->hSync, (SYNC_VOUT_FRAME_INFO *)pAVOutInfo);
    if (MT_SUCCESS != Ret)
    {
        MT_AVPLAY_INST_UNLOCK();
        return Ret;
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 AVPLAY_SetDmxBufFullCare(AVPLAY_S *pAvplay, MT_UNF_DMX_BUF_FULL_CARE_S *pDmxBufFullCare)
{
    MT_UNF_DMX_BUF_FULL_CARE_S    DmxBufFullCare;
    mt_u32  i = 0;
    mt_s32  Ret = MT_SUCCESS;

    if (!pAvplay || !pDmxBufFullCare)
    {
        MT_ERR_AVPLAY("%s para is null.\n", __func__);
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    memset(&DmxBufFullCare, 0, sizeof(DmxBufFullCare));
    DmxBufFullCare.pid = pDmxBufFullCare->pid;
    DmxBufFullCare.es_buf_full = pDmxBufFullCare->es_buf_full;
    DmxBufFullCare.dsc_buf_full = pDmxBufFullCare->dsc_buf_full;
    DmxBufFullCare.sec_buf_full = pDmxBufFullCare->sec_buf_full;

    if (DmxBufFullCare.pid == pAvplay->DmxVidPid)
    {
        Ret = MT_MPI_DMX_BufFullCareSet(pAvplay->hDmxVid, &DmxBufFullCare);
    }
    else
    {
        for (i = 0; i < pAvplay->DmxAudChnNum; i++)
        {
            if (DmxBufFullCare.pid == pAvplay->DmxAudPid[i])
            {
                Ret = MT_MPI_DMX_BufFullCareSet(pAvplay->hDmxAud[i], &DmxBufFullCare);
                break;
            }
        }

        if (i >= pAvplay->DmxAudChnNum)
        {
            MT_ERR_AVPLAY("apid: %d is not set\n", DmxBufFullCare.pid);
        }
    }

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("SetChannelAttr failed, ret:%x\n", Ret);
    }

    return Ret;
}

mt_s32 AVPLAY_GetAVSyncInfo(AVPLAY_S *pAvplay, mt_void *pAVSynInfo)
{
    mt_s32  Ret = MT_SUCCESS;

    if (!pAvplay || !pAVSynInfo)
    {
        MT_ERR_AVPLAY("%s para is null.\n", __func__);
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    Ret = MT_MPI_SYNC_GetAVSyncInfo(pAvplay->hSync, (MT_UNF_SYNC_AV_INFO_S *)pAVSynInfo);

    if (MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("%s failed. Ret = %x\n", __func__, Ret);
        return Ret;
    }

    return Ret;
}

mt_s32 MT_MPI_AVPLAY_Enable_AudHEAAC(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

	Ret = MT_MPI_ADEC_Enable_HEAAC(pAvplay->hAdec);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_Enable_HEAAC failed.\n");
	}

	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}

mt_s32 MT_MPI_AVPLAY_set_downmix_enable(mt_handle hAvplay, u32 enable)
{
	AVPLAY_S		   *pAvplay = MT_NULL;
	AVPLAY_USR_ADDR_S  AvplayUsrAddr;
	mt_s32				 Ret;

	AVPLAY_GET_INST_AND_LOCK();

	Ret = MT_MPI_ADEC_set_downmix_enable(pAvplay->hAdec, enable);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_set_downmix_enable failed.\n");
	}

	MT_AVPLAY_INST_UNLOCK();
	return Ret;
}

mt_s32 MT_MPI_AVPLAY_TrickSeekIn(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_DMX_TrickSeekIn(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_AVPLAY_TrickSeekIn failed.\n");
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_TrickSeekOut(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

    Ret = MT_MPI_DMX_TrickSeekOut(pAvplay->hDmxAud[pAvplay->CurDmxAudChn]);
	Ret = MT_MPI_ADEC_set_trickmode(pAvplay->hAdec, 0);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_AVPLAY_TrickSeekOut failed.\n");
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_SW_ADEC_ENABLE(mt_handle hAvplay)
{
	AVPLAY_S			*pAvplay = MT_NULL;
	AVPLAY_USR_ADDR_S	AvplayUsrAddr;
	mt_s32				Ret;
	MT_S32 				retval = MT_SUCCESS;
	mt_handle			hdmx_aud;
	MT_UNF_DMX_CHAN_ATTR_S dmx_attr;

	retval = MT_MPI_AVPLAY_GetDmxAudChnHandle(hAvplay, &hdmx_aud);
	if(retval != MT_SUCCESS) {
		MT_ERR_AVPLAY("MT_UNF_AVPLAY_GetDmxAudChnHandle fail\n");
		retval = MT_FAILURE;
    }

	retval = MT_MPI_DMX_GetChannelAttr(hdmx_aud, &dmx_attr);
	if(retval != MT_SUCCESS) {
		MT_ERR_AVPLAY("MT_UNF_DMX_GetChannelAttr fail\n");
		retval = MT_FAILURE;
    }

	dmx_attr.bEsToUser = MT_TRUE;
	retval = MT_MPI_DMX_SetChannelAttr(hdmx_aud, &dmx_attr);
	if(retval != MT_SUCCESS) {
		MT_ERR_AVPLAY("MT_UNF_DMX_GetChannelAttr fail\n");
		retval = MT_FAILURE;
    }

    AVPLAY_GET_INST_AND_LOCK();

	pAvplay->enable_sw_adec = 1;

    MT_AVPLAY_INST_UNLOCK();

    return retval;
}

//only for extern video decoder
mt_s32 MT_MPI_AVPLAY_EnableVdecSync(mt_handle hAvplay)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret;

    AVPLAY_GET_INST_AND_LOCK();

    MT_MPI_SYNC_Vid_Init(pAvplay->hSync, (mt_u32)0);

	Ret = MT_MPI_SYNC_Start(pAvplay->hSync, SYNC_CHAN_VID);

    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("call MT_MPI_SYNC_Vid_Init failed.\n");
    }

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetVideoESPhyAddrr(mt_handle hAvplay, phys_addr_t *esBuffPhyAddr, mt_u32 *esBuffSize)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret = MT_SUCCESS;

    AVPLAY_GET_INST_AND_LOCK();
	Ret = MT_MPI_VDEC_ChanGetEsBuffer(pAvplay->hVdec, esBuffPhyAddr, esBuffSize);
	if (Ret != MT_SUCCESS)
	{
		MT_ERR_AVPLAY("call MT_MPI_ADEC_PutBuffer failed.\n");
	}

    MT_AVPLAY_INST_UNLOCK();
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_ListAllPlayer(MT_UNF_AVPLAY_PLAYERINFO_S * info)
{
    mt_s32               Ret = MT_SUCCESS;


    if (g_AvplayDevFd < 0)
    {
        MT_ERR_AVPLAY("AVPLAY is not init.\n");
        return MT_ERR_AVPLAY_DEV_NO_INIT;
    }

    Ret = ioctl(g_AvplayDevFd, CMD_AVPLAY_GET_PLAYERINFO, info);
    if (Ret != MT_SUCCESS)
    {
        MT_ERR_AVPLAY("AVPLAY CMD_AVPLAY_GET_PLAYERINFO failed.\n");
    }
    return Ret;
}

mt_s32 MT_MPI_AVPLAY_GetMetaInfo(mt_handle hAvplay, MT_UNF_AVPLAY_METARINFO_S *pMetaInfo)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret = MT_SUCCESS;

    if (!pMetaInfo) {
        MT_ERR_AVPLAY("para pMetaInfo is null.\n");
        return MT_ERR_AVPLAY_NULL_PTR;
    }

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->hAdec != MT_INVALID_HANDLE) {
        Ret = MT_MPI_ADEC_GetInfo(pAvplay->hAdec, MT_MPI_ADEC_METAINFO, pMetaInfo);
        if (Ret != MT_SUCCESS) {
            MT_ERR_AVPLAY("call MT_MPI_ADEC_GetInfo failed.\n");
        }
    }
	MT_AVPLAY_INST_UNLOCK();

	return Ret;
}

mt_s32 MT_MPI_AVPLAY_SetPos(mt_handle hAvplay, float x, float y, float z)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret = MT_SUCCESS;

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->hAdec != MT_INVALID_HANDLE) {
        Ret = MT_MPI_ADEC_Set_VIVID_Pos(x,y,z);
        if (Ret != MT_SUCCESS) {
            MT_ERR_AVPLAY("call MT_MPI_AVPLAY_SetPos failed.\n");
        }
    }
	MT_AVPLAY_INST_UNLOCK();

	return Ret;
}

mt_s32 MT_MPI_AVPLAY_SelObj(mt_handle hAvplay, mt_u16 id, mt_u16 on)
{
    AVPLAY_S                *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S       AvplayUsrAddr;
    mt_s32                  Ret = MT_SUCCESS;

    AVPLAY_GET_INST_AND_LOCK();

    if (pAvplay->hAdec != MT_INVALID_HANDLE) {
        Ret = MT_MPI_ADEC_Sel_VIVID_Obj(id, on);
        if (Ret != MT_SUCCESS) {
            MT_ERR_AVPLAY("call MT_MPI_AVPLAY_SelObj failed.\n");
        }
    }
	MT_AVPLAY_INST_UNLOCK();

	return Ret;
}

mt_s32 MT_MPI_AVPLAY_set_trickmode(mt_handle hAvplay, u32 mode)
{
    AVPLAY_S           *pAvplay = MT_NULL;
    AVPLAY_USR_ADDR_S  AvplayUsrAddr;
    mt_s32               Ret = MT_FAILURE;

    AVPLAY_GET_INST_AND_LOCK();
	if (pAvplay->AudEnable) {
		Ret = MT_MPI_ADEC_set_trickmode(pAvplay->hAdec, mode);
	}
	MT_AVPLAY_INST_UNLOCK();

    return Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


