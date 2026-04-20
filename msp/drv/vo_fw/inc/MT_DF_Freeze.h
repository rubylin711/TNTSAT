/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_DF_FREEZE_H
#define MT_DF_FREEZE_H


#ifdef __cplusplus
extern "C" {
#endif

#include"mt_type.h"

typedef struct tagFreeze_OrderInfo
{
    //data info
    MT_U32 u32SourceAddr;
    MT_U32 u32DstAddr;
    MT_U32 u32Length;
}FREEZE_ORDERINFO;


MT_S32 FZ_Init(MT_VOID);
MT_S32 FZ_Fin(MT_VOID);
MT_S32 FZ_AddOrder(FREEZE_ORDERINFO * porder);
MT_VOID FZ_StopOder(MT_VOID);
MT_BOOL FZ_OrderProcessDone(mt_void);

#ifdef __cplusplus
}
#endif
#endif

