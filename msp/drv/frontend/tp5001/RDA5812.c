/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "TP5001.h"
#include "RDA5812.h"
#ifdef _USE_TP5001_CHIP_


TP_UINT8 rda_5812_init()
{
	TP_UINT8 ret;
	TP_UINT8 reg_data[16];
	
// reset tuner
	reg_data[0] = 0x04;
	reg_data[1] = 0x04;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;
	reg_data[1] = 0x05;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	// 初始化Tuner寄存器
	reg_data[0] = 0x30;
	reg_data[1] = 0x60;
	reg_data[2] = 0x04;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 3);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x38;
	reg_data[1] = 0x03;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x3A;
	reg_data[1] = 0x06;
	reg_data[2] = 0x6B;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 3);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x44;
	reg_data[1] = 0x55;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x7C;
	reg_data[1] = 0xC4;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x39;
	reg_data[1] = 0xba;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x3e;
	reg_data[1] = 0x83;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x41;
	reg_data[1] = 0xa2;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x4f;
	reg_data[1] = 0x07;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x53;
	reg_data[1] = 0xac;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x67;
	reg_data[1] = 0x1c;
	reg_data[2] = 0x81;
	reg_data[3] = 0x47;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 4);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x6B;
	reg_data[1] = 0x18;
	reg_data[2] = 0xC6;
	reg_data[3] = 0x4B;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 4);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x71;
	reg_data[1] = 0x8e;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x05;
	reg_data[1] = 0x10;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x15;
	reg_data[1] = 0x10;
	reg_data[2] = 0x30;
	reg_data[3] = 0x34;
	reg_data[4] = 0x3C;
	reg_data[5] = 0x3e;
	reg_data[6] = 0x3f;
	reg_data[7] = 0x3f;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 8);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x06;
	reg_data[1] = 0x1b;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x1c;
	reg_data[1] = 0x5c;
	reg_data[2] = 0x50;
	reg_data[3] = 0x8f;
	reg_data[4] = 0x4e;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 5);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x20;
	reg_data[1] = 0x8d;
	reg_data[2] = 0x53;
	reg_data[3] = 0x90;
	reg_data[4] = 0x53;
	reg_data[5] = 0x8d;
	reg_data[6] = 0x49;
	reg_data[7] = 0x73;
	reg_data[8] = 0x55;
	reg_data[9] = 0x80;
	reg_data[10] = 0x7f;
	reg_data[11] = 0xb2;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 12);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x81;
	reg_data[1] = 0x96;
	reg_data[2] = 0xcf;
	reg_data[3] = 0xcc;
	reg_data[4] = 0xce;
	reg_data[5] = 0x9d;
	reg_data[6] = 0x88;
	reg_data[7] = 0xa8;
	reg_data[8] = 0x0e;
	reg_data[9] = 0x00;
	reg_data[10] = 0x0d;
	reg_data[11] = 0x0f;
	reg_data[12] = 0x1a;
	reg_data[13] = 0x28;
	reg_data[14] = 0x55;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 15);
	if(ret != TP_SUCCESS)
		return ret;

	reg_data[0] = 0x91;
	reg_data[1] = 0x1b; 
	reg_data[2] = 0xff;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 3);
	if(ret != TP_SUCCESS)
		return ret;

	// 延时1ms
	TP_Delay(1);	
	
	return TP_SUCCESS;
}

TP_UINT8 rda_5812_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{
	TP_UINT8 ret;
	TP_UINT8 reg_data[16];	
	
	// rxon = 0
	reg_data[0] = 0x04;
	reg_data[1] = 0x81;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	// 设置频点
	frequency *= 0x200000;
	frequency /= 27;
	reg_data[0] = 0x07;
	reg_data[1] = (TP_UINT8)(frequency>>24);
	reg_data[2] = (TP_UINT8)(frequency>>16);
	reg_data[3] = (TP_UINT8)(frequency>>8);
	reg_data[4] = (TP_UINT8)(frequency);
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 5);
	if(ret != TP_SUCCESS)
		return ret;

	// 设置带宽
	//Symbol_Rate_Value *= 1.25;
	Symbol_Rate_Value *= 5;
	Symbol_Rate_Value /= 4;

	Symbol_Rate_Value /= 2;
	Symbol_Rate_Value /= 1000000; 
	if(Symbol_Rate_Value < 4)
		Symbol_Rate_Value = 4;
	else if(Symbol_Rate_Value > 40)
		Symbol_Rate_Value = 40;
	reg_data[0] = 0x0B;
	reg_data[1] = Symbol_Rate_Value & 0xFF;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;

	// rxon = 1
	reg_data[0] = 0x04;
	reg_data[1] = 0x83;
	ret = TP_iic_tuner_write(RDA5812_DEV_ADDR, reg_data, 2);
	if(ret != TP_SUCCESS)
		return ret;
	TP_Delay(50);
	return TP_SUCCESS;	
}
#endif // _USE_TP5001_CHIP_
