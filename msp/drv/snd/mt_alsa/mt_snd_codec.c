/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include "mt_snd_comm.h"

static int mt_codec_dai_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	return 0;
}

static int mt_codec_dai_set_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	return 0;
}

static int mt_codec_dai_set_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static const struct snd_soc_dai_ops mt_codec_dai_ops = {
	.hw_params	= mt_codec_dai_params,
	.set_fmt	= mt_codec_dai_set_fmt,
	.set_sysclk	= mt_codec_dai_set_sysclk,
};

static struct snd_soc_dai_driver mt_codec_dai = {
	.name = "mt-sound",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 8,
		.rates = MT_SND_RATES,
		.formats = MT_SND_FORMATS,
	},
	.ops = &mt_codec_dai_ops,
};

static int mt_sound_codec_probe(struct snd_soc_component *codec)
{   
    return 0;
}
static void mt_sound_codec_remove(struct snd_soc_component *codec)
{
    return;
}
static int mt_sound_codec_suspend(struct snd_soc_component *codec)
{
	return 0;
}
static int mt_sound_codec_resume(struct snd_soc_component *codec)
{
	return 0;
}
static int mt_sound_codec_set_bias_level(struct snd_soc_component *codec,
				 enum snd_soc_bias_level level)
{
	return 0;
}

static struct snd_soc_component_driver soc_codec_dev_mt = {

	.probe =	mt_sound_codec_probe,
	.remove =	mt_sound_codec_remove,
	.suspend =	mt_sound_codec_suspend,
	.resume =	mt_sound_codec_resume,
	.set_bias_level = mt_sound_codec_set_bias_level,
};

static int __init mt_codec_probe(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev,
			&soc_codec_dev_mt, &mt_codec_dai, 1);
}

static struct platform_driver mt_sound_codec_driver = {
	.driver = {
		.name = "mt-sound-codec",
		.owner = THIS_MODULE,
	},
	.probe = mt_codec_probe,
};

static struct platform_device *mt_sound_codec_device;

int mt_sound_codec_init(void)
{
    int ret = 0;
    
    //register device
    mt_sound_codec_device = platform_device_alloc("mt-sound-codec", -1);
    if (!mt_sound_codec_device) {
    	return -ENOMEM;
    }
	
    ret = platform_device_add(mt_sound_codec_device);
    if (ret) {
    	platform_device_put(mt_sound_codec_device);
       return ret;
    }

    //register drvier
    return platform_driver_register(&mt_sound_codec_driver);
}

void mt_sound_codec_deinit(void)
{
    platform_device_unregister(mt_sound_codec_device);
    platform_driver_unregister(&mt_sound_codec_driver);
}


