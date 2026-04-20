/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 *
 * ALSA SoC Audio Layer - Panther dac Controller driver
 *
 */

#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>

#include <sound/soc.h>
#include <sound/pcm_params.h>

#include "codec/dac.h"
#include "purin-audio.h"
#include "purin-dma.h"

//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define purin_fmt(fmt) "[%s: %d]: " fmt, __func__, __LINE__
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#endif
#define SAMPLE_RATE_SUPPORT (13)
#define MCLK_CLKSEL 0xBF50A504
#define DAI_CLKEN_REG 0xBF50A800

static int sample_rate_table[SAMPLE_RATE_SUPPORT] = {
    8000,  16000, 22050, 24000,  32000,
    44100, 48000, 88200, 96000, 128000, 176400, 192000
};

static int dac_rate_table[SAMPLE_RATE_SUPPORT] = {
    -1, -1, -1, -1, -1,
     2,  0, -1,  1, -1, -1, -1
};

#define ENABLE_INTERFACE()                                  \
    do {                                                    \
        if (!(AIREG_READ32(DAC_CFG) & 0x00000001UL))               \
            AIREG_UPDATE32(DAC_CFG, 0x00000001UL, 0x00000001UL);           \
    } while (0);

#define DISABLE_INTERFACE()                          \
    do { AIREG_UPDATE32(DAC_CFG, 0, 0x00000001UL); } while (0);

#define AOUT_CLKSEL_REG 0xBF50A504
#define AOUT_CLKSEL_VAL 0x5
static int dac_prepare(struct snd_pcm_substream *substream,
                       struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;
	//void* reg = NULL;
    spin_lock_irqsave(&(master->reg_lock), irqflags);
    purin_buf_specific(substream);

    /* unmask Interrupt */
    AIREG_UPDATE32(INTR_CTL, (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN), (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN));
    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
/*
	reg = ioremap(AOUT_CLKSEL_REG, 0x10);
	writel(AOUT_CLKSEL_VAL, reg);
	iounmap(reg);
*/
    return 0;
}

static int dac_trigger(struct snd_pcm_substream *substream, int cmd,
                struct snd_soc_dai *dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;
	void* reg = NULL;
	u32 val = 0;

	reg = ioremap(MCLK_CLKSEL, 0x10);
	val = readl(reg);
    switch (cmd) {
	    case SNDRV_PCM_TRIGGER_START:
	    case SNDRV_PCM_TRIGGER_RESUME:
	    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	        spin_lock_irqsave(&(master->reg_lock), irqflags);
	        /* dac is playback only */
	        ENABLE_TXDMA();
	        ENABLE_INTERFACE();
			writel(val|0x4, reg);
	        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
	        break;
	    case SNDRV_PCM_TRIGGER_STOP:
	    case SNDRV_PCM_TRIGGER_SUSPEND:
	    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	        spin_lock_irqsave(&(master->reg_lock), irqflags);
	        DISABLE_TXDMA();
			writel(val&(~0x4), reg);
	        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
	        break;
	    default:
	        ret = -EINVAL;
    }
	iounmap(reg);

    return ret;
}

static int purin_rate2idx(int rat)
{
    int i;
    for (i=0; i<SAMPLE_RATE_SUPPORT; i++) {
        if (rat == sample_rate_table[i]) {
            return i;
        }
    }
    return -1;
}

#define SYM4_CLKGEN_REG0 0xbf5d0098
#define SYM4_CLKGEN_REG1 0xbf5d0120
#define SYM4_CLKGEN_REG2 0xbf5d0128
static void purin_set_dac_mclk(int sample_rate)
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
	val &= 0x0fffffff;
    switch (sample_rate) {
        case 44100:
			val &= 0x00ffffff;
			val |= 0x82000000;
			break;

        case 48000:
			val |= 0x30000000;
			break;

		case 96000:
			val |= 0x10000000;
			break;

		default:
			pr_err("sample rate %d is not supported yet!",sample_rate);
			break;
    }
	writel(val, reg);
	iounmap(reg);

    return;
}

static int dac_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *cpu_dai)
{
    struct purin_master *master;
    struct purin_device *device;
    int idx = 0, ret = 0;
	void* reg = NULL;
    pr_debug("in function %s\n", __func__);

	reg = ioremap(DAI_CLKEN_REG, 0x10);
	writel(0x1, reg);
	iounmap(reg);

    master = snd_soc_dai_get_drvdata(cpu_dai);
    if (!master) {
        pr_debug("no master!!\n");
        ret = -ENODEV;
        goto out;
    }

    device = &master->tx_dev;
    device->bit_depth = snd_pcm_format_width(params_format(params));
    pr_debug("bit_depth=%d\n", device->bit_depth);
    idx = purin_rate2idx(params_rate(params));
    if (idx < 0 || dac_rate_table[idx] < 0) {
        printk(KERN_ERR "Not support sample rate\n");
        ret = -EINVAL;
        goto out;
    } else {
		purin_set_dac_mclk(params_rate(params));
	}
    pr_debug("rate:%d, idx:%d\n", params_rate(params), idx);

out:
    return ret;
}

static int dac_startup(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    pr_debug("in function %s\n", __func__);
    AIREG_UPDATE32(TX_CCR, (CCR_CH0_ENABLE|CCR_CH1_ENABLE), (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
    return 0;
}

static void dac_shutdown(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;

    spin_lock_irqsave(&(master->reg_lock), irqflags);

    pr_debug("shutdown %d\n", substream->stream);
    /* mask interrupt */
    AIREG_UPDATE32(INTR_CTL, 0, (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN));
    AIREG_UPDATE32(TX_CCR, 0, (CCR_CH0_ENABLE|CCR_CH1_ENABLE));
    DISABLE_INTERFACE();

    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
}

static const struct snd_soc_dai_ops dac_dai_ops = {
    .prepare    = dac_prepare,
    .trigger    = dac_trigger,
    .hw_params  = dac_hw_params,
    .startup    = dac_startup,
    .shutdown   = dac_shutdown,
};

#define DAC_DAI_SUPPORT_FORMATS (SNDRV_PCM_FMTBIT_S16_LE)

static struct snd_soc_dai_driver purin_dac_dai = {
    .playback = {
        .stream_name = "DAC Playback",
        .channels_min = 2,
        .channels_max = 2,
        .rates = DAC_RATES,
        .formats = DAC_DAI_SUPPORT_FORMATS, /* must be subset in purin_pcm_hardware settings */
    },
    .ops = &dac_dai_ops,
};

static const struct snd_soc_component_driver purin_dac_component = {
    .name       = "dac-dai",
};

static int dac_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;
    struct resource *res = NULL;
    struct purin_master *master = NULL;

    master = kzalloc(sizeof(struct purin_master), GFP_KERNEL);
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

    ret = devm_snd_soc_register_component(dev, &purin_dac_component, &purin_dac_dai, 1);
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

static int dac_remove(struct platform_device *pdev)
{
    struct device *dev;
    struct purin_master* master;

    dev=&pdev->dev;
    master = dev_get_drvdata(dev);

    if (likely(master)) {
        kfree(master);
        master = NULL;
    }
    snd_soc_unregister_component(&pdev->dev);
    return 0;
}

struct platform_device panther_dac_device = {
    .name       = "purin-dac",
    .id     = -1,
};

static struct platform_driver purin_dac_driver = {
    .probe  = dac_probe,
    .remove = dac_remove,
    .driver = {
        .name   = "purin-dac",
    },
};

static int __init purin_dac_dai_init(void)
{
	int ret = 0;

    ret = platform_device_register(&panther_dac_device);
    if (ret != 0) {
        pr_err("Failed to register purin-dac device: %d\n", ret);
        platform_device_put(&panther_dac_device);
        return ret;
    }

    ret = platform_driver_register(&purin_dac_driver);
	if (ret){
		pr_err("failed to register purin-dac driver!");
		return ret;
	}

	return ret;
}

static void __exit purin_dac_dai_exit(void)
{
    platform_driver_unregister(&purin_dac_driver);
}

module_init(purin_dac_dai_init);
module_exit(purin_dac_dai_exit);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Panther DAC Controller Driver");
MODULE_ALIAS("platform:panther-dac");
