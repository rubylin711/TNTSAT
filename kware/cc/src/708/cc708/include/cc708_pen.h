/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC708_PEN_H__
#define __CC708_PEN_H__

#include "mt_type.h"
#include "mt_cc708_def.h"

#ifdef __cplusplus
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
MT_U8 CC708_Pen_SetPenSize(CC708_PenDef_S *pstPen, MT_U8 u8PenSize, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetFontStyle(CC708_PenDef_S *pstPen, MT_U8 u8FontStyle, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextTag(CC708_PenDef_S *pstPen, MT_U8 u8TextTag, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextOffset(CC708_PenDef_S *pstPen, MT_U8 u8TextOffset, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextItalics(CC708_PenDef_S *pstPen, MT_U8 u8IsItalic, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextUnderline(CC708_PenDef_S *pstPen, MT_U8 u8IsUnderline, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextEdgeType(CC708_PenDef_S *pstPen, MT_U8 u8EdgeType, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextEdgeColor(CC708_PenDef_S *pstPen, MT_U32 u32EdgeColor, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextFgColor(CC708_PenDef_S *pstPen, MT_S32 s32FGColor, MT_S32 s32FGOpacity, MT_U8 u8IsVisible);

MT_U8 CC708_Pen_SetTextBgColor(CC708_PenDef_S *pstPen, MT_S32 s32BGColor, MT_S32 s32BGOpacity, MT_U8 u8IsVisible);

#ifdef __cplusplus
}
#endif
#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/
