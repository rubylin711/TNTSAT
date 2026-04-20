/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2020                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
 * Filename:			mt_fe_dmd_cs8800_c.c
 * Current Version:		07
 *
 * Description: Montage Symphony2 DVBC mode demodulator IC driver.
 *
 *  History:
 *  Description			Version		Date		Author
 *---------------------------------------------------------------------------
 *  File Create			00			2018.06.25	YZ.Huang
 *  Modify				01			2018.06.25	YZ.Huang
 *  Modify				02			2018.09.06	YZ.Huang
 *  Modify				03			2018.09.26	YZ.Huang
 *  Modify				04			2019.03.21	YZ.Huang
 *  Modify				05			2019.07.04	YZ.Huang
 *  Modify				06			2019.10.31	YZ.Huang
 *  Modify				07			2022.11.24	YZ.Huang
 *****************************************************************************/
#include <linux/printk.h>

#include "mt_fe_dmd_cs8800_C.h"
#include "mt_fe_i2c_cs8800.h"


//extern int mt_dbg_printf(unsigned int level, const char *p_fmt, ...);


static const U32 mes_log[] =
{
	    0,	  3010,	  4771,	  6021,	  6990,	  7781,	  8451,	  9031,	  9542,	 10000,
	10414,	 10792,	 11139,	 11461,	 11761,	 12041,	 12304,	 12553,	 12788,	 13010,
	13222,	 13424,	 13617,	 13802,	 13979,	 14150,	 14314,	 14472,	 14624,	 14771,
	14914,	 15052,	 15185,	 15315,	 15441,	 15563,	 15682,	 15798,	 15911,	 16021,
	16128,	 16232,	 16335,	 16435,	 16532,	 16628,	 16721,	 16812,	 16902,	 16990,
	17076,	 17160,	 17243,	 17324,	 17404,	 17482,	 17559,	 17634,	 17709,	 17782,
	17853,	 17924,	 17993,	 18062,	 18129,	 18195,	 18261,	 18325,	 18388,	 18451,
	18513,	 18573,	 18633,	 18692,	 18751,	 18808,	 18865,	 18921,	 18976,	 19031
};

#define	XTAL_KHz			28800

/************************************************************************
** Function: mt_fe_dmd_get_driver_version_c_cs8800
**
**
** Description:	This function is used to get the driver version in number
**				mode.
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter			Type		Description
**	----------------------------------------------------------------------
**	 p_version			U8*		version number pointer
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_get_driver_version_cs8800_c(U8* p_version)
{
	*p_version = 7;		/* driver version number */

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_init_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	//mt_dbg_printf(1, "%s[%d] -- test 1\n", __FUNCTION__, __LINE__);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x84, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x84, 0x40);

	//mt_dbg_printf(1, "%s[%d] -- test 2, tuner_type = %d, addr = 0x%02x\n", __FUNCTION__, __LINE__, 
	//				 handle->m_device_c_b.tuner_cfg.tuner_type, 
	//				 handle->m_device_c_b.tuner_cfg.tuner_dev_addr);

	if (handle->m_device_c_b.tuner_cfg.tuner_init != NULL)
	{
		if (handle->m_device_c_b.tuner_cfg.tuner_init_ok == 0)
		{
			if (handle->m_device_c_b.tuner_cfg.tuner_init != NULL)
				ret = handle->m_device_c_b.tuner_cfg.tuner_init(handle);
		}
	}

	//mt_dbg_printf(1, "%s[%d] -- test 3\n", __FUNCTION__, __LINE__);

	return ret;
}

MT_FE_RET _mt_fe_dmd_connect_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 uinverted = 0;
	U8 tmp1 = 0, tmp2 = 0, tmp;

	MT_BOOL	bAutoMode = FALSE;
	U16		iQAMMode = 64;
	U32		iSymbolRateKSs = 6875;
	S32		iFreqOffsetKHz = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if (handle->connect_mode != 1)	// not set tuner only
	{
		//printk("%s[%d] ---- connect_mode = %d, set demod 1\n", __FUNCTION__, __LINE__, handle->connect_mode);

		_mt_fe_sar_get_reg_cs8800(handle, 0x07, &tmp1);
		tmp1 |= 0x40;
		_mt_fe_sar_set_reg_cs8800(handle, 0x07, tmp1);
		_mt_delay_cs8800(1);
		tmp1 &= 0xBF;
		_mt_fe_sar_set_reg_cs8800(handle, 0x07, tmp1);


		uinverted = handle->m_device_c_b.input_params.inverted;

		if ((handle->m_device_c_b.input_params.qam == 16) || 
			(handle->m_device_c_b.input_params.qam == 32) || 
			(handle->m_device_c_b.input_params.qam == 64) || 
			(handle->m_device_c_b.input_params.qam == 128) || 
			(handle->m_device_c_b.input_params.qam == 256))
		{
			bAutoMode = FALSE;
		}
		else
		{
			bAutoMode = TRUE;
		}

		if (handle->m_device_c_b.input_params.symbol_rate_KSs == 0)
		{
			bAutoMode = TRUE;
		}

		if (bAutoMode)
		{
			_mt_fe_dmd_get_reg_c_cs8800(handle, 0xb3, &tmp);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xb3, 0x00);
		}

		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x86, &tmp1);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x87, &tmp2);

		if (handle->m_device_c_b.tuner_cfg.tuner_type != MtFeTN_MxL603)
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x80, 0x01);
	}


	if (handle->connect_mode != 2)	// not set demod only
	{
		//printk("%s[%d] ---- connect_mode = %d, set tuner 1\n", __FUNCTION__, __LINE__, handle->connect_mode);

		if (handle->m_device_c_b.tuner_cfg.tuner_init_ok == 0)
		{
			if (handle->m_device_c_b.tuner_cfg.tuner_init != NULL)
			{
				ret = handle->m_device_c_b.tuner_cfg.tuner_init(handle);
			}
		}

		if (handle->m_device_c_b.tuner_cfg.tuner_set != NULL)
		{
			ret = handle->m_device_c_b.tuner_cfg.tuner_set(handle, handle->m_device_c_b.input_params.input_freq_kHz, 0, 0);
		}
	}

	if (bAutoMode)
	{
		if (handle->connect_mode != 1)	// not set tuner only
		{
			//printk("%s[%d] ---- connect_mode = %d, set demod 2\n", __FUNCTION__, __LINE__, handle->connect_mode);

			handle->m_device_c_b.input_params.symbol_rate_KSs = 6875;
			handle->m_device_c_b.input_params.qam = 64;

			ret = _mt_fe_dmd_set_demod_cs8800_c(handle);
			ret = _mt_fe_dmd_set_symbol_rate_cs8800_c(handle, (U32)XTAL_KHz);
			ret = _mt_fe_dmd_set_QAM_cs8800_c(handle);
			ret = _mt_fe_dmd_set_tx_mode_cs8800_c(handle);		/*	J83A */
			ret = _mt_fe_dmd_set_output_mode_cs8800_c(handle);	/*	parallel TS out */
			ret = _mt_fe_dmd_set_demod_appendix_cs8800_c(handle);

			if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_MxL603)
			{
#if 1
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x00);//0x13 : 0xEC);
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x00);//0x8E : 0x72);
#else
				if (uinverted == 1)
				{
					_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x13);
					_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x8E);
				}
				else
				{
					_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0xEC);
					_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x72);
				}
#endif
			}
			else if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
			{
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x00);//0x13 : 0xEC);
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x00);//0x8E : 0x72);
			}
		}

		if (handle->connect_mode != 2)	// not set demod only
		{
			//printk("%s[%d] ---- connect_mode = %d, set tuner 2\n", __FUNCTION__, __LINE__, handle->connect_mode);

			if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
			{
				if (handle->m_device_c_b.board_cfg.iVppSelC == 0)		// 2V
				{
					_mt_fe_tn_set_reg_c_b_cs8800(handle, 0x22, 0xA5);
				}
				else	// 1V
				{
					_mt_fe_tn_set_reg_c_b_cs8800(handle, 0x22, 0x95);
				}
			}
		}

		if (handle->connect_mode != 1)	// not set tuner only
		{
			//printk("%s[%d] ---- connect_mode = %d, set demod 3\n", __FUNCTION__, __LINE__, handle->connect_mode);

			ret = mt_fe_auto_QAM_debug_cs8800_c(handle, 3, &iQAMMode, &iSymbolRateKSs, &iFreqOffsetKHz);

			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xe8, 0x3b);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xe5, 0xa6);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0e, 0xa5);
			//_mt_fe_dmd_set_reg_c_cs8800(handle, 0x02, 0x60);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xef, 0x88);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x18, 0x00);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x53, 0x03);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x59, 0x30);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf9, 0x0a);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x01, 0x62);

			if (iQAMMode == 256)
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf1, 0x88);
			else
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf1, 0xff);

			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf0, 0x83);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4c, 0x0c);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0c, 0x00);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xb3, tmp);

			if (ret == MtFeErr_Ok)
			{
				handle->m_device_c_b.tp_cfg.qam_code = iQAMMode;
				handle->m_device_c_b.tp_cfg.sym_KSs = (U16)iSymbolRateKSs;
				//handle->m_device_c_b.tp_cfg.freq_KHz = handle->m_device_c_b.input_params.input_freq_kHz - iFreqOffsetKHz;

				//handle->m_device_c_b.input_params.input_freq_kHz -= iFreqOffsetKHz;
				handle->m_device_c_b.input_params.symbol_rate_KSs = (U16)iSymbolRateKSs;
				handle->m_device_c_b.input_params.qam = iQAMMode;
			}

			if (handle->m_device_c_b.input_params.symbol_rate_KSs == 0)
			{
				handle->m_device_c_b.input_params.symbol_rate_KSs = 6875;
			}
		}
	}

	if (handle->connect_mode != 1)	// not set tuner only
	{
		//printk("%s[%d] ---- connect_mode = %d, set demod 4\n", __FUNCTION__, __LINE__, handle->connect_mode);

		ret = _mt_fe_dmd_set_demod_cs8800_c(handle);
		ret = _mt_fe_dmd_set_symbol_rate_cs8800_c(handle, (U32)XTAL_KHz);
		ret = _mt_fe_dmd_set_QAM_cs8800_c(handle);
		ret = _mt_fe_dmd_set_tx_mode_cs8800_c(handle);		/* J83A */
		ret = _mt_fe_dmd_set_output_mode_cs8800_c(handle);	/* parallel TS out */
		ret = _mt_fe_dmd_set_demod_appendix_cs8800_c(handle);

		if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_MxL603)
		{
#if 1
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x00);//0x13 : 0xEC);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x00);//0x8E : 0x72);
#else
			if (uinverted == 1)
			{
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x13);
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x8E);
			}
			else
			{
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0xEC);
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x72);
			}
#endif
		}
		else if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
		{
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x16, 0x00);//0x13 : 0xEC);
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x17, 0x00);//0x8E : 0x72);
		}

		tmp1 &= 0x7F;
		tmp2 &= 0x07;

		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x86, tmp1);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x87, tmp2);
	}

	if (handle->connect_mode != 2)	// not set demod only
	{
		//printk("%s[%d] ---- connect_mode = %d, set tuner 3\n", __FUNCTION__, __LINE__, handle->connect_mode);

		if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
		{
			if (handle->m_device_c_b.board_cfg.iVppSelC == 0)		// 2V
			{
				_mt_fe_tn_set_reg_c_b_cs8800(handle, 0x22, 0xA5);
			}
			else	// 1V
			{
				_mt_fe_tn_set_reg_c_b_cs8800(handle, 0x22, 0x95);
			}
		}
	}

	if (handle->connect_mode != 1)	// not set tuner only
	{
		//printk("%s[%d] ---- connect_mode = %d, set demod 5\n", __FUNCTION__, __LINE__, handle->connect_mode);

		_mt_fe_dmd_soft_reset_cs8800_c(handle);
	}

	_mt_sleep_cs8800(10);

	return ret;
}


/***********************************************
 Initialize the internal registers in M88DC2000
************************************************/
MT_FE_RET _mt_fe_dmd_set_demod_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x05, 0x0D);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x36, 0x80);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x43, 0x40);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x61, 0x40);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x90, 0x06);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xDE, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xA0, 0x03);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xDF, 0x91);	// 0x81
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFA, 0x40);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x37, 0x10);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF2, 0x9C);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF3, 0x40);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x30, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x31, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x32, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x33, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x35, 0x32);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x39, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x3A, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF1, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF5, 0x40);

	if (handle->m_device_c_b.input_params.symbol_rate_KSs >= 5000)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x42, 0x24);
	}
	else
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x42, 0x14);
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x92, 0x7F);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2B, 0x33);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2E, 0x80);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2D, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xA4, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xA3, 0x0D);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF6, 0x4E);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF7, 0x20);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x14, 0x08);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x10, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x11, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x13, 0x23);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x60, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x69, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6A, 0x03);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE0, 0x75);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4E, 0xD8);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x88, 0x80);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x52, 0x79);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x53, 0x03);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x59, 0x30);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5E, 0x02);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5F, 0x0F);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x71, 0x03);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x72, 0x12);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x73, 0x12);

	return MtFeErr_Ok;
}

/***********************************************
   Set 	   symbol rate
   sym:		symbol rate, unit: KBaud
   xtal:	unit, KHz
************************************************/
MT_FE_RET _mt_fe_dmd_set_symbol_rate_cs8800_c(MT_FE_CS8800_Device_Handle handle, U32 xtal)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8	value = 0;
	U8	reg6FH = 0, reg12H = 0;
	U32	fValue = 0;
	U32	dwValue = 0;
	U32	sym = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	sym = handle->m_device_c_b.input_params.symbol_rate_KSs;

	fValue = ((((sym + 10) << 16) / xtal)  << 16) + ((((sym + 10) << 16) % xtal) << 16) / xtal;
	dwValue = (U32)fValue;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x58, (U8)((dwValue >> 24) & 0xff));
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x57, (U8)((dwValue >> 16) & 0xff));
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x56, (U8)((dwValue >>  8) & 0xff));
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x55, (U8)((dwValue >>  0) & 0xff));


	fValue = 20480 * xtal / (10 * sym);
	dwValue = (U32)fValue;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5D, (U8)((dwValue >> 8) & 0xff));
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5C, (U8)((dwValue >> 0) & 0xff));


	if (((dwValue >> 16) & 0x0001) == 0)
		value = 0x00;
	else
		value = 0x80;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5A, value);


	if (sym <= 1800)
		value = 0x03;
	else
		value = 0x02;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x89, value);


	if (sym >= 6700)
	{
		reg6FH = 0x0D;
		reg12H = 0x30;
	}
	else if (sym >= 4000)
	{
		fValue = 22 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x30;
	}
	else if (sym >= 2000)
	{
		fValue = 14 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x20;
	}
	else
	{
		fValue = 7 * 4096 / sym;
		reg6FH = (U8)fValue;
		reg12H = 0x10;
	}
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x12, reg12H);


	if (sym < 3000)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6E, 0x18);
	}
	else
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6E, 0x36);
	}

	return ret;
}

/***********************************************
   Set the type of MPEG/TS interface
   type: 1, serial format; 2, parallel format; 0, common interface
************************************************/
MT_FE_RET _mt_fe_dmd_set_output_mode_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 regC2H = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if (handle->m_device_c_b.ts_out_mode == MtFeTsOutMode_Common)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC0, 0x43);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE2, 0x06);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0xC2, &regC2H);
		regC2H &= 0xC7;
		regC2H |= 0x10;
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC2, regC2H);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC1, 0x60);		//common interface
	}
	else if (handle->m_device_c_b.ts_out_mode == MtFeTsOutMode_Serial)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC0, 0x47);		//serial format
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE2, 0x02);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0xC2, &regC2H);
		regC2H &= 0xC7;
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC2, regC2H);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC1, 0x00);
	}
	else//if (ts_mode == MtFeTsOutMode_Parallel)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC0, 0x43);		//parallel format
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE2, 0x06);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0xC2, &regC2H);
		regC2H &= 0xC7;
		//regC2H &= 0xC0;
		//regC2H |= 0x04;
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC2, regC2H);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC1, 0x00);
	}

	return ret;
}


/***********************************************
   Set QAM mode
   Qam: QAM mode, 16, 32, 64, 128, 256
************************************************/
MT_FE_RET _mt_fe_dmd_set_QAM_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 reg00H = 0, reg4AH = 0, reg4DH = 0, reg8BH = 0, reg8EH = 0, regC2H = 0, reg44H = 0, reg4CH = 0, reg74H = 0;
	U8 value = 0;
	U16 qam = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	qam = handle->m_device_c_b.input_params.qam;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xC2, &regC2H);
	regC2H &= 0xF8;

	switch (qam)
	{
		case 16:	// 16 QAM
			reg00H = 0x08;
			reg4AH = 0x0F;
			regC2H |= 0x02;//0x04;
			reg44H = 0xAA;
			reg4CH = 0x0C;
			reg4DH = 0xF7;
			reg74H = 0x0E;
			reg8BH = 0x5A;
			reg8EH = 0xBD;
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6E, 0x18);
			break;

		case 32:	// 32 QAM
			reg00H = 0x18;
			reg4AH = 0xFB;
			regC2H |= 0x02;//0x04;
			reg44H = 0xAA;
			reg4CH = 0x0C;
			reg4DH = 0xF7;
			reg74H = 0x0E;
			reg8BH = 0x5A;
			reg8EH = 0xBD;
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6E, 0x18);
			break;

		case 64:	// 64 QAM
			reg00H = 0x48;
			reg4AH = 0xCD;
			regC2H |= 0x02;//0x04;
			reg44H = 0xAA;
			reg4CH = 0x0C;
			reg4DH = 0xF7;
			reg74H = 0x0E;
			reg8BH = 0x5A;
			reg8EH = 0xBD;
			break;

		case 128:	// 128 QAM
			reg00H = 0x28;
			reg4AH = 0xFF;
			regC2H |= 0x02;//0x04;
			reg44H = 0xA9;
			reg4CH = 0x08;
			reg4DH = 0xF5;
			reg74H = 0x0E;
			reg8BH = 0x5B;
			reg8EH = 0x9D;
			break;

		case 256:	// 256 QAM
			reg00H = 0x38;
			reg4AH = 0xCD;
			regC2H |= 0x02;//0x04;
			reg44H = 0xA9;
			reg4CH = 0x08;
			reg4DH = 0xF5;
			reg74H = 0x0E;
			reg8BH = 0x5B;
			reg8EH = 0x9D;
			break;

		default:	// 64 QAM
			reg00H = 0x48;
			reg4AH = 0xCD;
			regC2H |= 0x02;//0x04;
			reg44H = 0xAA;
			reg4CH = 0x0C;
			reg4DH = 0xF7;
			reg74H = 0x0E;
			reg8BH = 0x5A;
			reg8EH = 0xBD;
			break;
	}

	reg00H |= 0x80;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x00, reg00H);

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x88, &value);
	value |= 0x08;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x88, value);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4B, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4A, reg4AH);
	value &= 0xF7;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x88, value);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xC2, regC2H);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x44, reg44H);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4C, reg4CH);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4D, reg4DH);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x74, reg74H);

	return ret;
}


/***********************************************
   Set spectrum inversion and J83
   Inverted: 1, inverted; 0, not inverted
   J83: 0, J83A; 1, J83C
 ***********************************************/
MT_FE_RET _mt_fe_dmd_set_tx_mode_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 value = 0;
	U8 inverted = 0, j83 = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	inverted = handle->m_device_c_b.input_params.inverted;

	if (inverted)	value |= 0x08;		/*	spectrum inverted	*/
	if (j83)		value |= 0x01;		/*	J83C				*/

	ret = _mt_fe_dmd_set_reg_c_cs8800(handle, 0x83, value);

	return ret;
}

MT_FE_RET _mt_fe_dmd_set_demod_appendix_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 tmp = 0;
	U16 qam = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	qam = handle->m_device_c_b.input_params.qam;

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x00, &tmp);
	//tmp |= 0x80;
	//_mt_fe_dmd_set_reg_c_cs8800(handle, 0x00, tmp);

	//_mt_fe_dmd_set_reg_c_cs8800(handle, 0x01, 0x42);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x01, 0x62);

	if ((qam == 256) || (qam == 128))
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x02, 0x10);
	}
	else
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x02, 0x60);
	}

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x03, &tmp);
	//tmp |= 0x30;
	//_mt_fe_dmd_set_reg_c_cs8800(handle, 0x03, tmp);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x03, 0x38);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x04, 0x88);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x24, 0xFF);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x25, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x26, 0xF8);		// 0xF9
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x27, 0x80);

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x5F, &tmp);
	//tmp &= 0x3F;
	//_mt_fe_dmd_set_reg_c_cs8800(handle, 0x5F, tmp);


	if (qam == 16)
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x62, 0x18);
	else
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x62, 0x28);

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x00, &tmp);
	//if (((tmp & 0x70) == 0x20) || ((tmp & 0x70) == 0x10) || ((tmp & 0x70) == 0x00))		// 128 QAM or 32QAM or 16QAM
	if (qam == 16)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6C, 0x42);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF1, 0xFF);
	}
	else if ((qam == 32) || (qam == 128))
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6C, 0x52);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF1, 0xFF);
	}
	else if (qam == 256)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6C, 0x62);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF1, 0x88);
	}
	else // if (qam == 64)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6C, 0x62);
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF1, 0xFF);
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6D, 0xA3);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x6F, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x75, 0x18);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x7F, 0x71);

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x93, &tmp);
	//tmp &= 0x7F;
	tmp = 0x11;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x93, tmp);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE5, 0xA6);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xE8, 0x3B);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF0, 0x83);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF4, 0x0F);
	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0xF9, &tmp);
	//tmp = 0x08;
	//tmp &= 0xF8;
	//tmp |= 0x02;
	tmp = 0x0A;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xF9, tmp);

	if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_MxL603)
	{
		if ((qam == 16) || (qam == 32))
		{
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFB, 0x24);
		}
		else
		{
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFB, 0x44);
		}
	}
	else
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFB, 0x44);
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFC, 0xC1);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFD, 0x13);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xFE, 0x50);

	//_mt_fe_dmd_get_reg_c_cs8800(handle, 0x95, &tmp);
	//tmp |= 0x40;
	tmp = 0x40;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x95, tmp);

	if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_MxL603)
	{
		if ((qam == 16) || (qam == 32))
		{
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2A, 0x2A);
		}
		else
		{
			_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2A, 0x2C);
		}
	}
	else
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x2A, 0x27);
	}

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x86, &tmp);
	tmp |= 0x08;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x86, tmp);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xCE, 0x6D);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xB8, 0x3C);


	return ret;
}


/***********************************************
 Soft reset M88DC2000
 Reset the internal status of each function block,
 not reset the registers.
************************************************/
MT_FE_RET _mt_fe_dmd_soft_reset_cs8800_c(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x80, 0x01);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x82, 0x00);
	_mt_delay_cs8800(1);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x80, 0x00);

	return ret;
}


MT_FE_RET _mt_fe_dmd_get_statistics_cs8800_c(MT_FE_CS8800_Device_Handle handle,
										  U32 *agc_lock,
										  U32 *timing_lock,
										  U32 *dagc_lock,
										  U32 *carrier_lock,
										  U32 *sync_lock,
										  U32 *descrambler_lock,
										  U32 *chip_lock
										 )
{
	U8 tmp = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x43, &tmp);
	*agc_lock = ((tmp & 0x08) == 0x08) ? 1 : 0;

	//*descrambler_lock = ((tmp & 0x10) == 0x10) ? 1 : 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x5E, &tmp);
	*timing_lock = ((tmp & 0x80) == 0x80) ? 1 : 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x74, &tmp);
	*dagc_lock = ((tmp & 0x80) == 0x80) ? 1 : 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xA3, &tmp);
	*carrier_lock = ((tmp & 0x80) == 0x80) ? 1 : 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x85, &tmp);
	*sync_lock = ((tmp & 0x10) == 0x10) ? 1 : 0;

	*descrambler_lock = ((tmp & 0x01) == 0x01) ? 1 : 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x85, &tmp);

	*chip_lock = ((tmp & 0x11) == 0x11) ? 1 : 0;

	return MtFeErr_Ok;
}

/***********************************************
   Get lock status
   returned 1 when locked;0 when unlocked
************************************************/
MT_FE_RET _mt_fe_dmd_get_lock_state_cs8800_c(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *lock_status)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 reg85H = 0;
	U8 i = 0;

	*lock_status = MtFeLockState_Unlocked;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	for (i = 0; i < 10; i++)
	{
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x85, &reg85H);

		if ((reg85H & 0x11) != 0x11)
		{
			*lock_status = MtFeLockState_Unlocked;
			break;
		}

		*lock_status = MtFeLockState_Locked;
	}

	return ret;
}


/***********************************************
   Get BER (bit error rate)
************************************************/
MT_FE_RET _mt_fe_dmd_get_ber_cs8800_c(MT_FE_CS8800_Device_Handle handle, U32 *error_bits, U32 *total_bits)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U16	tmp = 0;

	U8 regA0H = 0, regA1H = 0, regA2H = 0, n_byte = 0;

	MT_FE_LOCK_STATE lock_status = MtFeLockState_Unlocked;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_lock_state_cs8800_c(handle, &lock_status);

	*error_bits = 0;
	*total_bits = 33554432;

	if (lock_status != MtFeLockState_Locked)
	{
		return MtFeErr_UnLock;
	}

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xA0, &regA0H);
	n_byte = (U8)(regA0H & 0x07);

	if ((regA0H & 0x80) != 0x80)
	{
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0xA1, &regA1H);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0xA2, &regA2H);

		tmp = (U16)((regA2H << 8) + regA1H);

		*error_bits = tmp;
		*total_bits = 1 << (n_byte * 2 + 15);

		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xA0, n_byte);
		n_byte |= 0x80;
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0xA0, n_byte);
	}
	else
	{
		ret = MtFeErr_Fail;
	}

	return ret;
}


/***********************************************
   Get PER (packet error rate)
************************************************/
MT_FE_RET _mt_fe_dmd_get_per_cs8800_c(MT_FE_CS8800_Device_Handle handle, U32 *error_packages, U32 *total_packages)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 regD0H = 0, regD1H = 0, regD4H = 0, regD5H = 0, regDFH = 0;

	MT_FE_LOCK_STATE lock_status = MtFeLockState_Unlocked;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_lock_state_cs8800_c(handle, &lock_status);

	if (lock_status != MtFeLockState_Locked)
	{
		*error_packages = 65535;
		*total_packages = 65535;

		return MtFeErr_UnLock;
	}

	//Read reg0xd1[7:0] , if (reg0xd1[7:0]<=7) do nothing
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xD1, &regD1H);
	if (regD1H <= 0x07)
	{
		*error_packages = 0;
		*total_packages = 65535;

		return MtFeErr_Fail;
	}

	// else set reg0xdf[1] to 1,
	//    then read reg0xd1[7:0], read reg0xd0[7:0], 
	//         read reg0xd5[7:0], read reg0xd4[7:0]
	// Total block number: {reg0xd1[7:0], reg0xd0[7:0]}, 16bits counter
	// Error block number: {reg0xd5[7:0], reg0xd4[7:0]}, 16bits counter

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xDF, &regDFH);
	regDFH |= 0x02;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xDF, regDFH);

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xD1, &regD1H);
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xD0, &regD0H);
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xD5, &regD5H);
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xD4, &regD4H);

	*error_packages = (regD5H << 8) + regD4H;
	*total_packages = (regD1H << 8) + regD0H;

	regDFH &= ~0x03;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xDF, regDFH);

	/*set 0xdf[0] to 1,no clears values*/
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0xDF, &regDFH);
	regDFH |= 0x01;
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xDF, regDFH);
	return ret;
}

/***********************************************
   Get accurate SNR (signal noise ratio) Unit: 0.001dB
************************************************/
MT_FE_RET _mt_fe_dmd_get_accurate_snr_cs8800_c(MT_FE_CS8800_Device_Handle handle, S32 *signal_snr)
{
	MT_FE_RET ret = MtFeErr_Ok;

	S32	snr = 0, iCnt = 0;
	U8	i = 0;
	U32	mse = 0;
	U8	reg91H = 0, reg08H = 0, reg07H = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	*signal_snr = 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x91, &reg91H);

	if ((reg91H & 0x23) != 0x03)
		return MtFeErr_Fail;

	mse = 0;
	for (i = 0; i < 30; i++)
	{
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x08, &reg08H);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x07, &reg07H);

		mse += (reg08H << 8) + reg07H;
	}
	mse /= 30;

	while (mse > 80)
	{
		mse /= 2;
		iCnt++;
	}

	switch (handle->m_device_c_b.input_params.qam)
	{
		case 16:	snr = 34080;	break;	/*	16QAM	*/
		case 32:	snr = 37600;	break;	/*	32QAM	*/
		case 64:	snr = 40310;	break;	/*	64QAM	*/
		case 128:	snr = 43720;	break;	/*	128QAM	*/
		case 256:	snr = 46390;	break;	/*	256QAM	*/
		default:	snr = 40310;	break;
	}


	snr -= (mes_log[mse - 1] + iCnt * 3010);		/*	C - 10*log10(MSE)	*/
	//snr /= 1000;

	*signal_snr = snr;

	return ret;
}
/***********************************************
   Get SNR (signal noise ratio)
************************************************/
MT_FE_RET _mt_fe_dmd_get_snr_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 *signal_snr)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U32	snr = 0;
	U8	i = 0;
	U32	mse = 0;
	U8 reg91H = 0, reg08H = 0, reg07H = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	*signal_snr = 0;

	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x91, &reg91H);

	if ((reg91H & 0x23) != 0x03)
		return MtFeErr_Fail;

	mse = 0;
	for (i = 0; i < 30; i++)
	{
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x08, &reg08H);
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x07, &reg07H);

		mse += (reg08H << 8) + reg07H;
	}
	mse /= 30;
	if (mse > 80)
		mse = 80;

	switch (handle->m_device_c_b.input_params.qam)
	{
		case 16:	snr = 34080;	break;	/*	16QAM	*/
		case 32:	snr = 37600;	break;	/*	32QAM	*/
		case 64:	snr = 40310;	break;	/*	64QAM	*/
		case 128:	snr = 43720;	break;	/*	128QAM	*/
		case 256:	snr = 46390;	break;	/*	256QAM	*/
		default:	snr = 40310;	break;
	}


	snr -= mes_log[mse - 1];					/*	C - 10*log10(MSE)	*/

	printk("\tDVB-C SNR = %d.%03d dB\n", snr / 1000, snr % 1000);

	snr /= 1000;
	if (snr > 0xff)
		snr = 0xff;

	*signal_snr = (U8)snr;

	return ret;
}

/***********************************************
   Get signal strength
************************************************/
MT_FE_RET _mt_fe_dmd_get_strength_gain_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 *signal_strength)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 SignalStrength = 0;
	U8 reg43H = 0;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	ret = _mt_fe_dmd_get_reg_c_cs8800(handle, 0x43, &reg43H);

	if ((reg43H & 0x08) == 0x08)
	{
		U8 reg3BH = 0, reg3CH = 0;

		ret = _mt_fe_dmd_get_reg_c_cs8800(handle, 0x3B, &reg3BH);
		ret = _mt_fe_dmd_get_reg_c_cs8800(handle, 0x3C, &reg3CH);

		SignalStrength = (U8)(255 - (reg3BH >> 1) - (reg3CH >> 1));
	}

	*signal_strength = SignalStrength;

	return ret;
}

MT_FE_RET _mt_fe_dmd_get_strength_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent)
{
	S32	level_rel = 0, percent;
	S8	tuner_strength = 0;
	//S8	level_ref = 0;

	if (handle->m_device_c_b.tuner_cfg.tuner_strength != NULL)
	{
		handle->m_device_c_b.tuner_cfg.tuner_strength(handle, &tuner_strength);
	}
	else
		tuner_strength = 0;


#if 1	// Vestel
	//tuner_strength *= 10;
	level_rel = tuner_strength * 10;

	if (handle->m_device_c_b.input_params.qam == 256)
	{
		if (level_rel >= -300)
		{
			percent =  1000;
		}
		else if (level_rel >= -550)
		{
			percent = 750 + (550 + level_rel);
		}
		else if (level_rel >= -625)
		{
			percent = 650 + (625 + level_rel) * 4 / 3;
		}
		else if (level_rel >= -675)
		{
			percent = 300 + (675 + level_rel) * 7;
		}
		else if (level_rel >= -725)
		{
			percent = 100 + (725 + level_rel) * 4;
		}
		else if (level_rel >= -775)
		{
			percent = 25 + (775 + level_rel) * 3 / 2;
		}
		else
		{
			percent = 0;
		}
	}
	else if (handle->m_device_c_b.input_params.qam == 128)
	{
		if (level_rel >= -300)
		{
			percent =  1000;
		}
		else if (level_rel >= -550)
		{
			percent = 750 + (550 + level_rel);
		}
		else if (level_rel >= -625)
		{
			percent = 650 + (625 + level_rel) * 4 / 3;
		}
		else if (level_rel >= -675)
		{
			percent = 350 + (675 + level_rel) * 6;
		}
		else if (level_rel >= -725)
		{
			percent = 200 + (725 + level_rel) * 3;
		}
		else if (level_rel >= -775)
		{
			percent = 50 + (775 + level_rel)  * 3;
		}
		else
		{
			percent = 0;
		}
	}
	else  //64 QAM, 32 QAM or 16 QAM
	{
		if (level_rel >= -300)
		{
			percent =  1000;
		}
		else if (level_rel >= -550)
		{
			percent = 750 + (550 + level_rel);
		}
		else if (level_rel >= -625)
		{
			percent = 600 + (625 + level_rel) * 2;
		}
		else if (level_rel >= -675)
		{
			percent = 350 + (675 + level_rel) * 5;
		}
		else if (level_rel >= -725)
		{
			percent = 250 + (725 + level_rel) * 2;
		}
		else if (level_rel >= -775)
		{
			percent = 100 + (775 + level_rel) * 3;
		}
		else
		{
			percent = 0;
		}
	}

	*ssi_percent = (U8)(percent / 10);

#else

	if (handle->m_device_c_b.input_params.qam == 16)
		level_ref = -77;
	else if (handle->m_device_c_b.input_params.qam == 32)
		level_ref = -74;
	else if (handle->m_device_c_b.input_params.qam == 64)
		level_ref = -71;
	else if (handle->m_device_c_b.input_params.qam == 128)
		level_ref = -68;
	else if (handle->m_device_c_b.input_params.qam == 256)
		level_ref = -65;
	else
		level_ref = -71;


	level_rel = tuner_strength - level_ref;

	if (level_rel <= -15)
		*ssi_percent = 0;
	else if (level_rel < 0)
		*ssi_percent = (U8)((level_rel + 15) * 2 / 3);
	else if (level_rel < 20)
		*ssi_percent = (U8)(level_rel * 4 + 10);
	else if (level_rel < 35)
		*ssi_percent = (U8)((level_rel - 20) * 2 / 3 + 90);
	else
		*ssi_percent = 100;
#endif

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_quality_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 *p_percent)
{
#if 1		// Vestel
	//S32	percent = 0;
	U8	i = 0;
	//S32	CN_Rel = 0, CN_Perfor = 0;
	U32	snr = 0, snr_100, snr_int;
	U32	mse = 0, iRatio, iRatio_100;
	U8 reg91H = 0, reg08H = 0, reg07H = 0;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_lock_state_cs8800_c(handle, &q_state);

	if (q_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x91, &reg91H);

		if ((reg91H & 0x23) != 0x03)
		{
			*p_percent = 0;
			return MtFeErr_Fail;
		}

		mse = 0;

		for (i = 0; i < 25; i++)
		{
			_mt_fe_dmd_get_reg_c_cs8800(handle, 0x08, &reg08H);
			_mt_fe_dmd_get_reg_c_cs8800(handle, 0x07, &reg07H);

			mse += (reg08H << 8) + reg07H;

			_mt_delay_cs8800(1);
		}

		iRatio = mse / 25;
		iRatio_100 = (mse * 100 - iRatio * 100) / 2500;

		if (iRatio < 1)
		{
			iRatio = 1;
			iRatio_100 = 0;
		}

		if (iRatio >= 80)
		{
			iRatio = 80;
			iRatio_100 = 0;
		}

		snr_int = mes_log[iRatio - 1];

		if (iRatio == 80)
			snr_100 = 0;
		else
			snr_100 = (mes_log[iRatio] - mes_log[iRatio - 1]) * iRatio_100 / 100;

		snr_int += snr_100;

		switch (handle->m_device_c_b.input_params.qam)
		{
			case 16:	snr = 34080;	break;	/*	16QAM	*/
			case 32:	snr = 37600;	break;	/*	32QAM	*/
			case 64:	snr = 40310;	break;	/*	64QAM	*/
			case 128:	snr = 43720;	break;	/*	128QAM	*/
			case 256:	snr = 46390;	break;	/*	256QAM	*/
			default:	snr = 40310;	break;
		}

		//snr -= mes_log[mse - 1];					/*	C - 10*log10(MSE)	*/
		snr -= snr_int;


		switch (handle->m_device_c_b.input_params.qam)
		{
			case 16:
				if (snr <= 16000)
					*p_percent = 0;
				else if (snr <= 16200)
					*p_percent =  0 + (snr - 16000) / 20;
				else if (snr <= 16700)
					*p_percent = 10 + (snr - 16200) / 50;
				else if (snr <= 17500)
					*p_percent = 20 + (snr - 16700) / 80;
				else if (snr <= 18400)
					*p_percent = 30 + (snr - 17500) / 60;
				else if (snr <= 19300)
					*p_percent = 45 + (snr - 18400) / 60;
				else if (snr <= 20300)
					*p_percent = 60 + (snr - 19300) / 50;
				else if (snr <= 21300)
					*p_percent = 80 + (snr - 20300) / 100;
				else
					*p_percent = 100;
				break;

			case 32:
				if (snr <= 18900)
					*p_percent = 0;
				else if (snr <= 20300)
					*p_percent =  0 + (snr - 18900) / 70;
				else if (snr <= 21300)
					*p_percent = 20 + (snr - 20300) / 100;
				else if (snr <= 22200)
					*p_percent = 30 + (snr - 21300) / 60;
				else if (snr <= 23100)
					*p_percent = 45 + (snr - 22200) / 60;
				else if (snr <= 24100)
					*p_percent = 60 + (snr - 23100) / 50;
				else if (snr <= 25500)
					*p_percent = 80 + (snr - 24100) / 70;
				else
					*p_percent = 100;
				break;

			case 128:
				if (snr <= 24750)
					*p_percent = 0;
				else if (snr <= 25000)
					*p_percent =  0 + (snr - 24750) / 25;
				else if (snr <= 26000)
					*p_percent = 10 + (snr - 25000) / 50;
				else if (snr <= 26900)
					*p_percent = 30 + (snr - 26000) / 60;
				else if (snr <= 27800)
					*p_percent = 45 + (snr - 26900) / 60;
				else if (snr <= 28800)
					*p_percent = 60 + (snr - 27800) / 100;
				else if (snr <= 29600)
					*p_percent = 80 + (snr - 28800) / 80;
				else if (snr <= 30600)
					*p_percent = 90 + (snr - 29600) / 100;
				else
					*p_percent = 100;
				break;

			case 256:
				if (snr <= 27280)
					*p_percent = 0;
				else if (snr <= 27480)
					*p_percent =  0 + (snr - 27280) / 10;
				else if (snr <= 28080)
					*p_percent = 20 + (snr - 27480) / 60;
				else if (snr <= 28880)
					*p_percent = 30 + (snr - 28080) / 80;
				else if (snr <= 29880)
					*p_percent = 40 + (snr - 28880) / 100;
				else if (snr <= 30580)
					*p_percent = 50 + (snr - 29880) / 70;
				else if (snr <= 31480)
					*p_percent = 60 + (snr - 30580) / 30;
				else if (snr <= 32080)
					*p_percent = 90 + (snr - 31480) / 60;
				else
					*p_percent = 100;
				break;

			case 64:
			default:
				if (snr <= 21300)
					*p_percent = 0;
				else if (snr <= 21500)
					*p_percent =  0 + (snr - 21300) / 10;
				else if (snr <= 22500)
					*p_percent = 10 + (snr - 21500) / 50;
				else if (snr <= 23400)
					*p_percent = 30 + (snr - 22500) / 60;
				else if (snr <= 24300)
					*p_percent = 45 + (snr - 23400) / 60;
				else if (snr <= 25300)
					*p_percent = 60 + (snr - 24300) / 100;
				else if (snr <= 26300)
					*p_percent = 70 + (snr - 25300) / 50;
				else if (snr <= 27300)
					*p_percent = 90 + (snr - 26300) / 100;
				else
					*p_percent = 100;
				break;
		}
	}
	else
	{
		*p_percent = 0;
	}
#else
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
	S32	percent = 0;
	U8	i = 0;
	S32	CN_Rel = 0, CN_Perfor = 0;
	U32	snr = 0;
	U32	mse = 0;
	U8 reg91H = 0, reg08H = 0, reg07H = 0;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_lock_state_cs8800_c(handle, &q_state);

	if (q_state == MtFeLockState_Locked)
	{
		if (handle->m_device_c_b.input_params.qam == 16)
			CN_Perfor = 2000;
		else if (handle->m_device_c_b.input_params.qam == 32)
			CN_Perfor = 2300;
		else if (handle->m_device_c_b.input_params.qam == 64)
			CN_Perfor = 2600;
		else if (handle->m_device_c_b.input_params.qam == 128)
			CN_Perfor = 2900;
		else if (handle->m_device_c_b.input_params.qam == 256)
			CN_Perfor = 3200;
		else
			CN_Perfor = 3000;

		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x91, &reg91H);

		if ((reg91H & 0x23) != 0x03)
			return MtFeErr_Fail;

		mse = 0;
		for (i = 0; i < 30; i++)
		{
			_mt_fe_dmd_get_reg_c_cs8800(handle, 0x08, &reg08H);
			_mt_fe_dmd_get_reg_c_cs8800(handle, 0x07, &reg07H);

			mse += (reg08H << 8) + reg07H;
		}
		mse /= 30;
		if (mse > 80)
			mse = 80;

		switch (handle->m_device_c_b.input_params.qam)
		{
			case 16:	snr = 34080;	break;	/*	16QAM	*/
			case 32:	snr = 37600;	break;	/*	32QAM	*/
			case 64:	snr = 40310;	break;	/*	64QAM	*/
			case 128:	snr = 43720;	break;	/*	128QAM	*/
			case 256:	snr = 46390;	break;	/*	256QAM	*/
			default:	snr = 40310;	break;
		}

		snr -= mes_log[mse - 1];					/*	C - 10*log10(MSE)	*/
		snr /= 10;
		if (snr > 25500)
			snr = 25500;

		CN_Rel = snr - CN_Perfor;

		if (CN_Rel < -500)
			percent = 0;
		else if (CN_Rel < -200)
			percent = (S32)((14 * 500 + 14 * CN_Rel) / 300);
		else if (CN_Rel < 600)
			percent = (S32)(14 + (86 * (CN_Rel + 200) / 800));
		else
			percent = 100;

		if (percent > 100)
			*p_percent = 100;
		else if (percent < 0)
			*p_percent = 0;
		else
			*p_percent =(U8)percent;
	}
	else
	{
		*p_percent = 0;
	}
#endif

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_auto_QAM_debug_cs8800_c
**
**	DESCRIPTION:
**		auto get qam,symbol rate,freq offset in  Symphony2 dvbc
**		only used when debug
**	IN:
**		MT_FE_CS8800_Device_Handle handle
**		U8 auto_mode:
**			1: auto-qam only,
**			2: auto-qam and auto-symbol rate
**			3: auto-qam and auto-symbol rate and auto frequency offset
**	OUT:
**		U16 *auto_qam	 : QAM mode, 16, 32, 64, 128, 256
**		U32 *auto_symbol : symbol rate, unit: KBaud
**		S32 *auto_offset : frequency offset ,unit: KBaud
**
**	RETURN:
*/
MT_FE_RET mt_fe_auto_QAM_debug_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 auto_mode, U16 *auto_qam, U32 *auto_symbol, S32 *auto_offset)
{
	U8	tmp1 = 0;
	S8	biggest_indicator = 0;
	U8	biggest_index = 0;
	U8	i = 0;
	S8	qam_indicator_value[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	U16	qam_mode = 0;
	U32	symbol_rate_12bit = 0;
	S32	symbol_rate = 0;  //KHz
	S32	frequency_offset_16bit = 0;
	S32	symbol_rate_error = 0;
	S32	frequency_offset = 0; //KHz
	U16	crystal_freq = 28800; //KHz
	S8	smallest_in_max_value = 0x7f;
	U32	count = 0;

	//auto_mode = 1, do auto-qam only;
	//auto_mode = 2, do auto-qam and auto-symbol rate
	//auto_mode = 3, do auto-qam and auto-symbol rate and auto frequency offset

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x80, 0x01);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xe8, 0x3a);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xe5, 0x26);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0e, 0x24);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x02, 0x18);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xef, 0x44);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x18, 0xc0);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x53, 0x00);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x59, 0x2d);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf9, 0x02);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x01, 0x41);

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf1, 0xff);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0xf0, 0xa3);
	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x4c, 0x4c);

	switch (auto_mode)
	{
		case 1:	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0c, 0x80); break;
		case 2:	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0c, 0xc0); break;
		case 3:	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0c, 0xe0); break;
		default:
				_mt_fe_dmd_set_reg_c_cs8800(handle, 0x0c, 0x80); break;
	}

	_mt_fe_dmd_set_reg_c_cs8800(handle, 0x80, 0x00);

	count = 0;
	do
	{
		_mt_delay_cs8800(5);

		count++;
		if (count > 200)
		{
			return MtFeErr_Fail;
		}
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x0c, &tmp1);
		tmp1 = (U8)(tmp1 & 0x10);
	} while (tmp1 == 0);

	for (i = 0; i < 10; i++)
	{
		_mt_fe_dmd_set_reg_c_cs8800(handle, 0x18, i);
		tmp1 = 0;
		_mt_fe_dmd_get_reg_c_cs8800(handle, 0x0d, &tmp1);
		qam_indicator_value[i] = tmp1;
	}

	biggest_indicator = qam_indicator_value[0];
	biggest_index = 0;
	for (i = 0; i < 10; i++)
	{
		if (qam_indicator_value[i] > biggest_indicator)
		{
			biggest_indicator = qam_indicator_value[i];
			biggest_index = i;
		}
	}

	if (biggest_indicator < smallest_in_max_value)
		smallest_in_max_value = biggest_indicator;

	switch (biggest_index)
	{
		case 0:
		case 5:
			if (biggest_indicator < 60)
				biggest_index = 4;
			else
				biggest_index = 0;
			break;

		case 1:
		case 6:
			if (biggest_indicator < 50)
				biggest_index = 4;
			else
				biggest_index = 1;
			break;

		case 2:
		case 7:
			if (biggest_indicator < 50)
				biggest_index = 4;
			else
				biggest_index = 2;
			break;

		case 3:
		case 8:
			if (biggest_indicator < 50)
				biggest_index = 4;
			else
				biggest_index = 3;
			break;

		default:
			biggest_index = 4;
			break;
	}

	switch (biggest_index)
	{
		case 0:		qam_mode = 16;	break;
		case 1:		qam_mode = 32;	break;
		case 2:		qam_mode = 64;	break;
		case 3:		qam_mode = 128;	break;
		case 4:		qam_mode = 256;	break;
		case 5:		qam_mode = 16;	break;
		case 6:		qam_mode = 32;	break;
		case 7:		qam_mode = 64;	break;
		case 8:		qam_mode = 128;	break;
		case 9:		qam_mode = 256;	break;
		default:	qam_mode = 0;	break;
	}

	tmp1 = 0;
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x19, &tmp1);
	symbol_rate_12bit = tmp1 << 4;
	tmp1 = 0;
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x1a, &tmp1);
	symbol_rate_12bit = symbol_rate_12bit | (tmp1 & 0x0f);

	symbol_rate = symbol_rate_12bit * crystal_freq / 4096;

	tmp1 = 0;
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x1b, &tmp1);
	frequency_offset_16bit = tmp1 << 8;
	tmp1 = 0;
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x1c, &tmp1);
	frequency_offset_16bit = frequency_offset_16bit | tmp1;
	if (frequency_offset_16bit > 32767)
		frequency_offset_16bit = frequency_offset_16bit - 65536;
	frequency_offset = frequency_offset_16bit * crystal_freq / 65536;

	tmp1 = 0;
	_mt_fe_dmd_get_reg_c_cs8800(handle, 0x51, &tmp1);
	symbol_rate_error = (S32)tmp1;

	if (symbol_rate_error >= 128)
		symbol_rate_error = symbol_rate_error - 256;
	symbol_rate = symbol_rate - symbol_rate_error * symbol_rate / 2000;

	*auto_qam = qam_mode;
	*auto_symbol = (U32)symbol_rate;
	*auto_offset = (S32)frequency_offset;

	return MtFeErr_Ok;
}

