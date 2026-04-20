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
* File:				mt_fe_def_cs8800.h
*
* Current version:		00.03
*
* Description:
*
* Log:	Description		Version		Date		Author
*----------------------------------------------------------------------------
*		Create			00.01		2018.06.27	BJ.Wang
*		Modify			00.01		2018.06.28	BJ.Wang
*		Modify			00.02		2018.09.25	BJ.Wang
*		Modify			00.03		2018.10.17	BJ.Wang
*****************************************************************************/
#ifndef __MT_FE_DEF_CS8800_H__
#define __MT_FE_DEF_CS8800_H__

#include "mt_fe_common.h"

#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#include <linux/spinlock.h>
#else  //ucos
#include <stdio.h>
#define MT_BOOL   BOOL
#define mt_handle int
#endif

#define MT_FE_DMD_DVBC_SUPPORT				1
#define MT_FE_DMD_J83B_SUPPORT				1
#define MT_FE_DMD_DVBS_S2_SUPPORT			1


#define MT_FE_CFG_CUSTOMER_SELECT			0	// 0: Public	1: Vestel


#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_cs8800_S_S2.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MT_FE_CS8800_DRIVER_VERSION						0x00000
#define MT_FE_CS8800_DEBUG								1			/*	0 off, 1 on									*/


#define MT_FE_CS8800_DEVICE_I2C_ADDRESS				0x18		// 0x18, 0x1C, 0x1E
	// system I2C address			0x18		0x1C		0x1E
	// SAR module address			0x80
	// DVB-C module address			0x38		0x3A		0x3A
	// J83B module address			0xb8		0x3A		0x3A
	// DVB-S&S2 module address		0xD0		0xD4		0xD6


typedef struct _MT_FE_DVBCTT2_INPUT_SETTINGS
{
	MT_FE_BANDWIDTH		demod_bandwidth;
	U32					input_freq_kHz;
	U16					symbol_rate_KSs;
	U16					qam;
	U8					inverted;
	U8					plp_No;
} MT_FE_DVBCTT2_INPUT_SETTINGS;

typedef enum _MT_FE_DEMOD_MODE
{
	MtFeDemodMode_Undef = 0,
	MtFeDemodMode_DVBCTT2,
	MtFeDemodMode_DVBSS2,
} MT_FE_DEMOD_MODE;

typedef enum _MT_FE_CS8800_SUPPORTED_TUNER
{
	MtFeTnId_Undef = 0,
	/*DVB-C, J83B*/
	MtFeTN_MxL603,
	MtFeTN_TC3800,
	MtFeTN_TC6800,

	/*DVB-S, S2*/
	MtFeTn_TS2022,
	MtFeTn_TS6011,
	MtFeTn_RT720,
	MtFeTn_GeneralTuner = 0xFFFFFFFF
} MT_FE_CS8800_SUPPORTED_TUNER;

typedef enum _MT_FE_CS8800_TUNER_TC3800_XTAL
{
	MtFeTN_TC3800_XTAL_24M = 24000,
	MtFeTN_TC3800_XTAL_27M = 27000
} MT_FE_CS8800_TUNER_TC3800_XTAL;

typedef enum _MT_FE_CS8800_TUNER_TC6800_XTAL
{
	MtFeTN_TC6800_XTAL_24M = 24000,
	MtFeTN_TC6800_XTAL_27M = 27000
} MT_FE_CS8800_TUNER_TC6800_XTAL;

typedef enum _MT_FE_CS8800_TUNER_TC3800_LOOP
{
	MtFeTN_TC3800_LT_OFF = 0,
	MtFeTN_TC3800_LT_ON = 1,
	MtFeTN_TC3800_LT_ON_ALWAYS = 2
} MT_FE_CS8800_TUNER_TC3800_LOOP;

typedef struct _MT_FE_CS8800_TN_DEV_SETTINGS
{
	MT_FE_CS8800_SUPPORTED_TUNER	tuner_type;
	U8							tuner_dev_addr;					/* tuner device i2c addres*/
	U8							tuner_init_ok;					/*tuner init yes or no,0 :no 1:yes*/
	U8							tuner_open;						/*tuner open yes or no,0 :no 1:yes*/
	U8							tuner_lna_type;
	mt_handle					tuner_handle;
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
} MT_FE_CS8800_TN_DEV_SETTINGS, *MT_FE_CS8800_Tuner_Handle;

typedef struct _MT_FE_TP_PARAMS_CTT2_CS8800
{
	U32		freq_KHz;
	U16		sym_KSs;
	U16		qam_code;
	MT_BOOL	inverted;
	U32		param_reserved;
} MT_FE_TP_PARAMS_CTT2_CS8800;

typedef struct _MT_FE_BOARD_SETTINGS_CTT2_CS8800
{
	MT_BOOL	bSingleXTAL;
	MT_BOOL	bReadCali;
	U8		iVppSel;			// 0: 2Vpp, 1: 1Vpp, select Vpp of DVB-T/T2
	U8		iVppSelC;			// 0: 2Vpp, 1: 1Vpp, select Vpp of DVB-C/J83.B
	U8		iGainStep;			// 0: default, 1: FY
	U32		xtal_KHz;
	U32		chip_version;
	U32		on_board_reserved;
} MT_FE_BOARD_SETTINGS_CTT2_CS8800;

typedef enum _MT_FE_CONNECT_MODE_CS8800
{
	MT_FE_CS8800_CONNECT_MODE_NORMAL			 = 0,	// normal connect mode, set tuner & demod
	MT_FE_CS8800_CONNECT_MODE_SET_TUNER_ONLY	 = 1,	// set tuner only
	MT_FE_CS8800_CONNECT_MODE_SET_DEMOD_ONLY	 = 2,	// set demod only
	MT_FE_CS8800_CONNECT_MODE_UNDEF				 = 0xFF
} MT_FE_CONNECT_MODE_CS8800;

typedef struct _MT_FE_SETTING_CTT2_CS8800
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
} MT_FE_TS_SETTINGS_CTT2_CS8800;

typedef struct _MT_FE_CTT2_DEVICE_SETTINGS
{
	MT_FE_TYPE							demod_type;
	MT_FE_TYPE							demod_current_type;
	//U8								dmd_dev_addr;		/* demod device i2c address*/
	MT_FE_TS_OUT_MODE					ts_out_mode;
	U8									m_iPageNo;			/* J83.B register page number */
	U8									mcu_status;
	MT_BOOL								bCalibrationOK;
	MT_FE_DVBCTT2_INPUT_SETTINGS		input_params;

	MT_FE_TP_PARAMS_CTT2_CS8800			tp_cfg;
	MT_FE_BOARD_SETTINGS_CTT2_CS8800	board_cfg;
	MT_FE_TS_SETTINGS_CTT2_CS8800		ts_cfg;


	MT_FE_CS8800_TN_DEV_SETTINGS		tuner_cfg;

	U8								calibration_data[28];
} MT_FE_CTT2_DEVICE_SETTINGS, *MT_FE_CTT2_Device_Handle;

typedef struct _MT_FE_SS2_DEVICE_SETTINGS
{
	MT_FE_TYPE							demod_type;
	MT_FE_TYPE							demod_current_type;
#if MT_FE_DMD_DVBS_S2_SUPPORT
	//U8								demod_dev_addr;		/* demod device i2c address*/
	MT_FE_TS_OUT_MODE					ts_out_mode;
	U8									mcu_status;
	MT_FE_DVBSS2_INPUT_SETTINGS			input_params;
	MT_FE_BOARD_SETTINGS_SS2_CS8800		board_cfg;			/* Board settings */
	MT_FE_TS_SETTINGS_SS2_CS8800		ts_cfg;				/* TS settings */
	MT_FE_LNB_SETTINGS_SS2_CS8800		lnb_cfg;			/* LNB settings */
	MT_FE_GLOBAL_SETTINGS_SS2_CS8800	global_cfg;			/* Global settings */
	MT_FE_BS_SETTINGS_SS2_CS8800		bs_cfg;
	MT_FE_TP_PARAMS_SS2_CS8800			tp_cfg;				/* Channel parameters */

	MT_FE_CS8800_TN_DEV_SETTINGS		tuner_cfg;
#endif
} MT_FE_SS2_DEVICE_SETTINGS, *MT_FE_SS2_Device_Handle;

typedef struct _MT_FE_CS8800_DEVICE_SETTINGS
{
	U8								sar_dev_addr;
	U8								dvbc_dev_addr;
	U8								j83b_dev_addr;
	U8								dvbs_dev_addr;
	U16								chip_version;
	MT_FE_XTAL						sys_dev_xtal;

	MT_BOOL							bSysInitOk;
    MT_BOOL                         bDVBSInitOk;
    MT_BOOL                         bDVBCInitOk;
    MT_BOOL                         bJ83BInitOk;

	MT_FE_CONNECT_MODE_CS8800		connect_mode;		/* Connect mode -- 1: set tuner only; 2: set demod only; 0 or others: normal mode, set tuner & demod */
	U8								custom_config_sel;	/* Custom configuration select, 0: General; 1: Customer Config 1; 2: ... */
														/* bit[3:0] for S/S2 module, bit[7:4] for B/C/T/T2 module */
	MT_FE_CTT2_DEVICE_SETTINGS		m_device_c_b;		/* Main module instance, include DVB-T2, DVB-T, DVB-C and J83.B */
	MT_FE_SS2_DEVICE_SETTINGS		m_device_ss2;		/* Aux module instance, include DVB-S & DVB-S2 or DVB-S2X */
	spinlock_t 						blindscan_status_slock;

	MT_FE_RET	(*Set32Bits)(U32 reg_addr, U32 reg_data);
	MT_FE_RET	(*Get32Bits)(U32 reg_addr, U32 *p_data);
} MT_FE_CS8800_DEVICE_SETTINGS, *MT_FE_CS8800_Device_Handle;

MT_FE_RET mt_fe_get_driver_version_cs8800(U8 *pd_ver);
MT_FE_RET mt_fe_dmd_cs8800_config_default(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_system_init_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_close_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_clock_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_config_adc_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET _mt_fe_dmd_re_calibration_cs8800_c_b(MT_FE_CS8800_Device_Handle handle, U8 *p_buf);
MT_FE_RET mt_fe_dmd_calibration_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_calibration_cs8800_c_b(MT_FE_CS8800_Device_Handle handle, U8 iMaxTimes);
MT_FE_RET mt_fe_dmd_config_default_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_CS8800_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_open_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_close_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_hard_reset_cs8800(void);
MT_FE_RET mt_fe_dmd_set_output_mode_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_set_bw_c_b_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_accurate_snr_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_snr);
MT_FE_RET mt_fe_dmd_get_snr_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_snr);
MT_FE_RET mt_fe_dmd_get_quality_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_quality_nordig_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_strength_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_get_per_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt);
MT_FE_RET mt_fe_auto_QAM_debug_cs8800_c(MT_FE_CS8800_Device_Handle handle, U8 auto_mode, U16 *auto_qam, U32 *auto_symbol, S32 *auto_offset);
MT_FE_RET mt_fe_dmd_sleep_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_wake_cs8800(MT_FE_CS8800_Device_Handle handle);

MT_FE_RET _mt_fe_dmd_get_snr_cs8800_b(MT_FE_CS8800_Device_Handle handle, U16 *signal_snr);

MT_FE_RET _mt_fe_dmd_soft_reset_cs8800_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_get_frames_cs8800_b(MT_FE_CS8800_Device_Handle handle, U8 *good_frames, U8 *bad_frames, U8 *total_frames);

#if MT_FE_DMD_DVBS_S2_SUPPORT
MT_FE_RET mt_fe_dmd_wake_up_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_driver_version_cs8800_ss2(U8* p_version);
MT_FE_RET mt_fe_dmd_cs8800_ss2_config_default(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_default_ss2_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_CS8800_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_close_ss2_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_open_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_soft_reset_ss2_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_ss2_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_quality_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_accurate_snr_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, S32 *p_snr);
MT_FE_RET mt_fe_dmd_get_snr_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, S8 *p_snr);
MT_FE_RET mt_fe_dmd_get_strength_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_blindscan_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_set_LNB_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_tone_burst_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET _mt_fe_dmd_bs_connect_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info);

int _mt_fe_dmd_cs8800_set_dss_scid_filter(MT_FE_CS8800_Device_Handle handle, MT_FE_DS6113_DSS_SCID_FILTER_T  *scid_filter);
#if 0
MT_FE_RET mt_fe_dmd_get_snr_dB_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, double *p_snr_dB);
#endif

MT_FE_RET mt_fe_dmd_connect_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type);
MT_FE_RET _mt_fe_dmd_get_sym_rate_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_get_carrier_offset_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_fec_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);
MT_FE_RET mt_fe_dmd_get_channel_info_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info);
MT_FE_RET mt_fe_dmd_get_strength_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_dmd_get_sat_quality_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_lock_state_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_pure_lock_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_fast_lock_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_init_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_carrier_offset_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_bs_set_reg_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U8 bs_times);
MT_FE_RET mt_fe_dmd_blindscan_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_set_LNB_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_tone_burst_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET mt_fe_dmd_set_ts_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U8 ucTsId);
MT_FE_RET mt_fe_dmd_clear_ts_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);
MT_BOOL _mt_fe_dmd_check_MIS_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_per_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags);
MT_FE_RET mt_fe_dmd_get_pre_ber_cs8800_ss2(MT_FE_CS8800_Device_Handle handle, U32 *p_err_bits, U32 *p_total_bits);

MT_FE_RET mt_fe_unicable_retry_cs8800_ss2(MT_FE_CS8800_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts6011_cs8800(MT_FE_CS8800_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_RT720_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_RT720_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_RT720_cs8800(MT_FE_CS8800_Device_Handle handle);
#endif

MT_FE_RET mt_fe_tn_init_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle, U32 freq, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_tc6800_c_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_tc6800_c_b(MT_FE_CS8800_Device_Handle handle, U32 Freq_KHz, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_tc6800_c_b(MT_FE_CS8800_Device_Handle handle,S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc6800_c_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_tc6800_c_b(MT_FE_CS8800_Device_Handle handle);
MT_FE_RET mt_fe_tn_application_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 tc6800_application);
MT_FE_RET mt_fe_tn_loop_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, S16 tc6800_1oop);
MT_FE_RET mt_fe_tn_loop_add_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 tc6800_1oop_add);
MT_FE_RET mt_fe_tn_xtal_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U32 xtal_khz);
MT_FE_RET mt_fe_tn_im_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 tc6800_im);
MT_FE_RET mt_fe_tn_clkout_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 tc6800_clock);
MT_FE_RET mt_fe_tn_get_diagnose_info_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U32* data1, U32* data2);
void mt_fe_tn_set_tc6800_dbg_level_c_b_cs8800(U32 tc6800_dbg_level);
void mt_fe_tn_get_tc6800_dbg_level_c_b_cs8800(U32 *tc6800_dbg_level);
MT_FE_RET mt_fe_tn_get_gain_tc6800_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_dmd_cs8800_set_gs_package_mode(MT_FE_CS8800_Device_Handle handle, U32 package_mode);
MT_FE_RET mt_fe_dmd_cs8800_get_ts_gs_mode(MT_FE_CS8800_Device_Handle handle, U32 *p_mode);
MT_FE_RET mt_fe_dmd_cs8800_gse_mode_for_test(MT_FE_CS8800_Device_Handle handle, U32 param);
#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_T2_DEF_H__ */
