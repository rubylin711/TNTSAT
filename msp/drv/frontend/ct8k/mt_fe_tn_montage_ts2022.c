/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2014                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
 * Filename:      mt_fe_tn_montage_ts2022.c
 *
 * Description:   Montage M88TS2022 Digital Satellite Tuner IC driver.
 *
 * Author:        Daniel.Zhu
 * Version:       1.12.03
 * Date:          2017-06-08
 ****************************************************************************/

#include "mt_fe_def.h"

#if MT_FE_DMD_DVBS_S2_SUPPORT

#if 1//(MT_FE_TN_SUPPORT_TS2022 > 0)

#include "mt_fe_i2c_ct8k.h"


#define MT_FE_CRYSTAL_KHZ				27000		/* Crystal Frequency of M88TS2022 used, unit: KHz , range: 16000 - 32000 */

#define AVG_NUM							16
#define SIGNAL_STRENGTH_RATIO			100

#define TUNER_UNDEF						0
#define TUNER_M88TS2020					1			//define the tuner Version
#define TUNER_M88TS2022					2

#define DEMOD_TYPE_ALI					0			// define the demodulator type, if it is Ali chip, please set it to 1, else set it to 0

//extern void _mt_sleep(U32 ms);


static S32	g_tuner_freq_offset_KHz = 0;
static U8	gvTunerVersion = TUNER_M88TS2022;

static MT_FE_RET ts2022_tn_get_reg(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
	return _mt_fe_tn_get_reg_ss2_ct8k(handle, reg_addr, p_data);
}

static MT_FE_RET ts2022_tn_set_reg(MT_FE_CT8K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_tn_set_reg_ss2_ct8k(handle, reg_addr, reg_data);
}


/*****************************************************************************/
/* FUNCTIONS:*/
/* Place holders for user to define M88TS2022 register read and write routines.*/
/* These should be defined elsewhere by the user.*/
/*****************************************************************************/
static void mt_fe_WaitTime(U16 ms)
{					//Wait time , unit : ms
	_mt_delay_ct8k(ms);
}

static void mt_fe_SleepTime(U16 ms)
{
	//Wait time , unit : ms
	_mt_sleep_ct8k(ms);
}

/***************************************************************************/
/* Function to Check the tuner version: 0: Error  1: M88TS2020  2: M88TS2022*/
/***************************************************************************/

static U8 CheckTunerVersion(MT_FE_CT8K_Device_Handle handle)
{
	U8 buf = 0;

	// Wake Up the tuner

	ts2022_tn_get_reg(handle, 0x00, &buf);
	buf &= 0x03;

	if(buf == 0x00)
	{
		ts2022_tn_set_reg(handle, 0x00, 0x01);
		mt_fe_WaitTime(2);
	}
	ts2022_tn_set_reg(handle, 0x00, 0x03);
	mt_fe_WaitTime(2);

	//Check the tuner version

	ts2022_tn_get_reg(handle, 0x00, &buf);

	if((buf == 0x01) || (buf == 0x41) || (buf == 0x81))
	{
		return TUNER_M88TS2020;
	}
	else if((buf == 0xc3) || (buf == 0x83))
	{
		return TUNER_M88TS2022;
	}
	else
	{
		return TUNER_UNDEF;
	}
}

/***************************************************************************/
/* Function to Initialize the M88TS2022 */
/***************************************************************************/
static void InitialTuner(MT_FE_CT8K_Device_Handle handle)
{
	U8 buf = 0;

	gvTunerVersion = CheckTunerVersion(handle);	// check the tuner version and power on the tuner

	if(gvTunerVersion == TUNER_UNDEF)
	{
		return;
	}

	if(gvTunerVersion == TUNER_M88TS2020)
	{					//For M88TS2020
		if(TUNER_RFBYPASS_ON)
		{
			ts2022_tn_set_reg(handle, 0x62, 0xfd);
		}
		else
		{
			ts2022_tn_set_reg(handle, 0x62, 0xbd);
		}

		mt_fe_WaitTime(2);

		if(TUNER_CKOUT_ON)
		{
			if(TUNER_CKOUT_DIV)
			{
				buf = TUNER_CKOUT_DIV;
				buf &= 0x1f;
				ts2022_tn_set_reg(handle, 0x05, buf);
				mt_fe_WaitTime(2);
			}
			ts2022_tn_set_reg(handle, 0x42, 0x73);
			mt_fe_WaitTime(2);
		}
		else
		{
			ts2022_tn_set_reg(handle, 0x42, 0x63);
		}

		ts2022_tn_set_reg(handle, 0x07, 0x02);
		ts2022_tn_set_reg(handle, 0x08, 0x01);
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{								//For M88TS2022
		if(TUNER_RFBYPASS_ON)
		{
			ts2022_tn_set_reg(handle, 0x62, 0xec);
		}
		else
		{
			ts2022_tn_set_reg(handle, 0x62, 0x6c);
		}

		mt_fe_WaitTime(2);

		if(TUNER_CKOUT_XTAL)
		{
			ts2022_tn_set_reg(handle, 0x42, 0x6c);
			mt_fe_WaitTime(2);
		}
		else if(TUNER_CKOUT_ON)
		{
			if(TUNER_CKOUT_DIV)
			{
				buf = TUNER_CKOUT_DIV;
				buf &= 0x1f;
				ts2022_tn_set_reg(handle, 0x05, buf);
				mt_fe_WaitTime(2);
			}
			ts2022_tn_set_reg(handle, 0x42, 0x70);
			mt_fe_WaitTime(2);
		}
		else
		{
			ts2022_tn_set_reg(handle, 0x42, 0x60);
		}

		ts2022_tn_set_reg(handle, 0x7d, 0x9d);
		ts2022_tn_set_reg(handle, 0x7c, 0x9a);
		ts2022_tn_set_reg(handle, 0x7a, 0x76);

		ts2022_tn_set_reg(handle, 0x3b, 0xc1);
		ts2022_tn_set_reg(handle, 0x63, 0x88);

		ts2022_tn_set_reg(handle, 0x61, 0x85);
		ts2022_tn_set_reg(handle, 0x22, 0x30);
		ts2022_tn_set_reg(handle, 0x30, 0x40);
		ts2022_tn_set_reg(handle, 0x20, 0x23);
		ts2022_tn_set_reg(handle, 0x24, 0x02);
		ts2022_tn_set_reg(handle, 0x12, 0xa0);

		if(DEMOD_TYPE_ALI)
		{
			ts2022_tn_set_reg(handle, 0x07, 0x33);
			ts2022_tn_set_reg(handle, 0x24, 0x01);
		}
	}
}

/**************************************************************************
 * Function to Set the M88TS2022
 *fPLL:    Frequency         			unit: MHz	from 950 to 2150
 *fSym:    SymbolRate         			unit: KS/s  from 1000 to 45000
 *lpfOffset: Set the low pass filter offset when the demodulator set the PLL offset at low symbolrate  unit: KHz
 *gainHold:  The flag of AGC gain hold, the tuner gain is hold when gainHold == 1 , default please set gainHold = 0
 *return:	 Frequency offset of PLL 	 	unit: KHz
 **************************************************************************/
static S32 SetTuner(MT_FE_CT8K_Device_Handle handle, U32 fPLL, U32 fSym, U16 lpfOffset, U8 gainHold)
{
	U8		buf = 0, capCode = 0, div4 = 0, changePLL = 0, K = 0, lpf_mxdiv = 0, divMax = 0, divMin = 0, RFgain = 0;
	U32		gdiv28 = 0;
	U32		N = 0, lpf_gm = 0, f3dB = 0, fREF = 0, divN = 0, lpf_coeff = 0;
	S32		freqOffset = 0;

	//Initialize the tuner
	//InitialTuner();

	if(gvTunerVersion == TUNER_UNDEF)
	{
		return 0;
	}

	//Set the PLL

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		ts2022_tn_set_reg(handle, 0x10, 0x00);
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		ts2022_tn_set_reg(handle, 0x10, 0x0b);
		ts2022_tn_set_reg(handle, 0x11, 0x40);
	}

	div4 = 0;
	changePLL = 0;
	K = 0;
	divN = 0;
	N = 0;
	fREF = 2;

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		K = (MT_FE_CRYSTAL_KHZ / 1000 + 1) / 2 - 8;

		if (fPLL < 1146)
		{
			ts2022_tn_set_reg(handle, 0x10, 0x11);
			div4 = 1;
			divN = fPLL * (K + 8) * 4000 / MT_FE_CRYSTAL_KHZ;
		}
		else
		{
			ts2022_tn_set_reg(handle, 0x10, 0x01);
			divN = fPLL * (K + 8) * 2000 / MT_FE_CRYSTAL_KHZ;
		}

		divN =divN + divN % 2;

		N = divN - 1024;

		buf = (U8)((N >> 8) & 0x0f);
		ts2022_tn_set_reg(handle, 0x01, buf);

		buf = (U8)(N & 0xff);
		ts2022_tn_set_reg(handle, 0x02, buf);

		buf = K;
		ts2022_tn_set_reg(handle, 0x03, buf);
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		if(fREF == 1)
			K = MT_FE_CRYSTAL_KHZ / 1000 - 8;
		else
			K = (MT_FE_CRYSTAL_KHZ / 1000 + 1) / 2 - 8;

		if (fPLL < 1103)
		{
			ts2022_tn_set_reg(handle, 0x10, 0x1b);
			div4 = 1;
			divN = fPLL * (K+8) * 4000 / MT_FE_CRYSTAL_KHZ;
		}
		else
		{
			divN = fPLL * (K+8) * 2000 / MT_FE_CRYSTAL_KHZ;
		}

		divN = divN + divN % 2;

		if (divN < 4095)
		{
			N = divN - 1024;
		}
		else if (divN < 6143)
		{
			N = divN + 1024;
		}
		else
		{
			N = divN + 3072;
		}

		buf = (U8)((N >> 8) & 0x3f);
		ts2022_tn_set_reg(handle, 0x01, buf);

		buf = (U8)(N & 0xff);
		ts2022_tn_set_reg(handle, 0x02, buf);

		buf = K;
		ts2022_tn_set_reg(handle, 0x03, buf);
	}

	ts2022_tn_set_reg(handle, 0x51, 0x0f);
	ts2022_tn_set_reg(handle, 0x51, 0x1f);
	ts2022_tn_set_reg(handle, 0x50, 0x10);
	ts2022_tn_set_reg(handle, 0x50, 0x00);
	mt_fe_WaitTime(5);

	ts2022_tn_get_reg(handle, 0x15, &buf);
	if((buf & 0x40) != 0x40)
	{
		ts2022_tn_set_reg(handle, 0x51, 0x0f);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x10);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(5);
	}

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		ts2022_tn_get_reg(handle, 0x66, &buf);
		changePLL = (((buf & 0x80) >> 7) != div4);

		if(changePLL)
		{
			ts2022_tn_set_reg(handle, 0x10, 0x11);

			div4 = 1;

			divN = fPLL * (K + 8) * 4000 / MT_FE_CRYSTAL_KHZ;
			divN = divN + divN % 2;
			N = divN - 1024;

			buf = (U8)((N >>8) & 0x0f);
			ts2022_tn_set_reg(handle, 0x01, buf);

			buf = (U8)(N & 0xff);
			ts2022_tn_set_reg(handle, 0x02, buf);

			ts2022_tn_set_reg(handle, 0x51, 0x0f);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x10);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_WaitTime(5);
		}
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		ts2022_tn_get_reg(handle, 0x14, &buf);
		buf &= 0x7f;

		if(buf < 64)
		{
			ts2022_tn_get_reg(handle, 0x10, &buf);
			buf |= 0x80;
			ts2022_tn_set_reg(handle, 0x10, buf);
			ts2022_tn_set_reg(handle, 0x11, 0x6f);

			ts2022_tn_set_reg(handle, 0x51, 0x0f);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x10);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_WaitTime(5);

			ts2022_tn_get_reg(handle, 0x15, &buf);
			if((buf & 0x40) != 0x40)
			{
				ts2022_tn_set_reg(handle, 0x51, 0x0f);
				ts2022_tn_set_reg(handle, 0x51, 0x1f);
				ts2022_tn_set_reg(handle, 0x50, 0x10);
				ts2022_tn_set_reg(handle, 0x50, 0x00);
				mt_fe_WaitTime(5);
			}
		}

		ts2022_tn_get_reg(handle, 0x14, &buf);
		buf &= 0x1f;

		if(buf > 19)
		{
			ts2022_tn_get_reg(handle, 0x10, &buf);
			buf &= 0xfd;
			ts2022_tn_set_reg(handle, 0x10, buf);
		}
	}

	freqOffset = (S32)(divN * MT_FE_CRYSTAL_KHZ / (K + 8)/(div4 + 1) / 2 - fPLL * 1000);

	// set the RF gain
	if(gvTunerVersion == TUNER_M88TS2020)
	{
		ts2022_tn_set_reg(handle, 0x60, 0x79);
	}

	ts2022_tn_set_reg(handle, 0x51, 0x17);
	ts2022_tn_set_reg(handle, 0x51, 0x1f);
	ts2022_tn_set_reg(handle, 0x50, 0x08);
	ts2022_tn_set_reg(handle, 0x50, 0x00);
	mt_fe_WaitTime(5);

	ts2022_tn_get_reg(handle, 0x3c, &buf);
	if(buf == 0)
	{
		ts2022_tn_set_reg(handle, 0x51, 0x17);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x08);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(5);
	}

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		ts2022_tn_get_reg(handle, 0x3d, &buf);
		RFgain = (U8)(buf & 0x0f);

		if(RFgain < 15)
		{
			if(RFgain < 4)	RFgain = 0;
			else			RFgain = (U8)(RFgain - 3);

			buf = (U8)(((RFgain << 3) | 0x01) & 0x79);
			ts2022_tn_set_reg(handle, 0x60, buf);

			ts2022_tn_set_reg(handle, 0x51, 0x17);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x08);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_WaitTime(5);
		}
	}

	// set the LPF

	if(gvTunerVersion == TUNER_M88TS2022)
	{
		ts2022_tn_set_reg(handle, 0x25, 0x00);
		ts2022_tn_set_reg(handle, 0x27, 0x70);
		ts2022_tn_set_reg(handle, 0x41, 0x09);

		ts2022_tn_set_reg(handle, 0x08, 0x0b);
	}

	f3dB = fSym * 135 / 200 + 2000;

	f3dB += lpfOffset;

	if(f3dB < 7000)		f3dB = 7000;
	if(f3dB > 40000)	f3dB = 40000;

	gdiv28 = (MT_FE_CRYSTAL_KHZ / 1000 * 1694 + 500) / 1000;

	buf = (U8)gdiv28;
	ts2022_tn_set_reg(handle, 0x04, buf);

	ts2022_tn_set_reg(handle, 0x51, 0x1b);
	ts2022_tn_set_reg(handle, 0x51, 0x1f);
	ts2022_tn_set_reg(handle, 0x50, 0x04);
	ts2022_tn_set_reg(handle, 0x50, 0x00);
	mt_fe_WaitTime(2);

	ts2022_tn_get_reg(handle, 0x26, &buf);
	if(buf == 0x00)
	{
		ts2022_tn_set_reg(handle, 0x51, 0x1b);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x04);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(2);

		ts2022_tn_get_reg(handle, 0x26, &buf);
	}

	capCode = (U8)(buf & 0x3f);

	if(gvTunerVersion == TUNER_M88TS2022)
	{
		ts2022_tn_set_reg(handle, 0x41, 0x0d);

		ts2022_tn_set_reg(handle, 0x51, 0x1b);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x04);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(2);

		ts2022_tn_get_reg(handle, 0x26, &buf);
		if(buf == 0x00)
		{
			ts2022_tn_set_reg(handle, 0x51, 0x1b);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x04);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_WaitTime(2);

			ts2022_tn_get_reg(handle, 0x26, &buf);
		}

		buf &= 0x3f;
		capCode = (U8)((capCode + buf) / 2);
	}

	gdiv28 = gdiv28 * 207 / (capCode * 2 + 151);

	divMax = (U8)(gdiv28 * 135 / 100);
	divMin = (U8)(gdiv28 * 78 / 100);

	if(divMax > 63)	divMax = 63;

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		lpf_coeff = 2766;
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		lpf_coeff = 3200;
	}

	lpf_gm = (f3dB * gdiv28 * 2 / lpf_coeff / (MT_FE_CRYSTAL_KHZ / 1000) + 1) / 2;

	if (lpf_gm > 23)	lpf_gm = 23;
	if (lpf_gm < 1)		lpf_gm = 1;

	lpf_mxdiv = (U8)((lpf_gm * (MT_FE_CRYSTAL_KHZ / 1000) * lpf_coeff * 2 / f3dB + 1) / 2);

	if (lpf_mxdiv < divMin)
	{
		lpf_gm++;
		lpf_mxdiv = (U8)((lpf_gm * (MT_FE_CRYSTAL_KHZ / 1000) * lpf_coeff * 2 / f3dB + 1) / 2);
	}

	if (lpf_mxdiv > divMax)
	{
		lpf_mxdiv = divMax;
	}

	buf = lpf_mxdiv;
	ts2022_tn_set_reg(handle, 0x04, buf);

	buf = (U8)lpf_gm;
	ts2022_tn_set_reg(handle, 0x06, buf);

	ts2022_tn_set_reg(handle, 0x51, 0x1b);
	ts2022_tn_set_reg(handle, 0x51, 0x1f);
	ts2022_tn_set_reg(handle, 0x50, 0x04);
	ts2022_tn_set_reg(handle, 0x50, 0x00);
	mt_fe_WaitTime(2);

	ts2022_tn_get_reg(handle, 0x26, &buf);
	if(buf == 0x00)
	{
		ts2022_tn_set_reg(handle, 0x51, 0x1b);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x04);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(2);

		ts2022_tn_get_reg(handle, 0x26, &buf);
	}

	if(gvTunerVersion == TUNER_M88TS2022)
	{
		capCode = (U8)(buf & 0x3f);

		ts2022_tn_set_reg(handle, 0x41, 0x09);

		ts2022_tn_set_reg(handle, 0x51, 0x1b);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x04);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_WaitTime(2);

		ts2022_tn_get_reg(handle, 0x26, &buf);
		if(buf == 0x00)
		{
			ts2022_tn_set_reg(handle, 0x51, 0x1b);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x04);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_WaitTime(2);

			ts2022_tn_get_reg(handle, 0x26, &buf);
		}

		buf &= 0x3f;
		capCode = (U8)((capCode + buf) / 2);

		buf = (U8)(capCode | 0x80);
		ts2022_tn_set_reg(handle, 0x25, buf);
		ts2022_tn_set_reg(handle, 0x27, 0x30);

		ts2022_tn_set_reg(handle, 0x08, 0x09);
	}

	// Set the BB gain
	// default should set gainHold = 0;
	// except when the AGC of demodulator is hold, for example application at blind scan use Haier demodulator

	if(gainHold == 0)
	{
		ts2022_tn_set_reg(handle, 0x51, 0x1e);
		ts2022_tn_set_reg(handle, 0x51, 0x1f);
		ts2022_tn_set_reg(handle, 0x50, 0x01);
		ts2022_tn_set_reg(handle, 0x50, 0x00);
		mt_fe_SleepTime(20);

		ts2022_tn_get_reg(handle, 0x21, &buf);
		if(buf == 0x00)
		{
			ts2022_tn_set_reg(handle, 0x51, 0x1e);
			ts2022_tn_set_reg(handle, 0x51, 0x1f);
			ts2022_tn_set_reg(handle, 0x50, 0x01);
			ts2022_tn_set_reg(handle, 0x50, 0x00);
			mt_fe_SleepTime(20);
		}

		if(gvTunerVersion == TUNER_M88TS2020)
		{
			if(RFgain == 15)
			{
				mt_fe_SleepTime(20);
				ts2022_tn_get_reg(handle, 0x21, &buf);
				buf &= 0x0f;

				if(buf < 3)
				{
					ts2022_tn_set_reg(handle, 0x60, 0x61);

					ts2022_tn_set_reg(handle, 0x51, 0x17);
					ts2022_tn_set_reg(handle, 0x51, 0x1f);
					ts2022_tn_set_reg(handle, 0x50, 0x08);
					ts2022_tn_set_reg(handle, 0x50, 0x00);
					mt_fe_SleepTime(20);
				}
			}
		}

		//User should delay 100ms here to wait the Tuner gain stable before checking the chip lock status
		//If there have delay time at the function of setting demodulator, you can take out it to reduce the lock time;
		mt_fe_SleepTime(40);
	}

	return freqOffset;	// return the frequency offset : KHz
}


/***************************************************************************/
/* Function to set the M88TS2022 into Sleep mode                           */
/***************************************************************************/
static void SleepTuner(MT_FE_CT8K_Device_Handle handle)
{
	ts2022_tn_set_reg(handle, 0x00, 0x00);
}

/*************************************************************************/
/*  Function to get the tuner gain*/
/*  Vagc:the voltage of the AGC from the demodulator;	unit: mV  from 0 to 2600*/
/*  Return: total gain of tuner	in 0.01dB*/
/*  How to calculate the signal strength use this function please refer to the driver user's manual*/
/************************************************************************/
static S32	GetTunerGain(MT_FE_CT8K_Device_Handle handle, U16 Vagc)
{
	U8  buf = 0, gain1 = 0, gain2 = 0, gain3 = 0;
	S32 gain = 0;

	ts2022_tn_get_reg(handle, 0x3d, &buf);
	gain1 = (U8)(buf & 0x1f);
	ts2022_tn_get_reg(handle, 0x21, &buf);
	gain2 = (U8)(buf & 0x1f);

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		//if(gain1 < 0)		gain1 = 0;
		if(gain1 > 15)		gain1 = 15;
		//if(gain2 < 0)		gain2 = 0;
		if(gain2 > 13)		gain2 = 13;

		if(Vagc < 400)		Vagc = 400;
		if(Vagc > 1100)		Vagc = 1100;

		gain = (U16) gain1 * 233 + (U16) gain2 * 350 + Vagc * 24 / 10 + 1000;
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		ts2022_tn_get_reg(handle, 0x66, &buf);
		gain3 = (U8)((buf >> 3) & 0x07);

		//if(gain1 < 0)	gain1 = 0;
		if(gain1 > 15)	gain1 = 15;
		if(gain2 < 2)	gain2 = 2;
		if(gain2 > 16)	gain2 = 16;
		//if(gain3 < 0)	gain3 = 0;
		if(gain3 > 6)	gain3 = 6;

		if(Vagc < 600)	Vagc = 600;
		if(Vagc > 1600)	Vagc = 1600;

		gain = (U16) gain1 * 265 + (U16) gain2 * 338 + (U16) gain3 * 285 + Vagc * 176 / 100 - 3000;
	}

	return gain;
}


/*************************************************************************/
/*  This is a demo function to calculate the signal strength use M88TS2022GetTunerGain*/
/*  This is defined for applications using Montage DVB-S2 demodulator*/
/*  This function should be defined by users if they use a different type of demodulator*/
/*  Return: the signal strength	0% to 100%*/
/************************************************************************/
static U8 GetSignalStrength(MT_FE_CT8K_Device_Handle handle)
{
	U8  strength = 0;
	S32  Vagc = 0;
	S32 gain = 0;
	MT_FE_LOCK_STATE   	lockstate;

	U8 AgcPWM = 0;

	U8		i = 0;
	U16		sum = 0;
	static U8 signal_strength[AVG_NUM];
	static U8 signal_index = 0;

	// Step 1: read the AGC PWM rate from the demodulator register

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x3f, &AgcPWM);

	// Step 2: Calculate the AGC voltage based on the AGC PWM rate, unit: mV

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		Vagc = AgcPWM * 20 - 1166;
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		Vagc = AgcPWM * 16 - 670;
	}

	if(Vagc < 0)  Vagc = 0;

	// Step 3: Calculate the total gain of the tuner

	gain = GetTunerGain(handle, (U16)Vagc);			//Get the total tuner gain in 0.01dB

	// Step 4: Calculate the signal strength based on the total gain of the tuner

	if(gain > 8500)			strength = 0;									//0%           no signal or weak signal
	else if(gain > 6500)	strength = (U8)(0 + (8500 - gain) * 3 / 100);	//0% - 60%     weak signal
	else if(gain > 4500)	strength = (U8)(60 + (6500 - gain) * 3 / 200);	//60% - 90%    normal signal
	else					strength = (U8)(90 + (4500 - gain) / 500);		//90% - 99%    strong signal


	// Step 5: Adjust the display of the signal strength according to your requirements

	mt_fe_dmd_get_pure_lock_ct8k_ss2(handle, &lockstate);		// Get the lock status of the demodulator M88DS3002;

	// when the channel is locked, set the signal strength bigger

	if((strength < 40) && (lockstate == MtFeLockState_Locked))
		strength = (U8)(20 + strength / 2);

	//smooth the display of signal strength when the signal fluctuates, average AVG_NUM times

	signal_strength[signal_index] = strength;
	signal_index++;
	if (signal_index == AVG_NUM)
		signal_index = 0;

	sum = 0;
	for (i = 0; i < AVG_NUM; i++)
		sum += signal_strength[i];

	strength = (U8)(sum / AVG_NUM);

	//Scale the display by multiplying the signal strength with a coefficient

	strength = (U8)((U16)strength * SIGNAL_STRENGTH_RATIO / 100);

	//Limit the display within 0% to 100% to avoid errors

	if(strength > 100)  strength = 100;
	//if(strength < 0)    strength = 0;

	//Return the signal strength
	return (U8) strength;
}


/***************************************************************************/
/* Function to Initialize the M88TS2022 */
/***************************************************************************/
MT_FE_RET mt_fe_tn_init_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	InitialTuner(handle);

	handle->m_device_ss2.tuner_cfg.tuner_open = 1;
	return MtFeErr_Ok;
}

/*************************************************************************
** Function: mt_fe_tn_get_tuner_freq_offset_ts2022
**
**
** Description:	return the tuner freq. offset, unit: KHz
**
**
** Inputs: none
**
**
** Outputs:	none
**
**
*************************************************************************/
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset)
{
	*p_offset = g_tuner_freq_offset_KHz;

	return MtFeErr_Ok;
}


/*************************************************************************/
/*	Function to Set the M88TS2022 */
/*	freq_MHz:			U32	Frequency					unit: MHz  from 950 to 2150 */
/*	sym_rate_KSs:		U32	SymbolRate					unit: KS/s from 1000 to 45000 */
/*	lpf_offset_KHz:		S16	low pass filter offset		unit: KHz*/
/************************************************************************/
MT_FE_RET mt_fe_tn_set_freq_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	U32 freq_MHz = (freq_KHz + 500) / 1000;


	if(!handle->m_device_ss2.tuner_cfg.tuner_init_ok)
	{
		mt_fe_tn_init_ts2022_ct8k(handle);
	}

	g_tuner_freq_offset_KHz = SetTuner(handle, freq_MHz, sym_rate_KSs, lpf_offset_KHz, 0);

	g_tuner_freq_offset_KHz += (S32)(freq_MHz * 1000 - freq_KHz);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_gain_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain)
{
	S16 Vagc = 0;
	U32 gain = 0;

	U8 AgcPWM = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x3f, &AgcPWM);

	if(gvTunerVersion == TUNER_M88TS2020)
	{
		Vagc = (S16)(AgcPWM * 20 - 1166);
	}
	else if(gvTunerVersion == TUNER_M88TS2022)
	{
		Vagc = (S16)(AgcPWM * 16 - 670);
	}

	if(Vagc < 0)  Vagc = 0;

	gain = GetTunerGain(handle, (U16)Vagc);

	*p_gain = gain;

	return MtFeErr_Ok;
}

/***************************************************************************/
/* Function to set the M88TS2022 into Sleep mode*/
/***************************************************************************/
MT_FE_RET mt_fe_tn_sleep_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	SleepTuner(handle);
	mt_fe_SleepTime(50);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_wake_up_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	U8 val;

	ts2022_tn_get_reg(handle, 0x00, &val);
	if((val&0x01) == 0x00)
	{
		ts2022_tn_set_reg(handle, 0x00, 0x01);
		mt_fe_SleepTime(50);
	}

	ts2022_tn_set_reg(handle, 0x00, 0x03);
	mt_fe_SleepTime(50);

	return MtFeErr_Ok;
}

/*************************************************************************/
/*	This is a demo function to calculate the signal strength use mt_fe_tn_get_tuner_gain*/
/*  This is defined for applications using Montage demodulator*/
/*  This function should be defined by users if they use a different type of demodulator*/
/*	Return: the signal strength			 0% to 100%*/
/************************************************************************/
MT_FE_RET mt_fe_tn_get_strength_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength)
{
	*p_strength = GetSignalStrength(handle);

	return MtFeErr_Ok;
}


// fix 105318 start
extern MT_FE_CT8K_Device_Handle mt_fe_ct8k_get_handle(void);

void mt_fe_ct8k_tn_attach_ts2022(void)
{
	MT_FE_CT8K_Device_Handle handle = mt_fe_ct8k_get_handle();

	handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_TS2022;

	if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0)
	{
		handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0xc2;
	}

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.tuner_cfg.tuner_open = 0;

	handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_ts2022_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_ts2022_ct8k;

	handle->m_device_ss2.tuner_cfg.tuner_set_application = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_loop_through = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_xtal = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_clkout = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_get_diagnose_info = NULL;

	handle->m_device_ss2.board_cfg.bIQInverted = FALSE;
	handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;

	return;
}
// fix 105318 end


#endif/*#if (MT_FE_TN_SUPPORT_TS2022 > 0)*/

#endif
