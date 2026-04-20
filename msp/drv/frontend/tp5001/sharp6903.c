/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "TP5001.h"
#include "sharp6903.h"
#include "QM1D1C0045if.h"
#ifdef _USE_TP5001_CHIP_

TP_UINT8 sharp_6903_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{
	//TP_UINT8 ret = 0;
	TP_UINT8 BW[31] = {0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0,11,0,12,0,13,0,14,0,15};

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
	if(Symbol_Rate_Value < 4)
		Symbol_Rate_Value = 4;
	else if(Symbol_Rate_Value > 34)
		Symbol_Rate_Value = 34;

	Symbol_Rate_Value = BW[Symbol_Rate_Value-4];

	QM1D1C0045_init(frequency*1000,Symbol_Rate_Value);

	return TP_SUCCESS;
}
#endif // _USE_TP5001_CHIP_
