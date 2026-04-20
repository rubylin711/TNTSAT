/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_SURFACE_H__
#define __MTGO_SURFACE_H__

/* add include here */
#include "mt_go_surface.h"
#include "mtgo_common.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/

#define MAX_SURFACE_NAME_LEN 16

#define IS_HAVE_CLIPRECT(pSurface) (pSurface->IsHaveClip == MT_TRUE)
#define MTGO_GetSurfaceRealRect(pSurface, pRect, pOptRect) \
do \
{ \
    MT_RECT SurfaceRect = {0}; \
    mt_s32 s32Ret; \
    (mt_void)Surface_GetSurfaceSize(pSurface, &(SurfaceRect.w), &(SurfaceRect.h)); \
    s32Ret = MTGO_GetRealRect(&SurfaceRect, pRect, pOptRect); \
    if (MT_SUCCESS != s32Ret) \
    {\
        MTGO_ERROR(s32Ret);\
        return s32Ret;\
    }\
}\
while (0)


/*************************** Structure Definition ****************************/

typedef enum
{
	MTGO_SUR_MEM_E,    /**< MTGO create  */
	MTGO_SUR_EMPTY_E,  /**<  user create */
	MTGO_SUR_BUTT
}MTGO_SUR_TYPE_E;


typedef struct
{
    MTGO_PF_E    PixelFormat;
    mt_s32       Width;
    mt_s32       Height;
    MT_BOOL      HasColorKey;
    MT_COLOR     ColorKey;  /**< color key */
    MT_COLOR     *Palette;   /**< color palette  */
    phys_addr_t pPhyPalette;  /**< physical color palette */
    MTGO_MOD_E   Model;     /**< model which Surface is belong to  */
    MT_PIXELDATA Data;
    mt_u8        Alpha;     /**< alpha channel */
    MT_BOOL      Locked;    /**< lock */
    MTGO_SUR_TYPE_E Type;   /**< surface type*/
    MT_BOOL      IsHaveClip;/**<*/
    MT_RECT      ClipRect;  /**< */
    mt_char      SurfaceName[MAX_SURFACE_NAME_LEN]; /**< surface name , no longer than MAX_SURFACE_NAME_LEN*/
} MTGO_SURFACE_S;

typedef enum
{
    MTGO_SYNC_CPU = MTGO_SYNC_MODE_CPU,
    MTGO_SYNC_TDE = MTGO_SYNC_MODE_TDE,
    MTGO_SYNC_DEC,
    MTGO_SYNC_JPEG,
    MTGO_SYNC_REFRESH,
    MTGO_SYNC_BUTT
}MTGO_SYNC_E;
/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
/*#ifndef TEST_IN_ROOTBOX
mt_s32 MTGO_InitSurface(mt_void *pBuffer , mt_u32 u32BufLen );
#else*/
mt_s32 MTGO_InitSurface(mt_void);
//#endif

mt_s32 MTGO_DeinitSurface(mt_void);
/** 
\brief create surface YUV and  CLUT format
\param[in] Surface
\param[in] pType
\retval none
\return none
*/
mt_s32 MTGO_CreateSurfaceFromMem(const MTGO_SURINFO_S *pSurInfo, MTGO_MOD_E Mode, MTGO_HANDLE *pSurface);

/** 
\brief create surface YUV and  CLUT format
\param[in] Surface  Width, Height, PixelFormat  must have £¬ if pitch value  is 0  than use the default value¡£
\param[in] pType
\retval none
\return none
*/
mt_s32 MTGO_CreateSurface(const MTGO_SURINFO_S *pSurInfo, MTGO_MOD_E Mode, MTGO_HANDLE *pSurface, mt_u32 MemModID);


/** 
\brief free surface
\param[in] Surface  Surface pointer
\retval none
\return none
*/
mt_void MTGO_FreeSurface(MTGO_HANDLE Surface);

/** 
\brief set surface memory type
\param[in] Surface
\param[in] Type
\retval none
\return none
*/
mt_s32 Surface_SetSurfaceType(MTGO_HANDLE Surface, MTGO_SUR_TYPE_E Type);

/**
 \brief create common surface structure 
 \param[out] pSurface create surface handle 
 \param[in] Width surface  width 
 \param[in] Height surface height 
 \param[in] PixelFormat 
 \retval MT_SUCCESS 
 \retval MTGO_ERR_NOMEM 
 \return mt_s32
 */
mt_s32 Surface_CreateSurface(MTGO_HANDLE* pSurface, mt_s32 Width, mt_s32 Height, MTGO_PF_E PixelFormat);

/**
 \brief free common Surface structure
 \param[in] Surface
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_void Surface_FreeSurface(MTGO_HANDLE Surface);

/**
 \brief freee Surface pravid data 
 \param[in] Surface
 \retval MT_SUCCESS free
 \return mt_s32
 */
mt_void Surface_FreeSurfacePrivateData(MTGO_HANDLE Surface);

/**
 \brief set Surface pravid data 
 \param[in] Surface Surface handle 
 \param[in] Module  
 \param[in] pData  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_SetSurfacePrivateData(MTGO_HANDLE Surface, MTGO_MOD_E Module, const MT_PIXELDATA pData);

/**
 \brief set Surface color palette 
 \param[in] Surface Surface handle 
 \param[in] Palette  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_SetSurfacePalette(MTGO_HANDLE Surface, const MT_PALETTE Palette);
//#ifdef TEST_IN_ROOTBOX
/**
 \brief  get Surface color palette 
 \param[in] Surface Surface handle 
 \param[out] Palette  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_GetSurfacePalette(MTGO_HANDLE Surface, const MT_PALETTE Palette);
//#endif

/**
 \brief set Surface color key 
 \param[in] Surface Surface handle 
 \param[in] ColorKey  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_SetSurfaceColorKey(MTGO_HANDLE Surface, MT_COLOR ColorKey);

/**
 \brief get Surface color key 
 \param[in] Surface Surface handle 
 \param[in] pColorKey  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_GetSurfaceColorKey(MTGO_HANDLE Surface, MT_COLOR* pColorKey);

/**
 \brief set surface alpha channel value 
 \param[in] Surface surface handle 
 \param[in] Alpha alpha value , range 0-255,  0 means full transparence  ,255 no transparence 
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_SetSurfaceAlpha(MTGO_HANDLE Surface, mt_u8 Alpha);

/**
 \brief  get surface alpha channel value 
 \param[in] Surface surface handle 
 \param[out] pAlpha  alhpa value pointer 
 \retval MT_SUCCESS  
 \return mt_void
 */
mt_void Surface_GetSurfaceAlpha(MTGO_HANDLE Surface, mt_u8* pAlpha);

/**
 \brief  lock surface 
 \param[in] Surface surface handle 
 \param[out] pData  
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_LockSurface(MTGO_HANDLE Surface, MT_PIXELDATA pData);

/**
 \brief unlock surface
 \param[in] Surface surface handle 
 \retval MT_SUCCESS  
 \return mt_s32
 */
mt_s32 Surface_UnlockSurface(MTGO_HANDLE Surface);

/**
 \brief  get surface size , pWidth and pHeight can't all null 
 \param[in] Surface surface handle 
 \param[out] pWidth surface ouput width address 
 \param[out] pHeight surface ouput height address 
 \retval MT_SUCCESS  
 \retval MTGO_ERR_NULLPTR pWidth and pHeight can't all null 
 \return mt_s32
 */
mt_s32 Surface_GetSurfaceSize(MTGO_HANDLE Surface, mt_s32* pWidth, mt_s32* pHeight);

/**
 \brief get surface pixel format
 \param[in] Surface surface handle 
 \param[out] pPixelFormat   output pixel format 
 \param[out] pBpp  (bytes per pixel)
 \retval MT_SUCCESS  
 \retval MTGO_ERR_NULLPTR pPixelFormat 
 \return mt_s32
 */
mt_s32 Surface_GetSurfacePixelFormat(MTGO_HANDLE Surface, MTGO_PF_E* pPixelFormat, mt_u32 *pBpp);

/**
 \brief  
 \param[in] PixelFormat  
 \param[out] pBpp  (bits per pixel)
 \retval MT_SUCCESS  
 \return mt_s32
 */

//mt_void Surface_CalculateBitsPerPixel(MTGO_PF_E PixelFormat, mt_u32* pBits);
mt_void Surface_CalculateBitsPerPixel0(MTGO_PF_E PixelFormat, mt_u32* pBits);

mt_void Surface_CalculateBpp0(MTGO_PF_E PixelFormat, mt_u32* pBpp);

mt_void Surface_CalculateBpp1(MTGO_PF_E PixelFormat, mt_u32* pBpp);

mt_void Surface_CalculateStride0(MTGO_PF_E PixelFormat, mt_u32 Width, mt_u32 Height, mt_u32* pWStride, mt_u32* pHStride);

mt_void Surface_CalculateStride1(MTGO_PF_E PixelFormat, mt_u32 Width, mt_u32 Height, mt_u32* pWStride, mt_u32* pHStride);

#ifdef CONFIG_MT_FPGA_GPE
mt_s32  Surface_compare(MTGO_HANDLE src1Surface, MTGO_HANDLE src2Surface);
mt_s32 Surface_SetVideoLayer(MTGO_HANDLE Surface);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __MTGO_SURFACE_H__ */


