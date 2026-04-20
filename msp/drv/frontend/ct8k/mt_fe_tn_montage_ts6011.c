/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2019 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/* Filename:      mt_fe_tn_montage_ts6011.c
 *
 * Description:   Montage M88TS6011 Digital Satellite  Tuner driver.
 *
 * Author:        Jerock Qin
 * Version:       0.01.10
 * Date:          2019-08-01
 * Log:
 *  Version     Date        Author      Description
 * ---------------------------------------------------------------------------
 *  0.00.00     2016.03.24  Tuo.Ma          Create
 *  0.00.01     2016.05.11  Jerock.Qin      Modify
 *  0.00.02     2016.05.12  Jerock.Qin      Modify
 *  0.00.03     2016.05.13  Jerock.Qin      Modify
 *  0.00.04     2016.05.16  Jerock.Qin      Modify
 *  0.00.05     2016.05.18  Jerock.Qin      Modify
 *  0.00.06     2016.06.14  Jerock.Qin      Modify
 *  0.01.01     2016.08.03  Jerock.Qin      Modify
 *  0.01.02     2016.08.18  Jerock.Qin      Modify
 *  0.01.03     2016.08.22  Jerock.Qin      Modify
 *  0.01.04     2016.08.24  Jerock.Qin      Modify
 *  0.01.05     2016.09.19  Jerock.Qin      Modify
 *  0.01.06     2016.11.01  Jerock.Qin      Modify
 *  0.01.07     2017.08.02  Daniel          Modify
 *  0.01.08     2018.03.30  Daniel          Modify
 *  0.01.09     2019.01.08  Daniel          Modify
 *  0.01.10     2019.08.20  Daniel          Modify
 *****************************************************************************/


#include "mt_fe_tn_montage_ts6011.h"


#if MT_FE_DMD_DVBS_S2_SUPPORT

static MT_FE_TN_DEVICE_SETTINGS_TS6011 ts6011_config;
static MT_FE_Tuner_Handle_TS6011 ts6011_handle = &ts6011_config;
static MT_FE_CT8K_Device_Handle demod_handle;

static S32 _mt_fe_tn_get_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 *reg_data)
{
	return _mt_fe_tn_get_reg_ss2_ct8k(demod_handle, reg_addr, reg_data);
}

static S32 _mt_fe_tn_set_reg_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_tn_set_reg_ss2_ct8k(demod_handle, reg_addr, reg_data);
}

#if 0
static S32 _mt_fe_sat_tn_set_reg_bit_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U8 reg_addr, U8 data, U8 high_bit, U8 low_bit)
{
	U8 tmp = 0, value = 0;

	if(high_bit < low_bit)
	{
		tmp = high_bit;
		high_bit = low_bit;
		low_bit = tmp;
	}

	data <<= (7 + low_bit - high_bit);
	data &= 0xFF;
	data >>= (7 - high_bit);
	data &= 0xFF;

	tmp = 0xFF;
	tmp <<= (7 + low_bit - high_bit);
	tmp &= 0xFF;
	tmp >>= (7 - high_bit);
	tmp &= 0xFF;

	_mt_fe_tn_get_reg_ts6011(handle, reg_addr, &value);
	value &= ~tmp;
	value |= data;
	_mt_fe_tn_set_reg_ts6011(handle, reg_addr, value);

	return 0;
}
#endif

static void _mt_sleep_ts6011(U32 ticks_ms)
{					//Wait time , unit : ms
	_mt_sleep_ct8k(ticks_ms);
}

static void _mt_delay_ts6011(U32 ticks_ms)
{
	//Wait time , unit : ms
	_mt_delay_ct8k(ticks_ms);
}

static void _mt_fe_tn_set_pll_freq_ts6011(MT_FE_Tuner_Handle_TS6011 handle)
{
	U32 fcry_KHz;
	U8  refDiv1;
	U8  refDiv2;
	U8  ucLoDiv1;
	U8  ucLomod1;
	U8  ucLoDiv2;
	U8  ucLomod2;

	U32 ulNDiv1;
	U32 ulNDiv2;

	U8 reg27, reg29;

	U8 div1m;
	U8 div1p5m;

	fcry_KHz = handle->tuner_crystal_KHz;

	if(handle->tuner_crystal_KHz == 27000)
	{
		div1m = 19;
		div1p5m = 10;
		_mt_fe_tn_set_reg_ts6011(handle, 0x41, 0x82);
	}
	else if(handle->tuner_crystal_KHz == 24000)
	{
		div1m = 16;
		div1p5m = 8;
		_mt_fe_tn_set_reg_ts6011(handle, 0x41, 0x8a);
	}
	else
	{
		div1m = 19;
		div1p5m = 10;
		_mt_fe_tn_set_reg_ts6011(handle, 0x41, 0x82);
	}

	if(handle->tuner_freq_MHz >= 1550)
	{
		ucLoDiv1 = 2;
		ucLomod1 = 0;
		refDiv1  = div1m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2  = div1m;
	}
	else if(handle->tuner_freq_MHz >= 1380)
	{
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1  = div1p5m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2  = div1m;
	}
	else if(handle->tuner_freq_MHz >= 1070)
	{
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1  = div1p5m;
		ucLoDiv2 = 3;
		ucLomod2 = 16;
		refDiv2  = div1p5m;
	}
	else if(handle->tuner_freq_MHz >= 1000)
	{
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1  = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
	}
	else if(handle->tuner_freq_MHz >= 775)
	{
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1  = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
	}
	else if(handle->tuner_freq_MHz >= 700)
	{
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1  = div1p5m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
	}
	else if(handle->tuner_freq_MHz >= 520)
	{
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1  = div1p5m;
		ucLoDiv2 = 6;
		ucLomod2 = 48;
		refDiv2  = div1p5m;
	}
	else
	{
		ucLoDiv1 = 8;
		ucLomod1 = 96;
		refDiv1  = div1m;
		ucLoDiv2 = 8;
		ucLomod2 = 96;
		refDiv2  = div1m;
	}

	ulNDiv1 = ((handle->tuner_freq_MHz * ucLoDiv1 * 1000) * (refDiv1 + 8) / fcry_KHz - 1024) / 2;
	ulNDiv2 = ((handle->tuner_freq_MHz * ucLoDiv2 * 1000) * (refDiv2 + 8) / fcry_KHz - 1024) / 2;

	reg27 = (U8)((((ulNDiv1 >> 8) & 0x0F) + ucLomod1) & 0x7F);
	_mt_fe_tn_set_reg_ts6011(handle, 0x27, reg27);
	_mt_fe_tn_set_reg_ts6011(handle, 0x28, (U8)(ulNDiv1 & 0xFF));
	reg29 = (U8)((((ulNDiv2 >> 8) & 0x0F) + ucLomod2) & 0x7F);
	_mt_fe_tn_set_reg_ts6011(handle, 0x29, reg29);
	_mt_fe_tn_set_reg_ts6011(handle, 0x2a, (U8)(ulNDiv2 & 0xFF));
	refDiv1= (U8)(refDiv1 & 0x1F);
	_mt_fe_tn_set_reg_ts6011(handle, 0x36, refDiv1);
	_mt_fe_tn_set_reg_ts6011(handle, 0x39, refDiv2);

	_mt_fe_tn_set_reg_ts6011(handle, 0x2f, 0xf5);
	_mt_fe_tn_set_reg_ts6011(handle, 0x30, 0x05);

	if(refDiv1 == 19)
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x2c, 0x02);
	}
	else
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x2c, 0x00);
	}

	return;
}


static void _mt_fe_tn_set_bb_ts6011(MT_FE_Tuner_Handle_TS6011 handle)
{
	U32 f3dB;
	U8  reg40, tmp;

	f3dB = handle->tuner_symbol_rate_KSs * 9 / 14 + 2000;

	f3dB += handle->tuner_lpf_offset_KHz;

	if(f3dB < 6000)		f3dB = 6000;
	if(f3dB > 43000)	f3dB = 43000;

	reg40 = (U8)(f3dB / 1000);

	_mt_fe_tn_get_reg_ts6011(handle, 0x40, &tmp);
	tmp &= 0xC0;
	tmp |= (reg40 & 0x3F);
	_mt_fe_tn_set_reg_ts6011(handle, 0x40, tmp);
}

static S32 mt_fe_tn_ts6011_wakeup(MT_FE_Tuner_Handle_TS6011 handle)
{
	_mt_fe_tn_set_reg_ts6011(handle, 0x07, 0x7d);
	_mt_sleep_ts6011(10);
	_mt_fe_tn_set_reg_ts6011(handle, 0x6d, 0x1a);

	return 0;
}

static S32 mt_fe_tn_ts6011_sleep(MT_FE_Tuner_Handle_TS6011 handle)
{
	_mt_fe_tn_set_reg_ts6011(handle, 0x07, 0x6d);
	_mt_delay_ts6011(1);
	_mt_fe_tn_set_reg_ts6011(handle, 0x6d, 0x18);

	return 0;
}


/*************************************************************************/
/*  Function to get the tuner gain*/
/*  Vagc:  the voltage of the AGC from the demodulator;	unit: mV  from 0 to 3300*/
/*  Return: total gain of tuner	in 0.01dB */
/*  How to calculate the signal strength use this function please refer to the driver user's manual*/
/************************************************************************/
static S32 mt_fe_tn_ts6011_get_gain_ct8k(MT_FE_Tuner_Handle_TS6011 handle, U32 Vagc)
{
	U32 Total_Gain = 0, HIGH_G = 8350;
	U32 delta = 0;

	U8 reg5a, reg5f, reg70, reg72, reg3F;

	U32  RF_GC = 0, IF_GC = 0, BB_GC = 0, VGA_GC = 0, TIA_GC = 0;
	U32  RFG = 0, IFG = 0, BBG = 0, VGAG = 0, TIAG = 0, AGC_G = 0;

	U32 i = 0;

	U32 RFGS[13] = {0, 308, 294, 290, 285, 281, 289, 294, 296, 275, 293, 302, 329};
	U32 IFGS[11] = {0, 318, 304, 302, 298, 291, 303, 298, 281, 300, 276};
	U32 BBGS[13] = {0, 260, 297, 280, 289, 287, 296, 297, 304, 297, 293, 301, 295};

	S32 iGain;

	_mt_fe_tn_get_reg_ts6011(handle, 0x5A, &reg5a);
	RF_GC = reg5a & 0x0f;
	if(RF_GC > 12)
	{
		RF_GC = 12;
	}

	_mt_fe_tn_get_reg_ts6011(handle, 0x5F, &reg5f);
	IF_GC = reg5f & 0x0f;
	if(IF_GC > 10)
	{
		IF_GC = 10;
	}

	_mt_fe_tn_get_reg_ts6011(handle, 0x70, &reg70);
	BB_GC = reg70 & 0x0f;
	if(BB_GC > 12)
	{
		BB_GC = 12;
	}

	_mt_fe_tn_get_reg_ts6011(handle, 0x72, &reg72);
	VGA_GC = reg72 & 0x03;
	if(VGA_GC > 2)
	{
		VGA_GC = 2;
	}

	_mt_fe_tn_get_reg_ts6011(handle, 0x3F, &reg3F);
	TIA_GC = (reg3F >> 4) & 0x07;
	if(TIA_GC > 5)
	{
		TIA_GC = 5;
	}

	if(handle->tuner_freq_MHz > 1850)
	{
		RFGS[11] = 270;
		RFGS[12] = 230;

		IFGS[8] = 250;
		IFGS[9] = 240;
		IFGS[10] = 190;
	}
	else if(handle->tuner_freq_MHz > 1450)
	{
		RFGS[11] = 270;
		RFGS[12] = 265;

		IFGS[8] = 260;
		IFGS[9] = 265;
		IFGS[10] = 220;
	}

	for(i = 0; i <= (12 - RF_GC); i ++)
	{
		RFG += RFGS[i];
	}
	/*fix issue30287 stack-out-of-bounds*/
	for(i = 0; i <= (10 - IF_GC); i ++)
	{
		IFG += IFGS[i];
	}

	for(i = 0; i <= (12 - BB_GC); i ++)
	{
		BBG += BBGS[i];
	}

	VGAG = (2 - VGA_GC) * 3 * 100;

	TIAG = (5 - TIA_GC) * 3 * 100;


	if(Vagc < 1100)		Vagc = 1100;
	if(Vagc > 1800)		Vagc = 1800;

	AGC_G = Vagc - 1088;

	Total_Gain = HIGH_G - RFG - IFG -  BBG + TIAG - VGAG + AGC_G;

	if(handle->tuner_freq_MHz >= 1500)
	{
		delta = 1150;
	}
	else if(handle->tuner_freq_MHz > 1000)
	{
		delta = 1250;
	}
	else
	{
		delta = 1150;
	}

	iGain = (S32)(Total_Gain - delta + 1600);

	return iGain;
}

static void mt_fe_tn_ts6011_init_ct8k(MT_FE_Tuner_Handle_TS6011 handle)
{
	if(handle->tuner_init_OK == 0)
	{
		handle->tuner_init_OK			 = 1;
		handle->tuner_dev_addr			 = 0x58;

		handle->tuner_custom_cfg		 = 0;
		handle->tuner_version			 = 109;
		handle->tuner_time				 = 19010810;

		handle->tuner_crystal_KHz		 = 27000;
		handle->tuner_input_mode		 = 0;
		handle->tuner_clock_out			 = 1;     // "0" CLK-output disable, "1" CLK-output enable.
	}

	mt_fe_tn_ts6011_wakeup(handle);

	_mt_fe_tn_set_reg_ts6011(handle, 0x2B, 0x1e);
	_mt_fe_tn_set_reg_ts6011(handle, 0x2F, 0xf5);
	_mt_fe_tn_set_reg_ts6011(handle, 0x30, 0x05);
	_mt_fe_tn_set_reg_ts6011(handle, 0x82, 0x80);

	_mt_fe_tn_set_reg_ts6011(handle, 0x57, 0x2e);
	_mt_fe_tn_set_reg_ts6011(handle, 0x58, 0x02);
	_mt_fe_tn_set_reg_ts6011(handle, 0x46, 0x14);
	_mt_fe_tn_set_reg_ts6011(handle, 0x47, 0xec);
	_mt_fe_tn_set_reg_ts6011(handle, 0x4a, 0x0a);
	_mt_fe_tn_set_reg_ts6011(handle, 0x4b, 0xf6);

	_mt_fe_tn_set_reg_ts6011(handle, 0x91, 0x19);
	_mt_fe_tn_set_reg_ts6011(handle, 0x92, 0x45);

	_mt_fe_tn_set_reg_ts6011(handle, 0x6b, 0x10);
	_mt_fe_tn_set_reg_ts6011(handle, 0x77, 0x25);

	if(handle->tuner_input_mode == 1)
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x67, 0x2a);
		_mt_fe_tn_set_reg_ts6011(handle, 0x68, 0x05);
		_mt_fe_tn_set_reg_ts6011(handle, 0x6a, 0x06);
	}
	else
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x67, 0x08);
		_mt_fe_tn_set_reg_ts6011(handle, 0x68, 0x15);
		_mt_fe_tn_set_reg_ts6011(handle, 0x6a, 0x02);
	}


	if(handle->tuner_clock_out == 1)
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x0a, 0x00);
	}
	else
	{
		_mt_fe_tn_set_reg_ts6011(handle, 0x0a, 0x24);
	}

#if MT_FE_TS6011_DEBUG_PRNT
	mt_fe_print_ts6011(("TS6011 initialize OK!\n"));
#endif

	return;
}

static void mt_fe_tn_set_tuner_ts6011(MT_FE_Tuner_Handle_TS6011 handle, U32 freq_MHz, U32 symbol_rate_KSs)
{
#if MT_FE_TS6011_DEBUG_PRNT
	mt_fe_print_ts6011(("TS6011 set tuner frequency: %4d MHz, %5d KSs!\n", freq_MHz, symbol_rate_KSs));
#endif

	handle->tuner_freq_MHz = freq_MHz;
	handle->tuner_symbol_rate_KSs = symbol_rate_KSs;

	_mt_fe_tn_set_reg_ts6011(handle, 0x5a, 0x0c);
	_mt_fe_tn_set_reg_ts6011(handle, 0x5b, 0x4c);
	_mt_fe_tn_set_reg_ts6011(handle, 0x5c, 0x54);
	_mt_fe_tn_set_reg_ts6011(handle, 0x5f, 0x01);
	_mt_fe_tn_set_reg_ts6011(handle, 0x60, 0x8b);

	_mt_fe_tn_set_pll_freq_ts6011(handle);
	_mt_fe_tn_set_bb_ts6011(handle);

	_mt_fe_tn_set_reg_ts6011(handle, 0x6d, 0x1a);
	_mt_fe_tn_set_reg_ts6011(handle, 0x82, 0x80);
	_mt_fe_tn_set_reg_ts6011(handle, 0x00, 0x01);
	_mt_fe_tn_set_reg_ts6011(handle, 0x00, 0x00);

	return;
}

void mt_fe_tn_ts6011_adjust_AGC_ct8k(MT_FE_Tuner_Handle_TS6011 handle)
{
	_mt_sleep_ts6011(10);

	_mt_fe_tn_set_reg_ts6011(handle, 0x5b, 0xcc);
	_mt_fe_tn_set_reg_ts6011(handle, 0x5c, 0xf4);
	_mt_fe_tn_set_reg_ts6011(handle, 0x60, 0xcb);

	return;
}

static S32 mt_fe_tn_ts6011_get_freq_offset_ct8k(MT_FE_Tuner_Handle_TS6011 handle)
{
	//S32 freq_offset_KHz = 0;

	return 0;
}

static S32 mt_fe_tn_ts6011_set_freq_ct8k(MT_FE_Tuner_Handle_TS6011 handle, U32 Freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	U32 freq_MHz = (Freq_KHz + 500) / 1000;

	handle->tuner_lpf_offset_KHz = lpf_offset_KHz;
	mt_fe_tn_set_tuner_ts6011(handle, freq_MHz, sym_rate_KSs);

	return 0;
}

#define AVG_NUM							1
#define SIGNAL_STRENGTH_RATIO			100

/*************************************************************************/
/*  This is a demo function to calculate the signal strength use mt_fe_tn_ts6011_get_gain_ct8k*/
/*  This is defined for applications using Montage DVB-S2 demodulator*/
/*  This function should be defined by users if they use a different type of demodulator*/
/*  Return: the signal strength	0% to 100%*/
/************************************************************************/
S32 mt_fe_tn_ts6011_get_signal_strength(MT_FE_Tuner_Handle_TS6011 handle, S32 Vagc, MT_BOOL bLocked)
{
	U8  strength = 0;
	S32 gain = 0;

	U8		i = 0;
	U16		sum = 0;
	static U8 signal_strength[AVG_NUM];
	static U8 signal_index = 0;

	// Step 1: read the AGC PWM rate from the demodulator register

	// Step 2: Calculate the AGC voltage based on the AGC PWM rate, unit: mV

	// Step 3: Calculate the total gain of the tuner

	gain = mt_fe_tn_ts6011_get_gain_ct8k(handle, Vagc);  //Get the total tuner gain in 0.01dB

	// Step 4: Calculate the signal strength based on the total gain of the tuner

#if 0
	if(gain > 8500)			strength = 0;									//0%           no signal or weak signal
	else if(gain > 6500)	strength = (U8)(0 + (8500 - gain) * 3 / 100);	//0% - 60%     weak signal
	else if(gain > 4500)	strength = (U8)(60 + (6500 - gain) * 3 / 200);	//60% - 90%    normal signal
	else					strength = (U8)(90 + (4500 - gain) / 500);		//90% - 99%    strong signal
#else
	if(gain > 9500)			strength = 0;
	else if(gain > 9000)	strength = (U8)(0 + (9500 - gain) * 20 / 1000);
	else if(gain > 8000)	strength = (U8)(10 + (9000 - gain) * 10 / 1000);
	else if(gain > 7000)	strength = (U8)(20 + (8000 - gain) * 20 / 1000);
	else if(gain > 6000)	strength = (U8)(40 + (7000 - gain) * 15 / 1000);
	else if(gain > 5000)	strength = (U8)(55 + (6000 - gain) * 20 / 1000);
	else if(gain > 4000)	strength = (U8)(75 + (5000 - gain) * 15 / 1000);
	else if(gain > 3000)	strength = (U8)(90 + (4000 - gain) * 5 / 1000);
	else					strength = (U8)(95 + (3000 - gain) * 1 / 1000);
#endif


	// Step 5: Adjust the display of the signal strength according to your requirements

	//mt_fe_dmd_get_pure_lock_ct8k_ss2(handle, &lockstate);		// Get the lock status of the demodulator M88DS3002;

	// when the channel is locked, set the signal strength bigger

#if 0
	if((strength < 40) && bLocked)
		strength = (U8)(20 + strength / 2);
#endif

	//smooth the display of signal strength when the signal fluctuates, average AVG_NUM times

	signal_strength[signal_index] = strength;
	signal_index ++;
	if (signal_index == AVG_NUM)
		signal_index = 0;

	sum = 0;
	for (i = 0; i < AVG_NUM; i ++)
		sum += signal_strength[i];

	strength = (U8)(sum / AVG_NUM);

	//Scale the display by multiplying the signal strength with a coefficient

	strength = (U8)((U16)strength * SIGNAL_STRENGTH_RATIO / 100);

	//Limit the display within 0% to 100% to avoid errors

	if(strength > 100)  strength = 100;
	//if(strength < 0)    strength = 0;

	//Return the signal strength
	return  strength;
}


/***************************************************************************/
/* Function to Initialize the M88TS6011 */
/***************************************************************************/

MT_FE_RET mt_fe_tn_init_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	ts6011_handle->tuner_dev_addr = handle->m_device_ss2.tuner_cfg.tuner_dev_addr;

	mt_fe_tn_ts6011_init_ct8k(ts6011_handle);

	handle->m_device_ss2.tuner_cfg.tuner_open = 1;

	return MtFeErr_Ok;
}

/*************************************************************************
** Function: mt_fe_tn_get_tuner_freq_offset_ts6011_ct8k
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
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset)
{
	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	*p_offset = mt_fe_tn_ts6011_get_freq_offset_ct8k(ts6011_handle);

	return MtFeErr_Ok;
}


/*************************************************************************/
/*	Function to Set the M88TS6011 */
/*	freq_KHz:			U32	Frequency					unit: MHz  from 950000 to 2150000 */
/*	sym_rate_KSs:		U32	SymbolRate					unit: KS/s from 1000 to 45000 */
/*	lpf_offset_KHz:		S16	low pass filter offset		unit: KHz*/
/************************************************************************/

MT_FE_RET mt_fe_tn_set_freq_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	if(!handle->m_device_ss2.tuner_cfg.tuner_init_ok)
	{
		mt_fe_tn_init_ts6011_ct8k(handle);
	}

	mt_fe_tn_ts6011_set_freq_ct8k(ts6011_handle, freq_KHz, sym_rate_KSs, lpf_offset_KHz);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_gain_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain)
{
	S32 Vagc;
	U8  AgcPWM;

	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	_mt_fe_dmd_get_reg_ss2_ct8k(demod_handle, 0x3f, &AgcPWM);

	//Vagc = AgcPWM * 33 - 1024;
	Vagc = AgcPWM * 26 ;           //modify 20201203

	if(Vagc < 0)  Vagc = 0;

	*p_gain = mt_fe_tn_ts6011_get_gain_ct8k(ts6011_handle, Vagc);

	return MtFeErr_Ok;
}

/***************************************************************************/
/* Function to set the M88TS6011 into Sleep mode*/
/***************************************************************************/

MT_FE_RET mt_fe_tn_sleep_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	mt_fe_tn_ts6011_sleep(ts6011_handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_wake_up_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	mt_fe_tn_ts6011_wakeup(ts6011_handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_strength_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength)
{
	S32 level = -8000;
	S32 gain = 0;

	S32 Vagc;
	U8  AgcPWM;

#if 0
	MT_BOOL bLocked = FALSE;

	MT_FE_LOCK_STATE lockstate;
#endif

	demod_handle = handle;

	handle->m_device_ss2.tuner_cfg.tuner_handle = (U32)ts6011_handle;

	// Step 1: read the AGC PWM rate from the demodulator register

	_mt_fe_dmd_get_reg_ss2_ct8k(demod_handle, 0x3f, &AgcPWM);

	// Step 2: Calculate the AGC voltage based on the AGC PWM rate, unit: mV

//	Vagc = AgcPWM * 33 - 1024;
	Vagc = AgcPWM * 26;           //modify 20201203

	if(Vagc < 0)  Vagc = 0;


	gain = mt_fe_tn_ts6011_get_gain_ct8k(ts6011_handle, Vagc);


#if 0
	if(gain > 8500)			level = 0;								//0%        no signal or weak signal
	else if(gain > 6500)	level = 0 + (8500 - gain) * 3 / 100;	//0% - 60%    weak signal
	else if(gain > 4500)	level = 60 + (6500 - gain) * 3 / 200;	//60% - 90%   normal signal
	else					level = 90 + (4500 - gain) / 500;		//90% - 99%   strong signal


	mt_fe_dmd_get_pure_lock_ct8k_ss2(demod_handle, &lockstate);
	bLocked = (lockstate == MtFeLockState_Locked) ? TRUE : FALSE;

	if((level < 40) && bLocked)
		level = 20 + level / 2;
#else
	if(gain > 9500)			level = 0;
	else if(gain > 9000)	level = (U8)(0 + (9500 - gain) * 20 / 1000);
	else if(gain > 8000)	level = (U8)(10 + (9000 - gain) * 10 / 1000);
	else if(gain > 7000)	level = (U8)(20 + (8000 - gain) * 20 / 1000);
	else if(gain > 6000)	level = (U8)(40 + (7000 - gain) * 15 / 1000);
	else if(gain > 5000)	level = (U8)(55 + (6000 - gain) * 20 / 1000);
	else if(gain > 4000)	level = (U8)(75 + (5000 - gain) * 15 / 1000);
	else if(gain > 3000)	level = (U8)(90 + (4000 - gain) * 5 / 1000);
	else					level = (U8)(95 + (3000 - gain) * 1 / 1000);
#endif

	*p_strength = level;
	//*p_strength = gain/100;

	return MtFeErr_Ok;
}



extern MT_FE_CT8K_Device_Handle mt_fe_ct8k_get_handle(void);

void mt_fe_ct8k_tn_attach_ts6011(void)
{
	MT_FE_CT8K_Device_Handle handle = mt_fe_ct8k_get_handle();

	handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_TS6011;

	if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
		handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x58;

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.tuner_cfg.tuner_open = 0;
	handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_ts6011_ct8k;
	handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_ts6011_ct8k;

	handle->m_device_ss2.tuner_cfg.tuner_set_application = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_loop_through = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_xtal = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_clkout = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_get_diagnose_info = NULL;

	handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
	handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;

	return;
}



#endif

