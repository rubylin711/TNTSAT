/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "string.h"
#include "mt_type.h"

#include "mt_go_comm.h"
#include "mt_go_surface.h"
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mtgo_adp_sys.h"
#ifdef TEST_IN_ROOTBOX
#include "adp_vmem.h"
#else
#include "mtgo_memory.h"
#endif

/***************************** Macro Definition ******************************/
#define MAKERGB(r, g, b) (0xff000000 | ((r) << 16) | ((g) << 8) | (b))

#define SURFACE_ALIGN_0 0
//#define SURFACE_ALIGN_1  1
#define SURFACE_ALIGN_4 4
#define SURFACE_ALIGN_8 8
#define SURFACE_ALIGN_16 16
//#define SURFACE_ALIGN_64 64
#define SURFACE_ALIGN_128 128
#define SURFACE_ALIGN_32 32


/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#ifndef MTGO_CODE_CUT
static mt_void CreateDefaultPalette(MT_PALETTE Palette)
{

    mt_u32 i = 0;
    mt_u32 offset[] = {
	0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF
    };
    mt_u8 r, g, b;

    MTGO_ASSERT(Palette != MT_NULL);

    Palette[i++] = MAKERGB(0, 0, 0);
    Palette[i++] = MAKERGB(128, 0, 0);
    Palette[i++] = MAKERGB(0, 128, 0);
    Palette[i++] = MAKERGB(128, 128, 0);
    Palette[i++] = MAKERGB(0, 0, 128);
    Palette[i++] = MAKERGB(128, 0, 128);
    Palette[i++] = MAKERGB(0, 128, 128);
    Palette[i++] = MAKERGB(128, 128, 128);
    Palette[i++] = MAKERGB(192, 192, 192);
    Palette[i++] = MAKERGB(255, 0, 0);
    Palette[i++] = MAKERGB(0, 255, 0);
    Palette[i++] = MAKERGB(255, 255, 0);
    Palette[i++] = MAKERGB(0, 0, 255);
    Palette[i++] = MAKERGB(255, 0, 255);
    Palette[i++] = MAKERGB(0, 255, 255);
    Palette[i++] = MAKERGB(255, 255, 255);

    for (; i < 40; i++) {
	Palette[i] = MAKERGB(0, 0, 0);
    }

    for (r = 0; r < 6; r++) {
	for (g = 0; g < 6; g++) {
	    for (b = 0; b < 6; b++) {
		Palette[i++] = MAKERGB(offset[r], offset[g], offset[b]);
	    }
	}
    }

    return;
}
#endif

mt_void Surface_CalculateBpp0(MTGO_PF_E PixelFormat, mt_u32 *pBpp)
{
    switch (PixelFormat) {
    case MTGO_PF_CLUT1:
    case MTGO_PF_CLUT4:
	*pBpp = 0;
	break;
    case MTGO_PF_CLUT8:
    case MTGO_PF_A8:
    case MTGO_PF_233:
	*pBpp = 1;
	break;
#ifndef MTGO_CODE_CUT
    case MTGO_PF_0444:
    case MTGO_PF_4444:
    case MTGO_PF_0555:
#endif
    case MTGO_PF_ACLUT88:
    case MTGO_PF_1555:
    //case MTGO_PF_0555:
    case MTGO_PF_565:
	*pBpp = 2;
	break;
    case MTGO_PF_8565:
	*pBpp = 3;
	break;
    case MTGO_PF_UYVY:
	*pBpp = 2;
	break;
    case MTGO_PF_8888:
	case MTGO_PF_YUV8888:
	*pBpp = 4;
	break;
    case MTGO_PF_0888:
#ifndef RGB24
	*pBpp = 4;
#else
	*pBpp = 3;
#endif
	break;
    case MTGO_PF_YUV400:
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV422:
    case MTGO_PF_YUV422_V:
    case MTGO_PF_YUV444:
    case MTGO_PF_YUV420TILE:
	*pBpp = 2;
	break;
	case MTGO_PF_SP_CMYK:
	*pBpp = 2;
	break;
    case MTGO_PF_XYLC:
        *pBpp = 4;    
	break;
    default:
	*pBpp = 0;
	return;
    }

    return;
}

mt_void Surface_CalculateBpp1(MTGO_PF_E PixelFormat, mt_u32 *pBpp)
{
    switch (PixelFormat) {
    /* case MTGO_PF_CLUT1:
    case MTGO_PF_CLUT4:
    case MTGO_PF_CLUT8:
    case MTGO_PF_A8:
*/
    case MTGO_PF_ACLUT88:
    case MTGO_PF_0444:
    case MTGO_PF_4444:
    case MTGO_PF_1555:
    case MTGO_PF_0555:
    case MTGO_PF_565:
    case MTGO_PF_8565:
    case MTGO_PF_8888:
    case MTGO_PF_0888:
    case MTGO_PF_233:
    case MTGO_PF_XYLC:
	*pBpp = 0;
	break;
    case MTGO_PF_YUV400:
	*pBpp = 1;
	break;
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV420TILE:
	*pBpp = 1;
	break;
    case MTGO_PF_YUV422:
	*pBpp = 1;
	break;
    case MTGO_PF_YUV422_V:
	*pBpp = 2;
	break;
    case MTGO_PF_YUV444:
	*pBpp = 2;
	break;
	case MTGO_PF_SP_CMYK:
	*pBpp = 2;
	break;
    default:
	*pBpp = 0;
	return;
    }

    return;
}
#ifndef MTGO_CODE_CUT
#if 0
static mt_void Surface_CalculateBitsPerPixel1(MTGO_PF_E PixelFormat, mt_u32 *pBits)
{
    switch (PixelFormat) {
    /*   case MTGO_PF_CLUT1:
    case MTGO_PF_A1:
    case MTGO_PF_CLUT4:
    case MTGO_PF_CLUT8:
    case MTGO_PF_A8:
    
    case MTGO_PF_0444:
    case MTGO_PF_4444:*/
    case MTGO_PF_1555:
    case MTGO_PF_0555:
    case MTGO_PF_565:
    case MTGO_PF_8565:
    case MTGO_PF_8888:
    case MTGO_PF_0888:
    case MTGO_PF_233:
	*pBits = 0;
	break;
    case MTGO_PF_YUV400:
	*pBits = 0;
	break;
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV420TILE:
	*pBits = 4;
	break;
    case MTGO_PF_YUV422:
	*pBits = 8;
	break;
    case MTGO_PF_YUV422_V:
	*pBits = 8;
	break;
    case MTGO_PF_YUV444:
	*pBits = 16;
	break;
	case MTGO_PF_SP_CMYK:
	*pBits = 16;
	break;

    default:
	*pBits = 0;
	return;
    }

    return;
}
#endif
#endif
mt_void Surface_CalculateBitsPerPixel0(MTGO_PF_E PixelFormat, mt_u32 *pBits)
{
    switch (PixelFormat) {
    case MTGO_PF_CLUT1:
    case MTGO_PF_A1:
	*pBits = 1;
	break;
    case MTGO_PF_CLUT4:
	*pBits = 4;
	break;
    case MTGO_PF_CLUT8:
    case MTGO_PF_A8:
    case MTGO_PF_233:
	*pBits = 8;
	break;
    case MTGO_PF_ACLUT88:
    case MTGO_PF_0444:
    case MTGO_PF_4444:
    case MTGO_PF_1555:
    case MTGO_PF_0555:
    case MTGO_PF_565:
	*pBits = 16;
	break;
    case MTGO_PF_UYVY:
	*pBits = 16;
	break;
    case MTGO_PF_8888:
    case MTGO_PF_YUV8888:
	*pBits = 32;
	break;
    case MTGO_PF_0888:
#ifdef RGB24
	*pBits = 24;
#else
	*pBits = 32;
#endif
	break;
    case MTGO_PF_8565:
	*pBits = 24;
	break;
    case MTGO_PF_YUV400:
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV420TILE:
    case MTGO_PF_YUV422:
    case MTGO_PF_YUV422_V:
    case MTGO_PF_YUV444:
	*pBits = 8;
	break;
	case MTGO_PF_SP_CMYK:
	*pBits = 16;
	break;
    case MTGO_PF_XYLC:
        *pBits = 32;
        break;

    default:
	*pBits = 0;
	return;
    }

    return;
}

mt_void Surface_CalculateStride0(MTGO_PF_E PixelFormat, mt_u32 Width, mt_u32 Height, mt_u32 *pWStride, mt_u32 *pHStride)
{
    mt_u32 BitPerPixel;
    mt_u32 WAlign = 0, HAlign = 0;

    Surface_CalculateBitsPerPixel0(PixelFormat, &BitPerPixel);

    switch (PixelFormat) {
    case MTGO_PF_A8:
    case MTGO_PF_A1:
    case MTGO_PF_CLUT1:
    case MTGO_PF_CLUT4:
    case MTGO_PF_CLUT8:
    case MTGO_PF_ACLUT88:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_0;
	break;
    case MTGO_PF_0444:
    case MTGO_PF_4444:
    case MTGO_PF_1555:
    case MTGO_PF_0555:
    case MTGO_PF_565:
    case MTGO_PF_8565:
    case MTGO_PF_8888:
    case MTGO_PF_0888:
    case MTGO_PF_UYVY:
    case MTGO_PF_YUV8888:
    case MTGO_PF_233:
    case MTGO_PF_XYLC:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_0;
	break;
    case MTGO_PF_YUV400:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_8;
	break;
    case MTGO_PF_YUV420:
    case MTGO_PF_YUV420TILE:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_16;
	break;
    case MTGO_PF_YUV422:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_8;
	break;
    case MTGO_PF_YUV422_V:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_16;
	break;
    case MTGO_PF_YUV444:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_8;
	break;
    case MTGO_PF_SP_CMYK:
	WAlign = SURFACE_ALIGN_32;
	HAlign = SURFACE_ALIGN_8;
	break;	
    default:
	WAlign = SURFACE_ALIGN_0;
	HAlign = SURFACE_ALIGN_0;
	return;
    }

    //*pWStride = ((Width * BitPerPixel + ((WAlign - 1)<<3))>>3)&(~(WAlign - 1));
    *pWStride = (((Width * BitPerPixel + 7) >> 3) + (WAlign - 1)) & (~(WAlign - 1));

    if (HAlign != 0) {
	*pHStride = (Height + HAlign - 1) & (~(HAlign - 1));
    } else {
	*pHStride = Height;
    }
    return;
}

mt_void Surface_CalculateStride1(MTGO_PF_E PixelFormat, mt_u32 Width, mt_u32 Height, mt_u32 *pWStride, mt_u32 *pHStride)
{

    mt_u32 WStride, HStride;

    Surface_CalculateStride0(PixelFormat, Width, Height, &WStride, &HStride);

//printf("\r\n WStride:%d, PixelFormat:%d", WStride, PixelFormat);
    switch (PixelFormat) {
    case MTGO_PF_CLUT1:
    case MTGO_PF_A1:
    case MTGO_PF_CLUT4:
    case MTGO_PF_CLUT8:
    case MTGO_PF_ACLUT88:
    case MTGO_PF_A8:
    case MTGO_PF_0444:
    case MTGO_PF_4444:
    case MTGO_PF_1555:
    case MTGO_PF_0555:
    case MTGO_PF_565:
    case MTGO_PF_8565:
    case MTGO_PF_8888:
    case MTGO_PF_0888:
    case MTGO_PF_233:
    case MTGO_PF_XYLC:
	*pWStride = 0;
	*pHStride = 0;
	return;
    case MTGO_PF_YUV400:
	*pWStride = 0;
	*pHStride = 0;
	break;
    case MTGO_PF_YUV420:
	*pWStride = WStride;
	*pHStride = HStride >> 1;
	break;
    case MTGO_PF_YUV422:        
    case MTGO_PF_YUV420TILE:
	*pWStride = WStride;
	*pHStride = HStride;
	break;
    case MTGO_PF_YUV422_V:
	*pWStride = WStride << 1;
	*pHStride = HStride >> 1;
	break;
    case MTGO_PF_YUV444:
	*pWStride = WStride << 1;
	*pHStride = HStride;
	break;
    case MTGO_PF_SP_CMYK:
	*pWStride = WStride;
	*pHStride = HStride;
	break;	
    default:
	return;
    }

    return;
}

mt_s32 Surface_SetSurfaceType(MTGO_HANDLE Surface, MTGO_SUR_TYPE_E Type)
{
    MTGO_SURFACE_S *pSurfaceInstance = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != pSurfaceInstance);

    pSurfaceInstance->Type = Type;

    return MT_SUCCESS;
}

mt_s32 Surface_CreateSurface(MTGO_HANDLE *pSurface, mt_s32 Width, mt_s32 Height, MTGO_PF_E PixelFormat)
{
    MTGO_SURFACE_S *pSurfaceInstance;
    phys_addr_t pPhyAddr = 0;
    
    MTGO_ASSERT(MT_NULL != pSurface);
    //printf("Surface_CreateSurface: [%d][%d][%d]\n", Width, Height, PixelFormat);

    /** ·ÖÅäÊµÀýÄÚ´æ */
    pSurfaceInstance = (MTGO_SURFACE_S *)MTGO_Malloc(sizeof(MTGO_SURFACE_S));
    if (MT_NULL == pSurfaceInstance) {
	MTGO_ERROR(MTGO_ERR_NOMEM);
	return MTGO_ERR_NOMEM;
    }

    MTGO_MemSet(pSurfaceInstance, 0, sizeof(MTGO_SURFACE_S));

    pSurfaceInstance->Model = MTGO_MOD_BUTT;
    pSurfaceInstance->PixelFormat = PixelFormat;
    pSurfaceInstance->Alpha = 0xff;
    pSurfaceInstance->Width = Width;
    pSurfaceInstance->Height = Height;
    pSurfaceInstance->ClipRect.w = Width;
    pSurfaceInstance->ClipRect.h = Height;
#ifndef MTGO_CODE_CUT
    if ((PixelFormat == MTGO_PF_CLUT8)||
        (PixelFormat == MTGO_PF_CLUT4)||
        (PixelFormat == MTGO_PF_CLUT1) ||
       (PixelFormat == MTGO_PF_ACLUT88)  ) {
	/** the address must be phyaddress, for TDE to use(adp_gfx.c ADP_MEMSurfaceToTDESurface)*/

	pSurfaceInstance->Palette = (MT_COLOR *)MTGO_MMZ_Malloc(sizeof(MT_PALETTE), &pPhyAddr);
	pSurfaceInstance->pPhyPalette = pPhyAddr; //pSurfaceInstance->Palette;

	if (MT_NULL == pSurfaceInstance->Palette) {
	    MTGO_ERROR(MTGO_ERR_NOMEM);
	    MTGO_Free(pSurfaceInstance);
	    return MTGO_ERR_NOMEM;
	}
	//printf("length:%d\n", sizeof(MT_PALETTE));
	CreateDefaultPalette(pSurfaceInstance->Palette);
    }
#endif
    *pSurface = (MTGO_HANDLE)pSurfaceInstance;

    return MT_SUCCESS;
}

mt_s32 Surface_SetSurfacePrivateData(MTGO_HANDLE Surface, MTGO_MOD_E Model, const MT_PIXELDATA pData)
{
    MTGO_SURFACE_S *pSurfaceInstance = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != pSurfaceInstance);

    if (MT_TRUE == pSurfaceInstance->Locked) {
	//MTGO_SetError(MTGO_ERR_LOCKED);
	//return MTGO_ERR_LOCKED;
    }

    pSurfaceInstance->Model = Model;

    /* copy surface pixel data  */
    MTGO_MemCopy(pSurfaceInstance->Data, pData, sizeof(MT_PIXELDATA));
    return MT_SUCCESS;
}

mt_void Surface_FreeSurfacePrivateData(MTGO_HANDLE Surface)
{
    MTGO_ASSERT(MT_NULL != Surface);
    MTGO_SURFACE_S *pSurfaceInstance = (MTGO_SURFACE_S *)Surface;

    /** sub surface no needed free */
    if (pSurfaceInstance->Type == MTGO_SUR_MEM_E) {
	/** free Surface privad data  */
	MTGO_MMZ_Free(pSurfaceInstance->Data[0].pData);
    }

    return;
}

mt_void Surface_FreeSurface(MTGO_HANDLE Surface)
{
    MTGO_SURFACE_S *pSurfaceInstance;

    MTGO_ASSERT(MT_NULL != Surface);
    pSurfaceInstance = (MTGO_SURFACE_S *)Surface;

    if (MT_NULL != pSurfaceInstance) {
	if (/*(pSurfaceInstance->PixelFormat == MTGO_PF_CLUT8)||
		    (pSurfaceInstance->PixelFormat == MTGO_PF_CLUT4)||
		    (pSurfaceInstance->PixelFormat == MTGO_PF_CLUT1)*/ 0) {
	    MTGO_MMZ_Free(pSurfaceInstance->Palette);
	}
	MTGO_Free(pSurfaceInstance);
    }

    return;
}

mt_s32 Surface_SetSurfacePalette(MTGO_HANDLE Surface, const MT_PALETTE Palette)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);
    MTGO_ASSERT(MT_NULL != Palette);

    /* color paletten only be used in the case of 1/2/4/8bit  */
    if (!IS_CLUT_FORMAT(p->PixelFormat)) {
	MTGO_ERROR(MTGO_ERR_INVSURFACEPF);
	return MTGO_ERR_INVSURFACEPF;
    }

    /* copy collor palette to surface */
    MTGO_MemCopy(p->Palette, Palette, sizeof(MT_PALETTE));

    return MT_SUCCESS;
}

mt_s32 Surface_GetSurfacePalette(MTGO_HANDLE Surface, const MT_PALETTE Palette)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);
    MTGO_ASSERT(MT_NULL != Palette);

    MTGO_MemCopy(Palette, p->Palette, sizeof(MT_PALETTE));

    return MT_SUCCESS;
}

mt_s32 Surface_SetSurfaceColorKey(MTGO_HANDLE Surface, MT_COLOR ColorKey)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);

    /** remember color key */
    p->HasColorKey = MT_TRUE;
    p->ColorKey = ColorKey;
    return MT_SUCCESS;
}

mt_s32 Surface_GetSurfaceColorKey(MTGO_HANDLE Surface, MT_COLOR *pColorKey)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    /** check parameter */
    MTGO_ASSERT(MT_NULL != pColorKey);
    MTGO_ASSERT(MT_NULL != p);

    if (MT_TRUE != p->HasColorKey) {
	//MTGO_SetError(MTGO_ERR_NOCOLORKEY);
	MTGO_ERROR(MTGO_ERR_NOCOLORKEY);
	return MTGO_ERR_NOCOLORKEY;
    }

    *pColorKey = p->ColorKey;

    return MT_SUCCESS;
}

mt_s32 Surface_SetSurfaceAlpha(MTGO_HANDLE Surface, mt_u8 Alpha)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);

    /** record alpha channel value */
    p->Alpha = Alpha;
    return MT_SUCCESS;
}

mt_void Surface_GetSurfaceAlpha(MTGO_HANDLE Surface, mt_u8 *pAlpha)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);
    MTGO_ASSERT(MT_NULL != pAlpha);

    /** return alpha channel */
    *pAlpha = p->Alpha;
    return;
}

mt_s32 Surface_LockSurface(MTGO_HANDLE Surface, MT_PIXELDATA pData)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    /** check parameter */
    MTGO_ASSERT(MT_NULL != p);
    MTGO_ASSERT(MT_NULL != pData);

    /**  copy pixel data  */
    MTGO_MemCopy(pData, p->Data, sizeof(MT_PIXELDATA));

    /** record lock status */
    p->Locked = MT_TRUE;
    return MT_SUCCESS;
}

mt_s32 Surface_UnlockSurface(MTGO_HANDLE Surface)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT(MT_NULL != p);
    if (!p->Locked) {
	MTGO_ERROR(MTGO_ERR_NOTLOCKED);
	return MTGO_ERR_NOTLOCKED;
    }

    /** record lock status */
    p->Locked = MT_FALSE;
    return MT_SUCCESS;
}

mt_s32 Surface_GetSurfaceSize(MTGO_HANDLE Surface, mt_s32 *pWidth, mt_s32 *pHeight)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;

    MTGO_ASSERT((MT_NULL != pWidth) || (MT_NULL != pHeight));

    if (MT_NULL != pWidth) {
	*pWidth = p->Width;
    }

    if (MT_NULL != pHeight) {
	*pHeight = p->Height;
    }

    return MT_SUCCESS;
}
mt_s32 Surface_GetSurfacePixelFormat(MTGO_HANDLE Surface, MTGO_PF_E *pPixelFormat, mt_u32 *pBpp)
{
    MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;
    MTGO_ASSERT(MT_NULL != p);
    MTGO_ASSERT(MT_NULL != pPixelFormat);

    *pPixelFormat = p->PixelFormat;
    if (pBpp) {
	Surface_CalculateBpp0(p->PixelFormat, pBpp);
    }
    return MT_SUCCESS;
}

#ifdef CONFIG_MT_FPGA_GPE
#include "mt_common.h"
mt_s32  Surface_compare(MTGO_HANDLE src1Surface, MTGO_HANDLE src2Surface)
{
	MT_PIXELDATA src1_pData;
	MT_PIXELDATA src2_pData;
	mt_s32 i = 0, j = 0;
	mt_u8 *src1_ptr = NULL;
	mt_u8 *src2_ptr = NULL;
	//mt_u32          Bpp = 0;
	//mt_u32          pitch = 0;
	mt_u32          err_flag = 0;
	MTGO_SURFACE_S *src1 = (MTGO_SURFACE_S *)src1Surface;
	MTGO_SURFACE_S *src2 = (MTGO_SURFACE_S *)src2Surface;

	MTGO_ASSERT(MT_NULL != src1);
	MTGO_ASSERT(MT_NULL != src2);

	 MT_GO_LockSurface(src1Surface, src1_pData, MT_TRUE);
	 MT_GO_LockSurface(src2Surface, src2_pData, MT_TRUE);


    if((src1->Width != src2->Width) ||
         (src1->Height != src2->Height) ||
         (src1->PixelFormat != src2->PixelFormat) ||
         (src1_pData[0].Bpp != src2_pData[0].Bpp)/* ||
         (src1_pData[0].Pitch != src2_pData[0].Pitch)*/)
    {
    	printf("Err Surface_compare : Surface unsame W<%d %d> H<%d %d> Fmt<%d %d> Bpp<%d %d> Pitch<%d %d>\n", src1->Width , src2->Width, src1->Height, 
    		src2->Height, src1->PixelFormat, src2->PixelFormat, src1_pData[0].Bpp, src2_pData[0].Bpp,  
    		src1_pData[0].Pitch, src2_pData[0].Pitch);
    	return MT_FAILURE;
    }
    else
    {
        printf("Surface_compare info: Surface  W<%d %d> H<%d %d> Fmt<%d %d> Bpp<%d %d> Pitch<%d %d>\n", src1->Width , src2->Width, src1->Height, 
            src2->Height, src1->PixelFormat, src2->PixelFormat, src1_pData[0].Bpp, src2_pData[0].Bpp,  
            src1_pData[0].Pitch, src2_pData[0].Pitch);			
    }

    src1_ptr = (MT_U8 *)src1_pData[0].pData;
    src2_ptr = (MT_U8 *)src2_pData[0].pData;
  //  pitch = src1_pData[0].Pitch;
  //  Bpp =  src1_pData[0].Bpp;
    for(i = 0; i <  src1->Height; i++)
    {
        for(j =0; j <  src1->Width; j++)
        {
            if( (src1_ptr[i * src1_pData[0].Pitch + j] != src2_ptr[i * src2_pData[0].Pitch + j])) 
            {
                err_flag ++;
            }          
        }
    }
    if(err_flag > 0)
    {
        printf("\n<%s> : <%d> : check data wrong (%d)\n", __FUNCTION__, __LINE__, err_flag);
        return MT_FAILURE;
    }
    else
    {
        printf("<%s> : <%d> : check data success\n", __FUNCTION__, __LINE__);
        return MT_SUCCESS;
    }
}


mt_s32 Surface_SetVideoLayer(MTGO_HANDLE Surface)
{
    MT_PIXELDATA pData;
    //MTGO_SURFACE_S *srcSurface = (MTGO_SURFACE_S *)Surface;
    MTGO_ASSERT(MT_NULL != srcSurface);
    mt_u32 dtmp = 0;

    MT_GO_LockSurface(Surface, pData, MT_TRUE);

     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440204), (phys_addr_t)pData[0].pPhyData);  //luma top
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440248), (phys_addr_t)pData[1].pPhyData);  //chroma top

     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440224), (phys_addr_t)pData[0].pPhyData);  //luma top 2
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440268), (phys_addr_t)pData[1].pPhyData);  //chroma top 2

     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440210), (phys_addr_t)pData[0].pPhyData); //luma bot 2nd 0
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440254), (phys_addr_t)pData[1].pPhyData); //chroma  2nd 0

     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440230), (phys_addr_t)pData[0].pPhyData); //luma bot 2nd 2
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440274), (phys_addr_t)pData[1].pPhyData); //chroma bot 2nd 2


     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440208), (phys_addr_t)pData[0].pPhyData); //luma bot 
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf44024c), (phys_addr_t)pData[1].pPhyData);  // chroma bot
     
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440228), (phys_addr_t)pData[0].pPhyData); //luma bot 2
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf44026c), (phys_addr_t)pData[1].pPhyData); //chroma bot 2
    
     
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440214), (phys_addr_t)pData[0].pPhyData); //luma bot  2nd 0
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440258), (phys_addr_t)pData[1].pPhyData); //chroma bot  2nd 0
     
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440234), (phys_addr_t)pData[0].pPhyData); //luma bot  2nd 2
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf440278), (phys_addr_t)pData[1].pPhyData); //chroma bot 2nd 2

     mt_sys_write_register(SYMPHONY_IO_PA(0xbf4401e4), (phys_addr_t)pData[0].pPhyData); //motion cur 0
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf4401ec), (phys_addr_t)pData[0].pPhyData); //motion  cur 2
       
   
    //set linear input, and video stride
    mt_sys_read_register(SYMPHONY_IO_PA(0xbf440024), &dtmp);
    dtmp |=  (pData[0].Pitch << 16) |(1 << 8);
    mt_sys_write_register(SYMPHONY_IO_PA(0xbf440024), dtmp);

    //set endian or set endian in JPG decoder
     mt_sys_read_register(SYMPHONY_IO_PA(0xbf44001c), &dtmp);
     dtmp &= (~0x0f);
     dtmp |=  0x8;
     mt_sys_write_register(SYMPHONY_IO_PA(0xbf44001c), dtmp);

    return MT_SUCCESS;
}


#endif



