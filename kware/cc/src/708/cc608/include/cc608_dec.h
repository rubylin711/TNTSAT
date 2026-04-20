/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef _CC608_DEC_H_
#define _CC608_DEC_H_

#include "mt_type.h"

#if defined __cplusplus || defined __cplusplus__
extern "C" {
#endif

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/

/*****************************************************************************
*                    Type Definitions
*****************************************************************************/

/*****************************************************************************
*                    Extern Data Declarations
*****************************************************************************/

 /*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/

void CC608_DEC_PAC(MT_U8 moduleID,MT_U8 b1,MT_U8 b2,MT_U8 u8Flag);
void CC608_DEC_Color(MT_U8 moduleID, MT_U8 b1, MT_U8 b2);
void CC608_DEC_MidRow(MT_U8 moduleID,MT_U8 b1, MT_U8 b2);
void CC608_DEC_Command(MT_U8 moduleID, MT_U8 u8IsCaption, MT_U8 b1, MT_U8 b2);
MT_U16 CC608_DEC_ExtCharSet(MT_U8 moduleID, MT_U16 u16LastChar,MT_U8 b1, MT_U8 b2);
MT_S32 CC608_DEC_Decode(MT_U8 module_id,MT_U16 *pu16CCData,  MT_U8 u8FieldNum);
MT_S32 CC608_DEC_CheckXDS(MT_U8 moduleID, MT_U16 *pu16CCData,  MT_U8 u8FieldNum);
MT_S32 CC608_DEC_Convert2Unicode(MT_U8 *pu8CCChars, MT_U32 u32Len, MT_U16 *pu16UniChars);

#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif //#ifndef _CC608DEC_H

/*****************************************************************************
*                    End Of File
*****************************************************************************/







