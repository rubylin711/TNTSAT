/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef _CC608_H_
#define _CC608_H_

#include "ccdisp_api.h"
#include "mt_type.h"
#include "mt_cc608_def.h"

#if defined __cplusplus || defined __cplusplus__
extern "C" {
#endif

/*******************************************************************************
* CC module Main screen structure
*******************************************************************************/
typedef struct tag_CC608_ELEMENT_S
{
    MT_U8 *     pu8Buffer;
    MT_U16      u16BufferSize;
    MT_U32      u32DataSize;
    MT_U16      u16Flags;
}CC608_ELEMENT_S;

/*****************************************************************************
*                    Extern Data Declarations
*****************************************************************************/

 /*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/
MT_S32 CC608_Init(MT_VOID);
MT_S32 CC608_DeInit(MT_VOID);
MT_S32 CC608_Create(MT_U8 moduleid);

MT_S32 CC608_Destroy(MT_U8 moduleid);
MT_S32 CC608_Config(MT_U8 moduleid,MT_UNF_CC_608_CONFIGPARAM_S *pstCC608Config);
MT_S32 CC608_GetConfig(MT_U8 moduleid,MT_UNF_CC_608_CONFIGPARAM_S *pstCC608ConfigParam);
MT_U8  CC608_IsStart(MT_U8 moduleid);
MT_S32 CC608_Start(MT_U8 moduleid);
MT_S32 CC608_Stop(MT_U8 moduleid);
MT_S32 CC608_Reset(MT_U8 moduleid);

MT_S32 CC608_VBIParse(MT_U8 moduleid, MT_U8 *pu8CCData, MT_U8 u8CCCount);
MT_S32 CC608_Decode(MT_U8 module_id,MT_U16 *pu16CCData,  MT_U8 u8FieldNum);
MT_S32 CC608_ConvertCC608Char2Unicode(MT_U8 *pu8CCChars, MT_U32 u32Len, MT_U16 *pu16UniChars);
void  CC608_XDSReset(MT_U8 moduleid);

#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/
