/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _PURIN_AUDIO_H
#define _PURIN_AUDIO_H

#include "mach/symphony_reg_base_addr.h"
#include "mach/symphony_regs.h"
#include "purin-proc.h"

#define INTERNAL_CLOCK_AUTO_GATING

extern struct snd_info snd_proc_info;

static inline u32 AIREG_READ32(u32 x)
{
    u32 val = (*(volatile u32*)(AI_BASE+(x)));
    return val;
}

static inline void AIREG_WRITE32(u32 x, u32 val)
{
    if (snd_proc_info.debug_type & SND_DEBUG_TYPE_REGUPDATE)
    {
        printk("W %x %x\n", x, val);
    }
    (*(volatile u32*)(AI_BASE+(x)) = (u32)(val));
    wmb();
}

static inline void AIREG_UPDATE32(u32 x, u32 val, u32 mask)
{
    u32 newval;

    if (snd_proc_info.debug_type & SND_DEBUG_TYPE_REGUPDATE)
    {
        printk("U %x %x\n", x, val);
    }

    newval = *(volatile u32*) (AI_BASE+(x));
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));
    *(volatile u32*)(AI_BASE+(x)) = newval;
    wmb();
}

#define CFGREG(tmp, val, mask) \
    do { tmp = ((tmp&(~mask))|val); } while(0)

/* Global configuration register settings */
#define GCR_TX_XMIT_CLK_EDGE            0x80000000UL
#define GCR_TX_XMIT_CLK_EDGE_RISING     0x00000000UL
#define GCR_TX_XMIT_CLK_EDGE_FALLING    0x80000000UL
#define GCR_RX_SAMPLE_CLK_EDGE          0x40000000UL
#define GCR_RX_SAMPLE_CLK_EDGE_RISING   0x00000000UL
#define GCR_RX_SAMPLE_CLK_EDGE_FALLING  0x40000000UL
#define GCR_FSYNC_SHARE                 0x20000000UL
#define GCR_CTRL_MODE                   0x10000000UL
#define GCR_CTRL_MODE_MASTER            0x00000000UL
#define GCR_CTRL_MODE_SLAVE             0x10000000UL
#define GCR_BCLK                        0x00700000UL
#define GCR_BCLK_DIV_1                  0x00000000UL
#define GCR_BCLK_DIV_2                  0x00100000UL
#define GCR_BCLK_DIV_4                  0x00200000UL
#define GCR_BCLK_DIV_8                  0x00300000UL
#define GCR_BCLK_FROM_TX                0x00700000UL
#define GCR_PDM_DATA_MODE               0x00080000UL
#define GCR_LJ_MODE                     0x00020000UL
#define GCR_CHANNEL_NUM                 0x00007000UL
#define GCR_CHANNEL_1                   0x00000000UL
#define GCR_CHANNEL_2                   0x00001000UL
#define GCR_CHANNEL_4                   0x00002000UL
#define GCR_CHANNEL_8                   0x00003000UL
#define GCR_CHANNEL_BITS                0x00000300UL
#define GCR_CHANNEL_16BITS              0x00000100UL
#define GCR_CHANNEL_32BITS              0x00000300UL
#define GCR_ENABLE                      0x00000007UL
#define GCR_PDM_ENABLE                  0x00000004UL
#define GCR_I2S_ENABLE                  0x00000002UL
#define GCR_PCM_ENABLE                  0x00000001UL

/* Channel configuration register settings */
#define CCR_BUS_MODE                    0x00003000UL
#define CCR_BUS_MODE_8BITS              0x00000000UL
#define CCR_BUS_MODE_16BITS             0x00001000UL
#define CCR_BUS_MODE_24BITS             0x00002000UL
#define CCR_BUS_MODE_32BITS             0x00003000UL
#define CCR_BIT_ORDER                   0x00000400UL
#define CCR_BIT_ORDER_LSB_FIRST         0x00000000UL
#define CCR_BIT_ORDER_MSB_FIRST         0x00000400UL
#define CCR_RXDMA_EN                    0x00000008UL
#define CCR_TXDMA_EN                    0x00000004UL
#define CCR_CH1_ENABLE                  0x00000002UL
#define CCR_CH0_ENABLE                  0x00000001UL

/* Channel FS internal register settings */
#define CFS_INTEVAL                     0x0000FFFFUL

/* Ring Buffer settings */
#define RBUF_SIZE_RESERVED_BITS         0x0000001FUL

#define TX_ADDRESS0     (0x000)
#define TX_LEN0         (0x004)
#define TX_RD_POINTER0  (0x008)
#define TX_WR_POINTER0  (0x00C)

#define TX_ADDRESS1     (0x010)
#define TX_LEN1         (0x014)
#define TX_RD_POINTER1  (0x018)
#define TX_WR_POINTER1  (0x01C)

#define RX0_ADDRESS0    (0x020)
#define RX0_LEN0        (0x024)
#define RX0_RD_POINTER0 (0x028)
#define RX0_WR_POINTER0 (0x02C)

#define RX0_ADDRESS1    (0x030)
#define RX0_LEN1        (0x034)
#define RX0_RD_POINTER1 (0x038)
#define RX0_WR_POINTER1 (0x03C)

#define RX1_ADDRESS0    (0x040)
#define RX1_LEN0        (0x044)
#define RX1_RD_POINTER0 (0x048)
#define RX1_WR_POINTER0 (0x04C)

#define RX1_ADDRESS1    (0x050)
#define RX1_LEN1        (0x054)
#define RX1_RD_POINTER1 (0x058)
#define RX1_WR_POINTER1 (0x05C)

#define INTR_CTL        (0x060)
    #define TX_THD_INT_EN           (0x01 << 10)
    #define TX_FIFO_UD_INT_EN       (0x01 << 9)
    #define TX_RING_UD_INT_EN       (0x01 << 8)
    #define RX1_THD_INT_EN          (0x01 << 6)
    #define RX1_FIFO_OV_INT_EN      (0x01 << 5)
    #define RX1_RING_OV_INT_EN      (0x01 << 4)
    #define RX0_THD_INT_EN          (0x01 << 2)
    #define RX0_FIFO_OV_INT_EN      (0x01 << 1)
    #define RX0_RING_OV_INT_EN      (0x01 << 0)
#define THD_CTRL0       (0x064)
    #define THD_CTRL0_TX_FIFO_THD   0x0000007FUL
#define THD_CTRL1       (0x068)
    #define THD_CTRL1_QOS_THD       0x1F000000UL
    #define THD_CTRL1_INT_THD_OLD   0x000FFFFFUL
#define INTR_STS        (0x06C)
    #define TX_THD_INT              (0x01 << 26)
    #define TX_FIFO_UD_INT          (0x01 << 25)
    #define TX_RING_UD_INT          (0x01 << 24)
    #define RX1_THD_INT             (0x01 << 22)
    #define RX1_FIFO_OV_INT         (0x01 << 21)
    #define RX1_RING_OV_INT         (0x01 << 20)
    #define RX0_THD_INT             (0x01 << 18)
    #define RX0_FIFO_OV_INT         (0x01 << 17)
    #define RX0_RING_OV_INT         (0x01 << 16)
#define TX_DMA_CTRL     (0x070)
#define RX0_DMA_CTRL    (0x074)
#define RX1_DMA_CTRL    (0x078)
    #define DMA_CTRL_IDLE           (0x01 << 4)
    #define DMA_CTRL_TX_SUSPEND     (0x01 << 3)
    #define DMA_CTRL_BUF1_EN        (0x01 << 2)
    #define DMA_CTRL_BUF0_EN        (0x01 << 1)
    #define DMA_CTRL_ENABLE         (0x01 << 0)

#define INT_THD_TX      (0x080)
#define INT_THD_RX0     (0x084)
#define INT_THD_RX1     (0x088)
#define ICLK_GATE_CTL   (0x08C)
    #define AHB_ICLK_NO_GATE        (0x01 << 1)
    #define AXI_ICLK_NO_GATE        (0x01 << 0)

#define PDM_CTL         (0x100)
	#define PDM_SYS_CLK				(0x02 << 10)
    #define PDM_PINS                (0x01 << 9)
    #define PDM_RESET               (0x01 << 8)
    #define PDM_CH_EN               (0x07 << 5)
    #define PDM3_EN                 (0x01 << 7)
    #define PDM2_EN                 (0x01 << 6)
    #define PDM1_EN                 (0x01 << 5)
    #define PDM_LR_SWAP             (0x01 << 4)
    #define PDM_BUS_MODE            (0x03 << 2)
    #define PDM_BUS_24BITS          (0x00 << 2)
    #define PDM_BUS_20BITS          (0x01 << 2)
    #define PDM_BUS_16BITS          (0x02 << 2)
    #define PDM_BYPASS              (0x01 << 1)
    #define PDM_EN                  (0x01 << 0)
#define PDM_DLY         (0x104)
    #define PDM3_DLY                (0x07 << 16)
    #define PDM2_DLY                (0x07 << 8)
    #define PDM1_DLY                (0x07 << 0)

#define TX_GCR          (0x200)
#define RX0_GCR         (0x210)
#define RX1_GCR         (0x220)

#define TX_CCR          (0x204)
#define RX0_CCR         (0x214)
#define RX1_CCR         (0x224)

#define TX_CFS          (0x208)
#define RX0_CFS         (0x218)
#define RX1_CFS         (0x228)

#define TX_DUMMY_TONE       (0x230)
#define HW_PATCH            (0x234)
#define ADC_CFG             (0x240)
#define ADC_TEST_MODE       (0x244)
#define DAC_CFG             (0x250)
#define GLOBAL_DEBUG_REG    (0x260)
    #define DAI_EXTERNAL_LOOPBACK   0x00000200UL
    #define DAI_INTERNAL_LOOPBACK   0x00000100UL
#define GLOBAL_DEBUG_REG0   (0x264)
#define GLOBAL_DEBUG_REG1   (0x268)

#define LPSD_CTL        (0x300)
    #define LPSD_EN                 (0x01 << 31)
    #define LPSD_SEL_RX1            (0x01 << 25)
    #define LPSD_RSHFT_BIT          0x00FF0000UL
    #define LPSD_FRAME_SIZE         0x0000FF00UL
#define LPSD_PARAM1     (0x304)
#define LPSD_PARAM2     (0x308)
    #define LPSD_DC_FILTER          (0x01 << 16)
#define LPSD_CLKDIV     (0x30C)
#define LPSD_STS        (0x310)
    #define LPSD_LOSE_INT           (0x01 << 1)
    #define LPSD_DETECT_INT         (0x01 << 0)
#define LPSD_INT        (0x314)
    #define LPSD_LOSE_INT_EN        (0x01 << 1)
    #define LPSD_DETECT_INT_EN      (0x01 << 0)

enum {
    LPSD_SOUND_ABSENCE_STATE,
    LPSD_SOUND_PRESENCE_STATE,
    LPSD_INITIAL_STATE,
};

#define INTR_STATUS_MASK           (TX_THD_INT_EN|TX_FIFO_UD_INT_EN|TX_RING_UD_INT_EN|RX0_THD_INT_EN|RX0_FIFO_OV_INT_EN|RX0_RING_OV_INT_EN|RX1_THD_INT_EN|RX1_FIFO_OV_INT_EN|RX1_RING_OV_INT_EN)

struct purin_device {
    int format;         // 1:I2S 3:LJ 4:PCM 5:PCM(LJ) 7:PDM based on soc-dai.h
    int ctrl_mode;      // 0:Master 1:Slave
    int stream;         // SNDRV_PCM_STREAM_PLAYBACK/SNDRV_PCM_STREAM_CAPTURE
    int fs_rate;        // sample rate
    int sclk_rate;      // system clock rate
    int bclk_div;       // bclk_div = system clock rate / bit clock rate
    int chan_num;       // channel number
    int edge_invert;    // data valid edge invert setting
    int bit_depth;      // bit depth
};

struct purin_master {
    struct resource *res;
    struct platform_device *pdev;

    struct purin_device tx_dev;
    struct purin_device rx_dev;
    spinlock_t reg_lock;    /* protects update AIREG */
};

#define ENABLE_TXDMA()                                      \
do {                                                        \
    if (!(AIREG_READ32(TX_CCR) & CCR_TXDMA_EN)) {           \
        AIREG_UPDATE32(TX_DMA_CTRL, (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE), (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
        AIREG_UPDATE32(TX_CCR, CCR_TXDMA_EN, CCR_TXDMA_EN); \
    }                                                       \
} while (0);

#define ENABLE_RXDMA()                                       \
do {                                                         \
    if (!(AIREG_READ32(RX0_CCR) & CCR_RXDMA_EN)) {           \
        AIREG_UPDATE32(RX0_DMA_CTRL, (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE), (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
        AIREG_UPDATE32(RX0_CCR, CCR_RXDMA_EN, CCR_RXDMA_EN); \
    }                                                        \
} while (0);

#define ENABLE_RX1DMA()                                       \
do {                                                         \
    if (!(AIREG_READ32(RX1_CCR) & CCR_RXDMA_EN)) {           \
        AIREG_UPDATE32(RX1_DMA_CTRL, (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE), (DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
        AIREG_UPDATE32(RX1_CCR, CCR_RXDMA_EN, CCR_RXDMA_EN); \
    }                                                        \
} while (0);

#define DISABLE_TXDMA()                \
do {                                   \
    AIREG_UPDATE32(TX_CCR, 0, CCR_TXDMA_EN); \
    AIREG_UPDATE32(TX_DMA_CTRL, 0, (DMA_CTRL_BUF1_EN|DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
} while (0);

#define DISABLE_RXDMA()                \
do {                                   \
    AIREG_UPDATE32(RX0_CCR, 0, CCR_RXDMA_EN); \
    AIREG_UPDATE32(RX0_DMA_CTRL, 0, (DMA_CTRL_BUF1_EN|DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
} while (0);

#define DISABLE_RX1DMA()                \
do {                                   \
    AIREG_UPDATE32(RX1_CCR, 0, CCR_RXDMA_EN); \
    AIREG_UPDATE32(RX1_DMA_CTRL, 0, (DMA_CTRL_BUF1_EN|DMA_CTRL_BUF0_EN|DMA_CTRL_ENABLE)); \
} while (0);

/*
 * Logic for a codec as connected on a Panther Device
 */

struct external_audio_data {
#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
    struct task_struct *micarray_task;
    void (*micarray_suspend)(int mic_idx);
    void (*micarray_resume)(int mic_idx);
#endif
    /* array size of codec_i2c "2" for Tx and Rx */
    struct i2c_client *codec_i2c[2];
#if defined(CONFIG_PANTHER_SND_PDM_RX0)
    struct platform_device *pdm_device;
#endif
	struct platform_device *es7134_device;
};

#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
enum {
    MICARRAY_POWERSAVING_NONE = 0,
    MICARRAY_POWERSAVING_SUSPEND,
    MICARRAY_POWERSAVING_RESUME,
};

int purin_suspend_micarray_kthread_init(struct external_audio_data *data);
void purin_suspend_micarray_kthread_cleanup(struct external_audio_data *data);
#endif

#endif
