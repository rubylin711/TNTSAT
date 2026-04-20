/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __JPG_SUSPEND_H__
#define __JPG_SUSPEND_H__


/*********************************add include here******************************/

#include "mt_jpeg_config.h"

#ifdef CONFIG_JPEG_SUSPEND

#include "mt_type.h"
#include "mt_jpeg_hal_api.h"

/*****************************************************************************/

#ifdef __cplusplus
#if __cplusplus
   extern "C" 
{
#endif
#endif /* __cplusplus */


    /***************************** Macro Definition ******************************/

	 /*************************** Enum Definition ****************************/

	/*************************** Structure Definition ****************************/

    /********************** Global Variable declaration **************************/
 
    /******************************* API declaration *****************************/

	/*****************************************************************************
	* func			: JPG_WaitDecTaskDone
	* description	: waite the jpeg decode task done
					  CNcomment: 等待解码任务完成  CNend\n
	* param[in] 	: NULL
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPG_WaitDecTaskDone(MT_VOID);


	/*****************************************************************************
	* func			: JPG_GetResumeValue
	* description	: get the value that resume need
					  CNcomment: 获取待机唤醒需要的值  CNend\n
	* param[in] 	: *pSaveInfo
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPG_GetResumeValue(MT_JPG_SAVEINFO_S *pSaveInfo);
	
	/*****************************************************************************
	* func			: JPG_SuspendInit
	* description	: suspend initial
					  CNcomment: 待机初始化  CNend\n
	* param[in] 	: u32JpegRegBase
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPG_SuspendInit(MT_U32 u32JpegRegBase);


	/*****************************************************************************
	* func			: JPG_SuspendExit
	* description	: suspend exit
					  CNcomment: 待机去初始化  CNend\n
	* param[in] 	: u32JpegRegBase
	* retval		: NA
	* others:		: NA
	*****************************************************************************/
	MT_VOID JPG_SuspendExit(MT_VOID);

    /****************************************************************************/



#ifdef __cplusplus
    
#if __cplusplus
   
}
#endif
#endif /* __cplusplus */

#endif

#endif /* __JPG_SUSPEND_H__*/
