/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include "mt_type.h"
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt_snd_comm.h"
#include "mt_unf_misc.h"
#include "mach/symphony_reg_base_addr.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_mmz.h"
#include "mt_common.h"

#define AUDIO_VOLUME_MAX 150
#define AUDIO_VOLUME_MAX_SAFE 100
#define AUD_VOLUME_STEP_COEF_100_150_50_ARIA_INT  1086
#define AUDIO_VOLUME_MAX_VALUE ((1 << 21) - 1)
#define AUDIO_VOLUME_MAX_VALUE_SAFE 0x7fff
#define AUD_VOLUME_STEP_COEF_1_100_99_ARIA_INT  137999
#define AUD_VOLUME_EF 0x20000

extern unsigned int aout_size;
extern mmz_buffer_s aout_buf;

extern const int srcflt_coef[256];
extern int crm_module_reset(mt_u32 m_id);
extern int crm_module_clk_set(mt_u32 m_id, mt_u32 type);
extern int crm_module_clk_enable(mt_u32 mod_id, MT_BOOL onoff);

static int samplerate_2_index(int sample)
{
    int index = 0;

    if(sample <= 8000)
      index = 0xa;
    else if(sample <= 11025)
      index = 0x8;
    else if(sample <= 12000)
      index = 0x9;
    else if(sample <= 16000)
      index = 0x2;
    else if(sample <= 22050)
      index = 0x0;
    else if(sample <= 24000)
      index = 0x1;
    else if(sample <= 32000)
      index = 0x6;
    else if(sample <= 44100)
      index = 0x4;
    else if(sample <= 48000)
      index = 0x5;
    else if(sample <= 64000)
      index = 0xe;
    else if(sample <= 88200)
      index = 0xc;
    else
      index = 0xd;
   return index;
}

static void mt_snd_get_volume_gainq(unsigned char *p_gainq, unsigned int *p_volume, unsigned char percent_l)
{
	unsigned char gainQ = 0;
	unsigned int volume = 0;
	unsigned char step;

	// percent_l = 0; vol is 0,do nothing; else do something
	if(percent_l) {
		//100~150
		if(percent_l > AUDIO_VOLUME_MAX_SAFE) {
			volume = AUDIO_VOLUME_MAX_VALUE;
			step = AUDIO_VOLUME_MAX - percent_l;

			while(step--) {
			volume *= 1000;
			volume /= AUD_VOLUME_STEP_COEF_100_150_50_ARIA_INT;
		}
	} else { //1~100
		volume = AUDIO_VOLUME_MAX_VALUE_SAFE;
		step = AUDIO_VOLUME_MAX_SAFE - percent_l;

		while(step--) {
			volume *= AUD_VOLUME_EF;
			volume /= AUD_VOLUME_STEP_COEF_1_100_99_ARIA_INT;
		}
	}
		//get gainq and volume
		while(volume > 0xffff) {
			gainQ++;
			volume >>= 1;
		}
	}

	*p_gainq = gainQ;
	*p_volume = (unsigned int) volume;
}

static int mt_snd_set_volume(int volume)
{
	unsigned char gainq = 0;
	unsigned int scale = 0;

	if (volume == 0) {
		MT_SND_REG_WRITE32(REG_AUD_VOL, 0);
	} else {
		if( (volume > AUDIO_VOLUME_MAX_SAFE))
			volume = AUDIO_VOLUME_MAX_SAFE;

		mt_snd_get_volume_gainq(&gainq, &scale, volume);
		MT_SND_REG_WRITE32(REG_AUD_VOL, gainq<<16|scale);
	}
	udelay(50);		//-OK

	return 0;
}

static int mt_snd_dai_set_fmt(struct snd_soc_dai *cpu_dai,
				   unsigned int fmt)
{
	return 0;
}
static int mt_snd_dai_set_clkdiv(struct snd_soc_dai *cpu_dai,
				int div_id, int div)
{
	return 0;
}

static u32 clc_aout_div(u32 sample_rate)
{
	u32 aout_clksel = 0, tmp = 0, n = 10;
	u64 div = 0;
	u64 div_tmp;
	u64 pll_no_div_freq = 2880000000; //must u64, 2880M
	u32 u32_pll_no_div_freq = (u32)pll_no_div_freq;

	aout_clksel = (MT_AOUT_CLK_REG_READ32(REG_AOUT_CLKSEL) & 0x3);
	div = sample_rate;
	div <<= 33;	// sample_rate * 2^25 * 256

	if (0 == aout_clksel)	// 288M
	{
		n = 10;
	}
	else if (1 == aout_clksel)	// 262M
	{
		n = 11;
	}
	else if (2 == aout_clksel)	// 206M
	{
		n = 14;
	}
	else if (3 == aout_clksel)	// 131M
	{
		n = 22;
	}

	div *= n;
	div_tmp = div;
	do_div(div_tmp, u32_pll_no_div_freq);
	tmp = div_tmp;

	/* The value of tmp affects the output precision of AO, so next checking
	 * is import, tmp maybe not the best value, maybe tmp + 1.
	 */
	if ((div - pll_no_div_freq*tmp) > (pll_no_div_freq*(tmp + 1) - div))
	{
		tmp += 1;
	}

	return tmp;
}

static int mt_snd_dai_prepare(struct snd_pcm_substream *substream,
                                           struct snd_soc_dai *dai)
{
	int i = 0;
	int val = 0;
	int data = 0;
	int dma_len = 0;
	int rate_index = 0;
	int aud_frm_int = 0;
	unsigned char ch_mode[9] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
    struct snd_pcm_runtime *runtime = substream->runtime;

	MT_SND_REG_WRITE32(REG_AUD_CLK_DIV, clc_aout_div(runtime->rate));

	dma_len = runtime->dma_bytes;// / runtime->channels;
	MT_SND_REG_WRITE32(REG_AUD_PCM_BUF_BASE, ((u32)aout_buf.startPhyAddr)>>3);
	MT_SND_REG_WRITE32(REG_AUD_PCM_BUF_LEN, aout_size>>3);

	rate_index = samplerate_2_index(runtime->rate);
	val = rate_index | (ch_mode[runtime->channels] << 8);
	MT_SND_REG_WRITE32(REG_AUD_CH_SRT, val);

	for(i = 0;i < 256;i++) {
		data = srcflt_coef[i] & 0x001fffff; //low 21bits
		data |= 0x80000000;  //wr_coef_en
		data |= i << 21;       //wr_coef_addr
		MT_SND_REG_WRITE32(REG_AUD_SRC, data);
	}

	//set volume
	mt_snd_set_volume(100);

	//set aout buffer channel mode
	if(runtime->channels > 2)
		MT_SND_REG_WRITE32(REG_AUD_PP_EN, 0x0);
	else
		MT_SND_REG_WRITE32(REG_AUD_PP_EN, 0x8);

	//set sample num per frame
	MT_SND_REG_WRITE32(REG_AUD_SAMPLE_NUM_FRM, runtime->period_size);
	val = MT_SND_REG_READ32(REG_AUD_I2S_SPDIF);
	val = 0x43305254;//fixed value for test
	MT_SND_REG_WRITE32(REG_AUD_I2S_SPDIF, val);

	//enable aud frm interrupt
	aud_frm_int = MT_SND_REG_READ32(REG_AUD_INTR_SET);
	aud_frm_int |= 0x200000;
	aud_frm_int &= 0xffffdfff;
	MT_SND_REG_WRITE32(REG_AUD_INTR_SET, aud_frm_int);

	MT_SND_REG_WRITE32(REG_AUD_PLAY_STOP, 1);

	return 0;
}

static void mt_snd_dai_shutdown(struct snd_pcm_substream *substream,
                                           struct snd_soc_dai *dai)
{
	int aud_frm_int = 0;

	aud_frm_int = MT_SND_REG_READ32(REG_AUD_INTR_SET);
	aud_frm_int &= ~0x1;
	MT_SND_REG_WRITE32(REG_AUD_INTR_SET, aud_frm_int);	

	//crm_module_reset(HAL_AUDIO_OUT);
	
}

static int mt_snd_dai_set_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{ 
	struct snd_pcm_runtime *runtime = substream->runtime;

	//crm_module_reset(HAL_AUDIO_OUT);
    if(0x8 == runtime->channels){//7.1 channel
		//crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_288M);
    }else{//5.1 channel and others
		//crm_module_clk_set(HAL_AUDIO_OUT, AOUT_CLKSEL_262M);
    }
	//crm_module_clk_enable(HAL_AUDIO_OUT, 1);

	return 0;
}

static const struct snd_soc_dai_ops mt_hw_sound_dai_ops = {
	.startup	= NULL,
	.shutdown	= mt_snd_dai_shutdown,
	.prepare	= mt_snd_dai_prepare,
	.trigger	= NULL,
	.hw_params	= mt_snd_dai_set_hw_params,
	.set_fmt	= mt_snd_dai_set_fmt,
	.set_clkdiv	= mt_snd_dai_set_clkdiv,
};

static struct snd_soc_dai_driver mt_hw_sound_dai = {
	.name = "montage-i2s",
	.playback = {
		.channels_min = 1,
		.channels_max = 8,
		.rates = MT_SND_RATES,
		.formats = MT_SND_FORMATS,
	},
	.ops = &mt_hw_sound_dai_ops,
};

static const struct snd_soc_component_driver mt_sound_i2s_component = {
	.name		= "mt-sound-dai",
};

static __init int mt_hw_sound_dev_probe(struct platform_device *pdev)
{
	return snd_soc_register_component(&pdev->dev, &mt_sound_i2s_component,
					 &mt_hw_sound_dai, 1);
}

static __exit int mt_hw_sound_dev_remove(struct platform_device *pdev)
{
    snd_soc_unregister_component(&pdev->dev);
    return 0;
}

static struct platform_driver mt_hw_sound_driver = {
    	.driver = {
		.name = "mt-sound-dai",
		.owner = THIS_MODULE,
	},
	.probe  = mt_hw_sound_dev_probe,
	.remove = __exit_p(mt_hw_sound_dev_remove),
};

static struct platform_device *mt_hw_sound_device;

int mt_sound_i2s_init(void)
{
    int ret = 0;
    //register device
    mt_hw_sound_device = platform_device_alloc("mt-sound-dai", -1);
    if (!mt_hw_sound_device) {
    	return -ENOMEM;
    }
    
    ret = platform_device_add(mt_hw_sound_device);
    if (ret) {
    	platform_device_put(mt_hw_sound_device);
       return ret;
    }

    //register drvier
    return platform_driver_register(&mt_hw_sound_driver);
}

void mt_sound_i2s_deinit(void)
{
    platform_device_unregister(mt_hw_sound_device);
    platform_driver_unregister(&mt_hw_sound_driver);
}

