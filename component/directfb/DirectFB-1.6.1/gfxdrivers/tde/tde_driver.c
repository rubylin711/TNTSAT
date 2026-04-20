/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <config.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <core/state.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include "mt_tde_type.h"
#include "mt_tde_api.h"
#include "mt_tde_errcode.h"

#include "tde_2d.h"
#include "tde_gfxdriver.h"
#include "tde_driver.h"

/***************************** Macro Definition ******************************/
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/*Get A/R/G/B component*/
/*CNcomment:根据MT_COLOR获取ARGB */
#define GetARGB(c, a, r, g, b) \
    do \
    {\
        (a) = ((c) >> 24) & 0xff; (r) = ((c) >> 16) & 0xff; (g) = ((c) >> 8) & 0xff; (b) = (c) & 0xff; \
    } while (0)
    

/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/

/*Bulid colorfmt map table*/
/*CNcomment:建立像素格式影射表*/
struct
{
    DFBSurfacePixelFormat        DfbPF;
    TDE2_COLOR_FMT_E             TdePF;
} g_PF_MapTable[] = {
    {DSPF_RGB444,    TDE2_COLOR_FMT_RGB444  },
    {DSPF_ARGB4444,  TDE2_COLOR_FMT_ARGB4444},
    {DSPF_RGB555,    TDE2_COLOR_FMT_RGB555  },
    {DSPF_RGB16,     TDE2_COLOR_FMT_RGB565  },
    {DSPF_ARGB1555,  TDE2_COLOR_FMT_ARGB1555},
    {DSPF_RGB24,     TDE2_COLOR_FMT_RGB888  },
    {DSPF_ARGB,      TDE2_COLOR_FMT_ARGB8888},
};



/******************************* API declaration *****************************/



static MT_S32 ADP_DFBConvertFormat(DFBSurfacePixelFormat ColorFmt, TDE2_COLOR_FMT_E *pTDEColorFmt)
{
    MT_S32 i;

    for (i = 0; i < (MT_S32)ARRAY_SIZE(g_PF_MapTable); i++)
    {
        if (g_PF_MapTable[i].DfbPF == ColorFmt)
        {
            *pTDEColorFmt = g_PF_MapTable[i].TdePF;
            return MT_SUCCESS;
        }
    }

    return MT_FAILURE;
}

static MT_S32 ADP_DFBConverKey(unsigned long ColorKey, TDE2_COLOR_FMT_E PixelF, TDE2_COLORKEY_U *pKeyValue)
{
    MT_U8 a, r, g, b;
    MT_U32 RMin = 0, RMax = 0, GMin = 0, GMax = 0, BMin = 0, BMax = 0; 

    GetARGB(ColorKey, a, r, g, b);
    switch (PixelF)
    {
    case TDE2_COLOR_FMT_RGB444:
    case TDE2_COLOR_FMT_ARGB4444:
        RMin = r & 0xf0;
        RMax = r | 0x0f;
        GMin = g & 0xf0;
        GMax = g | 0x0f;
        BMin = b & 0xf0;
        BMax = b | 0x0f;
        break;
    case TDE2_COLOR_FMT_RGB555:
    case TDE2_COLOR_FMT_ARGB1555:
        RMin = r & 0xf8;
        RMax = r | 0x07;
        GMin = g & 0xf8;
        GMax = g | 0x07;
        BMin = b & 0xf8;
        BMax = b | 0x07;
        break;
    case TDE2_COLOR_FMT_RGB565:
        RMin = r & 0xf8;
        RMax = r | 0x07;
        GMin = g & 0xfc;
        GMax = g | 0x03;
        BMin = b & 0xf8;
        BMax = b | 0x07;
        break;
    case TDE2_COLOR_FMT_RGB888:
    case TDE2_COLOR_FMT_ARGB8888:
        RMin = RMax = r;
        GMin = GMax = g;
        BMin = BMax = b;
        break;
    default:
        break;
    }
    pKeyValue->struCkARGB.stRed.u8CompMin     = (MT_U8)RMin;
    pKeyValue->struCkARGB.stRed.u8CompMax     = (MT_U8)RMax;
    pKeyValue->struCkARGB.stGreen.u8CompMin   = (MT_U8)GMin;
    pKeyValue->struCkARGB.stGreen.u8CompMax   = (MT_U8)GMax;
    pKeyValue->struCkARGB.stBlue.u8CompMin    = (MT_U8)BMin;
    pKeyValue->struCkARGB.stBlue.u8CompMax    = (MT_U8)BMax;
    pKeyValue->struCkARGB.stRed.u8CompMask    = 0xff;
    pKeyValue->struCkARGB.stGreen.u8CompMask  = 0xff;
    pKeyValue->struCkARGB.stBlue.u8CompMask   = 0xff;
    pKeyValue->struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
    return MT_SUCCESS;
}

MT_S32 ADP_DFBGenerateTDEOpt(void *dev, TDE2_OPT_S* pTDEOpt, MT_BOOL bScale)
{
    TDE2_OPT_S stOpt;
    TDEDeviceData           *tdev     = dev;    

    memset(&stOpt, 0, sizeof(TDE2_OPT_S));

    if ((tdev->blittingflags & DSBLIT_SRC_COLORKEY) || (tdev->blittingflags & DSBLIT_DST_COLORKEY))
    {
        //MT_U8 clutIndex = 0;
        //MT_BOOL IsClut = MT_FALSE;

        if (tdev->blittingflags & DSBLIT_SRC_COLORKEY)
        {
            (MT_VOID)ADP_DFBConverKey(tdev->src_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
        }
        else
        {
            (MT_VOID)ADP_DFBConverKey(tdev->dst_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
            stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
        }
    }
    else
    {
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }

    /** process alpha */
    if ((tdev->blittingflags & DSBLIT_BLEND_ALPHACHANNEL) || (tdev->blittingflags & DSBLIT_BLEND_COLORALPHA))
    {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
        
        if (tdev->blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
        {
            stOpt.stBlendOpt.bPixelAlphaEnable =  MT_TRUE;  
            stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_CONFIG;

		if(tdev->dst_blend<=8&&tdev->dst_blend>=7)
		{
			tdev->dst_blend = tdev->dst_blend +2;
		}
		else if(tdev->dst_blend<=10&&tdev->dst_blend>=9)
		{
			tdev->dst_blend = tdev->dst_blend -2;
		}

		if(tdev->src_blend<=8&&tdev->src_blend>=7)
		{
			tdev->src_blend = tdev->src_blend +2;
		}
		else if(tdev->src_blend<=10&&tdev->src_blend>=9)
		{
			tdev->src_blend = tdev->src_blend -2;
		}
		
		stOpt.stBlendOpt.eSrc1BlendMode = tdev->dst_blend - 1 ;
		stOpt.stBlendOpt.eSrc2BlendMode = tdev->src_blend - 1;              
        }

        stOpt.u8GlobalAlpha =  0xff;
        /** process global alpha opt */
        if (tdev->blittingflags & DSBLIT_BLEND_COLORALPHA)
        {
            stOpt.stBlendOpt.bGlobalAlphaEnable = MT_TRUE;
            stOpt.u8GlobalAlpha = (tdev->color_pixel >> 24);
        }

        if (tdev->blittingflags & DSBLIT_SRC_PREMULTIPLY)
        {
            stOpt.stBlendOpt.bSrc2AlphaPremulti = MT_TRUE;
        }

        if (tdev->blittingflags & DSBLIT_DST_PREMULTIPLY)
        {
            stOpt.stBlendOpt.bSrc1AlphaPremulti = MT_TRUE;
        }

    }

    if (tdev->blittingflags & DSBLIT_COLORIZE)
    {
        stOpt.enAluCmd |= TDE2_ALUCMD_COLORIZE;
        stOpt.u32Colorize = tdev->color_pixel;
    }

    if (tdev->drawingflags & DSBLIT_XOR)
    {
        stOpt.enAluCmd |= TDE2_ALUCMD_ROP;
        stOpt.enRopCode_Alpha = stOpt.enRopCode_Color = TDE2_ROP_XORPEN;
    }

    /** deal with clip rect*/
#if 0    
    {
        //to be modify attention
        stOpt.enClipMode = TDE2_CLIPMODE_INSIDE;
        stOpt.stClipRect.s32Xpos = tdev->clip_region.x1;
        stOpt.stClipRect.s32Ypos = tdev->clip_region.y1;
        stOpt.stClipRect.u32Width = tdev->clip_region.x2 - tdev->clip_region.x1 + 1;
        stOpt.stClipRect.u32Height = tdev->clip_region.y2 - tdev->clip_region.y1 + 1;
    }
#endif
    /** deal scale*/
    if (bScale)
    {
        stOpt.bResize  = MT_TRUE;
        stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
    }
    memcpy(pTDEOpt, &stOpt, sizeof(stOpt));
    return MT_SUCCESS;
}

MT_S32 ADP_DFBGenerateTDEFillOpt(void *dev, TDE2_OPT_S* pTDEOpt, MT_BOOL bScale)
{
    TDE2_OPT_S stOpt;
    TDEDeviceData           *tdev     = dev;    

    memset(&stOpt, 0, sizeof(TDE2_OPT_S));

    if (tdev->drawingflags & DSDRAW_DST_COLORKEY)
    {
        (MT_VOID)ADP_DFBConverKey(tdev->dst_colorkey, tdev->src_surface.enColorFmt, &stOpt.unColorKeyValue);
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
    }
    else
    {
        stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }

    /** process alpha */
    if ((tdev->drawingflags & DSDRAW_BLEND) )
    {
        stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
        stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
        
        stOpt.stBlendOpt.bPixelAlphaEnable =  MT_TRUE;  
        stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_CONFIG;
		
	if(tdev->dst_blend<=8&&tdev->dst_blend>=7)
	{
		tdev->dst_blend = tdev->dst_blend +2;
	}
	else if(tdev->dst_blend<=10&&tdev->dst_blend>=9)
	{
		tdev->dst_blend = tdev->dst_blend -2;
	}

	if(tdev->src_blend<=8&&tdev->src_blend>=7)
	{
		tdev->src_blend = tdev->src_blend +2;
	}
	else if(tdev->src_blend<=10&&tdev->src_blend>=9)
	{
		tdev->src_blend = tdev->src_blend -2;
	}
	
	
        stOpt.stBlendOpt.eSrc1BlendMode = tdev->dst_blend - 1 ;
        stOpt.stBlendOpt.eSrc2BlendMode = tdev->src_blend - 1;              
        stOpt.u8GlobalAlpha = 255;
        if (tdev->drawingflags & DSDRAW_SRC_PREMULTIPLY)
        {
            stOpt.stBlendOpt.bSrc2AlphaPremulti = MT_TRUE;
        }

        if (tdev->drawingflags & DSDRAW_DST_PREMULTIPLY)
        {
            stOpt.stBlendOpt.bSrc1AlphaPremulti = MT_TRUE;
        }

    }

    if (tdev->drawingflags & DSDRAW_XOR)
    {
        stOpt.enAluCmd |= TDE2_ALUCMD_ROP;
        stOpt.enRopCode_Alpha = stOpt.enRopCode_Color = TDE2_ROP_XORPEN;
    }

    /** deal with clip rect*/
    memcpy(pTDEOpt, &stOpt, sizeof(stOpt));
    return MT_SUCCESS;
}

MT_S32 ADP_DFBSurfaceToTDESurface(CoreSurfaceBufferLock *pSurLock, CoreSurfaceConfig *pConfig, TDE2_SURFACE_S* pTDESurface)
{
    pTDESurface->u32PhyAddr = pSurLock->phys;
    if (MT_SUCCESS != ADP_DFBConvertFormat(pSurLock->buffer->format, &(pTDESurface->enColorFmt)))
    {   
       // D_DEBUG_AT( TDE, "unsupported format\n");
        return MT_FAILURE;
    }

    pTDESurface->u32Height = (MT_U32)pConfig->size.h;
    pTDESurface->u32Width  = (MT_U32)pConfig->size.w;
    pTDESurface->u32Stride = pSurLock->pitch;
    /** if clut format , reload the clut, attention the pSurface->Palette address must be phyaddress*/
    pTDESurface->pu8ClutPhyAddr = NULL;
    pTDESurface->bYCbCrClut = MT_FALSE;

    /** below to be modified*/
    pTDESurface->bAlphaMax255  = MT_TRUE;
    pTDESurface->bAlphaExt1555 = MT_FALSE;
    pTDESurface->u8Alpha0 = 0;
    pTDESurface->u8Alpha1 = 255;
    return MT_SUCCESS;
    
}

