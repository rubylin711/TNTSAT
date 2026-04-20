/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#if defined(CONFIG_SRAM_AUDIO_ENABLE)
#include <linux/module.h>
#include <sound/pcm.h>
#include "purin-dma.h"
#include "mach/symphony_regs.h"
#include "mach/symphony_reg_base_addr.h"

#define PURIN_PERIOD_NUM 24
#define SRAM_PERIOD_SIZE 0xA00U
#define SRAM_ADDRESS_LIMIT      (SRAM_REGION_START + SRAM_REGION_SIZE)
#define SRAM_ADDRESS_BASE       SRAM_REGION_START
#define SRAM_AUDIO_ADDR_BASE    SRAM_ADDRESS_BASE
#define SRAM_AVAILABLE_SIZE     (SRAM_ADDRESS_LIMIT - SRAM_ADDRESS_BASE)

#define SRAM_AUDIO_RX_BASE      SRAM_ADDRESS_BASE
#define SRAM_AUDIO_RX_SIZE      (SRAM_PERIOD_SIZE * PURIN_PERIOD_NUM)

#define SRAM_AUDIO_TX_BASE      (SRAM_AUDIO_RX_BASE + SRAM_AUDIO_RX_SIZE)
#define SRAM_AUDIO_TX_SIZE      (SRAM_PERIOD_SIZE * PURIN_PERIOD_NUM)

#if defined(CONFIG_SRAM_AUDIO_ENABLE)
unsigned int audio_dma_sram_size = SRAM_AVAILABLE_SIZE;
#else
unsigned int audio_dma_sram_size = 0;
#endif

void check_if_size_available_for_sram(void) {
    u32 audio_usage = SRAM_AUDIO_TX_BASE + SRAM_AUDIO_TX_SIZE;
    printk(KERN_EMERG "available for sram %x\n", audio_usage);
    if (audio_usage - SRAM_AUDIO_ADDR_BASE > SRAM_AVAILABLE_SIZE)
    {
        panic("exceed the limit of sram in audio data");
    }
}

void *alloc_audio_dma(dma_addr_t* phy_addr, int stream)
{
    void *alloc = 0;
    u32 addr = SRAM_AUDIO_RX_BASE;
    if (stream == SNDRV_PCM_STREAM_PLAYBACK)
    {
        addr = SRAM_AUDIO_TX_BASE;
    }
    alloc = (void*)addr;
    *phy_addr = (dma_addr_t)alloc;

    return (void *) SYMPHONY_IO_VA((dma_addr_t)alloc);
}
#endif
