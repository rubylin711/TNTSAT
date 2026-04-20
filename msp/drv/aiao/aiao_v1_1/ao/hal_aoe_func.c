/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//#include <asm/setup.h>
#include <asm/io.h>
//#include <mach/hardware.h>
//#include <mach/platform.h>
#include <linux/delay.h>
#include "mt_type.h"
//#include "mt_audsp_aoe.h"
#include "hal_aoe_func.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_mem.h"
#include "mt_drv_module.h"
#include "drv_ao_private.h"

#include "mt_module_debug.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

// todo
#define  IO_ADDRESS_TODO(x)  IO_ADDRESS(x)

static MT_BOOL g_bSwAoeFlag = MT_TRUE;  /* MT_TRUE: sw; MT_FALSE: hw */

ulong  g_audio_reg_base = 0;


//------------------------------------------------------------------
// reg IO

void aria_set_audio_reg_0_channel(ulong u32RegBase, mt_u8 value)
{
   reg_aud_snt_ch_srt_t d;

   d.all = *((volatile mt_u32*)(u32RegBase+ REG_AUD_SNT_CH_SRT));
   d.bitc.ch_input_mode = value;
   *((volatile mt_u32*)(u32RegBase+ REG_AUD_SNT_CH_SRT)) = d.all;
}

//----------------------------------------------------------------------
// reg

static inline void hal_put_u32(volatile mt_u32 *p_addr, mt_u32  data)
{
  /*!
      Write 32 bits register
    */
  *p_addr = data;
}

void reg_sym_linux_00_reg_32_bit(mt_u8 data)
{
	  ulong addr;
    reg_aud_snt_ch_srt_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_CH_SRT;

    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_32b_flag = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_00_reg_sample_rate_bit(mt_u8 data)
{
	  ulong addr;
    reg_aud_snt_ch_srt_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_CH_SRT;

    d.all = *(volatile u32 *)addr;
    d.bitc.sample_rate = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_04_reg_justified_bit(mt_u8 data)
{
	  ulong addr;
    reg_aud_snt_i2s_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.justified_mode = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_04_reg_pcm_chan_spdif_bit(mt_u8 data)
{
	  ulong addr;
    reg_aud_snt_i2s_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_audout_buf = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

mt_u32 reg_sym_linux_04_reg_get_all(void)
{
  ulong addr;

  if(!g_audio_reg_base)
      return 0;

  addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
  return (*(volatile reg_aud_snt_i2s_spdif_t *)addr).all;
}
void reg_sym_linux_04_reg_spdif_chan_spdif_bit(mt_u8 data)
{
	ulong addr;
	reg_aud_snt_i2s_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_spd_buf = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_04_reg_spdif_chan_sel_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_i2s_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_path_sel = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_04_reg_spdif_chan_sel_hdmi_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_i2s_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_I2S_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_path_hdmi = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_volume_reg_gainq_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_vol_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_VOL;
    d.all = *(volatile u32 *)addr;
    d.bitc.gainq = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_volume_reg_scale_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_vol_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_VOL;
    d.all = *(volatile u32 *)addr;
    d.bitc.volume_scale = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_vol_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_VOL;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_hdmi_mute = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_volume_reg_spdif_mute_coax_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_vol_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_VOL;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_coax_mute = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_volume_reg(mt_u8 gainq, mt_u16 volume_scale, mt_u8 mute)
{
	ulong addr;
    reg_aud_snt_vol_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_VOL;
    d.all = *(volatile u32 *)addr;
    d.bitc.volume_scale = volume_scale;
    d.bitc.gainq = gainq;
    d.bitc.spdif_hdmi_mute = mute;
    d.bitc.spdif_coax_mute = mute;
    //can not set following mute reg bits,
    //or has pupu issue.
    //why?
    /*d.bitc.adec_2ch_mute = mute;
    d.bitc.left_mute = mute;
    d.bitc.right_mute = mute;
    d.bitc.bass_mute = mute;
    d.bitc.center_mute = mute;
    d.bitc.sl_mute = mute;
    d.bitc.sr_mute = mute;
    d.bitc.rsl_mute = mute;*/
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_fade_reg(mt_u8 fader_enable, mt_u8 fade_in/*1: fade in, 0: fade out*/, mt_u16 target_gain, mt_u16 fade_timestep)
{
	ulong addr;
    reg_aud_snt_fader_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_FADER;
    d.all = *(volatile u32 *)addr;
    if (fader_enable)
    {
		d.bitc.fader_enable = 1;
		d.bitc.fade_in = fade_in;
		d.bitc.target_gain = target_gain;
		d.bitc.fade_timestep = fade_timestep;
    }
    else
    {
		d.bitc.fader_enable = 0;
    }
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pp_reg_src_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_pp_en_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_EN;
    d.all = *(volatile u32 *)addr;
    d.bitc.src_en = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pp_reg_src_mod_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_pp_en_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_EN;
    d.all = *(volatile u32 *)addr;
    d.bitc.old_src_flag = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pp_reg_chan_mod_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_pp_en_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_EN;
    d.all = *(volatile u32 *)addr;
    //0: 8ch mode
    //1: 2ch mode
    d.bitc.chan_mod_pcm = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pp_reg_downmix_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_pp_en_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_EN;
    d.all = *(volatile u32 *)addr;
    d.bitc.downmix_en = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pp_reg_downmix_hdmi_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_pp_en_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_EN;
    d.all = *(volatile u32 *)addr;
    d.bitc.downmix2hdmi = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_clk1_reg_div_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_clk_div_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_CLK_DIV;
    d.all = *(volatile u32 *)addr;
    d.bitc.clk_divider_factor = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

mt_u32 reg_sym_linux_get_clk1_reg_div_bit(void)
{
    ulong addr;

    if(!g_audio_reg_base)
      return 0;

    addr = g_audio_reg_base + REG_AUD_SNT_CLK_DIV;

    return (*(volatile reg_aud_snt_clk_div_t *)addr).bitc.clk_divider_factor;
}

void reg_sym_linux_clk1_reg_div_bit2(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_clk_div_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_CLK_DIV2;
    d.all = *(volatile u32 *)addr;
    d.bitc.clk_divider_factor = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
mt_u32 reg_sym_linux_get_clk1_reg_div_bit2(void)
{
	ulong addr;

  if(!g_audio_reg_base)
      return 0;

  addr = g_audio_reg_base + REG_AUD_SNT_CLK_DIV2;

  return (*(volatile reg_aud_snt_clk_div_t *)addr).bitc.clk_divider_factor;
}

void reg_sym_linux_ppbase_reg_ppbase_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pp_buf_base_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_BUF_BASE;
    d.all = *(volatile u32 *)addr;
    d.bitc.pp_buf_base = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pplen_reg_pplen_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pp_one_buf_len_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PP_ONE_BUF_LEN;
    d.all = *(volatile u32 *)addr;
    d.bitc.pp_one_buf_len = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}


void reg_sym_linux_pcmbase_reg_pcmbase_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pcm_buf_base_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PCM_BUF_BASE;
    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_buf_base = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_pcmlen_reg_pcmlen_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pcm_buf_len_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_PCM_BUF_LEN;
    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_buf_len = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_spdbase_reg_spdbase_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pcm_buf_base_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SPD_BUF_BASE;
    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_buf_base = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_spdlen_reg_spdlen_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_pcm_buf_len_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SPD_BUF_LEN;
    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_buf_len = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_mix_buff_base_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_mix_buf_base_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_MIX_BUF_BASE_ADDR;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_buf_base = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_mix_buff_len_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_mix_buf_len_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_MIX_BUF_LEN;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_buf_len = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_mix_func_cfg_mix_en_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_mix_func_config_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_MIX_FUNC_CONFIG;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_func_en = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_aud_vol_cfg_spdif_pcm_vol_ctrl_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_vol_cfg_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_AUD_VOL_CFG_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.apdif_pcm_vol_ctrl = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_aud_vol_cfg_spdif_mix_spdif_alpha_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_vol_cfg_spdif_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_AUD_VOL_CFG_SPDIF;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_spdif_alpha = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_mix_func_alpha_mix_adac_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_mix_func_alpha_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_MIX_FUNC_ALPHA;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_adac_alpha = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_mix_func_alpha_mix_hdmi_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_mix_func_alpha_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_MIX_FUNC_ALPHA;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_hdmi_alpha = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_mix_func_cfg_aud_res_reg0_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_res_regs_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_AUD_RES_REG0;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_mix_func_cfg_aud_res_reg3_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_res_regs_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_AUD_RES_REG3;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_left_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_LEFT;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_left_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_LEFT;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_downmix_coef_left_2_all_bit(mt_u32 data)
{
    ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
      return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_LEFT;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_right_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RIGHT;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_right_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RIGHT;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}
void reg_sym_linux_downmix_coef_right_2_all_bit(mt_u32 data)
{
	  ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RIGHT;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_bass_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_BASS;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_bass_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_BASS;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_bass_2_all_bit(mt_u32 data)
{
	  ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_BASS;
    d.all = *(volatile u32 *)addr;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_center_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_CENTER;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_center_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_CENTER;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_center_2_all_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_CENTER;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sl_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SL;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sl_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SL;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sl_2_all_bit(mt_u32 data)
{
	  ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SL;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sr_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SR;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sr_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SR;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_sr_2_all_bit(mt_u32 data)
{
	  ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_SR;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsl_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSL;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsl_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSL;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsl_2_all_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSL;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsr_2_left_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSR;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_left = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsr_2_right_bit(mt_u16 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSR;
    d.all = *(volatile u32 *)addr;
    d.bitc.to_right = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_downmix_coef_rsr_2_all_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_downmix_coef_left_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_DOWNMIX_COEF_RSR;
    d.all = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}


void reg_sym_linux_set_aud_play_stop_bit(mt_u8 data)
{
	ulong addr = 0;
	 reg_aud_play_stop_t d;
    if(!g_audio_reg_base)
        return;
    addr = g_audio_reg_base + REG_AUD_SNT_AUD_PLAY_STOP;   
    d.all = *(volatile u32 *)addr;
    d.bitc.pcm_play_stop = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_reset_reg_sync_reset_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_sync_reset_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;
    d.all = *(volatile u32 *)addr;
    d.bitc.sync_reset = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_reset_reg_spdif_reset_bit(mt_u8 data)
{
	ulong addr;
    reg_aud_snt_sync_reset_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;
    d.all = *(volatile u32 *)addr;
    d.bitc.spdif_reset = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

void reg_sym_linux_reset_reg_mixbuf_reset_bit(mt_u8 data)
{
 ulong addr;
    reg_aud_snt_sync_reset_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;
    d.all = *(volatile u32 *)addr;
    d.bitc.mix_reset = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

mt_u8  reg_sym_linux_reset_reg_sync_wait_bit_read(void)
{
	ulong addr;

    if(!g_audio_reg_base)
        return 0;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;

    return (*(volatile reg_aud_snt_sync_reset_t *)addr).bitc.sync_reset_wait;
}

mt_u8  reg_sym_linux_reset_reg_spdif_wait_bit_read(void)
{
	ulong addr;

    if(!g_audio_reg_base)
        return 0;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;

    return (*(volatile reg_aud_snt_sync_reset_t *)addr).bitc.spdif_reset_wait;
}

mt_u8  reg_sym_linux_reset_reg_mixbuf_wait_bit_read(void)
{
    ulong addr;

    if(!g_audio_reg_base)
        return 0;

    addr = g_audio_reg_base + REG_AUD_SNT_SYNC_RESET;

    return (*(volatile reg_aud_snt_sync_reset_t *)addr).bitc.mix_reset_wait;
} 


void reg_sym_linux_sample_num_reg_sample_num_bit(mt_u32 data)
{
	ulong addr;
    reg_aud_snt_samp_num_frm_t d;

    if(!g_audio_reg_base)
        return;

    addr = g_audio_reg_base + REG_AUD_SNT_SAMP_NUM_FRM;
    d.all = *(volatile u32 *)addr;
    d.bitc.samp_num_perfrm = data;
    hal_put_u32((volatile mt_u32  *)addr, d.all);
}

//-------------------------------------------------------------------

static mt_void AoeIOAddressMap(mt_void)
{
   // if(!g_audio_reg_base)
    //g_audio_reg_base = ioremap(0xffd90000, 0x300);
    return;
}

static mt_void IOaddressUnmap(mt_void)
{
  //if(g_audio_reg_base)
    //iounmap(g_audio_reg_base);
}

mt_s32 iHAL_AOE_Init(MT_BOOL bSwAoeFlag)
{
    AoeIOAddressMap();
    g_bSwAoeFlag = bSwAoeFlag;
    return MT_SUCCESS;
}

mt_void iHAL_AOE_DeInit(mt_void)
{
    IOaddressUnmap();

    return;
}

mt_void iHAL_AOE_GetHwCapability(mt_u32 *pu32Capability)
{
    //TODO
}
mt_void iHAL_AOE_GetHwVersion(mt_u32 *pu32Version)
{
    //TODO
}

extern mt_u8 g_PPBuf_Reseted;
extern mt_u8 g_PCMBuf_Reseted;
extern mt_u8 g_SPDBuf_Reseted;
extern mt_u8 g_MIXBuf_Reseted;
mt_void iHAL_AOE_AIP_Reset_PCMBuf(mt_void)
{
    mt_u32 soft_walk_around = 0;

    //reset  pp and pcm
    reg_sym_linux_set_aud_play_stop_bit(1);
    mdelay(1);
    reg_sym_linux_reset_reg_sync_reset_bit(1);
    while(1)
    {
      mdelay(1);
      if(reg_sym_linux_reset_reg_sync_wait_bit_read()){
        g_PPBuf_Reseted = 1;
        g_PCMBuf_Reseted = 1;
        break;
      }
      
      soft_walk_around++;
      if(soft_walk_around > 10)
      {
        MT_INFO_AO("\n\n  pp reset  time out break\n\n");
        soft_walk_around = 0;
        break;
      }
    }
    reg_sym_linux_reset_reg_sync_reset_bit(0);
    mdelay(1);
    reg_sym_linux_set_aud_play_stop_bit(0);
    mdelay(1);
}

mt_void iHAL_AOE_AIP_Reset_SPDBuf(mt_void)
{
    mt_u32 soft_walk_around = 0;
    
    //reset  spdif
    reg_sym_linux_set_aud_play_stop_bit(1);
    mdelay(1);
    reg_sym_linux_reset_reg_spdif_reset_bit(1);
    while(1)
    {
      mdelay(1);
      if(reg_sym_linux_reset_reg_spdif_wait_bit_read()){
        g_SPDBuf_Reseted = 1;
        break;
      }

      soft_walk_around++;
      if(soft_walk_around > 10)
      {
        MT_INFO_AO("\n\n  spdif reset  time out break\n\n");
        soft_walk_around = 0;
        break;
      }
    }
    reg_sym_linux_reset_reg_spdif_reset_bit(0);
    mdelay(1);
    reg_sym_linux_set_aud_play_stop_bit(0);
    mdelay(1); 
}

mt_void iHAL_AOE_AIP_Reset_MIXBuf(mt_void)
{
    mt_u32 soft_walk_around = 0;

    //reset mix buff
    reg_sym_linux_set_aud_play_stop_bit(1);
    mdelay(10);
    reg_sym_linux_reset_reg_mixbuf_reset_bit(1);
    while(1)
    {
      mdelay(1);
      if(reg_sym_linux_reset_reg_mixbuf_wait_bit_read()){
        g_MIXBuf_Reseted = 1;
        break;
      }
      
      soft_walk_around++;
      if(soft_walk_around > 10)
      {
        MT_FATAL_AO("\n\n  mix buff reset  time out break!!!\n\n");
        soft_walk_around = 0;
        break;
      }
    }
    reg_sym_linux_reset_reg_mixbuf_reset_bit(0);
    mdelay(1);
    reg_sym_linux_set_aud_play_stop_bit(0);
    mdelay(1);
}

mt_void iHAL_AOE_AIP_reset_aout_buffer(mt_void)
{
    iHAL_AOE_AIP_Reset_PCMBuf();

    iHAL_AOE_AIP_Reset_SPDBuf();

#ifdef CONFIG_MT_CHIP_SYMPHONY4	
    iHAL_AOE_AIP_Reset_MIXBuf();
#endif    
}

mt_s32 iHAL_AOE_AIP_SetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr)
{
    //if(SND_ENGINE_TYPE_PCM == pstAttr->sound_type)
    {
      //mt_u32  i,data;
      mt_u32 pp_base,pp_len,pcm_base,pcm_len;

      MT_INFO_AO("\n\n  set pcm addr    phy 0x%x  , vir 0x%x\n\n\n",
        pstAttr->stBufInAttr.stRbfAttr.u32BufPhyAddr ,pstAttr->stBufInAttr.stRbfAttr.u32BufVirAddr);


      #if 1
      //#define PP_BUF_CONFIG_LEN (96 * 1024) // 96K  8K * 8CHAN * 1.5
      pp_base = pstAttr->stBufInAttr.stRbfAttr.u32BufPhyAddr & 0x3fffffff;
      pp_len    = PP_BUF_CONFIG_LEN >> 3;

      pcm_base = pp_base + PP_BUF_CONFIG_LEN;
      pcm_len = pstAttr->stBufInAttr.stRbfAttr.u32BufSize - PP_BUF_CONFIG_LEN;


      reg_sym_linux_ppbase_reg_ppbase_bit(pp_base >> 3 );
      reg_sym_linux_pplen_reg_pplen_bit(pp_len >> 3);


      //set pcm buffer addr ; pcm buffer size
      reg_sym_linux_pcmbase_reg_pcmbase_bit(pcm_base >> 3);
      reg_sym_linux_pcmlen_reg_pcmlen_bit(pcm_len >> 3 );

      MT_ERR_AO("reg pcm addr 0x%x len 0x%x\n", (pcm_base >> 3), (pcm_len >> 3));


      if(pstAttr->stBufInAttr.stRbfAttr.b_spdif_mode)
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(1); //spdif data
        reg_sym_linux_pp_reg_chan_mod_bit(1);


        pstAttr->stBufInAttr.stRbfAttr.u32BufVirAddr += PP_BUF_CONFIG_LEN;
        pstAttr->stBufInAttr.stRbfAttr.u32BufSize = pcm_len;
        MT_INFO_AO("iHAL_AOE_AIP_SetAttr2 pcmmmmmmmmmm buf len 0x%x  \n\n\n\n", pstAttr->stBufInAttr.stRbfAttr.u32BufSize);
      }
      else
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(0); //pcm data
        reg_sym_linux_pp_reg_chan_mod_bit(0);

        pstAttr->stBufInAttr.stRbfAttr.u32BufSize = pp_len;
        MT_INFO_AO("iHAL_AOE_AIP_SetAttr2 pppppp buf len 0x%x  \n\n\n\n", pstAttr->stBufInAttr.stRbfAttr.u32BufSize);
      }
      #else
        pp_base = pstAttr->stBufInAttr.stRbfAttr.u32BufPhyAddr & 0x3fffffff;
      pp_len    = pstAttr->stBufInAttr.stRbfAttr.u32BufSize >> (1 + 3);

      pcm_base = pp_base + (pstAttr->stBufInAttr.stRbfAttr.u32BufSize >> 1);
      pcm_len = pstAttr->stBufInAttr.stRbfAttr.u32BufSize >> 1;


      reg_sym_linux_ppbase_reg_ppbase_bit(pp_base >> 3 );
      reg_sym_linux_pplen_reg_pplen_bit(pp_len >> 3);


      //set pcm buffer addr ; pcm buffer size
      reg_sym_linux_pcmbase_reg_pcmbase_bit(pcm_base >> 3);
      reg_sym_linux_pcmlen_reg_pcmlen_bit(pcm_len >> 3 );

      MT_INFO_AO("\n\n\n iHAL_AOE_AIP_SetAttr reg pcm addr  0x%x  len 0x%x\n\n\n",
        (pcm_base >> 3), (pcm_len >> 3));


      if(pstAttr->stBufInAttr.stRbfAttr.b_spdif_mode)
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(1); //spdif data


        pstAttr->stBufInAttr.stRbfAttr.u32BufVirAddr += (pstAttr->stBufInAttr.stRbfAttr.u32BufSize >> 1);
        pstAttr->stBufInAttr.stRbfAttr.u32BufSize >>= 1;
        MT_INFO_AO("\n\n\n\n\n  iHAL_AOE_AIP_SetAttr2 pcmmmmmmmmmm buf len 0x%x  \n\n\n\n", pstAttr->stBufInAttr.stRbfAttr.u32BufSize);
      }
      else
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(0); //pcm data
        pstAttr->stBufInAttr.stRbfAttr.u32BufSize >>= 4;
        MT_INFO_AO("\n\n\n\n\n  iHAL_AOE_AIP_SetAttr2 pppppp buf len 0x%x  \n\n\n\n", pstAttr->stBufInAttr.stRbfAttr.u32BufSize);
      }

      #endif

      //reset  pp  pcm  buffer  and spdif buffer
      iHAL_AOE_AIP_reset_aout_buffer();
    }
    return MT_SUCCESS;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 iHAL_Init_Out_Buf_Reg(AOE_AIP_CHN_ATTR_NEW_S *pstAttr, AOE_AIP_CHN_ATTR_NEW_S *pstAttr2, AOE_AIP_CHN_ATTR_NEW_S *pstAttr3, MT_BOOL b_spdif)
#else
mt_s32 iHAL_Init_Out_Buf_Reg(AOE_AIP_CHN_ATTR_NEW_S *pstAttr, AOE_AIP_CHN_ATTR_NEW_S *pstAttr2, MT_BOOL b_spdif)
#endif
{

      //mt_u32  i,data;
      mt_u32 pp_base,pp_len,pcm_base,pcm_len;

    //  MT_ALWAYS_PRINT("set pcm addr phy 0x%px, vir 0x%px\n", pstAttr->u32StartPhyAddr ,pstAttr->u32StartVirAddr);


      //set pp
      pp_base = pstAttr->u32StartPhyAddr & 0x3fffffff;
      pp_len    = PP_BUF_CONFIG_LEN >> 3;

      reg_sym_linux_ppbase_reg_ppbase_bit(pp_base >> 3 );
      reg_sym_linux_pplen_reg_pplen_bit(pp_len >> 3);

      //set pcm
      pcm_base = pp_base + PP_BUF_CONFIG_LEN;
      pcm_len = pstAttr->u32Size - PP_BUF_CONFIG_LEN;

      //set pcm buffer addr ; pcm buffer size
      reg_sym_linux_pcmbase_reg_pcmbase_bit(pcm_base >> 3);
      reg_sym_linux_pcmlen_reg_pcmlen_bit(pcm_len >> 3 );


      //set spdif
      reg_sym_linux_spdbase_reg_spdbase_bit((pstAttr2->u32StartPhyAddr & 0x3fffffff) >> 3);
      reg_sym_linux_spdlen_reg_spdlen_bit(pstAttr2->u32Size >> 3 );

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
      //set mix
      reg_sym_linux_mix_buff_base_bit((pstAttr3->u32StartPhyAddr & 0x3fffffff) >> 3);
      reg_sym_linux_mix_buff_len_bit(pstAttr3->u32Size >> 3 );
#endif	  

      if(b_spdif)
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(1); //spdif data
        reg_sym_linux_pp_reg_chan_mod_bit(1);
      }
      else
      {
        reg_sym_linux_04_reg_pcm_chan_spdif_bit(0); //pcm data
        reg_sym_linux_pp_reg_chan_mod_bit(0);
      }

        reg_sym_linux_04_reg_spdif_chan_spdif_bit(1);  //spdif format
        reg_sym_linux_04_reg_spdif_chan_sel_bit(0 );  //choose spdif channel  ,connect pcm

      //reset  pp  pcm  buffer  and spdif buffer
      iHAL_AOE_AIP_reset_aout_buffer();

    return MT_SUCCESS;
}
mt_s32 iHAL_AOE_AIP_GetStatus(AOE_AIP_ID_E enAIP, mt_void *pstStatus)
{
    return MT_SUCCESS;
}
mt_s32 iHAL_AOE_AIP_Create(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr)
{
    return MT_SUCCESS;
}

mt_void iHAL_AOE_AIP_Destroy(AOE_AIP_ID_E enAIP)
{
    return;
}


mt_u32  iHAL_AOE_AIP_Group_AckCmd(mt_u32 u32AckCmdMask)
{
    return MT_SUCCESS;
}

mt_void iHAL_AOE_AIP_SetVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumedB)
{
    return ;
}

mt_void iHAL_AOE_AIP_SetLRVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB)
{
    return ;
}

mt_void iHAL_AOE_AIP_SetMute(AOE_AIP_ID_E enAIP, MT_BOOL bMute)
{
    return;
}

mt_void iHAL_AOE_AIP_SetChannelMode(AOE_AIP_ID_E enAIP, mt_u32 u32ChannelMode)
{
    return;
}

mt_s32 iHAL_AOE_AIP_SetSpeed(AOE_AIP_ID_E enAIP, mt_s32 s32AdjSpeed)
{
    return MT_SUCCESS;
}

mt_void iHAL_AOE_AIP_GetRptrAndWptrRegAddr(AOE_AIP_ID_E enAIP, mt_u32 *pu32WptrReg, mt_u32 *pu32RptrReg)
{
}

mt_void iHAL_AOE_AIP_ReSetRptrAndWptrReg(AOE_AIP_ID_E enAIP)
{
}

mt_u32 iHAL_AOE_AIP_GetFiFoDelayMs(AOE_AIP_ID_E enAIP)
{
    return MT_SUCCESS;
}

/* aop func */
mt_void iHAL_AOE_AOP_SetMute(AOE_AOP_ID_E enAOP, MT_BOOL bMute)
{

    return;
}

mt_void iHAL_AOE_AOP_SetLRVolume(AOE_AOP_ID_E enAOP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB)
{

    return;
}

mt_void iHAL_AOE_AOP_GetRptrAndWptrRegAddr(AOE_AOP_ID_E enAOP, mt_u32 *pu32WptrReg, mt_u32 *pu32RptrReg)
{
}

mt_s32 iHAL_AOE_AOP_SetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_AOP_GetStatus(AOE_AOP_ID_E enAOP, mt_void *pstStatus)
{
    return MT_SUCCESS;
}

mt_void                 iHAL_AOE_AOP_Destroy(AOE_AOP_ID_E enAOP)
{
    return;
}

mt_s32 iHAL_AOE_AOP_SetAefBypass(AOE_AOP_ID_E enAOP, MT_BOOL bBypass)
{
    return MT_SUCCESS;
}


mt_s32 iHAL_AOE_ENGINE_SetAttr(AOE_ENGINE_ID_E enEngine, AOE_ENGINE_CHN_ATTR_S stAttr)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_AttachAip(AOE_ENGINE_ID_E enEngine, AOE_AIP_ID_E enAIP)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_DetachAip(AOE_ENGINE_ID_E enEngine, AOE_AIP_ID_E enAIP)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_AttachAop(AOE_ENGINE_ID_E enEngine, AOE_AOP_ID_E enAOP)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_DetachAop(AOE_ENGINE_ID_E enEngine, AOE_AOP_ID_E enAOP)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_AttachAef(AOE_ENGINE_ID_E enEngine, mt_u32 u32AefId)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_DetachAef(AOE_ENGINE_ID_E enEngine, mt_u32 u32AefId)
{
    return MT_SUCCESS;
}

mt_s32 iHAL_AOE_ENGINE_GetStatus(AOE_ENGINE_ID_E enEngine, mt_void *pstStatus)
{
    return MT_SUCCESS;
}

mt_void                 iHAL_AOE_ENGINE_Destroy(AOE_ENGINE_ID_E enENGINE)
{
    return;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
