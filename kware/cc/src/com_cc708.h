/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _COM_CC708_H_
#define _COM_CC708_H_

#include "mt_type.h"
#include "cc_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

MT_S32 Com_CC708_Init(MT_VOID);
MT_S32 Com_CC708_Deinit(MT_VOID);
MT_S32 Com_CC708_Create( MT_UNF_CC_PARAM_S *pstCCParam, MT_HANDLE *phCC);
MT_S32 Com_CC708_Destroy(MT_HANDLE hCC);
MT_S32 Com_CC708_Start(MT_HANDLE hCC);
MT_S32 Com_CC708_Stop(MT_HANDLE hCC);
MT_S32 Com_CC708_Reset(MT_HANDLE hCC);

MT_S32 Com_CC708_DtvCC_ParsePicUsrData(MT_U8 *pu8CCPicData, MT_U8 u8CCdataLength, MT_BOOL bCheckATSC608);
MT_S32 Com_CC708_ProcessData(MT_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
