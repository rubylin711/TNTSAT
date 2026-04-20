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
#ifndef __DRV_SYNC_REG_H__
#define __DRV_SYNC_REG_H__

ulong mt_get_avsync_base(void);
/*
 * 0xBF4A0000
 */
#define REG_AVSYNC_BASE					mt_get_avsync_base()
#define REG_AVSYNC_PCRREC_CFG			(REG_AVSYNC_BASE + 0x00)
#define BIT_PCRRECOVERY_ENABLE			0	/* STC PCR RECOVERY使能 */
#define REG_AVSYNC_VIDEO_CFG			(REG_AVSYNC_BASE + 0x100)
#define BIT_LOCAL_VPTS_ADD_EN			2	/* 本地视频输出VPTS累加使能位 */
#define REG_AVSYNC_AUDIO_CFG			(REG_AVSYNC_BASE + 0x104)
#define BIT_LOCAL_APTS_ADD_EN			2	/* 本地音频输出APTS累加使能位 */
#define REG_AVSYNC_PCR_CFG              (REG_AVSYNC_BASE + 0x110)
#define REG_AVSYNC_VPTS_CFG				(REG_AVSYNC_BASE + 0x120)
#define REG_AVSYNC_APTS_CFG				(REG_AVSYNC_BASE + 0x124)
#define REG_AVSYNC_PCR_CNT_BASE			(REG_AVSYNC_BASE + 0x140)
#define REG_AVSYNC_PCR_CNT_EXT			(REG_AVSYNC_BASE + 0x144)
#define REG_AVSYNC_STC_CNT_BASE			(REG_AVSYNC_BASE + 0x148)
#define REG_AVSYNC_STC_CNT_EXT			(REG_AVSYNC_BASE + 0x14c)
#define REG_AVSYNC_TS_VPTS			    (REG_AVSYNC_BASE + 0x150)
#define REG_AVSYNC_TS_APTS			    (REG_AVSYNC_BASE + 0x158)
#define REG_AVSYNC_LOCAL_VPTS			(REG_AVSYNC_BASE + 0x160)
#define REG_AVSYNC_LOCAL_APTS			(REG_AVSYNC_BASE + 0x164)

#define REG_AVSYNC_INT_ENABLE           (REG_AVSYNC_BASE + 0x1f4)
#define REG_AVSYNC_INT_STATE            (REG_AVSYNC_BASE + 0x1fc)
#define BIT_PCR_INT_EN				    (0)
#define BIT_PCR_INT_STA				    (0)
#define BIT_VPTS_INT_EN				    (1)
#define BIT_VPTS_INT_STA				(1)
#define BIT_APTS_INT_EN				    (3)
#define BIT_APTS_INT_STA				(3)
#define BIT_AUD_ONE_FRM_OUT_INT_EN      (12)
#define BIT_AUD_ONE_FRM_OUT_INT_STA		(12)

#define REG_AVSYNC_FAKE_PCR_STC_SET		(REG_AVSYNC_BASE + 0x300)
#define BIT_STC_CNT_SET					4	/* STC计数器加载模式使能 */
#define BIT_PCR_CNT_SET					1	/* PCR计数器加载模式使能 */
#define REG_AVSYNC_FAKE_PCR_CNT_CFG		(REG_AVSYNC_BASE + 0x304)
#define BIT_PCR_CNT_EN					8	/* PCR计数器计数使能 */
#define REG_AVSYNC_FAKE_STC_CNT_CFG		(REG_AVSYNC_BASE + 0x308)
#define BIT_STC_CNT_EN					8	/* STC计数器计数使能 */
#define REG_AVSYNC_FAKE_PCR_STC_TEST	(REG_AVSYNC_BASE + 0x30c)
#define REG_AVSYNC_FAKE_STC_CNT_INI_0   (REG_AVSYNC_BASE + 0x318)
#define REG_AVSYNC_FAKE_STC_CNT_INI_1   (REG_AVSYNC_BASE + 0x31c)

void symphony_pcr_enable(void);
void symphony_pcr_disable(void);
void symphony_pcr_reload(void);
mt_dvb_pts32 symphony_pcr_get(void);
void symphony_stc_enable(void);
void symphony_stc_disable(void);
void symphony_stc_reload(void);
void symphony_stc_pcr_recovery(MT_BOOL enable);
mt_dvb_pts32 symphony_stc_get(void);
mt_dvb_pts32 symphony_play_apts_get(void);
mt_dvb_pts32 symphony_ts_apts_get(void);
void symphony_apts_set(mt_dvb_pts32 apts);
void symphony_apts_add_enable(MT_BOOL enable);
mt_dvb_pts32 symphony_play_vpts_get(void);
mt_dvb_pts32 symphony_ts_vpts_get(void);
void symphony_vpts_set(mt_dvb_pts32 vpts);
void symphony_vpts_add_enable(MT_BOOL enable);
mt_u32 symphony_avsync_set_stc_cnt_load_mode(mt_u32 mode);
mt_u32 symhony_avsync_set_stc_cnt_ini_base_value(mt_u32 value);
mt_u32 symphony_avsync_irq_enable(mt_u32 offset);
mt_u32 symphony_avsync_irq_disable(mt_u32 offset);
mt_u32 symphony_avsync_get_int_sta(void);
mt_u32 symphony_avsync_clr_int_sta(mt_u32 offset);
mt_u32 symphony_audio_sel_cfg(mt_u32 ch);
mt_u32 symphony_video_sel_cfg(mt_u32 ch);
mt_u32 symphony_pcr_sel_cfg(mt_u32 ch);
mt_u32 symphony_avsync_get_irq_cfg(void);

#endif

