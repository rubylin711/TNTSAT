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


#include "mt_jpeglib.h"
#include "jpegint.h"
#include "jdatasrc.h"

#include "mt_type.h"
#include "jpeg_hdec_adp.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_rwreg.h"
#include "mt_jpeg_config.h"
#include "mt_jpeg_hal_api.h"
#include "mt_drv_jpeg_reg.h"
#include "jpeg_hdec_mem.h"
#include "mt_module_debug.h"
//#include "mt_jpeg_hdec_test.h"

#ifdef CONFIG_JPEG_SUSPEND
#include "jpeg_hdec_suspend.h"
#endif

#ifdef CONFIG_JPEG_TEST_HARD_DEC_CAPA
#include <sys/time.h>
#endif

#if defined(CONFIG_JPEG_ANDROID_DEBUG_ENABLE) && defined(CONFIG_JPEG_DEBUG_INFO)
#define LOG_TAG "libjpeg"
#endif

/***************************** Macro Definition ******************************/
#define JPEG_DUMP_REG_ENABLE 0

#define JPG_RESUME_VALUE     0x01
#define JPG_EOF_VALUE        0x02
#define JPG_TIMEOUT_MS				1000

#ifdef CONFIG_JPEG_TEST_HARD_DEC_CAPA
#define HARDDEC_TINIT()   struct timeval tv_start, tv_end; unsigned int time_cost,line_start
#define HARDDEC_TSTART()  gettimeofday(&tv_start, NULL);line_start = __LINE__
#define HARDDEC_TEND()     \
gettimeofday(&tv_end, NULL); \
time_cost = ((tv_end.tv_usec - tv_start.tv_usec) + (tv_end.tv_sec - tv_start.tv_sec)*1000000); \
JPEG_TRACE("=============================================================================\n"); \
JPEG_TRACE("FROM LINE: %d TO LINE: %d COST: %d us\n",line_start, __LINE__, time_cost);         \
JPEG_TRACE("=============================================================================\n")
#endif

/******************** to see wmtch include file we want to use***************/



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/

#ifdef CONFIG_JPEG_TEST_ALL_DEC_RANDOM_RESET
static MT_S32 sg_s32ResetNum = 0;
#endif


/******************************* API forward declarations *******************/
MT_S32 JPEG_HDEC_SendStreamFromPhyMemInterleave(j_decompress_ptr cinfo);
MT_S32 JPEG_HDEC_SendStreamFromPhyMemNonInterleave(j_decompress_ptr cinfo);
extern void start_pass_huff_decoder (j_decompress_ptr cinfo);
extern void jcodec_set_tab(j_decompress_ptr p_cinfo, MT_BOOL auto_flag);
MT_S32 JPEG_HDEC_SofthuffStart(j_decompress_ptr cinfo);
extern MT_VOID JPEG_HDEC_SetIdctBuf(const struct jpeg_decompress_struct *cinfo, MT_U32 comp_id);
extern int read_markers (j_decompress_ptr cinfo);

/*****************************************************************************
* func			: JPEG_HDEC_CheckOut
* description	: check whether should memcpy to output buffer
				  CNcomment:  判断是否需要输出到输出buffer中    CNend\n
* param[in]	    : *pJpegHandle   CNcomment: 解码器句柄          CNend\n
* retval		: MT_SUCCESS  CNcomment:  成功		   CNend\n
* retval		: MT_FAILURE  CNcomment:  失败		   CNend\n
* others:		: NA
*****************************************************************************/
static MT_S32 JPEG_HDEC_CheckOut(const JPEG_HDEC_HANDLE_S *pJpegHandle)
{
#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(   (MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)
		    &&((MT_TRUE == pJpegHandle->bOutYCbCrSP) ||(MT_TRUE == pJpegHandle->bDecARGB)))
#else
		if(  (MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)
		   &&(MT_TRUE == pJpegHandle->bOutYCbCrSP))
#endif
		{
			return MT_FAILURE;
		}

#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if((MT_TRUE != pJpegHandle->bOutYCbCrSP) && (MT_TRUE != pJpegHandle->bDecARGB))
#else
		if(MT_TRUE != pJpegHandle->bOutYCbCrSP)
#endif
		{
			return MT_FAILURE;
		}

		return MT_SUCCESS;

}

/*****************************************************************************
* func			: JPEG_HDEC_GetIntStatus
* description	: get interrupt status
				  CNcomment:  获取中断状态             CNend\n
* param[in]	    : *pJpegHandle   CNcomment: 解码器句柄 CNend\n
* retval		: MT_SUCCESS  CNcomment:  成功		   CNend\n
* retval		: MT_FAILURE  CNcomment:  失败		   CNend\n
* others:		: NA
*****************************************************************************/
static MT_S32 JPEG_HDEC_GetIntStatus(const JPEG_HDEC_HANDLE_S *pJpegHandle, JPG_INTTYPE_E *pIntType, MT_U32 u32TimeOut)
{

		MT_S32 s32RetVal;
		JPG_GETINTTYPE_S GetIntType;

		GetIntType.IntType = JPG_INTTYPE_NONE;
		GetIntType.TimeOut = u32TimeOut;

		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_GETINTSTATUS, &GetIntType);

		if (MT_SUCCESS != s32RetVal)
		{
			return s32RetVal;
		}
		*pIntType = GetIntType.IntType;
		return MT_SUCCESS;
    
}
/*****************************************************************************
* func			: JPEG_HDEC_SuspendDispose
* description	: dispose the suspend
				  CNcomment:  待机处理	 CNend\n
* param[in] 	: cinfo 	          CNcomment:  解码对象	           CNend\n
* param[in] 	: pbResumeOk     	  CNcomment:  待机已经唤醒         CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
#if 0
static MT_VOID JPEG_HDEC_SuspendDispose(const struct jpeg_decompress_struct *cinfo,MT_BOOL *pbResumeOk)
{
#ifdef CONFIG_JPEG_SUSPEND
		MT_BOOL bSuspendSingal   = MT_FALSE;
		MT_BOOL bResumeSingal    =  MT_FALSE;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		JPEG_HDEC_GetSuspendSignal(pJpegHandle,&bSuspendSingal);
		JPEG_HDEC_GetResumeSignal(pJpegHandle,&bResumeSingal);
		if( (MT_TRUE == bSuspendSingal) ||  (MT_TRUE == bResumeSingal))
		{
			JPEG_HDEC_Resume(cinfo);
			*pbResumeOk = MT_TRUE;
#if defined(CONFIG_JPEG_FPGA_TEST_ENABLE) && defined(CONFIG_JPEG_FPGA_TEST_SUSPEND_ENABLE) && defined(CONFIG_JPEG_SUSPEND)
			JPEG_TRACE("===================================================\n");
			JPEG_TRACE("模拟待机生效\n");
			JPEG_TRACE("===================================================\n");
#endif
		}
#endif
}
#endif
#if JPEG_DUMP_REG_ENABLE
static MT_VOID JPEG_DUMP_REG(volatile mt_char *pJpegRegVirAddr)
{
	MT_U32 i, tmp;
	for(i = 0;i < 0xfc; i+=4)
	{
		tmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegRegVirAddr,JCODEC_BASE_ADDR + i);
		printf("\r\n 0x%08x=0x%08x", JCODEC_BASE_ADDR + i, tmp);
	}
}
#endif


/*****************************************************************************
* func			: JPEG_HDEC_SendStreamFromPhyMem
* description	: get the stream from physics memory
				  CNcomment:  码流来源连续的物理内存的处理方式	 CNend\n
* param[in] 	: cinfo 	  CNcomment:  解码对象	   CNend\n
* retval		: MT_SUCCESS  CNcomment:  成功		   CNend\n
* retval		: MT_FAILURE  CNcomment:  失败		   CNend\n
* others:		: NA
*****************************************************************************/

#ifdef CONFIG_JPEG_STREAMBUF_4ALIGN
/**
 ** before 3716CV200 EC,the save buffer should 4bytes align
 ** CNcomment:3716CV200 EC之前的存储码流buffer起始地址需要4字节对齐 CNend\n
 **/
MT_S32 JPEG_HDEC_SendStreamFromPhyMem(j_decompress_ptr cinfo)
{

		JPG_INTTYPE_E eIntStatus        = JPG_INTTYPE_NONE;
		MT_S32 s32Cnt                    = 0;
		MT_U32 u32ConsumSize            = 0;
		MT_U64 u64LeaveSize             = 0;
		MT_U32 u32AlignSize             = 0;
		MT_U32 u32DecSize               = 0;
		MT_BOOL bStartDec               = MT_TRUE;
		MT_CHAR* pDataPhyAddr           = NULL;
		MT_CHAR* pDataVirAddr           = NULL;
		MT_CHAR* pSaveStreamPhyAddr     = NULL;
		MT_CHAR* pSaveStreamVirAddr     = NULL;
		MT_CHAR* pSaveStreamEndPhyAddr  = NULL;

		MT_S32 s32Ret = MT_SUCCESS;

		MT_BOOL bReachEOF = MT_FALSE;
		MT_BOOL b4Align   = MT_FALSE;/**< whether 4bytes align *//**<CNcomment:是否4字节对齐*/

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);


#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		/**
		** output the scen message to file
		** CNcomment:导解码现场 CNend\n
		**/
		if(MT_TRUE == pJpegHandle->bSaveScen)
		{
			MT_JPEG_OpenScenFile(cinfo);
		}
#endif

		/**
		** calc the consume stream size
		** CNcomment:进入硬件之前消耗的码流数等于每次读码流累加-4096中剩余的码流数 CNend\n
		**/	 
		u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
		u64LeaveSize  = pJpegHandle->stHDecDataBuf.u64DataSize - ((MT_U64)u32ConsumSize);

		pDataPhyAddr          = pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
		pDataVirAddr          = pJpegHandle->stHDecDataBuf.pDataVirBuf + u32ConsumSize;
		pSaveStreamPhyAddr    = pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
		pSaveStreamVirAddr    = pJpegHandle->stHDecDataBuf.pDataVirBuf + u32ConsumSize;
		pSaveStreamEndPhyAddr = pJpegHandle->stHDecDataBuf.pDataPhyBuf + pJpegHandle->stHDecDataBuf.u64DataSize;
		if(0 == (ulong)pDataPhyAddr % JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN)
		{
			b4Align = MT_TRUE;
		}
		else
		{
			/**
			**第一次解码JPGD_HDEC_MMZ_ALIGN_16BYTES + 4字节对齐要解的码流大小，因为要是码流太少解不了
			**/
			pSaveStreamPhyAddr = (MT_CHAR*)(((ulong)pSaveStreamPhyAddr + 16 + JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1) & (~(JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1)));
			pSaveStreamVirAddr = (MT_CHAR*)(((ulong)pSaveStreamVirAddr + 16 + JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1) & (~(JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1)));
			u32AlignSize       = (ulong)pSaveStreamPhyAddr - (ulong)pDataPhyAddr;
		}
		if(u64LeaveSize <= u32AlignSize)
		{
			u32DecSize  = (MT_U32)u64LeaveSize;
			bReachEOF   = MT_TRUE;
		}
		else
		{
			u32DecSize  = u32AlignSize;
			bReachEOF   = MT_FALSE;
		}

		if(MT_FALSE == b4Align)
		{
			memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,pDataVirAddr,u32DecSize);
			/** 刷码流数据 **/
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,(ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
#else
			s32Ret = MT_GFX_Flush((ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
#endif
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}
		}

#ifdef CONFIG_JPEG_TEST_HARD_DEC_CAPA
		HARDDEC_TINIT();
		HARDDEC_TSTART();
#endif

		do
		{

			if(MT_TRUE == bStartDec && MT_FALSE == b4Align)
			{
#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
				if(MT_TRUE == pJpegHandle->bSaveScen)
				{
				   /**
					** should save scen before start decode
					** CNcomment:要在解码启动前保存现场，否则解码会失败 CNend\n
					**/
					MT_JPEG_OutScenData(cinfo,pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf,pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32DecSize,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,(MT_U64)u32DecSize,MT_TRUE);
				}
#endif
				//JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADDR,(MT_S32 )(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf - JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN));
		        //JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADDR,(MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32DecSize + JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32DecSize));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(bReachEOF ? JPG_EOF_VALUE : 0x0));
#ifdef CONFIG_JPEG_4KDDR_DISABLE
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x5);
#else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x1);
#endif
			}
			else if(MT_TRUE == bStartDec && MT_TRUE == b4Align)
			{
#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
				if(MT_TRUE == pJpegHandle->bSaveScen)
				{  /**
					** should save scen before start decode
					** CNcomment:要在解码启动前保存现场，否则解码会失败 CNend\n
					**/
					MT_JPEG_OutScenData(cinfo,pSaveStreamPhyAddr,pSaveStreamEndPhyAddr,pSaveStreamVirAddr,(MT_U64)u32DecSize,MT_TRUE);
				}
#endif
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)pSaveStreamPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)pSaveStreamEndPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME, 0x2);
#ifdef CONFIG_JPEG_4KDDR_DISABLE
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x5);
#else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x1);
#endif
			}
			else
			{
#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
				if(MT_TRUE == pJpegHandle->bSaveScen)
				{  /**
					** should save scen before start decode
					** CNcomment:要在解码启动前保存现场，否则解码会失败 CNend\n
					**/
					MT_JPEG_OutScenData(cinfo,pSaveStreamPhyAddr,pSaveStreamEndPhyAddr,pSaveStreamVirAddr,(MT_U64)(pSaveStreamEndPhyAddr - pSaveStreamPhyAddr),MT_FALSE);
				}
#endif
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)pSaveStreamPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)pSaveStreamEndPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(JPG_EOF_VALUE|JPG_RESUME_VALUE));
			}

			bStartDec  = MT_FALSE;
			bReachEOF  = MT_TRUE;

			eIntStatus = JPG_INTTYPE_ERROR;
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, 100000);
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}
			if(JPG_INTTYPE_ERROR == eIntStatus) 
			{
				goto FAIL;
			}
			else if(JPG_INTTYPE_FINISH == eIntStatus) 
			{  
				break;
			}
			else if(JPG_INTTYPE_CONTINUE == eIntStatus)
			{
				//JPEG_HDEC_SetStreamBuf(cinfo);
				continue;
			}
			else 
			{
				goto FAIL;
			}

		}wmtle(JPG_INTTYPE_FINISH != eIntStatus);

#ifdef CONFIG_JPEG_TEST_HARD_DEC_CAPA
		HARDDEC_TEND();
		/** mtmd.l 0xf8c40018 **/
		/** 残差的值 mtmd.l 0xf8c40008 **/
#endif

		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row           = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)&0xffff);
		cinfo->MCU_rows_in_scan       = ((MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)>>16)&0xffff);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membersmtp[s32Cnt] = s32Cnt;
		}

		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
			MT_JPEG_CloseScenFile(cinfo);
#endif
			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}

#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		MT_JPEG_CloseScenFile(cinfo);
#endif

		return MT_SUCCESS;

		FAIL:

#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		if(MT_TRUE == pJpegHandle->bSaveScen)
		{
			MT_JPEG_CloseScenFile(cinfo);
		}
#endif
		return MT_FAILURE;

}

MT_S32 JPEG_HDEC_SendStreamFromReturnPhyMem(j_decompress_ptr cinfo)
{

		JPG_INTTYPE_E eIntStatus        = JPG_INTTYPE_NONE;
		MT_S32 s32Cnt                    = 0;
		MT_U32 u32ConsumSize            = 0;
		MT_U64 u64LeaveSize             = 0;
		MT_U32 u32AlignSize             = 0;
		MT_U32 u32DecSize               = 0;
		MT_BOOL bStartDec               = MT_TRUE;
		MT_BOOL bUseReturn              = MT_FALSE; /** 使用码流折回 **/
		phys_addr_t pDataPhyAddr           = 0;
		phys_addr_t pDataStartPhyAddr      = 0;
		MT_CHAR* pDataVirAddr           = NULL;
		phys_addr_t pSaveStreamPhyAddr     = 0;
		MT_CHAR* pSaveStreamVirAddr     = NULL;
		phys_addr_t pSaveStreamEndPhyAddr  = 0;

		MT_S32 s32Ret = MT_SUCCESS;

		MT_BOOL bReachEOF = MT_FALSE;
		MT_BOOL bNeedContinue = MT_FALSE;
		
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		/**
		** calc the consume stream size
		** CNcomment:进入硬件之前消耗的码流数等于每次读码流累加-4096中剩余的码流数 CNend\n
		**/
		if(pJpegHandle->stHDecDataBuf.u32ConsumeDataSize <= pJpegHandle->stHDecDataBuf.u64DataSize)
		{/** 剩余码流拿去copy然后解码 **/
			u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
			pDataVirAddr  = pJpegHandle->stHDecDataBuf.pDataVirBuf + u32ConsumSize;
			pDataPhyAddr  = pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
			u32DecSize    = pJpegHandle->stHDecDataBuf.u64DataSize - u32ConsumSize;
			if(u32DecSize > (MT_U32)(JPGD_HARD_BUFFER))
			{
				pDataStartPhyAddr = (phys_addr_t)(((phys_addr_t)pDataPhyAddr + JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN + JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN - 1) & (~(JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN - 1)));
				u32DecSize = pDataStartPhyAddr - pDataPhyAddr;
				bUseReturn = MT_TRUE;
			}
			memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,pDataVirAddr,u32DecSize);
			#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,(ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
			#else
			s32Ret = MT_GFX_Flush((ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
			#endif
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}
			if(MT_FALSE == bUseReturn)
			{
				pSaveStreamPhyAddr    = pJpegHandle->stHDecDataBuf.pStartBufPhy;
			}
			else
			{/** 使用码流折回 **/
				pSaveStreamPhyAddr = pDataStartPhyAddr;
			}
			pSaveStreamEndPhyAddr = pSaveStreamPhyAddr + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			bNeedContinue = MT_TRUE;
			bReachEOF   = MT_FALSE;
		}
		else
		{
			u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - pJpegHandle->stHDecDataBuf.u64DataSize - cinfo->src->bytes_in_buffer;
			u64LeaveSize  = pJpegHandle->stHDecDataBuf.s32StreamReturnLen - ((MT_U64)u32ConsumSize);
	
			pDataPhyAddr          = pJpegHandle->stHDecDataBuf.pStartBufPhy + u32ConsumSize;
			pDataVirAddr          = pJpegHandle->stHDecDataBuf.pDataVirBufReturn + u32ConsumSize;
			pSaveStreamPhyAddr    = pJpegHandle->stHDecDataBuf.pStartBufPhy + u32ConsumSize;
			pSaveStreamVirAddr    = pJpegHandle->stHDecDataBuf.pDataVirBufReturn + u32ConsumSize;
			pSaveStreamEndPhyAddr = pJpegHandle->stHDecDataBuf.pStartBufPhy + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			if(0 == pDataPhyAddr % JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN)
			{
				bNeedContinue = MT_FALSE;
			}
			else
			{
				pSaveStreamPhyAddr = (((phys_addr_t)pSaveStreamPhyAddr + 16 + JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1) & (~(JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1)));
				pSaveStreamVirAddr = (MT_CHAR*)(((ulong)pSaveStreamVirAddr + 16 + JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1) & (~(JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN - 1)));
				u32AlignSize       = pSaveStreamPhyAddr - pDataPhyAddr;
				bNeedContinue = MT_TRUE;
			}
			if(u64LeaveSize <= u32AlignSize)
			{
				u32DecSize  = (MT_U32)u64LeaveSize;
				bReachEOF   = MT_TRUE;
			}
			else
			{
				u32DecSize  = u32AlignSize;
				bReachEOF   = MT_FALSE;
			}

			if(MT_TRUE == bNeedContinue)
			{
				memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,pDataVirAddr,u32DecSize);
				/** 刷码流数据 **/
				#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
				s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,(ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
				#else
				s32Ret = MT_GFX_Flush((ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
				#endif
				if(MT_SUCCESS != s32Ret)
				{
					goto FAIL;
				}
			}
		}
		do
		{

			if(MT_TRUE == bStartDec && MT_TRUE == bNeedContinue)
			{
				//JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADDR,(MT_S32 )(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf - JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN));
				//JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADDR,(MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32DecSize + JPGD_HDEC_MMZ_YUVSP_BUFFER_ALIGN));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)(pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32DecSize));
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(bReachEOF ? JPG_EOF_VALUE : 0x0));
#ifdef CONFIG_JPEG_4KDDR_DISABLE
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x5);
#else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x1);
#endif

			}
			else if(MT_TRUE == bStartDec && MT_FALSE == bNeedContinue)
			{
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)pSaveStreamPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)pSaveStreamEndPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME, 0x2);
#ifdef CONFIG_JPEG_4KDDR_DISABLE
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x5);
#else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x1);
#endif

			}
			else
			{
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)pSaveStreamPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)pSaveStreamEndPhyAddr);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(JPG_EOF_VALUE|JPG_RESUME_VALUE));
			}

			bStartDec  = MT_FALSE;
			bReachEOF  = MT_TRUE;

			eIntStatus = JPG_INTTYPE_ERROR;
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}
			if(JPG_INTTYPE_ERROR == eIntStatus) 
			{
				goto FAIL;
			}
			else if(JPG_INTTYPE_FINISH == eIntStatus) 
			{  
				break;
			}
			else if(JPG_INTTYPE_CONTINUE == eIntStatus)
			{/** 重新配置码流buffer地址寄存器 **/
				//JPEG_HDEC_SetStreamBuf(cinfo);
				continue;
			}
			else 
			{
				goto FAIL;
			}

		}wmtle(JPG_INTTYPE_FINISH != eIntStatus);
		
		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row           = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)&0xffff);
		cinfo->MCU_rows_in_scan       = ((MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)>>16)&0xffff);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membersmtp[s32Cnt] = s32Cnt;
		}

		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}

		return MT_SUCCESS;

		FAIL:
			
		return MT_FAILURE;

}

#else

extern phys_addr_t get_tar_addr(phys_addr_t  base_addr, MT_U32 h, MT_U32 stride, MT_U32 w, MT_U32 k, MT_JPEG_ROT_MOD_E rot_mode);
MT_S32 JPEG_HDEC_SendStreamFromPhyMem(j_decompress_ptr cinfo)
{
    if(cinfo->comps_in_scan > 1)
        return JPEG_HDEC_SendStreamFromPhyMemInterleave(cinfo);
    else
        return JPEG_HDEC_SendStreamFromPhyMemNonInterleave(cinfo);
}
#ifdef CONFIG_MT_FPGA_GPE
#define TEST_JPG_RESET
#endif

MT_S32 MT_JPEG_SET_TEST_INFO(j_decompress_ptr cinfo, MT_U32 value)
{
#ifdef TEST_JPG_RESET
    JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    pJpegHandle->s32ClientData = value;
#endif    
	return MT_SUCCESS;
}

static mt_void JPEG_HDEC_SaveRegs(j_decompress_ptr cinfo, MT_U32 comp_id)
{
  MT_U32 dtmp = 0;
  JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][0].u32RegAddr = JCODEC_START;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][0].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_EN);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][1].u32RegAddr = JCODEC_EN;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][1].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][2].u32RegAddr = JCODEC_HUFF_MCU_CFG0;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][2].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG1);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][3].u32RegAddr = JCODEC_HUFF_MCU_CFG1;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][3].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_SIZE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][4].u32RegAddr = JCODEC_PIC_SIZE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][4].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_RI_MCU);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][5].u32RegAddr = JCODEC_PIC_RI_MCU;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][5].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][6].u32RegAddr = JCODEC_PIC_TYPE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][6].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_SWAP_MOD);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][7].u32RegAddr = JCODEC_BS_SWAP_MOD;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][7].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BUF_START_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][8].u32RegAddr = JCODEC_BUF_START_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][8].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BUF_END_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][9].u32RegAddr = JCODEC_BUF_END_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][9].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BUF_CUR_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][10].u32RegAddr = JCODEC_BUF_CUR_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][10].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_START_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][11].u32RegAddr = JCODEC_BS_START_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][11].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_END_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][12].u32RegAddr = JCODEC_BS_END_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][12].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_LU_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][13].u32RegAddr = JCODEC_PIC_LU_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][13].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_LU_STRIDE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][14].u32RegAddr = JCODEC_PIC_LU_STRIDE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][14].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CB_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][15].u32RegAddr = JCODEC_PIC_CB_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][15].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CR_ADDR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][16].u32RegAddr = JCODEC_PIC_CR_ADDR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][16].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CH_STRIDE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][17].u32RegAddr = JCODEC_PIC_CH_STRIDE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][17].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_STATE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][18].u32RegAddr = JCODEC_STATE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][18].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_ERR);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][19].u32RegAddr = JCODEC_PIC_ERR;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][19].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_INT_STATE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][20].u32RegAddr = JCODEC_INT_STATE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][20].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_INT_EN);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][21].u32RegAddr = JCODEC_INT_EN;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][21].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_EN);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][22].u32RegAddr = JCODEC_CFG_EN;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][22].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CFG_PORT);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][23].u32RegAddr = JCODEC_CFG_PORT;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][23].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_START);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][24].u32RegAddr = JCODEC_CLIP_START;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][24].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_END);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][25].u32RegAddr = JCODEC_CLIP_END;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][25].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_QUANT_COEF_CNT);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][26].u32RegAddr = JCODEC_QUANT_COEF_CNT;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][26].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_CTRL);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][27].u32RegAddr = JCODEC_AXI_CTRL;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][27].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_STATUS);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][28].u32RegAddr = JCODEC_AXI_STATUS;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][28].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_DEC_CLIP);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][29].u32RegAddr = JCODEC_DEC_CLIP;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][29].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_CODE_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][30].u32RegAddr = JCODEC_LU_DC_MAX_CODE_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][30].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_CODE_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][31].u32RegAddr = JCODEC_LU_AC_MAX_CODE_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][31].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_CODE_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][32].u32RegAddr = JCODEC_CH_DC_MAX_CODE_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][32].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_CODE_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][33].u32RegAddr = JCODEC_CH_AC_MAX_CODE_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][33].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_DC_MAX_ADDR_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][34].u32RegAddr = JCODEC_LU_DC_MAX_ADDR_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][34].u32RegVal = dtmp;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_LU_AC_MAX_ADDR_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][35].u32RegAddr = JCODEC_LU_AC_MAX_ADDR_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][35].u32RegVal = dtmp;  
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_DC_MAX_ADDR_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][36].u32RegAddr = JCODEC_CH_DC_MAX_ADDR_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][36].u32RegVal = dtmp;  
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CH_AC_MAX_ADDR_BASE);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][37].u32RegAddr = JCODEC_CH_AC_MAX_ADDR_BASE;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][37].u32RegVal = dtmp;  
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_DCT_COEF);
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][38].u32RegAddr = JCODEC_DCT_COEF;
  pJpegHandle->stJpegSofInfo.stJpgReg[comp_id][38].u32RegVal = dtmp;       
}


MT_S32 JPEG_HDEC_SendStreamFromPhyMemInterleave(j_decompress_ptr cinfo)
{  
		JPG_INTTYPE_E eIntStatus         = JPG_INTTYPE_NONE;
		MT_S32 s32Cnt                    = 0;
		MT_U32 u32ConsumSize             = 0;
		phys_addr_t pSaveStreamPhyAddr     = 0;
		phys_addr_t pSaveStreamEndPhyAddr  = 0;

		MT_S32 s32Ret = MT_SUCCESS;
        MT_U32 dtmp = 0;
        jpeg_component_info *compptr = cinfo->comp_info;  
        MT_U32 huff_tab_fix = 0;
        MT_U32 iq_tab_fix = 0;
        MT_U32 mcu_blk_sel = 0;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
        phys_addr_t u32CurAddrPhy = 0;
        ulong   u32CurAddrVir = 0;
        MT_U8 *p_ptr = NULL;		
		ulong pDataVirBuf;
		phys_addr_t pDataPhyBuf;
//		MT_U32 u64DataSize;

              
		/**
		** no alloc the stream buffer, we can set in Makefile about JPGD_HARD_BUFFER
		** CNcomment:这时候不用自己分配码流buffer了，可以在Makefile中设置JPGD_HARD_BUFFER = 0 CNend\n
		**/

		/**
		** calc the consume stream size
		** CNcomment:进入硬件之前消耗的码流数等于每次读码流累加-4096中剩余的码流数 CNend\n
		**/
		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bNeedStreamReturn)
		{
			if(pJpegHandle->stHDecDataBuf.u32ConsumeDataSize <= pJpegHandle->stHDecDataBuf.u64DataSize)
			{
				u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
				pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
				pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pStartBufPhy + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			}
			else
			{
				u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - pJpegHandle->stHDecDataBuf.u64DataSize - cinfo->src->bytes_in_buffer;
				pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pStartBufPhy + u32ConsumSize;
				pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pStartBufPhy + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			}
		}
		else
		{
			u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
			pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
			pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + pJpegHandle->stHDecDataBuf.u64DataSize;
		}

		pDataVirBuf = (ulong)pJpegHandle->stHDecDataBuf.pDataVirBuf;
		pDataPhyBuf = (phys_addr_t)pJpegHandle->stHDecDataBuf.pDataPhyBuf;
//		u64DataSize = (MT_U32)pJpegHandle->stHDecDataBuf.u64DataSize;

		/**
		** start the decode, no need resume
		** CNcomment:直接启动解码，不需要续码流 CNend\n
		**/
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pSaveStreamPhyAddr);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pSaveStreamEndPhyAddr - 1);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);   
              JPEG_HDEC_SaveRegs(cinfo, 0);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		
//			  printf("\r\n pSaveStreamPhyAddr:0x%08x, pSaveStreamEndPhyAddr:0x%08x", pSaveStreamPhyAddr,pSaveStreamEndPhyAddr);

#ifdef CONFIG_MT_FPGA_GPE //#ifdef TEST_JPG_RESET
            MT_S32 s32RetVal = 0;
        	if(cinfo->p_jpeg_dbg_info->reset_test_enable &&  (1 == cinfo->p_jpeg_dbg_info->reset_test_type) && cinfo->p_jpeg_dbg_info->reset_test)
        	{
                while(!(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_STATE) & 0x1))
                {
                    MT_U32 tmp = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_STATE);
                    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_STATE, tmp & 0xfffffffe);

                    if(pJpegHandle->s32ClientData == 0x1111) //reset test when running
                    {                       
                        if(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BUF_CUR_ADDR) >=
                            (JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_END_ADDR) 
                            - JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_START_ADDR)) / 2
                            + JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BS_START_ADDR))
                        {
                            //disable jpeg dec
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0);

                          //stop jpeg  
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_CTRL, 1);
                            tmp = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_STATUS);
                            while(0x1 != (tmp & 0x1))  //AXI_BUS_IDLE
                            {
                              tmp = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_STATUS);
                            }
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_CTRL, 0);
    
                            //reset jpeg
                            s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_RESET);
                             if(s32RetVal != 0)
                            {
                                 printf("\r\n reset jpeg when running: reset fail\n");
                           }
                            MT_INFO_JPEG("\r\n reset jpeg when running");
                            break;
                        }
                    }

                  if((tmp & (1 << 8)) == (1 << 8))
                  {
                        u32CurAddrPhy = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1;
                        u32CurAddrVir = (ulong)(pJpegHandle->stHDecDataBuf.pDataVirBuf) + u32CurAddrPhy - pJpegHandle->stHDecDataBuf.pDataPhyBuf;                        
                        p_ptr = (MT_U8 *)u32CurAddrVir;
                        if(*p_ptr != MARKER_EOI)
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
                        }
                        else
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);         
                        }
                  }
              }      
         }
          else
#endif
        {
		eIntStatus = JPG_INTTYPE_ERROR;
		do
		{
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
			
			if(MT_SUCCESS != s32Ret)
			{
				if(JPG_INTTYPE_CONTINUE == (eIntStatus & JPG_INTTYPE_CONTINUE) && u32ConsumSize > 0)
				{/*we comsumed some data, but no interrupt & no err, maybe the jpg is incomplete*/
						u32CurAddrPhy = (ulong)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
					if(u32CurAddrPhy < pDataPhyBuf)
							u32CurAddrPhy = (ulong)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
					u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;
					p_ptr = (MT_U8 *)u32CurAddrVir;
					if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT)){
						JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);
						/*some bad jpg has no end, stop scan, do not return err & show part of the picture as in windows PC*/
						break;
					}
				}
				goto FAIL;
			} 
			if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
			{
			    u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1;
				if(u32CurAddrPhy < pDataPhyBuf)
				    u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				//u32CurAddrPhy += (MT_U32)pSaveStreamEndPhyAddr - (MT_U32)pSaveStreamPhyAddr;
				u32CurAddrVir = pDataVirBuf +(ulong) (u32CurAddrPhy - pDataPhyBuf);   

				p_ptr = (MT_U8 *)u32CurAddrVir;
				if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				{
					JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				}
				else
				{
					JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);      
				}
			}   			
		}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));
        }

			   //JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);

               //jpg_buf_clr
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);

#ifdef CONFIG_MT_FPGA_GPE
            if(cinfo->p_jpeg_dbg_info->reset_test_enable && (cinfo->p_jpeg_dbg_info->reset_test_type == 0))
            {
	            if(pJpegHandle->s32ClientData == 0x2222) //reset test when running
	            {
	                MT_U32 tmp;
	              //stop jpeg  
	                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_CTRL, 1);
	                tmp = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_STATUS);
	                while(0x1 != (tmp & 0x1))  //AXI_BUS_IDLE
	                {
	                  tmp = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_STATUS);
	                }
	                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_AXI_CTRL, 0);                
	                //reset jpeg
	                s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_RESET);
                       if(s32RetVal != 0)
                        {
                             printf("\r\n reset jpeg after dec finish: reset fail\n");
                       }
	                printf("\r\n reset jpeg after dec finish");
	            }
            }
#endif

              if(cinfo->jpeg_color_space == JCS_RGB)
              {     
                //------------------G 
                dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                dtmp |= (RGB_G & 0x3f);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);
                
                huff_tab_fix = 0xf;
                if(compptr[1].quant_tbl_no == 0)
                    iq_tab_fix = 0x8;
                else if(compptr[1].quant_tbl_no == 1)
                    iq_tab_fix = 0x9;
                else
                    iq_tab_fix = 0xa;
                mcu_blk_sel = 0x9;
                dtmp = (MT_U32)(0xff << 24) | (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

                MT_INFO_JPEG("\r\n G~~~~~~~~~ quant_tbl_no:%d, dc_tbl_no:%d, ac_tbl_no:%d", 
                compptr[1].quant_tbl_no, compptr[1].dc_tbl_no, compptr[1].ac_tbl_no);  
                    
                JPEG_HDEC_SaveRegs(cinfo, 1);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		


    		eIntStatus = JPG_INTTYPE_ERROR;
			do
			{
	    		s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
	    		if(MT_SUCCESS != s32Ret)
	    		{
	    			goto FAIL;
	    		}
	            if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
	            {
	                u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                      if(u32CurAddrPhy < pDataPhyBuf)
                      {
                            u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
                      }
	                u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;                        
	                p_ptr = (MT_U8 *)u32CurAddrVir;
	                if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
	                {
	                    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
	                }
	                else
	                {
	                    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);      
	                }
	              }			
			  }while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));
               //jpg_buf_clr
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);

                //------------------------B
                dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                dtmp |= (RGB_B & 0x3f);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);                
                
                huff_tab_fix = 0xf;
                if(compptr[2].quant_tbl_no == 0)
                    iq_tab_fix = 0x8;
                else if(compptr[2].quant_tbl_no == 1)
                    iq_tab_fix = 0x9;
                else
                    iq_tab_fix = 0xa;
                mcu_blk_sel = 0xa;
                dtmp = (MT_U32)(0xff << 24) | (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

                MT_INFO_JPEG("\r\n B~~~~~~~~~ quant_tbl_no:%d, dc_tbl_no:%d, ac_tbl_no:%d", 
                    compptr[2].quant_tbl_no, compptr[2].dc_tbl_no, compptr[2].ac_tbl_no);  
                JPEG_HDEC_SaveRegs(cinfo, 2);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		


       		eIntStatus = JPG_INTTYPE_ERROR;
			do
			{
				s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
				if(MT_SUCCESS != s32Ret)
				{
					goto FAIL;
				}
				if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
				{
				  u32CurAddrPhy = (ulong)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                  if(u32CurAddrPhy < pDataPhyBuf)
                      u32CurAddrPhy = (ulong)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				  u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;						
				  p_ptr = (MT_U8 *)u32CurAddrVir;
				  if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				  }
				  else
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);	   
				  }
				}		  
			}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));
                   
                  //jpg_buf_clr
                 JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);
              }
              else if((cinfo->jpeg_color_space == JCS_CMYK) || 
                (cinfo->jpeg_color_space == JCS_YCCK))
              {
                  //------------------------M
                dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                dtmp |= (CMYK_M & 0x3f);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);
                
                           
                huff_tab_fix = 0xf;
                if(compptr[1].quant_tbl_no == 0)
                    iq_tab_fix = 0x8;
                else if(compptr[1].quant_tbl_no == 1)
                    iq_tab_fix = 0x9;
                else
                    iq_tab_fix = 0xa;
                mcu_blk_sel = 0x9;
                dtmp = (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

                MT_INFO_JPEG("\r\n M~~~~~~~~~ quant_tbl_no:%d, dc_tbl_no:%d, ac_tbl_no:%d", 
                compptr[1].quant_tbl_no, compptr[1].dc_tbl_no, compptr[1].ac_tbl_no);  
                
                JPEG_HDEC_SaveRegs(cinfo, 1);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		


       		eIntStatus = JPG_INTTYPE_ERROR;
			do
			{
	       		s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
	       		if(MT_SUCCESS != s32Ret)
	       		{
	       			goto FAIL;
	       		}
				if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
				{
				  u32CurAddrPhy = (ulong)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                  if(u32CurAddrPhy < pDataPhyBuf)
                      u32CurAddrPhy = (ulong)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				  u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;						
				  p_ptr = (MT_U8 *)u32CurAddrVir;
				  if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				  }
				  else
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);	   
				  }
				}		  
			}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));

                   
                  //jpg_buf_clr
                 JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);
               

               //------------------------Y
                dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                dtmp |= (CMYK_Y & 0x3f);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);                 


                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,   
                	             JCODEC_PIC_CH_STRIDE, 				 
                	             (phys_addr_t)pJpegHandle->stJpegSofInfo.u32COutStride);      

                dtmp = get_tar_addr((phys_addr_t)pJpegHandle->stMiddleSurface.pMiddlePhy[1], 
                    pJpegHandle->stJpegSofInfo.u32YOutHeight, 
                    pJpegHandle->stJpegSofInfo.u32COutStride,
                    pJpegHandle->stJpegSofInfo.u32YOutWidth, 
                    2,
                    pJpegHandle->out_rot_mod);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_CB_ADDR, (MT_S32)dtmp);   

                
                huff_tab_fix = 0xf;
                if(compptr[2].quant_tbl_no == 0)
                    iq_tab_fix = 0x8;
                else if(compptr[2].quant_tbl_no == 1)
                    iq_tab_fix = 0x9;
                else
                    iq_tab_fix = 0xa;
                mcu_blk_sel = 0xa;
                dtmp = (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

                MT_INFO_JPEG("\r\n Y~~~~~~~~~ quant_tbl_no:%d, dc_tbl_no:%d, ac_tbl_no:%d", 
                    compptr[2].quant_tbl_no, compptr[2].dc_tbl_no, compptr[2].ac_tbl_no);   
                JPEG_HDEC_SaveRegs(cinfo, 2);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		


       		eIntStatus = JPG_INTTYPE_ERROR;
			do
			{
	       		s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
	       		if(MT_SUCCESS != s32Ret)
	       		{
	       			goto FAIL;
	       		}
				if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
				{
				  u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                  if(u32CurAddrPhy < pDataPhyBuf)
                      u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				  u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;						
				  p_ptr = (MT_U8 *)u32CurAddrVir;
				  if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				  }
				  else
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);	   
				  }
				}		  
			}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));

                   
                  //jpg_buf_clr
                 JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);                  

                  //------------------------K
                dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                dtmp |= (CMYK_K & 0x3f);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp);                         

                
                huff_tab_fix = 0xf;
                if(compptr[3].quant_tbl_no == 0)
                    iq_tab_fix = 0x8;
                else if(compptr[3].quant_tbl_no == 1)
                    iq_tab_fix = 0x9;
                else
                    iq_tab_fix = 0xa;
                mcu_blk_sel = 0xb;
                dtmp = (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);

                MT_INFO_JPEG("\r\n K~~~~~~~~~ quant_tbl_no:%d, dc_tbl_no:%d, ac_tbl_no:%d", 
                    compptr[3].quant_tbl_no, compptr[3].dc_tbl_no, compptr[3].ac_tbl_no);  
                JPEG_HDEC_SaveRegs(cinfo, 3);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		


       		eIntStatus = JPG_INTTYPE_ERROR;
			do
			{
	       		s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
	       		if(MT_SUCCESS != s32Ret)
	       		{
	       			goto FAIL;
	       		}
				if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
				{
				  u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                  if(u32CurAddrPhy < pDataPhyBuf)
                       u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				  u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;						
				  p_ptr = (MT_U8 *)u32CurAddrVir;
				  if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				  }
				  else
				  {
					  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);	   
				  }
				}		  
			}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));

                   
                  //jpg_buf_clr
                 JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);           
              }   


              
		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
              cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
        
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membership[s32Cnt] = s32Cnt;
		}
      
		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
            MT_INFO_JPEG("\r\n ~~~~~~cpy stMiddleSurface.pMiddleVir to stOutDesc.stOutSurface.pOutVir");
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}
        
		return MT_SUCCESS;

	FAIL:

		return MT_FAILURE;
}

MT_S32 JPEG_HDEC_SendStreamFromPhyMemNonInterleave(j_decompress_ptr cinfo)
{  
		JPG_INTTYPE_E eIntStatus         = JPG_INTTYPE_NONE;
		MT_S32 s32Cnt                    = 0;
		MT_U32 u32ConsumSize             = 0;
		phys_addr_t pSaveStreamPhyAddr     = 0;
		phys_addr_t pSaveStreamEndPhyAddr  = 0;

		MT_S32 s32Ret = MT_SUCCESS;
              MT_U32 dtmp = 0;
              jpeg_component_info *compptr = cinfo->comp_info;  
              MT_U32 huff_tab_fix = 0;
              MT_U32 iq_tab_fix = 0;
              MT_U32 mcu_blk_sel = 0;
              MT_U32 i = 0;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
              MT_U32 reached_marker = 0;
              MT_U32 clip_start_x, clip_start_y, clip_end_x, clip_end_y;
              MT_U32 pic_in_type = 0;
              MT_U32 huff_tab_sel0 = 0;
              struct jpeg_source_mgr * datasrc = NULL;
              ulong u32CurAddrPhy = 0;
              ulong u32CurAddrVir = 0;
              MT_U8 *p_ptr = NULL;
              ulong pDataVirBuf;
              ulong pDataPhyBuf;
//              MT_U32 u64DataSize;
              
		/**
		** no alloc the stream buffer, we can set in Makefile about JPGD_HARD_BUFFER
		** CNcomment:这时候不用自己分配码流buffer了，可以在Makefile中设置JPGD_HARD_BUFFER = 0 CNend\n
		**/

		/**
		** calc the consume stream size
		** CNcomment:进入硬件之前消耗的码流数等于每次读码流累加-4096中剩余的码流数 CNend\n
		**/
		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bNeedStreamReturn)
		{
			if(pJpegHandle->stHDecDataBuf.u32ConsumeDataSize <= pJpegHandle->stHDecDataBuf.u64DataSize)
			{
				u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
				pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
				pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pStartBufPhy + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			}
			else
			{
				u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - pJpegHandle->stHDecDataBuf.u64DataSize - cinfo->src->bytes_in_buffer;
				pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pStartBufPhy + u32ConsumSize;
				pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pStartBufPhy + pJpegHandle->stHDecDataBuf.s32StreamReturnLen;
			}
		}
		else
		{
			u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
			pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
			pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + pJpegHandle->stHDecDataBuf.u64DataSize;
		}
        
              pDataVirBuf = (ulong)pJpegHandle->stHDecDataBuf.pDataVirBuf;
              pDataPhyBuf = (ulong)pJpegHandle->stHDecDataBuf.pDataPhyBuf;
//              u64DataSize = pJpegHandle->stHDecDataBuf.u64DataSize;
		/**
		** start the decode, no need resume
		** CNcomment:直接启动解码，不需要续码流 CNend\n
		**/
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pSaveStreamPhyAddr);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pSaveStreamEndPhyAddr - 1);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);    


              for(i = 0; i < (mt_u32)cinfo->num_components; i++)
              {
#if 0             
                {
                    int j = 0;
                    for (j = 0x00; j <= 0x100; j += 4)
                      printf("reg[0x%08x]: 0x%08x\n", JCODEC_BASE_ADDR + j,
                      JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BASE_ADDR + j));
                }
    #endif

                eIntStatus = JPG_INTTYPE_ERROR;
                JPEG_HDEC_SaveRegs(cinfo, i);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		    
				do
				{
		       		s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
		       		if(MT_SUCCESS != s32Ret)
		       		{
		       		    	MT_ERR_JPEG("\r\n ----------\n");
		       			goto FAIL;
		       		}
                    if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
                    {
                        u32CurAddrPhy = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1;
                        if(u32CurAddrPhy < pDataPhyBuf)
                          u32CurAddrPhy = JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
                        u32CurAddrVir = pDataVirBuf + u32CurAddrPhy - pDataPhyBuf;  
                        p_ptr = (MT_U8 *)u32CurAddrVir;
                        if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
                        }
                        else
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);         
                        }
                      }    

					}while(JPG_INTTYPE_FINISH != (eIntStatus & JPG_INTTYPE_FINISH));
//				JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);

                   if(i == (mt_u32)(cinfo->num_components - 1))
                   {
                      //jpg_buf_clr
                     JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);        
                      break;
                   }
  				   //---------------------start next component

                   do
                  {
                    reached_marker = (mt_u32)read_markers(cinfo);
			MT_INFO_JPEG("\r\nreached_marker = %d", reached_marker);
                  }while ((reached_marker != JPEG_REACHED_SOS) && (reached_marker != JPEG_REACHED_EOI));

                  start_pass_huff_decoder(cinfo);
                  //jpg_buf_clr
                  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);     

                  dtmp = ((pJpegHandle->stJpegSofInfo.u32CMcuAlignHeight & 0xffff) << 16) | pJpegHandle->stJpegSofInfo.u32CMcuAlignWidth;
                  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_SIZE, (MT_S32)dtmp);     

                  if(cinfo->jpeg_color_space == JCS_YCbCr)
                    pic_in_type = YUV040 + i;
                  else if(cinfo->jpeg_color_space == JCS_RGB)
                    pic_in_type = RGB040 + i;

                  dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
                  dtmp |= (pic_in_type & 0x3f);
                  dtmp &= ~(3 << 20);
                  dtmp |= pJpegHandle->stJpegSofInfo.sJpegOtherInfo.pic_ds << 20;
                  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE, (MT_S32)dtmp); 

                  if(cinfo->max_h_samp_factor == 2)
                  {
                    clip_start_x = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x / 2;
                    clip_end_x = (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x +1) / 2 - 1;
                  }
                  else
                  {
                    clip_start_x = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_x;
                    clip_end_x = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_x;
                  }
                  if(cinfo->max_v_samp_factor == 2)
                  {
                    clip_start_y = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y / 2;
                    clip_end_y = (pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y +1) / 2 - 1;
                  }
                  else
                  {
                    clip_start_y = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_start_y;
                    clip_end_y = pJpegHandle->stJpegSofInfo.sJpegOtherInfo.clip_end_y;
                  } 
      
                dtmp = ((clip_start_y & 0x3fff) << 16) | (clip_start_x & 0x3fff);
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_START, (MT_S32)dtmp);
                dtmp = ((clip_end_y & 0x3fff) << 16) | (clip_end_x & 0x3fff);                   
                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_END, (MT_S32)dtmp);
                jcodec_set_tab(cinfo, FALSE); 


                  huff_tab_fix = 0xf;
                  if(compptr[i + 1].quant_tbl_no == 0)
                      iq_tab_fix = 0x8;
                  else if(compptr[i + 1].quant_tbl_no == 1)
                      iq_tab_fix = 0x9;
                  else
                      iq_tab_fix = 0xa;    
                  mcu_blk_sel = 0;

                  dtmp = (mt_u32)((0xff << 24) | (huff_tab_fix << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4));
                  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);
                  
                  huff_tab_sel0 = (mt_u32)(compptr[1].dc_tbl_no);
                  dtmp = (phys_addr_t)(0x1 | (huff_tab_sel0 << 3));
                  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG1, (phys_addr_t)dtmp);
                  
                  // setup marker offset U
                  datasrc = cinfo->src;

    			u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
			MT_INFO_JPEG("\r\n bytes_in_buffer:0x%08x", datasrc->bytes_in_buffer); 
			pSaveStreamPhyAddr     =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
			pSaveStreamEndPhyAddr  =   pJpegHandle->stHDecDataBuf.pDataPhyBuf + pJpegHandle->stHDecDataBuf.u64DataSize;
			
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pSaveStreamPhyAddr);
              JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pSaveStreamEndPhyAddr - 1);

                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_EN, 0x10101);
                            
                }
		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
              cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
        
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membership[s32Cnt] = s32Cnt;
		}
      
		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
           		MT_INFO_JPEG("\r\n ~~~~~~cpy stMiddleSurface.pMiddleVir to stOutDesc.stOutSurface.pOutVir");
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}
        
		return MT_SUCCESS;

	FAIL:

		return MT_FAILURE;
}
#endif  /** CONFIG_JPEG_STREAMBUF_4ALIGN **/


/*****************************************************************************
* func			: JPEG_HDEC_SendStreamFromVirMem
* description	: get the stream from virtual memory
                  CNcomment:  码流来源虚拟内存的处理方式   CNend\n
* param[in]     : cinfo       CNcomment:  解码对象     CNend\n
* retval	    : MT_SUCCESS  CNcomment:  成功         CNend\n
* retval	    : MT_FAILURE  CNcomment:  失败         CNend\n
* others:	    : NA
*****************************************************************************/
MT_S32 JPEG_HDEC_SendStreamFromVirMem(j_decompress_ptr cinfo)
{
         

		JPG_INTTYPE_E eIntStatus = JPG_INTTYPE_NONE;
		MT_S32  s32Cnt            = 0;


		MT_U32 u32ReadDataSize  = 0;
		/**
		 ** the continue stream size should big than 16bytes
		 ** CNcomment:每一段续码流必须大于16个字节，保守 CNend\n
		 **/
		MT_U32 u32NeedDecCnt		       = 0;
		MT_U32 u32ConsumSize              = 0;
		MT_U64 u64LeaveSize               = 0;
		MT_U32 u32TotalSize               = 0;
		MT_CHAR* pDataVirAddr             = NULL;
		phys_addr_t  pSaveStreamStartPhyAddr = 0;
		phys_addr_t pSaveStreamEndPhyAddr   = 0;

              MT_S32 s32Ret = MT_SUCCESS;
              phys_addr_t u32CurAddrPhy = 0;
              ulong u32CurAddrVir = 0;
              MT_U8 *p_ptr = NULL;
              MT_U32 marker_type = 0;
              MT_BOOL b_start = MT_FALSE;

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		/**
		** tell the hard the stream is end.
		** CNcomment:这个是必须的，要告诉硬件码流已经读完了，否则硬件会一直解
		**           硬件本省无法判断读码流结束 CNend\n
		**/

		u32ReadDataSize = JPGD_STREAM_BUFFER;
		u32ConsumSize   = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
              u64LeaveSize    = pJpegHandle->stHDecDataBuf.u64DataSize - ((MT_U64)u32ConsumSize);
		pDataVirAddr    = pJpegHandle->stHDecDataBuf.pDataVirBuf + u32ConsumSize;
		pSaveStreamStartPhyAddr = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf;

		eIntStatus = JPG_INTTYPE_CONTINUE;
		do {
                    	MT_INFO_JPEG("\r\n eIntStatus:0x%08x", eIntStatus);
			if(JPG_INTTYPE_ERROR == (eIntStatus & JPG_INTTYPE_ERROR)) 
			{
				goto FAIL;
			}     
                     if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
                    {
                        u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                        if(u32CurAddrPhy < (phys_addr_t)pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf)
                        {
                            u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
                         }
                        u32CurAddrVir = (ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf + (u32CurAddrPhy -pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf);                        
                        p_ptr = (MT_U8 *)u32CurAddrVir;
                        if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
                        }
                        else
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);
                            if(*p_ptr == MARKER_EOI)
                                marker_type |= EOI_REACHED;
                            else if(*p_ptr == MARKER_DHT)
                                marker_type |= DHT_REACHED;
                            else if(*p_ptr == MARKER_DQT)            
                                marker_type |= DQT_REACHED;      
                            else if(*p_ptr == MARKER_SOS)            
                            {
                                marker_type |= SOS_REACHED;    
                            }
                            
                            MT_INFO_JPEG("\r\n p_ptr:0x%02x, 0x%02x, 0x%02x,0x%02x", p_ptr[0], p_ptr[1], p_ptr[2], p_ptr[3]);
                        }
                      }
                      if((JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH)) || ((marker_type & EOI_REACHED) == EOI_REACHED))
                        {
                            break;
                        }
                     if(JPG_INTTYPE_CONTINUE == (eIntStatus & JPG_INTTYPE_CONTINUE)) 
                    {
				MT_INFO_JPEG("\r\n JPG_INTTYPE_CONTINUE");
        			{
        				u32NeedDecCnt  = u32ReadDataSize < ((MT_U32)u64LeaveSize - u32TotalSize) ? u32ReadDataSize : ((MT_U32)u64LeaveSize - u32TotalSize);
        				memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, (pDataVirAddr + u32TotalSize),u32NeedDecCnt);
        				u32TotalSize  += u32NeedDecCnt;
        			}

        						
        			/** 刷码流数据 **/
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
        			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
#else
        			s32Ret = MT_GFX_Flush(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
#endif
        			if(MT_SUCCESS != s32Ret)
        			{
        			   	goto FAIL;
        			}

        			pSaveStreamEndPhyAddr   = pSaveStreamStartPhyAddr + u32NeedDecCnt;


        			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pSaveStreamStartPhyAddr);
        			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pSaveStreamEndPhyAddr - 1);
                             JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);    
                             if(b_start == MT_FALSE)
                             {
                                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);
                                b_start = MT_TRUE;
                             }
                             else
                                JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 4);
                        }
     			eIntStatus = JPG_INTTYPE_ERROR;
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
                    }while (JPG_INTTYPE_FINISH != eIntStatus);

//		JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);

		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
		   cinfo->MCU_membership[s32Cnt] = s32Cnt;
		}

		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_SUCCESS;
		}


#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if((0 != pJpegHandle->stJpegSofInfo.u32YSize) && (pJpegHandle->stOutDesc.stOutSurface.pOutVir[0] != NULL))
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if((0 != pJpegHandle->stJpegSofInfo.u32CSize) && (pJpegHandle->stOutDesc.stOutSurface.pOutVir[1] != NULL))
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}

		return MT_SUCCESS;

		/** if decode failure jump here **/
		FAIL:


			 return MT_FAILURE;
		  
}


/*****************************************************************************
* func			: JPEG_HDEC_SendStreamFromFile
* description	: CNcomment:  码流来源文件
* param[in] 	: cinfo 	  CNcomment:  解码对象
* param[in] 	: NA
* retval		: MT_SUCCESS  CNcomment:  成功
* retval		: MT_FAILURE  CNcomment:  失败
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_SendStreamFromFile(j_decompress_ptr cinfo)
{ 
		JPG_INTTYPE_E eIntStatus = JPG_INTTYPE_NONE;
		MT_BOOL bRetVal           = MT_FALSE;

		MT_BOOL bReachEOF         = MT_FALSE;
		MT_BOOL bStartDec         = MT_FALSE;
		MT_S32  s32Cnt            = 0;

		MT_U32 u32ReadDataSize       = 0;
		MT_U32 u32ResumeSize		   = 0;
		/**
		** the continue stream size should big than 16bytes
		** CNcomment:每一段续码流必须大于16个字节，保守 CNend\n
		**/
		MT_U32 u32NeedDecCnt		   = 0;
		phys_addr_t pStreamStartPhyAddr = 0;
		MT_CHAR* pStreamStartVirAddr = NULL;
		phys_addr_t pStreamEndPhyAddr   = 0;

		MT_S32 s32Ret = MT_SUCCESS;
		phys_addr_t u32CurAddrPhy = 0;
		ulong u32CurAddrVir = 0;
		MT_U8 *p_ptr = NULL;        
		MT_U32 marker_type = 0;


        my_src_ptr src = (my_src_ptr)cinfo->src;
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);



		pJpegHandle->u32CurrentOffset = (MT_U32)ftell(src->infile);
        pJpegHandle->u32CurrentOffset = pJpegHandle->u32CurrentOffset - cinfo->src->bytes_in_buffer;


		/**
		** tell the hard the stream is end.
		** CNcomment:这个是必须的，要告诉硬件码流已经读完了，否则硬件会一直解
		**           硬件本省无法判断读码流结束 CNend\n
		**/
//		bReachEOF = MT_FALSE;

		pStreamStartVirAddr = pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf;
		u32ReadDataSize     = pJpegHandle->stHDecDataBuf.u32ReadDataSize;

		eIntStatus = JPG_INTTYPE_CONTINUE;
		do {
                    	MT_INFO_JPEG("\r\n eIntStatus:0x%08x", eIntStatus);
			if(JPG_INTTYPE_ERROR == (eIntStatus & JPG_INTTYPE_ERROR)) 
			{
				goto FAIL;
			}     
                     if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
                    {
                        u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
                        if(u32CurAddrPhy < (phys_addr_t)pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf)
                        {
                            u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
                        }
                        u32CurAddrVir = (ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf + (u32CurAddrPhy -pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf);                        
                        p_ptr = (MT_U8 *)u32CurAddrVir;
                        if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
                        }
                        else
                        {
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);
                            if(*p_ptr == MARKER_EOI)
                                marker_type |= EOI_REACHED;
                            else if(*p_ptr == MARKER_DHT)
                                marker_type |= DHT_REACHED;
                            else if(*p_ptr == MARKER_DQT)            
                                marker_type |= DQT_REACHED;      
                            else if(*p_ptr == MARKER_SOS)        
                            {
                                marker_type |= SOS_REACHED;         
                            }
                            	MT_INFO_JPEG("\r\n p_ptr:0x%02x, 0x%02x, 0x%02x,0x%02x", p_ptr[0], p_ptr[1], p_ptr[2], p_ptr[3]);
                        }
                      }
                      if((JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH)) || (marker_type == MARKER_EOI))
                        {
                            break;
                        }
                     if(JPG_INTTYPE_CONTINUE == (eIntStatus & JPG_INTTYPE_CONTINUE)) 
                    {
			/**
			** the consume stream size
			** CNcomment:消耗的码流数 CNend\n
			**/


			if(0 != cinfo->src->bytes_in_buffer)
			{
				/**
				** copy the leave stream to save stream buffer and start decode
				** CNcomment:拷贝剩余码流到码流buffer中，然后启动解码 CNend\n
				**/
				u32NeedDecCnt = cinfo->src->bytes_in_buffer;
				memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,   \
				(char*)cinfo->src->next_input_byte,	   \
				u32NeedDecCnt);
				if(	  (0xFF == (*(cinfo->src->next_input_byte + u32NeedDecCnt - 2)))
					&&(0xD9 == (*(cinfo->src->next_input_byte + u32NeedDecCnt - 1))))
				{
					bReachEOF = MT_TRUE;
                                   bReachEOF = bReachEOF; //clean warning
				}
				else
				{
					pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_TRUE;
					pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf = pStreamStartVirAddr + u32NeedDecCnt;

					pJpegHandle->stHDecDataBuf.u32ReadDataSize = \
					pJpegHandle->stHDecDataBuf.u32ReadDataSize - u32NeedDecCnt;

					bRetVal = (*cinfo->src->fill_input_buffer)(cinfo);/*lint !e64 ignore by y00181162, because return value is ok */  
					if(MT_FALSE == bRetVal)
					{
						goto FAIL;
					}

					u32NeedDecCnt = u32NeedDecCnt + cinfo->src->bytes_in_buffer;

					/** 还原码流首地址 **/
					pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf     = pStreamStartVirAddr;
					pJpegHandle->stHDecDataBuf.u32ReadDataSize = u32ReadDataSize;
				}

			}

			if(0 == cinfo->src->bytes_in_buffer)
			{
 
				/**
				** there is not stream,should read data to save stream buffer
				** CNcomment:没有码流了，需要读码流，这时候直接读码流大小字节到码流buffer中 CNend\n
				**/
				pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_TRUE;
				pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf = pStreamStartVirAddr + u32ResumeSize;
				pJpegHandle->stHDecDataBuf.u32ReadDataSize = pJpegHandle->stHDecDataBuf.u32ReadDataSize - u32ResumeSize;
				bRetVal = (*cinfo->src->fill_input_buffer)(cinfo); /*lint !e64 ignore by y00181162, because return value is ok */  
				if(MT_FALSE == bRetVal)
				{
					goto FAIL;
				}
				u32NeedDecCnt = u32ResumeSize + cinfo->src->bytes_in_buffer;
				pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf  = pStreamStartVirAddr;
				pJpegHandle->stHDecDataBuf.u32ReadDataSize    = u32ReadDataSize;

			}

			/**
			** dinit the para
			** CNcomment:去初始化相关变量 CNend\n
			**/
			u32ResumeSize = 0;
			cinfo->src->bytes_in_buffer = 0;

			/** 刷码流数据 **/
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
#else
			s32Ret = MT_GFX_Flush(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
#endif
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}
			pStreamStartPhyAddr = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf;
			pStreamEndPhyAddr   = pStreamStartPhyAddr + u32NeedDecCnt;

			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pStreamStartPhyAddr);
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pStreamEndPhyAddr - 1);
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);    
			if(bStartDec == MT_FALSE)
			{
				JPEG_HDEC_SaveRegs(cinfo, 0);
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);
				bStartDec = MT_TRUE;
			}
			else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 4);
			}
			eIntStatus = JPG_INTTYPE_ERROR;
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
		}while (JPG_INTTYPE_FINISH != eIntStatus);
        


		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membership[s32Cnt] = s32Cnt;
		}


		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{

			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}


		return MT_SUCCESS;

		/** if decode failure jump here **/
		FAIL:

			 cinfo->src->bytes_in_buffer  = 0;
			 /**
			  ** change the read stream dispose
			  ** CNcomment:硬件解码失败之后读码流还是走原先软解的路了 CNend\n
			  **/
			 pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_FALSE;

			 return MT_FAILURE;	  
}
/*****************************************************************************
* func			: JPEG_HDEC_SendStreamFromCallBack
* description	: CNcomment:  码流来源外部处理
* param[in] 	: cinfo 	  CNcomment:  解码对象
* param[in] 	: NA
* retval		: MT_SUCCESS  CNcomment:  成功
* retval		: MT_FAILURE  CNcomment:  失败
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_SendStreamFromCallBack(j_decompress_ptr cinfo)
{      
		JPG_INTTYPE_E eIntStatus = JPG_INTTYPE_NONE;
		MT_BOOL bReachEOF         = MT_FALSE;
		MT_BOOL bStartDec         = MT_FALSE;
		MT_S32  s32Cnt            = 0;
		MT_S32  s32RetVal         = MT_SUCCESS;


		MT_U32 u32ReadDataSize  = 0;
		/**
		** the continue stream size should big than 16bytes
		** CNcomment:每一段续码流必须大于16个字节，保守 CNend\n
		**/
		MT_U32 u32ReadSize            = 0; /** every times read data bytes **/
		MT_U32 u32ReadPos             = 0;
		MT_U32 u32NeedDecCnt		   = 0;
		phys_addr_t pStreamStartPhyAddr = 0;
		MT_CHAR* pStreamStartVirAddr = NULL;
		phys_addr_t pStreamEndPhyAddr   = 0;

		MT_S32 s32Ret = MT_SUCCESS;
		phys_addr_t u32CurAddrPhy = 0;
		ulong u32CurAddrVir = 0;
		MT_U8 *p_ptr = NULL;        
		MT_U32 marker_type = 0;

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);


		/**
		** tell the hard the stream is end.
		** CNcomment:这个是必须的，要告诉硬件码流已经读完了，否则硬件会一直解
		**           硬件本省无法判断读码流结束 CNend\n
		**/
		bReachEOF = MT_FALSE;

		pStreamStartVirAddr = pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf;
		u32ReadDataSize     = pJpegHandle->stHDecDataBuf.u32ReadDataSize;

#ifdef CONFIG_JPEG_ADD_GOOGLEFUNCTION
		pJpegHandle->u32CurrentOffset = cinfo->src->current_offset - cinfo->src->bytes_in_buffer;
#endif

		eIntStatus = JPG_INTTYPE_CONTINUE;
		do {
			MT_INFO_JPEG("\r\n eIntStatus:0x%08x", eIntStatus);
			if(JPG_INTTYPE_ERROR == (eIntStatus & JPG_INTTYPE_ERROR)) 
			{
				goto FAIL;
			}     
			if(JPG_INTTYPE_MARKER == (eIntStatus & JPG_INTTYPE_MARKER))
			{
				u32CurAddrPhy = (phys_addr_t)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_CUR_ADDR) - 1);
				if(u32CurAddrPhy < (phys_addr_t)pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf)
					u32CurAddrPhy = (phys_addr_t)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BUF_END_ADDR);
				u32CurAddrVir = (ulong)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf + (u32CurAddrPhy -pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf);                        
				p_ptr = (MT_U8 *)u32CurAddrVir;
				if((*p_ptr != MARKER_EOI) && (*p_ptr != MARKER_DHT) && (*p_ptr != MARKER_SOS) && (*p_ptr != MARKER_DQT))
				{
					JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 12);
				}
				else
				{
					JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 13);
					if(*p_ptr == MARKER_EOI)
						marker_type |= EOI_REACHED;
					else if(*p_ptr == MARKER_DHT)
						marker_type |= DHT_REACHED;
					else if(*p_ptr == MARKER_DQT)            
						marker_type |= DQT_REACHED;      
					else if(*p_ptr == MARKER_SOS)            
						marker_type |= SOS_REACHED;         
					MT_INFO_JPEG("\r\n p_ptr:0x%02x, 0x%02x, 0x%02x,0x%02x", p_ptr[0], p_ptr[1], p_ptr[2], p_ptr[3]);
				}
			}
			if((JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH)) || (marker_type == MARKER_EOI))
			{
            	break;
 			}
			if(JPG_INTTYPE_CONTINUE == (eIntStatus & JPG_INTTYPE_CONTINUE)) 
			{

    			/**
    			** the consume stream size
    			** CNcomment:消耗的码流数 CNend\n
    			**/

    			do
    			{/** read stream data till as 1M bytes **/

    				u32ReadSize = 0;
    				if (0 == cinfo->src->bytes_in_buffer)
    				{
    					s32RetVal = (*cinfo->src->fill_input_buffer)(cinfo);
    					if (0==cinfo->src->bytes_in_buffer)
    					{
    						break;
    					}
    					u32ReadPos = 0; /** when read stream, is zero **/
    				}
    				if (cinfo->src->bytes_in_buffer <= (u32ReadDataSize - u32NeedDecCnt))
    				{
    					u32ReadSize = cinfo->src->bytes_in_buffer;
    				}
    				else
    				{
    					u32ReadSize = u32ReadDataSize - u32NeedDecCnt;
    				}
    				memcpy(pStreamStartVirAddr + u32NeedDecCnt, (char*)cinfo->src->next_input_byte+u32ReadPos, u32ReadSize);

    				u32ReadPos     = u32ReadSize;
    				u32NeedDecCnt += u32ReadSize;
    				cinfo->src->bytes_in_buffer -= u32ReadSize;

    			} while (u32NeedDecCnt < u32ReadDataSize);


    			bReachEOF = MT_TRUE;
    			if (cinfo->src->bytes_in_buffer) 
    			{
    				bReachEOF = MT_FALSE;
    				bReachEOF = bReachEOF; //clean warning
    			}
    			else
    			{/** is last stream data **/
    				s32RetVal = cinfo->src->fill_input_buffer(cinfo);
    				if (cinfo->src->bytes_in_buffer)
    				{
    					bReachEOF = MT_FALSE;
    				}

    			}
    			if(MT_FALSE == s32RetVal)
    			{
    				/**do notmtng,cancle pc-lint warning **/
    			}
    			
    			/** 刷码流数据 **/
#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
    			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
#else
    			s32Ret = MT_GFX_Flush(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
#endif
    			if(MT_SUCCESS != s32Ret)
    			{
    				goto FAIL;
    			}


    			pStreamStartPhyAddr = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf;
    			pStreamEndPhyAddr   = pStreamStartPhyAddr + u32NeedDecCnt;



    			u32NeedDecCnt = 0;

        			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_START_ADDR, (phys_addr_t)pStreamStartPhyAddr);
        			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_BS_END_ADDR,(phys_addr_t)pStreamEndPhyAddr - 1);
                         JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);    
                         if(bStartDec == MT_FALSE)
                         {
                            JPEG_HDEC_SaveRegs(cinfo, 0);
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);
                            bStartDec = MT_TRUE;
                         }
                         else
                            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 1 << 4);
                      }
     			eIntStatus = JPG_INTTYPE_ERROR;
			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);


                    }while (JPG_INTTYPE_FINISH != eIntStatus);
        


		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membership[s32Cnt] = s32Cnt;
		}




		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{

			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}



		return MT_SUCCESS;

		/** if decode failure jump here **/
		FAIL:

		cinfo->src->bytes_in_buffer  = 0;
		/**
		** change the read stream dispose
		** CNcomment:硬件解码失败之后读码流还是走原先软解的路了 CNend\n
		**/
		pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_FALSE;
		  
		return MT_FAILURE;
	  
}

/*****************************************************************************
* func			: JPEG_HDEC_SendStreamFromFileWithConDDRChange
* description	: get the stream from file
		  	      CNcomment:  码流来源文件的处理方式,这里是测试
		  	  			  续码流地址发生变化的情况
* param[in] 	: cinfo 	  CNcomment:  解码对象
* param[in] 	: NA
* retval		: MT_SUCCESS  CNcomment:  成功
* retval		: MT_FAILURE  CNcomment:  失败
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_SendStreamFromFileWithConDDRChange(j_decompress_ptr cinfo)
{
#ifdef JPGOK         
#ifdef CONFIG_JPEG_FPGA_TEST_CONTINUE_STREAM_DDR_CHANGE

		JPG_INTTYPE_E eIntStatus = JPG_INTTYPE_NONE;
		MT_BOOL bRetVal           = MT_FALSE;
		MT_BOOL bReachEOF         = MT_FALSE;
		MT_BOOL bStartDec         = MT_FALSE;
		MT_S32  s32Cnt            = 0;
		MT_U32 u32ReadDataSize   = 0;
		MT_U32 u32OffsetValue     = 0;

		MT_U32 u32NeedDecCnt		   = 0;
		MT_CHAR* pStreamStartPhyAddr = NULL;
		MT_CHAR* pStreamStartVirAddr = NULL;
		MT_CHAR* pStreamEndPhyAddr   = NULL;

		MT_S32 s32Ret = MT_SUCCESS;

#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		MT_BOOL bStartFirst = MT_TRUE;
#endif

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		/**
		** output the scen message to file
		** CNcomment:导解码现场 CNend\n
		**/
		if(MT_TRUE == pJpegHandle->bSaveScen)
		{
			MT_JPEG_OpenScenFile(cinfo);
		}
#endif

		bReachEOF = MT_FALSE;

		pStreamStartVirAddr = pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf;
		u32ReadDataSize     = pJpegHandle->stHDecDataBuf.u32ReadDataSize;

		JPEG_TRACE("==================================================================\n");
		JPEG_TRACE("the stream start address is 0x%lx\n",pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf);
		JPEG_TRACE("the stream end   address is 0x%lx\n",pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32ReadDataSize);
		JPEG_TRACE("==================================================================\n");

		do 
		{

			if(0 != cinfo->src->bytes_in_buffer)
			{
				u32NeedDecCnt = cinfo->src->bytes_in_buffer;
				memcpy(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf,(char*)cinfo->src->next_input_byte,	u32NeedDecCnt);
				if(	  (0xFF == (*(cinfo->src->next_input_byte + u32NeedDecCnt - 2)))
					&&(0xD9 == (*(cinfo->src->next_input_byte + u32NeedDecCnt - 1))))
				{
					bReachEOF = MT_TRUE;
				}
				else
				{
					pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_TRUE;
					pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf = pStreamStartVirAddr + u32NeedDecCnt;

					pJpegHandle->stHDecDataBuf.u32ReadDataSize = \
				                      pJpegHandle->stHDecDataBuf.u32ReadDataSize - u32NeedDecCnt;

					bRetVal = (*cinfo->src->fill_input_buffer)(cinfo);/*lint !e64 ignore by y00181162, because return value is ok */  
					if(MT_FALSE == bRetVal)
					{
						goto FAIL;
					}

					u32NeedDecCnt = u32NeedDecCnt + cinfo->src->bytes_in_buffer;
					pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf     = pStreamStartVirAddr;
					pJpegHandle->stHDecDataBuf.u32ReadDataSize = u32ReadDataSize;
				}

			}

			if(0 == cinfo->src->bytes_in_buffer)
			{
				u32OffsetValue = rand() % 50 + 1;
				if( (pStreamStartVirAddr + u32OffsetValue) >  (pStreamStartVirAddr + u32ReadDataSize))
				{
					JPEG_TRACE("已经超出了存储码流地址的范围，回车键结束");
					getchar();
					return MT_FAILURE;
				}
			}

			if(0 == cinfo->src->bytes_in_buffer)
			{
				pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_TRUE;
				pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf = pStreamStartVirAddr + u32OffsetValue;
				pJpegHandle->stHDecDataBuf.u32ReadDataSize   = pJpegHandle->stHDecDataBuf.u32ReadDataSize / 2;
				bRetVal = (*cinfo->src->fill_input_buffer)(cinfo); /*lint !e64 ignore by y00181162, because return value is ok */  
				if(MT_FALSE == bRetVal)
				{
					goto FAIL;
				}
				u32NeedDecCnt = cinfo->src->bytes_in_buffer;
				pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf  = pStreamStartVirAddr;
				pJpegHandle->stHDecDataBuf.u32ReadDataSize    = u32ReadDataSize;

			}
			if(MT_TRUE == bStartDec && u32NeedDecCnt < pJpegHandle->stHDecDataBuf.u32ReadDataSize / 2)
			{
				bReachEOF = MT_TRUE;
			}

			if(   (0xFF == (MT_UCHAR)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf[0]) \
				&& (0xD9 == (MT_UCHAR)pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf[1]) \
			    && (2 == cinfo->src->bytes_in_buffer))
			{
				bReachEOF = MT_TRUE;
			}

			cinfo->src->bytes_in_buffer = 0;

#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
			s32Ret = MT_GFX_Flush(pJpegHandle->s32MMZDev,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, pJpegHandle->pSaveStreamMemHandle);
#else
			s32Ret = MT_GFX_Flush(pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf, 0, 0);
#endif
			if(MT_SUCCESS != s32Ret)
			{
				goto FAIL;
			}

			pStreamStartPhyAddr = pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf + u32OffsetValue;
			pStreamEndPhyAddr   = pStreamStartPhyAddr + u32NeedDecCnt;

			JPEG_TRACE("==================================================================\n");
			JPEG_TRACE("the stream start address is 0x%llx\n",pStreamStartPhyAddr);
			JPEG_TRACE("==================================================================\n");

#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
			if(MT_TRUE == pJpegHandle->bSaveScen)
			{	/**
				** should save scen before start decode
				** CNcomment:要在解码启动前保存现场，否则解码会失败 CNend\n
				**/
				MT_JPEG_OutScenData(cinfo,pStreamStartPhyAddr,pStreamEndPhyAddr,pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf + u32OffsetValue,(MT_U64)u32NeedDecCnt,bStartFirst);
				bStartFirst = MT_FALSE;
			}
#endif
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD, (MT_S32)pStreamStartPhyAddr);
			JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD,(MT_S32)pStreamEndPhyAddr);

			if(MT_FALSE == bStartDec)
			{ 
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(bReachEOF ? JPG_EOF_VALUE : 0x0));
#ifdef CONFIG_JPEG_4KDDR_DISABLE
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x5);
#else
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_START, 0x1);
#endif
				bStartDec  = MT_TRUE;

			}
			else
			{
				JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_RESUME,(bReachEOF ? (JPG_EOF_VALUE|JPG_RESUME_VALUE) : JPG_RESUME_VALUE));
			}

			eIntStatus = JPG_INTTYPE_ERROR;

			s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
			if(JPG_INTTYPE_ERROR == eIntStatus) 
			{
				goto FAIL;
			}
			else if(JPG_INTTYPE_FINISH == eIntStatus) 
			{
				break;
			}
			else if(JPG_INTTYPE_CONTINUE == eIntStatus)
			{
				continue;
			}
			else 
			{
				goto FAIL;
			}


		} wmtle (JPG_INTTYPE_FINISH != eIntStatus);

		cinfo->output_scanline        = 0;
		cinfo->global_state           = DSTATE_STOPPING;
		cinfo->inputctl->eoi_reached  = MT_TRUE;
		cinfo->rec_outbuf_height      = 1;
		cinfo->MCUs_per_row           = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)&0xffff);
		cinfo->MCU_rows_in_scan       = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_PICSIZE)>>16)&0xffff);
		cinfo->blocks_in_MCU          =  cinfo->num_components;

		for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
		{
			cinfo->MCU_membersmtp[s32Cnt] = s32Cnt;
		}

#ifdef CONFIG_JPEG_FPGA_TEST_SAVE_SCEN_ENABLE
		MT_JPEG_CloseScenFile(cinfo);
#endif

		s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
		if(MT_SUCCESS != s32Ret)
		{
			return MT_SUCCESS;
		}

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
#else
		if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
#endif
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
		}
		if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
		{
			memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
		}

		return MT_SUCCESS;

		FAIL:
			cinfo->src->bytes_in_buffer  = 0;
			pJpegHandle->stHDecDataBuf.bReadToDataBuf  = MT_FALSE;	  
#endif
		
		return MT_FAILURE;
#endif
	return MT_FAILURE;
}


MT_S32 JPEG_HDEC_SofthuffStart(j_decompress_ptr cinfo)
{

  JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
//  MT_U32 mcu_blk_size;
  JPG_INTTYPE_E eIntStatus         = JPG_INTTYPE_NONE;
  MT_S32 s32Ret = MT_SUCCESS;
  MT_S32 s32Cnt                    = 0;
  jpeg_component_info *compptr = cinfo->comp_info;  
  MT_U32 iq_tab_fix = 0;
  MT_U32 mcu_blk_sel = 0;
  MT_U32 dtmp = 0;
  MT_U32 pic_in_type = 0;
  MT_U32 active_width = 0, active_height = 0;
  MT_U32 active_x = 0, active_y = 0;   
  MT_U32 src_width = 0;
  MT_U32 src_height = 0;  
//  MT_U32 i = 0;

  if(cinfo->jpeg_color_space == JCS_YCbCr || cinfo->jpeg_color_space == JCS_GRAYSCALE)
  {
	pic_in_type = YUV400;   
	dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
	dtmp |= (pic_in_type & 0x3f);
	JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_TYPE, (MT_S32)dtmp);  	
  }
  JPEG_HDEC_SetIdctBuf(cinfo, 0);
  if(compptr[0].quant_tbl_no == 0)
	  iq_tab_fix = 0x8;
  else if(compptr[0].quant_tbl_no == 1)
	  iq_tab_fix = 0x9;
  else
	  iq_tab_fix = 0xa;

  mcu_blk_sel = 0x0;
  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0);
  dtmp |=  (0xf << 20) | (iq_tab_fix << 16) | (mcu_blk_sel << 4) | 0x1;
  if((cinfo->jpeg_color_space == JCS_YCbCr) || (cinfo->jpeg_color_space == JCS_RGB) ||
		(cinfo->jpeg_color_space == JCS_GRAYSCALE))
	  dtmp |= (0xff << 24);   
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);     
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x10101);    
#if 0
  {
    int i;
    for (i = 0x4; i <= 0x1fc; i += 4)
        printf("\r\n reg[0x%08x]: 0x%08x", JCODEC_BASE_ADDR + i, JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_BASE_ADDR + i));
  }
 #endif  
    JPEG_HDEC_SaveRegs(cinfo, 0);
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);		

    eIntStatus = JPG_INTTYPE_ERROR;
    s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
    if(MT_SUCCESS != s32Ret)
    {
    goto FAIL;
    }    
    if(JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH))
    {
    /** do nothing **/
		MT_INFO_JPEG("\r\n JPEG_HDEC_SofthuffStart decode finish");
    }
    else 
    {
    goto FAIL;
    }
//	JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);

    //jpg_buf_clr
    JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);

	MT_INFO_JPEG("\r\n component 0 finish");
//--------------------------------------------------------

	if(cinfo->jpeg_color_space != JCS_GRAYSCALE)
	{

  if(cinfo->jpeg_color_space == JCS_YCbCr)
  { 	  
	  pic_in_type = YUV040;   	  
      dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
      dtmp |= (pic_in_type & 0x3f);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_TYPE, (MT_S32)dtmp);  
  
	  if((compptr[1].h_samp_factor != compptr[0].h_samp_factor) 
		  || (compptr[1].v_samp_factor != compptr[0].v_samp_factor))
	  {
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_SIZE);
		  src_width = dtmp & 0xffff;
		  src_height = (dtmp >> 16) & 0xffff;	  
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_START);
		  active_x = dtmp & 0x3FFF;
		  active_y = (dtmp >> 16) & 0x3FFF;
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_END);
		  active_width = (dtmp & 0x3FFF) + 1;
		  active_height = ((dtmp >> 16) & 0x3FFF) + 1;	  
	  
		  if(compptr[1].h_samp_factor * 2 == compptr[0].h_samp_factor)
		  {
			  src_width = src_width >> 1;
			  active_x = active_x >> 1;
			  active_width = active_width >> 1;
		  }
		  
		  if(compptr[1].v_samp_factor * 2 == compptr[0].v_samp_factor)
		  {
			  src_height = src_height >> 1; 
			  active_y = active_y << 1;
			  active_height = active_height << 1;
		  }
		  
		  dtmp = ((src_height & 0xffff) << 16) | src_width;
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_SIZE, (MT_S32)dtmp);	
	  
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_CLIP_START, ((active_y & 0x3FFF) << 16) | (active_x & 0x3FFF));	
		  dtmp = (((active_height - 1) & 0x3fff) << 16) | ((active_width - 1) & 0x3fff);	
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_CLIP_END, (MT_S32)dtmp);
	  } 		  
  }
  else if(cinfo->jpeg_color_space == JCS_RGB)
  { 
	  pic_in_type = RGB_G;	 
	  dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
	  dtmp |= (pic_in_type & 0x3f);
	  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_TYPE, (MT_S32)dtmp);
  }
  else if((cinfo->jpeg_color_space == JCS_CMYK) || 
	(cinfo->jpeg_color_space == JCS_YCCK))
  {		 
	MT_INFO_JPEG("\r\n will add later");  
  }
  	
  JPEG_HDEC_SetIdctBuf(cinfo, 1);
  
  if(compptr[1].quant_tbl_no == 0)
	  iq_tab_fix = 0x8;
  else if(compptr[1].quant_tbl_no == 1)
	  iq_tab_fix = 0x9;
  else
	  iq_tab_fix = 0xa;

  dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0) & (~0xf0000));
  dtmp |= (iq_tab_fix << 16);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x1);
  JPEG_HDEC_SaveRegs(cinfo, 1);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1);	  

  eIntStatus = JPG_INTTYPE_ERROR;
  s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
  if(MT_SUCCESS != s32Ret)
  {
  goto FAIL;
  }    
  if(JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH))
  {
  /** do nothing **/
		MT_INFO_JPEG("\r\n JPEG_HDEC_SofthuffStart decode finish");
  }
  else 
  {
  goto FAIL;
  }
//  JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);

  //jpg_buf_clr
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);
	MT_INFO_JPEG("\r\n component 1 finish");
  //--------------------------------------------------------
  if(cinfo->jpeg_color_space == JCS_YCbCr)
  { 	  
	  pic_in_type = YUV004;   	  
      dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
      dtmp |= (pic_in_type & 0x3f);
      JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_TYPE, (MT_S32)dtmp);  

	  if((compptr[2].h_samp_factor != compptr[1].h_samp_factor) 
		  || (compptr[2].v_samp_factor != compptr[1].v_samp_factor))
	  {
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_SIZE);
		  src_width = dtmp & 0xffff;
		  src_height = (dtmp >> 16) & 0xffff;	  
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_START);
		  active_x = dtmp & 0x3FFF;
		  active_y = (dtmp >> 16) & 0x3FFF;
		  dtmp = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_CLIP_END);
		  active_width = (dtmp & 0x3FFF) + 1;
		  active_height = ((dtmp >> 16) & 0x3FFF) + 1;	 
		  
		  if(compptr[2].h_samp_factor * 2 == compptr[1].h_samp_factor)
		  {
			  src_width = src_width >> 1;
			  active_x = active_x >> 1;
			  active_width = active_width >> 1;
		  }
		  
		  if(compptr[2].v_samp_factor * 2 == compptr[1].v_samp_factor)
		  {
			  src_height = src_height >> 1; 
			  active_y = active_y << 1;
			  active_height = active_height << 1;
		  }
		  
		  dtmp = ((src_height & 0xffff) << 16) | src_width;
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_SIZE, (MT_S32)dtmp);	
	  
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_CLIP_START, ((active_y & 0x3FFF) << 16) | (active_x & 0x3FFF));	
		  dtmp = (((active_height - 1) & 0x3fff) << 16) | ((active_width - 1) & 0x3fff);	
		  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_CLIP_END, (MT_S32)dtmp);		  
	  	} 	  
  }
  else if(cinfo->jpeg_color_space == JCS_RGB)
  { 
	  pic_in_type = RGB_B;	 
	  dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_PIC_TYPE) & (~0x3f));
	  dtmp |= (pic_in_type & 0x3f);
	  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_PIC_TYPE, (MT_S32)dtmp);	  
  }
  else if((cinfo->jpeg_color_space == JCS_CMYK) || 
	(cinfo->jpeg_color_space == JCS_YCCK))
  {
	MT_INFO_JPEG("\r\n will add later"); 
  }

  JPEG_HDEC_SetIdctBuf(cinfo, 2);
  
  if(compptr[2].quant_tbl_no == 0)
  	iq_tab_fix = 0x8;
  else if(compptr[2].quant_tbl_no == 1)
  	iq_tab_fix = 0x9;
  else
  	iq_tab_fix = 0xa;
  
  dtmp = (MT_U32)(JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr, JCODEC_HUFF_MCU_CFG0) & (~0xf0000));
  dtmp |= (iq_tab_fix << 16);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_HUFF_MCU_CFG0, (MT_S32)dtmp);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_EN, 0x1);
  JPEG_HDEC_SaveRegs(cinfo, 2);
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_START, 0x1); 	
  
  eIntStatus = JPG_INTTYPE_ERROR;
  s32Ret = JPEG_HDEC_GetIntStatus(pJpegHandle, &eIntStatus, JPG_TIMEOUT_MS);
  if(MT_SUCCESS != s32Ret)
  {
  goto FAIL;
  }	 
  if(JPG_INTTYPE_FINISH== (eIntStatus & JPG_INTTYPE_FINISH))
  {
  /** do nothing **/
		MT_INFO_JPEG("\r\n JPEG_HDEC_SofthuffStart decode finish");
  }
  else 
  {
  goto FAIL;
  }
//  JPEG_DUMP_REG(pJpegHandle->pJpegRegVirAddr);
  //jpg_buf_clr
  JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr, JCODEC_START, 1 << 8);
	MT_INFO_JPEG("\r\n component 2 finish");

		}
  

  cinfo->output_scanline        = 0;
  cinfo->global_state           = DSTATE_STOPPING;
  cinfo->inputctl->eoi_reached  = MT_TRUE;
  cinfo->rec_outbuf_height      = 1;
        cinfo->MCUs_per_row  = (1 == pJpegHandle->u8Fac[0][0])?((cinfo->image_width   + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_width  + JPEG_MCU_16ALIGN - 1)>>4);
  cinfo->MCU_rows_in_scan  = (1 == pJpegHandle->u8Fac[0][1])?((cinfo->image_height  + JPEG_MCU_8ALIGN - 1)>>3) : ((cinfo->image_height + JPEG_MCU_16ALIGN - 1)>>4);
  
  cinfo->blocks_in_MCU          =  cinfo->num_components;
  
  for(s32Cnt=0; s32Cnt<cinfo->num_components; s32Cnt++)
  {
  	cinfo->MCU_membership[s32Cnt] = s32Cnt;
  }

    /**
    ** the jpeg hard decode finish
    ** CNcomment: jpeg硬件解码完成 CNend\n
    **/	
    pJpegHandle->bHdecEnd  = MT_TRUE;  
  
  s32Ret = JPEG_HDEC_CheckOut(pJpegHandle);
  if(MT_SUCCESS != s32Ret)
  {
  	return MT_SUCCESS;
  }
  
  #ifdef  CONFIG_JPEG_HARDDEC2ARGB
  if(0 != pJpegHandle->stJpegSofInfo.u32YSize && MT_TRUE != pJpegHandle->bDecARGB)
  #else
  if(0 != pJpegHandle->stJpegSofInfo.u32YSize)
  #endif
  {
  	memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[0],pJpegHandle->stMiddleSurface.pMiddleVir[0],pJpegHandle->stJpegSofInfo.u32YSize);
      MT_INFO_JPEG("\r\n ~~~~~~cpy stMiddleSurface.pMiddleVir to stOutDesc.stOutSurface.pOutVir");
  }
  if(0 != pJpegHandle->stJpegSofInfo.u32CSize)
  {
  	memcpy(pJpegHandle->stOutDesc.stOutSurface.pOutVir[1],pJpegHandle->stMiddleSurface.pMiddleVir[1],pJpegHandle->stJpegSofInfo.u32CSize);
  }


  return MT_SUCCESS;
  
  FAIL:
  
  return MT_FAILURE;    
}

