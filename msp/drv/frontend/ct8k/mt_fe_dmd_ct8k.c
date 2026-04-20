/********************************************************************************************/
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
* File:				mt_fe_dmd_ct8k.c
*
* Current version:	00.20
*
* Description:		Symphony2 IC Driver.
*
* Log:
*	Description		Version		Date			Author
*--------------------------------------------------------------------------
*	Create			00.01		2018.06.18		YZ.Huang
*	Modify			00.01		2018.06.26		YZ.Huang
*	Modify			00.04		2018.09.05		YZ.Huang
*	Modify			00.05		2018.09.25		YZ.Huang
*	Modify			00.06		2018.10.17		YZ.Huang
*	Modify			00.07		2018.12.11		YZ.Huang
*	Modify			00.09		2019.01.11		YZ.Huang
*	Modify			00.10		2019.01.16		YZ.Huang
*	Modify			00.11		2019.02.25		YZ.Huang
*	Modify			00.12		2019.03.13		YZ.Huang
*	Modify			00.13		2019.03.14		YZ.Huang
*	Modify			00.14		2019.07.23		YZ.Huang
*	Modify			00.15		2019.08.19		YZ.Huang
*	Modify			00.16		2019.11.08		YZ.Huang
*	Modify			00.17		2020.04.20		YZ.Huang
*	Modify			00.18		2020.09.14		YZ.Huang
*	Modify			00.19		2020.09.30		YZ.Huang
*	Modify			00.20		2021.02.23		YZ.Huang
***************************************************************************************************************/
#include <linux/printk.h>


#include "mt_fe_def.h"
#include "mt_fe_i2c_ct8k.h"
#include "mt_fe_dmd_ct8k_T.h"
#include "mt_fe_dmd_ct8k_T2.h"

#if MT_FE_DMD_DVBC_SUPPORT
#include "mt_fe_dmd_ct8k_C.h"
#endif

#if MT_FE_DMD_J83B_SUPPORT
#include "mt_fe_dmd_ct8k_B.h"
#endif

#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_ct8k_S_S2.h"
#endif


extern void port_ct8k_config_dvbt2_memory(MT_FE_CT8K_Device_Handle handle);
extern void port_ct8k_release_dvbt2_memory(MT_FE_CT8K_Device_Handle handle);


U32 SetSeveralBits2Data(U32 target_data, U32 source_bits, U8 bit_high, U8 bit_low)
{
	U32 tmp_data;
	U8 bit_tmp;

	if((bit_high > 31) || (bit_low > 31))
	{
		return target_data;
	}

	if(bit_high == bit_low)
	{
		source_bits &= 0x01;
		source_bits <<= bit_low;

		tmp_data = 0x01 << bit_low;

		target_data &= ~tmp_data;
		target_data |= source_bits;

		return target_data;
	}

	if(bit_high < bit_low)
	{
		bit_tmp = bit_high;
		bit_high = bit_low;
		bit_low = bit_tmp;
	}

	tmp_data = 0xFFFFFFFF;
	tmp_data >>= bit_low;
	tmp_data <<= bit_low;
	tmp_data <<= (31 - bit_high);
	tmp_data >>= (31 - bit_high);
	tmp_data &= 0xFFFFFFFF;

	source_bits <<= bit_low;
	source_bits &= tmp_data;

	target_data &= ~tmp_data;
	target_data |= source_bits;


	return target_data;
}


/*	FUNCTIN:
**		mt_fe_get_driver_version_ct8k
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
MT_FE_RET mt_fe_get_driver_version_ct8k(U8 *pd_ver)
{
	MT_FE_RET ret = MtFeErr_Ok;

	#if MT_FE_DMD_DVBS_S2_SUPPORT
		mt_fe_dmd_get_driver_version_ct8k_ss2(&pd_ver[0]);/*s/s2 driver software's version*/
	#else
		pd_ver[0] = 0;
	#endif

	#if MT_FE_DMD_DVBC_SUPPORT
		mt_fe_dmd_get_driver_version_ct8k_c(&pd_ver[1]);/*c driver software's version*/
	#else
		pd_ver[1] = 0;
	#endif

	#if MT_FE_DMD_J83B_SUPPORT
		mt_fe_dmd_get_driver_version_ct8k_b(&pd_ver[2]);/*b driver software's version*/
	#else
		pd_ver[2] = 0;
	#endif

	mt_fe_dmd_get_driver_version_ct8k_t(&pd_ver[3]);/*t  driver software's version*/
	mt_fe_dmd_get_driver_version_ct8k_t2(&pd_ver[4]);/*t2 driver software's version*/

	return ret;
}

MT_FE_RET mt_fe_dmd_ct8k_config_default(MT_FE_CT8K_Device_Handle handle)
{
	handle->sys_dev_addr = 0x18;
	handle->sar_dev_addr = 0x80;

	handle->sys_dev_xtal = MtFeXTALMode_27M;

	handle->bSysInitOk	 = FALSE;
	handle->bTunerBusOn	 = FALSE;

	handle->bSupportDualOutput	 = FALSE;

	handle->ulT2ShareMemAddr = 0;

	mt_fe_dmd_config_default_ctt2_ct8k(handle);

#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_dmd_config_default_ss2_ct8k(handle);
#endif

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_calibration_ct8k_ct(MT_FE_CT8K_Device_Handle handle, U8 iMaxTimes)
{
	U8 tmp1, tmp2, tmp3, i;

	_mt_fe_sar_get_reg_ct8k(handle, 0x32, &tmp1);
	tmp1 |= 0x53;//53//5f
	_mt_fe_sar_set_reg_ct8k(handle, 0x32, tmp1); //set clear lms_lock log

	_mt_fe_sar_get_reg_ct8k(handle, 0x31, &tmp1);
	tmp1 |= 0x0F;
	_mt_fe_sar_set_reg_ct8k(handle, 0x31, tmp1); //enable calibration & ext control

	_mt_fe_sar_get_reg_ct8k(handle, 0x33, &tmp1);
	tmp1 |= 0x43;//42-div2//43-div4//4b-div8
	_mt_fe_sar_set_reg_ct8k(handle, 0x33, tmp1); //enable ext control & clock div4

	_mt_fe_sar_get_reg_ct8k(handle, 0x34, &tmp1);
	tmp1 |= 0x40;//43//40
	_mt_fe_sar_set_reg_ct8k(handle, 0x34, tmp1); //enable ext control

	_mt_fe_sar_set_reg_ct8k(handle, 0x40, 0x71); 

	for(i = 0; i < iMaxTimes; i ++)
	{
		_mt_fe_sar_get_reg_ct8k(handle, 0x30, &tmp2);
		tmp2 |= 0x08;
		_mt_fe_sar_set_reg_ct8k(handle, 0x30, tmp2); //reset calibration

		_mt_delay_ct8k(5);//wait 5ms

		_mt_fe_sar_get_reg_ct8k(handle, 0x32, &tmp1);
		tmp1 &= 0x3F;
		_mt_fe_sar_set_reg_ct8k(handle, 0x32, tmp1);//release clear lms_lock log

		tmp2 &= 0xF0;
		_mt_fe_sar_set_reg_ct8k(handle, 0x30, tmp2);//release calibration

		_mt_delay_ct8k(10); //wait 10ms

		_mt_fe_sar_get_reg_ct8k(handle, 0x39, &tmp3);  //judge if calibration is success
		if((tmp3 >= 0x29) && (tmp3 <= 0x2F))  //after calibration, reg 0x39 should be 0x29 - 0x2F
			break;
	}

	if((tmp3 >= 0x29) && (tmp3 <= 0x2F))
	{
		mt_fe_print(("\tCT8000 C & T calibration OK!\n"));
	}
	else
	{
		mt_fe_print(("\tCT8000 C & T calibration failed!\n"));
	}

	_mt_fe_sar_get_reg_ct8k(handle, 0x31, &tmp1);
	tmp1 &= 0x00;
	_mt_fe_sar_set_reg_ct8k(handle, 0x31, tmp1); //disable calibration

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_re_calibration_ct8k_ct(MT_FE_CT8K_Device_Handle handle, U8 *p_buf)
{
	U8 tmp1 = 0, i = 0, tmp2 = 0;

	/*Read Registers*/
	_mt_sleep_ct8k(40); //wait 40ms

	for(i = 0x39; i <= 0x3F; i ++)
	{
		_mt_fe_sar_get_reg_ct8k(handle, i, &tmp1);
	}

	_mt_fe_sar_get_reg_ct8k(handle, 0x36, &tmp2);
	tmp2 |= 0x02;
	_mt_fe_sar_set_reg_ct8k(handle, 0x36, tmp2);

	for(i = 0; i < 28; i ++)
	{
		//_mt_fe_sar_get_reg_ct8k(handle, 0x35, &tmp1);
		_mt_fe_sar_get_reg_ct8k(handle, 0x35, &p_buf[i]);
	}

	tmp2 &= 0xFD;
	_mt_fe_sar_set_reg_ct8k(handle, 0x36, tmp2);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_double_check_calibration_result(MT_FE_CT8K_Device_Handle handle, U8 *p_buf, U8 *p_tmp)
{
#if 1
	U16 temp1 = 0, temp2 = 0;


	//if((p_buf[0] > 0xCA) || (p_buf[0] < 0xC8))		// [0xC9 - 1, 0xC9 + 1]
	//if((MT_ABS(p_buf[0], 0xC9) > 1) || (MT_ABS(p_tmp[0], 0xC9) > 1))
	//	return MtFeErr_Fail;

	temp1 = (p_buf[0] << 8) + p_buf[1];
	temp2 = (p_tmp[0] << 8) + p_tmp[1];

	if(MT_ABS(temp1, temp2) > 128)
		return MtFeErr_Fail;


	//if((p_buf[2] > 0x79) || (p_buf[2] < 0x77))
	//if((MT_ABS(p_buf[2], 0x78) > 1) || (MT_ABS(p_tmp[2], 0x78) > 1))
	//	return MtFeErr_Fail;

	temp1 = (p_buf[2] << 8) + p_buf[3];
	temp2 = (p_tmp[2] << 8) + p_tmp[3];

	if(MT_ABS(temp1, temp2) > 64)
		return MtFeErr_Fail;


	//if((p_buf[4] > 0x4B) || (p_buf[4] < 0x49))
	//if((MT_ABS(p_buf[4], 0x4A) > 1) || (MT_ABS(p_tmp[4], 0x4A) > 1))
	//	return MtFeErr_Fail;

	temp1 = (p_buf[4] << 8) + p_buf[5];
	temp2 = (p_tmp[4] << 8) + p_tmp[5];

	if(MT_ABS(temp1, temp2) > 48)
		return MtFeErr_Fail;


	//if((p_buf[6] > 0x2F) || (p_buf[6] < 0x2B))
	//if((MT_ABS(p_buf[6], 0x2D) > 2) || (MT_ABS(p_tmp[6], 0x2D) > 2))
	//	return MtFeErr_Fail;

	temp1 = (p_buf[6] << 8) + p_buf[7];
	temp2 = (p_tmp[6] << 8) + p_tmp[7];

	if(MT_ABS(temp1, temp2) > 32)
		return MtFeErr_Fail;

#else
	if((p_buf[0] > 0xCA) || (p_buf[0] < 0xC8))
		return MtFeErr_Fail;

	if((p_buf[2] > 0x79) || (p_buf[2] < 0x77))
		return MtFeErr_Fail;

	if((p_buf[4] > 0x4B) || (p_buf[4] < 0x49))
		return MtFeErr_Fail;

	if((p_buf[6] > 0x2F) || (p_buf[6] < 0x2B))
		return MtFeErr_Fail;
#endif

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_calibration_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	U8 data = 0, tmp = 0;
	U32 reg1 = 0, reg2 = 0, reg3 = 0, reg = 0;
	MT_FE_RET ret = MtFeErr_Ok;
	//int i = 0, j = 0;
	//U8 buf[28];

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

// fix 105076 start
#if 0
	if(handle->m_device_ctt2.demod_current_type == MtFeType_Undef)
	{
		mt_fe_dmd_open_ctt2_ct8k(handle, MtFeType_J83B);	// Enter J83.B mode to enable clock
	}
#else
	_mt_fe_dmd_get_reg_ct8k(handle, 0x04, &tmp);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x97);
#endif

	handle->Get32Bits(0xBF157004, &reg);
	if((reg & 0x40000000) == 0x40000000)	// bit 30 = 1, USB_PLL_PI original status = power down
	{
		handle->Set32Bits(0xBF157004, reg & 0xBFFFFFFF);	// bit 30 = 0, power on USB_PLL_PI

		_mt_sleep_ct8k(10);
	}


	handle->Get32Bits(0xBF5D00D0, &reg1);
	// bit[10:8] = _100
	reg1 &= 0xFFFFF8FF;
	reg1 |= 0x00000400;
	handle->Set32Bits(0xBF5D00D0, reg1);

	handle->Get32Bits(0xBF5D0098, &reg1);
	handle->Get32Bits(0xBF5D009C, &reg2);
	handle->Get32Bits(0xBF5D004C, &reg3);

	//handle->Set32Bits(0xBF5D0098, 0xF000281F);
	handle->Set32Bits(0xBF5D0098, (reg1 & 0xFFFFF000) | 0x081F);
	handle->Set32Bits(0xBF5D009C, 0x00404040);
	//handle->Set32Bits(0xBF5D004C, 0x0EA1BB01);
	handle->Set32Bits(0xBF5D004C, ((0x00A1 << 16) | (reg3 & 0x8FFE) | 0x5001));

	_mt_fe_dmd_get_reg_ct8k(handle, 0x0A, &data);
	data &= 0xFB;		// 2V
	if(handle->m_device_ctt2.board_cfg.iVppSel == 1)		// 1V
	{
		data |= 0x04;
	}
	_mt_fe_dmd_set_reg_ct8k(handle, 0x0A, data);  //Set 1V or 2V Vdpp


	_mt_fe_dmd_calibration_ct8k_ct(handle, 5);

	if(handle->m_device_ctt2.board_cfg.bReadCali)
	{
		_mt_fe_dmd_re_calibration_ct8k_ct(handle, handle->m_device_ctt2.calibration_data);
	}

#if 0
	do 
	{
		_mt_fe_dmd_calibration_ct8k_ct(handle, 5);

		if(handle->m_device_ctt2.board_cfg.bReadCali)
		{
			_mt_fe_dmd_re_calibration_ct8k_ct(handle, handle->m_device_ctt2.calibration_data);
		}


		// calibration & result check
		_mt_delay_ct8k(2);

		for(j = 0; j < 28; j ++)
		{
			buf[j] = handle->m_device_ctt2.calibration_data[j];
		}

		_mt_fe_dmd_calibration_ct8k_ct(handle, 5);

		if(handle->m_device_ctt2.board_cfg.bReadCali)
		{
			_mt_fe_dmd_re_calibration_ct8k_ct(handle, handle->m_device_ctt2.calibration_data);
		}

		ret = _mt_fe_dmd_double_check_calibration_result(handle, handle->m_device_ctt2.calibration_data, buf);

		if(ret != MtFeErr_Ok)
		{
			U32 temp = 0;

			handle->Get32Bits(0xBF157000, &temp);
			temp |= (1 << 20);
			handle->Set32Bits(0xBF157000, temp);
			_mt_delay_ct8k(5);
			temp &= ~(1 << 20);
			handle->Set32Bits(0xBF157000, temp);
			_mt_delay_ct8k(5);
		}

		i ++;
	} while((i < 4) && (ret != MtFeErr_Ok));
#endif


	handle->m_device_ctt2.bCalibrationOK = (ret == MtFeErr_Ok) ? TRUE : FALSE;

	handle->Set32Bits(0xBF5D0098, reg1);
	handle->Set32Bits(0xBF5D009C, reg2);
	handle->Set32Bits(0xBF5D004C, reg3);


	if((reg & 0x40000000) == 0x40000000)	// bit 30 = 1, USB_PLL_PI original status = power down
	{
		handle->Set32Bits(0xBF157004, reg | 0x40000000);	// bit 30 = 1, power down USB_PLL_PI

		_mt_sleep_ct8k(10);
	}

	_mt_fe_dmd_set_reg_ct8k(handle, 0x04, tmp);


	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_config_clock_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type)
{
	U32 ulTmp1;
	MT_BOOL bXtal27M;

	MT_BOOL bTT2 = FALSE, bSS2 = FALSE, bJ83B = FALSE, bBigSym = FALSE;

	MT_BOOL bMode1 = FALSE, bMode2 = FALSE, bMode3 = FALSE, bMode4 = FALSE, bMode5 = FALSE;

//	static MT_BOOL bMode4Ok = FALSE;
//	static MT_BOOL bMode5Ok = FALSE;

	MT_FE_TYPE ctt2_type = handle->m_device_ctt2.demod_type;
	MT_FE_TYPE ss2_type = handle->m_device_ss2.demod_type;

	MT_FE_RET ret = MtFeErr_Ok;

	//MT_BOOL bXtal27M = TRUE;

	bMode1 = (handle->mode_select >> 0) & 0x01;
	bMode2 = (handle->mode_select >> 1) & 0x01;
	bMode3 = (handle->mode_select >> 2) & 0x01;
	bMode4 = (handle->mode_select >> 3) & 0x01;
	bMode5 = (handle->mode_select >> 4) & 0x01;

#if 0	// 20200611 disable logs
	printk( "%s[%d] -- mode_select = 0x%02x\n", __FUNCTION__, __LINE__, handle->mode_select); 
	printk( "Support modes: 1[%d], 2[%d], 3[%d], 4[%d], 5[%d]\n", bMode1, bMode2, bMode3, bMode4, bMode5);


	handle->Get32Bits(0xBF5D004C, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF5D004C = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

	handle->Get32Bits(0xBF5D0048, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF5D0048 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

	handle->Get32Bits(0xBF5D0098, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF5D0098 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

	handle->Get32Bits(0xBF157000, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF157000 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

	handle->Get32Bits(0xBF157004, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF157004 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

	handle->Get32Bits(0xBF500000, &ulTmp1);
	printk( "%s[%d] -- Get register 0xBF500000 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);
#endif

	if((bMode4 || bMode5) && !bMode1 && !bMode2 && !bMode3)
	{
#if 1
		return MtFeErr_Ok;
#else
		handle->Get32Bits(0xBF5D004C, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF5D004C = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

		handle->Get32Bits(0xBF5D0048, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF5D0048 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

		handle->Get32Bits(0xBF5D0098, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF5D0098 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

		handle->Get32Bits(0xBF157000, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF157000 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

		handle->Get32Bits(0xBF157004, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF157004 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);

		handle->Get32Bits(0xBF500000, &ulTmp1);
		printk( "%s[%d] -- Get register 0xBF500000 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp1);
#endif
	}


	if((ctt2_type == MtFeType_DVBT) || (ctt2_type == MtFeType_DVBT2) || (ctt2_type == MtFeType_DVBT_T2))
	{
		bTT2 = TRUE;
		bJ83B = FALSE;
	}
	else
	{
		bTT2 = FALSE;

		if(ctt2_type == MtFeType_J83B)
		{
			bJ83B = TRUE;
		}
		else
		{
			bJ83B = FALSE;
		}
	}

	if((ss2_type == MtFeType_DVBS) || (ss2_type == MtFeType_DVBS2) || (ss2_type == MtFeType_DVBS_S2))
	{
		bSS2 = TRUE;
	}
	else
	{
		bSS2 = FALSE;
	}


	handle->Get32Bits(0xBF140020, &ulTmp1);
	if(ulTmp1 & 0x40000000)
	{
		bXtal27M = FALSE;
	}
	else
	{
		bXtal27M = TRUE;
	}

	if(handle->chip_version == 0x9000)		// Sym2 0xA0
	{
		if(demod_type == MtFeType_J83B)		// S: 96M		C: 54M
		{
			if(bXtal27M)
			{
				handle->Set32Bits(0xBF5D004C, 0x0EA1BB02);
				handle->Set32Bits(0xBF5D0048, 0x5C94A1C9);
			}
			else
			{
				handle->Set32Bits(0xBF5D004C, 0x0EA1BB02);
				handle->Set32Bits(0xBF5D0048, 0x5C67A0C9);
			}
		}
		else if((demod_type == MtFeType_DVBS) || 
				(demod_type == MtFeType_DVBS2) || 
				(demod_type == MtFeType_DVBS_S2))	// S: 96M or 108M
		{
			if(handle->m_device_ss2.tp_cfg.iSymRateKSs > 46000)
			{
				handle->m_device_ss2.global_cfg.iMclkKHz = 108000;
				bBigSym = TRUE;

				if(bXtal27M)
				{
					handle->Set32Bits(0xBF5D004C, 0x0EA1BB10);
					handle->Set32Bits(0xBF5D0048, 0x1CD081B9);
				}
				else
				{
					handle->Set32Bits(0xBF5D004C, 0x0EA1BB10);
					handle->Set32Bits(0xBF5D0048, 0x5C9480B9);
				}

				_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x4c);
			}
			else
			{
				handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
				bBigSym = FALSE;

				handle->Set32Bits(0xBF5D004C, 0x0EA1BB00);
				handle->Set32Bits(0xBF5D0048, 0x5C9481F9);

				_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);
			}
		}
		else								// S: 96M		C: 28.8M
		{
			handle->Set32Bits(0xBF5D004C, 0x0EA1BB00);
			handle->Set32Bits(0xBF5D0048, 0x5C9481F9);
		}
	}
	else	// Sym2 0xA1 or later
	{
		if(demod_type == MtFeType_J83B)		// J83.B: 54M
		{
			if(bSS2)
			{
				if(handle->m_device_ss2.tp_cfg.iSymRateKSs > 46000)			// S: 108M
				{	// mode 3		108 + 54
					if(bMode3)
					{
						handle->m_device_ss2.global_cfg.iMclkKHz = 108000;
						bBigSym = TRUE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);
						//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
						handle->Set32Bits(0xBF5D0048, 0x1CB8A089);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x4c);

						//printk("%s[%d] -- Set mode 3 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
				else					// S: 96M
				{	// mode 2		 96 + 54
					if(bMode2)
					{
						handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
						bBigSym = FALSE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB0E);
						//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
						handle->Set32Bits(0xBF5D0048, 0x1CB8A0C9);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);

						//printk( "%s[%d] -- Set mode 2 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
			}
			else		// J83.B only 54M, S 96M
			{	// mode 2		 96 + 54
				if(bMode2)
				{
					handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
					bBigSym = FALSE;

					handle->Set32Bits(0xBF5D004C, 0x0EA1BB0E);
					//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
					handle->Set32Bits(0xBF5D0048, 0x1CB8A0C9);

					_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);

					//printk( "%s[%d] -- Set mode 2 OK!\n", __FUNCTION__, __LINE__);
				}
				else
				{
					ret = MtFeErr_Fail;
				}
			}
		}
		else if((demod_type == MtFeType_DVBS) || 
				(demod_type == MtFeType_DVBS2) || 
				(demod_type == MtFeType_DVBS_S2))	// S: 96M or 108M
		{
			if(handle->m_device_ss2.tp_cfg.iSymRateKSs > 46000)
			{
#if 0
				if(ctt2_type == MtFeType_J83B)		// J83.B: 54M
				{	// mode 3		108 + 54
					if(bMode3)
					{
						handle->m_device_ss2.global_cfg.iMclkKHz = 108000;
						bBigSym = TRUE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);
						//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
						handle->Set32Bits(0xBF5D0048, 0x1CB8A089);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x4c);

						//printk( "%s[%d] -- Set mode 3 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
				else		// C: 28.8M
#endif
				{
					if(bMode3)
					{	// mode 3		108 + 54
						handle->m_device_ss2.global_cfg.iMclkKHz = 108000;
						bBigSym = TRUE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);
						//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
						handle->Set32Bits(0xBF5D0048, 0x1CB8A089);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x4c);

						//printk( "%s[%d] -- Set mode 3 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
			}
			else
			{
				if(ctt2_type == MtFeType_J83B)		// J83.B: 54M
				{	// mode 2		 96 + 54
					if(bMode2)
					{
						handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
						bBigSym = FALSE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB0E);
						//handle->Set32Bits(0xBF5D0098, 0xF000F00F);
						handle->Set32Bits(0xBF5D0048, 0x1CB8A0C9);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);

						//printk( "%s[%d] -- Set mode 2 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
				else
				{	// mode 1/4	 96 + 28.8
					if(bMode1)
					{
						handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
						bBigSym = FALSE;

						handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);

						handle->Get32Bits(0xBF5D0048, &ulTmp1);
						ulTmp1 &= ~0x00002000;	// bit 13 = 0
						handle->Set32Bits(0xBF5D0048, ulTmp1);

						_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);

						//printk( "%s[%d] -- Set mode 1 OK!\n", __FUNCTION__, __LINE__);
					}
					else
					{
						ret = MtFeErr_Fail;
					}
				}
			}
		}
		else								// S: 96M		C: 28.8M
		{	// mode 1/4	 96 + 28.8
			if(bMode1)
			{
				handle->m_device_ss2.global_cfg.iMclkKHz = 96000;
				bBigSym = FALSE;

				handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);
				handle->Get32Bits(0xBF5D0048, &ulTmp1);
				ulTmp1 &= ~0x00002000;	// bit 13 = 0
				handle->Set32Bits(0xBF5D0048, ulTmp1);

				_mt_fe_ct8k_set_reg(handle->m_device_ss2.demod_dev_addr, 0xa0, 0x44);

				//printk( "%s[%d] -- Set mode 1 OK!\n", __FUNCTION__, __LINE__);
			}
			else
			{
				ret = MtFeErr_Fail;
			}
		}
	}

	_mt_sleep_ct8k(10);

	if(bMode1 && bMode2 && bMode3 && bMode4 && bMode5)
	{
		handle->Get32Bits(0xBF157004, &ulTmp1);

		if(bJ83B || bBigSym)
		{
			if((ulTmp1 & 0x80000000) != 0)
			{
				ulTmp1 &= ~0x80000000;
				handle->Set32Bits(0xBF157004, ulTmp1);

				_mt_sleep_ct8k(10);
			}
		}
		else
		{
			if((ulTmp1 & 0x80000000) == 0)
			{
				ulTmp1 |= 0x80000000;
				handle->Set32Bits(0xBF157004, ulTmp1);

				_mt_sleep_ct8k(10);
			}
		}
	}


	//return MtFeErr_Ok;
	return ret;
}

/*	FUNCTIN:
**		mt_fe_set_demod_type_ct8k
**
**	DESCRIPTION:
**		set Symphony2 demod_type
**
**	IN:
**		MT_FE_CT8K_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode
**		MT_FE_TYPE demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_set_demod_type_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode, MT_FE_TYPE demod_type)
{
	U8	reg_04 = 0, reg_06 = 0, reg_07 = 0, data = 0;		// fix 105076

	U32 ulTmp1, ulTmp2, ulTmp3;

	MT_FE_RET ret = MtFeErr_Ok;

	MT_BOOL bTT2 = FALSE, bSS2 = FALSE;

	MT_BOOL bMode1 = FALSE, bMode2 = FALSE, bMode3 = FALSE, bMode4 = FALSE, bMode5 = FALSE;


	MT_FE_TYPE ctt2_type = handle->m_device_ctt2.demod_type;
	MT_FE_TYPE ss2_type = handle->m_device_ss2.demod_type;

	//MT_BOOL bXtal27M = TRUE;

	bMode1 = (handle->mode_select >> 0) & 0x01;
	bMode2 = (handle->mode_select >> 1) & 0x01;
	bMode3 = (handle->mode_select >> 2) & 0x01;
	bMode4 = (handle->mode_select >> 3) & 0x01;
	bMode5 = (handle->mode_select >> 4) & 0x01;

	if((ctt2_type == MtFeType_DVBT) || (ctt2_type == MtFeType_DVBT2) || (ctt2_type == MtFeType_DVBT_T2))
	{
		bTT2 = TRUE;
	}
	else
	{
		bTT2 = FALSE;
	}

	if((ss2_type == MtFeType_DVBS)|| (ss2_type == MtFeType_DVBS2) || (ss2_type == MtFeType_DVBS_S2))
	{
		bSS2 = TRUE;
	}
	else
	{
		bSS2 = FALSE;
	}


	if(bMode1 && bMode2 && bMode3 && bMode4 && bMode5)
	{
		handle->Get32Bits(0xBF157004, &ulTmp1);

		if((ulTmp1 & 0x80000000) != 0)
		{
			ulTmp1 &= ~0x80000000;
			handle->Set32Bits(0xBF157004, ulTmp1);

			_mt_sleep_ct8k(10);
		}
	}


	if (demod_mode == MtFeDemodMode_DVBCTT2)
	{
		handle->Get32Bits(0xBF13C00C, &ulTmp1);
		handle->Get32Bits(0xBF5D0094, &ulTmp2);
		handle->Get32Bits(0xBF138008, &ulTmp3);

		//1¡¢Reg0xbf13c00c [7:4] = 0x2£¬PINMUX select TC-IF_AGC£»
		ulTmp1 &= 0xFFFFFF0F;
		ulTmp1 |= 0x00000020;
		handle->Set32Bits(0xBF13C00C, ulTmp1);

		//2¡¢Reg0xbf5d0094 bit3 = 0, VINN/VINP analog mode£»
		//ulTmp2 &= 0xFFFFFFFE;
		ulTmp2 &= 0xFFFFFFF7;
		handle->Set32Bits(0xBF5D0094, ulTmp2);

		//3¡¢Reg0xbf138008 bit0 = 1 to select DVB-C demod TS to PTI2£»
		ulTmp3 |= 0x00000001;
		handle->Set32Bits(0xBF138008, ulTmp3);
	}
#if MT_FE_DMD_DVBS_S2_SUPPORT
	else if(demod_mode == MtFeDemodMode_DVBSS2)
	{
		handle->Get32Bits(0xBF13C008, &ulTmp1);
		//handle->Get32Bits(0xBF13C018, &ulTmp2);
		handle->Get32Bits(0xBF5D0094, &ulTmp3);


		//1¡¢ÅäÖÃReg0xbf13c008 [15:12] = 0x4£¬ PINMUX select S-RFAGC mode£»
		ulTmp1 &= 0xFFFF0FFF;
		ulTmp1 |= 0x00004000;
		handle->Set32Bits(0xBF13C008, ulTmp1);

#if 0	// removed for JTAG
		//2¡¢ÅäÖÃReg0xbf13c018 bit0 = 1 S_RFAGC CMOS mode£»//sym2 a0 ¿ÉÒÔ²»ÒªÅäÖÃ
		ulTmp2 |= 0x00000001;
		handle->Set32Bits(0xBF13C018, ulTmp2);
#endif

		//3¡¢ÅäÖÃReg0xbf5d0094 bit0 = 0, IQ analog mode£»
		ulTmp3 &= 0xFFFFFFFE;
		handle->Set32Bits(0xBF5D0094, ulTmp3);
	}
#endif

	handle->Get32Bits(0xBF5D00D0, &ulTmp1);
	//BF5D00D0H[28 : 27] = _00
	//BF5D00D0H[22] = 0
	ulTmp1 &= 0xE7BFFFFF;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);

	//wait 1ms
	_mt_delay_ct8k(1);

	//BF5D00D0H[22] = 1
	ulTmp1 |= 0x00400000;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);


	_mt_fe_dmd_get_reg_ct8k(handle, 0x04, &reg_04);
	_mt_fe_dmd_get_reg_ct8k(handle, 0x06, &reg_06);
	_mt_fe_dmd_get_reg_ct8k(handle, 0x07, &reg_07);

	if (demod_mode == MtFeDemodMode_DVBCTT2)
	{
		// fix 105076 start
		_mt_fe_dmd_get_reg_ct8k(handle, 0x0A, &data);
		data &= 0xFB;		// 2V

		switch (demod_type)
		{
#if MT_FE_DMD_DVBC_SUPPORT
			case MtFeType_DVBC:
				if(bSS2)
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x09);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x0a);
				}
				else
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x01);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x08);
				}
				_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x00);
				if(handle->m_device_ctt2.board_cfg.iVppSelC == 1)		// 1V
				{
					data |= 0x04;
				}
				break;
#endif

#if MT_FE_DMD_J83B_SUPPORT
			case MtFeType_J83B:
				if(bSS2)
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x0c);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x02);
				}
				else
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x04);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x00);
				}
				_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x20);
				if(handle->m_device_ctt2.board_cfg.iVppSelC == 1)		// 1V
				{
					data |= 0x04;
				}
				break;
#endif

			case MtFeType_DVBT:
				//_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x12);
				_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x80);
				_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x05);
				_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);
				if(handle->m_device_ctt2.board_cfg.iVppSel == 1)		// 1V
				{
					data |= 0x04;
				}
				break;

			case MtFeType_DVBT2:
				_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x10);
				_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x05);
				_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);
				if(handle->m_device_ctt2.board_cfg.iVppSel == 1)		// 1V
				{
					data |= 0x04;
				}
				break;

			case MtFeType_DVBT_T2:
				_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x80);
				_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x05);
				_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);
				if(handle->m_device_ctt2.board_cfg.iVppSel == 1)		// 1V
				{
					data |= 0x04;
				}
				break;

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}

		_mt_fe_dmd_set_reg_ct8k(handle, 0x0A, data);  //Set 1V or 2V Vdpp
		// fix 105076 end

		handle->m_device_ctt2.demod_type = demod_type;

		if(handle->chip_version == 0x9000)		// Sym2 A0
			handle->m_device_ss2.demod_type = MtFeType_Undef;
	}
#if MT_FE_DMD_DVBS_S2_SUPPORT
	else if(demod_mode == MtFeDemodMode_DVBSS2)
	{
		switch (demod_type)
		{
			case MtFeType_DVBS:
			case MtFeType_DVBS2:
			case MtFeType_DVBS_S2:
				if(ctt2_type == MtFeType_DVBC)
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x09);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x0a);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x00);
				}
				else if(ctt2_type == MtFeType_J83B)
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x0c);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x02);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x20);
				}
				else
				{
					_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x08);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x02);
					_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x00);
				}
				break;

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}


		if(ret == MtFeErr_Ok)
		{
			handle->m_device_ss2.demod_type = demod_type;

			if(handle->chip_version == 0x9000)		// Sym2 A0
				handle->m_device_ctt2.demod_type = MtFeType_Undef;
		}
	}
#endif


	mt_fe_dmd_config_clock_ct8k(handle, demod_type);

	return ret;
}

MT_FE_RET mt_fe_system_init_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_TYPE demod_type = MtFeType_Undef;
	U8 reg_03 = 0;

	U32 ulTmp = 0, ulData = 0;

	if(handle->bSysInitOk)
		return MtFeErr_Ok;


	_mt_fe_dmd_get_reg_ct8k(handle, 0x03, &reg_03);
	reg_03 |= 0x80;
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, reg_03);
	_mt_delay_ct8k(1);
	reg_03 &= 0x7F;
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, reg_03);


#if 1	// 180905
	handle->Get32Bits(0xBF140008, &ulTmp);
	handle->Set32Bits(0xBF140008, ulTmp | 0xFFFF);

	handle->Get32Bits(0xBF140004, &ulData);

	handle->Set32Bits(0xBF140008, ulTmp);

	handle->chip_version = (U16)(ulData & 0xFFFF);

#if 0	// 190225
	if((handle->chip_version == 0x9000) || 				// Symphony2 A0
	   (handle->chip_version == 0x9001))					// Symphony2 A1
	{
		handle->Get32Bits(0xBF140024, &ulTmp);
		if(((ulTmp & 0xFF) == 0x04) || ((ulTmp & 0xFF) == 0x05))	// 144 pin
		{
			if((handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800) || 
			   (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800))
			{
				handle->Get32Bits(0xBF157004, &ulTmp);
				ulTmp &= 0xFFCFFFFF;
				ulTmp |= 0x00200000;
				handle->Set32Bits(0xBF157004, ulTmp);
				ulTmp |= 0x00300000;
				handle->Set32Bits(0xBF157004, ulTmp);
			}
		}
	}
#endif
#endif

	_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x00);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x05, 0x00);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x06, 0x00);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x00);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x08, 0x00);

	_mt_fe_dmd_set_reg_ct8k(handle, 0x09, 0x04);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x00, 0x01);

	handle->bTunerBusOn = FALSE;

	//mt_fe_dmd_config_default_ctt2_ct8k(handle);

	//mt_fe_dmd_select_tuner_ctt2_ct8k(handle, handle->m_device_ctt2.tuner_cfg.tuner_type);

	mt_fe_dmd_calibration_ctt2_ct8k(handle);

#if MT_FE_DMD_DVBS_S2_SUPPORT
	demod_type = MtFeType_DVBS; //MtFeType_DVBS2;
	//mt_fe_dmd_config_default_ss2_ct8k(handle);

	//mt_fe_dmd_select_tuner_ss2_ct8k(handle, handle->m_device_ss2.tuner_cfg.tuner_type);

	_mt_fe_dmd_get_reg_ct8k(handle, 0x03, &reg_03);
	reg_03 = (U8)(reg_03 | 0x0f);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, reg_03);
	reg_03 = (U8)(reg_03 & 0xf0);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, reg_03);

	//mt_fe_dmd_open_ss2_ct8k(handle, demod_type);
	//mt_fe_dmd_close_ss2_ct8k(handle);
#endif

	handle->bSysInitOk = TRUE;

	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_close_ct8k
**
**	DESCRIPTION:
**		Finalize Symphony2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	mt_fe_dmd_close_ctt2_ct8k(handle);
	mt_fe_dmd_close_ss2_ct8k(handle);

	_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x00);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x00);

	handle->m_device_ctt2.demod_current_type = MtFeType_Undef;
	handle->m_device_ss2.demod_current_type = MtFeType_Undef;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_config_default_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_CTT2_Device_Handle handle_ctt2 = &handle->m_device_ctt2;

	if(handle_ctt2 == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle_ctt2->demod_type							 = MtFeType_Undef;
	handle_ctt2->ts_out_mode						 = MtFeTsOutMode_Parallel;
	handle_ctt2->m_iPageNo							 = 0;
	handle_ctt2->input_params.demod_bandwidth		 = MtFeBandwidth_8M;

	handle_ctt2->board_cfg.iVppSel					 = 1;		// 0: 2V; 1: 1V
	handle_ctt2->board_cfg.iVppSelC					 = 0;		// fix 105076
	handle_ctt2->board_cfg.bReadCali				 = TRUE;


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

	//handle_ctt2->dmd_dev_addr				 = handle->sys_dev_addr;
	handle->m_device_ctt2.mcu_status		 = 0;			/* demod device i2c address for C&T */
	handle->m_device_ctt2.bCalibrationOK	 = FALSE;
	handle_ctt2->demod_current_type			 = MtFeType_Undef;
	handle_ctt2->tuner_cfg.tuner_type		 = MtFeTN_TC6800;
	handle_ctt2->tuner_cfg.tuner_dev_addr	 = 0xC6;		/* tuner device i2c address */
	handle_ctt2->tuner_cfg.tuner_init_ok	 = 0;			/* tuner init yes or no,0 :no 1:yes */
	handle_ctt2->tuner_cfg.tuner_open		 = 0;			/* tuner open yes or no,0 :no 1:yes */
	handle_ctt2->tuner_cfg.tuner_lna_type	 = 0;
	handle_ctt2->tuner_cfg.tuner_sleep		 = NULL;		/* set tuner function */
	handle_ctt2->tuner_cfg.tuner_wakeup		 = NULL;		/* set tuner function */

	handle->Get32Bits = _mt_fe_read32_ct8k;
	handle->Set32Bits = _mt_fe_write32_ct8k;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_select_tuner_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_CT8K_SUPPORTED_TUNER tuner_type)
{
	MT_FE_RET ret = MtFeErr_NoSupportTuner;

	switch(tuner_type)
	{
		case MtFeTN_MxL603:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_MXL603
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_MxL603;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc0;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_MxL603_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_MxL603_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_MxL603_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = NULL;
#endif
			ret = MtFeErr_Ok;
			break;

		case MtFeTN_TC3800:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_TC3800
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_TC3800;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc2;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_set_application = (MT_FE_RET(*)(void *, U8))mt_fe_tn_application_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_loop_through = (MT_FE_RET(*)(void *, S16))mt_fe_tn_loop_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_xtal = (MT_FE_RET(*)(void *, U32))mt_fe_tn_xtal_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_clkout = (MT_FE_RET(*)(void *, U8))mt_fe_tn_clkout_tc3800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_get_diagnose_info = (MT_FE_RET(*)(void *, U32*, U32*))mt_fe_tn_get_diagnose_info_tc3800_tc_ct8k;
			ret = MtFeErr_Ok;
#endif
			break;

		case MtFeTN_TC6800:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_TC6800
			handle->m_device_ctt2.tuner_cfg.tuner_type = MtFeTN_TC6800;
			if (handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_ctt2.tuner_cfg.tuner_dev_addr = 0xc6;
			}
			handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_open = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_ctt2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_tc6800_ct8k;//NULL;
			handle->m_device_ctt2.tuner_cfg.tuner_set_application = NULL;//(MT_FE_RET(*)(void *, U8))mt_fe_tn_application_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_loop_through = (MT_FE_RET(*)(void *, S16))mt_fe_tn_loop_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_xtal = (MT_FE_RET(*)(void *, U32))mt_fe_tn_xtal_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_set_clkout = (MT_FE_RET(*)(void *, U8))mt_fe_tn_clkout_tc6800_tc_ct8k;
			handle->m_device_ctt2.tuner_cfg.tuner_get_diagnose_info = (MT_FE_RET(*)(void *, U32*, U32*))mt_fe_tn_get_diagnose_info_tc6800_tc_ct8k;
			ret = MtFeErr_Ok;
#endif
			break;

		default:
			break;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_open_ctt2_ct8k
**
**	DESCRIPTION:
**		initialize Symphony2 DVB-C, J83.B, DVB-T or DVB-T2
**
**	IN:
**		MT_FE_CT8K_Device_Handle handle
**		MT_FE_TYPE 	demod_type
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if 1
	if (handle->m_device_ctt2.demod_current_type == demod_type)
	{
		ret = MtFeErr_Ok;
		return ret;
	}
#endif

	_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x00);
	_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x01);


	if ((demod_type != MtFeType_DVBT)
		&& (demod_type != MtFeType_DVBT2)
		&& (demod_type != MtFeType_DVBT_T2)
#if MT_FE_DMD_DVBC_SUPPORT
		&& (demod_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
		&& (demod_type != MtFeType_J83B)
#endif
		)
	{
		ret = MtFeErr_Fail;
		return ret;
	}

	if((demod_type == MtFeType_DVBT) || 
	   (demod_type == MtFeType_DVBT2) || 
	   (demod_type == MtFeType_DVBT_T2))
	{
		U32 ulTmp;

		handle->Get32Bits(0xBF13C014, &ulTmp);
		ulTmp &= 0xFFFFFFF0;
		ulTmp |= 0x01;
		handle->Set32Bits(0xBF13C014, ulTmp);


		handle->Get32Bits(0xBF5D0094, &ulTmp);
		ulTmp &= 0xFFFFFFF1;
		ulTmp |= 0x00000006;
		handle->Set32Bits(0xBF5D0094, ulTmp);
	}
	else if((demod_type == MtFeType_DVBC) || 
			(demod_type == MtFeType_J83B))
	{
		U32 ulTmp;

		handle->Get32Bits(0xBF5D0094, &ulTmp);
		ulTmp &= 0xFFFFFFF1;
		ulTmp |= 0x00000006;
		handle->Set32Bits(0xBF5D0094, ulTmp);
	}


#if MT_FE_DMD_DVBS_S2_SUPPORT
	if (handle->m_device_ss2.demod_current_type != MtFeType_Undef)
	{
		if((handle->chip_version == 0x9000) || (!handle->bSupportDualOutput))		// Sym2 A0 or not support dual-ts output
		{
			mt_fe_dmd_close_ss2_ct8k(handle);
		}
		else			// Sym2 A1 or later
		{
			if (((demod_type == MtFeType_DVBT2) || 
				 (demod_type == MtFeType_DVBT) || 
				 (demod_type == MtFeType_DVBT_T2)))
				mt_fe_dmd_close_ss2_ct8k(handle);
		}
	}
#endif

#if 0
	if(handle->m_device_ctt2.demod_current_type != MtFeType_Undef)
	{
		if(demod_type != handle->m_device_ctt2.demod_current_type)
		{
			mt_fe_dmd_close_ctt2_ct8k(handle);
			ret = mt_fe_set_demod_type_ct8k(handle, MtFeDemodMode_DVBCTT2, demod_type);
		}
	}
	else
#endif
	{
		ret = mt_fe_set_demod_type_ct8k(handle, MtFeDemodMode_DVBCTT2, demod_type);
	}

	if (ret == MtFeErr_Ok)
	{
#if MT_FE_DMD_J83B_SUPPORT
		if(demod_type == MtFeType_J83B)
			handle->m_device_ctt2.dmd_dev_addr = 0xB8;
		else
#endif
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

		if((handle->m_device_ctt2.tuner_cfg.tuner_open != 0) && (handle->m_device_ctt2.demod_current_type != MtFeType_Undef))
		{
			if(handle->m_device_ctt2.tuner_cfg.tuner_wakeup != NULL)
			{
				mt_fe_i2c_repeat_enable_ct8k(handle);
				handle->m_device_ctt2.tuner_cfg.tuner_wakeup(handle);
				mt_fe_i2c_repeat_disable_ct8k(handle);
			}
		}


		if ((demod_type == MtFeType_DVBT) || 
			(demod_type == MtFeType_DVBT_T2))
		{
			ret = _mt_fe_dmd_init_ct8k_t2(handle);
			ret = _mt_fe_dmd_init_ct8k_t(handle);
		}
		else if(demod_type == MtFeType_DVBT2)
		{
			ret = _mt_fe_dmd_init_ct8k_t2(handle);
		}
#if MT_FE_DMD_DVBC_SUPPORT
		else if(demod_type == MtFeType_DVBC)
		{
			ret = _mt_fe_dmd_init_ct8k_c(handle);
		}
#endif
#if MT_FE_DMD_J83B_SUPPORT
		else if(demod_type == MtFeType_J83B)
		{
			ret = _mt_fe_dmd_init_ct8k_b(handle);
		}
#endif


#if 1	// 190109
		if((demod_type == MtFeType_DVBT) || 
		   (demod_type == MtFeType_DVBT2) || 
		   (demod_type == MtFeType_DVBT_T2))
		{
			U32 ulTmp;

			if((handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC3800) || 
			   (handle->m_device_ctt2.tuner_cfg.tuner_type == MtFeTN_TC6800))
			{
				handle->Get32Bits(0xBF13C014, &ulTmp);
				ulTmp &= 0xFFFFFFF0;
				ulTmp |= 0x00000002;
				handle->Set32Bits(0xBF13C014, ulTmp);
			}
		}
#endif

		//mt_fe_dmd_select_tuner_ctt2_ct8k(handle, handle->m_device_ctt2.tuner_cfg.tuner_type);

		handle->m_device_ctt2.demod_current_type = handle->m_device_ctt2.demod_type;

		if(handle->chip_version == 0x9000)
		{
			handle->m_device_ss2.demod_current_type = MtFeType_Undef;
		}

		handle->m_device_ctt2.cell_info.bHighByteOk = FALSE;
		handle->m_device_ctt2.cell_info.bLowByteOk = FALSE;
		handle->m_device_ctt2.cell_info.usCellId = 0;
	}

	return ret;
}


/*	FUNCTIN:
**		mt_fe_dmd_close_ctt2_ct8k
**
**	DESCRIPTION:
**		finalize Symphony2 DVB-C, J83.B, DVB-T or DVB-T2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	if((handle->m_device_ctt2.tuner_cfg.tuner_open != 0) && (handle->m_device_ctt2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ctt2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}

	handle->m_device_ctt2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ctt2.demod_type = MtFeType_Undef;
	handle->m_device_ctt2.demod_current_type = MtFeType_Undef;

	if(handle->m_device_ss2.demod_current_type == MtFeType_Undef)
		handle->bSysInitOk = FALSE;

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_ctt2_ct8k
**
**	DESCRIPTION:
**		connect to a special channel for dvbc or dvbt or dvbt2
**
**	IN:
**
**		MT_FE_CT8K_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	ret = mt_fe_dmd_open_ctt2_ct8k(handle, handle->m_device_ctt2.demod_type);

	if (ret == MtFeErr_Ok)
	{
#if 1
		if (handle->m_device_ctt2.demod_type == MtFeType_DVBT)
		{
			ret = _mt_fe_dmd_connect_ct8k_t2(handle);
			ret = _mt_fe_dmd_set_demod_ct8k_t(handle);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x01);
			_mt_delay_ct8k(5);
			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x00);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);
		}
		else if	(handle->m_device_ctt2.demod_type == MtFeType_DVBT2)
		{
			ret = _mt_fe_dmd_connect_ct8k_t2(handle);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x01);
			_mt_delay_ct8k(5);
			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x00);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);
		}
		else if	(handle->m_device_ctt2.demod_type == MtFeType_DVBT_T2)
		{
			ret = _mt_fe_dmd_connect_ct8k_t2(handle);
			ret = _mt_fe_dmd_set_demod_ct8k_t(handle);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x01);
			_mt_delay_ct8k(5);
			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x00);

			_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x04);


#if 0
			if(handle->m_device_ctt2.demod_type == MtFeType_DVBT_T2)
			{
				U8 tmp;
				int iCnt = 0, iLockCnt = 60;
				MT_FE_TYPE mDtvType = MtFeType_Undef;

				if(handle->m_device_ctt2.input_params.demod_bandwidth == MtFeBandwidth_6M)
				{
					iLockCnt = 85;
				}
				else if(handle->m_device_ctt2.input_params.demod_bandwidth == MtFeBandwidth_7M)
				{
					iLockCnt = 75;
				}
				else
				{
					iLockCnt = 60;
				}

				do 
				{
					_mt_sleep_ct8k(20);

					_mt_fe_dmd_get_reg_ct8k(handle, 0x09, &tmp);
					if((tmp & 0x60) == 0x20)
					{
						mDtvType = MtFeType_DVBT;
						break;
					}
					else if((tmp & 0x60) == 0x40)
					{
						mDtvType = MtFeType_DVBT2;
						break;
					}

					iCnt ++;
				} while (iCnt < iLockCnt);

				if((mDtvType == MtFeType_DVBT) || (mDtvType == MtFeType_DVBT2))
				{
					handle->m_device_ctt2.demod_current_type = mDtvType;
				}
				else
				{
					handle->m_device_ctt2.demod_current_type = MtFeType_DVBT_T2;
				}
			}
#endif
		}
#else
		if ((handle->m_device_ctt2.demod_type == MtFeType_DVBT) || 
			(handle->m_device_ctt2.demod_type == MtFeType_DVBT2) || 
			(handle->m_device_ctt2.demod_type == MtFeType_DVBT_T2))
		{
			U8 tmp;
			int iCnt = 0;
			MT_FE_TYPE mDtvType = MtFeType_Undef;

			//_mt_fe_dmd_set_reg_ct8k(handle, 0x04, 0x80);
			//_mt_fe_dmd_set_reg_ct8k(handle, 0x07, 0x05);

			//ret = _mt_fe_dmd_connect_ct8k_t(handle);
			ret = _mt_fe_dmd_connect_ct8k_t2(handle);
			ret = _mt_fe_dmd_set_demod_ct8k_t(handle);


			do 
			{
				_mt_sleep_ct8k(20);

				_mt_fe_dmd_get_reg_ct8k(handle, 0x09, &tmp);

				if((tmp & 0x60) == 0x20)
				{
					mDtvType = MtFeType_DVBT;
					break;
				}
				else if((tmp & 0x60) == 0x40)
				{
					mDtvType = MtFeType_DVBT2;
					break;
				}

				iCnt ++;
			} while (iCnt < 50);

			if((mDtvType == MtFeType_DVBT) || (mDtvType == MtFeType_DVBT2))
			{
				handle->m_device_ctt2.demod_current_type = mDtvType;
			}
			else
			{
				handle->m_device_ctt2.demod_current_type = MtFeType_DVBT_T2;
			}
		}
#endif
#if MT_FE_DMD_DVBC_SUPPORT
		else if (handle->m_device_ctt2.demod_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_connect_ct8k_c(handle);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		else if (handle->m_device_ctt2.demod_type == MtFeType_J83B)
			ret = _mt_fe_dmd_connect_ct8k_b(handle);
#endif
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_ctt2_ct8k
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_CT8K_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ctt2.demod_current_type != MtFeType_DVBT)
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT_T2)
#if MT_FE_DMD_DVBC_SUPPORT
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_J83B)
#endif
	 )
	{
		ret = MtFeErr_Fail;
	}

	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_get_lock_state_ct8k_t(handle, p_state);
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
			ret = _mt_fe_dmd_get_lock_state_ct8k_t2(handle, p_state);
#if MT_FE_DMD_DVBC_SUPPORT
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_get_lock_state_ct8k_c(handle, p_state);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
			ret = _mt_fe_dmd_get_lock_state_ct8k_b(handle, p_state);
#endif
		else		// T_T2
		{
			U8 tmp;

			_mt_fe_dmd_get_reg_ct8k(handle, 0x09, &tmp);

			if((tmp & 0x60) == 0x20)
			{
				handle->m_device_ctt2.demod_current_type = MtFeType_DVBT;
				ret = _mt_fe_dmd_get_lock_state_ct8k_t(handle, p_state);
			}
			else if((tmp & 0x60) == 0x40)
			{
				handle->m_device_ctt2.demod_current_type = MtFeType_DVBT2;
				ret = _mt_fe_dmd_get_lock_state_ct8k_t2(handle, p_state);
			}
			else
			{
				*p_state = MtFeLockState_Unlocked;
			}
		}
	}

	return ret;
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
MT_FE_RET mt_fe_dmd_hard_reset_ct8k(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_ctt2_ct8k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ctt2.demod_current_type != MtFeType_DVBT)
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT_T2)
#if MT_FE_DMD_DVBC_SUPPORT
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
	 && (handle->m_device_ctt2.demod_current_type != MtFeType_J83B)
#endif
	   )
	{
		ret = MtFeErr_Fail;
	}

	if (ret == MtFeErr_Ok)
	{
		if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
			ret = _mt_fe_dmd_soft_reset_ct8k_t(handle);
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
			ret = _mt_fe_dmd_soft_reset_ct8k_t2(handle);
#if MT_FE_DMD_DVBC_SUPPORT
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_soft_reset_ct8k_c(handle);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
			ret = _mt_fe_dmd_soft_reset_ct8k_b(handle);
#endif
	}

	return ret;
}


/*	FUNCTIN:
**		mt_fe_dmd_set_output_mode_ct8k
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
MT_FE_RET mt_fe_dmd_set_output_mode_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_set_output_mode_ct8k_t(handle);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_set_output_mode_ct8k_t2(handle);
#if MT_FE_DMD_DVBC_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_set_output_mode_ct8k_c(handle);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_set_bw_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_set_bw_ct8k_t(handle);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_set_bw_ct8k_t2(handle);
//	else if (handle->demod_type == MtFeType_DVBC)
//		ret = _mt_fe_dmd_set_bw_ct8k_c(handle);

	return ret;
}

MT_FE_RET mt_fe_dmd_get_snr_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_snr)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
	{
		U16 snr_t = 0;

		ret = _mt_fe_dmd_get_snr_ct8k_t(handle, &snr_t);
		*p_snr = snr_t / 10;
	}
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_snr_ct8k_t2(handle, p_snr);
#if MT_FE_DMD_DVBC_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_snr_ct8k_c(handle, p_snr);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
	{
		U16 snr_b = 0;

		ret = _mt_fe_dmd_get_snr_ct8k_b(handle, &snr_b);

		*p_snr = snr_b / 10;
	}
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_quality_ct8k_t(handle, p_percent);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_quality_ct8k_t2(handle, p_percent);
#if MT_FE_DMD_DVBC_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_ct8k_c(handle, p_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_quality_ct8k_b(handle, p_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_nordig_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_quality_nordig_ct8k_t(handle, p_percent);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_quality_nordig_ct8k_t2(handle, p_percent);
#if MT_FE_DMD_DVBC_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_ct8k_c(handle, p_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_quality_ct8k_b(handle, p_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
		ret = _mt_fe_dmd_get_strength_ct8k_t(handle, ssi_percent);
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
		ret = _mt_fe_dmd_get_strength_ct8k_t2(handle, ssi_percent);
#if MT_FE_DMD_DVBC_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_strength_ct8k_c(handle, ssi_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_strength_ct8k_b(handle, ssi_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_cell_info_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
	{
		ret = _mt_fe_dmd_get_cell_info_ct8k_t(handle);
	}
	else if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2)
	{
		ret = _mt_fe_dmd_get_cell_info_ct8k_t2(handle);
	}
	else
	{
		ret = MtFeErr_NoMatch;
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_inform_t2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_T2_TPS_INFO *tps_info)
{
	if (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT2)
		return MtFeErr_Fail;
	else
		return _mt_fe_dmd_get_tps_info_ct8k_t2(handle, tps_info);
}

MT_FE_RET mt_fe_dmd_get_inform_t_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info)
{
	if (handle->m_device_ctt2.demod_current_type != MtFeType_DVBT)
		return MtFeErr_Fail;
	else
		return _mt_fe_dmd_get_tps_info_ct8k_t(handle, tps_info);
}

MT_FE_RET mt_fe_dmd_get_per_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt)
{
	if (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT)
	{
		return mt_fe_dmd_get_ber_ct8k_t(handle, p_err_cnt, p_total_cnt);
	}
	else if(handle->m_device_ctt2.demod_current_type == MtFeType_DVBC)
	{
		return _mt_fe_dmd_get_ber_ct8k_c(handle, p_err_cnt, p_total_cnt);
	}
	else if(handle->m_device_ctt2.demod_current_type == MtFeType_J83B)
	{
		return _mt_fe_dmd_get_ber_ct8k_b(handle, p_err_cnt, p_total_cnt);
	}
	else
	{
		return MtFeErr_Ok;
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_sleep_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	U8	data = 0;

	if((handle->m_device_ctt2.tuner_cfg.tuner_open != 0) && (handle->m_device_ctt2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ctt2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}
#endif

	_mt_fe_dmd_get_reg_ct8k(handle, 0x03, &data);
	data = (U8)(data | 0x20);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_wake_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	U8	data = 0;

	_mt_fe_dmd_get_reg_ct8k(handle, 0x03, &data);
	data = (U8)(data & 0xdf);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);

	if((handle->m_device_ctt2.tuner_cfg.tuner_open != 0) && (handle->m_device_ctt2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ctt2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ctt2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}
#endif

	return MtFeErr_Ok;
}

/****************************************DVBS/S2 APIS**********************************************/

#if MT_FE_DMD_DVBS_S2_SUPPORT

MT_FE_RET mt_fe_dmd_config_default_ss2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	return mt_fe_dmd_ct8k_ss2_config_default(handle);
}

MT_FE_RET mt_fe_dmd_select_tuner_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_CT8K_SUPPORTED_TUNER tuner_type)
{
	MT_FE_RET ret = MtFeErr_NoSupportTuner;

	switch(tuner_type)
	{
		case MtFeTn_TS2022:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_TS2022
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
			handle->m_device_ss2.board_cfg.bIQInverted = FALSE;
			handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;
			handle->m_device_ss2.bs_cfg.bs_symrate = BLINDSCAN_SYMRATEKSs;	// 40000;
			ret = MtFeErr_Ok;
#endif
			break;

		case MtFeTn_TS6011:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_TS6011
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
			handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
			handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;
			handle->m_device_ss2.bs_cfg.bs_symrate = BLINDSCAN_SYMRATEKSs;	// 40000;
			ret = MtFeErr_Ok;
#endif
			break;

		case MtFeTn_RDA5815M:
#ifdef CONFIG_MT_FRONTEND_DMD_CT8K_TN_RDA5815M
#if 1
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_RDA5815M;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x18;
			handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_RDA5815M_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_RDA5815M_ct8k;
			handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
			handle->m_device_ss2.board_cfg.bAGCPolar = TRUE;
			handle->m_device_ss2.bs_cfg.bs_symrate = 40000;	// BLINDSCAN_SYMRATEKSs;
			ret = MtFeErr_Ok;
#else
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_RDA5815;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x18;
			handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_RDA5815S_ct8k;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_RDA5815S_ct8k;
			handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
			handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;
			ret = MtFeErr_Ok;
#endif
#endif
			break;

		default:
			break;
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_close_ss2_ct8k
**
**	DESCRIPTION:
**		finalize Symphony2 dvbs or  dvbs2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ss2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	if((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.demod_type = MtFeType_Undef;
	handle->m_device_ss2.demod_current_type = MtFeType_Undef;

	if(handle->m_device_ctt2.demod_current_type == MtFeType_Undef)
		handle->bSysInitOk = FALSE;


	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_dmd_open_ss2_ct8k
**
**	DESCRIPTION:
**		initialize Symphony2 dvbs or dvbs2
**
**	IN:
**		MT_FE_CT8K_Device_Handle		handle
**		MT_FE_TYPE 	demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if((demod_type != MtFeType_DVBS) && 
	   (demod_type != MtFeType_DVBS2) && 
	   (demod_type != MtFeType_DVBS_S2))
	{
		demod_type = MtFeType_DVBS_S2;
	}

	if (handle->m_device_ss2.demod_current_type == demod_type)
		return ret;

#if 1
	if(((handle->m_device_ss2.demod_current_type == MtFeType_DVBS) || 
		(handle->m_device_ss2.demod_current_type == MtFeType_DVBS2) || 
		(handle->m_device_ss2.demod_current_type == MtFeType_DVBS_S2)) && 
	   ((demod_type == MtFeType_DVBS) || 
		(demod_type == MtFeType_DVBS2) || 
		(demod_type == MtFeType_DVBS_S2))
	  )
	{
		handle->m_device_ss2.demod_type = demod_type;
		handle->m_device_ss2.demod_current_type = handle->m_device_ss2.demod_type;

		if(handle->chip_version == 0x9000)		// Sym2 A0
			handle->m_device_ctt2.demod_current_type = MtFeType_Undef;

		return MtFeErr_Ok;
	}
#endif	// fix 104719 end

	if ((demod_type != MtFeType_DVBS) && (demod_type != MtFeType_DVBS2) && (demod_type != MtFeType_DVBS_S2))
	{
		ret = MtFeErr_Fail;
		return ret;
	}


	_mt_fe_dmd_set_reg_t2_ct8k(handle, 0xf0, 0x00);
	_mt_fe_dmd_set_reg_t2_ct8k(handle, 0x00, 0x01);


	//mt_fe_dmd_select_tuner_ss2_ct8k(handle, handle->m_device_ss2.tuner_cfg.tuner_type);

	if (handle->m_device_ctt2.demod_current_type != MtFeType_Undef)
	{
		if((handle->chip_version == 0x9000) || (!handle->bSupportDualOutput))		// Sym2 A0 or not support dual-ts output
		{
			mt_fe_dmd_close_ctt2_ct8k(handle);
		}
		else			// Sym2 A1 or later
		{
			if (((demod_type == MtFeType_DVBS2) || 
				 (demod_type == MtFeType_DVBS) || 
				 (demod_type == MtFeType_DVBS_S2)) && 
				((handle->m_device_ctt2.demod_current_type == MtFeType_DVBT2) || 
				 (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT) || 
				 (handle->m_device_ctt2.demod_current_type == MtFeType_DVBT_T2)))
				mt_fe_dmd_close_ctt2_ct8k(handle);
		}
	}


	ret = mt_fe_set_demod_type_ct8k(handle, MtFeDemodMode_DVBSS2, demod_type);


	if((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if(handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
	}


	//if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_init_ct8k_ss2(handle);
	}


	handle->m_device_ss2.demod_current_type = handle->m_device_ss2.demod_type;

	if(handle->chip_version == 0x9000)
	{
		handle->m_device_ctt2.demod_current_type = MtFeType_Undef;
	}


	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_connect_ss2_ct8k
**
**	DESCRIPTION:
**		connect to a special channel for dvbs or dvbs2
**
**	IN:
**
**		MT_FE_CT8K_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_ss2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);

	if (ret == MtFeErr_Ok)
	{
		U32 iFreqMHz = (handle->m_device_ss2.input_params.input_freq_kHz + 500) / 1000;
		U32 iSymRateKSs = handle->m_device_ss2.input_params.symbol_rate_KSs;
		MT_FE_TYPE dvbs_type = handle->m_device_ss2.demod_type;

		ret = mt_fe_dmd_connect_ct8k_ss2(handle, iFreqMHz, iSymRateKSs, dvbs_type);
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_get_lock_state_ss2_ct8k
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_CT8K_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_lock_state_ct8k_ss2(handle, p_state);
	}

	return ret;
}

/*	FUNCTIN:
**		mt_fe_dmd_soft_reset_ss2_ct8k
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_ss2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_soft_reset_ct8k_ss2(handle);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_sat_quality_ct8k_ss2(handle, p_percent);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	S8 	p_strength = 0;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_strength_ct8k_ss2(handle, &p_strength);
		*ssi_percent =(U8)(p_strength);
	}

	return 	ret;
}

MT_FE_RET mt_fe_dmd_blindscan_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, MtFeType_DVBS_S2);
	}

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_blindscan_ct8k_ss2(handle, begin_freq_MHz, end_freq_MHz, p_bs_info);
	}

#if 0
	{
		int iLogCnt = 0;
		U8 tmp = 0;

		printk("\n%s() %d, dump T2 registers\n", __FUNCTION__, __LINE__);

		for(iLogCnt = 0; iLogCnt < 0x100; iLogCnt ++)
		{
			_mt_fe_dmd_get_reg_t2_ct8k(handle, (U8)iLogCnt, &tmp);
			printk("%02x - %02x\n", (U8)iLogCnt, tmp);
			//if((iLogCnt & 0x07) == 0x07)
			//	printk("\n");
		}

		printk("\n%s() %d, dump SS2 registers\n", __FUNCTION__, __LINE__);

		for(iLogCnt = 0; iLogCnt < 0x100; iLogCnt ++)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, (U8)iLogCnt, &tmp);
			printk("%02x - %02x\n", (U8)iLogCnt, tmp);
			//if((iLogCnt & 0x07) == 0x07)
			//	printk("\n");
		}

		printk("\n%s() %d, dump tuner registers\n", __FUNCTION__, __LINE__);

		for(iLogCnt = 0; iLogCnt < 0x100; iLogCnt ++)
		{
			_mt_fe_tn_get_reg_ss2_ct8k(handle, (U8)iLogCnt, &tmp);
			printk("%02x - %02x\n", (U8)iLogCnt, tmp);
			//if((iLogCnt & 0x07) == 0x07)
			//	printk("\n");
		}
	}
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL bs_abort)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_blindscan_abort_ct8k_ss2(handle, bs_abort);

	return ret;
}

MT_FE_RET  mt_fe_dmd_set_LNB_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_set_LNB_ct8k_ss2(handle, is_LNB_enable, is_22k_enable, voltage_type, is_envelop_mode);

	return ret;
}

MT_FE_RET  mt_fe_dmd_DiSEqC_send_tone_burst_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_send_tone_burst_ct8k_ss2(handle, mode, is_envelop_mode);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_send_receive_msg_ct8k_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) &&
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) &&
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_send_msg_ct8k_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) &&
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) &&
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret =  mt_fe_dmd_open_ss2_ct8k(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_receive_msg_ct8k_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_set_OLF_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_OLF_enable)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	reg_00 = 0;

	_mt_fe_sys_get_reg_ct8k(handle, 0x1C, &reg_00);
	if(is_OLF_enable)
	{
		reg_00 = reg_00 | (1 << 7);
		_mt_fe_sys_set_reg_ct8k(handle, 0x1C, reg_00);
	}
	else
	{
		reg_00 = reg_00 & (~ (1 << 7));
		_mt_fe_sys_set_reg_ct8k(handle, 0x1C, reg_00);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_OLF_status_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_status)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8	reg_00 = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xA2, &reg_00);
	if(reg_00 & (1 << 2))
		*p_status =  1;
	else
		*p_status = 0;

	return ret;
}

#endif

