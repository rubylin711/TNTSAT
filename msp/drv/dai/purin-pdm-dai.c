/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 *
 * ALSA SoC Audio Layer - Panther PDM Controller driver
 *
 */
#include <linux/module.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "codec/pdm.h"
#include "purin-audio.h"
#include "purin-dma.h"
#include <linux/io.h>

//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define purin_fmt(fmt) "[%s: %d]: " fmt, __func__, __LINE__
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#endif

#if defined(CONFIG_PANTHER_SND_PDM_RX0)
#define ENABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(RX0_GCR, GCR_PDM_ENABLE, (GCR_PDM_ENABLE|GCR_I2S_ENABLE|GCR_PCM_ENABLE)); } while (0);

#define DISABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(RX0_GCR, 0, (GCR_PDM_ENABLE|GCR_I2S_ENABLE|GCR_PCM_ENABLE)); } while (0);
#else
#define ENABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(RX1_GCR, GCR_PDM_ENABLE, (GCR_PDM_ENABLE|GCR_I2S_ENABLE|GCR_PCM_ENABLE)); } while (0);

#define DISABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(RX1_GCR, 0, (GCR_PDM_ENABLE|GCR_I2S_ENABLE|GCR_PCM_ENABLE)); } while (0);
#endif

#define SYM4_PDM_PINMUX_REG 0xbf13c144
#define SYM4_PDM_CLKGEN_REG0 0xbf5d0120
#define SYM4_PDM_CLKGEN_REG2 0xbf5d0128

static int pdm_init(struct purin_device *dev)
{
    int ret = 0;
    u32 gcr_val = 0;
    u32 ccr_val = 0;
    u32 pdm_val = 0;
	u32 dly_val = 0;	
	void* reg = NULL;

	u32 val = 0;
	reg = ioremap(SYM4_PDM_PINMUX_REG, 0x100);
	
	val = readl((void *)reg);
	val &= ~(0xf);
	val |= 0x7;
	writel(val, (void *)reg);

	val = readl((void *)reg+4);
	val &= ~(0xf);
	val |= 0x4;
	writel(val, (void *)reg+4);

	val = readl((void *)reg+8);
	val &= ~(0xf);
	val |= 0x4;
	writel(val, (void *)reg+8);

	val = readl((void *)reg+0xc);
	val &= ~(0xf);
	val |= 0x6;
	writel(val, (void *)reg+0xc);

	iounmap(reg);	

#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    gcr_val = AIREG_READ32(RX0_GCR);
    ccr_val = AIREG_READ32(RX0_CCR);
#else
    gcr_val = AIREG_READ32(RX1_GCR);
    ccr_val = AIREG_READ32(RX1_CCR);
#endif
    pdm_val = AIREG_READ32(PDM_CTL);
	dly_val = AIREG_READ32(PDM_DLY);

    switch (dev->bit_depth) {
        case 16:
            CFGREG(gcr_val, GCR_CHANNEL_16BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_16BITS, CCR_BUS_MODE);
            CFGREG(pdm_val, PDM_BUS_16BITS, PDM_BUS_MODE);
            break;
        case 24:
#if defined(PDM_BYPASS_TEST)
        case 32:
#endif
            CFGREG(gcr_val, GCR_CHANNEL_32BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_24BITS, CCR_BUS_MODE);
            CFGREG(pdm_val, PDM_BUS_24BITS, PDM_BUS_MODE);
			CFGREG(dly_val, PDM3_DLY|PDM2_DLY|PDM1_DLY, PDM3_DLY|PDM2_DLY|PDM1_DLY);
            break;
        default:
            ret = -EINVAL;
            goto result;
    }

    switch (dev->chan_num) {
        case 2:
            CFGREG(gcr_val, GCR_CHANNEL_2, GCR_CHANNEL_NUM);
            break;
        case 4:
            CFGREG(gcr_val, GCR_CHANNEL_4, GCR_CHANNEL_NUM);
            break;
        case 8:
            CFGREG(gcr_val, GCR_CHANNEL_8, GCR_CHANNEL_NUM);
            break;
        default:
            ret = -EINVAL;
            goto result;
    }
    CFGREG(pdm_val, PDM_PINS, PDM_PINS);
	CFGREG(pdm_val, PDM_SYS_CLK, PDM_SYS_CLK);//66M AXI CLK don't set this
	CFGREG(ccr_val, CCR_BIT_ORDER_MSB_FIRST, CCR_BIT_ORDER_MSB_FIRST);
    CFGREG(ccr_val, CCR_CH0_ENABLE, CCR_CH0_ENABLE);
    CFGREG(ccr_val, CCR_CH1_ENABLE, CCR_CH1_ENABLE);

	AIREG_WRITE32(PDM_DLY, dly_val);
    AIREG_WRITE32(PDM_CTL, pdm_val);
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    AIREG_WRITE32(RX0_CCR, ccr_val);
    AIREG_WRITE32(RX0_GCR, gcr_val);
#else
    AIREG_WRITE32(RX1_CCR, ccr_val);
    AIREG_WRITE32(RX1_GCR, gcr_val);
#endif

result:
    return ret;
}

static int pdm_prepare(struct snd_pcm_substream *substream,
                       struct snd_soc_dai *dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    struct purin_device *device = &master->rx_dev;
    unsigned long irqflags;
    pr_debug("in function %s\n", __func__);

    spin_lock_irqsave(&(master->reg_lock), irqflags);
    purin_buf_specific(substream);
    ret = pdm_init(device);
    if (ret < 0) {
        printk(KERN_ERR "[%s:%d]FORMAT not Support\n", __func__, __LINE__);
        goto result;
    }

    /* unmask Interrupt */
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    AIREG_UPDATE32(INTR_CTL, (RX0_THD_INT_EN|RX0_FIFO_OV_INT_EN|RX0_RING_OV_INT_EN), (RX0_THD_INT_EN|RX0_FIFO_OV_INT_EN|RX0_RING_OV_INT_EN));
#else
    AIREG_UPDATE32(INTR_CTL, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN), (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
#endif

result:
    spin_unlock_irqrestore(&(master->reg_lock), irqflags);

    return ret;
}

static int pdm_trigger(struct snd_pcm_substream *substream, int cmd,
                struct snd_soc_dai *dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    struct purin_device *device = &master->rx_dev;
    unsigned long irqflags;
    pr_debug("in function %s\n", __func__);

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        spin_lock_irqsave(&(master->reg_lock), irqflags);
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
        ENABLE_RXDMA();
#else
        ENABLE_RX1DMA();
#endif
        ENABLE_INTERFACE();
        AIREG_UPDATE32(PDM_CTL, PDM_EN, PDM_EN);
        switch (device->chan_num) {
            case 2:
                AIREG_UPDATE32(PDM_CTL, (PDM1_EN), PDM_CH_EN);
                break;
            case 4:
                AIREG_UPDATE32(PDM_CTL, (PDM1_EN|PDM2_EN), PDM_CH_EN);
                break;
            case 8:
                AIREG_UPDATE32(PDM_CTL, (PDM1_EN|PDM2_EN|PDM3_EN), PDM_CH_EN);
                break;
            default:
                return-EINVAL;
        }
#if defined(CONFIG_PANTHER_SND_LPSD)
        AIREG_UPDATE32(LPSD_CTL, LPSD_EN, LPSD_EN);
#endif
        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        spin_lock_irqsave(&(master->reg_lock), irqflags);
#if defined(CONFIG_PANTHER_SND_LPSD)
        AIREG_UPDATE32(LPSD_CTL, 0, LPSD_EN);
#endif
        DISABLE_INTERFACE();
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
        DISABLE_RXDMA();
#else
        DISABLE_RX1DMA();
#endif
        AIREG_UPDATE32(PDM_CTL, 0, (PDM_CH_EN|PDM_EN));
        /* reset */
        AIREG_UPDATE32(PDM_CTL, PDM_RESET, PDM_RESET);
        AIREG_UPDATE32(PDM_CTL, 0, PDM_RESET);
        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
        break;
    default:
        ret = -EINVAL;
    }

    return ret;
}

static int pdm_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *cpu_dai)
{
    struct purin_master *master;
    struct purin_device *device;
    int ret = 0;
	void* reg = NULL;
	u32 val = 0;

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
    device->chan_num = params_channels(params);
    pr_debug("chan_num=%d\n", device->chan_num);
	device->fs_rate = params_rate(params);

	reg = ioremap(SYM4_PDM_CLKGEN_REG0, 0x100);	
	switch(device->fs_rate) {
		case 16000:
			val = 0;
			break;
		case 48000:
			val = 0x1000;
			break;
		default:
			pr_err("unsupported pdm sample rate:%d",device->fs_rate);
			val = 0;
			break;
	}
	writel(val, reg+8);
	iounmap(reg);

out:
    return ret;
}

static int pdm_startup(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    pr_debug("in function %s\n", __func__);
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    AIREG_UPDATE32(RX0_CCR, (CCR_CH0_ENABLE|CCR_CH1_ENABLE), (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
#else
    AIREG_UPDATE32(RX1_CCR, (CCR_CH0_ENABLE|CCR_CH1_ENABLE), (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
#endif
    return 0;
}

static void pdm_shutdown(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;

    spin_lock_irqsave(&(master->reg_lock), irqflags);

    pr_debug("shutdown %d\n", substream->stream);
    /* mask interrupt */
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    AIREG_UPDATE32(INTR_CTL, 0, (RX0_THD_INT_EN|RX0_FIFO_OV_INT_EN|RX0_RING_OV_INT_EN));
#else
    AIREG_UPDATE32(INTR_CTL, 0, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
#endif
    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
}

static const struct snd_soc_dai_ops pdm_dai_ops = {
    .prepare    = pdm_prepare,
    .trigger    = pdm_trigger,
    .hw_params  = pdm_hw_params,
    .startup    = pdm_startup,
    .shutdown   = pdm_shutdown,
};

static struct snd_soc_dai_driver purin_pdm_dai = {
    .capture = {
        .stream_name = "PDM Capture",
        .channels_min = 2,
        .channels_max = 8,
        .rates = PDM_RATES,
        .formats = PDM_FORMATS, /* must be subset in purin_pcm_hardware settings */
    },
    .ops = &pdm_dai_ops,
};

static const struct snd_soc_component_driver purin_pdm_component = {
    .name       = "pdm-dai",
};

static int pdm_probe(struct platform_device *pdev)
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

#if defined(CONFIG_PM)
    device_init_wakeup(dev, true);
#endif
    ret = devm_snd_soc_register_component(dev, &purin_pdm_component, &purin_pdm_dai, 1);
    if (unlikely(ret != 0)) {
        dev_err(dev, "failed to register dai\n");
        goto err_irq;
    }
    master->res = res;
    master->pdev = pdev;
    dev_set_drvdata(dev, (void *)master);

    return ret;

err_irq:
    if (master) {
        kfree(master);
        master = NULL;
    }
    return ret;
}

static int pdm_remove(struct platform_device *pdev)
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
#if defined(CONFIG_PM)
    device_init_wakeup(dev, false);
#endif
    return 0;
}

struct platform_device panther_pdm_device = {
    .name       = "purin-pdm",
    .id     = -1,
};

static struct platform_driver purin_pdm_driver = {
    .probe  = pdm_probe,
    .remove = pdm_remove,
    .driver = {
        .name   = "purin-pdm",
    },
};

static int __init purin_pdm_dai_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_pdm_device);
    if (ret != 0) {
        pr_err("Failed to register purin-pdm device: %d\n", ret);
        platform_device_put(&panther_pdm_device);
        return ret;
    }
    return platform_driver_register(&purin_pdm_driver);
}

static void __exit purin_pdm_dai_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&purin_pdm_driver);
}

module_init(purin_pdm_dai_init);
module_exit(purin_pdm_dai_exit);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Panther PDM Controller Driver");
MODULE_ALIAS("platform:panther-pdm");
