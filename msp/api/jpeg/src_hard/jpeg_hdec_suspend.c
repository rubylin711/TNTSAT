/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "jpeg_hdec_suspend.h"

#ifdef CONFIG_JPEG_SUSPEND

#include "jpeg_hdec_api.h"
#include "jpeg_hdec_rwreg.h"
#include "jpeg_hdec_adp.h"
#include "mt_jpeg_hal_api.h"
#include "mt_drv_jpeg_reg.h"

/***************************** Macro Definition ******************************/

/******************** to see wmtch include file we want to use***************/



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/



/******************************* API forward declarations *******************/

/******************************* API realization *****************************/


/*****************************************************************************
* func			: JPEG_HDEC_GetSuspendSignal
* description	: get the suspend signal
				  CNcomment:  获取待机信号             CNend\n
* param[in] 	: *pJpegHandle    CNcomment:  解码器句柄   CNend\n
* param[in] 	: bSuspendSignal  CNcomment:  待机信号 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_GetSuspendSignal(const JPEG_HDEC_HANDLE_S  *pJpegHandle,MT_BOOL *bSuspendSignal)
{


		MT_S32  s32RetVal = 0;
		MT_BOOL bSuspend  = MT_FALSE;
		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_GETSUSPEND, &bSuspend);
		if(MT_SUCCESS != s32RetVal)
		{
			return;
		}

		*bSuspendSignal = bSuspend;

}

/*****************************************************************************
* func			: JPEG_HDEC_GetResumeSignal
* description	: get the resume signal
				  CNcomment:  获取待机唤醒信号             CNend\n
* param[in] 	: *pJpegHandle    CNcomment:  解码器句柄   CNend\n
* param[in] 	: bResumeSignal  CNcomment:   待机唤醒信号 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_GetResumeSignal(const JPEG_HDEC_HANDLE_S  *pJpegHandle,MT_BOOL *bResumeSignal)
{

		MT_S32  s32RetVal = 0;
		MT_BOOL bResume   = MT_FALSE;

		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_GETRESUME, &bResume);
		if(MT_SUCCESS != s32RetVal)
		{
			return;
		}

		*bResumeSignal = bResume;

}

/*****************************************************************************
* func			: JPEG_HDEC_Resume
* description	: suspend resume
				  CNcomment: 待机唤醒 CNend\n
* param[in] 	: cinfo       CNcomment:  解码对象         CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
MT_VOID JPEG_HDEC_Resume(const struct jpeg_decompress_struct *cinfo)
{


		MT_S32 s32RetVal = 0;
        MT_JPG_SAVEINFO_S stResumeValue;

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		memset(&stResumeValue,0,sizeof(MT_JPG_SAVEINFO_S));

#if defined(CONFIG_JPEG_FPGA_TEST_ENABLE) && defined(CONFIG_JPEG_FPGA_TEST_SUSPEND_ENABLE)
		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_RESET);
		if(MT_SUCCESS != s32RetVal)
		{
			 return;
		}
		//system("mtmd.l 0xFF300000");
#endif
		/**
		** cancel reset
		** CNcomment:撤消复位 CNend\n
		**/
		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_CANCEL_RESET);
		if(MT_SUCCESS != s32RetVal)
		{
			 return;
		}
		/**
		** waite cancel reset success
		** CNcomment:等待撤消复位成功 CNend\n
		**/
		MT_USLEEP(1000); /** 1ms at least **/

		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_GETRESUMEVALUE, &stResumeValue);
		if(MT_SUCCESS != s32RetVal)
		{
			return;
		}
        pJpegHandle->u32ResByteConsum = stResumeValue.u32ResByteConsu;

		s32RetVal = JPEG_HDEC_SetPara(cinfo);
		if(MT_SUCCESS != s32RetVal)
		{
			 return;
		}

		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_BSRES_DATA0_CFG, (MT_S32)stResumeValue.u32ResumeData0);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_BSRES_DATA1_CFG, (MT_S32)stResumeValue.u32ResumeData1);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_BSRES_BIT_CFG,   (MT_S32)stResumeValue.u32ResBitRemain);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_MCUY_CFG,        (MT_S32)stResumeValue.u32ResMcuy);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PD_Y_CFG,        (MT_S32)stResumeValue.u32Pdy);
		JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PD_CBCR_CFG,     (MT_S32)stResumeValue.u32Pdcbcr);

#if 0
		JPEG_TRACE("===============================================================\n");
		JPEG_TRACE("pJpegHandle->u32ResByteConsum = %d\n",pJpegHandle->u32ResByteConsum);
		JPEG_TRACE("stResumeValue.u32ResumeData0  = %d\n",stResumeValue.u32ResumeData0);
		JPEG_TRACE("stResumeValue.u32ResumeData1  = %d\n",stResumeValue.u32ResumeData1);
		JPEG_TRACE("stResumeValue.u32ResBitRemain = %d\n",stResumeValue.u32ResBitRemain);
		JPEG_TRACE("stResumeValue.u32ResMcuy      = %d\n",stResumeValue.u32ResMcuy);
		JPEG_TRACE("stResumeValue.u32Pdy          = %d\n",stResumeValue.u32Pdy);
		JPEG_TRACE("stResumeValue.u32Pdcbcr       = %d\n",stResumeValue.u32Pdcbcr);
		JPEG_TRACE("===============================================================\n");
#endif
}
#endif /** END IF  CONFIG_JPEG_SUSPEND **/
