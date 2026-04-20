/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "string.h"

#include "mtgo_common.h"
#include "mt_go_decoder.h"
#include "mtgo_surface.h"
#include "mtgo_blit.h"
#include "mtgo_adp_sys.h"
#include "mt_go_config.h"
#include "mt_api_mmz.h"
#include "mt_common.h"

//#include "mt_drv_mmz.h"
//#include <sys/time.h>
#ifdef MTGO_GIF_SUPPORT
#include "mtgo_gif.h"
#endif

#ifdef MTGO_BMP_SUPPORT
#include "mtgo_bmp.h"
#endif

#ifdef MTGO_PNG_SUPPORT
#include "adp_png.h"
#endif

#ifdef MTGO_JPEG_SUPPORT
#include "adp_jpeg.h"
#endif 

/***************************** Macro Definition ******************************/
//lint -e605
/** max picture count in a file  */
//#define MTGO_DEC_MAX_PICCOUNT 1000
#define HEAD_MAX_LENGTH 10
#define INVALID_HANDLE 0x0         
#define JPEG_MEM_SIZE 0x100000
/*************************** Structure Definition ****************************/
//static struct timeval begin_time, end_time;

/** sturcture about decoder instance  */
typedef struct _MTGO_DEC_INSTANCE_S
{
    MTGO_DEC_IMGTYPE_E ImgType; /**< deocder image type */
    MTGO_DEC_ROTATE_E rotate;
    mt_handle          Decoder; /**<decoder target , after adapt  for example GIF or jpeg decoder instance */
    mt_s32 (*CreateDecoder)(mt_handle * pDecoder, const MTGO_DEC_ATTR_S * pSrcDesc); /**< create instance in adaptive layer */
    mt_s32 (*DestroyDecoder)(mt_handle Decoder); /**< destroy instance on adaptive layer */
    mt_s32 (*ResetDecoder)(mt_handle Decoder); /**< restart instance on adaptive layer  */
    mt_s32 (*DecCommInfo)(mt_handle Decoder, MTGO_DEC_PRIMARYINFO_S * pPrimaryInfo); /**<decoder picture info */
    mt_s32 (*DecImgInfo)(mt_handle Decoder, mt_u32 Index, MTGO_DEC_IMGINFO_S * pImgInfo); /**< decoder index picture info */
    mt_s32 (*DecImgData)(mt_handle Decoder, mt_u32 Index, MTGO_SURFACE_S * pSurface); /**< decoder image data  */
    mt_s32 (*GetActualSize)(mt_handle Decoder, mt_s32 Index, const MT_RECT *pSrcRect, MTGO_SURINFO_S *pSurInfo); /**< get the size which decoder actual can support */
	mt_s32 (*SetRotate)(mt_handle Decoder, MTGO_DEC_ROTATE_E rotate);
	mt_s32 (*GetCropRect)(mt_handle Decoder, MT_RECT *pCropRect);
   mt_s32 (*SetRawData)(mt_handle Decoder, mt_u32 enable);
} MTGO_DEC_INSTANCE_S;  

static mt_s32  s_InitDecCount = 0;

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
mt_s32 MTGO_DeinitDecoder(mt_void);
mt_s32 MTGO_InitDecoder(mt_void);


static mt_s32 MTGO_ADP_InitDecoder(mt_void)
{
#ifdef MTGO_JPEG_SUPPORT
//    return MTGO_ADP_JpegInit();
	return MT_SUCCESS;
#else
    return MT_SUCCESS;
#endif
}

static mt_s32 MTGO_ADP_DeInitDecoder(mt_void)
{
#ifdef MTGO_JPEG_SUPPORT
//    return MTGO_ADP_JpegDeInit();
	return MT_SUCCESS;
#else
    return MT_SUCCESS;
#endif
}

/**
 \brief  decode de initial 
 \param[in] mt_void
 \param[out] none
 \retval none
 \return none
 */
mt_s32 MTGO_DeinitDecoder(mt_void)
{
     /** avoid has not do initial */
    if (UN_INIT_STATE == s_InitDecCount)
    {
        MTGO_ERROR(MTGO_ERR_NOTINIT);
        return MTGO_ERR_NOTINIT;
    }

    /** exit if has do initial */
    if (s_InitDecCount != CLEAR_INIT_STATE)
    {   
        s_InitDecCount--;
        return MT_SUCCESS;
    }
    (mt_void)MTGO_ADP_DeInitDecoder();
    s_InitDecCount--;
    return MT_SUCCESS;
}
/**
 \brief decoder initial 
 \param[in] mt_void
 \param[out] none
 \retval none
 \return none
 */
mt_s32 MTGO_InitDecoder(mt_void)
{
    mt_s32 s32Ret ;
    //mmz_buffer_s stJpegBuf;
    
    /** re initial and just remember the times  */
    if (UN_INIT_STATE != s_InitDecCount)
    {
        s_InitDecCount++;
        return MT_SUCCESS;
    }
/*
    s32Ret = HI_MEM_Alloc(&stJpegBuf.u32StartPhyAddr, JPEG_MEM_SIZE);
    if(s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(-1);
        return MT_FAILURE;
    }     
*/
    
    /* allocate memory for jpeg decode*/
/*
    s32Ret = HI_MMB_Init(stJpegBuf.u32StartPhyAddr,JPEG_MEM_SIZE);    
    if (s32Ret != 0)
    {
        MTGO_ERROR(s32Ret);
        return MT_FAILURE;
    }
 */
  
    s32Ret = MTGO_ADP_InitDecoder();
    if(MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    s_InitDecCount++;

    return MT_SUCCESS;
}


/**
 \brief query input file type 
 \param[in] const MTGO_DEC_ATTR_S *pSrcDesc input source info 
 \param[out] MTGO_DEC_IMGTYPE_E * pImgType output info
 \retval none
 \return none
 */
static mt_s32 DEC_GetSrcType(const MTGO_DEC_ATTR_S *pSrcDesc, MTGO_DEC_IMGTYPE_E * pImgType)
{
    mt_char HeadInfo[HEAD_MAX_LENGTH];  /** is 10 bytes enougch ?*/

    /** parser input source and get header info*/
        if (MT_NULL_PTR == pSrcDesc->SrcInfo.MemInfo.pAddr)
        {
            MTGO_ERROR(MTGO_ERR_NULLPTR);
            return MTGO_ERR_NULLPTR;
        }
        MTGO_MemCopy(HeadInfo, pSrcDesc->SrcInfo.MemInfo.pAddr, HEAD_MAX_LENGTH);

#ifdef MTGO_GIF_SUPPORT
    /** parser header ,and get image format*/
    /** check is GIF ?*/
    if (MTGO_Strncmp(HeadInfo, (const mt_char *)"GIF", 3) == 0)
    {
        if (MTGO_Strncmp((HeadInfo + 3), (const mt_char *)"87a", 3) == 0 || MTGO_Strncmp((HeadInfo + 3), (const mt_char *)"89a", 3) == 0)
        {
            *pImgType = MTGO_DEC_IMGTYPE_GIF;
            return MT_SUCCESS;
        }
    }
#endif

#ifdef MTGO_JPEG_SUPPORT

    /** check is jpeg? */
    if ((0xff == *((mt_u8*)HeadInfo)) && (0xD8 == *((mt_u8*)(HeadInfo + 1))))
    {
        *pImgType = MTGO_DEC_IMGTYPE_JPEG;
        return MT_SUCCESS;
    }
#endif
    // TODO:research a better way for format recognisze
    
#ifdef MTGO_BMP_SUPPORT
    /** check is BMP ? */
    if ((0x42 == *((mt_u8*)HeadInfo)) && (0x4d == *((mt_u8*)(HeadInfo + 1))))
    {
        *pImgType = MTGO_DEC_IMGTYPE_BMP;
        return MT_SUCCESS;
    }
#endif

#ifdef MTGO_PNG_SUPPORT
    /** check is PNG ?*/
    mt_u8 png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    if (0 == MTGO_MemCmp(HeadInfo, png_signature, 8))
    {
        *pImgType = MTGO_DEC_IMGTYPE_PNG;
        return MT_SUCCESS;
    }
#endif
    MTGO_ERROR(MTGO_ERR_INVIMAGETYPE);
    return MTGO_ERR_INVIMAGETYPE;
}; /*lint !e818 */

mt_s32 MT_GO_CreateDecoder(const MTGO_DEC_ATTR_S *pSrcDesc, mt_handle *pDecoder)
{
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    MTGO_DEC_IMGTYPE_E ImgType = MTGO_DEC_IMGTPYE_BUTT;
    mt_s32 Ret;

    //gettimeofday(&begin_time);
    /** check parameter  */
    if ((MT_NULL_PTR == pSrcDesc) || (MT_NULL_PTR == pDecoder))
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }
    if (UN_INIT_STATE == s_InitDecCount )
    {
        MTGO_ERROR(MTGO_ERR_NOTINIT);
        return MTGO_ERR_NOTINIT;
    }
    /** get file type*/
    Ret = DEC_GetSrcType(pSrcDesc, &ImgType);
    if (Ret != MT_SUCCESS)
    {
        MTGO_ERROR(Ret);
        return Ret;
    }
  /*  printf("<%s>  ImgType is : %d\n", __FUNCTION__, ImgType);

    if(MTGO_DEC_IMGTYPE_PNG != ImgType)
    {
        MTGO_ERROR(MTGO_ERR_INVSRCTYPE);
        printf("<%s> is not png file (ImgType %d)\n", __FUNCTION__, ImgType);
        return MTGO_ERR_INVSRCTYPE;
    }*/
    
    /** create decoder instance */
    pDecInstance = (MTGO_DEC_INSTANCE_S*)MTGO_Malloc(sizeof (MTGO_DEC_INSTANCE_S));
    if (MT_NULL_PTR == pDecInstance)
    {
        MTGO_ERROR(MTGO_ERR_NOMEM);
        return MTGO_ERR_NOMEM;
    }
    MTGO_MemSet(pDecInstance, 0, sizeof (MTGO_DEC_INSTANCE_S));
    //BM_TRACE("++++++++decFile Info ImgType %d \n",ImgType);
    /*lint -e64 */
    /** adaptive decoder */
    switch (ImgType)
    {
#ifdef MTGO_GIF_SUPPORT
    case MTGO_DEC_IMGTYPE_GIF:
    {
        pDecInstance->ImgType = MTGO_DEC_IMGTYPE_GIF;
        pDecInstance->CreateDecoder  = GIF_CreateDecoder; 
        pDecInstance->DestroyDecoder = GIF_DestroyDecoder;
        pDecInstance->ResetDecoder = GIF_ResetDecoder;    
        pDecInstance->DecCommInfo = GIF_DecCommInfo;      
        pDecInstance->DecImgInfo = GIF_DecImgInfo;     
        pDecInstance->DecImgData = GIF_DecImgData;
 #if 0
        pDecInstance->DecExtendData = GIF_DecExtendData;
        pDecInstance->ReleaseDecExtendData = GIF_ReleaseDecExtendData;
        pDecInstance->WriteStream = MT_NULL_PTR;
 #endif
        pDecInstance->GetActualSize = GIF_GetActualSize;
 		pDecInstance->SetRotate = NULL;
		pDecInstance->GetCropRect = NULL;
		pDecInstance->SetRawData = NULL;
        break;
    }
#endif

#ifdef MTGO_JPEG_SUPPORT
    case MTGO_DEC_IMGTYPE_JPEG:
    {
        pDecInstance->ImgType = MTGO_DEC_IMGTYPE_JPEG;
        pDecInstance->CreateDecoder  = MTGO_ADP_JPGCreateDecoder;
        pDecInstance->DestroyDecoder = MTGO_ADP_JPGDestroyDecoder;
        pDecInstance->ResetDecoder = MTGO_ADP_JPGResetDecoder;
        pDecInstance->DecCommInfo = MTGO_ADP_JPGDecCommInfo;
        pDecInstance->DecImgInfo = MTGO_ADP_JPGDecImgInfo;
        pDecInstance->DecImgData = MTGO_ADP_JPGDecImgData;
        pDecInstance->GetActualSize = MTGO_ADP_JPGGetActualSize;
		pDecInstance->SetRotate = MTGO_ADP_JPGSetRotate;
		pDecInstance->GetCropRect = MTGO_ADP_JPGGetCropRect;
		pDecInstance->SetRawData = NULL;
        break;
    }
#endif

#ifdef MTGO_BMP_SUPPORT
    case MTGO_DEC_IMGTYPE_BMP:
    {
        pDecInstance->ImgType = MTGO_DEC_IMGTYPE_BMP;
        pDecInstance->CreateDecoder  = BMP_CreateDecoder;
        pDecInstance->DestroyDecoder = BMP_DestroyDecoder;
        pDecInstance->ResetDecoder = BMP_ResetDecoder;
        pDecInstance->DecCommInfo = BMP_DecCommInfo;
        pDecInstance->DecImgInfo = BMP_DecImgInfo;
        pDecInstance->DecImgData = BMP_DecImgData;
#if 0
        pDecInstance->DecExtendData = BMP_DecExtendData;
        pDecInstance->ReleaseDecExtendData = BMP_ReleaseDecExtendData;
        pDecInstance->WriteStream = MT_NULL_PTR;
 #endif        
        pDecInstance->GetActualSize = BMP_GetActualSize;
 		pDecInstance->SetRotate = NULL;
		pDecInstance->GetCropRect = NULL;
		pDecInstance->SetRawData = NULL;
        break;
    }
#endif

#ifdef MTGO_PNG_SUPPORT
    case MTGO_DEC_IMGTYPE_PNG:
    {
        pDecInstance->ImgType = MTGO_DEC_IMGTYPE_PNG;
        pDecInstance->CreateDecoder  = MTGO_ADP_PngCreateDecoder;
        pDecInstance->DestroyDecoder = MTGO_ADP_PngDestroyDecoder;
        pDecInstance->ResetDecoder = MTGO_ADP_PngResetDecoder;
        pDecInstance->DecCommInfo = MTGO_ADP_PngDecCommInfo;
        pDecInstance->DecImgInfo = MTGO_ADP_PngDecImgInfo;
        pDecInstance->DecImgData = MTGO_ADP_PngDecImgData;
#if 0
        pDecInstance->DecExtendData = MTGO_ADP_PngDecExtendData;
        pDecInstance->ReleaseDecExtendData = MTGO_ADP_PngReleaseDecExtendData;
        pDecInstance->WriteStream = MT_NULL_PTR; 
 #endif       
        pDecInstance->GetActualSize = MTGO_ADP_PngGetActualSize;
 		pDecInstance->SetRotate = NULL;
		pDecInstance->GetCropRect = NULL;
		pDecInstance->SetRawData = MTGO_ADP_PngSetRawData;
        break;
    }
#endif

    default:
    {
        MTGO_ERROR(MTGO_ERR_INVSRCTYPE);
        Ret = MTGO_ERR_INVSRCTYPE;
        goto adpfailed;
    }
    }
    /*lint +e64 */

    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->CreateDecoder);
    /** instance initial*/
    Ret = pDecInstance->CreateDecoder(&(pDecInstance->Decoder), pSrcDesc);
    if (MT_SUCCESS != Ret)
    {
        if (MTGO_ERR_NOMEM != Ret)
        {
            MTGO_ERROR(MTGO_ERR_INVIMGDATA);
            Ret = MTGO_ERR_INVIMGDATA;
        }

        MTGO_SetError(Ret);
        goto adpfailed;
    }

    /** get decoder handle*/
    *pDecoder =  (mt_handle) pDecInstance;

    return MT_SUCCESS;
adpfailed:
    MTGO_Free(pDecInstance);
    pDecInstance = MT_NULL_PTR;
    return Ret;
}

mt_s32 MT_GO_DestroyDecoder(mt_handle Decoder)
{
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;

    pDecInstance = (MTGO_DEC_INSTANCE_S*)Decoder;
    /** destroy decoderinstance */
    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->DestroyDecoder);
    (mt_void)pDecInstance->DestroyDecoder(pDecInstance->Decoder);
    MTGO_Free(pDecInstance);
    //MT_MMB_DeInit();
    //gettimeofday(&end_time);

#if 0//def CONFIG_EMU
    printf("\nPEF-feature1: CPU decode jpeg 1 frame:   %d ms  (minnan, ypshi)\n\n", (end_time.tv_sec - begin_time.tv_sec)*1000 + 
				(end_time.tv_usec - begin_time.tv_usec)/1000);
#endif
    return MT_SUCCESS;
};

mt_s32 MT_GO_ResetDecoder(mt_handle Decoder)
{
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    
    pDecInstance = (MTGO_DEC_INSTANCE_S*)Decoder;
    /**restet decoder */
    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->ResetDecoder);
    return pDecInstance->ResetDecoder(pDecInstance->Decoder);
}


#ifndef MTGO_CODE_CUT
mt_s32 MT_GO_DecCommInfo(mt_handle Decoder, MTGO_DEC_PRIMARYINFO_S *pPrimaryInfo)
{
    mt_s32 Ret;
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    MTGO_DEC_IMGINFO_S ImgInfo;
    /** check parameter*/
    if (MT_NULL_PTR == pPrimaryInfo)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    pDecInstance = (MTGO_DEC_INSTANCE_S*)Decoder;
    /**decoder common info*/
    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->DecCommInfo);
    Ret = pDecInstance->DecCommInfo(pDecInstance->Decoder, pPrimaryInfo);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(Ret);
        return Ret;
    }

    /**  if width and teigth of screen is 0 ,get the first picture's info*/
    if (( pPrimaryInfo->ScrHeight == 0 )&&(pPrimaryInfo->ScrWidth == 0))
    {
        Ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, 0, &ImgInfo);
        if (MT_SUCCESS != Ret)
        {
            MTGO_ERROR(Ret);
            return Ret;
        }
        pPrimaryInfo->ScrWidth = ImgInfo.Width;
        pPrimaryInfo->ScrHeight = ImgInfo.Height;
    }
    return MT_SUCCESS;
}

mt_s32 MT_GO_DecSetRawDataFlag(mt_handle Decoder, mt_u32 enable)
{
	    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    	    pDecInstance = (MTGO_DEC_INSTANCE_S*)Decoder;

    	    if(pDecInstance->SetRawData != NULL)
	         (mt_void)pDecInstance->SetRawData(pDecInstance->Decoder, enable);

	   return MT_SUCCESS;
}

mt_s32 MT_GO_DecImgInfo(mt_handle Decoder, mt_u32 Index, MTGO_DEC_IMGINFO_S *pImgInfo)
{
    mt_s32 Ret;
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo;

    /**check parameter*/
    if (MT_NULL_PTR == pImgInfo)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    pDecInstance = (MTGO_DEC_INSTANCE_S*)Decoder;
    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->DecCommInfo);
    Ret = pDecInstance->DecCommInfo(pDecInstance->Decoder, &PrimaryInfo);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(Ret);
        return Ret;
    }

    if (Index >= PrimaryInfo.Count)
    {
        MTGO_ERROR(MTGO_ERR_INVINDEX);
        return MTGO_ERR_INVINDEX;
    }


    /**decoder index image info*/
    MTGO_ASSERT(MT_NULL_PTR != pDecInstance->DecImgInfo);
    Ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, Index, pImgInfo);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(Ret);
        return Ret;
    }

    if (pDecInstance->ImgType == MTGO_DEC_IMGTYPE_GIF)
    {
        MTGO_ASSERT(MTGO_PF_8888 == pImgInfo->Format); /**< GIF type  output format can only be MTGO_PF_8888 */
    }
    else if (pDecInstance->ImgType == MTGO_DEC_IMGTYPE_JPEG)
    {
        //MTGO_ASSERT((MTGO_PF_YUV420 == pImgInfo->Format) || (MTGO_PF_YUV422 == pImgInfo->Format));/**< GIF格式只能输出MTGO_PF_YUV420和MTGO_PF_YUV422格式图片 */
    }

    // TODO:BMP PNG how to confirm BMP format

    return MT_SUCCESS;
}

static mt_s32 MTGO_DecImgDataWithScale(const MTGO_DEC_INSTANCE_S *pDecInstance, mt_u32 Index, MTGO_HANDLE InSurface, 
                                            const MTGO_DEC_IMGATTR_S *pImgAttr,  MTGO_HANDLE *pOutSurface, MTGO_DEC_IMGTYPE_E ImgType)
{
    MTGO_HANDLE TmpSurface = 0, DstSurface = 0, rotate_TmpSurface = 0;
    MTGO_SURINFO_S SurInfo = {0};
    MT_RECT SrcRect = {0};
    MTGO_DEC_IMGINFO_S ImgInfo;
    MTGO_BLTOPT2_S BlitOpt;

    mt_s32 ret;
    MT_RECT BSrcRect = {0}, BDstRect = {0};

    MTGO_MemSet(&BlitOpt, 0, sizeof(MTGO_BLTOPT2_S));
    /** allocate midddle layer buffer*/        
    /** calculate middle layer buffer size use target buffer*/
    if(pDecInstance->rotate != MTGO_DEC_ROTATE_90 && pDecInstance->rotate != MTGO_DEC_ROTATE_270){
        SrcRect.w = (mt_s32)pImgAttr->Width;
        SrcRect.h = (mt_s32)pImgAttr->Height;
    }else{
        SrcRect.w = (mt_s32)pImgAttr->Height;
        SrcRect.h = (mt_s32)pImgAttr->Width;
    }
        
    MTGO_MemSet(&SurInfo, 0 , sizeof(MTGO_SURINFO_S));
    (mt_void)pDecInstance->GetActualSize(pDecInstance->Decoder, (mt_s32)Index, &SrcRect, &SurInfo);
    ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_DEC, &TmpSurface, 0);
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    //BM_TRACE("++++++++start to DecImgData\n"); 
    /** start decoding */
    ret = pDecInstance->DecImgData(pDecInstance->Decoder, Index, (MTGO_SURFACE_S*)TmpSurface);
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        goto  err;
    } 
    //BM_TRACE("++++++++stop to DecImgData\n");

    /** start scaling */
	if(pDecInstance->GetCropRect)
		(mt_void)pDecInstance->GetCropRect(pDecInstance->Decoder, &BSrcRect);
	else	
	    (mt_void)Surface_GetSurfaceSize(TmpSurface, &(BSrcRect.w), &(BSrcRect.h));

    if(pDecInstance->rotate == MTGO_DEC_ROTATE_NONE){
        if(pImgAttr->Width != SurInfo.Width ||  pImgAttr->Height != SurInfo.Height || INVALID_HANDLE != InSurface){
            /** allocate target buffer*/
            if (INVALID_HANDLE != InSurface)
            {
                /** set target buffer */
                DstSurface = InSurface;
            }
            else
            {
                /** MtGo allocate target buffer*/
                MTGO_MemSet(&SurInfo, 0 , sizeof(MTGO_SURINFO_S));
                SurInfo.PixelFormat = pImgAttr->Format;
                SurInfo.Width = pImgAttr->Width;
                SurInfo.Height = pImgAttr->Height;
                ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_DEC, &DstSurface, 0);
                if (ret != MT_SUCCESS)
                {
                    MTGO_ERROR(ret);
                    goto err;
                }
            }

            (mt_void)Surface_GetSurfaceSize(DstSurface, &(BDstRect.w), &(BDstRect.h));
            ret = Bliter_StretchBlit(TmpSurface, &BSrcRect, DstSurface, &BDstRect, &BlitOpt);
            if (ret != MT_SUCCESS)
            {
                MTGO_ERROR(ret);
                goto  err;
            }

            MTGO_FreeSurface(TmpSurface);
        }
        else {
            DstSurface = TmpSurface;
        }
        TmpSurface = 0;
    }else{
        if(pDecInstance->rotate != MTGO_DEC_ROTATE_90 && pDecInstance->rotate != MTGO_DEC_ROTATE_270){
            BDstRect.w = pImgAttr->Width;
            BDstRect.h = pImgAttr->Height;
        }else{
            BDstRect.w = pImgAttr->Height;
            BDstRect.h = pImgAttr->Width;
        }
        if(pImgAttr->Width != SurInfo.Width ||  pImgAttr->Height != SurInfo.Height){
            /** MtGo allocate target buffer*/
            MTGO_MemSet(&SurInfo, 0 , sizeof(MTGO_SURINFO_S));
            SurInfo.Width =  (mt_s32)BDstRect.w;
            SurInfo.Height = (mt_s32)BDstRect.h;
            SurInfo.PixelFormat = pImgAttr->Format;
            
            ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_DEC, &rotate_TmpSurface, 0);
            if (ret != MT_SUCCESS)
            {
                MTGO_ERROR(ret);
                goto err;
            }
            (mt_void)Surface_GetSurfaceSize(rotate_TmpSurface, &(BDstRect.w), &(BDstRect.h));
            //printf("1BSrcRect.w %d, BSrcRect.h %d, BDstRect.w %d, BDstRect.h %d\n", BSrcRect.w, BSrcRect.h, BDstRect.w, BDstRect.h);
            ret = Bliter_StretchBlit(TmpSurface, &BSrcRect, rotate_TmpSurface, &BDstRect, &BlitOpt);
            if (ret != MT_SUCCESS)
            {
                MTGO_ERROR(ret);
                goto  err;
            }
            MTGO_FreeSurface(TmpSurface);//free it immediately to reduce the usage of memory
        }else {
            rotate_TmpSurface = TmpSurface;
        }
        TmpSurface = 0;
        
        /** allocate target buffer*/
        if (INVALID_HANDLE != InSurface)
        {
            /** set target buffer */
            DstSurface = InSurface;
        }
        else
        {
            /** MtGo allocate target buffer*/
            MTGO_MemSet(&SurInfo, 0 , sizeof(MTGO_SURINFO_S));
            SurInfo.PixelFormat = pImgAttr->Format;
            SurInfo.Width = pImgAttr->Width;
            SurInfo.Height = pImgAttr->Height;
            ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_DEC, &DstSurface, 0);
            if (ret != MT_SUCCESS)
            {
                MTGO_ERROR(ret);
                goto err;
            }
        }

        (mt_void)Surface_GetSurfaceSize(rotate_TmpSurface, &(BSrcRect.w), &(BSrcRect.h));
        (mt_void)Surface_GetSurfaceSize(DstSurface, &(BDstRect.w), &(BDstRect.h));
        //printf("2BSrcRect.w %d, BSrcRect.h %d, BDstRect.w %d, BDstRect.h %d\n", BSrcRect.w, BSrcRect.h, BDstRect.w, BDstRect.h);
        ret = Bliter_RotateMirror (rotate_TmpSurface, &BSrcRect, DstSurface, &BDstRect, pDecInstance->rotate, 0);
        if (ret != MT_SUCCESS)
        {
            MTGO_ERROR(ret);
            goto err;
        }
        MTGO_FreeSurface(rotate_TmpSurface);
        rotate_TmpSurface = 0;
    }

    /** get index decode image info*/
    ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto  err;        
    }

    /** set alpha value  */
    (mt_void)Surface_SetSurfaceAlpha(DstSurface, ImgInfo.Alpha);

    /** set colorkey attribute */
    if (MT_TRUE == ImgInfo.IsHaveKey)
    {
        (mt_void)Surface_SetSurfaceColorKey(DstSurface, ImgInfo.Key);
    }

    /** set Surface color palette  */


    MTGO_ASSERT(!IS_CLUT_FORMAT(pImgAttr->Format));

    /**set output */
    if (INVALID_HANDLE == InSurface)
    {
        *pOutSurface =  DstSurface;   
    }

    return MT_SUCCESS;
    
err:
    if (INVALID_HANDLE == InSurface)
    {
        if(DstSurface)
            MTGO_FreeSurface(DstSurface);
        DstSurface = 0;
    }

    if(rotate_TmpSurface)
        MTGO_FreeSurface(rotate_TmpSurface);
    rotate_TmpSurface = 0;
    if(TmpSurface)
        MTGO_FreeSurface(TmpSurface);
    TmpSurface = 0;

    return ret;
}
#endif


static mt_s32 MTGO_DecImgData(const MTGO_DEC_INSTANCE_S *pDecInstance, mt_u32 Index, MTGO_HANDLE InSurface, 
                               const MTGO_DEC_IMGATTR_S *pImgAttr,  MTGO_HANDLE *pOutSurface, MTGO_DEC_IMGTYPE_E ImgType)
{
    MTGO_HANDLE DstSurface = 0;
    MTGO_SURINFO_S SurInfo;
    MTGO_DEC_IMGINFO_S ImgInfo;
		MT_RECT SrcRect = {0};
    mt_s32 ret;

    if (INVALID_HANDLE != InSurface)
    {
        DstSurface = InSurface;
    }
    else
    {
        MTGO_MemSet(&SurInfo, 0 , sizeof(MTGO_SURINFO_S));
#if 0				
        SurInfo.Width =  (mt_s32)pImgAttr->Width;
        SurInfo.Height = (mt_s32)pImgAttr->Height;
        SurInfo.PixelFormat = pImgAttr->Format;
				printf("\r\n w:%d, h:%d, pix:%d", SurInfo.Width, SurInfo.Height, SurInfo.PixelFormat);
#endif			
				SrcRect.w = (mt_s32)pImgAttr->Width;
				SrcRect.h = (mt_s32)pImgAttr->Height;
				(mt_void)pDecInstance->GetActualSize(pDecInstance->Decoder, (mt_s32)Index, &SrcRect, &SurInfo);
//jpg case1192 yuv420sp output video layer need disable format convert
//png/jpg 功耗测试， gra时候会关闭， 故需避免做MTGO_DecImgDataWithScale操作
/*
if(SurInfo.Width != SrcRect.w || SurInfo.Height != SrcRect.h)
        {
          MTGO_DEC_IMGATTR_S ImgAttr = {0};
          ImgAttr.Format = MTGO_PF_8888;
          ImgAttr.Width = pImgAttr->Width;
          ImgAttr.Height = pImgAttr->Height;
          return MTGO_DecImgDataWithScale(pDecInstance, Index, InSurface, 
                                            &ImgAttr,  pOutSurface, ImgType);
        }
*/
        if(SurInfo.Width != SrcRect.w || SurInfo.Height != SrcRect.h)
        {
          MTGO_DEC_IMGATTR_S ImgAttr = {0};
          ImgAttr.Format = MTGO_PF_8888;
          ImgAttr.Width = pImgAttr->Width;
          ImgAttr.Height = pImgAttr->Height;
          return MTGO_DecImgDataWithScale(pDecInstance, Index, InSurface, 
                                            &ImgAttr,  pOutSurface, ImgType);
        }
        ret = MTGO_CreateSurface(&SurInfo, MTGO_MOD_DEC, &DstSurface, 0);
        if (ret != MT_SUCCESS)
        {
            MTGO_ERROR(ret);
            return ret;
        }            
    }

    ret = pDecInstance->DecImgData(pDecInstance->Decoder, Index, (MTGO_SURFACE_S *)DstSurface);
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        goto err0;
    } 
 
    /** get index decoding image info*/
    ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, Index, &ImgInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        goto err0;        
    }

    /** set alpha value  */
    (mt_void)Surface_SetSurfaceAlpha(DstSurface, ImgInfo.Alpha);

    /** set colorkey attribute */
    if (MT_TRUE == ImgInfo.IsHaveKey)
    {
        (mt_void)Surface_SetSurfaceColorKey(DstSurface, ImgInfo.Key);
    }

    if (INVALID_HANDLE == InSurface)
    {
        *pOutSurface = DstSurface;
    }
    return MT_SUCCESS;
err0:

    if (INVALID_HANDLE == InSurface)
    {
        MTGO_FreeSurface(DstSurface);
    }
    return ret;
}

mt_s32 MT_GO_SetImgRotate(mt_handle Decoder, MTGO_DEC_ROTATE_E rotate)
{
    mt_s32 ret;
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;

	pDecInstance = (MTGO_DEC_INSTANCE_S *)Decoder;
	if(pDecInstance->SetRotate == NULL){
        pDecInstance->rotate = rotate;
		return MT_SUCCESS;
    }else{
		ret = pDecInstance->SetRotate(pDecInstance->Decoder, rotate);
	}
	return ret;
}

mt_s32 MT_GO_DecImgData(mt_handle Decoder, mt_u32 Index, const MTGO_DEC_IMGATTR_S *pImgAttr, mt_handle *pSurface)
{
    mt_s32 ret;
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    MTGO_DEC_IMGINFO_S DecInfo;
    MTGO_HANDLE pOutSurface;
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo = {0};
    MTGO_DEC_IMGATTR_S ExpImgAttr;
    
    /**check parameter */
    if (MT_NULL_PTR == pSurface)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    /** check param*/
    pDecInstance = (MTGO_DEC_INSTANCE_S *)Decoder;
    ret = pDecInstance->DecCommInfo(pDecInstance->Decoder, &PrimaryInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    if (Index >= PrimaryInfo.Count)
    {
        MTGO_ERROR(MTGO_ERR_INVINDEX);
        return MTGO_ERR_INVINDEX;
    }
    
    ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, Index, &DecInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    if (pImgAttr == NULL)
    {
        ExpImgAttr.Width = DecInfo.Width;
        ExpImgAttr.Height = DecInfo.Height;
        ExpImgAttr.Format = DecInfo.Format;
        
    }
    else
    {
        MTGO_MemCopy(&ExpImgAttr, pImgAttr, sizeof(MTGO_DEC_IMGATTR_S) );
    }

    if((pDecInstance->ImgType == MTGO_DEC_IMGTYPE_JPEG) 
        && ((pDecInstance->rotate  == MTGO_DEC_ROTATE_90) 
        || (pDecInstance->rotate  == MTGO_DEC_ROTATE_270)))
    {  

        if((ExpImgAttr.Width > 2048) ||  (ExpImgAttr.Height > 2048))
        {
             ExpImgAttr.Width =1080;
             ExpImgAttr.Height  = 1920;
             ExpImgAttr.Format = MTGO_PF_8888;
        }
    }
    else if((pDecInstance->ImgType == MTGO_DEC_IMGTYPE_JPEG) 
        || (pDecInstance->ImgType == MTGO_DEC_IMGTYPE_BMP))
    {
         if((ExpImgAttr.Width > 2048) ||  (ExpImgAttr.Height > 2048))
        {
             ExpImgAttr.Width = 1920;
             ExpImgAttr.Height  = 1080;
             ExpImgAttr.Format = MTGO_PF_8888;
        }
    }
    if((ExpImgAttr.Width == 0) || (ExpImgAttr.Height == 0))
    {
        ret = MT_FAILURE;
        MTGO_ERROR(ret);
        return ret;
    }
    //printf("DecInfo.Width %d, DecInfo.Height %d, ExpImgAttr.Width %d, ExpImgAttr.Height %d\n", 
    //    DecInfo.Width, DecInfo.Height, ExpImgAttr.Width, ExpImgAttr.Height);
    /** decode imgdata */
    if ((DecInfo.Width != ExpImgAttr.Width) || (DecInfo.Height != ExpImgAttr.Height) || (DecInfo.Format != ExpImgAttr.Format)
        || (pDecInstance->rotate != MTGO_DEC_ROTATE_NONE))
    {
        if (pImgAttr == NULL){
            ExpImgAttr.Format = MTGO_PF_8888;
        }
        ret = MTGO_DecImgDataWithScale(pDecInstance, Index, (MTGO_HANDLE)NULL, &ExpImgAttr, &pOutSurface, PrimaryInfo.ImgType);
  //      return MT_FAILURE;
    }
    else
    {
        ret = MTGO_DecImgData(pDecInstance, Index, (MTGO_HANDLE)NULL, &ExpImgAttr , &pOutSurface, PrimaryInfo.ImgType);
    }
    
    if (ret != MT_SUCCESS)
    {
        MTGO_ERROR(ret);
        return ret;
    }
    /** capture the surface*/
    *pSurface = (mt_handle)pOutSurface;
    return MT_SUCCESS;

}


#ifndef MTGO_CODE_CUT
mt_s32 MT_GO_DecImgToSurface(mt_handle Decoder, mt_u32 Index, mt_handle Surface)
{
    mt_s32 ret;
    MTGO_HANDLE pDecoder, pSurface;
    mt_s32 u32DstWidth, u32DstHeight;
    MTGO_PF_E DstPF;
    mt_u32 Bpp;
    MTGO_DEC_INSTANCE_S *pDecInstance = MT_NULL_PTR;
    MTGO_DEC_IMGINFO_S DecInfo;
    MTGO_DEC_IMGATTR_S ImgAttr = {0};
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo= {0};

    pDecoder = Decoder;
    pSurface = Surface;
    pDecInstance = (MTGO_DEC_INSTANCE_S *)pDecoder;
    ret = pDecInstance->DecCommInfo(pDecInstance->Decoder, &PrimaryInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }

    if (Index >= PrimaryInfo.Count)
    {
        MTGO_ERROR(MTGO_ERR_INVINDEX);
        return MTGO_ERR_INVINDEX;
    }
    
    (mt_void)Surface_GetSurfaceSize(pSurface, &u32DstWidth, &u32DstHeight);
    (mt_void)Surface_GetSurfacePixelFormat(pSurface, &DstPF, &Bpp);
    ret = pDecInstance->DecImgInfo(pDecInstance->Decoder, Index, &DecInfo);
    if (MT_SUCCESS != ret)
    {
        MTGO_ERROR(ret);
        return ret;
    }
    ImgAttr.Format = DstPF;
    ImgAttr.Width = (mt_u32)u32DstWidth;
    ImgAttr.Height = (mt_u32)u32DstHeight;
   // BM_TRACE("+++++ DecInfo.Width %d u32DstWidth %d DecInfo.Height %d u32DstHeight %d DecInfo.Format %d  DstPF %d \n",DecInfo.Width , u32DstWidth , DecInfo.Height , u32DstHeight , DecInfo.Format , DstPF);
   
    if ((DecInfo.Width != (mt_u32)u32DstWidth) || (DecInfo.Height != (mt_u32)u32DstHeight) || (DecInfo.Format != DstPF))
    {   
        ret = MTGO_DecImgDataWithScale(pDecInstance, Index, pSurface, &ImgAttr, NULL, PrimaryInfo.ImgType);
    }
    else
    {
        ret = MTGO_DecImgData(pDecInstance, Index, pSurface, &ImgAttr , NULL, PrimaryInfo.ImgType);
    }

    return ret;
}
#endif
//lint +e605

mt_s32 MT_GO_DecodeFile(const mt_char* pszFile, mt_handle* pSurface)
{
	return MT_FAILURE;
}

