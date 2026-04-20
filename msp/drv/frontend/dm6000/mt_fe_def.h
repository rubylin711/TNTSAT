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
* File:				mt_fe_def.h
*
* Current version:		00.70
*
* Description:
*
* Log:	Description		Version		Date		Author
*----------------------------------------------------------------------------
*		Create			00.10		2014.05.18	BJ.Wang
*		Modify			00.20		2015.07.15	BJ.Wang
*		Modify			00.70		2018.03.08	YZ.Huang
*****************************************************************************/
#ifndef __MT_FE_DEF_DM6K_H__
#define __MT_FE_DEF_DM6K_H__

//#include <stdio.h>
#include "mt_fe_common.h"


#define MT_FE_DMD_DVBC_SUPPORT				1
#define MT_FE_DMD_DVBS_S2_SUPPORT			1

#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_dm6k_S_S2.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MT_FE_DM6K_DRIVER_VERSION						0x00000
#define MT_FE_DM6K_DEBUG								1			/*	0 off, 1 on									*/

#define DVBT2_STANDARD	13

#define MT_FE_DM6K_DEVICE_I2C_ADDRESS				0x18		// 0x18, 0x1C, 0x1E
	// system I2C address			0x18		0x1C		0x1E
	// DVB-T2 module address		0x18		0x1C		0x1E
	// DVB-C&T module address		0x38		0x3A		0x3A
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

typedef enum _MT_FE_DM6K_SUPPORTED_TUNER
{
	MtFeTnId_Undef = 0,
	/*DVB-C, T, T2*/
	MtFeTN_MxL603,
	MtFeTN_TC3800,
	MtFeTN_TC6800,
	/*DVB-S, S2*/
	MtFeTn_TS2022,
	MtFeTn_TS6011,
	MtFeTn_GeneralTuner = 0xFFFFFFFF
} MT_FE_DM6K_SUPPORTED_TUNER;

typedef struct _MT_FE_DM6K_TN_DEV_SETTINGS
{
	MT_FE_DM6K_SUPPORTED_TUNER	tuner_type;
	U8							tuner_dev_addr;					/* tuner device i2c addres*/
	U8							tuner_init_ok;					/*tuner init yes or no,0 :no 1:yes*/
	U8							tuner_open;						/*tuner open yes or no,0 :no 1:yes*/
	U8							tuner_lna_type;
	MT_FE_RET (*tuner_init)(void *handle);						/* init tuner function */
	MT_FE_RET (*tuner_set)(void *handle, U32 freq, U32 sym_rate, S16 lpf_offset);				/* set tuner freq function */
	MT_FE_RET (*tuner_strength)(void *handle, S8 *p_strength);	/* get signal strength function */
	MT_FE_RET (*tuner_sleep)(void *handle);						/* set tuner sleep function */
	MT_FE_RET (*tuner_wakeup)(void *handle);					/* set tuner wake function */
	MT_FE_RET (*tuner_get_offset)(void *handle, S32 *p_offset);	/* get tuner offset function */
	MT_FE_RET (*tuner_get_gain)(void *handle, U32 *p_gain);		/* get tuner gain function */
} MT_FE_DM6K_TN_DEV_SETTINGS, *MT_FE_DM6K_Tuner_Handle;

typedef struct _MT_FE_DM6K_CELL_ID_INFO
{
	MT_BOOL		bHighByteOk;
	MT_BOOL		bLowByteOk;
	U16			usCellId;
} MT_FE_DM6K_CELL_ID_INFO;

typedef struct _MT_FE_CTT2_DEVICE_SETTINGS
{
	MT_FE_TYPE						demod_type;
	MT_FE_TS_OUT_MODE				ts_out_mode;
	U8								m_iSerialTSNo;
	U8								dmd_dev_addr;		/* demod device i2c addres*/
	U8								mcu_status;
	MT_FE_TYPE						demod_current_type;
	MT_FE_DVBCTT2_INPUT_SETTINGS	input_params;
	#if MT_FE_DMD_DVBC_SUPPORT
	U8								dvbc_chip_mode;// 0: new DC2800        1: new Jazz
	#endif
	MT_FE_DM6K_TN_DEV_SETTINGS		tuner_cfg;
	MT_FE_DM6K_CELL_ID_INFO			cell_info;
} MT_FE_CTT2_DEVICE_SETTINGS, *MT_FE_CTT2_Device_Handle;

typedef struct _MT_FE_SS2_DEVICE_SETTINGS
{
	MT_FE_TYPE						demod_type;
	MT_FE_TYPE						demod_current_type;
#if MT_FE_DMD_DVBS_S2_SUPPORT
	MT_FE_TS_OUT_MODE				ts_out_mode;
	U8								m_iSerialTSNo;
	U8								demod_dev_addr;		/* demod device i2c addres*/
	U8								mcu_status;
	MT_FE_DVBSS2_INPUT_SETTINGS		input_params;
	MT_FE_BOARD_SETTINGS_SS2_DM6K	board_cfg;			/* Board settings */
	MT_FE_TS_SETTINGS_SS2_DM6K		ts_cfg;				/* TS settings */
	MT_FE_LNB_SETTINGS_SS2_DM6K		lnb_cfg;			/* LNB settings */
	MT_FE_GLOBAL_SETTINGS_SS2_DM6K	global_cfg;			/* Global settings */
	//MT_FE_TP_PARAMS_SS2_DM6K		tp_cfg;				/* Channel parameters */

	MT_FE_DM6K_TN_DEV_SETTINGS		tuner_cfg;
#endif

} MT_FE_SS2_DEVICE_SETTINGS, *MT_FE_SS2_Device_Handle;

typedef struct _MT_FE_DM6K_DEVICE_SETTINGS
{
	U8								sys_dev_addr;		/* 2-wire bus device address */
	MT_FE_XTAL						sys_dev_xtal;
	MT_BOOL							bTunerBusOn;		/* 2-wire bus repeater for tuner is ON or not */
	MT_FE_CTT2_DEVICE_SETTINGS		m_device_ctt2;		/* Main module instance, include DVB-T2, DVB-T & DVB-C */
	MT_FE_SS2_DEVICE_SETTINGS		m_device_ss2;		/* Aux module instance, include DVB-S & DVB-S2 */
} MT_FE_DM6K_DEVICE_SETTINGS, *MT_FE_DM6K_Device_Handle;

MT_FE_RET mt_fe_get_driver_version_dm6k(U8 *pd_ver);
MT_FE_RET mt_fe_system_init_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_default_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DM6K_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_open_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_close_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_hard_reset_dm6k(void);
MT_FE_RET mt_fe_dmd_set_output_mode_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_set_bw_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_snr_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_snr);
MT_FE_RET mt_fe_dmd_get_quality_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_quality_nordig_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_strength_ctt2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_get_inform_t2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_T2_TPS_INFO *tps_info);
MT_FE_RET mt_fe_dmd_select_plp_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 plp_id);
MT_FE_RET mt_fe_dmd_get_plp_num_dm6k_t2(MT_FE_DM6K_Device_Handle handle, U8 *plp_num);
MT_FE_RET mt_fe_auto_QAM_debug_dm6k_c(MT_FE_DM6K_Device_Handle handle, U8 auto_mode, U16 *auto_qam, U32 *auto_symbol, S32 *auto_offset);
MT_FE_RET mt_fe_dmd_set_hierarchy_dm6k_t(MT_FE_DM6K_Device_Handle handle,U8 hi_id);
MT_FE_RET mt_fe_dmd_get_hierarchy_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *hi_num);
MT_FE_RET mt_fe_dmd_sleep_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_wake_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_i2c_repeat_enable_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_i2c_repeat_disable_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_fec_reset_dm6k_t2(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_clear_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_reset_cci_dm6k_t(MT_FE_DM6K_Device_Handle handle, U8 *reset_cci);
MT_FE_RET _mt_fe_dmd_get_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *total_pack, U32 *error_pack, U32 *corrected_pack);
MT_FE_RET _mt_fe_dmd_get_mean_error_pack_dm6k_t(MT_FE_DM6K_Device_Handle handle, U32 *total_pack,U32 *error_pack,U32 *corrected_pack);

#if MT_FE_DMD_DVBS_S2_SUPPORT
MT_FE_RET mt_fe_dmd_get_driver_version_dm6k_ss2(U8* p_version);
MT_FE_RET mt_fe_dmd_ss2_config_default(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_config_default_ss2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_select_tuner_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DM6K_SUPPORTED_TUNER tuner_type);
MT_FE_RET mt_fe_dmd_close_ss2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_open_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_TYPE demod_type);
MT_FE_RET mt_fe_dmd_soft_reset_ss2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_connect_ss2_dm6k(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_quality_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *p_percent);
MT_FE_RET mt_fe_dmd_get_strength_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U8 *ssi_percent);
MT_FE_RET mt_fe_dmd_blindscan_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET mt_fe_dmd_set_LNB_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_tone_burst_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_dm6k(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);

MT_FE_RET mt_fe_dmd_connect_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type);
MT_FE_RET _mt_fe_dmd_get_sym_rate_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_get_carrier_offset_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_fec_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);
MT_FE_RET mt_fe_dmd_get_strength_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_dmd_get_sat_quality_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U8 *p_percent);
MT_FE_RET _mt_fe_dmd_get_mclk_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U32 *p_MCLK_KHz);
MT_FE_RET mt_fe_dmd_get_lock_state_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_get_pure_lock_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_LOCK_STATE *p_state);
MT_FE_RET mt_fe_dmd_soft_reset_dm6k_ss2(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_dmd_init_dm6k_ss2(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_set_carrier_offset_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_bs_set_reg_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U8 bs_times);
MT_FE_RET mt_fe_dmd_blindscan_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info);
MT_FE_RET mt_fe_dmd_blindscan_abort_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL bs_abort);
MT_FE_RET  mt_fe_dmd_set_LNB_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode);
MT_FE_RET  mt_fe_dmd_DiSEqC_send_tone_burst_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode);
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg);
MT_FE_RET mt_fe_dmd_get_per_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags);
MT_FE_RET mt_fe_dmd_get_snr_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, S8 *p_snr);
MT_FE_RET _mt_fe_dmd_bs_connect_dm6k_ss2(MT_FE_DM6K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
									 U16 start_index,
									 U16 scanned_tp);
MT_FE_RET mt_fe_dmd_register_notify_dm6k_ss2(void (*callback)(MT_FE_MSG msg, void *p_tp_info));


MT_FE_RET mt_fe_tn_init_ts2022(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts2022(MT_FE_DM6K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_ts2022(MT_FE_DM6K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts2022(MT_FE_DM6K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_ts2022(MT_FE_DM6K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_ts2022(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts2022(MT_FE_DM6K_Device_Handle handle);

MT_FE_RET mt_fe_tn_init_ts6011(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_ts6011(MT_FE_DM6K_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET mt_fe_tn_get_strength_ts6011(MT_FE_DM6K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_get_tuner_freq_offset_ts6011(MT_FE_DM6K_Device_Handle handle, S32 *p_offset);
MT_FE_RET mt_fe_tn_get_gain_ts6011(MT_FE_DM6K_Device_Handle handle, U32 *p_gain);
MT_FE_RET mt_fe_tn_sleep_ts6011(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_ts6011(MT_FE_DM6K_Device_Handle handle);

#endif

MT_FE_RET mt_fe_tn_Init_MxL603(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_MxL603(MT_FE_DM6K_Device_Handle handle, U32 freq, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_MxL603(MT_FE_DM6K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_MxL603(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_MxL603(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_init_tc3800_tc(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_tc3800_tc(MT_FE_DM6K_Device_Handle handle, U32 Freq_KHz, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_tc3800_tc(MT_FE_DM6K_Device_Handle handle, S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc3800_tc(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_tc3800_tc(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_init_tc6800_tc(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_set_freq_tc6800_tc(MT_FE_DM6K_Device_Handle handle, U32 Freq_KHz, U32 sym_rate, S16 lpf_offset);
MT_FE_RET mt_fe_tn_get_strength_tc6800_tc(MT_FE_DM6K_Device_Handle handle,S8 *p_strength);
MT_FE_RET mt_fe_tn_sleep_tc6800_tc(MT_FE_DM6K_Device_Handle handle);
MT_FE_RET mt_fe_tn_wake_up_tc6800_tc(MT_FE_DM6K_Device_Handle handle);

#ifdef __cplusplus
}
#endif


#endif /* __MT_FE_T2_DEF_H__ */
