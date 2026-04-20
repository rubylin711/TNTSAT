/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2018 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/

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

#include <linux/printk.h>

#include "mt_fe_def.h"

#include "mt_fe_i2c_ct8k.h"


#if MT_FE_DMD_DVBS_S2_SUPPORT

typedef long int INT32;
typedef unsigned long UINT32;

typedef unsigned char AVL_uchar;
typedef unsigned long AVL_uint32;


#define Xtal_27M
//#define Xtal_30M
//#define Xtal_24M

#define		RDA_Tuner_Sleeping		0x02
#define		RDA_Tuner_Waking		0x03


static void RDA5815Sleep(U32 delay_ms)
{
    _mt_sleep_ct8k(delay_ms);
}

static void RDA5815WriteReg(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	_mt_fe_tn_set_reg_ss2_ct8k(handle, reg_addr, reg_data);
	//printk("RDA5815 Set Reg [0x%02x]---- 0x%02x = 0x%02x\n", handle->m_device_ss2.tuner_cfg.tuner_dev_addr, reg_addr, reg_data);
}

static void RDA5815ReadReg(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8* reg_data)
{
	_mt_fe_tn_get_reg_ss2_ct8k(handle, reg_addr, reg_data);
	//printk("RDA5815 Get Reg [0x%02x]---- 0x%02x = 0x%02x\n", handle->m_device_ss2.tuner_cfg.tuner_dev_addr, reg_addr, *reg_data);
}

static void RDA5815Initial(MT_FE_CT8K_Device_Handle handle)
{
    //RDA5815WriteReg(register address,register data);

	RDA5815Sleep(1);//Wait 1ms.
	// Chip register soft reset
	RDA5815WriteReg(handle, 0x04, 0x04);
	RDA5815WriteReg(handle, 0x04, 0x05);

	// Initial configuration start

	//pll setting
	RDA5815WriteReg(handle, 0x1a, 0x13);
	RDA5815WriteReg(handle, 0x41, 0x53);
	RDA5815WriteReg(handle, 0x38, 0x9B);
	RDA5815WriteReg(handle, 0x39, 0x15);
	RDA5815WriteReg(handle, 0x3A, 0x00);
	RDA5815WriteReg(handle, 0x3B, 0x00);
	RDA5815WriteReg(handle, 0x3C, 0x0c);
	RDA5815WriteReg(handle, 0x0c, 0xE2);
	RDA5815WriteReg(handle, 0x2e, 0x6F);

#ifdef Xtal_27M
	RDA5815WriteReg(handle, 0x72, 0x07);	// v1.1, 1538~1539
	RDA5815WriteReg(handle, 0x73, 0x10);
	RDA5815WriteReg(handle, 0x74, 0x71);
	RDA5815WriteReg(handle, 0x75, 0x06); // v1.1, 1363~1364, 1862~1863
	RDA5815WriteReg(handle, 0x76, 0x40);
	RDA5815WriteReg(handle, 0x77, 0x89);
	RDA5815WriteReg(handle, 0x79, 0x04);	// v1.1, 900
	RDA5815WriteReg(handle, 0x7A, 0x2A);
	RDA5815WriteReg(handle, 0x7B, 0xAA);
	RDA5815WriteReg(handle, 0x7C, 0xAB);
#endif
#ifdef Xtal_30M
	RDA5815WriteReg(handle, 0x72, 0x06);	// v1.2, 1544~1545
	RDA5815WriteReg(handle, 0x73, 0x60);
	RDA5815WriteReg(handle, 0x74, 0x66);
	RDA5815WriteReg(handle, 0x75, 0x05); // v1.2, 1364~1365, 1859~1860
	RDA5815WriteReg(handle, 0x76, 0xA0);
	RDA5815WriteReg(handle, 0x77, 0x7B);
	RDA5815WriteReg(handle, 0x79, 0x03);	// v1.2, 901
	RDA5815WriteReg(handle, 0x7A, 0xC0);
	RDA5815WriteReg(handle, 0x7B, 0x00);
	RDA5815WriteReg(handle, 0x7C, 0x00);
#endif
#ifdef Xtal_24M
	RDA5815WriteReg(handle, 0x72, 0x08);	// v1.3, 1547~1548
	RDA5815WriteReg(handle, 0x73, 0x00);
	RDA5815WriteReg(handle, 0x74, 0x80);
	RDA5815WriteReg(handle, 0x75, 0x07); // v1.3, 1367~1368, 1859~1860
	RDA5815WriteReg(handle, 0x76, 0x10);
	RDA5815WriteReg(handle, 0x77, 0x9A);
	RDA5815WriteReg(handle, 0x79, 0x04);	// v1.3, 901
	RDA5815WriteReg(handle, 0x7A, 0xB0);
	RDA5815WriteReg(handle, 0x7B, 0x00);
	RDA5815WriteReg(handle, 0x7C, 0x00);
#endif

	RDA5815WriteReg(handle, 0x2f, 0x57);
	RDA5815WriteReg(handle, 0x0d, 0x70);
	RDA5815WriteReg(handle, 0x18, 0x4B);
	RDA5815WriteReg(handle, 0x30, 0xFF);
	RDA5815WriteReg(handle, 0x5c, 0xFF);
	RDA5815WriteReg(handle, 0x65, 0x00);
	RDA5815WriteReg(handle, 0x70, 0x3F);
	RDA5815WriteReg(handle, 0x71, 0x3F);
	RDA5815WriteReg(handle, 0x53, 0xA8);
	RDA5815WriteReg(handle, 0x46, 0x21);
	RDA5815WriteReg(handle, 0x47, 0x84);
	RDA5815WriteReg(handle, 0x48, 0x10);
	RDA5815WriteReg(handle, 0x49, 0x08);
	RDA5815WriteReg(handle, 0x60, 0x80);
	RDA5815WriteReg(handle, 0x61, 0x80);
	RDA5815WriteReg(handle, 0x6A, 0x08);
	RDA5815WriteReg(handle, 0x6B, 0x63);
	RDA5815WriteReg(handle, 0x69, 0xF8);
	RDA5815WriteReg(handle, 0x57, 0x64);
	RDA5815WriteReg(handle, 0x05, 0xaa);
	RDA5815WriteReg(handle, 0x06, 0xaa);
	RDA5815WriteReg(handle, 0x15, 0xAE);
	RDA5815WriteReg(handle, 0x4a, 0x67);
	RDA5815WriteReg(handle, 0x4b, 0x77);

	//agc setting

	RDA5815WriteReg(handle, 0x4f, 0x40);
	RDA5815WriteReg(handle, 0x5b, 0x20);

	RDA5815WriteReg(handle, 0x16, 0x0C);
	RDA5815WriteReg(handle, 0x18, 0x0C);
	RDA5815WriteReg(handle, 0x30, 0x1C);
	RDA5815WriteReg(handle, 0x5c, 0x2C);
	RDA5815WriteReg(handle, 0x6c, 0x3C);
	RDA5815WriteReg(handle, 0x6e, 0x3C);
	RDA5815WriteReg(handle, 0x1b, 0x7C);
	RDA5815WriteReg(handle, 0x1d, 0xBD);
	RDA5815WriteReg(handle, 0x1f, 0xBD);
	RDA5815WriteReg(handle, 0x21, 0xBE);
	RDA5815WriteReg(handle, 0x23, 0xBE);
	RDA5815WriteReg(handle, 0x25, 0xFE);
	RDA5815WriteReg(handle, 0x27, 0xFF);
	RDA5815WriteReg(handle, 0x29, 0xFF);
	RDA5815WriteReg(handle, 0xb3, 0xFF);
	RDA5815WriteReg(handle, 0xb5, 0xFF);

	RDA5815WriteReg(handle, 0x17, 0xF0);
	RDA5815WriteReg(handle, 0x19, 0xF0);
	RDA5815WriteReg(handle, 0x31, 0xF0);
	RDA5815WriteReg(handle, 0x5d, 0xF0);
	RDA5815WriteReg(handle, 0x6d, 0xF0);
	RDA5815WriteReg(handle, 0x6f, 0xF1);
	RDA5815WriteReg(handle, 0x1c, 0xF5);
	RDA5815WriteReg(handle, 0x1e, 0x35);
	RDA5815WriteReg(handle, 0x20, 0x79);
	RDA5815WriteReg(handle, 0x22, 0x9D);
	RDA5815WriteReg(handle, 0x24, 0xBE);
	RDA5815WriteReg(handle, 0x26, 0xBE);
	RDA5815WriteReg(handle, 0x28, 0xBE);
	RDA5815WriteReg(handle, 0x2a, 0xCF);
	RDA5815WriteReg(handle, 0xb4, 0xDF);
	RDA5815WriteReg(handle, 0xb6, 0x0F);

	RDA5815WriteReg(handle, 0xb7, 0x15);	//start
	RDA5815WriteReg(handle, 0xb9, 0x6c);
	RDA5815WriteReg(handle, 0xbb, 0x63);
	RDA5815WriteReg(handle, 0xbd, 0x5a);
	RDA5815WriteReg(handle, 0xbf, 0x5a);
	RDA5815WriteReg(handle, 0xc1, 0x55);
	RDA5815WriteReg(handle, 0xc3, 0x55);
	RDA5815WriteReg(handle, 0xc5, 0x47);
	RDA5815WriteReg(handle, 0xa3, 0x53);
	RDA5815WriteReg(handle, 0xa5, 0x4f);
	RDA5815WriteReg(handle, 0xa7, 0x4e);
	RDA5815WriteReg(handle, 0xa9, 0x4e);
	RDA5815WriteReg(handle, 0xab, 0x54);
	RDA5815WriteReg(handle, 0xad, 0x31);
	RDA5815WriteReg(handle, 0xaf, 0x43);
	RDA5815WriteReg(handle, 0xb1, 0x9f);

	RDA5815WriteReg(handle, 0xb8, 0x6c); //end
	RDA5815WriteReg(handle, 0xba, 0x92);
	RDA5815WriteReg(handle, 0xbc, 0x8a);
	RDA5815WriteReg(handle, 0xbe, 0x8a);
	RDA5815WriteReg(handle, 0xc0, 0x82);
	RDA5815WriteReg(handle, 0xc2, 0x93);
	RDA5815WriteReg(handle, 0xc4, 0x85);
	RDA5815WriteReg(handle, 0xc6, 0x77);
	RDA5815WriteReg(handle, 0xa4, 0x82);
	RDA5815WriteReg(handle, 0xa6, 0x7e);
	RDA5815WriteReg(handle, 0xa8, 0x7d);
	RDA5815WriteReg(handle, 0xaa, 0x6f);
	RDA5815WriteReg(handle, 0xac, 0x65);
	RDA5815WriteReg(handle, 0xae, 0x43);
	RDA5815WriteReg(handle, 0xb0, 0x9f);
	RDA5815WriteReg(handle, 0xb2, 0xf0);

	RDA5815WriteReg(handle, 0x81, 0x92); //rise
	RDA5815WriteReg(handle, 0x82, 0xb4);
	RDA5815WriteReg(handle, 0x83, 0xb3);
	RDA5815WriteReg(handle, 0x84, 0xac);
	RDA5815WriteReg(handle, 0x85, 0xba);
	RDA5815WriteReg(handle, 0x86, 0xbc);
	RDA5815WriteReg(handle, 0x87, 0xaf);
	RDA5815WriteReg(handle, 0x88, 0xa2);
	RDA5815WriteReg(handle, 0x89, 0xac);
	RDA5815WriteReg(handle, 0x8a, 0xa9);
	RDA5815WriteReg(handle, 0x8b, 0x9b);
	RDA5815WriteReg(handle, 0x8c, 0x7d);
	RDA5815WriteReg(handle, 0x8d, 0x74);
	RDA5815WriteReg(handle, 0x8e, 0x9f);
	RDA5815WriteReg(handle, 0x8f, 0xf0);

	RDA5815WriteReg(handle, 0x90, 0x15); //fall
	RDA5815WriteReg(handle, 0x91, 0x39);
	RDA5815WriteReg(handle, 0x92, 0x30);
	RDA5815WriteReg(handle, 0x93, 0x27);
	RDA5815WriteReg(handle, 0x94, 0x29);
	RDA5815WriteReg(handle, 0x95, 0x0d);
	RDA5815WriteReg(handle, 0x96, 0x10);
	RDA5815WriteReg(handle, 0x97, 0x1e);
	RDA5815WriteReg(handle, 0x98, 0x1a);
	RDA5815WriteReg(handle, 0x99, 0x19);
	RDA5815WriteReg(handle, 0x9a, 0x19);
	RDA5815WriteReg(handle, 0x9b, 0x32);
	RDA5815WriteReg(handle, 0x9c, 0x1f);
	RDA5815WriteReg(handle, 0x9d, 0x31);
	RDA5815WriteReg(handle, 0x9e, 0x43);

	RDA5815Sleep(10);//Wait 10ms;

	// Initial configuration end
}

/********************************************************************************/
//	Function to Set the RDA5815m
//	fPLL:		Frequency			unit: MHz from 950 to 2150
//	fSym:	SymbolRate			unit: KSps from 1000 to 60000
/********************************************************************************/
static INT32 RDA5815Set(MT_FE_CT8K_Device_Handle handle, unsigned long fPLL, unsigned long fSym)
{
	unsigned char buffer;//,buff;
	unsigned long temp_value = 0;
	unsigned long bw;/*,temp_value1 = 0,temp_value2=0 ;*/
	unsigned char Filter_bw_control_bit;

	RDA5815WriteReg(handle, 0x04, 0xc1); //add by rda 2011.8.9,RXON = 0 , change normal working state to idle state
	RDA5815WriteReg(handle, 0x2b, 0x95);//clk_interface_27m=0  add by rda 2012.1.12

	//set frequency start
#ifdef Xtal_27M		// v1.1
	temp_value = (unsigned long)fPLL* 77672;//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_30M		// v1.2
	temp_value = (unsigned long)fPLL* 69905;//((2^21) / RDA5815_XTALFREQ);
#endif
#ifdef Xtal_24M		// v1.3
	temp_value = (unsigned long)fPLL* 87381;//((2^21) / RDA5815_XTALFREQ);
#endif

	buffer = ((unsigned char)((temp_value>>24)&0xff));
	RDA5815WriteReg(handle, 0x07,buffer);
	buffer = ((unsigned char)((temp_value>>16)&0xff));
	RDA5815WriteReg(handle, 0x08,buffer);
	buffer = ((unsigned char)((temp_value>>8)&0xff));
	RDA5815WriteReg(handle, 0x09,buffer);
	buffer = ((unsigned char)(temp_value&0xff));
	RDA5815WriteReg(handle, 0x0a,buffer);
	//set frequency end

	// set Filter bandwidth start
	bw=fSym;		//kHz

	Filter_bw_control_bit = (unsigned char)((bw*135/200+4000)/1000);

	if(Filter_bw_control_bit<4)
		Filter_bw_control_bit = 4;    // MHz
	else if(Filter_bw_control_bit>40)
		Filter_bw_control_bit = 40;   // MHz

	Filter_bw_control_bit&=0x3f;
	Filter_bw_control_bit|=0x40;		//v1.5

	RDA5815WriteReg(handle, 0x0b,Filter_bw_control_bit);
	// set Filter bandwidth end

	RDA5815WriteReg(handle, 0x04, 0xc3);		//add by rda 2011.8.9,RXON = 0 ,rxon=1,normal working
	RDA5815WriteReg(handle, 0x2b, 0x97);		//clk_interface_27m=1  add by rda 2012.1.12
	RDA5815Sleep(5);//Wait 5ms;

	return 0;
}

// to get the lock status of tuner VCO & PLL
unsigned char RDA5815m_LockStatus(MT_FE_CT8K_Device_Handle handle)
{
	unsigned char buffer, i ;

	for(i = 0; i < 100; i ++)		// Loop times: 100 is recommanded; at least 30.
	{
		RDA5815ReadReg(handle, 0x03, &buffer);
		if((buffer & 0x03) != 0x03)
		{
			return 1;
		}
	}

	return 0;
}

// to set RDA tuner to sleep status (low power consumption)
unsigned char RDA5815m_Sleep(MT_FE_CT8K_Device_Handle handle)
{
	unsigned char buffer;

	RDA5815ReadReg(handle, 0x04, &buffer);
	if((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		RDA5815WriteReg(handle, 0x04, (U8)(buffer & 0x7F));
	}

	return 0;
}

// to wake up RDA tuner from sleep status
unsigned char RDA5815m_Wakeup(MT_FE_CT8K_Device_Handle handle)
{
	unsigned char buffer;

	RDA5815ReadReg(handle, 0x04, &buffer);
	if((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		RDA5815WriteReg(handle, 0x04, (U8)(buffer | 0x80));
	}

	return 0;
}

// to get the running status of RDA tuner -- "Sleeping" or "Waking".
unsigned char RDA5815m_RunningStatus(MT_FE_CT8K_Device_Handle handle)
{
	unsigned char buffer;

	RDA5815ReadReg(handle, 0x04, &buffer);
	if((buffer == 0xff) || (buffer == 0))
	{
		return 1;
	}
	else
	{
		if(buffer & 0x80)
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

unsigned char RDA5815m_CheckExist(MT_FE_CT8K_Device_Handle handle)	// To detect tuner's part number by reading chip ID registers
{
	unsigned char buffer;

	RDA5815ReadReg(handle, 0x00, &buffer);
	if(buffer == 0x58)
	{
		RDA5815ReadReg(handle, 0x01, &buffer);
		if(buffer == 0xf8)
		{
			return 0;	// RDA5815m
		}
		else
		{
			return 2;	// RDA chip, not RDA5815m
		}
	}
	else
	{
		return 1;	// not RDA chip, or IIC error
	}

	return 0;
}

MT_FE_RET mt_fe_tn_init_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	RDA5815Initial(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_set_freq_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	S32 ret = 0;

	ret = RDA5815Set(handle, freq_KHz / 1000, sym_rate_KSs);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_gain_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain)
{
    U8  freq_band;
    //unsigned int buf_reg16, buf_reg17;
    U32 i2v, filter, st1, st2, pre, post;
    U8  buffer;
    U32 buf_vga, gain_stage, stage_code;
    U32 max_vga = 30;     //unit : 0.1dB
    //U32 fPLL = g_stTunerOps[u32TunerPort].stCurrPara.u32Frequency / 1000;
    U32 fPLL = handle->m_device_ss2.input_params.input_freq_kHz / 1000;
    U32 total_gain;

    /* Gain stage limits                 st1  pre   st2 post i2v filter*/
#define RDA5815m_Gain_Stage__0 0xc00 /* '11   00    00  00   00  00 ' */
#define RDA5815m_Gain_Stage__1 0xc00 /* '11   00    00  00   00  00 ' */
#define RDA5815m_Gain_Stage__2 0xc01 /* '11   00    00  00   00  01 ' */
#define RDA5815m_Gain_Stage__3 0xc02 /* '11   00    00  00   00  10 ' */
#define RDA5815m_Gain_Stage__4 0xc03 /* '11   00    00  00   00  11 ' */
#define RDA5815m_Gain_Stage__5 0xc13 /* '11   00    00  01   00  11 ' */
#define RDA5815m_Gain_Stage__6 0xd17 /* '11   01    00  01   01  11 ' */
#define RDA5815m_Gain_Stage__7 0xd5b /* '11   01    01  01   10  11 ' */
#define RDA5815m_Gain_Stage__8 0xe5b /* '11   10    01  01   10  11 ' */
#define RDA5815m_Gain_Stage__9 0xf9b /* '11   11    10  01   10  11 ' */
#define RDA5815m_Gain_Stage_10 0xfab /* '11   11    10  10   10  11 ' */
#define RDA5815m_Gain_Stage_11 0xfaf /* '11   11    10  10   11  11 ' */
#define RDA5815m_Gain_Stage_12 0xfef /* '11   11    11  10   11  11 ' */
#define RDA5815m_Gain_Stage_13 0xfff /* '11   11    11  11   11  11 ' */
#define RDA5815m_Gain_Stage_14 0xfff /* '11   11    11  11   11  11 ' */
#define RDA5815m_Gain_Stage_15 0xfff /* '11   11    11  11   11  11 ' */

   /* gain band/gain_stage:    0     1       2     3     4      5      6     7    8     9    10    11    12    13    14    15 */
    S8 gain[13][16] = {
                            { -8,   -8,  -2,  4,  10,  16,  25,  34,  41,  49,  55,  61,  66,  72,  72,  72},  /*  1   950MHz <= Freq < 1000MHz */
                            { -7,   -7,  -1,  5,  11,  16,  26,  34,  41,  49,  55,  61,  66,  72,  72,  72},  /*  2  1000MHz <= Freq < 1100MHz */
                            { -7,   -7,  -1,  6,  12,  17,  27,  35,  41,  49,  55,  61,  65,  72,  72,  72},  /*  3  1100MHz <= Freq < 1200MHz */
                            { -6,   -6,   0,  6,  12,  18,  27,  35,  41,  49,  55,  61,  65,  72,  72,  72},  /*  4  1200MHz <= Freq < 1300MHz */
                            { -6,   -6,   0,  6,  12,  18,  27,  36,  41,  48,  54,  60,  65,  71,  71,  71},  /*  5  1300MHz <= Freq < 1400MHz */
                            { -6,   -6,   0,  6,  12,  18,  27,  35,  41,  48,  54,  60,  64,  71,  71,  71},  /*  6  1400MHz <= Freq < 1500MHz */
                            { -5,   -5,   1,  7,  13,  19,  28,  36,  41,  48,  54,  60,  64,  71,  71,  71},  /*  7  1500MHz <= Freq < 1600MHz */
                            { -4,   -4,   2,  8,  14,  19,  28,  37,  42,  48,  54,  60,  64,  71,  71,  71},  /*  8  1600MHz <= Freq < 1700MHz */
                            { -4,   -4,   2,  8,  14,  20,  28,  36,  41,  47,  54,  60,  63,  70,  70,  70},  /*  9  1700MHz <= Freq < 1800MHz */
                            { -4,   -5,   2,  8,  14,  19,  28,  37,  41,  47,  53,  59,  63,  70,  70,  70},  /* 10  1800MHz <= Freq < 1900MHz */
                            { -4,   -4,   2,  8,  14,  19,  28,  36,  40,  46,  53,  59,  62,  69,  69,  69},  /* 11  1900MHz <= Freq < 2000MHz */
                            { -4,   -4,   2,  8,  14,  20,  28,  36,  40,  46,  53,  59,  62,  69,  69,  69},  /* 12  2000MHz <= Freq < 2100MHz */
                            { -5,   -5,   1,  7,  13,  19,  28,  36,  40,  45,  52,  58,  61,  68,  68,  68}   /* 13  2100MHz <= Freq <= 2150MHz */
                        };

         if (fPLL <  950)           return -1;
    else if (fPLL < 1000)          {freq_band =  0;}
    else if (fPLL < 1100)          {freq_band =  1;}
    else if (fPLL < 1200)          {freq_band =  2;}
    else if (fPLL < 1300)          {freq_band =  3;}
    else if (fPLL < 1400)          {freq_band =  4;}
    else if (fPLL < 1500)          {freq_band =  5;}
    else if (fPLL < 1600)          {freq_band =  6;}
    else if (fPLL < 1700)          {freq_band =  7;}
    else if (fPLL < 1800)          {freq_band =  8;}
    else if (fPLL < 1900)          {freq_band =  9;}
    else if (fPLL < 2000)          {freq_band = 10;}
    else if (fPLL < 2100)          {freq_band = 11;}
    else if (fPLL <= 2150)         {freq_band = 12;}
    else                            return -1;

    RDA5815ReadReg(handle, 0x16, &buffer);  //i2v_gain_bit_0,   Filter_gain_bit_0,      lna_gain_st1_0,     lna_gain_st2_0
    i2v     = (unsigned int)(buffer & 0xc0) >> 6;
    filter  = (unsigned int)(buffer & 0x30) >> 4;
    st1     = (unsigned int)(buffer & 0x0c) >> 2;
    st2     = (unsigned int)(buffer & 0x03);

    RDA5815ReadReg(handle, 0x17, &buffer);  //Reserved,                                 lna_gain_pre_0,     lna_gain_post_0
    pre     = (unsigned int)(buffer & 0x0c) >> 2;
    post    = (unsigned int)(buffer & 0x03);

    RDA5815ReadReg(handle, 0xB7, &buffer);  //vga_gain_bit_stage_0_start[8:1]
    buf_vga = (unsigned int)buffer & 0xff;

    stage_code = (st1 << 10) + (pre << 8) + (st2 << 6) + (post << 4) + (i2v << 2) + (filter << 0);


         if (stage_code == RDA5815m_Gain_Stage_13) { gain_stage = 13; }
    else if (stage_code == RDA5815m_Gain_Stage_12) { gain_stage = 12; }
    else if (stage_code == RDA5815m_Gain_Stage_11) { gain_stage = 11; }
    else if (stage_code == RDA5815m_Gain_Stage_10) { gain_stage = 10; }
    else if (stage_code == RDA5815m_Gain_Stage__9) { gain_stage =  9; }
    else if (stage_code == RDA5815m_Gain_Stage__8) { gain_stage =  8; }
    else if (stage_code == RDA5815m_Gain_Stage__7) { gain_stage =  7; }
    else if (stage_code == RDA5815m_Gain_Stage__6) { gain_stage =  6; }
    else if (stage_code == RDA5815m_Gain_Stage__5) { gain_stage =  5; }
    else if (stage_code == RDA5815m_Gain_Stage__4) { gain_stage =  4; }
    else if (stage_code == RDA5815m_Gain_Stage__3) { gain_stage =  3; }
    else if (stage_code == RDA5815m_Gain_Stage__2) { gain_stage =  2; }
    else if (stage_code == RDA5815m_Gain_Stage__1) { gain_stage =  1; }
    else                                           { gain_stage =  0; }

    total_gain = gain[freq_band][gain_stage] * 10 + (buf_vga * max_vga * 10 / 255);      //unit: 0.1 dB

	*p_gain = total_gain;

	return MtFeErr_Ok;
}

#define STRENGTH_RATIO 100

MT_FE_RET mt_fe_tn_get_strength_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength)
{
	U32 gain = 0;

    //rssi = (unsigned char)((total_gain-min(gain))*255/(max(gain)+max_vga-min(gain)));
    //pu32SignalStrength[1] = 1070 - total_gain - 12;

	mt_fe_tn_get_gain_RDA5815M_ct8k(handle, &gain);

	*p_strength = (U8)((1070 - gain - 12) * 100 / 255);

	if(*p_strength > 100) *p_strength = 100;

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset)
{
	*p_offset = 0;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_sleep_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	RDA5815m_Sleep(handle);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_wake_up_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	RDA5815m_Wakeup(handle);

	return MtFeErr_Ok;
}

extern MT_FE_CT8K_Device_Handle mt_fe_ct8k_get_handle(void);

void mt_fe_ct8k_tn_attach_RDA5815M(void)
{
	MT_FE_CT8K_Device_Handle handle = mt_fe_ct8k_get_handle();

	handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_RDA5815M;

	if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
		handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x18;

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.tuner_cfg.tuner_open = 0;
	handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_RDA5815M_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_RDA5815M_ct8k;

	handle->m_device_ss2.tuner_cfg.tuner_set_application = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_loop_through = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_xtal = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_clkout = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_get_diagnose_info = NULL;

	handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
	handle->m_device_ss2.board_cfg.bAGCPolar = TRUE;

	handle->m_device_ss2.bs_cfg.bs_symrate = 40000;	// BLINDSCAN_SYMRATEKSs;

	return;
}


#endif

