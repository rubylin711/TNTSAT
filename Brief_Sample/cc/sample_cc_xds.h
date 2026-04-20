/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _SAMPLE_CC_XDS_H_
#define _SAMPLE_CC_XDS_H_

#include "mt_type.h"

#if defined __cplusplus || defined __cplusplus__
extern "C" {
#endif

MT_VOID CC608_XDS_Init(MT_VOID);
MT_S32  CC608_XDS_Decode(mt_u8 u8XDSClass, mt_u8 u8XDSType, mt_u8 *pu8Data, mt_u8 u8DataLen);


#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif

