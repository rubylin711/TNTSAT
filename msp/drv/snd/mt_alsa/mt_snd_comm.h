/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __ALSA_SND_HDMI_COMM_H__
#define __ALSA_SND_HDMI_COMM_H__

#include <sound/soc.h>
#include <sound/pcm.h>

#define MT_SND_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
	SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define MT_SND_FORMATS (SNDRV_PCM_FMTBIT_S16_LE)

#define REG_AUD_CH_SRT				0x00
#define REG_AUD_I2S_SPDIF			0x04
#define REG_AUD_VOL                 0x08
#define REG_AUD_PP_EN				0x0c
#define REG_AUD_CLK_DIV				0x10
#define REG_AUD_PCM_BUF_BASE 		0x18
#define REG_AUD_PCM_BUF_LEN			0x1c
#define REG_AUD_PCM_BUF_WRCMD		0x24
#define REG_AUD_PP_BUF_BASE 		0x28
#define REG_AUD_PP_ONE_BUF_LEN		0x2c
#define REG_AUD_PP_BUF_FUL_THD		0x30
#define REG_AUD_PP_BUF_WRCMD		0x34
#define REG_AUD_SRC					0x4c
#define REG_AUD_SAMPLE_NUM_FRM		0x90
#define REG_AUD_INTR_SET			0xa0
#define REG_AUD_PLAY_STOP			0xDC

#define REG_AUD_PCM_RADDR			0x100
#define REG_AUD_PP_RADDR			0x110

#define REG_AUD_PP_BUF0_CNT			0x114
#define REG_AUD_PP_BUF1_CNT			0x118
#define REG_AUD_PP_BUF2_CNT			0x150
#define REG_AUD_PP_BUF3_CNT			0x154
#define REG_AUD_PP_BUF4_CNT			0x158
#define REG_AUD_PP_BUF5_CNT			0x15c
#define REG_AUD_PP_BUF6_CNT			0x160
#define REG_AUD_PP_BUF7_CNT			0x164

#define REG_AOUT_CLKSEL				0xA504

struct mt_snd_sound_hwparams {
	unsigned int channels;		 /* channels */
	unsigned int rate;		        /* rate in Hz */
	snd_pcm_format_t format;	 /* SNDRV_PCM_FORMAT_* */
    unsigned int frame_size;
    unsigned int buffer_bytes;
	snd_pcm_uframes_t period_size;  /* period size */
	unsigned int periods;		          /* periods */
	snd_pcm_uframes_t buffer_size;  /* buffer size */
};

struct mt_soundaudio_data {
    struct mt_snd_sound_hwparams hwparam;
    struct mutex mutex;
	int snd_started;
	int snd_stopped;
	struct snd_dma_buffer *dmab;
};

extern ulong g_snd_base;
extern ulong g_aout_clk_base;
#define MT_SND_REG_WRITE32(addr, val)  (HAL_PUT_U32((volatile u32 *)(g_snd_base + addr), val))
#define MT_SND_REG_READ32(addr)  (HAL_GET_U32((volatile u32 *)(g_snd_base + addr)))
#define MT_AOUT_CLK_REG_READ32(addr) (HAL_GET_U32((volatile u32 *)(g_aout_clk_base + addr)))
#endif
