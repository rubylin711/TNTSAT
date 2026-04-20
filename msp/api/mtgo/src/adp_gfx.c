/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include <string.h>

#include "mt_tde_type.h"
#include "mt_tde_api.h"
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mtgo_blit.h"
#include "mt_go_bliter.h"
#include "mtgo_adp_sys.h"
#include "mt_module_debug.h"
#include "adp_gfx.h"

/***************************** Macro Definition ******************************/
#define TIMEOUT 10
MT_BOOL g_SynFlag = MT_TRUE;

#define TDE_FLAG g_SynFlag

#define MAX_PIXELFORMAT 3
#define MAX_OPERATION_NUM 6

#define FORMAT_CLUT 0
#define FORMAT_RGB 1
#define FORMAT_YUV 2
//#define FORMAT_ALPHA    3
#define FORMAT_INVALID 4

#define GFX_TRUE 1  /*hardware support */
#define GFX_FALSE 0 /*hardware not support */
#define GFX_PART 2  /*hardware support partly */

#define BLIT_OPT_COPY 0 //lint -e750
#define BLIT_OPT_COLORKEY 0x1
#define BLIT_OPT_ALPHA 0x2
#define BLIT_OPT_COLORKEY2ALPHA (BLIT_OPT_COLORKEY | BLIT_OPT_ALPHA)
#define BLIT_OPT_ROP 0x4
#define BLIT_OPT_COLORKEY2ROP (BLIT_OPT_COLORKEY | BLIT_OPT_ROP)

#define OPERAT_TYPE_MAXNUM 5 /** all kinds of  operation type, include "LINE     CIRCLE    FILLRECT  RECTANGLE   ELLIPSE"*/

#define BLIT_CHECK_OPTINDEX(BlitMask, OptIndex) (OptIndex = BlitMask)
#define BLIT_CHECK_SRCFORMAT(PF, SrcIndex) \
    do {                                   \
	if (IS_RGB_FORMAT(PF)) {           \
	    SrcIndex = FORMAT_RGB;         \
	} else if (IS_YUV_FORMAT(PF)) {    \
	    SrcIndex = FORMAT_YUV;         \
	} else if (IS_CLUT_FORMAT(PF)) {   \
	    SrcIndex = FORMAT_CLUT;        \
	} else {                           \
	    SrcIndex = FORMAT_INVALID;     \
	}                                  \
    } while (0)

#define BLIT_CHECK_DSTFORMAT(PF, DstIndex) \
    do {                                   \
	if (IS_RGB_FORMAT(PF)) {           \
	    DstIndex = FORMAT_RGB;         \
	} else if (IS_YUV_FORMAT(PF)) {    \
	    DstIndex = FORMAT_YUV;         \
	} else if (IS_CLUT_FORMAT(PF)) {   \
	    DstIndex = FORMAT_CLUT;        \
	} else {                           \
	    DstIndex = FORMAT_INVALID;     \
	}                                  \
    } while (0)

#define ADP_ConvertRop(TdeRop, MtGoRop) (TdeRop = (TDE2_ROP_CODE_E)(MtGoRop))

#define CLEAN_WARN(a)  do {  \
                                               (a)=(a); \
                                        } while(0)

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static MT_BOOL g_MtgoInitTde = MT_FALSE;

#ifdef CONFIG_MT_FPGA_GPE
MT_BOOL fpga_b3dResize;
TDE2_POS_GROUP_S fpga_Scale3dDst;
#endif


/** maping pixel format table*/
struct
{
    MTGO_PF_E MtGoPF;
    TDE2_COLOR_FMT_E X5PF;
} g_MtGo_PF_MapTable[] = {
#ifndef MTGO_CODE_CUT
      { MTGO_PF_0444, TDE2_COLOR_FMT_RGB444 },
      { MTGO_PF_4444, TDE2_COLOR_FMT_ARGB4444 },
      { MTGO_PF_8565, TDE2_COLOR_FMT_ARGB8565 },
      { MTGO_PF_0888, TDE2_COLOR_FMT_RGB24 },
      { MTGO_PF_0555, TDE2_COLOR_FMT_RGB555 },
#endif
      { MTGO_PF_565, TDE2_COLOR_FMT_RGB565 },
      { MTGO_PF_1555, TDE2_COLOR_FMT_ARGB1555 },
//      { MTGO_PF_0888,  TDE2_COLOR_FMT_RGB24  },
      { MTGO_PF_8888, TDE2_COLOR_FMT_ARGB8888 },
      //{MTGO_PF_8565,  TDE2_COLOR_FMT_ARGB8565},
      { MTGO_PF_CLUT1, TDE2_COLOR_FMT_CLUT1 },
      { MTGO_PF_CLUT4, TDE2_COLOR_FMT_CLUT4 },
      { MTGO_PF_CLUT8, TDE2_COLOR_FMT_CLUT8 },
      { MTGO_PF_A1, TDE2_COLOR_FMT_A1 },
      { MTGO_PF_A8, TDE2_COLOR_FMT_A8 },
      { MTGO_PF_UYVY, TDE2_COLOR_FMT_CbY0CrY1 },
      { MTGO_PF_YUV8888, TDE2_COLOR_FMT_AYCbCr8888 },
	  {	MTGO_PF_SP_CMYK, TDE2_COLOR_FMT_JPG_SP_CMYK},
#ifdef CONFIG_MT_FPGA_GPE
      { MTGO_PF_YUV420, TDE2_COLOR_FMT_JPG_YCbCr420MBP },
    { MTGO_PF_ACLUT88, TDE2_COLOR_FMT_ACLUT88 },
    { MTGO_PF_233, TDE2_COLOR_FMT_RGB233 },
    { MTGO_PF_XYLC, TDE2_COLOR_FMT_XYLC },
     { MTGO_PF_YUV420TILE, TDE2_COLOR_FMT_TILE }
#endif     
  };

struct
{
    MTGO_PF_E MtGoPF;
    TDE2_MB_COLOR_FMT_E X5PF;
} g_MtGo_MBPF_MapTable[] = {
      { MTGO_PF_YUV400, TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP },
      { MTGO_PF_YUV420, TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP },
      { MTGO_PF_YUV422, TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP },
      { MTGO_PF_YUV422_V, TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP },
      { MTGO_PF_YUV444, TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP },
	  { MTGO_PF_SP_CMYK, TDE2_MB_COLOR_FMT_JPG_SP_CMYK},	  
      { MTGO_PF_YUV420TILE, TDE2_MB_COLOR_FMT_TILE }
  };
/******************************* API declaration *****************************/
#if 0
mt_s32 ADP_GFXBlitBility(const MTGO_SURFACE_S *pSrcSurface, const MTGO_SURFACE_S *pDstSurface, const MTGO_BLTOPT2_S *pBlitOpt)
{
    return GFX_TRUE;
}

mt_s32 ADP_GFXStretchBlitBility(const MTGO_SURFACE_S *pSrcSurface, const MTGO_SURFACE_S *pDstSurface, const MTGO_BLTOPT2_S *pBlitOpt)
{
    return GFX_TRUE;
}

mt_s32 ADP_GFXOperateBility(const MTGO_SURFACE_S *pDstSurface, const GFX_OPT_TYPE_E GfxOpt, const MTGO_BLTOPT2_S *pBlitOpt)
{
    return GFX_TRUE;
}
#endif
mt_s32 ADP_ConvertFormat(MTGO_PF_E ColorFmt, TDE2_COLOR_FMT_E *pX5ColorFmt)
{
    mt_s32 i;

    for (i = 0; i < (mt_s32)MTGO_ARRAY_SIZE(g_MtGo_PF_MapTable); i++) {
	if (g_MtGo_PF_MapTable[i].MtGoPF == ColorFmt) {
	    *pX5ColorFmt = g_MtGo_PF_MapTable[i].X5PF;
	    return MT_SUCCESS;
	}
    }

    MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
    return MTGO_ERR_INTERNAL;
}
static mt_s32 ADP_ConvertMBFormat(MTGO_PF_E ColorFmt, TDE2_MB_COLOR_FMT_E *pX5ColorFmt)
{
    mt_s32 i;

    for (i = 0; i < (mt_s32)MTGO_ARRAY_SIZE(g_MtGo_MBPF_MapTable); i++) {
	if (g_MtGo_MBPF_MapTable[i].MtGoPF == ColorFmt) {
	    *pX5ColorFmt = g_MtGo_MBPF_MapTable[i].X5PF;
	    return MT_SUCCESS;
	}
    }

    MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
    return MTGO_ERR_INTERNAL;
}

static mt_s32 ADP_ColorExpand(const MTGO_SURFACE_S *pSrcSurface, MT_COLOR Color, mt_u32 *pFillData)
{
    switch (pSrcSurface->PixelFormat) {
#ifndef MTGO_CODE_CUT
    case MTGO_PF_0444:
    case MTGO_PF_4444:
	*pFillData = (mt_u32)RGB32TO4444(Color);
	break;
    case MTGO_PF_0555:
#endif
    case MTGO_PF_1555:

	*pFillData = (mt_u32)RGB32TO1555(Color);
	break;

    //case MTGO_PF_0888:
    case MTGO_PF_8888:
	*pFillData = (mt_u32)Color;
	break;

    case MTGO_PF_565:
	*pFillData = RGB32TO565(Color);
	break;
#ifndef MTGO_CODE_CUT
    case MTGO_PF_8565:
	*pFillData = RGB32TO8565(Color);
	break;
#endif
    case MTGO_PF_CLUT1:
    case MTGO_PF_CLUT4:
    case MTGO_PF_CLUT8:    
    case MTGO_PF_A1:
    case MTGO_PF_A8:
        *pFillData = Color;
        break;
    default:

    case MTGO_PF_UYVY:
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV420TILE:
	*pFillData = (mt_u32)Color;
	break;

    case MTGO_PF_YUV8888:
	*pFillData = (mt_u32)Color;
	break;
    case MTGO_PF_0888:
    case MTGO_PF_ACLUT88:
        *pFillData = (mt_u32)Color;
        break;
    case MTGO_PF_233:
    case MTGO_PF_XYLC:
          *pFillData = Color;
        break;

	MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

//lint -e550
#ifndef MTGO_CODE_CUT
#define KEY_MIN(color, bit_num) (color & (((1 << bit_num) - 1) << (8 - bit_num)))
#define KEY_MAX(color, bit_num) (color | ((1 << (8 - bit_num)) - 1))

static mt_s32 ADP_ConverKey(MT_COLOR ColorKey, MTGO_PF_E PixelF, TDE2_COLORKEY_U *pKeyValue)
{
    mt_u8 a, r, g, b;
    mt_u32 RMin = 0, RMax = 0, GMin = 0, GMax = 0, BMin = 0, BMax = 0;

    GetARGB(ColorKey, a, r, g, b);
    CLEAN_WARN(a);
    switch (PixelF) {
    case MTGO_PF_0444:
    case MTGO_PF_4444:
	RMin = r & 0xf0;
	RMax = r | 0x0f;
	GMin = g & 0xf0;
	GMax = g | 0x0f;
	BMin = b & 0xf0;
	BMax = b | 0x0f;
	break;
    case MTGO_PF_0555:
    case MTGO_PF_1555:
	RMin = r & 0xf8;
	RMax = r | 0x07;
	GMin = g & 0xf8;
	GMax = g | 0x07;
	BMin = b & 0xf8;
	BMax = b | 0x07;
	break;
    case MTGO_PF_565:
    case MTGO_PF_8565:
	RMin = r & 0xf8;
	RMax = r | 0x07;
	GMin = g & 0xfc;
	GMax = g | 0x03;
	BMin = b & 0xf8;
	BMax = b | 0x07;
	break;
    case MTGO_PF_0888:
    case MTGO_PF_8888:
	RMin = RMax = r;
	GMin = GMax = g;
	BMin = BMax = b;
	break;
    case MTGO_PF_233:
        RMin = KEY_MIN(r, 2);
        GMin = KEY_MIN(g, 3);
        BMin = KEY_MIN(b, 3);
        RMax = KEY_MAX(r, 2);
        GMax = KEY_MAX(g, 3);
        BMax = KEY_MAX(b, 3);
	break;
     case MTGO_PF_UYVY:
     case MTGO_PF_YUV420:
        RMin = r;
        GMin = g;
        BMin = b;
        RMax = r;
        GMax = g;
        BMax = b;
        break;
    default:
	MT_ERR_MTGO(" conver key unsupport\n");
	break;
    }

    pKeyValue->struCkARGB.stRed.u8CompMin = (mt_u8)RMin;
    pKeyValue->struCkARGB.stRed.u8CompMax = (mt_u8)RMax;
    pKeyValue->struCkARGB.stGreen.u8CompMin = (mt_u8)GMin;
    pKeyValue->struCkARGB.stGreen.u8CompMax = (mt_u8)GMax;
    pKeyValue->struCkARGB.stBlue.u8CompMin = (mt_u8)BMin;
    pKeyValue->struCkARGB.stBlue.u8CompMax = (mt_u8)BMax;

    pKeyValue->struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
    return MT_SUCCESS;
}
#endif
//lint +e550

mt_s32 ADP_MEMSurfaceToTDESurface(const MTGO_SURFACE_S *pSurface, TDE2_SURFACE_S *pTDESurface)
{
    pTDESurface->u32PhyAddr = pSurface->Data[0].pPhyData;
	pTDESurface->u32VirAddr = (ulong)pSurface->Data[0].pData;
    if (MT_SUCCESS != ADP_ConvertFormat(pSurface->PixelFormat, &(pTDESurface->enColorFmt))) {
	MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
	return MTGO_ERR_INVPIXELFMT;
    }

    pTDESurface->u32Height = (mt_u32)pSurface->Height;
    pTDESurface->u32Width = (mt_u32)pSurface->Width;
    pTDESurface->u32Stride = pSurface->Data[0].Pitch;
    /** if clut format , reload the clut, attention the pSurface->Palette address must be phyaddress*/
    if (IS_CLUT_FORMAT(pSurface->PixelFormat)) {
	pTDESurface->pu8ClutPhyAddr = pSurface->pPhyPalette;
	pTDESurface->pu8ClutVirAddr = (mt_u8 *)pSurface->Palette;
	pTDESurface->bYCbCrClut = MT_FALSE;
    } else {
	pTDESurface->pu8ClutPhyAddr = 0;
	pTDESurface->pu8ClutVirAddr = NULL;
	pTDESurface->bYCbCrClut = MT_FALSE;
    }
    /** below to be modified*/
    pTDESurface->bAlphaMax255 = MT_TRUE;
    pTDESurface->bAlphaExt1555 = MT_TRUE;
    pTDESurface->u8Alpha0 = 0;
    pTDESurface->u8Alpha1 = 255;
    return MT_SUCCESS;
}

mt_s32 ADP_MEMSurfaceToTDEMBSurface(const MTGO_SURFACE_S *pSurface, TDE2_MB_S *pTDEMBSurface)
{
    mt_s32 s32Ret;

    s32Ret = ADP_ConvertMBFormat(pSurface->PixelFormat, &pTDEMBSurface->enMbFmt);
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_INVPIXELFMT;
    }
    pTDEMBSurface->u32YPhyAddr = pSurface->Data[0].pPhyData;
	pTDEMBSurface->u32YVirAddr = (ulong)pSurface->Data[0].pData;
    pTDEMBSurface->u32CbCrPhyAddr = pSurface->Data[1].pPhyData;
	pTDEMBSurface->u32CbCrVirAddr = (ulong)pSurface->Data[1].pData;
    pTDEMBSurface->u32YStride = pSurface->Data[0].Pitch;
    pTDEMBSurface->u32CbCrStride = pSurface->Data[1].Pitch;
    pTDEMBSurface->u32YWidth = (mt_u32)pSurface->Width;
    pTDEMBSurface->u32YHeight = (mt_u32)pSurface->Height;
    return MT_SUCCESS;
}

#ifdef CONFIG_MT_FPGA_GPE
static mt_s32 ADP_CSCBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, 
   MT_BOOL bScale, const MTGO_BLTOPT2_S *pBlitOpt)

#else
static mt_s32 ADP_CSCBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, MT_BOOL bScale)
#endif
{
    TDE2_SURFACE_S TDEDstSurface;
    TDE2_MBOPT_S stMbOpt;
	TDE2_OPT_S stOpt = {0};
    TDE2_MB_S TDEMBSurface;
	TDE2_SURFACE_S TDESurface;
    TDE2_RECT_S SrcRect = { pSrcRect->x, pSrcRect->y, (mt_u32)pSrcRect->w, (mt_u32)pSrcRect->h };
    TDE2_RECT_S DstRect = { pDstRect->x, pDstRect->y, (mt_u32)pDstRect->w, (mt_u32)pDstRect->h };
    TDE_HANDLE s32Handle;
    mt_s32 s32Ret;
    /** doing flicker resisting when doing scaling*/
    MTGO_MemSet(&stMbOpt, 0, sizeof(TDE2_MBOPT_S));
    /** convert source and dest surface to TDE format */
    s32Ret = ADP_MEMSurfaceToTDEMBSurface(pSrcSurface, &TDEMBSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    /** prepare dst surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDEDstSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    /** convert surface to X5 marco block surface */
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	MTGO_ERROR(s32Handle);
	return MTGO_ERR_DEPEND_TDE;
    }
#ifndef MTGO_CODE_CUT
    /** enable clip rect*/
    if (IS_HAVE_CLIPRECT(pDstSurface)) {
	stMbOpt.enClipMode = TDE2_CLIPMODE_INSIDE;
	stMbOpt.stClipRect.s32Xpos = pDstSurface->ClipRect.x;
	stMbOpt.stClipRect.s32Ypos = pDstSurface->ClipRect.y;
	stMbOpt.stClipRect.u32Width = (mt_u32)pDstSurface->ClipRect.w;
	stMbOpt.stClipRect.u32Height = (mt_u32)pDstSurface->ClipRect.h;
    }
#endif
    /** set the scale param*/
    if (MT_TRUE == bScale) {
	stMbOpt.enResize = TDE2_MBRESIZE_QUALITY_LOW;
    }
   // printf("<%s> : <%d> \n", __FUNCTION__, __LINE__);
#ifdef CONFIG_MT_FPGA_GPE
      if (pBlitOpt->ColorKeyFrom != MTGO_CKEY_NONE) {
	mt_u8 clutIndex = 0;
	MT_BOOL IsClut = MT_FALSE;

	switch (pBlitOpt->ColorKeyFrom) {
	case MTGO_CKEY_SRC:
		if(pSrcSurface != NULL)
		{
		    if (IS_CLUT_FORMAT(pSrcSurface->PixelFormat)) {
			clutIndex = (mt_u8)pSrcSurface->ColorKey;
			IsClut = MT_TRUE;
		    } else {
			(mt_void) ADP_ConverKey(pSrcSurface->ColorKey, pSrcSurface->PixelFormat, &stOpt.unColorKeyValue);
            
		   // printf("<%s> : <%d> %d %x %x\n", __FUNCTION__, __LINE__, pSrcSurface->PixelFormat, pSrcSurface->ColorKey, stOpt.unColorKeyValue);
		    }
		    stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
		}
	    break;
	case MTGO_CKEY_DST:
	    if (IS_CLUT_FORMAT(pDstSurface->PixelFormat)) {
		clutIndex = (mt_u8)pDstSurface->ColorKey;
		IsClut = MT_TRUE;
	    } else {
		(mt_void) ADP_ConverKey(pDstSurface->ColorKey, pDstSurface->PixelFormat, &stOpt.unColorKeyValue);
	    }
	    stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
	    break;
	default:
	    return MTGO_ERR_UNSUPPORTED;
	}

	if (IsClut == MT_TRUE) {
	    stOpt.unColorKeyValue.struCkClut.stClut.u8CompMin = clutIndex;
	    stOpt.unColorKeyValue.struCkClut.stClut.u8CompMax = clutIndex;
	    stOpt.unColorKeyValue.struCkClut.stAlpha.bCompIgnore = MT_TRUE;
	}
    } 
      else
    {
	stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }
#ifdef CONFIG_MT_FPGA_GPE

    if(fpga_b3dResize)
    {
        stOpt.b3dResize = MT_TRUE;
        stOpt.Scale3dDst.pos00.x = fpga_Scale3dDst.pos00.x;
        stOpt.Scale3dDst.pos00.y = fpga_Scale3dDst.pos00.y;
        stOpt.Scale3dDst.pos10.x = fpga_Scale3dDst.pos10.x;
        stOpt.Scale3dDst.pos10.y = fpga_Scale3dDst.pos10.y;
        stOpt.Scale3dDst.pos01.x = fpga_Scale3dDst.pos01.x;
        stOpt.Scale3dDst.pos01.y = fpga_Scale3dDst.pos01.y;
        stOpt.Scale3dDst.pos11.x = fpga_Scale3dDst.pos11.x;
        stOpt.Scale3dDst.pos11.y = fpga_Scale3dDst.pos11.y;
        stMbOpt.enResize = TDE2_MBRESIZE_QUALITY_LOW;
    }
    fpga_b3dResize = FALSE;
#endif      
#endif

#if 0
    /** semi-planar YUV to packet */
    s32Ret = MT_TDE2_MbBlit(s32Handle, &TDEMBSurface, &SrcRect, &TDEDstSurface, &DstRect, &stMbOpt);
#else

	ADP_TDEMBSurfaceToTDESurface(&TDEMBSurface, &TDESurface);
	ADP_TDEMBOptToTDEOpt(&stMbOpt, &stOpt);	
	s32Ret = MT_TDE2_Bitblit(s32Handle, NULL, NULL,
	                       &TDESurface, &SrcRect, &TDEDstSurface,
	                       &DstRect, &stOpt);
#endif

    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	(mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	return MTGO_ERR_DEPEND_TDE;
    }

    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }
    return MT_SUCCESS;
}

mt_s32 ADP_GenerateTDEOpt(const MTGO_SURFACE_S *pSrcSurface,
                                 const MTGO_SURFACE_S *pDstSurface,
                                 const MTGO_BLTOPT2_S *pBlitOpt,
                                 TDE2_OPT_S *pTDEOpt, MT_BOOL bScale)
{
    TDE2_OPT_S stOpt;

    MTGO_MemSet(&stOpt, 0, sizeof(TDE2_OPT_S));
#ifndef MTGO_CODE_CUT
    /** handle colorkey */
    if (pBlitOpt->ColorKeyFrom != MTGO_CKEY_NONE) {
	mt_u8 clutIndex = 0;
	MT_BOOL IsClut = MT_FALSE;

	switch (pBlitOpt->ColorKeyFrom) {
	case MTGO_CKEY_SRC:
		if(pSrcSurface != NULL)
		{
		    if (IS_CLUT_FORMAT(pSrcSurface->PixelFormat)) {
			clutIndex = (mt_u8)pSrcSurface->ColorKey;
			IsClut = MT_TRUE;
		    } else {
			(mt_void) ADP_ConverKey(pSrcSurface->ColorKey, pSrcSurface->PixelFormat, &stOpt.unColorKeyValue);
		    }
		    stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_FOREGROUND;
		}
	    break;
	case MTGO_CKEY_DST:
	    if (IS_CLUT_FORMAT(pDstSurface->PixelFormat)) {
		clutIndex = (mt_u8)pDstSurface->ColorKey;
		IsClut = MT_TRUE;
	    } else {
		(mt_void) ADP_ConverKey(pDstSurface->ColorKey, pDstSurface->PixelFormat, &stOpt.unColorKeyValue);
	    }
	    stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_BACKGROUND;
	    break;
	default:
	    return MTGO_ERR_UNSUPPORTED;
	}

	if (IsClut == MT_TRUE) {
	    stOpt.unColorKeyValue.struCkClut.stClut.u8CompMin = clutIndex;
	    stOpt.unColorKeyValue.struCkClut.stClut.u8CompMax = clutIndex;
	    stOpt.unColorKeyValue.struCkClut.stAlpha.bCompIgnore = MT_TRUE;
	}
    } else
#endif
    {
	stOpt.enColorKeyMode = TDE2_COLORKEY_MODE_NONE;
    }
#ifndef MTGO_CODE_CUT
    /** process ROP */
    if (pBlitOpt->EnableRop) {
	stOpt.enAluCmd = TDE2_ALUCMD_ROP;
	ADP_ConvertRop(stOpt.enRopCode_Color, pBlitOpt->Rop);
	ADP_ConvertRop(stOpt.enRopCode_Alpha, pBlitOpt->RopAlpha);
    } else {

	stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
	stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
	/** process pixel alpha opt */
	switch (pBlitOpt->PixelAlphaComp) {
	/*   case MTGO_COMPOPT_AKS:
            stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
            stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
            break;
        case MTGO_COMPOPT_AKD:
            stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_BACKGROUND;
            stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
            break;*/

	case MTGO_COMPOPT_NONE:
	    stOpt.enAluCmd = TDE2_ALUCMD_NONE;
	    break;
	case MTGO_COMPOPT_CLEAR:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_CLEAR;			
		break;
	case MTGO_COMPOPT_SRC:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRC;			
		break;			
	case MTGO_COMPOPT_SRCOVER:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCOVER;			
		break;
	case MTGO_COMPOPT_DSTOVER:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_DSTOVER;			
		break;	
	case MTGO_COMPOPT_SRCIN:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCIN;			
		break;
	case MTGO_COMPOPT_DSTIN:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_DSTIN;			
		break;		
	case MTGO_COMPOPT_SRCOUT:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCOUT;			
		break;
	case MTGO_COMPOPT_DSTOUT:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_DSTOUT;			
		break;			
	case MTGO_COMPOPT_SRCATOP:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_SRCATOP;			
		break;
	case MTGO_COMPOPT_DSTATOP:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_DSTATOP;			
		break;	
	case MTGO_COMPOPT_ADD:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_ADD;			
		break;
	case MTGO_COMPOPT_XOR:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_XOR;			
		break;		
	case MTGO_COMPOPT_DST:
		stOpt.stBlendOpt.eBlendCmd = TDE2_BLENDCMD_DST;			
		break;			
	default:
	    stOpt.enAluCmd = TDE2_ALUCMD_NONE;
	    break;
	}

	if(pBlitOpt->EnablePixelAlpha)
	{
		stOpt.stSurfaceCfg.enSrcPreMult = MT_TRUE;
//		stOpt.stBlendOpt.bSrc2AlphaPremulti = MT_TRUE;
	}
	/** process global alpha opt */
	switch (pBlitOpt->EnableGlobalAlpha) {
	case MT_TRUE:
		if(pSrcSurface != NULL)
		{
	    	stOpt.u8GlobalAlpha = pSrcSurface->Alpha;
			stOpt.stSurfaceCfg.enSrcGlobalAlpha = MT_TRUE;
		}
	    break;
	case MT_FALSE:
	    stOpt.u8GlobalAlpha = 0xff;
	    break;
	default:
	    stOpt.u8GlobalAlpha = 0xff;
	    break;
	}
    }

				

    /** global alpha enable pixel alpha disable*/
    if ((stOpt.enAluCmd == TDE2_ALUCMD_NONE) && (pBlitOpt->EnableGlobalAlpha)) {
	stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
	stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
    }
    /** deal with clip rect*/
    if (IS_HAVE_CLIPRECT(pDstSurface)) {
	stOpt.enClipMode = TDE2_CLIPMODE_INSIDE;
	stOpt.stClipRect.s32Xpos = pDstSurface->ClipRect.x;
	stOpt.stClipRect.s32Ypos = pDstSurface->ClipRect.y;
	stOpt.stClipRect.u32Width = (mt_u32)pDstSurface->ClipRect.w;
	stOpt.stClipRect.u32Height = (mt_u32)pDstSurface->ClipRect.h;
    }
	if(pSrcSurface != NULL)
	{
	    if (IS_CLUT_FORMAT(pSrcSurface->PixelFormat)) {
		stOpt.bClutReload = MT_TRUE;
	    }
	}
#else
    stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
    stOpt.enAluCmd = TDE2_ALUCMD_NONE;
    stOpt.u8GlobalAlpha = 0xff;
#endif

    /** deal scale*/
    if (bScale) {
	stOpt.bResize = MT_TRUE;
	stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
    }
    memcpy(pTDEOpt, &stOpt, sizeof(stOpt));
    return MT_SUCCESS;
}

#ifndef MTGO_CODE_CUT
static TDE2_COLOR_FMT_E ADP_ConvertA2NOAFormat(TDE2_COLOR_FMT_E enColorFmt)
{
    switch (enColorFmt) {
    case TDE2_COLOR_FMT_ARGB4444:
	return TDE2_COLOR_FMT_RGB444;
    case TDE2_COLOR_FMT_ARGB1555:
	return TDE2_COLOR_FMT_RGB555;
//#ifndef RGB24
    case TDE2_COLOR_FMT_ARGB8888:
	return TDE2_COLOR_FMT_RGB888;
//#endif
    default:
	return enColorFmt;
    }

    //return enColorFmt;
}
#endif

static mt_s32 ADP_GFXBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt, MT_BOOL bScale)
{
    mt_s32 s32Ret = MT_SUCCESS;
    TDE_HANDLE s32Handle;
    TDE2_SURFACE_S TDESrcSurface, TDEDstSurface;
    TDE2_RECT_S SrcRect = { pSrcRect->x, pSrcRect->y, (mt_u32)pSrcRect->w, (mt_u32)pSrcRect->h };
    TDE2_RECT_S DstRect = { pDstRect->x, pDstRect->y, (mt_u32)pDstRect->w, (mt_u32)pDstRect->h };
    TDE2_OPT_S stOpt;

    MTGO_MemSet(&stOpt, 0, sizeof(TDE2_OPT_S));
    /** prepare surface*/
    s32Ret = ADP_MEMSurfaceToTDESurface(pSrcSurface, &TDESrcSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    /** prepare dst surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDEDstSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

#if 0
    printf("+++++ src  Addr 0x%x pf %d w %d h %d p %d \n",pSrcSurface->Data[0].pPhyData,pSrcSurface->PixelFormat,
        pSrcSurface->Width,pSrcSurface->Height,pSrcSurface->Data[0].Pitch);
    
    printf("+++++ dst  Addr 0x%x pf %d w %d h %d p %d \n",pDstSurface->Data[0].pPhyData,pDstSurface->PixelFormat,
        pDstSurface->Width,pDstSurface->Height,pDstSurface->Data[0].Pitch);
#endif

    /** prepare alpha,key,rop opt */
    s32Ret = ADP_GenerateTDEOpt(pSrcSurface, pDstSurface, pBlitOpt, &stOpt, bScale);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

#ifndef MTGO_CODE_CUT
    if ((stOpt.enAluCmd == TDE2_ALUCMD_NONE) && (pBlitOpt->EnableGlobalAlpha)) {
	TDESrcSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDESrcSurface.enColorFmt);
	TDEDstSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDEDstSurface.enColorFmt);

	/** global alpha enable pixel alpha disable*/
	stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
	stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
    }
#endif

    /** do tde job*/
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	MTGO_ERROR(MTGO_ERR_INTERNAL);
	return MTGO_ERR_INTERNAL;
    }

    s32Ret = MT_TDE2_Bitblit(s32Handle, &TDEDstSurface, &DstRect, &TDESrcSurface, &SrcRect, &TDEDstSurface,
                             &DstRect, &stOpt);
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	(mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	return MTGO_ERR_DEPEND_TDE;
    }

    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }
    return MT_SUCCESS;
}

static mt_s32 ADP_GFXBlitEx(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pBckSurface, const MT_RECT* pBckRect, 
	const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt, MT_BOOL bScale)
{
    mt_s32 s32Ret = MT_SUCCESS;
    TDE_HANDLE s32Handle;
    TDE2_SURFACE_S TDESrcSurface, TDEDstSurface, TDEBckSurface;
    TDE2_RECT_S SrcRect = { pSrcRect->x, pSrcRect->y, (mt_u32)pSrcRect->w, (mt_u32)pSrcRect->h };
	TDE2_RECT_S BckRect = { pBckRect->x, pBckRect->y, (mt_u32)pBckRect->w, (mt_u32)pBckRect->h };
    TDE2_RECT_S DstRect = { pDstRect->x, pDstRect->y, (mt_u32)pDstRect->w, (mt_u32)pDstRect->h };
    TDE2_OPT_S stOpt;

    MTGO_MemSet(&stOpt, 0, sizeof(TDE2_OPT_S));
    /** prepare surface*/
    s32Ret = ADP_MEMSurfaceToTDESurface(pSrcSurface, &TDESrcSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
    /** prepare dst surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDEDstSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }
	/** prepare back surface */
	s32Ret = ADP_MEMSurfaceToTDESurface(pBckSurface, &TDEBckSurface);
	if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
	}


#if 0
    printf("+++++ src  Addr 0x%x pf %d w %d h %d p %d \n",pSrcSurface->Data[0].pPhyData,pSrcSurface->PixelFormat,
        pSrcSurface->Width,pSrcSurface->Height,pSrcSurface->Data[0].Pitch);
    
    printf("+++++ dst  Addr 0x%x pf %d w %d h %d p %d \n",pDstSurface->Data[0].pPhyData,pDstSurface->PixelFormat,
        pDstSurface->Width,pDstSurface->Height,pDstSurface->Data[0].Pitch);

    printf("+++++ bck  Addr 0x%x pf %d w %d h %d p %d \n",pBckSurface->Data[0].pPhyData,pBckSurface->PixelFormat,
        pBckSurface->Width,pBckSurface->Height,pBckSurface->Data[0].Pitch);	
#endif

    /** prepare alpha,key,rop opt */
    s32Ret = ADP_GenerateTDEOpt(pSrcSurface, pDstSurface, pBlitOpt, &stOpt, bScale);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

#ifndef MTGO_CODE_CUT
    if ((stOpt.enAluCmd == TDE2_ALUCMD_NONE) && (pBlitOpt->EnableGlobalAlpha)) {
	TDESrcSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDESrcSurface.enColorFmt);
	TDEDstSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDEDstSurface.enColorFmt);

	/** global alpha enable pixel alpha disable*/
	stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
	stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
    }
#endif

    /** do tde job*/
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	MTGO_ERROR(MTGO_ERR_INTERNAL);
	return MTGO_ERR_INTERNAL;
    }

    s32Ret = MT_TDE2_Bitblit(s32Handle, &TDEBckSurface, &BckRect, &TDESrcSurface, &SrcRect, &TDEDstSurface,
                             &DstRect, &stOpt);
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	(mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	return MTGO_ERR_DEPEND_TDE;
    }

    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }
    return MT_SUCCESS;
}


static mt_s32 ADP_FillRect(const MTGO_SURFACE_S *pDstSurface, const MTGO_OPRECT_S *pOpRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_u32 u32FillData;
    TDE2_SURFACE_S TDESurface;
    mt_s32 s32Ret;
    TDE2_RECT_S Rect = { pOpRect->Rect.x, pOpRect->Rect.y, (mt_u32)pOpRect->Rect.w, (mt_u32)pOpRect->Rect.h };
    TDE_HANDLE s32Handle;
    MTGO_COMPOPT_E CompositeOpt = pBlitOpt->PixelAlphaComp;

    /** convert memsurface to tde surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDESurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    if ((CompositeOpt == MTGO_COMPOPT_NONE) && (!IS_HAVE_CLIPRECT(pDstSurface))) {
	/** quick fill */

	/** color convertion */
	s32Ret = ADP_ColorExpand(pDstSurface, pOpRect->Color, &u32FillData);
	if (s32Ret != MT_SUCCESS) {
	    MTGO_ERROR(s32Ret);
	    return s32Ret;
	}

	/** call X5 quick fill */
	s32Handle = MT_TDE2_BeginJob();
	if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	    MTGO_ERROR(s32Handle);
	    return MTGO_ERR_DEPEND_TDE;
	}

	//s32Ret = MT_TDE2_QuickFill(s32Handle, &TDESurface, &Rect, u32FillData);
	s32Ret = MT_TDE2_QuickFill(s32Handle, &TDESurface, &Rect, pOpRect->Color);
	if (s32Ret != MT_SUCCESS) {
	    MTGO_ERROR(s32Ret);
	    (mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	    return MTGO_ERR_DEPEND_TDE;
	}

	s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	    MTGO_ERROR(s32Ret);
	    return MTGO_ERR_DEPEND_TDE;
	}
    }
#ifndef MTGO_CODE_CUT
    else {
	TDE2_OPT_S TDEOpt;

	MTGO_MemSet(&TDEOpt, 0, sizeof(TDE2_OPT_S));
	/** using X5 SolidDraw do alpha blending */

	s32Handle = MT_TDE2_BeginJob();
	if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	    MTGO_ERROR(s32Handle);
	    return MTGO_ERR_DEPEND_TDE;
	}

#if 0
	TDEOpt.enAluCmd = TDE2_ALUCMD_BLEND;
	TDEOpt.u8GlobalAlpha = 0xff; /*> to be modify global alpha should be disable */
	switch (CompositeOpt) {
	/*   case MTGO_COMPOPT_AKD:
                TDEOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_BACKGROUND;
                break;
            case MTGO_COMPOPT_AKS:
                TDEOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
                break;*/
	default:
	    TDEOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_NORM;
	    break;
	}
	/** set clip rect*/
	if (IS_HAVE_CLIPRECT(pDstSurface)) {
	    TDEOpt.enClipMode = TDE2_CLIPMODE_INSIDE;
	    TDEOpt.stClipRect.s32Xpos = pDstSurface->ClipRect.x;
	    TDEOpt.stClipRect.s32Ypos = pDstSurface->ClipRect.y;
	    TDEOpt.stClipRect.u32Width = (mt_u32)pDstSurface->ClipRect.w;
	    TDEOpt.stClipRect.u32Height = (mt_u32)pDstSurface->ClipRect.h;
	}
#endif
    s32Ret = ADP_GenerateTDEOpt(NULL, pDstSurface, pBlitOpt, &TDEOpt, MT_FALSE);

    TDEOpt.enPaint = MT_TRUE;
	TDEOpt.stPaintOpt.paint_type = TDE2_PAINT_TYPE_COLOR;
	TDEOpt.stPaintOpt.paint_color = pOpRect->Color;	
	s32Ret = MT_TDE2_Bitblit(s32Handle, NULL, NULL, NULL, NULL, &TDESurface, &Rect, &TDEOpt);

	s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	    MTGO_ERROR(s32Ret);
	    return MTGO_ERR_DEPEND_TDE;
	}
    }
#endif

    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_InitBlitter(mt_void)
{
    mt_s32 s32Ret;
    /** tde initial */
    s32Ret = MT_TDE2_Open();
    if (MT_SUCCESS != s32Ret) /** attention if reopen need to be deal*/
    {
	MTGO_ERROR(s32Ret);
	return MT_FAILURE;
    }
    s32Ret = MT_TDE2_SetAlphaThresholdState(MT_TRUE);
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }

    /** when the target color format is 1555, we needed to change the threshold value*/
    s32Ret = MT_TDE2_SetAlphaThresholdValue(0x80);
    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }

    g_MtgoInitTde = MT_TRUE;
    return MT_SUCCESS;
}

mt_void MTGO_ADP_DeinitBlitter(mt_void)
{
    MT_TDE2_Close();
    g_MtgoInitTde = MT_FALSE;
    return;
}

/**  a collection of operations about 2D module */
mt_s32 MTGO_ADP_GFXBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
#if 0
    mt_s32 s32Ret;

    /** check bility*/
    s32Ret = ADP_GFXBlitBility(pSrcSurface, pDstSurface, pBlitOpt);
    if (GFX_FALSE == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }

    if (GFX_PART == s32Ret) {
	/** to be modify , multiple operation*/
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
#endif
    /** YUV 2 RGB*/
    if (IS_SP_YUV_FORMAT(pSrcSurface->PixelFormat) || (pSrcSurface->PixelFormat == MTGO_PF_SP_CMYK)) {
#ifdef CONFIG_MT_FPGA_GPE        
    return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_FALSE, pBlitOpt);
#else        
	return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_FALSE);
#endif
//	return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pSrcRect, MT_FALSE);
    }

    /** RGB 2 RGB: attention no support clut blit*/
    return ADP_GFXBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, pBlitOpt, MT_FALSE);
}

mt_s32 MTGO_ADP_GFXStretchBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
#if 0
    mt_s32 s32Ret;
        /** check bility*/
    s32Ret = ADP_GFXStretchBlitBility(pSrcSurface, pDstSurface, pBlitOpt);
    if (GFX_FALSE == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }

    if (GFX_PART == s32Ret) {
	/** to be modify , multiple operation*/
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
#endif
    /** YUV 2 RGB*/
    if (IS_SP_YUV_FORMAT(pSrcSurface->PixelFormat) || (pSrcSurface->PixelFormat == MTGO_PF_SP_CMYK)) {
#ifdef CONFIG_MT_FPGA_GPE        
    return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_TRUE, pBlitOpt);
#else        
	return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_TRUE);
#endif
    }

    /** RGB 2 RGB: attention no support clut blit*/
    return ADP_GFXBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, pBlitOpt, MT_TRUE);
}

/**  a collection of operations about 2D module */
mt_s32 MTGO_ADP_GFXBlitEx(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pBckSurface, const MT_RECT *pBckRect,
	const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
#if 0
    mt_s32 s32Ret;
    /** check bility*/
    s32Ret = ADP_GFXBlitBility(pSrcSurface, pDstSurface, pBlitOpt);
    if (GFX_FALSE == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }

    if (GFX_PART == s32Ret) {
	/** to be modify , multiple operation*/
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
#endif
    /** YUV 2 RGB*/
    if (IS_SP_YUV_FORMAT(pSrcSurface->PixelFormat)) {
#ifdef CONFIG_MT_FPGA_GPE        
    return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_FALSE, pBlitOpt);
#else        
	return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_FALSE);
#endif
    }

    /** RGB 2 RGB: attention no support clut blit*/
    return ADP_GFXBlitEx(pSrcSurface, pSrcRect, pBckSurface, pBckRect, pDstSurface, pDstRect, pBlitOpt, MT_FALSE);
}

mt_s32 MTGO_ADP_GFXStretchBlitEx(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pBckSurface, const MT_RECT *pBckRect,
	const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
#if 0 
    mt_s32 s32Ret;
   
    /** check bility*/
    s32Ret = ADP_GFXStretchBlitBility(pSrcSurface, pDstSurface, pBlitOpt);
    if (GFX_FALSE == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }

    if (GFX_PART == s32Ret) {
	/** to be modify , multiple operation*/
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
#endif
    /** YUV 2 RGB*/
    if (IS_SP_YUV_FORMAT(pSrcSurface->PixelFormat)) {
#ifdef CONFIG_MT_FPGA_GPE        
            return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_TRUE, pBlitOpt);
#else        
            return ADP_CSCBlit(pSrcSurface, pSrcRect, pDstSurface, pDstRect, MT_TRUE);
#endif
    }

    /** RGB 2 RGB: attention no support clut blit*/
    return ADP_GFXBlitEx(pSrcSurface, pSrcRect, pBckSurface, pBckRect, pDstSurface, pDstRect, pBlitOpt, MT_TRUE);
}

mt_s32 MTGO_ADP_GFXOperate(const MTGO_SURFACE_S *pDstSurface, const GFX_OPT_TYPE_E GfxOpt, const void *pInfo, const MTGO_BLTOPT2_S *pBlitOpt)
{
#if 0
    mt_s32 s32Ret;

    s32Ret = ADP_GFXOperateBility(pDstSurface, GfxOpt, pBlitOpt);
    if (GFX_FALSE == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
    if (GFX_PART == s32Ret) {
	MTGO_ERROR(MTGO_ERR_UNSUPPORTED);
	return MTGO_ERR_UNSUPPORTED;
    }
#endif
    if (GfxOpt == GFX_OPT_FILLRECT) {
	return ADP_FillRect(pDstSurface, (const MTGO_OPRECT_S *)pInfo, pBlitOpt);
    } else {
	return MTGO_ERR_UNSUPPORTED;
    }
}

mt_s32 MTGO_ADP_GFXMaskBlit(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pMaskSurface, const MT_RECT *pMaskRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect, const MTGO_BLTOPT2_S *pBlitOpt)
{
    mt_s32 s32Ret = MT_SUCCESS;
    TDE_HANDLE s32Handle;
    TDE2_SURFACE_S TDESrcSurface, TDEMaskSurface, TDEDstSurface;

    TDE2_RECT_S SrcRect = { pSrcRect->x, pSrcRect->y, (mt_u32)pSrcRect->w, (mt_u32)pSrcRect->h };
    TDE2_RECT_S MaskRect = { pMaskRect->x, pMaskRect->y, (mt_u32)pMaskRect->w, (mt_u32)pMaskRect->h };
    TDE2_RECT_S DstRect = { pDstRect->x, pDstRect->y, (mt_u32)pDstRect->w, (mt_u32)pDstRect->h };
    TDE2_OPT_S stOpt;

    MTGO_MemSet(&stOpt, 0, sizeof(TDE2_OPT_S));
    /** prepare surface*/
    s32Ret = ADP_MEMSurfaceToTDESurface(pSrcSurface, &TDESrcSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    s32Ret = ADP_MEMSurfaceToTDESurface(pMaskSurface, &TDEMaskSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    /** prepare dst surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDEDstSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

#if 0
    printf("+++++ src  Addr 0x%x pf %d w %d h %d p %d \n", pSrcSurface->Data[0].pPhyData, pSrcSurface->PixelFormat,
           pSrcSurface->Width, pSrcSurface->Height, pSrcSurface->Data[0].Pitch);

    printf("Rect: [%d][%d][%d][%d] \n", pSrcRect->x, pSrcRect->y, pSrcRect->w, pSrcRect->h);

    printf("+++++ msk  Addr 0x%x pf %d w %d h %d p %d \n", pMaskSurface->Data[0].pPhyData, pMaskSurface->PixelFormat,
           pMaskSurface->Width, pMaskSurface->Height, pMaskSurface->Data[0].Pitch);

    printf("Rect: [%d][%d][%d][%d] \n", pMaskRect->x, pMaskRect->y, pMaskRect->w, pMaskRect->h);

    printf("+++++ dst  Addr 0x%x pf %d w %d h %d p %d \n", pDstSurface->Data[0].pPhyData, pDstSurface->PixelFormat,
           pDstSurface->Width, pDstSurface->Height, pDstSurface->Data[0].Pitch);
    printf("Rect: [%d][%d][%d][%d] \n", pDstRect->x, pDstRect->y, pDstRect->w, pDstRect->h);

#endif

    /** prepare alpha,key,rop opt */
    s32Ret = ADP_GenerateTDEOpt(pSrcSurface, pDstSurface, pBlitOpt, &stOpt, MT_FALSE);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

#ifndef MTGO_CODE_CUT
    if ((stOpt.enAluCmd == TDE2_ALUCMD_NONE) && (pBlitOpt->EnableGlobalAlpha)) {
	TDESrcSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDESrcSurface.enColorFmt);
	TDEDstSurface.enColorFmt = ADP_ConvertA2NOAFormat(TDEDstSurface.enColorFmt);

	/** global alpha enable pixel alpha disable*/
	stOpt.enOutAlphaFrom = TDE2_OUTALPHA_FROM_FOREGROUND;
	stOpt.enAluCmd = TDE2_ALUCMD_BLEND;
    }
#endif

    /** do tde job*/
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	MTGO_ERROR(MTGO_ERR_INTERNAL);
	return MTGO_ERR_INTERNAL;
    }

    if (pBlitOpt->EnableRop) {
	s32Ret = MT_TDE2_BitmapMaskRop(s32Handle, &TDEDstSurface, &DstRect, &TDESrcSurface, &SrcRect, &TDEMaskSurface, &MaskRect, &TDEDstSurface, &DstRect, pBlitOpt->Rop, pBlitOpt->RopAlpha);
    } else {
	s32Ret = MT_TDE2_BitmapMaskBlend(s32Handle, &TDEDstSurface, &DstRect, &TDESrcSurface, &SrcRect, &TDEMaskSurface, &MaskRect, &TDEDstSurface, &DstRect, 0xFF, TDE2_ALUCMD_BLEND);
    }

    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	(mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	return MTGO_ERR_DEPEND_TDE;
    }

    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }
    return MT_SUCCESS;
}


mt_s32 MTGO_ADP_RotateMirror(const MTGO_SURFACE_S *pSrcSurface, const MT_RECT *pSrcRect, const MTGO_SURFACE_S *pDstSurface, const MT_RECT *pDstRect,
                                                    const MTGO_ROTATE_E rotate_mod, const MTGO_MIRROR_E mirror_mod)
{
    mt_s32 s32Ret = MT_SUCCESS;
    TDE_HANDLE s32Handle;
    TDE2_SURFACE_S TDESrcSurface, TDEDstSurface;

    TDE2_RECT_S SrcRect = { pSrcRect->x, pSrcRect->y, (mt_u32)pSrcRect->w, (mt_u32)pSrcRect->h };
    TDE2_RECT_S DstRect = { pDstRect->x, pDstRect->y, (mt_u32)pDstRect->w, (mt_u32)pDstRect->h };
    TDE2_OPT_S stOpt;

    MTGO_MemSet(&stOpt, 0, sizeof(TDE2_OPT_S));
    /** prepare surface*/
    s32Ret = ADP_MEMSurfaceToTDESurface(pSrcSurface, &TDESrcSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

    /** prepare dst surface */
    s32Ret = ADP_MEMSurfaceToTDESurface(pDstSurface, &TDEDstSurface);
    if (MT_SUCCESS != s32Ret) {
	MTGO_ERROR(s32Ret);
	return s32Ret;
    }

	switch(mirror_mod)
	{
    case MTGO_MIRROR_NONE:
		stOpt.enMirror = TDE2_MIRROR_NONE;
		break;
	case MTGO_MIRROR_LR:
		stOpt.enMirror = TDE2_MIRROR_HORIZONTAL;
		break;		
	case MTGO_MIRROR_TB:
		stOpt.enMirror = TDE2_MIRROR_VERTICAL;
		break;			
	default:
		stOpt.enMirror = TDE2_MIRROR_NONE;
		break;		
	}

	switch(rotate_mod)
	{
    case MTGO_ROTATE_NONE:
		stOpt.enRotator = TDE2_ROTATE_NONE;
		break;
	case MTGO_ROTATE_90:
		stOpt.enRotator = TDE2_ROTATE_90;
		break;		
	case MTGO_ROTATE_180:
		stOpt.enRotator = TDE2_ROTATE_180;
		break;
	case MTGO_ROTATE_270:
		stOpt.enRotator = TDE2_ROTATE_270;
		break;
	default:
		stOpt.enRotator = TDE2_ROTATE_NONE;
		break;		
	}	

    /** do tde job*/
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle == MT_ERR_TDE_INVALID_HANDLE) {
	MTGO_ERROR(MTGO_ERR_INTERNAL);
	return MTGO_ERR_INTERNAL;
    }

    MT_TDE2_Bitblit(s32Handle, NULL, NULL, &TDESrcSurface, &SrcRect,
                                &TDEDstSurface, &DstRect, &stOpt);


    if (s32Ret != MT_SUCCESS) {
	MTGO_ERROR(s32Ret);
	(mt_void) MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
	return MTGO_ERR_DEPEND_TDE;
    }

    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, TDE_FLAG, TIMEOUT);
    if ((MT_SUCCESS != s32Ret) && (MT_ERR_TDE_JOB_TIMEOUT != s32Ret)) {
	MTGO_ERROR(s32Ret);
	return MTGO_ERR_DEPEND_TDE;
    }
    return MT_SUCCESS;
}

