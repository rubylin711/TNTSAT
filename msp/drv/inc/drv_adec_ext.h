/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_ADEC_EXT_H__
#define __MT_DRV_ADEC_EXT_H__

#include "mt_type.h"
#include "mt_mpi_adec.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

#ifdef MT_MCE_SUPPORT
#define ADEC_KERNEL_DECODE
#endif

#ifdef ADEC_KERNEL_DECODE

typedef enum mtADEC_KERNEL_TYPE_E
{
    ADEC_KEL_TPYE_MPEG,   /**<mpeg */
    ADEC_KEL_TPYE_BUTT
} ADEC_KERNEL_TYPE_E;

typedef struct mtADEC_SLIM_ATTR_S
{
    ADEC_KERNEL_TYPE_E enCodecType;
    mt_u32             u32InBufSize;                  /* Input buffer  size              */
    mt_u32             u32OutBufNum;               /* Output buffer number, buffer size depend on  u32CodecID       */
} ADEC_SLIM_ATTR_S;

mt_s32          MT_DRV_ADEC_Open(mt_handle *phAdec, ADEC_SLIM_ATTR_S *pstAdecAttr);
mt_s32          MT_DRV_ADEC_Close (mt_handle hAdec);
mt_s32          MT_DRV_ADEC_Reset(mt_handle hAdec);
mt_s32          MT_DRV_ADEC_SetAttr(mt_handle hAdec, ADEC_SLIM_ATTR_S *pstAdecAttr);
mt_s32          MT_DRV_ADEC_GetAttr(mt_handle hAdec, ADEC_SLIM_ATTR_S *pstAdecAttr);
mt_s32          MT_DRV_ADEC_SendStream (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u32 u32PtsMs);
mt_s32			MT_DRV_ADEC_ReceiveFrame (mt_handle hAdec, MT_UNF_AO_FRAMEINFO_S *pstAOFrame,
                                       ADEC_EXTFRAMEINFO_S *pstExtInfo);
mt_s32			MT_DRV_ADEC_ReleaseFrame(mt_handle hAdec, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame);
mt_s32			MT_DRV_ADEC_GetStatusInfo(mt_handle hAdec, ADEC_STATUSINFO_S *pstStatusinfo);
mt_s32			MT_DRV_ADEC_Pull(mt_handle hAdec);

#endif

mt_s32          MT_DRV_ADEC_Init(mt_void);
mt_void         MT_DRV_ADEC_DeInit(mt_void);

mt_s32			ADEC_DRV_ModInit(mt_void);
mt_void			ADEC_DRV_ModExit(mt_void);

mt_s32 ADEC_DRV_SYSDATE_SET(mt_sysdate_t *date);
mt_s32 ADEC_DRV_SYSDATE_GET(mt_sysdate_t *date);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif //__MT_DRV_ADEC_EXT_H__
