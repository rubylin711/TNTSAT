/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/******************************************************
QM1D1C0045.c
----------------------------------------------------
Rf IC control functions

<Revision History>
'11/02/09 : OKAMOTO [QM1D1C0045] First release
----------------------------------------------------
Copyright(C) 2010 SHARP CORPORATION
******************************************************/

// #include <linux/math.h>
// #include "windows.h"
#include "TP5001.h"
#include "QM1D1C0045if.h"

#ifdef SHARP_DEBUG_4R5GEN_FOR_CYGWIN
#include <stdio.h>
#endif
#ifdef _USE_TP5001_CHIP_

const UINT8 SlvAddr = 0x60;
#define QM1D1C0045_SLAVE_ADDR 0x60
#define MAX_NUMBER_TEXT_SIZE 8
#define MAX_NUMBER_REG_TEXT_SIZE 2

#define MAX_COMBO_LPF_NUM 13
#define DEF_XTAL_FREQ 16000

#define INIT_DUMMY_RESET 0x0C

//=========================================================================
// GLOBAL VARIALBLES
//=========================================================================

const unsigned long QM1D1C0045_d[QM1D1C0045_INI_CGF_MAX] = 
{
	0x0017a6b0, //	LOCAL_FREQ,
	0x00003e80, //	XTAL_FREQ,
	0x00000058, //	CHIP_ID,
	0x00000014, //	LPF_WAIT_TIME,
	0x00000004, //	FAST_SEARCH_WAIT_TIME,
	0x0000000f, //	NORMAL_SEARCH_WAIT_TIME,
};

UINT8 QM1D1C0045_d_reg[QM1D1C0045_INI_REG_MAX] = 
{
	0x58,
	0x1c,
	0xc0,
	0x10,
	0xbc,
	0xc1,
	0x15,
	0x34,
	0x06,
	0x3e,
	0x00,
	0x00,
	0x43,
	0x00,
	0x00,
	0x00,
	0x00,
	0xff,
	0xf3,
	0x00,
	0x25,
	0x35,
	0xdc,
	0xd6,
	0x66,
	0xcf,
	0x95,
	0xf5,
	0x96,
	0x7b,
	0x09,
	0x00,
};

const UINT8 QM1D1C0045_d_flg[QM1D1C0045_INI_REG_MAX] = /* 0:R, 1:R/W */
{
	0, // 0x0
	1, // 0x1
	1, // 0x2
	1, // 0x3
	1, // 0x4
	1, // 0x5
	1, // 0x6
	1, // 0x7
	1, // 0x8
	1, // 0x9
	1, // 0xA
	1, // 0xB
	1, // 0xC
	0, // 0xD
	0, // 0xE
	0, // 0xF
	0, // 0x10
	1, // 0x11
	1, // 0x12
	1, // 0x13
	1, // 0x14
	1, // 0x15
	1, // 0x16
	1, // 0x17
	1, // 0x18
	1, // 0x19
	1, // 0x1A
	1, // 0x1B
	1, // 0x1C
	1, // 0x1D
	1, // 0x1E
	1, // 0x1F
};

const unsigned long QM1D1C0045_local_f[] = 
{
	// kHz
	2151000,
	1950000,
	1800000,
	1600000,
	1450000,
	1250000,
	1200000,
	975000,
	950000,
	0,
	0,
	0,
	0,
	0,
	0,
};

const unsigned long QM1D1C0045_div2[] = 
{
	1,
	1,
	1,
	1,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

const unsigned long QM1D1C0045_vco_band[] = 
{
	7,
	6,
	5,
	4,
	3,
	2,
	7,
	6,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};

const UINT8 QM1D1C0045_lpf_max[] = 
{
	14,
	28,
	34,
	0,
	0,
};

const UINT8 QM1D1C0045_csel_offset[] = 
{
	2,
	1,
	0,
	0,
	0,
};

MT_BOOL RfTunerI2CWriteHandler(UINT8, UINT8 *, UINT16 *);
MT_BOOL RfTunerI2CReadHandler(UINT8, UINT8 *, UINT16 *);

QM1D1C0045_CONFIG_STRUCT QM1D1C0045ConfigList =
{
	1550000,				 /*XtalFreqKHz*/
	16000,					 /*XtalFreqKHz*/
	FALSE,					 /*b_fast_search_mode*/
	FALSE,					 /*b_loop_through*/
	FALSE,					 /*b_tuner_standby*/
	TRUE,					 /*b_head_amp*/
	QM1D1C0045_LPF_FC_10MHz, /*lpf*/
	20,						 /*QM1D1C0045_LpfWaitTime*/
	4,						 /*QM1D1C0045_FastSearchWaitTime*/
	15,						 /*QM1D1C0045_NormalSearchWaitTime*/
};

/* PROTO TYPE */
MT_BOOL QM1D1C0045_set_searchmode(PQM1D1C0045_CONFIG_STRUCT apConfig);
MT_BOOL QM1D1C0045_Set_Operation_Param(PQM1D1C0045_CONFIG_STRUCT apConfig);
void QM1D1C0045_set_lpf(QM1D1C0045_LPF_FC lpf);
void QM1D1C0045_get_lock_status(MT_BOOL *pbLock);

//=========================================================================
// GLOBAL VARIALBLES
//=========================================================================
static QM1D1C0045_I2C_WR_HANDLER g_qm1d1c0045_i2c_write_handler = NULL;
static QM1D1C0045_I2C_RD_HANDLER g_qm1d1c0045_i2c_read_handler = NULL;

/*====================================================*
	QM1D1C0045_set_i2c_handler
   --------------------------------------------------
	Description     set handler for i2c access
	Argument        each handler pointer
	Return Value	MT_BOOL (TRUE:success, FALSE:error)
 *====================================================*/
MT_BOOL QM1D1C0045_set_i2c_handler(QM1D1C0045_I2C_WR_HANDLER apWriteHandler, QM1D1C0045_I2C_RD_HANDLER apReadHandler)
{
	if ((apWriteHandler == NULL) || (apReadHandler == NULL))
	{
		return FALSE;
	}

	g_qm1d1c0045_i2c_write_handler = apWriteHandler;
	g_qm1d1c0045_i2c_read_handler = apReadHandler;

	return TRUE;
}

/*====================================================*
	QM1D1C0045_set_i2c_slave_addr
   --------------------------------------------------
	Description     Set and read slave address
	Argument        UINT8 SlaveAddr
	Return Value	slave address
 *====================================================*/
#define QM1D1C0045_ILLEAGAL_SLAVE_ADDR 0xff
UINT8 QM1D1C0045_i2c_slave_addr_set(UINT8 SlaveAddr)
{
	static UINT8 static_SlaveAddr = QM1D1C0045_ILLEAGAL_SLAVE_ADDR;

	if (SlaveAddr != QM1D1C0045_ILLEAGAL_SLAVE_ADDR)
	{
		static_SlaveAddr = SlaveAddr;
	}

	return static_SlaveAddr;
}

/*====================================================*
	QM1D1C0045_register_real_write
   --------------------------------------------------
	Description     register write
	Argument        RegAddr
					RegData
	Return Value	MT_BOOL (TRUE:success, FALSE:error)
 *====================================================*/
MT_BOOL QM1D1C0045_register_real_write(UINT8 RegAddr, UINT8 RegData)
{
	UINT8 result;
	//	UINT8 slvAddr;
	UINT8 i2c_tmp_buf[3];
	UINT16 i2c_access_size = 2;

	i2c_tmp_buf[0] = RegAddr;
	i2c_tmp_buf[1] = RegData;
#ifdef TEMP_CODE
	slvAddr = QM1D1C0045_i2c_slave_addr_set(QM1D1C0045_SLAVE_ADDR);
	return g_qm1d1c0045_i2c_write_handler(slvAddr, i2c_tmp_buf, &i2c_access_size);
#else
	result = TP_iic_tuner_write(QM1D1C0045_SLAVE_ADDR, i2c_tmp_buf, i2c_access_size);
	if (result == 0)
		return TRUE;
	else
	{
		// printf("QM1D1C0045_register_real_write failed\n");
		return FALSE;
	}
#endif
}

/*====================================================*
	QM1D1C0045_register_real_read
   --------------------------------------------------
	Description     register read
	Argument        RegAddr (Register Address)
					apData (Read data)
	Return Value	MT_BOOL (TRUE:success, FALSE:error)
 *====================================================*/
MT_BOOL QM1D1C0045_register_real_read(UINT8 RegAddr, UINT8 *apData)
{
#ifndef TEMP_CODE
	UINT8 result;
	result = TP_iic_tuner_read(QM1D1C0045_SLAVE_ADDR, RegAddr, apData, 1);
	if (result == 0)
		return TRUE;
	else
	{
		// printf("QM1D1C0045_register_real_read failed\n");
		return FALSE;
	}
#else
	MT_BOOL bRetVal;
	UINT8 slvAddr;

	slvAddr = QM1D1C0045_i2c_slave_addr_set(QM1D1C0045_SLAVE_ADDR);

	{
		UINT8 i2c_tmp_buf[1];
		UINT16 i2c_access_size = 1;
		i2c_tmp_buf[0] = RegAddr;
		bRetVal = (*g_qm1d1c0045_i2c_write_handler)(slvAddr, i2c_tmp_buf, &i2c_access_size);
	}
	if (bRetVal != TRUE)
	{
		return bRetVal;
	}

	{
		UINT16 i2c_access_size = 1;
		bRetVal = (*g_qm1d1c0045_i2c_read_handler)(slvAddr, apData, &i2c_access_size);
	}
	if (bRetVal != TRUE)
	{
		return bRetVal;
	}
#endif

	return TRUE;
}

UINT8 QM1D1C0045_pll_getdata_once(QM1D1C0045_INIT_REG_DATA RegAddr)
{
	UINT8 data;
#ifdef SHARP_DEBUG_4R5GEN_FOR_CYGWIN
	// printf(HERE "I2C Read\n");
	data = QM1D1C0045_d_reg[RegAddr];
#endif
	QM1D1C0045_register_real_read(RegAddr, &data);
	return data;
}

MT_BOOL QM1D1C0045_pll_setdata_once(QM1D1C0045_INIT_REG_DATA RegAddr, UINT8 RegData)
{
	MT_BOOL bRetValue;
#ifdef SHARP_DEBUG_4R5GEN_FOR_CYGWIN
// printf(HERE "I2C Write\n");
#endif
	bRetValue = QM1D1C0045_register_real_write(RegAddr, RegData);
	return bRetValue;
}

MT_BOOL QM1D1C0045_Initialize(PQM1D1C0045_CONFIG_STRUCT apConfig)
{
	UINT8 i_data, i;
	MT_BOOL bRetValue;

	if (apConfig == NULL)
	{
		return FALSE;
	}

	/*Dummy Access*/
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, INIT_DUMMY_RESET);
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, INIT_DUMMY_RESET);
	/*Soft Reaet ON*/
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, INIT_DUMMY_RESET);
	TP_Delay(1);
	/*Soft Reaet OFF*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_01);
	i_data |= 0x10;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, i_data);

	/*ID Check*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_00);
#ifdef SHARP_DEBUG_4R5GEN_FOR_CYGWIN
// printf(HERE "RF IC ID = 0x%.2X\n" , i_data);
#endif

#ifdef TEMP_CODE
	if (QM1D1C0045_d[QM1D1C0045_CHIP_ID] != i_data)
	{
		return FALSE; //"I2C Comm Error", NULL, MB_ICONWARNING);
	}
#endif
	/*LPF Tuning On*/
	TP_Delay(1);
	QM1D1C0045_d_reg[QM1D1C0045_REG_0C] |= 0x40;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0C, QM1D1C0045_d_reg[QM1D1C0045_REG_0C]);
	TP_Delay(apConfig->ui_QM1D1C0045_LpfWaitTime);

	for (i = 0; i < QM1D1C0045_INI_REG_MAX; i++)
	{
		if (QM1D1C0045_d_flg[i] == TRUE)
		{
			QM1D1C0045_pll_setdata_once((QM1D1C0045_INIT_REG_DATA)i, QM1D1C0045_d_reg[i]);
		}
	}
	//	apConfig->b_QM1D1C0045_fast_search_mode = FALSE;//Normal Search
	//	apConfig->b_QM1D1C0045_loop_through = TRUE;//LoopThrough Enable
	//	apConfig->b_QM1D1C0045_tuner_standby = FALSE;//Normal Mode

	bRetValue = QM1D1C0045_Set_Operation_Param(apConfig);
	if (!bRetValue)
	{
		return FALSE;
	}
	bRetValue = QM1D1C0045_set_searchmode(apConfig);
	if (!bRetValue)
	{
		return FALSE;
	}

	return TRUE;
}

int mypow(int num, int n)
{
	int value = 1;
	int i = 1;

	if (n == 0)
	{
		value = 1;
	}
	else
	{
		while (i++ <= n)
		{
			value *= num;
		}
	}

	return value;
}

MT_BOOL QM1D1C0045_LocalLpfTuning(PQM1D1C0045_CONFIG_STRUCT apConfig)
{
	UINT8 i_data, i_data1, i;
	unsigned int COMP_CTRL;

	if (apConfig == NULL)
	{
		return FALSE;
	}

	/*LPF*/
	QM1D1C0045_set_lpf(apConfig->QM1D1C0045_lpf);

	/*div2/vco_band*/
	for (i = 0; i < 15; i++)
	{
		if (QM1D1C0045_local_f[i] == 0)
		{
			continue;
		}
		if ((QM1D1C0045_local_f[i + 1] <= apConfig->ui_QM1D1C0045_RFChannelkHz) && (QM1D1C0045_local_f[i] > apConfig->ui_QM1D1C0045_RFChannelkHz))
		{
			i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_02);
			//			printf("QM1D1C0045_LocalLpfTuning i:%d,apData:%d\n",i,i_data);
			i_data &= 0x0F;
			i_data |= ((QM1D1C0045_div2[i] << 7) & 0x80);
			i_data |= ((QM1D1C0045_vco_band[i] << 4) & 0x70);
			//			printf("QM1D1C0045_LocalLpfTuning i:%d,apData:%d\n",i,i_data);
			QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_02, i_data);
		}
	}

	/*PLL Counter*/
	{
		int F_ref, pll_ref_div, alpha, N, A, sd;
		//double M, beta;
		int M, beta;

		if (apConfig->ui_QM1D1C0045_XtalFreqKHz == DEF_XTAL_FREQ)
		{
			F_ref = apConfig->ui_QM1D1C0045_XtalFreqKHz;
			pll_ref_div = 0;
		}
		else
		{
			F_ref = (apConfig->ui_QM1D1C0045_XtalFreqKHz >> 1);
			pll_ref_div = 1;
		}

		//M = (double)((apConfig->ui_QM1D1C0045_RFChannelkHz) / ((double)F_ref));
		//alpha = (int)(M + 0.5);
		//beta = (double)(M - alpha);
		//N = (int)((double)(alpha - 12.0) / 4.0);
		//A = alpha - 4 * (N + 1) - 5;

		M = apConfig->ui_QM1D1C0045_RFChannelkHz / F_ref;
		alpha = (apConfig->ui_QM1D1C0045_RFChannelkHz + F_ref / 2 ) / F_ref;
		beta = M - alpha;
		N = (int)((alpha - 12) / 4);
		A = alpha - 4 * (N + 1) - 5;

		if (beta >= 0)
		{
			// sd = (int)(pow(2.,20.) * beta);
			sd = (int)(mypow(2, 20) * beta);
		}
		else
		{
			// sd = (int)(0x400000 + pow(2., 20.) * beta);
			sd = (int)(0x400000 + mypow(2, 20) * beta);
		}

		QM1D1C0045_d_reg[QM1D1C0045_REG_06] &= 0x40;
		QM1D1C0045_d_reg[QM1D1C0045_REG_06] |= ((pll_ref_div << 7) | (N));
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_06, QM1D1C0045_d_reg[QM1D1C0045_REG_06]);

		QM1D1C0045_d_reg[QM1D1C0045_REG_07] &= 0xF0;
		QM1D1C0045_d_reg[QM1D1C0045_REG_07] |= (A & 0x0F);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_07, QM1D1C0045_d_reg[QM1D1C0045_REG_07]);

		/*LPF_CLK , LPF_FC*/
		i_data = QM1D1C0045_d_reg[QM1D1C0045_REG_08] & 0xF0;
		if (apConfig->ui_QM1D1C0045_XtalFreqKHz >= 6000 && apConfig->ui_QM1D1C0045_XtalFreqKHz < 66000)
		{
			//i_data1 = ((unsigned char)((double)(apConfig->ui_QM1D1C0045_XtalFreqKHz) / 4000. + 0.5) * 2 - 4) >> 1;
			i_data1 = ((unsigned char)((apConfig->ui_QM1D1C0045_XtalFreqKHz + 2000) / 4000) * 2 - 4) >> 1;
		}
		else if (apConfig->ui_QM1D1C0045_XtalFreqKHz < 6000)
		{
			i_data1 = 0x00;
		}
		else
		{
			i_data1 = 0x0F;
		}
		i_data |= i_data1;
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_08, i_data);

		QM1D1C0045_d_reg[QM1D1C0045_REG_09] &= 0xC0;
		QM1D1C0045_d_reg[QM1D1C0045_REG_09] |= ((sd >> 16) & 0x3F);
		QM1D1C0045_d_reg[QM1D1C0045_REG_0A] = (UINT8)((sd >> 8) & 0xFF);
		QM1D1C0045_d_reg[QM1D1C0045_REG_0B] = (UINT8)(sd & 0xFF);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_09, QM1D1C0045_d_reg[QM1D1C0045_REG_09]);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0A, QM1D1C0045_d_reg[QM1D1C0045_REG_0A]);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0B, QM1D1C0045_d_reg[QM1D1C0045_REG_0B]);
	}

	/*COMP_CTRL*/
	if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 17000)
	{
		COMP_CTRL = 0x04;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 18000)
	{
		COMP_CTRL = 0x01;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 19000)
	{
		COMP_CTRL = 0x02;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 20000)
	{
		COMP_CTRL = 0x04;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 21000)
	{
		COMP_CTRL = 0x05;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 22000)
	{
		COMP_CTRL = 0x02;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 23000)
	{
		COMP_CTRL = 0x03;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 24000)
	{
		COMP_CTRL = 0x04;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 25000)
	{
		COMP_CTRL = 0x05;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 26000)
	{
		COMP_CTRL = 0x03;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 27000)
	{
		COMP_CTRL = 0x04;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 28000)
	{
		COMP_CTRL = 0x05;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 29000)
	{
		COMP_CTRL = 0x06;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 30000)
	{
		COMP_CTRL = 0x04;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 31000)
	{
		COMP_CTRL = 0x05;
	}
	else if (apConfig->ui_QM1D1C0045_XtalFreqKHz == 32000)
	{
		COMP_CTRL = 0x05;
	}
	else
	{
		COMP_CTRL = 0x03;
	}
	QM1D1C0045_d_reg[QM1D1C0045_REG_1D] &= 0xF8;
	QM1D1C0045_d_reg[QM1D1C0045_REG_1D] |= COMP_CTRL;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_1D, QM1D1C0045_d_reg[QM1D1C0045_REG_1D]);

	/*BBLPF_cur*/
	QM1D1C0045_d_reg[QM1D1C0045_REG_1B] &= 0xFC;
	QM1D1C0045_d_reg[QM1D1C0045_REG_1B] |= QM1D1C0045_LPF_ADJUSTMENT_CURRENT_25UA;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_1B, QM1D1C0045_d_reg[QM1D1C0045_REG_1B]);

	/*VCO_TM , LPF_TM*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_0C);
	i_data &= 0x3F;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0C, i_data);
	TP_Delay(1); // 1024usec

	/*VCO_TM , LPF_TM*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_0C);
	//	printf("QM1D1C0045_LocalLpfTuning REG0x0c:%d\n",i_data);
	i_data |= 0xC0;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0C, i_data);

	TP_Delay(apConfig->ui_QM1D1C0045_LpfWaitTime);

	/*LPF_FC*/
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_08, QM1D1C0045_d_reg[QM1D1C0045_REG_08]);

	/*CSEL_Offset*/
	QM1D1C0045_d_reg[QM1D1C0045_REG_13] &= 0x9F;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_13, QM1D1C0045_d_reg[QM1D1C0045_REG_13]);

	/*BBLPF_cur*/
	QM1D1C0045_d_reg[QM1D1C0045_REG_1B] &= 0xFC;
	QM1D1C0045_d_reg[QM1D1C0045_REG_1B] |= QM1D1C0045_LPF_ADJUSTMENT_CURRENT_37R5UA;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_1B, QM1D1C0045_d_reg[QM1D1C0045_REG_1B]);

	/*PLL Lock*/
	{
		MT_BOOL bLock;
		QM1D1C0045_get_lock_status(&bLock);
		if (bLock != TRUE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

MT_BOOL QM1D1C0045_LocalLpfCutOffSetting(PQM1D1C0045_CONFIG_STRUCT apConfig)
{
	UINT8 i_data, i;
	if (apConfig == NULL)
	{
		return FALSE;
	}

	/*div2/vco_band*/
	for (i = 0; i < 15; i++)
	{
		if (QM1D1C0045_local_f[i] == 0)
		{
			continue;
		}
		if ((QM1D1C0045_local_f[i + 1] <= apConfig->ui_QM1D1C0045_RFChannelkHz) && (QM1D1C0045_local_f[i] > apConfig->ui_QM1D1C0045_RFChannelkHz))
		{
			i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_02);
			i_data &= 0x0F;
			i_data |= ((QM1D1C0045_div2[i] << 7) & 0x80);
			i_data |= ((QM1D1C0045_vco_band[i] << 4) & 0x70);
			QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_02, i_data);
		}
	}

	/*PLL Counter*/
	{
		int F_ref, pll_ref_div, alpha, N, A, sd;
		//double M, beta;
		int M, beta;

		if (apConfig->ui_QM1D1C0045_XtalFreqKHz == DEF_XTAL_FREQ)
		{
			F_ref = apConfig->ui_QM1D1C0045_XtalFreqKHz;
			pll_ref_div = 0;
		}
		else
		{
			F_ref = (apConfig->ui_QM1D1C0045_XtalFreqKHz >> 1);
			pll_ref_div = 1;
		}

		//M = (double)((apConfig->ui_QM1D1C0045_RFChannelkHz) / ((double)F_ref));
		//alpha = (int)(M + 0.5);
		//beta = (double)(M - alpha);
		//N = (int)((double)(alpha - 12.0) / 4.0);
		//A = alpha - 4 * (N + 1) - 5;

		M = apConfig->ui_QM1D1C0045_RFChannelkHz / F_ref;
		alpha = (apConfig->ui_QM1D1C0045_RFChannelkHz + F_ref / 2) / F_ref;
		beta = M - alpha;
		N = (int)((alpha - 12) / 4);
		A = alpha - 4 * (N + 1) - 5;

		if (beta >= 0)
		{
			// sd = (int)(pow(2.,20.)*beta);
			sd = (int)(mypow(2, 20) * beta);
		}
		else
		{
			// sd = (int)(0x400000 + pow(2.,20.)*beta);
			sd = (int)(0x400000 + mypow(2, 20) * beta);
		}
		QM1D1C0045_d_reg[QM1D1C0045_REG_06] &= 0x40;
		QM1D1C0045_d_reg[QM1D1C0045_REG_06] |= ((pll_ref_div << 7) | (N));
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_06, QM1D1C0045_d_reg[QM1D1C0045_REG_06]);

		QM1D1C0045_d_reg[QM1D1C0045_REG_07] &= 0xF0;
		QM1D1C0045_d_reg[QM1D1C0045_REG_07] |= (A & 0x0F);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_07, QM1D1C0045_d_reg[QM1D1C0045_REG_07]);

		/*LPF_CLK , LPF_FC*/
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_08, QM1D1C0045_d_reg[QM1D1C0045_REG_08]);

		QM1D1C0045_d_reg[QM1D1C0045_REG_09] &= 0xC0;
		QM1D1C0045_d_reg[QM1D1C0045_REG_09] |= ((sd >> 16) & 0x3F);
		QM1D1C0045_d_reg[QM1D1C0045_REG_0A] = (UINT8)((sd >> 8) & 0xFF);
		QM1D1C0045_d_reg[QM1D1C0045_REG_0B] = (UINT8)(sd & 0xFF);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_09, QM1D1C0045_d_reg[QM1D1C0045_REG_09]);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0A, QM1D1C0045_d_reg[QM1D1C0045_REG_0A]);
		QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0B, QM1D1C0045_d_reg[QM1D1C0045_REG_0B]);
	}

	/*CSEL_Offset*/
	QM1D1C0045_d_reg[QM1D1C0045_REG_13] &= 0x9F;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_13, QM1D1C0045_d_reg[QM1D1C0045_REG_13]);

	/*VCO_TM , LPF_TM*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_0C);
	i_data &= 0x7F;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0C, i_data);
	TP_Delay(1); // 1024usec

	/*VCO_TM , LPF_TM*/
	i_data = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_0C);
	i_data |= 0x80;
	QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_0C, i_data);

	if (QM1D1C0045_d_reg[QM1D1C0045_REG_03] & 0x01)
	{ // Fast
		TP_Delay(apConfig->ui_QM1D1C0045_FastSearchWaitTime);
	}
	else
	{
		TP_Delay(apConfig->ui_QM1D1C0045_NormalSearchWaitTime);
	}

	/*PLL Lock*/
	{
		MT_BOOL bLock;
		QM1D1C0045_get_lock_status(&bLock);
		if (bLock != TRUE)
		{
			return FALSE;
		}
	}
	return TRUE;
}

void QM1D1C0045_set_lpf(QM1D1C0045_LPF_FC lpf)
{
	QM1D1C0045_d_reg[QM1D1C0045_REG_08] &= 0xF0;
	QM1D1C0045_d_reg[QM1D1C0045_REG_08] |= (lpf & 0x0F);
}

MT_BOOL QM1D1C0045_Set_Operation_Param(PQM1D1C0045_CONFIG_STRUCT apConfig)
{
	UINT8 u8TmpData;
	MT_BOOL bRetValue;

	QM1D1C0045_d_reg[QM1D1C0045_REG_01] = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_01); // Volatile
	QM1D1C0045_d_reg[QM1D1C0045_REG_05] = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_05); // Volatile

	if (apConfig->b_QM1D1C0045_loop_through && apConfig->b_QM1D1C0045_tuner_standby)
	{
		// LoopThrough = Enable , TunerMode = Standby
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 3); // BB_REG_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 2); // HA_LT_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 1); // LT_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= 0x01;		// STDBY
		QM1D1C0045_d_reg[QM1D1C0045_REG_05] |= (0x01 << 3); // pfd_rst STANDBY

		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_05, QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
		if (!bRetValue)
		{
			return bRetValue;
		}
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}
	else if (apConfig->b_QM1D1C0045_loop_through && !(apConfig->b_QM1D1C0045_tuner_standby))
	{
		// LoopThrough = Enable , TunerMode = Normal
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 3);			  // BB_REG_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 2);			  // HA_LT_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 1);			  // LT_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01)) & 0xFF;	  // NORMAL
		QM1D1C0045_d_reg[QM1D1C0045_REG_05] &= (~(0x01 << 3)) & 0xFF; // pfd_rst NORMAL

		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
		if (!bRetValue)
		{
			return bRetValue;
		}
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_05, QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}
	else if (!(apConfig->b_QM1D1C0045_loop_through) && apConfig->b_QM1D1C0045_tuner_standby)
	{
		// LoopThrough = Disable , TunerMode = Standby
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01 << 3)) & 0xFF; // BB_REG_disable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01 << 2)) & 0xFF; // HA_LT_disable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01 << 1)) & 0xFF; // LT_disable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= 0x01;				  // STDBY
		QM1D1C0045_d_reg[QM1D1C0045_REG_05] |= (0x01 << 3);			  // pfd_rst STANDBY

		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_05, QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
		if (!bRetValue)
		{
			return bRetValue;
		}
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}
	else
	{ //!(iLoopThrough) && !(iTunerMode)
		// LoopThrough = Disable , TunerMode = Normal
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 3);			  // BB_REG_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] |= (0x01 << 2);			  // HA_LT_enable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01 << 1)) & 0xFF; // LT_disable
		QM1D1C0045_d_reg[QM1D1C0045_REG_01] &= (~(0x01)) & 0xFF;	  // NORMAL
		QM1D1C0045_d_reg[QM1D1C0045_REG_05] &= (~(0x01 << 3)) & 0xFF; // pfd_rst NORMAL

		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_01, QM1D1C0045_d_reg[QM1D1C0045_REG_01]);
		if (!bRetValue)
		{
			return bRetValue;
		}
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_05, QM1D1C0045_d_reg[QM1D1C0045_REG_05]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}

	/*Head Amp*/
	u8TmpData = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_01);
	if (u8TmpData & 0x04)
	{
		apConfig->b_QM1D1C0045_head_amp = TRUE;
	}
	else
	{
		apConfig->b_QM1D1C0045_head_amp = FALSE;
	}
	return TRUE;
}

MT_BOOL QM1D1C0045_set_searchmode(PQM1D1C0045_CONFIG_STRUCT apConfig)
{
	MT_BOOL bRetValue;

	if (apConfig->b_QM1D1C0045_fast_search_mode)
	{
		QM1D1C0045_d_reg[QM1D1C0045_REG_03] = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_03);
		QM1D1C0045_d_reg[QM1D1C0045_REG_03] |= 0x01;
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_03, QM1D1C0045_d_reg[QM1D1C0045_REG_03]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}
	else
	{
		QM1D1C0045_d_reg[QM1D1C0045_REG_03] = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_03);
		QM1D1C0045_d_reg[QM1D1C0045_REG_03] &= 0xFE;
		bRetValue = QM1D1C0045_pll_setdata_once(QM1D1C0045_REG_03, QM1D1C0045_d_reg[QM1D1C0045_REG_03]);
		if (!bRetValue)
		{
			return bRetValue;
		}
	}
	return TRUE;
}

void QM1D1C0045_get_lock_status(MT_BOOL *pbLock)
{
	QM1D1C0045_d_reg[QM1D1C0045_REG_0D] = QM1D1C0045_pll_getdata_once(QM1D1C0045_REG_0D);
	if (QM1D1C0045_d_reg[QM1D1C0045_REG_0D] & 0x40)
	{
		*pbLock = TRUE;
	}
	else
	{
		*pbLock = FALSE;
	}
}

MT_BOOL RfTunerI2CWriteHandler(UINT8 aSlvAddr, UINT8 *apData, UINT16 *apLength)
{
	TP_iic_tuner_write(aSlvAddr, apData, *apLength);
	// int i;
	// #ifdef	SHARP_DEBUG_4R5GEN_FOR_CYGWIN
	// printf(HERE "\t");
	// #endif
	// for(i=0 ; i<*apLength ; i++){
	//	printf("apData = 0x%.2X , " , apData[i]);
	// }
	// printf("\n");
	return TRUE;
	// #endif
}

MT_BOOL RfTunerI2CReadHandler(UINT8 aSlvAddr, UINT8 *apData, UINT16 *apLength)
{
	TP_iic_tuner_read(aSlvAddr, apData[0], apData, *apLength);
	return TRUE;
}

/*Add main func. for make */

int QM1D1C0045_init(UINT32 channel_freq, unsigned int rate)
{
	//int choice = -1; // , i;
	// int RfFreq, XtalFreq;// , LPF;
	int XtalFreq;
	MT_BOOL bRetVal;
//	MT_BOOL LoopThrough , StandbyMode;
#ifdef TEMP_CODE
	bRetVal = QM1D1C0045_set_i2c_handler((QM1D1C0045_I2C_WR_HANDLER)(RfTunerI2CWriteHandler), (QM1D1C0045_I2C_RD_HANDLER)(RfTunerI2CReadHandler));
	if (!bRetVal)
	{
		return FALSE;
	}
	QM1D1C0045_i2c_slave_addr_set(SlvAddr);
#endif
	QM1D1C0045ConfigList.b_QM1D1C0045_fast_search_mode = FALSE;
	QM1D1C0045ConfigList.b_QM1D1C0045_loop_through = FALSE; // TRUE;//false
	QM1D1C0045ConfigList.b_QM1D1C0045_tuner_standby = FALSE;
	bRetVal = QM1D1C0045_Initialize(&QM1D1C0045ConfigList);
	//	if(!bRetVal){
	//		printf("Initialize Error\n");
	//	}

#ifdef TEMP_CODE
	// set operation param
	/*Loop Through*/
	QM1D1C0045ConfigList.b_QM1D1C0045_loop_through = FALSE;
	/*Standby Mode*/
	QM1D1C0045ConfigList.b_QM1D1C0045_tuner_standby = FALSE;
	QM1D1C0045_Set_Operation_Param(&QM1D1C0045ConfigList);
#endif
	/*RF Freq [kHz]*/
	QM1D1C0045ConfigList.ui_QM1D1C0045_RFChannelkHz = channel_freq;
	XtalFreq = 16000;
	QM1D1C0045ConfigList.ui_QM1D1C0045_XtalFreqKHz = XtalFreq;
	// printf("RF Freq = %d[kHz] , X'tal Freq = %d[kHz]\n\n" , channel_freq , XtalFreq);

	/*X'tal Freq [kHz]*/
	//	QM1D1C0045ConfigList.QM1D1C0045_lpf = (QM1D1C0045_LPF_FC)QM1D1C0045_LPF_FC_20MHz;
	QM1D1C0045ConfigList.QM1D1C0045_lpf = rate;
	QM1D1C0045ConfigList.ui_QM1D1C0045_LpfWaitTime = 20;

	bRetVal = QM1D1C0045_LocalLpfTuning(&QM1D1C0045ConfigList);
	if (!bRetVal)
	{
		// printf("LocalLpfTuning Error\n");
	}

	/*RF Freq [kHz]*/
	// if(RfFreq<950000 || RfFreq>2150000){
	//	printf("%d [kHz] is not supported\n",channel_freq);
	// }else{
	QM1D1C0045ConfigList.ui_QM1D1C0045_RFChannelkHz = channel_freq;
	//	break;
	//}
	// printf("RF Freq = %d[kHz]\n\n" , channel_freq);

	/*X'tal Freq [kHz]*/
	// printf("X'tal Freq [kHz] .......");
	// scanf("%d",&XtalFreq);
	QM1D1C0045ConfigList.ui_QM1D1C0045_XtalFreqKHz = 16000;
	// printf("RF Freq = %d[kHz] , X'tal Freq = %d[kHz]\n\n" , channel_freq , XtalFreq);

	QM1D1C0045ConfigList.ui_QM1D1C0045_FastSearchWaitTime = 4;
	QM1D1C0045ConfigList.ui_QM1D1C0045_NormalSearchWaitTime = 15;

	bRetVal = QM1D1C0045_LocalLpfCutOffSetting(&QM1D1C0045ConfigList);
	if (!bRetVal)
	{
		// printf("LocalLpfCutOffSetting Error\n");
	}
	return 0;
}

#endif //_USE_TP5001_CHIP_
