/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_PVR_REC_CTRL_H__
#define __MT_PVR_REC_CTRL_H__

#include "mt_pvr_priv.h"
#include "mt_pvr_index.h"
#include "mt_pvr_fifo.h"

#include "mt_drv_pvr.h"
#include "mt_mpi_demux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


enum PTS_LOOP_STATUS {
	PTS_OVER_LOOP = 0,    //normal. over pts reloop
       PTS_CROSS_LOOP,
};

#define PVR_REC_MIN_DMXID 0
#define PVR_REC_MAX_DMXID 4

#define PVR_REC_DMX_GET_SC_TIME_OUT     1000  /* ms */
#define PVR_REC_DMX_GET_STREAM_TIME_OUT 1500  /* ms */

#define PVR_REC_APPEND_LEN  (PVR_TS_LEN * 2)
#define PVR_REC_MAX_PID        (24)
#define PVR_REC_MAX_EVENT    (256)
#define PVR_REC_LOOPTIME_IS_0_MAX_EVENT       (2)
#define PVR_REC_MAX_NODE     (0xffff)

#define CHECK_REC_CHNID(u32ChnID)\
    do\
    {\
        if ((u32ChnID < PVR_REC_START_NUM) || (u32ChnID >= (PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM)))\
        {\
           MT_ERR_PVR("Rec chn(%u) id invalid!\n", u32ChnID);\
           return MT_ERR_PVR_INVALID_CHNID;\
        }\
        if(g_stPvrRecChns[u32ChnID - PVR_REC_START_NUM].enState == MT_UNF_PVR_REC_STATE_INVALID)\
        {\
            MT_ERR_PVR("Rec chn(%u) state(%d) invalid!\n", u32ChnID, g_stPvrRecChns[u32ChnID - PVR_REC_START_NUM].enState);\
            return MT_ERR_PVR_CHN_NOT_INIT;\
        }\
        if( g_stRecInit.bInit == MT_FALSE)\
        {\
            MT_ERR_PVR("pvr rec is not init!\n");\
            return MT_ERR_PVR_NOT_INIT;\
        }\
    } while (0)

#define CHECK_REC_INIT(pCommAttr)\
    do\
    {\
        if ((pCommAttr)->bInit != MT_TRUE)\
        {\
            MT_ERR_PVR("Record Module is not Initialized!\n");\
            return MT_ERR_PVR_NOT_INIT;\
        }\
    } while (0)

#define CHECK_REC_DEMUX_ID(DemuxID)\
    do\
    {\
        if (DemuxID > PVR_REC_MAX_DMXID || DemuxID < PVR_REC_MIN_DMXID)\
        {\
            return MT_ERR_PVR_REC_INVALID_DMXID;\
        }\
    } while (0)

#define CHECK_REC_CHN_INIT(enState)\
    do\
    {\
        if (MT_UNF_PVR_REC_STATE_INVALID ==  enState )\
        {\
            return MT_ERR_PVR_CHN_NOT_INIT;\
        }\
    } while (0)

#define CHECK_REC_CHN_INIT_UNLOCK(pRecChnAttr)\
            do\
            {\
                if (MT_UNF_PVR_REC_STATE_INVALID ==  pRecChnAttr->enState )\
                {\
                    PVR_UNLOCK_REC(pRecChnAttr);\
                    return MT_ERR_PVR_CHN_NOT_INIT;\
                }\
            } while (0)


/* common information for record module                                     */
typedef struct mtPVR_REC_COMM_S
{
    MT_BOOL             bInit ;                             /* module init flag */
    MT_S32              s32Reserved;                        /* reserved */
} PVR_REC_COMM_S;

typedef struct mtPVR_SCD_INFO_S
{
    MT_U32 ScdCnt;
    DMX_IDX_DATA_S ScdBuffer[20];
}PVR_SCD_INFO_S;

typedef enum mtPVR_REC_PID_STATUS_S
{
    PVR_REC_PID_INIT = 0,
    PVR_REC_PID_PRE_ADD,
    PVR_REC_PID_ADDED,
    PVR_REC_PID_MAX
}PVR_REC_PID_STATUS_S;

typedef struct mtPVR_REC_PID_S
{
    MT_U32 DmxRecId;
    MT_U32 pid;
    MT_UNF_DMX_CHAN_TYPE_E ChnType;
    PVR_REC_PID_STATUS_S status;
}PVR_REC_PID_S;

typedef struct mtTimeShift_REC_EVENT_S
{
    PVR_EVENT_STATUS_E  eventState;                       
    MT_U8 u8EventName[PVR_MAX_FILENAME_LEN];
    MT_U32  u32EventStartTime;
    PVR_TIMESHIFT_EVENT_TYPE  eventType;
    MT_BOOL bSaved;
    MT_U32 fd_idx;
    MT_U32 lst_dsptime;
    MT_U64 u64StartFrame;
    MT_U64 u64EndFrame;    
}TimeShift_REC_EVENT_S;


typedef struct mtPVR_EVENT_S
{
    MT_UNF_PVR_REC_CA_TYPE_E  caType;
    MT_UNF_PVR_REC_STATE_E  state;
    MT_U32 event_total_count;
    TimeShift_REC_EVENT_S  *event_rec;//[PVR_REC_MAX_EVENT];
    TimeShift_REC_EVENT_S  g_current_timeshift_rec_event;
}PVR_EVENT_S;


typedef struct mtPVE_COPY_SHARE_S
{
    MT_U32  flg_share;                  //share flag
    MT_U32  flg_masterorslave;          //master:1,salve:0
    MT_U32  Shared_recchid;             //peer's chid
    void  *Shared_recchptr;             //peer's pointer
    MT_U64  offset_inrecbuf;            //offset in globle-recording buffer
    MT_U32  can_stop;                   //can stop,default=0
    MT_U32 time_offset_ms;
}PVE_COPY_SHARE_S;

/* attributes of record channel                                             */
typedef struct mtPVR_REC_CHN_S
{
    MT_U32                  u32magic1;
    MT_U32                  line_magc1;
    pthread_mutex_t         stMutex;
    pthread_mutex_t         stMutex_valid;
    MT_U32                  u32ChnID;
    MT_U32                  line_magc2;
    MT_U32                  u32magic2;

    MT_HANDLE               hCipher;                   /* cipher handle */
    MT_U8                   *hCipher_buffer;
    mt_mmz_buf_s            hsec_mmz_rec;
    MT_U32                  chiptype;                  //1.sym1 or 2.sym2
    PVR_INDEX_HANDLE        IndexHandle;               /* index handle */

    MT_UNF_PVR_REC_ATTR_S   stUserCfg;                 /* record attributes for user configure */

    MT_U32                  u32Flashlen;
    volatile MT_U64         u64CurFileSize;            /* current size of record file, included rewind */
    MT_UNF_PVR_REC_STATE_E  enState;                   /* record state */
    PVR_FILE64              dataFile;                  /* descriptor of record file */

    pthread_t               RecordIndexThread;         /* record thread pids */
    pthread_t               RecordWIdxThread;         /* record thread pids */
    pthread_t               RecordStreamThread;

    MT_S32                  s32OverFixTimes;
    MT_BOOL               bEventFlg;                 /* Event flag */

    struct timespec      tv_start;
    struct timespec      tv_stop;
        
    ExtraCallBack         writeCallBack;
    MT_U32                  u32RecStartTimeMs;
    MT_HANDLE            DemuxRecHandle;
    PVR_REC_PID_S     rec_pid[PVR_REC_MAX_PID];
    mt_handle              rec_pidhandle[PVR_REC_MAX_PID];
    MT_S32                  rec_handles;
    PVR_EVENT_S         timeshiftEventHandle;
    MT_U32                  hardware_rec_buffer_rp;
    MT_U32                  encrypt_rec_buff_wp;
    MT_U32                  encrypt_rec_buff_rp;
    MT_U32                  real_write_data_len;
    MT_U32                  real_file_start_frame;
    MT_U64                  re_rec_firstsize;           //rec in exist file
    MT_U32                  First_RawTime;              //default=0xffffffff.
    MT_U32                  Last_IdxTime;               //time of last index
    MT_U32                  timeShiftEventTsFileFirstNode;
	PVE_COPY_SHARE_S     share_rec;
} PVR_REC_CHN_S;

extern MT_BOOL PVR_Rec_IsFileSaving(const MT_CHAR *pFileName);
extern MT_BOOL PVR_Rec_IsChnRecording(MT_U32 u32ChnID);
extern MT_S32 PVR_Rec_MarkPausePos(MT_U32 u32ChnID);
extern MT_BOOL PVR_Rec_IsRecording(void);
extern MT_S32 MT_PVR_CreateIdxFile2(const MT_CHAR* pstTsFileName, MT_CHAR* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr);
MT_S32 MT_PVR_RecRegisterWriteCallBack(MT_U32 u32ChnID, ExtraCallBack writeCallBack);
MT_S32 MT_PVR_RecUnRegisterWriteCallBack(MT_U32 u32ChnID);
PVR_REC_CHN_S* PVRRecGetChnAttrByName(const MT_CHAR *pFileName);

extern ssize_t PVR_PWRITE64(PVR_REC_CHN_S *pRecChn, const void *pMem,  MT_U32 size, PVR_FILE64 file, MT_U64 offset);
MT_S32 MT_PVR_Get_Last_index(MT_UNF_DMX_REC_INDEX_S *last_index);
MT_S32 MT_PVR_StopCache_Event_Rec(MT_U32 u32ChnID);
MT_S32 MT_PVR_Save_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName,  MT_BOOL  internalCall);
MT_S32 MT_PVR_Stop_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName);
MT_S32 MT_PVR_Start_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName, MT_U32    u32StartTimeMs,  PVR_TIMESHIFT_EVENT_TYPE  type);
MT_S32 MT_PVR_Update_Index_Event_Rec(MT_U32 u32ChnID, MT_U8 *pu8Data, MT_U32 u32Bytes2Write);
MT_S32 MT_PVR_Delete_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pTimeShiftPath, MT_CHAR *pEeventName);

MT_S32 MT_PVR_AddDelPid_Ex(mt_u32 chnId, mt_u32 u32DmxId, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType,MT_BOOL flag_adddel,MT_BOOL flag_index);
MT_S32 MT_PVR_Get_Pids(mt_u32 chnId, int *pid, int *num);

MT_S32 MT_PVR_RecCopyChn(MT_U32 *pu32ChnID, MT_U32 u32_SRC_ChnID,const MT_UNF_PVR_REC_ATTR_S *pstRecAttr);
 
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifdef __MT_PVR_H__ */

