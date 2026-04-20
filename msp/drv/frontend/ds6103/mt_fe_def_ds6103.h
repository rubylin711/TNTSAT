/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2019 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/*****************************************************************************
 *----------------------------------------------------------------------------
 *
 * File:                 mt_fe_def.h
 *
 * Current version:      2.01.13
 *
 * Description:
 *
 * Log:  Description             Version     Date            Author
 *----------------------------------------------------------------------------
 *       Create                  2.00.00     2017.03.12      YZ.Huang
 *       Modify                  2.00.09     2017.04.20      YZ.Huang
 *       Modify                  2.01.03     2017.10.26      YZ.Huang
 *       Modify                  2.01.05     2017.11.16      YZ.Huang
 *       Modify                  2.01.07     2017.12.13      YZ.Huang
 *       Modify                  2.01.09     2018.10.16      YZ.Huang
 *       Modify                  2.01.13     2019.07.02      YZ.Huang
 *****************************************************************************/
#ifndef __MT_FE_DEF_H__
#define __MT_FE_DEF_H__


#ifdef __cplusplus
extern "C" {
#endif

//#include <stdio.h>

#include "mt_fe_common_ds6103.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x)         ((void)(x))
#endif



typedef enum _MT_FE_LNB_VOLTAGE
{
    MtFeLNB_13V = 0
    ,MtFeLNB_18V
} MT_FE_LNB_VOLTAGE;


typedef enum _MT_FE_CODE_RATE
{
    MtFeCodeRate_Undef = 0
    ,MtFeCodeRate_1_4
    ,MtFeCodeRate_1_3
    ,MtFeCodeRate_2_5
    ,MtFeCodeRate_1_2
    ,MtFeCodeRate_3_5
    ,MtFeCodeRate_2_3
    ,MtFeCodeRate_3_4
    ,MtFeCodeRate_4_5
    ,MtFeCodeRate_5_6
    ,MtFeCodeRate_7_8
    ,MtFeCodeRate_8_9
    ,MtFeCodeRate_9_10
    //,MtFeCodeRate_1_2L
    //,MtFeCodeRate_3_5L
    //,MtFeCodeRate_2_3L
    ,MtFeCodeRate_5_9
    //,MtFeCodeRate_5_9L
    ,MtFeCodeRate_7_9
    ,MtFeCodeRate_4_15
    ,MtFeCodeRate_7_15
    ,MtFeCodeRate_8_15
    //,MtFeCodeRate_8_15L
    ,MtFeCodeRate_11_15
    //,MtFeCodeRate_11_15L
    ,MtFeCodeRate_13_18
    ,MtFeCodeRate_9_20
    ,MtFeCodeRate_11_20
    ,MtFeCodeRate_23_36
    ,MtFeCodeRate_25_36
    ,MtFeCodeRate_11_45
    ,MtFeCodeRate_13_45
    ,MtFeCodeRate_14_45
    ,MtFeCodeRate_26_45
    //,MtFeCodeRate_26_45L
    ,MtFeCodeRate_28_45
    ,MtFeCodeRate_29_45
    //,MtFeCodeRate_29_45L
    ,MtFeCodeRate_31_45
    //,MtFeCodeRate_31_45L
    ,MtFeCodeRate_32_45
    //,MtFeCodeRate_32_45L
    ,MtFeCodeRate_77_90
} MT_FE_CODE_RATE;

typedef enum _MT_FE_ROLL_OFF
{
    MtFeRollOff_Undef = 0
    ,MtFeRollOff_0p35
    ,MtFeRollOff_0p25
    ,MtFeRollOff_0p20
    ,MtFeRollOff_0p15
    ,MtFeRollOff_0p10
    ,MtFeRollOff_0p05
} MT_FE_ROLL_OFF;


typedef enum _MT_FE_SPECTRUM_MODE
{
    MtFeSpectrum_Undef = 0
    ,MtFeSpectrum_Normal
    ,MtFeSpectrum_Inversion
} MT_FE_SPECTRUM_MODE;


typedef enum _MT_FE_LNB_LOCAL_OSC
{
    MtFeLNB_Single_Local_OSC = 0
    ,MtFeLNB_Dual_Local_OSC
} MT_FE_LNB_LOCAL_OSC;


typedef enum _MT_FE_BAND_TYPE
{
    MtFeBand_C = 0
    ,MtFeBand_Ku
} MT_FE_BAND_TYPE;


typedef enum _MT_FE_MSG
{
    MtFeMsg_BSTpFind = 0
    ,MtFeMsg_BSTpLocked
    ,MtFeMsg_BSTpUnlock
    ,MtFeMsg_BSStart
    ,MtFeMsg_BSFinish
    ,MtFeMsg_BSOneWinFinish
    ,MtFeMsg_BSAbort
    ,MtFeMsg_CCMFind
} MT_FE_MSG;

typedef struct _MT_FE_CHAN_INFO_DVBS2
{
    U8                  iPlsCode;
    MT_FE_TYPE          type;
    MT_FE_MOD_MODE      mod_mode;
    MT_FE_ROLL_OFF      roll_off;
    MT_FE_CODE_RATE     code_rate;
    MT_FE_BOOL          is_pilot_on;
    MT_FE_SPECTRUM_MODE is_spectrum_inv;
    MT_FE_BOOL          is_dummy_frame;
    S8                  iVcmCycle;
    S8                  iFrameLength;		/*0: Normal; 1: Short; 2: Medium*/
} MT_FE_CHAN_INFO_DVBS2;

typedef enum _MT_FE_DiSEqC_TONE_BURST
{
    MtFeDiSEqCToneBurst_Moulated = 0
    ,MtFeDiSEqCToneBurst_Unmoulated
} MT_FE_DiSEqC_TONE_BURST;


typedef struct _MT_FE_DiSEqC_MSG
{
    U8      data_send[8];
    U8      size_send;
    MT_BOOL    is_enable_receive;
    MT_BOOL    is_envelop_mode;
    U8      data_receive[8];
    U8      size_receive;
} MT_FE_DiSEqC_MSG;

typedef enum _MT_FE_DMD_ID
{
    MtFeDmdId_Undef,
    MtFeDmdId_DS300X,
    MtFeDmdId_DS3002B,
    MtFeDmdId_DS3103,
    MtFeDmdId_DS3103B,
    MtFeDmdId_DS3103C,
    MtFeDmdId_DS6103,
    MtFeDmdId_RS6000,
    MtFeDmdId_RS6060,
    MtFeDmdId_RS6000B,
    MtFeDmdId_Unknown
} MT_FE_DMD_ID;

typedef struct _MT_FE_TP_INFO
{
    U32  freq_KHz;
    U32  sym_rate_KSs;
    MT_FE_TYPE dvb_type;
    MT_FE_CODE_RATE code_rate;
    MT_BOOL bCheckCCM;
    S8   iTsCnt;
    U8   ucTsId[32];
    U8   ucCurTsId;
    MT_BOOL bHavePLS;
    U8   ucPLSCode[3];
    U32  uidx_notch_last;
    U32  uidx_up;
    U32  uidx_notch;
    U32  upsd_sn;
}MT_FE_TP_INFO;

typedef struct _MT_FE_BS_TP_INFO
{
    U8      bs_algorithm;
    U16     tp_num;
    U16     tp_max_num;
    MT_FE_TP_INFO *p_tp_info;
} MT_FE_BS_TP_INFO;

typedef enum _MT_FE_TS_DRIVER_ABILITY
{
    MtFeTsDriverAbility_4mA  = 0,
    MtFeTsDriverAbility_8mA  = 1,
    MtFeTsDriverAbility_12mA = 2,
    MtFeTsDriverAbility_16mA = 3
} MT_FE_TS_DRIVER_ABILITY;

typedef enum _MT_FE_TN_SUPPORTED_TUNER
{
    MtFeTn_Undef             = 0,
    MtFeTn_Montage_RS6000    = 1,
    MtFeTn_Montage_TS2022    = 2,
    MtFeTn_Montage_TS6011    = 3,
    MtFeTn_RDA_5815S         = 4,
    MtFeTn_RDA_5815M         = 5,
    MtFeTn_AV2018            = 6,
    MtFeTn_AV2012            = 7,
    MtFeTn_Unsupported       = -1
} MT_FE_TN_SUPPORTED_TUNER;


typedef struct _MT_FE_BOARD_SETTINGS_DS6103
{
	MT_BOOL		bIQInverted;
	MT_BOOL		bAGCPolar;
	MT_BOOL		bSpectrumInverted;

	MT_BOOL		bPolarLNBEnable;			// LNB_ENABLE_WHEN_LNB_EN_HIGH  0
	MT_BOOL		bPolarLNB13V;				// LNB_13V_WHEN_VSEL_HIGH       1
	MT_BOOL		bPolarLNBStandby;			// LNB_VSEL_STANDBY_HIGH        1
	MT_BOOL		bPolarDiSEqCOut;			// LNB_DISEQC_OUT_FORCE_HIGH    0
	MT_BOOL		bDiSEqCOutputOnly;			// LNB_DISEQC_OUT_ONLY_OUTPUT   0
} MT_FE_BOARD_SETTINGS_DS6103;


typedef struct _MT_FE_TS_SETTINGS_DS6103
{
	MT_FE_TS_OUT_MODE		mTsMode;
	MT_BOOL					bHighZ;
	MT_BOOL					bPinSwitch;
	MT_FE_TS_DRIVER_ABILITY	mPinDriver;
	MT_FE_TS_OUT_MAX_CLOCK	mMaxClock;
	U8						iCustomDutyHigh;
	U8						iCustomDutyLow;
	U8						iSerialDriverMode;
	MT_BOOL					bAutoSerial;
	MT_BOOL					bAutoParallel;
	MT_BOOL					bAutoCI;
	MT_BOOL					bRisingEdge;
	MT_BOOL					bHighSync;
	MT_BOOL					bHighValid;
	MT_BOOL					bHighError;
	MT_BOOL					b3SerialTsOut;
	MT_BOOL					bParallelClkFixed;
} MT_FE_TS_SETTINGS_DS6103;

typedef struct _MT_FE_LNB_SETTINGS_DS6103
{
	MT_BOOL					bLnbOn;
	MT_BOOL					b22K;
	MT_FE_LNB_VOLTAGE 		mLnbVoltage;

	MT_BOOL					bToneBurst;
	MT_FE_DiSEqC_TONE_BURST	mToneBurst;

	MT_BOOL					bDiSEqCSend;
	U8						iSizeSend;
	U8						iDataSend[8];

	MT_BOOL					bDiSEqCReceive;
	U8						iSizeReceive;
	U8						iDataReceive[8];

	MT_BOOL					bEnvelopMode;

	MT_BOOL					bUnicable;
	U8						iBankIndex;
	U8						iUBIndex;
	U16						iUBFreqMHz;
	U8						iUBCount;
	U16						iUBList[32];
	U8						iUBVer;
	MT_BOOL					bSpectrumInverted;
} MT_FE_LNB_SETTINGS_DS6103;

typedef struct _MT_FE_TP_PARAMS_DS6103
{
	MT_FE_TYPE				mConnectType;
	MT_FE_TYPE				mCurrentType;
	U32						iFreqKHz;
	U32						iSymRateKSs;
	MT_FE_CODE_RATE			mCodeRate;
	U32						iCarrierOffsetKHz;
	MT_BOOL					bLimitCarrierOffset;
	S32						iOffsetRangeKHz;
	MT_BOOL					bCheckCCM;
	S8						iTsCnt;
	U8						ucTsId[32];
	U8						ucCurTsId;
	MT_BOOL					bHavePLS;
	U8						ucPLSCode[3];
	MT_BOOL					bSuperSearch;
} MT_FE_TP_PARAMS_DS6103;

typedef struct _MT_FE_GLOBAL_SETTINGS_DS6103
{
	MT_BOOL	bEqCoefChecked;
	MT_BOOL	bTsClockChecked;
	U8		iAutoTuneCnt;

	S32		iMclkKHz;

	U32		iSerialMclkHz;


	MT_BOOL	bLastTpAFlag;

	U16		iTpIndex;
	U16		iLockedTpCnt;
	U16		iScannedTpCnt;
	U32		iCurCompareTpAKHz;
	MT_BOOL	bCancelBs;
	U8		bBsStatus;//0:not start or has finished, 1:start to blindscan
} MT_FE_GLOBAL_SETTINGS_DS6103;


typedef struct _MT_FE_TN_DEVICE_SETTINGS_DS6103
{
	MT_FE_TN_SUPPORTED_TUNER tuner_type;

	U8		tuner_init_OK;
	U8		tuner_dev_addr;

	U32		tuner_freq_MHz;
	U32		tuner_symbol_rate_KSs;
	U32		tuner_lo_freq_KHz;
	U32		tuner_lpf_offset_KHz;

	U32		tuner_crystal_KHz;

	U16		tuner_custom_cfg;
	U32		tuner_driver_version;

	U8		tuner_input_mode;
	U8		tuner_clock_out;

	U32		tuner_time;

	mt_handle		tuner_handle;

	MT_FE_RET (*tuner_init)(void *handle);												/* tuner initialize function */
	MT_FE_RET (*tuner_set)(void *handle, U32 freq_KHz, U32 sym_KSs, S16 lpf_offset_KHz);/* set tuner function */
	S32 (*tuner_get_offset)(void *handle);												/* get frequency offset function */
	MT_FE_RET (*tuner_get_strength)(void *handle, U32 *p_gain, S32 *p_strength);		/* get signal strength function */
	MT_FE_RET (*tuner_get_version)(void *handle, U32 *version_no, U32 *version_time);	/* Get tuner version information */
	MT_FE_RET (*tuner_sleep)(void *handle);												/* Sleep the tuner */
	MT_FE_RET (*tuner_wakeup)(void *handle);											/* Wakeup the tuner */
} MT_FE_TN_DEVICE_SETTINGS_DS6103, *MT_FE_Tuner_Handle_DS6103;

typedef struct _MT_FE_DS6103_SETTINGS
{
	U8								dev_index;			/* device index */
	U8								demod_dev_addr;		/* demodulator device 2-wire bus address */
	U8								it_dev_addr;		/* internal device address */
	MT_FE_DMD_ID					demod_id;			/* demodulator ID */
	MT_FE_SUPPORTED_DEMOD			demod_type;			/* demodulator type */
	MT_FE_TYPE						dtv_mode;			/* dtv mode */

	MT_FE_TN_DEVICE_SETTINGS_DS6103 tuner_cfg;			/* Tuner settings */
	MT_FE_BOARD_SETTINGS_DS6103		board_cfg;			/* Board settings */
	MT_FE_TS_SETTINGS_DS6103		ts_cfg;				/* TS settings */
	MT_FE_LNB_SETTINGS_DS6103		lnb_cfg;			/* LNB settings */
	MT_FE_GLOBAL_SETTINGS_DS6103	global_cfg;			/* Global settings */

	MT_FE_TP_PARAMS_DS6103			tp_cfg;				/* Channel parameters */

	U32								version_number;		/* demod version number */
	U32								version_time;		/* demod version time */

	MT_FE_RET	(*dmd_set_reg)(void *handle, U8 reg_index, U8 reg_value);
	MT_FE_RET	(*dmd_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
	MT_FE_RET	(*write_fw)(void *handle, U8 reg_index, U8 *p_buf, U16 n_byte);
	void		(*mt_sleep)(U32 ticks_ms);
	MT_FE_RET	(*tn_set_reg)(void *handle, U8 reg_index, U8 reg_value);
	MT_FE_RET	(*tn_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
	MT_FE_RET	(*tn_read)(void *handle, U8 reg_index, U8 *p_buf, U16 n_byte);
	MT_FE_RET	(*tn_write)(void *handle, U8 *p_buf, U16 n_byte);
	MT_FE_RET	(*it_set_reg)(void *handle, U8 reg_index, U8 reg_value);
	MT_FE_RET	(*it_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
} MT_FE_DS6103_DEVICE_SETTINGS, *MT_FE_DS6103_Device_Handle;


/* Tuner DEFINES */
#define MT_FE_IT_I2C_ADDR   0x5C        /* Tuner address */

/* set Frequency Offset to tuner When symbol rate < 5000 KSs */
#define FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz       3000

/* Tuner APIs */
void mt_fe_it_wakeup_ds6103(MT_FE_DS6103_Device_Handle handle);
void mt_fe_it_sleep_ds6103(MT_FE_DS6103_Device_Handle handle);
void mt_fe_it_init_ds6103(MT_FE_DS6103_Device_Handle handle);
void mt_fe_it_reset_ds6103(MT_FE_DS6103_Device_Handle handle);
void mt_fe_it_dmpll_select_xm_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *xm_KHz, U32 symbol_rate, U32 tuner_freq_MHz);
void mt_fe_it_dmpll_select_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, U32 tuner_freq_MHz, MT_BOOL bBs);
void mt_fe_it_dmpll_get_ts_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_MCLK_KHz);
void mt_fe_it_dmpll_set_ts_mclk_ds6103(MT_FE_DS6103_Device_Handle handle, MT_FE_TS_OUT_MODE ts_mode, U32 MCLK_KHz);



#define AGC_POLAR       0
#define IQ_INVERTED     0


/* TS6011 */
#define MT_FE_TN_I2C_ADDR_TS6011 0x58


MT_FE_RET mt_fe_tn_init_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_get_tuner_freq_offset_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_get_strength_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength);
MT_FE_RET mt_fe_tn_sleep_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts6011_ds6103(MT_FE_DS6103_Device_Handle handle);


/* RDA5815S */
#define MT_FE_TN_I2C_ADDR_RDA5815S 0x18


MT_FE_RET mt_fe_tn_init_RDA5815S_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_RDA5815S_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_get_tuner_freq_offset_RDA5815S_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_get_strength_RDA5815S_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength);


/* RDA5815M */
#define MT_FE_TN_I2C_ADDR_RDA5815M 0x18


MT_FE_RET mt_fe_tn_init_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_get_tuner_freq_offset_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_get_strength_RDA5815M_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength);


/* AV2018 */
#define MT_FE_TN_I2C_ADDR_AV2018 0xC4


MT_FE_RET mt_fe_tn_init_AV2018_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_AV2018_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_get_tuner_freq_offset_AV2018_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_get_strength_AV2018_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength);


/* AV2012 */
#define MT_FE_TN_I2C_ADDR_AV2012 0xC6


MT_FE_RET mt_fe_tn_init_AV2012_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_AV2012_ds6103(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
S32 mt_fe_tn_get_tuner_freq_offset_AV2012_ds6103(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_tn_get_strength_AV2012_ds6103(MT_FE_DS6103_Device_Handle handle, U32 *p_gain, S32 *p_strength);


/*****************************************************************************/


/* demod&tuner address */
#define MT_FE_DMD_DEV_ADDR_DS6103            0xD4        /* Demodulator address    */


/* AC coupling control*/
#define MT_FE_ENABLE_AC_COUPLING            1        /*1: AC coupling (recommended in reference design) 0: DC coupling*/


/*    CCI    threshold */
#define MT_FE_CCI_THRESHOLD                 2

/*    DVB-S2 equalizer coefficient ratio threshold */
#define MT_FE_S2_EQ_COEF_THRESHOLD          5


/*    CLOCK DEFINES*/
#define MT_FE_MCLK_KHZ                      96000

#define MT_FE_MCLK_KHZ_SERIAL_S2            144000//96000


// Improve the driver capability or not
// 0: 4mA, 1: 8mA, 2: 12mA, 3, 16mA
#define MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI    0    // Parallel Mode or Common Interface Mode
#define MT_FE_ENHANCE_TS_PIN_LEVEL_SERIAL         0    // Serial Mode

/* CLOCK OUTPUT TO DECODER*/
#define MT_FE_ENABLE_27MHZ_CLOCK_OUT        0
#define MT_FE_ENABLE_13_P_5_MHZ_CLOCK_OUT   0


//#define MT_FE_PARALLEL_CLK_OUTPUT_FIXED_24MHZ     1



/*****************  CHECK CARRIER OFFSET DEFINEs  *********************
**    This define is used to unlock DS6103 if the carrier offset
** is larger than the defined carrier offset.
**      User can set this define to "1" and set the carrier offset
** (6MHz offset as default).
**      If the carrier offset if larger than the defined value, DS6103
** will be unlocked even if it locked.
**
**********************************************************************/
#define MT_FE_CHECK_CARRIER_OFFSET          1
#define MT_FE_CARRIER_OFFSET_KHz            6000 /*6MHz carrier offset*/


/******************** LNB and DISEQC DEFINES ************************/
/*  Maybe user need change the defines according to reference design  */
#define LNB_ENABLE_WHEN_LNB_EN_HIGH         1
#define LNB_13V_WHEN_VSEL_HIGH              0
#define LNB_VSEL_STANDBY_HIGH               1
#define LNB_DISEQC_OUT_FORCE_HIGH           0
#define LNB_DISEQC_OUT_ONLY_OUTPUT          0


/*
  Select TS output pin order
  For Serial TS mode, swap pin D0 & pin D7
  For Parallel or CI mode, swap the order of D0 ~ D7
*/
#define MT_FE_TS_PIN_ORDER_D0_D7        0        // 0: D0, 1: D7

#define MT_FE_TS_CLOCK_AUTO_SET_FOR_SERIAL_MODE 1
#define MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE     1


/**************    Blind Scan DEFINEs *****************/
#define MT_FE_BS_TIMES              2

#define MT_FE_BS_TIMES_1ST          0
#define MT_FE_BS_TIMES_2ND          1
#define MT_FE_BS_TIMES_3RD          2
#define MT_FE_BS_TIMES_ONE_LOOP     4


#define MT_FE_BS_ALGORITHM_A        1
#define MT_FE_BS_ALGORITHM_B        2
#define MT_FE_BS_ALGORITHM_DEF      MT_FE_BS_ALGORITHM_A

/*process scanned TP defines*/
#define BLINDSCAN_LPF_OFFSET_KHz    3000
#define FREQ_MAX_KHz                9999000


/*blind scan register defines*/
#define    FFT_LENGTH       512
#define    PSD_OVERLAP      96
#define    AVE_TIMES        256
#define    DATA_LENGTH      512

#define    NOTCH_RANGE_F    8
#define    START_RANGE_F    15
#define    FIND_NOTCH_STEP  1
#define    PSD_SCALE        0
#define    SM_BUFDEP        8

#define    BLINDSCAN_SYMRATEKSs     45000//40000

#define    ONE_LOOP_SM_BUF                  4
#define    SNR_THR_ONE_LOOP                 64
#define    ERR_BOUND_FACTOR_ONE_LOOP        36//52
#define    FLAT_THR_ONE_LOOP                28
#define    THR_FACTOR_ONE_LOOP              5

#define    TWO_LOOP_SM_BUF_1ST              28
#define    TWO_LOOP_SNR_THR_1ST             96
#define    TWO_LOOP_ERR_BOUND_FACTOR_1ST    56
#define    TWO_LOOP_FLAT_THR_1ST            24
#define    TWO_LOOP_THR_FACTOR_1ST          1

#define    TWO_LOOP_SM_BUF_2ND              2
#define    TWO_LOOP_SNR_THR_2ND             64
#define    TWO_LOOP_ERR_BOUND_FACTOR_2ND    36
#define    TWO_LOOP_FLAT_THR_2ND            28
#define    TWO_LOOP_THR_FACTOR_2ND          5

#define    TUNER_SLIP_STEP          11

#define    THR_FACTOR_1ST           TWO_LOOP_THR_FACTOR_1ST         //1
#define    THR_FACTOR_2ND           THR_FACTOR_ONE_LOOP             //5
#define    THR_FACTOR_3RD           TWO_LOOP_THR_FACTOR_2ND         //10

#define    ERR_BOUND_FACTOR_1ST     TWO_LOOP_ERR_BOUND_FACTOR_1ST   //56
#define    ERR_BOUND_FACTOR_2ND     ERR_BOUND_FACTOR_ONE_LOOP       //40
#define    ERR_BOUND_FACTOR_3RD     TWO_LOOP_ERR_BOUND_FACTOR_2ND   //40

#define    FLAT_THR_1ST             TWO_LOOP_FLAT_THR_1ST           //24
#define    FLAT_THR_2ND             FLAT_THR_ONE_LOOP               //24
#define    FLAT_THR_3RD             TWO_LOOP_FLAT_THR_2ND           //24

#define    SNR_THR_1ST              TWO_LOOP_SNR_THR_1ST            //96
#define    SNR_THR_2ND              SNR_THR_ONE_LOOP                //80
#define    SNR_THR_3RD              TWO_LOOP_SNR_THR_2ND            //64

#define    SM_BUF_1ST               TWO_LOOP_SM_BUF_1ST             //14
#define    SM_BUF_2ND               ONE_LOOP_SM_BUF                 //10
#define    SM_BUF_3RD               TWO_LOOP_SM_BUF_2ND             //4



#define TMP1   ((AVE_TIMES/32 - 1)*16 + (DATA_LENGTH/128 - 1))  // = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
#define TMP2   (64 + PSD_OVERLAP/16)                            // = (64 + 96/16) = (64 + 6) = 0x46
#define TMP3   (PSD_SCALE*32)                                   // = (0*32) = 0
#define TMP4   (NOTCH_RANGE_F*16 + START_RANGE_F)               // = (8*16 + 15) = 0x8F


/* Demodulator APIs */
MT_FE_RET mt_fe_dmd_ds6103_config_default(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_init(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_soft_reset(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_global_reset(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_get_demod_id(MT_FE_DS6103_Device_Handle handle, MT_FE_DMD_ID *dmd_id);
MT_FE_RET mt_fe_dmd_ds6103_hard_reset(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_sleep(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_wake_up(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_get_driver_version(MT_FE_DS6103_Device_Handle handle, U32* p_version_number, U32 *p_version_time);

MT_FE_RET mt_fe_dmd_ds6103_select_tuner(MT_FE_DS6103_Device_Handle handle, MT_FE_TN_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_ds6103_change_ts_out_mode(MT_FE_DS6103_Device_Handle handle, MT_FE_TS_OUT_MODE mode);

MT_FE_RET mt_fe_dmd_ds6103_connect(MT_FE_DS6103_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type);
MT_FE_RET mt_fe_dmd_ds6103_get_snr(MT_FE_DS6103_Device_Handle handle, S8 *p_snr);
MT_FE_RET mt_fe_dmd_ds6103_get_sat_quality(MT_FE_DS6103_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_ds6103_get_strength(MT_FE_DS6103_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_dmd_ds6103_get_lock_state(MT_FE_DS6103_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_ds6103_get_pure_lock(MT_FE_DS6103_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_ds6103_set_ts(MT_FE_DS6103_Device_Handle handle, U8 iIndex, U8 ucTsId);
MT_FE_RET mt_fe_dmd_ds6103_clear_ts(MT_FE_DS6103_Device_Handle handle);
MT_FE_RET mt_fe_dmd_ds6103_get_per(MT_FE_DS6103_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags);
MT_FE_RET mt_fe_dmd_ds6103_get_channel_info(MT_FE_DS6103_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info);

MT_FE_RET mt_fe_dmd_ds6103_set_LNB(MT_FE_DS6103_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_ds6103_DiSEqC_send_tone_burst(MT_FE_DS6103_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_ds6103_DiSEqC_send_receive_msg(MT_FE_DS6103_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_ds6103_DiSEqC_send_msg(MT_FE_DS6103_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_ds6103_DiSEqC_receive_msg(MT_FE_DS6103_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET mt_fe_dmd_ds6103_blindscan_abort(MT_FE_DS6103_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_ds6103_register_notify(void (*callback)(MT_FE_MSG msg, void *p_tp_info));
MT_FE_RET mt_fe_dmd_ds6103_blindscan(MT_FE_DS6103_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_ds6103_blindscan_get_psd(MT_FE_DS6103_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);

MT_BOOL _mt_fe_dmd_ds6103_check_MIS(MT_FE_DS6103_Device_Handle handle);

MT_FE_RET _mt_fe_dmd_ds6103_bs_connect(MT_FE_DS6103_Device_Handle handle,
									 MT_FE_BS_TP_INFO *p_bs_info,
									 U16 start_index,
									 U16 scanned_tp);

MT_FE_RET _mt_fe_dmd_ds6103_get_carrier_offset(MT_FE_DS6103_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_ds6103_get_sym_rate(MT_FE_DS6103_Device_Handle handle, U32 *sym_rate_KSs);


typedef struct _MT_FE_UNICABLE_DEVICE
{
    U32     freq_MHz;
    MT_BOOL    UB_ok;

    U32     result_MHz;
    U32     result_PSD;
    U32     result_gain;
} MT_FE_UNICABLE_DEVICE;


MT_FE_RET mt_fe_dmd_ds6103_unicable_set_tuner(MT_FE_DS6103_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32* real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz);

MT_FE_RET mt_fe_dmd_ds6103_unicable_retry(MT_FE_DS6103_Device_Handle handle);


typedef struct _MT_FE_PLS_INFO
{
    U8              iPLSCode;
    MT_BOOL            bValid;
    MT_FE_TYPE      mDvbType;
    MT_FE_MOD_MODE  mModMode;
    MT_FE_CODE_RATE mCodeRate;
    MT_BOOL            bPilotOn;
    MT_BOOL            bDummyFrame;
    S8              iFrameLength;		/*0: Normal; 1: Short; 2: Medium*/
    S8              iVcmCycle;
} MT_FE_PLS_INFO;


#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_DEF_H__ */

