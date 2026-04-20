/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_HAL_CAST_H__
#define __MT_HAL_CAST_H__

#include "mt_type.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define AIAO_CAST_BUFSIZE_MIN 2048
/***************************** Macro Definition ******************************/
typedef enum
{
    AIAO_CAST_0 = 0x00,
    AIAO_CAST_1 = 0x01,
    AIAO_CAST_2 = 0x02,
    AIAO_CAST_3 = 0x03,
    
    AIAO_CAST_BUTT = 0x4,
} AIAO_CAST_ID_E;

typedef enum
{
    AIAO_CAST_STATUS_STOP = 0,
    AIAO_CAST_STATUS_START,
    AIAO_CAST_STATUS_CAST_BUTT,
} AIAO_CAST_STATUS_E;

/* void internal mmz */
typedef struct
{
    phys_addr_t  u32BufPhyAddr;  
    ulong   u32BufVirAddr;  
    mt_u32  u32BufSize;
    mt_u32  u32WptrAddr;
    mt_u32  u32RptrAddr;
} AIAO_CAST_MEM_ATTR_S;

typedef struct
{
    mt_u32  todo;
} AIAO_CAST_RBUF_ATTR_S;

typedef struct
{
    AIAO_CAST_MEM_ATTR_S extDmaMem;
    mt_u32 u32BufBitPerSample; /**<I/O, bit per sampling*//**<CNcomment:OUT. Bit per sample */
    mt_u32 u32BufChannels; /**<I/O, number of channels*//**<CNcomment:OUT. 输出声道数  */
    mt_u32 u32BufSampleRate; /**<I/O, sampling rate*//**<CNcomment:OUT. 输出采样频率 */
    mt_u32 u32BufDataFormat;          /**<I/O, 0, linear pcm, 1, iec61937 */
    mt_u32 u32BufLatencyThdMs;   /* 40 ~ 1000 ms */

} AIAO_CAST_ATTR_S;


/* global function */
mt_s32					HAL_CAST_Init(mt_void);
mt_void					HAL_CAST_DeInit(mt_void);

/* port function */
mt_s32					HAL_CAST_Create(AIAO_CAST_ID_E *penCast, AIAO_CAST_ATTR_S *pstAttr);
mt_void					HAL_CAST_Destroy(AIAO_CAST_ID_E enCast);
mt_s32					HAL_CAST_SetAttr(AIAO_CAST_ID_E enCast, AIAO_CAST_ATTR_S *pstAttr);
mt_s32					HAL_CAST_GetAttr(AIAO_CAST_ID_E enCast, AIAO_CAST_ATTR_S *pstAttr);
mt_s32					HAL_CAST_Start(AIAO_CAST_ID_E enCast);
mt_s32					HAL_CAST_Stop(AIAO_CAST_ID_E enCast);
//mt_s32					HAL_CAST_GetStatus(AIAO_CAST_ID_E enCast, mt_void *pstStatus);

/* port buffer function */
mt_u32                  HAL_CAST_ReadData(AIAO_CAST_ID_E enCAST, mt_u32 * pu32DataOffset, mt_u32 u32DestSize);
mt_u32                  HAL_CAST_ReleaseData(AIAO_CAST_ID_E enCAST, mt_u32 u32DestSize);
mt_u32					HAL_CAST_QueryBufData(AIAO_CAST_ID_E enCast);
mt_u32					HAL_CAST_QueryBufFree(AIAO_CAST_ID_E enCast);
mt_u32				    HAL_CAST_WriteData(AIAO_CAST_ID_E enCast, mt_u8 * pu32Src, mt_u32 u3SrcLen);
mt_u32				    HAL_CAST_GetDelayMs(AIAO_CAST_ID_E enCast, mt_u32 * pu32Delayms); 
mt_s32					HAL_CAST_GetRbfAttr(AIAO_CAST_ID_E enCast, AIAO_CAST_RBUF_ATTR_S *pstRbfAttr);

mt_void HAL_CAST_GetDefAttr(AIAO_CAST_ATTR_S *pstAttr);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_HAL_CAST_H__
