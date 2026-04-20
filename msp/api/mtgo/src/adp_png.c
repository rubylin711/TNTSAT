/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include "adp_png.h"
#include "mt_go_config.h"

#ifdef MTGO_PNG_SUPPORT

#include "png.h"
#include "mtgo_common.h"
#include "mtgo_adp_sys.h"
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_png_api.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
//extern void png_hw_force_exit(void);

#ifdef MTGO_PNG_SUPPORT
/***************************** Macro Definition ******************************/

#define MEM_IO_SUPPORT 1 
extern pix_fmt_t colortype_to_pixfmt(mt_u32 color_type, mt_u32 color_depth);
/*************************** Structure Definition ****************************/
typedef struct _PNG_DEC
{
#if MEM_IO_SUPPORT
    IO_HANDLE hStream;
#else
    FILE *pFile;
#endif
    png_structp pPng;
    png_infop   pInfo;
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)    
    MT_BOOL bHwDec;
    MT_HANDLE hHWPng;
#endif
#ifdef CONFIG_MT_FPGA_GPE  
    MT_BOOL dbg_HWSupport;
	MT_BOOL dbg_comp_dis;
#endif
   mt_u32 raw_data_enable;
} PNG_DEC_S;

/********************** Global Variable declaration **************************/
#ifdef CONFIG_MT_FPGA_GPE
#include <sys/time.h>
static  struct timeval loop_test_start, loop_test_end;
ulong total_cost_time = 0;
#endif

static pix_fmt_t mtgofmt_to_pixfmt(MTGO_PF_E mtgo_fmt);
/******************************* API declaration *****************************/
#if MEM_IO_SUPPORT
static void PNG_Read(png_structp mtgo_png_ptr, png_bytep data, png_size_t length)
{
    mt_s32 Ret;
    mt_u32 CopyLen;
    MT_BOOL EndFlag;

    if (mtgo_png_ptr == NULL)
    {
        return;
    }
#if PNG_LIBPNG_VER < 10634
    Ret = MTGO_ADP_IORead((IO_HANDLE)(mtgo_png_ptr->io_ptr), (mt_void*)data, length, &CopyLen, &EndFlag);
#else
	Ret = MTGO_ADP_IORead((IO_HANDLE)(png_get_io_ptr(mtgo_png_ptr)), (mt_void*)data, length, &CopyLen, &EndFlag);
#endif
    if ((MT_SUCCESS != Ret) || (CopyLen != length))
    {
        png_error(mtgo_png_ptr, (const mt_char *)"Read Error");
    }

    return;
}

#endif

/** pixel format 8888 */
static mt_void PNG_Cvt_8888(const PNG_DEC_S *pPngDec)
{
    mt_s32 ColorType, BitDepth;
    png_structp pPng = pPngDec->pPng;
    png_infop pInfo = pPngDec->pInfo;

    ColorType = png_get_color_type(pPng, pInfo);
    BitDepth = png_get_bit_depth(pPng, pInfo);

    /** bitcount->8 */
    if (ColorType == PNG_COLOR_TYPE_PALETTE)
    {
        png_set_expand(pPng);
    }

    if ((ColorType == PNG_COLOR_TYPE_GRAY) && (BitDepth < 8))
    {
        (mt_void)png_set_expand(pPng);
    }

    if (png_get_valid(pPng, pInfo, PNG_INFO_tRNS))
    {
        png_set_expand(pPng);
    }

    if (BitDepth == 16)
    {
        //mtgo_png_set_swap(pPng);
        png_set_strip_16(pPng);
    }

    /** GRAY->RGB */
    if ((ColorType == PNG_COLOR_TYPE_GRAY)
        || (ColorType == PNG_COLOR_TYPE_GRAY_ALPHA))
    {
        png_set_gray_to_rgb(pPng);
    }

    /** RGB->RGBA */

    //mtgo_png_set_filler(pPng, 0xff, PNG_FILLER_AFTER);
    png_set_add_alpha(pPng, 0xff, PNG_FILLER_AFTER);

    /** RGBA->BGRA */
    png_set_bgr(pPng);
#if 0
    /** Gamma Correction */
    HI_DOUBLE FileGamma, ScreenGamma;

    ScreenGamma = 2.2; /* A good guess for a  PC monitor in a bright office or a dim room */
    if (mtgo_png_get_gAMA(pPng, pInfo, &FileGamma))
    {
        mtgo_png_set_gamma(pPng, ScreenGamma, FileGamma);
    }
#endif


    (mt_void)png_read_update_info(pPng, pInfo);

    return;
}

static mt_void PNG_Get_BkgdColor(const PNG_DEC_S *pPngDec, MT_BOOL *pExist, MT_COLOR *pColor)
{
    mt_u8 Red, Green, Blue;
    png_color_16p pBackground;
    png_structp pPng = pPngDec->pPng;
    png_infop pInfo = pPngDec->pInfo;

    /* setjmp() must be called in every function that calls a PNG-reading
     * libpng function */

    if (setjmp(png_jmpbuf(pPng)))
    {
        (mt_void)png_destroy_read_struct(&pPng, &pInfo, NULL);
        return;
    }

    if (!png_get_valid(pPng, pInfo, PNG_INFO_bKGD))
    {
        *pExist = MT_FALSE;
        return;
    }

    if (!png_get_bKGD(pPng, pInfo, &pBackground))
    {
        *pExist = MT_FALSE;
        return;
    }

    /** bitdepth has been convert to 8 so*/
    Red     = (mt_u8)pBackground->red;
    Green   = (mt_u8)pBackground->green;
    Blue    = (mt_u8)pBackground->blue;
    *pExist = MT_TRUE;
    *pColor = (mt_u32)(((mt_u32)0xFF << 24) | (Red << 16) | (Green << 8) | Blue);

    return;
} 

mt_s32 MTGO_ADP_PngGetActualSize(DEC_HANDLE PngDec, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo)
{
    MTGO_DEC_IMGINFO_S ImgInfo;
    png_infop pInfo;
    PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;
    png_structp pPng;
    pix_fmt_t png_fmt = PIX_FMT_ARGB8888;
    mt_s32 ret;

    pPng  = pPngDec->pPng;
    pInfo = pPngDec->pInfo;
    
    ret = (mt_s32)MTGO_ADP_PngDecImgInfo(PngDec, (mt_u32)Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    pSurInfo->Width = (mt_s32)ImgInfo.Width;
    pSurInfo->Height = (mt_s32)ImgInfo.Height;
    pSurInfo->PixelFormat = ImgInfo.Format;
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)

    if(pPngDec->bHwDec)
    {
      if(pPngDec->raw_data_enable > 0)
      	{
      		png_fmt = mtgofmt_to_pixfmt(pSurInfo->PixelFormat);
      	}
	MT_PNG_GetRowBytes(pPngDec->hHWPng, png_fmt, &(pSurInfo->Pitch[0]));
    }
    else
#endif      
      pSurInfo->Pitch[0] = (mt_u32)png_get_rowbytes(pPng, pInfo);

    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_PngSetRawData(DEC_HANDLE PngDec, mt_u32 enable)
{
      PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;
	  
      pPngDec->raw_data_enable = enable > 0 ? 1 : 0;
      return MT_SUCCESS;
}

mt_s32 MTGO_ADP_PngCreateDecoder(DEC_HANDLE *pPngDec, const MTGO_DEC_ATTR_S *pSrcDesc)
{
    PNG_DEC_S *pstPngDec;
    png_structp pPng;
    png_infop pInfo;
#ifdef CONFIG_MT_FPGA_GPE
		MT_PNG_DBGCFG_S dbg_cfg;
#endif
    if ((MT_NULL == pPngDec) || (MT_NULL == pSrcDesc))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    /** allocate memory for decode instance*/
    pstPngDec = (PNG_DEC_S*)MTGO_Malloc(sizeof(PNG_DEC_S));
    if (MT_NULL == pstPngDec)
    {
        MTGO_ERROR(MT_FAILURE);   
        return MT_FAILURE;
    }

    /** initial png decode instance */
#if MEM_IO_SUPPORT
    /**create decode instance handle */
    IO_DESC_S stIODesc;
    IO_HANDLE hStream;
    mt_s32 ret;
#ifdef TEST_IN_ROOTBOX
    stIODesc.Type = (IO_TYPE_E)pSrcDesc->SrcType;
#endif
    stIODesc.IoInfo.MemInfo.pAddr  = pSrcDesc->SrcInfo.MemInfo.pAddr;
    stIODesc.IoInfo.MemInfo.Length = pSrcDesc->SrcInfo.MemInfo.Length;
#ifdef TEST_IN_ROOTBOX
    stIODesc.IoInfo.pFileName = (const mt_char *)pSrcDesc->SrcInfo.pFileName;
#endif
    ret = MTGO_ADP_IOCreate(&hStream, &stIODesc);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(MT_FAILURE);
        goto ERR1;
    }

    pstPngDec->hStream = hStream;
#endif

#ifdef CONFIG_MT_FPGA_GPE  
	dbg_cfg.comp_dis = pSrcDesc->dbg_cfg.comp_dis;
	dbg_cfg.sw_dec = pSrcDesc->dbg_cfg.sw_dec;
	dbg_cfg.clut_gray_to_8bit =pSrcDesc->dbg_cfg.clut_gray_to_8bit;
	dbg_cfg.conv2argb_dis = pSrcDesc->dbg_cfg.conv2argb_dis;
	dbg_cfg.reset_test = pSrcDesc->dbg_cfg.reset_test;
      //printf("MTGO_ADP_PngCreateDecoder : <%d %d %d %d %d\n", dbg_cfg.comp_dis, dbg_cfg.sw_dec,
      //  dbg_cfg.clut_gray_to_8bit , dbg_cfg.conv2argb_dis, dbg_cfg.reset_test);
#endif


    pPng = png_create_read_struct((const mt_char *)PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (MT_NULL == pPng)
    {  
        MTGO_ERROR(MT_FAILURE);  
        goto ERR2;
    }

    pInfo = png_create_info_struct(pPng);
    if (MT_NULL == pInfo)
    {
        MTGO_ERROR(MT_FAILURE);   
        goto ERR3;
    }

    pstPngDec->pPng  = pPng;
    pstPngDec->pInfo = pInfo;

    /** read png file info*/

    // TODO:think about when needed to call setjmp
    if (setjmp(png_jmpbuf(pPng)))
    {
        MTGO_ERROR(MT_FAILURE);   
        goto ERR3;
    }

    /** setup the read function */
#if MEM_IO_SUPPORT
    (mt_void)png_set_read_fn(pPng, (png_voidp)hStream, PNG_Read);
#else
    png_init_io(pPng, pFile);
#endif

    (mt_void)png_read_info(pPng, pInfo);
    
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
#ifdef CONFIG_MT_FPGA_GPE
    pstPngDec->bHwDec = MT_PNG_IfHwSupport(!dbg_cfg.sw_dec);
#else		
    pstPngDec->bHwDec = MT_PNG_IfHwSupport(pPng, pInfo);
#endif
    if(pstPngDec->bHwDec) 
    {
      ret = MT_PNG_Open(&pstPngDec->hHWPng);
      if(ret != MT_SUCCESS)
        pstPngDec->bHwDec = MT_FALSE;
      else
      {
#ifdef CONFIG_MT_FPGA_GPE
      	if(pSrcDesc->dbg_cfg.wr_reg_test_flg)
      	{
      		// png_hw_force_exit();
            MT_PNG_reg_test();
            goto ERR3;
        }
      //   printf("<%s> : <%d> : %lx %x\n", __FUNCTION__, __LINE__, (ulong)pSrcDesc->SrcInfo.MemInfo.pAddr , pSrcDesc->SrcInfo.MemInfo.Length);
        MT_PNG_SetStream(pstPngDec->hHWPng, (ulong)pSrcDesc->SrcInfo.MemInfo.pAddr, pSrcDesc->SrcInfo.MemInfo.Length);
        MT_PNG_SetDecInfo(pstPngDec->hHWPng, pPng, pInfo);
        MT_PNG_SetDbgInfo(pstPngDec->hHWPng, &dbg_cfg);
#else
        MT_PNG_SetStream(pstPngDec->hHWPng, (ulong)pSrcDesc->SrcInfo.MemInfo.pAddr, pSrcDesc->SrcInfo.MemInfo.Length);
        MT_PNG_SetDecInfo(pstPngDec->hHWPng, pPng, pInfo);
#endif
      }
    }
    else
#endif
    PNG_Cvt_8888(pstPngDec);

    *pPngDec = (DEC_HANDLE)pstPngDec;
    return MT_SUCCESS;

ERR3:
    if (MT_NULL == pInfo)
    {
        (mt_void)png_destroy_read_struct(&pPng, NULL, NULL);
    }
    else
    {
        (mt_void)png_destroy_read_struct(&pPng, &pInfo, NULL);
    }

ERR2:
#if MEM_IO_SUPPORT
    (mt_void)MTGO_ADP_IODestroy(hStream);
#endif
ERR1:
    MTGO_Free(pstPngDec);
    pstPngDec = MT_NULL;
    
    MTGO_ERROR(MT_FAILURE);

    return MT_FAILURE;
}

mt_s32 MTGO_ADP_PngDestroyDecoder(DEC_HANDLE PngDec)
{
    PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;

    if (MT_NULL == pPngDec)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(pPngDec->bHwDec) 
    {
      MT_PNG_Close(pPngDec->hHWPng);
    }
#endif

#if MEM_IO_SUPPORT
    mt_s32 Ret;
    Ret = MTGO_ADP_IODestroy(pPngDec->hStream);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
#endif


    if (pPngDec->pPng && pPngDec->pInfo)
    {
        (mt_void)png_destroy_read_struct(&(pPngDec->pPng), &(pPngDec->pInfo), MT_NULL);
        pPngDec->pPng  = MT_NULL;
        pPngDec->pInfo = MT_NULL;
    }
    MTGO_Free(pPngDec);
    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_PngResetDecoder(DEC_HANDLE PngDec)
{
    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_PngDecCommInfo(DEC_HANDLE PngDec, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;
    png_structp pPng;
    png_infop pInfo;

    if (MT_NULL == pPngDec)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pPng  = pPngDec->pPng;
    pInfo = pPngDec->pInfo;

    if (setjmp(png_jmpbuf(pPng)))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pPrimaryInfo->ScrWidth  = (mt_u32)png_get_image_width(pPng, pInfo);
    pPrimaryInfo->ScrHeight = (mt_u32)png_get_image_height(pPng, pInfo);
    PNG_Get_BkgdColor(pPngDec, &(pPrimaryInfo->IsHaveBGColor), &(pPrimaryInfo->BGColor));
    pPrimaryInfo->Count   = 1;
    pPrimaryInfo->ImgType = MTGO_DEC_IMGTYPE_PNG;

    return MT_SUCCESS;
}

static MTGO_PF_E pixfmt_to_mtgofmt(pix_fmt_t png_fmt)
{
      MTGO_PF_E mtgo_fmt = MTGO_PF_8888;

     switch(png_fmt)
     {
          case PIX_FMT_RGBPALETTE1:
		  	mtgo_fmt = MTGO_PF_CLUT1;
		  	break;
          case PIX_FMT_RGBPALETTE4:
		  	mtgo_fmt = MTGO_PF_CLUT4;
		  	break;
          case PIX_FMT_RGBPALETTE8:
		  	mtgo_fmt = MTGO_PF_CLUT8;
		  	break;
	   case PIX_FMT_GRAY_8:
	   	      mtgo_fmt = MTGO_PF_A8;
		      break;
	   default:
	   	break;
     }

      return mtgo_fmt;
}

static pix_fmt_t mtgofmt_to_pixfmt(MTGO_PF_E mtgo_fmt)
{
      pix_fmt_t pix_fmt = PIX_FMT_ARGB8888;

     switch(mtgo_fmt)
     {
          case MTGO_PF_CLUT1 :
		  	pix_fmt = PIX_FMT_RGBPALETTE1;
		  	break;
          case MTGO_PF_CLUT4 :
		  	pix_fmt = PIX_FMT_RGBPALETTE4;
		  	break;
          case MTGO_PF_CLUT8 :
		  	pix_fmt = PIX_FMT_RGBPALETTE8;
		  	break;
	   case MTGO_PF_A8:
		  	pix_fmt = PIX_FMT_GRAY_8;
		  	break;
	   default:
	   	break;
     }

      return pix_fmt;
}


mt_s32 MTGO_ADP_PngDecImgInfo(DEC_HANDLE PngDec, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;
    png_structp pPng;
    png_infop pInfo;
    mt_u32 color_depth = 0;
    mt_u32 color_type = 0;
    pix_fmt_t png_fmt;

    if ((MT_NULL == pPngDec) || (MT_NULL == pImgInfo))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pPng  = pPngDec->pPng;
    pInfo = pPngDec->pInfo;

    if (setjmp(png_jmpbuf(pPng)))
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }

    pImgInfo->OffSetX = 0;
    pImgInfo->OffSetY = 0;
    pImgInfo->Width  = (mt_u32)png_get_image_width(pPng, pInfo);
    pImgInfo->Height = (mt_u32)png_get_image_height(pPng, pInfo);
    pImgInfo->Alpha = 0xFF;
    pImgInfo->IsHaveKey = MT_FALSE;
    pImgInfo->Format = MTGO_PF_8888;
    pImgInfo->DelayTime = 0;
    pImgInfo->DisposalMethod = (mt_u32)png_get_rowbytes(pPng, pInfo);
    if(pPngDec->raw_data_enable > 0)
    {
    	color_depth = png_get_bit_depth(pPng, pInfo);
    	color_type = png_get_color_type(pPng, pInfo);
	    png_fmt = colortype_to_pixfmt(color_type, color_depth);	    
	    pImgInfo->Format = pixfmt_to_mtgofmt(png_fmt);
	    //printf("<%s> : <%d> pImgInfo->Format : %d <%d %d %d>\n", __FUNCTION__, __LINE__, pImgInfo->Format ,color_type, color_depth, png_fmt);
    }

    return MT_SUCCESS;
}

mt_s32 MTGO_ADP_PngDecImgData(DEC_HANDLE PngDec, mt_u32 Index, MTGO_SURFACE_S *pSurface)
{
    PNG_DEC_S *pPngDec = (PNG_DEC_S*)PngDec;
    png_structp pPng;
    png_infop pInfo;
    mt_u32 Pitch, Height, i;
    mt_u8 *pData = MT_NULL, **ppLines = MT_NULL;
    MT_PIXELDATA pPixelData = {0};

    MTGO_ASSERT(pPngDec != NULL);
    MTGO_ASSERT(pSurface != NULL);    
   
    pPng  = pPngDec->pPng;
    pInfo = pPngDec->pInfo;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(pPngDec->bHwDec)
    {
      mt_s32 ret;
      MT_PNG_IMAGE_INFO_S outImageInfo = {0};
      (mt_void)Surface_LockSurface((MTGO_HANDLE)pSurface, pPixelData);
      outImageInfo.phyAddr = pPixelData[0].pPhyData;
      outImageInfo.virAddr = pPixelData[0].pData;
      outImageInfo.pixFormat = mtgofmt_to_pixfmt(pSurface->PixelFormat);// PIX_FMT_ARGB8888//pSurface->PixelFormat;
      //printf("<%s> : <%d>  outImageInfo.pixFormat : PIX_FMT_ARGB8888 : %d\n", __FUNCTION__, __LINE__,   outImageInfo.pixFormat);
      MT_PNG_SetOutImgInfo(pPngDec->hHWPng, &outImageInfo);
      (mt_void)Surface_UnlockSurface((MTGO_HANDLE)pSurface);   
#ifdef CONFIG_MT_FPGA_GPE
      gettimeofday(&loop_test_start, NULL);
	  printf("start decoding PNG PIC\n");
   
#endif
      ret = MT_PNG_Decode(pPngDec->hHWPng);
#ifdef CONFIG_MT_FPGA_GPE
	gettimeofday(&loop_test_end, NULL);
		total_cost_time += ((loop_test_end.tv_sec - loop_test_start.tv_sec) * 1000000 + loop_test_end.tv_usec) - loop_test_start.tv_usec;
   printf("PNG decode finish  total_cost_time : %ld  : %ld %ld %ld %ld %ld\n", 	 total_cost_time,  loop_test_end.tv_sec,  loop_test_start.tv_sec, loop_test_end.tv_usec
    ,  loop_test_start.tv_usec, (1000000*(loop_test_end.tv_sec - loop_test_start.tv_sec) + loop_test_end.tv_usec) - loop_test_start.tv_usec);
#endif

     if(outImageInfo.pixFormat <= PIX_FMT_RGBPALETTE8)
     {
        MT_PALETTE Palette256;
        if(MT_SUCCESS == MT_PNG_GetPalette(pPngDec->hHWPng, Palette256, 256))
        {
		    Surface_SetSurfacePalette((MTGO_HANDLE)pSurface, Palette256);
        }
     }
  
      return ret;
    }
    else      
#endif
    {
      if (setjmp(png_jmpbuf(pPng)))
      {
          MTGO_ERROR(MT_FAILURE);
          goto ERR1;
      }

      /** request image memory  */

      Pitch  = (mt_u32)png_get_rowbytes(pPng, pInfo);
      Height = (mt_u32)png_get_image_height(pPng, pInfo);
      
      (mt_void)Surface_LockSurface((MTGO_HANDLE)pSurface, pPixelData);
      pData = (mt_u8*)pPixelData[0].pData;
      
      (mt_void)Surface_UnlockSurface((MTGO_HANDLE)pSurface);    
      /** request line address memory, and initial it */
      ppLines = (mt_u8**)MTGO_Malloc(Height * sizeof(mt_u8 *));
      if (MT_NULL == ppLines)
      {
          MTGO_ERROR(MT_FAILURE);
          goto ERR1;
      }

      for (i = 0; i < Height; i++)
      {
          ppLines[i] = pData + i * Pitch;
      }

      /* read image data */
      (mt_void)png_read_image(pPng, ppLines);

      /** free resource*/
      MTGO_Free(ppLines);
      ppLines = NULL;

      return MT_SUCCESS;

  ERR1:
#if 0
      if (MT_NULL != ppLines)
      {
          MTGO_Free(ppLines);
          ppLines = NULL;
      }
#endif
      MTGO_ERROR(MTGO_ERR_DEPEND_PNG);
      return MTGO_ERR_DEPEND_PNG;
    }
}

#if 0
mt_s32 MTGO_ADP_PngDecExtendData(DEC_HANDLE PngDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void **pData,
                                 mt_u32 *pLength)
{
    return MT_FAILURE;
} /*lint !e818 */

mt_s32 MTGO_ADP_PngReleaseDecExtendData(DEC_HANDLE PngDec, MTGO_DEC_EXTENDTYPE_E DecExtendType, mt_void *pData)
{
    return MT_FAILURE;
} 
#endif

//lint   +e550  +e831
//lint +e64  +e1013  +e63  +e746  +e534  +e830  +e1055  +e522  +e40  +e10  +e601
#endif

#ifdef __cplusplus
}
#endif



