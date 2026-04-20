/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_cc708_def.h"
#include "cc708_cursor.h"
#include "mt_type.h"

void CC708_Cursor_ResetToOrigin(CC708_Window_S *pstWindow, CC708_Cursor_S * pstCursor, 
                                           CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32CurRow = 0;
    pstCursor->s32CurColumn = 0;
    pstCursor->s32X = pstWindow->u16X;
    pstCursor->s32Y = pstWindow->u16Y+1;
    return;

}

void CC708_Cursor_MoveCursorForwards(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32ColumnCount,
                                    CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32X += u32X;
    pstCursor->s32CurColumn += u32ColumnCount;
    return;
}

void CC708_Cursor_MoveCursorBackwards(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32ColumnCount,
                                     CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{

    pstCursor->s32X -= u32X;
    pstCursor->s32CurColumn -= u32ColumnCount;
    return;

}

void CC708_Cursor_MoveCursorToPosition(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32Column, MT_U32 u32Y, MT_U32 u32Row,
                                      CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32X = u32X;
    pstCursor->s32CurColumn = u32Column;
    pstCursor->s32Y = u32Y;
    pstCursor->s32CurRow = u32Row;
    return;

}

void CC708_Cursor_MoveCursorForwardsOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X,
                                            CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32X += u32X;
    return ;
}

void CC708_Cursor_MoveCursorForwardsOffScreen(CC708_Cursor_S * pstCursor, MT_U32 u32ColumnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32CurColumn += u32ColumnCount;
    return;
}

void CC708_Cursor_MoveCursorBackwardsOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X,
                                             CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32X -= u32X;
    return;
}

void CC708_Cursor_MoveCursorBackwardsOffScreen(CC708_Cursor_S * pstCursor, MT_U32 u32ColumnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32CurColumn -= u32ColumnCount;
    return;
}

void CC708_Cursor_MoveCursorOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32Y,
                                    CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
     pstCursor->s32X = u32X;
     pstCursor->s32Y = u32Y;
     return;
}

void CC708_Cursor_MoveCursorOffScreen(CC708_Cursor_S * pstCursor, MT_U32 u32Row, MT_U32 u32Column,
                                     CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
    pstCursor->s32CurColumn = u32Column;
    pstCursor->s32CurRow = u32Row;
    return;
}

MT_S32 CC708_Cursor_GetCurrentColumn(CC708_Cursor_S * pstCursor)
{
        return pstCursor->s32CurColumn;
}


MT_S32 CC708_Cursor_GetCurrentRow(CC708_Cursor_S * pstCursor)
{
        return pstCursor->s32CurRow;
}

MT_S32 CC708_Cursor_GetCurrentXPosition(CC708_Cursor_S * pstCursor,
                                    CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
        return pstCursor->s32X;
}

MT_S32 CC708_Cursor_GetCurrentYPosition(CC708_Cursor_S * pstCursor,
                                    CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection)
{
        return pstCursor->s32Y;
}
/*******************************************************************************
*                       Static Function Definition
*******************************************************************************/
