/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __VPSS_MT_H__
#define __VPSS_MT_H__

#include "mt_drv_video.h"
#include "vpss_common.h"
#include "vpss_osal.h"


typedef struct
{
    mmz_buffer_s stMBuf;
    mt_u32 u32MadMvAddr[3];
}HIS_MAD_MEM_S;

typedef struct
{
    HIS_MAD_MEM_S stMadMtInfo;
}VPSS_MT_INFO_S;

typedef struct
{
    mt_u32 u32WPhyAddr;
    mt_u32 u32RPhyAddr;
    mt_u32 u32Stride;
}VPSS_MT_ADDR_S;

mt_s32 VPSS_MT_Init(VPSS_MT_INFO_S *pstMtInfo);

mt_s32 VPSS_MT_DeInit(VPSS_MT_INFO_S *pstMtInfo);

mt_s32 VPSS_MT_GetAddr(VPSS_MT_INFO_S *pstMtInfo,VPSS_MT_ADDR_S *pstAddr);
#endif
