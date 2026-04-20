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
* File:				mt_fe_dmd_dm6k.c
*
* Current version:	00.71
*
* Description:		M88DM6000 IC Driver.
*
* Log:
*	Description		Version		Date				Author
*--------------------------------------------------------------------------
*	Create			00.10			2014.05.18		BJ.Wang
*	Modify			00.20			2014.09.26		BJ.Wang
*	Modify			00.30			2014.12.23		BJ.Wang
*	Modify			00.40			2015.01.16		BJ.Wang
*	Modify			00.50			2015.02.04		BJ.Wang
*	Modify			00.62			2016.01.15		BJ.Wang
*	Modify			00.63			2016.04.21		BJ.Wang
*	Modify			00.64			2017.03.07		BJ.Wang
*	Modify			00.65			2018.02.28		BJ.Wang
*	Modify			00.70			2018.03.08		BJ.Wang
*	Modify			00.71			2018.04.18		BJ.Wang
***************************************************************************************************************/

#include <linux/printk.h>


#include "mt_fe_def.h"
#include "mt_fe_i2c_dm6k.h"
#include "mt_fe_dmd_dm6k_T.h"
#include "mt_fe_dmd_dm6k_T2.h"

#if MT_FE_DMD_DVBC_SUPPORT
#include "mt_fe_dmd_dm6k_C.h"
#endif

#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_dm6k_S_S2.h"
#endif

/*	FUNCTIN:
**		mt_fe_get_driver_version_dm6k
**
**	DESCRIPTION:
**		get version of this driver
**
**	IN:
**		none
**
**	OUT:
**		*pd_ver
**
**	RETURN:
*/
MT_FE_RET mt_fe_get_driver_version_dm6k(U8 *pd_ver)
{
	MT_FE_RET ret = MtFeErr_Ok;

	#if MT_FE_DMD_DVBS_S2_SUPPORT
		mt_fe_dmd_get_driver_version_dm6k_ss2(&pd_ver[0]);/*s/s2 driver software's version*/
	#else
		pd_ver[0] = 0;
	#endif

	#if MT_FE_DMD_DVBC_SUPPORT
		mt_fe_dmd_get_driver_version_dm6k_c(&pd_ver[1]);/*c driver software's version*/
	#else
		pd_ver[1] = 0;
	#endif


	mt_fe_dmd_get_driver_version_dm6k_t(&pd_ver[2]);/*t  driver software's version*/
	mt_fe_dmd_get_driver_version_dm6k_t2(&pd_ver[4]);/*t2 driver software's version*/

	return ret;
}


/*	FUNCTIN:
**		mt_fe_set_demod_type_dm6k
**
**	DESCRIPTION:
**		set M88DM6000 demod_type
**
**	IN:
**		MT_FE_DM6K_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode
**		MT_FE_TYPE demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_set_demod_type_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode, MT_FE_TYPE demod_type)
{
	U8	reg_04 = 0, reg_06 = 0, reg_07 = 0, reg_08 = 0, reg_03 = 0;

	MT_FE_RET ret = MtFeErr_Ok;

	_mt_fe_dmd_get_reg_dm6k(handle, 0x04, &reg_04);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x06, &reg_06);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x07, &reg_07);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x08, &reg_08);

	if (demod_mode == MtFeDemodMode_DVBCTT2)
	{
		switch (demod_type)
		{
			case MtFeType_DVBC:
				reg_07 = (U8)(reg_07 | 0x60);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				reg_07 = (U8)(reg_07 & 0x9f);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				reg_04 = (U8)(reg_04 | 0x01);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
				reg_07 = (U8)(reg_07 | 0x08);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);

				if (handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel)
				{
					reg_08 = (U8)(reg_08 & 0x0f);
					reg_08 = (U8)(reg_08 | 0x30);
				}
				else if (handle->m_device_ctt2.m_iSerialTSNo == 1)
				{
					reg_08 = (U8)(reg_08 & 0xfc);
					reg_08 = (U8)(reg_08 | 0x03);
				}
				else if (handle->m_device_ctt2.m_iSerialTSNo == 2)
				{
					reg_08 = (U8)(reg_08 & 0xf3);
					reg_08 = (U8)(reg_08 | 0x0c);
				}
				_mt_fe_dmd_set_reg_dm6k(handle, 0x08, reg_08);

				break;

			case MtFeType_DVBT:
				reg_07 = (U8)(reg_07 | 0x90);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				reg_07 = (U8)(reg_07 & 0x6f);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				reg_04 = (U8)(reg_04 | 0x02);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
				reg_07 = (U8)(reg_07 | 0x04);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);

				if (handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel)
				{
					reg_08 = (U8)(reg_08 & 0x0f);
					reg_08 = (U8)(reg_08 | 0x20);
				}
				else if (handle->m_device_ctt2.m_iSerialTSNo == 1)
				{
					reg_08 = (U8)(reg_08 & 0xfc);
					reg_08 = (U8)(reg_08 | 0x02);
				}
				else if (handle->m_device_ctt2.m_iSerialTSNo == 2)
				{
					reg_08 = (U8)(reg_08 & 0xf3);
					reg_08 = (U8)(reg_08 | 0x08);
				}
				_mt_fe_dmd_set_reg_dm6k(handle, 0x08, reg_08);

				break;

			case MtFeType_DVBT2:
				if(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)
				{
					reg_07 = (U8)(reg_07 & 0xfe);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
					reg_04 = (U8)(reg_04 | 0x10);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
					reg_07 = (U8)(reg_07 | 0x01);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);

					if (handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel)
						reg_08 = (U8)(reg_08 & 0x0f);
					else if (handle->m_device_ctt2.m_iSerialTSNo == 1)
						reg_08 = (U8)(reg_08 & 0xfc);
					else if (handle->m_device_ctt2.m_iSerialTSNo == 2)
						reg_08 = (U8)(reg_08 & 0xf3);

					_mt_fe_dmd_set_reg_dm6k(handle, 0x08, reg_08);
				}
				else
				{
					ret = MtFeErr_S2Block;
				}

				break;

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}


		if(ret == MtFeErr_Ok)
		{
			if (handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel)
				reg_06 = (U8)((reg_06 & 0xf8) | 0x01);
			else if (handle->m_device_ctt2.m_iSerialTSNo == 1)
			{
				if ((reg_06 & 0x07) == 0x03)
					reg_06 = (U8)((reg_06 & 0xf8) | 0x04);
				else if ((reg_06 & 0x07) == 0x04)
				{
					//reg_06 = reg_06 ;
				}
				else
					reg_06 = (U8)((reg_06 & 0xf8) | 0x02);
			}
			else if (handle->m_device_ctt2.m_iSerialTSNo == 2)
			{
				if ((reg_06 & 0x07) == 0x02)
					reg_06 = (U8)((reg_06 & 0xfd) | 0x04);
				else if ((reg_06 & 0x07) == 0x04)
				{
					//reg_06 = reg_06 ;
				}
				else
					reg_06 = (U8)((reg_06 &0xf8) | 0x03);
			}
			_mt_fe_dmd_set_reg_dm6k(handle, 0x06, reg_06);

			handle->m_device_ctt2.demod_type = demod_type;
		}
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x17);
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0xa0);
	}
	#if MT_FE_DMD_DVBS_S2_SUPPORT
	else if(demod_mode == MtFeDemodMode_DVBSS2)
	{
		_mt_fe_dmd_get_reg_dm6k(handle, 0x03, &reg_03);
		switch (demod_type)
		{
			case MtFeType_DVBS:
				reg_03 = (U8)(reg_03 | 0x01);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
				reg_03 = (U8)(reg_03 & 0xfe);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
				reg_04 = (U8)(reg_04 | 0x04);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
				reg_07 = (U8)(reg_07 | 0x02);
				_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				break;

			case MtFeType_DVBS2:
				if(handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)
				{
					reg_03 = (U8)(reg_03 | 0x01);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
					reg_03 = (U8)(reg_03 & 0xfe);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
					reg_04 = (U8)(reg_04 | 0x08);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
					reg_07 = (U8)(reg_07 | 0x02);
					_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
				}
				else
				{
					ret = MtFeErr_T2Block;
				}
				break;

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}
		if(ret == MtFeErr_Ok)
		{
			if ((handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Parallel) || (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Common))
			{
				reg_08 = (U8)(reg_08 & 0x0f);
				reg_08 = (U8)(reg_08 | 0x10);
				reg_06 = (U8)((reg_06 & 0xf8) | 0x01);
			}
			else if (handle->m_device_ss2.m_iSerialTSNo == 1)
			{
				reg_08 = (U8)(reg_08 & 0xfc);
				reg_08 = (U8)(reg_08 | 0x01);

				if ((reg_06 & 0x07) == 0x03)
					reg_06 = (U8)((reg_06 & 0xf8) | 0x04);
				else if ((reg_06 & 0x07) == 0x04)
				{
					//reg_06 = reg_06;
				}
				else
					reg_06 = (U8)(reg_06 | 0x02);
			}
			else if (handle->m_device_ss2.m_iSerialTSNo == 2)
			{
				reg_08 = (U8)(reg_08 & 0xf3);
				reg_08 = (U8)(reg_08 | 0x04);

				if ((reg_06 & 0x07) == 0x02)
					reg_06 = (U8)((reg_06 & 0xfd) | 0x04);
				else if ((reg_06 & 0x07) == 0x04)
				{
					//reg_06 = reg_06;
				}
				else
					reg_06 = (U8)(reg_06 | 0x03);
			}
			_mt_fe_dmd_set_reg_dm6k(handle, 0x08, reg_08);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x06, reg_06);

			handle->m_device_ss2.demod_type = demod_type;
		}
	}
	#endif

	return ret;
}

MT_FE_RET mt_fe_system_init_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_TYPE demod_type = MtFeType_Undef;
	U8 reg_03 = 0;

	_mt_fe_dmd_set_reg_dm6k(handle, 0x04, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x05, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x06, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x07, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x08, 0x00);

	_mt_fe_dmd_set_reg_dm6k(handle, 0x09, 0x04);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x1c);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x46);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x18);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x4a);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x17);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0xa0);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x01);

	if(handle->sys_dev_xtal == MtFeXTALMode_27M)
	{
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x08);
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0xa0);
	}

	handle->bTunerBusOn = FALSE;
	mt_fe_dmd_config_default_ctt2_dm6k(handle);

	#if MT_FE_DMD_DVBS_S2_SUPPORT
	demod_type = MtFeType_DVBS;
	mt_fe_dmd_config_default_ss2_dm6k(handle);

	_mt_fe_dmd_get_reg_dm6k(handle, 0x03, &reg_03);
	reg_03 = (U8)(reg_03 | 0x0f);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
	reg_03 = (U8)(reg_03 & 0xf0);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);

	mt_fe_dmd_open_ss2_dm6k(handle, demod_type);
	mt_fe_dmd_close_ss2_dm6k(handle);
	#endif

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_config_default_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_CTT2_Device_Handle handle_ctt2 = &handle->m_device_ctt2;

	if(handle_ctt2 == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle_ctt2->demod_type							 = MtFeType_Undef;
	handle_ctt2->ts_out_mode						 = MtFeTsOutMode_Serial;
	handle_ctt2->m_iSerialTSNo						 = 1;
	handle_ctt2->input_params.demod_bandwidth		 = MtFeBandwidth_8M;

	switch(handle->sys_dev_addr)
	{
		case 0x1C:
		case 0x1E:
			handle->m_device_ctt2.dmd_dev_addr	 = 0x3A;
			break;

		case 0x18:
		default:
			handle->m_device_ctt2.dmd_dev_addr	 = 0x38;
	}
	//handle_ctt2->dmd_dev_addr					 = handle->sys_dev_addr;
	handle->m_device_ctt2.mcu_status			 = 0;/* demod device i2c addres for C&T*/
	handle_ctt2->demod_current_type				 = MtFeType_Undef;
	#if MT_FE_DMD_DVBC_SUPPORT
	handle_ctt2->dvbc_chip_mode					 = 1;// 0: new DC2800        1: new Jazz
	#endif
	handle_ctt2->tuner_cfg.tuner_type		 = MtFeTnId_Undef;
	handle_ctt2->tuner_cfg.tuner_dev_addr	 = 0x00;		/* tuner device i2c addres*/
	handle_ctt2->tuner_cfg.tuner_init_ok	 = 0;			/*tuner init yes or no,0 :no 1:yes*/
	handle_ctt2->tuner_cfg.tuner_open		 = 0;			/*tuner open yes or no,0 :no 1:yes*/
	handle_ctt2->tuner_cfg.tuner_lna_type	 = 0;
	handle_ctt2->tuner_cfg.tuner_sleep		 = NULL;									/* set tuner function */
	handle_ctt2->tuner_cfg.tuner_wakeup		 = NULL;									/* set tuner function */

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_select_tuner_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DM6K_SUPPORTED_TUNER tuner_type)
{
	MT_FE_RET ret = MtFeErr_NoSupportTuner;
	switch(tuner_type)
	{
		case MtFeTN_MxL603:
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_MxL603;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc0;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_Init_MxL603;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_MxL603;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_MxL603;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = NULL;
			ret = MtFeErr_Ok;
			break;
		case MtFeTN_TC3800:
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_TC3800;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc2;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_tc3800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_tc3800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_tc3800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_tc3800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = NULL;
			ret = MtFeErr_Ok;
			break;
		case MtFeTN_TC6800:
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_TC6800;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc6;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_tc6800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_tc6800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_tc6800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_tc6800_tc;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = NULL;
			ret = MtFeErr_Ok;
			break;
		default:
			break;
	}

	return ret;
}
/*	FUNCTIN:
**		mt_fe_dmd_open_ctt2_dm6k
**
**	DESCRIPTION:
**		initialize M88DM6000 dvbc or dvbt or dvbt2
**
**	IN:
**		MT_FE_DM6K_Device_Handle handle
**		MT_FE_TYPE 	demod_type
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == demod_type)
	{
		ret = MtFeErr_Ok;
		return ret;
	}

	if ((demod_type != MtFeType_DVBT) && (demod_type != MtFeType_DVBT2) && (demod_type != MtFeType_DVBT_T2) && (demod_type != MtFeType_DVBC))
	{
		ret = MtFeErr_Fail;
		return ret;
	}

	if (handle->m_device_ss2.demod_current_type != MtFeType_Undef)
	{
#if MT_FE_DMD_DVBS_S2_SUPPORT
		if ((demod_type == MtFeType_DVBT2) && (handle->m_device_ss2.demod_current_type == MtFeType_DVBS2 ))
			mt_fe_dmd_close_ss2_dm6k(handle);
		else if((handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel) && (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Parallel))
			mt_fe_dmd_close_ss2_dm6k(handle);
		else if((handle->m_device_ctt2.ts_out_mode == handle->m_device_ss2.ts_out_mode) && (handle->m_device_ctt2.m_iSerialTSNo == handle->m_device_ss2.m_iSerialTSNo))
			mt_fe_dmd_close_ss2_dm6k(handle);
#endif
	}

	if( handle->m_device_ctt2.demod_current_type != MtFeType_Undef)
	{
		if(demod_type != handle->m_device_ctt2.demod_current_type)
		{
			mt_fe_dmd_close_ctt2_dm6k(handle);
			ret = mt_fe_set_demod_type_dm6k(handle, MtFeDemodMode_DVBCTT2, demod_type);
		}
	}
	else
	{
		ret = mt_fe_set_demod_type_dm6k(handle, MtFeDemodMode_DVBCTT2, demod_type);
	}

	if (ret == MtFeErr_Ok)
	{
		if(demod_type == MtFeType_DVBT2)
			handle->m_device_ctt2.dmd_dev_addr = handle->sys_dev_addr;
		else
		{
			switch(handle->sys_dev_addr)
			{
				case 0x1C:
				case 0x1E:
					handle->m_device_ctt2.dmd_dev_addr = 0x3A;
					break;

				case 0x18:
				default:
					handle->m_device_ctt2.dmd_dev_addr = 0x38;
			}
		}

		if (demod_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_init_dm6k_t(handle);
		else if ((demod_type == MtFeType_DVBT2) || (handle->m_device_ctt2.demod_type == MtFeType_DVBT_T2))
			ret = _mt_fe_dmd_init_dm6k_t2(handle);

		//_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
		//_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x01);//close s2 adc
		handle->m_device_ctt2.demod_current_type = handle->m_device_ctt2.demod_type;

		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
	}

	return ret;
}


/*	FUNCTIN:
**		mt_fe_dmd_close_ctt2_dm6k
**
**	DESCRIPTION:
**		finalize M88DM6000 dvbc or dvbt or dvbt2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	reg_04 = 0;
	U8	reg_06 = 0;
	U8	reg_07 = 0;
	U8	reg_08 = 0;

	_mt_fe_dmd_get_reg_dm6k(handle, 0x04, &reg_04);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x06, &reg_06);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x07, &reg_07);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x08, &reg_08);

	switch (handle->m_device_ctt2.demod_current_type)
	{
	#if MT_FE_DMD_DVBC_SUPPORT
		case MtFeType_DVBC:
			reg_07 = (U8)(reg_07 | 0x60);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_07 = (U8)(reg_07 & 0xf7);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_04 = (U8)(reg_04 & 0xfe);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
			break;
	#endif
		case MtFeType_DVBT:
			reg_07 = (U8)(reg_07 | 0x90);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_07 = (U8)(reg_07 & 0xfb);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_04 = (U8)(reg_04 & 0xfd);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
			break;

		case MtFeType_DVBT2:
		case MtFeType_DVBT_T2:
			reg_07 = (U8)(reg_07& 0xfe);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_04 = (U8)(reg_04 & 0xef);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
			break;

		default:
			ret = MtFeErr_Fail;
			break;
	}
	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel)
			reg_06 = (U8)(reg_06 & 0xf8);
		else if (handle->m_device_ctt2.m_iSerialTSNo == 1)
		{
			if ((reg_06 & 0x07) == 0x04)
				reg_06 = (U8)((reg_06 & 0xf8) | 0x03);
			else
				reg_06 = (U8)(reg_06 & 0xf8);
		}
		else if (handle->m_device_ctt2.m_iSerialTSNo == 2)
		{
			if ((reg_06 & 0x07) == 0x04)
				reg_06 = (U8)((reg_06 & 0xf8) | 0x02);
			else
				reg_06 = (U8)(reg_06 & 0xf8);
		}
		_mt_fe_dmd_set_reg_dm6k(handle, 0x06, reg_06);

		handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
		handle->m_device_ctt2.demod_current_type = MtFeType_Undef;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_ctt2_dm6k
**
**	DESCRIPTION:
**		connect to a special channel for dvbc or dvbt or dvbt2
**
**	IN:
**
**		MT_FE_DM6K_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	ret = mt_fe_dmd_open_ctt2_dm6k(handle, handle->m_device_ctt2.demod_type);


	printk("----mt_fe_dmd_get_lock_state_ctt2_dm6k(), type = %d, ret = %d\n", handle->m_device_ctt2.demod_type, ret);


	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.demod_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_connect_dm6k_t(handle);
		else if ((handle->m_device_ctt2.demod_type == MtFeType_DVBT2) || (handle->m_device_ctt2.demod_type == MtFeType_DVBT_T2))
			ret = _mt_fe_dmd_connect_dm6k_t2(handle);

		#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_ctt2.demod_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_connect_dm6k_c(handle);
		#endif
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_ctt2_dm6k
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_DM6K_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ctt2.demod_current_type != MtFeType_DVBT) && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)\
		&& (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT_T2) && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBC))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_get_lock_state_dm6k_t(handle, p_state);
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
			ret = _mt_fe_dmd_get_lock_state_dm6k_t2(handle, p_state);
		#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_get_lock_state_dm6k_c(handle, p_state);
		#endif
	}

	return ret;
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
MT_FE_RET mt_fe_dmd_hard_reset_dm6k(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_ctt2_dm6k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ctt2.demod_current_type != MtFeType_DVBT) && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)\
		&& (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT_T2) && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBC))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_soft_reset_dm6k_t(handle);
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
			ret = _mt_fe_dmd_soft_reset_dm6k_t2(handle);
	#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_soft_reset_dm6k_c(handle);
	#endif
	}

	return ret;
}


/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_dm6k
**
**	DESCRIPTION:
**		select the serial interface or parallel interface.
**
**	IN:
**		mode	-	MtFeDmdTsOutputMode_Serial
**				-	MtFeDmdTsOutputMode_Parallel
**
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_output_mode_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_set_output_mode_dm6k_t(handle);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_set_output_mode_dm6k_t2(handle);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_set_output_mode_dm6k_c(handle);

	return ret;
}

MT_FE_RET mt_fe_dmd_set_bw_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_set_bw_dm6k_t(handle);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_set_bw_dm6k_t2(handle);
//	else if (handle->demod_type == MtFeType_DVBC)
//		ret = _mt_fe_dmd_set_bw_dm6k_c(handle);

	return ret;
}

MT_FE_RET mt_fe_dmd_get_snr_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_snr)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_snr_dm6k_t(handle, p_snr);
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_snr_dm6k_t2(handle, p_snr);
	#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_snr_dm6k_c(handle, p_snr);
	#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_quality_dm6k_t(handle, p_percent);
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_quality_dm6k_t2(handle, p_percent);
	#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_dm6k_c(handle, p_percent);
	#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_nordig_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_quality_nordig_dm6k_t(handle, p_percent);
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_quality_nordig_dm6k_t2(handle, p_percent);
	#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_dm6k_c(handle, p_percent);
	#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_strength_dm6k_t(handle, ssi_percent);
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_strength_dm6k_t2(handle, ssi_percent);
	#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_strength_dm6k_c(handle, ssi_percent);
	#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_cell_info_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
	{
		ret = _mt_fe_dmd_get_cell_info_dm6k_t(handle);
	}
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
	{
		ret = _mt_fe_dmd_get_cell_info_dm6k_t(handle);
	}
	else
	{
		ret = MtFeErr_NoMatch;
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_inform_t2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_T2_TPS_INFO *tps_info)
{
	if (handle->m_device_ctt2.demod_type != MtFeType_DVBT2)
		return MtFeErr_Fail;
	else
		return _mt_fe_dmd_get_tps_info_dm6k_t2(handle, tps_info);
}

MT_FE_RET mt_fe_dmd_sleep_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	U8	data = 0;

	if(handle->m_device_ctt2.tuner_cfg.tuner_open != 0)
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			handle->m_device_ctt2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if(handle->m_device_ss2.tuner_cfg.tuner_open != 0)
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}
	}
#endif

	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x20);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x09);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x07);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0xa1);

	_mt_fe_dmd_get_reg_dm6k(handle, 0x03, &data);
	data = (U8)(data | 0x20);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x03, data);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_wake_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	U8	data = 0;

	_mt_fe_dmd_get_reg_dm6k(handle, 0x03, &data);
	data = (U8)(data & 0xdf);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x03, data);

	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x07);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x81);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x01);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x00);
	_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x00);

	if(handle->m_device_ctt2.tuner_cfg.tuner_open != 0)
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if(handle->m_device_ss2.tuner_cfg.tuner_open != 0)
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_dm6k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_dm6k(handle);
		}
	}
#endif

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_i2c_repeat_enable_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	data = 0;

	ret =  _mt_fe_dmd_get_reg_dm6k(handle, 0x03, &data);
	data = (U8)(data | 0x10);
	ret =  _mt_fe_dmd_set_reg_dm6k(handle, 0x03, data);

	handle->bTunerBusOn = TRUE;

	return ret;
}

MT_FE_RET mt_fe_i2c_repeat_disable_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	data = 0;

	ret =  _mt_fe_dmd_get_reg_dm6k(handle, 0x03, &data);
	data = (U8)(data & 0xef);
	ret =  _mt_fe_dmd_set_reg_dm6k(handle, 0x03, data);

	handle->bTunerBusOn = FALSE;

	return ret;
}

/****************************************DVBS/S2 APIS**********************************************/

#if MT_FE_DMD_DVBS_S2_SUPPORT

MT_FE_RET mt_fe_dmd_config_default_ss2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	return mt_fe_dmd_ss2_config_default(handle);
}

MT_FE_RET mt_fe_dmd_select_tuner_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DM6K_SUPPORTED_TUNER tuner_type)
{
	MT_FE_RET ret = MtFeErr_NoSupportTuner;
	switch(tuner_type)
	{
		case MtFeTn_TS2022:
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_TS2022;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0)
			{
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0xc2;
			}
			handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_ts2022;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_ts2022;
			handle->m_device_ss2.board_cfg.bIQInverted = FALSE;
			ret = MtFeErr_Ok;
			break;

		case MtFeTn_TS6011:
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_TS6011;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x58;
			handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_ts6011;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_ts6011;
			handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
			ret = MtFeErr_Ok;
			break;

		default:
			break;
	}

	return ret;
}
/*	FUNCTIN:
**		mt_fe_dmd_close_ss2_dm6k
**
**	DESCRIPTION:
**		finalize M88DM6000 dvbs or  dvbs2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ss2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	reg_04 = 0, reg_06 = 0, reg_07 = 0, reg_03 = 0;

	_mt_fe_dmd_get_reg_dm6k(handle, 0x04, &reg_04);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x06, &reg_06);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x07, &reg_07);
	_mt_fe_dmd_get_reg_dm6k(handle, 0x03, &reg_03);

	switch (handle->m_device_ss2.demod_current_type)
	{
		case MtFeType_DVBS:
		case MtFeType_DVBS_S2:
			reg_03 = (U8)(reg_03 | 0x01);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
			reg_07 = (U8)(reg_07 & 0xfd);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_04 = (U8)(reg_04 & 0xfb);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
			break;
		case MtFeType_DVBS2:
			reg_03 = (U8)(reg_03 | 0x01);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x03, reg_03);
			reg_07 = (U8)(reg_07 & 0xfd);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x07, reg_07);
			reg_04 = (U8)(reg_04 & 0xf7);
			_mt_fe_dmd_set_reg_dm6k(handle, 0x04, reg_04);
			break;

		default:
			ret = MtFeErr_Fail;
			break;
	}

	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Parallel)
			reg_06 = (U8)(reg_06 & 0xf8);
		else if (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Serial1)
		{
			if ((reg_06 & 0x07) == 0x04)
				reg_06 = (U8)((reg_06 & 0xfb) | 0x03);
			else
				reg_06 = (U8)(reg_06 & 0xf8);
		}
		else if (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Serial2)
		{
			if ((reg_06 & 0x07) == 0x04)
				reg_06 = (U8)((reg_06 & 0xfb) | 0x02);
			else
				reg_06 = (U8)(reg_06 & 0xf8);
		}
		_mt_fe_dmd_set_reg_dm6k(handle, 0x06, reg_06);

		_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, 0x01);

		//handle->m_device_ss2.demod_type = MtFeType_Undef;
		handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
		handle->m_device_ss2.demod_current_type = MtFeType_Undef;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_open_ss2_dm6k
**
**	DESCRIPTION:
**		initialize M88DM6000 dvbs or dvbs2
**
**	IN:
**		MT_FE_DM6K_Device_Handle		handle
**		MT_FE_TYPE 	demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8 data = 0;

	if (handle->m_device_ss2.demod_current_type == demod_type)
		return ret;

	if ((demod_type != MtFeType_DVBS) && (demod_type != MtFeType_DVBS2) && (demod_type != MtFeType_DVBS_S2))
	{
		ret = MtFeErr_Fail;
		return ret;
	}

	if (handle->m_device_ctt2.demod_current_type != MtFeType_Undef)
	{
		if ((demod_type == MtFeType_DVBS2) && (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2))
			mt_fe_dmd_close_ctt2_dm6k(handle);
		else if((handle->m_device_ctt2.ts_out_mode == MtFeTsOutMode_Parallel) && (handle->m_device_ss2.ts_out_mode == MtFeTsOutMode_Parallel))
			mt_fe_dmd_close_ctt2_dm6k(handle);
		else if((handle->m_device_ctt2.ts_out_mode == handle->m_device_ss2.ts_out_mode) && (handle->m_device_ctt2.m_iSerialTSNo == handle->m_device_ss2.m_iSerialTSNo))
			mt_fe_dmd_close_ctt2_dm6k(handle);
	}

	if (handle->m_device_ss2.demod_current_type != MtFeType_Undef)
	{
		if(demod_type != handle->m_device_ss2.demod_current_type)
		{
			mt_fe_dmd_close_ss2_dm6k(handle);
			ret = mt_fe_set_demod_type_dm6k(handle, MtFeDemodMode_DVBSS2, demod_type);
		}
	}
	else
	{
		ret = mt_fe_set_demod_type_dm6k(handle, MtFeDemodMode_DVBSS2, demod_type);
	}

	if (ret == MtFeErr_Ok)
	{
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1e, 0x01);
		_mt_fe_dmd_get_reg_dm6k(handle, 0x1f, &data);
		data = (U8)(data& 0xfe);
		_mt_fe_dmd_set_reg_dm6k(handle, 0x1f, data);
		ret = mt_fe_dmd_init_dm6k_ss2(handle);
	}

//	if(handle->m_device_ss2.tuner_cfg.tuner_init != NULL)
//	{
//		handle->m_device_ss2.tuner_cfg.tuner_init(handle);
//	}
	handle->m_device_ss2.demod_current_type = handle->m_device_ss2.demod_type;

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_ss2_dm6k
**
**	DESCRIPTION:
**		connect to a special channel for dvbs or dvbs2
**
**	IN:
**
**		MT_FE_DM6K_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_ss2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	mt_fe_dmd_open_ss2_dm6k(handle, handle->m_device_ss2.demod_type);

	if (ret == MtFeErr_Ok)
	{
		U32 iFreqMHz = (handle->m_device_ss2.input_params.input_freq_kHz + 500) / 1000;
		U32 iSymRateKSs = handle->m_device_ss2.input_params.symbol_rate_KSs;
		MT_FE_TYPE dvbs_type = handle->m_device_ss2.demod_type;

		ret = mt_fe_dmd_connect_dm6k_ss2(handle, iFreqMHz, iSymRateKSs, dvbs_type);
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_ss2_dm6k
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_DM6K_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;
	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_lock_state_dm6k_ss2(handle, p_state);
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_ss2_dm6k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_ss2_dm6k(MT_FE_DM6K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_soft_reset_dm6k_ss2(handle);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_sat_quality_dm6k_ss2(handle, p_percent);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	S8 	p_strength = 0;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_strength_dm6k_ss2(handle, &p_strength);
		*ssi_percent =(U8)( p_strength);
	}

	return 	ret;
}

MT_FE_RET mt_fe_dmd_blindscan_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_blindscan_dm6k_ss2(handle, begin_freq_MHz, end_freq_MHz, p_bs_info);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL bs_abort)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_blindscan_abort_dm6k_ss2(handle, bs_abort);

	return ret;
}

MT_FE_RET  mt_fe_dmd_set_LNB_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_set_LNB_dm6k_ss2(handle, is_LNB_enable, is_22k_enable, voltage_type, is_envelop_mode);

	return ret;
}

MT_FE_RET  mt_fe_dmd_DiSEqC_send_tone_burst_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_DiSEqC_send_tone_burst_dm6k_ss2(handle, mode, is_envelop_mode);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_DiSEqC_send_receive_msg_dm6k_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_DiSEqC_send_msg_dm6k_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && (handle->m_device_ss2.demod_current_type != MtFeType_DVBS2)\
		&& (handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_dm6k(handle,  handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_DiSEqC_receive_msg_dm6k_ss2(handle, msg);

	return ret;
}

#endif
