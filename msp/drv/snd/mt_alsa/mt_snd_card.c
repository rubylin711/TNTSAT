/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/clk.h>
#include <linux/module.h>
#include <sound/soc.h>
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_snd_comm.h"
#include "mach/symphony_reg_base_addr.h"
#include "mt_mach/symphony_io.h"

ulong g_snd_base;
ulong g_aout_clk_base;

SND_SOC_DAILINK_DEF(mt_snd_dai,
		    DAILINK_COMP_ARRAY(COMP_CPU("mt-sound-dai")));

SND_SOC_DAILINK_DEF(mt_snd_codec,
		    DAILINK_COMP_ARRAY(COMP_CODEC("mt-sound-codec",
						  "mt-sound")));

SND_SOC_DAILINK_DEF(mt_snd_dma,
		    DAILINK_COMP_ARRAY(COMP_PLATFORM("mt-sound-dma")));

static int mt_sound_suspend_post(struct snd_soc_card *card)
{
	return 0;
}

static int mt_sound_resume_pre(struct snd_soc_card *card)
{
	return 0;
}

static int mt_codec_init(struct snd_soc_pcm_runtime *rtd)
{
    return 0;
}
static int mt_codec_sound_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
    return 0;
}

static struct snd_soc_ops mt_codec_ops = {
	.hw_params = mt_codec_sound_params,
};


static struct snd_soc_dai_link mt_sound_dai[] = {
    {
    	.name = "MONTAGE SOUND",
    	.stream_name = "Playback",		
    	.init = mt_codec_init,	
    	.ops = &mt_codec_ops,
    	SND_SOC_DAILINK_REG(mt_snd_dai, mt_snd_codec, mt_snd_dma),
    },
};

static struct snd_soc_card sound_snd_soc_mt = {
    .name = "MONTAGE-SOUND",
    .owner = THIS_MODULE,
    .dai_link = mt_sound_dai,
    .num_links =ARRAY_SIZE(mt_sound_dai),
    .dapm_widgets = NULL,
    .num_dapm_widgets = 0,
    .dapm_routes = NULL,
    .num_dapm_routes = 0,
    .controls = NULL,
    .num_controls = 0,
    .suspend_post = &mt_sound_suspend_post,
	.resume_pre = &mt_sound_resume_pre,
};

static __init int mt_sound_snd_dev_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &sound_snd_soc_mt;
	int ret;

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",
			ret);
	}

	return ret;
}

static __exit int mt_sound_snd_dev_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	return 0;
}

static struct platform_driver mt_sound_sndcard_driver = {
    	.driver = {
		.name = "mt-snd-card",
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
	},
	.probe  = mt_sound_snd_dev_probe,
	.remove = __exit_p(mt_sound_snd_dev_remove),
};

static struct platform_device *mt_sound_snd_device;

int mt_sound_init(void)
{
    int ret = 0;
    //register device
    mt_sound_snd_device = platform_device_alloc("mt-snd-card", -1);
    if (!mt_sound_snd_device) {
    	pr_err("mt-sndcard SoC snd device: Unable to register");
    	return -ENOMEM;
    }
    
    ret = platform_device_add(mt_sound_snd_device);
    if (ret) {
    	pr_err("mt-sndcard SoC snd device: Unable to add");
    	platform_device_put(mt_sound_snd_device);
       return ret;
    }

    //register drvier
    return platform_driver_register(&mt_sound_sndcard_driver);
}

void mt_sound_exit(void)
{
    platform_device_unregister(mt_sound_snd_device);
    platform_driver_unregister(&mt_sound_sndcard_driver);
}
extern int mt_snd_dma_init(void);
extern int mt_sound_i2s_init(void);
extern int mt_sound_codec_init(void);
extern void mt_sound_codec_deinit(void);
extern void mt_sound_i2s_deinit(void);
extern void mt_snd_dma_deinit(void);

int mt_sound_alsa_init(void)
{
    int ret;

	g_snd_base = mt_get_audioout_base();
	g_aout_clk_base = mt_get_clk_base();
    //step 1  register platform dma component
    ret = mt_snd_dma_init();
    if(ret) {
        return ret;
    }
	
    //step 2  register cpu dai component
    ret = mt_sound_i2s_init();
    if(ret) {
        goto  Err_I2s_Init;
    }
	
    //step 3 register codec component
    ret = mt_sound_codec_init();
    if(ret) {
        goto  Err_Codec_Init;
    }
	
    //step 4 register card link 
    ret = mt_sound_init();
    if(ret) {
        goto  Err_SndCard_Init;
    }
	
    return 0;

Err_SndCard_Init:
    mt_sound_codec_deinit();
Err_Codec_Init:
    mt_sound_i2s_deinit();
Err_I2s_Init:
    mt_snd_dma_deinit();

    return ret;
}

void mt_sound_alsa_exit(void)
{
    mt_sound_codec_deinit();
    mt_sound_i2s_deinit();
    mt_snd_dma_deinit();
    mt_sound_exit();
}

module_init(mt_sound_alsa_init);
module_exit(mt_sound_alsa_exit);

MODULE_AUTHOR("Montage");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MT SOUND ALSA driver");
