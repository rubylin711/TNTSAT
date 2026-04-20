/********************************************************************************************/
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2016 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2014                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_def_ct8k.h
*
* Current version:		00.04
*
* Description:
*
* Log:	Description		Version		Date		Author
*----------------------------------------------------------------------------
*		Create			00.01		2018.06.27	BJ.Wang
*		Modify			00.01		2018.06.28	BJ.Wang
*		Modify			00.02		2018.09.25	BJ.Wang
*		Modify			00.03		2018.10.17	BJ.Wang
*		Modify			00.04		2020.09.28	YZ.Huang
*****************************************************************************/
#ifndef __MT_FE_DEF_CT8K_H__
#define __MT_FE_DEF_CT8K_H__

//#include <stdio.h>
#include "mt_fe_common.h"


#define MT_FE_DMD_DVBC_SUPPORT				1
#define MT_FE_DMD_J83B_SUPPORT				1
#define MT_FE_DMD_DVBS_S2_SUPPORT			1


#define MT_FE_CFG_CUSTOMER_SELECT			0	// 0: Public	1: Vestel


#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_ct8k_S_S2.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MT_FE_CT8K_DRIVER_VERSION						0x00000
#define MT_FE_CT8K_DEBUG								1			/*	0 off, 1 on									*/

#define DVBT2_STANDARD	13

#define MT_FE_CT8K_DEVICE_I2C_ADDRESS				0x18		// 0x18, 0x1C, 0x1E
	// system I2C address			0x18		0x1C		0x1E
	// DVB-T2 module address		0x18		0x1C		0x1E
	// DVB-C&T module address		0x38		0x3A		0x3A
	// DVB-S&S2 module address		0xD0		0xD4		0xD6


// Abstract value of delta a and b
#define MT_ABS(a, b)	(((a) >= (b)) ? ((a) - (b)) : ((b) - (a)))


typedef struct _MT_FE_DVBCTT2_INPUT_SETTINGS
{
	MT_FE_BANDWIDTH		demod_bandwidth;
	U32					input_freq_kHz;
	U16					symbol_rate_KSs;
	U16					qam;
	U8					inverted;
	U8					plp_No;
} MT_FE_DVBCTT2_INPUT_SETTINGS;

typedef enum _MT_FE_T2_PILOT
{
	MtFePilot_UnDefine,
	MtFePilot_PP1,
	MtFePilot_PP2,
	MtFePilot_PP3,
	MtFePilot_PP4,
	MtFePilot_PP5,
	MtFePilot_PP6,
	MtFePilot_PP7,
	MtFePilot_PP8
} MT_FE_T2_PILOT;

typedef enum _MT_FE_T2_GUARD_INTERVAL
{
	MtFeGuarInt_Undef = 0,
	MtFeGuarInt_1P4,
	MtFeGuarInt_1P8,
	MtFeGuarInt_1P16,
	MtFeGuarInt_1P32,
	MtFeGuarInt_1P128,
	MtFeGuarInt_19P128,
	MtFeGuarInt_19P256
} MT_FE_T2_GUARD_INTERVAL, MT_FE_T_GUARD_INTERVAL;

typedef enum _MT_FE_T2_PLP_TYPE
{
	MtFeT2PlpDataType_Common     = 0,
	MtFeT2PlpDataType_Type1      = 1,
	MtFeT2PlpDataType_Type2      = 2,
	MtFeT2PlpDataType_Reserved
} MT_FE_T2_PLP_TYPE;

typedef enum _MT_FE_T2_PLP_PAYLOAD_TYPE
{
	MtFeT2PlpPayloadType_GFPS        = 0,
	MtFeT2PlpPayloadType_GCS         = 1,
	MtFeT2PlpPayloadType_GSE         = 2,
	MtFeT2PlpPayloadType_TS          = 3,
	MtFeT2PlpPayloadType_Reserved
} MT_FE_T2_PLP_PAYLOAD_TYPE;

typedef enum _MT_FE_T2_PLP_FEC_TYPE
{
	MtFeT2PlpFecTypeLdpc_16K       = 0,
	MtFeT2PlpFecTypeLdpc_64K       = 1,
	MtFeT2PlpFecType_Reserved
} MT_FE_T2_PLP_FEC_TYPE;

typedef enum _MT_FE_T2_PLP_MODE
{
	MtFeT2PlpMode_NotSpecified     = 0,
	MtFeT2PlpMode_NormalMode       = 1,
	MtFeT2PlpMode_HighEfficiency   = 2,
	MtFeT2PlpMode_Reserved         = 3
} MT_FE_T2_PLP_MODE;


typedef struct _MT_FE_T2_PLP_INFO
{
	U8							PLP_ID;              // :  8;
	MT_FE_T2_PLP_TYPE			PLP_TYPE;            // :  3;
	MT_FE_T2_PLP_PAYLOAD_TYPE	PLP_PAYLOAD_TYPE;    // :  5;
	U8							FF_FLAG;             // :  1;
	U8							FIRST_RF_IDX;        // :  3;
	U8							FIRST_FRAME_IDX;     // :  8;
	U8							PLP_GROUP_ID;        // :  8;
	MT_FE_CODE_RATE				PLP_COD;             // :  3;
	MT_FE_MOD_MODE				PLP_MOD;             // :  3;
	U8							PLP_ROTATION;        // :  1;
	MT_FE_T2_PLP_FEC_TYPE		PLP_FEC_TYPE;        // :  2;
	U16							PLP_NUM_BLOCKS_MAX;  // : 10;
	U8							FRAME_INTERVAL;      // :  8;
	U8							TIME_IL_LENGTH;      // :  8;
	U8							TIME_IL_TYPE;        // :  1;
	U8							IN_BAND_A_FLAG;      // :  1;
	U8							IN_BAND_B_FLAG;      // :  1;
	U16							RESERVED_1;          // : 11;
	MT_FE_T2_PLP_MODE			PLP_MODE;            // :  2;
	U8							STATIC_FLAG;         // :  1;
	U8							STATIC_PADDING_FLAG; // :  1;
} MT_FE_T2_PLP_INFO;

typedef struct _MT_FE_T2_PLP_LIST
{
	U8					S2;
	U8					NUM_RF;

	U8					NUM_PLP;
	U8					NUM_AUX;
	U8					AUX_CONFIG_RFU;

	//U16					code_index;
	//U16					code_size;

	U16					plp_cnt;
	U16					plp_max;
	MT_FE_T2_PLP_INFO*	plp_info;
} MT_FE_T2_PLP_LIST;

typedef struct _MT_FE_T2_PLP_L1_BITBUF_T
{
	U32		bbuf;
	U8		bits_left;
	U8		data;
	U16		index;
	U16		size;
} MT_FE_T2_PLP_L1_BITBUF_T;

typedef struct _MT_FE_T2_TPS_INFO
{
	MT_FE_MOD_MODE			t2_qam;
	MT_FE_FFT				t2_fft;
	MT_FE_T2_GUARD_INTERVAL	t2_guard;
	MT_FE_T2_PILOT			t2_pp;
	MT_FE_CODE_RATE			t2_code;
} MT_FE_T2_TPS_INFO;

typedef struct _MT_FE_T_TPS_INFO
{
	MT_FE_MOD_MODE			t_qam;
	MT_FE_FFT				t_fft;
	MT_FE_T_GUARD_INTERVAL	t_guard;
	MT_FE_CODE_RATE			t_code;
} MT_FE_T_TPS_INFO;

typedef enum _MT_FE_DEMOD_MODE
{
	MtFeDemodMode_Undef = 0,
	MtFeDemodMode_DVBCTT2,
	MtFeDemodMode_DVBSS2,
} MT_FE_DEMOD_MODE;

typedef enum _MT_FE_CT8K_SUPPORTED_TUNER
{
	MtFeTnId_Undef = 0,
	/*DVB-C, T, T2*/
	MtFeTN_MxL603,
	MtFeTN_TC3800,
	MtFeTN_TC6800,

	/*DVB-S, S2*/
	MtFeTn_TS2022,
	MtFeTn_TS6011,
	MtFeTn_RDA5815M,
	MtFeTn_GeneralTuner = 0xFFFFFFFF
} MT_FE_CT8K_SUPPORTED_TUNER;

typedef enum _MT_FE_CT8K_TUNER_TC3800_XTAL
{
	MtFeTN_TC3800_XTAL_24M = 24000,
	MtFeTN_TC3800_XTAL_27M = 27000
} MT_FE_CT8K_TUNER_TC3800_XTAL;

typedef enum _MT_FE_CT8K_TUNER_TC6800_XTAL
{
	MtFeTN_TC6800_XTAL_24M = 24000,
	MtFeTN_TC6800_XTAL_27M = 27000
} MT_FE_CT8K_TUNER_TC6800_XTAL;

typedef enum _MT_FE_CT8K_TUNER_TC3800_LOOP
{
	MtFeTN_TC3800_LT_OFF = 0,
	MtFeTN_TC3800_LT_ON = 1,
	MtFeTN_TC3800_LT_ON_ALWAYS = 2
} MT_FE_CT8K_TUNER_TC3800_LOOP;

typedef struct _MT_FE_CT8K_TN_DEV_SETTINGS
{
	MT_FE_CT8K_SUPPORTED_TUNER	tuner_type;
	U8							tuner_dev_addr;					/* tuner device i2c addres*/
	U8							tuner_init_ok;					/*tuner init yes or no,0 :no 1:yes*/
	U8							tuner_open;						/*tuner open yes or no,0 :no 1:yes*/
	U8							tuner_lna_type;
	U32							tuner_handle;
	MT_FE_RET (*tuner_init)(void *handle);						/* init tuner function */
	MT_FE_RET (*tuner_set)(void *handle, U32 freq, U32 sym_rate, S16 lpf_offset);				/* set tuner freq function */
	MT_FE_RET (*tuner_strength)(void *handle, S8 *p_strength);	/* get signal strength function */
	MT_FE_RET (*tuner_sleep)(void *handle);						/* set tuner sleep function */
	MT_FE_RET (*tuner_wakeup)(void *handle);					/* set tuner wake function */
	MT_FE_RET (*tuner_get_offset)(void *handle, S32 *p_offset);	/* get tuner offset function */
	MT_FE_RET (*tuner_get_gain)(void *handle, U32 *p_gain);		/* get tuner gain function */
	MT_FE_RET (*tuner_set_application)(void * handle, U8 user_app);
	MT_FE_RET (*tuner_set_loop_through)(void *handle, S16 en_1oopthrough);
	//MT_FE_RET (*tuner_loop_add_tc)(void *handle, U8 tc3800_1oop_add);
	MT_FE_RET (*tuner_set_xtal)(void *handle, U32 xtal_khz);
	//MT_FE_RET (*tuner_im_tc3800_tc)(void *handle, U8 tc3800_im);
	MT_FE_RET (*tuner_set_clkout)(void *handle, U8 en_clockout);
	MT_FE_RET (*tuner_get_diagnose_info)(void *handle, U32* data1, U32* data2);
} MT_FE_CT8K_TN_DEV_SETTINGS, *MT_FE_CT8K_Tuner_Handle;

typedef struct _MT_FE_CT8K_CELL_ID_INFO
{
	MT_BOOL		bHighByteOk;
	MT_BOOL		bLowByteOk;
	U16			usCellId;
} MT_FE_CT8K_CELL_ID_INFO;


typedef struct _MT_FE_TP_PARAMS_CTT2_CT8K
{
	U32		freq_KHz;
	U16		sym_KSs;
	U16		qam_code;
	MT_BOOL	inverted;
	U32		param_reserved;
} MT_FE_TP_PARAMS_CTT2_CT8K;

typedef struct _MT_FE_BOARD_SETTINGS_CTT2_CT8K
{
	MT_BOOL	bSingleXTAL;
	MT_BOOL	bReadCali;
	U8		iVppSel;			// 0: 2Vpp, 1: 1Vpp, select Vpp of DVB-T/T2
	U8		iVppSelC;			// 0: 2Vpp, 1: 1Vpp, select Vpp of DVB-C/J83.B		// fix 105076
	U8		iGainStep;			// 0: default, 1: FY
	U32		xtal_KHz;
	U32		chip_version;
	U32		on_board_reserved;
} MT_FE_BOARD_SETTINGS_CTT2_CT8K;

typedef struct _MT_FE_SETTING_CTT2_CT8K
{
	MT_BOOL								active_edge_clk;		/*selects the active edge of pin M_CKOUT to update the MPEG-TS output.
																	0: active at falling edge		1: active at rising edge*/
	MT_BOOL								polar_sync;				/*define the output polarity of pin M_SYNC
																	0: pin M_SYNC is active high ;	1: pin M_SYNC is active low*/
	MT_BOOL								polar_val;				/*define the output polarity of pin M_VAL
																	0: pin M_VAL is active high ;	1: pin M_VAL is active low*/
	MT_BOOL								polar_err;				/*define the output polarity of pin M_ERR
																	0: pin M_ERR is active high ;	1: pin M_ERR is active low*/
	MT_FE_TS_OUT_MODE					output_mode;			/*parallel interface, serial interface, common  interface*/
	MT_FE_TS_OUT_MAX_CLOCK				output_clock;
	U8									custom_duty_cycle;		/* User defined ducy cycle of output clock. tmp1 = nibble[7:4], tmp2 = nibble[3:0] */
																/* If enable auto adjusting function, this variable defines the minimum divide ratio */
	MT_BOOL								serial_pin_select;		/*in the serial output mode only, select data output pin
																	0: select pin D0;	1: select pin D7*/
	MT_BOOL								output_high_Z;			/*TS output high-impedance status
																	0: disable high-Z, normal output;	1: enable high-Z, no output*/
} MT_FE_TS_SETTINGS_CTT2_CT8K;

typedef struct _MT_FE_CTT2_DEVICE_SETTINGS
{
	MT_FE_TYPE						demod_type;
	MT_FE_TYPE						demod_current_type;
	U8								dmd_dev_addr;		/* demod device i2c address*/
	MT_FE_TS_OUT_MODE				ts_out_mode;
	U8								m_iPageNo;			/* J83.B register page number */
	U8								mcu_status;
	MT_BOOL							bCalibrationOK;
	MT_FE_DVBCTT2_INPUT_SETTINGS	input_params;

	MT_FE_TP_PARAMS_CTT2_CT8K		tp_cfg;
	MT_FE_BOARD_SETTINGS_CTT2_CT8K	board_cfg;
	MT_FE_TS_SETTINGS_CTT2_CT8K		ts_cfg;


	MT_FE_CT8K_TN_DEV_SETTINGS		tuner_cfg;

	MT_FE_CT8K_CELL_ID_INFO			cell_info;
	MT_FE_T2_PLP_L1_BITBUF_T		L1_bitbuf;
	U8								calibration_data[28];
} MT_FE_CTT2_DEVICE_SETTINGS, *MT_FE_CTT2_Device_Handle;

typedef struct _MT_FE_SS2_DEVICE_SETTINGS
{
	MT_FE_TYPE						demod_type;
	MT_FE_TYPE						demod_current_type;
#if MT_FE_DMD_DVBS_S2_SUPPORT
	U8								demod_dev_addr;		/* demod device i2c address*/
	MT_FE_TS_OUT_MODE				ts_out_mode;
	U8								mcu_status;
	MT_FE_DVBSS2_INPUT_SETTINGS		input_params;
	MT_FE_BOARD_SETTINGS_SS2_CT8K	board_cfg;			/* Board settings */
	MT_FE_TS_SETTINGS_SS2_CT8K		ts_cfg;				/* TS settings */
	MT_FE_LNB_SETTINGS_SS2_CT8K		lnb_cfg;			/* LNB settings */
	MT_FE_GLOBAL_SETTINGS_SS2_CT8K	global_cfg;			/* Global settings */
	MT_FE_BS_SETTINGS_SS2_CT8K		bs_cfg;
	MT_FE_TP_PARAMS_SS2_CT8K		tp_cfg;				/* Channel parameters */

	MT_FE_CT8K_TN_DEV_SETTINGS		tuner_cfg;
#endif
} MT_FE_SS2_DEVICE_SETTINGS, *MT_FE_SS2_Device_Handle;

typedef struct _MT_FE_CT8K_DEVICE_SETTINGS
{
	U8								sys_dev_addr;		/* 2-wire bus device address */
	U8								sar_dev_addr;
	U16								chip_version;
	MT_FE_XTAL						sys_dev_xtal;
	MT_BOOL							bSysInitOk;
	MT_BOOL							bTunerBusOn;		/* 2-wire bus repeater for tuner is ON or not */
	MT_BOOL							bSupportDualOutput;	/* Support S + C or S + B dual TS output or not */    // fix 112220
	U8								mode_select;		/* bit 0~4, indicates mode 1~5 */
	U8								custom_config_sel;	/* Custom configuration select, 0: General; 1: Customer Config 1; 2: ... */
	U32								ulT2ShareMemAddr;	/* Shared memory address for DVB-T2 */
	MT_FE_CTT2_DEVICE_SETTINGS		m_device_ctt2;		/* Main module instance, include DVB-T2, DVB-T, DVB-C and J83.B */
	MT_FE_SS2_DEVICE_SETTINGS		m_device_ss2;		/* Aux module instance, include DVB-S & DVB-S2 or DVB-S2X */

	MT_FE_RET	(*Set32Bits)(U32 reg_addr, U32 reg_data);
	MT_FE_RET	(*Get32Bits)(U32 reg_addr, U32 *p_data);
} MT_FE_CT8K_DEVICE_SETTINGS, *MT_FE_CT8K_Device_Handle;

MT_FE_RET mt_fe_get_driver_version_ct8k(U8 *pd_ver);
MT_FE_RET mt_fe_dmd_ct8k_config_default(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_system_init_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_close_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_clock_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type);

MT_FE_RET _mt_fe_dmd_re_calibration_ct8k_ct(MT_FE_CT8K_Device_Handle handle, U8 *p_buf);
MT_FE_RET mt_fe_dmd_calibration_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_calibration_ct8k_ct(MT_FE_CT8K_Device_Handle handle, U8 iMaxTimes);
MT_FE_RET mt_fe_dmd_config_default_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_CT8K_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_open_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_close_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_hard_reset_ct8k(void);
MT_FE_RET mt_fe_dmd_set_output_mode_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_set_bw_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_per_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt);
MT_FE_RET mt_fe_dmd_get_snr_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_snr);
MT_FE_RET mt_fe_dmd_get_quality_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_quality_nordig_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_strength_ctt2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_get_inform_t2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_T2_TPS_INFO *tps_info);
MT_FE_RET mt_fe_dmd_select_plp_ct8k_t2(MT_FE_CT8K_Device_Handle handle, U8 plp_id);
MT_FE_RET mt_fe_dmd_get_plp_num_ct8k_t2(MT_FE_CT8K_Device_Handle handle, U8 *plp_num);
MT_FE_RET mt_fe_auto_QAM_debug_ct8k_c(MT_FE_CT8K_Device_Handle handle, U8 auto_mode, U16 *auto_qam, U32 *auto_symbol, S32 *auto_offset);
MT_FE_RET mt_fe_dmd_get_inform_t_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_T_TPS_INFO *tps_info);
MT_FE_RET mt_fe_dmd_set_hierarchy_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 hi_id);
MT_FE_RET mt_fe_dmd_get_hierarchy_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *hi_num);
MT_FE_RET mt_fe_dmd_sleep_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_wake_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_fec_reset_ct8k_t2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_clear_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_reset_cci_ct8k_t(MT_FE_CT8K_Device_Handle handle, U8 *reset_cci);
MT_FE_RET _mt_fe_dmd_get_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack);
MT_FE_RET _mt_fe_dmd_get_mean_error_pack_ct8k_t(MT_FE_CT8K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack);
MT_FE_RET mt_fe_dmd_get_offset_ct8k_c(MT_FE_CT8K_Device_Handle handle, S32 *freq_offset_KHz, S32 *symbol_rate_offset_KSs);
MT_FE_RET mt_fe_dmd_get_symbol_rate_ct8k_c(MT_FE_CT8K_Device_Handle handle, U16 *symbol_rate_KSs);

MT_FE_RET mt_fe_dmd_get_all_plp_info_ct8k_t2(MT_FE_CT8K_Device_Handle handle, MT_FE_T2_PLP_LIST* plp_list);

MT_FE_RET _mt_fe_dmd_soft_reset_ct8k_b(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_frames_ct8k_b(MT_FE_CT8K_Device_Handle handle, U8 *good_frames, U8 *bad_frames, U8 *total_frames);

#if MT_FE_DMD_DVBS_S2_SUPPORT
MT_FE_RET mt_fe_dmd_calibration_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_wake_up_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_ss2(U8* p_version);
MT_FE_RET mt_fe_dmd_ct8k_ss2_config_default(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_default_ss2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_CT8K_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_close_ss2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_open_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_soft_reset_ss2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_ss2_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_quality_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_strength_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_blindscan_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_set_LNB_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_tone_burst_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET mt_fe_dmd_set_OLF_ct8k(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_OLF_enable);
MT_FE_RET mt_fe_dmd_get_OLF_status_ct8k(MT_FE_CT8K_Device_Handle handle, U8 *p_status);

MT_FE_RET mt_fe_dmd_connect_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type);
MT_FE_RET _mt_fe_dmd_get_sym_rate_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_get_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_fec_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);
MT_FE_RET mt_fe_dmd_get_snr_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S8 *p_snr);
MT_FE_RET mt_fe_dmd_get_strength_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_dmd_get_sat_quality_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 *p_percent);
MT_FE_RET _mt_fe_dmd_get_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *p_MCLK_KHz);
MT_FE_RET mt_fe_dmd_get_lock_state_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_pure_lock_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_init_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_bs_set_reg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 bs_times);
MT_FE_RET mt_fe_dmd_blindscan_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_set_LNB_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_tone_burst_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);


MT_FE_RET _mt_fe_dmd_bs_connect_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info);

MT_FE_RET mt_fe_dmd_get_channel_info_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info);

MT_FE_RET mt_fe_dmd_set_ts_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 ucTsId);	// fix 105341
MT_FE_RET mt_fe_dmd_clear_ts_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_BOOL _mt_fe_dmd_check_MIS_ct8k_ss2(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_per_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags);


MT_FE_RET mt_fe_tn_init_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts2022_ct8k(MT_FE_CT8K_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts6011_ct8k(MT_FE_CT8K_Device_Handle handle);

#if 1
MT_FE_RET mt_fe_tn_init_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_gain_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_sleep_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_RDA5815M_ct8k(MT_FE_CT8K_Device_Handle handle);
#else
MT_FE_RET mt_fe_tn_init_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_gain_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_sleep_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_RDA5815S_ct8k(MT_FE_CT8K_Device_Handle handle);
#endif
#endif

MT_FE_RET mt_fe_tn_init_MxL603_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_MxL603_ct8k(MT_FE_CT8K_Device_Handle handle, U32 freq, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_MxL603_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_MxL603_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_MxL603_ct8k(MT_FE_CT8K_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32 Freq_KHz, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_application_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc3800_application);
MT_FE_RET mt_fe_tn_loop_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, S16 tc3800_1oop);
MT_FE_RET mt_fe_tn_loop_add_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc3800_1oop_add);
MT_FE_RET mt_fe_tn_xtal_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32 xtal_khz);
MT_FE_RET mt_fe_tn_im_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc3800_im);
MT_FE_RET mt_fe_tn_clkout_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc3800_clock);
void mt_fe_tn_get_diagnose_info_tc3800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32* data1, U32* data2);
void mt_fe_tn_set_tc3800_dbg_level_ct8k(U32 tc3800_dbg_level);
void mt_fe_tn_get_tc3800_dbg_level_ct8k(U32 *tc3800_dbg_level);


MT_FE_RET mt_fe_tn_init_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32 Freq_KHz, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle);
MT_FE_RET mt_fe_tn_application_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc6800_application);
MT_FE_RET mt_fe_tn_loop_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, S16 tc6800_1oop);
MT_FE_RET mt_fe_tn_loop_add_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc6800_1oop_add);
MT_FE_RET mt_fe_tn_xtal_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32 xtal_khz);
MT_FE_RET mt_fe_tn_im_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc6800_im);
MT_FE_RET mt_fe_tn_clkout_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U8 tc6800_clock);
void mt_fe_tn_get_diagnose_info_tc6800_tc_ct8k(MT_FE_CT8K_Device_Handle handle, U32* data1, U32* data2);
void mt_fe_tn_set_tc6800_dbg_level_ct8k(U32 tc6800_dbg_level);
void mt_fe_tn_get_tc6800_dbg_level_ct8k(U32 *tc6800_dbg_level);
MT_FE_RET mt_fe_tn_get_gain_tc6800_ct8k(MT_FE_CT8K_Device_Handle handle, U32 *p_gain);

#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_T2_DEF_H__ */
