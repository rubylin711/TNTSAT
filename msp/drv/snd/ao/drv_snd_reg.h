/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_reg.h <2022-10-31>																*/
/********************************************************************************************/
#ifndef __DRV_SND_REG_H__
#define __DRV_SND_REG_H__

#include <asm/io.h>

extern mt_u32 drv_snd_reg_rd(mt_u32 p_addr);
extern mt_void drv_snd_reg_wt(mt_u32 p_addr, mt_u32 data);

#define AOUT_MCLK	262		//refer to 0xbf50a504[1:0], TODO... check aout clock dynamically
//#define AUD_SAMPLE_RATE_EF  327860		//327860 = 256 * 2^25 / 262000(mclk:262M) * 10
//#define AOUT_MCLK	108		//refer to 0xbf50a504[1:0], TODO... check aout clock dynamically
#define AUD_SAMPLE_RATE_EF  327860		//327860 = 256 * 2^25 / 262000(mclk:262M) * 10

#define AOUT_MCLK_131M		131000000
#define AOUT_MCLK_206M		206000000
#define AOUT_MCLK_262M		261818238
#define AOUT_MCLK_288M		288000000

//aout_mclk
//for symphony4
#define REG_AOUT_CLK_REG_BASE		0xBF50A500

#define REG_AOUT_CLKEN_REG			0xA500
#define REG_AOUT_CLKEN_REG_MASK		0x7
#define REG_AOUT_CLKEN_REG_SHIFT	0

#define REG_AOUT_CLKSEL_REG			0xA504

#define REG_AOUT_SRSTN_REG			0xA50C
#define REG_AOUT_SRSTN_REG_MASK		0x1f
#define REG_AOUT_SRSTN_REG_SHIFT	0

//0xBF50A500
typedef union stAUD_AOUT_CLKEN_REG_t
{
	u32 all;
	struct
	{
		u32 aout_clken			: 1;
		u32 spdif_clken			: 1;
		u32 					: 30;
	} bit;
} stAUD_AOUT_CLKEN_REG;

//0xBF50A504
typedef union stAUD_AOUT_CLKSEL_REG_t
{
	u32 all;
	struct
	{
		u32 aout_clksel			: 2;//0-288M, 1-262M, 2-206M, 3-131M
		u32 mclk_clksel			: 1;//0-dig_mclk, 1-aout_dac_clock
		u32 					: 29;
	} bit;
} stAUD_AOUT_CLKSEL_REG;

//0xBF50A50C
typedef union stAUD_AOUT_SRSTN_REG_t
{
	u32 all;
	struct
	{
		u32 aout_ahb			: 1;
		u32 aout_axi			: 1;
		u32 aout_core			: 1;
		u32 aout_mclk			: 1;
		u32 aout_adac			: 1;
		u32 					: 27;
	} bit;
} stAUD_AOUT_SRSTN_REG;


//hdmi audio control
#define REG_HDMI_AUDIO_CTRL		0xBF480050

//aout reg base
#define REG_AOUT_REG_BASE		0xBF490000
#define AOUT_REG_NUM			132		//0x0~0x20C	

enum{
	REG_AUD_CH_SRT_CFG			= 0x00,
	REG_AUD_I2S_SPDIF_CFG		= 0x04,
	REG_AUD_VOL_CFG				= 0x08,
	REG_AUD_PP_EN_CFG			= 0x0C,
	
	//aout/mix buff clock
	REG_AUD_CLK_DIV_CFG			= 0x10,
	REG_AUD_CLK_ADJ				= 0x14,
	
	//aout buff
	REG_AUD_OUT_BUF_BASE		= 0x18,
	REG_AUD_OUT_BUF_LEN			= 0x1C,
	REG_AUD_OUT_BUF_FUL_THD		= 0x20,
	REG_AUD_OUT_BUF_WRCMD		= 0x24,
	
	//pp buff
	REG_AUD_PP_BUF_BASE			= 0x28,
	REG_AUD_PP_BUF_LEN			= 0x2C,
	REG_AUD_PP_BUF_FUL_THD		= 0x30,
	REG_AUD_PP_BUF_WRCMD		= 0x34,
	
	//spd buff
	REG_AUD_SPD_BUF_BASE		= 0x38,
	REG_AUD_SPD_BUF_LEN			= 0x3C,
	REG_AUD_SPD_BUF_FUL_THD		= 0x40,
	REG_AUD_SPD_BUF_WRCMD		= 0x44,
	
	REG_AUD_OUT_BUF_JUMP_EN		= 0x48,
	REG_AUD_SRC_CFG				= 0x4C,
	
	//agc
	REG_AUD_AGC_CFG0			= 0x50,
	REG_AUD_AGC_CFG1			= 0x54,
	REG_AUD_AGC_CFG2			= 0x58,

	//fade in/out
	REG_AUD_FADER_CFG			= 0x5C,

	//downmix
	REG_AUD_DMX_CFG0			= 0x60,
	REG_AUD_DMX_CFG1			= 0x64,
	REG_AUD_DMX_CFG2			= 0x68,
	REG_AUD_DMX_CFG3			= 0x6C,
	REG_AUD_DMX_CFG4			= 0x70,
	REG_AUD_DMX_CFG5			= 0x74,
	REG_AUD_DMX_CFG6			= 0x78,
	REG_AUD_DMX_CFG7			= 0x7C,

	//buff reset
	REG_AUD_SYNC_RESET			= 0x80,
	
	//preload
	REG_AUD_DATA_PRELOAD		= 0x84,
	
	//spd buff clock
	REG_AUD_CLK_DIV_CFG_2		= 0x88,
	REG_AUD_CLK_ADJ_2			= 0x8C,

	REG_AUD_SAMP_NUM_FRM		= 0x90,
	REG_AUD_PPBUFW_INTR_TIMER	= 0x94,
	REG_AUD_BUFRW_INTR_TIMER	= 0x98,
	REG_AUD_PCMFIFO_INTR_TIMER	= 0x9C,
	REG_AUD_INTR_SET			= 0xA0,
	REG_AUD_SPD_BUF_JUMP_EN		= 0xA4,
	
	REG_AUD_VOL_CFG_2			= 0xA8,
	REG_AUD_VOL_CFG_AD			= 0xAC,
	REG_AUD_VOL_CFG_SPDIF		= 0xB0,
	
	REG_AUD_OUT_FIFO_THD		= 0xB4,
	REG_AUD_SPD_FIFO_THD		= 0xB8,
	
	REG_AUD_AHB_DATA			= 0xBC,
	
	REG_AUD_MIX_BASE_ADDR		= 0xC0,
	REG_AUD_MIX_BUF_LEN			= 0xC4,
	REG_AUD_MIX_BUF_FULL_THD	= 0xC8,
	REG_AUD_MIX_BUF_WR_CMD		= 0xCC,
	REG_AUD_MIX_FUNC_CONFIG		= 0xD0,
	REG_AUD_MIX_FUNC_ALPHA		= 0xD4,

	REG_AUD_FRM_INTR_CFG		= 0xD8,
	REG_AUD_PLAY_STOP			= 0xDC,

	/***********addr blank1***********/
	
	REG_AUD_OUTFIFO_EMPTY_FLAG	= 0xE0,

	/***********addr blank2***********/
	
	REG_AUD_OUT_BUF_RADDR		= 0x100,
	REG_AUD_OUT_BUF_CNT			= 0x104,
	REG_AUD_OUT_FIFO_CNT		= 0x108,
	REG_AUD_OUT_BUF_WADDR		= 0x10C,

	REG_AUD_PP_BUF_RADDR		= 0x110,
	REG_AUD_PP_BUF0_CNT			= 0x114,
	REG_AUD_PP_BUF1_CNT			= 0x118,
	REG_AUD_PP_FIFO_CNT0		= 0x10C,
	REG_AUD_PP_FIFO_CNT1		= 0x120,
	
	REG_AUD_SPD_BUF_RADDR		= 0x124,
	REG_AUD_SPD_BUF_CNT			= 0x128,
	
	REG_AUD_BUF_FULL_FLAG		= 0x12C,
	REG_AUD_FRMINTR_CNT			= 0x130,
	REG_AUD_PPINTR_CNT			= 0x134,
	REG_AUD_BUFINTR_CNT			= 0x138,
	REG_AUD_PCMINTR_CNT			= 0x13C,
	REG_AUD_FRM_CNT				= 0x140,
	REG_AUD_FRM_CNT_2			= 0x144,
	
	REG_AUD_MIX_BUF_CNT			= 0x148,
	REG_AUD_MIX_BUF_RADDR		= 0x14C,
	
	REG_AUD_PP_BUF2_CNT			= 0x150,
	REG_AUD_PP_BUF3_CNT			= 0x154,
	REG_AUD_PP_BUF4_CNT			= 0x158,
	REG_AUD_PP_BUF5_CNT			= 0x15C,
	REG_AUD_PP_BUF6_CNT			= 0x160,
	REG_AUD_PP_BUF7_CNT			= 0x164,
	
	REG_AUD_PP_SRC_CNT			= 0x168,
	REG_AUD_FIFO_EMPTY_CNT		= 0x16C,

	/***********addr blank3***********/
	
	REG_AUD_CHIP_ID				= 0x17C,
	
	REG_AUD_AXIARB_CFG_0		= 0x180,
	REG_AUD_AXIARB_CFG_1		= 0x184,
	REG_AUD_AXIARB_CFG_2		= 0x188,
	REG_AUD_AXIARB_CFG_3		= 0x18C,
	REG_AUD_AXIARB_CFG_4		= 0x190,
	REG_AUD_AXIARB_CFG_5		= 0x194,
	REG_AUD_AXIARB_CFG_6		= 0x198,

	REG_AUD_SNT_AUD_RES_REG0	= 0x200,
	REG_AUD_SNT_AUD_RES_REG1	= 0x204,
	REG_AUD_SNT_AUD_RES_REG2	= 0x205,
	REG_AUD_SNT_AUD_RES_REG3	= 0x20c,

	REG_AUD_ADDRESS_INVALID
};


/*****************************************************************************
[STRUCTURE]
******************************************************************************/
//00H
typedef union stAUD_CH_SRT_CFG_t
{
	u32 all;
	struct
	{
		u32 sample_rate					: 4;
		u32 pcm_32b_flag				: 1;
		u32								: 3;
		u32 ch_input_mode				: 8;
		u32 spdbuf_sample_rate			: 4;
		u32 spdbuf_sample_rate_sel		: 1;
		u32 spdbuf_128fs				: 1;
		u32 							: 2;
		u32 pcmbuf_sample_rate			: 4;
		u32 pcmbuf_sample_rate_sel		: 1;
		u32 							: 3;
	} bit;
} stAUD_CH_SRT_CFG;

//04H
typedef union stAUD_I2S_SPDIF_CFG_t
{
	u32 all;
	struct
	{
		u32 right_flag					: 1;
		u32 justified_mode				: 2;
		u32 hdmi_manu_mute				: 1;
		u32 i2s_ch_sel					: 3;
		u32 spdif_mixbuf				: 1;
		u32 spdif_audout_buf			: 1;
		u32 spdif_spd_buf				: 1;
		u32 spdif_path_sel_spdif		: 1;
		u32 spdif_path_sel_hdmi			: 1;
		u32 adac_ch_sel					: 3;
		u32 adac_manu_mute				: 1;
		u32 adac_valid_flag_cnt			: 7;
		u32 new_adac_valid_flag			: 1;
		u32 pcm_valid_cnt				: 7;
		u32 adac_pcm_decim2				: 1;
	} bit;
} stAUD_I2S_SPDIF_CFG;

//08H
typedef union stAUD_VOL_CFG_t
{
	u32 all;
	struct
	{
		u32 volume_scale				: 16;
		u32 gainq						: 3;
		u32 gainq_manu_zero				: 1;
		u32 i2s_mute					: 1;
		u32 spdif_hdmi_mute				: 1;
		u32 spdif_coax_mute				: 1;
		u32 adac_2ch_mute				: 1;
		u32 left_mute					: 1;
		u32 right_mute					: 1;
		u32 bass_mute					: 1;
		u32 center_mute					: 1;
		u32 sl_mute						: 1;
		u32 sr_mute						: 1;
		u32 rsl_mute					: 1;
		u32 rsr_mute					: 1;
	} bit;
} stAUD_VOL_CFG;

//0CH
typedef union stAUD_PP_EN_CFG_t
{
	u32 all;
	struct
	{
		u32 src_en						: 1;
		u32 downmix_en					: 1;
		u32 agc_en						: 1;
		u32 chan_mod_pcmbuf				: 1;
		u32 ahbdata_en					: 2; 
		u32 							: 1;
		u32 old_src_flag				: 1;
		u32 dmx_hdmi_en					: 1;
		u32 dmx_manu_close				: 1;
		u32 chmode_manu_2ch				: 1;
		u32 							: 1;
		u32 audio_lr_copy				: 2;
		u32 adac_audio_lr_copy			: 2;
		u32 							: 16;
	} bit;
} stAUD_PP_EN_CFG;

//10H
typedef union stAUD_CLK_DIV_CFG_t
{
	u32 all;
	struct
	{
		u32 clk_divider_factor			: 25;
		u32 							: 7;
	} bit;
} stAUD_CLK_DIV_CFG;

//14H
typedef union stAUD_CLK_ADJ_t
{
	u32 all;
	struct
	{
		u32 sample_rate_offset			: 25;
		u32 							: 7;
	} bit;
} stAUD_CLK_ADJ;

//18H
typedef union stAUD_OUT_BUF_BASE_t
{
	u32 all;
	struct
	{
		u32 audio_out_buf_base_address	: 29;
		u32 							: 3;
	} bit;
} stAUD_OUT_BUF_BASE;

//1CH
typedef union stAUD_OUT_BUF_LEN_t
{
	u32 all;
	struct
	{
		u32 audio_out_buf_length		: 25;
		u32 							: 7;
	} bit;
} stAUD_OUT_BUF_LEN;

//20H
typedef union stAUD_OUT_BUF_FUL_THD_t
{
	u32 all;
	struct
	{
		u32 audout_buf_full_thd			: 25;
		u32 							: 7;
	} bit;
} stAUD_OUT_BUF_FUL_THD;

//24H
typedef union stAUD_OUT_BUF_WRCMD_t
{
	u32 all;
	struct
	{
		u32 aud_out_buf_wr_len			: 25;
		u32 							: 6;
		u32 aud_out_buf_wr_cmd			: 1;
	} bit;
} stAUD_OUT_BUF_WRCMD;

//28H
typedef union stAUD_PP_BUF_BASE_t
{
	u32 all;
	struct
	{
		u32 ppbuf0_base_address			: 29;
		u32 							: 3;
	} bit;
} stAUD_PP_BUF_BASE;

//2CH
typedef union stAUD_PP_BUF_LEN_t
{
	u32 all;
	struct
	{
		u32 one_pp_buffer_length		: 20;
		u32 							: 12;
	} bit;
} stAUD_PP_BUF_LEN;

//30H
typedef union stAUD_PP_BUF_FUL_THD_t
{
	u32 all;
	struct
	{
		u32 pp_buf_full_thd				: 20;
		u32 							: 12;
	} bit;
} stAUD_PP_BUF_FUL_THD;

//34H
typedef union stAUD_PP_BUF_WRCMD_t
{
	u32 all;
	struct
	{
		u32 pp_buf_wr_len				: 20;
		u32 							: 4;
		u32 pp_buf_wr_sel				: 3;
		u32 							: 4;
		u32 pp_buf_wr_cmd				: 1;
	} bit;
} stAUD_PP_BUF_WRCMD;

//38H
typedef union stAUD_SPD_BUF_BASE_t
{
	u32 all;
	struct
	{
		u32 aud_spd_buf_base			: 29;
		u32 							: 3;
	} bit;
} stAUD_SPD_BUF_BASE;

//3CH
typedef union stAUD_SPD_BUF_LEN_t
{
	u32 all;
	struct
	{
		u32 aud_spd_buf_len				: 25;
		u32 							: 7;
	} bit;
} stAUD_SPD_BUF_LEN;

//40H
typedef union stAUD_SPD_BUF_FUL_THD_t
{
	u32 all;
	struct
	{
		u32 spd_buf_full_thd			: 25;
		u32 							: 7;
	} bit;
} stAUD_SPD_BUF_FUL_THD;

//44H
typedef union stAUD_SPD_BUF_WRCMD_t
{
	u32 all;
	struct
	{
		u32 aud_spdbuf_wr_len			: 25;
		u32 							: 6;
		u32 aud_spdbuf_wr_cmd			: 1;
	} bit;
} stAUD_SPD_BUF_WRCMD;

//48H
typedef union stAUD_OUT_BUF_JUMP_EN_t
{
	u32 all;
	struct
	{
		u32 audio_out_buf_jump_cnt		: 25;
		u32 							: 6;
		u32 audio_out_buf_jump_en			: 1;
	} bit;
} stAUD_OUT_BUF_JUMP_EN;

//4CH
typedef union stAUD_SRC_CFG_t
{
	u32 all;
	struct
	{
		u32 src_coef					: 21;
		u32 coef_address				: 9;
		u32 							: 1;
		u32 coef_wr						: 1;
	} bit;
} stAUD_SRC_CFG;

//5cH
typedef union stAUD_FADER_CFG_t
{
	u32 all;
	struct
	{
		u32	fade_en						: 1;
		u32 fade_in						: 1; //0-raw data, 1-pcm data
		u32	fade_step					: 2;
		u32 target_gain					: 12;
		u32 time_step					: 16;
	} bit;
} stAUD_FADER_CFG;

//80H
typedef union stAUD_SYNC_RESET_t
{
	u32 all;
	struct
	{
		u32 audio_sync_reset			: 1;		// aout && pp
		u32 mixbuf_reset				: 1;
		u32 							: 2;
		u32 spdbuf_reset				: 1;
		u32 							: 19;
		u32 ppbuf_clear_end_flag		: 1;
		u32 mixbuf_clear_end_flag		: 1;
		u32 							: 2;
		u32 spdbuf_clear_end_flag		: 1;
		u32 							: 3;
	} bit;
} stAUD_SYNC_RESET;

//88H
typedef union stAUD_CLK_DIV_CFG_2_t
{
	u32 all;
	struct
	{
		u32 clk_divider_factor			: 25;
		u32 							: 7;
	} bit;
} stAUD_CLK_DIV_CFG_2;

//a8H
typedef union aud_vol_cfg2
{
	u32 all;
	struct
	{
		u32 hdmi_vol_ctrl				: 16;
		u32 adac_vol_ctrl				: 16;
	} bit;
} aud_vol_cfg2_t;

//b0H
typedef union aud_vol_cfg_spdif
{
	u32 all;
	struct
	{
		u32 spdif_pcm_vol_ctrl			: 16;
		u32 mix_spdif_alpha				: 16;
	} bit;
} aud_vol_cfg_spdif_t;

//c0H
typedef union stAUD_MIX_BUF_BASE_t
{
	u32 all;
	struct
	{
		u32 aud_mix_buf_base			: 29;
		u32 							: 3;
	} bit;
} stAUD_MIX_BUF_BASE;

//c4H
typedef union stAUD_MIX_BUF_LEN_t
{
	u32 all;
	struct
	{
		u32 aud_mix_buf_len				: 25;
		u32 							: 7;
	} bit;
} stAUD_MIX_BUF_LEN;

//c8H
typedef union stAUD_MIX_BUF_FUL_THD_t
{
	u32 all;
	struct
	{
		u32 spd_mix_full_thd			: 25;
		u32 							: 7;
	} bit;
} stAUD_MIX_BUF_FUL_THD;

//ccH
typedef union stAUD_MIX_BUF_WRCMD_t
{
	u32 all;
	struct
	{
		u32 aud_mixbuf_wr_len			: 25;
		u32 							: 6;
		u32 aud_mixbuf_wr_cmd			: 1;
	} bit;
} stAUD_MIX_BUF_WRCMD;

//d4H
typedef union aud_mix_func_alpha
{
	u32 all;
	struct
	{
		u32 mix_hdmi_alpha			: 16;
		u32 mix_adac_alpha			: 16;
	} bit;
} aud_mix_func_alpha_t;

//d0H
typedef union aud_mix_func_config
{
	u32 all;
	struct
	{
		u32 mix_func_en					: 1;
		u32 ssrc_enable					: 1;
		u32 							: 2;
		u32 ssrc_int					: 3;
		u32 							: 1;
		u32 ssrc_frac					: 12;
		u32 							: 12;
	} bit;
} aud_mix_func_config_t;

//DCH
typedef union stAUD_PLAY_STOP_t
{
	u32 all;
	struct
	{
		u32 pcm_play_stop				: 1;
		u32 							: 31;
	} bit;
} stAUD_PLAY_STOP;

//20cH
typedef union reg_aud_res_reg3
{
	u32 all;
	struct
	{
		u32								: 1;
		u32 pcm_buf_data_type			: 1; //0-raw data, 1-pcm data
		u32								: 30;
	} bit;
} reg_aud_res_reg3_t;

/*****************************************************************************
[FUNCTION]
******************************************************************************/
#define MREAD(A) (*((volatile unsigned int *)(A)))
#define MWRITE(A, V) *((volatile unsigned int *)(A)) = (V)

mt_u32 drv_snd_reg_rd(mt_u32 p_addr);
mt_void drv_snd_reg_wt(mt_u32 p_addr, mt_u32 data);
mt_u32 drv_aout_clk_reg_rd(mt_u32 p_addr);
mt_void drv_aout_clk_reg_wt(mt_u32 p_addr, mt_u32 data);

mt_u32 snd_reg_00H_get_sample_rate(void);
void snd_reg_00H_set_sample_rate(mt_u32 data);
mt_u32 snd_reg_00H_get_pcm_32b_flag(void);
void snd_reg_00H_set_pcm_32b_flag(mt_u32 data);
mt_u32 snd_reg_00H_get_ch_input_mode(void);
void snd_reg_00H_set_ch_input_mode(mt_u32 data);
mt_u32 snd_reg_00H_get_spdbuf_sample_rate(void);
void snd_reg_00H_set_spdbuf_sample_rate(mt_u32 data);
mt_u32 snd_reg_00H_get_spdbuf_sample_rate_sel(void);
void snd_reg_00H_set_spdbuf_sample_rate_sel(mt_u32 data);
mt_u32 snd_reg_00H_get_spdbuf_128fs(void);
void snd_reg_00H_set_spdbuf_128fs(mt_u32 data);
mt_u32 snd_reg_00H_get_pcmbuf_sample_rate(void);
void snd_reg_00H_set_pcmbuf_sample_rate(mt_u32 data);
mt_u32 snd_reg_00H_get_pcmbuf_sample_rate_sel(void);
void snd_reg_00H_set_pcmbuf_sample_rate_sel(mt_u32 data);

mt_u32 snd_reg_04H_get_right_flag(void);
void snd_reg_04H_set_right_flag(mt_u32 data);
mt_u32 snd_reg_04H_get_justified_mode(void);
void snd_reg_04H_set_justified_mode(mt_u32 data);
mt_u32 snd_reg_04H_get_hdmi_manu_mute(void);
void snd_reg_04H_set_hdmi_manu_mute(mt_u32 data);
mt_u32 snd_reg_04H_get_i2s_ch_sel(void);
void snd_reg_04H_set_i2s_ch_sel(mt_u32 data);
mt_u32 snd_reg_04H_get_spdif_mixbuf(void);
void snd_reg_04H_set_spdif_mixbuf(mt_u32 data);
mt_u32 snd_reg_04H_get_spdif_audout_buf(void);
void snd_reg_04H_set_spdif_audout_buf(mt_u32 data);
mt_u32 snd_reg_04H_get_spdif_spd_buf(void);
void snd_reg_04H_set_spdif_spd_buf(mt_u32 data);
mt_u32 snd_reg_04H_get_spdif_path_sel_spdif(void);
void snd_reg_04H_set_spdif_path_sel_spdif(mt_u32 data);
mt_u32 snd_reg_04H_get_spdif_path_sel_hdmi(void);
void snd_reg_04H_set_spdif_path_sel_hdmi(mt_u32 data);
mt_u32 snd_reg_04H_get_adac_ch_sel(void);
void snd_reg_04H_set_adac_ch_sel(mt_u32 data);
mt_u32 snd_reg_04H_get_adac_manu_mute(void);
void snd_reg_04H_set_adac_manu_mute(mt_u32 data);
mt_u32 snd_reg_04H_get_adac_valid_flag_cnt(void);
void snd_reg_04H_set_adac_valid_flag_cnt(mt_u32 data);
mt_u32 snd_reg_04H_get_new_adac_valid_flag(void);
void snd_reg_04H_set_new_adac_valid_flag(mt_u32 data);
mt_u32 snd_reg_04H_get_pcm_valid_cnt(void);
void snd_reg_04H_set_pcm_valid_cnt(mt_u32 data);
mt_u32 snd_reg_04H_get_adac_pcm_decim2(void);
void snd_reg_04H_set_adac_pcm_decim2(mt_u32 data);

mt_u32 snd_reg_08H_get_volume_scale(void);
void snd_reg_08H_set_volume_scale(mt_u32 data);
mt_u32 snd_reg_08H_get_gainq(void);
void snd_reg_08H_set_gainq(mt_u32 data);
mt_u32 snd_reg_08H_get_gainq_manu_zero(void);
void snd_reg_08H_set_gainq_manu_zero(mt_u32 data);
mt_u32 snd_reg_08H_get_i2s_mute(void);
void snd_reg_08H_set_i2s_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_spdif_hdmi_mute(void);
void snd_reg_08H_set_spdif_hdmi_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_spdif_coax_mute(void);
void snd_reg_08H_set_spdif_coax_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_adac_2ch_mute(void);
void snd_reg_08H_set_adac_2ch_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_left_mute(void);
void snd_reg_08H_set_left_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_right_mute(void);
void snd_reg_08H_set_right_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_bass_mute(void);
void snd_reg_08H_set_bass_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_center_mute(void);
void snd_reg_08H_set_center_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_sl_mute(void);
void snd_reg_08H_set_sl_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_sr_mute(void);
void snd_reg_08H_set_sr_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_rsl_mute(void);
void snd_reg_08H_set_rsl_mute(mt_u32 data);
mt_u32 snd_reg_08H_get_rsr_mute(void);
void snd_reg_08H_set_rsr_mute(mt_u32 data);

mt_u32 snd_reg_0CH_get_src_en(void);
void snd_reg_0CH_set_src_en(mt_u32 data);
mt_u32 snd_reg_0CH_get_downmix_en(void);
void snd_reg_0CH_set_downmix_en(mt_u32 data);
mt_u32 snd_reg_0CH_get_agc_en(void);
void snd_reg_0CH_set_agc_en(mt_u32 data);
mt_u32 snd_reg_0CH_get_chan_mod_pcmbuf(void);
void snd_reg_0CH_set_chan_mod_pcmbuf(mt_u32 data);
mt_u32 snd_reg_0CH_get_ahbdata_en(void);
void snd_reg_0CH_set_ahbdata_en(mt_u32 data);
mt_u32 snd_reg_0CH_get_old_src_flag(void);
void snd_reg_0CH_set_old_src_flag(mt_u32 data);
mt_u32 snd_reg_0CH_get_dmx_hdmi_en(void);
void snd_reg_0CH_set_dmx_hdmi_en(mt_u32 data);
mt_u32 snd_reg_0CH_get_dmx_manu_close(void);
void snd_reg_0CH_set_dmx_manu_close(mt_u32 data);
mt_u32 snd_reg_0CH_get_chmode_manu_2ch(void);
void snd_reg_0CH_set_chmode_manu_2ch(mt_u32 data);
mt_u32 snd_reg_0CH_get_audio_lr_copy(void);
void snd_reg_0CH_set_audio_lr_copy(mt_u32 data);
mt_u32 snd_reg_0CH_get_adac_audio_lr_copy(void);
void snd_reg_0CH_set_adac_audio_lr_copy(mt_u32 data);

mt_u32 snd_reg_10H_get_clk_divider_factor(void);
void snd_reg_10H_set_clk_divider_factor(mt_u32 data);

mt_u32 snd_reg_14H_get_sample_rate_offset(void);
void snd_reg_14H_set_sample_rate_offset(mt_u32 data);

mt_u32 snd_reg_18H_get_audio_out_buf_base_address(void);
void snd_reg_18H_set_audio_out_buf_base_address(mt_u32 data);

mt_u32 snd_reg_1CH_get_audio_out_buf_length(void);
void snd_reg_1CH_set_audio_out_buf_length(mt_u32 data);

mt_u32 snd_reg_20H_get_audout_buf_full_thd(void);
void snd_reg_20H_set_audout_buf_full_thd(mt_u32 data);

mt_u32 snd_reg_24H_get_aud_out_buf_wr_len(void);
void snd_reg_24H_set_aud_out_buf_wr_len(mt_u32 data);
mt_u32 snd_reg_24H_get_aud_out_buf_wr_cmd(void);
void snd_reg_24H_set_aud_out_buf_wr_cmd(mt_u32 data);

mt_u32 snd_reg_28H_get_pp_buf_base(void);
void snd_reg_28H_set_pp_buf_base(mt_u32 data);
mt_u32 snd_reg_2cH_get_pp_buf_len(void);
void snd_reg_2cH_set_pp_buf_len(mt_u32 data);
mt_u32 snd_reg_30H_get_pp_buf_full_thd(void);
void snd_reg_30H_set_pp_buf_full_thd(mt_u32 data);
void snd_reg_34H_set_pp_buf_wr_cmd(mt_u32 idx, mt_u32 len);

mt_u32 snd_reg_38H_get_spd_buf_base_address(void);
void snd_reg_38H_set_spd_buf_base_address(mt_u32 data);
mt_u32 snd_reg_3CH_get_spd_buf_length(void);
void snd_reg_3CH_set_spd_buf_length(mt_u32 data);
mt_u32 snd_reg_40H_get_spd_buf_full_thd(void);
void snd_reg_40H_set_spd_buf_full_thd(mt_u32 data);
mt_u32 snd_reg_44H_get_spd_buf_wr_len(void);
void snd_reg_44H_set_spd_buf_wr_len(mt_u32 data);
mt_u32 snd_reg_44H_get_spd_buf_wr_cmd(void);
void snd_reg_44H_set_spd_buf_wr_cmd(mt_u32 data);

mt_u32 snd_reg_4CH_get_src_coef(void);
void snd_reg_4CH_set_src_coef(mt_u32 data);
mt_u32 snd_reg_4CH_get_coef_address(void);
void snd_reg_4CH_set_coef_address(mt_u32 data);
mt_u32 snd_reg_4CH_get_coef_wr(void);
void snd_reg_4CH_set_coef_wr(mt_u32 data);
void snd_reg_5CH_set_fader_en(mt_u32 data);
void snd_reg_5CH_set_fader_in(mt_u32 data);
void snd_reg_5CH_set_fader_step(mt_u32 data);
void snd_reg_5CH_set_fader_target_gain(mt_u32 data);
void snd_reg_5CH_set_fader_time_step(mt_u32 data);

void snd_reg_60h_set_coef_left_all(mt_u32 data);
void snd_reg_64h_set_coef_right_all(mt_u32 data);
void snd_reg_68h_set_coef_bass_all(mt_u32 data);
void snd_reg_6ch_set_coef_center_all(mt_u32 data);
void snd_reg_70h_set_coef_sl_all(mt_u32 data);
void snd_reg_74h_set_coef_sr_all(mt_u32 data);
void snd_reg_78h_set_coef_rsl_all(mt_u32 data);
void snd_reg_7ch_set_coef_rsr_all(mt_u32 data);

void snd_reg_80H_reset_pp_aout_buf(void);
void snd_reg_80H_reset_mix_buf(void);
void snd_reg_80H_reset_spd_buf(void);
mt_u32 snd_reg_80H_get_pp_aout_buf_reset_end(void);
mt_u32 snd_reg_80H_get_mix_buf_reset_end(void);
mt_u32 snd_reg_80H_get_spd_buf_reset_end(void);

mt_u32 snd_reg_88H_get_clk_divider_factor(void);
void snd_reg_88H_set_clk_divider_factor(mt_u32 data);

mt_u32 snd_reg_a8h_get_hdmi_vol_ctrl(void);
void snd_reg_a8h_set_hdmi_vol_ctrl(mt_u32 data);
mt_u32 snd_reg_a8h_get_adac_vol_ctrl(void);
void snd_reg_a8h_set_adac_vol_ctrl(mt_u32 data);

mt_u32 snd_reg_b0h_get_spdif_pcm_vol_ctrl(void);
void snd_reg_b0h_set_spdif_pcm_vol_ctrl(mt_u32 data);
mt_u32 snd_reg_b0h_get_mix_spdif_alpha(void);
void snd_reg_b0h_set_mix_spdif_alpha(mt_u32 data);

mt_u32 snd_reg_c0H_get_mix_buf_base_address(void);
void snd_reg_c0H_set_mix_buf_base_address(mt_u32 data);
mt_u32 snd_reg_c4H_get_mix_buf_length(void);
void snd_reg_c4H_set_mix_buf_length(mt_u32 data);
mt_u32 snd_reg_c8H_get_mix_buf_full_thd(void);
void snd_reg_c8H_set_mix_buf_full_thd(mt_u32 data);
mt_u32 snd_reg_ccH_get_mix_buf_wr_len(void);
void snd_reg_ccH_set_mix_buf_wr_len(mt_u32 data);
mt_u32 snd_reg_ccH_get_mix_buf_wr_cmd(void);
void snd_reg_ccH_set_mix_buf_wr_cmd(mt_u32 data);

mt_u32 snd_reg_d0h_get_mix_func_en(void);
void snd_reg_d0h_set_mix_func_en(mt_u32 data);
mt_u32 snd_reg_d0h_get_ssrc_enable(void);
void snd_reg_d0h_set_ssrc_enable(mt_u32 data);
mt_u32 snd_reg_d0h_get_ssrc_int(void);
void snd_reg_d0h_set_ssrc_int(mt_u32 data);
mt_u32 snd_reg_d0h_get_ssrc_frac(void);
void snd_reg_d0h_set_ssrc_frac(mt_u32 data);

mt_u32 snd_reg_d4h_get_mix_hdmi_alpha(void);
void snd_reg_d4h_set_mix_hdmi_alpha(mt_u32 data);
mt_u32 snd_reg_d4h_get_mix_adac_alpha(void);
void snd_reg_d4h_set_mix_adac_alpha(mt_u32 data);

mt_u32 snd_reg_DCH_get_pcm_play_stop(void);
void snd_reg_DCH_set_pcm_play_stop(mt_u32 data);

void snd_reg_20ch_set_pcm_buf_data_type(mt_u32 data);


#endif

