/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _CCDISP_API_H
#define _CCDISP_API_H

#include "mt_type.h"
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

typedef enum tagCCDISP_WINDOW_ID_E
{
    CCDISP_WINDOW_ID_0 = 0,
    CCDISP_WINDOW_ID_1,
    CCDISP_WINDOW_ID_2,
    CCDISP_WINDOW_ID_3,
    CCDISP_WINDOW_ID_4,
    CCDISP_WINDOW_ID_5,
    CCDISP_WINDOW_ID_6,
    CCDISP_WINDOW_ID_7,
    CCDISP_WINDOW_ID_BUTT
} CCDISP_WINDOW_ID_E;

/*the maximum cc screens for cc*/
#define CCDISP_MAX_WINDOWS    ((MT_S32)CCDISP_WINDOW_ID_BUTT)


typedef enum tagCCDISP_ROLLSPEED_E
{
    CCDISP_SPEED1 = 0,
    CCDISP_SPEED2,
    CCDISP_SPEED3,
    CCDISP_SPEED4,
    CCDISP_SPEED5,
    CCDISP_SPEED6,
} CCDISP_ROLLSPEED_E;


typedef struct tagCCDISP_OPT_S
{
    MT_UNF_CC_GETPTS_CB_FN       pfnGetPts;
    MT_UNF_CC_DISPLAY_CB_FN      pfnDisplay;
    MT_UNF_CC_GETTEXTSIZE_CB_FN  pfnGetTextSize;
    //MT_UNF_CC_GETTEXTSIZE_EX_CB_FN  pfnGetTextSizeEx;
    MT_UNF_CC_BLIT_CB_FN         pfnBlit;
    MT_UNF_CC_VBI_CB_FN          pfnVBIOutput;
    MT_UNF_CC_XDS_CB_FN          pfnXDSOutput;
    MT_VOID*                     pUserData;
} CCDISP_OPT_S;


typedef struct tagCCDisp_Text_S
{
    MT_U32                 u32FgColor;
    MT_UNF_CC_COLOR_S     stFgColor;
    MT_UNF_CC_OPACITY_E   enFgOpacity;
    MT_U32                 u32BgColor;
    MT_UNF_CC_COLOR_S     stBgColor;
    MT_UNF_CC_OPACITY_E   enBgOpacity;
    MT_UNF_CC_FONTNAME_E  enFontName;
    MT_UNF_CC_FONTSTYLE_E enFontStyle;
    MT_UNF_CC_FONTSIZE_E  enFontSize;

    MT_UNF_CC_EdgeType_E  enEdgeType;
    MT_U32                 u32EdgeColor;
    MT_UNF_CC_COLOR_S     stEdgeColor;
    MT_UNF_CC_OPACITY_E   enEdgeOpacity;

    //MT_BOOL bFlash;
    //MT_S32  s32UnderlinePos;
    //MT_S32  s32UnderlineThick;
} CCDisp_Text_S;

typedef struct tagCCDisp_Window_S
{
    CCDISP_WINDOW_ID_E   enWinID;
    MT_BOOL              bVisible;
    MT_UNF_CC_RECT_S     stWinRect;
    MT_U32                u32WinColor;
    MT_UNF_CC_COLOR_S    stWinColor;
    MT_UNF_CC_OPACITY_E  enWinOpacity;
    CCDisp_Text_S        stText;
} CCDisp_Window_S;

typedef struct tagCCDisp_Screen_S
{
    MT_UNF_CC_RECT_S     stScreenRect;
    MT_UNF_CC_RECT_S     stClipArea;
    MT_U32                u32ScreenColor;
    MT_UNF_CC_COLOR_S    stScreenColor;
    MT_UNF_CC_OPACITY_E  enScreenOpacity;

    //MT_U16 u16Display_x;
    //MT_U16 u16Display_y;
    //MT_U16 u16displayWidth;
    //MT_U16 u16DisplayHeight;

    //MT_U8 real_font_size[MT_UNF_CC_FN_BUTT][MT_UNF_CC_FONTSIZE_BUTT];   /*to store the font size of nano-x matched theCCFontSize*/
    //MT_U32 flag;                 /*underline and edge type*/

    CCDisp_Window_S astWindow[CCDISP_MAX_WINDOWS];
    CCDISP_OPT_S stOpt;
} CCDisp_Screen_S;

typedef struct tagCCDISP_INIT_PARAM_S
{
    CCDISP_OPT_S stOpt;
} CCDISP_INIT_PARAM_S;


typedef struct tagCCDisp_FlashText_S
{
    MT_UNF_CC_RECT_S    stRect;
    MT_U16          *            pu16Text;
    MT_S32                        s32TextLen;
    MT_U32                        u32FgColor;
    MT_U32                        u32BgColor;
    MT_U32                        u32DisplayWidth;
    MT_U32                        u32DisplayHeight;
    MT_UNF_CC_FONTSIZE_E  enFontSize;
    MT_U8                         u8AribFontSize;
    //MT_UNF_CC_ARIB_SCALE_E        enScaleType;
    MT_U8                         u8CharInterval;
    MT_U8                         u8LineInterval;
    //MT_UNF_CC_ARIB_PRTDIRECT_E    enPrtDirect;

    MT_BOOL bFgFlash;//fg flash or bg flash
    struct tagCCDisp_FlashText_S *   next;
}CCDISP_FlashText_S;

/*****************************************************************************
*                    Extern Data Declarations
*****************************************************************************/

 /*****************************************************************************
*                    Extern Function Prototypes
*****************************************************************************/

MT_S32 CCDISP_Init(CCDISP_INIT_PARAM_S *pstInitParam);
MT_S32 CCDISP_DeInit(void);


MT_S32 CCDISP_Screen_SetSize(MT_UNF_CC_RECT_S stScreenRect);
MT_S32 CCDISP_Screen_SetClipArea(MT_UNF_CC_RECT_S stClipArea);
MT_S32 CCDISP_Screen_DisableClip(void);
MT_S32 CCDISP_Screen_GetColor(MT_U32 *pu32Color, MT_UNF_CC_OPACITY_E *penOpacity);
MT_S32 CCDISP_Screen_SetColor(MT_U32 u32Color, MT_UNF_CC_OPACITY_E enOpacity);
MT_S32 CCDISP_Screen_GetColorByRGB(MT_UNF_CC_COLOR_S *pstColor);
MT_S32 CCDISP_Screen_SetColorByRGB(MT_UNF_CC_COLOR_S stColor);
MT_S32 CCDISP_Screen_Clear(void);


MT_S32 CCDISP_Window_Create(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_RECT_S stWinRect);
MT_S32 CCDISP_Window_Destroy(CCDISP_WINDOW_ID_E enWinID);
MT_S32 CCDISP_Window_Show(CCDISP_WINDOW_ID_E enWinID);
MT_S32 CCDISP_Window_Hide(CCDISP_WINDOW_ID_E enWinID);
MT_S32 CCDISP_Window_GetSize(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_RECT_S *pstWinRect);
MT_S32 CCDISP_Window_SetSize(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_RECT_S stWinRect);
MT_S32 CCDISP_Window_GetColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 *pu32WinColor, MT_UNF_CC_OPACITY_E *penWinOpacity);
MT_S32 CCDISP_Window_SetColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 u32WinColor, MT_UNF_CC_OPACITY_E enWinOpacity);
MT_S32 CCDISP_Window_GetColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S *pstWinColor);
MT_S32 CCDISP_Window_SetColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S stWinColor);
MT_S32 CCDISP_Window_Clear(CCDISP_WINDOW_ID_E enWinID);
MT_S32 CCDISP_Window_FillRect(CCDISP_WINDOW_ID_E enWinID, MT_U32 u32Color,
                              MT_UNF_CC_OPACITY_E enOpacity, MT_UNF_CC_RECT_S stRect,MT_U32 flag);
MT_S32 CCDISP_Window_FillRectByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S stColor, MT_UNF_CC_RECT_S stRect,MT_U32 flag);
MT_S32 CCDISP_Window_BlockMove(CCDISP_WINDOW_ID_E enWinID, MT_S32 s32DstX, MT_S32 s32DstY,
                               MT_S32 s32Width, MT_S32 s32Height, MT_S32 s32SrcX, MT_S32 s32SrcY);
MT_S32 CCDISP_Window_Rollup(CCDISP_WINDOW_ID_E enWinID, MT_U16 x, MT_U16 width,
                            MT_U16 u16StartY, MT_U16 u16EndY, MT_U16 u16Offset, CCDISP_ROLLSPEED_E enSpeed);


MT_S32 CCDISP_Text_GetSize(CCDISP_WINDOW_ID_E enWinID, MT_U16 *pu16Str,
                           MT_S32 s32Length, MT_S32 *ps32Width, MT_S32 *ps32Height);
MT_S32 CCDISP_Text_Draw(CCDISP_WINDOW_ID_E enWinID, MT_S32 x, MT_S32 y,
                                MT_U16 *pu16Str, MT_S32 s32StrLen, MT_S32 *ps32DrawWidth,MT_U32 flag);

MT_S32 CCDISP_Text_SetEdgeType(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_EdgeType_E enEdgeType);
MT_S32 CCDISP_Text_SetEdgeColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 u32EdgeColor, MT_UNF_CC_OPACITY_E enEdgeOpacity);
MT_S32 CCDISP_Text_SetEdgeColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S stEdgeColor);
MT_S32 CCDISP_Text_SetUnderLine(CCDISP_WINDOW_ID_E enWinID, MT_BOOL bUnderline);
MT_S32 CCDISP_Text_SetItalic(CCDISP_WINDOW_ID_E enWinID, MT_BOOL bItalic);
MT_S32 CCDISP_Text_SetFlash(CCDISP_WINDOW_ID_E enWinID, MT_BOOL bFlash);
MT_S32 CCDISP_Text_SetFontName(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_FONTNAME_E enFontName);
MT_S32 CCDISP_Text_SetFontSize(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_FONTSIZE_E enFontSize);
MT_S32 CCDISP_Text_GetFGColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 *pu32FgColor, MT_UNF_CC_OPACITY_E *penFgOpacity);
MT_S32 CCDISP_Text_SetFGColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 u32FgColor, MT_UNF_CC_OPACITY_E enFgOpacity);
MT_S32 CCDISP_Text_GetBGColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 *pu32BgColor, MT_UNF_CC_OPACITY_E *penBgOpacity);
MT_S32 CCDISP_Text_SetBGColor(CCDISP_WINDOW_ID_E enWinID, MT_U32 u32BgColor, MT_UNF_CC_OPACITY_E enBgOpacity);

MT_S32 CCDISP_Text_GetFGColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S *pstFgColor);
MT_S32 CCDISP_Text_SetFGColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S stFgColor);
MT_S32 CCDISP_Text_GetBGColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S *pstBgColor);
MT_S32 CCDISP_Text_SetBGColorByRGB(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_COLOR_S stBgColor);

MT_S32 CCDISP_Bitmap_Draw(CCDISP_WINDOW_ID_E enWinID, MT_UNF_CC_RECT_S stRect, MT_UNF_CC_BITMAP_S *pstBitmap);

MT_S32 CCDISP_SendVbiData(MT_UNF_CC_VBI_DADA_S *pstVbiDataField1,MT_UNF_CC_VBI_DADA_S *pstVbiDataField2);

MT_S32 CCDISP_GetCurPts(S64 *ps64CurPts);

MT_S32 CCDISP_OutputXDSData(MT_U8 u8XDSClass, MT_U8 u8XDSPacketType, MT_U8 *pu8Data, MT_U8 u8Length);

MT_S32 CCDISP_Text_CharFlash(MT_VOID);
MT_S32 CCDISP_Text_DeleteCharFlash(CCDISP_WINDOW_ID_E enWinID);

#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif //#ifndef _PERU_CCDISP_API_H

/*****************************************************************************
*                    End Of File
*****************************************************************************/
