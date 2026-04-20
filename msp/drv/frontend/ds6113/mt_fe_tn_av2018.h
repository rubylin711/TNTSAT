/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifndef _AV2018_DS6113_H
#define _AV2018_DS6113_H

#ifdef _CPLUSPLUS
extern "C" {
#endif

/***Data Types**********************************************************************/
typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;

/***enumerator structures************************************************************/
typedef enum
{							// I2C Write Address setting by Hardware pins
	ADDR0_L = 0xC4, // Pin7 ADDR0 = GND
	ADDR0_H = 0xC6	// Pin7 ADDR0 = OPEN
}
TUNER_I2C_ADDRESS;

typedef enum
{                      // For Tuner model name =1 only
	Differential = 0,  // IQ Differential mode
	Single = 1         // IQ Single end mode, only output at RXIP, RXQP
}
TUNER_IQ_MODE;

typedef enum          // BaseBand programmable amplifier gain setting
{
	PGA_0dB     =0,
	PGA_1_5dB   =1,
	PGA_3dB     =2,
	PGA_4_5dB   =3,
	PGA_6dB     =4,
	PGA_7_5dB   =5,
	PGA_9dB     =6,
	PGA_10_5dB  =7,
	PGA_12dB    =8
}
TUNER_PGA_GAIN;

typedef enum		 // BaseBand programmable amplifier output driving current setting
{
	PGA_500uA =0,
	PGA_1mA   =1,
	PGA_1_5mA =2,
	PGA_2mA   =3
}
TUNER_PGA_DRIVE_CURRENT;

typedef enum
{
	LOW       =0,
	MEDIUM    =1,
	HIGH      =2,
	MAXIMUM   =3
}
TUNER_XO_DRIVE_CURRENT;

typedef enum          // Crystal oscillator ON/OFF for stand-by
{
	XO_OFF = 0,
	XO_ON = 1
}
TUNER_XO_ENABLE;

typedef enum         // RF loop through ON/OFF
{
	RFLP_OFF = 0,
	RFLP_ON = 1
}
TUNER_RFLP_ENABLE;

typedef enum        // power down setting for stand-by
{
	Wake_Up = 0,    //0 = Wake up Tuner
	Power_Down = 1  //1 = Power down Tuner
}
TUNER_SOFTWARE_POWERDOWN;

typedef enum            // Fine-tune function setting
{
	FT_OFF      = 0x00,	// FT_block=0, FT_EN=0, FT_hold=0. The Gain is at best sensitivity gain.
	FT_ON       = 0x02,	// FT_block=0, FT_EN=1, FT_hold=0. Turn on to fine-tuned gain continuously between sensitivity and linearity.
	FT_Hold     = 0x03, // FT_block=0, FT_EN=1, FT_hold=1. Stop fine-tuning the gain and hold the current fine-tuned gain
	FT_Delay_ON = 0x06	// FT_block=1, FT_EN=1, FT_hold=0. Turn on Fine-tune Function after 12ms delay
}
TUNER_FINE_TUNE;

typedef enum            // Error code of tuner at different error status
{
	Tuner_No_Error    =0,
	Tuner_Error		  =1,
	PLL_Lock_Error    =3,
	Filter_Lock_Error =5,
	I2C_Error         =9
}
TUNER_ErrorCode;


/***tuner parameter structures************************************************************/
typedef struct
{
	/** Tuner Hardware variables**/
	UINT32                   crystal_khz;	 // (Unit:KHz) Tuner_crystal supporting range: 13000~37000KHz

	TUNER_I2C_ADDRESS        I2C_ADDR;		 // Tuner I2C address at write mode

	/** Tuner Register varaibles**/
	TUNER_IQ_MODE            IQmode;		 // RXIQ differential out or Single out
	TUNER_PGA_GAIN           PGA_Gain;       // RXout BaseBand Programmable gain
	TUNER_PGA_DRIVE_CURRENT  PGA_Current;    // RXout BaseBand PGA output driving current
	TUNER_XO_DRIVE_CURRENT   XO_Current;     // Crystal oscillator driving current
	TUNER_XO_ENABLE          XO_EN;			 // Crystal oscillator ON/OFF
	TUNER_RFLP_ENABLE        RFLP_EN;		 // Loopthrough ON/OFF
	TUNER_SOFTWARE_POWERDOWN PD_EN;          // Tuner software power down ON/OFF
	TUNER_FINE_TUNE          FT;	         // Front-end Gain fine tune between linearity and sensitvity

	UINT8			         blind_scan;     // System Blind Scan indication for fine-tune function
}
TunerPara, *pTunerPara;


/***Tuner Control Functions********************************************************************/
#if 0
static TUNER_ErrorCode Tuner_Parameter_Initial (pTunerPara pAVtuner);
static TUNER_ErrorCode Tuner_Register_Initial (pTunerPara pAVtuner);
#endif

#if 0
static TUNER_ErrorCode Tuner_Set_Channel_Frequency(pTunerPara pAVtuner, UINT32 channel_freq_mhz);
#else
static TUNER_ErrorCode Tuner_Set_Channel_Frequency(pTunerPara pAVtuner, UINT32 channel_freq_khz, UINT32 bb_sym);
#endif

#if 0
static TUNER_ErrorCode Tuner_Set_Filter_Bandwith (pTunerPara pAVtuner, UINT32 filter_BW_khz);
static TUNER_ErrorCode Tuner_Set_RFLP_PD (pTunerPara pAVtuner);
static TUNER_ErrorCode Tuner_Set_Fine_Tune (pTunerPara pAVtuner);
static TUNER_ErrorCode Tuner_Set_RXout_PGA (pTunerPara pAVtuner);
static TUNER_ErrorCode Tuner_Get_Channel_Lock (pTunerPara pAVtuner);
static TUNER_ErrorCode Tuner_Get_Filter_Lock (pTunerPara pAVtuner);
#endif

/***Customer define Functions********************************************************************/
static TUNER_ErrorCode Tuner_I2C_Write (pTunerPara pAVtuner, UINT8 reg_start, UINT8* buff, UINT8 length);
//static TUNER_ErrorCode Tuner_I2C_Read (pTunerPara pAVtuner, UINT8 reg_start, UINT8* buff, UINT8 length);
static TUNER_ErrorCode Time_DELAY_MS (UINT32 ms);

#ifdef _CPLUSPLUS
}
#endif

#endif //_AV2018_DS6113_H

