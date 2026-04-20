/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

//#include "stdafx.h"
#include "ISharpTuner.h"

extern void HD_Delay(UINT32 uiMS);
extern UINT8 Write_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);
extern UINT8 Read_I2C(UINT8 device_address, UINT16 length, UINT8 *value_buffer);

UINT8 Sharp_ITuner_GetStatus(SharpTuner *Tuner)
{
	UINT8 data;

	Read_I2C(Tuner->ucSlaveAddr, 1, &data);

	return data;
}

UINT8 Sharp_ITuner_CommitSetting(SharpTuner *Tuner)
{
	//UINT8 data;

	Tuner->ucRegData[0] &= 0x7f;
	Tuner->ucRegData[2] |= 0x80;

	Tuner->ucRegData[2] &= ~(0x7 << 2);
	Tuner->ucRegData[3] &= ~(0x3 << 2);

	Write_I2C(Tuner->ucSlaveAddr, 4, Tuner->ucRegData);

	Tuner->ucRegData[2] |= (0x1 << 2);
	Write_I2C(Tuner->ucSlaveAddr, 1, (Tuner->ucRegData) + 2);
	HD_Delay(12);

	Tuner->ucRegData[2] |= ((((Tuner->ucLPF) >> 1) & 0x1) << 3); /* PD4 */
	Tuner->ucRegData[2] |= ((((Tuner->ucLPF) >> 0) & 0x1) << 4); /* PD5 */
	Tuner->ucRegData[3] |= ((((Tuner->ucLPF) >> 3) & 0x1) << 2); /* PD2 */
	Tuner->ucRegData[3] |= ((((Tuner->ucLPF) >> 2) & 0x1) << 3); /* PD3 */
	Write_I2C(Tuner->ucSlaveAddr, 2, (Tuner->ucRegData) + 2);

	return 0;
}

UINT8 Sharp_ITuner_SetBBGain(Tuner_BBGain_Sharp BBGain, SharpTuner *Tuner)
{
	Tuner->ucRegData[0] &= ~(0x3 << 5);
	Tuner->ucRegData[0] |= (BBGain << 5);

	return 0;
}

UINT8 Sharp_ITuner_SetChargePump(Tuner_PumpCurrent_Sharp Current, SharpTuner *Tuner)
{
	Tuner->ucRegData[2] &= ~(0x3 << 5);
	Tuner->ucRegData[2] |= (Current << 5);

	return 0;
}

UINT8 Sharp_ITuner_SetFrequency(UINT16 uiFrequency_100kHz, SharpTuner *Tuner)
{
	UINT16 P, N, A, DIV;

	if (uiFrequency_100kHz < 9500)
	{
		return 1;
	}
	else if (uiFrequency_100kHz < 9860)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x5 << 5);
		P = 16;
		DIV = 1;
	}
	else if (uiFrequency_100kHz < 10730)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x6 << 5);
		P = 16;
		DIV = 1;
	}
	else if (uiFrequency_100kHz < 11540)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x7 << 5);
		P = 32;
		DIV = 1;
	}
	else if (uiFrequency_100kHz < 12910)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x1 << 5);
		P = 32;
		DIV = 0;
	}
	else if (uiFrequency_100kHz < 14470)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x2 << 5);
		P = 32;
		DIV = 0;
	}
	else if (uiFrequency_100kHz < 16150)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x3 << 5);
		P = 32;
		DIV = 0;
	}
	else if (uiFrequency_100kHz < 17910)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x4 << 5);
		P = 32;
		DIV = 0;
	}
	else if (uiFrequency_100kHz < 19720)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x5 << 5);
		P = 32;
		DIV = 0;
	}
	else if (uiFrequency_100kHz < 21500)
	{
		Tuner->ucRegData[3] &= ~(0x7 << 5);
		Tuner->ucRegData[3] |= (0x6 << 5);
		P = 32;
		DIV = 0;
	}
	else
	{
		return 1;
	}

	A = (uiFrequency_100kHz / 10) % P;
	N = (uiFrequency_100kHz / 10) / P;

	Tuner->ucRegData[3] &= ~(0x1 << 4);
	if (P == 16)
	{
		Tuner->ucRegData[3] |= (0x1 << 4);
	}

	Tuner->ucRegData[3] &= ~(0x1 << 1);
	Tuner->ucRegData[3] |= (DIV << 1);

	Tuner->ucRegData[1] &= ~(0x1f << 0);
	Tuner->ucRegData[1] |= (A << 0);

	Tuner->ucRegData[1] &= ~(0x7 << 5);
	Tuner->ucRegData[1] |= (N << 5);
	Tuner->ucRegData[0] &= ~(0x1f << 0);
	Tuner->ucRegData[0] |= ((N >> 3) << 0);

	return 0;
}

UINT8 Sharp_ITuner_SetLPF(UINT16 uiLPF_10kHz, SharpTuner *Tuner)
{
	uiLPF_10kHz /= 100;

	if (uiLPF_10kHz < 5)
	{
		uiLPF_10kHz = 5;
	}

	if (uiLPF_10kHz > 34)
	{
		uiLPF_10kHz = 34;
	}

	Tuner->ucLPF = (uiLPF_10kHz - 10) / 2 + 3;

	return 0;
}

UINT16 Sharp_ITuner_CalculateLPF(UINT16 uiSymbolRate_10kHz)
{
	UINT32 lpf = uiSymbolRate_10kHz;

	lpf *= 81;
	lpf /= 100;
	lpf += 500;

	return ((UINT16)lpf);
}

UINT8 Sharp_ITuner_GetLockStatus(UINT8 ucTunerSlaveAddr)
{
	SharpTuner Tuner;

	Tuner.ucSlaveAddr = ucTunerSlaveAddr;

	return (Sharp_ITuner_GetStatus(&Tuner));
}

UINT8 Sharp_ITuner_Lock(UINT8 ucTunerSlaveAddr, UINT16 uiFrequency_100kHz, UINT16 uiLPF_10kHz, void *pParameters)
{
	SharpTuner Tuner;
	SharpTunerPara *pPara;

	Tuner.ucSlaveAddr = ucTunerSlaveAddr;
	Tuner.ucRegData[0] = Tuner.ucRegData[1] = Tuner.ucRegData[2] = Tuner.ucRegData[3] = 0;
	Sharp_ITuner_SetLPF(uiLPF_10kHz, &Tuner);
	Sharp_ITuner_SetFrequency(uiFrequency_100kHz, &Tuner);
	if (NULL == pParameters)
	{ /*use default values*/
		Sharp_ITuner_SetChargePump(PC_360_694_Sharp, &Tuner);
		Sharp_ITuner_SetBBGain(Bbg_2_Sharp, &Tuner);
	}
	else
	{ /*use custom value*/
		pPara = (SharpTunerPara *)pParameters;
		Sharp_ITuner_SetChargePump(pPara->ChargPump, &Tuner);
		Sharp_ITuner_SetBBGain(pPara->BBGain, &Tuner);
	}

	return (Sharp_ITuner_CommitSetting(&Tuner));
}

