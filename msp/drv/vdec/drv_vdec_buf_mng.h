/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_vdec_buf_mng.h
  Version       : Initial Draft
  Author        : Montage MA-SW
  Created       : 2016/01/06
  Description   : Definitions of buffer manager.
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

*******************************************************************************/

#ifndef __MT_VDEC_BUFFER_MNG_H__
#define __MT_VDEC_BUFFER_MNG_H__

/******************************* Include Files *******************************/

#include "mt_type.h"
#include "mt_error_mpi.h"
#include "mt_drv_video.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/****************************** Macro Definition *****************************/


#define BUFMNG_DEBUG                    (0)
#define BUFMNG_64BITS_PTS_SUPPORT       (0)
#define BUFMNG_MARKER_SUPPORT           (1)
#define BUFMNG_PARTED_FRAME_SUPPORT     (1)
#define BUFMNG_INDEX_SUPPORT            (0)
#define BUFMNG_MULTI_READ_SUPPORT       (1)
#define BUFMNG_GET_SMSP                 (0) /* Get As More As Possible */

/* Error Definition */
#define MT_ERR_BM_INVALID_PARA      (MT_ERR_VDEC_INVALID_PARA)
#define MT_ERR_BM_NO_MEMORY         (MT_ERR_VDEC_MALLOC_FAILED)
#define MT_ERR_BM_BUFFER_FULL       (MT_ERR_VDEC_BUFFER_FULL)
#define MT_ERR_BM_BUFFER_EMPTY      (MT_FAILURE)
#define MT_ERR_BM_FREE_ERR          (MT_FAILURE)
#define MT_ERR_BM_WRITE_FREE_ERR    (MT_FAILURE)
#define MT_ERR_BM_READ_FREE_ERR     (MT_FAILURE)
#define MT_ERR_BM_BUSY              (MT_ERR_VDEC_INVALID_STATE)

#define BUFMNG_NOT_END_FRAME_BIT     (0x00000001)
#define BUFMNG_END_OF_STREAM_BIT    (0x00000002)
#define BUFMNG_DISCONTINUOUS_BIT    (0x00000004)

/*************************** Structure Definition ****************************/

typedef enum tagBUFMNG_ALLOC_TYPE_E
{
    BUFMNG_ALLOC_INNER = 0, /* Allocate by BUGMNG */
    BUFMNG_ALLOC_OUTER,     /* Allocate by user */
    BUFMNG_ALLOC_BUTT
}BUFMNG_ALLOC_TYPE_E;

typedef struct tagBUFMNG_INST_CONFIG_S
{
    BUFMNG_ALLOC_TYPE_E enAllocType;    /* MMZ allocate type */
    phys_addr_t u32PhyAddr;          /* Start physical address. */
    mt_u8* pu8UsrVirAddr;       /* Start user virtual address. */
    mt_u8* pu8KnlVirAddr;       /* Start kernel virtual address. */
    mt_u32 u32Size;             /* Size */

    phys_addr_t u32DescPhyAddr;
    mt_u8* pu8KnlVirDescAddr;
    mt_u32 u32KnlVirDescBufSize;

    mt_char aszName[16];        /* Buffer name */
}BUFMNG_INST_CONFIG_S;

typedef struct tagBUFMNG_BUF_S
{
    phys_addr_t u32PhyAddr;      /* Physical address */
    mt_u8* pu8UsrVirAddr;   /* User virtual address */
    mt_u8* pu8KnlVirAddr;   /* Kernel virtual address */
    mt_u32 u32ESBufLeftSize; /* Es buf left size */
    mt_u32 u32Size;         /* Buffer size, in the unit of byte.*/
    mt_u64 u64Pts;          /* PTS of the data filled in a buffer.*/
    mt_u32 u32Index;        /* Index, always output, don't set */
    mt_u32 u32Marker;       /* bit0: 0:Frame over/1:Half frame; bit1: 1:Stream over */
    mt_u32 u32PtsValide;
    mt_u32 u32FrameFinsh;
    mt_u32 u32PreFrameFinsh;
    mt_u32 u32ScrapSize;
    mt_u32 u32EosFlag;
}BUFMNG_BUF_S;

typedef struct tagBM_STATUS_S
{
    mt_u32 u32Used;
    mt_u32 u32Free;
    mt_u32 u32DataNum;      /* For stream mode, it is undecoded packet number.
                               For frame mode, it is undecoded frame number, support BUFMNG_NOT_END_FRAME_BIT.*/
    mt_u32 u32GetTry;       /* GetWriteBuf try times */
    mt_u32 u32GetOK;        /* GetWriteBuf ok times */
    mt_u32 u32PutTry;       /* PutWriteBuf try times */
    mt_u32 u32PutOK;        /* PutWriteBuf ok times */
    mt_u32 u32RecvTry;      /* GetReadBuf try times */
    mt_u32 u32RecvOK;       /* GetReadBuf ok times */
    mt_u32 u32RlsTry;       /* PutReadBuf try times */
    mt_u32 u32RlsOK;        /* PutReadBuf ok times */

	mt_u32 u32RdPtr;		/* Read Pointer */
	mt_u32 u32WrPtr;		/* Write Pointer */
}BUFMNG_STATUS_S;

/******************************* API Declaration *****************************/

mt_s32 BUFMNG_Init(mt_void);
mt_s32 BUFMNG_DeInit(mt_void);
mt_s32 BUFMNG_Create_forUsrData(mt_handle *phBuf, BUFMNG_INST_CONFIG_S* pstConfig);
mt_s32 BUFMNG_Create(mt_handle hVdec, mt_handle *phBuf, BUFMNG_INST_CONFIG_S* pstConfig,mt_u32 pip_en);
mt_s32 BUFMNG_SetUserAddr(mt_handle hBuf, ulong u32Addr);
mt_s32 BUFMNG_Destroy_forUsrData(mt_handle hBuf);
mt_s32 BUFMNG_Destroy(mt_handle hBuf);
mt_s32 BUFMNG_Get(mt_handle hBuf, BUFMNG_INST_CONFIG_S* pstConfig);
mt_s32 BUFMNG_GetWriteBuffer_forUsrData(mt_handle hBuf, BUFMNG_BUF_S *pstBuf);
mt_s32 BUFMNG_GetWriteBuffer(mt_handle hBuf, BUFMNG_BUF_S *pstBuf);

/** Append SW PTS Descriptor buffer to this ES buffer */
mt_s32 BUFMNG_AppendDescriptor(mt_handle hBuf, mt_char *name, ulong size);

/**
 * @brief Get next SW Descriptor buffer address
 *
 * @param[in] hBuf Buffer handle
 * @param[out] pu32Addr Descriptor buffer address
 *
 * @return
 *      MT_SUCCESS, success
 *      MT_FAILURE, failure
 */
mt_s32 BUFMNG_GetDescWriteBuffer(mt_handle hBuf, ulong *pu32Addr);

/* Can be put by pu8KnlVirAddr or pu8UsrVirAddr */

mt_s32 BUFMNG_PutWriteBuffer_forUsrData(mt_handle hBuf, BUFMNG_BUF_S *pBuf);
mt_s32 BUFMNG_PutWriteBuffer(mt_handle hBuf, BUFMNG_BUF_S *pBuf);
mt_s32 BUFMNG_AcqReadBuffer(mt_handle hBuf, BUFMNG_BUF_S *pBuf);
/* Can be put by pu8KnlVirAddr or pu8UsrVirAddr */
mt_s32 BUFMNG_RlsReadBuffer(mt_handle hBuf, BUFMNG_BUF_S *pBuf);
mt_s32 BUFMNG_Reset(mt_handle hBuf);
mt_s32 BUFMNG_GetStatus(mt_handle hBuf, BUFMNG_STATUS_S* pstStatus);
mt_void BUFMNG_SaveInit(mt_void);
MT_BOOL BUFMNG_CheckFile(mt_s32 Handle, mt_s8 Flag);
mt_s32 BUFMNG_OpenFile(mt_s32 Handle, mt_s8 *FilePath, mt_s8 Flag);
mt_s32 BUFMNG_CloseFile(mt_s32 Handle, mt_s8 Flag);
mt_s32 BUFMNG_SaveRaw(mt_s32 Handle, mt_s8 *Addr, mt_s32 Length);
mt_s32 BUFMNG_SaveYuv(mt_s32 Handle, MT_DRV_VIDEO_FRAME_S *pstFrame,MT_UNF_VCODEC_TYPE_E enType);

#if (BUFMNG_DEBUG==1)
mt_s32 BUFMNG_Debug(mt_handle hBuf);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_VDEC_BUFFER_MNG_H__ */

