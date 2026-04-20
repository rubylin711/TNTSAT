/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _PURIN_DMA_H
#define _PURIN_DMA_H

struct purin_runtime_data {
    struct purin_device *device;
    int active;
#if defined(UPDATE_RX_HWPTR_BY_PERIOD)
    int sw_index; /* write pointer index for Rx */
#endif
    int wr_index;
    dma_addr_t wr_ptr; /* write pointer physical address for Tx*/
    dma_addr_t rd_ptr; /* read pointer physical address for Rx */

    spinlock_t lock;

    int is_rx1;

    int playback_in_mute;
    int playback_data_all_zero;

#ifdef CONFIG_PCMGPIOCONTROL
    struct purin_pcm_event pcm_event;
#endif

};

#define PURIN_BUF_ALIGN    32
#define PURIN_MAX_BUFSIZE  4*8*(384000*1) //S32_LE; 8 channel; 1s for 384KHz

#ifdef CONFIG_PCMGPIOCONTROL
struct purin_pcm_event{
    int action;                    // start:0, stop:1;
    struct delayed_work gpioctrl;
};
#endif

#if defined (CONFIG_GPIO_AUDIO_AMP_POWER) && (CONFIG_GPIO_AUDIO_AMP_POWER_NUM)

    #define CFG_IO_AUDIO_AMP_POWER CONFIG_GPIO_AUDIO_AMP_POWER_NUM
    #define AUDIO_AMP_POWER CFG_IO_AUDIO_AMP_POWER

    #if defined (CONFIG_GPIO_AUDIO_AMP_POWER_LOW_ACTIVE)
        #define AUDIO_AMP_ACTIVE_LEVEL 0
    #else
        #define AUDIO_AMP_ACTIVE_LEVEL 1
    #endif

    int purin_spk_on(int enable);

#endif

void purin_buf_specific(struct snd_pcm_substream *substream);

#if defined(CONFIG_SND_ALOOP)
void start_update_pos_timer(unsigned int period_byte_size);
#endif

//#define AUDIO_RECOVERY

#endif
