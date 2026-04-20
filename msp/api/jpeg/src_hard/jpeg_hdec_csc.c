/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_jpeg_config.h"

#ifndef CONFIG_JPEG_CSC_DISABLE
#include  "jpeg_hdec_csc.h"
	#if 0
	#include  "mt_tde_ioctl.h"
	#else
	#include  "mt_drv_tde.h"
	#endif

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/


/********************** Global Variable declaration **************************/

/******************************* API forward declarations *******************/

/******************************* API realization *****************************/
/*****************************************************************************
* func			: JPEG_HDEC_CSC_Open
* description	: Open the csc device
				  CNcomment: CSC转换设备打开 CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	  CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_CSC_Open(void)
{
#if 0
	MT_S32 s32CscDev = -1;
	s32CscDev = open(CSC_DEV, O_RDWR, 0);
	if (s32CscDev < 0)
	{
		return MT_ERR_TDE_DEV_OPEN_FAILED;
	}
	return s32CscDev;
#else
	return MT_TDE2_Open();
#endif
}

/*****************************************************************************
* func			: JPEG_HDEC_CSC_Close
* description	: close the csc device
				  CNcomment: CSC转换设备关闭 CNend\n
* param[in]	    : s32CscDev     CNcomment: CSC 设备   CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	  CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	  CNend\n
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_CSC_Close(MT_S32 s32CscDev)
{
#if 0
	close(s32CscDev);
#else
	MT_TDE2_Close();
#endif
}

/*****************************************************************************
* func			: JPEG_HDEC_CSC_BeginJob
* description	: create csc task
				  CNcomment: 创建CSC任务 CNend\n
* param[in]	    : s32CscDev     CNcomment: CSC 设备   CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	  CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_CSC_BeginJob(MT_S32 s32CscDev)
{
#if 0
	TDE_HANDLE s32Handle;
	if(s32CscDev < 0)
	{
		return MT_FAILURE;
	}
	if (ioctl(s32CscDev, TDE_BEGIN_JOB, &s32Handle) < 0)
	{
		return MT_ERR_TDE_INVALID_HANDLE;
	}
	return s32Handle;
#else
	return MT_TDE2_BeginJob();
#endif
}

/*****************************************************************************
* func			: JPEG_HDEC_CSC_MbBlit
* description	: run csc task
				  CNcomment: 执行任务 CNend\n
* param[in]	    : s32CscDev     CNcomment: CSC 设备   CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	  CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	  CNend\n
* others:		: NA
*****************************************************************************/

MT_S32 JPEG_HDEC_CSC_MbBlit(TDE_HANDLE s32Handle, TDE2_MB_S* pstMB, TDE2_RECT_S  *pstMbRect, TDE2_SURFACE_S* pstDst,TDE2_RECT_S  *pstDstRect,TDE2_MBOPT_S* pstMbOpt,MT_S32 s32CscDev)
{

#if 0
	TDE_MBBITBLT_CMD_S stMbBlit = {0};

	if(s32CscDev < 0)
	{
		return MT_FAILURE;
	}
	if ((NULL == pstMB) || (NULL == pstDst) || (NULL == pstMbOpt))
	{
		return MT_ERR_TDE_NULL_PTR;
	}

	stMbBlit.s32Handle = s32Handle;
	memcpy(&stMbBlit.stMB, pstMB, sizeof(TDE2_MB_S));
	memcpy(&stMbBlit.stMbRect, pstMbRect, sizeof(TDE2_RECT_S));
	memcpy(&stMbBlit.stDst, pstDst, sizeof(TDE2_SURFACE_S));
	memcpy(&stMbBlit.stDstRect, pstDstRect, sizeof(TDE2_RECT_S));
	memcpy(&stMbBlit.stMbOpt, pstMbOpt, sizeof(TDE2_MBOPT_S));

	return ioctl(s32CscDev, TDE_MB_BITBLT, &stMbBlit);
#else
	TDE2_SURFACE_S TDESurface = {0};
	TDE2_OPT_S stOpt = {0};
	MT_S32 ret;

	if(s32CscDev < 0)
	{
		return MT_FAILURE;
	}
	if ((NULL == pstMB) || (NULL == pstDst) || (NULL == pstMbOpt))
	{
		return MT_ERR_TDE_NULL_PTR;
	}
	ADP_TDEMBSurfaceToTDESurface(pstMB, &TDESurface);
	ADP_TDEMBOptToTDEOpt(pstMbOpt, &stOpt); 
	ret = MT_TDE2_Bitblit(s32Handle, NULL, NULL,
						   &TDESurface, pstMbRect, pstDst,
						   pstDstRect, &stOpt);	
	return ret;
#endif
}


/*****************************************************************************
* func			: JPEG_HDEC_CSC_EndJob
* description	: submit csc task
				  CNcomment: 提交任务 CNend\n
* param[in]	    : s32CscDev     CNcomment: CSC 设备   CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	  CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_CSC_EndJob(TDE_HANDLE s32Handle, MT_BOOL bSync, MT_BOOL bBlock, MT_U32 u32TimeOut,MT_S32 s32CscDev)
{
#if 0
	TDE_ENDJOB_CMD_S stEndJob;

	if(s32CscDev < 0)
	{
		return MT_FAILURE;
	}
	/* Disable sync function */
	bSync = MT_FALSE;
	
	stEndJob.s32Handle  = s32Handle;
	stEndJob.bSync	    = bSync;
	stEndJob.bBlock     = bBlock;
	stEndJob.u32TimeOut = u32TimeOut;

	return ioctl(s32CscDev, TDE_END_JOB, &stEndJob);
#else
    return MT_TDE2_EndJob(s32Handle, bSync, bBlock, u32TimeOut);

#endif
}
#endif
