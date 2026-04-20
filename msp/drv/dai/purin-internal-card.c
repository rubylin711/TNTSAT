/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 *
 * ALSA SoC Audio Layer - Panther S/PDIF soc card driver
 *
 */
#include <linux/clk.h>
#include <linux/module.h>
#include <sound/soc.h>
#include "purin-dma.h"

struct internal_audio_data {
    struct platform_device *codec_device[4];
};

static struct snd_soc_dai_link purin_internal_dai[] = {
    {
    .name = "DAC",
    .stream_name = "DAC Playback",
    .platform_name = "purin-internal-dma", // declared in purin-dma.c
    .cpu_dai_name = "purin-dac",      // declared in platform.c
    .codec_dai_name = "dac-hifi",     // declared in codec/dac.c
    .codec_name = "panther-dac",      // declared in codec/dac.c & purin_dac_init (platform_device)
    },
//#if defined(CONFIG_PANTHER_SND_PDM_RX1)
    {
    .name = "PDM",
    .stream_name = "PDM Capture",
    .platform_name = "purin-internal-dma", // declared in purin-dma.c，platform.c
    .cpu_dai_name = "purin-pdm",      // declared in purin-pdm-dai.c, platform.c
    .codec_dai_name = "pdm-hifi",     // declared in codec/pdm.c
    .codec_name = "panther-pdm",      // declared in codec/pdm.c
    },
//#else
    {
    .name = "ADC",
    .stream_name = "ADC Capture",
    .platform_name = "purin-internal-dma", // declared in purin-dma.c
    .cpu_dai_name = "purin-adc",      // declared in platform.c
    .codec_dai_name = "adc-hifi",     // declared in codec/adc.c
    .codec_name = "panther-adc",      // declared in codec/adc.c & purin_adc_init (platform_device)
    },
//#endif
};

static struct snd_soc_card purin_internal_card = {
    .name = "internal",
    .owner = THIS_MODULE,
    .dai_link = purin_internal_dai,
    .num_links = ARRAY_SIZE(purin_internal_dai),
};

static int purin_internal_probe(struct platform_device *op)
{
    struct snd_soc_card *card = &purin_internal_card;
    struct internal_audio_data *pdata;
    int ret;

    pdata = devm_kzalloc(&op->dev, sizeof(struct internal_audio_data),
                         GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    card->dev = &op->dev;
    pdata->codec_device[0] = NULL;
    pdata->codec_device[1] = NULL;

    /* dac */
    pdata->codec_device[0] = platform_device_alloc("panther-dac", -1);
    if (!pdata->codec_device[0]) {
        dev_err(&op->dev, "platform_device_alloc() failed\n");
    } else {
        ret = platform_device_add(pdata->codec_device[0]);
        if (ret) {
            platform_device_put(pdata->codec_device[0]);
            pdata->codec_device[0] = NULL;
            dev_err(&op->dev, "platform_device_add() failed: %d\n", ret);
        }
    }

    /* pdm */
    pdata->codec_device[1] = platform_device_alloc("panther-pdm", -1);
    /* adc */
    pdata->codec_device[2] = platform_device_alloc("panther-adc", -1);

    if (!pdata->codec_device[1]) {
        dev_err(&op->dev, "platform_device_alloc() failed\n");
    } else {
        ret = platform_device_add(pdata->codec_device[1]);
        if (ret) {
            platform_device_put(pdata->codec_device[1]);
            pdata->codec_device[1] = NULL;
            dev_err(&op->dev, "platform_device_add() failed: %d\n", ret);
        }
    }

    if (!pdata->codec_device[2]) {
        dev_err(&op->dev, "platform_device_alloc() failed\n");
    } else {
        ret = platform_device_add(pdata->codec_device[2]);
        if (ret) {
            platform_device_put(pdata->codec_device[2]);
            pdata->codec_device[2] = NULL;
            dev_err(&op->dev, "platform_device_add() failed: %d\n", ret);
        }
    }

    ret = snd_soc_register_card(card);
    if (ret)
        dev_err(&op->dev, "snd_soc_register_card() failed: %d\n", ret);

    snd_soc_card_set_drvdata(card, pdata);
    return ret;
}

static int purin_internal_remove(struct platform_device *op)
{
    struct snd_soc_card *card = platform_get_drvdata(op);
    struct internal_audio_data *pdata = snd_soc_card_get_drvdata(card);
    int ret;

    ret = snd_soc_unregister_card(card);
    if (pdata->codec_device[0])
        platform_device_unregister(pdata->codec_device[0]);
    if (pdata->codec_device[1])
        platform_device_unregister(pdata->codec_device[1]);

    if (likely(pdata)) {
        kfree(pdata);
    }

    return ret;
}

struct platform_device panther_internal_device = {
    .name       = "purin-internal",
    .id     = -1,
};

static struct platform_driver purin_internal_driver = {
    .probe    = purin_internal_probe,
    .remove   = purin_internal_remove,
    .driver   = {
        .name    = "purin-internal",
        .owner   = THIS_MODULE,
        //.pm     = &snd_soc_pm_ops,
    },
};

static int __init purin_internal_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_internal_device);
    if (ret != 0) {
        pr_err("Failed to register panther-internal device: %d\n", ret);
        platform_device_put(&panther_internal_device);
        return ret;
    }

    return platform_driver_register(&purin_internal_driver);
}

static void __exit purin_internal_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&purin_internal_driver);
}

//module_platform_driver(purin_internal_driver);
module_init(purin_internal_init);
module_exit(purin_internal_exit);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Internel Card driver");
