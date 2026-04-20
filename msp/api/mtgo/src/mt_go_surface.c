/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "string.h"

#include "mt_go_surface.h"
#include "mtgo_surface.h"


/***************************** Macro Definition ******************************/
MT_BOOL g_bEnAutoSync = MT_TRUE;

/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

mt_s32 MT_GO_SetSurfaceAlpha(mt_handle Surface, mt_u8 Alpha)
{	
  return Surface_SetSurfaceAlpha(Surface, Alpha);
}

mt_s32 MT_GO_GetSurfaceAlpha(mt_handle Surface, mt_u8* pAlpha)
{
	Surface_GetSurfaceAlpha(Surface, pAlpha);
	return MT_SUCCESS;
}

mt_s32 MT_GO_EnableSurfaceColorKey(mt_handle Surface, MT_BOOL Enable)
{
	  MTGO_SURFACE_S *p = (MTGO_SURFACE_S *)Surface;
    if(NULL == p)
      return MT_FAILURE;

    p->HasColorKey = Enable;
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetSurfaceColorKey(mt_handle Surface, MT_COLOR ColorKey)
{
    return Surface_SetSurfaceColorKey(Surface, ColorKey);
}

mt_s32 MT_GO_GetSurfaceColorKey(mt_handle Surface, MT_COLOR* pColorKey)
{
	return Surface_GetSurfaceColorKey(Surface, pColorKey);
}


mt_s32 MT_GO_SetSurfacePalette(mt_handle Surface, const MT_PALETTE Palette)
{
  return Surface_SetSurfacePalette(Surface, Palette);
}

mt_s32 MT_GO_GetSurfacePalette(mt_handle Surface, MT_PALETTE Palette)
{
	return Surface_GetSurfacePalette(Surface, Palette);
}


mt_s32 MT_GO_CreateSurface(mt_s32 Width, mt_s32 Height, MTGO_PF_E PixelFormat, mt_handle* pSurface)
{
    mt_s32 ret;
    MTGO_HANDLE Surface;
    MTGO_SURINFO_S SurInfo;
    mt_u32 u32PartNum = 0;
    /*config color key YUV and CLUT8 format memory */

    /** check parameters */
    if (MT_NULL == pSurface)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    /** width and height should >0 */
    if ((0 >= Width) || (0 >= Height))
    {
        MTGO_ERROR(MTGO_ERR_INVSURFACESIZE);
        return MTGO_ERR_INVSURFACESIZE;
    }
    if ((!IS_RGB_FORMAT(PixelFormat)) && (!IS_CLUT_FORMAT(PixelFormat)) && (!IS_ALPHA_FORMAT(PixelFormat)) && (!IS_YUV_FORMAT(PixelFormat)))
    {
        MTGO_ERROR(MTGO_ERR_INVSURFACEPF);
        return MTGO_ERR_INVSURFACEPF;
    }
    MTGO_MemSet(&SurInfo, 0, sizeof (MTGO_SURINFO_S));
    SurInfo.Width  = Width;
    SurInfo.Height = Height;
    SurInfo.PixelFormat = PixelFormat;

    ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_MEMSURFACE, &Surface, u32PartNum);
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    *pSurface = Surface; 
    return MT_SUCCESS;
}

mt_s32 MT_GO_FreeSurface(mt_handle Surface)
{
    MTGO_FreeSurface(Surface);
    return MT_SUCCESS;
}

mt_s32 MT_GO_CreateSurfaceFromMem(const MTGO_SURINFO_S *pSurInfo, mt_handle * pSurface)
{
    mt_s32 ret;
    MTGO_HANDLE Surface;
#ifndef MTGO_CODE_CUT
    /** check parameters */
    if ((MT_NULL == pSurface) || (MT_NULL == pSurInfo))
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    /** width and heighe should >0 */
    if ((0 >= pSurInfo->Width) || (0 >= pSurInfo->Height))
    {
        MTGO_ERROR(MTGO_ERR_INVSURFACESIZE);
        return MTGO_ERR_INVSURFACESIZE;
    }

    if ((!(IS_YUV_FORMAT(pSurInfo->PixelFormat))) && (!(IS_CLUT_FORMAT(pSurInfo->PixelFormat))) && (!(IS_RGB_FORMAT(pSurInfo->PixelFormat))))
    {
        MTGO_ERROR(MTGO_ERR_INVSURFACEPF);
        return MTGO_ERR_INVSURFACEPF;
    }

    if (!pSurInfo->Pitch[0] || !pSurInfo->pVirAddr[0] || !pSurInfo->pPhyAddr[0])
    {
        MTGO_ERROR(MTGO_ERR_INVPARAM);
        return MTGO_ERR_INVPARAM;
    }

//    if (IS_YUV_FORMAT(pSurInfo->PixelFormat))
    if (IS_SP_FORMAT(pSurInfo->PixelFormat))		
    {
        if (!pSurInfo->Pitch[1] || !pSurInfo->pVirAddr[1] || !pSurInfo->pPhyAddr[1])
        {
            MTGO_ERROR(MTGO_ERR_INVPARAM);
            return MTGO_ERR_INVPARAM;
        }
    }
#endif    
    ret = MTGO_CreateSurfaceFromMem(pSurInfo, MTGO_MOD_MEMSURFACE, &Surface);
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
    	return ret;
    }
    
    *pSurface = Surface;
    return MT_SUCCESS;
}

mt_s32 MT_GO_LockSurface(mt_handle Surface, MT_PIXELDATA pData,MT_BOOL bSync)
{
    return Surface_LockSurface(Surface, pData);
}

mt_s32 MT_GO_UnlockSurface(mt_handle Surface)
{

    return Surface_UnlockSurface (Surface);
}

mt_s32 MT_GO_GetSurfaceSize(mt_handle Surface, mt_s32* pWidth, mt_s32* pHeight)
{
    if ((MT_NULL == pWidth) && (MT_NULL == pHeight))
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    return Surface_GetSurfaceSize (Surface, pWidth, pHeight);
}

mt_s32 MT_GO_GetSurfacePixelFormat(mt_handle Surface, MTGO_PF_E* pPixelFormat)
{
    if (MT_NULL == pPixelFormat)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    return Surface_GetSurfacePixelFormat (Surface, pPixelFormat, MT_NULL);
}




