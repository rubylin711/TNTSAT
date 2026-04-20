/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mt_type.h"
#include "mtgo_common.h"
#include "mtgo_surface.h"
#include "mtgo_gif.h"
#include "mtgo_io.h"
#include "mtgo_adp_sys.h"
#include "mtgo_surface.h"
#include "mt_go_config.h"

#ifdef MTGO_GIF_SUPPORT
//#define GIF_MAIN_FUNC
//#define GIF_PRINT_DBGINFO

//#define GIF_MAX_IMAGE 128     /**< max image in  GIF  */
#define GIF_MAX_IMAGE 12800     /**< max image in  GIF  */
#define GIF_MAX_LWZ_BITS 12   /**< max bits of LWZ */

//#define GIF_INTERLACE 0x40
//#define GIF_LOCALCOLORMAP 0x80

/**memory allocate mode  */
typedef enum _GIF_MALLOC_MEMORY
{
    GIF_MALLOC_SYS = 0, /**<use the way malloc*/
    GIF_MALLOC_MTGO,    /**< use the way umap*/
    GIF_MALLOC_BUTT
} GIF_MALLOC_MEMORY;


/**image extend part control block */
typedef struct
{
    mt_u8  active;           /**< flag of extension control block exsit  */
    mt_u8  disposalMethod;   /**<ignore  */
    mt_u8  userInputFlag;    /**flag of wait for user input */
    mt_u8  trsFlag;          /**< flag of transparence  */
    mt_u16 delayTime;        /**< delay time unit (10ms) */
    mt_u8  trsColorIndex;    /** transparence color index */
    mt_u8  reserved;         /**<resever for align */
} GIF_GCTRLEXT;

/**single picture info */
typedef struct
{
    mt_u32       dataindex;     /**< offset position of picture data in the file */
    mt_u16       imageLPos;     /**< margin of left */
    mt_u16       imageTPos;     /**<margin of top */
    mt_u16       imageWidth;    /**< image width */
    mt_u16       imageHeight;   
    mt_u8        lFlag;         /**< flag: is local color palette */
    mt_u8        interlaceFlag; /**< flag: is interlace */
    mt_u8        sortFlag;      /**< flag is  local color palette by priority */
    mt_u8        reserved;      /**< resever for align */
    mt_u32       lSize;         /**< size of local color palette */
    mt_u8 *      pColorTable;   /**< pointer for local color palette */
    mt_u8 *      dataBuf;       /**< pointer of image data buffer  */
    mt_u32       format;        /**< image output format  */
    GIF_GCTRLEXT ctrlExt;       /**< extension control block */
    MT_BOOL      bMPalate;      /**< flag  is  color palette modified*/
} GIF_IMAGE_INFO;

/**GIFglobal info */
typedef struct
{
    mt_u16 scrWidth;         
    mt_u16 scrHeight;        
    mt_u8  gFlag;            
    mt_u8  colorRes;      
    mt_u8  gSort;            
    mt_u8  BKColorIdx;     
    mt_u8  pixelAspectRatio; 
    mt_u8  reserved[3];      
    mt_u32 gSize;           
    mt_u8 *gColorTable;     
} GIF_GLOBAL_INFO;


/**GIF structure about decoder instance */
typedef struct
{
    DEC_HANDLE      gifDecoder;                  /**decoder handle */
    IO_HANDLE       gifIo;                      /**< file handle*/
    mt_u32          gifVer;                     /**< GIF version */
    GIF_GLOBAL_INFO gifGlobalInfo;              /**< GIF global  */
    GIF_IMAGE_INFO  gifImageInfo[GIF_MAX_IMAGE]; /**< GIF frame info */
    mt_u32          gifImageCount;              /**< pictures counts contained in the GIF file*/
    /** variables used by LWZ */
    mt_s32  fresh;
    mt_s32  code_size;
    mt_s32  set_code_size;
    mt_s32  max_code;
    mt_s32  max_code_size;
    mt_s32  firstcode;
    mt_s32  oldcode;
    mt_s32  clear_code;
    mt_s32  end_code;
    mt_s32  table[2][(1 << GIF_MAX_LWZ_BITS)];
    mt_s32  stack[(1 << (GIF_MAX_LWZ_BITS)) * 2];
    mt_s32 *sp;
    mt_s32  ZeroDataBlock;
    mt_s32  curbit;
    mt_s32  lastbit;
    mt_s32  done;
    mt_s32  last_byte;
    mt_u8   buf[280];
} GIF_DECODER_INSTANCE;

/**GIF decode local functions */
static mt_s32	gif_CreateDecoder( DEC_HANDLE *pGifDec, const MTGO_DEC_ATTR_S *pSrcDesc );
static mt_s32	gif_DestroyDecoder(DEC_HANDLE GifDec);
static mt_s32	gif_ResetDecoder(DEC_HANDLE GifDec);
static mt_s32	gif_DecCommInfo(DEC_HANDLE GifDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo);
static mt_s32	gif_DecImgInfo(DEC_HANDLE GifDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo);
static mt_s32	gif_SetDecImgAttr(DEC_HANDLE GifDec, mt_u32 Index, const MTGO_DEC_IMGATTR_S *pImgAttr);
static mt_s32	gif_DecImgData(DEC_HANDLE GifDec, mt_u32 Index, MTGO_SURFACE_S *pSurface);
//static mt_s32	gif_ReleaseDecImgData(DEC_HANDLE GifDec, MTGO_DEC_IMGDATA_S *pImgData);
static mt_void	gif_CleanInstance( GIF_DECODER_INSTANCE *pGifInstance );
static mt_s32	gif_AnalyseFile( GIF_DECODER_INSTANCE *pGifInstance );
static mt_s32	gif_GetFileType( GIF_DECODER_INSTANCE *pGifInstance );
static mt_s32	gif_GetGlobalInfo( GIF_DECODER_INSTANCE *pGifInstance );
static mt_s32	gif_GetImageInfo( GIF_DECODER_INSTANCE *pGifInstance );
static mt_s32	gif_GetDataBlock( GIF_DECODER_INSTANCE *pGifInstance, mt_u8 *buf );
static mt_s32	gif_GetCode( GIF_DECODER_INSTANCE *pGifInstance, mt_s32 code_size, mt_s32 flag );
static mt_s32	gif_LZWReadByte( GIF_DECODER_INSTANCE *pGifInstance, mt_s32 flag, mt_s32 input_code_size );
static mt_s32	gif_ExtractData( GIF_DECODER_INSTANCE *pGifInstance, mt_u32 image_index );
static mt_s32	gif_ReadFile( IO_HANDLE hFileIo, mt_u8 *pu8Addr, mt_u32 u32Len );
static mt_s32	gif_SeekFile( IO_HANDLE hFileIo, mt_s32 pos );
static mt_s32	gif_SeekOffFile( IO_HANDLE hFileIo, mt_s32 off );
static mt_s32	gif_GetPos( IO_HANDLE hFileIo, mt_u32 *pos );

//static mt_s32 gif_GetLen( IO_HANDLE hFileIo, mt_u32 *len );
static mt_void* gif_Malloc( mt_u32 flag, mt_u32 size );
static mt_void	gif_Free( mt_u32 flag, mt_void *pdata );

//static mt_s32   gif_DecHandleAlloc(IO_HANDLE* pHandle, mt_void* pAddr, MTGO_MOD_E Modle);
static mt_void* gif_DecGetInstance(DEC_HANDLE Handle, MTGO_MOD_E Modle);


#define GIF_READ(hFileIo, pu8Addr, u32Len) \
    do \
    { \
        mt_s32 io_ret; \
        io_ret = gif_ReadFile(hFileIo, pu8Addr, u32Len); \
        if (MT_SUCCESS != io_ret)\
        { \
            return io_ret; \
        } \
    } while (0)

#define GIF_SEEK(hFileIo, pos) \
    do \
    { \
        mt_s32 io_ret; \
        io_ret = gif_SeekFile(hFileIo, pos); \
        if (MT_SUCCESS != io_ret)\
        { \
            return io_ret; \
        } \
    } while (0)

#define GIF_SEEKOFF(hFileIo, off) \
    do \
    { \
        mt_s32 io_ret; \
        io_ret = gif_SeekOffFile(hFileIo, off); \
        if (MT_SUCCESS != io_ret)\
        { \
            return io_ret; \
        } \
    } while (0)

/*jummping line use in interlace */
static mt_s32 dpass[] = {
    8, 8, 4, 2
};
static mt_s32 restart[] = {
    0, 4, 2, 1, 32767
};

/**********************interface for decoder common part (B)**********************/

/**
 \brief create GIF decoder 
 \param[out] *pGifDec return decoder handle 
 \param[in] *pSrcDesc decoder attribute
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
mt_s32 GIF_CreateDecoder(DEC_HANDLE *pGifDec, const MTGO_DEC_ATTR_S *pSrcDesc)
{
    return (gif_CreateDecoder(pGifDec, pSrcDesc));
}

/**
 \brief  destroy GIF decoder 
 \param[in] GifDec decoder handle
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
mt_s32 GIF_DestroyDecoder(DEC_HANDLE GifDec)
{
    return (gif_DestroyDecoder(GifDec));
}

/**
 \brief reset decoder 
 \param[in] GifDec decoder handle
 \retval MT_SUCCESS 
 \retval MT_FAILURE
 \return mt_s32
 */
mt_s32 GIF_ResetDecoder(DEC_HANDLE GifDec)
{
    return (gif_ResetDecoder(GifDec));
}

/**
 \brief   get GIF global info
 \param[in] GifDec decode handle
 \param[out] *pPrimaryInfo pointer to  global info
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
mt_s32 GIF_DecCommInfo(DEC_HANDLE GifDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    return (gif_DecCommInfo(GifDec, pPrimaryInfo));
}

/** return actual resoltion */
mt_s32 GIF_GetActualSize(DEC_HANDLE GifDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo)
{
    MTGO_DEC_IMGINFO_S ImgInfo;
    mt_s32 ret;

    ret = GIF_DecImgInfo(GifDec, (mt_u32)Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    pSurInfo->Width = (mt_s32)ImgInfo.Width;
    pSurInfo->Height = (mt_s32)ImgInfo.Height;
    pSurInfo->PixelFormat = ImgInfo.Format;
    pSurInfo->Pitch[0] = ((ImgInfo.Width + 63) / 64) * 64 * 4;
    return MT_SUCCESS;
}

/**
 \brief get the GIF image info
 \param[in] GifDec decoder handle
 \param[in] Index image index
 \param[out] *pImgInfo pointer for single image info
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
mt_s32 GIF_DecImgInfo(DEC_HANDLE GifDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    return (gif_DecImgInfo(GifDec, Index, pImgInfo));
}

/**
 \brief set GIF attribute
 \param[in] GifDec decoder handle
 \param[in] Index image index
 \param[in] *pDecInputInfo attribute of single image
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
mt_s32 GIF_SetDecImgAttr(DEC_HANDLE GifDec, mt_u32 Index, const MTGO_DEC_IMGATTR_S *pImgAttr)
{
    return (gif_SetDecImgAttr(GifDec, Index, pImgAttr));
}


mt_s32 GIF_DecImgData(DEC_HANDLE GifDec, mt_u32 Index, MTGO_SURFACE_S *pSurface)
{
    return (gif_DecImgData(GifDec, Index, pSurface));
}
#if 0
mt_s32 GIF_ReleaseDecImgData(DEC_HANDLE GifDec, MTGO_DEC_IMGDATA_S *pImgData)
{
    return (gif_ReleaseDecImgData(GifDec, pImgData));
}
#endif

#ifdef TEST_IN_ROOTBOX
mt_s32 GIF_DecExtendData(DEC_HANDLE GifDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void **pData, mt_u32 *pLength)
{
    return MT_FAILURE;
} /*lint !e818 */

mt_s32 GIF_ReleaseDecExtendData(DEC_HANDLE GifDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void *pData)
{
    return MT_FAILURE;
} /*lint !e818 */
#endif
/**********************interface function for decoder common part**********************/

/**
 \brief create GIF decoder
 \param[out] *pGifDec return GIF decoder handle
 \param[in] *pSrcDesc decoder attribute
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_CreateDecoder( DEC_HANDLE *pGifDec, const MTGO_DEC_ATTR_S *pSrcDesc )
{
    mt_s32 ret;
    IO_DESC_S gifIoInfo;
    DEC_HANDLE gifIo;    
    GIF_DECODER_INSTANCE *pGifInstance;

#ifdef TEST_IN_ROOTBOX
    gifIoInfo.Type = (IO_TYPE_E)pSrcDesc->SrcType;
#endif
    gifIoInfo.IoInfo.MemInfo.pAddr  = pSrcDesc->SrcInfo.MemInfo.pAddr;
    gifIoInfo.IoInfo.MemInfo.Length = pSrcDesc->SrcInfo.MemInfo.Length;
#ifdef TEST_IN_ROOTBOX    
    gifIoInfo.IoInfo.pFileName = (const mt_char *)pSrcDesc->SrcInfo.pFileName;
#endif    
    ret = MTGO_ADP_IOCreate(&gifIo, &gifIoInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    /**allocate memory for decoder instance */
    pGifInstance = (GIF_DECODER_INSTANCE *)gif_Malloc(GIF_MALLOC_SYS, sizeof(GIF_DECODER_INSTANCE));
    if (NULL == pGifInstance)
    {
        if (MT_SUCCESS != MTGO_ADP_IODestroy(gifIo))
        {
#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("MTGO_ADP_IODestroy error!\n");
#endif
        }
        MTGO_ERROR(MTGO_ERR_NOMEM);
        return MTGO_ERR_NOMEM;
    }


    *pGifDec = (DEC_HANDLE)pGifInstance;
#if 0
    ret = gif_DecHandleAlloc( pGifDec, pGifInstance, MTGO_MOD_COMM );
    if (MT_SUCCESS != ret)
    {
        gif_Free(GIF_MALLOC_SYS, pGifInstance);
        if (MT_SUCCESS != MTGO_ADP_IODestroy(gifIo))
        {
 #ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("MTGO_ADP_IODestroy error!\n");
 #endif
        }

        return MT_FAILURE;
    }
#endif


    MTGO_MemSet(pGifInstance, 0, sizeof(GIF_DECODER_INSTANCE));
    pGifInstance->gifDecoder = *pGifDec;
    pGifInstance->gifIo = gifIo;


    ret = gif_AnalyseFile( pGifInstance );
    if (MT_SUCCESS != ret)
    {
        gif_CleanInstance( pGifInstance );
        gif_Free(GIF_MALLOC_SYS, pGifInstance);
        (mt_void)MTGO_ADP_IODestroy(gifIo);
        MTGO_ERROR(ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
} /*lint !e818 */

/**
 \brief destroy GIF decoder
 \param[in] GifDec decoder handle
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_DestroyDecoder(DEC_HANDLE GifDec)
{
    GIF_DECODER_INSTANCE *pGifInstance;
    
    pGifInstance = (GIF_DECODER_INSTANCE*)
        gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    if (MT_SUCCESS != MTGO_ADP_IODestroy(pGifInstance->gifIo))
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("MTGO_ADP_IODestroy error!\n");
#endif
    }
    gif_CleanInstance(pGifInstance);
    //Handle_Free(GifDec);
    gif_Free(GIF_MALLOC_SYS, pGifInstance);
    return MT_SUCCESS;
}

/**
 \brief reset GIF decoder
 \param[in] GifDec decoder handle
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_ResetDecoder(DEC_HANDLE GifDec)
{
    GIF_DECODER_INSTANCE *pGifInstance;

    pGifInstance = (GIF_DECODER_INSTANCE*)gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pGifInstance->fresh = 0;
    pGifInstance->set_code_size = 0;
    pGifInstance->code_size = 0;
    pGifInstance->max_code_size = 0;
    pGifInstance->max_code = 0;    
    pGifInstance->firstcode = 0;
    pGifInstance->oldcode = 0;
    pGifInstance->clear_code = 0;
    pGifInstance->end_code = 0;
    MTGO_MemSet(&pGifInstance->table[0][0], 0, 2 * (1 << GIF_MAX_LWZ_BITS) * 4);
    MTGO_MemSet(pGifInstance->stack, 0, 2 * (1 << GIF_MAX_LWZ_BITS) * 4);
    pGifInstance->sp = NULL;
    pGifInstance->ZeroDataBlock = 0;

    return MT_SUCCESS;
}

/**
 \brief get GIF global info
 \param[in] GifDec 
 \param[out] *pPrimaryInfo 
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_DecCommInfo(DEC_HANDLE GifDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    GIF_DECODER_INSTANCE *pGifInstance;

    pGifInstance = (GIF_DECODER_INSTANCE*)gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pPrimaryInfo->Count = pGifInstance->gifImageCount;
    pPrimaryInfo->ImgType = MTGO_DEC_IMGTYPE_GIF;    
    pPrimaryInfo->ScrWidth  = pGifInstance->gifGlobalInfo.scrWidth;
    pPrimaryInfo->ScrHeight = pGifInstance->gifGlobalInfo.scrHeight;

    /** calculate global color */
    if ((pGifInstance->gifGlobalInfo.gFlag > 0) && (pGifInstance->gifGlobalInfo.gColorTable != NULL))
    {
        mt_u32 Index;

        pPrimaryInfo->IsHaveBGColor = MT_TRUE;
        Index = pGifInstance->gifGlobalInfo.BKColorIdx;
        pPrimaryInfo->BGColor = 0;

        /** red*/
        pPrimaryInfo->BGColor |= (mt_u32)(pGifInstance->gifGlobalInfo.gColorTable[Index * 3 + 0] << 16);

        /** green*/
        pPrimaryInfo->BGColor |= (mt_u32)(pGifInstance->gifGlobalInfo.gColorTable[Index * 3 + 1] << 8);

        /** blue*/
        pPrimaryInfo->BGColor |= (mt_u32)(pGifInstance->gifGlobalInfo.gColorTable[Index * 3 + 2]);

        /** alpha*/
        pPrimaryInfo->BGColor |= 0xff000000;
    }
    else
    {
        pPrimaryInfo->IsHaveBGColor = MT_FALSE;
    }

    return MT_SUCCESS;
}

static MT_BOOL gif_IsKeyFlickPalate(MT_COLOR Color, mt_u32 KeyIndex, mt_u8 *pPalate, mt_u32 u32Num)
{
    mt_s32 Index;
    mt_u8 r, g, b;

    r = (Color & 0X00FF0000)>>16;
    g = (Color & 0X0000FF00)>>8;
    b = (Color & 0X000000FF); 
    
    for (Index = 0; Index < (mt_s32)u32Num; Index++)
    {
        /** take no attention to CKEY*/
        if ((mt_s32)KeyIndex == Index)
            continue;
        if ((r == pPalate[Index*3 + 2])&&(g == pPalate[Index*3 + 1])&&(b == pPalate[Index*3]))
            return MT_TRUE;
    }
        
    return MT_FALSE;
}

static mt_s32 gif_DealConfickPalate(mt_u32 KeyIndex, mt_u8 *pPalate, mt_u32 u32Num)
{
    MT_COLOR KeyColor, RanColor = 0x00787878;
    
    KeyColor = (mt_u32)((pPalate[KeyIndex * 3]) | ((pPalate[KeyIndex * 3 + 1]) << 8) | ((pPalate[KeyIndex * 3 + 2])<<16));

    /** check is COLORKEY conflict*/
    if (MT_FALSE == gif_IsKeyFlickPalate (KeyColor, KeyIndex, pPalate, u32Num) )
    {
        /** no conflict*/
        return MT_SUCCESS;
    }
    
    for (;;)
    {
    /** check is COLORKEY conflict*/
        if (MT_TRUE == gif_IsKeyFlickPalate (RanColor, KeyIndex, pPalate, u32Num) )
        {
            /** get a rand color */
            RanColor += 0x10;
            continue;
        }
            
        /** change color palette*/        
        pPalate[KeyIndex*3]  = (mt_u8)(RanColor & 0x000000ff);
        pPalate[KeyIndex*3 + 1]  = (mt_u8)((RanColor & 0x0000ff00) >> 8);
        pPalate[KeyIndex*3 + 2]  = (mt_u8)((RanColor & 0x00ff0000) >> 16);        
        break;

    }
    return MT_SUCCESS;
}

/**
 \brief  get the image info which needed index
 \param[in] GifDec decoder handle
 \param[in] Index image index
 \param[out] *pImgInfo 
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_DecImgInfo(DEC_HANDLE GifDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    GIF_DECODER_INSTANCE *pGifInstance;
    mt_u32 keyindex;

    pGifInstance = (GIF_DECODER_INSTANCE*)gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    if (Index >= pGifInstance->gifImageCount)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pImgInfo->OffSetX = pGifInstance->gifImageInfo[Index].imageLPos;
    pImgInfo->OffSetY = pGifInstance->gifImageInfo[Index].imageTPos;
    pImgInfo->Height = pGifInstance->gifImageInfo[Index].imageHeight;
    pImgInfo->Width  = pGifInstance->gifImageInfo[Index].imageWidth;
    pImgInfo->Alpha = 255;


    pImgInfo->IsHaveKey = MT_FALSE;
    pImgInfo->Key = 0;
    if (pGifInstance->gifImageInfo[Index].ctrlExt.active > 0)
    {
        if (pGifInstance->gifImageInfo[Index].ctrlExt.trsFlag > 0)
        {
            keyindex = (mt_u32)pGifInstance->gifImageInfo[Index].ctrlExt.trsColorIndex;
            if (pGifInstance->gifImageInfo[Index].lFlag > 0)
            {
                if (pGifInstance->gifImageInfo[Index].pColorTable == MT_NULL_PTR)
                    return MT_FAILURE;
                if (pGifInstance->gifImageInfo[Index].bMPalate == MT_FALSE)
                {
                    pGifInstance->gifImageInfo[Index].bMPalate = MT_TRUE;
                    (mt_void)gif_DealConfickPalate(keyindex, pGifInstance->gifImageInfo[Index].pColorTable, pGifInstance->gifImageInfo[Index].lSize);
                }
                /*red*/
                pImgInfo->Key |= (mt_u32)(pGifInstance->gifImageInfo[Index].pColorTable[keyindex * 3 + 0] << 16);

                /*green*/
                pImgInfo->Key |= (mt_u32)(pGifInstance->gifImageInfo[Index].pColorTable[keyindex * 3 + 1] << 8);

                /*blue*/
                pImgInfo->Key |= pGifInstance->gifImageInfo[Index].pColorTable[keyindex * 3 + 2];
            }
            else
            {
                if (pGifInstance->gifGlobalInfo.gColorTable == MT_NULL_PTR)
                    return MT_FAILURE;
                if (pGifInstance->gifImageInfo[Index].bMPalate == MT_FALSE)
                {
                    pGifInstance->gifImageInfo[Index].bMPalate = MT_TRUE;
                    (mt_void)gif_DealConfickPalate(keyindex, pGifInstance->gifGlobalInfo.gColorTable, pGifInstance->gifGlobalInfo.gSize);                    
                }


                /*red*/
                pImgInfo->Key |= (mt_u32)(pGifInstance->gifGlobalInfo.gColorTable[keyindex * 3 + 0] << 16);

                /*green*/
                pImgInfo->Key |= (mt_u32)(pGifInstance->gifGlobalInfo.gColorTable[keyindex * 3 + 1] << 8);

                /*blue*/
                pImgInfo->Key |= pGifInstance->gifGlobalInfo.gColorTable[keyindex * 3 + 2];


            }

            pImgInfo->Key |= 0xff000000;
            pImgInfo->IsHaveKey = MT_TRUE;
        }
    }

    pImgInfo->Format = (MTGO_PF_E)pGifInstance->gifImageInfo[Index].format;
    if (pGifInstance->gifImageInfo[Index].ctrlExt.active > 0)
    {
        pImgInfo->DelayTime = (mt_u32)pGifInstance->gifImageInfo[Index].ctrlExt.delayTime;
        pImgInfo->DisposalMethod = (mt_u32)pGifInstance->gifImageInfo[Index].ctrlExt.disposalMethod;        
    }
    else
    {
        pImgInfo->DelayTime = 0;
        pImgInfo->DisposalMethod = 0;        
    }
    return MT_SUCCESS;
}


/**
 \brief set GIF attribute
 \param[in] GifDec decoder handle
 \param[in] Index image index
 \param[in] *pDecInputInfo pointer for single image info
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_SetDecImgAttr(DEC_HANDLE GifDec, mt_u32 Index, const MTGO_DEC_IMGATTR_S *pImgAttr)
{
    GIF_DECODER_INSTANCE *pGifInstance;

    pGifInstance = (GIF_DECODER_INSTANCE*)gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    if (Index >= pGifInstance->gifImageCount)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    /**now our solution can only support  RGB8888 and RGB888 */
    pGifInstance->gifImageInfo[Index].format = pImgAttr->Format;
    if (pGifInstance->gifImageInfo[Index].format != MTGO_PF_0888)
    {
        pGifInstance->gifImageInfo[Index].format = MTGO_PF_8888;
    }

    return MT_SUCCESS;
}



/**
 \brief deocde the image 
 \param[in] GifDec 
 \param[in] Index 
 \param[out] *ImgInfo 
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_DecImgData(DEC_HANDLE GifDec, mt_u32 Index, MTGO_SURFACE_S *pSurface)
{
    mt_s32 ret;
    mt_u32 i, j, pitch, width, height, pixbytes, temp;
    GIF_DECODER_INSTANCE *pGifInstance;
    mt_u8 *pOutVirAddr, *pImgBuf, *pCol;
#ifdef GIF_PRINT_DBGINFO    
    mt_u8 *pOutPhyAddr;
#endif    
    MT_PIXELDATA pData = {0};
    pGifInstance = (GIF_DECODER_INSTANCE*)gif_DecGetInstance(GifDec, MTGO_MOD_COMM);
    if (NULL == pGifInstance)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    if (Index >= pGifInstance->gifImageCount)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }


    if (NULL == pGifInstance->gifImageInfo[Index].dataBuf)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("gif_ExtractData\n");
#endif
        if(Index >= 1)
        {
            if( pGifInstance->gifImageInfo[Index - 1].dataBuf != NULL)
            {
 #ifdef TEST_IN_ROOTBOX        
            gif_Free(GIF_MALLOC_SYS, pGifInstance->gifImageInfo[Index - 1].dataBuf);
#else
            gif_Free(GIF_MALLOC_MTGO, pGifInstance->gifImageInfo[Index - 1].dataBuf);
#endif
            pGifInstance->gifImageInfo[Index - 1].dataBuf  = NULL;
            }
        }
        ret = gif_ExtractData(pGifInstance, Index);
        if (MT_SUCCESS != ret)
        {
            MTGO_ERROR(MT_FAILURE);
            return MT_FAILURE;
        }
    }

    height = pGifInstance->gifImageInfo[Index].imageHeight;
    width = pGifInstance->gifImageInfo[Index].imageWidth;
    //(MTGO_PF_E)pGifInstance->gifImageInfo[Index].format;
    if (pGifInstance->gifImageInfo[Index].format == MTGO_PF_0888)
    {
        pixbytes = 3;
    }
    else
    {
        pixbytes = 4;
    }



#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("width = %u, height = %u, pixbyte = %u\n", width, height, pixbytes);
//    MTGO_PrintF("gif_Malloc, size = %u\n", height * pitch);
#endif

     (mt_void)Surface_LockSurface((MTGO_HANDLE)pSurface, pData);

    /**get the physical address of display buffer */
    pOutVirAddr = (mt_u8*)pData[0].pData;
#ifdef GIF_PRINT_DBGINFO    
    pOutPhyAddr = (mt_u8*)pData[0].pPhyData;
#endif
    pitch = pData[0].Pitch;

#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("pOutVirAddr= %x\n, PHYADDR:%x\n", pOutVirAddr, pOutPhyAddr);
#endif

    if (pGifInstance->gifImageInfo[Index].lFlag)
    {
        pCol = pGifInstance->gifImageInfo[Index].pColorTable;
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("Local palette, colsize = %u\n", 
                    pGifInstance->gifImageInfo[Index].lSize);
#endif
    }
    else
    {
        pCol = pGifInstance->gifGlobalInfo.gColorTable;
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("Global palette, colsize = %u\n", 
                    pGifInstance->gifGlobalInfo.gSize);
#endif
    }
    if (pCol == NULL)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    /**transfer the decoded data to format RGB and copy them to display buffer */
    pImgBuf = pGifInstance->gifImageInfo[Index].dataBuf;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            temp = (mt_u32)(pImgBuf[i * width + j] * 3);

            /**change order as the RGB palette use inf frame buffer is reiverse*/
            pOutVirAddr[i * pitch + j * pixbytes + 0] = pCol[temp + 2];
            pOutVirAddr[i * pitch + j * pixbytes + 1] = pCol[temp + 1];
            pOutVirAddr[i * pitch + j * pixbytes + 2] = pCol[temp + 0];
            if (pixbytes == 4)
            {
                pOutVirAddr[i * pitch + j * pixbytes + 3] = 0xff;
            }
        }
    }

    return MT_SUCCESS;
} 

#if 0
/**
 \brief free image data
 \param[in] GifDec decoder handle
 \param[in] *ImgInfo 
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_ReleaseDecImgData(DEC_HANDLE GifDec, MTGO_DEC_IMGDATA_S *pImgData)
{
    mt_void *pOutVirAddr;

    if ((NULL == pImgData) || (pImgData->VirAddr[0] == 0))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pOutVirAddr = (mt_void*)pImgData->VirAddr[0];
    gif_Free(GIF_MALLOC_MTGO, pOutVirAddr);
    MTGO_MemSet(pImgData, 0, sizeof(MTGO_DEC_IMGDATA_S));

    return MT_SUCCESS;
}
#endif

static mt_void gif_CleanInstance( GIF_DECODER_INSTANCE *pGifInstance )
{
    mt_u32 i;

    if (NULL != pGifInstance->gifGlobalInfo.gColorTable)
    {
        gif_Free(GIF_MALLOC_SYS, pGifInstance->gifGlobalInfo.gColorTable);
        pGifInstance->gifGlobalInfo.gColorTable = NULL;
    }

    for (i = 0; i < pGifInstance->gifImageCount; i++)
    {
        if (NULL != pGifInstance->gifImageInfo[i].pColorTable)
        {
            gif_Free(GIF_MALLOC_SYS, pGifInstance->gifImageInfo[i].pColorTable);
            pGifInstance->gifImageInfo[i].pColorTable = NULL;
        }

        if (NULL != pGifInstance->gifImageInfo[i].dataBuf)
        {
#ifdef TEST_IN_ROOTBOX        
            gif_Free(GIF_MALLOC_SYS, pGifInstance->gifImageInfo[i].dataBuf);
#else
            gif_Free(GIF_MALLOC_MTGO, pGifInstance->gifImageInfo[i].dataBuf);
#endif
            pGifInstance->gifImageInfo[i].dataBuf = NULL;
        }
    }

    return;
}

/**
 \brief parser GIF file
 \param[in] *pGifInstance GIF decoder instance
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_AnalyseFile( GIF_DECODER_INSTANCE *pGifInstance )
{
    mt_s32 ret;

    /**get GIF version */
#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("gif_GetFileType\n");
#endif
    ret = gif_GetFileType(pGifInstance);
    if (ret != MT_SUCCESS)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("gif_GetFileType error!\n");
#endif
        MTGO_ERROR(ret);
        return MT_FAILURE;
    }

    /**get GIF global info */
#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("gif_GetGlobalInfo\n");
#endif
    ret = gif_GetGlobalInfo(pGifInstance);
    if (ret != MT_SUCCESS)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("gif_GetGlobalInfo error!\n");
#endif
        MTGO_ERROR(ret);
        return MT_FAILURE;
    }

    /**get GIF image info*/
#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("gif_GetImageInfo\n");
#endif
    ret = gif_GetImageInfo(pGifInstance);
    if (ret != MT_SUCCESS && pGifInstance->gifImageCount == 0)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("gif_GetImageInfo error!\n");
#endif
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/**
 \brief get GIF file type
 \param[in] *pGifInstance 
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_GetFileType( GIF_DECODER_INSTANCE *pGifInstance )
{
    mt_u8 cc[4];

    MTGO_MemSet(cc, 0, 4);
    GIF_READ(pGifInstance->gifIo, cc, 3);

    /*if(strncmp(cc,"GIF",3) != 0)*/
    if ((cc[0] != 'G') || (cc[1] != 'I') || (cc[2] != 'F'))
    {
        return MT_FAILURE;
    }

    MTGO_MemSet(cc, 0, 4);
    GIF_READ(pGifInstance->gifIo, cc, 3);

    /*if( (strncmp(cc, "89a", 3) != 0) && (strncmp(cc, "87a", 3) != 0) )*/
    if ((cc[0] != '8') || ((cc[1] != '7') && (cc[1] != '9')) || (cc[2] != 'a'))
    {
        return MT_FAILURE;
    }

    /*if( strncmp(cc,"89a",3) == 0 )*/
    if (cc[1] == '9')
    {
        pGifInstance->gifVer = 1;
    }
    else
    {
        pGifInstance->gifVer = 0;
    }

    return MT_SUCCESS;
}


static mt_s32 gif_GetGlobalInfo( GIF_DECODER_INSTANCE *pGifInstance )
{
    GIF_GLOBAL_INFO *pGifGlobalInfo;
    mt_u8 be;

    pGifGlobalInfo = &pGifInstance->gifGlobalInfo;
    GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pGifGlobalInfo->scrWidth, 2);
    GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pGifGlobalInfo->scrHeight, 2);
    GIF_READ(pGifInstance->gifIo, &be, 1);

    /**get flag of global palette existed */
    if ((be & 0x80) != 0)
    {
        pGifGlobalInfo->gFlag = MT_TRUE;
    }
    else
    {
        pGifGlobalInfo->gFlag = MT_FALSE;
    }

    pGifGlobalInfo->gColorTable = NULL;
    pGifGlobalInfo->colorRes = (mt_u8)(((be & 0x70) >> 4) + 1);    
    if (pGifGlobalInfo->gFlag > 0)
    {
        if ((be & 0x08) != 0)
        {
            pGifGlobalInfo->gSort = MT_TRUE;
        }
        else
        {
            pGifGlobalInfo->gSort = MT_FALSE;
        }

        pGifGlobalInfo->gSize   = 1;
        pGifGlobalInfo->gSize <<= ((be & 0x07) + 1);
        pGifGlobalInfo->gColorTable = (mt_u8 *)gif_Malloc(GIF_MALLOC_SYS, pGifGlobalInfo->gSize * 3);
        if (NULL == pGifGlobalInfo->gColorTable)
        {
            MTGO_ERROR(MTGO_ERR_NOMEM);
            return MTGO_ERR_NOMEM;
        }
    }

    GIF_READ(pGifInstance->gifIo, &be, 1);
    pGifGlobalInfo->BKColorIdx = be;
    GIF_READ(pGifInstance->gifIo, &be, 1);
    pGifGlobalInfo->pixelAspectRatio = be;
    if (pGifGlobalInfo->gFlag > 0)
    {
        GIF_READ(pGifInstance->gifIo, pGifGlobalInfo->gColorTable, pGifGlobalInfo->gSize * 3);
    }

    return MT_SUCCESS;
}

/**
 \brief get GIFsingle info
 \param[in] *pGifInstance GIF
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_GetImageInfo( GIF_DECODER_INSTANCE *pGifInstance )
{
    mt_u8 be;
    GIF_IMAGE_INFO *pf;
    mt_u32 pos;

    /*for filter the data of extension control block*/
    GIF_GCTRLEXT ctrlExt = {0};

    pGifInstance->gifImageCount = 0;
    while (pGifInstance->gifImageCount < GIF_MAX_IMAGE)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("gif_GetImageInfo : gifImageCount = %u\n", pGifInstance->gifImageCount);
#endif
        pf = &pGifInstance->gifImageInfo[pGifInstance->gifImageCount];
        pf->format = MTGO_PF_8888;
        GIF_READ(pGifInstance->gifIo, &be, 1);
        switch (be)
        {
        case 0x21:     /**extension block */

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("Process new block 0x%02X\n", be);
#endif
            GIF_READ(pGifInstance->gifIo, &be, 1);
            switch (be)
            {
            case 0xf9:         
                while (pGifInstance->gifImageCount < GIF_MAX_IMAGE)
                {
                    GIF_READ(pGifInstance->gifIo, &be, 1);
                    if (be == 0)
                    {
                        break;
                    }

                    if (be == 4)
                    {
                        ctrlExt.active = MT_TRUE;
                        GIF_READ(pGifInstance->gifIo, &be, 1);
                        ctrlExt.disposalMethod = (be & 0x1c) >> 2;
                        if ((be & 0x02) != 0)
                        {
                            ctrlExt.userInputFlag = MT_TRUE;
                        }
                        else
                        {
                            ctrlExt.userInputFlag = MT_FALSE;
                        }

                        if ((be & 0x01) != 0)
                        {
                            ctrlExt.trsFlag = MT_TRUE;
                        }
                        else
                        {
                            ctrlExt.trsFlag = MT_FALSE;
                        }

                        GIF_READ(pGifInstance->gifIo, (mt_u8*)&(ctrlExt.delayTime), 2);
                        GIF_READ(pGifInstance->gifIo, &be, 1);
                        ctrlExt.trsColorIndex = be;
                    }
                    else
                    {
                        GIF_SEEKOFF(pGifInstance->gifIo, be);
                    }
                }

#if 0
                while (pGifInstance->gifImageCount < GIF_MAX_IMAGE)
                {
                    GIF_READ(pGifInstance->gifIo, &be, 1);
                    if (be == 0)
                    {
                        break;
                    }

                    if (be == 4)
                    {
                        pf->ctrlExt.active = TRUE;
                        GIF_READ(pGifInstance->gifIo, &be, 1);
                        pf->ctrlExt.disposalMethod = (be & 0x1c) >> 2;
                        if ((be & 0x02) != 0)
                        {
                            pf->ctrlExt.userInputFlag = TRUE;
                        }
                        else
                        {
                            pf->ctrlExt.userInputFlag = FALSE;
                        }

                        if ((be & 0x01) != 0)
                        {
                            pf->ctrlExt.trsFlag = TRUE;
                        }
                        else
                        {
                            pf->ctrlExt.trsFlag = FALSE;
                        }

                        GIF_READ(pGifInstance->gifIo, (mt_u8 *)&(pf->ctrlExt.delayTime), 2);
                        GIF_READ(pGifInstance->gifIo, &be, 1);
                        pf->ctrlExt.trsColorIndex = be;
                    }
                    else
                    {
                        GIF_SEEKOFF(pGifInstance->gifIo, be);
                    }
                }
#endif


                break;
            case 0xfe:         /**explain block */
            case 0x01:         /**text extend block */                
            case 0xff:       /*lint !e616 !e825*/  
                if(be == 0x01)
                  ctrlExt.active = MT_FALSE;
                while (pGifInstance->gifImageCount < GIF_MAX_IMAGE)
                {
                    GIF_READ(pGifInstance->gifIo, &be, 1);
                    if (be == 0)
                    {
                        break;
                    }

                    GIF_SEEKOFF(pGifInstance->gifIo, be);
                }

                break;
            default:

#ifdef GIF_PRINT_DBGINFO
                MTGO_PrintF("0x21 Error!\n");
#endif
                return MT_FAILURE;
            }

            break;
        case 0x2c:     /**image data block */
        {
            mt_u8 bp;

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("Process new block 0x%02X\n", be);
#endif
            GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pf->imageLPos, 2);
            GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pf->imageTPos, 2);
            GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pf->imageWidth, 2);
            GIF_READ(pGifInstance->gifIo, (mt_u8 *)&pf->imageHeight, 2);
            GIF_READ(pGifInstance->gifIo, &bp, 1);

            if ((bp & 0x40) != 0)
            {
                pf->interlaceFlag = MT_TRUE;
            }
            else
            {
                pf->interlaceFlag = MT_FALSE;
            }
            
            if ((bp & 0x80) != 0)
            {
                pf->lFlag = MT_TRUE;
            }
            else
            {
                pf->lFlag = MT_FALSE;
            }            

            if ((bp & 0x20) != 0)
            {
                pf->sortFlag = MT_TRUE;
            }
            else
            {
                pf->sortFlag = MT_FALSE;
            }

            if (pf->lFlag)
            {
                pf->lSize   = 1;
                pf->lSize <<= ((bp & 0x07) + 1);
                pf->pColorTable = (mt_u8 *)gif_Malloc(GIF_MALLOC_SYS, pf->lSize * 3);
                if (pf->pColorTable == NULL)
                {
#ifdef GIF_PRINT_DBGINFO
                    MTGO_PrintF("Alloc pf->pColorTable error!\n");
#endif
                    return MTGO_ERR_NOMEM;
                }

                GIF_READ(pGifInstance->gifIo, pf->pColorTable, pf->lSize * 3);
            }

            if (MT_SUCCESS != gif_GetPos(pGifInstance->gifIo, &pos))
            {
                return MT_FAILURE;
            }

            pf->dataindex = pos;

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("pf->dataindex = %u(0x%X)\n", pf->dataindex, pf->dataindex);
#endif

            /**skip image data */
            GIF_READ(pGifInstance->gifIo, &be, 1);     /**skip  bytes of LZW len */
            while (pGifInstance->gifImageCount < GIF_MAX_IMAGE)
            {
                GIF_READ(pGifInstance->gifIo, &be, 1);     /**get length about image data block */

#ifdef GIF_PRINT_DBGINFO
             //   MTGO_PrintF("be = 0x%02X\n", be);
#endif
                if (be == 0)      /**end of image data block */
                {
                    break;
                }

                GIF_SEEKOFF(pGifInstance->gifIo, be);
            }

            if (MT_TRUE == ctrlExt.active)
            {
                pf->ctrlExt.active = MT_TRUE;
                pf->ctrlExt.disposalMethod = ctrlExt.disposalMethod;
                pf->ctrlExt.userInputFlag = ctrlExt.userInputFlag;
                pf->ctrlExt.delayTime = ctrlExt.delayTime;
                pf->ctrlExt.trsFlag   = ctrlExt.trsFlag;                
                pf->ctrlExt.trsColorIndex = ctrlExt.trsColorIndex;

                ctrlExt.active = MT_FALSE;
            }

            pGifInstance->gifImageCount++;

            break;
        }
        case 0x3b:     /**GIF file end flag */

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("Process new block 0x%02X, end of file!\n", be);
#endif
            return MT_SUCCESS;
        case 0x00:     /**block end flag */

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("Process new block 0x%02X\n", be);
#endif
            break;
        default:     /**invalid  */

#ifdef GIF_PRINT_DBGINFO
            MTGO_PrintF("Process new block 0x%02X is invalid block\n", be);
#endif
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

/**
 \brief get LWZ block
 \param[in] *pGifInstance GIF decoder instance 
 \param[out] *buf pointer for LWZ data buffer
 \retval LWZ data size
 \retval -1 
 \return mt_s32
 */
static mt_s32 gif_GetDataBlock( GIF_DECODER_INSTANCE *pGifInstance, mt_u8 *buf )
{
    mt_u8 count;

    if (MT_SUCCESS != gif_ReadFile(pGifInstance->gifIo, &count, 1))
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("error in getting DataBlock size\n");
#endif
        return -1;
    }

    pGifInstance->ZeroDataBlock = count == 0;

    if ((count != 0) && (MT_SUCCESS != gif_ReadFile(pGifInstance->gifIo, buf, count)))
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("error in reading DataBlock\n");
#endif
        return -1;
    }

    return count;
}

/**
 \brief decoder LWZ data 
 \param[in] *pGifInstance GIF decoder instance 
 \param[in] code_size LWZ data size
 \param[in] flag  initial flag
 \retval decoder output data 
 \retval -1 
 \return mt_s32
 */
static mt_s32 gif_GetCode( GIF_DECODER_INSTANCE *pGifInstance, mt_s32 code_size, mt_s32 flag )
{
    mt_s32 i, j, ret;
    mt_u8 count;

    if (flag)
    {
        pGifInstance->lastbit = 0;
        pGifInstance->curbit  = 0;        
        pGifInstance->done = MT_FALSE;
        return 0;
    }

    if ((pGifInstance->curbit + code_size) >= pGifInstance->lastbit)
    {
        if (pGifInstance->done)
        {
            if (pGifInstance->curbit >= pGifInstance->lastbit)
            {
#ifdef GIF_PRINT_DBGINFO
                MTGO_PrintF("ran off the end of my bits" );
#endif
            }

            return -1;
        }

        pGifInstance->buf[0] = pGifInstance->buf[pGifInstance->last_byte - 2];
        pGifInstance->buf[1] = pGifInstance->buf[pGifInstance->last_byte - 1];

        if ((count = (mt_u8)gif_GetDataBlock(pGifInstance, &pGifInstance->buf[2])) == 0)
        {
            pGifInstance->done = MT_TRUE;
        }

        pGifInstance->last_byte = 2 + count;
        pGifInstance->curbit  = (pGifInstance->curbit - pGifInstance->lastbit) + 16;
        pGifInstance->lastbit = (2 + count) * 8;
    }

    ret = 0;
    for (i = pGifInstance->curbit, j = 0; j < code_size; ++i, ++j)
    {
        ret |= ((pGifInstance->buf[i / 8] & (1 << (i % 8))) != 0) << j; /*lint !e514 */
    }

    pGifInstance->curbit += code_size;

    return ret;
}

/**
 \brief decode LZW data
 \param[in] *pGifInstance GIF pointer to decoder instancce 
 \param[in] flag  of initialed 
 \param[in] input_code_size inpuut LWZ data len
 \retval output data has been decoded
 \retval < 0 
 \return mt_s32
 */
static mt_s32 gif_LZWReadByte( GIF_DECODER_INSTANCE *pGifInstance, mt_s32 flag, mt_s32 input_code_size )
{
    GIF_DECODER_INSTANCE *p;    
    register mt_s32 i;
    mt_s32 code, incode;

    p = pGifInstance;
    if (flag)
    {
        p->set_code_size = input_code_size;
        p->code_size  = p->set_code_size + 1;
        p->clear_code = 1 << p->set_code_size;
        p->end_code = p->clear_code + 1;
        p->max_code_size = 2 * p->clear_code;
        p->max_code = p->clear_code + 2;
        i = gif_GetCode(p, 0, MT_TRUE);
        p->fresh = MT_TRUE;
        for (i = 0; i < p->clear_code; ++i)
        {
            p->table[0][i] = 0;
            p->table[1][i] = i;
        }

        for (; i < (1 << GIF_MAX_LWZ_BITS); ++i)
        {
            /*p->table[0][i] = p->table[1][0] = 0;*/


            p->table[0][i] = p->table[1][i] = 0;
        }

        p->sp = p->stack;

        return 0;
    }
    else if (p->fresh)
    {
        p->fresh = MT_FALSE;
        do
        {
            p->firstcode = p->oldcode = gif_GetCode(p, p->code_size, MT_FALSE);
        } while (p->firstcode == p->clear_code);

        return p->firstcode;
    }

    if (p->sp > p->stack)
    {
        return *--p->sp;
    }

    while ((code = gif_GetCode(p, p->code_size, MT_FALSE)) >= 0)
    {
        if (code == p->clear_code)
        {
            for (i = 0; i < p->clear_code; ++i)
            {
                p->table[0][i] = 0;
                p->table[1][i] = i;
            }

            for (; i < (1 << GIF_MAX_LWZ_BITS); ++i)
            {
                p->table[0][i] = p->table[1][i] = 0;
            }

            p->code_size = p->set_code_size + 1;
            p->max_code_size = 2 * p->clear_code;
            p->max_code = p->clear_code + 2;
            p->sp = p->stack;
            p->firstcode = p->oldcode = gif_GetCode(p, p->code_size, MT_FALSE);

            return p->firstcode;
        }
        else if (code == p->end_code)
        {
            mt_s32 count;
            mt_u8 buf[260];

            if (p->ZeroDataBlock)
            {
                return -2;
            }

            while ((count = gif_GetDataBlock(p, buf)) > 0)
            {
                ;
            }

            if (count != 0)
            {
#ifdef GIF_PRINT_DBGINFO
                MTGO_PrintF("missing EOD in data stream (common occurence)\n");
#endif
            }

            return -2;
        }

        incode = code;
        if (code >= p->max_code)
        {
            *p->sp++ = p->firstcode;
            code = p->oldcode;
        }

        while (code >= p->clear_code)
        {
            *p->sp++ = p->table[1][code];
            if (code == p->table[0][code])
            {
#ifdef GIF_PRINT_DBGINFO
                MTGO_PrintF("circular table entry BIG ERROR");
#endif
            }

            code = p->table[0][code];
        }

        *p->sp++ = p->firstcode = p->table[1][code];

        if ((code = p->max_code) < (1 << GIF_MAX_LWZ_BITS))
        {
            p->table[1][code] = p->firstcode;
            p->table[0][code] = p->oldcode;            
            ++p->max_code;
            if ((p->max_code >= p->max_code_size) && (p->max_code_size < (1 << GIF_MAX_LWZ_BITS)))
            {
                p->max_code_size *= 2;
                ++p->code_size;
            }
        }

        p->oldcode = incode;

        if (p->sp > p->stack)
        {
            return *--p->sp;
        }
    }

    return code;
}

/**
 \brief decode the image has been index
 \param[in] *pGifInstance GIF poniter to decoder instance
 \param[in] image_index image index
 \retval MT_SUCCESS 
 \retval MT_FAILURE 
 \return mt_s32
 */
static mt_s32 gif_ExtractData( GIF_DECODER_INSTANCE *pGifInstance, mt_u32 image_index )
{
    GIF_IMAGE_INFO *f;
    mt_u8 c;
    mt_s32 v;
    mt_s32 len, height;    
    mt_u8 *out;
    mt_s32 get_data_num = 0;
    mt_s32 xpos = 0, ypos = 0, pass = 0;

    if (image_index >= pGifInstance->gifImageCount)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("frame_index %d error!\n", image_index);
#endif
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    f   = &pGifInstance->gifImageInfo[image_index];
    height = f->imageHeight;
    len = f->imageWidth;    
#ifdef TEST_IN_ROOTBOX
    f->dataBuf = (mt_u8 *)gif_Malloc(GIF_MALLOC_SYS, (mt_u32)(len * height));
#else
    f->dataBuf = (mt_u8 *)gif_Malloc(GIF_MALLOC_MTGO, (mt_u32)(len * height));
#endif
    if (f->dataBuf == NULL)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("Alloc databuf error!\n");
#endif
        MTGO_ERROR(MTGO_ERR_NOMEM);
        return MTGO_ERR_NOMEM;
    }

    out = f->dataBuf;

    /* Initialize the Compression routines */

#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("Seek data %u\n", f->dataindex);
#endif
    GIF_SEEK(pGifInstance->gifIo, (mt_s32)f->dataindex);

    if (MT_SUCCESS != gif_ReadFile(pGifInstance->gifIo, &c, 1))
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("EOF / read error on image data\n");
#endif
#ifdef TEST_IN_ROOTBOX
        gif_Free(GIF_MALLOC_SYS, f->dataBuf);
#else
        gif_Free(GIF_MALLOC_MTGO, f->dataBuf);
#endif
        f->dataBuf = NULL;
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    if (gif_LZWReadByte(pGifInstance, MT_TRUE, c) < 0)
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("error reading image\n");
#endif
#ifdef TEST_IN_ROOTBOX
        gif_Free(GIF_MALLOC_SYS, f->dataBuf);
#else
        gif_Free(GIF_MALLOC_MTGO, f->dataBuf);
#endif

        f->dataBuf = NULL;
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    while ((ypos < height) && (v = gif_LZWReadByte(pGifInstance, MT_FALSE, c)) >= 0)
    {
        out[ypos * len + xpos] = (mt_u8)v;

        get_data_num++;
        ++xpos;
        if (xpos == len)
        {
            xpos = 0;
            if (f->interlaceFlag)
            {
                ypos += dpass[pass];
                if (ypos >= height)
                {
                    ypos = restart[++pass];
                }
            }
            else
            {
                ++ypos;
            }
        }
    }

#ifdef GIF_PRINT_DBGINFO
    MTGO_PrintF("get_data_num = %d\n", get_data_num);
#endif

    if (get_data_num != (len * height))
    {
#ifdef GIF_PRINT_DBGINFO
        MTGO_PrintF("decode pixel number is not match!\n");
#endif
#ifdef TEST_IN_ROOTBOX
       gif_Free(GIF_MALLOC_SYS, f->dataBuf);
#else
       gif_Free(GIF_MALLOC_MTGO, f->dataBuf);
#endif
        f->dataBuf = NULL;
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/**
 \brief ´ÓGIF read data from the file 
 \param[in] hFileIo GIF file handle
 \param[out] *pu8Addr address of read data buffer
 \param[in] u32Len  read length 
 \retval MT_SUCCESS  
 \retval MT_FAILURE  
 \return mt_s32
 */
static mt_s32 gif_ReadFile( IO_HANDLE hFileIo, mt_u8 *pu8Addr, mt_u32 u32Len )
{
    mt_u32 read_len;    
    MT_BOOL EndFlag;
    mt_s32 ret;

    ret = MTGO_ADP_IORead(hFileIo, pu8Addr, u32Len, &read_len, &EndFlag);
    if ((MT_SUCCESS != ret) || (u32Len != read_len))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/**
 \brief  do file seek position
 \param[in] hFileIo GIF fille handle
 \param[in] pos  seek position
 \retval MT_SUCCESS  
 \retval MT_FAILURE  
 \return mt_s32
 */
static mt_s32 gif_SeekFile( IO_HANDLE hFileIo, mt_s32 pos )
{
    return (MTGO_ADP_IOSeek(hFileIo, IO_POS_SET, pos));
}

/**
 \brief seek position to the offset from current position 
 \param[in] hFileIo GIF fille handle
 \param[in] pos  offset from current position
 \retval MT_SUCCESS  
 \retval MT_FAILURE  
 \return mt_s32
 */
static mt_s32 gif_SeekOffFile( IO_HANDLE hFileIo, mt_s32 off )
{
    return (MTGO_ADP_IOSeek(hFileIo, IO_POS_CUR, off));
}

/**
 \brief  get current file position 
 \param[in] hFileIo GIF file handle 
 \param[out] *pos  current file position
 \retval MT_SUCCESS  
 \retval MT_FAILURE  
 \return mt_s32
 */
static mt_s32 gif_GetPos( IO_HANDLE hFileIo, mt_u32 *pos )
{
    return (MTGO_ADP_IOGetPos(hFileIo, pos));
}

/**
 \brief  allocate memory 
 \param[in] flag  mode
                        0 use malloc
                        1 use umap
 \param[in] size 
 \retval   success
 \retval NULL   fail 
 \return mt_void*
 */
static mt_void* gif_Malloc( mt_u32 flag, mt_u32 size )
{
    mt_void *p = NULL;
    phys_addr_t phyAddr = 0;
	
    if (flag >= GIF_MALLOC_BUTT)
    {
        return NULL;
    }

    if (flag == GIF_MALLOC_MTGO)
    {
#ifdef TEST_IN_ROOTBOX
        p = MTGO_MMZ_Malloc(size, MTGO_MMZ_DEFAULT);
#else
        //p = MTGO_MMZ_Malloc(size);
		p = (mt_u8*)MTGO_MMZ_Malloc(size, &phyAddr);

#endif
    }
    else
    {
        p = (mt_void *)MTGO_Malloc(size);
    }

    return p;
}

/**
 \brief  free the memory 
 \param[in] flag  mode
                        0 use malloc
                        1 use umap
 \param[in] *pdata  
 \return mt_void
 */
static mt_void gif_Free( mt_u32 flag, mt_void *pdata )
{
    if ((flag >= GIF_MALLOC_BUTT) || (pdata == NULL))
    {
        return;
    }

    if (flag == GIF_MALLOC_MTGO)
    {
        MTGO_MMZ_Free( pdata);
    }
    else
    {
        //free(pdata);
        MTGO_Free(pdata);
    }
}

/**
 \brief get decoder instance 
 \param[in] pHandle GIF handle 
 \param[in] Modle handle type 
 \retval poniter to decoder instance success
 \retval NULL fail
 \return mt_void*
 */
static mt_void* gif_DecGetInstance(DEC_HANDLE Handle, MTGO_MOD_E Modle)
{
    mt_void *p;

    p = (mt_void *)Handle;

    return p;

}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


