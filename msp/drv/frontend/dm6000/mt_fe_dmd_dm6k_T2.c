/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2018                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_dm6k_T2.c
*
* Current version:	00.71
*
* Description:		M88DM6000 IC Driver For DVBT2 Mode.
*
* Log:	Description		Version		Date			Author
*------------------------------------------------------------------------
*		Create			00.00		2013.12.26		BJ.Wang
*		Modify			00.10		2014.01.10		BJ.Wang
*		Modify			00.20		2014.09.25		BJ.Wang
*		Modify			00.50		2015.02.15		BJ.Wang
*		Modify			00.60		2015.05.19		BJ.Wang
*		Modify			00.61		2015.06.10		BJ.Wang
*		Modify			00.62		2016.01.15		BJ.Wang
*		Modify			00.63		2016.07.04		BJ.Wang
*		Modify			00.64		2017.08.28		BJ.Wang
*		Modify			00.65		2018.02.28		YZ.Huang
*		Modify			00.70		2018.03.07		YZ.Huang
*		Modify			00.71		2018.04.18		YZ.Huang
********************************************************************************************************/

#include <linux/printk.h>

// #include <math.h>
#include "mt_fe_dmd_dm6k_T2.h"
#include "mt_fe_i2c_dm6k.h"
#include "mt_fe_dmd_fw_dm6k_T2.h"


typedef struct _MT_FE_CN_NORDIG
{
	MT_FE_MOD_MODE		smod;
	MT_FE_CODE_RATE		scode;
	U8					cn_perfor;
} MT_FE_CN_NORDIG;

typedef struct _MT_FE_DELTA
{
	MT_FE_T2_PILOT		dpilot;
	MT_FE_FFT			dfft;
	U8					delta_bp;
} MT_FE_DELTA;

typedef struct _MT_FE_LEVEL_NORDIG
{
	MT_FE_MOD_MODE		smod;
	MT_FE_CODE_RATE		scode;
	S8					level_ref;
} MT_FE_LEVEL_NORDIG;

static const MT_FE_CN_NORDIG cn_nordig[] =
{
	{MtFeModMode_Qpsk, MtFeCodeRate_1_2, 35}, {MtFeModMode_Qpsk, MtFeCodeRate_3_5, 47},
	{MtFeModMode_Qpsk, MtFeCodeRate_2_3, 56}, {MtFeModMode_Qpsk, MtFeCodeRate_3_4, 66},
	{MtFeModMode_Qpsk, MtFeCodeRate_4_5, 72}, {MtFeModMode_Qpsk, MtFeCodeRate_5_6, 77},

	{MtFeModMode_16Qam, MtFeCodeRate_1_2, 87}, {MtFeModMode_16Qam, MtFeCodeRate_3_5, 101},
	{MtFeModMode_16Qam, MtFeCodeRate_2_3, 114}, {MtFeModMode_16Qam, MtFeCodeRate_3_4, 125},
	{MtFeModMode_16Qam, MtFeCodeRate_4_5, 133}, {MtFeModMode_16Qam, MtFeCodeRate_5_6, 138},

	{MtFeModMode_64Qam, MtFeCodeRate_1_2, 130}, {MtFeModMode_64Qam, MtFeCodeRate_3_5, 148},
	{MtFeModMode_64Qam, MtFeCodeRate_2_3, 162}, {MtFeModMode_64Qam, MtFeCodeRate_3_4, 177},
	{MtFeModMode_64Qam, MtFeCodeRate_4_5, 187}, {MtFeModMode_64Qam, MtFeCodeRate_5_6, 194},

	{MtFeModMode_256Qam, MtFeCodeRate_1_2, 170}, {MtFeModMode_256Qam, MtFeCodeRate_3_5, 194},
	{MtFeModMode_256Qam, MtFeCodeRate_2_3, 208}, {MtFeModMode_256Qam, MtFeCodeRate_3_4, 229},
	{MtFeModMode_256Qam, MtFeCodeRate_4_5, 243}, {MtFeModMode_256Qam, MtFeCodeRate_5_6, 251}
};

static const MT_FE_DELTA delta_value[] =
{
	{MtFePilot_PP1, MtFeFFTMode_1K, 34}, {MtFePilot_PP1, MtFeFFTMode_2K, 35},
	{MtFePilot_PP1, MtFeFFTMode_4K, 39}, {MtFePilot_PP1, MtFeFFTMode_8K, 41},
	{MtFePilot_PP1, MtFeFFTMode_8E, 41}, {MtFePilot_PP1, MtFeFFTMode_16K, 41},
	{MtFePilot_PP1, MtFeFFTMode_16E, 42},

	{MtFePilot_PP2, MtFeFFTMode_1K, 32}, {MtFePilot_PP2, MtFeFFTMode_2K, 33},
	{MtFePilot_PP2, MtFeFFTMode_4K, 37}, {MtFePilot_PP2, MtFeFFTMode_8K, 39},
	{MtFePilot_PP2, MtFeFFTMode_8E, 41}, {MtFePilot_PP2, MtFeFFTMode_16K, 38},
	{MtFePilot_PP2, MtFeFFTMode_16E, 38}, {MtFePilot_PP2, MtFeFFTMode_32K, 37},
	{MtFePilot_PP2, MtFeFFTMode_32E, 37},

	{MtFePilot_PP3, MtFeFFTMode_1K, 44}, {MtFePilot_PP3, MtFeFFTMode_2K, 43},
	{MtFePilot_PP3, MtFeFFTMode_4K, 47}, {MtFePilot_PP3, MtFeFFTMode_8K, 49},
	{MtFePilot_PP3, MtFeFFTMode_8E, 50}, {MtFePilot_PP3, MtFeFFTMode_16K, 49},
	{MtFePilot_PP3, MtFeFFTMode_16E, 49}, {MtFePilot_PP3, MtFeFFTMode_32K, 48},
	{MtFePilot_PP3, MtFeFFTMode_32E, 48},

	{MtFePilot_PP4, MtFeFFTMode_1K, 42}, {MtFePilot_PP4, MtFeFFTMode_2K, 42},
	{MtFePilot_PP4, MtFeFFTMode_4K, 45}, {MtFePilot_PP4, MtFeFFTMode_8K, 48},
	{MtFePilot_PP4, MtFeFFTMode_8E, 48}, {MtFePilot_PP4, MtFeFFTMode_16K, 47},
	{MtFePilot_PP4, MtFeFFTMode_16E, 47}, {MtFePilot_PP4, MtFeFFTMode_32K, 45},
	{MtFePilot_PP4, MtFeFFTMode_32E, 45},

	{MtFePilot_PP5, MtFeFFTMode_1K, 48}, {MtFePilot_PP5, MtFeFFTMode_2K, 47},
	{MtFePilot_PP5, MtFeFFTMode_4K, 51}, {MtFePilot_PP5, MtFeFFTMode_8K, 53},
	{MtFePilot_PP5, MtFeFFTMode_8E, 52}, {MtFePilot_PP5, MtFeFFTMode_16K, 52},
	{MtFePilot_PP5, MtFeFFTMode_16E, 52},

	{MtFePilot_PP6, MtFeFFTMode_16K, 49},	{MtFePilot_PP6, MtFeFFTMode_16E, 49},
	{MtFePilot_PP6, MtFeFFTMode_32K, 48},	{MtFePilot_PP6, MtFeFFTMode_32E, 48},

	{MtFePilot_PP7, MtFeFFTMode_1K, 29}, {MtFePilot_PP7, MtFeFFTMode_2K, 29},
	{MtFePilot_PP7, MtFeFFTMode_4K, 34}, {MtFePilot_PP7, MtFeFFTMode_8K, 37},
	{MtFePilot_PP7, MtFeFFTMode_8E, 39}, {MtFePilot_PP7, MtFeFFTMode_16K, 33},
	{MtFePilot_PP7, MtFeFFTMode_16E, 34}, {MtFePilot_PP7, MtFeFFTMode_32K, 33},
	{MtFePilot_PP7, MtFeFFTMode_32E, 33},

	{MtFePilot_PP8, MtFeFFTMode_8K, 37},{MtFePilot_PP8, MtFeFFTMode_8E, 38},
	{MtFePilot_PP8, MtFeFFTMode_16K, 35},	{MtFePilot_PP8, MtFeFFTMode_16E, 35},
	{MtFePilot_PP8, MtFeFFTMode_32K, 35},	{MtFePilot_PP8, MtFeFFTMode_32E, 35}
};

static const MT_FE_LEVEL_NORDIG level_nordig[] =
{
	{MtFeModMode_Qpsk, MtFeCodeRate_1_2, -96}, {MtFeModMode_Qpsk, MtFeCodeRate_3_5, -95},
	{MtFeModMode_Qpsk, MtFeCodeRate_2_3, -94}, {MtFeModMode_Qpsk, MtFeCodeRate_3_4, -93},
	{MtFeModMode_Qpsk, MtFeCodeRate_4_5, -92}, {MtFeModMode_Qpsk, MtFeCodeRate_5_6, -92},

	{MtFeModMode_16Qam, MtFeCodeRate_1_2, -91}, {MtFeModMode_16Qam, MtFeCodeRate_3_5, -89},
	{MtFeModMode_16Qam, MtFeCodeRate_2_3, -88}, {MtFeModMode_16Qam, MtFeCodeRate_3_4, -87},
	{MtFeModMode_16Qam, MtFeCodeRate_4_5, -86}, {MtFeModMode_16Qam, MtFeCodeRate_5_6, -86},

	{MtFeModMode_64Qam, MtFeCodeRate_1_2, -86}, {MtFeModMode_64Qam, MtFeCodeRate_3_5, -85},
	{MtFeModMode_64Qam, MtFeCodeRate_2_3, -83}, {MtFeModMode_64Qam, MtFeCodeRate_3_4, -82},
	{MtFeModMode_64Qam, MtFeCodeRate_4_5, -81}, {MtFeModMode_64Qam, MtFeCodeRate_5_6, -80},

	{MtFeModMode_256Qam, MtFeCodeRate_1_2, -82}, {MtFeModMode_256Qam, MtFeCodeRate_3_5, -80},
	{MtFeModMode_256Qam, MtFeCodeRate_2_3, -78}, {MtFeModMode_256Qam, MtFeCodeRate_3_4, -76},
	{MtFeModMode_256Qam, MtFeCodeRate_4_5, -75}, {MtFeModMode_256Qam, MtFeCodeRate_5_6, -74}
};

static const U32 snr_log10[] =
{
	0,		3010,	4771,	6021, 	6990,	7781,	8451,	9031,	9542,	10000,
	10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,
	13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,
	14914,	15052,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,
	16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,
	17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,
	17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,
	18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031,
	19085,	19138,	19191,	19243,	19294,	19345,	19395,	19445,	19494,	19542
};

MT_FE_RET _mt_fe_dmd_get_reg_t2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
	return _mt_fe_dm6k_get_reg(handle->sys_dev_addr, reg_addr, p_data);
}

MT_FE_RET _mt_fe_dmd_set_reg_t2(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_dm6k_set_reg(handle->sys_dev_addr, reg_addr, reg_data);
}


/*******************************************************************************************
** Function: mt_fe_dmd_get_driver_version_t2_dm6k
**
**
** Description:	This function is used to get the driver version for dvbt2 mode
**
**
** Inputs:   None
**
** Outputs:
**
**	  Parameter			Type		Description
**	-------------------------------------------------------------
**	 p_version			U8*		version number pointer
**
**********************************************************************************************/
MT_FE_RET mt_fe_dmd_get_driver_version_dm6k_t2(U8* p_version)
{
	*p_version = 71;		/* driver version number */

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_init_reg_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	_mt_fe_dmd_set_reg_t2(handle, 0x1e, 0x13);
	_mt_fe_dmd_set_reg_t2(handle, 0x1f, 0x10);
	_mt_fe_dmd_set_reg_t2(handle, 0x5f, 0x50);
	_mt_fe_dmd_set_reg_t2(handle, 0x51, 0xdd);
	_mt_fe_dmd_set_reg_t2(handle, 0x53, 0x40);
	_mt_fe_dmd_set_reg_t2(handle, 0x55, 0xf7);
	_mt_fe_dmd_set_reg_t2(handle, 0x50, 0x50);
	_mt_fe_dmd_set_reg_t2(handle, 0xf0, 0x00);
	_mt_fe_dmd_set_reg_t2(handle, 0x20, 0x23);
	_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0x22);

	_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0x66);
	_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0xad);

	_mt_fe_dmd_set_reg_t2(handle, 0x29, 0x2c);
	_mt_fe_dmd_set_reg_t2(handle, 0x28, 0x71);
	_mt_fe_dmd_set_reg_t2(handle, 0x27, 0xc7);

	_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x00);
	_mt_fe_dmd_set_reg_t2(handle, 0x31, 0xca);
	_mt_fe_dmd_set_reg_t2(handle, 0x35, 0x8a);
	_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x22);
	_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x25);
	_mt_fe_dmd_set_reg_t2(handle, 0x86, 0xc6);
	_mt_fe_dmd_set_reg_t2(handle, 0x87, 0x16);
	_mt_fe_dmd_set_reg_t2(handle, 0x88, 0x39);

	_mt_fe_dmd_set_reg_t2(handle, 0x30, 0x00);
	_mt_fe_dmd_set_reg_t2(handle, 0x33, 0x00);
	_mt_fe_dmd_set_reg_t2(handle, 0x9d, 0x00);

	_mt_fe_dmd_set_reg_t2(handle, 0xb1, 0xff);
	_mt_fe_dmd_set_reg_t2(handle, 0x80, 0x10);
	_mt_fe_dmd_set_reg_t2(handle, 0x81, 0x40);
	_mt_fe_dmd_set_reg_t2(handle, 0x7f, 0x2b);
	_mt_fe_dmd_set_reg_t2(handle, 0xbf, 0x1f);
	_mt_fe_dmd_set_reg_t2(handle, 0x87, 0x16);
	_mt_fe_dmd_set_reg_t2(handle, 0x88, 0x39);

	_mt_fe_dmd_set_reg_t2(handle, 0x8c, 0x20);
	_mt_fe_dmd_set_reg_t2(handle, 0x8d, 0x16);
	_mt_fe_dmd_set_reg_t2(handle, 0x8e, 0x70);
	_mt_fe_dmd_set_reg_t2(handle, 0x8f, 0x70);

	_mt_fe_dmd_set_reg_t2(handle, 0x61, 0x30);
	_mt_fe_dmd_set_reg_t2(handle, 0x62, 0x30);
	_mt_fe_dmd_set_reg_t2(handle, 0x6e, 0x80);
	_mt_fe_dmd_set_reg_t2(handle, 0xe8, 0x30);
	_mt_fe_dmd_set_reg_t2(handle, 0x49, 0x82);

	_mt_fe_dmd_set_reg_t2(handle, 0x72, 0x00);
	_mt_fe_dmd_set_reg_t2(handle, 0xd0, 0x32);
	_mt_fe_dmd_set_reg_t2(handle, 0xf0, 0x04);
	_mt_fe_dmd_set_reg_t2(handle, 0x74, 0x04);
	_mt_fe_dmd_set_reg_t2(handle, 0x89, 0x19);
	_mt_fe_dmd_set_reg_t2(handle, 0x84, 0xb4);
	_mt_fe_dmd_set_reg_t2(handle, 0x82, 0x2f);

	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x49);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x06);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x48);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x02);

	_mt_fe_dmd_set_reg_t2(handle, 0x73, 0x00);

	/////ADC
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x66);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x88);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x67);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x02);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x68);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xf1);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6a);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x88);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6b);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x01);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6c);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x1d);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6d);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x02);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6e);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xcb);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x6f);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x00);

	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x70);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x1a);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x71);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x01);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x72);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xd8);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x73);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x02);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x74);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x58);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x75);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x07);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x76);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xd2);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x77);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x02);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x78);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x88);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x79);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x01);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x7a);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x97);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x7b);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x07);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x7c);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xb2);
	/////////ADC
	_mt_fe_dmd_set_reg_t2(handle, 0x7a, 0x04);
	_mt_fe_dmd_set_reg_t2(handle, 0xec, 0x41);
	_mt_fe_dmd_set_reg_t2(handle, 0x76, 0x11);
	_mt_fe_dmd_set_reg_t2(handle, 0x77, 0x94);

	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x24);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x7f);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x40);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x07);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x41);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x07);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x42);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0x07);
	_mt_fe_dmd_set_reg_t2(handle, 0xb3, 0x27);
	_mt_fe_dmd_set_reg_t2(handle, 0xb4, 0xdc);

	return ret;
}

/*	FUNCTIN:
**		_mt_fe_dmd_download_fw_dm6k_t2
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_download_fw_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U32 i = 0, w_size = 0, firm_size = 0, firm_last_size = 0;
	U8 tmp[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	U8 chip_fir[2] = {0, 0};
	const U8* p_fw = m88dm6k_fm_t2;
	const U8* p_check = m88dm6k_fm_check;

	if(handle->m_device_ctt2.demod_type != MtFeType_DVBT2)
	{
		return MtFeErr_NoMatch;
	}

	if ((handle->m_device_ctt2.mcu_status != 2) && (handle->m_device_ctt2.mcu_status != 3))
	{
		firm_size = sizeof(m88dm6k_fm_t2) / 16;
		firm_last_size = sizeof(m88dm6k_fm_t2) % 16;
		_mt_fe_dmd_set_reg_t2(handle, 0xf2, 0x00);
		_mt_fe_dmd_set_reg_t2(handle, 0xf3, 0x00);
		_mt_fe_dmd_set_reg_t2(handle, 0xf9, 0xff);
		_mt_fe_dmd_set_reg_t2(handle, 0xfa, 0x00);
		_mt_fe_dmd_set_reg_t2(handle, 0xf0, 0x03);

		_mt_sleep_dm6k(1);

		tmp[0] = 0xf1;
		for (i = 0; i < firm_size; i ++)
		{
			for(w_size = 1; w_size < 17; w_size ++)
			{
				tmp[w_size] = *(p_fw ++);
			}

			ret = _mt_fe_dmd_write_dm6k(handle->sys_dev_addr, tmp, 17);
			if (ret != MtFeErr_Ok)
				break;
		}

		for(w_size = 1; w_size < (firm_last_size + 1); w_size ++)
		{
			tmp[w_size] = *(p_fw ++);
		}

		ret = _mt_fe_dmd_write_dm6k(handle->sys_dev_addr, tmp, (U16)(firm_last_size + 1));

		_mt_fe_dmd_set_reg_t2(handle, 0xf0, 0x04);

		_mt_fe_dmd_get_reg_t2(handle, 0xf9, &chip_fir[0]);
		_mt_fe_dmd_get_reg_t2(handle, 0xfa, &chip_fir[1]);
		if ((chip_fir[0] != p_check[0]) || (chip_fir[1] != p_check[1]))
		{
			mt_fe_print(("MT: _mt_fe_dmd_download_fw_dm6k_t2 FAILED!    [ret = %d]\n", ret));
			return MtFeErr_FirmwareErr;
		}
		_mt_fe_dmd_soft_reset_dm6k_t2(handle);
		handle->m_device_ctt2.mcu_status = 2;
	}

	if(handle->m_device_ctt2.mcu_status == 3)
		handle->m_device_ctt2.mcu_status = 2;

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_dm6k_t2
**
**	DESCRIPTION:
**		select the serial interface or parallel interface.
**
**	IN:
**
**
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_set_output_mode_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	U8	tmp = 0;
	MT_FE_TS_OUT_MODE mode = MtFeTsOutMode_Common;

	mode = handle->m_device_ctt2.ts_out_mode;
	_mt_fe_dmd_get_reg_t2(handle, 0xec, &tmp);


	if (mode == MtFeTsOutMode_Serial)
	{
		tmp &= ~0x04;
	}
	else if(mode == MtFeTsOutMode_Parallel)
	{
		tmp |= 0x04;
		_mt_fe_dmd_set_reg_t2(handle, 0x06, 0x01);
	}
	else if(mode == MtFeTsOutMode_Common)
	{
		tmp |= 0x0c;
	}
	else	// undef, return undef error code
	{
		return MtFeErr_Undef;
	}

	_mt_fe_dmd_set_reg_t2(handle, 0xec, tmp);

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_set_IF_dm6k_t2
**
**	DESCRIPTION:
**		set IF.
**
**	IN:
**		IF_Khz	-	IF frequency(khz)
**				-
**
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_set_IF_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U32 IF_Khz)
{
	U32 IF_set = 0, FC_INT = 0, IF_Center = 0;

	IF_Center = IF_Khz;

	if(IF_Center >= 28800)
		IF_set = IF_Center - 28800;
	else
		IF_set = IF_Center;

	FC_INT = (IF_set * 131072) / 225;

	_mt_fe_dmd_set_reg_t2(handle, 0x27, (U8)FC_INT);
	_mt_fe_dmd_set_reg_t2(handle, 0x28, (U8)(FC_INT >> 8));
	_mt_fe_dmd_set_reg_t2(handle, 0x29, (U8)(FC_INT >> 16));


	FC_INT = (IF_set * 32768) / 28800;
	_mt_fe_dmd_set_reg_t2(handle, 0x87, (U8)((FC_INT >> 8) & 0x7f));
	_mt_fe_dmd_set_reg_t2(handle, 0x88, (U8)(FC_INT));

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_bw_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	tmp = 0;
	MT_FE_BANDWIDTH bandwidth = MtFeBandwidth_Undef;

	bandwidth = handle->m_device_ctt2.input_params.demod_bandwidth;
	_mt_fe_dmd_get_reg_t2(handle, 0x36, &tmp);

	switch(bandwidth)
	{
		case MtFeBandwidth_8M:
			_mt_fe_dmd_set_reg_t2(handle, 0x24, 0x00);
			_mt_fe_dmd_set_reg_t2(handle, 0x31, 0xca);
			_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x00);
			_mt_fe_dmd_set_reg_t2(handle, 0x35, 0x8a);
			_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x22);

			_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0x22);
			_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0x66);
			_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0x66);

			_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x28);
			_mt_fe_dmd_set_reg_t2(handle, 0x86, 0xa2);
			_mt_fe_dmd_set_reg_t2(handle, 0x60, 0xd2);
			break;
		case MtFeBandwidth_7M:
			_mt_fe_dmd_set_reg_t2(handle, 0x24, 0x01);
			_mt_fe_dmd_set_reg_t2(handle, 0x31, 0xe6);
			_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x00);
			_mt_fe_dmd_set_reg_t2(handle, 0x35, 0x39);
			_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x22);

			_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0x29);
			_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0x99);
			_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0x9a);

			_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x23);
			_mt_fe_dmd_set_reg_t2(handle, 0x86, 0x8e);
			_mt_fe_dmd_set_reg_t2(handle, 0x60, 0xd0);
			break;
		case MtFeBandwidth_6M:
			_mt_fe_dmd_set_reg_t2(handle, 0x24, 0x02);
			_mt_fe_dmd_set_reg_t2(handle, 0x31, 0x0d);
			_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x01);
			_mt_fe_dmd_set_reg_t2(handle, 0x35, 0xcf);
			_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x33);

			_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0x33);
			_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0x33);
			_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0x33);

			_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x1e);
			_mt_fe_dmd_set_reg_t2(handle, 0x86, 0x7a);
			_mt_fe_dmd_set_reg_t2(handle, 0x60, 0xd0);
			break;
		case MtFeBandwidth_5M:
			_mt_fe_dmd_set_reg_t2(handle, 0x24, 0x03);
			_mt_fe_dmd_set_reg_t2(handle, 0x31, 0xca);
			_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x00);
			_mt_fe_dmd_set_reg_t2(handle, 0x35, 0x2c);
			_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x33);

			_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0x40);
			_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0xa3);
			_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0xd7);

			_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x19);
			_mt_fe_dmd_set_reg_t2(handle, 0x86, 0x66);
			_mt_fe_dmd_set_reg_t2(handle, 0x60, 0xd0);
			break;
		case MtFeBandwidth_1P7M:
			_mt_fe_dmd_set_reg_t2(handle, 0x24, 0x04);
			_mt_fe_dmd_set_reg_t2(handle, 0x31, 0xca);
			_mt_fe_dmd_set_reg_t2(handle, 0x32, 0x00);
			_mt_fe_dmd_set_reg_t2(handle, 0x35, 0xe7);
			_mt_fe_dmd_set_reg_t2(handle, 0x36, 0x73);

			_mt_fe_dmd_set_reg_t2(handle, 0x2f, 0xe9);
			_mt_fe_dmd_set_reg_t2(handle, 0x2e, 0xbf);
			_mt_fe_dmd_set_reg_t2(handle, 0x2d, 0x1f);

			_mt_fe_dmd_set_reg_t2(handle, 0x85, 0x08);
			_mt_fe_dmd_set_reg_t2(handle, 0x86, 0x33);
			break;
		default:
			ret = MtFeErr_Undef;
			break;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_init_dm6k_t2
**
**	DESCRIPTION:
**		initialize DVB-T2 module of M88DM6000
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_init_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	U32 if_khz = 0;
	U8	value = 0;

	_mt_fe_dmd_download_fw_dm6k_t2(handle);
	_mt_fe_dmd_init_reg_dm6k_t2(handle);
	_mt_fe_dmd_set_output_mode_dm6k_t2(handle);
	_mt_fe_dmd_set_bw_dm6k_t2(handle);

	if(handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_MxL603)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t2(handle, if_khz);
	}
	else if(handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t2(handle, if_khz);
		_mt_fe_dmd_get_reg_t2(handle, 0x5f, &value);
		value = (U8)(value | 0x80);
		_mt_fe_dmd_set_reg_t2(handle, 0x5f, value);
	}
	else if(handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t2(handle, if_khz);

		_mt_fe_dmd_get_reg_t2(handle, 0x5f, &value);
		value = (U8)(value | 0x80);
		_mt_fe_dmd_set_reg_t2(handle, 0x5f, value);
	}

	if(handle->m_device_ctt2.tuner_cfg.tuner_init != NULL)
	{
		mt_fe_i2c_repeat_enable_dm6k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_init(handle);
		mt_fe_i2c_repeat_disable_dm6k(handle);
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_hard_reset_dm6k2
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_hard_reset_dm6k_t2(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_dm6k_t2
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_soft_reset_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	_mt_fe_dmd_set_reg_t2(handle, 0x00, 0x01);
	_mt_sleep_dm6k(5);
	_mt_fe_dmd_set_reg_t2(handle, 0x00, 0x00);

	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_connect_dm6k
**
**	DESCRIPTION:
**		connect to a special channel
**
**	IN:
**
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_connect_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	printk("----_mt_fe_dmd_connect_dm6k_t2(), freq = %d, bw = %d\n", handle->m_device_ctt2.input_params.input_freq_kHz, handle->m_device_ctt2.input_params.demod_bandwidth);

	mt_fe_dmd_select_plp_dm6k_t2(handle,handle->m_device_ctt2.input_params.plp_No);

	if (handle->m_device_ctt2.tuner_cfg.tuner_set != NULL)
	{
		if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800)
		{
			_mt_fe_dmd_set_reg_t2(handle, 0x00, 0x01);
		}
		else if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800)
		{
			_mt_fe_dmd_set_reg_t2(handle, 0x00, 0x01);
		}

		mt_fe_i2c_repeat_enable_dm6k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_set(handle, handle->m_device_ctt2.input_params.input_freq_kHz, 0, 0);
		mt_fe_i2c_repeat_disable_dm6k(handle);
	}

	_mt_fe_dmd_set_bw_dm6k_t2(handle);
	_mt_fe_dmd_soft_reset_dm6k_t2(handle);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_select_plp_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 plp_id)
{
	_mt_fe_dmd_set_reg_t2(handle, 0x9d, plp_id);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_get_plp_num_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *plp_num)
{
	MT_FE_LOCK_STATE lock_state = MtFeLockState_Undef;
	U8 tmp = 0;

	_mt_fe_dmd_get_lock_state_dm6k_t2(handle, &lock_state);

	if(lock_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t2(handle, 0x9f, &tmp);
		*plp_num = tmp;
	}
	else
		*plp_num = 0;

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_dm6k_t2
**
**	DESCRIPTION:
**
**	IN:
**		none.
**
**	OUT:
**		*p_state	-	MtFeDmdLockState_Unlocked
**					-	MtFeDmdLockState_Locked
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_get_lock_state_dm6k_t2(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	value = 0, data = 0;

	*p_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_reg_t2(handle, 0x6b, &value);

	if (((value & 0xf0) != 0x00) && ((value & 0xf0) != 0x10) && ((value & 0xf0) != 0xa0))
	{
		if((value & 0xf0) == 0xb0)
		{
			value = 0;
			_mt_fe_dmd_get_reg_t2(handle, 0xe0, &value);
			_mt_fe_dmd_get_reg_t2(handle, 0x99, &data);

			if ((value & 0x01) || (data & 0x80))
				*p_state = MtFeLockState_Locked;
			else
				*p_state = MtFeLockState_Waiting;
		}
	}
	else
		*p_state = MtFeLockState_Unlocked;

	if(*p_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_cell_info_dm6k_t2(handle);
	}
	else
	{
		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
	}

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_get_tps_info_dm6k_t2(MT_FE_DM6K_Device_Handle handle, MT_FE_T2_TPS_INFO *tps_info)
{
	U8	tmp = 0, tmp1 = 0;
	U8	plp_num = 0;
	MT_FE_MOD_MODE			tp_qam = MtFeModMode_Undef;
	MT_FE_FFT				tp_fft = MtFeFFTMode_Undef;
	MT_FE_T2_GUARD_INTERVAL	tp_guard = MtFeGuarInt_19P128;
	MT_FE_T2_PILOT			tp_pp = MtFePilot_PP1;
	MT_FE_CODE_RATE			tp_code = MtFeCodeRate_Undef;

	_mt_fe_dmd_get_reg_t2(handle, 0x94, &tmp);

	if ((tmp & 0x07) == 0x00)
		tp_qam = MtFeModMode_Qpsk;			// BPSK
	else if ((tmp & 0x07) == 0x01)
		tp_qam = MtFeModMode_16Qam;
	else if ((tmp & 0x07) == 0x02)
		tp_qam = MtFeModMode_64Qam;
	else if ((tmp & 0x07) == 0x03)
		tp_qam = MtFeModMode_256Qam;

	if ((tmp & 0x70) == 0x00)
		tp_code = MtFeCodeRate_1_2;			// BPSK
	else if ((tmp & 0x70) == 0x10)
		tp_code = MtFeCodeRate_3_5;
	else if ((tmp & 0x70) == 0x20)
		tp_code = MtFeCodeRate_2_3;
	else if ((tmp & 0x70) == 0x30)
		tp_code = MtFeCodeRate_3_4;
	else if ((tmp & 0x70) == 0x40)
		tp_code = MtFeCodeRate_4_5;
	else if ((tmp & 0x70) == 0x50)
		tp_code = MtFeCodeRate_5_6;

	_mt_fe_dmd_get_reg_t2(handle, 0x9f, &plp_num);

	tmp = 0;
	_mt_fe_dmd_get_reg_t2(handle, 0xa1, &tmp);
	if ((tmp & 0xf0) == 0x00)
		tp_pp = MtFePilot_PP1;
	else if ((tmp & 0xf0) == 0x10)
		tp_pp = MtFePilot_PP2;
	else if ((tmp & 0xf0) == 0x20)
		tp_pp = MtFePilot_PP3;
	else if ((tmp & 0xf0) == 0x30)
		tp_pp = MtFePilot_PP4;
	else if ((tmp & 0xf0) == 0x40)
		tp_pp = MtFePilot_PP5;
	else if ((tmp & 0xf0) == 0x50)
		tp_pp = MtFePilot_PP6;
	else if ((tmp & 0xf0) == 0x60)
		tp_pp = MtFePilot_PP7;
	else if ((tmp & 0xf0) == 0x70)
		tp_pp = MtFePilot_PP8;

	tmp1 = 0;
	_mt_fe_dmd_get_reg_t2(handle, 0x6b, &tmp1);

	if ((tmp1 & 0x07) == 1)
		tp_fft = MtFeFFTMode_1K;
	else if ((tmp1 & 0x07) == 2)
		tp_fft = MtFeFFTMode_2K;
	else if ((tmp1 & 0x07) == 3)
		tp_fft = MtFeFFTMode_4K;
	else if ((tmp1 & 0x07) == 4)
	{
		if ((tmp & 0x08) == 0x08)
			tp_fft = MtFeFFTMode_8E;
		else
			tp_fft = MtFeFFTMode_8K;
	}
	else if ((tmp1 & 0x07) == 5)
	{
		if ((tmp & 0x08) == 0x08)
			tp_fft = MtFeFFTMode_16E;
		else
			tp_fft = MtFeFFTMode_16K;
	}
	else
	{
		if ((tmp & 0x08) == 0x08)
			tp_fft = MtFeFFTMode_32E;
		else
			tp_fft = MtFeFFTMode_32K;
	}

	if ((tmp & 0x07) == 0x00)
		tp_guard = MtFeGuarInt_1P32;
	else if ((tmp & 0x07) == 0x01)
		tp_guard = MtFeGuarInt_1P16;
	else if ((tmp & 0x07) == 0x02)
		tp_guard = MtFeGuarInt_1P8;
	else if ((tmp & 0x07) == 0x03)
		tp_guard = MtFeGuarInt_1P4;
	else if ((tmp & 0x07) == 0x04)
		tp_guard = MtFeGuarInt_1P128;
	else if ((tmp & 0x07) == 0x05)
		tp_guard = MtFeGuarInt_19P128;
	else if ((tmp & 0x07) == 0x06)
		tp_guard = MtFeGuarInt_19P256;


	tps_info->t2_qam = tp_qam;
	tps_info->t2_fft = tp_fft;
	tps_info->t2_guard = tp_guard;
	tps_info->t2_pp = tp_pp;
	tps_info->t2_code = tp_code;

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_get_snr_dm6k_t2
**
**	DESCRIPTION:
**		get the signal SNR
**
**	IN:
**		none.
**
**	OUT:
**		*p_snr
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_get_snr_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *p_snr)
{
	#define MSE_LOOP	20
	#define LOG_2		3010
	typedef struct _MT_FE_SNR
	{
		U8		mse;
		S8		snr;
	} MT_FE_SNR;

	U8	tmp = 0;
	U8	i = 0;
	U32	mse = 0;
	S32	t2_snr = 0;
	U8 	mod = 0;


	mse = 0;

	for (i = 0; i < MSE_LOOP; i ++)
	{
		_mt_fe_dmd_get_reg_t2(handle, 0x97, &tmp);
		mse += tmp;
		_mt_fe_dmd_get_reg_t2(handle, 0x96, &tmp);
		mse += ((tmp & 0x0f) << 8);
	}

	mse /= MSE_LOOP;
	if (mse <= 90)
	{
		t2_snr = (S32)(snr_log10[mse - 1]);
		i = 0;
	}
	else if (mse <= 180)
	{
		if (mse % 2)
			t2_snr = (S32)((snr_log10[(mse / 2) - 1] + snr_log10[(mse / 2)]) / 2);
		else
			t2_snr = (S32)(snr_log10[(mse / 2) - 1]);
		i = 1;
	}
	else
	{
		i = 0;

		do
		{
			mse = mse / 2;

			i ++;
		} while(mse > 90);

		t2_snr = snr_log10[mse - 1];
	}

	mod = 0;
	_mt_fe_dmd_get_reg_t2(handle, 0xa1, &mod);

	if (((mod & 0xf0) == 0x00) || ((mod & 0xf0) == 0x10))
		t2_snr = (S32)((i * LOG_2 + t2_snr - 2500) / 1000);
	else if (((mod & 0xf0) == 0x20) || ((mod & 0xf0) == 0x30))
		t2_snr = (S32)((i * LOG_2 + t2_snr - 4900) / 1000);
	else
		t2_snr = (S32)((i * LOG_2 + t2_snr - 7400) / 1000);

	if(t2_snr < 0)
		t2_snr = 0;

	*p_snr = (U8)t2_snr;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_quality_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *p_percent)
{
/*
	SQI = 0 if C/Nrel < -3 dB
SQI = (C/Nrel +3) * BER_SQI if -3 dB = C/Nrel = 3 dB
SQI = 100 if C/Nrel > 3 dB
where
C/Nrel is DVB-T2 mode depended of the relative C/N of the received signal value in [dB]
and
C/Nrel = C/Nrec - C/NNordigP1
where
C/Nrec is the C/N value expressed in [dB] for the entire received DVB-T2 signal.
C/NNordigP1 is the required C/N value in [dB] for the received PLP in DVB-T2 mode independently
of the pilot pattern in profile 1 defined in Table 3.11.
BER_SQI is calculated with the formula.
BER_SQI = 0 if BER > 10-4
BER_SQI = (100/15) if 10-7 = BER = 10-4
BER_SQI = (100/6) if BER < 10-7
*/
#if 0
	U8 i = 0;
	U8 tmp = 0;
	U32	percent= 0;
	U8 cal_cotinue = 1;
	U32	ldpc_error_cnt0 = 0, ldpc_error_cnt1 = 0;
	U32	ldpc_frame_cnt0 = 0, ldpc_frame_cnt = 0;
	U8	SNR = 0;
	S32	CN_Rel = 0,CN_Perfor = 0;
	MT_FE_T2_TPS_INFO 	q_tps_info;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;


	_mt_fe_dmd_get_lock_state_dm6k_t2(handle, &q_state);
	if(q_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t2(handle, 0xd6, &tmp);
		ldpc_error_cnt0 = tmp << 16;
		_mt_fe_dmd_get_reg_t2(handle, 0xd5, &tmp);
		ldpc_error_cnt0 += tmp << 8;
		_mt_fe_dmd_get_reg_t2(handle, 0xd4, &tmp);
		ldpc_error_cnt0 += tmp;

		_mt_fe_dmd_get_reg_t2(handle, 0xdb, &tmp);
		ldpc_frame_cnt0 = tmp << 16;
		_mt_fe_dmd_get_reg_t2(handle, 0xda, &tmp);
		ldpc_frame_cnt0 += tmp << 8;
		_mt_fe_dmd_get_reg_t2(handle, 0xd9, &tmp);
		ldpc_frame_cnt0 += tmp;
		if ((ldpc_frame_cnt0 >= 0x00ffff00))
		{
			tmp = 0;
			_mt_fe_dmd_get_reg_t2(handle, 0xd0, &tmp);
			tmp = (U8)(tmp | 0x08);
			_mt_fe_dmd_set_reg_t2(handle, 0xd0, tmp);
			ldpc_frame_cnt0 = 0;
			ldpc_error_cnt0 = 0;
		}

		while(cal_cotinue)
		{
			cal_cotinue ++;

			_mt_sleep_dm6k(2);

			_mt_fe_dmd_get_reg_t2(handle, 0xd6, &tmp);
			ldpc_error_cnt1 = tmp << 16;
			_mt_fe_dmd_get_reg_t2(handle, 0xd5, &tmp);
			ldpc_error_cnt1 += tmp << 8;
			_mt_fe_dmd_get_reg_t2(handle, 0xd4, &tmp);
			ldpc_error_cnt1 += tmp;

			_mt_fe_dmd_get_reg_t2(handle, 0xdb, &tmp);
			ldpc_frame_cnt = tmp << 16;
			_mt_fe_dmd_get_reg_t2(handle, 0xda, &tmp);
			ldpc_frame_cnt += tmp << 8;
			_mt_fe_dmd_get_reg_t2(handle, 0xd9, &tmp);
			ldpc_frame_cnt += tmp;

			if((ldpc_frame_cnt - ldpc_frame_cnt0) > 100)
			{
				cal_cotinue = 0;
				if (ldpc_error_cnt1 >= ldpc_error_cnt0)
					percent = 100000 * (ldpc_error_cnt1 - ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
				else
					//percent = 100000 * (0xffffff - ldpc_error_cnt1 - ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
					percent = 100000 * (0xffffff - ldpc_error_cnt1 + ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
			}
			else
			{
				percent = 0;
			}

			if (cal_cotinue > 10)
				break;
		}

		_mt_fe_dmd_get_snr_dm6k_t2(handle,&SNR);
		_mt_fe_dmd_get_tps_info_dm6k_t2(handle, &q_tps_info);
		for(i = 0; i < 60; )
		{
			if((delta_value[i].dpilot == q_tps_info.t2_pp) && (delta_value[i].dfft == q_tps_info.t2_fft))
			{
				CN_Perfor = delta_value[i].delta_bp - 37;
				break;
			}

			i ++;
		}

		if(i == 60)
		{
			CN_Perfor = -100;
		}
		else
		{
			for(i = 0; i < 24; )
			{
				if((cn_nordig[i].smod == q_tps_info.t2_qam) && (cn_nordig[i].scode == q_tps_info.t2_code))
				{
					CN_Perfor = CN_Perfor + cn_nordig[i].cn_perfor * 10;
					break;
				}

				i++;
			}

			if(i == 24)
			{
				CN_Perfor = -100;
			}
		}

		if(SNR < 3)
			SNR = (U8)(SNR + 2);
		else if(SNR < 5)
			SNR = (U8)(SNR + 1);

		CN_Rel = 100 * SNR - CN_Perfor;

		if(CN_Rel < -300)
		{
			*p_percent  = 0;
		}
		else if(CN_Rel > 300)
		{
			*p_percent  = 100;
		}
		else
		{
			if(percent > 3000)
				*p_percent = 0;
			else if (percent < 30)
				*p_percent = (U8)((CN_Rel + 300) / 6 + 10);
			else
				*p_percent =(U8)((CN_Rel + 300) / 15 + 10);
		}
	}
	else
	{
		*p_percent = 0;
	}
#endif

	U8	snr_t2 = 0;
	U32	percent= 0;
	MT_FE_RET ret = MtFeErr_Ok;
	S32	level_rel = 0;
	S8	tuner_strength = 0;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;


	_mt_fe_dmd_get_lock_state_dm6k_t2(handle, &q_state);
	_mt_fe_dmd_get_snr_dm6k_t2(handle, &snr_t2);
	if((q_state == MtFeLockState_Locked) && (snr_t2 > 0))
	{
		percent = snr_t2 * 35 / 10;
		if(percent > 100)
			percent = 100;
	}
	else
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_strength!= NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			ret = handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}

		level_rel = (tuner_strength + 85) * 2;
		if(level_rel > 50)
			level_rel = 50;
		else if(level_rel < 0)
			level_rel = 0;
		percent = (U32)level_rel;
	}

	*p_percent = (U8)percent;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_fec_reset_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	U8 reg_value = 0;

	_mt_fe_dmd_get_reg_t2(handle, 0x6b, &reg_value);

	if ((reg_value & 0xf0) == 0xb0)
	{
		_mt_fe_dmd_set_reg_t2(handle, 0x70, 0x13);
		_mt_fe_dmd_set_reg_t2(handle, 0x71, 0x80);
		_mt_fe_dmd_get_reg_t2(handle, 0x9b, &reg_value);
	}

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_strength_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_T2_TPS_INFO	q_tps_info;
	MT_FE_RET			ret = MtFeErr_Ok;
	S32					level_rel = 0;
	U8					i= 0;
	S8					tuner_strength = 0;

	_mt_fe_dmd_get_tps_info_dm6k_t2(handle, &q_tps_info);

	for(i = 0; i < 24; )
	{
		if((level_nordig[i].smod == q_tps_info.t2_qam) && (level_nordig[i].scode == q_tps_info.t2_code))
		{
			if(handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
			{
				mt_fe_i2c_repeat_enable_dm6k(handle);
				ret = handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
				mt_fe_i2c_repeat_disable_dm6k(handle);
			}
			else
				tuner_strength = -100;

			level_rel = tuner_strength - level_nordig[i].level_ref;

			break;
		}

		i ++;
	}

	if(i == 24)
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			ret = handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}
		else
			tuner_strength = -100;

		level_rel = tuner_strength + 78;
	}

	if(ret != MtFeErr_Ok)
		return ret;

	if(level_rel <= -15)
		*ssi_percent = 0;
	else if(level_rel < 0)
		*ssi_percent = (U8)((level_rel + 15) * 2 / 3);
	else if(level_rel < 20)
		*ssi_percent =(U8)(level_rel * 4 + 10);
	else if(level_rel < 35)
		*ssi_percent = (U8)((level_rel - 20) * 2 / 3 + 90);
	else
		*ssi_percent = 100;

	return ret;
}

MT_FE_RET _mt_fe_dmd_get_quality_nordig_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *p_percent)
{
/*
	SQI = 0 if C/Nrel < -3 dB
SQI = (C/Nrel +3) * BER_SQI if -3 dB = C/Nrel = 3 dB
SQI = 100 if C/Nrel > 3 dB
where
C/Nrel is DVB-T2 mode depended of the relative C/N of the received signal value in [dB]
and
C/Nrel = C/Nrec - C/NNordigP1
where
C/Nrec is the C/N value expressed in [dB] for the entire received DVB-T2 signal.
C/NNordigP1 is the required C/N value in [dB] for the received PLP in DVB-T2 mode independently
of the pilot pattern in profile 1 defined in Table 3.11.
BER_SQI is calculated with the formula.
BER_SQI = 0 if BER > 10-4
BER_SQI = (100/15) if 10-7 = BER = 10-4
BER_SQI = (100/6) if BER < 10-7
*/

	U8 i = 0;
	U8 tmp = 0;
	U32	percent= 0;
	U8 cal_cotinue = 1;
	U32	ldpc_error_cnt0 = 0, ldpc_error_cnt1 = 0;
	U32	ldpc_frame_cnt0 = 0, ldpc_frame_cnt = 0;
	U8	SNR = 0;
	S32	CN_Rel = 0,CN_Perfor = 0;
	MT_FE_T2_TPS_INFO 	q_tps_info;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;


	_mt_fe_dmd_get_lock_state_dm6k_t2(handle, &q_state);
	if(q_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t2(handle, 0xd6, &tmp);
		ldpc_error_cnt0 = tmp << 16;
		_mt_fe_dmd_get_reg_t2(handle, 0xd5, &tmp);
		ldpc_error_cnt0 += tmp << 8;
		_mt_fe_dmd_get_reg_t2(handle, 0xd4, &tmp);
		ldpc_error_cnt0 += tmp;

		_mt_fe_dmd_get_reg_t2(handle, 0xdb, &tmp);
		ldpc_frame_cnt0 = tmp << 16;
		_mt_fe_dmd_get_reg_t2(handle, 0xda, &tmp);
		ldpc_frame_cnt0 += tmp << 8;
		_mt_fe_dmd_get_reg_t2(handle, 0xd9, &tmp);
		ldpc_frame_cnt0 += tmp;
		if ((ldpc_frame_cnt0 >= 0x00ffff00))
		{
			tmp = 0;
			_mt_fe_dmd_get_reg_t2(handle, 0xd0, &tmp);
			tmp = (U8)(tmp | 0x08);
			_mt_fe_dmd_set_reg_t2(handle, 0xd0, tmp);
			ldpc_frame_cnt0 = 0;
			ldpc_error_cnt0 = 0;
		}

		while(cal_cotinue)
		{
			cal_cotinue ++;

			_mt_sleep_dm6k(2);

			_mt_fe_dmd_get_reg_t2(handle, 0xd6, &tmp);
			ldpc_error_cnt1 = tmp << 16;
			_mt_fe_dmd_get_reg_t2(handle, 0xd5, &tmp);
			ldpc_error_cnt1 += tmp << 8;
			_mt_fe_dmd_get_reg_t2(handle, 0xd4, &tmp);
			ldpc_error_cnt1 += tmp;

			_mt_fe_dmd_get_reg_t2(handle, 0xdb, &tmp);
			ldpc_frame_cnt = tmp << 16;
			_mt_fe_dmd_get_reg_t2(handle, 0xda, &tmp);
			ldpc_frame_cnt += tmp << 8;
			_mt_fe_dmd_get_reg_t2(handle, 0xd9, &tmp);
			ldpc_frame_cnt += tmp;

			if((ldpc_frame_cnt - ldpc_frame_cnt0) > 100)
			{
				cal_cotinue = 0;
				if (ldpc_error_cnt1 >= ldpc_error_cnt0)
					percent = 100000 * (ldpc_error_cnt1 - ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
				else
					//percent = 100000 * (0xffffff - ldpc_error_cnt1 - ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
					percent = 100000 * (0xffffff - ldpc_error_cnt1 + ldpc_error_cnt0) / (ldpc_frame_cnt - ldpc_frame_cnt0);
			}
			else
			{
				percent = 0;
			}

			if (cal_cotinue > 10)
				break;
		}

		_mt_fe_dmd_get_snr_dm6k_t2(handle, &SNR);
		_mt_fe_dmd_get_tps_info_dm6k_t2(handle, &q_tps_info);
		for(i= 0; i < 60; )
		{
			if((delta_value[i].dpilot == q_tps_info.t2_pp) && (delta_value[i].dfft == q_tps_info.t2_fft))
			{
				CN_Perfor = delta_value[i].delta_bp - 37;
				break;
			}

			i ++;
		}

		if(i == 60)
		{
			CN_Perfor = -100;
		}
		else
		{
			for(i = 0; i < 24; )
			{
				if((cn_nordig[i].smod == q_tps_info.t2_qam) && (cn_nordig[i].scode == q_tps_info.t2_code))
				{
					CN_Perfor = CN_Perfor + cn_nordig[i].cn_perfor * 10;
					break;
				}

				i ++;
			}

			if(i == 24)
			{
				CN_Perfor = -100;
			}
		}

		if(SNR < 3)
			SNR = (U8)(SNR + 2);
		else if(SNR < 5)
			SNR = (U8)(SNR + 1);

		CN_Rel = 100 * SNR - CN_Perfor;

		if(CN_Rel < -300)
		{
			*p_percent  = 0;
		}
		else if(CN_Rel > 300)
		{
			*p_percent  = 100;
		}
		else
		{
			if(percent > 3000)
				*p_percent = 0;
			else if (percent < 30)
				*p_percent = (U8)((CN_Rel + 300) / 6 + 10);
			else
				*p_percent = (U8)((CN_Rel + 300) / 15 + 10);
		}

		if(*p_percent > 100)
			*p_percent = 100;
	}
	else
	{
		*p_percent = 0;
	}

	if(*p_percent > 100)
		*p_percent = 100;

	return MtFeErr_Ok;
}

#if 0
typedef struct _MT_FE_BER_LOOKUP_DM6K_T2
{
	double dbPreBer;
	double dbPostBer;
} MT_FE_BER_LOOKUP_DM6K_T2;

static MT_FE_BER_LOOKUP_DM6K_T2 mBerTable[5][32] = 
{
	{	// QPSK
		{5.00E-1, 5.00E-2},
		{3.80E-1, 3.90E-2},
		{2.55E-1, 1.79E-3},
		{4.23E-2, 1.21E-7},
		{3.28E-2, 1.21E-7},
		{2.22E-2, 1.21E-7},
		{1.39E-2, 1.21E-7},
		{7.90E-3, 1.21E-7},
		{3.94E-3, 1.21E-7},
		{1.72E-3, 1.21E-7},
		{3.10E-4, 1.21E-7},
		{1.33E-4, 1.21E-7},
		{3.35E-5, 1.21E-7},
		{4.95E-6, 1.21E-7},
		{4.82E-7, 1.21E-7},
		{2.41E-7, 1.21E-7},
		{1.26E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7},
		{1.25E-7, 1.21E-7}
	},
	{	// 16QAM
		{5.00E-1, 5.00E-2},
		{5.00E-1, 5.00E-2},
		{5.00E-1, 5.00E-2},
		{5.00E-1, 5.00E-2},
		{5.00E-1, 5.00E-2},
		{5.00E-1, 5.00E-2},
		{3.80E-1, 3.90E-2},
		{5.54E-2, 1.21E-7},
		{4.23E-2, 1.21E-7},
		{3.28E-2, 1.21E-7},
		{2.42E-2, 1.21E-7},
		{1.65E-2, 1.21E-7},
		{1.04E-2, 1.21E-7},
		{5.90E-3, 1.21E-7},
		{3.10E-3, 1.21E-7},
		{1.33E-3, 1.21E-7},
		{4.69E-4, 1.21E-7},
		{1.45E-4, 1.21E-7},
		{4.37E-5, 1.21E-7},
		{1.65E-5, 1.21E-7},
		{1.23E-5, 1.21E-7},
		{4.11E-6, 1.21E-7},
		{6.03E-6, 1.21E-7},
		{2.44E-6, 1.21E-7},
		{1.92E-6, 1.21E-7},
		{2.40E-6, 1.21E-7},
		{2.30E-6, 1.21E-7},
		{1.41E-6, 1.21E-7},
		{1.92E-6, 1.21E-7},
		{7.23E-7, 1.21E-7},
		{1.23E-7, 1.21E-7},
		{1.23E-7, 1.21E-7}
	},
	{	// 64QAM
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{3.80E-1, 3.90E-2},
		{7.06E-2, 1.21E-7},
		{6.14E-2, 1.21E-7},
		{5.22E-2, 1.21E-7},
		{4.33E-2, 1.21E-7},
		{3.54E-2, 1.21E-7},
		{2.77E-2, 1.21E-7},
		{2.07E-2, 1.21E-7},
		{1.46E-2, 1.21E-7},
		{9.34E-3, 1.21E-7},
		{5.53E-3, 1.21E-7},
		{2.93E-3, 1.21E-7},
		{1.49E-3, 1.21E-7},
		{6.20E-4, 1.21E-7},
		{2.50E-4, 1.21E-7},
		{7.41E-5, 1.21E-7},
		{4.41E-5, 1.21E-7},
		{4.89E-5, 1.21E-7},
		{4.22E-5, 1.21E-7},
		{2.61E-5, 1.21E-7},
		{2.15E-5, 1.21E-7},
		{1.66E-5, 1.21E-7}
	},
	{	// 256QAM_2/3
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{3.80E-1, 3.90E-2},
		{6.55E-2, 1.21E-7},
		{5.52E-2, 1.21E-7},
		{4.85E-2, 1.21E-7},
		{3.52E-2, 1.21E-7},
		{2.67E-2, 1.21E-7},
		{1.93E-2, 1.21E-7},
		{1.43E-2, 1.21E-7},
		{8.68E-3, 1.21E-7},
		{5.42E-3, 1.21E-7},
		{2.58E-3, 1.21E-7},
		{1.76E-3, 1.21E-7},
		{6.18E-4, 1.21E-7},
		{1.54E-4, 1.21E-7}
	},
	{	// 256QAM_5/6
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{5.00E-1, 5.00E-1},
		{3.80E-1, 3.90E-2},
		{2.46E-2, 1.21E-7},
		{2.23E-2, 1.21E-7},
		{1.56E-2, 1.21E-7},
		{9.89E-3, 1.21E-7},
		{5.99E-3, 1.21E-7},
		{3.22E-3, 1.21E-7},
		{1.59E-3, 1.21E-7},
		{7.54E-4, 1.21E-7},
		{3.35E-4, 1.21E-7}
	}
};

MT_FE_RET _mt_fe_dmd_get_per_dm6k_t2(MT_FE_DM6K_Device_Handle handle, double *pDbPreBer, double *pDbPostBer)
{
#define MSE_LOOP_PER	2
#define LOG_2_PER		3010
	typedef struct _MT_FE_SNR
	{
		U8		mse;
		S8		snr;
	} MT_FE_SNR;

	U8	tmp = 0;
	U8	i = 0;
	U32	mse = 0;
	double	t2_snr = 0;
	U8 	mod = 0;
	U8	iBerGroupIndex = 0;
	int	iBerIndex = 0;

	MT_FE_LOCK_STATE mLockState;
	MT_FE_T2_TPS_INFO mT2Info;


	_mt_fe_dmd_get_lock_state_dm6k_t2(handle, &mLockState);

	if(mLockState != MtFeLockState_Locked)
	{
		*pDbPostBer = 0.05;
		*pDbPreBer = 0.5;

		return MtFeErr_UnLock;
	}

	mse = 0;

	for (i = 0; i < MSE_LOOP_PER; i ++)
	{
		_mt_fe_dmd_get_reg_t2(handle, 0x97, &tmp);
		mse += tmp;
		_mt_fe_dmd_get_reg_t2(handle, 0x96, &tmp);
		mse += ((tmp & 0x0f) << 8);
	}

	mse /= MSE_LOOP_PER;
	if (mse <= 90)
	{
		t2_snr = (S32)(snr_log10[mse - 1]);
		i = 0;
	}
	else if (mse <= 180)
	{
		if (mse % 2)
			t2_snr = (S32)((snr_log10[(mse / 2) - 1] + snr_log10[(mse / 2)]) / 2);
		else
			t2_snr = (S32)(snr_log10[(mse / 2) - 1]);
		i = 1;
	}
	else
	{
		i = 0;

		do
		{
			mse = mse / 2;

			i ++;
		} while(mse > 90);

		t2_snr = snr_log10[mse - 1];
	}

	mod = 0;
	_mt_fe_dmd_get_reg_t2(handle, 0xa1, &mod);
	if (((mod & 0xf0) == 0x00) || ((mod & 0xf0) == 0x10))
		t2_snr = (i * LOG_2_PER + t2_snr - 2500) / 1000.0;
	else if (((mod & 0xf0) == 0x20) || ((mod & 0xf0) == 0x30))
		t2_snr = (i * LOG_2_PER + t2_snr - 4900) / 1000.0;
	else
		t2_snr = (i * LOG_2_PER + t2_snr - 7400) / 1000.0;

	if(t2_snr < 0)
		t2_snr = 0;

	if(t2_snr > 31)
		t2_snr = 31;

	iBerIndex = (int)t2_snr;

	_mt_fe_dmd_get_tps_info_dm6k_t2(handle, &mT2Info);
	switch(mT2Info.t2_qam)
	{
		case MtFeModMode_Qpsk:		iBerGroupIndex = 0;			break;
		case MtFeModMode_16Qam:		iBerGroupIndex = 1;			break;
		case MtFeModMode_64Qam:		iBerGroupIndex = 2;			break;
		case MtFeModMode_256Qam:	iBerGroupIndex = 3;			break;
		default:					iBerGroupIndex = 4;			break;
	}

	if(iBerIndex < 31)
	{
		*pDbPreBer = mBerTable[iBerGroupIndex][iBerIndex].dbPreBer + (t2_snr - iBerIndex) * (mBerTable[iBerGroupIndex][iBerIndex + 1].dbPreBer - mBerTable[iBerGroupIndex][iBerIndex].dbPreBer);
		*pDbPostBer = mBerTable[iBerGroupIndex][iBerIndex].dbPostBer + (t2_snr - iBerIndex) * (mBerTable[iBerGroupIndex][iBerIndex + 1].dbPostBer - mBerTable[iBerGroupIndex][iBerIndex].dbPostBer);
	}
	else
	{
		*pDbPreBer = mBerTable[iBerGroupIndex][iBerIndex].dbPreBer;
		*pDbPostBer = mBerTable[iBerGroupIndex][iBerIndex].dbPostBer;
	}

	return MtFeErr_Ok;
}
#endif

MT_FE_RET _mt_fe_dmd_get_cell_info_dm6k_t2(MT_FE_DM6K_Device_Handle handle)
{
	U8 tmp1, tmp2;
	MT_BOOL bOk = FALSE;

	if(handle->m_device_ctt2.demod_type != MtFeType_DVBT2)
	{
		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
		return MtFeErr_NoMatch;
	}

	bOk = (handle->m_device_ctt2.cell_info.bHighByteOk && handle->m_device_ctt2.cell_info.bLowByteOk) ? TRUE : FALSE;

	if(bOk)
	{
		return MtFeErr_Ok;
	}

	//In DVBT2  regs  ,   Set reg0x70=0x15  , read  reg0x71  to get cell_id[15:8].
	//                    Set reg0x70=0x14  , read  reg0x71  to get cell_id[7:0].

	_mt_fe_dmd_set_reg_t2(handle, 0x70, 0x15);
	_mt_fe_dmd_get_reg_t2(handle, 0x71, &tmp1);
	_mt_fe_dmd_set_reg_t2(handle, 0x70, 0x14);
	_mt_fe_dmd_get_reg_t2(handle, 0x71, &tmp2);

	handle->m_device_ctt2.cell_info.usCellId = (tmp1 << 8) + tmp2;
	handle->m_device_ctt2.cell_info.bHighByteOk = TRUE;
	handle->m_device_ctt2.cell_info.bLowByteOk = TRUE;

	return MtFeErr_Ok;
}

