/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#ifdef _USE_TP5001_CHIP_
/******************************************************************************** 
* (c) COPYRIGHT 2011 RAONTECH, Inc. ALL RIGHTS RESERVED.
* 
* This software is the property of RAONTECH and is furnished under license by RAONTECH.                
* This software may be used only in accordance with the terms of said license.                         
* This copyright notice may not be removed, modified or obliterated without the prior                  
* written permission of RAONTECH, Inc.                                                                 
*                                                                                                      
* This software may not be copied, transmitted, provided to or otherwise made available                
* to any other person, company, corporation or other entity except as specified in the                 
* terms of said license.                                                                               
*                                                                                                      
* No right, title, ownership or other interest in the software is hereby granted or transferred.       
*                                                                                                      
* The information contained herein is subject to change without notice and should 
* not be construed as a commitment by RAONTECH, Inc.                                                                    
* 
* TITLE 	  : RAONTECH TV RF services header file. 
*
* FILENAME    : raon_rf_mtv600.h
*
* DESCRIPTION : 
*		Library of routines to initialize, and operate on, the RAONTECH RF chip.
*
********************************************************************************/

/******************************************************************************** 
* REVISION HISTORY
*
*    DATE	  	  NAME				REMARKS
* ----------  -------------    --------------------------------------------------
* 11/24/2011  Yang, Maverick   CS version Release.      
* 06/25/2011  Yang, Maverick   Created.                                                              
********************************************************************************/

#ifndef __RAON_RF_MTV600_H__
#define __RAON_RF_MTV600_H__

/*##############################################################################
#
# COMMON configurations
#
################################################################################*/
/*==============================================================================
 * Includes the user header files if neccessry.
 *============================================================================*/ 
 
//Inchude the header file for I2C function and Delay function.
#include "TP5001.h"
#include "mt_type.h"

#ifdef __cplusplus 
extern "C"{ 
#endif  

/*==============================================================================
 * The slave address for I2C
 *============================================================================*/ 
#define RAONTV_CHIP_ADDR	0x61   //0xC2 


/*##############################################################################
#
# Host Interface specific configurations
#
################################################################################*/

//	unsigned char mtv600_i2c_read(U8 reg);
//	void mtv600_i2c_write(U8 reg, U8 val);
	
#define	RTV_REG_GET(reg)            		//mtv600_i2c_read((U8)(reg))
#define	RTV_REG_SET(reg, val)       	//	mtv600_i2c_write((U8)(reg), (U8)(val))
#define	RTV_REG_MASK_SET(reg, mask, val) 								\
		do {																\
			U8 tmp;															\
			tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));	\
			RTV_REG_SET(reg, tmp);											\
		} while(0)

/*==============================================================================
 * Defines the delay macro in milliseconds.
 *============================================================================*/  
#define RTV_DELAY_MS(ms)   // Sleep(ms) 

/*##############################################################################
#
# HardWare depended configurations
#
################################################################################*/
/*==============================================================================
 * X-TAL frequency unit : kHz
 *============================================================================*/ 
#define RTV_SRC_CLK_FREQ_KHz			16000

/*==============================================================================
 * BBAGC Polarity selection : Normal or Inverse
*============================================================================*/ 
#define RTV_AGC_POL_INVERSE
//#define RTV_AGC_POL_NORMAL

#if defined (RTV_AGC_POL_INVERSE) && defined (RTV_AGC_POL_NORMAL)
    #error "Must define AGC polarity just one type" 
#endif
/*==============================================================================
 * Error Code
 *============================================================================*/ 
#define RTV_SUCCESS							0
#define RTV_INVAILD_LPF_BW_TYPE				-1
#define RTV_INVAILD_FREQUENCY_RANGE			-2
#define RTV_INVAILD_RF_BAND                 -3




#define RTV_RSSI_DIVIDER 10.0 

TP_INT32  rtvRF_SetFrequency( TP_UINT32 dwFreqKHz,TP_UINT32 dwSymbolRateKsps, TP_UINT8 uRollOff);
TP_INT32  rtvRF_Initilize(TP_UINT32 dwSymbolRateKsps, TP_UINT8 uRollOff);
TP_INT32  rtvRF_GetRSSI(void);

#ifdef __cplusplus 
} 
#endif 

#endif /* __RAON_RF_MTV600_H__ */
#endif

