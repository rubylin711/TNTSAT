/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
/*
 * SoC audio for Panther
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/sysctl.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include "purin-dma.h"
#include "purin-audio.h"

//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG pr_fmt(fmt), ##__VA_ARGS__)
#endif

#define CODEC_SYSCLK 0
#define PANTHER_I2C_ADDR_0 0x1b
#define _STR(s) #s
#define PURIN_CODEC_SUFFIX(name, bus, addr) name "." _STR(bus) "-00" _STR(addr)

extern int purin_get_mclk(unsigned int sample_rate, int stream);

#if defined(CONFIG_PANTHER_SND_TX_I2C_BUS)
static struct i2c_board_info purin_tx_i2c_dev = {
#if defined(CONFIG_PANTHER_SND_TX_WM8750)
    I2C_BOARD_INFO("wm8750", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_TX_MP320)
    I2C_BOARD_INFO("mp320", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_TX_DUMMY)
    I2C_BOARD_INFO("panther-dummy", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_TX_TLV320AIC3111)
    I2C_BOARD_INFO("tlv320aic3111", 0x18)
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES8388)
    I2C_BOARD_INFO("es8388", 0x10)
#endif
#if defined(CONFIG_PANTHER_SND_TX_ESS9018Q2C)
    I2C_BOARD_INFO("es9018k2m", 0x48)
#endif
#if defined(CONFIG_PANTHER_SND_TX_PCM1681)
    I2C_BOARD_INFO("pcm1681", 0x4c)
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES7149)
    I2C_BOARD_INFO("es7144", 0x10)
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES8311)
    I2C_BOARD_INFO("ES8311", 0x30)
#endif

};
#endif

#if 1//defined(CONFIG_PANTHER_SND_RX_I2C_BUS) && (!defined(CONFIG_PANTHER_SND_TX_I2C_BUS) || (CONFIG_PANTHER_SND_RX_I2C_BUS != CONFIG_PANTHER_SND_TX_I2C_BUS))
static struct i2c_board_info purin_rx_i2c_dev = {
#if defined(CONFIG_PANTHER_SND_RX_WM8750)
    I2C_BOARD_INFO("wm8750", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_RX_MP320)
    I2C_BOARD_INFO("mp320", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_RX_DUMMY)
    I2C_BOARD_INFO("panther-dummy", 0x1b)
#endif
#if defined(CONFIG_PANTHER_SND_RX_TLV320AIC3111)
    I2C_BOARD_INFO("tlv320aic3111", 0x18)
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES8388)
    I2C_BOARD_INFO("es8388", 0x10)
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES7243)
    I2C_BOARD_INFO("MicArray_0", 0x10)
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES7210)
    I2C_BOARD_INFO("MicArray_0", 0x41)//es7210_0
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES8311)
    I2C_BOARD_INFO("ES8311", 0x30)
#endif

};
#endif

#if defined(CONFIG_PANTHER_SND_RX_ES7210)
static struct i2c_board_info purin_rx_i2c_dev_1 = {
	I2C_BOARD_INFO("MicArray_1", 0x40)//es7210_1
};
#endif

#if defined(AUDIO_RECOVERY)
int sysctl_ai_recovery = 0;
extern void start_recovery_timer(void);
static int proc_start_recovery(struct ctl_table *table, int write,
                     void __user *buffer, size_t *lenp, loff_t *ppos)
{
    int ret = proc_dointvec(table, write, buffer, lenp, ppos);
    if (sysctl_ai_recovery == 1)
    {
        start_recovery_timer();
    }
    return ret;
}
#endif

static struct ctl_table ai_table[] = {
#if defined(AUDIO_RECOVERY)
    {
        //.ctl_name   = CTL_UNNUMBERED,
        .procname   = "ai.recovery",
        .data       = &sysctl_ai_recovery,
        .maxlen     = sizeof(int),
        .mode       = 0644,
        .proc_handler   = proc_start_recovery
    },
#endif

    { //.ctl_name = 0,
    }
};

static char *stream_name[] =
{
    "PLAYBACK", "CAPTURE"
};

static int purin_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *codec_dai = rtd->codec_dai;
    unsigned int sample_rate = params_rate(params);
    int mclk;
    int ret = 0;

    pr_debug("\n");
    mclk = purin_get_mclk(sample_rate, substream->stream);
    if (mclk < 0) {
        dev_err(rtd->dev, "not support sample rate %d\n", sample_rate);
        return mclk;
    }

    dev_info(rtd->dev, "%s in %d samples and %d main clocks per second\n", stream_name[substream->stream],
             sample_rate, mclk);

    /* set the codec system clock for DAC and ADC */
    // it will call codec set_dai_sysclk
    ret = snd_soc_dai_set_sysclk(codec_dai, CODEC_SYSCLK, mclk, SND_SOC_CLOCK_IN);
    if (ret < 0)
        return ret;

    return 0;
}

static struct snd_soc_ops purin_ops = {
    .hw_params = purin_hw_params,
};

static struct snd_soc_dai_link purin_codec_dai []= {
#if defined(CONFIG_PANTHER_SND_TX_WM8750)
    {
    .name = "wm8750-TX",
    .stream_name = "WM8750 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "wm8750-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("wm8750", CONFIG_PANTHER_SND_TX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_MP320)
    {
    .name = "mp320-TX",
    .stream_name = "MP320 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "mp320-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("mp320", CONFIG_PANTHER_SND_TX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_DUMMY)
    {
    .name = "DUMMY-TX",
    .stream_name = "DUMMY PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "dummy-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("panther-dummy", CONFIG_PANTHER_SND_TX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_TLV320AIC3111)
    {
    .name = "tlv320aic3111-TX",
    .stream_name = "TLV320AIC3111 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "tlv320aic31xx-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("tlv320aic31xx-codec", CONFIG_PANTHER_SND_TX_I2C_BUS, 18),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_ESS9018Q2C)
    {
    .name = "es9018k2m",
    .stream_name = "ESS9018Q2C",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "es9018k2m-dai",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es9018k2m-i2c", CONFIG_PANTHER_SND_TX_I2C_BUS, 48),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES7149)
    {
    .name = "es7144",
    .stream_name = "Playback",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "ES7144 HiFi DAC",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es7144", CONFIG_PANTHER_SND_TX_I2C_BUS, 10),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES8388)
    {
    .name = "es8388-TX",
    .stream_name = "ES8388 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "es8388-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es8388", CONFIG_PANTHER_SND_TX_I2C_BUS, 10),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_PCM1681)
    {
    .name = "pcm1681",
    .stream_name = "pcm1681 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "pcm1681-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("pcm1681", CONFIG_PANTHER_SND_TX_I2C_BUS, 4c),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_LEFT_J),
    },
#endif
#if defined(CONFIG_PANTHER_SND_TX_ES8311)
    {
    .name = "ES8311-TX",
    .stream_name = "ES8311 PLAYBACK",
    .cpu_dai_name = "purin-audio-tx",
    .codec_dai_name = "ES8311 HiFi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("ES8311", CONFIG_PANTHER_SND_TX_I2C_BUS, 30),
    .ops = &purin_ops,
    //需要强制打开I2S或者PCM，此处仅选择数据格式
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),//I2S codec slave mode
    //.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_LEFT_J),//I2S codec slave mode LJ mode
    //.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_I2S),//I2S codec master mode
    //.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_LEFT_J),//I2S codec master mode LJ mode

    //.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_LEFT_J),//PCM codec slave mode LJ mode
    //.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),//PCM codec slave mode
	//.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_LEFT_J),//PCM codec master mode LJ mode
	//.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_I2S),//PCM codec master mode
    },
#endif
//#if defined(CONFIG_PANTHER_SND_TX_ES7134)
	{
	.name = "es7134 tx",
	.stream_name = "es7134 Playback",
	.platform_name = "purin-external-dma", // declared in purin-dma.c
	.cpu_dai_name = "purin-audio-tx",	   // declared in platform.c
	.codec_dai_name = "es7134-hifi",	 // declared in codec/dac.c
	.codec_name = "es7134", 	 // declared in codec/es7134.c
	.ops = &purin_ops,
	.dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
	},
//#endif
#if defined(CONFIG_PANTHER_SND_RX_WM8750)
    {
    .name = "wm8750-RX",
    .stream_name = "WM8750 CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "wm8750-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("wm8750", CONFIG_PANTHER_SND_RX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_MP320)
    {
    .name = "mp320-RX",
    .stream_name = "MP320 CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "mp320-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("mp320", CONFIG_PANTHER_SND_RX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_DUMMY)
    {
    .name = "DUMMY-RX",
    .stream_name = "DUMMY CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "dummy-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("panther-dummy", CONFIG_PANTHER_SND_RX_I2C_BUS, 1b),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_TLV320AIC3111)
    {
    .name = "tlv320aic3111-RX",
    .stream_name = "TLV320AIC3111 CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "tlv320aic31xx-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("tlv320aic31xx-codec", CONFIG_PANTHER_SND_RX_I2C_BUS, 18),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES7243)
    {
    .name = "MicArray_0",
    .stream_name = "MicArray_0",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "ES7243 HiFi 0",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es7243-audio-adc", CONFIG_PANTHER_SND_RX_I2C_BUS, 10),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES7210)
    {
    .name = "MicArray_0",
    .stream_name = "MicArray_0",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "ES7210 HiFi ADC 0",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es7210-audio-adc", CONFIG_PANTHER_SND_RX_I2C_BUS, 41),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_LEFT_J),
    },
#endif
#if defined(CONFIG_PANTHER_SND_RX_ES8388)
    {
    .name = "es8388-RX",
    .stream_name = "ES8388 CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "es8388-hifi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("es8388", CONFIG_PANTHER_SND_RX_I2C_BUS, 10),
    .ops = &purin_ops,
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    },
#endif
#if 0
#if defined(CONFIG_PANTHER_SND_RX_ES8311)
    {
    .name = "ES8311-RX",
    .stream_name = "ES8311 CAPTURE",
    .cpu_dai_name = "purin-audio-rx",
    .codec_dai_name = "ES8311 HiFi",
    .platform_name = "purin-external-dma",
    .codec_name = PURIN_CODEC_SUFFIX("ES8311", CONFIG_PANTHER_SND_RX_I2C_BUS, 30),
    .ops = &purin_ops,
    //.dai_fmt = (SND_SOC_DAIFMT_IB_IF | SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_I2S),
    .dai_fmt = (SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBM_CFM | SND_SOC_DAIFMT_I2S),
    },
#endif
#endif

#if 0
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    {
    .name = "PDM",
    .stream_name = "PDM Capture",
    .platform_name = "purin-external-dma", // declared in purin-dma.c
    .cpu_dai_name = "purin-pdm",      // declared in platform.c
    .codec_dai_name = "pdm-hifi",     // declared in codec/pdm.c
    .codec_name = "panther-pdm",      // declared in codec/pdm.c & purin_pdm_init (platform_device)
    },
#endif
#endif
};

/* Panther audio machine driver */
static struct snd_soc_card purin_external_card = {
      .name = "external",
      .owner = THIS_MODULE,
      .dai_link = purin_codec_dai,
      .num_links = ARRAY_SIZE(purin_codec_dai),
};

#ifdef CONFIG_SYSCTL
static struct ctl_table_header *ai_table_header;
#endif

// first address use here, others use in es7210,es7243
unsigned short mic_i2c_addr[4] = { PANTHER_I2C_ADDR_0, I2C_CLIENT_END, I2C_CLIENT_END, I2C_CLIENT_END };
unsigned int i2c_dev_num = 0;
static char g_codec_name[64];

static int __init setup_i2c_addr(char *str) {
    int i = 0;
    unsigned short i2c_addr_0 = PANTHER_I2C_ADDR_0;
    int ints[5] = {0};
    printk("setup_i2c_addr\n");
    str = get_options(str, ARRAY_SIZE(ints), ints);

    if (ints[0] > 4) {
        printk("Invaild mic_i2c_addr configuration\n");
        return 1;
    }

    if (ints[0] > 0) {
        i2c_dev_num = ints[0];

        for(i = 0; i+1 <= i2c_dev_num; ++i)
        {
            mic_i2c_addr[i] = ints[i+1];
            printk("mic_i2c_addr[%d] = %x\n", i, mic_i2c_addr[i]);
        }
        i2c_addr_0 = mic_i2c_addr[0];
    }

    for (i=0; i<ARRAY_SIZE(purin_codec_dai); i++) {
        if (!strcmp(purin_codec_dai[i].name, "MicArray_0")) {
            // default es7243
            sprintf(g_codec_name, "es7243-audio-adc.0-00%02x", i2c_addr_0);

            if (!strncmp(purin_codec_dai[i].codec_name, "es7210", 6)) {
                sprintf(g_codec_name, "es7210-audio-adc.0-00%02x", i2c_addr_0);
            }
            purin_codec_dai[i].codec_name = g_codec_name;
            printk("codec_name %s\n", purin_codec_dai[i].codec_name);
#if defined(CONFIG_PANTHER_SND_RX_I2C_BUS) && (!defined(CONFIG_PANTHER_SND_TX_I2C_BUS) || (CONFIG_PANTHER_SND_RX_I2C_BUS != CONFIG_PANTHER_SND_TX_I2C_BUS))
            purin_rx_i2c_dev.addr = i2c_addr_0;
            printk("purin_rx_i2c_dev.addr %x\n", purin_rx_i2c_dev.addr);
#endif
        }
    }
    return 1;
}

__setup("mic_i2c_addr=", setup_i2c_addr);

#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
volatile u32 purin_suspend_mic;
extern void es7243_suspend_for_powersaving(int mic_idx);
extern void es7243_resume_for_powersaving(int mic_idx);
#endif

static int purin_external_probe(struct platform_device *op)
{
    struct snd_soc_card *card = &purin_external_card;
    struct external_audio_data *pdata;
    struct i2c_adapter *adapter;
    struct i2c_client *client;
    int ret;
#if defined(CONFIG_PANTHER_SND_RX_ES7210)
	struct i2c_client *client2;
#endif

    pdata = devm_kzalloc(&op->dev, sizeof(struct external_audio_data),
                 GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    card->dev = &op->dev;
    pdata->codec_i2c[SNDRV_PCM_STREAM_PLAYBACK] = NULL;
    pdata->codec_i2c[SNDRV_PCM_STREAM_CAPTURE] = NULL;
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    pdata->pdm_device = NULL;
#endif
	pdata->es7134_device = NULL;

#if defined(CONFIG_PANTHER_SND_TX_I2C_BUS)
    adapter = i2c_get_adapter(CONFIG_PANTHER_SND_TX_I2C_BUS);
    if (!adapter)
        return -ENODEV;

    client = i2c_new_device(adapter, &purin_tx_i2c_dev);
    if (!client) {
        i2c_put_adapter(adapter);
    } else {
        client->adapter = adapter;
        pdata->codec_i2c[SNDRV_PCM_STREAM_PLAYBACK] = client;
    }
#endif
#if 1//defined(CONFIG_PANTHER_SND_RX_I2C_BUS) && (!defined(CONFIG_PANTHER_SND_TX_I2C_BUS) || (CONFIG_PANTHER_SND_RX_I2C_BUS != CONFIG_PANTHER_SND_TX_I2C_BUS))
    adapter = i2c_get_adapter(CONFIG_PANTHER_SND_RX_I2C_BUS);
    if (!adapter)
        return -ENODEV;

    client = i2c_new_device(adapter, &purin_rx_i2c_dev);
    if (!client) {
        i2c_put_adapter(adapter);
    } else {
        client->adapter = adapter;
        pdata->codec_i2c[SNDRV_PCM_STREAM_CAPTURE] = client;
	#if defined(CONFIG_PANTHER_SND_RX_ES7210)
		client2 = i2c_new_device(adapter, &purin_rx_i2c_dev_1);
	    if (!client) {
	        i2c_put_adapter(adapter);
	    }
	#endif
    }
#endif

	pdata->es7134_device = platform_device_alloc("es7134", -1);
	if (!pdata->es7134_device){
		dev_err(&op->dev, "platform_device_alloc() failed\n");
	} else {
		ret = platform_device_add(pdata->es7134_device);
		if (ret) {
			platform_device_put(pdata->es7134_device);
			pdata->es7134_device = NULL;
			dev_err(&op->dev, "platform_device_add() failed: %d\n", ret);
		}
	}

    ret = snd_soc_register_card(card);
    if (ret) {
        dev_err(&op->dev, "snd_soc_register_card() failed: %d\n", ret);
        return ret;
    }

    snd_soc_card_set_drvdata(card, pdata);
    strcpy(card->snd_card->mixername, card->dai_link->name);

#ifdef CONFIG_SYSCTL
    ai_table_header = register_sysctl_table(ai_table);
    if (ai_table_header == NULL)
        return -ENOMEM;
#endif

#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
    pdata->micarray_suspend = es7243_suspend_for_powersaving;
    pdata->micarray_resume = es7243_resume_for_powersaving;
    purin_suspend_micarray_kthread_init(pdata);
#endif

    return ret;
}

static int purin_external_remove(struct platform_device *op)
{
    struct snd_soc_card *card = platform_get_drvdata(op);
    struct external_audio_data *pdata = snd_soc_card_get_drvdata(card);
    int ret;

#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
    purin_suspend_micarray_kthread_cleanup(pdata);
#endif

    ret = snd_soc_unregister_card(card);
    if (pdata->codec_i2c[SNDRV_PCM_STREAM_PLAYBACK]) {
        i2c_put_adapter(pdata->codec_i2c[SNDRV_PCM_STREAM_PLAYBACK]->adapter);
        i2c_unregister_device(pdata->codec_i2c[SNDRV_PCM_STREAM_PLAYBACK]);
    }
    if (pdata->codec_i2c[SNDRV_PCM_STREAM_CAPTURE]) {
        i2c_put_adapter(pdata->codec_i2c[SNDRV_PCM_STREAM_CAPTURE]->adapter);
        i2c_unregister_device(pdata->codec_i2c[SNDRV_PCM_STREAM_CAPTURE]);
    }
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    if (pdata->pdm_device)
        platform_device_unregister(pdata->pdm_device);
#endif

if (pdata->es7134_device)
	platform_device_unregister(pdata->es7134_device);

#ifdef CONFIG_SYSCTL
    unregister_sysctl_table(ai_table_header);
    ai_table_header = NULL;
#endif

    return ret;
}

struct platform_device panther_external_device = {
    .name       = "purin-external",
    .id     = -1,
};

static struct platform_driver purin_external_driver = {
    .probe      = purin_external_probe,
    .remove     = purin_external_remove,
    .driver     = {
        .name   = "purin-external",
        .owner = THIS_MODULE,
        //.pm     = &snd_soc_pm_ops,
    },
};

static int __init purin_external_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_external_device);
    if (ret != 0) {
        pr_err("Failed to register panther-external device: %d\n", ret);
        platform_device_put(&panther_external_device);
        return ret;
    }

    return platform_driver_register(&purin_external_driver);
}

static void __exit purin_external_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&purin_external_driver);
}
//module_platform_driver(purin_external_driver);
module_init(purin_external_init);
module_exit(purin_external_exit);

MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Externel Card driver");
#endif //defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
