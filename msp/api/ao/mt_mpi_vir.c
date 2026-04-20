/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "mt_mpi_vir.h"
#include "mt_mpi_mem.h"
#include "mt_error_mpi.h"
#include "mt_module_debug.h"

#define VIRTRACK_LOCK(Mutex)        pthread_mutex_lock(&Mutex)
#define VIRTRACK_UNLOCK(Mutex)      pthread_mutex_unlock(&Mutex)

typedef struct
{
    pthread_mutex_t   stMutex;               /* Mutex */
    mt_u32    uTrackFlag;
    mt_handle hTrack[AO_MAX_VIRTUAL_TRACK_NUM];
} VIR_TRACK_RS_STATE_S;


static VIR_TRACK_RS_STATE_S g_stVirTrack;

mt_s32 VirInitBuf(VIR_BUFFUR_S *pstBuf, mt_u32 u32Size);
mt_void VirDeInitBuf(VIR_BUFFUR_S *pstBuf);
mt_u32 VirGetFreeID(mt_void);
mt_u32 VIRTrack2VirID(mt_handle hTrack);

static mt_void VIRResetPTSQue(VIR_PTS_QUE_S *pstPTSQue)
{
    if (!pstPTSQue)
    {
        return;
    }

    memset(pstPTSQue->stPTSArry, 0, sizeof(VIR_PTS_S) * VIR_MAX_STORED_PTS_NUM);
    pstPTSQue->u32PTSreadIdx  = 0;
    pstPTSQue->u32PTSwriteIdx = 0;
    pstPTSQue->u32LastPtsMs = (mt_u32)-1;
    return;
}
static mt_void VIRFindPTS(VIR_BUFFUR_S *pstBuf, mt_u32 *pu32FoundPts, mt_u32 *pu32FoundPos)
{
    mt_u32 u32FoundPos, u32RdPos, u32WtPos, u32FoundPts;
    VIR_PTS_S *pstPTS = MT_NULL;
    VIR_PTS_QUE_S *pstPTSQue;
    mt_u32 u32ReadPtr;

    u32FoundPts = (mt_u32)-1;
    u32ReadPtr = pstBuf->u32Read;
    pstPTSQue = &(pstBuf->stPTSQue);
    u32RdPos = pstPTSQue->u32PTSreadIdx;
    u32WtPos = pstPTSQue->u32PTSwriteIdx;
    pstPTS = pstPTSQue->stPTSArry;

    for (u32FoundPos = u32RdPos; u32FoundPos != u32WtPos;  u32FoundPos = (u32FoundPos + 1) % VIR_MAX_STORED_PTS_NUM)
    {
        if (pstPTS[u32FoundPos].u32BegPtr < pstPTS[u32FoundPos].u32EndPtr)
        {
            if ((pstPTS[u32FoundPos].u32BegPtr <= u32ReadPtr) && (pstPTS[u32FoundPos].u32EndPtr >= u32ReadPtr))
            {
                u32FoundPts = pstPTS[u32FoundPos].u32PtsMs;
                break;
            }
        }
        else
        {
            if ((pstPTS[u32FoundPos].u32BegPtr <= u32ReadPtr) || (pstPTS[u32FoundPos].u32EndPtr >= u32ReadPtr))
            {
                u32FoundPts = pstPTS[u32FoundPos].u32PtsMs;
                break;
            }
        }
    }

    if(u32FoundPos == u32WtPos)
    {
        u32FoundPos = (mt_u32)-1;
    }

    *pu32FoundPos = u32FoundPos;
    *pu32FoundPts = u32FoundPts;
    return;
}

static mt_u32 VIRAcquirePTS(VIR_BUFFUR_S *pstBuf, mt_u32 u32PcmSamplesPerFrame,
                               mt_u32 u32PcmSampleRate)
{
    mt_u32 u32PtsMs;
    mt_u32 u32FoundPos, u32FoundPts;
    VIR_PTS_QUE_S *pstPTSQue;

    pstPTSQue = &(pstBuf->stPTSQue);
    VIRFindPTS(pstBuf, &u32FoundPts, &u32FoundPos);

    if (((mt_u32)-1) == u32FoundPts)
    {
        /*can not find a valid PTS*/
        if (((mt_u32)-1) != pstPTSQue->u32LastPtsMs)
        {
            mt_u32 u32Delta;
            u32Delta = (u32PcmSamplesPerFrame * 1000) / u32PcmSampleRate;
            u32PtsMs = pstPTSQue->u32LastPtsMs + u32Delta;
            if (((mt_u32)-1) == u32PtsMs)
            {
               u32PtsMs = 0;
            }
        }
        else
        {
            u32PtsMs = (mt_u32)-1;
        }
    }
    else
    {
        /* Found a valid PTS */
        u32PtsMs = u32FoundPts;
    }
    return u32PtsMs;
}

static mt_void VIRReleasePTS(VIR_BUFFUR_S *pstBuf, mt_u32 u32PtsMs)

{
    mt_u32 u32FoundPos, u32FoundPts;
    VIR_PTS_QUE_S *pstPTSQue;
    VIR_PTS_S *pstPTS;

    pstPTSQue = &(pstBuf->stPTSQue);
    pstPTS = pstPTSQue->stPTSArry;
    pstPTSQue->u32LastPtsMs = u32PtsMs;
    VIRFindPTS(pstBuf, &u32FoundPts, &u32FoundPos);
    if((mt_u32)-1 != u32FoundPos)
    {
        pstPTS[u32FoundPos].u32PtsMs = (mt_u32)(-1);
        pstPTSQue->u32PTSreadIdx = (u32FoundPos + 1) % VIR_MAX_STORED_PTS_NUM;
    }
    return;
}


static mt_void VIRStorePTS (VIR_BUFFUR_S *pstBuf, mt_u32 u32PtsMs, mt_u32 u32Size)
{
    VIR_PTS_QUE_S *pstPTSQue= &(pstBuf->stPTSQue);
    VIR_PTS_S *pstPTSArray = pstPTSQue->stPTSArry;
    mt_u32 u32CalcEndPtr;

    /* make sure there are space to store */
    if ((pstPTSQue->u32PTSwriteIdx + 1) % VIR_MAX_STORED_PTS_NUM != pstPTSQue->u32PTSreadIdx)
    {
        if ((pstBuf->u32Write + u32Size) < pstBuf->u32End)
        {
            u32CalcEndPtr = pstBuf->u32Write + u32Size;
        }
        else
        {
            u32CalcEndPtr = pstBuf->u32Write + u32Size - (pstBuf->u32End - pstBuf->u32Start);
        }

        pstPTSArray[pstPTSQue->u32PTSwriteIdx].u32PtsMs  = u32PtsMs;
        pstPTSArray[pstPTSQue->u32PTSwriteIdx].u32BegPtr = pstBuf->u32Write;
        pstPTSArray[pstPTSQue->u32PTSwriteIdx].u32EndPtr = u32CalcEndPtr;
        pstPTSQue->u32PTSwriteIdx = (pstPTSQue->u32PTSwriteIdx + 1) % VIR_MAX_STORED_PTS_NUM;
    }
    else
    {
        MT_WARN_AO("Not enough PTS buffer, discard current PTS(%d)\n", u32PtsMs);
    }
    return;
}


mt_s32 VirInitBuf(VIR_BUFFUR_S *pstBuf, mt_u32 u32Size)
{
    if ((u32Size < VIR_MIN_OUTBUF_SIZE) || (u32Size > VIR_MAX_OUTBUF_SIZE))
    {
        MT_ERR_AO(" invalid input buffer size(%d) minsize(%d) maxsize(%d)!\n", u32Size,
                    VIR_MIN_OUTBUF_SIZE, VIR_MAX_OUTBUF_SIZE);
        return MT_FAILURE;
    }

    pstBuf->pu8BufBase = (mt_u8 *)mt_malloc(MT_ID_AO, u32Size + VIR_MAX_FRAME_SIZE);
    if (MT_NULL_PTR == pstBuf->pu8BufBase)
    {
        MT_FATAL_AO("MALLOC pstBuf error\n");
        return MT_FAILURE;
    }
    pstBuf->u32Start = 0;
    pstBuf->u32End = u32Size;
    pstBuf->u32Read = 0;
    pstBuf->u32Write = 0;
    pstBuf->s32BitPerSample = 16;
    pstBuf->u32SampleRate = (mt_u32)MT_UNF_SAMPLE_RATE_48K;
    pstBuf->u32Channel = 2;
    pstBuf->u32PcmSamplesPerFrame = 1024;

    VIRResetPTSQue(&(pstBuf->stPTSQue));

    return MT_SUCCESS;
}

mt_void VirDeInitBuf(VIR_BUFFUR_S *pstBuf)
{
    VIRResetPTSQue(&(pstBuf->stPTSQue));
    if (pstBuf->pu8BufBase)
    {
        mt_free(MT_ID_AO, (mt_void*)pstBuf->pu8BufBase);
        pstBuf->pu8BufBase = MT_NULL;
    }

    pstBuf->u32Start = 0;
    pstBuf->u32End = 0;
    pstBuf->u32Read = 0;
    pstBuf->u32Write = 0;
    return;
}

static mt_u32  VIRGetDataSize(VIR_BUFFUR_S *pstBuf)
{
    if (pstBuf->u32Read > pstBuf->u32Write)
    {
        return (pstBuf->u32End - pstBuf->u32Start) - (pstBuf->u32Read - pstBuf->u32Write);
    }
    else
    {
        return pstBuf->u32Write - pstBuf->u32Read;
    }
}

static mt_u32  VIRGetIdleSize(VIR_BUFFUR_S *pstBuf)
{
    if (pstBuf->u32Read > pstBuf->u32Write)
    {
        return (pstBuf->u32Read - pstBuf->u32Write);
    }
    else
    {
        return ((pstBuf->u32End - pstBuf->u32Start) + pstBuf->u32Read) - pstBuf->u32Write;
    }
}

static mt_void  VIRFlushBuf(VIR_BUFFUR_S *pstBuf)
{
    pstBuf->u32Write = pstBuf->u32Read;
    VIRResetPTSQue(&(pstBuf->stPTSQue));
    return;
}

mt_u32 VirGetFreeID(mt_void)
{
    mt_u32 i;
    for(i = 0; i < AO_MAX_VIRTUAL_TRACK_NUM; i++)
    {
        if(!(g_stVirTrack.uTrackFlag & ((mt_u32)1L << i)))
        {
            return i;
        }
    }
    return AO_MAX_VIRTUAL_TRACK_NUM;
}

mt_u32 VIRTrack2VirID(mt_handle hTrack)
{
    return ((hTrack & AO_TRACK_CHNID_MASK) - AO_MAX_REAL_TRACK_NUM);
}

mt_void VIR_InitRS(mt_void)
{
    mt_u32 i;

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    g_stVirTrack.uTrackFlag = 0;
    for(i = 0; i < AO_MAX_VIRTUAL_TRACK_NUM; i++)
    {
        g_stVirTrack.hTrack[i] = MT_NULL;
    }

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return;
}

mt_void VIR_DeInitRS(mt_void)
{
    mt_u32 i;

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    g_stVirTrack.uTrackFlag = 0;
    for(i = 0; i < AO_MAX_VIRTUAL_TRACK_NUM; i++)
    {
        g_stVirTrack.hTrack[i] = MT_NULL;
    }

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return;
}


mt_s32 VIR_CreateTrack(const MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr, mt_handle *phTrack)
{
    mt_s32 s32Ret;
    VIR_TRACK_STATE_S *pstVir;
    mt_u32 u32VirID;

    CHECK_AO_NULL_PTR(phTrack);
    CHECK_AO_NULL_PTR(pstTrackAttr);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VirGetFreeID();
    if(AO_MAX_VIRTUAL_TRACK_NUM == u32VirID)
    {
        MT_ERR_AO("have not free virtual track ID!\n");
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    pstVir = (VIR_TRACK_STATE_S *)mt_malloc(MT_ID_AO, sizeof(VIR_TRACK_STATE_S));
    if (MT_NULL == pstVir)
    {
        MT_ERR_AO("malloc virtual track fail\n");
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    memset(pstVir, 0, sizeof(VIR_TRACK_STATE_S));
    memcpy(&pstVir->stTrackAttr, pstTrackAttr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
    pstVir->u32BufSize = pstTrackAttr->u32OutputBufSize;

    /*output buf*/
    pstVir->pstBuf = (VIR_BUFFUR_S *)mt_malloc(MT_ID_AO, sizeof(VIR_BUFFUR_S));
    if(MT_NULL == pstVir->pstBuf)
    {
        mt_free(MT_ID_AO, (mt_void *)pstVir);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    s32Ret = VirInitBuf(pstVir->pstBuf, pstVir->u32BufSize);
    if(MT_SUCCESS != s32Ret)
    {
        mt_free(MT_ID_AO, (mt_void *)(pstVir->pstBuf));
        mt_free(MT_ID_AO, (mt_void *)pstVir);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    g_stVirTrack.uTrackFlag |= (mt_u32)1L << u32VirID;
    g_stVirTrack.hTrack[u32VirID] = (mt_handle)pstVir;

    *phTrack = (MT_ID_AO << 16) | (MT_ID_TRACK << 8) | (AO_MAX_REAL_TRACK_NUM + u32VirID);

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}

mt_s32 VIR_DestroyTrack(mt_handle hTrack)
{
    VIR_TRACK_STATE_S *pstVir = MT_NULL;
    mt_u32 u32VirID;

    CHECK_Track(hTrack);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VIRTrack2VirID(hTrack);

    pstVir = (VIR_TRACK_STATE_S *)g_stVirTrack.hTrack[u32VirID];
    if(pstVir)
    {
        if(pstVir->pstBuf)
        {
            VirDeInitBuf(pstVir->pstBuf);
            mt_free(MT_ID_AO, (mt_void*)(pstVir->pstBuf));
            pstVir->pstBuf = MT_NULL;
        }
        mt_free(MT_ID_AO, (mt_void*)(pstVir));
        g_stVirTrack.hTrack[u32VirID] = MT_NULL;
        g_stVirTrack.uTrackFlag &= ~((mt_u32)1L << u32VirID);
    }

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}

mt_s32  VIR_GetAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr)
{
    VIR_TRACK_STATE_S *pstVir = MT_NULL;
    mt_u32 u32VirID;

    CHECK_Track(hTrack);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VIRTrack2VirID(hTrack);
    pstVir = (VIR_TRACK_STATE_S *)g_stVirTrack.hTrack[u32VirID];
    if(MT_NULL == pstVir)
    {
        MT_ERR_AO("virtual track(%d) is null!\n", u32VirID);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    memcpy(pstAttr, &pstVir->stTrackAttr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}

mt_s32  VIR_SendData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
    mt_u8 *pu8Src, *pu8Dst;
    mt_u32 u32Size, u32Idle, u32TailSpace;
    VIR_TRACK_STATE_S *pstVir = MT_NULL;
    mt_u32 u32VirID;
    VIR_BUFFUR_S *pstBuf;

    CHECK_Track(hTrack);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VIRTrack2VirID(hTrack);

    pstVir = (VIR_TRACK_STATE_S *)g_stVirTrack.hTrack[u32VirID];
    if(MT_NULL == pstVir)
    {
        MT_ERR_AO("virtual track(%d) is null!\n", u32VirID);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    pstBuf = pstVir->pstBuf;

    if(pstAOFrame->u32PcmSamplesPerFrame == 0)
    {
        MT_WARN_AO("There is no PcmData!\n");
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    if(pstBuf->u32Channel != pstAOFrame->u32Channels
        ||pstBuf->s32BitPerSample != pstAOFrame->s32BitPerSample
        ||pstBuf->u32SampleRate != pstAOFrame->u32SampleRate)
    {
        VIRFlushBuf(pstBuf);   //samplerate or channle or bitpersampler is modified , track buffer will reset.
        pstBuf->u32Channel = pstAOFrame->u32Channels;
        pstBuf->s32BitPerSample = pstAOFrame->s32BitPerSample;
        pstBuf->u32SampleRate = pstAOFrame->u32SampleRate;
    }

    if(16 == pstAOFrame->s32BitPerSample)
    {
        u32Size  = pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s16);
    }
    else
    {
        u32Size  = pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s32);
    }

    u32Idle  = VIRGetIdleSize(pstBuf);
    if(u32Idle <= u32Size)
    {
        MT_INFO_AO("Track buf is full, clear buffer");
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_ERR_AO_OUT_BUF_FULL;   //verify
        //HIAOFlushVirtualBuf(pstBuf);
        //u32Idle = pstBuf->u32End - pstBuf->u32Start;
    }

    if (pstBuf->u32Write < pstBuf->u32Read)
    {
        u32TailSpace = u32Idle;
    }
    else
    {
        u32TailSpace = pstBuf->u32End - pstBuf->u32Write;
    }

    pu8Src = (mt_u8*)(pstAOFrame->ps32PcmBuffer);
    pu8Dst = (mt_u8*)(pstBuf->pu8BufBase + (pstBuf->u32Write - pstBuf->u32Start));

    if (u32Size <= u32TailSpace)
    {
        memcpy((mt_void *)pu8Dst, (mt_void *)pu8Src, u32Size);
    }
    else
    {
        memcpy((mt_void *)pu8Dst, (mt_void *)pu8Src, u32TailSpace);

        memcpy((mt_void *)pstBuf->pu8BufBase, (mt_void *)(pu8Src + u32TailSpace), u32Size - u32TailSpace);
    }

    VIRStorePTS(pstBuf, (mt_u32)pstAOFrame->u64PtsMs, u32Size);
    /* update WritePtr */
    pstBuf->u32Write += u32Size;
    if (pstBuf->u32Write >= pstBuf->u32End)
    {
        pstBuf->u32Write -= (pstBuf->u32End - pstBuf->u32Start);
    }

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}

mt_s32  VIR_AcquireFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
    mt_u32 u32AcqSize, u32DataSize, u32TailData;
    mt_u8 *pu8Src, *pu8Dst;
    static mt_u32 u32FrameIdx = 0;
    VIR_TRACK_STATE_S *pstVir = MT_NULL;
    mt_u32 u32VirID;
    VIR_BUFFUR_S *pstBuf;

    CHECK_Track(hTrack);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VIRTrack2VirID(hTrack);

    pstVir = (VIR_TRACK_STATE_S *)g_stVirTrack.hTrack[u32VirID];
    if(MT_NULL == pstVir)
    {
        MT_ERR_AO("virtual track(%d) is null!\n", u32VirID);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    pstBuf = pstVir->pstBuf;

    pstAOFrame->u32SampleRate = pstBuf->u32SampleRate;
    pstAOFrame->s32BitPerSample = pstBuf->s32BitPerSample;
    pstAOFrame->u32Channels = pstBuf->u32Channel;
    pstAOFrame->u32PcmSamplesPerFrame = pstBuf->u32PcmSamplesPerFrame;
    //pstAOFrame->u32PcmSamplesPerFrame = pstAOFrame->u32SampleRate * 10 / 1000; //default 10ms
    if(16 == pstAOFrame->s32BitPerSample)
    {
        u32AcqSize = pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s16);
    }
    else
    {
        u32AcqSize = pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s32);
    }

    u32DataSize = VIRGetDataSize(pstBuf);
    if(u32AcqSize > u32DataSize)
    {
        MT_WARN_AO("Acquire size(%d) exceed buf data size(%d)\n", u32AcqSize, u32DataSize);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_ERR_AO_VIRTUALBUF_EMPTY;
    }

    pstAOFrame->ps32PcmBuffer = (ulong *)(pstBuf->pu8BufBase + (pstBuf->u32Read - pstBuf->u32Start));

    if (pstBuf->u32Write < pstBuf->u32Read)
    {
        u32TailData = pstBuf->u32End - pstBuf->u32Read;
        if (u32AcqSize > u32TailData)
        {
            pu8Src = pstBuf->pu8BufBase + pstBuf->u32Start;
            pu8Dst = pstBuf->pu8BufBase + pstBuf->u32End;
            memcpy((mt_void *)pu8Dst, (mt_void *)pu8Src, u32AcqSize - u32TailData);
        }
    }

    pstAOFrame->u64PtsMs = VIRAcquirePTS(pstBuf, pstAOFrame->u32PcmSamplesPerFrame, pstAOFrame->u32SampleRate);

    pstAOFrame->bInterleaved = MT_TRUE;
    pstAOFrame->u32FrameIndex = u32FrameIdx++;
    pstAOFrame->u32BitsBytesPerFrame = 0;
    pstAOFrame->ps32BitsBuffer = MT_NULL;
    pstAOFrame->u32IEC61937DataType = 0;

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}

mt_s32  VIR_ReleaseFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)

{
    VIR_TRACK_STATE_S *pstVir = MT_NULL;
    mt_u32 u32VirID;
    VIR_BUFFUR_S *pstBuf;

    CHECK_Track(hTrack);

    VIRTRACK_LOCK(g_stVirTrack.stMutex);

    u32VirID = VIRTrack2VirID(hTrack);

    pstVir = (VIR_TRACK_STATE_S *)g_stVirTrack.hTrack[u32VirID];
    if(MT_NULL == pstVir)
    {
        MT_ERR_AO("virtual track(%d) is null!\n", u32VirID);
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_FAILURE;
    }

    pstBuf = pstVir->pstBuf;

    if(pstBuf->u32Channel != pstAOFrame->u32Channels
        ||pstBuf->s32BitPerSample != pstAOFrame->s32BitPerSample
        ||pstBuf->u32SampleRate != pstAOFrame->u32SampleRate)
    {
        VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
        return MT_SUCCESS; //verify
    }

    VIRReleasePTS(pstBuf, (mt_u32)pstAOFrame->u64PtsMs);
    /* update ReadPtr */
    if(16 == pstAOFrame->s32BitPerSample)
    {
        pstBuf->u32Read += pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s16);
    }
    else
    {
        pstBuf->u32Read += pstAOFrame->u32PcmSamplesPerFrame * pstAOFrame->u32Channels * sizeof(mt_s32);
    }
    if (pstBuf->u32Read >= pstBuf->u32End)
    {
        pstBuf->u32Read -= (pstBuf->u32End - pstBuf->u32Start);
    }
    pstAOFrame->ps32PcmBuffer = MT_NULL;

    VIRTRACK_UNLOCK(g_stVirTrack.stMutex);
    return MT_SUCCESS;
}


