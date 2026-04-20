/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*****************************************************************************
 *----------------------------------------------------------------------------
 *
 * File:                 mt_fe_def.h
 *
 * Current version:      0.06.08
 *
 * Description:
 *
 * Log:  Description     Version     Date            Author
 *----------------------------------------------------------------------------
 *       File Create     0.00.01     2016.08.12      YZ.Huang
 *       Modify          0.01.00     2016.09.07      YZ.Huang
 *       Modify          0.02.00     2016.10.10      YZ.Huang
 *       Modify          0.03.00     2016.10.11      YZ.Huang
 *       Modify          0.06.06     2016.10.11      YZ.Huang
 *       Modify          0.06.08     2018.03.13      YZ.Huang
 *****************************************************************************/
#ifndef __MT_FE_DEF_CS8K_SAT_H__
#define __MT_FE_DEF_CS8K_SAT_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdio.h>

#include "mt_fe_common_cs8000_sat.h"

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) ((void)(x))
#endif

typedef enum _MT_FE_LNB_VOLTAGE {
    MtFeLNB_13V = 0,
    MtFeLNB_18V
} MT_FE_LNB_VOLTAGE;

typedef enum _MT_FE_CODE_RATE {
    MtFeCodeRate_Undef = 0,
    MtFeCodeRate_1_4,
    MtFeCodeRate_1_3,
    MtFeCodeRate_2_5,
    MtFeCodeRate_1_2,
    MtFeCodeRate_3_5,
    MtFeCodeRate_2_3,
    MtFeCodeRate_3_4,
    MtFeCodeRate_4_5,
    MtFeCodeRate_5_6,
    MtFeCodeRate_7_8,
    MtFeCodeRate_8_9,
    MtFeCodeRate_9_10
} MT_FE_CODE_RATE;

typedef enum _MT_FE_ROLL_OFF {
    MtFeRollOff_Undef = 0,
    MtFeRollOff_0p35,
    MtFeRollOff_0p25,
    MtFeRollOff_0p20
} MT_FE_ROLL_OFF;

typedef enum _MT_FE_SPECTRUM_MODE {
    MtFeSpectrum_Undef = 0,
    MtFeSpectrum_Normal,
    MtFeSpectrum_Inversion
} MT_FE_SPECTRUM_MODE;

typedef enum _MT_FE_LNB_LOCAL_OSC {
    MtFeLNB_Single_Local_OSC = 0,
    MtFeLNB_Dual_Local_OSC
} MT_FE_LNB_LOCAL_OSC;

typedef enum _MT_FE_BAND_TYPE {
    MtFeBand_C = 0,
    MtFeBand_Ku
} MT_FE_BAND_TYPE;

typedef enum _MT_FE_MSG {
    MtFeMsg_BSTpFind = 0,
    MtFeMsg_BSTpLocked,
    MtFeMsg_BSTpUnlock,
    MtFeMsg_BSStart,
    MtFeMsg_BSFinish,
    MtFeMsg_BSOneWinFinish,
    MtFeMsg_BSAbort,
    MtFeMsg_CCMFind
} MT_FE_MSG;

typedef struct _MT_FE_CHAN_INFO_DVBS2
{
    MT_FE_TYPE type;
    MT_FE_MOD_MODE mod_mode;
    MT_FE_ROLL_OFF roll_off;
    MT_FE_CODE_RATE code_rate;
    MT_FE_BOOL is_pilot_on;
    MT_FE_SPECTRUM_MODE is_spectrum_inv;
} MT_FE_CHAN_INFO_DVBS2;

typedef enum _MT_FE_DiSEqC_TONE_BURST {
    MtFeDiSEqCToneBurst_Moulated = 0,
    MtFeDiSEqCToneBurst_Unmoulated
} MT_FE_DiSEqC_TONE_BURST;

typedef struct _MT_FE_DiSEqC_MSG
{
    U8 data_send[8];
    U8 size_send;
    MT_BOOL is_enable_receive;
    MT_BOOL is_envelop_mode;
    U8 data_receive[8];
    U8 size_receive;
} MT_FE_DiSEqC_MSG;

typedef enum _MT_FE_DMD_ID {
    MtFeDmdId_Undef,
    MtFeDmdId_DS300X,
    MtFeDmdId_DS3002B,
    MtFeDmdId_DS3103,
    MtFeDmdId_DS3103B,
    MtFeDmdId_RS6000,
    MtFeDmdId_CS8000_SAT,
    MtFeDmdId_Unknown
} MT_FE_DMD_ID;

typedef struct _MT_FE_TP_INFO
{
    U32 freq_KHz;
    U16 sym_rate_KSs;
    MT_FE_TYPE dvb_type;
    MT_FE_CODE_RATE code_rate;
    U32 iLockTime;
    U32 uidx_notch_last;
    U32 uidx_up;
    U32 uidx_notch;
    U32 upsd_sn;

    MT_BOOL bCheckCCM;
    S8 iTsCnt;
    U8 ucTsId[16];
    U8 ucCurTsId;
} MT_FE_TP_INFO;

typedef struct _MT_FE_BS_TP_INFO
{
    U8 bs_times;
    U8 bs_algorithm;
    U16 tp_num;
    U16 tp_max_num;
    MT_BOOL bNewGain;
    U32 iPreGain;
    U32 iCurGain;
    U8 bs_cur_loop;
    U32 next_freq_KHz; //return next freq to app
    MT_FE_TP_INFO *p_tp_info;
} MT_FE_BS_TP_INFO;

typedef enum _MT_FE_TS_DRIVER_ABILITY {
    MtFeTsDriverAbility_4mA = 0,
    MtFeTsDriverAbility_8mA = 1,
    MtFeTsDriverAbility_12mA = 2,
    MtFeTsDriverAbility_16mA = 3
} MT_FE_TS_DRIVER_ABILITY;

typedef enum _MT_FE_CS8000_SAT_SUPPORTED_TUNER {
    MT_FE_TN_NOTSUPPORT = 0x0000,
    /*DVB-C*/
    TN_MONTAGE_TC2800,
    TN_MONTAGE_TC2000,
    TN_ALPS_TDAC,
    TN_THOMSON_DCT7070X,
    TN_XUGUANG_XDCT6A,
    TN_NXP_TDA18250,
    TN_LG_TDCC,
    TN_LG_TDCC_G1X1F,
    TN_MXL_608,
    TN_MONTAGE_TC3800,
    TN_NXP_TDA18250A,
    TN_MXL_203,
    TN_MONTAGE_TC6800,
    TN_RAFAEL_R836,
    /*DVB-S/S2*/
    TN_SHARP_6306,
    TN_SHARP_7306,
    TN_SHARP_7803,
    TN_SHARP_7903,
    TN_AV2011,
    TN_AV2012,
    TN_AV2026,
    TN_ST6110,
    TN_ST6110_ON_BOARD,
    TN_MONTAGE_TS2022,
    TN_MONTAGE_TS6011,
    MT_FE_TN_USERDEFINE = 0xFFFF
} MT_FE_CS8000_SAT_SUPPORTED_TUNER;

typedef struct _MT_FE_BOARD_SETTINGS_CS8000_SAT
{
    MT_BOOL bIQInverted;
    MT_BOOL bAGCPolar;
    MT_BOOL bSpectrumInverted;

    MT_BOOL bPolarLNBEnable;   // LNB_ENABLE_WHEN_LNB_EN_HIGH  0
    MT_BOOL bPolarLNB13V;      // LNB_13V_WHEN_VSEL_HIGH       1
    MT_BOOL bPolarLNBStandby;  // LNB_VSEL_STANDBY_HIGH        1
    MT_BOOL bPolarDiSEqCOut;   // LNB_DISEQC_OUT_FORCE_HIGH    0
    MT_BOOL bDiSEqCOutputOnly; // LNB_DISEQC_OUT_ONLY_OUTPUT   0
} MT_FE_BOARD_SETTINGS_CS8000_SAT;

typedef struct _MT_FE_TS_SETTINGS_CS8000_SAT
{
    MT_FE_TS_OUT_MODE mTsMode;
    MT_BOOL bHighZ;
    MT_BOOL bPinSwitch;
    MT_FE_TS_DRIVER_ABILITY mPinDriver;
    MT_FE_TS_OUT_MAX_CLOCK mMaxClock;
    U8 iCustomDutyHigh;
    U8 iCustomDutyLow;
    U8 iSerialDriverMode;
    MT_BOOL bAutoSerial;
    MT_BOOL bAutoParallel;
    MT_BOOL bAutoCI;
    MT_BOOL bRisingEdge;
    MT_BOOL bHighSync;
    MT_BOOL bHighValid;
    MT_BOOL bHighError;
} MT_FE_TS_SETTINGS_CS8000_SAT;

typedef struct _MT_FE_LNB_SETTINGS_CS8000_SAT
{
    MT_BOOL bLnbOn;
    MT_BOOL b22K;
    MT_FE_LNB_VOLTAGE mLnbVoltage;

    MT_BOOL bToneBurst;
    MT_FE_DiSEqC_TONE_BURST mToneBurst;

    MT_BOOL bDiSEqCSend;
    U8 iSizeSend;
    U8 iDataSend[8];

    MT_BOOL bDiSEqCReceive;
    U8 iSizeReceive;
    U8 iDataReceive[8];

    MT_BOOL bEnvelopMode;

    MT_BOOL bUnicable;
    U8 iBankIndex;
    U8 iUBIndex;
    U16 iUBFreqMHz;
    U8 iUBCount;
    U16 iUBList[32];
    U8 iUBVer;
} MT_FE_LNB_SETTINGS_CS8000_SAT;

typedef struct _MT_FE_TP_PARAMS_CS8000_SAT
{
    MT_FE_TYPE mConnectType;
    MT_FE_TYPE mCurrentType;
    U32 iFreqKHz;
    U32 iSymRateKSs;
    MT_FE_CODE_RATE mCodeRate;
    U32 iCarrierOffsetKHz;
    MT_BOOL bLimitCarrierOffset;
    S32 iOffsetRangeKHz;
    MT_BOOL bCheckCCM;
    S8 iTsCnt;
    U8 ucTsId[16];
    U8 ucCurTsId;
	MT_BOOL					bHavePLS;
	U8						ucPLSCode[3];
} MT_FE_TP_PARAMS_CS8000_SAT;

typedef struct _MT_FE_GLOBAL_SETTINGS_CS8000_SAT
{
    MT_BOOL bTsClockChecked;

    S32 iMclkKHz;

    U32 iSerialMclkHz;

    MT_BOOL bLastTpAFlag;

    U16 iTpIndex;
    U16 iLockedTpCnt;
    U16 iScannedTpCnt;
    U32 iCurCompareTpAKHz;
    MT_BOOL bCancelBs;
    U8 iTotalBsTimes;
    MT_BOOL bSpecialProcessed;
} MT_FE_GLOBAL_SETTINGS_CS8000_SAT;

typedef struct _MT_FE_BS_SETTINGS_CS8000_SAT
{
    U16 fft_length;
    U8 psd_overlap;
    U16 ave_times;
    U16 data_length;
    U8 notch_range_f;
    U8 start_range_f;
    U8 find_notch_step;
    U8 psd_scale;
    U8 sm_bufdep;

    U8 tuner_slip_step;

    U16 bs_symrate;
    U16 bs_skip_step;

    //U8		sm_buf_1_1st;
    //U8		snr_thr_1_1st;
    //U8		err_bound_factor_1_1st;
    //U8		flat_thr_1_1st;
    //U8		thr_factor_1_1st;

    //U8		sm_buf_2_1st;
    //U8		snr_thr_2_1st;
    //U8		err_bound_factor_2_1st;
    //U8		flat_thr_2_1st;
    //U8		thr_factor_2_1st;

    //U8		sm_buf_2_2nd;
    //U8		snr_thr_2_2nd;
    //U8		err_bound_factor_2_2nd;
    //U8		flat_thr_2_2nd;
    //U8		thr_factor_2_2nd;

    U8 sm_buf_1st;
    U8 sm_buf_2nd;
    U8 sm_buf_3rd;

    U8 snr_thr_1st;
    U8 snr_thr_2nd;
    U8 snr_thr_3rd;

    U8 err_bound_factor_1st;
    U8 err_bound_factor_2nd;
    U8 err_bound_factor_3rd;

    U8 flat_thr_1st;
    U8 flat_thr_2nd;
    U8 flat_thr_3rd;

    U8 thr_factor_1st;
    U8 thr_factor_2nd;
    U8 thr_factor_3rd;
} MT_FE_BS_SETTINGS_CS8000_SAT;

typedef struct _MT_FE_TN_Device_Settings_CS8000_SAT
{
    MT_FE_CS8000_SAT_SUPPORTED_TUNER tuner_type;

    U8 tuner_init_OK;
    U8 tuner_dev_addr;

    U32 tuner_freq_MHz;
    U32 tuner_symbol_rate_KSs;
    U32 tuner_lo_freq_KHz;
    U32 tuner_lpf_offset_KHz;

    U32 tuner_crystal_KHz;

    U16 tuner_custom_cfg;
    U32 tuner_driver_version;

    U8 tuner_input_mode;
    U8 tuner_clock_out;

    U32 tuner_handle;

    MT_FE_RET (*tuner_init)(void *handle);                                               /* tuner initialize function */
    MT_FE_RET (*tuner_set)(void *handle, U32 freq_KHz, U32 sym_KSs, S16 lpf_offset_KHz); /* set tuner function */
    MT_FE_RET (*tuner_get_offset)(void *handle, S32 *freq_offset_KHz);                   /* get frequency offset function */
    MT_FE_RET (*tuner_get_strength)(void *handle, U32 *p_gain, S32 *p_strength);         /* get signal strength function */
    MT_FE_RET (*tuner_get_version)(void *handle, U32 *version_no, U32 *version_time);    /* Get tuner version information */
    MT_FE_RET (*tuner_sleep)(void *handle);                                              /* Sleep the tuner */
    MT_FE_RET (*tuner_wakeup)(void *handle);                                             /* Wakeup the tuner */
    MT_FE_RET (*tuner_adjust_agc)(void *handle);                                         /* adjust the tuner agc */
    void *p_tun_bus;                                                                     //2-wire bus device, if tuner i2c is repeated by demod, set NULL here
} MT_FE_TN_DEVICE_SETTINGS_CS8000_SAT, *MT_FE_Tuner_Handle_CS8000_SAT;

typedef struct _MT_FE_CS8000_SAT_SETTINGS
{
    U8 dev_index;                     /* device index */
    U8 demod_dev_addr;                /* demodulator device 2-wire bus address */
    void *p_dem_bus;                  /* 2-wire bus device handle */
    MT_FE_DMD_ID demod_id;            /* demodulator ID */
    MT_FE_SUPPORTED_DEMOD demod_type; /* demodulator type */
    MT_FE_TYPE dtv_mode;              /* dtv mode */

    MT_FE_TN_DEVICE_SETTINGS_CS8000_SAT tuner_cfg; /* Tuner settings */
    MT_FE_BOARD_SETTINGS_CS8000_SAT board_cfg;     /* Board settings */
    MT_FE_TS_SETTINGS_CS8000_SAT ts_cfg;           /* TS settings */
    MT_FE_LNB_SETTINGS_CS8000_SAT lnb_cfg;         /* LNB settings */
    MT_FE_GLOBAL_SETTINGS_CS8000_SAT global_cfg;   /* Global settings */
    MT_FE_BS_SETTINGS_CS8000_SAT bs_cfg;           /* Blind scan settings */

    MT_FE_TP_PARAMS_CS8000_SAT tp_cfg; /* Channel parameters */

    U32 version_number; /* demod version number */
    U32 version_time;   /* demod version time */

    MT_FE_RET (*dmd_set_reg)(void *handle, U8 reg_index, U8 reg_value);
    MT_FE_RET (*dmd_get_reg)(void *handle, U8 reg_index, U8 *p_buf);
    MT_FE_RET (*write_fw)(void *handle, U8 reg_index, U8 *p_buf, U16 n_byte);
    MT_FE_RET (*dmd_read)(void *handle, U8 reg_index, U8 *p_buf, U16 n_byte);
    void (*mt_sleep)(U32 ticks_ms);
    MT_FE_RET (*tn_set_reg)(void *handle, U8 reg_index, U8 reg_value);
    MT_FE_RET (*tn_get_reg)(void *handle, U8 reg_index, U8 *p_buf);

    MT_FE_RET (*Set32Bits)(U32 reg_addr, U32 reg_data);
    MT_FE_RET (*Get32Bits)(U32 reg_addr, U32 *p_data);
    void *lnb_agent_priv;
} MT_FE_CS8000_SAT_DEVICE_SETTINGS, *MT_FE_CS8000_SAT_Device_Handle;

/* Tuner DEFINES */
#define MT_FE_TN_I2C_ADDR 0xC0 // 0x42        /* Tuner address */

/* set Frequency Offset to tuner When symbol rate < 5000 KSs */
#define FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz 3000

/* Tuner APIs */
#define AGC_POLAR 0
#define IQ_INVERTED 0

MT_FE_RET mt_fe_sat_tn_init_ts2022(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_sat_tn_set_freq_ts2022(MT_FE_CS8000_SAT_Device_Handle handle, U32 Freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_sat_tn_get_tuner_freq_offset_ts2022(MT_FE_CS8000_SAT_Device_Handle handle, S32 *p_freq_offset_KHz);
MT_FE_RET mt_fe_sat_tn_get_strength_ts2022(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_gain, S32 *p_strength);
MT_FE_RET mt_fe_sat_tn_get_version_ts2022(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_version_number, U32 *p_version_time);
MT_FE_RET mt_fe_sat_tn_wakeup_ts2022(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_sat_tn_sleep_ts2022(MT_FE_CS8000_SAT_Device_Handle handle);

#define TUNER_RFBYPASS_ON 1  /* If you need loop through function, please set it to 1 , else set it to 0 */
#define TUNER_CKOUT_ON 0     /* If you need clock out from tuner clkout pin to demodulator use, please set it to 1 , else set it 0*/
#define TUNER_CKOUT_DIV 0x01 /* The clock out division */
#define TUNER_CKOUT_XTAL 1   /* If you need clock out from tuner xtalout pin to demodulator use, please set it to 1 , else set it 0*/

/*TS6011*/
MT_FE_RET mt_fe_sat_tn_init_ts6011(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_sat_tn_set_freq_ts6011(MT_FE_CS8000_SAT_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_sat_tn_get_tuner_freq_offset_ts6011(MT_FE_CS8000_SAT_Device_Handle handle, S32 *p_freq_offset_KHz);
MT_FE_RET mt_fe_sat_tn_get_strength_ts6011(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_gain, S32 *p_strength);
MT_FE_RET mt_fe_sat_tn_get_version_ts6011(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_version_number, U32 *p_version_time);
MT_FE_RET mt_fe_sat_tn_sleep_ts6011(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_sat_tn_wake_up_ts6011(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_sat_tn_adjust_agc_ts6011(MT_FE_CS8000_SAT_Device_Handle handle);

/* demod&tuner address */
#define MT_FE_DMD_DEV_ADDR_CS8K_SAT 0xd0 /* Demodulator address    */

/* AC coupling control*/
#define MT_FE_ENABLE_AC_COUPLING 1 /*1: AC coupling (recommended in reference design) 0: DC coupling*/

/*    CCI    threshold */
#define MT_FE_CCI_THRESHOLD 2

/*    CLOCK DEFINES*/
#define MT_FE_MCLK_KHZ 96000

// Improve the driver capability or not
// 0: 4mA, 1: 8mA, 2: 12mA, 3, 16mA
#define MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI 0 // Parallel Mode or Common Interface Mode

/*****************  CHECK CARRIER OFFSET DEFINEs  *********************
**    This define is used to unlock CS8000 satellite module if the carrier offset
** is larger than the defined carrier offset.
**      User can set this define to "1" and set the carrier offset
** (4MHz offset as default).
**      If the carrier offset if larger than the defined value, CS8000 Sat
** will be unlocked even if it locked.
**
**********************************************************************/
#define MT_FE_CHECK_CARRIER_OFFSET 1
#define MT_FE_CARRIER_OFFSET_KHz 6000 /*6MHz carrier offset*/

/******************** LNB and DISEQC DEFINES ************************/
/*  Maybe user need change the defines according to reference design  */
#define LNB_ENABLE_WHEN_LNB_EN_HIGH 1 //0
#define LNB_13V_WHEN_VSEL_HIGH 0      //1
#define LNB_VSEL_STANDBY_HIGH 1
#define LNB_DISEQC_OUT_FORCE_HIGH 0
#define LNB_DISEQC_OUT_ONLY_OUTPUT 0

/*
  Select TS output pin order
  For Serial TS mode, swap pin D0 & pin D7
  For Parallel or CI mode, swap the order of D0 ~ D7
*/
#define MT_FE_TS_PIN_ORDER_D0_D7 0 // 0: D0, 1: D7

#define MT_FE_TS_CLOCK_AUTO_SET_FOR_SERIAL_MODE 1
#define MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE 1

/**************    Blind Scan DEFINEs *****************/
#define MT_FE_BS_TIMES 2

#define MT_FE_BS_TIMES_1ST 0
#define MT_FE_BS_TIMES_2ND 1
#define MT_FE_BS_TIMES_3RD 2
#define MT_FE_BS_TIMES_ONE_LOOP 4

#define MT_FE_BS_ALGORITHM_A 1
#define MT_FE_BS_ALGORITHM_B 2
#define MT_FE_BS_ALGORITHM_DEF MT_FE_BS_ALGORITHM_A

/*process scanned TP defines*/
#define BLINDSCAN_LPF_OFFSET_KHz 3000
#define FREQ_MAX_KHz 9999000

/*blind scan register defines*/
#define FFT_LENGTH 512
#define PSD_OVERLAP 96
#define AVE_TIMES 256
#define DATA_LENGTH 512

#define NOTCH_RANGE_F 8
#define START_RANGE_F 15
#define FIND_NOTCH_STEP 1
#define PSD_SCALE 0
#define SM_BUFDEP 8

#define BLINDSCAN_SYMRATEKSs 45000

#define ONE_LOOP_SM_BUF 4
#define SNR_THR_ONE_LOOP 64
#define ERR_BOUND_FACTOR_ONE_LOOP 36
#define FLAT_THR_ONE_LOOP 28
#define THR_FACTOR_ONE_LOOP 5

#define TWO_LOOP_SM_BUF_1ST 28
#define TWO_LOOP_SNR_THR_1ST 96
#define TWO_LOOP_ERR_BOUND_FACTOR_1ST 56
#define TWO_LOOP_FLAT_THR_1ST 24
#define TWO_LOOP_THR_FACTOR_1ST 1

#define TWO_LOOP_SM_BUF_2ND 2
#define TWO_LOOP_SNR_THR_2ND 64
#define TWO_LOOP_ERR_BOUND_FACTOR_2ND 36
#define TWO_LOOP_FLAT_THR_2ND 28
#define TWO_LOOP_THR_FACTOR_2ND 5

#define TUNER_SLIP_STEP 11

#define THR_FACTOR_1ST TWO_LOOP_THR_FACTOR_1ST
#define THR_FACTOR_2ND THR_FACTOR_ONE_LOOP
#define THR_FACTOR_3RD TWO_LOOP_THR_FACTOR_2ND

#define ERR_BOUND_FACTOR_1ST TWO_LOOP_ERR_BOUND_FACTOR_1ST
#define ERR_BOUND_FACTOR_2ND ERR_BOUND_FACTOR_ONE_LOOP
#define ERR_BOUND_FACTOR_3RD TWO_LOOP_ERR_BOUND_FACTOR_2ND

#define FLAT_THR_1ST TWO_LOOP_FLAT_THR_1ST
#define FLAT_THR_2ND FLAT_THR_ONE_LOOP
#define FLAT_THR_3RD TWO_LOOP_FLAT_THR_2ND

#define SNR_THR_1ST TWO_LOOP_SNR_THR_1ST
#define SNR_THR_2ND SNR_THR_ONE_LOOP
#define SNR_THR_3RD TWO_LOOP_SNR_THR_2ND

#define SM_BUF_1ST TWO_LOOP_SM_BUF_1ST
#define SM_BUF_2ND ONE_LOOP_SM_BUF
#define SM_BUF_3RD TWO_LOOP_SM_BUF_2ND

//#define TMP1   ((AVE_TIMES/32 - 1)*16 + (DATA_LENGTH/128 - 1))  // = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
//#define TMP2   (64 + PSD_OVERLAP/16)                            // = (64 + 96/16) = (64 + 6) = 0x46
//#define TMP3   (PSD_SCALE*32)                                   // = (0*32) = 0
//#define TMP4   (NOTCH_RANGE_F*16 + START_RANGE_F)               // = (8*16 + 15) = 0x8F

/* Demodulator APIs */
MT_FE_RET mt_fe_dmd_cs8k_sat_config_default(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_init(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_soft_reset(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_global_reset(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_hard_reset(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_sleep(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_wake_up(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_driver_version(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_version_number, U32 *p_version_time);
MT_FE_RET mt_fe_dmd_cs8k_sat_connect(MT_FE_CS8000_SAT_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type, U32 is_BS);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_snr(MT_FE_CS8000_SAT_Device_Handle handle, S8 *p_snr);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_strength(MT_FE_CS8000_SAT_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_sat_quality(MT_FE_CS8000_SAT_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_lock_state(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_LOCK_STATE *p_state, U32 is_BS);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_pure_lock(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_cs8k_sat_set_ts(MT_FE_CS8000_SAT_Device_Handle handle, U8 ucTsId);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_per(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags);
MT_FE_RET mt_fe_dmd_cs8k_sat_get_channel_info(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info);

MT_FE_RET mt_fe_dmd_cs8k_sat_set_LNB(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_send_tone_burst(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_send_msg(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_receive_msg(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan_abort(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_cs8k_sat_register_notify(void (*callback)(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_MSG msg, void *p_tp_info));
MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U32 is_connect);

MT_FE_RET mt_fe_dmd_cs8k_sat_check_tuner(MT_FE_CS8000_SAT_Device_Handle handle);
MT_FE_RET mt_fe_dmd_cs8k_sat_select_tuner(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_CS8000_SAT_SUPPORTED_TUNER tuner_type);
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_demod(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type);
MT_FE_RET _mt_fe_dmd_cs8k_sat_init_reg(MT_FE_CS8000_SAT_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size);
MT_FE_RET _mt_fe_dmd_cs8k_sat_download_fw(MT_FE_CS8000_SAT_Device_Handle handle, const U8 *p_fw);
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_ts_out_mode(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TS_OUT_MODE mode);
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_sym_rate(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_sym_rate(MT_FE_CS8000_SAT_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_total_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_fec(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 bs_times);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_set_demod(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                                    U32 cur_scan_freq_KHz,
                                                    U8 bs_times);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                                 U32 freq_KHz,
                                                 U32 symbol_rate_KSs,
                                                 U8 bs_times);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_TP_scan(MT_FE_CS8000_SAT_Device_Handle handle, U32 start_freq_KHz,
                                         U32 end_freq_KHz,
                                         MT_FE_BS_TP_INFO *p_bs_info,
                                         U16 *scanned_tp,
                                         U8 bs_times);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_A(MT_FE_CS8000_SAT_Device_Handle handle, U32 start_freq_KHz,
                                   U32 end_freq_KHz,
                                   MT_FE_BS_TP_INFO *p_bs_info,
                                   U8 bs_times, U32 is_connect);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_B(MT_FE_CS8000_SAT_Device_Handle handle, U32 start_freq_KHz,
                                   U32 end_freq_KHz,
                                   MT_FE_BS_TP_INFO *p_bs_info,
                                   U8 bs_times, U32 is_connect);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                         U16 start_index,
                                         U16 scanned_tp);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect_test(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info);

MT_FE_RET _mt_fe_dmd_cs8k_sat_get_current_ts_mode(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TS_OUT_MODE *p_ts_mode);

typedef struct _MT_FE_UNICABLE_DEVICE
{
    U32 freq_MHz;
    MT_BOOL UB_ok;

    U32 result_MHz;
    U32 result_PSD;
    U32 result_gain;
} MT_FE_UNICABLE_DEVICE;

void mt_fe_dmd_cs8k_sat_unicable_blind_detect(MT_FE_CS8000_SAT_Device_Handle handle, U16 start_freq_MHz, U16 stop_freq_MHz, MT_FE_UNICABLE_DEVICE *p_ub_list, MT_BOOL need_init);

MT_FE_RET mt_fe_cs8k_sat_unicable_set_tuner(MT_FE_CS8000_SAT_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32 *real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz);

MT_FE_RET mt_fe_dmd_cs8k_sat_get_psd(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz);
MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan_test(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U32 is_connect);

#ifdef __cplusplus
}
#endif

#endif /* __MT_FE_DEF_H__ */
