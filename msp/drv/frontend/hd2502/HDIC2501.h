/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#pragma once

#include <linux/printk.h>

#include "mt_type.h" //flax

#include "HDICType.h"

#include "RDA5812.h"
#include "RDA5815M.h"
#include "ISharpTuner.h"
#include "mt_fe_tn_montage_ts6011.h"


#define HDIC2501_I2C_ADDRESS				0xEE

//for IsAuto Select
#define ISAUTO_YES							0x0F
#define ISAUTO_NO							0x00

//for SymbolRateAutoDetect Select
#define SYMBOLRATEAUTODETECT_AUTO			0x30
#define SYMBOLRATEAUTODETECT_MANUAL			0x00

//for Constellation Select
#define CONSTELLATION_MASK					0x03

#define CONSTELLATION_QPSK					0x00
#define CONSTELLATION_8PSK					0x01

//for CodingRate Select
#define CODINGRATE_MASK						0x3C

#define CODINGRATE_1_2						0x08
#define CODINGRATE_3_5						0x0C
#define CODINGRATE_2_3						0x10
#define CODINGRATE_3_4						0x14
#define CODINGRATE_4_5						0x18
#define CODINGRATE_5_6						0x1C
#define CODINGRATE_13_15					0x20
#define CODINGRATE_9_10						0x24

//for ScramblerMode Select
//#define SCRAMBERMODE_MASK					0x30
#define SCRAMBERMODE_MASK					0xC0

#define SCRAMBERMODE_MODE0					0x00
#define SCRAMBERMODE_MODE1					0x40
#define SCRAMBERMODE_MODE2					0x80
#define SCRAMBERMODE_MODE3					0xC0

//for Pilot Select
#define PILOT_0								0x00
#define PILOT_32							0x01
#define PILOT_64							0x02

//for RollOff Select
#define ROLLOFF_MASK						0x30

#define ROLLOFF_0_35						0x00//READ: 0x00
#define ROLLOFF_0_25						0x04//READ: 0x10
#define ROLLOFF_0_20						0x08//READ: 0x20
//READ ROLLOFF_RESERVED : 0x30
 
//for OperationMode Select
#define OPERATIONSMODE_MASK					0x40

#define OPERATIONSMODE_CCM					0x00
#define OPERATIONSMODE_VCM					0x10
#define OPERATIONSMODE_ACM					0x20	

//for TS Edge Select
#define TSEDGE_POSEDGE						0x01
#define TSEDGE_NEGEDGE						0x00

//for TS Serial Select
#define TS_PARALLEL							0x02
#define TS_SERIAL							0x00

//for TS NullPacket Select
#define NULLPACKET_ENABLED					0x08   
#define NULLPACKET_DELETED					0x00

//for TS TSorGS Select
#define TSORGS_MASK							0x80

#define TSORGS_TS							0x00 //Transport Stream
#define TSORGS_GS							0x80 //Gernal Stream

INT8 HDIC2501_InitReg(void);
INT8 HDIC2501_IsDemodLocked(UINT8 *locked, UINT8 *Status);
INT8 HDIC2501_SetTsFormat(UINT8 Edge, UINT8 Serial, UINT8 NullPacket);
INT8 HDIC2501_SetSymbolRate(UINT32 SymbolRate);
INT8 HDIC2501_SetParameters(UINT8 SymbolRateAutoDetect, UINT32 SymbolRate_Hz);
INT8 HDIC2501_GetMode(UINT8 *Constellation, UINT8 *CodingRate, UINT8 *ScramblerMode, UINT8 *TSorGS, UINT8 *RollOff, UINT8 *OperationMode, UINT8 *TS_ID);
INT8 HDIC2501_GetSymbolRate(UINT32 *SymbolRate);
#if 1
INT8 HDIC2501_GetSignalSNR(UINT32 *SignalSNR);
INT8 HDIC2501_GetSignalBER(UINT32 *pSignalBER);
INT8 HDIC2501_GetLdpcBER(UINT16 *pLdpcBER);
#else
INT8 HDIC2501_GetSignalSNR(double *SignalSNR);
INT8 HDIC2501_GetSignalBER(double *pSignalBER);
#endif
#if 0
INT8 HDIC2501_GetLdpcBER(double *pLdpcBER);
#endif
INT8 HDIC2501_GetFieldStrength(UINT16 *FieldStrength);
INT8 HDIC2501_SoftReset(void);
UINT8 HDIC2501_SetPolar(UINT8 Polar);
INT8 HDIC2501_OpenTunerI2C(UINT8 Address);
INT8 HDIC2501_CloseTunerI2C(void);
INT8 HDIC2501_SetTuner(UINT32 Frequency);
INT16 HDIC2501_GetNewSAW(UINT32 SymbolRate, UINT8 Rolloff);
INT8 HDIC2501_WriteRegister(UINT8 Register, UINT8 Data);
INT8 HDIC2501_ReadRegister(UINT8 Register, UINT8 *Data);
void HDIC2501_Wait(UINT16 millisecond);
INT8 HDIC2501_PLLReset(void);
INT8 HDIC2501_HWReset(int tuner_id);

void HDIC2501_Init(int tuner_id);

void Pio_Init(UINT8 gpio_pin);


