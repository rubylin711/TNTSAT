/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>


#include "mt_jpeglib.h"
#include "jdatasrc.h"
#include "mt_jerror.h"
#include "mt_module_debug.h"

#include "mt_type.h"
#include "mt_jpeg_config.h"
#include "jpeg_hdec_api.h"
#include "jpeg_hdec_error.h"
#include "jpeg_hdec_mem.h"
#include "jpeg_hdec_rwreg.h"
#include "jpeg_hdec_adp.h"
#include "jpeg_hdec_csc.h"
#include "mt_drv_jpeg_reg.h"
#include "mt_jpeg_hal_api.h"

#ifdef CONFIG_JPEG_PROC_ENABLE
#include "jpegint.h"
#endif


#if    defined(CONFIG_JPEG_TEST_SAVE_BMP_PIC)     \
	|| defined(CONFIG_JPEG_TEST_SAVE_YUVSP_DATA)  \
	|| defined(CONFIG_JPEG_TEST_CMTP_RANDOM_RESET)\
	|| defined(CONFIG_JPEG_FPGA_TEST_CONTINUE_STREAM_DDR_CHANGE)
#include "mt_jpeg_hdec_test.h"
#endif

#if defined(CONFIG_JPEG_ANDROID_DEBUG_ENABLE) && defined(CONFIG_JPEG_DEBUG_INFO)
#include <cutils/properties.h>
#define LOG_TAG "libjpeg"
#endif


#define ANDROID_BROWER_FUNCTION

/***************************** Macro Definition ******************************/

/** the jpeg structure init pointer */
/** CNcomment:jpeg私有机构体初始化指针 */
#define CLIENT_DATA_MARK				  0x00FFFFFF

/** the hard dec inflexion size */
/** CNcomment:软件和硬件解码的拐点大小 */
#define JPGD_HDEC_FLEXION_SIZE		  0 //100000000

/*************************** Structure Definition ****************************/


#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC

/** Structure of the some function should realize before main function */
/** CNcomment:一些必须在main函数之前实现的功能变量 */
typedef struct tagJPEG_DECOMPRESS_RES 
{
	MT_S32     s32CscDev;         /**< the csc device           *//**<CNcomment:打开csc设备        */
    MT_CHAR*   pStreamPhyBuf;   /**< The stream physics address *//**<CNcomment:码流buffer物理地址 */
    MT_CHAR*   pStreamVirBuf;   /**< The stream virtual address *//**<CNcomment:码流buffer虚拟地址 */
	
}JPEG_DECOMPRESS_RES;

static JPEG_DECOMPRESS_RES g_stJpegDecompressRes = {-1, NULL, NULL};


/********************** Global Variable declaration **************************/

#ifdef CONFIG_JPEG_FPGA_TEST_ENABLE
/**如果是使用FPGA测试，则分配的内存方式是自己封装的，以及其它实现方式也不一样**/
MT_S32 sg_s32MMZDev = -1;
#endif

/******************************* API forward declarations *******************/

extern MT_S32 MT_JPEG_SET_TEST_INFO(j_decompress_ptr cinfo, MT_U32 value);

/******************************* API realization *****************************/


/***************************************************************************************
* func			: __attribute__ ((constructor))
* description	: tmts function will realize before main function, so some function will
                  realize in tmts function.
                  CNcomment: 应用程序起来之后，也就是第一次调用libjpeg库的时候会先调用
                             该函数，然后再调用main函数，直到退出jpeg应用程序 CNend\n
* param[in] 	: NA
* retval		: MT_SUCCESS 成功
* retval		: MT_FAILURE 失败
* others:		: NA
***************************************************************************************/
void __attribute__ ((constructor)) jpeg_lib_creat(void)
{
		/**
		 ** when malloc mem failure at soft decode, pthread will be killed, so
		 ** we want to use MMZ malloc. we should open at creat decompress
		 ** CNcomment: 要是使用malloc分配的内存，在软件解码过程中要是内存不足
		 **            会导致系统直接挂死，所以要使用mmz来分配内存 CNend\n
		 **/
#ifdef CONFIG_JPEG_FPGA_TEST_ENABLE
		/** only the fpga test used **/
		MMZ_INIT(MMZ_DEV);
#endif

}


/***************************************************************************************
* func			: __attribute__ ((destructor))
* description	: when exit the program, will call tmts function
                  CNcomment: 当退出可执行程序的时候会调用该函数 CNend\n
* param[in] 	: NA
* retval		: MT_SUCCESS   CNcomment: 成功  CNend\n
* retval		: MT_FAILURE   CNcomment: 失败  CNend\n
* others:		: NA
***************************************************************************************/
void __attribute__ ((destructor)) jpeg_lib_destroy(void)
{


		  /**
		   ** close tde device
		   ** CNcomment:关闭TDE设备  CNend\n
		   **/
#ifndef CONFIG_JPEG_CSC_DISABLE
          if(g_stJpegDecompressRes.s32CscDev >= 0) 
		  {
             JPEG_HDEC_CSC_Close(g_stJpegDecompressRes.s32CscDev);
          }
          g_stJpegDecompressRes.s32CscDev = -1;
#endif

#ifdef CONFIG_JPEG_FPGA_TEST_ENABLE
		  MMZ_DINIT();
#endif

          /**
           ** free the stream buffer mem
           ** CNcomment: 释放码流buffer内存 CNend\n
           **/
          if(NULL != g_stJpegDecompressRes.pStreamPhyBuf)
          {
            JPEG_HDEC_FreeStreamMem(g_stJpegDecompressRes.pStreamPhyBuf, g_stJpegDecompressRes.pStreamVirBuf);
          }
          g_stJpegDecompressRes.pStreamPhyBuf = NULL;
		  g_stJpegDecompressRes.pStreamVirBuf = NULL;

}
#endif


/*****************************************************************************
* func			: JPEG_HDEC_OpenDev
* description	: open some device that decode need
				  CNcomment: 打开解码需要的相关设备 	   CNend\n
* param[in] 	: cinfo 		CNcomment: 解码对象    CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	   CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_OpenDev(const struct jpeg_decompress_struct *cinfo)
{

		MT_S32 s32RetVal = 0;

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		if(pJpegHandle->s32JpegDev < 0)
		{
			pJpegHandle->s32JpegDev = open(JPG_DEV, O_RDWR | O_SYNC);
			if(pJpegHandle->s32JpegDev < 0)
			{
				return MT_FAILURE; 
			}
		}

		/**
		** get jpeg device, tmts has signal
		** CNcomment: 获取硬件设备，这里有信号量锁，使之支持多任务 CNend\n
		**/
		s32RetVal = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_GETDEVICE);
		if (MT_SUCCESS != s32RetVal)
		{
			return MT_FAILURE;
		}

		/**
		** mmap the device virtual
		** CNcomment: 映射jpeg设备虚拟地址 CNend\n
		**/
		pJpegHandle->pJpegRegVirAddr  = (volatile char*  )mmap(NULL, \
															 JPGD_REG_LENGTH, 	       \
															 PROT_READ | PROT_WRITE,   \
															 MAP_SHARED,			   \
															 pJpegHandle->s32JpegDev,  \
															 (off_t)0);
		if(MAP_FAILED == pJpegHandle->pJpegRegVirAddr)   
		{	  
			return MT_FAILURE; 
		}


#ifdef CONFIG_JPEG_TEST_CMTP_RANDOM_RESET
		MT_JPEG_SetJpegDev(pJpegHandle->s32JpegDev);
		MT_JPEG_SetJpegVir(pJpegHandle->pJpegRegVirAddr);
#endif

		/**
		 ** open tde device
		 ** CNcomment: 打开TDE设备  CNend\n
		 **/
#ifndef CONFIG_JPEG_CSC_DISABLE
	#ifndef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
		if(pJpegHandle->s32CscDev < 0)
		{
			pJpegHandle->s32CscDev = JPEG_HDEC_CSC_Open();
		}
	#else
		if(g_stJpegDecompressRes.s32CscDev < 0)
		{
			g_stJpegDecompressRes.s32CscDev = JPEG_HDEC_CSC_Open();
		}
		pJpegHandle->s32CscDev = g_stJpegDecompressRes.s32CscDev;
	#endif
	   if(pJpegHandle->s32CscDev < 0)
	   {
			return MT_FAILURE;
	   }
#endif
		return MT_SUCCESS;


}

#ifdef CONFIG_JPEG_PROC_ENABLE

/*****************************************************************************
* func			: JPEG_HDEC_SetProcInfo
* description	: set the proc information
				  CNcomment: 设置proc信息		  CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象 CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	  CNend\n
* others:		: NA
*****************************************************************************/
static MT_S32 JPEG_HDEC_SetProcInfo(const struct jpeg_decompress_struct *cinfo)
{

     MT_S32 s32Ret = MT_SUCCESS;
     MT_JPEG_PROC_INFO_S stProcInfo;

	 const MT_U32 u32InFmt[10]  = {0,1,2,3,4,5,6,7,8,9};
	 const MT_U32 u32OutFmt[20] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
	 MT_U32 i;
   
     JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

     if(NULL == pJpegHandle->pJpegRegVirAddr)
     {
        return MT_FAILURE;
     }
     stProcInfo.u32YWidth        = pJpegHandle->stJpegSofInfo.u32YMcuAlignWidth;
	 stProcInfo.u32YHeight       = pJpegHandle->stJpegSofInfo.u32YMcuAlignHeight;
	 stProcInfo.u32YSize         = pJpegHandle->stJpegSofInfo.u32YSize;
	 stProcInfo.u32CWidth        = pJpegHandle->stJpegSofInfo.u32CMcuAlignWidth;
	 stProcInfo.u32CHeight       = pJpegHandle->stJpegSofInfo.u32CMcuAlignHeight;
	 stProcInfo.u32CSize         = pJpegHandle->stJpegSofInfo.u32CSize;
	 stProcInfo.u32YStride       = pJpegHandle->stJpegSofInfo.u32YOutStride;
	 stProcInfo.u32CbCrStride    = pJpegHandle->stJpegSofInfo.u32COutStride;
	 stProcInfo.u32DisplayW      = pJpegHandle->stJpegSofInfo.u32YOutWidth;
	 stProcInfo.u32DisplayH      = pJpegHandle->stJpegSofInfo.u32YOutHeight;
	 stProcInfo.u32DisplayStride = pJpegHandle->stJpegSofInfo.u32DisplayStride;
#if 0   
	 stProcInfo.u32DecW          = pJpegHandle->stJpegSofInfo.u32DecW;
	 stProcInfo.u32DecH          = pJpegHandle->stJpegSofInfo.u32DecH; 
	 stProcInfo.u32DecStride     = pJpegHandle->stJpegSofInfo.u32DecStride;
	 stProcInfo.u32DataStartAddr = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADDR);
	 stProcInfo.u32DataEndAddr   = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADDR);
	 stProcInfo.u32SaveStartAddr = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_STADD);
	 stProcInfo.u32SaveEndAddr   = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_ENDADD);
#endif

  for(i = 0; i < (mt_u32)cinfo->num_components; i++)
    memcpy(stProcInfo.stJpgReg[i], pJpegHandle->stJpegSofInfo.stJpgReg[i], JCODEC_REG_NUM * sizeof(MT_JPG_PROC_S));
	 stProcInfo.u32InWidth       = cinfo->image_width;
	 stProcInfo.u32InHeight      = cinfo->image_height;
   stProcInfo.bIsProgressive = cinfo->progressive_mode;
   stProcInfo.u32NumComponents = (mt_u32)cinfo->num_components;
   stProcInfo.u32CompsInScan = (mt_u32)cinfo->comps_in_scan;
	 stProcInfo.u32OutWidth      = cinfo->output_width;
	 stProcInfo.u32OutHeight     = cinfo->output_height;
	 stProcInfo.u32OutStride     = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
     stProcInfo.u32InFmt         = u32InFmt[pJpegHandle->enImageFmt];
	 stProcInfo.u32OutFmt        = u32OutFmt[cinfo->out_color_space];
     stProcInfo.u32OutPhyBuf     = pJpegHandle->stOutDesc.stOutSurface.pOutPhy[0];
	 if(0 == pJpegHandle->u32ScalRation)
	 {
	     stProcInfo.u32Scale     = 1;
	 }
	 else if(1 == pJpegHandle->u32ScalRation)
	 {
	     stProcInfo.u32Scale     = 2;
	 }
	 else if(2 == pJpegHandle->u32ScalRation)
	 {
	     stProcInfo.u32Scale     = 4;
	 }
	 else
	 {
	      stProcInfo.u32Scale     = 8;
	 }

	 if(DSTATE_START == cinfo->global_state)
	 {	/**
		 **create decompress
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_CREATE_DECOMPRESS;
	 }
	 else if(DSTATE_INHEADER == cinfo->global_state)
	 {  /**
		 **read header ready
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_READ_HEADER;
	 }
	 else if(DSTATE_SCANNING == cinfo->global_state)
	 {  /**
		 **start decompress ready
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_START_DECOMPRESS;
	 }
	 else if(DSTATE_SCANNING == cinfo->global_state)
	 {  /**
		 **read scanlines ready
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_READ_SCANLINES;
	 }
	 else if(DSTATE_STOPPING == cinfo->global_state)
	 {
	    /**
		 **finish decompress
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_FINISH_DECOMPRESS;
	 }
	 else if(0 == cinfo->global_state)
	 {  /**
		 **destory decompress
		 **/
	     pJpegHandle->eDecState = JPEG_DEC_FINISH_DESTORY_DECOMPRESS;
	 }
	 
	 stProcInfo.eDecState        = pJpegHandle->eDecState;
	 
	 if(MT_TRUE == pJpegHandle->bHdecEnd)
	 {
	    stProcInfo.eDecodeType      = JPEG_DEC_HW;
	 }
	 else
	 {
	    stProcInfo.eDecodeType      = JPEG_DEC_SW;
	 }
	 
	 s32Ret = ioctl(pJpegHandle->s32JpegDev, CMD_JPG_READPROC, &stProcInfo);
     if(MT_SUCCESS != s32Ret)
     {
        return MT_FAILURE;
     }
     return MT_SUCCESS;
	 
}
#endif


/*****************************************************************************
* func			: JPEG_HDEC_CloseDev
* description	: closxe some device that decode need
				  CNcomment: 关闭解码打开的相关设备 	   CNend\n
* param[in] 	: cinfo 		CNcomment: 解码对象    CNend\n
* retval		: MT_SUCCESS	CNcomment: 成功 	   CNend\n
* retval		: MT_FAILURE	CNcomment: 失败 	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_CloseDev(const struct jpeg_common_struct *cinfo)
{

		MT_S32 s32Ret = MT_SUCCESS;
#ifdef CONFIG_JPEG_4KDDR_DISABLE_SHOWMSG
		MT_U32 u32AXI4KCNT   = 0;
		MT_U32 u32AXI16MCNT  = 0;
		MT_U32 u32ReadAXI4KCNT   = 0;
		MT_U32 u32WriteAXI4KCNT  = 0;
		MT_U32 u32ReadAXI16MCNT  = 0;
		MT_U32 u32WriteAXI16MCNT = 0;
#endif
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

#ifdef CONFIG_JPEG_PROC_ENABLE
		s32Ret = JPEG_HDEC_SetProcInfo((j_decompress_ptr)cinfo); /*lint !e740 !e826 ignore by y00181162, because tmts cast is ok */  
#endif

#ifdef CONFIG_JPEG_TEST_CMTP_RANDOM_RESET
		MT_JPEG_RandomResetInit();
#endif

#ifdef CONFIG_JPEG_4KDDR_DISABLE_SHOWMSG
		if (NULL != pJpegHandle->pJpegRegVirAddr)
		{
			u32AXI4KCNT  = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_AXI4KCNT);
			u32AXI16MCNT = (MT_U32)JPEG_HDEC_ReadReg(pJpegHandle->pJpegRegVirAddr,JPGD_REG_AXI16MCNT);
		}
		u32ReadAXI4KCNT   = u32AXI4KCNT & 0xffff0000;
		u32WriteAXI4KCNT  = u32AXI4KCNT & 0x0000ffff;
		u32ReadAXI16MCNT  = u32AXI16MCNT & 0xffff0000;
		u32WriteAXI16MCNT = u32AXI16MCNT & 0x0000ffff;
		if(0 != u32AXI4KCNT || 0 != u32AXI16MCNT || 0 != u32ReadAXI4KCNT || 0 != u32WriteAXI4KCNT || 0 != u32ReadAXI16MCNT || 0 != u32WriteAXI16MCNT)
		{
			JPEG_TRACE("\n=====================================================\n");
			JPEG_TRACE("跨4K次数  = %d\n", u32AXI4KCNT);
			JPEG_TRACE("跨16M次数 = %d\n", u32AXI16MCNT);
			JPEG_TRACE("读地址跨4K次数 = %d\n", u32ReadAXI4KCNT);
			JPEG_TRACE("写地址跨4K次数 = %d\n", u32WriteAXI4KCNT);
			JPEG_TRACE("读地址跨16M次数 = %d\n",u32ReadAXI16MCNT);
			JPEG_TRACE("写地址跨16M次数 = %d\n",u32WriteAXI16MCNT);
			JPEG_TRACE("=====================================================\n");
			sleep(2);
		}
		#ifdef CONFIG_JPEG_4KDDR_DISABLE
		if(0 != u32AXI4KCNT || 0 != u32AXI16MCNT)
		{
			JPEG_TRACE("\n=====================================================\n");
			JPEG_TRACE("跨4K开关关掉异常,逻辑有问题\n");
			JPEG_TRACE("按回车键结束\n");
			JPEG_TRACE("=====================================================\n");
			getchar();
		}
		#endif
#endif

		if (NULL != pJpegHandle->pJpegRegVirAddr)
		{
			  s32Ret = munmap((void*)pJpegHandle->pJpegRegVirAddr, JPGD_REG_LENGTH);
			  pJpegHandle->pJpegRegVirAddr = NULL;
		}


#ifndef CONFIG_JPEG_CSC_DISABLE
	#ifndef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
		if(pJpegHandle->s32CscDev >= 0)
		{
			JPEG_HDEC_CSC_Close(pJpegHandle->s32CscDev);
		}
		pJpegHandle->s32CscDev = -1;
	#endif
#endif
		/**
		 **close jpeg device
		 **/
		if(pJpegHandle->s32JpegDev < 0)
		{
		    return MT_SUCCESS;
		}

		close(pJpegHandle->s32JpegDev);
		pJpegHandle->s32JpegDev = -1;

		if(MT_SUCCESS != s32Ret)
		{
		    return MT_FAILURE;
		}

		return MT_SUCCESS;
		

}

/*****************************************************************************
* func			: JPEG_HDEC_Init
* description	: init the private structure para
                  CNcomment: 初始化私有结构体变量   CNend\n
* param[in] 	: cinfo       CNcomment: 解码对象   CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_Init(j_common_ptr cinfo)
{

#ifdef CONFIG_JPEG_GETDECTIME
		MT_S32 s32Ret  = MT_SUCCESS;
#endif

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = NULL;

		pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)calloc(1, SIZEOF(JPEG_HDEC_HANDLE_S));
		JPEG_ASSERT((NULL == pJpegHandle), JPEG_ERR_NOMEM);

		/**
		** if use external stream,when dec failure, call start decompress
		** again,do not need call hard decode again
		** CNcomment: 如果使用外部码流，当解码失败的时候会第二次调用解码
		**            就不需要再走硬件了CNend\n
		**/
		pJpegHandle->bFirstDec          =  MT_TRUE;

		/** default the jpeg hard decode is 4bytes align, but tde is need 16bytes align**/
		pJpegHandle->u32StrideAlign     = JPGD_HDEC_MMZ_CSCOUT_STRIDE_ALIGN;

#ifdef CONFIG_JPEG_GETDECTIME
		s32Ret  = MT_GFX_GetTimeStamp(&pJpegHandle->u32CurTime,NULL);
		if(MT_SUCCESS != s32Ret)
		{
			free(pJpegHandle);
			return MT_FAILURE;
		}
#endif
		pJpegHandle->s32ClientData      =  CLIENT_DATA_MARK;
		pJpegHandle->s32JpegDev        = -1;
		pJpegHandle->s32CscDev          = -1;
		pJpegHandle->u32Inflexion       =  JPGD_HDEC_FLEXION_SIZE;
		pJpegHandle->u32Alpha           =  0xFF;

#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pJpegHandle->s32MMZDev			=  -1;
#endif

#ifdef CONFIG_JPEG_PROC_ENABLE
		pJpegHandle->eDecState          =  JPEG_DEC_STATE_BUTT;
#endif

		/**
		** the jpeg format
		** CNcomment: 原始jpeg图片格式 CNend\n
		**/
		pJpegHandle->enImageFmt   =  JPEG_FMT_BUTT;

		/**
		** save the jpeg handle pointer
		** CNcomment: 存储jpeg句柄指针 CNend\n
		**/
		cinfo->client_data = (void *)pJpegHandle;


		return MT_SUCCESS;


}

/*****************************************************************************
* func			: JPEG_HDEC_ReleaseRes
* description	: release the resouce
				  CNcomment:  释放资源       CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象	CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败  CNend\n
* others:		: NA
*****************************************************************************/
static MT_VOID JPEG_HDEC_ReleaseRes(const struct jpeg_common_struct *cinfo)
{

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		/**
		** get the stream mem
		** CNcomment: 获取硬件解码的码流buffer CNend\n
		**/	 
#ifndef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
		JPEG_HDEC_FreeStreamMem(pJpegHandle);
#endif

		JPEG_HDEC_FreeYUVMem(pJpegHandle);

#if 0
#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
			JPEG_HDEC_FreeMinMem(pJpegHandle);
		}
#endif
#endif
		JPEG_HDEC_FreeOutMem(pJpegHandle);

#if 0
		if(NULL != pJpegHandle->stJpegHtoSInfo.pLeaveBuf)
		{
			free(pJpegHandle->stJpegHtoSInfo.pLeaveBuf);
			pJpegHandle->stJpegHtoSInfo.pLeaveBuf = NULL;
		}
#endif

#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		if(pJpegHandle->s32MMZDev >= 0)
		{
			gfx_mem_close(pJpegHandle->s32MMZDev);
			pJpegHandle->s32MMZDev = -1;
		}
#endif

}

/*****************************************************************************
* func			: JPEG_HDEC_Destroy
* description	: dinit the private structure para
				  CNcomment:  销毁硬件解码器        CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象	CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_Destroy(const struct jpeg_common_struct *cinfo)
{


		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		if (NULL == pJpegHandle)
		{
			return MT_SUCCESS;
		}
		/**
		 ** if memory leak, take out tmts check
		 ** CNcomment: 要是有内存释放问题，去掉该判断 CNend\n
		 **/
		if(MT_FALSE == pJpegHandle->bReleaseRes)
		{
			JPEG_HDEC_ReleaseRes(cinfo);
		}

		free(pJpegHandle);
		pJpegHandle = NULL;

		return MT_SUCCESS;
		 
}
/*****************************************************************************
* func			: JPEG_HDEC_Abort
* description	: when want use the decompress again,call tmts
				  CNcomment:  如果想继续使用解码器，调用该接口 CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象	           CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_Abort(const struct jpeg_common_struct *cinfo)
{
#ifdef CONFIG_JPEG_GETDECTIME
		MT_S32 s32Ret  = MT_SUCCESS;
		MT_U32 u32PreTime = 0;
#endif

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		if (NULL == pJpegHandle)
		{
			return MT_SUCCESS;
		}

	   /**
		** if memory leak, take out tmts check
		** CNcomment: 要是有内存释放问题，去掉该判断 CNend\n
		**/
		if(MT_FALSE == pJpegHandle->bReleaseRes)
		{
			JPEG_HDEC_ReleaseRes(cinfo);
		}

		/**
		** dinit the para, these are the same as init para value
		** CNcomment: 去初始化变量的值，保证和初始化变量的值保持一致 CNend\n
		**/
#ifdef CONFIG_JPEG_GETDECTIME
		u32PreTime = pJpegHandle->u32CurTime;
#endif		
		memset(pJpegHandle,0,sizeof(JPEG_HDEC_HANDLE_S));		
		pJpegHandle->bReleaseRes        =  MT_TRUE;
		

		pJpegHandle->bFirstDec			=  MT_TRUE;
		pJpegHandle->u32StrideAlign 	= JPGD_HDEC_MMZ_CSCOUT_STRIDE_ALIGN;

#ifdef CONFIG_JPEG_GETDECTIME
		s32Ret	= MT_GFX_GetTimeStamp(&pJpegHandle->u32CurTime,NULL);
		if(MT_SUCCESS != s32Ret)
		{
			free(pJpegHandle);
			return MT_FAILURE;
		}
		pJpegHandle->u32DecTime = pJpegHandle->u32CurTime - u32PreTime;
#endif
		pJpegHandle->s32ClientData	 =  CLIENT_DATA_MARK;
		pJpegHandle->s32JpegDev 	 =  -1;
		pJpegHandle->s32CscDev       = -1;

#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pJpegHandle->s32MMZDev		 =  -1;
#endif

		pJpegHandle->u32Inflexion	 =  JPGD_HDEC_FLEXION_SIZE;
		pJpegHandle->u32Alpha		 =  0xFF;

#ifdef CONFIG_JPEG_PROC_ENABLE
		pJpegHandle->eDecState		 =  JPEG_DEC_STATE_BUTT;
#endif

		pJpegHandle->enImageFmt      =  JPEG_FMT_BUTT;

		return MT_SUCCESS;
		 
}

/*****************************************************************************
* func			: JPEG_HDEC_CheckCropSurface
* description	: check the crop rect whether is reasonable
				  CNcomment: 判断裁剪区域是否合理 CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象 CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
static MT_VOID JPEG_HDEC_CheckCropSurface(const struct jpeg_decompress_struct *cinfo)
{

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		if(  (pJpegHandle->stOutDesc.stCropRect.w <= 0) || (pJpegHandle->stOutDesc.stCropRect.h <= 0)
		   ||(pJpegHandle->stOutDesc.stCropRect.x < 0)  ||  (pJpegHandle->stOutDesc.stCropRect.y < 0)
		   ||((MT_U32)(pJpegHandle->stOutDesc.stCropRect.x + pJpegHandle->stOutDesc.stCropRect.w) > cinfo->output_width)
		   ||((MT_U32)(pJpegHandle->stOutDesc.stCropRect.y + pJpegHandle->stOutDesc.stCropRect.h) > cinfo->output_height))
		{
			ERREXIT(cinfo, JERR_CROP_CANNOT_SUPPORT);  /*lint !e740  ignore by y00181162, because tmts function is macro */ 
		}

}

/*****************************************************************************
* func			: JPEG_HDEC_IfSupport
* description	: check whether the hard decode support
				  CNcomment: 判断是否支持硬件解码 CNend\n
* param[in] 	: cinfo 	  CNcomment: 解码对象 CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	  CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	  CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_IfSupport(j_decompress_ptr cinfo)
{

		MT_U32 u32ImageSize	   = 0; /**< the jpeg picture size  *//**<CNcomment:图片大小         */
		MT_S32 s32RetVal		   = MT_FAILURE;
#ifdef CONFIG_JPEG_DEBUG_INFO
	#ifdef CONFIG_JPEG_ANDROID_DEBUG_ENABLE
		MT_CHAR JpegDecMod[256]   = {0}; /**< select jpeg decode module   *//**<CNcomment:选择解码方式  */
	#else
		MT_CHAR *pJpegDecMod       = NULL;
	#endif
#endif

#ifdef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
		const MT_U32 u32StreamSize = JPGD_HARD_BUFFER;
#endif

#if defined(CONFIG_JPEG_ADD_GOOGLEFUNCTION) && defined(ANDROID_BROWER_FUNCTION)
		MT_CHAR ProcName[25] = {0};
        MT_CHAR Name[25];
		MT_S32 s32Pid = 0;
		MT_CHAR ProcessStatusPath[256] ;
		FILE* pProcessStatus = NULL;
#endif

#if 0
#ifndef CONFIG_JPEG_SET_SAMPLEFACTOR/**mtfone has revise tmts bug **/
		/** HSCP201405300013 HSCP201405290010 DTS2014061006717**/
		MT_S32 ci = 0;
		MT_BOOL bY22 = MT_FALSE;
		MT_BOOL bU12 = MT_FALSE;
		MT_BOOL bV12 = MT_FALSE;
		jpeg_component_info *compptr = NULL;
#endif
#endif
#ifdef CONFIG_JPEG_STREAMBUF_4ALIGN
		MT_U32 u32ConsumSize  = 0;
		MT_CHAR* pDataPhyAddr = NULL;
#endif

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		/**
		 ** at hard decode, we check tmts message only once just ok 
		 ** because only one program can operation,and if the message
		 ** is wrong, the hardware can not support, so the followed 
		 ** can not operation
		 ** CNcomment: 硬件解码过程只判断一次，假如这个值被改变了，要么使用内部的退出函数
		 **            退出整个应用，要么使用用户回调的错误管理函数结束该张图片解码 CNend\n
		 **/

#ifdef CONFIG_JPEG_TEST_CMTP_RANDOM_RESET
		MT_JPEG_RandomResetInit();
#endif


		JPEG_HDEC_GetImagInfo(cinfo);

		JPEG_ASSERT((pJpegHandle->u32ScalRation > 3), JPEG_ERR_UNSUPPORT_SCALE);


		if(MT_TRUE == pJpegHandle->stOutDesc.bCrop)
		{
			JPEG_HDEC_CheckCropSurface(cinfo);
		}

		pJpegHandle->bReleaseRes =  MT_FALSE;



		
#ifdef CONFIG_JPEG_DEBUG_INFO
	#ifdef CONFIG_JPEG_ANDROID_DEBUG_ENABLE
			/**
			** how to use tmts, when run android,you can get default value = "hw",
			** so when you not run( setprop JPEGDECMOD soft(or other char valu) ),is all run hard decode
			** CNcomment:android程序运行过程中,首先获取默认的值hw，要是运行
			** 过程中没有setprop JPEGDECMOD soft(除了hw字符) 就一直是hw值了 CNend\n
			**/
			property_get("JPEGDECMOD",JpegDecMod,"hw");
			if(0 != strncmp("hw", JpegDecMod, strlen("hw")>strlen(JpegDecMod)?strlen("hw"):strlen(JpegDecMod)))
			{
				JPEG_TRACE("=== force to soft decode !\n");
				MT_ERR_JPEG("-----------------------\n");
				return MT_FAILURE;
			}
	#else
			/**
			**use the export entironment var
			**export JPEGDECMOD=soft 软件解码
			**默认硬件解码支持走硬件解码
			**/
			pJpegDecMod = getenv( "JPEGDECMOD" );
			if(pJpegDecMod && 0 == strncmp("soft", pJpegDecMod, strlen("soft")>strlen(pJpegDecMod)?strlen("soft"):strlen(pJpegDecMod)))
			{ 
				JPEG_TRACE("=== force to soft decode !\n");
				MT_ERR_JPEG("-----------------------\n");
				return MT_FAILURE;
			}
	#endif
#endif


#if defined(CONFIG_JPEG_ADD_GOOGLEFUNCTION) && defined(ANDROID_BROWER_FUNCTION)
		s32Pid = getpid();
		memset(ProcessStatusPath, 0, 256);
		snprintf(ProcessStatusPath,256,"%s%d%s","/proc/",s32Pid,"/status");
		pProcessStatus = fopen(ProcessStatusPath, "r");
		if(NULL != pProcessStatus)
		{
			memset(ProcName, 0, 25);
			fscanf(pProcessStatus,"%s %s",Name,ProcName);
			fclose(pProcessStatus);
		}
		#if 0
		/** revise by y0018162,the browser anr because the stream seek pointer is null **/
		if(0 == strncmp("android.browser", ProcName, strlen("android.browser")>strlen(ProcName)?strlen("android.browser"):strlen(ProcName)))
		{
			return MT_FAILURE;
		}
		#endif
		if(0 == strncmp("ndroid.cts.stub", ProcName, strlen("ndroid.cts.stub")>strlen(ProcName)?strlen("ndroid.cts.stub"):strlen(ProcName)))
		{
			MT_ERR_JPEG("-----------------------\n");
			return MT_FAILURE;
		}
#endif


		/**
		** the hard decode support resolution
		** CNcomment: 硬件支持的解码分辨率 CNend\n
		**/
		if (	(cinfo->image_width  < 1)
			|| (cinfo->image_width  > 8192)
			|| (cinfo->image_height < 1)
			|| (cinfo->image_height > 8192))
		{
			MT_ERR_JPEG("\n<%s> : <%d> Picture Resolution (%d x %d)Out of range, Olny support 1x1 ~ 8192x8192\n",__FUNCTION__, __LINE__,  cinfo->image_width , cinfo->image_height);
			return MT_FAILURE;
		}

		/**
		** Get the image inflexion, use hardwire decode or soft decode
		** CNcomment: 获取软解和硬解的拐点 CNend\n
		**/
		u32ImageSize = cinfo->image_width * cinfo->image_height;
		if(u32ImageSize <= pJpegHandle->u32Inflexion)
		{
			MT_ERR_JPEG("\n<%s> : <%d> Picture Out of Spec :u32ImageSize: %x < u32Inflexion : %x\n",__FUNCTION__, __LINE__,  u32ImageSize,  pJpegHandle->u32Inflexion);
			return MT_FAILURE;
		}


		/**
		** progressive, arith code ,data_prcidion !=8, cann't use hard decode 
		** CNcomment: progressive arith code data_prcidion !=8硬件不支持 CNend\n
		**/
		if((FALSE != cinfo->arith_code) ||(8 != cinfo->data_precision))
		{
			MT_ERR_JPEG("\n<%s> : <%d> Picture Out of Spec data_precision: %d arith_code : %d\n",__FUNCTION__, __LINE__,  cinfo->data_precision, cinfo->arith_code);
			return MT_FAILURE;
		}
#if 0
		if(	(FALSE != cinfo->progressive_mode) ||(FALSE != cinfo->arith_code) ||(8 != cinfo->data_precision))
		{
			return MT_FAILURE;
		}
#endif
	   /**
		** the leave stream dispose
		** CNcomment: 剩余码流处理 CNend\n
		**/
#if 0
		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUseFileData)
		{
			pJpegHandle->stJpegHtoSInfo.pLeaveBuf	= (MT_CHAR*)calloc(1, INPUT_BUF_SIZE);
			if(NULL == pJpegHandle->stJpegHtoSInfo.pLeaveBuf)
			{
				return MT_FAILURE;
			}
		}
#endif

#ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
		pJpegHandle->s32MMZDev = gfx_mem_open();
		if(pJpegHandle->s32MMZDev < 0)
		{
			MT_ERR_JPEG("<%s> : <%d> err\n", __FUNCTION__, __LINE__);
			return MT_FAILURE;
		}
#endif

		/**
		** if the jpeg image have not any dqt table,we use standard table
		** CNcomment:要是jpeg文件没有带量化表就使用标准量化表 CNend\n
		**/
#ifndef CONFIG_JPEG_MPG_DEC_ENABLE
		if(NULL == cinfo->quant_tbl_ptrs[0])
		{
			ERREXIT(cinfo, JERR_NO_QUANT_TABLE); /*lint !e740 ignore by y00181162, because tmts is needed */
		}
#endif
		/**
		** if the jpeg image have not  huff table,we use standard table
		** CNcomment:要是jpeg文件没有带哈夫曼表就使用标准哈夫曼表 CNend\n
		**/
#ifndef CONFIG_JPEG_MPG_DEC_ENABLE
		if (	(NULL == cinfo->dc_huff_tbl_ptrs[0]) || (NULL != cinfo->dc_huff_tbl_ptrs[2]) 
			 || (NULL == cinfo->ac_huff_tbl_ptrs[0]) || (NULL != cinfo->ac_huff_tbl_ptrs[2]) )
		{
			ERREXIT(cinfo, JERR_BAD_HUFF_TABLE); /*lint !e740 ignore by y00181162, because tmts is needed */
		}
#endif

	   /** here all information is hard decode need,so failure should use soft dec,should not exit **/

	   /**
		** get the stream mem, if the stream from the user physics mem,
		** no need alloc tmts mem.is critical variable,so should consider
		** the many pthread.
		** CNcomment: 获取码流buffer内存，要是码流来源于用户连续的物理内存
		** 		   这里就不需要分配了，只需要给宏开关赋值为0即可。这个
		** 		   地方属于临界资源，所以要考虑到多线程的问题，要是有问题就不使用这种方式了 CNend\n
		**/
		/** if use the user stream phy buffer,should not alloc 1M buffer **/
#ifdef CONFIG_JPEG_STREAMBUF_4ALIGN
		u32ConsumSize = pJpegHandle->stHDecDataBuf.u32ConsumeDataSize - cinfo->src->bytes_in_buffer;
		pDataPhyAddr  = pJpegHandle->stHDecDataBuf.pDataPhyBuf + u32ConsumSize;
		if((MT_FALSE == pJpegHandle->stHDecDataBuf.bUserPhyMem) || (0 != ((MT_S32)pDataPhyAddr % JPGD_HDEC_MMZ_BUG_BUFFER_ALIGN)))
#else
		if(MT_FALSE == pJpegHandle->stHDecDataBuf.bUserPhyMem)
#endif
		{
			#ifndef CONFIG_JPEG_REALIZEFUNC_BEFORMAINFUNC
				s32RetVal = JPEG_HDEC_GetStreamMem(pJpegHandle,JPGD_HARD_BUFFER);
				if(MT_SUCCESS != s32RetVal)
				{/** soft decode, output user buffer **/
					MT_ERR_JPEG("<%s> : <%d> err\n", __FUNCTION__, __LINE__);
					return MT_FAILURE;
				}
			#else
				if(NULL == g_stJpegDecompressRes.pStreamPhyBuf)
				{/** soft decode, output user buffer **/
					s32RetVal = JPEG_HDEC_GetStreamMem(u32StreamSize,&g_stJpegDecompressRes.pStreamPhyBuf,&g_stJpegDecompressRes.pStreamVirBuf);
				}
				if(NULL == g_stJpegDecompressRes.pStreamPhyBuf)
				{/** soft decode, output user buffer **/
					MT_ERR_JPEG("<%s> : <%d> err\n", __FUNCTION__, __LINE__);
					return MT_FAILURE;
				}
				pJpegHandle->stHDecDataBuf.pSaveStreamPhyBuf = g_stJpegDecompressRes.pStreamPhyBuf;
				pJpegHandle->stHDecDataBuf.pSaveStreamVirBuf = g_stJpegDecompressRes.pStreamVirBuf;
				pJpegHandle->stHDecDataBuf.u32ReadDataSize	 = JPGD_STREAM_BUFFER;
			#endif
		}

		/**
		** get the middle mem
		** CNcomment: 获取硬件解码的中间buffer CNend\n
		**/ 
		s32RetVal = JPEG_HDEC_GetYUVMem(pJpegHandle);
		if(MT_SUCCESS != s32RetVal)
		{/** soft decode, output user buffer **/
			MT_ERR_JPEG("<%s> : <%d> err\n", __FUNCTION__, __LINE__);
			return MT_FAILURE;
		}

#if 0
#ifdef CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
			s32RetVal = JPEG_HDEC_GetMinMem(pJpegHandle);
			if(MT_SUCCESS != s32RetVal)
			{/** soft decode, output user buffer **/
				return MT_FAILURE;
			}
		}
#endif
#endif
		s32RetVal = JPEG_HDEC_GetOutMem(cinfo);
		if(MT_SUCCESS != s32RetVal)
		{/** soft decode, output user buffer **/
			MT_ERR_JPEG("<%s> : <%d> err\n", __FUNCTION__, __LINE__);
			return MT_FAILURE;
		}

		return MT_SUCCESS;
		 

}

/*****************************************************************************
* func			: JPEG_HDEC_Start
* description	: start jpeg hard decode
				  CNcomment: 开始硬件解码
* param[in] 	: cinfo 	  CNcomment: 解码对象  CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	   CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_Start(j_decompress_ptr cinfo)
{

		MT_S32 s32RetVal	=  MT_FAILURE;

#ifdef CONFIG_JPEG_FPGA_TEST_CONTINUE_STREAM_DDR_CHANGE
		MT_BOOL bConStreamChange = MT_FALSE;
#endif

#ifdef CONFIG_JPEG_OUTPUT_LUPIXSUM
		MT_U32 u32RegistLuaPixSum0 = 0;
		MT_U64 u64RegistLuaPixSum1 = 0;
#endif
    mt_u32 dtmp = 0;

    JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
        
    mt_sys_read_register(SYMPHONY_IO_PA(0xBF138140), &dtmp);  //read auto start(auto clk)
    if(dtmp & (0x1 << 2))
    {
        dtmp = 0;
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &dtmp);  //read auto start(auto clk)
        dtmp |=  (0x1 << 2);  //enable jpg auto start to enbale png IP clk start
        mt_sys_write_register(SYMPHONY_IO_PA(0xBF138148), dtmp);        
    }


#ifdef CONFIG_MT_FPGA_GPE
    if(cinfo->p_jpeg_dbg_info->reset_test_enable)
    {
        if(cinfo->p_jpeg_dbg_info->reset_test_type == 0)
        {
              MT_JPEG_SET_TEST_INFO(cinfo, 0x2222);
        }
        else
        {
            MT_JPEG_SET_TEST_INFO(cinfo, 0x1111);
        }
    }
 //   printf("<%s> : <%d> pJpegRegVirAddr : %lx\n", __FUNCTION__, __LINE__, (ulong)pJpegHandle->pJpegRegVirAddr);

    if(cinfo->p_jpeg_dbg_info->stress_test_enable)
    {
        if(cinfo->p_jpeg_dbg_info->stress_test_axi_wr_last_mod)                    
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_AXI_WR_LAST_MODE,0x01);
        else
            JPEG_HDEC_WriteReg(pJpegHandle->pJpegRegVirAddr,JCODEC_AXI_WR_LAST_MODE,0x00);                
    }
#endif
		/**
		** set parameter thar hard decode need
		** CNcomment:配置硬件解码需要的参数 CNend\n
		**/
		s32RetVal = JPEG_HDEC_SetPara(cinfo);
		if(MT_SUCCESS != s32RetVal)
		{
			return MT_FAILURE;
		}

	   /**
		** tmts var only used at use callback stream function
		** CNcomment: 这个变量仅用在使用回调码流函数进行解码的情况 CNend\n
		**/
		pJpegHandle->bInHardDec = MT_TRUE;
		
		/**
		 ** send the stream to hard register to start dec
		 ** CNcomment:将码流送给硬件寄存器开始解码 CNend\n
		 **/
		if(MT_FALSE == pJpegHandle->stHDecDataBuf.bUseInsideData)
		{
			s32RetVal = JPEG_HDEC_SendStreamFromCallBack(cinfo);
			MT_INFO_JPEG("-----------------------\n");
		}
		else
		{
			if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUserPhyMem)
			{
				s32RetVal = JPEG_HDEC_SendStreamFromPhyMem(cinfo);
			}
			else if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUseFileData)
			{
				s32RetVal = JPEG_HDEC_SendStreamFromFile(cinfo);
			}
			else
			{
				s32RetVal = JPEG_HDEC_SendStreamFromVirMem(cinfo);
			}
		}

		if(MT_FAILURE == s32RetVal)
		{
			 return MT_FAILURE;
		}

		
		/**
		 ** the jpeg hard decode finish
		 ** CNcomment: jpeg硬件解码完成 CNend\n
		 **/	
		pJpegHandle->bHdecEnd  = MT_TRUE;

		return MT_SUCCESS;
		
}

MT_VOID MT_JPEG_IfHardDec(const struct jpeg_decompress_struct *cinfo,MT_BOOL *pHardDec)
{

    JPEG_HDEC_HANDLE_S_PTR	pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
    if(MT_TRUE == pJpegHandle->bHdecEnd)
    {
       *pHardDec  = MT_TRUE;
    }
	else
	{
	   *pHardDec  = MT_FALSE;
	}

}

/*****************************************************************************
* func			: JPEG_HDEC_SetComponent
* description	: set components
				  CNcomment: 设置组件数 CNend\n
* param[in] 	: cinfo 	 CNcomment: 解码对象 CNend\n
* retval		: MT_SUCCESS CNcomment: 成功	 CNend\n
* retval		: MT_FAILURE CNcomment: 失败	 CNend\n
* others:		: NA
*****************************************************************************/
#if defined(CONFIG_JPEG_CSC_DISABLE) || defined(CONFIG_JPEG_HARDDEC2ARGB)
static MT_VOID JPEG_HDEC_SetCompoent(j_decompress_ptr cinfo)
{
	
	switch(cinfo->out_color_space)
	{
	    case JCS_CMYK:
		case JCS_ARGB_8888:
		case JCS_ABGR_8888:
		#ifdef CONFIG_JPEG_ADD_GOOGLEFUNCTION
		case JCS_RGBA_8888:
		#endif
			cinfo->output_components = 4;
			break;
		case JCS_RGB:
		case JCS_BGR:
		case JCS_CrCbY:
			cinfo->output_components = 3;
			break;
		case JCS_ARGB_1555:
		case JCS_ABGR_1555:
		case JCS_RGB_565:
		case JCS_BGR_565:
			cinfo->output_components = 2;
			break;
		default:
			break;
	}
	
}
#endif

/*****************************************************************************
* func			: JPEG_HDEC_HardCSC
* description	: use hard csc
				  CNcomment: 使用硬件进行颜色空间?? CNend\n
* param[in] 	: cinfo 	 CNcomment: 解码对象 CNend\n
* retval		: MT_SUCCESS CNcomment: 成功	 CNend\n
* retval		: MT_FAILURE CNcomment: 失败	 CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_HardCSC(j_decompress_ptr cinfo)
{
#ifndef CONFIG_JPEG_CSC_DISABLE
		/** have set value,so should not init **/
		TDE2_MB_S		SrcSurface;
		TDE2_SURFACE_S	DstSurface;
		TDE2_RECT_S  SrcRect,DstRect;

		TDE2_MBOPT_S  stMbOpt;
		TDE_HANDLE s32Handle;
		MT_S32 s32Ret   =  MT_SUCCESS;

		TDE2_MB_COLOR_FMT_E enMbFmt[6] = 
		{
			TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP,
			TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP,
			TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP,
			TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP,
			TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP,
			TDE2_MB_COLOR_FMT_BUTT,
		};

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
			/** 认为TDE转换成功了 **/
			pJpegHandle->bCSCEnd = MT_TRUE;
			JPEG_HDEC_SetCompoent(cinfo);
			return MT_SUCCESS;
		}
#endif

		if( (MT_TRUE == pJpegHandle->bOutYCbCrSP)||(MT_TRUE == pJpegHandle->bCSCEnd))
		{/**
		  ** no need tde csc,only add the sanlines
		  ** CNcomment: 不需要TDE转换了，只需要增加行数 CNend\n
		  **/
			return MT_SUCCESS;
		}

        /**
        ** if has image quality quest,you should change the tde coef
        ** CNcomment: 如果有图片质量的需求就要通过make menuconfig来修改TDE系数，默认没有分配好内存 CNend\n
        **/

        /**
        ** src data from jpeg hard dec output
        ** CNcomment: jpeg 硬件解码的输出数据 CNend\n
        **/
        SrcSurface.u32YPhyAddr    = pJpegHandle->stMiddleSurface.pMiddlePhy[0];
        SrcSurface.u32CbCrPhyAddr = pJpegHandle->stMiddleSurface.pMiddlePhy[1];
        if(pJpegHandle->enImageFmt == JPEG_FMT_CMYK)
            SrcSurface.enMbFmt = TDE2_MB_COLOR_FMT_JPG_SP_CMYK;
        else
            SrcSurface.enMbFmt        = enMbFmt[pJpegHandle->enImageFmt];
//      SrcSurface.u32YWidth      = pJpegHandle->stJpegSofInfo.u32DisplayW;
//      SrcSurface.u32YHeight     = pJpegHandle->stJpegSofInfo.u32DisplayH;
//      SrcSurface.u32YStride     = pJpegHandle->stJpegSofInfo.u32YStride;
//      SrcSurface.u32CbCrStride  = pJpegHandle->stJpegSofInfo.u32CbCrStride;
        
        SrcSurface.u32YWidth      = pJpegHandle->stJpegSofInfo.u32YOutWidth;
        SrcSurface.u32YHeight     = pJpegHandle->stJpegSofInfo.u32YOutHeight;
        SrcSurface.u32YStride     = pJpegHandle->stJpegSofInfo.u32YOutStride;
        SrcSurface.u32CbCrStride  = pJpegHandle->stJpegSofInfo.u32COutStride;

#if 0
//在JPEG_HDEC_GetYUVMem里，将CONFIG_JPEG_TEST_SAVE_YUVSP_DATA替成打开状态，否则pMiddleVir[0]为0
		{
			MT_U32 i;
			MT_U32 *ptr1, *ptr2;
			ptr1 = (MT_U32 *)(pJpegHandle->stMiddleSurface.pMiddleVir[0]);
			ptr2 = (MT_U32 *)(pJpegHandle->stMiddleSurface.pMiddleVir[1]);
			for(i = 0; i < 100; i++)
				printf("\r\n 0x%08x", ptr1[i]);
		
		printf("\r\n ~~~~~~~~~~~~uv");
				for(i = 0; i < 100; i++)
				printf("\r\n 0x%08x", ptr2[i]);
		}
#endif


#if 0
		JPEG_TRACE("============================================================================\n");
        /** crop debug **/
		JPEG_TRACE("SrcSurface.u32YWidth      = %d\n",SrcSurface.u32YWidth);
		JPEG_TRACE("SrcSurface.u32YHeight     = %d\n",SrcSurface.u32YHeight);
		JPEG_TRACE("SrcSurface.u32YStride     = %d\n",SrcSurface.u32YStride);
		JPEG_TRACE("SrcSurface.u32CbCrStride  = %d\n",SrcSurface.u32CbCrStride);
		JPEG_TRACE("============================================================================\n");
#endif
		/**
		** tde csc output data,not use memset, because the memset cost many times.
		** CNcomment: tde转换之后的输出数据 CNend\n
		**/
		switch(cinfo->out_color_space)
		{
			case JCS_RGB:
				DstSurface.enColorFmt    = TDE2_COLOR_FMT_BGR888;
				cinfo->output_components = 4;
				break; 
			case JCS_BGR:
				DstSurface.enColorFmt	 = TDE2_COLOR_FMT_RGB888;
				cinfo->output_components = 4;
				break; 
			case JCS_ARGB_8888:
			#ifdef CONFIG_JPEG_ADD_GOOGLEFUNCTION
			case JCS_RGBA_8888:
			#endif
				DstSurface.enColorFmt	 = TDE2_COLOR_FMT_ABGR8888;
				cinfo->output_components = 4;
				break;
			case JCS_ABGR_8888:
				DstSurface.enColorFmt	 = TDE2_COLOR_FMT_ARGB8888;
				cinfo->output_components = 4;
				break;
			case JCS_ARGB_1555:
				DstSurface.enColorFmt	 = TDE2_COLOR_FMT_ABGR1555;
				cinfo->output_components = 2;
				break;
			case JCS_ABGR_1555:
				DstSurface.enColorFmt	 = TDE2_COLOR_FMT_ARGB1555;
				cinfo->output_components = 2;
				break;
			case JCS_RGB_565:
				#ifdef CONFIG_JPEG_LITTLE_TRANSFORM_BIG_ENDIAN
				DstSurface.enColorFmt    = TDE2_COLOR_FMT_RGB565;
				#else
				DstSurface.enColorFmt    = TDE2_COLOR_FMT_BGR565;
				#endif
				cinfo->output_components = 2;
				break;
			case JCS_BGR_565:
				DstSurface.enColorFmt	  = TDE2_COLOR_FMT_RGB565;
				cinfo->output_components = 2;
				break;
			case JCS_CrCbY:
				DstSurface.enColorFmt    = TDE2_COLOR_FMT_YCbCr888;
				cinfo->output_components = 3;
				break; 
			case JCS_CMYK:
				DstSurface.enColorFmt    = TDE2_COLOR_FMT_ABGR8888;
				cinfo->output_components = 4;
				break; 				
			default:
				return MT_FAILURE;
		}

        DstSurface.u32PhyAddr     = pJpegHandle->stMiddleSurface.pOutPhy;
        if(MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem)
        {
            DstSurface.u32Stride      = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];
        }
        else
        {
            DstSurface.u32Stride      = pJpegHandle->stJpegSofInfo.u32DisplayStride;
        }
        DstSurface.u32Width       = (MT_U32)pJpegHandle->stOutDesc.stCropRect.w;
        DstSurface.u32Height      = (MT_U32)pJpegHandle->stOutDesc.stCropRect.h;
        DstSurface.pu8ClutPhyAddr = 0;
        DstSurface.bYCbCrClut     = MT_FALSE;
        DstSurface.bAlphaMax255   = MT_TRUE;
        DstSurface.bAlphaExt1555  = MT_TRUE;
        DstSurface.u8Alpha0       = 0;
        DstSurface.u8Alpha1       = 255;
        DstSurface.u32CbCrPhyAddr = 0;
        DstSurface.u32CbCrStride  = 0;

		/**
		 ** if the rect equal with the output size, that has been crop.other has no crop.
		 ** CNcomment:是否有裁剪是看rect大小,要是和输出大小保持一致就没有裁剪 CNend\n
		 **/
		SrcRect.s32Xpos   = pJpegHandle->stOutDesc.stCropRect.x;
		SrcRect.s32Ypos   = pJpegHandle->stOutDesc.stCropRect.y;
		SrcRect.u32Width  = (MT_U32)pJpegHandle->stOutDesc.stCropRect.w;
		SrcRect.u32Height = (MT_U32)pJpegHandle->stOutDesc.stCropRect.h;
		DstRect.s32Xpos   = 0;
		DstRect.s32Ypos   = 0;
		DstRect.u32Width  = (MT_U32)pJpegHandle->stOutDesc.stCropRect.w;
		DstRect.u32Height = (MT_U32)pJpegHandle->stOutDesc.stCropRect.h;

#if 0
		JPEG_TRACE("============================================================================\n");
        /** crop debug **/
		JPEG_TRACE("DstSurface.u32Width  = %d\n",DstSurface.u32Width);
		JPEG_TRACE("DstSurface.u32Height = %d\n",DstSurface.u32Height);
		JPEG_TRACE("DstSurface.u32Stride = %d\n",DstSurface.u32Stride);
		JPEG_TRACE("DstRect.s32Xpos      = %d\n",DstRect.s32Xpos);
		JPEG_TRACE("DstRect.s32Ypos      = %d\n",DstRect.s32Ypos);
		JPEG_TRACE("DstRect.u32Width     = %d\n",DstRect.u32Width);
		JPEG_TRACE("DstRect.u32Height    = %d\n",DstRect.u32Height);
		JPEG_TRACE("============================================================================\n");
#endif
		/**
		**这个操作性能会变差，但是消告警
		**/
		memset(&stMbOpt,0,sizeof(TDE2_MBOPT_S));
		stMbOpt.enResize   = TDE2_MBRESIZE_QUALITY_LOW;
		//stMbOpt.bDeflicker = MT_TRUE;

		
		if ((s32Handle = JPEG_HDEC_CSC_BeginJob(pJpegHandle->s32CscDev)) != MT_ERR_TDE_INVALID_HANDLE)
		{
			s32Ret = JPEG_HDEC_CSC_MbBlit(s32Handle, &SrcSurface, &SrcRect, &DstSurface, &DstRect, &stMbOpt,pJpegHandle->s32CscDev);
			if(MT_SUCCESS != s32Ret)
			{
			    JPEG_TRACE("==== JPEG_HDEC_CSC_MbBlit Failure,s32Ret = 0x%x!\n",s32Ret);
				return MT_FAILURE;
			}
			/**
			** if MT_TRUE,is no sync. and MT_FALSE you should call tde wait for done to
			** waite the tde work finish.
			** CNcomment:MT_TRUE 阻塞，要是非阻塞要调用waitfordone等待TDE操作完成 CNend\n
			**/
			s32Ret = JPEG_HDEC_CSC_EndJob(s32Handle, MT_FALSE, MT_TRUE, 10000,pJpegHandle->s32CscDev);
			if(MT_SUCCESS != s32Ret)
			{
			    JPEG_TRACE("==== JPEG_HDEC_CSC_EndJob Failure,s32Ret = 0x%x!\n",s32Ret);
				return MT_FAILURE;
			}
		}

		pJpegHandle->bCSCEnd = MT_TRUE;

#ifdef CONFIG_JPEG_TEST_SAVE_BMP_PIC
		MT_JPEG_SaveBmp(DstSurface.u32PhyAddr, \
						DstRect.u32Width,      \
						DstRect.u32Height,     \
						DstSurface.u32Stride,  \
						cinfo);
#endif


#else
		#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
			/** 认为TDE转换成功了 **/
			pJpegHandle->bCSCEnd = MT_TRUE;
		}
		#endif
		JPEG_HDEC_SetCompoent(cinfo);
#endif
		return MT_SUCCESS;
		  

}

/*****************************************************************************
* func			: JPEG_HDEC_CheckCpy
* description	: check whethe should cpy to user buffer
				  CNcomment:  确认是否需要拷贝到用户 buffer中 CNend\n
* param[in] 	: cinfo 	 CNcomment:  解码对象  CNend\n
* param[out] 	: max_lines  CNcomment:  解码行数  CNend\n
* retval		: MT_SUCCESS CNcomment:  成功	   CNend\n
* retval		: MT_FAILURE CNcomment:  失败	   CNend\n
* others:		: NA
*****************************************************************************/
static MT_S32 JPEG_HDEC_CheckCpy(j_decompress_ptr cinfo,JDIMENSION max_lines)
{
	MT_U32 u32Cnt			 = 0;
	JPEG_HDEC_HANDLE_S_PTR	pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

	if(    (MT_FALSE == pJpegHandle->bCSCEnd)
		|| (MT_TRUE == pJpegHandle->stOutDesc.stOutSurface.bUserPhyMem))
	{  
		/**
		** not use tde convert or use physics buffer,so not output the usr buffer
		** CNcomment: tde转换失败或者使用物理内存，所以不需要输出到用户buffer中 CNend\n
		**/
		for(u32Cnt=0; u32Cnt<max_lines; u32Cnt++)
		{
			(cinfo->output_scanline)++;
		}
		return (MT_S32)max_lines;
	}

	if(   (MT_TRUE == pJpegHandle->stOutDesc.bCrop)
		&&( ((MT_S32)(cinfo->output_scanline) < pJpegHandle->stOutDesc.stCropRect.y)
		   ||((MT_S32)(cinfo->output_scanline+1) > (pJpegHandle->stOutDesc.stCropRect.h + pJpegHandle->stOutDesc.stCropRect.y))))
	{
		for(u32Cnt=0; u32Cnt<max_lines; u32Cnt++)
		{
			(cinfo->output_scanline)++;
		}
		return (MT_S32)max_lines;
	}

	return MT_FAILURE;

}

/*****************************************************************************
* func			: JPEG_HDEC_OutUserBuf
* description	: output the scanlines buffer
				  CNcomment:  输出到用户行buffer中 CNend\n
* param[in] 	: cinfo 	 CNcomment:  解码对象  CNend\n
* param[out] 	: max_lines  CNcomment:  解码行数  CNend\n
* param[out]	: scanlines  CNcomment:  行buffer  CNend\n
* retval		: MT_SUCCESS CNcomment:  成功	   CNend\n
* retval		: MT_FAILURE CNcomment:  失败	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_OutUserBuf(j_decompress_ptr cinfo,JDIMENSION max_lines, MT_CHAR *scanlines)
{


		MT_U32 u32Cnt           = 0;
		MT_U32 u32SrcStride     = 0;
		MT_U32 u32DstStride     = 0;
		MT_S32 s32BufSrcLength  = 0;
		MT_CHAR *pDstBuf         = NULL;
		MT_CHAR *pSrcBuf         = NULL;
		MT_S32 s32Ret            = 0;
		
		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		if ((max_lines+(cinfo->output_scanline)) > (cinfo->output_height))
		{
			max_lines = (cinfo->output_height) - (cinfo->output_scanline);
		}

		s32Ret = JPEG_HDEC_CheckCpy(cinfo,max_lines);
		if(MT_FAILURE != s32Ret)
		{
			return s32Ret;
		}
		
		/**
		** is not set output description,so is output scanlines buffer
		** CNcomment:说明没有设置解码输出的属性，是输出到行buffer中 CNend\n
		**/
//		u32SrcStride  = pJpegHandle->stJpegSofInfo.u32YOutStride;
		u32SrcStride  = pJpegHandle->stJpegSofInfo.u32DisplayStride;
		u32DstStride  = pJpegHandle->stOutDesc.stOutSurface.u32OutStride[0];


#ifdef  CONFIG_JPEG_HARDDEC2ARGB
		if(MT_TRUE == pJpegHandle->bDecARGB)
		{
			pSrcBuf   = pJpegHandle->stMiddleSurface.pMiddleVir[0] + ((MT_S32)cinfo->output_scanline - pJpegHandle->stOutDesc.stCropRect.y) * (MT_S32)(u32SrcStride);
		}
		else
#endif                   
		{
			pSrcBuf   = pJpegHandle->stMiddleSurface.pOutVir + ((MT_S32)cinfo->output_scanline - pJpegHandle->stOutDesc.stCropRect.y) * (MT_S32)(u32SrcStride);
		}

		if(NULL != scanlines)
		{
			pDstBuf   = scanlines;
		}
		else
		{
			pDstBuf   = pJpegHandle->stOutDesc.stOutSurface.pOutVir[0] + ((MT_S32)cinfo->output_scanline - pJpegHandle->stOutDesc.stCropRect.y) * (MT_S32)(u32DstStride);
		}
		/**
		** data size in reality
		** CNcomment: 实际的数据大小 CNend\n
		**/
		s32BufSrcLength = (cinfo->output_components) * (pJpegHandle->stOutDesc.stCropRect.w);
		for(u32Cnt = 0; u32Cnt < max_lines; u32Cnt++)
		{
			memcpy(pDstBuf,pSrcBuf,(size_t)s32BufSrcLength);
			(cinfo->output_scanline)++;
		}

#if 0
		if(cinfo->output_scanline == pJpegHandle->stOutDesc.stCropRect.h - 1)
		{
			JPEG_TRACE("============================================================================\n");
			JPEG_TRACE("s32BufSrcLength  = %d\n",s32BufSrcLength);
			JPEG_TRACE("u32Stride        = %d\n",u32Stride);
			JPEG_TRACE("============================================================================\n");
		}
#endif

		return (MT_S32)max_lines;

}
/*****************************************************************************
* func			: JPEG_HDEC_DuplicateStreamInfo
* description	: save the stream information before into hard decode
				  CNcomment: 在进入硬件解码之前保存码流信息，包括码流位置
				  剩余的码流以及剩余码流数。
* param[in] 	: cinfo 	  CNcomment: 解码对象  CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	   CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32	JPEG_HDEC_DuplicateStreamInfo(const struct jpeg_decompress_struct *cinfo)
{
#if 0
		my_src_ptr src = (my_src_ptr)cinfo->src; /*lint !e740 !e826 ignore by y00181162, because tmts is needed */  

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUseFileData)
		{  /**
			** only use file stream or use external stream should save the data.
			** because the mem stream decode, the hard has no change the 
			** cinfo->src->next_input_byte buffer and leave data.
			** CNcomment: 使用文件码流才需要回退，因为内存码流硬件解码的时候
			**            没有使用cinfo->src->next_input_byte这块临时buffer以及
			**            没有改变cinfo->src->bytes_in_buffer剩余码流大小 CNend\n
			**/
			pJpegHandle->stJpegHtoSInfo.u32FilePos = (MT_U32)ftell(src->infile);
			memcpy(pJpegHandle->stJpegHtoSInfo.pLeaveBuf,	\
			(char*)cinfo->src->next_input_byte,		\
			cinfo->src->bytes_in_buffer);
			pJpegHandle->stJpegHtoSInfo.u32LeaveByte = cinfo->src->bytes_in_buffer;

		}
#endif
		return MT_SUCCESS;

}

/*****************************************************************************
* func			: JPEG_HDEC_ResumeStreamInfo
* description	: resume the stream information when hard decode failure,and
				  then into soft decode
				  CNcomment: 当硬件解码失败的时候恢复原先保存的码流信息，然后
				  继续进行软件解码
* param[in] 	: cinfo 	  CNcomment: 解码对象  CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	   CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32	JPEG_HDEC_ResumeStreamInfo(j_decompress_ptr cinfo)
{
#if 0

		MT_S32 s32Ret;
		my_src_ptr src = (my_src_ptr) cinfo->src; /*lint !e740 !e826 ignore by y00181162, because tmts is needed */  

		JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

		if(MT_TRUE == pJpegHandle->stHDecDataBuf.bUseFileData)
		{
			s32Ret = fseek(src->infile,(long)pJpegHandle->stJpegHtoSInfo.u32FilePos,SEEK_SET);
			if(MT_SUCCESS != s32Ret)
			{ /**
			   ** the stream back failure,not soft decode again
			   ** CNcomment: 码流回退错误，不需要在进行软件解码了 CNend\n
			   **/
				ERREXIT(cinfo, JERR_STREAM_BACK_FAILURE); /*lint !e740  ignore by y00181162, because tmts function is macro */  
			}
			memcpy((char*)cinfo->src->next_input_byte,		   \
				   pJpegHandle->stJpegHtoSInfo.pLeaveBuf,	   \
				   pJpegHandle->stJpegHtoSInfo.u32LeaveByte);
			
			cinfo->src->bytes_in_buffer = pJpegHandle->stJpegHtoSInfo.u32LeaveByte;
			
		}
#endif
		return MT_SUCCESS;
		
}
/*****************************************************************************
* func			: JPEG_HDEC_CheckStreamMemType
* description	: check the stream buffer type, if user no call the function
				  of set stream buffer type,call tmts function
				  CNcomment: 查询码流buffer类型，是连续的物理内存，还是虚拟
				  内存，要是用户没有调用设置码流buffer类型，调用该接口
* param[in] 	: cinfo 	  CNcomment: 解码对象  CNend\n
* param[in] 	: pVirBuf	  CNcomment: 虚拟内存  CNend\n
* retval		: MT_SUCCESS  CNcomment: 成功	   CNend\n
* retval		: MT_FAILURE  CNcomment: 失败	   CNend\n
* others:		: NA
*****************************************************************************/
MT_S32 JPEG_HDEC_CheckStreamMemType(const struct jpeg_decompress_struct *cinfo,MT_UCHAR* pVirBuf)
{
     ulong u32Size     = 0;
	 MT_S32 s32Ret      = MT_SUCCESS;
	 phys_addr_t u32PhyAddr  = 0;

	 JPEG_HDEC_HANDLE_S_PTR  pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);

	 #ifdef CONFIG_JPEG_USE_PRIVATE_MMZ
     s32Ret = MT_GFX_GetPhyaddr(pJpegHandle->s32MMZDev,(MT_VOID*)pVirBuf, &u32PhyAddr,&u32Size, NULL);
	 #else
	 s32Ret = MT_GFX_GetPhyaddr((MT_VOID*)pVirBuf, &u32PhyAddr,&u32Size);
	 #endif
	 if(MT_SUCCESS == s32Ret)
	 {
          pJpegHandle->stHDecDataBuf.bUserPhyMem = MT_TRUE;
		  pJpegHandle->stHDecDataBuf.pDataPhyBuf = u32PhyAddr;
		  return MT_SUCCESS;
	 }
	 else
	 {
	     return MT_FAILURE;
	 }
	 
}

MT_S32 JPEG_GetJpegCropRect(j_decompress_ptr cinfo, MT_JPEG_RECT_S *pCropRect)
{
	JPEG_HDEC_HANDLE_S_PTR pJpegHandle = (JPEG_HDEC_HANDLE_S_PTR)(cinfo->client_data);
	if(pJpegHandle == NULL)
		return MT_FAILURE;
	pCropRect->x = pJpegHandle->stOutDesc.stCropRect.x;
	pCropRect->y = pJpegHandle->stOutDesc.stCropRect.y;
	pCropRect->w = pJpegHandle->stOutDesc.stCropRect.w;
	pCropRect->h = pJpegHandle->stOutDesc.stCropRect.h;

	MT_INFO_JPEG("\r\n get crop:%d,%d,%d,%d", pCropRect->x, pCropRect->y,pCropRect->w,pCropRect->h);
	return MT_SUCCESS;
	
}


