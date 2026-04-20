/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "TP_TYPE.h"
#include "TP5001.h"
#include "IIC.h"

#ifdef _USE_TP5001_CHIP_

// ==========================================================================================================
// Marco & define
//
// #define FPGA_PLATFORM
// #define TUNER_CONTROL_BY_HOST

#define SERIES_PORT


#define SHARP_6306_TUNER 1
#define RDA_5812_TUNER 2
#define SHARP_6903_TUNER 3
#define S305_TUNER 4
#define AV2020_TUNER 5
#define RDA_5815_TUNER 6
#define RDA_5815M_TUNER 7
// ==========================================================================================================

// ==========================================================================================================
// Global Variable
//
const TP_UINT8 g_code_version[] = {"TP500101.62"};
TP_UINT32 g_symbol_rate;
TP_UINT8 gTuner_initialized = 0;
TP_UINT8 gxtal_sel;
TP_UINT8 gui_tuner_type;
TP_UINT8 g_chip_type;
// ==========================================================================================================


TP_UINT8 CalcNumberBits_Max32(TP_UINT32 number)
{
	int bits = 0;

	for (bits = 31; bits >= 0; bits--)
	{
		if ((number & (1 << bits)) != 0)
		{
			return bits;
		}
	}

	return 0;
}

TP_UINT32 MT_BigNumberMultiplyAndDivide(TP_UINT32 num_n, TP_UINT32 div_d, TP_UINT32 mul_m)
{
#if 0
	TP_UINT32 number = 0;
	TP_UINT8 bits_b = 0, bits_d = 0, bits_m = 0;

	do
	{
		bits_b = CalcNumberBits_Max32(num_n);
		bits_d = CalcNumberBits_Max32(div_d);
		bits_m = CalcNumberBits_Max32(mul_m);

		number += num_n / div_d * mul_m;
		num_n %= div_d;

	} while (bits_m > 0);

	return number;
#else
	TP_UINT32 number = num_n;

	number = (num_n / div_d) * mul_m + (num_n % div_d) * mul_m / div_d;

	return number;
#endif
}

// ==========================================================================================================
// Internal Function
//
// ----------------------------------------------------------------------------------------------------------
// Description: used by set_symbol_rate function
//
TP_UINT8 getN(TP_UINT32 symbol_rate, TP_UINT32 sample_clock)
{
	//TP_FLOAT A;
	TP_INT32 A;

	//A = (TP_FLOAT)sample_clock / (TP_FLOAT)symbol_rate;
	A = sample_clock / (symbol_rate / 100);

	printk("%s[%d] ---- sample_clock[%d], symbol_rate[%d], A[%d]\n", __FUNCTION__, __LINE__, sample_clock, symbol_rate, A);

#if 0
	if (A > 2.05 && A <= 3.34)
		return 1;
	else if (A > (2 * 1.67) && A <= (4 * 1.67))
		return 2;
	else if (A > (4 * 1.67) && A <= (8 * 1.67))
		return 4;
	else if (A > (8 * 1.67) && A <= (16 * 1.67))
		return 8;
	else if (A > (16 * 1.67) && A <= (32 * 1.67))
		return 16;
	else if (A > (32 * 1.67) && A <= (64 * 1.67))
		return 32;
	else
		return 0;
#else
	if (A > 205 && A <= 334)
		return 1;
	else if (A > (2 * 167) && A <= (4 * 167))
		return 2;
	else if (A > (4 * 167) && A <= (8 * 167))
		return 4;
	else if (A > (8 * 167) && A <= (16 * 167))
		return 8;
	else if (A > (16 * 167) && A <= (32 * 167))
		return 16;
	else if (A > (32 * 167) && A <= (64 * 167))
		return 32;
	else
		return 0;
#endif
}
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// Host iic write/read one byte
//
TP_UINT8 IIC_WRITE_ONE_BYTE(TP_UINT16 REG_ADDR, TP_UINT8 REG_VALUE)
{
	TP_UINT8 result;
	result = TP_iic_write(TP5001_DEVADDR, REG_ADDR, &REG_VALUE, 1);
	return result;
}

TP_UINT8 IIC_READ_ONE_BYTE(TP_UINT16 REG_ADDR, TP_UINT8 *REG_VALUE)
{
	TP_UINT8 result;
	result = TP_iic_read(TP5001_DEVADDR, REG_ADDR, REG_VALUE, 1);
	return result;
}
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// TP5001 IIC Master used for tuner, read and write
//
TP_UINT8 IIC_repeat_start_read(TP_UINT8 reg_addr, TP_UINT8 *read_buffer, TP_UINT32 read_length)
{
	TP_UINT16 tuner_addr;
	TP_UINT8 register_value;
	TP_UINT8 i, result;

	// �ȴ�0x0c bit0 is 1  cmd_idle
	i = 0;
	while (1)
	{
		result = IIC_READ_ONE_BYTE(0xFE0C, &register_value);
		if (result != TP_SUCCESS)
			return result;
		i++;
		if ((register_value & 0x01) == 1)
			break;
		if (i == 200)
			return TP_IIC_IDLE_ERR;
	}

	result = IIC_WRITE_ONE_BYTE(0xFE10, reg_addr);
	if (result != TP_SUCCESS)
		return result;

	register_value = 0x80 | (read_length - 1);
	result = IIC_WRITE_ONE_BYTE(0xFE08, register_value);
	if (result != TP_SUCCESS)
		return result;

	i = 0;
	while (1)
	{
		result = IIC_READ_ONE_BYTE(0xFE0C, &register_value);
		if (result != TP_SUCCESS)
			return result;
		i++;
		if ((register_value & 0x01) == 1)
		{
			if ((register_value & 0x06) != 0)
			{
				return TP_IIC_IDLE_ERR;
			}
			break;
		}
		if (i == 200)
			return TP_IIC_IDLE_ERR;
	}

	tuner_addr = 0xFE20;
	for (i = 0; i < read_length; i++)
	{
		result = IIC_READ_ONE_BYTE(tuner_addr, &read_buffer[i]);
		if (result != TP_SUCCESS)
			return result;
		tuner_addr++;
	}
	return TP_SUCCESS;
}

TP_UINT8 tuner_register_read(TP_UINT8 device_addr, TP_UINT8 reg_addr, TP_UINT8 *read_buffer, TP_UINT32 read_length)
{
#ifdef TUNER_CONTROL_BY_HOST
	return TP_iic_read_tuner(device_addr, reg_addr, read_buffer, read_length);
#else
	TP_UINT8 result;
	if (read_length > 16)
		return TP_TUNER_IIC_WR_TOO_LONG;

	result = IIC_repeat_start_read(reg_addr, read_buffer, read_length);
	printk("%s[%d] ---- dev_addr[0x%02x], reg_addr[0x%02x], read_len[%d], read_buf[0x%02x, 0x%02x]\n", 
			__FUNCTION__, __LINE__, device_addr, reg_addr, read_length, read_buffer[0], read_buffer[1]);
	if (result != TP_SUCCESS)
		return result;
#endif

	return TP_SUCCESS;
}

TP_UINT8 tuner_register_write(TP_UINT8 device_addr, TP_UINT8 *write_buffer, TP_UINT32 write_length)
{
#ifdef TUNER_CONTROL_BY_HOST
	return TP_iic_write_tuner(device_addr, write_buffer, write_length);
#else

	TP_UINT16 tuner_addr;
	TP_UINT8 register_value;
	TP_UINT8 i, result;

	if (write_length > 16)
	{
		//printk("%s[%d] ---- len [%d]\n", __FUNCTION__, __LINE__, write_length);
		return TP_TUNER_IIC_WR_TOO_LONG;
	}

	// �ȴ�0x0c bit0 is 1  cmd_idle
	i = 0;
	while (1)
	{
		result = IIC_READ_ONE_BYTE(0xFE0C, &register_value);
		if (result != TP_SUCCESS)
		{
			//printk("%s[%d] ---- [%3d] ---- 0xFE0C = [0x%02x] result = %d\n", __FUNCTION__, __LINE__, i, register_value, result);
			return result;
		}

		i++;
		if ((register_value & 0x01) == 1)
			break;

		if (i == 200)
		{
			//printk("%s[%d] ---- [%3d] ---- 0xFE0C = [0x%02x] result = %d\n", __FUNCTION__, __LINE__, i, register_value, result);

			return TP_IIC_IDLE_ERR;
		}
	}

	tuner_addr = 0xFE10;

	for (i = 0; i < write_length; i++)
	{
		result = IIC_WRITE_ONE_BYTE(tuner_addr, write_buffer[i]);
		if (result != TP_SUCCESS)
		{
			//printk("%s[%d] ---- result = %d\n", __FUNCTION__, __LINE__, result);

			return result;
		}
		tuner_addr++;
	}

	register_value = 0x40 | (write_length - 1);
	result = IIC_WRITE_ONE_BYTE(0xFE08, register_value);
	if (result != TP_SUCCESS)
	{
		//printk("%s[%d] ---- result = %d\n", __FUNCTION__, __LINE__, result);

		return result;
	}

	// �ȴ�0x0c bit0 is 1  cmd_idle
	i = 0;
	while (1)
	{
		result = IIC_READ_ONE_BYTE(0xFE0C, &register_value);
		if (result != TP_SUCCESS)
		{
			//printk("%s[%d] ---- [%3d] ---- 0xFE0C = [0x%02x] result = %d\n", __FUNCTION__, __LINE__, i, register_value, result);
			return result;
		}

		i++;
		if ((register_value & 0x01) == 1)
		{
			if ((register_value & 0x06) != 0)
			{
				//printk("%s[%d] ---- [%3d] ---- 0xFE0C = [0x%02x] result = %d\n", __FUNCTION__, __LINE__, i, register_value, result);

				return TP_IIC_IDLE_ERR;
			}
			break;
		}

		if (i == 200)
		{
			//printk("%s[%d] ---- [%3d] ---- 0xFE0C = [0x%02x] result = %d\n", __FUNCTION__, __LINE__, i, register_value, result);

			return TP_IIC_IDLE_ERR;
		}
	}

	return TP_SUCCESS;
#endif
}
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// tuner iic write
// device_addr: input, never to use, device addr must set after system_init(), write 0xFE01 register
// write_buffer: input, buffer to store reg + data
// write_length: input, length of reg + data
//
TP_UINT8 TP_iic_tuner_write(TP_UINT8 device_addr, TP_UINT8 *write_buffer, TP_UINT32 write_length)
{
	TP_UINT8 ret = TP_SUCCESS;

	ret = tuner_register_write(device_addr, write_buffer, write_length);

	//printk("%s[%d] ---- dev_addr[0x%02x], ret = %d\n", __FUNCTION__, __LINE__, device_addr, ret);

	return ret;
}
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// tuner iic read
// device_addr: input, never to use, device addr must set after system_init(), write 0xFE01 register
// reg_addr: register address
// value: read value buffer
// length: read value length
//
TP_UINT8 TP_iic_tuner_read(TP_UINT8 device_addr, TP_UINT8 reg_addr, TP_UINT8 *value, TP_UINT32 length)
{
	TP_UINT8 ret = TP_SUCCESS;

	ret = tuner_register_read(0, reg_addr, value, length);

	printk("%s[%d] ---- dev_addr[%02x], reg_addr[%02x], len[%d], value[%02x], ret = %d\n", 
			__FUNCTION__, __LINE__, device_addr, reg_addr, length, value[0], ret);

	return ret;
}
// ----------------------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------------------------
// system init: used for PLL ClockEnable IIC TS and some system level init
//
TP_UINT8 system_init(void)
{
	TP_UINT8 result;
	TP_UINT8 reg_value[5];

	// ��ʼ��IIC�˿�
	result = IIC_WRITE_ONE_BYTE(0xFF80, 0x00);
	//printk("%s[%d] ---- Log1, ret = %d\n", __FUNCTION__, __LINE__, result);
	if (result != TP_SUCCESS)
		return result;

	// check TP5001 chip ID
	result = TP_iic_read(TP5001_DEVADDR, 0x3F0F, reg_value, 2);
	//printk("%s[%d] ---- Log2, reg_value[0] = 0x%02x, reg_value[1] =  0x%02x, ret = %d\n", __FUNCTION__, __LINE__, reg_value[0], reg_value[1], result);
	if (result != TP_SUCCESS)
		return result;
	if (reg_value[0] != 0x50 || reg_value[1] != 0x01)
		return TP_CHIP_ID_ERROR;

#ifndef FPGA_PLATFORM
	// PLL CFG
	// PLL �ο�ʱ������ʹ��
	// PEG_CLK_SEL �½���
	// PLL LOCKӲ���Զ�ѡ��
	// PLL ��Ƶ����Ӳ���Զ�ѡ��
	// PLL IPʹ��

	// get xtal pin
	result = IIC_READ_ONE_BYTE(0x3F09, &gxtal_sel);
	if (result != TP_SUCCESS)
	{
		gxtal_sel = 0;
		return result;
	}

	//printk("%s[%d] ---- gxtal_sel = %d\n", __FUNCTION__, __LINE__, gxtal_sel);
	//gxtal_sel = 3;

	// clk div cfg :spi  sys ldpc
	if (gxtal_sel == 3)
	{
		reg_value[0] = 0x38;
		reg_value[1] = 0x05;
		reg_value[2] = 0x02;
		reg_value[3] = 0x05;
	}
	else
	{
		reg_value[0] = 0x25;
		reg_value[1] = 0x03;
		reg_value[2] = 0x01;
		reg_value[3] = 0x03;
	}
	result = TP_iic_write(TP5001_DEVADDR, 0x3F0B, reg_value, 4);
	if (result != TP_SUCCESS)
		return result;

	// set PLL =============================================
	result = IIC_WRITE_ONE_BYTE(0x3f00, 0xD7); // PDRST���ָߵ�ƽ
	if (result != TP_SUCCESS)
		return result;

	// XIN  <---->  clk cfg    PLL_OUT_CLK 400MHz
	// Pll_m_cfg Pll_od_cfg Pll_n_cfg
	// dvbspi_pdiv  Sys_pdiv_reg  Ldpc_pdiv_reg
	if (gxtal_sel == 0) // 4M
	{
		reg_value[0] = 190;
		reg_value[1] = 0x01;
		reg_value[2] = 0x01;
		reg_value[3] = 0x00; // low 6 bits
		reg_value[4] = 0x23; // high 8 bits
		result = TP_iic_write(TP5001_DEVADDR, 0x3F04, reg_value, 5);
		if (result != TP_SUCCESS)
			return result;
	}
	else if (gxtal_sel == 1) // 10M
	{
		reg_value[0] = 76;
		reg_value[1] = 0x01;
		reg_value[2] = 0x01;
		reg_value[3] = 0x00; // low 6 bits
		reg_value[4] = 0x54; // high 8 bits
		result = TP_iic_write(TP5001_DEVADDR, 0x3F04, reg_value, 5);
		if (result != TP_SUCCESS)
			return result;
	}
	else if (gxtal_sel == 2) // 20M
	{
		reg_value[0] = 76;
		reg_value[1] = 0x01;
		reg_value[2] = 0x02;
		reg_value[3] = 0x00; // low 6 bits
		reg_value[4] = 0xa8; // high 8 bits
		result = TP_iic_write(TP5001_DEVADDR, 0x3F04, reg_value, 5);
		if (result != TP_SUCCESS)
			return result;
	}
	else if (gxtal_sel == 3) // 27M
	{
		reg_value[0] = 0xBE;
		reg_value[1] = 0;
		reg_value[2] = 9;
		reg_value[3] = 0x00; // low 6 bits
		reg_value[4] = 0xe8; // high 8 bits
		result = TP_iic_write(TP5001_DEVADDR, 0x3F04, reg_value, 5);
		if (result != TP_SUCCESS)
			return result;
	}

	result = IIC_WRITE_ONE_BYTE(0x3f00, 0xD6); // ������M N OD ֮��PDRST ����
	if (result != TP_SUCCESS)
		return result;
	//==========end of set pll====================

	// set adc cfg0 and adc_en  ========================
	result = IIC_WRITE_ONE_BYTE(0x3f01, 0xF3); // 72<->10bit  73 <-> 8bit
	if (result != TP_SUCCESS)
		return result;
	result = IIC_WRITE_ONE_BYTE(0x3f03, 0x03); // 07 <->high 8 bit 03 <->low 8bit
	if (result != TP_SUCCESS)
		return result;
	// ========== end set adc ===========================
	// clk_en
	result = IIC_WRITE_ONE_BYTE(0x3f0A, 0x7F);
	if (result != TP_SUCCESS)
		return result;

#endif

	// set GPIO is output, default is 0
	result = IIC_WRITE_ONE_BYTE(0x3D01, 0xFF);
	if (result != TP_SUCCESS)
		return result;

	// ����TS�ӿ�
	if (gxtal_sel == 3)
		reg_value[0] = 0x38; //
	else
		reg_value[0] = 0x25;											  // clockΪ396(396.6)/40 = 9.9MHz
	result = TP_set_ts_interface(negitive_edge, para_port, reg_value[0]); // Ĭ�� �½��ز��� ���ж˿�
	if (result != TP_SUCCESS)
		return result;

#ifdef SERIES_PORT
	if (gxtal_sel == 3)
		reg_value[0] = 7; // 570/(7+1) = 71.25Mhz
	else
		reg_value[0] = 4;													// 380/(4+1) = 76MHz
	result = TP_set_ts_interface(negitive_edge, series_port, reg_value[0]); // Ĭ�� �½��ز��� ���ж˿�
	if (result != TP_SUCCESS)
		return result;
#endif

	//  IIC_MSTʹ��
	result = IIC_WRITE_ONE_BYTE(0xFE00, 0x01);
	if (result != TP_SUCCESS)
		return result;

	// ����Tuner IIC�ٶ�Ϊ400KHz Ĭ��clockΪ95MHz / 238 = 399KHz
	result = IIC_WRITE_ONE_BYTE(0xFE02, 0xEA);
	if (result != TP_SUCCESS)
		return result;

	// set ldpc output mode, ����������ݶ���?
	result = IIC_WRITE_ONE_BYTE(0x0A10, 0x04);
	if (result != TP_SUCCESS)
		return result;
	return TP_SUCCESS;
}

// ==========================================================================================================

// ==========================================================================================================
// External Function
//

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_version
// Description: TP5001оƬ���õ��汾��
// Output:
//		pVersion: ���SDK�汾�ŵ�Buffer����Ҫ16�ֽڵĴ洢�ռ�
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_get_version(TP_INT8 *pVersion)
{
	TP_UINT8 i;

	for (i = 0; i < sizeof(g_code_version); i++)
	{
		*(pVersion + i) = g_code_version[i];
	}

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_symbol_rate
// Description: TP5001оƬ�����÷�����
// Input:
//		symbol_rate:  �����ʣ���λHz
// Return:
//		TP_SUCCESS: �����ɹ�
//

TP_UINT8 TP_set_symbol_rate(TP_UINT32 symbol_rate)
{
	//TP_FLOAT temp;
	TP_UINT32 temp;
	TP_UINT32 res;
	TP_UINT8 result;
	TP_UINT8 reg_value[8];
	TP_UINT32 sample_rate;

	//TP_UINT64 tmp;

	TP_UINT32 tmp1, tmp2, tmp3, tmp4, tmp5;

	//printk("%s[%d] ---- g_symbol_rate = %d, symbol_rate = %d\n", __FUNCTION__, __LINE__, g_symbol_rate, symbol_rate);

	g_symbol_rate = symbol_rate;

	//printk("%s[%d] ---- g_symbol_rate = %d, symbol_rate = %d\n", __FUNCTION__, __LINE__, g_symbol_rate, symbol_rate);

	if (symbol_rate >= 20000000)
	{
		if (gui_tuner_type == SHARP_6306_TUNER)
		{
			reg_value[0] = 0x16; // wnr change
			reg_value[1] = 0x06; // wnr change
		}
		else
		{
			reg_value[0] = 0x15; // wnr change   15
			reg_value[1] = 0x05; // wnr change   05
		}
	}
	else if (symbol_rate >= 8000000)
	{
		reg_value[0] = 0x14; // wnr change   15
		reg_value[1] = 0x04; // wnr change   05
	}
	else if (symbol_rate >= 3000000)
	{
		reg_value[0] = 0x12; // wnr change   15
		reg_value[1] = 0x02; // wnr change   05
	}
	else
	{
		reg_value[0] = 0x11; // wnr change   15
		reg_value[1] = 0x01; // wnr change   05
	}

	//printk("%s[%d] ---- reg_value[0] = 0x%02x, reg_value[1] = 0x%02x\n", __FUNCTION__, __LINE__, reg_value[0], reg_value[1]);

	result = TP_iic_write(TP5001_DEVADDR, 0x0538, reg_value, 2);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x0538 ret = %d\n", __FUNCTION__, __LINE__, result);

	// demod soft reset
	result = IIC_WRITE_ONE_BYTE(0x1100, 0xfe);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x1100[0xfe] ret = %d\n", __FUNCTION__, __LINE__, result);

	result = IIC_WRITE_ONE_BYTE(0x1100, 0xfc);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x1100[0xfc] ret = %d\n", __FUNCTION__, __LINE__, result);

#ifndef FPGA_PLATFORM
	sample_rate = 95000000;
#else
	sample_rate = 60000000;
#endif

	//temp = (TP_FLOAT)sample_rate;
	temp = sample_rate;

	//printk("%s[%d] ---- temp = sample_rate = %d\n", __FUNCTION__, __LINE__, temp);

	// ���㼰����bitsync����

#if 1
	temp /= getN(symbol_rate, sample_rate);

	//printk("%s[%d] ---- temp = %d\n", __FUNCTION__, __LINE__, temp);

	//temp /= symbol_rate;
	//temp *= 1048576; // 1<<20
	//temp = ((temp / symbol_rate) << 20) + ((temp % symbol_rate) << 20) / symbol_rate;

	//tmp = (TP_UINT64)temp * 1048576 / symbol_rate;
	//printk("%s[%d] ---- tmp = %d\n", __FUNCTION__, __LINE__, tmp);

	tmp1 = temp % symbol_rate;
	tmp2 = tmp1 * (1 << 4) % symbol_rate;
	tmp3 = tmp2 * (1 << 4) % symbol_rate;
	tmp4 = tmp3 * (1 << 4) % symbol_rate;
	tmp5 = tmp4 * (1 << 4) % symbol_rate;
	temp = ((temp / symbol_rate) * (1 << 20)) + 
		   ((tmp1 * (1 << 4) / symbol_rate) * (1 << 16)) + 
		   ((tmp2 * (1 << 4) / symbol_rate) * (1 << 12)) + 
		   ((tmp3 * (1 << 4) / symbol_rate) * (1 <<  8)) + 
		   ((tmp4 * (1 << 4) / symbol_rate) * (1 <<  4)) + 
		   ((tmp5 * (1 << 4) / symbol_rate) * (1 <<  0));

	//printk("%s[%d] ---- tmp1 = %d, tmp2 = %d, tmp3 = %d, tmp4 = %d, tmp4 = %d\n", __FUNCTION__, __LINE__, tmp1, tmp2, tmp3, tmp4, tmp5);
	
	//printk("%s[%d] ---- temp = %d\n", __FUNCTION__, __LINE__, temp);

	temp /= 2;

	//printk("%s[%d] ---- temp = %d\n", __FUNCTION__, __LINE__, temp);

#else
	temp /= getN(symbol_rate, sample_rate);
	temp /= symbol_rate;
	temp *= 1048576; // 1<<20
	temp /= 2;
#endif

	res = (TP_UINT32)temp;
	reg_value[0] = (res)&0xFF;
	reg_value[1] = (res >> 8) & 0xFF;
	reg_value[2] = (res >> 16) & 0xFF;
	result = TP_iic_write(TP5001_DEVADDR, 0x0309, reg_value, 3);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x0309 ret = %d\n", __FUNCTION__, __LINE__, result);

	// 	NCO Decimation Ratio
#if 1
	temp = symbol_rate;

	//printk("%s[%d] ---- temp = symbol_rate = %d\n", __FUNCTION__, __LINE__, temp);


	//temp /= sample_rate; // ADC������
	//temp *= 32768;
	//temp = ((temp / sample_rate) << 15) + ((temp % sample_rate) << 15) / sample_rate;

	//tmp = (TP_UINT64)temp * 32768 / sample_rate;
	//printk("%s[%d] ---- tmp = %d\n", __FUNCTION__, __LINE__, tmp);
	
	tmp1 = temp % sample_rate;
	tmp2 = tmp1 * (1 << 5) % sample_rate;
	tmp3 = tmp2 * (1 << 5) % sample_rate;
	temp = ((temp / sample_rate) * (1 << 15)) + 
		   ((tmp1 * (1 << 5) / sample_rate) * (1 << 10)) + 
		   ((tmp2 * (1 << 5) / sample_rate) * (1 <<  5)) + 
		   ((tmp3 * (1 << 5) / sample_rate) * (1 <<  0));

	//printk("%s[%d] ---- tmp1 = %d, tmp2 = %d, tmp3 = %d\n", __FUNCTION__, __LINE__, tmp1, tmp2, tmp3);

	//printk("%s[%d] ---- temp = %d\n", __FUNCTION__, __LINE__, temp);

#else
	temp = (TP_FLOAT)symbol_rate;
	temp /= sample_rate; // ADC������
	temp *= 32768;
#endif

	//printk("%s[%d] ---- temp = 0x%08x\n", __FUNCTION__, __LINE__, temp);


	res = (TP_UINT32)temp;
	reg_value[0] = res & 0xFF;
	reg_value[1] = (res >> 8) & 0x7F;
	result = TP_iic_write(TP5001_DEVADDR, 0x0114, reg_value, 2);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x0114 ret = %d\n", __FUNCTION__, __LINE__, result);

	// FIR
	reg_value[0] = getN(symbol_rate, sample_rate);
	switch (reg_value[0])
	{
	case 1:
		reg_value[0] = 0;
		break;
	case 2:
		reg_value[0] = 1;
		break;
	case 4:
		reg_value[0] = 2;
		break;
	case 8:
		reg_value[0] = 3;
		break;
	case 16:
		reg_value[0] = 4;
		break;
	case 32:
		reg_value[0] = 5;
		break;
	default:
		break;
	}
	result = IIC_WRITE_ONE_BYTE(0x011f, reg_value[0]);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x011f ret = %d\n", __FUNCTION__, __LINE__, result);

	result = IIC_WRITE_ONE_BYTE(0x0D04, 0x3a);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x0D04 ret = %d\n", __FUNCTION__, __LINE__, result);

	// ���?
	result = IIC_WRITE_ONE_BYTE(0x1100, 0xfd);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x1100 ret = %d\n", __FUNCTION__, __LINE__, result);

	// enable adc output
	result = IIC_WRITE_ONE_BYTE(0x3F01, 0x73);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Write 0x3F01 ret = %d\n", __FUNCTION__, __LINE__, result);

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_signal_quality
// Description: TPоƬ���õ��ź�����
// Output:
//		p_quality_percent:  ���� 0: 0%  --   100: 100%
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_get_signal_quality(TP_UINT8 *p_quality_percent)
{
	TP_UINT8 result;
	TP_UINT8 reg_buffer[5];
	TP_UINT32 beta, total;
	//TP_FLOAT chn_pwr, noise_pwr;
	TP_UINT32 signal_pwr, noise_pwr;
	TP_UINT8 chn_pwr;
	TP_UINT32 temp;
	TP_UINT8 i;
	TP_UINT8 mean_count;

	result = IIC_READ_ONE_BYTE(0x050d, reg_buffer);
	if (result != TP_SUCCESS)
		return result;

	if ((reg_buffer[0] & 0x01) == 0x01)
	{

		beta = total = 0;
		result = IIC_READ_ONE_BYTE(0x110d, reg_buffer);
		if (result != TP_SUCCESS)
			return result;
		if (reg_buffer[0] == 0x80)
			mean_count = 100;
		else
			mean_count = 1;

		for (i = 0; i < mean_count; i++)
		{

			result = TP_iic_read(TP5001_DEVADDR, 0x054d, reg_buffer, 5);
			if (result != TP_SUCCESS)
				return result;

			temp = reg_buffer[0] | (reg_buffer[1] << 8) | ((reg_buffer[2] & 0x03) << 16);

			beta += temp;

			temp = reg_buffer[3] | ((reg_buffer[4] & 0x7f) << 8);

			total += temp;
		}

#if 1
		signal_pwr = beta;
		noise_pwr = total;

		if (noise_pwr == 0)
			chn_pwr = 0;
		else
			signal_pwr = (signal_pwr / noise_pwr) * 10000 + (signal_pwr % noise_pwr) * 10000 / noise_pwr;

		if (signal_pwr < 10000)
			chn_pwr = 0;
		else if (signal_pwr < 10233)
			chn_pwr = 1;
		else if (signal_pwr < 10471)
			chn_pwr = 2;
		else if (signal_pwr < 10715)
			chn_pwr = 3;
		else if (signal_pwr < 10965)
			chn_pwr = 4;
		else if (signal_pwr < 11220)
			chn_pwr = 5;
		else if (signal_pwr < 11482)
			chn_pwr = 6;
		else if (signal_pwr < 11749)
			chn_pwr = 7;
		else if (signal_pwr < 12023)
			chn_pwr = 8;
		else if (signal_pwr < 12303)
			chn_pwr = 9;
		else if (signal_pwr < 12589)
			chn_pwr = 10;
		else if (signal_pwr < 12882)
			chn_pwr = 11;
		else if (signal_pwr < 13183)
			chn_pwr = 12;
		else if (signal_pwr < 13490)
			chn_pwr = 13;
		else if (signal_pwr < 13804)
			chn_pwr = 14;
		else if (signal_pwr < 14125)
			chn_pwr = 15;
		else if (signal_pwr < 14454)
			chn_pwr = 16;
		else if (signal_pwr < 14791)
			chn_pwr = 17;
		else if (signal_pwr < 15136)
			chn_pwr = 18;
		else if (signal_pwr < 15488)
			chn_pwr = 19;
		else if (signal_pwr < 15849)
			chn_pwr = 20;
		else if (signal_pwr < 16218)
			chn_pwr = 21;
		else if (signal_pwr < 16596)
			chn_pwr = 22;
		else if (signal_pwr < 16982)
			chn_pwr = 23;
		else if (signal_pwr < 17378)
			chn_pwr = 24;
		else if (signal_pwr < 17783)
			chn_pwr = 25;
		else if (signal_pwr < 18197)
			chn_pwr = 26;
		else if (signal_pwr < 18621)
			chn_pwr = 27;
		else if (signal_pwr < 19055)
			chn_pwr = 28;
		else if (signal_pwr < 19498)
			chn_pwr = 29;
		else if (signal_pwr < 19953)
			chn_pwr = 30;
		else if (signal_pwr < 20417)
			chn_pwr = 31;
		else if (signal_pwr < 20893)
			chn_pwr = 32;
		else if (signal_pwr < 21380)
			chn_pwr = 33;
		else if (signal_pwr < 21878)
			chn_pwr = 34;
		else if (signal_pwr < 22387)
			chn_pwr = 35;
		else if (signal_pwr < 22909)
			chn_pwr = 36;
		else if (signal_pwr < 23442)
			chn_pwr = 37;
		else if (signal_pwr < 23988)
			chn_pwr = 38;
		else if (signal_pwr < 24547)
			chn_pwr = 39;
		else if (signal_pwr < 25119)
			chn_pwr = 40;
		else if (signal_pwr < 25704)
			chn_pwr = 41;
		else if (signal_pwr < 26303)
			chn_pwr = 42;
		else if (signal_pwr < 26915)
			chn_pwr = 43;
		else if (signal_pwr < 27542)
			chn_pwr = 44;
		else if (signal_pwr < 28184)
			chn_pwr = 45;
		else if (signal_pwr < 28840)
			chn_pwr = 46;
		else if (signal_pwr < 29512)
			chn_pwr = 47;
		else if (signal_pwr < 30200)
			chn_pwr = 48;
		else if (signal_pwr < 30903)
			chn_pwr = 49;
		else if (signal_pwr < 31623)
			chn_pwr = 50;
		else if (signal_pwr < 32359)
			chn_pwr = 51;
		else if (signal_pwr < 33113)
			chn_pwr = 52;
		else if (signal_pwr < 33884)
			chn_pwr = 53;
		else if (signal_pwr < 34674)
			chn_pwr = 54;
		else if (signal_pwr < 35481)
			chn_pwr = 55;
		else if (signal_pwr < 36308)
			chn_pwr = 56;
		else if (signal_pwr < 37154)
			chn_pwr = 57;
		else if (signal_pwr < 38019)
			chn_pwr = 58;
		else if (signal_pwr < 38905)
			chn_pwr = 59;
		else if (signal_pwr < 39811)
			chn_pwr = 60;
		else if (signal_pwr < 40738)
			chn_pwr = 61;
		else if (signal_pwr < 41687)
			chn_pwr = 62;
		else if (signal_pwr < 42658)
			chn_pwr = 63;
		else if (signal_pwr < 43652)
			chn_pwr = 64;
		else if (signal_pwr < 44668)
			chn_pwr = 65;
		else if (signal_pwr < 45709)
			chn_pwr = 66;
		else if (signal_pwr < 46774)
			chn_pwr = 67;
		else if (signal_pwr < 47863)
			chn_pwr = 68;
		else if (signal_pwr < 48978)
			chn_pwr = 69;
		else if (signal_pwr < 50119)
			chn_pwr = 70;
		else if (signal_pwr < 52481)
			chn_pwr = 71;
		else if (signal_pwr < 54954)
			chn_pwr = 72;
		else if (signal_pwr < 57544)
			chn_pwr = 73;
		else if (signal_pwr < 60256)
			chn_pwr = 74;
		else if (signal_pwr < 63096)
			chn_pwr = 75;
		else if (signal_pwr < 66069)
			chn_pwr = 76;
		else if (signal_pwr < 69183)
			chn_pwr = 77;
		else if (signal_pwr < 72444)
			chn_pwr = 78;
		else if (signal_pwr < 75858)
			chn_pwr = 79;
		else if (signal_pwr < 79433)
			chn_pwr = 80;
		else if (signal_pwr < 83176)
			chn_pwr = 81;
		else if (signal_pwr < 87096)
			chn_pwr = 82;
		else if (signal_pwr < 91201)
			chn_pwr = 83;
		else if (signal_pwr < 95499)
			chn_pwr = 84;
		else if (signal_pwr < 100000)
			chn_pwr = 85;
		else if (signal_pwr < 104713)
			chn_pwr = 86;
		else if (signal_pwr < 109648)
			chn_pwr = 87;
		else if (signal_pwr < 114815)
			chn_pwr = 88;
		else if (signal_pwr < 120226)
			chn_pwr = 89;
		else if (signal_pwr < 125893)
			chn_pwr = 90;
		else if (signal_pwr < 131826)
			chn_pwr = 91;
		else if (signal_pwr < 138038)
			chn_pwr = 92;
		else if (signal_pwr < 144544)
			chn_pwr = 93;
		else if (signal_pwr < 151356)
			chn_pwr = 94;
		else if (signal_pwr < 158489)
			chn_pwr = 95;
		else if (signal_pwr < 165959)
			chn_pwr = 96;
		else if (signal_pwr < 173780)
			chn_pwr = 97;
		else if (signal_pwr < 181970)
			chn_pwr = 98;
		else if (signal_pwr < 190546)
			chn_pwr = 99;
		else
			chn_pwr = 100;
	}
	else
	{
		chn_pwr = 0;
	}
#else
		chn_pwr = (TP_FLOAT)beta;
		noise_pwr = (TP_FLOAT)total;

		if (noise_pwr == 0)
			chn_pwr = 0;
		else
			chn_pwr /= noise_pwr;

		if (chn_pwr < 1)
			chn_pwr = 0;
		else if (chn_pwr < 1.0233)
			chn_pwr = 1;
		else if (chn_pwr < 1.0471)
			chn_pwr = 2;
		else if (chn_pwr < 1.0715)
			chn_pwr = 3;
		else if (chn_pwr < 1.0965)
			chn_pwr = 4;
		else if (chn_pwr < 1.1220)
			chn_pwr = 5;
		else if (chn_pwr < 1.1482)
			chn_pwr = 6;
		else if (chn_pwr < 1.1749)
			chn_pwr = 7;
		else if (chn_pwr < 1.2023)
			chn_pwr = 8;
		else if (chn_pwr < 1.2303)
			chn_pwr = 9;
		else if (chn_pwr < 1.2589)
			chn_pwr = 10;
		else if (chn_pwr < 1.2882)
			chn_pwr = 11;
		else if (chn_pwr < 1.3183)
			chn_pwr = 12;
		else if (chn_pwr < 1.3490)
			chn_pwr = 13;
		else if (chn_pwr < 1.3804)
			chn_pwr = 14;
		else if (chn_pwr < 1.4125)
			chn_pwr = 15;
		else if (chn_pwr < 1.4454)
			chn_pwr = 16;
		else if (chn_pwr < 1.4791)
			chn_pwr = 17;
		else if (chn_pwr < 1.5136)
			chn_pwr = 18;
		else if (chn_pwr < 1.5488)
			chn_pwr = 19;
		else if (chn_pwr < 1.5849)
			chn_pwr = 20;
		else if (chn_pwr < 1.6218)
			chn_pwr = 21;
		else if (chn_pwr < 1.6596)
			chn_pwr = 22;
		else if (chn_pwr < 1.6982)
			chn_pwr = 23;
		else if (chn_pwr < 1.7378)
			chn_pwr = 24;
		else if (chn_pwr < 1.7783)
			chn_pwr = 25;
		else if (chn_pwr < 1.8197)
			chn_pwr = 26;
		else if (chn_pwr < 1.8621)
			chn_pwr = 27;
		else if (chn_pwr < 1.9055)
			chn_pwr = 28;
		else if (chn_pwr < 1.9498)
			chn_pwr = 29;
		else if (chn_pwr < 1.9953)
			chn_pwr = 30;
		else if (chn_pwr < 2.0417)
			chn_pwr = 31;
		else if (chn_pwr < 2.0893)
			chn_pwr = 32;
		else if (chn_pwr < 2.1380)
			chn_pwr = 33;
		else if (chn_pwr < 2.1878)
			chn_pwr = 34;
		else if (chn_pwr < 2.2387)
			chn_pwr = 35;
		else if (chn_pwr < 2.2909)
			chn_pwr = 36;
		else if (chn_pwr < 2.3442)
			chn_pwr = 37;
		else if (chn_pwr < 2.3988)
			chn_pwr = 38;
		else if (chn_pwr < 2.4547)
			chn_pwr = 39;
		else if (chn_pwr < 2.5119)
			chn_pwr = 40;
		else if (chn_pwr < 2.5704)
			chn_pwr = 41;
		else if (chn_pwr < 2.6303)
			chn_pwr = 42;
		else if (chn_pwr < 2.6915)
			chn_pwr = 43;
		else if (chn_pwr < 2.7542)
			chn_pwr = 44;
		else if (chn_pwr < 2.8184)
			chn_pwr = 45;
		else if (chn_pwr < 2.8840)
			chn_pwr = 46;
		else if (chn_pwr < 2.9512)
			chn_pwr = 47;
		else if (chn_pwr < 3.0200)
			chn_pwr = 48;
		else if (chn_pwr < 3.0903)
			chn_pwr = 49;
		else if (chn_pwr < 3.1623)
			chn_pwr = 50;
		else if (chn_pwr < 3.2359)
			chn_pwr = 51;
		else if (chn_pwr < 3.3113)
			chn_pwr = 52;
		else if (chn_pwr < 3.3884)
			chn_pwr = 53;
		else if (chn_pwr < 3.4674)
			chn_pwr = 54;
		else if (chn_pwr < 3.5481)
			chn_pwr = 55;
		else if (chn_pwr < 3.6308)
			chn_pwr = 56;
		else if (chn_pwr < 3.7154)
			chn_pwr = 57;
		else if (chn_pwr < 3.8019)
			chn_pwr = 58;
		else if (chn_pwr < 3.8905)
			chn_pwr = 59;
		else if (chn_pwr < 3.9811)
			chn_pwr = 60;
		else if (chn_pwr < 4.0738)
			chn_pwr = 61;
		else if (chn_pwr < 4.1687)
			chn_pwr = 62;
		else if (chn_pwr < 4.2658)
			chn_pwr = 63;
		else if (chn_pwr < 4.3652)
			chn_pwr = 64;
		else if (chn_pwr < 4.4668)
			chn_pwr = 65;
		else if (chn_pwr < 4.5709)
			chn_pwr = 66;
		else if (chn_pwr < 4.6774)
			chn_pwr = 67;
		else if (chn_pwr < 4.7863)
			chn_pwr = 68;
		else if (chn_pwr < 4.8978)
			chn_pwr = 69;
		else if (chn_pwr < 5.0119)
			chn_pwr = 70;
		else if (chn_pwr < 5.2481)
			chn_pwr = 71;
		else if (chn_pwr < 5.4954)
			chn_pwr = 72;
		else if (chn_pwr < 5.7544)
			chn_pwr = 73;
		else if (chn_pwr < 6.0256)
			chn_pwr = 74;
		else if (chn_pwr < 6.3096)
			chn_pwr = 75;
		else if (chn_pwr < 6.6069)
			chn_pwr = 76;
		else if (chn_pwr < 6.9183)
			chn_pwr = 77;
		else if (chn_pwr < 7.2444)
			chn_pwr = 78;
		else if (chn_pwr < 7.5858)
			chn_pwr = 79;
		else if (chn_pwr < 7.9433)
			chn_pwr = 80;
		else if (chn_pwr < 8.3176)
			chn_pwr = 81;
		else if (chn_pwr < 8.7096)
			chn_pwr = 82;
		else if (chn_pwr < 9.1201)
			chn_pwr = 83;
		else if (chn_pwr < 9.5499)
			chn_pwr = 84;
		else if (chn_pwr < 10.0000)
			chn_pwr = 85;
		else if (chn_pwr < 10.4713)
			chn_pwr = 86;
		else if (chn_pwr < 10.9648)
			chn_pwr = 87;
		else if (chn_pwr < 11.4815)
			chn_pwr = 88;
		else if (chn_pwr < 12.0226)
			chn_pwr = 89;
		else if (chn_pwr < 12.5893)
			chn_pwr = 90;
		else if (chn_pwr < 13.1826)
			chn_pwr = 91;
		else if (chn_pwr < 13.8038)
			chn_pwr = 92;
		else if (chn_pwr < 14.4544)
			chn_pwr = 93;
		else if (chn_pwr < 15.1356)
			chn_pwr = 94;
		else if (chn_pwr < 15.8489)
			chn_pwr = 95;
		else if (chn_pwr < 16.5959)
			chn_pwr = 96;
		else if (chn_pwr < 17.3780)
			chn_pwr = 97;
		else if (chn_pwr < 18.1970)
			chn_pwr = 98;
		else if (chn_pwr < 19.0546)
			chn_pwr = 99;
		else
			chn_pwr = 100;
	}
	else
	{
		chn_pwr = 0;
	}
#endif

	*p_quality_percent = (TP_UINT8)chn_pwr;

	return TP_SUCCESS;
}

#if 1
TP_UINT8 TP_get_signal_quality_DB(TP_INT32 *p_quality_DB)
{
	TP_UINT8 result;
	TP_UINT8 reg_buffer[5];
	TP_UINT32 beta, total;
	//TP_FLOAT chn_pwr, noise_pwr;
	TP_UINT32 signal_pwr, noise_pwr;
	TP_INT32 chn_pwr;
	TP_UINT32 temp;
	TP_UINT8 i;
	TP_UINT8 mean_count;

	result = IIC_READ_ONE_BYTE(0x050d, reg_buffer);
	if (result != TP_SUCCESS)
		return result;

	if ((reg_buffer[0] & 0x01) == 0x01)
	{

		beta = total = 0;
		result = IIC_READ_ONE_BYTE(0x110d, reg_buffer);
		if (result != TP_SUCCESS)
			return result;
		if (reg_buffer[0] == 0x80)
			mean_count = 100;
		else
			mean_count = 1;

		for (i = 0; i < mean_count; i++)
		{

			result = TP_iic_read(TP5001_DEVADDR, 0x054d, reg_buffer, 5);
			if (result != TP_SUCCESS)
				return result;

			temp = reg_buffer[0] | (reg_buffer[1] << 8) | ((reg_buffer[2] & 0x03) << 16);

			beta += temp;

			temp = reg_buffer[3] | ((reg_buffer[4] & 0x7f) << 8);

			total += temp;
		}

		signal_pwr = beta;
		noise_pwr = total;

		if (signal_pwr == 0)
			chn_pwr = 0;
		else
			//chn_pwr /= noise_pwr;
			signal_pwr = (signal_pwr / noise_pwr) * 10000 + (signal_pwr % noise_pwr) * 10000 / noise_pwr;

		if (signal_pwr < 10000)
			chn_pwr = 0;
		else if (signal_pwr < 10233)
			chn_pwr = 1;
		else if (signal_pwr < 10471)
			chn_pwr = 2;
		else if (signal_pwr < 10715)
			chn_pwr = 3;
		else if (signal_pwr < 10965)
			chn_pwr = 4;
		else if (signal_pwr < 11220)
			chn_pwr = 5;
		else if (signal_pwr < 11482)
			chn_pwr = 6;
		else if (signal_pwr < 11749)
			chn_pwr = 7;
		else if (signal_pwr < 12023)
			chn_pwr = 8;
		else if (signal_pwr < 12303)
			chn_pwr = 9;
		else if (signal_pwr < 12589)
			chn_pwr = 10;
		else if (signal_pwr < 12882)
			chn_pwr = 11;
		else if (signal_pwr < 13183)
			chn_pwr = 12;
		else if (signal_pwr < 13490)
			chn_pwr = 13;
		else if (signal_pwr < 13804)
			chn_pwr = 14;
		else if (signal_pwr < 14125)
			chn_pwr = 15;
		else if (signal_pwr < 14454)
			chn_pwr = 16;
		else if (signal_pwr < 14791)
			chn_pwr = 17;
		else if (signal_pwr < 15136)
			chn_pwr = 18;
		else if (signal_pwr < 15488)
			chn_pwr = 19;
		else if (signal_pwr < 15849)
			chn_pwr = 20;
		else if (signal_pwr < 16218)
			chn_pwr = 21;
		else if (signal_pwr < 16596)
			chn_pwr = 22;
		else if (signal_pwr < 16982)
			chn_pwr = 23;
		else if (signal_pwr < 17378)
			chn_pwr = 24;
		else if (signal_pwr < 17783)
			chn_pwr = 25;
		else if (signal_pwr < 18197)
			chn_pwr = 26;
		else if (signal_pwr < 18621)
			chn_pwr = 27;
		else if (signal_pwr < 19055)
			chn_pwr = 28;
		else if (signal_pwr < 19498)
			chn_pwr = 29;
		else if (signal_pwr < 19953)
			chn_pwr = 30;
		else if (signal_pwr < 20417)
			chn_pwr = 31;
		else if (signal_pwr < 20893)
			chn_pwr = 32;
		else if (signal_pwr < 21380)
			chn_pwr = 33;
		else if (signal_pwr < 21878)
			chn_pwr = 34;
		else if (signal_pwr < 22387)
			chn_pwr = 35;
		else if (signal_pwr < 22909)
			chn_pwr = 36;
		else if (signal_pwr < 23442)
			chn_pwr = 37;
		else if (signal_pwr < 23988)
			chn_pwr = 38;
		else if (signal_pwr < 24547)
			chn_pwr = 39;
		else if (signal_pwr < 25119)
			chn_pwr = 40;
		else if (signal_pwr < 25704)
			chn_pwr = 41;
		else if (signal_pwr < 26303)
			chn_pwr = 42;
		else if (signal_pwr < 26915)
			chn_pwr = 43;
		else if (signal_pwr < 27542)
			chn_pwr = 44;
		else if (signal_pwr < 28184)
			chn_pwr = 45;
		else if (signal_pwr < 28840)
			chn_pwr = 46;
		else if (signal_pwr < 29512)
			chn_pwr = 47;
		else if (signal_pwr < 30200)
			chn_pwr = 48;
		else if (signal_pwr < 30903)
			chn_pwr = 49;
		else if (signal_pwr < 31623)
			chn_pwr = 50;
		else if (signal_pwr < 32359)
			chn_pwr = 51;
		else if (signal_pwr < 33113)
			chn_pwr = 52;
		else if (signal_pwr < 33884)
			chn_pwr = 53;
		else if (signal_pwr < 34674)
			chn_pwr = 54;
		else if (signal_pwr < 35481)
			chn_pwr = 55;
		else if (signal_pwr < 36308)
			chn_pwr = 56;
		else if (signal_pwr < 37154)
			chn_pwr = 57;
		else if (signal_pwr < 38019)
			chn_pwr = 58;
		else if (signal_pwr < 38905)
			chn_pwr = 59;
		else if (signal_pwr < 39811)
			chn_pwr = 60;
		else if (signal_pwr < 40738)
			chn_pwr = 61;
		else if (signal_pwr < 41687)
			chn_pwr = 62;
		else if (signal_pwr < 42658)
			chn_pwr = 63;
		else if (signal_pwr < 43652)
			chn_pwr = 64;
		else if (signal_pwr < 44668)
			chn_pwr = 65;
		else if (signal_pwr < 45709)
			chn_pwr = 66;
		else if (signal_pwr < 46774)
			chn_pwr = 67;
		else if (signal_pwr < 47863)
			chn_pwr = 68;
		else if (signal_pwr < 48978)
			chn_pwr = 69;
		else if (signal_pwr < 50119)
			chn_pwr = 70;
		else if (signal_pwr < 52481)
			chn_pwr = 72;
		else if (signal_pwr < 54954)
			chn_pwr = 74;
		else if (signal_pwr < 57544)
			chn_pwr = 76;
		else if (signal_pwr < 60256)
			chn_pwr = 78;
		else if (signal_pwr < 63096)
			chn_pwr = 80;
		else if (signal_pwr < 66069)
			chn_pwr = 82;
		else if (signal_pwr < 69183)
			chn_pwr = 84;
		else if (signal_pwr < 72444)
			chn_pwr = 86;
		else if (signal_pwr < 75858)
			chn_pwr = 88;
		else if (signal_pwr < 79433)
			chn_pwr = 90;
		else if (signal_pwr < 83176)
			chn_pwr = 92;
		else if (signal_pwr < 87096)
			chn_pwr = 94;
		else if (signal_pwr < 91201)
			chn_pwr = 96;
		else if (signal_pwr < 95499)
			chn_pwr = 98;
		else if (signal_pwr < 100000)
			chn_pwr = 100;
		else if (signal_pwr < 104713)
			chn_pwr = 102;
		else if (signal_pwr < 109648)
			chn_pwr = 104;
		else if (signal_pwr < 114815)
			chn_pwr = 106;
		else if (signal_pwr < 120226)
			chn_pwr = 108;
		else if (signal_pwr < 125893)
			chn_pwr = 110;
		else if (signal_pwr < 131826)
			chn_pwr = 112;
		else if (signal_pwr < 138038)
			chn_pwr = 114;
		else if (signal_pwr < 144544)
			chn_pwr = 116;
		else if (signal_pwr < 151356)
			chn_pwr = 118;
		else if (signal_pwr < 158489)
			chn_pwr = 120;
		else if (signal_pwr < 165959)
			chn_pwr = 122;
		else if (signal_pwr < 173780)
			chn_pwr = 124;
		else if (signal_pwr < 181970)
			chn_pwr = 126;
		else if (signal_pwr < 190546)
			chn_pwr = 128;
		else
			chn_pwr = 130;
	}
	else
	{
		chn_pwr = 0;
	}

	*p_quality_DB = chn_pwr;

	return TP_SUCCESS;
}
#else
TP_UINT8 TP_get_signal_quality_DB(TP_FLOAT *p_quality_DB)
{
	TP_UINT8 result;
	TP_UINT8 reg_buffer[5];
	TP_UINT32 beta, total;
	TP_FLOAT chn_pwr, noise_pwr;
	TP_UINT32 temp;
	TP_UINT8 i;
	TP_UINT8 mean_count;

	result = IIC_READ_ONE_BYTE(0x050d, reg_buffer);
	if (result != TP_SUCCESS)
		return result;

	if ((reg_buffer[0] & 0x01) == 0x01)
	{
		beta = total = 0;
		result = IIC_READ_ONE_BYTE(0x110d, reg_buffer);
		if (result != TP_SUCCESS)
			return result;
		if (reg_buffer[0] == 0x80)
			mean_count = 100;
		else
			mean_count = 1;

		for (i = 0; i < mean_count; i++)
		{
			result = TP_iic_read(TP5001_DEVADDR, 0x054d, reg_buffer, 5);
			if (result != TP_SUCCESS)
				return result;

			temp = reg_buffer[0] | (reg_buffer[1] << 8) | ((reg_buffer[2] & 0x03) << 16);

			beta += temp;

			temp = reg_buffer[3] | ((reg_buffer[4] & 0x7f) << 8);

			total += temp;
		}

		chn_pwr = (TP_FLOAT)beta;
		noise_pwr = (TP_FLOAT)total;

		if (chn_pwr == 0)
			chn_pwr = 0;
		else
			chn_pwr /= noise_pwr;

		if (chn_pwr < 1)
			chn_pwr = 0;
		else if (chn_pwr < 1.0233)
			chn_pwr = (TP_FLOAT)0.1;
		else if (chn_pwr < 1.0471)
			chn_pwr = (TP_FLOAT)0.2;
		else if (chn_pwr < 1.0715)
			chn_pwr = (TP_FLOAT)0.3;
		else if (chn_pwr < 1.0965)
			chn_pwr = (TP_FLOAT)0.4;
		else if (chn_pwr < 1.1220)
			chn_pwr = (TP_FLOAT)0.5;
		else if (chn_pwr < 1.1482)
			chn_pwr = (TP_FLOAT)0.6;
		else if (chn_pwr < 1.1749)
			chn_pwr = (TP_FLOAT)0.7;
		else if (chn_pwr < 1.2023)
			chn_pwr = (TP_FLOAT)0.8;
		else if (chn_pwr < 1.2303)
			chn_pwr = (TP_FLOAT)0.9;
		else if (chn_pwr < 1.2589)
			chn_pwr = (TP_FLOAT)1.0;
		else if (chn_pwr < 1.2882)
			chn_pwr = (TP_FLOAT)1.1;
		else if (chn_pwr < 1.3183)
			chn_pwr = (TP_FLOAT)1.2;
		else if (chn_pwr < 1.3490)
			chn_pwr = (TP_FLOAT)1.3;
		else if (chn_pwr < 1.3804)
			chn_pwr = (TP_FLOAT)1.4;
		else if (chn_pwr < 1.4125)
			chn_pwr = (TP_FLOAT)1.5;
		else if (chn_pwr < 1.4454)
			chn_pwr = (TP_FLOAT)1.6;
		else if (chn_pwr < 1.4791)
			chn_pwr = (TP_FLOAT)1.7;
		else if (chn_pwr < 1.5136)
			chn_pwr = (TP_FLOAT)1.8;
		else if (chn_pwr < 1.5488)
			chn_pwr = (TP_FLOAT)1.9;
		else if (chn_pwr < 1.5849)
			chn_pwr = (TP_FLOAT)2.0;
		else if (chn_pwr < 1.6218)
			chn_pwr = (TP_FLOAT)2.1;
		else if (chn_pwr < 1.6596)
			chn_pwr = (TP_FLOAT)2.2;
		else if (chn_pwr < 1.6982)
			chn_pwr = (TP_FLOAT)2.3;
		else if (chn_pwr < 1.7378)
			chn_pwr = (TP_FLOAT)2.4;
		else if (chn_pwr < 1.7783)
			chn_pwr = (TP_FLOAT)2.5;
		else if (chn_pwr < 1.8197)
			chn_pwr = (TP_FLOAT)2.6;
		else if (chn_pwr < 1.8621)
			chn_pwr = (TP_FLOAT)2.7;
		else if (chn_pwr < 1.9055)
			chn_pwr = (TP_FLOAT)2.8;
		else if (chn_pwr < 1.9498)
			chn_pwr = (TP_FLOAT)2.9;
		else if (chn_pwr < 1.9953)
			chn_pwr = (TP_FLOAT)3.0;
		else if (chn_pwr < 2.0417)
			chn_pwr = (TP_FLOAT)3.1;
		else if (chn_pwr < 2.0893)
			chn_pwr = (TP_FLOAT)3.2;
		else if (chn_pwr < 2.1380)
			chn_pwr = (TP_FLOAT)3.3;
		else if (chn_pwr < 2.1878)
			chn_pwr = (TP_FLOAT)3.4;
		else if (chn_pwr < 2.2387)
			chn_pwr = (TP_FLOAT)3.5;
		else if (chn_pwr < 2.2909)
			chn_pwr = (TP_FLOAT)3.6;
		else if (chn_pwr < 2.3442)
			chn_pwr = (TP_FLOAT)3.7;
		else if (chn_pwr < 2.3988)
			chn_pwr = (TP_FLOAT)3.8;
		else if (chn_pwr < 2.4547)
			chn_pwr = (TP_FLOAT)3.9;
		else if (chn_pwr < 2.5119)
			chn_pwr = (TP_FLOAT)4.0;
		else if (chn_pwr < 2.5704)
			chn_pwr = (TP_FLOAT)4.1;
		else if (chn_pwr < 2.6303)
			chn_pwr = (TP_FLOAT)4.2;
		else if (chn_pwr < 2.6915)
			chn_pwr = (TP_FLOAT)4.3;
		else if (chn_pwr < 2.7542)
			chn_pwr = (TP_FLOAT)4.4;
		else if (chn_pwr < 2.8184)
			chn_pwr = (TP_FLOAT)4.5;
		else if (chn_pwr < 2.8840)
			chn_pwr = (TP_FLOAT)4.6;
		else if (chn_pwr < 2.9512)
			chn_pwr = (TP_FLOAT)4.7;
		else if (chn_pwr < 3.0200)
			chn_pwr = (TP_FLOAT)4.8;
		else if (chn_pwr < 3.0903)
			chn_pwr = (TP_FLOAT)4.9;
		else if (chn_pwr < 3.1623)
			chn_pwr = (TP_FLOAT)5.0;
		else if (chn_pwr < 3.2359)
			chn_pwr = (TP_FLOAT)5.1;
		else if (chn_pwr < 3.3113)
			chn_pwr = (TP_FLOAT)5.2;
		else if (chn_pwr < 3.3884)
			chn_pwr = (TP_FLOAT)5.3;
		else if (chn_pwr < 3.4674)
			chn_pwr = (TP_FLOAT)5.4;
		else if (chn_pwr < 3.5481)
			chn_pwr = (TP_FLOAT)5.5;
		else if (chn_pwr < 3.6308)
			chn_pwr = (TP_FLOAT)5.6;
		else if (chn_pwr < 3.7154)
			chn_pwr = (TP_FLOAT)5.7;
		else if (chn_pwr < 3.8019)
			chn_pwr = (TP_FLOAT)5.8;
		else if (chn_pwr < 3.8905)
			chn_pwr = (TP_FLOAT)5.9;
		else if (chn_pwr < 3.9811)
			chn_pwr = (TP_FLOAT)6.0;
		else if (chn_pwr < 4.0738)
			chn_pwr = (TP_FLOAT)6.1;
		else if (chn_pwr < 4.1687)
			chn_pwr = (TP_FLOAT)6.2;
		else if (chn_pwr < 4.2658)
			chn_pwr = (TP_FLOAT)6.3;
		else if (chn_pwr < 4.3652)
			chn_pwr = (TP_FLOAT)6.4;
		else if (chn_pwr < 4.4668)
			chn_pwr = (TP_FLOAT)6.5;
		else if (chn_pwr < 4.5709)
			chn_pwr = (TP_FLOAT)6.6;
		else if (chn_pwr < 4.6774)
			chn_pwr = (TP_FLOAT)6.7;
		else if (chn_pwr < 4.7863)
			chn_pwr = (TP_FLOAT)6.8;
		else if (chn_pwr < 4.8978)
			chn_pwr = (TP_FLOAT)6.9;
		else if (chn_pwr < 5.0119)
			chn_pwr = (TP_FLOAT)7.0;
		else if (chn_pwr < 5.2481)
			chn_pwr = (TP_FLOAT)7.2;
		else if (chn_pwr < 5.4954)
			chn_pwr = (TP_FLOAT)7.4;
		else if (chn_pwr < 5.7544)
			chn_pwr = (TP_FLOAT)7.6;
		else if (chn_pwr < 6.0256)
			chn_pwr = (TP_FLOAT)7.8;
		else if (chn_pwr < 6.3096)
			chn_pwr = (TP_FLOAT)8.0;
		else if (chn_pwr < 6.6069)
			chn_pwr = (TP_FLOAT)8.2;
		else if (chn_pwr < 6.9183)
			chn_pwr = (TP_FLOAT)8.4;
		else if (chn_pwr < 7.2444)
			chn_pwr = (TP_FLOAT)8.6;
		else if (chn_pwr < 7.5858)
			chn_pwr = (TP_FLOAT)8.8;
		else if (chn_pwr < 7.9433)
			chn_pwr = (TP_FLOAT)9.0;
		else if (chn_pwr < 8.3176)
			chn_pwr = (TP_FLOAT)9.2;
		else if (chn_pwr < 8.7096)
			chn_pwr = (TP_FLOAT)9.4;
		else if (chn_pwr < 9.1201)
			chn_pwr = (TP_FLOAT)9.6;
		else if (chn_pwr < 9.5499)
			chn_pwr = (TP_FLOAT)9.8;
		else if (chn_pwr < 10.0000)
			chn_pwr = (TP_FLOAT)10.0;
		else if (chn_pwr < 10.4713)
			chn_pwr = (TP_FLOAT)10.2;
		else if (chn_pwr < 10.9648)
			chn_pwr = (TP_FLOAT)10.4;
		else if (chn_pwr < 11.4815)
			chn_pwr = (TP_FLOAT)10.6;
		else if (chn_pwr < 12.0226)
			chn_pwr = (TP_FLOAT)10.8;
		else if (chn_pwr < 12.5893)
			chn_pwr = (TP_FLOAT)11.0;
		else if (chn_pwr < 13.1826)
			chn_pwr = (TP_FLOAT)11.2;
		else if (chn_pwr < 13.8038)
			chn_pwr = (TP_FLOAT)11.4;
		else if (chn_pwr < 14.4544)
			chn_pwr = (TP_FLOAT)11.6;
		else if (chn_pwr < 15.1356)
			chn_pwr = (TP_FLOAT)11.8;
		else if (chn_pwr < 15.8489)
			chn_pwr = (TP_FLOAT)12.0;
		else if (chn_pwr < 16.5959)
			chn_pwr = (TP_FLOAT)12.2;
		else if (chn_pwr < 17.3780)
			chn_pwr = (TP_FLOAT)12.4;
		else if (chn_pwr < 18.1970)
			chn_pwr = (TP_FLOAT)12.6;
		else if (chn_pwr < 19.0546)
			chn_pwr = (TP_FLOAT)12.8;
		else
			chn_pwr = (TP_FLOAT)13.0;
	}
	else
	{
		chn_pwr = (TP_FLOAT)0;
	}

	*p_quality_DB = chn_pwr;

	return TP_SUCCESS;
}
#endif

// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_signal_strength
// Description: TPоƬ���õ��ź�ǿ��
// Output:
//		p_signal_strength:  �ź�ǿ��
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_get_signal_strength(TP_UINT8 *p_signal_strength)
{
	TP_UINT8 register_value[2];
	TP_UINT8 result;
	TP_INT32 sum = 0;

	if (gui_tuner_type == RDA_5812_TUNER)
	{
#if 1
		TP_UINT8 A, B, C, D, E, P1, P2;
		TP_UINT16 P;
		TP_INT32 temp = 0, Plog = 0;
		TP_UINT8 reg_addr, device_addr = RDA5812_DEV_ADDR;
		TP_INT32 err = 0;

		TP_INT32 Gain_lna[] = {-73, -15, 36, 72, 110, 160, 200};
		TP_INT32 Gain_i2v[] = {0, 67, 136, 210};
		TP_INT32 Gain_filter[] = {-60, -2, 56, 112};

		reg_addr = 0x91;
		result = TP_iic_tuner_read(device_addr, reg_addr, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		A = (register_value[0] >> 6) & 0x03;

		reg_addr = 0x05;
		result = TP_iic_tuner_read(device_addr, reg_addr, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		B = (register_value[0] >> 4) & 0x03;
		C = (register_value[0] >> 2) & 0x03;
		D = register_value[0] & 0x03;
		E = register_value[1];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 10;
		else
			Plog = err * 20;

		temp = -(140 + Gain_lna[A + B] + Gain_i2v[C] + Gain_filter[D] + 2 * E);
		temp = (temp * 2 + Plog * 2 / 16) / 10 + 190;
		sum = (TP_INT32)temp;
#else
		TP_UINT8 A, B, C, D, E, P1, P2;
		TP_UINT16 P;
		TP_FLOAT temp = 0, Plog = 0;
		TP_UINT8 reg_addr, device_addr = RDA5812_DEV_ADDR;
		TP_FLOAT err = 0;

		TP_FLOAT Gain_lna[] = {-7.3, -1.5, 3.6, 7.2, 11, 16, 20};
		TP_FLOAT Gain_i2v[] = {0, 6.7, 13.6, 21};
		TP_FLOAT Gain_filter[] = {-6, -0.2, 5.6, 11.2};

		reg_addr = 0x91;
		result = TP_iic_tuner_read(device_addr, reg_addr, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		A = (register_value[0] >> 6) & 0x03;

		reg_addr = 0x05;
		result = TP_iic_tuner_read(device_addr, reg_addr, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		B = (register_value[0] >> 4) & 0x03;
		C = (register_value[0] >> 2) & 0x03;
		D = register_value[0] & 0x03;
		E = register_value[1];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;

		temp = -(14 + Gain_lna[A + B] + Gain_i2v[C] + Gain_filter[D] + 0.2 * E) + Plog;
		temp = (temp + 95) * 2;
		sum = (TP_INT32)temp;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}
	else if (gui_tuner_type == SHARP_6903_TUNER)
	{
#if 1
		TP_INT32 temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_INT32 err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;

		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 10000;
		else
			Plog = err * 20000;

		temp1 = 3838;
		temp1 = temp1 * temp - 900000 - 50000;

		temp1 = (temp1 * 2 + Plog * 2 / 16) / 10000 + 190;
		sum = (TP_INT32)temp1;
#else
		TP_FLOAT temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_FLOAT err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;

		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;
		temp1 = 0.3838;
		temp1 = temp1 * temp - 90 - 5;

		temp1 = temp1 + Plog;
		temp1 = (temp1 + 95) * 2;
		sum = (TP_INT32)temp1;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}
	else if (gui_tuner_type == RDA_5815_TUNER)
	{
#if 1
		TP_INT32 temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_INT32 err = 0, Plog = 0;
		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;

		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 10000;
		else
			Plog = err * 20000;

		temp1 = 3672;
		temp1 = temp1 * temp - 887000;

		temp1 = (temp1 * 2 + Plog * 2 / 16) / 10000 + 190;
		sum = (TP_INT32)temp1;
#else
		TP_FLOAT temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_FLOAT err = 0, Plog = 0;
		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;

		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;

		temp1 = 0.3672;
		temp1 = temp1 * temp - 88.7;

		temp1 = temp1 + Plog;
		temp1 = (temp1 + 95) * 2;
		sum = (TP_INT32)temp1;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}
	else if (gui_tuner_type == AV2020_TUNER)
	{
#if 1
		TP_INT32 temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_INT32 err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 100;
		else
			Plog = err * 200;

		temp1 = 49;
		temp1 = temp1 * temp - 12800;

		temp1 = (temp1 * 2 + Plog * 2 / 16) / 100 + 190;
		sum = (TP_INT32)temp1;
#else
		TP_FLOAT temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_FLOAT err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;

		temp1 = (TP_FLOAT)(0.49);
		temp1 = temp1 * temp - 128;

		temp1 = temp1 + Plog;
		temp1 = (temp1 + 95) * 2;
		sum = (TP_INT32)temp1;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}
	else if (gui_tuner_type == S305_TUNER)
	{
#if 1
		TP_INT32 temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_INT32 err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 10000;
		else
			Plog = err * 20000;

		temp1 = 5898;
		temp1 = temp1 * temp - 1278411;

		temp1 = (temp1 * 2 + Plog * 2 / 16) / 10000 + 190;
		sum = (TP_INT32)temp1;
#else
		TP_FLOAT temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_FLOAT err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;

		temp1 = (TP_FLOAT)0.5898;
		temp1 = temp1 * temp - 127.8411;

		temp1 = temp1 + Plog;
		temp1 = (temp1 + 95) * 2;
		sum = (TP_INT32)temp1;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}
	else
	{
#if 1
		TP_INT32 temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_INT32 err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = P - 128;
		if (err < 0)
			Plog = err * 10000;
		else
			Plog = err * 20000;

		if (temp < 105)
		{
			temp1 = temp * 30000 / 20 - 89 - 1;
		}
		else
		{
			temp1 = temp * 30000 / 7 - 118 - 2;
		}

		temp1 = (temp1 * 2 + Plog * 2 / 16) / 10000 + 190;
		sum = (TP_INT32)temp1;
#else
		TP_FLOAT temp = 0, temp1 = 0;
		TP_UINT8 P1, P2;
		TP_UINT16 P;
		TP_FLOAT err = 0, Plog = 0;

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0E, register_value, 1);
		if (result != TP_SUCCESS)
			return result;
		temp = register_value[0];

		result = TP_iic_read(TP5001_DEVADDR, 0x0D0C, register_value, 2);
		if (result != TP_SUCCESS)
			return result;
		P1 = register_value[0];
		P2 = register_value[1] & 0x01;
		P = P1 + P2 * 256;

		err = (TP_FLOAT)(P - 128);
		if (err < 0)
			Plog = err / 16;
		else
			Plog = err / 8;

		if (temp < 105)
		{
			temp1 = (TP_FLOAT)(3.0 / 20.0);
			temp1 = temp1 * temp - 89 - 1;
		}
		else
		{
			temp1 = (TP_FLOAT)(3.0 / 7.0);
			temp1 = temp1 * temp - 118 - 2;
		}
		temp1 = temp1 + Plog;
		temp1 = (temp1 + 95) * 2;
		sum = (TP_INT32)temp1;
#endif

		if (sum <= 0)
			sum = 0;
		if (sum > 100)
			sum = 100;

		*p_signal_strength = (TP_INT8)sum;
	}

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_freq_offset
// Description: TPоƬ���õ�Ƶƫֵ
// Output:
//		p_freq_offset: Ƶ��ƫ��ֵ����λ(KHz)
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_get_freq_offset(TP_INT32 *p_freq_offset)
{
#if 1
	TP_UINT8 result;
	TP_UINT8 reg_value[3];
	TP_INT32 temp;

	TP_INT32 Temp2;

	result = TP_iic_read(TP5001_DEVADDR, 0x0128, reg_value, 3);
	if (result != TP_SUCCESS)
		return result;

	Temp2 = (reg_value[0] << 16) | (reg_value[1] << 8) | reg_value[2];
	if (Temp2 > (1 << 23) - 1)
		Temp2 = Temp2 - (1 << 24);
	temp = Temp2;

	temp = temp / (1 << 24) * g_symbol_rate + (temp % (1 << 24)) * g_symbol_rate / (1 << 24);
	temp = temp / 1000; // output KHz
	temp = -temp;
#else
	TP_UINT8 result;
	TP_UINT8 reg_value[3];
	TP_FLOAT temp;

	TP_INT32 Temp2;

	result = TP_iic_read(TP5001_DEVADDR, 0x0128, reg_value, 3);
	if (result != TP_SUCCESS)
		return result;

	Temp2 = (reg_value[0] << 16) | (reg_value[1] << 8) | reg_value[2];
	if (Temp2 > (1 << 23) - 1)
		Temp2 = Temp2 - (1 << 24);
	temp = (TP_FLOAT)Temp2;

	temp /= (1 << 24);

	temp *= g_symbol_rate;

	temp = temp / 1000; // output KHz

	temp = -temp;
#endif

	*p_freq_offset = (TP_INT32)temp;

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_lock_status
// Description: TPоƬ���õ�����״̬
// Return:
//		TP_SUCCESS: ����
//		TP_NOT_LOCK: û������
//
TP_UINT8 TP_get_lock_status(void)
{
	TP_UINT8 register_value;
	TP_UINT8 result;
	TP_INT32 TP_offset;

	result = IIC_READ_ONE_BYTE(0x110d, &register_value);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- 0x110d = 0x%02x\n", __FUNCTION__, __LINE__, register_value);

	if ((register_value & 0x80) == 0x80)
	{
		TP_get_freq_offset(&TP_offset);
		//printk("%s[%d] ---- TP_offset = %d\n", __FUNCTION__, __LINE__, TP_offset);
		if ((TP_offset >= -5000) && (TP_offset <= 5000))
			return TP_SUCCESS;
		else
			return TP_NOT_LOCK;
	}
	else
		return TP_NOT_LOCK;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_rf_pola
// Description: TPоƬ�����ü���
// Input:
//		Pola: ���ԣ�Normal: ����(VH_SEL�ܽ�Ϊ��)��Invert: ����(VH_SEL�ܽ�Ϊ��)
// Return:
//		TP_SUCCESS: ����
//
TP_UINT8 TP_set_rf_pola(TP_RFAGCPola Pola)
{
	TP_UINT8 result;
	TP_UINT8 register_value;

	if (Pola == RA_Normal)
	{
		register_value = (1 << 6);
		result = IIC_WRITE_ONE_BYTE(0x3E00, register_value);
		if (result != TP_SUCCESS)
			return result;
	}
	else if (Pola == RA_Invert)
	{
		register_value = 0;
		result = IIC_WRITE_ONE_BYTE(0x3E00, register_value);
		if (result != TP_SUCCESS)
			return result;
	}
	else
		return TP_PARA_ERR;

	return TP_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_rf_tuner
// Description: TPоƬ������Tuner
// Input:
//		frequency:  Ƶ��, xx(MHz)
//		Symbol_Rate_Value:	������ xx(Hz)
// Return:
//		TP_SUCCESS: �����ɹ�
//		TP_SET_TUNER_ERR: ������������?
//
TP_UINT8 TP_set_rf_tuner(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{
	TP_UINT8 result;

	//printk("%s[%d] ---- Log 0, gui_tuner_type = %d, \n", __FUNCTION__, __LINE__, gui_tuner_type);

	printk("%s[%d] ---- gui_tuner_type = %d, freq[%d], symbol_rate[%d]\n", __FUNCTION__, __LINE__, gui_tuner_type, frequency, Symbol_Rate_Value);

	if (gui_tuner_type == RDA_5812_TUNER)
	{
		if (gTuner_initialized == 0)
		{
			gTuner_initialized = 1;

			result = rda_5812_init();
			if (result != TP_SUCCESS)
				return result;
		}

		return rda_5812_set_frequency(frequency, Symbol_Rate_Value);
	}

	//printk("%s[%d] ---- Tuner not RDA5812\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == RDA_5815_TUNER)
	{
		if (gTuner_initialized == 0)
		{
			gTuner_initialized = 1;

			result = rda_5815_init();
			if (result != TP_SUCCESS)
				return result;
		}

		return rda_5815_set_frequency(frequency, Symbol_Rate_Value / 1000);
	}

	//printk("%s[%d] ---- Tuner not RDA5815\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == RDA_5815M_TUNER)
	{
		if (gTuner_initialized == 0)
		{
			gTuner_initialized = 1;

			result = rda_5815M_init();
			//printk("%s[%d] ---- rda_5815M_init(), result = %d\n", __FUNCTION__, __LINE__, result);
			if (result != TP_SUCCESS)
				return result;
		}

		//return rda_5815M_set_frequency(frequency, Symbol_Rate_Value / 1000);
		result = rda_5815M_set_frequency(frequency, Symbol_Rate_Value / 1000);
		//printk("%s[%d] ---- rda_5815M_set_frequency(), result = %d\n", __FUNCTION__, __LINE__, result);

		return result;
	}

	printk("%s[%d] ---- Tuner not RDA5815M\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == AV2020_TUNER)
	{
		if (gTuner_initialized == 0)
		{
			gTuner_initialized = 1;

			result = av2020_init();
			if (result != TP_SUCCESS)
				return result;
		}

		return Tuner_control(frequency, Symbol_Rate_Value);
	}

	printk("%s[%d] ---- Tuner not AV2020\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == S305_TUNER)
	{
		return S305_set_frequency(frequency, Symbol_Rate_Value);
	}

	printk("%s[%d] ---- Tuner not S305\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == SHARP_6306_TUNER)
	{
		return sharp_6306_set_frequency(frequency, Symbol_Rate_Value);
	}

	printk("%s[%d] ---- Tuner not Sharp 6306\n", __FUNCTION__, __LINE__);

	if (gui_tuner_type == SHARP_6903_TUNER)
	{
		return sharp_6903_set_frequency(frequency, Symbol_Rate_Value);
	}

	printk("%s[%d] ---- Tuner not Sharp 6903\n", __FUNCTION__, __LINE__);

	return TP_TUNER_ID_ERR;
}

// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_iq_switch
// Description: TPоƬ������IQ����
// Input:
//		iq_pola: IQ�ļ���   Normal:����   Invert: ����
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_set_iq_switch(TP_IQPola iq_pola)
{
	TP_UINT8 ret;

	if (iq_pola == Normal)
	{
#ifdef FPGA_PLATFORM
		ret = IIC_WRITE_ONE_BYTE(0x0000, 0x01);
#else
		ret = IIC_WRITE_ONE_BYTE(0x0000, 0x00);
#endif
		if (ret != TP_SUCCESS)
			return ret;
	}
	else if (iq_pola == Invert)
	{
#ifdef FPGA_PLATFORM
		ret = IIC_WRITE_ONE_BYTE(0x0000, 0x03);
#else
		ret = IIC_WRITE_ONE_BYTE(0x0000, 0x02);
#endif
		if (ret != TP_SUCCESS)
			return ret;
	}
	else
	{
		return TP_PARA_ERR;
	}

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_ts_interface
// Description: TPоƬ������AGC����
// Input:
//		active_edge: positive_edge, ������     negitive_edge, �½���
//		port_type: series_port, ����      para_port, ����
//		clock_div: SPIʱ�ӷ�Ƶ����,    SPI_CLK = MAIN_CLK / (clock_div)     MAIN_CLK:  4M:380   10M:380   20M:380   27M:378
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_set_ts_interface(TP_SPI_edge active_edge, TP_SPI_port_type port_type, TP_UINT8 clock_div)
{
	TP_UINT8 reg_value;
	TP_UINT8 ret;

	reg_value = 0x0;

	// �޸�MPEG_CLK_SEL�ź������ػ����½���
	ret = IIC_READ_ONE_BYTE(0x3f00, &reg_value);
	if (ret != TP_SUCCESS)
		return ret;
	reg_value = (reg_value & 0xef) | (active_edge << 4);

	ret = IIC_WRITE_ONE_BYTE(0x3f00, reg_value);
	if (ret != TP_SUCCESS)
		return ret;

	// SSI MODE
	ret = IIC_READ_ONE_BYTE(0x0C01, &reg_value);
	if (ret != TP_SUCCESS)
		return ret;

	if (port_type == out_disable)
	{
		reg_value |= 0x04; // set register bit 2 = 0   output enable
		ret = IIC_WRITE_ONE_BYTE(0x0C01, reg_value);
		if (ret != TP_SUCCESS)
			return ret;
	}
	else
	{
		reg_value = (reg_value & 0xfa) | port_type; // set register bit 2 = 0   output enable
		ret = IIC_WRITE_ONE_BYTE(0x0C01, reg_value);
		if (ret != TP_SUCCESS)
			return ret;
	}
	// set clk div
	if (clock_div != 0)
	{
		reg_value = clock_div;
		ret = IIC_WRITE_ONE_BYTE(0x3F0B, reg_value);
		if (ret != TP_SUCCESS)
			return ret;
	}

	return TP_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_set_sleep
// Description: TPоƬ����˯��ģʽ
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_set_sleep(void)
{
	// TP_Reset();
	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_soft_reset
// Description: TPоƬ����λ
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_soft_reset(void)
{
	TP_UINT8 register_value;
	TP_UINT8 result;

	result = IIC_READ_ONE_BYTE(0x3F00, &register_value);
	if (result != TP_SUCCESS)
		return result;
	register_value &= ~0x80;
	result = IIC_WRITE_ONE_BYTE(0x3F00, register_value);
	if (result != TP_SUCCESS)
		return result;

	TP_Delay(1);

	return TP_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------
//
// Function Name: TP_SendDiseqcCommand
// Description: TP Diseqc interface
// Paramenter:
//			length: input, diseqc command length
//          pCommands: input, diseqc command buffer          pReturnLength: Input/Output command return length
//          pReturnBuffer: return buffer
// Return:
//		TP_SUCCESS: OK

TP_UINT8 TP_Diseqc_close(void)
{
	TP_UINT8 result, temp;
	// close diseqc module clk
	result = IIC_READ_ONE_BYTE(0x3F0A, &temp);
	if (result != TP_SUCCESS)
		return result;

	result = IIC_WRITE_ONE_BYTE(0x3F0A, temp & ~0x10);

	return result;
}

TP_UINT8 TP_Diseqc_Init(int sys_clk)
{
	TP_UINT8 result, temp;
	TP_UINT8 clk_div_pause[3];
	int div;

	// open diseqc module clk
	result = IIC_READ_ONE_BYTE(0x3F0A, &temp);
	if (result != TP_SUCCESS)
		return result;
	if ((temp & 0x10) == 0)
	{
		result = IIC_WRITE_ONE_BYTE(0x3F0A, temp | 0x10);
		if (result != TP_SUCCESS)
			return result;
	}
	// diseqc init, cfg 22k clk, pause time
	result = IIC_WRITE_ONE_BYTE(0x3E00, 0x80); // reset diseqc module except regs
	if (result != TP_SUCCESS)
		return result;

	div = sys_clk / 22000;
	clk_div_pause[0] = (TP_UINT8)div;
	clk_div_pause[1] = (TP_UINT8)(div >> 8);
	clk_div_pause[2] = 1;
	result = TP_iic_write(TP5001_DEVADDR, 0x3E02, clk_div_pause, 3);
	return result;
}

TP_UINT8 TP_SendDiseqcCommand(TP_UINT8 length,
							  TP_UINT8 *pCommands,
							  TP_UINT8 *pReturnLength,
							  TP_UINT8 *pReturnBuffer)
{
	TP_UINT8 result, i;
	TP_UINT8 tx_buf[4];
	TP_UINT8 rx_buf[4];

	// check parameter
	if ((length > 8) || (length < 1))
		return TP_PARA_ERR;
	if (*pReturnLength > 16)
		return TP_PARA_ERR;

	// open clk, reset diseqc, set 22k divider pause time
	result = TP_Diseqc_Init(100000000);
	if (result != TP_SUCCESS)
		return result;
	// because of use loop test mode, rx -- tx, so must firstly enbale rx
	// write rx ctrl
	result = IIC_WRITE_ONE_BYTE(0x3E10, 0x1);
	if (result != TP_SUCCESS)
		return result;

	// write tx data
	result = TP_iic_write(TP5001_DEVADDR, 0x3E08, pCommands, length);
	if (result != TP_SUCCESS)
		return result;

	tx_buf[0] = (1 << 6) | (1 << 2); // write tx ctrl
	tx_buf[1] = length - 1;			 // write tx cnt, 0=send 1 byte, 7=send 8 bytes data
	result = TP_iic_write(TP5001_DEVADDR, 0x3E00, tx_buf, 2);
	if (result != TP_SUCCESS)
		return result;

	// read rx cnt, untill this reg value is equ to (tx_cnt+1)
	while (1)
	{
		result = IIC_READ_ONE_BYTE(0x3E11, rx_buf);
		if (result != TP_SUCCESS)
			return result;
		if (rx_buf[0] == *pReturnLength)
			break;
	}
	// read data parity result
	result = IIC_READ_ONE_BYTE(0x3E12, rx_buf + 1);
	if (result != TP_SUCCESS)
		return result;
	result = IIC_READ_ONE_BYTE(0x3E13, rx_buf + 2);
	if (result != TP_SUCCESS)
		return result;

	// read rx data
	for (i = 0; i < *pReturnLength; i++, pReturnBuffer++)
	{
		result = IIC_READ_ONE_BYTE(0x3E14, pReturnBuffer);
		if (result != TP_SUCCESS)
			return result;
	}
	// close module clk
	result = TP_Diseqc_close();
	return result;
}

TP_UINT8 TP_tuner_dynamic_detect(void)
{
	TP_UINT8 reg_data[2];
	TP_UINT8 result = 0;
	TP_UINT8 found = 0;


	gui_tuner_type = 0;

	// RDA5812 RDA5815 addr = 0x0C
	printk("%s[%d] ---- tuner_type = %d, found = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, found);

	if (found == 0)
	{
		result = IIC_WRITE_ONE_BYTE(0xFE01, RDA5812_DEV_ADDR << 1);
		//printk("%s[%d] ---- IIC_WRITE_ONE_BYTE -- 0xFE01 = 0x%02x\n", __FUNCTION__, __LINE__, RDA5812_DEV_ADDR << 1);
		if (result == TP_SUCCESS)
		{
#if 0
			TP_INT8 i = 0;

			for (i = 11; i > 0; i--)
			{
				printk("%s[%d] ==============%2d seconds==================\n\n", __FUNCTION__, __LINE__, i - 1);
				TP_Delay(1000);
			}
#endif

			result = TP_iic_tuner_read(RDA5812_DEV_ADDR, 0x01, reg_data, 1);

			//printk("%s[%d] ---- TP_iic_tuner_read(), dev_addr[%02x], 0x01 = 0x%02x\n", __FUNCTION__, __LINE__, RDA5812_DEV_ADDR, reg_data[0]);

			if (result == TP_SUCCESS)
			{
				if ((reg_data[0] & 0xF0) == 0xC0)
				{
					gui_tuner_type = RDA_5812_TUNER;
					found = 1;
				}
				else if ((reg_data[0] & 0xFF) == 0xF8)
				{
					gui_tuner_type = RDA_5815M_TUNER;
					found = 1;
				}
				else if ((reg_data[0] & 0xF0) == 0xF0)
				{
					gui_tuner_type = RDA_5815_TUNER;
					found = 1;
				}
			}
		}
	}

	printk("%s[%d] ---- tuner_type = %d, found = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, found);

	// AV2020  addr = 0x63
	if (found == 0)
	{
		AV2020_DEV_ADDR = 0x63;
		result = IIC_WRITE_ONE_BYTE(0xFE01, AV2020_DEV_ADDR << 1);
		if (result == TP_SUCCESS)
		{
			result = TP_iic_tuner_read(AV2020_DEV_ADDR, 0x00, reg_data, 1);
			if (result == TP_SUCCESS)
			{
				gui_tuner_type = AV2020_TUNER;
				found = 1;
			}
		}
		if (found == 0)
		{
			AV2020_DEV_ADDR = 0x62;
			result = IIC_WRITE_ONE_BYTE(0xFE01, AV2020_DEV_ADDR << 1);
			if (result == TP_SUCCESS)
			{
				result = TP_iic_tuner_read(AV2020_DEV_ADDR, 0x00, reg_data, 1);
				if (result == TP_SUCCESS)
				{
					gui_tuner_type = AV2020_TUNER;
					found = 1;
				}
			}
		}
	}

	printk("%s[%d] ---- tuner_type = %d, found = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, found);

	// SHARP_6306,SHARP6903  addr = 0x60
	if (found == 0)
	{
		result = IIC_WRITE_ONE_BYTE(0xFE01, SHARP6306_DEV_ADDR << 1);
		if (result == TP_SUCCESS)
		{
			result = TP_iic_tuner_read(SHARP6306_DEV_ADDR, 0x00, reg_data, 1);
			if (result == TP_SUCCESS)
			{
				if (reg_data[0] == 0x68)
				{
					gui_tuner_type = SHARP_6903_TUNER;
					found = 1;
				}
				else
				{
					gui_tuner_type = SHARP_6306_TUNER;
					found = 1;
				}
			}
		}
	}

	printk("%s[%d] ---- tuner_type = %d, found = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, found);

	// S305 addr = 0x61
	// S305 addr = 0x61 mtv600
	if (found == 0)
	{
		result = IIC_WRITE_ONE_BYTE(0xFE01, S305_DEV_ADDR << 1);
		if (result == TP_SUCCESS)
		{
			reg_data[0] = 0x08; // for MTV600_TUNER this reg is read only
			reg_data[1] = 0x4E;
			result = TP_iic_tuner_write(S305_DEV_ADDR, reg_data, 2);
			if (result == TP_SUCCESS)
			{
				result = TP_iic_tuner_read(S305_DEV_ADDR, 0x00, reg_data, 2);
				if (result == TP_SUCCESS)
				{
					if (reg_data[1] == 0x4E)
					{
						gui_tuner_type = S305_TUNER;
						found = 1;
					}
				}
			}
		}
	}

	printk("%s[%d] ---- tuner_type = %d, found = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, found);

	if (found == 0)
		return TP_TUNER_ID_ERR;
	else
		return TP_SUCCESS;
}

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_init
// Description: TPоƬ����ʼ��
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_init(void)
{
	TP_UINT8 result;
	TP_UINT8 reg_value[6];

	// ��ʼ��ȫ�ֱ���
	g_symbol_rate = 0;

	//printk("%s[%d] ---- Log0, gui_tuner_type = %d\n", __FUNCTION__, __LINE__, gui_tuner_type);

	result = system_init();
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Log1, gui_tuner_type = %d, ret = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, result);

#if 1
	result = TP_tuner_dynamic_detect();
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Log2, gui_tuner_type = %d, ret = %d\n", __FUNCTION__, __LINE__, gui_tuner_type, result);
#endif

	if (gui_tuner_type == 0)
		return TP_TUNER_ID_ERR;

	// ����IQ
	if (gui_tuner_type == SHARP_6306_TUNER)
		result = TP_set_iq_switch(Normal);
	else if (gui_tuner_type == SHARP_6903_TUNER)
		result = TP_set_iq_switch(Normal);
	else if (gui_tuner_type == RDA_5812_TUNER)
		result = TP_set_iq_switch(Invert);
	else if (gui_tuner_type == RDA_5815_TUNER)
		result = TP_set_iq_switch(Invert);
	else if (gui_tuner_type == S305_TUNER)
		result = TP_set_iq_switch(Invert);
	else if (gui_tuner_type == AV2020_TUNER)
		result = TP_set_iq_switch(Normal);
	else if (gui_tuner_type == RDA_5815M_TUNER)
		result = TP_set_iq_switch(Invert);
	else
		result = TP_set_iq_switch(Normal);
	if (result != TP_SUCCESS)
		return result;


	//printk("%s[%d] ---- Log3, ret = %d\n", __FUNCTION__, __LINE__, result);

	result = IIC_WRITE_ONE_BYTE(0x0500, 0x01);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Log4, ret = %d\n", __FUNCTION__, __LINE__, result);

	reg_value[0] = 0x22;
	reg_value[1] = 0x04;
	reg_value[2] = 0x04;
	reg_value[3] = 0x03;
	result = TP_iic_write(TP5001_DEVADDR, 0x1103, reg_value, 4);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Log5, ret = %d\n", __FUNCTION__, __LINE__, result);

	reg_value[0] = 0x20;
	reg_value[1] = 0x00;
	reg_value[2] = 0x05;
	reg_value[3] = 0x00;
	result = TP_iic_write(TP5001_DEVADDR, 0x0305, reg_value, 4);
	if (result != TP_SUCCESS)
		return result;

	//printk("%s[%d] ---- Log6, ret = %d\n", __FUNCTION__, __LINE__, result);

#ifdef FPGA_PLATFORM
	result = IIC_WRITE_ONE_BYTE(0x0d0b, 0x7f);
	if (result != TP_SUCCESS)
		return result;
#endif

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_deinit
// Description: TPоƬ������ʼ��
// Return:
//		TP_SUCCESS: �����ɹ�
//
TP_UINT8 TP_deinit(void)
{
	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------
// Function Name: TP_get_statistic_ber_bler
// Description: GK5109SоƬ���õ�BER��BLERͳ��ֵ
// Return:
//		TP_SUCCESS: �����ɹ�
TP_UINT8 TP_get_statistic_ber_bler(TP_UINT64 *pBer_total, TP_UINT64 *pBer_error, TP_UINT64 *pBler_total, TP_UINT64 *pBler_error)
{
	TP_UINT8 result;
	TP_UINT64 tempValue;
	TP_UINT8 temp[16];
	TP_UINT32 i;

	//	if(g_chip_type == CHIP_TYPE_GK5109S)
	//	{
	result = TP_iic_read(TP5001_DEVADDR, 0x0B00, temp, 5);
	if (result != TP_SUCCESS)
		return result;
	tempValue = 0;
	for (i = 0; i < 5; i++)
	{
		tempValue |= temp[4 - i];
		tempValue <<= 8;
	}
	tempValue >>= 8;
	*pBer_total = tempValue;

	result = TP_iic_read(TP5001_DEVADDR, 0x0B05, temp, 5);
	if (result != TP_SUCCESS)
		return result;
	tempValue = 0;
	for (i = 0; i < 5; i++)
	{
		tempValue |= temp[4 - i];
		tempValue <<= 8;
	}
	tempValue >>= 8;
	*pBer_error = tempValue;

	result = TP_iic_read(TP5001_DEVADDR, 0x0A11, temp, 4);
	if (result != TP_SUCCESS)
		return result;
	tempValue = 0;
	for (i = 0; i < 4; i++)
	{
		tempValue |= temp[3 - i];
		tempValue <<= 8;
	}
	tempValue >>= 8;
	*pBler_total = tempValue;

	result = TP_iic_read(TP5001_DEVADDR, 0x0A15, temp, 4);
	if (result != TP_SUCCESS)
		return result;
	tempValue = 0;
	for (i = 0; i < 4; i++)
	{
		tempValue |= temp[3 - i];
		tempValue <<= 8;
	}
	tempValue >>= 8;
	*pBler_error = tempValue;
	//	}

	return TP_SUCCESS;
}

//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------
// Function Name: TP_clear_statistic_ber_bler
// Description: GK5109SоƬ�����BER��BLERͳ��ֵ
// Return:
//		TP_SUCCESS: �����ɹ�

TP_UINT8 TP_clear_statistic_ber_bler(TP_UINT8 type)
{
	TP_UINT8 temp[2];
	TP_UINT8 result;

	temp[0] = 1;
	if (type & 0x01)
	{
		result = TP_iic_write(TP5001_DEVADDR, 0x0B0A, temp, 1);
		if (result != TP_SUCCESS)
			return result;
	}
	temp[0] = 5;
	if (type & 0x02)
	{
		result = TP_iic_write(TP5001_DEVADDR, 0x0A10, temp, 1);
		if (result != TP_SUCCESS)
			return result;
	}

	return TP_SUCCESS;
}
// ---------------------------------------------------------------------------------------------------------

#endif //_USE_TP5001_CHIP_

