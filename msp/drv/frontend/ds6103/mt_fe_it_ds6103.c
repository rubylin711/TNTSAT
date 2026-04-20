/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2017 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*****************************************************************************/
/* Filename:      mt_fe_it_ds6103.c
 *
 * Description:   Montage M88DS6103 Digital Satellite Tuner IC driver.
 *
 * Author:        Jerock Qin
 * Version:       0.00.06
 * Date:          2017-11-27
 *****************************************************************************/
/*****************************************************************************
 *  Version     Date        Author      Description
 * ---------------------------------------------------------------------------
 *  0.00.00     2017.03.16  Kenny           Create
 *  0.00.01     2017.03.24  Youzhong        Modify
 *  0.00.02     2017.03.28  Kenny           Modify    R6E=0x39 ; R83=0x01
 *  0.00.03     2017.03.28  Lidan           Modify    support 250MHz
 *  0.00.04     2017.05.04  Kenny           modify ro increase agc reference 3dB R70=90 R71=F0 R72=B6 R73=EB R74=6F R75=FC
 *  0.00.05     2017.11.16  Kenny           modify
 *  0.00.06     2017.11.27  Kenny           modify
 *****************************************************************************/

#include "mt_fe_i2c_ds6103.h"


void mt_fe_it_wakeup_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	handle->it_set_reg(handle, 0x10, 0x01);
	handle->it_set_reg(handle, 0x11, 0x01);

	return;
}

void mt_fe_it_sleep_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	handle->it_set_reg(handle, 0x10, 0x00);
	handle->it_set_reg(handle, 0x11, 0x00);

	return;
}


void mt_fe_it_init_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	handle->it_set_reg(handle, 0x10, 0x01);
	handle->it_set_reg(handle, 0x11, 0x01);
	handle->it_set_reg(handle, 0x24, 0x04);
	handle->it_set_reg(handle, 0x84, 0x04);
	handle->it_set_reg(handle, 0x15, 0x6c);

	return;
}

void mt_fe_it_reset_ds6103(MT_FE_DS6103_Device_Handle handle)
{
	handle->it_set_reg(handle, 0x04, 0x01);
	handle->it_set_reg(handle, 0x04, 0x00);
	handle->mt_sleep(1);

	return;
}


void mt_fe_it_dmpll_select_xm_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *xm_KHz, U32 symbol_rate, U32 tuner_freq_MHz)
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


	handle->it_get_reg(handle, 0x16, &reg16);
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

void mt_fe_it_dmpll_select_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, U32 tuner_freq_MHz, MT_BOOL bBs)
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

	handle->it_get_reg(handle, 0x15, &reg15);
	reg15 &= ~0x01;

	//handle->it_set_reg(handle, 0x19, 0x07);

	handle->it_set_reg(handle, 0x15, reg15);
	handle->it_set_reg(handle, 0x16, reg16);

	handle->it_set_reg(handle, 0x17, 0xc1);
	handle->it_set_reg(handle, 0x17, 0x81);

	handle->mt_sleep(5);

	//handle->it_set_reg(handle, 0x19, 0x01);
	//handle->mt_sleep(1);

	return;
}


void mt_fe_it_dmpll_set_ts_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, MT_FE_TS_OUT_MODE ts_mode, U32 MCLK_KHz)
{
	U8 reg15, reg16, reg1D, reg1E, reg1F, tmp;
	U8 sm, f0 = 0, f1 = 0, f2 = 0, f3 = 0;
	U16 pll_div_fb, N;
	U32 div;

	handle->it_get_reg(handle, 0x15, &reg15);
	handle->it_get_reg(handle, 0x16, &reg16);
	handle->it_get_reg(handle, 0x1d, &reg1D);

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

	handle->it_set_reg(handle, 0x1d, reg1D);
	handle->it_set_reg(handle, 0x1e, reg1E);
	handle->it_set_reg(handle, 0x1f, reg1F);
	handle->mt_sleep(1);

	return;
}


void mt_fe_it_dmpll_get_ts_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_MCLK_KHz)
{
	U8 reg15, reg16, reg1D, reg1E, reg1F;
	U8 sm, f0, f1, f2, f3;
	U16 pll_div_fb, N;
	U32 MCLK_KHz;

	*p_MCLK_KHz = MT_FE_MCLK_KHZ;

	handle->it_get_reg(handle, 0x15, &reg15);
	handle->it_get_reg(handle, 0x16, &reg16);
	handle->it_get_reg(handle, 0x1d, &reg1D);
	handle->it_get_reg(handle, 0x1e, &reg1E);
	handle->it_get_reg(handle, 0x1f, &reg1F);

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

