/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#define MODULE_TAG "[MTAV PLT]"
#include "stream_filter_os_support.h"

#ifdef __LINUX__
#include <pthread.h>
#include "mt_common.h"
#include "mt_unf_dma.h"
static pthread_mutex_t audio_dma_mutex = PTHREAD_MUTEX_INITIALIZER;
#define OS_AUDIO_LOCK()     pthread_mutex_lock(&audio_dma_mutex)
#define OS_AUDIO_UNLOCK()   pthread_mutex_unlock(&audio_dma_mutex)
#else
#define OS_AUDIO_LOCK()     do{}while(0)
#define OS_AUDIO_UNLOCK()   do{}while(0)
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if MT_DES("Internal Data and Structure", 1)



#endif

#if MT_DES("Internal Function", 1)

#ifdef CONFIG_MT_MONTAGE_PLATFORM
static void dump_hex(char * title, unsigned char *buf, int len)
{
    int i = 0;
    MTAVSF_LOG("%s:", title);
    while (len--){
        MTAVSF_LOG("%02x", buf[i++]);
    }

    MTAVSF_LOG("\n");
}

static int aud_is_secure_buffer(void)
{
    static unsigned int flag = 0xa5a5a5a5;
    phys_addr_t pcm_max  = 0;
    phys_addr_t ddr_info = 0;
    unsigned long size   = 0;
    if (flag == 0xa5a5a5a5){
        mt_mmz_get_start_size("pcm", &ddr_info, &size);
        pcm_max = ddr_info + size;
        mt_mmz_get_start_size("adec_es", &ddr_info, &size);
        if (ddr_info < pcm_max) {
            flag = 1;
        } else {
            flag = 0;
        }
    }
    return flag;
}

static void *aud_dmacpy_tee(void *dst, const void *src, size_t size)
{
#define AUD_DMA_CHANNEL_ID  6
    int ret = -1;
    phys_addr_t psrc = 0;
    phys_addr_t pdst = 0;
    unsigned char *vsrc_mmz = NULL;
    const unsigned int ch = AUD_DMA_CHANNEL_ID;

    if((NULL == dst) || (NULL == src)) {
        MTAVSF_LOG(MODULE_TAG"Audio dam copy para error\n");
        return NULL;
    }

    OS_AUDIO_LOCK();

    while (MT_EDMA_STATUS_FREE != mt_unf_dma_check((MT_EDMA_CH_E)ch)) {
        MTAVSF_MSLEEP(10);
    }

    mt_unf_dma_release_channel((MT_EDMA_CH_E)ch);

    ret = mt_unf_dma_request_channel((MT_EDMA_CH_E)ch);
    if (ret != MTAVSF_SUCCESS) {
        MTAVSF_LOG(MODULE_TAG"DMA copy channel is busy!\n");
        ret = MTAVSF_FAILURE;
        goto exit;
    }

    psrc = mt_mmz_new(size, 0, "ddr",  "aud_mmz_buf");
    if(0 != psrc){
        /* Copy not continuous address to mmz zone */
        vsrc_mmz = (unsigned char *) mt_mmz_map(psrc, 0);
        if (NULL == vsrc_mmz) {
            ret = MTAVSF_FAILURE;
            MTAVSF_LOG(MODULE_TAG"MMZ Map ERROR\n");
            goto exit;
        }
        memcpy(vsrc_mmz, (const void *)src, size);
    } else {
        ret = MTAVSF_FAILURE;
        MTAVSF_LOG(MODULE_TAG"New MMZ ERROR\n");
        goto exit;
    }

    ret = mt_mem_get_phyaddr((void *)dst, &pdst);
    if(ret != MTAVSF_SUCCESS){
        MTAVSF_LOG(MODULE_TAG"Get phyaddr ERROR\n");
        goto exit;
    }

    ret = mt_unf_dma_memcpy((MT_EDMA_CH_E)ch, psrc, pdst, size);
    if (ret != MTAVSF_SUCCESS) {
        MTAVSF_LOG(MODULE_TAG"dma-%d copy from %p to %p with size(0x%x) failed\n", ch, src, dst, (int) size);
        ret = MTAVSF_FAILURE;
        goto exit;
    }

    ret = MTAVSF_SUCCESS;
exit:
    MTAVSF_LOGA(MODULE_TAG"[%s] ch%d: dst = 0x%lx, src = 0x%lx \n", __func__, ch, dst, src);
    mt_unf_dma_release_channel((MT_EDMA_CH_E)ch);
    if (NULL != vsrc_mmz) {
        mt_mmz_unmap(vsrc_mmz);
        mt_mmz_delete(psrc);
    }

    OS_AUDIO_UNLOCK();

    if (ret != MTAVSF_SUCCESS){
        return NULL;
    }
    return dst;
}

static MTAVSF_MEMCP_FUNC get_audio_memcp_func(void)
{
    MTAVSF_LOG(MODULE_TAG"Init audio dma copy!\n");
    return aud_dmacpy_tee;
}
#else
static int aud_is_secure_buffer(void)
{
    return 0;
}
static MTAVSF_MEMCP_FUNC get_audio_memcp_func(void)
{
    return memcpy;
}
#endif /* CONFIG_MT_TEE_SUPPORT END */
#endif /* Internal Function END */

#if MT_DES("External API Definition", 1)
MTAVSF_MEMCP_FUNC avfilter_get_memcp_func(
    unsigned int codec_id, unsigned int encrypted)
{
    /* encrypted stream use old flow */
    if (encrypted ||
        0 == aud_is_secure_buffer() ||
        MTAVSF_MEDIA_TYPE_AUDIO !=
        mtlz_avfilter_get_media_type(codec_id)) {
        return memcpy;
    }
 #ifdef CONFIG_MT_EXT_VVID_SUPPORT	
	if(MTAV_CODEC_AID_AV3A == codec_id)
	{
		return memcpy;
	}
	else
#endif		
	{
    	return get_audio_memcp_func();
	}
}

#endif /* External API Definition END */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
