/******************************************************************************
*
* Copyright (C) 2014 Montage Technologies Co., Ltd.  All rights reserved. 
*
* Tmts program is confidential and proprietary to Montage  Technologies Co., Ltd. (Montage), 
* and may not be copied, reproduced, modified, disclosed to others, published or used, in
* whole or in part, without the express prior written permission of Montage.
*
******************************************************************************
File Name	    : jpeg_hdec_rwreg.c
Version		    : Initial Draft
Author		    : y00181162
Created		    : 2014/06/20
Description	    : write data to register and read data from register
                  CNcomment: 读写寄存器 CNend\n
Function List 	:

			  		  
History       	:
Date				Author        		Modification
2014/06/20		    y00181162		    Created file      	
******************************************************************************/

/*********************************add include here******************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_type.h"
#include "jpeg_hdec_adp.h"
#include "jpeg_hdec_rwreg.h"
#include "mt_drv_jpeg_reg.h"
#include "mpi_memdev.h"

/***************************** Macro Definition ******************************/


/******************** to see which include file we want to use***************/



/*************************** Structure Definition ****************************/



/********************** Global Variable declaration **************************/

/******************************* API forward declarations *******************/

/******************************* API realization *****************************/


/*****************************************************************************
* func			: JPEG_HDEC_WriteReg
* description	: writd data to register
				  CNcomment:  写寄存器									CNend\n
* param[in] 	: pJpegRegVirAddr  CNcomment:  寄存器映射上来的虚拟地址 CNend\n
* param[in] 	: s32PhyOff 	   CNcomment:  偏移地址 				CNend\n
* param[in] 	: s32Val		   CNcomment:  要写的寄存器的值 		CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
mt_void JPEG_HDEC_WriteReg(volatile mt_char *pJpegRegVirAddr, mt_s32 s32PhyOff, mt_s32 s32Val)
{

		volatile mt_s32  *ps32Addr = NULL;

		/**
		** the phycial offset address can not larger than register length
		** CNcomment: 偏移地址不能大于寄存器长度 CNend\n
		**/
		if(s32PhyOff < JPGD_REG_LENGTH)
		{
		    ps32Addr  = (volatile mt_s32*)(pJpegRegVirAddr + s32PhyOff); /*lint !e826 ignore by y00181162, because this is needed */  
			mpi_write_reg32(ps32Addr, (mt_u32)s32Val);
		}
	  
}

/*****************************************************************************
* func			: JPEG_HDEC_ReadReg
* description	: read data from register
				  CNcomment:  读寄存器的值									CNend\n
* param[in] 	: pJpegRegVirAddr	CNcomment:	寄存器映射上来的虚拟地址	CNend\n
* param[in] 	: s32PhyOff 		CNcomment:	偏移地址					CNend\n
* retval		: HI_SUCCESS		CNcomment:	成功						CNend\n
* retval		: HI_FAILURE		CNcomment:	失败						CNend\n
* others:		: NA
*****************************************************************************/
mt_s32 JPEG_HDEC_ReadReg(const volatile mt_char *pJpegRegVirAddr, mt_s32 s32PhyOff)
{

		return s32PhyOff < JPGD_REG_LENGTH ?  mpi_read_reg32((pJpegRegVirAddr + s32PhyOff)) : MT_FAILURE; /*lint !e826 ignore by y00181162, because this is needed */  

}

/*****************************************************************************
* func			: JPEG_HDEC_CpyData2Reg
* description	: copy the mem data to register
				  CNcomment:  将buf中的值写到寄存器中					CNend\n
* param[in] 	: pJpegRegVirAddr  CNcomment:  寄存器映射上来的虚拟地址 CNend\n
* param[in] 	: pInMem		   CNcomment:  要输出的buffer地址		CNend\n
* param[in] 	: s32PhyOff 	   CNcomment:  偏移地址 				CNend\n
* param[in] 	: u32Bytes		   CNcomment:  要写的字节数 			CNend\n
* retval		: NA
* others:		: NA
*****************************************************************************/
mt_void JPEG_HDEC_CpyData2Reg(volatile mt_char *pJpegRegVirAddr,const mt_void *pInMem,mt_s32 s32PhyOff,mt_u32 u32Bytes)
{

      
		mt_u32 u32Cnt = 0;

		for(u32Cnt = 0; u32Cnt < u32Bytes; u32Cnt += 4)
		{
			mpi_write_reg32((pJpegRegVirAddr + s32PhyOff + u32Cnt), *(int *)((char*)pInMem + u32Cnt));/*lint !e826 ignore by y00181162, because this is needed */  
		}

}

/*****************************************************************************
* func			: JPEG_HDEC_CpyData2Buf
* description	: copy the register data to mem
				  CNcomment:  将寄存器中的值写到用户buffer中				CNend\n
* param[in] 	: pJpegRegVirAddr	  CNcomment:  寄存器映射上来的虚拟地址	CNend\n
* param[in] 	: s32PhyOff 		  CNcomment:  偏移地址			  CNend\n
* param[in] 	: u32Bytes			  CNcomment:  要输出的字节数	  CNend\n
* param[out]	: pOutMem			  CNcomment:  输出buffer地址	  CNend\n
* retval		: HI_SUCCESS		  CNcomment:  成功				  CNend\n
* retval		: HI_FAILURE		  CNcomment:  失败				  CNend\n
* others:		: NA
*****************************************************************************/
mt_s32 JPEG_HDEC_CpyData2Buf(const volatile mt_char *pJpegRegVirAddr,const mt_s32 s32PhyOff,const mt_u32 u32Bytes,mt_void *pOutMem)
{

		mt_u32 u32Cnt = 0;

		for(u32Cnt = 0; u32Cnt < u32Bytes; u32Cnt += 4)
		{ 
			mpi_write_reg32(((char*)pOutMem + u32Cnt), *(int *)(pJpegRegVirAddr + s32PhyOff + u32Cnt));/*lint !e826 ignore by y00181162, because this is needed */  
		}
		return MT_SUCCESS;
}
