/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 *
 * ALSA SoC Audio Layer - Panther adc Controller driver
 *
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>

#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "codec/adc.h"
#include "purin-audio.h"
#include "purin-dma.h"

//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define purin_fmt(fmt) "[%s: %d]: " fmt, __func__, __LINE__
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#endif

#define ADC_IS_ENABLED (AIREG_READ32(ADC_CFG) & 0x00000001UL)

#define ENABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(ADC_CFG, 0x00000001UL, 0x00000001UL); } while (0);

#define DISABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(ADC_CFG, 0, 0x00000001UL); } while (0);

#define ANALOG_AO_REG 0xbf157004 
#define ANALOG_AO_VAL 0x4A0002
static int adc_prepare(struct snd_pcm_substream *substream,
                       struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;
	void* reg = NULL;
    pr_debug("in function %s\n", __func__);
    spin_lock_irqsave(&(master->reg_lock), irqflags);

    if (!ADC_IS_ENABLED) {
        pr_debug("in function %s\n", __func__);
        purin_buf_specific(substream);
        /* unmask Interrupt */
        AIREG_UPDATE32(INTR_CTL, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN), (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
    }

    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
    if (!ADC_IS_ENABLED) {
        pr_debug("in function %s\n", __func__);
    }

	reg = ioremap(ANALOG_AO_REG, 0x10);
	writel(ANALOG_AO_VAL, reg);
	iounmap(reg);

    return 0;
}

static int adc_trigger(struct snd_pcm_substream *substream, int cmd,
                struct snd_soc_dai *dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;
    pr_debug("in function %s\n", __func__);

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        spin_lock_irqsave(&(master->reg_lock), irqflags);
        /* adc is record only */
        if (!ADC_IS_ENABLED) {
            pr_debug("in function %s\n", __func__);
			ENABLE_RX1DMA();
            ENABLE_INTERFACE();
#if defined(CONFIG_PANTHER_SND_LPSD)
            AIREG_UPDATE32(LPSD_CTL, LPSD_EN, LPSD_EN);
#endif
        }
        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
#if defined(CONFIG_PANTHER_SND_LPSD)
        AIREG_UPDATE32(LPSD_CTL, 0, LPSD_EN);
#endif
        break;
    default:
        ret = -EINVAL;
    }

    return ret;
}

#define SYM4_CLKGEN_REG0 0xbf5d0098
#define SYM4_CLKGEN_REG1 0xbf5d0120
#define SYM4_CLKGEN_REG2 0xbf5d0128
static void purin_set_adc_mclk(int sample_rate)
{
	void* reg = NULL;
	u32 val = 0;

	reg = ioremap(SYM4_CLKGEN_REG0, 0x10);
	val = 0xf000281f;
	writel(val, reg);
	iounmap(reg);

	reg = ioremap(SYM4_CLKGEN_REG2, 0x10);
	val = 0x0;
	writel(val, reg);
	iounmap(reg);

	reg = ioremap(SYM4_CLKGEN_REG1, 0x10);
	val = readl(reg);
	val &= 0xffff0fff;
	switch (sample_rate) {
		case 16000:
			val |= 0x4000;
			break;

		case 32000:
			val |= 0x2000;
			break;

		case 48000:
			val |= 0x1000;
			break;

		case 64000:
			val |= 0x0;
			break;

		default:
			pr_err("sample rate %d is not supported yet!",sample_rate);
			break;
	}
	writel(val, reg);
	iounmap(reg);

	return;
}

static int adc_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *cpu_dai)
{
    struct purin_master *master;
    struct purin_device *device;
    int ret = 0;
    pr_debug("in function %s\n", __func__);

    master = snd_soc_dai_get_drvdata(cpu_dai);
    if (!master) {
        pr_debug("no master!!\n");
        ret = -ENODEV;
        goto out;
    }

    device = &master->rx_dev;
    device->bit_depth = snd_pcm_format_width(params_format(params));
    pr_debug("bit_depth=%d\n", device->bit_depth);
	purin_set_adc_mclk(params_rate(params));

#if 0
    if (params_rate(params) != 64000) { // only support 64k in ADC
        ret = -EINVAL;
        goto out;
    }
#endif

out:

    return ret;
}

static int adc_startup(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    pr_debug("in function %s\n", __func__);
    AIREG_UPDATE32(RX1_CCR, (CCR_CH0_ENABLE|CCR_CH1_ENABLE), (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
    return 0;
}

static void adc_shutdown(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;

    spin_lock_irqsave(&(master->reg_lock), irqflags);

    pr_debug("shutdown %d\n", substream->stream);
    /* mask interrupt */
    AIREG_UPDATE32(INTR_CTL, 0, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
    AIREG_UPDATE32(RX1_CCR, 0, (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
    DISABLE_INTERFACE();
    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
}

static const struct snd_soc_dai_ops adc_dai_ops = {
    .prepare    = adc_prepare,
    .trigger    = adc_trigger,
    .hw_params  = adc_hw_params,
    .startup    = adc_startup,
    .shutdown   = adc_shutdown,
};

#define ADC_DAI_SUPPORT_FORMATS (SNDRV_PCM_FMTBIT_S16_LE)

static struct snd_soc_dai_driver purin_adc_dai = {
    .capture = {
        .stream_name = "ADC Capture",
        .channels_min = 2,
        .channels_max = 2,
        .rates = ADC_RATES,
        .formats = ADC_DAI_SUPPORT_FORMATS, /* must be subset in purin_pcm_hardware settings */
    },
    .ops = &adc_dai_ops,
};

static const struct snd_soc_component_driver purin_adc_component = {
    .name       = "adc-dai",
};

static int adc_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;
    struct resource *res = NULL;
    struct purin_master *master = NULL;

    master = kzalloc(sizeof(struct purin_master), GFP_KERNEL);
    pr_debug("in function %s\n", __func__);
    if (NULL == master) {
        dev_err(dev, "Could not allocate master\n");
        ret = -ENOMEM;
        goto err_irq;
    }

    spin_lock_init(&(master->reg_lock));

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (unlikely(res < 0)) {
        dev_err(dev, "no memory specified\n");
        ret = -ENOENT;
        goto err_irq;
    }

    ret = devm_snd_soc_register_component(dev, &purin_adc_component, &purin_adc_dai, 1);
    if (unlikely(ret != 0)) {
        dev_err(dev, "failed to register dai\n");
        goto err_irq;
    }
    master->res = res;
    master->pdev = pdev;
    dev_set_drvdata(dev, (void *)master);

    return ret;

err_irq:
    if (master != NULL) {
        kfree(master);
        master = NULL;
    }
    return ret;
}

static int adc_remove(struct platform_device *pdev)
{
    struct device *dev;
    struct purin_master* master;

    dev=&pdev->dev;
    master = dev_get_drvdata(dev);
    pr_debug("in function %s\n", __func__);
    if (likely(master)) {
        kfree(master);
        master = NULL;
    }
    snd_soc_unregister_component(&pdev->dev);
    return 0;
}

struct platform_device panther_adc_device = {
    .name       = "purin-adc",
    .id     = -1,
};

static struct platform_driver purin_adc_driver = {
    .probe  = adc_probe,
    .remove = adc_remove,
    .driver = {
        .name   = "purin-adc",
    },
};

static int __init purin_adc_dai_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_adc_device);
    if (ret != 0) {
        pr_err("Failed to register purin-adc device: %d\n", ret);
        platform_device_put(&panther_adc_device);
        return ret;
    }

    return platform_driver_register(&purin_adc_driver);
}

static void __exit purin_adc_dai_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&purin_adc_driver);
}

module_init(purin_adc_dai_init);
module_exit(purin_adc_dai_exit);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Panther ADC Controller Driver");
MODULE_ALIAS("platform:panther-adc");
