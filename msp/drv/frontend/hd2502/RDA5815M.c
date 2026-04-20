/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

/*
 Filename:		RDA5815m_x.c
 Description:	RDA5815m - Digital Satellite Tuner IC driver.
version 1.0		The primary version,  created by hongxin wang  2012-8-28
version 1.1		Modify the register 0x38 from 0x93 to 0x 9B for increasing the amplitude of XOUT
version 1.2		Add configuration for Xtal = 30MHz. 2013-04-26
version 1.3		Add configuration for Xtal = 24MHz. 2013-06-09
version 1.4		AGC optimized to enhance max gain. 2013-06-27
version 1.4.1	Code simplified to avoid bugs for modes -- Xtal_24M and Xtal_30M. 2013-07-25
version 1.5		Optimize bandwidth calculation. 2013-8-19
version 1.6		Correct some functions' return value. Add APIs -- "RDA5815m_LockStatus", "RDA5815m_Sleep", "RDA5815m_Wakeup", "RDA5815m_RunningStatus". 2013-11-20
version 1.6.2	Add APIs to detect tuner's existing and get RSSI. 2014-05-26
version 1.6.3	Modify RSSI 2014-09-11
*/

#include "RDA5815M.h"

extern void HD_Delay(UINT32 uiMS);
extern UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
extern UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);

unsigned long gPLL = 0;

#define Xtal_27M
// #define Xtal_30M
// #define Xtal_24M

#define RDA_Tuner_Sleeping 0x02
#define RDA_Tuner_Waking 0x03


void RDA5815mSleep(UINT32 ms)
{
	HD_Delay(ms);
}

UINT8 RDA5815mWriteReg(UINT8 reg_addr, UINT8 reg_data)
{
	UINT8 buf[2];

	buf[0] = reg_addr;
	buf[1] = reg_data;

	return Write_I2C(HD2502_RDA5815M_DEV_ADDR, 2, buf);
}

UINT8 RDA5815mReadReg(UINT8 tuner_addr, UINT8 reg_addr, UINT8 *buffer)
{
	UINT8 ret = 0;

	ret = Write_I2C(tuner_addr, 1, &reg_addr);

	return Read_I2C(tuner_addr, 1, buffer);
}


void RDA5815mInitial(void)
{
	// RDA5815mWriteReg(register address,register data);

	RDA5815mSleep(1); // Wait 1ms.
					  //  Chip register soft reset
	RDA5815mWriteReg(0x04, 0x04);
	RDA5815mWriteReg(0x04, 0x05);

	// Initial configuration start

	// pll setting
	RDA5815mWriteReg(0x1a, 0x13);
	RDA5815mWriteReg(0x41, 0x53);
	RDA5815mWriteReg(0x38, 0x9B);
	RDA5815mWriteReg(0x39, 0x15);
	RDA5815mWriteReg(0x3A, 0x00);
	RDA5815mWriteReg(0x3B, 0x00);
	RDA5815mWriteReg(0x3C, 0x0c);
	RDA5815mWriteReg(0x0c, 0xE2);
	RDA5815mWriteReg(0x2e, 0x6F);

#ifdef Xtal_27M
	RDA5815mWriteReg(0x72, 0x07); // v1.1, 1538~1539
	RDA5815mWriteReg(0x73, 0x10);
	RDA5815mWriteReg(0x74, 0x71);
	RDA5815mWriteReg(0x75, 0x06); // v1.1, 1363~1364, 1862~1863
	RDA5815mWriteReg(0x76, 0x40);
	RDA5815mWriteReg(0x77, 0x89);
	RDA5815mWriteReg(0x79, 0x04); // v1.1, 900
	RDA5815mWriteReg(0x7A, 0x2A);
	RDA5815mWriteReg(0x7B, 0xAA);
	RDA5815mWriteReg(0x7C, 0xAB);
#endif
#ifdef Xtal_30M
	RDA5815mWriteReg(0x72, 0x06); // v1.2, 1544~1545
	RDA5815mWriteReg(0x73, 0x60);
	RDA5815mWriteReg(0x74, 0x66);
	RDA5815mWriteReg(0x75, 0x05); // v1.2, 1364~1365, 1859~1860
	RDA5815mWriteReg(0x76, 0xA0);
	RDA5815mWriteReg(0x77, 0x7B);
	RDA5815mWriteReg(0x79, 0x03); // v1.2, 901
	RDA5815mWriteReg(0x7A, 0xC0);
	RDA5815mWriteReg(0x7B, 0x00);
	RDA5815mWriteReg(0x7C, 0x00);
#endif
#ifdef Xtal_24M
	RDA5815mWriteReg(0x72, 0x08); // v1.3, 1547~1548
	RDA5815mWriteReg(0x73, 0x00);
	RDA5815mWriteReg(0x74, 0x80);
	RDA5815mWriteReg(0x75, 0x07); // v1.3, 1367~1368, 1859~1860
	RDA5815mWriteReg(0x76, 0x10);
	RDA5815mWriteReg(0x77, 0x9A);
	RDA5815mWriteReg(0x79, 0x04); // v1.3, 901
	RDA5815mWriteReg(0x7A, 0xB0);
	RDA5815mWriteReg(0x7B, 0x00);
	RDA5815mWriteReg(0x7C, 0x00);
#endif

	RDA5815mWriteReg(0x2f, 0x57);
	RDA5815mWriteReg(0x0d, 0x70);
	RDA5815mWriteReg(0x18, 0x4B);
	RDA5815mWriteReg(0x30, 0xFF);
	RDA5815mWriteReg(0x5c, 0xFF);
	RDA5815mWriteReg(0x65, 0x00);
	RDA5815mWriteReg(0x70, 0x3F);
	RDA5815mWriteReg(0x71, 0x3F);
	RDA5815mWriteReg(0x53, 0xA8);
	RDA5815mWriteReg(0x46, 0x21);
	RDA5815mWriteReg(0x47, 0x84);
	RDA5815mWriteReg(0x48, 0x10);
	RDA5815mWriteReg(0x49, 0x08);
	RDA5815mWriteReg(0x60, 0x80);
	RDA5815mWriteReg(0x61, 0x80);
	RDA5815mWriteReg(0x6A, 0x08);
	RDA5815mWriteReg(0x6B, 0x63);
	RDA5815mWriteReg(0x69, 0xF8);
	RDA5815mWriteReg(0x57, 0x64);
	RDA5815mWriteReg(0x05, 0xaa);
	RDA5815mWriteReg(0x06, 0xaa);
	RDA5815mWriteReg(0x15, 0xAE);
	RDA5815mWriteReg(0x4a, 0x67);
	RDA5815mWriteReg(0x4b, 0x77);

	// agc setting

	RDA5815mWriteReg(0x4f, 0x40);
	RDA5815mWriteReg(0x5b, 0x20);

	RDA5815mWriteReg(0x16, 0x0C);
	RDA5815mWriteReg(0x18, 0x0C);
	RDA5815mWriteReg(0x30, 0x1C);
	RDA5815mWriteReg(0x5c, 0x2C);
	RDA5815mWriteReg(0x6c, 0x3C);
	RDA5815mWriteReg(0x6e, 0x3C);
	RDA5815mWriteReg(0x1b, 0x7C);
	RDA5815mWriteReg(0x1d, 0xBD);
	RDA5815mWriteReg(0x1f, 0xBD);
	RDA5815mWriteReg(0x21, 0xBE);
	RDA5815mWriteReg(0x23, 0xBE);
	RDA5815mWriteReg(0x25, 0xFE);
	RDA5815mWriteReg(0x27, 0xFF);
	RDA5815mWriteReg(0x29, 0xFF);
	RDA5815mWriteReg(0xb3, 0xFF);
	RDA5815mWriteReg(0xb5, 0xFF);

	RDA5815mWriteReg(0x17, 0xF0);
	RDA5815mWriteReg(0x19, 0xF0);
	RDA5815mWriteReg(0x31, 0xF0);
	RDA5815mWriteReg(0x5d, 0xF0);
	RDA5815mWriteReg(0x6d, 0xF0);
	RDA5815mWriteReg(0x6f, 0xF1);
	RDA5815mWriteReg(0x1c, 0xF5);
	RDA5815mWriteReg(0x1e, 0x35);
	RDA5815mWriteReg(0x20, 0x79);
	RDA5815mWriteReg(0x22, 0x9D);
	RDA5815mWriteReg(0x24, 0xBE);
	RDA5815mWriteReg(0x26, 0xBE);
	RDA5815mWriteReg(0x28, 0xBE);
	RDA5815mWriteReg(0x2a, 0xCF);
	RDA5815mWriteReg(0xb4, 0xDF);
	RDA5815mWriteReg(0xb6, 0x0F);

	RDA5815mWriteReg(0xb7, 0x15); // start
	RDA5815mWriteReg(0xb9, 0x6c);
	RDA5815mWriteReg(0xbb, 0x63);
	RDA5815mWriteReg(0xbd, 0x5a);
	RDA5815mWriteReg(0xbf, 0x5a);
	RDA5815mWriteReg(0xc1, 0x55);
	RDA5815mWriteReg(0xc3, 0x55);
	RDA5815mWriteReg(0xc5, 0x47);
	RDA5815mWriteReg(0xa3, 0x53);
	RDA5815mWriteReg(0xa5, 0x4f);
	RDA5815mWriteReg(0xa7, 0x4e);
	RDA5815mWriteReg(0xa9, 0x4e);
	RDA5815mWriteReg(0xab, 0x54);
	RDA5815mWriteReg(0xad, 0x31);
	RDA5815mWriteReg(0xaf, 0x43);
	RDA5815mWriteReg(0xb1, 0x9f);

	RDA5815mWriteReg(0xb8, 0x6c); // end
	RDA5815mWriteReg(0xba, 0x92);
	RDA5815mWriteReg(0xbc, 0x8a);
	RDA5815mWriteReg(0xbe, 0x8a);
	RDA5815mWriteReg(0xc0, 0x82);
	RDA5815mWriteReg(0xc2, 0x93);
	RDA5815mWriteReg(0xc4, 0x85);
	RDA5815mWriteReg(0xc6, 0x77);
	RDA5815mWriteReg(0xa4, 0x82);
	RDA5815mWriteReg(0xa6, 0x7e);
	RDA5815mWriteReg(0xa8, 0x7d);
	RDA5815mWriteReg(0xaa, 0x6f);
	RDA5815mWriteReg(0xac, 0x65);
	RDA5815mWriteReg(0xae, 0x43);
	RDA5815mWriteReg(0xb0, 0x9f);
	RDA5815mWriteReg(0xb2, 0xf0);

	RDA5815mWriteReg(0x81, 0x92); // rise
	RDA5815mWriteReg(0x82, 0xb4);
	RDA5815mWriteReg(0x83, 0xb3);
	RDA5815mWriteReg(0x84, 0xac);
	RDA5815mWriteReg(0x85, 0xba);
	RDA5815mWriteReg(0x86, 0xbc);
	RDA5815mWriteReg(0x87, 0xaf);
	RDA5815mWriteReg(0x88, 0xa2);
	RDA5815mWriteReg(0x89, 0xac);
	RDA5815mWriteReg(0x8a, 0xa9);
	RDA5815mWriteReg(0x8b, 0x9b);
	RDA5815mWriteReg(0x8c, 0x7d);
	RDA5815mWriteReg(0x8d, 0x74);
	RDA5815mWriteReg(0x8e, 0x9f);
	RDA5815mWriteReg(0x8f, 0xf0);

	RDA5815mWriteReg(0x90, 0x15); // fall
	RDA5815mWriteReg(0x91, 0x39);
	RDA5815mWriteReg(0x92, 0x30);
	RDA5815mWriteReg(0x93, 0x27);
	RDA5815mWriteReg(0x94, 0x29);
	RDA5815mWriteReg(0x95, 0x0d);
	RDA5815mWriteReg(0x96, 0x10);
	RDA5815mWriteReg(0x97, 0x1e);
	RDA5815mWriteReg(0x98, 0x1a);
	RDA5815mWriteReg(0x99, 0x19);
	RDA5815mWriteReg(0x9a, 0x19);
	RDA5815mWriteReg(0x9b, 0x32);
	RDA5815mWriteReg(0x9c, 0x1f);
	RDA5815mWriteReg(0x9d, 0x31);
	RDA5815mWriteReg(0x9e, 0x43);

	RDA5815mSleep(10); // Wait 10ms;

	// Initial configuration end
}

/********************************************************************************/
//	Function to Set the RDA5815m
//	fPLL:		Frequency			unit: MHz from 950 to 2150
//	fSym:	SymbolRate			unit: KSps from 1000 to 60000
/********************************************************************************/
unsigned char RDA5815mSet(unsigned long fPLL, unsigned long fSym)
{
	unsigned char buffer;//, buff;
	unsigned long temp_value = 0;
	unsigned long bw; /*,temp_value1 = 0,temp_value2=0 ;*/
	unsigned char Filter_bw_control_bit;

	gPLL = fPLL;

	RDA5815mWriteReg(0x04, 0xc1); // add by rda 2011.8.9,RXON = 0 , change normal working state to idle state
	RDA5815mWriteReg(0x2b, 0x95); // clk_interface_27m=0  add by rda 2012.1.12

	// set frequency start
#ifdef Xtal_27M								  // v1.1
	temp_value = (unsigned long)fPLL * 77672; //((2^21) / RDA5815m_XTALFREQ);
#endif
#ifdef Xtal_30M								  // v1.2
	temp_value = (unsigned long)fPLL * 69905; //((2^21) / RDA5815m_XTALFREQ);
#endif
#ifdef Xtal_24M								  // v1.3
	temp_value = (unsigned long)fPLL * 87381; //((2^21) / RDA5815m_XTALFREQ);
#endif

	buffer = ((unsigned char)((temp_value >> 24) & 0xff));
	RDA5815mWriteReg(0x07, buffer);
	buffer = ((unsigned char)((temp_value >> 16) & 0xff));
	RDA5815mWriteReg(0x08, buffer);
	buffer = ((unsigned char)((temp_value >> 8) & 0xff));
	RDA5815mWriteReg(0x09, buffer);
	buffer = ((unsigned char)(temp_value & 0xff));
	RDA5815mWriteReg(0x0a, buffer);
	// set frequency end

	// set Filter bandwidth start
	bw = fSym; // kHz

	Filter_bw_control_bit = (unsigned char)((bw * 135 / 200 + 4000) / 1000);

	if (Filter_bw_control_bit < 4)
		Filter_bw_control_bit = 4; // MHz
	else if (Filter_bw_control_bit > 40)
		Filter_bw_control_bit = 40; // MHz

	Filter_bw_control_bit &= 0x3f;
	Filter_bw_control_bit |= 0x40; // v1.5

	RDA5815mWriteReg(0x0b, Filter_bw_control_bit);
	// set Filter bandwidth end

	RDA5815mWriteReg(0x04, 0xc3); // add by rda 2011.8.9,RXON = 0 ,rxon=1,normal working
	RDA5815mWriteReg(0x2b, 0x97); // clk_interface_27m=1  add by rda 2012.1.12
	RDA5815mSleep(5);			  // Wait 5ms;

	return 0;
}

// to get the lock status of tuner VCO & PLL
unsigned char RDA5815m_LockStatus(unsigned char tuner_addr)
{
	unsigned char buffer, i;

	for (i = 0; i < 100; i++) // Loop times: 100 is recommanded; at least 30.
	{
		RDA5815mReadReg(tuner_addr, 0x03, &buffer);
		if ((buffer & 0x03) != 0x03)
		{
			return 1;
		}
	}

	return 0;
}

// to set RDA tuner to sleep status (low power consumption)
unsigned char RDA5815m_Sleep(unsigned char tuner_addr)
{
	unsigned char buffer;

	RDA5815mReadReg(tuner_addr, 0x04, &buffer);
	if ((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		RDA5815mWriteReg(0x04, buffer & 0x7F);
	}

	return 0;
}

// to wake up RDA tuner from sleep status
unsigned char RDA5815m_Wakeup(unsigned char tuner_addr)
{
	unsigned char buffer;

	RDA5815mReadReg(tuner_addr, 0x04, &buffer);
	if ((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		RDA5815mWriteReg(0x04, buffer | 0x80);
	}

	return 0;
}

// to get the running status of RDA tuner -- "Sleeping" or "Waking".
unsigned char RDA5815m_RunningStatus(unsigned char tuner_addr)
{
	unsigned char buffer;

	RDA5815mReadReg(tuner_addr, 0x04, &buffer);
	if ((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		if (buffer & 0x80)
		{
			return RDA_Tuner_Waking;
		}
		else
		{
			return RDA_Tuner_Sleeping;
		}
	}

	return 0;
}

unsigned char RDA5815m_CheckExist(unsigned char tuner_addr) // To detect tuner's part number by reading chip ID registers
{
	unsigned char buffer;

	RDA5815mReadReg(tuner_addr, 0x00, &buffer);
	if (buffer == 0x58)
	{
		RDA5815mReadReg(tuner_addr, 0x01, &buffer);
		if (buffer == 0xf8)
		{
			return 0; // RDA5815m
		}
		else
		{
			return 2; // RDA chip, not RDA5815m
		}
	}
	else
	{
		return 1; // not RDA chip, or IIC error
	}

	return 0;
}

/********************************************************************************/
//	Function to Get RSSI
//	fPLL:	Frequency		unit: MHz from 950 to 2150
//	rssi:	RSSI			range: 0 ~ 255.
/********************************************************************************/
int L1_RF_RDA5815m_RSSI(unsigned char tuner_addr)
{
	unsigned char buffer;
	unsigned long fPLL = 0;

	/* Gain stage limits                 st1  pre   st2 post i2v fil  */
#define RDA5815m_Gain_Stage__0 0xc00 /* '11   00	00	00	 00	 00	' */
#define RDA5815m_Gain_Stage__1 0xc00 /* '11   00	00	00	 00	 00 ' */
#define RDA5815m_Gain_Stage__2 0xc01 /* '11   00	00	00	 00	 01 ' */
#define RDA5815m_Gain_Stage__3 0xc02 /* '11   00	00	00	 00	 10 ' */
#define RDA5815m_Gain_Stage__4 0xc03 /* '11   00	00	00	 00	 11 ' */
#define RDA5815m_Gain_Stage__5 0xc13 /* '11   00	00	01	 00	 11 ' */
#define RDA5815m_Gain_Stage__6 0xd17 /* '11   01	00	01	 01	 11 ' */
#define RDA5815m_Gain_Stage__7 0xd5b /* '11   01	01	01	 10	 11 ' */
#define RDA5815m_Gain_Stage__8 0xe5b /* '11   10	01	01	 10	 11 ' */
#define RDA5815m_Gain_Stage__9 0xf9b /* '11   11	10	01	 10	 11 ' */
#define RDA5815m_Gain_Stage_10 0xfab /* '11   11	10	10	 10	 11 ' */
#define RDA5815m_Gain_Stage_11 0xfaf /* '11   11	10	10	 11	 11 ' */
#define RDA5815m_Gain_Stage_12 0xfef /* '11   11	11	10	 11	 11 ' */
#define RDA5815m_Gain_Stage_13 0xfff /* '11   11	11	11	 11	 11 ' */
#define RDA5815m_Gain_Stage_14 0xfff /* '11   11	11	11	 11	 11 ' */
#define RDA5815m_Gain_Stage_15 0xfff /* '11   11	11	11	 11	 11 ' */

	unsigned char data16, data17, st1, pre, st2, post, i2v, filter, vga;
	unsigned int stage_code;
	unsigned char gain_stage;
	unsigned char band;
#if 0
	double vga_gain, total_gain;

	/* gain band/gain_stage:    0       1      2     3     4      5      6     7     8     9     10    11    12    13    14   15 */
	double gain[13][16] = 
	{
		{-7.6, -7.7, -1.7, 4.3, 10.2, 15.8, 25.4, 34.2, 40.7, 49.4, 55.2, 61.4, 65.5, 71.6, 71.6, 71.6}, /*  1   950MHz<=Freq<1000MHz  */
		{-7.2, -7.2, -1.2, 4.8, 10.8, 16.4, 25.9, 34.2, 40.8, 49.3, 55.3, 61.4, 65.5, 71.8, 71.8, 71.8}, /*  2  1000MHz<=Freq<1100MHz  */
		{-6.5, -6.6, -0.5, 5.5, 11.5, 17.1, 26.5, 34.8, 41.1, 49.2, 55.1, 61.1, 65.4, 71.6, 71.6, 71.6}, /*  3  1100MHz<=Freq<1200MHz  */
		{-5.9, -5.8, 0, 6.1, 12.1, 17.7, 27.0, 35.3, 41.4, 49.0, 54.9, 61.0, 65.1, 71.5, 71.5, 71.5},	 /*  4  1200MHz<=Freq<1300MHz  */
		{-5.8, -5.8, 0.2, 6.2, 12.2, 17.9, 27.1, 35.5, 40.9, 48.4, 54.3, 60.4, 64.5, 71.0, 71.0, 71.0},	 /*  5  1300MHz<=Freq<1400MHz  */
		{-5.7, -5.6, 0.4, 6.4, 12.4, 18.1, 27.2, 35.3, 41.3, 48.0, 53.9, 60.1, 64.1, 70.7, 70.7, 70.7},	 /*  6  1400MHz<=Freq<1500MHz  */
		{-5.0, -5.0, 1.0, 7.1, 13.0, 18.7, 27.8, 36.0, 41.2, 48.0, 54.1, 60.2, 64.2, 70.8, 70.8, 70.8},	 /*  7  1500MHz<=Freq<1600MHz  */
		{-4.2, -4.1, 1.7, 7.7, 13.8, 19.4, 28.4, 36.8, 41.7, 47.9, 54.2, 60.2, 64.1, 70.9, 70.9, 70.9},	 /*  8  1600MHz<=Freq<1700MHz  */
		{-4.2, -4.1, 1.8, 7.8, 13.8, 19.5, 28.3, 36.4, 41.3, 47.4, 53.6, 59.8, 63.4, 70.3, 70.3, 70.3},	 /*  9  1700MHz<=Freq<1800MHz  */
		{-4.4, -4.5, 1.5, 7.5, 13.5, 19.2, 27.9, 36.5, 40.6, 46.6, 53.0, 59.1, 62.5, 69.6, 69.6, 69.6},	 /* 10  1800MHz<=Freq<1900MHz  */
		{-4.3, -4.4, 1.7, 7.6, 13.7, 19.4, 28.0, 35.6, 40.3, 46.4, 52.7, 58.9, 62.2, 69.2, 69.2, 69.2},	 /* 11  1900MHz<=Freq<2000MHz  */
		{-4.2, -4.2, 1.9, 7.8, 13.8, 19.6, 28.0, 36.2, 40.4, 46.0, 52.6, 58.6, 61.7, 68.7, 68.7, 68.7},	 /* 12  2000MHz<=Freq<2100MHz  */
		{-4.6, -4.7, 1.4, 7.4, 13.4, 19.2, 27.5, 35.6, 39.5, 45.2, 51.7, 57.8, 60.6, 67.7, 67.6, 67.7}	 /* 13  2100MHz<=Freq<=2150MHz */
	};
#else
	unsigned long vga_gain, total_gain;

	/* gain band/gain_stage:    0       1      2     3     4      5      6     7     8     9     10    11    12    13    14   15 */
	unsigned long gain[13][16] = 
	{
		{-76, -77, -17, 43, 102, 158, 254, 342, 407, 494, 552, 614, 655, 716, 716, 716}, /*  1   950MHz<=Freq<1000MHz  */
		{-72, -72, -12, 48, 108, 164, 259, 342, 408, 493, 553, 614, 655, 718, 718, 718}, /*  2  1000MHz<=Freq<1100MHz  */
		{-65, -66, -05, 55, 115, 171, 265, 348, 411, 492, 551, 611, 654, 716, 716, 716}, /*  3  1100MHz<=Freq<1200MHz  */
		{-59, -58,   0, 61, 121, 177, 270, 353, 414, 490, 549, 610, 651, 715, 715, 715}, /*  4  1200MHz<=Freq<1300MHz  */
		{-58, -58,  02, 62, 122, 179, 271, 355, 409, 484, 543, 604, 645, 710, 710, 710}, /*  5  1300MHz<=Freq<1400MHz  */
		{-57, -56,  04, 64, 124, 181, 272, 353, 413, 480, 539, 601, 641, 707, 707, 707}, /*  6  1400MHz<=Freq<1500MHz  */
		{-50, -50,  10, 71, 130, 187, 278, 360, 412, 480, 541, 602, 642, 708, 708, 708}, /*  7  1500MHz<=Freq<1600MHz  */
		{-42, -41,  17, 77, 138, 194, 284, 368, 417, 479, 542, 602, 641, 709, 709, 709}, /*  8  1600MHz<=Freq<1700MHz  */
		{-42, -41,  18, 78, 138, 195, 283, 364, 413, 474, 536, 598, 634, 703, 703, 703}, /*  9  1700MHz<=Freq<1800MHz  */
		{-44, -45,  15, 75, 135, 192, 279, 365, 406, 466, 530, 591, 625, 696, 696, 696}, /* 10  1800MHz<=Freq<1900MHz  */
		{-43, -44,  17, 76, 137, 194, 280, 356, 403, 464, 527, 589, 622, 692, 692, 692}, /* 11  1900MHz<=Freq<2000MHz  */
		{-42, -42,  19, 78, 138, 196, 280, 362, 404, 460, 526, 586, 617, 687, 687, 687}, /* 12  2000MHz<=Freq<2100MHz  */
		{-46, -47,  14, 74, 134, 192, 275, 356, 395, 452, 517, 578, 606, 677, 676, 677}	 /* 13  2100MHz<=Freq<=2150MHz */
	};
#endif

	RDA5815mReadReg(tuner_addr, 0x16, &data16);
	i2v = (data16 & 0xc0) >> 6;
	filter = (data16 & 0x30) >> 4;
	st1 = (data16 & 0x0c) >> 2;
	st2 = (data16 & 0x03) >> 0;

	RDA5815mReadReg(tuner_addr, 0x17, &data17);
	pre = (data17 & 0x0c) >> 2;
	post = (data17 & 0x03) >> 0;

	stage_code = (st1 << 10) + (pre << 8) + (st2 << 6) + (post << 4) + (i2v << 2) + (filter << 0);

	if (stage_code == RDA5815m_Gain_Stage_13)
	{
		gain_stage = 13;
	}
	else if (stage_code == RDA5815m_Gain_Stage_12)
	{
		gain_stage = 12;
	}
	else if (stage_code == RDA5815m_Gain_Stage_11)
	{
		gain_stage = 11;
	}
	else if (stage_code == RDA5815m_Gain_Stage_10)
	{
		gain_stage = 10;
	}
	else if (stage_code == RDA5815m_Gain_Stage__9)
	{
		gain_stage = 9;
	}
	else if (stage_code == RDA5815m_Gain_Stage__8)
	{
		gain_stage = 8;
	}
	else if (stage_code == RDA5815m_Gain_Stage__7)
	{
		gain_stage = 7;
	}
	else if (stage_code == RDA5815m_Gain_Stage__6)
	{
		gain_stage = 6;
	}
	else if (stage_code == RDA5815m_Gain_Stage__5)
	{
		gain_stage = 5;
	}
	else if (stage_code == RDA5815m_Gain_Stage__4)
	{
		gain_stage = 4;
	}
	else if (stage_code == RDA5815m_Gain_Stage__3)
	{
		gain_stage = 3;
	}
	else if (stage_code == RDA5815m_Gain_Stage__2)
	{
		gain_stage = 2;
	}
	else if (stage_code == RDA5815m_Gain_Stage__1)
	{
		gain_stage = 1;
	}
	else
	{
		gain_stage = 0;
	}

	fPLL = gPLL;

	if (fPLL < 950)
		return 1;
	else if (fPLL < 1000)
	{
		band = 0;
	}
	else if (fPLL < 1100)
	{
		band = 1;
	}
	else if (fPLL < 1200)
	{
		band = 2;
	}
	else if (fPLL < 1300)
	{
		band = 3;
	}
	else if (fPLL < 1400)
	{
		band = 4;
	}
	else if (fPLL < 1500)
	{
		band = 5;
	}
	else if (fPLL < 1600)
	{
		band = 6;
	}
	else if (fPLL < 1700)
	{
		band = 7;
	}
	else if (fPLL < 1800)
	{
		band = 8;
	}
	else if (fPLL < 1900)
	{
		band = 9;
	}
	else if (fPLL < 2000)
	{
		band = 10;
	}
	else if (fPLL < 2100)
	{
		band = 11;
	}
	else if (fPLL <= 2150)
	{
		band = 12;
	}
	else
		return 1;

	RDA5815mReadReg(tuner_addr, 0xb7, &buffer);
	vga = buffer;

#if 0
	vga_gain = vga * 30.0 / 255;

	total_gain = gain[band][gain_stage] + vga_gain;

	if (0)
	{
		printk("RDA5815m RSSI = gain[%d][%d] + vga_gain = %f + %f = %f\n", band, gain_stage, gain[band][gain_stage], vga_gain, total_gain);
	}

	return -total_gain;
#else
	vga_gain = (unsigned long )(vga * 300 / 255);

	total_gain = gain[band][gain_stage] + vga_gain;

	if (0)
	{
		printk("RDA5815m RSSI = gain[%d][%d] + vga_gain = %ld + %ld = %ld\n", band, gain_stage, gain[band][gain_stage], vga_gain, total_gain);
	}

	return -total_gain / 10;
#endif
}

