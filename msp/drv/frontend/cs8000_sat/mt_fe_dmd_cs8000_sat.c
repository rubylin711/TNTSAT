/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*****************************************************************************
 *----------------------------------------------------------------------------
 *
 *	FILE:				mt_fe_dmd_cs8000_sat.c
 *
 *	VERSION:			0.06.09
 *
 *	DESCRIPTION:
 *			Define the interfaces of CS8000 Sat.
 *
 *   History:
 *   Description     Version     Date            Author
 *----------------------------------------------------------------------------
 *   File Create     0.00.01     2016.08.12      YZ.Huang
 *   Modify          0.01.00     2016.09.07      YZ.Huang
 *   Modify          0.02.00     2016.10.10      YZ.Huang
 *   Modify          0.03.00     2016.10.11      YZ.Huang
 *   Modify          0.03.01     2016.10.12      YZ.Huang
 *   Modify          0.03.02     2016.10.18      YZ.Huang
 *   Modify          0.04.00     2016.10.26      YZ.Huang
 *   Modify          0.04.01     2016.10.27      YZ.Huang
 *   Modify          0.04.02     2016.11.16      YZ.Huang
 *   Modify          0.05.00     2016.11.22      YZ.Huang
 *   Modify          0.05.01     2016.11.24      YZ.Huang
 *   Modify          0.05.02     2016.12.01      YZ.Huang
 *   Modify          0.05.03     2016.12.08      YZ.Huang
 *   Modify          0.05.04     2016.12.23      YZ.Huang
 *   Modify          0.05.05     2016.12.30      YZ.Huang
 *   Modify          0.05.06     2017.01.05      YZ.Huang
 *   Modify          0.06.00     2017.03.08      YZ.Huang
 *   Modify          0.06.01     2017.03.21      YZ.Huang
 *   Modify          0.06.02     2017.05.15      YZ.Huang
 *   Modify          0.06.03     2017.05.22      YZ.Huang
 *   Modify          0.06.04     2017.06.12      YZ.Huang
 *   Modify          0.06.05     2017.12.22      YZ.Huang
 *   Modify          0.06.06     2018.01.09      YZ.Huang
 *   Modify          0.06.07     2018.02.01      YZ.Huang
 *   Modify          0.06.08     2018.03.19      YZ.Huang
 *   Modify          0.06.09     2018.03.29      YZ.Huang
 *****************************************************************************/

//#include "mt_type.h"
//#include "sys_define.h"
//#include "drv_dev.h"
//#include "mt_debug.h"
#include <linux/printk.h>

#include "mt_fe_def_cs8000_sat.h"
#include "mt_fe_cs8000_sat_i2c.h"
#include "mt_fe_dmd_cs8000_sat_fw.h"
#include "mt_fe_dmd_cs8000_sat_reg_tbl.h"

#include "mt_module_debug.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

/* 0xbf140000 */
#define R_CHIP_INFO_BASE	VERSION_REG

/* 0xbf5d0000 */
#define R_ANALOG_SW_BASE	SYMPHONY_ANALOG_SW_BASE

/************************
****external function****
*************************/
extern void _mt_cs8k_sat_sleep(U32 ms);

/************************
****internal function****
*************************/
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

/* function for blind scan*/
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
                                   U8 bs_times,
                                   U32 is_connect);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_B(MT_FE_CS8000_SAT_Device_Handle handle, U32 start_freq_KHz,
                                   U32 end_freq_KHz,
                                   MT_FE_BS_TP_INFO *p_bs_info,
                                   U8 bs_times,
                                   U32 is_connect);
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                         U16 start_index,
                                         U16 scanned_tp);

MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect_test(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info);

void mt_fe_cs8k_sat_dummy_function(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_MSG msg, void *p_param)
{
    UNUSED_PARAMETER(msg);
    UNUSED_PARAMETER(p_param);
}
void (*mt_fe_cs8k_sat_bs_notify)(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_MSG msg, void *p_tp_info) = mt_fe_cs8k_sat_dummy_function;

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
MT_FE_RET _mt_fe_dmd_cs8k_sat_config_bs_params(MT_FE_CS8000_SAT_Device_Handle handle)
{
    if (handle == NULL) {
	return MtFeErr_Uninit;
    }

    handle->bs_cfg.fft_length = FFT_LENGTH;   // 512
    handle->bs_cfg.psd_overlap = PSD_OVERLAP; // 96;
    handle->bs_cfg.ave_times = AVE_TIMES;     // 256;
    handle->bs_cfg.data_length = DATA_LENGTH; // 512;

    handle->bs_cfg.notch_range_f = NOTCH_RANGE_F;     // 8;
    handle->bs_cfg.start_range_f = START_RANGE_F;     // 15;
    handle->bs_cfg.find_notch_step = FIND_NOTCH_STEP; // 1;
    handle->bs_cfg.psd_scale = PSD_SCALE;             // 0;
    handle->bs_cfg.sm_bufdep = SM_BUFDEP;             // 8;

    handle->bs_cfg.bs_symrate = BLINDSCAN_SYMRATEKSs; // 40000;

    handle->bs_cfg.tuner_slip_step = TUNER_SLIP_STEP; // 11;

    switch (handle->tuner_cfg.tuner_type) {
    case TN_MONTAGE_TS2022: {
	handle->bs_cfg.bs_skip_step = 36000;

	handle->bs_cfg.sm_buf_1st = 28; // TWO_LOOP_SM_BUF_1ST
	handle->bs_cfg.sm_buf_2nd = 4;  // ONE_LOOP_SM_BUF
	handle->bs_cfg.sm_buf_3rd = 2;  // TWO_LOOP_SM_BUF_2ND

	handle->bs_cfg.snr_thr_1st = 96; // TWO_LOOP_SNR_THR_1ST
	handle->bs_cfg.snr_thr_2nd = 64; // SNR_THR_ONE_LOOP
	handle->bs_cfg.snr_thr_3rd = 64; // TWO_LOOP_SNR_THR_2ND

	handle->bs_cfg.err_bound_factor_1st = 56; // TWO_LOOP_ERR_BOUND_FACTOR_1ST
	handle->bs_cfg.err_bound_factor_2nd = 36; // ERR_BOUND_FACTOR_ONE_LOOP
	handle->bs_cfg.err_bound_factor_3rd = 36; // TWO_LOOP_ERR_BOUND_FACTOR_2ND

	handle->bs_cfg.flat_thr_1st = 24; // TWO_LOOP_FLAT_THR_1ST
	handle->bs_cfg.flat_thr_2nd = 28; // FLAT_THR_ONE_LOOP
	handle->bs_cfg.flat_thr_3rd = 28; // TWO_LOOP_FLAT_THR_2ND

	handle->bs_cfg.thr_factor_1st = 1; // TWO_LOOP_THR_FACTOR_1ST
	handle->bs_cfg.thr_factor_2nd = 5; // THR_FACTOR_ONE_LOOP
	handle->bs_cfg.thr_factor_3rd = 5; // TWO_LOOP_THR_FACTOR_2ND
    } break;

    case TN_MONTAGE_TS6011:
    default: {
	handle->bs_cfg.bs_skip_step = 36000;

	handle->bs_cfg.sm_buf_1st = 28; // TWO_LOOP_SM_BUF_1ST
	handle->bs_cfg.sm_buf_2nd = 4;  // ONE_LOOP_SM_BUF
	handle->bs_cfg.sm_buf_3rd = 2;  // TWO_LOOP_SM_BUF_2ND

	handle->bs_cfg.snr_thr_1st = 64; // TWO_LOOP_SNR_THR_1ST
	handle->bs_cfg.snr_thr_2nd = 64; // SNR_THR_ONE_LOOP
	handle->bs_cfg.snr_thr_3rd = 32; // TWO_LOOP_SNR_THR_2ND

	handle->bs_cfg.err_bound_factor_1st = 56; // TWO_LOOP_ERR_BOUND_FACTOR_1ST
	handle->bs_cfg.err_bound_factor_2nd = 36; // ERR_BOUND_FACTOR_ONE_LOOP
	handle->bs_cfg.err_bound_factor_3rd = 36; // TWO_LOOP_ERR_BOUND_FACTOR_2ND

	handle->bs_cfg.flat_thr_1st = 24; // TWO_LOOP_FLAT_THR_1ST
	handle->bs_cfg.flat_thr_2nd = 28; // FLAT_THR_ONE_LOOP
	handle->bs_cfg.flat_thr_3rd = 28; // TWO_LOOP_FLAT_THR_2ND

	handle->bs_cfg.thr_factor_1st = 1; // TWO_LOOP_THR_FACTOR_1ST
	handle->bs_cfg.thr_factor_2nd = 5; // THR_FACTOR_ONE_LOOP
	handle->bs_cfg.thr_factor_3rd = 5; // TWO_LOOP_THR_FACTOR_2ND
    } break;
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_config_default(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 i;

    if (handle == NULL) {
	return MtFeErr_Uninit;
    }

    handle->dev_index = 0;
    handle->demod_dev_addr = MT_FE_DMD_DEV_ADDR_CS8K_SAT;
    handle->demod_id = MtFeDmdId_CS8000_SAT;
    handle->demod_type = MT_FE_DEMOD_CS8000_SAT;
    handle->dtv_mode = MtFeType_DvbS2;

    handle->tuner_cfg.tuner_type = TN_MONTAGE_TS2022;
    handle->tuner_cfg.tuner_init_OK = 0;
    handle->tuner_cfg.tuner_dev_addr = 0xc0; //0x58;				// MT_FE_TN_I2C_ADDR;

#ifdef CFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022_SUPPORT
    handle->tuner_cfg.tuner_init = (MT_FE_RET (*)(void *))mt_fe_sat_tn_init_ts2022;
    handle->tuner_cfg.tuner_set = (MT_FE_RET (*)(void *, U32, U32, S16))mt_fe_sat_tn_set_freq_ts2022;
    handle->tuner_cfg.tuner_get_offset = (MT_FE_RET (*)(void *, S32 *))mt_fe_sat_tn_get_tuner_freq_offset_ts2022;
    handle->tuner_cfg.tuner_get_strength = (MT_FE_RET (*)(void * handle, U32 *, S32 *))mt_fe_sat_tn_get_strength_ts2022;
    handle->tuner_cfg.tuner_get_version = (MT_FE_RET (*)(void *, U32 *, U32 *))mt_fe_sat_tn_get_version_ts2022;
    handle->tuner_cfg.tuner_sleep = (MT_FE_RET (*)(void *))mt_fe_sat_tn_sleep_ts2022;
    handle->tuner_cfg.tuner_wakeup = (MT_FE_RET (*)(void *))mt_fe_sat_tn_wakeup_ts2022;
#endif

    handle->board_cfg.bIQInverted = FALSE;
    handle->board_cfg.bAGCPolar = FALSE;
    handle->board_cfg.bSpectrumInverted = FALSE;
    handle->board_cfg.bPolarLNBEnable = LNB_ENABLE_WHEN_LNB_EN_HIGH;
    handle->board_cfg.bPolarLNB13V = LNB_13V_WHEN_VSEL_HIGH;
    handle->board_cfg.bPolarLNBStandby = LNB_VSEL_STANDBY_HIGH;
    handle->board_cfg.bPolarDiSEqCOut = LNB_DISEQC_OUT_FORCE_HIGH;
    handle->board_cfg.bDiSEqCOutputOnly = LNB_DISEQC_OUT_ONLY_OUTPUT;

    handle->ts_cfg.mTsMode = MtFeTsOutMode_Parallel;

    handle->lnb_cfg.bLnbOn = TRUE;
    handle->lnb_cfg.b22K = FALSE;
    handle->lnb_cfg.mLnbVoltage = MtFeLNB_13V;

    handle->lnb_cfg.bToneBurst = FALSE;
    handle->lnb_cfg.mToneBurst = MtFeDiSEqCToneBurst_Moulated;

    handle->lnb_cfg.bDiSEqCSend = FALSE;
    handle->lnb_cfg.iSizeSend = 0;
    for (i = 0; i < 8; i++) {
	handle->lnb_cfg.iDataSend[i] = 0;
    }

    handle->lnb_cfg.bDiSEqCReceive = FALSE;
    handle->lnb_cfg.iSizeReceive = 0;
    for (i = 0; i < 8; i++) {
	handle->lnb_cfg.iDataReceive[i] = 0;
    }

    handle->lnb_cfg.bEnvelopMode = FALSE;

    handle->lnb_cfg.bUnicable = FALSE;
    handle->lnb_cfg.iBankIndex = 0;
    handle->lnb_cfg.iUBIndex = 0;
    handle->lnb_cfg.iUBFreqMHz = 1210;

    handle->lnb_cfg.iUBCount = 0;
    for (i = 0; i < 32; i++) {
	handle->lnb_cfg.iUBList[i] = 0;
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


    handle->lnb_cfg.iUBVer = 1; /* 2: Unicable II; 1 or others: Unicable I */

    handle->global_cfg.iMclkKHz = 96000;
    handle->global_cfg.iSerialMclkHz = 96000;

    handle->global_cfg.bLastTpAFlag = FALSE;
    handle->global_cfg.iTpIndex = 0;
    handle->global_cfg.iLockedTpCnt = 0;
    handle->global_cfg.iScannedTpCnt = 0;
    handle->global_cfg.iCurCompareTpAKHz = 0;
    handle->global_cfg.bCancelBs = FALSE;
    handle->global_cfg.iTotalBsTimes = 0;

    handle->tp_cfg.mConnectType = MtFeType_DvbS;
    handle->tp_cfg.mCurrentType = MtFeType_DvbS;
    handle->tp_cfg.iFreqKHz = 0;
    handle->tp_cfg.iSymRateKSs = 0;
    handle->tp_cfg.mCodeRate = MtFeCodeRate_Undef;
    handle->tp_cfg.iCarrierOffsetKHz = 0;
    handle->tp_cfg.bLimitCarrierOffset = MT_FE_CHECK_CARRIER_OFFSET;
    handle->tp_cfg.iOffsetRangeKHz = MT_FE_CARRIER_OFFSET_KHz;

    _mt_fe_dmd_cs8k_sat_config_bs_params(handle);

	handle->version_number				 = 0x00000609;
	handle->version_time				 = 0x18032918;


    handle->dmd_set_reg = (MT_FE_RET (*)(void *, U8, U8))_mt_fe_cs8k_sat_dmd_set_reg;
    handle->dmd_get_reg = (MT_FE_RET (*)(void *, U8, U8 *))_mt_fe_cs8k_sat_dmd_get_reg;
    handle->write_fw = (MT_FE_RET (*)(void *, U8, U8 *, U16))_mt_fe_cs8k_sat_write_fw;
    handle->dmd_read = (MT_FE_RET (*)(void *, U8, U8 *, U16))NULL;
    handle->mt_sleep = (void (*)(U32))_mt_cs8k_sat_sleep;
    handle->tn_set_reg = (MT_FE_RET (*)(void *, U8, U8))_mt_fe_cs8k_sat_tn_set_reg;
    handle->tn_get_reg = (MT_FE_RET (*)(void *, U8, U8 *))_mt_fe_cs8k_sat_tn_get_reg;
    handle->Set32Bits = (MT_FE_RET (*)(U32, U32))_mt_fe_cs8k_sat_write32;
    handle->Get32Bits = (MT_FE_RET (*)(U32, U32 *))_mt_fe_cs8k_sat_read32;

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_select_tuner(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_CS8000_SAT_SUPPORTED_TUNER tuner_type)
{
    handle->tuner_cfg.tuner_type = tuner_type;

    if (tuner_type == TN_MONTAGE_TS6011) {
#ifdef CFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS6011_SUPPORT
	handle->board_cfg.bAGCPolar = 0;
	handle->board_cfg.bIQInverted = 1;

	handle->tuner_cfg.tuner_init = (MT_FE_RET (*)(void *))mt_fe_sat_tn_init_ts6011;
	handle->tuner_cfg.tuner_set = (MT_FE_RET (*)(void *, U32, U32, S16))mt_fe_sat_tn_set_freq_ts6011;
	handle->tuner_cfg.tuner_get_offset = (MT_FE_RET (*)(void *, S32 *))mt_fe_sat_tn_get_tuner_freq_offset_ts6011;
	handle->tuner_cfg.tuner_get_strength = (MT_FE_RET (*)(void * handle, U32 *, S32 *))mt_fe_sat_tn_get_strength_ts6011;

	handle->tuner_cfg.tuner_sleep = (MT_FE_RET (*)(void *))mt_fe_sat_tn_sleep_ts6011;
	handle->tuner_cfg.tuner_wakeup = (MT_FE_RET (*)(void *))mt_fe_sat_tn_wake_up_ts6011;
	handle->tuner_cfg.tuner_get_version = (MT_FE_RET (*)(void *, U32 *, U32 *))mt_fe_sat_tn_get_version_ts6011;
	handle->tuner_cfg.tuner_adjust_agc = (MT_FE_RET (*)(void *))mt_fe_sat_tn_adjust_agc_ts6011;

	handle->tuner_cfg.tuner_dev_addr = 0x58;
#endif
    } else if (tuner_type == TN_MONTAGE_TS2022) {
#ifdef CFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022_SUPPORT
	handle->board_cfg.bAGCPolar = 0;
	handle->board_cfg.bIQInverted = 0;

	handle->tuner_cfg.tuner_init = (MT_FE_RET (*)(void *))mt_fe_sat_tn_init_ts2022;
	handle->tuner_cfg.tuner_set = (MT_FE_RET (*)(void *, U32, U32, S16))mt_fe_sat_tn_set_freq_ts2022;
	handle->tuner_cfg.tuner_get_offset = (MT_FE_RET (*)(void *, S32 *))mt_fe_sat_tn_get_tuner_freq_offset_ts2022;
	handle->tuner_cfg.tuner_get_strength = (MT_FE_RET (*)(void * handle, U32 *, S32 *))mt_fe_sat_tn_get_strength_ts2022;

	handle->tuner_cfg.tuner_sleep = (MT_FE_RET (*)(void *))mt_fe_sat_tn_sleep_ts2022;
	handle->tuner_cfg.tuner_wakeup = (MT_FE_RET (*)(void *))mt_fe_sat_tn_wakeup_ts2022;
	handle->tuner_cfg.tuner_get_version = (MT_FE_RET (*)(void *, U32 *, U32 *))mt_fe_sat_tn_get_version_ts2022;

	handle->tuner_cfg.tuner_dev_addr = 0xC0;
#endif
    } else {
	handle->board_cfg.bAGCPolar = 0;
	handle->board_cfg.bIQInverted = 0;

	handle->tuner_cfg.tuner_init = NULL;
	handle->tuner_cfg.tuner_set = NULL;
	handle->tuner_cfg.tuner_get_offset = NULL;
	handle->tuner_cfg.tuner_get_strength = NULL;

	handle->tuner_cfg.tuner_sleep = NULL;
	handle->tuner_cfg.tuner_wakeup = NULL;
	handle->tuner_cfg.tuner_get_version = NULL;

	handle->tuner_cfg.tuner_dev_addr = MT_FE_TN_I2C_ADDR;
    }

    handle->tuner_cfg.tuner_init_OK = FALSE;

    mt_fe_dmd_cs8k_sat_check_tuner(handle);

    return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_driver_version
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
MT_FE_RET mt_fe_dmd_cs8k_sat_get_driver_version(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_version_number, U32 *p_version_time)
{
    *p_version_number = handle->version_number; /* driver version number */
    *p_version_time = handle->version_time;     /* driver version time */

    return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_cs8k_sat_soft_reset
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
MT_FE_RET mt_fe_dmd_cs8k_sat_soft_reset(MT_FE_CS8000_SAT_Device_Handle handle)
{
    handle->dmd_set_reg(handle, 0xb2, 0x01);
    handle->dmd_set_reg(handle, 0x00, 0x01);
    handle->mt_sleep(1);
    handle->dmd_set_reg(handle, 0x00, 0x00);
    handle->dmd_set_reg(handle, 0xb2, 0x00);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_global_reset(MT_FE_CS8000_SAT_Device_Handle handle)
{
    handle->dmd_set_reg(handle, 0x07, 0x80);
    handle->dmd_set_reg(handle, 0x07, 0x00);

    return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_cs8k_sat_hard_reset
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
MT_FE_RET mt_fe_dmd_cs8k_sat_hard_reset(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 val;

    U8 val_regb2;

    /*
		TODO:
			 do the hardware reset by the GPIO pin.
	*/

    //mt_fe_dmd_cs8k_sat_check_tuner(handle);

    mt_fe_dmd_cs8k_sat_wake_up(handle);

    if (handle->tp_cfg.bLimitCarrierOffset) {
	handle->dmd_get_reg(handle, 0xb2, &val_regb2);
	if (val_regb2 == 0x01) {
	    handle->dmd_set_reg(handle, 0x00, 0x00);
	    handle->dmd_set_reg(handle, 0xb2, 0x00);
	}
    }

    mt_fe_dmd_cs8k_sat_global_reset(handle);

    handle->mt_sleep(1);

    handle->dmd_get_reg(handle, 0x08, &val);
    val |= 0x01;
    handle->dmd_set_reg(handle, 0x08, val);

    /* reset global defines*/
    handle->tp_cfg.mCurrentType = MtFeType_DvbS;
    handle->tp_cfg.mConnectType = MtFeType_DvbS;
    handle->tp_cfg.iFreqKHz = 0;
    handle->tp_cfg.iSymRateKSs = 0;

    handle->mt_sleep(1);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_check_tuner(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 reg_0x00, reg_0x01;
    U8 i = 0;

    MT_FE_RET ret = MtFeErr_Ok;

    U8 ucTunerAddr = handle->tuner_cfg.tuner_dev_addr;

    if ((handle->tuner_cfg.tuner_type != TN_MONTAGE_TS2022) && (handle->tuner_cfg.tuner_type != TN_MONTAGE_TS6011)) {
	return MtFeErr_Ok;
    }

    for (i = 0; i < 4; i++) {
	switch (i) {
	case 0:
	    handle->tuner_cfg.tuner_dev_addr = 0x58;
	    break;

	case 1:
	    handle->tuner_cfg.tuner_dev_addr = 0x5A;
	    break;

	case 2:
	    handle->tuner_cfg.tuner_dev_addr = 0xC0;
	    break;

	case 3:
	default:
	    handle->tuner_cfg.tuner_dev_addr = 0xC2;
	    break;
	}

	ret = handle->tn_get_reg(handle, 0x00, &reg_0x00);
	if (ret != MtFeErr_Ok)
	    continue;

	if (i < 2) {
	    ret = handle->tn_get_reg(handle, 0x01, &reg_0x01);
	    if (ret != MtFeErr_Ok)
		continue;

	    if (((reg_0x00 & 0xFE) == handle->tuner_cfg.tuner_dev_addr) && ((reg_0x01 & 0xFE) == 0x58)) {
		handle->tuner_cfg.tuner_type = TN_MONTAGE_TS6011;

		handle->board_cfg.bAGCPolar = 0;
		handle->board_cfg.bIQInverted = 1;

#ifdef CFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS6011_SUPPORT
		handle->tuner_cfg.tuner_init = (MT_FE_RET (*)(void *))mt_fe_sat_tn_init_ts6011;
		handle->tuner_cfg.tuner_set = (MT_FE_RET (*)(void *, U32, U32, S16))mt_fe_sat_tn_set_freq_ts6011;
		handle->tuner_cfg.tuner_get_offset = (MT_FE_RET (*)(void *, S32 *))mt_fe_sat_tn_get_tuner_freq_offset_ts6011;
		handle->tuner_cfg.tuner_get_strength = (MT_FE_RET (*)(void * handle, U32 *, S32 *))mt_fe_sat_tn_get_strength_ts6011;

		handle->tuner_cfg.tuner_sleep = (MT_FE_RET (*)(void *))mt_fe_sat_tn_sleep_ts6011;
		handle->tuner_cfg.tuner_wakeup = (MT_FE_RET (*)(void *))mt_fe_sat_tn_wake_up_ts6011;
		handle->tuner_cfg.tuner_get_version = (MT_FE_RET (*)(void *, U32 *, U32 *))mt_fe_sat_tn_get_version_ts6011;
#endif

		break;
	    }
	} else {
	    if ((reg_0x00 & 0xFC) == 0xC0) {
		handle->tuner_cfg.tuner_type = TN_MONTAGE_TS2022;

		handle->board_cfg.bAGCPolar = 0;
		handle->board_cfg.bIQInverted = 0;

#ifdef CFG_MT_FRONTEND_DMD_CS8K_SATELLITE_TN_TS2022_SUPPORT
		handle->tuner_cfg.tuner_init = (MT_FE_RET (*)(void *))mt_fe_sat_tn_init_ts2022;
		handle->tuner_cfg.tuner_set = (MT_FE_RET (*)(void *, U32, U32, S16))mt_fe_sat_tn_set_freq_ts2022;
		handle->tuner_cfg.tuner_get_offset = (MT_FE_RET (*)(void *, S32 *))mt_fe_sat_tn_get_tuner_freq_offset_ts2022;
		handle->tuner_cfg.tuner_get_strength = (MT_FE_RET (*)(void * handle, U32 *, S32 *))mt_fe_sat_tn_get_strength_ts2022;

		handle->tuner_cfg.tuner_sleep = (MT_FE_RET (*)(void *))mt_fe_sat_tn_sleep_ts2022;
		handle->tuner_cfg.tuner_wakeup = (MT_FE_RET (*)(void *))mt_fe_sat_tn_wakeup_ts2022;
		handle->tuner_cfg.tuner_get_version = (MT_FE_RET (*)(void *, U32 *, U32 *))mt_fe_sat_tn_get_version_ts2022;
#endif

		break;
	    }
	}
    }

    if (i == 4) {
	handle->tuner_cfg.tuner_dev_addr = ucTunerAddr;
    }

    _mt_fe_dmd_cs8k_sat_config_bs_params(handle);

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_calibration(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U32 ulData = 0, ulTmp;
    U8 ucTmp;

    //Read  BF140004H[15:0]   ( Note: Set BF140008[15:0] = 0xffff)
    //A0: 0x8000; A1:0x8001

    handle->Get32Bits(R_CHIP_INFO_BASE + 0x0008, &ulTmp);
    handle->Set32Bits(R_CHIP_INFO_BASE + 0x0008, ulTmp | 0xFFFF);

    handle->Get32Bits(R_CHIP_INFO_BASE + 0x0004, &ulData);

    handle->Set32Bits(R_CHIP_INFO_BASE + 0x0008, ulTmp);

    if ((ulData & 0xFFFF) == 0x8000) // 0xA0
    {
	//r 1f157000 to data
	//handle->Get32Bits(0xBF157000, &ulData);

	//w 1f157000 0xffdffffe
	//handle->Set32Bits(0xBF157000, 0xFFDFFFFE);

	//w 1f5d00c8 0xc49c0093
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xC49C0093);

	//wait 1mS
	handle->mt_sleep(1);

	//w 1f5d00c8 0xc49c0193
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xC49C0193);

	//wait 1mS
	handle->mt_sleep(1);

	//w 1f5d00c8 0xc49c0093
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xC49C0093);

	//wait 10mS
	handle->mt_sleep(10);
	//w 1f157000 data
	//handle->Set32Bits(0xBF157000, ulData);
    } else if (((ulData & 0xFFFF) == 0x8001) || ((ulData & 0xFFFF) == 0x8009)) // A1
    {
	handle->Get32Bits(0xBF157004, &ulTmp);
	ulTmp &= 0xFFFFFFC7; // bit[5:3] = 0'b001
	ulTmp |= 0x00000008;
	handle->Set32Bits(0xBF157004, ulTmp);
	//handle->Set32Bits(0xBF157004, 0x4A010C);

	//w 1f5d00c8 0xc49c0093
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xB49C006B);

	//wait 1mS
	handle->mt_sleep(1);

	//w 1f5d00c8 0xc49c0193
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xB49C016B);

	//wait 1mS
	handle->mt_sleep(1);

	//w 1f5d00c8 0xc49c0093
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0xB49C006B);

	//wait 10mS
	handle->mt_sleep(10);
	//w 1f157000 data
	//handle->Set32Bits(0xBF157000, ulData);

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x0094, &ulData);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, ulData | 0x100000);

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &ulTmp);
	ucTmp = (ulTmp >> 7) & 0x07;
	if (ucTmp == 0) {
	    handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C806B);
	} else if (ucTmp == 4) {
	    handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C406B);
	} else {
	    handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C006B);
	}

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, ulData);
    } else // if(((ulData & 0xFFFF) == 0x8002) || ((ulData & 0xFFFF) == 0x800A))		// A2
    {
	U32 j = 0;
	U32 comp0b = 32, comp1b = 32, comp2b = 32;

	U32 data = 0, done = 0, comp0 = 0, comp1 = 0, comp2 = 0, balance_all = 0, done_all = 0, delay_code = 0;
	U32 error = 0, error0 = 0, error1 = 0, error2 = 0;
	U32 offset0 = 0, offset1 = 0, offset2 = 0, offset_cal = 0;
	S32 delta0 = 0, delta1 = 0, delta2 = 0;

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C026B); // 0x349C026B
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00CC, 0x41040);    //set offset code,0:0x0,1:0x2082,32:0x41040,63:0x7FFFE

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C036B); // 0x349C036B
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C026B); // 0x349C026B

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &data);
	comp0b = (data >> 1) & 0x3f;
	comp1b = (data >> 7) & 0x3f;
	comp2b = (data >> 13) & 0x3f;

	for (j = 1; j <= 64; j++) {
	    handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x8000E);
	    handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xC000E);

	    handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &data);
	    done = data & 0x01;
	    comp0 = (data >> 1) & 0x3f;
	    comp1 = (data >> 7) & 0x3f;
	    comp2 = (data >> 13) & 0x3f;
	    delta0 = comp0 - comp0b;
	    delta1 = comp1 - comp1b;
	    delta2 = comp2 - comp2b;
	    if (delta0 > 1 || delta0 < -1) {
		if (error0 == 0) {
		    offset0 = comp0b;
		}
		error0 = error0 + 1;
	    }

	    if (delta1 > 1 || delta1 < -1) {
		if (error1 == 0) {
		    offset1 = comp1b;
		}
		error1 = error1 + 1;
	    }

	    if (delta2 > 1 || delta2 < -1) {
		if (error2 == 0) {
		    offset2 = comp2b;
		}
		error2 = error2 + 1;
	    }

	    comp0b = comp0;
	    comp1b = comp1;
	    comp2b = comp2;
	    //m_pIParaI2c->DelayUs(100);
	    handle->mt_sleep(1);
	}

	if (error0 == 0) {
	    offset0 = comp0;
	}

	if (error1 == 0) {
	    offset1 = comp1;
	}

	if (error2 == 0) {
	    offset2 = comp2;
	}

	offset_cal = ((offset2 & 0x3f) << 13) + ((offset1 & 0x3f) << 7) + ((offset0 & 0x3f) << 1) + 1;

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00CC, offset_cal);
	handle->Set32Bits(R_ANALOG_SW_BASE + 0x00C8, 0x349C426B); // 0x349C426B

	//disp offset info
	error = ((error2 & 0x1) << 2) + ((error1 & 0x1) << 1) + ((error0 & 0x1));

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &data);
	done = data & 0x01;
	comp0 = (data >> 1) & 0x3f;
	comp1 = (data >> 7) & 0x3f;
	comp2 = (data >> 13) & 0x3f;

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0x18000E);

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &data);
	balance_all = (data >> 1) & 0x7;
	done_all = (data >> 4) & 0x7;
	delay_code = (data >> 7) & 0x7;

	handle->Set32Bits(R_ANALOG_SW_BASE + 0x0094, 0xE);

	handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &data);
	mt_fe_print(("0x%08X %d %d %d, %d %d %d, %d,%d %d %d\n", data, comp0, comp1, comp2, done, done_all, balance_all, delay_code, error0, error1, error2));
    }

    // 0x1F5D00D4 Bit0: s-adc  calibration done
    // 0 - calibration fail
    // 1 - calibration ok

    handle->Get32Bits(R_ANALOG_SW_BASE + 0x00D4, &ulData);
    //mt_fe_print(("\tCS8000 satellite calibration %s!\n", (ulData & 0x01) ? "OK" : "failed"));
    MT_INFO_FRONTEND("\tCS8000 satellite calibration %s!\n", (ulData & 0x01) ? "OK" : "failed");

    return MtFeErr_Ok;
}

/******************************************************************************
** Function: mt_fe_dmd_cs8k_sat_init
**
**
** Description:  This function is used to initial CS8000 Sat.
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
MT_FE_RET mt_fe_dmd_cs8k_sat_init(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 val;

    mt_fe_dmd_cs8k_sat_calibration(handle);

    mt_fe_dmd_cs8k_sat_hard_reset(handle);
    _mt_fe_dmd_cs8k_sat_init_reg(handle, reg_tbl_def_cs8000_sat, sizeof(reg_tbl_def_cs8000_sat) / 2);

    if (handle->tuner_cfg.tuner_init != NULL)
	handle->tuner_cfg.tuner_init(handle);

    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;

    handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2)); // XOR

    //handle->dmd_set_reg(handle, 0x07, 0xE0);		// global reset, global DiSEqC reset, global FEC reset
    //handle->dmd_set_reg(handle, 0x07, 0x00);

    _mt_fe_dmd_cs8k_sat_download_fw(handle, cs8000_sat_fw);

    //_mt_fe_dmd_cs8k_sat_set_ts_out_mode(handle, handle->ts_cfg.mTsMode);

    if (handle->board_cfg.bSpectrumInverted) // 0x4d, bit 1
    {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val |= 0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    } else {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val &= ~0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    }

    if (handle->board_cfg.bAGCPolar) // 0x30, bit 4
    {
	handle->dmd_get_reg(handle, 0x30, &val);
	val |= 0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    } else {
	handle->dmd_get_reg(handle, 0x30, &val);
	val &= ~0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    }

    //_mt_fe_sys_set_reg(handle, 0x18, 0x55);
    handle->dmd_set_reg(handle, 0x08, 0x47);
    handle->dmd_set_reg(handle, 0xf1, 0x01); //add by wbj 20161117

    return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_cs8k_sat_sleep
**
**
** Description:	This function is used to sleep CS8000 Sat.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_cs8k_sat_sleep(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 val;

    handle->dmd_get_reg(handle, 0x08, &val);
    val &= ~0x01;
    handle->dmd_set_reg(handle, 0x08, val);

    if (handle->tuner_cfg.tuner_sleep != NULL)
	handle->tuner_cfg.tuner_sleep(handle);

    handle->dmd_get_reg(handle, 0x04, &val);
    val |= 0x01;
    handle->dmd_set_reg(handle, 0x04, val);

    return MtFeErr_Ok;
}

/************************************************************************
** Function: mt_fe_dmd_cs8k_sat_wake_up
**
**
** Description:	This function is used to wake up CS8000 Sat.
**
**
** Inputs:   None
**
**
** Outputs:  None
**
*************************************************************************/
MT_FE_RET mt_fe_dmd_cs8k_sat_wake_up(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 val;

    handle->dmd_get_reg(handle, 0x04, &val);
    val &= ~0x01;
    handle->dmd_set_reg(handle, 0x04, val);

    if (handle->tuner_cfg.tuner_wakeup != NULL)
	handle->tuner_cfg.tuner_wakeup(handle);

    handle->dmd_get_reg(handle, 0x08, &val);
    val |= 0x01;
    handle->dmd_set_reg(handle, 0x08, val);

    mt_fe_dmd_cs8k_sat_calibration(handle);

    return MtFeErr_Ok;
}

/**********************************************************************************************
** Function: mt_fe_dmd_cs8k_sat_connect
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
MT_FE_RET mt_fe_dmd_cs8k_sat_connect(MT_FE_CS8000_SAT_Device_Handle handle, U32 freq_MHz, U32 sym_rate_KSs, MT_FE_TYPE dvbs_type, U32 is_BS)
{
    S16 lpf_offset_KHz;
    U32 freq_tmp_KHz;

    U32 real_freq_KHz;
    S32 deviceoffset_KHz;
    S32 iTunerOffsetKHz = 0;

    int i;

    MT_FE_RET ret = MtFeErr_Ok;

    U8 val_regb2;

    if ((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
	return MtFeErr_NullPointer;

    mt_fe_dmd_cs8k_sat_global_reset(handle);

    handle->tp_cfg.bCheckCCM = FALSE;
    handle->tp_cfg.iTsCnt = 0;

    for (i = 0; i < 16; i++) {
	handle->tp_cfg.ucTsId[i] = 0;
    }

    handle->tp_cfg.mConnectType = dvbs_type;
    handle->tp_cfg.iSymRateKSs = sym_rate_KSs;

    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;
    handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2)); // XOR

    if (handle->tp_cfg.bLimitCarrierOffset) {
	handle->dmd_get_reg(handle, 0xb2, &val_regb2);
	if (val_regb2 == 0x01) {
	    handle->dmd_set_reg(handle, 0x00, 0x00);
	    handle->dmd_set_reg(handle, 0xb2, 0x00);
	}
    }

    freq_tmp_KHz = freq_MHz * 1000;
    lpf_offset_KHz = 0;

    if ((sym_rate_KSs < 5000) && (!handle->lnb_cfg.bUnicable)) {
	lpf_offset_KHz = FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
	freq_tmp_KHz += FREQ_OFFSET_AT_SMALL_SYM_RATE_KHz;
    }

    handle->tp_cfg.iFreqKHz = freq_tmp_KHz;

    /*set tuner*/
    if (handle->lnb_cfg.bUnicable) {
	mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_tmp_KHz, sym_rate_KSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	deviceoffset_KHz = real_freq_KHz - freq_tmp_KHz;
    } else {
	handle->tuner_cfg.tuner_set(handle, freq_tmp_KHz, sym_rate_KSs, lpf_offset_KHz);
	real_freq_KHz = freq_tmp_KHz;
	deviceoffset_KHz = 0;
    }

    handle->dmd_set_reg(handle, 0xb2, 0x01);
    handle->dmd_set_reg(handle, 0x00, 0x01);

    handle->dmd_get_reg(handle, 0x08, &val_regb2);
    if (dvbs_type == MtFeType_DvbS) {
	val_regb2 = (U8)((val_regb2 & 0xfb) | 0x40);
	handle->dmd_set_reg(handle, 0x08, val_regb2);
	handle->dmd_set_reg(handle, 0xe0, 0xf8);
    } else if (dvbs_type == MtFeType_DvbS2) {
	val_regb2 = (U8)(val_regb2 | 0x44);
	handle->dmd_set_reg(handle, 0x08, val_regb2);
    } else // if (dvbs_type == MtFeType_DvbS_S2)
    {
	val_regb2 = (U8)(val_regb2 & 0xbb);
	handle->dmd_set_reg(handle, 0x08, val_regb2);
	handle->dmd_set_reg(handle, 0xe0, 0xf8);
    }

    /*set demod registers*/
    _mt_fe_dmd_cs8k_sat_set_demod(handle, sym_rate_KSs, handle->tp_cfg.mCurrentType);

    /*set carrier offset*/
    handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
    _mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz + lpf_offset_KHz + deviceoffset_KHz);

    handle->dmd_set_reg(handle, 0x00, 0x00);
    handle->dmd_set_reg(handle, 0xb2, 0x00);
    // fix bug 101393 start
	if(handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	{
		handle->tuner_cfg.tuner_adjust_agc(handle);
	}
    // fix bug 101393 end
    return ret;
}

/******************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_lock_state
**
**
** Description:	This function is called by app level to get the
**			lock status of CS8000 Sat.
**
**
** Inputs:   None
**
**
** Outputs:
**
**		Parameter	Type				Description
**		-----------------------------------------------------------
**		p_state		MT_FE_LOCK_STATE*	lock status of CS8000 Sat
**
**
*******************************************************************/
MT_FE_RET mt_fe_dmd_cs8k_sat_get_lock_state(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_LOCK_STATE *p_state, U32 is_BS)
{
    U8 tmp, ucTmp;
    int iCnt;

    U8 val_regb2;
    S32 carrier_offset_KHz;

    if (p_state == 0)
	return MtFeErr_Param;

    *p_state = MtFeLockState_Waiting;

    if (handle->tp_cfg.bLimitCarrierOffset) {
	handle->dmd_get_reg(handle, 0xb2, &val_regb2);
	if (val_regb2 == 0x01) {
	    *p_state = MtFeLockState_Unlocked;

	    return MtFeErr_Ok;
	}
    }

    handle->dmd_get_reg(handle, 0x08, &tmp);
    if (tmp & 0x08) {
	handle->dmd_get_reg(handle, 0x0d, &tmp);
	if ((tmp & 0x8f) == 0x8f) {
	    *p_state = MtFeLockState_Locked;
	    handle->tp_cfg.mCurrentType = MtFeType_DvbS2;
	    if (!handle->tp_cfg.bCheckCCM) {
		handle->dmd_set_reg(handle, 0xE5, 0x10);
		handle->dmd_set_reg(handle, 0xE5, 0x40);
		handle->mt_sleep(10);
		handle->dmd_get_reg(handle, 0xE5, &ucTmp);
		handle->tp_cfg.iTsCnt = ucTmp & 0x0F;

		if (handle->tp_cfg.iTsCnt > 1) {
		    mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", handle->tp_cfg.iTsCnt));

		    for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++) {
			ucTmp = (U8)iCnt;

			handle->dmd_set_reg(handle, 0xE6, ucTmp);
			handle->dmd_get_reg(handle, 0xE7, &ucTmp);

			handle->tp_cfg.ucTsId[iCnt] = ucTmp;

			mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
		    }

		    handle->dmd_set_reg(handle, 0xE6, 0x0F);
		    for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++) {
			if (handle->tp_cfg.ucTsId[iCnt] == handle->tp_cfg.ucCurTsId) {
			    handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucCurTsId);

			    mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, handle->tp_cfg.ucCurTsId));

			    break;
			}
		    }
		    if (iCnt == handle->tp_cfg.iTsCnt) {
			// Default select TS0 output
			handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucTsId[0]);
			handle->tp_cfg.ucCurTsId = handle->tp_cfg.ucTsId[0];

			mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
		    }
		} else {
		    handle->dmd_set_reg(handle, 0xE5, 0x00);
		}

		handle->tp_cfg.bCheckCCM = TRUE;
	    }
	} else if (((tmp & 0x0C) == 0x0C) && (!handle->tp_cfg.bCheckCCM)) {
	    handle->dmd_set_reg(handle, 0xE5, 0x10);
	    handle->dmd_set_reg(handle, 0xE5, 0x40);
	    handle->mt_sleep(10);
	    handle->dmd_get_reg(handle, 0xE5, &ucTmp);
	    handle->tp_cfg.iTsCnt = ucTmp & 0x0F;

	    if (handle->tp_cfg.iTsCnt > 1) {
		mt_fe_print(("\tCCM find before FEC! Total %d streams!\n", handle->tp_cfg.iTsCnt));

		for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++) {
		    ucTmp = (U8)iCnt;

		    handle->dmd_set_reg(handle, 0xE6, ucTmp);
		    handle->dmd_get_reg(handle, 0xE7, &ucTmp);

		    handle->tp_cfg.ucTsId[iCnt] = ucTmp;

		    mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
		}

		handle->dmd_set_reg(handle, 0xE6, 0x0F);
		for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++) {
		    if (handle->tp_cfg.ucTsId[iCnt] == handle->tp_cfg.ucCurTsId) {
			handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucCurTsId);

			mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, handle->tp_cfg.ucCurTsId));

			break;
		    }
		}

		if (iCnt == handle->tp_cfg.iTsCnt) {
		    // Default select TS0 output
		    handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucTsId[0]);
		    handle->tp_cfg.ucCurTsId = handle->tp_cfg.ucTsId[0];

		    mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
		}
	    } else {
		handle->dmd_set_reg(handle, 0xE5, 0x00);
	    }

	    handle->tp_cfg.bCheckCCM = TRUE;
	}
    } else {
	handle->dmd_get_reg(handle, 0x0d, &tmp);
	if ((tmp & 0xf7) == 0xf7) {
	    *p_state = MtFeLockState_Locked;
	    handle->tp_cfg.mCurrentType = MtFeType_DvbS;
	}
    }

    if (*p_state == MtFeLockState_Locked) {
	MT_FE_CHAN_INFO_DVBS2 ch_info;

	if (handle->tp_cfg.bLimitCarrierOffset) {
	    carrier_offset_KHz = 0;
	    _mt_fe_dmd_cs8k_sat_get_total_carrier_offset(handle, &carrier_offset_KHz);

	    if ((carrier_offset_KHz >= handle->tp_cfg.iOffsetRangeKHz) ||
	        (carrier_offset_KHz <= -handle->tp_cfg.iOffsetRangeKHz)) {
		handle->dmd_set_reg(handle, 0xb2, 0x01);
		handle->dmd_set_reg(handle, 0x00, 0x01);

		*p_state = MtFeLockState_Unlocked;
	    }
	}

	mt_fe_dmd_cs8k_sat_get_channel_info(handle, &ch_info);

	if ((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk)) {
	    handle->dmd_set_reg(handle, 0x9e, 0x9f);
	    handle->dmd_set_reg(handle, 0x9f, 0x0c);

	    handle->dmd_get_reg(handle, 0x9d, &tmp);
	    tmp &= 0xFE;
	    handle->dmd_set_reg(handle, 0x9d, tmp);
	}
    } else {
	handle->dmd_get_reg(handle, 0xb8, &tmp);
	if ((tmp & 0xc0) != 0x00) {
	    *p_state = MtFeLockState_Unlocked;
	}
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_get_pure_lock(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
    U8 tmp;

    if (p_state == 0)
	return MtFeErr_Param;

    *p_state = MtFeLockState_Unlocked;

    handle->dmd_get_reg(handle, 0x08, &tmp);
    if (tmp & 0x08) {
	handle->dmd_get_reg(handle, 0x0d, &tmp);
	if ((tmp & 0x8f) == 0x8f) {
	    *p_state = MtFeLockState_Locked;
	    handle->tp_cfg.mCurrentType = MtFeType_DvbS2;
	}
    } else {
	handle->dmd_get_reg(handle, 0x0d, &tmp);
	if ((tmp & 0xf7) == 0xf7) {
	    *p_state = MtFeLockState_Locked;
	    handle->tp_cfg.mCurrentType = MtFeType_DvbS;
	}
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_set_ts(MT_FE_CS8000_SAT_Device_Handle handle, U8 ucTsId)
{
    MT_FE_RET ret = MtFeErr_Ok;

    int iCnt;

    for (iCnt = 0; iCnt < handle->tp_cfg.iTsCnt; iCnt++) {
	if (handle->tp_cfg.ucTsId[iCnt] == ucTsId) {
	    handle->dmd_set_reg(handle, 0xE6, 0x0F);
	    handle->dmd_set_reg(handle, 0xE7, ucTsId);

	    mt_fe_print(("\tManual select TS %2d---- 0x%02X\n", iCnt, ucTsId));

	    handle->tp_cfg.ucCurTsId = ucTsId;

	    ret = MtFeErr_Ok;

	    break;
	}
    }

    if (iCnt == handle->tp_cfg.iTsCnt) {
	//handle->dmd_set_reg(handle, 0xE7, handle->tp_cfg.ucTsId[0]);

	//mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));

	mt_fe_print(("\tCan't find this TS ---- 0x%02X\n", ucTsId));

	ret = MtFeErr_Param;
    }

    return ret;
}

/*********************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_per
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
MT_FE_RET mt_fe_dmd_cs8k_sat_get_per(MT_FE_CS8000_SAT_Device_Handle handle, U32 *p_total_packags, U32 *p_err_packags)
{
#define TOTAL_PACKET_S 3000

    U8 tmp1, tmp2, tmp3;
    U8 val_0xF6, val_0xF7, val_0xF8;
    U32 code_rate_fac = 0, ldpc_frame_cnt;
    static U32 pre_err_packags = 0;
    static U32 pre_total_packags = 0xffff;
    MT_FE_CHAN_INFO_DVBS2 p_info;

    *p_err_packags = pre_err_packags;
    *p_total_packags = pre_total_packags;

    pre_err_packags = 0;

    if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2) {
	mt_fe_dmd_cs8k_sat_get_channel_info(handle, &p_info);
	switch (p_info.code_rate) {
	case MtFeCodeRate_1_4:
	    code_rate_fac = 16008 - 80;
	    break;
	case MtFeCodeRate_1_3:
	    code_rate_fac = 21408 - 80;
	    break;
	case MtFeCodeRate_2_5:
	    code_rate_fac = 25728 - 80;
	    break;
	case MtFeCodeRate_1_2:
	    code_rate_fac = 32208 - 80;
	    break;
	case MtFeCodeRate_3_5:
	    code_rate_fac = 38688 - 80;
	    break;
	case MtFeCodeRate_2_3:
	    code_rate_fac = 43040 - 80;
	    break;
	case MtFeCodeRate_3_4:
	    code_rate_fac = 48408 - 80;
	    break;
	case MtFeCodeRate_4_5:
	    code_rate_fac = 51648 - 80;
	    break;
	case MtFeCodeRate_5_6:
	    code_rate_fac = 53840 - 80;
	    break;
	case MtFeCodeRate_8_9:
	    code_rate_fac = 57472 - 80;
	    break;
	case MtFeCodeRate_9_10:
	    code_rate_fac = 58192 - 80;
	    break;
	default:
	    break;
	}

	handle->dmd_get_reg(handle, 0xdb, &tmp1);
	handle->dmd_get_reg(handle, 0xda, &tmp2);
	handle->dmd_get_reg(handle, 0xd9, &tmp3);
	ldpc_frame_cnt = (tmp1 << 16) | (tmp2 << 8) | tmp3;

	handle->dmd_get_reg(handle, 0xf8, &tmp1);
	handle->dmd_get_reg(handle, 0xf7, &tmp2);
	pre_err_packags = (tmp1 << 8) | tmp2;

	pre_total_packags = code_rate_fac * ldpc_frame_cnt / (188 * 8);

	if (ldpc_frame_cnt > TOTAL_PACKET_S) {
	    handle->dmd_set_reg(handle, 0xd1, 0x01);
	    handle->dmd_set_reg(handle, 0xf9, 0x01);
	    handle->dmd_set_reg(handle, 0xf9, 0x00);
	    handle->dmd_set_reg(handle, 0xd1, 0x00);

	    *p_err_packags = pre_err_packags;
	    *p_total_packags = pre_total_packags;
	}
    } else {
	//handle->dmd_set_reg(handle, 0xf9, 0x04);
	handle->dmd_get_reg(handle, 0xd5, &val_0xF8);

	if ((val_0xF8 & 0x80) == 0) {
	    handle->dmd_get_reg(handle, 0xd6, &val_0xF6);
	    handle->dmd_get_reg(handle, 0xd7, &val_0xF7);

	    pre_err_packags = (val_0xF7 << 8) | val_0xF6;

	    //val_0xF8 |= 0x10;
	    handle->dmd_set_reg(handle, 0xd5, 0x82);
	}

	*p_err_packags = pre_err_packags;

	*p_total_packags = 8388608 / 16;
    }

    return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_snr
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
MT_FE_RET mt_fe_dmd_cs8k_sat_get_snr(MT_FE_CS8000_SAT_Device_Handle handle, S8 *p_snr)
{
    const U16 mes_log10[] =
        {
	    0, 3010, 4771, 6021, 6990, 7781, 8451, 9031, 9542, 10000,
	    10414, 10792, 11139, 11461, 11761, 12041, 12304, 12553, 12788, 13010,
	    13222, 13424, 13617, 13802, 13979, 14150, 14314, 14472, 14624, 14771,
	    14914, 15052, 15185, 15315, 15441, 15563, 15682, 15798, 15911, 16021,
	    16128, 16232, 16335, 16435, 16532, 16628, 16721, 16812, 16902, 16990,
	    17076, 17160, 17243, 17324, 17404, 17482, 17559, 17634, 17709, 17782,
	    17853, 17924, 17993, 18062, 18129, 18195, 18261, 18325, 18388, 18451,
	    18513, 18573, 18633, 18692, 18751, 18808, 18865, 18921, 18976, 19031
	};

    const U16 mes_loge[] =
        {
	    0, 6931, 10986, 13863, 16094, 17918, 19459, 20794, 21972, 23026,
	    23979, 24849, 25649, 26391, 27081, 27726, 28332, 28904, 29444, 29957,
	    30445, 30910, 31355, 31781, 32189, 32581, 32958, 33322, 33673, 34012,
	    34340, 34657
	};

    static S8 snr;

    U8 val, npow1, npow2, spow1, cnt;
    U16 tmp;
    U32 npow, spow, snr_total;

    *p_snr = 0;

    handle->dmd_get_reg(handle, 0x08, &val);
    if ((val & 0x08) != 0) {
	cnt = 10;
	npow = 0;
	spow = 0;

	while (cnt > 0) {
	    handle->dmd_get_reg(handle, 0x8c, &npow1);
	    handle->dmd_get_reg(handle, 0x8d, &npow2);
	    npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

	    handle->dmd_get_reg(handle, 0x8e, &spow1);
	    spow += ((spow1 * spow1) >> 1);

	    cnt--;
	}

	npow /= 10;
	spow /= 10;

	if (spow == 0) {
	    snr = 0;
	} else if (npow == 0) {
	    snr = 19;
	} else {
	    if (spow > npow) {
		tmp = (U16)((spow) / npow);
		if (tmp > 80)
		    tmp = 80;

		snr = (S8)(mes_log10[tmp - 1] / 1000);
	    } else {
		tmp = (U16)(npow / spow);
		if (tmp > 80)
		    tmp = 80;

		snr = (S8)(-(mes_log10[tmp - 1] / 1000));
	    }
	}
    } else {
	cnt = 10;
	snr_total = 0;

	while (cnt > 0) {
	    handle->dmd_get_reg(handle, 0xff, &val);
	    snr_total += val;

	    cnt--;
	}

	tmp = (U16)(snr_total / 80);
	if (tmp > 0) {
	    if (tmp > 32) // impossible
		tmp = 32;

	    snr = (S8)((mes_loge[tmp - 1] * 10) / 23026);
	} else {
	    snr = 0;
	}
    }

    *p_snr = snr;

    return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_strength
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
MT_FE_RET mt_fe_dmd_cs8k_sat_get_strength(MT_FE_CS8000_SAT_Device_Handle handle, S8 *p_strength)
{
    S32 iStrength = 0;
    U32 iGain = 0;

    if (handle->tuner_cfg.tuner_get_strength != NULL) {
	handle->tuner_cfg.tuner_get_strength(handle, &iGain, &iStrength);
	*p_strength = (S8)iStrength;
    } else {
	*p_strength = 0;

	return MtFeErr_NullPointer;
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_get_sat_quality(MT_FE_CS8000_SAT_Device_Handle handle, U8 *p_percent)
{
    MT_FE_LOCK_STATE lock_state;

    mt_fe_dmd_cs8k_sat_get_pure_lock(handle, &lock_state);

    if (lock_state == MtFeLockState_Locked) {
	const U16 mes_log10[] =
	    {
	        0, 3010, 4771, 6021, 6990, 7781, 8451, 9031, 9542, 10000,
	        10414, 10792, 11139, 11461, 11761, 12041, 12304, 12553, 12788, 13010,
	        13222, 13424, 13617, 13802, 13979, 14150, 14314, 14472, 14624, 14771,
	        14914, 15052, 15185, 15315, 15441, 15563, 15682, 15798, 15911, 16021,
	        16128, 16232, 16335, 16435, 16532, 16628, 16721, 16812, 16902, 16990,
	        17076, 17160, 17243, 17324, 17404, 17482, 17559, 17634, 17709, 17782,
	        17853, 17924, 17993, 18062, 18129, 18195, 18261, 18325, 18388, 18451,
	        18513, 18573, 18633, 18692, 18751, 18808, 18865, 18921, 18976, 19031,
	        19085, 19138, 19191, 19243, 19294, 19345, 19395, 19445, 19494, 19542,
	        19590, 19638, 19685, 19731, 19777, 19823, 19868, 19912, 19956, 20000
	    };

	/*		const U16 mes_loge[] =
		{
			0,		6931,	10986,	13863, 	16094,	17918,	19459,	20794,	21972,	23026,
			23979,	24849,	25649,	26391,	27081,	27726,	28332,	28904,	29444,	29957,
			30445,	30910,	31355,	31781,	32189,	32581,	32958,	33322,	33673,	34012,
			34340,	34657
		};*/

	static S16 snr;

	U8 val, npow1, npow2, spow1, cnt;
	U16 tmp;
	U32 npow, spow, snr_total;

	*p_percent = 0;
	if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2) {
	    cnt = 10;
	    npow = 0;
	    spow = 0;

	    while (cnt > 0) {
		handle->dmd_get_reg(handle, 0x8c, &npow1);
		handle->dmd_get_reg(handle, 0x8d, &npow2);
		npow += (((npow1 & 0x3F) + (U16)(npow2 << 6)) >> 2);

		handle->dmd_get_reg(handle, 0x8e, &spow1);
		spow += ((spow1 * spow1) >> 1);

		cnt--;
	    }

	    npow /= 10;
	    spow /= 10;

	    if (spow == 0) {
		snr = 0;
	    } else if (npow == 0) {
		snr = 20;
	    } else {
		if (spow > npow) {
		    tmp = (U16)((spow) / npow);
		    if (tmp > 100)
			tmp = 100;

		    snr = (S8)(mes_log10[tmp - 1] / 1000);
		} else {
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

	    *p_percent = 30 + (snr + 5) * 70 / 25; /* 30% ~ 100% */
	} else {
	    cnt = 10;
	    snr_total = 0;

	    while (cnt > 0) {
		handle->dmd_get_reg(handle, 0xff, &val);
		snr_total += val;

		cnt--;
	    }

	    snr = snr_total / 10;

	    *p_percent = 30 + snr * 70 / 256; /* 30% ~ 100% */
	}
    } else {
	U8 val, cnt;

	U16 total = 0;

	handle->dmd_set_reg(handle, 0x63, 0x4f);

	for (cnt = 0; cnt < 3; cnt++) {
	    handle->dmd_get_reg(handle, 0x8f, &val);

	    total += val;
	}

	val = total / 3;

	if (val < 0x0e) {
	    val = 0x0e;
	}

	if (val > 0x87) {
	    val = 0x87;
	}

	*p_percent = (val - 0x0e) * 29 / 121; /* 0% ~ 29% */

	handle->dmd_set_reg(handle, 0x63, 0x60);
    }

    return MtFeErr_Ok;
}

/******************************************************************
** Function: mt_fe_dmd_cs8k_sat_get_channel_info
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
MT_FE_RET mt_fe_dmd_cs8k_sat_get_channel_info(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_CHAN_INFO_DVBS2 *p_info)
{
    U8 tmp, val_0x7E;

    p_info->type = handle->tp_cfg.mCurrentType;

    if (handle->tp_cfg.mCurrentType == MtFeType_DvbS2) {
	handle->dmd_get_reg(handle, 0x7e, &val_0x7E);
	tmp = (U8)((val_0x7E & 0xC0) >> 6);
	switch (tmp) {
	case 0:
	    p_info->mod_mode = MtFeModMode_Qpsk;
	    break;
	case 1:
	    p_info->mod_mode = MtFeModMode_8psk;
	    break;
	case 2:
	    p_info->mod_mode = MtFeModMode_16Apsk;
	    break;
	case 3:
	    p_info->mod_mode = MtFeModMode_32Apsk;
	    break;
	default:
	    p_info->mod_mode = MtFeModMode_Undef;
	    break;
	}

	p_info->is_pilot_on = ((val_0x7E & 0x20) != 0) ? MtFe_True : MtFe_False;

	tmp = (U8)(val_0x7E & 0x0f);
	switch (tmp) {
	case 0:
	    p_info->code_rate = MtFeCodeRate_1_4;
	    break;
	case 1:
	    p_info->code_rate = MtFeCodeRate_1_3;
	    break;
	case 2:
	    p_info->code_rate = MtFeCodeRate_2_5;
	    break;
	case 3:
	    p_info->code_rate = MtFeCodeRate_1_2;
	    break;
	case 4:
	    p_info->code_rate = MtFeCodeRate_3_5;
	    break;
	case 5:
	    p_info->code_rate = MtFeCodeRate_2_3;
	    break;
	case 6:
	    p_info->code_rate = MtFeCodeRate_3_4;
	    break;
	case 7:
	    p_info->code_rate = MtFeCodeRate_4_5;
	    break;
	case 8:
	    p_info->code_rate = MtFeCodeRate_5_6;
	    break;
	case 9:
	    p_info->code_rate = MtFeCodeRate_8_9;
	    break;
	case 10:
	    p_info->code_rate = MtFeCodeRate_9_10;
	    break;
	default:
	    p_info->code_rate = MtFeCodeRate_Undef;
	    break;
	}

	handle->dmd_get_reg(handle, 0x89, &tmp);
	p_info->is_spectrum_inv = ((tmp & 0x80) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;

	handle->dmd_get_reg(handle, 0xf2, &tmp);
	tmp &= 0x03;
	switch (tmp) {
	case 0:
	    p_info->roll_off = MtFeRollOff_0p35;
	    break;
	case 1:
	    p_info->roll_off = MtFeRollOff_0p25;
	    break;
	case 2:
	    p_info->roll_off = MtFeRollOff_0p20;
	    break;
	default:
	    p_info->roll_off = MtFeRollOff_Undef;
	    break;
	}
    } else {
	p_info->mod_mode = MtFeModMode_Qpsk;

	handle->dmd_get_reg(handle, 0xe6, &tmp);
	tmp = (U8)(tmp >> 5);
	switch (tmp) {
	case 0:
	    p_info->code_rate = MtFeCodeRate_7_8;
	    break;
	case 1:
	    p_info->code_rate = MtFeCodeRate_5_6;
	    break;
	case 2:
	    p_info->code_rate = MtFeCodeRate_3_4;
	    break;
	case 3:
	    p_info->code_rate = MtFeCodeRate_2_3;
	    break;
	case 4:
	    p_info->code_rate = MtFeCodeRate_1_2;
	    break;
	default:
	    p_info->code_rate = MtFeCodeRate_Undef;
	    break;
	}

	p_info->is_pilot_on = MtFe_False;

	handle->dmd_get_reg(handle, 0xe0, &tmp);
	p_info->is_spectrum_inv = ((tmp & 0x04) != 0) ? MtFeSpectrum_Inversion : MtFeSpectrum_Normal;

	p_info->roll_off = MtFeRollOff_0p35;
    }

    return MtFeErr_Ok;
}

/*********************************************************************************
** Function: mt_fe_dmd_cs8k_sat_set_LNB
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
MT_FE_RET mt_fe_dmd_cs8k_sat_set_LNB(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
    U8 val_0xa1, val_0xa2;

    handle->dmd_get_reg(handle, 0xa1, &val_0xa1);
    handle->dmd_get_reg(handle, 0xa2, &val_0xa2);

    if (is_LNB_enable != MtFe_True) {
	val_0xa1 |= 0x40;

	/*set LNB_EN pin HIGH or LOW */
	if (handle->board_cfg.bPolarLNBEnable) {
	    val_0xa2 &= ~0x02;
	} else {
	    val_0xa2 |= 0x02;
	}

	/*set V_SEL Pin mode*/

	if (handle->board_cfg.bPolarLNBStandby) {
	    val_0xa2 |= 0x01;
	} else {
	    val_0xa2 &= ~0x01;
	}

	/*set DiseQc_OUT mode*/
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    val_0xa2 |= 0xc0;
	} else {
	    val_0xa2 &= ~0xc0;
	    val_0xa2 |= 0x80;
	}
    } else {
	if (handle->board_cfg.bPolarLNBEnable) {
	    val_0xa2 |= 0x02;
	} else {
	    val_0xa2 &= ~0x02;
	}

	if (handle->board_cfg.bPolarLNB13V) {
	    if (voltage_type == MtFeLNB_13V)
		val_0xa2 |= 0x01;
	    else
		val_0xa2 &= ~0x01;
	} else {
	    if (voltage_type == MtFeLNB_13V)
		val_0xa2 &= ~0x01;
	    else
		val_0xa2 |= 0x01;
	}

	if (is_22k_enable == MtFe_True) {
	    val_0xa1 |= 0x04;
	    val_0xa1 &= ~0x03;
	    val_0xa1 &= ~0x40;
	    val_0xa2 &= ~0xc0;
	} else {
	    if (handle->board_cfg.bPolarDiSEqCOut) {
		val_0xa2 |= 0xc0;
	    } else {
		val_0xa2 &= ~0xc0;
		val_0xa2 |= 0x80;
	    }
	}
    }

    if (is_envelop_mode != MtFe_True) {
	val_0xa2 &= ~0x20;
    } else {
	val_0xa2 |= 0x20;
    }

    handle->dmd_set_reg(handle, 0xa2, val_0xa2);
    handle->dmd_set_reg(handle, 0xa1, val_0xa1);

    handle->lnb_cfg.b22K = is_22k_enable;
    handle->lnb_cfg.mLnbVoltage = voltage_type;

    return MtFeErr_Ok;
}

/*************************************************************************************
** Function: mt_fe_dmd_cs8k_sat_DiSEqC_send_tone_burst
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
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_send_tone_burst(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
    U8 val;
    S32 time_out;
    handle->dmd_get_reg(handle, 0xa2, &val);
    val &= ~0xc0;
    if (is_envelop_mode != MtFe_True) {
	val &= ~0x20;
    } else {
	val |= 0x20;
    }
    handle->dmd_set_reg(handle, 0xa2, val);

    if (mode == MtFeDiSEqCToneBurst_Moulated)
	handle->dmd_set_reg(handle, 0xa1, 0x01);
    else
	handle->dmd_set_reg(handle, 0xa1, 0x02);

    handle->mt_sleep(13);

    time_out = 5;
    do {
	handle->dmd_get_reg(handle, 0xa1, &val);
	if ((val & 0x40) == 0)
	    break;

	handle->mt_sleep(1);
	time_out--;

    } while (time_out > 0);

    handle->dmd_get_reg(handle, 0xa2, &val);
    if (handle->board_cfg.bPolarDiSEqCOut) {
	val |= 0xc0;
    } else {
	val &= ~0xc0;
	val |= 0x80;
    }
    handle->dmd_set_reg(handle, 0xa2, val);

    return MtFeErr_Ok;
}

/*****************************************************************************
** Function: mt_fe_dmd_cs8k_sat_DiSEqC_send_receive_msg
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
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_send_msg(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
    U8 i;
    U8 tmp;
    S32 time_out;

#if 0
    if(handle->lnb_agent_priv != NULL)
    {
      nim_diseqc_cmd_t diseqc_cmd = {0};
      diseqc_cmd.mode = NIM_DISEQC_BYTES;
      diseqc_cmd.p_tx_buf = msg->data_send;
      diseqc_cmd.p_rx_buf = msg->data_receive;
      diseqc_cmd.tx_len = msg->size_send;
      diseqc_cmd.rx_len = msg->size_receive;
      nim_diseqc_ctrl(handle->lnb_agent_priv, &diseqc_cmd);
      return MtFeErr_Ok;
    }
#endif

    if (msg->size_send > 8)
	return MtFeErr_Param;

    handle->dmd_get_reg(handle, 0xa2, &tmp);
    tmp &= ~0xc0;
    if (msg->is_envelop_mode != TRUE) {
	tmp &= ~0x20;
    } else {
	tmp |= 0x20;
    }
    handle->dmd_set_reg(handle, 0xa2, tmp);

    for (i = 0; i < msg->size_send; i++) {
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
    while (time_out > 0) {
	handle->mt_sleep(10);
	time_out -= 10;

	handle->dmd_get_reg(handle, 0xa1, &tmp);
	if ((tmp & 0x40) == 0)
	    break;
    }

    if (time_out <= 0) {
	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    tmp |= 0xc0;
	} else {
	    tmp &= ~0xc0;
	    tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);

	return MtFeErr_TimeOut;
    }

    return MtFeErr_Ok;

#if 0
	if (!(msg->is_enable_receive))
	{
		handle->dmd_get_reg(handle, 0xa2, &tmp);
		if(handle->board_cfg.bPolarDiSEqCOut)
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
		tmp =(U8)((tmp >> 3) & 0x07);
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
		if(handle->board_cfg.bPolarDiSEqCOut)
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
		tmp =(U8)((tmp >> 3) & 0x07);
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
		if(handle->board_cfg.bPolarDiSEqCOut)
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

	for (i = 0; i < msg->size_receive; i ++)
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
		if(handle->board_cfg.bPolarDiSEqCOut)
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
	if(handle->board_cfg.bPolarDiSEqCOut)
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
#endif
}

/* user must call send message function before call recieve message function */
MT_FE_RET mt_fe_dmd_cs8k_sat_DiSEqC_receive_msg(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
    U8 i;
    U8 tmp, tmp1;
    S32 time_out;

    if (!(msg->is_enable_receive)) {
	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    tmp |= 0xc0;
	} else {
	    tmp &= ~0xc0;
	    tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);

	return MtFeErr_Ok;
    }

    time_out = 170;
    while (time_out > 0) {
	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp = (U8)((tmp >> 3) & 0x07);
	if (tmp != 0)
	    break;

	handle->mt_sleep(10);
	time_out -= 10;
    }

    if (time_out <= 0) {
	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    tmp |= 0xc0;
	} else {
	    tmp &= ~0xc0;
	    tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);

	return MtFeErr_TimeOut;
    }

    /*	1.5 * 9 * 8	= 108ms	*/
    time_out = 150;
    tmp1 = tmp;
    while (time_out > 0) {
	handle->mt_sleep(15);
	time_out -= 15;

	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp = (U8)((tmp >> 3) & 0x07);
	if (tmp == tmp1 || tmp == 7)
	    break;

	tmp1 = tmp;
    }

    if (time_out <= 0) {
	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    tmp |= 0xc0;
	} else {
	    tmp &= ~0xc0;
	    tmp |= 0x80;
	}
	handle->dmd_set_reg(handle, 0xa2, tmp);

	return MtFeErr_TimeOut;
    }

    msg->size_receive = tmp;
    handle->dmd_set_reg(handle, 0xa1, 0x40);

    for (i = 0; i < msg->size_receive; i++) {
	handle->dmd_get_reg(handle, (U8)(0xa3 + i), &(msg->data_receive[i]));
    }

    /*	Indicates the parity error occurs	*/
    handle->dmd_get_reg(handle, 0xab, &tmp);
    tmp <<= (8 - msg->size_receive);
    tmp &= 0xFF;
    if (tmp != 0) {
	handle->dmd_get_reg(handle, 0xa1, &tmp);
	tmp &= ~0x80;
	tmp |= 0x40;
	handle->dmd_set_reg(handle, 0xa1, tmp);

	handle->dmd_get_reg(handle, 0xa2, &tmp);
	if (handle->board_cfg.bPolarDiSEqCOut) {
	    tmp |= 0xc0;
	} else {
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
    if (handle->board_cfg.bPolarDiSEqCOut) {
	tmp |= 0xc0;
    } else {
	tmp &= ~0xc0;
	tmp |= 0x80;
    }
    handle->dmd_set_reg(handle, 0xa2, tmp);

    return MtFeErr_Ok;
}
/**********************************************************************
** Function: mt_fe_cs8k_sat_blindscan_abort
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
MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan_abort(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BOOL bs_abort)
{
    handle->global_cfg.bCancelBs = (bs_abort == MtFe_True) ? TRUE : FALSE;

    return MtFeErr_Ok;
}

/*********************************************************************************
** Function: mt_fe_dmd_cs8k_sat_register_blindscan_callback
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
MT_FE_RET mt_fe_dmd_cs8k_sat_register_notify(void (*callback)(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_MSG msg, void *p_tp_info))
{
    if (callback == NULL)
	return MtFeErr_NullPointer;

    mt_fe_cs8k_sat_bs_notify = callback;

    return MtFeErr_Ok;
}

/******************************************************************************************
** Function: mt_fe_dmd_cs8k_sat_blindscan
**
**
** Description:	blind scan satellite from the start freq. to the end freq.
**
**
** Inputs:
**
**		Parameter			type								description
**	---------------------------------------------------------------------------------------
**		begin_freq_MHz		U32									blind scan start freq.
**		end_freq_MHz		U32									blind scan stop freq.
**
**
** Outputs:
**
**		Paramet			type								description
**	---------------------------------------------------------------------------------------
**		p_bs_info			MT_FE_BS_TP_INFO* 	blind scan result pointer
**
**
********************************************************************************************/
MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info, U32 is_connect)
{
    U8 times;
    U32 begin_freq_KHz, end_freq_KHz;

    if ((p_bs_info->p_tp_info == NULL) || (p_bs_info->tp_max_num == 0))
	return MtFeErr_Param;

    mt_fe_dmd_cs8k_sat_global_reset(handle);

    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSStart, 0);

    handle->global_cfg.iScannedTpCnt = 0;
    handle->global_cfg.iLockedTpCnt = 0;
    handle->global_cfg.bCancelBs = FALSE;
    handle->global_cfg.iTotalBsTimes = p_bs_info->bs_times;

    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;
    handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2)); // XOR

    begin_freq_KHz = begin_freq_MHz * 1000;
    end_freq_KHz = end_freq_MHz * 1000;

    p_bs_info->tp_num = 0;

    for (times = MT_FE_BS_TIMES_1ST; times < (p_bs_info->bs_times); times++) {
	p_bs_info->bNewGain = TRUE;
	p_bs_info->iPreGain = 0;
	p_bs_info->iCurGain = 0;
	p_bs_info->bs_cur_loop = times;

	if (p_bs_info->bs_algorithm == MT_FE_BS_ALGORITHM_A) {
	    _mt_fe_dmd_cs8k_sat_bs_A(handle, begin_freq_KHz, end_freq_KHz, p_bs_info, times, is_connect);
	} else if (p_bs_info->bs_algorithm == MT_FE_BS_ALGORITHM_B) {
	    _mt_fe_dmd_cs8k_sat_bs_B(handle, begin_freq_KHz, end_freq_KHz, p_bs_info, times, is_connect);
	} else {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSFinish, 0);
	    return MtFeErr_Fail;
	}

	if (handle->global_cfg.bCancelBs) {
	    handle->global_cfg.bCancelBs = FALSE;
	    break;
	}
    }

    handle->mt_sleep(1000);

    _mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP(handle, p_bs_info, (U32)FREQ_MAX_KHz);

    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSFinish, 0);

    return MtFeErr_Ok;
}

/******************************************************************
** Function: _mt_fe_dmd_cs8k_sat_set_ts_divide_ratio
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_ts_divide_ratio(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TYPE type, U8 dr_high, U8 dr_low)
{
    U8 val, tmp1, tmp2;

    tmp1 = dr_high;
    tmp2 = dr_low;

/* Tscancode: duplicate branches for 'if' and 'else'. */
//    if (type == MtFeType_DvbS2) {
	tmp1 -= 1;
	tmp2 -= 1;

	tmp1 &= 0x3f;
	tmp2 &= 0x3f;

	//handle->dmd_set_reg(handle, 0xfe, 0x20);	// default value

	handle->dmd_get_reg(handle, 0xfe, &val); // ci_divrange_h_1 bits[5:2]
	val &= 0xF0;                             // bits[3:0]
	val |= (tmp1 >> 2) & 0x0f;
	handle->dmd_set_reg(handle, 0xfe, val);

	val = (U8)((tmp1 & 0x03) << 6); // ci_divrange_h_0 bits[1:0]
	val |= tmp2;                    // ci_divrange_l   bits[5:0]
	handle->dmd_set_reg(handle, 0xea, val);
/*
    } else // DVB-S
    {
	tmp1 -= 1;
	tmp2 -= 1;

	tmp1 &= 0x3f;
	tmp2 &= 0x3f;

	//handle->dmd_set_reg(handle, 0xfe, 0x20);	// default value

	handle->dmd_get_reg(handle, 0xfe, &val); // ci_divrange_h_1 bits[5:2]
	val &= 0xF0;                             // bits[3:0]
	val |= (tmp1 >> 2) & 0x0f;
	handle->dmd_set_reg(handle, 0xfe, val);

	val = (U8)((tmp1 & 0x03) << 6); // ci_divrange_h_0 bits[1:0]
	val |= tmp2;                    // ci_divrange_l   bits[5:0]
	handle->dmd_set_reg(handle, 0xea, val);
    }
*/

    return MtFeErr_Ok;
}

/******************************************************************
** Function: _mt_fe_dmd_cs8k_sat_set_ts_out_mode
**
**
** Description:	set the TS output mode
**
**
** Inputs:
**
**	Parameter				Type		Description
**	----------------------------------------------------------
**	mode					U8			passed by up level
**
**	MtFeTsOutMode_Serial	1			Serial
**	MtFeTsOutMode_Parallel	2			Parallel
**	MtFeTsOutMode_Common	3			Common Interface
**
** Outputs:		none
**
**
*******************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_ts_out_mode(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TS_OUT_MODE mode)
{
    U8 tmp, tmp1, tmp2, val_0x08, val = 0;

    handle->dmd_get_reg(handle, 0x08, &val_0x08);

    /*set DVBS mode*/
    tmp = (U8)(val_0x08 & (~0x04));
    handle->dmd_set_reg(handle, 0x08, tmp);

    if (mode == MtFeTsOutMode_Common) {
	//6MHz TS clock output
	tmp1 = 6;
	tmp2 = 6;
    } else if (mode == MtFeTsOutMode_Parallel) {
	if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_12_MHz) {
	    tmp1 = 4;
	    tmp2 = 4;
	} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_16_MHz) {
	    tmp1 = 3;
	    tmp2 = 3;
	} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_19_p_2_MHz) {
	    tmp1 = 3;
	    tmp2 = 2;
	/* Tscancode: duplicate branches for 'if' and 'else'. */
	//} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_24_MHz) {
	    //tmp1 = 2;
	    //tmp2 = 2;
	} else {
	    tmp1 = 2;
	    tmp2 = 2;
	}
    } else // if(mode == MtFeTsOutMode_Serial)
    {
	tmp1 = 0;
	tmp2 = 0;
    }

    _mt_fe_dmd_cs8k_sat_set_ts_divide_ratio(handle, MtFeType_DvbS, tmp1, tmp2);

    /*set DVBS2 mode*/
    tmp = (U8)(val_0x08 | 0x04);
    handle->dmd_set_reg(handle, 0x08, tmp);

    if (mode == MtFeTsOutMode_Common) {
	//6MHz TS clock output
	tmp1 = 8;
	tmp2 = 8;
    } else if (mode == MtFeTsOutMode_Parallel) {
	if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_12_MHz) {
	    tmp1 = 6;
	    tmp2 = 6;
	} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_16_MHz) {
	    tmp1 = 5;
	    tmp2 = 4;
	} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_19_p_2_MHz) {
	    /*actually 18MHz in DVBS2 mode*/
	    tmp1 = 4;
	    tmp2 = 4;
	} else if (handle->ts_cfg.mMaxClock == MtFeTSOut_Max_Clock_24_MHz) {
	    tmp1 = 3;
	    tmp2 = 3;
	} else {
	    tmp1 = 2;
	    tmp2 = 2;
	}
    } else // if(MT_FE_TS_OUTPUT_MODE == MtFeTsOutMode_Serial)
    {
	tmp1 = 0;
	tmp2 = 0;
    }

    _mt_fe_dmd_cs8k_sat_set_ts_divide_ratio(handle, MtFeType_DvbS2, tmp1, tmp2);

    handle->dmd_get_reg(handle, 0xfd, &tmp);
    if (mode == MtFeTsOutMode_Parallel) {
	tmp &= ~0x01;
	tmp &= ~0x04;
    } else if (mode == MtFeTsOutMode_Serial) {
	tmp &= ~0x01;
	tmp |= 0x04;
    } else {
	tmp |= 0x01;
	tmp &= ~0x04;
    }

    if (handle->ts_cfg.bRisingEdge) {
	tmp &= ~0xf8;
	tmp |= 0x02;
    } else {
	tmp &= ~0xb8;
	tmp |= 0x42;
    }
    handle->dmd_set_reg(handle, 0xfd, tmp);

    handle->dmd_set_reg(handle, 0x08, val_0x08);

    val = 0;
    if (mode != MtFeTsOutMode_Serial) {
	tmp = MT_FE_ENHANCE_TS_PIN_LEVEL_PARALLEL_CI;

	val |= tmp & 0x03;
	val |= (tmp << 2) & 0x0C;
	val |= (tmp << 4) & 0x30;
	val |= (tmp << 6) & 0xC0;
    } else {
	val = 0x00;
    }
    handle->dmd_set_reg(handle, 0x20, val);

    handle->ts_cfg.mTsMode = mode;

    return MtFeErr_Ok;
}

/*****************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_set_demod
**
**
** Description:	set some registers according the param.
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------------------
**	sym_rate_KSs	U8			symbol rate passed by app level
**	type			MT_FE_TYPE	passed by up level
**
**
** Outputs:
**
**
******************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_demod(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs, MT_FE_TYPE type)
{
    MT_FE_RET ret = MtFeErr_Ok;
    U8 val, tmp;
    //	U32			ts_clk = 24000;

    _mt_fe_dmd_cs8k_sat_init_reg(handle, reg_tbl_def_cs8000_sat, sizeof(reg_tbl_def_cs8000_sat) / 2);

    if (handle->board_cfg.bSpectrumInverted) // 0x4d, bit 1
    {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val |= 0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    } else {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val &= ~0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    }

    if (handle->board_cfg.bAGCPolar) // 0x30, bit 4
    {
	handle->dmd_get_reg(handle, 0x30, &val);
	val |= 0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    } else {
	handle->dmd_get_reg(handle, 0x30, &val);
	val &= ~0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    }

    if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS2022)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else
	handle->dmd_set_reg(handle, 0x33, 0x35);

    handle->dmd_get_reg(handle, 0xC9, &tmp);
    tmp |= 0x08;
    handle->dmd_set_reg(handle, 0xC9, tmp);

    if (sym_rate_KSs <= 3000) {
	handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 32 * 100 / 64 = 400
	handle->dmd_set_reg(handle, 0xc8, 0x20);
	handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
	handle->dmd_set_reg(handle, 0xc7, 0x00);
    } else if (sym_rate_KSs <= 10000) {
	handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 16 * 100 / 64 = 200
	handle->dmd_set_reg(handle, 0xc8, 0x10);
	handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
	handle->dmd_set_reg(handle, 0xc7, 0x00);
    } else {
	handle->dmd_set_reg(handle, 0xc3, 0x08); // 8 * 6 * 100 / 64 = 75
	handle->dmd_set_reg(handle, 0xc8, 0x06);
	handle->dmd_set_reg(handle, 0xc4, 0x08); // 8 * 0 * 100 / 128 = 0
	handle->dmd_set_reg(handle, 0xc7, 0x00);
    }

	_mt_fe_dmd_cs8k_sat_set_sym_rate(handle, sym_rate_KSs);


	if(handle->tp_cfg.bHavePLS)
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

	return ret;
}

/*****************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_set_sym_rate
**
**
** Description:	set symbol rate
**
**
** Inputs:
**
**	Parameter		Type		Description
**	----------------------------------------------------------------------
**	sym_rate_KSs	U8			symbol rate passed by app level
**
**
** Outputs:
**
**
*******************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_sym_rate(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs)
{
    U32 tmp;
    U8 reg_0x61, reg_0x62;

    tmp = ((sym_rate_KSs << 15) + (handle->global_cfg.iMclkKHz / 4)) / (handle->global_cfg.iMclkKHz / 2);

    reg_0x61 = (U8)(tmp & 0x00FF);
    reg_0x62 = (U8)((tmp & 0xFF00) >> 8);

    handle->dmd_set_reg(handle, 0x61, reg_0x61);
    handle->dmd_set_reg(handle, 0x62, reg_0x62);

    return MtFeErr_Ok;
}

/*****************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_get_sym_rate
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_sym_rate(MT_FE_CS8000_SAT_Device_Handle handle, U32 *sym_rate_KSs)
{
    U16 tmp;
    U32 sym_rate_tmp;
    U8 val_0x6d, val_0x6e;

    handle->dmd_get_reg(handle, 0x6d, &val_0x6d);
    handle->dmd_get_reg(handle, 0x6e, &val_0x6e);

    tmp = (U16)((val_0x6e << 8) | val_0x6d);

    sym_rate_tmp = tmp * handle->global_cfg.iMclkKHz;
    sym_rate_tmp /= (1 << 16);

    *sym_rate_KSs = sym_rate_tmp;

    return MtFeErr_Ok;
}

/*************************************************************************
** Function:  _mt_fe_dmd_cs8k_sat_set_carrier_offset
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_set_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 carrier_offset_KHz)
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
** Function: _mt_fe_dmd_cs8k_sat_get_carrier_offset
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 *carrier_offset_KHz)
{
    S32 val;
    U8 tmp, val_0x5e, val_0x5f;

    handle->dmd_get_reg(handle, 0x5d, &tmp);
    tmp &= 0xf8;
    handle->dmd_set_reg(handle, 0x5d, tmp);

    handle->dmd_get_reg(handle, 0x5e, &val_0x5e);
    handle->dmd_get_reg(handle, 0x5f, &val_0x5f);
    val = (val_0x5f << 8) | val_0x5e;

    if ((val & 0x8000) == 0x8000)
	val -= (1 << 16);

    *carrier_offset_KHz = (val * handle->global_cfg.iMclkKHz) / (1 << 16);

    return MtFeErr_Ok;
}

/***********************************************************************
** Function: _mt_fe_dmd_cs8k_sat_get_total_carrier_offset
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_total_carrier_offset(MT_FE_CS8000_SAT_Device_Handle handle, S32 *carrier_offset_KHz)
{
    U8 tmp, val_0x5e, val_0x5f;
    S32 val_0x5e_0x5f_1, val_0x5e_0x5f_2, nval_1, nval_2;

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

    return MtFeErr_Ok;
}

/*************************************************************************
** Function:  _mt_fe_dmd_cs8k_sat_get_fec
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_fec(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TP_INFO *p_info, MT_FE_TYPE tp_type)
{
    U8 tmp, val_0x7E;

    if (tp_type == MtFeType_DvbS2) {
	handle->dmd_get_reg(handle, 0x7e, &val_0x7E);
	tmp = (U8)(val_0x7E & 0x0f);
	switch (tmp) {
	case 0:
	    p_info->code_rate = MtFeCodeRate_1_4;
	    break;
	case 1:
	    p_info->code_rate = MtFeCodeRate_1_3;
	    break;
	case 2:
	    p_info->code_rate = MtFeCodeRate_2_5;
	    break;
	case 3:
	    p_info->code_rate = MtFeCodeRate_1_2;
	    break;
	case 4:
	    p_info->code_rate = MtFeCodeRate_3_5;
	    break;
	case 5:
	    p_info->code_rate = MtFeCodeRate_2_3;
	    break;
	case 6:
	    p_info->code_rate = MtFeCodeRate_3_4;
	    break;
	case 7:
	    p_info->code_rate = MtFeCodeRate_4_5;
	    break;
	case 8:
	    p_info->code_rate = MtFeCodeRate_5_6;
	    break;
	case 9:
	    p_info->code_rate = MtFeCodeRate_8_9;
	    break;
	case 10:
	    p_info->code_rate = MtFeCodeRate_9_10;
	    break;
	default:
	    p_info->code_rate = MtFeCodeRate_Undef;
	    break;
	}
    } else {
	handle->dmd_get_reg(handle, 0xe6, &tmp);
	tmp = (U8)(tmp >> 5);
	switch (tmp) {
	case 0:
	    p_info->code_rate = MtFeCodeRate_7_8;
	    break;
	case 1:
	    p_info->code_rate = MtFeCodeRate_5_6;
	    break;
	case 2:
	    p_info->code_rate = MtFeCodeRate_3_4;
	    break;
	case 3:
	    p_info->code_rate = MtFeCodeRate_2_3;
	    break;
	case 4:
	    p_info->code_rate = MtFeCodeRate_1_2;
	    break;
	default:
	    p_info->code_rate = MtFeCodeRate_Undef;
	    break;
	}
    }

    return MtFeErr_Ok;
}

/*****************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_get_current_ts_mode
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_get_current_ts_mode(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_TS_OUT_MODE *p_ts_mode)
{
    U8 val_0xfd;

    *p_ts_mode = MtFeTsOutMode_Common;

    handle->dmd_get_reg(handle, 0xfd, &val_0xfd);

    if ((val_0xfd & 0x01) == 0x00) {
	*p_ts_mode = ((val_0xfd & 0x04) == 0) ? MtFeTsOutMode_Parallel : MtFeTsOutMode_Serial;
    } else {
	*p_ts_mode = MtFeTsOutMode_Common;
    }

    handle->ts_cfg.mTsMode = *p_ts_mode;

    return MtFeErr_Ok;
}

/**********************************************************************
** Function: _mt_fe_dmd_cs8k_sat_init_reg
**
**
** Description:	initialize the register of CS8000 Sat
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_init_reg(MT_FE_CS8000_SAT_Device_Handle handle, const U8 (*p_reg_tabl)[2], S32 size)
{
    MT_FE_RET ret;
    U32 i;

    for (i = 0; i < (U32)size; i++) {
	ret = handle->dmd_set_reg(handle, p_reg_tabl[i][0], p_reg_tabl[i][1]);

	if (ret != MtFeErr_Ok)
	    return ret;
    }

    return MtFeErr_Ok;
}

/*******************************************************************
** Function: _mt_fe_dmd_cs8k_sat_download_fw
**
**
** Description:	download the mcu code to CS8000 Sat
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_download_fw(MT_FE_CS8000_SAT_Device_Handle handle, const U8 *p_fw)
{
    U32 i;
    MT_FE_RET ret = MtFeErr_Ok;

    handle->dmd_set_reg(handle, 0xb2, 0x01);

    for (i = 0; i < 192; i++) {
	ret = _mt_fe_cs8k_sat_write_fw(handle, 0xb0, (U8 *)&(p_fw[64 * i]), 64);
	if (ret != MtFeErr_Ok) {
	    break;
	}
    }

    handle->dmd_set_reg(handle, 0xb2, 0x00);

    if (ret != MtFeErr_Ok)
	return MtFeErr_I2cErr;

    return MtFeErr_Ok;
}

/*************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_set_reg
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_set_reg(MT_FE_CS8000_SAT_Device_Handle handle, U8 bs_times)
{
    U8 tmp, sm_buf;
    U8 TMP1, TMP2, TMP3, TMP4;

    TMP1 = (handle->bs_cfg.ave_times / 32 - 1) * 16 + (handle->bs_cfg.data_length / 128 - 1); // = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
    TMP2 = 64 + handle->bs_cfg.psd_overlap / 16;                                              // = (64 + 96/16) = (64 + 6) = 0x46
    TMP3 = handle->bs_cfg.psd_scale * 32;                                                     // = (0*32) = 0
    TMP4 = handle->bs_cfg.notch_range_f * 16 + handle->bs_cfg.start_range_f;                  // = (8*16 + 15) = 0x8F

    switch (bs_times) {
    case MT_FE_BS_TIMES_1ST:
	if (handle->global_cfg.iTotalBsTimes == 1)
	    sm_buf = handle->bs_cfg.sm_buf_2nd; // ONE_LOOP_SM_BUF
	else
	    sm_buf = handle->bs_cfg.sm_buf_1st; // SM_BUF_1ST		// TWO_LOOP_SM_BUF_1ST
	break;

    case MT_FE_BS_TIMES_2ND:
	if (handle->global_cfg.iTotalBsTimes == 2)
	    sm_buf = handle->bs_cfg.sm_buf_3rd; // TWO_LOOP_SM_BUF_2ND
	else
	    sm_buf = handle->bs_cfg.sm_buf_2nd; // SM_BUF_2ND
	break;

    case MT_FE_BS_TIMES_3RD:
	sm_buf = handle->bs_cfg.sm_buf_3rd; // SM_BUF_3RD;
	break;

    default:
	sm_buf = handle->bs_cfg.sm_buf_2nd; // ONE_LOOP_SM_BUF;
	break;
    }

    handle->dmd_set_reg(handle, 0xB2, 0x01);
    handle->dmd_set_reg(handle, 0x00, 0x01);

    handle->dmd_set_reg(handle, 0x4A, 0x00);

    handle->dmd_get_reg(handle, 0x4D, &tmp);
    tmp |= 0x91;
    handle->dmd_set_reg(handle, 0x4D, tmp);

    handle->dmd_get_reg(handle, 0x90, &tmp);
    tmp |= TMP1;
    handle->dmd_set_reg(handle, 0x90, tmp);

    if (sm_buf == handle->bs_cfg.sm_buf_1st)
	tmp = 128 + TMP2;
    else
	tmp = TMP2;

    handle->dmd_set_reg(handle, 0x91, tmp);

    tmp = (U8)(TMP3 + sm_buf - 1);
    tmp |= 0x80;
    handle->dmd_set_reg(handle, 0x92, tmp);

    handle->dmd_get_reg(handle, 0x93, &tmp);
    tmp &= 0x0F;
    tmp |= TMP4;
    handle->dmd_set_reg(handle, 0x93, tmp);

    handle->dmd_get_reg(handle, 0x94, &tmp);
    switch (bs_times) {
    case MT_FE_BS_TIMES_1ST:
	if (handle->global_cfg.iTotalBsTimes == 1)
	    tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_2nd); // THR_FACTOR_ONE_LOOP
	else
	    tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_1st); // TWO_LOOP_THR_FACTOR_1ST
	break;

    case MT_FE_BS_TIMES_2ND:
	if (handle->global_cfg.iTotalBsTimes == 2)
	    tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_3rd);
	else
	    tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_2nd);
	break;

    case MT_FE_BS_TIMES_3RD:
	tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_3rd);
	break;

    default:
	tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_2nd);
	break;
    }
    handle->dmd_set_reg(handle, 0x94, tmp);

    switch (bs_times) {
    case MT_FE_BS_TIMES_1ST:
	if (handle->global_cfg.iTotalBsTimes == 1)
	    tmp = 64 + handle->bs_cfg.err_bound_factor_2nd;
	else
	    tmp = 64 + handle->bs_cfg.err_bound_factor_1st;
	break;

    case MT_FE_BS_TIMES_2ND:
	if (handle->global_cfg.iTotalBsTimes == 2)
	    tmp = 64 + handle->bs_cfg.err_bound_factor_3rd;
	else
	    tmp = 64 + handle->bs_cfg.err_bound_factor_2nd;
	break;

    case MT_FE_BS_TIMES_3RD:
	tmp = 64 + handle->bs_cfg.err_bound_factor_3rd;
	break;

    default:
	tmp = 64 + handle->bs_cfg.err_bound_factor_2nd;
	break;
    }
    handle->dmd_set_reg(handle, 0x95, tmp);

    switch (bs_times) {
    case MT_FE_BS_TIMES_1ST:
	if (handle->global_cfg.iTotalBsTimes == 1)
	    tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_2nd / 16 - 1;
	else
	    tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_1st / 16 - 1;

	break;

    case MT_FE_BS_TIMES_2ND:
	if (handle->global_cfg.iTotalBsTimes == 2)
	    tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_3rd / 16 - 1;
	else
	    tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_2nd / 16 - 1;
	break;

    case MT_FE_BS_TIMES_3RD:
	tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_3rd / 16 - 1;
	break;

    default:
	tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_2nd / 16 - 1;
	break;
    }
    handle->dmd_set_reg(handle, 0x97, tmp);

    switch (bs_times) {
    case MT_FE_BS_TIMES_1ST:
	if (handle->global_cfg.iTotalBsTimes == 1)
	    tmp = handle->bs_cfg.flat_thr_2nd;
	else
	    tmp = handle->bs_cfg.flat_thr_1st;
	break;

    case MT_FE_BS_TIMES_2ND:
	if (handle->global_cfg.iTotalBsTimes == 2)
	    tmp = handle->bs_cfg.flat_thr_3rd;
	else
	    tmp = handle->bs_cfg.flat_thr_2nd;
	break;

    case MT_FE_BS_TIMES_3RD:
	tmp = handle->bs_cfg.flat_thr_3rd;
	break;

    default:
	tmp = handle->bs_cfg.flat_thr_2nd;
	break;
    }
    handle->dmd_set_reg(handle, 0x99, tmp);

    handle->dmd_set_reg(handle, 0x30, (U8)(handle->board_cfg.bAGCPolar ? 0x18 : 0x08));

    handle->dmd_set_reg(handle, 0x32, 0x44);

    if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS2022)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else
	handle->dmd_set_reg(handle, 0x33, 0x35);

    handle->dmd_set_reg(handle, 0x35, 0x7F);

    handle->dmd_set_reg(handle, 0x4B, 0x04);
    handle->dmd_set_reg(handle, 0x56, 0x01);
    handle->dmd_set_reg(handle, 0xA0, 0x44);
    handle->dmd_set_reg(handle, 0x08, 0x83);

    handle->dmd_set_reg(handle, 0x70, 0x00);

    if (handle->board_cfg.bSpectrumInverted) // 0x4d, bit 1
    {
	handle->dmd_get_reg(handle, 0x4d, &tmp);
	tmp |= 0x02;
	handle->dmd_set_reg(handle, 0x4d, tmp);
    } else {
	handle->dmd_get_reg(handle, 0x4d, &tmp);
	tmp &= ~0x02;
	handle->dmd_set_reg(handle, 0x4d, tmp);
    }

    handle->dmd_set_reg(handle, 0x00, 0x00);
    handle->dmd_set_reg(handle, 0xB2, 0x00);

    return MtFeErr_Ok;
}

/*****************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_set_demod
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_set_demod(MT_FE_CS8000_SAT_Device_Handle handle, U32 sym_rate_KSs)
{
    U8 val;
    MT_FE_RET ret = MtFeErr_Ok;
    //	U32			ts_clk = 24000;

    ret = _mt_fe_dmd_cs8k_sat_init_reg(handle, reg_tbl_def_blindscan_cs8000_sat, sizeof(reg_tbl_def_blindscan_cs8000_sat) / 2);

    //handle->dmd_set_reg(handle, 0xc6, 0x40);

    if (handle->board_cfg.bSpectrumInverted) // 0x4d, bit 1
    {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val |= 0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    } else {
	handle->dmd_get_reg(handle, 0x4d, &val);
	val &= ~0x02;
	handle->dmd_set_reg(handle, 0x4d, val);
    }

    if (handle->board_cfg.bAGCPolar) // 0x30, bit 4
    {
	handle->dmd_get_reg(handle, 0x30, &val);
	val |= 0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    } else {
	handle->dmd_get_reg(handle, 0x30, &val);
	val &= ~0x10;
	handle->dmd_set_reg(handle, 0x30, val);
    }

    if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS2022)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else
	handle->dmd_set_reg(handle, 0x33, 0x35);

    if (sym_rate_KSs <= 2500) {
	handle->dmd_set_reg(handle, 0xc3, 0x08);
	handle->dmd_set_reg(handle, 0xc8, 0x00);
	handle->dmd_set_reg(handle, 0xc4, 0x04);
	handle->dmd_set_reg(handle, 0xc7, 0x1b);
    } else if (sym_rate_KSs <= 5000) {
	handle->dmd_set_reg(handle, 0xc3, 0x08);
	handle->dmd_set_reg(handle, 0xc8, 0x00);
	handle->dmd_set_reg(handle, 0xc4, 0x04);
	handle->dmd_set_reg(handle, 0xc7, 0x0b);
	/* Tscancode: duplicate branches for 'if' and 'else'. */
    //} else if (sym_rate_KSs <= 20000) {
	//handle->dmd_set_reg(handle, 0xc3, 0x08);
	//handle->dmd_set_reg(handle, 0xc8, 0x02);
	//handle->dmd_set_reg(handle, 0xc4, 0x04);
	//handle->dmd_set_reg(handle, 0xc7, 0x16);
    } else {
	handle->dmd_set_reg(handle, 0xc3, 0x08);
	handle->dmd_set_reg(handle, 0xc8, 0x02);
	handle->dmd_set_reg(handle, 0xc4, 0x04);
	handle->dmd_set_reg(handle, 0xc7, 0x16);
    }

    _mt_fe_dmd_cs8k_sat_set_sym_rate(handle, sym_rate_KSs);

    return ret;
}

/******************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP
**
**
** Description:	save the scanned TPs
**
**
** Inputs:
**
**		Parameter			type		description
**	---------------------------------------------------------------------------------------
**		freq_KHz			U32			freq. of the TP need to save
**		symbol_rate_KSs		U32			S.R. of the TP need to save
**		bs_times			U8			blind scan times
**
**
** Outputs:
**
**		Parameter		type						description
**	---------------------------------------------------------------------------------------
**		p_bs_info		MT_FE_BS_TP_INFO *			blind scan struture pointer
**
**
*******************************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                                 U32 freq_KHz,
                                                 U32 symbol_rate_KSs,
                                                 U8 bs_times)
{
    U16 i, j;
    MT_FE_TP_INFO *tmp = NULL;

    tmp = p_bs_info->p_tp_info;

    if (handle->global_cfg.iScannedTpCnt > (p_bs_info->tp_max_num)) {
	return MtFeErr_NoMemory;
    }

    if (((bs_times == MT_FE_BS_TIMES_1ST) || (handle->global_cfg.iTotalBsTimes == 1)) && !handle->board_cfg.bSpectrumInverted) {
	tmp[handle->global_cfg.iScannedTpCnt - 1].freq_KHz = freq_KHz;
	tmp[handle->global_cfg.iScannedTpCnt - 1].sym_rate_KSs = (U16)symbol_rate_KSs;
	tmp[handle->global_cfg.iScannedTpCnt - 1].dvb_type = MtFeType_DTV_Unknown;
	tmp[handle->global_cfg.iScannedTpCnt - 1].code_rate = MtFeCodeRate_Undef;
    } else {
	for (i = 0; i < handle->global_cfg.iScannedTpCnt; i++) {
	    if (handle->lnb_cfg.bUnicable) {
		S32 delta_freq_KHz, delta_sym_rate;

		delta_freq_KHz = freq_KHz - tmp[i].freq_KHz;
		delta_sym_rate = symbol_rate_KSs - tmp[i].sym_rate_KSs;

		if ((delta_freq_KHz < 1000) && (delta_freq_KHz > -1000) &&
		    (delta_sym_rate < 1000) && (delta_sym_rate > -1000)) {
		    if (tmp[i].dvb_type == MtFeType_DTV_Unknown) {
			tmp[i].freq_KHz = freq_KHz;
			if (delta_sym_rate > 0)
			    tmp[i].sym_rate_KSs = (U16)symbol_rate_KSs;
		    } else if (tmp[i].dvb_type == MtFeType_DTV_Checked) {
			if (delta_sym_rate > 0) {
			    tmp[i].freq_KHz = freq_KHz;
			    tmp[i].sym_rate_KSs = (U16)symbol_rate_KSs;
			    tmp[i].dvb_type = MtFeType_DTV_Unknown;
			}
		    }

		    return MtFeErr_Ok;
		}
	    }

	    if (freq_KHz <= tmp[i].freq_KHz) {
		for (j = handle->global_cfg.iScannedTpCnt; j > i; j--) {
		    tmp[j].freq_KHz = tmp[j - 1].freq_KHz;
		    tmp[j].sym_rate_KSs = tmp[j - 1].sym_rate_KSs;
		    tmp[j].dvb_type = tmp[j - 1].dvb_type;
		    tmp[j].code_rate = tmp[j - 1].code_rate;
		}

		tmp[j].freq_KHz = freq_KHz;
		tmp[j].sym_rate_KSs = (U16)symbol_rate_KSs;
		tmp[j].dvb_type = MtFeType_DTV_Unknown;
		tmp[j].code_rate = MtFeCodeRate_Undef;

		break;
	    }
	}

	if (i == handle->global_cfg.iScannedTpCnt) {
	    tmp[handle->global_cfg.iScannedTpCnt - 1].freq_KHz = freq_KHz;
	    tmp[handle->global_cfg.iScannedTpCnt - 1].sym_rate_KSs = (U16)symbol_rate_KSs;
	    tmp[handle->global_cfg.iScannedTpCnt - 1].dvb_type = MtFeType_DTV_Unknown;
	    tmp[handle->global_cfg.iScannedTpCnt - 1].code_rate = MtFeCodeRate_Undef;
	}
    }

    return MtFeErr_Ok;
}

/********************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 compare_freq_KHz)
{
    U16 i;
    U16 scanned_TP_num, index = 0, move_steps, sym_tmp;
    U32 fre_tmp;

    scanned_TP_num = handle->global_cfg.iScannedTpCnt;

    for (i = 0; i < scanned_TP_num; i++) {
	if (p_bs_info->p_tp_info[i].freq_KHz >= compare_freq_KHz) {
	    index = i;
	    break;
	}
    }

    if (i == scanned_TP_num)
	index = scanned_TP_num;

    i = 0;
    move_steps = 0;

    while (i <= index) {
	if (p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked) {
	    i++;
	    continue;
	}

	while ((p_bs_info->p_tp_info[i].dvb_type == MtFeType_DTV_Checked) && (i <= index)) {
	    move_steps++;
	    i++;
	}

	while ((p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Checked) && (i <= index)) {
	    fre_tmp = p_bs_info->p_tp_info[i - move_steps].freq_KHz;
	    sym_tmp = p_bs_info->p_tp_info[i - move_steps].sym_rate_KSs;

	    p_bs_info->p_tp_info[i - move_steps].freq_KHz = p_bs_info->p_tp_info[i].freq_KHz;
	    p_bs_info->p_tp_info[i - move_steps].sym_rate_KSs = p_bs_info->p_tp_info[i].sym_rate_KSs;
	    p_bs_info->p_tp_info[i - move_steps].dvb_type = p_bs_info->p_tp_info[i].dvb_type;
	    p_bs_info->p_tp_info[i - move_steps].code_rate = p_bs_info->p_tp_info[i].code_rate;

	    p_bs_info->p_tp_info[i].freq_KHz = fre_tmp;
	    p_bs_info->p_tp_info[i].sym_rate_KSs = sym_tmp;
	    p_bs_info->p_tp_info[i].dvb_type = MtFeType_DTV_Checked;
	    p_bs_info->p_tp_info[i].code_rate = MtFeCodeRate_Undef;

	    i++;
	}
    }

    return MtFeErr_Ok;
}

/********************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_compare_deal_TP
**
**
** Description:	process the scanned new TP in each blind scan
**
**
** Inputs:
**
**		Parameter			type					description
**	---------------------------------------------------------------------------------------
**		cur_scan_freq_KHz	U32						start frequency in each blind scan bandwidth
**		bs_times			U8						blind scan times
**
**
** Outputs:
**
**		Parameter			type					description
**	------------------------------------------------------------------------------------
**		p_bs_info			MT_FE_BS_TP_INFO *		tp structure pointer
**
**
********************************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_compare_deal_TP(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info,
                                                 U32 cur_scan_freq_KHz,
                                                 U8 bs_times)
{
    MT_FE_RET ret = MtFeErr_Ok;

    U8 tmp1, tmp2, tmp3, tmp4, covered_ratio;
    U16 i, last_check_index;
    U32 centerfreqKHz_tpA, symrateKSs_tpA;
    U32 centerfreqKHz_tpB, symrateKSs_tpB;
    U32 left_boundary, right_boundary;
    S32 s_value, delta_freq_KHz, delta_symbol_rate_KSs, freq_delta_KHz;
    S32 similar_symbol_rate_KSs, freq_offset_tmp, symrate_offset_tmp;

#if (MT_FE_DEBUG != 0)
    MT_FE_TP_INFO tp_info;
#endif

    s_value = 0;
    covered_ratio = 0;

    centerfreqKHz_tpA = 0;
    centerfreqKHz_tpB = 0;
    symrateKSs_tpA = 0;
    symrateKSs_tpB = 0;

    handle->dmd_set_reg(handle, 0x9A, 0x20);
    handle->dmd_get_reg(handle, 0x9B, &tmp1);
    handle->dmd_get_reg(handle, 0x9B, &tmp2);
    handle->dmd_get_reg(handle, 0x9B, &tmp3);
    handle->dmd_get_reg(handle, 0x9B, &tmp4);

    symrateKSs_tpB = (U32)((((tmp4 & 0x03) << 8) | tmp3) * handle->global_cfg.iMclkKHz / FFT_LENGTH);

    s_value = (S32)(((tmp2 & 0x03) << 8) | tmp1);
    if (s_value >= FFT_LENGTH)
	s_value -= 1024;

    centerfreqKHz_tpB = cur_scan_freq_KHz + s_value * (handle->global_cfg.iMclkKHz / FFT_LENGTH);
    centerfreqKHz_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].freq_KHz;
    symrateKSs_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].sym_rate_KSs;

    if (centerfreqKHz_tpB < 945000) {
	return MtFeErr_Ok;
    }

#if 1
    if ((symrateKSs_tpB > 50000) || (symrateKSs_tpB <= 2000)) {
	return MtFeErr_Ok;
    }
#else
    if ((symrateKSs_tpB > 50000) || (symrateKSs_tpB < 1400)) {
	return MtFeErr_Ok;
    }
#endif
    if ((symrateKSs_tpB < 10000) && ((bs_times == MT_FE_BS_TIMES_1ST) && (handle->global_cfg.iTotalBsTimes > 1))) {
	return MtFeErr_Ok;
    }
    if ((symrateKSs_tpB > 15000) && (bs_times == MT_FE_BS_TIMES_3RD)) {
	return MtFeErr_Ok;
    }

#if 1
    if (symrateKSs_tpB > 45000) {
	symrateKSs_tpB = 45000;
    } else if (symrateKSs_tpB > 2000) {
	symrateKSs_tpB -= 400;
    } else {
	symrateKSs_tpB -= 500;
    }
#endif

#if (MT_FE_DEBUG != 0)
    tp_info.freq_KHz = centerfreqKHz_tpB;
    tp_info.sym_rate_KSs = (U16)symrateKSs_tpB;
    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpFind, &tp_info);
#endif

    if ((bs_times == MT_FE_BS_TIMES_1ST) || (handle->global_cfg.iTotalBsTimes == 1)) {
	handle->global_cfg.iScannedTpCnt++;
	ret = _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP(handle, p_bs_info, centerfreqKHz_tpB, symrateKSs_tpB, bs_times);
	if (ret != MtFeErr_Ok)
	    return ret;

	return MtFeErr_Ok;
    }

    while (!handle->global_cfg.bCancelBs) {
	right_boundary = centerfreqKHz_tpB + symrateKSs_tpB / 2;
	left_boundary = centerfreqKHz_tpA - symrateKSs_tpA / 2;

	if (right_boundary <= left_boundary) {
	    if (handle->global_cfg.iTpIndex > 0)
		handle->global_cfg.iTpIndex--;
	    else
		handle->global_cfg.iTpIndex = 0;

	    centerfreqKHz_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].freq_KHz;

	    handle->global_cfg.iScannedTpCnt++;
	    ret = _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP(handle, p_bs_info, centerfreqKHz_tpB, symrateKSs_tpB, bs_times);
	    if (ret != MtFeErr_Ok)
		return ret;

	    /*update tpA index*/
	    for (i = 0; i < handle->global_cfg.iScannedTpCnt; i++) {
		if (handle->global_cfg.bCancelBs)
		    break;

		if (centerfreqKHz_tpA == (p_bs_info->p_tp_info[i].freq_KHz)) {
		    handle->global_cfg.iTpIndex = i;
		    break;
		}
	    }

	    break;
	}

	if ((p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].dvb_type == MtFeType_DvbS) ||
	    (p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].dvb_type == MtFeType_DvbS2)) {
	    if (symrateKSs_tpB > 6000)
		covered_ratio = 2;
	    else
		covered_ratio = 3;

	    if (symrateKSs_tpB > 25000)
		freq_delta_KHz = 7000;
	    else if (symrateKSs_tpB > 10000)
		freq_delta_KHz = 4000;
	    else if (symrateKSs_tpB > 5000)
		freq_delta_KHz = 3000;
	    else if (symrateKSs_tpB > 3000)
		freq_delta_KHz = 2000;
	    else
		freq_delta_KHz = 1000;

	    if (centerfreqKHz_tpB >= centerfreqKHz_tpA) {
		delta_freq_KHz = centerfreqKHz_tpB - centerfreqKHz_tpA;
		delta_symbol_rate_KSs = (centerfreqKHz_tpA + symrateKSs_tpA / 2) - (centerfreqKHz_tpB - symrateKSs_tpB / 2);
	    } else {
		delta_freq_KHz = centerfreqKHz_tpA - centerfreqKHz_tpB;
		delta_symbol_rate_KSs = (centerfreqKHz_tpB + symrateKSs_tpB / 2) - (centerfreqKHz_tpA - symrateKSs_tpA / 2);
	    }

	    if ((delta_symbol_rate_KSs >= (S32)(symrateKSs_tpB / covered_ratio)) &&
	        (delta_freq_KHz <= (S32)(freq_delta_KHz))) {
		mt_fe_print(("\n\tOverlap Skip TP!!!!!!!   Freq. == %d\n", centerfreqKHz_tpB));
		break;
	    }
	} else {
	    if (symrateKSs_tpB > 25000) {
		freq_offset_tmp = 1000;
		symrate_offset_tmp = 400;
	    } else if (symrateKSs_tpB > 10000) {
		freq_offset_tmp = 800;
		symrate_offset_tmp = 250;
	    } else if (symrateKSs_tpB > 5000) {
		freq_offset_tmp = 500;
		symrate_offset_tmp = 200;
	    } else if (symrateKSs_tpB > 3000) {
		freq_offset_tmp = 500;
		symrate_offset_tmp = 200;
		/* Tscancode: duplicate branches for 'if' and 'else'. */
	    //} else if (symrateKSs_tpB >= 1000) {
		//freq_offset_tmp = 500;
		//symrate_offset_tmp = 100;
	    } else {
		freq_offset_tmp = 500;
		symrate_offset_tmp = 100;
	    }

	    if (symrateKSs_tpB >= symrateKSs_tpA)
		similar_symbol_rate_KSs = symrateKSs_tpB - symrateKSs_tpA;
	    else
		similar_symbol_rate_KSs = symrateKSs_tpA - symrateKSs_tpB;

	    if (centerfreqKHz_tpB >= centerfreqKHz_tpA)
		delta_freq_KHz = centerfreqKHz_tpB - centerfreqKHz_tpA;
	    else
		delta_freq_KHz = centerfreqKHz_tpA - centerfreqKHz_tpB;

	    if ((similar_symbol_rate_KSs <= symrate_offset_tmp) &&
	        (delta_freq_KHz <= freq_offset_tmp)) {
		mt_fe_print(("\n\tSimilar Skip TP!!!!!!!  Freq. == %d\n", centerfreqKHz_tpB));
		break;
	    }
	}

	last_check_index = handle->global_cfg.iTpIndex;
	handle->global_cfg.iTpIndex++;

	/*find next tpA*/
	for (i = handle->global_cfg.iTpIndex;; i++) {
	    if (handle->global_cfg.bCancelBs)
		break;

	    if (handle->global_cfg.iTpIndex >= handle->global_cfg.iScannedTpCnt) {
		handle->global_cfg.bLastTpAFlag = TRUE;
		handle->global_cfg.iTpIndex = last_check_index;

		break;
	    }

	    if (p_bs_info->p_tp_info[i].dvb_type != MtFeType_DTV_Unknown) {
		handle->global_cfg.iTpIndex = i;
		centerfreqKHz_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].freq_KHz;
		symrateKSs_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].sym_rate_KSs;
		break;
	    }
	}

	if (handle->global_cfg.bLastTpAFlag) {
	    handle->global_cfg.iTpIndex = last_check_index;
	    centerfreqKHz_tpA = p_bs_info->p_tp_info[handle->global_cfg.iTpIndex].freq_KHz;

	    handle->global_cfg.iScannedTpCnt++;
	    ret = _mt_fe_dmd_cs8k_sat_bs_save_scanned_TP(handle, p_bs_info, centerfreqKHz_tpB, symrateKSs_tpB, bs_times);
	    if (ret != MtFeErr_Ok)
		return ret;

	    break;
	}
    }

    handle->global_cfg.iCurCompareTpAKHz = centerfreqKHz_tpA;

    return MtFeErr_Ok;
}

/********************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP
**
**
** Description:	process the scanned new TP in each blind scan bandwidth
**
**
** Inputs:
**
**		Parameter			type					description
**	---------------------------------------------------------------------------------------
**		cur_scan_freq_KHz	U32						start frequency in each blind scan bandwidth
**		bs_times			U8						blind scan times
**
**
** Outputs:
**
**		Parameter			type					description
**	------------------------------------------------------------------------------------
**		p_bs_info			MT_FE_BS_TP_INFO *		tp structure pointer
**
**
********************************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP(MT_FE_CS8000_SAT_Device_Handle handle,
                                                    MT_FE_BS_TP_INFO *p_bs_info,
                                                    U32 cur_scan_freq_KHz,
                                                    U8 bs_times)
{
    MT_FE_BOOL b_analog_locked, b_fftdone;

    U8 tmp, cnt;
    U16 i, totaltpnum;

    for (i = 0; i < 2; i++) {
	b_fftdone = MtFe_False;
	b_analog_locked = MtFe_False;

	cnt = 10;
	do {
	    handle->dmd_get_reg(handle, 0x0d, &tmp);
	    b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
	    handle->mt_sleep(50);
	    cnt--;
	} while ((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

	if (cnt == 0)
	    break;

	handle->dmd_set_reg(handle, 0x9a, 0x80);

	cnt = 50;
	do {
	    handle->dmd_get_reg(handle, 0x9a, &tmp);
	    b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
	    handle->mt_sleep(10);
	    cnt--;
	} while ((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

	if (b_fftdone || (1 == i)) {
	    break;
	} else {
	    handle->dmd_set_reg(handle, 0x5f, 0x00);
	    handle->dmd_set_reg(handle, 0x5e, 0x70);
	}
    }

    handle->dmd_get_reg(handle, 0x9a, &tmp);
    totaltpnum = (U16)(tmp & 0x1F);

    handle->global_cfg.iTpIndex = 0;
    handle->global_cfg.bLastTpAFlag = FALSE;
    while ((totaltpnum > 0) && (!handle->global_cfg.bCancelBs)) {
	totaltpnum--;

	_mt_fe_dmd_cs8k_sat_bs_compare_deal_TP(handle, p_bs_info, cur_scan_freq_KHz, bs_times);
    }

    if (bs_times == MT_FE_BS_TIMES_3RD) {
	//_mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP(p_bs_info, handle->global_cfg.iCurCompareTPAKHz);

	for (i = 0; i < p_bs_info->tp_max_num; i++) {
	    if (handle->global_cfg.iCurCompareTpAKHz == (p_bs_info->p_tp_info[i].freq_KHz)) {
		handle->global_cfg.iTpIndex = i;
		break;
	    }
	}
    }

    return MtFeErr_Ok;
}

/******************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_A
**
**
** Description:	scan the TP from the start_freq_KHz to end_freq_KHz
**
**
** Inputs:
**
**		Parameter			type								description
**	---------------------------------------------------------------------------------------
**		start_freq_KHz		U32									blind scan start freq.
**		end_freq_KHz		U32									blind scan stop freq.
**		bs_times			U8									blind scan times
**
** Outputs:
**
**		Parameter			type								description
**	---------------------------------------------------------------------------------------
**		p_bs_info			MT_FE_BS_TP_INFO* 					blind scan result pointer
**
**
*******************************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_A(MT_FE_CS8000_SAT_Device_Handle handle,
                                   U32 start_freq_KHz,
                                   U32 end_freq_KHz,
                                   MT_FE_BS_TP_INFO *p_bs_info,
                                   U8 bs_times,
                                   U32 is_connect)
{
    U8 val_0x95, val_0x96, end_flag = 0, TP_found_flag;
    U32 freq_KHz, tmp_freq_KHz, tmp_tp_scanned;
    MT_FE_RET ret = MtFeErr_Ok;

    U32 real_freq_KHz;
    S32 iTunerOffsetKHz = 0;

    if ((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
	return MtFeErr_NullPointer;

    tmp_tp_scanned = 0;
    freq_KHz = start_freq_KHz;

    while ((freq_KHz <= end_freq_KHz) && (!handle->global_cfg.bCancelBs)) {
	_mt_fe_dmd_cs8k_sat_bs_set_reg(handle, bs_times);

	handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

	if (handle->lnb_cfg.bUnicable) {
	    mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    freq_KHz = real_freq_KHz;
	} else {
	    handle->tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
	}

	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz);

	tmp_freq_KHz = 0;
	TP_found_flag = 0;

	ret = _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP(handle, p_bs_info, freq_KHz, bs_times);
	if (ret != MtFeErr_Ok)
	    return ret;

	handle->dmd_get_reg(handle, 0x95, &val_0x95);
	handle->dmd_get_reg(handle, 0x96, &val_0x96);

	tmp_freq_KHz = (U32)((((((val_0x95 & 0xc0) >> 6) << 8) | val_0x96) * handle->global_cfg.iMclkKHz) / FFT_LENGTH);
	if (tmp_freq_KHz > 96000) {
	    tmp_freq_KHz = 10000;
	}

	if (handle->lnb_cfg.bUnicable) {
	    if ((tmp_freq_KHz < 4000) && (tmp_freq_KHz != 0)) {
		tmp_freq_KHz = 4000;
	    }
	}

	freq_KHz += tmp_freq_KHz;

	if (tmp_freq_KHz == 0) {
	    freq_KHz += 36000;
	}

	if (tmp_tp_scanned < handle->global_cfg.iScannedTpCnt) {
	    tmp_tp_scanned = handle->global_cfg.iScannedTpCnt;
	    if (is_connect)
		_mt_fe_dmd_cs8k_sat_bs_connect(handle, p_bs_info, 0, handle->global_cfg.iScannedTpCnt);
	    else
		p_bs_info->tp_num = handle->global_cfg.iScannedTpCnt;
	}

	if (end_flag == 1) {
	    end_flag = 0;
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSOneWinFinish, &(freq_KHz));
	    break;
	}
	if (freq_KHz >= 2149000) {
	    freq_KHz = 2149000;
	    end_flag = 1;
	}

	mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSOneWinFinish, &(freq_KHz));
    }
    /* return next_freq_KHz */
    p_bs_info->next_freq_KHz = freq_KHz;

    return MtFeErr_Ok;
}

/******************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_B
**
**
** Description:	scan the TP from the start_freq_KHz to end_freq_KHz
**
**
** Inputs:
**
**		Parameter			type								description
**	---------------------------------------------------------------------------------------
**		start_freq_KHz		U32									blind scan start freq.
**		end_freq_KHz		U32									blind scan stop freq.
**		bs_times			U8									blind scan times
**
**
** Outputs:
**
**		Parameter			type								description
**	---------------------------------------------------------------------------------------
**		p_bs_info			MT_FE_BS_TP_INFO* 					blind scan result pointer
**
**
*******************************************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_B(MT_FE_CS8000_SAT_Device_Handle handle,
                                   U32 start_freq_KHz,
                                   U32 end_freq_KHz,
                                   MT_FE_BS_TP_INFO *p_bs_info,
                                   U8 bs_times,
                                   U32 is_connect)
{
    U8 val_0x95, val_0x96, end_flag = 0, TP_found_flag;
    U32 freq_KHz, tmp_freq_KHz;
    MT_FE_RET ret = MtFeErr_Ok;

    U32 real_freq_KHz;
    S32 iTunerOffsetKHz = 0;

    if ((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
	return MtFeErr_NullPointer;

    freq_KHz = start_freq_KHz;

    _mt_fe_dmd_cs8k_sat_bs_set_reg(handle, bs_times);

    while ((freq_KHz < end_freq_KHz) && (!handle->global_cfg.bCancelBs)) {
	handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

	if (handle->lnb_cfg.bUnicable) {
	    mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    freq_KHz = real_freq_KHz;
	} else {
	    handle->tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
	}

	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz);

	tmp_freq_KHz = 0;
	TP_found_flag = 0;

	ret = _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP(handle, p_bs_info, freq_KHz, bs_times);
	if (ret != MtFeErr_Ok)
	    return ret;

	handle->dmd_get_reg(handle, 0x95, &val_0x95);
	handle->dmd_get_reg(handle, 0x96, &val_0x96);

	tmp_freq_KHz = (U32)((((((val_0x95 & 0xc0) >> 6) << 8) | val_0x96) * handle->global_cfg.iMclkKHz) / FFT_LENGTH);
	if (tmp_freq_KHz > 96000) {
	    tmp_freq_KHz = 10000;
	}

	if (handle->lnb_cfg.bUnicable) {
	    if ((tmp_freq_KHz < 4000) && (tmp_freq_KHz != 0)) {
		tmp_freq_KHz = 4000;
	    }
	}

	freq_KHz = (U32)(freq_KHz + tmp_freq_KHz);

	if (tmp_freq_KHz == 0) {
	    freq_KHz += 36000;
	}

	if (end_flag == 1) {
	    end_flag = 0;
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSOneWinFinish, &(freq_KHz));
	    break;
	}
	if (freq_KHz >= 2149000) {
	    freq_KHz = 2149000;
	    end_flag = 1;
	}

	mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSOneWinFinish, &(freq_KHz));
    }

    if (is_connect)
	_mt_fe_dmd_cs8k_sat_bs_connect(handle, p_bs_info, 0, handle->global_cfg.iScannedTpCnt);
    p_bs_info->next_freq_KHz = freq_KHz;

    return MtFeErr_Ok;
}

/************************************************************************************
** Function: _mt_fe_dmd_cs8k_sat_bs_connect
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
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect(MT_FE_CS8000_SAT_Device_Handle handle,
                                         MT_FE_BS_TP_INFO *p_bs_info,
                                         U16 start_index,
                                         U16 scanned_tp)
{
    U16 index, timing_thr, crl_thr;
    U8 count, timing_fail, crl_fail, tmp, same_locked_TP_flag, val_reg08 = 0;
    U32 sym_rate_KSs, centerfreq_KHz, truefreq_KHz;
    S16 lpf_offset_KHz, i;
    S32 carrieroffset_KHz, delta_freq_KHz, delta_smy_rate_KSs, deviceoffset_KHz;

    MT_FE_RET ret = MtFeErr_Ok;

    U32 real_freq_KHz;
    S32 iTunerOffsetKHz = 0;

    MT_FE_TYPE current_mode = MtFeType_Undef;
    MT_FE_LOCK_STATE lockstate;
    MT_FE_TP_INFO *p_scanned_tp_info = (MT_FE_TP_INFO *)(p_bs_info->p_tp_info);

    S32 iStrength = 0, delta = 0;
    U32 iGain = 0, iGainDiscard = 0;

    U8 ucTmp;
    int iCnt;

    if ((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
	return MtFeErr_NullPointer;

    for (index = start_index; index < scanned_tp; index++) {
	if (handle->global_cfg.bCancelBs)
	    break;

	if (p_scanned_tp_info[index].dvb_type != MtFeType_DTV_Unknown)
	    continue;

	p_scanned_tp_info[index].bCheckCCM = FALSE;
	p_scanned_tp_info[index].iTsCnt = 0;
	p_scanned_tp_info[index].ucCurTsId = 0;

	for (i = 0; i < 16; i++) {
	    p_scanned_tp_info[index].ucTsId[i] = 0;
	}

	centerfreq_KHz = p_scanned_tp_info[index].freq_KHz;
	sym_rate_KSs = p_scanned_tp_info[index].sym_rate_KSs;

	lpf_offset_KHz = 0;
	truefreq_KHz = 0;

	mt_fe_dmd_cs8k_sat_global_reset(handle);
	handle->dmd_set_reg(handle, 0xb2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);

	if ((sym_rate_KSs < 5000) && (!handle->lnb_cfg.bUnicable)) {
	    lpf_offset_KHz = 0;  //BLINDSCAN_LPF_OFFSET_KHz;
	    centerfreq_KHz += 0; //BLINDSCAN_LPF_OFFSET_KHz;
	}

	handle->dmd_get_reg(handle, 0x08, &val_reg08);
	val_reg08 = (U8)(val_reg08 & 0x3b);
	handle->dmd_set_reg(handle, 0x08, val_reg08);
	handle->dmd_set_reg(handle, 0xe0, 0xf8);

	handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = sym_rate_KSs;

	if (handle->lnb_cfg.bUnicable) {
	    ret = mt_fe_cs8k_sat_unicable_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    deviceoffset_KHz = real_freq_KHz - centerfreq_KHz;
	} else {
	    ret = handle->tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
	    real_freq_KHz = centerfreq_KHz;
	    deviceoffset_KHz = 0;
	}

	if (ret != MtFeErr_Ok) // Error parameter
	{
	    lockstate = MtFeLockState_Unlocked;
	    continue;
	}

	_mt_fe_dmd_cs8k_sat_bs_set_demod(handle, sym_rate_KSs);

	if (sym_rate_KSs <= 5000) {
	    handle->dmd_set_reg(handle, 0xc0, 0x04);
	    handle->dmd_set_reg(handle, 0x8a, 0x09);
	    handle->dmd_set_reg(handle, 0x8b, 0x22);
	    handle->dmd_set_reg(handle, 0x8c, 0x88);
	}

	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz + lpf_offset_KHz + deviceoffset_KHz);

	handle->dmd_set_reg(handle, 0xb2, 0x00);

	handle->mt_sleep(70);

	handle->tuner_cfg.tuner_get_strength(handle, &iGain, &iStrength);
	if (sym_rate_KSs >= 3000) {
	    p_bs_info->iCurGain = iGain;
	}

	mt_fe_print(("\tgain = %d, Freq = %d, SymRate = %d\n", iGain, centerfreq_KHz, sym_rate_KSs));

	if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS2022)
	    iGainDiscard = 8800;
	else if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	    iGainDiscard = 9600;
	else
	    iGainDiscard = 8800;

	if ((iGain > iGainDiscard) && (sym_rate_KSs >= 2500)) {
	    lockstate = MtFeLockState_Unlocked; // Treat the TP as unlock

	    p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;

	    mt_fe_print(("\tDiscard!!! Gain > %d, Symbol Rate = %dKSs\n", iGainDiscard, sym_rate_KSs));
	    break;
	}

	if ((!p_bs_info->bNewGain) && (p_bs_info->bs_times == 1) && (sym_rate_KSs >= 3000)) {
	    S32 gain_thr = 0;

	    if (p_bs_info->iCurGain > p_bs_info->iPreGain)
		delta = p_bs_info->iCurGain - p_bs_info->iPreGain;
	    else
		delta = 0;

	    gain_thr = 1400;

	    if (delta > gain_thr) {
		mt_fe_print(("\tDelta gain > %d, current = %d, preview = %d\n", gain_thr, p_bs_info->iCurGain, p_bs_info->iPreGain));

		lockstate = MtFeLockState_Unlocked; // Treat the TP as unlock

		p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;
		break;
	    }
	}

	lockstate = MtFeLockState_Waiting;

	if (lockstate == MtFeLockState_Unlocked) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;

	    continue;
	}

	if (sym_rate_KSs >= 10000) {
	    count = 25;
	    timing_thr = 1;
	    crl_thr = 1;
	} else if (sym_rate_KSs >= 4000) {
	    count = 28;
	    timing_thr = 1;
	    crl_thr = 1;
	} else if (sym_rate_KSs >= 2000) {
	    count = 35;
	    timing_thr = 1;
	    crl_thr = 1;
	} else {
	    count = 40;
	    timing_thr = 1;
	    crl_thr = 1;
	}

	timing_fail = 0;
	crl_fail = 0;
	lockstate = MtFeLockState_Unlocked;

	while ((count > 0) && (!handle->global_cfg.bCancelBs)) {
	    handle->dmd_get_reg(handle, 0x08, &tmp);
	    if ((tmp & 0x08) != 0) {
		handle->tp_cfg.mCurrentType = MtFeType_DvbS2;
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0x8f) == 0x8f) {
		    lockstate = MtFeLockState_Locked;

		    if (!p_scanned_tp_info[index].bCheckCCM) {
			handle->dmd_set_reg(handle, 0xE5, 0x10);
			handle->dmd_set_reg(handle, 0xE5, 0x40);
			handle->mt_sleep(10);
			handle->dmd_get_reg(handle, 0xE5, &ucTmp);
			p_scanned_tp_info[index].iTsCnt = ucTmp & 0x0F;

			if (p_scanned_tp_info[index].iTsCnt > 1) {
			    mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));

				for(iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt ++)	// important multi-stream bug fix, comment by YZ.Huang @ 171226, fix 99487
				{
				ucTmp = (U8)iCnt;

				handle->dmd_set_reg(handle, 0xE6, ucTmp);
				handle->dmd_get_reg(handle, 0xE7, &ucTmp);

				p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;

				mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
			    }

			    handle->dmd_set_reg(handle, 0xE6, 0x0F);
			    handle->dmd_set_reg(handle, 0xE7, p_scanned_tp_info[index].ucTsId[0]); // default select TS 0
			    p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
			    mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
			} else {
			    handle->dmd_set_reg(handle, 0xE5, 0x00);
			}

			p_scanned_tp_info[index].bCheckCCM = TRUE;
		    }
		} else if (((tmp & 0x0C) == 0x0C) && (!p_scanned_tp_info[index].bCheckCCM)) {
		    handle->dmd_set_reg(handle, 0xE5, 0x10);
		    handle->dmd_set_reg(handle, 0xE5, 0x40);
		    handle->mt_sleep(10);
		    handle->dmd_get_reg(handle, 0xE5, &ucTmp);
		    p_scanned_tp_info[index].iTsCnt = ucTmp & 0x0F;

		    if (p_scanned_tp_info[index].iTsCnt > 1) {
			mt_fe_print(("\tCCM find before FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));
            // important multi-stream bug fix, comment by YZ.Huang @ 171226, fix99487
			for (iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt++) {
			    ucTmp = (U8)iCnt;

			    handle->dmd_set_reg(handle, 0xE6, ucTmp);
			    handle->dmd_get_reg(handle, 0xE7, &ucTmp);

			    p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;

			    mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
			}

			handle->dmd_set_reg(handle, 0xE6, 0x0F);
			handle->dmd_set_reg(handle, 0xE7, p_scanned_tp_info[index].ucTsId[0]); // default select TS 0
			p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
			mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
		    } else {
			handle->dmd_set_reg(handle, 0xE5, 0x00);
		    }

		    p_scanned_tp_info[index].bCheckCCM = TRUE;
		}
	    } else {
		handle->tp_cfg.mCurrentType = MtFeType_DvbS;
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7) {
		    lockstate = MtFeLockState_Locked;
		}
	    }

	    current_mode = handle->tp_cfg.mCurrentType;

	    if (lockstate == MtFeLockState_Locked) {
		S32 carrier_offset;
		U8 reg_0x0c;

		if (sym_rate_KSs >= 3000) {
		    p_bs_info->iPreGain = p_bs_info->iCurGain;
		    p_bs_info->bNewGain = FALSE;
		}

		handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
		if (((reg_0x0c & 0x04) == 0x00) && (!handle->lnb_cfg.bUnicable)) {
		    // AFC on
		    _mt_fe_dmd_cs8k_sat_get_total_carrier_offset(handle, &carrier_offset);
		    if ((carrier_offset > ((S32)sym_rate_KSs)) || (carrier_offset < -((S32)sym_rate_KSs))) {
			handle->tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
			real_freq_KHz = centerfreq_KHz;
			deviceoffset_KHz = 0;

			handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;

			handle->dmd_set_reg(handle, 0xb2, 0x01);

			_mt_fe_dmd_cs8k_sat_bs_set_demod(handle, sym_rate_KSs);

			if (sym_rate_KSs <= 5000) {
			    handle->dmd_set_reg(handle, 0xc0, 0x04);
			    handle->dmd_set_reg(handle, 0x8a, 0x09);
			    handle->dmd_set_reg(handle, 0x8b, 0x22);
			    handle->dmd_set_reg(handle, 0x8c, 0x88);
			}

			handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
			reg_0x0c |= 0x04;
			handle->dmd_set_reg(handle, 0x0c, reg_0x0c);

			handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
			_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz + lpf_offset_KHz + deviceoffset_KHz);

			handle->dmd_set_reg(handle, 0xb2, 0x00);

			count = 40;
			lockstate = MtFeLockState_Waiting;
			mt_fe_print(("\tCarrier offset out of range! Frequency = %d, CarrierOffset = %d, SymbolRate = %d\n", centerfreq_KHz, carrier_offset, sym_rate_KSs));
		    } else {
			// normal carrier offset
			break;
		    }
		} else {
		    // AFC off
		    break;
		}
	    } else {
		handle->dmd_get_reg(handle, 0xb3, &tmp);
		if ((tmp & 0x80) == 0x80) {
		    timing_fail++;
		    if (timing_fail >= timing_thr) {
			timing_fail = 1;
			break;
		    }
		    mt_fe_dmd_cs8k_sat_soft_reset(handle);
		    count = 30;
		} else if (((tmp & 0x40) == 0x40) && ((tmp & 0x80) != 0x80)) {
		    crl_fail++;
		    if (crl_fail >= crl_thr) {
			crl_fail = 1;
			break;
		    }
		    mt_fe_dmd_cs8k_sat_soft_reset(handle);
		    count = 50;
		}
	    }

	    handle->mt_sleep(50);
	    count--;
	}

	if (count == 0) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    continue;
	}

	if (timing_fail == 1) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    mt_fe_print(("timing fail!!!\n"));
	    continue;
	} else if (crl_fail == 1) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    continue;
	}

	if (lockstate == MtFeLockState_Locked) {
	    MT_FE_CHAN_INFO_DVBS2 ch_info;

	    _mt_fe_dmd_cs8k_sat_get_carrier_offset(handle, &carrieroffset_KHz);

	    handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	    truefreq_KHz = real_freq_KHz + iTunerOffsetKHz - carrieroffset_KHz;

	    _mt_fe_dmd_cs8k_sat_get_sym_rate(handle, &sym_rate_KSs);

	    same_locked_TP_flag = 0;
	    for (i = (S16)(handle->global_cfg.iScannedTpCnt - 1); i >= 0; i--) {
		if (handle->global_cfg.bCancelBs)
		    break;

		if (p_bs_info->p_tp_info[i].freq_KHz > truefreq_KHz)
		    delta_freq_KHz = p_bs_info->p_tp_info[i].freq_KHz - truefreq_KHz;
		else
		    delta_freq_KHz = truefreq_KHz - p_bs_info->p_tp_info[i].freq_KHz;

		if (p_bs_info->p_tp_info[i].sym_rate_KSs > sym_rate_KSs)
		    delta_smy_rate_KSs = p_bs_info->p_tp_info[i].sym_rate_KSs - sym_rate_KSs;
		else
		    delta_smy_rate_KSs = sym_rate_KSs - p_bs_info->p_tp_info[i].sym_rate_KSs;

		if (((delta_smy_rate_KSs < 50) && (delta_freq_KHz < (p_bs_info->p_tp_info[i].sym_rate_KSs / 2))) &&
		    (p_bs_info->p_tp_info[i].dvb_type == current_mode)) {
		    same_locked_TP_flag = 1;
		    p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;

		    break;
		}
	    }

	    if (same_locked_TP_flag == 1)
		continue;

	    mt_fe_dmd_cs8k_sat_get_channel_info(handle, &ch_info);

	    if ((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk)) {
		handle->dmd_set_reg(handle, 0x9e, 0x9f);
		handle->dmd_set_reg(handle, 0x9f, 0x0c);

		handle->dmd_get_reg(handle, 0x9d, &tmp);
		tmp &= 0xFE;
		handle->dmd_set_reg(handle, 0x9d, tmp);
	    }

	    handle->global_cfg.iLockedTpCnt++;
	    p_bs_info->p_tp_info[index].freq_KHz = truefreq_KHz;
	    p_bs_info->p_tp_info[index].sym_rate_KSs = (U16)sym_rate_KSs;
	    p_bs_info->p_tp_info[index].dvb_type = current_mode;
	    _mt_fe_dmd_cs8k_sat_get_fec(handle, &(p_bs_info->p_tp_info[index]), current_mode);

	    p_bs_info->tp_num = handle->global_cfg.iLockedTpCnt;

	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpLocked, &(p_bs_info->p_tp_info[index]));

	} else {
	    if (current_mode == MtFeType_DvbS2)
		p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;

	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	}
    }

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_cs8k_sat_unicable_set_tuner(MT_FE_CS8000_SAT_Device_Handle handle, U32 freq_KHz, U32 symbol_rate_KSs, U32 *real_freq_KHz, U8 ub_select, U8 bank_select, U32 ub_freq_MHz)
{
#if 1
    U32 tmp_freq_MHz;
    MT_FE_DiSEqC_MSG msg;
    U16 T_chan;

    if (handle->lnb_cfg.iUBVer == 2) // 2.0
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
    } else // 1.0
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

    //mt_fe_dmd_cs8k_sat_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);

    //handle->mt_sleep(200);

    //mt_fe_dmd_cs8k_sat_DiSEqC_send_receive_msg(handle, &msg);
    mt_fe_dmd_cs8k_sat_DiSEqC_send_msg(handle, &msg);
#endif

    //handle->mt_sleep(200);

    //mt_fe_dmd_cs8k_sat_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_13V, MtFe_False);

    //handle->mt_sleep(500);
    handle->tuner_cfg.tuner_set(handle, ub_freq_MHz * 1000, symbol_rate_KSs, 0);

    handle->tp_cfg.iFreqKHz = ub_freq_MHz * 1000;

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_cs8k_sat_get_CCI(MT_FE_CS8000_SAT_Device_Handle handle, S32 *pCCIFreq, S32 *pCCIAmp)
{
    U32 bit07_00 = 0;
    U32 bit15_08 = 0;
    U32 bit15_00 = 0;
    U8 val;
    S32 x = 0;

    handle->dmd_get_reg(handle, 0x5C, &val);
    bit07_00 = val;

    handle->dmd_get_reg(handle, 0x5B, &val);
    bit15_08 = val;

    bit15_00 = (bit15_08 << 8) | bit07_00;
    x = (S32)bit15_00;
    if (x >= 32768) {
	x -= 65536;
    }
    *pCCIFreq = x;

    handle->dmd_get_reg(handle, 0x5A, &val);
    bit07_00 = val;

    handle->dmd_get_reg(handle, 0x59, &val);
    bit15_08 = val;

    bit15_00 = bit15_08 << 8 | bit07_00;
    x = (S32)bit15_00;
    if (x >= 32768) {
	x -= 65536;
    }
    *pCCIAmp = x / 128;

    return MtFeErr_Ok;
}

MT_FE_BOOL mt_fe_cs8k_sat_unicable_scan_UB(MT_FE_CS8000_SAT_Device_Handle handle, U32 startFreq, U32 *result_freq, S32 *pCCIAmp, U32 *pCCIGain) // Unit: KHz
{
#if 0
	U8 i, cnt, reg_0xb3, reg_0x59, CCI_on_flag_cnt;
	S8 val_cci;

	U8 val;

	U8 Readval = 0, DeciMode = 0;
	S32 x = 0;
	U32 bit15_08 = 0, bit15_00 = 0;
	DB fCCIFreq = 0.0, fTotalFO = 0.0;
	S32 CCIFreq = 0, CCIAmp = 0;

	S32 MaxFreq, MinFreq;
	S32 MaxAmp, MinAmp;

	S32 iStrength = 0;
	S32 iTunerOffsetKHz = 0;


	if(handle->tuner_cfg.tuner_get_offset == NULL)
	{
		return MtFe_False;
	}

	*result_freq = 0;
	*pCCIAmp = 0;
	*pCCIGain = 0;

	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;
	handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2));		// XOR

	handle->tuner_cfg.tuner_set(handle, startFreq, 40000, 0);

	handle->tp_cfg.iFreqKHz = (startFreq + 500) / 1000 * 1000;

	handle->dmd_set_reg(handle, 0xb2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);


	_mt_fe_dmd_cs8k_sat_init_reg(handle, cci_detect_reg_tbl_def_cs8000_sat, sizeof(cci_detect_reg_tbl_def_cs8000_sat)/2);


	if(handle->board_cfg.bSpectrumInverted)			// 0x4d, bit 1
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

	if(handle->board_cfg.bAGCPolar)			// 0x30, bit 4
	{
		handle->dmd_get_reg(handle, 0x30, &val);
		val |= 0x10;
		handle->dmd_set_reg(handle, 0x30, val);
	}
	else
	{
		handle->dmd_get_reg(handle, 0x30, &val);
		val &= ~0x10;
		handle->dmd_set_reg(handle, 0x30, val);
	}


	_mt_fe_dmd_cs8k_sat_set_sym_rate(handle, 40000);

	handle->dmd_set_reg(handle, 0x00, 0x00);
	handle->dmd_set_reg(handle, 0xb2, 0x00);


	handle->mt_sleep(100);

	cnt = 0;

	handle->dmd_get_reg(handle, 0xb3, &reg_0xb3);

	while((reg_0xb3 == 0x38) || (reg_0xb3 <= 3))
	{
		cnt ++;

		if(cnt > 10)
		{
			mt_fe_print(("\tFailed to get CCI 1!\n"));
			return MtFe_False;
		}

		handle->mt_sleep(10);

		handle->dmd_get_reg(handle, 0xb3, &reg_0xb3);
	}

	CCI_on_flag_cnt = 0;
	for(i = 0; i < 15; i++)
	{
		handle->dmd_get_reg(handle, 0x59, &reg_0x59);
		val_cci = (S8)reg_0x59;

		handle->dmd_get_reg(handle, 0xb3, &reg_0xb3);

		if(val_cci < 0)
		{
			CCI_on_flag_cnt = 0;
			break;
		}

		if(val_cci > 2)		// 2				//MT_FE_CCI_THRESHOLD
			CCI_on_flag_cnt++;
	}


	if (CCI_on_flag_cnt < 10)
	{
		mt_fe_print(("\tFailed to get CCI 2!\n"));
		return MtFe_False;
	}

	handle->mt_sleep(1);


	handle->dmd_get_reg(handle, 0x5D, &Readval);
	Readval = (U8)(Readval & 0xF8);
	handle->dmd_set_reg(handle, 0x5D, Readval);

	handle->dmd_get_reg(handle, 0x5F, &Readval);
	bit15_08 = Readval << 8;

	handle->dmd_get_reg(handle, 0x5E, &Readval);

	bit15_00 = bit15_08 | Readval;
	x = (int)bit15_00;
	if(x >= 32768)
	{
		x -= 65536;
	}

	fTotalFO = x / 65536.0 * handle->global_cfg.iMclkKHz;


	handle->dmd_get_reg(handle, 0xC9, &Readval);
	DeciMode = (U8)((Readval& 0x70) >> 4);


	MaxAmp = 0x80000000;
	MinAmp = 0x7FFFFFFF;
	MaxFreq = 0x80000000;
	MinFreq = 0x7FFFFFFF;

	for(i = 0; i < 5; i ++)
	{
		_mt_fe_dmd_cs8k_sat_get_CCI(handle, &CCIFreq, &CCIAmp);

		mt_fe_print(("\tFreq = %d, Amp = %d\n", CCIFreq, CCIAmp));

		if(MaxFreq < CCIFreq)	MaxFreq = CCIFreq;
		if(MinFreq > CCIFreq)	MinFreq = CCIFreq;
		if(MaxAmp < CCIAmp)		MaxAmp = CCIAmp;
		if(MinAmp > CCIAmp)		MinAmp = CCIAmp;

		if((MaxAmp - MinAmp) >= 0x10)
		{
			mt_fe_print(("\tMaxAmp = %d, MinAmp = %d\n", MaxAmp, MinAmp));
			return MtFe_False;
		}

		if((MaxFreq - MinFreq) >= 0x100)
		{
			mt_fe_print(("\tMaxFreq = %d, MinFreq = %d\n", MaxFreq, MinFreq));
			return MtFe_False;
		}

		handle->mt_sleep(10);
	}

	*pCCIAmp = CCIAmp;

	fCCIFreq = (CCIFreq / 65536.0 * handle->global_cfg.iMclkKHz) / (1 << DeciMode) - fTotalFO;
	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	mt_fe_print(("\tCCI Offset = [%d], TN_Offset = [%d]\n", (S32)fCCIFreq, iTunerOffsetKHz));


	fCCIFreq -= iTunerOffsetKHz;

	*result_freq = startFreq - (S32)fCCIFreq;

	mt_fe_print(("\tAmp=[%d],Freq=[%d],CciFo=[%d],TnFo=[%d],Result=[%d]\n", *pCCIAmp, startFreq, (S32)fCCIFreq, iTunerOffsetKHz, *result_freq));

	*result_freq += 500;
	*result_freq /= 1000;

	if(handle->tuner_cfg.tuner_get_strength != NULL)
		handle->tuner_cfg.tuner_get_strength(handle, pCCIGain, &iStrength);

	if(*pCCIGain >= 0x80000000)
	{
		*pCCIGain = 0;
	}

	if(*pCCIGain < 1151)
	{
		*pCCIGain = 1151;
	}
#endif
    return MtFe_True;
}

MT_FE_BOOL _mt_fe_dmd_cs8k_sat_unicable_check_result_duplicate(MT_FE_UNICABLE_DEVICE *p_ub_list, U16 count, U32 target_freq_MHz)
{
    U16 i;

    for (i = 0; i < count; i++) {
	if (target_freq_MHz < (p_ub_list[i].freq_MHz - 1)) {
	    continue;
	}

	if (target_freq_MHz > (p_ub_list[i].freq_MHz + 1)) {
	    continue;
	}

	return MtFe_True;
    }

    return MtFe_False;
}

void mt_fe_dmd_cs8k_sat_unicable_blind_detect(MT_FE_CS8000_SAT_Device_Handle handle, U16 start_freq_MHz, U16 stop_freq_MHz, MT_FE_UNICABLE_DEVICE *p_ub_list, MT_BOOL need_init)
{
    U16 cur_freq_MHz = 0;

    U32 CCI_result_MHz = 0;
    S32 CCI_Amp = 0;
    U32 CCI_Gain = 0;

    U16 i;
    U8 ub_count = 0;

    U32 min_gain = 0xFFFFFFFF, max_gain = 0;

    U32 total_gain = 0;

    handle->lnb_cfg.iUBCount = 0;

    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
    //handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;
    handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2)); // XOR

    if (need_init != FALSE) {
	MT_FE_DiSEqC_MSG msg;

	msg.data_send[0] = 0xE0;
	msg.data_send[1] = 0x10;
	msg.data_send[2] = 0x5B;
	msg.data_send[3] = 0x00;
	msg.data_send[4] = 0x00;

	msg.size_send = 5;
	msg.is_enable_receive = FALSE;
	msg.is_envelop_mode = FALSE;

	mt_fe_dmd_cs8k_sat_set_LNB(handle, MtFe_True, MtFe_False, MtFeLNB_18V, MtFe_False);
	//mt_fe_dmd_cs8k_sat_DiSEqC_send_receive_msg(handle, &msg);
	mt_fe_dmd_cs8k_sat_DiSEqC_send_msg(handle, &msg);
	handle->mt_sleep(10);
	mt_fe_print(("\tDiSEqC messages have been sent out!\n"));
    }

    for (cur_freq_MHz = start_freq_MHz + 15; cur_freq_MHz < stop_freq_MHz; cur_freq_MHz += 30) {
	mt_fe_print(("\n\t--------[%4d]MHz--------\n", cur_freq_MHz));

	if (mt_fe_cs8k_sat_unicable_scan_UB(handle, cur_freq_MHz * 1000, &CCI_result_MHz, &CCI_Amp, &CCI_Gain) == MtFe_True) {
	    if (_mt_fe_dmd_cs8k_sat_unicable_check_result_duplicate(p_ub_list, handle->lnb_cfg.iUBCount, CCI_result_MHz) == MtFe_False) {
		U32 Verify_MHz = 0;
		S32 Verify_Amp = 0;
		U32 Verify_Gain = 0;

		mt_fe_print(("\t[CCI -- %4d]MHz----[AMP -- %4d]----[Gain -- %d]\n", CCI_result_MHz, CCI_Amp, CCI_Gain));

		if (mt_fe_cs8k_sat_unicable_scan_UB(handle, (CCI_result_MHz + 5) * 1000, &Verify_MHz, &Verify_Amp, &Verify_Gain) == MtFe_True) {
		    mt_fe_print(("\t[VFY -- %4d]MHz----[AMP -- %4d]----[Gain -- %d]\n", Verify_MHz, Verify_Amp, Verify_Gain));

		    if (Verify_MHz == CCI_result_MHz) {
			p_ub_list[handle->lnb_cfg.iUBCount].freq_MHz = CCI_result_MHz;
			p_ub_list[handle->lnb_cfg.iUBCount].result_MHz = CCI_result_MHz;
			p_ub_list[handle->lnb_cfg.iUBCount].result_gain = CCI_Gain;
			p_ub_list[handle->lnb_cfg.iUBCount].UB_ok = TRUE;

			handle->lnb_cfg.iUBCount++;

			if (CCI_Gain > max_gain)
			    max_gain = CCI_Gain;
			if (CCI_Gain < min_gain)
			    min_gain = CCI_Gain;
		    }
		}
	    }
	}
    }

    mt_fe_print(("\n\tTotal %d %s found!\n\n", handle->lnb_cfg.iUBCount, (handle->lnb_cfg.iUBCount == 1) ? "UB is" : "UBs are"));

    if (min_gain > max_gain) {
	min_gain = max_gain;
    }

    ub_count = handle->lnb_cfg.iUBCount;
    if (handle->lnb_cfg.iUBCount > 0) {
	U32 average_gain = 0;
	U32 gain_ratio = 0;
	U32 valid_ub_num = 0;

	total_gain = 0;

	for (i = 0; i < handle->lnb_cfg.iUBCount; i++) {
	    if (p_ub_list[i].UB_ok == TRUE) {
		gain_ratio = p_ub_list[i].result_gain * 10 / min_gain;

		if (gain_ratio < 15) {
		    total_gain += p_ub_list[i].result_gain;
		    valid_ub_num++;
		}
	    }
	}

	if (valid_ub_num > 0) {
	    average_gain = total_gain / valid_ub_num;

	    for (i = 0; i < handle->lnb_cfg.iUBCount; i++) {
		if (p_ub_list[i].UB_ok == TRUE) {
		    gain_ratio = p_ub_list[i].result_gain * 100 / average_gain;

		    mt_fe_print(("\t[%02d]--Freq = [%4d] Gain = [%d], Avg = [%d], Ratio = [%d]--%s!\n", i, p_ub_list[i].result_MHz, p_ub_list[i].result_gain, average_gain, gain_ratio, (gain_ratio > 150) ? "FAIL" : "OK"));

		    if (gain_ratio > 150) {
			p_ub_list[i].UB_ok = FALSE;

			ub_count--;

			continue;
		    }
		}
	    }
	}
    }

    handle->lnb_cfg.iUBCount = ub_count;

    return;
}

/***************          Blind scan test        ******************************/

MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_set_reg_psd(MT_FE_CS8000_SAT_Device_Handle handle)
{
    U8 tmp, sm_buf;

    U8 TMP1, TMP2, TMP3, TMP4;

    TMP1 = (handle->bs_cfg.ave_times / 32 - 1) * 16 + (handle->bs_cfg.data_length / 128 - 1); // = ((256/32 - 1)*16 + (512/128 - 1)) = ((7 * 16 + 3)) = 0x73
    TMP2 = 64 + handle->bs_cfg.psd_overlap / 16;                                              // = (64 + 96/16) = (64 + 6) = 0x46
    TMP3 = handle->bs_cfg.psd_scale * 32;                                                     // = (0*32) = 0
    TMP4 = handle->bs_cfg.notch_range_f * 16 + handle->bs_cfg.start_range_f;                  // = (8*16 + 15) = 0x8F

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

    if (sm_buf == handle->bs_cfg.sm_buf_1st)
	tmp = 128 + TMP2;
    else
	tmp = 0x42; //TMP2;
    handle->dmd_set_reg(handle, 0x91, tmp);

    tmp = (U8)(TMP3 + sm_buf - 1);
    tmp &= 0x7f;
    handle->dmd_set_reg(handle, 0x92, tmp);

    handle->dmd_get_reg(handle, 0x93, &tmp);
    tmp &= 0x0F;
    tmp |= TMP4;
    handle->dmd_set_reg(handle, 0x93, tmp);

    handle->dmd_get_reg(handle, 0x94, &tmp);
    tmp |= (handle->bs_cfg.find_notch_step * 16 + handle->bs_cfg.thr_factor_2nd); // THR_FACTOR_ONE_LOOP
    handle->dmd_set_reg(handle, 0x94, tmp);

    tmp = 64 + handle->bs_cfg.err_bound_factor_2nd;
    handle->dmd_set_reg(handle, 0x95, tmp);

    tmp = handle->bs_cfg.tuner_slip_step * 16 + handle->bs_cfg.snr_thr_2nd / 16 - 1;
    handle->dmd_set_reg(handle, 0x97, tmp);

    tmp = handle->bs_cfg.flat_thr_2nd;
    handle->dmd_set_reg(handle, 0x99, tmp);

    handle->dmd_set_reg(handle, 0x30, (U8)(handle->board_cfg.bAGCPolar ? 0x18 : 0x08));

    handle->dmd_set_reg(handle, 0x32, 0x44);

    if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS2022)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else if (handle->tuner_cfg.tuner_type == TN_MONTAGE_TS6011)
	handle->dmd_set_reg(handle, 0x33, 0x99);
    else
	handle->dmd_set_reg(handle, 0x33, 0x35);

    handle->dmd_set_reg(handle, 0x35, 0x7F);
    handle->dmd_set_reg(handle, 0x4B, 0x04);
    handle->dmd_set_reg(handle, 0x56, 0x01);
    handle->dmd_set_reg(handle, 0xA0, 0x44);
    handle->dmd_set_reg(handle, 0x08, 0x83);
    handle->dmd_set_reg(handle, 0x70, 0x00);

    if (handle->board_cfg.bSpectrumInverted) // 0x4d, bit 1
    {
	handle->dmd_get_reg(handle, 0x4d, &tmp);
	tmp |= 0x02;
	handle->dmd_set_reg(handle, 0x4d, tmp);
    } else {
	handle->dmd_get_reg(handle, 0x4d, &tmp);
	tmp &= ~0x02;
	handle->dmd_set_reg(handle, 0x4d, tmp);
    }

    handle->dmd_set_reg(handle, 0x00, 0x00);
    handle->dmd_set_reg(handle, 0xB2, 0x00);

    return MtFeErr_Ok;
}

#if 0
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP_psd(MT_FE_CS8000_SAT_Device_Handle handle, U32 cur_scan_freq_KHz)
{
	MT_FE_BOOL	b_analog_locked, b_fftdone;
	U8  tmp, cnt;
	U16	totaltpnum;
	FILE	*fpOut;
	U32 gain_val = 0;
	S32 iStrength = 0;

	//U32		nSize = (512 - 16 * 2 + 2 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	U32		nSize = (512 - 16 * 2) * 2;		// sm_buf = 2, overlap = 16, nSize = 968
	//U32		nSize = (512 - 16 -1) * 2;
	U8		buffer[1024] = {0};						// sm = 2;
	U32		dwPSDArray[512];						// 484 / 2
	U32		nCount, i, j;
	U32		dwTmp;

	char	file_name[100];

	sprintf(file_name, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H":"L", (handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);

	handle->dmd_set_reg(handle, 0x90, 0x70);

	for(i = 0; i < 2; i++)
	{
		b_fftdone		= MtFe_False;
		b_analog_locked = MtFe_False;

		cnt = 10;
		do
		{
			handle->dmd_get_reg(handle, 0x0d, &tmp);
			b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
			handle->mt_sleep(50);
			cnt--;
		}while((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

		if(cnt == 0)
			break;

		handle->dmd_set_reg(handle, 0x9a, 0x80);

		cnt = 50;
		do
		{
			handle->dmd_get_reg(handle, 0x9a, &tmp);
			b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
			handle->mt_sleep(10);
			cnt--;
		}while((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

		if(b_fftdone || (1 == i))
		{
			break;
		}
		else
		{
			handle->dmd_set_reg(handle, 0x5f, 0x00);
			handle->dmd_set_reg(handle, 0x5e, 0x70);
		}
	}

	mt_fe_print(("\tAnalog_Locked = %d, FFT_Done = %d\n", b_analog_locked, b_fftdone));

	fpOut = fopen(file_name, "w+");

	handle->tn_get_reg(handle, 0x5A, &tmp);
	fprintf(fpOut, "RF Gain: %d\n", tmp & 0x0f);

	handle->tn_get_reg(handle, 0x5F, &tmp);
	fprintf(fpOut, "IF Gain: %d\n", tmp & 0x0f);

	handle->tn_get_reg(handle, 0x77, &tmp);
	fprintf(fpOut, "BB Gain: %d\n", (tmp >> 4) & 0x0f);

	handle->dmd_get_reg(handle, 0x3F, &tmp);
	fprintf(fpOut, "PWM: %d\n", tmp);

	//	handle->tuner_cfg.(handle, &gain_val);
	gain_val = 0;


	handle->tuner_cfg.tuner_get_strength(handle,&gain_val, &iStrength);
	fprintf(fpOut, "Tuner Gain: %d\n", gain_val);

	fprintf(fpOut, "Strength: %d\n", iStrength);

	fclose(fpOut);

	handle->dmd_get_reg(handle, 0x9a, &tmp);
	totaltpnum = (U16)(tmp & 0x1F);

	nCount = 1;

	handle->dmd_set_reg(handle, 0x9A, 0x40);

	for(j=0; j<nSize; j++)
	{
		handle->dmd_get_reg(handle, 0x9B, &tmp);
		buffer[j] = tmp;
	}

	//for(i=2*4; i<nSize; i+=2)					// sm_buf = 4
	for(i=2*2; i<nSize; i+=2)					// sm_buf = 4
	{
		dwTmp = buffer[i];
		dwTmp = dwTmp + (buffer[i+1] * 256);
		dwPSDArray[nCount++] = dwTmp;
	}


	fpOut = fopen(file_name, "a");

	for(i=1; i<nCount; i++)
	{
		fprintf(fpOut, "%d\n", dwPSDArray[i]);
	}
	fclose(fpOut);


	handle->mt_sleep(50);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_get_psd(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz)
{
	U32 begin_freq_KHz, end_freq_KHz;
	U32 real_freq_KHz;
	U32 freq_KHz;
	S32 iTunerOffsetKHz = 0;

	mt_fe_dmd_cs8k_sat_global_reset(handle);

	mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSStart, 0);

	handle->global_cfg.iScannedTpCnt	 = 0;
	handle->global_cfg.iLockedTpCnt		 = 0;
	handle->global_cfg.bCancelBs		 = FALSE;
	handle->global_cfg.iTotalBsTimes	 = 1;//p_bs_info->bs_times;

	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ handle->lnb_cfg.bUnicable;		// XOR
	//handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted;
	handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2));		// XOR

	begin_freq_KHz = begin_freq_MHz*1000;
	end_freq_KHz   = end_freq_MHz*1000;

	if((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
		return MtFeErr_NullPointer;

	freq_KHz  = begin_freq_KHz;

	while((freq_KHz < end_freq_KHz)&&(!handle->global_cfg.bCancelBs))
	{
		_mt_fe_dmd_cs8k_sat_bs_set_reg_psd(handle);


		handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
		handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

		if(handle->lnb_cfg.bUnicable)
		{
			mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
			freq_KHz = real_freq_KHz;
		}
		else
		{
			handle->tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
		}

		handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
		_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz);


		mt_fe_print(("\t---------get psd----- %dMhz-----\n", freq_KHz/1000));

		_mt_fe_dmd_cs8k_sat_bs_process_scanned_TP_psd(handle, freq_KHz);

		freq_KHz += 2000;
	}
	handle->mt_sleep(1000);

	mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSFinish, 0);

	return MtFeErr_Ok;
}
#endif

//////////////////////////////////////////////
#define FIND_DEBUG 0
#define PSD_WRITE 0

#define mt_fe_min(a, b) (((a) < (b)) ? (a) : (b))
#define FFT_N 512
#define SM_WIN 4 //6//8	//% The smoothen window length

#define OVLP 84 //32//;//64;//96;//%256;
#define SR_X_thr 15000

U8 Fs = 96;
U32 ct_freq = 0; //%randint(1,1,[0 25])*2;

S8 tuner_offset = 0;
U32 SymRate_last = 0, CtFreq_last = 0;
S32 idx_dw_last = 0;
U32 idx_notch_last_tureCh = 1;
U32 idx_ch_max = 1;
U32 idx_ch_min = 1;
U32 idx_notch = 1;
U32 ch_max = 0;
U32 ch_min = 0;
U32 idx_notch_last = 1;
U32 idx_psd_start = 0;
U32 ave_peak = 0;
//U8  wide_TP_flag = 0;
U32 psd_offset = 11;
S32 SNR_last = -10;
U32 ch_noise_lev = 0;
//U32 ct_freq_actual = 0;

void strmcp(char *des, char *sour, U8 m)
{
    U8 n = 0;

    char *descpy;
    descpy = des;

    for (n = 0; n < m; n++)
	sour++;

    while ((*sour) != '\0') {
	*descpy = *sour;
	descpy++;
	sour++;
    }
}

#if 0
void mt_fe_dmd_bs_global_init()
{
	Fs  = 96;
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

void mt_fe_dmd_bs_find_Min(U32 *sour_value, U32 *min_value, U32 *min_index, U32 start_index, U32 end_index)
{
    U32 data = 0;
    U32 i = 0, index = 0;

    data = sour_value[start_index];
    index = start_index;
    for (i = start_index; i <= end_index; i++) {
	if (sour_value[i] < data) {
	    data = sour_value[i];
	    index = i;
	}
    }

    *min_value = data;
    *min_index = (index - start_index + 1);
}

void mt_fe_dmd_bs_find_Max(U32 *sour_value, U32 *max_value, U32 *max_index, U32 start_index, U32 end_index)
{
    U32 data = 0;
    U32 i = 0, index = 0;

    data = sour_value[start_index];
    index = start_index;

    for (i = start_index; i <= end_index; i++) {
	if (sour_value[i] > data) {
	    data = sour_value[i];
	    index = i;
	}
    }

    *max_value = data;

    *max_index = (index - start_index + 1);
}

void mt_fe_dmd_bs_find_Mean(U32 *sour_value, U32 *mean_value, U32 find_value, U32 start_index, U32 end_index)
{
    U32 add_value = 0;
    U32 i = 0, index = 0;

    for (i = start_index; i <= end_index; i++) {
	if (sour_value[i] >= find_value) {
	    add_value = add_value + sour_value[i];
	    index++;
	}
    }

    if (index != 0)
	*mean_value = add_value / index;
    else {
	mt_fe_print(("\tmt_fe_dmd_bs_find_Mean() ---- index = 0!\n"));
    }
}

void mt_fe_dmd_bs_ch_measure(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info, U32 *psd, U32 idx_ps_sn)
{
#define MAX_sub_TP_N 6 ////4

    U32 SR_max = 58000;
    MT_FE_TP_INFO tp_info;
    //U32 LO_freq = 0;
    U32 K = 125;             //double K=-0.0625*2;//0.0625*0;//%%%%Ufix4_En5
    U8 MERGE_THR = 125;      //1.25;//% larger the value, more likely to merge the adjacent TP
    U8 BW2SR_ratio = 135;    //1.35;
                             //	double Thr_sig = (double)3/16;
    S32 SNR_filt_thr = 3512; //-4.0;
    S32 cap_value0 = 0, cap_value1 = 0;
    U32 sub_TP_spilt_mode = 0;
    U32 idx_sub_TP = 0;
    U32 idx_notch_split[20] = { 0, 0, 0, 0 }; //% this means there max intermediate notches number allowed is 4 (5 split TPs)
    U32 ii = 0, i = 0;
    U32 idx = 0;
    //U32 min_tmp = 0;
    U32 sub_TP_N = 0;
    U32 notch_all[20] = { 0, 0, 0, 0, 0, 0 };

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
    U8 K_high = 0; // -0.1;
    U8 K_low = 0;  //0.1;
    U32 idx_tmp = 0, idx_L_high = 0, idx_L_low = 0, idx_R_high = 0, idx_R_low = 0;
    U32 ch_thr_high, ch_thr_low;
    S32 SNR;
    //U32 win_tmp = 0;
    //double edge_max = 0, edge_min = 0;
    //U32	edge_max_index = 0, edge_min_index = 0;
    //U8  merge_flag = 0;
    U32 idx_up_merge = 0;
    U32 idx_dw_merge = 0;
    U32 SymRate_merge = 0;
    U32 CtFreq_merge = 0;
    U32 all_under_thr_idx[256];
    U32 all_thr_idx = 0;
    U32 L1 = 0;
    U32 val = 0;
    U32 pos = 0, tmp = 0;
    U8 rep_flag = 0;
    U32 idx_notch1 = 0, idx_notch_last1 = 0;
    U8 count = 0;

    //[ch_max idx_ch_max]= max(psd(idx_notch_last:idx_notch));% find the peak within [idx_notch last:idx_notch] for symbol rate calculation
    //idx_ch_max = idx_ch_max + idx_notch_last -1;% to align the idx with psd start

    mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
    idx_ch_max = idx_ch_max + idx_notch_last - 1;

    while (1) {
	count++;
	if (count > 15) {
	    mt_fe_print(("all in  while1 !\n"));

	    break;
	}

	sig_level = 0;
	sig_idx = 0;

	if (rep_flag == 0) {
	    for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++) {
		sig_level = sig_level + psd[idx_tmp];
		sig_idx = sig_idx + 1;
	    }

	    if (sig_idx != 0)
		sig_thr = sig_level / sig_idx;
	    else {
		sig_thr = 1;
	    }

	    sig_level = 0;
	    sig_idx = 0;

	    for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++) {
		if (psd[idx_tmp] >= sig_thr) {
		    sig_level = sig_level + psd[idx_tmp];
		    sig_idx = sig_idx + 1;
		}
	    }

	    if (sig_idx != 0)
		sig_level = (2 * sig_level / sig_idx + 1) / 2;
	    else {
		sig_level = 1;

		//sig_level =psd[idx_notch_last-1];
		mt_fe_print(("sig_level error !\n"));
	    }
	} else {
	    for (idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp++) {
		sig_level = sig_level + psd[idx_tmp];
		sig_idx = sig_idx + 1;
	    }

	    if (sig_idx != 0)
		sig_thr = sig_level / sig_idx;
	    else {
		sig_thr = 1;
	    }

	    sig_level = 0;
	    sig_idx = 0;

	    for (idx_tmp = idx_notch_last1; idx_tmp <= idx_notch1; idx_tmp++) {
		if (psd[idx_tmp] >= sig_thr) {
		    sig_level = sig_level + psd[idx_tmp];
		    sig_idx = sig_idx + 1;
		}
	    }

	    if (sig_idx != 0)
		sig_level = (2 * sig_level / sig_idx + 1) / 2;
	    else {
		sig_level = 1;
		//sig_level =psd[idx_notch_last-1];
		mt_fe_print(("sig_level error !\n"));
	    }
	}

	if (psd[idx_notch_last] <= psd[idx_notch])
	    noise_level = psd[idx_notch_last];
	else
	    noise_level = psd[idx_notch];

	ch_thr_L = (sig_level * (500 + K) + noise_level * (500 - K) + 500) / 1000;
	ch_thr_R = (sig_level * (500 + K) + noise_level * (500 - K) + 500) / 1000;

	//% limit the right side thr to right side min level
	if (ch_thr_L < psd[idx_notch_last])
	    ch_thr_L = psd[idx_notch_last];

	if (ch_thr_R < psd[idx_notch])
	    ch_thr_R = psd[idx_notch] + 1;

	//%locate the falling edge thr crossing point
	for (idx_tmp = idx_ch_max; idx_tmp <= (idx_notch - 1); idx_tmp++) {
	    if ((psd[idx_tmp + 1] <= ch_thr_R) && (psd[idx_tmp] > ch_thr_R)) {
		idx_dw = idx_tmp;
		if (sub_TP_spilt_mode)
		    break;
	    }
	}

	//% locate the rising edge thr crossing  point
	for (idx_tmp = idx_ch_max; idx_tmp >= idx_notch_last; idx_tmp--) {
	    if ((psd[idx_tmp + 1] > ch_thr_L) && (psd[idx_tmp] <= ch_thr_L)) {
		idx_up = idx_tmp;
		if (sub_TP_spilt_mode)
		    break;
	    }
	}

	//%-----------------refine the symbol rate-----------------------------------
	if (idx_dw != 1) {
	    //idx_dw = idx_dw + mt_fe_min(1,(psd[idx_dw]-ch_thr_R)/(psd[idx_dw] - psd[idx_dw+1]));
	    if ((psd[idx_dw] - ch_thr_R) >= ((psd[idx_dw] - psd[idx_dw + 1]))) {
		idx_dw = idx_dw + 1;
		idx_dwd = idx_dw * 100;
	    } else {
		if (psd[idx_dw] != psd[idx_dw + 1])
		    idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / (psd[idx_dw] - psd[idx_dw + 1]) + idx_dw * 200 + 1) / 2;
		else
		    idx_dwd = (200 * (psd[idx_dw] - ch_thr_R) / 1 + idx_dw * 200 + 1) / 2;
	    }
	}

	if (idx_up != 1) {
	    //idx_up = idx_up - mt_fe_min(1,(ch_thr_L-psd[idx_up])/(psd[idx_up] - psd[idx_up+1]));
	    if ((ch_thr_L - psd[idx_up]) >= ((psd[idx_up + 1] - psd[idx_up]))) {
		//idx_up = idx_up;        // -1;
		idx_upd = idx_up * 100; // -1;
	    } else {
		if (psd[idx_up + 1] != psd[idx_up])
		    idx_upd = (200 * (ch_thr_L - psd[idx_up]) / (psd[idx_up + 1] - psd[idx_up]) + idx_up * 200 + 1) / 2;
		else
		    idx_upd = (200 * (ch_thr_L - psd[idx_up]) / 1 + idx_up * 200 + 1) / 2;
	    }
	}

	tmp = 0;
	if ((idx_dw > idx_up) && (idx_notch > idx_notch_last)) {
	    if ((2 * (idx_dw - idx_up)) < (idx_notch - idx_notch_last))
		tmp = 1;
	} else if ((idx_dw < idx_up) && (idx_notch < idx_notch_last)) {
	    if ((2 * (idx_up - idx_dw)) < (idx_notch_last - idx_notch))
		tmp = 1;
	}

	if ((tmp != 0) && (rep_flag == 0) && (sub_TP_spilt_mode == 0))
	    // if (idx_dw - idx_up)/(idx_notch - idx_notch_last) < 1/2 && rep_flag == 0
	{
	    if ((5 * idx_up) > idx_dw) {
		tmp = (5 * idx_up - idx_dw + 2) / 4;
		if (idx_notch_last < tmp) {
		    mt_fe_dmd_bs_find_Min(psd, &val, &pos, tmp, idx_up);
		    //[val,pos] = min(psd(tmp:idx_up));
		    idx_notch_last1 = pos + tmp - 1;
		} else
		    idx_notch_last1 = idx_notch_last;
	    } else
		idx_notch_last1 = idx_notch_last;

	    if ((5 * idx_dw) > idx_up) {
		tmp = (5 * idx_dw - idx_up + 2) / 4;
		if (idx_notch > tmp) {
		    mt_fe_dmd_bs_find_Min(psd, &val, &pos, idx_dw, tmp);
		    //[val,pos] = min(psd(round(idx_dw):tmp));
		    idx_notch1 = pos + idx_dw - 1;
		} else
		    idx_notch1 = idx_notch;
	    } else
		idx_notch1 = idx_notch;

	    rep_flag = 1;

	    continue;
	}

	//%%%%% locate the up cross threshold point
	if ((idx_up != 1) || (idx_dw != 1)) {
	    //% if there is  down or up cross point, compute the symbol rate and center freq
	    if (idx_dwd > idx_upd)
		SymRate_raw = (idx_dwd - idx_upd) * Fs * 10 / FFT_N;
	    else
		SymRate_raw = (idx_upd - idx_dwd) * Fs * 10 / FFT_N;

	    if (SymRate_raw > 60000)
		mt_fe_print(("Error symbol rate %d\n", SymRate_raw));

	    //CtFreq=(idx_dw/2+idx_up/2-(FFT_N-2*OVLP)/2+tuner_offset)*Fs/FFT_N+ct_freq;
	    if ((idx_dwd + idx_upd + 200 * OVLP + 200 * tuner_offset) >= (100 * FFT_N))
		CtFreq_raw = ((10 * idx_dwd + 10 * idx_upd + 2000 * OVLP + 2000 * tuner_offset - 1000 * FFT_N) * Fs / (2 * FFT_N) + ct_freq);
	    else
		CtFreq_raw = (ct_freq - (1000 * FFT_N - 10 * idx_dwd - 10 * idx_upd - 2000 * OVLP - 2000 * tuner_offset) * Fs / (2 * FFT_N));

//SNR = 10*log10(sig_level^2 / noise_level^2-1);

#if 0
			//SNR = 10*log10(((double)sig_level*(double)sig_level) / (noise_level*noise_level)-1);
			if(((double)sig_level * (double)sig_level) > 3.512 * (ch_noise_lev * ch_noise_lev))//SNR_filt_thr =4 so 2.512
			//if(((double)sig_level*(double)sig_level)>1.5*(noise_level*noise_level))
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

	    //if ((SymRate>SR_min) && (SymRate<SR_max) && (SNR> SNR_filt_thr))
	    if ((SymRate_raw > 1200) && (SymRate_raw < SR_max)) // -2.0))
	    {
		if (SNR > SNR_filt_thr) {
		    idx_notch_last_tureCh = idx_notch;
		}

		if (SymRate_raw > SR_X_thr) {
		    ch_thr_high = (sig_level * (5 + K_high) + ch_thr_L * (5 - K_high) + 5) / 10;
		    mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_notch_last, idx_ch_max);
		    ch_thr_low = (ch_thr_L * (5 - K_low) + val * (5 + K_low) + 5) / 10;

		    for (idx_tmp = idx_ch_max; idx_tmp >= idx_up; idx_tmp--) {
			idx_L_high = idx_tmp;
			if ((psd[idx_tmp] <= ch_thr_high) && (psd[idx_tmp + 1] > ch_thr_high))
			    break;
		    }

		    for (idx_tmp = idx_up; idx_tmp >= idx_notch_last; idx_tmp--) {
			idx_L_low = idx_tmp;
			if ((psd[idx_tmp] <= ch_thr_low) && (psd[idx_tmp + 1] > ch_thr_low))
			    break;
		    }

		    if (idx_L_high <= idx_L_low)
			slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) + 1) / 2;
		    else
			slpoe_L = (2 * (10 * ch_thr_high - 10 * ch_thr_low) / (idx_L_high - idx_L_low) + 1) / 2;

		    //  idx_start_fix = idx_up-round((ch_thr_L - noise_level)/slpoe_L);
		    if (slpoe_L == 0)
			slpoe_L = 1;

		    if (idx_upd > ((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L))
			idx_start_fix = idx_upd - (((1000 * ch_thr_L - 1000 * noise_level) / slpoe_L) + 50) / 100 * 100; ///100;
		    else
			idx_start_fix = 0;

		    ch_thr_high = (sig_level * (5 + K_high) + ch_thr_R * (5 - K_high) + 5) / 10;
		    mt_fe_dmd_bs_find_Min(psd, &val, &idx_tmp, idx_ch_max, idx_notch);

		    //ch_thr_low = ch_thr_L*(0.5-K_low) + min(psd(idx_ch_max:idx_notch))*(0.5+K_low);
		    ch_thr_low = (ch_thr_R * (5 - K_low) + val * (5 + K_low) + 5) / 10;

		    for (idx_tmp = idx_ch_max; idx_tmp <= idx_dw; idx_tmp++) {
			idx_R_high = idx_tmp;
			if ((psd[idx_tmp + 1] <= ch_thr_high) && (psd[idx_tmp] > ch_thr_high))
			    break;
		    }

		    for (idx_tmp = idx_dw; idx_tmp <= idx_notch; idx_tmp++) {
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

		    idx_end_fix = ((1000 * (ch_thr_R - noise_level) / slpoe_R) + 50) / 100 * 100 + idx_dwd; ///100;

		    //idx_start_fix = max(idx_start_fix,idx_notch_last);
		    //idx_end_fix = min(idx_end_fix,idx_notch);
		    if (idx_start_fix < (100 * idx_notch_last))
			idx_start_fix = 100 * idx_notch_last;

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

		    // CtFreq_fix = (idx_start_fix/2+idx_end_fix/2-(FFT_N-2*OVLP)/2+tuner_offset)/FFT_N*(double)Fs+ct_freq;
		} else {
		    SymRate_fix = 0;
		    CtFreq_fix = 0;
		}

		if ((SymRate_raw > SR_X_thr) && (SymRate_fix > SymRate_raw)) {
		    SymRate = SymRate_fix;
		    CtFreq = CtFreq_fix;

		    if (sub_TP_spilt_mode)
			;
		    else {
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
		    //if(SymRate_fix >SymRate_raw)
		    //	idx_dw_tmp = idx_dw + (((SymRate_fix - SymRate_raw)*FFT_N)+Fs)/(2*Fs*1000);
		    //else
		    //	idx_dw_tmp = idx_dw - (((SymRate_raw -SymRate_fix )*FFT_N)+Fs)/(2*Fs*1000);
		    //if(sub_TP_spilt_mode==0)
		    //idx_dw_tmp = min(idx_notch,idx_dw + round((SymRate_fix - SymRate_raw)/2/Fs*FFT_N));
		    // idx_dw_tmp = min(idx_notch,round((CtFreq + SymRate/2  - ct_freq)/Fs*FFT_N)+FFT_N/2-OVLP+tuner_offset);
		} else {
		    SymRate = SymRate_raw;
		    CtFreq = CtFreq_raw;
		    if (sub_TP_spilt_mode == 0)
			idx_dw_tmp = idx_dw * 100;
		}

		if (SNR > SNR_filt_thr) {
		    if (handle->global_cfg.iScannedTpCnt > 100) {
			mt_fe_print(("global_cfg.iScannedTpCnt > 100!\n"));

			handle->mt_sleep(20000);
		    }

#if 1
		    //handle->global_cfg.iScannedTpCnt ++;
		    //printk("%s@%d: handle->global_cfg.iScannedTpCnt=%d\n",__FUNCTION__,__LINE__,
		    //	handle->global_cfg.iScannedTpCnt);
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = CtFreq;
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate;
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;

		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
		    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
		    handle->global_cfg.iScannedTpCnt++;
#endif

		    tp_info.freq_KHz = CtFreq;
		    tp_info.sym_rate_KSs = SymRate;
		    tp_info.uidx_notch = idx_notch;
		    tp_info.uidx_notch_last = idx_notch_last;
		    tp_info.uidx_up = idx_up;
		    tp_info.upsd_sn = idx_ps_sn;
		    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpFind, &tp_info);
		}

		if (sub_TP_spilt_mode == 0) {
		    //if(idx_dw_last > 0)
		    {
			//if ((S32)idx_up < idx_dw_last)
			//mt_fe_print(("merge err idx_up=%d,idx_dw_last=%d\r\n",idx_up,idx_dw_last));
		    }

		    if (((((S32)idx_upd - idx_dw_last) * 10 * Fs / FFT_N) < (S32)(SymRate_last + SymRate) * (MERGE_THR - 100) / 200) && (SymRate_last > 3000) && (SymRate_raw > 3000)
				&& ((SymRate_last + SymRate) < SR_max ) && ((S32)(100 * idx_up) >= idx_dw_last))
			{
			if (((SNR_last > SNR) && (((SNR - 1000) * 4) >= (SNR_last - 1000)) && (SNR_last > SNR_filt_thr)) || ((SNR_last < SNR) && (((SNR_last - 1000) * 4) >= (SNR - 1000)) && (SNR > SNR_filt_thr))) {
#if 1
				//printk("%s@%d: handle->global_cfg.iScannedTpCnt=%d\n",__FUNCTION__,__LINE__,
				//	handle->global_cfg.iScannedTpCnt);
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = (CtFreq_raw + CtFreq_last) / 2;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate_last + SymRate;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
			    p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
			    handle->global_cfg.iScannedTpCnt++;
#endif

			    tp_info.freq_KHz = (CtFreq_raw + CtFreq_last) / 2;
			    tp_info.sym_rate_KSs = SymRate_last + SymRate;
			    tp_info.uidx_notch_last = idx_notch_last;
			    tp_info.uidx_up = idx_up;
			    tp_info.uidx_notch = idx_notch;
			    tp_info.upsd_sn = idx_ps_sn;
			    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpFind, &tp_info);
			}
		    } else {
			cap_value0 = psd[idx_notch_last] - psd[idx_notch];
			cap_value1 = psd[idx_ch_max] - psd[idx_notch];
			if ((1000 * cap_value0) > (707 * cap_value1))
			    //if (((psd[idx_notch_last] - psd[idx_notch])*0.707)>(psd[idx_ch_max]-psd[idx_notch]) )
			{
			    if (handle->global_cfg.iScannedTpCnt > 1) {
				if (p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 2].upsd_sn == p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 1].upsd_sn) {
				    if (p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 2].uidx_notch == p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt - 1].uidx_notch_last) {
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

#if 1
					//printk("%s@%d: handle->global_cfg.iScannedTpCnt=%d\n",__FUNCTION__,__LINE__,
					//	handle->global_cfg.iScannedTpCnt);
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].freq_KHz = CtFreq_merge;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].sym_rate_KSs = SymRate_merge;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].dvb_type = MtFeType_DTV_Unknown;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch_last = idx_notch_last;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_up = idx_up;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].uidx_notch = idx_notch;
					p_bs_info->p_tp_info[handle->global_cfg.iScannedTpCnt].upsd_sn = idx_ps_sn;
					handle->global_cfg.iScannedTpCnt++;
#endif

					tp_info.freq_KHz = CtFreq_merge;
					tp_info.sym_rate_KSs = SymRate_merge;
					tp_info.uidx_notch_last = idx_notch_last;
					tp_info.uidx_up = idx_up;
					tp_info.uidx_notch = idx_notch;
					tp_info.upsd_sn = idx_ps_sn;
					mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpFind, &tp_info);
				    }
				}
			    }
			}
		    }
		}

		//if(handle->global_cfg.iScannedTpCnt > 0)
		if (sub_TP_spilt_mode == 0) {
		    CtFreq_last = CtFreq;   //CtFreq_raw;
		    SymRate_last = SymRate; //SymRate_raw;
		    idx_dw_last = (S32)idx_dw_tmp;
		    SNR_last = SNR;
		}

		//handle->global_cfg.iScannedTpCnt ++;
	    }

#if 0 ////20161026
		}
#endif

#if 0 //20161026
		else
		{
			if ((idx_dw + idx_up + 2 * OVLP + 2 * tuner_offset) > FFT_N)
				CtFreq = (idx_dw + idx_up + 2 * OVLP + 2 * tuner_offset - FFT_N) * Fs / (2 * FFT_N) + ct_freq;
			else
				CtFreq = ct_freq - (FFT_N - idx_dw - idx_up - 2 * OVLP - 2 * tuner_offset) * Fs / (2 * FFT_N);

			//mt_fe_print(("measure deal_ch,FREQ=%fM   SymRate=%fK\n",CtFreq,SymRate));
		}
#endif

	    /*%----- split one big TP to several small TPs by intermediate notches*/
	    if (sub_TP_spilt_mode == 0) //% When spilt TP is under detection, once split TP was detected, this step will be skipped
	    {
		//idx_up = (U32)(idx_upd/100);
		//idx_dw = (U32)(idx_dwd/100);
		all_thr_idx = 0;

		for (idx_tmp = idx_up + 1; idx_tmp <= idx_ch_max; idx_tmp++) {
		    if (psd[idx_tmp] < ch_thr_L) {
			all_under_thr_idx[all_thr_idx] = idx_tmp;
			all_thr_idx = all_thr_idx + 1;
			if (all_thr_idx >= 20)
			    break;
		    }
		}

		for (idx_tmp = idx_ch_max + 1; idx_tmp <= idx_dw; idx_tmp++) {
		    if (psd[idx_tmp] < ch_thr_R) {
			all_under_thr_idx[all_thr_idx] = idx_tmp;
			all_thr_idx = all_thr_idx + 1;
			if (all_thr_idx >= 20)
			    break;
		    }
		}

		// all_under_thr_idx = find(psd(floor(idx_up)+1:floor(idx_dw))<ch_thr_L)+floor(idx_up);
		// if ((cnt_up > 1 || cnt_dw > 1) && (SymRate_raw >2000 && SymRate_raw<36000 && SNR> 0.2))//% more than one threshold crossing point
		//if (isempty(all_under_thr_idx)==0) && (SymRate_raw > SR_min*2 && SymRate_raw< SR_max && SNR> SNR_filt_thr)
		if ((all_thr_idx > 0) && (SymRate_raw > 2400 && SymRate_raw < SR_max && SNR > SNR_filt_thr)) //% more than one threshold crossing point
		{
		    // disp('split found TP');
		    L1 = all_under_thr_idx[0];
		    ii = 1;

		    if (all_thr_idx > 20) {
			all_thr_idx = 20;
			mt_fe_print(("all_thr_idx > 20 !\n"));
		    }

		    for (idx = 0; idx < all_thr_idx; idx++) {
			if (((idx + 1) < all_thr_idx) && (all_under_thr_idx[idx + 1] - all_under_thr_idx[idx] > 1)) {
			    mt_fe_dmd_bs_find_Min(psd, &val, &pos, L1, all_under_thr_idx[idx]);
			    idx_notch_split[ii - 1] = L1 + pos - 1;
			    L1 = all_under_thr_idx[idx + 1];
			    ii = ii + 1;
			} else if ((idx + 1) == all_thr_idx) {
			    mt_fe_dmd_bs_find_Min(psd, &val, &pos, L1, all_under_thr_idx[idx]);
			    idx_notch_split[ii - 1] = L1 + pos - 1;
			    ii = ii + 1;
			}
		    }

		    for (idx = 0; idx < ii - 1; idx++) {
			mt_fe_dmd_bs_find_Max(psd, &val, &pos, idx_notch_split[idx], idx_notch);
			pos = pos + idx_notch_split[idx] - 1;

			if ((4 * (psd[pos] - psd[idx_notch_split[idx]])) < (psd[pos] - psd[idx_notch])) {
			    ii = idx + 1;
			    break;
			}
		    }

		    sub_TP_N = ii;

		    if (sub_TP_N > MAX_sub_TP_N) {
			mt_fe_print(("sub_TP_N too large %d!!!\n", sub_TP_N));
			sub_TP_N = MAX_sub_TP_N;
			idx_sub_TP = 0;
			sub_TP_spilt_mode = 1;
			notch_all[0] = idx_notch_last;

			for (i = 1; i < MAX_sub_TP_N; i++) {
			    notch_all[i] = idx_notch_split[i - 1];
			}

			notch_all[sub_TP_N] = idx_notch;
		    } else if (sub_TP_N == 1) {
			break;
		    } else {
			idx_sub_TP = 0;
			sub_TP_spilt_mode = 1;
			notch_all[0] = idx_notch_last;

			for (i = 1; i <= ii - 1; i++) {
			    notch_all[i] = idx_notch_split[i - 1];
			}

			notch_all[sub_TP_N] = idx_notch;
			if (idx_notch == 0)
			    idx_notch = 0;
		    }
		} else {
		    break; //% No suitable sub TP splited
		}
	    }

	    // % if split TP was detected (sub_TP_spilt_mode == 1), measured all the sub TP
	    idx_sub_TP = idx_sub_TP + 1; //% measure all the children TPs contains in current parent TP in turn (from lest to right)
	    if (idx_sub_TP > sub_TP_N) {
		break; //% when all sub TPs were done
	    } else     //% measure all children TP (from left to right)
	    {
		idx_notch_last = notch_all[idx_sub_TP - 1];
		idx_notch = notch_all[idx_sub_TP];
		if (idx_notch == 0)
		    idx_notch = 0;

		mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
		//[ch_min idx_ch_min]= min(psd(idx_notch_last:idx_notch));% find the min in current searched range
		mt_fe_dmd_bs_find_Min(psd, &ch_min, &idx_ch_min, idx_notch_last, idx_notch);
		idx_ch_max = idx_ch_max + idx_notch_last - 1; //% to align the idx with psd start
		idx_ch_min = idx_ch_min + idx_notch_last - 1;
		idx_up = 1;
		idx_dw = 1;
	    }
	} else
	    break;

	if (rep_flag == 1)
	    rep_flag = 0;
    }
}

MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_process_get_range_list(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, U32 *range_list, U8 *list_num)
{
    ///////////////////////////20160727
    U8 F_step = 24;            //% effective freq range sweep step
    S8 sig_thr_low_bound = 30; //55;//8;//% The lower bound of effective signal strength
    S8 MAX_sig_stre_dif = 15;  //10;//25;//% the larger this value, the less range of effective freq range
    //float alpha = 1/2;
    S32 sig_stre_grid = 0;
    U8 F_idx = 0;
    S8 dwTmp1 = 0;
    U8 map_idx = 0;
    S8 ave_sig_str = 0, sig_stre_thr = 0;
    U8 find_ok = 0;
    S8 F_grid_map_tmp[108]; //(2150-950)/12=100
                            //	U8	F_grid_map[108];
    U8 weak_sig_thr = 65;   //20;
    U8 min_sig = 0xff;
    U8 max_sig = 0x00;
    U8 cnt = 0;
    U16 i = 0, j = 0;
    U32 freq_KHz = 0, ct_freq_actual = 0, real_freq_KHz = 0;
    U8 fre_seg_idx = 0;
    U32 set_freq = 0;
//clock_t  clock6, clock7;

//S8 iFRangeList[54][2];
///////////////////

#if FIND_DEBUG
    FILE *fpOut1;
    S32 gain_val = 0;
    S8 iStrength = 0;
    char file_name1[100];
    char str3[100];
    char str4[20];
#else
    //////////////add b wbj///////////////
    MT_FE_BOOL b_analog_locked, b_fftdone;

    U8 tmp;
//	U8 tmp1,tmp2,tmp3,tmp4;
#endif

    ///////////////////////////////20160727
    set_freq = begin_freq_MHz;
    while (set_freq < end_freq_MHz) {
	freq_KHz = set_freq * 1000;
	handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;

#if FIND_DEBUG
	sprintf(file_name1, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H" : "L",
	        (handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);

	fpOut1 = fopen(file_name1, "r");
	fgets(str3, 100, fpOut1);
	fgets(str3, 100, fpOut1);
	fgets(str3, 100, fpOut1);
	fgets(str3, 100, fpOut1);
	fgets(str3, 100, fpOut1);
	fgets(str3, 100, fpOut1);
	//	fgets(str,100,fpOut);
	strmcp(str4, str3, 9);

	dwTmp1 = atoi(str4);
//sig_stre = dwTmp1;
#else
	//////////////add b wbj///////////////
	//clock6 = clock();
	_mt_fe_dmd_cs8k_sat_bs_set_reg_psd(handle);
	handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;
	/*
		handle->dmd_set_reg(handle, 0x06, 0xe0);

		_mt_fe_dmd_rs6k_select_mclk(handle, handle->tp_cfg.iFreqKHz / 1000);
		_mt_fe_dmd_rs6k_set_ts_mclk(handle, 96000);
		handle->dmd_set_reg(handle, 0x06, 0x00);
		*/
	//clock7 = clock();
	// mt_fe_print(("other time %d\n", clock7 - clock6));

	//handle->mt_sleep(10);
	//clock6 = clock();
	if (handle->lnb_cfg.bUnicable) {
	    mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    freq_KHz = real_freq_KHz;
	} else {
	    handle->tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
	}

	//clock7 = clock();
	//mt_fe_print(("set tuner time %d\n", clock7 - clock6));
	//_mt_fe_dmd_rs6k_set_carrier_offset(handle, mt_fe_tn_get_offset(handle));

	//	tmp_freq_KHz = 0;
	//	TP_found_flag = 0;

	//	ret = _mt_fe_dmd_rs6k_bs_process_scanned_TP_test(handle, p_bs_info, freq_KHz, bs_times,&sig_stre);
	handle->dmd_set_reg(handle, 0x90, 0x70);
	//handle->dmd_get_reg(handle, 0x94, &tmp);
	//handle->dmd_get_reg(handle, 0x95, &tmp1);
	//handle->dmd_get_reg(handle, 0x97, &tmp2);
	//handle->dmd_get_reg(handle, 0x99, &tmp3);
	//handle->dmd_get_reg(handle, 0x90, &tmp4);
	//mt_fe_print(("0x94=0x%x,0x95=0x%x,0x97=0x%x,0x99=0x%x,0x90=0x%x\n", tmp,tmp1,tmp2,tmp3,tmp4));

	for (i = 0; i < 2; i++) {
	    b_fftdone = MtFe_False;
	    b_analog_locked = MtFe_False;
	    cnt = 50;
	    do {
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
	mt_fe_dmd_cs8k_sat_get_strength(handle, &dwTmp1);
#endif

	F_grid_map_tmp[F_idx] = dwTmp1;
	sig_stre_grid = dwTmp1 + sig_stre_grid;

	if (min_sig > dwTmp1)
	    min_sig = (U8)dwTmp1;

	if (max_sig <= dwTmp1)
	    max_sig = (U8)dwTmp1;

	F_idx++;
	set_freq = set_freq + F_step;
    }

    fre_seg_idx = 0;
    if (min_sig > weak_sig_thr) {
	range_list[2 * fre_seg_idx] = begin_freq_MHz;
	range_list[2 * fre_seg_idx + 1] = end_freq_MHz;
	fre_seg_idx++;
    } else {
	if (F_idx == 0)
	    F_idx = 1;

	sig_stre_thr = sig_stre_grid / F_idx;
	// sig_stre_thr = alpha*max(sig_stre_grid) + (1-alpha)*min(sig_stre_grid);
	sig_stre_thr = (max_sig + min_sig) / 2;
	sig_stre_grid = 0;
	map_idx = 0;

	for (i = 0; i < F_idx; i++) {
	    if (F_grid_map_tmp[i] >= sig_stre_thr) {
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

	for (i = 0; i < F_idx;) {
	    if (F_grid_map_tmp[i] < ave_sig_str) {
		cnt++;
	    } else {
		if (cnt <= 48 / F_step) {
		    for (j = 0; j <= cnt; j++) {
			F_grid_map_tmp[i - j] = ave_sig_str + 2;
		    }

		    cnt = 0;
		}
	    }

	    i++;
	}

	if (F_grid_map_tmp[0] >= ave_sig_str)
	    range_list[0] = ct_freq_actual;

	for (i = 1; i < F_idx;) {
	    if ((F_grid_map_tmp[i - 1] < ave_sig_str) && (F_grid_map_tmp[i] >= ave_sig_str)) {
		range_list[2 * fre_seg_idx] = ct_freq_actual + F_step * (i - 1);
	    } else if ((F_grid_map_tmp[i - 1] >= ave_sig_str) && (F_grid_map_tmp[i] < ave_sig_str)) {
		if (range_list[2 * fre_seg_idx] == 0)
		    range_list[2 * fre_seg_idx] = ct_freq_actual;

		range_list[2 * fre_seg_idx + 1] = ct_freq_actual + F_step * (i);

		if (end_freq_MHz - range_list[2 * fre_seg_idx + 1] <= F_step)
		    range_list[2 * fre_seg_idx + 1] = end_freq_MHz;

		fre_seg_idx++;
	    } else if ((i == (F_idx - 1)) && (F_grid_map_tmp[i] >= ave_sig_str)) {
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

    if (fre_seg_idx > 1) {
	for (i = 1; i < fre_seg_idx; i++) {
	    if ((range_list[2 * i] - F_step) <= range_list[2 * (i - 1) + 1]) {
		range_list[2 * (i - 1) + 1] = range_list[2 * i + 1];
		fre_seg_idx = fre_seg_idx - 1;

		for (j = i; j < fre_seg_idx; j++) {
		    range_list[2 * j + 1] = range_list[2 * (j + 1) + 1];
		    range_list[2 * j] = range_list[2 * (j + 1)];
		}
	    }
	}
    }

    *list_num = fre_seg_idx;

    return MtFeErr_Ok;
}

/*************************************************************/
MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_connect_test(MT_FE_CS8000_SAT_Device_Handle handle, MT_FE_BS_TP_INFO *p_bs_info)
{
    U16 index, timing_thr, crl_thr;
    U8 count, timing_fail, crl_fail, tmp, same_locked_TP_flag, val_reg08 = 0;
    U32 sym_rate_KSs, centerfreq_KHz, truefreq_KHz;
    S16 lpf_offset_KHz, i;
    S32 carrieroffset_KHz, delta_freq_KHz, delta_smy_rate_KSs, deviceoffset_KHz;
    //U32		target_mclk = MT_FE_MCLK_KHZ;
    //MT_FE_TS_OUT_MODE	ts_mode = MtFeTsOutMode_Common;

    MT_FE_RET ret = MtFeErr_Ok;

    U32 real_freq_KHz;
    S32 iTunerOffsetKHz = 0;

    MT_FE_TYPE current_mode = MtFeType_DvbS;
    MT_FE_LOCK_STATE lockstate;
    MT_FE_TP_INFO *p_scanned_tp_info = (MT_FE_TP_INFO *)(p_bs_info->p_tp_info);

    //S32 gain_val = 0, delta = 0;

    U8 ucTmp;
    int iCnt;

    if ((handle->tuner_cfg.tuner_set == NULL) || (handle->tuner_cfg.tuner_get_offset == NULL))
	return MtFeErr_NullPointer;

	//printk("%s@%d: handle->global_cfg.iScannedTpCnt=%d\n",__FUNCTION__,__LINE__,
	//	handle->global_cfg.iScannedTpCnt);

    for (index = 0; index < handle->global_cfg.iScannedTpCnt; index++)
        //for(index = start_index; index < scanned_tp; index++)
    {
		//printk("%s: bs connect TP - %d\n",__FUNCTION__,index);

	if (handle->global_cfg.bCancelBs)
	    break;

	if (p_scanned_tp_info[index].dvb_type != MtFeType_DTV_Unknown)
	    continue;

	p_scanned_tp_info[index].bCheckCCM = FALSE;
	p_scanned_tp_info[index].iTsCnt = 0;
	p_scanned_tp_info[index].ucCurTsId = 0;

	for (i = 0; i < 16; i++) {
	    p_scanned_tp_info[index].ucTsId[i] = 0;
	}

	centerfreq_KHz = p_scanned_tp_info[index].freq_KHz;
	sym_rate_KSs = p_scanned_tp_info[index].sym_rate_KSs;

	lpf_offset_KHz = 0;
	truefreq_KHz = 0;

	mt_fe_dmd_cs8k_sat_global_reset(handle);
	handle->dmd_set_reg(handle, 0xb2, 0x01);
	handle->dmd_set_reg(handle, 0x00, 0x01);

	if ((sym_rate_KSs < 5000) && (!handle->lnb_cfg.bUnicable)) {
	    lpf_offset_KHz = 0;  //BLINDSCAN_LPF_OFFSET_KHz;
	    centerfreq_KHz += 0; //BLINDSCAN_LPF_OFFSET_KHz;
	}

	handle->dmd_get_reg(handle, 0x08, &val_reg08);
	val_reg08 = (U8)(val_reg08 & 0x3b);
	handle->dmd_set_reg(handle, 0x08, val_reg08);
	handle->dmd_set_reg(handle, 0xe0, 0xf8);

	//handle->global_cfg.bEqCoefChecked = FALSE;

	handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = sym_rate_KSs;

	if (handle->lnb_cfg.bUnicable) {
	    ret = mt_fe_cs8k_sat_unicable_set_tuner(handle, centerfreq_KHz, sym_rate_KSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    deviceoffset_KHz = real_freq_KHz - centerfreq_KHz;
	} else {
	    ret = handle->tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
	    real_freq_KHz = centerfreq_KHz;
	    deviceoffset_KHz = 0;
	}

	if (ret != MtFeErr_Ok) // Error parameter
	{
	    lockstate = MtFeLockState_Unlocked;
	    continue;
	}

	_mt_fe_dmd_cs8k_sat_bs_set_demod(handle, sym_rate_KSs);

	if (sym_rate_KSs <= 5000) //&& (current_mode == MtFeType_DvbS2))
	{
	    handle->dmd_set_reg(handle, 0xc0, 0x04);
	    handle->dmd_set_reg(handle, 0x8a, 0x09);
	    handle->dmd_set_reg(handle, 0x8b, 0x22);
	    handle->dmd_set_reg(handle, 0x8c, 0x88);
	}

	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz + lpf_offset_KHz + deviceoffset_KHz);

	handle->dmd_set_reg(handle, 0xb2, 0x00);

	handle->mt_sleep(70);

	lockstate = MtFeLockState_Waiting;

	if (lockstate == MtFeLockState_Unlocked) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;

	    continue;
	}

	if (sym_rate_KSs >= 10000) {
	    count = 25;
	    timing_thr = 1;
	    crl_thr = 1;
	} else if (sym_rate_KSs >= 4000) {
	    count = 28;
	    timing_thr = 1;
	    crl_thr = 1;
	} else if (sym_rate_KSs >= 2000) {
	    count = 35;
	    timing_thr = 1;
	    crl_thr = 1;
	} else {
	    count = 40;
	    timing_thr = 1;
	    crl_thr = 1;
	}

	timing_fail = 0;
	crl_fail = 0;
	lockstate = MtFeLockState_Unlocked;

	while ((count > 0) && (!handle->global_cfg.bCancelBs)) {
	    handle->dmd_get_reg(handle, 0x08, &tmp);
	    if ((tmp & 0x08) != 0) {
		handle->tp_cfg.mCurrentType = MtFeType_DvbS2;
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0x8f) == 0x8f) {
		    lockstate = MtFeLockState_Locked;

		    if (!p_scanned_tp_info[index].bCheckCCM) {
			handle->dmd_set_reg(handle, 0xE5, 0x10);
			handle->dmd_set_reg(handle, 0xE5, 0x40);
			handle->mt_sleep(10);
			handle->dmd_get_reg(handle, 0xE5, &ucTmp);
			p_scanned_tp_info[index].iTsCnt = ucTmp & 0x0F;

			if (p_scanned_tp_info[index].iTsCnt > 1) {
			    mt_fe_print(("\tCCM find after FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));

			    for (iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt++) {
				ucTmp = (U8)iCnt;

				handle->dmd_set_reg(handle, 0xE6, ucTmp);
				handle->dmd_get_reg(handle, 0xE7, &ucTmp);

				p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;

				mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
			    }

			    handle->dmd_set_reg(handle, 0xE6, 0x0F);
			    handle->dmd_set_reg(handle, 0xE7, p_scanned_tp_info[index].ucTsId[0]); // default select TS 0
			    p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
			    mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
			} else {
			    handle->dmd_set_reg(handle, 0xE5, 0x00);
			}

			p_scanned_tp_info[index].bCheckCCM = TRUE;
		    }
		} else if (((tmp & 0x0C) == 0x0C) && (!p_scanned_tp_info[index].bCheckCCM)) {
		    handle->dmd_set_reg(handle, 0xE5, 0x10);
		    handle->dmd_set_reg(handle, 0xE5, 0x40);
		    handle->mt_sleep(10);
		    handle->dmd_get_reg(handle, 0xE5, &ucTmp);
		    p_scanned_tp_info[index].iTsCnt = ucTmp & 0x0F;

		    if (p_scanned_tp_info[index].iTsCnt > 1) {
			mt_fe_print(("\tCCM find before FEC! Total %d streams!\n", p_scanned_tp_info[index].iTsCnt));

			for (iCnt = 0; iCnt < p_scanned_tp_info[index].iTsCnt; iCnt++) {
			    ucTmp = (U8)iCnt;

			    handle->dmd_set_reg(handle, 0xE6, ucTmp);
			    handle->dmd_get_reg(handle, 0xE7, &ucTmp);

			    p_scanned_tp_info[index].ucTsId[iCnt] = ucTmp;

			    mt_fe_print(("\tTS %2d -- 0x%02X\n", iCnt, ucTmp));
			}

			handle->dmd_set_reg(handle, 0xE6, 0x0F);
			handle->dmd_set_reg(handle, 0xE7, p_scanned_tp_info[index].ucTsId[0]); // default select TS 0
			p_scanned_tp_info[index].ucCurTsId = p_scanned_tp_info[index].ucTsId[0];
			mt_fe_print(("\tDefault select TS 0 ---- 0x%02X\n", handle->tp_cfg.ucTsId[0]));
		    } else {
			handle->dmd_set_reg(handle, 0xE5, 0x00);
		    }

		    p_scanned_tp_info[index].bCheckCCM = TRUE;
		}
	    } else {
		handle->tp_cfg.mCurrentType = MtFeType_DvbS;
		handle->dmd_get_reg(handle, 0x0d, &tmp);
		if ((tmp & 0xf7) == 0xf7) {
		    lockstate = MtFeLockState_Locked;
		}
	    }

	    current_mode = handle->tp_cfg.mCurrentType;

	    if (lockstate == MtFeLockState_Locked) {
		S32 carrier_offset;
		U8 reg_0x0c;

		if (sym_rate_KSs >= 3000) {
		    p_bs_info->iPreGain = p_bs_info->iCurGain;
		    p_bs_info->bNewGain = FALSE;
		}

		handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
		if (((reg_0x0c & 0x04) == 0x00) && (!handle->lnb_cfg.bUnicable)) {
		    // AFC on
		    _mt_fe_dmd_cs8k_sat_get_total_carrier_offset(handle, &carrier_offset);
		    if ((carrier_offset > ((S32)sym_rate_KSs)) || (carrier_offset < -((S32)sym_rate_KSs))) {
			handle->tuner_cfg.tuner_set(handle, centerfreq_KHz, sym_rate_KSs, lpf_offset_KHz);
			real_freq_KHz = centerfreq_KHz;
			deviceoffset_KHz = 0;

			handle->tp_cfg.iFreqKHz = (centerfreq_KHz + 500) / 1000 * 1000;

			handle->dmd_set_reg(handle, 0xb2, 0x01);

			_mt_fe_dmd_cs8k_sat_bs_set_demod(handle, sym_rate_KSs);

			if (sym_rate_KSs <= 5000) //&&(current_mode == MtFeType_DvbS2))
			{
			    handle->dmd_set_reg(handle, 0xc0, 0x04);
			    handle->dmd_set_reg(handle, 0x8a, 0x09);
			    handle->dmd_set_reg(handle, 0x8b, 0x22);
			    handle->dmd_set_reg(handle, 0x8c, 0x88);
			}

			handle->dmd_get_reg(handle, 0x0c, &reg_0x0c);
			reg_0x0c |= 0x04;
			handle->dmd_set_reg(handle, 0x0c, reg_0x0c);

			handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
			_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz + lpf_offset_KHz + deviceoffset_KHz);

			handle->dmd_set_reg(handle, 0xb2, 0x00);

			count = 40;
			lockstate = MtFeLockState_Waiting;
			mt_fe_print(("\tCarrier offset out of range! Frequency = %d, CarrierOffset = %d, SymbolRate = %d\n", centerfreq_KHz, carrier_offset, sym_rate_KSs));
		    } else {
			// normal carrier offset
			break;
		    }
		} else {
		    // AFC off
		    break;
		}
	    } else {
		handle->dmd_get_reg(handle, 0xb3, &tmp);
		if ((tmp & 0x80) == 0x80) {
		    timing_fail++;
		    if (timing_fail >= timing_thr) {
			timing_fail = 1;
			break;
		    }
		    mt_fe_dmd_cs8k_sat_soft_reset(handle);
		    count = 30;
		} else if (((tmp & 0x40) == 0x40) && ((tmp & 0x80) != 0x80)) {
		    crl_fail++;
		    if (crl_fail >= crl_thr) {
			crl_fail = 1;
			break;
		    }
		    mt_fe_dmd_cs8k_sat_soft_reset(handle);
		    count = 50;
		}
	    }

	    handle->mt_sleep(50);
	    count--;
	}

	if (count == 0) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    continue;
	}

	if (timing_fail == 1) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    mt_fe_print(("timing fail!!!\n"));
	    continue;
	} else if (crl_fail == 1) {
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	    p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;
	    continue;
	}

	if (lockstate == MtFeLockState_Locked) {
	    MT_FE_CHAN_INFO_DVBS2 ch_info;

	    _mt_fe_dmd_cs8k_sat_get_carrier_offset(handle, &carrieroffset_KHz);

	    handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	    truefreq_KHz = real_freq_KHz + iTunerOffsetKHz - carrieroffset_KHz;

	    _mt_fe_dmd_cs8k_sat_get_sym_rate(handle, &sym_rate_KSs);

	    same_locked_TP_flag = 0;
	    for (i = (S16)(handle->global_cfg.iScannedTpCnt - 1); i >= 0; i--) {
		if (handle->global_cfg.bCancelBs)
		    break;

		if (p_bs_info->p_tp_info[i].freq_KHz > truefreq_KHz)
		    delta_freq_KHz = p_bs_info->p_tp_info[i].freq_KHz - truefreq_KHz;
		else
		    delta_freq_KHz = truefreq_KHz - p_bs_info->p_tp_info[i].freq_KHz;

		if (p_bs_info->p_tp_info[i].sym_rate_KSs > sym_rate_KSs)
		    delta_smy_rate_KSs = p_bs_info->p_tp_info[i].sym_rate_KSs - sym_rate_KSs;
		else
		    delta_smy_rate_KSs = sym_rate_KSs - p_bs_info->p_tp_info[i].sym_rate_KSs;

		if (((delta_smy_rate_KSs < (p_bs_info->p_tp_info[i].sym_rate_KSs / 10)) && (delta_freq_KHz < (p_bs_info->p_tp_info[i].sym_rate_KSs / 2))) &&
		    (p_bs_info->p_tp_info[i].dvb_type == current_mode)) {
		    same_locked_TP_flag = 1;
		    p_scanned_tp_info[index].dvb_type = MtFeType_DTV_Checked;

		    break;
		}
	    }

	    if (same_locked_TP_flag == 1)
		continue;

	    mt_fe_dmd_cs8k_sat_get_channel_info(handle, &ch_info);

	    if ((ch_info.code_rate == MtFeCodeRate_3_4) && (ch_info.mod_mode == MtFeModMode_8psk)) {
		handle->dmd_set_reg(handle, 0x9e, 0x9f);
		handle->dmd_set_reg(handle, 0x9f, 0x0c);

		handle->dmd_get_reg(handle, 0x9d, &tmp);
		tmp &= 0xFE;
		handle->dmd_set_reg(handle, 0x9d, tmp);
	    }

	    handle->global_cfg.iLockedTpCnt++;
	    p_bs_info->p_tp_info[index].freq_KHz = truefreq_KHz;
	    p_bs_info->p_tp_info[index].sym_rate_KSs = (U16)sym_rate_KSs;
	    p_bs_info->p_tp_info[index].dvb_type = current_mode;
	    _mt_fe_dmd_cs8k_sat_get_fec(handle, &(p_bs_info->p_tp_info[index]), current_mode);

	    p_bs_info->tp_num = handle->global_cfg.iLockedTpCnt;
	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpLocked, &(p_bs_info->p_tp_info[index]));

	} else {
	    if (current_mode == MtFeType_DvbS2)
		p_bs_info->p_tp_info[index].dvb_type = MtFeType_DTV_Checked;

	    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSTpUnlock, &(p_bs_info->p_tp_info[index]));
	}
    }

    return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP_test(MT_FE_CS8000_SAT_Device_Handle handle, U32 *psd, U32 cur_scan_freq_KHz, U8 bs_times, S8 *strength)
{
    MT_FE_BOOL b_analog_locked, b_fftdone;

    U8 tmp, cnt;
    U16 totaltpnum;
    U8 tmp1, tmp2, tmp3, tmp4;
    //clock_t	clock1, clock2, clock3;//, clock4;
    S8 iStrength = 0;
    U32 nSize = (FFT_N - 16 * 2) * 2; // sm_buf = 2, overlap = 16, nSize = 968
    U8 buffer[968] = { 0 };           // sm = 2;
    //U32		dwPSDArray[484];						// 484 / 2
    U32 dwPSDArray[FFT_N + 10];
    U32 i = 0, j = 0;
    U32 dwTmp = 0;
    S32 diff[FFT_N + 2];
    U32 tone_idx = 0;
    S32 spur_thr = 5000;
    U32 mean_tone = 0;
    U32 nCount = 0;

#if PSD_WRITE
    FILE *fpOut;
    S32 gain_val = 0;

    char file_name[100];

    sprintf(file_name, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H" : "L",
            (handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V) ? "H" : "V", handle->tp_cfg.iFreqKHz / 1000);
#endif

    handle->dmd_set_reg(handle, 0x90, 0x70);

    for (i = 0; i < 2; i++) {
	b_fftdone = MtFe_False;
	b_analog_locked = MtFe_False;
	//clock1 = clock();
	cnt = 50;
	do {
	    handle->dmd_get_reg(handle, 0x0d, &tmp);
	    b_analog_locked = ((tmp & 0x01) == 0x01) ? MtFe_True : MtFe_False;
	    handle->mt_sleep(10);
	    cnt--;
	} while ((b_analog_locked == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

	if (cnt == 0)
	    break;

	handle->dmd_set_reg(handle, 0x9a, 0x80);
	//clock2 = clock();
	//mt_fe_print(("agc time %d\n", clock2 - clock1));
	cnt = 100; //50
	do {
	    handle->dmd_get_reg(handle, 0x9a, &tmp);
	    b_fftdone = ((tmp & 0x80) == 0x00) ? MtFe_True : MtFe_False;
	    handle->mt_sleep(10);
	    cnt--;
	} while ((b_fftdone == MtFe_False) && (cnt > 0) && (!handle->global_cfg.bCancelBs));

	//clock3 = clock();
	//mt_fe_print(("fftdone time %d\n", clock3-clock2));

	if (b_fftdone || (1 == i)) {
	    handle->dmd_get_reg(handle, 0x94, &tmp);
	    handle->dmd_get_reg(handle, 0x95, &tmp1);
	    handle->dmd_get_reg(handle, 0x97, &tmp2);
	    handle->dmd_get_reg(handle, 0x99, &tmp3);
	    handle->dmd_get_reg(handle, 0x90, &tmp4);
	    //mt_fe_print(("0x94=0x%x,0x95=0x%x,0x97=0x%x,0x99=0x%x,0x90=0x%x\n", tmp,tmp1,tmp2,tmp3,tmp4));
	    break;
	} else {
	    handle->dmd_set_reg(handle, 0x5f, 0x00);
	    handle->dmd_set_reg(handle, 0x5e, 0x70);
	}
    }

	//clock4 = clock();
	//mt_fe_print(("total time %d\n", clock4 - clock1));
	//mt_fe_print(("\tAnalog_Locked = %d, FFT_Done = %d\n", b_analog_locked, b_fftdone));
	//mt_fe_print(("\t------Get psd freq = %dKHz-------\n", cur_scan_freq_KHz));

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

    mt_fe_dmd_cs8k_sat_get_strength(handle, &iStrength);
    fprintf(fpOut, "Strength: %d\n", iStrength);
    *strength = iStrength;
    fclose(fpOut);
#else

    mt_fe_dmd_cs8k_sat_get_strength(handle, &iStrength);
    //fprintf(fpOut, "Strength: %d\n", iStrength);
    *strength = iStrength;
#endif

    for (i = 0; i < 5; i++) {
	handle->dmd_get_reg(handle, 0x3f, &tmp);
	//mt_fe_print(("\t0x%02x", tmp));
    }
    //mt_fe_print(("\n"));

    handle->dmd_get_reg(handle, 0x9a, &tmp);
    totaltpnum = (U16)(tmp & 0x1F);

    //#else
    //U32		nSize = (512 - 192 + 4) * 2;		// sm_buf = 4, nSize = 648
    //U8		buffer[648] = {0};					// sm = 4;
    //U32		dwPSDArray[324];					// 648 / 2


	//FILE	*fpOut;

	nCount = 1;

    //clock2 = clock();
    handle->dmd_set_reg(handle, 0x9A, 0x40);

    for (j = 0; j < nSize; j++) {
	handle->dmd_get_reg(handle, 0x9B, &tmp);
	buffer[j] = tmp;
    }


    dwTmp = FFT_N - 478;
    for (i = 0; i < dwTmp; i++) {
	psd[nCount++] = 0;
    }

    for (i = 2 * 2; i < nSize; i += 2) // sm_buf = 4
    {
	dwTmp = buffer[i];
	dwTmp = dwTmp + (buffer[i + 1] * 256);
	dwPSDArray[(i - 2) / 2] = dwTmp;
	psd[nCount++] = dwTmp;
    }


#if defined(ONLY_PSD_DEBUG)
    fpOut = fopen(file_name, "a");

	if (fpOut != NULL)
	{
	    for (i = 1; i <= nSize / 2; i++) {
			fprintf(fpOut, "%d\n", dwPSDArray[i]);
	    }
	    fclose(fpOut);
    }
    mt_fe_print(("Write PSD data freq:%dHZ ok!\r\n", handle->tp_cfg.iFreqKHz / 1000));
#endif

#if 1
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
					mean_tone = psd[i + 4] - psd[i - 2];//(psd[i-2]+psd[i-1]+psd[i]+psd[i+1]+psd[i+2]+psd[i+3]+psd[i+4])/6;
					for(j = 0; j < 5; j ++)
						psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];

					mean_tone = 0;
				}
				else
				{
					mean_tone = psd[i - 2] - psd[i + 4];//(psd[i-2]+psd[i-1]+psd[i]+psd[i+1]+psd[i+2]+psd[i+3]+psd[i+4])/6;
					for(j = 0; j < 5;  j++)
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
			if(j >= i)	// overflow bug fix, comment by YZ.Huang @ 171226, fix99487
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
	handle->mt_sleep(50);

	handle->dmd_set_reg(handle, 0x9A, 0x20);

	handle->dmd_get_reg(handle, 0x9B, &tmp1);
	handle->dmd_get_reg(handle, 0x9B, &tmp2);
	handle->dmd_get_reg(handle, 0x9B, &tmp3);
	handle->dmd_get_reg(handle, 0x9B, &tmp4);

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_cs8k_sat_bs_A_test(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info,
                                        U8 bs_times, U32 is_connect)
{
    //global gain_RF  gain_IF gain_BB gain_PWM gain_tuner sig_stre path_com PSD_AVE_N;
    //load FcFs_Tab.mat;
    //PSD_SRC = 'External1';//%using captured PSD for simulation
    U32 psd[FFT_N + 32];
    U32 SR_min = 1200; //% min symbol rate in Msps
    //double C=3/16;//%%%Ufix6_En6
    U8 K1 = 12;
    U16 K_thr1 = 707;
    U16 K_thr2 = 600;
    U8 SEARCH_STEP = 1; //% increasing step for the CH searching in one PSD
    //U32 PSD_AVE_N = 96;//128;  //                 %%%% specified FFT average time to produce a PSD
    //% OVLP=192;%for External1 data source;
    U32 FORCE_TUNER_SKIP = 8; //24;//%MHz, When no CH found, force Tuner to skip in freq
    U32 ris_point_lim = 12;   //8;
    U32 verf_range = 16;

    //%------------------- Variables initialization --------------------------
    U32 ct_freq_start = 950;
    U32 ct_freq_end = 2150;
    //S8	sig_stre_thr = 30;

    //%---------------------------------------------------------------------------------------------------------------
    U32 tuner_offset_en = 1;
    //U32 FcFs_Tab_en = 0;

    /*for rep_times=1*/

    //U32 ct_freq_last = 0;
    ///%     mode_sel =  'find_next_notch';
    U32 first_psd_flag = 1; //% the currrent search is the very first search attemp of current satellite
    U32 psd_SN = 1;         //% PSD serial number
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

    //U8	end_flag = 0;//, TP_found_flag;
    U32 freq_KHz, tmp_tp_scanned; //, tmp_freq_KHz;
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
    U8 fre_seg_idx = 0;
    U8 i = 0;
    U32 j = 0;
    U32 idx_tmp = 0, idx_pwr_rg_start = 0, idx_pwr_rg_end = 0;
    //U8  fir_freq = 0;
    U32 iTunerOffsetKHz = 0;
    U8 flag_scan_done = 0;
    U32 sig_thr = 0; //, noise_level = 0;
    U32 sig_level = 0;
    U32 sig_idx = 0;
    U32 nCount = 0;
    //////form gloab
    U32 ct_freq_actual = 0;
    U8 same_freq = 0;
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
    //_mt_fe_dmd_cs8k_sat_bs_process_get_range_list(handle, begin_freq_MHz, end_freq_MHz - 2, F_range_list, &fre_seg_idx);
    //clock5 = clock();
    //mt_fe_print(("get strength time %d\n", clock5 - clock4));

    fre_seg_idx = 1;
    F_range_list[0] = begin_freq_MHz;
    F_range_list[1] = end_freq_MHz;
///////////////////////////////////
#if 0
	for(i=0;i<fre_seg_idx;i++)
	{
#endif
    ct_freq_start = F_range_list[2 * i];   //+2;
    ct_freq_end = F_range_list[2 * i + 1]; //+2;
    flag_scan_done = 0;
    ct_freq = ct_freq_start * 1000;
    ct_freq_actual = ct_freq;
    first_psd_flag = 1;
    cnt_rep_psd = 0;
    SymRate_last = -30; //%ChTb(CHID-1,2);
    CtFreq_last = 0;    //%ChTb(CHID-1,1
    idx_dw_last = -100 * (FFT_N - 2 * OVLP);
    K_thr = K_thr1;
    //mt_fe_print(("start  %d  End %d\n", ct_freq_start, ct_freq_end));
    //mt_fe_print(("Range %d / %d, Start %6d;  End %6d\n", i + 1, fre_seg_idx, (U32)ct_freq_start, (U32)ct_freq_end));

    //while ((ct_freq<=ct_freq_end)&&(!handle->global_cfg.bCancelBs))
    while ((flag_scan_done == 0) && (!handle->global_cfg.bCancelBs)) {
//  %%%%%%%%%%%%%%%%% FFT %%%%%%%%%%%%%%%%%%%%
/*  [psd,psd_fft] = load_psd(ct_freq,PSD_SRC,FFT_N,OVLP,SM_WIN,tuner_offset);*/
//   %%%%%%%%%%%%% Initialization before every window PSD    %%%%%%%%%

#if FIND_DEBUG
	FILE *fpOut;
	S32 gain_val = 0;
	S8 iStrength = 0;
	char file_name[100];

	U32 nSize = (FFT_N - 16 * 2) * 2; // sm_buf = 2, overlap = 16, nSize = 968
	U8 buffer[968] = { 0 };           // sm = 2;
	//U32		dwPSDArray[484];						// 484 / 2
	//			U32		dwPSDArray[FFT_N+10];
	U32 i = 0, j = 0;
	U32 dwTmp = 0;
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
//   mt_fe_dmd_rs6k_global_reset(handle);
//   mt_fe_tn_init_rs6000(handle);
//clock4 = clock();

#if FIND_DEBUG
#else
	_mt_fe_dmd_cs8k_sat_bs_set_reg_psd(handle);
#endif

	handle->tp_cfg.iFreqKHz = (freq_KHz + 500) / 1000 * 1000;
	handle->tp_cfg.iSymRateKSs = BLINDSCAN_SYMRATEKSs;

//_mt_fe_dmd_rs6k_select_mclk(handle, handle->tp_cfg.iFreqKHz / 1000);
#if FIND_DEBUG
#else
	if (handle->lnb_cfg.bUnicable) {
	    mt_fe_cs8k_sat_unicable_set_tuner(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, &real_freq_KHz, handle->lnb_cfg.iUBIndex, handle->lnb_cfg.iBankIndex, handle->lnb_cfg.iUBFreqMHz);
	    freq_KHz = real_freq_KHz;
	} else {
	    handle->tuner_cfg.tuner_set(handle, freq_KHz, BLINDSCAN_SYMRATEKSs, 0);
	}

	handle->tuner_cfg.tuner_get_offset(handle, &iTunerOffsetKHz);
	_mt_fe_dmd_cs8k_sat_set_carrier_offset(handle, iTunerOffsetKHz);

	//clock5 = clock();
	//mt_fe_print(("tuner time %dms\n", clock5 - clock4));
	//tmp_freq_KHz = 0;
	//TP_found_flag = 0;
	//clock4 = clock();
	ret = _mt_fe_dmd_cs8k_sat_bs_process_scanned_TP_test(handle, psd, freq_KHz, bs_times, &sig_stre);
#endif

#if FIND_DEBUG
	sprintf(file_name, "PSD_%s_%s_%d.txt", handle->lnb_cfg.b22K ? "H" : "L", (
	                                                                             handle->lnb_cfg.mLnbVoltage == MtFeLNB_18V)
	                                                                             ? "H"
	                                                                             : "V",
	        handle->tp_cfg.iFreqKHz / 1000);

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
		//fgets(str, 100, fpOut);
		strmcp(str1, str, 9);

		//dwTmp = atoi(str1);
		//sig_stre = dwTmp;

		dwTmp = FFT_N - 482 + 4;

		for (i = 0; i < dwTmp; i++) {
		    psd[nCount++] = 0;
		}

		while (!feof(fpOut)) {
		    fscanf(fpOut, "%d\n", &dwTmp);
		    psd[nCount++] = dwTmp;
		}
		fclose(fpOut);
	}

#if 1
	dwTmp = FFT_N - 482 + 4;
	for (i = dwTmp + 1; i < nCount; i++) {
	    diff[i] = psd[i + 1] - psd[i];
	}

	for (i = dwTmp + 1; i < nCount - 1;) {
	    diff[i] = diff[i + 1] - diff[i];
	    if (diff[i] < (0 - spur_thr * 2)) {
		if ((i > (dwTmp + 2)) && (i < (nCount - 4))) {
		    tone_idx++;
		    diff[i] = 1;
		} else
		    diff[i] = 0;
	    } else
		diff[i] = 0;

	    i++;
	}

	if (tone_idx > 0) {
	    for (i = dwTmp + 1; i < nCount - 2; i++) {
		if (diff[i] == 1) ////cha 1
		{
		    if (psd[i + 4] > psd[i - 2]) {
			mean_tone = psd[i + 4] - psd[i - 2];
			for (j = 0; j < 5; j++)
			    psd[i - 1 + j] = (mean_tone * (j + 1) + 3) / 6 + psd[i - 2];
			mean_tone = 0;
		    } else {
			mean_tone = psd[i - 2] - psd[i + 4];
			for (j = 0; j < 5; j++)
			    psd[i - 1 + j] = psd[i - 2] - (mean_tone * (j + 1) + 3) / 6;
			mean_tone = 0;
		    }
		}
	    }
	}
#endif

	nCount = nCount - 1;
	for (j = nCount; j >= 1; j--) {
	    dwTmp = 0;
	    for (i = 0; i < SM_WIN; i++) {
		dwTmp = dwTmp + psd[j - i];
	    }

	    psd[j] = (dwTmp + SM_WIN / 2) / SM_WIN;
	}

	j = FFT_N - 2 * OVLP;
	if ((nCount) > j) {
	    if (tuner_offset > 0)
		dwTmp = OVLP + (SM_WIN + 1) / 2 + tuner_offset;
	    else
		dwTmp = OVLP + (SM_WIN + 1) / 2 - (U8)tuner_offset;
	    for (i = 1; i <= j; i++) {
		psd[i] = psd[i + dwTmp];
	    }
	}
#endif

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
	// wide_TP_flag = 0;

	//clock4 = clock();
	//////////////////////////////////
	//%%%%%%%%%%%%%%%% Search in current PSD %%%%%%%%%%%%%%%%%%%
	//if (idx_notch_last == 1)//% first CH in current PSD, re-locate the idx_notch_last by searching local min position
	if (psd_offset > 9)
	    mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, (psd_offset - 8), (psd_offset + 8));
	else
	    mt_fe_dmd_bs_find_Min(psd, &val, &idx_notch_last, 1, (psd_offset + 8)); /*[val , idx_notch_last] = min(psd(1:FFT_N/32));*/
	while (idx_notch_last < 64) {
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

	while (idx_curr <= FFT_N - 2 * OVLP) {
	    //     %             switch mode_sel
	    //       %                 case 'find_next_notch'
	    //      %                             thr_notch_tureTP = ch_noise_lev*K1;% Effective notch threshold
	    if (psd[idx_curr] >= ch_max) //%%%% update the max of the PSD in the range of next possible ch
	    {
		ch_max = psd[idx_curr];
		idx_ch_max = idx_curr;
	    } else if (psd[idx_curr] <= ch_min) //%%%% update the min of the PSD in the range of next possible ch
	    {
		ch_min = psd[idx_curr];
		idx_ch_min = idx_curr;
	    }

	    //  %------------------- searching the next notch --------------------------------
	    if ((idx_curr > idx_ch_max) || (cnt_notch > 0)) //% after a  first rising edge,during a falling edge OR
	    {
		if (psd[idx_curr] <= psd[idx_notch]) // % update the notch point
		{
		    idx_notch = idx_curr;
		    cnt_notch = 0;
		} else if (psd[idx_curr] > psd[idx_notch]) // % if notch point stays, then  cnt+
		{
		    cnt_notch = cnt_notch + 1; //SEARCH_STEP;

		    if (idx_curr >= FFT_N - 2 * OVLP) //%if a notch has been found near the right hand bound of PSD
			cnt_notch = ris_point_lim;
		}

		if (cnt_notch == ris_point_lim) //%&& idx_notch-idx_notch_last > 8)//%% if notch point stays long enough
		{
		    if (idx_notch_last > idx_notch) {
			//disp('Opps');
		    }
		    mt_fe_dmd_bs_find_Max(psd, &ch_max, &idx_ch_max, idx_notch_last, idx_notch);
		    /*[ch_max idx_ch_max]= max(psd(idx_notch_last:idx_notch));*/ //% find the peak in current searched range
		                                                                 //[ch_min idx_ch_min]= min(psd(idx_notch_last:idx_notch));% find the min in current searched range
		    mt_fe_dmd_bs_find_Min(psd, &ch_min, &idx_ch_min, idx_notch_last, idx_notch);
		    idx_ch_max = idx_ch_max + idx_notch_last - 1; //% to align the idx with psd start
		    idx_ch_min = idx_ch_min + idx_notch_last - 1;
		    //ch_noise_lev = min([ch_min psd(idx_notch) ch_noise_lev]);
		    if (psd[idx_notch] >= ch_min) {
			if (ch_noise_lev > ch_min)
			    ch_noise_lev = ch_min;
		    } else {
			if (ch_noise_lev > psd[idx_notch])
			    ch_noise_lev = psd[idx_notch];
		    }

		    if (psd[idx_notch] < ch_noise_lev) {
			ch_noise_lev = psd[idx_notch];
		    }

		    //	idx_notch_last_tmp = idx_notch_last;

		    sig_level = 0;
		    sig_idx = 0;

		    for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++) {
			sig_level = sig_level + psd[idx_tmp];
			sig_idx = sig_idx + 1;
		    }

		    if (sig_idx != 0)
			sig_thr = sig_level / sig_idx;
		    else {
			mt_fe_print(("\tmt_fe_dmd_cs8k_sat_bs_A_test_2 ---- sig_idx = 0!\n"));
		    }

		    sig_level = 0;
		    sig_idx = 0;

		    for (idx_tmp = idx_notch_last; idx_tmp <= idx_notch; idx_tmp++) {
			if (psd[idx_tmp] >= sig_thr) {
			    sig_level = sig_level + psd[idx_tmp];
			    sig_idx = sig_idx + 1;
			}
		    }

		    if (sig_idx != 0)
			ave_peak = sig_level / sig_idx;
		    else {
			mt_fe_print(("\tmt_fe_dmd_cs8k_sat_bs_A_test_1 ---- sig_idx = 0!\n"));
		    }

		    thr_notch_tureTP = (ch_noise_lev * K_thr + ave_peak * (1000 - K_thr) + 500) / 1000; //% once a notch found, update thr_notch_tureTP

		    if (((psd[idx_notch] < thr_notch_tureTP) && (ch_max > (thr_notch_tureTP))) || (first_psd_flag == 1)) {
			//       % if notch level is low enough and its peak is high enough
			//      % or the currrent search is the very first search attemp of current satellite, than record the notch as an effective notch
			//       %======== Found a CH ============
			K_thr = K_thr1;
			if (first_psd_flag == 0) // % if an effective CH found, just measure it (always skip the first found ch in every start of Blind scan)
			{
			    if ((4000 * idx_notch - 4000 * idx_notch_last) > (SR_min * 5 * FFT_N / Fs)) //% filter the small notch
			    {
				mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd, psd_SN);
			    } else {
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
		    } else //%if the peak is lower than thr,implies it is a fake ch, just skip it
		    {
			if (ch_max < thr_notch_tureTP) //% if the peak is lower than thr, implies that the notch is lower than thr also,  the found CH is uneffecitve,record it and skip
			{
			    //ch_type = Noise_CH;
			    //plot_notch_search;
			    if (psd[idx_notch] < psd[idx_notch_last]) // % if the notch is lower than the last notch,  then record the notch as an effective notch too
			    {
				idx_notch_last = idx_notch;
				idx_start = idx_notch;
			    }

			    ch_min = psd[idx_notch];
			    idx_ch_min = idx_notch;
			    ch_max = psd[idx_notch];
			    idx_ch_max = idx_notch;
			} else // if (psd[idx_notch] >= thr_notch_tureTP)// % the notch is not low enough, implies the CH is still in the rising stage, keep the idx_notch_last unchange
			{
			    if ((1000 * psd[idx_notch] > (707 * thr_notch_tureTP)) && (((1000 - 707) * ave_peak) > (1000 * psd[idx_notch] - 707 * thr_notch_tureTP)))
			        //if ((ave_peak - psd[idx_notch])/((ave_peak  - thr_notch_tureTP)) > 0.707)
			    {
				//  % if the peak is relative high enough, then ture TP decision is depend on if the
				//   % right neibough zone has another loewr notch
				if (idx_notch >= idx_notch_last)
				    verf_range = (U32)(2 * (idx_notch - idx_notch_last) / 5 + 1) / 2;
				else
				    verf_range = 0;

				if ((idx_notch + verf_range) <= (FFT_N - 2 * OVLP)) {
				    mt_fe_dmd_bs_find_Min(psd, &ext_min, &idx_ext_min, (idx_notch + 1), (idx_notch + verf_range));
				    mt_fe_dmd_bs_find_Max(psd, &ext_max, &idx_ext_max, (idx_notch + 1), (idx_notch + verf_range));
				    //[ext_min idx_ext_min] = min(psd(idx_notch+[1:verf_range]));
				    //[ext_max idx_ext_max] = max(psd(idx_notch+[1:verf_range]));
				    if ((idx_ext_min > idx_ext_max)) //  && ((ext_max - psd[idx_notch])< (psd[idx_notch]-ext_min)))
				    {                                //  % if a lower notch found in the
					                             //   % right hand zone, go on finding
					                             //  % next notch without skipping
					                             //   % current notch
					                             //ch_type = Split_CH;
					                             //plot_notch_search;
					                             //disp(' ');
				    } else {
					//       %if no lower notch found in the right hand zone
					mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd, psd_SN);
					//	ch_type = True_CH;
					//	mt_fe_print(("true_ch2,index=%d\n",idx_notch));
					//plot_notch_search;
					idx_notch_last = idx_notch;
					idx_start = idx_notch;
				    }
				} else //%idx_notch+verf_range > (FFT_N-2*OVLP)
				{
				    //ch_type = Split_CH;
				    //plot_notch_search;
				    //disp(' ');
				}
			    } else //%(ch_max - psd(idx_notch))/((ch_max  - thr_notch_tureTP)) < 0.707, implies the peak might not high enough
			    {
				//ch_type = Split_CH;
				//plot_notch_search;
				if (psd[idx_start] < thr_notch_tureTP) //% when a rapid rising edge occurs(means left rising edge has a sharper slope than falling edge)
				{
				    idx_notch_last = idx_start; //% Keep the idx_notch_last
				}

				idx_start = idx_notch;
				//  % disp('Split');
			    }
			    ch_min = psd[idx_notch]; //%???
			    idx_ch_min = idx_notch;  //%???
			    ch_max = psd[idx_notch]; //%??? initial the ch_max for next notch search process
			    idx_ch_max = idx_notch;  //%???
			}
		    }

		    cnt_notch = 0;        //% clear the counter after decision of a notch
		    idx_curr = idx_notch; //% move idx_curr forward to currnet found notch
		}
	    } else //% if idx_curr is sitll in a rising edge
	    {
		idx_notch = idx_curr;
	    }

	    //   %increase the idx_curr by SEARCH_STEP points
	    idx_curr = idx_curr + SEARCH_STEP;
	    //   % if PSD ended
	    if (idx_curr > FFT_N - 2 * OVLP) {
		psd_SN = psd_SN + 1;
		mt_fe_dmd_bs_find_Max(psd, &test_value, &test_index, 1, nCount);
		//if (max(psd) < ch_noise_lev*K1)// % no ch high enough through out the whole PSD
		if (test_value * 10 < ch_noise_lev * K1) // % no ch high enough through out the whole PSD
		{
		    idx_psd_shift = FFT_N - 2 * OVLP; //% jump the whole PSD to begin with next search
		    K1 = 12;
		    K_thr = K_thr1;
		} else //%if max(psd) >= thr_notch_tureTP
		{
		    if (idx_notch_last == idx_psd_start) // % no effective notch but peak is high enough,implies thr may be too low
		    {
			if (cnt_rep_psd < 1) {
			    idx_psd_shift = 1;
			    mt_fe_dmd_bs_find_Max(psd, &test_value, &test_index, 1, nCount);

			    if (test_value >= 50) {
				if ((100 * test_value - 5000) > (U32)(12 * K1))
				    K1 = 12 * K1 / 10;
				else
				    K1 = 10 * test_value - 500;
			    } else
				K1 = 12 * K1 / 10;

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
			} else //% cnt_rep_psd == 1
			{
			    cnt_rep_psd = 0;
			    idx_psd_shift = (FFT_N * FORCE_TUNER_SKIP + Fs / 2) / Fs; //% skip the whole psd
			    K1 = 12;                                                  //% recovery the original K1 thr
			    K_thr = K_thr1;
			}
		    } else //% at least one notch ever found in current PSD
		    {
			if (idx_notch_last > ((FFT_N - 2 * OVLP) / 8)) {
			    if (idx_notch_last_tureCh > (FFT_N - 2 * OVLP) / 2) //% if last found TP fall in the left half of PSD
				idx_psd_shift = idx_notch_last_tureCh;          //-8;//% then shift the PSD according the last TP position to aviod LPF effect
			    else
				idx_psd_shift = idx_notch_last;
			} else {
				/* Tscancode: duplicate branches for 'if' and 'else'. */
			    //if (idx_notch_last > 40) {
				idx_psd_shift = idx_notch_last; //-8;//% make the tuner jump according to last effective notch
				                                //if(idx_notch_last_tureCh>8)
				                                //	idx_psd_shift = idx_notch_last-8;
			    //} else {
				//idx_psd_shift = idx_notch_last;
			    //}
			}
			K1 = 12;
		    }
		}

		if ((psd[FFT_N - 2 * OVLP] < thr_notch_tureTP) && (idx_notch_last == idx_psd_start)) //%if no TP ever found in this PSD
		{
		    //idx_pwr_rg = find(psd>=thr_notch_tureTP);
		    //idx_pwr_rg_start = idx_pwr_rg(1);
		    idx_tmp = 0; // idx_pwr_rg_end = idx_pwr_rg(length(idx_pwr_rg));

		    for (j = 1; j <= FFT_N - 2 * OVLP; j++) {
			if (psd[j] >= thr_notch_tureTP) {
			    idx_tmp++;
			    idx_pwr_rg_end = j;
			}

			if (idx_tmp == 1)
			    idx_pwr_rg_start = j;
		    }

		    if (idx_tmp > 0) {
			if (idx_pwr_rg_start < ((FFT_N - 2 * OVLP) / 4)) {
			    idx_notch_last = 1;
			    if ((idx_pwr_rg_end + (idx_tmp + 2) / 4) > (FFT_N - 2 * OVLP))
				idx_notch = FFT_N - 2 * OVLP; //mt_fe_dmd_bs_find_Min(psd, &ext_min, &idx_notch,FFT_N-2*OVLP,idx_pwr_rg_end+(idx_tmp+2)/4);//;
			    else
				idx_notch = idx_pwr_rg_end + (idx_tmp + 2) / 4;
			    idx_curr = FFT_N - 2 * OVLP;
			    // wide_TP_flag = 1;
			    mt_fe_dmd_bs_ch_measure(handle, p_bs_info, psd, psd_SN);
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
						freq_edge = ct_freq_actual- Fs*((FFT_N-2*OVLP)/FFT_N)/2;
						ct_freq_tmp = ct_freq+[-4:2:4];
						freq_diff = 10;
						for idx = 1:length(ct_freq_tmp)
							Fs_tmp = FcFs_Tab(find(FcFs_Tab(:,1)==ct_freq_tmp(idx)),2)/1e3;
							freq_edge_tmp(idx) = ct_freq_tmp(idx) - Fs_tmp*((FFT_N-2*OVLP)/FFT_N)/2;
							if freq_edge_tmp(idx) < freq_edge && freq_edge-freq_edge_tmp(idx) < freq_diff
								 Fs = Fs_tmp;
								ct_freq = ct_freq_tmp(idx);
								 freq_diff = freq_edge-freq_edge_tmp(idx);
						end
						end
						ct_freq_actual=ct_freq+freq_diff;
					}
					else*/
		{
		    Fs = 96;
		}

		if (ct_freq_actual > ct_freq) {
		    tuner_offset = (2 * ((ct_freq_actual - ct_freq) * FFT_N / Fs) + 1000) / 2000;
		} else {
		    tuner_offset = (2 * ((S32)(ct_freq_actual - ct_freq) * FFT_N / Fs) - 1000) / 2000;
		}
		if (tuner_offset_en == 0) {
		    ct_freq_actual = ct_freq; //% ct_freq_actual equals to ct_freq
		    psd_offset = (U32)tuner_offset;
		    tuner_offset = 0;
		} else {
		    ct_freq_actual = ct_freq + 1000 * tuner_offset * Fs / FFT_N;
		    psd_offset = 0;
		}

		if (FFT_N > 2 * OVLP) {
		    if (ct_freq >= (1000 * ct_freq_end))
			flag_scan_done = 1;
		} else {
		    if ((ct_freq + (2000 * OVLP - FFT_N) / 2 / FFT_N * Fs) > (1000 * ct_freq_end))
			flag_scan_done = 1;
		    else {
			flag_scan_done = 0;
			if (ct_freq >= (1000 * ct_freq_end))
			    flag_scan_done = 1;
		    }
		}

		if (ct_freq == freq_KHz) {
		    same_freq++;
		    if (same_freq > 3) {
			mt_fe_print(("find same freq %d\n", ct_freq));
			ct_freq = ct_freq + 8000;
		    }
		} else
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

//clock4 = clock();
//mt_fe_print(("find tp time %d\n", clock4 - clock5));

#if FIND_DEBUG
#else
    if (is_connect)
	_mt_fe_dmd_cs8k_sat_bs_connect_test(handle, p_bs_info);
    else
	p_bs_info->tp_num = handle->global_cfg.iScannedTpCnt;
//mt_fe_dmd_cs8k_sat_bs_lock_test(handle, p_bs_info);
//mt_fe_dmd_rs6k_bs_lock_test(handle, p_bs_info);
#endif

    //clock5 = clock();
    //mt_fe_print(("lock  tp time %d\n", clock5 - clock4));
    //mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSFinish, 0);
    //    %---------------------- plot CH map according to the Blind Scan results -------------------
    //    %---------------------- Compare the scan result to reference Channel table --------------
    //Scan_result_stat;
    //CH_mis_fake_rec(rep_times,1:2) = [CH_mis_num cnt_Fake_CH];
    p_bs_info->next_freq_KHz = ct_freq;

    return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_cs8k_sat_blindscan_test(MT_FE_CS8000_SAT_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz,
                                            MT_FE_BS_TP_INFO *p_bs_info, U32 is_connect)
{
    U8 times = 0;

    if ((p_bs_info->p_tp_info == NULL) || (p_bs_info->tp_max_num == 0))
	return MtFeErr_Param;

    mt_fe_dmd_cs8k_sat_global_reset(handle);

    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSStart, 0);

    handle->global_cfg.iScannedTpCnt = 0;
    handle->global_cfg.iLockedTpCnt = 0;
    handle->global_cfg.bCancelBs = FALSE;
    handle->global_cfg.iTotalBsTimes = p_bs_info->bs_times;

    handle->board_cfg.bSpectrumInverted = handle->board_cfg.bIQInverted ^ (handle->lnb_cfg.bUnicable && (handle->lnb_cfg.iUBVer != 2)); // XOR

    p_bs_info->tp_num = 0;

    //for(times = MT_FE_BS_TIMES_1ST; times < (p_bs_info->bs_times); times++)
    //{
    p_bs_info->bNewGain = TRUE;
    p_bs_info->iPreGain = 0;
    p_bs_info->iCurGain = 0;
    //p_bs_info->bs_cur_loop = times;

    //if(p_bs_info->bs_algorithm == MT_FE_BS_ALGORITHM_A)
    //{
    _mt_fe_dmd_cs8k_sat_bs_A_test(handle, begin_freq_MHz, end_freq_MHz, p_bs_info, times, is_connect);
    //}
    //else if(p_bs_info->bs_algorithm == MT_FE_BS_ALGORITHM_B)
    //{
    //	_mt_fe_dmd_cs8k_sat_bs_B(handle, begin_freq_MHz, end_freq_MHz, p_bs_info, times);
    //}
    //else
    //{
    //	mt_fe_cs8k_sat_bs_notify(MtFeMsg_BSFinish, 0);
    //	return MtFeErr_Fail;
    //}

    //if(handle->global_cfg.bCancelBs)
    //	{
    //handle->global_cfg.bCancelBs = FALSE;
    //	break;
    //}
    //}

    handle->mt_sleep(1000);

    _mt_fe_dmd_cs8k_sat_bs_remove_unlocked_TP(handle, p_bs_info, (U32)FREQ_MAX_KHz);

    mt_fe_cs8k_sat_bs_notify(handle, MtFeMsg_BSFinish, 0);

    return MtFeErr_Ok;
}
