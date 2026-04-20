/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 * ALSA SoC DMA driver
 */

#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <asm/delay.h>

#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/pcm.h>

#ifdef DBG_UNDERRUN
#include <linux/jiffies.h>
#include <linux/ktime.h>
#endif

#include "mach/irqs.h"
#define IRQ_PCM IRQ_AI_ID

#include "purin-audio.h"
#include "purin-dma.h"
#include "purin-proc.h"

#if defined(CONFIG_SRAM_AUDIO_ENABLE)
#include "purin-sram-resource.h"
#endif

#if defined(CONFIG_PANTHER_NOTIFY_USER_TX_MUTE)
#define ENABLE_NETLINK
#endif

#define ENABLE_SYSCALL_LASTEST_CAPTURE_DATA

#if defined(ENABLE_SYSCALL_LASTEST_CAPTURE_DATA)
#include "purin-sys-snd.h"
volatile lastest_capture_info lastest_capture;
#endif

#if defined(ENABLE_NETLINK)
#define PURIN_CHECK_TX_ALL_DATA_ZERO
#define NETLINK_USER 31
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#endif

#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
#define PURIN_CHECK_TX_ALL_DATA_ZERO
extern volatile u32 purin_suspend_mic;
#endif

#ifdef CFG_IO_AUDIO_AMP_POWER
#include <asm/mach-panther/gpio.h>
#include <linux/gpio.h>
#endif

#include <linux/workqueue.h>

//#define RING_BUFFER_TEST
#define SRAM_BUFFER_SIZE 40*1024
#define purin_fmt(fmt) "[%s: %d]: " fmt, __func__, __LINE__
//#define PURIN_DEBUG
#ifdef PURIN_DEBUG
#undef pr_debug
#define pr_debug(fmt, ...) \
    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#endif

#undef pr_err
#define pr_err(fmt, ...)    printk(KERN_EMERG purin_fmt(fmt), ##__VA_ARGS__)
#define ASSERT(cond, message)   \
do {                            \
    if (!(cond)) {               \
        panic(message);         \
    }                           \
} while(0)

#ifdef CONFIG_PCMGPIOCONTROL
#define hotplug_path uevent_helper
static void purin_pcm_event_hook(struct work_struct *work);
#endif

#if defined (CFG_IO_AUDIO_AMP_POWER)
static int gpio_status = -1;
#endif

int purin_spk_on(int enable)
{
#if defined (CFG_IO_AUDIO_AMP_POWER)
    if (!gpio_status) {
        if (enable)
            gpio_set_value(AUDIO_AMP_POWER, AUDIO_AMP_ACTIVE_LEVEL);
        else
            gpio_set_value(AUDIO_AMP_POWER, !AUDIO_AMP_ACTIVE_LEVEL);
    }
#endif
    return 0;
}

#define DMA_SUPPORTED_FORMATS (SNDRV_PCM_FMTBIT_S8 |        \
                               SNDRV_PCM_FMTBIT_U8 |        \
                               SNDRV_PCM_FMTBIT_S16_LE |    \
                               SNDRV_PCM_FMTBIT_S16_4LE |   \
                               SNDRV_PCM_FMTBIT_S20_3LE |   \
                               SNDRV_PCM_FMTBIT_S24_LE |    \
                               SNDRV_PCM_FMTBIT_S32_LE)

/**
 *  audio data is SNDRV_PCM_INFO_INTERLEAVED in panther
 */
static const struct snd_pcm_hardware purin_pcm_hardware = {
    .info           = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP | SNDRV_PCM_INFO_MMAP_VALID,
    .formats        = DMA_SUPPORTED_FORMATS,
    .rate_min       = 8000,
    .rate_max       = 384000,
    .channels_min   = 1,
    .channels_max   = 16,
    .fifo_size      = 32,
    .period_bytes_min   = 320, //S16_LE;160 samples
    .period_bytes_max   = 0xffffffff,
    .periods_min        = PURIN_BUF_ALIGN,
    .periods_max        = UINT_MAX,
    .buffer_bytes_max   = PURIN_MAX_BUFSIZE,
};

struct purin_pcm_stream
{
    struct snd_pcm_substream *tx_substream;
    struct snd_pcm_substream *rx0_substream;
    struct snd_pcm_substream *rx1_substream;
    spinlock_t pcm_lock;
};
static struct purin_pcm_stream ai_pcm;

#if defined(AUDIO_RECOVERY)
//#define AUTO_DETECT_RX_OVERRUN
void purin_buf_specific(struct snd_pcm_substream *substream);
//static int dma_tx_new_period(struct snd_pcm_substream *substream);
//static int dma_rx_new_period(struct snd_pcm_substream *substream);
extern void purin_channel_init(int stream);
extern int sysctl_ai_recovery;
enum {
    RECOVERY_NONE     = 0,
    RECOVERY_SHUTDOWN,
    RECOVERY_START,
    RECOVERY_DONE
};

enum {
    BACKUP_CFG0 = 0,
    BACKUP_CFG1 = 0,
    BACKUP_CFG2 = 0,
    BACKUP_INTR,
    BACKUP_NUM
};

struct recovery_info {
    u32 state;
    u32 reg_backup[BACKUP_NUM];  // CONF, INTR
};

static struct recovery_info recovery;

static void disable_interface(void)
{
    AIREG_UPDATE32(TX_GCR, 0, 0x00000007UL);
    AIREG_UPDATE32(RX0_GCR, 0, 0x00000007UL);
    AIREG_UPDATE32(RX1_GCR, 0, 0x00000007UL);

    if (snd_proc_info.debug_type & SND_DEBUG_RECOVERY)
        printk("audio : [dis]enabled_interface\n");
}

static void enable_interface(void)
{
    AIREG_WRITE32(TX_GCR, recovery.reg_backup[BACKUP_CFG0]);
    AIREG_WRITE32(RX0_GCR, recovery.reg_backup[BACKUP_CFG1]);
    AIREG_WRITE32(RX1_GCR, recovery.reg_backup[BACKUP_CFG2]);

    if (snd_proc_info.debug_type & SND_DEBUG_RECOVERY)
        printk("audio : [en]enabled_interface\n");
}

static struct hrtimer recovery_timer;
#if defined(CONFIG_PM)
void audio_stayawake(int is_resume);
#endif
void start_recovery_timer(void)
{
#if defined(CONFIG_PM)
    //audio_stayawake(1);
#endif
    if (snd_proc_info.debug_type & SND_DEBUG_RECOVERY)
        printk("audio : start_recovery_timer\n");
    recovery.reg_backup[BACKUP_INTR] = AIREG_READ32(INTR_CTL);
    AIREG_UPDATE32(INTR_CTL, 0, INTR_STATUS_MASK);  // it will UNMASK after RECOVERY_START
    recovery.state = RECOVERY_SHUTDOWN;
    hrtimer_start(&recovery_timer, ktime_set(0, (300 * 1000)), HRTIMER_MODE_REL);  // 300 micro sec
}

void do_backup(void)
{
    recovery.reg_backup[BACKUP_CFG0] = AIREG_READ32(TX_GCR);
    recovery.reg_backup[BACKUP_CFG1] = AIREG_READ32(RX0_GCR);
    recovery.reg_backup[BACKUP_CFG2] = AIREG_READ32(RX1_GCR);
}

void do_suspend(void)
{
#if 0 // TODO
    unsigned long reset_device_ids[] = { DEVICE_ID_PCM, 0 };
    do_backup();
#if 0 // switch mclk pinmux to gpio, try to cancel popup noise
    i2s_mclk_suspend();
#endif
    AIREG_UPDATE32(CH0_CONF, 0, 0x07);  // DISABLE TXDMA, RXDMA, CH0
    AIREG_UPDATE32(CH1_CONF, 0, 0x07);  // DISABLE TXDMA, RXDMA, CH1
    disable_interface();
    pmu_reset_devices(reset_device_ids);
#endif
}

void do_resume(void)
{
#if 0 // TODO
    unsigned long reset_device_ids[] = { DEVICE_ID_PCM, 0 };

    pmu_reset_devices(reset_device_ids);
    if (ai_pcm.tx_substream) {
//      printk(KERN_DEBUG "[%s]tx_substream\n", __func__);
        purin_buf_specific(ai_pcm.tx_substream);
        dma_tx_new_period(ai_pcm.tx_substream);
    }
    if (ai_pcm.rx0_substream) {
//      printk(KERN_DEBUG "[%s]rx0_substream\n", __func__);
        purin_buf_specific(ai_pcm.rx0_substream);
        dma_rx_new_period(ai_pcm.rx0_substream);
    }
    /* setting about channel info about pcm, i2s, bit width*/
    if (ai_pcm.tx_substream)
        purin_channel_init(SNDRV_PCM_STREAM_PLAYBACK);
    if (ai_pcm.rx0_substream)
        purin_channel_init(SNDRV_PCM_STREAM_CAPTURE);
    /* restore CFG, but we do not enable i2s/pcm here */
    AIREG_UPDATE32(CFG, recovery.reg_backup[BACKUP_CFG], 0xFFFEFFFE);
    AIREG_WRITE32(INTR_CTL, recovery.reg_backup[BACKUP_INTR]);
    recovery.state = RECOVERY_DONE;
    sysctl_ai_recovery = 0;
    udelay(100);
    enable_interface();
#if 0 // return pinmux from gpio to mclk, try to cancel popup noise
    i2s_mclk_resume();
#endif
#endif
}

static enum hrtimer_restart recovery_timer_func(struct hrtimer *timer)
{
    switch(recovery.state) {
    case RECOVERY_SHUTDOWN:
        do_suspend();
        recovery.state = RECOVERY_START;
        hrtimer_start(&recovery_timer, ktime_set(0, (10 * 1000)), HRTIMER_MODE_REL);
        break;
    case RECOVERY_START:
        do_resume();
        hrtimer_start(&recovery_timer, ktime_set(1, 0), HRTIMER_MODE_REL); // 1 s
        break;
    case RECOVERY_DONE:
        recovery.state = RECOVERY_NONE;
        break;
    default:
        break;
    }
    return HRTIMER_NORESTART;
}
#endif
#if defined(CONFIG_PM)
static atomic_t awake_flag = ATOMIC_INIT(0);
void audio_stayawake(int is_resume)
{
    struct snd_soc_pcm_runtime *rtd = ai_pcm.rx0_substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

    if (snd_proc_info.wakelock_enabled == SND_WAKELOCK_DISABLE) {
        return;
    }
    if (atomic_xchg(&awake_flag, 1) == 1) {
        return;
    }
    if (is_resume == 0 && jiffies - lastest_capture.wakelock_jiffies < snd_proc_info.wakelock_isr_skiptime) {
        atomic_set(&awake_flag, 0);
        return;
    }
    if (snd_proc_info.debug_type & SND_DEBUG_TYPE_PM)
        printk("audio_stayawake\n");
    lastest_capture.wakelock_jiffies = jiffies;
    pm_stay_awake(cpu_dai->dev);
}

void audio_relax(void)
{
    struct snd_soc_pcm_runtime *rtd = ai_pcm.rx0_substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    if (atomic_xchg(&awake_flag, 0) == 1) {
        if (snd_proc_info.debug_type & SND_DEBUG_TYPE_PM)
            printk("audio_relax\n");
        pm_relax(cpu_dai->dev);
        return;
    }
}

void audio_suspend(void)
{
    if (snd_proc_info.debug_type & SND_DEBUG_TYPE_PM)
        printk("audio_suspend\n");
#if 0 // TODO
    recovery.reg_backup[BACKUP_INTR] = AIREG_READ32(INTR_CTL);
    AIREG_UPDATE32(INTR_CTL, 0, INTR_STATUS_MASK);  // it will UNMASK after RECOVERY_START
    lastest_capture.status = LASTEST_CAPTURE_WAITING;
    do_suspend();
    recovery.state = RECOVERY_START;
#endif
    disable_irq_nosync(IRQ_PCM);
#if defined(CONFIG_PANTHER_SND_LPSD)
    /* clear LPSD status then enable voice detection interrupt */
    AIREG_WRITE32(LPSD_STS, (LPSD_LOSE_INT|LPSD_DETECT_INT));
    AIREG_WRITE32(LPSD_INT, LPSD_DETECT_INT_EN);
#endif
}

void audio_resume(void)
{
    if (ai_pcm.rx0_substream) {
        struct snd_pcm_runtime *runtime = ai_pcm.rx0_substream->runtime;
        struct purin_runtime_data *purin_rtd = runtime->private_data;

        purin_rtd->rd_ptr = AIREG_READ32(RX0_RD_POINTER0);
    }
#if defined(CONFIG_PANTHER_SND_LPSD)
    /* disable voice detection interrupt */
    AIREG_WRITE32(LPSD_INT, 0);
#endif
    enable_irq(IRQ_PCM);
    if (snd_proc_info.debug_type & SND_DEBUG_TYPE_PM)
        printk("audio_resume\n");
    //audio_stayawake(1);
#if 0 // TODO
    do_resume();
    recovery.state = RECOVERY_NONE;
    //hrtimer_start(&recovery_timer, ktime_set(0, (500)), HRTIMER_MODE_REL);  // 500ns
#endif
}
#endif

#if defined(ENABLE_NETLINK)
#define NL_AUDIO_MSG_MUTE    1
#define NL_AUDIO_MSG_UNMUTE  0

static struct sock *purin_nl_sk = NULL;
static int purin_nl_pid = -1;
static int registered_netlink = 0;

static void notify_user_mute(int mute)
{
    struct nlmsghdr *nlh;
    int msg_size;
    char *msg = NULL;
    struct sk_buff *skb_out;
    int res;

    if (purin_nl_pid < 0) {
        return;
    }

    if (mute) {
        msg = "mute";
    } else {
        msg = "unmute";
    }

    printk(KERN_DEBUG "Entering: %s\n", __FUNCTION__);
    msg_size = strlen(msg) + 1;

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    if (nlh == NULL) {
        nlmsg_free(skb_out);
        printk(KERN_INFO "Error while put msg to skb_out\n");
        return;
    }
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(purin_nl_sk, skb_out, purin_nl_pid);

    if (res < 0) {
        printk(KERN_DEBUG "Error while sending bak to user\n");
    }
}

static void purin_nl_recv_register(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    char *msg = "Success";
    int msg_size;
    int res;

    msg_size = strlen(msg);
    nlh = (struct nlmsghdr*)skb->data;
    printk(KERN_DEBUG "Netlink received msg payload:%s\n",(char*)nlmsg_data(nlh));
    purin_nl_pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    if (nlh == NULL) {
        nlmsg_free(skb_out);
        printk(KERN_INFO "Error while put msg to skb_out\n");
        return;
    }

    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);
    res = nlmsg_unicast(purin_nl_sk, skb_out, purin_nl_pid);

    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

static struct netlink_kernel_cfg purin_nl_cfg = {
    .input = purin_nl_recv_register,
};

static void init_netlink(void)
{
    if (registered_netlink != 0)
    {
        return;
    }
    purin_nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &purin_nl_cfg);
    if (!purin_nl_sk)
    {
        printk(KERN_DEBUG "Error creating socket.\n");
        return;
    }
    registered_netlink = 1;
}

static void finalize_netlink(void)
{
    if (registered_netlink == 0)
        return;

    netlink_kernel_release(purin_nl_sk);
}

#endif

#if defined(PURIN_CHECK_TX_ALL_DATA_ZERO)
static void checking_tx_data_is_prepare_playing_or_mute(struct snd_pcm_substream *substream) {
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    u32 compare_idx = 0;
    u16* src_ptr;
    int period_for_checking = purin_rtd->wr_index;
    u32 offset = 0;
    u32 frame_bytes = frames_to_bytes(runtime, runtime->period_size);

    if (purin_rtd->playback_in_mute == 1) {
        period_for_checking += 2;               // data in 20 ms later.
        period_for_checking %= runtime->periods;
    } else {
        purin_rtd->playback_data_all_zero++;
    }

    offset = frame_bytes * period_for_checking;

    src_ptr = (u16*)(runtime->dma_area + offset);
    for (compare_idx = 0; compare_idx < frame_bytes/2; ++compare_idx) {
        // frame_bytes is 16bit stereo
        if (src_ptr[compare_idx] != 0) {
            purin_rtd->playback_data_all_zero = 0;
            return;
        }
    }

}
#endif

#if 0
/**
 * fill data to tx descriptor for playing music
 */
static int dma_tx_new_period(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct purin_runtime_data *purin_rtd = runtime->private_data;

    unsigned int dma_size;
    unsigned int offset;
    dma_addr_t mem_addr;
    int ret = 0;
    if (purin_rtd->active) {
        dma_size = frames_to_bytes(runtime, runtime->period_size);
        offset = dma_size * purin_rtd->wr_index;

#if defined(PURIN_CHECK_TX_ALL_DATA_ZERO)
{
        checking_tx_data_is_prepare_playing_or_mute(substream);
        if (purin_rtd->playback_data_all_zero == 0 && purin_rtd->playback_in_mute == 1)
        {
            printk("[audio]tx unmute\n");
#if defined(ENABLE_NETLINK)
            pmu_internal_dac_enable(1);
            notify_user_mute(NL_AUDIO_MSG_UNMUTE);
#endif
            purin_rtd->playback_in_mute = 0;
#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
            purin_suspend_mic = MICARRAY_POWERSAVING_RESUME;
#endif
        }
        else if (purin_rtd->playback_data_all_zero >= 200  && purin_rtd->playback_in_mute == 0)
        {
            // at last 2 second zero data to mute dac
            // or it interrupts current playing.
            printk("[audio]tx mute\n");
#if defined(ENABLE_NETLINK)
            pmu_internal_dac_enable(0);
            notify_user_mute(NL_AUDIO_MSG_MUTE);
#endif
            purin_rtd->playback_in_mute = 1;
#if defined(CONFIG_PANTHER_MICARRAY_POWERSAVING)
            if (purin_suspend_mic == MICARRAY_POWERSAVING_NONE)
            {
                purin_suspend_mic = MICARRAY_POWERSAVING_SUSPEND;
            }
#endif
        }
}
#endif
        snd_BUG_ON(dma_size > purin_pcm_hardware.period_bytes_max);
        /* runtime->dma_addr is physical address */
        mem_addr = (dma_addr_t)(runtime->dma_addr + offset);
        AIREG_WRITE32(TX_WR_POINTER0, mem_addr);

        purin_rtd->wr_index += 1;
        purin_rtd->wr_index %= runtime->periods;
        //printk(KERN_EMERG "..purin_rtd->wr_index=%d, offset=%d", purin_rtd->wr_index, offset);

        /*--- this section aims for underrun recovery ---*/
        {
            struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

#if defined(AUDIO_RECOVERY)
            if (recovery.state == RECOVERY_START) {
                return ret;
            }
#endif
            if (likely(cpu_dai->driver->ops->trigger)) {
                ret = cpu_dai->driver->ops->trigger(substream, SNDRV_PCM_TRIGGER_START, cpu_dai);
                if (unlikely(ret < 0)) {
                    printk(KERN_ERR "re trigger failed\n");
                }
            }
        }
    }
    return ret;
}
#endif

static void audio_stop_dma(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;

    purin_rtd->active = 0;
    purin_rtd->wr_index = 0;
#if defined(UPDATE_RX_HWPTR_BY_PERIOD)
    purin_rtd->sw_index = 0;
#endif
}

/**
 * prepare rx descriptor to capture
 */
#if 0
static int dma_rx_new_period(struct snd_pcm_substream *substream){
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct purin_runtime_data *purin_rtd = runtime->private_data;

    unsigned int dma_size;
    unsigned int offset;
    dma_addr_t mem_addr;
    int ret = 0;

    if (purin_rtd->active) {
        dma_size = frames_to_bytes(runtime, runtime->period_size);
        offset = dma_size * purin_rtd->wr_index;
        snd_BUG_ON(dma_size > purin_pcm_hardware.period_bytes_max);

        mem_addr = (dma_addr_t)(runtime->dma_addr + offset);

        AIREG_WRITE32(RX0_RD_POINTER0, mem_addr);
#if 0
        pr_debug("dma_addr = 0x%x, dma_area = 0x%x\n", (unsigned int) runtime->dma_addr, (unsigned int) runtime->dma_area);
        pr_debug("mem_addr = %x, dma_size = %d\n", (unsigned int) mem_addr, dma_size);
        print_hex_dump(KERN_EMERG, "hexdump:", DUMP_PREFIX_OFFSET,
            16, 1, runtime->dma_area + offset, dma_size, false);
#endif

        purin_rtd->wr_index += 1;
        purin_rtd->wr_index %= runtime->periods;

        /* this section aims for overrun recovery */
        {
            struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

#if defined(AUDIO_RECOVERY)
            if (recovery.state == RECOVERY_START) {
                return ret;
            }
#endif
            if (cpu_dai->driver->ops->trigger) {
                ret = cpu_dai->driver->ops->trigger(substream, SNDRV_PCM_TRIGGER_START, cpu_dai);
                if (ret < 0) {
                    printk(KERN_ERR "re trigger failed\n");
                }
            }
        }
    }
    return ret;
}
#endif

static irqreturn_t tx_interrupt(struct snd_pcm_substream *substream) {
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
#if defined(CONFIG_SND_ALOOP)
    unsigned int total_offset = 0;
#endif
    unsigned long irqflags;

    if (!(purin_rtd->active)) {
        return IRQ_HANDLED;
    }

#ifdef DBG_UNDERRUN
    enter_lock_isr_jiffies = jiffies;
#endif

    snd_pcm_period_elapsed(substream);
#if defined(CONFIG_SND_ALOOP)
    total_offset += frames_to_bytes(runtime, runtime->period_size);
#endif

    /* update wr_ptr every period for avoiding hardware Tx underrun event */
    // in free-wheeling mode (dmix)
    // we don't need any Condition
    spin_lock_irqsave(&purin_rtd->lock, irqflags);
    purin_rtd->wr_index += 1;
    purin_rtd->wr_index %= runtime->periods;
    purin_rtd->wr_ptr = runtime->dma_addr + purin_rtd->wr_index * frames_to_bytes(runtime, runtime->period_size);
    AIREG_WRITE32(TX_WR_POINTER0, purin_rtd->wr_ptr);
    spin_unlock_irqrestore(&purin_rtd->lock, irqflags);

#if defined(CONFIG_SND_ALOOP)
    if (total_offset != 0) {
        start_update_pos_timer(total_offset);
    }
#endif

#ifdef DBG_UNDERRUN
    if (jiffies - enter_lock_isr_jiffies > 2) {
        printk(KERN_DEBUG "\n*********\njiffies %lu, enter_lock_isr_jiffies %lu\n********\n",
               jiffies, enter_lock_isr_jiffies);
    }
#endif
    return IRQ_HANDLED;
}

#if defined(ENABLE_SYSCALL_LASTEST_CAPTURE_DATA)
static void copy_capture_data(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    unsigned int offset = 0;
    unsigned int buffer_size = frames_to_bytes(runtime, runtime->buffer_size);
    unsigned int period_size = frames_to_bytes(runtime, runtime->period_size);

#if defined(CONFIG_PM)
    if (ktime_to_us(ktime_sub(ktime_get(), lastest_capture.req_ktime)) < AT_LEAST_WAITING_US) {
        if (lastest_capture.status == LASTEST_CAPTURE_REQUEST) {
            lastest_capture.status = LASTEST_CAPTURE_WAITING;
            if (snd_proc_info.debug_type & SND_DEBUG_TYPE_PM)
                printk("audio_waitnext_isr\n");
            return;
        }
    }
#endif
    if (lastest_capture.output_size != period_size) {
        printk("output_size %d, %d\n", lastest_capture.output_size, period_size);
        lastest_capture.output_size = (lastest_capture.output_size > period_size) ?
            period_size : lastest_capture.output_size;
    }
    if (1 == purin_rtd->is_rx1)
        offset = (AIREG_READ32(RX1_WR_POINTER0) - runtime->dma_addr + buffer_size - period_size) % buffer_size;
    else
        offset = (AIREG_READ32(RX0_WR_POINTER0) - runtime->dma_addr + buffer_size - period_size) % buffer_size;
    lastest_capture.output_buf = runtime->dma_area + offset;
    //memcpy(lastest_capture.output_buf, (runtime->dma_area + offset), period_size);
#if 0//defined(CONFIG_PM)
    audio_stayawake(0);
#endif
    lastest_capture.status = LASTEST_CAPTURE_DONE;
}
#endif

int micarray_channels = 0;
#if defined(AUTO_DETECT_RX_OVERRUN)
int autorecovey_rx_overrun(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    unsigned int buffer_size = frames_to_bytes(runtime, runtime->buffer_size);
    unsigned int period_size = frames_to_bytes(runtime, runtime->period_size);
    unsigned int offset = 0;
    int i;
    u16* rx_data;

    if (purin_rtd->is_rx1 == 1) {
        return 0;
    }

    if (micarray_channels >= 8 || micarray_channels == 0) {
        return 0; // we can not detect overrun if the number of channels >= 8
    }
    if (1 == purin_rtd->is_rx1)
        offset = (AIREG_READ32(RX1_WR_POINTER0) - runtime->dma_addr + buffer_size - period_size) % buffer_size;
    else
        offset = (AIREG_READ32(RX0_WR_POINTER0) - runtime->dma_addr + buffer_size - period_size) % buffer_size;
    rx_data = (u16*)(runtime->dma_area + offset);
    if (rx_data[0] == 0xFFFF && rx_data[1] == 0xFFFF &&
       rx_data[2] == 0xFFFF && rx_data[3] == 0xFFFF &&
       rx_data[4] == 0xFFFF && rx_data[5] == 0xFFFF &&
       rx_data[6] == 0xFFFF && rx_data[7] == 0xFFFF
       ) {
        // sometimes all 0xFFFF at startup
        return 0;
    }

    for(i = micarray_channels; i < 8; i++) {
        if (rx_data[i] != 0) {
            if (snd_proc_info.debug_type & SND_DEBUG_RECOVERY) {
                printk("recovery micarray %d\n", micarray_channels);
                print_hex_dump(KERN_DEBUG, "hexdump:", DUMP_PREFIX_OFFSET,
                               8, 2, runtime->dma_area + offset, 16, false);
            }
            sysctl_ai_recovery = 1;
            start_recovery_timer();
            return 1;
        }
    }
    return 0;
}
#endif
//static int rx_debug = 0;
static irqreturn_t rx_interrupt(struct snd_pcm_substream *substream) {
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;

    unsigned long irqflags;

#if 0
	if(rx_debug){
		printk("debug:0x%x 0x%x 0x%x 0x%x",
		*(u32*)runtime->dma_area,*(u32*)(runtime->dma_area+4),*(u32*)(runtime->dma_area+8),*(u32*)(runtime->dma_area+12));
		printk("debug:0x%x 0x%x 0x%x 0x%x",
		*(u32*)(runtime->dma_area+16),*(u32*)(runtime->dma_area+20),*(u32*)(runtime->dma_area+24),*(u32*)(runtime->dma_area+28));
		rx_debug = 0;
	}
#endif

    if (!(purin_rtd->active)) {
        return IRQ_HANDLED;
    }

#if defined(UPDATE_RX_HWPTR_BY_PERIOD)
    purin_rtd->sw_index += 1;
    purin_rtd->sw_index %= runtime->periods;
#endif
    snd_pcm_period_elapsed(substream);
    if (purin_rtd->active) {
        /* update rd_ptr every period for avoiding hardware Rx overrun event */
        spin_lock_irqsave(&purin_rtd->lock, irqflags);
        if (1 == purin_rtd->is_rx1)
            AIREG_WRITE32(RX1_RD_POINTER0, purin_rtd->rd_ptr);
        else
			#ifdef RING_BUFFER_TEST
            	AIREG_WRITE32(RX0_RD_POINTER1, purin_rtd->rd_ptr);
			#else
				AIREG_WRITE32(RX0_RD_POINTER0, purin_rtd->rd_ptr);
			#endif
        purin_rtd->rd_ptr = runtime->dma_addr + (purin_rtd->rd_ptr - runtime->dma_addr + frames_to_bytes(runtime, runtime->period_size)) % runtime->dma_bytes;
        spin_unlock_irqrestore(&purin_rtd->lock, irqflags);
    }

#if defined(ENABLE_SYSCALL_LASTEST_CAPTURE_DATA)
#if (defined(AUTO_DETECT_RX_OVERRUN))
    if (autorecovey_rx_overrun(substream) == 0)
#endif
    {
        if (lastest_capture.status != LASTEST_CAPTURE_DONE) {
            copy_capture_data(substream);
        }
    }
#endif

    return IRQ_HANDLED;
}

static irqreturn_t pcm_isr(int irq, void *dev)
{
    unsigned int status;
    irqreturn_t irq_ret = IRQ_NONE;
	#ifdef RING_BUFFER_TEST
	u32 rx0_addr0 = AIREG_READ32(RX0_ADDRESS0);
	u32 rx0_addr1 = AIREG_READ32(RX0_ADDRESS1);
	u32 rx0_wr0 = AIREG_READ32(RX0_WR_POINTER0);
	u32 rx0_buf0_finish = AIREG_READ32(RX0_WR_POINTER0) & 0x1;
	#endif

    status = AIREG_READ32(INTR_STS);
    pr_debug("status %x\n", status);
    AIREG_WRITE32(INTR_STS, status);   // clear status
    pr_debug("status %x\n", status);

    if (status & TX_FIFO_UD_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+TX_ADDRESS0), 32, false);
        pr_err("TX FIFO underrun!!!\n");
    }

    if (status & RX0_FIFO_OV_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+RX0_ADDRESS0), 32, false);
        pr_err("RX0 FIFO overrun!!!\n");
    }

    if (status & RX1_FIFO_OV_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+RX1_ADDRESS0), 32, false);
        pr_err("RX1 FIFO overrun!!!\n");
    }

    if (status & TX_RING_UD_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+TX_ADDRESS0), 32, false);
        pr_err("TX buffer underrun!!!\n");
#if defined(AUDIO_RECOVERY)
        if (snd_proc_info.debug_type & SND_DEBUG_RECOVERY)
            printk(KERN_DEBUG "IRQ UNDERRUN %x\n", status);
        sysctl_ai_recovery = 1;
#endif
    }

    if (status & RX0_RING_OV_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+RX0_ADDRESS0), 32, false);
		#ifdef RING_BUFFER_TEST
		if(rx0_addr1 == 0 || rx0_buf0_finish)//for ring buffer test
		#endif
        pr_err("RX0 buffer overrun!!!\n");
    }

    if (status & RX1_RING_OV_INT) {
        print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_ADDRESS,
                               16, 4, (void *)(AI_BASE+RX1_ADDRESS0), 32, false);
        pr_err("RX1 buffer overrun!!!\n");
    }

#if defined(AUDIO_RECOVERY)
    if ((sysctl_ai_recovery != 0) && (recovery.state == RECOVERY_NONE)) {
        start_recovery_timer();
        return IRQ_HANDLED;
    }
#endif

    if (status & TX_THD_INT) {
        if (ai_pcm.tx_substream != NULL) {
            irq_ret |= tx_interrupt(ai_pcm.tx_substream);
        } else {
            pr_err("tx_substream is NULL in pcm_isr\n");
        }
    }

    if (status & RX0_THD_INT) {
    #ifdef RING_BUFFER_TEST
    	if(AIREG_READ32(RX0_DMA_CTRL) & DMA_CTRL_BUF0_EN) {
	    	if(rx0_wr0 >= rx0_addr0 + 0x1f400) {
				AIREG_UPDATE32(RX0_DMA_CTRL, (DMA_CTRL_BUF1_EN|DMA_CTRL_ENABLE), (DMA_CTRL_BUF1_EN|DMA_CTRL_ENABLE));
				AIREG_WRITE32(RX0_RD_POINTER0, rx0_wr0);
				printk("buffer0 full: rx0_wr0=0x%x",AIREG_READ32(RX0_WR_POINTER0));
	    	}
	    } else {
	        if (ai_pcm.rx0_substream != NULL) {
	            irq_ret |= rx_interrupt(ai_pcm.rx0_substream);
	        } else {
	            pr_err("rx0_substream is NULL in pcm_isr\n");
	        }
	    }
	#else
		if (ai_pcm.rx0_substream != NULL) {
			irq_ret |= rx_interrupt(ai_pcm.rx0_substream);
		} else {
			pr_err("rx0_substream is NULL in pcm_isr\n");
		}
	#endif
    }

    if (status & RX1_THD_INT) {
        if (ai_pcm.rx1_substream != NULL) {
            irq_ret |= rx_interrupt(ai_pcm.rx1_substream);
        } else {
            pr_err("rx1_substream is NULL in pcm_isr\n");
        }
    }

    return irq_ret;
}

static int purin_dma_setup_handlers(struct snd_pcm_substream *substream)
{
    int ret = -ENODEV;
    int stream_type = substream->stream;
    int is_tx = (stream_type == SNDRV_PCM_STREAM_PLAYBACK) ? 1 : 0;
//  const char * irq_name = (is_tx ? "AI_TX" : "AI_RX");
    unsigned long irqflags;
    struct purin_runtime_data *purin_rtd = substream->runtime->private_data;

    if (stream_type > SNDRV_PCM_STREAM_LAST) {
        printk(KERN_CRIT "Can't register IRQ %d for unsupported stream_type %d\n", IRQ_PCM, stream_type);
        return ret;
    }

    spin_lock_irqsave(&(ai_pcm.pcm_lock), irqflags);
    if ((NULL == ai_pcm.tx_substream) && (NULL == ai_pcm.rx0_substream) && (NULL == ai_pcm.rx1_substream)) {
        if (is_tx) {
            ai_pcm.tx_substream = substream;
        } else if (1 == purin_rtd->is_rx1) {
            ai_pcm.rx1_substream = substream;
        } else {
            ai_pcm.rx0_substream = substream;
        }
        ret = request_irq(IRQ_PCM, pcm_isr, IRQF_SHARED, "AI_PCM", (void *)&ai_pcm);
        if (unlikely(ret)) {
            printk(KERN_CRIT "Can't register IRQ %d for DMA stream_type %d\n", IRQ_PCM, stream_type);
            if (is_tx) {
                ai_pcm.tx_substream = NULL;
            } else if (1 == purin_rtd->is_rx1) {
                ai_pcm.rx1_substream = NULL;
            } else {
                ai_pcm.rx0_substream = NULL;
            }
        }
    } else {
        if (is_tx) {
            if (ai_pcm.tx_substream == NULL) {
                ai_pcm.tx_substream = substream;
                ret = 0;
            } else {
                pr_err("tx_substream duplicate irq request\n");
            }
        } else if (1 == purin_rtd->is_rx1) {
            if (ai_pcm.rx1_substream == NULL) {
                pr_debug("rx1_substream\n");
                ai_pcm.rx1_substream = substream;
                ret = 0;
            } else {
                pr_err("rx1_substream duplicate irq request\n");
            }
        } else {
            if (ai_pcm.rx0_substream == NULL) {
                ai_pcm.rx0_substream = substream;
                ret = 0;
            } else {
                pr_err("rx0_substream duplicate irq request\n");
            }
        }
        //printk(KERN_NOTICE "IRQ %d is registered, ignore DMA stream_type %s\n", IRQ_PCM, irq_name);
    }
    spin_unlock_irqrestore(&(ai_pcm.pcm_lock), irqflags);

    return ret;
}

/**
 * when start playing music, (aplay or others)
 */
static int purin_pcm_open(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd;
    int ret;

    /*
     *  set a step constraint for satisfying the limit of ring buffer size
     */
    ret = snd_pcm_hw_constraint_step(runtime, 0, SNDRV_PCM_HW_PARAM_PERIODS, PURIN_BUF_ALIGN);
    if (unlikely(ret < 0))
        goto out;

    ret = snd_pcm_hw_constraint_pow2(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS);
    if (unlikely(ret < 0))
        goto out;

    ret = snd_soc_set_runtime_hwparams(substream, &purin_pcm_hardware);
    if (unlikely(ret < 0))
        goto out;

    ret = -ENOMEM;
    purin_rtd = kzalloc(sizeof(struct purin_runtime_data), GFP_KERNEL);
    if (unlikely(!purin_rtd))
        goto out;

    spin_lock_init(&purin_rtd->lock);
    runtime->private_data = purin_rtd;

#if defined(ENABLE_SYSCALL_LASTEST_CAPTURE_DATA)
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        lastest_capture.status = LASTEST_CAPTURE_DONE;
#endif

#ifdef CONFIG_PCMGPIOCONTROL
    INIT_DELAYED_WORK(&purin_rtd->pcm_event.gpioctrl, purin_pcm_event_hook);
#endif
    return 0;

 out:
    return ret;
}

#if defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
static int purin_pcm_external_open(struct snd_pcm_substream *substream)
{
    int ret = purin_pcm_open(substream);
    struct purin_runtime_data *purin_rtd;
    if (unlikely(ret < 0))
        goto out;

    purin_rtd = substream->runtime->private_data;
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {//to run RX0 RX1 at the same time for test.
		if(AIREG_READ32(RX0_ADDRESS0)) {
        	purin_rtd->is_rx1 = 1;		
			printk("select rx1");
		} else {
			purin_rtd->is_rx1 = 0;
			printk("select rx0");
		}
    }

    /* setup handlers for interrupt*/
    ret = purin_dma_setup_handlers(substream);
    if (unlikely(ret < 0)) {
        printk(KERN_ERR "ERR: Error %d setting interrupt function\n", ret);
        goto err1;
    }

#if defined(CONFIG_PM)
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
        //audio_stayawake(0);
    }
#endif

    return 0;

err1:
    kfree(purin_rtd);
out:
    return ret;
}
#endif

static int purin_pcm_internal_open(struct snd_pcm_substream *substream)
{
    int ret = purin_pcm_open(substream);
    struct purin_runtime_data *purin_rtd;
    if (unlikely(ret < 0))
        goto out;

    purin_rtd = substream->runtime->private_data;
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		if(AIREG_READ32(RX0_ADDRESS0)){
        	purin_rtd->is_rx1 = 1;		
			printk("select rx1");
		} else {
			purin_rtd->is_rx1 = 0;
			printk("select rx0");
		}
    }

    /* setup handlers for interrupt*/
    ret = purin_dma_setup_handlers(substream);
    if (unlikely(ret < 0)) {
        printk(KERN_ERR "ERR: Error %d setting interrupt function\n", ret);
        goto err1;
    }
    return 0;

err1:
    kfree(purin_rtd);
out:
    return ret;
}

/**
 * when stop playing music, (aplay or others)
 */
static int purin_pcm_close(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    unsigned long irqflags;
    int ret = -EINVAL;
    spin_lock_irqsave(&(ai_pcm.pcm_lock), irqflags);
    pr_debug("start %d\n", substream->stream);
    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        if (ai_pcm.tx_substream == substream) {
            ai_pcm.tx_substream = NULL;
            ret = 0;
        } else {
            pr_err("tx_substream not equal\n");
        }
    } else if (1 == purin_rtd->is_rx1) {
        if (ai_pcm.rx1_substream == substream) {
            ai_pcm.rx1_substream = NULL;
            ret = 0;
        } else {
            pr_err("rx1_substream not equal\n");
        }
    } else {
        if (ai_pcm.rx0_substream == substream) {
            ai_pcm.rx0_substream = NULL;
            ret = 0;
        } else {
            pr_err("rx0_substream not equal\n");
        }
    }

    if ((NULL == ai_pcm.tx_substream) && (NULL == ai_pcm.rx0_substream) && (NULL == ai_pcm.rx1_substream)) {
        pr_debug("free irq %d\n", IRQ_PCM);
        free_irq(IRQ_PCM, (void *)&ai_pcm);
    }
    spin_unlock_irqrestore(&(ai_pcm.pcm_lock), irqflags);

    kfree(purin_rtd);
    return ret;
}
#define MAX_SUBSTREAM_NUM 5
struct substream_dma_info{
	struct snd_pcm_substream * substream;
	dma_addr_t dma_addr;
	unsigned char* dma_area;
};
//static struct substream_dma_info pre_substream_dma[MAX_SUBSTREAM_NUM] = {0};
static int purin_pcm_hw_params_common_part(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    int ret = 0;
	//int i = 0;
	//struct substream_dma_info* pre_dma_info = NULL;
    struct snd_pcm_runtime *runtime = substream->runtime;
    size_t buffer_size = params_buffer_bytes(params);

    if (buffer_size & RBUF_SIZE_RESERVED_BITS) {
        pr_err("buffer_bytes(0x%x) isn't the multiple of 32\n", buffer_size);
        return -EINVAL;
    }

    /* copy buffer information to runtime->dma_buffer */
    substream->dma_buffer.dev.type = SNDRV_DMA_TYPE_DEV;
	ret = snd_pcm_lib_malloc_pages(substream, buffer_size);

#if 0
#if 1
	for(i=0; i<MAX_SUBSTREAM_NUM; i++){
		pre_dma_info = &pre_substream_dma[i];
		if(pre_dma_info->substream == NULL)
			break;

		if(pre_dma_info->substream == substream){
			printk("find pre dma info,idx:%d",i);
			break;
		}
		if(i==MAX_SUBSTREAM_NUM-1){
			printk("substream nums is more than %d",MAX_SUBSTREAM_NUM);
			return -ENOMEM;
		}
	}
	if(pre_dma_info->substream == NULL){
    	ret = snd_pcm_lib_malloc_pages(substream, buffer_size);
		pre_dma_info->substream = substream;
		pre_dma_info->dma_area = runtime->dma_area;
		pre_dma_info->dma_addr = runtime->dma_addr;
	}else{
		struct snd_dma_buffer *dmab = NULL;
		dmab = kzalloc(sizeof(*dmab), GFP_KERNEL);
		if (! dmab)
			return -ENOMEM;
		dmab->dev = substream->dma_buffer.dev;
		dmab->dev.type = substream->dma_buffer.dev.type;
		dmab->dev.dev = substream->dma_buffer.dev.dev;
		dmab->area = pre_dma_info->dma_area;
		dmab->addr = pre_dma_info->dma_addr;
		dmab->bytes = buffer_size;

		snd_pcm_set_runtime_buffer(substream, dmab);
		runtime->dma_bytes = dmab->bytes;
	}
    //ret = snd_pcm_lib_malloc_pages(substream, SRAM_BUFFER_SIZE);
   	//runtime->dma_buffer_p = 0x10;
	//runtime->dma_area = 0x10;
    //runtime->dma_bytes = buffer_size;
	//runtime->dma_addr = 0xbe800000;//ioremap(0xbe800000, buffer_size);
	//runtime->dma_area = ioremap(0xbe800000, buffer_size);

#else

	{
		struct snd_dma_buffer *dmab = NULL;
		dmab = kzalloc(sizeof(*dmab), GFP_KERNEL);
		if (! dmab)
			return -ENOMEM;
		dmab->dev = substream->dma_buffer.dev;
		dmab->dev.type = substream->dma_buffer.dev.type;
		dmab->dev.dev = substream->dma_buffer.dev.dev;
		dmab->area = ioremap(0xf000000, buffer_size);//ioremap(0x12e00000, buffer_size);
		dmab->addr = 0xf000000;//0x12e00000;
		dmab->bytes = buffer_size;

		snd_pcm_set_runtime_buffer(substream, dmab);
		runtime->dma_bytes = dmab->bytes;
	}
#endif
#endif

    pr_debug("%s: snd_purin_audio_hw_params runtime->dma_addr 0x(%x)\n",
        __func__, (unsigned int)runtime->dma_addr);
    pr_debug("%s: snd_purin_audio_hw_params runtime->dma_area 0x(%x)\n",
        __func__, (unsigned int)runtime->dma_area);
    pr_debug("%s: snd_purin_audio_hw_params runtime->dma_bytes %d\n",
        __func__, (unsigned int)runtime->dma_bytes);

    return ret;
}

static int purin_pcm_hw_params(struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
    int ret = purin_pcm_hw_params_common_part(substream, params);
    if (ret < 0) {
        return ret;
    }

#if defined(CONFIG_PANTHER_SND_LPSD) && defined(CONFIG_PANTHER_SND_LPSD_TARGET)
    if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
        struct snd_pcm_runtime *runtime = substream->runtime;
        struct purin_runtime_data *purin_rtd = runtime->private_data;

        if (CONFIG_PANTHER_SND_LPSD_TARGET == purin_rtd->is_rx1) {
            int format = params_format(params);
            int width = snd_pcm_format_width(format);
            int rate = params_rate(params);
# define LPSD_WIDTH 16
# define LPSD_BASE_RATE 16000

            if ((LPSD_WIDTH == width) && (0 == (rate%LPSD_BASE_RATE))) {
                int period = (params_channels(params)/2)*(rate/LPSD_BASE_RATE); //unit: word

                if (format == SNDRV_PCM_FORMAT_S16_4LE) { //for PDM
                    AIREG_UPDATE32(LPSD_PARAM2, LPSD_DC_FILTER, LPSD_DC_FILTER);
                    period <<= 1;
                }
                AIREG_UPDATE32(LPSD_CTL, period<<8, LPSD_FRAME_SIZE);
# if (CONFIG_PANTHER_SND_LPSD_TARGET==1)
                AIREG_UPDATE32(LPSD_CTL, LPSD_SEL_RX1, LPSD_SEL_RX1);
# else
                AIREG_UPDATE32(LPSD_CTL, 0, LPSD_SEL_RX1);
# endif
            }
            else {
                pr_err("LPSD can't work in this settings\n");
                return -EINVAL;
            }
        }
    }
#endif

    return 0;
}

static int purin_pcm_hw_free(struct snd_pcm_substream *substream)
{
	int ret = 0;
    pr_debug("%s:%d\n", __func__, __LINE__);
    //ret = snd_pcm_lib_free_pages(substream);
	return ret;
}

static int purin_pcm_prepare(struct snd_pcm_substream *substream)
{
    struct purin_runtime_data *purin_rtd = substream->runtime->private_data;

    purin_rtd->wr_index = 0;
#if defined(UPDATE_RX_HWPTR_BY_PERIOD)
    purin_rtd->sw_index = 0;
#endif

    return 0;
}

/* initialize ring buffer registers */
void purin_buf_specific(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;

    if (SNDRV_PCM_STREAM_PLAYBACK == substream->stream) {
        AIREG_WRITE32(TX_ADDRESS0, runtime->dma_addr);
        AIREG_WRITE32(TX_LEN0, runtime->dma_bytes);
        AIREG_WRITE32(INT_THD_TX, frames_to_bytes(runtime, runtime->period_size)>>2);
		//printk("wei debug config dma addr:0x%x, dma area:0x%x",runtime->dma_addr,runtime->dma_area);
    } else {
        if (purin_rtd->is_rx1) {
			//printk("wei debug config rx1 dma:0x%x",runtime->dma_addr);
            AIREG_WRITE32(RX1_ADDRESS0, runtime->dma_addr);
            AIREG_WRITE32(RX1_LEN0, runtime->dma_bytes);
            AIREG_WRITE32(INT_THD_RX1, frames_to_bytes(runtime, runtime->period_size)>>2);
        } else {
			#ifdef RING_BUFFER_TEST
			//printk("wei debug config rx0 dma:0x%x, area:0x%x",runtime->dma_addr,runtime->dma_area);
            AIREG_WRITE32(RX0_ADDRESS0, 0xBE800000);//for ring buffer test
            AIREG_WRITE32(RX0_LEN0, 0X20000);//for ring buffer test

            AIREG_WRITE32(RX0_ADDRESS1, runtime->dma_addr);
            AIREG_WRITE32(RX0_LEN1, runtime->dma_bytes);
            AIREG_WRITE32(INT_THD_RX0, frames_to_bytes(runtime, runtime->period_size)>>2);
			#else
			//printk("wei debug config rx0 dma:0x%x, area:0x%x",runtime->dma_addr,runtime->dma_area);
            AIREG_WRITE32(RX0_ADDRESS0, runtime->dma_addr);
            AIREG_WRITE32(RX0_LEN0, runtime->dma_bytes);
            AIREG_WRITE32(INT_THD_RX0, frames_to_bytes(runtime, runtime->period_size)>>2);
			#endif
        }
    }
}

#ifdef CONFIG_PCMGPIOCONTROL
static void purin_pcm_event_hook(struct work_struct *work)
{
    struct purin_pcm_event *pcm_event = (struct purin_pcm_event *)container_of(work, struct purin_pcm_event, gpioctrl.work);
    struct kobj_uevent_env *env = kzalloc(sizeof(struct kobj_uevent_env), GFP_KERNEL);
    char *argv[3] = {
        hotplug_path,
        "pcm",
        NULL
    };

    if (!env) {
        pr_debug("[%s:%d]pcm ENOMEM\n",__func__,__LINE__);
        return;
    }

    if (add_uevent_var(env, "ACTION=%s", (pcm_event->action) ? "stop" : "start")) {
        pr_debug("[%s:%d]pcm ENOMEM\n",__func__,__LINE__);
        kfree(env);
        return;
    }

    call_usermodehelper(argv[0], argv, env->envp, UMH_WAIT_EXEC);
    kfree(env);
}
#endif

#ifdef PURIN_DEBUG
static void purin_print_trigger_cmd(int cmd)
{
    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
        pr_debug("SNDRV_PCM_TRIGGER_START\n");
        break;
    case SNDRV_PCM_TRIGGER_STOP:
        pr_debug("SNDRV_PCM_TRIGGER_STOP\n");
        break;
    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
        pr_debug("SNDRV_PCM_TRIGGER_PAUSE_PUSH\n");
        break;
    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
        pr_debug("SNDRV_PCM_TRIGGER_PAUSE_RELEASE\n");
        break;
    case SNDRV_PCM_TRIGGER_SUSPEND:
        pr_debug("SNDRV_PCM_TRIGGER_SUSPEND\n");
        break;
    case SNDRV_PCM_TRIGGER_RESUME:
        pr_debug("SNDRV_PCM_TRIGGER_RESUME\n");
        break;
    default:
        pr_debug("NO SUCH COMMAND\n");
    }
}
#endif

static int purin_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    int ret = 0;
    unsigned long irqflags;

    #ifdef PURIN_DEBUG
    purin_print_trigger_cmd(cmd);
    #endif

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
        spin_lock_irqsave(&purin_rtd->lock, irqflags);
        purin_rtd->active = 1;
        if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
            /* gpio control */
            #ifdef CONFIG_PCMGPIOCONTROL
                purin_rtd->pcm_event.action = 0;
                schedule_delayed_work(&purin_rtd->pcm_event.gpioctrl, 1);
                pr_debug("%s:%d requested stream startup \n", __func__, __LINE__);
            #endif

            #if defined (CFG_IO_AUDIO_AMP_POWER)
                if (!gpio_status) {
                    pr_debug("%s:%d GPIO%d Audio AMP Power On\n", __func__, __LINE__, AUDIO_AMP_POWER);
                    gpio_set_value(AUDIO_AMP_POWER, AUDIO_AMP_ACTIVE_LEVEL);
                }
            #endif

            purin_rtd->wr_index = 2;
            purin_rtd->wr_ptr = runtime->dma_addr + purin_rtd->wr_index * frames_to_bytes(runtime, runtime->period_size);
            AIREG_WRITE32(TX_WR_POINTER0, purin_rtd->wr_ptr);
        } else{
            purin_rtd->rd_ptr = runtime->dma_addr;
            if (1 == purin_rtd->is_rx1) {
                AIREG_WRITE32(RX1_RD_POINTER0, purin_rtd->rd_ptr);
            } else {
				#ifdef RING_BUFFER_TEST
                //AIREG_WRITE32(RX0_RD_POINTER0, purin_rtd->rd_ptr);
                AIREG_WRITE32(RX0_RD_POINTER1, runtime->dma_addr);
				AIREG_WRITE32(RX0_RD_POINTER0, 0xBE800000);
				#else
				AIREG_WRITE32(RX0_RD_POINTER0, purin_rtd->rd_ptr);
				#endif
            }
        }
        spin_unlock_irqrestore(&purin_rtd->lock, irqflags);
        break;

    case SNDRV_PCM_TRIGGER_STOP:
        spin_lock_irqsave(&purin_rtd->lock, irqflags);
        /* requested stream shutdown */
        audio_stop_dma(substream);
        spin_unlock_irqrestore(&purin_rtd->lock, irqflags);

        /* gpio control */
        #ifdef CONFIG_PCMGPIOCONTROL
            pr_debug("%s:%d requested stream stutdown \n", __func__, __LINE__);
            purin_rtd->pcm_event.action = 1;
            schedule_delayed_work(&purin_rtd->pcm_event.gpioctrl, 1);
        #endif

        #if defined (CFG_IO_AUDIO_AMP_POWER)
            if (!gpio_status) {
                pr_debug("%s:%d GPIO%d Audio AMP Power Off\n", __func__, __LINE__, AUDIO_AMP_POWER);
                gpio_set_value(AUDIO_AMP_POWER, !AUDIO_AMP_ACTIVE_LEVEL);
            }
        #endif
        break;

    default:
        ret = -EINVAL;
    }

    return ret;
}

/**
 * determine the current position of the DMA transfer
 * called by snd_pcm_period_elapsed()
 */
static snd_pcm_uframes_t purin_pcm_pointer(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct purin_runtime_data *purin_rtd = runtime->private_data;
    unsigned int offset = 0;

    if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        offset = AIREG_READ32(TX_RD_POINTER0) - runtime->dma_addr;
    } else {
#if defined(UPDATE_RX_HWPTR_BY_PERIOD)
        offset = (runtime->period_size * (purin_rtd->sw_index));
#else
        if (1 == purin_rtd->is_rx1) {
            offset = AIREG_READ32(RX1_WR_POINTER0) - runtime->dma_addr;
        } else {
			#ifdef RING_BUFFER_TEST
			offset = AIREG_READ32(RX0_WR_POINTER1) - runtime->dma_addr;
			#else
            offset = AIREG_READ32(RX0_WR_POINTER0) - runtime->dma_addr;
			#endif
        }
#endif
    }

    offset = bytes_to_frames(runtime, offset);
    if (offset >= runtime->buffer_size) {
        pr_debug("runtime:periods=%d \nperiod_size=%d \nbuffer_size=%d \n offset=%d\n",
                 runtime->periods, (int)runtime->period_size, (int)runtime->buffer_size,
                 offset);
        offset = 0;
    }

    return offset;
}

#if defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
static struct snd_pcm_ops purin_external_ops = {
    .open       = purin_pcm_external_open,
    .close      = purin_pcm_close,
    .ioctl      = snd_pcm_lib_ioctl,
    .hw_params  = purin_pcm_hw_params,
    .hw_free    = purin_pcm_hw_free,
    .prepare    = purin_pcm_prepare,
    .trigger    = purin_pcm_trigger,
    .pointer    = purin_pcm_pointer,
    .mmap       = snd_pcm_lib_mmap_iomem,
};
#endif

static struct snd_pcm_ops purin_internal_ops = {
    .open       = purin_pcm_internal_open,
    .close      = purin_pcm_close,
    .ioctl      = snd_pcm_lib_ioctl,
    .hw_params  = purin_pcm_hw_params,
    .hw_free    = purin_pcm_hw_free,
    .prepare    = purin_pcm_prepare,
    .trigger    = purin_pcm_trigger,
    .pointer    = purin_pcm_pointer,
    .mmap       = snd_pcm_lib_mmap_iomem,
};

static u64 purin_dma_dmamask = DMA_BIT_MASK(32);
static int purin_soc_dma_new(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_card *card = rtd->card->snd_card;
    int ret = 0;

    /* tell the driver that we only support 32bit addr_phy */
    if (!card->dev->dma_mask)
        card->dev->dma_mask = &purin_dma_dmamask;
    if (!card->dev->coherent_dma_mask)
        card->dev->coherent_dma_mask = DMA_BIT_MASK(32);

#if defined(INTERNAL_CLOCK_AUTO_GATING)
    /* enable auto gating function */
    AIREG_UPDATE32(ICLK_GATE_CTL, 0, (AHB_ICLK_NO_GATE|AXI_ICLK_NO_GATE));
	//AIREG_UPDATE32(ICLK_GATE_CTL, 0x803, (AHB_ICLK_NO_GATE|AXI_ICLK_NO_GATE));

#endif

#if defined(CONFIG_SRAM_AUDIO_ENABLE)
    /* it will panic if total size we need is not available*/
    check_if_size_available_for_sram();
#endif

    spin_lock_init(&(ai_pcm.pcm_lock));

    return ret;
}

static void purin_soc_dma_free(struct snd_pcm *pcm)
{
}

#if defined(CONFIG_PANTHER_SND_RX_EXT_CARD) || defined(CONFIG_PANTHER_SND_TX_EXT_CARD)
/*
 * about pcm/i2s for external codec
 */
static struct snd_soc_platform_driver panther_external_platform = {
    .ops        = &purin_external_ops,
    .pcm_new    = purin_soc_dma_new,
    .pcm_free   = purin_soc_dma_free,
};

static int panther_external_platform_probe(struct platform_device *pdev)
{
#if defined (CFG_IO_AUDIO_AMP_POWER)
    if (!(gpio_status = gpio_request(AUDIO_AMP_POWER, "purin_amp_en"))) {
        gpio_direction_output(AUDIO_AMP_POWER, !AUDIO_AMP_ACTIVE_LEVEL);
    } else {
        printk(KERN_ERR "ERR: request gpio[%d] for Audio AMP Power\n", AUDIO_AMP_POWER);
    }
#endif
    return snd_soc_register_platform(&pdev->dev, &panther_external_platform);
}

static int panther_external_platform_remove(struct platform_device *pdev)
{
#if defined (CFG_IO_AUDIO_AMP_POWER)
    if (!gpio_status) {
        gpio_set_value(AUDIO_AMP_POWER, !AUDIO_AMP_ACTIVE_LEVEL);
        gpio_free(AUDIO_AMP_POWER);
        gpio_status = -1;
    }
#endif
#if defined(ENABLE_NETLINK)
    finalize_netlink();
#endif
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}

struct platform_device panther_audio_dma = {
    .name       = "purin-external-dma",
    .id         = -1,
};

static struct platform_driver panther_external_dma_platform = {
    .driver = {
            .name = "purin-external-dma",
    },
    .probe = panther_external_platform_probe,
    .remove = panther_external_platform_remove,
};

static int __init panther_external_dma_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_audio_dma);
    if (ret != 0) {
        pr_err("Failed to register purin-external-dma device: %d\n", ret);
        platform_device_put(&panther_audio_dma);
        return ret;
    }

    return platform_driver_register(&panther_external_dma_platform);
}

static void __exit panther_external_dma_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&panther_external_dma_platform);
}

//module_platform_driver(panther_external_dma_platform);
module_init(panther_external_dma_init);
module_exit(panther_external_dma_exit);
#endif

/*
 * about pcm/i2s for internal adc/dac
 */
static struct snd_soc_platform_driver panther_internal_platform = {
    .ops        = &purin_internal_ops,
    .pcm_new    = purin_soc_dma_new,
    .pcm_free   = purin_soc_dma_free,
};

static int panther_internal_platform_probe(struct platform_device *pdev)
{
    #if defined(ENABLE_NETLINK)
    init_netlink();
    #endif
    #if defined(AUDIO_RECOVERY)
    hrtimer_init(&recovery_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    recovery_timer.function = recovery_timer_func;
    #endif
    return snd_soc_register_platform(&pdev->dev, &panther_internal_platform);
}

static int panther_internal_platform_remove(struct platform_device *pdev)
{
    #if defined(ENABLE_NETLINK)
    finalize_netlink();
    #endif
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}

struct platform_device panther_dac_dma = {
    .name       = "purin-internal-dma",
    .id     = -1,
};

static struct platform_driver panther_internal_dma_platform = {
    .driver = {
            .name = "purin-internal-dma",
    },
    .probe = panther_internal_platform_probe,
    .remove = panther_internal_platform_remove,
};

static int __init panther_internal_dma_init(void)
{
	int ret = 0;

    pr_debug("in function %s\n", __func__);
    ret = platform_device_register(&panther_dac_dma);
    if (ret != 0) {
        pr_err("Failed to register purin-internal-dma device: %d\n", ret);
        platform_device_put(&panther_dac_dma);
        return ret;
    }

    return platform_driver_register(&panther_internal_dma_platform);
}

static void __exit panther_internal_dma_exit(void)
{
    pr_debug("in function %s\n", __func__);
    platform_driver_unregister(&panther_internal_dma_platform);
}
//module_platform_driver(panther_internal_dma_platform);
module_init(panther_internal_dma_init);
module_exit(panther_internal_dma_exit);
/*
 *
 */
MODULE_AUTHOR("Edden Tsai");
MODULE_DESCRIPTION("Panther AUDIO DMA module");
MODULE_LICENSE("GPL");
