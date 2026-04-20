/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "sharp6306.h"
#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

TP_UINT8 sharp_6306_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{
	TP_UINT8 PSC_DIV, VCO_DIV, BA;
	TP_UINT8 PD;
	TP_UINT8 reg_value[15];
	TP_UINT8 P, N, A;
	TP_UINT8 ret;
	TP_UINT8 REF;

	// 得到PSC_DIV、VCO_DIV和BA的值
	if(frequency <= 986)
	{
		PSC_DIV = 1; VCO_DIV = 1; BA = 5;
	}
	else if(frequency <= 1073)
	{
		PSC_DIV = 1; VCO_DIV = 1; BA = 6;
	}
	else if(frequency <= 1154)
	{
		PSC_DIV = 0; VCO_DIV = 1; BA = 7;
	}
	else if(frequency <= 1291)
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 1;
	}
	else if(frequency <= 1447)
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 2;
	}
	else if(frequency <= 1615)
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 3;
	}
	else if(frequency <= 1791)
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 4;
	}
	else if(frequency <= 1972)
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 5;
	}
	else
	{
		PSC_DIV = 0; VCO_DIV = 0; BA = 6;
	}
	
//		if(frequency < 1024)
//			REF = 1;
//		else
		REF = 0;

	P = 32 - (PSC_DIV*16);		// 设置P，PSC_DIV = 0时，P = 32     PSC_DIV = 1时，P = 16

	if(REF == 0)
	{
		N = frequency / P;
		A = frequency % P;
	}
	else
	{
		N = (frequency*2) / P;
		A = (frequency*2) % P;
	}

	// 设置带宽
	//Symbol_Rate_Value *= 1.25;
	Symbol_Rate_Value *= 5;
	Symbol_Rate_Value /= 4;

	Symbol_Rate_Value /= 2;
	Symbol_Rate_Value /= 1000000;

	//取带宽偶数
	if(Symbol_Rate_Value % 2)
	{
		Symbol_Rate_Value += 1;
	}

	// 计算PD
	//if(Symbol_Rate_Value < 10 || Symbol_Rate_Value > 34)					// change @ TP01.00.11
	//	return TP_SET_TUNER_ERR;
	if(Symbol_Rate_Value < 10)
		Symbol_Rate_Value = 10;
	else if(Symbol_Rate_Value > 34)
		Symbol_Rate_Value = 34;
	PD = 3 + (Symbol_Rate_Value - 10) / 2;

	// 组合发送
	// 第一次
	reg_value[0] = 0x40 | ((N>>3) & 0x1F);
	reg_value[1] = ((N & 0x07)<<5) | (A & 0x1F);
	reg_value[2] = 0xC0 | REF;
	reg_value[3] = ((BA & 0x07) << 5) | (PSC_DIV<<4) | (VCO_DIV<<1);
	ret = TP_iic_tuner_write(SHARP6306_DEV_ADDR, reg_value, 4);
	if(ret != TP_SUCCESS)
		return ret;

	// 第二次
	reg_value[0] = 0xC0 | (1<<2) | REF;	// TM = 1
	ret = TP_iic_tuner_write(SHARP6306_DEV_ADDR, reg_value, 1);
	if(ret != TP_SUCCESS)
		return ret;

	// 延时10ms
	TP_Delay(10);

	// 第三次
	reg_value[0] = 0xC0 | (1<<2) | ((PD & 0x01)<<4) | ((PD & 0x02)<<2) | REF;	// TM = 1 & PD
	reg_value[1] = ((BA & 0x07) << 5) | (PSC_DIV<<4) | (VCO_DIV<<1) | ((PD & 0x04)<<1) | ((PD & 0x08)>>1);
	ret = TP_iic_tuner_write(SHARP6306_DEV_ADDR, reg_value, 2);
	if(ret != TP_SUCCESS)
		return ret;

	return TP_SUCCESS;
}

#endif// _USE_TP5001_CHIP_
