/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 *
 * ALSA SoC Audio Layer - Panther pdm codec driver
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <linux/of.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "pdm.h"
#define DRV_NAME "panther-pdm"

static const struct snd_soc_dapm_widget pdm_widgets[] = {
    SND_SOC_DAPM_OUTPUT("pdm-in"),
};

static const struct snd_soc_dapm_route pdm_routes[] = {
    { "pdm-in", NULL, "Capture" },
};

static int pdm_set_dai_sysclk(struct snd_soc_dai *cpu_dai,
                int clk_id, unsigned int freq, int dir)
{
    return 0;
}

static int pdm_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *socdai)
{
    return 0;
}

static int pdm_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    return 0;
}

static struct snd_soc_dai_ops pdm_dai_ops = {
    .hw_params  = pdm_hw_params,
    .set_fmt    = pdm_set_dai_fmt,
    .set_sysclk = pdm_set_dai_sysclk,
};

static struct snd_soc_codec_driver soc_codec_pdm = {
	.component_driver = {
    .dapm_widgets = pdm_widgets,
    .num_dapm_widgets = ARRAY_SIZE(pdm_widgets),
    .dapm_routes = pdm_routes,
    .num_dapm_routes = ARRAY_SIZE(pdm_routes),
	},
};

static struct snd_soc_dai_driver pdm_dai = {
    .name = "pdm-hifi",
    .capture = {
        .stream_name  = "Capture",
        .channels_min = 2,
        .channels_max = 8,
        .rates        = PDM_RATES,
        .formats      = PDM_FORMATS,
    },
    .ops = &pdm_dai_ops,
};

static int pdm_probe(struct platform_device *pdev)
{
    int ret = 0;

    ret = snd_soc_register_codec(&pdev->dev, &soc_codec_pdm,
            &pdm_dai, 1);
    return ret;
}

static int pdm_remove(struct platform_device *pdev)
{
    snd_soc_unregister_codec(&pdev->dev);
    return 0;
}

static struct platform_driver pdm_driver = {
    .probe      = pdm_probe,
    .remove     = pdm_remove,
    .driver     = {
        .name   = DRV_NAME,
        .owner  = THIS_MODULE,
    },
};

module_platform_driver(pdm_driver);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Panther PDM codec driver");
MODULE_ALIAS("platform:" DRV_NAME);
