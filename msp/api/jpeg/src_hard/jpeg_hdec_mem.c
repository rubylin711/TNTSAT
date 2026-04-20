/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include  "mt_jpeg_config.h"
#include  "jpeg_hdec_mem.h"
#include  "jpeg_hdec_api.h"
#include  "jpeg_hdec_error.h"

/***************************** Macro Definition ******************************/

/** the first class is jpeg */
/** CNcomment:第一级内存为jpeg分区 */
#define MMZ_TAG          "jpeg"
/** the second class is jpeg */
/** CNcomment:第二级内存为grapmtcs分区 */
//#define MMZ_TAG_1        "grapmtcs"
/** the last class is jpeg */
/** CNcomment:最后一级内存为整个MMZ分区 */
//#define MMZ_TAG_2        ""

/** the module name */
/** CNcomment:分配给jpeg模块的名字，这里可以通过mmz proc来查看分配给谁了 */
#define MMZ_MODULE       "JPEG_STREAM_OUT_BUF"


#if defined(CONFIG_JPEG_ANDROID_DEBUG_ENABLE) && defined(CONFIG_JPEG_DEBUG_INFO)
#define LOG_TAG    "libjpeg"
#endif

/*************************** Structure Definition ****************************/


/********************** Global Variable declaration **************************/

/******************************* API forward declarations *******************/

/******************************* API realization *****************************/


/*****************************************************************************
* func			: JPEG_HDEC_GetStreamMem
* description	: alloc the stream buffer mem
                  CNcomment: 分配码流buffer内存 CNend\n
* param[in] 	: u32MemSize   CNcomment: 要分配的内存大小    CNend\n
* param[out]	: pOutPhyAddr  CNcomment: 分配得到的物理地址  CNend\n
* param[out]	: pOutVirAddr  CNcomment: 分配得到的虚拟地址  CNend\n
* retval		: MT_SUCCESS   CNcomment: 成功  CNend\n
* retval		: MT_FAILURE   CNcomment: 失败   CNend\n
* others:		: NA
*****************************************************************************/
#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
MT_S32	JPEG_HDEC_GetStreamMem(const MT_U32 u32MemSize,MT_CHAR **pOutPhyAddr,MT_CHAR **pOutVirAddr)
#else
MT_S32	JPEG_HDEC_GetStreamMem(JPEG_HDEC_HANDLE_S_PTR	 pJpegHandle,const MT_U32 u32MemSize)
#endif
{


		phys_addr_t pPhyBuf = 0;
		MT_CHAR *pVirBuf = NULL;
		MT_U32 u32StreamSize = JPGD_STREAM_BUFFER;

		JPEG_ASSERT((0 == u32MemSize),MT_FAILURE);

		if(u32StreamSize < 4096)/*lint !e774 ignore by y00181162, because tmts cast is ok */  
		{
			//JPEG_TRACE("the save stream size is small than the input buffer size\n");
			return MT_FAILURE;
		}
		/**
		** use the tmtrd class manage to alloc mem,the stream buffer should 64bytes align
		** CNcomment: 使用三级分配管理来分配内存 CNend\n
		**/
        #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pPhyBuf = (MT_CHAR*)MT_GFX_AllocMem(pJpegHandle->s32MMZDev,u32MemSize, JPGD_HDEC_MMZ_STREAM_BUFFER_ALIGN, (MT_CHAR*)MMZ_TAG, (MT_CHAR*)MMZ_MODULE, &(pJpegHandle->pSaveStreamMemHandle));
        #else
		pPhyBuf = MT_GFX_AllocMem(u32MemSize, JPGD_HDEC_MMZ_STREAM_BUFFER_ALIGN, (MT_CHAR*)MMZ_TAG, (MT_CHAR*)MMZ_MODULE);
        #endif
		if(0 == pPhyBuf)
		{
			//JPEG_TRACE("%s %s %d == MT_GFX_AllocMem FAILURE\n",__FILE__,__FUNCTION__,__LINE__);
			return MT_FAILURE;
		}
	    #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pVirBuf = (MT_CHAR*)MT_GFX_MapCached(pJpegHandle->s32MMZDev,(phys_addr_t)pPhyBuf, pJpegHandle->pSaveStreamMemHandle);
        #else
		pVirBuf = (MT_CHAR*)MT_GFX_MapCached(pPhyBuf);
        #endif
		if(NULL == pVirBuf)
		{
			//JPEG_TRACE("MT_GFX_MapCached FAILURE\n");
			return MT_FAILURE;
		}
#if 0
		/**
		** when use tmts mem, should memset tmts mem
		** CNcomment: 在使用该内存的地址初始化该内存，memset需要时间，使用与否慎用 CNend\n
		**/
		memset(pVirAddr,0,u32MemSize);
		MT_GFX_Flush(pVirBuf, 0, 0);
#endif


#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
		*pOutPhyAddr = pPhyBuf;
		*pOutVirAddr = pVirBuf;
#else
		pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf      = pPhyBuf;
		pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf      = pVirBuf;
		/**
		** use the virtual memory, every time should read data size
		** CNcomment: 虚拟内存码流的时候每次需要读取的码流大小 CNend\n
		**/
		pJpegHandle->stHDecDataBuf.u32ReadDataSize	      = u32StreamSize;
#endif


		return MT_SUCCESS;

		
}



/*****************************************************************************
* func			: JPEG_HDEC_FreeStreamMem
* description	: free the stream buffer mem
                  CNcomment: 释放码流buffer内存 CNend\n
* param[in] 	: pInPhyAddr    CNcomment: 要释放的码流buffer物理地址 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
MT_VOID JPEG_HDEC_FreeStreamMem(MT_CHAR *pInPhyAddr, MT_CHAR *pInVirAddr)
#else
MT_VOID JPEG_HDEC_FreeStreamMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle)
#endif
{

       MT_S32 s32Ret = MT_SUCCESS;
	   
#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
	  if(NULL == pInPhyAddr)
	  {
	     return;
	  }
	  s32Ret = MT_GFX_Unmap(pInVirAddr);
	  s32Ret = MT_GFX_FreeMem(pInPhyAddr);
	  if(MT_SUCCESS != s32Ret)
	  {
	     return;
	  }
#else

      if(NULL == pJpegHandle)
      {
           JPEG_TRACE("%s :%s : %d (the pJpegHandle is NULL)\n",__FILE__,__FUNCTION__,__LINE__);
		   return;
      }
	  
	  if( 0 == pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf)
	  {
		   return;
	  }
	  #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
	  s32Ret = MT_GFX_Unmap(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
	  s32Ret = MT_GFX_FreeMem(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf, pJpegHandle->pSaveStreamMemHandle);
	  #else
	  s32Ret = MT_GFX_Unmap(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf);
	  s32Ret = MT_GFX_FreeMem(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf);
	  #endif
	  if(MT_SUCCESS != s32Ret)
	  {
	      JPEG_TRACE("MT_GFX_Unmap or  MT_GFX_FreeMem FAILURE\n");
	      return;
	  }
	  pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf	    = 0;
	  pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf	    = NULL;
#endif
	  
}


/*****************************************************************************
* func			: JPEG_HDEC_GetYUVMem
* description	: get the hard decode output mem
				  CNcomment: 获取硬件解码输出的内存 CNend\n
* param[in]	    : pJpegHandle   CNcomment: 解码器句柄 CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_GetYUVMem(JPEG_HDEC_HANDLE_S_PTR	pJpegHandle)
{


		MT_U32 u32MemSize = 0;
		phys_addr_t pYUVPhy  = 0;
		MT_CHAR *pYUVVir  = NULL;
		MT_U32 u32Align   = 0;
		MT_S32 s32Ret = MT_SUCCESS;
        
		/**
		 ** check whether to alloc jpeg hard decode middle mem
		 ** CNcomment: 判断是否分配硬件解码的中间buffer CNend\n
		 **/
#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE == pJpegHandle->bDecARGB)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))	
#endif
		{/**
		  ** use user mem
		  ** CNcomment: 使用用户内存 CNend\n
		  **/
		      pJpegHandle->stMiddleSurface.pMiddlePhy[0] = pJpegHandle->stOutDesc.stOutSurface.pOutPhy[0];
		      pJpegHandle->stMiddleSurface.pMiddlePhy[1] = pJpegHandle->stOutDesc.stOutSurface.pOutPhy[1];
			  pJpegHandle->stMiddleSurface.pMiddlePhy[2] = pJpegHandle->stOutDesc.stOutSurface.pOutPhy[2];
		      pJpegHandle->stMiddleSurface.pMiddleVir[0] = pJpegHandle->stOutDesc.stOutSurface.pOutVir[0];
		      pJpegHandle->stMiddleSurface.pMiddleVir[1] = pJpegHandle->stOutDesc.stOutSurface.pOutVir[1];
			  pJpegHandle->stMiddleSurface.pMiddleVir[2] = pJpegHandle->stOutDesc.stOutSurface.pOutVir[2];
		      return MT_SUCCESS;
		}


#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
		  /**
		   ** 4bytes align just ok
		   ** CNcomment: 4字节对齐就可以了 CNend\n
		   **/
		   u32Align   = JPGD_HDEC_MMZ_ARGB_BUFFER_ALIGN;
		   u32MemSize = pJpegHandle->stJpegSofInfo.u32YSize;
		}
		else
#endif
		{
		   u32Align   = JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN;
		   u32MemSize = pJpegHandle->stJpegSofInfo.u32YSize + pJpegHandle->stJpegSofInfo.u32CSize;
		}
		/**
		 ** use the tmtrd class manage to alloc mem,the stream buffer should 128bytes align
		 ** CNcomment: 使用三级分配管理来分配内存,buffer要128字节对齐 CNend\n
		 **/
		#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pYUVPhy = (MT_CHAR*)MT_GFX_AllocMem(pJpegHandle->s32MMZDev,u32MemSize,u32Align,(MT_CHAR*)MMZ_TAG,(MT_CHAR*)MMZ_MODULE, &(pJpegHandle->pMiddleMemHandle));
		#else
		pYUVPhy = MT_GFX_AllocMem(u32MemSize,u32Align,(MT_CHAR*)MMZ_TAG,(MT_CHAR*)MMZ_MODULE);
		#endif
		if(0 == pYUVPhy)
		{
		     //JPEG_TRACE("%s %s %d == MT_GFX_AllocMem FAILURE\n",__FILE__,__FUNCTION__,__LINE__);
		     return MT_FAILURE;
		}

#ifndef CONFIG_JPEG_TEST_SAVE_YUVSP_DATA
/** if need save yuvsp data,should virtual address **/
		#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if( (MT_TRUE == pJpegHandle->bOutYCbCrSP) || (MT_TRUE == pJpegHandle->bDecARGB))
		#else
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP)
		#endif
#endif
		{
			#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			pYUVVir = (MT_CHAR *)MT_GFX_MapCached(pJpegHandle->s32MMZDev,(phys_addr_t)pYUVPhy, pJpegHandle->pMiddleMemHandle);
			#else
			pYUVVir = (MT_CHAR *)MT_GFX_MapCached(pYUVPhy);
			#endif
			if (NULL == pYUVVir)
			{
				//JPEG_TRACE("MT_GFX_MapCached FAILURE\n");
				return MT_FAILURE;
			}
			/**
			** when use tmts mem, should memset tmts mem
			** CNcomment: 在使用该内存的地址初始化该内存，memset需要时间，使用与否慎用 CNend\n
			**/
			//memset(pYUVVir,0,u32MemSize);
			#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,(void*)pYUVVir, pJpegHandle->pMiddleMemHandle);
			#else
			s32Ret = MT_GFX_Flush((void*)pYUVVir, 0, 0);
			#endif
			if(MT_SUCCESS != s32Ret)
			{
				return MT_FAILURE;
			}
			pJpegHandle->stMiddleSurface.pMiddleVir[0] = pYUVVir;
			pJpegHandle->stMiddleSurface.pMiddleVir[1] = pYUVVir + pJpegHandle->stJpegSofInfo.u32YSize;
			
		}

		pJpegHandle->stMiddleSurface.pMiddlePhy[0] = pYUVPhy;
		pJpegHandle->stMiddleSurface.pMiddlePhy[1] = pYUVPhy + pJpegHandle->stJpegSofInfo.u32YSize;
		 
		return MT_SUCCESS;


}

/*****************************************************************************
* func			: JPEG_HDEC_FreeYUVMem
* description	: free the hard decode output mem
				  CNcomment: 释放硬件解码输出的地址  CNend\n
* param[in]	    : pJpegHandle   CNcomment: 解码器句柄 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_FreeYUVMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle)
{

		MT_S32 s32Ret = MT_SUCCESS;


		if(NULL == pJpegHandle)
		{
			JPEG_TRACE("%s :%s : %d (the pJpegHandle is NULL)\n",__FILE__,__FUNCTION__,__LINE__);
			return;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP || (MT_TRUE == pJpegHandle->bDecARGB))
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
#else
		if(   (MT_TRUE == pJpegHandle->bOutYCbCrSP)
			&&(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))

#endif
		{/**
		** use user mem
		** CNcomment: 使用用户内存 CNend\n
		**/
			return;
		}

		if(0 == pJpegHandle->stMiddleSurface.pMiddlePhy[0])
		{
			return;
		}

#ifndef CONFIG_JPEG_TEST_SAVE_YUVSP_DATA
		#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if( (MT_TRUE == pJpegHandle->bOutYCbCrSP) || (MT_TRUE == pJpegHandle->bDecARGB))
		#else
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP)
		#endif
#endif
		{
		    #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		    s32Ret = MT_GFX_Unmap(pJpegHandle->s32MMZDev,pJpegHandle->stMiddleSurface.pMiddleVir[0], pJpegHandle->pMiddleMemHandle);
			#else
			s32Ret = MT_GFX_Unmap(pJpegHandle->stMiddleSurface.pMiddleVir[0]);
			#endif
			if(MT_SUCCESS != s32Ret)
			{
				JPEG_TRACE("MT_GFX_Unmap FAILURE\n");
				return;
			}
		}

		#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		s32Ret = MT_GFX_FreeMem(pJpegHandle->s32MMZDev,(phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[0], pJpegHandle->pMiddleMemHandle);
		#else
		s32Ret = MT_GFX_FreeMem(pJpegHandle->stMiddleSurface.pMiddlePhy[0]);
		#endif
		if(MT_SUCCESS != s32Ret)
		{
			JPEG_TRACE("MT_GFX_FreeMem FAILURE\n");
			return;
		}
		pJpegHandle->stMiddleSurface.pMiddlePhy[0]  = 0;
		pJpegHandle->stMiddleSurface.pMiddlePhy[1]  = 0;
		pJpegHandle->stMiddleSurface.pMiddleVir[0]  = NULL;
		pJpegHandle->stMiddleSurface.pMiddleVir[1]  = NULL;
					
}


#ifdef CONFIG_JPEG_HARDDEC2ARGB
/*****************************************************************************
* func			: JPEG_HDEC_GetMinMem
* description	: get dec output argb min memory
				  CNcomment: 获取硬件解码输出为ARGB的行buffer CNend\n
* param[in]	    : pJpegHandle   CNcomment: 解码器句柄 CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_GetMinMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle)
{

		MT_U32 u32MemSize = 0;
		phys_addr_t pMinPhy  = 0;

   		u32MemSize = pJpegHandle->stJpegSofInfo.u32RGBSizeReg;

		/**
	 	 ** use the tmtrd class manage to alloc mem,the min buffer should 128bytes align
	     ** CNcomment: 使用三级分配管理来分配内存,argb行buffer要128字节对齐 CNend\n
	     **/
	    #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
        pMinPhy = (MT_CHAR*)MT_GFX_AllocMem(pJpegHandle->s32MMZDev,u32MemSize,JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN,(MT_CHAR*)MMZ_TAG,(MT_CHAR*)MMZ_MODULE, &(pJpegHandle->pMinMemHandle));
        #else
		pMinPhy = MT_GFX_AllocMem(u32MemSize,JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN,(MT_CHAR*)MMZ_TAG,(MT_CHAR*)MMZ_MODULE);
		#endif
		if(0 == pMinPhy)
		{
		     //JPEG_TRACE("%s %s %d == MT_GFX_AllocMem FAILURE\n",__FILE__,__FUNCTION__,__LINE__);
			 return MT_FAILURE;
		}

		pJpegHandle->pMinPhyBuf   =   pMinPhy;
			
		return MT_SUCCESS;


}

/*****************************************************************************
* func			: JPEG_HDEC_FreeMinMem
* description	: free dec output argb min memory
				  CNcomment: 释放硬件解码输出为ARGB的行buffer  CNend\n
* param[in]	    : pJpegHandle   CNcomment: 解码器句柄 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_FreeMinMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle)
{

	    MT_S32 s32Ret = MT_SUCCESS;

		if(NULL == pJpegHandle)
		{
			 JPEG_TRACE("%s :%s : %d (the pJpegHandle is NULL)\n",__FILE__,__FUNCTION__,__LINE__);
			 return;
		}

		if(0 == pJpegHandle->pMinPhyBuf)
		{
		   return;
		}
        #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		s32Ret = MT_GFX_FreeMem(pJpegHandle->s32MMZDev,(MT_U32)pJpegHandle->pMinPhyBuf, pJpegHandle->pMinMemHandle);
		#else
        s32Ret = MT_GFX_FreeMem((phys_addr_t)pJpegHandle->pMinPhyBuf);
		#endif
		if(MT_SUCCESS != s32Ret)
		{
		   JPEG_TRACE("MT_GFX_FreeMem FAILURE\n");
		   return;
		}
				
		pJpegHandle->pMinPhyBuf  = 0;

}
#endif


/*****************************************************************************
* func			: JPEG_HDEC_GetOutMem
* description	: get the output buffer
				  CNcomment: 分配最终输出的内存 	 CNend\n
* param[in]	    : cinfo         CNcomment: 解码对象  CNend\n
* retval		: MT_SUCCESS    CNcomment: 成功		  CNend\n
* retval		: MT_FAILURE    CNcomment: 失败		  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_GetOutMem(const struct jpeg_decompress_struct *cinfo)
{


		MT_U32 u32OutStride = 0;
		MT_U32 u32MemSize   = 0;
		phys_addr_t pOutPhy    = 0;
		MT_CHAR* pOutVir    = NULL;
        MT_S32 s32Ret = MT_SUCCESS;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);


#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE  == pJpegHandle->bDecARGB)
#else
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP)
#endif
		{
		/**
		** shoule not csc,so not alloc output mem
		** CNcomment: 不需要颜色空间转换，所以就不需要分配输出buffer CNend\n
		**/
			return MT_SUCCESS;
		}

		if(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)
		{  
		/**
		** use user mem
		** CNcomment: 使用用户内存 CNend\n
		**/
			pJpegHandle->stMiddleSurface.pOutPhy = pJpegHandle->stOutDesc.stOutSurface.pOutPhy[0];
			pJpegHandle->stMiddleSurface.pOutVir = pJpegHandle->stOutDesc.stOutSurface.pOutVir[0];
			return MT_SUCCESS;
		}
//		u32OutStride = pJpegHandle->stJpegSofInfo.u32YOutStride;
		u32OutStride = pJpegHandle->stJpegSofInfo.u32DisplayStride;
		u32MemSize   = u32OutStride * ((MT_U32)pJpegHandle->stOutDesc.stCropRect.h);
		
		/**
		** align depend the pixle
		** CNcomment: 按照像素对齐 CNend\n
		**/
		/** 3字节对齐是有问题的，因为map不上来**/
		#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pOutPhy = (MT_CHAR*)MT_GFX_AllocMem(pJpegHandle->s32MMZDev,u32MemSize, JPGD_HDEC_MMZ_CSCOUT_BUFFER_ALIGN, (MT_CHAR*)MMZ_TAG, (MT_CHAR*)MMZ_MODULE, &(pJpegHandle->pOutMemHandle));
        #else
		pOutPhy = MT_GFX_AllocMem(u32MemSize, JPGD_HDEC_MMZ_CSCOUT_BUFFER_ALIGN, (MT_CHAR*)MMZ_TAG, (MT_CHAR*)MMZ_MODULE);
		#endif
		if(0 == pOutPhy)
		{
			//JPEG_TRACE("%s %s %d == MT_GFX_AllocMem FAILURE\n",__FILE__,__FUNCTION__,__LINE__);
			return MT_FAILURE;
		}
		#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pOutVir = (MT_CHAR*)MT_GFX_MapCached(pJpegHandle->s32MMZDev,(phys_addr_t)pOutPhy, pJpegHandle->pOutMemHandle);
		#else
		pOutVir = (MT_CHAR*)MT_GFX_MapCached(pOutPhy);
		#endif
		if (NULL == pOutVir)
		{
			//JPEG_TRACE("MT_GFX_MapCached FAILURE\n");
			return MT_FAILURE;
		}
		/** memset 也需要耗时间，假如解码正常就不要memset操作了 **/
		//memset(pOutVir,0,u32MemSize);
		#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,(ulong)pOutVir, pJpegHandle->pOutMemHandle);
        #else
		s32Ret = MT_GFX_Flush((void*)pOutVir, 0, 0);
		#endif
		if(MT_SUCCESS != s32Ret)
		{
			return MT_FAILURE;
		}
		/**
		** 要是用户没有设置输出图像大小则就使用默认的输出，也就是只能1/2/4/8四种缩放
		**/
		pJpegHandle->stMiddleSurface.pOutPhy   =  pOutPhy;
		pJpegHandle->stMiddleSurface.pOutVir   =  pOutVir;

		return MT_SUCCESS;

		
}

/*****************************************************************************
* func			: JPEG_HDEC_FreeOutMem
* description	: free the output buf
				  CNcomment: 释放最终输出的内存 	   CNend\n
* param[in]	    : pJpegHandle   CNcomment: 解码器句柄  CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_FreeOutMem(JPEG_HDEC_HANDLE_S_PTR	 pJpegHandle)
{

		MT_S32 s32Ret = MT_SUCCESS;

		if(NULL == pJpegHandle)
		{
				JPEG_TRACE("%s :%s : %d (the pJpegHandle is NULL)\n",__FILE__,__FUNCTION__,__LINE__);
				return;
		}
#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP || MT_TRUE  == pJpegHandle->bDecARGB)
#else
		if(MT_TRUE == pJpegHandle->bOutYCbCrSP)
#endif
		{
			return;
		}

		if(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)
		{   
			return;
		}


		if(0 == pJpegHandle->stMiddleSurface.pOutPhy)
		{
			return;
		}
        #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		s32Ret = MT_GFX_Unmap(pJpegHandle->s32MMZDev,pJpegHandle->stMiddleSurface.pOutVir, pJpegHandle->pOutMemHandle);
		s32Ret = MT_GFX_FreeMem(pJpegHandle->s32MMZDev,pJpegHandle->stMiddleSurface.pOutPhy, pJpegHandle->pOutMemHandle);
		#else
		s32Ret = MT_GFX_Unmap(pJpegHandle->stMiddleSurface.pOutVir);
		s32Ret = MT_GFX_FreeMem((phys_addr_t)pJpegHandle->stMiddleSurface.pOutPhy);
		#endif
		if(MT_SUCCESS != s32Ret)
		{
			JPEG_TRACE("MT_GFX_Unmap or MT_GFX_FreeMem FAILURE\n");
			return;
		}
		pJpegHandle->stMiddleSurface.pOutPhy  = 0;
		pJpegHandle->stMiddleSurface.pOutVir  = NULL; 

}

#ifdef CONFIG_GFX_JPGE_ENC_ENABLE

/*****************************************************************************
 * func 		 : JPGE_HENC_GetEncMem
 * description	 : get encode need memory
                   CNcomment: 分配编码需要的内存        CNend\n
 * param[in]	 : s32MemSize   CNcomment: 要分配的内存大小  CNend\n
 * param[ou]	 : pu32OutPhy   CNcomment: 返回物理地址      CNend\n
 * param[ou]	 : pOutVir      CNcomment: 返回虚拟地址      CNend\n
 * retval		 : NA
 * others:		 : NA
 *****************************************************************************/
 MT_S32 JPGE_HENC_GetEncMem(MT_S32 s32MemSize,JPGE_HENC_HANDLE_S_PTR pJpgeHandle)
{

		MT_S32 s32Ret = MT_SUCCESS;
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pJpgeHandle->pPhy = (MT_CHAR*)MT_GFX_AllocMem(pJpgeHandle->s32MMZDev,s32MemSize, JPGD_HDEC_MMZ_STREAM_BUFFER_ALIGN, (MT_CHAR*)JPGE_TAG, (MT_CHAR*)JPGE_MODULE, &(pJpgeHandle->pMemHandle));
#else
		pJpgeHandle->pPhy = MT_GFX_AllocMem(s32MemSize, JPGD_HDEC_MMZ_STREAM_BUFFER_ALIGN, (MT_CHAR*)JPGE_TAG, (MT_CHAR*)JPGE_MODULE);
#endif
		if(0 == pJpgeHandle->pPhy)
		{
			return MT_FAILURE;
		}
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pJpgeHandle->pVir = (MT_CHAR*)MT_GFX_MapCached(pJpgeHandle->s32MMZDev,(phys_addr_t)pJpgeHandle->pPhy, pJpgeHandle->pMemHandle);
#else
	    pJpgeHandle->pVir = (MT_CHAR*)MT_GFX_Map((phys_addr_t)pJpgeHandle->pPhy);
#endif
		if(NULL == pJpgeHandle->pVir)
		{
			#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
				MT_GFX_FreeMem(pJpgeHandle->s32MMZDev,(phys_addr_t)pJpgeHandle->pPhy, pJpgeHandle->pMemHandle);
			#else
				MT_GFX_FreeMem((phys_addr_t)pJpgeHandle->pPhy);
			#endif
			pJpgeHandle->pPhy = 0;
			pJpgeHandle->pVir = NULL;
			return MT_FAILURE;
		}

		memset(pJpgeHandle->pVir,0,s32MemSize);
		
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		s32Ret = MT_GFX_Flush(pJpgeHandle->s32MMZDev,(ulong)pJpgeHandle->pVir, pJpgeHandle->pMemHandle);
#else
		s32Ret = MT_GFX_Flush((void*)pJpgeHandle->pVir, 0, 0);
#endif
		if(MT_SUCCESS != s32Ret)
		{
			#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
				MT_GFX_Unmap(pJpgeHandle->s32MMZDev,pJpgeHandle->pVir, pJpgeHandle->pMemHandle);
				MT_GFX_FreeMem(pJpgeHandle->s32MMZDev,pJpgeHandle->pPhy, pJpgeHandle->pMemHandle);
			#else
				MT_GFX_Unmap((ulong)pJpgeHandle->pVir);
				MT_GFX_FreeMem(pJpgeHandle->pPhy);
			#endif
			return MT_FAILURE;
		}
		
		return MT_SUCCESS;
		
}


/*****************************************************************************
 * func 		 : JPGE_HENC_FreeEncMem
 * description	 : free encode memory
                   CNcomment: 释放编码内存                   CNend\n
 * param[in]	 : u32PhyAddr   CNcomment: 要释放的物理地址  CNend\n
 * retval		 : NA
 * others:		 : NA
 *****************************************************************************/
 MT_S32 JPGE_HENC_FreeEncMem(JPGE_HENC_HANDLE_S_PTR pJpgeHandle)
{

	MT_S32 s32Ret = MT_SUCCESS;
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
	s32Ret = MT_GFX_Unmap(pJpgeHandle->s32MMZDev,pJpgeHandle->pVir, pJpgeHandle->pMemHandle);
	s32Ret = MT_GFX_FreeMem(pJpgeHandle->s32MMZDev,pJpgeHandle->pPhy, pJpgeHandle->pMemHandle);
#else
	s32Ret = MT_GFX_Unmap((ulong)pJpgeHandle->pVir);
	s32Ret = MT_GFX_FreeMem((phys_addr_t)pJpgeHandle->pPhy);
#endif
	if(MT_SUCCESS != s32Ret)
	{
	   JPEG_TRACE("MT_GFX_Unmap or MT_GFX_FreeMem FAILURE\n");
	   return MT_FAILURE;
	}
	return MT_SUCCESS;
}
#endif
