/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*
 Filename:		RDA5815M_1.X.c
 Description:	RDA5815M Digital Satellite Tuner IC driver.
version 1.0		The primary version,  created by hongxin wang  2012-8-28
version 1.1		Modify the register 0x38 from 0x93 to 0x 9B for increasing the amplitude of XOUT
version 1.2		Add configuration for Xtal = 30MHz. 2013-04-26
version 1.3		Add configuration for Xtal = 24MHz. 2013-06-09
version 1.4		AGC optimized to enhance max gain.
version 1.4.1		Code simplified to avoid bugs for modes -- Xtal_24M and Xtal_30M.
version 1.5		Optimize bandwidth calculation. 2013-8-19
*/

#include "mt_fe_def_ds6103.h"
#include "mt_fe_i2c_ds6103.h"


typedef long int INT32;
typedef unsigned long UINT32;

typedef unsigned char AVL_uchar;
typedef unsigned long AVL_uint32;


extern MT_FE_DS6103_Device_Handle ds6103_handle;

#define Xtal_27M
//#define Xtal_30M
//#define Xtal_24M


static void RDA5815Sleep(U32 delay_ms)
{
	ds6103_handle->mt_sleep(delay_ms);
}

static void RDA5815WriteReg(U8 addr, U8 data)
{
	ds6103_handle->tn_set_reg(ds6103_handle, addr, data);
}

#if 0
static void RDA5815ReadReg(U8 addr, U8* p_data)
{
	ds6103_handle->tn_get_reg(ds6103_handle, addr, p_data);
}
#endif


static void RDA5815Initial(void)
{
//RDA5815WriteReg(register address,register data);

	RDA5815Sleep(1);//Wait 1ms.
	// Chip register soft reset
	RDA5815WriteReg(0x04,0x04);
	RDA5815WriteReg(0x04,0x05);

	// Initial configuration start

	//pll setting
	RDA5815WriteReg(0x1a,0x13);
	RDA5815WriteReg(0x41,0x53);
	RDA5815WriteReg(0x38,0x9B);
	RDA5815WriteReg(0x39,0x15);
	RDA5815WriteReg(0x3A,0x00);
	RDA5815WriteReg(0x3B,0x00);
	RDA5815WriteReg(0x3C,0x0c);
	RDA5815WriteReg(0x0c,0xE2);
	RDA5815WriteReg(0x2e,0x6F);

#ifdef Xtal_27M
	RDA5815WriteReg(0x72,0x07);	// v1.1, 1538~1539
	RDA5815WriteReg(0x73,0x10);
	RDA5815WriteReg(0x74,0x71);
	RDA5815WriteReg(0x75,0x06); // v1.1, 1363~1364, 1862~1863
	RDA5815WriteReg(0x76,0x40);
	RDA5815WriteReg(0x77,0x89);
	RDA5815WriteReg(0x79,0x04);	// v1.1, 900
	RDA5815WriteReg(0x7A,0x2A);
	RDA5815WriteReg(0x7B,0xAA);
	RDA5815WriteReg(0x7C,0xAB);
#endif
#ifdef Xtal_30M
	RDA5815WriteReg(0x72,0x06);	// v1.2, 1544~1545
	RDA5815WriteReg(0x73,0x60);
	RDA5815WriteReg(0x74,0x66);
	RDA5815WriteReg(0x75,0x05); // v1.2, 1364~1365, 1859~1860
	RDA5815WriteReg(0x76,0xA0);
	RDA5815WriteReg(0x77,0x7B);
	RDA5815WriteReg(0x79,0x03);	// v1.2, 901
	RDA5815WriteReg(0x7A,0xC0);
	RDA5815WriteReg(0x7B,0x00);
	RDA5815WriteReg(0x7C,0x00);
#endif
#ifdef Xtal_24M
	RDA5815WriteReg(0x72,0x08);	// v1.3, 1547~1548
	RDA5815WriteReg(0x73,0x00);
	RDA5815WriteReg(0x74,0x80);
	RDA5815WriteReg(0x75,0x07); // v1.3, 1367~1368, 1859~1860
	RDA5815WriteReg(0x76,0x10);
	RDA5815WriteReg(0x77,0x9A);
	RDA5815WriteReg(0x79,0x04);	// v1.3, 901
	RDA5815WriteReg(0x7A,0xB0);
	RDA5815WriteReg(0x7B,0x00);
	RDA5815WriteReg(0x7C,0x00);
#endif

	RDA5815WriteReg(0x2f,0x57);
	RDA5815WriteReg(0x0d,0x70);
	RDA5815WriteReg(0x18,0x4B);
	RDA5815WriteReg(0x30,0xFF);
	RDA5815WriteReg(0x5c,0xFF);
	RDA5815WriteReg(0x65,0x00);
	RDA5815WriteReg(0x70,0x3F);
	RDA5815WriteReg(0x71,0x3F);
	RDA5815WriteReg(0x53,0xA8);
	RDA5815WriteReg(0x46,0x21);
	RDA5815WriteReg(0x47,0x84);
	RDA5815WriteReg(0x48,0x10);
	RDA5815WriteReg(0x49,0x08);
	RDA5815WriteReg(0x60,0x80);
	RDA5815WriteReg(0x61,0x80);
	RDA5815WriteReg(0x6A,0x08);
	RDA5815WriteReg(0x6B,0x63);
	RDA5815WriteReg(0x69,0xF8);
	RDA5815WriteReg(0x57,0x64);
	RDA5815WriteReg(0x05,0xaa);
	RDA5815WriteReg(0x06,0xaa);
	RDA5815WriteReg(0x15,0xAE);
	RDA5815WriteReg(0x4a,0x67);
	RDA5815WriteReg(0x4b,0x77);

	//agc setting

	RDA5815WriteReg(0x4f,0x40);
	RDA5815WriteReg(0x5b,0x20);

	RDA5815WriteReg(0x16,0x0C);
	RDA5815WriteReg(0x18,0x0C);
	RDA5815WriteReg(0x30,0x1C);
	RDA5815WriteReg(0x5c,0x2C);
	RDA5815WriteReg(0x6c,0x3C);
	RDA5815WriteReg(0x6e,0x3C);
	RDA5815WriteReg(0x1b,0x7C);
	RDA5815WriteReg(0x1d,0xBD);
	RDA5815WriteReg(0x1f,0xBD);
	RDA5815WriteReg(0x21,0xBE);
	RDA5815WriteReg(0x23,0xBE);
	RDA5815WriteReg(0x25,0xFE);
	RDA5815WriteReg(0x27,0xFF);
	RDA5815WriteReg(0x29,0xFF);
	RDA5815WriteReg(0xb3,0xFF);
	RDA5815WriteReg(0xb5,0xFF);

	RDA5815WriteReg(0x17,0xF0);
	RDA5815WriteReg(0x19,0xF0);
	RDA5815WriteReg(0x31,0xF0);
	RDA5815WriteReg(0x5d,0xF0);
	RDA5815WriteReg(0x6d,0xF0);
	RDA5815WriteReg(0x6f,0xF1);
	RDA5815WriteReg(0x1c,0xF5);
	RDA5815WriteReg(0x1e,0x35);
	RDA5815WriteReg(0x20,0x79);
	RDA5815WriteReg(0x22,0x9D);
	RDA5815WriteReg(0x24,0xBE);
	RDA5815WriteReg(0x26,0xBE);
	RDA5815WriteReg(0x28,0xBE);
	RDA5815WriteReg(0x2a,0xCF);
	RDA5815WriteReg(0xb4,0xDF);
	RDA5815WriteReg(0xb6,0x0F);

	RDA5815WriteReg(0xb7,0x15);	//start
	RDA5815WriteReg(0xb9,0x6c);
	RDA5815WriteReg(0xbb,0x63);
	RDA5815WriteReg(0xbd,0x5a);
	RDA5815WriteReg(0xbf,0x5a);
	RDA5815WriteReg(0xc1,0x55);
	RDA5815WriteReg(0xc3,0x55);
	RDA5815WriteReg(0xc5,0x47);
	RDA5815WriteReg(0xa3,0x53);
	RDA5815WriteReg(0xa5,0x4f);
	RDA5815WriteReg(0xa7,0x4e);
	RDA5815WriteReg(0xa9,0x4e);
	RDA5815WriteReg(0xab,0x54);
	RDA5815WriteReg(0xad,0x31);
	RDA5815WriteReg(0xaf,0x43);
	RDA5815WriteReg(0xb1,0x9f);

	RDA5815WriteReg(0xb8,0x6c); //end
	RDA5815WriteReg(0xba,0x92);
	RDA5815WriteReg(0xbc,0x8a);
	RDA5815WriteReg(0xbe,0x8a);
	RDA5815WriteReg(0xc0,0x82);
	RDA5815WriteReg(0xc2,0x93);
	RDA5815WriteReg(0xc4,0x85);
	RDA5815WriteReg(0xc6,0x77);
	RDA5815WriteReg(0xa4,0x82);
	RDA5815WriteReg(0xa6,0x7e);
	RDA5815WriteReg(0xa8,0x7d);
	RDA5815WriteReg(0xaa,0x6f);
	RDA5815WriteReg(0xac,0x65);
	RDA5815WriteReg(0xae,0x43);
	RDA5815WriteReg(0xb0,0x9f);
	RDA5815WriteReg(0xb2,0xf0);

	RDA5815WriteReg(0x81,0x92); //rise
	RDA5815WriteReg(0x82,0xb4);
	RDA5815WriteReg(0x83,0xb3);
	RDA5815WriteReg(0x84,0xac);
	RDA5815WriteReg(0x85,0xba);
	RDA5815WriteReg(0x86,0xbc);
	RDA5815WriteReg(0x87,0xaf);
	RDA5815WriteReg(0x88,0xa2);
	RDA5815WriteReg(0x89,0xac);
	RDA5815WriteReg(0x8a,0xa9);
	RDA5815WriteReg(0x8b,0x9b);
	RDA5815WriteReg(0x8c,0x7d);
	RDA5815WriteReg(0x8d,0x74);
	RDA5815WriteReg(0x8e,0x9f);
	RDA5815WriteReg(0x8f,0xf0);

	RDA5815WriteReg(0x90,0x15); //fall
	RDA5815WriteReg(0x91,0x39);
	RDA5815WriteReg(0x92,0x30);
	RDA5815WriteReg(0x93,0x27);
	RDA5815WriteReg(0x94,0x29);
	RDA5815WriteReg(0x95,0x0d);
	RDA5815WriteReg(0x96,0x10);
	RDA5815WriteReg(0x97,0x1e);
	RDA5815WriteReg(0x98,0x1a);
	RDA5815WriteReg(0x99,0x19);
	RDA5815WriteReg(0x9a,0x19);
	RDA5815WriteReg(0x9b,0x32);
	RDA5815WriteReg(0x9c,0x1f);
	RDA5815WriteReg(0x9d,0x31);
	RDA5815WriteReg(0x9e,0x43);

	RDA5815Sleep(10);//Wait 10ms;

	// Initial configuration end
}

/********************************************************************************/
//	Function to Set the RDA5815
//	fPLL:		Frequency			unit: MHz from 250 to 2300
//	fSym:	SymbolRate			unit: KSps from 1000 to 60000
/********************************************************************************/

static INT32 RDA5815Set(unsigned long fPLL, unsigned long fSym )
{
	unsigned char buffer;//,buff;
	unsigned long temp_value = 0;
	unsigned long bw;/*,temp_value1 = 0,temp_value2=0 ;*/
	unsigned char Filter_bw_control_bit;

	RDA5815WriteReg(0x04,0xc1); //add by rda 2011.8.9,RXON = 0 , change normal working state to idle state
	RDA5815WriteReg(0x2b,0x95);//clk_interface_27m=0  add by rda 2012.1.12

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
	RDA5815WriteReg(0x07,buffer);
	buffer = ((unsigned char)((temp_value>>16)&0xff));
	RDA5815WriteReg(0x08,buffer);
	buffer = ((unsigned char)((temp_value>>8)&0xff));
	RDA5815WriteReg(0x09,buffer);
	buffer = ((unsigned char)( temp_value&0xff));
	RDA5815WriteReg(0x0a,buffer);
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

	RDA5815WriteReg(0x0b,Filter_bw_control_bit);
	// set Filter bandwidth end

	RDA5815WriteReg(0x04,0xc3);		//add by rda 2011.8.9,RXON = 0 ,rxon=1,normal working
	RDA5815WriteReg(0x2b,0x97);		//clk_interface_27m=1  add by rda 2012.1.12
	RDA5815Sleep(5);//Wait 5ms;

	return 1;
}

MT_FE_RET mt_fe_tn_init_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	ds6103_handle = handle;

	RDA5815Initial();

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_set_freq_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	S32 ret = 0;

	ds6103_handle = handle;

	ret = RDA5815Set(freq_KHz / 1000, sym_rate_KSs);

	return (ret == 1) ? MtFeErr_Ok : MtFeErr_Fail;
}

MT_FE_RET mt_fe_tn_get_gain_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain)
{
	U8 reg_0x3f, reg_0x40;
	U32 tmp;

	ds6103_handle = handle;

	handle->dmd_get_reg(handle, 0x3f, &reg_0x3f);
	handle->dmd_get_reg(handle, 0x40, &reg_0x40);

	tmp = reg_0x3f * 32 + reg_0x40;

	*p_gain = tmp;

	return MtFeErr_Ok;
}


#define STRENGTH_RATIO 100

MT_FE_RET mt_fe_tn_get_strength_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength)
{
	U32 gain_val;
	S32 strength;
	MT_FE_LOCK_STATE lock_state;

	ds6103_handle = handle;

	mt_fe_tn_get_gain_RDA5815M_ds6103(handle, &gain_val);

	if (gain_val <= 338)		strength = 99;										  //about -30dBm    99%
	else if (gain_val > 2080)	strength = 0;
	else 						strength = 99 - (gain_val - 338) * 10 / 156;


	mt_fe_dmd_ds6103_get_pure_lock(handle, &lock_state);
	if ((lock_state == MtFeLockState_Locked) && (strength < 40))
		strength = 20 + strength / 2;


	strength = strength * STRENGTH_RATIO / 100;


	if (strength >= 100) strength = 99;
	if (strength < 0) strength = 0;

	*p_gain = gain_val;
	*p_strength = strength;

	return MtFeErr_Ok;
}


S32 mt_fe_tn_get_tuner_freq_offset_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	ds6103_handle = handle;

	return 0;
}

