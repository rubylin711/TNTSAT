/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _CC608_DATA_H_
#define _CC608_DATA_H_

#include "mt_type.h"
#include "mt_cc608_def.h"
#include "mt_unf_cc.h"

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

void CC608_DATA_Create(MT_U8 module_id);
void CC608_DATA_Reset(MT_U8 module_id);

/*caption and it's attributes functions*/
void CC608_DATA_SetBgColor(MT_U8 module_id, MT_U32 u32BgColor, MT_UNF_CC_OPACITY_E enBgOpct);
void CC608_DATA_SetTextAttr(MT_U8 module_id, MT_U32 u32TxtColor, MT_U8 u8IsUnderline);
void CC608_DATA_GetTextAttr(MT_U8 module_id, MT_U32 *pu32TxtColor, MT_U8 *pu8IsUnderline);
void CC608_DATA_SetTextItalic(MT_U8 module_id, MT_U8 u8IsItalic);

void CC608_DATA_SetDefaultAttr(MT_U8 module_id);

void CC608_DATA_Caption(MT_U8 module_id, MT_U16 *pu16CaptionData, MT_U8 u8Len);

void CC608_DATA_SetRow(MT_U8 module_id, MT_U8 u8Row);
void CC608_DATA_SetColumn(MT_U8 module_id, MT_U8 u8Column);
void CC608_DATA_SetDefaultRowColumn(MT_U8 module_id, CC608_MODE_E enMode, CC608_DISP_STYLE_E enStyle);
MT_S32 CC608_DATA_CheckRowColumnValidation(MT_U8 module_id);
void CC608_DATA_SetPacCode(MT_U8 module_id, MT_U8 u8Pac);

MT_S32 CC608_DATA_Substitute(MT_U8 module_id);

/*control code command*/
MT_S32 CC608_DATA_ResumeCL(MT_U8 module_id);   /*Resume caption loading, enter pop on style*/
MT_S32 CC608_DATA_BackSpace(MT_U8 module_id);  /*backspace*/
MT_S32 CC608_DATA_AlarmOff(MT_U8 module_id);   /*reserved */
MT_S32 CC608_DATA_AlarmOn(MT_U8 module_id);    /*reserved */
MT_S32 CC608_DATA_DeleteToER(MT_U8 module_id); /*Delete to End of Row*/
MT_S32 CC608_DATA_Rollup(MT_U8 module_id, MT_U8 u8RollupRows);  /*rollup text rows*/
MT_S32 CC608_DATA_FlashOn(MT_U8 module_id, MT_U8 u8IsFlash);  /*caption flash on*/
MT_S32 CC608_DATA_ResumeDC(MT_U8 module_id);        /*Resume Direct Captioning, Enter Paint-on style*/


/*text mode functions*/
MT_S32 CC608_DATA_TextRestart(MT_U8 module_id);   /*text mode start*/
MT_S32 CC608_DATA_TextDisplay(MT_U8 module_id);   /*display text*/

MT_S32 CC608_DATA_EraseDM(MT_U8 module_id);       /*erase displayed memory(on screen)*/
MT_S32 CC608_DATA_CarriageReturn(MT_U8 module_id); /*change line*/
MT_S32 CC608_DATA_EraseNM(MT_U8 module_id);        /*Erase non-display memory*/
MT_S32 CC608_DATA_FlipMemory(MT_U8 module_id);     /*end of caption*/
MT_S32 CC608_DATA_Tab(MT_U8 module_id, MT_U8 u8Columns);

void CC608_DATA_ResumeLastAttr(MT_U8 module_id); /*set the last attr of one row*/

MT_S32 CC608_DATA_ClearSTA(MT_U8 module_id);

MT_S32 CC608_TimeoutErase_TimerInit(MT_U8 module_id);
void   CC608_TimeoutErase_TimerStop(MT_U8 module_id);
MT_S32 CC608_TimeoutErase_Start(MT_U8 module_id, MT_U32 u32Seconds);

#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif

/*****************************************************************************
*                    End Of File

*****************************************************************************/
