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
* This copyright noitce may not be remoced, modified or obliterated without the prior                  
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
* TITLE 	  : RAONTECH TV RF services source file. 
*
* FILENAME    : raon_rf_mtv600.c
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
#include "raon_rf_mtv600.h"
#include "TP5001.h"

typedef int                 INT;
typedef unsigned int        UINT;
typedef long                LONG;
typedef unsigned long       ULONG;

typedef volatile U8			VU8;
typedef volatile U16		VU16;
typedef volatile U32		VU32;

/*==============================================================================
 * ENUM define
 *============================================================================*/
typedef struct
{
	U8	bReg;
	U8	bVal;
}RTV_REG_INIT_INFO;

static const RTV_REG_INIT_INFO t_MTV600_INIT[] =   
{
	{0x20,	0x0b},
	{0x21,	0xb8},
	{0x22,	0x09},
	{0x24,	0x20},
	{0x25,	0x00},
	{0x26,	0x81},
	{0x27,	0x20},
	{0x28,	0xe2},
	{0x29,	0xd4},
	{0x2a,	0x02},
	{0x2b,	0x0c},
	{0x2c,	0x00},
	{0x2d,	0x2f},
	{0x2e,	0x10},
	{0x2f,	0x80},
	{0x30,	0x10},
	{0x31,	0x8c},
	{0x32,	0x16},
	{0x33,	0x34},
	{0x34,	0x30},
	{0x35,	0x4a},
	{0x36,	0x81},
	{0x37,	0x92},
	{0x38,	0x90},
	{0x39,	0x18},
	{0x3a,	0x00},
	{0x3b,	0x44},
	{0x3c,	0x92},
	{0x3d,	0x85},
	{0x3e,	0x00},
	{0x3f,	0x00},
	{0x40,	0xff},
	{0x41,	0x57},
	{0x42,	0x9a},
	{0x43,	0x04}
}; 

volatile U8 g_aeLpfBwType;

INT Setmaskreg(U8 reg,U8 mask,U8 value)
{
	INT nRet;
	U8 regvalue[3];
	//tmp = (RTV_REG_GET(reg)|(U8)(mask)) & (U8)((~(mask))|(val));

	nRet = TP_iic_tuner_read(RAONTV_CHIP_ADDR, reg,regvalue,1);
	if(nRet != 0)
		return nRet;
	regvalue[1] = (regvalue[0]|(U8)(mask)) & (U8)((~(mask))|(value));

	regvalue[0] = reg;

	nRet = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(nRet != 0)
		return nRet;

	nRet = RTV_SUCCESS;

	return nRet;
}

static INT rtvRF_ChangeLpfBwType(U32 dwSymbolRateKsps, U8 uRollOff)
{	
    U8 uReg26=0;
	U8 uCalculatedLPF=0;
	U32 dwSymbolrateReconfig=0;
	INT result;
	U8 regvalue[4];

	dwSymbolrateReconfig = dwSymbolRateKsps * (100 + uRollOff);
    dwSymbolrateReconfig = (dwSymbolrateReconfig >> 1) * 120;

    uCalculatedLPF = (dwSymbolrateReconfig/10000000) + 1;   //ksps(1000) * rolloff(100) * 120(100) = 10000000 is to be MHz of LPF Setting.// +1 means ceil().

    if((4 > uCalculatedLPF) || (40 < uCalculatedLPF))       //LPF Setting unit is MHz with from 4MHz to 40MHz boundary.  
   	    return RTV_INVAILD_LPF_BW_TYPE;

	if(g_aeLpfBwType == uCalculatedLPF)
	return RTV_SUCCESS;

    //uReg26 = (RTV_REG_GET(0x26) & 0x3F);
	result = TP_iic_tuner_read(RAONTV_CHIP_ADDR, 0x26,&uReg26,1);
	if(result != 0)
		return result;

	uReg26 = uReg26 & 0x3F;
		
	//RTV_REG_SET(0x26,((uCalculatedLPF & 0x01)<<6 ) | ((uCalculatedLPF & 0x20)<<2 ) | uReg26);
	regvalue[0] = 0x26;
	regvalue[1] = ((uCalculatedLPF & 0x01)<<6 ) | ((uCalculatedLPF & 0x20)<<2 ) | uReg26;
	result = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(result != 0)
		return result;

	//RTV_REG_MASK_SET(0x23,0x0C,((uCalculatedLPF & 0x06)<<1));
	result = Setmaskreg(0x23,0x0c,((uCalculatedLPF & 0x06)<<1));
	if(result != 0)
		return result;

	//RTV_REG_MASK_SET(0x22,0x18,(uCalculatedLPF & 0x18));
	result = Setmaskreg(0x22,0x18,(uCalculatedLPF & 0x18));
	if(result != 0)
	
	g_aeLpfBwType = uCalculatedLPF;
	
	return RTV_SUCCESS;	
}

static INT rtvRF_SetVcoBand(U32 dwLoFreq)
{
    INT nRet;
	U8 nVcoBandI2C;
//	U8 regvalue[3];
	
	if((dwLoFreq > 900000)&&(dwLoFreq < 1175000)) 
	{
	 	nVcoBandI2C = 2;
	}
    else if((dwLoFreq >= 1175000)&&(dwLoFreq < 1750000)) 
    {
	 	nVcoBandI2C = 1;
    }
	else if((dwLoFreq >= 1750000)&&(dwLoFreq < 2200000)) 
	{
     	nVcoBandI2C = 0;
	}	
	else 
		return  RTV_INVAILD_FREQUENCY_RANGE;

	//RTV_REG_MASK_SET(0x23, 0xC0,nVcoBandI2C << 6 );
	nRet = Setmaskreg(0x23,0xc0,nVcoBandI2C << 6 );
	if(nRet != 0)
		return nRet;

	nRet = RTV_SUCCESS;
	
	return nRet;
}


INT rtvRF_SetFrequency( U32 dwChFreqKHz,U32 dwSymbolRateKsps, U8 uRollOff)
{

	U8 pllf_mul=0, r_div=4;
	U32 dwPLLN,dwPLLF, dwPLLNF; 
	U8 regvalue[3],nRet;
		
	if(rtvRF_ChangeLpfBwType(dwSymbolRateKsps,uRollOff)!= RTV_SUCCESS)
		return RTV_INVAILD_LPF_BW_TYPE;
	
	if( rtvRF_SetVcoBand( dwChFreqKHz) != RTV_SUCCESS)
		return RTV_INVAILD_FREQUENCY_RANGE;

	dwPLLN = ( dwChFreqKHz / RTV_SRC_CLK_FREQ_KHz  );		
	dwPLLF = dwChFreqKHz -(dwPLLN * RTV_SRC_CLK_FREQ_KHz);
	if (RTV_SRC_CLK_FREQ_KHz==13000 || RTV_SRC_CLK_FREQ_KHz==27000)
	{
		pllf_mul=1;
		r_div=3;
	}
	
	//dwPLLNF = (dwPLLN << 20 ) + (  ((dwPLLF <<16) / (RTV_SRC_CLK_FREQ_KHz>>r_div))  << pllf_mul);
	dwPLLNF = ((dwPLLN << 20 ) + (  ((dwPLLF <<16) / (RTV_SRC_CLK_FREQ_KHz>>r_div))  << pllf_mul)) >> 1;



	 /* Important :  Do not modify the writing sequence */
	//RTV_REG_SET(0x25, ((dwPLLNF>>29) & 0x01));
	regvalue[0] = 0x25;
    regvalue[1] = ((dwPLLNF>>29) & 0x01);
	nRet = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(nRet != 0)
		return nRet;
	
	//RTV_REG_MASK_SET(0x31, 0x01, ((dwPLLNF>>28) & 0x01));
	nRet = Setmaskreg(0x31,0x01,((dwPLLNF>>28) & 0x01) );
	if(nRet != 0)
		return nRet;
	

	//RTV_REG_MASK_SET(0x20, 0x1F, (dwPLLNF>>23) & 0x1F);
	nRet = Setmaskreg(0x20,0x1f,(dwPLLNF>>23) & 0x1F );
	if(nRet != 0)
		return nRet;
	
	/*Address 0x21 must be written as last of PLL value setting sequence*/	
	//RTV_REG_SET(0x21, (dwPLLNF>>15) & 0xFF);
	regvalue[0] = 0x21;
    regvalue[1] = (dwPLLNF>>15) & 0xFF;
	nRet = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(nRet != 0)
		return nRet;
	TP_Delay(1);  //1ms Delay	
		
	return RTV_SUCCESS;
}

static INT rtvRF_ConfigureLpfBwType(U32 dwSymbolRateKsps, U8 uRollOff)
{

	U8 uReg26=0,result;
	U8 uCalculatedLPF=0;
	U32 dwSymbolrateReconfig=0;
	U8 regvalue[3];

	dwSymbolrateReconfig = dwSymbolRateKsps * (100 + uRollOff);
    dwSymbolrateReconfig = (dwSymbolrateReconfig >> 1) * 120;

    uCalculatedLPF = (dwSymbolrateReconfig/10000000) + 1;   //ksps(1000) * rolloff(100) * 120(100) = 10000000 is to be MHz of LPF Setting. +1 means ceil().

    if((4 > uCalculatedLPF) || (40 < uCalculatedLPF))       //LPF Setting unit is MHz with from 4MHz to 40MHz boundary.  
   	    return RTV_INVAILD_LPF_BW_TYPE;

    //uReg26 = (RTV_REG_GET(0x26) & 0x3F);
	result = TP_iic_tuner_read(RAONTV_CHIP_ADDR, 0x26,regvalue,1);
	if(result != 0)
		return result;
	uReg26 = regvalue[0] & 0x3F;

		
	//RTV_REG_SET(0x26,((uCalculatedLPF & 0x01)<<6 ) | ((uCalculatedLPF & 0x20)<<2 ) | uReg26);
	regvalue[0] = 0x26;
	regvalue[1] = ((uCalculatedLPF & 0x01)<<6 ) | ((uCalculatedLPF & 0x20)<<2 ) | uReg26;
	result = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(result != 0)
		return result;


	//RTV_REG_MASK_SET(0x23,0x0C,((uCalculatedLPF & 0x06)<<1));
	result = Setmaskreg(0x23,0x0c,((uCalculatedLPF & 0x06)<<1) );
	if(result != 0)
		return result;

	//RTV_REG_MASK_SET(0x22,0x18,(uCalculatedLPF & 0x18));
	result = Setmaskreg(0x22,0x18,(uCalculatedLPF & 0x18) );
	if(result != 0)
		return result;

	g_aeLpfBwType = uCalculatedLPF;
	
	return RTV_SUCCESS;
}

S32  rtvRF_GetRSSI(void)
{
  
	S32 rssi=0;
	U8 uReg05=0;
	U8 result;
  
 // uReg05 = RTV_REG_GET(0x05);
	result = TP_iic_tuner_read(RAONTV_CHIP_ADDR, 0x05,&uReg05,1);
	if(result != 0)
		return result;

	rssi = (S32)((-0.7*RTV_RSSI_DIVIDER)  * (uReg05 - 60 ));

	return rssi;
}

INT rtvRF_Initilize(U32 dwSymbolRateKsps, U8 uRollOff)
{

	UINT nNumTblEntry=0;
	const RTV_REG_INIT_INFO *ptInitTbl = NULL;
	U8 regvalue[3],result;
	
	ptInitTbl = t_MTV600_INIT;
	nNumTblEntry = sizeof(t_MTV600_INIT) / sizeof(RTV_REG_INIT_INFO);
		
	do
	{
		//RTV_REG_SET(ptInitTbl->bReg, ptInitTbl->bVal);
		regvalue[0] = ptInitTbl->bReg;
		regvalue[1] = ptInitTbl->bVal;
		result = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
		if(result != 0)
			return result;
		ptInitTbl++;						
	} while( --nNumTblEntry );

#if defined(RTV_AGC_POL_INVERSE)
	//RTV_REG_SET(0x23, 0x40);
	regvalue[0] = 0x23;
	regvalue[1] = 0x40;
	result = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(result != 0)
		return result;
#elif defined(RTV_AGC_POL_NORMAL)
	//RTV_REG_SET(0x23, 0x42);
	regvalue[0] = 0x23;
	regvalue[1] = 0x42;
	result = TP_iic_tuner_write(RAONTV_CHIP_ADDR, regvalue, 2);
	if(result != 0)
		return result;
#else 
    #error "AGC Polarity type is not defined"
#endif 
	return rtvRF_ConfigureLpfBwType(dwSymbolRateKsps,uRollOff);
}
#endif
