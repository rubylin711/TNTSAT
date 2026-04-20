/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2018                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
*	FILE:				mt_fe_dmd_ct8k_S_S2.c
*
*	VERSION:			12
*
*	DESCRIPTION:
*			Define the interfaces to initialize and get the channel info from Symphony2
*
*	History:
*	Description		Version		Date			Author
*---------------------------------------------------------------------------
*	File Create		00			2018.06.26		YZ.Huang
*	Modify			01			2018.06.26		YZ.Huang
*	Modify			02			2018.10.17		YZ.Huang
*	Modify			03			2018.11.30		YZ.Huang
*	Modify			04			2018.12.28		YZ.Huang
*	Modify			05			2019.04.24		YZ.Huang
*	Modify			06			2019.07.03		YZ.Huang
*	Modify			07			2019.07.23		YZ.Huang
*	Modify			08			2019.08.01		YZ.Huang
*	Modify			09			2019.09.06		YZ.Huang
*	Modify			10			2020.03.05		YZ.Huang
*	Modify			11			2020.04.20		YZ.Huang
*	Modify			12			2021.03.25		YZ.Huang
******************************************************************************/
#include <linux/printk.h>


#include "mt_fe_def.h"
#include "mt_fe_i2c_ct8k.h"
#include "mt_fe_dmd_ct8k_S_S2.h"
#include "mt_fe_dmd_fw_ct8k_S_S2.h"
#include "mt_fe_dmd_reg_tbl_ct8k_S_S2.h"
#include "mt_fe_tn_montage_ts6011.h"


#if MT_FE_DMD_DVBS_S2_SUPPORT
/************************
****external function****
*************************/
extern void _mt_sleep_ct8k(U32 ms);
extern void _mt_delay_ct8k(U32 ms);



/************************
****internal function****
*************************/
MT_FE_RET _mt_fe_dmd_set_demod_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type);
MT_FE_RET _mt_fe_dmd_init_reg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size);
MT_FE_RET _mt_fe_dmd_download_fw_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, const U8* p_fw, U16 fw_size);
MT_FE_RET _mt_fe_dmd_set_ts_out_mode_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TS_OUT_MODE mode);
MT_FE_RET _mt_fe_dmd_set_sym_rate_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_get_sym_rate_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_set_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_total_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_get_fec_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);

MT_FE_RET _mt_fe_dmd_select_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 tuner_freq_MHz);
MT_FE_RET _mt_fe_dmd_get_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *p_MCLK_KHz);
MT_FE_RET _mt_fe_dmd_set_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 MCLK_KHz);
MT_FE_RET _mt_fe_dmd_set_ts_divide_ratio_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE type, U8 dr_high, U8 dr_low);


/* function for blind scan*/
MT_FE_RET _mt_fe_dmd_bs_set_demod_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type);
MT_FE_RET _mt_fe_dmd_bs_remove_unlocked_TP_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz);
MT_FE_RET _mt_fe_dmd_bs_connect_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info);


MT_FE_RET mt_fe_unicable_set_tuner_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32* real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz);
MT_FE_RET mt_fe_dmd_get_channel_info_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info);


static void mt_fe_dummy_function_ct8k_ss2(MT_FE_MSG msg, void* p_param)
{
	UNUSED_PARAMETER(msg);
	UNUSED_PARAMETER(p_param);
}

static void (*mt_fe_bs_notify_ct8k)(MT_FE_MSG msg, void *p_tp_info) = mt_fe_dummy_function_ct8k_ss2;


static const U16 mes_log10[] = 
{
	    0,	 3010,	 4771,	 6021,	 6990,	 7781,	 8451,	 9031,	 9542,	10000,
	10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,
	13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,
	14914,	15052,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,
	16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,
	17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,
	17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,
	18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031,
	19085,	19138,	19191,	19243,	19294,	19345,	19395,	19445,	19494,	19542,
	19590,	19638,	19685,	19731,	19777,	19823,	19868,	19912,	19956,	20000
};


static MT_FE_PLS_INFO mPLSInfoTable[] = 
{
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x00,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  TRUE,          0},
    {0x01,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   TRUE,          0},
    {0x02,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  TRUE,          1},
    {0x03,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   TRUE,          1},
    {0x04,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       FALSE,  FALSE,         0},
    {0x05,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       TRUE,   FALSE,         0},
    {0x06,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       FALSE,  FALSE,         1},
    {0x07,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       TRUE,   FALSE,         1},
    {0x08,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       FALSE,  FALSE,         0},
    {0x09,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       TRUE,   FALSE,         0},
    {0x0A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       FALSE,  FALSE,         1},
    {0x0B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       TRUE,   FALSE,         1},
    {0x0C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       FALSE,  FALSE,         0},
    {0x0D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       TRUE,   FALSE,         0},
    {0x0E,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       FALSE,  FALSE,         1},
    {0x0F,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       TRUE,   FALSE,         1},
    {0x10,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       FALSE,  FALSE,         0},
    {0x11,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       TRUE,   FALSE,         0},
    {0x12,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       FALSE,  FALSE,         1},
    {0x13,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       TRUE,   FALSE,         1},
    {0x14,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x15,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x16,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0x17,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0x18,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x19,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x1A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x1B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x1C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x1D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x1E,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x1F,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x20,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x21,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x22,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x23,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x24,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x25,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x26,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x27,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x28,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x29,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x2A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x2B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x2C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x2D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x2E,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x2F,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x30,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x31,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x32,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0x33,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0x34,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x35,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x36,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x37,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x38,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x39,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x3A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x3B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x3C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x3D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x3E,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x3F,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x40,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x41,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x42,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x43,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x44,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x45,       TRUE,   MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x46,       FALSE,  MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x47,       FALSE,  MtFeType_DVBS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x48,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x49,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x4A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x4B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x4C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x4D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x4E,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x4F,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x50,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x51,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x52,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x53,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x54,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x55,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x56,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x57,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x58,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x59,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x5A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x5B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x5C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x5D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x5E,       FALSE,  MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x5F,       FALSE,  MtFeType_DVBS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x60,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x61,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x62,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x63,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x64,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x65,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x66,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x67,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x68,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x69,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x6A,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x6B,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x6C,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x6D,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x6E,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x6F,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x70,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x71,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x72,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x73,       TRUE,   MtFeType_DVBS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x74,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x75,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x76,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x77,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
    {0x78,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x79,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x7A,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x7B,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
    {0x7C,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x7D,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x7E,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x7F,       FALSE,  MtFeType_DVBS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x80,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x81,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x82,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x83,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x84,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_13_45,     FALSE,  FALSE,         0},
    {0x85,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_13_45,     TRUE,   FALSE,         0},
    {0x86,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_9_20,      FALSE,  FALSE,         0},
    {0x87,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_9_20,      TRUE,   FALSE,         0},
    {0x88,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_20,     FALSE,  FALSE,         0},
    {0x89,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_20,     TRUE,   FALSE,         0},
    {0x8A,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_5_9,       FALSE,  FALSE,         0},
    {0x8B,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_5_9,       TRUE,   FALSE,         0},
    {0x8C,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_26_45,     FALSE,  FALSE,         0},
    {0x8D,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_26_45,     TRUE,   FALSE,         0},
    {0x8E,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_23_36,     FALSE,  FALSE,         0},
    {0x8F,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_23_36,     TRUE,   FALSE,         0},
    {0x90,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_25_36,     FALSE,  FALSE,         0},
    {0x91,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_25_36,     TRUE,   FALSE,         0},
    {0x92,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_13_18,     FALSE,  FALSE,         0},
    {0x93,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_13_18,     TRUE,   FALSE,         0},
    {0x94,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2,       FALSE,  FALSE,         0},
    {0x95,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2,       TRUE,   FALSE,         0},
    {0x96,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,      FALSE,  FALSE,         0},
    {0x97,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,      TRUE,   FALSE,         0},
    {0x98,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9,       FALSE,  FALSE,         0},
    {0x99,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9,       TRUE,   FALSE,         0},
    {0x9A,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     FALSE,  FALSE,         0},
    {0x9B,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     TRUE,   FALSE,         0},
    {0x9C,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x9D,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x9E,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x9F,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xA0,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_28_45,     FALSE,  FALSE,         0},
    {0xA1,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_28_45,     TRUE,   FALSE,         0},
    {0xA2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_23_36,     FALSE,  FALSE,         0},
    {0xA3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_23_36,     TRUE,   FALSE,         0},
    {0xA4,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xA5,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xA6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      FALSE,  FALSE,         0},
    {0xA7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      TRUE,   FALSE,         0},
    {0xA8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_13_18,     FALSE,  FALSE,         0},
    {0xA9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_13_18,     TRUE,   FALSE,         0},
    {0xAA,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xAB,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xAC,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_77_90,     FALSE,  FALSE,         0},
    {0xAD,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_77_90,     TRUE,   FALSE,         0},
    {0xAE,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xAF,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xB0,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xB1,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xB2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xB3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xB4,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xB5,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xB6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xB7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xB8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xB9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xBA,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xBB,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xBC,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xBD,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xBE,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xBF,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xC0,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xC1,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xC2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0xC3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0xC4,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xC5,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xC6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0xC7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0xC8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0xC9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0xCA,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xCB,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xCC,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,     FALSE,  FALSE,         0},
    {0xCD,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,     TRUE,   FALSE,         0},
    {0xCE,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xCF,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xD0,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,     FALSE,  FALSE,         0},
    {0xD1,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,     TRUE,   FALSE,         0},
    {0xD2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xD3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xD4,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xD5,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xD6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0xD7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0xD8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_45,     FALSE,  FALSE,         1},
    {0xD9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_45,     TRUE,   FALSE,         1},
    {0xDA,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_4_15,      FALSE,  FALSE,         1},
    {0xDB,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_4_15,      TRUE,   FALSE,         1},
    {0xDC,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_14_45,     FALSE,  FALSE,         1},
    {0xDD,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_14_45,     TRUE,   FALSE,         1},
    {0xDE,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xDF,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
//   PLS Code,   Valid,  Dvb Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xE0,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xE1,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xE2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xE3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xE4,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xE5,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
    {0xE6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xE7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xE8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_26_45,     FALSE,  FALSE,         1},
    {0xE9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_26_45,     TRUE,   FALSE,         1},
    {0xEA,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xEB,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_8psk,       MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xEC,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xED,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
    {0xEE,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xEF,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xF0,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     FALSE,  FALSE,         1},
    {0xF1,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     TRUE,   FALSE,         1},
    {0xF2,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0xF3,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0xF4,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xF5,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xF6,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0xF7,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0xF8,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xF9,       TRUE,   MtFeType_DVBS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xFA,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFB,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xFC,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFD,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xFE,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFF,       FALSE,  MtFeType_DVBS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0}
};



/************************************************************************
** Function: _mt_fe_dmd_da3100_init_bs_params_default
**
**
** Description:	This function is to configure the device parameters of blind scan
**
**
**
** Inputs:
**
**	  Parameter			Type						Description
**	----------------------------------------------------------------------
**	  tuner_config		MT_FE_TN_DEV_SETTINGS*
**
** Outputs:	None
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_config_bs_params_ss2_ct8k(MT_FE_CT8K_Device_Handle handle)
{
	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle->m_device_ss2.bs_cfg.fft_length		 = FFT_LENGTH;		// 512
	handle->m_device_ss2.bs_cfg.psd_overlap		 = PSD_OVERLAP;		// 96;
	handle->m_device_ss2.bs_cfg.ave_times		 = AVE_TIMES;		// 256;
	handle->m_device_ss2.bs_cfg.data_length		 = DATA_LENGTH;		// 512;

	handle->m_device_ss2.bs_cfg.notch_range_f	 = NOTCH_RANGE_F;	// 8;
	handle->m_device_ss2.bs_cfg.start_range_f	 = START_RANGE_F;	// 15;
	handle->m_device_ss2.bs_cfg.find_notch_step	 = FIND_NOTCH_STEP;	// 1;
	handle->m_device_ss2.bs_cfg.psd_scale		 = PSD_SCALE;		// 0;
	handle->m_device_ss2.bs_cfg.sm_bufdep		 = SM_BUFDEP;		// 8;

	handle->m_device_ss2.bs_cfg.bs_symrate		 = BLINDSCAN_SYMRATEKSs;	// 40000;

	handle->m_device_ss2.bs_cfg.tuner_slip_step	 = TUNER_SLIP_STEP;	// 11;

	switch (handle->m_device_ss2.tuner_cfg.tuner_type)
	{
		case MtFeTn_TS2022:
		{
			handle->m_device_ss2.bs_cfg.bs_skip_step		 = 36000;

			handle->m_device_ss2.bs_cfg.sm_buf_1st		 = 28;			// TWO_LOOP_SM_BUF_1ST
			handle->m_device_ss2.bs_cfg.sm_buf_2nd		 = 4;			// ONE_LOOP_SM_BUF
			handle->m_device_ss2.bs_cfg.sm_buf_3rd		 = 2;			// TWO_LOOP_SM_BUF_2ND

			handle->m_device_ss2.bs_cfg.snr_thr_1st		 = 96;			// TWO_LOOP_SNR_THR_1ST
			handle->m_device_ss2.bs_cfg.snr_thr_2nd		 = 64;			// SNR_THR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.snr_thr_3rd		 = 64;			// TWO_LOOP_SNR_THR_2ND

			handle->m_device_ss2.bs_cfg.err_bound_factor_1st	 = 56;		// TWO_LOOP_ERR_BOUND_FACTOR_1ST
			handle->m_device_ss2.bs_cfg.err_bound_factor_2nd	 = 36;		// ERR_BOUND_FACTOR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.err_bound_factor_3rd	 = 36;		// TWO_LOOP_ERR_BOUND_FACTOR_2ND

			handle->m_device_ss2.bs_cfg.flat_thr_1st		 = 24;			// TWO_LOOP_FLAT_THR_1ST
			handle->m_device_ss2.bs_cfg.flat_thr_2nd		 = 28;			// FLAT_THR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.flat_thr_3rd		 = 28;			// TWO_LOOP_FLAT_THR_2ND

			handle->m_device_ss2.bs_cfg.thr_factor_1st	 = 1;			// TWO_LOOP_THR_FACTOR_1ST
			handle->m_device_ss2.bs_cfg.thr_factor_2nd	 = 5;			// THR_FACTOR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.thr_factor_3rd	 = 5;			// TWO_LOOP_THR_FACTOR_2ND
		}
		break;

		case MtFeTn_TS6011:
		default:
		{
			handle->m_device_ss2.bs_cfg.bs_skip_step		 = 36000;

			handle->m_device_ss2.bs_cfg.sm_buf_1st		 = 28;			// TWO_LOOP_SM_BUF_1ST
			handle->m_device_ss2.bs_cfg.sm_buf_2nd		 = 4;			// ONE_LOOP_SM_BUF
			handle->m_device_ss2.bs_cfg.sm_buf_3rd		 = 2;			// TWO_LOOP_SM_BUF_2ND

			handle->m_device_ss2.bs_cfg.snr_thr_1st		 = 64;			// TWO_LOOP_SNR_THR_1ST
			handle->m_device_ss2.bs_cfg.snr_thr_2nd		 = 64;			// SNR_THR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.snr_thr_3rd		 = 32;			// TWO_LOOP_SNR_THR_2ND

			handle->m_device_ss2.bs_cfg.err_bound_factor_1st	 = 56;		// TWO_LOOP_ERR_BOUND_FACTOR_1ST
			handle->m_device_ss2.bs_cfg.err_bound_factor_2nd	 = 36;		// ERR_BOUND_FACTOR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.err_bound_factor_3rd	 = 36;		// TWO_LOOP_ERR_BOUND_FACTOR_2ND

			handle->m_device_ss2.bs_cfg.flat_thr_1st		 = 24;			// TWO_LOOP_FLAT_THR_1ST
			handle->m_device_ss2.bs_cfg.flat_thr_2nd		 = 28;			// FLAT_THR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.flat_thr_3rd		 = 28;			// TWO_LOOP_FLAT_THR_2ND

			handle->m_device_ss2.bs_cfg.thr_factor_1st	 = 1;			// TWO_LOOP_THR_FACTOR_1ST
			handle->m_device_ss2.bs_cfg.thr_factor_2nd	 = 5;			// THR_FACTOR_ONE_LOOP
			handle->m_device_ss2.bs_cfg.thr_factor_3rd	 = 5;			// TWO_LOOP_THR_FACTOR_2ND
		}
		break;
	}

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_ct8k_ss2_config_default(MT_FE_CT8K_Device_Handle handle)
{
	U8 i;

	MT_FE_SS2_Device_Handle handle_ss2;

	if(handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle_ss2 = &handle->m_device_ss2;

	handle_ss2->demod_type = MtFeType_Undef;
	handle_ss2->ts_out_mode = MtFeTsOutMode_Parallel;

	switch(handle->sys_dev_addr)
	{
		case 0x1E:
			handle_ss2->demod_dev_addr = 0xd6;
			break;

		case 0x1C:
			handle_ss2->demod_dev_addr = 0xd4;
			break;

		case 0x1A:
			handle_ss2->demod_dev_addr = 0xd2;
			break;

		case 0x18:
		default:
			handle_ss2->demod_dev_addr = 0xd0;
			break;
	}

	handle_ss2->mcu_status					 = 0;

	handle_ss2->board_cfg.bIQInverted		 = FALSE;
	handle_ss2->board_cfg.bAGCPolar			 = FALSE;
	handle_ss2->board_cfg.bSpectrumInverted	 = FALSE;
	handle_ss2->board_cfg.bPolarLNBEnable	 = LNB_ENABLE_WHEN_LNB_EN_HIGH;
	handle_ss2->board_cfg.bPolarLNB13V		 = LNB_13V_WHEN_VSEL_HIGH;
	handle_ss2->board_cfg.bPolarLNBStandby	 = LNB_VSEL_STANDBY_HIGH;
	handle_ss2->board_cfg.bPolarDiSEqCOut	 = LNB_DISEQC_OUT_FORCE_HIGH;
	handle_ss2->board_cfg.bDiSEqCOutputOnly	 = LNB_DISEQC_OUT_ONLY_OUTPUT;

	handle_ss2->ts_cfg.bPinSwitch			 = MT_FE_TS_PIN_ORDER_D0_D7;
	handle_ss2->ts_cfg.mPinDriver			 = MtFeTsDriverAbility_4mA;
	handle_ss2->ts_cfg.mMaxClock			 = MtFeTSOut_Max_Clock_Auto;
	handle_ss2->ts_cfg.iCustomDutyHigh		 = 0;
	handle_ss2->ts_cfg.iCustomDutyLow		 = 0;
	handle_ss2->ts_cfg.bAutoSerial			 = MT_FE_TS_CLOCK_AUTO_SET_FOR_SERIAL_MODE;
	handle_ss2->ts_cfg.bAutoParallel		 = MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE;
	handle_ss2->ts_cfg.bAutoCI				 = MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE;

	handle_ss2->lnb_cfg.bLnbOn				 = TRUE;
	handle_ss2->lnb_cfg.b22K				 = FALSE;
	handle_ss2->lnb_cfg.mLnbVoltage			 = MtFeLNB_13V;

	handle_ss2->lnb_cfg.bToneBurst			 = FALSE;
	handle_ss2->lnb_cfg.mToneBurst			 = MtFeDiSEqCToneBurst_Moulated;

	handle_ss2->lnb_cfg.bDiSEqCSend			 = FALSE;
	handle_ss2->lnb_cfg.iSizeSend			 = 0;

	for(i = 0; i < 8; i ++)
	{
		handle_ss2->lnb_cfg.iDataSend[i]	 = 0;
	}

	handle_ss2->lnb_cfg.bDiSEqCReceive		 = FALSE;
	handle_ss2->lnb_cfg.iSizeReceive		 = 0;
	for(i = 0; i < 8; i ++)
	{
		handle_ss2->lnb_cfg.iDataReceive[i]	 = 0;
	}

	handle_ss2->lnb_cfg.bEnvelopMode		 = FALSE;

	handle_ss2->lnb_cfg.bUnicable			 = FALSE;
	handle_ss2->lnb_cfg.iBankIndex			 = 0;
	handle_ss2->lnb_cfg.iUBIndex			 = 0;
	handle_ss2->lnb_cfg.iUBFreqMHz			 = 1210;

	handle_ss2->lnb_cfg.iUBCount			 = 0;

	for(i = 0; i < 32; i ++)
	{
		handle_ss2->lnb_cfg.iUBList[i]		 = 0;
	}

#if 1	// Inverto IDLU-32UL40-UNBOO-OPP
	handle_ss2->lnb_cfg.iUBList[0]			 = 1210;
	handle_ss2->lnb_cfg.iUBList[1]			 = 1420;
	handle_ss2->lnb_cfg.iUBList[2]			 = 1680;
	handle_ss2->lnb_cfg.iUBList[3]			 = 2040;
	handle_ss2->lnb_cfg.iUBList[4]			 = 984;
	handle_ss2->lnb_cfg.iUBList[5]			 = 1020;
	handle_ss2->lnb_cfg.iUBList[6]			 = 1056;
	handle_ss2->lnb_cfg.iUBList[7]			 = 1092;
	handle_ss2->lnb_cfg.iUBList[8]			 = 1128;
	handle_ss2->lnb_cfg.iUBList[9]			 = 1164;
	handle_ss2->lnb_cfg.iUBList[10]			 = 1256;
	handle_ss2->lnb_cfg.iUBList[11]			 = 1292;
	handle_ss2->lnb_cfg.iUBList[12]			 = 1328;
	handle_ss2->lnb_cfg.iUBList[13]			 = 1364;
	handle_ss2->lnb_cfg.iUBList[14]			 = 1458;
	handle_ss2->lnb_cfg.iUBList[15]			 = 1494;
	handle_ss2->lnb_cfg.iUBList[16]			 = 1530;
	handle_ss2->lnb_cfg.iUBList[17]			 = 1566;
	handle_ss2->lnb_cfg.iUBList[18]			 = 1602;
	handle_ss2->lnb_cfg.iUBList[19]			 = 1638;
	handle_ss2->lnb_cfg.iUBList[20]			 = 1716;
	handle_ss2->lnb_cfg.iUBList[21]			 = 1752;
	handle_ss2->lnb_cfg.iUBList[22]			 = 1788;
	handle_ss2->lnb_cfg.iUBList[23]			 = 1824;
	handle_ss2->lnb_cfg.iUBList[24]			 = 1860;
	handle_ss2->lnb_cfg.iUBList[25]			 = 1896;
	handle_ss2->lnb_cfg.iUBList[26]			 = 1932;
	handle_ss2->lnb_cfg.iUBList[27]			 = 1968;
	handle_ss2->lnb_cfg.iUBList[28]			 = 2004;
	handle_ss2->lnb_cfg.iUBList[29]			 = 2076;
	handle_ss2->lnb_cfg.iUBList[30]			 = 2112;
	handle_ss2->lnb_cfg.iUBList[31]			 = 2148;
#else
	handle_ss2->lnb_cfg.iUBList[0]			 = 1210;
	handle_ss2->lnb_cfg.iUBList[1]			 = 1420;
	handle_ss2->lnb_cfg.iUBList[2]			 = 1680;
	handle_ss2->lnb_cfg.iUBList[3]			 = 2040,
	handle_ss2->lnb_cfg.iUBList[4]			 = 1005;
	handle_ss2->lnb_cfg.iUBList[5]			 = 1050;
	handle_ss2->lnb_cfg.iUBList[6]			 = 1095;
	handle_ss2->lnb_cfg.iUBList[7]			 = 1140;
	handle_ss2->lnb_cfg.iUBList[8]			 = 1260;
	handle_ss2->lnb_cfg.iUBList[9]			 = 1305;
	handle_ss2->lnb_cfg.iUBList[10]			 = 1350;
	handle_ss2->lnb_cfg.iUBList[11]			 = 1475;
	handle_ss2->lnb_cfg.iUBList[12]			 = 1520;
	handle_ss2->lnb_cfg.iUBList[13]			 = 1565;
	handle_ss2->lnb_cfg.iUBList[14]			 = 1610;
	handle_ss2->lnb_cfg.iUBList[15]			 = 1725;
	handle_ss2->lnb_cfg.iUBList[16]			 = 1770;
	handle_ss2->lnb_cfg.iUBList[17]			 = 1815;
	handle_ss2->lnb_cfg.iUBList[18]			 = 1860;
	handle_ss2->lnb_cfg.iUBList[19]			 = 1905;
	handle_ss2->lnb_cfg.iUBList[20]			 = 1950;
	handle_ss2->lnb_cfg.iUBList[21]			 = 1995;
	handle_ss2->lnb_cfg.iUBList[22]			 = 2085;
	handle_ss2->lnb_cfg.iUBList[23]			 = 2130;
#endif

	handle_ss2->lnb_cfg.iUBVer				 = 1;		/* 2: Unicable II; 1 or others: Unicable I */

	handle_ss2->global_cfg.iMclkKHz			 = 96000;
	handle_ss2->global_cfg.iSerialMclkHz	 = 96000;

	handle_ss2->global_cfg.bLastTpAFlag		 = FALSE;
	handle_ss2->global_cfg.iTpIndex			 = 0;
	handle_ss2->global_cfg.iLockedTpCnt		 = 0;
	handle_ss2->global_cfg.iScannedTpCnt	 = 0;
	handle_ss2->global_cfg.iCurCompareTpAKHz = 0;
	handle_ss2->global_cfg.bCancelBs		 = FALSE;
	handle_ss2->global_cfg.iTotalBsTimes	 = 0;

	handle_ss2->tp_cfg.iFreqKHz				 = 0;
	handle_ss2->tp_cfg.iSymRateKSs			 = 0;


	handle_ss2->tuner_cfg.tuner_dev_addr	 = 0x58;
	handle_ss2->tuner_cfg.tuner_type		 = MtFeTn_TS6011;
	handle_ss2->tuner_cfg.tuner_init_ok		 = 0;

	return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_get_driver_version_ct8k_ss2
**
**
** Description:	This function is used to get the driver version in number
**				mode.
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter			Type		Description
**	----------------------------------------------------------------------
**	 p_version			U8*		version number pointer
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_get_driver_version_ct8k_ss2(U8* p_version)
{
	*p_version = 12;		/* driver version number */

	return MtFeErr_Ok;
}


/************************************************************************
** Function: mt_fe_dmd_soft_reset_ct8k_ss2
**
**
** Description:	This function is used to do software reset.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_soft_reset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x01);
	_mt_delay_ct8k(1);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_global_reset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U8 data = 0;

	_mt_fe_dmd_get_reg_ct8k(handle, 0x03, &data);
	data = (U8)(data | 0x08);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);
	data = (U8)(data & 0xf7);
	_mt_fe_dmd_set_reg_ct8k(handle, 0x03, data);

	return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_hard_reset_ct8k_ss2
**
**
** Description:	This function is used to do chip reset by GPIO pin
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_hard_reset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 val = 0;

	U8 val_regb2 = 0;

	/*
		TODO:
			 do the hardware reset by the GPIO pin.
	*/

	mt_fe_dmd_wake_up_ct8k_ss2(handle);

	if(handle->m_device_ss2.tp_cfg.bLimitCarrierOffset)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb2, &val_regb2);
		if(val_regb2 == 0x01)
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);
		}
	}

	mt_fe_dmd_global_reset_ct8k_ss2(handle);

	_mt_delay_ct8k(1);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val);
	val |= 0x01;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val);


	/* reset global defines*/
	handle->m_device_ss2.tp_cfg.mCurrentType			 = MtFeType_DVBS;
	handle->m_device_ss2.tp_cfg.mConnectType			 = MtFeType_DVBS;
	handle->m_device_ss2.tp_cfg.iFreqKHz				 = 0;
	handle->m_device_ss2.tp_cfg.iSymRateKSs				 = 0;

	_mt_delay_ct8k(1);

	return ret;
}


MT_FE_RET mt_fe_dmd_calibration_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U32 ulData = 0, ulTmp;
	U8 ucTmp;

	if(handle->chip_version == 0x8000)		// 0xA0
	{
		//r 1f157000 to data
		//handle->Get32Bits(0xBF157000, &ulData);

		//w 1f157000 0xffdffffe
		//handle->Set32Bits(0xBF157000, 0xFFDFFFFE);

		//w 1f5d00c8 0xc49c0093
		handle->Set32Bits(0xBF5D00C8, 0xC49C0093);

		//wait 1mS
		_mt_delay_ct8k(1);

		//w 1f5d00c8 0xc49c0193
		handle->Set32Bits(0xBF5D00C8, 0xC49C0193);

		//wait 1mS
		_mt_delay_ct8k(1);

		//w 1f5d00c8 0xc49c0093
		handle->Set32Bits(0xBF5D00C8, 0xC49C0093);

		//wait 10mS
		_mt_sleep_ct8k(10);
		//w 1f157000 data
		//handle->Set32Bits(0xBF157000, ulData);
	}
	else if((handle->chip_version == 0x8001) || (handle->chip_version == 0x8009))		// A1
	{
		handle->Get32Bits(0xBF157004, &ulTmp);
		ulTmp &= 0xFFFFFFC7;		// bit[5:3] = 0'b001
		ulTmp |= 0x00000008;
		handle->Set32Bits(0xBF157004, ulTmp);
		//handle->Set32Bits(0xBF157004, 0x4A010C);

		//w 1f5d00c8 0xc49c0093
		handle->Set32Bits(0xBF5D00C8, 0xB49C006B);

		//wait 1mS
		_mt_delay_ct8k(1);

		//w 1f5d00c8 0xc49c0193
		handle->Set32Bits(0xBF5D00C8, 0xB49C016B);

		//wait 1mS
		_mt_delay_ct8k(1);

		//w 1f5d00c8 0xc49c0093
		handle->Set32Bits(0xBF5D00C8, 0xB49C006B);

		//wait 10mS
		_mt_sleep_ct8k(10);
		//w 1f157000 data
		//handle->Set32Bits(0xBF157000, ulData);

		handle->Get32Bits(0xBF5D0094, &ulData);
		handle->Set32Bits(0xBF5D0094, ulData | 0x100000);

		handle->Get32Bits(0xBF5D00D4, &ulTmp);
		ucTmp = (ulTmp >> 7) & 0x07;
		if(ucTmp == 0)
		{
			handle->Set32Bits(0xBF5D00C8, 0x349C806B);
		}
		else if(ucTmp == 4)
		{
			handle->Set32Bits(0xBF5D00C8, 0x349C406B);
		}
		else
		{
			handle->Set32Bits(0xBF5D00C8, 0x349C006B);
		}

		handle->Set32Bits(0xBF5D0094, ulData);
	}
	else// if((handle->chip_version == 0x8002) || (handle->chip_version == 0x800A))		// A2 or Symphony2 (0x9000 or 0x9001)
	{
		U32 j = 0;
		U32 comp0b = 32, comp1b = 32, comp2b = 32;

		U32 data = 0, done = 0, comp0 = 0, comp1 = 0, comp2 = 0, balance_all = 0, done_all = 0, delay_code = 0;
		U32 error = 0, error0 = 0, error1 = 0, error2 = 0;
		U32 offset0 = 0, offset1 = 0, offset2 = 0, offset_cal = 0;
		S32 delta0 = 0, delta1 = 0, delta2 = 0;


		handle->Set32Bits(0xBF5D00C8, 0x349C026B);		// 0x349C026B
		handle->Set32Bits(0xBF5D00CC, 0x41040);//set offset code,0:0x0,1:0x2082,32:0x41040,63:0x7FFFE


#if 1
		handle->Get32Bits(0xBF5D0094, &ulData);
		//mt_dbg_printf(1, "%s[%d] -- 0xBF5D00D4 = 0x%08X\n", __FUNCTION__, __LINE__, ulData);

		ulTmp = ulData & 0x08;

		for(j = 0; j < 5; j ++)
		{
			ulData = 0x80006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
			ulData = 0xC0006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
		}
		
		handle->Set32Bits(0xBF5D00C8, 0x349C036B);		// 0x349C036B

		for(j = 0; j < 5; j ++)
		{
			ulData = 0x80006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
			ulData = 0xC0006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
		}

		handle->Set32Bits(0xBF5D00C8, 0x349C026B);		// 0x349C026B
#else
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D00C8, 0x349C036B);		// 0x349C036B
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D0094, 0x8000E);
		handle->Set32Bits(0xBF5D0094, 0xC000E);
		handle->Set32Bits(0xBF5D00C8, 0x349C026B);		// 0x349C026B
#endif
		_mt_delay_ct8k(5);

		handle->Get32Bits(0xBF5D00D4, &data);
		comp0b = (data >> 1) & 0x3f;
		comp1b = (data >> 7) & 0x3f;
		comp2b = (data >> 13) & 0x3f;

		for(j = 1; j <= 64; j ++)
		{
#if 1
			ulData = 0x80006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
#else
			handle->Set32Bits(0xBF5D0094, 0x8000E);
#endif

			_mt_delay_ct8k(1);

#if 1
			ulData = 0xC0006 | ulTmp;
			handle->Set32Bits(0xBF5D0094, ulData);
#else
			handle->Set32Bits(0xBF5D0094, 0xC000E);
#endif

			handle->Get32Bits(0xBF5D00D4, &data);
			done = data & 0x01;
			comp0 = (data >> 1) & 0x3f;
			comp1 = (data >> 7) & 0x3f;
			comp2 = (data >> 13) & 0x3f;
			delta0 = comp0 - comp0b;
			delta1 = comp1 - comp1b;
			delta2 = comp2 - comp2b;
			if (delta0 > 1 || delta0 < -1)
			{
				if(error0 == 0)
				{
					offset0 = comp0b;
				}
				error0 = error0 + 1;
			}

			if (delta1 > 1 || delta1 < -1)
			{
				if(error1 == 0)
				{
					offset1 = comp1b;
				}
				error1 = error1 + 1;
			}

			if (delta2 > 1 || delta2 < -1)
			{
				if(error2 == 0)
				{
					offset2 = comp2b;
				}
				error2 = error2 + 1;
			}

			comp0b = comp0;
			comp1b = comp1;
			comp2b = comp2;

			_mt_delay_ct8k(1);
		}

		if (error0 == 0)
		{
			offset0 = comp0;
		}

		if (error1 == 0)
		{
			offset1 = comp1;
		}

		if (error2 == 0)
		{
			offset2 = comp2;
		}

		offset_cal = ((offset2 & 0x3f) << 13) + ((offset1 & 0x3f) << 7) + ((offset0 & 0x3f) << 1) + 1;

		handle->Set32Bits(0xBF5D00CC, offset_cal);
		handle->Set32Bits(0xBF5D00C8, 0x349C426B);		// 0x349C426B

		//disp offset info
		error = ((error2 & 0x1) << 2) + ((error1 & 0x1) << 1) + ((error0 & 0x1));

		handle->Get32Bits(0xBF5D00D4, &data);
		done = data & 0x01;
		comp0 = (data >> 1) & 0x3f;
		comp1 = (data >> 7) & 0x3f;
		comp2 = (data >> 13) & 0x3f;

#if 1
		ulData = 0x180006 | ulTmp;
		handle->Set32Bits(0xBF5D0094, ulData);
#else
		handle->Set32Bits(0xBF5D0094, 0x18000E);
#endif

		handle->Get32Bits(0xBF5D00D4, &data);
		balance_all = (data >> 1) & 0x7;
		done_all = (data >> 4) & 0x7;
		delay_code = (data >> 7) & 0x7;

#if 1
		ulData = 0x06 | ulTmp;
		handle->Set32Bits(0xBF5D0094, ulData);
#else
		handle->Set32Bits(0xBF5D0094, 0xE);
#endif

		handle->Get32Bits(0xBF5D00D4, &data);
		mt_fe_print(("0x%08X %d %d %d, %d %d %d, %d,%d %d %d\n", data, comp0, comp1, comp2, done, done_all, balance_all, delay_code, error0, error1, error2));
	}


	// 0xBF5D00D4 Bit0: s-adc  calibration done
	// 0 - calibration fail
	// 1 - calibration ok

	handle->Get32Bits(0xBF5D00D4, &ulData);
	mt_fe_print(("\tSymphony2 satellite module calibration %s!\n", (ulData & 0x01) ? "OK" : "failed"));

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_set_demod_mode_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE type)
{
	MT_FE_RET ret = MtFeErr_Ok;

	switch(type)
	{
		case MtFeType_DVBS2:
			_mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x04, 0x08);
			_mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x07, 0x02);

			handle->m_device_ss2.demod_current_type = MtFeType_DVBS2;

			break;

		case MtFeType_DVBS:
		default:
			_mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x04, 0x08);
			_mt_fe_ct8k_set_reg(handle->sys_dev_addr, 0x07, 0x02);

			handle->m_device_ss2.demod_current_type = MtFeType_DVBS;
			break;
	}

	return ret;
}


/******************************************************************************
** Function: mt_fe_dmd_init_ct8k_ss2
**
**
** Description:  This function is used to initial Symphony2 satellite module.
**				 Only called once after power on.
**
**
** Note:
**
**		You can change the TS output mode with the define "MT_FE_TS_OUPUT_MODE"
**	in "mt_fe_def.h"
**
**
** Inputs:   None
**
** Outputs:  None
**
*******************************************************************************/
MT_FE_RET mt_fe_dmd_init_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U8 val = 0;

	//mt_fe_dmd_calibration_ct8k_ss2(handle);

	mt_fe_dmd_hard_reset_ct8k_ss2(handle);

	if(handle->m_device_ss2.tuner_cfg.tuner_init != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_init(handle);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ (handle->m_device_ss2.lnb_cfg.bUnicable && (handle->m_device_ss2.lnb_cfg.iUBVer != 2));		// XOR
	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted;		// XOR

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x07, 0xE0);		// global reset, global diseqc reset, golbal fec reset
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x07, 0x00);

	_mt_fe_dmd_init_reg_ct8k_ss2(handle, reg_tbl_def_ct8k_ss2, sizeof(reg_tbl_def_ct8k_ss2) / 2);

	//_mt_fe_dmd_set_demod_mode_ct8k_ss2(handle, handle->m_device_ss2.tp_cfg.mCurrentType);

	_mt_fe_dmd_download_fw_ct8k_ss2(handle, m88ct8kss2_fm, sizeof(m88ct8kss2_fm) / sizeof(U8));

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb9, &val);

	if(val != ct8k_ss2_fm_ver)
	{
		mt_fe_print(("\tDownload firmware failed! Version no = 0x%02X!\n", val));
	}

	//_mt_fe_dmd_set_ts_out_mode_ct8k_ss2(handle, handle->m_device_ss2.ts_out_mode);


	if(handle->m_device_ss2.board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val |= 0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val &= ~0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}

	if(handle->m_device_ss2.board_cfg.bAGCPolar)			// 0x30, bit 4
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x30, &val);
		val |= 0x10;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, val);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x30, &val);
		val &= ~0x10;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, val);
	}

	return MtFeErr_Ok;
}


/************************************************************************
** Function: mt_fe_dmd_sleep_ct8k_ss2
**
**
** Description:	This function is used to sleep Symphony2 satellite module.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_sleep_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U8 val = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val);
	val &= ~0x01;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val);

	if(handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x04, &val);
	val |= 0x01;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x04, val);

	return MtFeErr_Ok;
}



/************************************************************************
** Function: mt_fe_dmd_wake_up_ct8k_ss2
**
**
** Description:	This function is used to wake up Symphony2 satellite module.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_wake_up_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U8 val = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x04, &val);
	val &= ~0x01;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x04, val);

	if(handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val);
	val |= 0x01;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val);

	mt_fe_dmd_calibration_ct8k_ss2(handle);

	return MtFeErr_Ok;
}



/**********************************************************************************************
** Function: mt_fe_dmd_connect_ct8k_ss2
**
**
** Description:	This function is used to lock the specified TP.
**
**
** Inputs:
**
**		Parameter		Type			Description
**		----------------------------------------------------------------------
**		freq_MHz		U32			TP's frequency(MHz) passed by app level
**		sym_rate_KSs	U32			TP's symbol rate(KSs) passed by app level
**		dvbs_type		MT_FE_TYPE	DVBS or DVBS2 mode	passed by app level
**
**
**	Note:
**
**		The dvbs_type passed by app level may be 3 mode.
**		"MtFeType_DVBS" or "MtFeType_DVBS2" or "MtFeType_DTV_Unknown".
**
**		If the type is 	"MtFeType_DTV_Unknown", driver will try DVBS and DVBS2
**  automatically.
**
**
**
** Outputs:  None
**
**********************************************************************************************/
MT_FE_RET mt_fe_dmd_connect_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type)
{
	S16	lpf_offset_KHz = 0;
	U32	freq_tmp_KHz = 0;

	U32	real_freq_KHz = 0;
	S32	deviceoffset_KHz = 0;
	S32	tuner_offset_KHZ = 0;

	int i;

	MT_FE_RET ret = MtFeErr_Ok;


	U8  val_regb2;

	if((handle->m_device_ss2.tuner_cfg.tuner_set == NULL) || (handle->m_device_ss2.tuner_cfg.tuner_get_offset == NULL))
		return MtFeErr_NullPointer;


	mt_fe_dmd_global_reset_ct8k_ss2(handle);

	mt_fe_dmd_clear_ts_ct8k_ss2(handle);

	handle->m_device_ss2.tp_cfg.bCheckCCM = FALSE;
	handle->m_device_ss2.tp_cfg.iTsCnt = 0;

	for(i = 0; i < 16; i ++)
	{
		handle->m_device_ss2.tp_cfg.ucTsId[i] = 0;
	}

	handle->m_device_ss2.tp_cfg.mConnectType		 = dvbs_type;
	handle->m_device_ss2.tp_cfg.iSymRateKSs			 = sym_rate_KSs;

	handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ (handle->m_device_ss2.lnb_cfg.bUnicable && (handle->m_device_ss2.lnb_cfg.iUBVer != 2));		// XOR
	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted;		// XOR

	if(handle->m_device_ss2.tp_cfg.bLimitCarrierOffset)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb2, &val_regb2);
		if(val_regb2 == 0x01)
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);
		}
	}


	freq_tmp_KHz	 = freq_MHz * 1000;
	lpf_offset_KHz	 = 0;

	if ((sym_rate_KSs < 5000) && (!handle->m_device_ss2.lnb_cfg.bUnicable))
	{
		lpf_offset_KHz = FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
		freq_tmp_KHz  += FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
	}

	handle->m_device_ss2.tp_cfg.iFreqKHz = freq_tmp_KHz;

	mt_fe_dmd_config_clock_ct8k(handle, dvbs_type);

	/*set tuner*/
	if(handle->m_device_ss2.lnb_cfg.bUnicable)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		mt_fe_unicable_set_tuner_ct8k_ss2(handle, freq_tmp_KHz, sym_rate_KSs, &real_freq_KHz,
		handle->m_device_ss2.lnb_cfg.iUBIndex,
		handle->m_device_ss2.lnb_cfg.iBankIndex,
		handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
		deviceoffset_KHz = real_freq_KHz - freq_tmp_KHz;
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}
	else
	{
		//mt_fe_tn_set_tuner(handle, freq_tmp_KHz, sym_rate_KSs, lpf_offset_KHz);
		if(handle->m_device_ss2.tuner_cfg.tuner_set != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_set(handle, freq_tmp_KHz, sym_rate_KSs, lpf_offset_KHz);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}

		real_freq_KHz = freq_tmp_KHz;
		deviceoffset_KHz = 0;
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x01);


	/*set demod registers*/
	_mt_fe_dmd_set_demod_ct8k_ss2(handle, sym_rate_KSs, handle->m_device_ss2.tp_cfg.mConnectType);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val_regb2);
	if (dvbs_type == MtFeType_DVBS)
	{
		val_regb2 = (U8)((val_regb2 & 0xfb) | 0x40);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val_regb2);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xe0, 0xf8);
	}
	else if (dvbs_type == MtFeType_DVBS2)
	{
		val_regb2 = (U8)(val_regb2 | 0x44);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val_regb2);
	}
	else// if (dvbs_type == MtFeType_DVBS_S2)
	{
		val_regb2 = (U8)(val_regb2 & 0xbb);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val_regb2);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xe0, 0xf8);
	}

	/*set carrier offset*/
	if (handle->m_device_ss2.tuner_cfg.tuner_get_offset != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &tuner_offset_KHZ);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	_mt_fe_dmd_set_carrier_offset_ct8k_ss2(handle, tuner_offset_KHZ+ lpf_offset_KHz + deviceoffset_KHz);
	//_mt_fe_dmd_set_carrier_offset_ct8k_ss2(handle, lpf_offset_KHz + deviceoffset_KHz);


	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);


	if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
	{
		mt_fe_tn_ts6011_adjust_AGC_ct8k((MT_FE_Tuner_Handle_TS6011)(handle->m_device_ss2.tuner_cfg.tuner_handle));
	}

	return ret;
}



/******************************************************************
** Function: mt_fe_dmd_get_lock_state_ct8k_ss2
**
**
** Description:	This function is called by app level to get the
**			lock status of Symphony2 satellite module.
**
**
** Inputs:   None
**
**
** Outputs:
**
**		Parameter	Type				Description
**		-----------------------------------------------------------
**		p_state		MT_FE_LOCK_STATE*	lock status of Symphony2 satellite module
**
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_get_lock_state_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8	tmp = 0, ucTmp = 0;
	int iCnt = 0;

	U8 val_regb2;
	S32 carrier_offset_KHz;

	if (p_state == 0)
		return MtFeErr_Param;

	*p_state = MtFeLockState_Waiting;

	if(handle->m_device_ss2.tp_cfg.bLimitCarrierOffset)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb2, &val_regb2);
		if(val_regb2 == 0x01)
		{
			*p_state = MtFeLockState_Unlocked;

			return MtFeErr_Ok;
		}
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &tmp);
	if(tmp == 0xfe)
	{
		U8 tmp1;

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xae, 0x4f);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x7f, 0xc4);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, 0xc0);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
		ucTmp |= 0x04;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, ucTmp);

		mt_fe_dmd_soft_reset_ct8k_ss2(handle);

		iCnt = 0;

		do
		{
			iCnt ++;
			if(iCnt > 1000)
			{
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, 0x01);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, 0x00);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, 0x00);

				mt_fe_dmd_soft_reset_ct8k_ss2(handle);

				return MtFeErr_TimeOut;
			}

			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x24, &tmp1);

			_mt_delay_ct8k(1);
		}while((tmp1 & 0x08) == 0x00);

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xae, 0x09);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x7f, 0x04);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
		ucTmp &= ~0x04;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, ucTmp);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x22, &tmp1);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, tmp1);
		handle->m_device_ss2.tp_cfg.ucPLSCode[0] = tmp1;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x23, &tmp1);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, tmp1);
		handle->m_device_ss2.tp_cfg.ucPLSCode[1] = tmp1;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x24, &tmp1);
		tmp1 &= 0x03;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, tmp1);
		handle->m_device_ss2.tp_cfg.ucPLSCode[2] = tmp1;

		handle->m_device_ss2.tp_cfg.bHavePLS = TRUE;

		mt_fe_dmd_soft_reset_ct8k_ss2(handle);
	}


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp);
	if(tmp & 0x08)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
		if(((handle->m_device_ss2.tp_cfg.iSymRateKSs > 47100) && (handle->m_device_ss2.tp_cfg.iSymRateKSs < 47500) && ((tmp & 0x0f) == 0x0f)) || 
			((tmp & 0x8f) == 0x8f))
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x89, &tmp);
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
			tmp &= 0x80;
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, (U8)((ucTmp & 0xf7) | (tmp >> 4) | 0x02));

			//if(handle->m_device_ss2.tp_cfg.ucCurTsId == 0)
			{
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &ucTmp);
				if(ucTmp == 0xfe)
				{
					mt_fe_print(("\tPLS before checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x!\n", ucTmp, tmp));

					return MtFeErr_Ok;
				}

				if(!handle->m_device_ss2.tp_cfg.bCheckCCM)
				{
					MT_BOOL bMIS = FALSE;

					bMIS = _mt_fe_dmd_check_MIS_ct8k_ss2(handle);

					if(bMIS)
					{
						int iTmpCnt = 0;
						//_mt_sleep_ct8k(10);		// Delay for MIS ready

						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xfa, 0x00);
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf0, 0x00);
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf0, 0x03);
						//_mt_sleep_ct8k(500);

						do
						{
							_mt_sleep_ct8k(50);
							_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &ucTmp);

							iTmpCnt ++;
						} while ((ucTmp != 0xfe) && (iTmpCnt < 20));		// 50ms * 20 = 1000ms

						mt_fe_print(("\tPLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms!\n", ucTmp, tmp, 50 * iTmpCnt));

						if(ucTmp == 0xfe)
						{
							return MtFeErr_Ok;
						}

						_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf1, &ucTmp);
						handle->m_device_ss2.tp_cfg.iTsCnt = ucTmp & 0x1F;

						if(handle->m_device_ss2.tp_cfg.iTsCnt >= 1)
						{
							mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", handle->m_device_ss2.tp_cfg.iTsCnt));

							for(iCnt = 0; iCnt < handle->m_device_ss2.tp_cfg.iTsCnt; iCnt ++)
							{
								if(iCnt >= 32)
								{
									ret = MtFeErr_NoMemory;
									break;
								}

								ucTmp = (U8)iCnt;

								_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf2, ucTmp);
								_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf3, &ucTmp);

								handle->m_device_ss2.tp_cfg.ucTsId[iCnt] = ucTmp;

								mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
							}

							for(iCnt = 0; iCnt < handle->m_device_ss2.tp_cfg.iTsCnt; iCnt ++)
							{
								if(handle->m_device_ss2.tp_cfg.ucTsId[iCnt] == handle->m_device_ss2.tp_cfg.ucCurTsId)
								{
									_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf5, handle->m_device_ss2.tp_cfg.ucCurTsId);

									mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, handle->m_device_ss2.tp_cfg.ucCurTsId));

									break;
								}
							}

							if(iCnt == handle->m_device_ss2.tp_cfg.iTsCnt)
							{
								// Default select TS0 output
								_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf5, handle->m_device_ss2.tp_cfg.ucTsId[0]);
								handle->m_device_ss2.tp_cfg.ucCurTsId = handle->m_device_ss2.tp_cfg.ucTsId[0];

								mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->m_device_ss2.tp_cfg.ucTsId[0]));
							}
						}

						handle->m_device_ss2.tp_cfg.bCheckCCM = TRUE;
					}
				}
			}

			*p_state = MtFeLockState_Locked;
			handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS2;

			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf9, 0x01);
		}
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
		if((tmp & 0xf7) == 0xf7)
		{
			*p_state = MtFeLockState_Locked;
			handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS;
		}
	}

	if (*p_state == MtFeLockState_Locked)
	{
		MT_FE_CHAN_INFO_DVBS2 ch_info;

		if(handle->m_device_ss2.tp_cfg.bLimitCarrierOffset)
		{
			carrier_offset_KHz = 0;
			if(handle->m_device_ss2.lnb_cfg.bUnicable)
			{
				_mt_fe_dmd_get_carrier_offset_ct8k_ss2(handle, &carrier_offset_KHz);
			}
			else
			{
				_mt_fe_dmd_get_total_carrier_offset_ct8k_ss2(handle, &carrier_offset_KHz);
			}

			if((carrier_offset_KHz >= handle->m_device_ss2.tp_cfg.iOffsetRangeKHz) || 
			   (carrier_offset_KHz <= - handle->m_device_ss2.tp_cfg.iOffsetRangeKHz))
			{
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x01);

				*p_state = MtFeLockState_Unlocked;

				return MtFeErr_Ok;
			}
		}

		mt_fe_dmd_get_channel_info_ct8k_ss2(handle, &ch_info);

		if((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk))
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9e, 0x9f);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9f, 0x0c);

			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9d, &tmp);
			tmp &= 0xFE;
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9d, tmp);
		}
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb3, &tmp);
		if((tmp & 0xc0) != 0x00)
		{
			*p_state = MtFeLockState_Unlocked;
		}
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_get_pure_lock_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	tmp;

	if (p_state == 0)
		return MtFeErr_Param;

	*p_state = MtFeLockState_Unlocked;


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp);
	if ((tmp & 0x08) == 0x08)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
		if(((handle->m_device_ss2.tp_cfg.iSymRateKSs > 47100) && (handle->m_device_ss2.tp_cfg.iSymRateKSs < 47500) && ((tmp & 0x0f) == 0x0f)) || 
			((tmp & 0x8f) == 0x8f))
		{
			*p_state = MtFeLockState_Locked;
			handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS2;
		}
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7)
		{
			*p_state = MtFeLockState_Locked;
			handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS;
		}
	}

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_set_ts_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 ucTsId)
{
	MT_FE_RET ret = MtFeErr_Ok;

#if 0
	int iCnt;


	for(iCnt = 0; iCnt < handle->m_device_ss2.tp_cfg.iTsCnt; iCnt ++)
	{
		if(handle->m_device_ss2.tp_cfg.ucTsId[iCnt] == ucTsId)
		{
#endif
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf5, ucTsId);

			//mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, ucTsId));

			handle->m_device_ss2.tp_cfg.ucCurTsId = ucTsId;

#if 0
			ret = MtFeErr_Ok;

			break;
		}
	}

	if(iCnt == handle->m_device_ss2.tp_cfg.iTsCnt)
	{
		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xE7, handle->m_device_ss2.tp_cfg.ucTsId[0]);

		//mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->m_device_ss2.tp_cfg.ucTsId[0]));

		mt_fe_print(("\tCan't find this TS ---- 0x%02X\n", ucTsId));

		ret = MtFeErr_Param;
	}
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_clear_ts_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf5, 0x00);

	return MtFeErr_Ok;
}

/*********************************************************************
** Function: mt_fe_dmd_get_per_ct8k_ss2
**
**
** Description:	Get the two package counter value to calculate the per at
**			app level.
**
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter			Type			Description
**	---------------------------------------------------------------
**	  p_total_packags	U32*			total packags
**	  p_err_packags		U32*			error packags
**
**
** NOTE:	This function outputs error and total packages in DVBS2 mode,
**		and error and total bits in DVBS mode. You can calculate the PER
**		and BER in APP level.
**
**********************************************************************/
MT_FE_RET mt_fe_dmd_get_per_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags)
{
	#define TOTAL_PACKET_S	3000

	U8		tmp1 = 0, tmp2 = 0, tmp3 = 0;
	U8		val_0xF6 = 0, val_0xF7 = 0, val_0xF8 = 0;
	U32		code_rate_fac = 0, ldpc_frame_cnt = 0;
	static  U32 pre_err_packags   = 0;
	static  U32 pre_total_packags = 0xffff;
	MT_FE_CHAN_INFO_DVBS2 p_info;


	*p_err_packags   = pre_err_packags;
	*p_total_packags = pre_total_packags;


	pre_err_packags = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp1);
	if ((tmp1 & 0x08) == 0x08)		// DVB-S2
	{
		mt_fe_dmd_get_channel_info_ct8k_ss2(handle, &p_info);
		switch (p_info.code_rate)
		{
			case MtFeCodeRate_1_4:	code_rate_fac = 16008 - 80;	break;
			case MtFeCodeRate_1_3:	code_rate_fac = 21408 - 80;	break;
			case MtFeCodeRate_2_5:	code_rate_fac = 25728 - 80;	break;
			case MtFeCodeRate_1_2:	code_rate_fac = 32208 - 80;	break;
			case MtFeCodeRate_3_5:	code_rate_fac = 38688 - 80;	break;
			case MtFeCodeRate_2_3:	code_rate_fac = 43040 - 80;	break;
			case MtFeCodeRate_3_4:	code_rate_fac = 48408 - 80;	break;
			case MtFeCodeRate_4_5:	code_rate_fac = 51648 - 80;	break;
			case MtFeCodeRate_5_6:	code_rate_fac = 53840 - 80;	break;
			case MtFeCodeRate_8_9:	code_rate_fac = 57472 - 80;	break;
			case MtFeCodeRate_9_10:	code_rate_fac = 58192 - 80;	break;
			case MtFeCodeRate_5_9:		code_rate_fac = 64800 * 5 / 9 - 192 - 80;	break;
			case MtFeCodeRate_7_9:		code_rate_fac = 64800 * 7 / 9 - 192 - 80;	break;
			case MtFeCodeRate_4_15:		code_rate_fac = 64800 * 4 / 15 - 192 - 80;	break;
			case MtFeCodeRate_7_15:		code_rate_fac = 64800 * 7 / 15 - 192 - 80;	break;
			case MtFeCodeRate_8_15:		code_rate_fac = 64800 * 8 / 15 - 192 - 80;	break;
			case MtFeCodeRate_11_15:	code_rate_fac = 64800 * 11 / 15 - 192 - 80;	break;
			case MtFeCodeRate_13_18:	code_rate_fac = 64800 * 13 / 18 - 192 - 80;	break;
			case MtFeCodeRate_9_20:		code_rate_fac = 64800 * 9 / 20 - 192 - 80;	break;
			case MtFeCodeRate_11_20:	code_rate_fac = 64800 * 11 / 20 - 192 - 80;	break;
			case MtFeCodeRate_23_36:	code_rate_fac = 64800 * 23 / 36 - 192 - 80;	break;
			case MtFeCodeRate_25_36:	code_rate_fac = 64800 * 25 / 36 - 192 - 80;	break;
			case MtFeCodeRate_11_45:	code_rate_fac = 64800 * 11 / 45 - 192 - 80;	break;
			case MtFeCodeRate_13_45:	code_rate_fac = 64800 * 13 / 45 - 192 - 80;	break;
			case MtFeCodeRate_14_45:	code_rate_fac = 64800 * 14 / 45 - 192 - 80;	break;
			case MtFeCodeRate_26_45:	code_rate_fac = 64800 * 26 / 45 - 192 - 80;	break;
			case MtFeCodeRate_28_45:	code_rate_fac = 64800 * 28 / 45 - 192 - 80;	break;
			case MtFeCodeRate_29_45:	code_rate_fac = 64800 * 29 / 45 - 192 - 80;	break;
			case MtFeCodeRate_31_45:	code_rate_fac = 64800 * 31 / 45 - 192 - 80;	break;
			case MtFeCodeRate_32_45:	code_rate_fac = 64800 * 32 / 45 - 192 - 80;	break;
			case MtFeCodeRate_77_90:	code_rate_fac = 64800 * 77 / 90 - 192 - 80;	break;
			default:	break;
		}

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xdb, &tmp1);
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xda, &tmp2);
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xd9, &tmp3);
		ldpc_frame_cnt = (tmp1 << 16) | (tmp2 << 8) | tmp3;


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf8, &tmp1);
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf7, &tmp2);
		pre_err_packags = (tmp1 << 8) | tmp2;


		pre_total_packags = code_rate_fac * ldpc_frame_cnt / (188 * 8);


		if (ldpc_frame_cnt > TOTAL_PACKET_S)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xd0, &tmp1);
			tmp1 = tmp1 | 0x08;
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xd0, tmp1);

			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf9, 0x01);

			tmp1 = tmp1 & 0xf7;
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xd0, tmp1);

			*p_err_packags   = pre_err_packags;
			*p_total_packags = pre_total_packags;
		}
	}
	else		// DVB-S
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xd5, &val_0xF8);

		if((val_0xF8 & 0x80) == 0)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xd6, &val_0xF6);
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xd7, &val_0xF7);

			pre_err_packags = (val_0xF7 << 8) | val_0xF6;

			//val_0xF8 |= 0x10;
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xd5, 0x82);
		}

		*p_err_packags   = pre_err_packags;

		*p_total_packags = 8388608 / 16;
	}


	return MtFeErr_Ok;
}



/******************************************************************
** Function: mt_fe_dmd_get_snr_ct8k_ss2
**
**
** Description:	get snr value, unit dB
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter		Type		Description
**	---------------------------------------------------------------
**	  p_snr			S8*			snr with dB unit
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_get_snr_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S8 *p_snr)
{
	const U16 mes_log10[] =
	{
		    0,	 3010,	 4771,	 6021,	 6990,	 7781,	 8451,	 9031,	 9542,	10000,
		10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,
		13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,
		14914,	15052,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,
		16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,
		17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,
		17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,
		18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031
	};


	const U16 mes_loge[] =
	{
		    0,	 6931,	10986,	13863,	16094,	17918,	19459,	20794,	21972,	23026,
		23979,	24849,	25649,	26391,	27081,	27726,	28332,	28904,	29444,	29957,
		30445,	30910,	31355,	31781,	32189,	32581,	32958,	33322,	33673,	34012,
		34340,	34657
	};



	static S8 snr;

	U8		val, npow1, npow2, spow1, cnt;
	U16		tmp;
	U32		npow, spow, snr_total;


	*p_snr = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val);
	if ((val & 0x08) != 0)	// DVB-S2
	{
		cnt  = 10;
		npow = 0;
		spow = 0;

		while(cnt > 0)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8c, &npow1);
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8d, &npow2);
			npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8e, &spow1);
			spow += ((spow1 * spow1) >> 1);

			cnt--;
		}

		npow /= 10;
		spow /= 10;

		if(spow == 0)
		{
			snr = 0;
		}
		else if(npow == 0)
		{
			snr = 19;
		}
		else
		{
			if(spow > npow)
			{
				tmp = (U16)((spow) / npow);
				if (tmp > 80)
					tmp = 80;

				snr = (S8)(mes_log10[tmp - 1] / 1000);
			}
			else
			{
				tmp = (U16)(npow / spow);
				if (tmp > 80)
					tmp = 80;

				snr = (S8)(-(mes_log10[tmp - 1] / 1000));
			}
		}
	}
	else
	{
		cnt = 10;
		snr_total = 0;

		while(cnt > 0)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xff, &val);
			snr_total += val;

			cnt--;
		}

		tmp = (U16)(snr_total / 80);
		if(tmp > 0)
		{
			if (tmp > 32)		// impossible
				tmp = 32;

			snr =  (S8)((mes_loge[tmp - 1] * 10) / 23026);
		}
		else
		{
			snr = 0;
		}
	}



	*p_snr = snr;


	return MtFeErr_Ok;
}



/******************************************************************
** Function: mt_fe_dmd_get_strength_ct8k_ss2
**
**
** Description:	get signal strength
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter			Type		Description
**	---------------------------------------------------------------
**	  p_strength		S8*			signal strength value(0----100)
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_get_strength_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S8 *p_strength)
{
	if (handle->m_device_ss2.tuner_cfg.tuner_strength != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_strength(handle, p_strength);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}
	else
		*p_strength = 0;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_get_sat_quality_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 *p_percent)
{
	MT_FE_LOCK_STATE lock_state;


	mt_fe_dmd_get_pure_lock_ct8k_ss2(handle, &lock_state);

	if(lock_state == MtFeLockState_Locked)
	{
		const U16 mes_log10[] = 
		{
			    0,	 3010,	 4771,	 6021,	 6990,	 7781,	 8451,	 9031,	 9542,	10000,
			10414,	10792,	11139,	11461,	11761,	12041,	12304,	12553,	12788,	13010,
			13222,	13424,	13617,	13802,	13979,	14150,	14314,	14472,	14624,	14771,
			14914,	15052,	15185,	15315,	15441,	15563,	15682,	15798,	15911,	16021,
			16128,	16232,	16335,	16435,	16532,	16628,	16721,	16812,	16902,	16990,
			17076,	17160,	17243,	17324,	17404,	17482,	17559,	17634,	17709,	17782,
			17853,	17924,	17993,	18062,	18129,	18195,	18261,	18325,	18388,	18451,
			18513,	18573,	18633,	18692,	18751,	18808,	18865,	18921,	18976,	19031,
			19085,	19138,	19191,	19243,	19294,	19345,	19395,	19445,	19494,	19542,
			19590,	19638,	19685,	19731,	19777,	19823,	19868,	19912,	19956,	20000
		};

		static S16 snr;

		U8		val, npow1, npow2, spow1, cnt;
		U16		tmp;
		U32		npow, spow, snr_total;


		*p_percent = 0;
		if (handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
		{
			cnt  = 10;
			npow = 0;
			spow = 0;

			while(cnt > 0)
			{
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8c, &npow1);
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8d, &npow2);
				npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8e, &spow1);
				spow += ((spow1 * spow1) >> 1);

				cnt--;
			}

			npow /= 10;
			spow /= 10;

			if(spow == 0)
			{
				snr = 0;
			}
			else if(npow == 0)
			{
				snr = 20;
			}
			else
			{
				if(spow > npow)
				{
					tmp = (U16)((spow) / npow);
					if (tmp > 100)
						tmp = 100;

					snr = (S8)(mes_log10[tmp - 1] / 1000);
				}
				else
				{
					tmp = (U16)(npow / spow);
					if (tmp > 100)
						tmp = 100;

					snr = (S8)(-(mes_log10[tmp - 1] / 1000));
				}
			}

			if (snr < -5)
				snr = -5;

			if (snr > 20)
				snr = 20;

			*p_percent = (U8)(30 + (snr + 5) * 70 / 25);		/* 30% ~ 100% */
		}
		else
		{
			cnt = 10;
			snr_total = 0;

			while(cnt > 0)
			{
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xff, &val);
				snr_total += val;

				cnt--;
			}

			snr = (S16)(snr_total / 10);


			*p_percent = (U8)(30 + snr * 70 / 256);		/* 30% ~ 100% */
		}
	}
	else
	{
		U8 val = 0, cnt = 0;

		U16 total = 0;

		for(cnt = 0; cnt < 3; cnt ++)
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x8f, &val);

			total = total + (U16)val;
		}

		val = (U8)(total / 3);

		if(val < 0x0e)
		{
			val = 0x0e;
		}

		if(val > 0x87)
		{
			val = 0x87;
		}

		*p_percent = (U8)((val - 0x0e) * 29 / 121);			/* 0% ~ 29% */
	}

	return MtFeErr_Ok;
}


/******************************************************************
** Function: mt_fe_dmd_get_channel_info_ct8k_ss2
**
**
** Description:	get channel info
**
**
** Inputs:   None
**
**
** Outputs:
**
**	  Parameter		Type					Description
**	---------------------------------------------------------------
**	  p_info		MT_FE_CHAN_INFO_DVBS2*	get the channel info.
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_get_channel_info_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info)
{
	U8	tmp = 0, /*val_0x7E = 0, */val_0x17 = 0, val_0x18 = 0, ucPLSCode = 0;


	//p_info->type = handle->m_device_ss2.tp_cfg.mCurrentType;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp);
	if((tmp & 0x08) == 0x08)	// DVB-S2
	//if(handle->m_device_ss2.tp_cfg.mCurrentType == MtFeType_DVBS2)
	{
		handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS2;

		p_info->type = MtFeType_DVBS2;

#if 1
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8a, 0x01);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x17, &val_0x17);
		p_info->is_dummy_frame = (val_0x17 & 0x40) ? MtFe_True : MtFe_False;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x18, &val_0x18);
		tmp = (val_0x18 >> 5) & 0x07;
		switch(tmp)
		{
			case 1:		p_info->mod_mode = MtFeModMode_8psk;		break;
			case 2:		p_info->mod_mode = MtFeModMode_16Apsk;		break;
			case 3:		p_info->mod_mode = MtFeModMode_32Apsk;		break;
			case 4:		p_info->mod_mode = MtFeModMode_64Apsk;		break;
			case 5:		p_info->mod_mode = MtFeModMode_128Apsk;		break;
			case 6:
			case 7:		p_info->mod_mode = MtFeModMode_256Apsk;		break;
			case 0:
			default:	p_info->mod_mode = MtFeModMode_Qpsk;		break;
		}

		p_info->iVcmCycle = val_0x18 & 0x1F;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x19, &ucPLSCode);

		p_info->iPlsCode		 = ucPLSCode;
		p_info->type			 = mPLSInfoTable[ucPLSCode].mDvbType;

		if(mPLSInfoTable[ucPLSCode].bValid)
		{
			p_info->mod_mode		 = mPLSInfoTable[ucPLSCode].mModMode;
			p_info->code_rate		 = mPLSInfoTable[ucPLSCode].mCodeRate;
			p_info->is_dummy_frame	 = mPLSInfoTable[ucPLSCode].bDummyFrame ? MtFe_True : MtFe_False;
			p_info->is_pilot_on		 = mPLSInfoTable[ucPLSCode].bPilotOn ? MtFe_True : MtFe_False;
			p_info->iFrameLength	 = mPLSInfoTable[ucPLSCode].iFrameLength;
		}
		else
		{
			//p_info->mod_mode		 = mPLSInfoTable[ucPLSCode].mModMode;
			p_info->code_rate		 = mPLSInfoTable[ucPLSCode].mCodeRate;
			//p_info->is_dummy_frame	 = mPLSInfoTable[ucPLSCode].bDummyFrame ? MtFe_True : MtFe_False;
			p_info->is_pilot_on		 = mPLSInfoTable[ucPLSCode].bPilotOn ? MtFe_True : MtFe_False;
			p_info->iFrameLength	 = mPLSInfoTable[ucPLSCode].iFrameLength;
		}

#else
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x7e, &val_0x7E);
		tmp = (U8)((val_0x7E&0xC0) >> 6);
		switch(tmp)
		{
			case 0:  p_info->mod_mode = MtFeModMode_Qpsk;	break;
			case 1:  p_info->mod_mode = MtFeModMode_8psk;	break;
			case 2:  p_info->mod_mode = MtFeModMode_16Apsk;	break;
			case 3:  p_info->mod_mode = MtFeModMode_32Apsk;	break;
			default: p_info->mod_mode = MtFeModMode_Undef;	break;
		}


		p_info->is_pilot_on = ((val_0x7E & 0x20) != 0) ? MtFe_True : MtFe_False;


		tmp = (U8)(val_0x7E & 0x0f);
		switch(tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_1_4;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_1_3;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_2_5;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_3_5;		break;
			case 5:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 6:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 7:  p_info->code_rate = MtFeCodeRate_4_5;		break;
			case 8:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 9:  p_info->code_rate = MtFeCodeRate_8_9;		break;
			case 10: p_info->code_rate = MtFeCodeRate_9_10;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}
#endif


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x89, &tmp);
		p_info->is_spectrum_inv = ((tmp & 0x80) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf2, &tmp);
		tmp	&= 0x03;
		if(p_info->type == MtFeType_DVBS2X)		// DVB-S2X
		{
			switch(tmp)
			{
				case 0:  p_info->roll_off = MtFeRollOff_0p15; 	break;
				case 1:  p_info->roll_off = MtFeRollOff_0p10; 	break;
				case 2:  p_info->roll_off = MtFeRollOff_0p05; 	break;
				default: p_info->roll_off = MtFeRollOff_Undef; 	break;
			}
		}
		else									// DVB-S2
		{
			switch(tmp)
			{
				case 0:  p_info->roll_off = MtFeRollOff_0p35; 	break;
				case 1:  p_info->roll_off = MtFeRollOff_0p25; 	break;
				case 2:  p_info->roll_off = MtFeRollOff_0p20; 	break;
				default: p_info->roll_off = MtFeRollOff_Undef; 	break;
			}
		}
	}
	else
	{
		handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS;

		p_info->type = MtFeType_DVBS;

		p_info->mod_mode = MtFeModMode_Qpsk;


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xe6, &tmp);
		tmp = (U8)(tmp >> 5);
		switch(tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_7_8;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}

		p_info->is_pilot_on = MtFe_False;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xe0, &tmp);
		p_info->is_spectrum_inv = ((tmp & 0x04) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;

		p_info->roll_off = MtFeRollOff_0p35;

		p_info->is_dummy_frame = MtFe_False;
		p_info->iVcmCycle = -1;
		p_info->iPlsCode = 0;
		p_info->iFrameLength = 0;
	}

	return MtFeErr_Ok;
}


/*********************************************************************************
** Function: mt_fe_dmd_set_LNB_ct8k_ss2
**
**
** Description:	set LNB	status.
**
**
** Inputs:
**
**		Parameter		Type				Description
**	---------------------------------------------------------------
**		is_LNB_enable	MT_FE_BOOL			Enable or Disable
**		is_22k_enable	MT_FE_BOOL			Enable or Disable
**		mode			MT_FE_LNB_VOLTAGE	13V	or 18V
**		is_envelop_mode	MT_FE_BOOL			Enable or Disable
**
**
** Note:
**
**		Here the LNB setting should cooperate with the hardware circuit.
**
**		You should set the define "LNB_ENABLE_WHEN_LNB_EN_HIGH",
**	"LNB_13V_WHEN_VSEL_HIGH", "LNB_VSEL_STANDBY_HIGH" and
**	"LNB_DISEQC_OUT_STANDBY_HIGH" first according to the circuit designed.
**
**
**
** Outputs:		none
**
**
***********************************************************************************/
MT_FE_RET  mt_fe_dmd_set_LNB_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
	U8	val_0xa1 = 0, val_0xa2 = 0;


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &val_0xa1);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &val_0xa2);


	if(is_LNB_enable != MtFe_True)
	{
		val_0xa1 |= 0x40;

		/*set LNB_EN pin HIGH or LOW */
		if(handle->m_device_ss2.board_cfg.bPolarLNBEnable)
		{
			val_0xa2 &= ~0x02;
		}
		else
		{
			val_0xa2 |= 0x02;
		}


		/*set V_SEL Pin mode*/
		if(handle->m_device_ss2.board_cfg.bPolarLNBStandby)
		{
			val_0xa2 |= 0x01;
		}
		else
		{
			val_0xa2 &= ~0x01;
		}


		/*set DiSEqC_OUT mode*/
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			val_0xa2 |= 0xc0;
		}
		else
		{
			val_0xa2 &= ~0xc0;
			val_0xa2 |= 0x80;
		}
	}
	else
	{
		if(handle->m_device_ss2.board_cfg.bPolarLNBEnable)
		{
			val_0xa2 |= 0x02;
		}
		else
		{
			val_0xa2 &= ~0x02;
		}


		if(handle->m_device_ss2.board_cfg.bPolarLNB13V)
		{
			if (voltage_type == MtFeLNB_13V)
				val_0xa2 |= 0x01;
			else
				val_0xa2 &= ~0x01;
		}
		else
		{
			if (voltage_type == MtFeLNB_13V)
				val_0xa2 &= ~0x01;
			else
				val_0xa2 |= 0x01;
		}


		if (is_22k_enable == MtFe_True)
		{
			val_0xa1 |= 0x04;
			val_0xa1 &= ~0x03;
			val_0xa1 &= ~0x40;
			val_0xa2 &= ~0xc0;
		}
		else
		{
			if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
			{
				val_0xa2 |= 0xc0;
			}
			else
			{
				val_0xa2 &= ~0xc0;
				val_0xa2 |= 0x80;
			}
		}
	}

	if(is_envelop_mode != MtFe_True)
	{
		val_0xa2 &= ~0x20;
	}
	else
	{
		val_0xa2 |= 0x20;
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, val_0xa2);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, val_0xa1);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, val_0xa2);

	handle->m_device_ss2.lnb_cfg.b22K = is_22k_enable;
	handle->m_device_ss2.lnb_cfg.mLnbVoltage = voltage_type;

	return MtFeErr_Ok;
}


/*************************************************************************************
** Function: mt_fe_dmd_DiSEqC_send_tone_burst_ct8k_ss2
**
**
** Description:	send DiSEqC message in tone burst mode
**				1) modulated tone burst
**				2) unmodulated tone burst
**
**
** Inputs:
**
**		Parameter			Type						Description
**	---------------------------------------------------------------------------------
**		mode				MT_FE_DiSEqC_TONE_BURST		modulated/unmodulated tone burst
**		is_envelop_mode		MT_FE_BOOL
**
**
** Outputs:		none
**
**
**************************************************************************************/
MT_FE_RET  mt_fe_dmd_DiSEqC_send_tone_burst_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
	U8	val = 0;
	S32	time_out = 0;


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &val);
	val &= ~0xc0;
	if(is_envelop_mode != MtFe_True)
	{
		val &= ~0x20;
	}
	else
	{
		val |= 0x20;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, val);


	if (mode == MtFeDiSEqCToneBurst_Moulated)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, 0x01);
	else
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, 0x02);

	_mt_sleep_ct8k(13);

	time_out = 5;
	do
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &val);
		if ((val & 0x40) == 0)
			break;

		_mt_delay_ct8k(1);
		time_out --;

	} while (time_out > 0);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &val);
	if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
	{
		val |= 0xc0;
	}
	else
	{
		val &= ~0xc0;
		val |= 0x80;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, val);

	return MtFeErr_Ok;
}

/*****************************************************************************
** Function: mt_fe_dmd_DiSEqC_send_receive_msg_ct8k_ss2
**
**
** Description:	send data with reply message by device in DiSEqC mode.
**
**
** Inputs:
**
**	  Parameter			Type				Description
**	----------------------------------------------------------------------
**	  msg				MT_FE_DiSEqC_MSG	DiSEqC message pointer
**
**
** Outputs:		none
**
**
**
*******************************************************************************/
MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i = 0;
	U8	tmp = 0, tmp1 = 0;
	S32	time_out = 0;

	if (msg->size_send > 8)
		return MtFeErr_Param;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if(msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i ++)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(10);
		time_out -= 10;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	if (!(msg->is_enable_receive))
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		_mt_sleep_ct8k(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1 = tmp;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(15);
		time_out -= 15;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i ++)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Fail;
	}


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i = 0;
	U8	tmp = 0/*, tmp1 = 0*/;
	S32	time_out = 0;

	if (msg->size_send > 8)
		return MtFeErr_Param;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if(msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i ++)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(10);
		time_out -= 10;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	return MtFeErr_Ok;

#if 0
	if (!(msg->is_enable_receive))
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		_mt_sleep_ct8k(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1 = tmp;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(15);
		time_out -= 15;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i ++)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Fail;
	}


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	return MtFeErr_Ok;
#endif
}

/* user must call send message function before call recieve message function */
MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i = 0;
	U8	tmp = 0, tmp1 = 0;
	S32	time_out = 0;

#if 0
	if (msg->size_send > 8)
		return MtFeErr_Param;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if(msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i ++)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(10);
		time_out -= 10;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}
#endif


	if (!(msg->is_enable_receive))
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		_mt_sleep_ct8k(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1 = tmp;
	while (time_out > 0)
	{
		_mt_sleep_ct8k(15);
		time_out -= 15;

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i ++)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);


		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
		if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);

		return MtFeErr_Fail;
	}


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa1, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xa2, &tmp);
	if(handle->m_device_ss2.board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xa2, tmp);


	return MtFeErr_Ok;
}


/**********************************************************************
** Function: mt_fe_blindscan_abort_ct8k_ss2
**
**
** Description:	cancel blind scan process
**
**
** Inputs:
**
**		Parameter		type					description
**	-------------------------------------------------------------------
**		bool			MT_FE_BOOL				MtFe_True: cancel
**
**
** Outputs:		none
**
**
************************************************************************/
MT_FE_RET mt_fe_dmd_blindscan_abort_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BOOL bs_abort)
{
	handle->m_device_ss2.global_cfg.bCancelBs = (bs_abort == MtFe_True) ? TRUE : FALSE;

	return MtFeErr_Ok;
}



/*********************************************************************************
** Function: mt_fe_dmd_register_blindscan_callback_ct8k_ss2
**
**
** Description:	register the blindscan callback function
**
**
** Inputs:
**
**		Parameter		type					description
**	-----------------------------------------------------------------------------
**		callback		MT_FE_RET (*)()			callback function pointer
**
**
** Outputs:		none
**
**
**********************************************************************************/
MT_FE_RET mt_fe_dmd_register_notify_ct8k_ss2(void (*callback)(MT_FE_MSG msg, void *p_tp_info))
{
	if(callback == NULL)
		return MtFeErr_NullPointer;

	mt_fe_bs_notify_ct8k = callback;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_get_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *p_MCLK_KHz)
{
	U8 reg10 = 0, reg0E = 0, reg0F = 0;

	_mt_fe_sys_get_reg_ct8k(handle, 0x10, &reg10);
	_mt_fe_sys_get_reg_ct8k(handle, 0x0F, &reg0F);
	_mt_fe_sys_get_reg_ct8k(handle, 0x0E, &reg0E);

	if((reg10 == 0x24) && (reg0F == 0x09) && (reg0E == 0x90))
	{
		*p_MCLK_KHz = 192000;
	}
	else if((reg10 == 0x24) && (reg0F == 0x0C) && (reg0E == 0xC0))
	{
		*p_MCLK_KHz = 144000;
	}
	else if((reg10 == 0x26) && (reg0F == 0x0A) && (reg0E == 0xAA))
	{
		*p_MCLK_KHz = 115200;
	}
	else if((reg10 == 0x27) && (reg0F == 0xCC) && (reg0E == 0xCC))
	{
		*p_MCLK_KHz = 72000;
	}
	else
	{
		*p_MCLK_KHz = 96000;
	}

	return MtFeErr_Ok;
}

/******************************************************************
** Function: _mt_fe_dmd_set_mclk_ct8k_ss2
**
**
** Description:	set TS Clock Ratio for DS3002B/DS3103/DS3103B.
**
**
** Inputs:
**	MCLK_KHz		U32				The MCLK(KHz) to be set.
**
**
** Outputs:		None
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_set_mclk_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 MCLK_KHz)
{
	switch(MCLK_KHz)
	{
		case 192000:
			_mt_fe_sys_set_reg_ct8k(handle, 0x10, 0x24);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0f, 0x09);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0e, 0x90);
			break;

		case 144000:
			_mt_fe_sys_set_reg_ct8k(handle, 0x10, 0x24);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0F, 0x0C);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0E, 0xC0);
			break;

		case 115200:
			_mt_fe_sys_set_reg_ct8k(handle, 0x10, 0x26);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0F, 0x0A);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0E, 0xAA);
			break;

		case 72000:
			_mt_fe_sys_set_reg_ct8k(handle, 0x10, 0x27);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0F, 0xCC);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0E, 0xCC);
			break;

		case 96000:
		default:
			_mt_fe_sys_set_reg_ct8k(handle, 0x10, 0x27);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0F, 0x99);
			_mt_fe_sys_set_reg_ct8k(handle, 0x0E, 0x99);
			break;
	}

	_mt_delay_ct8k(5);

	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_set_ts_divide_ratio_ct8k_ss2
**
**
** Description:	set the TS divide ratio
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	type			MT_FE_TYPE	MtFeType_DVBS2 or MtFeType_DVBS.
**	dr_high			U8			The length of high level of the MPEG clock.
**	dr_low			U8			The length of low level of the MPEG clock.
**
**
** Outputs:		none
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_set_ts_divide_ratio_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TYPE type, U8 dr_high, U8 dr_low)
{
	U8 val = 0, tmp1 = 0, tmp2 = 0;

	tmp1 = dr_high;
	tmp2 = dr_low;

#if 0
	if(type == MtFeType_DVBS2)
	{
		tmp1 -= 1;
		tmp2 -= 1;

		tmp1 &= 0x3f;
		tmp2 &= 0x3f;

		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xfe, 0x20);	// default value

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xfe, &val);		// ci_divrange_h_1 bits[5:2]
		val &= 0xF0;						// bits[3:0]
		val |= (tmp1 >> 2) & 0x0f;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xfe, val);

		val = (U8)((tmp1 & 0x03) << 6);		// ci_divrange_h_0 bits[1:0]
		val |= tmp2;						// ci_divrange_l   bits[5:0]
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xea, val);
	}
	else	// DVB-S
#endif
	{
		tmp1 -= 1;
		tmp2 -= 1;

		tmp1 &= 0x3f;
		tmp2 &= 0x3f;

		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xfe, 0x20);	// default value

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xfe, &val);		// ci_divrange_h_1 bits[5:2]
		val &= 0xF0;						// bits[3:0]
		val |= (tmp1 >> 2) & 0x0f;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xfe, val);

		val = (U8)((tmp1 & 0x03) << 6);		// ci_divrange_h_0 bits[1:0]
		val |= tmp2;						// ci_divrange_l   bits[5:0]
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xea, val);
	}

	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_set_ts_out_mode_ct8k_ss2
**
**
** Description:	set the TS output mode
**
**
** Inputs:
**
**	Parameter				Type				Description
**	----------------------------------------------------------
**	mode					MT_FE_TS_OUT_MODE	passed by up level
**
**	MtFeTsOutMode_Serial	1					Serial
**	MtFeTsOutMode_Parallel	2					Parallel
**	MtFeTsOutMode_Common	3					Common Interface
**
** Outputs:		none
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_set_ts_out_mode_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TS_OUT_MODE mode)
{

	return MtFeErr_Ok;
}


/*****************************************************************************
** Function: _mt_fe_dmd_set_demod_ct8k_ss2
**
**
** Description:	set some registers according the param.
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------------------
**	sym_rate_KSs	U32			symbol rate passed by app level
**	type			MT_FE_TYPE	passed by up level
**
**
** Outputs:
**
**
******************************************************************************/
MT_FE_RET _mt_fe_dmd_set_demod_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8			val, tmp;

	_mt_fe_dmd_init_reg_ct8k_ss2(handle, reg_tbl_def_ct8k_ss2, sizeof(reg_tbl_def_ct8k_ss2)/2);

	if((sym_rate_KSs > 47100) && (sym_rate_KSs < 47500))
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xe6, 0x00);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xe7, 0x03);
	}

	if(handle->m_device_ss2.board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val |= 0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val &= ~0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val);
	val &= 0x7f;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, (U8)(handle->m_device_ss2.board_cfg.bAGCPolar ? 0x18 : 0x08));

	if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS2022)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_RDA5815M)
		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0xbd);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x5a);
	else
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x35);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xc9, &tmp);
	tmp |= 0x08;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc9, tmp);

	if(sym_rate_KSs <= 3000)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08); // 8 * 32 * 100 / 64 = 400
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x20);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x00);
	}
	else if(sym_rate_KSs <= 10000)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08); // 8 * 16 * 100 / 64 = 200
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x10);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x00);
	}
	else
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08); // 8 * 6 * 100 / 64 = 75
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x06);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x00);
	}

	_mt_fe_dmd_set_sym_rate_ct8k_ss2(handle, sym_rate_KSs);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x76, &val);
	val &= ~0x80;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x76, val);


	if(handle->m_device_ss2.tp_cfg.bHavePLS)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, handle->m_device_ss2.tp_cfg.ucPLSCode[0]);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, handle->m_device_ss2.tp_cfg.ucPLSCode[1]);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, handle->m_device_ss2.tp_cfg.ucPLSCode[2]);
	}
	else
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, 0x01);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, 0x00);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, 0x00);
	}

#if 0
	if(sym_rate_KSs > 5000)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x63, 0x60);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x64, 0x30);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x65, 0x40);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x68, 0x26);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x69, 0x4c);
	}
#endif

	return ret;
}



/*****************************************************************************
** Function: _mt_fe_dmd_set_sym_rate_ct8k_ss2
**
**
** Description:	set symbol rate
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------------------
**	sym_rate_KSs	U32			symbol rate passed by app level
**
**
** Outputs:
**
**
*******************************************************************************/
MT_FE_RET _mt_fe_dmd_set_sym_rate_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs)
{
	U32	tmp = 0;
	U8	reg_0x61 = 0, reg_0x62 = 0;

	tmp = ((sym_rate_KSs << 15) + (handle->m_device_ss2.global_cfg.iMclkKHz / 4)) / (handle->m_device_ss2.global_cfg.iMclkKHz / 2);

	reg_0x61 = (U8)(tmp & 0x00FF);
	reg_0x62 = (U8)((tmp & 0xFF00) >> 8);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x61, reg_0x61);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x62, reg_0x62);


	return MtFeErr_Ok;
}



/*****************************************************************************
** Function: _mt_fe_dmd_get_sym_rate_ct8k_ss2
**
**
** Description:	get symbol rate from register
**
**
** Inputs:	none
**
**
**
** Outputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------------------
**	sym_rate_KSs	U32*		symbol rate read from register
**
**
*******************************************************************************/
 MT_FE_RET _mt_fe_dmd_get_sym_rate_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *sym_rate_KSs)
{
	U16	tmp = 0;
	U32	sym_rate_tmp = 0;
	U8	val_0x6d = 0, val_0x6e = 0;


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x6d, &val_0x6d);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x6e, &val_0x6e);

	tmp = (U16)((val_0x6e << 8) | val_0x6d);


	sym_rate_tmp = tmp * handle->m_device_ss2.global_cfg.iMclkKHz;
	sym_rate_tmp /= (1 << 16);

	*sym_rate_KSs = sym_rate_tmp;


	return MtFeErr_Ok;
}



/*************************************************************************
** Function:  _mt_fe_dmd_set_carrier_offset_ct8k_ss2
**
**
** Description:	set blind scan registers
**
**
** Inputs:
**
**	Parameter				Type		Description
**	----------------------------------------------------------------------
**	carrier_offset_KHz		U32			carrier offset
**
**
** Outputs:		none
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_set_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 carrier_offset_KHz)
{
	S32 tmp = 0;


	tmp = carrier_offset_KHz;
	tmp *= 65536;

	tmp = (2 * tmp + handle->m_device_ss2.global_cfg.iMclkKHz) / (2 * handle->m_device_ss2.global_cfg.iMclkKHz);

	if(tmp < 0)
		tmp += 65536;


	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5f, (U8)(tmp >> 8));
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5e, (U8)(tmp & 0xFF));

	handle->m_device_ss2.tp_cfg.iCarrierOffsetKHz = carrier_offset_KHz;

	return MtFeErr_Ok;
}


/***********************************************************************
** Function: _mt_fe_dmd_get_carrier_offset_ct8k_ss2
**
**
** Description:	get the carrier offset
**
**
** Inputs:	none
**
**
** Outputs:
**
**	Parameter				Type		Description
**	--------------------------------------------------------------------
**	carrier_offset_KHz		S32*		carrier offset value
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_get_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 *carrier_offset_KHz)
{
	S32 val = 0;
	U8	tmp = 0, val_0x5e = 0, val_0x5f = 0;


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5d, &tmp);
	tmp &= 0xf8;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5d, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5e, &val_0x5e);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5f, &val_0x5f);
	val = (val_0x5f<<8) | val_0x5e;


	if((val & 0x8000) == 0x8000)
		val -= (1<<16);


	*carrier_offset_KHz = (val * handle->m_device_ss2.global_cfg.iMclkKHz) / (1<<16);

	return MtFeErr_Ok;
}


/***********************************************************************
** Function: _mt_fe_dmd_get_total_carrier_offset_ct8k_ss2
**
**
** Description:	get the total carrier offset
**
**
** Inputs:	none
**
**
** Outputs:
**
**	Parameter				Type		Description
**	--------------------------------------------------------------------
**	carrier_offset_KHz		S32*		carrier offset value
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_get_total_carrier_offset_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, S32 *carrier_offset_KHz)
{
	U8	tmp = 0, val_0x5e = 0, val_0x5f = 0;
	S32 val_0x5e_0x5f_1 = 0, val_0x5e_0x5f_2 = 0, nval_1 = 0, nval_2 = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5d, &tmp);
	tmp &= 0xf8;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5d, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5e, &val_0x5e);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5f, &val_0x5f);
	val_0x5e_0x5f_1 = (val_0x5f << 8) | val_0x5e;

	tmp |= 0x06;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5d, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5e, &val_0x5e);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x5f, &val_0x5f);
	val_0x5e_0x5f_2 = (val_0x5f << 8) | val_0x5e;


	if(((val_0x5e_0x5f_1 >> 15) & 0x01) == 0x01)
		nval_1 = val_0x5e_0x5f_1 - (1 << 16);
	else
		nval_1 = val_0x5e_0x5f_1;

	if(((val_0x5e_0x5f_2 >> 15) & 0x01) == 0x01)
		nval_2 = val_0x5e_0x5f_2 - (1 << 16);
	else
		nval_2 = val_0x5e_0x5f_2;

	*carrier_offset_KHz = (nval_1 - nval_2) * handle->m_device_ss2.global_cfg.iMclkKHz / (1 << 16);

	return MtFeErr_Ok;
}



/*************************************************************************
** Function:  _mt_fe_dmd_get_fec_ct8k_ss2
**
**
** Description:	get code rate
**
**
** Inputs:
**
**	Parameter			Type				Description
**	----------------------------------------------------------------------
**	tp_type				MT_FE_TYPE			DVBS/DVBS2
**
**
** Outputs:
**
**	Parameter			Type				Description
**	----------------------------------------------------------------------
**	p_info				MT_FE_TP_INFO*		structure pointer
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_get_fec_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type)
{
	U8 tmp = 0, val_0x7E = 0;


	if(tp_type == MtFeType_DVBS2)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x7e, &val_0x7E);
		tmp = (U8)(val_0x7E & 0x0f);
		switch(tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_1_4;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_1_3;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_2_5;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_3_5;		break;
			case 5:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 6:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 7:  p_info->code_rate = MtFeCodeRate_4_5;		break;
			case 8:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 9:  p_info->code_rate = MtFeCodeRate_8_9;		break;
			case 10: p_info->code_rate = MtFeCodeRate_9_10;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xe6, &tmp);
		tmp = (U8)(tmp >> 5);
		switch(tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_7_8;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}
	}

	return MtFeErr_Ok;
}

/**********************************************************************
** Function: _mt_fe_dmd_init_reg_ct8k_ss2
**
**
** Description:	initialize the register of Symphony2 satellite module
**
**
** Inputs:
*
**	Parameter		Type				Description
**	---------------------------------------------------------------
**	p_reg_tabl		const U8 *[2]		pointer to the array
**	size			S32					the array size
**
** Outputs:
**
**
***********************************************************************/
MT_FE_RET _mt_fe_dmd_init_reg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U32			i = 0;

	for (i=0; i < (U32)size; i++)
	{
		ret = _mt_fe_dmd_set_reg_ss2_ct8k(handle, p_reg_tabl[i][0], p_reg_tabl[i][1]);

		if (ret != MtFeErr_Ok)
			return ret;
	}

	return ret;
}



/*******************************************************************
** Function: _mt_fe_dmd_download_fw_ct8k_ss2
**
**
** Description:	download the mcu code to Symphony2 satellite module
**
**
** Inputs:
**
**	Parameter	Type		Description
**	------------------------------------------------------------
**	p_fw		const U8*	pointer of the mcu array
**
** Outputs:
**
**
********************************************************************/
MT_FE_RET _mt_fe_dmd_download_fw_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, const U8* p_fw, U16 fw_size)
{
	U32 i = 0;
	U8 tmp;
	MT_FE_RET	ret = MtFeErr_Ok;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb9, &tmp);
	if(tmp == ct8k_ss2_fm_ver)
	{
		handle->m_device_ss2.mcu_status = 1;
	}
	else
	{
		handle->m_device_ss2.mcu_status = 0;
	}

	if (handle->m_device_ss2.mcu_status == 0)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);

#if 0
		for(i = 0; i < 192; i++)
		{
			ret = _mt_fe_dmd_write_fw_ct8k(handle->m_device_ss2.demod_dev_addr, 0xb0, (U8*)&(p_fw[64 * i]), 64);
			if (ret != MtFeErr_Ok)
			{
				break;
			}
		}
#else
		i = 0;

		while (fw_size > 0)
		{
			if (fw_size >= 64)
			{
				ret = _mt_fe_dmd_write_fw_ct8k(handle->m_device_ss2.demod_dev_addr, 0xb0, (U8 *)&(p_fw[64 * i]), 64);
				fw_size -= 64;
			}
			else
			{
				ret = _mt_fe_dmd_write_fw_ct8k(handle->m_device_ss2.demod_dev_addr, 0xb0, (U8 *)&(p_fw[64 * i]), fw_size);
				fw_size = 0;
			}

			i ++;

			if (ret != MtFeErr_Ok)
			{
				break;
			}
		}
#endif
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);

		if (ret != MtFeErr_Ok)
			return MtFeErr_I2cErr;

		_mt_sleep_ct8k(50);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb9, &tmp);
		if(tmp == ct8k_ss2_fm_ver)
		{
			handle->m_device_ss2.mcu_status = 1;
		}
		else
		{
			handle->m_device_ss2.mcu_status = 0;
		}
	}

	return MtFeErr_Ok;
}


/*************************************************************************
** Function: _mt_fe_dmd_bs_set_reg_ct8k_ss2
**
**
** Description:	set blind scan registers
**
**
** Inputs:
**
**	Parameter		type				description
**	---------------------------------------------------------------------------
**	bs_times		U8					blind scan times
**
**
** Outputs:		none
**
**
*************************************************************************/
MT_FE_RET _mt_fe_dmd_bs_set_reg_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U8 bs_times)
{
	U8 tmp = 0, sm_buf = 0;

	U8 TMP1, TMP2, TMP3, TMP4;

	TMP1 = (handle->m_device_ss2.bs_cfg.ave_times / 32 - 1) * 16 + (handle->m_device_ss2.bs_cfg.data_length / 128 - 1);	// = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
	TMP2 = 64 + handle->m_device_ss2.bs_cfg.psd_overlap / 16;												// = (64 + 96/16) = (64 + 6) = 0x46
	TMP3 = handle->m_device_ss2.bs_cfg.psd_scale * 32;														// = (0*32) = 0
	TMP4 = handle->m_device_ss2.bs_cfg.notch_range_f * 16 + handle->m_device_ss2.bs_cfg.start_range_f;					// = (8*16 + 15) = 0x8F

	switch(bs_times)
	{
		case MT_FE_BS_TIMES_1ST:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 1)
				sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_2nd;		// ONE_LOOP_SM_BUF
			else
				sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_1st;		// SM_BUF_1ST		// TWO_LOOP_SM_BUF_1ST
			break;

		case MT_FE_BS_TIMES_2ND:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 2)
				sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_3rd;		// TWO_LOOP_SM_BUF_2ND
			else
				sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_2nd;		// SM_BUF_2ND
			break;

		case MT_FE_BS_TIMES_3RD:
			sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_3rd;			// SM_BUF_3RD;
			break;

		default:
			sm_buf = handle->m_device_ss2.bs_cfg.sm_buf_2nd;			// ONE_LOOP_SM_BUF;
			break;
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xB2, 0x01);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x01);


	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4A, 0x00);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4D, &tmp);
	tmp |= 0x91;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4D, tmp);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x90, &tmp);
	tmp |= TMP1;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x90, tmp);


	if(sm_buf == handle->m_device_ss2.bs_cfg.sm_buf_1st)
		tmp = 128 + TMP2;
	else
		tmp = TMP2;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x91, tmp);

	tmp = (U8)(TMP3 + sm_buf - 1);
	tmp |= 0x80;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x92, tmp);


	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x93, &tmp);
	tmp &= 0x0F;
	tmp |= TMP4;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x93, tmp);



	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x94, &tmp);
	switch(bs_times)
	{
		case MT_FE_BS_TIMES_1ST:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 1)
				tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_2nd);		// THR_FACTOR_ONE_LOOP
			else
				tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_1st);		// TWO_LOOP_THR_FACTOR_1ST
			break;

		case MT_FE_BS_TIMES_2ND:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 2)
				tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_3rd);
			else
				tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_2nd);
			break;

		case MT_FE_BS_TIMES_3RD:
			tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_3rd);
			break;

		default:
			tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_2nd);
			break;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x94, tmp);


	switch(bs_times)
	{
		case MT_FE_BS_TIMES_1ST:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 1)
				tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_2nd;
			else
				tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_1st;
			break;

		case MT_FE_BS_TIMES_2ND:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 2)
				tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_3rd;
			else
				tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_2nd;
			break;

		case MT_FE_BS_TIMES_3RD:
			tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_3rd;
			break;

		default:
			tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_2nd;
			break;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x95, tmp);


	switch(bs_times)
	{
		case MT_FE_BS_TIMES_1ST:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 1)
				tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_2nd / 16 - 1;
			else
				tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_1st / 16 - 1;

			break;

		case MT_FE_BS_TIMES_2ND:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 2)
				tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_3rd / 16 - 1;
			else
				tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_2nd / 16 - 1;
			break;

		case MT_FE_BS_TIMES_3RD:
			tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_3rd / 16 - 1;
			break;

		default:
			tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_2nd / 16 - 1;
			break;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x97, tmp);


	switch(bs_times)
	{
		case MT_FE_BS_TIMES_1ST:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 1)
				tmp = handle->m_device_ss2.bs_cfg.flat_thr_2nd;
			else
				tmp = handle->m_device_ss2.bs_cfg.flat_thr_1st;
			break;

		case MT_FE_BS_TIMES_2ND:
			if(handle->m_device_ss2.global_cfg.iTotalBsTimes == 2)
				tmp = handle->m_device_ss2.bs_cfg.flat_thr_3rd;
			else
				tmp = handle->m_device_ss2.bs_cfg.flat_thr_2nd;
			break;

		case MT_FE_BS_TIMES_3RD:
			tmp = handle->m_device_ss2.bs_cfg.flat_thr_3rd;
			break;

		default:
			tmp = handle->m_device_ss2.bs_cfg.flat_thr_2nd;
			break;
	}
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x99, tmp);


	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, (U8)((handle->m_device_ss2.board_cfg.bAGCPolar) ? 0x18 : 0x08));

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x32, 0x44);

	//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);

	if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS2022)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_RDA5815M)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0xbd);
		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x5a);
	else
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x35);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x35, 0x7F);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4B, 0x04);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x56, 0x01);
	//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xA0, 0x44);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, 0x83);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x70, 0x00);


	if(handle->m_device_ss2.board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &tmp);
		tmp |= 0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, tmp);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &tmp);
		tmp &= ~0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, tmp);
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xB2, 0x00);

	return MtFeErr_Ok;
}


/*****************************************************************************
** Function: _mt_fe_dmd_bs_set_demod_ct8k_ss2
**
**
** Description:	set demod registers in blind scan mode
**
**
** Inputs:
**
**	Parameter				type				description
**	---------------------------------------------------------------------------
**	sym_rate_KSs			U32					symbol rate passed by app level
**	type					MT_FE_TYPE			passed by up level
**
**
** Outputs:
**
**
******************************************************************************/
MT_FE_RET _mt_fe_dmd_bs_set_demod_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type)
{
	U8			val;
	MT_FE_RET	ret = MtFeErr_Ok;

	ret = _mt_fe_dmd_init_reg_ct8k_ss2(handle, reg_tbl_def_blindscan_ct8k_ss2, sizeof(reg_tbl_def_blindscan_ct8k_ss2)/2);


	if(handle->m_device_ss2.board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val |= 0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &val);
		val &= ~0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, val);
	}

	if(handle->m_device_ss2.board_cfg.bAGCPolar)			// 0x30, bit 4
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x30, &val);
		val |= 0x10;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, val);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x30, &val);
		val &= ~0x10;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, val);
	}

	if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS2022)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_RDA5815M)
		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0xbd);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x5a);
	else
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x35);

	if(sym_rate_KSs <= 2500)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x02);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x04);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x0b);
	}
	else if(sym_rate_KSs <= 5000)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x02);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x04);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x0b);
	}
#if 0
	else if(sym_rate_KSs <= 20000)
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x02);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x04);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x16);
	}
#endif
	else
	{
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc3, 0x08);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc8, 0x02);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc4, 0x04);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc7, 0x16);
	}

	_mt_fe_dmd_set_sym_rate_ct8k_ss2(handle, sym_rate_KSs);

	return ret;
}


/********************************************************************************************
** Function: _mt_fe_dmd_bs_remove_unlocked_TP_ct8k_ss2
**
**
** Description:	remove the unlocked TP if the TP's freq. is lower than the compared freq.
**
**
** Inputs:
**
**	Parameter			type									description
**	-----------------------------------------------------------------------------------------
**	p_bs_info			MT_FE_BS_TP_INFO *						blind scan struture pointer
**	compare_freq_MHz	U32										the campared TP freq.
**
** Outputs:		none
**
**
*********************************************************************************************/
MT_FE_RET _mt_fe_dmd_bs_remove_unlocked_TP_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz)
{
	U16	 i = 0;
	U16	 scanned_TP_num = 0, index = 0, move_steps = 0, sym_tmp = 0;
	U32  fre_tmp = 0;

	scanned_TP_num = handle->m_device_ss2.global_cfg.iScannedTpCnt;

	for(i = 0; i < scanned_TP_num; i++)
	{
		if(p_bs_info->p_tp_info[i].freq_KHz >= compare_freq_KHz)
		{
			index = i;
			break;
		}
	}


	if(i == scanned_TP_num)
		index = scanned_TP_num;


	i = 0;
	move_steps = 0;

	while(i <= index)
	{
		if(p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked)
		{
			i ++;
			continue;
		}

		while((p_bs_info->p_tp_info[i].dvb_type == MtFeType_DTV_Checked) && (i <= index))
		{
			move_steps ++;
			i ++;
		}

		while((p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked) && (i <= index))
		{
			fre_tmp = p_bs_info->p_tp_info[i - move_steps].freq_KHz;
			sym_tmp = p_bs_info->p_tp_info[i - move_steps].sym_rate_KSs;

			p_bs_info->p_tp_info[i - move_steps].freq_KHz     = p_bs_info->p_tp_info[i].freq_KHz;
			p_bs_info->p_tp_info[i - move_steps].sym_rate_KSs = p_bs_info->p_tp_info[i].sym_rate_KSs;
			p_bs_info->p_tp_info[i - move_steps].dvb_type     = p_bs_info->p_tp_info[i].dvb_type;
			p_bs_info->p_tp_info[i - move_steps].code_rate    = p_bs_info->p_tp_info[i].code_rate;

			p_bs_info->p_tp_info[i].freq_KHz	 = fre_tmp;
			p_bs_info->p_tp_info[i].sym_rate_KSs = sym_tmp;
			p_bs_info->p_tp_info[i].dvb_type	 = MtFeType_DTV_Checked;
			p_bs_info->p_tp_info[i].code_rate	 = MtFeCodeRate_Undef;

			i ++;
		}
	}

	return MtFeErr_Ok;
}


/************************************************************************************
** Function: _mt_fe_dmd_bs_connect_ct8k_ss2
**
**
** Description:	try to lock the scanned TPs according to the start TP index and the
**				TP numbers need to lock
**
**
** Inputs:
**
**		Parameter		type				description
**	--------------------------------------------------------------------
**
** Outputs:
**
**		Parameter		type				description
**	---------------------------------------------------------------------------------
**		p_bs_info		MT_FE_BS_TP_INFO *	the blindscan TP pointer
**
**
*************************************************************************************/
MT_FE_RET _mt_fe_dmd_bs_connect_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info)
{
	U16		index = 0, timing_thr = 0, crl_thr = 0;
	U8		count = 0, timing_fail = 0, /*crl_fail, */tmp = 0, same_locked_TP_flag = 0, val_reg08 = 0, ucTmp = 0;
	U32		sym_rate_KSs = 0, centerfreq_KHz = 0, truefreq_KHz = 0;
	S16		lpf_offset_KHz, i;
	S32		carrieroffset_KHz = 0, delta_freq_KHz = 0, delta_smy_rate_KSs = 0, deviceoffset_KHz = 0;

	MT_FE_RET		ret = MtFeErr_Ok;

	int iCnt = 0;

	U32 real_freq_KHz = 0;
	S32 tuner_offset_KHZ = 0;

	MT_FE_TYPE			current_mode = MtFeType_DVBS;
	MT_FE_LOCK_STATE	lockstate;
	MT_FE_TP_INFO 		*p_scanned_tp_info = (MT_FE_TP_INFO *)(p_bs_info->p_tp_info);

#if 0
	U32 gain_val = 0, delta = 0;
#endif

	//if((mt_fe_tn_set_tuner == NULL) || (mt_fe_tn_get_offset == NULL))
	if(handle->m_device_ss2.tuner_cfg.tuner_set == NULL)
		return MtFeErr_NullPointer;

	//for(index = start_index; index < scanned_tp; index++)
	for(index = 0; index < handle->m_device_ss2.global_cfg.iScannedTpCnt; index ++)
	{
		if(handle->m_device_ss2.global_cfg.bCancelBs) break;

		if(p_scanned_tp_info[index].dvb_type != MtFeType_DTV_Unknown)
			continue;

		p_scanned_tp_info[index].bCheckCCM = FALSE;
		p_scanned_tp_info[index].iTsCnt = 0;
		p_scanned_tp_info[index].ucCurTsId = 0;

		// fix 106074 start
		handle->m_device_ss2.tp_cfg.bCheckCCM = FALSE;
		handle->m_device_ss2.tp_cfg.iTsCnt = 0;
		handle->m_device_ss2.tp_cfg.ucCurTsId = 0;

		for(i = 0; i < 16; i ++)
		{
			p_scanned_tp_info[index].ucTsId[i] = 0;
			handle->m_device_ss2.tp_cfg.ucTsId[i] = 0;
		}

		p_scanned_tp_info[index].bHavePLS = FALSE;
		handle->m_device_ss2.tp_cfg.bHavePLS = FALSE;
		for(i = 0; i < 3; i ++)
		{
			p_scanned_tp_info[index].ucPLSCode[i] = 0;
			handle->m_device_ss2.tp_cfg.ucPLSCode[i] = 0;
		}

		centerfreq_KHz	= p_scanned_tp_info[index].freq_KHz;
		sym_rate_KSs	= p_scanned_tp_info[index].sym_rate_KSs;

		handle->m_device_ss2.tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;
		handle->m_device_ss2.tp_cfg.iSymRateKSs = sym_rate_KSs;

		lpf_offset_KHz = 0;
		truefreq_KHz   = 0;

		mt_fe_dmd_global_reset_ct8k_ss2(handle);

		mt_fe_dmd_clear_ts_ct8k_ss2(handle);

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);

		if ((sym_rate_KSs < 5000) && (!handle->m_device_ss2.lnb_cfg.bUnicable))
		{
			lpf_offset_KHz	  = 0;//BLINDSCAN_LPF_OFFSET_KHz;
			centerfreq_KHz	 += 0;//BLINDSCAN_LPF_OFFSET_KHz;
		}

		mt_fe_dmd_config_clock_ct8k(handle, MtFeType_DVBS_S2);

		if(handle->m_device_ss2.lnb_cfg.bUnicable)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			ret = mt_fe_unicable_set_tuner_ct8k_ss2(handle, centerfreq_KHz,
			   sym_rate_KSs, &real_freq_KHz,
			   handle->m_device_ss2.lnb_cfg.iUBIndex,
			   handle->m_device_ss2.lnb_cfg.iBankIndex,
			   handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
			deviceoffset_KHz = real_freq_KHz - centerfreq_KHz;
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}
		else
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			//ret = mt_fe_tn_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
			handle->m_device_ss2.tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
			real_freq_KHz = centerfreq_KHz;
			deviceoffset_KHz = 0;
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}

		if(ret != MtFeErr_Ok)		// Error parameter
		{
			lockstate = MtFeLockState_Unlocked;
			break;
		}

		_mt_fe_dmd_bs_set_demod_ct8k_ss2(handle, sym_rate_KSs, current_mode);

		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &val_reg08);
		val_reg08 = (U8)(val_reg08 & 0x3b);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, val_reg08);
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xe0, 0xf8);


		if(sym_rate_KSs <= 5000)
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc0, 0x04);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8a, 0x09);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8b, 0x22);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8c, 0x88);
		}

		if (handle->m_device_ss2.tuner_cfg.tuner_get_offset != NULL)
		{
			mt_fe_i2c_repeat_enable_ct8k(handle);
			handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &tuner_offset_KHZ);
			mt_fe_i2c_repeat_disable_ct8k(handle);
		}

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);

		_mt_sleep_ct8k(70);

		lockstate = MtFeLockState_Waiting;

		if(sym_rate_KSs >= 10000)
		{
			count = 25;
			timing_thr = 1;
			crl_thr = 1;
		}
		else if(sym_rate_KSs >= 4000)
		{
			count = 28;
			timing_thr = 1;
			crl_thr = 1;
		}
		else if(sym_rate_KSs >= 2000)
		{
			count = 35;
			timing_thr = 1;
			crl_thr = 1;
		}
		else
		{
			count = 40;
			timing_thr = 1;
			crl_thr = 1;
		}

		timing_fail = 0;
		//crl_fail = 0;
		lockstate = MtFeLockState_Unlocked;

		while((count > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs))
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &tmp);
			if(tmp == 0xfe)
			{
				U8 tmp1;

				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xae, 0x4f);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x7f, 0xc4);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, 0xc0);

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
				ucTmp |= 0x04;
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, ucTmp);

				mt_fe_dmd_soft_reset_ct8k_ss2(handle);

				iCnt = 0;

				do
				{
					iCnt ++;
					if(iCnt > 1000)
					{
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, 0x01);
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, 0x00);
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, 0x00);

						mt_fe_dmd_soft_reset_ct8k_ss2(handle);

						return MtFeErr_TimeOut;
					}

					_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x24, &tmp1);

					_mt_delay_ct8k(1);
				}while((tmp1 & 0x08) == 0x00);

				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xae, 0x09);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x7f, 0x04);

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
				ucTmp &= ~0x04;
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, ucTmp);

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x22, &tmp1);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x22, tmp1);
				p_scanned_tp_info[index].ucPLSCode[0] = tmp1;
				handle->m_device_ss2.tp_cfg.ucPLSCode[0] = tmp1;

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x23, &tmp1);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x23, tmp1);
				p_scanned_tp_info[index].ucPLSCode[1] = tmp1;
				handle->m_device_ss2.tp_cfg.ucPLSCode[1] = tmp1;

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x24, &tmp1);
				tmp1 &= 0x03;
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x24, tmp1);
				p_scanned_tp_info[index].ucPLSCode[2] = tmp1;
				handle->m_device_ss2.tp_cfg.ucPLSCode[2] = tmp1;

				p_scanned_tp_info[index].bHavePLS = TRUE;
				handle->m_device_ss2.tp_cfg.bHavePLS = TRUE;

				mt_fe_dmd_soft_reset_ct8k_ss2(handle);
			}


			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp);
			if (tmp & 0x08)
			{
				current_mode = MtFeType_DVBS2;
				handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS2;

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
				if((tmp & 0x0f) == 0x0f)
				{
					_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x89, &tmp);
					_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xca, &ucTmp);
					tmp &= 0x80;
					_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xca, (U8)((ucTmp & 0xf7) | (tmp >> 4) | 0x02));

					_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &ucTmp);
					if(ucTmp == 0xfe)
					{
						mt_fe_print(("\tBS -- PLS before checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay 50 ms!\n", ucTmp, tmp));

						_mt_sleep_ct8k(50);

						continue;
					}

					if(!p_scanned_tp_info[index].bCheckCCM)
					{
						MT_BOOL bMIS = FALSE;

						bMIS = _mt_fe_dmd_check_MIS_ct8k_ss2(handle);

						if(bMIS)
						{
							int iTmpCnt = 0;
							//_mt_sleep_ct8k(10);		// Delay for MIS ready

							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xFA, 0x00);
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xF0, 0x00);
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xF0, 0x03);
							//_mt_sleep_ct8k(500);

							do
							{
								_mt_sleep_ct8k(50);
								_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xbe, &ucTmp);

								iTmpCnt ++;
							} while ((ucTmp != 0xfe) && (iTmpCnt < 20));		// 50ms * 20 = 1000ms

							mt_fe_print(("\tBS -- PLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms!\n", ucTmp, tmp, 50 * iTmpCnt));

							if(ucTmp == 0xfe)
							{
								continue;
							}

							_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xf1, &ucTmp);
							p_scanned_tp_info[index].iTsCnt = ucTmp & 0x1F;
							handle->m_device_ss2.tp_cfg.iTsCnt = ucTmp & 0x1F;

							if(p_scanned_tp_info[index].iTsCnt >= 1)
							{
								mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));

								for(iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt ++)
								{
									if(iCnt >= 32)
									{
										ret = MtFeErr_NoMemory;
										break;
									}

									ucTmp = (U8)iCnt;

									_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xF2, ucTmp);
									_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xF3, &ucTmp);

									p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;
									handle->m_device_ss2.tp_cfg.ucTsId[iCnt] = ucTmp;

									mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
								}

								_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xF5, p_scanned_tp_info[index].ucTsId[0]);	// default select TS 0

								p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
								handle->m_device_ss2.tp_cfg.ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
								mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", p_scanned_tp_info[index].ucTsId[0]));
							}

							p_scanned_tp_info[index].bCheckCCM = TRUE;
							handle->m_device_ss2.tp_cfg.bCheckCCM = TRUE;
						}
					}

					lockstate = MtFeLockState_Locked;

					_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xf9, 0x01);
				}
			}
			else
			{
				current_mode = MtFeType_DVBS;
				handle->m_device_ss2.tp_cfg.mCurrentType = MtFeType_DVBS;
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
				if ((tmp & 0xf7) == 0xf7)
				{
					lockstate = MtFeLockState_Locked;
				}
			}

			if(lockstate == MtFeLockState_Locked)
			{
				S32 carrier_offset;
				U8 reg_0x0c;

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0c, &reg_0x0c);
				if(((reg_0x0c & 0x04) == 0x00) && (!handle->m_device_ss2.lnb_cfg.bUnicable))
				{
					// AFC on
					_mt_fe_dmd_get_total_carrier_offset_ct8k_ss2(handle, &carrier_offset);
					if((carrier_offset > ((S32)sym_rate_KSs)) || (carrier_offset < -((S32)sym_rate_KSs)))
					{
						//mt_fe_tn_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
						if(handle->m_device_ss2.tuner_cfg.tuner_set != NULL)
						{
							mt_fe_i2c_repeat_enable_ct8k(handle);
							handle->m_device_ss2.tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
							mt_fe_i2c_repeat_disable_ct8k(handle);
						}

						real_freq_KHz = centerfreq_KHz;
						deviceoffset_KHz = 0;

						handle->m_device_ss2.tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;

						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x01);

						_mt_fe_dmd_bs_set_demod_ct8k_ss2(handle, sym_rate_KSs, current_mode);

						if((sym_rate_KSs <= 5000) && (current_mode == MtFeType_DVBS2))
						{
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xc0, 0x04);
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8a, 0x09);
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8b, 0x22);
							_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x8c, 0x88);
						}

						_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0c, &reg_0x0c);
						reg_0x0c |= 0x04;
						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x0c, reg_0x0c);


						if (handle->m_device_ss2.tuner_cfg.tuner_get_offset != NULL)
						{
							mt_fe_i2c_repeat_enable_ct8k(handle);
							handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &tuner_offset_KHZ);
							mt_fe_i2c_repeat_disable_ct8k(handle);
						}

						_mt_fe_dmd_set_carrier_offset_ct8k_ss2(handle, tuner_offset_KHZ + lpf_offset_KHz + deviceoffset_KHz);

						_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xb2, 0x00);

						count = 40;
						lockstate = MtFeLockState_Waiting;
						mt_fe_print(("\tCarrier offset out of range! Frequency = %d, CarrierOffset = %d, SymbolRate = %d\n", centerfreq_KHz, carrier_offset, sym_rate_KSs));
					}
					else
					{
						// normal carrier offset
						break;
					}
				}
				else
				{
					// AFC off
					break;
				}
			}
			else
			{
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xb3, &tmp);
				if((tmp & 0x80) == 0x80)
				{
					timing_fail ++;
					if(timing_fail >= timing_thr)
					{
						timing_fail = 1;
						break;
					}
					mt_fe_dmd_soft_reset_ct8k_ss2(handle);
					count = 30;
				}
			}

			_mt_sleep_ct8k(50);
			count--;
		}

		if(count == 0)
		{
			mt_fe_bs_notify_ct8k(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			continue;
		}


		if(timing_fail == 1)
		{
			mt_fe_bs_notify_ct8k(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			mt_fe_print(("\tTiming fail!!!\n"));
			continue;
		}
#if 0
		else if(crl_fail == 1)
		{
			mt_fe_bs_notify_ct8k(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			continue;
		}
#endif

		if(lockstate == MtFeLockState_Locked)
		{
			MT_FE_CHAN_INFO_DVBS2 ch_info;

			_mt_fe_dmd_get_carrier_offset_ct8k_ss2(handle, &carrieroffset_KHz);

			handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &tuner_offset_KHZ);
			truefreq_KHz = real_freq_KHz + tuner_offset_KHZ - carrieroffset_KHz;

			_mt_fe_dmd_get_sym_rate_ct8k_ss2(handle, &sym_rate_KSs);

			same_locked_TP_flag = 0;
			for(i = (S16)(handle->m_device_ss2.global_cfg.iScannedTpCnt - 1); i >= 0; i --)
			{
				if(handle->m_device_ss2.global_cfg.bCancelBs) break;

				if(p_bs_info->p_tp_info[i].freq_KHz > truefreq_KHz)
					delta_freq_KHz = p_bs_info->p_tp_info[i].freq_KHz - truefreq_KHz;
				else
					delta_freq_KHz = truefreq_KHz - p_bs_info->p_tp_info[i].freq_KHz;

				if(p_bs_info->p_tp_info[i].sym_rate_KSs > sym_rate_KSs)
					delta_smy_rate_KSs =  p_bs_info->p_tp_info[i].sym_rate_KSs - sym_rate_KSs;
				else
					delta_smy_rate_KSs =  sym_rate_KSs - p_bs_info->p_tp_info[i].sym_rate_KSs;


				if(((delta_smy_rate_KSs < 50) && (delta_freq_KHz < (p_bs_info->p_tp_info[i].sym_rate_KSs / 2))) && 
				   (p_bs_info->p_tp_info[i].dvb_type == current_mode))
				{
					same_locked_TP_flag = 1;
					p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;

					break;
				}
			}

			if(same_locked_TP_flag == 1)
				continue;


			mt_fe_dmd_get_channel_info_ct8k_ss2(handle, &ch_info);

			if((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9e, 0x9f);
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9f, 0x0c);

				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9d, &tmp);
				tmp &= 0xFE;
				_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9d, tmp);
			}

			handle->m_device_ss2.global_cfg.iLockedTpCnt ++;
			p_bs_info->p_tp_info[index].freq_KHz 	 = truefreq_KHz;
			p_bs_info->p_tp_info[index].sym_rate_KSs = (U16)sym_rate_KSs;
			p_bs_info->p_tp_info[index].dvb_type 	 = current_mode;
			_mt_fe_dmd_get_fec_ct8k_ss2(handle, &(p_bs_info->p_tp_info[index]), current_mode);

			p_bs_info->tp_num = handle->m_device_ss2.global_cfg.iLockedTpCnt;

			mt_fe_bs_notify_ct8k(MtFeMsg_BSTpLocked, &(p_bs_info->p_tp_info[index]));
		}
		else
		{
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			mt_fe_bs_notify_ct8k(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
		}
	}

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_unicable_set_tuner_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32* real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz)
{
	U32 tmp_freq_MHz = 0;
	MT_FE_DiSEqC_MSG msg;
	U16 T_chan = 0;

	if(handle->m_device_ss2.lnb_cfg.iUBVer == 2)	// 2.0
	{
		tmp_freq_MHz = freq_KHz / 1000;

		if(tmp_freq_MHz < 100)
			tmp_freq_MHz = 100;

		T_chan = tmp_freq_MHz - 100;

		if(T_chan > 2047)
			T_chan = 2047;

		ub_select %= 32;
		bank_select &= 0x0F;

		msg.data_send[0] = 0x70;
		msg.data_send[1] = ((ub_select & 0x1F) << 3) | ((T_chan >> 8) & 0x07);
		msg.data_send[2] = T_chan & 0xFF;
		msg.data_send[3] = (bank_select << 4) | bank_select;
		//msg.data_send[3] = (bank_select << 4);
		//msg.data_send[3] = bank_select;

		msg.size_send = 4;
		msg.is_enable_receive = FALSE;
		msg.is_envelop_mode = FALSE;


		if(ub_freq_MHz == 0)
			ub_freq_MHz = handle->m_device_ss2.lnb_cfg.iUBList[ub_select];

		tmp_freq_MHz = T_chan + 100;

		*real_freq_KHz = tmp_freq_MHz * 1000;
	}
	else					// 1.0
	{
		ub_select &= 0x07;
		bank_select &= 0x07;

		tmp_freq_MHz = freq_KHz / 1000;

		T_chan = (U16)((tmp_freq_MHz + ub_freq_MHz) / 4 - 350);

		msg.data_send[0] = 0xe0;
		msg.data_send[1] = 0x00;
		msg.data_send[2] = 0x5a;

		msg.data_send[3] = (U8)((ub_select << 5) & 0xE0);
		msg.data_send[3] = (U8)(msg.data_send[3] + ((bank_select << 2) & 0x1C));
		msg.data_send[3] = (U8)(msg.data_send[3] + ((T_chan >> 8) & 0x03));

		msg.data_send[4] = (U8)(T_chan & 0xff);

		msg.size_send = 5;
		msg.is_enable_receive = FALSE;
		msg.is_envelop_mode = FALSE;


		tmp_freq_MHz += ub_freq_MHz;
		tmp_freq_MHz /= 4;
		tmp_freq_MHz *= 4;
		tmp_freq_MHz -= ub_freq_MHz;

		*real_freq_KHz = tmp_freq_MHz * 1000;
	}


	mt_fe_dmd_set_LNB_ct8k_ss2(handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
	_mt_sleep_ct8k(15);	// fix 110884

	mt_fe_dmd_DiSEqC_send_receive_msg_ct8k_ss2(handle, &msg);

	_mt_sleep_ct8k(15);	// fix 110884
	mt_fe_dmd_set_LNB_ct8k_ss2(handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);

	//mt_fe_tn_set_tuner(handle, ub_freq_MHz * 1000, symbol_rate_KSs, 0);
	if(handle->m_device_ss2.tuner_cfg.tuner_set != NULL)
	{
		mt_fe_i2c_repeat_enable_ct8k(handle);
		handle->m_device_ss2.tuner_cfg.tuner_set(handle, ub_freq_MHz * 1000, symbol_rate_KSs, 0);
		mt_fe_i2c_repeat_disable_ct8k(handle);
	}

	return MtFeErr_Ok;
}


/***************          Blind scan test        ******************************/

MT_FE_RET _mt_fe_dmd_bs_set_reg_psd_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	U8 tmp, sm_buf;

	U8 ucTmp1, ucTmp2, ucTmp3, ucTmp4;

	ucTmp1 = (handle->m_device_ss2.bs_cfg.ave_times / 32 - 1) * 16 + (handle->m_device_ss2.bs_cfg.data_length / 128 - 1);	// = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
	ucTmp2 = 64 + handle->m_device_ss2.bs_cfg.psd_overlap / 16;												// = (64 + 96/16) = (64 + 6) = 0x46
	ucTmp3 = handle->m_device_ss2.bs_cfg.psd_scale * 32;														// = (0*32) = 0
	ucTmp4 = handle->m_device_ss2.bs_cfg.notch_range_f * 16 + handle->m_device_ss2.bs_cfg.start_range_f;					// = (8*16 + 15) = 0x8F

	sm_buf = 2;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xB2, 0x01);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x01);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4A, 0x00);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4D, &tmp);
	tmp |= 0x91;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4D, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x90, &tmp);
	tmp |= ucTmp1;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x90, tmp);

	if(sm_buf == handle->m_device_ss2.bs_cfg.sm_buf_1st)
		tmp = 128 + ucTmp2;
	else
		tmp = 0x42;//ucTmp2;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x91, tmp);

	tmp = (U8)(ucTmp3 + sm_buf - 1);
	tmp &= 0x7f;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x92, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x93, &tmp);
	tmp &= 0x0F;
	tmp |= ucTmp4;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x93, tmp);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x94, &tmp);
	tmp |= (handle->m_device_ss2.bs_cfg.find_notch_step * 16 + handle->m_device_ss2.bs_cfg.thr_factor_2nd);		// THR_FACTOR_ONE_LOOP
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x94, tmp);

	tmp = 64 + handle->m_device_ss2.bs_cfg.err_bound_factor_2nd;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x95, tmp);

	tmp = handle->m_device_ss2.bs_cfg.tuner_slip_step * 16 + handle->m_device_ss2.bs_cfg.snr_thr_2nd / 16 - 1;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x97, tmp);

	tmp = handle->m_device_ss2.bs_cfg.flat_thr_2nd;
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x99, tmp);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x30, (U8)(handle->m_device_ss2.board_cfg.bAGCPolar ? 0x18 : 0x08));

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x32, 0x44);

	if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS2022)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x99);
	else if(handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_RDA5815M)
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0xbd);
		//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x5a);
	else
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x33, 0x35);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x35, 0x7F);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4B, 0x04);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x56, 0x01);
	//_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xA0, 0x44);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x08, 0x83);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x70, 0x00);

	if(handle->m_device_ss2.board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &tmp);
		tmp |= 0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, tmp);
	}
	else
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x4d, &tmp);
		tmp &= ~0x02;
		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x4d, tmp);
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x00, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xB2, 0x00);

	return MtFeErr_Ok;
}


#if 0
MT_FE_RET _mt_fe_dmd_bs_process_scanned_TP_psd_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 cur_scan_freq_KHz)
{
	MT_FE_BOOL	b_analog_locked, b_fftdone;
	U8	tmp, cnt;
	U16	totaltpnum;
	FILE	*fpOut;
	U32	gain_val = 0;
	S8	iStrength = 0;

	//U32		nSize = (512 - 16 * 2 + 2 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	U32		nSize = (512 - 16 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	//U32		nSize = (512 - 16 -1) * 2;
	U8		buffer[1024] = {0};						// sm = 2;
	U32		dwPSDArray[512];						// 484 / 2
	U32		nCount, i, j;
	U32		dwTmp;

	char	file_name[100];

	sprintf(file_name, "PSD_%s_%s_%d.txt", handle->m_device_ss2.lnb_cfg.b22K ? "H" : "L", (handle->m_device_ss2.lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->m_device_ss2.tp_cfg.iFreqKHz / 1000);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x90, 0x70);

	for(i = 0; i < 2; i ++)
	{
		b_fftdone		= MtFe_False;
		b_analog_locked = MtFe_False;

		cnt = 10;
		do
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
			b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
			_mt_sleep_ct8k(50);
			cnt --;
		}while((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs));

		if(cnt == 0)
			break;

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9a, 0x80);

		cnt = 50;
		do
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9a, &tmp);
			b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
			_mt_sleep_ct8k(10);
			cnt --;
		}while((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs));

		if(b_fftdone || (1 == i))
		{
			break;
		}
		else
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5f, 0x00);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5e, 0x70);
		}
	}

	mt_fe_print(("\tAnalog_Locked = %d, FFT_Done = %d\n", b_analog_locked, b_fftdone));

	fpOut = fopen(file_name, "w+");

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x5A, &tmp);
	fprintf(fpOut, "RF Gain: %d\n", tmp & 0x0f);

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x5F, &tmp);
	fprintf(fpOut, "IF Gain: %d\n", tmp & 0x0f);

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x77, &tmp);
	fprintf(fpOut, "BB Gain: %d\n", (tmp >> 4) & 0x0f);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x3F, &tmp);
	fprintf(fpOut, "PWM: %d\n", tmp);

	//	handle->m_device_ss2.tuner_cfg.(handle, &gain_val);
	gain_val = 0;


	handle->m_device_ss2.tuner_cfg.tuner_get_gain(handle, &gain_val);
	handle->m_device_ss2.tuner_cfg.tuner_strength(handle, &iStrength);
	fprintf(fpOut, "Tuner Gain: %d\n", gain_val);

	fprintf(fpOut, "Strength: %d\n", iStrength);

	fclose(fpOut);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9a, &tmp);
	totaltpnum = (U16)(tmp & 0x1F);

	nCount = 1;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9A, 0x40);

	for(j = 0; j < nSize; j ++)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp);
		buffer[j] = tmp;
	}

	//for(i = 2 * 4; i < nSize; i += 2)					// sm_buf = 4
	for(i = 2 * 2; i < nSize; i += 2)					// sm_buf = 2
	{
		dwTmp = buffer[i];
		dwTmp = dwTmp + (buffer[i + 1] * 256);
		dwPSDArray[nCount ++] = dwTmp;
	}


	fpOut = fopen(file_name, "a");

	for(i = 1; i < nCount; i ++)
	{
		fprintf(fpOut, "%d\n", dwPSDArray[i]);
	}
	fclose(fpOut);


	_mt_sleep_ct8k(50);

	return MtFeErr_Ok;
}
#endif

#if 0
MT_FE_RET mt_fe_dmd_get_psd_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz)
{
	U32 begin_freq_KHz, end_freq_KHz;
	U32 real_freq_KHz;
	U32 freq_KHz;
	S32 iTunerOffsetKHz = 0;

	mt_fe_dmd_global_reset_ct8k_ss2(handle);

	mt_fe_bs_notify_ct8k(MtFeMsg_BSStart, 0);

	handle->m_device_ss2.global_cfg.iScannedTpCnt	 = 0;
	handle->m_device_ss2.global_cfg.iLockedTpCnt	 = 0;
	handle->m_device_ss2.global_cfg.bCancelBs		 = FALSE;
	handle->m_device_ss2.global_cfg.iTotalBsTimes	 = 1;//p_bs_info->bs_times;

	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ handle->m_device_ss2.lnb_cfg.bUnicable;		// XOR
	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted;
	handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ (handle->m_device_ss2.lnb_cfg.bUnicable && (handle->m_device_ss2.lnb_cfg.iUBVer != 2));		// XOR

	begin_freq_KHz = begin_freq_MHz * 1000;
	end_freq_KHz   = end_freq_MHz * 1000;

	if((handle->m_device_ss2.tuner_cfg.tuner_set == NULL) || (handle->m_device_ss2.tuner_cfg.tuner_get_offset == NULL))
		return MtFeErr_NullPointer;

	freq_KHz  = begin_freq_KHz;

	while((freq_KHz < end_freq_KHz) && (!handle->m_device_ss2.global_cfg.bCancelBs))
	{
		_mt_fe_dmd_bs_set_reg_psd_ct8k_ss2(handle);


		handle->m_device_ss2.tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
		handle->m_device_ss2.tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

		mt_fe_dmd_config_clock_ct8k(handle, MtFeType_DVBS_S2);

		if(handle->m_device_ss2.lnb_cfg.bUnicable)
		{
			mt_fe_unicable_set_tuner_ct8k_ss2(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->m_device_ss2.lnb_cfg.iUBIndex, handle->m_device_ss2.lnb_cfg.iBankIndex, handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
			freq_KHz = real_freq_KHz;
		}
		else
		{
			handle->m_device_ss2.tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
		}

		handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
		_mt_fe_dmd_set_carrier_offset_ct8k_ss2(handle, iTunerOffsetKHz);


		mt_fe_print(("\t---------get psd----- %dMhz-----\n", freq_KHz/1000));

		_mt_fe_dmd_bs_process_scanned_TP_psd_ct8k_ss2(handle, freq_KHz);

		freq_KHz += 2000;
	}
	_mt_sleep_ct8k(1000);

	mt_fe_bs_notify_ct8k(MtFeMsg_BSFinish, p_bs_info);

	return MtFeErr_Ok;
}
#endif


//////////////////////////////////////////////
#define	FIND_DEBUG		0
#define	PSD_WRITE		0


#define mt_fe_min(a, b)				(((a) < (b)) ? (a) : (b))
#define FFT_N		512
#define SM_WIN	4//6//8	//% The smoothen window length

#define OVLP	84   //32//;//64;//96;//%256;
#define	SR_X_thr	15000


static U8  Fs = 96;
static U32 ct_freq = 0;//%randint(1,1,[0 25])*2;

static S8  tuner_offset = 0;
static U32 SymRate_last = 0, CtFreq_last = 0;
static S32 idx_dw_last = 0;
static U32 idx_notch_last_tureCh = 1;
static U32 idx_ch_max = 1;
static U32 idx_ch_min = 1;
static U32 idx_notch = 1;
static U32 ch_max = 0;
static U32 ch_min = 0;
static U32 idx_notch_last = 1;
static U32 idx_psd_start = 0;
static U32 ave_peak = 0;
//U8  wide_TP_flag = 0;
static U32 psd_offset = 11;
static S32 SNR_last = -10;
static U32 ch_noise_lev = 0;
//U32 ct_freq_actual = 0;

#if FIND_DEBUG
static void strmcp(char *des, char *sour, U8 m)
{
	U8 n = 0;

	char *descpy;
	descpy = des;

	for(n = 0; n < m; n ++)
		sour ++;

	while((*sour) != '\0')
	{
		*descpy = *sour;
		descpy ++;
		sour ++;
	}
}
#endif

#if 0
static void mt_fe_dmd_bs_global_init()
{
	Fs = 96;
	ct_freq = 0;//%randint(1,1,[0 25])*2;

	tuner_offset = 0;
	SymRate_last = 0, CtFreq_last = 0;
	idx_dw_last = 0;
	idx_notch_last_tureCh = 1;
	idx_ch_max = 1;
	idx_ch_min = 1;
	idx_notch = 1;
	ch_max = 0;
	ch_min = 0;
	idx_notch_last = 1;
	idx_psd_start = 0;
	ave_peak = 0;
	//U8	wide_TP_flag = 0;
	psd_offset = 11;
	SNR_last = -10;
	ch_noise_lev = 0;
	//ct_freq_actual = 0;
}
#endif

static void mt_fe_dmd_bs_find_Min(U32 *sour_value, U32 *min_value, U32 *min_index, U32 start_index, U32 end_index)
{
	U32 data = 0;
	U32 i = 0, index = 0;

	data = sour_value[start_index];
	index = start_index;
	for(i = start_index; i <= end_index; i ++)
	{
		if(sour_value[i] < data)
		{
			data = sour_value[i];
			index = i;
		}
	}

	*min_value = data;
	*min_index = (index - start_index + 1);
}

static void mt_fe_dmd_bs_find_Max(U32 *sour_value, U32 *max_value, U32 *max_index, U32 start_index, U32 end_index)
{
	U32 data = 0;
	U32 i = 0, index = 0;

	data = sour_value[start_index];
	index = start_index;

	for(i = start_index; i <= end_index; i ++)
	{
		if(sour_value[i] > data)
		{
			data = sour_value[i];
			index = i;
		}
	}

	*max_value = data;

	*max_index = (index - start_index + 1);
}

#if 0
static void mt_fe_dmd_bs_find_Mean(U32 *sour_value, U32 *mean_value, U32 find_value, U32 start_index, U32 end_index)
{
	U32 add_value = 0;
	U32 i = 0, index = 0;

	for(i = start_index; i <= end_index; i ++)
	{
		if(sour_value[i] >= find_value)
		{
			add_value = add_value + sour_value[i];
			index ++;
		}
	}

	if(index != 0)
		*mean_value = add_value / index;
	else
	{
		mt_fe_print(("\tmt_fe_dmd_bs_find_Mean() ---- index = 0!\n"));
	}
}
#endif

static void mt_fe_dmd_bs_ch_measure(MT_FE_CT8K_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 *psd, U32 idx_ps_sn)
{
	#define	MAX_sub_TP_N	6////4

	U32 SR_max = 58000;
	MT_FE_TP_INFO tp_info;
	U32 K = 125; //double K=-0.0625*2;//0.0625*0;//%%%%Ufix4_En5
	U8  MERGE_THR = 125;//1.25;//% larger the value, more likely to merge the adjacent TP
	U8  BW2SR_ratio = 135;//1.35;
	//double Thr_sig = (double)3/16;
	S32 SNR_filt_thr = 3512;//2585(old psd);//3512;//3512==4,2585=2 ,2995=3
	S32	cap_value0 = 0, cap_value1 = 0;
	U32 sub_TP_spilt_mode = 0;
	U32 idx_sub_TP = 0;
	U32 idx_notch_split[20] = {0, 0, 0, 0};//% this means there max intermediate notches number allowed is 4 (5 split TPs)
	U32	ii = 0, i = 0;
	U32 idx = 0;
	U32 sub_TP_N = 0;
	U32 notch_all[20] = {0, 0, 0, 0, 0, 0};

	U32 sig_thr = 0, noise_level = 0;
	U32 sig_level = 0;
	U32 SymRate = 0, CtFreq = 0;
	U32 SymRate_raw = 0, CtFreq_raw = 0;
	U32 SymRate_fix = 0, CtFreq_fix = 0;
	U32 ch_thr_L = 0, ch_thr_R = 0;
	U32 slpoe_L = 0, slpoe_R = 0;
	U32 idx_up = 1, idx_dw = 1, idx_dw_tmp = 0;
	U32 sig_idx = 0;
	U32 idx_upd = 0, idx_dwd = 0;
	U32 idx_start_fix = 0, idx_end_fix = 0;
	U8  K_high = 0;// -0.1;
	U8  K_low = 0;//0.1;
	U32 idx_tmp = 0, idx_L_high = 0, idx_L_low = 0, idx_R_high = 0, idx_R_low = 0;
	U32 ch_thr_high, ch_thr_low;
	S32 SNR;
	//U32 win_tmp = 0;
	//double edge_max = 0, edge_min = 0;
	//U32	edge_max_index = 0, edge_min_index = 0;
	U32 idx_up_merge = 0;
	U32 idx_dw_merge = 0;
	U32 SymRate_merge = 0;
	U32 CtFreq_merge = 0;
	U32 all_under_thr_idx[256];
	U32 all_thr_idx = 0;
	U32 L1 = 0;
	U32 val = 0;
	U32 pos = 0, tmp = 0;
	U8  rep_flag = 0;
	U32 idx_notch1 = 0, idx_notch_last1 = 0;
	U8  count = 0;

	//[ch_max idx_ch_max]= max(psd(idx_notch_last:idx_notch));% find the peak within [idx_notch last:idx_notch] for symbol rate calculation
	//idx_ch_max = idx_ch_max + idx_notch_last -1;% to align the idx with psd start

	mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
	idx_ch_max = idx_ch_max + idx_notch_last - 1;

	while(1)
	{
		count ++;
		if(count > 15)
		{
			mt_fe_print(("all in  while1 !\n"));

			break;
		}

		sig_level = 0;
		sig_idx = 0;

		if(rep_flag == 0)
		{
			for(idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp ++)
			{
				sig_level = sig_level + psd[idx_tmp];
				sig_idx = sig_idx + 1;
			}

			if(sig_idx != 0)
				sig_thr = sig_level / sig_idx;
			else
			{
				sig_thr = 1;
			}

			sig_level = 0;
			sig_idx = 0;

			for(idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp ++)
			{
				if(psd[idx_tmp] >= sig_thr)
				{
					sig_level = sig_level + psd[idx_tmp];
					sig_idx = sig_idx + 1;
				}
			}

			if(sig_idx != 0)
				sig_level = (2 * sig_level / sig_idx + 1) / 2;
			else
			{
				sig_level = 1;

				//sig_level =psd[idx_notch_last-1];
				mt_fe_print(("sig_level error !\n"));
			}
		}
		else
		{
			for(idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp ++)
			{
				sig_level = sig_level + psd[idx_tmp];
				sig_idx = sig_idx + 1;
			}

			if(sig_idx != 0)
				sig_thr = sig_level / sig_idx;
			else
			{
				sig_thr = 1;
			}

			sig_level = 0;
			sig_idx = 0;

			for(idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp ++)
			{
				if(psd[idx_tmp] >= sig_thr)
				{
					sig_level = sig_level + psd[idx_tmp];
					sig_idx = sig_idx + 1;
				}
			}

			if(sig_idx != 0)
				sig_level = (2 * sig_level / sig_idx + 1) / 2;
			else
			{
				sig_level = 1;
				//sig_level = psd[idx_notch_last - 1];
				mt_fe_print(("sig_level error !\n"));
			}
		}

		if (psd[idx_notch_last] <= psd[idx_notch])
			noise_level = psd[idx_notch_last];
		else
			noise_level = psd[idx_notch];

		ch_thr_L = (sig_level * (500 + K) + noise_level * (500 - K) + 500) / 1000;;
		ch_thr_R = (sig_level * (500 + K) + noise_level * (500 - K) + 500) / 1000;

		//% limit the right side thr to right side min level
		if (ch_thr_L < psd[idx_notch_last])
			ch_thr_L = psd[idx_notch_last];

		if (ch_thr_R < psd[idx_notch])
			ch_thr_R = psd[idx_notch] + 1;

		//%locate the falling edge thr crossing point
		for(idx_tmp = idx_ch_max; idx_tmp <= (idx_notch - 1); idx_tmp ++)
		{
			if ((psd[idx_tmp + 1] <= ch_thr_R) && (psd[idx_tmp] > ch_thr_R))
			{
				idx_dw = idx_tmp;
				if(sub_TP_spilt_mode)
					break;
			}
		}

		//% locate the rising edge thr crossing  point
		for(idx_tmp = idx_ch_max; idx_tmp >= idx_notch_last; idx_tmp --)
		{
			if ((psd[idx_tmp + 1] > ch_thr_L) && (psd[idx_tmp] <= ch_thr_L))
			{
				idx_up = idx_tmp;
				if(sub_TP_spilt_mode)
					break;
			}
		}

		//%-----------------refine the symbol rate-----------------------------------
		if(idx_dw != 1)
		{
			//idx_dw = idx_dw + mt_fe_min(1,(psd[idx_dw]-ch_thr_R)/(psd[idx_dw] - psd[idx_dw+1]));
			if((psd[idx_dw] - ch_thr_R) >= ((psd[idx_dw] - psd[idx_dw + 1])))
			{
				idx_dw = idx_dw + 1;
				idx_dwd = idx_dw * 100;
			}
			else
			{
				if(psd[idx_dw] != psd[idx_dw + 1])
					idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / (psd[idx_dw] - psd[idx_dw + 1]) + idx_dw * 200 + 1) / 2;
				else
					idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / 1 + idx_dw * 200 + 1) / 2;
			}
		}

		if(idx_up != 1)
		{
			//idx_up = idx_up - mt_fe_min(1,(ch_thr_L-psd[idx_up])/(psd[idx_up] - psd[idx_up+1]));
			if((ch_thr_L - psd[idx_up]) >= ((psd[idx_up + 1] - psd[idx_up])))
			{
				//idx_up = idx_up;// -1;
				idx_upd = idx_up * 100;// -1;
			}
			else
			{
				if(psd[idx_up + 1] != psd[idx_up])
					idx_upd = (200 * (ch_thr_L - psd[idx_up]) / (psd[idx_up + 1] - psd[idx_up]) + idx_up * 200 + 1) / 2;
				else
					idx_upd = (200 * (ch_thr_L - psd[idx_up]) / 1 + idx_up * 200 + 1) / 2;
			}
		}

		tmp = 0;
		if((idx_dw > idx_up) && (idx_notch > idx_notch_last))
		{
			if((2 * (idx_dw - idx_up)) < (idx_notch - idx_notch_last))
				tmp = 1;
		}
		else if((idx_dw < idx_up) && (idx_notch < idx_notch_last))
		{
			if((2 * (idx_up - idx_dw)) < ( idx_notch_last - idx_notch))
				tmp = 1;
		}

		if((tmp != 0) && (rep_flag == 0) && (sub_TP_spilt_mode == 0))
		// if (idx_dw - idx_up) / (idx_notch - idx_notch_last) < 1 / 2 && rep_flag == 0
		{
			if((5 * idx_up) > idx_dw)
			{
				tmp = (5 * idx_up - idx_dw + 2) / 4;
				if (idx_notch_last < tmp)
				{
					mt_fe_dmd_bs_find_Min(psd, &val, &pos, tmp, idx_up);
					//[val,pos] = min(psd(tmp:idx_up));
					idx_notch_last1 = pos + tmp - 1;
				}
				else
					idx_notch_last1 = idx_notch_last;
			}
			else
				idx_notch_last1 = idx_notch_last;

			if((5 * idx_dw) > idx_up)
			{
				tmp = (5 * idx_dw - idx_up + 2) / 4;
				if (idx_notch > tmp)
				{
					mt_fe_dmd_bs_find_Min(psd, &val, &pos, idx_dw, tmp);
					//[val,pos] = min(psd(round(idx_dw):tmp));
					idx_notch1 = pos + idx_dw - 1;
				}
				else
					idx_notch1 = idx_notch;
			}
			else
				idx_notch1 = idx_notch;

			rep_flag = 1;

			continue;
		}

		//%%%%% locate the up cross threshold point
		if ((idx_up != 1) || (idx_dw != 1))
		{
			//% if there is  down or up cross point, compute the symbol rate and center freq
			if(idx_dwd > idx_upd)
				SymRate_raw = (idx_dwd - idx_upd) * Fs * 10 / FFT_N;
			else
				SymRate_raw = (idx_upd - idx_dwd) * Fs * 10 / FFT_N;

			if(SymRate_raw > 60000)
				mt_fe_print(("Error symbol rate %d\n", SymRate_raw));

			//CtFreq = (idx_dw / 2 + idx_up / 2 - (FFT_N - 2 * OVLP) / 2 + tuner_offset) * Fs / FFT_N + ct_freq;
			if ((idx_dwd + idx_upd + 200 * OVLP + 200 * tuner_offset) >= (100 * FFT_N))
				CtFreq_raw = ((10 * idx_dwd + 10 * idx_upd + 2000 * OVLP + 2000 * tuner_offset - 1000 * FFT_N) * Fs / (2 * FFT_N) + ct_freq);
			else
				CtFreq_raw = (ct_freq - (1000 * FFT_N - 10 * idx_dwd - 10 * idx_upd - 2000 * OVLP - 2000 * tuner_offset) * Fs / (2 * FFT_N));

			//SNR = 10 * log10(sig_level^2 / noise_level^2 - 1);

			#if 0
			//SNR = 10 * log10(((double)sig_level * (double)sig_level) / (noise_level * noise_level) - 1);
			if(((double)sig_level * (double)sig_level) > 3.512 * (ch_noise_lev * ch_noise_lev))//SNR_filt_thr = 4 so 2.512
			//if(((double)sig_level * (double)sig_level)> 1.5*(noise_level * noise_level))
				SNR = 1.0;
			else
				SNR = 0.0;
			#endif

			if(ch_noise_lev != 0)
				SNR = (sig_level * sig_level) / ch_noise_lev * 1000 / ch_noise_lev;
			else
				SNR = (sig_level * sig_level) / 1 * 1000 / 1;

			#if 0
			win_tmp = ((idx_dwd - idx_upd) * 0.1 + 0.5);
			idx_up = (U32)(idx_upd + 0.5);
			idx_dw = (U32)(idx_dwd + 0.5);

			if((idx_up - win_tmp) > idx_notch_last)
			{
				mt_fe_dmd_bs_find_Max(psd, &edge_max, &edge_max_index, (idx_up - win_tmp), (idx_up + win_tmp));
				mt_fe_dmd_bs_find_Min(psd, &edge_min, &edge_min_index, (idx_up - win_tmp), (idx_up + win_tmp));
				delta_tmp_L = (edge_max - edge_min) / (2 * win_tmp);
			}
			else
			{
				mt_fe_dmd_bs_find_Max(psd, &edge_max, &edge_max_index, (idx_notch_last), (idx_up + win_tmp));
				mt_fe_dmd_bs_find_Min(psd, &edge_min, &edge_min_index, (idx_notch_last), (idx_up + win_tmp));
				delta_tmp_L = (edge_max - edge_min) / (idx_up + win_tmp - idx_notch_last);
			}

			if((idx_dw + win_tmp) < idx_notch)
			{
				mt_fe_dmd_bs_find_Max(psd, &edge_max, &edge_max_index, (idx_dw - win_tmp), (idx_dw + win_tmp));
				mt_fe_dmd_bs_find_Min(psd, &edge_min, &edge_min_index, (idx_dw - win_tmp), (idx_dw + win_tmp));
				delta_tmp_R = (edge_max - edge_min) / (2 * win_tmp);
			}
			else
			{
				mt_fe_dmd_bs_find_Max(psd, &edge_max, &edge_max_index, (idx_dw - win_tmp), idx_notch);
				mt_fe_dmd_bs_find_Min(psd, &edge_min, &edge_min_index, (idx_dw - win_tmp), idx_notch);
				delta_tmp_R = (edge_max - edge_min) / (idx_notch + win_tmp - idx_dw);
			}
			#endif

			//if ((SymRate > SR_min) && (SymRate < SR_max) && (SNR > SNR_filt_thr))
			if ((SymRate_raw > 1200) && (SymRate_raw < SR_max))// -2.0))
			{
				if(SNR > SNR_filt_thr)
				{
					idx_notch_last_tureCh = idx_notch;
				}

				if (SymRate_raw > SR_X_thr)
				{
					ch_thr_high = (sig_level * (5 + K_high) + ch_thr_L * (5 - K_high) + 5) / 10;
					mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_notch_last, idx_ch_max);
					ch_thr_low = (ch_thr_L * (5 - K_low) + val * (5 + K_low) + 5) / 10;

					for(idx_tmp = idx_ch_max; idx_tmp >= idx_up; idx_tmp --)
					{
						idx_L_high = idx_tmp;
						if ((psd[idx_tmp] <= ch_thr_high) && (psd[idx_tmp + 1] > ch_thr_high))
							break;
					}

					for (idx_tmp = idx_up; idx_tmp >= idx_notch_last; idx_tmp --)
					{
						idx_L_low = idx_tmp;
						if ((psd[idx_tmp] <= ch_thr_low) && (psd[idx_tmp + 1] > ch_thr_low))
							break;
					}

					if(idx_L_high <= idx_L_low)
						slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) + 1) / 2;
					else
						slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) / (idx_L_high - idx_L_low) + 1) / 2;

					//  idx_start_fix = idx_up - round((ch_thr_L - noise_level) / slpoe_L);
					if(slpoe_L == 0)
						slpoe_L = 1;

					if(idx_upd > ((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L))
						idx_start_fix = idx_upd - (((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L) + 50) / 100 * 100;///100;
					else
						idx_start_fix = 0;

					ch_thr_high = (sig_level * (5 + K_high) + ch_thr_R * (5 - K_high) + 5) / 10;
					mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_ch_max, idx_notch);

					//ch_thr_low = ch_thr_L * (0.5 - K_low) + min(psd(idx_ch_max : idx_notch)) * (0.5 + K_low);
					ch_thr_low = (ch_thr_R * (5 - K_low) + val * (5 + K_low) + 5) / 10;

					for (idx_tmp = idx_ch_max; idx_tmp <= idx_dw; idx_tmp ++)
					{
						idx_R_high = idx_tmp;
						if ((psd[idx_tmp + 1] <= ch_thr_high) && (psd[idx_tmp] > ch_thr_high))
							break;
					}

					for (idx_tmp = idx_dw; idx_tmp <= idx_notch; idx_tmp ++)
					{
						idx_R_low = idx_tmp;
						if ((psd[idx_tmp + 1] <= ch_thr_low) && (psd[idx_tmp] > ch_thr_low))
							break;
					}

					if(idx_R_low <= idx_R_high)
						slpoe_R = (2 * (10 * ch_thr_high - 10 * ch_thr_low) + 1) / 2;
					else
						slpoe_R = (2 * (10 * ch_thr_high - 10 * ch_thr_low) / (idx_R_low - idx_R_high) + 1) / 2;

					if(slpoe_R == 0)
						slpoe_R = 1;

					idx_end_fix = ((1000 * (ch_thr_R - noise_level) / slpoe_R) + 50) / 100 * 100 + idx_dwd;///100;

					//idx_start_fix = max(idx_start_fix,idx_notch_last);
					//idx_end_fix = min(idx_end_fix,idx_notch);
					if(idx_start_fix < (100 * idx_notch_last))
						idx_start_fix =	100 * idx_notch_last;

					if(idx_end_fix > (100 * idx_notch))
						idx_end_fix = 100 * idx_notch;

					if(idx_end_fix > idx_start_fix)
						SymRate_fix = 100 * (idx_end_fix - idx_start_fix) * Fs / FFT_N * 10 / BW2SR_ratio;
					else
						SymRate_fix = 0;

					if ((idx_start_fix + idx_end_fix + 200 * OVLP + 200 * tuner_offset) >= (100 * FFT_N))
						CtFreq_fix = ((10 * idx_start_fix + 10 * idx_end_fix + 2000 * OVLP + 2000 * tuner_offset - 1000 * FFT_N) * Fs / (2 * FFT_N) + ct_freq);
					else
						CtFreq_fix = (ct_freq - (1000 * FFT_N - 10 * idx_start_fix - 10 * idx_end_fix - 2000 * OVLP - 2000 * tuner_offset) * Fs / (2 * FFT_N));

					// CtFreq_fix = (idx_start_fix / 2 + idx_end_fix / 2 - (FFT_N - 2 * OVLP) / 2 + tuner_offset) / FFT_N * (double)Fs + ct_freq;
				}
				else
				{
					SymRate_fix = 0;
					CtFreq_fix = 0;
				}

				if ((SymRate_raw > SR_X_thr) && (SymRate_fix > SymRate_raw))
				{
					SymRate = SymRate_fix;
					CtFreq = CtFreq_fix;

					if(sub_TP_spilt_mode)
						;
					else
					{
						if(SymRate_fix > SymRate_raw)
							idx_dw_tmp = idx_dw + (((SymRate_fix - SymRate_raw) * FFT_N) + Fs) / (2 * Fs * 1000);
						else
							idx_dw_tmp = idx_dw - (((SymRate_raw - SymRate_fix) * FFT_N) + Fs) / (2 * Fs * 1000);

						if((CtFreq + SymRate / 2) > ct_freq)
							idx_dw_tmp = 100 * FFT_N / 2 - 100 * OVLP + 100 * tuner_offset + (((CtFreq + SymRate / 2 - ct_freq) * FFT_N) + 500 * Fs) / (10 * Fs);
						else
							idx_dw_tmp = 100 * FFT_N / 2 - 100 * OVLP + 100 * tuner_offset - (((ct_freq - CtFreq - SymRate / 2) * FFT_N) + 500 * Fs) / (10 * Fs);

						if(idx_dw_tmp > (100 * idx_notch))
							idx_dw_tmp = idx_notch * 100;
					}
					//if(SymRate_fix > SymRate_raw)
					//	idx_dw_tmp = idx_dw + (((SymRate_fix - SymRate_raw) * FFT_N) + Fs) / (2 * Fs * 1000);
					//else
					//	idx_dw_tmp = idx_dw - (((SymRate_raw - SymRate_fix ) * FFT_N) + Fs) / (2 * Fs * 1000);
					//if(sub_TP_spilt_mode == 0)
					//idx_dw_tmp = min(idx_notch, idx_dw + round((SymRate_fix - SymRate_raw) / 2 / Fs * FFT_N));
					// idx_dw_tmp = min(idx_notch, round((CtFreq + SymRate / 2  - ct_freq) / Fs * FFT_N) + FFT_N / 2 - OVLP + tuner_offset);
				}
				else
				{
					SymRate = SymRate_raw;
					CtFreq = CtFreq_raw;
					if(sub_TP_spilt_mode == 0)
						idx_dw_tmp = idx_dw * 100;
				}

				if (SNR > SNR_filt_thr)
				{
					if(handle->m_device_ss2.global_cfg.iScannedTpCnt > 100)
					{
						mt_fe_print(("global_cfg.iScannedTpCnt > 100!\n"));

						_mt_sleep_ct8k(20000);
					}

					#if 1
					//handle->m_device_ss2.global_cfg.iScannedTpCnt ++;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].freq_KHz = CtFreq;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;

					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_up = idx_up;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
					p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
					handle->m_device_ss2.global_cfg.iScannedTpCnt ++;
					#endif

					tp_info.freq_KHz     = CtFreq;
					tp_info.sym_rate_KSs = SymRate;
					tp_info.uidx_notch = idx_notch;
					tp_info.uidx_notch_last = idx_notch_last;
					tp_info.uidx_up = idx_up;
					tp_info.upsd_sn = idx_ps_sn;
					mt_fe_bs_notify_ct8k(MtFeMsg_BSTpFind, &tp_info);
				}

				if(sub_TP_spilt_mode == 0)
				{
					//if(idx_dw_last > 0)
					{
						//if ((S32)idx_up < idx_dw_last)
						//mt_fe_print(("merge err idx_up = %d, idx_dw_last = %d\r\n", idx_up, idx_dw_last));
					}

					if (((((S32)idx_upd - idx_dw_last) * 10 * Fs / FFT_N) < (S32)(SymRate_last + SymRate) * (MERGE_THR - 100) / 200) && (SymRate_last > 3000) && (SymRate_raw > 3000)
						&& ((SymRate_last + SymRate) < SR_max ) && ((S32)(100 * idx_up) >= idx_dw_last))
					{
						if(((SNR_last > SNR) && (((SNR - 1000) * 4) >= (SNR_last - 1000)) && (SNR_last > SNR_filt_thr)) || ((SNR_last < SNR) && (((SNR_last - 1000) * 4) >= (SNR - 1000)) && (SNR > SNR_filt_thr)))
						{
							#if 1
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].freq_KHz = (CtFreq_raw+CtFreq_last) / 2;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate_last + SymRate;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_up = idx_up;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
							p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
							handle->m_device_ss2.global_cfg.iScannedTpCnt ++;
							#endif

							tp_info.freq_KHz     = (CtFreq_raw + CtFreq_last) / 2;
							tp_info.sym_rate_KSs = SymRate_last + SymRate;
							tp_info.uidx_notch_last = idx_notch_last;
							tp_info.uidx_up = idx_up;
							tp_info.uidx_notch = idx_notch;
							tp_info.upsd_sn = idx_ps_sn;
							mt_fe_bs_notify_ct8k(MtFeMsg_BSTpFind, &tp_info);
						}
					}
					else
					{
						cap_value0 = psd[idx_notch_last] - psd[idx_notch];
						cap_value1 = psd[idx_ch_max] - psd[idx_notch];
						if((1000 * cap_value0) > (707 * cap_value1))
						//if (((psd[idx_notch_last] - psd[idx_notch]) * 0.707) > (psd[idx_ch_max] - psd[idx_notch]))
						{
							if(handle->m_device_ss2.global_cfg.iScannedTpCnt > 1)
							{
								if(p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt - 2].upsd_sn == p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt - 1].upsd_sn)
								{
									if(p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt - 2].uidx_notch == p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt - 1].uidx_notch_last)
									{
										idx_up_merge = p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt - 2].uidx_up;
										idx_dw_merge = idx_dw;

										if(idx_dw_merge >= idx_up_merge)
											SymRate_merge = (idx_dw_merge - idx_up_merge) * Fs * 1000 / FFT_N;
										else
											SymRate_merge = (idx_up_merge - idx_dw_merge) * Fs * 1000 / FFT_N;

										if((idx_up_merge + idx_dw_merge + 2 * tuner_offset) >= (FFT_N - 2 * OVLP))
											CtFreq_merge = 1000 * (idx_up_merge + idx_dw_merge + 2 * tuner_offset - (FFT_N - 2 * OVLP)) * Fs / FFT_N / 2 + ct_freq;
										else
											CtFreq_merge = ct_freq - 1000 * ((FFT_N - 2 * OVLP) - idx_up_merge - idx_dw_merge - 2 * tuner_offset) * Fs / FFT_N / 2;

										#if 1
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].freq_KHz = CtFreq_merge;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate_merge;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_up = idx_up;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
										p_bs_info->p_tp_info[handle->m_device_ss2.global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
										handle->m_device_ss2.global_cfg.iScannedTpCnt ++;
										#endif

										tp_info.freq_KHz     = CtFreq_merge;
										tp_info.sym_rate_KSs = SymRate_merge;
										tp_info.uidx_notch_last = idx_notch_last;
										tp_info.uidx_up = idx_up;
										tp_info.uidx_notch = idx_notch;
										tp_info.upsd_sn = idx_ps_sn;
										mt_fe_bs_notify_ct8k(MtFeMsg_BSTpFind, &tp_info);
									}
								}
							}
						}
					}
				}

				//if(handle->m_device_ss2.global_cfg.iScannedTpCnt > 0)
				if (sub_TP_spilt_mode == 0)
				{
					CtFreq_last = CtFreq;//CtFreq_raw;
					SymRate_last = SymRate;//SymRate_raw;
					idx_dw_last = (S32)idx_dw_tmp;
					SNR_last = SNR;
				}

				//handle->m_device_ss2.global_cfg.iScannedTpCnt ++;
			}

		#if 0	////20161026
		}
		#endif

		#if 0//20161026
		else
		{
			if ((idx_dw + idx_up + 2 * OVLP + 2 * tuner_offset) > FFT_N)
				CtFreq = (idx_dw + idx_up + 2 * OVLP + 2 * tuner_offset - FFT_N) * Fs / (2 * FFT_N) + ct_freq;
			else
				CtFreq = ct_freq - (FFT_N - idx_dw - idx_up - 2 * OVLP - 2 * tuner_offset) * Fs / (2 * FFT_N);

			//mt_fe_print(("measure deal_ch,FREQ = %fM   SymRate = %fK\n", CtFreq, SymRate));
		}
		#endif

			/*%----- split one big TP to several small TPs by intermediate notches*/
			if (sub_TP_spilt_mode == 0) //% When spilt TP is under detection, once split TP was detected, this step will be skipped
			{
				//idx_up = (U32)(idx_upd / 100);
				//idx_dw = (U32)(idx_dwd / 100);
				all_thr_idx = 0;

				for(idx_tmp = idx_up + 1; idx_tmp <= idx_ch_max; idx_tmp ++)
				{
					if(psd[idx_tmp] < ch_thr_L)
					{
						all_under_thr_idx[all_thr_idx] = idx_tmp;
						all_thr_idx = all_thr_idx + 1;
						if(all_thr_idx >= 20)
							break;
					}
				}

				for(idx_tmp = idx_ch_max + 1; idx_tmp <= idx_dw; idx_tmp ++)
				{
					if(psd[idx_tmp] < ch_thr_R)
					{
						all_under_thr_idx[all_thr_idx] = idx_tmp;
						all_thr_idx = all_thr_idx + 1;
						if(all_thr_idx >= 20)
							break;
					}
				}

				// all_under_thr_idx = find(psd(floor(idx_up) + 1 : floor(idx_dw)) < ch_thr_L) + floor(idx_up);
				// if ((cnt_up > 1 || cnt_dw > 1) && (SymRate_raw > 2000 && SymRate_raw < 36000 && SNR > 0.2))//% more than one threshold crossing point
				//if (isempty(all_under_thr_idx) == 0) && (SymRate_raw > SR_min * 2 && SymRate_raw < SR_max && SNR > SNR_filt_thr)
				if ((all_thr_idx > 0) && (SymRate_raw > 2400 && SymRate_raw < SR_max && SNR > SNR_filt_thr))//% more than one threshold crossing point
				{
					// disp('split found TP');
					L1 = all_under_thr_idx[0];
					ii = 1;

					if(all_thr_idx > 20)
					{
						all_thr_idx = 20;
						mt_fe_print(("all_thr_idx > 20 !\n"));
					}

					for (idx = 0; idx < all_thr_idx; idx ++)
					{
						if (((idx + 1) < all_thr_idx) && (all_under_thr_idx[idx + 1] - all_under_thr_idx[idx] > 1))
						{
							mt_fe_dmd_bs_find_Min(psd, &val, &pos, L1, all_under_thr_idx[idx]);
							idx_notch_split[ii - 1] = L1 + pos - 1;
							L1 = all_under_thr_idx[idx + 1];
							ii = ii + 1;
						}
						else if ((idx + 1) == all_thr_idx)
						{
							mt_fe_dmd_bs_find_Min(psd, &val, &pos, L1, all_under_thr_idx[idx]);
							idx_notch_split[ii - 1] = L1 + pos - 1;
							ii = ii + 1;
						}
					}

					for(idx = 0; idx < ii - 1; idx ++)
					{
						mt_fe_dmd_bs_find_Max(psd, &val, &pos, idx_notch_split[idx], idx_notch);
						pos = pos + idx_notch_split[idx] - 1;

						if ((4 * (psd[pos] - psd[idx_notch_split[idx]])) < (psd[pos] - psd[idx_notch]))
						{
							ii = idx + 1;
							break;
						}
					}

					sub_TP_N = ii;

					if(sub_TP_N > MAX_sub_TP_N)
					{
						mt_fe_print(("sub_TP_N too large %d!!!\n", sub_TP_N));
						sub_TP_N = MAX_sub_TP_N;
						idx_sub_TP = 0;
						sub_TP_spilt_mode = 1;
						notch_all[0] = idx_notch_last;

						for(i = 1; i < MAX_sub_TP_N; i ++)
						{
							notch_all[i] = idx_notch_split[i - 1];
						}

						notch_all[sub_TP_N] = idx_notch;
					}
					else if(sub_TP_N == 1)
					{
						break;
					}
					else
					{
						idx_sub_TP = 0;
						sub_TP_spilt_mode = 1;
						notch_all[0] = idx_notch_last;

						for(i = 1; i <= ii - 1; i ++)
						{
							notch_all[i] = idx_notch_split[i - 1];
						}

						notch_all[sub_TP_N] = idx_notch;
						if(idx_notch == 0)
							idx_notch = 0;
					}
				}
				else
				{
					break;//% No suitable sub TP splited
				}
			}

			// % if split TP was detected (sub_TP_spilt_mode == 1), measured all the sub TP
			idx_sub_TP = idx_sub_TP + 1;//% measure all the children TPs contains in current parent TP in turn (from lest to right)
			if (idx_sub_TP > sub_TP_N)
			{
				break;//% when all sub TPs were done
			}
			else//% measure all children TP (from left to right)
			{
				idx_notch_last = notch_all[idx_sub_TP - 1];
				idx_notch = notch_all[idx_sub_TP];
				if(idx_notch == 0)
					idx_notch = 0;

				mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
				//[ch_min idx_ch_min]= min(psd(idx_notch_last:idx_notch));% find the min in current searched range
				mt_fe_dmd_bs_find_Min(psd, &ch_min, &idx_ch_min, idx_notch_last, idx_notch);
				idx_ch_max = idx_ch_max + idx_notch_last - 1;//% to align the idx with psd start
				idx_ch_min = idx_ch_min + idx_notch_last - 1;
				idx_up = 1;
				idx_dw = 1;
			}
		}
		else
			break;

		if (rep_flag == 1)
			rep_flag = 0;
	}
}

MT_FE_RET _mt_fe_dmd_bs_process_get_range_list_ct8k_ss2(MT_FE_CT8K_Device_Handle handle,U32 begin_freq_MHz, U32 end_freq_MHz, U32 *range_list,U8 *list_num)
{
	U8	F_step = 24; //% effective freq range sweep step
	S8	sig_thr_low_bound = 30;//55;//8;//% The lower bound of effective signal strength
	S8	MAX_sig_stre_dif = 15;//10;//25;//% the larger this value, the less range of effective freq range
	S32	sig_stre_grid = 0;
	U8	F_idx = 0;
	S8	dwTmp1 = 0;
	U8	map_idx = 0;
	S8	ave_sig_str = 0, sig_stre_thr = 0;
	U8	find_ok = 0;
	S8	F_grid_map_tmp[108];//(2150 - 950) / 12 = 100
	U8	weak_sig_thr = 65;//20;
	U8	min_sig = 0xff;
	U8	max_sig = 0x00;
	U8	cnt = 0;
	U16	i = 0, j = 0;
	U32	freq_KHz = 0, ct_freq_actual = 0, real_freq_KHz = 0;
	U8	fre_seg_idx = 0;
	U32	set_freq = 0;

#if FIND_DEBUG
	FILE	*fpOut1;
	S32		gain_val = 0;
	S8		iStrength = 0;
	char	file_name1[100];
	char	str3[100];
	char	str4[20];
#else
	//////////////add b wbj///////////////
	MT_FE_BOOL	b_analog_locked, b_fftdone;

	U8	tmp;
#endif

	set_freq = begin_freq_MHz;
	while (set_freq < end_freq_MHz)
	{
		freq_KHz = set_freq * 1000;
		handle->m_device_ss2.tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;

#if FIND_DEBUG
		sprintf(file_name1, "PSD_%s_%s_%d.txt", handle->m_device_ss2.lnb_cfg.b22K ? "H":"L",
				 (handle->m_device_ss2.lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->m_device_ss2.tp_cfg.iFreqKHz / 1000);

		fpOut1= fopen(file_name1, "r");
		fgets(str3, 100, fpOut1);
		fgets(str3, 100, fpOut1);
		fgets(str3, 100, fpOut1);
		fgets(str3, 100, fpOut1);
		fgets(str3, 100, fpOut1);
		fgets(str3, 100, fpOut1);
		strmcp(str4, str3, 9);

		dwTmp1 = atoi(str4);
#else
		_mt_fe_dmd_bs_set_reg_psd_ct8k_ss2(handle);
		handle->m_device_ss2.tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
		handle->m_device_ss2.tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

		mt_fe_dmd_config_clock_ct8k(handle, MtFeType_DVBS_S2);

		if(handle->m_device_ss2.lnb_cfg.bUnicable)
		{
			mt_fe_unicable_set_tuner_ct8k_ss2(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->m_device_ss2.lnb_cfg.iUBIndex, handle->m_device_ss2.lnb_cfg.iBankIndex, handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
			freq_KHz = real_freq_KHz;
		}
		else
		{
			handle->m_device_ss2.tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
		}

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x90, 0x70);

		for(i = 0; i < 2; i ++)
		{
			b_fftdone		= MtFe_False;
			b_analog_locked = MtFe_False;
			cnt = 50;
			do
			{
				_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
				b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
				_mt_sleep_ct8k(10);
				cnt--;
			}while((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs));

			if(cnt == 0)
				break;
		}

		//clock6 = clock();
		// mt_fe_print(("agc lock time %d\n", clock6 - clock7));
		mt_fe_dmd_get_strength_ct8k_ss2(handle, &dwTmp1);
		#endif

		F_grid_map_tmp[F_idx] = dwTmp1;
		sig_stre_grid = dwTmp1 + sig_stre_grid;

		if(min_sig > dwTmp1)
			min_sig = (U8)dwTmp1;

		if(max_sig <= dwTmp1)
			max_sig = (U8)dwTmp1;

		F_idx ++;
		set_freq = set_freq + F_step;
	}

	fre_seg_idx = 0;
	if(min_sig > weak_sig_thr)
	{
		range_list[2 * fre_seg_idx] = begin_freq_MHz;
		range_list[2 * fre_seg_idx + 1] = end_freq_MHz;
		fre_seg_idx ++;
	}
	else
	{
		if(F_idx == 0)
			F_idx = 1;

		sig_stre_thr = sig_stre_grid / F_idx;
		// sig_stre_thr = alpha*max(sig_stre_grid) + (1-alpha)*min(sig_stre_grid);
		sig_stre_thr = (max_sig + min_sig) / 2;
		sig_stre_grid = 0;
		map_idx = 0;

		for(i = 0; i < F_idx; i ++)
		{
			if(F_grid_map_tmp[i] >= sig_stre_thr)
			{
				sig_stre_grid = F_grid_map_tmp[i] + sig_stre_grid;
				map_idx ++;
			}
		}

		//fprintf(pFilePPG, "\n");

		if(map_idx == 0)
			ave_sig_str = MAX_sig_stre_dif;
		else
			ave_sig_str = sig_stre_grid / map_idx;

		if((ave_sig_str - MAX_sig_stre_dif) < sig_thr_low_bound)
			ave_sig_str = sig_thr_low_bound;
		else
			ave_sig_str = ave_sig_str - MAX_sig_stre_dif;

		if (ave_sig_str > sig_stre_thr)
			ave_sig_str = sig_stre_thr;

		cnt = 0;
		fre_seg_idx = 0;
		find_ok = 0;
		ct_freq_actual = begin_freq_MHz;

		for(i = 0; i < F_idx; )
		{
			if(F_grid_map_tmp[i] < ave_sig_str)
			{
				cnt ++;
			}
			else
			{
				if (cnt <= 48 / F_step)
				{
					for(j = 0; j <= cnt; j ++)
					{
						F_grid_map_tmp[i - j] = ave_sig_str + 2;
					}

					cnt = 0;
				}
			}

			i ++;
		}

		if(F_grid_map_tmp[0] >= ave_sig_str)
			range_list[0] = ct_freq_actual;

		for(i = 1; i < F_idx; )
		{
			if((F_grid_map_tmp[i - 1] < ave_sig_str) && (F_grid_map_tmp[i] >= ave_sig_str))
			{
				range_list[2 * fre_seg_idx] = ct_freq_actual + F_step * (i - 1);
			}
			else if((F_grid_map_tmp[i - 1] >= ave_sig_str) && (F_grid_map_tmp[i] < ave_sig_str))
			{
				if(range_list[2 * fre_seg_idx] == 0)
					range_list[2 * fre_seg_idx] = ct_freq_actual;

				range_list[2 * fre_seg_idx + 1] = ct_freq_actual + F_step * (i);

				if (end_freq_MHz - range_list[2 * fre_seg_idx + 1] <= F_step)
					range_list[2 * fre_seg_idx + 1] = end_freq_MHz;

				fre_seg_idx ++;
			}
			else if((i == (F_idx - 1)) && (F_grid_map_tmp[i] >= ave_sig_str))
			{
				if(range_list[2 * fre_seg_idx] == 0)
					range_list[2 * fre_seg_idx] = ct_freq_actual;

				range_list[2 * fre_seg_idx + 1] = ct_freq_actual + F_step * i;

				if (end_freq_MHz - range_list[2 * fre_seg_idx + 1] <= F_step)
					range_list[2 * fre_seg_idx + 1] = end_freq_MHz;

				fre_seg_idx ++;
			}

			i ++;
		}
	}

	if(fre_seg_idx > 1)
	{
		for(i = 1; i < fre_seg_idx; i ++)
		{
			if((range_list[2 * i] - F_step) <= range_list[2 * (i - 1) + 1])
			{
				range_list[2 * (i - 1) + 1] = range_list[2 * i + 1];
				fre_seg_idx = fre_seg_idx - 1;

				for(j = i; j < fre_seg_idx; j ++)
				{
					range_list[2 * j + 1] = range_list[2 * (j + 1) + 1];
					range_list[2 * j] = range_list[2 * (j + 1)];
				}
			}
		}
	}

	*list_num = fre_seg_idx;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_bs_process_scanned_TP_test_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 *psd, U32 cur_scan_freq_KHz, U8 bs_times, S8 *strength)
{
	MT_FE_BOOL	b_analog_locked, b_fftdone;

	U8	tmp, cnt;
	U16	totaltpnum;
	U8	tmp1, tmp2, tmp3, tmp4;
	S8	iStrength = 0;
	U32	nSize = (FFT_N - 16 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	U8	buffer[968] = {0};						// sm = 2;
	U32	dwPSDArray[FFT_N + 10];
	U32	i = 0, j = 0;
	U32	dwTmp = 0;
	S32	diff[FFT_N + 2];
	U32	tone_idx = 0;
	S32	spur_thr = 5000;
	U32	mean_tone = 0;
	U32	nCount = 0;

#if PSD_WRITE
	FILE	*fpOut;
	S32 gain_val = 0;

	char	file_name[100];

	sprintf(file_name, "PSD_%s_%s_%d.txt", handle->m_device_ss2.lnb_cfg.b22K ? "H":"L",
			 (handle->m_device_ss2.lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->m_device_ss2.tp_cfg.iFreqKHz / 1000);
#endif


	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x90, 0x70);

	for(i = 0; i < 2; i ++)
	{
		b_fftdone		= MtFe_False;
		b_analog_locked = MtFe_False;

		cnt = 50;
		do
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x0d, &tmp);
			b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
			_mt_sleep_ct8k(10);
			cnt--;
		}while((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs));

		if(cnt == 0)
		{
			mt_fe_print(("agc time over 500 ms unlock, cnt = %d\n", cnt));
			break;
		}

		_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9a, 0x80);

		cnt = 100;//50
		do
		{
			_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9a, &tmp);
			b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
			_mt_sleep_ct8k(10);
			cnt--;
		}while((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->m_device_ss2.global_cfg.bCancelBs));

		if(b_fftdone || (1 == i))
		{
			break;
		}
		else
		{
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5f, 0x00);
			_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x5e, 0x70);
		}
	}

	if(!(b_fftdone && b_analog_locked))
	{
		mt_fe_print(("agc unlock = %d or b_fftdone undone = %d !\n", b_analog_locked, b_fftdone));

		return 0;
	}


#if  (PSD_WRITE)
	fpOut = fopen(file_name, "w+");

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x5A, &tmp);
	fprintf(fpOut, "RF Gain: %d\n", tmp & 0x0f);

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x5F, &tmp);
	fprintf(fpOut, "IF Gain: %d\n", tmp & 0x0f);

	_mt_fe_tn_get_reg_ss2_ct8k(handle, 0x77, &tmp);
	fprintf(fpOut, "BB Gain: %d\n", (tmp >> 4) & 0x0f);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x3F, &tmp);
	fprintf(fpOut, "PWM: %d\n", tmp);

	//mt_fe_tn_get_gain(handle, &gain_val);
	gain_val = 0;
	fprintf(fpOut, "Tuner Gain: %d\n", gain_val);

	mt_fe_dmd_get_strength_ct8k_ss2(handle, &iStrength);
	fprintf(fpOut, "Strength: %d\n", iStrength);
	*strength = iStrength;
	fclose(fpOut);
#else

	mt_fe_dmd_get_strength_ct8k_ss2(handle, &iStrength);
	//fprintf(fpOut, "Strength: %d\n", iStrength);
	*strength = iStrength;
#endif

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9a, &tmp);
	totaltpnum = (U16)(tmp & 0x1F);

	nCount = 1;

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9A, 0x40);

	for(j = 0; j < nSize; j ++)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp);
		buffer[j] = tmp;
	}


#if 1
	dwTmp = FFT_N - 478;
	for(i = 0; i < dwTmp; i ++)
	{
		psd[nCount ++] = 0;
	}

	for(i = 2 * 2; i < nSize; i += 2)			// sm_buf = 2
	{
		dwTmp = buffer[i];
		dwTmp = dwTmp + (buffer[i + 1] * 256);
		dwPSDArray[(i - 2) / 2] = dwTmp;
		psd[nCount ++] = dwTmp;
	}

	dwTmp = FFT_N - 482 + 4;
	for(i = dwTmp + 1; i < nCount; i ++)
	{
		diff[i]= psd[i + 1] - psd[i];
	}

	for(i = dwTmp + 1; i < nCount - 1; )
	{
		diff[i] = diff[i + 1] - diff[i];
		if(diff[i] < (0 - spur_thr * 2))
		{
			if((i > (dwTmp + 2)) && (i < (nCount - 4)))
			{
				tone_idx ++;
				diff[i] = 1;
			}
			else
				diff[i] = 0;
		}
		else
			diff[i] = 0;

		i ++;
	}

	if(tone_idx > 0)
	{
		for(i = dwTmp + 1; i < nCount - 2; i ++)
		{
			if(diff[i] == 1) ////cha 1
			{
				if(psd[i + 4] > psd[i - 2])
				{
					mean_tone = psd[i + 4] - psd[i - 2];//(psd[i - 2] + psd[i - 1] + psd[i] + psd[i + 1] + psd[i + 2] + psd[i + 3] + psd[i + 4]) / 6;
					for(j = 0; j < 5; j ++)
						psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];

					mean_tone = 0;
				}
				else
				{
					mean_tone = psd[i - 2] - psd[i + 4];//(psd[i - 2] + psd[i - 1] + psd[i] + psd[i + 1] + psd[i + 2] + psd[i + 3] + psd[i + 4]) / 6;
					for(j = 0; j < 5; j ++)
						psd[i - 1 + j] = psd[i - 2] - (mean_tone * (j + 1) + 3) / 6;
					mean_tone = 0;
				}
			}
		}
	}
#endif

	nCount = nCount - 1;
	for(j = nCount; j >= 1; j --)
	{
		dwTmp = 0;
		for(i = 0; i < SM_WIN; i ++)
		{
			if(j >= i)
				dwTmp = dwTmp + psd[j - i];
		}
		psd[j] = (dwTmp + SM_WIN / 2) / SM_WIN;
	}

	j = FFT_N - 2 * OVLP;
	if((nCount) > j)
	{
		dwTmp = OVLP + SM_WIN / 2 + tuner_offset;
		for(i = 1; i <= j; i ++)
		{
			psd[i] = psd[i + dwTmp];
		}
	}


#if PSD_WRITE
	fpOut = fopen(file_name, "a");
	if (fpOut != NULL)
	{
		for(i = 1; i < nCount; i ++)
		{
			fprintf(fpOut, "%d\n", psd[i]);//dwPSDArray[i]);
		}
		fclose(fpOut);
	}
#endif

	nCount = j;
	_mt_sleep_ct8k(50);

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0x9A, 0x20);

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp1);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp2);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp3);
	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x9B, &tmp4);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_bs_A_test_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U8 bs_times)
{
	//global gain_RF  gain_IF gain_BB gain_PWM gain_tuner sig_stre path_com PSD_AVE_N;
	//load FcFs_Tab.mat;
	//PSD_SRC = 'External1';//%using captured PSD for simulation
	U32 psd[FFT_N + 32];
	U32 SR_min = 1200;//% min symbol rate in Msps
	//double C = 3 / 16;//%%%Ufix6_En6
	U8  K1 = 12;
	U16 K_thr1 = 707;
	U16 K_thr2 = 600;
	U8  SEARCH_STEP = 1;//% increasing step for the CH searching in one PSD
	//U32 PSD_AVE_N = 96;//128;  //                 %%%% specified FFT average time to produce a PSD
	//% OVLP = 192;%for External1 data source;
	U32 FORCE_TUNER_SKIP = 8;//24;//%MHz, When no CH found, force Tuner to skip in freq
	U32 ris_point_lim = 12;//8;
	U32 verf_range = 16;

	//%------------------- Variables initialization --------------------------
	U32 ct_freq_start = 950;
	U32 ct_freq_end = 2150;
	//S8	sig_stre_thr = 30;

	//%---------------------------------------------------------------------------------------------------------------
	U32 tuner_offset_en = 1;
	//U32 FcFs_Tab_en = 0;

	/*for rep_times=1*/

	///%     mode_sel =  'find_next_notch';
	U32 first_psd_flag = 1;//% the currrent search is the very first search attemp of current satellite
	U32 psd_SN = 1;//% PSD serial number
	U32 cnt_rep_psd = 0;
	U16 K_thr = 707;
	//U32 CHID = 1;
	//U32 ChTb[]={};

	////////////////////////ADD BY WBJ
	S8 sig_stre = 0;
	U32 thr_notch_tureTP = 0;
	//U32 val = 0;
	U32 val = 0;

	U32 idx_ext_min = 0, idx_ext_max = 0;
	U32 ext_min = 0, ext_max = 0;
	U32 idx_psd_shift = 0;

	U32 freq_KHz = 0, tmp_tp_scanned = 0;//, tmp_freq_KHz;
	MT_FE_RET ret = MtFeErr_Ok;
	U32 real_freq_KHz = 0;
	//clock_t	clock4, clock5;

	U32 idx_up = 1;
	U32 idx_dw = 1;
	U32 hit_psd_right_bound = 0;
	U32 found_no_ch_flag = 1;
	U32 idx_curr = 1;
	U32 split_flag = 0;
	U32 cnt_start_ris = 0;
	U32 cnt_notch = 0;

	U32 idx_start = 1;
	U32 test_index = 0;


	U32 test_value = 0;
	//	MT_FE_CH_TPE ch_type = Fake_CH;

	//	if(mt_fe_tn_set_tuner == NULL)
	//	return MtFeErr_NullPointer;
	U32 F_range_list[18 * 2]; //16*2
	U8  fre_seg_idx = 0;
	U8  i = 0;
	U32 j = 0;
	U32 idx_tmp = 0, idx_pwr_rg_start = 0, idx_pwr_rg_end = 0;
	U32 iTunerOffsetKHz = 0;
	U8  flag_scan_done = 0;
	U32 sig_thr = 0;
	U32 sig_level = 0;
	U32 sig_idx = 0;
	U32 nCount = 0;
	//////form gloab
	U32 ct_freq_actual = 0;
	U8  same_freq = 0;
	///////////

	tmp_tp_scanned = 0;
	ct_freq_start = begin_freq_MHz;
	ct_freq = ct_freq_start * 1000;
	ct_freq_end = end_freq_MHz - 2;
	ct_freq_actual = ct_freq;
	tuner_offset = 0;

	//memset(F_range_list,0,36*sizeof(U32));
	//////////////////////////////
	//clock4 = clock();
	//_mt_fe_dmd_bs_process_get_range_list_ct8k_ss2(handle, begin_freq_MHz, end_freq_MHz - 2, F_range_list, &fre_seg_idx);
	//clock5 = clock();
	//mt_fe_print(("get strength time %d\n", clock5 - clock4));

	fre_seg_idx = 1;
	F_range_list[0] = begin_freq_MHz;
	F_range_list[1] = end_freq_MHz;
	///////////////////////////////////
	#if 0
	for(i = 0; i < fre_seg_idx; i ++)
	{
	#endif
		ct_freq_start = F_range_list[2 * i];//+2;
		ct_freq_end = F_range_list[2 * i + 1];//+2;
		flag_scan_done = 0;
		ct_freq = ct_freq_start * 1000;
		ct_freq_actual = ct_freq;
		first_psd_flag = 1;
		cnt_rep_psd = 0;
		SymRate_last = -30;//%ChTb(CHID-1,2);
		CtFreq_last = 0;//%ChTb(CHID-1,1
		idx_dw_last = -100 * (FFT_N - 2 * OVLP);
		K_thr = K_thr1;
			//mt_fe_print(("start  %f  End %f\n", ct_freq_start, ct_freq_end));
		//mt_fe_print(("Range %d / %d, Start %6d;  End %6d\n", i + 1, fre_seg_idx, (U32)ct_freq_start, (U32)ct_freq_end));

		//while ((ct_freq <= ct_freq_end) && (!handle->m_device_ss2.global_cfg.bCancelBs))
		while ((flag_scan_done == 0) && (!handle->m_device_ss2.global_cfg.bCancelBs))
		{
			//  %%%%%%%%%%%%%%%%% FFT %%%%%%%%%%%%%%%%%%%%
			/*  [psd, psd_fft] = load_psd(ct_freq, PSD_SRC, FFT_N, OVLP,SM_WIN, tuner_offset);*/
			//   %%%%%%%%%%%%% Initialization before every window PSD    %%%%%%%%%

			#if FIND_DEBUG
			FILE	*fpOut;
			S32 gain_val = 0;
			S8 iStrength = 0;
			char	file_name[100];



			U32		nSize = (FFT_N - 16 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
			U8		buffer[968] = {0};						// sm = 2;
			//U32		dwPSDArray[484];						// 484 / 2
			//U32		dwPSDArray[FFT_N + 10];
			U32		i = 0, j = 0;
			U32		dwTmp = 0;
			char str[100];
			char str1[20];
			S32 diff[FFT_N + 2];
			U32 tone_idx = 0;
			S32 spur_thr = 5000;
			U32 mean_tone = 0;
			#endif


			// load ([path_com 'ChTb_ref.mat']);
			//////////////add b wbj///////////////
			freq_KHz = ct_freq;

			#if FIND_DEBUG
			#else
			_mt_fe_dmd_bs_set_reg_psd_ct8k_ss2(handle);
			#endif

			handle->m_device_ss2.tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
			handle->m_device_ss2.tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

			mt_fe_dmd_config_clock_ct8k(handle, MtFeType_DVBS_S2);

#if FIND_DEBUG
#else
			if(handle->m_device_ss2.lnb_cfg.bUnicable)
			{
				mt_fe_unicable_set_tuner_ct8k_ss2(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->m_device_ss2.lnb_cfg.iUBIndex, handle->m_device_ss2.lnb_cfg.iBankIndex, handle->m_device_ss2.lnb_cfg.iUBFreqMHz);
				freq_KHz = real_freq_KHz;
			}
			else
			{
				handle->m_device_ss2.tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
			}

			handle->m_device_ss2.tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
			_mt_fe_dmd_set_carrier_offset_ct8k_ss2(handle, iTunerOffsetKHz);

			ret = _mt_fe_dmd_bs_process_scanned_TP_test_ct8k_ss2(handle, psd, freq_KHz, bs_times, &sig_stre);
#endif


#if FIND_DEBUG
			sprintf(file_name, "PSD_%s_%s_%d.txt", handle->m_device_ss2.lnb_cfg.b22K ? "H":"L", (
				handle->m_device_ss2.lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->m_device_ss2.tp_cfg.iFreqKHz / 1000);

			nCount = 1;

			fpOut = fopen(file_name, "r");
			if (fpOut != NULL)
			{
				fgets(str, 100, fpOut);
				fgets(str, 100, fpOut);
				fgets(str, 100, fpOut);
				fgets(str, 100, fpOut);
				fgets(str, 100, fpOut);
				fgets(str, 100, fpOut);

				strmcp(str1, str, 9);

				dwTmp = FFT_N - 482 + 4;

				for(i = 0; i < dwTmp; i ++)
				{
					psd[nCount ++] = 0;
				}

				while (!feof(fpOut))
				{
					fscanf(fpOut,"%d\n", &dwTmp);
					psd[nCount ++] = dwTmp;
				}
				fclose(fpOut);
			}
#if 1
			dwTmp = FFT_N - 482 + 4;
			for(i = dwTmp + 1; i < nCount; i ++)
			{
				diff[i] = psd[i + 1] - psd[i];
			}

			for(i = dwTmp + 1; i < nCount - 1; )
			{
				diff[i]= diff[i + 1] - diff[i];
				if(diff[i] < (0 - spur_thr * 2))
				{
					if((i > (dwTmp + 2)) && (i < (nCount - 4)))
					{
						tone_idx ++;
						diff[i] = 1;
					}
					else
						diff[i] = 0;
				}
				else
					diff[i] = 0;

				i ++;
			}

			if(tone_idx > 0)
			{
				for(i = dwTmp + 1; i < nCount - 2; i ++)
				{
					if(diff[i] == 1) ////cha 1
					{
						if(psd[i + 4] > psd[i - 2])
						{
							mean_tone = psd[i + 4] - psd[i - 2];
							for(j = 0; j < 5; j ++)
								psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];
							mean_tone = 0;
						}
						else
						{
							mean_tone = psd[i - 2] - psd[i + 4];
							for(j = 0; j < 5; j ++)
								psd[i - 1 + j] = psd[i - 2] - (mean_tone * (j + 1) + 3) / 6;
							mean_tone = 0;
						}
					}
				}
			}
#endif	// #if 1

			nCount = nCount - 1;
			for(j = nCount; j >= 1; j --)
			{
				dwTmp = 0;
				for(i = 0; i < SM_WIN; i ++)
				{
					dwTmp = dwTmp + psd[j - i];
				}

				psd[j] = (dwTmp + SM_WIN / 2) / SM_WIN;
			}

			j = FFT_N - 2 * OVLP;
			if((nCount) > j)
			{
				if(tuner_offset > 0)
					dwTmp = OVLP + (SM_WIN + 1) / 2 + tuner_offset;
				else
					dwTmp = OVLP + (SM_WIN + 1) / 2 - (U8)tuner_offset;
				for(i = 1; i <= j; i ++)
				{
					psd[i] = psd[i + dwTmp];
				}
			}
#endif	// #if FIND_DEBUG

			nCount = 512;
			idx_ch_max = 1;
			idx_ch_min = 1;
			idx_notch = 1;
			idx_up = 1;
			idx_dw = 1;
			hit_psd_right_bound = 0;
			found_no_ch_flag = 1;
			idx_curr = 1;
			split_flag = 0;
			cnt_start_ris = 0;
			cnt_notch = 0;
			idx_notch_last = 1;
			idx_psd_start = 1;
			idx_start = 1;

			ch_max = psd[1];
			ch_min = psd[1];
			ch_noise_lev = psd[1];
			idx_notch_last_tureCh = 1;
			idx_psd_shift = 1;

			if(psd_offset > 9)
				mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, (psd_offset - 8), (psd_offset + 8));
			else
				mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, 1, (psd_offset + 8));

			while(idx_notch_last < 64)
			{
				if(psd[idx_notch_last + 1] <= psd[idx_notch_last])
					idx_notch_last = idx_notch_last + 1;
				else
					break;
			}

			idx_notch = idx_notch_last;
			idx_psd_start = idx_notch_last;
			idx_start = idx_notch;
			idx_curr = idx_notch;
			ch_noise_lev = psd[idx_notch_last];

			while( idx_curr <= FFT_N - 2 * OVLP)
			{
				//	%             switch mode_sel
				//	%                 case 'find_next_notch'
				//	%                             thr_notch_tureTP = ch_noise_lev*K1;% Effective notch threshold
				if (psd[idx_curr] >= ch_max) //%%%% update the max of the PSD in the range of next possible ch
				{
					ch_max = psd[idx_curr];
					idx_ch_max = idx_curr;
				}
				else if (psd[idx_curr] <= ch_min) //%%%% update the min of the PSD in the range of next possible ch
				{
					ch_min = psd[idx_curr];
					idx_ch_min = idx_curr;
				}

				//  %------------------- searching the next notch --------------------------------
				if ((idx_curr > idx_ch_max) || ( cnt_notch > 0))//% after a  first rising edge,during a falling edge OR
				{
					if (psd[idx_curr] <= psd[idx_notch])// % update the notch point
					{
						idx_notch = idx_curr;
						cnt_notch = 0;
					}
					else if ( psd[idx_curr] > psd[idx_notch])// % if notch point stays, then  cnt+
					{
						cnt_notch = cnt_notch + 1;//SEARCH_STEP;

						if (idx_curr >= FFT_N - 2 * OVLP)//%if a notch has been found near the right hand bound of PSD
							cnt_notch = ris_point_lim;
					}

					if (cnt_notch == ris_point_lim )//%&& idx_notch-idx_notch_last > 8)//%% if notch point stays long enough
					{
						if (idx_notch_last > idx_notch)
						{
							//disp('Opps');
						}
						mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
							/*[ch_max idx_ch_max] = max(psd(idx_notch_last:idx_notch));*///% find the peak in current searched range
							//[ch_min idx_ch_min] = min(psd(idx_notch_last:idx_notch));% find the min in current searched range
						mt_fe_dmd_bs_find_Min(psd, &ch_min, &idx_ch_min, idx_notch_last, idx_notch);
						idx_ch_max = idx_ch_max + idx_notch_last - 1;//% to align the idx with psd start
						idx_ch_min = idx_ch_min + idx_notch_last - 1;
							//ch_noise_lev = min([ch_min psd(idx_notch) ch_noise_lev]);
						if(psd[idx_notch] >= ch_min)
						{
							if(ch_noise_lev > ch_min)
								ch_noise_lev = ch_min;
						}
						else
						{
							if(ch_noise_lev > psd[idx_notch])
								ch_noise_lev = psd[idx_notch];
						}

						if (psd[idx_notch] < ch_noise_lev)
						{
							ch_noise_lev = psd[idx_notch];
						}

					//	idx_notch_last_tmp = idx_notch_last;

						sig_level = 0;
						sig_idx = 0;

						for(idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp ++)
						{
							sig_level = sig_level + psd[idx_tmp];
							sig_idx = sig_idx + 1;
						}

						if(sig_idx != 0)
							sig_thr = sig_level / sig_idx;
						else
						{
							mt_fe_print(("\tmt_fe_dmd_bs_A_test_ct8k_ss2_2 ---- sig_idx = 0!\n"));
						}

						sig_level = 0;
						sig_idx = 0;

						for(idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp ++)
						{
							if(psd[idx_tmp] >= sig_thr)
							{
								sig_level = sig_level + psd[idx_tmp];
								sig_idx = sig_idx + 1;
							}
						}

						if(sig_idx != 0)
							ave_peak = sig_level / sig_idx;
						else
						{
							mt_fe_print(("\tmt_fe_dmd_bs_A_test_ct8k_ss2_1 ---- sig_idx = 0!\n"));
						}

						thr_notch_tureTP = (ch_noise_lev * K_thr + ave_peak * (1000 - K_thr) + 500) / 1000;//% once a notch found, update thr_notch_tureTP

						if (((psd[idx_notch] < thr_notch_tureTP ) && (ch_max > thr_notch_tureTP)) || (first_psd_flag == 1))
						{
							//       % if notch level is low enough and its peak is high enough
							//      % or the currrent search is the very first search attemp of current satellite, than record the notch as an effective notch
							//       %======== Found a CH ============
							K_thr = K_thr1;
							if (first_psd_flag == 0)// % if an effective CH found, just measure it (always skip the first found ch in every start of Blind scan)
							{
								if ((4000 * idx_notch - 4000 * idx_notch_last) > (SR_min * 5 * FFT_N / Fs))//% filter the small notch
								{
									mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd, psd_SN);
								}
								else
								{
									if (idx_psd_start == idx_notch_last)
										idx_psd_start = idx_notch;
								}
							}

							//plot_notch_search;
							idx_notch_last = idx_notch;
							idx_start = idx_notch;
							ch_min = psd[idx_notch];
							idx_ch_min = idx_notch;
							ch_max = psd[idx_notch];
							idx_ch_max = idx_notch;
							first_psd_flag = 0;
						}
						else//%if the peak is lower than thr,implies it is a fake ch, just skip it
						{
							if (ch_max < thr_notch_tureTP) //% if the peak is lower than thr, implies that the notch is lower than thr also,  the found CH is uneffecitve,record it and skip
							{
								//ch_type = Noise_CH;
								//plot_notch_search;
								if( psd[idx_notch] < psd[idx_notch_last])// % if the notch is lower than the last notch,  then record the notch as an effective notch too
								{
									idx_notch_last = idx_notch;
									idx_start = idx_notch;
								}

								ch_min = psd[idx_notch];
								idx_ch_min = idx_notch;
								ch_max = psd[idx_notch];
								idx_ch_max = idx_notch;
							}
							else// if (psd[idx_notch] >= thr_notch_tureTP)// % the notch is not low enough, implies the CH is still in the rising stage, keep the idx_notch_last unchange
							{
								if((1000 * psd[idx_notch] > (707 * thr_notch_tureTP)) && (((1000 - 707) * ave_peak) > (1000 * psd[idx_notch] - 707 * thr_notch_tureTP)))
									//if ((ave_peak - psd[idx_notch]) / ((ave_peak  - thr_notch_tureTP)) > 0.707)
								{
									//  % if the peak is relative high enough, then ture TP decision is depend on if the
									//   % right neibough zone has another loewr notch
									if(idx_notch >= idx_notch_last)
										verf_range = (U32)(2 * (idx_notch - idx_notch_last) / 5 + 1) / 2;
									else
										verf_range = 0;

									if ((idx_notch + verf_range) <= (FFT_N - 2 * OVLP))
									{
										mt_fe_dmd_bs_find_Min(psd, &ext_min, &idx_ext_min, (idx_notch + 1), (idx_notch + verf_range));
										mt_fe_dmd_bs_find_Max(psd, &ext_max, &idx_ext_max, (idx_notch + 1), (idx_notch + verf_range));
										//[ext_min idx_ext_min] = min(psd(idx_notch+[1:verf_range]));
										//[ext_max idx_ext_max] = max(psd(idx_notch+[1:verf_range]));
										if ((idx_ext_min > idx_ext_max))//  && ((ext_max - psd[idx_notch])< (psd[idx_notch]-ext_min)))
										{	//  % if a lower notch found in the
											//   % right hand zone, go on finding
											//  % next notch without skipping
											//   % current notch
											//ch_type = Split_CH;
											//plot_notch_search;
											//disp(' ');
										}
										else
										{
											//       %if no lower notch found in the right hand zone
											mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd, psd_SN);
											//	ch_type = True_CH;
											//	mt_fe_print(("true_ch2,index=%d\n",idx_notch));
												//plot_notch_search;
											idx_notch_last = idx_notch;
											idx_start = idx_notch;
										}
									}
									else//%idx_notch+verf_range > (FFT_N-2*OVLP)
									{
										//ch_type = Split_CH;
										//plot_notch_search;
										//disp(' ');
									}
								}
								else//%(ch_max - psd(idx_notch)) / ((ch_max  - thr_notch_tureTP)) < 0.707, implies the peak might not high enough
								{
									//ch_type = Split_CH;
									//plot_notch_search;
									if (psd[idx_start] < thr_notch_tureTP)//% when a rapid rising edge occurs(means left rising edge has a sharper slope than falling edge)
									{
										idx_notch_last = idx_start;//% Keep the idx_notch_last
									}

									idx_start = idx_notch;
									//  % disp('Split');
								}
								ch_min = psd[idx_notch];//%???
								idx_ch_min = idx_notch;//%???
								ch_max = psd[idx_notch];//%??? initial the ch_max for next notch search process
								idx_ch_max = idx_notch;//%???
							}
						}

						cnt_notch = 0;//% clear the counter after decision of a notch
						idx_curr = idx_notch;//% move idx_curr forward to currnet found notch
					}
				}
				else//% if idx_curr is sitll in a rising edge
				{
					idx_notch = idx_curr;
				}

				//   %increase the idx_curr by SEARCH_STEP points
				idx_curr = idx_curr + SEARCH_STEP;
				//   % if PSD ended
				if (idx_curr > FFT_N - 2 * OVLP)
				{
					psd_SN = psd_SN + 1;
					mt_fe_dmd_bs_find_Max(psd, &test_value, &test_index, 1, nCount);
						//if (max(psd) < ch_noise_lev * K1)// % no ch high enough through out the whole PSD
					if (test_value * 10 < ch_noise_lev * K1)// % no ch high enough through out the whole PSD
					{
						idx_psd_shift = FFT_N - 2 * OVLP;//% jump the whole PSD to begin with next search
						K1 = 12;
						K_thr = K_thr1;
					}
					else//%if max(psd) >= thr_notch_tureTP
					{
						if (idx_notch_last == idx_psd_start)// % no effective notch but peak is high enough,implies thr may be too low
						{
							if (cnt_rep_psd < 1)
							{
								idx_psd_shift = 1;
								mt_fe_dmd_bs_find_Max(psd, &test_value, &test_index, 1, nCount);

								if(test_value >= 50)
								{
									if((100 * test_value - 5000) > (U32)(12 * K1))
										K1= 12 * K1 / 10;
									else
										K1= 10 * test_value - 500;
								}
								else
									K1= 12 * K1 / 10;

								//K1 = min(K1*1.2,(max(psd)-50)/ch_noise_lev);
								K_thr = K_thr2;
								cnt_rep_psd = cnt_rep_psd + 1;
								//----- initial the current PSD??scan vairables again ------------
								ch_max = psd[1];
								ch_min = psd[1];
								idx_ch_max = 1;
								idx_ch_min = 1;
								idx_up = 1;
								idx_dw = 1;
								cnt_notch = 0;
								idx_notch_last_tureCh = 1;
								// ch_type = 'Fake Ch';
								//                    wide_TP_flag = 0;//% If current found TP is a wide TP, set to '1'
								//  idx_pwr_rg = zeros(FFT_N,1);
								psd_SN = psd_SN - 1;
								//------------------------------------------------------
							}
							else//% cnt_rep_psd == 1
							{
								cnt_rep_psd = 0;
								idx_psd_shift = (FFT_N * FORCE_TUNER_SKIP + Fs / 2) / Fs;//% skip the whole psd
								K1 = 12;//% recovery the original K1 thr
								K_thr = K_thr1;
							}
						}
						else//% at least one notch ever found in current PSD
						{
							if (idx_notch_last > ((FFT_N - 2 * OVLP) / 8))
							{
								if (idx_notch_last_tureCh > (FFT_N - 2 * OVLP) / 2) //% if last found TP fall in the left half of PSD
									idx_psd_shift = idx_notch_last_tureCh;//-8;//% then shift the PSD according the last TP position to aviod LPF effect
								else
									idx_psd_shift = idx_notch_last;
							}
							else
							{
#if 0
								if( idx_notch_last > 40)
								{
									idx_psd_shift = idx_notch_last;//-8;//% make the tuner jump according to last effective notch
										//if(idx_notch_last_tureCh>8)
										//	idx_psd_shift = idx_notch_last-8;
								}
								else
#endif
								{
									idx_psd_shift = idx_notch_last;
								}
							}
							K1 = 12;
						}
					}

					if ((psd[FFT_N - 2 * OVLP] < thr_notch_tureTP) && (idx_notch_last == idx_psd_start))//%if no TP ever found in this PSD
					{
						//idx_pwr_rg = find(psd>=thr_notch_tureTP);
						//idx_pwr_rg_start = idx_pwr_rg(1);
						idx_tmp = 0;// idx_pwr_rg_end = idx_pwr_rg(length(idx_pwr_rg));

						for(j = 1; j <= FFT_N - 2 * OVLP; j ++)
						{
							if(psd[j] >= thr_notch_tureTP)
							{
								idx_tmp ++;
								idx_pwr_rg_end = j;
							}

							if(idx_tmp == 1)
								idx_pwr_rg_start = j;
						}

						if (idx_tmp > 0)
						{
							if(idx_pwr_rg_start < ((FFT_N - 2 * OVLP) / 4))
							{
								idx_notch_last = 1;
								if((idx_pwr_rg_end + (idx_tmp + 2) / 4) > (FFT_N - 2 * OVLP))
									idx_notch = FFT_N - 2 * OVLP;//mt_fe_dmd_bs_find_Min(psd, &ext_min, &idx_notch,FFT_N-2*OVLP,idx_pwr_rg_end+(idx_tmp+2)/4);//;
								else
									idx_notch = idx_pwr_rg_end + (idx_tmp + 2) / 4;
								idx_curr = FFT_N - 2 * OVLP;
								// wide_TP_flag = 1;
								mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd ,psd_SN);
								//    plot_notch_search;
								idx_curr = FFT_N - 2 * OVLP + 1;
								idx_psd_shift = idx_notch;
							}
						}
					}

					//	 Fs_last = Fs;
					idx_dw_last = idx_dw_last - 100 * idx_psd_shift + 100;
					ct_freq_actual = (ct_freq_actual + (idx_psd_shift - 1) * Fs * 1000 / FFT_N + 50) / 100 * 100;
					ct_freq = ct_freq_actual - (U32)ct_freq_actual % 2000;

					/*if (FcFs_Tab_en == 1)
					{
						freq_edge = ct_freq_actual - Fs * ((FFT_N - 2 * OVLP) / FFT_N) / 2;
						ct_freq_tmp = ct_freq + [-4 : 2 : 4];
						freq_diff = 10;
						for idx = 1:length(ct_freq_tmp)
							Fs_tmp = FcFs_Tab(find(FcFs_Tab(: , 1) == ct_freq_tmp(idx)), 2) / 1e3;
							freq_edge_tmp(idx) = ct_freq_tmp(idx) - Fs_tmp * ((FFT_N - 2 * OVLP) / FFT_N) / 2;
							if freq_edge_tmp(idx) < freq_edge && freq_edge - freq_edge_tmp(idx) < freq_diff
								Fs = Fs_tmp;
								ct_freq = ct_freq_tmp(idx);
								freq_diff = freq_edge - freq_edge_tmp(idx);
						end
						end
						ct_freq_actual = ct_freq + freq_diff;
					}
					else*/
					{
						Fs = 96;
					}

					if(ct_freq_actual > ct_freq)
					{
						tuner_offset = (2 * ((ct_freq_actual - ct_freq) * FFT_N / Fs) + 1000) / 2000;
					}
					else
					{
						tuner_offset = (2 * ((S32)(ct_freq_actual - ct_freq) * FFT_N / Fs) - 1000) / 2000;
					}
					if (tuner_offset_en == 0)
					{
						ct_freq_actual = ct_freq;//% ct_freq_actual equals to ct_freq
						psd_offset = tuner_offset;
						tuner_offset = 0;
					}
					else
					{
						ct_freq_actual = ct_freq  + 1000 * tuner_offset * Fs / FFT_N;
						psd_offset = 0;
					}

					if(FFT_N > 2 * OVLP)
					{
						if (ct_freq >= (1000 * ct_freq_end))
							flag_scan_done = 1;
					}
					else
					{
						if((ct_freq + (2000 * OVLP - FFT_N) / 2 / FFT_N * Fs) > (1000 * ct_freq_end))
							flag_scan_done = 1;
						else
						{
							flag_scan_done = 0;
							if (ct_freq >= (1000 * ct_freq_end))
								flag_scan_done = 1;
						}
					}

					if(ct_freq == freq_KHz)
					{
						same_freq ++;
						if(same_freq > 3)
						{
							mt_fe_print(("find same freq %d\n", ct_freq));
							ct_freq = ct_freq + 8000;
						}
					}
					else
						same_freq = 0;
				}
			}

			//clock5 = clock();
			//mt_fe_print(("cal tp time %dms\n", clock5 - clock4));

		//mt_fe_dmd_rs6k_bs_lock_test(handle, p_bs_info);
		}

	#if 0
	}
	#endif
	p_bs_info->next_freq_KHz = freq_KHz;
	//clock4 = clock();
	//mt_fe_print(("find tp time %d\n", clock4 - clock5));

	#if FIND_DEBUG
	#else
	_mt_fe_dmd_bs_connect_ct8k_ss2(handle, p_bs_info);
	#endif

	//clock5 = clock();
	//mt_fe_print(("lock  tp time %d\n", clock5 - clock4));
	//mt_fe_bs_notify_ct8k(MtFeMsg_BSFinish, p_bs_info);
	//    %---------------------- plot CH map according to the Blind Scan results -------------------
	//    %---------------------- Compare the scan result to reference Channel table --------------
	//Scan_result_stat;
	//CH_mis_fake_rec(rep_times,1:2) = [CH_mis_num cnt_Fake_CH];

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_blindscan_ct8k_ss2(MT_FE_CT8K_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info)
{
	U8 times = 0;

	if((p_bs_info->p_tp_info == NULL) || (p_bs_info->tp_max_num == 0))
		return MtFeErr_Param;

	mt_fe_dmd_global_reset_ct8k_ss2(handle);

	mt_fe_bs_notify_ct8k(MtFeMsg_BSStart, 0);

	handle->m_device_ss2.global_cfg.iScannedTpCnt	 = 0;
	handle->m_device_ss2.global_cfg.iLockedTpCnt	 = 0;
	//handle->m_device_ss2.global_cfg.bCancelBs		 = FALSE;
	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ handle->m_device_ss2.lnb_cfg.bUnicable;		// XOR
	handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted ^ (handle->m_device_ss2.lnb_cfg.bUnicable && (handle->m_device_ss2.lnb_cfg.iUBVer != 2));		// XOR
	//handle->m_device_ss2.board_cfg.bSpectrumInverted = handle->m_device_ss2.board_cfg.bIQInverted;		// XOR
	handle->m_device_ss2.board_cfg.bSpectrumInverted ^= handle->m_device_ss2.lnb_cfg.bSpectrumInverted;


	p_bs_info->tp_num = 0;

	_mt_fe_dmd_bs_A_test_ct8k_ss2(handle, begin_freq_MHz, end_freq_MHz, p_bs_info, times);

	if (!handle->m_device_ss2.global_cfg.bCancelBs)
	{
		_mt_sleep_ct8k(1000);

		_mt_fe_dmd_bs_remove_unlocked_TP_ct8k_ss2(handle, p_bs_info, (U32)FREQ_MAX_KHz);
	}

	mt_fe_bs_notify_ct8k(MtFeMsg_BSFinish, p_bs_info);

	return MtFeErr_Ok;
}


MT_BOOL _mt_fe_dmd_check_MIS_ct8k_ss2(MT_FE_CT8K_Device_Handle handle)
{
	MT_BOOL bMIS = FALSE;

	U8 tmp;
	int iCnt = 0;

	_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0x08, &tmp);
	if((tmp & 0x08) == 0x00)	// DVB-S
	{
		return bMIS;
	}

	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xE6, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xE8, 0x00);
	_mt_fe_dmd_set_reg_ss2_ct8k(handle, 0xE8, 0x01);


	do
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xE8, &tmp);

		if((tmp & 0x10) == 0x10)
		{
			break;
		}

		_mt_delay_ct8k(1);

		iCnt ++;
	} while (iCnt < 100);

	if(iCnt != 100)
	{
		_mt_fe_dmd_get_reg_ss2_ct8k(handle, 0xE9, &tmp);

		if((tmp & 0x20) != 0x20)
		{
			bMIS = TRUE;
		}
		else
		{
			bMIS = FALSE;
		}
	}

	return bMIS;
}


#endif

