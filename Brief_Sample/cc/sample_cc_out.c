/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_disp.h"
#include "mt_go.h"
#include "mt_go_text.h"
#include "mt_go_bliter.h"
#include "sample_cc_out.h"
#include "sample_cc_common.h"
#include "mt_unf_cc.h"
/***************************** Macro Definition ******************************/
#ifdef CC_DEBUG
#define CC_DBG_PRINT(fmt, args...)  printf(fmt, ##args)
#else
#define CC_DBG_PRINT(fmt, args...)
#endif

#define CC_FONT_FILE "./res/DroidSansFallbackLegacy.ttf"

#define COLOR2ARGB(a, c) (a) = ((c).u8Alpha << 24) | ((c).u8Red << 16) | ((c).u8Green << 8) | (c).u8Blue
#define ANDROID_COLOR2ARGB(a, c)  (a) = ((c).u8Alpha << 24) | ((c).u8Red << 16) | ((c).u8Green << 8) | (c).u8Blue
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
mt_u32 u32VirtualScreenWidth = 1280;
mt_u32 u32VirtualScreenHeight = 720;
mt_u32 u32CCSurfaceWidth;
mt_u32 u32CCSurfaceHeight;
static MT_HANDLE s_hVBI = 0xffffff;
static MT_HANDLE s_hccLayer = MTGO_INVALID_HANDLE;
static MT_HANDLE s_LayerSurface = MTGO_INVALID_HANDLE;
static MT_HANDLE s_hFontOutStardard= MTGO_INVALID_HANDLE;
static MT_HANDLE s_hFontOutLarge= MTGO_INVALID_HANDLE;
static MT_HANDLE s_hFontOutSmall= MTGO_INVALID_HANDLE;
static MT_HANDLE s_hCCSurface = MTGO_INVALID_HANDLE;
/******************************* API declaration *****************************/
static mt_s32 GetUnicode_Len(mt_u32 c)
{
    mt_s32 len = 0;
    
    if(c < 0x80)
    {
        len = 1;
    }
    else if(c < 0x800)
    {
        len = 2;
    }
    else if(c < 0x10000)
    {
        len = 3;
    }
    else if(c < 0x200000)
    {
        len = 4;
    }
    else if(c < 0x4000000)
    {
        len = 5;
    }
    else
    {
        len = 6;
    }
    
    return len;
}

static mt_s32 ConvUNICODEToUTF8code(mt_u32 c, mt_u8 *outbuf)
{
    mt_u32 len = 0;
    mt_s32 first = 0;
    
    if(c < 0x80)
    {
        first = 0;
        len = 1;
    }
    else if(c < 0x800)
    {
        first = 0xc0;
        len = 2;
    }
    else if(c < 0x10000)
    {
        first = 0xe0;
        len = 3;
    }
    else if(c < 0x200000)
    {
        first = 0xf0;
        len = 4;
    }
    else if(c < 0x4000000)
    {
        first = 0xf8;
        len = 5;
    }
    else
    {
        first = 0xfc;
        len = 6;
    }

    for(mt_s32 i = len - 1; i > 0; --i)
    {
        outbuf[i] = (c & 0x3f) | 0x80;
        c >>= 6;
    }
    outbuf[0] = c | first;

    return len;
}


static mt_s32 UTF16LE_to_UTF8(const mt_u8 *pInbuf, mt_s32 InLen, mt_u8 *pOutbuf, mt_s32 *pOutLen )
{
    const mt_u8 *in = NULL;
    mt_u8       *p = NULL;
    mt_s32      s32Outsize = 0;
    mt_s32      s32len = 0;
    mt_s32      i = 0;
    mt_u32      unicode = 0;
    mt_u32      tmp = 0;
    
    s32Outsize = *pOutLen;
    in = (const mt_u8*)pInbuf;
    p = pOutbuf;
    for(i = 0; i < InLen; i += 2)
    {
        if((i + 1) < InLen)
        {
            unicode = in[i];
            tmp = in[i + 1];
            unicode |= tmp << 8;
        }
        else
        {
            break;
        }

        s32len =  GetUnicode_Len(unicode);
        
        if( s32Outsize < s32len )
        {
            break;
        }
        
        s32Outsize -= s32len;

        p += ConvUNICODEToUTF8code(unicode, p);
    }
    *p = 0;
    *pOutLen = p - pOutbuf;

    return MT_SUCCESS;
}


static mt_u8 JustifyTranscode(mt_u8 u8justify)
{
    switch(u8justify)
    {
        case MT_UNF_CC_JUSTIFY_LEFT:
            u8justify = MTGO_LAYOUT_LEFT;
            break;
        case MT_UNF_CC_JUSTIFY_RIGHT:
            u8justify = MTGO_LAYOUT_RIGHT;
            break;
        case MT_UNF_CC_JUSTIFY_CENTER:
            u8justify = MTGO_LAYOUT_HCENTER;
            break;
        case MT_UNF_CC_JUSTIFY_FULL:
            u8justify = MTGO_LAYOUT_HCENTER;
            break;
        default:
            u8justify = MTGO_LAYOUT_LEFT;
            break;
    }
    
    return u8justify;
}


static mt_u8 ReverseChar(mt_u8 Data)
{
    mt_u8 b0 = 0;
    mt_u8 b1 = 0;
    mt_u8 b2 = 0;
    mt_u8 b3 = 0;
    mt_u8 b4 = 0;
    mt_u8 b5 = 0;
    mt_u8 b6 = 0;
    mt_u8 b7 = 0;
    mt_u8 Reversed = 0;

    b0 = (Data >> 0) & 0x1;
    b1 = (Data >> 1) & 0x1;
    b2 = (Data >> 2) & 0x1;
    b3 = (Data >> 3) & 0x1;
    b4 = (Data >> 4) & 0x1;
    b5 = (Data >> 5) & 0x1;
    b6 = (Data >> 6) & 0x1;
    b7 = (Data >> 7) & 0x1;
    Reversed = (b7 << 0)
             | (b6 << 1)
             | (b5 << 2)
             | (b4 << 3)
             | (b3 << 4)
             | (b2 << 5)
             | (b1 << 6)
             | (b0 << 7);
    
    return Reversed;
}

static mt_s32 MtgoInit(MT_VOID)
{
    mt_s32            s32Ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_TEXT_INFO_S  stFontInfo = { 0 };

    s32Ret = MT_GO_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to init mtgo! ret = 0x%x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = MT_GO_InitText();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to init mtgo_text! ret = 0x%x!\n", s32Ret);
	    goto ERR0;
    }

    MT_GO_GetLayerDefaultParam(MTGO_LAYER_OSD0, &stLayerInfo);

    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    stLayerInfo.PixelFormat = MTGO_PF_8888;
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, &s_hccLayer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to create the layer hd 0, ret = 0x%x !\n", s32Ret);
        goto ERR1;
    }

    s32Ret =  MT_GO_GetLayerSurface(s_hccLayer, &s_LayerSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to get layer surface! s32Ret = 0x%x!\n", s32Ret);
        goto ERR2;
    }


    (mt_void)MT_GO_FillRect(s_LayerSurface, NULL, 0x00000000, MTGO_COMPOPT_NONE);

    stFontInfo.pFontFile = CC_FONT_FILE;
    stFontInfo.u32Size = 20;

    if(MT_SUCCESS != MT_GO_CreateTextEx(&stFontInfo, &s_hFontOutStardard))
    {
        SAMPLE_CC_ERR_PRINT("failed to create the stardard font: %s!ret = 0x%x\n", CC_FONT_FILE, s32Ret);
        goto ERR3;
    }

    stFontInfo.u32Size = 24;
    if(MT_SUCCESS != MT_GO_CreateTextEx(&stFontInfo, &s_hFontOutLarge))
    {
        SAMPLE_CC_ERR_PRINT("failed to create the large font: %s!ret = 0x%x\n", CC_FONT_FILE, s32Ret);
        goto ERR4;
    }

    stFontInfo.u32Size = 14;
    if(MT_SUCCESS != MT_GO_CreateTextEx(&stFontInfo, &s_hFontOutSmall))
    {
        SAMPLE_CC_ERR_PRINT("failed to create the small font: %s!ret = 0x%x\n", CC_FONT_FILE, s32Ret);
        goto ERR5;
    }

    return MT_SUCCESS;
    
ERR5:
    (mt_void)MT_GO_DestroyText(s_hFontOutLarge);
    s_hFontOutLarge = MTGO_INVALID_HANDLE;
ERR4:
    (mt_void)MT_GO_DestroyText(s_hFontOutStardard);
    s_hFontOutStardard = MTGO_INVALID_HANDLE;
ERR3:
    (mt_void)MT_GO_FreeSurface(s_LayerSurface);
    s_LayerSurface = MTGO_INVALID_HANDLE;
ERR2:
    (mt_void)MT_GO_DestroyLayer(s_hccLayer);
    s_hccLayer = MTGO_INVALID_HANDLE;
ERR1:
    (mt_void)MT_GO_DeinitText();
ERR0:
    (mt_void)MT_GO_Deinit();

    return MT_FAILURE;
}


static mt_s32 MtgoDeInit(MT_VOID)
{
    if (MTGO_INVALID_HANDLE != s_hFontOutSmall)
    {
        (mt_void)MT_GO_DestroyText(s_hFontOutSmall);
        s_hFontOutSmall = MTGO_INVALID_HANDLE;
    }
    
    if (MTGO_INVALID_HANDLE != s_hFontOutLarge)
    {
        (mt_void)MT_GO_DestroyText(s_hFontOutLarge);
        s_hFontOutLarge = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hFontOutStardard)
    {
        (mt_void)MT_GO_DestroyText(s_hFontOutStardard);
        s_hFontOutStardard = MTGO_INVALID_HANDLE;
    }

    s_LayerSurface = MTGO_INVALID_HANDLE;
    
    if (MTGO_INVALID_HANDLE != s_hCCSurface)
    {
        (mt_void)MT_GO_FreeSurface(s_hCCSurface);
        s_hCCSurface = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hccLayer)
    {
        (mt_void)MT_GO_DestroyLayer(s_hccLayer);
        s_hccLayer = MTGO_INVALID_HANDLE;
    }
    
    (mt_void)MT_GO_DeinitText();

    (mt_void)MT_GO_Deinit();

    return MT_SUCCESS;
}


mt_s32 CC_Output_Init(mt_handle* phOut)
{
    mt_s32                s32Ret = MT_FAILURE;
    MT_UNF_DISP_VBI_CFG_S stVBICfg = { 0 };

    if (MTGO_INVALID_HANDLE == s_LayerSurface)
    {
        s32Ret = MtgoInit();
    }

    stVBICfg.enType =  MT_UNF_DISP_VBI_TYPE_CC;
    MT_UNF_DISP_CreateVBI(MT_UNF_DISPLAY0, &stVBICfg, &s_hVBI);

    *phOut = 0xFE00;

    return s32Ret;
}


mt_s32 CC_Output_DeInit(mt_handle hOut)
{
    mt_s32 s32Ret = MT_SUCCESS;

    if (MTGO_INVALID_HANDLE != s_LayerSurface)
    {
        s32Ret = MtgoDeInit();
    }

    if (0xffffff != s_hVBI)
    {
        (mt_void)MT_UNF_DISP_DestroyVBI(s_hVBI);
    }

    return s32Ret;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 CC_Output_OnDraw(mt_u32 u32UserData, MT_UNF_CC_DISPLAY_PARAM_S *pstDisplayParam)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 CC_Output_OnDraw(ulong u32UserData, MT_UNF_CC_DISPLAY_PARAM_S *pstDisplayParam)
#endif
{
    mt_u8         *pu8InBuf = NULL;
    mt_u8         pu8OutBuf[128] = { 0 };
    mt_u8         u8justify = 0;
    mt_u8         u8WordWrap = 0;
    mt_u8         u8fontstyle = 0;
    mt_s32        s32Ret = MT_FAILURE;
    mt_s32        s32SurfaceWidth = 0;
    mt_s32        s32SurfaceHeight = 0;
    mt_s32        s32InLen = 0;
    mt_s32        s32OutLen = 0;
    mt_u32        u32bgColor = 0;
    mt_u32        u32fgColor = 0;
    mt_u32        u32Color = 0;
    mt_handle     hFontOut = 0;
    MT_RECT       SrcRect = { 0 };
    MTGO_LAYOUT_E enlayout = { 0 };
    MTGO_BLTOPT_S stBlitOpt = { 0 };

    if(NULL == pstDisplayParam)
    {
        return MT_FAILURE;
    }

    u32CCSurfaceWidth = pstDisplayParam->u32DisplayWidth;
    u32CCSurfaceHeight = pstDisplayParam->u32DisplayHeight;

    SrcRect.x = pstDisplayParam->stRect.x;
    SrcRect.y = pstDisplayParam->stRect.y;
    SrcRect.w = pstDisplayParam->stRect.width;
    SrcRect.h = pstDisplayParam->stRect.height;
    CC_DBG_PRINT("DisplayWidth: %d, DisplayHeight: %d\n", pstDisplayParam->u32DisplayWidth, pstDisplayParam->u32DisplayHeight);
    if (MTGO_INVALID_HANDLE == s_hCCSurface)
    {
        CC_DBG_PRINT("DisplayWidth: %d, DisplayHeight: %d\n", pstDisplayParam->u32DisplayWidth, pstDisplayParam->u32DisplayHeight);
        (mt_void)MT_GO_CreateSurface(pstDisplayParam->u32DisplayWidth, pstDisplayParam->u32DisplayHeight, MTGO_PF_8888, &s_hCCSurface);
        (mt_void)MT_GO_FillRect(s_hCCSurface, MT_NULL, 0x00000000, MTGO_COMPOPT_NONE);
    }
    else
    {
        (mt_void)MT_GO_GetSurfaceSize(s_hCCSurface, &s32SurfaceWidth, &s32SurfaceHeight);
        if(s32SurfaceWidth != pstDisplayParam->u32DisplayWidth || s32SurfaceHeight != pstDisplayParam->u32DisplayHeight)
        {
            CC_DBG_PRINT("DisplayWidth: %d, DisplayHeight: %d\n", pstDisplayParam->u32DisplayWidth, pstDisplayParam->u32DisplayHeight);
            (mt_void)MT_GO_FreeSurface(s_hCCSurface);
            (mt_void)MT_GO_CreateSurface(pstDisplayParam->u32DisplayWidth, pstDisplayParam->u32DisplayHeight, MTGO_PF_8888, &s_hCCSurface);
            (mt_void)MT_GO_FillRect(s_hCCSurface, MT_NULL, 0x00000000, MTGO_COMPOPT_NONE);
        }
    }

    switch (pstDisplayParam->enOpt)
    {
        case MT_UNF_CC_OPT_DRAWTEXT:
        {
            pu8InBuf = (mt_u8 *)pstDisplayParam->unDispParam.stText.pu16Text;
            s32InLen = pstDisplayParam->unDispParam.stText.u8TextLen * 2;
            s32OutLen = sizeof(pu8OutBuf);
            s32Ret = UTF16LE_to_UTF8(pu8InBuf, s32InLen, pu8OutBuf, &s32OutLen);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to UTF16LE_to_UTF8, ret = %x\n", s32Ret);
            }
            
            CC_DBG_PRINT("\nOnDraw: [%d, %s]\n", s32OutLen, pu8OutBuf);

            if(pstDisplayParam->unDispParam.stText.enFontSize == MT_UNF_CC_FONTSIZE_LARGE)
            {
                hFontOut = s_hFontOutLarge;
            }
            else if(pstDisplayParam->unDispParam.stText.enFontSize == MT_UNF_CC_FONTSIZE_SMALL)
            {
                hFontOut = s_hFontOutSmall;
            }
            else
            {
                hFontOut = s_hFontOutStardard;
            }
            
            COLOR2ARGB(u32bgColor, pstDisplayParam->unDispParam.stText.stBgColor);
            COLOR2ARGB(u32fgColor, pstDisplayParam->unDispParam.stText.stFgColor);
            MT_GO_SetTextColor(hFontOut, u32fgColor);
            MT_GO_SetTextBGColor(hFontOut, u32bgColor);

            if(0 == pstDisplayParam->unDispParam.stText.stBgColor.u8Alpha)
            {
                (mt_void)MT_GO_SetTextBGTransparent(hFontOut, MT_TRUE);
            }
            else
            {
                (mt_void)MT_GO_SetTextBGTransparent(hFontOut, MT_FALSE);
            }

            CC_DBG_PRINT("u32bgColor: %#x\n", u32bgColor);
            CC_DBG_PRINT("u32fgColor: %#x\n", u32fgColor);
            CC_DBG_PRINT("u8edgeType: %d\n", pstDisplayParam->unDispParam.stText.enEdgetype);
            CC_DBG_PRINT("u8fontSize: [%d], small, standard, large: 1, 2, 3\n", pstDisplayParam->unDispParam.stText.enFontSize);

            u8justify = pstDisplayParam->unDispParam.stText.u8Justify;
            enlayout = JustifyTranscode(u8justify);
            u8WordWrap = pstDisplayParam->unDispParam.stText.u8WordWrap;
            if(u8WordWrap)
            {
                enlayout = enlayout | MTGO_LAYOUT_WRAP;
            }

            u8fontstyle = pstDisplayParam->unDispParam.stText.enFontStyle;
            if((u8fontstyle ==  MT_UNF_CC_FONTSTYLE_ITALIC) || (u8fontstyle == MT_UNF_CC_FONTSTYLE_ITALIC_UNDERLINE))
            {
                MT_GO_SetTextStyle(hFontOut, MTGO_TEXT_STYLE_ITALIC);
            }
            else
            {
                MT_GO_SetTextStyle(hFontOut, MTGO_TEXT_STYLE_NORMAL);
            }

            CC_DBG_PRINT("Text Rect[%d %d %d %d]\n\n", SrcRect.x, SrcRect.y, SrcRect.w, SrcRect.h);

            s32Ret = MT_GO_TextOutEx(hFontOut, s_hCCSurface, (mt_char *)pu8OutBuf, &SrcRect, enlayout);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_CC_ERR_PRINT("failed to text out char!, ret = %x\n", s32Ret);
            }

            memset(&stBlitOpt, 0, sizeof(MTGO_BLTOPT_S));
            stBlitOpt.EnableScale = MT_TRUE;
            (mt_void)MT_GO_Blit (s_hCCSurface, MT_NULL, s_LayerSurface, MT_NULL, &stBlitOpt);
            (mt_void)MT_GO_RefreshLayer(s_hccLayer, MT_NULL);

            break;
        }
        case MT_UNF_CC_OPT_FILLRECT:
        {
            COLOR2ARGB(u32Color, pstDisplayParam->unDispParam.stFillRect.stColor);

            CC_DBG_PRINT("\nFillRect, u32Color: %#x, Rect[%d %d %d %d]\n\n", u32Color, SrcRect.x, SrcRect.y, SrcRect.w, SrcRect.h);

            (mt_void)MT_GO_FillRect(s_hCCSurface, &SrcRect, u32Color, MTGO_COMPOPT_NONE);

            memset(&stBlitOpt, 0, sizeof(MTGO_BLTOPT_S));
            stBlitOpt.EnableScale = MT_TRUE;
            (mt_void)MT_GO_Blit (s_hCCSurface, MT_NULL, s_LayerSurface, MT_NULL, &stBlitOpt);
            (mt_void)MT_GO_RefreshLayer(s_hccLayer, NULL);

            break;
        }
        case MT_UNF_CC_OPT_DRAWBITMAP:
        default:
            SAMPLE_CC_ERR_PRINT("Not implement\n");
            break;

    }

    return s32Ret;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 CC_Output_GetTextSize(mt_u32 u32Userdata, mt_u16 *u16Str,mt_s32 s32StrNum, mt_s32 *ps32Width, mt_s32 *ps32Heigth)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 CC_Output_GetTextSize(ulong u32Userdata, mt_u16 *u16Str,mt_s32 s32StrNum, mt_s32 *ps32Width, mt_s32 *ps32Heigth)
#endif
{
    mt_u8  pu8OutBuf[128] = { 0 };
    mt_u8  *pu8InBuf = 0;
    mt_s32 s32Ret = MT_FAILURE; 
    mt_s32 s32InLen = 0;
    mt_s32 s32OutLen = 0;
    
    if((NULL == u16Str) || (0 == s32StrNum) || (NULL == ps32Width) || (NULL == ps32Heigth))
    {
        return MT_FAILURE;
    }
    
    pu8InBuf = (mt_u8 *)u16Str;
    s32InLen = s32StrNum * 2;
    s32OutLen = sizeof(pu8OutBuf);
    s32Ret = UTF16LE_to_UTF8(pu8InBuf, s32InLen, pu8OutBuf, &s32OutLen);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to UTF16LE_to_UTF8, ret = %x\n", s32Ret);
    }

    *ps32Width = *ps32Heigth = 0;
    s32Ret = MT_GO_GetTextExtent(s_hFontOutStardard, (const mt_char*)pu8OutBuf, ps32Width, ps32Heigth);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_CC_ERR_PRINT("failed to MT_GO_GetTextExtent, ret = %x\n", s32Ret);
    }

    return  MT_SUCCESS;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 CC_Output_Blit(mt_u32 u32UserPrivatData, MT_UNF_CC_RECT_S *pstSrcRect, MT_UNF_CC_RECT_S *pstDestRect)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 CC_Output_Blit(ulong u32UserPrivatData, MT_UNF_CC_RECT_S *pstSrcRect, MT_UNF_CC_RECT_S *pstDestRect)
#endif
{
    MT_RECT       stSrcRect = { 0 };
    MT_RECT       stDstRect = { 0 };
    MTGO_BLTOPT_S stBlitOpt = { 0 };
    
    if(NULL == pstSrcRect || NULL == pstDestRect)
    {
        return MT_FAILURE;
    }

    if (MTGO_INVALID_HANDLE == s_hCCSurface)
    {
        return MT_SUCCESS;
    }
    
    stSrcRect.x = pstSrcRect->x;
    stSrcRect.y = pstSrcRect->y;
    stSrcRect.w = pstSrcRect->width;
    stSrcRect.h = pstSrcRect->height;

    stDstRect.x = pstDestRect->x;
    stDstRect.y = pstDestRect->y;
    stDstRect.w = pstDestRect->width;
    stDstRect.h = pstDestRect->height;
    CC_DBG_PRINT("Blit: [%d, %d, %d, %d]->[%d, %d, %d, %d]\n",
                    stSrcRect.x, stSrcRect.y, stSrcRect.w, 
                    stSrcRect.h, stDstRect.x, stDstRect.y, 
                    stDstRect.w, stDstRect.h);
    (mt_void)MT_GO_Blit(s_hCCSurface, &stSrcRect, s_hCCSurface, &stDstRect, MT_NULL);

    memset(&stBlitOpt, 0, sizeof(MTGO_BLTOPT_S));
    stBlitOpt.EnableScale = MT_TRUE;
    (mt_void)MT_GO_Blit(s_hCCSurface, MT_NULL, s_LayerSurface, MT_NULL , &stBlitOpt);
    (mt_void)MT_GO_RefreshLayer(s_hccLayer, MT_NULL);

    return MT_SUCCESS;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 CC_Output_VBIOutput(mt_u32 u32UserData, MT_UNF_CC_VBI_DADA_S *pstVBIData)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 CC_Output_VBIOutput(ulong u32UserData, MT_UNF_CC_VBI_DADA_S *pstVBIData)
#endif
{
    mt_s32       s32Ret = MT_FAILURE;
    static mt_u8 buf[20] = {0x00, 0x00, 0x01, 0xbd, 0x00, 0x0e, 0x8f, 0x80, 0x05, 0x21, 
                            0x25, 0xb5, 0xf1, 0x27, 0x99, 0xc5, 0x03, 0x00, 0x00, 0x00};
    MT_UNF_DISP_VBI_DATA_S stVBIData = { 0 };

    if(pstVBIData->u8FieldParity)
    {
        buf[17] = 0xf5;
    }
    else
    {
        buf[17] = 0xd5;
    }
    
    buf[18] = ReverseChar(pstVBIData->u8Data1);
    buf[19] = ReverseChar(pstVBIData->u8Data2);
    stVBIData.enType = MT_UNF_DISP_VBI_TYPE_CC;
    stVBIData.pu8DataAddr = buf;
    stVBIData.u32DataLen = sizeof(buf);

    if(0xffffff != s_hVBI)
    {
        s32Ret = MT_UNF_DISP_SendVBIData(s_hVBI, &stVBIData);
    }

    return s32Ret;
}
