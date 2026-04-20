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
 * Filename:            mt_fe_dmd_ct8k_b.c
 *
 * Description:         Montage Symphony2 J83B demodulator driver.
 *
 * Current Version:     0.00.05
 *
 *  History:
 *
 *  Description     Version     Date        Author
 *---------------------------------------------------------------------------
 *  File Create     0.00.01     2018.06.22  YZ.Huang
 *  Modify          0.00.01     2018.06.22  YZ.Huang
 *  Modify          0.00.02     2018.08.22  YZ.Huang
 *  Modify          0.00.03     2019.01.03  YZ.Huang
 *  Modify          0.00.04     2019.07.25  YZ.Huang
 *  Modify          0.00.05     2019.11.26  YZ.Huang
 *****************************************************************************/


#include "mt_fe_i2c_ct8k.h"
#include "mt_fe_dmd_ct8k_B.h"


#define ABS(x) ((x) < 0 ? -(x) : (x))

#define CT8K_J83B_SNR_SMOOTH_CNT		10

const U16 mse2snr_q64[256] = 
{
  290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290, 290,
  290, 288, 286, 284, 282, 281, 279, 277, 275, 273, 271, 269, 268, 266, 264, 262,
  260, 259, 258, 256, 255, 254, 252, 251, 250, 249, 248, 246, 245, 244, 242, 241,
  240, 239, 239, 238, 237, 237, 236, 235, 235, 234, 233, 233, 232, 231, 231, 230,
  229, 229, 228, 228, 227, 226, 226, 225, 224, 224, 223, 222, 222, 221, 221, 220,
  220, 219, 219, 218, 218, 218, 217, 217, 217, 216, 216, 215, 215, 214, 212, 211,
  210, 210, 209, 209, 208, 208, 207, 206, 206, 205, 205, 205, 204, 204, 204, 204,
  203, 203, 203, 202, 202, 202, 202, 201, 201, 201, 201, 200, 200, 200, 199, 199,
  199, 198, 198, 197, 197, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190,
  190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190
};

const U16 mse2snr_q256[256] = 
{
  400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400,
  400, 400, 398, 396, 394, 392, 390, 388, 386, 384, 382, 380, 379, 378, 377, 376,
  375, 374, 373, 372, 371, 370, 362, 354, 346, 338, 330, 328, 326, 324, 322, 320,
  319, 318, 317, 316, 315, 314, 313, 312, 311, 310, 308, 306, 304, 302, 300, 298,
  295, 295, 294, 294, 294, 293, 293, 292, 292, 292, 291, 291, 291, 290, 290, 289,
  288, 288, 287, 286, 285, 284, 283, 282, 282, 281, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280,
  280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280, 280
};


MT_FE_RET mt_fe_dmd_config_tuner_settings_ct8k_b(MT_FE_CT8K_Device_Handle handle, MT_FE_CT8K_TN_DEV_SETTINGS *tuner_config)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if(tuner_config == NULL)
	{
		return MtFeErr_NullPointer;
	}

	handle->m_device_ctt2.tuner_cfg.tuner_init_ok		 = tuner_config->tuner_init_ok;
	handle->m_device_ctt2.tuner_cfg.tuner_type			 = tuner_config->tuner_type;
	handle->m_device_ctt2.tuner_cfg.tuner_dev_addr		 = tuner_config->tuner_dev_addr;

	handle->m_device_ctt2.tuner_cfg.tuner_init			 = tuner_config->tuner_init;
	handle->m_device_ctt2.tuner_cfg.tuner_set			 = tuner_config->tuner_set;
	handle->m_device_ctt2.tuner_cfg.tuner_get_offset	 = tuner_config->tuner_get_offset;
	handle->m_device_ctt2.tuner_cfg.tuner_strength		 = tuner_config->tuner_strength;

	handle->m_device_ctt2.tuner_cfg.tuner_sleep			 = tuner_config->tuner_sleep;
	handle->m_device_ctt2.tuner_cfg.tuner_wakeup		 = tuner_config->tuner_wakeup;

	return ret;
}


MT_FE_RET _mt_fe_dmd_init_ct8k_b(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if(handle->m_device_ctt2.tuner_cfg.tuner_init != NULL)
	{
		ret = handle->m_device_ctt2.tuner_cfg.tuner_init(handle);
	}

	//_mt_fe_dmd_set_reg_b_ct8k(handle, 0xFE, 0x00);
	handle->m_device_ctt2.m_iPageNo = 0;

	return ret;
}


MT_FE_RET _mt_fe_dmd_connect_ct8k_b(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U32 freq_KHz;
	U16 symbol_rate_KSs, qam;
	U8 inverted;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	freq_KHz = handle->m_device_ctt2.input_params.input_freq_kHz;
	symbol_rate_KSs = handle->m_device_ctt2.input_params.symbol_rate_KSs;
	qam = handle->m_device_ctt2.input_params.qam;
	inverted = handle->m_device_ctt2.input_params.inverted;


	if(handle->m_device_ctt2.tuner_cfg.tuner_init_ok == FALSE)
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_init != NULL)
		{
			ret = handle->m_device_ctt2.tuner_cfg.tuner_init(handle);
		}
	}

	if(handle->m_device_ctt2.tuner_cfg.tuner_set != NULL)
	{
		ret = handle->m_device_ctt2.tuner_cfg.tuner_set(handle, freq_KHz, symbol_rate_KSs, 0);
	}

	_mt_fe_dmd_set_demod_ct8k_b(handle);

	_mt_fe_dmd_set_symbol_rate_ct8k_b(handle);

	_mt_fe_dmd_set_QAM_ct8k_b(handle, qam);
	//_mt_fe_dmd_set_ts_output_ct8k_b(handle, handle->m_device_ctt2.ts_out_mode);

	return ret;
}


/***********************************************
 Initialize the internal registers in M88DC2000
************************************************/
MT_FE_RET _mt_fe_dmd_set_demod_ct8k_b(MT_FE_CT8K_Device_Handle handle)
{
	return MtFeErr_Ok;
}

/***********************************************
   Set symbol rate
   sym:		symbol rate, unit: KBaud
   xtal:	unit, KHz
************************************************/
MT_FE_RET _mt_fe_dmd_set_symbol_rate_ct8k_b(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	return ret;
}

/***********************************************
   Set the type of MPEG/TS interface
   type: 1, serial format; 2, parallel format; 0, common interface
************************************************/
MT_FE_RET _mt_fe_dmd_set_ts_output_ct8k_b(MT_FE_CT8K_Device_Handle handle, MT_FE_TS_OUT_MODE ts_mode)
{
	MT_FE_RET ret = MtFeErr_Ok;


	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	return ret;
}


/***********************************************
   Set QAM mode
   Qam: QAM mode, 16, 32, 64, 128, 256
************************************************/
MT_FE_RET _mt_fe_dmd_set_QAM_ct8k_b(MT_FE_CT8K_Device_Handle handle, U16 qam)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if(qam == 256)
	{
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A3, 0x09);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x081, 0xC4);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A5, 0x80);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B5, 0x00);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B6, 0xBD);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B7, 0xA1);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A6, 0x67);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x262, 0x20);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21C, 0x30);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x250, 0x0D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x251, 0xCD);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x252, 0xE0);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x253, 0x05);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x254, 0xA7);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x255, 0xFF);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x256, 0xED);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x257, 0x5B);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x258, 0xAE);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x259, 0xE6);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25A, 0x3D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25B, 0x0F);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25C, 0x0D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25D, 0xEA);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25E, 0xF2);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25F, 0x51);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x260, 0xF5);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x261, 0x06);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21A, 0x01);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x546, 0x40);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x210, 0x26);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x211, 0xF6);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x212, 0x84);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x213, 0x02);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x502, 0x01);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x121, 0x04);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x122, 0x04);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52E, 0x10);

		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x121, 0x20);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x122, 0x04);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0d8, 0x30);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21a, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52b, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x526, 0x03);	// 0x01); @ 190508
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52e, 0x10);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x268, 0x01);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x539, 0x01);	// added @ 190614
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x548, 0x13);	// 0x66 @ 190614	// 0x55 @ 180822
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x549, 0x01);


		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A7, 0x40);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0xCA);
	}
	else	// qam == 64
	{
		//A, 64Qam :
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A3, 0x09);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x081, 0xC4);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A5, 0x80);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B5, 0x00);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B6, 0xBD);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0B7, 0xA1);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A6, 0x67);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x262, 0x20);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21C, 0x30);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x250, 0x0D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x251, 0xCD);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x252, 0xE0);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x253, 0x05);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x254, 0xA7);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x255, 0xFF);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x256, 0xED);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x257, 0x5B);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x258, 0xAE);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x259, 0xE6);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25A, 0x3D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25B, 0x0F);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25C, 0x0D);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25D, 0xEA);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25E, 0xF2);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x25F, 0x51);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x260, 0xF5);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x261, 0x06);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21A, 0x01);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x546, 0x40);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x210, 0xC7);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x211, 0xAA);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x212, 0xAB);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x213, 0x02);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x502, 0x00);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x121, 0x04);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x122, 0x04);
		//_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52E, 0x10);

		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x121, 0x04);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x122, 0x04);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0d8, 0x20);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x21a, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52b, 0x00);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x526, 0x01);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52e, 0x10);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x268, 0x01);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x548, 0x32);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x549, 0x01);	// 0x00 @ 180822


		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A7, 0x40);
		_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0xCA);
	}

	_mt_fe_dmd_set_page_reg_ct8k(handle, 0x52B, 0x00);

	return ret;
}


/***********************************************
 Soft reset M88DC2000
 Reset the internal status of each funtion block,
 not reset the registers.
************************************************/
MT_FE_RET _mt_fe_dmd_soft_reset_ct8k_b(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0x00);
	_mt_fe_dmd_set_page_reg_ct8k(handle, 0x0A4, 0xCA);

	return ret;
}


MT_FE_RET _mt_fe_dmd_get_statistics_ct8k_b(MT_FE_CT8K_Device_Handle handle,
										  U32 *agc_lock,
										  U32 *timing_lock,
										  U32 *dagc_lock,
										  U32 *carrier_lock,
										  U32 *chip_lock
										 )
{
	U8 tmp = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	//0x52c : InRcvrSm state output                                    [R/W_00]    [0x00]
	//bit 7     : TS_LOCK
	//bit 6     : FEC_LOCK
	//bit 5     : MSE_LOCK
	//bit 4     : STR_LOCK
	//bit 3     : Reserved
	//bit 2 ~ 0 : InRcvrState[2:0]        --- system state machine, also need display for debug
	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x52C, &tmp);

	*agc_lock = (tmp >> 7) & 0x01;
	*timing_lock = (tmp >> 6) & 0x01;
	*dagc_lock = (tmp >> 5) & 0x01;
	*carrier_lock = (tmp >> 4) & 0x01;

	*chip_lock = ((tmp & 0xF0) == 0xF0) ? 1 : 0;

	return MtFeErr_Ok;
}

/***********************************************
   Get lock status
   returned 1 when locked;0 when unlocked
************************************************/
MT_FE_RET _mt_fe_dmd_get_lock_state_ct8k_b(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *lock_status)
{
	MT_FE_RET ret = MtFeErr_Ok;

	U8 tmp;

	*lock_status = MtFeLockState_Unlocked;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x52C, &tmp);

	if((tmp & 0xF0) == 0xF0)
		*lock_status = MtFeLockState_Locked;

	return ret;
}


/***********************************************
   Get BER (bit error rate)
************************************************/
MT_FE_RET _mt_fe_dmd_get_ber_ct8k_b(MT_FE_CT8K_Device_Handle handle, U32 *error_bits, U32 *total_bits)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 tmp = 0;
	int i_index = 0;
	U16 qam, total = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	*error_bits = 0;
	*total_bits = 200000;

	qam = handle->m_device_ctt2.input_params.qam;

	/* read register address 0x522 */
	for(i_index = 0; i_index < CT8K_J83B_SNR_SMOOTH_CNT; i_index ++)
	{
		_mt_fe_dmd_get_page_reg_ct8k(handle, 0x522, &tmp);
		total += tmp;
	}

	if(CT8K_J83B_SNR_SMOOTH_CNT > 0)
	{
		tmp = total / CT8K_J83B_SNR_SMOOTH_CNT;
	}

	if(qam == 256)		// 256 QAM
	{
		*total_bits = 78848 * 32 * 2;

		if(tmp <= 0x2F)
		{
			*error_bits = 0;
		}
		else
		{
			*error_bits = tmp - 0x2F;
		}

		if(tmp <= 0x4F)
		{
			*error_bits *= 2;
		}
		else if(tmp <= 0x5F)
		{
			*error_bits *= 4;
		}
		else if(tmp <= 0x61)
		{
			*error_bits *= 6;
		}
		else if(tmp <= 0x63)
		{
			*error_bits *= 8;
		}
		else if(tmp <= 0x65)
		{
			*error_bits *= 16;
		}
		else if(tmp <= 0x6F)
		{
			*error_bits *= (tmp - 0x65 + 24);
		}
		else
		{
			*error_bits *= 64;
		}
	}
	else				//  64 QAM
	{
		*total_bits = 53760 * 32;
	
		if(tmp <= 0x3F)
		{
			*error_bits = 0;
		}
		else
		{
			*error_bits = tmp - 0x3F;
		}
		
		if(tmp <= 0x5C)
		{
			*error_bits *= 2;
		}
		else if(tmp <= 0x63)
		{
			*error_bits *= 3;
		}
		else if(tmp <= 0x6C)
		{
			*error_bits *= 4;
		}
		else if(tmp <= 0x6F)
		{
			*error_bits *= 5;
		}
		else if(tmp <= 0x73)
		{
			*error_bits *= 6;
		}
		else if(tmp <= 0x77)
		{
			*error_bits *= 7;
		}
		else if(tmp <= 0x7F)
		{
			*error_bits *= 8;
		}
		else if(tmp <= 0x8F)
		{
			*error_bits *= (9 + (tmp - 0x7F) / 2);
		}
		else if(tmp <= 0x9F)
		{
			*error_bits *= (20 + (tmp - 0x8F) / 4);
		}
		else
		{
			*error_bits *= 32;
		}
	}


	return ret;
}

/***********************************************
   Get SNR (signal noise ratio), in 0.1dB, signal_snr = dB * 10
************************************************/
MT_FE_RET _mt_fe_dmd_get_snr_ct8k_b(MT_FE_CT8K_Device_Handle handle, U16 *signal_snr)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 tmp = 0;
	int i_index = 0;
	U16 qam, total = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	qam = handle->m_device_ctt2.input_params.qam;

	/* read register address 0x522 */
	for(i_index = 0; i_index < CT8K_J83B_SNR_SMOOTH_CNT; i_index ++)
	{
		_mt_fe_dmd_get_page_reg_ct8k(handle, 0x522, &tmp);
		total += tmp;
	}

	if(CT8K_J83B_SNR_SMOOTH_CNT > 0)
	{
		tmp = total / CT8K_J83B_SNR_SMOOTH_CNT;
	}

	i_index = (int)tmp;

	/*out of bound protection*/
	if(i_index < 0)
		i_index = 0;
	else if(i_index > 255)
		i_index = 255;

	if (qam == 64)
	{
		*signal_snr = mse2snr_q64[i_index];
	}
	else if (qam == 256)
	{
		*signal_snr = mse2snr_q256[i_index];
	}

	/*read register 0x541 to check lock */
	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x541, &tmp);
	if (((tmp & 0xc0) == 0x80) && ((tmp & 0x30) == 0x20))
	{
		/* signal is locked */
		return ret;
	}

	/*signal is not locked, return 0 */
	*signal_snr = 0;

	return ret;
}


MT_FE_RET _mt_fe_dmd_get_quality_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 tmp = 0;
	U16 total = 0, i_index = 0;
	MT_FE_LOCK_STATE mLockState = MtFeLockState_Undef;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_lock_state_ct8k_b(handle, &mLockState);
	if(mLockState != MtFeLockState_Locked)
	{
		*p_percent = 0;
		return MtFeErr_Fail;
	}

	/* read register address 0x522 */
	for(i_index = 0; i_index < CT8K_J83B_SNR_SMOOTH_CNT; i_index ++)
	{
		_mt_fe_dmd_get_page_reg_ct8k(handle, 0x522, &tmp);
		total += tmp;
	}

	if(CT8K_J83B_SNR_SMOOTH_CNT > 0)
	{
		tmp = total / CT8K_J83B_SNR_SMOOTH_CNT;
	}

	if(tmp >= 0x84)
	{
		*p_percent = 0;
	}
	else if(tmp > 0x20)
	{
		*p_percent = 0x84 - tmp;
	}
	else
	{
		*p_percent = 100;
	}

	/*read register 0x541 to check lock */
	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x541, &tmp);
	if (((tmp & 0xc0) == 0x80) && ((tmp & 0x30) == 0x20))
	{
		/* signal is locked */
		return ret;
	}

	*p_percent = 0;

	return ret;
}


/***********************************************
   Get signal strength
************************************************/
MT_FE_RET _mt_fe_dmd_get_strength_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *signal_strength)
{
#if 1
	S32	level_rel = 0;
	S8	tuner_strength = 0;
	S8	level_ref = 0;

	if (handle->m_device_ctt2.input_params.qam == 256)
		level_ref = -65;
	else
		level_ref = -71;


	if(handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}
	else
		tuner_strength = 0;

	level_rel = tuner_strength - level_ref;

	if(level_rel <= -15)
		*signal_strength = 0;
	else if(level_rel < 0)
		*signal_strength = (U8)((level_rel + 15) * 2 / 3);
	else if(level_rel < 20)
		*signal_strength = (U8)(level_rel * 4 + 10);
	else if(level_rel < 35)
		*signal_strength = (U8)((level_rel - 20) * 2 / 3 + 90);
	else
		*signal_strength = 100;

	return MtFeErr_Ok;
#else
	MT_FE_RET ret = MtFeErr_Ok;
	U8 SignalStrength = 0;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	if (handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
	{
		S8 strength = 0;

		handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &strength);

		SignalStrength = strength;
	}

	*signal_strength = SignalStrength;

	return ret;
#endif
}


MT_FE_RET _mt_fe_dmd_get_offset_ct8k_b(MT_FE_CT8K_Device_Handle handle, S32 *freq_offset_KHz, S32 *symbol_rate_offset_KSs)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	*freq_offset_KHz = 0;
	*symbol_rate_offset_KSs = 0;

	return ret;
}


MT_FE_RET _mt_fe_dmd_get_symbol_rate(MT_FE_CT8K_Device_Handle handle, U16 *symbol_rate_KSs)
{
	return MtFeErr_Ok;
}


// fix 113084 start
MT_FE_RET _mt_fe_dmd_get_frames_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *good_frames, U8 *bad_frames, U8 *total_frames)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x542, total_frames);
	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x543, bad_frames);
	_mt_fe_dmd_get_page_reg_ct8k(handle, 0x544, good_frames);

	return ret;
}
// fix 113084 end


MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_b(U8 *version_no)
{
	*version_no		 = 5;

	return MtFeErr_Ok;
}

