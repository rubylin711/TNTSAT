/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 * Copyright (c) 2017 BayLibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * The full GNU General Public License is included in this distribution
 * in the file called COPYING.
 */

#include <linux/module.h>
#include <sound/soc.h>
#include <linux/io.h>

/*
 * The everest 7134 is a very simple DA converter with no register
 */

static int es7134_set_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	fmt &= (SND_SOC_DAIFMT_FORMAT_MASK | SND_SOC_DAIFMT_INV_MASK |
		SND_SOC_DAIFMT_MASTER_MASK);

	if (fmt != (SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		    SND_SOC_DAIFMT_CBS_CFS)) {
		dev_err(codec_dai->dev, "Invalid DAI format\n");
		return -EINVAL;
	}

	return 0;
}

static int es7134_digital_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int es7134_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	return 0;
}

static int es7134_set_dai_sysclk(struct snd_soc_dai *cpu_dai,
			     int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static const struct snd_soc_dai_ops es7134_dai_ops = {
	.set_fmt	= es7134_set_fmt,
	.hw_params	= es7134_hw_params,
	.digital_mute	= es7134_digital_mute,
	.set_sysclk = es7134_set_dai_sysclk,
};

static struct snd_soc_dai_driver es7134_dai = {
	.name = "es7134-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000,
		.formats = (SNDRV_PCM_FMTBIT_S16_LE  |
			    SNDRV_PCM_FMTBIT_S18_3LE |
			    SNDRV_PCM_FMTBIT_S20_3LE |
			    SNDRV_PCM_FMTBIT_S24_3LE |
			    SNDRV_PCM_FMTBIT_S24_LE),
	},
	.ops = &es7134_dai_ops,
};

static const struct snd_soc_dapm_widget es7134_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("AOUTL"),
	SND_SOC_DAPM_OUTPUT("AOUTR"),
	SND_SOC_DAPM_DAC("DAC", "Playback", SND_SOC_NOPM, 0, 0),
};

static const struct snd_soc_dapm_route es7134_dapm_routes[] = {
	{ "AOUTL", NULL, "DAC" },
	{ "AOUTR", NULL, "DAC" },
};

static const struct snd_soc_codec_driver es7134_codec_driver = {
	.component_driver = {
		.dapm_widgets		= es7134_dapm_widgets,
		.num_dapm_widgets	= ARRAY_SIZE(es7134_dapm_widgets),
		.dapm_routes		= es7134_dapm_routes,
		.num_dapm_routes	= ARRAY_SIZE(es7134_dapm_routes),
	},
};

#define SYM4_DAI_TX_PINMUX_REG 0xbf13c08C

static void sym4_dai_tx_pinmux_cfg(void)
{
    void* reg = NULL;
	u32 idx = 0, val = 0;
            /* pinmux set */
	reg = ioremap(SYM4_DAI_TX_PINMUX_REG, 0x20);
    for(idx = 0; idx < 2; idx++) {
        val = readl(reg);
        val &= ~(0x7);
        val |= 0x5;
        writel(val, reg);
        reg += 4;
    }
	for(idx = 0; idx < 2; idx++) {
        val = readl(reg);
        val &= ~(0x7);
        val |= 0x4;
        writel(val, reg);
        reg += 4;
    }
    iounmap(reg);		
}



static int es7134_probe(struct platform_device *pdev)
{
	sym4_dai_tx_pinmux_cfg();
	return snd_soc_register_codec(&pdev->dev,
				      &es7134_codec_driver,
				      &es7134_dai, 1);
}

static int es7134_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id es7134_ids[] = {
	{ .compatible = "everest,es7134", },
	{ .compatible = "everest,es7144", },
	{ }
};
MODULE_DEVICE_TABLE(of, es7134_ids);
#endif

static struct platform_driver es7134_driver = {
	.driver = {
		.name = "es7134",
		.of_match_table = of_match_ptr(es7134_ids),
	},
	.probe = es7134_probe,
	.remove = es7134_remove,
};

module_platform_driver(es7134_driver);

MODULE_DESCRIPTION("ASoC ES7134 audio codec driver");
MODULE_AUTHOR("Jerome Brunet <jbrunet@baylibre.com>");
MODULE_LICENSE("GPL");
