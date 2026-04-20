/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_PLAY_CTRL_H__
#define __MT_PVR_PLAY_CTRL_H__

#include "mt_type.h"
#include "mt_pvr_fifo.h"
#include "mt_pvr_index.h"

#include "mt_drv_pvr.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#define PVR_IDX_CACHED_FRMNUM   5120
#define PVR_PLAY_CACHE_NUM  32
#define FRAME_TOP_FIELD  0
#define FRAME_BOT_FIELD  1

#define PVR_PLAY_DMX_GET_BUF_TIME_OUT 5000  /* ms */


/*whether move the short pakcet less than 188 byte to the end of TS or not.
    0:not move, and after that fill it with 0xff
    1:move it, and before that fill it with 0xff 
*/
#define PVR_TS_MOVE_TO_END   1


#define PVR_PLAY_DO_NOT_MARK_DISPLAY 0xffU

#define PVR_PLAY_STEP_WATI_TIME   1000UL  /* ms */

#define PVR_PLAY_MAX_FRAME_SIZE  (1024*1024*10)   /* the size of max frame */

#define PVR_PLAY_PICTURE_HEADER_LEN  4			/* the length of picture header ID, in byte */


#define WHENCE_STRING(whence)   ((0 == (whence)) ? "SEEK_SET" : ((1 == (whence)) ? "SEEK_CUR" : "SEEK_END"))

#define PVR_TIME_CTRL_INTERVAL 1000	

#define PVR_DEFAULT_FRAME_BUFF_NUM  6
#define PVR_VO_FRMBUFF_NUM_OF_DISABLE_DEI  3
#define PVR_VO_FRMBUFF_NUM_OF_ENABLE_DEI   7

#define PVR_ENABLE_DISP_OPTIMIZE  1
#define PVR_DISABLE_DISP_OPTIMIZE 0

/* check channel validity                                                   */
#define PVR_PLAY_CHECK_CHN(u32Chn)\
    do {\
        if (u32Chn >= PVR_PLAY_MAX_CHN_NUM )\
        {\
            MT_ERR_PVR("play chn(%u) id invalid!\n", u32Chn);\
            return MT_ERR_PVR_INVALID_CHNID;\
        }\
        if(g_stPvrPlayChns[u32Chn].enState == MT_UNF_PVR_PLAY_STATE_INVALID)\
        {\
            MT_ERR_PVR("play chn(%u) state(%d) invalid!\n", u32Chn, g_stPvrPlayChns[u32Chn].enState);\
            return MT_ERR_PVR_CHN_NOT_INIT;\
        }\
        if( g_stPlayInit.bInit == MT_FALSE)\
        {\
            MT_ERR_PVR("pvr is not init!\n");\
            return MT_ERR_PVR_NOT_INIT;\
        }\
    }while(0)

/* check play module initialized                                            */
#define PVR_PLAY_CHECK_INIT(pCommAttr)\
    do {\
        if (MT_FALSE == (pCommAttr)->bInit)\
        {\
            MT_ERR_PVR("play not inti yet!\n");\
            return MT_ERR_PVR_NOT_INIT;\
        }\
    }while(0)

#define PVR_PLAY_CHECK_CHN_INIT(enState)\
            do\
            {\
                if (MT_UNF_PVR_PLAY_STATE_INVALID ==  enState )\
                {\
                    return MT_ERR_PVR_CHN_NOT_INIT;\
                }\
            } while (0)

#define PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr)\
                    do\
                    {\
                        if (MT_UNF_PVR_PLAY_STATE_INVALID ==  pChnAttr->enState )\
                        {\
                            PVR_UNLOCK_PLAY(pChnAttr);\
                            return MT_ERR_PVR_CHN_NOT_INIT;\
                        }\
                    } while (0)

/* PVR ts file read.
return pointer offset forward on success.
otherwise, return the file header.*/
#define  PVR_PLAY_READ_FILE(pu8Addr, offset, size, pChnAttr) \
            do \
            {\
                ssize_t  n;\
                if ((n = PVR_PREAD64(pu8Addr, (size), \
                            pChnAttr->s32DataFile, (offset))) == -1)\
                {\
                    if (NULL != &errno)\
                    {\
                        if (EINTR == errno)\
                        {\
                            continue;\
                        }\
                        else if (errno)\
                        { \
                            return MT_ERR_PVR_FILE_CANT_READ;\
                        }\
                        else\
                        {\
                            MT_ERR_PVR("read err1,  want:%u, off:%llu,%x,%x \n", (size), offset,n,errno);\
                            return MT_ERR_PVR_FILE_TILL_END;\
                        }\
                    }\
                }\
                if ((0 == n) && (0 != (size)))\
                {\
                    MT_ERR_PVR("read 0,  want:%u, off:%llu \n", (size), offset);\
                    return MT_ERR_PVR_FILE_TILL_END;\
                }\
           }while(0)


/* common information for play module                                      */
typedef struct mtPVR_PLAY_COMM_S
{
    MT_BOOL bInit;
    MT_S32  s32Reserved;
} PVR_PLAY_COMM_S;

/* the info struction for calculating trick mode rate */
typedef struct mtPVR_TPLAY_SPEED_CTRL_S
{
    MT_U32               u32RefFrmPtsMs;         /* the PTS of reference frame, usually, the first frame PTS*/
    MT_U32               u32RefFrmSysTimeMs;     /* the system time of reference frame output */

}PVR_TPLAY_SPEED_CTRL_S;

/* frame tag from pvr to demux */
typedef struct mtPVR_FRAME_TAG
{
    MT_U32          u32DispEnableFlag;       
    MT_U32          u32DispFrameDistance;   
    MT_U32          u32DistanceBeforeFirstFrame;
    MT_U32          u32GopNum;
} PVR_FRAME_TAG_S;

typedef struct mtPVR_SMOOTH_PARA
{
    MT_U32  u32StartCtrlTimeInMs;
    MT_U32  u32LastCtrlTimeInMs;
    MT_U32  u32StartCtrlPtsInMs;
    MT_U32  u32FrameNumAfterLastDisp;
    MT_U32  u32GopCnt;
    MT_UNF_PVR_PLAY_SPEED_E enBackwardLastSpeed;
    MT_U32 u32TimeCtrlSkipFlag;
    MT_UNF_PVR_PLAY_SPEED_E enSmoothLastSpeed;
    MT_U32 u32BackCount;
    MT_U32 u32BackLastVORate;
    MT_U32 u32BackAverageVORate;	
}PVR_SMOOTH_PARA_S;

#ifdef PVR_PROC_SUPPORT
typedef struct mtPVR_PLAY_FF_PROC_S
{
    MT_U32          u32TimeCtrlFindFrm;
    MT_U32          u32TimeCtrlCurFrm;
    MT_U32          u32FirstFrm;
    MT_U32          u32TryFrmNum;
    MT_U32          u32NextIFrm;
    MT_U32          u32TotalFrmNum;
    MT_U32          u32TotalIFrmNum;
    MT_U32          u32TotalPFrmNum;
    MT_U32          u32TotalBFrmNum;
    MT_U32          u32NextTimeStartFrm;
}PVR_PLAY_FF_PROC_S;

typedef struct mt_PVR_PLAY_FB_PROC_S
{
    MT_U32          u32OptimizeFlg;
    MT_U32          u32DispDistance;
    MT_U32          u32SupportMaxGopSize;
    MT_U32          u32FirstFrm;
    MT_U32          u32TotalFrmNum;
    MT_U32          u32TotalGopNum;
    MT_U32          u32TotalPFrmNum;
    MT_U32          u32TotalBFrmNum;
}PVR_PLAY_FB_PROC_S;

/* attributes of play channel                                               */
typedef struct mtPVR_PLAY_CHN_PROC_S
{
    MT_U32          u32PrintFlg;
    MT_U32          u32ChipId;
    MT_U32          u32ChipVer;
    MT_U32          u32Width;
    MT_U32          u32Heigth;
    MT_U32          u32DecAblity;
    MT_U32          u32OrigFrmRate;
    MT_U32          u32FieldFlg;
    MT_U32          u32SetFrmRateInt;
    MT_U32          u32SetFrmRateDec;
    PVR_PLAY_FF_PROC_S stFFCtrlParameter;
    PVR_PLAY_FB_PROC_S stFBCtrlParameter;
} PVR_PLAY_PROC_S;
#endif

typedef enum mtPVR_PLAY_SEND_DATA_E
{
    PVR_PLAY_SEND_DATA_ALL,
	PVR_PLAY_SEND_DATA_IP,
	PVR_PLAY_SEND_DATA_I,
	PVR_PLAY_SEND_DATA_SKIP_I,
} PVR_PLAY_SEND_DATA_E;


typedef struct mtPVR_PLAY_POLICY_S
{
	MT_UNF_AVPLAY_TPLAY_DIRECT_E    enTplayDirect;      /**<Tplay direction*//**<CNcomment: TPLAY·½Ïò */
	MT_UNF_VCODEC_FRMRATE_S stDispFrc;
	PVR_PLAY_SEND_DATA_E enSendDataMode;
  MT_U32 u32SkipPCnt_EveryI;                  //every I discard x p
  MT_U32 u32SkipPMore_EveryxI;                //every x I discard more p
} PVR_PLAY_POLICY_S;

typedef struct mtPVR_PlAY_FRAMES
{
    MT_U32  frameid;
    MT_U32  size;
    MT_U32  tpts;   //true pts
    MT_U32  dpts;   //disp pts
    MT_U64  offset;   //offset in file
}PVR_PlAY_FRAMES;
/* attributes of play channel                                               */
typedef struct mtPVR_PLAY_CHN_S
{
    MT_U32           u32magic1;
    MT_U32           line_magc1;
    pthread_mutex_t  stMutex;
    pthread_mutex_t  stMutex_valid;
    MT_U32           line_magc2;
    MT_U32           u32magic2;
    MT_U32           u32chnID;

    MT_HANDLE        hAvplay;                 /* avplay handle */
    MT_HANDLE        hTsBuffer;               /* TS buffer handle */
    MT_HANDLE        hCipher;                 /* cipher handle */
    MT_HANDLE        hCipher_slot;            /* cipher handle */
    MT_U8            hCipher_head[32];          //head buffer
    MT_U8            hCipher_tail[32];          //tail buffer
    MT_U32           chiptype;                  //chip type.sym1 sym2
    MT_U8            *hCipher_buffer;
    mt_mmz_buf_s    hsec_mmz_ply;
    PVR_INDEX_HANDLE IndexHandle;             /* index handle */

    MT_UNF_PVR_PLAY_ATTR_S  stUserCfg;               /* play attributes for user configure */

    MT_U64           u64LastSeqHeadOffset;    /* last sequence offset */

    MT_UNF_PVR_PLAY_STATE_E enState;                 /* play state */
    MT_UNF_PVR_PLAY_STATE_E enLastState;                 /* last play state */
    MT_UNF_PVR_PLAY_SPEED_E enSpeed;

    PVR_FILE64       s32DataFile;             /* descriptor of play file */
    MT_U64           u64CurReadPos;           /* current data file read position */
    PVR_PHY_BUF_S    stCipherBuf;             /* cipher buffer for data decrypt */
    MT_BOOL          bCAStreamHeadSent;

    pthread_t        PlayStreamThread;        /* play thread id   */
    MT_BOOL          bQuickUpdateStatus;      /* new play status incoming */
    MT_BOOL          bPlayMainThreadStop;
    MT_BOOL          bEndOfFile;             /* playing to EOF */
    MT_BOOL          bTillStartOfFile;       /* TRUE: reach to the start of file, FALSE: reach to the end of the file,  used together with bEndOfFile */
    MT_BOOL          bTsBufReset;

    MT_BOOL          bPlayingTsNoIdx;
    MT_U64           u64TsFileSize;          /* the size of ts file, to control the end for playing without index file */

    MT_U32           u32LastEsBufSize;
    MT_U32           u32LastPtsMs;
    PVR_TPLAY_SPEED_CTRL_S stTplayCtlInfo;      /* control info for trick mode */
    MT_UNF_PVR_PLAY_STATUS_S stLastStatus;     /* the last play status, when failure to get current play status, return this */
    ExtraCallBack     readCallBack;
    MT_U32           u32FrmNum;
    MT_U32           u32VoFrmNum;
    MT_UNF_PVR_PLAY_SPEED_E enFBTimeCtrlLastSpeed;
    MT_U32           u32CurPlayTimeMs;
    MT_U64           u64CurPlayPosition;
    MT_UNF_AVPLAY_CONTROL_INFO_S  stVdecCtrlInfo;
    MT_U32           u32GopNumOfStart;
    PVR_FRAME_TAG_S  stFrmTag;
    PVR_SMOOTH_PARA_S stSmoothPara;
    MT_BOOL          bTimeShiftStartFlg;
	MT_BOOL          bRecordedVideoExist;        /*use for judgement of call avplay invoke with cmd = MT_UNF_AVPLAY_INVOKE_VCODEC and some other vedio corresponede cmd*/
	MT_BOOL          bNotAvailableTsBuff;
	//MT_BOOL          bSendRewindFail;
	//MT_U32           u32FailToSendTimes;
    MT_UNF_SYNC_REF_E enLastSyncState;
#ifdef PVR_PROC_SUPPORT
    PVR_PLAY_PROC_S stPlayProcInfo;
#endif
	PVR_PLAY_SEND_DATA_E enSendDataMode;
  MT_U32 u32SkipPCnt_EveryI;                  //every I discard x p
  MT_U32 u32SkipPMore_EveryxI;                //every x I discard more p.
  MT_U32 u32PushAddPoint;                     //push a Iframe, add a point. if more than 1, do action
  MT_U32 u32PushCount_Print;                  //numbers of pushed frame,for print
	MT_UNF_AVPLAY_TPLAY_DIRECT_E	enTplayDirect;
	MT_U32 u32FrameRate;
  MT_U32 u32FrameRate_fromav;
  MT_U32 u32FrameRate_trick;
	MT_U32 u32IPCnt;
	MT_U32 u32ICnt;
  MT_U32 fieldtype;              //get from index start pts
  //PVR_INDEX_ENTRY_S  idxcache[PVR_PLAY_CACHE_NUM];  //cache frames to discard some p frame
  MT_U32             idxwp;
  //MT_U32             idxrp;
  //MT_U32             idxmax;    //mustbe 2^n.mask=idxmax-1
  //MT_U8   *diocache256k;  //dio cache 256k
  //MT_U64  fstartdpos;     //start offset in file
  //MT_U64  fendpos;        //end offset in file
  PVR_PlAY_FRAMES   *framecache;//[PVR_IDX_CACHED_FRMNUM];
  MT_U32            framewp;
  MT_U32            framesz;
  MT_U64            vframe_start_glb;   //vir-frame'start pos in globe. merge some true-frame to vir-frame.
  MT_U64            vframe_start;   //vir-frame'start pos in file. merge some true-frame to vir-frame.
  MT_U32            vframe_size;    //vir-frame'size
  MT_U32            vframe_reseted; //vir-frame reseted
  MT_U32            play_errcode;   //read data error
  MT_U32            play_status_disptime;   //display time
  MT_U32            current_playing_frame;  //get from pts
  MT_U32            play_real_pushed;       //pushed
  MT_U32            first_push;
  MT_U32            pause2resume_forirdeto; //resume replace stop start
  MT_U32            start_timeoffset;//for irdeto pvr play from time offset(ms)
  MT_U32			set_eof;				//set eof
  MT_BOOL          data_send_over; // data send start or end of file
  MT_BOOL         bEofSeek2Start;
} PVR_PLAY_CHN_S;




MT_S32 MT_PVR_PlayRegisterReadCallBack(MT_U32 u32Chn, ExtraCallBack readCallBack);

MT_S32 MT_PVR_PlayUnRegisterReadCallBack(MT_U32 u32Chn);

MT_BOOL PVR_Play_IsFilePlayingSlowPauseBack(const MT_CHAR *pFileName);
MT_BOOL PVR_Play_IsPlaying(void);

void PVRPlaySyncTrickPlayTime(PVR_PLAY_CHN_S *pChnAttr);
MT_S32 MT_PVR_ChangeAudioPid(MT_U32 u32Chn, MT_BOOL aorv, MT_U32 type, MT_U32 pid);

MT_S32 MT_Pvr_Play_Read_File(MT_U8 *pu8Addr, MT_U64 offset, MT_U32 size, PVR_PLAY_CHN_S *pChnAttr);
MT_S32 MT_PVR_TPlayPauseResume(MT_U32 u32Chn, MT_BOOL bPause);
MT_S32 MT_PVR_GetTSBufferStatus(MT_U32 u32ChnID, MT_UNF_DMX_TSBUF_STATUS_S *Status);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifdef __MT_PVR_PLAY_CTRL_H__ */

