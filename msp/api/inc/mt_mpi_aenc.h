/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AENC_H__
#define __MT_AENC_H__

#include "mt_type.h"
#include "mt_unf_sound.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
typedef struct hiAENC_ATTR_S
{
    mt_u32               u32CodecID;
    mt_u32               u32InBufSize;               /* Input buffer  size              */
    mt_u32               u32OutBufNum;             /* Output buffer number, buffer size depend on  u32CodecID       */
    MT_HAENCODE_OPENPARAM_S sOpenParam;
} AENC_ATTR_S;

/* Inputting audio stream structure                                                     */
typedef struct hiAENC_STREAM_S
{
    mt_u8  *pu8Data;
    mt_u32  u32Bytes;
    mt_u32  u32PtsMs;
} AENC_STREAM_S;


mt_s32 MT_MPI_AENC_RegisterEncoder(const mt_char *pszCodecDllName);
mt_s32 MT_MPI_AENC_SetConfigEncoder(mt_handle hAenc, mt_void *pstConfigStructure);
mt_s32 MT_MPI_AENC_Init(const mt_char* pszCodecNameTable[]);
mt_s32 MT_MPI_AENC_DeInit(mt_void);
mt_s32 MT_MPI_AENC_Open(mt_handle *phAenc, const MT_UNF_AENC_ATTR_S *pstAencAttr);
mt_s32 MT_MPI_AENC_Close (mt_handle hAenc);
mt_s32 MT_MPI_AENC_SendBuffer(mt_u32 hAenc, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32 MT_MPI_AENC_ReceiveStream(mt_handle hAenc, AENC_STREAM_S *pstStream, mt_u32 u32TimeoutMs);
mt_s32 MT_MPI_AENC_ReleaseStream(mt_handle hAenc, const AENC_STREAM_S *pstStream);
mt_s32 MT_MPI_AENC_SetEnable(mt_handle hAenc, MT_BOOL bEnable);
mt_s32 MT_MPI_AENC_AttachInput(mt_handle hAenc, mt_handle hSource);
mt_s32 MT_MPI_AENC_GetAttachSrc(mt_handle hAenc, mt_handle *hSrc);
mt_s32 MT_MPI_AENC_DetachInput(mt_handle hAenc);
mt_s32 MT_MPI_AENC_SetAttr(mt_handle hAenc, const MT_UNF_AENC_ATTR_S *pstAencAttr);
mt_s32 MT_MPI_AENC_GetAttr(mt_handle hAenc, MT_UNF_AENC_ATTR_S *pstAencAttr);


#if 0
mt_s32 MT_MPI_AENC_ShowRegisterEncoder(mt_void);
mt_s32 MT_MPI_AENC_SetAutoSRC(mt_handle hAenc, MT_BOOL bEnable);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_AENC_H__ */

