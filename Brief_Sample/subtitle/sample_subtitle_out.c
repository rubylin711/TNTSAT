/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_common.h"

#include "mt_unf_disp.h"
#include "mt_unf_avplay.h"
#include "mt_unf_subtouput.h"
#include "mt_tde_type.h"
#include "mt_tde_api.h"
#ifdef ANDROID
#include "mt_adp_osd.h"
#else
#include "mt_go.h"
#endif

#include "sample_subtitle_out.h"
#define MT_FATAL_SUBT(fmt...)      MT_FATAL_PRINT(MT_ID_SUBT, fmt)
#define MT_ERR_SUBT(fmt...)        MT_ERR_PRINT(MT_ID_SUBT, fmt)
#define MT_WARN_SUBT(fmt...)       MT_WARN_PRINT(MT_ID_SUBT, fmt)
#define MT_INFO_SUBT(fmt...)       MT_INFO_PRINT(MT_ID_SUBT, fmt)

#define USED_SURFACE_TO_FILE 0

#define SUBT_OUTPUT_HANDLE (0xFE00)
#define BITWIDTH_8_BITS 8
#define BITWIDTH_32_BITS 32
#define SUBT_OFFSET_Y  30

#ifdef ANDROID
#define  DefaultSurfaceWidth 720
#define  DefaultSurfaceHeight 576
#define MT_INVALID_HANDLE (0xffffffff)
static MT_HANDLE s_hSurface = MT_INVALID_HANDLE;
static mt_u8 * bitmapBuffer = MT_NULL;
static mt_u32  SurfaceWidth=DefaultSurfaceWidth,SurfaceHeight=DefaultSurfaceHeight;
mt_u32 ScreenWidth = 1280;
mt_u32 ScreenHeight = 720;
#else
static MT_HANDLE s_hLayer = MTGO_INVALID_HANDLE;
static MT_HANDLE s_hLayerSurface = MTGO_INVALID_HANDLE;
static MT_HANDLE s_hSubtSurface = MTGO_INVALID_HANDLE;
static MT_HANDLE s_hFont = MTGO_INVALID_HANDLE;

static mt_s32 mtgoInit(void)
{
    mt_s32 s32Ret = 0;

    MTGO_LAYER_INFO_S stLayerInfo = {0};

    s32Ret = MT_GO_Init();

    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to init mtgo! ret = 0x%x !\n", s32Ret);
        return MT_FAILURE;
    }

    (mt_void)MT_GO_GetLayerDefaultParam(MTGO_LAYER_OSD0, &stLayerInfo);

    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    stLayerInfo.PixelFormat = MTGO_PF_8888;
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, &s_hLayer);

    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to create the layer hd 0, ret = 0x%x !\n", s32Ret);
        goto RET;
    }

    s32Ret =  MT_GO_GetLayerSurface(s_hLayer, &s_hLayerSurface);

    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to get layer surface! s32Ret = 0x%x !\n", s32Ret);
        goto RET;
    }

    s32Ret = MT_GO_CreateSurface(720,576,MTGO_PF_8888,&s_hSubtSurface);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_SUBT("failed to create subt surface! s32Ret = 0x%x\n", s32Ret);
        goto RET;
    }
    (mt_void)MT_GO_FillRect(s_hSubtSurface, NULL, 0xFFFF0000, MTGO_COMPOPT_NONE);

    return MT_SUCCESS;

RET:
    if (MTGO_INVALID_HANDLE != s_hSubtSurface)
    {
        (mt_void)MT_GO_FreeSurface(s_hSubtSurface);
        s_hSubtSurface = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hLayer)
    {
        (mt_void)MT_GO_DestroyLayer(s_hLayer);
        s_hLayer = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hFont)
    {
        (mt_void)MT_GO_DestroyText(s_hFont);
        s_hFont = MTGO_INVALID_HANDLE;
    }

    s_hLayerSurface = MTGO_INVALID_HANDLE;

    (mt_void)MT_GO_Deinit();

    return MT_FAILURE;
}

static mt_s32 mtgoDeInit(void)
{
    if (MTGO_INVALID_HANDLE != s_hSubtSurface)
    {
        (mt_void)MT_GO_FreeSurface(s_hSubtSurface);
        s_hSubtSurface = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hLayer)
    {
        (mt_void)MT_GO_DestroyLayer(s_hLayer);
        s_hLayer = MTGO_INVALID_HANDLE;
    }

    if (MTGO_INVALID_HANDLE != s_hFont)
    {
        (mt_void)MT_GO_DestroyText(s_hFont);
        s_hFont = MTGO_INVALID_HANDLE;
    }

    s_hLayerSurface = MTGO_INVALID_HANDLE;

    (mt_void)MT_GO_Deinit();

    return MT_FAILURE;
}

static mt_s32 ConvertRect(MT_HANDLE hSrcSurface, MT_HANDLE hDstSurface, mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h, MT_RECT *pstRect)
{
    mt_s32 s32SrcWidth = 0, s32SrcHeight = 0;
    mt_s32 s32DstWidth = 0, s32DstHeight = 0;

    (mt_void)MT_GO_GetSurfaceSize(hSrcSurface, &s32SrcWidth, &s32SrcHeight);
    (mt_void)MT_GO_GetSurfaceSize(hDstSurface, &s32DstWidth, &s32DstHeight);

    pstRect->x = (x * s32DstWidth) / s32SrcWidth;
    pstRect->y = (y * s32DstHeight) / s32SrcHeight;
    pstRect->w = (w * s32DstWidth) / s32SrcWidth;
    pstRect->h = (h * s32DstHeight) / s32SrcHeight;

#if 0
    if ((pstRect->x + pstRect->w) > s32DstWidth)
    {
        pstRect->w = s32DstWidth - pstRect->x;
    }
    if ((pstRect->y + pstRect->h) > s32DstHeight)
    {
        pstRect->h = s32DstHeight - pstRect->y;
    }
#else
    if ((pstRect->x + pstRect->w) > s32DstWidth)
    {
        pstRect->w = s32DstWidth - pstRect->x;
    }
    if(pstRect->y < SUBT_OFFSET_Y)
    {
       pstRect->y = SUBT_OFFSET_Y;
    }
    if(pstRect->y + pstRect->h + SUBT_OFFSET_Y > s32DstHeight)
    {
        if(pstRect->y >= (SUBT_OFFSET_Y<<1))
        {
            pstRect->y -= SUBT_OFFSET_Y;
        }
        else if(pstRect->y < (SUBT_OFFSET_Y<<1))
        {
            pstRect->y = SUBT_OFFSET_Y;
        }
        pstRect->h = ((s32DstHeight - SUBT_OFFSET_Y - pstRect->y) < 0)?0:
                        (s32DstHeight - SUBT_OFFSET_Y - pstRect->y);
    }
#endif
    return MT_SUCCESS;
}

#endif

#ifdef ANDROID
mt_void OsdDeInit(mt_void)
{
    if (MT_INVALID_HANDLE != s_hSurface)
    {
        MTADP_OSD_ClearSurface(s_hSurface);
        MTADP_OSD_DestroySurface(s_hSurface);
        s_hSurface = MT_INVALID_HANDLE;
    }

    (mt_void)MTADP_OSD_DeInit();
    return;

}

mt_s32 OsdInit(mt_void)
{
    mt_s32 s32Ret = 0;
    MTADP_SURFACE_ATTR_S    stSurAttr;

    (mt_void)MTADP_OSD_Init();

    MT_UNF_DISP_GetVirtualScreen(MT_UNF_DISPLAY0, &ScreenWidth, &ScreenHeight);
    stSurAttr.u32Width = ScreenWidth;
    stSurAttr.u32Height = ScreenHeight;
    stSurAttr.enPixelFormat = MTADP_PF_8888;

    s32Ret = MTADP_OSD_CreateSurface(&stSurAttr, &s_hSurface);

    if (MT_SUCCESS != s32Ret)
    {
        printf( "failed to MTADP_OSD_CreateSurface, ret = 0x%x !\n", s32Ret);
        return MT_FAILURE;
    }
    return MT_SUCCESS;

}

static mt_s32 AndroidConvertRect( mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h, MT_RECT_S*pstRect)
{

    pstRect->s32X = (x * ScreenWidth) / SurfaceWidth;
    pstRect->s32Y= (y * ScreenHeight) / SurfaceHeight;
    pstRect->s32Width = (w * ScreenWidth) / SurfaceWidth;
    pstRect->s32Height = (h * ScreenHeight) / SurfaceHeight;

    if ((pstRect->s32X + pstRect->s32Width) > ScreenWidth)
    {
        pstRect->s32Width = ScreenWidth - pstRect->s32X;
    }
    if ((pstRect->s32Y + pstRect->s32Height) > ScreenHeight)
    {
        pstRect->s32Height = ScreenHeight - pstRect->s32Y;
    }

    return MT_SUCCESS;
}

#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 Subt_Output_OnDraw(mt_u32 u32UserData, const MT_UNF_SO_SUBTITLE_INFO_S *pstInfo, mt_void *pArg)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 Subt_Output_OnDraw(ulong u32UserData, const MT_UNF_SO_SUBTITLE_INFO_S *pstInfo, mt_void *pArg)
#endif
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_s32 s32Width = 0, s32Height = 0;
#ifdef ANDROID
    MT_RECT_S stSrcRect = {0, 0, 0, 0};
    MT_RECT_S stDesRect = {0, 0, 0, 0};
#else
    MT_PIXELDATA pData;
    MTGO_BLTOPT_S stBlitOpt;
    MT_RECT rect = {0, 0, 0, 0};
#endif

    if (0 == pstInfo->unSubtitleParam.stGfx.u32CanvasWidth || 0 == pstInfo->unSubtitleParam.stGfx.u32CanvasHeight)
    {
        return MT_SUCCESS;
    }

    //printf("display w/h is [%d][%d]\n", pstInfo->unSubtitleParam.stGfx.u32CanvasWidth,
    //                                            pstInfo->unSubtitleParam.stGfx.u32CanvasHeight);
#ifdef ANDROID

    if(MT_NULL==s_hSurface)
    {
        MTADP_SURFACE_ATTR_S    stSurAttr;

        (mt_void)MTADP_OSD_Init();

        stSurAttr.u32Width = ScreenWidth;
        stSurAttr.u32Height = ScreenHeight;
        stSurAttr.enPixelFormat = MTADP_PF_8888;

        s32Ret = MTADP_OSD_CreateSurface(&stSurAttr, &s_hSurface);
        if (MT_SUCCESS != s32Ret)
        {
                 printf( "failed to MTADP_OSD_CreateSurface, ret = 0x%x !\n", s32Ret);
                 return MT_FAILURE;
        }
    }

#else

    if (MTGO_INVALID_HANDLE != s_hSubtSurface)
    {
        (mt_void)MT_GO_LockSurface(s_hSubtSurface, pData, MT_TRUE);
        (mt_void)MT_GO_GetSurfaceSize(s_hSubtSurface, &s32Width, &s32Height);
        (mt_void)MT_GO_UnlockSurface(s_hSubtSurface);
    }
    if (s32Width != pstInfo->unSubtitleParam.stGfx.u32CanvasWidth ||
        s32Height != pstInfo->unSubtitleParam.stGfx.u32CanvasHeight)
    {
        if (MTGO_INVALID_HANDLE != s_hSubtSurface)
        {
            (mt_void)MT_GO_FreeSurface(s_hSubtSurface);
            s_hSubtSurface = MTGO_INVALID_HANDLE;
        }
        s32Ret = MT_GO_CreateSurface(pstInfo->unSubtitleParam.stGfx.u32CanvasWidth,pstInfo->unSubtitleParam.stGfx.u32CanvasHeight,
                                        MTGO_PF_8888,&s_hSubtSurface);
        if (s32Ret == MT_FAILURE)
        {
            return MT_FAILURE;
        }
    }
#endif

    switch (pstInfo->eType)
    {
    case MT_UNF_SUBTITLE_BITMAP:
        MT_INFO_SUBT("sub title bitmap! \n");
        mt_u32 u32Index=0;
        mt_u32 i = 0, j = 0;
        mt_u32 u32PaletteIdx = 0;

#ifdef ANDROID
     if(MT_NULL==bitmapBuffer)
     {
            bitmapBuffer = (mt_u8 *)malloc(SurfaceWidth*SurfaceHeight*4);
            if(MT_NULL==bitmapBuffer)
            {
                    return MT_FAILURE;
            }
     }

        if (SurfaceWidth != pstInfo->unSubtitleParam.stGfx.u32CanvasWidth ||
        SurfaceHeight != pstInfo->unSubtitleParam.stGfx.u32CanvasHeight)
    {
         if(bitmapBuffer != MT_NULL)
         {
              free(bitmapBuffer);
              bitmapBuffer=MT_NULL;
         }
          bitmapBuffer = (mt_u8 *)malloc(pstInfo->unSubtitleParam.stGfx.u32CanvasWidth*pstInfo->unSubtitleParam.stGfx.u32CanvasHeight*4);
         if(bitmapBuffer == MT_NULL)
         {
            MT_ERR_SUBT("create subt bitmapbuffer fail!\n");
            return MT_FAILURE;
         }
         SurfaceWidth=pstInfo->unSubtitleParam.stGfx.u32CanvasWidth;
         SurfaceHeight=pstInfo->unSubtitleParam.stGfx.u32CanvasHeight;
     }
     memset(bitmapBuffer, 0x00, SurfaceWidth * SurfaceHeight * 4 );
#else
        mt_u8 *pu8Surface = NULL;
        MT_RECT DstRect = {0, 0, 0, 0};


        (mt_void)MT_GO_LockSurface(s_hSubtSurface, pData, MT_TRUE);
        (mt_void)MT_GO_GetSurfaceSize(s_hSubtSurface, &s32Width, &s32Height);

        pu8Surface = (mt_u8*)pData[0].pData;

        if (NULL == pu8Surface)
        {
            (mt_void)MT_GO_UnlockSurface(s_hSubtSurface);
            return MT_SUCCESS;
        }

        if (s32Width * s32Height * pData[0].Bpp < pstInfo->unSubtitleParam.stGfx.h * pstInfo->unSubtitleParam.stGfx.w * 4)
        {
            (mt_void)MT_GO_UnlockSurface(s_hSubtSurface);
            return MT_SUCCESS;
        }

#endif
        //printf("pstInfo->unSubtitleParam.stGfx.s32BitWidth=%d  h=%d, w=%d, x=%d, y=%d\n",pstInfo->unSubtitleParam.stGfx.s32BitWidth,
        //pstInfo->unSubtitleParam.stGfx.h,pstInfo->unSubtitleParam.stGfx.w,
        //pstInfo->unSubtitleParam.stGfx.x,pstInfo->unSubtitleParam.stGfx.y);
        if(BITWIDTH_8_BITS == pstInfo->unSubtitleParam.stGfx.s32BitWidth)
        {
            for (i = 0; i < pstInfo->unSubtitleParam.stGfx.h; i++)
            {
                for (j = 0; j < pstInfo->unSubtitleParam.stGfx.w; j++)
                {
                    if (i * pstInfo->unSubtitleParam.stGfx.w + j > pstInfo->unSubtitleParam.stGfx.u32Len)
                    {
                        break;
                    }

                    u32PaletteIdx = pstInfo->unSubtitleParam.stGfx.pu8PixData[i * pstInfo->unSubtitleParam.stGfx.w + j];
                    if (u32PaletteIdx >= MT_UNF_SO_PALETTE_ENTRY)
                    {
                        break;
                    }
#ifdef ANDROID
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 3]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Alpha;
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 2]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Blue;
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 1]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Green;
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 0]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Red;
#else
                    pu8Surface[i * pData[0].Pitch + 4 * j + 3]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Alpha;
                    pu8Surface[i * pData[0].Pitch + 4 * j + 2]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Red;
                    pu8Surface[i * pData[0].Pitch + 4 * j + 1]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Green;
                    pu8Surface[i * pData[0].Pitch + 4 * j + 0]
                        = pstInfo->unSubtitleParam.stGfx.stPalette[u32PaletteIdx].u8Blue;

#endif
                }
            }
        }
        else if(BITWIDTH_32_BITS == pstInfo->unSubtitleParam.stGfx.s32BitWidth)
        {
            for (i = 0; i < pstInfo->unSubtitleParam.stGfx.h; i++)
            {
                for (j = 0; j < pstInfo->unSubtitleParam.stGfx.w; j++)
                {
                    if (i * (pstInfo->unSubtitleParam.stGfx.w) + j > (pstInfo->unSubtitleParam.stGfx.u32Len))
                    {
                        break;
                    }
#ifdef ANDROID
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 3]
                        =  pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*alpha*/
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 0]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Blue*/
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 1]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Green*/
                    bitmapBuffer[i * SurfaceWidth*4 + 4 * j + 2]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Red*/
#else
                    pu8Surface[i * pData[0].Pitch + 4 * j + 3]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*alpha*/
                    pu8Surface[i * pData[0].Pitch + 4 * j + 2]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Red*/
                    pu8Surface[i * pData[0].Pitch + 4 * j + 1]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Green*/
                    pu8Surface[i * pData[0].Pitch + 4 * j + 0]
                        = pstInfo->unSubtitleParam.stGfx.pu8PixData[u32Index++];/*u8Blue*/

#endif
                }
            }
        }


#ifdef ANDROID
       stSrcRect.s32X = pstInfo->unSubtitleParam.stGfx.x;
       stSrcRect.s32Y = pstInfo->unSubtitleParam.stGfx.y;
       stSrcRect.s32Width = pstInfo->unSubtitleParam.stGfx.w;
       stSrcRect.s32Height = pstInfo->unSubtitleParam.stGfx.h;

       MTADP_SURFACE_ATTR_S pstAttr;
       pstAttr.enPixelFormat=MTADP_PF_8888;
       pstAttr.u32Width=SurfaceWidth;
       pstAttr.u32Height=SurfaceHeight;

       MTADP_OSD_SubDrawBitmap(s_hSurface,bitmapBuffer,&pstAttr,&stSrcRect);

#else
        (mt_void)MT_GO_UnlockSurface(s_hSubtSurface);

#if USED_SURFACE_TO_FILE
        MTGO_ENC_ATTR_S stEncAttr;
        stEncAttr.ExpectType = MTGO_IMGTYPE_BMP;
        stEncAttr.QualityLevel = 1;
        MT_GO_EncodeToFile(s_hSubtSurface, MT_NULL, &stEncAttr);
#endif

#if 0
        rect.x = 0;
        rect.y = 0;
        rect.w = pstInfo->unSubtitleParam.stGfx.w;
        rect.h = pstInfo->unSubtitleParam.stGfx.h;
        (mt_void)ConvertRect(s_hSubtSurface, s_hLayerSurface, pstInfo->unSubtitleParam.stGfx.x, pstInfo->unSubtitleParam.stGfx.y,
                    pstInfo->unSubtitleParam.stGfx.w, pstInfo->unSubtitleParam.stGfx.h, &DstRect);

        memset(&stBlitOpt, 0, sizeof(MTGO_BLTOPT_S));
        stBlitOpt.EnableScale = MT_TRUE;


        s32Ret = MT_GO_Blit (s_hSubtSurface, &rect,
                       s_hLayerSurface, &DstRect,
                       &stBlitOpt);
#else

{
                TDE_HANDLE handle;
                TDE2_SURFACE_S stSrc;
                TDE2_SURFACE_S stDst;
                TDE2_OPT_S opt = {0};
                MTGO_PF_E fmt;
                mt_u32 tdeFmt;
                TDE2_RECT_S src_rect = {0};
                TDE2_RECT_S dst_rect = {0};

                src_rect.s32Xpos = 0;
                src_rect.s32Ypos = 0;
                src_rect.u32Width = pstInfo->unSubtitleParam.stGfx.w;
                src_rect.u32Height = pstInfo->unSubtitleParam.stGfx.h;
                (mt_void)ConvertRect(s_hSubtSurface, s_hLayerSurface, pstInfo->unSubtitleParam.stGfx.x, pstInfo->unSubtitleParam.stGfx.y,
                                    pstInfo->unSubtitleParam.stGfx.w, pstInfo->unSubtitleParam.stGfx.h, &DstRect);

                dst_rect.s32Xpos = DstRect.x;
                dst_rect.s32Ypos = DstRect.y;
                dst_rect.u32Width = DstRect.w;
                dst_rect.u32Height = DstRect.h;

                opt.enAluCmd = TDE2_ALUCMD_ROP;
                opt.enRopCode_Color = TDE2_ROP_COPYPEN;
                opt.enRopCode_Alpha = TDE2_ROP_COPYPEN;
                opt.u32Colorize = 0;


                opt.bResize = MT_TRUE;

                opt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
                opt.enColorKeySelect = TDE2_MASK_KEY_MATCH;

                MT_GO_GetSurfacePixelFormat(s_hSubtSurface, &fmt);
                MT_GO_ConvertTDEFmt(fmt, &tdeFmt);


                opt.unColorKeyValue.struCkARGB.stAlpha.u8CompMin = 0;
                opt.unColorKeyValue.struCkARGB.stAlpha.u8CompMax = 0;

                opt.unColorKeyValue.struCkARGB.stRed.bCompIgnore = MT_TRUE;
                opt.unColorKeyValue.struCkARGB.stGreen.bCompIgnore = MT_TRUE;
                opt.unColorKeyValue.struCkARGB.stBlue.bCompIgnore = MT_TRUE;



                MT_GO_MEMSurfaceToTDESurface(s_hSubtSurface, &stSrc);
                MT_GO_MEMSurfaceToTDESurface(s_hLayerSurface, &stDst);


                handle = MT_TDE2_BeginJob();

                MT_TDE2_Bitblit(handle, NULL, NULL, &stSrc, &src_rect,
                                            &stDst, &dst_rect, &opt);

                MT_TDE2_EndJob(handle, MT_FALSE, MT_TRUE, 500);
}
#endif

        (mt_void)MT_GO_RefreshLayer(s_hLayer, &DstRect);
#endif

        break;

    case MT_UNF_SUBTITLE_TEXT:
        if (NULL == pstInfo->unSubtitleParam.stText.pu8Data)
        {
            return MT_FAILURE;
        }

        MT_INFO_SUBT("OUTPUT: %s \n", pstInfo->unSubtitleParam.stText.pu8Data);
#ifdef ANDROID
       stSrcRect.s32X = 0;
       stSrcRect.s32Y = 0;
       stSrcRect.s32Width = pstInfo->unSubtitleParam.stText.w;
       stSrcRect.s32Height = pstInfo->unSubtitleParam.stText.h;
       AndroidConvertRect( pstInfo->unSubtitleParam.stText.x, pstInfo->unSubtitleParam.stText.y,
                        pstInfo->unSubtitleParam.stText.w, pstInfo->unSubtitleParam.stText.h, &stDesRect);
       mt_char* pszText = (mt_char*)pstInfo->unSubtitleParam.stText.pu8Data;

       MTADP_CCTEXT_ATTR_S pstCCTextAttr;
       pstCCTextAttr.u32BufLen = strlen(pszText);
       pstCCTextAttr.u32bgColor = 0xff000000;
       pstCCTextAttr.u32fgColor = 0xffffffff;
       pstCCTextAttr.u8fontSize = 21;

        MTADP_SURFACE_ATTR_S pstSurAttr;
        pstSurAttr.enPixelFormat=MTADP_PF_8888;
        pstSurAttr.u32Width=ScreenWidth;
        pstSurAttr.u32Height=ScreenHeight;

        MTADP_OSD_DrawCCText(s_hSurface, &pstSurAttr,&stDesRect, (mt_char*)pszText, &pstCCTextAttr);

#else
        rect.x = 0;
        rect.y = 0;
        rect.w = pstInfo->unSubtitleParam.stText.w;
        rect.h = pstInfo->unSubtitleParam.stText.h;
        (mt_void)ConvertRect(s_hSubtSurface, s_hLayerSurface, pstInfo->unSubtitleParam.stText.x, pstInfo->unSubtitleParam.stText.y,
                        pstInfo->unSubtitleParam.stText.w, pstInfo->unSubtitleParam.stText.h, &DstRect);

        if (MTGO_INVALID_HANDLE != s_hFont && MTGO_INVALID_HANDLE != s_hLayer)
        {
            mt_char* pszText = (mt_char*)pstInfo->unSubtitleParam.stText.pu8Data;
            //mt_u8 i=0;
            //int len = pstInfo->unSubtitleParam.stText.u32Len;
            //for(i=0;i<len;i++) printf("%c",pszText[i]);
            (mt_void)MT_GO_TextOutEx(s_hFont, s_hSubtSurface, pszText, &rect,
                MTGO_LAYOUT_WRAP | MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_BOTTOM);
            memset(&stBlitOpt, 0, sizeof(MTGO_BLTOPT_S));
            stBlitOpt.EnableScale = MT_TRUE;
            (mt_void)MT_GO_Blit (s_hSubtSurface, &rect,
                           s_hLayerSurface, &DstRect,
                           &stBlitOpt);

            (mt_void)MT_GO_RefreshLayer(s_hLayer, NULL);
        }
#endif

        break;

    case MT_UNF_SUBTITLE_ASS:
    default:
        break;
    }

    return MT_SUCCESS;
}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
mt_s32 Subt_Output_OnClear(mt_u32 u32UserData, mt_void *pArg)
#elif defined CONFIG_MT_CHIP_SYMPHONY6
mt_s32 Subt_Output_OnClear(ulong u32UserData, mt_void *pArg)
#endif
{
    MT_UNF_SO_CLEAR_PARAM_S* pParam = (MT_UNF_SO_CLEAR_PARAM_S*)pArg;

    mt_char TextClear[] = "";

    MT_INFO_SUBT("CLEAR Subtitle!\n");

#ifdef ANDROID
    MT_RECT_S stSrcRect = {0, 0, 0, 0};
    MT_RECT_S stDesRect = {0, 0, 0, 0};
    AndroidConvertRect(pParam->x, pParam->y, pParam->w, pParam->h,&stDesRect);

        MTADP_SURFACE_ATTR_S    pstSurAttr;
        pstSurAttr.u32Width = ScreenWidth;
        pstSurAttr.u32Height = ScreenHeight;
        pstSurAttr.enPixelFormat = MTADP_PF_8888;

     if (stDesRect.s32Width == 0 || stDesRect.s32Height == 0)
    {
        stDesRect.s32X = 0 ;
        stDesRect.s32Y = 0 ;
        stDesRect.s32Width = ScreenWidth;
        stDesRect.s32Height = ScreenHeight;

        MTADP_OSD_FillCCRect(s_hSurface,&pstSurAttr, &stDesRect, 0x00000000);

    }
    else
    {
        MTADP_OSD_FillCCRect(s_hSurface, &pstSurAttr,&stDesRect, 0x00000000);
    }
#else
     MT_RECT rect = {0, 0, 0, 0};
    (mt_void)ConvertRect(s_hSubtSurface, s_hLayerSurface, pParam->x, pParam->y, pParam->w, pParam->h, &rect);
    if (rect.w == 0 || rect.h == 0)
    {
        (mt_void)MT_GO_FillRect(s_hLayerSurface, NULL, 0x00000000, MTGO_COMPOPT_NONE);
    }
    else
    {
        (mt_void)MT_GO_FillRect(s_hLayerSurface, &rect, 0x00000000, MTGO_COMPOPT_NONE);
    }

    if (MTGO_INVALID_HANDLE != s_hFont)
    {
        (mt_void)MT_GO_TextOutEx(s_hFont, s_hLayerSurface, TextClear, &rect,
            MTGO_LAYOUT_WRAP | MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_BOTTOM);
    }
    (mt_void)MT_GO_RefreshLayer(s_hLayer, NULL);
#endif
    return MT_SUCCESS;
}

mt_s32 Subt_Output_Init(MT_HANDLE* phOut)
{

#ifdef ANDROID
     *phOut = SUBT_OUTPUT_HANDLE;
     return OsdInit();
#else
    if (MTGO_INVALID_HANDLE == s_hLayerSurface)
    {
        (mt_void)mtgoInit();
    }
    *phOut = SUBT_OUTPUT_HANDLE;
    return MT_SUCCESS;
#endif
}

mt_s32 Subt_Output_DeInit(MT_HANDLE hOut)
{
#ifdef ANDROID
     OsdDeInit();
     if(bitmapBuffer != MT_NULL)
     {
              free(bitmapBuffer);
              bitmapBuffer=MT_NULL;
      }
#else
    if (MTGO_INVALID_HANDLE != s_hLayerSurface)
    {
        (mt_void)mtgoDeInit();
    }
#endif
    return MT_SUCCESS;
}


