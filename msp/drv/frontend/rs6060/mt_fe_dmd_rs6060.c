/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
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
 *	FILE:				mt_fe_dmd_rs6060.c
 *
 *	VERSION:			2.02.11
 *
 *	DESCRIPTION:
 *			Define the interfaces of RS6060.
 *
 *   History:
 *   Description     Version     Date            Author
 *----------------------------------------------------------------------------
 *   File Create     2.00.00     2017.03.12      YZ.Huang
 *   Modify          2.00.01     2017.03.23      YZ.Huang
 *   Modify          2.00.02     2017.03.29      YZ.Huang
 *   Modify          2.00.03     2017.03.29      YZ.Huang
 *   Modify          2.00.04     2017.03.30      YZ.Huang
 *   Modify          2.00.05     2017.03.31      YZ.Huang
 *   Modify          2.00.06     2017.04.07      YZ.Huang
 *   Modify          2.00.07     2017.04.11      YZ.Huang
 *   Modify          2.00.08     2017.04.12      YZ.Huang
 *   Modify          2.00.09     2017.04.20      YZ.Huang
 *   Modify          2.00.10     2017.04.25      YZ.Huang
 *   Modify          2.00.11     2017.05.02      YZ.Huang
 *   Modify          2.00.12     2017.05.08      YZ.Huang
 *   Modify          2.00.13     2017.05.09      YZ.Huang
 *   Modify          2.00.14     2017.05.16      YZ.Huang
 *   Modify          2.00.15     2017.05.24      YZ.Huang
 *   Modify          2.00.16     2017.05.25      YZ.Huang
 *   Modify          2.00.17     2017.05.26      YZ.Huang
 *   Modify          2.01.00     2017.07.05      YZ.Huang
 *   Modify          2.01.01     2017.07.14      YZ.Huang
 *   Modify          2.01.02     2017.08.02      YZ.Huang
 *   Modify          2.01.03     2017.08.11      YZ.Huang
 *   Modify          2.01.04     2017.08.22      YZ.Huang
 *   Modify          2.01.05     2017.09.01      YZ.Huang
 *   Modify          2.01.06     2017.09.01      YZ.Huang
 *   Modify          2.01.07     2017.09.11      YZ.Huang
 *   Modify          2.01.08     2017.10.20      YZ.Huang
 *   Modify          2.01.09     2017.11.14      YZ.Huang
 *   Modify          2.01.10     2017.11.30      YZ.Huang
 *   Modify          2.01.11     2017.12.11      YZ.Huang
 *   Modify          2.01.12     2017.12.12      YZ.Huang
 *   Modify          2.01.13     2018.01.15      YZ.Huang
 *   Modify          2.01.14     2018.01.16      YZ.Huang
 *   Modify          2.01.15     2018.02.01      YZ.Huang
 *   Modify          2.01.16     2018.06.13      YZ.Huang
 *   Modify          2.01.17     2018.10.11      YZ.Huang
 *   Modify          2.01.18     2018.10.16      YZ.Huang
 *   Modify          2.01.19     2018.10.25      YZ.Huang
 *   Modify          2.02.00     2019.01.24      YZ.Huang
 *   Modify          2.02.01     2019.04.11      YZ.Huang
 *   Modify          2.02.02     2019.07.03      YZ.Huang
 *   Modify          2.02.03     2019.07.05      YZ.Huang
 *   Modify          2.02.04     2019.08.01      YZ.Huang
 *   Modify          2.02.05     2019.09.06      YZ.Huang
 *   Modify          2.02.06     2020.12.16      YZ.Huang
 *   Modify          2.02.07     2021.01.19      YZ.Huang
 *   Modify          2.02.08     2021.03.22      YZ.Huang
 *   Modify          2.02.09     2022.03.01      YZ.Huang
 *   Modify          2.02.10     2022.07.21      YZ.Huang
 *   Modify          2.02.11     2022.10.20      YZ.Huang
 *****************************************************************************/

#include <linux/printk.h>
#include <linux/slab.h>

#include "mt_fe_def.h"
#include "mt_fe_i2c_rs6060.h"
#include "mt_fe_dmd_rs6060_fw.h"
#include "mt_fe_dmd_rs6060_reg_tbl.h"



/************************
****external function****
*************************/
extern void _mt_sleep(U32 ms);

extern mt_void mt_unf_fe_calc_PLS_gold_code(u8 *pNormalCode, u32 PLSGoldCode);


/************************
****internal function****
*************************/
MT_FE_RET _mt_fe_dmd_rs6060_set_demod(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type);
MT_FE_RET _mt_fe_dmd_rs6060_init_reg(MT_FE_RS6060_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size);
MT_FE_RET _mt_fe_dmd_rs6060_download_fw(MT_FE_RS6060_Device_Handle handle, const U8* p_fw, U16 fw_size);
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_out_mode(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE mode);
MT_FE_RET _mt_fe_dmd_rs6060_set_clock_ratio(MT_FE_RS6060_Device_Handle handle);
MT_FE_RET _mt_fe_dmd_rs6060_set_sym_rate(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_rs6060_get_sym_rate(MT_FE_RS6060_Device_Handle handle, U32 *sym_rate_KSs);
MT_FE_RET _mt_fe_dmd_rs6060_set_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_get_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_get_total_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 *carrier_offset_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_get_fec(MT_FE_RS6060_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type);

MT_FE_RET _mt_fe_dmd_rs6060_select_mclk(MT_FE_RS6060_Device_Handle handle, U32 tuner_freq_MHz);
MT_FE_RET _mt_fe_dmd_rs6060_get_ts_mclk(MT_FE_RS6060_Device_Handle handle, U32 *p_MCLK_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_mclk(MT_FE_RS6060_Device_Handle handle, U32 MCLK_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_divide_ratio(MT_FE_RS6060_Device_Handle handle, MT_FE_TYPE type, U8 dr_high, U8 dr_low);
MT_FE_RET _mt_fe_dmd_rs6060_get_current_ts_mode(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE *p_ts_mode);


/* function pointer */
S32 (*mt_fe_tn_get_offset)(MT_FE_RS6060_Device_Handle handle);
MT_FE_RET (*mt_fe_tn_set_tuner)(MT_FE_RS6060_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz);
MT_FE_RET (*mt_fe_tn_cal_offset)(MT_FE_RS6060_Device_Handle handle, U32 freqKHz, U32 *realfreqKHz);
MT_FE_RET (*mt_fe_tn_sleep)(MT_FE_RS6060_Device_Handle handle);
MT_FE_RET (*mt_fe_tn_wake_up)(MT_FE_RS6060_Device_Handle handle);
MT_FE_RET (*mt_fe_tn_get_gain)(MT_FE_RS6060_Device_Handle handle, S32 *p_gain);



/* function for blind scan*/
MT_FE_RET _mt_fe_dmd_rs6060_bs_set_demod(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type);
MT_FE_RET _mt_fe_dmd_rs6060_bs_remove_unlocked_TP(MT_FE_RS6060_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz);
MT_FE_RET _mt_fe_dmd_rs6060_bs_connect(MT_FE_RS6060_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
									 U16 start_index,
									 U16 scanned_tp);


static U32 mPLSGoldTable[] =
{
	260334,
	248232
};


static MT_FE_PLS_INFO mPLSInfoTable[] = 
{
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x00,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  TRUE,          0},
    {0x01,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   TRUE,          0},
    {0x02,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  TRUE,          1},
    {0x03,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   TRUE,          1},
    {0x04,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       FALSE,  FALSE,         0},
    {0x05,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       TRUE,   FALSE,         0},
    {0x06,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       FALSE,  FALSE,         1},
    {0x07,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_4,       TRUE,   FALSE,         1},
    {0x08,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       FALSE,  FALSE,         0},
    {0x09,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       TRUE,   FALSE,         0},
    {0x0A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       FALSE,  FALSE,         1},
    {0x0B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_3,       TRUE,   FALSE,         1},
    {0x0C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       FALSE,  FALSE,         0},
    {0x0D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       TRUE,   FALSE,         0},
    {0x0E,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       FALSE,  FALSE,         1},
    {0x0F,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_5,       TRUE,   FALSE,         1},
    {0x10,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       FALSE,  FALSE,         0},
    {0x11,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       TRUE,   FALSE,         0},
    {0x12,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       FALSE,  FALSE,         1},
    {0x13,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_1_2,       TRUE,   FALSE,         1},
    {0x14,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x15,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x16,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0x17,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0x18,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x19,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x1A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x1B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x1C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x1D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x1E,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x1F,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x20,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x21,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x22,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x23,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x24,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x25,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x26,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x27,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x28,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x29,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x2A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x2B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x2C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x2D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x2E,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x2F,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Qpsk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x30,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x31,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x32,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0x33,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0x34,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x35,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x36,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x37,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x38,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x39,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x3A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x3B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x3C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x3D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x3E,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x3F,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x40,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x41,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x42,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x43,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x44,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x45,       TRUE,   MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x46,       FALSE,  MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x47,       FALSE,  MtFeType_DvbS2,     MtFeModMode_8psk,       MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x48,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0x49,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0x4A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0x4B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0x4C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x4D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x4E,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x4F,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x50,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x51,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x52,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x53,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x54,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x55,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x56,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x57,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x58,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x59,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x5A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x5B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x5C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x5D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x5E,       FALSE,  MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x5F,       FALSE,  MtFeType_DvbS2,     MtFeModMode_16Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x60,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0x61,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0x62,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       FALSE,  FALSE,         1},
    {0x63,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0x64,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0x65,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0x66,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         1},
    {0x67,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         1},
    {0x68,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0x69,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0x6A,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         1},
    {0x6B,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         1},
    {0x6C,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         0},
    {0x6D,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         0},
    {0x6E,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       FALSE,  FALSE,         1},
    {0x6F,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_8_9,       TRUE,   FALSE,         1},
    {0x70,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         0},
    {0x71,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         0},
    {0x72,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      FALSE,  FALSE,         1},
    {0x73,       TRUE,   MtFeType_DvbS2,     MtFeModMode_32Apsk,     MtFeCodeRate_9_10,      TRUE,   FALSE,         1},
    {0x74,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x75,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x76,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x77,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
    {0x78,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x79,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x7A,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x7B,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
    {0x7C,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x7D,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x7E,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         1},
    {0x7F,       FALSE,  MtFeType_DvbS2,     MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         1},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0x80,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x81,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x82,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0x83,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0x84,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_13_45,     FALSE,  FALSE,         0},
    {0x85,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_13_45,     TRUE,   FALSE,         0},
    {0x86,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_9_20,      FALSE,  FALSE,         0},
    {0x87,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_9_20,      TRUE,   FALSE,         0},
    {0x88,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_20,     FALSE,  FALSE,         0},
    {0x89,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_20,     TRUE,   FALSE,         0},
    {0x8A,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_5_9,       FALSE,  FALSE,         0},
    {0x8B,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_5_9,       TRUE,   FALSE,         0},
    {0x8C,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_26_45,     FALSE,  FALSE,         0},
    {0x8D,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8Apsk_L,    MtFeCodeRate_26_45,     TRUE,   FALSE,         0},
    {0x8E,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_23_36,     FALSE,  FALSE,         0},
    {0x8F,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_23_36,     TRUE,   FALSE,         0},
    {0x90,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_25_36,     FALSE,  FALSE,         0},
    {0x91,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_25_36,     TRUE,   FALSE,         0},
    {0x92,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_13_18,     FALSE,  FALSE,         0},
    {0x93,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_13_18,     TRUE,   FALSE,         0},
    {0x94,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2,       FALSE,  FALSE,         0},
    {0x95,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_1_2,       TRUE,   FALSE,         0},
    {0x96,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,      FALSE,  FALSE,         0},
    {0x97,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_8_15,      TRUE,   FALSE,         0},
    {0x98,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9,       FALSE,  FALSE,         0},
    {0x99,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_5_9,       TRUE,   FALSE,         0},
    {0x9A,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     FALSE,  FALSE,         0},
    {0x9B,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     TRUE,   FALSE,         0},
    {0x9C,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x9D,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
    {0x9E,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5,       FALSE,  FALSE,         0},
    {0x9F,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_3_5,       TRUE,   FALSE,         0},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xA0,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_28_45,     FALSE,  FALSE,         0},
    {0xA1,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_28_45,     TRUE,   FALSE,         0},
    {0xA2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_23_36,     FALSE,  FALSE,         0},
    {0xA3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_23_36,     TRUE,   FALSE,         0},
    {0xA4,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xA5,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk_L,   MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xA6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      FALSE,  FALSE,         0},
    {0xA7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      TRUE,   FALSE,         0},
    {0xA8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_13_18,     FALSE,  FALSE,         0},
    {0xA9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_13_18,     TRUE,   FALSE,         0},
    {0xAA,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xAB,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xAC,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_77_90,     FALSE,  FALSE,         0},
    {0xAD,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_77_90,     TRUE,   FALSE,         0},
    {0xAE,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xAF,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk_L,   MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xB0,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xB1,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xB2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xB3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xB4,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xB5,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xB6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xB7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xB8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xB9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk_L,   MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xBA,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xBB,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xBC,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xBD,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xBE,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xBF,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xC0,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xC1,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xC2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_4_5,       FALSE,  FALSE,         0},
    {0xC3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_4_5,       TRUE,   FALSE,         0},
    {0xC4,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xC5,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xC6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_5_6,       FALSE,  FALSE,         0},
    {0xC7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_64Apsk,     MtFeCodeRate_5_6,       TRUE,   FALSE,         0},
    {0xC8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0xC9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_3_4,       TRUE,   FALSE,         0},
    {0xCA,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_7_9,       FALSE,  FALSE,         0},
    {0xCB,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_128Apsk,    MtFeCodeRate_7_9,       TRUE,   FALSE,         0},
    {0xCC,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,     FALSE,  FALSE,         0},
    {0xCD,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_29_45,     TRUE,   FALSE,         0},
    {0xCE,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3,       FALSE,  FALSE,         0},
    {0xCF,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_2_3,       TRUE,   FALSE,         0},
    {0xD0,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,     FALSE,  FALSE,         0},
    {0xD1,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_31_45,     TRUE,   FALSE,         0},
    {0xD2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_32_45,     FALSE,  FALSE,         0},
    {0xD3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_32_45,     TRUE,   FALSE,         0},
    {0xD4,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,     FALSE,  FALSE,         0},
    {0xD5,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk_L,  MtFeCodeRate_11_15,     TRUE,   FALSE,         0},
    {0xD6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_3_4,       FALSE,  FALSE,         0},
    {0xD7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_256Apsk,    MtFeCodeRate_3_4,       TRUE,   FALSE,         1},
    {0xD8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_45,     FALSE,  FALSE,         1},
    {0xD9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_11_45,     TRUE,   FALSE,         1},
    {0xDA,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_4_15,      FALSE,  FALSE,         1},
    {0xDB,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_4_15,      TRUE,   FALSE,         1},
    {0xDC,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_14_45,     FALSE,  FALSE,         1},
    {0xDD,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_14_45,     TRUE,   FALSE,         1},
    {0xDE,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xDF,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
//   PLS Code,   Valid,  DVB Type,           Mod Mode,               Code Rate,              Pilot,  Dummy Frames,  Frame Length
    {0xE0,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xE1,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xE2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xE3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_Qpsk,       MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xE4,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xE5,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
    {0xE6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xE7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xE8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_26_45,     FALSE,  FALSE,         1},
    {0xE9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_26_45,     TRUE,   FALSE,         1},
    {0xEA,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xEB,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_8psk,       MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xEC,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_15,      FALSE,  FALSE,         1},
    {0xED,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_7_15,      TRUE,   FALSE,         1},
    {0xEE,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      FALSE,  FALSE,         1},
    {0xEF,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_8_15,      TRUE,   FALSE,         1},
    {0xF0,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     FALSE,  FALSE,         1},
    {0xF1,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_26_45,     TRUE,   FALSE,         1},
    {0xF2,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       FALSE,  FALSE,         1},
    {0xF3,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_3_5,       TRUE,   FALSE,         1},
    {0xF4,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xF5,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_16Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xF6,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_2_3,       FALSE,  FALSE,         1},
    {0xF7,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_2_3,       TRUE,   FALSE,         1},
    {0xF8,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     FALSE,  FALSE,         1},
    {0xF9,       TRUE,   MtFeType_DvbS2X,    MtFeModMode_32Apsk,     MtFeCodeRate_32_45,     TRUE,   FALSE,         1},
    {0xFA,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFB,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xFC,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFD,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0},
    {0xFE,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     FALSE,  FALSE,         0},
    {0xFF,       FALSE,  MtFeType_DvbS2X,    MtFeModMode_Undef,      MtFeCodeRate_Undef,     TRUE,   FALSE,         0}
};


void mt_fe_dummy_function(MT_FE_MSG msg, void* p_param)
{
	UNUSED_PARAMETER(msg);
	UNUSED_PARAMETER(p_param);
}

static void (*mt_fe_bs_notify)(MT_FE_MSG msg, void *p_tp_info) = mt_fe_dummy_function;


MT_FE_RET mt_fe_dmd_rs6060_config_default(MT_FE_RS6060_Device_Handle handle)
{
	U8 i;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle->dev_index		 = 0;
	handle->demod_dev_addr	 = MT_FE_DMD_DEV_ADDR_RS6060;
	handle->demod_id		 = MtFeDmdId_RS6060;
	handle->demod_type		 = MT_FE_DEMOD_M88RS6060;
	handle->dtv_mode		 = MtFeType_DvbS2;

	handle->tuner_cfg.tuner_init_OK		 = 0;
	handle->tuner_cfg.tuner_dev_addr	 = MT_FE_TN_I2C_ADDR;

	handle->board_cfg.bIQInverted		 = FALSE;
	handle->board_cfg.bAGCPolar			 = FALSE;
	handle->board_cfg.bSpectrumInverted	 = FALSE;
	handle->board_cfg.bPolarLNBEnable	 = LNB_ENABLE_WHEN_LNB_EN_HIGH;
	handle->board_cfg.bPolarLNB13V		 = LNB_13V_WHEN_VSEL_HIGH;
	handle->board_cfg.bPolarLNBStandby	 = LNB_VSEL_STANDBY_HIGH;
	handle->board_cfg.bPolarDiSEqCOut	 = LNB_DISEQC_OUT_FORCE_HIGH;
	handle->board_cfg.bDiSEqCOutputOnly	 = LNB_DISEQC_OUT_ONLY_OUTPUT;

	handle->ts_cfg.mTsMode				 = MtFeTsOutMode_Serial;
	handle->ts_cfg.bHighZ				 = FALSE;
	handle->ts_cfg.bPinSwitch			 = MT_FE_TS_PIN_ORDER_D0_D7;
	handle->ts_cfg.mPinDriver			 = MtFeTsDriverAbility_4mA;
	handle->ts_cfg.mMaxClock			 = MtFeTSOut_Max_Clock_Auto;
	handle->ts_cfg.iCustomDutyHigh		 = 0;
	handle->ts_cfg.iCustomDutyLow		 = 0;
	
	handle->ts_cfg.iSerialDriverMode	 = 1;	/* 0 - RS6060 only; 1 - RS6060 + other demodulators */
	
	handle->ts_cfg.bAutoSerial			 = MT_FE_TS_CLOCK_AUTO_SET_FOR_SERIAL_MODE;
	handle->ts_cfg.bAutoParallel		 = MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE;
	handle->ts_cfg.bAutoCI				 = MT_FE_TS_CLOCK_AUTO_SET_FOR_CI_MODE;
	handle->ts_cfg.bRisingEdge			 = FALSE;
	handle->ts_cfg.bHighSync			 = FALSE;
	handle->ts_cfg.bHighValid			 = FALSE;
	handle->ts_cfg.bHighError			 = FALSE;
	handle->ts_cfg.b3SerialTsOut		 = FALSE;
	handle->ts_cfg.bParallelClkFixed	 = FALSE;

	handle->lnb_cfg.bLnbOn				 = TRUE;
	handle->lnb_cfg.b22K				 = FALSE;
	handle->lnb_cfg.mLnbVoltage			 = MtFeLNB_13V;

	handle->lnb_cfg.bToneBurst			 = FALSE;
	handle->lnb_cfg.mToneBurst			 = MtFeDiSEqCToneBurst_Moulated;

	handle->lnb_cfg.bDiSEqCSend			 = FALSE;
	handle->lnb_cfg.iSizeSend			 = 0;
	for (i = 0; i < 8; i++)
	{
		handle->lnb_cfg.iDataSend[i] = 0;
	}

	handle->lnb_cfg.bDiSEqCReceive		 = FALSE;
	handle->lnb_cfg.iSizeReceive		 = 0;
	for (i = 0; i < 8; i++)
	{
		handle->lnb_cfg.iDataReceive[i] = 0;
	}

	handle->lnb_cfg.bEnvelopMode		 = FALSE;

	handle->lnb_cfg.bUnicable			 = FALSE;
	handle->lnb_cfg.iBankIndex			 = 0;
	handle->lnb_cfg.iUBIndex			 = 0;
	handle->lnb_cfg.iUBFreqMHz			 = 1210;

	handle->lnb_cfg.iUBCount			 = 0;
	for (i = 0; i < 32; i++)
	{
		handle->lnb_cfg.iUBList[i]		 = 0;
	}

#if 1	// Inverto IDLU-32UL40-UNBOO-OPP
	handle->lnb_cfg.iUBList[0]			 = 1210;
	handle->lnb_cfg.iUBList[1]			 = 1420;
	handle->lnb_cfg.iUBList[2]			 = 1680;
	handle->lnb_cfg.iUBList[3]			 = 2040;
	handle->lnb_cfg.iUBList[4]			 = 984;
	handle->lnb_cfg.iUBList[5]			 = 1020;
	handle->lnb_cfg.iUBList[6]			 = 1056;
	handle->lnb_cfg.iUBList[7]			 = 1092;
	handle->lnb_cfg.iUBList[8]			 = 1128;
	handle->lnb_cfg.iUBList[9]			 = 1164;
	handle->lnb_cfg.iUBList[10]			 = 1256;
	handle->lnb_cfg.iUBList[11]			 = 1292;
	handle->lnb_cfg.iUBList[12]			 = 1328;
	handle->lnb_cfg.iUBList[13]			 = 1364;
	handle->lnb_cfg.iUBList[14]			 = 1458;
	handle->lnb_cfg.iUBList[15]			 = 1494;
	handle->lnb_cfg.iUBList[16]			 = 1530;
	handle->lnb_cfg.iUBList[17]			 = 1566;
	handle->lnb_cfg.iUBList[18]			 = 1602;
	handle->lnb_cfg.iUBList[19]			 = 1638;
	handle->lnb_cfg.iUBList[20]			 = 1716;
	handle->lnb_cfg.iUBList[21]			 = 1752;
	handle->lnb_cfg.iUBList[22]			 = 1788;
	handle->lnb_cfg.iUBList[23]			 = 1824;
	handle->lnb_cfg.iUBList[24]			 = 1860;
	handle->lnb_cfg.iUBList[25]			 = 1896;
	handle->lnb_cfg.iUBList[26]			 = 1932;
	handle->lnb_cfg.iUBList[27]			 = 1968;
	handle->lnb_cfg.iUBList[28]			 = 2004;
	handle->lnb_cfg.iUBList[29]			 = 2076;
	handle->lnb_cfg.iUBList[30]			 = 2112;
	handle->lnb_cfg.iUBList[31]			 = 2148;
#else
	handle->lnb_cfg.iUBList[0]			 = 1210;
	handle->lnb_cfg.iUBList[1]			 = 1420;
	handle->lnb_cfg.iUBList[2]			 = 1680;
	handle->lnb_cfg.iUBList[3]			 = 2040,
	handle->lnb_cfg.iUBList[4]			 = 1005;
	handle->lnb_cfg.iUBList[5]			 = 1050;
	handle->lnb_cfg.iUBList[6]			 = 1095;
	handle->lnb_cfg.iUBList[7]			 = 1140;
	handle->lnb_cfg.iUBList[8]			 = 1260;
	handle->lnb_cfg.iUBList[9]			 = 1305;
	handle->lnb_cfg.iUBList[10]			 = 1350;
	handle->lnb_cfg.iUBList[11]			 = 1475;
	handle->lnb_cfg.iUBList[12]			 = 1520;
	handle->lnb_cfg.iUBList[13]			 = 1565;
	handle->lnb_cfg.iUBList[14]			 = 1610;
	handle->lnb_cfg.iUBList[15]			 = 1725;
	handle->lnb_cfg.iUBList[16]			 = 1770;
	handle->lnb_cfg.iUBList[17]			 = 1815;
	handle->lnb_cfg.iUBList[18]			 = 1860;
	handle->lnb_cfg.iUBList[19]			 = 1905;
	handle->lnb_cfg.iUBList[20]			 = 1950;
	handle->lnb_cfg.iUBList[21]			 = 1995;
	handle->lnb_cfg.iUBList[22]			 = 2085;
	handle->lnb_cfg.iUBList[23]			 = 2130;
#endif

	handle->lnb_cfg.iUBVer				 = 1;		/* 2: Unicable II; 1 or others: Unicable I */

	handle->global_cfg.iMclkKHz			 = 96000;
	handle->global_cfg.iSerialMclkHz	 = 96000;

	handle->global_cfg.bLastTpAFlag		 = FALSE;
	handle->global_cfg.iTpIndex			 = 0;
	handle->global_cfg.iLockedTpCnt		 = 0;
	handle->global_cfg.iScannedTpCnt	 = 0;
	handle->global_cfg.iCurCompareTpAKHz = 0;
	handle->global_cfg.bCancelBs		 = FALSE;

	handle->tp_cfg.mConnectType			 = MtFeType_DvbS;
	handle->tp_cfg.mCurrentType			 = MtFeType_DvbS;
	handle->tp_cfg.iFreqKHz				 = 0;
	handle->tp_cfg.iSymRateKSs			 = 0;
	handle->tp_cfg.mCodeRate			 = MtFeCodeRate_Undef;
	handle->tp_cfg.iCarrierOffsetKHz	 = 0;
	handle->tp_cfg.bLimitCarrierOffset	 = MT_FE_CHECK_CARRIER_OFFSET;
	handle->tp_cfg.iOffsetRangeKHz		 = MT_FE_CARRIER_OFFSET_KHz;
	handle->tp_cfg.bCheckCCM			 = FALSE;
	handle->tp_cfg.iTsCnt				 = 0;
	for (i = 0; i < 32; i++)
	{
		handle->tp_cfg.ucTsId[i]		 = 0;
	}
	handle->tp_cfg.ucCurTsId			 = 0;
	handle->tp_cfg.bHavePLS				 = FALSE;
	for (i = 0; i < 3; i++)
	{
		handle->tp_cfg.ucPLSCode[i]		 = 0;
	}

	handle->version_number				 = 0x00020211;
	handle->version_time				 = 0x22102016;

	handle->dmd_set_reg	 = (MT_FE_RET (*)(void *, U8, U8))_mt_fe_dmd_set_reg_rs6060;
	handle->dmd_get_reg	 = (MT_FE_RET (*)(void *, U8, U8*))_mt_fe_dmd_get_reg_rs6060;
	handle->write_fw	 = (MT_FE_RET (*)(void *, U8, U8*, U16))_mt_fe_write_fw_rs6060;
	handle->dmd_read	 = NULL;
	handle->mt_sleep	 = _mt_sleep_rs6060;
	handle->tn_set_reg	 = (MT_FE_RET (*)(void *, U8, U8))_mt_fe_tn_set_reg_rs6060;
	handle->tn_get_reg	 = (MT_FE_RET (*)(void *, U8, U8*))_mt_fe_tn_get_reg_rs6060;

	return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_rs6060_get_driver_version
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
**	 p_version			U32*		version number pointer
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_get_driver_version(MT_FE_RS6060_Device_Handle handle, U32* p_version_number, U32 *p_version_time)
{
	*p_version_number	 = handle->version_number;		/* driver version number */
	*p_version_time		 = handle->version_time;		/* driver version time */

	return MtFeErr_Ok;
}


/************************************************************************
** Function: mt_fe_dmd_rs6060_soft_reset
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
MT_FE_RET mt_fe_dmd_rs6060_soft_reset(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	handle->dmd_set_reg(handle, 0xb2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);
	handle->mt_sleep(1);
	handle->dmd_set_reg(handle, 0x00, 0x00);
	handle->dmd_set_reg(handle, 0xb2, 0x00);

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_rs6060_global_reset(MT_FE_RS6060_Device_Handle handle)
{
	handle->dmd_set_reg(handle, 0x07, 0x80);
	handle->dmd_set_reg(handle, 0x07, 0x00);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_rs6060_get_demod_id(MT_FE_RS6060_Device_Handle handle, MT_FE_DMD_ID *dmd_id)
{
	U8 val_00 = 0, val_01 = 0, val_02 = 0, val_b2 = 0;

	*dmd_id = MtFeDmdId_Undef;

	handle->dmd_get_reg(handle, 0x01, &val_01);
	handle->dmd_get_reg(handle, 0x02, &val_02);
	handle->dmd_get_reg(handle, 0xb2, &val_b2);
	handle->dmd_get_reg(handle, 0x00, &val_00);

	val_00 &= 0xFE;

	if ((val_02 == 0x00) && ((val_01 == 0xA0) || (val_01 == 0xA1)) && ((val_b2 & 0xC0) == 0xC0) && 
			(val_00 == 0xE8))
	{
		*dmd_id = MtFeDmdId_RS6000;
		mt_fe_print(("\tChip ID = [RS6000]!\n"));
	}
	//else if ((val_02 == 0x00) && (val_00 == 0xE2))
	else if (((val_01 == 0xA0) || (val_01 == 0xA1)) && (val_00 == 0xE2))
	{
		*dmd_id = MtFeDmdId_RS6060;
		mt_fe_print(("\tChip ID = [RS6060]!\n"));
	}
	else
	{
		*dmd_id = MtFeDmdId_Unknown;
	}

#if 1
	handle->dmd_get_reg(handle, 0x0b, &val_00);

	switch (val_00 & 0x90)
	{
		case 0x90:		handle->tuner_cfg.tuner_dev_addr = 0x5E;		break;
		case 0x80:		handle->tuner_cfg.tuner_dev_addr = 0x5C;		break;
		case 0x10:		handle->tuner_cfg.tuner_dev_addr = 0x5A;		break;
		case 0x00:
		default:		handle->tuner_cfg.tuner_dev_addr = 0x58;		break;
	}
#endif

	return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_rs6060_hard_reset
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
MT_FE_RET mt_fe_dmd_rs6060_hard_reset(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	U8 val_regb2;

	/*
		TODO:
			 do the hardware reset by the GPIO pin.
	*/

#if 1
	handle->dmd_get_reg(handle, 0x0b, &val);
	val &= ~0x90;
	switch (handle->tuner_cfg.tuner_dev_addr)
	{
		case 0x5E:		val |= 0x90;	break;
		case 0x5C:		val |= 0x80;	break;
		case 0x5A:		val |= 0x10;	break;
		case 0x58:
		default:		break;
	}
	handle->dmd_set_reg(handle, 0x0b, val);
#endif

	mt_fe_tn_reset_rs6060(handle);

	mt_fe_dmd_rs6060_wake_up(handle);

	mt_fe_dmd_rs6060_get_demod_id(handle, &(handle->demod_id));

	if (handle->tp_cfg.bLimitCarrierOffset)
	{
		handle->dmd_get_reg(handle, 0xb2, &val_regb2);
		if (val_regb2 == 0x01)
		{
			handle->dmd_set_reg(handle, 0x00, 0x00);
			handle->dmd_set_reg(handle, 0xb2, 0x00);
		}
	}

	mt_fe_dmd_rs6060_global_reset(handle);

	handle->mt_sleep(1);

	handle->dmd_get_reg(handle, 0x08, &val);
	val |= 0x01;
	handle->dmd_set_reg(handle, 0x08, val);


	/* reset global defines*/
	handle->tp_cfg.mCurrentType			 = MtFeType_DvbS;
	handle->tp_cfg.mConnectType			 = MtFeType_DvbS;
	handle->tp_cfg.iFreqKHz				 = 0;
	handle->tp_cfg.iSymRateKSs			 = 0;

	if (((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
		((handle->ts_cfg.mTsMode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
		((handle->ts_cfg.mTsMode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
	{
		handle->global_cfg.bTsClockChecked = FALSE;
	}

	handle->mt_sleep(1);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_rs6060_set_high_Z_state(MT_FE_RS6060_Device_Handle handle, MT_BOOL high_Z_output)
{
	U8 val = 0;

	handle->dmd_get_reg(handle, 0x0b, &val);

	if (high_Z_output == FALSE)		// TS output normal state
	{
		val |= 0x01;
	}
	else							// TS output high Z state
	{
		val &= ~0x01;
	}

	handle->dmd_set_reg(handle, 0x0b, val);

	return MtFeErr_Ok;
}


/******************************************************************************
** Function: mt_fe_dmd_rs6060_init
**
**
** Description:  This function is used to initial RS6060.
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
MT_FE_RET mt_fe_dmd_rs6060_init(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	mt_fe_dmd_rs6060_hard_reset(handle);

	mt_fe_tn_set_tuner	 = mt_fe_tn_set_freq_rs6060;
	mt_fe_tn_get_offset	 = mt_fe_tn_get_tuner_freq_offset_rs6060;
	mt_fe_tn_sleep		 = mt_fe_tn_sleep_rs6060;
	mt_fe_tn_wake_up	 = mt_fe_tn_wakeup_rs6060;

	mt_fe_tn_get_gain	 = mt_fe_tn_get_gain_rs6060;


	mt_fe_tn_init_rs6060(handle);

	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
	handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2));		// XOR

	handle->dmd_set_reg(handle, 0x07, 0xE0);		// global reset, global DiSEqC reset, global FEC reset
	handle->dmd_set_reg(handle, 0x07, 0x00);

	_mt_fe_dmd_rs6060_download_fw(handle, rs6060_fm, sizeof(rs6060_fm) / sizeof(U8));

	handle->mt_sleep(1);

	handle->dmd_get_reg(handle, 0xb9, &val);
	if (val != rs6060_fw_ver)
	{
		mt_fe_print(("\tDownload RS6060 firmware failed! Version no = 0x%02X!\n", val));
	}

	_mt_fe_dmd_rs6060_set_ts_out_mode(handle, handle->ts_cfg.mTsMode);


	if (handle->board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val |= 0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}
	else
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val &= ~0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_rs6060_change_ts_out_mode
**
**
** Description:  This function is used to change the TS output mode.
**
**
** Inputs:
**	Parameter				Type		Description
**	----------------------------------------------------------
**	mode					U8			passed by up level
**
**	MtFeTsOutMode_Serial	1			Serial
**	MtFeTsOutMode_Parallel	2			Parallel
**	MtFeTsOutMode_Common	3			Common Interface
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_change_ts_out_mode(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE mode)
{
	U8 tmp, val;

	MT_FE_LOCK_STATE lockstate = MtFeLockState_Unlocked;

	//U32 locked_sym_rate;

	U32 target_mclk = MT_FE_MCLK_KHZ, cur_mclk = MT_FE_MCLK_KHZ;

	handle->dmd_get_reg(handle, 0xfd, &tmp);
	if (mode == MtFeTsOutMode_Parallel)
	{
		tmp &= ~0x01;
		tmp &= ~0x04;

		handle->dmd_set_reg(handle, 0xfa, 0x01);
		handle->dmd_set_reg(handle, 0xf1, 0x60);
		handle->dmd_set_reg(handle, 0xfa, 0x00);
	}
	else if (mode == MtFeTsOutMode_Serial)
	{
		tmp &= ~0x01;
		tmp |= 0x04;
	}
	else
	{
		tmp |= 0x01;
		tmp &= ~0x04;


		handle->dmd_set_reg(handle, 0xfa, 0x01);
		handle->dmd_set_reg(handle, 0xf1, 0x60);
		handle->dmd_set_reg(handle, 0xfa, 0x00);
	}
	tmp |= 0x80;
	handle->dmd_set_reg(handle, 0xfd, tmp);


	//mt_fe_dmd_rs6060_get_lock_state(&lockstate);
	if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2)
	{
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0x8f) == 0x8f)
		{
			lockstate = MtFeLockState_Locked;
		}
	}
	else	// DVB-S
	{
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7)
		{
			lockstate = MtFeLockState_Locked;
		}
	}

	_mt_fe_dmd_rs6060_get_ts_mclk(handle, &cur_mclk);

	target_mclk = 144000;

	if (cur_mclk != target_mclk)
	{
		handle->dmd_set_reg(handle, 0x06, 0xe0);
		_mt_fe_dmd_rs6060_set_ts_mclk(handle, target_mclk);
		handle->dmd_set_reg(handle, 0x06, 0x00);
	}

	if (mode == MtFeTsOutMode_Serial)
	{
		if (handle->ts_cfg.bAutoSerial != 0)
		{
			if (lockstate == MtFeLockState_Locked)
			{
				_mt_fe_dmd_rs6060_set_clock_ratio(handle);
			}
		}
	}

	#if 1
	if (mode != MtFeTsOutMode_Serial)
	{
		val = 0;
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0C;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xC0;

		handle->dmd_set_reg(handle, 0x0a, val);
	}
	#else
	val = 0;
	if (mode != MtFeTsOutMode_Serial)
	{
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0C;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xC0;
	}
	else
	{
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_SERIAL;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0C;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xC0;
	}
	handle->dmd_set_reg(handle, 0x0a, val);
	#endif

	handle->dmd_get_reg(handle, 0x0b, &tmp);
	if (handle->ts_cfg.bPinSwitch)
	{
		tmp |= 0x20;
	}
	else
	{
		tmp &= ~0x20;
	}
	handle->dmd_set_reg(handle, 0x0b, tmp);


	handle->dmd_get_reg(handle, 0x0c, &tmp);
	if ((mode == MtFeTsOutMode_Serial) && handle->ts_cfg.b3SerialTsOut)
	{
		handle->dmd_set_reg(handle, 0xf4, 0x03);
		tmp |= 0x80;
	}
	else
	{
		handle->dmd_set_reg(handle, 0xf4, 0x01);
		tmp &= ~0x80;
	}
	handle->dmd_set_reg(handle, 0x0c, tmp);


	handle->ts_cfg.mTsMode = mode;


	return MtFeErr_Ok;
}


/************************************************************************
** Function: mt_fe_dmd_rs6060_sleep
**
**
** Description:	This function is used to sleep RS6060.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_sleep(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	mt_fe_dmd_rs6060_set_high_Z_state(handle, TRUE);

	handle->dmd_get_reg(handle, 0x08, &val);
	val &= ~0x01;
	handle->dmd_set_reg(handle, 0x08, val);

	if (mt_fe_tn_sleep != NULL)
		mt_fe_tn_sleep(handle);

	handle->dmd_get_reg(handle, 0x04, &val);
	val |= 0x01;
	handle->dmd_set_reg(handle, 0x04, val);

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}


/************************************************************************
** Function: mt_fe_dmd_rs6060_wake_up
**
**
** Description:	This function is used to wake up RS6060.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_wake_up(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	handle->dmd_get_reg(handle, 0x04, &val);
	val &= ~0x01;
	handle->dmd_set_reg(handle, 0x04, val);

	if (mt_fe_tn_wake_up != NULL)
		mt_fe_tn_wake_up(handle);

	handle->dmd_get_reg(handle, 0x08, &val);
	val |= 0x01;
	handle->dmd_set_reg(handle, 0x08, val);

	mt_fe_dmd_rs6060_set_high_Z_state(handle, FALSE);

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}


/**********************************************************************************************
** Function: mt_fe_dmd_rs6060_connect
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
**		"MtFeType_DvbS" or "MtFeType_DvbS2" or "MtFeType_DTV_Unknown".
**
**		If the type is 	"MtFeType_DTV_Unknown", driver will try DVBS and DVBS2
**  automatically.
**
**
**
** Outputs:  None
**
**********************************************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_connect(MT_FE_RS6060_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type)
{
	S16	lpf_offset_KHz;
	U32	freq_tmp_KHz;

	U32	real_freq_KHz;
	S32	deviceoffset_KHz;

	MT_FE_RET ret = MtFeErr_Ok;

	int i;

	MT_FE_TS_OUT_MODE ts_mode = MtFeTsOutMode_Common;

	U32 target_mclk = 144000;//192000;


	U8  val_regb2;

	if ((mt_fe_tn_set_tuner == NULL) || (mt_fe_tn_get_offset == NULL))
		return MtFeErr_NullPointer;

	mt_fe_dmd_rs6060_global_reset(handle);

	mt_fe_dmd_rs6060_clear_ts(handle);

	handle->tp_cfg.bCheckCCM = FALSE;
	handle->tp_cfg.iTsCnt = 0;

	for (i = 0; i < 32; i++)
	{
		handle->tp_cfg.ucTsId[i] = 0;
	}

	handle->tp_cfg.mConnectType			 = dvbs_type;
	handle->tp_cfg.iSymRateKSs			 = sym_rate_KSs;

	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
	handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2));		// XOR

	_mt_fe_dmd_rs6060_get_current_ts_mode(handle, &ts_mode);

	if (((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
		((ts_mode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
		((ts_mode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
	{
		handle->global_cfg.bTsClockChecked	 = TRUE;
	}

	if (handle->tp_cfg.bLimitCarrierOffset)
	{
		handle->dmd_get_reg(handle, 0xb2, &val_regb2);
		if (val_regb2 == 0x01)
		{
			handle->dmd_set_reg(handle, 0x00, 0x00);
			handle->dmd_set_reg(handle, 0xb2, 0x00);
		}
	}

	freq_tmp_KHz	 = freq_MHz * 1000;
	lpf_offset_KHz	 = 0;

	if ((sym_rate_KSs < 5000) && (handle->lnb_cfg.bUnicable == 0))
	{
		lpf_offset_KHz = FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
		freq_tmp_KHz  += FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
	}

	handle->tp_cfg.iFreqKHz = freq_tmp_KHz;

	handle->dmd_set_reg(handle, 0x06, 0xe0);
	_mt_fe_dmd_rs6060_select_mclk(handle, handle->tp_cfg.iFreqKHz / 1000);
	_mt_fe_dmd_rs6060_set_ts_mclk(handle, target_mclk);
	handle->dmd_set_reg(handle, 0x06, 0x00);

	handle->mt_sleep(10);

	/*set tuner*/
	if (handle->lnb_cfg.bUnicable == 1)
	{
		mt_fe_dmd_rs6060_unicable_set_tuner(handle, freq_tmp_KHz, sym_rate_KSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
		deviceoffset_KHz = real_freq_KHz - freq_tmp_KHz;
	}
	else
	{
		mt_fe_tn_set_tuner(handle, freq_tmp_KHz, sym_rate_KSs, lpf_offset_KHz);
		real_freq_KHz = freq_tmp_KHz;
		deviceoffset_KHz = 0;
	}

	handle->dmd_set_reg(handle, 0xb2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);

	/*set demod registers*/
	_mt_fe_dmd_rs6060_set_demod(handle, sym_rate_KSs, handle->tp_cfg.mCurrentType);

	handle->dmd_get_reg(handle, 0x08, &val_regb2);
	if (dvbs_type == MtFeType_DvbS)
	{
		val_regb2 = (U8)((val_regb2 & 0xfb) | 0x40);
		handle->dmd_set_reg(handle, 0x08, val_regb2);
		handle->dmd_set_reg(handle, 0xe0, 0xf8);
	}
	else if (dvbs_type == MtFeType_DvbS2)
	{
		val_regb2 = (U8)(val_regb2 | 0x44);
		handle->dmd_set_reg(handle, 0x08, val_regb2);
	}
	else	// Unknown
	{
		val_regb2 = (U8)(val_regb2 & 0xbb);
		handle->dmd_set_reg(handle, 0x08, val_regb2);
		handle->dmd_set_reg(handle, 0xe0, 0xf8);
	}

	/*set carrier offset*/
	_mt_fe_dmd_rs6060_set_carrier_offset(handle, mt_fe_tn_get_offset(handle) + lpf_offset_KHz + deviceoffset_KHz);


	handle->dmd_set_reg(handle, 0x00, 0x00);
	handle->dmd_set_reg(handle, 0xb2, 0x00);

	mt_fe_tn_adjust_AGC_rs6060(handle);

	handle->dmd_get_reg(handle, 0x00, &val_regb2);

	return ret;
}


#if 0
void mt_fe_dmd_rs6060_calc_PLS_gold_code(U8 *pNormalCode, U32 PLSGoldCode)
{
	typedef struct _PLS_Table_t
	{
		U32	iIndex;
		U8	PLSCode[3];
	}PLS_Table_t;

	static PLS_Table_t PLS_List[] =
	{
		{     0, 	{0x01, 	0x00, 	0x00}},
		{  5000, 	{0x0d, 	0xe0, 	0x00}},
		{ 10000, 	{0x51, 	0x15, 	0x00}},
		{ 15000, 	{0xcf, 	0xc9, 	0x00}},
		{ 20000, 	{0x67, 	0x33, 	0x03}},
		{ 25000, 	{0x02, 	0xc9, 	0x02}},
		{ 30000, 	{0xe5, 	0xc6, 	0x01}},
		{ 35000, 	{0xdb, 	0xc0, 	0x03}},
		{ 40000, 	{0x7c, 	0x5f, 	0x02}},
		{ 45000, 	{0x8d, 	0x65, 	0x00}},
		{ 50000, 	{0x14, 	0x96, 	0x00}},
		{ 55000, 	{0xf7, 	0x61, 	0x03}},
		{ 60000, 	{0xbc, 	0x28, 	0x00}},
		{ 65000, 	{0x77, 	0xa9, 	0x01}},
		{ 70000, 	{0xe7, 	0x05, 	0x01}},
		{ 75000, 	{0x88, 	0x85, 	0x01}},
		{ 80000, 	{0x2f, 	0xbb, 	0x02}},
		{ 85000, 	{0xe1, 	0x07, 	0x00}},
		{ 90000, 	{0xd5, 	0x67, 	0x01}},
		{ 95000, 	{0x94, 	0x37, 	0x03}},
		{100000, 	{0x57, 	0x39, 	0x02}},
		{105000, 	{0xc7, 	0x03, 	0x00}},
		{110000, 	{0xbf, 	0x12, 	0x00}},
		{115000, 	{0x50, 	0x0e, 	0x00}},
		{120000, 	{0xca, 	0xc4, 	0x00}},
		{125000, 	{0x46, 	0xc3, 	0x00}},
		{130000, 	{0x2f, 	0xc6, 	0x01}},
		{135000, 	{0x7c, 	0xe5, 	0x01}},
		{140000, 	{0xb9, 	0x36, 	0x01}},
		{145000, 	{0x9d, 	0xe5, 	0x01}},
		{150000, 	{0xc4, 	0x32, 	0x01}},
		{155000, 	{0x13, 	0xb3, 	0x00}},
		{160000, 	{0x0c, 	0x9f, 	0x02}},
		{165000, 	{0xb2, 	0xb5, 	0x03}},
		{170000, 	{0xac, 	0x7e, 	0x01}},
		{175000, 	{0xb6, 	0xa2, 	0x01}},
		{180000, 	{0xb6, 	0x3e, 	0x01}},
		{185000, 	{0x17, 	0x2c, 	0x02}},
		{190000, 	{0xd7, 	0x2a, 	0x02}},
		{195000, 	{0x93, 	0x61, 	0x02}},
		{200000, 	{0x67, 	0x92, 	0x02}},
		{205000, 	{0x38, 	0x07, 	0x01}},
		{210000, 	{0xb4, 	0x5a, 	0x01}},
		{215000, 	{0xed, 	0x31, 	0x02}},
		{220000, 	{0x9e, 	0x4d, 	0x02}},
		{225000, 	{0x17, 	0x08, 	0x02}},
		{230000, 	{0x37, 	0xb9, 	0x00}},
		{235000, 	{0x2c, 	0xed, 	0x00}},
		{240000, 	{0xe0, 	0x64, 	0x00}},
		{245000, 	{0x90, 	0x39, 	0x01}},
		{250000, 	{0x35, 	0x0e, 	0x01}},
		{255000, 	{0x1c, 	0x9e, 	0x02}},
		{260000, 	{0x58, 	0x78, 	0x00}}
	};

	unsigned char x0;
	unsigned char x1;
	unsigned char x2;
	unsigned char x3;
	unsigned char x4;
	unsigned char x5;
	unsigned char x6;
	unsigned char x7;
	unsigned char x8;
	unsigned char x9;
	unsigned char x10;
	unsigned char x11;
	unsigned char x12;
	unsigned char x13;
	unsigned char x14;
	unsigned char x15;
	unsigned char x16;
	unsigned char x17;
	U32 i;
	unsigned char tmp;

	U32 ulPLSCode = 0, ulPLSIndex = 0;
	int iPLSCnt = sizeof(PLS_List) / sizeof(PLS_Table_t), iPLSListIndex;
	U8 iPLSCode[3];

	ulPLSCode = PLSGoldCode;

	iPLSListIndex = ulPLSCode / 5000;

	if(iPLSListIndex > iPLSCnt - 1)
		iPLSListIndex = iPLSCnt - 1;

	ulPLSIndex = PLS_List[iPLSListIndex].iIndex;
	iPLSCode[0] = PLS_List[iPLSListIndex].PLSCode[0];
	iPLSCode[1] = PLS_List[iPLSListIndex].PLSCode[1];
	iPLSCode[2] = PLS_List[iPLSListIndex].PLSCode[2];

	x0  = (iPLSCode[0] >> 0) & 0x01;
	x1  = (iPLSCode[0] >> 1) & 0x01;
	x2  = (iPLSCode[0] >> 2) & 0x01;
	x3  = (iPLSCode[0] >> 3) & 0x01;
	x4  = (iPLSCode[0] >> 4) & 0x01;
	x5  = (iPLSCode[0] >> 5) & 0x01;
	x6  = (iPLSCode[0] >> 6) & 0x01;
	x7  = (iPLSCode[0] >> 7) & 0x01;
	x8  = (iPLSCode[1] >> 0) & 0x01;
	x9  = (iPLSCode[1] >> 1) & 0x01;
	x10 = (iPLSCode[1] >> 2) & 0x01;
	x11 = (iPLSCode[1] >> 3) & 0x01;
	x12 = (iPLSCode[1] >> 4) & 0x01;
	x13 = (iPLSCode[1] >> 5) & 0x01;
	x14 = (iPLSCode[1] >> 6) & 0x01;
	x15 = (iPLSCode[1] >> 7) & 0x01;
	x16 = (iPLSCode[2] >> 0) & 0x01;
	x17 = (iPLSCode[2] >> 1) & 0x01;

	for(i = ulPLSIndex; i <= ulPLSCode; i++)
	{
		iPLSCode[0] = (x7  << 7) + ( x6 << 6) + (x5  << 5) + (x4  << 4) + (x3  << 3) + (x2  << 2) + (x1 << 1) + (x0 << 0);
		iPLSCode[1] = (x15 << 7) + (x14 << 6) + (x13 << 5) + (x12 << 4) + (x11 << 3) + (x10 << 2) + (x9 << 1) + (x8 << 0);
		iPLSCode[2] = (x17 << 1) +  x16;

		tmp	 = (x0 ^ x7) & 0x01;
		x0	 = x1;
		x1	 = x2;
		x2	 = x3;
		x3	 = x4;
		x4	 = x5;
		x5	 = x6;
		x6	 = x7;
		x7	 = x8;
		x8	 = x9;
		x9	 = x10;
		x10	 = x11;
		x11	 = x12;
		x12	 = x13;
		x13	 = x14;
		x14	 = x15;
		x15	 = x16;
		x16	 = x17;
		x17	 = tmp;
	}

	pNormalCode[0] = iPLSCode[0];
	pNormalCode[1] = iPLSCode[1];
	pNormalCode[2] = iPLSCode[2];

	return;
}
#endif


/******************************************************************
** Function: mt_fe_dmd_rs6060_get_lock_state
**
**
** Description:	This function is called by app level to get the
**			lock status of RS6060.
**
**
** Inputs:   None
**
**
** Outputs:
**
**		Parameter	Type				Description
**		-----------------------------------------------------------
**		p_state		MT_FE_LOCK_STATE*	lock status of RS6060
**
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_get_lock_state(MT_FE_RS6060_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	tmp, ucTmp, tmp1;

	MT_FE_RET ret = MtFeErr_Ok;

	int iCnt;

	U8 val_regb2;
	S32 carrier_offset_KHz;

	if (p_state == 0)
	{
		ret = MtFeErr_Param;

		return ret;
	}

	*p_state = MtFeLockState_Waiting;

	if (handle->tp_cfg.bLimitCarrierOffset)
	{
		handle->dmd_get_reg(handle, 0xb2, &val_regb2);
		if (val_regb2 == 0x01)
		{
			*p_state = MtFeLockState_Unlocked;

			return MtFeErr_Ok;
		}
	}

	handle->dmd_get_reg(handle, 0xbe, &tmp);
	if (tmp == 0xfe)
	{
		handle->dmd_set_reg(handle, 0xae, 0x4f);
		handle->dmd_set_reg(handle, 0x7f, 0xc4);
		handle->dmd_set_reg(handle, 0x24, 0xc0);

		handle->dmd_get_reg(handle, 0xca, &ucTmp);
		ucTmp |= 0x04;
		handle->dmd_set_reg(handle, 0xca, ucTmp);

		mt_fe_dmd_rs6060_soft_reset(handle);

		iCnt = 0;

		do
		{
			iCnt++;
			if (iCnt > 1000)
			{
				handle->dmd_set_reg(handle, 0x22, 0x01);
				handle->dmd_set_reg(handle, 0x23, 0x00);
				handle->dmd_set_reg(handle, 0x24, 0x00);

				mt_fe_dmd_rs6060_soft_reset(handle);

				handle->dmd_set_reg(handle, 0xae, 0x09);
				handle->dmd_set_reg(handle, 0x7f, 0x04);

				handle->dmd_get_reg(handle, 0xca, &ucTmp);
				ucTmp &= ~0x04;
				handle->dmd_set_reg(handle, 0xca, ucTmp);

				handle->dmd_get_reg(handle, 0x00, &tmp);

				return MtFeErr_TimeOut;
			}

			handle->dmd_get_reg(handle, 0x24, &tmp1);

			handle->mt_sleep(1);
		} while ((tmp1 & 0x08) == 0x00);

		handle->dmd_set_reg(handle, 0xae, 0x09);
		handle->dmd_set_reg(handle, 0x7f, 0x04);

		handle->dmd_get_reg(handle, 0xca, &ucTmp);
		ucTmp &= ~0x04;
		handle->dmd_set_reg(handle, 0xca, ucTmp);

		handle->dmd_get_reg(handle, 0x22, &tmp1);
		handle->dmd_set_reg(handle, 0x22, tmp1);
		handle->tp_cfg.ucPLSCode[0] = tmp1;

		handle->dmd_get_reg(handle, 0x23, &tmp1);
		handle->dmd_set_reg(handle, 0x23, tmp1);
		handle->tp_cfg.ucPLSCode[1] = tmp1;

		handle->dmd_get_reg(handle, 0x24, &tmp1);
		tmp1 &= 0x03;
		handle->dmd_set_reg(handle, 0x24, tmp1);
		handle->tp_cfg.ucPLSCode[2] = tmp1;

		handle->tp_cfg.bHavePLS = TRUE;

		mt_fe_dmd_rs6060_soft_reset(handle);
	}

	handle->dmd_get_reg(handle, 0x08, &tmp);
	if (tmp & 0x08)
	{
		handle->dmd_get_reg(handle, 0x7e, &ucTmp);

		if (handle->tp_cfg.bSuperSearch && ((ucTmp & 0x20) != 0x20))
		{
			//printk("%s[%d] ---- Start super search!\n", __FUNCTION__, __LINE__);

			handle->dmd_get_reg(handle, 0x0d, &tmp);
			if ((tmp & 0x8f) == 0x0f)
			{
				U32 iPLSCnt = 0, iPLSIndex = 0;
				U8 val;

				iPLSCnt = sizeof(mPLSGoldTable) / sizeof(U32);

				for (iPLSIndex = 0; iPLSIndex < iPLSCnt; iPLSIndex++)
				{
					U8 PLSCode[3] = {0x01, 0x00, 0x00};

					//mt_fe_dmd_rs6060_calc_PLS_gold_code(PLSCode, mPLSGoldTable[iPLSIndex]);
					mt_unf_fe_calc_PLS_gold_code(PLSCode, mPLSGoldTable[iPLSIndex]);

					handle->dmd_set_reg(handle, 0x22, PLSCode[0]);
					handle->dmd_set_reg(handle, 0x23, PLSCode[1]);
					handle->dmd_set_reg(handle, 0x24, PLSCode[2]);

					//mt_fe_dmd_rs6060_soft_reset(handle);

					handle->mt_sleep(100);

					handle->dmd_get_reg(handle, 0x0d, &val);
					if ((val & 0x8f) == 0x8f)
					{
						//printk("%s[%d] ---- Super search OK, PLS Gold = %d!\n", __FUNCTION__, __LINE__, mPLSGoldTable[iPLSIndex]);

						handle->dmd_get_reg(handle, 0x22, &tmp1);
						//handle->dmd_set_reg(handle, 0x22, tmp1);
						handle->tp_cfg.ucPLSCode[0] = tmp1;

						handle->dmd_get_reg(handle, 0x23, &tmp1);
						//handle->dmd_set_reg(handle, 0x23, tmp1);
						handle->tp_cfg.ucPLSCode[1] = tmp1;

						handle->dmd_get_reg(handle, 0x24, &tmp1);
						tmp1 &= 0x03;
						//handle->dmd_set_reg(handle, 0x24, tmp1);
						handle->tp_cfg.ucPLSCode[2] = tmp1;

						handle->tp_cfg.bHavePLS = TRUE;

						break;
					}
				}

				if ((iPLSIndex == iPLSCnt) && ((val & 0x8f) != 0x8f))
				{
					handle->dmd_set_reg(handle, 0x22, 0x01);
					handle->dmd_set_reg(handle, 0x23, 0x00);
					handle->dmd_set_reg(handle, 0x24, 0x00);

					mt_fe_dmd_rs6060_soft_reset(handle);

#if 0
					handle->dmd_set_reg(handle, 0xae, 0x09);
					handle->dmd_set_reg(handle, 0x7f, 0x04);

					handle->dmd_get_reg(handle, 0xca, &ucTmp);
					ucTmp &= ~0x04;
					handle->dmd_set_reg(handle, 0xca, ucTmp);
#endif

					if (handle->tp_cfg.iSymRateKSs >= 20000)
					{
						handle->mt_sleep(100);
					}
					else if (handle->tp_cfg.iSymRateKSs >= 15000)
					{
						handle->mt_sleep(200);
					}
					else if (handle->tp_cfg.iSymRateKSs >= 10000)
					{
						handle->mt_sleep(300);
					}
					else
					{
						handle->mt_sleep(400);
					}

					//printk("%s[%d] ---- Super search failed\n", __FUNCTION__, __LINE__);
				}
			}
		}

		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if (((handle->tp_cfg.iSymRateKSs > 47100) && (handle->tp_cfg.iSymRateKSs < 47500) && ((tmp & 0x0f) == 0x0f)) || 
			((tmp & 0x8f) == 0x8f))
		{
			handle->dmd_get_reg(handle, 0x89, &tmp);
			handle->dmd_get_reg(handle, 0xca, &ucTmp);
			tmp &= 0x80;
			handle->dmd_set_reg(handle, 0xca, (U8)((ucTmp & 0xf7) | (tmp >> 4) | 0x02));

			handle->dmd_get_reg(handle, 0x19, &tmp);
			if ((tmp != 0x04) && (tmp != 0x06) && (tmp != 0x08) && (tmp != 0x0a) && (tmp != 0x30) && (tmp != 0x32) && (tmp != 0x34) && (tmp != 0x36) 
			 && (tmp < 0x80))
			{
				handle->dmd_set_reg(handle, 0xbd, 0x82);
				handle->dmd_set_reg(handle, 0x80, 0x78);//0xa8
				handle->dmd_set_reg(handle, 0x81, 0x8a);//0xea
			}
			else
			{
				handle->dmd_set_reg(handle, 0xbd, 0x82);
				handle->dmd_set_reg(handle, 0x80, 0xa8);//0xa8
				handle->dmd_set_reg(handle, 0x81, 0xea);//0xea
			}

			//if (handle->tp_cfg.ucCurTsId == 0)
			{
				handle->dmd_get_reg(handle, 0xbe, &ucTmp);
				if (ucTmp == 0xfe)
				{
					mt_fe_print(("\tPLS before checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x!\n", ucTmp, tmp));

					handle->dmd_get_reg(handle, 0x00, &tmp);

					return MtFeErr_Ok;
				}

				if (!handle->tp_cfg.bCheckCCM)
				{
					MT_BOOL bMIS = FALSE;

					bMIS = _mt_fe_dmd_rs6060_check_MIS(handle);

					if (bMIS)
					{
						int iTmpCnt = 0, iMaxCnt = 100;
						//handle->mt_sleep(10);		// Delay for MIS ready

						handle->dmd_set_reg(handle, 0xfa, 0x00);
						handle->dmd_set_reg(handle, 0xf0, 0x00);
						handle->dmd_set_reg(handle, 0xf0, 0x03);
						//handle->mt_sleep(500);

#if 1
						if (handle->tp_cfg.iSymRateKSs < 5000)
						{
							iMaxCnt = 40;
						}
						else if (handle->tp_cfg.iSymRateKSs < 10000)
						{
							iMaxCnt = 30;
						}
						else
						{
							iMaxCnt = 20;
						}

						do
						{
							handle->mt_sleep(50);
							handle->dmd_get_reg(handle, 0xbe, &ucTmp);

							iTmpCnt++;
						} while ((ucTmp != 0xfe) && (iTmpCnt < iMaxCnt)); // 50ms * 20 = 1000ms

						//printk("\tPLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms! [%d/%d]\n", ucTmp, tmp, 50 * iTmpCnt, iTmpCnt, iMaxCnt);

						mt_fe_print(("\tPLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms! [%d/%d]\n", ucTmp, tmp, 50 * iTmpCnt, iTmpCnt, iMaxCnt));
#else
						do
						{
							handle->mt_sleep(50);
							handle->dmd_get_reg(handle, 0xbe, &ucTmp);

							iTmpCnt++;
						} while ((ucTmp != 0xfe) && (iTmpCnt < 20));		// 50ms * 20 = 1000ms

						mt_fe_print(("\tPLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms!\n", ucTmp, tmp, 50 * iTmpCnt));
#endif

						if (ucTmp == 0xfe)
						{
							handle->dmd_get_reg(handle, 0x00, &tmp);

							return MtFeErr_Ok;
						}

						handle->dmd_get_reg(handle, 0xf1, &ucTmp);
						handle->tp_cfg.iTsCnt = ucTmp & 0x1F;

						if (handle->tp_cfg.iTsCnt >= 1)
						{
							mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", handle->tp_cfg.iTsCnt));

							for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++)
							{
								if (iCnt >= 32)
								{
									ret = MtFeErr_NoMemory;
									break;
								}

								ucTmp = (U8)iCnt;

								handle->dmd_set_reg(handle, 0xf2, ucTmp);
								handle->dmd_get_reg(handle, 0xf3, &ucTmp);

								handle->tp_cfg.ucTsId[iCnt] = ucTmp;

								mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
							}

							for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++)
							{
								if (handle->tp_cfg.ucTsId[iCnt] == handle->tp_cfg.ucCurTsId)
								{
									handle->dmd_set_reg(handle, 0xf5, handle->tp_cfg.ucCurTsId);

									mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, handle->tp_cfg.ucCurTsId));

									break;
								}
							}

							if (iCnt == handle->tp_cfg.iTsCnt)
							{
								// Default select TS0 output
								handle->dmd_set_reg(handle, 0xf5, handle->tp_cfg.ucTsId[0]);
								handle->tp_cfg.ucCurTsId = handle->tp_cfg.ucTsId[0];

								mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
							}
						}

						handle->tp_cfg.bCheckCCM = TRUE;
					}
				}
			}

			*p_state = MtFeLockState_Locked;
			handle->tp_cfg.mCurrentType = MtFeType_DvbS2;

			handle->dmd_set_reg(handle, 0xf9, 0x01);
		}
	}
	else
	{
		handle->dmd_set_reg(handle, 0xbd, 0x83);
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7)
		{
			*p_state = MtFeLockState_Locked;
			handle->tp_cfg.mCurrentType = MtFeType_DvbS;
		}
	}

	if (*p_state == MtFeLockState_Locked)
	{
		MT_FE_CHAN_INFO_DVBS2 ch_info;

		if (handle->tp_cfg.bLimitCarrierOffset)
		{
			carrier_offset_KHz = 0;
			if (handle->lnb_cfg.bUnicable)
			{
				_mt_fe_dmd_rs6060_get_carrier_offset(handle, &carrier_offset_KHz);
			}
			else
			{
				_mt_fe_dmd_rs6060_get_total_carrier_offset(handle, &carrier_offset_KHz);
			}

			mt_fe_print(("carrier_offset_KHz: %d", carrier_offset_KHz));

			if ((carrier_offset_KHz >= handle->tp_cfg.iOffsetRangeKHz) || 
				(carrier_offset_KHz <= - handle->tp_cfg.iOffsetRangeKHz))
			{
				handle->dmd_set_reg(handle, 0xb2, 0x01);
				handle->dmd_set_reg(handle, 0x00, 0x01);

				*p_state = MtFeLockState_Unlocked;

				handle->dmd_get_reg(handle, 0x00, &tmp);

				return MtFeErr_Ok;
			}
		}

		handle->dmd_get_reg(handle, 0x19, &ucTmp);
		if (ucTmp < 0x80)
		{
			mt_fe_dmd_rs6060_get_channel_info(handle, &ch_info);

			if ((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x0c);
			}
			else if ((ch_info.code_rate == MtFeCodeRate_3_5) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x07);
			}
			else if ((ch_info.code_rate == MtFeCodeRate_2_3) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x09);
			}
			else if ((ch_info.code_rate == MtFeCodeRate_5_6) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x12);
			}
			else if ((ch_info.code_rate == MtFeCodeRate_8_9) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x18);
			}
			else if ((ch_info.code_rate == MtFeCodeRate_9_10) && (ch_info.mod_mode == MtFeModMode_8psk))
			{
				handle->dmd_set_reg(handle, 0x9f, 0x19);
			}

			if (ch_info.mod_mode == MtFeModMode_8psk)
			{
				handle->dmd_set_reg(handle, 0x9e, 0xff);

				handle->dmd_get_reg(handle, 0x9d, &tmp);
				tmp &= 0xfe;
				handle->dmd_set_reg(handle, 0x9d, tmp);
			}
		}

#if 1
		if (!handle->ts_cfg.bParallelClkFixed)
		{
			if (((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
				((handle->ts_cfg.mTsMode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
				((handle->ts_cfg.mTsMode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
			{
				if (handle->global_cfg.bTsClockChecked)
				{
					handle->global_cfg.bTsClockChecked = FALSE;
					_mt_fe_dmd_rs6060_set_clock_ratio(handle);
				}
			}
		}
		else
		{
			if ((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0))
			{
				if (handle->global_cfg.bTsClockChecked)
				{
					handle->global_cfg.bTsClockChecked = FALSE;
					_mt_fe_dmd_rs6060_set_clock_ratio(handle);
				}
			}
		}
#else

#if (MT_FE_PARALLEL_CLK_OUTPUT_FIXED_24MHZ == 0)
		if (((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
			((handle->ts_cfg.mTsMode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
			((handle->ts_cfg.mTsMode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
#else
		if ((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0))
#endif
		{
			if (handle->global_cfg.bTsClockChecked)
			{
				handle->global_cfg.bTsClockChecked = FALSE;
				_mt_fe_dmd_rs6060_set_clock_ratio(handle);
			}
		}
#endif
	}
	else
	{
		handle->dmd_get_reg(handle, 0xb3, &tmp);
		if ((tmp & 0xc0) != 0x00)
		{
			*p_state = MtFeLockState_Unlocked;
		}
	}

	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_rs6060_get_pure_lock(MT_FE_RS6060_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	U8	tmp;
	S32	carrier_offset_KHz;

	if (p_state == 0)
		return MtFeErr_Param;

	*p_state = MtFeLockState_Unlocked;


	handle->dmd_get_reg(handle, 0x08, &tmp);
	if ((tmp & 0x08) == 0x08)
	{
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if (((handle->tp_cfg.iSymRateKSs > 47100) && (handle->tp_cfg.iSymRateKSs < 47500) && ((tmp & 0x0f) == 0x0f)) || 
			((tmp & 0x8f) == 0x8f))
		{
			*p_state = MtFeLockState_Locked;
			handle->tp_cfg.mCurrentType = MtFeType_DvbS2;
		}
	}
	else
	{
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7)
		{
			*p_state = MtFeLockState_Locked;
			handle->tp_cfg.mCurrentType = MtFeType_DvbS;
		}
	}

	if (*p_state == MtFeLockState_Locked)
	{
		if (handle->tp_cfg.bLimitCarrierOffset)
		{
			carrier_offset_KHz = 0;
			if (handle->lnb_cfg.bUnicable)
			{
				_mt_fe_dmd_rs6060_get_carrier_offset(handle, &carrier_offset_KHz);
			}
			else
			{
				_mt_fe_dmd_rs6060_get_total_carrier_offset(handle, &carrier_offset_KHz);
			}

			mt_fe_print(("carrier_offset_KHz: %d", carrier_offset_KHz));

			if ((carrier_offset_KHz >= handle->tp_cfg.iOffsetRangeKHz) || 
				(carrier_offset_KHz <= - handle->tp_cfg.iOffsetRangeKHz))
			{
				handle->dmd_set_reg(handle, 0xb2, 0x01);
				handle->dmd_set_reg(handle, 0x00, 0x01);

				*p_state = MtFeLockState_Unlocked;

				return MtFeErr_Ok;
			}
		}
	}

	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_rs6060_set_ts(MT_FE_RS6060_Device_Handle handle, U8 iIndex, U8 ucTsId)
{
	MT_FE_RET ret = MtFeErr_Ok;
	U8 val;

#if 0
	int iCnt;


	for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++)
	{
		if (handle->tp_cfg.ucTsId[iCnt] == ucTsId)
		{
#endif
			if (iIndex == 0)
			{
				handle->dmd_set_reg(handle, 0xf5, ucTsId);
			}
			else if ((iIndex == 1) && (handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial))
			{
				handle->dmd_set_reg(handle, 0xf6, ucTsId);
			}
			else if ((iIndex == 2) && (handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial))
			{
				handle->dmd_set_reg(handle, 0xfa, 0x01);
				handle->dmd_set_reg(handle, 0xf0, ucTsId);
				handle->dmd_set_reg(handle, 0xfa, 0x00);
			}

			mt_fe_print(("\tManual select TS%d -- 0x%02X\n", iIndex, ucTsId));

			handle->tp_cfg.ucCurTsId = ucTsId;

#if 0
			ret = MtFeErr_Ok;

			break;
		}
	}

	if (iCnt == handle->tp_cfg.iTsCnt)
	{
		//handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucTsId[0]);

		//mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));

		mt_fe_print(("\tCan't find this TS ---- 0x%02X\n", ucTsId));

		ret = MtFeErr_Param;
	}
#endif

	handle->dmd_get_reg(handle, 0x00, &val);

	return ret;
}

MT_FE_RET mt_fe_dmd_rs6060_clear_ts(MT_FE_RS6060_Device_Handle handle)
{
	U8 val;

	if ((handle->ts_cfg.mTsMode == MtFeTsOutMode_Serial) && handle->ts_cfg.b3SerialTsOut)
	{
		handle->dmd_set_reg(handle, 0xf5, 0x00);
		handle->dmd_set_reg(handle, 0xf6, 0x00);
		handle->dmd_set_reg(handle, 0xfa, 0x01);
		handle->dmd_set_reg(handle, 0xf0, 0x00);
		handle->dmd_set_reg(handle, 0xfa, 0x00);
	}
	else
	{
		handle->dmd_set_reg(handle, 0xf5, 0x00);
	}

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}

/*********************************************************************
** Function: mt_fe_dmd_rs6060_get_per
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
MT_FE_RET mt_fe_dmd_rs6060_get_per(MT_FE_RS6060_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags)
{
	#define TOTAL_PACKET_S	3000

	U8		tmp1, tmp2, tmp3;
	U8		val_0xF6, val_0xF7, val_0xF8;
	U32		code_rate_fac = 0, ldpc_frame_cnt;
	static  U32 pre_err_packags   = 0;
	static  U32 pre_total_packags = 0xffff;
	MT_FE_CHAN_INFO_DVBS2 p_info;


	*p_err_packags   = pre_err_packags;
	*p_total_packags = pre_total_packags;


	pre_err_packags = 0;

	handle->dmd_get_reg(handle, 0x08, &tmp1);
	if ((tmp1 & 0x08) == 0x08)		// DVB-S2
	{
		mt_fe_dmd_rs6060_get_channel_info(handle, &p_info);
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

		handle->dmd_get_reg(handle, 0xd7, &tmp1);
		handle->dmd_get_reg(handle, 0xd6, &tmp2);
		handle->dmd_get_reg(handle, 0xd5, &tmp3);
		ldpc_frame_cnt = (tmp1 << 16) | (tmp2 << 8) | tmp3;


		handle->dmd_get_reg(handle, 0xf8, &tmp1);
		handle->dmd_get_reg(handle, 0xf7, &tmp2);
		pre_err_packags = (tmp1 << 8) | tmp2;


		pre_total_packags = code_rate_fac * ldpc_frame_cnt / (188 * 8);


		if (ldpc_frame_cnt > TOTAL_PACKET_S)
		{
			handle->dmd_get_reg(handle, 0xd1, &tmp1);
			tmp1 = tmp1 | 0x01;
			handle->dmd_set_reg(handle, 0xd1, tmp1);

			handle->dmd_set_reg(handle, 0xf9, 0x01);
			handle->dmd_set_reg(handle, 0xf9, 0x00);

			tmp1 = tmp1 & 0xfe;
			handle->dmd_set_reg(handle, 0xd1, tmp1);

			*p_err_packags   = pre_err_packags;
			*p_total_packags = pre_total_packags;
		}
	}
	else		// DVB-S
	{
		handle->dmd_get_reg(handle, 0xd5, &val_0xF8);

		if ((val_0xF8 & 0x80) == 0)
		{
			handle->dmd_get_reg(handle, 0xd6, &val_0xF6);
			handle->dmd_get_reg(handle, 0xd7, &val_0xF7);

			pre_err_packags = (val_0xF7 << 8) | val_0xF6;

			//val_0xF8 |= 0x10;
			handle->dmd_set_reg(handle, 0xd5, 0x82);
		}

		*p_err_packags   = pre_err_packags;

		*p_total_packags = 8388608 / 16;
	}

	handle->dmd_get_reg(handle, 0x00, &tmp1);

	return MtFeErr_Ok;
}
/******************************************************************
** Function: mt_fe_dmd_rs6060_get_accurate_snr
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
**	  p_snr			S32*			snr with 0.001 dB unit
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_rs6060_get_accurate_snr(MT_FE_RS6060_Device_Handle handle, S32 *p_snr)
{
	static const U16 mse_log10_10[] = 
	{
		-10000, 	 -6989, 	 -5228, 	 -3979, 	 -3010, 	 -2218, 	 -1549, 	  -969, 	  -457, 
		     0, 	   413, 	   791, 	  1139, 	  1461, 	  1760, 	  2041, 	  2304, 	  2552, 	  2787, 
		  3010, 	  3222, 	  3424, 	  3617, 	  3802, 	  3979, 	  4149, 	  4313, 	  4471, 	  4623, 
		  4771, 	  4913, 	  5051, 	  5185, 	  5314, 	  5440, 	  5563, 	  5682, 	  5797, 	  5910, 
		  6020, 	  6127, 	  6232, 	  6334, 	  6434, 	  6532, 	  6627, 	  6720, 	  6812, 	  6901, 
		  6989, 	  7075, 	  7160, 	  7242, 	  7323, 	  7403, 	  7481, 	  7558, 	  7634, 	  7708, 
		  7781, 	  7853, 	  7923, 	  7993, 	  8061, 	  8129, 	  8195, 	  8260, 	  8325, 	  8388, 
		  8450, 	  8512, 	  8573, 	  8633, 	  8692, 	  8750, 	  8808, 	  8864, 	  8920, 	  8976, 
		  9030, 	  9084, 	  9138, 	  9190, 	  9242, 	  9294, 	  9344, 	  9395, 	  9444, 	  9493, 
		  9542, 	  9590, 	  9637, 	  9684, 	  9731, 	  9777, 	  9822, 	  9867, 	  9912, 	  9956, 
		 10000, 	 10043, 	 10086, 	 10128, 	 10170, 	 10211, 	 10253, 	 10293, 	 10334, 	 10374, 
		 10413, 	 10453, 	 10492, 	 10530, 	 10569, 	 10606, 	 10644, 	 10681, 	 10718, 	 10755, 
		 10791, 	 10827, 	 10863, 	 10899, 	 10934, 	 10969, 	 11003, 	 11038, 	 11072, 	 11105, 
		 11139, 	 11172, 	 11205, 	 11238, 	 11271, 	 11303, 	 11335, 	 11367, 	 11398, 	 11430, 
		 11461, 	 11492, 	 11522, 	 11553, 	 11583, 	 11613, 	 11643, 	 11673, 	 11702, 	 11731, 
		 11760, 	 11789, 	 11818, 	 11846, 	 11875, 	 11903, 	 11931, 	 11958, 	 11986, 	 12013, 
		 12041, 	 12068, 	 12095, 	 12121, 	 12148, 	 12174, 	 12201, 	 12227, 	 12253, 	 12278, 
		 12304, 	 12329, 	 12355, 	 12380, 	 12405, 	 12430, 	 12455, 	 12479, 	 12504, 	 12528, 
		 12552, 	 12576, 	 12600, 	 12624, 	 12648, 	 12671, 	 12695, 	 12718, 	 12741, 	 12764, 
		 12787, 	 12810, 	 12833, 	 12855, 	 12878, 	 12900, 	 12922, 	 12944, 	 12966, 	 12988, 
		 13010, 	 13031, 	 13053, 	 13074, 	 13096, 	 13117, 	 13138, 	 13159, 	 13180, 	 13201, 
		 13222, 	 13242, 	 13263, 	 13283, 	 13304, 	 13324, 	 13344, 	 13364, 	 13384, 	 13404, 
		 13424, 	 13443, 	 13463, 	 13483, 	 13502, 	 13521, 	 13541, 	 13560, 	 13579, 	 13598, 
		 13617, 	 13636, 	 13654, 	 13673, 	 13692, 	 13710, 	 13729, 	 13747, 	 13765, 	 13783, 
		 13802, 	 13820, 	 13838, 	 13856, 	 13873, 	 13891, 	 13909, 	 13926, 	 13944, 	 13961, 
		 13979, 	 13996, 	 14014, 	 14031, 	 14048, 	 14065, 	 14082, 	 14099, 	 14116, 	 14132, 
		 14149, 	 14166, 	 14183, 	 14199, 	 14216, 	 14232, 	 14248, 	 14265, 	 14281, 	 14297, 
		 14313, 	 14329, 	 14345, 	 14361, 	 14377, 	 14393, 	 14409, 	 14424, 	 14440, 	 14456, 
		 14471, 	 14487, 	 14502, 	 14517, 	 14533, 	 14548, 	 14563, 	 14578, 	 14593, 	 14608, 
		 14623, 	 14638, 	 14653, 	 14668, 	 14683, 	 14698, 	 14712, 	 14727, 	 14742, 	 14756, 
		 14771, 	 14785, 	 14800, 	 14814, 	 14828, 	 14842, 	 14857, 	 14871, 	 14885, 	 14899, 
		 14913, 	 14927, 	 14941, 	 14955, 	 14969, 	 14983, 	 14996, 	 15010, 	 15024, 	 15037, 
		 15051, 	 15065, 	 15078, 	 15092, 	 15105, 	 15118, 	 15132, 	 15145, 	 15158, 	 15171, 
		 15185, 	 15198, 	 15211, 	 15224, 	 15237, 	 15250, 	 15263, 	 15276, 	 15289, 	 15301, 
		 15314, 	 15327, 	 15340, 	 15352, 	 15365, 	 15378, 	 15390, 	 15403, 	 15415, 	 15428, 
		 15440, 	 15453, 	 15465, 	 15477, 	 15490, 	 15502, 	 15514, 	 15526, 	 15538, 	 15550, 
		 15563, 	 15575, 	 15587, 	 15599, 	 15611, 	 15622, 	 15634, 	 15646, 	 15658, 	 15670, 
		 15682, 	 15693, 	 15705, 	 15717, 	 15728, 	 15740, 	 15751, 	 15763, 	 15774, 	 15786, 
		 15797, 	 15809, 	 15820, 	 15831, 	 15843, 	 15854, 	 15865, 	 15877, 	 15888, 	 15899, 
		 15910, 	 15921, 	 15932, 	 15943, 	 15954, 	 15965, 	 15976, 	 15987, 	 15998, 	 16009, 
		 16020, 	 16031, 	 16042, 	 16053, 	 16063, 	 16074, 	 16085, 	 16095, 	 16106, 	 16117, 
		 16127, 	 16138, 	 16148, 	 16159, 	 16170, 	 16180, 	 16190, 	 16201, 	 16211, 	 16222, 
		 16232, 	 16242, 	 16253, 	 16263, 	 16273, 	 16283, 	 16294, 	 16304, 	 16314, 	 16324, 
		 16334, 	 16344, 	 16354, 	 16364, 	 16374, 	 16384, 	 16394, 	 16404, 	 16414, 	 16424, 
		 16434, 	 16444, 	 16454, 	 16464, 	 16473, 	 16483, 	 16493, 	 16503, 	 16512, 	 16522, 
		 16532, 	 16541, 	 16551, 	 16560, 	 16570, 	 16580, 	 16589, 	 16599, 	 16608, 	 16618, 
		 16627, 	 16637, 	 16646, 	 16655, 	 16665, 	 16674, 	 16683, 	 16693, 	 16702, 	 16711, 
		 16720, 	 16730, 	 16739, 	 16748, 	 16757, 	 16766, 	 16776, 	 16785, 	 16794, 	 16803, 
		 16812, 	 16821, 	 16830, 	 16839, 	 16848, 	 16857, 	 16866, 	 16875, 	 16884, 	 16893, 
		 16901, 	 16910, 	 16919, 	 16928, 	 16937, 	 16946, 	 16954, 	 16963, 	 16972, 	 16981, 
		 16989, 	 16998, 	 17007, 	 17015, 	 17024, 	 17032, 	 17041, 	 17050, 	 17058, 	 17067, 
		 17075, 	 17084, 	 17092, 	 17101, 	 17109, 	 17118, 	 17126, 	 17134, 	 17143, 	 17151, 
		 17160, 	 17168, 	 17176, 	 17185, 	 17193, 	 17201, 	 17209, 	 17218, 	 17226, 	 17234, 
		 17242, 	 17250, 	 17259, 	 17267, 	 17275, 	 17283, 	 17291, 	 17299, 	 17307, 	 17315, 
		 17323, 	 17331, 	 17339, 	 17347, 	 17355, 	 17363, 	 17371, 	 17379, 	 17387, 	 17395, 
		 17403, 	 17411, 	 17419, 	 17427, 	 17435, 	 17442, 	 17450, 	 17458, 	 17466, 	 17474, 
		 17481, 	 17489, 	 17497, 	 17505, 	 17512, 	 17520, 	 17528, 	 17535, 	 17543, 	 17551, 
		 17558, 	 17566, 	 17573, 	 17581, 	 17589, 	 17596, 	 17604, 	 17611, 	 17619, 	 17626, 
		 17634, 	 17641, 	 17649, 	 17656, 	 17664, 	 17671, 	 17678, 	 17686, 	 17693, 	 17701, 
		 17708, 	 17715, 	 17723, 	 17730, 	 17737, 	 17745, 	 17752, 	 17759, 	 17767, 	 17774, 
		 17781, 	 17788, 	 17795, 	 17803, 	 17810, 	 17817, 	 17824, 	 17831, 	 17839, 	 17846, 
		 17853, 	 17860, 	 17867, 	 17874, 	 17881, 	 17888, 	 17895, 	 17902, 	 17909, 	 17916, 
		 17923, 	 17930, 	 17937, 	 17944, 	 17951, 	 17958, 	 17965, 	 17972, 	 17979, 	 17986, 
		 17993, 	 18000, 	 18007, 	 18014, 	 18020, 	 18027, 	 18034, 	 18041, 	 18048, 	 18055, 
		 18061, 	 18068, 	 18075, 	 18082, 	 18088, 	 18095, 	 18102, 	 18109, 	 18115, 	 18122, 
		 18129, 	 18135, 	 18142, 	 18149, 	 18155, 	 18162, 	 18169, 	 18175, 	 18182, 	 18188, 
		 18195, 	 18202, 	 18208, 	 18215, 	 18221, 	 18228, 	 18234, 	 18241, 	 18247, 	 18254, 
		 18260, 	 18267, 	 18273, 	 18280, 	 18286, 	 18293, 	 18299, 	 18305, 	 18312, 	 18318, 
		 18325, 	 18331, 	 18337, 	 18344, 	 18350, 	 18356, 	 18363, 	 18369, 	 18375, 	 18382, 
		 18388, 	 18394, 	 18401, 	 18407, 	 18413, 	 18419, 	 18426, 	 18432, 	 18438, 	 18444, 
		 18450, 	 18457, 	 18463, 	 18469, 	 18475, 	 18481, 	 18488, 	 18494, 	 18500, 	 18506, 
		 18512, 	 18518, 	 18524, 	 18530, 	 18536, 	 18543, 	 18549, 	 18555, 	 18561, 	 18567, 
		 18573, 	 18579, 	 18585, 	 18591, 	 18597, 	 18603, 	 18609, 	 18615, 	 18621, 	 18627, 
		 18633, 	 18639, 	 18645, 	 18651, 	 18656, 	 18662, 	 18668, 	 18674, 	 18680, 	 18686, 
		 18692, 	 18698, 	 18704, 	 18709, 	 18715, 	 18721, 	 18727, 	 18733, 	 18739, 	 18744, 
		 18750, 	 18756, 	 18762, 	 18767, 	 18773, 	 18779, 	 18785, 	 18790, 	 18796, 	 18802, 
		 18808, 	 18813, 	 18819, 	 18825, 	 18830, 	 18836, 	 18842, 	 18847, 	 18853, 	 18859, 
		 18864, 	 18870, 	 18876, 	 18881, 	 18887, 	 18893, 	 18898, 	 18904, 	 18909, 	 18915, 
		 18920, 	 18926, 	 18932, 	 18937, 	 18943, 	 18948, 	 18954, 	 18959, 	 18965, 	 18970, 
		 18976, 	 18981, 	 18987, 	 18992, 	 18998, 	 19003, 	 19009, 	 19014, 	 19020, 	 19025, 
		 19030, 	 19036, 	 19041, 	 19047, 	 19052, 	 19057, 	 19063, 	 19068, 	 19074, 	 19079, 
		 19084, 	 19090, 	 19095, 	 19100, 	 19106, 	 19111, 	 19116, 	 19122, 	 19127, 	 19132, 
		 19138, 	 19143, 	 19148, 	 19153, 	 19159, 	 19164, 	 19169, 	 19175, 	 19180, 	 19185, 
		 19190, 	 19196, 	 19201, 	 19206, 	 19211, 	 19216, 	 19222, 	 19227, 	 19232, 	 19237, 
		 19242, 	 19247, 	 19253, 	 19258, 	 19263, 	 19268, 	 19273, 	 19278, 	 19283, 	 19289, 
		 19294, 	 19299, 	 19304, 	 19309, 	 19314, 	 19319, 	 19324, 	 19329, 	 19334, 	 19339, 
		 19344, 	 19350, 	 19355, 	 19360, 	 19365, 	 19370, 	 19375, 	 19380, 	 19385, 	 19390, 
		 19395, 	 19400, 	 19405, 	 19410, 	 19415, 	 19420, 	 19425, 	 19429, 	 19434, 	 19439, 
		 19444, 	 19449, 	 19454, 	 19459, 	 19464, 	 19469, 	 19474, 	 19479, 	 19484, 	 19489, 
		 19493, 	 19498, 	 19503, 	 19508, 	 19513, 	 19518, 	 19523, 	 19527, 	 19532, 	 19537, 
		 19542, 	 19547, 	 19552, 	 19556, 	 19561, 	 19566, 	 19571, 	 19576, 	 19580, 	 19585, 
		 19590, 	 19595, 	 19599, 	 19604, 	 19609, 	 19614, 	 19618, 	 19623, 	 19628, 	 19633, 
		 19637, 	 19642, 	 19647, 	 19652, 	 19656, 	 19661, 	 19666, 	 19670, 	 19675, 	 19680, 
		 19684, 	 19689, 	 19694, 	 19698, 	 19703, 	 19708, 	 19712, 	 19717, 	 19722, 	 19726, 
		 19731, 	 19735, 	 19740, 	 19745, 	 19749, 	 19754, 	 19758, 	 19763, 	 19768, 	 19772, 
		 19777, 	 19781, 	 19786, 	 19790, 	 19795, 	 19800, 	 19804, 	 19809, 	 19813, 	 19818, 
		 19822, 	 19827, 	 19831, 	 19836, 	 19840, 	 19845, 	 19849, 	 19854, 	 19858, 	 19863, 
		 19867, 	 19872, 	 19876, 	 19881, 	 19885, 	 19890, 	 19894, 	 19898, 	 19903, 	 19907, 
		 19912, 	 19916, 	 19921, 	 19925, 	 19929, 	 19934, 	 19938, 	 19943, 	 19947, 	 19951, 
		 19956, 	 19960, 	 19965, 	 19969, 	 19973, 	 19978, 	 19982, 	 19986, 	 19991, 	 19995, 
		 20000
	};

	const U16 mse_loge_10[] = 
	{
		-23025, 	-16094, 	-12039, 	 -9162, 	 -6931, 	 -5108, 	 -3566, 	 -2231, 	 -1053, 
		     0, 	   953, 	  1823, 	  2623, 	  3364, 	  4054, 	  4700, 	  5306, 	  5877, 	  6418, 
		  6931, 	  7419, 	  7884, 	  8329, 	  8754, 	  9162, 	  9555, 	  9932, 	 10296, 	 10647, 
		 10986, 	 11314, 	 11631, 	 11939, 	 12237, 	 12527, 	 12809, 	 13083, 	 13350, 	 13609, 
		 13862, 	 14109, 	 14350, 	 14586, 	 14816, 	 15040, 	 15260, 	 15475, 	 15686, 	 15892, 
		 16094, 	 16292, 	 16486, 	 16677, 	 16863, 	 17047, 	 17227, 	 17404, 	 17578, 	 17749, 
		 17917, 	 18082, 	 18245, 	 18405, 	 18562, 	 18718, 	 18870, 	 19021, 	 19169, 	 19315, 
		 19459, 	 19600, 	 19740, 	 19878, 	 20014, 	 20149, 	 20281, 	 20412, 	 20541, 	 20668, 
		 20794, 	 20918, 	 21041, 	 21162, 	 21282, 	 21400, 	 21517, 	 21633, 	 21747, 	 21860, 
		 21972, 	 22082, 	 22192, 	 22300, 	 22407, 	 22512, 	 22617, 	 22721, 	 22823, 	 22925, 
		 23025, 	 23125, 	 23223, 	 23321, 	 23418, 	 23513, 	 23608, 	 23702, 	 23795, 	 23887, 
		 23978, 	 24069, 	 24159, 	 24248, 	 24336, 	 24423, 	 24510, 	 24595, 	 24680, 	 24765, 
		 24849, 	 24932, 	 25014, 	 25095, 	 25176, 	 25257, 	 25336, 	 25416, 	 25494, 	 25572, 
		 25649, 	 25726, 	 25802, 	 25877, 	 25952, 	 26026, 	 26100, 	 26173, 	 26246, 	 26318, 
		 26390, 	 26461, 	 26532, 	 26602, 	 26672, 	 26741, 	 26810, 	 26878, 	 26946, 	 27013, 
		 27080, 	 27146, 	 27212, 	 27278, 	 27343, 	 27408, 	 27472, 	 27536, 	 27600, 	 27663, 
		 27725, 	 27788, 	 27850, 	 27911, 	 27972, 	 28033, 	 28094, 	 28154, 	 28213, 	 28273, 
		 28332, 	 28390, 	 28449, 	 28507, 	 28564, 	 28622, 	 28678, 	 28735, 	 28791, 	 28848, 
		 28903, 	 28959, 	 29014, 	 29069, 	 29123, 	 29177, 	 29231, 	 29285, 	 29338, 	 29391, 
		 29444, 	 29496, 	 29549, 	 29601, 	 29652, 	 29704, 	 29755, 	 29806, 	 29856, 	 29907, 
		 29957, 	 30007, 	 30056, 	 30106, 	 30155, 	 30204, 	 30252, 	 30301, 	 30349, 	 30397, 
		 30445, 	 30492, 	 30540, 	 30587, 	 30633, 	 30680, 	 30726, 	 30773, 	 30819, 	 30864, 
		 30910, 	 30955, 	 31000, 	 31045, 	 31090, 	 31135, 	 31179, 	 31223, 	 31267, 	 31311, 
		 31354, 	 31398, 	 31441, 	 31484, 	 31527, 	 31570, 	 31612, 	 31654, 	 31696, 	 31738, 
		 31780, 	 31822, 	 31863, 	 31904, 	 31945, 	 31986, 	 32027, 	 32068, 	 32108, 	 32148, 
		 32188, 	 32228, 	 32268, 	 32308, 	 32347, 	 32386, 	 32425, 	 32464, 	 32503, 	 32542, 
		 32580, 	 32619, 	 32657, 	 32695, 	 32733, 	 32771, 	 32809, 	 32846, 	 32884, 	 32921, 
		 32958, 	 32995, 	 33032, 	 33068, 	 33105, 	 33141, 	 33178, 	 33214, 	 33250, 	 33286, 
		 33322, 	 33357, 	 33393, 	 33428, 	 33463, 	 33499, 	 33534, 	 33568, 	 33603, 	 33638, 
		 33672, 	 33707, 	 33741, 	 33775, 	 33809, 	 33843, 	 33877, 	 33911, 	 33945, 	 33978, 
		 34011, 	 34045, 	 34078, 	 34111, 	 34144, 	 34177, 	 34210, 	 34242, 	 34275, 	 34307, 
		 34339, 	 34372, 	 34404, 	 34436, 	 34468, 	 34499, 	 34531, 	 34563, 	 34594, 	 34626, 
		 34657
	};


	S32		snr;

	U8		val, npow1, npow2, spow1, cnt;
	int		tmp;
	U32		npow, spow, snr_total;

	int	iCnt = 0;


	*p_snr = 0;

    handle->dmd_get_reg(handle, 0x08, &val);
	if ((val & 0x08) != 0)	// DVB-S2
	{
		cnt  = 10;
		npow = 0;
		spow = 0;

		while (cnt > 0)
		{

            handle->dmd_get_reg(handle, 0x8c, &npow1);
			handle->dmd_get_reg(handle, 0x8d, &npow2);
			npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

            handle->dmd_get_reg(handle, 0x8e, &spow1);
			spow += ((spow1 * spow1) >> 1);

			cnt--;
		}

		if (spow == 0)
		{
			snr = 0;
		}
		else if (npow == 0)
		{
			snr = 30000;
		}
		else
		{
			if (spow >= npow)
			{
				tmp = (U16)(spow * 10 / npow);

				while (tmp >= 1000)
				{
					tmp /= 2;
					iCnt++;
				}

				snr = mse_log10_10[tmp - 1] + iCnt * 3010;
			}
			else
			{
				tmp = (U16)(npow * 10 / spow);
				while (tmp > 1000)
				{
					tmp /= 2;
					iCnt++;
				}

				snr = -(mse_log10_10[tmp - 1] + iCnt * 3010);
			}
		}
	}
	else
	{
		cnt = 10;
		snr_total = 0;

		while (cnt > 0)
		{
            handle->dmd_get_reg(handle, 0xff, &val);
			snr_total += val;

			cnt--;
		}

		tmp = snr_total / 8;
		if (tmp > 9)
		{
			snr =  (mse_loge_10[tmp - 1] * 10000) / 23026;
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
** Function: mt_fe_dmd_rs6060_get_snr
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
MT_FE_RET mt_fe_dmd_rs6060_get_snr(MT_FE_RS6060_Device_Handle handle, S8 *p_snr)
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



	static	S8 snr;

	U8		val, npow1, npow2, spow1, cnt;
	U16		tmp;
	U32		npow, spow, snr_total;


	*p_snr = 0;

	handle->dmd_get_reg(handle, 0x08, &val);
	if ((val & 0x08) != 0)	//DVB-S2
	{
		cnt  = 10;
		npow = 0;
		spow = 0;

		while (cnt > 0)
		{
			handle->dmd_get_reg(handle, 0x8c, &npow1);
			handle->dmd_get_reg(handle, 0x8d, &npow2);
			npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

			handle->dmd_get_reg(handle, 0x8e, &spow1);
			spow += ((spow1 * spow1) >> 1);

			cnt--;
		}

		npow /= 10;
		spow /= 10;

		if (spow == 0)
		{
			snr = 0;
		}
		else if (npow == 0)
		{
			snr = 19;
		}
		else
		{
			if (spow > npow)
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

		while (cnt > 0)
		{
			handle->dmd_get_reg(handle, 0xff, &val);
			snr_total += val;

			cnt--;
		}

		tmp = (U16)(snr_total / 80);
		if (tmp > 0)
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

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}


/******************************************************************
** Function: mt_fe_dmd_rs6060_get_strength
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
MT_FE_RET mt_fe_dmd_rs6060_get_strength(MT_FE_RS6060_Device_Handle handle, S8 *p_strength)
{
	U8 val;

	*p_strength = (S8)mt_fe_tn_get_signal_strength_rs6060(handle);

	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}

#if 1
MT_FE_RET mt_fe_dmd_rs6060_get_sat_quality(MT_FE_RS6060_Device_Handle handle, U8 *p_percent)
{
	MT_FE_LOCK_STATE lock_state;
	U8 val_00;

	mt_fe_dmd_rs6060_get_pure_lock(handle, &lock_state);

	if (lock_state == MtFeLockState_Locked)
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
		if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2)
		{
			cnt  = 10;
			npow = 0;
			spow = 0;

			while (cnt > 0)
			{
				handle->dmd_get_reg(handle, 0x8c, &npow1);
				handle->dmd_get_reg(handle, 0x8d, &npow2);
				npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

				handle->dmd_get_reg(handle, 0x8e, &spow1);
				spow += ((spow1 * spow1) >> 1);

				cnt--;
			}

			npow /= 10;
			spow /= 10;

			if (spow == 0)
			{
				snr = 0;
			}
			else if (npow == 0)
			{
				snr = 20;
			}
			else
			{
				if (spow > npow)
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

			*p_percent = 30 + (snr + 5) * 70 / 25;		/* 30% ~ 100% */
		}
		else
		{
			cnt = 10;
			snr_total = 0;

			while (cnt > 0)
			{
				handle->dmd_get_reg(handle, 0xff, &val);
				snr_total += val;

				cnt--;
			}

			snr = snr_total / 10;


			*p_percent = 30 + snr * 70 / 256;		/* 30% ~ 100% */
		}
	}
	else
	{
		U8 val, cnt;

		U16 total = 0;

		//handle->dmd_set_reg(handle, 0x63, 0x4f);

		for (cnt = 0; cnt < 3; cnt++)
		{
			handle->dmd_get_reg(handle, 0x8f, &val);

			total += val;
		}

		val = total / 3;

		if (val < 0x0e)
		{
			val = 0x0e;
		}

		if (val > 0x87)
		{
			val = 0x87;
		}

		*p_percent = (val - 0x0e) * 29 / 121;			/* 0% ~ 29% */
	}

	handle->dmd_get_reg(handle, 0x00, &val_00);

	return MtFeErr_Ok;
}
#else
MT_FE_RET mt_fe_dmd_rs6060_get_sat_quality(MT_FE_RS6060_Device_Handle handle, U8 *p_percent)
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

	const U16 mes_loge[] = 
	{
		    0,	 6931,	10986,	13863, 	16094,	17918,	19459,	20794,	21972,	23026,
		23979,	24849,	25649,	26391,	27081,	27726,	28332,	28904,	29444,	29957,
		30445,	30910,	31355,	31781,	32189,	32581,	32958,	33322,	33673,	34012,
		34340,	34657
	};


	MT_FE_CHAN_INFO_DVBS2 ch_info;

	S32		snr, snr_100, snr_32;

	U8		val, npow1, npow2, spow1, cnt;
	U32		npow, spow, snr_total;


	*p_percent = 0;

	if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2)
	{
		cnt  = 10;
		npow = 0;
		spow = 0;

		while (cnt > 0)
		{
			handle->dmd_get_reg(handle, 0x8c, &npow1);
			handle->dmd_get_reg(handle, 0x8d, &npow2);
			npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

			handle->dmd_get_reg(handle, 0x8e, &spow1);
			spow += ((spow1 * spow1) >> 1);

			cnt--;
		}

		if ((spow == 0) || (spow < npow))
		{
			*p_percent = 0;
		}
		else if (npow == 0)
		{
			*p_percent = 100;
		}
		else
		{
			U32 iRatio = spow / npow;
			U32 iRatio_100 = spow * 100 / npow - iRatio * 100;

			if (iRatio < 1)
			{
				iRatio = 1;
				iRatio_100 = 0;
			}

			if (iRatio >= 100)
			{
				iRatio = 100;
				iRatio_100 = 0;
			}

			snr = mes_log10[iRatio - 1];

			if (iRatio == 100)
				snr_100 = 0;
			else
				snr_100 = (mes_log10[iRatio] - mes_log10[iRatio - 1]) * iRatio_100 / 100;

			snr += snr_100;

			mt_fe_dmd_rs6060_get_channel_info(handle, &ch_info);

			if (ch_info.mod_mode == MtFeModMode_8psk)
			{
				if (ch_info.code_rate == MtFeCodeRate_9_10)
				{
					if (snr < 9000)
						snr = 9000;

					if (snr > 12000)
						snr = 12000;

					*p_percent = (U8)((snr - 9000) / 30);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_8_9)
				{
					if (snr < 9000)
						snr = 9000;

					if (snr > 12000)
						snr = 12000;

					*p_percent = (U8)((snr - 9000) / 30);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_5_6)
				{
					if (snr < 8000)
						snr = 8000;

					if (snr > 11000)
						snr = 11000;

					*p_percent = (U8)((snr - 8000) / 30);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_3_4)
				{
					if (snr < 7200)
						snr = 7200;

					if (snr > 8000)
						snr = snr * 11 / 10;

					if (snr > 11000)
						snr = 11000;

					*p_percent = (U8)((snr - 7200) / 38);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_2_3)
				{
					if (snr < 5500)
						snr = 5500;

					if (snr > 6000)
						snr = 12 * snr / 10;

					if (snr > 11000)
						snr = 11000;

					*p_percent = (U8)((snr - 5500) / 55);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_3_5)
				{
					if (snr < 4500)
						snr = 4500;

					if (snr > 6000)
						snr = 12 * snr / 10;

					if (snr > 10000)
						snr = 10000;

					*p_percent = (U8)((snr - 4500) / 55);		/* 0% ~ 100% */
				}
				else
				{
					if (snr < 5000)
						snr = 5000;

					if (snr > 6000)
						snr = 12 * snr / 10;

					if (snr > 15000)
						snr = 15000;

					*p_percent = (U8)((snr - 5000) / 100);		/* 0% ~ 100% */
				}
			}
			else if (ch_info.mod_mode == MtFeModMode_Qpsk)
			{
				if (ch_info.code_rate == MtFeCodeRate_5_6)
				{
					if (snr < 4000)
						snr = 4000;

					if (snr > 4500)
						snr = 12 * snr / 10;

					if (snr > 8000)
						snr = 8000;

					*p_percent = (U8)((snr - 4000) / 40);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_3_4)
				{
					if (snr < 2000)
						snr = 2000;

					if (snr > 3200)
						snr = 11 * snr / 10;

					if (snr > 7000)
						snr = 7000;

					*p_percent = (U8)((snr - 2000) / 50);		/* 0% ~ 100% */
				}
				else if (ch_info.code_rate == MtFeCodeRate_2_3)
				{
					if (snr < 2000)
						snr = 2000;

					if (snr > 2600)
						snr = 12 * snr / 10;

					if (snr > 6000)
						snr = 6000;

					*p_percent = (U8)((snr - 2000) / 40);		/* 0% ~ 100% */
				}
				else
				{
					if (snr < 2000)
						snr = 2000;

					if (snr > 3200)
						snr = 11 * snr / 10;

					if (snr > 7000)
						snr = 7000;

					*p_percent = (U8)((snr - 2000) / 50);		/* 0% ~ 100% */
				}
			}
			else // not 8psk and qpsk
			{
				if (snr < 5000)
					snr = 5000;

				if (snr > 6000)
					snr = 12 * snr / 10;

				if (snr > 15000)
					snr = 15000;

				*p_percent = (U8)((snr - 5000) / 100);			/* 0% ~ 100% */
			}
		}
	}
	else
	{
		cnt = 10;
		snr_total = 0;

		while (cnt > 0)
		{
			handle->dmd_get_reg(handle, 0xff, &val);
			snr_total += val;

			cnt--;
		}

		if (snr_total < 80)
		{
			*p_percent = 0;
		}
		else if (snr_total >= 2550)
		{
			*p_percent = 100;
		}
		else
		{
			U32 iRatio = snr_total / 80;
			U32 iRatio_32 = snr_total * 32 / 80 - iRatio * 32;

			snr = mes_loge[iRatio - 1];

			snr_32 = (mes_loge[iRatio] - mes_loge[iRatio - 1]) * iRatio_32 / 32;

			snr += snr_32;

			snr *= 100;
			snr /= 23026;

			mt_fe_dmd_rs6060_get_channel_info(handle, &ch_info);
			if (ch_info.code_rate == MtFeCodeRate_7_8)
			{
				if (snr < 55)
					snr = 55;

				if (snr > 65)
				{
					snr = 102 * snr / 100;
				}

				if (snr > 110)
					snr = 110;

				*p_percent = (U8)((snr - 55) * 100 / 55);		/* 0% ~ 100% */
			}
			else if (ch_info.code_rate == MtFeCodeRate_5_6)
			{
				if (snr < 50)
					snr = 50;

				if (snr > 60)
				{
					snr = 105 * snr / 100;
				}

				if (snr > 100)
					snr = 100;

				*p_percent = (U8)((snr - 50) * 100 / 50);		/* 0% ~ 100% */
			}
			else if (ch_info.code_rate == MtFeCodeRate_3_4)
			{
				if (snr < 50)
					snr = 50;

				if (snr > 60)
				{
					snr = 105 * snr / 100;
				}

				if (snr > 85)
					snr = 85;

				*p_percent = (U8)((snr - 50) * 100 / 35);		/* 0% ~ 100% */
			}
			else if (ch_info.code_rate == MtFeCodeRate_2_3)
			{
				if (snr < 45)
					snr = 45;

				if (snr > 50)
				{
					snr = 112 * snr / 100;
				}

				if (snr > 85)
					snr = 85;

				*p_percent = (U8)((snr - 45) * 100 / 40);		/* 0% ~ 100% */
			}
			else if (ch_info.code_rate == MtFeCodeRate_1_2)
			{
				if (snr < 40)
					snr = 40;

				if (snr > 50)
				{
					snr = 115 * snr / 100;
				}

				if (snr > 80)
					snr = 80;

				*p_percent = (U8)((snr - 40) * 100 / 40);		/* 0% ~ 100% */
			}
		}
	}

	return MtFeErr_Ok;
}
#endif

/******************************************************************
** Function: mt_fe_dmd_rs6060_get_channel_info
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
MT_FE_RET mt_fe_dmd_rs6060_get_channel_info(MT_FE_RS6060_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info)
{
	U8	tmp, val_0x17, val_0x18, ucPLSCode/*, val_0x7E*/;


	//info->type = handle->tp_cfg.mCurrentType;

	handle->dmd_get_reg(handle, 0x08, &tmp);
	if ((tmp & 0x08) == 0x08)	// DVB-S2
	//(handle->tp_cfg.mCurrentType == MtFeType_DvbS2)
	{
		handle->tp_cfg.mCurrentType = MtFeType_DvbS2;

		p_info->type = MtFeType_DvbS2;

#if 1
		handle->dmd_set_reg(handle, 0x8a, 0x01);

		handle->dmd_get_reg(handle, 0x17, &val_0x17);
		p_info->is_dummy_frame = (val_0x17 & 0x40) ? MtFe_True : MtFe_False;

		handle->dmd_get_reg(handle, 0x18, &val_0x18);
		tmp = (val_0x18 >> 5) & 0x07;
		switch (tmp)
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

		handle->dmd_get_reg(handle, 0x19, &ucPLSCode);

		p_info->iPlsCode		 = ucPLSCode;
		p_info->type			 = mPLSInfoTable[ucPLSCode].mDvbType;

		if (mPLSInfoTable[ucPLSCode].bValid)
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
		handle->dmd_get_reg(handle, 0x7e, &val_0x7E);
		tmp = (U8)((val_0x7E&0xC0) >> 6);
		switch (tmp)
		{
			case 0:  p_info->mod_mode = MtFeModMode_Qpsk;	break;
			case 1:  p_info->mod_mode = MtFeModMode_8psk;	break;
			case 2:  p_info->mod_mode = MtFeModMode_16Apsk;	break;
			case 3:  p_info->mod_mode = MtFeModMode_32Apsk;	break;
			default: p_info->mod_mode = MtFeModMode_Undef;	break;
		}


		p_info->is_pilot_on = ((val_0x7E & 0x20) != 0) ? MtFe_True : MtFe_False;


		tmp = (U8)(val_0x7E & 0x0f);
		switch (tmp)
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

		handle->dmd_get_reg(handle, 0x89, &tmp);
		p_info->is_spectrum_inv = ((tmp & 0x80) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;


		handle->dmd_get_reg(handle, 0x76, &tmp);
		tmp	&= 0x03;
		if (p_info->type == MtFeType_DvbS2X)		// DVB-S2X
		{
			switch (tmp)
			{
				case 0:  p_info->roll_off = MtFeRollOff_0p15; 	break;
				case 1:  p_info->roll_off = MtFeRollOff_0p10; 	break;
				case 2:  p_info->roll_off = MtFeRollOff_0p05; 	break;
				default: p_info->roll_off = MtFeRollOff_Undef; 	break;
			}
		}
		else									// DVB-S2
		{
			switch (tmp)
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
		handle->tp_cfg.mCurrentType = MtFeType_DvbS;

		p_info->type = MtFeType_DvbS;

		p_info->mod_mode = MtFeModMode_Qpsk;


		handle->dmd_get_reg(handle, 0xe6, &tmp);
		tmp = (U8)(tmp >> 5);
		switch (tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_7_8;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}

		p_info->is_pilot_on = MtFe_False;

		handle->dmd_get_reg(handle, 0xe0, &tmp);
		p_info->is_spectrum_inv = ((tmp & 0x04) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;

		p_info->roll_off = MtFeRollOff_0p35;

		p_info->is_dummy_frame = MtFe_False;
		p_info->iVcmCycle = -1;
		p_info->iPlsCode = 0;
		p_info->iFrameLength = 0;
	}


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


/*********************************************************************************
** Function: mt_fe_dmd_rs6060_set_LNB
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
MT_FE_RET mt_fe_dmd_rs6060_set_LNB(MT_FE_RS6060_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
	U8	val_0xa1, val_0xa2;


	handle->dmd_get_reg(handle, 0xa1, &val_0xa1);
	handle->dmd_get_reg(handle, 0xa2, &val_0xa2);


	if (is_LNB_enable != MtFe_True)
	{
		val_0xa1 |= 0x40;

		/*set LNB_EN pin HIGH or LOW */
		if (handle->board_cfg.bPolarLNBEnable)
		{
			val_0xa2 &= ~0x02;
		}
		else
		{
			val_0xa2 |= 0x02;
		}


		/*set V_SEL Pin mode*/
		if (handle->board_cfg.bPolarLNBStandby)
		{
			val_0xa2 |= 0x01;
		}
		else
		{
			val_0xa2 &= ~0x01;
		}


		/*set DiSEqC_OUT mode*/
		if (handle->board_cfg.bPolarDiSEqCOut)
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
		if (handle->board_cfg.bPolarLNBEnable)
		{
			val_0xa2 |= 0x02;
		}
		else
		{
			val_0xa2 &= ~0x02;
		}


		if (handle->board_cfg.bPolarLNB13V)
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
			if (handle->board_cfg.bPolarDiSEqCOut)
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

	if (is_envelop_mode != MtFe_True)
	{
		val_0xa2 &= ~0x20;
	}
	else
	{
		val_0xa2 |= 0x20;
	}

	handle->dmd_set_reg(handle, 0xa2, val_0xa2);
	handle->dmd_set_reg(handle, 0xa1, val_0xa1);

	handle->lnb_cfg.b22K = is_22k_enable;
	handle->lnb_cfg.mLnbVoltage = voltage_type;


	handle->dmd_get_reg(handle, 0x00, &val_0xa1);

	return MtFeErr_Ok;
}


/*************************************************************************************
** Function: mt_fe_dmd_rs6060_DiSEqC_send_tone_burst
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
MT_FE_RET mt_fe_dmd_rs6060_DiSEqC_send_tone_burst(MT_FE_RS6060_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
	U8	val;
	S32	time_out;


	handle->dmd_get_reg(handle, 0xa2, &val);
	val &= ~0xc0;
	if (is_envelop_mode != MtFe_True)
	{
		val &= ~0x20;
	}
	else
	{
		val |= 0x20;
	}
	handle->dmd_set_reg(handle, 0xa2, val);


	if (mode == MtFeDiSEqCToneBurst_Moulated)
		handle->dmd_set_reg(handle, 0xa1, 0x01);
	else
		handle->dmd_set_reg(handle, 0xa1, 0x02);

	handle->mt_sleep(13);

	time_out = 5;
	do
	{
		handle->dmd_get_reg(handle, 0xa1, &val);
		if ((val & 0x40) == 0)
			break;

		handle->mt_sleep(1);
		time_out--;

	} while (time_out > 0);


	handle->dmd_get_reg(handle, 0xa2, &val);
	if (handle->board_cfg.bPolarDiSEqCOut)
	{
		val |= 0xc0;
	}
	else
	{
		val &= ~0xc0;
		val |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, val);


	handle->dmd_get_reg(handle, 0x00, &val);

	return MtFeErr_Ok;
}

/*****************************************************************************
** Function: mt_fe_dmd_rs6060_DiSEqC_send_receive_msg
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
MT_FE_RET mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(MT_FE_RS6060_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i;
	U8	tmp, tmp1;
	S32	time_out;

	if (msg->size_send > 8)
		return MtFeErr_Param;

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if (msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i++)
	{
		handle->dmd_set_reg(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	handle->dmd_set_reg(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		handle->mt_sleep(10);
		time_out -= 10;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}

	if (!(msg->is_enable_receive))
	{
		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		handle->mt_sleep(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1     = tmp;
	while (time_out > 0)
	{
		handle->mt_sleep(15);
		time_out -= 15;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	handle->dmd_set_reg(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i++)
	{
		handle->dmd_get_reg(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	handle->dmd_get_reg(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_Fail;
	}


	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);

	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_rs6060_DiSEqC_send_msg(MT_FE_RS6060_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i;
	U8	tmp;
	/*U8  tmp1; clean warning*/
	S32	time_out;

	if (msg->size_send > 8)
		return MtFeErr_Param;

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if (msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i++)
	{
		handle->dmd_set_reg(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	handle->dmd_set_reg(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		handle->mt_sleep(10);
		time_out -= 10;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}

#if 0
	if (!(msg->is_enable_receive))
	{
		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		handle->mt_sleep(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1     = tmp;
	while (time_out > 0)
	{
		handle->mt_sleep(15);
		time_out -= 15;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	handle->dmd_set_reg(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i++)
	{
		handle->dmd_get_reg(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	handle->dmd_get_reg(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		return MtFeErr_Fail;
	}


	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);
#endif

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_rs6060_DiSEqC_receive_msg(MT_FE_RS6060_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	U8	i;
	U8	tmp, tmp1;
	S32	time_out;

#if 0
	if (msg->size_send > 8)
		return MtFeErr_Param;

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	tmp &= ~0xc0;
	if (msg->is_envelop_mode != TRUE)
	{
		tmp &= ~0x20;
	}
	else
	{
		tmp |= 0x20;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);


	for (i = 0; i < msg->size_send; i++)
	{
		handle->dmd_set_reg(handle, (U8)(0xa3 + i), msg->data_send[i]);
	}

	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x38;
	tmp &= ~0x40;
	tmp |= ((msg->size_send - 1) << 3) | 0x07;
	if (msg->is_enable_receive)
		tmp |= 0x80;
	else
		tmp &= ~0x80;

	handle->dmd_set_reg(handle, 0xa1, tmp);


	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	while (time_out > 0)
	{
		handle->mt_sleep(10);
		time_out -= 10;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		if ((tmp & 0x40) == 0)
			break;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		return MtFeErr_TimeOut;
	}
#endif

	if (!(msg->is_enable_receive))
	{
		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_Ok;
	}

	time_out = 170;
	while (time_out > 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp != 0)
			break;

		handle->mt_sleep(10);
		time_out -= 10;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}

	/*	1.5 * 9 * 8	= 108ms	*/
	time_out = 150;
	tmp1     = tmp;
	while (time_out > 0)
	{
		handle->mt_sleep(15);
		time_out -= 15;

		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp = (U8)((tmp >> 3) & 0x07);
		if (tmp == tmp1 || tmp == 7)
			break;

		tmp1 = tmp;
	}

	if (time_out <= 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_TimeOut;
	}


	msg->size_receive = tmp;
	handle->dmd_set_reg(handle, 0xa1, 0x40);

	for (i = 0; i < msg->size_receive; i++)
	{
		handle->dmd_get_reg(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
	}



	/*	Indicates the parity error occurs	*/
	handle->dmd_get_reg(handle, 0xab, &tmp);
	tmp <<= (8 - msg->size_receive);
	tmp &= 0xFF;
	if (tmp != 0)
	{
		handle->dmd_get_reg(handle, 0xa1, &tmp);
		tmp &= ~0x80;
		tmp |= 0x40;
		handle->dmd_set_reg(handle, 0xa1, tmp);


		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if (handle->board_cfg.bPolarDiSEqCOut)
		{
			tmp |= 0xc0;
		}
		else
		{
			tmp &= ~0xc0;
			tmp |= 0x80;
		}
		handle->dmd_set_reg(handle, 0xa2, tmp);

		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_Fail;
	}


	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut)
	{
		tmp |= 0xc0;
	}
	else
	{
		tmp &= ~0xc0;
		tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


/**********************************************************************
** Function: mt_fe_rs6060_blindscan_abort
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
MT_FE_RET mt_fe_dmd_rs6060_blindscan_abort(MT_FE_RS6060_Device_Handle handle, MT_FE_BOOL bs_abort)
{
	handle->global_cfg.bCancelBs = (bs_abort == MtFe_True) ? TRUE : FALSE;

	return MtFeErr_Ok;
}



/*********************************************************************************
** Function: mt_fe_dmd_rs6060_register_blindscan_callback
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
MT_FE_RET mt_fe_dmd_rs6060_register_notify(void (*callback)(MT_FE_MSG msg, void *p_tp_info))
{
	if (callback == NULL)
		return MtFeErr_NullPointer;

	mt_fe_bs_notify = callback;

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_rs6060_select_xm(MT_FE_RS6060_Device_Handle handle, U32 *xm_KHz, U32 tuner_freq_MHz)
{
	U32 symbol_rate = 0;

	_mt_fe_dmd_rs6060_get_sym_rate(handle, &symbol_rate);

	mt_fe_tn_dmpll_select_xm_rs6060(handle, xm_KHz, symbol_rate, tuner_freq_MHz);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_rs6060_select_mclk(MT_FE_RS6060_Device_Handle handle, U32 tuner_freq_MHz)
{
	mt_fe_tn_dmpll_select_mclk_rs6060(handle, tuner_freq_MHz, FALSE);

	if (handle->global_cfg.iMclkKHz == 93000)
		handle->dmd_set_reg(handle, 0xa0, 0x42);
	else if (handle->global_cfg.iMclkKHz == 96000)
		handle->dmd_set_reg(handle, 0xa0, 0x44);
	else if (handle->global_cfg.iMclkKHz == 99000)
		handle->dmd_set_reg(handle, 0xa0, 0x46);
	else if (handle->global_cfg.iMclkKHz == 110250)
		handle->dmd_set_reg(handle, 0xa0, 0x4e);
	else
		handle->dmd_set_reg(handle, 0xa0, 0x44);


	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_rs6060_get_ts_mclk(MT_FE_RS6060_Device_Handle handle, U32 *p_MCLK_KHz)
{
	mt_fe_tn_dmpll_get_ts_mclk_rs6060(handle, p_MCLK_KHz);

	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_rs6060_set_ts_mclk
**
**
** Description:	set TS Clock Ratio for RS6060.
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
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_mclk(MT_FE_RS6060_Device_Handle handle, U32 MCLK_KHz)
{
	MT_FE_TS_OUT_MODE ts_mode = MtFeTsOutMode_Common;

	_mt_fe_dmd_rs6060_get_current_ts_mode(handle, &ts_mode);

	mt_fe_tn_dmpll_set_ts_mclk_rs6060(handle, ts_mode, MCLK_KHz);

	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_rs6060_set_ts_divide_ratio
**
**
** Description:	set the TS divide ratio
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------
**	type			MT_FE_TYPE	MtFeType_DvbS2 or MtFeType_DvbS.
**	dr_high			U8			The length of high level of the MPEG clock.
**	dr_low			U8			The length of low level of the MPEG clock.
**
**
** Outputs:		none
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_divide_ratio(MT_FE_RS6060_Device_Handle handle, MT_FE_TYPE type, U8 dr_high, U8 dr_low)
{
	U8 val, tmp1, tmp2;

	tmp1 = dr_high;
	tmp2 = dr_low;


	if (type == MtFeType_DvbS2)
	{
		tmp1 -= 1;
		tmp2 -= 1;

		tmp1 &= 0x3f;
		tmp2 &= 0x3f;

		//handle->dmd_set_reg(handle, 0xfe, 0x20);	// default value

		handle->dmd_get_reg(handle, 0xfe, &val);		// ci_divrange_h_1 bits[5:2]
		val &= 0xF0;						// bits[3:0]
		val |= (tmp1 >> 2) & 0x0f;
		handle->dmd_set_reg(handle, 0xfe, val);

		val = (U8)((tmp1 & 0x03) << 6);		// ci_divrange_h_0 bits[1:0]
		val |= tmp2;						// ci_divrange_l   bits[5:0]
		handle->dmd_set_reg(handle, 0xea, val);
	}
	else	// DVB-S
	{
		tmp1 -= 1;
		tmp2 -= 1;

		tmp1 &= 0x3f;
		tmp2 &= 0x3f;

		//handle->dmd_set_reg(handle, 0xfe, 0x20);	// default value

		handle->dmd_get_reg(handle, 0xfe, &val);		// ci_divrange_h_1 bits[5:2]
		val &= 0xF0;						// bits[3:0]
		val |= (tmp1 >> 2) & 0x0f;
		handle->dmd_set_reg(handle, 0xfe, val);

		val = (U8)((tmp1 & 0x03) << 6);		// ci_divrange_h_0 bits[1:0]
		val |= tmp2;						// ci_divrange_l   bits[5:0]
		handle->dmd_set_reg(handle, 0xea, val);
	}

	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_rs6060_set_clock_ratio
**
**
** Description:	set TS Clock Ratio
**
**
** Inputs:		None
**
**
** Outputs:		None
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_set_clock_ratio(MT_FE_RS6060_Device_Handle handle)
{
	U8  mod_fac, tmp1, tmp2, val;
	U32 input_datarate, locked_sym_rate_KSs;
	U32 MClk_KHz = 96000;
	U16 divid_ratio = 0;
	MT_FE_CHAN_INFO_DVBS2 p_info;

	MT_FE_TS_OUT_MODE ts_mode = MtFeTsOutMode_Common;

	locked_sym_rate_KSs = 0;

	_mt_fe_dmd_rs6060_get_sym_rate(handle, &locked_sym_rate_KSs);

	if (locked_sym_rate_KSs == 0)
	{
		return MtFeErr_Param;
	}

	handle->dmd_get_reg(handle, 0x9d, &val);
	val |= 0x08;
	handle->dmd_set_reg(handle, 0x9d, val);


	mt_fe_dmd_rs6060_get_channel_info(handle, &p_info);

	_mt_fe_dmd_rs6060_get_current_ts_mode(handle, &ts_mode);


	if (((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial == 0)) || 
		((ts_mode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel == 0)) || 
		((ts_mode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI == 0)))
	{
		return MtFeErr_Ok;
	}

	if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2)			// for dvb-s2
	{
		switch (p_info.mod_mode)
		{
			case MtFeModMode_8psk:
			case MtFeModMode_8Apsk_L:	mod_fac = 3; break;

			case MtFeModMode_16Apsk:
			case MtFeModMode_16Apsk_L:	mod_fac = 4; break;

			case MtFeModMode_32Apsk:
			case MtFeModMode_32Apsk_L:	mod_fac = 5; break;

			case MtFeModMode_64Apsk:
			case MtFeModMode_64Apsk_L:	mod_fac = 6; break;

			case MtFeModMode_128Apsk:
			case MtFeModMode_128Apsk_L:	mod_fac = 7; break;

			case MtFeModMode_256Apsk:
			case MtFeModMode_256Apsk_L:	mod_fac = 8; break;

			case MtFeModMode_Qpsk:
			default:					mod_fac = 2; break;
		}

		switch (p_info.code_rate)
		{
			case MtFeCodeRate_1_4:		input_datarate = locked_sym_rate_KSs*mod_fac/8/4;		break;
			case MtFeCodeRate_1_3:		input_datarate = locked_sym_rate_KSs*mod_fac/8/3;		break;
			case MtFeCodeRate_2_5:		input_datarate = locked_sym_rate_KSs*mod_fac*2/8/5;		break;
			case MtFeCodeRate_1_2:		input_datarate = locked_sym_rate_KSs*mod_fac/8/2;		break;
			case MtFeCodeRate_3_5:		input_datarate = locked_sym_rate_KSs*mod_fac*3/8/5;		break;
			case MtFeCodeRate_2_3:		input_datarate = locked_sym_rate_KSs*mod_fac*2/8/3;		break;
			case MtFeCodeRate_3_4:		input_datarate = locked_sym_rate_KSs*mod_fac*3/8/4;		break;
			case MtFeCodeRate_4_5:		input_datarate = locked_sym_rate_KSs*mod_fac*4/8/5;		break;
			case MtFeCodeRate_5_6:		input_datarate = locked_sym_rate_KSs*mod_fac*5/8/6;		break;
			case MtFeCodeRate_8_9:		input_datarate = locked_sym_rate_KSs*mod_fac*8/8/9;		break;
			case MtFeCodeRate_9_10:		input_datarate = locked_sym_rate_KSs*mod_fac*9/8/10;	break;
			case MtFeCodeRate_5_9:		input_datarate = locked_sym_rate_KSs*mod_fac*5/8/9;		break;
			case MtFeCodeRate_7_9:		input_datarate = locked_sym_rate_KSs*mod_fac*7/8/9;		break;
			case MtFeCodeRate_4_15:		input_datarate = locked_sym_rate_KSs*mod_fac*4/8/15;	break;
			case MtFeCodeRate_7_15:		input_datarate = locked_sym_rate_KSs*mod_fac*7/8/15;	break;
			case MtFeCodeRate_8_15:		input_datarate = locked_sym_rate_KSs*mod_fac*8/8/15;	break;
			case MtFeCodeRate_11_15:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/15;	break;
			case MtFeCodeRate_13_18:	input_datarate = locked_sym_rate_KSs*mod_fac*13/8/18;	break;
			case MtFeCodeRate_9_20:		input_datarate = locked_sym_rate_KSs*mod_fac*9/8/20;	break;
			case MtFeCodeRate_11_20:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/20;	break;
			case MtFeCodeRate_23_36:	input_datarate = locked_sym_rate_KSs*mod_fac*23/8/36;	break;
			case MtFeCodeRate_25_36:	input_datarate = locked_sym_rate_KSs*mod_fac*25/8/36;	break;
			case MtFeCodeRate_11_45:	input_datarate = locked_sym_rate_KSs*mod_fac*11/8/45;	break;
			case MtFeCodeRate_13_45:	input_datarate = locked_sym_rate_KSs*mod_fac*13/8/45;	break;
			case MtFeCodeRate_14_45:	input_datarate = locked_sym_rate_KSs*mod_fac*14/8/45;	break;
			case MtFeCodeRate_26_45:	input_datarate = locked_sym_rate_KSs*mod_fac*26/8/45;	break;
			case MtFeCodeRate_28_45:	input_datarate = locked_sym_rate_KSs*mod_fac*28/8/45;	break;
			case MtFeCodeRate_29_45:	input_datarate = locked_sym_rate_KSs*mod_fac*29/8/45;	break;
			case MtFeCodeRate_31_45:	input_datarate = locked_sym_rate_KSs*mod_fac*31/8/45;	break;
			case MtFeCodeRate_32_45:	input_datarate = locked_sym_rate_KSs*mod_fac*32/8/45;	break;
			case MtFeCodeRate_77_90:	input_datarate = locked_sym_rate_KSs*mod_fac*77/8/90;	break;
			default:					input_datarate = locked_sym_rate_KSs*mod_fac*2/8/3;		break;
		}

		_mt_fe_dmd_rs6060_get_ts_mclk(handle, &MClk_KHz);

		if (ts_mode == MtFeTsOutMode_Serial)
		{
			U32 target_mclk = MClk_KHz;

			//input_datarate *= 49;
			//input_datarate /= 5;			// input_datarate * 1.225 * 8 = input_datarate * 49 / 5

			input_datarate *= 8;

			target_mclk = input_datarate;

			_mt_fe_dmd_rs6060_select_xm(handle, &target_mclk, handle->tp_cfg.iFreqKHz / 1000);

			if (target_mclk != MClk_KHz)
			{
				handle->dmd_set_reg(handle, 0x06, 0xe0);
				_mt_fe_dmd_rs6060_set_ts_mclk(handle, target_mclk);
				handle->dmd_set_reg(handle, 0x06, 0x00);
			}

			_mt_fe_dmd_rs6060_get_ts_mclk(handle, &(handle->global_cfg.iSerialMclkHz));

			if (handle->global_cfg.iSerialMclkHz > 116000)
			{
				if (handle->ts_cfg.iSerialDriverMode == 1)
				{
					handle->dmd_set_reg(handle, 0x0a, 0xaa);
				}
				else
				{
					//handle->dmd_set_reg(handle, 0x0a, 0x55);
					handle->dmd_set_reg(handle, 0x0a, 0x01);
				}
			}
			else
			{
				if (handle->ts_cfg.iSerialDriverMode == 1)
				{
					handle->dmd_set_reg(handle, 0x0a, 0x55);
				}
				else
				{
					handle->dmd_set_reg(handle, 0x0a, 0x00);
				}
			}

			return MtFeErr_Ok;
		}
		else		// Parallel or CI mode
		{
			handle->global_cfg.iSerialMclkHz = input_datarate * 49 / 5;

			input_datarate = input_datarate * 105 / 100;

			if (handle->global_cfg.iSerialMclkHz > 115200)
			{
				handle->global_cfg.iSerialMclkHz = 144000;
			}
			else if (handle->global_cfg.iSerialMclkHz > 96000)
			{
				handle->global_cfg.iSerialMclkHz = 115200;
			}
			else if (handle->global_cfg.iSerialMclkHz > 72000)
			{
				handle->global_cfg.iSerialMclkHz = 96000;
			}
			else
			{
				handle->global_cfg.iSerialMclkHz = 72000;
			}

			// Set lower limit of TS output clock for Parallel and CI modes
			if (input_datarate < 6000)
			{
				input_datarate = 6000;
			}

			if (input_datarate != 0)
			{
				divid_ratio = (U16)(MClk_KHz / input_datarate);
			}
			else
			{
				divid_ratio = 0xFF;
			}

			//divid_ratio -= 1;

			//if (divid_ratio > 128)
			//	divid_ratio = 128;

			if (divid_ratio < 8)
				divid_ratio = 8;

			if (ts_mode == MtFeTsOutMode_Common)
			{
				if (divid_ratio > 27)
					divid_ratio = 27;

				if ((divid_ratio == 14) || (divid_ratio == 15))
					divid_ratio = 13;
				if ((divid_ratio == 19) || (divid_ratio == 20))
					divid_ratio = 18;
			}
			else
			{
				if (divid_ratio > 24)
					divid_ratio = 24;

				if ((divid_ratio == 12) || (divid_ratio == 13))
					divid_ratio = 11;
				if ((divid_ratio == 19) || (divid_ratio == 20))
					divid_ratio = 18;
			}

			tmp1 = (U8)(divid_ratio / 2);
			tmp2 = (U8)(divid_ratio / 2);

			if ((divid_ratio % 2) != 0)
				tmp2 += 1;
		}
	}
	else				// for dvb-s
	{
		mod_fac = 2;

		switch (p_info.code_rate)
		{
			case MtFeCodeRate_1_2:	input_datarate = locked_sym_rate_KSs*mod_fac/2/8;	break;
			case MtFeCodeRate_2_3:	input_datarate = locked_sym_rate_KSs*mod_fac*2/3/8;	break;
			case MtFeCodeRate_3_4:	input_datarate = locked_sym_rate_KSs*mod_fac*3/4/8;	break;
			case MtFeCodeRate_5_6:	input_datarate = locked_sym_rate_KSs*mod_fac*5/6/8;	break;
			case MtFeCodeRate_7_8:	input_datarate = locked_sym_rate_KSs*mod_fac*7/8/8;	break;
			default:				input_datarate = locked_sym_rate_KSs*mod_fac*3/4/8;	break;
		}


		_mt_fe_dmd_rs6060_get_ts_mclk(handle, &MClk_KHz);

		if (ts_mode == MtFeTsOutMode_Serial)
		{
			U32 target_mclk = MClk_KHz;

			//input_datarate *= 49;
			//input_datarate /= 5;			// input_datarate * 1.225 * 8 = input_datarate * 49 / 5

			input_datarate *= 8;

			target_mclk = input_datarate;

			_mt_fe_dmd_rs6060_select_xm(handle, &target_mclk, handle->tp_cfg.iFreqKHz / 1000);

			if (target_mclk != MClk_KHz)
			{
				handle->dmd_set_reg(handle, 0x06, 0xe0);
				_mt_fe_dmd_rs6060_set_ts_mclk(handle, target_mclk);
				handle->dmd_set_reg(handle, 0x06, 0x00);
			}

			_mt_fe_dmd_rs6060_get_ts_mclk(handle, &(handle->global_cfg.iSerialMclkHz));

			if (handle->global_cfg.iSerialMclkHz > 116000)
			{
				if (handle->ts_cfg.iSerialDriverMode == 1)
				{
					handle->dmd_set_reg(handle, 0x0a, 0xaa);
				}
				else
				{
					//handle->dmd_set_reg(handle, 0x0a, 0x55);
					handle->dmd_set_reg(handle, 0x0a, 0x01);
				}
			}
			else
			{
				if (handle->ts_cfg.iSerialDriverMode == 1)
				{
					handle->dmd_set_reg(handle, 0x0a, 0x55);
				}
				else
				{
					handle->dmd_set_reg(handle, 0x0a, 0x00);
				}
			}

			return MtFeErr_Ok;
		}
		else		// Parallel or CI mode
		{
			handle->global_cfg.iSerialMclkHz = input_datarate * 46 / 5;

			input_datarate = input_datarate * 105 / 100;

			if (handle->global_cfg.iSerialMclkHz > 72000)
			{
				handle->global_cfg.iSerialMclkHz = 96000;
			}
			else
			{
				handle->global_cfg.iSerialMclkHz = 72000;
			}

			// Set lower limit of TS output clock for Parallel and CI modes
			if (input_datarate < 6000)
			{
				input_datarate = 6000;
			}

			if (input_datarate != 0)
			{
				divid_ratio = (U16)(MClk_KHz / input_datarate);
			}
			else
			{
				divid_ratio = 0xFF;
			}

			//divid_ratio -= 1;

			//if (divid_ratio > 128)
			//	divid_ratio = 128;

			if (divid_ratio < 8)
				divid_ratio = 8;

			if (ts_mode == MtFeTsOutMode_Common)
			{
				if (divid_ratio > 27)
					divid_ratio = 27;
			}
			else
			{
				if (divid_ratio > 24)
					divid_ratio = 24;
			}

			tmp1 = (U8)(divid_ratio / 2);
			tmp2 = (U8)(divid_ratio / 2);

			if ((divid_ratio % 2) != 0)
				tmp2 += 1;
		}
	}


	_mt_fe_dmd_rs6060_set_ts_divide_ratio(handle, handle->tp_cfg.mCurrentType, tmp1, tmp2);


	return MtFeErr_Ok;
}


/******************************************************************
** Function: _mt_fe_dmd_rs6060_set_ts_out_mode
**
**
** Description:	set the TS output mode
**
**
** Inputs:
**
**	Parameter				Type		Description
**	----------------------------------------------------------
**	mode					MT_FE_TS_OUT_MODE	passed by up level
**
**	MtFeTsOutMode_Serial	1			Serial
**	MtFeTsOutMode_Parallel	2			Parallel
**	MtFeTsOutMode_Common	3			Common Interface
**
** Outputs:		none
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_set_ts_out_mode(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE mode)
{
	U8	tmp, val = 0;


	handle->dmd_get_reg(handle, 0x0b, &val);
	val &= ~0x01;
	handle->dmd_set_reg(handle, 0x0b, val);

	handle->dmd_get_reg(handle, 0xfd, &tmp);
	if (mode == MtFeTsOutMode_Parallel)
	{
		tmp &= ~0x01;
		tmp &= ~0x04;

		handle->dmd_set_reg(handle, 0xfa, 0x01);
		handle->dmd_set_reg(handle, 0xf1, 0x60);
		handle->dmd_set_reg(handle, 0xfa, 0x00);
	}
	else if (mode == MtFeTsOutMode_Serial)
	{
		tmp &= ~0x01;
		tmp |= 0x04;
	}
	else
	{
		tmp |= 0x01;
		tmp &= ~0x04;

		handle->dmd_set_reg(handle, 0xfa, 0x01);
		handle->dmd_set_reg(handle, 0xf1, 0x60);
		handle->dmd_set_reg(handle, 0xfa, 0x00);
	}

	if (handle->ts_cfg.bRisingEdge)
	{
		tmp &= ~0xf8;
		tmp |= 0x02;
	}
	else
	{
		tmp &= ~0xb8;
		tmp |= 0x42;
	}
	tmp |= 0x80;
	handle->dmd_set_reg(handle, 0xfd, tmp);


	val = 0;
	if (mode != MtFeTsOutMode_Serial)
	{
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0C;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xC0;
	}
	else
	{
		#if 0
		tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_SERIAL;

		val |= tmp & 0x03;
		val |= (tmp << 2) & 0x0C;
		val |= (tmp << 4) & 0x30;
		val |= (tmp << 6) & 0xC0;
		#endif

		val = 0x00;
	}
	handle->dmd_set_reg(handle, 0x0a, val);


	handle->dmd_get_reg(handle, 0x0b, &tmp);
	if (handle->ts_cfg.bPinSwitch)
	{
		tmp |= 0x20;
	}
	else
	{
		tmp &= ~0x20;
	}

	tmp |= 0x01;
	handle->dmd_set_reg(handle, 0x0b, tmp);


	handle->dmd_get_reg(handle, 0x0c, &tmp);
	if ((mode == MtFeTsOutMode_Serial) && handle->ts_cfg.b3SerialTsOut)
	{
		handle->dmd_set_reg(handle, 0xf4, 0x03);
		tmp |= 0x80;
	}
	else
	{
		handle->dmd_set_reg(handle, 0xf4, 0x01);
		tmp &= ~0x80;
	}
	handle->dmd_set_reg(handle, 0x0c, tmp);


	handle->ts_cfg.mTsMode = mode;


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


/*****************************************************************************
** Function: _mt_fe_dmd_rs6060_set_demod
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
MT_FE_RET _mt_fe_dmd_rs6060_set_demod(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	U8			val, tmp;

	ret = _mt_fe_dmd_rs6060_init_reg(handle, rs6060_reg_tbl_def, sizeof(rs6060_reg_tbl_def) / 2);

	if ((sym_rate_KSs > 47100) && (sym_rate_KSs < 47500))
	{
		handle->dmd_set_reg(handle, 0xe6, 0x00);
		handle->dmd_set_reg(handle, 0xe7, 0x03);
	}

	if (handle->board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val |= 0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}
	else
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val &= ~0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}

	handle->dmd_get_reg(handle, 0x08, &val);
	val &= 0x7f;
	handle->dmd_set_reg(handle, 0x08, val);

	handle->dmd_get_reg(handle, 0xc9, &tmp);
	tmp |= 0x08;
	handle->dmd_set_reg(handle, 0xc9, tmp);


	if (sym_rate_KSs <= 3000)
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 32 * 100 / 64 = 400
		handle->dmd_set_reg(handle, 0xc8, 0x20);
		handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		handle->dmd_set_reg(handle, 0xc7, 0x00);
	}
	else if (sym_rate_KSs <= 10000)
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 16 * 100 / 64 = 200
		handle->dmd_set_reg(handle, 0xc8, 0x10);
		handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		handle->dmd_set_reg(handle, 0xc7, 0x00);
	}
	else
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 6 * 100 / 64 = 75
		handle->dmd_set_reg(handle, 0xc8, 0x06);
		handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
		handle->dmd_set_reg(handle, 0xc7, 0x00);
	}

	_mt_fe_dmd_rs6060_set_sym_rate(handle, sym_rate_KSs);

	handle->dmd_get_reg(handle, 0x76, &val);
	val &= ~0x80;
	handle->dmd_set_reg(handle, 0x76, val);


	if (handle->tp_cfg.bHavePLS)
	{
		handle->dmd_set_reg(handle, 0x22, handle->tp_cfg.ucPLSCode[0]);
		handle->dmd_set_reg(handle, 0x23, handle->tp_cfg.ucPLSCode[1]);
		handle->dmd_set_reg(handle, 0x24, handle->tp_cfg.ucPLSCode[2]);
	}
	else
	{
		handle->dmd_set_reg(handle, 0x22, 0x01);
		handle->dmd_set_reg(handle, 0x23, 0x00);
		handle->dmd_set_reg(handle, 0x24, 0x00);
	}

	//handle->dmd_set_reg(handle, 0xf5, handle->tp_cfg.ucCurTsId);

#if 0
	if (sym_rate_KSs > 5000)
	{
		handle->dmd_set_reg(handle, 0x63, 0x60);
		handle->dmd_set_reg(handle, 0x64, 0x30);
		handle->dmd_set_reg(handle, 0x65, 0x40);
		handle->dmd_set_reg(handle, 0x68, 0x26);
		handle->dmd_set_reg(handle, 0x69, 0x4c);
	}
#endif

	return ret;
}


/*****************************************************************************
** Function: _mt_fe_dmd_rs6060_set_sym_rate
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
MT_FE_RET _mt_fe_dmd_rs6060_set_sym_rate(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs)
{
	U32	tmp;
	U8	reg_0x61, reg_0x62;

	tmp = ((sym_rate_KSs << 15) + (handle->global_cfg.iMclkKHz / 4)) / (handle->global_cfg.iMclkKHz / 2);

	reg_0x61 = (U8)(tmp & 0x00FF);
	reg_0x62 = (U8)((tmp & 0xFF00) >> 8);

	handle->dmd_set_reg(handle, 0x61, reg_0x61);
	handle->dmd_set_reg(handle, 0x62, reg_0x62);


	return MtFeErr_Ok;
}


/*****************************************************************************
** Function: _mt_fe_dmd_rs6060_get_sym_rate
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
MT_FE_RET _mt_fe_dmd_rs6060_get_sym_rate(MT_FE_RS6060_Device_Handle handle, U32 *sym_rate_KSs)
{
	U16	tmp;
	U32	sym_rate_tmp;
	U8	val_0x6d, val_0x6e;


	handle->dmd_get_reg(handle, 0x6d, &val_0x6d);
	handle->dmd_get_reg(handle, 0x6e, &val_0x6e);

	tmp = (U16)((val_0x6e << 8) | val_0x6d);


	sym_rate_tmp = tmp * handle->global_cfg.iMclkKHz;
	sym_rate_tmp /= (1 << 16);

	*sym_rate_KSs = sym_rate_tmp;


	handle->dmd_get_reg(handle, 0x00, &val_0x6d);

	return MtFeErr_Ok;
}


/*************************************************************************
** Function:  _mt_fe_dmd_rs6060_set_carrier_offset
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
MT_FE_RET _mt_fe_dmd_rs6060_set_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 carrier_offset_KHz)
{
	S32 tmp;

	tmp = carrier_offset_KHz;
	tmp *= 65536;

	tmp = (2 * tmp + handle->global_cfg.iMclkKHz) / (2 * handle->global_cfg.iMclkKHz);

	if (tmp < 0)
		tmp += 65536;


	handle->dmd_set_reg(handle, 0x5f, (U8)(tmp >> 8));
	handle->dmd_set_reg(handle, 0x5e, (U8)(tmp & 0xFF));

	handle->tp_cfg.iCarrierOffsetKHz = carrier_offset_KHz;

	return MtFeErr_Ok;
}


/***********************************************************************
** Function: _mt_fe_dmd_rs6060_get_carrier_offset
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
MT_FE_RET _mt_fe_dmd_rs6060_get_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 *carrier_offset_KHz)
{
	S32	val;
	U8	tmp, val_0x5e, val_0x5f;


	handle->dmd_get_reg(handle, 0x5d, &tmp);
	tmp &= 0xf8;
	handle->dmd_set_reg(handle, 0x5d, tmp);

	handle->dmd_get_reg(handle, 0x5e, &val_0x5e);
	handle->dmd_get_reg(handle, 0x5f, &val_0x5f);
	val = (val_0x5f << 8) | val_0x5e;


	if ((val & 0x8000) == 0x8000)
		val -= (1 << 16);


	*carrier_offset_KHz = (val * handle->global_cfg.iMclkKHz) / (1 << 16);


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


/***********************************************************************
** Function: _mt_fe_dmd_rs6060_get_total_carrier_offset
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
MT_FE_RET _mt_fe_dmd_rs6060_get_total_carrier_offset(MT_FE_RS6060_Device_Handle handle, S32 *carrier_offset_KHz)
{
	U8	tmp, val_0x5e, val_0x5f;
	S32	val_0x5e_0x5f_1, val_0x5e_0x5f_2, nval_1, nval_2;

	handle->dmd_get_reg(handle, 0x5d, &tmp);
	tmp &= 0xf8;
	handle->dmd_set_reg(handle, 0x5d, tmp);

	handle->dmd_get_reg(handle, 0x5e, &val_0x5e);
	handle->dmd_get_reg(handle, 0x5f, &val_0x5f);
	val_0x5e_0x5f_1 = (val_0x5f << 8) | val_0x5e;

	tmp |= 0x06;
	handle->dmd_set_reg(handle, 0x5d, tmp);

	handle->dmd_get_reg(handle, 0x5e, &val_0x5e);
	handle->dmd_get_reg(handle, 0x5f, &val_0x5f);
	val_0x5e_0x5f_2 = (val_0x5f << 8) | val_0x5e;


	if (((val_0x5e_0x5f_1 >> 15) & 0x01) == 0x01)
		nval_1 = val_0x5e_0x5f_1 - (1 << 16);
	else
		nval_1 = val_0x5e_0x5f_1;

	if (((val_0x5e_0x5f_2 >> 15) & 0x01) == 0x01)
		nval_2 = val_0x5e_0x5f_2 - (1 << 16);
	else
		nval_2 = val_0x5e_0x5f_2;

	*carrier_offset_KHz = (nval_1 - nval_2) * handle->global_cfg.iMclkKHz / (1 << 16);


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}



/*************************************************************************
** Function:  _mt_fe_dmd_rs6060_get_fec
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
MT_FE_RET _mt_fe_dmd_rs6060_get_fec(MT_FE_RS6060_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type)
{
	U8 tmp, val_0x7E;


	if (tp_type == MtFeType_DvbS2)
	{
		handle->dmd_get_reg(handle, 0x7e, &val_0x7E);
		tmp = (U8)(val_0x7E & 0x0f);
		switch (tmp)
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
		handle->dmd_get_reg(handle, 0xe6, &tmp);
		tmp = (U8)(tmp >> 5);
		switch (tmp)
		{
			case 0:  p_info->code_rate = MtFeCodeRate_7_8;		break;
			case 1:  p_info->code_rate = MtFeCodeRate_5_6;		break;
			case 2:  p_info->code_rate = MtFeCodeRate_3_4;		break;
			case 3:  p_info->code_rate = MtFeCodeRate_2_3;		break;
			case 4:  p_info->code_rate = MtFeCodeRate_1_2;		break;
			default: p_info->code_rate = MtFeCodeRate_Undef;	break;
		}
	}


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}



/*****************************************************************************
** Function: _mt_fe_dmd_rs6060_get_current_ts_mode
**
**
** Description:	get current TS output mode from register
**
**
** Inputs:	none
**
**
**
** Outputs:
**
**	Parameter					Type		Description
**	----------------------------------------------------------------------
**	p_ts_mode					U8*			Index of current TS output mode.
**		MtFeTsOutMode_Serial				1
**		MtFeTsOutMode_Parallel				2
**		MtFeTsOutMode_Common				3
**
**
*******************************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_get_current_ts_mode(MT_FE_RS6060_Device_Handle handle, MT_FE_TS_OUT_MODE *p_ts_mode)
{
	U8 val_0xfd;

	*p_ts_mode = MtFeTsOutMode_Common;

	handle->dmd_get_reg(handle, 0xfd, &val_0xfd);

	if ((val_0xfd & 0x01) == 0x00)
	{
		*p_ts_mode = ((val_0xfd & 0x04) == 0) ? MtFeTsOutMode_Parallel : MtFeTsOutMode_Serial;
	}
	else
	{
		*p_ts_mode = MtFeTsOutMode_Common;
	}

	handle->ts_cfg.mTsMode = *p_ts_mode;


	handle->dmd_get_reg(handle, 0x00, &val_0xfd);

	return MtFeErr_Ok;
}


/**********************************************************************
** Function: _mt_fe_dmd_rs6060_init_reg
**
**
** Description:	initialize the register of RS6060
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
MT_FE_RET _mt_fe_dmd_rs6060_init_reg(MT_FE_RS6060_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size)
{
	MT_FE_RET	ret;
	U32			i;

	for (i = 0; i < (U32)size; i++)
	{
		ret = handle->dmd_set_reg(handle, p_reg_tabl[i][0], p_reg_tabl[i][1]);

		if (ret != MtFeErr_Ok)
			return ret;
	}

	return MtFeErr_Ok;
}


/*******************************************************************
** Function: _mt_fe_dmd_rs6060_download_fw
**
**
** Description:	download the MCU code to RS6060
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
MT_FE_RET _mt_fe_dmd_rs6060_download_fw(MT_FE_RS6060_Device_Handle handle, const U8* p_fw, U16 fw_size)
{
	U32 i;
	MT_FE_RET ret = MtFeErr_Ok;

	handle->dmd_set_reg(handle, 0xb2, 0x01);

#if 0
	for (i = 0; i < 192; i++)
	{
		ret = handle->write_fw(handle, 0xb0, (U8*)&(p_fw[64 * i]), 64);
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
			ret = handle->write_fw(handle, 0xb0, (U8 *)&(p_fw[64 * i]), 64);
			fw_size -= 64;
		}
		else
		{
			ret = handle->write_fw(handle, 0xb0, (U8 *)&(p_fw[64 * i]), fw_size);
			fw_size = 0;
		}

		i++;

		if (ret != MtFeErr_Ok)
		{
			break;
		}
	}
#endif

	handle->dmd_set_reg(handle, 0xb2, 0x00);

	if (ret != MtFeErr_Ok)
		return MtFeErr_I2cErr;


	return MtFeErr_Ok;
}


/*****************************************************************************
** Function: _mt_fe_dmd_rs6060_bs_set_demod
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
MT_FE_RET _mt_fe_dmd_rs6060_bs_set_demod(MT_FE_RS6060_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type)
{
	U8			val;
	MT_FE_RET	ret = MtFeErr_Ok;

	ret = _mt_fe_dmd_rs6060_init_reg(handle, rs6060_reg_tbl_bs_def, sizeof(rs6060_reg_tbl_bs_def) / 2);

	if (handle->board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val |= 0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}
	else
	{
		handle->dmd_get_reg(handle, 0x4d, &val);
		val &= ~0x02;
		handle->dmd_set_reg(handle, 0x4d, val);
	}

	if (sym_rate_KSs <= 2500)
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08);
		handle->dmd_set_reg(handle, 0xc8, 0x02);
		handle->dmd_set_reg(handle, 0xc4, 0x04);
		handle->dmd_set_reg(handle, 0xc7, 0x0b);
	}
	else if (sym_rate_KSs <= 5000)
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08);
		handle->dmd_set_reg(handle, 0xc8, 0x02);
		handle->dmd_set_reg(handle, 0xc4, 0x04);
		handle->dmd_set_reg(handle, 0xc7, 0x0b);
	}
	else if (sym_rate_KSs <= 20000)
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08);
		handle->dmd_set_reg(handle, 0xc8, 0x02);
		handle->dmd_set_reg(handle, 0xc4, 0x04);
		handle->dmd_set_reg(handle, 0xc7, 0x16);
	}
	else
	{
		handle->dmd_set_reg(handle, 0xc3, 0x08);
		handle->dmd_set_reg(handle, 0xc8, 0x02);
		handle->dmd_set_reg(handle, 0xc4, 0x02);
		handle->dmd_set_reg(handle, 0xc7, 0x16);
	}

	_mt_fe_dmd_rs6060_set_sym_rate(handle, sym_rate_KSs);

	return ret;
}


/********************************************************************************************
** Function: _mt_fe_dmd_rs6060_bs_remove_unlocked_TP
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
MT_FE_RET _mt_fe_dmd_rs6060_bs_remove_unlocked_TP(MT_FE_RS6060_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz)
{
	U16 i;
	U16 scanned_TP_num, index = 0, move_steps;
	U32 sym_tmp;
	U32 fre_tmp;

	scanned_TP_num = handle->global_cfg.iScannedTpCnt;

	for (i = 0; i < scanned_TP_num; i++)
	{
		if (p_bs_info->p_tp_info[i].freq_KHz >= compare_freq_KHz)
		{
			index = i;
			break;
		}
	}


	if (i == scanned_TP_num)
		index = scanned_TP_num;


	i = 0;
	move_steps = 0;

	while (i <= index)
	{
		if (p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked)
		{
			i++;
			continue;
		}

		while ((p_bs_info->p_tp_info[i].dvb_type == MtFeType_DTV_Checked) && (i <= index))
		{
			move_steps++;
			i++;
		}

		while ((p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked) && (i <= index))
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

			i++;
		}
	}

	return MtFeErr_Ok;
}


/************************************************************************************
** Function: _mt_fe_dmd_rs6060_bs_connect
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
**		start_index		U16					the start TP index in memory
**		scanned_tp		U16					TP num need to try lock
**
** Outputs:
**
**		Parameter		type				description
**	---------------------------------------------------------------------------------
**		p_bs_info		MT_FE_BS_TP_INFO *	the blindscan TP pointer
**
**
*************************************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_bs_connect(MT_FE_RS6060_Device_Handle handle,
									 MT_FE_BS_TP_INFO *p_bs_info,
									 U16 start_index,
									 U16 scanned_tp)
{
	U16		index, timing_thr, crl_thr;
	U8		count, timing_fail, /*crl_fail, */tmp, same_locked_TP_flag;
	U32		sym_rate_KSs, centerfreq_KHz, truefreq_KHz;
	S16		lpf_offset_KHz, i;
	S32		carrieroffset_KHz, delta_freq_KHz, delta_smy_rate_KSs, deviceoffset_KHz;
	U32		target_mclk = 144000;//192000;//MT_FE_MCLK_KHZ;
	MT_FE_TS_OUT_MODE	ts_mode = MtFeTsOutMode_Common;


	MT_FE_RET		ret = MtFeErr_Ok;

	U8 val_reg08, ucTmp, tmp1;

	int iCnt;

	U32 real_freq_KHz;

	MT_FE_TYPE			current_mode = MtFeType_DvbS;
	MT_FE_LOCK_STATE	lockstate;
	MT_FE_TP_INFO 		*p_scanned_tp_info = (MT_FE_TP_INFO *)(p_bs_info->p_tp_info);


	if ((mt_fe_tn_set_tuner == NULL) || (mt_fe_tn_get_offset == NULL))
		return MtFeErr_NullPointer;

	for (index = start_index; index < scanned_tp; index++)
	{
		if (handle->global_cfg.bCancelBs) break;

		if (p_scanned_tp_info[index].dvb_type != MtFeType_DTV_Unknown)
			continue;

		p_scanned_tp_info[index].bCheckCCM = FALSE;
		p_scanned_tp_info[index].iTsCnt = 0;
		p_scanned_tp_info[index].ucCurTsId = 0;

		// fix 106074 start
		handle->tp_cfg.bCheckCCM = FALSE;
		handle->tp_cfg.iTsCnt = 0;
		handle->tp_cfg.ucCurTsId = 0;

		for (i = 0; i < 32; i++)
		{
			p_scanned_tp_info[index].ucTsId[i] = 0;
			handle->tp_cfg.ucTsId[i] = 0;
		}

		p_scanned_tp_info[index].bHavePLS = FALSE;
		handle->tp_cfg.bHavePLS = FALSE;
		for (i = 0; i < 3; i++)
		{
			p_scanned_tp_info[index].ucPLSCode[i] = 0;
			handle->tp_cfg.ucPLSCode[i] = 0;
		}

		centerfreq_KHz	 = p_scanned_tp_info[index].freq_KHz;
		sym_rate_KSs	 = p_scanned_tp_info[index].sym_rate_KSs;

		if (sym_rate_KSs > 49200)	sym_rate_KSs = 49200;

		handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;
		handle->tp_cfg.iSymRateKSs = sym_rate_KSs;

		lpf_offset_KHz = 0;
		truefreq_KHz   = 0;

		mt_fe_dmd_rs6060_global_reset(handle);

		mt_fe_dmd_rs6060_clear_ts(handle);

		handle->dmd_set_reg(handle, 0xb2, 0x01);
		handle->dmd_set_reg(handle, 0x00, 0x00);

		if ((sym_rate_KSs < 5000) && (handle->lnb_cfg.bUnicable == 0))
		{
			lpf_offset_KHz	  = 0;//BLINDSCAN_LPF_OFFSET_KHz;
			centerfreq_KHz	 += 0;//BLINDSCAN_LPF_OFFSET_KHz;
		}

		if (handle->global_cfg.bCancelBs) break;


		handle->dmd_set_reg(handle, 0x06, 0xe0);
		_mt_fe_dmd_rs6060_select_mclk(handle, handle->tp_cfg.iFreqKHz / 1000);
		_mt_fe_dmd_rs6060_set_ts_mclk(handle, target_mclk);
		handle->dmd_set_reg(handle, 0x06, 0x00);

		handle->mt_sleep(10);

		if (handle->lnb_cfg.bUnicable)
		{
			ret = mt_fe_dmd_rs6060_unicable_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
			deviceoffset_KHz = real_freq_KHz - centerfreq_KHz;
		}
		else
		{
			ret = mt_fe_tn_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
			real_freq_KHz = centerfreq_KHz;
			deviceoffset_KHz = 0;
		}

		if (ret != MtFeErr_Ok)		// Error parameter
		{
			lockstate = MtFeLockState_Unlocked;
			break;
		}

		_mt_fe_dmd_rs6060_bs_set_demod(handle, sym_rate_KSs, current_mode);

		handle->dmd_get_reg(handle, 0x08, &val_reg08);
		val_reg08 = (U8)(val_reg08 & 0x3b);
		handle->dmd_set_reg(handle, 0x08, val_reg08);
		handle->dmd_set_reg(handle, 0xe0, 0xf8);

		if (sym_rate_KSs <= 5000)
		{
			handle->dmd_set_reg(handle, 0xc0, 0x04);
			handle->dmd_set_reg(handle, 0x8a, 0x09);
			handle->dmd_set_reg(handle, 0x8b, 0x22);
			handle->dmd_set_reg(handle, 0x8c, 0x88);
		}

		_mt_fe_dmd_rs6060_set_carrier_offset(handle, mt_fe_tn_get_offset(handle) + lpf_offset_KHz + deviceoffset_KHz);

		handle->dmd_set_reg(handle, 0xb2, 0x00);

		handle->mt_sleep(70);

		lockstate = MtFeLockState_Waiting;

		if (sym_rate_KSs >= 10000)
		{
			count = 25;
			timing_thr = 1;
			crl_thr = 1;
		}
		else if (sym_rate_KSs >= 4000)
		{
			count = 28;
			timing_thr = 1;
			crl_thr = 1;
		}
		else if (sym_rate_KSs >= 2000)
		{
			count = 35;
			timing_thr = 1;
			crl_thr = 1;
		}
		else if (sym_rate_KSs >= 1500)
		{
			count = 45;
			timing_thr = 1;
			crl_thr = 1;
		}
		else
		{
			count = 55;//40;
			timing_thr = 1;
			crl_thr = 1;
		}

		timing_fail = 0;
		//crl_fail = 0;
		lockstate = MtFeLockState_Unlocked;

		while ((count > 0) && (!handle->global_cfg.bCancelBs))
		{
			handle->dmd_get_reg(handle, 0xbe, &tmp);
			if (tmp == 0xfe)
			{
				handle->dmd_set_reg(handle, 0xae, 0x4f);
				handle->dmd_set_reg(handle, 0x7f, 0xc4);
				handle->dmd_set_reg(handle, 0x24, 0xc0);

				handle->dmd_get_reg(handle, 0xca, &ucTmp);
				ucTmp |= 0x04;
				handle->dmd_set_reg(handle, 0xca, ucTmp);

				mt_fe_dmd_rs6060_soft_reset(handle);

				iCnt = 0;

				do
				{
					iCnt++;
					if (iCnt > 1000)
					{
						handle->dmd_set_reg(handle, 0x22, 0x01);
						handle->dmd_set_reg(handle, 0x23, 0x00);
						handle->dmd_set_reg(handle, 0x24, 0x00);

						mt_fe_dmd_rs6060_soft_reset(handle);

						handle->dmd_set_reg(handle, 0xae, 0x09);
						handle->dmd_set_reg(handle, 0x7f, 0x04);

						handle->dmd_get_reg(handle, 0xca, &ucTmp);
						ucTmp &= ~0x04;
						handle->dmd_set_reg(handle, 0xca, ucTmp);

						handle->dmd_get_reg(handle, 0x00, &tmp);

						return MtFeErr_TimeOut;
					}

					handle->dmd_get_reg(handle, 0x24, &tmp1);

					handle->mt_sleep(1);
				} while ((tmp1 & 0x08) == 0x00);

				handle->dmd_set_reg(handle, 0xae, 0x09);
				handle->dmd_set_reg(handle, 0x7f, 0x04);

				handle->dmd_get_reg(handle, 0xca, &ucTmp);
				ucTmp &= ~0x04;
				handle->dmd_set_reg(handle, 0xca, ucTmp);

				handle->dmd_get_reg(handle, 0x22, &tmp1);
				handle->dmd_set_reg(handle, 0x22, tmp1);
				p_scanned_tp_info[index].ucPLSCode[0] = tmp1;
				handle->tp_cfg.ucPLSCode[0] = tmp1;

				handle->dmd_get_reg(handle, 0x23, &tmp1);
				handle->dmd_set_reg(handle, 0x23, tmp1);
				p_scanned_tp_info[index].ucPLSCode[1] = tmp1;
				handle->tp_cfg.ucPLSCode[1] = tmp1;

				handle->dmd_get_reg(handle, 0x24, &tmp1);
				tmp1 &= 0x03;
				handle->dmd_set_reg(handle, 0x24, tmp1);
				p_scanned_tp_info[index].ucPLSCode[2] = tmp1;
				handle->tp_cfg.ucPLSCode[2] = tmp1;

				p_scanned_tp_info[index].bHavePLS = TRUE;
				handle->tp_cfg.bHavePLS = TRUE;

				mt_fe_dmd_rs6060_soft_reset(handle);
			}


			handle->dmd_get_reg(handle, 0x08, &tmp);
			if (tmp & 0x08)
			{
				current_mode = MtFeType_DvbS2;
				handle->tp_cfg.mCurrentType = MtFeType_DvbS2;

				handle->dmd_get_reg(handle, 0x7e, &ucTmp);

				if (handle->tp_cfg.bSuperSearch && ((ucTmp & 0x20) != 0x20))
				{
					U8 val = 0;

					//printk("%s[%d] ---- Start super blindscan!\n", __FUNCTION__, __LINE__);

					handle->dmd_get_reg(handle, 0x0d, &tmp);
					if ((tmp & 0x8f) == 0x0f)
					{
						U32 iPLSCnt = 0, iPLSIndex = 0;

						iPLSCnt = sizeof(mPLSGoldTable) / sizeof(U32);

						for (iPLSIndex = 0; iPLSIndex < iPLSCnt; iPLSIndex++)
						{
							U8 PLSCode[3] = {0x01, 0x00, 0x00};

							//mt_fe_dmd_rs6060_calc_PLS_gold_code(PLSCode, mPLSGoldTable[iPLSIndex]);
							mt_unf_fe_calc_PLS_gold_code(PLSCode, mPLSGoldTable[iPLSIndex]);

							handle->dmd_set_reg(handle, 0x22, PLSCode[0]);
							handle->dmd_set_reg(handle, 0x23, PLSCode[1]);
							handle->dmd_set_reg(handle, 0x24, PLSCode[2]);

							//mt_fe_dmd_rs6060_soft_reset(handle);

							handle->mt_sleep(100);

							handle->dmd_get_reg(handle, 0x0d, &val);
							if ((val & 0x8f) == 0x8f)
							{
								//printk("%s[%d] ---- Super blindscan OK, PLS Gold = %d!\n", __FUNCTION__, __LINE__, mPLSGoldTable[iPLSIndex]);

								handle->dmd_get_reg(handle, 0x22, &tmp1);
								//handle->dmd_set_reg(handle, 0x22, tmp1);
								p_scanned_tp_info[index].ucPLSCode[0] = tmp1;
								handle->tp_cfg.ucPLSCode[0] = tmp1;

								handle->dmd_get_reg(handle, 0x23, &tmp1);
								//handle->dmd_set_reg(handle, 0x23, tmp1);
								p_scanned_tp_info[index].ucPLSCode[1] = tmp1;
								handle->tp_cfg.ucPLSCode[1] = tmp1;

								handle->dmd_get_reg(handle, 0x24, &tmp1);
								tmp1 &= 0x03;
								//handle->dmd_set_reg(handle, 0x24, tmp1);
								p_scanned_tp_info[index].ucPLSCode[2] = tmp1;
								handle->tp_cfg.ucPLSCode[2] = tmp1;

								p_scanned_tp_info[index].bHavePLS = TRUE;
								handle->tp_cfg.bHavePLS = TRUE;

								break;
							}
						}

						if ((iPLSIndex == iPLSCnt) && ((val & 0x8f) != 0x8f))
						{
							handle->dmd_set_reg(handle, 0x22, 0x01);
							handle->dmd_set_reg(handle, 0x23, 0x00);
							handle->dmd_set_reg(handle, 0x24, 0x00);

							mt_fe_dmd_rs6060_soft_reset(handle);

#if 0
							handle->dmd_set_reg(handle, 0xae, 0x09);
							handle->dmd_set_reg(handle, 0x7f, 0x04);

							handle->dmd_get_reg(handle, 0xca, &ucTmp);
							ucTmp &= ~0x04;
							handle->dmd_set_reg(handle, 0xca, ucTmp);
#endif

							if (sym_rate_KSs >= 20000)
							{
								handle->mt_sleep(100);
							}
							else if (sym_rate_KSs >= 15000)
							{
								handle->mt_sleep(200);
							}
							else if (sym_rate_KSs >= 10000)
							{
								handle->mt_sleep(300);
							}
							else
							{
								handle->mt_sleep(400);
							}

							//printk("%s[%d] ---- Super blindscan failed\n", __FUNCTION__, __LINE__);
						}
					}
				}

				handle->dmd_get_reg(handle, 0x0d, &tmp);
				if ((tmp & 0x0f) == 0x0f)
				{
					handle->dmd_get_reg(handle, 0x89, &tmp);
					handle->dmd_get_reg(handle, 0xca, &ucTmp);
					tmp &= 0x80;
					handle->dmd_set_reg(handle, 0xca, (U8)((ucTmp & 0xf7) | (tmp >> 4) | 0x02));

					handle->dmd_get_reg(handle, 0x19, &tmp);
					if ((tmp != 0x04) && (tmp != 0x06) && (tmp != 0x08) && (tmp != 0x0a) && (tmp != 0x30) && (tmp != 0x32) && (tmp != 0x34) && (tmp != 0x36) 
					 && (tmp < 0x80))
					{
						handle->dmd_set_reg(handle, 0xbd, 0x82);
						handle->dmd_set_reg(handle, 0x80, 0x78);//0xa8
						handle->dmd_set_reg(handle, 0x81, 0x8a);//0xea
					}
					else
					{
						handle->dmd_set_reg(handle, 0xbd, 0x82);
						handle->dmd_set_reg(handle, 0x80, 0xa8);//0xa8
						handle->dmd_set_reg(handle, 0x81, 0xea);//0xea
					}

					handle->dmd_get_reg(handle, 0xbe, &ucTmp);
					if (ucTmp == 0xfe)
					{
						mt_fe_print(("\tBS -- PLS before checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay 50 ms!\n", ucTmp, tmp));

						handle->mt_sleep(50);

						continue;
					}

					if (!p_scanned_tp_info[index].bCheckCCM)
					{
						MT_BOOL bMIS = FALSE;

						bMIS = _mt_fe_dmd_rs6060_check_MIS(handle);

						if (bMIS)
						{
							int iTmpCnt = 0, iMaxCnt = 100;
							//handle->mt_sleep(10);		// Delay for MIS ready

							handle->dmd_set_reg(handle, 0xFA, 0x00);
							handle->dmd_set_reg(handle, 0xF0, 0x00);
							handle->dmd_set_reg(handle, 0xF0, 0x03);
							//handle->mt_sleep(500);

#if 1
							if (handle->tp_cfg.iSymRateKSs < 5000)
							{
								iMaxCnt = 40;
							}
							else if (handle->tp_cfg.iSymRateKSs < 10000)
							{
								iMaxCnt = 30;
							}
							else
							{
								iMaxCnt = 20;
							}

							do
							{
								handle->mt_sleep(50);
								handle->dmd_get_reg(handle, 0xbe, &ucTmp);

								iTmpCnt++;
							} while ((ucTmp != 0xfe) && (iTmpCnt < iMaxCnt)); // 50ms * 20 = 1000ms

							mt_fe_print(("\tPLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms! [%d/%d]\n", ucTmp, tmp, 50 * iTmpCnt, iTmpCnt, iMaxCnt));
#else
							do
							{
								handle->mt_sleep(50);
								handle->dmd_get_reg(handle, 0xbe, &ucTmp);

								iTmpCnt++;
							} while ((ucTmp != 0xfe) && (iTmpCnt < 20));		// 50ms * 20 = 1000ms

							mt_fe_print(("\tBS -- PLS after  checking MIS: [0xbe] = 0x%02x! [0x0d] = 0x%02x! Delay %d ms!\n", ucTmp, tmp, 50 * iTmpCnt));
#endif

							if (ucTmp == 0xfe)
							{
								continue;
							}

							handle->dmd_get_reg(handle, 0xf1, &ucTmp);
							p_scanned_tp_info[index].iTsCnt = ucTmp & 0x1F;
							handle->tp_cfg.iTsCnt = ucTmp & 0x1F;

							if (p_scanned_tp_info[index].iTsCnt >= 1)
							{
								mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));

								for (iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt++)
								{
									if (iCnt >= 32)
									{
										ret = MtFeErr_NoMemory;
										break;
									}

									ucTmp = (U8)iCnt;

									handle->dmd_set_reg(handle, 0xF2, ucTmp);
									handle->dmd_get_reg(handle, 0xF3, &ucTmp);

									p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;
									handle->tp_cfg.ucTsId[iCnt] = ucTmp;

									mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
								}

								handle->dmd_set_reg(handle, 0xF5, p_scanned_tp_info[index].ucTsId[0]);	// default select TS 0

								p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
								handle->tp_cfg.ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
								mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", p_scanned_tp_info[index].ucTsId[0]));
							}

							p_scanned_tp_info[index].bCheckCCM = TRUE;
							handle->tp_cfg.bCheckCCM = TRUE;
						}
					}

					lockstate = MtFeLockState_Locked;

					handle->dmd_set_reg(handle, 0xf9, 0x01);
				}
			}
			else
			{
				handle->dmd_set_reg(handle, 0xbd, 0x83);
				current_mode = MtFeType_DvbS;
				handle->tp_cfg.mCurrentType = MtFeType_DvbS;
				handle->dmd_get_reg(handle, 0x0d, &tmp);
				if ((tmp & 0xf7) == 0xf7)
					lockstate = MtFeLockState_Locked;
			}

			if (lockstate == MtFeLockState_Locked)
			{
				S32 carrier_offset;
				U8 reg_0x0c;

				handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
				if (((reg_0x0c & 0x04) == 0x00) && (handle->lnb_cfg.bUnicable == 0))
				{
					// AFC on
					_mt_fe_dmd_rs6060_get_total_carrier_offset(handle, &carrier_offset);
					if ((carrier_offset > ((S32)sym_rate_KSs)) || (carrier_offset < -((S32)sym_rate_KSs)))
					{
						mt_fe_tn_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
						real_freq_KHz = centerfreq_KHz;
						deviceoffset_KHz = 0;

						handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;

						handle->dmd_set_reg(handle, 0xb2, 0x01);

						_mt_fe_dmd_rs6060_bs_set_demod(handle, sym_rate_KSs, current_mode);

						if ((sym_rate_KSs <= 5000) && (current_mode == MtFeType_DvbS2))
						{
							handle->dmd_set_reg(handle, 0xc0, 0x04);
							handle->dmd_set_reg(handle, 0x8a, 0x09);
							handle->dmd_set_reg(handle, 0x8b, 0x22);
							handle->dmd_set_reg(handle, 0x8c, 0x88);
						}

						handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
						reg_0x0c |= 0x04;
						handle->dmd_set_reg(handle, 0x0c, reg_0x0c);


						_mt_fe_dmd_rs6060_set_carrier_offset(handle, mt_fe_tn_get_offset(handle) + lpf_offset_KHz + deviceoffset_KHz);

						handle->dmd_set_reg(handle, 0xb2, 0x00);

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
				handle->dmd_get_reg(handle, 0xb3, &tmp);
				if ((tmp & 0x80) == 0x80)
				{
					timing_fail++;
					if (timing_fail >= timing_thr)
					{
						timing_fail = 1;
						break;
					}
					mt_fe_dmd_rs6060_soft_reset(handle);
					count = 30;
				}
#if 0
				else if (((tmp & 0x40) == 0x40) && ((tmp & 0x80) != 0x80))
				{
					crl_fail++;
					if (crl_fail >= crl_thr)
					{
						crl_fail = 1;
						break;
					}
					mt_fe_dmd_rs6060_soft_reset(handle);
					count = 50;
				}
#endif
			}

			handle->mt_sleep(50);
			count--;
		}

		if (count == 0)
		{
			mt_fe_bs_notify(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			continue;
		}


		if (timing_fail == 1)
		{
			mt_fe_bs_notify(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			mt_fe_print(("timing fail!!!\n"));
			continue;
		}
#if 0
		else if (crl_fail == 1)
		{
			mt_fe_bs_notify(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			continue;
		}
#endif

		if (lockstate == MtFeLockState_Locked)
		{
			MT_FE_CHAN_INFO_DVBS2 ch_info;

			_mt_fe_dmd_rs6060_get_carrier_offset(handle, &carrieroffset_KHz);

			truefreq_KHz = real_freq_KHz + mt_fe_tn_get_offset(handle) - carrieroffset_KHz;

			_mt_fe_dmd_rs6060_get_sym_rate(handle, &sym_rate_KSs);

			same_locked_TP_flag = 0;
			for (i = (S16)(handle->global_cfg.iScannedTpCnt - 1); i >= 0; i--)
			{
				if (handle->global_cfg.bCancelBs) break;

				if (p_bs_info->p_tp_info[i].freq_KHz > truefreq_KHz)
					delta_freq_KHz = p_bs_info->p_tp_info[i].freq_KHz - truefreq_KHz;
				else
					delta_freq_KHz = truefreq_KHz - p_bs_info->p_tp_info[i].freq_KHz;

				if (p_bs_info->p_tp_info[i].sym_rate_KSs > sym_rate_KSs)
					delta_smy_rate_KSs = p_bs_info->p_tp_info[i].sym_rate_KSs - sym_rate_KSs;
				else
					delta_smy_rate_KSs = sym_rate_KSs - p_bs_info->p_tp_info[i].sym_rate_KSs;


				if (((delta_smy_rate_KSs < 50) && (delta_freq_KHz < (p_bs_info->p_tp_info[i].sym_rate_KSs / 2))) && 
					(p_bs_info->p_tp_info[i].dvb_type == current_mode))
				{
					same_locked_TP_flag = 1;
					p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;

					break;
				}
			}

			if (same_locked_TP_flag == 1)
				continue;


			handle->dmd_get_reg(handle, 0x19, &ucTmp);
			if (ucTmp < 0x80)
			{
				mt_fe_dmd_rs6060_get_channel_info(handle, &ch_info);

				if ((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x0c);
				}
				else if ((ch_info.code_rate == MtFeCodeRate_3_5) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x07);
				}
				else if ((ch_info.code_rate == MtFeCodeRate_2_3) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x09);
				}
				else if ((ch_info.code_rate == MtFeCodeRate_5_6) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x12);
				}
				else if ((ch_info.code_rate == MtFeCodeRate_8_9) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x18);
				}
				else if ((ch_info.code_rate == MtFeCodeRate_9_10) && (ch_info.mod_mode == MtFeModMode_8psk))
				{
					handle->dmd_set_reg(handle, 0x9f, 0x19);
				}

				if (ch_info.mod_mode == MtFeModMode_8psk)
				{
					handle->dmd_set_reg(handle, 0x9e, 0xff);

					handle->dmd_get_reg(handle, 0x9d, &tmp);
					tmp &= 0xfe;
					handle->dmd_set_reg(handle, 0x9d, tmp);
				}
			}

			if (sym_rate_KSs > 990)
			{
				handle->global_cfg.iLockedTpCnt++;
				p_bs_info->p_tp_info[index].freq_KHz = truefreq_KHz;

				if (sym_rate_KSs < 1000)
					sym_rate_KSs = 1000;

				p_bs_info->p_tp_info[index].sym_rate_KSs = sym_rate_KSs;
				p_bs_info->p_tp_info[index].dvb_type = current_mode;
				_mt_fe_dmd_rs6060_get_fec(handle, &(p_bs_info->p_tp_info[index]), current_mode);

				p_bs_info->tp_num = handle->global_cfg.iLockedTpCnt;

#if 1
				if (!handle->ts_cfg.bParallelClkFixed)
				{
					if (((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
						((ts_mode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
						((ts_mode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
					{
						_mt_fe_dmd_rs6060_set_clock_ratio(handle);
					}
				}
				else
				{
					if ((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0))
					{
						_mt_fe_dmd_rs6060_set_clock_ratio(handle);
					}
				}
#else

#if (MT_FE_PARALLEL_CLK_OUTPUT_FIXED_24MHZ == 0)
				if (((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0)) || 
					((ts_mode == MtFeTsOutMode_Parallel) && (handle->ts_cfg.bAutoParallel != 0)) || 
					((ts_mode == MtFeTsOutMode_Common) && (handle->ts_cfg.bAutoCI != 0)))
#else
				if ((ts_mode == MtFeTsOutMode_Serial) && (handle->ts_cfg.bAutoSerial != 0))
#endif
				{
					_mt_fe_dmd_rs6060_set_clock_ratio(handle);
				}
#endif

				mt_fe_bs_notify(MtFeMsg_BSTpLocked, &(p_bs_info->p_tp_info[index]));
			}
		}
		else
		{
			p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
			mt_fe_bs_notify(MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
		}
	}

	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_rs6060_unicable_set_tuner(MT_FE_RS6060_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32* real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz)
{
	U32 tmp_freq_MHz;
	MT_FE_DiSEqC_MSG msg;
	U16 T_chan;

	if (handle->lnb_cfg.iUBVer == 2)	// 2.0
	{
		tmp_freq_MHz = freq_KHz / 1000;

		if (tmp_freq_MHz < 100)
			tmp_freq_MHz = 100;

		T_chan = tmp_freq_MHz - 100;

		if (T_chan > 2047)
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


		if (ub_freq_MHz == 0)
			ub_freq_MHz = handle->lnb_cfg.iUBList[ub_select];

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


	mt_fe_dmd_rs6060_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
	handle->mt_sleep(15);

	mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(handle, &msg);

	handle->mt_sleep(1);
	mt_fe_dmd_rs6060_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);


	mt_fe_tn_set_tuner(handle, ub_freq_MHz * 1000, symbol_rate_KSs, 0);

	handle->tp_cfg.iFreqKHz = ub_freq_MHz * 1000;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_rs6060_unicable_retry(MT_FE_RS6060_Device_Handle handle)
{
	U32 tmp_freq_MHz;
	MT_FE_DiSEqC_MSG msg;
	U16 T_chan;

	if (handle->lnb_cfg.bUnicable)
	{
		U32 freq_KHz = handle->tp_cfg.iFreqKHz;
		//U32 symbol_rate_KSs = handle->tp_cfg.iSymRateKSs;
		U8  ub_select = handle->lnb_cfg.iUBIndex;
		U8  bank_select = handle->lnb_cfg.iBankIndex;
		U32 ub_freq_MHz = handle->lnb_cfg.iUBFreqMHz;

		if (handle->lnb_cfg.iUBVer == 2)	// 2.0
		{
			tmp_freq_MHz = freq_KHz / 1000;

			if (tmp_freq_MHz < 100)
				tmp_freq_MHz = 100;

			T_chan = tmp_freq_MHz - 100;

			if (T_chan > 2047)
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


			if (ub_freq_MHz == 0)
				ub_freq_MHz = handle->lnb_cfg.iUBList[ub_select];

			tmp_freq_MHz = T_chan + 100;

			//*real_freq_KHz = tmp_freq_MHz * 1000;
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

			//*real_freq_KHz = tmp_freq_MHz * 1000;
		}


		mt_fe_dmd_rs6060_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
		handle->mt_sleep(15);

		mt_fe_dmd_rs6060_DiSEqC_send_receive_msg(handle, &msg);

		handle->mt_sleep(1);
		mt_fe_dmd_rs6060_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);
	}

	return MtFeErr_Ok;
}


/*************************************************************/
MT_FE_RET _mt_fe_dmd_rs6060_select_mclk_psd(MT_FE_RS6060_Device_Handle handle)
{
	mt_fe_tn_dmpll_select_mclk_rs6060(handle, handle->tp_cfg.iFreqKHz / 1000, TRUE);

	if (handle->global_cfg.iMclkKHz == 93000)
		handle->dmd_set_reg(handle, 0xa0, 0x42);
	else if (handle->global_cfg.iMclkKHz == 96000)
		handle->dmd_set_reg(handle, 0xa0, 0x44);
	else if (handle->global_cfg.iMclkKHz == 99000)
		handle->dmd_set_reg(handle, 0xa0, 0x46);
	else if (handle->global_cfg.iMclkKHz == 110250)
		handle->dmd_set_reg(handle, 0xa0, 0x4e);
	else
		handle->dmd_set_reg(handle, 0xa0, 0x44);


	return MtFeErr_Ok;
}


/***************          Blind scan test        ******************************/

MT_FE_RET _mt_fe_dmd_rs6060_bs_set_reg_psd(MT_FE_RS6060_Device_Handle handle)
{
	U8 tmp, sm_buf;

	sm_buf = 2;

	handle->dmd_set_reg(handle, 0xB2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);

	handle->dmd_set_reg(handle, 0x4A, 0x00);

	handle->dmd_get_reg(handle, 0x4D, &tmp);
	tmp |= 0x91;
	handle->dmd_set_reg(handle, 0x4D, tmp);

	handle->dmd_get_reg(handle, 0x90, &tmp);
	tmp |= TMP1;
	handle->dmd_set_reg(handle, 0x90, tmp);

	tmp = 0x42;
	handle->dmd_set_reg(handle, 0x91, tmp);

	tmp = (U8)(TMP3 + sm_buf - 1);
	//tmp |= 0x80;
	tmp &= ~0x80;
	handle->dmd_set_reg(handle, 0x92, tmp);


	tmp = TUNER_SLIP_STEP * 16 + SNR_THR_ONE_LOOP / 16 - 1;
	//tmp = tmp | 0x0f;
	handle->dmd_set_reg(handle, 0x97, tmp);

	tmp = FLAT_THR_ONE_LOOP;
	//tmp = tmp | 0x3f;
	handle->dmd_set_reg(handle, 0x99, tmp);

	handle->dmd_set_reg(handle, 0x4B, 0x04);
	handle->dmd_set_reg(handle, 0x56, 0x01);
	handle->dmd_set_reg(handle, 0xA0, 0x44);
	handle->dmd_set_reg(handle, 0x08, 0x83);
	handle->dmd_set_reg(handle, 0x70, 0x00);

	if (handle->board_cfg.bSpectrumInverted)			// 0x4d, bit 1
	{
		handle->dmd_get_reg(handle, 0x4d, &tmp);
		tmp |= 0x02;
		handle->dmd_set_reg(handle, 0x4d, tmp);
	}
	else
	{
		handle->dmd_get_reg(handle, 0x4d, &tmp);
		tmp &= ~0x02;
		handle->dmd_set_reg(handle, 0x4d, tmp);
	}

	handle->dmd_set_reg(handle, 0x30, 0x88);

	handle->dmd_set_reg(handle, 0x00, 0x00);
	handle->dmd_set_reg(handle, 0xB2, 0x00);

	return MtFeErr_Ok;
}


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

__attribute__((unused)) static void strmcp(char *des, char *sour, U8 m)
{
	U8 n = 0;

	char *descpy;
	descpy = des;

	for (n = 0; n < m; n++)
		sour++;

	while ((*sour) != '\0')
	{
		*descpy = *sour;
		descpy++;
		sour++;
	}
}

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
	for (i = start_index; i <= end_index; i++)
	{
		if (sour_value[i] < data)
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

	for (i = start_index; i <= end_index; i++)
	{
		if (sour_value[i] > data)
		{
			data = sour_value[i];
			index = i;
		}
	}

	*max_value = data;

	*max_index = (index - start_index + 1);
}

__attribute__((unused)) static void mt_fe_dmd_bs_find_Mean(U32 *sour_value, U32 *mean_value, U32 find_value, U32 start_index, U32 end_index)
{
	U32 add_value = 0;
	U32 i = 0, index = 0;

	for (i = start_index; i <= end_index; i++)
	{
		if (sour_value[i] >= find_value)
		{
			add_value = add_value + sour_value[i];
			index++;
		}
	}

	if (index != 0)
		*mean_value = add_value / index;
	else
	{
		//sprintf(log_str, "\tmt_fe_dmd_bs_find_Mean() ---- index = 0!\n");
		//mt_fe_dbg_log2file(log_str);
	}
}

static U32 _mt_fe_dmd_bs_adjust_small_symbol_rate(U32 symbol_rate)
{
	U32 new_sym_rate = symbol_rate;

	if (symbol_rate < 1650)
		new_sym_rate = symbol_rate - 250;

	if (symbol_rate < 1000)
		new_sym_rate = 1000;

	//printk("%s[%d] ---- org_symbol_rate = [%d], adjusted to [%d]\n", __FUNCTION__, __LINE__, symbol_rate, new_sym_rate);	

	return new_sym_rate;	
}

static void mt_fe_dmd_bs_ch_measure(MT_FE_RS6060_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 *psd , U32 idx_ps_sn)
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

	while (1)
	{
		count++;
		if (count > 15)
		{
			mt_fe_print(("all in  while1 !\n"));

			break;
		}

		sig_level = 0;
		sig_idx = 0;

		if (rep_flag == 0)
		{
			for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++)
			{
				sig_level = sig_level + psd[idx_tmp];
				sig_idx = sig_idx + 1;
			}

			if (sig_idx != 0)
				sig_thr = sig_level / sig_idx;
			else
			{
				sig_thr = 1;
			}

			sig_level = 0;
			sig_idx = 0;

			for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++)
			{
				if (psd[idx_tmp] >= sig_thr)
				{
					sig_level = sig_level + psd[idx_tmp];
					sig_idx = sig_idx + 1;
				}
			}

			if (sig_idx != 0)
				sig_level = (2 * sig_level / sig_idx + 1) / 2;
			else
			{
				sig_level = 1;

				//sig_level = psd[idx_notch_last - 1];
				mt_fe_print(("sig_level error !\n"));
			}
		}
		else
		{
			for (idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp++)
			{
				sig_level = sig_level + psd[idx_tmp];
				sig_idx = sig_idx + 1;
			}

			if (sig_idx != 0)
				sig_thr = sig_level / sig_idx;
			else
			{
				sig_thr = 1;
			}

			sig_level = 0;
			sig_idx = 0;

			for (idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp++)
			{
				if (psd[idx_tmp] >= sig_thr)
				{
					sig_level = sig_level + psd[idx_tmp];
					sig_idx = sig_idx + 1;
				}
			}

			if (sig_idx != 0)
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
		for (idx_tmp = idx_ch_max; idx_tmp <= (idx_notch - 1); idx_tmp++)
		{
			if ((psd[idx_tmp + 1] <= ch_thr_R) && (psd[idx_tmp] > ch_thr_R))
			{
				idx_dw = idx_tmp;
				if (sub_TP_spilt_mode)
					break;
			}
		}

		//% locate the rising edge thr crossing  point
		for (idx_tmp = idx_ch_max; idx_tmp >= idx_notch_last; idx_tmp--)
		{
			if ((psd[idx_tmp + 1] > ch_thr_L) && (psd[idx_tmp] <= ch_thr_L))
			{
				idx_up = idx_tmp;
				if (sub_TP_spilt_mode)
					break;
			}
		}

		//%-----------------refine the symbol rate-----------------------------------
		if (idx_dw != 1)
		{
			//idx_dw = idx_dw + mt_fe_min(1, (psd[idx_dw] - ch_thr_R) / (psd[idx_dw] - psd[idx_dw + 1]));
			if ((psd[idx_dw] - ch_thr_R) >= ((psd[idx_dw] - psd[idx_dw + 1])))
			{
				idx_dw = idx_dw + 1;
				idx_dwd = idx_dw * 100;
			}
			else
			{
				if (psd[idx_dw] != psd[idx_dw + 1])
					idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / (psd[idx_dw] - psd[idx_dw + 1]) + idx_dw * 200 + 1) / 2;
				else
					idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / 1 + idx_dw * 200 + 1) / 2;
			}
		}

		if (idx_up != 1)
		{
			//idx_up = idx_up - mt_fe_min(1, (ch_thr_L - psd[idx_up]) / (psd[idx_up] - psd[idx_up + 1]));
			if ((ch_thr_L - psd[idx_up]) >= ((psd[idx_up + 1] - psd[idx_up])))
			{
				//idx_up = idx_up;// -1;
				idx_upd = idx_up * 100;// -1;
			}
			else
			{
				if (psd[idx_up + 1] != psd[idx_up])
					idx_upd = (200 * (ch_thr_L - psd[idx_up]) / (psd[idx_up + 1] - psd[idx_up]) + idx_up * 200 + 1) / 2;
				else
					idx_upd = (200 * (ch_thr_L - psd[idx_up]) / 1 + idx_up * 200 + 1) / 2;
			}
		}

		tmp = 0;
		if ((idx_dw > idx_up) && (idx_notch > idx_notch_last))
		{
			if ((2 * (idx_dw - idx_up)) < (idx_notch - idx_notch_last))
				tmp = 1;
		}
		else if ((idx_dw < idx_up) && (idx_notch < idx_notch_last))
		{
			if ((2 * (idx_up - idx_dw)) < ( idx_notch_last - idx_notch))
				tmp = 1;
		}

		if ((tmp != 0) && (rep_flag == 0) && (sub_TP_spilt_mode == 0))
		// if (idx_dw - idx_up) / (idx_notch - idx_notch_last) < 1 / 2 && rep_flag == 0
		{
			if ((5 * idx_up) > idx_dw)
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

			if ((5 * idx_dw) > idx_up)
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
			if (idx_dwd > idx_upd)
				SymRate_raw = (idx_dwd - idx_upd) * Fs * 10 / FFT_N;
			else
				SymRate_raw = (idx_upd - idx_dwd) * Fs * 10 / FFT_N;

			if (SymRate_raw > 60000)
				mt_fe_print(("Error symbol rate %d\n", SymRate_raw));

			//CtFreq = (idx_dw / 2 + idx_up / 2 - (FFT_N - 2 * OVLP) / 2 + tuner_offset) * Fs / FFT_N + ct_freq;
			if ((idx_dwd + idx_upd + 200 * OVLP + 200 * tuner_offset) >= (100 * FFT_N))
				CtFreq_raw = ((10 * idx_dwd + 10 * idx_upd + 2000 * OVLP + 2000 * tuner_offset - 1000 * FFT_N) * Fs / (2 * FFT_N) + ct_freq);
			else
				CtFreq_raw = (ct_freq - (1000 * FFT_N - 10 * idx_dwd - 10 * idx_upd - 2000 * OVLP - 2000 * tuner_offset) * Fs / (2 * FFT_N));

			//SNR = 10 * log10(sig_level^2 / noise_level^2 - 1);

			#if 0
			//SNR = 10 * log10(((double)sig_level * (double)sig_level) / (noise_level * noise_level) - 1);
			if (((double)sig_level * (double)sig_level) > 3.512 * (ch_noise_lev * ch_noise_lev))//SNR_filt_thr = 4 so 2.512
			//if (((double)sig_level * (double)sig_level) > 1.5 * (noise_level * noise_level))
				SNR = 1.0;
			else
				SNR = 0.0;
			#endif

			if (ch_noise_lev != 0)
				SNR = (sig_level * sig_level) / ch_noise_lev * 1000 / ch_noise_lev;
			else
				SNR = (sig_level * sig_level) / 1 * 1000 / 1;

			#if 0
			win_tmp = ((idx_dwd - idx_upd) * 0.1 + 0.5);
			idx_up = (U32)(idx_upd + 0.5);
			idx_dw = (U32)(idx_dwd + 0.5);

			if ((idx_up - win_tmp) > idx_notch_last)
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

			if ((idx_dw + win_tmp) < idx_notch)
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
				if (SNR > SNR_filt_thr)
				{
					idx_notch_last_tureCh = idx_notch;
				}

				if (SymRate_raw > SR_X_thr)
				{
					ch_thr_high = (sig_level * (5 + K_high) + ch_thr_L * (5 - K_high) + 5) / 10;
					mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_notch_last, idx_ch_max);
					ch_thr_low = (ch_thr_L * (5 - K_low) + val * (5 + K_low) + 5) / 10;

					for (idx_tmp = idx_ch_max; idx_tmp >= idx_up; idx_tmp--)
					{
						idx_L_high = idx_tmp;
						if ((psd[idx_tmp] <= ch_thr_high) && (psd[idx_tmp + 1] > ch_thr_high))
							break;
					}

					for (idx_tmp = idx_up; idx_tmp >= idx_notch_last; idx_tmp--)
					{
						idx_L_low = idx_tmp;
						if ((psd[idx_tmp] <= ch_thr_low) && (psd[idx_tmp + 1] > ch_thr_low))
							break;
					}

					if (idx_L_high <= idx_L_low)
						slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) + 1) / 2;
					else
						slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) / (idx_L_high - idx_L_low) + 1) / 2;

					//  idx_start_fix = idx_up - round((ch_thr_L - noise_level) / slpoe_L);
					if (slpoe_L == 0)
						slpoe_L = 1;

					if (idx_upd > ((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L))
						idx_start_fix = idx_upd - (((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L) + 50) / 100 * 100;///100;
					else
						idx_start_fix = 0;

					ch_thr_high = (sig_level * (5 + K_high) + ch_thr_R * (5 - K_high) + 5) / 10;
					mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_ch_max, idx_notch);

					//ch_thr_low = ch_thr_L * (0.5 - K_low) + min(psd(idx_ch_max : idx_notch)) * (0.5 + K_low);
					ch_thr_low = (ch_thr_R * (5 - K_low) + val * (5 + K_low) + 5) / 10;

					for (idx_tmp = idx_ch_max; idx_tmp <= idx_dw; idx_tmp++)
					{
						idx_R_high = idx_tmp;
						if ((psd[idx_tmp + 1] <= ch_thr_high) && (psd[idx_tmp] > ch_thr_high))
							break;
					}

					for (idx_tmp = idx_dw; idx_tmp <= idx_notch; idx_tmp++)
					{
						idx_R_low = idx_tmp;
						if ((psd[idx_tmp + 1] <= ch_thr_low) && (psd[idx_tmp] > ch_thr_low))
							break;
					}

					if (idx_R_low <= idx_R_high)
						slpoe_R = (2 * (10 * ch_thr_high - 10 * ch_thr_low) + 1) / 2;
					else
						slpoe_R = (2 * (10 * ch_thr_high - 10 * ch_thr_low) / (idx_R_low - idx_R_high) + 1) / 2;

					if (slpoe_R == 0)
						slpoe_R = 1;

					idx_end_fix = ((1000 * (ch_thr_R - noise_level) / slpoe_R) + 50) / 100 * 100 + idx_dwd;///100;

					//idx_start_fix = max(idx_start_fix, idx_notch_last);
					//idx_end_fix = min(idx_end_fix, idx_notch);
					if (idx_start_fix < (100 * idx_notch_last))
						idx_start_fix =	100 * idx_notch_last;

					if (idx_end_fix > (100 * idx_notch))
						idx_end_fix = 100 * idx_notch;

					if (idx_end_fix > idx_start_fix)
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

					if (sub_TP_spilt_mode)
						;
					else
					{
						if (SymRate_fix > SymRate_raw)
							idx_dw_tmp = idx_dw + (((SymRate_fix - SymRate_raw) * FFT_N) + Fs) / (2 * Fs * 1000);
						else
							idx_dw_tmp = idx_dw - (((SymRate_raw - SymRate_fix) * FFT_N) + Fs) / (2 * Fs * 1000);

						if ((CtFreq + SymRate / 2) > ct_freq)
							idx_dw_tmp = 100 * FFT_N / 2 - 100 * OVLP + 100 * tuner_offset + (((CtFreq + SymRate / 2 - ct_freq) * FFT_N) + 500 * Fs) / (10 * Fs);
						else
							idx_dw_tmp = 100 * FFT_N / 2 - 100 * OVLP + 100 * tuner_offset - (((ct_freq - CtFreq - SymRate / 2) * FFT_N) + 500 * Fs) / (10 * Fs);

						if (idx_dw_tmp > (100 * idx_notch))
							idx_dw_tmp = idx_notch * 100;
					}
					//if (SymRate_fix > SymRate_raw)
					//	idx_dw_tmp = idx_dw + (((SymRate_fix - SymRate_raw) * FFT_N) + Fs) / (2 * Fs * 1000);
					//else
					//	idx_dw_tmp = idx_dw - (((SymRate_raw - SymRate_fix ) * FFT_N) + Fs) / (2 * Fs * 1000);
					//if (sub_TP_spilt_mode == 0)
					//idx_dw_tmp = min(idx_notch, idx_dw + round((SymRate_fix - SymRate_raw) / 2 / Fs * FFT_N));
					// idx_dw_tmp = min(idx_notch, round((CtFreq + SymRate / 2  - ct_freq) / Fs * FFT_N) + FFT_N / 2 - OVLP + tuner_offset);
				}
				else
				{
					SymRate = SymRate_raw;
					CtFreq = CtFreq_raw;
					if (sub_TP_spilt_mode == 0)
						idx_dw_tmp = idx_dw * 100;
				}

				if (SNR > SNR_filt_thr)
				{
					if (handle->global_cfg.iScannedTpCnt > 100)
					{
						mt_fe_print(("global_cfg.iScannedTpCnt > 100!\n"));

						//handle->mt_sleep(20000);
					}

					SymRate = _mt_fe_dmd_bs_adjust_small_symbol_rate(SymRate);

					#if 1
					//handle->global_cfg.iScannedTpCnt++;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = CtFreq;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;

					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
					handle->global_cfg.iScannedTpCnt++;
					#endif

					tp_info.freq_KHz     = CtFreq;
					tp_info.sym_rate_KSs = SymRate;
					tp_info.uidx_notch = idx_notch;
					tp_info.uidx_notch_last = idx_notch_last;
					tp_info.uidx_up = idx_up;
					tp_info.upsd_sn = idx_ps_sn;
					mt_fe_bs_notify(MtFeMsg_BSTpFind, &tp_info);
				}

				if (sub_TP_spilt_mode == 0)
				{
					//if (idx_dw_last > 0)
					{
						//if ((S32)idx_up < idx_dw_last)
						//mt_fe_print(("merge err idx_up = %d, idx_dw_last = %d\r\n", idx_up, idx_dw_last));
					}

					if (((((S32)idx_upd - idx_dw_last) * 10 * Fs / FFT_N) < (S32)(SymRate_last + SymRate) * (MERGE_THR - 100) / 200) && (SymRate_last > 3000) && (SymRate_raw > 3000)
						&& ((SymRate_last + SymRate) < SR_max ) && ((S32)(100 * idx_up) >= idx_dw_last))
					{
						if (((SNR_last > SNR) && (((SNR - 1000) * 4) >= (SNR_last - 1000)) && (SNR_last > SNR_filt_thr)) || ((SNR_last < SNR) && (((SNR_last - 1000) * 4) >= (SNR - 1000)) && (SNR > SNR_filt_thr)))
						{
							#if 1
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = (CtFreq_raw + CtFreq_last) / 2;
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = _mt_fe_dmd_bs_adjust_small_symbol_rate(SymRate_last + SymRate);
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
							p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
							handle->global_cfg.iScannedTpCnt++;
							#endif

							tp_info.freq_KHz     = (CtFreq_raw + CtFreq_last) / 2;
							tp_info.sym_rate_KSs = _mt_fe_dmd_bs_adjust_small_symbol_rate(SymRate_last + SymRate);
							tp_info.uidx_notch_last = idx_notch_last;
							tp_info.uidx_up = idx_up;
							tp_info.uidx_notch = idx_notch;
							tp_info.upsd_sn = idx_ps_sn;
							mt_fe_bs_notify(MtFeMsg_BSTpFind, &tp_info);
						}
					}
					else
					{
						cap_value0 = psd[idx_notch_last] - psd[idx_notch];
						cap_value1 = psd[idx_ch_max] - psd[idx_notch];
						if ((1000 * cap_value0) > (707 * cap_value1))
						//if (((psd[idx_notch_last] - psd[idx_notch]) * 0.707) > (psd[idx_ch_max] - psd[idx_notch]))
						{
							if (handle->global_cfg.iScannedTpCnt > 1)
							{
								if (p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 2].upsd_sn == p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 1].upsd_sn)
								{
									if (p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 2].uidx_notch == p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 1].uidx_notch_last)
									{
										idx_up_merge = p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 2].uidx_up;
										idx_dw_merge = idx_dw;

										if (idx_dw_merge >= idx_up_merge)
											SymRate_merge = (idx_dw_merge - idx_up_merge) * Fs * 1000 / FFT_N;
										else
											SymRate_merge = (idx_up_merge - idx_dw_merge) * Fs * 1000 / FFT_N;

										if ((idx_up_merge + idx_dw_merge + 2 * tuner_offset) >= (FFT_N - 2 * OVLP))
											CtFreq_merge = 1000 * (idx_up_merge + idx_dw_merge + 2 * tuner_offset - (FFT_N - 2 * OVLP)) * Fs / FFT_N / 2 + ct_freq;
										else
											CtFreq_merge = ct_freq - 1000 * ((FFT_N - 2 * OVLP) - idx_up_merge - idx_dw_merge - 2 * tuner_offset) * Fs / FFT_N / 2;

										SymRate_merge = _mt_fe_dmd_bs_adjust_small_symbol_rate(SymRate_merge);

										#if 1
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = CtFreq_merge;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate_merge;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
										p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
										handle->global_cfg.iScannedTpCnt++;
										#endif

										tp_info.freq_KHz     = CtFreq_merge;
										tp_info.sym_rate_KSs = SymRate_merge;
										tp_info.uidx_notch_last = idx_notch_last;
										tp_info.uidx_up = idx_up;
										tp_info.uidx_notch = idx_notch;
										tp_info.upsd_sn = idx_ps_sn;
										mt_fe_bs_notify(MtFeMsg_BSTpFind, &tp_info);
									}
								}
							}
						}
					}
				}

				//if (handle->global_cfg.iScannedTpCnt > 0)
				if (sub_TP_spilt_mode == 0)
				{
					CtFreq_last = CtFreq;//CtFreq_raw;
					SymRate_last = SymRate;//SymRate_raw;
					idx_dw_last = idx_dw_tmp;
					SNR_last = SNR;
				}

				//handle->global_cfg.iScannedTpCnt++;
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

				for (idx_tmp = idx_up + 1; idx_tmp <= idx_ch_max; idx_tmp++)
				{
					if (psd[idx_tmp] < ch_thr_L)
					{
						all_under_thr_idx[all_thr_idx] = idx_tmp;
						all_thr_idx = all_thr_idx + 1;
						if (all_thr_idx >= 20)
							break;
					}
				}

				for (idx_tmp = idx_ch_max + 1; idx_tmp <= idx_dw; idx_tmp++)
				{
					if (psd[idx_tmp] < ch_thr_R)
					{
						all_under_thr_idx[all_thr_idx] = idx_tmp;
						all_thr_idx = all_thr_idx + 1;
						if (all_thr_idx >= 20)
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

					if (all_thr_idx > 20)
					{
						all_thr_idx = 20;
						mt_fe_print(("all_thr_idx > 20 !\n"));
					}

					for (idx = 0; idx < all_thr_idx; idx++)
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

					for (idx = 0; idx < ii - 1; idx++)
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

					if (sub_TP_N > MAX_sub_TP_N)
					{
						mt_fe_print(("sub_TP_N too large %d!!!\n", sub_TP_N));
						sub_TP_N = MAX_sub_TP_N;
						idx_sub_TP = 0;
						sub_TP_spilt_mode = 1;
						notch_all[0] = idx_notch_last;

						for (i = 1; i < MAX_sub_TP_N; i++)
						{
							notch_all[i] = idx_notch_split[i - 1];
						}

						notch_all[sub_TP_N] = idx_notch;
					}
					else if (sub_TP_N == 1)
					{
						break;
					}
					else
					{
						idx_sub_TP = 0;
						sub_TP_spilt_mode = 1;
						notch_all[0] = idx_notch_last;

						for (i = 1; i <= ii - 1; i++)
						{
							notch_all[i] = idx_notch_split[i - 1];
						}

						notch_all[sub_TP_N] = idx_notch;
						if (idx_notch == 0)
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
				if (idx_notch == 0)
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

MT_FE_RET _mt_fe_dmd_rs6060_bs_process_get_range_list(MT_FE_RS6060_Device_Handle handle,U32 begin_freq_MHz, U32 end_freq_MHz, U32 *range_list,U8 *list_num)
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
		handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;

#if FIND_DEBUG
		sprintf(file_name1, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H":"L",
				 (handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);

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
		_mt_fe_dmd_rs6060_bs_set_reg_psd(handle);
		handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
		handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

		if (handle->lnb_cfg.bUnicable)
		{
			mt_fe_dmd_rs6060_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
			freq_KHz = real_freq_KHz;
		}
		else
		{
			mt_fe_tn_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
		}

		handle->dmd_set_reg(handle, 0x90, 0x70);

		for (i = 0; i < 2; i++)
		{
			b_fftdone		 = MtFe_False;
			b_analog_locked	 = MtFe_False;
			cnt = 50;
			do
			{
				handle->dmd_get_reg(handle, 0x0d, &tmp);
				b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
				handle->mt_sleep(10);
				cnt--;
			} while ((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

			if (cnt == 0)
				break;
		}

		//clock6 = clock();
		// mt_fe_print(("agc lock time %d\n", clock6 - clock7));
		mt_fe_dmd_rs6060_get_strength(handle, &dwTmp1);
#endif

		F_grid_map_tmp[F_idx] = dwTmp1;
		sig_stre_grid = dwTmp1 + sig_stre_grid;

		if (min_sig > dwTmp1)
			min_sig = dwTmp1;

		if (max_sig <= dwTmp1)
			max_sig = dwTmp1;

		F_idx++;
		set_freq = set_freq + F_step;
	}

	fre_seg_idx = 0;
	if (min_sig > weak_sig_thr)
	{
		range_list[2 * fre_seg_idx] = begin_freq_MHz;
		range_list[2 * fre_seg_idx + 1] = end_freq_MHz;
		fre_seg_idx++;
	}
	else
	{
		if (F_idx == 0)
			F_idx = 1;

		sig_stre_thr = sig_stre_grid / F_idx;
		// sig_stre_thr = alpha * max(sig_stre_grid) + (1 - alpha) * min(sig_stre_grid);
		sig_stre_thr = (max_sig + min_sig) / 2;
		sig_stre_grid = 0;
		map_idx = 0;

		for (i = 0; i < F_idx; i++)
		{
			if (F_grid_map_tmp[i] >= sig_stre_thr)
			{
				sig_stre_grid = F_grid_map_tmp[i] + sig_stre_grid;
				map_idx++;
			}
		}

		//fprintf(pFilePPG, "\n");

		if (map_idx == 0)
			ave_sig_str = MAX_sig_stre_dif;
		else
			ave_sig_str = sig_stre_grid / map_idx;

		if ((ave_sig_str - MAX_sig_stre_dif) < sig_thr_low_bound)
			ave_sig_str = sig_thr_low_bound;
		else
			ave_sig_str = ave_sig_str - MAX_sig_stre_dif;

		if (ave_sig_str > sig_stre_thr)
			ave_sig_str = sig_stre_thr;

		cnt = 0;
		fre_seg_idx = 0;
		find_ok = 0;
		ct_freq_actual = begin_freq_MHz;

		for (i = 0; i < F_idx; )
		{
			if (F_grid_map_tmp[i] < ave_sig_str)
			{
				cnt++;
			}
			else
			{
				if (cnt <= 48 / F_step)
				{
					for (j = 0; j <= cnt; j++)
					{
						F_grid_map_tmp[i - j] = ave_sig_str + 2;
					}

					cnt = 0;
				}
			}

			i++;
		}

		if (F_grid_map_tmp[0] >= ave_sig_str)
			range_list[0] = ct_freq_actual;

		for (i = 1; i < F_idx; )
		{
			if ((F_grid_map_tmp[i - 1] < ave_sig_str) && (F_grid_map_tmp[i] >= ave_sig_str))
			{
				range_list[2 * fre_seg_idx] = ct_freq_actual + F_step * (i - 1);
			}
			else if ((F_grid_map_tmp[i - 1] >= ave_sig_str) && (F_grid_map_tmp[i] < ave_sig_str))
			{
				if (range_list[2 * fre_seg_idx] == 0)
					range_list[2 * fre_seg_idx] = ct_freq_actual;

				range_list[2 * fre_seg_idx + 1] = ct_freq_actual + F_step * (i);

				if (end_freq_MHz - range_list[2 * fre_seg_idx + 1] <= F_step)
					range_list[2 * fre_seg_idx + 1] = end_freq_MHz;

				fre_seg_idx++;
			}
			else if ((i == (F_idx - 1)) && (F_grid_map_tmp[i] >= ave_sig_str))
			{
				if (range_list[2 * fre_seg_idx] == 0)
					range_list[2 * fre_seg_idx] = ct_freq_actual;

				range_list[2 * fre_seg_idx + 1] = ct_freq_actual + F_step * i;

				if (end_freq_MHz - range_list[2 * fre_seg_idx + 1] <= F_step)
					range_list[2 * fre_seg_idx + 1] = end_freq_MHz;

				fre_seg_idx++;
			}

			i++;
		}
	}

	if (fre_seg_idx > 1)
	{
		for (i = 1; i < fre_seg_idx; i++)
		{
			if ((range_list[2 * i] - F_step) <= range_list[2 * (i - 1) + 1])
			{
				range_list[2 * (i - 1) + 1] = range_list[2 * i + 1];
				fre_seg_idx = fre_seg_idx - 1;

				for (j = i; j < fre_seg_idx; j++)
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

MT_FE_RET _mt_fe_dmd_rs6060_bs_process_scanned_TP_soft(MT_FE_RS6060_Device_Handle handle, U32 *psd, U32 cur_scan_freq_KHz, U8 bs_times, S8 *strength)
{
	MT_FE_BOOL	b_analog_locked, b_fftdone;

	U8	tmp, cnt;
	U16	totaltpnum;
	U8	tmp1, tmp2, tmp3, tmp4;
	S8	iStrength = 0;
	U32	nSize = (FFT_N - 16 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	//U8	buffer[968] = {0};						// sm = 2;
	//U32	dwPSDArray[FFT_N + 10];
	U32	i = 0, j = 0;
	U32	dwTmp = 0;
	//S32	diff[FFT_N + 2];
	U32	tone_idx = 0;
	S32	spur_thr = 5000;
	U32	mean_tone = 0;
	U32	nCount = 0;
	MT_FE_RET ret = 0;

	/*Clean warning larger than 2048 bytes [-Wframe-larger-than=]*/
	U8 *buffer = (U8 *)kmalloc(968 * sizeof(U8), GFP_KERNEL);
	U32 *dwPSDArray = (U32 *)kmalloc((FFT_N + 10) * sizeof(U32), GFP_KERNEL);
	S32 *diff = (S32 *)kmalloc((FFT_N + 2) * sizeof(S32), GFP_KERNEL);

	if (buffer == NULL)
	{
		printk("%s[%d] malloc buffer FAILED!\n", __FUNCTION__, __LINE__);
		ret = MtFeErr_NoMemory;
		goto END;
	}

	if (dwPSDArray == NULL)
	{
		printk("%s[%d] malloc dwPSDArray FAILED!\n", __FUNCTION__, __LINE__);
		ret = MtFeErr_NoMemory;
		goto FREE_BUFFER;
	}

	if (diff == NULL)
	{
		printk("%s[%d] malloc diff FAILED!\n", __FUNCTION__, __LINE__);
		ret = MtFeErr_NoMemory;
		goto FREE_DWPSDARRAY;
	}

#if PSD_WRITE
	FILE	*fpOut;
	S32 gain_val = 0;

	char	file_name[100];

	sprintf(file_name, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H":"L",
			 (handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);
#endif


	handle->dmd_set_reg(handle, 0x90, 0x70);

	for (i = 0; i < 2; i++)
	{
		b_fftdone		= MtFe_False;
		b_analog_locked = MtFe_False;

		cnt = 50;
		do
		{
			handle->dmd_get_reg(handle, 0x0d, &tmp);
			b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
			handle->mt_sleep(10);
			cnt--;
		} while ((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

		if (cnt == 0)
		{
			mt_fe_print(("agc time over 500 ms unlock, cnt = %d\n", cnt));
			break;
		}

		handle->dmd_set_reg(handle, 0x9a, 0x80);

		cnt = 100;//50
		do
		{
			handle->dmd_get_reg(handle, 0x9a, &tmp);
			b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
			handle->mt_sleep(10);
			cnt--;
		} while ((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

		if (b_fftdone || (1 == i))
		{
			break;
		}
		else
		{
			handle->dmd_set_reg(handle, 0x5f, 0x00);
			handle->dmd_set_reg(handle, 0x5e, 0x70);
		}
	}

	if (!(b_fftdone && b_analog_locked))
	{
		mt_fe_print(("agc unlock = %d or b_fftdone undone = %d !\n", b_analog_locked, b_fftdone));

		//return 0;
		ret = MtFeErr_Ok;
		goto FREE_DIFF;
	}


#if (PSD_WRITE)
	fpOut = fopen(file_name, "w+");

	handle->tn_get_reg(handle, 0x5A, &tmp);
	fprintf(fpOut, "RF Gain: %d\n", tmp & 0x0f);

	handle->tn_get_reg(handle, 0x5F, &tmp);
	fprintf(fpOut, "IF Gain: %d\n", tmp & 0x0f);

	handle->tn_get_reg(handle, 0x77, &tmp);
	fprintf(fpOut, "BB Gain: %d\n", (tmp >> 4) & 0x0f);

	handle->dmd_get_reg(handle, 0x3F, &tmp);
	fprintf(fpOut, "PWM: %d\n", tmp);

	//mt_fe_tn_get_gain(handle, &gain_val);
	gain_val = 0;
	fprintf(fpOut, "Tuner Gain: %d\n", gain_val);

	//mt_fe_tn_get_gain(handle, &gain_val);
	mt_fe_dmd_rs6060_get_strength(handle, &iStrength);
	fprintf(fpOut, "Strength: %d\n", iStrength);
	*strength = iStrength;
	fclose(fpOut);
#else

	mt_fe_dmd_rs6060_get_strength(handle,&iStrength);
	//fprintf(fpOut, "Strength: %d\n", iStrength);
	*strength = iStrength;
#endif

	handle->dmd_get_reg(handle, 0x9a, &tmp);
	totaltpnum = (U16)(tmp & 0x1F);

	nCount = 1;

	handle->dmd_set_reg(handle, 0x9A, 0x40);

	for (j = 0; j < nSize; j++)
	{
		handle->dmd_get_reg(handle, 0x9B, &tmp);
		buffer[j] = tmp;
	}


#if 1
	dwTmp = FFT_N - 478;
	for (i = 0; i < dwTmp; i++)
	{
		psd[nCount++] = 0;
	}

	for (i = 2 * 2; i < nSize; i += 2)			// sm_buf = 2
	{
		dwTmp = buffer[i];
		dwTmp = dwTmp + (buffer[i + 1] * 256);
		dwPSDArray[(i - 2) / 2] = dwTmp;
		psd[nCount++] = dwTmp;
	}

	dwTmp = FFT_N - 482 + 4;
	for (i = dwTmp + 1; i < nCount; i++)
	{
		diff[i]= psd[i + 1] - psd[i];
	}

	for (i = dwTmp + 1; i < nCount - 1; )
	{
		diff[i] = diff[i + 1] - diff[i];
		if (diff[i] < (0 - spur_thr * 2))
		{
			if ((i > (dwTmp + 2)) && (i < (nCount - 4)))
			{
				tone_idx++;
				diff[i] = 1;
			}
			else
				diff[i] = 0;
		}
		else
			diff[i] = 0;

		i++;
	}

	if (tone_idx > 0)
	{
		for (i = dwTmp + 1; i < nCount - 2; i++)
		{
			if (diff[i] == 1) ////cha 1
			{
				if (psd[i + 4] > psd[i - 2])
				{
					mean_tone = psd[i + 4] - psd[i - 2];//(psd[i - 2] + psd[i - 1] + psd[i] + psd[i + 1] + psd[i + 2] + psd[i + 3] + psd[i + 4]) / 6;
					for (j = 0; j < 5; j++)
						psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];

					mean_tone = 0;
				}
				else
				{
					mean_tone = psd[i - 2] - psd[i + 4];//(psd[i - 2] + psd[i - 1] + psd[i] + psd[i + 1] + psd[i + 2] + psd[i + 3] + psd[i + 4]) / 6;
					for (j = 0; j < 5; j++)
						psd[i - 1 + j] = psd[i - 2] - (mean_tone * (j + 1) + 3) / 6;
					mean_tone = 0;
				}
			}
		}
	}
#endif

	nCount = nCount - 1;
	for (j = nCount; j >= 1; j--)
	{
		dwTmp = 0;
		for (i = 0; i < SM_WIN; i++)
		{
			if (j >= i)
				dwTmp = dwTmp + psd[j - i];
		}
		psd[j] = (dwTmp + SM_WIN / 2) / SM_WIN;
	}

	j = FFT_N - 2 * OVLP;
	if ((nCount) > j)
	{
		dwTmp = OVLP + SM_WIN / 2 + tuner_offset;
		for (i = 1; i <= j; i++)
		{
			psd[i] = psd[i + dwTmp];
		}
	}


#if PSD_WRITE
	fpOut = fopen(file_name, "a");
	if (fpOut != NULL)
	{
		for (i = 1; i < nCount; i++)
		{
			fprintf(fpOut, "%d\n", psd[i]);//dwPSDArray[i]);
		}
		fclose(fpOut);
	}
#endif

	nCount = j;
	handle->mt_sleep(50);

	handle->dmd_set_reg(handle, 0x9A, 0x20);

	handle->dmd_get_reg(handle, 0x9B, &tmp1);
	handle->dmd_get_reg(handle, 0x9B, &tmp2);
	handle->dmd_get_reg(handle, 0x9B, &tmp3);
	handle->dmd_get_reg(handle, 0x9B, &tmp4);

	ret = MtFeErr_Ok;

FREE_DIFF:
	if(diff)
		kfree(diff);

FREE_DWPSDARRAY:
	if(dwPSDArray)
		kfree(dwPSDArray);

FREE_BUFFER:
	if(buffer)
		kfree(buffer);

END:
	return ret;
}

MT_FE_RET _mt_fe_dmd_rs6060_bs_A_soft(MT_FE_RS6060_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U8 bs_times)
{
	//global gain_RF  gain_IF gain_BB gain_PWM gain_tuner sig_stre path_com PSD_AVE_N;
	//load FcFs_Tab.mat;
	//PSD_SRC = 'External1';//%using captured PSD for simulation
	//U32 psd[FFT_N + 32];
	/*Clean warning larger than 2048 bytes [-Wframe-larger-than=]*/
	U32 *psd = (U32 *)kmalloc((FFT_N + 32) * sizeof(U32), GFP_KERNEL);
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
	U32 ct_freq_start = 920;//950;
	U32 ct_freq_end = 2180;//2150;
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

	//	if (mt_fe_tn_set_tuner == NULL)
	//	return MtFeErr_NullPointer;
	U32 F_range_list[18 * 2]; //16*2
	U8  fre_seg_idx = 0;
	U8  i = 0;
	U32 j = 0;
	U32 idx_tmp = 0, idx_pwr_rg_start = 0, idx_pwr_rg_end = 0;
	U8  flag_scan_done = 0;
	U32 sig_thr = 0;
	U32 sig_level = 0;
	U32 sig_idx = 0;
	U32 nCount = 0;
	//////form gloab
	U32 ct_freq_actual = 0;
	U8  same_freq = 0;
	///////////

	if (psd == NULL)
	{
		printk("%s[%d] malloc psd FAILED!\n", __FUNCTION__, __LINE__);
		ret = MtFeErr_NoMemory;
		goto END;
	}

	tmp_tp_scanned = 0;
	//ct_freq_start = begin_freq_MHz;
	if (begin_freq_MHz > 30)
		ct_freq_start = begin_freq_MHz - 30;
	else
		ct_freq_start = begin_freq_MHz;
	ct_freq = ct_freq_start * 1000;
	//ct_freq_end = end_freq_MHz - 2;
	ct_freq_end = end_freq_MHz + 30;
	ct_freq_actual = ct_freq;
	tuner_offset = 0;

	//memset(F_range_list,0,36*sizeof(U32));
	//////////////////////////////
	//clock4 = clock();
	//_mt_fe_dmd_rs6060_bs_process_get_range_list(handle, begin_freq_MHz, end_freq_MHz - 2, F_range_list, &fre_seg_idx);
	//clock5 = clock();
	//mt_fe_print(("get strength time %d\n", clock5 - clock4));

	fre_seg_idx = 1;
	//F_range_list[0] = begin_freq_MHz;
	if (begin_freq_MHz > 30)
		F_range_list[0] = begin_freq_MHz - 30;
	else
		F_range_list[0] = begin_freq_MHz;
	//F_range_list[1] = end_freq_MHz;
	F_range_list[1] = end_freq_MHz + 30;
	///////////////////////////////////
	#if 0
	for (i = 0; i < fre_seg_idx; i++)
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

		//while ((ct_freq <= ct_freq_end) && (!handle->global_cfg.bCancelBs))
		while ((flag_scan_done == 0) && (!handle->global_cfg.bCancelBs))
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

			U8 tmp;

			// load ([path_com 'ChTb_ref.mat']);
			//////////////add b wbj///////////////
			freq_KHz = ct_freq;

			#if FIND_DEBUG
			#else
			_mt_fe_dmd_rs6060_bs_set_reg_psd(handle);
			#endif

			handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
			handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

#if 1
			handle->dmd_set_reg(handle, 0x06, 0xe0);
			_mt_fe_dmd_rs6060_select_mclk_psd(handle);
			_mt_fe_dmd_rs6060_set_ts_mclk(handle, 96000);
			handle->dmd_set_reg(handle, 0x06, 0x00);
#endif
			handle->mt_sleep(10);

#if FIND_DEBUG
#else
			if (handle->lnb_cfg.bUnicable)
			{
				mt_fe_dmd_rs6060_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
				freq_KHz = real_freq_KHz;
			}
			else
			{
				mt_fe_tn_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
			}

			_mt_fe_dmd_rs6060_set_carrier_offset(handle, mt_fe_tn_get_offset(handle));

			ret = _mt_fe_dmd_rs6060_bs_process_scanned_TP_soft(handle, psd, freq_KHz, bs_times, &sig_stre);
#endif

			handle->dmd_get_reg(handle, 0x00, &tmp);

#if FIND_DEBUG
			sprintf(file_name, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H":"L", (
				handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);

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

				for (i = 0; i < dwTmp; i++)
				{
					psd[nCount++] = 0;
				}

				while (!feof(fpOut))
				{
					fscanf(fpOut,"%d\n", &dwTmp);
					psd[nCount++] = dwTmp;
				}
				fclose(fpOut);
			}
#if 1
			dwTmp = FFT_N - 482 + 4;
			for (i = dwTmp + 1; i < nCount; i++)
			{
				diff[i] = psd[i + 1] - psd[i];
			}

			for (i = dwTmp + 1; i < nCount - 1; )
			{
				diff[i]= diff[i + 1] - diff[i];
				if (diff[i] < (0 - spur_thr * 2))
				{
					if ((i > (dwTmp + 2)) && (i < (nCount - 4)))
					{
						tone_idx++;
						diff[i] = 1;
					}
					else
						diff[i] = 0;
				}
				else
					diff[i] = 0;

				i++;
			}

			if (tone_idx > 0)
			{
				for (i = dwTmp + 1; i < nCount - 2; i++)
				{
					if (diff[i] == 1) ////cha 1
					{
						if (psd[i + 4] > psd[i - 2])
						{
							mean_tone = psd[i + 4] - psd[i - 2];
							for (j = 0; j < 5; j++)
								psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];
							mean_tone = 0;
						}
						else
						{
							mean_tone = psd[i - 2] - psd[i + 4];
							for (j = 0; j < 5; j++)
								psd[i - 1 + j] = psd[i - 2] - (mean_tone * (j + 1) + 3) / 6;
							mean_tone = 0;
						}
					}
				}
			}
#endif	// #if 1

			nCount = nCount - 1;
			for (j = nCount; j >= 1; j--)
			{
				dwTmp = 0;
				for (i = 0; i < SM_WIN; i++)
				{
					dwTmp = dwTmp + psd[j - i];
				}

				psd[j] = (dwTmp + SM_WIN / 2) / SM_WIN;
			}

			j = FFT_N - 2 * OVLP;
			if ((nCount) > j)
			{
				if (tuner_offset > 0)
					dwTmp = OVLP + (SM_WIN + 1) / 2 + tuner_offset;
				else
					dwTmp = OVLP + (SM_WIN + 1) / 2 - (U8)tuner_offset;
				for (i = 1; i <= j; i++)
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

			if (psd_offset > 9)
				mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, (psd_offset - 8), (psd_offset + 8));
			else
				mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, 1, (psd_offset + 8));

			while (idx_notch_last < 64)
			{
				if (psd[idx_notch_last + 1] <= psd[idx_notch_last])
					idx_notch_last = idx_notch_last + 1;
				else
					break;
			}

			idx_notch = idx_notch_last;
			idx_psd_start = idx_notch_last;
			idx_start = idx_notch;
			idx_curr = idx_notch;
			ch_noise_lev = psd[idx_notch_last];

			while (idx_curr <= FFT_N - 2 * OVLP)
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
						if (psd[idx_notch] >= ch_min)
						{
							if (ch_noise_lev > ch_min)
								ch_noise_lev = ch_min;
						}
						else
						{
							if (ch_noise_lev > psd[idx_notch])
								ch_noise_lev = psd[idx_notch];
						}

						if (psd[idx_notch] < ch_noise_lev)
						{
							ch_noise_lev = psd[idx_notch];
						}

					//	idx_notch_last_tmp = idx_notch_last;

						sig_level = 0;
						sig_idx = 0;

						for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++)
						{
							sig_level = sig_level + psd[idx_tmp];
							sig_idx = sig_idx + 1;
						}

						if (sig_idx != 0)
							sig_thr = sig_level / sig_idx;
						else
						{
							//sprintf(log_str, "\tmt_fe_dmd_rs6060_bs_A_test_2 ---- sig_idx = 0!\n");
							//mt_fe_dbg_log2file(log_str);
						}

						sig_level = 0;
						sig_idx = 0;

						for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++)
						{
							if (psd[idx_tmp] >= sig_thr)
							{
								sig_level = sig_level + psd[idx_tmp];
								sig_idx = sig_idx + 1;
							}
						}

						if (sig_idx != 0)
							ave_peak = sig_level / sig_idx;
						else
						{
							//sprintf(log_str, "\tmt_fe_dmd_rs6060_bs_A_test_1 ---- sig_idx = 0!\n");
							//mt_fe_dbg_log2file(log_str);
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
								if ( psd[idx_notch] < psd[idx_notch_last])// % if the notch is lower than the last notch,  then record the notch as an effective notch too
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
								if ((1000 * psd[idx_notch] > (707 * thr_notch_tureTP)) && (((1000 - 707) * ave_peak) > (1000 * psd[idx_notch] - 707 * thr_notch_tureTP)))
									//if ((ave_peak - psd[idx_notch]) / ((ave_peak  - thr_notch_tureTP)) > 0.707)
								{
									//  % if the peak is relative high enough, then ture TP decision is depend on if the
									//   % right neibough zone has another loewr notch
									if (idx_notch >= idx_notch_last)
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

								if (test_value >= 50)
								{
									if ((100 * test_value - 5000) > (U32)(12 * K1))
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
								if ( idx_notch_last > 40)
								{
									idx_psd_shift = idx_notch_last;//-8;//% make the tuner jump according to last effective notch
										//if (idx_notch_last_tureCh>8)
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

						for (j = 1; j <= FFT_N - 2 * OVLP; j++)
						{
							if (psd[j] >= thr_notch_tureTP)
							{
								idx_tmp++;
								idx_pwr_rg_end = j;
							}

							if (idx_tmp == 1)
								idx_pwr_rg_start = j;
						}

						if (idx_tmp > 0)
						{
							if (idx_pwr_rg_start < ((FFT_N - 2 * OVLP) / 4))
							{
								idx_notch_last = 1;
								if ((idx_pwr_rg_end + (idx_tmp + 2) / 4) > (FFT_N - 2 * OVLP))
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

					if (ct_freq_actual > ct_freq)
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

					if (FFT_N > 2 * OVLP)
					{
						if (ct_freq >= (1000 * ct_freq_end))
							flag_scan_done = 1;
					}
					else
					{
						if ((ct_freq + (2000 * OVLP - FFT_N) / 2 / FFT_N * Fs) > (1000 * ct_freq_end))
							flag_scan_done = 1;
						else
						{
							flag_scan_done = 0;
							if (ct_freq >= (1000 * ct_freq_end))
								flag_scan_done = 1;
						}
					}

					if (ct_freq == freq_KHz)
					{
						same_freq++;
						if (same_freq > 3)
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
	//p_bs_info->next_freq_KHz = freq_KHz;
	//p_bs_info->next_freq_KHz = ct_freq;
	//clock4 = clock();
	//mt_fe_print(("find tp time %d\n", clock4 - clock5));

	#if FIND_DEBUG
	#else
	_mt_fe_dmd_rs6060_bs_connect(handle, p_bs_info, 0, handle->global_cfg.iScannedTpCnt);
	#endif

	//clock5 = clock();
	//mt_fe_print(("lock  tp time %d\n", clock5 - clock4));
	//mt_fe_bs_notify(MtFeMsg_BSFinish, p_bs_info);
	//    %---------------------- plot CH map according to the Blind Scan results -------------------
	//    %---------------------- Compare the scan result to reference Channel table --------------
	//Scan_result_stat;
	//CH_mis_fake_rec(rep_times, 1 : 2) = [CH_mis_num cnt_Fake_CH];

	ret = MtFeErr_Ok;

	if(psd)
		kfree(psd);

END:
	return ret;
}


MT_FE_RET mt_fe_dmd_rs6060_blindscan(MT_FE_RS6060_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info)
{
	U8 times = 0;

	if ((p_bs_info->p_tp_info == NULL) || (p_bs_info->tp_max_num == 0))
		return MtFeErr_Param;

	mt_fe_dmd_rs6060_global_reset(handle);

	mt_fe_bs_notify(MtFeMsg_BSStart, 0);

	handle->global_cfg.iScannedTpCnt	 = 0;
	handle->global_cfg.iLockedTpCnt		 = 0;
	handle->global_cfg.bCancelBs		 = FALSE;

	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
	handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2));		// XOR

	p_bs_info->tp_num = 0;

	_mt_fe_dmd_rs6060_bs_A_soft(handle, begin_freq_MHz, end_freq_MHz, p_bs_info, times);

	if (!handle->global_cfg.bCancelBs)
	{
		handle->mt_sleep(1000);

		_mt_fe_dmd_rs6060_bs_remove_unlocked_TP(handle, p_bs_info, (U32)FREQ_MAX_KHz);
	}

	mt_fe_bs_notify(MtFeMsg_BSFinish, p_bs_info);

	return MtFeErr_Ok;
}


MT_BOOL _mt_fe_dmd_rs6060_check_MIS(MT_FE_RS6060_Device_Handle handle)
{
	MT_BOOL bMIS = FALSE;

	U8 tmp;
	int iCnt = 0;

	handle->dmd_get_reg(handle, 0x08, &tmp);
	if ((tmp & 0x08) == 0x00)	// DVB-S
	{
		return bMIS;
	}

	handle->dmd_set_reg(handle, 0xE6, 0x00);
	handle->dmd_set_reg(handle, 0xE8, 0x00);
	handle->dmd_set_reg(handle, 0xE8, 0x01);


	do
	{
		handle->dmd_get_reg(handle, 0xE8, &tmp);

		if ((tmp & 0x10) == 0x10)
		{
			break;
		}

		handle->mt_sleep(1);

		iCnt++;
	} while (iCnt < 100);

	if (iCnt != 100)
	{
		handle->dmd_get_reg(handle, 0xE9, &tmp);

		if ((tmp & 0x20) != 0x20)
		{
			bMIS = TRUE;
		}
		else
		{
			bMIS = FALSE;
		}
	}


	handle->dmd_get_reg(handle, 0x00, &tmp);

	return bMIS;
}

/*
Function:
    Check signal type (ACM/CCM), for S2 only.
Output:
    m_type: 
        0, ACM;
        1, CCM

Result:
        MtFeErr_Ok:   Check ACM/CCM OK: 
        MtFeErr_Fail: Check ACM/CCM failed
 */
__attribute__((unused)) MT_FE_RET _mt_fe_dmd_rs6060_check_ACM_CCM(MT_FE_RS6060_Device_Handle handle, U8 *m_type)
{
	/*U8 type = 0;Clean warning Wunused-variable*/

	U8 tmp;
	int iCnt = 0;

	handle->dmd_get_reg(handle, 0x08, &tmp);
	if ((tmp & 0x08) == 0x00)	// DVB-S
	{
		return MtFeErr_Fail;
	}

	handle->dmd_set_reg(handle, 0xE6, 0x00);
	handle->dmd_set_reg(handle, 0xE8, 0x00);
	handle->dmd_set_reg(handle, 0xE8, 0x01);


	do
	{
		handle->dmd_get_reg(handle, 0xE8, &tmp);

		if ((tmp & 0x10) == 0x10)
		{
			break;
		}

		handle->mt_sleep(1);

		iCnt++;
	} while (iCnt < 100);

	if (iCnt != 100)
	{
		handle->dmd_get_reg(handle, 0xE9, &tmp);

		if ((tmp & 0x10) != 0x10)
		{
			*m_type = 0;
		}
		else
		{
			*m_type = 1;
		}
	}
	else
	{
		handle->dmd_get_reg(handle, 0x00, &tmp);

		return MtFeErr_Fail;
	}

	handle->dmd_get_reg(handle, 0x00, &tmp);

	return MtFeErr_Ok;
}


