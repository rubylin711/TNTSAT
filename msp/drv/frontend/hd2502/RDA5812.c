/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

/*
 * Filename:      RDA5812Tuner.c
 *
 * Description:   RDA5812 Digital Satellite Tuner IC driver.
 */

/****************************************************************************/

/* Function to Initialize the RDA5812 */
/***************************************************************************/
//#include "stdafx.h"

#include "HDICType.h"
#include "HDIC2501.h"
#include "RDA5812.h"

extern void HD_Delay(UINT32 uiMS);
extern UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
extern UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);


void RDA5812WriteReg(UINT8 Register, UINT8 Value)
{
	UINT8 Temp[2];
	Temp[0] = Register;
	Temp[1] = Value;
	Write_I2C(RDA5812_I2C_ADDRESS, 2, Temp);
}

void RDA5812Initial(void)
{
	HD_Delay(1); // Wait 1ms.

	// Chip register soft reset
	RDA5812WriteReg(0x04, 0x04); // Mov 8'b0000_0100, 04H;
	RDA5812WriteReg(0x04, 0x05); // Mov 8'b0000_0101, 04H;

	// Initial configuration start
	RDA5812WriteReg(0x30, 0x60); // Mov 8'b0110_0000, 30H;
	RDA5812WriteReg(0x31, 0x04); // Mov 8'b0000_0100, 31H;
	RDA5812WriteReg(0x38, 0x03); // Mov 8'b0000_0011, 38H;
	RDA5812WriteReg(0x3A, 0x06); // Mov 8'b0000_0110, 3AH;
	RDA5812WriteReg(0x3B, 0x6b); // Mov 8'b0110_1011, 3BH;
	RDA5812WriteReg(0x44, 0x55); // Mov 8'b0101_0101, 44H;
	RDA5812WriteReg(0x7C, 0xc4); // Mov 8'b1100_0100, 7CH;
	RDA5812WriteReg(0x39, 0xba); // Mov 8'b1011_1010, 39H;
	RDA5812WriteReg(0x3E, 0x83); // Mov 8'b1000_0011, 3EH;
	RDA5812WriteReg(0x41, 0xa2); // Mov 8'b1010_0010, 41H;
	RDA5812WriteReg(0x4F, 0x07); // Mov 8'b0000_0111, 4FH;
	RDA5812WriteReg(0x53, 0xac); // Mov 8'b1010_1100, 53H;
	RDA5812WriteReg(0x67, 0x1c); // Mov 8'b0001_1100, 67H;
	RDA5812WriteReg(0x68, 0x81); // Mov 8'b1000_0001, 68H;
	RDA5812WriteReg(0x69, 0x47); // Mov 8'b0100_0111, 69H;
	RDA5812WriteReg(0x6B, 0x18); // Mov 8'b0001_1000, 6BH;
	RDA5812WriteReg(0x6C, 0xc6); // Mov 8'b1100_0110, 6CH;
	RDA5812WriteReg(0x6D, 0x4b); // Mov 8'b0100_1011, 6DH;
	RDA5812WriteReg(0x71, 0x8e); // Mov 8'b1000_1110, 71H;
	RDA5812WriteReg(0x05, 0x10); // Mov 8'b0001_0000, 05H;
	RDA5812WriteReg(0x15, 0x10); // Mov 8'b0001_0000, 15H;
	RDA5812WriteReg(0x16, 0x30); // Mov 8'b0011_0000, 16H;
	RDA5812WriteReg(0x17, 0x34); // Mov 8'b0011_0100, 17H;
	RDA5812WriteReg(0x18, 0x3c); // Mov 8'b0011_1100, 18H;
	RDA5812WriteReg(0x19, 0x3e); // Mov 8'b0011_1110, 19H;
	RDA5812WriteReg(0x1a, 0x3f); // Mov 8'b0011_1111, 1aH;
	RDA5812WriteReg(0x1b, 0x3f); // Mov 8'b0011_1111, 1bH;
	RDA5812WriteReg(0x06, 0x1b); // Mov 8'b0001_1011, 06H;
	RDA5812WriteReg(0x1c, 0x5c); // Mov 8'b0101_1100, 1cH;
	RDA5812WriteReg(0x1d, 0x50); // Mov 8'b0101_0000, 1dH;
	RDA5812WriteReg(0x1e, 0x8f); // Mov 8'b1000_1111, 1eH;
	RDA5812WriteReg(0x1f, 0x4e); // Mov 8'b0100_1110, 1fH;
	RDA5812WriteReg(0x20, 0x8d); // Mov 8'b1000_1101, 20H;
	RDA5812WriteReg(0x21, 0x53); // Mov 8'b0101_0011, 21H;
	RDA5812WriteReg(0x22, 0x90); // Mov 8'b1001_0000, 22H;
	RDA5812WriteReg(0x23, 0x53); // Mov 8'b0101_0011, 23H;
	RDA5812WriteReg(0x24, 0x8d); // Mov 8'b1000_1101, 24H;
	RDA5812WriteReg(0x25, 0x49); // Mov 8'b0100_1001, 25H;
	RDA5812WriteReg(0x26, 0x73); // Mov 8'b1110_0011, 26H;
	RDA5812WriteReg(0x27, 0x55); // Mov 8'b0101_1101, 27H;
	RDA5812WriteReg(0x28, 0x80); // Mov 8'b1000_0000, 28H;
	RDA5812WriteReg(0x29, 0x7f); // Mov 8'b0111_1111, 29H;
	RDA5812WriteReg(0x2a, 0xb2); // Mov 8'b1011_0010, 2aH;
	RDA5812WriteReg(0x81, 0x96); // Mov 8'b1001_0110, 81H;
	RDA5812WriteReg(0x82, 0xcf); // Mov 8'b1100_1111, 82H;
	RDA5812WriteReg(0x83, 0xcc); // Mov 8'b1100_1100, 83H;
	RDA5812WriteReg(0x84, 0xce); // Mov 8'b1100_1110, 84H;
	RDA5812WriteReg(0x85, 0x9d); // Mov 8'b1001_1101, 85H;
	RDA5812WriteReg(0x86, 0x88); // Mov 8'b1000_1000, 86H;
	RDA5812WriteReg(0x87, 0xa8); // Mov 8'b1010_1000, 87H;
	RDA5812WriteReg(0x88, 0x0e); // Mov 8'b0000_1110, 88H;
	RDA5812WriteReg(0x89, 0x00); // Mov 8'b0000_0000, 89H;
	RDA5812WriteReg(0x8a, 0x0d); // Mov 8'b0000_1101, 8aH;
	RDA5812WriteReg(0x8b, 0x0f); // Mov 8'b0000_1111, 8bH;
	RDA5812WriteReg(0x8c, 0x1a); // Mov 8'b0001_1010, 8cH;
	RDA5812WriteReg(0x8d, 0x28); // Mov 8'b0010_1000, 8dH;
	RDA5812WriteReg(0x8e, 0x55); // Mov 8'b1010_1010, 8eH;
	RDA5812WriteReg(0x91, 0x1b); // Mov 8'b0001_1011, 91H;
	RDA5812WriteReg(0x92, 0xff); // Mov 8'b1111_1111, 92H;

	HD_Delay(10); // Wait 10ms;

	// Normal working start
}

/*************************************************************************/
/*	Function to Set the RDA5812 */

/*	fPLL:   Frequency        			unit: MHz  from 950 to 2150 */
/*	fSym:   SymbolRate       			unit: KS/s from 1000 to 45000 */
/*  gainHold:  The flag of AGC gain hold, the tuner gain is hold when gainHold == 1 , default please set gainHold = 0   */
/*  return: Frequency offset of PLL  	unit: KHz */
/************************************************************************/

INT32 RDA5812Set(UINT32 fPLL, UINT32 fSym, UINT8 gainHold)
{
	UINT8 buffer;
	UINT32 temp_value = 0;
	UINT32 bw; /*,temp_value1 = 0,temp_value2 = 0;*/
	UINT8 Filter_bw_control_bit;

	// 频率设置
	// RXON = 0, change normal working state to idle state
	RDA5812WriteReg(0x04, 0x81);
	// temp_value2 = fSym;
	// LOG_PRINTF("temp_valuel = %d\ttemp_value2 = %d\n", temp_value1, temp_value2);

	temp_value = (UINT32)(fPLL * 77672); //((2 << 21) / RDA5810_XTALFREQ);

	buffer = ((UINT8)((temp_value >> 24) & 0xff));
	RDA5812WriteReg(0x07, buffer);
	// LOG_PRINTF("\t[0x07] -- write: 0x%x\n", buffer);
	// RDA5812ReadReg(0x07); //while(1){}
	buffer = ((UINT8)((temp_value >> 16) & 0xff));
	RDA5812WriteReg(0x08, buffer);
	buffer = ((UINT8)((temp_value >> 8) & 0xff));
	RDA5812WriteReg(0x09, buffer);
	buffer = ((UINT8)(temp_value & 0xff));
	RDA5812WriteReg(0x0a, buffer);

	// Filter bandwidth setting
	// 限制带宽范围
	bw = fSym;
	// Printf("bw = %d\n", bw);

	if (bw < 4000)
		bw = 4000; // KHz
	else if (bw > 45000)
		bw = 45000; // KHz

	Filter_bw_control_bit = (UINT8)((bw * 135 / 200 + 4000) / 1000);

	Filter_bw_control_bit &= 0x3f;
	RDA5812WriteReg(0x0b, Filter_bw_control_bit);

	RDA5812WriteReg(0x04, 0xa3);

	HD_Delay(50); // Wait 50ms;
	// LOG_PRINTF("set 5812 ok!\n");

	return 1;
}


