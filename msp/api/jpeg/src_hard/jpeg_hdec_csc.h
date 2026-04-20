/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPEG_HDEC_CSC_H__
#define __JPEG_HDEC_CSC_H__

#include  "mt_jpeg_config.h"


#ifndef CONFIG_JPEG_CSC_DISABLE

/*********************************add include here******************************/

#include  "jpeg_hdec_api.h"
#if 0
#include  "mt_tde_type.h"
#else
#include  "mt_tde_api.h"
#endif

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
	 * func 		 : JPEG_HDEC_CSC_Open
	 * description	 : Open the csc device
					   CNcomment: CSC转换设备打开 CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_CSC_Open(void);

	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_CSC_Close
	 * description	 : close the csc device
					   CNcomment: CSC转换设备关闭 CNend\n
	 * param[in]	 : s32CscDev     CNcomment: CSC 设备   CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_VOID JPEG_HDEC_CSC_Close(MT_S32 s32CscDev);

	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_CSC_BeginJob
	 * description	 : create csc task
					   CNcomment: 创建CSC任务 CNend\n
	 * param[in]	 : s32CscDev     CNcomment: CSC 设备   CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_CSC_BeginJob(MT_S32 s32CscDev);
	 
     /*****************************************************************************
	 * func 		 : JPEG_HDEC_CSC_MbBlit
	 * description	 : run csc task
					   CNcomment: 执行任务 CNend\n
	 * param[in]	 : s32CscDev     CNcomment: CSC 设备   CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_CSC_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S* pstMB, TDE2_RECT_S  *pstMbRect, TDE2_SURFACE_S* pstDst,TDE2_RECT_S  *pstDstRect,TDE2_MBOPT_S* pstMbOpt,MT_S32 s32CscDev);


	 /*****************************************************************************
	 * func 		 : JPEG_HDEC_CSC_EndJob
	 * description	 : submit csc task
					   CNcomment: 提交任务 CNend\n
	 * param[in]	 : s32CscDev     CNcomment: CSC 设备   CNend\n
	 * retval		 : MT_SUCCESS	 CNcomment: 成功       CNend\n
	 * retval		 : MT_FAILURE	 CNcomment: 失败       CNend\n
	 * others:		 : NA
	 *****************************************************************************/
	 MT_S32 JPEG_HDEC_CSC_EndJob(TDE_HANDLE s32Handle, MT_BOOL bSync, MT_BOOL bBlock, MT_U32 u32TimeOut,MT_S32 s32CscDev);
	 
	/** @} */  /*! <!-- API declaration end */
	
    /****************************************************************************/



#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */


#endif


#endif /* __JPEG_HDEC_CSC_H__*/
