/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_INDEX_H__
#define __MT_PVR_INDEX_H__

#include <sys/ioctl.h>

#include "mt_type.h"
#include "mt_pvr_priv.h"
#include "mt_pvr_fifo.h"

#include "mt_unf_demux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define PVR_ALAWYS_PRINT    printf
#define PTS_LOOP_MS     95443717   //1ffffffff/90

#define PVR_TPLAY_MIN_DISTANCE      120  /* the min interval of trickmode for sending frame, the max value, decode 40 frame per second with 1920*1080 definition */

#define PVR_TPLAY_MIN_FRAME_RATE	 40	 /* ms, per frame */

#define PVR_TPLAY_FRAME_SHOW_TIME    40UL  /* ms, no need to play too fast at TrickMode  */ 

#define PVR_INDEX_REC                0
#define PVR_INDEX_PLAY               1
//#define PVR_IDX_CACHE_LOCK(p_mutex)       (void)pthread_mutex_lock(p_mutex)
//#define PVR_IDX_CACHE_UNLOCK(p_mutex)     (void)pthread_mutex_unlock(p_mutex)
#define PVR_IDX_CACHE_LOCK_W(handle)       do{(void)pthread_mutex_lock(&(handle->stIdxWriteCache.stCacheMutex));handle->line_magc2=__LINE__;handle->u32magic2++;}while(0)
#define PVR_IDX_CACHE_UNLOCK_W(handle)     do{handle->u32magic2--;handle->line_magc2=0;(void)pthread_mutex_unlock(&(handle->stIdxWriteCache.stCacheMutex));}while(0)
#define PVR_IDX_CACHE_LOCK_R(handle)       do{(void)pthread_mutex_lock(&(handle->stIdxReadCache.stCacheMutex));handle->line_magc3=__LINE__;handle->u32magic3++;}while(0)
#define PVR_IDX_CACHE_UNLOCK_R(handle)     do{handle->u32magic3--;handle->line_magc3=0;(void)pthread_mutex_unlock(&(handle->stIdxReadCache.stCacheMutex));}while(0)
#if 0
#define PVR_INDEX_LOCK(p_mutex)        MT_INFO_PVR("==>\n");(void)pthread_mutex_lock(p_mutex);MT_INFO_PVR("==|\n")
#define PVR_INDEX_UNLOCK(p_mutex)      MT_INFO_PVR("<==\n");(void)pthread_mutex_unlock(p_mutex);MT_INFO_PVR("==|\n")
#else
//#define PVR_INDEX_LOCK(p_mutex)       (void)pthread_mutex_lock(p_mutex)
//#define PVR_INDEX_UNLOCK(p_mutex)     (void)pthread_mutex_unlock(p_mutex)
#define PVR_INDEX_LOCK(handle)        do{(void)pthread_mutex_lock(&(handle->stMutex));handle->line_magc1=__LINE__;handle->u32magic1++;}while(0)
#define PVR_INDEX_UNLOCK(handle)      do{handle->u32magic1--;handle->line_magc1=0;(void)pthread_mutex_unlock(&(handle->stMutex));}while(0)
#endif

#define PVR_INDEX_ERR_INVALID    (-2)

/* frame type definition                                                    */
    /*
    001	intra-coded (I)
    010	predictive-coded (P)
    011	bidirectionally-predictive-coded (B)
    100	shall not be used
    (dc intra-coded (D) in ISO/IEC11172-2)
    */
#define PVR_INDEX_FRAME_I            0x01
#define PVR_INDEX_FRAME_P            0x02
#define PVR_INDEX_FRAME_B            0x03

#define FIELD_ENCODE_IP 0
#define FIELD_ENCODE_II  1


/* start code type definition(data from SCD buffer) */
#define PVR_INDEX_SC_TYPE_TS         0x1      /* ts packet header */
#define PVR_INDEX_SC_TYPE_PTS        0x2      /* pes packet header */
#define PVR_INDEX_SC_TYPE_PAUSE      0x3      /* pause flag */
#define PVR_INDEX_SC_TYPE_PIC        0x4      /* the start 00 00 01 of frame data */
#define PVR_INDEX_SC_TYPE_PIC_SHORT  0x5      /* the short head 00 01 of frame data */
#define PVR_INDEX_SC_TYPE_PES_ERR    0xf      /* the header of PES syntax error */


#define PVR_INDEX_HEADER_CODE        0x5A5A5A5A
#define PVR_DFT_RESERVED_REC_SIZE    (1024*1024)
#define PVR_DFT_IDX_WRITECACHE_SIZE      (4*1024)
#define PVR_DFT_IDX_READCACHE_SIZE       (16*1024)

#define PVR_MIN_CYC_SIZE (50 * 1024 * 1024LLU)
#define PVR_MIN_CYC_TIMEMS (5 *1000)            //5s

#define PVR_MIN_CYC_DIFF (4LLU * 1024LLU * 1024LLU)

#define PVR_INDEX_PAUSE_INVALID_OFFSET      ((MT_U32)(-1))
#define PVR_INDEX_STEPBACK_INVALID_OFFSET   ((MT_U32)(-1))
#define PVR_INDEX_INVALID_PTSMS             ((MT_U32)(-1))
#define PVR_INDEX_DEFFRAME_PTSMS            (40)
#define PVR_INDEX_INVALID_SEQHEAD_OFFSET    ((MT_U64)(-1))
#define PVR_INDEX_INVALID_I_FRAME_OFFSET    (0x3fffU)
#define PVR_INDEX_PAUSE_SEQHEAD_OFFSET      ((MT_U64)(-2))
#define PVR_INDEX_SCD_WRAP_MS               (47721858)/*scd Wrap-around value in MS:0xffffffff/90*/

#define MAX_FRAME_NUM_ONCE_FETCH 256
#define MAX_GOP_NUM_ONCE_FETCH 256

#define PVR_REC_INDEX_MAGIC_WORD  0x696E6478 //ASCII code of "indx"

/* rewind record or not */
#define PVR_INDEX_IS_REWIND(handle)         ((handle)->stCycMgr.bIsRewind)
#define PVR_INDEX_TYPE_REWIND(handle)       ((handle)->stCycMgr.enRewindType)

/* record or not */
#define PVR_INDEX_IS_REC(handle)         ((handle)->stCycMgr.bIsRec)

/* play or not */
#define PVR_INDEX_IS_PLAY(handle)         ((handle)->stCycMgr.bIsPlay)

/*whether index type is audio or not */
#define PVR_INDEX_IS_TYPE_AUDIO(handle)     ((handle)->enIndexType == MT_UNF_PVR_REC_INDEX_TYPE_AUDIO)

/* the max size of ts file */
#define PVR_INDEX_MAX_FILE_SIZE(handle)     ((handle)->stCycMgr.u64MaxCycSize)



/* get frame type macro                                                      */
#define PVR_INDEX_get_frameType(pEntry) \
(((pEntry)->u16FrameTypeAndGop >> 14) & 0x3UL)

/* get offset from previous I frame macro                                   */
#define PVR_INDEX_get_preIoffset(pEntry) \
((pEntry)->u16FrameTypeAndGop & 0x3fffUL)


/* judgement of frame type                                                  */
#define PVR_INDEX_is_Iframe(pEntry) \
((((pEntry)->u16FrameTypeAndGop >> 14) & 0x3UL) == PVR_INDEX_FRAME_I)

#define PVR_INDEX_is_Bframe(pEntry) \
((((pEntry)->u16FrameTypeAndGop >> 14) & 0x3UL) == PVR_INDEX_FRAME_B)

#define PVR_INDEX_is_Pframe(pEntry) \
((((pEntry)->u16FrameTypeAndGop >> 14) & 0x3UL) == PVR_INDEX_FRAME_P)

#define PVR_INDEX_OverFlow(pEntry)\
((((pEntry)->u16UpFlowFlag) & 0x1UL) == 0x1UL)

#define PVR_INDEX_Masked(pEntry) \
((((pEntry)->u16UpFlowFlag >> 1) & 0x1UL) == 0x1UL)

#define PVR_INDEX_REC_CHN_UNUSED 0
#define PVR_INDEX_REC_CHN_USED 1


#define PVR_IS_SPEED_SEND_ALL(speed) \
    (MT_UNF_PVR_PLAY_SPEED_NORMAL == (speed) \
     || MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_8X_SLOW_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_16X_SLOW_FORWARD == (speed)\
     || MT_UNF_PVR_PLAY_SPEED_32X_SLOW_FORWARD == (speed))

/* pvr index user list                                                      */
typedef enum mtPVR_INDEX_USER_E
{
    PVR_INDEX_USER_FREE = 0x00,                   /* no one use it */
    PVR_INDEX_USER_REC  = 0x01,                   /* used by record */
    PVR_INDEX_USER_PLAY = 0x02,                   /* used by play */
    PVR_INDEX_USER_BOTH = 0x03,                   /* used by record and play meantime */
    PVR_INDEX_USER_BUTT
} PVR_INDEX_USER_E;

typedef enum mtPVR_INDEX_REWIND_TYPE_E
{
    PVR_INDEX_REWIND_BY_SIZE = 0x00,                   /* rewind by size */
    PVR_INDEX_REWIND_BY_TIME = 0x01,                   /* rewind by time */
    PVR_INDEX_REWIND_BY_BOTH = 0x02,
    PVR_INDEX_REWIND_BUTT
} PVR_INDEX_REWIND_TYPE_E;
typedef struct pvr_thread_attr
{
    MT_U32 flag_valid;
    MT_S32 schedpolicy;
    MT_S32 priority;
    MT_S32 stacksize;
}PVR_THREAD_ATTR;

/*append rec info*/
typedef struct pvr_append_rec
{
    MT_U32 u32TotalTime;        //s
    MT_U32 u32LastIndex;        //last index
}PVR_APPEND_REC;

/* rewind record control info */
typedef struct mtPVR_CYC_MGR_S
{
    MT_BOOL bIsRewind;          /* rewind record or not */
    MT_U32  u32StartFrame;      /* the first valid frame number in index on cycle playing */
    MT_U32  u32EndFrame;        /* the last valid frame number in index on cycle playing */
    MT_U32  u32LastFrame;       /* the last number of frame cycle end */

    MT_S32  s32CycTimes;        /* the times for cycle record */
    MT_U32  u32Reserve;         /* u64 aligned */
    MT_U64  u64MaxCycSize;      /* max file size of cycle record */
    MT_U64  u64MaxCycTimeInMs;  /* max time length of cycle record */
    MT_U64  u64AllRecStartInMs;  /* already rec time in ms */
    MT_U64  u64AllRecNowInMs;  /* already rec time in ms */
    PVR_INDEX_REWIND_TYPE_E enRewindType;  /* rewind type */
}PVR_CYC_MGR_S;

/*idx cache buffer*/
typedef struct
{
    MT_U8* pu8Addr;                              /*buffer addr*/
    MT_U32 u32BufferLen;                         /*buffer length*/
    MT_U32 u32UsedSize;                          /*used size of buffer*/
    MT_U32 u32StartOffset;                       /*start offset*/
    pthread_mutex_t stCacheMutex;                /*cache lock*/
} HIPVR_IDX_BUF_S;

/* the gop info struction of index */
typedef struct mtPVR_INDEX_INFO
{
    MT_U32          u32GopTotalNum;     
    MT_U32          u32FrameTotalNum;  
    MT_U32          u32MaxGopSize;          /* the max size of GOP in history */
    MT_U32          u32GopSizeInfo[13];    /* gopnum of gopsize in 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 */
} PVR_INDEX_INFO_S;

typedef struct mtPVR_REC_INDEX_INFO
{
    MT_U32 u32MagicWord;
    MT_U32 u32LastGopSize;
    MT_U32 u32Reserved[14];
    PVR_INDEX_INFO_S stIdxInfo;
}PVR_REC_INDEX_INFO_S;

/* pvr index handle descriptor                                              */
typedef struct mtPVR_INDEXER_S
{
    MT_U32               u32magic1;
    MT_U32               line_magc1;
    pthread_mutex_t      stMutex;
    HIPVR_IDX_BUF_S      stIdxWriteCache;
    MT_U32               u32magic2;
    MT_U32               line_magc2;
    HIPVR_IDX_BUF_S      stIdxReadCache;
    MT_U32               line_magc3;
    MT_U32               u32magic3;
    
    MT_BOOL              bIsRec;                 /* record or not */
    MT_BOOL              bIsPlay;                /* play or not */
    MT_U32               UpdatingEvent_CheckTimes;    /* updating event,how many times to recheck */

    PVR_FILE             s32ReadFd;              /* read descriptor for index file */
    PVR_FILE             s32Readdirect_Fd;        /* direct read */
    PVR_FILE             s32SeekFd;              /* seek descriptor for index file */
    PVR_FILE             s32WriteFd;             /* write descriptor for index file */
    PVR_FILE             s32HeaderFd;            /* write index header for index file */

    MT_UNF_PVR_REC_INDEX_TYPE_E enIndexType;     /* the type of index, in common about record and play. assigned init index */
    PVR_CYC_MGR_S               stCycMgr;        /* control rewind record, and save the frame position, regardless of rewind record */

    MT_U64               u64GlobalOffset;        /* last write frame offset, the total data size from start record to current play or record, included the rewind data */
    MT_U64               u64FileSizeGlobal;      /* the actual file saved size, for debug only  */
    
    MT_U32               u32IdxStartOffsetLen;   /* the length of file header, included header info and user info. in common between record and play*/

    MT_U32               u32LastDavBufOffset;    /* last DAV buffer offset, on recording, save the offset of dav buffer */
    MT_U32               u32DavBufSize;          /* demux dav buffer size, on recording, save the size of dav buffer */
    MT_U32               u32IdxBufSize;

    MT_U32               u32PauseFrame;          /* mark a pause flag for recording file, included pause the record file on live and pause the playing timeshif */
    MT_U64               u64PauseOffset;         /* the offset from the record start to pause flag. used for checking the pause position rewriten or not by rewind record. */

    MT_U16               u16RecLastIframe;       /* on recording, save the previous I frame position */
    MT_U16               u16RecUpFlowFlag;       /* on recording, dav up flow flag */
    MT_U32               u32RecLastValidPtsMs;   /* on recording, save the previous valid PTS */
    MT_U32               u32RecPicParser;        /* on recording, FIDX ID */
    MT_U32               u32RecFirstFrmTimeMs;   /* on recording, save the system time at the first frame incoming */

    MT_U32               u32WriteFrame;          /* the write pointer, frame number of index file on recording */
    MT_U32               u32ReadFrame;           /* the read porinter, frame number of index file on playing */
    MT_U32               u32PlayFrame;           /* the current playing frame, frame number of index file*/
    PVR_INDEX_ENTRY_S    stCurRecFrame;          /* the current frame info of recording */
    PVR_INDEX_ENTRY_S    stCurPlayFrame;         /* the current frame info of outputing */
    MT_BOOL 			 bIsFristIframe;		 /* flag the first I frame or not on ff and rw trick mode */
    MT_U32 				 u32FrameDistance;		 /* the frame number of trick mode between I frame */   
    MT_U32               u32RecReachPlay;        /* record catchs up the play or not, catched and reset it */

	MT_U32 				 u32FflushTime;			 /* fresh time pointer flag */
    MT_U32               u32DmxClkTimeMs;
    MT_U32               u32FRollTime;

    MT_UNF_PVR_FILE_ATTR_S    stIndexFileAttr;   /* for pure play, the file attribute of the exist index file, and just only assigned on creating play channel */

    MT_CHAR              szIdxFileName[PVR_MAX_FILENAME_LEN+4];
    
//    MT_U32 u32LastDispTime;                      /* the latest recording disptime */
//    MT_U32 u32DeltaDispTimeMs;                   /* the delta value of disptime after the signal lose */
    MT_S32 u32LastDispTime;                      /* the latest recording disptime */
    MT_S32 u32DeltaDispTimeMs;                   /* the delta value of disptime after the signal lose */	
    MT_U32 u32TimeShiftTillEndTimeMs;            /* the recording disptime when timeshift till end */
    MT_U32 u32TimeShiftTillEndCnt;               /* the counter of timeshift till end */
    PVR_REC_INDEX_INFO_S   stRecIdxInfo;
    MT_BOOL            bTimeRewindFlg;           /*Time ok, Need to do rewind*/
    MT_BOOL            bTimeRewindTsPauseFlg;    /*ts paused.vir! modify:record ts size,we can't stop ts,it will overflow*/
	MT_BOOL            bTimeRewinded;
	MT_U64             u64TimeRewindMaxSize;        /*all size before rewind*/
	MT_U64             u64TimeTsPausePos;         /*rewid from here*/
    MT_UNF_DMX_REC_INDEX_S *idxcache;
    MT_U32             idxstart;
    MT_U32             idxend;
    MT_U32             idxmax;  //mustbe 2^n.mask=idxmax-1
    MT_U64             recsize64;
    MT_U32             flag_lastI;                //last frame is Iframe
    MT_UNF_VCODEC_TYPE_E             VdecType;    //vdec type
    MT_U32             idx_chn_num_avap;
    MT_U32             cause_resume;              //1:sof\2:rectostart;3:eof;4:play reach rec;
    MT_U32             Latest_DisplayTimeMs;      //the latest pts
    MT_U32             Last_DisplayTimeMs;        //the end's pts
    /*cache idx ,speed up,start*/
    MT_U32    u32StartFrame_dread;
    MT_U32    u32EndFrame_dread;
    MT_U32    u32LastFrame_dread;
    PVR_INDEX_ENTRY_S startEntry;
    PVR_INDEX_ENTRY_S endEntry;
    PVR_INDEX_ENTRY_S lastEntry;
    /*cache idx ,speed up,end*/
    MT_U16    field_encode;// for filed encode mode type, 0 is I P frame encode, 1 is I I frame encode
    MT_U32    iframe_interal_max;
}PVR_INDEX_S, *PVR_INDEX_HANDLE;

typedef struct
{
    MT_U32  u32FrameNum;  
    MT_U32  u32PTS;
    MT_U32  u32FrameSize;
    MT_U32  u32FrameType;
    PVR_INDEX_ENTRY_S stIndexEntry;
} MT_PVR_FETCH_FRAME_S;

typedef struct
{
    MT_U32  u32TotalFrameNum; 
    MT_U32  u32FirstFrameNum;
    MT_U32  u32LastFrameNum;
    MT_U32  u32PFrameNum;
    MT_U32  u32BFrameNum;
    MT_U32  u32WithoutBLargerSize;  /* the max gopsize value, except B frames */ 
    MT_PVR_FETCH_FRAME_S  sFrame[MAX_FRAME_NUM_ONCE_FETCH];  /* the description of every frame, contains 256 frames */
} MT_PVR_FETCH_GOP_S;

typedef struct
{
    MT_U32  u32TotalFrameNum;    /* total frame numbers */
    MT_U32  u32IFrameNum;        /* total I frame numbers */
    MT_U32  u32PFrameNum;        /* total P frame numbers */
    MT_U32  u32BFrameNum;        /* total B frame numbers */
	MT_U32  u32GopNum;          /* total GOP numbers */
    MT_PVR_FETCH_FRAME_S  sFrame[MAX_FRAME_NUM_ONCE_FETCH];  /* the description of every frame, contains 256 frames */
	MT_PVR_FETCH_GOP_S sGop[MAX_GOP_NUM_ONCE_FETCH];         /* the description of every GOP, contains 256 GOPs */

} MT_PVR_FETCH_RESULT_S;

typedef struct
{
    MT_U32  u32TotalFrameNum;        /* total frame numbers */
    MT_PVR_FETCH_FRAME_S  sFrame[MAX_FRAME_NUM_ONCE_FETCH];  /* the description of every frame, contains 256 frames */
} MT_PVR_SEND_RESULT_S;

typedef struct
{
    MT_UNF_PVR_PLAY_SPEED_E  enSpeed;     /* trick mode speed */
    MT_UNF_VCODEC_TYPE_E  enVideoType;    /* vedio type */
    MT_U32               enChipID;        /* chip ID */
    MT_U32               enChipVer;       /* chip version */
    MT_U32               u32Width;        /* width */
    MT_U32               u32Height;       /* heigth */
    MT_U32               u32VoRate;       /* frame rate of VO */
    MT_U32               u32VoDropFrame;  /* flag of VO drop frame */  
}MT_PVR_FAST_FORWARD_BACKWARD_S;



MT_U32 PVRIndexGetCurTimeMs(MT_VOID);



/* init index module, and create index handle, and destroy   */
MT_S32 PVR_Index_Init(MT_VOID);
PVR_INDEX_HANDLE PVR_Index_CreatPlay(MT_U32 chnID,
                                const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr,
                                MT_BOOL *pIsNoIdx,
                                MT_U32    *tsFileFirstNode,
                                MT_BOOL bSupportTimeShiftEvent,
                                MT_BOOL bIsTimeShiftEventFile);
PVR_INDEX_HANDLE PVR_Index_CreatRec(MT_U32 chnID,
                                MT_UNF_PVR_REC_ATTR_S *pstRecAttr);

MT_S32 PVR_Index_Destroy(PVR_INDEX_HANDLE handle, MT_U32 u32PlayOrRec);

/* attr opration */
MT_VOID PVR_Index_ResetRecAttr(PVR_INDEX_HANDLE handle);
MT_VOID PVR_Index_ResetPlayAttr(PVR_INDEX_HANDLE handle);
MT_S32 PVR_Index_ChangePlayMode(PVR_INDEX_HANDLE handle);

/***** save frame *****/
MT_S32 PVR_Index_SaveFramePosition(MT_U32 u32ChnID, MT_U32 InstIdx, MT_UNF_DMX_REC_INDEX_S *pstScInfo,MT_U32 u32DirectFlag);
MT_S32 PVR_Index_FlushIdxWriteCache(PVR_INDEX_HANDLE    handle);
MT_S32 PVR_Index_IfOffsetInWriteCache(PVR_INDEX_HANDLE    handle,MT_U32 u32Offset,MT_U32 u32Size);
MT_VOID PVR_Index_RecUpdateStartFrame(PVR_INDEX_HANDLE handle, MT_U64 u64RecSize);
MT_VOID PVR_Index_RecUpdateEndFrame(PVR_INDEX_HANDLE handle);

/* get frame opration    */
MT_S32 PVRIndexGetPlayNextEntry(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pEntry);
MT_S32 PVR_Index_GetNextFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pstFrame);
MT_S32 PVR_Index_GetNextIFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pstFrame);
MT_S32 PVR_Index_GetPreIFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pstFrame);
MT_S32 PVR_Index_GetCurrentFrame(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry);
MT_S32 PVR_Index_QueryFrameByPTS(const PVR_INDEX_HANDLE handle, MT_U32 u32SearchPTS, PVR_INDEX_ENTRY_S *pEntry, MT_U32 *pu32Pos, MT_U32 IsForword);
MT_S32 PVR_Index_QueryFrameByTime(const PVR_INDEX_HANDLE handle, MT_U32 u32SearchTime, PVR_INDEX_ENTRY_S *pEntry, MT_U32 *pu32Pos);
MT_S32 PVR_Index_GetFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num);
MT_U32 PVRIndexFindFrameByPTS(PVR_INDEX_HANDLE handle, MT_U32 u32PtsSearched, MT_U32 u32FrmPos, MT_U32 IsForword);
MT_S32 PVR_Index_GetNearbyIFrameByNum(const PVR_INDEX_HANDLE handle,  PVR_INDEX_ENTRY_S *pEntry, MT_U32 num,MT_S32 direct);



/* seek opration */
MT_S32 PVR_Index_SeekToPTS(PVR_INDEX_HANDLE handle, MT_U32 u32PtsMs, MT_U32 IsForword, MT_U32 IsNextForword);
MT_S32 PVR_Index_SeekToTime(PVR_INDEX_HANDLE handle, MT_U32 u32TimeMs);
MT_S32 PVR_Index_SeekToStart(PVR_INDEX_HANDLE handle);
MT_S32 PVR_Index_SeekToEnd(PVR_INDEX_HANDLE handle);
MT_S32 PVR_Index_SeekToPauseOrStart(PVR_INDEX_HANDLE handle);
MT_S32 PVR_Index_SeekByTime(PVR_INDEX_HANDLE handle, MT_S64 offset, MT_S32 whence, MT_U32 curplaytime);
MT_S32 PVR_Index_SeekByFrame2I(PVR_INDEX_HANDLE handle, MT_S32 offset, MT_S32 whence);


/* for timeshift pause  */
MT_S32 PVR_Index_MarkPausePos(PVR_INDEX_HANDLE handle);

/*file opration*/
MT_S32 PVR_Index_PlayGetFileAttrByFileName(const MT_CHAR *pFileName, PVR_INDEX_HANDLE pIdxHandle, MT_UNF_PVR_FILE_ATTR_S *pAttr);
MT_VOID PVR_Index_GetIdxFileName(MT_CHAR* pIdxFileName, MT_CHAR* pSrcFileName);
MT_S32 PVR_Index_PrepareHeaderInfo(PVR_INDEX_HANDLE handle, MT_U32 u32UsrDataLen, MT_U32 u32Vtype);
MT_S32 PVR_Index_GetUsrDataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32BuffSize);
MT_S32 PVR_Index_SetUsrDataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32UsrDataLen);
MT_S32 PVR_Index_GetCADataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32BuffSize);
MT_S32 PVR_Index_SetCADataInfo(MT_S32 s32Fd, MT_U8* pBuff, MT_U32 u32CADataLen);
MT_VOID PVR_Index_GetIdxInfo(PVR_INDEX_HANDLE handle);
MT_VOID PVR_Index_GetRecIdxInfo(PVR_INDEX_HANDLE handle);
MT_VOID PVR_Index_RecIdxInfo(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pstIdxEntry);
MT_VOID PVR_Index_UpdateIdxInfoWhenRewind(PVR_INDEX_HANDLE handle);
MT_VOID PVR_Index_RecLastIdxInfo(PVR_INDEX_HANDLE handle);
MT_BOOL PVR_Index_CheckSetRecReachPlay(PVR_INDEX_HANDLE handle);
MT_BOOL PVR_Index_QureyClearRecReachPlay(PVR_INDEX_HANDLE handle);

MT_S32 PVR_Index_GetVtype(PVR_INDEX_HANDLE handle);
MT_S32 PVR_Index_GetMaxBitrate(PVR_INDEX_HANDLE piIndexHandle);
MT_S32 PVR_Index_GetStreamBitRate(PVR_INDEX_HANDLE piIndexHandle,MT_U32 *pBitRate,MT_U32 u32StartFrameNum,MT_U32 u32EndFrameNum);
MT_S32 PVR_Index_GetFBwardIPBFrameNum(PVR_INDEX_HANDLE handle, MT_U32 u32Direction, MT_U32 u32FrameType, MT_U32 u32CurFrameNum, MT_U32 *pu32NextFrameNum);
MT_S32 PVR_Index_GetCurGOPAttr(PVR_INDEX_HANDLE piIndexHandle, MT_PVR_FETCH_GOP_S *pPvrGopAttr, MT_U32 u32StartIFrameNum);
MT_S32 PVR_Index_GetForwardGOPAttr(PVR_INDEX_HANDLE piIndexHandle, MT_PVR_FETCH_RESULT_S *pPvrFetchRes, MT_U32 u32StartFrameNum, MT_U32 u32FrameNum);
MT_S32 PVR_Index_GetBackwardGOPAttr(PVR_INDEX_HANDLE piIndexHandle, MT_PVR_FETCH_RESULT_S *pPvrFetchRes, MT_U32 u32StartFrameNum, MT_U32 u32FrameNum);
MT_S32 PVR_Index_GetNextGOPAttr(PVR_INDEX_HANDLE piIndexHandle, MT_PVR_FETCH_GOP_S *pPvrGopAttr, MT_U32 u32StartIFrameNum);
MT_S32 PVR_Index_GetPreGOPAttr(PVR_INDEX_HANDLE piIndexHandle, MT_PVR_FETCH_GOP_S *pPvrGopAttr, MT_U32 u32StartIFrameNum);
MT_S32 PVR_Index_GetFrmNumByEntry(PVR_INDEX_HANDLE pstIndexHandle, PTR_PVR_INDEX_ENTRY pstIndexEntry, MT_S32 *ps32FrmNum);
MT_S32 PVR_Index_GetFrameRate(PVR_INDEX_HANDLE piIndexHandle, MT_U32 *pFrameRate);
MT_S32 MT_Index_GetTotalCntI_IP(PVR_INDEX_HANDLE handle, MT_U32 *pu32ICnt, MT_U32 *pu32IPCnt,MT_U32 *pu32IPBCnt,MT_U32 Max);
MT_S32 PVR_Index_GetNextIPFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame);
MT_S32 PVR_Index_GetPreXFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame);
MT_S64 PVR_Index_CutEventPart(int srcfd, int dstfd, PVR_IDX_HEADER_INFO_S *stIdxHeaderInfo,  MT_U32  headerInfoLen, MT_U64 startpos,  MT_U64 endpos);

MT_BOOL Pvr_Check_ReadInRange(MT_U32 s,MT_U32 e,MT_U32 r);
MT_S32 Pvr_Adjust_ReadInGoodRange(PVR_INDEX_HANDLE IndexHandle,MT_U32 dirct);
MT_U32 Pvr_Move_Readpos_forward(MT_U32 s,MT_U32 e,MT_U32 l,MT_U32 r,MT_U32 mx);
MT_U32 Pvr_Move_Readpos_backward(MT_U32 s,MT_U32 e,MT_U32 l,MT_U32 r,MT_U32 mx);
MT_U32 Pvr_Calc_distance(MT_U32 s,MT_U32 l,MT_U32 x);
MT_S32 PVRIndexGetHeaderInfoByName(MT_U8 *indexFileName, PVR_IDX_HEADER_INFO_S* pHeadInfo,MT_U32 size);

MT_S32 PVR_Index_GetLastIdxWriteCacheIndexEntry(PVR_INDEX_HANDLE    handle,PVR_INDEX_ENTRY_S *last_index);

INLINE MT_S32 PVRIndexGetHeaderInfo(MT_S32 s32Fd, PVR_IDX_HEADER_INFO_S* pHeadInfo);
INLINE MT_S32 PVRIndexSaveHeaderInfo(MT_U8 *indexFileName, PVR_IDX_HEADER_INFO_S* pHeadInfo);

MT_S32 PVR_Index_SeekToStart_AndDistance(PVR_INDEX_HANDLE handle,MT_U32 lockidx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifdef __MT_PVR_INDEX_H__ */

