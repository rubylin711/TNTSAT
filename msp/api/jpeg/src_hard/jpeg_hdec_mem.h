/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPEG_HDEC_MEM_H__
#define __JPEG_HDEC_MEM_H__


/*********************************add include here******************************/

#include  "mt_jpeglib.h"
#include  "mt_type.h"

#include "jpeg_hdec_api.h"

#ifdef CONFIG_GFX_JPGE_ENC_ENABLE
#include "jpge_henc_api.h"
#endif

#include "mt_jpeg_config.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C" 
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/
    /** \addtogroup 	 JPEG MEM MACRO */
    /** @{ */  /** <!-- 【JPEG MEM MACRO】 */

	 /** tmts macro is from the make menuconfig */
	 /** CNcomment:底下这些宏变量可以来之make menuconfig
				   注意硬件buf地址大小要64字节对齐，存储码流buffer没有
				   做此要求，但要在硬件buf范围之内mtmd.l 0x60100000  0x20
				   偏移地址来确定，前两个是buf地址，后两个是码流buf地
				   址，这块临时buffer要永远存在供硬件使用 */
#ifdef CONFIG_JPEG_FPGA_TEST_SUSPEND_ENABLE
	 #define JPGD_HARD_BUFFER				      (1024 * 100 + 64)
	 #define JPGD_STREAM_BUFFER				  (JPGD_HARD_BUFFER - 64)
#else
	 #ifndef  CFG_MT_JPEG6B_STREAMBUFFER_SIZE
	 /** the hard buffer size */
	 /** CNcomment:硬件buffer大小,要64字节对齐 */
	 #define JPGD_HARD_BUFFER				      (1024 * 1024)
	 /** the save stream size,1M is the best,the buffer size should >= INPUT_BUF_SIZE */
	 /** CNcomment:存储码流的buffer大小，经过测试1M是最好的,-64是为了保证在硬件buf范围之内,
	               码流buffer大小必须大于 INPUT_BUF_SIZE = 4096 */
	 #define JPGD_STREAM_BUFFER				  (JPGD_HARD_BUFFER - 64)
	 #else
	 #define JPGD_HARD_BUFFER					  (CFG_MT_JPEG6B_STREAMBUFFER_SIZE) > (4096 + 64) ? (CFG_MT_JPEG6B_STREAMBUFFER_SIZE) : (4096 + 64)
	 #define JPGD_STREAM_BUFFER				  (JPGD_HARD_BUFFER - 64)
	 #endif
#endif
	 /** 底下这里可以像码流buffer一样先固定分配，要是不够销毁按照图像实际大小分配，要是够不再分配 **/
     /** the jpeg hard decode output buffer size */
	 /** 这里暂时不做性能优化，因为需要的固定内存多了 **/
	 #define JPGD_HARD_DEC_OUTBUFFER_SIZE      (1920 * 1280 * 2.4)
	 /** the tde csc output buffer size */
	 /** 这里暂时不做性能优化，因为需要的固定内存多了 **/
     #define TDE_CSC_OUTBUFFER_SIZE             (1920 * 1280 * 4)


	 /** the 8bytes align */
	 /** CNcomment:8字节对齐 */
	 #define JPEG_MCU_8ALIGN                  8
	 /** the 16bytes align */
	 /** CNcomment:16字节对齐 */
	 #define JPEG_MCU_16ALIGN 	              16
	 
	 /** 16bytes align */
	 /** CNcomment:16字节对齐 */
	 #define JPGD_HDEC_MMZ_CSCOUT_STRIDE_ALIGN 	 16
	 /** 128bytes align */
	 /** CNcomment:128字节对齐 */
	 #define JPGD_HDEC_MMZ_YUVSP_STRIDE_ALIGN 	  128
	 /** 128bytes align */
	 /** CNcomment:128字节对齐 */
	 #define JPGD_HDEC_MMZ_ARGB_STRIDE_ALIGN 	  128


	  /** 4bytes align */
	  /** CNcomment:4字节对齐 */
	  #define JPGD_HDEC_MMZ_ARGB_BUFFER_ALIGN 	  4
	  /** 16bytes align */
	  /** CNcomment:16字节对齐 */
	  #define JPGD_HDEC_MMZ_CSCOUT_BUFFER_ALIGN 	  16
	  /** 64bytes align */
	  /** CNcomment:64字节对齐 */
	  #define JPGD_HDEC_MMZ_STREAM_BUFFER_ALIGN 	  64
	  /** 128bytes align */
	  /** CNcomment:128字节对齐 */
	  #define JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN 	  128

#ifdef CONFIG_JPEG_STREAMBUF_4ALIGN
	  /** 16bytes align */
	  /** CNcomment:16字节对齐 */
	  #define JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN 	  4
#endif
	  
	 /** @} */	/*! <!-- Macro Definition end */


	 /*************************** Enum Definition ****************************/

	/** \addtogroup      JPEG MEM ENUM */
    /** @{ */  /** <!-- 【JPEG MEM ENUM】 */


	
    /** @} */  /*! <!-- enum Definition end */

	/*************************** Structure Definition ****************************/

	/** \addtogroup      JPEG MEM STRUCTURE */
    /** @{ */  /** <!-- 【JPEG MEM STRUCTURE】 */

	/** @} */  /*! <!-- Structure Definition end */

	
    /********************** Global Variable declaration **************************/
 
    /******************************* API declaration *****************************/

	/** \addtogroup      JPEG MEM API */
    /** @{ */  /** <!-- 【JPEG MEM API】 */
	

	/*****************************************************************************
	* func			: JPEG_HDEC_GetStreamMem
	* description	: alloc the stream buffer mem
					  CNcomment: 分配码流buffer内存 CNend\n
	* param[in] 	: u32MemSize   CNcomment: 要分配的内存大小	  CNend\n
	* param[out]	: pOutPhyAddr  CNcomment: 分配得到的物理地址  CNend\n
	* param[out]	: pOutVirAddr  CNcomment: 分配得到的虚拟地址  CNend\n
	* retval		: MT_SUCCESS   CNcomment: 成功	CNend\n
	* retval		: MT_FAILURE   CNcomment: 失败	 CNend\n
	* others:		: NA
	*****************************************************************************/
    #ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
	MT_S32	JPEG_HDEC_GetStreamMem(const MT_U32 u32MemSize,MT_CHAR **pOutPhyAddr,MT_CHAR **pOutVirAddr);
    #else
	MT_S32	JPEG_HDEC_GetStreamMem(JPEG_HDEC_HANDLE_S_PTR	 pJpegHandle,const MT_U32 u32MemSize);
    #endif

	
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_FreeStreamMem
	 * description	 : free the stream buffer mem
					   CNcomment: 释放码流buffer内存 CNend\n
	 * param[in]	 : pInPhyAddr	 CNcomment: 要释放的码流buffer物理地址 CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 #ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
	MT_VOID JPEG_HDEC_FreeStreamMem(MT_CHAR *pInPhyAddr, MT_CHAR *pInVirAddr);
	 #else
     MT_VOID JPEG_HDEC_FreeStreamMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle);
	 #endif
	
	
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_GetYUVMem
	 * description	 : get the hard decode output mem
					   CNcomment: 获取硬件解码输出的内存 CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄 CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_GetYUVMem(JPEG_HDEC_HANDLE_S_PTR	 pJpegHandle);
	
	
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_FreeYUVMem
	 * description	 : free the hard decode output mem
					   CNcomment: 释放硬件解码输出的地址  CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄  CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 MT_VOID JPEG_HDEC_FreeYUVMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle);
	
	 #ifdef CONFIG_JPEG_HARDDEC2ARGB
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_GetMinMem
	 * description	 : get dec output argb min memory
					   CNcomment: 获取硬件解码输出为ARGB的行buffer CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄 CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_GetMinMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle);
	
	
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_FreeMinMem
	 * description	 : free dec output argb min memory
					   CNcomment: 释放硬件解码输出为ARGB的行buffer  CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄  CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 MT_VOID JPEG_HDEC_FreeMinMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle);
	 #endif

	 
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_GetOutMem
	 * description	 : get the output buffer
	                   CNcomment: 分配最终输出的内存      CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄 CNend\n
	 * retval		 : MT_SUCCESS    CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE    CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_GetOutMem(const struct jpeg_decompress_struct *cinfo);
	 
	
	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_FreeOutMem
	 * description	 : free the output buf
	                   CNcomment: 释放最终输出的内存        CNend\n
	 * param[in]	 : pJpegHandle   CNcomment: 解码器句柄  CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 MT_VOID JPEG_HDEC_FreeOutMem(JPEG_HDEC_HANDLE_S_PTR pJpegHandle);



#ifdef CONFIG_GFX_JPGE_ENC_ENABLE

	/*****************************************************************************
	 * func 		 : JPGE_HENC_GetEncMem
	 * description	 : get encode need memory
	                   CNcomment: 分配编码需要的内存        CNend\n
	 * param[in]	 : s32MemSize   CNcomment: 要分配的内存大小  CNend\n
	 * param[ou]	 : pJpgeHandle   CNcomment: 编码句柄         CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPGE_HENC_GetEncMem(MT_S32 s32MemSize,JPGE_HENC_HANDLE_S_PTR pJpgeHandle);


	/*****************************************************************************
	 * func 		 : JPGE_HENC_FreeEncMem
	 * description	 : free encode memory
	                   CNcomment: 释放编码内存                   CNend\n
	 * param[in]	 : pJpgeHandle   CNcomment: 编码句柄  CNend\n
	 * retval		 : NA
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPGE_HENC_FreeEncMem(JPGE_HENC_HANDLE_S_PTR pJpgeHandle);
	
#endif
	
	/** @} */  /*! <!-- API declaration end */
	
    /****************************************************************************/



#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */

#endif /* __JPEG_HDEC_MEM_H__*/
