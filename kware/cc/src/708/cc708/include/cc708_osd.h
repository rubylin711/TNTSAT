/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CC708_OSD_H__
#define __CC708_OSD_H__

#include "mt_type.h"
#include "cc708.h"

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
MT_S32 CC708_OSD_Start(MT_VOID);
MT_S32 CC708_OSD_Stop(MT_VOID);
MT_S32 CC708_OSD_Reset(MT_VOID);


MT_S32 CC708_OSD_DefineWindow (MT_U8 u8WindowID,MT_U8 u8Priority,MT_U8 u8AnchorPoint,
                                            MT_U8 u8RelativePositioning,MT_U8 u8AnchorVertical,
                                            MT_U8 u8AnchorHorizontal,MT_U8 u8RowCount,MT_U8 u8ColumnCount,
                                            MT_U8 u8RowLock,MT_U8 u8ColumnLock,MT_U8 u8IsVisible,
                                            MT_U8 u8WindowStyleID, MT_U8 u8PenStyleID);
MT_S32 CC708_OSD_SetWindowAttributes (MT_U8 u8Justify,MT_U8 u8PrintDirection,MT_U8 u8ScrollDirection,
                                                      MT_U8 u8WordWrap,MT_U8 u8DisplayEffect,MT_U8 u8EffectDirection,
                                                      MT_U8 u8EffectSpeed,MT_U32 u32FillColor,MT_U8 u8FillOpacity,
                                                      MT_U8 u8BorderType,MT_U32 u32BorderColor);

MT_S32 CC708_OSD_SetCurrentWindow(MT_U8 u8WindowID);
MT_S32 CC708_OSD_ClearWindows(MT_U8 u8WindowMap);
MT_S32 CC708_OSD_DeleteWindows(MT_U8 u8WindowMap);
MT_S32 CC708_OSD_DisplayWindows(MT_U8 u8WindowMap);
MT_S32 CC708_OSD_HideWindows(MT_U8 u8WindowMap);
MT_S32 CC708_OSD_ToggleWindows(MT_U8 u8WindowMap);
MT_S32 CC708_OSD_CharFlash(MT_VOID);


MT_S32 CC708_OSD_SetPenAttributes (MT_U8 u8FontSize,MT_U8 u8FontName,MT_U8 u8TextTag,MT_U8 u8TextOffset,
                                               MT_U8 u8IsItalic,MT_U8 u8IsUnderline,MT_U8 u8EdgeType);
MT_S32 CC708_OSD_SetPenColor (MT_U32 u32FGColor,MT_U8 u8FGOpacity,MT_U32 u32BGColor,
                                          MT_U8 u8BGOpacity,MT_U32 u32EdgeColor);
MT_S32 CC708_OSD_SetPenLocation (MT_U8 u8Row,MT_U8 u8Column);

MT_S32 CC708_OSD_ETX(MT_VOID);
MT_S32 CC708_OSD_BSText(MT_VOID);
MT_S32 CC708_OSD_FFText(MT_VOID);
MT_S32 CC708_OSD_CRText(MT_VOID);
MT_S32 CC708_OSD_HCRText(MT_VOID);

MT_S32 CC708_OSD_ClrCCScreen(MT_VOID);
MT_VOID CC708_OSD_DrawCaptionToWindow(MT_U16 *pu16Text,MT_S32 s32Len);

#ifdef __cplusplus
}
#endif

#endif //#ifndef _DTVCCOSD_H_

/*****************************************************************************
*                    End Of File
*****************************************************************************/
