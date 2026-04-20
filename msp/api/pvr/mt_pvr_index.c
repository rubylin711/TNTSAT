/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/times.h>

#include "mt_module.h"
#include "mt_mpi_mem.h"

#include "mt_module_debug.h"
#include "mt_pvr_index.h"
#include "mt_pvr_rec_ctrl.h"
#include "mt_pvr_play_ctrl.h"
#include "mt_pvr_fifo.h"
#include "mt_module_debug.h"
#include "mt_pvr_addon_api.h"
 
//#include "mt_pvr_stub.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

//#define PVR_DEBUG_REWIND_TIME /* for pvr debug rewind time use */

#ifndef MT_MALLOC
#define MT_MALLOC mt_malloc
#endif
#ifndef MT_FREE
#define MT_FREE mt_free
#endif


#ifndef MT_SYS_GetVersion
#define MT_SYS_GetVersion mt_sys_get_version
#endif

#define PVR_INDEX_NUM (PVR_REC_MAX_CHN_NUM + PVR_PLAY_MAX_CHN_NUM)

/**
    only replay, use the first 2,
    only record, use the last 3,
    on timeshift, player use the index struct of record.
    only play, the index channel number is equal the play channel.
    but, the index channel number is not equal the record channel, it should be equal with the index parser channel number
*/
static PVR_INDEX_S g_stPVRIndex[PVR_INDEX_NUM];
static MT_U32 g_u32RecChnStat[PVR_REC_MAX_CHN_NUM] = {PVR_INDEX_REC_CHN_UNUSED};
static MT_U32 g_u32PvrIndexInit = 0; 


#define PVR_DIFF_FRAME_THR  700
#define PVR_DFT_GOP_LEN     120
#define PVR_DFT_STEP_LEN    10


#define PVR_DIFF_FFLUSH_HEADINFO 10000

/* 
  Rewind record case
  =====================
  |////|xxxxx |/////////|
  |////|xxxxx |/////////|
   =====================
           /\         /\               /\
            |          |                 |
   EndFrame  StartFrame  LastFrame
   
   1.Ajust EndFrame -StartFrame gap frames for avoid ts overflow
   calculate by 25 f/S,  frames=25*seconds
   defalut seconid set 1,for rewind rec time more accurately(>=1S)
   but will drop at least  1S ts data avoid over write after rewind
   because the gap frames will auto adjust after rewind in PVR_Index_Adjust_StartFrame 
   and the max adjust frames is PVR_DFT_GOP_LEN,so the rewind rec time diff value may 1S~5S 
   2.If disable 'PVR_REWIND_ADJUST_START_FRAME',endframe and start frame gap is 120
   the rewind rec time diff about 5S
*/
#define PVR_REWIND_ADJUST_START_FRAME
#ifdef PVR_REWIND_ADJUST_START_FRAME
#define PVR_REWIND_FRAMES_GAP   (25*1) 
#define PVR_REWIND_MAX_ADJUST_FRAMES (PVR_DFT_GOP_LEN - PVR_REWIND_FRAMES_GAP)
static MT_U32 g_pvr_rewind_adjust_count = 0;
#endif


#define PVR_GET_HEADER_OFFSET() ((ulong) (&((PVR_IDX_HEADER_INFO_S *)0)->stCycInfo))
#define PVR_GET_USR_DATA_OFFSET(headInfo) (sizeof(PVR_IDX_HEADER_INFO_S) + PVR_MAX_CADATA_LEN)
#define PVR_GET_CA_DATA_OFFSET() (sizeof(PVR_IDX_HEADER_INFO_S))
#define PVR_GET_IDX_INFO_OFFSET(headInfo) (sizeof(PVR_IDX_HEADER_INFO_S) + (headInfo.u32CADataInfoLen) - sizeof(PVR_REC_INDEX_INFO_S))

#define PVR_IS_CYC_READFRAME_INVLD(start, end, read) \
(((end) > (start) && ((read) < (start) || (read) > (end)))\
     || ((end) < (start) && ((read) < (start) && (read) > (end))) )

#define PVR_IDX_IS_REWIND(handle) ((handle)->stCycMgr.bIsRewind)
 
//printf("++write.id=%x\n",offset/sizeof(PVR_INDEX_ENTRY_S));
#define PVR_WRITE_INDEX(saveSz, wantSz, buf, fd, offset, handle) \
do{\
saveSz = (mt_u32)PVR_WRITE(buf, (size_t)wantSz, fd, (offset + handle->u32IdxStartOffsetLen));\
if ((mt_u32)saveSz != (mt_u32)wantSz)\
{\
    if (NULL != &errno)\
    {\
        if (ENOSPC == errno)\
        {\
            MT_ERR_PVR("PVR_WRITE fail:%d, want:%u\n", saveSz, wantSz);\
            PVR_IDX_CACHE_UNLOCK_W(handle);\
            return MT_ERR_PVR_FILE_DISC_FULL;\
        }\
        else\
        {\
            MT_ERR_PVR("PVR_WRITE fail:%d, want:%u\n", saveSz, wantSz);\
            PVR_IDX_CACHE_UNLOCK_W(handle);\
            return MT_ERR_PVR_FILE_CANT_WRITE;\
        }\
    }\
}\
}while(0)
static MT_S32 Pvr_Write_Index_Func(mt_u32 saveSz,mt_u32 wantSz, void *buf,PVR_FILE fd,MT_U32 offset, PVR_INDEX_HANDLE handle)
{
    PVR_IDX_CACHE_LOCK_W(handle);
    PVR_WRITE_INDEX(saveSz, wantSz, buf, fd, offset, handle);
    PVR_IDX_CACHE_UNLOCK_W(handle);
    return MT_SUCCESS;
}
#define PVR_READ_INDEX_DIRECTLY(readSz,buf, size, fd,  offset, handle)\
do{\
if (PVR_Index_IfOffsetInWriteCache(handle,offset,size))\
{\
    PVR_Index_FlushIdxWriteCache(handle);\
}\
readSz = PVR_READALL(buf, size, fd,  (off_t)(offset + handle->u32IdxStartOffsetLen));\
}while(0)

                       
#define PVR_READ_INDEX(readSz, buf, size, fd, offset, handle)\
do{\
  readSz = PVRCacheReadIdx(handle,fd,(MT_VOID*)buf,size,offset,0);\
}while(0)


#define PVR_IDX_CHECK_CYC_SIZE(pstRecAttr) \
    do{\
        if ((pstRecAttr->u64MaxFileSize == 0) && (pstRecAttr->u64MaxTimeInMs == 0))\
        {\
            MT_WARN_PVR("invalidate u64MaxFileSize and u64MaxTimeInMs error!\n");\
        }\
        else if ((pstRecAttr->u64MaxFileSize > 0) && (pstRecAttr->u64MaxTimeInMs > 0))\
        {\
            if ((pstRecAttr->u64MaxFileSize < PVR_MIN_CYC_SIZE) || (pstRecAttr->u64MaxTimeInMs < PVR_MIN_CYC_TIMEMS))\
            {\
                MT_WARN_PVR("invalidate u64MaxFileSize and u64MaxTimeInMs error!\n");\
            }\
        }\
        else if ((pstRecAttr->u64MaxFileSize > 0) && (pstRecAttr->u64MaxFileSize < PVR_MIN_CYC_SIZE))\
        {\
            MT_WARN_PVR("invalidate u64MaxFileSize and u64MaxTimeInMs error!\n");\
        }\
        else if ((pstRecAttr->u64MaxTimeInMs > 0) && (pstRecAttr->u64MaxTimeInMs < PVR_MIN_CYC_TIMEMS))\
        {\
            MT_WARN_PVR("invalidate u64MaxFileSize and u64MaxTimeInMs error!\n");\
        }\
    }while(0)

#define MKSTR(exp) # exp
#define MKMARCOTOSTR(exp) MKSTR(exp)
static const MT_U8 s_szPvrVersion[] __attribute__((used)) = "SDK_VERSION:["\
										MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
										__DATE__", "__TIME__"]";
static MT_S32 PVR_Index_GetPreIFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num);
/*use cache when read*/
static ssize_t PVRCacheReadIdx(PVR_INDEX_HANDLE  handle,PVR_FILE fd, MT_VOID* pData,size_t size, 
                              MT_U32 offset,MT_U32 u32DirectFlag);
static MT_S32 PVR_Index_IfOffsetReadCache(PVR_INDEX_HANDLE  handle,MT_U32 u32Offset,MT_U32 u32Size);
static MT_S32 PVRIndexGetEntryByNum(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry, MT_U32 u32FrameNum);
void pvr_crypto_init(PVR_REC_CHN_S **srec,PVR_PLAY_CHN_S** sply);

MT_U32 Pvr_Calc_distance(MT_U32 s,MT_U32 l,MT_U32 x)
{
    if(x>=s){
        return (x-s);
    }else{
        return (l-(s-x));
    }
}
MT_BOOL Pvr_Check_ReadInRange(MT_U32 s,MT_U32 e,MT_U32 r)
{
    if(s<=e){//0.s.e.l
        if((r<=e)&&(r>=s)){//0.s.r.e
            return MT_TRUE;
        }
        return MT_FALSE;
    }else{//0.e.s.l
        if((r>e)&&(r<s)){//0.r.e.s.r.l
            return MT_FALSE;
        }
        return MT_TRUE;
    }
}
MT_U32 Pvr_Move_Readpos_forward(MT_U32 s,MT_U32 e,MT_U32 l,MT_U32 r,MT_U32 mx)
{
    while(mx){
        if(r==e){
            break;
        }
        r++;
        if(r==l){
            r=0;
        }
        mx--;
    }
    return r;
}
MT_U32 Pvr_Move_Readpos_backward(MT_U32 s,MT_U32 e,MT_U32 l,MT_U32 r,MT_U32 mx)
{
    while(mx){
        if(r==s){
            break;
        }
        r--;
        if(r==0){
            r=l;
        }
        mx--;
    }
    return r;
}

MT_S32 Pvr_Adjust_ReadInGoodRange(PVR_INDEX_HANDLE IndexHandle,MT_U32 dirct)
{//adjust r in valid range
    MT_S32 s32ret=0;
    if(IndexHandle){
        MT_U32 s = IndexHandle->stCycMgr.u32StartFrame;
        MT_U32 e = IndexHandle->stCycMgr.u32EndFrame;
        MT_U32 l = IndexHandle->stCycMgr.u32LastFrame;
        MT_U32 r = IndexHandle->u32ReadFrame;
        if(e>0){
            e--;
        }
        if(l>0){
            l--;
        }
        if(r>l){//r beyond l. reset r to s
            IndexHandle->u32ReadFrame = Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
            //printf("+++resume3:%x,%x,%x,%x,%x\n",s,e,l,r,IndexHandle->u32ReadFrame);
        }else{
            s32ret=Pvr_Check_ReadInRange(s,e,r);
            if(MT_FALSE==s32ret){//not in range.seek to s+PVR_TPLAY_MIN_DISTANCE
                if(dirct){
                    IndexHandle->u32ReadFrame = Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                    //printf("+++resume0:%x,%x,%x,%x,%x\n",s,e,l,r,IndexHandle->u32ReadFrame);
                }else{
                    IndexHandle->u32ReadFrame = e;
                    //printf("+++resume1:%x,%x,%x,%x,%x\n",s,e,l,r,IndexHandle->u32ReadFrame);
                }
            }
            //printf("+++resume2:%x,%x,%x,%x,%x\n",s,e,l,r,IndexHandle->u32ReadFrame);
        }
    }
    return s32ret;
}

MT_S32 PVR_Index_SeekToStart_AndDistance(PVR_INDEX_HANDLE handle,MT_U32 lockidx)
{
    MT_U32 s,e,l;

    MT_ASSERT_RET(handle != NULL);

    if(lockidx){
        PVR_INDEX_LOCK(handle);
    }
    s = handle->stCycMgr.u32StartFrame;
    e = handle->stCycMgr.u32EndFrame;
    l = handle->stCycMgr.u32LastFrame;
    if(e>0){
        e--;
    }
    if(l>0){
        l--;
    }
    handle->u32ReadFrame = Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
    //printf("+++resume333:%x,%x,%x,%x\n",s,e,l,handle->u32ReadFrame);
    if(lockidx){
        PVR_INDEX_UNLOCK(handle);
    }
    //MT_WARN_PVR("%s,%x, %x, %x, %x\n", __FUNCTION__, s,e,l,handle->u32ReadFrame);
    return MT_SUCCESS;
}
/*
static MT_U32 PVRIndex_UpdatePreEntryTimeMs(PVR_INDEX_HANDLE pvrIndexHandle, MT_U32 u32TimeMs)
{
    MT_U32 u32MaxCount = sizeof(pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs)/sizeof(pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[0]);
    MT_U32 u32Index = 0;
    MT_U32 u32AverageRet = 0;
    MT_U64 u64TotalValue = 0;

    for (u32Index=0; u32Index<(u32MaxCount-1); u32Index++)
    {
        pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32Index] = pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32Index+1];

        u64TotalValue += pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32Index];
    }

    pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32MaxCount-1] = u32TimeMs;

    u64TotalValue += pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32MaxCount-1];

    u32AverageRet = u64TotalValue / u32MaxCount;

    pvrIndexHandle->stPreEntryTime.s32Count++;
    pvrIndexHandle->stPreEntryTime.s32Count %= u32MaxCount;

    return u32AverageRet;
}

static MT_U32 PVRIndex_GetPreEntryAverageTimeMs(PVR_INDEX_HANDLE pvrIndexHandle)
{
    MT_U32 u32MaxCount = sizeof(pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs)/sizeof(pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[0]);
    MT_U32 u32Index = 0;
    MT_U32 u32AverageRet = 0;
    MT_U64 u64TotalValue = 0;

    for (u32Index=0; u32Index<u32MaxCount; u32Index++)
    {
        u64TotalValue += pvrIndexHandle->stPreEntryTime.u32PreEntryTimeMs[u32Index];
    }

    u32AverageRet = u64TotalValue / u32MaxCount;

    return u32AverageRet;
}
*/
/*
MT_U32 Pvr_Calc_distance(MT_U32 s,MT_U32 l,MT_U32 x)
{
    if(x>=s){
        return (x-s);
    }else{
        return (l-(s-x));
    }
}

MT_S32 Pvr_Check_ReadInRange(MT_U32 s,MT_U32 e,MT_U32 r)
{
    if(s<=e){//0.s.e.l
        if((r<=e)&&(r>=s)){//0.s.r.e
            return MT_TRUE;
        }
        return MT_FALSE;
    }else{//0.e.s.l
        if((r>e)&&(r<s)){//0.r.e.s.r.l
            return MT_FALSE;
        }
        return MT_TRUE;
    }
}
*/
MT_U32 PVRIndexGetCurTimeMs(MT_VOID)
{
    MT_U32    Ticks;
	struct tms buf;
    Ticks = (MT_U32)times(&buf);

    return Ticks * 10;
}

/*****************************************************************************
 Prototype       : PVRIndexIsFileRecording
 Description     : check whether the file is recording or not.
 Input           : pIdxFileName--- to check the file
 Output          : handle------- PVR index handle
 Return Value    : On successfully, return MT_TRUE. otherwise return MT_FALSE;
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2010/06/18
    Author       : j40671
    Modification : Created function

*****************************************************************************/
/**
@brief Check the index file whether is the recording one or not. If MT_TRUE, param handle will save the recording index handle.

@param[in] pIdxFileName : the name of index file
@param[out] handle      : if return MT_TRUE, this param save the recording index handle.

@return MT_BOOL

@retval MT_FALSE The index file does NOT record, yet
@retval MT_TRUE The index file is recording

@author j40671
@date 2010/06/18
@sa PVRIndexIsFilePlaying.
*/
static MT_BOOL PVRIndexIsFileRecording(const MT_CHAR *pIdxFileName, PVR_INDEX_HANDLE *handle)
{
    MT_U32 i;

    if ((NULL == handle) || (NULL == pIdxFileName))
    {
        MT_ERR_PVR("\nInput pointer parameter is NULL!\n");
        return MT_FALSE;   
    }

    for (i = 0; i < PVR_INDEX_NUM; i++)
    {
        if (g_stPVRIndex[i].bIsRec)
        {
            if  (!strncmp(g_stPVRIndex[i].szIdxFileName, pIdxFileName,strlen(pIdxFileName)))
            {
                *handle = &g_stPVRIndex[i];

                return MT_TRUE;
            }
        }
    }

    return MT_FALSE;
}

/*****************************************************************************
 Prototype       : PVRIndexIsFilePlaying
 Description     : get index handle by file name
 Input           : pFileName  **
 Output          : handle     **
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2010/06/18
    Author       : j40671
    Modification : Created function

*****************************************************************************/
/**
@brief Check the index file whether is the playing one or not. If MT_TRUE, param handle will save the playing index handle.

@param[in] pIdxFileName : the name of index file
@param[out] handle      : if return MT_TRUE, this param save the playing index handle.

@return MT_BOOL

@retval MT_FALSE The index file does NOT play, yet
@retval MT_TRUE The index file is playing

@author j40671
@date 2010/06/18
@sa PVRIndexIsFileRecording.
*/

static MT_BOOL PVRIndexIsFilePlaying(const MT_CHAR *pIdxFileName, PVR_INDEX_HANDLE *handle)
{
    MT_U32 i;
    
    if ((NULL == handle) || (NULL == pIdxFileName))
    {
        MT_ERR_PVR("\nInput pointer parameter is NULL!\n");
        return MT_FALSE;   
    }

    for (i = 0; i < PVR_INDEX_NUM; i++)
    {
        if (g_stPVRIndex[i].bIsPlay)
        {
            if  (!strncmp(g_stPVRIndex[i].szIdxFileName, pIdxFileName,strlen(pIdxFileName)))
            {
                *handle = &g_stPVRIndex[i];

                return MT_TRUE;
            }
        }
    }

    return MT_FALSE;
}

#if 0
STATIC INLINE MT_U32 PVRIndexCycAdjReadFrame(const PVR_INDEX_HANDLE handle, MT_U32 u32CurReadFrame)
{
    MT_U32 u32ReadFrame = u32CurReadFrame;
    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U32 u32LastFrame = 0;

    /*

    MT_ERR_PVR("==>: S:%d, E:%d, L:%d, C:%d, O:%d, u32ReadFrame:%d\n", handle->stCycMgr.u32StartFrame,
                handle->stCycMgr.u32EndFrame, handle->stCycMgr.u32LastFrame,
                handle->u32ReadFrame, handle->u32ReadFrame, u32ReadFrame);
    */
    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32EndFrame = handle->stCycMgr.u32EndFrame; ///--------wxl for test
    if ((MT_S32)u32EndFrame < 0)
        u32EndFrame = 0;

    u32LastFrame = handle->stCycMgr.u32LastFrame;

    if (u32StartFrame < u32EndFrame)
    {
        if ((MT_S32)u32ReadFrame < (MT_S32)u32StartFrame)
        {
            u32ReadFrame = u32StartFrame;
        }
        if (u32ReadFrame > u32EndFrame)
        {
            u32ReadFrame = u32EndFrame;
        }
    }
    else
    {
        if ((MT_S32)u32ReadFrame < 0)
        {
            u32ReadFrame = u32LastFrame  + (MT_S32)u32ReadFrame;
        }
        else if (u32ReadFrame > (u32LastFrame - 1))
        {
            u32ReadFrame = u32ReadFrame - u32LastFrame;
        }
        else if ((u32ReadFrame < u32StartFrame) && (u32ReadFrame > u32EndFrame))
        {
            u32ReadFrame = ((u32ReadFrame - u32EndFrame) > (u32StartFrame - u32ReadFrame)) ? (u32StartFrame) : (u32EndFrame);
            //u32ReadFrame = u32EndFrame;
        }
    }

    /*

    MT_ERR_PVR("<==: S:%d, E:%d, L:%d, C:%d, O:%d\n", handle->stCycMgr.u32StartFrame,
                handle->stCycMgr.u32EndFrame, handle->stCycMgr.u32LastFrame,
                u32ReadFrame, handle->u32ReadFrame);
    */
    return u32ReadFrame;
}

/* move over or move back s32Offset frames.If beyond scope, the pointor will be directed to beginning or end*/
/*CNcomment:向前移或向后移s32Offset帧，移动后超出开始和结束，则将读指针放到开始或结束 */
STATIC INLINE MT_VOID PVRIndexCycMoveReadFrame(PVR_INDEX_HANDLE handle, MT_S32 s32Offset)
{
    MT_U32 u32ReadFrame = 0;
    //MT_U32 u32OldReadFrame = handle->u32ReadFrame;

    /*

    MT_ERR_PVR("**** Move, S:%u, E:%u, L:%u, C:%u, Off:%d ****\n",
        handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
        handle->stCycMgr.u32LastFrame, handle->u32ReadFrame, s32Offset);

    */
    /*if did not rewind, move it directly*//*CNcomment: 没有环绕直接移动就好了*/
    if (!PVR_IDX_IS_REWIND(handle))
    {
        handle->u32ReadFrame += s32Offset;
        if ((MT_S32)handle->u32ReadFrame < 0)
        {
            handle->u32ReadFrame = 0;
        }

        if ((MT_S32)handle->u32ReadFrame > handle->stCycMgr.u32EndFrame)
        {
            handle->u32ReadFrame = handle->stCycMgr.u32EndFrame;
        }
    }
    else /* call moving function when rewind*//*CNcomment:环绕的情况，使用环绕移动函数 */
    {
        u32ReadFrame = handle->u32ReadFrame + s32Offset;
        handle->u32ReadFrame = PVRIndexCycAdjReadFrame(handle, u32ReadFrame);
    }

}
#endif

/** Check whether someone frame is the end frame or not
 *
 *  @param[in] handle    Index handle
 *  @param[in] u32FrmPos To check frame
 *
 *  @retval ::MT_TRUE u32FrmPos is the end frame.
 *  @retval ::MT_FALSE u32FrmPos is not the end frame.
 *
 *  @note
 *
 *  @see ::PVRIndexIsFrameStart.
 */
STATIC INLINE MT_BOOL PVRIndexIsFrameEnd(PVR_INDEX_HANDLE handle, MT_U32 u32FrmPos)
{
    if (u32FrmPos == handle->stCycMgr.u32EndFrame)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

/**
 *  @brief Check whether someone frame is the start frame or not.
 *
 *  @param[] handle    : index handle
 *  @param[] u32FrmPos : to check frame
 *
 *  @retval ::MT_TRUE u32FrmPos is start frame.
 *  @retval ::MT_FALSE u32FrmPos is NOT start frame.
 *
 *  @note
 *
 *  @see ::PVRIndexIsFrameEnd.
 */
STATIC INLINE MT_BOOL PVRIndexIsFrameStart(PVR_INDEX_HANDLE handle, MT_U32 u32FrmPos)
{
    if (u32FrmPos == handle->stCycMgr.u32StartFrame)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}


/* between start and end, whether some position is valid frame position or not */
STATIC INLINE MT_BOOL PVRIndexIsFrameValid(PVR_INDEX_HANDLE handle, MT_U32 u32FrmPos)
{
    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U32 u32LastFrame = 0;

    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32EndFrame = handle->stCycMgr.u32EndFrame;
    u32LastFrame = handle->stCycMgr.u32LastFrame;

    /*
        1. Not rewind case
        =================
        |//////////////////// |
        |//////////////////// |
        =================
        /\                                  /\
         |                                   |
         StartFrame                    EndFrame

         2. Rewind case
         =================
         |////|xxxxx |/////////|
         |////|xxxxx |/////////|
         =================
                /\         /\               /\
                 |          |                |
          EndFrame  StartFrame  LastFrame
        */
    /* Not rewind case */
    if (u32StartFrame < u32EndFrame)
    {
        if ((u32FrmPos >= u32StartFrame) && (u32FrmPos <= u32EndFrame))
        {
            return MT_TRUE;
        }
    }
    else
    {
        /* rewind case */
        if (((u32FrmPos >= u32StartFrame) && (u32FrmPos <= u32LastFrame))
            || (u32FrmPos <= u32EndFrame))
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}
/*! @}*/
#if 0 /* by g00182102 gaoyanfeng */

/* calculate the total frame number, input three valid position. allow rewind or not*/
STATIC INLINE MT_U32 PVRIndexCalcFrameNum(MT_U32 u32StartFrame, MT_U32 u32EndFrame, MT_U32 u32LastFrame)
{
    MT_U32 u32FrameNum = 0;

    if (u32StartFrame < u32EndFrame)
    {
        u32FrameNum = u32EndFrame - u32StartFrame;
    }
    else
    {
        u32FrameNum = u32LastFrame - u32StartFrame + u32EndFrame;
    }

    return u32FrameNum;
}


/*
away from u32FrmPos with s32Offset in the direction of forward or backward.
if s32Offset reach to start, it will be start.
if s32Offset reach to end, it will be end.
the return value is the new position.
*/
STATIC INLINE MT_U32 PVRIndexCalcNewPos(PVR_INDEX_HANDLE handle, MT_U32 u32FrmPos, MT_S32 s32Offset)
{
    MT_U32 u32NewPos = 0;
    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U32 u32LastFrame = 0;
    MT_U32 u32Pos2Start = 0; /* the frame number from current pos to start pos*/
    MT_U32 u32Pos2End = 0;  /* the frame numbre from current pos to end pos */

    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32EndFrame = handle->stCycMgr.u32EndFrame;
    u32LastFrame = handle->stCycMgr.u32LastFrame;

    /* check whether the start position is valid or not. invalid, set it to start or end */
    if (!PVRIndexIsFrameValid(handle, u32FrmPos))
    {
        /* direction forward, set it to start */
        if (s32Offset <= 0)
        {
            u32NewPos = u32StartFrame;
        }
        else
        {
            u32NewPos = u32EndFrame;
        }
        return u32NewPos;
    }

    /* direction forward, that is toward the start position, whether it over than the start, if that, set it to start */
    if (s32Offset < 0)
    {
        u32Pos2Start = PVRIndexCalcFrameNum(u32StartFrame, u32FrmPos, u32LastFrame);
        if (abs(s32Offset) > u32Pos2Start)
        {
            u32NewPos = u32StartFrame;
            return u32NewPos;
        }
    }
    else /* direction backward, whether it over than the end, if that, set it to end */
    {
        u32Pos2End = PVRIndexCalcFrameNum(u32FrmPos, u32EndFrame, u32LastFrame);
        if (abs(s32Offset) > u32Pos2End)
        {
            u32NewPos = u32EndFrame;
            return u32NewPos;
        }
    }

    /* for security, make further check it */
    u32NewPos = u32FrmPos + s32Offset;

    if (u32StartFrame < u32EndFrame) /*not rewind*/
    {
        /* check the boundary, the value should be range from u32StartFrame to u32EndFrame. */
        if ((MT_S32)u32NewPos < (MT_S32)u32StartFrame)
        {
            u32NewPos = u32StartFrame;
        }

        if (u32NewPos > u32EndFrame)
        {
            u32NewPos = u32EndFrame;
        }
    }
    else /*be rewind*/
    {
        if ((MT_S32)u32NewPos < 0)
        {
            u32NewPos = u32LastFrame  + (MT_S32)u32NewPos;
        }
        else if (u32NewPos > (u32LastFrame - 1))
        {
            u32NewPos = u32NewPos - u32LastFrame;
        }
        else if ((u32NewPos < u32StartFrame) && (u32NewPos > u32EndFrame))
        {
            /* between start and end, assigned with the most close to it. normally, never incoming here*/
            u32NewPos = ((u32NewPos - u32EndFrame) > (u32StartFrame - u32NewPos)) ? (u32StartFrame) : (u32EndFrame);
        }
    }

    if(abs(s32Offset) > 1)
    {
    MT_WARN_PVR("\033[1;42;34m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! POSITION IS NOT EQUAL Info start !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    MT_WARN_PVR("u32StartFrame = %u, u32EndFrame = %u, u32LastFrame = %u\n",u32StartFrame, u32EndFrame, u32LastFrame);
    MT_WARN_PVR("new position = %u u32FrmPos = %u, s32Offset = %d\n",u32NewPos, u32FrmPos, s32Offset);
    MT_WARN_PVR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! POSITION IS NOT EQUAL Info end!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\033[0m\n");
    }

    return u32NewPos;
}
#else

//#define DBG_FRAME_POS

#ifdef DBG_FRAME_POS
#define MAX_PRINT_TIMES 10 /* Max print info times. */
static int g_dbgFlag = 0; /* used only by PVRIndexCalcNewPos, just only for debug, enable print times */
static int nPrintTimes = 0;
#endif



/** @addtogroup PVR_INNER_FUN */
/*! @{ */
STATIC INLINE MT_U32 PVRIndexCalcNewPos(PVR_INDEX_HANDLE handle, MT_U32 u32FrmPos, MT_S32 s32Offset)
{
    MT_U32 u32NewPos = 0;

    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U32 u32LastFrame = 0;

    MT_U32 u32Boundary = 0;
    MT_U32 u32DiffToStart = 0;
    MT_U32 u32DiffToEnd = 0;

    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32EndFrame = handle->stCycMgr.u32EndFrame;
    u32LastFrame = handle->stCycMgr.u32LastFrame;

#ifdef DBG_FRAME_POS
    if(g_dbgFlag >= 0)
    {
        MT_ERR_PVR("\033[1;45;37m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Info start !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        MT_ERR_PVR("u32StartFrame = %u, u32EndFrame = %u, u32LastFrame = %u\n",u32StartFrame, u32EndFrame, u32LastFrame);
        MT_ERR_PVR("u32FrmPos = %u, s32Offset = %d\n",u32FrmPos, s32Offset);
        MT_ERR_PVR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Info end!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\033[0m\n");
    }
#else
    MT_INFO_PVR("\033[1;45;37m ################\n");
    MT_INFO_PVR("u32StartFrame = %u, u32EndFrame = %u, u32LastFrame = %u\n",u32StartFrame, u32EndFrame, u32LastFrame);
    MT_INFO_PVR("u32FrmPos = %u, s32Offset = %d\n",u32FrmPos, s32Offset);
    MT_INFO_PVR("##############################################\n\033[0m\n");
#endif

    /* check whether the frame position is valid or not. invalid, set it to start or end */
    if (!PVRIndexIsFrameValid(handle, u32FrmPos))
    {
#ifdef DBG_FRAME_POS
        MT_ERR_PVR("\033[1;45;37m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Info start !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        MT_ERR_PVR("u32StartFrame = %u, u32EndFrame = %u, u32LastFrame = %u\n",u32StartFrame, u32EndFrame, u32LastFrame);
        MT_ERR_PVR("u32FrmPos = %u, s32Offset = %d\n",u32FrmPos, s32Offset);
        MT_ERR_PVR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Info end!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\033[0m\n");
#endif
        if ( (u32StartFrame > u32EndFrame) && (u32FrmPos > u32EndFrame) && (u32FrmPos < u32StartFrame) )
        {
            u32DiffToStart = u32StartFrame - u32FrmPos;
            u32DiffToEnd = u32FrmPos - u32EndFrame;

            u32NewPos = (u32DiffToStart > u32DiffToEnd ) ? u32EndFrame : u32StartFrame;

            MT_WARN_PVR("Frame %u value invalid, set it to Frame %u, offset to end:%u, offset to start:%u\n",u32FrmPos,u32NewPos,u32DiffToEnd,u32DiffToStart);
        }
        else
        {
            /* more than u32LastFrame */
            u32NewPos = u32LastFrame;

            MT_WARN_PVR("Frame value invalid, set it to Frame %u\n",u32NewPos);
        }

        MT_WARN_PVR("\033[1;37;30mNow u32NewPos is %u in \n\033[0m", u32NewPos);

        return u32NewPos;
    }

    u32NewPos = u32FrmPos + (MT_U32)s32Offset;

    if(s32Offset >= 0)/* away from the u32FrmPos forward direction toward to u32EndFrame */
    {
        /*
            1. Not rewind (X stands for the sample point)
            --------------------------------------------------
            |                    /     \                     |
            |(offset direction)  ---|--- (offset direction)  |
            |                    \     /                     |
            ------------------------X-------------------------
            /\                      /\                      /\
            ||                      ||                      ||
            u32StartFrame        u32FrmPos     u32LastFrame(u32EndFrame)

            *****************************************************************

            2. Rewind (X1 and X2 stand for the sample point)
            -------------------------------------------
            |              |\    /\  |   u32FrmPos    |
            |              | \  /  \ |     ||         |
            |              |  \/    \|     \/         |
            ---X1--------------------------X2----------
            /\             /\        /\               /\
            ||             ||        ||               ||
            u32FrmPos  u32EndFrame u32StartFrame u32LastFrame

            In the above graphic, the "\    /\" stands for invalid frame position bound.
                                        \  /  \
                                         \/    \

            Notes:(offset forward direction)
            (1) Not rewind, the right boundary should always be u32LastFrame(u32EndFrame).
            (2) Rewind, sample point range from 0 to u32EndFrame, which is similar with Not rewind, the right boudary is u32EndFrame.
            (3) Rewind, sample point range from u32StartFrame to u32LastFrame.The right boundary as following:
                (a). Right boundary -- u32LastFrame, u32NewPos not over u32LastFrame.
                (b). Right boundary -- u32EndFrame, u32NewPos over u32LastFrame, which rewind to u32EndFrame, so it.

        */

        u32Boundary = u32EndFrame; /* not rewind, the boundary should be the u32EndFrame */

        if( (u32StartFrame >= u32EndFrame) && (u32FrmPos >= u32StartFrame) && (u32FrmPos <= u32LastFrame) )
        {
            if(u32NewPos > u32LastFrame)/* rewind, the frame is like X2 */
            {
                MT_WARN_PVR("Frame:%u rewind, offset:%d, LastFrame:%u\n",u32FrmPos, s32Offset, u32LastFrame);
                
                /* in this case, frame rewind to the u32EndFrame direction, so the right boundary should be the u32EndFrame */
                if (u32NewPos == u32LastFrame + 1)
                    u32NewPos = 0;
                else
                    u32NewPos -= u32LastFrame;
                MT_WARN_PVR("So subtract LastFrame, new frame:%u\n",u32NewPos);
            }
            else
            {
                /* in this case, frame is not real rewind, so the right boundary should be the u32LastFrame */
                u32Boundary = u32LastFrame;
            }
        }

        /* check the right boundary, make sure the value valid */
        u32NewPos = (u32NewPos > u32Boundary) ? u32Boundary : u32NewPos;
#ifdef DBG_FRAME_POS
        if (u32NewPos == 0)
        {
            MT_WARN_PVR("Seek forward, frame position:%u, u32Boundary = %d u32EndFrame = %d u32LastFrame = %d s32Offset = %d\n",u32NewPos,u32Boundary, u32EndFrame,u32LastFrame, s32Offset);
        }
#endif
    }
    else/* away from the u32FrmPos backward direction toward to u32StartFrame */
    {
        /*
           Notes:(offset backward direction)
            (1) Not rewind, the left boundary should always be u32StartFrame(that is ZERO).
            (2) Rewind, sample point range from u32StartFrame to u32LastFrame, which is similar with Not rewind,
                the left boudary is u32StartFrame(NOT ZERO).
            (3) Rewind, sample point range from 0 to u32EndFrame.The left boundary as following:
                (a). left boundary -- ZERO, u32NewPos more than ZERO.
                (b). left boundary -- u32StartFrame, u32NewPos less than ZERO, which rewind to u32StartFrame, so it.

        */
        u32Boundary = u32StartFrame; /* not rewind, the boundary should be the u32StartFrame */

        if( (u32StartFrame >= u32EndFrame) && (u32FrmPos <= u32EndFrame))
        {
            /* rewind*/
            if((MT_S32)u32NewPos < 0)
            {
                MT_WARN_PVR("Frame:%u rewind, offset is %d, LastFrame:%u\n",u32FrmPos, s32Offset, u32LastFrame);
                
                /* frame rewind to the range u32StartFrame to u32LastFrame, like Frame X1 */
                if ((MT_S32)u32NewPos == -1)
                    u32NewPos = u32LastFrame;
                else
                    u32NewPos = u32LastFrame + u32NewPos;
            }
            else
            {
                /* In this case, the left boundary should be zero, NOT u32StartFrame! */
                u32Boundary = 0;
                MT_WARN_PVR("Seek backward, ref frame:%u, u32EndFrame=%d u32LastFrame=%d s32Offset=%d u32NewPos=%d\n",u32FrmPos,u32Boundary, u32EndFrame,u32LastFrame,s32Offset,u32NewPos);
            }
        }

        /* check the left boundary, make sure the value valid.*/
        u32NewPos = ((MT_S32)u32NewPos < (MT_S32)u32Boundary) ? u32Boundary : u32NewPos;
#ifdef DBG_FRAME_POS
        if (u32NewPos == 0)
        {
            MT_WARN_PVR("Seek backward, frame position:%u, u32FrmPos = %d u32Boundary = %d u32EndFrame = %d u32LastFrame = %d s32Offset = %d\n",u32NewPos,u32FrmPos,u32Boundary, u32EndFrame,u32LastFrame,s32Offset);
        }
#endif
    }

#ifdef DBG_FRAME_POS
    if(abs(s32Offset) > 1)
    {
        MT_ERR_PVR("\033[1;42;34m!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! POSITION IS NOT EQUAL Info start !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        MT_ERR_PVR("u32StartFrame = %u, u32EndFrame = %u, u32LastFrame = %u\n",u32StartFrame, u32EndFrame, u32LastFrame);
        MT_ERR_PVR("new position = %u u32FrmPos = %u, s32Offset = %d\n",u32NewPos, u32FrmPos, s32Offset);
        MT_ERR_PVR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! POSITION IS NOT EQUAL Info end!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\033[0m\n");
        g_dbgFlag = MAX_PRINT_TIMES;
        nPrintTimes = 0;
    }

    if(g_dbgFlag >= 0)
    {
        MT_ERR_PVR("\033[1;37;30mNow u32NewPos is %u\n\033[0m", u32NewPos);
        g_dbgFlag--;
    }

    if(nPrintTimes == 0)
    {
        MT_ERR_PVR("\033[1;37;30mNow u32NewPos is %u \n\033[0m", u32NewPos);
        nPrintTimes = 1;
    }
#endif

    MT_INFO_PVR("\033[1;37;30mnow u32NewPos is %u in \n\033[0m", u32NewPos);

    return u32NewPos;
}
#endif


/* move s32Offset direction forward or backward, over the end or start, set it to the end or start */
STATIC INLINE MT_VOID PVRIndexCycMoveReadFrame(PVR_INDEX_HANDLE handle, MT_S32 s32Offset)
{
    /* in regardless of rewind, calculate the new position for read by the following interface. */
    handle->u32ReadFrame = PVRIndexCalcNewPos(handle, handle->u32ReadFrame, s32Offset);
}

/**
 * @brief Record cycle infomation
 *
 *
 *  @param[in] handle : The recording index handle
 *
 *  @retval :: MT_FAILURE on failure.
 *  @retval :: MT_SUCCESS on success.
 *
 *  @note
 *
 *  @see ::
 */
STATIC MT_S32 PVRIndexRecordCycInfo(PVR_INDEX_HANDLE handle, PVR_CYC_HEADER_INFO_S  *destCycInfo,MT_S32 force)
{
    PVR_CYC_HEADER_INFO_S stCycInfo={0,};
    MT_U32                u32CurrentTime;
    MT_U32                u32TimeDiff;
    MT_S32 s32WriteRet = sizeof(PVR_CYC_HEADER_INFO_S);
    
    //MT_ASSERT_RET((PVR_INDEX_HANDLE)NULL != handle);
    if(NULL == handle)
    {
        MT_ERR_PVR("index handle==null\n");
        return MT_FAILURE;
    }
    stCycInfo.u32StartFrame = handle->stCycMgr.u32StartFrame;
    stCycInfo.u32IsRewind   = handle->stCycMgr.bIsRewind;
    if (handle->stCycMgr.u32EndFrame <= handle->stCycMgr.u32StartFrame)
    {
        stCycInfo.u32EndFrame   = (0 == handle->stCycMgr.u32EndFrame)?0:(handle->stCycMgr.u32EndFrame - 1);
        stCycInfo.u32LastFrame  = handle->stCycMgr.u32LastFrame;
    }
    else
    {
        stCycInfo.u32EndFrame   = handle->stCycMgr.u32EndFrame - 1;
        stCycInfo.u32LastFrame  = handle->stCycMgr.u32LastFrame - 1;
    }    
    stCycInfo.u32TimeShiftEventLoopFirstNode  = handle->stCycMgr.u32Reserve;
    //MT_INFO_PVR("XXXXXXXXXXXXXXX record cyc info XXXXXXXXXXXXXXXX\n");
    /*
    MT_INFO_PVR("S:%d, E:%d, L:%d\n", handle->stCycMgr.u32StartFrame,
                handle->stCycMgr.u32EndFrame, handle->stCycMgr.u32LastFrame);
    */

    /*
    u32FileSize = handle->u32IdxStartOffsetLen + handle->stCycMgr.u32LastFrame*sizeof(PVR_INDEX_ENTRY_S);
    if(u32FileSize>=(PVR_INDEX_SPACE_MAX*handle->u32IndexSpaceNum))
    {
        ret = PVR_GetPatitionSpaceInfor(handle->szIdxFileName);
        if(MT_SUCCESS==ret)
        {
            handle->u32IndexSpaceNum++;
            PVR_CREATEI_INX_FILE(handle->s32WriteFd,handle->u32IndexSpaceNum,(handle->u32IdxStartOffsetLen + (handle->stCycMgr.u32LastFrame-1)*sizeof(PVR_INDEX_ENTRY_S)));
        }
    }*/
//lint -e774 -e413
    if ((PVR_INDEX_HANDLE)NULL != handle)
    {
	    s32WriteRet = PVR_WRITE((void *)&stCycInfo, (size_t)sizeof(PVR_CYC_HEADER_INFO_S),
                                 handle->s32HeaderFd, (off_t)PVR_GET_HEADER_OFFSET());
           memcpy(destCycInfo, &stCycInfo,  (size_t)sizeof(PVR_CYC_HEADER_INFO_S));                   
        /* after rewind case, update u64ValidSize in PVR_IDX_HEADER_INFO_S */
        PVR_WRITE((void *)&(handle->stCycMgr.u64MaxCycSize), sizeof(MT_U64),
	              handle->s32HeaderFd, ((off_t)PVR_GET_HEADER_OFFSET() + (off_t)sizeof(PVR_CYC_HEADER_INFO_S) + (off_t)sizeof(MT_U32)));
        //printf("stCycInfo.u32EndFrame=%x,%x\n",destCycInfo->u32EndFrame, destCycInfo->u32LastFrame);
    }
//lint +e774 +e413
    if ( s32WriteRet < 0)
    {
        MT_ERR_PVR("write cyc info err\n");
        return MT_FAILURE;
    }
    u32CurrentTime = PVRIndexGetCurTimeMs();
    if(handle->u32FflushTime==0)
    {
        handle->u32FflushTime = u32CurrentTime;
    }
    if(handle->u32FflushTime<=u32CurrentTime)
    {
        u32TimeDiff = u32CurrentTime - handle->u32FflushTime;
    }
    else
    {
        u32TimeDiff = 0xffffffff - handle->u32FflushTime + u32CurrentTime;
    }
    if((u32TimeDiff>=PVR_DIFF_FFLUSH_HEADINFO) || force)
    {
        handle->u32FflushTime = u32CurrentTime;
        PVR_Index_FlushIdxWriteCache(handle);       //write data
      	PVR_FSYNC(handle->s32HeaderFd);             //write header
    }
    return MT_SUCCESS;
}

INLINE MT_S32 PVRIndexSaveHeaderInfo(MT_U8 *indexFileName, PVR_IDX_HEADER_INFO_S* pHeadInfo)
{
    int s32Fd = -1;
    MT_S32 s32ReadRet = 0;
    if(indexFileName == NULL)  return  MT_FAILURE;
    s32Fd = PVR_OPEN((char*)indexFileName, PVR_FOPEN_MODE_INDEX_WRITE);
    if(s32Fd < 0)  return  MT_FAILURE;
    s32ReadRet =PVR_WRITE(pHeadInfo, sizeof(PVR_IDX_HEADER_INFO_S), s32Fd, 0);   //re-write header-zone
    if (s32ReadRet == 0)    
    {
        MT_ERR_PVR("read Header info err, ret:%d, fd:%d, size:%d\n", s32ReadRet, s32Fd, s32ReadRet);
        return MT_FAILURE;
    }
    PVR_CLOSE(s32Fd);
    return MT_SUCCESS;
}

MT_S32 PVRIndexGetHeaderInfoByName(MT_U8 *indexFileName, PVR_IDX_HEADER_INFO_S* pHeadInfo,MT_U32 size)
{
    int s32Fd = -1;
    MT_S32 s32ReadRet = 0;
    
    if(indexFileName == NULL)  return  MT_FAILURE;
    s32Fd = PVR_OPEN((char*)indexFileName, PVR_FOPEN_MODE_DATA_READ);
    if(s32Fd < 0)
    {
        MT_ERR_PVR("read Header open err, fd :%d\n", s32Fd);
        return  MT_FAILURE;
    }
    s32ReadRet = PVR_READ(pHeadInfo,size, s32Fd, 0);
    if (s32ReadRet == 0)    
    {
        MT_ERR_PVR("read Header info err, ret:%d, fd:%d, size:%d\n", s32ReadRet, s32Fd, s32ReadRet);
        memset(pHeadInfo, 0, size);
        return MT_FAILURE;
    }
    PVR_CLOSE(s32Fd);
    return MT_SUCCESS;
}

/* get the header struct info from index file */
INLINE MT_S32 PVRIndexGetHeaderInfo(MT_S32 s32Fd, PVR_IDX_HEADER_INFO_S* pHeadInfo)
{
    MT_S32 s32ReadRet = sizeof(PVR_IDX_HEADER_INFO_S);
    MT_S64 indexFileSize;
    MT_S32 tmpOffset;
    MT_U32 indexEntryNum;


    s32ReadRet = PVR_READ(pHeadInfo, sizeof(PVR_IDX_HEADER_INFO_S), s32Fd, 0);
    //if (s32ReadRet == (MT_S32)sizeof(PVR_IDX_HEADER_INFO_S))
    if (s32ReadRet == 0)    
    {
        MT_ERR_PVR("read Header info err, ret:%d, fd:%d, size:%d\n", s32ReadRet, s32Fd, s32ReadRet);
        memset(pHeadInfo, 0, sizeof(PVR_IDX_HEADER_INFO_S));
        return MT_FAILURE;
    }

    if (PVR_INDEX_HEADER_CODE != pHeadInfo->u32StartCode)
    {
        MT_ERR_PVR("Header info StartCode:0x%x, No head at this file, still play.\n", pHeadInfo->u32StartCode);
        memset(pHeadInfo, 0, sizeof(PVR_IDX_HEADER_INFO_S));
        return MT_FAILURE;
    }

    /* for temp use, TODO: we must make sure the index file biger than cycInfo */
    tmpOffset = (MT_S32)pvr_lseek(s32Fd, 0, SEEK_CUR);
    if (tmpOffset < 0)
    {
        MT_ERR_PVR("can't seek to 0.\n");
        memset(pHeadInfo, 0, sizeof(PVR_IDX_HEADER_INFO_S));
        return MT_FAILURE;
    }
    indexFileSize = (MT_S64)pvr_lseek(s32Fd, 0, SEEK_END);
    pvr_lseek(s32Fd, tmpOffset, SEEK_SET);

    indexEntryNum = (MT_U32)(((MT_U64)indexFileSize - (MT_U64)pHeadInfo->u32HeaderLen)/(MT_U64)sizeof(PVR_INDEX_ENTRY_S));

    if (pHeadInfo->stCycInfo.u32EndFrame > indexEntryNum)
    {
        MT_ERR_PVR("HeadInfo's CycInfo.EndFrame(%u) > indexEntryNum(%u).\n", pHeadInfo->stCycInfo.u32EndFrame, indexEntryNum);
        pHeadInfo->stCycInfo.u32EndFrame = indexEntryNum;
    }

    if (pHeadInfo->stCycInfo.u32LastFrame > indexEntryNum)
    {
        MT_ERR_PVR("HeadInfo's CycInfo.LastFrame(%u) > indexEntryNum(%u).\n", pHeadInfo->stCycInfo.u32LastFrame, indexEntryNum);
        pHeadInfo->stCycInfo.u32LastFrame = indexEntryNum;
    }

    return MT_SUCCESS;
}

/* read the header info, and allowed without header info */
static MT_S32 PVRIndexReadHeaderInfo(PVR_INDEX_HANDLE handle,PVR_IDX_HEADER_INFO_S *outinfor)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo;

    memset(&stIdxHeaderInfo, 0, sizeof(PVR_IDX_HEADER_INFO_S));

    /* means not found the header info in this index file */
    if (MT_SUCCESS != PVRIndexGetHeaderInfo(handle->s32HeaderFd, &stIdxHeaderInfo))
    {
        return MT_FAILURE;
    }
    else
    {
        handle->u32IdxStartOffsetLen =  stIdxHeaderInfo.u32HeaderLen;
        handle->stCycMgr.bIsRewind = (MT_BOOL)(stIdxHeaderInfo.stCycInfo.u32IsRewind);
        handle->stCycMgr.u32StartFrame = stIdxHeaderInfo.stCycInfo.u32StartFrame;
        handle->stCycMgr.u32EndFrame = stIdxHeaderInfo.stCycInfo.u32EndFrame;
        handle->stCycMgr.u32LastFrame = stIdxHeaderInfo.stCycInfo.u32LastFrame;
        handle->stCycMgr.u64MaxCycSize = stIdxHeaderInfo.u64ValidSize;

        handle->u32ReadFrame = handle->stCycMgr.u32StartFrame;
        if(outinfor){
            memcpy(outinfor,&stIdxHeaderInfo,sizeof(PVR_IDX_HEADER_INFO_S));
        }
        PVR_EVENT_D("++load from file:%x,%x,%x\n",handle->stCycMgr.u32StartFrame,handle->stCycMgr.u32EndFrame,handle->stCycMgr.u32LastFrame);
    }

    return MT_SUCCESS;
}

/* initialize */
STATIC INLINE MT_VOID PVRIndexSetDftAttr(PVR_INDEX_HANDLE handle)
{
    MT_INFO_PVR("index set default attr.\n");

    handle->u64GlobalOffset = 0;
    handle->u32LastDavBufOffset = 0;
    handle->u32PauseFrame  = 0;
    handle->u64PauseOffset = PVR_INDEX_PAUSE_INVALID_OFFSET;
    handle->u32ReadFrame  = 0;
    handle->u32WriteFrame = 0;
    handle->u16RecLastIframe = PVR_INDEX_INVALID_I_FRAME_OFFSET;
    handle->u32RecLastValidPtsMs = PVR_INDEX_INVALID_PTSMS;
    handle->u32RecPicParser = 0xffffffff;
    handle->u16RecUpFlowFlag = 0;
    handle->u32RecFirstFrmTimeMs = 0;
    handle->u32RecReachPlay = 0;

    handle->s32WriteFd = PVR_FILE_INVALID_FILE;
    handle->s32ReadFd = PVR_FILE_INVALID_FILE;
    handle->s32SeekFd = PVR_FILE_INVALID_FILE;
    handle->s32HeaderFd = PVR_FILE_INVALID_FILE;
    handle->u32IdxStartOffsetLen = 0;
    memset(&handle->stCurPlayFrame, 0, sizeof(PVR_INDEX_ENTRY_S) );
    memset(&handle->stCurRecFrame, 0, sizeof(PVR_INDEX_ENTRY_S) );
    memset(&handle->stIndexFileAttr, 0, sizeof(MT_UNF_PVR_FILE_ATTR_S) );
    memset(&handle->stCycMgr, 0, sizeof(PVR_CYC_MGR_S));
    memset(handle->szIdxFileName, 0, PVR_MAX_FILENAME_LEN+4);
    //memset(&handle->appendrec, 0, sizeof(PVR_APPEND_REC));
}

static MT_S32 PVRCacheWriteIdx(MT_U32 u32ChnID, MT_U32 InstIdx,MT_U8* pu8Data,MT_U32 u32Bytes2Write,MT_U32 u32Offset,
                                   MT_U32 u32DirectFlag,MT_U32* u32WriteFlag)
{
    PVR_INDEX_HANDLE    handle;
    MT_U32 u32SaveSz;
    handle = &g_stPVRIndex[InstIdx + PVR_PLAY_MAX_CHN_NUM];
    
    //save event, update index info
    MT_PVR_Update_Index_Event_Rec(u32ChnID, pu8Data, u32Bytes2Write);

    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        mt_u32 recChannelId = u32ChnID - PVR_REC_START_NUM;
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_index_flush)
            {
                if(MPVR_ADDON_STATUS_OK != p_addon->rec_inter.on_index_flush(p_addon->handlesrec[recChannelId], pu8Data, 
                u32Bytes2Write, u32Offset/sizeof(PVR_INDEX_ENTRY_S)))
                {
                    MT_FATAL_PVR("addon flush index faile.\n");
                    return MT_FAILURE;
                }
            }
        }
    }
    PVR_IDX_CACHE_LOCK_W(handle);
    
    if (u32DirectFlag || handle->stIdxWriteCache.u32BufferLen == 0)/*direct write*/
    {
        //lint -e774
        if (handle->stIdxWriteCache.u32BufferLen && handle->stIdxWriteCache.u32UsedSize)/*have data cache*/
        {
            if ((u32Offset == (handle->stIdxWriteCache.u32StartOffset + handle->stIdxWriteCache.u32UsedSize)) &&
                ((handle->stIdxWriteCache.u32BufferLen - handle->stIdxWriteCache.u32UsedSize) >= u32Bytes2Write))/*data Contiguous*/
            {
                memcpy((handle->stIdxWriteCache.pu8Addr + handle->stIdxWriteCache.u32UsedSize),pu8Data,u32Bytes2Write);
                PVR_WRITE_INDEX(u32SaveSz, (handle->stIdxWriteCache.u32UsedSize + u32Bytes2Write),handle->stIdxWriteCache.pu8Addr,
                                handle->s32WriteFd,handle->stIdxWriteCache.u32StartOffset, handle);
                *u32WriteFlag = 1;
                handle->stIdxWriteCache.u32UsedSize = 0;
                handle->stIdxWriteCache.u32StartOffset = 0;
            }
            else
            {
                PVR_WRITE_INDEX(u32SaveSz, handle->stIdxWriteCache.u32UsedSize,handle->stIdxWriteCache.pu8Addr,
                                handle->s32WriteFd,handle->stIdxWriteCache.u32StartOffset, handle);
                PVR_WRITE_INDEX(u32SaveSz, u32Bytes2Write, pu8Data, handle->s32WriteFd,u32Offset, handle);
                *u32WriteFlag = 1;
                handle->stIdxWriteCache.u32UsedSize = 0;
                handle->stIdxWriteCache.u32StartOffset = 0;
            }
        }
        else/*no cache data*/
        {
            PVR_WRITE_INDEX(u32SaveSz, u32Bytes2Write, pu8Data, handle->s32WriteFd,u32Offset, handle);
            *u32WriteFlag = 1;
        }
        //lint +e774
    }
    else
    {
        /*data not Contiguous or buffer not enough,clear the cache first*/
        if ((u32Offset != (handle->stIdxWriteCache.u32StartOffset + handle->stIdxWriteCache.u32UsedSize))
            ||
            ((handle->stIdxWriteCache.u32BufferLen - handle->stIdxWriteCache.u32UsedSize) < u32Bytes2Write)) 
            
        {
            //lint -e774
            PVR_WRITE_INDEX(u32SaveSz, handle->stIdxWriteCache.u32UsedSize,handle->stIdxWriteCache.pu8Addr, 
                            handle->s32WriteFd,handle->stIdxWriteCache.u32StartOffset, handle);
            *u32WriteFlag = 1;
            handle->stIdxWriteCache.u32UsedSize = 0;
            handle->stIdxWriteCache.u32StartOffset = 0;
            //lint +e774
        }
        /*1.data Contiguous && buffer enough;2.buffer is empty;----cache it*/
        memcpy(handle->stIdxWriteCache.pu8Addr + handle->stIdxWriteCache.u32UsedSize,pu8Data,u32Bytes2Write);
        if(handle->stIdxWriteCache.u32UsedSize == 0)//cache is empty
        {
           handle->stIdxWriteCache.u32StartOffset  = u32Offset;
        }
        handle->stIdxWriteCache.u32UsedSize +=  u32Bytes2Write;            
    }
    PVR_IDX_CACHE_UNLOCK_W(handle);
    
    PVR_IDX_CACHE_LOCK_R(handle);
    if (PVR_Index_IfOffsetReadCache(handle,u32Offset,u32Bytes2Write))//write data been cached or appears in cache
    {
        handle->stIdxReadCache.u32UsedSize = 0;//invalid the read cache buffer
    }
    PVR_IDX_CACHE_UNLOCK_R(handle);
    
    return MT_SUCCESS;
}

MT_S32 PVR_Index_FlushIdxWriteCache(PVR_INDEX_HANDLE    handle)
{
    MT_U32 u32SaveSz;
    if (handle->stIdxWriteCache.u32BufferLen == 0)
    {
        return MT_SUCCESS;
    }
    PVR_IDX_CACHE_LOCK_W(handle);
    if (handle->stIdxWriteCache.u32UsedSize)
    {
        //lint -e774
        PVR_WRITE_INDEX(u32SaveSz, handle->stIdxWriteCache.u32UsedSize,handle->stIdxWriteCache.pu8Addr, 
                            handle->s32WriteFd,handle->stIdxWriteCache.u32StartOffset, handle);
        handle->stIdxWriteCache.u32UsedSize = 0;
        handle->stIdxWriteCache.u32StartOffset = 0;
        memset(handle->stIdxWriteCache.pu8Addr, 0x5a, handle->stIdxWriteCache.u32BufferLen);
        //lint +e774
    }
    PVR_IDX_CACHE_UNLOCK_W(handle);
    return MT_SUCCESS;
}

MT_S32 PVR_Index_GetLastIdxWriteCacheIndexEntry(PVR_INDEX_HANDLE    handle,PVR_INDEX_ENTRY_S *last_index)
{
    MT_U32 ret = MT_SUCCESS;
    if (handle->stIdxWriteCache.u32BufferLen == 0)
    {
        return !MT_SUCCESS;
    }
    PVR_IDX_CACHE_LOCK_W(handle);
    if (handle->stIdxWriteCache.u32UsedSize)
    {
        memcpy(last_index,(handle->stIdxWriteCache.pu8Addr + handle->stIdxWriteCache.u32UsedSize-sizeof(PVR_INDEX_ENTRY_S)),sizeof(PVR_INDEX_ENTRY_S));
    }else
    {
        ret = !MT_SUCCESS;
    }
    
    PVR_IDX_CACHE_UNLOCK_W(handle);
    return ret;
}

/*check if the offset in cache:1 in cache,0 not in cache*/
MT_S32 PVR_Index_IfOffsetInWriteCache(PVR_INDEX_HANDLE  handle,MT_U32 u32Offset,MT_U32 u32Size)
{
    if (handle->stIdxWriteCache.u32BufferLen == 0)
    {
        return 0;
    }
    PVR_IDX_CACHE_LOCK_W(handle);
    if (handle->stIdxWriteCache.u32BufferLen != 0 && handle->stIdxWriteCache.u32UsedSize)
    {
        if ((u32Offset >= handle->stIdxWriteCache.u32StartOffset && 
            u32Offset <= (handle->stIdxWriteCache.u32StartOffset + handle->stIdxWriteCache.u32UsedSize)) ||
            (u32Offset < handle->stIdxWriteCache.u32StartOffset && 
            ((u32Offset+u32Size) > handle->stIdxWriteCache.u32StartOffset)))
        {
            PVR_IDX_CACHE_UNLOCK_W(handle);
            return 1;
        }
    }
    PVR_IDX_CACHE_UNLOCK_W(handle);
    return 0;
}

/*check if the offset in read cache:1 all in cache,0 not in cache,2 offset in cache*/
static MT_S32 PVR_Index_IfOffsetReadCache(PVR_INDEX_HANDLE  handle,MT_U32 u32Offset,MT_U32 u32Size)
{
    if (handle->stIdxReadCache.u32BufferLen && handle->stIdxReadCache.u32UsedSize)/*have data cached*/
    {
        if (u32Offset >= handle->stIdxReadCache.u32StartOffset && 
            ((u32Offset-handle->stIdxReadCache.u32StartOffset) <= handle->stIdxReadCache.u32UsedSize))
        {
            if ((u32Offset - handle->stIdxReadCache.u32StartOffset + u32Size) <= handle->stIdxReadCache.u32UsedSize)
            {
                return 1;
            }
            return 2;
        }
    }
    return 0;
}


static ssize_t PVRCacheReadIdx(PVR_INDEX_HANDLE  handle,PVR_FILE fd, MT_VOID* pData,size_t size, 
                              MT_U32 offset,MT_U32 u32DirectFlag)
{
    ssize_t readNum = 0;
    MT_S32 s32CachedFlag;
    MT_S32 s32CacheStartReadOffset;
    MT_U8* pDataAddr;
    static MT_U32 u32CacheNum = 0,NotCacheNum = 0;
    
    PVR_IDX_CACHE_LOCK_R(handle);
    if (handle->stIdxReadCache.u32BufferLen == 0 || u32DirectFlag || 
        size > handle->stIdxReadCache.u32BufferLen)/*read directly*/
    {
        PVR_READ_INDEX_DIRECTLY(readNum, pData, size, fd, offset, handle);
        NotCacheNum++;
    }
    else
    {
        s32CachedFlag = PVR_Index_IfOffsetReadCache(handle,offset,size);
        if (s32CachedFlag == 1)/*cached*/
        {
            pDataAddr = handle->stIdxReadCache.pu8Addr + offset - handle->stIdxReadCache.u32StartOffset;
            memcpy(pData,pDataAddr,size);
            readNum = (ssize_t)size;
            u32CacheNum ++;
        }
        else/*not cached*/
        {
            /*
            flush cache buffer:
            --------------------------------------------------
            |                    /     \                     |
            |       offset in middle of cache buffer         |
            |                    \     /                     |
            ------------------------X-------------------------
            /\                      /\                      /\
            ||                      ||                      ||
            cache start        offset          cache end
            */
            s32CacheStartReadOffset = (MT_S32)(offset - (handle->stIdxReadCache.u32BufferLen / 2));
            if (s32CacheStartReadOffset < 0)
            {
                s32CacheStartReadOffset = 0;
            }
            PVR_READ_INDEX_DIRECTLY(readNum, handle->stIdxReadCache.pu8Addr, handle->stIdxReadCache.u32BufferLen, 
                                    fd, (MT_U32)s32CacheStartReadOffset, handle);
            if(0 >= readNum){ //read error,set valid=0.fix bug123874
                handle->stIdxReadCache.u32UsedSize = (mt_u32)0;
            }else{
                handle->stIdxReadCache.u32UsedSize = (mt_u32)readNum;
            }
            handle->stIdxReadCache.u32StartOffset = (mt_u32)s32CacheStartReadOffset;
            s32CachedFlag = PVR_Index_IfOffsetReadCache(handle,offset,size);/*check again*/
            if (s32CachedFlag == 1)
            {
                pDataAddr = handle->stIdxReadCache.pu8Addr + offset - handle->stIdxReadCache.u32StartOffset;
                memcpy(pData,pDataAddr,size);
                readNum = (ssize_t)size;
                NotCacheNum++;
            }
            else/*try read directly */
            {
                MT_WARN_PVR("idx read cache not works!\n");
                PVR_READ_INDEX_DIRECTLY(readNum, pData, size, fd, offset, handle); 
                NotCacheNum++;
            }
        }         
    }
    if ((!(u32CacheNum%5000 ) && u32CacheNum != 0) || (!(NotCacheNum %5000) && NotCacheNum != 0))
    {
//        MT_INFO_PVR(">>>> u32CacheNum:%d,NotCacheNum:%d\n",u32CacheNum,NotCacheNum);
    }
    PVR_IDX_CACHE_UNLOCK_R(handle);
    if(readNum > 0 && NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply && p_addon->play_inter.on_index_check)
        {
            if(MPVR_ADDON_STATUS_OK != p_addon->play_inter.on_index_check(p_addon->handlesply, pData, 
            readNum,offset/sizeof(PVR_INDEX_ENTRY_S)))
            {
                MT_FATAL_PVR("addon check index faile.\n");
                readNum = -1;
                handle->stIdxReadCache.u32UsedSize = 0;
            }
        }
    }
    return readNum;
}

#ifdef PVR_REWIND_ADJUST_START_FRAME
static MT_S32 PVR_Index_Adjust_StartFrame(PVR_INDEX_HANDLE    handle,PVR_CYC_MGR_S *pCycMgr,PVR_INDEX_ENTRY_S *indexEntry)
{
    PVR_INDEX_ENTRY_S startEntry={0};
    //PVR_INDEX_ENTRY_S endEntry={0};
    MT_S32 ret = 0;
    MT_U64 offset = 0;
    MT_U32 pre_startframe = 0;
    //MT_U32 time_start = 0,time_end = 0;

    if(pCycMgr->s32CycTimes > 0 && (pCycMgr->u32EndFrame < pCycMgr->u32StartFrame))
    {
        //MT_PVR_SysGetTimeStampMs(&time_start);
        if(g_pvr_rewind_adjust_count > PVR_REWIND_MAX_ADJUST_FRAMES)
            return MT_SUCCESS;

        //printf("%s s32CycTimes=%d s=%d e=%d diff=%d l=%d r=%d\n",__FUNCTION__,pCycMgr->s32CycTimes,
        //    pCycMgr->u32StartFrame,pCycMgr->u32EndFrame,(pCycMgr->u32StartFrame-pCycMgr->u32EndFrame ),pCycMgr->u32LastFrame,handle->u32ReadFrame);
        memset(&startEntry, 0, sizeof(PVR_INDEX_ENTRY_S));
        #if 0
        memset(&endEntry, 0, sizeof(PVR_INDEX_ENTRY_S));
        ret = PVRIndexGetEntryByNum(handle,&endEntry,pCycMgr->u32EndFrame);
        if(ret != MT_SUCCESS)
        {   
            MT_ERR_PVR("get end frame(%d) index entry failed, ret=0x%x\n",pCycMgr->u32EndFrame,ret);
            return MT_FAILURE;
        }    
        #endif
        ret = PVRIndexGetEntryByNum(handle,&startEntry,pCycMgr->u32StartFrame);
        if(ret != MT_SUCCESS)
        {   
            MT_ERR_PVR("get start frame(%d) index entry failed, ret=0x%x\n",pCycMgr->u32StartFrame,ret);
            return MT_FAILURE;
        }
        offset = (indexEntry->u64Offset + indexEntry->u32FrameSize);
        //printf("u64Offset+u32FrameSize=%lld u64MaxCycSize=%lld\n",offset,pCycMgr->u64MaxCycSize);
        //printf("u64Offset+u32FrameSize=%lld (offset=%lld,size=%d) start u64Offset=%lld size=%d diff=%lld s=%d e=%d diff=%d\n",
        //offset,indexEntry->u64Offset,indexEntry->u32FrameSize, startEntry.u64Offset,startEntry.u32FrameSize,(startEntry.u64Offset-offset),pCycMgr->u32StartFrame,pCycMgr->u32EndFrame,(pCycMgr->u32StartFrame-pCycMgr->u32EndFrame+1));
        //if((indexEntry->u64Offset + indexEntry->u32FrameSize) > pCycMgr->u64MaxCycSize)  
        if(offset > startEntry.u64Offset)
        {            
#if 0
            MT_U8 frame_type = PVR_INDEX_get_frameType(indexEntry);
            MT_U32 gop = (mt_u32)((indexEntry->u16FrameTypeAndGop & 0x3fff) + 1);
            printf("frame_type=%d u32FrameSize=%d gop=%d\n",frame_type,indexEntry->u32FrameSize,gop);
#endif
            pre_startframe = pCycMgr->u32StartFrame;
            MT_WARN_PVR("u64Offset+u32FrameSize=%lld start u64Offset=%lld s32CycTimes=%d s=%d e=%d diff=%d r=%d l=%d\n",
                offset,startEntry.u64Offset,pCycMgr->s32CycTimes,pCycMgr->u32StartFrame,pCycMgr->u32EndFrame,(pCycMgr->u32StartFrame-pCycMgr->u32EndFrame),
                handle->u32ReadFrame,pCycMgr->u32LastFrame);
            //offset = (indexEntry->u64Offset + indexEntry->u32FrameSize) % pCycMgr->u64MaxCycSize;
            //adjust start frame
            while(startEntry.u64Offset < offset)
            {
                if((pCycMgr->u32StartFrame +1)  > pCycMgr->u32LastFrame){
                    MT_ERR_PVR("adjust start frame(%d) ++ will over last frame=%d\n",pCycMgr->u32StartFrame,pCycMgr->u32LastFrame);
                   return MT_FAILURE;
                } 
                pCycMgr->u32StartFrame ++;                  
                ret = PVRIndexGetEntryByNum(handle,&startEntry,pCycMgr->u32StartFrame);
                if(ret != MT_SUCCESS)
                {   
                    MT_ERR_PVR("retry get start frame(%d) index entry failed, ret=0x%x\n",pCycMgr->u32StartFrame,ret);
                    return MT_FAILURE;
                }
            }
            g_pvr_rewind_adjust_count += (pCycMgr->u32StartFrame-pre_startframe);
            MT_WARN_PVR("after adjust start frame form %d to %d adjust=%d diff=%d count=%d\n",
                pre_startframe,pCycMgr->u32StartFrame,(pCycMgr->u32StartFrame-pre_startframe),(pCycMgr->u32StartFrame-pCycMgr->u32EndFrame),g_pvr_rewind_adjust_count);
            
        }

        #if 0
        MT_PVR_SysGetTimeStampMs(&time_end);
        if((time_end-time_start) > 10)
            printf("%s  use time=%d\n",__FUNCTION__,(time_end-time_start));
        #endif
    }
    return MT_SUCCESS;
}
#endif


/** save valid index into the file, called by FIDX_FeedStartCode.
 *
 *  @param[in] InstIdx
 *  @param[in] pstDmxIndexInfo
 *
 *  @retval ::MT_SUCCESS
 *  @retval ::
 *
 *  @note
 *
 *  @see ::
 */
MT_S32 PVR_Index_SaveFramePosition(MT_U32 u32ChnID, MT_U32 InstIdx, MT_UNF_DMX_REC_INDEX_S *pstDmxIndexInfo,MT_U32 u32DirectFlag)
{
    PVR_INDEX_HANDLE    handle;
    MT_U32              byte2Save;
    PVR_INDEX_ENTRY_S   indexEntry;
    PVR_INDEX_ENTRY_S   startEntry;	
	PVR_INDEX_ENTRY_S	TimeRewindEntry;
    PVR_CYC_MGR_S       *pCycMgr;
 //   MT_U32              u32CurFrmTimeMs;
	//MT_S32              u32CurFrmTimeMs;
    //MT_U32              u32TimeNow = 0;
    /*MT_U64              u64CurCycTimeMs = 0;  */
    MT_BOOL             bRewindFlg = MT_FALSE;
    MT_S32              Ret;
    MT_U32              u32WriteFlag = 0;
 	MT_U32              TimeRewindEntryNum = 0;
	MT_U32              TimeRewindUpdateEntrySize = 0;
#ifdef PVR_DEBUG_REWIND_TIME
    MT_U32 time_start = 0,time_temp= 0;
    static MT_U32 pre_rewind_time = 0,pre_check_time = 0;
    MT_U32 rewind_time = 0;
#endif
    
    handle = &g_stPVRIndex[InstIdx + PVR_PLAY_MAX_CHN_NUM];
    pCycMgr = &(handle->stCycMgr);
    memset(&indexEntry,0,sizeof(PVR_INDEX_ENTRY_S));
    memset(&startEntry, 0, sizeof(PVR_INDEX_ENTRY_S));

    if ((MT_UNF_FRAME_TYPE_I != pstDmxIndexInfo->enFrameType) &&
        (MT_UNF_FRAME_TYPE_P != pstDmxIndexInfo->enFrameType) &&
        (MT_UNF_FRAME_TYPE_B != pstDmxIndexInfo->enFrameType))
    {
        #ifdef PVR_DEBUG_REWIND_TIME
        printf("%s %d invalid frame type=%d\n",__FUNCTION__,__LINE__,pstDmxIndexInfo->enFrameType);
        #endif
        return MT_SUCCESS;
    }

    if (PVR_INDEX_INVALID_I_FRAME_OFFSET != handle->u16RecLastIframe)
    {
        handle->u16RecLastIframe++;
    }
    else
    {
        handle->u16RecLastIframe = 0;
    }

    if(pCycMgr->u32StartFrame == 0 && pCycMgr->u32EndFrame == 0 && MT_UNF_FRAME_TYPE_I != pstDmxIndexInfo->enFrameType) {
        //printf("discard frame type %d, pts 0x%x, ofs %#llx\n", pstDmxIndexInfo->enFrameType, pstDmxIndexInfo->u32PtsMs, pstDmxIndexInfo->u64GlobalOffset);
        return MT_SUCCESS;
    }

    if (MT_UNF_FRAME_TYPE_I == pstDmxIndexInfo->enFrameType)
    {
        handle->u16RecLastIframe = 0;
    }
    else
    {
//        MT_INFO_PVR("Get a Frame: %d, %d, %lld\n", pstDmxIndexInfo->eFrameType, pstDmxIndexInfo->s32FrameSize, pstDmxIndexInfo->s64GlobalOffset);
    }

    if ((PVR_INDEX_INVALID_PTSMS != pstDmxIndexInfo->u32PtsMs) && (0 != pstDmxIndexInfo->u32PtsMs))
    {
        handle->u32RecLastValidPtsMs = pstDmxIndexInfo->u32PtsMs;
    }
/*
    if ((0 == pCycMgr->s32CycTimes) && (0 == handle->u32WriteFrame))
    {
        handle->u32RecFirstFrmTimeMs = handle->u32DmxClkTimeMs;// handle->u32RecLastValidPtsMs;///PVRIndexGetCurTimeMs();
        u32CurFrmTimeMs = 0;
        indexEntry.u32DisplayTimeMs = 0;
    }
    else
    {
        u32TimeNow = handle->u32DmxClkTimeMs;///PVRIndexGetCurTimeMs();
        if (u32TimeNow >= handle->stCurRecFrame.u32DisplayTimeMs)
        {
            u32CurFrmTimeMs = (mt_s32)(u32TimeNow - handle->u32RecFirstFrmTimeMs);
        }
        else
        {
            MT_WARN_PVR("The time rewinded firstTimeMs(%u)\n", handle->u32RecFirstFrmTimeMs);
            if ((handle->u32FRollTime*PVR_INDEX_SCD_WRAP_MS + u32TimeNow) < handle->stCurRecFrame.u32DisplayTimeMs)
            {
                handle->u32FRollTime ++;
            }
            u32CurFrmTimeMs = (mt_s32)((handle->u32FRollTime*PVR_INDEX_SCD_WRAP_MS) - handle->u32RecFirstFrmTimeMs + u32TimeNow);
            MT_WARN_PVR("cur indexEntry.u32DisplayTimeMs = %u set the reference time is %u, displayTimeMs increase = %u\n",indexEntry.u32DisplayTimeMs, u32TimeNow,u32CurFrmTimeMs);
        }
    }
	  MT_INFO_PVR("\r\n u32TimeNow:%d, stCurRecFrame.u32DisplayTimeMs:%d,u32CurFrmTimeMs:%d",u32TimeNow, handle->stCurRecFrame.u32DisplayTimeMs,u32CurFrmTimeMs);	
    MT_INFO_PVR("\r\n  u32LastDispTime:%d, ref:%u",  handle->u32LastDispTime,handle->u32RecFirstFrmTimeMs);
*/
    indexEntry.u16FrameTypeAndGop = (mt_u16)(((((pstDmxIndexInfo->enFrameType) & 0x3) << 14)) | (handle->u16RecLastIframe & 0x3fff));
    indexEntry.u16UpFlowFlag = (mt_u8)handle->u16RecUpFlowFlag;
    indexEntry.s32CycTimes = (MT_U32)pCycMgr->s32CycTimes;
    indexEntry.u64GlobalOffset = (MT_U64)pstDmxIndexInfo->u64GlobalOffset;
    indexEntry.u8StcOffsinTs=(mt_u8)pstDmxIndexInfo->u32HdrData[0];  //reuse!
    //printf("++off=%llx,off=%x\n",indexEntry.u64GlobalOffset,indexEntry.u8StcOffsinTs);
    indexEntry.u32FrameSize = (MT_U32)pstDmxIndexInfo->u32FrameSize;
    indexEntry.u32PtsMs = handle->u32RecLastValidPtsMs;
    indexEntry.u16IndexType = (MT_U16)handle->enIndexType;
    indexEntry.u161stFrameOfTT = 0;
    /* In case the recording data lose, compensate the current time */
    /*
    if ((u32CurFrmTimeMs - handle->u32LastDispTime) >= 1000)
    {
        handle->u32DeltaDispTimeMs += (u32CurFrmTimeMs - handle->u32LastDispTime);
    }
    */
    indexEntry.u32DisplayTimeMs = pstDmxIndexInfo->u32DataTimeMs;//indexEntry.u32DisplayTimeMs = u32CurFrmTimeMs - handle->u32DeltaDispTimeMs;
    //printf("+++pts:%x,%x\n",indexEntry.u32PtsMs,indexEntry.u32DisplayTimeMs);
	  //MT_INFO_PVR("\r\n u32DeltaDispTimeMs:%d, indexEntry.u32DisplayTimeMs:%d\n", handle->u32DeltaDispTimeMs, indexEntry.u32DisplayTimeMs);
    //handle->u32LastDispTime = u32CurFrmTimeMs;

    byte2Save = sizeof(PVR_INDEX_ENTRY_S);
    handle->u16RecUpFlowFlag = 0;

    PVR_INDEX_LOCK(handle);

#ifdef PVR_DEBUG_REWIND_TIME
    if(pre_rewind_time == 0){
        MT_PVR_SysGetTimeStampMs(&pre_rewind_time);
        printf("%s %d get pre rewind time\n",__FUNCTION__,__LINE__);
    }
#endif
    pCycMgr->u64AllRecNowInMs = indexEntry.u32DisplayTimeMs;
    if (PVR_INDEX_REWIND_BY_BOTH == pCycMgr->enRewindType)
    {
        if((pCycMgr->u64AllRecNowInMs-pCycMgr->u64AllRecStartInMs) > pCycMgr->u64MaxCycTimeInMs)
        {
            pCycMgr->enRewindType = PVR_INDEX_REWIND_BY_TIME;
        }
    
        if (pstDmxIndexInfo->u64GlobalOffset > pCycMgr->u64MaxCycSize)
        {
            pCycMgr->enRewindType = PVR_INDEX_REWIND_BY_SIZE;
        }
    }

    if (PVR_IDX_IS_REWIND(handle))
    {
        /*time  rewind*/   
        if (PVR_INDEX_REWIND_BY_TIME == pCycMgr->enRewindType)
        {
			indexEntry.u64Offset = (MT_U64)pstDmxIndexInfo->u64GlobalOffset - handle->u64TimeRewindMaxSize;
            
            #ifdef PVR_DEBUG_REWIND_TIME
            MT_PVR_SysGetTimeStampMs(&time_temp);
            if((time_temp - pre_check_time) > 1000){
                printf("rectime=%llu(%llu,%llu) u64MaxCycTimeInMs=%llu s32CycTimes=%d\n",(pCycMgr->u64AllRecNowInMs-pCycMgr->u64AllRecStartInMs),
                        pCycMgr->u64AllRecNowInMs,pCycMgr->u64AllRecStartInMs,
                        pCycMgr->u64MaxCycTimeInMs,pCycMgr->s32CycTimes);
                pre_check_time = time_temp;
            }	    
            #endif
				
            if(((pCycMgr->u64AllRecNowInMs-pCycMgr->u64AllRecStartInMs) - (pCycMgr->u64MaxCycTimeInMs * (mt_u64)pCycMgr->s32CycTimes)) > pCycMgr->u64MaxCycTimeInMs)
            {
                handle->bTimeRewindFlg = MT_TRUE;
                #ifdef PVR_DEBUG_REWIND_TIME
                MT_PVR_SysGetTimeStampMs(&time_start);
                #endif

                if ((MT_TRUE == handle->bTimeRewindTsPauseFlg) &&
                    (pstDmxIndexInfo->u64GlobalOffset > handle->u64TimeTsPausePos))
                {
                    pCycMgr->u64MaxCycSize = handle->u64TimeTsPausePos - handle->u64TimeRewindMaxSize;
                    #ifdef PVR_DEBUG_REWIND_TIME
                    MT_PVR_SysGetTimeStampMs(&time_temp);
                    printf("get bTimeRewindTsPauseFlg use time=%d u64MaxCycSize=%llu\n",(time_temp-time_start),pCycMgr->u64MaxCycSize);
                    #endif
                    /* update the index entry u64Offset nearby the point, which was beyond the max cycle size */
                    TimeRewindEntryNum = pCycMgr->u32EndFrame - 1;
                    (void)PVR_Index_FlushIdxWriteCache(handle);
                    if (MT_SUCCESS == PVRIndexGetEntryByNum(handle,&TimeRewindEntry,TimeRewindEntryNum))
                    {
                    	while(TimeRewindEntry.u64Offset >= pCycMgr->u64MaxCycSize)
                    	{
                    		TimeRewindEntry.u64Offset -= pCycMgr->u64MaxCycSize;
                    		Pvr_Write_Index_Func(TimeRewindUpdateEntrySize, 
                    						  sizeof(PVR_INDEX_ENTRY_S),
                    						  &TimeRewindEntry,
                                			  handle->s32WriteFd,
                                			  TimeRewindEntryNum*sizeof(PVR_INDEX_ENTRY_S), 
                                			  handle);//PVR_WRITE_INDEX
                    		/*if (TimeRewindUpdateEntrySize != sizeof(PVR_INDEX_ENTRY_S))
                    		{
                    			MT_ERR_PVR("Update index:%d u64Offset fail! write size=%d!\n", TimeRewindEntryNum, TimeRewindUpdateEntrySize);
                    		}*/
                    		
                    		if (MT_SUCCESS != PVRIndexGetEntryByNum(handle, &TimeRewindEntry, --TimeRewindEntryNum))
                    		{
                    			MT_ERR_PVR("Get the end index entry fail when update index u64Offset entryNum=%d!\n", TimeRewindEntryNum);
                    		}
                    	}
                    }
                    else
                    {
                    	MT_ERR_PVR("Get the end index entry fail when update index u64Offset entryNum=%d!\n", TimeRewindEntryNum);
                    }

                    bRewindFlg = MT_TRUE;
                    handle->bTimeRewindFlg = MT_FALSE;
                    handle->bTimeRewindTsPauseFlg = MT_FALSE;
					handle->bTimeRewinded = MT_TRUE;
					handle->u64TimeRewindMaxSize += pCycMgr->u64MaxCycSize;
                    
                    indexEntry.u64Offset = (MT_U64)pstDmxIndexInfo->u64GlobalOffset - handle->u64TimeRewindMaxSize;
                    
                    #ifdef PVR_DEBUG_REWIND_TIME
                    MT_PVR_SysGetTimeStampMs(&time_temp);
                    rewind_time = (time_temp - pre_rewind_time); 
                    printf("-----update index use time=%d rewind use time=%d (%d-%d)\n",(time_temp-time_start),rewind_time,time_temp,pre_rewind_time);
                    pre_rewind_time = 0;
                    #endif
                    MT_INFO_PVR("time rewind index pCycMgr->u64MaxCycSize =%#llx indexEntry.u64Offset=%#llx \n",
                                 pCycMgr->u64MaxCycSize,indexEntry.u64Offset);  
                }
            }
        }
        else /*fix file size rewind*/
        {
            indexEntry.u64Offset = pstDmxIndexInfo->u64GlobalOffset - (pCycMgr->u64MaxCycSize * (MT_U64)pCycMgr->s32CycTimes);

            if (indexEntry.u64Offset > pCycMgr->u64MaxCycSize)
            {
                indexEntry.u64Offset = indexEntry.u64Offset - pCycMgr->u64MaxCycSize;
                bRewindFlg = MT_TRUE;
                MT_INFO_PVR("file size rewind index pCycMgr->u64MaxCycSize =%#llx indexEntry.u64Offset=%#llx \n",
                             pCycMgr->u64MaxCycSize,indexEntry.u64Offset);
            }
        }
        /*
        MT_INFO_PVR("before save index: type=%d, offset=0x%llx/0x%llx, PTS=%u, write=%d\n",
                        indexEntry.u16IndexType,
                        indexEntry.u64Offset,
                        indexEntry.u64GlobalOffset,
                        indexEntry.u32PtsMs,
                        handle->u32WriteFrame);
        */
        //printf("+++pass , current frame\n");
        memcpy(&handle->stCurRecFrame, &indexEntry, sizeof(PVR_INDEX_ENTRY_S));

        /* if saved stream less than 50M use direct write */
        if(0==pCycMgr->u32LastFrame)
        {//must get pos==0
            u32DirectFlag = 1;
        }

        PVR_Index_RecIdxInfo(handle, &indexEntry);
        if(bRewindFlg){ //if rewind. rewrite zero-position
            handle->u32WriteFrame=0;
        }
        Ret = PVRCacheWriteIdx(u32ChnID, InstIdx,(MT_U8*)&indexEntry,byte2Save,handle->u32WriteFrame * sizeof(PVR_INDEX_ENTRY_S),
                                   u32DirectFlag,&u32WriteFlag);
        if (Ret != MT_SUCCESS)
        {
            PVR_INDEX_UNLOCK(handle);
            return Ret;
        }

        handle->u32WriteFrame++;

        /* find the cycle of ts, so clear the pointer to zero */
        if(bRewindFlg)
        {
            pCycMgr->s32CycTimes++;
            #ifdef PVR_DEBUG_REWIND_TIME
            printf("%s rewind enter s32CycTimes=%d w=%d r=%d s=%d e=%d l=%d\n",__FUNCTION__,pCycMgr->s32CycTimes,handle->u32WriteFrame,handle->u32ReadFrame,
                pCycMgr->u32StartFrame,pCycMgr->u32EndFrame,pCycMgr->u32LastFrame);
            #endif
            handle->u32WriteFrame = 1;
            if(0 < pCycMgr->u32EndFrame){
                pCycMgr->u32EndFrame--;
            }
            pCycMgr->u32LastFrame = pCycMgr->u32EndFrame;
            #if 0
            if (MT_SUCCESS == PVR_Index_GetFBwardIPBFrameNum(handle, 1, PVR_INDEX_FRAME_I, 
																 PVRIndexIsFrameValid(handle, pCycMgr->u32StartFrame)?pCycMgr->u32StartFrame:0, &u32RewindStartFrm))
            {
                pCycMgr->u32StartFrame = u32RewindStartFrm;/* next I frame number, from 0 */
            }
            else
            {
                MT_ERR_PVR("Get next I frame number fail, use default rewind GAP 60\n");
                pCycMgr->u32StartFrame = PVR_DFT_GOP_LEN;
            }
            #else
            if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == handle->enIndexType){
                pCycMgr->u32StartFrame = 25;
            }else{
                #ifdef PVR_REWIND_ADJUST_START_FRAME
                pCycMgr->u32StartFrame = PVR_REWIND_FRAMES_GAP; /* fixed Bug #14969 #14970  adjust in PVR_Index_Adjust_StartFrame */
                g_pvr_rewind_adjust_count = 0;/* reset check time */
                #else
                pCycMgr->u32StartFrame = PVR_DFT_GOP_LEN;
                #endif
            }
            #endif
            pCycMgr->u32EndFrame = 1;
            bRewindFlg = MT_FALSE;  
            #ifdef PVR_DEBUG_REWIND_TIME
            printf("%s rewind after w=%d r=%d s=%d e=%d l=%d\n",__FUNCTION__,handle->u32WriteFrame,handle->u32ReadFrame,
                pCycMgr->u32StartFrame,pCycMgr->u32EndFrame,pCycMgr->u32LastFrame);
            #endif
            PVR_Index_UpdateIdxInfoWhenRewind(handle);
            if(handle->u32ReadFrame >= pCycMgr->u32LastFrame){
                //printf("++abnormal idx %x,%x\n",handle->u32ReadFrame,pCycMgr->u32LastFrame);
                handle->u32ReadFrame=0;
            }
        }
        else /* normally move */
        {
            pCycMgr->u32EndFrame++;

            /* not rewind, but index maybe reach to the start, tune the end frame postion */
            if (pCycMgr->u32LastFrame < pCycMgr->u32EndFrame)
            {
                pCycMgr->u32LastFrame++;
            }
            else /* not increase Last, need to move start */
            {
                pCycMgr->u32StartFrame++;
#if 0      
				/*if the latest frame size beyond START frame offset, then jump START frame to next I frame */
                Ret = PVRIndexGetEntryByNum(handle,&startEntry,pCycMgr->u32StartFrame);
                if (MT_SUCCESS == Ret)
                {
                    if ((indexEntry.u64Offset + (MT_U64)indexEntry.u32FrameSize) > startEntry.u64Offset)
                    {
                        if (MT_SUCCESS == PVR_Index_GetFBwardIPBFrameNum(handle, 1, PVR_INDEX_FRAME_I, pCycMgr->u32StartFrame, &u32RewindStartFrm))
                        {
                            /* next I frame in the start of the TS file */
                            if (u32RewindStartFrm < pCycMgr->u32EndFrame)
                            {
                                pCycMgr->u32StartFrame = 0;
                            }
                            else
                            {
                                pCycMgr->u32StartFrame = u32RewindStartFrm;/* next I frame number, from 0 */
                            }
                        }
                        else
                        {
                            MT_ERR_PVR("Get next I frame number fail, plus start frame number only\n");
                        }
                    }
                }
                else
                {
                    MT_ERR_PVR("Get start frame entry fail, plus start frame number only\n");
                }
#endif
            }

            if (pCycMgr->u32StartFrame >  pCycMgr->u32LastFrame)
            {
                pCycMgr->u32StartFrame = 0;
            }
        }
    }
    else
    {
        if(PVR_INDEX_REWIND_BY_TIME == pCycMgr->enRewindType)
        {
            //u64CurCycTimeMs = (MT_U64)(handle->u32DmxClkTimeMs - handle->u32RecFirstFrmTimeMs);
            
            if(((pCycMgr->u64AllRecNowInMs-pCycMgr->u64AllRecStartInMs)/1000) >= (pCycMgr->u64MaxCycTimeInMs - 1)/1000)
            {
                pCycMgr->u64MaxCycSize = (MT_U64)pstDmxIndexInfo->u64GlobalOffset / PVR_FIFO_WRITE_BLOCK_SIZE * PVR_FIFO_WRITE_BLOCK_SIZE;
                MT_INFO_PVR("reach fix size: u64MaxCycSize =%llu \n\n",pCycMgr->u64MaxCycSize);
            }
        }
        
        indexEntry.u64Offset = (MT_U64)pstDmxIndexInfo->u64GlobalOffset;
        /*
        MT_INFO_PVR("no rewind before save index: type=%d, offset=0x%llx/0x%llx, PTS=%u, write=%d\n",
                        indexEntry.u16IndexType,
                        indexEntry.u64Offset,
                        indexEntry.u64GlobalOffset,
                        indexEntry.u32PtsMs,
                        handle->u32WriteFrame);
        */
        memcpy(&handle->stCurRecFrame, &indexEntry, sizeof(PVR_INDEX_ENTRY_S));

        /* if saved stream less than 50M use direct write */
        if(0==pCycMgr->u32LastFrame)
        {//must get pos==0
            u32DirectFlag = 1;
        }
        
        PVR_Index_RecIdxInfo(handle, &indexEntry);
        Ret = PVRCacheWriteIdx(u32ChnID, InstIdx,(MT_U8*)&indexEntry,byte2Save,handle->u32WriteFrame * sizeof(PVR_INDEX_ENTRY_S),u32DirectFlag,&u32WriteFlag);
        if (Ret != MT_SUCCESS)
        {
            PVR_INDEX_UNLOCK(handle);
            return Ret;
        }

        handle->u32WriteFrame++;
        pCycMgr->u32EndFrame++;
        pCycMgr->u32LastFrame++;
        if(1)
        {
            MT_U32 startFrame = 0;
            MT_U32 startTsFileNode = 0;
            
            PVR_UpdateSingleEventLoopStartFrame(u32ChnID, &startFrame, &startTsFileNode);
            if(startFrame  > 0 )     pCycMgr->u32StartFrame = startFrame;
            if(startTsFileNode > 0)    pCycMgr->u32Reserve = startTsFileNode;
            MT_INFO_PVR("S:%u, E:%u, L:%u, W:%u\n",
                    pCycMgr->u32StartFrame,
                    pCycMgr->u32EndFrame,
                    pCycMgr->u32LastFrame,
                    handle->u32WriteFrame
                    );
                    
       } 
    }

#ifdef PVR_REWIND_ADJUST_START_FRAME
    PVR_Index_Adjust_StartFrame(handle,pCycMgr,&indexEntry);
#endif

	//PVR_FSYNC(handle->s32WriteFd); /* now we must sync index file to fit index head. */
    if (u32DirectFlag || u32WriteFlag)
	{
	    PVR_CYC_HEADER_INFO_S  destCycInfo;
	    PVRIndexRecordCycInfo(handle, &destCycInfo,0);
	    //PVR_EVENT_D("destCycInfo.endFrame=0x%x,  0x%x\n",destCycInfo.u32EndFrame, destCycInfo.u32LastFrame);
	    //MT_PVR_Update_Index_Event_Rec(u32ChnID, (MT_U8*)&indexEntry, byte2Save);
	}
    handle->Latest_DisplayTimeMs=indexEntry.u32DisplayTimeMs;
    MT_INFO_PVR("S:%u, E:%u, L:%u, W:%u PTS=0x%x Time=%d\n",
                    pCycMgr->u32StartFrame,
                    pCycMgr->u32EndFrame,
                    pCycMgr->u32LastFrame,
                    handle->u32WriteFrame,
                    indexEntry.u32PtsMs,
                    indexEntry.u32DisplayTimeMs);

    MT_ASSERT(handle->u32WriteFrame >= pCycMgr->u32EndFrame);
    //MT_ASSERT(handle->stCurRecFrame.u32FrameSize == (indexEntry.u64SeqHeadOffset - handle->stCurRecFrame.u64SeqHeadOffset));

    /*
    MT_INFO_PVR("save index(%d): type=%d, offset=0x%llx/0x%llx, PTS=%u, Time=%u\n",
                        InstIdx,
                        indexEntry.u16IndexType,
                        indexEntry.u64Offset,
                        indexEntry.u64GlobalOffset,
                        indexEntry.u32PtsMs,
                        indexEntry.u32DisplayTimeMs);
    */
    PVR_INDEX_UNLOCK(handle);

    return 0;
}


MT_S32 PVR_Index_Init(MT_VOID)
{
    MT_U32 i;

    if (0 == g_u32PvrIndexInit)
    {
        for (i = 0; i < PVR_INDEX_NUM; i++)
        {
            memset(&g_stPVRIndex[i], 0, sizeof(PVR_INDEX_S));
        }
        g_u32PvrIndexInit++;
    }

    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : PVR_Index_CreatPlay
 Description     : init index module, get index handle
 Input           : pfileName  **
                    enIndexType: need this just only on recording
 Output          : None
 Return Value    : MT_NULL_PTR:failure, maybe operate file fail, included the wrong index file format
                   handle: success, retruen the handle of index
 Global Variable
    Read Only    :
    Read & Write : pIsNoIdx :index file exist or not
  History
  1.Date         : 2008/4/16
    Author       : q46153
    Modification : Created function
  2.Date         : 2010/06/19
    Author       : Jiang Lei
    Modification : modify for HD
*****************************************************************************/
PVR_INDEX_HANDLE PVR_Index_CreatPlay(MT_U32 chnID,
                                const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr,
                                MT_BOOL *pIsNoIdx,
                                MT_U32    *tsFileFirstNode,
                                MT_BOOL bSupportTimeShiftEvent,
                                MT_BOOL bIsTimeShiftEventFile)
{
    MT_S32 ret = MT_SUCCESS;
    PVR_INDEX_HANDLE handle;
    MT_CHAR szIndexName[PVR_MAX_FILENAME_LEN + 4];
    //MT_BOOL bPlayOnly = MT_FALSE;
    //MT_U32 PicParser;
    PVR_IDX_HEADER_INFO_S headrinfo={0};

    if(!pstPlayAttr)
    {
        return MT_NULL_PTR;
    }
    PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);

    //snprintf(szIndexName,sizeof(szIndexName), "%s.%s", pstPlayAttr->szFileName, "idx");
    PVR_Index_GetIdxFileName(szIndexName,  (MT_CHAR*)pstPlayAttr->szFileName);    
	/* on recording file, play it, and return the recorded index handle */
    if (PVRIndexIsFileRecording(szIndexName, &handle))
    {
        handle->bIsPlay = MT_TRUE;
        if(bSupportTimeShiftEvent && bIsTimeShiftEventFile)
        {
            strcat(szIndexName, "_event0");
            PVR_EVENT_D("%s==================================%d\n",szIndexName,__LINE__);
            if(!PVR_CHECK_FILE_EXIST(szIndexName))   return handle;
        }
        else
        {
            return handle;
         }   
    }
    PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);

    handle = &g_stPVRIndex[chnID];
    MT_ASSERT(MT_FALSE == handle->bIsPlay);

    memset(&handle->stIdxReadCache,0,sizeof(HIPVR_IDX_BUF_S));

    handle->stIdxReadCache.pu8Addr = MT_MALLOC(MT_ID_PVR, PVR_DFT_IDX_READCACHE_SIZE);
    if (handle->stIdxReadCache.pu8Addr)
    {
        handle->stIdxReadCache.u32BufferLen = PVR_DFT_IDX_READCACHE_SIZE;
        handle->stIdxReadCache.u32UsedSize = 0;
        handle->stIdxReadCache.u32StartOffset = 0;
    }
    else
    {
        MT_ERR_PVR("MT_MALLOC read cache buffer failed!\n");
        handle->stIdxReadCache.u32BufferLen = 0;
    }
    if(-1 == pthread_mutex_init(&(handle->stIdxReadCache.stCacheMutex), NULL))
    {
        MT_ERR_PVR("init mutex lock for PVR index failed,check it\n");
        return MT_NULL_PTR;
    }

    
    PVRIndexSetDftAttr(handle);

    memset(handle->szIdxFileName, 0, sizeof(handle->szIdxFileName));
    strncpy(handle->szIdxFileName, szIndexName, strlen(szIndexName));

    if(-1 == pthread_mutex_init(&(handle->stMutex), NULL))
    {
        MT_ERR_PVR("init mutex lock for PVR index failed \n");
        goto ErrorExit;
    }
    PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);


    handle->bIsPlay = MT_TRUE;
    handle->bIsRec = MT_FALSE;
    handle->UpdatingEvent_CheckTimes = 0;
    PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);

    /* check whether index file exist or not, if not, just  track it and not to open */
    if (PVR_CHECK_FILE_EXIST(szIndexName))
    {
        *pIsNoIdx = MT_FALSE;
        handle->s32ReadFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
        if (PVR_FILE_INVALID_FILE == handle->s32ReadFd)
        {
            MT_ERR_PVR("PVR open Index File for read failed !\n");
            goto ErrorExit;
        }
        handle->s32Readdirect_Fd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
        if (PVR_FILE_INVALID_FILE == handle->s32Readdirect_Fd)
        {
            MT_ERR_PVR("PVR open Index File for read failed !\n");
            PVR_CLOSE(handle->s32ReadFd);
            goto ErrorExit;
        }

        handle->s32SeekFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
        if (PVR_FILE_INVALID_FILE == handle->s32SeekFd)
        {
            MT_ERR_PVR("PVR open Index File for seek failed !\n");
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            goto ErrorExit;
        }

        handle->s32HeaderFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_BOTH);
        if (PVR_FILE_INVALID_FILE == handle->s32HeaderFd)
        {
            MT_ERR_PVR("PVR open Index File for Header failed !\n");
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            PVR_CLOSE(handle->s32SeekFd);
            goto ErrorExit;
        }

        ret = PVRIndexReadHeaderInfo(handle,&headrinfo);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR read Index File Header failed !\n");
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            PVR_CLOSE(handle->s32SeekFd);
            PVR_CLOSE(handle->s32HeaderFd);
            goto ErrorExit;
        }
        PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);

        ret = PVR_Index_PlayGetFileAttrByFileName(pstPlayAttr->szFileName, MT_NULL, &handle->stIndexFileAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR read Index File Header failed !\n");
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            PVR_CLOSE(handle->s32SeekFd);
            PVR_CLOSE(handle->s32HeaderFd);
            goto ErrorExit;
        }
        handle->Latest_DisplayTimeMs =  handle->stIndexFileAttr.u32EndTimeInMs;
        handle->enIndexType = handle->stIndexFileAttr.enIdxType;

        /*alone play, not open write handle */
        handle->s32WriteFd = PVR_FILE_INVALID_FILE;
    }
    else  /* NO index file, no need to process! */
    {
        //*pIsNoIdx = MT_TRUE;
        //handle->enIndexType = MT_UNF_PVR_REC_INDEX_TYPE_NONE;
        MT_ERR_PVR("No index file for '%s' found!.\n", pstPlayAttr->szFileName);
        goto ErrorExit;
    }
    PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);
    {
        PVR_APPEND_REC recinfo={0};
        MT_S32 appendrec_off = (sizeof(PVR_IDX_HEADER_INFO_S) + headrinfo.u32CADataInfoLen + headrinfo.u32UsrDataInfoLen);
        pvr_pread(handle->s32HeaderFd,&recinfo, sizeof(PVR_APPEND_REC),appendrec_off);
        //handle->appendrec=recinfo;
        //printf("++p.%x,%x\n",recinfo.u32TotalTime,recinfo.u32LastIndex);
    }
    //PVR_EVENT_D("%s==================================%d\n",__FILE__,__LINE__);
   if(bSupportTimeShiftEvent && bIsTimeShiftEventFile)
   {
	    if(headrinfo.shareEvent[0].u64ShareFileOffset > 0)
	    {
	        PVR_EVENT_D("headrinfo.u64ShareFileOffset ===== %d, headrinfo.shareFile=%s\n",headrinfo.shareEvent[0].u64ShareFileOffset,headrinfo.shareEvent[0].shareFileName);
	    }
   	 *tsFileFirstNode = headrinfo.stCycInfo.u32TimeShiftEventLoopFirstNode;
    }
    else
    {
	*tsFileFirstNode = 0;
    }
    PVR_EVENT_D("%s=================================*tsFileFirstNode=%d\n",__FILE__,*tsFileFirstNode);
    handle->u32StartFrame_dread=0xffffffff;
    handle->u32EndFrame_dread=0xffffffff;
    handle->u32LastFrame_dread=0xffffffff;
    handle->u32magic1=0xaa55aa55;
    handle->u32magic2=0xaa55aa55;
    handle->u32magic3=0xaa55aa55;
    handle->line_magc1=0;
    handle->line_magc2=0;
    handle->line_magc3=0;
    
    return handle;

ErrorExit:
    if (handle->stIdxReadCache.pu8Addr)
    {
        MT_FREE(MT_ID_PVR, handle->stIdxReadCache.pu8Addr);
    }
    memset(&handle->stIdxReadCache,0,sizeof(HIPVR_IDX_BUF_S));
    handle->bIsPlay = MT_FALSE;
    (MT_VOID)pthread_mutex_destroy(&(handle->stMutex));
    return MT_NULL_PTR;
}

/*****************************************************************************
 Prototype       : PVR_Index_CreatRec
 Description     : create pvr rec channel
 Input           : pfileName  **
                    enIndexType: need this just only on recording
 Output          : None
 Return Value    : MT_NULL_PTR: failure, maybe operate file failure or the file playing, which the need to record.
                   handle: success, retruen the handle of index
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/16
    Author       : q46153
    Modification : Created function
  2.Date         : 2010/06/19
    Author       : Jiang Lei
    Modification : modify for HD
*****************************************************************************/
PVR_INDEX_HANDLE PVR_Index_CreatRec(MT_U32 chnID,
                                MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
    PVR_INDEX_HANDLE handle = MT_NULL;
    MT_CHAR szIndexName[PVR_MAX_FILENAME_LEN + 4] = {0};
    MT_U32 i = 0;
    MT_S32 flag_idx_exist=0;

    if(!pstRecAttr)
    {
        return MT_NULL_PTR;
    }

    PVR_Index_GetIdxFileName(szIndexName,  pstRecAttr->szFileName);
    //snprintf(szIndexName,sizeof(szIndexName), "%s.ts.%s", pstRecAttr->szFileName, "idx");

    /** if the file has been playing, return MT_NULL_PTR */
    if (PVRIndexIsFilePlaying(szIndexName, &handle))
    {
        MT_ERR_PVR("the file %s is playing, please stop it before recordring the same file.\n", pstRecAttr->szFileName);
        return MT_NULL_PTR;
    }

    for(i = 0; i < sizeof(g_u32RecChnStat)/sizeof(MT_U32); i++)
    {
        if (PVR_INDEX_REC_CHN_UNUSED == g_u32RecChnStat[i])
        {
            g_u32RecChnStat[i] = PVR_INDEX_REC_CHN_USED;
            break;
        }
    }
    if (i >= PVR_REC_MAX_CHN_NUM)
    {
        MT_ERR_PVR("Rec channel %d is out of range.\n", i);
        return MT_NULL_PTR;
    }

    handle = &g_stPVRIndex[i + PVR_PLAY_MAX_CHN_NUM];

    MT_ASSERT(MT_FALSE == handle->bIsRec);

    PVRIndexSetDftAttr(handle);

    memset(handle->szIdxFileName, 0, sizeof(handle->szIdxFileName));
    strncpy(handle->szIdxFileName, szIndexName, strlen(szIndexName));

    if(-1 == pthread_mutex_init(&(handle->stMutex), NULL))
    {
        MT_ERR_PVR("init mutex lock for PVR index failed \n");
        g_u32RecChnStat[i] = PVR_INDEX_REC_CHN_UNUSED;
        return MT_NULL_PTR;
    }

    PVR_IDX_CHECK_CYC_SIZE(pstRecAttr);

    handle->bIsPlay = MT_FALSE;
    handle->bIsRec = MT_TRUE;
    handle->UpdatingEvent_CheckTimes = 0;//fixed Bug #26280 ff till end can't resume issue
    handle->u32RecPicParser = i;
    handle->enIndexType = pstRecAttr->enIndexType;
    handle->u32DavBufSize = pstRecAttr->u32DavBufSize;
    handle->u32IdxBufSize = pstRecAttr->u32IdxBufSize;
    handle->stCycMgr.bIsRewind = pstRecAttr->bRewind;
    handle->stCycMgr.u64MaxCycSize = (pstRecAttr->u64MaxFileSize / PVR_FIFO_WRITE_BLOCK_SIZE) * PVR_FIFO_WRITE_BLOCK_SIZE;
    handle->stCycMgr.u64MaxCycTimeInMs = pstRecAttr->u64MaxTimeInMs ;
    handle->stCycMgr.u64AllRecStartInMs =0 ;
    handle->stCycMgr.u64AllRecNowInMs =0 ;
    
    handle->stCycMgr.enRewindType = PVR_INDEX_REWIND_BY_BOTH;
    
    if ((handle->stCycMgr.u64MaxCycSize == 0) &&  (handle->stCycMgr.u64MaxCycTimeInMs > 0))
    {
        handle->stCycMgr.enRewindType = PVR_INDEX_REWIND_BY_TIME;
    }
    if ((handle->stCycMgr.u64MaxCycSize > 0) &&  (handle->stCycMgr.u64MaxCycTimeInMs == 0))
    {
        handle->stCycMgr.enRewindType = PVR_INDEX_REWIND_BY_SIZE;
    }

    handle->u32FflushTime = 0;
    if(pstRecAttr->bEnable_CTrec){
        //printf("+++idx. %s\n",szIndexName);
    	  if (PVR_CHECK_FILE_EXIST(szIndexName)){
            flag_idx_exist=1;
        }
    }
    handle->s32WriteFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_WRITE);
    if (PVR_FILE_INVALID_FILE == handle->s32WriteFd)
    {
        MT_ERR_PVR("PVR open Index File for write failed !\n");
        goto ErrorExit;
    }

    handle->s32SeekFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
    if (PVR_FILE_INVALID_FILE == handle->s32SeekFd)
    {
        MT_ERR_PVR("PVR open Index File for seek failed !\n");
        PVR_CLOSE(handle->s32WriteFd);
        goto ErrorExit;
    }

    handle->s32HeaderFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_BOTH);
    if (PVR_FILE_INVALID_FILE == handle->s32HeaderFd)
    {
        MT_ERR_PVR("PVR open Index File for Idx Header failed !\n");
        PVR_CLOSE(handle->s32WriteFd);
        PVR_CLOSE(handle->s32SeekFd);
        goto ErrorExit;
    }

    handle->s32ReadFd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
    if (PVR_FILE_INVALID_FILE == handle->s32ReadFd)
    {
        PVR_CLOSE(handle->s32WriteFd);
        PVR_CLOSE(handle->s32SeekFd);
        PVR_CLOSE(handle->s32HeaderFd);
        (MT_VOID)remove(szIndexName);
        MT_ERR_PVR("PVR open Index File for read failed !\n");
        goto ErrorExit;
    }
    handle->s32Readdirect_Fd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
    if (PVR_FILE_INVALID_FILE == handle->s32Readdirect_Fd)
    {
        PVR_CLOSE(handle->s32WriteFd);
        PVR_CLOSE(handle->s32SeekFd);
        PVR_CLOSE(handle->s32HeaderFd);
        PVR_CLOSE(handle->s32ReadFd);
        (MT_VOID)remove(szIndexName);
        MT_ERR_PVR("PVR open Index File for read failed !\n");
        goto ErrorExit;
    }

    memset(&handle->stIdxWriteCache,0,sizeof(HIPVR_IDX_BUF_S));
    handle->stIdxWriteCache.pu8Addr = MT_MALLOC(MT_ID_PVR, PVR_DFT_IDX_WRITECACHE_SIZE);
    if (handle->stIdxWriteCache.pu8Addr)
    {
        handle->stIdxWriteCache.u32BufferLen = PVR_DFT_IDX_WRITECACHE_SIZE;
        handle->stIdxWriteCache.u32UsedSize = 0;
        handle->stIdxWriteCache.u32StartOffset = 0;
    }
    else
    {
        handle->stIdxWriteCache.u32BufferLen = 0;
        MT_ERR_PVR("MT_MALLOC write cache buffer failed!\n");
    }
    if(-1 == pthread_mutex_init(&(handle->stIdxWriteCache.stCacheMutex), NULL))
    {
        MT_ERR_PVR("init mutex lock for PVR index failed,check it\n");
        return MT_NULL_PTR;
    } 

    memset(&handle->stIdxReadCache,0,sizeof(HIPVR_IDX_BUF_S));
    handle->stIdxReadCache.pu8Addr = MT_MALLOC(MT_ID_PVR, PVR_DFT_IDX_READCACHE_SIZE);
    if (handle->stIdxReadCache.pu8Addr)
    {
        handle->stIdxReadCache.u32BufferLen = PVR_DFT_IDX_READCACHE_SIZE;
        handle->stIdxReadCache.u32UsedSize = 0;
        handle->stIdxReadCache.u32StartOffset = 0;
    }
    else
    {
        MT_ERR_PVR("MT_MALLOC read cache buffer failed!\n");
        handle->stIdxReadCache.u32BufferLen = 0;
    }        
    if(-1 == pthread_mutex_init(&(handle->stIdxReadCache.stCacheMutex), NULL))
    {
        MT_ERR_PVR("init mutex lock for PVR index failed,check it\n");
        return MT_NULL_PTR;
    }
        
    handle->u32FRollTime = 0;
    handle->u32DeltaDispTimeMs = 0;
    handle->u32LastDispTime = 0;
    handle->u32TimeShiftTillEndTimeMs = 0;
    handle->u32TimeShiftTillEndCnt = 0;
	  handle->bTimeRewindFlg = MT_FALSE;
    handle->bTimeRewindTsPauseFlg = MT_FALSE;
	  handle->u64TimeRewindMaxSize = 0;
    memset(&(handle->stRecIdxInfo), 0, sizeof(PVR_REC_INDEX_INFO_S));

    UNUSED(chnID);
    if(flag_idx_exist){
        int ret=0;
        PVR_IDX_HEADER_INFO_S headrinfo={0};
        //printf("+++will load idx file\n");
        ret = PVRIndexReadHeaderInfo(handle,&headrinfo);
        //printf("+++over load idx file\n");
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR read Index File Header failed !\n");
            PVR_CLOSE(handle->s32WriteFd);
            PVR_CLOSE(handle->s32SeekFd);
            PVR_CLOSE(handle->s32HeaderFd);
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            goto ErrorExit;
        }
        //printf("+++will load idx attr\n");
        ret = PVR_Index_PlayGetFileAttrByFileName(szIndexName, MT_NULL, &handle->stIndexFileAttr);
        //printf("+++over load idx attr\n");
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR read Index File Header failed !\n");
            PVR_CLOSE(handle->s32WriteFd);
            PVR_CLOSE(handle->s32SeekFd);
            PVR_CLOSE(handle->s32HeaderFd);
            PVR_CLOSE(handle->s32ReadFd);
            PVR_CLOSE(handle->s32Readdirect_Fd);
            goto ErrorExit;
        }
        {
            PVR_INDEX_ENTRY_S e={0};
            PVRIndexGetEntryByNum(handle, &e, handle->stCycMgr.u32EndFrame);
            //printf("+++idx.e=%x,%x\n",handle->stCycMgr.u32EndFrame,e.u32DisplayTimeMs);
            handle->Last_DisplayTimeMs=e.u32DisplayTimeMs;
        }
        handle->enIndexType = handle->stIndexFileAttr.enIdxType;
        handle->stCycMgr.u32EndFrame++;
        handle->stCycMgr.u32LastFrame++;
        handle->u32WriteFrame=handle->stCycMgr.u32EndFrame;
    }else{
        handle->Last_DisplayTimeMs=0;
    }
    handle->u32StartFrame_dread=0xffffffff;
    handle->u32EndFrame_dread=0xffffffff;
    handle->u32LastFrame_dread=0xffffffff;
    handle->u32magic1=0xaa55aa55;
    handle->u32magic2=0xaa55aa55;
    handle->u32magic3=0xaa55aa55;
    handle->line_magc1=0;
    handle->line_magc2=0;
    handle->line_magc3=0;
    //printf("+++create.s=%x,e=%x,w=%x\n",handle->stCycMgr.u32EndFrame,handle->stCycMgr.u32LastFrame,handle->u32WriteFrame);
    return handle;

ErrorExit:
    g_u32RecChnStat[i] = PVR_INDEX_REC_CHN_UNUSED;
    (MT_VOID)pthread_mutex_destroy(&(handle->stMutex));
    return MT_NULL_PTR;
}


/*****************************************************************************
 Prototype       : PVR_Index_Destroy
 Description     : de-init index module, release index handle and relative resource
                 may stop either record or play at any time, which are independent
 Input           : handle  **
                   u32PlayOrRec  need to stop record or play
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/16
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_Destroy(PVR_INDEX_HANDLE handle, MT_U32 u32PlayOrRec)
{
    PVR_CHECK_POINTER(handle);
    PVR_CYC_HEADER_INFO_S  destCycInfo;
    MT_CHAR  szFileName[PVR_MAX_FILENAME_LEN] = {0};

    if (PVR_INDEX_PLAY == u32PlayOrRec)
    {
        handle->bIsPlay = MT_FALSE;
    }
    else
    {
        handle->bIsRec = MT_FALSE;
    }

    if (handle->bIsPlay || handle->bIsRec)
    {
        MT_WARN_PVR("also play or rec was using this index.\n");
        return MT_SUCCESS;
    }
    
    if (PVR_Index_FlushIdxWriteCache(handle) != MT_SUCCESS)
    {
        MT_ERR_PVR("rec flush cache error!\n");
    }

    memcpy(szFileName, handle->szIdxFileName, 
           ((ulong)strstr(handle->szIdxFileName,".idx") - (ulong)handle->szIdxFileName));    
    
    if(NULL != PVRRecGetChnAttrByName(szFileName))
    {
        PVRIndexRecordCycInfo(handle, &destCycInfo,1);
    }

    (MT_VOID)pthread_mutex_destroy(&(handle->stMutex));

    /* close index file                                                        */
    if (handle->s32ReadFd && (handle->s32ReadFd != PVR_FILE_INVALID_FILE))
    {
        PVR_CLOSE(handle->s32ReadFd);
        handle->s32ReadFd = PVR_FILE_INVALID_FILE;
    }
    if (handle->s32Readdirect_Fd && (handle->s32Readdirect_Fd != PVR_FILE_INVALID_FILE))
    {
        PVR_CLOSE(handle->s32Readdirect_Fd);
        handle->s32Readdirect_Fd = PVR_FILE_INVALID_FILE;
    }

    if (handle->s32WriteFd && (handle->s32WriteFd != PVR_FILE_INVALID_FILE))
    {
        PVR_CLOSE(handle->s32WriteFd);
        handle->s32WriteFd = PVR_FILE_INVALID_FILE;
    }

    if (handle->s32SeekFd && (handle->s32SeekFd != PVR_FILE_INVALID_FILE))
    {
        PVR_CLOSE(handle->s32SeekFd);
        handle->s32SeekFd = PVR_FILE_INVALID_FILE;
    }

    if (handle->s32HeaderFd && (handle->s32HeaderFd != PVR_FILE_INVALID_FILE))
    {
        PVR_CLOSE(handle->s32HeaderFd);
        handle->s32HeaderFd = PVR_FILE_INVALID_FILE;
    }

    /* release index handle                                                 */
    if (handle->u32RecPicParser != 0xffffffff)
    {
        g_u32RecChnStat[handle->u32RecPicParser] = PVR_INDEX_REC_CHN_UNUSED;
        handle->u32RecPicParser = 0xffffffff;
    }
    
    if (handle->stIdxWriteCache.pu8Addr)
    {
        MT_FREE(MT_ID_PVR, handle->stIdxWriteCache.pu8Addr);
        handle->stIdxWriteCache.pu8Addr = MT_NULL;
        handle->stIdxWriteCache.u32StartOffset = 0;
        handle->stIdxWriteCache.u32UsedSize = 0;
    }
        (MT_VOID)pthread_mutex_destroy(&(handle->stIdxWriteCache.stCacheMutex));        
        
    if (handle->stIdxReadCache.pu8Addr)
    {
        MT_FREE(MT_ID_PVR, handle->stIdxReadCache.pu8Addr);
        handle->stIdxReadCache.pu8Addr = MT_NULL;
        handle->stIdxReadCache.u32StartOffset = 0;
        handle->stIdxReadCache.u32UsedSize = 0;
    }
    (MT_VOID)pthread_mutex_destroy(&(handle->stIdxReadCache.stCacheMutex));    

    return MT_SUCCESS;
}



/*
  |---n * sizeof(PVR_INDEX_ENTRY_S)---|
  +---------+-------------------------+-------...
  | CycInfo    | UsrDataInfo                          | Idx data
  +---------+-------------------------+---------...
*/
MT_S32 PVR_Index_PrepareHeaderInfo(PVR_INDEX_HANDLE handle, MT_U32 u32UsrDataLen, MT_U32 u32Vtype)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo;
    MT_U32 u32HeadInfoSize = 0;
    MT_S32 s32WriteRet;
    MT_U8* pTmpBuff = NULL;

    memset(&stIdxHeaderInfo, 0x0, sizeof(PVR_IDX_HEADER_INFO_S));

    stIdxHeaderInfo.u32CADataInfoLen = PVR_MAX_CADATA_LEN;

    /* compute HeaderSize, which should be the times of sizeof(PVR_INDEX_ENTRY_S)*/
    u32HeadInfoSize = (sizeof(PVR_IDX_HEADER_INFO_S) + stIdxHeaderInfo.u32CADataInfoLen + u32UsrDataLen + sizeof(PVR_INDEX_ENTRY_S))
                      / sizeof(PVR_INDEX_ENTRY_S) * sizeof(PVR_INDEX_ENTRY_S);
    MT_INFO_PVR("headInfoSize: [%d]\n",u32HeadInfoSize);

    stIdxHeaderInfo.u32HeaderLen = u32HeadInfoSize;
    stIdxHeaderInfo.u32StartCode = PVR_INDEX_HEADER_CODE;
    stIdxHeaderInfo.u32UsrDataInfoLen = u32UsrDataLen;



    stIdxHeaderInfo.u64ValidSize = handle->stCycMgr.u64MaxCycSize;
    stIdxHeaderInfo.stCycInfo.u32StartFrame = handle->stCycMgr.u32StartFrame;
    stIdxHeaderInfo.stCycInfo.u32EndFrame   = handle->stCycMgr.u32EndFrame;
    stIdxHeaderInfo.stCycInfo.u32LastFrame  = handle->stCycMgr.u32LastFrame;
    stIdxHeaderInfo.stCycInfo.u32IsRewind   = handle->stCycMgr.bIsRewind;
    /* use u32Reserved low 16bits to store MT_UNF_VCODEC_TYPE_E */
    stIdxHeaderInfo.u32Reserved = 0xFFFF & (100 + u32Vtype); 
    if ((MT_S32)stIdxHeaderInfo.u32UsrDataInfoLen < 0)
    {
        MT_ERR_PVR("calc usr data len:%d err\n", stIdxHeaderInfo.u32UsrDataInfoLen);
        return MT_FAILURE;
    }

    pTmpBuff = MT_MALLOC(MT_ID_PVR, u32HeadInfoSize);
    if (NULL == pTmpBuff)
    {
        MT_ERR_PVR("no mem, want=%u\n", u32HeadInfoSize);
        return MT_FAILURE;
    }
    memset(pTmpBuff, 0x0, u32HeadInfoSize);
    memcpy(pTmpBuff, &stIdxHeaderInfo, sizeof(PVR_IDX_HEADER_INFO_S));

    s32WriteRet = (MT_S32)u32HeadInfoSize;
    if (s32WriteRet != PVR_WRITE((MT_VOID*)pTmpBuff, (size_t)u32HeadInfoSize, handle->s32HeaderFd, 0))
    {
        MT_ERR_PVR("write header info err, fd:%d, size:%d\n", handle->s32HeaderFd, u32HeadInfoSize);
        MT_FREE(MT_ID_PVR, pTmpBuff);
        return MT_FAILURE;
    }
    MT_INFO_PVR("write header info ok(%uByte writen), UDLen:%u, MaxSize:%llu\n", s32WriteRet, stIdxHeaderInfo.u32UsrDataInfoLen, stIdxHeaderInfo.u64ValidSize);
    {
        PVR_APPEND_REC recinfo={0};
        MT_S32 appendrec_off=(sizeof(PVR_IDX_HEADER_INFO_S) + stIdxHeaderInfo.u32CADataInfoLen + stIdxHeaderInfo.u32UsrDataInfoLen);
        pvr_pwrite(handle->s32HeaderFd,&recinfo, sizeof(PVR_APPEND_REC),appendrec_off);
        //printf("+++rec.%x,%x\n",appendrec_off,recinfo.u32TotalTime,recinfo.u32LastIndex);
    }
    PVR_FSYNC(handle->s32HeaderFd);

    handle->u32IdxStartOffsetLen = u32HeadInfoSize;

    MT_FREE(MT_ID_PVR, pTmpBuff);

    return MT_SUCCESS;
}

MT_S64 PVR_Index_CutEventPart(int srcfd, int dstfd,  PVR_IDX_HEADER_INFO_S *stIdxHeaderInfo, MT_U32 u32HeadInfoSize, MT_U64 startpos,  MT_U64 endpos)
{
    PVR_IDX_HEADER_INFO_S oldHeaderInfo={0,};
    PVR_INDEX_ENTRY_S stIdxtmp;
    PVR_INDEX_ENTRY_S *idxData=NULL;
    MT_U32 u32IndexSize=0;
    MT_U32 i=0,j=0;
    MT_U32 s;   //start
    MT_U32 e;   //end
    MT_U32 m=0,tmpm=0xffffffff; //middle
    MT_U32 findstart=0xffffffff,findend=0xffffffff;

    PVR_EVENT_D("%s====%d\n",__FILE__,__LINE__);
    if(NULL==stIdxHeaderInfo)
    {
        return -1;
    }
    memcpy(&oldHeaderInfo, stIdxHeaderInfo, sizeof(PVR_IDX_HEADER_INFO_S));
    
    u32IndexSize = sizeof(PVR_INDEX_ENTRY_S);
    //u32HeadInfoSize = (sizeof(PVR_IDX_HEADER_INFO_S) + PVR_MAX_CADATA_LEN + u32UsrDataLen + sizeof(PVR_INDEX_ENTRY_S))
    //                  / u32IndexSize * u32IndexSize;
    //stIdxHeaderInfo=(PVR_IDX_HEADER_INFO_S *)malloc(u32HeadInfoSize);
    PVR_EVENT_D("%s  >>>> startpos=%llx\n",__FILE__,startpos);
    PVR_READ(stIdxHeaderInfo,u32HeadInfoSize,srcfd,0);
    PVR_WRITE(stIdxHeaderInfo,u32HeadInfoSize,dstfd,0); //write header-zone
    
    if(5 > stIdxHeaderInfo->stCycInfo.u32EndFrame)
    {
        MT_ERR_PVR("%s=== stIdxHeaderInfo->stCycInfo.u32EndFrame=%d\n",__FILE__,stIdxHeaderInfo->stCycInfo.u32EndFrame);
        return -1;
    }
    /*find start-pos by startoffset*/
    s=0;
    e=stIdxHeaderInfo->stCycInfo.u32EndFrame-1;
    m=((s+e)>>1);      //1/2 pos
    while(1){
        PVR_READ(&stIdxtmp,u32IndexSize,srcfd,u32HeadInfoSize+m*u32IndexSize);
        //printf("idx1.%llx,%llx,%x,%x,%llx\n",stIdxtmp.u64GlobalOffset,stIdxtmp.u64Offset,stIdxtmp.u32FrameSize,stIdxtmp.u32PtsMs,startpos);
        //printf("::.%d,%d,%d\n",s,m,e);
        if(stIdxtmp.u64GlobalOffset<startpos){
            s=m;
            tmpm=((s+e)>>1);
        }else if(stIdxtmp.u64GlobalOffset>startpos){
            e=m;
            tmpm=((s+e)>>1);
        }else{
            findstart=m;
            break;
        }
        if(m==tmpm){
            findstart=e;
            break;
        }
        m=tmpm;
    }
    
    /*find end-pos by endoffset*/
    s=0;
    e=stIdxHeaderInfo->stCycInfo.u32EndFrame-1;
    m=((s+e)>>1);      //1/2 pos
    while(1){
        PVR_READ(&stIdxtmp,u32IndexSize,srcfd,u32HeadInfoSize+m*u32IndexSize);
        //printf("idx2.%llx,%llx,%x,%x,%llx\n",stIdxtmp.u64GlobalOffset,stIdxtmp.u64Offset,stIdxtmp.u32FrameSize,stIdxtmp.u32PtsMs,endpos);
        //printf("::.%d,%d,%d\n",s,m,e);
        if(stIdxtmp.u64GlobalOffset<endpos){
            s=m;
            tmpm=((s+e)>>1);
        }else if(stIdxtmp.u64GlobalOffset>endpos){
            e=m;
            tmpm=((s+e)>>1);
        }else{
            findend=m;
            break;
        }
        if(m==tmpm){
            findend=s;
            break;
        }
        m=tmpm;
    }
    if((0xffffffff==findstart) || (0xffffffff==findend)){
        //free(stIdxHeaderInfo);
        MT_ERR_PVR("%s====%d\n",__FILE__,__LINE__);

        return -1;
    }

    j=0;
#if 1    
    if((findend >  findstart) && (findstart >= 0))
    {
        PVR_INDEX_ENTRY_S *tmp = NULL;
        idxData = (PVR_INDEX_ENTRY_S *)malloc((findend - findstart) * u32IndexSize);
		if (NULL == idxData)
		{
			MT_ERR_PVR("%s====%d, idxData is NULL!\n",__FILE__,__LINE__);
			return -1;
		}
        
		memset(idxData, 0x00, ((findend - findstart) * u32IndexSize));
        tmp= idxData;
        PVR_READ(tmp,((findend - findstart) * u32IndexSize),srcfd,u32HeadInfoSize + findstart*u32IndexSize);
        for(i=findstart; i<findend; i++){
            tmp->u64GlobalOffset-=startpos;
            tmp->u64Offset-=startpos;
            tmp ++;
            j++;
        }
        PVR_WRITE(idxData,((findend - findstart) * u32IndexSize), dstfd, u32HeadInfoSize);
        PVR_FSYNC(dstfd);        

        if(idxData)  free(idxData);
    }
#else
    for(i=findstart; i<findend; i++){
        PVR_READ(&stIdxtmp,u32IndexSize,srcfd,u32HeadInfoSize+i*u32IndexSize);
        stIdxtmp.u64GlobalOffset-=startpos;
        stIdxtmp.u64Offset-=startpos;
        PVR_WRITE(&stIdxtmp,u32IndexSize,dstfd,u32HeadInfoSize+j*u32IndexSize);
        j++;
    }
#endif    
    PVR_EVENT_D("%x,%x, startpos=0x%llx, j = %d,  u64Offset=%llx\n",findstart,findend,startpos, j, stIdxtmp.u64Offset);

    stIdxHeaderInfo->stCycInfo.u32StartFrame=0;
    stIdxHeaderInfo->stCycInfo.u32EndFrame= j-1;
    stIdxHeaderInfo->stCycInfo.u32LastFrame = stIdxHeaderInfo->stCycInfo.u32EndFrame;
    stIdxHeaderInfo->stCycInfo.u32IsRewind=0;
    PVR_EVENT_D("+++save.header:%d,%d,%d,%d\n",
      stIdxHeaderInfo->u32HeaderLen,stIdxHeaderInfo->u32CADataInfoLen,
      stIdxHeaderInfo->u32UsrDataInfoLen,stIdxHeaderInfo->stCycInfo.u32EndFrame);
    //memset(stIdxHeaderInfo->shareEvent[0].shareFileName, 0, PVR_MAX_FILENAME_LEN);
    //if(oldHeaderInfo.shareEvent[0].u64ShareFileOffset > 0)
    //{
        //strcpy(stIdxHeaderInfo->shareEvent[0].shareFileName, oldHeaderInfo.shareEvent[0].shareFileName);
        //stIdxHeaderInfo->shareEvent[0].u64ShareFileOffset = oldHeaderInfo.shareEvent[0].u64ShareFileOffset;
        //stIdxHeaderInfo->shareFlagTsFile = oldHeaderInfo.shareFlagTsFile;
    //}
    stIdxHeaderInfo->shareFlagTsFile = oldHeaderInfo.shareFlagTsFile;
    PVR_WRITE(stIdxHeaderInfo, u32HeadInfoSize, dstfd, 0);   //re-write header-zone
    //free(stIdxHeaderInfo);
    PVR_EVENT_D("stIdxHeaderInfo->shareFileName%s, stIdxHeaderInfo->u64ShareFileOffset=%d\n",
                            stIdxHeaderInfo->shareEvent[0].shareFileName,startpos);
                            
    return stIdxHeaderInfo->stCycInfo.u32EndFrame;
}

/* reset the player attribute, called when start play*/
MT_VOID PVR_Index_ResetPlayAttr(PVR_INDEX_HANDLE handle)
{
    handle->u32ReadFrame  = handle->stCycMgr.u32StartFrame;
    memset(&handle->stCurPlayFrame, 0, sizeof(PVR_INDEX_ENTRY_S));
}

/* reset the player attribute, called when start record */
MT_VOID PVR_Index_ResetRecAttr(PVR_INDEX_HANDLE handle)
{
    handle->u64GlobalOffset = 0;
    handle->u32LastDavBufOffset = 0;
    handle->u32PauseFrame  = 0;
    handle->u64PauseOffset = PVR_INDEX_PAUSE_INVALID_OFFSET;
    handle->u32WriteFrame = 0;
    handle->u16RecLastIframe = PVR_INDEX_INVALID_I_FRAME_OFFSET;
    handle->u32RecLastValidPtsMs = PVR_INDEX_INVALID_PTSMS;
    handle->u16RecUpFlowFlag = 0;
    handle->u32RecFirstFrmTimeMs = 0;
    handle->stCycMgr.u32StartFrame = 0;
    handle->stCycMgr.u32EndFrame = 0;
    handle->stCycMgr.u32LastFrame = 0;
    handle->stCycMgr.s32CycTimes = 0;
    handle->stCycMgr.u32StartFrame = 0;

    memset(&handle->stCurRecFrame, 0, sizeof(PVR_INDEX_ENTRY_S) );
}

/* set current frame size is zero, prevent from repeatly sending the last frame when switch play mode */
MT_S32 PVR_Index_ChangePlayMode(PVR_INDEX_HANDLE handle)
{
    handle->stCurPlayFrame.u32FrameSize = 0;
    return MT_SUCCESS;
}


MT_S32 PVR_Index_GetUsrDataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32BuffSize)
{
    //PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_S32 s32ReadRet;

    s32ReadRet = (MT_S32)PVR_READ(pBuff, (size_t)u32BuffSize, s32Fd, 0);//(off_t)PVR_GET_USR_DATA_OFFSET(stIdxHeaderInfo));
    //printf("read usr data info :0x%x,0x%x\n", s32ReadRet,u32BuffSize);
    if((s32ReadRet == 0)  || (s32ReadRet > (MT_S32)u32BuffSize)){
        return MT_FAILURE;
    }
    return s32ReadRet;
}

MT_S32 PVR_Index_SetUsrDataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32UsrDataLen)
{
    //PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_S32 s32WriteRet = (MT_S32)u32UsrDataLen;

    s32WriteRet = (MT_S32)PVR_WRITE((MT_VOID*)pBuff, (size_t)u32UsrDataLen, s32Fd, 0);//(off_t)PVR_GET_USR_DATA_OFFSET(stIdxHeaderInfo));
    //printf("write usr data info err:0x%x,0x%x\n", s32WriteRet,u32UsrDataLen);
    if(s32WriteRet != (MT_S32)u32UsrDataLen){
        return MT_FAILURE;
    }

    return s32WriteRet;
}

MT_S32 PVR_Index_GetCADataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32BuffSize)
{
//    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_S32 s32ReadRet;
    MT_U32 u32ReadLen;
    /*	
    if (MT_SUCCESS != PVRIndexGetHeaderInfo(s32Fd, &stIdxHeaderInfo))
    {
        MT_ERR_PVR("No CADataInfo in this file.\n");
        return MT_ERR_PVR_FILE_CANT_READ;
    }
    u32ReadLen = (stIdxHeaderInfo.u32CADataInfoLen > u32BuffSize) ? (u32BuffSize) : (stIdxHeaderInfo.u32CADataInfoLen);
    */
    u32ReadLen = u32BuffSize;

    //s32ReadRet = PVR_READ(pBuff, u32ReadLen, s32Fd, PVR_GET_CA_DATA_OFFSET());
    s32ReadRet = PVR_READ(pBuff, u32ReadLen, s32Fd, 0);
    if ((mt_u32)s32ReadRet != u32ReadLen)
    {
        //MT_ERR_PVR("read usr CA info err, read ret:0x%x, g_u32CurrentFrameCaId=%d\n", s32ReadRet, g_u32CurrentFrameCaId);
        //return MT_FAILURE;
    }
    //printf("g_u32CurrentFrameCaId = %d, s32ReadRet=%d\n",g_u32CurrentFrameCaId,s32ReadRet);
    return s32ReadRet;
}


MT_S32 PVR_Index_SetCADataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32CADataLen)
{
    //PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_S32 s32WriteRet = (MT_S32)u32CADataLen;
    //printf("%s=======%d\n",__FUNCTION__,__LINE__);
	
/*	
    if (MT_SUCCESS != PVRIndexGetHeaderInfo(s32Fd, &stIdxHeaderInfo))
    {
        return MT_ERR_PVR_FILE_CANT_READ;
    }
    printf("%s=======%d\n",__FUNCTION__,__LINE__);
    if (stIdxHeaderInfo.u32CADataInfoLen < u32CADataLen)
    {
        MT_ERR_PVR("CA data len is no enough:%d\n", stIdxHeaderInfo.u32CADataInfoLen);
        return MT_FAILURE;
    }
    */
    
    //printf("%s=======%d\n",__FUNCTION__,__LINE__);
    //s32WriteRet = PVR_WRITE(pBuff, u32CADataLen, s32Fd, PVR_GET_CA_DATA_OFFSET());
    s32WriteRet = PVR_WRITE(pBuff, u32CADataLen, s32Fd, 0);	
    if ((mt_u32)s32WriteRet != u32CADataLen)
    {
        MT_ERR_PVR("read CA data info err:0x%x\n", s32WriteRet);
        return MT_FAILURE;
    }
    //printf("%s >>> u32CADataLen=%d\n",__FUNCTION__,s32WriteRet);
    return s32WriteRet;
}

MT_VOID PVR_Index_GetIdxInfo(PVR_INDEX_HANDLE handle)
{
    MT_S32 i = 0;
    MT_U32 u32StartFrm = 0, u32EndFrm = 0, u32LastFrm = 0;
    MT_U32 u32FindStart = 0;
    PVR_INDEX_ENTRY_S stEntryTmp = {0};
    MT_U32 u32CurGopSize = 0, u32GopSizeSeg = 0;
    
    u32StartFrm = handle->stCycMgr.u32StartFrame;
    u32EndFrm = handle->stCycMgr.u32EndFrame;
    u32LastFrm = handle->stCycMgr.u32LastFrame;

    if (u32StartFrm >= u32EndFrm)
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = u32LastFrm - u32StartFrm + u32EndFrm;
    }
    else
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = u32LastFrm + 1;
    }

    for(i = (MT_S32)u32EndFrm; i >= (MT_S32)u32FindStart; i--)
    {
        if (MT_SUCCESS == PVR_Index_GetFrameByNum(handle, &stEntryTmp, (mt_u32)i))
        {
            if (0 != (stEntryTmp.u16FrameTypeAndGop & 0x3fff))
            {
                u32CurGopSize = (mt_u32)((stEntryTmp.u16FrameTypeAndGop & 0x3fff) + 1);
                
                handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum++;
                
                if(0 != handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum)
                {
                    u32GopSizeSeg = (u32CurGopSize/10);
                    u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
                    handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]++;
                }

                if (handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize < u32CurGopSize)
                {
                    handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize = u32CurGopSize;
                }

                i -= (MT_S32)(stEntryTmp.u16FrameTypeAndGop & 0x3fff);
            }
        }

        if ((u32StartFrm >= u32EndFrm)&&(i <= 0))
        {
            u32FindStart = u32StartFrm;
            i = (MT_S32)u32LastFrm;
            u32EndFrm = u32LastFrm;
            continue;
        }
    } 

    if (MT_SUCCESS == PVR_Index_GetFrameByNum(handle, &stEntryTmp, u32FindStart))
    {
        if (0 != (stEntryTmp.u16FrameTypeAndGop & 0x3fff))
        {
            handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum--;
            u32GopSizeSeg = (u32CurGopSize/10);
            u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
            handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]--;
        }
    }
}

MT_VOID PVR_Index_GetRecIdxInfo(PVR_INDEX_HANDLE handle)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_S32 s32ReadRet = 0;

    if (MT_SUCCESS != PVRIndexGetHeaderInfo(handle->s32HeaderFd, &stIdxHeaderInfo))
    {
        MT_ERR_PVR("Can't get index header info.\n");
        return;
    }
    
    s32ReadRet = PVR_READ(&(handle->stRecIdxInfo), 
                            sizeof(PVR_REC_INDEX_INFO_S), 
                            handle->s32HeaderFd, 
                            PVR_GET_IDX_INFO_OFFSET(stIdxHeaderInfo));
    
    if (s32ReadRet != sizeof(PVR_REC_INDEX_INFO_S))
    {
        MT_ERR_PVR("Write index info fail ret=0x%x\n", s32ReadRet);
        return;
    }
}


MT_VOID PVR_Index_RecIdxInfo(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pstIdxEntry)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    PVR_INDEX_ENTRY_S stRewindStartEntry = {0};
    MT_S32 s32WriteRet = 0;
    MT_U32 u32GopSizeSeg = 0;
    MT_U32 u32RewindStartGopFrmNum = 0;

    if (MT_SUCCESS != PVRIndexGetHeaderInfo(handle->s32HeaderFd, &stIdxHeaderInfo))
    {
        MT_ERR_PVR("Can't get index header info.\n");
        return;
    }

    handle->stRecIdxInfo.u32MagicWord = PVR_REC_INDEX_MAGIC_WORD;

    if (0 != handle->stCycMgr.s32CycTimes)
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = handle->stCycMgr.u32LastFrame - 
                                                          handle->stCycMgr.u32StartFrame + 
                                                          handle->stCycMgr.u32EndFrame;
    }
    else
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = handle->stCycMgr.u32LastFrame + 1;
    }

    if (0 != handle->stCycMgr.s32CycTimes)
    {
        u32RewindStartGopFrmNum = handle->stCycMgr.u32StartFrame;
        
        if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stRewindStartEntry,u32RewindStartGopFrmNum))
        {
            if (PVR_INDEX_is_Iframe(&stRewindStartEntry))
            {
                handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum--;
                
                u32RewindStartGopFrmNum++;
                if (u32RewindStartGopFrmNum > handle->stCycMgr.u32LastFrame)
                {
                    u32RewindStartGopFrmNum = 0;
                }

                do 
                {
                    if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stRewindStartEntry, 
                                                u32RewindStartGopFrmNum))
                    {
                        u32RewindStartGopFrmNum++;
                        if (u32RewindStartGopFrmNum > handle->stCycMgr.u32LastFrame)
                        {
                            u32RewindStartGopFrmNum = 0;
                        }
                    }
                    else
                    {
                        MT_ERR_PVR("Can't get index %d entry.\n",u32RewindStartGopFrmNum);
                    }
                }
                while (!PVR_INDEX_is_Iframe(&stRewindStartEntry));
                
                if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stRewindStartEntry, 
                                                (u32RewindStartGopFrmNum-2)))
                {
                    u32GopSizeSeg = (mt_u32)(((stRewindStartEntry.u16FrameTypeAndGop & 0x3fff) + 1)/10);
                    u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
                    handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]--;
                }
                else
                {
                    MT_WARN_PVR("Can't get index %d entry.\n",u32RewindStartGopFrmNum);
                }
                
                s32WriteRet = PVR_WRITE(&(handle->stRecIdxInfo), 
                            sizeof(PVR_REC_INDEX_INFO_S), 
                            handle->s32HeaderFd, 
                            PVR_GET_IDX_INFO_OFFSET(stIdxHeaderInfo));
    
                if (s32WriteRet != sizeof(PVR_REC_INDEX_INFO_S))
                {
                    MT_ERR_PVR("Write index info fail ret=0x%x\n", s32WriteRet);
                    return;
                }
            }
        }
        else
        {
            MT_ERR_PVR("Can't get index %d entry.\n",u32RewindStartGopFrmNum);
        }
    }

    if (PVR_INDEX_is_Iframe(pstIdxEntry))
    {
        handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum++;

        if (handle->stRecIdxInfo.u32LastGopSize > handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize)
        {
            handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize = handle->stRecIdxInfo.u32LastGopSize;
        }

        if (0 != handle->stRecIdxInfo.u32LastGopSize)
        {
            u32GopSizeSeg = (handle->stRecIdxInfo.u32LastGopSize/10);
            u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
            handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]++;
        }

        s32WriteRet = PVR_WRITE(&(handle->stRecIdxInfo), 
                            sizeof(PVR_REC_INDEX_INFO_S), 
                            handle->s32HeaderFd, 
                            PVR_GET_IDX_INFO_OFFSET(stIdxHeaderInfo));
    
        if (s32WriteRet != sizeof(PVR_REC_INDEX_INFO_S))
        {
            MT_ERR_PVR("Write index info fail ret=0x%x\n", s32WriteRet);
            return;
        }
    }

    handle->stRecIdxInfo.u32LastGopSize = (mt_u32)((pstIdxEntry->u16FrameTypeAndGop & 0x3fff) + 1);
    
    return;
}

MT_VOID PVR_Index_UpdateIdxInfoWhenRewind(PVR_INDEX_HANDLE handle)
{
    PVR_INDEX_ENTRY_S stEntryTmp = {0};
    MT_U32 u32GopSizeSeg = 0;
    MT_U32 i = handle->stCycMgr.u32EndFrame;

    if (1 != handle->stCycMgr.s32CycTimes)
    {
        return;
    }

    while(1) 
    {
        if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stEntryTmp, i))
        {
            if (PVR_INDEX_is_Iframe(&stEntryTmp))
            {
                if (i != handle->stCycMgr.u32EndFrame)
                {
                    if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stEntryTmp, (i-1)))
                    {
                        u32GopSizeSeg = (mt_u32)(((stEntryTmp.u16FrameTypeAndGop & 0x3fff) + 1)/10);
                        u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
                        handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]--;
                    }
                    else
                    {
                        MT_ERR_PVR("Can't get index %d entry.\n",i);
                    }
                }
                
                if (i >= handle->stCycMgr.u32StartFrame)
                {
                    break;
                }
                
                handle->stRecIdxInfo.stIdxInfo.u32GopTotalNum--;
            }
        }
        else
        {
            if(i >= handle->stCycMgr.u32StartFrame){    //fix bug109634 .i++ maybe Constantly run never stop!
                break;
            }
            MT_ERR_PVR("Can't get index %d entry1.\n",i);
        }
        i++;
    }
}

MT_VOID PVR_Index_RecLastIdxInfo(PVR_INDEX_HANDLE handle)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    PVR_INDEX_ENTRY_S stEntryTmp = {0};
    MT_U32 u32GopSizeSeg = 0;
    MT_S32 s32WriteRet = 0;

    if (MT_SUCCESS != PVRIndexGetHeaderInfo(handle->s32HeaderFd, &stIdxHeaderInfo))
    {
        MT_ERR_PVR("Can't get index header info.\n");
        return;
    }

    /*if (0 != handle->stCycMgr.s32CycTimes)
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = handle->stCycMgr.u32LastFrame - 
                                                          handle->stCycMgr.u32StartFrame + 
                                                          handle->stCycMgr.u32EndFrame;
    }
    else
    {
        handle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum = handle->stCycMgr.u32LastFrame;
    }*/

    if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &stEntryTmp, (handle->stCycMgr.u32EndFrame -1)))
    {
        if ((mt_u32)((stEntryTmp.u16FrameTypeAndGop & 0x3fff) + 1) > handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize)
        {
            handle->stRecIdxInfo.stIdxInfo.u32MaxGopSize = (mt_u32)((stEntryTmp.u16FrameTypeAndGop & 0x3fff) + 1);
        }
        
        u32GopSizeSeg = (mt_u32)(((stEntryTmp.u16FrameTypeAndGop & 0x3fff) + 1)/10);
        u32GopSizeSeg = (u32GopSizeSeg > 12) ? 12 : u32GopSizeSeg;
        handle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[u32GopSizeSeg]++;
    }
    else
    {
        MT_ERR_PVR("Can't get index %d entry.\n",handle->stCycMgr.u32EndFrame);
    }

    s32WriteRet = PVR_WRITE(&(handle->stRecIdxInfo), 
                            sizeof(PVR_REC_INDEX_INFO_S), 
                            handle->s32HeaderFd, 
                            PVR_GET_IDX_INFO_OFFSET(stIdxHeaderInfo));
    
    if (s32WriteRet != sizeof(PVR_REC_INDEX_INFO_S))
    {
        MT_ERR_PVR("Write index info fail ret=0x%x\n", s32WriteRet);
        return;
    }
}

MT_VOID PVR_Index_RecUpdateStartFrame(PVR_INDEX_HANDLE handle, MT_U64 u64RecSize)
{
    PVR_INDEX_ENTRY_S stEntryTmp = {0};
    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U64 u64EndOffset = 0;
    MT_S32 s32Ret = 0;

    u32StartFrame = handle->stCycMgr.u32StartFrame;
    if(u32StartFrame == 0 && handle->stCycMgr.u32LastFrame == 0) {
        MT_ERR_PVR("invalid index for update\n");
        return;
    }
    
    PVR_Index_GetFrameByNum(handle, &stEntryTmp, u32StartFrame);
    if(s32Ret != MT_SUCCESS) {
        MT_ERR_PVR("get start frame %u failed\n", u32StartFrame, s32Ret);
        return;
    }
    u64EndOffset = u64RecSize % handle->stCycMgr.u64MaxCycSize;
    printf("start frame %u offset %llu/%llu, cycles %d\n", u32StartFrame, stEntryTmp.u64Offset, stEntryTmp.u64GlobalOffset, stEntryTmp.s32CycTimes);

    u32EndFrame = handle->stCycMgr.u32EndFrame - 1;
    PVR_Index_GetFrameByNum(handle, &stEntryTmp, u32EndFrame);
    if(s32Ret != MT_SUCCESS) {
        MT_ERR_PVR("get end frame %u failed\n", u32EndFrame, s32Ret);
        return;
    }
   
    printf("end frame %u offset %llu/%llu, cycles %d\n", u32EndFrame-1, stEntryTmp.u64Offset, stEntryTmp.u64GlobalOffset, stEntryTmp.s32CycTimes);
    if(u64EndOffset > stEntryTmp.u64Offset) {
        
    } else {

    }

    //PVRIndexGetEntryByNum(handle, &stEntryTmp, u32EndFrame - 1);
}

MT_VOID PVR_Index_RecUpdateEndFrame(PVR_INDEX_HANDLE handle)
{
    PVR_INDEX_ENTRY_S stEntryTmp = {0};
    MT_U32 u32EndFrame = 0;
    MT_S32 u32Ret = 0;

    u32EndFrame = handle->stCycMgr.u32EndFrame;
    if(u32EndFrame == 0 && handle->stCycMgr.u32LastFrame == 0) {
        MT_ERR_PVR("invalid index for update\n");
        return;
    }
    
    u32Ret = PVR_Index_GetPreIFrameByNum(handle, &stEntryTmp, u32EndFrame);
    
    if(handle->stCycMgr.u32EndFrame >= u32Ret)
        u32EndFrame = handle->stCycMgr.u32EndFrame - u32Ret;
    else
        u32EndFrame = handle->stCycMgr.u32LastFrame - u32Ret - handle->stCycMgr.u32EndFrame;
    
    if(handle->stCycMgr.u32EndFrame == handle->stCycMgr.u32LastFrame)
        handle->stCycMgr.u32LastFrame = u32EndFrame;

    PVRIndexGetEntryByNum(handle, &stEntryTmp, u32EndFrame - 1);

    printf("update end frame from %u to %u/%u/%u, frame type is 0x%x, distance %u, cycles %d\n", handle->stCycMgr.u32EndFrame, 
        handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32LastFrame, u32EndFrame, stEntryTmp.u16FrameTypeAndGop, u32Ret, handle->stCycMgr.s32CycTimes);

    handle->stCycMgr.u32EndFrame = u32EndFrame;
}

#if 0
MT_BOOL PVR_Index_CheckSetRecReachPlay(PVR_INDEX_HANDLE handle)
{
    MT_U32 u32ReadFrame = 0;
    MT_U32 u32StartFrame = 0;
    MT_U32 u32EndFrame = 0;
    MT_U32 u32LastFrame = 0;

    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32EndFrame = handle->stCycMgr.u32EndFrame;
    u32ReadFrame = handle->u32ReadFrame;
    u32LastFrame = handle->stCycMgr.u32LastFrame;


    if (u32StartFrame < u32EndFrame) /* NOT cycled, 0--S--R--E--L  */
    {

        if ((MT_S32)u32StartFrame + PVR_TPLAY_MIN_DISTANCE > (MT_S32)u32ReadFrame)
        {
            MT_ERR_PVR("Rec almost over Play: S/R/E/L: %u,%u,%u,%u.\n",
                   u32StartFrame, u32ReadFrame,u32EndFrame, u32LastFrame);
            handle->u32RecReachPlay = 1;
            return MT_TRUE;
        }
        else
        {
            return MT_FALSE;
        }
    }
    else  /* Cycled */
    {
        if (u32ReadFrame > u32StartFrame) /* 0----E----S----R--L */
        {
            if (u32ReadFrame - u32StartFrame > PVR_TPLAY_MIN_DISTANCE)
            {
                return MT_FALSE;
            }
            else
            {
                MT_ERR_PVR("Rec almost over Play: E/S/R/L: %u,%u,%u,%u.\n",
                   u32EndFrame, u32StartFrame, u32ReadFrame, u32LastFrame);
                handle->u32RecReachPlay = 1;
                return MT_TRUE;
            }
        }
        else /* 0--R--E----S--L */
        {
            MT_U32 startToLast;

            startToLast = u32LastFrame - u32StartFrame;

            if (startToLast + u32ReadFrame > PVR_TPLAY_MIN_DISTANCE)
            {
                return MT_FALSE;
            }
            else
            {
                MT_ERR_PVR("Rec almost over Play: R/E/S/L: %u,%u,%u,%u.\n",
                   u32ReadFrame, u32EndFrame, u32StartFrame, u32LastFrame);

                handle->u32RecReachPlay = 1;
                return MT_TRUE;
            }
        }
    }
}
#endif

MT_BOOL PVR_Index_QureyClearRecReachPlay(PVR_INDEX_HANDLE handle)
{
    if (1 == handle->u32RecReachPlay)
    {
        PVR_IDX_D("----- u32RecReachPlay==1 -------\n");
        handle->u32RecReachPlay = 0;
        return MT_TRUE;
    }

    return MT_FALSE;
}

/*****************************************************************************
 Prototype       : PVR_IndexGetNextEntry
 Description     : to find next frame,
 first, check whether the read pointer is beyond the mark(by the result of read interface, if failure, the device maybe be unplugged)
 sedond, read frame
 last, move the read pointer to the next frame. again, check the valid, because, the end maybe not the last frame of index file
 so, this interface can be used for checking whether it reach to the end of the index file.
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
static MT_S32 PVRIndexGetNextEntry(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    ssize_t readNum;
    MT_U32 u32OldFrame;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    
    //PVR_EVENT_D("S:%d, E:%d, L:%d, R:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
     //          handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
    
    //PVR_EVENT_D("before get: Read frame:%u, Type:%u, offset:%llu, PTS:%u, Time:%u \n", handle->u32ReadFrame,
    //            PVR_INDEX_get_frameType(pEntry), pEntry->u64Offset, pEntry->u32PtsMs, pEntry->u32DisplayTimeMs);
    //readNum = PVR_READALL(pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd, (handle->u32ReadFrame * sizeof(PVR_INDEX_ENTRY_S)));
    if(handle->u32ReadFrame==handle->stCycMgr.u32EndFrame){ //sym4
        return MT_ERR_PVR_FILE_TILL_END;
    }
    PVR_READ_INDEX(readNum,pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd,
                             (handle->u32ReadFrame * sizeof(PVR_INDEX_ENTRY_S)), handle);

    if (readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        if (-1 == readNum)
        {
            PVR_EVENT_D("read index error: ");
            return MT_ERR_PVR_FILE_CANT_READ;
        }
        else
        {
            PVR_EVENT_D("read to end, cur and next is same: S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
            return MT_ERR_PVR_FILE_TILL_END;
        }
    }

    u32OldFrame = handle->u32ReadFrame;
    PVRIndexCycMoveReadFrame(handle, 1);
    if (u32OldFrame == handle->u32ReadFrame)
    {
        PVR_EVENT_D("read to end0, S:%d, E:%d, L:%d, C:%d, O:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame, u32OldFrame);
        return MT_ERR_PVR_FILE_TILL_END;
    }

    //PVR_EVENT_D("after get: Read frame:%u, Type:%u, offset:%llu, PTS:%u, Time:%u \n", handle->u32ReadFrame,
      //          PVR_INDEX_get_frameType(pEntry), pEntry->u64Offset, pEntry->u32PtsMs, pEntry->u32DisplayTimeMs);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_IndexGetPlayNextEntry
 Description     : to find next frame of current play frame,
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 
    Author       : 
    Modification : Created function

*****************************************************************************/
MT_S32 PVRIndexGetPlayNextEntry(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    ssize_t readNum;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    
    PVR_READ_INDEX(readNum, pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd,
                             (handle->u32PlayFrame * sizeof(PVR_INDEX_ENTRY_S)), handle);

    if (readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        if (-1 == readNum)
        {
            MT_WARN_PVR("read index error: ");
            return MT_ERR_PVR_FILE_CANT_READ;
        }
        else
        {
            MT_INFO_PVR("read to end, cur and next is same: S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
            return MT_ERR_PVR_FILE_TILL_END;
        }
    }
    MT_INFO_PVR("after get: Read frame:%u, Type:%u, offset:%llu, PTS:%u, Time:%u \n", handle->u32ReadFrame,
                PVR_INDEX_get_frameType(pEntry), pEntry->u64Offset, pEntry->u32PtsMs, pEntry->u32DisplayTimeMs);
    handle->u32PlayFrame = PVRIndexCalcNewPos(handle, handle->u32PlayFrame, 1);
    return MT_SUCCESS;
}



/*****************************************************************************
 Prototype       : PVRIndexGetNextIEntry
 Description     : to find next I frame
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_S32 PVRIndexGetNextIEntry(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    while (1)
    {
        ret = PVRIndexGetNextEntry(handle, pEntry);
        if (ret != MT_SUCCESS)
        {
            return ret;
        }

        /* I frame, and not found the frame upflow flag*/
        if (PVR_INDEX_is_Iframe(pEntry) && !(pEntry->u16UpFlowFlag))
        {
            break;
        }
        handle->u32FrameDistance++;
    }

    return MT_SUCCESS;
}
/*****************************************************************************
 Prototype       : PVRIndexGetNextIPEntry
 Description     : to find next I or P frame
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_S32 PVRIndexGetNextIPEntry(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    while (1)
    {
        ret = PVRIndexGetNextEntry(handle, pEntry);
        if (ret != MT_SUCCESS)
        {
            return ret;
        }

        /* I frame, and not found the frame upflow flag*/
        if ((PVR_INDEX_is_Iframe(pEntry) || PVR_INDEX_is_Pframe(pEntry)) && !(pEntry->u16UpFlowFlag))
        {
            break;
        }
        handle->u32FrameDistance++;
    }

    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : PVR_IndexGetPreEntry
 Description     : to find previous frame
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_S32 PVRIndexGetPreEntry(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    ssize_t readNum ;
    MT_U32 u32OldFrame;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    MT_INFO_PVR("S:%d, E:%d, L:%d, R:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
               handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);

    u32OldFrame = handle->u32ReadFrame;
    PVRIndexCycMoveReadFrame(handle, (MT_S32)(-1));

    if (u32OldFrame == handle->u32ReadFrame)
    {
        MT_WARN_PVR("read to start, cur and pre is same: S:%d, E:%d, L:%d, C:%d, O:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame, u32OldFrame);
        return MT_ERR_PVR_FILE_TILL_START;
    }

    PVR_READ_INDEX(readNum, pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd,
                             (handle->u32ReadFrame * sizeof(PVR_INDEX_ENTRY_S)), handle);
    if (readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        if (-1 == readNum)
        {
            MT_WARN_PVR("read index error: ");
            return MT_ERR_PVR_FILE_CANT_READ;
        }
        else
        {
            MT_WARN_PVR("read to start,  S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
            return MT_ERR_PVR_FILE_TILL_START;
        }
    }

    MT_INFO_PVR("after get: R:%u, Type:%u, offset:%llu, PTS:%u \n", handle->u32ReadFrame,
                PVR_INDEX_get_frameType(pEntry), pEntry->u64Offset, pEntry->u32PtsMs);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_IndexGetPreIEntry
 Description     : to find the previous I frame
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_S32 PVRIndexGetPreIEntry(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    while (1)
    {
        ret = PVRIndexGetPreEntry(handle, pEntry);
        if (ret != MT_SUCCESS)
        {
            return ret;
        }

        /* I frame, and not found the frame upflow flag*/
        if (PVR_INDEX_is_Iframe(pEntry) && !(pEntry->u16UpFlowFlag))
        {
            break;
        }
        handle->u32FrameDistance++;
    }


    return MT_SUCCESS;
}
STATIC INLINE MT_S32 PVRIndexGetPreXEntry(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    while (1)
    {
        ret = PVRIndexGetPreEntry(handle, pEntry);
        if (ret != MT_SUCCESS)
        {
            return ret;
        }

        /* I frame, and not found the frame upflow flag*/
        if (!(pEntry->u16UpFlowFlag))
        {
            break;
        }
        handle->u32FrameDistance++;
    }


    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : PVR_IndexGetCurrentEntry
 Description     : to get current frame information
                    if the read pointer reach to the end, it will return failure, because of it reach to the end of the file .
 Input           : handle  **
                   pEntry  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/17
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_S32 PVRIndexGetCurrentEntry(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry)
{
    ssize_t readNum ;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pEntry != NULL);

    MT_INFO_PVR("S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
               handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);

    if (handle->u32ReadFrame == handle->stCycMgr.u32EndFrame)
    {
        MT_WARN_PVR("read to end1, S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
               handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
        return MT_ERR_PVR_FILE_TILL_END;
    }

    PVR_READ_INDEX(readNum, pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd,
                             (handle->u32ReadFrame * sizeof(PVR_INDEX_ENTRY_S)), handle);
    if (readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        /* PVR play to the end of file, no way for PVR_EVENT_PLAY_EOF, AI7D02611 */
        if (-1 == readNum)
        {
            MT_WARN_PVR("read failed in PVRIndexGetCurrentEntry");
            return MT_ERR_PVR_FILE_CANT_READ;
        }
        else
        {
            MT_WARN_PVR("read to end2, S:%d, E:%d, L:%d, C:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame);
            return MT_ERR_PVR_FILE_TILL_END;
        }
    }

    MT_INFO_PVR("frame cur <Read frame:%u, Type:%u, offset:%llu, PTS:%u> \n", handle->u32ReadFrame,
                PVR_INDEX_get_frameType(pEntry), pEntry->u64Offset, pEntry->u32PtsMs);

    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRIndexGetEntryByNum(const PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry, MT_U32 u32FrameNum)
{
    ssize_t readNum ;
    PVR_READ_INDEX(readNum, pEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32ReadFd,
                             (u32FrameNum * sizeof(PVR_INDEX_ENTRY_S)), handle);
    if (readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        if (-1 == readNum)
        {
            MT_WARN_PVR("read failed in PVRIndexGetEntryByNum");
            return MT_ERR_PVR_FILE_CANT_READ;
        }
        else
        {
            MT_WARN_PVR("read to end0, S:%d, E:%d, L:%d, C:%d G:%d\n", handle->stCycMgr.u32StartFrame, handle->stCycMgr.u32EndFrame,
                   handle->stCycMgr.u32LastFrame, handle->u32ReadFrame, u32FrameNum);
            return MT_ERR_PVR_FILE_TILL_END;
        }
    }

    return MT_SUCCESS;
}


STATIC MT_U32 PVRIndexSeachByTime(PVR_INDEX_HANDLE handle, MT_U32 timeWant,
                                  MT_U32 start, MT_U32 end,  PVR_FILE seekFd)
{
    ssize_t l_readNum;
    MT_U32 target;
    PVR_INDEX_ENTRY_S entry;
    MT_U32 nextStart, nextEnd;

    memset(&entry, 0, sizeof(PVR_INDEX_ENTRY_S));   
    target = (start + end)/2;

    if (target == start || target == end)
    {
        MT_WARN_PVR("PVRIndexSeachByTime end, ret:%d\n", target);
        return target;
    }

    /* get target's time */
    PVR_READ_INDEX(l_readNum, &entry, sizeof(PVR_INDEX_ENTRY_S), seekFd,
                               (target*sizeof(PVR_INDEX_ENTRY_S)), handle);
    if (l_readNum != (ssize_t)sizeof(PVR_INDEX_ENTRY_S))
    {
        MT_ERR_PVR("read err,  want:%u, get:%u, off:%u\n", (sizeof(PVR_INDEX_ENTRY_S)), (l_readNum), target*sizeof(PVR_INDEX_ENTRY_S));
        if (-1 == l_readNum)
        {
            MT_WARN_PVR("read index error: ");
            return 0;
        }
        else if (0 == l_readNum) /* if meet error at the end of file, return the last frame AI7D03033 */
        {
            MT_U32 u32LastPos;
            u32LastPos = (MT_U32)PVR_SEEK(seekFd, (off_t)(0 - (MT_S32)sizeof(PVR_INDEX_ENTRY_S)), SEEK_END);
            if ((MT_S32)u32LastPos >= 0)
            {
                (void)PVR_READ(&entry, sizeof(PVR_INDEX_ENTRY_S), seekFd, u32LastPos);
                return (u32LastPos / sizeof(PVR_INDEX_ENTRY_S));
            }
            else
            {
                MT_WARN_PVR("can't get the last frame\n");
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }

    MT_INFO_PVR("^^^^ search time:want=%d, target=%d ^^^^\n", timeWant, entry.u32DisplayTimeMs);
    if (entry.u32DisplayTimeMs <= timeWant)
    {
        nextStart = target;
        nextEnd = end;
    }
    else
    {
        nextStart = start;
        nextEnd = target;
    }

    return PVRIndexSeachByTime(handle, timeWant, nextStart, nextEnd, seekFd);
}

/*****************************************************************************
 Prototype       : PVRIndexFindFrameByTime
 Description     : find a frame match the time
 Input           : handle           **
                   offsetFromStart  **
 Output          : None
 Return Value    : the frame ID from start of file.
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/6/30
    Author       : fd
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_U32 PVRIndexFindFrameByTime(PVR_INDEX_HANDLE handle, MT_U32 u32FindTime)
{
    MT_U32  frameId;
    MT_U32  u32StartFrame;
    MT_U32  u32EndFrame;

    PVR_INDEX_ENTRY_S lastEntry;
    MT_S32 s32Offset;
    MT_S32 s32Read;
    MT_U32 u32LastFrame;

    u32EndFrame = handle->stCycMgr.u32EndFrame;
    u32StartFrame = handle->stCycMgr.u32StartFrame;
    u32LastFrame = handle->stCycMgr.u32LastFrame;

    memset(&lastEntry, 0, sizeof(PVR_INDEX_ENTRY_S));   
    /* Not rewind, find it directly*/
    if (u32EndFrame > u32StartFrame)
    {
        frameId = PVRIndexSeachByTime(handle, u32FindTime, u32StartFrame,
                                      u32EndFrame,  handle->s32SeekFd);
    }
    else /* Rewind, find it for two part*/
    {
        s32Offset = (MT_S32)(sizeof(PVR_INDEX_ENTRY_S) * (handle->stCycMgr.u32LastFrame));
        PVR_READ_INDEX(s32Read, &lastEntry, sizeof(PVR_INDEX_ENTRY_S), handle->s32SeekFd, (MT_U32)s32Offset, handle);
        if (s32Read != (MT_S32)sizeof(PVR_INDEX_ENTRY_S))
        {
            MT_ERR_PVR("MT_PVR_GetFileAttrByFileName-read idx failed\n");
            return (handle->stCycMgr.u32StartFrame);
        }

        MT_INFO_PVR("last entry PTS=%d ms\n", lastEntry.u32DisplayTimeMs);

        if (u32FindTime <= lastEntry.u32DisplayTimeMs)
        {
            MT_WARN_PVR("u32FindTime:%u, u32DisplayTimeMs:%u, find it in the last section\n", u32FindTime,lastEntry.u32DisplayTimeMs);
            frameId = PVRIndexSeachByTime(handle, u32FindTime, u32StartFrame,
                                          u32LastFrame,  handle->s32SeekFd);
        }
        else /* find it in the start part */
        {
            MT_WARN_PVR("u32FindTime:%u, u32DisplayTimeMs:%u, find it in the first section\n",u32FindTime,lastEntry.u32DisplayTimeMs);
            if (0 == handle->stCycMgr.u32EndFrame)
            {
                return handle->stCycMgr.u32EndFrame;
            }
            else
            {
                frameId = PVRIndexSeachByTime(handle, u32FindTime, 0,
                                          u32EndFrame,  handle->s32SeekFd);
            }
            
        }
    }

    return frameId;
}

/* find PTS from u32FrmPos direction forward or backward, which close to the frame of u32PtsMs, and return the frame number */
MT_U32 PVRIndexFindFrameByPTS(PVR_INDEX_HANDLE handle, MT_U32 u32PtsSearched, MT_U32 u32FrmPos, MT_U32 IsForword)
{
    PVR_INDEX_ENTRY_S entry,IEntry;
    MT_U32 u32SearchPos;
    MT_U32 u32LastEntryPts;
    MT_S32 s32FrameNum;

    memset(&entry, 0, sizeof(PVR_INDEX_ENTRY_S)); 
	memset(&IEntry, 0, sizeof(PVR_INDEX_ENTRY_S));
    /*the pos invalid, return the start frame */
    if (!PVRIndexIsFrameValid(handle, u32FrmPos))
    {
        u32SearchPos = handle->stCycMgr.u32StartFrame;
        MT_WARN_PVR("PVRIndexFindFrameByPTS u32FrmPos %u is not valid, return start frame\n", u32FrmPos);
        return u32SearchPos;
    }

    u32SearchPos = u32FrmPos;

    /* reach to the end of TS */
    if (PVRIndexIsFrameEnd(handle, u32SearchPos))
    {
        MT_WARN_PVR("*****************************u32SearchPos:%u is to the end!\n", u32SearchPos);
        u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, -1);
        if (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &entry, u32SearchPos))
        {
            /* the finding frame more than the last frame, it maybe the last B/P frame, check the difference less than one sequence */
            if ((entry.u32PtsMs < u32PtsSearched) && (u32PtsSearched - entry.u32PtsMs < 5000))
            {
                MT_WARN_PVR("get to end and small than to seek: to seek=%u, end:%u\n", u32PtsSearched, entry.u32PtsMs);
                return u32SearchPos;
            }
        }
        else
        {
            MT_ERR_PVR("No frame in index file.\n");
            u32SearchPos = handle->stCycMgr.u32StartFrame;
            return u32SearchPos;
        }
    }
    else
    {
        MT_WARN_PVR("*****************************u32SearchPos:%u is not to the end and end frame is %u!\n", u32SearchPos,handle->stCycMgr.u32EndFrame);
        if (MT_SUCCESS != PVRIndexGetEntryByNum(handle, &entry, u32SearchPos))
        {
            MT_ERR_PVR("Frame in index file error.\n");
            u32SearchPos = handle->stCycMgr.u32StartFrame;
            return u32SearchPos;
        }
    }

    /* the reading equal the playing, return it directly */
    if (entry.u32PtsMs == u32PtsSearched)
    {
        MT_WARN_PVR("seek OK: to seek=%u, pos:%u\n", u32PtsSearched, u32SearchPos);
        return u32SearchPos;
    }

    /* save the fitst frame PTS */
    u32LastEntryPts = entry.u32PtsMs;

    /*find forward, used for play backward*/
    if (IsForword)
    {
        u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, 1);
        while(!PVRIndexIsFrameEnd(handle, u32SearchPos) && (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &entry, u32SearchPos)))
        {
            /* invalid PTS, continue*/
            if ((0 == entry.u32PtsMs) || (PVR_INDEX_INVALID_PTSMS == entry.u32PtsMs))
            {
                u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, 1);
                continue;
            }
            s32FrameNum = PVR_Index_GetPreIFrameByNum(handle,&IEntry,u32SearchPos);
            if (s32FrameNum > 0 && IEntry.u32PtsMs == entry.u32PtsMs)//prev I frame pts == current frame pts.
            {
                entry.u32PtsMs += (mt_u32)s32FrameNum*PVR_INDEX_DEFFRAME_PTSMS;//use 40ms as frame during time for simple
            }

            if ((entry.u32PtsMs == u32PtsSearched)  /* equal, find it */
                || ((u32LastEntryPts < u32PtsSearched) && (entry.u32PtsMs > u32PtsSearched)) /* previous frame less than next frame, use current I frame */
                || ((u32LastEntryPts > entry.u32PtsMs)
                    && (u32LastEntryPts < u32PtsSearched) && (entry.u32PtsMs < u32PtsSearched)
                    && ((MT_U32)abs((MT_S32)(u32LastEntryPts - u32PtsSearched)) < 2000))) /* rewind ts, both previous frame and next frame less than find fram, and before rewinding,  the difference between last frame and find frame less than 2 second */
            {
                break;
            }
            u32LastEntryPts = entry.u32PtsMs;
            u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, 1);
        }
    }
    else     /*find backward, used for play forward */
    {
        u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, -1);
        while (!PVRIndexIsFrameStart(handle, u32SearchPos) && (MT_SUCCESS == PVRIndexGetEntryByNum(handle, &entry, u32SearchPos)))
        {
            /* invalid PTS, continue*/
            if ((0 == entry.u32PtsMs) || (PVR_INDEX_INVALID_PTSMS == entry.u32PtsMs))
            {
                u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, -1);
                continue;
            }
            s32FrameNum = PVR_Index_GetPreIFrameByNum(handle,&IEntry,u32SearchPos);
            if (s32FrameNum > 0 && IEntry.u32PtsMs == entry.u32PtsMs)//prev I frame pts == current frame pts.
            {
                entry.u32PtsMs += (mt_u32)s32FrameNum*PVR_INDEX_DEFFRAME_PTSMS;//use 40ms as frame gap for simple
            }

            if ((entry.u32PtsMs == u32PtsSearched)  /* equal, find it */
                || ((u32LastEntryPts > u32PtsSearched) && (entry.u32PtsMs < u32PtsSearched)) /* previous frame more than next frame, use current I frame */
                || ((u32LastEntryPts <= entry.u32PtsMs)
                    && (u32LastEntryPts < u32PtsSearched) && (entry.u32PtsMs < u32PtsSearched)
                    && ((MT_U32)abs((int)(entry.u32PtsMs - u32PtsSearched)) < 2000))) /* rewind ts, both previous frame and next frame less than find fram, and before rewinding,  the difference between last frame and find frame less than 2 second */
            {
                break;
            }
            u32LastEntryPts = entry.u32PtsMs;
            u32SearchPos = PVRIndexCalcNewPos(handle, u32SearchPos, -1);
        }
    }

    MT_WARN_PVR("seek OK: to seek=%u, now:%u\n", u32PtsSearched, entry.u32PtsMs);

    return u32SearchPos;
}

/*****************************************************************************
 Prototype       : PVR_Index_SeekByFrame2I
 Description     : seek by frame
 Input           : handle  **
                   offset  ** number of frame
                   whence  ** from start frame
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/28
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_SeekByFrame2I(PVR_INDEX_HANDLE handle, MT_S32 offset, MT_S32 whence)
{
    MT_S32 ret = MT_SUCCESS;
//    MT_S32 pos;
//    MT_U32 maxFrameNum;

    MT_INFO_PVR("whence:%s, offset:%d\n", WHENCE_STRING(whence), offset);

    PVR_INDEX_LOCK(handle);
    switch ( whence )
    {
    case SEEK_SET :
        handle->u32ReadFrame = handle->stCycMgr.u32StartFrame;
        PVRIndexCycMoveReadFrame(handle, offset);
        break;
    case SEEK_CUR :
        PVRIndexCycMoveReadFrame(handle, offset);
        break;
    case SEEK_END:
        handle->u32ReadFrame = handle->stCycMgr.u32EndFrame;
        PVRIndexCycMoveReadFrame(handle, offset);
        break;
    default:
        PVR_INDEX_UNLOCK(handle);
        return MT_ERR_PVR_INVALID_PARA;
    }

    if ((SEEK_SET == whence) || ((SEEK_CUR == whence) && (offset > 0)))
    {
        ret = PVRIndexGetNextIEntry(handle, &(handle->stCurPlayFrame));
        if (MT_SUCCESS != ret)
        {
            ret = PVRIndexGetPreIEntry(handle, &(handle->stCurPlayFrame));
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("get next I entry error, file end.\n");
            }
        }
        else
        {
            PVRIndexCycMoveReadFrame(handle, -1);
        }
    }

    if ((SEEK_END == whence) || ((SEEK_CUR == whence) && (offset < 0)))
    {
        ret = PVRIndexGetPreIEntry(handle, &(handle->stCurPlayFrame));
        if (MT_SUCCESS != ret)
        {
            ret = PVRIndexGetNextIEntry(handle, &(handle->stCurPlayFrame));
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("get next I entry error, file end.\n");
            }
            else
            {
                PVRIndexCycMoveReadFrame(handle, -1);
            }
        }
    }

    MT_INFO_PVR("Ret:%#x. Cur frame Type:%lu, PTS:%u, at last seekto:%u\n",
               ret,
               PVR_INDEX_get_frameType (&(handle->stCurPlayFrame)),
               handle->stCurPlayFrame.u32PtsMs,
               handle->u32ReadFrame);

    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("can not find I frame@both, Err:%#x\n", ret);
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}

MT_S32 PVR_Index_SeekToPTS(PVR_INDEX_HANDLE handle, MT_U32 u32PtsMs, MT_U32 IsForword, MT_U32 IsNextForword)
{
    PVR_INDEX_ENTRY_S entry;
    MT_U32 u32PtsPos;

    memset(&entry,0,sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_LOCK(handle);
    MT_WARN_PVR("seek to PTS:%u\n", u32PtsMs);

    u32PtsPos = PVRIndexFindFrameByPTS(handle, u32PtsMs, handle->u32ReadFrame, IsForword);
    MT_WARN_PVR("seek to PTS:%u, Pos:%u\n", u32PtsMs, u32PtsPos);

    handle->u32ReadFrame = u32PtsPos;

    /*forward seek, used for backward play*/
    if (IsForword)
    {
        /* continuous to backward play, backward two frame */
        if (IsNextForword)
        {
            (void)PVRIndexGetPreIEntry(handle, &entry);
            (void)PVRIndexGetPreIEntry(handle, &entry);
        }
    }
    else     /*backward seek, used for forward play*/
    {
        /* coninous to forward play, forward two frames */
        if (!IsNextForword)
        {
            (void)PVRIndexGetNextIEntry(handle, &entry);
            (void)PVRIndexGetNextIEntry(handle, &entry);
            PVRIndexCycMoveReadFrame(handle, -1);
        }
    }
    MT_INFO_PVR("seek OK: to seek=%u, now:%u\n", u32PtsMs, entry.u32PtsMs);

    PVR_INDEX_UNLOCK(handle);

    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : PVR_Index_SeekToTime
 Description     : seek the read pointer of index to I frame closed to the time value
 Input           : handle         **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_SeekToTime(PVR_INDEX_HANDLE handle, MT_U32 u32TimeMs)
{
    MT_U32 frameToSeek;
    MT_U32 offsetEndFrame = 0;
    MT_U32 offsetStartFrame = 0;

    /* frame position to seek by time*/
    frameToSeek = PVRIndexFindFrameByTime(handle, u32TimeMs);

    MT_INFO_PVR("seek to time:%d, frame pos:%u\n", u32TimeMs, frameToSeek);

    if (PVR_IDX_IS_REWIND(handle)) /* rewind */
    {
        if ( MT_FALSE == PVRIndexIsFrameValid(handle, frameToSeek) )
        {
            /* frame position is invalid, so check which close to the frame, and then set it */
            offsetEndFrame = frameToSeek - handle->stCycMgr.u32EndFrame;
            offsetStartFrame = handle->stCycMgr.u32StartFrame - frameToSeek;

            MT_WARN_PVR("frame position(%u) to seek is invalid\n", frameToSeek);
            MT_WARN_PVR("Now startFrame is %u, endFrame is %u, lastFrame is %u\n", handle->stCycMgr.u32StartFrame,
                handle->stCycMgr.u32EndFrame, handle->stCycMgr.u32LastFrame);

            if(offsetStartFrame > offsetEndFrame)
            {
                frameToSeek = handle->stCycMgr.u32EndFrame;
            }
            else
            {
                frameToSeek = handle->stCycMgr.u32StartFrame+10;
            }
        }

        /* frameToSeek should be the offset from u32StartFrame*/
        if (frameToSeek >= handle->stCycMgr.u32StartFrame)
        {
            frameToSeek -= handle->stCycMgr.u32StartFrame;
        }
        else
        {
            frameToSeek = handle->stCycMgr.u32LastFrame - handle->stCycMgr.u32StartFrame + frameToSeek;
        }
    }

    MT_INFO_PVR("seek frame position is %u to PVR_Index_SeekByFrame2I. S:%u, E:%u, L:%u\n", frameToSeek, handle->stCycMgr.u32StartFrame,
                handle->stCycMgr.u32EndFrame, handle->stCycMgr.u32LastFrame);

    return PVR_Index_SeekByFrame2I(handle, (MT_S32)frameToSeek, SEEK_SET);

}

/*****************************************************************************
 Prototype       : PVR_Index_SeekByTime
 Description     : By current time,  the start and end time will offset some time, in millisecond.
 Input           : handle         **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_SeekByTime(PVR_INDEX_HANDLE handle, MT_S64 offset, MT_S32 whence, MT_U32 curplaytime)
{
    MT_S32 ret;
    PVR_INDEX_ENTRY_S    stFrameTmp;
    MT_U32 u32CurFrmTime;
    MT_U32 u32StartFrmTime;
    MT_U32 u32EndFrmTime;
    MT_U32 u32SeekToTime = 0;
    MT_U32 u32StartFrmPos;
    MT_U32 u32EndFrmPos;
    MT_S32 tmp_offset=(MT_S32)offset;
    
    PVR_ALAWYS_PRINT("seek pos(%lld) whence:%s.\n", offset,  WHENCE_STRING(whence));

    memset(&stFrameTmp, 0, sizeof(PVR_INDEX_ENTRY_S));    

    u32StartFrmPos = handle->stCycMgr.u32StartFrame;
    if (handle->stCycMgr.u32EndFrame > 0)
    {
        u32EndFrmPos = handle->stCycMgr.u32EndFrame - 1;
    }
    else
    {
        u32EndFrmPos = handle->stCycMgr.u32LastFrame - 1;
    }
        
    ret = PVRIndexGetEntryByNum(handle, &stFrameTmp, u32StartFrmPos);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("Can't get StartFrame:%d\n", u32StartFrmPos);
        return ret;
    }
    u32StartFrmTime = stFrameTmp.u32DisplayTimeMs;


    ret = PVRIndexGetEntryByNum(handle, &stFrameTmp, u32EndFrmPos);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("Can't get EndFrame:%d\n", u32EndFrmPos);
        return ret;
    }
    u32EndFrmTime = stFrameTmp.u32DisplayTimeMs;

    u32CurFrmTime = curplaytime;

    PVR_ALAWYS_PRINT("frame info start:%d, end:%d, cur:%d\n",
                     u32StartFrmTime, u32EndFrmTime, u32CurFrmTime);

    if (u32CurFrmTime < u32StartFrmTime)
    {
        u32CurFrmTime = u32StartFrmTime;
    }
    if (u32CurFrmTime > u32EndFrmTime)
    {
        u32CurFrmTime = u32EndFrmTime;
    }

    switch ( whence )
    {
    case SEEK_SET:
        if(tmp_offset>=0){
            u32SeekToTime = u32StartFrmTime+(MT_U32)offset;
        }else{
            u32SeekToTime = u32StartFrmTime-(MT_U32)(0-offset);
        }
        //u32SeekToTime = u32StartFrmTime+(MT_S32)offset;
        break;
    case SEEK_CUR:
        if(tmp_offset>=0){
            u32SeekToTime = u32CurFrmTime+(MT_U32)offset;
        }else{
            u32SeekToTime = u32CurFrmTime-(MT_U32)(0-offset);
        }
        //u32SeekToTime = u32CurFrmTime + (MT_S32)offset;
        break;
    case SEEK_END:
        if(tmp_offset>=0){
            u32SeekToTime = u32EndFrmTime+(MT_U32)offset;
        }else{
            u32SeekToTime = u32EndFrmTime-(MT_U32)(0-offset);
        }
        //u32SeekToTime = u32EndFrmTime + (MT_S32)offset;
        break;
    default:
        return MT_ERR_PVR_INVALID_PARA;
    }

    if ((MT_S32)u32SeekToTime > (MT_S32)u32EndFrmTime) /* over the end, set it the end */
    {
        u32SeekToTime = u32EndFrmTime;
    }
    else if ((MT_S32)u32SeekToTime < (MT_S32)u32StartFrmTime) /* less the start, set it the start */
    {
        if(handle->bIsRec && handle->stCycMgr.s32CycTimes){
            /* rewind record, if seek time set to rec start time, rec start frame may over play frame, need to adjust,  
                otherwise MT_PVR_PlayGetStatus may +++out of frame.range
            */
            PVR_ALAWYS_PRINT("rewind record cyctimes=%d, seek time=%d < start frm time=%d\n",handle->stCycMgr.s32CycTimes,(MT_S32)u32SeekToTime,(MT_S32)u32StartFrmTime);
            u32SeekToTime = u32StartFrmTime+2500;/* suggest move forward 2.5s,can adjust this value */
            PVR_ALAWYS_PRINT("move seek time forward 2.5s seek time=%d\n",(MT_S32)u32SeekToTime);
        }else{    
            u32SeekToTime = u32StartFrmTime;
        }
    }

    PVR_ALAWYS_PRINT("seek to time: %u.  whence:%s, offset:%lld, start:%d, end:%d, cur:%d\n",
               u32SeekToTime, WHENCE_STRING(whence), offset,  u32StartFrmTime, u32EndFrmTime, u32CurFrmTime);

    return PVR_Index_SeekToTime(handle, u32SeekToTime);
}

/*****************************************************************************
 Prototype       : PVR_Index_SeekToStart
 Description     : move the read pointer of  index to the start frame. if recording, move it direction backward more 20 frames.
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/09/21
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_SeekToStart(PVR_INDEX_HANDLE handle)
{
    PVR_INDEX_LOCK(handle);
    handle->u32ReadFrame = handle->stCycMgr.u32StartFrame;

    if ((handle->bIsRec) && (handle->stCycMgr.u32StartFrame >= handle->stCycMgr.u32EndFrame))
    {
        PVRIndexCycMoveReadFrame(handle, PVR_TPLAY_MIN_DISTANCE);
        PVR_IDX_D("---move read frame+PVR_TPLAY_MIN_DISTANCE r=%d\n",handle->u32ReadFrame );
    }

    PVR_INDEX_UNLOCK(handle);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_SeekToEnd
 Description     : move the read pointer of index to the end frame
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/09/21
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_SeekToEnd(PVR_INDEX_HANDLE handle)
{
    MT_ASSERT_RET(handle != NULL);

    PVR_INDEX_LOCK(handle);
    MT_WARN_PVR("seek to end\n");

    handle->u32ReadFrame = handle->stCycMgr.u32EndFrame;
     if(handle->u32ReadFrame>0){
        handle->u32ReadFrame--;
    }else{
        if(handle->stCycMgr.u32LastFrame>0){
            handle->u32ReadFrame=(handle->stCycMgr.u32LastFrame-1);
        }
    }
    //printf("+++resumee:%x\n",handle->u32ReadFrame);
   PVR_INDEX_UNLOCK(handle);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_SeekToPauseOrStart
 Description     : on starting to play, seek the read pointer of index to the marked pause frame or start frame.
                        if exist paused frame, seek it to that.
                        if rewritten the pause frame, seek it to the start frame
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_SeekToPauseOrStart(PVR_INDEX_HANDLE handle)
{
    MT_S32 ret = MT_SUCCESS;
    PVR_INDEX_ENTRY_S startEntry;
    MT_U32 u32StartFrameNum;
    MT_U32 u32SeekToPos;

    MT_ASSERT_RET(handle != NULL);
	
	memset(&startEntry, 0, sizeof(PVR_INDEX_ENTRY_S));
	
    PVR_INDEX_LOCK(handle);

    /* have been paused, play from pause postion, if the positiong haven't been rewritten by rewind file*/
    if (handle->u64PauseOffset != PVR_INDEX_PAUSE_INVALID_OFFSET)
    {
        MT_ASSERT(handle->u64PauseOffset <= handle->u64GlobalOffset);

        /* straight recording, the start should move direction to the backward, hold some frame(about 20 frames), which prevent  from catching up the live */
        u32StartFrameNum = PVRIndexCalcNewPos(handle, handle->stCycMgr.u32StartFrame, 20);

        ret = PVRIndexGetEntryByNum(handle, &startEntry, u32StartFrameNum);
        if (ret != MT_SUCCESS)
        {
            MT_ERR_PVR("Can't get StartFrame:%d\n", handle->stCycMgr.u32StartFrame);
            PVR_INDEX_UNLOCK(handle);
            return ret;
        }

        /* check whether pause postion is rewritten or not, if the offset of start frame more than pause frame, it implies rewrite the pause frame */
        if (startEntry.u64GlobalOffset >= handle->u64PauseOffset)
        {
            /* on rewriting, seek it to the start frame */
            u32SeekToPos = u32StartFrameNum;
            MT_INFO_PVR("Pause frame was covered, so seek to start frame:%u\n", u32SeekToPos);
        }
        else
        {
            /* case, not rewrite pause frame, seek to pause frame */
            u32SeekToPos = handle->u32PauseFrame;
            MT_INFO_PVR("Seek to pause frame:%u\n", u32SeekToPos);
        }

        handle->u32ReadFrame = u32SeekToPos;
        handle->u64PauseOffset = PVR_INDEX_PAUSE_INVALID_OFFSET;
        handle->u32PauseFrame = 0;
    }

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_GetNextFrame
 Description     : get next frame for decode
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/28
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_GetNextFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame)
{
    MT_S32 ret;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pFrame != NULL);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetNextEntry(handle, pFrame);
    if (MT_SUCCESS != ret)
    {
        MT_INFO_PVR("get next entry error, file end.\n");
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }

    if (1 == pFrame->u16UpFlowFlag)
    {
        ret = PVRIndexGetNextIEntry(handle, pFrame);
        if (MT_SUCCESS != ret)
        {
            MT_INFO_PVR("get next I entry error, file end.\n");
            PVR_INDEX_UNLOCK(handle);
            return ret;
        }
    }

    memcpy(&(handle->stCurPlayFrame), pFrame, sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_GetNextIFrame
 Description     : get next I frame for decode
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_GetNextIFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame)
{
    MT_S32 ret;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pFrame != NULL);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetNextIEntry(handle, pFrame);
    if (MT_SUCCESS != ret)
    {
        MT_WARN_PVR("get next I entry error, file end.\n");
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }
    memcpy(&(handle->stCurPlayFrame), pFrame, sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}
/*****************************************************************************
 Prototype       : PVR_Index_GetNextIPFrame
 Description     : get next I or P frame for decode
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_GetNextIPFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame)
{
    MT_S32 ret;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pFrame != NULL);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetNextIPEntry(handle, pFrame);
    if (MT_SUCCESS != ret)
    {
        MT_WARN_PVR("get next I or P entry error, file end.\n");
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }
    memcpy(&(handle->stCurPlayFrame), pFrame, sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : PVR_Index_GetPreIFrame
 Description     : get pre I frame for decode
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Index_GetPreIFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame)
{
    MT_S32 ret;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pFrame != NULL);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetPreIEntry(handle, pFrame);
    if (MT_SUCCESS != ret)
    {
        MT_WARN_PVR("get pre I entry error, file end.\n");
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }
    memcpy(&(handle->stCurPlayFrame), pFrame, sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}
MT_S32 PVR_Index_GetPreXFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame)
{
    MT_S32 ret;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pFrame != NULL);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetPreXEntry(handle, pFrame);
    if (MT_SUCCESS != ret)
    {
        MT_WARN_PVR("get pre x entry error, file end.\n");
        PVR_INDEX_UNLOCK(handle);
        return ret;
    }
    memcpy(&(handle->stCurPlayFrame), pFrame, sizeof(PVR_INDEX_ENTRY_S));

    PVR_INDEX_UNLOCK(handle);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_GetCurrentFrame
 Description     : get the frame pointed to the current read pointer. not move the read pointer
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_GetCurrentFrame(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry)
{
    MT_S32 ret;
    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    PVR_INDEX_LOCK(handle);

    ret =  PVRIndexGetCurrentEntry(handle, pEntry);

    PVR_INDEX_UNLOCK(handle);
    return ret;
}


/*****************************************************************************
 Prototype       : PVR_Index_GetFrameByNum
 Description     : get the index by the num. but not move the read pointer
 Input           : handle         **
                   pFrame         **
                   num  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/

MT_S32 PVR_Index_GetFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num)
{
    MT_S32 ret;
    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    PVR_INDEX_LOCK(handle);

    ret = PVRIndexGetEntryByNum(handle,pEntry,num);

    PVR_INDEX_UNLOCK(handle);
    return ret;
}


/*get the pre i frame by the num. but not move the read pointer*/
static MT_S32 PVR_Index_GetPreIFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num)
{
	MT_S32 ret;
	MT_U32 u32NewPos;
	MT_U32 u32FrameDistance = 0;
    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);
    u32NewPos = num;
	while (1)
    {
        num = PVRIndexCalcNewPos(handle, u32NewPos, -1);
        if (num == u32NewPos)
        {
            return -1;
        }
        u32NewPos = num;
        if (PVRIndexIsFrameValid(handle,u32NewPos) != MT_TRUE)
        {
            return -1;
        }
        ret = PVRIndexGetEntryByNum(handle,pEntry,u32NewPos);
        if (ret != MT_SUCCESS)
        {
            return -1;
        }

        /* I frame, and not found the frame upflow flag*/
        if (PVR_INDEX_is_Iframe(pEntry) && !(pEntry->u16UpFlowFlag))
        {
            break;
        }
        u32FrameDistance++;
        
    }
    return (MT_S32)u32FrameDistance;
}


/*get the nearby i frame by the num. but not move the read pointer*/
MT_S32 PVR_Index_GetNearbyIFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num,MT_S32 direct)
{
	MT_S32 ret;
	MT_U32 u32NewPos;
	MT_U32 u32FrameDistance = 0;
    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);
    u32NewPos = num;
	while (1)
    {
        num = PVRIndexCalcNewPos(handle, u32NewPos, direct);
        if (num == u32NewPos)
        {
            return -1;
        }
        u32NewPos = num;
        if (PVRIndexIsFrameValid(handle,u32NewPos) != MT_TRUE)
        {
            return -1;
        }
        ret = PVRIndexGetEntryByNum(handle,pEntry,u32NewPos);
        if (ret != MT_SUCCESS)
        {
            return -1;
        }

        /* I frame, and not found the frame upflow flag*/
        if (PVR_INDEX_is_Iframe(pEntry) && !(pEntry->u16UpFlowFlag))
        {
            break;
        }
        u32FrameDistance++;
        
    }
    return (MT_S32)u32FrameDistance;
}



/*****************************************************************************
 Prototype       : PVR_Index_GetCurrentFrame
 Description     : get current read pointer,pointed to frame. but not move read pointer
 Input           : handle         **
                   pFrame         **
                   pDisplayTimes  **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_QueryFrameByPTS(const PVR_INDEX_HANDLE handle, MT_U32 u32SearchPTS, PVR_INDEX_ENTRY_S *pEntry, MT_U32 *pu32Pos, MT_U32 IsForword)
{
    MT_U32 u32PtsPos;
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    PVR_INDEX_LOCK(handle);

    u32PtsPos = PVRIndexFindFrameByPTS(handle, u32SearchPTS, handle->u32ReadFrame, IsForword);
    *pu32Pos =  u32PtsPos;
    MT_WARN_PVR("search PTS:%u, Pos:%u\n", u32SearchPTS, u32PtsPos);

    ret = PVRIndexGetEntryByNum(handle, pEntry, u32PtsPos);
    if (ret != MT_SUCCESS)
    {
        if (MT_ERR_PVR_FILE_TILL_END == ret)
        {
            if (MT_SUCCESS != PVRIndexGetEntryByNum(handle, pEntry, --u32PtsPos))
            {
                MT_ERR_PVR("Can't get Frame:%d\n", u32PtsPos);
                PVR_INDEX_UNLOCK(handle);
                return ret;
            }
        }
    }
    MT_WARN_PVR("Pos:%u, realPTS:%u, time:%u\n", u32PtsPos, pEntry->u32PtsMs, pEntry->u32DisplayTimeMs);

    PVR_INDEX_UNLOCK(handle);

    return MT_SUCCESS;
}


MT_S32 PVR_Index_QueryFrameByTime(const PVR_INDEX_HANDLE handle, MT_U32 u32SearchTime, PVR_INDEX_ENTRY_S *pEntry, MT_U32 *pu32Pos)
{
    MT_U32 u32PtsPos;
    MT_S32 ret;

    MT_ASSERT_RET(NULL != handle);
    MT_ASSERT_RET(NULL != pEntry);

    PVR_INDEX_LOCK(handle);

    u32PtsPos = PVRIndexFindFrameByTime(handle, u32SearchTime);
    *pu32Pos =  u32PtsPos;
    MT_WARN_PVR("search Time:%u, Pos:%u\n", u32SearchTime, u32PtsPos);

    ret = PVRIndexGetEntryByNum(handle, pEntry, u32PtsPos);
    if (ret != MT_SUCCESS)
    {
        if (MT_ERR_PVR_FILE_TILL_END == ret)
        {
            if (MT_SUCCESS != PVRIndexGetEntryByNum(handle, pEntry, --u32PtsPos))
            {
                MT_ERR_PVR("Can't get Frame:%d\n", u32PtsPos);
                PVR_INDEX_UNLOCK(handle);
                return ret;
            }
        }
    }
    MT_WARN_PVR("Pos:%u, time:%u\n", u32PtsPos, pEntry->u32DisplayTimeMs);

    PVR_INDEX_UNLOCK(handle);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVR_Index_MarkPausePos
 Description     : mark a flag for timeshift, where flag current record position. if start timeshift, play it from this position
 Input           : handle         **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_MarkPausePos(PVR_INDEX_HANDLE handle)
{
    /* save the frame number of current recording frame*/
    handle->u32PauseFrame = handle->u32WriteFrame;

    /* save the absolute offset for the current frame, used for checking whether the pause position rewrote or not by rewind, on playing */
    handle->u64PauseOffset = handle->u64GlobalOffset;

    MT_WARN_PVR("<<==PVR_Index_MarkPausePos: frame=%d, global offset=%lld.\n",
                handle->u32PauseFrame, handle->u64PauseOffset);
    return MT_SUCCESS;

}

MT_VOID PVR_Index_GetIdxFileName(MT_CHAR* pIdxFileName, MT_CHAR* pSrcFileName)
{
    MT_CHAR* pSearch = NULL;
    MT_CHAR* pAppend = NULL;

    pSearch = strstr(pSrcFileName, ".idx");
    while(NULL != pSearch)
    {
        if (NULL != pSearch)
        {
            pAppend = pSearch;
            pSearch = strstr(pSearch + 1, ".idx");
        }
    }

    /* make sure it end with the .idx */
    if (NULL != pAppend && *(pAppend + 4) == 0)
    {
        strncpy(pIdxFileName, pSrcFileName,strlen(pSrcFileName)+1);
    }
    else
    {
        snprintf(pIdxFileName, PVR_MAX_FILENAME_LEN,"%s.idx", pSrcFileName);
    }
    return;
}


/*****************************************************************************
 Prototype       : PVR_Index_PlayGetFileAttrByFileName
 Description     : get the current state of the record file by file name. and can't get it without including index file.
 Input           : handle         **
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function
*****************************************************************************/
MT_S32 PVR_Index_PlayGetFileAttrByFileName(const MT_CHAR *pFileName, PVR_INDEX_HANDLE pIdxHandle, MT_UNF_PVR_FILE_ATTR_S *pAttr)
{
    MT_S32            fdIdx;
    MT_CHAR           szIndexName[PVR_MAX_FILENAME_LEN + 5] = {0};
    MT_S32            readNum;
    PVR_INDEX_ENTRY_S startEntry = {0};
    PVR_INDEX_ENTRY_S endEntry = {0};
    PVR_INDEX_ENTRY_S lastEntry = {0};
    MT_S32            s32Offset;
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo = {0};
    MT_BOOL bRetry = MT_FALSE;
    MT_U32  idxsize=sizeof(PVR_INDEX_ENTRY_S),readlen=0;
    if (!pFileName)
    {
        return MT_ERR_PVR_NUL_PTR;
    }
    if (!pAttr)
    {
        return MT_ERR_PVR_NUL_PTR;
    }

    memset(pAttr,0x0,sizeof(MT_UNF_PVR_FILE_ATTR_S));
    #if 0
    if (MT_NULL != pIdxHandle)
    {
		do
		{
	        if (MT_SUCCESS != PVRIndexGetEntryByNum(pIdxHandle, &startEntry, pIdxHandle->stCycMgr.u32StartFrame))
	        {
                MT_WARN_PVR("read start_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
							pIdxHandle->stCycMgr.u32StartFrame, 
					        pIdxHandle->stCycMgr.u32EndFrame,
					        pIdxHandle->stCycMgr.u32LastFrame);
	            bRetry = MT_TRUE;
				break;
	        }

	        if (MT_SUCCESS != PVRIndexGetEntryByNum(pIdxHandle, &endEntry, (pIdxHandle->stCycMgr.u32EndFrame - 1)))
	        {
            	MT_WARN_PVR("read end_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
						pIdxHandle->stCycMgr.u32StartFrame, 
				        pIdxHandle->stCycMgr.u32EndFrame,
				        pIdxHandle->stCycMgr.u32LastFrame);
	            bRetry = MT_TRUE;
				break;
	        }

	        if (MT_SUCCESS != PVRIndexGetEntryByNum(pIdxHandle, &lastEntry, (pIdxHandle->stCycMgr.u32LastFrame - 1)))
	        {
            	MT_WARN_PVR("read last_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
						pIdxHandle->stCycMgr.u32StartFrame, 
				        pIdxHandle->stCycMgr.u32EndFrame,
				        pIdxHandle->stCycMgr.u32LastFrame);
	            bRetry = MT_TRUE;
				break;
	        }

	        if (pIdxHandle->stCycMgr.u32EndFrame > pIdxHandle->stCycMgr.u32StartFrame)
	        {
	            pAttr->u32FrameNum = pIdxHandle->stCycMgr.u32EndFrame
	                                 - pIdxHandle->stCycMgr.u32StartFrame;
	        }
	        else
	        {
	            pAttr->u32FrameNum = pIdxHandle->stCycMgr.u32LastFrame
	                                 - pIdxHandle->stCycMgr.u32StartFrame
	                                 + pIdxHandle->stCycMgr.u32EndFrame;
	        }
	        pAttr->u32StartTimeInMs = startEntry.u32DisplayTimeMs;
	        pAttr->u32EndTimeInMs = (0 == pIdxHandle->stCycMgr.u32EndFrame) ? lastEntry.u32DisplayTimeMs : endEntry.u32DisplayTimeMs;
	        pAttr->u64ValidSizeInByte = lastEntry.u64Offset + lastEntry.u32FrameSize;
	        pAttr->enIdxType = (MT_UNF_PVR_REC_INDEX_TYPE_E)(startEntry.u16IndexType);
	        pAttr->u64CurWPos_Glb = lastEntry.u64GlobalOffset + lastEntry.u32FrameSize;
		}while(0);
 
    }
    #else
    if (MT_NULL != pIdxHandle)
    {
        if(0 != pIdxHandle->stCycMgr.u32LastFrame){
            do
            {
                if(pIdxHandle->u32StartFrame_dread != pIdxHandle->stCycMgr.u32StartFrame){  //read and cache
                    //printf("++read start .file\n");
                    readlen=PVR_READ((void*)&startEntry, idxsize , pIdxHandle->s32Readdirect_Fd, pIdxHandle->stCycMgr.u32StartFrame*idxsize+pIdxHandle->u32IdxStartOffsetLen);
                    if (readlen != idxsize){
                        MT_WARN_PVR("read start_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
                          pIdxHandle->stCycMgr.u32StartFrame, 
                          pIdxHandle->stCycMgr.u32EndFrame,
                          pIdxHandle->stCycMgr.u32LastFrame);
                          return MT_ERR_PVR_FILE_CANT_READ;
                    }
                    pIdxHandle->u32StartFrame_dread = pIdxHandle->stCycMgr.u32StartFrame;
                    memcpy(&pIdxHandle->startEntry,&startEntry,idxsize);
                }else{  //copy from cache. if not change
                    memcpy(&startEntry,&pIdxHandle->startEntry,idxsize);
                    //printf("++read start .cache\n");
                }

                if(pIdxHandle->bIsRec){   //copy from mem
                    memcpy(&endEntry,&pIdxHandle->stCurRecFrame,idxsize);
                    //printf("++read end .rec\n");
                }else{
                    if(pIdxHandle->u32EndFrame_dread != pIdxHandle->stCycMgr.u32EndFrame){  //read and cache
                        //printf("++read end .file\n");
                        readlen=PVR_READ((void*)&endEntry, idxsize , pIdxHandle->s32Readdirect_Fd, (pIdxHandle->stCycMgr.u32EndFrame - 1)*idxsize+pIdxHandle->u32IdxStartOffsetLen);
                        if(readlen != idxsize){
                          MT_WARN_PVR("read end_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
                            pIdxHandle->stCycMgr.u32StartFrame, 
                            pIdxHandle->stCycMgr.u32EndFrame,
                            pIdxHandle->stCycMgr.u32LastFrame);
                            return MT_ERR_PVR_FILE_CANT_READ;
                        }
                        pIdxHandle->u32EndFrame_dread = pIdxHandle->stCycMgr.u32EndFrame;
                        memcpy(&pIdxHandle->endEntry,&endEntry,idxsize);
                    }else{  //copy from cache. if not change
                        memcpy(&endEntry,&pIdxHandle->endEntry,idxsize);
                        //printf("++read end .cache\n");
                    }
                }

                if(pIdxHandle->stCycMgr.u32EndFrame != pIdxHandle->stCycMgr.u32LastFrame){
                    if(pIdxHandle->u32LastFrame_dread != pIdxHandle->stCycMgr.u32LastFrame){  //read and cache
                        //printf("++read lst .file\n");
                        readlen=PVR_READ((void*)&lastEntry, idxsize , pIdxHandle->s32Readdirect_Fd, (pIdxHandle->stCycMgr.u32LastFrame - 1)*idxsize+pIdxHandle->u32IdxStartOffsetLen);
                        if(readlen != idxsize){
                          MT_WARN_PVR("read end_idx failed, start frm:%d end frm:%d last frm:%d.\n", 
                            pIdxHandle->stCycMgr.u32StartFrame, 
                            pIdxHandle->stCycMgr.u32EndFrame,
                            pIdxHandle->stCycMgr.u32LastFrame);
                            return MT_ERR_PVR_FILE_CANT_READ;
                        }
                        pIdxHandle->u32LastFrame_dread = pIdxHandle->stCycMgr.u32LastFrame;
                        memcpy(&pIdxHandle->lastEntry,&lastEntry,idxsize);
                    }else{  //copy from cache. if not change
                        memcpy(&lastEntry,&pIdxHandle->lastEntry,idxsize);
                        //printf("++read lst .cache\n");
                    }
                }else{      //copy from end,if end=lst
                    memcpy(&lastEntry,&endEntry,idxsize);
                    //printf("++read lst .end\n");
                }

                if (pIdxHandle->stCycMgr.u32EndFrame > pIdxHandle->stCycMgr.u32StartFrame){
                    pAttr->u32FrameNum = pIdxHandle->stCycMgr.u32EndFrame
                                         - pIdxHandle->stCycMgr.u32StartFrame;
                }else{
                    pAttr->u32FrameNum = pIdxHandle->stCycMgr.u32LastFrame
                                         - pIdxHandle->stCycMgr.u32StartFrame
                                         + pIdxHandle->stCycMgr.u32EndFrame;
                }
                pAttr->u32StartTimeInMs = startEntry.u32DisplayTimeMs;
                pAttr->u32EndTimeInMs = (0 == pIdxHandle->stCycMgr.u32EndFrame) ? lastEntry.u32DisplayTimeMs : endEntry.u32DisplayTimeMs;
                pAttr->u64ValidSizeInByte = lastEntry.u64Offset + lastEntry.u32FrameSize;
                pAttr->enIdxType = (MT_UNF_PVR_REC_INDEX_TYPE_E)(startEntry.u16IndexType);
                pAttr->u64CurWPos_Glb = lastEntry.u64GlobalOffset + lastEntry.u32FrameSize;
            }while(0);
        }//else use defalut
    }
    #endif
    /*Add condition bRetry ,As some times read index entry may failed ,
    such as rec just start.there is no one index entry.
    we should open the index file again and retry. for DTS2014042105555*/
    if((MT_NULL == pIdxHandle) || bRetry)
    {
        PVR_Index_GetIdxFileName(szIndexName, (MT_CHAR*)pFileName);
        fdIdx = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_DATA_READ);
        if (fdIdx < 0)
        {
            MT_ERR_PVR("can not open index file:%s\n", szIndexName);
            return MT_ERR_PVR_FILE_CANT_OPEN;
        }

        if (MT_SUCCESS != PVRIndexGetHeaderInfo(fdIdx, &stIdxHeaderInfo))
        {
            MT_ERR_PVR("No Header Info in index File:%s\n", szIndexName);
            PVR_CLOSE(fdIdx);
            return MT_ERR_PVR_INDEX_FORMAT_ERR;
        }

        if (((0 == stIdxHeaderInfo.stCycInfo.u32StartFrame)
                && (0 == stIdxHeaderInfo.stCycInfo.u32EndFrame))
            || (0 == stIdxHeaderInfo.stCycInfo.u32LastFrame))
        {
            MT_WARN_PVR("No frame in index File:%s\n", szIndexName);
            pAttr->u32FrameNum = 0;
            pAttr->u32StartTimeInMs = 0;
            pAttr->u32EndTimeInMs = 0;
            pAttr->u64ValidSizeInByte = 0;
            pAttr->enIdxType = MT_UNF_PVR_REC_INDEX_TYPE_NONE;

            PVR_CLOSE(fdIdx);
            return MT_SUCCESS;
        }

        /* deal it with the same mode, regardless of rewind of the index file */

        /* read the start frame info */
        s32Offset = (MT_S32)(stIdxHeaderInfo.u32HeaderLen + sizeof(PVR_INDEX_ENTRY_S) * stIdxHeaderInfo.stCycInfo.u32StartFrame);
        readNum = PVR_READ(&startEntry, sizeof(PVR_INDEX_ENTRY_S), fdIdx, s32Offset);
        if (readNum != (MT_S32)sizeof(PVR_INDEX_ENTRY_S))
        {
            MT_ERR_PVR("read start_idx failed, offset:%d.\n", s32Offset);
            MT_ERR_PVR("The index file is too small, can't play.\n");
            PVR_CLOSE(fdIdx);
            return MT_ERR_PVR_FILE_CANT_READ;
        }

        /* read the end frame info */
        s32Offset = (MT_S32)(stIdxHeaderInfo.u32HeaderLen + sizeof(PVR_INDEX_ENTRY_S) *
                    (stIdxHeaderInfo.stCycInfo.u32EndFrame -1));
        readNum = PVR_READ(&endEntry, sizeof(PVR_INDEX_ENTRY_S), fdIdx, s32Offset);
        if (readNum != (MT_S32)sizeof(PVR_INDEX_ENTRY_S))
        {
            MT_ERR_PVR("read end_idx failed, endframe:%d, offset:%d.\n",stIdxHeaderInfo.stCycInfo.u32EndFrame, s32Offset);
            PVR_CLOSE(fdIdx);
            return MT_ERR_PVR_FILE_CANT_READ;
        }

        /* read the last frame info */
        s32Offset = (MT_S32)(stIdxHeaderInfo.u32HeaderLen + sizeof(PVR_INDEX_ENTRY_S) *
                    (stIdxHeaderInfo.stCycInfo.u32LastFrame - 1));
        readNum = PVR_READ(&lastEntry, sizeof(PVR_INDEX_ENTRY_S), fdIdx, s32Offset);
        if (readNum !=(MT_S32)sizeof(PVR_INDEX_ENTRY_S))
        {
            MT_ERR_PVR("read last_idx failed, lastFrame:%d, offset:%d.\n",stIdxHeaderInfo.stCycInfo.u32LastFrame, s32Offset);
            PVR_CLOSE(fdIdx);
            return MT_ERR_PVR_FILE_CANT_READ;
        }

        if (stIdxHeaderInfo.stCycInfo.u32EndFrame > stIdxHeaderInfo.stCycInfo.u32StartFrame)
        {
            pAttr->u32FrameNum = stIdxHeaderInfo.stCycInfo.u32EndFrame
                                 - stIdxHeaderInfo.stCycInfo.u32StartFrame;
        }
        else
        {
            pAttr->u32FrameNum = stIdxHeaderInfo.stCycInfo.u32LastFrame
                                 - stIdxHeaderInfo.stCycInfo.u32StartFrame
                                 + stIdxHeaderInfo.stCycInfo.u32EndFrame;
        }
        pAttr->u32StartTimeInMs = startEntry.u32DisplayTimeMs;
        pAttr->u32EndTimeInMs = (0 == stIdxHeaderInfo.stCycInfo.u32EndFrame) ? lastEntry.u32DisplayTimeMs : endEntry.u32DisplayTimeMs;
        pAttr->u64ValidSizeInByte = lastEntry.u64Offset + lastEntry.u32FrameSize;
        pAttr->enIdxType = (MT_UNF_PVR_REC_INDEX_TYPE_E)(startEntry.u16IndexType);
        pAttr->u64CurWPos_Glb = lastEntry.u64GlobalOffset + lastEntry.u32FrameSize;

        PVR_CLOSE(fdIdx);
    }

    return MT_SUCCESS;
}

MT_S32 PVR_Index_GetFrmNumByEntry(PVR_INDEX_HANDLE pstIndexHandle, PTR_PVR_INDEX_ENTRY pstIndexEntry, MT_S32 *ps32FrmNum)
{
    MT_S32 s32Ret = 0;
    MT_U32 u32StartFrmNum = 0;
    MT_U32 u32EndFrmNum = 0;
    MT_U32 u32LastFrmNum = 0;
    MT_U32 u32MidFrmNum = 0;

    PVR_INDEX_ENTRY_S stStartIndexEntry = {0};
    PVR_INDEX_ENTRY_S stEndIndexEntry = {0};
    PVR_INDEX_ENTRY_S stMidIndexEntry = {0};
    PVR_INDEX_ENTRY_S stZeroIndexEntry = {0};
    PVR_INDEX_ENTRY_S stLastIndexEntry = {0};

    u32StartFrmNum = pstIndexHandle->stCycMgr.u32StartFrame;
    u32EndFrmNum = pstIndexHandle->stCycMgr.u32EndFrame;
    u32LastFrmNum = pstIndexHandle->stCycMgr.u32LastFrame;

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stStartIndexEntry, u32StartFrmNum))
    {
        MT_ERR_PVR("get the %d entry fail.\n", u32StartFrmNum);
        return MT_FAILURE;
    }
    
    if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stZeroIndexEntry, 0))
    {
        MT_ERR_PVR("get the %d entry fail.\n", 0);
        return MT_FAILURE;
    }

    s32Ret = PVR_Index_GetFrameByNum(pstIndexHandle, &stLastIndexEntry, u32LastFrmNum);

    if(MT_SUCCESS != s32Ret)
    {
        if (MT_ERR_PVR_FILE_TILL_END == s32Ret)
        {
            s32Ret = PVR_Index_GetFrameByNum(pstIndexHandle, &stLastIndexEntry, --u32LastFrmNum);
            if (MT_SUCCESS != s32Ret)
            {
                MT_ERR_PVR("get the %d entry fail.\n", u32LastFrmNum);
                return MT_FAILURE;
            }
        }
        else
        {
            MT_ERR_PVR("get the %d entry fail.\n", u32LastFrmNum);
            return MT_FAILURE;
        }
    }

    s32Ret = PVR_Index_GetFrameByNum(pstIndexHandle, &stEndIndexEntry, u32EndFrmNum);

    if((MT_ERR_PVR_FILE_TILL_END == s32Ret) ||
       (stEndIndexEntry.u64GlobalOffset < stStartIndexEntry.u64GlobalOffset))
    {
        s32Ret = PVR_Index_GetFrameByNum(pstIndexHandle, &stEndIndexEntry, --u32EndFrmNum);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_PVR("get the %d entry fail.\n", u32EndFrmNum);
            return MT_FAILURE;
        }
    }
    else if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_PVR("get the %d entry fail.\n", u32LastFrmNum);
        return MT_FAILURE;
    }

    if (u32EndFrmNum <= u32StartFrmNum)
    {
        if((pstIndexEntry->u64GlobalOffset >= stStartIndexEntry.u64GlobalOffset) &&
          (pstIndexEntry->u64GlobalOffset <= stLastIndexEntry.u64GlobalOffset))
        {
            u32MidFrmNum = (u32LastFrmNum - u32StartFrmNum)/2;
            u32EndFrmNum = u32LastFrmNum;
            
            if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stEndIndexEntry, u32EndFrmNum))
            {
                MT_ERR_PVR("get the %d entry fail.\n", u32EndFrmNum);
                return MT_FAILURE;
            }
        }
        else if((pstIndexEntry->u64GlobalOffset <= stEndIndexEntry.u64GlobalOffset) &&
               (pstIndexEntry->u64GlobalOffset >= stZeroIndexEntry.u64GlobalOffset))
        {
            u32MidFrmNum = u32EndFrmNum/2;
            u32StartFrmNum = 0;
            
            if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stStartIndexEntry, u32StartFrmNum))
            {
                MT_ERR_PVR("get the %d entry fail.\n", u32StartFrmNum);
                return MT_FAILURE;
            }
        }
        else
        {
            MT_ERR_PVR("invalid entry offset=%#llx zero(0)ffset=%#llx start(%d)offset=%#llx end(%d)offset=%#llx last(%d)offset=%#llx.\n",
                pstIndexEntry->u64GlobalOffset,stZeroIndexEntry.u64GlobalOffset,
                u32StartFrmNum, stStartIndexEntry.u64GlobalOffset,
                u32EndFrmNum, stEndIndexEntry.u64GlobalOffset,
                u32LastFrmNum, stLastIndexEntry.u64GlobalOffset);
            return MT_FAILURE;
        }
    }
    else
    {
        u32MidFrmNum = u32EndFrmNum/2;
    }

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stMidIndexEntry, u32MidFrmNum))
    {
        MT_ERR_PVR("get the %d entry fail.\n", u32MidFrmNum);
        return MT_FAILURE;
    }
	
    while((MT_S32)u32StartFrmNum <= (MT_S32)u32EndFrmNum)
    {
        u32MidFrmNum = u32StartFrmNum + (u32EndFrmNum - u32StartFrmNum)/2;

        if(MT_SUCCESS != PVR_Index_GetFrameByNum(pstIndexHandle, &stMidIndexEntry, u32MidFrmNum))
        {
            MT_ERR_PVR("get the %d entry fail.\n", u32MidFrmNum);
            return MT_FAILURE;
        }
        
        if(stMidIndexEntry.u64GlobalOffset > pstIndexEntry->u64GlobalOffset)
		{
			u32EndFrmNum = u32MidFrmNum - 1;
		}
        else if(stMidIndexEntry.u64GlobalOffset < pstIndexEntry->u64GlobalOffset)
		{
			u32StartFrmNum = u32MidFrmNum + 1;
		}
		else
		{
            if (MT_TRUE == PVRIndexIsFrameValid(pstIndexHandle, u32MidFrmNum))
            {
                *ps32FrmNum = (MT_S32)u32MidFrmNum;
                return MT_SUCCESS;
            }
            else
            {
                MT_ERR_PVR("find invalid frame number %d(start=%d end=%d) from entry offset %#llx pts %d.\n", 
                    u32MidFrmNum, u32StartFrmNum, u32EndFrmNum, pstIndexEntry->u64GlobalOffset,pstIndexEntry->u32PtsMs);
                return MT_FAILURE;
            }
        }
    }
    MT_ERR_PVR("can not find frame number from entry offset %#llx pts %d start=%d end=%d mid=%d. \n", 
               pstIndexEntry->u64GlobalOffset,pstIndexEntry->u32PtsMs, u32StartFrmNum, u32EndFrmNum, u32MidFrmNum);
	return MT_FAILURE;
}


/*get vedio type from index header info   */
MT_S32 PVR_Index_GetVtype(PVR_INDEX_HANDLE handle)
{
    PVR_IDX_HEADER_INFO_S stIdxHeaderInfo;

    memset(&stIdxHeaderInfo, 0, sizeof(PVR_IDX_HEADER_INFO_S));

	/* means not found the header info in this index file */
    if (MT_SUCCESS != PVRIndexGetHeaderInfo(handle->s32HeaderFd, &stIdxHeaderInfo))
    {
        return MT_FAILURE;
    }
    else
        return (stIdxHeaderInfo.u32Reserved & 0xFFFF);
}
 
/* unused.warning
MT_S32 PVR_Index_GetFBwardAttr(PVR_PLAY_CHN_S *pChnAttr, MT_PVR_FAST_FORWARD_BACKWARD_S *FBwardAttr)
{
    MT_UNF_AVPLAY_STREAM_INFO_S stStreamInfo;
    MT_SYS_VERSION_S SysVer;
    MT_U32 u32VideoType = 0;

    memset(&SysVer, 0, sizeof(MT_SYS_VERSION_S));
    
    if (MT_SUCCESS != MT_SYS_GetVersion(&SysVer))
    {
        MT_FATAL_PVR("Cannot get system version\n");
        return MT_FAILURE;
    }

    // get vedio type, which is recored in index headinfo
    u32VideoType = PVR_Index_GetVtype(pChnAttr->IndexHandle);
    if (MT_FAILURE == (MT_S32)u32VideoType)
    {
        MT_FATAL_PVR("Cannot get video type\n");
        return MT_FAILURE;
    }

    if (MT_SUCCESS != MT_UNF_AVPLAY_GetStreamInfo(pChnAttr->hAvplay, &stStreamInfo))
    {
        MT_FATAL_PVR("Cannot get stream info\n");
        return MT_FAILURE;
    }

    FBwardAttr->enSpeed = pChnAttr->enSpeed;
    FBwardAttr->enVideoType = (MT_UNF_VCODEC_TYPE_E)(u32VideoType - 100);
    FBwardAttr->enChipID = SysVer.enChipTypeHardWare;
    FBwardAttr->enChipVer = SysVer.enChipVersion;
    FBwardAttr->u32Width = stStreamInfo.stVidStreamInfo.u32Width;
    FBwardAttr->u32Height = stStreamInfo.stVidStreamInfo.u32Height;

    return MT_SUCCESS;

}*/

/*reserved function for smooth play */
MT_S32 PVR_Index_GetMaxBitrate(PVR_INDEX_HANDLE piIndexHandle)
{
    return 0x100;
}

/*get stream bit rate  */
MT_S32 PVR_Index_GetStreamBitRate(PVR_INDEX_HANDLE piIndexHandle,
                                      MT_U32 *pBitRate,
                                      MT_U32 u32StartFrameNum, 
                                      MT_U32 u32EndFrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_U32 i = 0, u32StartTime = 0, u32EndTime = 0;
    MT_U64 u64TotalBytes = 0;

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartFrameNum))
    {
        MT_ERR_PVR("input start frame number is invalid.\n");
        return MT_FAILURE;
    }

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32EndFrameNum))
    {
        MT_ERR_PVR("input end frame number is invalid.\n");
        return MT_FAILURE;
    }

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32StartFrameNum))
    {
         MT_INFO_PVR("get the %d entry fail.\n", i);
         return MT_FAILURE;
    }
    u32StartTime = frame_tmp.u32DisplayTimeMs;

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32EndFrameNum))
    {
         MT_INFO_PVR("get the %d entry fail.\n", i);
         return MT_FAILURE;
    }
    u32EndTime = frame_tmp.u32DisplayTimeMs;

    if((u32StartTime == u32EndTime) || (u32StartTime >= u32EndTime))
    {
        MT_INFO_PVR("invalid pts, can not get bitrate.\n");
        return MT_FAILURE;
    }

    /* TODO: use global offset to calc total bytes */
    if (u32EndFrameNum < u32StartFrameNum)
    {
        for (i = u32StartFrameNum; i < piIndexHandle->stCycMgr.u32LastFrame; i++)
        {
            if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, i))
            {
                 MT_INFO_PVR("get the %d entry fail.\n", i);
                 return MT_FAILURE;
            }
            
            u64TotalBytes += frame_tmp.u32FrameSize;
        }

        for (i = 0; i < u32EndFrameNum; i++)
        {
            if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, i))
            {
                 MT_INFO_PVR("get the %d entry fail.\n", i);
                 return MT_FAILURE;
            }
            
            u64TotalBytes += frame_tmp.u32FrameSize;
        }
    }
    else
    {
        for (i = u32StartFrameNum; i < u32EndFrameNum; i++)
        {
            if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, i))
            {
                 MT_INFO_PVR("get the %d entry fail.\n", i);
                 return MT_FAILURE;
            }
            
            u64TotalBytes += frame_tmp.u32FrameSize;
        }
    }

   *pBitRate =  (MT_U32)((u64TotalBytes*8)/((u32EndTime - u32StartTime)/1000));
    
    return MT_SUCCESS;
}
#if 0
MT_S32 PVR_Index_GetFrameRate(PVR_INDEX_HANDLE piIndexHandle, MT_U32 *pFrameRate)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    //PVR_INDEX_ENTRY_S  frame_tmp2 ={0}; 
    MT_S32 s32Ret = 0;
    //MT_U32 u32StartTime = 0, u32EndTime = 0;
    MT_U32 u32TotalFrames = 0;
    MT_U32 u32StartFrmNum = 0;
    MT_U32 u32EndFrmNum = 0;
    MT_U32 u32LastFrmNum = 0;
    //MT_U32 u32TotalTimeMs = 0;

    MT_ASSERT_RET(piIndexHandle != NULL);
    //start=loopbuf.start; end=loopbuf.end; last=loopbuf.size
    MT_U32 counter_frames=120;              //because pts maybe reloop,if not live!
    u32StartFrmNum = piIndexHandle->stCycMgr.u32StartFrame;
    u32EndFrmNum = piIndexHandle->stCycMgr.u32EndFrame;
    u32LastFrmNum = piIndexHandle->stCycMgr.u32LastFrame;
    
    if (u32EndFrmNum < u32StartFrmNum){
        u32TotalFrames = (u32LastFrmNum - u32StartFrmNum) + u32EndFrmNum + 1;
    }else{
        u32TotalFrames = (u32EndFrmNum - u32StartFrmNum)+1;
    } 
    MT_INFO_PVR("f0.start=%d,end=%d,tail=%d,totalframe=%d\n",u32StartFrmNum,u32EndFrmNum,u32LastFrmNum,u32TotalFrames);
    if(u32TotalFrames<counter_frames){      //give default value
        *pFrameRate = 25;
        MT_WARN_PVR("\n[%s_%d]ERROR!!! get default fps = 25fps\n",__func__,__LINE__);
    }else{                                  //calculate
        int i; 
        MT_U32 start_pts = 0xffffffff;
        MT_U32 end_pts = 0;
        int total_fps_frames = 0;
        PVR_INDEX_ENTRY_S first_ftype={0};
        PVR_INDEX_ENTRY_S last_ftype={0};
        for(i=0; i<(mt_s32)counter_frames; i++){
            s32Ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, (u32StartFrmNum + (MT_U32)i)%u32LastFrmNum);
            if(MT_SUCCESS!=s32Ret){
                *pFrameRate=25;
                MT_INFO_PVR("read frame error1\n");
                return MT_SUCCESS;
            }
            if(i == 0)
                first_ftype = frame_tmp;
            if(i == (int)(counter_frames - 1))
                last_ftype = frame_tmp;
            if(frame_tmp.u32PtsMs < start_pts)
                start_pts = frame_tmp.u32PtsMs;
            if(frame_tmp.u32PtsMs > end_pts)
                end_pts = frame_tmp.u32PtsMs;
        }
        //add I or P frame before the current B frame, the pts of the I or P frame is larger than the first frame
        if(PVR_INDEX_is_Bframe(&first_ftype)){
            total_fps_frames = (int)(counter_frames + 1);
        }else{
            total_fps_frames = (int)(counter_frames);
        }
#if 0        
        if(PVR_INDEX_is_Bframe(&last_ftype)){
            total_fps_frames--;
        }else if(PVR_INDEX_is_Iframe(&last_ftype) || PVR_INDEX_is_Pframe(&last_ftype))
#endif        
        //add B frames after the last frame, the pts of the B frames are less than end_pts
        {
            for(;;){
              i++;
              if(i >= (mt_s32)u32TotalFrames)
                  break;
              s32Ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, (u32StartFrmNum + (mt_u32)i)%u32LastFrmNum);
              if(MT_SUCCESS!=s32Ret){
                  MT_INFO_PVR("read frame error2\n");
                  return MT_SUCCESS;
              }
              if(PVR_INDEX_is_Bframe(&frame_tmp) == 0){
                  break;
              }
              total_fps_frames++;
            }
        }
        if(start_pts < end_pts){
            *pFrameRate = ((mt_u32)total_fps_frames*10000 / (end_pts - start_pts) + 5)/10;
        }else{
            *pFrameRate = 25;
            MT_INFO_PVR("\n[%s_%d]ERROR!!! get default fps = 25fps\n",__func__,__LINE__);
        }
        if(0==*pFrameRate){
            *pFrameRate=25;
        }
        MT_INFO_PVR("[%s_%d]totalf=%d,spts=%d,epts=%d, fps=%d\n",__func__,__LINE__,total_fps_frames,start_pts,end_pts, *pFrameRate);
    }
    MT_INFO_PVR("\r\n ^^^^^^^^^^^%s,%s,pFrameRate:%d\n", __FILE__,__FUNCTION__,*pFrameRate);
    return MT_SUCCESS;
}
#else
MT_S32 PVR_Index_GetFrameRate(PVR_INDEX_HANDLE piIndexHandle, MT_U32 *pFrameRate)
{
    PVR_INDEX_ENTRY_S  frame_s ={0}; 
    PVR_INDEX_ENTRY_S  frame_e ={0}; 
    MT_S32 s32Ret = 0;
    MT_U32 u32TotalFrames = 0;

    MT_ASSERT_RET(piIndexHandle != NULL);
    MT_U32 s = piIndexHandle->stCycMgr.u32StartFrame;
    MT_U32 e = piIndexHandle->stCycMgr.u32EndFrame;
    MT_U32 l = piIndexHandle->stCycMgr.u32LastFrame;
    if(l>0){
        if(e>0){
            e--;
        }
        if(l>0){
            l--;
        }
        if (e < s){
            u32TotalFrames = (l - s) + e+1;
        }else{
            u32TotalFrames = (e - s)+1;
        } 
        if(u32TotalFrames<150){
            *pFrameRate = 25;
            PVR_ALAWYS_PRINT("\n[%s_%d]ERROR!!! get default0 fps = 25fps\n",__func__,__LINE__);
        }else{
            s32Ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_s, s);
            s32Ret |= PVR_Index_GetFrameByNum(piIndexHandle, &frame_e, e);
            if(s32Ret){
                *pFrameRate = 25;
                PVR_ALAWYS_PRINT("\n[%s_%d]ERROR!!! get default0 fps = 25fps\n",__func__,__LINE__);
            }else{
                if(frame_e.u32DisplayTimeMs>frame_s.u32DisplayTimeMs){
                    float total_frame = (float)u32TotalFrames;
                    float total_time  = (float)(frame_e.u32DisplayTimeMs-frame_s.u32DisplayTimeMs)/1000;
                    
                    *pFrameRate=(int)((total_frame/total_time)+0.5f);
                    PVR_ALAWYS_PRINT("s=%u e=%u l=%u totalframes=%u st=%u et=%u total_frame=%f total_time=%f *pFrameRate=%d\n",
                        s,e,l,u32TotalFrames,frame_s.u32DisplayTimeMs,frame_e.u32DisplayTimeMs,total_frame,total_time,*pFrameRate);
                }else{
                    *pFrameRate = 25;
                    PVR_ALAWYS_PRINT("\n[%s_%d]ERROR!!! get default2 fps = 25fps\n",__func__,__LINE__);
                }
            }
        }
    }

    if(*pFrameRate == 0){
        *pFrameRate = 25;
        PVR_ALAWYS_PRINT("\n[%s_%d]ERROR!!! get default3 fps = 25fps\n",__func__,__LINE__);
    }
    PVR_ALAWYS_PRINT("\r\n ^^^^^^^^^^^%s,%s,pFrameRate:%d\n", __FILE__,__FUNCTION__,*pFrameRate);
    return MT_SUCCESS;
}
#endif

/* get pre I/P/B frame number
   0 == u32Direction BACKWARD
   1 == u32Direction FORWARD*/
MT_S32 PVR_Index_GetFBwardIPBFrameNum(PVR_INDEX_HANDLE handle, MT_U32 u32Direction, MT_U32 u32FrameType, MT_U32 u32CurFrameNum, MT_U32 *pu32NextFrameNum)
{
    MT_S32 s32NextFrameNum = (MT_S32)u32CurFrameNum;
    PVR_INDEX_ENTRY_S pFrame; 
  	MT_S32 ret=0;
    MT_U32 looptimes=0;

    MT_ASSERT_RET(handle != NULL);
    MT_ASSERT_RET(pu32NextFrameNum != NULL);

    memset(&pFrame, 0, sizeof(PVR_INDEX_ENTRY_S));
    
	  if((handle->stCycMgr.u32StartFrame > handle->stCycMgr.u32LastFrame)||
       (handle->stCycMgr.u32EndFrame > handle->stCycMgr.u32LastFrame)){
        MT_ERR_PVR("bad index,s=%d,e=%d,l=%d\n",handle->stCycMgr.u32StartFrame,
				    handle->stCycMgr.u32EndFrame,
				    handle->stCycMgr.u32LastFrame);
        return MT_FAILURE;
    }
    if (MT_TRUE != PVRIndexIsFrameValid(handle, u32CurFrameNum))
    {
        MT_ERR_PVR("input frame number %d is invalid start=%d end=%d last=%d.\n", 
					u32CurFrameNum,
					handle->stCycMgr.u32StartFrame,
				    handle->stCycMgr.u32EndFrame,
				    handle->stCycMgr.u32LastFrame);
        return MT_FAILURE;
    }

    while (1)
    {
        if (0 == u32Direction)
            s32NextFrameNum--;
        else
            s32NextFrameNum++;

        if (handle->stCycMgr.u32EndFrame <= handle->stCycMgr.u32StartFrame)
        {
            if (0 == u32Direction)
                s32NextFrameNum = (0 > s32NextFrameNum) ? (MT_S32)handle->stCycMgr.u32LastFrame : s32NextFrameNum;
            else
                s32NextFrameNum = (handle->stCycMgr.u32LastFrame < (MT_U32)s32NextFrameNum) ? 0 : s32NextFrameNum;
        }
        
        if(0==s32NextFrameNum){  //maybe an bad loop.bug
            looptimes++;
            if(2==looptimes){
              return MT_FAILURE;
            }
        }

        if (MT_TRUE != PVRIndexIsFrameValid(handle, (mt_u32)s32NextFrameNum))
        {
            MT_ERR_PVR("next frame number %d is invalid. start=%d end=%d\n",s32NextFrameNum,handle->stCycMgr.u32StartFrame,handle->stCycMgr.u32EndFrame);
            return PVR_INDEX_ERR_INVALID;
        }
		
		    ret = PVRIndexGetEntryByNum(handle, &pFrame, (mt_u32)s32NextFrameNum);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVRIndexGetEntryByNum fail frame=%d start=%d end=%d last=%d ret=%#x.\n",
    				s32NextFrameNum,
    				handle->stCycMgr.u32StartFrame,
    				handle->stCycMgr.u32EndFrame,
    				handle->stCycMgr.u32LastFrame,
    				ret);
            return MT_FAILURE;
        }

		/* IPB frame, and not found the frame upflow flag*/
        if (u32FrameType == PVR_INDEX_FRAME_I)
        {
            if (PVR_INDEX_is_Iframe(&pFrame))
                break;
        }
        else if (u32FrameType == PVR_INDEX_FRAME_P)
        {
            if (PVR_INDEX_is_Pframe(&pFrame))
                break;
        }
        else if (u32FrameType == PVR_INDEX_FRAME_B)
        {
            if (PVR_INDEX_is_Bframe(&pFrame))
                break;
        }
        else
            return MT_FAILURE;
    }
    
    *pu32NextFrameNum = (MT_U32)s32NextFrameNum;
    return MT_SUCCESS;
}

/* unused
// get frame number, frame attribute, GOP attribute from one frame plus N forward
MT_S32 PVR_Index_GetForwardGOPAttr(PVR_INDEX_HANDLE piIndexHandle, 
                                MT_PVR_FETCH_RESULT_S *pPvrFetchRes, 
                                MT_U32 u32StartFrameNum, 
                                MT_U32 u32FrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_U32 i = 0, u32ReadFrameEnd = 0, u32GopTotalFrameNum = 0, u32GopFlag = 0;
    MT_S32 ret = 0;
	
	MT_ASSERT_RET(NULL != piIndexHandle);
	MT_ASSERT_RET(NULL != pPvrFetchRes);

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartFrameNum))
    {
        MT_ERR_PVR("input frame number is invalid.\n");
        return MT_FAILURE;
    }
    
    memset(pPvrFetchRes, 0, sizeof(MT_PVR_FETCH_RESULT_S));
    u32ReadFrameEnd = u32StartFrameNum + u32FrameNum;

    for(i = u32StartFrameNum; i < u32ReadFrameEnd; i++)
    {        
        if(i > piIndexHandle->stCycMgr.u32LastFrame)
        {
            if (piIndexHandle->stCycMgr.u32EndFrame <= piIndexHandle->stCycMgr.u32StartFrame)
            {
                i = 0;
                u32ReadFrameEnd = u32FrameNum - (piIndexHandle->stCycMgr.u32LastFrame - u32StartFrameNum + 1) - 1;
                u32ReadFrameEnd = (u32ReadFrameEnd >= piIndexHandle->stCycMgr.u32EndFrame) ? piIndexHandle->stCycMgr.u32EndFrame : u32ReadFrameEnd;
            }
            else
                break;
        }
        
        if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, i))
        {
            return MT_SUCCESS;
        }

        ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, i);
        if(MT_SUCCESS != ret)
        {
             if(ret == MT_ERR_PVR_FILE_TILL_END)
             {
                return MT_SUCCESS;
             }
             else
             {
                MT_WARN_PVR("get the %d entry fail.\n", i);
                return MT_FAILURE;
             }
        }

        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameNum = i;
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
        memcpy(&(pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
        pPvrFetchRes->u32TotalFrameNum++;

        if (PVR_INDEX_is_Iframe(&frame_tmp))
        {
            u32GopFlag = 1;
            pPvrFetchRes->u32IFrameNum++;
            if((0 != pPvrFetchRes->u32GopNum) && (0 != i))
                pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum - 1].u32LastFrameNum = i-1;
            pPvrFetchRes->u32GopNum++;
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum - 1].u32FirstFrameNum = i;
        }     
        
        if (PVR_INDEX_is_Bframe(&frame_tmp))
        {
            pPvrFetchRes->u32BFrameNum++;
            if (1 == u32GopFlag)  
                pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum - 1].u32BFrameNum++;
        }

        if (PVR_INDEX_is_Pframe(&frame_tmp))
        {
            pPvrFetchRes->u32PFrameNum++;
            if (1 == u32GopFlag)  
                pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].u32PFrameNum++;
        }
        
        if (1 == u32GopFlag)  
        {
            u32GopTotalFrameNum = pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].u32TotalFrameNum;
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].sFrame[u32GopTotalFrameNum].u32FrameNum = i;
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].sFrame[u32GopTotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].sFrame[u32GopTotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].sFrame[u32GopTotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
            memcpy(&(pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].sFrame[u32GopTotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
            pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].u32TotalFrameNum++;
        }
    }

    pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum-1].u32LastFrameNum = i - 1;

    return MT_SUCCESS;
}

// get frame number, frame attribute, GOP attribute from one frame plus N backward
MT_S32 PVR_Index_GetBackwardGOPAttr(PVR_INDEX_HANDLE piIndexHandle, 
                                MT_PVR_FETCH_RESULT_S *pPvrFetchRes, 
                                MT_U32 u32StartFrameNum, 
                                MT_U32 u32FrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_PVR_FETCH_GOP_S *gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
    MT_S32 i = 0 , s32ReadFrameEnd = 0;
    MT_U32 u32GopTotalFrameNum = 0 , u32GopFlag = 1;    
    MT_S32 ret = 0;
	
	MT_ASSERT_RET(NULL != piIndexHandle);
	MT_ASSERT_RET(NULL != pPvrFetchRes);

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartFrameNum))
    {
        MT_ERR_PVR("input frame number is invalid.\n");
        return MT_FAILURE;
    }    

    gop_tmp = (MT_PVR_FETCH_GOP_S *)MT_MALLOC(MT_ID_PVR, sizeof(MT_PVR_FETCH_GOP_S));
    
    if((MT_PVR_FETCH_GOP_S *)NULL == gop_tmp)
    {
        MT_WARN_PVR("MT_MALLOC MT_PVR_FETCH_GOP_S fail.\n");
        return MT_FAILURE;
    }
    
    memset(gop_tmp, 0, sizeof(MT_PVR_FETCH_GOP_S));
    memset(pPvrFetchRes, 0, sizeof(MT_PVR_FETCH_RESULT_S));
    
    s32ReadFrameEnd = (MT_S32)u32StartFrameNum - (MT_S32)u32FrameNum;
    for(i = u32StartFrameNum; i > s32ReadFrameEnd; i--)
    {        
        if(i < 0)
        {
            if (piIndexHandle->stCycMgr.u32EndFrame <= piIndexHandle->stCycMgr.u32StartFrame)
            {
                i = piIndexHandle->stCycMgr.u32LastFrame;
                s32ReadFrameEnd = (MT_S32)(piIndexHandle->stCycMgr.u32LastFrame) - ((MT_S32)u32FrameNum - (MT_S32)(u32StartFrameNum+1)) + 1;
                s32ReadFrameEnd = (s32ReadFrameEnd <= (MT_S32)(piIndexHandle->stCycMgr.u32StartFrame)) ? (MT_S32)(piIndexHandle->stCycMgr.u32StartFrame) : s32ReadFrameEnd;
            }
            else
                break;
        }
        if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, i))
        {
            MT_FREE(MT_ID_PVR, gop_tmp);
            gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
            return MT_SUCCESS;
        }

        ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, i);
        if(MT_SUCCESS != ret)
        {
            if(ret != MT_ERR_PVR_FILE_TILL_END)
            {
                MT_WARN_PVR("get the %d entry fail.\n", i);
                MT_FREE(MT_ID_PVR, gop_tmp);
                gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
                return MT_FAILURE;
            }
        }

        if (1 == u32GopFlag)
        {
            gop_tmp->u32LastFrameNum = i;
            u32GopFlag = 0;
        }

        u32GopTotalFrameNum = gop_tmp->u32TotalFrameNum;
        gop_tmp->sFrame[u32GopTotalFrameNum].u32FrameNum = i;
        gop_tmp->sFrame[u32GopTotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
        gop_tmp->sFrame[u32GopTotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
        gop_tmp->sFrame[u32GopTotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
        memcpy(&(gop_tmp->sFrame[u32GopTotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
        gop_tmp->u32TotalFrameNum++;
        
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameNum = i;
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
        pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
        memcpy(&(pPvrFetchRes->sFrame[pPvrFetchRes->u32TotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
        pPvrFetchRes->u32TotalFrameNum++;

        if (PVR_INDEX_is_Iframe(&frame_tmp))
        {
            u32GopFlag = 1;
            pPvrFetchRes->u32IFrameNum++;
            gop_tmp->u32FirstFrameNum = i;
        }     
        
        if (PVR_INDEX_is_Bframe(&frame_tmp))
        {
            pPvrFetchRes->u32BFrameNum++;
            gop_tmp->u32BFrameNum++;
        }

        if (PVR_INDEX_is_Pframe(&frame_tmp))
        {
            pPvrFetchRes->u32PFrameNum++;
            gop_tmp->u32PFrameNum++;
        }
        
        if (1 == u32GopFlag)  
        {
            MT_S32 j = 0, k = 0;
            memcpy((void *)&(pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum]), (void *)gop_tmp, sizeof(MT_PVR_FETCH_GOP_S));
            for(j = gop_tmp->u32TotalFrameNum - 1; j >= 0; j--)
            {
                memcpy((void *)&(pPvrFetchRes->sGop[pPvrFetchRes->u32GopNum].sFrame[k]),
                       (void *)&(gop_tmp->sFrame[j]),
                       sizeof(MT_PVR_FETCH_FRAME_S));
                k++;
            }
            memset(gop_tmp, 0, sizeof(MT_PVR_FETCH_GOP_S));
            pPvrFetchRes->u32GopNum++;
        }
    }

    MT_FREE(MT_ID_PVR, gop_tmp);
    gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
    return MT_SUCCESS;
}


// get current GOP's attribute, include total frame number, numbers of B/P frame, from one I frame
MT_S32 PVR_Index_GetCurGOPAttr(PVR_INDEX_HANDLE piIndexHandle, 
                                    MT_PVR_FETCH_GOP_S *pPvrGopAttr, 
                                    MT_U32 u32StartIFrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_U32 u32FrameNumTmp = u32StartIFrameNum, u32GopFlag = 0;
    MT_S32 u32Ret = 0;
	
	MT_ASSERT_RET(NULL != piIndexHandle);
	MT_ASSERT_RET(NULL != pPvrGopAttr);

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartIFrameNum))
    {
        MT_ERR_PVR("input frame number is invalid.\n");
        return MT_FAILURE;
    }

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32StartIFrameNum))
    {
         MT_WARN_PVR("get the %d entry fail.\n", u32StartIFrameNum);
         return MT_FAILURE;
    }

    if (!PVR_INDEX_is_Iframe(&frame_tmp))
    {
        MT_WARN_PVR("start frame is not a I frame.\n");
        return MT_FAILURE;
    }

    memset(pPvrGopAttr, 0, sizeof(MT_PVR_FETCH_GOP_S));

    while(1)
    {        
        if (piIndexHandle->stCycMgr.u32EndFrame <= piIndexHandle->stCycMgr.u32StartFrame)
        {
            if (u32FrameNumTmp > piIndexHandle->stCycMgr.u32LastFrame)
                u32FrameNumTmp = 0;
            if ((1 == u32GopFlag) &&
                (u32FrameNumTmp > piIndexHandle->stCycMgr.u32EndFrame) && 
                (u32FrameNumTmp < piIndexHandle->stCycMgr.u32StartFrame))
            {
                MT_WARN_PVR("Can not find next I frame.\n");
                return MT_FAILURE;
            }
        }
        else
        {
             if ((1 == u32GopFlag) && (u32FrameNumTmp > piIndexHandle->stCycMgr.u32LastFrame))
            {
                MT_WARN_PVR("Can not find next I frame.\n");
                return MT_FAILURE;
            }
        }

        u32Ret = PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32FrameNumTmp);
        if(MT_SUCCESS != u32Ret)
        {   
             if (MT_ERR_PVR_FILE_TILL_END == u32Ret)
             {
                MT_WARN_PVR("Reach the end of index, return success.\n");
                return MT_SUCCESS;
             }
             else
             {
                MT_WARN_PVR("get the %d entry fail.\n", u32FrameNumTmp);
                return MT_FAILURE;
             }
        }
        
        if (PVR_INDEX_is_Iframe(&frame_tmp))
        {
            u32GopFlag++;
            if (1 == u32GopFlag)
                pPvrGopAttr->u32FirstFrameNum = u32FrameNumTmp;
        }
        
        if (1 < u32GopFlag)
        {
            pPvrGopAttr->u32LastFrameNum = u32FrameNumTmp - 1;
            break;
        }
        
        if (piIndexHandle->stCycMgr.u32EndFrame == u32FrameNumTmp)
        {
            pPvrGopAttr->u32LastFrameNum = u32FrameNumTmp;
            break;
        }
        
        if (1 == u32GopFlag)
        {
            if (PVR_INDEX_is_Pframe(&frame_tmp))
                pPvrGopAttr->u32PFrameNum++;
            
            if (PVR_INDEX_is_Bframe(&frame_tmp))
                pPvrGopAttr->u32BFrameNum++;

            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameNum = u32FrameNumTmp;
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
            memcpy(&(pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
                
            pPvrGopAttr->u32TotalFrameNum++;
        }

        u32FrameNumTmp++;
    }

    return MT_SUCCESS;
}


// get the next GOP's attribute, include total frame number, numbers of B/P frame, from one I frame
MT_S32 PVR_Index_GetNextGOPAttr(PVR_INDEX_HANDLE piIndexHandle, 
                                    MT_PVR_FETCH_GOP_S *pPvrGopAttr, 
                                    MT_U32 u32StartIFrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_U32 u32FrameNumTmp = u32StartIFrameNum, u32GopFlag = 0;
	
	MT_ASSERT_RET(NULL != piIndexHandle);
	MT_ASSERT_RET(NULL != pPvrGopAttr);

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartIFrameNum))
    {
        MT_ERR_PVR("input frame number is invalid.\n");
        return MT_FAILURE;
    }

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32StartIFrameNum))
    {
         MT_WARN_PVR("get the %d entry fail.\n", u32StartIFrameNum);
         return MT_FAILURE;
    }

    if (!PVR_INDEX_is_Iframe(&frame_tmp))
    {
        MT_WARN_PVR("start frame is not a I frame.\n");
        return MT_FAILURE;
    }

    memset(pPvrGopAttr, 0, sizeof(MT_PVR_FETCH_GOP_S));

    while(1)
    {
        u32FrameNumTmp++;
        
        if (piIndexHandle->stCycMgr.u32EndFrame <= piIndexHandle->stCycMgr.u32StartFrame)
        {
            if (u32FrameNumTmp > piIndexHandle->stCycMgr.u32LastFrame)
                u32FrameNumTmp = 0;
            if ((0 == u32GopFlag) &&
                (u32FrameNumTmp > piIndexHandle->stCycMgr.u32EndFrame) && 
                (u32FrameNumTmp < piIndexHandle->stCycMgr.u32StartFrame))
            {
                MT_WARN_PVR("Can not find next I frame.\n");
                return MT_FAILURE;
            }
        }
        else
        {
             if ((0 == u32GopFlag) && (u32FrameNumTmp > piIndexHandle->stCycMgr.u32LastFrame))
            {
                MT_WARN_PVR("Can not find next I frame.\n");
                return MT_FAILURE;
            }
        }
                    
        if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32FrameNumTmp))
        {
             MT_WARN_PVR("get the %d entry fail.\n", u32FrameNumTmp);
             return MT_FAILURE;
        }
        
        if (PVR_INDEX_is_Iframe(&frame_tmp))
        {
            u32GopFlag++;
            if (1 == u32GopFlag)
                pPvrGopAttr->u32FirstFrameNum = u32FrameNumTmp;
        }
        
        if (1 < u32GopFlag)
        {
            pPvrGopAttr->u32LastFrameNum = u32FrameNumTmp - 1;
            break;
        }
        
        if (1 == u32GopFlag)
        {
            if (u32FrameNumTmp > piIndexHandle->stCycMgr.u32EndFrame)
            {
                pPvrGopAttr->u32LastFrameNum = u32FrameNumTmp - 1;
                break;
            }
            
            if (PVR_INDEX_is_Pframe(&frame_tmp))
                pPvrGopAttr->u32PFrameNum++;
            
            if (PVR_INDEX_is_Bframe(&frame_tmp))
                pPvrGopAttr->u32BFrameNum++;

            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameNum = u32FrameNumTmp;
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
            pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
            memcpy(&(pPvrGopAttr->sFrame[pPvrGopAttr->u32TotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
                
            pPvrGopAttr->u32TotalFrameNum++;
        }
    }

    return MT_SUCCESS;
}

// get the pre GOP's attribute, include total frame number, numbers of B/P frame, from one I frame
MT_S32 PVR_Index_GetPreGOPAttr(PVR_INDEX_HANDLE piIndexHandle, 
                               MT_PVR_FETCH_GOP_S *pPvrGopAttr, 
                               MT_U32 u32StartIFrameNum)
{
    PVR_INDEX_ENTRY_S  frame_tmp ={0}; 
    MT_PVR_FETCH_GOP_S *gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
    MT_S32 s32FrameNumTmp = (MT_S32)u32StartIFrameNum;
    MT_U32 u32GopFlag = 1;
    MT_S32 j = 0, k = 0;
	
	MT_ASSERT_RET(NULL != piIndexHandle);
	MT_ASSERT_RET(NULL != pPvrGopAttr);

    if (MT_TRUE != PVRIndexIsFrameValid(piIndexHandle, u32StartIFrameNum))
    {
        MT_ERR_PVR("input frame number is invalid.\n");
        return MT_FAILURE;
    }

    if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, u32StartIFrameNum))
    {
         MT_WARN_PVR("get the %d entry fail.\n", u32StartIFrameNum);
         return MT_FAILURE;
    }

    if (!PVR_INDEX_is_Iframe(&frame_tmp))
    {
        MT_WARN_PVR("start frame is not a I frame.\n");
        return MT_FAILURE;
    }

    gop_tmp = (MT_PVR_FETCH_GOP_S *)MT_MALLOC(MT_ID_PVR, sizeof(MT_PVR_FETCH_GOP_S));
    
    if((MT_PVR_FETCH_GOP_S *)NULL == gop_tmp)
    {
        MT_WARN_PVR("MT_MALLOC MT_PVR_FETCH_GOP_S fail.\n");
        return MT_FAILURE;
    }

    memset(pPvrGopAttr, 0, sizeof(MT_PVR_FETCH_GOP_S));
    memset(gop_tmp, 0, sizeof(MT_PVR_FETCH_GOP_S));

    while(1)
    {
        s32FrameNumTmp--;
        
        if (piIndexHandle->stCycMgr.u32EndFrame <= piIndexHandle->stCycMgr.u32StartFrame)
        {
            if (s32FrameNumTmp < 0)
                s32FrameNumTmp = (MT_S32)(piIndexHandle->stCycMgr.u32LastFrame);

            if ((s32FrameNumTmp < (MT_S32)(piIndexHandle->stCycMgr.u32StartFrame)) &&
                (s32FrameNumTmp > (MT_S32)(piIndexHandle->stCycMgr.u32EndFrame)))
            {
                MT_WARN_PVR("Can not find pre I frame.\n");
                MT_FREE(MT_ID_PVR, gop_tmp);
                gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
                return MT_FAILURE;
            }
        }
        else
        {
            if (s32FrameNumTmp < (MT_S32)(piIndexHandle->stCycMgr.u32StartFrame))
            {
                MT_WARN_PVR("Can not find pre I frame.\n");
                MT_FREE(MT_ID_PVR, gop_tmp);
                gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
                return MT_FAILURE;
            }
        }
        
        if(MT_SUCCESS != PVR_Index_GetFrameByNum(piIndexHandle, &frame_tmp, s32FrameNumTmp))
        {
             MT_WARN_PVR("get the %d entry fail.\n", s32FrameNumTmp);
             MT_FREE(MT_ID_PVR, gop_tmp);
             gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
             return MT_FAILURE;
        }

        if (1 == u32GopFlag)
        {
            gop_tmp->u32LastFrameNum = s32FrameNumTmp;
            u32GopFlag = 0;
        }

        if (PVR_INDEX_is_Pframe(&frame_tmp))
            gop_tmp->u32PFrameNum++;
        
        if (PVR_INDEX_is_Bframe(&frame_tmp))
            gop_tmp->u32BFrameNum++;

        gop_tmp->sFrame[gop_tmp->u32TotalFrameNum].u32FrameNum = s32FrameNumTmp;
        gop_tmp->sFrame[gop_tmp->u32TotalFrameNum].u32FrameSize = frame_tmp.u32FrameSize;
        gop_tmp->sFrame[gop_tmp->u32TotalFrameNum].u32FrameType = PVR_INDEX_get_frameType(&frame_tmp);
        gop_tmp->sFrame[gop_tmp->u32TotalFrameNum].u32PTS = frame_tmp.u32PtsMs;
        memcpy(&(gop_tmp->sFrame[gop_tmp->u32TotalFrameNum].stIndexEntry), &frame_tmp, sizeof(PVR_INDEX_ENTRY_S));
        gop_tmp->u32TotalFrameNum++;

        if (PVR_INDEX_is_Iframe(&frame_tmp))
        {
            gop_tmp->u32FirstFrameNum = s32FrameNumTmp;
            break;
        }
    }
    
    memcpy((void *)pPvrGopAttr, (void *)gop_tmp, sizeof(MT_PVR_FETCH_GOP_S));
    for(j = gop_tmp->u32TotalFrameNum - 1; j >= 0; j--)
    {
        memcpy((void *)&(pPvrGopAttr->sFrame[k]),
               (void *)&(gop_tmp->sFrame[j]),
               sizeof(MT_PVR_FETCH_FRAME_S));
        k++;
    }

    MT_FREE(MT_ID_PVR, gop_tmp);
    gop_tmp = (MT_PVR_FETCH_GOP_S *)NULL;
    return MT_SUCCESS;
}
*/

MT_S32 MT_Index_GetTotalCntI_IP(PVR_INDEX_HANDLE handle, MT_U32 *pu32ICnt, MT_U32 *pu32IPCnt,MT_U32 *pu32IPBCnt,MT_U32 Max)
{
    PVR_INDEX_ENTRY_S pFrame; 
  	MT_S32 ret=0;
  	MT_U32 i = 0;
    MT_U32 IPBCnt = 0;
    MT_U32 IPCnt = 0;
    MT_U32 ICnt = 0;
    MT_U32 first_iframe = 0xFFFFFFFF;
    MT_U32 pre_iframe_time = 0xFFFFFFFF;
    MT_U32 iframe_interval = 0;
    MT_ASSERT_RET(handle != NULL);

    handle->field_encode = FIELD_ENCODE_IP; //default IP ENCODE
    handle->iframe_interal_max = 0;
    if((0==pu32ICnt)||(0==pu32IPCnt)||(0==pu32IPBCnt)){
        return MT_FAILURE;
    }
    memset(&pFrame, 0, sizeof(PVR_INDEX_ENTRY_S));

    *pu32IPCnt=0;
    *pu32ICnt=0;
    *pu32IPBCnt=0;
    
    if((handle->stCycMgr.u32StartFrame > handle->stCycMgr.u32LastFrame)||
       (handle->stCycMgr.u32EndFrame > handle->stCycMgr.u32LastFrame)){
        return MT_FAILURE;
    }

	  for(i = handle->stCycMgr.u32StartFrame; ; i++)
    {   
        if(i == handle->stCycMgr.u32EndFrame){
            break;
        }
        if(i==handle->stCycMgr.u32LastFrame){
            i=0;
            continue;
        }
        if (MT_TRUE != PVRIndexIsFrameValid(handle, i)){
            MT_ERR_PVR("frame number %d is invalid. start=%d end=%d\n",i,handle->stCycMgr.u32StartFrame,handle->stCycMgr.u32EndFrame);
            continue;
        }
		
        ret = PVRIndexGetEntryByNum(handle, &pFrame, i);
        if (MT_SUCCESS != ret){
			*pu32IPBCnt = IPBCnt;
		    *pu32IPCnt = IPCnt;
		    *pu32ICnt = ICnt;
            MT_ERR_PVR("PVRIndexGetEntryByNum fail frame=%d start=%d end=%d last=%d ret=%#x.\n",
    				i,
    				handle->stCycMgr.u32StartFrame,
    				handle->stCycMgr.u32EndFrame,
    				handle->stCycMgr.u32LastFrame,
    				ret);
            return MT_SUCCESS;
        }
        if(PVR_INDEX_is_Iframe(&pFrame)){
            if(first_iframe == 0xFFFFFFFF){
                first_iframe = i;
                //PVR_ALAWYS_PRINT("First I frame=%d\n",first_iframe);
            }else if(first_iframe +1 == i){
                //two continue I frame,it should II encode for field mode
                handle->field_encode = FIELD_ENCODE_II;
                //PVR_ALAWYS_PRINT("II frame ecnode field_encode=%d next I frame=%d\n",handle->field_encode,i);
            }
            
            if(pre_iframe_time != 0xFFFFFFFF){
                if(pFrame.u32DisplayTimeMs > pre_iframe_time){
                    iframe_interval = pFrame.u32DisplayTimeMs - pre_iframe_time;
                    if(iframe_interval > handle->iframe_interal_max){
                        handle->iframe_interal_max = iframe_interval;
                        //pChnAttr->IndexHandle("----Iframe interval max=%d\n",handle->iframe_interal_max);
                    }
                }
            }
            pre_iframe_time = pFrame.u32DisplayTimeMs;    
            ICnt++;
        }

	if(PVR_INDEX_is_Iframe(&pFrame) || PVR_INDEX_is_Pframe(&pFrame)){
		  IPCnt++;
        }
        IPBCnt++;
        if(IPBCnt>=Max){
            break;
        }
    }
    *pu32IPBCnt = IPBCnt;
    *pu32IPCnt = IPCnt;
    *pu32ICnt = ICnt;
    return MT_SUCCESS;

}

/*
MT_S32 MT_Index_GetTotalCntIP(PVR_INDEX_HANDLE handle, MT_U32 *pu32IPCnt)
{
    PVR_INDEX_ENTRY_S pFrame; 
	MT_S32 ret=0;
	MT_U32 i = 0;
    MT_U32 IPCnt = 0;
    MT_ASSERT_RET(handle != NULL);

    memset(&pFrame, 0, sizeof(PVR_INDEX_ENTRY_S));

    *pu32IPCnt=0;
    
    if((handle->stCycMgr.u32StartFrame > handle->stCycMgr.u32LastFrame)||
       (handle->stCycMgr.u32EndFrame > handle->stCycMgr.u32LastFrame)){
        return MT_FAILURE;
    }

  	for(i = handle->stCycMgr.u32StartFrame; ; i++)
      {   
        if(i == handle->stCycMgr.u32EndFrame){
            break;
        }
        if(i==handle->stCycMgr.u32LastFrame){
            i=0;
            continue;
        }
        if (MT_TRUE != PVRIndexIsFrameValid(handle, i)){
            MT_ERR_PVR("frame number %d is invalid. start=%d end=%d\n",i,handle->stCycMgr.u32StartFrame,handle->stCycMgr.u32EndFrame);
            continue;
        }
		
		    ret = PVRIndexGetEntryByNum(handle, &pFrame, i);
        if (MT_SUCCESS != ret){
            MT_ERR_PVR("PVRIndexGetEntryByNum fail frame=%d start=%d end=%d last=%d ret=%#x.\n",
				i,
				handle->stCycMgr.u32StartFrame,
				handle->stCycMgr.u32EndFrame,
				handle->stCycMgr.u32LastFrame,
				ret);
            return MT_FAILURE;
        }

		if(PVR_INDEX_is_Iframe(&pFrame) || PVR_INDEX_is_Pframe(&pFrame)){
			IPCnt++;
        }
    }
    *pu32IPCnt = IPCnt;
    return MT_SUCCESS;

}
MT_S32 MT_Index_GetTotalCntI(PVR_INDEX_HANDLE handle, MT_U32 *pu32ICnt)
{
    PVR_INDEX_ENTRY_S pFrame; 
	  MT_S32 ret=0;
	  MT_U32 i = 0;
    MT_U32 ICnt = 0;
    MT_ASSERT_RET(handle != NULL);

    memset(&pFrame, 0, sizeof(PVR_INDEX_ENTRY_S));

    *pu32ICnt=0;
    if((handle->stCycMgr.u32StartFrame > handle->stCycMgr.u32LastFrame)||
       (handle->stCycMgr.u32EndFrame > handle->stCycMgr.u32LastFrame)){
        return MT_FAILURE;
    }

	  for(i = handle->stCycMgr.u32StartFrame; ; i++)
    {   
        if(i == handle->stCycMgr.u32EndFrame){
            break;
        }
        if(i==handle->stCycMgr.u32LastFrame){
            i=0;
            continue;
        }
        if (MT_TRUE != PVRIndexIsFrameValid(handle, i)){
            MT_ERR_PVR("frame number %d is invalid. start=%d end=%d\n",i,handle->stCycMgr.u32StartFrame,handle->stCycMgr.u32EndFrame);
            continue;
        }
		
		    ret = PVRIndexGetEntryByNum(handle, &pFrame, i);
        if (MT_SUCCESS != ret){
            MT_ERR_PVR("PVRIndexGetEntryByNum fail frame=%d start=%d end=%d last=%d ret=%#x.\n",
				i,
				handle->stCycMgr.u32StartFrame,
				handle->stCycMgr.u32EndFrame,
				handle->stCycMgr.u32LastFrame,
				ret);
            return MT_FAILURE;
        }

		    if(PVR_INDEX_is_Iframe(&pFrame)){
			  ICnt++;
        }
    }

    *pu32ICnt = ICnt;
    return MT_SUCCESS;
}*/

void pvr_crypto_init(PVR_REC_CHN_S **srec,PVR_PLAY_CHN_S** sply)
{//this for test!
    MT_UNF_PVR_CIPHER_S crypto;
    mt_u32 i=0;
    static PVR_REC_CHN_S rec;
    static PVR_PLAY_CHN_S ply;
    *srec=&rec;
    *sply=&ply;

    crypto.u32KeyLen=16;
    crypto.enType=MT_CIPHER_ALG_AES;
    crypto.bDoCipher=MT_TRUE;
    for(i=0;i<crypto.u32KeyLen;i++){
        crypto.au8Key[i]=(MT_U8)(10+i);
    }
    memcpy(&rec.stUserCfg.stEncryptCfg,&crypto,sizeof(MT_UNF_PVR_CIPHER_S));
    rec.chiptype=2;
    memcpy(&ply.stUserCfg.stDecryptCfg,&crypto,sizeof(MT_UNF_PVR_CIPHER_S));
    ply.chiptype=2;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

