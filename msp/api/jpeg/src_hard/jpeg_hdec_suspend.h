/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPEG_HDEC_SUSPEND_H__
#define __JPEG_HDEC_SUSPEND_H__


/*********************************add include here******************************/
#include "mt_jpeg_config.h"

#ifdef CONFIG_JPEG_SUSPEND
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include  "mt_jpeglib.h"
#include  "mt_type.h"

#include  "jpeg_hdec_api.h"

/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C" 
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/
    /** \addtogroup 	 JPEG SUSPEND MACRO */
    /** @{ */  /** <!-- 【JPEG SUSPEND MACRO】 */

	 /** @} */	/*! <!-- Macro Definition end */


	 /*************************** Enum Definition ****************************/
	
	/** \addtogroup      JPEG SUSPEND ENUM */
    /** @{ */  /** <!-- 【JPEG SUSPEND ENUM】 */


    /** @} */  /*! <!-- enum Definition end */

	/*************************** Structure Definition ****************************/
	/** \addtogroup      JPEG SUSPEND STRUCTURE */
    /** @{ */  /** <!-- 【JPEG SUSPEND STRUCTURE】 */


	/** @} */  /*! <!-- Structure Definition end */

	
    /********************** Global Variable declaration **************************/
 
    /******************************* API declaration *****************************/

	/** \addtogroup      JPEG SUSPEND API */
    /** @{ */  /** <!-- 【JPEG SUSPEND API】 */
	
	/*****************************************************************************
	* func			: JPEG_HDEC_GetSuspendSignal
	* description	: get the suspend signal
					  CNcomment:  获取待机信号			      CNend\n
	* param[in] 	:  *pJpegHandle   CNcomment:  解码器句柄  CNend\n
	* param[out] 	: bSuspendSignal  CNcomment:  待机信号    CNend\n
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPEG_HDEC_GetSuspendSignal(const JPEG_HDEC_HANDLE_S  *pJpegHandle, \
	                                                     MT_BOOL *bSuspendSignal);

	/*****************************************************************************
	* func			: JPEG_HDEC_GetResumeSignal
	* description	: get the resume signal
					  CNcomment:  获取待机唤醒信号			   CNend\n
	* param[in] 	:  *pJpegHandle   CNcomment:   解码器句柄  CNend\n
	* param[out] 	: bResumeSignal   CNcomment:   待机唤醒信号 CNend\n
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPEG_HDEC_GetResumeSignal(const JPEG_HDEC_HANDLE_S  *pJpegHandle,\
	                                                    MT_BOOL *bResumeSignal);

	/*****************************************************************************
	* func			: JPEG_HDEC_Resume
	* description	: suspend resume
					  CNcomment: 待机唤醒 CNend\n
	* param[in] 	: cinfo       CNcomment:  解码对象         CNend\n
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPEG_HDEC_Resume(const struct jpeg_decompress_struct *cinfo);
	
	/** @} */  /*! <!-- API declaration end */
	
    /****************************************************************************/



#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */

#endif /* END IF -DCONFIG_JPEG_SUSPEND*/


#endif /* __JPEG_HDEC_SUSPEND_H__*/
