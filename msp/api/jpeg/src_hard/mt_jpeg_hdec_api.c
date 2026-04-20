/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mt_type.h"
#include "mt_jpeg_config.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_adp.h"
#include "mt_jpeg_api.h"
#include "mt_module_debug.h"

/***************************** Macro Definition     ***************************/

/***************************** Structure Definition ***************************/

/********************** Global Variable declaration **************************/

/********************** API forward declarations    **************************/

/**********************       API realization       **************************/

/**
 \brief Sets dec output message. CNcomment:设置解码输出的信息上下文 CNend
 \attention \n
MT_JPEG_SetOutDesc should have called create jpeg decoder.set the output address \n
and output stride,set whether crop, set crop rect \n
CNcomment:必须在创建解码器之后，启动解码之前调用该接口，主要设置解码输出地址和输出 \n
          行间距，设置是否裁剪以及对应的裁剪区域 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	*pstSurfaceDesc. CNcomment:解码输出描述信息 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetOutDesc
 */

MT_S32  MT_JPEG_SetOutDesc(const struct jpeg_decompress_struct *cinfo,
                           const MT_JPEG_SURFACE_DESCRIPTION_S *pstSurfaceDesc)
{
	MT_S32 s32Cnt = 0;
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

	if (NULL == pstSurfaceDesc)
	{
		return MT_FAILURE;
	}

	for (s32Cnt = 0; s32Cnt < MAX_PIXEL_COMPONENT_NUM; s32Cnt++)
	{
		pJpegHandle->stOutDesc.stOutSurface.pOutPhy[s32Cnt] = pstSurfaceDesc->stOutSurface.pOutPhy[s32Cnt];
		pJpegHandle->stOutDesc.stOutSurface.pOutVir[s32Cnt] = pstSurfaceDesc->stOutSurface.pOutVir[s32Cnt];
		pJpegHandle->stOutDesc.stOutSurface.u32OutStride[s32Cnt] = pstSurfaceDesc->stOutSurface.u32OutStride[s32Cnt];
	}

	pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem = pstSurfaceDesc->stOutSurface.bUserPhyMem;

	if (MT_TRUE == pstSurfaceDesc->bCrop)
	{
		if( (pstSurfaceDesc->stCropRect.w <= 0)||(pstSurfaceDesc->stCropRect.h <= 0))
		{
			return MT_FAILURE;
		}
		pJpegHandle->stOutDesc.stCropRect.x = pstSurfaceDesc->stCropRect.x;
		pJpegHandle->stOutDesc.stCropRect.y = pstSurfaceDesc->stCropRect.y;
		pJpegHandle->stOutDesc.stCropRect.w = pstSurfaceDesc->stCropRect.w;
		pJpegHandle->stOutDesc.stCropRect.h = pstSurfaceDesc->stCropRect.h;
	}

	pJpegHandle->stOutDesc.bCrop = pstSurfaceDesc->bCrop;
	return MT_SUCCESS;
	
}

/**
 \brief Get Jpeg information. CNcomment:获取jpeg图片信息 CNend
 \attention \n
if you want to get input format and input width and input height,you should set bOutInfo false.\n
others you can get the information as follows: output rgb widht/height/stride/size or output \n
yuvsp lu width/height/stride/size and ch width/height/stride/size.\n
you call this function should after read header and set the ouput parameter.\n
CNcomment:当bOutInfo设置成FALSE的时候，可以获取到图片输出的宽度和高度以及像素格式，当设置成TRUE的 \n
          时候则可以获取到如下信息，要是解码RGB则获取到宽度/高度/行间距/大小,要是解码输出yuvsp，\n
          则可以获取的亮度和色度的宽度/高度/行间距/大小的信息。 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[out] pJpegInfo.	CNcomment:解码jpeg的相关信息  CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_GetJpegInfo
 */
MT_S32  MT_JPEG_GetJpegInfo(j_decompress_ptr cinfo, MT_JPEG_INFO_S *pJpegInfo)
{
		JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		if (NULL == pJpegInfo)
		{
			return MT_FAILURE;
		}

		if (MT_FALSE == pJpegInfo->bOutInfo)
		{
			memset(pJpegInfo, 0, sizeof(MT_JPEG_INFO_S));
			pJpegInfo->u32Width[0]  = cinfo->image_width;
			pJpegInfo->u32Height[0] = cinfo->image_height;
			pJpegInfo->enFmt = pJpegHandle->enImageFmt;
			return MT_SUCCESS;
		}

		JPEG_HDEC_GetImagInfo(cinfo);

		/**
		** output message,the output stride should 16byte align by tde request
		** CNcomment: 输出信息 CNend\n
		**/
		switch (cinfo->out_color_space)
		{
			case JCS_YUV400_SP:
			case JCS_YUV444_SP:
			case JCS_YUV420_SP:
			case JCS_YUV422_SP_12:
			case JCS_YUV422_SP_21:
			case JCS_GRAYSCALE:
				pJpegInfo->u32Width[0]  = pJpegHandle->stJpegSofInfo.u32YOutWidth;
				pJpegInfo->u32Width[1]  = pJpegHandle->stJpegSofInfo.u32COutWidth;
				pJpegInfo->u32Height[0] = pJpegHandle->stJpegSofInfo.u32YOutHeight;
				pJpegInfo->u32Height[1] = pJpegHandle->stJpegSofInfo.u32COutHeight;
				pJpegInfo->u32OutStride[0] = pJpegHandle->stJpegSofInfo.u32YOutStride;
				pJpegInfo->u32OutStride[1] = pJpegHandle->stJpegSofInfo.u32COutStride;
				pJpegInfo->u32OutSize[0] = pJpegHandle->stJpegSofInfo.u32YSize;
				pJpegInfo->u32OutSize[1] = pJpegHandle->stJpegSofInfo.u32CSize;
				break;
            case JCS_CMYK:
				pJpegInfo->u32Width[0]  = pJpegHandle->stJpegSofInfo.u32YOutWidth;
				pJpegInfo->u32Width[1]  = pJpegHandle->stJpegSofInfo.u32COutWidth;
				pJpegInfo->u32Height[0] = pJpegHandle->stJpegSofInfo.u32YOutHeight;
				pJpegInfo->u32Height[1] = pJpegHandle->stJpegSofInfo.u32COutHeight;
				pJpegInfo->u32OutStride[0] = pJpegHandle->stJpegSofInfo.u32YOutStride;
				pJpegInfo->u32OutStride[1] = pJpegHandle->stJpegSofInfo.u32COutStride;
				pJpegInfo->u32OutSize[0] = pJpegHandle->stJpegSofInfo.u32YSize;
				pJpegInfo->u32OutSize[1] = pJpegHandle->stJpegSofInfo.u32CSize;                            
                        break;
			case JCS_ARGB_8888:
			case JCS_ABGR_8888:			
			#ifdef CONFIG_JPEG_ADD_GOOGLEFUNCTION
			case JCS_RGBA_8888:
			#endif
			case JCS_RGB:
			case JCS_BGR:
			case JCS_RGB_565:
			case JCS_BGR_565:
			case JCS_ARGB_1555:
			case JCS_ABGR_1555:
			case JCS_CrCbY:
			case JCS_YCbCr:
//				pJpegInfo->u32Width[0]  = cinfo->output_width;
//				pJpegInfo->u32Height[0] = cinfo->output_height;
				pJpegInfo->u32Width[0]  = pJpegHandle->stJpegSofInfo.u32YOutWidth;
				pJpegInfo->u32Height[0] = pJpegHandle->stJpegSofInfo.u32YOutHeight;
				pJpegInfo->u32OutStride[0] = pJpegHandle->stJpegSofInfo.u32DisplayStride;
				pJpegInfo->u32OutSize[0] = pJpegInfo->u32OutStride[0] * pJpegInfo->u32Height[0];
				break;
			default:
				break;
		}


		return MT_SUCCESS;
	
}

#if 0
MT_S32  MT_JPEG_GetIdctBufSize(j_decompress_ptr cinfo, mt_u32 *p_idct_buf_size)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if(pJpegHandle->bSoftHuffDec)
    {
        p_idct_buf_size[0] = pJpegHandle->swHuff.idct_buf_size[0];
        if(p_idct_buf_size[0] == 0)
            return MT_FAILURE;
        p_idct_buf_size[1] = pJpegHandle->swHuff.idct_buf_size[1];
        p_idct_buf_size[2] = pJpegHandle->swHuff.idct_buf_size[2];
        p_idct_buf_size[3] = pJpegHandle->swHuff.idct_buf_size[3];
        return MT_SUCCESS;
    }
    else
        return MT_FAILURE;	
}

MT_S32  MT_JPEG_SetIdctBuf(j_decompress_ptr cinfo, mt_u32 *p_idct_phy, mt_u32 *p_idct_vir)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if(pJpegHandle->bSoftHuffDec)
    {
        pJpegHandle->swHuff.idct_buf_addr_phy[0] = p_idct_phy[0];
        if(p_idct_phy[0] == 0)
            return MT_FAILURE;
        pJpegHandle->swHuff.idct_buf_addr_phy[1] = p_idct_phy[1];
        pJpegHandle->swHuff.idct_buf_addr_phy[2] = p_idct_phy[2];
        pJpegHandle->swHuff.idct_buf_addr_phy[3] = p_idct_phy[3];
        
        pJpegHandle->swHuff.idct_buf_addr_vir[0] = p_idct_vir[0];
        if(p_idct_vir[0] == 0)
            return MT_FAILURE;
        pJpegHandle->swHuff.idct_buf_addr_vir[1] = p_idct_vir[1];
        pJpegHandle->swHuff.idct_buf_addr_vir[2] = p_idct_vir[2];
        pJpegHandle->swHuff.idct_buf_addr_vir[3] = p_idct_vir[3];
        return MT_SUCCESS;
    }
    else
        return MT_FAILURE;	
}
#endif
/**
 \brief set jpeg dec inflexion. CNcomment:在硬件解码支持的情况下，设置软解和硬解的拐点 CNend
 \attention \n
MT_JPEG_SetInflexion should have called jpeg_create_decompress.if no call this \n
function,use the default flexion \n
CNcomment:必须在创建完解码器之后调用该函数，要是没有设置拐点，使用默认的拐点大小 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	u32flexionSize. CNcomment:要设置的解码拐点大小 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetInflexion
 */
MT_S32 MT_JPEG_SetInflexion(const struct jpeg_decompress_struct *cinfo, const MT_U32 u32flexionSize)
{
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

	pJpegHandle->u32Inflexion = u32flexionSize;

	return MT_SUCCESS;
}

/**
 \brief get jpeg dec inflexion. CNcomment:获取软件和硬件解码的拐点 CNend
 \attention \n
MT_JPEG_GetInflexion should have called jpeg_create_decompress.\n
CNcomment:在调用MT_JPEG_GetInflexion之前必须已经创建好了解码器 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[out] pu32flexionSize. CNcomment:解码拐点大小 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetInflexion
 */
MT_S32 MT_JPEG_GetInflexion(const struct jpeg_decompress_struct *cinfo, MT_U32 *pu32flexionSize)
{
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

	if (NULL == pu32flexionSize)
	{
		return MT_FAILURE;
	}

	*pu32flexionSize = pJpegHandle->u32Inflexion;

	return MT_SUCCESS;
}

/**
 \brief set jpeg dec coef when output argb. CNcomment:在解码输出ARGB的情况下设置相关系数 CNend
 \attention \n
MT_JPEG_SetDecCoef should have called jpeg_create_decompress.set whether horizontal \n
and vertical fliter,whether set horizontal and ver sample, whether set csc coefficient, \n
and set there coefficient.if no call this function, use the default parameter. \n
CNcomment:必须在创建完解码器之后调用该函数，主要设置是否垂直和水平滤波，是否设置垂直和水平 \n
          采样系数，是否设置CSS系数，并设置相对应的系数，要是没有调用该函数，使用默认值 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	*pstDecCoef. CNcomment:解码系数 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetDecCoef
 */
MT_S32 MT_JPEG_SetDecCoef(const struct jpeg_decompress_struct *cinfo, const MT_JPEG_DEC_COEF_S *pstDecCoef)
{
#ifdef CONFIG_JPEG_HARDDEC2ARGB
	MT_S32 s32Cnt1 = 0;
	MT_S32 s32Cnt2 = 0;
	
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if (NULL == pstDecCoef)
    {
        return MT_FAILURE;
    }

    pJpegHandle->stDecCoef.bEnHorMedian = pstDecCoef->bEnHorMedian;
    pJpegHandle->stDecCoef.bEnVerMedian = pstDecCoef->bEnVerMedian;
    pJpegHandle->stDecCoef.bSetHorSampleCoef = pstDecCoef->bSetHorSampleCoef;
    pJpegHandle->stDecCoef.bSetVerSampleCoef = pstDecCoef->bSetVerSampleCoef;
    pJpegHandle->stDecCoef.bSetCSCCoef = pstDecCoef->bSetCSCCoef;

	for(s32Cnt1 = 0; s32Cnt1 < MAX_HORCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_HORCOEF_COL; s32Cnt2++)
		{
           pJpegHandle->stDecCoef.s16HorCoef[s32Cnt1][s32Cnt2] = pstDecCoef->s16HorCoef[s32Cnt1][s32Cnt2];
		}
	}
	for(s32Cnt1 = 0; s32Cnt1 < MAX_VERCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_VERCOEF_COL; s32Cnt2++)
		{
           pJpegHandle->stDecCoef.s16VerCoef[s32Cnt1][s32Cnt2] = pstDecCoef->s16VerCoef[s32Cnt1][s32Cnt2];
		}
	}
	for(s32Cnt1 = 0; s32Cnt1 < MAX_CSCCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_CSCCOEF_COL; s32Cnt2++)
		{
           pJpegHandle->stDecCoef.s16CSCCoef[s32Cnt1][s32Cnt2] = pstDecCoef->s16CSCCoef[s32Cnt1][s32Cnt2];
		}
	}
	
    return MT_SUCCESS;

#else
    return MT_FAILURE;
#endif

}

/**
 \brief get jpeg dec coef when output argb. CNcomment:在解码输出ARGB的情况下获取设置的相关系数 CNend
 \attention \n
MT_JPEG_GetDecCoef should have called MT_JPEG_SetDecCoef.\n
CNcomment:在调用MT_JPEG_GetDecCoef之前必须已经MT_JPEG_SetDecCoef CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[out]	pstOutDecCoef. CNcomment:输出解码系数 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_GetDecCoef
 */
#ifdef CONFIG_JPEG_HARDDEC2ARGB
MT_S32 MT_JPEG_GetDecCoef(const struct jpeg_decompress_struct *cinfo, MT_JPEG_DEC_COEF_S *pstOutDecCoef)
{
	MT_S32 s32Cnt1 = 0;
	MT_S32 s32Cnt2 = 0;
	
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if (NULL == pstOutDecCoef)
    {
        return MT_FAILURE;
    }

    pstOutDecCoef->bEnHorMedian = pJpegHandle->stDecCoef.bEnHorMedian;
    pstOutDecCoef->bEnVerMedian = pJpegHandle->stDecCoef.bEnVerMedian;
    pstOutDecCoef->bSetHorSampleCoef = pJpegHandle->stDecCoef.bSetHorSampleCoef;
    pstOutDecCoef->bSetVerSampleCoef = pJpegHandle->stDecCoef.bSetVerSampleCoef;
    pstOutDecCoef->bSetCSCCoef = pJpegHandle->stDecCoef.bSetCSCCoef;

	for(s32Cnt1 = 0; s32Cnt1 < MAX_HORCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_HORCOEF_COL; s32Cnt2++)
		{
		   pstOutDecCoef->s16HorCoef[s32Cnt1][s32Cnt2] = pJpegHandle->stDecCoef.s16HorCoef[s32Cnt1][s32Cnt2];
		}
	}
	for(s32Cnt1 = 0; s32Cnt1 < MAX_VERCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_VERCOEF_COL; s32Cnt2++)
		{
		   pstOutDecCoef->s16VerCoef[s32Cnt1][s32Cnt2] = pJpegHandle->stDecCoef.s16VerCoef[s32Cnt1][s32Cnt2];
		}
	}
	for(s32Cnt1 = 0; s32Cnt1 < MAX_CSCCOEF_ROW; s32Cnt1++)
	{
		for(s32Cnt2 = 0; s32Cnt2 < MAX_CSCCOEF_COL; s32Cnt2++)
		{
		   pstOutDecCoef->s16CSCCoef[s32Cnt1][s32Cnt2] = pJpegHandle->stDecCoef.s16CSCCoef[s32Cnt1][s32Cnt2];
		}
	}
    return MT_SUCCESS;

}
#else
MT_S32 MT_JPEG_GetDecCoef(const struct jpeg_decompress_struct *cinfo, MT_JPEG_DEC_COEF_S *pstOutDecCoef)
{
	if (NULL != pstOutDecCoef)
	{
		memset(pstOutDecCoef,0,sizeof(MT_JPEG_DEC_COEF_S));
	}
	return MT_FAILURE;
}
#endif

/**
 \brief set alpha value. CNcomment:设置alpha的值 CNend
 \attention \n
MT_JPEG_SetAlpha should have called jpeg_create_decompress.when decode output \n
argb8888 and argb8888,we can call this function,if no call it,use the default value. \n
CNcomment:必须在创建完解码器之后调用该函数，当解码输出为ARGB8888和ABGR8888的时候可以 \n
调用该函数，要是没有调用该函数，就使用默认的值 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	s32Alpha. CNcomment:设置alpha值 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetAlpha
 */
MT_S32 MT_JPEG_SetAlpha(const struct jpeg_decompress_struct *cinfo, const MT_U32 u32Alpha)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    pJpegHandle->u32Alpha = u32Alpha;

    return MT_SUCCESS;
}

/**
 \brief set stream from flag of use phy mem	or virtual mem. CNcomment:设置码流连续还是虚拟内存信息 CNend
 \attention \n
if want to use this function,should call between create decompress and
jpeg_stdio_src or jpeg_mem_src.if not call this we should check\n
CNcomment:如果要调用，必须在创建完解码器关联码流之前调用，如果没有调用该接口也有可能是连续的内存 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	pStreamPhyAddr. CNcomment:码流物理地址 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetStreamPhyMem
 */
MT_S32 MT_JPEG_SetStreamPhyMem(const struct jpeg_decompress_struct *cinfo, phys_addr_t pStreamPhyAddr)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    if (0 == pStreamPhyAddr)
    {
        return MT_FAILURE;
    }

    pJpegHandle->stHDecDataBuf.pDataPhyBuf = pStreamPhyAddr;
    pJpegHandle->stHDecDataBuf.bUserPhyMem = MT_TRUE;

    return MT_SUCCESS;
}

/** 
 \brief set stream mem information. CNcomment:设置码流内存信息 CNend
 \attention \n
 if want to use this function,should call between create decompress and 
 jpeg_stdio_src or jpeg_mem_src.if not call this we should check\n
 CNcomment:如果要调用，必须在创建完解码器关联码流之前调用 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	pPhyStremBuf.         CNcomment:码流buffer起始物理地址 CNend
 \param[in]	u32StremBufLen.       CNcomment:码流buffer大小         CNend
 \param[in]	pVirSaveReturnBuf.    CNcomment:码流buffer回绕起始地址 CNend
 \param[in]	s32StrmReturnSize.    CNcomment:回绕码流大小           CNend
 \param[in]	bStreamBufNeedReturn. CNcomment:码流buffer是否回绕     CNend

 \retval ::MT_SUCCESS 
 \retval ::MT_FAILURE

 \see \n
 ::MT_JPEG_SetBufInfo
 */
MT_S32 MT_JPEG_SetBufInfo(const struct jpeg_decompress_struct *cinfo,phys_addr_t pPhyStremBuf,MT_U32 u32StremBufLen, MT_CHAR* pVirSaveReturnBuf,MT_S32 s32StrmReturnSize,MT_BOOL bStreamBufNeedReturn)
{

	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
	if(0 == pPhyStremBuf)
	{/** 要处理码流回绕必须码流来源于物理地址 **/
		return MT_FAILURE;
	}

#ifdef CONFIG_JPEG_STREAMBUF_4ALIGN
	if(0 != (MT_S32)pPhyStremBuf % 4)
	{/**不是按照4字节对齐，则不支持 **/
		return MT_FAILURE;
	}
#endif
	pJpegHandle->stHDecDataBuf.pStartBufPhy = pPhyStremBuf;
	pJpegHandle->stHDecDataBuf.s32BufLen    = u32StremBufLen;
    pJpegHandle->stHDecDataBuf.pDataVirBufReturn  = pVirSaveReturnBuf;
	pJpegHandle->stHDecDataBuf.s32StreamReturnLen = s32StrmReturnSize;
	pJpegHandle->stHDecDataBuf.bNeedStreamReturn  = bStreamBufNeedReturn;
	
	return MT_SUCCESS;
	
}


/**
 \brief set if dec output yuv420sp. CNcomment:设置是否统一输出yuv420sp标识 CNend
 \attention \n
MT_JPEG_SetYCbCr420spFlag should have called jpeg_create_decompress.\n
CNcomment:在调用MT_JPEG_SetYCbCr420spFlag之前必须已经创建好了解码器 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	bOutYCbCr420sp. CNcomment:是否统一解码输出yuv420sp格式 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetYCbCr420spFlag
 */
MT_S32 MT_JPEG_SetYCbCr420spFlag(const struct jpeg_decompress_struct *cinfo, const MT_BOOL bOutYCbCr420sp)
{
#ifdef CONFIG_JPEG_OUTPUT_YUV420SP
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    pJpegHandle->bOutYUV420SP = bOutYCbCr420sp;

    return MT_SUCCESS;

#else
    return MT_FAILURE;
#endif

}

/**
 \brief set if output lu pixle sum value. CNcomment:设置是否统计亮度值标识 CNend
 \attention \n
MT_JPEG_SetLuPixSumFlag should have called jpeg_create_decompress.\n
CNcomment:在调用MT_JPEG_SetLuPixSumFlag之前必须已经创建好了解码器 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[in]	bLuPixSum. CNcomment:设置是否统计亮度值标识 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_SetLuPixSumFlag
 */
MT_S32 MT_JPEG_SetLuPixSumFlag(const struct jpeg_decompress_struct *cinfo, const MT_BOOL bLuPixSum)
{
#ifdef CONFIG_JPEG_OUTPUT_LUPIXSUM
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

    pJpegHandle->bLuPixSum = bLuPixSum;

    return MT_SUCCESS;

#else
    return MT_FAILURE;
#endif

}

/**
 \brief get lu pixle sum value. CNcomment:获取亮度值 CNend
 \attention \n
If you want to get the luminance value, you can call this function, \n
but you should call it after jpeg_start_decompress and have call MT_JPEG_SetLuPixSumFlag.\n
CNcomment:要是想得到亮度值，可以调用该函数，但必须在jpeg_start_decompress之后调用而且解码 \n
          之前要调用MT_JPEG_SetLuPixSumFlag CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[out] u64LuPixSum. CNcomment:输出亮度值 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_GetLuPixSum
 */
MT_S32 MT_JPEG_GetLuPixSum(const struct jpeg_decompress_struct *cinfo, MT_U64 *u64LuPixSum)

{
	if (NULL == u64LuPixSum)
	{
		return MT_FAILURE;
	}

#ifdef CONFIG_JPEG_OUTPUT_LUPIXSUM
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
	if(MT_TRUE == pJpegHandle->bLuPixSum)
	{
		*u64LuPixSum = pJpegHandle->u64LuPixValue;
	}
	return MT_SUCCESS;

#else
	*u64LuPixSum = 0;
	return MT_FAILURE;
#endif

}

/**
 \brief get jpeg dec time. CNcomment:获取jpeg解码时间 CNend
 \attention \n
If you want to know how much the decode cost time ,you can call MT_JPEG_GetDecTime, \n
but should have called it after jpeg_finish_decompress.\n
CNcomment:要是想看解码花费了多少时间可以调用该函数，但必须在解码完成之后调用 CNend\n

 \param[in]	cinfo. CNcomment:解码对象 CNend
 \param[out] pu32DecTime. CNcomment:输出整个解码时间 CNend

 \retval ::MT_SUCCESS
 \retval ::MT_FAILURE

 \see \n
::MT_JPEG_GetDecTime
 */
MT_S32 MT_JPEG_GetDecTime(const struct jpeg_decompress_struct *cinfo, MT_U32 *pu32DecTime)
{
#ifdef CONFIG_JPEG_GETDECTIME

	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
	if (NULL == pu32DecTime)
	{
		return MT_FAILURE;
	}

	*pu32DecTime = pJpegHandle->u32DecTime;

	return MT_SUCCESS;

#else

	return MT_FAILURE;

#endif
}


/** 
\brief set the leave memory size. CNcomment:设置剩余内存大小可以供解码使用 CNend
\attention \n
should have called it after  create jpeg decoder.\n
CNcomment:必须在创建解码器之后调用 CNend\n

\param[in]	cinfo. CNcomment:解码对象 CNend
\param[int] sMemSizeInfo. CNcomment:内存信息 CNend

\retval ::MT_SUCCESS 
\retval ::MT_FAILURE

\see \n
::MT_JPEG_SetLeaveMemSize
*/
MT_S32 MT_JPEG_SetLeaveMemSize(const struct jpeg_decompress_struct *cinfo, MT_JPEG_MEMSIZE_INFO_S sMemSizeInfo)
{

	MT_U32 u32MemSize = 0;
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
	if(NULL == pJpegHandle)
	{
		return MT_FAILURE;
	}

	switch(sMemSizeInfo.eCheckType)
	{
		case JPEG_MEMCHECK_MEMSIZE:
			u32MemSize = sMemSizeInfo.u32MemSize;
			break;
		case JPEG_MEMCHECK_RESOLUTION:
			u32MemSize = sMemSizeInfo.u32Width * sMemSizeInfo.u32Height * 4;
			break;
		default:
			break;
	}
	
	pJpegHandle->u32LeaveMemSize = u32MemSize;
		
	return MT_SUCCESS;
	
}

MT_S32 MT_JPEG_SetRotate(const struct jpeg_decompress_struct *cinfo, const MT_JPEG_ROT_MOD_E out_rot_mod)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    pJpegHandle->out_rot_mod = (4 - out_rot_mod) % 4;//right rotate changes to left rotate
    return MT_SUCCESS;
}

MT_S32 MT_JPEG_GetRotate(const struct jpeg_decompress_struct *cinfo, MT_JPEG_ROT_MOD_E *rot_mod)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    *rot_mod = pJpegHandle->out_rot_mod;
    return MT_SUCCESS;
}

MT_S32 MT_JPEG_SetClipValueMax(const struct jpeg_decompress_struct *cinfo, const MT_U32 clip_value_max)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    pJpegHandle->clip_value_max = clip_value_max;
    return MT_SUCCESS;
}

MT_S32 MT_JPEG_SetSoftHuff(const struct jpeg_decompress_struct *cinfo, const MT_BOOL on)
{
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
#ifdef CONFIG_MT_FPGA_GPE
    if(cinfo->p_jpeg_dbg_info->sw_dec)
        pJpegHandle->bSoftHuffDec= FALSE;
    else
#endif
    {
        pJpegHandle->bSoftHuffDec = on;
    }   
	MT_INFO_JPEG("\n~~~~~x~~~~~~~~~~%d, 0x%08x++", pJpegHandle->bSoftHuffDec, pJpegHandle);
    return MT_SUCCESS;
}





