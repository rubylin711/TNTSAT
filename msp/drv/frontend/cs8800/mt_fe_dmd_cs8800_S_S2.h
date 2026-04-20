/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2020                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:                 mt_fe_dmd_cs8800_S_S2.h
*
* Current version:     01
*
* Description:
*
* Log:	Description		Version		Date		Author
*      ---------------------------------------------------------------------
*		Create			00			2010.09.13	YZ.Huang
*		Modify			01			2014.07.31	YZ.Huang
****************************************************************************/
#ifndef __MT_FE_CS8800_S_S2_H__
#define __MT_FE_CS8800_S_S2_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "mt_fe_def.h"

typedef enum _MT_FE_LNB_VOLTAGE
{
    MtFeLNB_13V = 0
    ,MtFeLNB_18V
} MT_FE_LNB_VOLTAGE;

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
    MtFeDmdId_RS6000,
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
    U32     next_freq_KHz;
} MT_FE_BS_TP_INFO;

typedef enum _MT_FE_TS_DRIVER_ABILITY
{
    MtFeTsDriverAbility_4mA  = 0,
    MtFeTsDriverAbility_8mA  = 1,
    MtFeTsDriverAbility_12mA = 2,
    MtFeTsDriverAbility_16mA = 3
} MT_FE_TS_DRIVER_ABILITY;

typedef struct _MT_FE_DVBSS2_INPUT_SETTINGS
{
	U32			input_freq_kHz;
	U32			symbol_rate_KSs;
} MT_FE_DVBSS2_INPUT_SETTINGS;

typedef struct _MT_FE_BOARD_SETTINGS_SS2_CS8800
{
	MT_BOOL		bIQInverted;
	MT_BOOL		bAGCPolar;
	MT_BOOL		bSpectrumInverted;

	MT_BOOL		bPolarLNBEnable;			// LNB_ENABLE_WHEN_LNB_EN_HIGH  0
	MT_BOOL		bPolarLNB13V;				// LNB_13V_WHEN_VSEL_HIGH       1
	MT_BOOL		bPolarLNBStandby;			// LNB_VSEL_STANDBY_HIGH        1
	MT_BOOL		bPolarDiSEqCOut;			// LNB_DISEQC_OUT_FORCE_HIGH    0
	MT_BOOL		bDiSEqCOutputOnly;			// LNB_DISEQC_OUT_ONLY_OUTPUT   0
} MT_FE_BOARD_SETTINGS_SS2_CS8800;


typedef struct _MT_FE_TS_SETTINGS_SS2_CS8800
{
	//MT_BOOL					bHighZ;
	//MT_FE_TS_OUT_MODE		ts_out_mode;
	//U8					m_iSerialTSChannel;
	MT_BOOL					bPinSwitch;
	MT_FE_TS_DRIVER_ABILITY	mPinDriver;
	MT_FE_TS_OUT_MAX_CLOCK	mMaxClock;
	U8						iCustomDutyHigh;
	U8						iCustomDutyLow;
	U8						iSerialDriverMode;
	MT_BOOL					bAutoSerial;
	MT_BOOL					bAutoParallel;
	MT_BOOL					bAutoCI;
	//MT_BOOL					bRisingEdge;
	//MT_BOOL					bHighSync;
	//MT_BOOL					bHighValid;
	//MT_BOOL					bHighError;
	MT_BOOL					b3SerialTsOut;
} MT_FE_TS_SETTINGS_SS2_CS8800;

typedef struct _MT_FE_LNB_SETTINGS_SS2_CS8800
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
}MT_FE_LNB_SETTINGS_SS2_CS8800;

typedef struct _MT_FE_TP_PARAMS_SS2_CS8800
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
} MT_FE_TP_PARAMS_SS2_CS8800;

typedef struct _MT_FE_GLOBAL_SETTINGS_SS2_CS8800
{
	S32			iMclkKHz;

	U32			iSerialMclkHz;


	MT_BOOL		bLastTpAFlag;

	U16			iTpIndex;
	U16			iLockedTpCnt;
	U16			iScannedTpCnt;
	U32			iCurCompareTpAKHz;
	MT_BOOL		bCancelBs;
	U8			iTotalBsTimes;
	U8			bBsStatus;//0:not start or has finished, 1:start to blindscan
} MT_FE_GLOBAL_SETTINGS_SS2_CS8800;

typedef struct _MT_FE_BS_SETTINGS_SS2_CS8800
{
	U16		fft_length;
	U8		psd_overlap;
	U16		ave_times;
	U16		data_length;
	U8		notch_range_f;
	U8		start_range_f;
	U8		find_notch_step;
	U8		psd_scale;
	U8		sm_bufdep;

	U8		tuner_slip_step;

	U16		bs_symrate;
	U16		bs_skip_step;

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

	U8		sm_buf_1st;
	U8		sm_buf_2nd;
	U8		sm_buf_3rd;

	U8		snr_thr_1st;
	U8		snr_thr_2nd;
	U8		snr_thr_3rd;

	U8		err_bound_factor_1st;
	U8		err_bound_factor_2nd;
	U8		err_bound_factor_3rd;

	U8		flat_thr_1st;
	U8		flat_thr_2nd;
	U8		flat_thr_3rd;

	U8		thr_factor_1st;
	U8		thr_factor_2nd;
	U8		thr_factor_3rd;
} MT_FE_BS_SETTINGS_SS2_CS8800;

typedef enum _MT_FE_CS8800_TS_GS_MODE_T {
	CS8800_BBHEADER_UNKNOW = 0x00,
    /*!
        BBHeader is Transport Mode
      */
    CS8800_BBHEADER_TRANSPORT_MODE = 0x02,
    /*!
        BBHeader is Generic Packetized Mode
      */
    CS8800_BBHEADER_GENERIC_CONTINUOUS_MODE = 0x03,
    /*!
        BBHeader is GSE-HEM Mode
      */
    CS8800_BBHEADER_GSE_HEM_MODE = 0x04,
    /*!
        BBHeader is Generic Packetized Mode
      */
    CS8800_BBHEADER_GENERIC_PACKETIZED_MODE = 0x05,
} MT_FE_CS8800_TS_GS_MODE_T;

/* Tuner DEFINES */
#define MT_FE_TN_I2C_ADDR   0x42        /* Tuner address */

/* set Frequency Offset to tuner When symbol rate < 5000 KSs */
#define FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz       3000

/* Tuner APIs */
//#define AGC_POLAR       0
#define IQ_INVERTED     0


/* demod&tuner address */
#define MT_FE_DMD_DEV_ADDR_RS6K             0xd2        /* Demodulator address    */


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



/*****************  CHECK CARRIER OFFSET DEFINEs  *********************
**    This define is used to unlock Sym4/6 SS2 if the carrier offset
** is larger than the defined carrier offset.
**      User can set this define to "1" and set the carrier offset
** (6MHz offset as default).
**      If the carrier offset if larger than the defined value, Sym4/6 SS2
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



//#define TMP1   ((AVE_TIMES/32 - 1)*16 + (DATA_LENGTH/128 - 1))  // = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
//#define TMP2   (64 + PSD_OVERLAP/16)                            // = (64 + 96/16) = (64 + 6) = 0x46
//#define TMP3   (PSD_SCALE*32)                                   // = (0*32) = 0
//#define TMP4   (NOTCH_RANGE_F*16 + START_RANGE_F)               // = (8*16 + 15) = 0x8F


typedef struct _MT_FE_UNICABLE_DEVICE
{
    U32     freq_MHz;
    MT_BOOL    UB_ok;

    U32     result_MHz;
    U32     result_PSD;
    U32     result_gain;
} MT_FE_UNICABLE_DEVICE;


#define TUNER_RFBYPASS_ON       1       /* If you need loop through function, please set it to 1 , else set it to 0 */
#define TUNER_CKOUT_ON          0       /* If you need clock out from tuner clkout pin to demodulator use, please set it to 1 , else set it 0*/
#define TUNER_CKOUT_DIV         0x01    /* The clock out division */
#define TUNER_CKOUT_XTAL        1       /* If you need clock out from tuner xtalout pin to demodulator use, please set it to 1 , else set it 0*/



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

#define DS6113_FILTER_MAX_DEPTH     15

/**scid Filter attribute*/
/**CNcomment: SCID 过滤器属性*/
typedef struct _MT_FE_DS6113_DSS_SCID_FILTER_T
{
    U8 tuner_id;
    MT_BOOL b_filter_mode;    /**<b_filter_mode. 1:match pattern to filter ;  0:match pattern discard*/ /**< CNcomment: 1 : 匹配就过滤器 0: 匹配就丢弃， 不能同时使用*/
    U16  u16_scid[DS6113_FILTER_MAX_DEPTH];     /**<Matched bytes of a filter */ /**< CNcomment:过滤器匹配字节*/
    U16  u16_mask[DS6113_FILTER_MAX_DEPTH];   /**<Masked bytes of a filter. The conditions are set by bit. 0: no mask. Comparison is required. 1: mask. Comparison is not required.*/ /**< CNcomment:过滤器屏蔽字节,按bit设置, 0:没有mask，要进行比较, 1:mask起作用，不进行比较*/
} MT_FE_DS6113_DSS_SCID_FILTER_T;



#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_CS8800_S_S2_H__ */


