/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#include <linux/printk.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"

#include "drv_gpio_ioctl.h"
#include "port_hd2502.h"
//#include "stdafx.h"
//#include "math.h"
#include "HDICType.h"
#include "HDIC2501.h"
#include "mt_fe_tn_montage_ts6011.h"

extern void HD_Delay(UINT32 uiMS);
extern UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
extern UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);

extern mt_fe_hd2502_priv_handle g_hd2502_priv;


#define TUNER_RDA5815M     0
#define TUNER_RDA5812        1
#define TUNER_ISHARP          2
#define TUNER_TS6011          3

UINT8 hd2502_tuner_type = 0;




const unsigned short mt_fe_log10_table[][2] = 
{
	{   0,     0},
	{   1,     0},
	{   2,  3010},
	{   3,  4771},
	{   4,  6020},
	{   5,  6989},
	{   6,  7781},
	{   7,  8450},
	{   8,  9030},
	{   9,  9542},
	{  10, 10000},
	{  11, 10413},
	{  12, 10791},
	{  13, 11139},
	{  14, 11461},
	{  15, 11760},
	{  16, 12041},
	{  17, 12304},
	{  18, 12552},
	{  19, 12787},
	{  20, 13010},
	{  21, 13222},
	{  22, 13424},
	{  23, 13617},
	{  24, 13802},
	{  25, 13979},
	{  26, 14149},
	{  27, 14313},
	{  28, 14471},
	{  29, 14623},
	{  30, 14771},
	{  31, 14913},
	{  32, 15051},
	{  33, 15185},
	{  34, 15314},
	{  35, 15440},
	{  36, 15563},
	{  37, 15682},
	{  38, 15797},
	{  39, 15910},
	{  40, 16020},
	{  41, 16127},
	{  42, 16232},
	{  43, 16334},
	{  44, 16434},
	{  45, 16532},
	{  46, 16627},
	{  47, 16720},
	{  48, 16812},
	{  49, 16901},
	{  50, 16989},
	{  51, 17075},
	{  52, 17160},
	{  53, 17242},
	{  54, 17323},
	{  55, 17403},
	{  56, 17481},
	{  57, 17558},
	{  58, 17634},
	{  59, 17708},
	{  60, 17781},
	{  61, 17853},
	{  62, 17923},
	{  63, 17993},
	{  64, 18061},
	{  65, 18129},
	{  66, 18195},
	{  67, 18260},
	{  68, 18325},
	{  69, 18388},
	{  70, 18450},
	{  71, 18512},
	{  72, 18573},
	{  73, 18633},
	{  74, 18692},
	{  75, 18750},
	{  76, 18808},
	{  77, 18864},
	{  78, 18920},
	{  79, 18976},
	{  80, 19030},
	{  81, 19084},
	{  82, 19138},
	{  83, 19190},
	{  84, 19242},
	{  85, 19294},
	{  86, 19344},
	{  87, 19395},
	{  88, 19444},
	{  89, 19493},
	{  90, 19542},
	{  91, 19590},
	{  92, 19637},
	{  93, 19684},
	{  94, 19731},
	{  95, 19777},
	{  96, 19822},
	{  97, 19867},
	{  98, 19912},
	{  99, 19956},
	{ 100, 20000},
	{ 101, 20043},
	{ 114, 20569},
	{ 147, 21673},
	{ 220, 23424},
	{ 295, 24698},
	{ 330, 25185},
	{ 403, 26053},
	{ 489, 26893},
	{ 554, 27435},
	{ 733, 28651},
	{ 876, 29425},
	{1000, 30000}
};

int mt_fe_get_line_index(int value)
{
	int line_index = 0;
	int line_cnt = sizeof(mt_fe_log10_table) / sizeof(unsigned short) / 2;

	if (value < 0)
	{
		return 0;
	}

	for (line_index = 0; line_index < line_cnt; line_index++)
	{
		if (value < mt_fe_log10_table[line_index][0])
		{
			//printf("\tCurrent Line Index is %d\n", line_index);
			break;
		}
	}

	return line_index;
}

int mt_fe_calc_log10_10000_from_table(int value)
{
	int line_index;
	int calc_log10 = 0;

	int line_cnt = sizeof(mt_fe_log10_table) / sizeof(unsigned short) / 2;

	line_index = mt_fe_get_line_index(value);

	if (line_index <= 101)
		calc_log10 = mt_fe_log10_table[line_index - 1][1];
	else if (line_index == 0)
		calc_log10 = 0;
	else if (line_index >= line_cnt)
		calc_log10 = 30000;
	else
	{
		int delta = 0;

		calc_log10 = mt_fe_log10_table[line_index - 1][1];

		delta = (mt_fe_log10_table[line_index][1] - mt_fe_log10_table[line_index - 1][1]) * (value - mt_fe_log10_table[line_index - 1][0]) / (mt_fe_log10_table[line_index][0] - mt_fe_log10_table[line_index - 1][0]);

		calc_log10 += delta;
	}

	return calc_log10;
}



/*******************************************************************
*
* 2501初始化特定寄存器,需要在上电硬复位后调用此函数.
*
*******************************************************************/
INT8 HDIC2501_InitReg(void)
{
	UINT8 err = HDIC_NO_ERROR;

	HDIC2501_WriteRegister(0x22, 0xA8);
	HDIC2501_WriteRegister(0x6F, 0x20);

	return err;
}

/*******************************************************************
*
* 判断2501是否锁住信号，判断条件设置为0x60时，输出TS可能有错误，
* 设置判断条件为0x10，更为严格，但可能实际数据正确，但判定为失锁。
* 
*******************************************************************/
INT8 HDIC2501_IsDemodLocked(UINT8 *locked, UINT8 *Status)
{
	UINT8 err = HDIC_NO_ERROR;

	HDIC2501_ReadRegister(0x10, Status);

	if (((*Status) & 0x60) == 0x60)
	{
		*locked = 0x01;
	}
	else
	{
		*locked = 0x00;
	}

	return err;
}

/*******************************************************************
*
* 设置2501 TS输出格式
* 参数1:TSEDGE_POSEDGE、TSEDGE_NEGEDGE。上升沿、下降沿。
* 参数2:TS_SERIAL、TS_PARALLEL。串行TS、并行TS。
* 参数3:NULLPACKET_ENABLED、NULLPACKET_DELETED。允许空包、去空包。
* 
*******************************************************************/
INT8 HDIC2501_SetTsFormat(UINT8 Edge, UINT8 Serial, UINT8 NullPacket)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData = 0;

	HDIC2501_ReadRegister(0x05, &ReadData);
	ReadData &= 0xF0;
	ReadData |= 0x04;
	ReadData = ReadData | Edge | Serial | NullPacket;

	HDIC2501_WriteRegister(0x05, ReadData);

	return err;
}

/*******************************************************************
*
* 设置2501的符号率
* 例如符号率是28.8M, 4个寄存器的值应为0x01, 0xB7, 0x74, 0x00
*
*******************************************************************/
INT8 HDIC2501_SetSymbolRate(UINT32 SymbolRate)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 D3B, D3A, D39, D38;

	D3B = (UINT8)((SymbolRate & 0xff000000) >> 24);
	HDIC2501_WriteRegister(0x3B, D3B);

	D3A = (UINT8)((SymbolRate & 0x00ff0000) >> 16);
	HDIC2501_WriteRegister(0x3A, D3A);

	D39 = (UINT8)((SymbolRate & 0x0000ff00) >> 8);
	HDIC2501_WriteRegister(0x39, D39);

	D38 = (UINT8)((SymbolRate & 0x000000ff));
	HDIC2501_WriteRegister(0x38, D38);

	return (err);
}

/*******************************************************************
*
* 设置2501的工作
* 参数1:SYMBOLRATEAUTODETECT_AUTO、SYMBOLRATEAUTODETECT_MANUAL.自动符号率、手动符号率。
* 参数2:符号率。参数1设置为手动符号率时起效。
*
*******************************************************************/
INT8 HDIC2501_SetParameters(UINT8 SymbolRateAutoDetect, UINT32 SymbolRate_Hz)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 WriteData = 0x0F;

	WriteData |= SymbolRateAutoDetect;
	HDIC2501_WriteRegister(0x04, WriteData);

	if (SymbolRateAutoDetect == SYMBOLRATEAUTODETECT_MANUAL)
	{
		// Write SymbolRate_Hz
		HDIC2501_SetSymbolRate(SymbolRate_Hz);
	}

	return err;
}

/*******************************************************************
 *
 * 获取信号具体参数
 *
 *
 *******************************************************************/
INT8 HDIC2501_GetMode(UINT8 *Constellation, UINT8 *CodingRate, UINT8 *ScramblerMode,
							UINT8 *TSorGS, UINT8 *RollOff, UINT8 *OperationMode, UINT8 *TS_ID)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData = 0;

	HDIC2501_ReadRegister(0x04, &ReadData);

	/*
	Auto or manual, need read different register
	*/
	if ((ReadData & 0x0F) == 0x0F)
	{
		HDIC2501_ReadRegister(0x01, &ReadData);
	}
	else
	{
		HDIC2501_ReadRegister(0x03, &ReadData);
	}
	*Constellation = ReadData & CONSTELLATION_MASK;
	*CodingRate = ReadData & CODINGRATE_MASK;
	*ScramblerMode = ReadData & SCRAMBERMODE_MASK;

	if ((ReadData & CONSTELLATION_MASK) == CONSTELLATION_QPSK)
	{
		HDIC_Print(("Constellation : QPSK"));
	}
	else if ((ReadData & CONSTELLATION_MASK) == CONSTELLATION_8PSK)
	{
		HDIC_Print(("Constellation : 8PSK"));
	}

	// Display CodingRate
	if ((ReadData & CODINGRATE_MASK) == CODINGRATE_1_2)
	{
		HDIC_Print(("Coding Rate : 1/2"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_3_5)
	{
		HDIC_Print(("Coding Rate : 3/5"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_2_3)
	{
		HDIC_Print(("Coding Rate : 2/3"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_3_4)
	{
		HDIC_Print(("Coding Rate : 3/4"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_4_5)
	{
		HDIC_Print(("Coding Rate : 4/5"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_5_6)
	{
		HDIC_Print(("Coding Rate : 5/6"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_13_15)
	{
		HDIC_Print(("Coding Rate : 13/15"));
	}
	else if ((ReadData & CODINGRATE_MASK) == CODINGRATE_9_10)
	{
		HDIC_Print(("Coding Rate : 9/10"));
	}

	// Display  Scrambler Mode
	if ((ReadData & SCRAMBERMODE_MASK) == SCRAMBERMODE_MODE0)
	{
		HDIC_Print(("Scrambler Mode : Mode0"));
	}
	else if ((ReadData & SCRAMBERMODE_MASK) == SCRAMBERMODE_MODE1)
	{
		HDIC_Print(("Scrambler Mode : Mode1"));
	}
	else if ((ReadData & SCRAMBERMODE_MASK) == SCRAMBERMODE_MODE2)
	{
		HDIC_Print(("Scrambler Mode : Mode2"));
	}
	else if ((ReadData & SCRAMBERMODE_MASK) == SCRAMBERMODE_MODE3)
	{
		HDIC_Print(("Scrambler Mode : Mode3"));
	}

	/*Read TS configuration*/
	//========================================
	HDIC2501_ReadRegister(0xA0, &ReadData);
	*TSorGS = ReadData & TSORGS_MASK;
	*RollOff = ReadData & ROLLOFF_MASK;
	*OperationMode = ReadData & OPERATIONSMODE_MASK;
	*TS_ID = ReadData & 0x0F;
	//===Read from A0 Reg,Not the same value in reg 02 & 05
	// D7: 0: TS, 1: GS;
	// D6: 0: CCM&VCM, 1: ACM;
	// D5,D4: 00: 0.35 rolloff;  01: 0.25 rolloff; 10: 0.20 rolloff; 11: reversed rolloff
	// D7: 0: TS, 1: GS;
	if ((ReadData & TSORGS_MASK) == TSORGS_GS)
	{
		HDIC_Print(("GS  "));
	}
	else
	{
		HDIC_Print(("TS  "));
	}

	// D6: 0: CCM&VCM, 1: ACM;
	if ((ReadData & OPERATIONSMODE_MASK) == 0x40)
	{
		HDIC_Print(("ACM"));
	}
	else
	{
		HDIC_Print(("CCM&VCM"));
	}

	// D5,D4: 00: 0.35 rolloff;  01: 0.25 rolloff; 10: 0.20 rolloff; 11: reversed rolloff
	if ((ReadData & ROLLOFF_MASK) == 0x00)
	{
		HDIC_Print(("0.35 rolloff"));
	}
	else if ((ReadData & ROLLOFF_MASK) == 0x10)
	{
		HDIC_Print(("0.25 rolloff"));
	}
	else if ((ReadData & ROLLOFF_MASK) == 0x20)
	{
		HDIC_Print(("0.20 rolloff"));
	}
	else if ((ReadData & ROLLOFF_MASK) == 0x30)
	{
		HDIC_Print(("reversed rolloff"));
	}

	HDIC_Print(("TS ID : %d", (ReadData & 0x0F)));

	return err;
}

/*******************************************************************
*
* 获取信号的符号率
*
*******************************************************************/
INT8 HDIC2501_GetSymbolRate(UINT32 *SymbolRate)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData = 0;

	HDIC2501_ReadRegister(0x34, &ReadData);
	*SymbolRate = ReadData;

	HDIC2501_ReadRegister(0x35, &ReadData);
	*SymbolRate <<= 8;
	*SymbolRate |= ReadData;

	HDIC2501_ReadRegister(0x36, &ReadData);
	*SymbolRate <<= 8;
	*SymbolRate |= ReadData;

	HDIC2501_ReadRegister(0x37, &ReadData);
	*SymbolRate <<= 8;
	*SymbolRate |= ReadData;

	return (err);
}

/*******************************************************************
*
* 获取信号的信噪比
*
*******************************************************************/
#if 1
INT8 HDIC2501_GetSignalSNR(UINT32 *SignalSNR)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;
	UINT32 SNR_Temp;

	HDIC2501_ReadRegister(0x65, &ReadData1);
	HDIC2501_ReadRegister(0x66, &ReadData2);
	SNR_Temp = (UINT32)(ReadData2 * 256 + ReadData1);

	if (SNR_Temp != 0)
	{
		//*SignalSNR = 10 * log10((double)8192 / SNR_Temp);
		*SignalSNR = mt_fe_calc_log10_10000_from_table(8192 / SNR_Temp);
	}

	return (err);
}
#else
INT8 HDIC2501_GetSignalSNR(double *SignalSNR)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;
	UINT32 SNR_Temp;

	HDIC2501_ReadRegister(0x65, &ReadData1);
	HDIC2501_ReadRegister(0x66, &ReadData2);
	SNR_Temp = ReadData2 * 256 + ReadData1;

	if (SNR_Temp != 0)
	{
		//*SignalSNR = 10 * log10((double)8192 / SNR_Temp);
	}

	return (err);
}
#endif

/*******************************************************************
*
* 获取信号的误码率
*
*******************************************************************/
#if 1
INT8 HDIC2501_GetSignalBER(UINT32 *pSignalBER)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0, ReadData3 = 0;
	UINT32 Temp;

	HDIC2501_ReadRegister(0x0D, &ReadData1);
	HDIC2501_ReadRegister(0x0E, &ReadData2);
	HDIC2501_ReadRegister(0x0F, &ReadData3);

	Temp = (UINT32)((ReadData3 << 16) | (ReadData2 << 8) | ReadData1);

	*pSignalBER = Temp;

	return (err);
}

/*******************************************************************
*
* 获取2501的LDPC的误码率
*
*******************************************************************/
INT8 HDIC2501_GetLdpcBER(UINT16 *pLdpcBER)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	HDIC2501_ReadRegister(0xA1, &ReadData1);
	HDIC2501_ReadRegister(0xA2, &ReadData2);

	*pLdpcBER = (ReadData2 & 0x0f) * 256 + ReadData1;

	return (err);
}

#else
INT8 HDIC2501_GetSignalBER(double *pSignalBER)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0, ReadData3 = 0;
	UINT32 Temp;

	HDIC2501_ReadRegister(0x0D, &ReadData1);
	HDIC2501_ReadRegister(0x0E, &ReadData2);
	HDIC2501_ReadRegister(0x0F, &ReadData3);

	Temp = (UINT32)((ReadData3 << 16) | (ReadData2 << 8) | ReadData1);

	*pSignalBER = ((double)Temp) / 1000000;

	return (err);
}

/*******************************************************************
*
* 获取2501的LDPC的误码率
*
*******************************************************************/
INT8 HDIC2501_GetLdpcBER(double *pLdpcBER)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	HDIC2501_ReadRegister(0xA1, &ReadData1);
	HDIC2501_ReadRegister(0xA2, &ReadData2);

	*pLdpcBER = (double)((ReadData2 & 0x0f) * 256 + ReadData1) / 4096;

	return (err);
}
#endif

/*******************************************************************
*
* 获取信号强度
*
*******************************************************************/
INT8 HDIC2501_GetFieldStrength(UINT16 *FieldStrength)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0, ReadData2 = 0;

	HDIC2501_ReadRegister(0x48, &ReadData1);
	HDIC2501_ReadRegister(0x49, &ReadData2);

	*FieldStrength = (ReadData1 & 0x0f) * 256 + ReadData2;

	return (err);
}

/*******************************************************************
*
* 2501软复位,在配置完参数后,调用此函数让2501重新工作
*
*******************************************************************/
INT8 HDIC2501_SoftReset(void)
{
	UINT8 err = HDIC_NO_ERROR;

	HDIC2501_WriteRegister(0x06, 0xF0);
	HDIC2501_Wait(10);
	HDIC2501_WriteRegister(0x06, 0xE0);

	return (err);
}


UINT8 HDIC2501_SetPolar(UINT8 Polar)
{
	UINT8 readData = 0;

	HDIC2501_ReadRegister(0x12, &readData);
	if (Polar)
	{
		readData &= 0xEF;
		readData |= 0x10;
	}
	else
	{
		readData |= 0x10;
		readData &= 0xEF;
	}

	HDIC2501_WriteRegister(0x12, readData);

	return HDIC_NO_ERROR;
}


/*******************************************************************
*
* 打开2501 Tuner I2C转发功能
* 参数:Tuner的I2C设备地址.
*
*******************************************************************/
INT8 HDIC2501_OpenTunerI2C(UINT8 Address)
{
	UINT8 err = HDIC_NO_ERROR;

	// 20151126 规避控制tuner期间异常的中频信号，关闭AGC控制，并设置AGC放大倍数最小
	HDIC2501_WriteRegister(0x22, 0xA9);
	// 20151126
	HDIC2501_WriteRegister(0x0A, Address);
	HDIC2501_WriteRegister(0x09, 0x01);

	return (err);
}

/*******************************************************************
*
* 关闭2501 Tuner I2C转发功能
*
*******************************************************************/
INT8 HDIC2501_CloseTunerI2C(void)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 ReadData1 = 0;
	
	HDIC2501_WriteRegister(0x09, 0x00);
	HDIC2501_WriteRegister(0x0A, 0x00);

	// 20151126 规避控制tuner期间异常的中频信号，关闭AGC控制，并设置AGC放大倍数最小，此处打开AGC控制
	HDIC2501_WriteRegister(0x22, 0xA8);
	// 20151126
	HDIC2501_ReadRegister(0x22, &ReadData1);
	return (err);
}

/*******************************************************************
*
* 计算此符号下的tuner SAW的最佳值.
* tuner的SAW(滤波器)带宽可根据信号的符号率和滚降系数计算出一个最佳值
* 将Tuner设置为此值时,demod的性能最佳.
*
*******************************************************************/
INT16 HDIC2501_GetNewSAW(UINT32 SymbolRate, UINT8 Rolloff)
{
	UINT8 Temp1 = 0;
	UINT16 SAW;
	UINT32 Temp2;

	if (Rolloff == ROLLOFF_0_35)
	{
		Temp1 = 135;
	}
	else if (Rolloff == ROLLOFF_0_25)
	{
		Temp1 = 125;
	}
	else if (Rolloff == ROLLOFF_0_20)
	{
		Temp1 = 120;
	}

	// 计算出新的SAW设置,公式为SAW = SymbolRate*(1+滚降系数)/2;
	// 我们再将单位转为为10K.用户可以在下次锁同一频点时使用此SAW设置.
	Temp2 = (UINT32)(SymbolRate * Temp1 / 200);
	SAW = (UINT16)(Temp2 / 10000);
	
	return (SAW);
}

/******************************************************************
*
* The following functions need customers work.
* 下面的函数需要客户根据平台,自己实现
*
*******************************************************************/

/********************************************
* 配置Tuner
* 以SHARP Tuner为例子,并使用了I2C转发功能,
* 即Tuner的I2C是通过2501转发
********************************************/
INT8 HDIC2501_SetTuner(UINT32 Frequency)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 Tuner_Addr = 0xC0; // Set Tuner address here,0xC0 is just an example
	UINT16 Lpf;
	UINT8 ReadData = 0;


	if (hd2502_tuner_type == TUNER_RDA5815M)		// RDA5815M
	{
		Tuner_Addr = RDA5812_I2C_ADDRESS;

		HDIC2501_OpenTunerI2C(Tuner_Addr);

		RDA5815mInitial();

		RDA5815mSet(Frequency, 28800);
	}
	else  if (hd2502_tuner_type == TUNER_TS6011)		// TS6011
	{
		Tuner_Addr = TS6011_I2C_ADDRESS;//TS6011_I2C_ADDRESS;
		//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		HDIC2501_OpenTunerI2C(Tuner_Addr);
		//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		mt_fe_tn_init_ts6011_hd2502(g_hd2502_priv);
		//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		mt_fe_tn_ts6011_set_freq_hd2502((MT_FE_Tuner_Handle_TS6011)g_hd2502_priv->tuner_handle, Frequency*1000, 28800, 0);
		//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		if(g_hd2502_priv->tuner_handle){
			//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
			mt_fe_tn_adjust_AGC_ts6011_hd2502((MT_FE_Tuner_Handle_TS6011)g_hd2502_priv->tuner_handle );
		}
		//printk("[ %s : %d ]======  \n",__FUNCTION__,__LINE__);
		
	}
	else if (hd2502_tuner_type == TUNER_RDA5812)	// RDA5812
	{
		/*******************************************
		 * 
		 * Now Please config Tuner, RDA5812 Tuner作为例子
		 *
		 ********************************************/

		Tuner_Addr = RDA5812_I2C_ADDRESS;

		HDIC2501_OpenTunerI2C(Tuner_Addr);

		RDA5812Initial();

		/*需要锁两次*/
		RDA5812Set(Frequency, 28800, 0);
		RDA5812Set(Frequency, 28800, 0);
	}
	else if (hd2502_tuner_type == TUNER_ISHARP)		// ISharp
	{
		Tuner_Addr = ISHARP_TUNER_ADDR;

		HDIC2501_OpenTunerI2C(Tuner_Addr);

		/*******************************************
		 * 
		 * Now Please config Tuner, SHARP Tuner作为例子
		 *
		 ********************************************/

		Lpf = Sharp_ITuner_CalculateLPF(2000);//SAW

		Sharp_ITuner_Lock(Tuner_Addr, Frequency * 10, Lpf, 0);
		while(1)
		{
			ReadData = Sharp_ITuner_GetLockStatus(Tuner_Addr);
			if (ReadData == 0x48)
			{
				// Tuner锁住,跳出
				break;
			}
		}
	}
	

	HDIC2501_CloseTunerI2C();

	return err;
}

/********************************************
* 写2501的寄存器
* 2501是双字节的寄存器,加上寄存器的值,需要向2501
* 写三个字节的数据
********************************************/
INT8 HDIC2501_WriteRegister(UINT8 Register, UINT8 Data)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 RegisterData[3];

	RegisterData[0] = 0x00;
	RegisterData[1] = Register;
	RegisterData[2] = Data;
	Write_I2C(HDIC2501_I2C_ADDRESS, 3, RegisterData);

	return (err);
}

/********************************************
* 读2501的寄存器
* 2501是双字节的寄存器,所以先写入2个字的寄存器地址,
* 然后可以不需要产生停止位,再从2501读出1个字节.
********************************************/
INT8 HDIC2501_ReadRegister(UINT8 Register, UINT8 *Data)
{
	UINT8 err = HDIC_NO_ERROR;
	UINT8 RegisterData[2];

	RegisterData[0] = 0x00;
	RegisterData[1] = Register;

	Write_I2C(HDIC2501_I2C_ADDRESS, 2, RegisterData);
	/*Or you should use Write_I2C_WithoutStop()*/

	Read_I2C(HDIC2501_I2C_ADDRESS, 1, Data);

	return (err);
}

/**********************************
* 延时函数,单位毫秒
***********************************/
void HDIC2501_Wait(UINT16 millisecond)
{
	HD_Delay(millisecond);
}

void Pio_Init(UINT8 gpio_pin)
{
	if(gpio_pin == GPIO_70)
	{
		// BF13C118[2:0]=1 
		// GPIO_Mask:0xBF0A002C[6 ]=1，Write_EN:0xBF0A0024[6 ]=0，Set data 0xBF0A0020[6 ]=0 or 1                 

		UINT32 gpio_pinmux_val = 0;

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF13C118));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val &= 0xFFFFFFF8;
		gpio_pinmux_val |= 0x00000001;
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF13C118), gpio_pinmux_val);

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A002C));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val |= (1 << 6);
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A002C), gpio_pinmux_val);

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0024));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val &= ~(1 << 6);
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0024), gpio_pinmux_val);

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0020));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val |= (1 << 6);
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0020), gpio_pinmux_val);
	}
	else if(gpio_pin == GPIO_95)
	{
		// BF13C17C[2:0]=1 
		// GPIO_Mask:0xBF0A002C[31]=1，Write_EN:0xBF0A0024[31]=0，Set data 0xBF0A0020[31]=0 or 1   

		UINT32 gpio_pinmux_val = 0;

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF13C17C));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val &= 0xFFFFFFF8;
		gpio_pinmux_val |= 0x00000001;
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF13C17C), gpio_pinmux_val);

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A002C));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val |= (1 << 31);
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A002C), gpio_pinmux_val);

		gpio_pinmux_val = HAL_GET_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0024));
		//printk("gpio_pinmux_val1: 0xBF0A002C = 0x%08x\n", gpio_pinmux_val);
		gpio_pinmux_val &= ~(1 << 31);
		HAL_PUT_U32((volatile void *)SYMPHONY_IO_VA(0xBF0A0024), gpio_pinmux_val);
	}
	else
	{
		printk(" Pio_Init error >>>> %d\n",gpio_pin);
		return;
	}
	drv_gpio_io_enable(gpio_pin, TRUE);
	drv_gpio_set_dir(gpio_pin, GPIO_DIR_OUTPUT);
}

void Pio_SetHigh(UINT8 gpio_pin)
{
	drv_gpio_set_value(gpio_pin, GPIO_VALUE_HIGH_LEVEL);
}

void Pio_SetLow(UINT8 gpio_pin)
{
	drv_gpio_set_value(gpio_pin, GPIO_VALUE_LOW_LEVEL);
}


/*******************************************
* 
* 2501 Pin28 RST_N,
* 需要控制2501的第28脚,需要同硬件人员确认28脚和主芯片的哪个PIO脚连接
* 控制方法是,拉高--拉低--拉高
* 
********************************************/
INT8 HDIC2501_HWReset(int tuner_id)
{
	UINT8 err = HDIC_NO_ERROR;

	if(tuner_id == TUNER_RDA5815M)
	{
		Pio_SetHigh(GPIO_95);
		HDIC2501_Wait(10);
		Pio_SetLow(GPIO_95);
		HDIC2501_Wait(10);
		Pio_SetHigh(GPIO_95);

		HDIC2501_Wait(50);
	}
	if(tuner_id == TUNER_TS6011)
	{
		switch (symphony_get_chip_rev())
		{
			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				Pio_SetHigh(GPIO_70);
				HDIC2501_Wait(10);
				Pio_SetLow(GPIO_70);
				HDIC2501_Wait(10);
				Pio_SetHigh(GPIO_70);

				HDIC2501_Wait(50);
				break;
			case CHIP_SYMPHONY6_A1:
				printk("%s[%d] -- RC reset\n", __FUNCTION__, __LINE__);
				break;
			default:
				printk("%s[%d] -- Unknown chip\n", __FUNCTION__, __LINE__);
				break;
		}
	}

	return (err);
}

/*******************************************
* 
* 2501 Pin94 PLL_INIT_SIG,Please set GPIO to PLL reset 2501,
* 需要控制2501的第94脚,需要同硬件人员确认94脚和主芯片的哪个PIO脚连接
* 控制方法是,拉低--拉高--拉低--拉高--拉低
* 
********************************************/
INT8 HDIC2501_PLLReset(void)
{
	UINT8 err = HDIC_NO_ERROR;

#if 0
	Pio_SetLow();
	HDIC2501_Wait(10);
	Pio_SetHigh();
	HDIC2501_Wait(10);
	Pio_SetLow();
	HDIC2501_Wait(10);
	Pio_SetHigh();
	HDIC2501_Wait(10);
	Pio_SetLow();
#endif

	return (err);
}


/*******************************************************************
* 
* 需要在上电后调用一次此函数,硬复位HDIC2501芯片及初始化,此后不再需要调用.
*
*******************************************************************/
void HDIC2501_Init(int tuner_id)
{
	// PLL reset
	HDIC2501_PLLReset();

	// HW reset
	HDIC2501_HWReset(tuner_id);

	// 初始化特定寄存器
	HDIC2501_InitReg();

	//设置TS输出格式
	//HDIC2501_SetTsFormat(TSEDGE_POSEDGE, TS_SERIAL, NULLPACKET_ENABLED);
	HDIC2501_SetTsFormat(TSEDGE_POSEDGE, TS_SERIAL, NULLPACKET_DELETED);

	//HDIC2501_ReadRegister(0,  &data);
	//printk("HDIC2501_Init >>> data = %x\n",data);
}

/*******************************************************************
* 
* 2501锁频示例函数,包括配置tuner.
* 2501可以使用自动或手动模式,自动时芯片会自动侦测信号的符号率,手动则
* 需要传入符号率.
*
*******************************************************************/
void HDIC2501_Scan(void)
{
	//INT8 err = HDIC_NO_ERROR;
	// UINT8 Constellation = RX_Undefined, CodingRate = RX_Undefined, ScramblerMode = RX_Undefined, TSorGS = RX_Undefined;
	// UINT8 RollOff = RX_Undefined, OperationMode = RX_Undefined, TS_ID = RX_Undefined;
	// UINT32 SymbolRate;
	// UIN16 SAW;
	UINT8 CheckLockTemp, locked, Status;
	UINT32 Frequency = 11830; // 1M Uint // 频率值

	//------------------------------------------
	// Please set the tuner parameters and lock the tuner
	// 请设置Tuner参数，并锁定Tuner
	HDIC2501_SetTuner(Frequency);
	//------------------------------------------

	//------------------------------------------
	// Please set the demod parameters, please check the HDIC2501.h to get the Setting Value
	// 请设置Demod参数，设置的参数请查询HDIC2501.h
	// Example:
	// 自动,SymbolRate未使用,芯片可以自动侦测符号率
	// HDIC2501_SetParameters(SYMBOLRATEAUTODETECT_AUTO,0);
	// 手动,SymbolRate传入,如知道符号率,用手动模式会加快锁频过程.
	// HDIC2501_SetParameters(SYMBOLRATEAUTODETECT_MANUAL,28800000);
	HDIC2501_SetParameters(SYMBOLRATEAUTODETECT_MANUAL,28800000);

	//------------------------------------------
	HDIC2501_SoftReset();

	for (CheckLockTemp = 0; CheckLockTemp < 40; CheckLockTemp++)
	{
		HDIC2501_IsDemodLocked(&locked, &Status);

		HDIC2501_Wait(100); /* wait 100 ms */

		if (locked == 1)
		{
			// If necessery, invoke the HDIC2501_GetMode function to get the information
			// 此时你可以读出信号具体格式,符号率等
			// HDIC2501_GetMode(&Constellation, &CodingRate, &ScramblerMode, &TSorGS, &RollOff, &OperationMode, &TS_ID);
			// HDIC2501_GetSymbolRate(&SymbolRate);
			// 根据读出的符号率和滚降系数,可以去计算出新Tuner的SAW的配置.
			// SAW = HDIC2501_GetNewSAW(SymbolRate, Rolloff);
			HDIC_Print(("---> [2501] 2501 is locked \n"));
			break;
		}
	}
}



