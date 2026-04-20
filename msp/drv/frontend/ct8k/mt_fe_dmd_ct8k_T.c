/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2019                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_ct8k_T.c
*
* Current version:	00.06
*
* Description:		Symphony2 IC Driver For DVBT Mode.
*
* Log:	Description		Version		Date			Author
---------------------------------------------------------------------
*		Create			00.01		2018.06.18		BJ.Wang
*		Modify			00.01		2018.06.26		BJ.Wang
*		Modify			00.02		2018.12.06		YZ.Huang
*		Modify			00.03		2018.12.13		YZ.Huang
*		Modify			00.04		2018.12.27		YZ.Huang
*		Modify			00.05		2019.07.04		YZ.Huang
*		Modify			00.06		2019.11.08		YZ.Huang
******************************************************************************************************/
#include "mt_fe_dmd_ct8k_T.h"
#include "mt_fe_i2c_ct8k.h"
#include "mt_fe_dmd_fw_ct8k_T.h"

static U8	soft_reset_flag = 0;


static const U8 reg_tbl_init_def[][2] = 
{
	{0x07, 0x03}, {0x04, 0x02}, {0x20, 0x02}, {0x29, 0xe4}, {0x30, 0x3f},
	{0x31, 0xcc}, {0x32, 0xcc}, {0x33, 0xff}, {0x34, 0x00}, {0x35, 0x0f},
	{0x36, 0x0f}, {0x37, 0x10}, {0x38, 0xff}, {0x39, 0x10}, {0x3a, 0xff},
	{0x3b, 0xd0}, {0x3c, 0xe0}, {0x3d, 0x70}, {0x3e, 0x70}, {0x3f, 0x00},
	{0x48, 0x45}, {0x49, 0xb5}, {0x80, 0x08}, {0x81, 0x80}, {0x74, 0x0a},
	{0xac, 0x36}, {0xb2, 0x00}, {0xdd, 0x2f}, {0x53, 0x07}, {0xa8, 0x01},

	//* for new FPGA *//
	{0x54, 0x08}, {0x62, 0x14},	{0x63, 0x14}, {0x99, 0x0f},	{0x50, 0x01},	// for sample clock
	{0x87, 0x33}, {0x8a, 0x74},	{0x29, 0xe4}, {0x84, 0x00},	{0x85, 0x55},
	{0x88, 0x8c}, {0x8b, 0x59},	{0x83, 0x43}, {0x8a, 0x84}, {0xc1, 0x00},
	{0x70, 0x01}
};


static const U8 reg_tbl_init_def_a1[][2] = 
{
	{0x07, 0x03}, {0x04, 0x02}, {0x20, 0x02}, {0x29, 0xe4}, {0x30, 0x3f},
	{0x31, 0xcc}, {0x32, 0xcc}, {0x33, 0xff}, {0x34, 0x00}, {0x35, 0x0f},
	{0x36, 0x0f}, {0x37, 0x10}, {0x38, 0xff}, {0x39, 0x10}, {0x3a, 0xff},
	{0x3b, 0xd0}, {0x3c, 0xe0}, {0x3d, 0x70}, {0x3e, 0x70}, {0x3f, 0x00},
	{0x48, 0x45}, {0x49, 0xb5}, {0x80, 0x08}, {0x81, 0x80}, {0x70, 0x02},
	{0x74, 0x07}, {0xac, 0x36}, {0xb2, 0x00}, {0xdd, 0x2f}, {0x53, 0x07},
	{0xa8, 0x01},

	//* for new FPGA *//
	{0x54, 0x08}, {0x62, 0x14},	{0x63, 0x14}, {0x99, 0x0f},	{0x50, 0x01},	// for sample clock
	{0x87, 0x33}, {0x8a, 0x74},	{0x29, 0xe4}, {0x84, 0x00},	{0x85, 0x55},
	{0x88, 0x8c}, {0x8b, 0x59},	{0x83, 0x61}, {0x8a, 0x84}, {0xc1, 0x00},
};


typedef struct _MT_FE_CN_NORDIG
{
	MT_FE_MOD_MODE		smod;
	MT_FE_CODE_RATE		scode;
	U8					cn_perfor;
} MT_FE_CN_NORDIG;

typedef struct _MT_FE_DELTA
{
	MT_FE_FFT			dfft;
	U8					delta_bp;
} MT_FE_DELTA;

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
	{MtFeFFTMode_1K, 34},	{MtFeFFTMode_2K, 35},
	{MtFeFFTMode_4K, 39},	{MtFeFFTMode_8K, 41},
	{MtFeFFTMode_8E, 41},	{MtFeFFTMode_16K, 41},
	{MtFeFFTMode_16E, 42},

	{MtFeFFTMode_1K, 32},	{MtFeFFTMode_2K, 33},
	{MtFeFFTMode_4K, 37},	{MtFeFFTMode_8K, 39},
	{MtFeFFTMode_8E, 41},	{MtFeFFTMode_16K, 38},
	{MtFeFFTMode_16E, 38},	{MtFeFFTMode_32K, 37},
	{MtFeFFTMode_32E, 37},

	{MtFeFFTMode_1K, 44},	{MtFeFFTMode_2K, 43},
	{MtFeFFTMode_4K, 47},	{MtFeFFTMode_8K, 49},
	{MtFeFFTMode_8E, 50},	{MtFeFFTMode_16K, 49},
	{MtFeFFTMode_16E, 49},	{MtFeFFTMode_32K, 48},
	{MtFeFFTMode_32E, 48},

	{MtFeFFTMode_1K, 42},	{MtFeFFTMode_2K, 42},
	{MtFeFFTMode_4K, 45},	{MtFeFFTMode_8K, 48},
	{MtFeFFTMode_8E, 48},	{MtFeFFTMode_16K, 47},
	{MtFeFFTMode_16E, 47},	{MtFeFFTMode_32K, 45},
	{MtFeFFTMode_32E, 45},

	{MtFeFFTMode_1K, 48},	{MtFeFFTMode_2K, 47},
	{MtFeFFTMode_4K, 51},	{MtFeFFTMode_8K, 53},
	{MtFeFFTMode_8E, 52},	{MtFeFFTMode_16K, 52},
	{MtFeFFTMode_16E, 52},

	{MtFeFFTMode_16K, 49},	{MtFeFFTMode_16E, 49},
	{MtFeFFTMode_32K, 48},	{MtFeFFTMode_32E, 48},

	{MtFeFFTMode_1K, 29},	{MtFeFFTMode_2K, 29},
	{MtFeFFTMode_4K, 34},	{MtFeFFTMode_8K, 37},
	{MtFeFFTMode_8E, 39},	{MtFeFFTMode_16K, 33},
	{MtFeFFTMode_16E, 34},	{MtFeFFTMode_32K, 33},
	{MtFeFFTMode_32E, 33},

	{MtFeFFTMode_8K, 37},	{MtFeFFTMode_8E, 38},
	{MtFeFFTMode_16K, 35},	{MtFeFFTMode_16E, 35},
	{MtFeFFTMode_32K, 35},	{MtFeFFTMode_32E, 35}
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



/*
** Function: mt_fe_dmd_get_driver_version_t_ct8k
**
**
** Description:	This function is used to get the driver version for dvbt
**				mode.
**
** Inputs:   None
**
** Outputs:
**
**	  Parameter			Type		Description
**	----------------------------------------------------
**	 p_version			U8*		version number pointer
**
*/
MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_t(U8* p_version)
{
	*p_version = 6;		/* driver version number */

	return MtFeErr_Ok;
}

static MT_FE_RET _mt_fe_dmd_init_reg_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret;
	S32	i = 0, size = 0;

	if(handle->chip_version == 0x9000)	// Sym2 A0
	{
		size = sizeof(reg_tbl_init_def) / 2;

		for (i = 0; i < size; i ++)
		{
			ret = _mt_fe_dmd_set_reg_t_ct8k(handle, reg_tbl_init_def[i][0], reg_tbl_init_def[i][1]);

			if (ret != MtFeErr_Ok)
				return ret;
		}
	}
	else	// 0x9001: Sym2 A1
	{
		size = sizeof(reg_tbl_init_def_a1) / 2;

		for (i = 0; i < size; i ++)
		{
			ret = _mt_fe_dmd_set_reg_t_ct8k(handle, reg_tbl_init_def_a1[i][0], reg_tbl_init_def_a1[i][1]);

			if (ret != MtFeErr_Ok)
				return ret;
		}
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_download_fw_ct8k_t
**
**	DESCRIPTION:
**			load firmware for dvbt mode
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_download_fw_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U32 i = 0, firm_size = 0, firm_last_size = 0, w_size = 0;
	U8  tmp[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	const U8* p_fw = m88ct8k_t_fm;

	if((handle->m_device_ctt2.demod_type != MtFeType_DVBT) && 
	   (handle->m_device_ctt2.demod_type != MtFeType_DVBT2) && 
	   (handle->m_device_ctt2.demod_type != MtFeType_DVBT_T2))
	{
		return MtFeErr_NoMatch;
	}

	ret = _mt_fe_dmd_get_reg_t_ct8k(handle, 0x89, &tmp[0]);
	if ((ret != MtFeErr_Ok) || (tmp[0] != m88ct8k_fm_check_t))
	{
		handle->m_device_ctt2.mcu_status &= ~0x01;
	}
	else
	{
		handle->m_device_ctt2.mcu_status |= 0x01;
	}

	if ((handle->m_device_ctt2.mcu_status & 0x01) != 0x01)
	{
		firm_size = sizeof(m88ct8k_t_fm) / 16;
		firm_last_size = sizeof(m88ct8k_t_fm) % 16;

		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xb2, 0x01);
		_mt_delay_ct8k(1);

		tmp[0] = 0xb0;
		for (i = 0; i < firm_size; i ++)
		{
			for(w_size = 1; w_size < 17; w_size ++)
			{
				tmp[w_size] = *(p_fw ++);
			}
			ret = _mt_fe_dmd_write_ct8k(handle->m_device_ctt2.dmd_dev_addr, tmp, 17);
			if (ret != MtFeErr_Ok)
				break;
		}

		for(w_size = 1; w_size < (firm_last_size + 1); w_size ++)
		{
			tmp[w_size] = *(p_fw ++);
		}
		ret = _mt_fe_dmd_write_ct8k(handle->m_device_ctt2.dmd_dev_addr, tmp, (U16)(firm_last_size + 1));

		ret =_mt_fe_dmd_set_reg_t_ct8k(handle, 0xb2, 0x00);

		ret = _mt_fe_dmd_get_reg_t_ct8k(handle, 0x89, &tmp[0]);
		if ((ret != MtFeErr_Ok) || (tmp[0] != m88ct8k_fm_check_t))
		{
			handle->m_device_ctt2.mcu_status &= ~0x01;

			mt_fe_print(("MT: _mt_fe_dmd_download_fw_ct8k_t FAILED! Ver no = 0x%02x,  [ret = %d]\n", tmp[0], ret));

			return MtFeErr_I2cErr;
		}

		handle->m_device_ctt2.mcu_status |= 0x01;
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_set_IF_ct8k_t
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
MT_FE_RET _mt_fe_dmd_set_IF_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 IF_Khz)
{
	U32 IF_set = 0, FC_INT = 0, IF_Center = 0;

	IF_Center = IF_Khz;

	if(IF_Center >= 28800)
		IF_set = IF_Center - 28800;
	else
		IF_set = IF_Center;

	FC_INT = IF_set * 9321;

	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf0, (U8)FC_INT);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf1, (U8)(FC_INT >> 8));
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf2, (U8)(FC_INT >> 16));
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf3, (U8)((FC_INT >> 24) & 0x0f));

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_ct8k_t
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
MT_FE_RET _mt_fe_dmd_set_output_mode_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U8	tmp1 = 0, tmp2 = 0;
	MT_FE_TS_OUT_MODE	mode = MtFeTsOutMode_Parallel;

	mode = handle->m_device_ctt2.ts_out_mode;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xaa, &tmp1);
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xab, &tmp2);

	if (mode == MtFeTsOutMode_Serial)
	{
		tmp1 |= 0x04;
		tmp2 &= ~0x40;
	}
	else if(mode == MtFeTsOutMode_Parallel)
	{
		tmp1 &= ~0x04;
		tmp2 &= ~0x40;
	}
	else if(mode == MtFeTsOutMode_Common)
	{
		tmp2 |= 0x40;
	}
	else	// undef, return undef error code
	{
		//tmp2 |= 0x40;
		return MtFeErr_Undef;
	}

	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xaa, tmp1);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xab, tmp2);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_bw_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	MT_FE_BANDWIDTH bw = MtFeBandwidth_8M;
	U8	tmp = 0;

	bw = handle->m_device_ctt2.input_params.demod_bandwidth;
	switch(bw)
	{
		case MtFeBandwidth_8M:
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf4, 0x51);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf5, 0x14);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf6, 0x05);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf7, 0x9a);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf8, 0x99);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf9, 0x99);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xfa, 0xc9);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc0, 0x54);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc1, 0x00);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc2, 0x41);

			_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb3, &tmp);
			tmp = (U8)(tmp & 0x80);

			if (tmp == 0x80)
			{
				_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, 0x80);
				_mt_fe_dmd_set_reg_t_ct8k(handle, 0x97, 0x60);
			}
			else
			{
				_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, 0x84);
				_mt_fe_dmd_set_reg_t_ct8k(handle, 0x97, 0x87);
			}
			break;

		case MtFeBandwidth_7M:
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf4, 0xc7);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf5, 0x71);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf6, 0x04);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf7, 0x66);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf8, 0x66);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf9, 0x66);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xfa, 0xe6);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc0, 0x55);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc1, 0x00);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xc2, 0x81);
			break;

		case MtFeBandwidth_6M:
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf4, 0x3d);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf5, 0xcf);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf6, 0x13);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf7, 0xcd);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf8, 0xcc);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xf9, 0xcc);
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xfa, 0x0c);
			break;

		default:	//MtFeBandwidth_Undef Reserved

			break;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_init_ct8k_t
**
**	DESCRIPTION:
**		initialize DVB-T mode of Symphony2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_init_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U32	if_khz = 0;

	_mt_fe_dmd_init_reg_ct8k_t(handle);

	_mt_fe_dmd_set_reg_t_ct8k(handle, 0x19, 0x32);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0x1d, 0xb8);

	_mt_fe_dmd_download_fw_ct8k_t(handle);
	_mt_fe_dmd_set_output_mode_ct8k_t(handle);
	_mt_fe_dmd_set_bw_ct8k_t(handle);

	if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_MxL603)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_ct8k_t(handle, if_khz);
	}
	else if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_ct8k_t(handle, if_khz);
	}
	else if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_ct8k_t(handle, if_khz);
	}

	if(handle->m_device_ctt2.tuner_cfg.tuner_init != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_init(handle);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_hard_reset_ct8k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_hard_reset_ct8k_t(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_ct8k_t
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_soft_reset_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	//_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x52);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x42);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xdd, 0x2f);

	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xb2, 0x01);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0x00, 0x01);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0x00, 0x00);
	_mt_delay_ct8k(5);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xb2, 0x00);

	soft_reset_flag = 1;

	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_demod_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U8	reg_8a = 0;

	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	mt_fe_dmd_set_hierarchy_ct8k_t(handle, handle->m_device_ctt2.input_params.plp_No);

	_mt_fe_dmd_set_bw_ct8k_t(handle);

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x8a, &reg_8a);
	if(reg_8a & 0x40)
	{
		reg_8a = (U8)((reg_8a & 0x0f) | 0x80);
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, reg_8a);
	}
	_mt_fe_dmd_soft_reset_ct8k_t(handle);

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_ct8k
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
MT_FE_RET _mt_fe_dmd_connect_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U8	reg_8a = 0;

	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	mt_fe_dmd_set_hierarchy_ct8k_t(handle, handle->m_device_ctt2.input_params.plp_No);

	if (handle->m_device_ctt2.tuner_cfg.tuner_set != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_set(handle, handle->m_device_ctt2.input_params.input_freq_kHz, 0, 0);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	_mt_fe_dmd_set_bw_ct8k_t(handle);

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x8a, &reg_8a);
	if(reg_8a & 0x40)
	{
		reg_8a = (U8)((reg_8a & 0x0f) | 0x80);
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, reg_8a);
	}
	_mt_fe_dmd_soft_reset_ct8k_t(handle);

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_ct8k_t
**
**	DESCRIPTION:
**
**	IN:
**		none.
**
**	OUT:
**		*p_state	-	MtFeDmdLockState_Unlocked
**					-MtFeDmdLockState_Locked
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_get_lock_state_ct8k_t(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	value = 0;

	*p_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb7, &value);

	if(soft_reset_flag && ((value & 0x80) == 0))
	{
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb3, &value);	// DEMOD DEBUG Output
		if((value & 0x7f) != 0x0c)
		{
			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x42);
		}
		if ((value != 0xf1) && (value != 0xf2) && (value != 0xf3) && (value != 0xf4) && (value != 0xf5)
		 && (value != 0xf6) && (value != 0xf7) && (value != 0xf8) && (value != 0xf9) && (value != 0xf0)
		 && (value != 0xfb) && (value != 0x0c) && (value != 0x8c) && (value != 0x0b))
		{
			if((0x07 < value) && (value < 0x0d))
				*p_state = MtFeLockState_Waiting;
			else
				*p_state = MtFeLockState_Unlocked;

			return MtFeErr_Ok;
		}
		else
		{
			soft_reset_flag = 0;
		}
	}

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb3, &value);
	if(((value & 0x7f) == 0x0c))
	{
		*p_state = MtFeLockState_Locked;
	}
	else
	{
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x42);
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xac, &value);
		if ((value & 0x80) == 0)
		{
			mt_fe_print(("MT:	FEC Waiting!!\n"));
			*p_state = MtFeLockState_Waiting;
		}
		else
		{
			*p_state = MtFeLockState_Locked;
		}
	}

	if(*p_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_cell_info_ct8k_t(handle);
	}
	else
	{
		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_set_hierarchy_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 hi_id)
{
	U8 tmp = 0;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xbe, &tmp);
	tmp = (U8)((tmp & 0x7f) | (hi_id << 7));
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xbe, tmp);

	handle->m_device_ctt2.input_params.plp_No = hi_id;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_get_hierarchy_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *hi_num)
{
	MT_FE_LOCK_STATE lock_state = MtFeLockState_Undef;
	U8 tmp = 0;

	_mt_fe_dmd_get_lock_state_ct8k_t(handle, &lock_state);

	if(lock_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x91, &tmp);
		if (tmp & 0x70)
			*hi_num =  2;
		else
			*hi_num =  1;
	}
	else
		*hi_num =  0;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		_mt_fe_dmd_get_snr_ct8k_t
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
MT_FE_RET _mt_fe_dmd_get_snr_ct8k_t(MT_FE_CT8K_Device_Handle handle, U16 *p_snr)
{
#define MSE_LOOP_T	5
#define LOG_2_T		3010

	U8	tmp = 0;
	U8	i = 0;
	U32	mse = 0;
	U32	snr = 0;

#if 1
	U16	mse_tmp[MSE_LOOP_T], d4_total = 0;
	U8	d4_tmp[MSE_LOOP_T], d4_max = 0, d4_min = 0xFF, d4_avg = 0, d4_delta = 0;
	U8	bVaried = 0;

	for (i = 0; i < MSE_LOOP_T; i ++)
	{
		mse_tmp[i] = 0;

		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xd4, &tmp);
		d4_tmp[i] = tmp;
		mse_tmp[i] = (tmp << 8);

		if(d4_max < tmp)		d4_max = tmp;
		if(d4_min > tmp)		d4_min = tmp;
		d4_total += tmp;
		d4_avg = d4_total / (i + 1);


		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xd3, &tmp);
		mse_tmp[i] += tmp;
	}

	d4_delta = d4_max - d4_min;

	if(d4_delta > 0x10)
		bVaried = 1;

	for(i = 0; i < MSE_LOOP_T; i ++)
	{
		if(bVaried == 1)
			mse_tmp[i] &= 0xFF;

		mse += mse_tmp[i];
	}

	mse = (U32)(mse / MSE_LOOP_T);

#else
	for (i = 0; i < MSE_LOOP_T; i ++)
	{
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xd4, &tmp);
		mse = mse + (tmp << 8);
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0xd3, &tmp);
		mse = mse + tmp;
	}
	mse = (U32)(mse / MSE_LOOP_T);
#endif

	mse = mse / 32;

	if (mse <= 90)
	{
		snr = snr_log10[mse - 1];
		i = 0;
	}
	else if (mse <= 180)
	{
		if (mse % 2)
			snr = (snr_log10[(mse / 2) - 1] + snr_log10[(mse / 2)]) / 2;
		else
			snr = snr_log10[(mse / 2) - 1];
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

		snr = snr_log10[mse - 1];
	}

	snr = (i * LOG_2_T + snr);

	*p_snr = snr / 100;

	*p_snr += 10;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_get_ber_ct8k_t
**
**	DESCRIPTION:
**		get the byte error ratio
**
**	IN:
**		none.
**
**	OUT:
**		*p_err_cnt	-	error bit counter
**		*p_pkt_cnt	-	totale bit counter
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_ber_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt)
{
#define PKT_CNT_MODE		3		/*	can be 0, 1, 2 or 3	*/
#define PKT_CNT_MAX			(1 << (PKT_CNT_MODE * 2 + 12 + 3))

	static U16	last_ber_cnt = 0;
	U8			tmp, tmp1;

	*p_err_cnt = 0;
	*p_total_cnt = PKT_CNT_MAX;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa8, &tmp);
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb3, &tmp1);
	if (((tmp & 0x80) == 0) || ((tmp1 & 0x7f) != 0x0c))
	{
		last_ber_cnt = 0;
		*p_err_cnt   = 0;

		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xdd, 0x2f);
		//_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x52);
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x42);
		return MtFeErr_Fail;
	}

	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xdd, 0xff);
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xad, &tmp);
	if ((tmp & 0x40) == 0x40)///frist read ,set fail
	{
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x00 | PKT_CNT_MODE);
		_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x80 | PKT_CNT_MODE);
		return MtFeErr_Fail;
	}
	else
	{
		if ((tmp & 0x80) == 0x00)
		{
			_mt_fe_dmd_get_reg_t_ct8k(handle, 0xae, &tmp);
			last_ber_cnt = tmp;
			_mt_fe_dmd_get_reg_t_ct8k(handle, 0xaf, &tmp);
			last_ber_cnt += (U16)((tmp << 8));

			_mt_fe_dmd_set_reg_t_ct8k(handle, 0xad, 0x80 | PKT_CNT_MODE);//0xb0 | PKT_CNT_MODE);
		}
		else
			return MtFeErr_Fail;
	}

	*p_err_cnt = last_ber_cnt;


	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_get_quality_ct8k_t
**
**	DESCRIPTION:
**		get the signal quality
**
**	IN:
**		none.
**
**	OUT:
**		*quality
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_get_quality_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	U16	snr_t = 0;
	U32	percent = 0;
	MT_FE_RET	ret = MtFeErr_Ok;
	S32	level_rel = 0;
	S8	tuner_strength = 0;
	MT_FE_LOCK_STATE	q_state = MtFeLockState_Undef;


	_mt_fe_dmd_get_lock_state_ct8k_t(handle, &q_state);
	_mt_fe_dmd_get_snr_ct8k_t(handle, &snr_t);
	if((q_state == MtFeLockState_Locked) && (snr_t > 0))
	{
		percent = snr_t * 35 / 100;
		if(percent > 100)
			percent = 100;
	}
	else
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_strength!= NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			ret = handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
			mt_fe_i2c_repeat_disable_ct8k(handle);
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


MT_FE_RET _mt_fe_dmd_get_strength_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent)
{
	S8 tuner_strength = 0;

	if(handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}
	else
		tuner_strength = 0;

	if(tuner_strength >= -40)
		*ssi_percent = 100;
	else if ((tuner_strength < -40) && (tuner_strength >= -50))
		*ssi_percent = (U8)(95 + (50 + tuner_strength) / 2);
	else if ((tuner_strength < -50) && (tuner_strength >= -60))
		*ssi_percent = (U8)(80 + 3 * (60 + tuner_strength) / 2);
	else if ((tuner_strength < -60) && (tuner_strength >= -80))
		*ssi_percent = (U8)(20 + 3 * (80 + tuner_strength));
	else if ((tuner_strength < -80) && (tuner_strength >= -100))
		*ssi_percent = (U8)(100 + tuner_strength);
	else
		*ssi_percent = 0;

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_get_quality_nordig_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *quality)
{
	U16 snr_t = 0;

	S16	quality_value = 0;

	MT_FE_T_TPS_INFO tps_info_t;

	_mt_fe_dmd_get_tps_info_ct8k_t(handle, &tps_info_t);

	_mt_fe_dmd_get_snr_ct8k_t(handle, &snr_t);


	// * 12.5
	snr_t *= 5;
	snr_t /= 4;

	if((tps_info_t.t_qam == MtFeModMode_64Qam) && (tps_info_t.t_code == MtFeCodeRate_3_4))
	{
		quality_value = snr_t - 195;
	}
	else if(tps_info_t.t_qam == MtFeModMode_16Qam)
	{
		quality_value = snr_t - 100;
	}
	else if(tps_info_t.t_qam == MtFeModMode_Qpsk)
	{
		quality_value = snr_t - 50;
	}
	else
	{
		quality_value = snr_t - 175;
	}

	if(quality_value > 100)
		*quality = 100;
	else if(quality_value < 0)
		*quality = 0;
	else
		*quality = (U8)quality_value;

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_clear_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U8 tmp = 0;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa8, &tmp);
	tmp = (U8)(tmp & 0xfe);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xa8, tmp);

	_mt_delay_ct8k(1);

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa8, &tmp);
	tmp = (U8)(tmp | 0x01);
	_mt_fe_dmd_set_reg_t_ct8k(handle, 0xa8, tmp);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_reset_cci_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *reset_cci)
{
#define RATIO		5

	U32 bch_error_cnt0 = 0;
	U32 bch_cnt0 = 0;
	U8 reg_8a = 0,value =0;

	*reset_cci = 0;

	if((handle->m_device_ctt2.input_params.input_freq_kHz > 745000) && (handle->m_device_ctt2.input_params.input_freq_kHz < 747000))
	{
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x8a, &reg_8a);
		if(reg_8a & 0x80)
		{
			_mt_fe_dmd_get_reg_t_ct8k(handle, 0xb3, &value);
			if((value & 0x7f) != 0x0c)
			{
				reg_8a = (U8)((reg_8a & 0x0f) | 0x40);
				_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, reg_8a);
				_mt_fe_dmd_soft_reset_ct8k_t(handle);
				*reset_cci = 1;
			}
			else
			{
				_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa7, &value);
				bch_error_cnt0 = value << 8;
				_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa6, &value);
				bch_error_cnt0 = bch_error_cnt0 + value;

				_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa3, &value);
				bch_cnt0 = value << 8;
				_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa2, &value);
				bch_cnt0 = bch_cnt0 + value;

				if(bch_cnt0 == 0)
				{
					_mt_fe_dmd_clear_error_pack_ct8k_t(handle);
				}
				else if(((bch_error_cnt0 * 1000) / bch_cnt0) > RATIO)
				{
					reg_8a = (U8)((reg_8a & 0x0f) | 0x40);
					_mt_fe_dmd_set_reg_t_ct8k(handle, 0x8a, reg_8a);
					_mt_fe_dmd_soft_reset_ct8k_t(handle);
					*reset_cci = 1;
				}
				else
				{
					_mt_fe_dmd_clear_error_pack_ct8k_t(handle);
				}
			}
		}
		else
			*reset_cci = 0;
	}
	else
		*reset_cci = 0;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack)
{
	U32	bch_error_cnt = 0;
	U32	bch_cnt = 0;
	U32	corrected_cnt = 0;
	U8	value = 0;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa7, &value);
	bch_error_cnt = value << 8;
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa6, &value);
	bch_error_cnt = bch_error_cnt + value;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa3, &value);
	bch_cnt = value << 8;
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa2, &value);
	bch_cnt = bch_cnt + value;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa5, &value);
	corrected_cnt = value << 8;
	_mt_fe_dmd_get_reg_t_ct8k(handle, 0xa4, &value);
	corrected_cnt = corrected_cnt + value;

	*total_pack = bch_cnt;
	*error_pack = bch_error_cnt;
	*corrected_pack = corrected_cnt;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_mean_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack)
{
#define READ_TIMES	10

	U32 bch_error_cnt = 0, last_bch_error_cnt = 0;
	U32 bch_cnt = 0, last_bch_cnt = 0;
	U32 corrected_cnt = 0, last_corrected_cnt = 0;
	U8 value =0, lock_time=75;
	U8 wait_loop = 0, qam_mod = 0, tp_code = 0, fft_mode = 0;
	U8 i = 0;
	MT_FE_LOCK_STATE mt_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x95, &value);
	while((value & 0x04) == 0)// TPS_INFO_VALID flag
	{
		wait_loop ++;
		_mt_delay_ct8k(5);
		if (wait_loop > 40)
			break;
	}

	if (wait_loop > 40)
		return MtFeErr_TimeOut;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x91, &value);
	qam_mod = (U8)((value >> 2) & 0x03);

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x92, &value);
	tp_code = (U8)(value & 0x07);

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x90, &value);
	fft_mode = (U8)(value & 0xc0);

	_mt_fe_dmd_clear_error_pack_ct8k_t(handle);
	_mt_sleep_ct8k(200);
	_mt_fe_dmd_get_error_pack_ct8k_t(handle, &bch_cnt, &bch_error_cnt, &corrected_cnt);
	last_bch_cnt = bch_cnt;
	last_corrected_cnt = corrected_cnt;
	last_bch_error_cnt = bch_error_cnt;

	if((qam_mod == 0x02) && ((tp_code == 0x01) || (tp_code == 0x02)) && (fft_mode == 0x40))
	{
		for(i = 0; i < (READ_TIMES - 1); i ++)
		{
			_mt_fe_dmd_soft_reset_ct8k_t(handle);
			lock_time = 75;
			mt_state = MtFeLockState_Undef;
			while(lock_time --)
			{
				_mt_fe_dmd_get_lock_state_ct8k_t(handle, &mt_state);
				if(mt_state == MtFeLockState_Locked)
					break;
				_mt_sleep_ct8k(20);
			}

			if(mt_state == MtFeLockState_Locked)
			{
				_mt_fe_dmd_clear_error_pack_ct8k_t(handle);
				_mt_sleep_ct8k(200);
				_mt_fe_dmd_get_error_pack_ct8k_t(handle, &bch_cnt, &bch_error_cnt, &corrected_cnt);
				if ((bch_cnt != 0) && (last_bch_cnt != 0))
				{
					if((1000 * corrected_cnt / bch_cnt) < (1000 * last_corrected_cnt / last_bch_cnt))
					{
						last_bch_cnt = bch_cnt;
						last_corrected_cnt = corrected_cnt;
						last_bch_error_cnt = bch_error_cnt;
					}
				}
			}
		}
	}

	*total_pack = last_bch_cnt;
	*error_pack = last_bch_error_cnt;
	*corrected_pack = last_corrected_cnt;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_cell_info_ct8k_t(MT_FE_CT8K_Device_Handle handle)
{
	U8 tmp1, tmp2, tmp3, tmp4, iCnt = 0, iMaxCnt = 50;
	MT_BOOL bOk = FALSE;

	if(handle->m_device_ctt2.demod_type != MtFeType_DVBT)
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

	// Reg0x91 & Reg0x93
	// Reg0x91(b1-0)= 00B or 10B, Reg0x93 = Cell_ID(bit15-8)
	// Reg0x91(b1-0)= 01B or 11B, Reg0x93 = Cell_ID(bit7-0)

	do
	{
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x91, &tmp1);
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x93, &tmp2);
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x91, &tmp3);
		_mt_fe_dmd_get_reg_t_ct8k(handle, 0x93, &tmp4);

		if((tmp1 == tmp3) && (tmp2 == tmp4))
		{
			tmp1 &= 0x03;
			if((tmp1 == 0x00) || (tmp1 == 0x02))
			{
				handle->m_device_ctt2.cell_info.bHighByteOk = TRUE;
				handle->m_device_ctt2.cell_info.usCellId &= 0x00FF;
				handle->m_device_ctt2.cell_info.usCellId |= (tmp2 << 8);
			}
			else
			{
				handle->m_device_ctt2.cell_info.bLowByteOk = TRUE;
				handle->m_device_ctt2.cell_info.usCellId &= 0xFF00;
				handle->m_device_ctt2.cell_info.usCellId |= tmp2;
			}
		}

		bOk = (handle->m_device_ctt2.cell_info.bHighByteOk && handle->m_device_ctt2.cell_info.bLowByteOk) ? TRUE : FALSE;

		iCnt ++;
	}while((iCnt < iMaxCnt) && (!bOk));

	return bOk ? MtFeErr_Ok : MtFeErr_TimeOut;
}


MT_FE_RET _mt_fe_dmd_get_tps_info_ct8k_t(MT_FE_CT8K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info)
{
	U8	tmp = 0, tmp1 = 0;
	MT_FE_MOD_MODE			tp_qam = MtFeModMode_Undef;
	MT_FE_FFT				tp_fft = MtFeFFTMode_Undef;
	MT_FE_T_GUARD_INTERVAL	tp_guard = MtFeGuarInt_19P128;
	MT_FE_CODE_RATE			tp_code = MtFeCodeRate_Undef;

	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x90, &tmp);
	tmp1 = tmp >> 6;
	tmp1 &= 0x03;
	if(tmp1 == 0)
		tp_fft = MtFeFFTMode_2K;
	else if(tmp1 == 1)
		tp_fft = MtFeFFTMode_8K;
	else
		tp_fft = MtFeFFTMode_Undef;


	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x91, &tmp);
	tmp1 = tmp >> 2;
	tmp1 &= 0x03;
	if(tmp1 == 0)
		tp_qam = MtFeModMode_Qpsk;
	else if(tmp1 == 1)
		tp_qam = MtFeModMode_16Qam;
	else if(tmp1 == 2)
		tp_qam = MtFeModMode_64Qam;
	else
		tp_qam = MtFeModMode_Undef;

	//tmp1 = tmp >> 4;
	//tmp1 &= 0x07;


	_mt_fe_dmd_get_reg_t_ct8k(handle, 0x92, &tmp);
	tmp1 = tmp & 0x07;
	if(tmp1 == 0)
		tp_code = MtFeCodeRate_1_2;
	else if(tmp1 == 1)
		tp_code = MtFeCodeRate_2_3;
	else if(tmp1 == 2)
		tp_code = MtFeCodeRate_3_4;
	else if(tmp1 == 3)
		tp_code = MtFeCodeRate_5_6;
	else if(tmp1 == 4)
		tp_code = MtFeCodeRate_7_8;
	else
		tp_code = MtFeCodeRate_Undef;

	tmp1 = tmp >> 6;
	tmp1 &= 0x03;
	if(tmp1 == 0)
		tp_guard = MtFeGuarInt_1P32;
	else if(tmp1 == 1)
		tp_guard = MtFeGuarInt_1P16;
	else if(tmp1 == 2)
		tp_guard = MtFeGuarInt_1P8;
	else if(tmp1 == 3)
		tp_guard = MtFeGuarInt_1P4;
	else
		tp_guard = MtFeGuarInt_Undef;


	tps_info->t_qam = tp_qam;
	tps_info->t_fft = tp_fft;
	tps_info->t_guard = tp_guard;
	tps_info->t_code = tp_code;

	return MtFeErr_Ok;
}

