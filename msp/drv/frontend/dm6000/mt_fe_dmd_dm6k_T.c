/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2015                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_dm6k_t.c
*
* Current version:	00.42
*
* Description:		M88DM6000 IC Driver For DVBT Mode.
*
* Log:	Description		Version		Date			Author
---------------------------------------------------------------------
*		Create			00.00		2014.01.18		BJ.Wang
*		Modify			00.10		2014.06.26		BJ.Wang
*		Modify			00.20		2015.01.15		BJ.Wang
*		Modify			00.30		2016.07.04		BJ.Wang
*		Modify			00.40		2017.08.28		BJ.Wang
*		Modify			00.42		2018.03.09		YZ.Huang
******************************************************************************************************/

#include <linux/printk.h>

#include "mt_fe_dmd_dm6k_T.h"
#include "mt_fe_i2c_dm6k.h"
#include "mt_fe_dmd_fw_dm6k_T.h"

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


MT_FE_RET _mt_fe_dmd_get_reg_t(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 *p_data)
{
	return _mt_fe_dm6k_get_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, p_data);
}

MT_FE_RET _mt_fe_dmd_set_reg_t(MT_FE_DM6K_Device_Handle handle, U8 reg_addr, U8 reg_data)
{
	return _mt_fe_dm6k_set_reg(handle->m_device_ctt2.dmd_dev_addr, reg_addr, reg_data);
}

/*
** Function: mt_fe_dmd_get_driver_version_t_dm6k
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
MT_FE_RET mt_fe_dmd_get_driver_version_dm6k_t(U8* p_version)
{
	*p_version = 42;		/* driver version number */

	return MtFeErr_Ok;
}

static MT_FE_RET _mt_fe_dmd_init_reg_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret;
	S32	i = 0, size = 0;

	size = sizeof(reg_tbl_init_def) / 2;

	for (i = 0; i < size; i ++)
	{
		ret = _mt_fe_dmd_set_reg_t(handle, reg_tbl_init_def[i][0], reg_tbl_init_def[i][1]);

		if (ret != MtFeErr_Ok)
			return ret;
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_download_fw_dm6k_t
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
MT_FE_RET _mt_fe_dmd_download_fw_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U32 i = 0, firm_size = 0, firm_last_size = 0, w_size = 0;
	U8  tmp[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	const U8* p_fw = m88dm6k_t_fm;

	if(handle->m_device_ctt2.demod_type != MtFeType_DVBT)
	{
		return MtFeErr_NoMatch;
	}

	if ((handle->m_device_ctt2.mcu_status != 1) && (handle->m_device_ctt2.mcu_status != 3))
	{
		firm_size = sizeof(m88dm6k_t_fm) / 16;
		firm_last_size = sizeof(m88dm6k_t_fm) % 16;

		_mt_fe_dmd_set_reg_t(handle, 0xb2, 0x01);
		_mt_sleep_dm6k(1);

		tmp[0] = 0xb0;
		for (i = 0; i < firm_size; i ++)
		{
			for(w_size = 1; w_size < 17; w_size ++)
			{
				tmp[w_size] = *(p_fw ++);
			}
			ret = _mt_fe_dmd_write_dm6k(handle->m_device_ctt2.dmd_dev_addr, tmp, 17);
			if (ret != MtFeErr_Ok)
				break;
		}

		for(w_size = 1; w_size < (firm_last_size + 1); w_size ++)
		{
			tmp[w_size] = *(p_fw ++);
		}
		ret = _mt_fe_dmd_write_dm6k(handle->m_device_ctt2.dmd_dev_addr, tmp, (U16)(firm_last_size + 1));

		ret =_mt_fe_dmd_set_reg_t(handle, 0xb2, 0x00);
		if (ret != MtFeErr_Ok)
		{
			mt_fe_print(("MT: _mt_fe_dmd_download_fw_dm6k_t FAILED!    [ret = %d]\n", ret));
			return MtFeErr_I2cErr;
		}

		if (handle->m_device_ctt2.mcu_status == 2)
			handle->m_device_ctt2.mcu_status = 3;
		else
			handle->m_device_ctt2.mcu_status = 1;
	}

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_set_IF_dm6k_t
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
MT_FE_RET _mt_fe_dmd_set_IF_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 IF_Khz)
{
	U32 IF_set = 0, FC_INT = 0, IF_Center = 0;

	IF_Center = IF_Khz;

	if(IF_Center >= 28800)
		IF_set = IF_Center - 28800;
	else
		IF_set = IF_Center;

	FC_INT = IF_set * 9321;

	_mt_fe_dmd_set_reg_t(handle, 0xf0, (U8)FC_INT);
	_mt_fe_dmd_set_reg_t(handle, 0xf1, (U8)(FC_INT >> 8));
	_mt_fe_dmd_set_reg_t(handle, 0xf2, (U8)(FC_INT >> 16));
	_mt_fe_dmd_set_reg_t(handle, 0xf3, (U8)((FC_INT >> 24) & 0x0f));

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_dm6k_t
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
MT_FE_RET _mt_fe_dmd_set_output_mode_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	U8	tmp1 = 0, tmp2 = 0;
	MT_FE_TS_OUT_MODE	mode = MtFeTsOutMode_Common;

	mode = handle->m_device_ctt2.ts_out_mode;

	_mt_fe_dmd_get_reg_t(handle, 0xaa, &tmp1);
	_mt_fe_dmd_get_reg_t(handle, 0xab, &tmp2);

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

	_mt_fe_dmd_set_reg_t(handle, 0xaa, tmp1);
	_mt_fe_dmd_set_reg_t(handle, 0xab, tmp2);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_bw_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	MT_FE_BANDWIDTH bw = MtFeBandwidth_8M;
	U8	tmp = 0;

	bw = handle->m_device_ctt2.input_params.demod_bandwidth;
	switch(bw)
	{
		case MtFeBandwidth_8M:
			_mt_fe_dmd_set_reg_t(handle, 0xf4, 0x51);
			_mt_fe_dmd_set_reg_t(handle, 0xf5, 0x14);
			_mt_fe_dmd_set_reg_t(handle, 0xf6, 0x05);
			_mt_fe_dmd_set_reg_t(handle, 0xf7, 0x9a);
			_mt_fe_dmd_set_reg_t(handle, 0xf8, 0x99);
			_mt_fe_dmd_set_reg_t(handle, 0xf9, 0x99);
			_mt_fe_dmd_set_reg_t(handle, 0xfa, 0xc9);
			_mt_fe_dmd_set_reg_t(handle, 0xc0, 0x54);
			_mt_fe_dmd_set_reg_t(handle, 0xc1, 0x00);
			_mt_fe_dmd_set_reg_t(handle, 0xc2, 0x41);

			_mt_fe_dmd_get_reg_t(handle, 0xb3, &tmp);
			tmp = (U8)(tmp & 0x80);

			if (tmp == 0x80)
			{
				_mt_fe_dmd_set_reg_t(handle, 0x8a, 0x80);
				_mt_fe_dmd_set_reg_t(handle, 0x97, 0x60);
			}
			else
			{
				_mt_fe_dmd_set_reg_t(handle, 0x8a, 0x84);
				_mt_fe_dmd_set_reg_t(handle, 0x97, 0x87);
			}
			break;
		case MtFeBandwidth_7M:
			_mt_fe_dmd_set_reg_t(handle, 0xf4, 0xc7);
			_mt_fe_dmd_set_reg_t(handle, 0xf5, 0x71);
			_mt_fe_dmd_set_reg_t(handle, 0xf6, 0x04);
			_mt_fe_dmd_set_reg_t(handle, 0xf7, 0x66);
			_mt_fe_dmd_set_reg_t(handle, 0xf8, 0x66);
			_mt_fe_dmd_set_reg_t(handle, 0xf9, 0x66);
			_mt_fe_dmd_set_reg_t(handle, 0xfa, 0xe6);
			_mt_fe_dmd_set_reg_t(handle, 0xc0, 0x55);
			_mt_fe_dmd_set_reg_t(handle, 0xc1, 0x00);
			_mt_fe_dmd_set_reg_t(handle, 0xc2, 0x81);
			break;
		case MtFeBandwidth_6M:
			_mt_fe_dmd_set_reg_t(handle, 0xf4, 0x3d);
			_mt_fe_dmd_set_reg_t(handle, 0xf5, 0xcf);
			_mt_fe_dmd_set_reg_t(handle, 0xf6, 0x13);
			_mt_fe_dmd_set_reg_t(handle, 0xf7, 0xcd);
			_mt_fe_dmd_set_reg_t(handle, 0xf8, 0xcc);
			_mt_fe_dmd_set_reg_t(handle, 0xf9, 0xcc);
			_mt_fe_dmd_set_reg_t(handle, 0xfa, 0x0c);
			break;
		default:	//MtFeBandwidth_Undef Reserved

			break;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_init_dm6k_t
**
**	DESCRIPTION:
**		initialize DVB-T mode of M88DM6000
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_init_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	U32	if_khz = 0;

	_mt_fe_dmd_init_reg_dm6k_t(handle);

	_mt_fe_dmd_set_reg_t(handle, 0x19, 0x32);
	_mt_fe_dmd_set_reg_t(handle, 0x1d, 0xb8);

	_mt_fe_dmd_download_fw_dm6k_t(handle);
	_mt_fe_dmd_set_output_mode_dm6k_t(handle);
	_mt_fe_dmd_set_bw_dm6k_t(handle);

	if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_MxL603)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t(handle, if_khz);
	}
	else if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t(handle, if_khz);
	}
	else if (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800)
	{
		if_khz = 5000;
		_mt_fe_dmd_set_IF_dm6k_t(handle, if_khz);
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
**		mt_fe_dmd_hard_reset_dm6k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_hard_reset_dm6k_t(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_dm6k_t
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET _mt_fe_dmd_soft_reset_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	_mt_fe_dmd_set_reg_t(handle, 0xad, 0x52);
	_mt_fe_dmd_set_reg_t(handle, 0xdd, 0x2f);

	_mt_fe_dmd_set_reg_t(handle, 0xb2, 0x01);
	_mt_fe_dmd_set_reg_t(handle, 0x00, 0x01);
	_mt_fe_dmd_set_reg_t(handle, 0x00, 0x00);
	_mt_sleep_dm6k(5);
	_mt_fe_dmd_set_reg_t(handle, 0xb2, 0x00);

	soft_reset_flag = 1;

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
MT_FE_RET _mt_fe_dmd_connect_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	U8	reg_8a = 0;
	//int i;

	handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
	handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
	handle->m_device_ctt2.cell_info.usCellId = 0;

	printk("----_mt_fe_dmd_connect_dm6k_t(), freq = %d, bw = %d, plp_No = %d\n",
			handle->m_device_ctt2.input_params.input_freq_kHz,
			handle->m_device_ctt2.input_params.demod_bandwidth,
			handle->m_device_ctt2.input_params.plp_No);

	mt_fe_dmd_set_hierarchy_dm6k_t(handle,handle->m_device_ctt2.input_params.plp_No);

	if (handle->m_device_ctt2.tuner_cfg.tuner_set != NULL)
	{
		mt_fe_i2c_repeat_enable_dm6k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_set(handle, handle->m_device_ctt2.input_params.input_freq_kHz, 0, 0);
		mt_fe_i2c_repeat_disable_dm6k(handle);
	}

	_mt_fe_dmd_set_bw_dm6k_t(handle);

	_mt_fe_dmd_get_reg_t(handle, 0x8a, &reg_8a);
	if(reg_8a & 0x40)
	{
		reg_8a = (U8)((reg_8a & 0x0f) | 0x80);
		_mt_fe_dmd_set_reg_t(handle, 0x8a, reg_8a);
	}
	_mt_fe_dmd_soft_reset_dm6k_t(handle);

#if 0
	printk("----_mt_fe_dmd_connect_dm6k_t(), demod register list:\n");
	for(i = 0; i <= 0xFF; i ++)
	{
		_mt_fe_dmd_get_reg_t(handle, (U8)i, &reg_8a);
		printk("%02x - %02x\n", (U8)i, reg_8a);
	}

	printk("\n");
#endif

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_dm6k_t
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
MT_FE_RET _mt_fe_dmd_get_lock_state_dm6k_t(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	value = 0;

	*p_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_reg_t(handle, 0xb7, &value);

	if(soft_reset_flag && ((value & 0x80) == 0))
	{
		_mt_fe_dmd_get_reg_t(handle, 0xb3, &value);	// DEMOD DEBUG Output
		if ((value != 0xf1) && (value != 0xf2) && (value != 0xf3) && (value != 0xf4) && (value != 0xf5)
			 && (value != 0xf6) && (value != 0xf7) && (value != 0xf8) && (value != 0xf9) && (value != 0xf0)
			  && (value != 0xfb) && (value != 0x0c)&& (value != 0x8c) && (value != 0x0b))
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

	_mt_fe_dmd_get_reg_t(handle, 0xb3, &value);
	if((value == 0x0c))
	{
		*p_state = MtFeLockState_Locked;
	}
	else
	{
		_mt_fe_dmd_get_reg_t(handle, 0xac, &value);
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
		_mt_fe_dmd_get_cell_info_dm6k_t(handle);
	}
	else
	{
		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_set_hierarchy_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 hi_id)
{
	U8 tmp = 0;

	_mt_fe_dmd_get_reg_t(handle, 0xbe, &tmp);
	tmp = (U8)((tmp & 0x7f) | (hi_id << 7));
	_mt_fe_dmd_set_reg_t(handle, 0xbe, tmp);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_get_hierarchy_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *hi_num)
{
	MT_FE_LOCK_STATE lock_state = MtFeLockState_Undef;
	U8 tmp = 0;

	_mt_fe_dmd_get_lock_state_dm6k_t(handle, &lock_state);

	if(lock_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
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
**		_mt_fe_dmd_get_snr_dm6k_t
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
MT_FE_RET _mt_fe_dmd_get_snr_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *p_snr)
{
	#define MSE_LOOP	32
	U8	tmp = 0;
	U8	i = 0;
	U32	mse = 0;
	U32	snr = 0;
	U8 	qam_mod = 0,mod = 0;
	U8	wait_loop = 0;

	*p_snr = 0;

	_mt_fe_dmd_get_reg_t(handle, 0x95, &tmp);
	while((tmp & 0x04) == 0)// TPS_INFO_VALID flag
	{
		wait_loop ++;
		_mt_sleep_dm6k(5);
		if (wait_loop > 40)
			break;
	}

	if (wait_loop > 40)
		return MtFeErr_TimeOut;

	_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
	qam_mod = (U8)(( tmp >> 2) & 0x03);

	for (i = 0; i < MSE_LOOP; i ++)
	{
		for(mod = 0; mod <= qam_mod; mod ++)
		{
			_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
			tmp = (U8)((tmp & 0x07) | (2 * mod));
			_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

			_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
			if (tmp > 9)
				mse += 900;
			else
				mse += tmp * 100;

			_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
			tmp = (U8)((tmp & 0x07) | (2 * mod + 1));
			_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

			_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
			if (tmp > 9)
				mse += 900;
			else
				mse += tmp * 100;
		}
	}

	snr = mse / ((2 * qam_mod + 2) * MSE_LOOP);

	if(qam_mod == 0)///QPSK
	{
		if(snr >= 430)
			snr = 30 * 1000;
		else
			snr = 9 * 100 * 100 - 140 * snr;
	}
	else if(qam_mod == 1)///16QAM
	{
		if(snr >= 610)
			snr = 65 * 1000;
		else if(snr >= 460)
			snr = 2073 * 100 - 233 * snr;
		else
			snr = 1600 * 100 - 131 * snr;
	}
	else if(qam_mod == 2)///64QAM
	{
		if(snr >= 610)
			snr = 130 * 1000;
		else if(snr >= 520)
			snr = 3550 * 100 - 375 * snr;
		else
			snr = 2400 * 100 - 154 * snr;
	}

	*p_snr = (U8)(snr / (10 * 100));

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_ber_dm6k_t
**
**	DESCRIPTION:
**		get the byte error ratio
**
**	IN:
**		none.
**
**	OUT:
**		*p_err_cnt	-	error byte counter
**		*p_pkt_cnt	-	totale byte counter
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_ber_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt)
{
#define PKT_CNT_MODE		3		/*	can be 0, 1, 2 or 3	*/
#define PKT_CNT_MAX			(1 << (PKT_CNT_MODE * 2 + 12))

	static U16	last_ber_cnt = 0;
	U8			tmp, tmp1;

	*p_err_cnt = 0;
	*p_total_cnt = PKT_CNT_MAX;

	_mt_fe_dmd_get_reg_t(handle, 0xa8, &tmp);
	_mt_fe_dmd_get_reg_t(handle, 0xb3, &tmp1);
	if ((tmp & 0x80) == 0 || tmp1 != 0x0c)
	{
		last_ber_cnt = 0;
		*p_err_cnt   = 0;

		_mt_fe_dmd_set_reg_t(handle, 0xdd, 0x2f);
		_mt_fe_dmd_set_reg_t(handle, 0xad, 0x52);
		return MtFeErr_Fail;
	}

	_mt_fe_dmd_set_reg_t(handle, 0xdd, 0xff);
	_mt_fe_dmd_get_reg_t(handle, 0xad, &tmp);
	if ((tmp & 0x40) == 0x40)///frist read ,set fail
	{
		_mt_fe_dmd_set_reg_t(handle, 0xad, 0x00 | PKT_CNT_MODE);
		_mt_fe_dmd_set_reg_t(handle, 0xad, 0x80 | PKT_CNT_MODE);
		return MtFeErr_Fail;
	}
	else
	{
		if ((tmp & 0x80) == 0x00)
		{
			_mt_fe_dmd_get_reg_t(handle, 0xae, &tmp);
			last_ber_cnt = tmp;
			_mt_fe_dmd_get_reg_t(handle, 0xaf, &tmp);
			last_ber_cnt += (U16)((tmp << 8));

			_mt_fe_dmd_set_reg_t(handle, 0xad, 0x80 | PKT_CNT_MODE);//0xb0 | PKT_CNT_MODE);
		}
		else
			return MtFeErr_Fail;
	}

	*p_err_cnt = last_ber_cnt;


	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		_mt_fe_dmd_get_quality_dm6k_t
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
MT_FE_RET _mt_fe_dmd_get_quality_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *quality)
{
	#define MSE_LOOP	32

	U8	tmp = 0;
	U8	i = 0;
	U32	mse = 0;
	U8 	mod = 0;
	S16	quality_value = 0;

#if 0
	_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
	tmp &= 0x07;
	_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

	_mt_fe_dmd_get_reg_t(handle, 0x95, &tmp);
	if((tmp & 0x04) != 0)		// TPS_INFO_VALID flag
	{
		_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
		mod = (U8)((tmp >> 2) & 0x03);

		mse = 0;
		if (mod == 0)
		{
			for (i = 0; i < MSE_LOOP; i ++)
			{
				_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
				if (tmp > 5)
					mse += 10;
				else
					mse += (80 - tmp * 10);
			}

			quality_value = (S16)((((mse / MSE_LOOP) - 10) * 167) / 100);
		}
		else if (mod == 1)
		{
			for (i = 0; i < MSE_LOOP; i ++)
			{
				_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
				_mt_sleep_dm6k(4);

				if (tmp > 14)
					mse += 120;
				else if (tmp > 13)
					mse += 130;
				else if (tmp > 12)
					mse += 140;
				else if (tmp > 11)
					mse += 150;
				else if (tmp > 10)
					mse += 150;
				else if (tmp > 7)
					mse += 40;
				else
					mse += (170 - tmp * 110 / 7);
			}

			quality_value = (S16)((((mse / MSE_LOOP) - 80) * 125) / 100);
		}
		else if (mod == 2)
		{
			for (i = 0; i < MSE_LOOP; i ++)
			{
				_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
				if (tmp > 7)
					mse += 140;
				else
					mse += (230 - tmp * 110 / 7);
			}

			quality_value = (S16)((((mse / MSE_LOOP) - 130) * 125) / 100);
		}
	}

	if(quality_value > 100)
		*quality = 100;
	else if(quality_value < 0)
		*quality = 0;
	else
		*quality = (U8)quality_value;
#endif

	S32	level_rel = 0;
	S8	tuner_strength = 0;
	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;
	MT_FE_RET ret = MtFeErr_Ok;

	_mt_fe_dmd_get_lock_state_dm6k_t(handle, &q_state);
	if(q_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
		tmp &= 0x07;
		_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

		_mt_fe_dmd_get_reg_t(handle, 0x95, &tmp);
		if((tmp & 0x04) != 0)		// TPS_INFO_VALID flag
		{
			_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
			mod = (U8)((tmp >> 2) & 0x03);

			mse = 0;
			if (mod == 0)
			{
				for (i = 0; i < MSE_LOOP; i ++)
				{
					_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
					if (tmp > 5)
						mse += 10;
					else
						mse += (80 - tmp * 10);
				}

				quality_value = (S16)((1000 - (80 - (mse / MSE_LOOP)) * 7) / 10);
			}
			else if (mod == 1)
			{
				for (i = 0; i < MSE_LOOP; i ++)
				{
					_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
					_mt_sleep_dm6k(4);

					if (tmp > 14)
						mse += 120;
					else if (tmp > 13)
						mse += 130;
					else if (tmp > 12)
						mse += 140;
					else if (tmp > 11)
						mse += 150;
					else if (tmp > 10)
						mse += 150;
					else if (tmp > 7)
						mse += 40;
					else
						mse += (170 - tmp * 110 / 7);
				}

				quality_value = (S16)((1000 - (170 - (mse / MSE_LOOP)) * 7) / 10);
			}
			else if (mod == 2)
			{
				for (i = 0; i < MSE_LOOP; i ++)
				{
					_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
					if (tmp > 7)
						mse += 140;
					else
						mse += (230 - tmp * 110 / 7);
				}

				quality_value = (S16)((1000 - (230 - (mse / MSE_LOOP)) * 7) / 10);
			}
		}

		if(quality_value > 100)
			*quality = 100;
		else if(quality_value < 0)
			*quality = 0;
		else
			*quality = (U8)quality_value;
	}
	else
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_strength!= NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			ret = handle->m_device_ctt2.tuner_cfg.tuner_strength(handle,&tuner_strength);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}

		level_rel = (tuner_strength + 85) * 2;
		if(level_rel > 45)
			level_rel = 45;
		else if(level_rel < 0)
			level_rel = 0;

		*quality = (U8)level_rel;
	}

	return MtFeErr_Ok;
}


MT_FE_RET _mt_fe_dmd_get_strength_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent)
{
	S8 tuner_strength = 0;

	if(handle->m_device_ctt2.tuner_cfg.tuner_strength != NULL)
	{
		mt_fe_i2c_repeat_enable_dm6k(handle);
		handle->m_device_ctt2.tuner_cfg.tuner_strength(handle, &tuner_strength);
		mt_fe_i2c_repeat_disable_dm6k(handle);
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
		*ssi_percent =(U8)(20 + 3 * (80 + tuner_strength));
	else if ((tuner_strength < -80) && (tuner_strength >= -100))
		*ssi_percent =(U8)(100 + tuner_strength);
	else
		*ssi_percent = 0;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_quality_nordig_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *quality)
{
	#define NORDIG_MSE_LOOP	64//32

	U8	tmp = 0;
	U8	i = 0, wait_loop = 0;
	U32	mse = 0, snr = 0;
	U8 	mod = 0, tp_code = 0, qam_mod = 0;
	S16	quality_value = 0;

	MT_FE_LOCK_STATE q_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_lock_state_dm6k_t(handle, &q_state);

	if(q_state == MtFeLockState_Locked)
	{
		_mt_fe_dmd_get_reg_t(handle, 0x95, &tmp);
		while((tmp & 0x04) == 0)// TPS_INFO_VALID flag
		{
			wait_loop ++;
			_mt_sleep_dm6k(5);
			if (wait_loop > 40)
				break;
		}

		if (wait_loop > 40)
		{
			*quality = 0;
			return MtFeErr_TimeOut;
		}

		_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
		qam_mod = (U8)((tmp >> 2) & 0x03);
		_mt_fe_dmd_get_reg_t(handle, 0x92, &tmp);
		tp_code = (U8)(tmp & 0x07);

		for (i = 0; i < NORDIG_MSE_LOOP; i ++)
		{
			for(mod = 0; mod <= qam_mod; mod ++)
			{
				_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
				tmp = (U8)((tmp & 0x07) | (2 * mod));
				_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

				_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
				if (tmp > 9)
					mse += 900;
				else
					mse += tmp*100;

				_mt_fe_dmd_get_reg_t(handle, 0xd0, &tmp);
				tmp = (U8)((tmp & 0x07) | (2 * mod + 1));
				_mt_fe_dmd_set_reg_t(handle, 0xd0, tmp);

				_mt_fe_dmd_get_reg_t(handle, 0xd4, &tmp);
				if (tmp > 9)
					mse += 900;
				else
					mse += tmp * 100;
			}
		}

		snr = mse / ((2 * qam_mod + 2) * NORDIG_MSE_LOOP);

		if(qam_mod == 0)///QPSK
		{
			if(snr >= 430)
				snr = 30 * 1000;
			else
				snr = 9 * 100 * 100 - 140 * snr;

			snr = snr / (10 * 100);

			if(snr < 30)
				quality_value = 0;
			else if(snr <= 90)
				quality_value = (S16)((167 * snr - 5000) / 100);
			else
				quality_value = 100;
		}
		else if(qam_mod == 1)///16QAM
		{
			if(snr >= 610)
				snr = 65 * 1000;
			else if(snr >= 460)
				snr = 2073 * 100 - 233 * snr;
			else
				snr = 1600 * 100 - 131 * snr;

			snr = snr / (10 * 100);

			if(snr < 80)
				quality_value = 0;
			else if(snr <= 160)
				quality_value = (S16)((125 * snr - 10000) / 100);
			else
				quality_value = 100;
		}
		else if(qam_mod == 2)///64QAM
		{
			if(snr >= 610)
				snr = 130 * 1000;
			else if(snr >= 520)
				snr = 3550 * 100 - 375 * snr;
			else
				snr = 2400 * 100 - 154 * snr;

			if(tp_code == 1)
			{
				snr = snr / (10 * 100);
				if(snr < 150)
					quality_value = 0;
				else if(snr <= 210)
					quality_value = (S16)((167 * snr - 25000) / 100);
				else
					quality_value = 100;
			}
			else
			{
				snr = snr / (10 * 100);
				if(snr < 170)
					quality_value = 0;
				else if(snr <= 230)
					quality_value = (S16)((167 * snr - 28380) / 100);
				else
					quality_value = 100;
			}
		}
	}
	else
		*quality = 0;

	if(quality_value > 100)
		*quality = 100;
	else if(quality_value < 0)
		*quality = 0;
	else
		*quality = (U8)quality_value;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_clear_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle)
{
	U8 tmp = 0;

	_mt_fe_dmd_get_reg_t(handle, 0xa8, &tmp);
	tmp = (U8)(tmp & 0xfe);
	_mt_fe_dmd_set_reg_t(handle, 0xa8, tmp);

	_mt_sleep_dm6k(1);

	_mt_fe_dmd_get_reg_t(handle, 0xa8, &tmp);
	tmp = (U8)(tmp | 0x01);
	_mt_fe_dmd_set_reg_t(handle, 0xa8, tmp);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_reset_cci_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *reset_cci)
{
#define RATIO		5

	U32 bch_error_cnt0 = 0;
	U32 bch_cnt0 = 0;
	U8 reg_8a = 0,value =0;

	*reset_cci = 0;

	if((handle->m_device_ctt2.input_params.input_freq_kHz > 745000) && (handle->m_device_ctt2.input_params.input_freq_kHz < 747000))
	{
		_mt_fe_dmd_get_reg_t(handle, 0x8a, &reg_8a);
		if(reg_8a & 0x80)
		{
			_mt_fe_dmd_get_reg_t(handle, 0xb3, &value);
			if(value != 0x0c)
			{
				reg_8a = (U8)((reg_8a & 0x0f) | 0x40);
				_mt_fe_dmd_set_reg_t(handle, 0x8a, reg_8a);
				_mt_fe_dmd_soft_reset_dm6k_t(handle);
				*reset_cci = 1;
			}
			else
			{
				_mt_fe_dmd_get_reg_t(handle, 0xa7, &value);
				bch_error_cnt0 = value << 8;
				_mt_fe_dmd_get_reg_t(handle, 0xa6, &value);
				bch_error_cnt0 = bch_error_cnt0 + value;

				_mt_fe_dmd_get_reg_t(handle, 0xa3, &value);
				bch_cnt0 = value << 8;
				_mt_fe_dmd_get_reg_t(handle, 0xa2, &value);
				bch_cnt0 = bch_cnt0 + value;

				if(bch_cnt0 == 0)
				{
					_mt_fe_dmd_clear_error_pack_dm6k_t(handle);
				}
				else if(((bch_error_cnt0 * 1000) / bch_cnt0) > RATIO)
				{
					reg_8a = (U8)((reg_8a & 0x0f) | 0x40);
					_mt_fe_dmd_set_reg_t(handle, 0x8a, reg_8a);
					_mt_fe_dmd_soft_reset_dm6k_t(handle);
					*reset_cci = 1;
				}
				else
				{
					_mt_fe_dmd_clear_error_pack_dm6k_t(handle);
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

MT_FE_RET _mt_fe_dmd_get_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack)
{
	U32 bch_error_cnt = 0;
	U32 bch_cnt = 0;
	U32	corrected_cnt = 0;
	U8 value =0;

	_mt_fe_dmd_get_reg_t(handle, 0xa7, &value);
	bch_error_cnt = value << 8;
	_mt_fe_dmd_get_reg_t(handle, 0xa6, &value);
	bch_error_cnt = bch_error_cnt + value;

	_mt_fe_dmd_get_reg_t(handle, 0xa3, &value);
	bch_cnt = value << 8;
	_mt_fe_dmd_get_reg_t(handle, 0xa2, &value);
	bch_cnt = bch_cnt + value;

	_mt_fe_dmd_get_reg_t(handle, 0xa5, &value);
	corrected_cnt = value << 8;
	_mt_fe_dmd_get_reg_t(handle, 0xa4, &value);
	corrected_cnt = corrected_cnt + value;

	*total_pack = bch_cnt;
	*error_pack = bch_error_cnt;
	*corrected_pack = corrected_cnt;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_mean_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack)
{
	#define READ_TIMES	10

	U32 bch_error_cnt = 0, last_bch_error_cnt = 0;
	U32 bch_cnt = 0, last_bch_cnt = 0;
	U32 corrected_cnt = 0, last_corrected_cnt = 0;
	U8 value =0, lock_time=75;
	U8 wait_loop = 0, qam_mod = 0, tp_code = 0, fft_mode = 0;
	U8 i = 0;
	MT_FE_LOCK_STATE mt_state = MtFeLockState_Undef;

	_mt_fe_dmd_get_reg_t(handle, 0x95, &value);
	while((value & 0x04) == 0)// TPS_INFO_VALID flag
	{
		wait_loop ++;
		_mt_sleep_dm6k(5);
		if (wait_loop > 40)
			break;
	}

	if (wait_loop > 40)
		return MtFeErr_TimeOut;

	_mt_fe_dmd_get_reg_t(handle, 0x91, &value);
	qam_mod = (U8)((value >> 2) & 0x03);

	_mt_fe_dmd_get_reg_t(handle, 0x92, &value);
	tp_code = (U8)(value & 0x07);

	_mt_fe_dmd_get_reg_t(handle, 0x90, &value);
	fft_mode = (U8)(value & 0xc0);

	_mt_fe_dmd_clear_error_pack_dm6k_t(handle);
	_mt_sleep_dm6k(200);
	_mt_fe_dmd_get_error_pack_dm6k_t(handle, &bch_cnt, &bch_error_cnt, &corrected_cnt);
	last_bch_cnt = bch_cnt;
	last_corrected_cnt = corrected_cnt;
	last_bch_error_cnt = bch_error_cnt;

	if((qam_mod == 0x02) && ((tp_code == 0x01) || (tp_code == 0x02)) && (fft_mode == 0x40))
	{
		for(i = 0; i < (READ_TIMES - 1); i ++)
		{
			_mt_fe_dmd_soft_reset_dm6k_t(handle);
			lock_time = 75;
			mt_state = MtFeLockState_Undef;
			while(lock_time --)
			{
				_mt_fe_dmd_get_lock_state_dm6k_t(handle, &mt_state);
				if(mt_state == MtFeLockState_Locked)
					break;
				_mt_sleep_dm6k(20);
			}

			if(mt_state == MtFeLockState_Locked)
			{
				_mt_fe_dmd_clear_error_pack_dm6k_t(handle);
				_mt_sleep_dm6k(200);
				_mt_fe_dmd_get_error_pack_dm6k_t(handle, &bch_cnt, &bch_error_cnt, &corrected_cnt);
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

MT_FE_RET _mt_fe_dmd_get_cell_info_dm6k_t(MT_FE_DM6K_Device_Handle handle)
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
		_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp1);
		_mt_fe_dmd_get_reg_t(handle, 0x93, &tmp2);
		_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp3);
		_mt_fe_dmd_get_reg_t(handle, 0x93, &tmp4);

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


MT_FE_RET _mt_fe_dmd_get_tps_info_dm6k_t(MT_FE_DM6K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info)
{
	U8	tmp = 0, tmp1 = 0;
	MT_FE_MOD_MODE			tp_qam = MtFeModMode_Undef;
	MT_FE_FFT				tp_fft = MtFeFFTMode_Undef;
	MT_FE_T_GUARD_INTERVAL	tp_guard = MtFeGuarInt_19P128;
	MT_FE_CODE_RATE			tp_code = MtFeCodeRate_Undef;

	_mt_fe_dmd_get_reg_t(handle, 0x90, &tmp);
	tmp1 = tmp >> 6;
	tmp1 &= 0x03;
	if(tmp1 == 0)
		tp_fft = MtFeFFTMode_2K;
	else if(tmp1 == 1)
		tp_fft = MtFeFFTMode_8K;
	else
		tp_fft = MtFeFFTMode_Undef;


	_mt_fe_dmd_get_reg_t(handle, 0x91, &tmp);
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


	_mt_fe_dmd_get_reg_t(handle, 0x92, &tmp);
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

