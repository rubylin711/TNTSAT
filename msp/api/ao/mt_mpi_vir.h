/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  __MPI_HIAO_VIRTUAL_H__
#define  __MPI_HIAO_VIRTUAL_H__

#include "mt_module_debug.h"
#include "mt_mpi_ao.h"
#include "mt_drv_ao.h"

#define VIR_MIN_OUTBUF_SIZE   (32*1024)
#define VIR_MAX_OUTBUF_SIZE   (512*1024)
#define VIR_MAX_FRAME_SIZE    (32*1024)   //dra multichn   1024*8*sizeof(mt_s32) = 32K

#define CHECK_VIRTUAL_Track(track)                          \
    do {                                                    \
            if(((track & AO_TRACK_CHNID_MASK) >= AO_MAX_REAL_TRACK_NUM) && ((track & AO_TRACK_CHNID_MASK) <= AO_MAX_REAL_TRACK_NUM + AO_MAX_VIRTUAL_TRACK_NUM))              \
            {                                               \
                MT_INFO_AO("Virtual Track don't support this function\n");  \
                return MT_SUCCESS;                          \
            }                                               \
         } while(0)

#define CHECK_REAL_Track(track)                          \
    do {                                                    \
            if((track & AO_TRACK_CHNID_MASK) < AO_MAX_REAL_TRACK_NUM)              \
            {                                               \
                MT_ERR_AO("Real Track don't support this function\n");  \
                return MT_FAILURE;                          \
            }                                               \
         } while(0)

#define CHECK_Track(track)                          \
    do {                                                    \
            if((track & AO_TRACK_CHNID_MASK) >= (AO_MAX_REAL_TRACK_NUM + AO_MAX_VIRTUAL_TRACK_NUM))              \
            {                                               \
                MT_ERR_AO("Virtual Track don't support this function\n");  \
                return MT_FAILURE;                          \
            }                                               \
         } while(0)

#define VIR_MAX_STORED_PTS_NUM   (2*1024)   //(512 *1024) / 320  (VIRTUAL_MAX_OUTBUF_SIZE/MIN_FRAME_SIZE)

typedef struct hiVIR_PTS_S
{
    mt_u32 u32PtsMs;        /* Play Time Stamp  */
    mt_u32 u32BegPtr;      /* Stream start address of PTS */
    mt_u32 u32EndPtr;      /* Stream end   address of PTS */
} VIR_PTS_S;

typedef struct hiVIR_PTS_QUE_S
{
    mt_u32   u32LastPtsMs;
    mt_u32   u32PTSreadIdx;     /* PTS buffer read  ptr */
    mt_u32   u32PTSwriteIdx;    /* PTS buffer write ptr */
    VIR_PTS_S stPTSArry[VIR_MAX_STORED_PTS_NUM];
} VIR_PTS_QUE_S;

typedef struct hiVIR_BUFFUR_S
{
    /*buf ptr info*/
    mt_u8 *      pu8BufBase;
    mt_u32       u32Start;
    mt_u32       u32End;
    ulong       u32Write;
    ulong       u32Read;

    /*buf data info*/
    mt_u32       u32Channel;
    mt_s32       s32BitPerSample;
    mt_u32       u32SampleRate;
    mt_u32       u32PcmSamplesPerFrame;

    VIR_PTS_QUE_S stPTSQue;
}VIR_BUFFUR_S;

typedef struct
{
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr;
    VIR_BUFFUR_S             *pstBuf;
    mt_u32                   u32BufSize;
} VIR_TRACK_STATE_S;



mt_void VIR_InitRS(mt_void);
mt_void VIR_DeInitRS(mt_void);
mt_s32  VIR_CreateTrack(const MT_UNF_AUDIOTRACK_ATTR_S *pstTrackAttr, mt_handle *phTrack);
mt_s32  VIR_DestroyTrack(mt_handle hTrack);
mt_s32  VIR_GetAttr(mt_handle hTrack, MT_UNF_AUDIOTRACK_ATTR_S *pstAttr);
mt_s32  VIR_SendData(mt_handle hTrack, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32  VIR_AcquireFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32  VIR_ReleaseFrame(mt_handle hTrack, MT_UNF_AO_FRAMEINFO_S *pstAOFrame);

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif
#endif
