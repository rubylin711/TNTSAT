/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "S305.h"
#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

TP_UINT8 S305_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{
	/*
		S305 Register Control Program
		Version: 3.01 for telepath
		Data: 2011-5-24
		Main Feature:
		1. AFC enable; I2C is sent twice
		2. LNA bypass Disable
	*/
	TP_UINT32 m_LOFreq;
	//TP_FLOAT ratio;
	//TP_FLOAT Fref, Fpart;
	TP_INT32 ratio;
	TP_INT32 Fpart;
	TP_INT32 Ipart;
	TP_UINT8 reg_data[20];
	TP_UINT32 m_RegN, m_RegF, m_VCODiv, m_VCOSel;
	//TP_FLOAT m_FltBW;
	TP_INT32 m_FltBW;
	TP_UINT32 m_RegM, m_RegD, m_RegR, m_BBVGA;
	TP_UINT32 m_XtalFreq = 27; // Crystal frequency, 27MHz default

	reg_data[0] = 0x00;	 // data address
	reg_data[1] = 0x68;	 // chip_enable loop_disable| BBVGA
	reg_data[2] = 0x00;	 // BBfilter D
	reg_data[3] = 0x8D;	 // BBfilter M
	reg_data[4] = 0x5C;	 // AFC_eable
	reg_data[5] = 0x10;	 // VCO_div=3 | Ref_div=1
	reg_data[6] = 0x85;	 // CP=1.2mA  | LNA_bps=disable
	reg_data[7] = 0x99;	 // Xtalbuf=1 | Xtal_current=1
	reg_data[8] = 0x02;	 // N[10:4]
	reg_data[9] = 0x75;	 // N[3:0] | VCO_Sel
	reg_data[10] = 0x00; // F
	reg_data[11] = 0x00; // F

	m_RegR = 1;	 // Ref div = 1 default
	m_BBVGA = 8; // BBVGA Gain = 8 default

	m_LOFreq = frequency; // m_LOFreq is LO frequency we wanted to set
	if (m_LOFreq > 1442)
	{				  // VCO selection
		m_VCODiv = 0; // //divide by 2
		if (m_LOFreq > 2077)
			m_VCOSel = 14;
		else if (m_LOFreq > 2003)
			m_VCOSel = 13;
		else if (m_LOFreq > 1931)
			m_VCOSel = 12;
		else if (m_LOFreq > 1862)
			m_VCOSel = 11;
		else if (m_LOFreq > 1807)
			m_VCOSel = 10;
		else if (m_LOFreq > 1757)
			m_VCOSel = 9;
		else if (m_LOFreq > 1720)
			m_VCOSel = 8;
		else if (m_LOFreq > 1681)
			m_VCOSel = 7;
		else if (m_LOFreq > 1642)
			m_VCOSel = 6;
		else if (m_LOFreq > 1606)
			m_VCOSel = 5;
		else if (m_LOFreq > 1567)
			m_VCOSel = 4;
		else if (m_LOFreq > 1532)
			m_VCOSel = 3;
		else if (m_LOFreq > 1500)
			m_VCOSel = 2;
		else if (m_LOFreq > 1472)
			m_VCOSel = 1;
		else
			m_VCOSel = 0;
	}
	else if (m_LOFreq > 1021)
	{
		m_VCODiv = 1; // divide by 3
		if (m_LOFreq > 1385)
			m_VCOSel = 14;
		else if (m_LOFreq > 1335)
			m_VCOSel = 13;
		else if (m_LOFreq > 1287)
			m_VCOSel = 12;
		else if (m_LOFreq > 1241)
			m_VCOSel = 11;
		else if (m_LOFreq > 1205)
			m_VCOSel = 10;
		else if (m_LOFreq > 1171)
			m_VCOSel = 9;
		else if (m_LOFreq > 1147)
			m_VCOSel = 8;
		else if (m_LOFreq > 1121)
			m_VCOSel = 7;
		else if (m_LOFreq > 1095)
			m_VCOSel = 6;
		else if (m_LOFreq > 1071)
			m_VCOSel = 5;
		else if (m_LOFreq > 1045)
			m_VCOSel = 4;
		else
			m_VCOSel = 3;
	}
	else
	{
		m_VCODiv = 2; // divide by 4
		if (m_LOFreq > 1002)
			m_VCOSel = 13;
		else if (m_LOFreq > 966)
			m_VCOSel = 12;
		else
			m_VCOSel = 11;
	}

	// Calculate RegN and RegF
	//Fref = (TP_FLOAT)m_XtalFreq / m_RegR;
	//ratio = m_LOFreq / Fref;
	//Ipart = (TP_INT32)ratio;
	//Fpart = ratio - Ipart;
	//if (Fpart >= 0.5)
	//{
	//	m_RegN = Ipart;
	//	Fpart -= 0.5;
	//}
	//else
	//{
	//	m_RegN = Ipart - 1;
	//	Fpart += 0.5;
	//}
	//m_RegF = (TP_INT32)(Fpart * 65536);

	ratio = m_LOFreq * m_RegR / m_XtalFreq;
	Ipart = ratio;
	Fpart = (m_LOFreq * m_RegR) % m_XtalFreq;

	if (Fpart >= (m_XtalFreq / 2))
	{
		m_RegN = Ipart;
		Fpart -= (m_XtalFreq / 2);
	}
	else
	{
		m_RegN = Ipart - 1;
		Fpart += (m_XtalFreq / 2);
	}

	m_RegF = (Fpart << 16) / m_XtalFreq;


	// convert m_RegR value to Register bits
	if (m_RegR == 4)
		m_RegR -= 1;
	m_RegR -= 1;

	reg_data[5] = ((m_VCODiv << 4) & 0x70) + m_RegR;
	reg_data[8] = (m_RegN >> 4) & 0x7F;
	reg_data[9] = ((m_RegN << 4) & 0xF0) | (m_VCOSel & 0x0F);
	reg_data[10] = ((m_RegF >> 8) & 0xFF);
	reg_data[11] = m_RegF & 0xFF;

	// set baseband filter bandwidth
	Symbol_Rate_Value *= 5;
	Symbol_Rate_Value /= 4;

	Symbol_Rate_Value /= 2;
	Symbol_Rate_Value /= 1000000;

	// m_FltBW = Symbol_Rate_Value * 1.2;		//20% bandwidth margin

#if 0
	m_FltBW = (TP_FLOAT)1.2; // 20% bandwidth margin
	m_FltBW *= Symbol_Rate_Value;

	if (m_FltBW > 45.0)
		m_FltBW = 45.0; // the maximum filter 3dB bandwidth is 45MHz
	if (m_FltBW < 4.0)
		m_FltBW = 4.0; // the minimum filter 3dB bandwidth is 4MHz
#else
	m_FltBW = Symbol_Rate_Value * 12 / 10;

	if (m_FltBW > 45)
		m_FltBW = 45; // the maximum filter 3dB bandwidth is 45MHz
	if (m_FltBW < 4)
		m_FltBW = 4; // the minimum filter 3dB bandwidth is 4MHz
#endif

	m_RegM = (int)(m_XtalFreq / 2);
	//m_RegD = (int)((m_FltBW * 2 * m_RegM / m_XtalFreq - 4) / 0.345);
	m_RegD = (int)((m_FltBW * 2 * m_RegM / m_XtalFreq - 4) * 1000 / 345);
	if ((char)m_RegD < 0)
		m_RegD = 0;
	if (m_RegD > 127)
		m_RegD = 127;

	reg_data[1] = 0x60 | (m_BBVGA & 0x0F);
	reg_data[2] = (m_RegD & 0x7F);
	reg_data[3] = 0x80 | (m_RegM & 0x1F);

	// Send Data to I2C
	TP_iic_tuner_write(S305_DEV_ADDR, reg_data, 12);
	TP_Delay(50);		//
	reg_data[4] = 0x5D; // toggle the AFC indication bit
	TP_iic_tuner_write(S305_DEV_ADDR, reg_data, 12);

	return TP_SUCCESS;
}

TP_UINT8 S305_init()
{
	return TP_SUCCESS;
}

#endif // _USE_TP5001_CHIP_

