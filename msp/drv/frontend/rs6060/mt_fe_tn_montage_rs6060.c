/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2019 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*****************************************************************************/
/* Filename:      mt_fe_tn_montage_rs6060.c
 *
 * Description:   Montage M88RS6060 Digital Satellite Tuner IC driver.
 *
 * Author:        Jerock Qin
 * Version:       0.00.14
 * Date:          2019-08-01
 *****************************************************************************/
/*****************************************************************************
 *  Version     Date        Author      Description
 * ---------------------------------------------------------------------------
 *  0.00.00     2017.03.16  Kenny           Create
 *  0.00.01     2017.03.24  Youzhong        Modify
 *  0.00.02     2017.03.28  Kenny           Modify
 *  0.00.03     2017.03.28  Lidan           Modify
 *  0.00.04     2017.05.04  Kenny           Modify
 *  0.00.05     2017.08.09  Daniel          Modify
 *  0.00.06     2017.08.10  Daniel          Modify
 *  0.00.07     2017.08.17  YZ.Huang        Modify
 *  0.00.08     2017.08.29  YZ.Huang        Modify
 *  0.00.09     2017.11.14  YZ.Huang        Modify
 *  0.00.10     2017.11.27  Kenny.Wu        Modify
 *  0.00.11     2018.01.16  Kenny.Wu        Modify
 *  0.00.12     2018.06.13  YZ.Huang        Modify
 *  0.00.13     2019.04.11  Daniel          Modify
 *  0.00.14     2019.08.01  Daniel          Modify
 *****************************************************************************/

#include "mt_fe_i2c_rs6060.h"

void _mt_fe_tn_set_pll_freq_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	U32 fcry_KHz;
	U8  refDiv1;
	U8  refDiv2;
	U8  ucLoDiv1;
	U8  ucLomod1;
	U8  ucLoDiv2;
	U8  ucLomod2;
	U8  lodiv_en_opt_div2;
	U8  temp;

	U32 ulNDiv1;
	U32 ulNDiv2;

	U8 reg27, reg29;

	U8 div1m;
	U8 div1p5m;

	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	fcry_KHz = tuner_handle->tuner_crystal_KHz;


	if (tuner_handle->tuner_crystal_KHz == 27000)
	{
		div1m = 19;
		div1p5m = 10;
		handle->tn_set_reg(handle, 0x41, 0x82);
	}
	else if(tuner_handle->tuner_crystal_KHz == 24000)
	{
		div1m = 16;
		div1p5m = 8;
		handle->tn_set_reg(handle, 0x41, 0x8a);
	}
	else
	{
		div1m = 19;
		div1p5m = 10;
		handle->tn_set_reg(handle, 0x41, 0x82);
	}

	if(tuner_handle->tuner_freq_MHz >= 1550)
	{
		ucLoDiv1 = 2;
		ucLomod1 = 0;
		refDiv1  = div1m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 1380)
	{
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1  = div1p5m;
		ucLoDiv2 = 2;
		ucLomod2 = 0;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 1070)
	{
		ucLoDiv1 = 3;
		ucLomod1 = 16;
		refDiv1  = div1p5m;
		ucLoDiv2 = 3;
		ucLomod2 = 16;
		refDiv2  = div1p5m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 1000)
	{
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1  = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 775)
	{
		ucLoDiv1 = 4;
		ucLomod1 = 64;
		refDiv1  = div1m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 700)
	{
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1  = div1p5m;
		ucLoDiv2 = 4;
		ucLomod2 = 64;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 520)
	{
		ucLoDiv1 = 6;
		ucLomod1 = 48;
		refDiv1  = div1p5m;
		ucLoDiv2 = 6;
		ucLomod2 = 48;
		refDiv2  = div1p5m;
		lodiv_en_opt_div2 = 0;
	}
	else if(tuner_handle->tuner_freq_MHz >= 375)
	{
		ucLoDiv1 = 8;
		ucLomod1 = 96;
		refDiv1  = div1m;
		ucLoDiv2 = 8;
		ucLomod2 = 96;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 0;
	}
	else
	{
		ucLoDiv1 = 12;
		ucLomod1 = 80;
		refDiv1  = div1m;
		ucLoDiv2 = 12;
		ucLomod2 = 80;
		refDiv2  = div1m;
		lodiv_en_opt_div2 = 1;
	}

	ulNDiv1 = ((tuner_handle->tuner_freq_MHz * ucLoDiv1 * 1000) * (refDiv1 + 8) / fcry_KHz - 1024) / 2;
	ulNDiv2 = ((tuner_handle->tuner_freq_MHz * ucLoDiv2 * 1000) * (refDiv2 + 8) / fcry_KHz - 1024) / 2;

	reg27 = (((ulNDiv1 >> 8) & 0x0F) + ucLomod1) & 0x7F;
	handle->tn_set_reg(handle, 0x27, reg27);
	handle->tn_set_reg(handle, 0x28, (U8)(ulNDiv1 & 0xFF));

	reg29 = (((ulNDiv2 >> 8) & 0x0F) + ucLomod2) & 0x7f;
	handle->tn_set_reg(handle, 0x29, reg29);
	handle->tn_set_reg(handle, 0x2a, (U8)(ulNDiv2 & 0xFF));

	refDiv1= refDiv1 & 0x1F;
	handle->tn_set_reg(handle, 0x36, refDiv1);

	handle->tn_set_reg(handle, 0x39, refDiv2);

	handle->tn_get_reg(handle, 0x3d, &temp);
	if(lodiv_en_opt_div2 == 1)
	{
		temp |= 0x80;
	}
	else
	{
		temp &= 0x7F;
	}
	handle->tn_set_reg(handle, 0x3d, temp);

	if(refDiv1 == 19)
	{
		handle->tn_set_reg(handle, 0x2c, 0x02);
	}
	else
	{
		handle->tn_set_reg(handle, 0x2c, 0x00);
	}

	return;
}


void _mt_fe_tn_set_bb_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	U32 f3dB;
	U8  reg40;

	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	f3dB = tuner_handle->tuner_symbol_rate_KSs * 9 / 14 + 2000;

	f3dB += tuner_handle->tuner_lpf_offset_KHz;

	if(f3dB < 6000)		f3dB = 6000;
	if(f3dB > 43000)	f3dB = 43000;

	reg40 = f3dB / 1000;
	handle->tn_set_reg(handle, 0x40, reg40);
}

MT_FE_RET mt_fe_tn_wakeup_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	handle->tn_set_reg(handle, 0x10, 0xfb);
	handle->tn_set_reg(handle, 0x11, 0x01);
	handle->tn_set_reg(handle, 0x07, 0x7d);

	handle->mt_sleep(10);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_sleep_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	if((tuner_handle->tuner_loop_through_enable == 1) && (tuner_handle->tuner_loop_through_mode == 1))
	{
		handle->tn_set_reg(handle, 0x10, 0x80);
		handle->tn_set_reg(handle, 0x11, 0x00);
		handle->tn_set_reg(handle, 0x07, 0x7d);
	}
	else
	{
		handle->tn_set_reg(handle, 0x07, 0x6d);
		handle->tn_set_reg(handle, 0x10, 0x00);
		handle->tn_set_reg(handle, 0x11, 0x00);
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_gain_rs6060(MT_FE_RS6060_Device_Handle handle, S32 *p_gain)
{
	static S32 bb_list_dBm[16][16] =
	{
		{-5000, -4999, -4397, -4044, -3795, -3601, -3442, -3309, -3193, -3090, -2999, -2916, -2840, -2771, -2706, -2647},
		{-2590, -2538, -2488, -2441, -2397, -2354, -2314, -2275, -2238, -2203, -2169, -2136, -2104, -2074, -2044, -2016},
		{-1988, -1962, -1936, -1911, -1886, -1862, -1839, -1817, -1795, -1773, -1752, -1732, -1712, -1692, -1673, -1655},
		{-1636, -1618, -1601, -1584, -1567, -1550, -1534, -1518, -1502, -1487, -1472, -1457, -1442, -1428, -1414, -1400},
		{-1386, -1373, -1360, -1347, -1334, -1321, -1309, -1296, -1284, -1272, -1260, -1249, -1237, -1226, -1215, -1203},
		{-1193, -1182, -1171, -1161, -1150, -1140, -1130, -1120, -1110, -1100, -1090, -1081, -1071, -1062, -1052, -1043},
		{-1034, -1025, -1016, -1007,  -999,  -990,  -982,  -973,  -965,  -956,  -948,  -940,  -932,  -924,  -916,  -908},
		{ -900,  -893,  -885,  -877,  -870,  -862,  -855,  -848,  -840,  -833,  -826,  -819,  -812,  -805,  -798,  -791},
		{ -784,  -778,  -771,  -764,  -758,  -751,  -745,  -738,  -732,  -725,  -719,  -713,  -706,  -700,  -694,  -688},
		{ -682,  -676,  -670,  -664,  -658,  -652,  -647,  -641,  -635,  -629,  -624,  -618,  -612,  -607,  -601,  -596},
		{ -590,  -585,  -580,  -574,  -569,  -564,  -558,  -553,  -548,  -543,  -538,  -533,  -528,  -523,  -518,  -513},
		{ -508,  -503,  -498,  -493,  -488,  -483,  -479,  -474,  -469,  -464,  -460,  -455,  -450,  -446,  -441,  -437},
		{ -432,  -428,  -423,  -419,  -414,  -410,  -405,  -401,  -397,  -392,  -388,  -384,  -379,  -375,  -371,  -367},
		{ -363,  -358,  -354,  -350,  -346,  -342,  -338,  -334,  -330,  -326,  -322,  -318,  -314,  -310,  -306,  -302},
		{ -298,  -294,  -290,  -287,  -283,  -279,  -275,  -271,  -268,  -264,  -260,  -257,  -253,  -249,  -246,  -242},
		{ -238,  -235,  -231,  -227,  -224,  -220,  -217,  -213,  -210,  -206,  -203,  -199,  -196,  -192,  -189,  -186}
	};

	S32 BB_Power = 0;
	U32 Total_Gain = 8000;
	S32 delta = 0;

	U8 reg5a, reg5f, reg77, reg76, reg3f;
	U8 reg96 = 0;

	U32  PGA2_cri_GS = 46, PGA2_crf_GS = 290, TIA_GS = 290;
	U32  RF_GC = 1200, IF_GC = 1100, BB_GC = 300, PGA2_GC = 300, TIA_GC = 300;
	U32  PGA2_cri = 0, PGA2_crf = 0;
	U32  RFG = 0, IFG = 0, BBG = 0, PGA2G = 0, TIAG = 0;

	U32 i = 0;

	U32 RFGS[13] = {0, 276, 278, 283, 272, 294, 296, 292, 292, 299, 305, 292, 300};
	U32 IFGS[12] = {0,   0, 232, 268, 266, 289, 295, 290, 291, 298, 304, 304};
	U32 BBGS[13] = {0, 296, 297, 295, 298, 302, 293, 292, 286, 294, 278, 298, 267};


	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	handle->tn_get_reg(handle, 0x5A, &reg5a);
	RF_GC = reg5a & 0x0f;

	handle->tn_get_reg(handle, 0x5F, &reg5f);
	IF_GC = reg5f & 0x0f;

	handle->tn_get_reg(handle, 0x3F, &reg3f);
	TIA_GC = (reg3f >> 4) & 0x07;

	handle->tn_get_reg(handle, 0x77, &reg77);
	BB_GC = (reg77 >> 4) & 0x0f;

	handle->tn_get_reg(handle, 0x76, &reg76);
	PGA2_GC = reg76 & 0x3f;
	PGA2_cri = PGA2_GC >> 2;
	PGA2_crf = PGA2_GC & 0x03;

	if(tuner_handle->tuner_freq_MHz >= 1750)
	{
		RFGS[1] = 240;
		RFGS[2] = 260;

		IFGS[2] = 200;
		IFGS[3] = 245;
		IFGS[4] = 255;
	}
	else if(tuner_handle->tuner_freq_MHz >= 1350)
	{
		RFGS[12] = 285;

	}
	else
	{
		RFGS[1] = 310;
		RFGS[2] = 293;

		IFGS[2] = 270;
		IFGS[3] = 290;
		IFGS[4] = 280;
		IFGS[11] = 320;
	}

	for(i = 0; i <= RF_GC; i ++)
	{
		RFG += RFGS[i];
	}
	for(i = 1; i <= IF_GC; i ++)
	{
		IFG += IFGS[i];
	}

	TIAG = TIA_GC * TIA_GS;

	for(i = 0; i <= BB_GC; i ++)
	{
		BBG += BBGS[i];
	}

	PGA2G = PGA2_cri * PGA2_cri_GS + PGA2_crf * PGA2_crf_GS;

	Total_Gain = RFG + IFG - TIAG + BBG + PGA2G;

	if(tuner_handle->tuner_freq_MHz >= 1750)
	{
		delta = 800;
	}
	else if(tuner_handle->tuner_freq_MHz >= 1350)
	{
		delta = 900;
	}
	else
	{
		delta = 1000;
	}

	handle->tn_get_reg(handle, 0x96, &reg96);
	BB_Power = bb_list_dBm[(reg96 >> 4) & 0x0f][reg96 & 0x0f];

	*p_gain = Total_Gain - delta - BB_Power;


	handle->dmd_get_reg(handle, 0x00, &reg96);

	return MtFeErr_Ok;
}


S16 mt_fe_tn_get_signal_strength_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	S32 level = -8000;
	S32 gain = 0;


#if 1		// percent

#if 1
	MT_FE_LOCK_STATE lockstate;

	mt_fe_tn_get_gain_rs6060(handle, &gain);

	if(gain > 8500)			level = 0;								//0%        no signal or weak signal
	else if(gain > 6500)	level = 0 + (8500 - gain) * 3 / 100;	//0% - 60%    weak signal
	else if(gain > 4500)	level = 60 + (6500 - gain) * 3 / 200;	//60% - 90%   normal signal
	else					level = 90 + (4500 - gain) / 500;		//90% - 99%   strong signal


	mt_fe_dmd_rs6060_get_pure_lock(handle, &lockstate);
	if((level < 40) && (lockstate == MtFeLockState_Locked))
		level = 20 + level / 2;
#else
	mt_fe_tn_get_gain_rs6060(handle, &gain);

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
#else		// level
	level = - (gain / 100);
#endif

	return level;
}

void mt_fe_tn_init_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	tuner_handle->tuner_init_OK			 = 1;
	//tuner_handle->tuner_dev_addr		 = 0x58;
	tuner_handle->tuner_crystal_KHz		 = 27000;
	tuner_handle->tuner_custom_cfg		 = 0;
	tuner_handle->tuner_driver_version	 = 14;
	tuner_handle->tuner_input_mode		 = 0;
	tuner_handle->tuner_clock_out		 = 0;
	tuner_handle->tuner_loop_through_enable  = 0;
	tuner_handle->tuner_loop_through_mode    = 0;  //0: loop through off when tuner sleep, 1: loop through on when tuner sleep


	handle->tn_set_reg(handle, 0x15, 0x6c);		//dmpll
	handle->tn_set_reg(handle, 0x2b, 0x1e);

	mt_fe_tn_wakeup_rs6060(handle);
	if(tuner_handle->tuner_clock_out == 1)
	{
		handle->tn_set_reg(handle, 0x24, 0x44);
	}
	else
	{
		handle->tn_set_reg(handle, 0x24, 0x04);
	}

	handle->tn_set_reg(handle, 0x6e, 0x39);

	if(tuner_handle->tuner_loop_through_enable == 1)
	{
		handle->tn_set_reg(handle, 0x83, 0x03);
	}
	else
	{
		handle->tn_set_reg(handle, 0x83, 0x01);
	}

	handle->tn_set_reg(handle, 0x70, 0x90);
	handle->tn_set_reg(handle, 0x71, 0xf0);
	handle->tn_set_reg(handle, 0x72, 0xB6);
	handle->tn_set_reg(handle, 0x73, 0xEB);
	handle->tn_set_reg(handle, 0x74, 0x6F);
	handle->tn_set_reg(handle, 0x75, 0xFc);

	return;
}

void mt_fe_tn_set_tuner_rs6060(MT_FE_RS6060_Device_Handle handle, U32 freq_MHz, U32 symbol_rate_KSs)
{
	MT_FE_Tuner_Handle_RS6060 tuner_handle = &handle->tuner_cfg;

	tuner_handle->tuner_freq_MHz = freq_MHz;
	tuner_handle->tuner_symbol_rate_KSs = symbol_rate_KSs;

	handle->tn_set_reg(handle, 0x5b, 0x4c);
	handle->tn_set_reg(handle, 0x5c, 0x54);
	handle->tn_set_reg(handle, 0x60, 0x4b);

	_mt_fe_tn_set_pll_freq_rs6060(handle);
	_mt_fe_tn_set_bb_rs6060(handle);

	handle->tn_set_reg(handle, 0x00, 0x01);
	handle->tn_set_reg(handle, 0x00, 0x00);

	return;
}

S32 mt_fe_tn_get_tuner_freq_offset_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	S32 freq_offset_KHz = 0;

	return freq_offset_KHz;
}

MT_FE_RET mt_fe_tn_set_freq_rs6060(MT_FE_RS6060_Device_Handle handle, U32 Freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	U32 freq_MHz = (Freq_KHz + 500) / 1000;

	handle->tuner_cfg.tuner_lpf_offset_KHz = lpf_offset_KHz;

	mt_fe_tn_set_tuner_rs6060(handle, freq_MHz, sym_rate_KSs);

	return MtFeErr_Ok;
}

void mt_fe_tn_adjust_AGC_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	handle->mt_sleep(20);
	handle->tn_set_reg(handle, 0x5b, 0xcc);
	handle->tn_set_reg(handle, 0x5c, 0xf4);
	handle->tn_set_reg(handle, 0x60, 0xcb);

	return;
}

void mt_fe_tn_reset_rs6060(MT_FE_RS6060_Device_Handle handle)
{
	handle->tn_set_reg(handle, 0x04, 0x01);
	handle->tn_set_reg(handle, 0x04, 0x00);
	handle->mt_sleep(1);

	return;
}


void mt_fe_tn_dmpll_select_xm_rs6060(MT_FE_RS6060_Device_Handle handle, U32 *xm_KHz, U32 symbol_rate, U32 tuner_freq_MHz)
{
	static U32 xm_list_KHz[3][8] = {
									{96000, 102400, 107162, 109714, 115200, 128000, 135529, 144000},
									{93000,  99200, 111600, 117473, 124000, 139500, 144000, 148800},
									{99000, 105600, 108000, 110511, 118800, 132000, 144000, 148500}
								   };

	U8  reg16;
	U32 offset_KHz[8] = {0};
	U32 max_offset = 0;

	U8 i, xm_line, xm_cnt = 5;

	xm_cnt = sizeof(xm_list_KHz) / sizeof(U32);
	xm_cnt /= 3;

	// C = (symbol * 1.35 / 2 + 2) * 1.1;
	symbol_rate *= 135;
	symbol_rate /= 200;
	symbol_rate += 2000;
	symbol_rate *= 110;
	symbol_rate /= 100;


	handle->tn_get_reg(handle, 0x16, &reg16);
	if(reg16 == 92)
	{
		xm_line = 1;
	}
	else if(reg16 == 100)
	{
		xm_line = 2;
	}
	else//if(reg16 == 96)
	{
		xm_line = 0;
	}

	for(i = 0; i < xm_cnt; i ++)
	{
		if(*xm_KHz > xm_list_KHz[xm_line][i])
		{
			continue;
		}

		offset_KHz[i] = ((tuner_freq_MHz * 1000) % xm_list_KHz[xm_line][i]);

		if(offset_KHz[i] > (xm_list_KHz[xm_line][i] / 2))
			offset_KHz[i] = xm_list_KHz[xm_line][i] - offset_KHz[i];

		if(offset_KHz[i] > symbol_rate)
		{
			*xm_KHz = xm_list_KHz[xm_line][i];

			break;
		}

		if(offset_KHz[i] > max_offset)
		{
			max_offset = offset_KHz[i];

			*xm_KHz = xm_list_KHz[xm_line][i];
		}
	}

	if(i == xm_cnt)
	{
		*xm_KHz = xm_list_KHz[xm_line][xm_cnt - 1];
	}

	return;
}

void mt_fe_tn_dmpll_select_mclk_rs6060(MT_FE_RS6060_Device_Handle handle, U32 tuner_freq_MHz, MT_BOOL bBs)
{
	U32 adc_Freq_MHz[3] = {96, 93, 99};
	U8  reg16_list[3] = {96, 92, 100}, reg15, reg16;
	U32 offset_MHz[3];
	U32 max_offset = 0;

	U8 i;

	adc_Freq_MHz[0] = 96;
	adc_Freq_MHz[1] = 93;
	adc_Freq_MHz[2] = 99;

	reg16_list[0] = 96;
	reg16_list[1] = 92;
	reg16_list[2] = 100;

	reg16 = 96;
	handle->global_cfg.iMclkKHz = 96000;

	if(!bBs)
	{
		if(handle->tp_cfg.iSymRateKSs >= 46000)
		{
			handle->global_cfg.iMclkKHz = 99000;
			reg16 = 100;
		}
		else
		{
			for(i = 0; i < 3; i ++)
			{
				offset_MHz[i] = tuner_freq_MHz % adc_Freq_MHz[i];

				if(offset_MHz[i] > (adc_Freq_MHz[i] / 2))
					offset_MHz[i] = adc_Freq_MHz[i] - offset_MHz[i];

				if(offset_MHz[i] > max_offset)
				{
					max_offset = offset_MHz[i];
					reg16 = reg16_list[i];
					handle->global_cfg.iMclkKHz = adc_Freq_MHz[i] * 1000;
				}
			}
		}
	}

	handle->tn_get_reg(handle, 0x15, &reg15);
	reg15 &= ~0x01;

	//handle->tn_set_reg(handle, 0x19, 0x07);

	handle->tn_set_reg(handle, 0x15, reg15);
	handle->tn_set_reg(handle, 0x16, reg16);

	handle->tn_set_reg(handle, 0x17, 0xc1);
	handle->tn_set_reg(handle, 0x17, 0x81);

	handle->mt_sleep(5);

	//handle->tn_set_reg(handle, 0x19, 0x01);
	//handle->mt_sleep(1);

	return;
}


void mt_fe_tn_dmpll_set_ts_mclk_rs6060(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE ts_mode, U32 MCLK_KHz)
{
	U8 reg15, reg16, reg1D, reg1E, reg1F, tmp;
	U8 sm, f0 = 0, f1 = 0, f2 = 0, f3 = 0;
	U16 pll_div_fb, N;
	U32 div;

	handle->tn_get_reg(handle, 0x15, &reg15);
	handle->tn_get_reg(handle, 0x16, &reg16);
	handle->tn_get_reg(handle, 0x1d, &reg1D);

	if(ts_mode != MtFeTsOutMode_Serial)
	{
		if(reg16 == 92)
		{
			tmp = 93;
		}
		else if(reg16 == 100)
		{
			tmp = 99;
		}
		else // if(reg16 == 96)
		{
			tmp = 96;
		}

		MCLK_KHz *= tmp;
		MCLK_KHz /= 96;
	}

	pll_div_fb = (reg15 & 0x01) << 8;
	pll_div_fb += reg16;
	pll_div_fb += 32;

	div = 9000 * pll_div_fb * 4;
	div /= MCLK_KHz;

	if(ts_mode == MtFeTsOutMode_Serial)
	{
		if(div <= 32)
		{
			N = 2;

			f0 = 0;
			f1 = div / N;
			f2 = div - f1;
			f3 = 0;
		}
#if 0
		else if(div <= 34)
		{
			N = 3;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = div - f0 - f1;
			f3 = 0;
		}
#endif
		else if(div <= 64)
		{
			N = 4;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = (div - f0 - f1) / (N - 2);
			f3 = div - f0 - f1 - f2;
		}
		else
		{
			N = 4;

			f0 = 16;
			f1 = 16;
			f2 = 16;
			f3 = 16;
		}


		if(f0 == 16)
			f0 = 0;
		else if((f0 < 8) && (f0 != 0))
			f0 = 8;

		if(f1 == 16)
			f1 = 0;
		else if((f1 < 8) && (f1 != 0))
			f1 = 8;

		if(f2 == 16)
			f2 = 0;
		else if((f2 < 8) && (f2 != 0))
			f2 = 8;

		if(f3 == 16)
			f3 = 0;
		else if((f3 < 8) && (f3 != 0))
			f3 = 8;
	}
	else
	{
		if(div <= 32)
		{
			N = 2;

			f0 = 0;
			f1 = div / N;
			f2 = div - f1;
			f3 = 0;
		}
		else if(div <= 48)
		{
			N = 3;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = div - f0 - f1;
			f3 = 0;
		}
		else if(div <= 64)
		{
			N = 4;

			f0 = div / N;
			f1 = (div - f0) / (N - 1);
			f2 = (div - f0 - f1) / (N - 2);
			f3 = div - f0 - f1 - f2;
		}
		else
		{
			N = 4;

			f0 = 16;
			f1 = 16;
			f2 = 16;
			f3 = 16;
		}

		if(f0 == 16)
			f0 = 0;
		else if((f0 < 9) && (f0 != 0))
			f0 = 9;

		if(f1 == 16)
			f1 = 0;
		else if((f1 < 9) && (f1 != 0))
			f1 = 9;

		if(f2 == 16)
			f2 = 0;
		else if((f2 < 9) && (f2 != 0))
			f2 = 9;

		if(f3 == 16)
			f3 = 0;
		else if((f3 < 9) && (f3 != 0))
			f3 = 9;
	}

	sm = N - 1;

	reg1D &= ~0x03;
	reg1D |= sm;
	reg1D |= 0x80;

	reg1E = ((f3 << 4) + f2) & 0xFF;
	reg1F = ((f1 << 4) + f0) & 0xFF;

	handle->tn_set_reg(handle, 0x1d, reg1D);
	handle->tn_set_reg(handle, 0x1e, reg1E);
	handle->tn_set_reg(handle, 0x1f, reg1F);
	handle->mt_sleep(1);

	return;
}


void mt_fe_tn_dmpll_get_ts_mclk_rs6060(MT_FE_RS6060_Device_Handle handle, U32 *p_MCLK_KHz)
{
	U8 reg15, reg16, reg1D, reg1E, reg1F;
	U8 sm, f0, f1, f2, f3;
	U16 pll_div_fb, N;
	U32 MCLK_KHz;

	*p_MCLK_KHz = MT_FE_MCLK_KHZ;

	handle->tn_get_reg(handle, 0x15, &reg15);
	handle->tn_get_reg(handle, 0x16, &reg16);
	handle->tn_get_reg(handle, 0x1d, &reg1D);
	handle->tn_get_reg(handle, 0x1e, &reg1E);
	handle->tn_get_reg(handle, 0x1f, &reg1F);

	MCLK_KHz = 9000;

	pll_div_fb = reg15 & 0x01;
	pll_div_fb <<= 8;
	pll_div_fb += reg16;

	MCLK_KHz *= (pll_div_fb + 32);

	sm = reg1D & 0x03;

	f3 = (reg1E >> 4) & 0x0F;
	f2 = reg1E & 0x0F;
	f1 = (reg1F >> 4) & 0x0F;
	f0 = reg1F & 0x0F;

	if(f3 == 0)		f3 = 16;
	if(f2 == 0)		f2 = 16;
	if(f1 == 0)		f1 = 16;
	if(f0 == 0)		f0 = 16;

	N = f2 + f1;

	switch(sm)
	{
		case 3:
			N = f3 + f2 + f1 + f0;
			break;

		case 2:
			N = f2 + f1 + f0;
			break;

		case 1:
		case 0:
		default:
			N = f2 + f1;
			break;
	}

	MCLK_KHz *= 4;
	MCLK_KHz /= N;

	*p_MCLK_KHz = MCLK_KHz;

	return;
}

