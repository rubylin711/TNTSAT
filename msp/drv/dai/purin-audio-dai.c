/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
/*
 * ALSA SoC DAI driver
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include "purin-dma.h"
#include "purin-audio.h"

extern int snd_procfs_init(void);

//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define purin_fmt(fmt) "[%s: %d]: " fmt, __func__, __LINE__
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#endif

#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
int shared_bclk_div = 0;       // bclk_div = system clock rate / bit clock rate
#endif
#if defined(CONFIG_PANTHER_SND_FS_SHARED)
int shared_chan_num = 0;
int shared_fs_rate = 0;
int shared_bit_depth = 0;
#endif
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

static int purin_device_init(struct purin_device *dev)
{
    int ret = 0;
    unsigned long gcr_reg = (dev->stream == SNDRV_PCM_STREAM_PLAYBACK) ? TX_GCR : RX1_GCR;
    unsigned long ccr_reg = (dev->stream == SNDRV_PCM_STREAM_PLAYBACK) ? TX_CCR : RX1_CCR;
    unsigned long gcr_val = AIREG_READ32(gcr_reg);
    unsigned long ccr_val = AIREG_READ32(ccr_reg);

    /* common configuration */
    if (dev->ctrl_mode == 0)
        CFGREG(gcr_val, GCR_CTRL_MODE_MASTER, GCR_CTRL_MODE);
    else
        CFGREG(gcr_val, GCR_CTRL_MODE_SLAVE, GCR_CTRL_MODE);

    /* edge configuration */
    if (dev->edge_invert) {
        CFGREG(gcr_val, GCR_TX_XMIT_CLK_EDGE_RISING, GCR_TX_XMIT_CLK_EDGE);
        CFGREG(gcr_val, GCR_RX_SAMPLE_CLK_EDGE_FALLING, GCR_RX_SAMPLE_CLK_EDGE);
    } else {
        CFGREG(gcr_val, GCR_TX_XMIT_CLK_EDGE_FALLING, GCR_TX_XMIT_CLK_EDGE);
        CFGREG(gcr_val, GCR_RX_SAMPLE_CLK_EDGE_RISING, GCR_RX_SAMPLE_CLK_EDGE);
    }
    if (SND_SOC_DAIFMT_LEFT_J == dev->format || (SND_SOC_DAIFMT_DSP_B == dev->format))
        CFGREG(gcr_val, GCR_LJ_MODE, GCR_LJ_MODE);

    CFGREG(gcr_val, ilog2(dev->bclk_div)<<20, GCR_BCLK);
    CFGREG(gcr_val, ilog2(dev->chan_num)<<12, GCR_CHANNEL_NUM);
    CFGREG(gcr_val, 0, GCR_FSYNC_SHARE);

	switch(dev->fs_rate) {
		case 16000:
			break;

		case 22050:
			break;

		default:
			break;
	}

    switch (dev->bit_depth) {
        case 8:
            CFGREG(gcr_val, GCR_CHANNEL_16BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_8BITS, CCR_BUS_MODE);
            break;
        case 16:
            CFGREG(gcr_val, GCR_CHANNEL_16BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_16BITS, CCR_BUS_MODE);
            break;
        case 24:
            CFGREG(gcr_val, GCR_CHANNEL_32BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_24BITS, CCR_BUS_MODE);
            break;
        case 32:
            CFGREG(gcr_val, GCR_CHANNEL_32BITS, GCR_CHANNEL_BITS);
            CFGREG(ccr_val, CCR_BUS_MODE_32BITS, CCR_BUS_MODE);
            break;
        default:
            ret = -EINVAL;
            goto result;
    }

#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
    if (dev->stream == SNDRV_PCM_STREAM_CAPTURE) {
        if (!(AIREG_READ32(TX_GCR) & GCR_ENABLE)) { // Tx bclk enable not yet
            AIREG_WRITE32(TX_GCR, gcr_val);
            AIREG_WRITE32(TX_CCR, ccr_val);
        }
        CFGREG(gcr_val, GCR_BCLK_FROM_TX, GCR_BCLK);
        pr_debug("Rx share Tx's bclk\n");
    }
# if defined(CONFIG_PANTHER_SND_INTERNAL_LOOPBACK)
    AIREG_UPDATE32(GLOBAL_DEBUG_REG, DAI_INTERNAL_LOOPBACK, DAI_INTERNAL_LOOPBACK);
# endif
#endif
#if defined(CONFIG_PANTHER_SND_FS_SHARED)
    if (dev->stream == SNDRV_PCM_STREAM_CAPTURE) {
        CFGREG(gcr_val, GCR_FSYNC_SHARE, GCR_FSYNC_SHARE);
        pr_debug("Rx share Tx's fs\n");
    }
#endif

    AIREG_WRITE32(gcr_reg, gcr_val);
    AIREG_WRITE32(ccr_reg, ccr_val);
    pr_debug("dump PCM register : CFG %x, CHctl %x\n", AIREG_READ32(gcr_reg), AIREG_READ32(ccr_reg));
result:
    return ret;
}

/*
 * this funtion is called by snd_soc_dai_set_fmt
 * (from purin_hw_params in purin-external-card.c)
 */
static int purin_dai_set_dai_fmt(struct snd_soc_dai *cpu_dai,
        unsigned int fmt)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(cpu_dai);
    struct purin_device *device = cpu_dai->id ? &master->rx_dev : &master->tx_dev;

    pr_debug("name=%s fmt=%d\n", cpu_dai->name, fmt);

    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
    case SND_SOC_DAIFMT_CBS_CFS:    // codec is slave, panther is master
        device->ctrl_mode = 0;
        break;
    case SND_SOC_DAIFMT_CBM_CFM:    // codec is master, panther is slave
        device->ctrl_mode = 1;
        break;
    default:
        break;
    }

    switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
    case SND_SOC_DAIFMT_NB_NF: //normal BCLK and FSYNC
        device->edge_invert = 0;
        break;
    case SND_SOC_DAIFMT_IB_IF: //invert BCLK and FSYNC
        device->edge_invert = 1;
        break;
    default:
        break;
    }

    device->format = fmt & SND_SOC_DAIFMT_FORMAT_MASK;

    return 0;
}

static int purin_dai_startup(struct snd_pcm_substream *substream,
                                           struct snd_soc_dai *cpu_dai)
{
    unsigned long ccr_reg = (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ? TX_CCR : RX1_CCR;
    unsigned long ccr_val = AIREG_READ32(ccr_reg);

    /* set bit order to MSB */
    CFGREG(ccr_val, CCR_BIT_ORDER_MSB_FIRST, CCR_BIT_ORDER);

    CFGREG(ccr_val, CCR_CH0_ENABLE, CCR_CH0_ENABLE);
    CFGREG(ccr_val, CCR_CH1_ENABLE, CCR_CH1_ENABLE);

    AIREG_WRITE32(ccr_reg, ccr_val);

    return 0;
}

static int purin_dai_prepare(struct snd_pcm_substream *substream,
                                           struct snd_soc_dai *dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    struct purin_device *device = dai->id ? &master->rx_dev : &master->tx_dev;
    unsigned long irqflags;
    pr_debug("\n");
    spin_lock_irqsave(&(master->reg_lock), irqflags);

    purin_buf_specific(substream);
    ret = purin_device_init(device);
    if (ret < 0) {
        printk(KERN_ERR "[%s:%d]FORMAT not Support\n", __func__, __LINE__);
        goto result;
    }

#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
    shared_bclk_div = device->bclk_div;
#endif
#if defined(CONFIG_PANTHER_SND_FS_SHARED)
    shared_chan_num = device->chan_num;
    shared_fs_rate = device->fs_rate;
    shared_bit_depth = device->bit_depth;
#endif

    /* unmask interrupt */
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        AIREG_UPDATE32(INTR_CTL, (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN), (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN));
    } else {
        AIREG_UPDATE32(INTR_CTL, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN), (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
    }

result:
    spin_unlock_irqrestore(&(master->reg_lock), irqflags);

    return ret;
}

#define SYM4_CLKGEN_REG0 0xbf5d0098
#define SYM4_CLKGEN_REG1 0xbf5d0120
#define SYM4_CLKGEN_REG2 0xbf5d0128
int purin_get_mclk(unsigned int sample_rate, int stream)
{
	void* reg = NULL; 
	u32 val = 0, mclk = 0;;

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
	if (stream == SNDRV_PCM_STREAM_PLAYBACK) {
		val &= 0xf000ffff;
	    switch (sample_rate) {
			//case 16000:
				//return 16384000UL;
			//case 48000:
			//case 96000:
				//return 98304000UL;
	        case 8000:
	        case 16000:
	        case 32000:
				mclk = 8192000UL;
				val |= 0x0810000;
	            break;

	        case 22050:
	        case 44100:
	        case 88200:
	        case 176400:
	            mclk = 11289600UL;
				val |= 0x2200000;
				break;

	        case 48000:
	        case 96000:
	        case 192000:
	        case 384000:
	            mclk = 12288000UL;
				val |= 0x0800000;
				break;

			default:
				mclk = -EINVAL;
				pr_err("sample rate %d is not supported yet!",sample_rate);
				break;
	    }
	}else {
		val &= 0xfffff000;
	    switch (sample_rate) {
			//case 16000:
				//return 16384000UL;
			//case 48000:
			//case 96000:
				//return 98304000UL;
	        case 8000:
	        case 16000:
	        case 32000:
				mclk = 8192000UL;
				val |= 0x081;
	            break;

	        case 22050:
	        case 44100:
	        case 88200:
	        case 176400:
	            mclk = 11289600UL;
				val |= 0x220;
				break;

	        case 48000:
	        case 96000:
	        case 192000:
	        case 384000:
	            mclk = 12288000UL;
				val |= 0x080;
				break;

			default:
				mclk = -EINVAL;
				pr_err("sample rate %d is not supported yet!",sample_rate);
				break;
	    }
	}
	writel(val, reg);
	iounmap(reg);

    return mclk;
}

static int purin_dai_hw_params(struct snd_pcm_substream *substream,
                struct snd_pcm_hw_params *params,
                struct snd_soc_dai *cpu_dai)
{
    struct purin_master *master;
    struct purin_device *device;
    int bclk_rate;

    master = snd_soc_dai_get_drvdata(cpu_dai);
    if (!master) {
        pr_debug("%s:%d no master!!\n", __func__, __LINE__);
        return -1;
    }

    device = cpu_dai->id ? &master->rx_dev : &master->tx_dev;
    device->stream = substream->stream;
    device->chan_num = params_channels(params);
    device->fs_rate = params_rate(params);
    device->sclk_rate = purin_get_mclk(device->fs_rate, substream->stream);
    device->bit_depth = snd_pcm_format_width(params_format(params));
    pr_debug("bit_depth=%d\n", device->bit_depth);
    if (device->bit_depth <= 16) //wl=8/16, channel bit=16
        bclk_rate = (device->fs_rate * device->chan_num * 16);
    else //wl=24/32, channel bit=32
        bclk_rate = (device->fs_rate * device->chan_num * 32);

    if (bclk_rate > device->sclk_rate) {
        return -EINVAL;
    } else {
        if (device->sclk_rate % bclk_rate)
            return -EINVAL;
        else
            device->bclk_div = device->sclk_rate / bclk_rate;
    }
	printk("fs=%d,sclk=%d,depth=%d,chan=%d,bclk=%d,bclkdiv=%d\n",device->fs_rate,device->sclk_rate,device->bit_depth,device->chan_num,bclk_rate,device->bclk_div);

#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
    if ((shared_bclk_div > 0) && (shared_bclk_div != device->bclk_div)) {
        printk(KERN_ERR "the ratio of MCLK and BCLK has set up as %d\n", shared_bclk_div);
        return -EINVAL;
    }
#endif
#if defined(CONFIG_PANTHER_SND_FS_SHARED)
    /*
     * Inconsistent settings on Tx/Rx could cause Rx channel counter error
     */
    if ((shared_chan_num > 0) && (shared_chan_num != device->chan_num)) {
        printk(KERN_ERR "the number of channel has set up as %d\n", shared_chan_num);
        return -EINVAL;
    }
    if ((shared_fs_rate > 0) && (shared_fs_rate != device->fs_rate)) {
        printk(KERN_ERR "the rate of FSYNC has set up as %d\n", shared_fs_rate);
        return -EINVAL;
    }
    if ((shared_bit_depth > 0) && (shared_bit_depth != device->bit_depth)) {
        printk(KERN_ERR "the bit depth has set up as %d\n", shared_bit_depth);
        return -EINVAL;
    }
#endif

    return 0;
}

#if 0
#define ENABLE_INTERFACE(ch, fmt)                                         \
do {                                                                      \
    if ((SND_SOC_DAIFMT_I2S == fmt) || (SND_SOC_DAIFMT_LEFT_J == fmt))    \
        AIREG_UPDATE32(TX_GCR+(0x10*ch), GCR_I2S_ENABLE, GCR_I2S_ENABLE); \
    else                                                                  \
        AIREG_UPDATE32(TX_GCR+(0x10*ch), GCR_PCM_ENABLE, GCR_PCM_ENABLE); \
} while (0);
#else
#define ENABLE_INTERFACE(ch, fmt)                                         \
do {                                                                      \
        AIREG_UPDATE32(TX_GCR+(0x10*ch), GCR_I2S_ENABLE, GCR_I2S_ENABLE); \
} while (0);
#endif

#define DISABLE_INTERFACE(ch)                        \
do {                                                 \
    AIREG_UPDATE32(TX_GCR+(0x10*ch), 0, GCR_ENABLE); \
} while (0);

static int purin_dai_trigger(struct snd_pcm_substream *substream, int cmd,
                  struct snd_soc_dai *cpu_dai)
{
    int ret = 0;
    struct purin_master *master = snd_soc_dai_get_drvdata(cpu_dai);
    struct purin_device *device = cpu_dai->id ? &master->rx_dev : &master->tx_dev;
    unsigned long irqflags;
    //pr_debug("%s:%d name=%s cmd=%d id=%d\n", __func__, __LINE__, cpu_dai->name, cmd, cpu_dai->id);

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        spin_lock_irqsave(&(master->reg_lock), irqflags);
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
            ENABLE_TXDMA();
            ENABLE_INTERFACE(0, device->format);
        } else {
            ENABLE_RX1DMA();
#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
            if (!(AIREG_READ32(TX_GCR) & GCR_ENABLE)) // Tx bclk enable not yet
                ENABLE_INTERFACE(0, device->format);
#endif
            ENABLE_INTERFACE(2, device->format);
#if defined(CONFIG_PANTHER_SND_LPSD)
            AIREG_UPDATE32(LPSD_CTL, LPSD_EN, LPSD_EN);
#endif
        }
        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        spin_lock_irqsave(&(master->reg_lock), irqflags);
        pr_debug("stream %s stop\n", (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)? "PB":"REC");
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
            if (!(AIREG_READ32(RX1_GCR) & GCR_ENABLE)) { // bclk is not in use
                shared_bclk_div = 0;
# if defined(CONFIG_PANTHER_SND_FS_SHARED)
                shared_chan_num = shared_fs_rate = shared_bit_depth = 0;
# endif
                DISABLE_INTERFACE(0);
            }
#else
            DISABLE_INTERFACE(0);
#endif
            DISABLE_TXDMA();
        } else {
#if defined(CONFIG_PANTHER_SND_LPSD)
            AIREG_UPDATE32(LPSD_CTL, 0, LPSD_EN);
#endif
            DISABLE_INTERFACE(1);
            DISABLE_RX1DMA();
#if defined(CONFIG_PANTHER_SND_BCLK_SHARED)
            if (!(AIREG_READ32(TX_CCR) & CCR_TXDMA_EN)) { // Tx stream enable not yet
                DISABLE_INTERFACE(0);
                shared_bclk_div = 0;
# if defined(CONFIG_PANTHER_SND_FS_SHARED)
                shared_chan_num = shared_fs_rate = shared_bit_depth = 0;
# endif
            }
#endif
        }
        spin_unlock_irqrestore(&(master->reg_lock), irqflags);
        break;
    default:
        ret = -EINVAL;
    }

    return ret;
}

static void purin_dai_shutdown(struct snd_pcm_substream *substream,
                struct snd_soc_dai *dai)
{
    struct purin_master *master = snd_soc_dai_get_drvdata(dai);
    unsigned long irqflags;
    pr_debug("name=%s\n", dai->name);
    spin_lock_irqsave(&(master->reg_lock), irqflags);

    pr_debug("shutdown %d\n", substream->stream);
    /* mask interrupt */
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        AIREG_UPDATE32(INTR_CTL, 0, (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN));
    } else {
        AIREG_UPDATE32(INTR_CTL, 0, (RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN));
    }

    spin_unlock_irqrestore(&(master->reg_lock), irqflags);
}

#define PURIN_DAI_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | \
        SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_176400 | \
        SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000 | SNDRV_PCM_RATE_384000)

static const struct snd_soc_dai_ops purin_ops = {
    .startup    = purin_dai_startup,
    .shutdown   = purin_dai_shutdown,
    .prepare    = purin_dai_prepare,
    .trigger    = purin_dai_trigger,
    .hw_params  = purin_dai_hw_params,
    .set_fmt    = purin_dai_set_dai_fmt,
};

static const struct snd_soc_component_driver purin_dai_component = {
    .name       = "purin-audio",
};

#define PURIN_SUPPORT_FORMATS (SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | \
                               SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver purin_dai[] = {
    {
        .name = "purin-audio-tx",
        .playback = {
            .channels_min = 1,       //set channels_min 1 is due to mono wav and stereo wav need different machanism
            .channels_max = 16,
            .rates = PURIN_DAI_RATES,
            .formats = PURIN_SUPPORT_FORMATS,},
        .ops = &purin_ops,
    },
    {
        .name = "purin-audio-rx",
        .capture = {
            .channels_min = 1,
            .channels_max = 16,
            .rates = PURIN_DAI_RATES,
            .formats = PURIN_SUPPORT_FORMATS,},
        .ops = &purin_ops,
    }
};

static int purin_dai_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;
    struct resource *res = NULL;
    struct purin_master *master = NULL;

    master = kzalloc(sizeof(struct purin_master), GFP_KERNEL);
    if (NULL == master) {
        dev_err(dev, "Could not allocate master\n");
        ret = -ENOMEM;
        goto err_out;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (unlikely(res < 0)) {
        dev_err(dev, "no memory specified\n");
        ret = -ENOENT;
        goto err_out;
    }

    master->res = res;
    master->pdev = pdev;
    spin_lock_init(&(master->reg_lock));

    dev_set_drvdata(dev, (void *)master);

#if defined(CONFIG_PM)
    device_init_wakeup(dev, true);
#endif

    ret = devm_snd_soc_register_component(dev, &purin_dai_component, purin_dai, 2);
    if (unlikely(ret != 0)) {
        dev_err(dev, "failed to register dai\n");
        goto err_out;
    }

#ifdef CONFIG_SND_PROC_FS
	snd_procfs_init();
#endif
    return ret;

err_out:
#if !defined(AUDIO_RECOVERY)
    if (master) {
        kfree(master);
        master = NULL;
    }
#endif
    return ret;
}

static int purin_dai_remove(struct platform_device *pdev)
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
#if defined(CONFIG_PM)
    device_init_wakeup(dev, false);
#endif
    return 0;
}

struct platform_device panther_audio_device = {
    .name       = "purin-audio",
    .id     = -1,
};

static struct platform_driver purin_dai_driver = {
    .probe = purin_dai_probe,
    .remove = purin_dai_remove,

    .driver = {
        .name = "purin-audio",
        .owner = THIS_MODULE,
    },
};

static int __init purin_dai_init(void)
{
	int ret = 0;
	
    ret = platform_device_register(&panther_audio_device);
    if (ret != 0) {
        pr_err("Failed to register purin-audio device: %d\n", ret);
        platform_device_put(&panther_audio_device);
        return ret;
    }

    return platform_driver_register(&purin_dai_driver);
}

static void __exit purin_dai_exit(void)
{
    platform_driver_unregister(&purin_dai_driver);
}

module_init(purin_dai_init);
module_exit(purin_dai_exit);

/* Module information */
MODULE_AUTHOR("Terry Chiu");
MODULE_DESCRIPTION("ASoC DAI driver");
MODULE_LICENSE("GPL");
#endif //defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
