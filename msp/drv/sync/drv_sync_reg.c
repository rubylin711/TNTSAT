/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : avsync_symphony_reg.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/12/23
 * Description    : AVSync FW Registers, includes PTS/PCR/STC, and AOUT.
 * History        :
 * 1.Date         : 2019/12/23
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_error_mpi.h"
#include "mt_type.h"
#include "drv_sync_reg.h"

mt_u32 symphony_pcr_sel_cfg(mt_u32 ch)
{
	u32 value;

    if (ch > 7)
    {
        return MT_FAILURE;
    }

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_PCR_CFG);
    value &= 0xfffff0ff;
    value |= (ch << 8);
    value |= (1 << 12);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_PCR_CFG, value);

    return MT_SUCCESS;
}

void symphony_pcr_enable(void)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_CNT_CFG);

	value |= (0x01 << BIT_PCR_CNT_EN);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_CNT_CFG, value);
}

void symphony_pcr_disable(void)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_CNT_CFG);

	value &= ~(0x01 << BIT_PCR_CNT_EN);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_CNT_CFG, value);
}

void symphony_pcr_reload(void)
{
	u32 value;

	value = (0x01 << BIT_PCR_CNT_SET);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_STC_SET, value);
}

mt_dvb_pts32 symphony_pcr_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_PCR_CNT_BASE) << 1);
}

void symphony_stc_enable(void)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG);
	value |= (0x01 << BIT_STC_CNT_EN);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG, value);
}

void symphony_stc_disable(void)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG);
	value &= ~(0x01 << BIT_STC_CNT_EN);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG, value);
}

void symphony_stc_reload(void)
{
	u32 value;

	value = (0x01 << BIT_STC_CNT_SET);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_PCR_STC_SET, value);
}

void symphony_stc_pcr_recovery(MT_BOOL enable)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_PCRREC_CFG);

	if (enable)
	{
		value |= (0x01 << BIT_PCRRECOVERY_ENABLE);
	}
	else
	{
		value &= ~(0x01 << BIT_PCRRECOVERY_ENABLE);
	}

	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_PCRREC_CFG, value);
}

mt_dvb_pts32 symphony_stc_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_STC_CNT_BASE) << 1);
}

mt_dvb_pts32 symphony_play_apts_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_LOCAL_APTS) << 1);
}

mt_dvb_pts32 symphony_ts_apts_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_TS_APTS) << 1);
}

void symphony_apts_set(mt_dvb_pts32 apts)
{
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_APTS_CFG, (u32)(apts >> 1));
}

void symphony_apts_add_enable(MT_BOOL enable)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_AUDIO_CFG);

	if (enable)
	{
		value |= (0x01 << BIT_LOCAL_APTS_ADD_EN);
	}
	else
	{
		value &= ~(0x01 << BIT_LOCAL_APTS_ADD_EN);
	}

	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_AUDIO_CFG, value);
}

mt_u32 symphony_audio_sel_cfg(mt_u32 ch)
{
	u32 value;

    if (ch > 15)
    {
        return MT_FAILURE;
    }

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_AUDIO_CFG);
    value &= 0xfffff0ff;
    value |= (ch << 8);
    value |= (1 << 12);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_AUDIO_CFG, value);

    return MT_SUCCESS;
}

mt_dvb_pts32 symphony_play_vpts_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_LOCAL_VPTS) << 1);
}

mt_dvb_pts32 symphony_ts_vpts_get(void)
{
	return (mt_dvb_pts32)(HAL_GET_U32((volatile u32 *)REG_AVSYNC_TS_VPTS) << 1);
}

void symphony_vpts_set(mt_dvb_pts32 vpts)
{
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_VPTS_CFG, (u32)(vpts >> 1));
}

void symphony_vpts_add_enable(MT_BOOL enable)
{
	u32 value;

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_VIDEO_CFG);

	if (enable)
	{
		value |= (0x01 << BIT_LOCAL_VPTS_ADD_EN);
	}
	else
	{
		value &= ~(0x01 << BIT_LOCAL_VPTS_ADD_EN);
	}

	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_VIDEO_CFG, value);
}

mt_u32 symphony_video_sel_cfg(mt_u32 ch)
{
	u32 value;

    if (ch > 15)
    {
        return MT_FAILURE;
    }

	value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_VIDEO_CFG);
    value &= 0xfffff0ff;
    value |= (ch << 8);
    value |= (1 << 12);
	HAL_PUT_U32((volatile u32 *)REG_AVSYNC_VIDEO_CFG, value);

    return MT_SUCCESS;
}

mt_u32 symphony_avsync_set_stc_cnt_load_mode(mt_u32 mode)
{
    mt_u32 value = 0;

    if ((mode != 0) && (mode != 1) && (mode != 2) && (mode != 8))
    {
        return MT_ERR_SYNC_INVALID_PARA;
    }

    value = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG);
    value &= 0xfffffff0;
    value |= mode;
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_CFG, value);

    return MT_SUCCESS;
}

mt_u32 symhony_avsync_set_stc_cnt_ini_base_value(mt_u32 value)
{
    mt_u32 stc = 0;

    /*
     * The description of the FAKE_STC_CNT_INI_0 bit and FAKE_STC_CNT_INI_1
     * in AVsync HW spec is wrong. Correct configuration: the low 22 bits of
     * stc set to high 22 bits of REG_AVSYNC_FAKE_STC_CNT_INI_0, the high
     * 10 bits of stc set to low 10 bits of REG_AVSYNC_FAKE_STC_CNT_INI_0
     * and REG_AVSYNC_FAKE_STC_CNT_INI_0
    */
    stc = (value << 10);
    stc += (value >> 22);
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_INI_0, stc);

    stc = HAL_GET_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_INI_1);
    stc &= 0xffffc00;
    stc |= ((value >> 22) & 0x3ff);
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_FAKE_STC_CNT_INI_1, stc);
    return MT_SUCCESS;
}

mt_u32 symphony_avsync_irq_enable(mt_u32 offset)
{
    mt_u32 int_en = 0;

    if (offset >= 32)
    {
        return MT_FAILURE;
    }

    int_en = HAL_GET_U32((volatile u32 *)REG_AVSYNC_INT_ENABLE);
    int_en |= (1 << offset);
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_INT_ENABLE, int_en);

    return MT_SUCCESS;
}

mt_u32 symphony_avsync_irq_disable(mt_u32 offset)
{
    mt_u32 int_en = 0;

    if (offset >= 32)
    {
        return MT_FAILURE;
    }

    int_en = HAL_GET_U32((volatile u32 *)REG_AVSYNC_INT_ENABLE);
    int_en &= ~(1 << offset);
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_INT_ENABLE, int_en);

    return MT_SUCCESS;
}

mt_u32 symphony_avsync_get_irq_cfg(void)
{
    return  HAL_GET_U32((volatile u32 *)REG_AVSYNC_INT_ENABLE);
}

mt_u32 symphony_avsync_get_int_sta(void)
{
    mt_u32 int_sta = 0;

    int_sta = HAL_GET_U32((volatile u32 *)REG_AVSYNC_INT_STATE);
    return int_sta;
}

mt_u32 symphony_avsync_clr_int_sta(mt_u32 val)
{
    HAL_PUT_U32((volatile u32 *)REG_AVSYNC_INT_STATE, val);
    return MT_SUCCESS;
}


