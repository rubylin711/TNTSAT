/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC_CURSOR_H__
#define __CC_CURSOR_H__

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
void CC708_Cursor_ResetToOrigin(CC708_Window_S *pstWindow, CC708_Cursor_S * pstCursor, 
                               CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorForwards(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 columnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorBackwards(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 columnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorToPosition(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32Column, 
                                      MT_U32 u32Y, MT_U32 u32Row,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorForwardsOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorForwardsOffScreen(CC708_Cursor_S * pstCursor, MT_U32 columnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorBackwardsOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorBackwardsOffScreen(CC708_Cursor_S * pstCursor, MT_U32 columnCount,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorOnScreen(CC708_Cursor_S * pstCursor, MT_U32 u32X, MT_U32 u32Y,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

void CC708_Cursor_MoveCursorOffScreen(CC708_Cursor_S * pstCursor, MT_U32 u32Row, MT_U32 u32Column,
                                CC708_PrintDirection_E enPrintDirection, CC708_ScrollDirection_E enScrollDirection);

int CC708_Cursor_GetCurrentColumn(CC708_Cursor_S * pstCursor);

int CC708_Cursor_GetCurrentRow(CC708_Cursor_S * pstCursor);

int CC708_Cursor_GetCurrentXPosition(CC708_Cursor_S * pstCursor, 
                                    CC708_PrintDirection_E enPrintDirection, 
                                    CC708_ScrollDirection_E enScrollDirection);
int CC708_Cursor_GetCurrentYPosition(CC708_Cursor_S * pstCursor,
                                    CC708_PrintDirection_E enPrintDirection, 
                                    CC708_ScrollDirection_E enScrollDirection);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
*                    End Of File
*****************************************************************************/
