/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_reg.c <2022-10-31>																*/
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <uapi/linux/sched/types.h>
#include "mt_drv_mmz.h"
#include "mt_common.h"
#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "mt_drv_module.h"
#include "drv_snd_reg.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

/*****************************************************************************
[GLOBAL]
******************************************************************************/
ulong reg_aout_base_ioremap = 0;
ulong reg_aout_clk_base_ioremap = 0;
ulong reg_hdmi_audio_ctrl_ioremap = 0;

/*****************************************************************************
[FUNCTION]
******************************************************************************/
mt_u32 drv_snd_reg_rd(mt_u32 p_addr)
{
	if(0 != reg_aout_base_ioremap)
		return HAL_GET_U32((volatile u32 *)(reg_aout_base_ioremap+p_addr));
	else
		return 0;
}
mt_void drv_snd_reg_wt(mt_u32 p_addr, mt_u32 data)
{
	if(0 != reg_aout_base_ioremap)
		HAL_PUT_U32((volatile u32 *)(reg_aout_base_ioremap+p_addr), data);
}
		
mt_u32 drv_aout_clk_reg_rd(mt_u32 p_addr)
{
	if(0 != reg_aout_clk_base_ioremap)
		return HAL_GET_U32((volatile u32 *)(reg_aout_clk_base_ioremap+p_addr));
	else
		return 0;
}
mt_void drv_aout_clk_reg_wt(mt_u32 p_addr, mt_u32 data)
{
	if(0 != reg_aout_clk_base_ioremap)
		HAL_PUT_U32((volatile u32 *)(reg_aout_clk_base_ioremap+p_addr), data);
}

#define DRV_SND_REG_GET(structType, bitName, addr)	\
		do{	\
			structType reg;		\
			reg.all = drv_snd_reg_rd(addr);	\
			return reg.bit.bitName;	\
		}while(0)
		
#define DRV_SND_REG_SET(structType, bitName, addr, data)	\
		do{	\
			structType reg;		\
			reg.all = drv_snd_reg_rd(addr);	\
			reg.bit.bitName = data;	\
			drv_snd_reg_wt(addr, reg.all);	\
		}while(0)

/*****************************************************************************/
//00H
mt_u32 snd_reg_00H_get_sample_rate(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, sample_rate, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_sample_rate(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, sample_rate, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_pcm_32b_flag(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, pcm_32b_flag, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_pcm_32b_flag(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, pcm_32b_flag, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_ch_input_mode(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, ch_input_mode, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_ch_input_mode(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, ch_input_mode, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_spdbuf_sample_rate(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, spdbuf_sample_rate, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_spdbuf_sample_rate(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, spdbuf_sample_rate, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_spdbuf_sample_rate_sel(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, spdbuf_sample_rate_sel, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_spdbuf_sample_rate_sel(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, spdbuf_sample_rate_sel, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_spdbuf_128fs(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, spdbuf_128fs, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_spdbuf_128fs(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, spdbuf_128fs, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_pcmbuf_sample_rate(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, pcmbuf_sample_rate, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_pcmbuf_sample_rate(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, pcmbuf_sample_rate, REG_AUD_CH_SRT_CFG, data);
}
mt_u32 snd_reg_00H_get_pcmbuf_sample_rate_sel(void)
{
	DRV_SND_REG_GET(stAUD_CH_SRT_CFG, pcmbuf_sample_rate_sel, REG_AUD_CH_SRT_CFG);
}
void snd_reg_00H_set_pcmbuf_sample_rate_sel(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CH_SRT_CFG, pcmbuf_sample_rate_sel, REG_AUD_CH_SRT_CFG, data);
}

//04H
mt_u32 snd_reg_04H_get_right_flag(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, right_flag, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_right_flag(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, right_flag, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_justified_mode(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, justified_mode, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_justified_mode(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, justified_mode, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_hdmi_manu_mute(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, hdmi_manu_mute, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_hdmi_manu_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, hdmi_manu_mute, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_i2s_ch_sel(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, i2s_ch_sel, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_i2s_ch_sel(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, i2s_ch_sel, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_spdif_mixbuf(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, spdif_mixbuf, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_spdif_mixbuf(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, spdif_mixbuf, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_spdif_audout_buf(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, spdif_audout_buf, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_spdif_audout_buf(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, spdif_audout_buf, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_spdif_spd_buf(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, spdif_spd_buf, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_spdif_spd_buf(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, spdif_spd_buf, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_spdif_path_sel_spdif(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, spdif_path_sel_spdif, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_spdif_path_sel_spdif(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, spdif_path_sel_spdif, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_spdif_path_sel_hdmi(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, spdif_path_sel_hdmi, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_spdif_path_sel_hdmi(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, spdif_path_sel_hdmi, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_adac_ch_sel(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, adac_ch_sel, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_adac_ch_sel(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, adac_ch_sel, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_adac_manu_mute(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, adac_manu_mute, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_adac_manu_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, adac_manu_mute, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_adac_valid_flag_cnt(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, adac_valid_flag_cnt, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_adac_valid_flag_cnt(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, adac_valid_flag_cnt, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_new_adac_valid_flag(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, new_adac_valid_flag, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_new_adac_valid_flag(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, new_adac_valid_flag, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_pcm_valid_cnt(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, pcm_valid_cnt, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_pcm_valid_cnt(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, pcm_valid_cnt, REG_AUD_I2S_SPDIF_CFG, data);
}
mt_u32 snd_reg_04H_get_adac_pcm_decim2(void)
{
	DRV_SND_REG_GET(stAUD_I2S_SPDIF_CFG, adac_pcm_decim2, REG_AUD_I2S_SPDIF_CFG);
}
void snd_reg_04H_set_adac_pcm_decim2(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_I2S_SPDIF_CFG, adac_pcm_decim2, REG_AUD_I2S_SPDIF_CFG, data);
}

//08H
mt_u32 snd_reg_08H_get_volume_scale(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, volume_scale, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_volume_scale(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, volume_scale, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_gainq(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, gainq, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_gainq(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, gainq, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_gainq_manu_zero(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, gainq_manu_zero, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_gainq_manu_zero(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, gainq_manu_zero, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_i2s_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, i2s_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_i2s_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, i2s_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_spdif_hdmi_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, spdif_hdmi_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_spdif_hdmi_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, spdif_hdmi_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_spdif_coax_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, spdif_coax_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_spdif_coax_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, spdif_coax_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_adac_2ch_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, adac_2ch_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_adac_2ch_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, adac_2ch_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_left_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, left_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_left_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, left_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_right_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, right_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_right_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, right_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_bass_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, bass_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_bass_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, bass_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_center_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, center_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_center_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, center_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_sl_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, sl_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_sl_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, sl_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_sr_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, sr_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_sr_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, sr_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_rsl_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, rsl_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_rsl_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, rsl_mute, REG_AUD_VOL_CFG, data);
}
mt_u32 snd_reg_08H_get_rsr_mute(void)
{
	DRV_SND_REG_GET(stAUD_VOL_CFG, rsr_mute, REG_AUD_VOL_CFG);
}
void snd_reg_08H_set_rsr_mute(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_VOL_CFG, rsr_mute, REG_AUD_VOL_CFG, data);
}

//0CH
mt_u32 snd_reg_0CH_get_src_en(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, src_en, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_src_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, src_en, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_downmix_en(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, downmix_en, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_downmix_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, downmix_en, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_agc_en(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, agc_en, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_agc_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, agc_en, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_chan_mod_pcmbuf(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, chan_mod_pcmbuf, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_chan_mod_pcmbuf(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, chan_mod_pcmbuf, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_ahbdata_en(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, ahbdata_en, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_ahbdata_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, ahbdata_en, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_old_src_flag(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, old_src_flag, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_old_src_flag(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, old_src_flag, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_dmx_hdmi_en(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, dmx_hdmi_en, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_dmx_hdmi_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, dmx_hdmi_en, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_dmx_manu_close(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, dmx_manu_close, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_dmx_manu_close(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, dmx_manu_close, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_chmode_manu_2ch(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, chmode_manu_2ch, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_chmode_manu_2ch(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, chmode_manu_2ch, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_audio_lr_copy(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, audio_lr_copy, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_audio_lr_copy(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, audio_lr_copy, REG_AUD_PP_EN_CFG, data);
}
mt_u32 snd_reg_0CH_get_adac_audio_lr_copy(void)
{
	DRV_SND_REG_GET(stAUD_PP_EN_CFG, adac_audio_lr_copy, REG_AUD_PP_EN_CFG);
}
void snd_reg_0CH_set_adac_audio_lr_copy(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_EN_CFG, adac_audio_lr_copy, REG_AUD_PP_EN_CFG, data);
}

//10H
mt_u32 snd_reg_10H_get_clk_divider_factor(void)
{
	DRV_SND_REG_GET(stAUD_CLK_DIV_CFG, clk_divider_factor, REG_AUD_CLK_DIV_CFG);
}
void snd_reg_10H_set_clk_divider_factor(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CLK_DIV_CFG, clk_divider_factor, REG_AUD_CLK_DIV_CFG, data);
}

//14H
mt_u32 snd_reg_14H_get_sample_rate_offset(void)
{
	DRV_SND_REG_GET(stAUD_CLK_ADJ, sample_rate_offset, REG_AUD_CLK_ADJ);
}
void snd_reg_14H_set_sample_rate_offset(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CLK_ADJ, sample_rate_offset, REG_AUD_CLK_ADJ, data);
}

//18H
mt_u32 snd_reg_18H_get_audio_out_buf_base_address(void)
{
	DRV_SND_REG_GET(stAUD_OUT_BUF_BASE, audio_out_buf_base_address, REG_AUD_OUT_BUF_BASE);
}
void snd_reg_18H_set_audio_out_buf_base_address(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_OUT_BUF_BASE, audio_out_buf_base_address, REG_AUD_OUT_BUF_BASE, data);
}

//1CH
mt_u32 snd_reg_1CH_get_audio_out_buf_length(void)
{
	DRV_SND_REG_GET(stAUD_OUT_BUF_LEN, audio_out_buf_length, REG_AUD_OUT_BUF_LEN);
}
void snd_reg_1CH_set_audio_out_buf_length(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_OUT_BUF_LEN, audio_out_buf_length, REG_AUD_OUT_BUF_LEN, data);
}

//20H
mt_u32 snd_reg_20H_get_audout_buf_full_thd(void)
{
	DRV_SND_REG_GET(stAUD_OUT_BUF_FUL_THD, audout_buf_full_thd, REG_AUD_OUT_BUF_FUL_THD);
}
void snd_reg_20H_set_audout_buf_full_thd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_OUT_BUF_FUL_THD, audout_buf_full_thd, REG_AUD_OUT_BUF_FUL_THD, data);
}

//24H
mt_u32 snd_reg_24H_get_aud_out_buf_wr_len(void)
{
	DRV_SND_REG_GET(stAUD_OUT_BUF_WRCMD, aud_out_buf_wr_len, REG_AUD_OUT_BUF_WRCMD);
}
void snd_reg_24H_set_aud_out_buf_wr_len(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_OUT_BUF_WRCMD, aud_out_buf_wr_len, REG_AUD_OUT_BUF_WRCMD, data);
}
mt_u32 snd_reg_24H_get_aud_out_buf_wr_cmd(void)
{
	DRV_SND_REG_GET(stAUD_OUT_BUF_WRCMD, aud_out_buf_wr_cmd, REG_AUD_OUT_BUF_WRCMD);
}
void snd_reg_24H_set_aud_out_buf_wr_cmd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_OUT_BUF_WRCMD, aud_out_buf_wr_cmd, REG_AUD_OUT_BUF_WRCMD, data);
}

//28H
mt_u32 snd_reg_28H_get_pp_buf_base(void)
{
	DRV_SND_REG_GET(stAUD_PP_BUF_BASE, ppbuf0_base_address, REG_AUD_PP_BUF_BASE);
}
void snd_reg_28H_set_pp_buf_base(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_BUF_BASE, ppbuf0_base_address, REG_AUD_PP_BUF_BASE, data);
}

//2cH
mt_u32 snd_reg_2cH_get_pp_buf_len(void)
{
	DRV_SND_REG_GET(stAUD_PP_BUF_LEN, one_pp_buffer_length, REG_AUD_PP_BUF_LEN);
}
void snd_reg_2cH_set_pp_buf_len(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_BUF_LEN, one_pp_buffer_length, REG_AUD_PP_BUF_LEN, data);
}

//30H
mt_u32 snd_reg_30H_get_pp_buf_full_thd(void)
{
	DRV_SND_REG_GET(stAUD_PP_BUF_FUL_THD, pp_buf_full_thd, REG_AUD_PP_BUF_FUL_THD);
}
void snd_reg_30H_set_pp_buf_full_thd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PP_BUF_FUL_THD, pp_buf_full_thd, REG_AUD_PP_BUF_FUL_THD, data);
}

//34H
void snd_reg_34H_set_pp_buf_wr_cmd(mt_u32 idx, mt_u32 len)
{
	drv_snd_reg_wt(REG_AUD_PP_BUF_WRCMD, 0x80000000 | (idx << 24) | len);
}


//38H
mt_u32 snd_reg_38H_get_spd_buf_base_address(void)
{
	DRV_SND_REG_GET(stAUD_SPD_BUF_BASE, aud_spd_buf_base, REG_AUD_SPD_BUF_BASE);
}
void snd_reg_38H_set_spd_buf_base_address(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SPD_BUF_BASE, aud_spd_buf_base, REG_AUD_SPD_BUF_BASE, data);
}

//3CH
mt_u32 snd_reg_3CH_get_spd_buf_length(void)
{
	DRV_SND_REG_GET(stAUD_SPD_BUF_LEN, aud_spd_buf_len, REG_AUD_SPD_BUF_LEN);
}
void snd_reg_3CH_set_spd_buf_length(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SPD_BUF_LEN, aud_spd_buf_len, REG_AUD_SPD_BUF_LEN, data);
}

//40H
mt_u32 snd_reg_40H_get_spd_buf_full_thd(void)
{
	DRV_SND_REG_GET(stAUD_SPD_BUF_FUL_THD, spd_buf_full_thd, REG_AUD_SPD_BUF_FUL_THD);
}
void snd_reg_40H_set_spd_buf_full_thd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SPD_BUF_FUL_THD, spd_buf_full_thd, REG_AUD_SPD_BUF_FUL_THD, data);
}

//44H
mt_u32 snd_reg_44H_get_spd_buf_wr_len(void)
{
	DRV_SND_REG_GET(stAUD_SPD_BUF_WRCMD, aud_spdbuf_wr_len, REG_AUD_SPD_BUF_WRCMD);
}
void snd_reg_44H_set_spd_buf_wr_len(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SPD_BUF_WRCMD, aud_spdbuf_wr_len, REG_AUD_SPD_BUF_WRCMD, data);
}
mt_u32 snd_reg_44H_get_spd_buf_wr_cmd(void)
{
	DRV_SND_REG_GET(stAUD_SPD_BUF_WRCMD, aud_spdbuf_wr_cmd, REG_AUD_SPD_BUF_WRCMD);
}
void snd_reg_44H_set_spd_buf_wr_cmd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SPD_BUF_WRCMD, aud_spdbuf_wr_cmd, REG_AUD_SPD_BUF_WRCMD, data);
}

//4CH
mt_u32 snd_reg_4CH_get_src_coef(void)
{
	DRV_SND_REG_GET(stAUD_SRC_CFG, src_coef, REG_AUD_SRC_CFG);
}
void snd_reg_4CH_set_src_coef(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SRC_CFG, src_coef, REG_AUD_SRC_CFG, data);
}
mt_u32 snd_reg_4CH_get_coef_address(void)
{
	DRV_SND_REG_GET(stAUD_SRC_CFG, coef_address, REG_AUD_SRC_CFG);
}
void snd_reg_4CH_set_coef_address(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SRC_CFG, coef_address, REG_AUD_SRC_CFG, data);
}
mt_u32 snd_reg_4CH_get_coef_wr(void)
{
	DRV_SND_REG_GET(stAUD_SRC_CFG, coef_wr, REG_AUD_SRC_CFG);
}
void snd_reg_4CH_set_coef_wr(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_SRC_CFG, coef_wr, REG_AUD_SRC_CFG, data);
}
void snd_reg_5CH_set_fader_en(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_FADER_CFG, fade_en, REG_AUD_FADER_CFG, data);
}
void snd_reg_5CH_set_fader_in(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_FADER_CFG, fade_in, REG_AUD_FADER_CFG, data);
}
void snd_reg_5CH_set_fader_step(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_FADER_CFG, fade_step, REG_AUD_FADER_CFG, data);
}
void snd_reg_5CH_set_fader_target_gain(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_FADER_CFG, target_gain, REG_AUD_FADER_CFG, data);
}
void snd_reg_5CH_set_fader_time_step(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_FADER_CFG, time_step, REG_AUD_FADER_CFG, data);
}

// 60H
void snd_reg_60h_set_coef_left_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG0, data);
}
// 64H
void snd_reg_64h_set_coef_right_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG1, data);
}
// 68H
void snd_reg_68h_set_coef_bass_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG2, data);
}
// 6cH
void snd_reg_6ch_set_coef_center_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG3, data);
}
// 70H
void snd_reg_70h_set_coef_sl_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG4, data);
}
// 74H
void snd_reg_74h_set_coef_sr_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG5, data);
}
// 78H
void snd_reg_78h_set_coef_rsl_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG6, data);
}
// 7cH
void snd_reg_7ch_set_coef_rsr_all(mt_u32 data)
{
	drv_snd_reg_wt(REG_AUD_DMX_CFG7, data);
}

//80H
void snd_reg_80H_reset_pp_aout_buf(void)
{
	DRV_SND_REG_SET(stAUD_SYNC_RESET, audio_sync_reset, REG_AUD_SYNC_RESET, 1);
}

void snd_reg_80H_reset_mix_buf(void)
{
	DRV_SND_REG_SET(stAUD_SYNC_RESET, mixbuf_reset, REG_AUD_SYNC_RESET, 1);
}

void snd_reg_80H_reset_spd_buf(void)
{
	DRV_SND_REG_SET(stAUD_SYNC_RESET, spdbuf_reset, REG_AUD_SYNC_RESET, 1);
}

mt_u32 snd_reg_80H_get_pp_aout_buf_reset_end(void)
{
	DRV_SND_REG_GET(stAUD_SYNC_RESET, ppbuf_clear_end_flag, REG_AUD_SYNC_RESET);
}

mt_u32 snd_reg_80H_get_mix_buf_reset_end(void)
{
	DRV_SND_REG_GET(stAUD_SYNC_RESET, mixbuf_clear_end_flag, REG_AUD_SYNC_RESET);
}

mt_u32 snd_reg_80H_get_spd_buf_reset_end(void)
{
	DRV_SND_REG_GET(stAUD_SYNC_RESET, spdbuf_clear_end_flag, REG_AUD_SYNC_RESET);
}

//88H
mt_u32 snd_reg_88H_get_clk_divider_factor(void)
{
	DRV_SND_REG_GET(stAUD_CLK_DIV_CFG_2, clk_divider_factor, REG_AUD_CLK_DIV_CFG_2);
}
void snd_reg_88H_set_clk_divider_factor(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_CLK_DIV_CFG_2, clk_divider_factor, REG_AUD_CLK_DIV_CFG_2, data);
}

//a8H
mt_u32 snd_reg_a8h_get_hdmi_vol_ctrl(void)
{
	DRV_SND_REG_GET(aud_vol_cfg2_t, hdmi_vol_ctrl, REG_AUD_VOL_CFG_2);
}
void snd_reg_a8h_set_hdmi_vol_ctrl(mt_u32 data)
{
	DRV_SND_REG_SET(aud_vol_cfg2_t, hdmi_vol_ctrl, REG_AUD_VOL_CFG_2, data);
}
mt_u32 snd_reg_a8h_get_adac_vol_ctrl(void)
{
	DRV_SND_REG_GET(aud_vol_cfg2_t, adac_vol_ctrl, REG_AUD_VOL_CFG_2);
}
void snd_reg_a8h_set_adac_vol_ctrl(mt_u32 data)
{
	DRV_SND_REG_SET(aud_vol_cfg2_t, adac_vol_ctrl, REG_AUD_VOL_CFG_2, data);
}

//b0H
mt_u32 snd_reg_b0h_get_spdif_pcm_vol_ctrl(void)
{
	DRV_SND_REG_GET(aud_vol_cfg_spdif_t, spdif_pcm_vol_ctrl, REG_AUD_VOL_CFG_SPDIF);
}
void snd_reg_b0h_set_spdif_pcm_vol_ctrl(mt_u32 data)
{
	DRV_SND_REG_SET(aud_vol_cfg_spdif_t, spdif_pcm_vol_ctrl, REG_AUD_VOL_CFG_SPDIF, data);
}
mt_u32 snd_reg_b0h_get_mix_spdif_alpha(void)
{
	DRV_SND_REG_GET(aud_vol_cfg_spdif_t, mix_spdif_alpha, REG_AUD_VOL_CFG_SPDIF);
}
void snd_reg_b0h_set_mix_spdif_alpha(mt_u32 data)
{
	DRV_SND_REG_SET(aud_vol_cfg_spdif_t, mix_spdif_alpha, REG_AUD_VOL_CFG_SPDIF, data);
}

//c0H
mt_u32 snd_reg_c0H_get_mix_buf_base_address(void)
{
	DRV_SND_REG_GET(stAUD_MIX_BUF_BASE, aud_mix_buf_base, REG_AUD_MIX_BASE_ADDR);
}
void snd_reg_c0H_set_mix_buf_base_address(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_MIX_BUF_BASE, aud_mix_buf_base, REG_AUD_MIX_BASE_ADDR, data);
}

//c4H
mt_u32 snd_reg_c4H_get_mix_buf_length(void)
{
	DRV_SND_REG_GET(stAUD_MIX_BUF_LEN, aud_mix_buf_len, REG_AUD_MIX_BUF_LEN);
}
void snd_reg_c4H_set_mix_buf_length(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_MIX_BUF_LEN, aud_mix_buf_len, REG_AUD_MIX_BUF_LEN, data);
}

//c8H
mt_u32 snd_reg_c8H_get_mix_buf_full_thd(void)
{
	DRV_SND_REG_GET(stAUD_MIX_BUF_FUL_THD, spd_mix_full_thd, REG_AUD_MIX_BUF_FULL_THD);
}
void snd_reg_c8H_set_mix_buf_full_thd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_MIX_BUF_FUL_THD, spd_mix_full_thd, REG_AUD_MIX_BUF_FULL_THD, data);
}

//ccH
mt_u32 snd_reg_ccH_get_mix_buf_wr_len(void)
{
	DRV_SND_REG_GET(stAUD_MIX_BUF_WRCMD, aud_mixbuf_wr_len, REG_AUD_MIX_BUF_WR_CMD);
}
void snd_reg_ccH_set_mix_buf_wr_len(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_MIX_BUF_WRCMD, aud_mixbuf_wr_len, REG_AUD_MIX_BUF_WR_CMD, data);
}
mt_u32 snd_reg_ccH_get_mix_buf_wr_cmd(void)
{
	DRV_SND_REG_GET(stAUD_MIX_BUF_WRCMD, aud_mixbuf_wr_cmd, REG_AUD_MIX_BUF_WR_CMD);
}
void snd_reg_ccH_set_mix_buf_wr_cmd(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_MIX_BUF_WRCMD, aud_mixbuf_wr_cmd, REG_AUD_MIX_BUF_WR_CMD, data);
}

//d0H
mt_u32 snd_reg_d0h_get_mix_func_en(void)
{
	DRV_SND_REG_GET(aud_mix_func_config_t, mix_func_en, REG_AUD_MIX_FUNC_CONFIG);
}
void snd_reg_d0h_set_mix_func_en(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_config_t, mix_func_en, REG_AUD_MIX_FUNC_CONFIG, data);
}
mt_u32 snd_reg_d0h_get_ssrc_enable(void)
{
	DRV_SND_REG_GET(aud_mix_func_config_t, ssrc_enable, REG_AUD_MIX_FUNC_CONFIG);
}
void snd_reg_d0h_set_ssrc_enable(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_config_t, ssrc_enable, REG_AUD_MIX_FUNC_CONFIG, data);
}
mt_u32 snd_reg_d0h_get_ssrc_int(void)
{
	DRV_SND_REG_GET(aud_mix_func_config_t, ssrc_int, REG_AUD_MIX_FUNC_CONFIG);
}
void snd_reg_d0h_set_ssrc_int(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_config_t, ssrc_int, REG_AUD_MIX_FUNC_CONFIG, data);
}
mt_u32 snd_reg_d0h_get_ssrc_frac(void)
{
	DRV_SND_REG_GET(aud_mix_func_config_t, ssrc_frac, REG_AUD_MIX_FUNC_CONFIG);
}
void snd_reg_d0h_set_ssrc_frac(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_config_t, ssrc_frac, REG_AUD_MIX_FUNC_CONFIG, data);
}

//d4H
mt_u32 snd_reg_d4h_get_mix_hdmi_alpha(void)
{
	DRV_SND_REG_GET(aud_mix_func_alpha_t, mix_hdmi_alpha, REG_AUD_MIX_FUNC_ALPHA);
}
void snd_reg_d4h_set_mix_hdmi_alpha(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_alpha_t, mix_hdmi_alpha, REG_AUD_MIX_FUNC_ALPHA, data);
}
mt_u32 snd_reg_d4h_get_mix_adac_alpha(void)
{
	DRV_SND_REG_GET(aud_mix_func_alpha_t, mix_adac_alpha, REG_AUD_MIX_FUNC_ALPHA);
}
void snd_reg_d4h_set_mix_adac_alpha(mt_u32 data)
{
	DRV_SND_REG_SET(aud_mix_func_alpha_t, mix_adac_alpha, REG_AUD_MIX_FUNC_ALPHA, data);
}

//DCH
mt_u32 snd_reg_DCH_get_pcm_play_stop(void)
{
	DRV_SND_REG_GET(stAUD_PLAY_STOP, pcm_play_stop, REG_AUD_PLAY_STOP);
}
void snd_reg_DCH_set_pcm_play_stop(mt_u32 data)
{
	DRV_SND_REG_SET(stAUD_PLAY_STOP, pcm_play_stop, REG_AUD_PLAY_STOP, data);
}

//20cH
void snd_reg_20ch_set_pcm_buf_data_type(mt_u32 data)
{
	DRV_SND_REG_SET(reg_aud_res_reg3_t, pcm_buf_data_type, REG_AUD_SNT_AUD_RES_REG3, data);
}



