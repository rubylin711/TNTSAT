/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

/*
 */
#ifndef ISharpTuner_h_h
#define ISharpTuner_h_h

#include "HDIC2501.h"

#define ISHARP_TUNER_ADDR 0xC0

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum tag_Tuner_BBGain_Sharp
{
	Bbg_0_Sharp,
	Bbg_1_Sharp,
	Bbg_2_Sharp,
	Bbg_4_Sharp
} Tuner_BBGain_Sharp;

typedef enum tag_Tuner_PumpCurrent_Sharp
{
	PC_78_150_Sharp,
	PC_169_325_Sharp,
	PC_360_694_Sharp,
	PC_780_1500_Sharp
} Tuner_PumpCurrent_Sharp;

typedef struct tag_SharpTuner
{
	UCHAR ucLPF;
	UCHAR ucRegData[4];
	UCHAR ucSlaveAddr;
} SharpTuner;

typedef struct tag_SharpTunerPara
{
	Tuner_PumpCurrent_Sharp ChargPump;
	Tuner_BBGain_Sharp      BBGain;
} SharpTunerPara;

UCHAR Sharp_ITuner_GetStatus(SharpTuner *Tuner);

UCHAR Sharp_ITuner_CommitSetting(SharpTuner *Tuner);

UCHAR Sharp_ITuner_SetBBGain(Tuner_BBGain_Sharp BBGain, SharpTuner *Tuner);

UCHAR Sharp_ITuner_SetChargePump(Tuner_PumpCurrent_Sharp Current, SharpTuner *Tuner);

UCHAR Sharp_ITuner_SetFrequency(USHORT uiFrequency_100kHz, SharpTuner *Tuner);

UCHAR Sharp_ITuner_SetLPF(USHORT uiLPF_10kHz, SharpTuner *Tuner);

USHORT Sharp_ITuner_CalculateLPF(USHORT uiSymbolRate_10kHz);

UCHAR Sharp_ITuner_GetLockStatus(UCHAR ucTunerSlaveAddr);

UCHAR Sharp_ITuner_Lock(UCHAR ucTunerSlaveAddr, USHORT uiFrequency_100kHz, USHORT uiLPF_10kHz, void *pParameters);
#ifdef __cplusplus
}
#endif
#endif
