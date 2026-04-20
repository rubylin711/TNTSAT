/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*******************************************************************************
*                       Include files
*******************************************************************************/
#include "mt_cc708_def.h"
#include "cc708_pen.h"

#include "mt_type.h"
#include "cc_debug.h"


MT_U8 CC708_Pen_SetPenSize(CC708_PenDef_S *pstPen, MT_U8 u8PenSize, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->enFontSize != u8PenSize )
    {
        pstPen->enFontSize = (MT_UNF_CC_FONTSIZE_E)u8PenSize;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetFontStyle(CC708_PenDef_S *pstPen, MT_U8 u8FontStyle, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->enFontName != u8FontStyle )
    {
        pstPen->enFontName = (MT_UNF_CC_FONTNAME_E)u8FontStyle;
        u8PenChanged = 1;
    }
    
    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextTag(CC708_PenDef_S *pstPen, MT_U8 u8TextTag, MT_U8 u8IsVisible)
{
    pstPen->enTextTag = (CC708_TextTag_E)u8TextTag;

    return 1;
}

MT_U8 CC708_Pen_SetTextOffset(CC708_PenDef_S *pstPen, MT_U8 u8TextOffset, MT_U8 u8IsVisible)
{
    pstPen->enTextOffset = (CC708_TextOffset_E)u8TextOffset;

    return 0;
}

MT_U8 CC708_Pen_SetTextItalics(CC708_PenDef_S *pstPen, MT_U8 u8IsItalic, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->u8IsItalic != u8IsItalic )
    {
        pstPen->u8IsItalic = u8IsItalic;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextUnderline(CC708_PenDef_S *pstPen, MT_U8 u8IsUnderline, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->u8IsUnderline != u8IsUnderline )
    {
        pstPen->u8IsUnderline = u8IsUnderline;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextEdgeType(CC708_PenDef_S *pstPen, MT_U8 u8EdgeType, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->enEdgeType != u8EdgeType )
    {
        pstPen->enEdgeType = (MT_UNF_CC_EdgeType_E)u8EdgeType;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextEdgeColor(CC708_PenDef_S *pstPen, MT_U32 u32EdgeColor, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->u32EdgeColor != u32EdgeColor )
    {
        pstPen->u32EdgeColor = u32EdgeColor;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextFgColor(CC708_PenDef_S *pstPen, MT_S32 s32FGColor, MT_S32 s32FGOpacity, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    if ( pstPen->u32FGColor != s32FGColor )
    {
        pstPen->u32FGColor = s32FGColor;
        u8PenChanged = 1;
    }

    /* text opacity */
    if ( pstPen->u8FGOpacity != s32FGOpacity )
    {
        pstPen->u8FGOpacity = s32FGOpacity;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}

MT_U8 CC708_Pen_SetTextBgColor(CC708_PenDef_S *pstPen, MT_S32 s32BGColor, MT_S32 s32BGOpacity, MT_U8 u8IsVisible)
{
    MT_U8 u8PenChanged = 0;

    /* background color */
    if ( pstPen->u32BGColor != s32BGColor )
    {
        pstPen->u32BGColor = s32BGColor;
        u8PenChanged = 1;
    }

    /* background opacity */

    if ( pstPen->u8BGOpacity != s32BGOpacity )
    {
        pstPen->u8BGOpacity = s32BGOpacity;
        u8PenChanged = 1;
    }

    return u8PenChanged;
}
/*******************************************************************************
*                       Static Function Definition
*******************************************************************************/
