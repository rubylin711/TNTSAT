/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC708_DECODE_H__
#define __CC708_DECODE_H__

#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/


/*****************************************************************************
*                    Structure Definitions
*****************************************************************************/


 /*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/
MT_S32 CC708_DEC_Start(MT_U8 moduleID);

MT_S32 CC708_DEC_Stop(MT_U8 moduleID);

MT_S32 CC708_DEC_Reset(MT_U8 moduleID);

MT_S32 CC708_DEC_UserDataParse(MT_U8 *pu8UserData, MT_U32 u32DataLen, MT_BOOL bTopFieldFirst);

MT_S32 CC708_DEC_ProcessServicefifo(MT_VOID);

MT_S32 CC708_DtvCC_ParsePicUsrData( MT_U8 *pu8CCPicData, MT_U8 u8CCdataLength, MT_BOOL bCheckATSC608);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/