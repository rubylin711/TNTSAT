/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "mt_module.h"
#include "mt_mpi_mem.h"

#include "mt_drv_adec.h"
#include "mt_mpi_adec.h"

#include "mt_error_mpi.h"
#include "mt_module_debug.h"
#include "adec/adec_api.h"

#define adec_mem_map(addr, size) mt_mem_map(addr, size)

#define AV_CPU_ENABLE_USER  1

static int g_fd_adec = -1;
mt_s32 g_is_ac3or4 = 0;
aud_buf_param_t aud_buf_param;
static aud_share_buf_t *g_aud_share_buf = NULL;
static aud_share_buf_t *g_aud_share_buf_av = NULL;
static aud_share_buf_t *g_aud_share_buf_ta = NULL;
ADEC_ATTR_S g_adec_attr = {0};

static pthread_mutex_t   g_adecMutex = PTHREAD_MUTEX_INITIALIZER;

#define ADEC_GET_USRER_SHARE_ADDR()\
do{\
	(void)pthread_mutex_lock(&g_adecMutex);\
    if(1 == g_is_ac3or4)\
    {\
        g_aud_share_buf = g_aud_share_buf_ta;\
    }\
    else\
    {\
		g_aud_share_buf = g_aud_share_buf_av;\
    }\
    (void)pthread_mutex_unlock(&g_adecMutex); \
    if(NULL == g_aud_share_buf)\
    {\
    	return 0;\
    }\
   }while(0)
   

#if defined(CONFIG_MT_EXT_VVID_SUPPORT) || defined(CONFIG_MT_DOLBY_AC4_SUPPORT)
#define EXT_AUDIO_DECODER_ENABLE
#endif

#ifdef EXT_AUDIO_DECODER_ENABLE
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
//#include "AudioDecoder.h"
#include "ext_decoder/RingBuffer.h"
#include "ext_decoder/ext_decoder.h"
#include "snd/snd_api.h"
#include "mt_drv_dma.h"

#define DMA_COPY
#define RB_SIZE (2*1024*1024)
#define TMS_NUM 4096*2
static guint8 input_buffer[REQUEST_PACKET_SIZE];
static RING_BUFFER_PLUGIN_T *prb_adec = NULL;
static MEDIA_DATA_RING_BUFFER_T mbCtrl;
static _Bool bEof = FALSE;
static pthread_t thid_audio_decoder;
static guint64 current_tms = INVALID_TMS;
static guint32 frm_index = 0;
static pthread_attr_t thread_attr;
static ext_decoder_t *ext_dec = NULL;
static void *pcm_temp_buffer_vir = NULL;
static phys_addr_t pcm_temp_buffer_phy = 0;
static guint32 tmp_buffer_size = 2048*4*8;//max 8ch 2048 samples
static void *fp_tmp_es_buffer = NULL;
static guint32 fp_tmp_es_size = 8*1024;
static guint32 a_type = 0;

static void *ci_request_buffer(size_t *realsize, size_t reqsize);

static int mpi_snd_data_pkt_push(AUD_SND_DATA_PKT_QUE *pkt_que, SND_BUFF *sw_buf, AUD_SND_DATA_PKT *data)
{
	u32 idx = 0;
	u32 len = 0;
	int ret[AUD_SND_SW_BUFF_NUM] = {0};
	SND_BUFF *pBuff = NULL;
	AUD_SND_BUFF_ATTR *pAttr = NULL;
	#ifndef DMA_COPY
	char *base = NULL;
	#endif
	
	if ((NULL == pkt_que) || (NULL == sw_buf) || (NULL == data)){
		printf("<%s:%d> invalid parameter [%p/%p/%p]\n", __func__, __LINE__, pkt_que, sw_buf, data);
		return -1;
	}

#ifdef DMA_COPY
	if(0 == pcm_temp_buffer_phy) {
		pcm_temp_buffer_phy = mt_mmz_new(tmp_buffer_size, 128, MMZ_ZONE_DDR, "pcm_temp");
		pcm_temp_buffer_vir = mt_mmz_map(pcm_temp_buffer_phy, 0);
	}
	
	if(tmp_buffer_size < len) {
		if(pcm_temp_buffer_phy) {
			if(pcm_temp_buffer_vir) {
				mt_mmz_unmap(pcm_temp_buffer_vir);
				pcm_temp_buffer_vir = NULL;
			}
		
			mt_mmz_delete(pcm_temp_buffer_phy);
			pcm_temp_buffer_phy = 0;
		}

		pcm_temp_buffer_phy = mt_mmz_new(len, 128, MMZ_ZONE_DDR, "pcm_temp");
		pcm_temp_buffer_vir = mt_mmz_map(pcm_temp_buffer_phy, 0);
		tmp_buffer_size = len;
	}

	while (DMA_CHN_ID7 != mt_dma_request_chn_id((mt_s32)DMA_CHN_ID7)) {
		usleep(100);
	}

	while (DMA_STATUS_STOP != mt_dma_status(DMA_CHN_ID7)) {
		usleep(100);
	}
#endif

	//pkt info copy
	memcpy((void *)&pkt_que->stQue[pkt_que->wt], (void *)data, sizeof(AUD_SND_DATA_PKT));

	//data copy
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++) {
		if (data->stData[idx].len && (AUD_SND_DATA_FMT_NULL != data->stData[idx].fmt)) {
			pBuff = &sw_buf[idx];
			pAttr = &pkt_que->stQue[pkt_que->wt].stData[idx];
			len = data->stData[idx].len;
			ret[idx] = -1;
		#ifndef DMA_COPY
			base = (char *)pBuff->base_usr_vir;
		#else
			//base = (char *)(ulong)pBuff->base_phy;
			memcpy(pcm_temp_buffer_vir, (void *)(ulong)data->stData[idx].addr, len);
		#endif

			if (pBuff->rd > pBuff->wt) {
				if (pBuff->wt + len < pBuff->rd) {
				#ifndef DMA_COPY
					memcpy(base + pBuff->wt, (void *)data->stData[idx].addr, len);
				#else
					if (0 == mt_dma_memcpy_ext(DMA_CHN_ID7, (phys_addr_t)pBuff->base_phy + pBuff->wt, pcm_temp_buffer_phy, len, 0)) {
						MT_ERR_ADEC("dma copy error!\n");
					}
				#endif
					pAttr->addr = pBuff->wt;
					pBuff->wt = (pBuff->wt + len) % pBuff->size;
					ret[idx] = 0;
				}
			} else {
				if (pBuff->wt + len < pBuff->size) {
				#ifndef DMA_COPY
					memcpy(base + pBuff->wt, (void *)data->stData[idx].addr, len);
				#else
					if (0 == mt_dma_memcpy_ext(DMA_CHN_ID7, (phys_addr_t)pBuff->base_phy + pBuff->wt, pcm_temp_buffer_phy, len, 0)) {
						MT_ERR_ADEC("dma copy error!\n");
					}
				#endif
					pAttr->addr = pBuff->wt;
					pBuff->wt = (pBuff->wt + len) % pBuff->size;
					ret[idx] = 0;
				} else if (len < pBuff->rd) {
				#ifndef DMA_COPY
					memcpy(base, (void *)data->stData[idx].addr, len);
				#else
					if (0 == mt_dma_memcpy_ext(DMA_CHN_ID7, (phys_addr_t)pBuff->base_phy, pcm_temp_buffer_phy, len, 0)) {
						MT_ERR_ADEC("dma copy error!\n");
					}
				#endif
					pAttr->addr = 0;
					pBuff->wt = len % pBuff->size;
					ret[idx] = 0;
				}
			}

			if (0 == ret[idx]) {
				pAttr->len = len;
				pBuff->wtCnt++;
				pBuff->wtByte += len;
			}
		}
	}

#ifdef DMA_COPY
	while (MT_SUCCESS != mt_dma_release_chn_id(DMA_CHN_ID7)) {
		usleep(100);
	}
#endif

	//pkt queue wt update
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++) {
		if (0 != ret[idx]) {
			break;
		}
	}
	
	if (AUD_SND_SW_BUFF_NUM <= idx) {
		pkt_que->wt_cnt++;
		pkt_que->wt = (pkt_que->wt + 1) % pkt_que->num;
	} else {
		printf("sw buff push FAIL, rd/wt/num[%d/%d/%d]\n", pkt_que->rd, pkt_que->wt, pkt_que->num);
	}

	return 0;
}

static mt_s32 mpi_snd_sw_buff_check(SND_BUFF *pBuff, mt_u32 len)
{
	if (pBuff->rd > pBuff->wt)
	{
		if (pBuff->wt + len < pBuff->rd)
		{
			return MT_SUCCESS;
		}
	}
	else
	{
		if ((pBuff->wt + len < pBuff->size) || (len < pBuff->rd))
		{
			return MT_SUCCESS;
		}
	}

	return MT_FAILURE;
}

static int mpi_send_data_to_snd(AUD_SND_DATA_PKT_QUE *pkt_que, SND_BUFF *sw_buf, AUD_SND_DATA_PKT *data)
{
	int idx = 0;
	int ret;
	u32 pkt_que_free = 0;

	if ((NULL == pkt_que) || (NULL == sw_buf) || (NULL == data)) {
		printf("<%s:%d> invalid parameter [%p/%p/%p]\n", __func__, __LINE__, pkt_que, sw_buf, data);
		return -1;
	}

	//data package queue node check
	if (pkt_que->rd > pkt_que->wt) {
		pkt_que_free = pkt_que->rd - pkt_que->wt;
	} else {
		pkt_que_free = pkt_que->num - pkt_que->wt + pkt_que->rd;
	}
	
	if (!pkt_que_free) {
		printf("<%s:%d> snd data package queue no space [%d/%d/%d]\n", __func__, __LINE__, pkt_que->rd, pkt_que->wt, pkt_que->num);
		return -1;
	}

	//data package sw buffer space check
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++) {
		if (data->stData[idx].len && (AUD_SND_DATA_FMT_NULL != data->stData[idx].fmt)) {
			if (0 != mpi_snd_sw_buff_check(&sw_buf[idx], data->stData[idx].len)) {
				//printf("snd data package sw buffer[%d] no space, pkt_que.wt_cnt[%d]\n", idx, pkt_que->wt_cnt);
				return -1;
			}
		}
	}

	//data copy and pkt queue update
	ret = mpi_snd_data_pkt_push(pkt_que, sw_buf, data);

	return ret;
}

static int output_pcm(void *pcm, int count)
{
	int retry_cnt = 0, i;
	AUDIO_INFO_T audio_info;
	AUD_SND_DATA_PKT pkt = {0};
	adec_info_t *adec_info;

	if (count <= 0) 
		return -1;
	
	ADEC_GET_USRER_SHARE_ADDR();
	frm_index++;

	ext_dec->getinfo(&audio_info);
	pkt.frmIdx = frm_index;
	pkt.apts = (u32)((current_tms/1000)*45);
	pkt.sampleRate = audio_info.sample_rate;
	pkt.frameSample = count;
	pkt.channel = audio_info.channels;
	pkt.bitDepth = audio_info.bitdepth;
	pkt.interleaved = 0;
	pkt.validBit = 16;
	pkt.typeOriginal = 0;
	pkt.channelOriginal = audio_info.channelsOriginal;
	pkt.dualmono = 0;
	pkt.bsid = 0;
	pkt.acmod = 0;
	pkt.lfeon = 0;
	pkt.dolby_type = 0;//DOLBY_NONE;

	for (i = 0; i < 3; i++) {
		pkt.stData[i].fmt = AUD_SND_DATA_FMT_NULL;
		pkt.stData[i].addr = 0;
		pkt.stData[i].len = 0;
	}

	pkt.stData[0].fmt = AUD_SND_DATA_FMT_PCM;
	pkt.stData[0].addr = (u64)(ulong)pcm;
	pkt.stData[0].len = count * pkt.channel * (pkt.bitDepth>>3);

	//printf("output ch:%d rate:%d samples:%d\n",audio_info.channels, audio_info.sample_rate, count);
	while(mpi_send_data_to_snd(&g_aud_share_buf->snd_pkt, g_aud_share_buf->snd_sw_buf, &pkt)) {
		if(retry_cnt > 100) {
			MT_ERR_ADEC("mpi send data to sound error!\n");
			break;
		}
		usleep(5*1000);
		retry_cnt++;
	}

	adec_info = &g_aud_share_buf->adec_info;
	adec_info->parse_ok = frm_index;
	adec_info->decode_ok = frm_index;
	adec_info->frm_idx = frm_index;
	adec_info->sample_rate = pkt.sampleRate;
	adec_info->ch_num = pkt.channelOriginal;
	adec_info->sample_num = pkt.frameSample;
	adec_info->decode_state = ADEC_OUTPUT;
	
	return 0;
}

/*
 * Request a buffer containing part of the input audio data.
 */
static void *ci_request_buffer(size_t *realsize, size_t reqsize)
{
    int ret = RB_FAILURE;

    while(RB_FAILURE == ret) {
        ret = prb_adec->read(&mbCtrl, input_buffer, (u32)reqsize, 1, (u32*)realsize, &current_tms);
        if (RB_FAILURE == ret) {
            if (bEof) { //discard the remaining bytes
                *realsize = 0;
                break;
            } else {
                usleep(20*1000);
            }
        }
    }
	
    return ((void *)input_buffer);
}

static void ci_update_buffer(size_t size)
{
    prb_adec->validate(&mbCtrl, RB_FLAG_RD, size);
}

static void *audio_decoder_thread(void *arg)
{//request_buffer_fn_type and output_pcm_fn_type will be called in the same thread successively
	adec_info_t *adec_info;

	ADEC_GET_USRER_SHARE_ADDR();

	adec_info = &g_aud_share_buf->adec_info;

	if(NULL == ext_dec)
		pthread_exit(NULL);
	
	frm_index = 0;
	adec_info->status = ADEC_STARTED;
	adec_info->decode_state = ADEC_DECODE;

    ext_dec->initialize(ci_request_buffer, ci_update_buffer, output_pcm);
    if (ext_dec->run() != 0) {//loop until break or request bytes
        MT_ERR_ADEC("error: codec error\n");
    }
    ext_dec->finalize();

	adec_info->status = ADEC_STOPED;
	adec_info->decode_state = ADEC_READ;

    pthread_exit(NULL);
}
#endif

mt_s32 MT_MPI_ADEC_RegisterDeoder(const mt_char *pszCodecDllName)
{
	return MT_SUCCESS;
}

mt_s32 MT_MPI_ADEC_FoundSupportDeoder(HA_FORMAT_E enFormat, mt_u32 *penDstCodecID)
{
	return 0;
}


mt_s32 MT_MPI_ADEC_Init(void)
{
	mt_s32 ret = 0;

	MT_ALWAYS_PRINT("<%s:%d>\n", __func__, __LINE__);

	return ret;
}

mt_s32 MT_MPI_ADEC_deInit(void)
{
	return 0;
}

/*****************************************************************************
 Prototype    : MT_API_AO_Open
 Description  : open adec
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2006/06/13
    Author       : vicent feng
    Modification : Created function

*****************************************************************************/
mt_s32 MT_MPI_ADEC_Open(mt_handle *phAdec)
{
	int ret, es_buf_size, snd_buf_size;
	mt_char path_name[64];

	snprintf((mt_char*)path_name, sizeof(path_name), "/dev/%s", ADEC_DEVICE_NAME);

	g_fd_adec = open(path_name, O_RDWR | O_CLOEXEC);
	if (g_fd_adec < 0)
	{
		MT_ERR_ADEC("<%s:%d> open adec error!\n", __func__, __LINE__);
		return MT_FAILURE;
	}
	else
	{
		*phAdec = g_fd_adec;
		MT_INFO_ADEC("<%s:%d> open /dev/adec success!\n", __func__, __LINE__);
	}

	ret = ioctl((int)*phAdec, IO_ADEC_GET_AUD_MEM, &aud_buf_param);
	if (0 != ret)
	{
		MT_ERR_ADEC("ioctl IO_ADEC_ALLOC_SHARE_MEM error!\n");
		return -1;
	}

	es_buf_size = aud_buf_param.aud_buf_size_map.es_buf_size + aud_buf_param.aud_buf_size_map.ad_es_buf_size;
	snd_buf_size = aud_buf_param.aud_buf_size_map.snd_sw_buf0_size +
		aud_buf_param.aud_buf_size_map.snd_sw_buf1_size + aud_buf_param.aud_buf_size_map.snd_sw_buf2_size;

	if (NULL == g_aud_share_buf)
	{
		(void)pthread_mutex_lock(&g_adecMutex);
		g_aud_share_buf_ta = (aud_share_buf_t *)mt_mem_map_cache((phys_addr_t)aud_buf_param.share_buf_phy, sizeof(aud_share_buf_t));

#ifdef AV_CPU_ENABLE_USER		
		g_aud_share_buf_av = (aud_share_buf_t *)mt_mem_map((phys_addr_t)aud_buf_param.share_buf_phy_av, sizeof(aud_share_buf_t));
#else
		g_aud_share_buf_av = (aud_share_buf_t *)mt_mem_map((phys_addr_t)aud_buf_param.share_buf_phy, sizeof(aud_share_buf_t));
#endif
		g_aud_share_buf = g_aud_share_buf_ta ;
		
		g_aud_share_buf->es_buf.data_usr_vir = (u64)((ulong)adec_mem_map((phys_addr_t)aud_buf_param.es_buf_phy, es_buf_size));
		g_aud_share_buf->ad_es_buf.data_usr_vir = g_aud_share_buf->es_buf.data_usr_vir + aud_buf_param.aud_buf_size_map.es_buf_size;
		g_aud_share_buf->snd_sw_buf[0].base_usr_vir = (u64)((ulong)adec_mem_map((phys_addr_t)aud_buf_param.snd_buf_phy, snd_buf_size));
		g_aud_share_buf->snd_sw_buf[1].base_usr_vir = g_aud_share_buf->snd_sw_buf[0].base_usr_vir + aud_buf_param.aud_buf_size_map.snd_sw_buf0_size;
		g_aud_share_buf->snd_sw_buf[2].base_usr_vir = g_aud_share_buf->snd_sw_buf[0].base_usr_vir + aud_buf_param.aud_buf_size_map.snd_sw_buf1_size;

		// for av_cpu
		g_aud_share_buf_av->es_buf.data_usr_vir = g_aud_share_buf_ta->es_buf.data_usr_vir;
		g_aud_share_buf_av->ad_es_buf.data_usr_vir =  g_aud_share_buf_ta->ad_es_buf.data_usr_vir;
		g_aud_share_buf_av->snd_sw_buf[0].base_usr_vir =  g_aud_share_buf_ta->snd_sw_buf[0].base_usr_vir;
		g_aud_share_buf_av->snd_sw_buf[1].base_usr_vir = g_aud_share_buf_ta->snd_sw_buf[1].base_usr_vir;
		g_aud_share_buf_av->snd_sw_buf[2].base_usr_vir = g_aud_share_buf_ta->snd_sw_buf[2].base_usr_vir;
		(void)pthread_mutex_unlock(&g_adecMutex);
	}

	return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : MT_MPI_ADEC_Close
 Description  : close adec
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2006/06/13
    Author       : vicent feng
    Modification : Created function

*****************************************************************************/
mt_s32 MT_MPI_ADEC_Close (mt_handle hAdec)
{
	close(hAdec);
	MT_ALWAYS_PRINT("<%s:%d> close /dev/adec!\n", __func__, __LINE__);
	g_fd_adec = -1;
#ifdef EXT_AUDIO_DECODER_ENABLE
	if(pcm_temp_buffer_phy) {
		if(pcm_temp_buffer_vir) {
			mt_mmz_unmap(pcm_temp_buffer_vir);
			pcm_temp_buffer_vir = NULL;
		}

		mt_mmz_delete(pcm_temp_buffer_phy);
		pcm_temp_buffer_phy = 0;
	}

	if(fp_tmp_es_buffer)
		free(fp_tmp_es_buffer);
	fp_tmp_es_buffer = NULL;
#endif

	(void)pthread_mutex_lock(&g_adecMutex);
	if(g_aud_share_buf_ta != NULL)
	{
		mt_mem_unmap((void*)(ulong)g_aud_share_buf_ta->es_buf.data_usr_vir);
		mt_mem_unmap((void*)(ulong)g_aud_share_buf_ta->snd_sw_buf[0].base_usr_vir);
	}
	mt_mem_unmap((void*)(ulong)g_aud_share_buf_ta);
	mt_mem_unmap((void*)(ulong)g_aud_share_buf_av);
	g_aud_share_buf_ta = NULL;
	g_aud_share_buf_av = NULL;
	g_aud_share_buf = NULL ;
	(void)pthread_mutex_unlock(&g_adecMutex);
	return 0;
}

mt_s32 MT_MPI_ADEC_Start(mt_handle hAdec, mt_u32 aud_type)
{
	mt_s32 ret = 0;
	int i = 0;
#ifdef EXT_AUDIO_DECODER_ENABLE
	struct sched_param	 SchedParam;
	a_type = aud_type;
	adec_info_t *adec_info;
#endif

	if((aud_type ==  HA_AUDIO_ID_TRUEHD) ||
		(aud_type ==  HA_AUDIO_ID_AC3PASSTHROUGH) ||
		(aud_type ==  HA_AUDIO_ID_EAC3PASSTHROUGH) ||
		(aud_type ==  HA_AUDIO_ID_DOLBY_PLUS) ||
		(aud_type ==  HA_AUDIO_ID_DOLBY_TRUEHD) ||
		(aud_type ==  HA_AUDIO_ID_DOLBY_CONVERT) ||
		(aud_type ==  HA_AUDIO_ID_VVID) ||
		(aud_type ==  HA_AUDIO_ID_DOLBY_AC4) )
	{
		g_is_ac3or4 = 1;
	}
	else
	{
		g_is_ac3or4 = 0;
	}

#ifndef AV_CPU_ENABLE_USER
	g_is_ac3or4 = 1;  // allways on TEE
#endif

#ifndef CONFIG_MT_TEE_SUPPORT
	g_is_ac3or4 = 0;  // allways on av_cpu
#endif

#ifdef EXT_AUDIO_DECODER_ENABLE
	if(a_type == HA_AUDIO_ID_VVID)
		g_is_ac3or4 = 0;  // allways on av_cpu
#endif

	for(i=0; i<30; i++)
	{
		if (0 != ioctl(hAdec, IO_ADEC_GET_AUD_MEM, &aud_buf_param))
		{
			MT_ERR_ADEC("IO_ADEC_GET_AUD_MEM error!");
			return -1;
		}
		if(2 == aud_buf_param.ta_enabled)
		{	
			g_is_ac3or4 = 0;  // no ta exists
			break;
		}
		else if(1 == aud_buf_param.ta_enabled)
		{	
			break;
		}
		else if(0  == aud_buf_param.ta_enabled)
		{
			usleep(1000*100);
		}
	}

	MT_ALWAYS_PRINT("<%s:%d> type:%x  g_is_ac3or4 %d  aud_buf_param.ta_enabled %d i=%d \n",
		__func__, __LINE__, aud_type, g_is_ac3or4, aud_buf_param.ta_enabled, i);

#ifdef EXT_AUDIO_DECODER_ENABLE
	ADEC_GET_USRER_SHARE_ADDR();
	adec_info = &g_aud_share_buf->adec_info;
	adec_info->aud_type = a_type;
	
	if(a_type == HA_AUDIO_ID_VVID) {
		snprintf(adec_info->decoder_name, sizeof(adec_info->decoder_name), "vivid");
		bEof = FALSE;
		prb_adec = get_rb_plugin();
		prb_adec->initialize(&mbCtrl, RB_SIZE, REQUEST_PACKET_SIZE, TMS_NUM);

		ext_dec = attach_ext_decoder(aud_type);
		if(NULL != ext_dec)
			adec_info->is_supported = 1;
		else
			adec_info->is_supported = 0;
		
		pthread_attr_init(&thread_attr);
		(mt_void)pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
		(mt_void)pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
		(mt_void)pthread_attr_getschedparam(&thread_attr, &SchedParam);
		SchedParam.sched_priority = CONFIG_MT_HIGH_PERF_AVPLAY_PRIORITYSET_PAYLOAD;
		(mt_void)pthread_attr_setschedparam(&thread_attr, &SchedParam);
		ret = pthread_create(&thid_audio_decoder, &thread_attr, audio_decoder_thread, (void *)(&mbCtrl));
		if (MT_SUCCESS != ret){
			pthread_attr_destroy(&thread_attr);
			return MT_FAILURE;
		}	
	}
#endif

	ret = ioctl(hAdec, IO_ADEC_START, (ulong)(&g_adec_attr.sOpenPram));
	if (ret != MT_SUCCESS)
	{
		MT_ERR_ADEC("IO_ADEC_START error.\n");
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Stop(mt_handle hAdec, mt_u32 reset)
{
    mt_s32 ret = 0;
	
#ifdef EXT_AUDIO_DECODER_ENABLE
	void *status;

	if(a_type == HA_AUDIO_ID_VVID) {
		bEof = TRUE;
		if(ext_dec)
			ext_dec->stop();

		pthread_join(thid_audio_decoder, &status);//exit the decoder firstly
		pthread_attr_destroy(&thread_attr);
		prb_adec->reset(&mbCtrl); //ensure the sendStream not fall into the deadloop
		prb_adec->finalize(&mbCtrl);
		ext_dec = NULL;
	}
#endif

	ret = ioctl(hAdec, IO_ADEC_STOP, reset);
    return ret;
}

mt_s32 MT_MPI_ADEC_Flush_Buf(mt_handle hAdec)
{
	return ioctl(hAdec, IO_ADEC_FLUSH);
}

mt_s32 MT_MPI_ADEC_Reset_In_Buf(mt_handle hAdec)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_Reset_Out_Buf(mt_handle hAdec)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_SetDolbyDownmixMode(mt_handle hAdec, const mt_u32 *pDolbyDownmixMode)
{
	mt_u32 mode = 0;
	
	if ((NULL == g_aud_share_buf) || (NULL == pDolbyDownmixMode))
	{
		return -1;
	}
	
	ADEC_GET_USRER_SHARE_ADDR();

	mode = *pDolbyDownmixMode;
	if (mode > 1)
	{
		mode = 1;
	}
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->dolby_downmix_mode = (u32)mode;
		g_aud_share_buf_ta->dolby_downmix_mode = (u32)mode;
	}
	
	return 0;
}

mt_s32 MT_MPI_ADEC_SetDDPTestMode(mt_handle hAdec, MT_BOOL bEnable)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_SetAllAttr(mt_handle hAdec, ADEC_ATTR_S *pstAllAttr)
{
	static void *priv_data;
	static u32 priv_size = 0;
	
	if (NULL == pstAllAttr)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)pstAllAttr);
		return -1;
	}

	if ((priv_size < pstAllAttr->sOpenPram.u32CodecPrivateDataSize) && (NULL != priv_data))
	{
		free(priv_data);
		priv_data = NULL;
	}

	if (NULL == priv_data)
	{
		priv_data = malloc(pstAllAttr->sOpenPram.u32CodecPrivateDataSize);
		priv_size = pstAllAttr->sOpenPram.u32CodecPrivateDataSize;
		if (NULL == priv_data)
		{
			MT_ERR_ADEC("malloc priv_data error!\n");
			return -1;
		}
	}

	memcpy(priv_data, pstAllAttr->sOpenPram.pCodecPrivateData, pstAllAttr->sOpenPram.u32CodecPrivateDataSize);
	memcpy(&g_adec_attr, pstAllAttr, sizeof(ADEC_ATTR_S));
	g_adec_attr.sOpenPram.pCodecPrivateData = priv_data;
	g_adec_attr.sOpenPram.u32CodecId = pstAllAttr->u32CodecID;

	return 0;
}

mt_s32 MT_MPI_ADEC_GetAllAttr(mt_handle hAdec, ADEC_ATTR_S * pstAllAttr)
{
	if (NULL == pstAllAttr)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)pstAllAttr);
		return -1;
	}

	memcpy(pstAllAttr, &g_adec_attr, sizeof(ADEC_ATTR_S));

	return 0;
}

mt_s32 MT_MPI_ADEC_GetDelayMs(mt_handle hAdec, mt_u64 *pDelay)
{
	return 0;
}

void MT_MPI_ADEC_Set_Pause (mt_handle hAdec, int st)
{
	if (st)
	{
		ioctl(hAdec, IO_ADEC_PAUSE);
	}
	else
	{
		ioctl(hAdec, IO_ADEC_RESUME);
	}

	return ;
}

/*****************************************************************************
 Prototype    : MT_MPI_ADEC_SendStream
 Description  : send audio frame to MTAO
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2006/06/13
    Author       : vicent feng
    Modification : Created function

*****************************************************************************/
mt_s32 MT_MPI_ADEC_SendStream (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs)
{
    mt_s32 retval= MT_FAILURE;
	
#ifdef EXT_AUDIO_DECODER_ENABLE
		if(a_type == HA_AUDIO_ID_VVID) {
			if (pstStream->u32Size > 0) {
				int ret = RB_FAILURE;
				for(;;) {
					ret = prb_adec->write(&mbCtrl, pstStream->pu8Data, pstStream->u32Size, 1, u64PtsMs);
					if (RB_FAILURE == ret) {
						printf("Ring Buffer Full : request %d\n", pstStream->u32Size);
						usleep(20*1000);
					} else {
						prb_adec->validate(&mbCtrl, RB_FLAG_WR, pstStream->u32Size);
						break;
					}
				}
			}	
		}
		retval= MT_SUCCESS;
#else
		retval= MT_SUCCESS;
#endif

	
	return retval;
}

static int check_es_pkt(adec_es_buf_t *es_buf)
{
	u32 remain, rd, wt;

	if (NULL == es_buf)
	{
		MT_ERR_ADEC("invalid parameter! [0x%#lx]\n", (ulong)es_buf);
		return -1;
	}

	rd = es_buf->pkt_rd;
	wt = es_buf->pkt_wt;
	if (wt >= rd)
	{
		remain = es_buf->pkt_cnt - (wt - rd);
	}
	else
	{
		remain = rd - wt;
	}

	if (remain <= 1)
	{
		return -1;
	}

	return 0;
}

static int check_es_buf(adec_es_buf_t *es_buf, int size)
{
	uint32_t remain, rd, wt;
	adec_es_pkt_t *es_pkt;

	if ((NULL == es_buf) || (size <=0))
	{
		MT_ERR_ADEC("invalid parameter! [0x%#lx/%d]\n", (ulong)es_buf, size);
		return -1;
	}

	rd = es_buf->es_rd;
	wt = es_buf->es_wt;
	es_pkt = &es_buf->es_pkt[es_buf->pkt_wt];
	es_pkt->flag = 0;

	// clca actual space
	if (wt >= rd)
	{
		remain = es_buf->es_buf_size - (wt - rd);
	}
	else
	{
		remain = rd - wt;
	}

	// no space
	if (remain <= size)
	{
		return -1;
	}

	if (wt >= rd)
	{
		remain = es_buf->es_buf_size - wt;
		if (remain >= size)
		{
			return 0;
		}
		else
		{
			remain = rd;
			if (remain > size)
			{
				es_buf->es_wt = 0;
				es_pkt->flag = 1;
				return 0;
			}
		}
	}
	else
	{
		remain = rd - wt;
		if (remain > size)
		{
			return 0;
		}
	}

	return -1;
}

static int get_es_buf_remain_space(adec_es_buf_t *es_buf)
{
	uint32_t remain, rd, wt;

	if (NULL == es_buf)
	{
		MT_ERR_ADEC("invalid parameter! [0x%#lx]\n", (ulong)es_buf);
		return -1;
	}

	rd = es_buf->es_rd;
	wt = es_buf->es_wt;

	// clca actual space
	if (wt >= rd)
	{
		remain = es_buf->es_buf_size - (wt - rd);
	}
	else
	{
		remain = rd - wt;
	}

	// no space
	if (remain <= 1)
	{
		return -1;
	}

	// clca linear space
	if (wt >= rd)
	{
		remain = es_buf->es_buf_size - wt;
	}
	else
	{
		remain = rd - wt;
	}

	return remain;
}

static mt_s32 Get_ADEC_Buffer(mt_handle hAdec, mt_u32 u32RequestSize, MT_UNF_STREAM_BUF_S *pstStream)
{
	int ret;
	adec_es_buf_t *es_buf;
	u8 *data_vir;
	phys_addr_t data_phy;

	if ((NULL == g_aud_share_buf) || (NULL == pstStream))
	{
		MT_ERR_ADEC("invalid parameter [%#lx/%#lx]\n", (ulong)g_aud_share_buf, (ulong)pstStream);
		return MT_ERR_ADEC_INVALID_PARA;
	}

	ADEC_GET_USRER_SHARE_ADDR();
	es_buf = &g_aud_share_buf->es_buf;

	ret = check_es_pkt(es_buf);
	if (0 != ret)
	{
		return MT_ERR_ADEC_IN_BUF_FULL;
	}

	ret = check_es_buf(es_buf, u32RequestSize);
	if (0 != ret)
	{
		return MT_ERR_ADEC_IN_BUF_FULL;
	}

	memset(pstStream, 0, sizeof(MT_UNF_STREAM_BUF_S));
	data_vir = (u8 *)((ulong)es_buf->data_usr_vir);
	data_phy = (phys_addr_t)es_buf->data_phy;
	pstStream->pu8Data = data_vir + es_buf->es_wt;
	pstStream->u32PhyData = data_phy + es_buf->es_wt;
	pstStream->u32Size = u32RequestSize;

	return 0;

}
mt_s32 MT_MPI_ADEC_GetBuffer(mt_handle hAdec, mt_u32 u32RequestSize, MT_UNF_STREAM_BUF_S *pstStream)
{
#ifdef EXT_AUDIO_DECODER_ENABLE
	mt_u32 real_len = 0;
	if(a_type == HA_AUDIO_ID_VVID) {
		if (NULL == pstStream) {
			MT_ERR_ADEC("invalid parameter [%#lx]\n", (ulong)pstStream);
			return MT_ERR_ADEC_INVALID_PARA;
		}

		if(NULL == fp_tmp_es_buffer) {
			fp_tmp_es_buffer = malloc(fp_tmp_es_size);
			if(NULL == fp_tmp_es_buffer) {
				printf("malloc fp tmp es buffer failed!\n");
				return MT_ERR_ADEC_IN_BUFFER_SIZE_EXCEEDED;
			}
		}

		if(u32RequestSize > fp_tmp_es_size) {
			printf("req buffer size too large, support max size:0x%x!\n",fp_tmp_es_size);
			return MT_ERR_ADEC_IN_BUFFER_SIZE_EXCEEDED;
		}
		
		memset(pstStream, 0, sizeof(MT_UNF_STREAM_BUF_S));
		if(FALSE == prb_adec->is_enough(&mbCtrl, u32RequestSize, RB_FLAG_WR, &real_len, NULL)) {
			return MT_ERR_ADEC_IN_BUF_FULL;
		}

		pstStream->pu8Data = fp_tmp_es_buffer;
		pstStream->u32PhyData = 0;
		pstStream->u32Size = u32RequestSize;

		return 0;
	}else {
	
		return Get_ADEC_Buffer(hAdec, u32RequestSize, pstStream);
	}
#else

	return Get_ADEC_Buffer(hAdec, u32RequestSize, pstStream);

#endif
}

static mt_s32 Put_ADEC_Buffer (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs, mt_u32 PtsValide, MT_BOOL b_eos)
{
	adec_es_buf_t *es_buf;
	u32 pts, remain, es_wt;
	adec_es_pkt_t *es_pkt;

	if (NULL == pstStream)
	{
		MT_ERR_ADEC("invalid parameter [%#lx]\n", (ulong)pstStream);
		return MT_ERR_ADEC_INVALID_PARA;
	}
	
	ADEC_GET_USRER_SHARE_ADDR();
	es_buf = &g_aud_share_buf->es_buf;
	pts = u64PtsMs / 1000 * 45;

	remain = get_es_buf_remain_space(es_buf);
	if (pstStream->u32Size > remain)
	{
		MT_ERR_ADEC("es main been write overflow [%s:%d]\n", remain, pstStream->u32Size);
		return MT_ERR_ADEC_NULL_PTR;
	}

#ifdef CONFIG_MT_TEE_SUPPORT
	mt_mem_invalidate((void *)(ulong)es_buf->data_usr_vir, es_buf->es_buf_size);
#endif

	es_wt = es_buf->es_wt;
	es_buf->es_wt += pstStream->u32Size;
	es_buf->es_wt %= es_buf->es_buf_size;

	es_pkt = &es_buf->es_pkt[es_buf->pkt_wt];
	es_pkt->pts = pts;
	es_pkt->valid = PtsValide;
	es_pkt->es_rd = es_wt;
	es_pkt->size = pstStream->u32Size;
	es_buf->pkt_wt++;
	es_buf->pkt_wt %= es_buf->pkt_cnt;
	es_pkt->es_ts = 0; //  0 ES,  1 TS

#ifdef CONFIG_MT_TEE_SUPPORT
	mt_mem_flush(es_buf, sizeof(adec_es_buf_t));
#endif

	return 0;

}

mt_s32 MT_MPI_ADEC_PutBuffer (mt_handle hAdec, const MT_UNF_STREAM_BUF_S *pstStream, mt_u64 u64PtsMs, mt_u32 PtsValide, MT_BOOL b_eos)
{		
#ifdef EXT_AUDIO_DECODER_ENABLE
	if(a_type == HA_AUDIO_ID_VVID) {

		if (pstStream->u32Size > 0) {
			int ret = RB_FAILURE;
			for(;;) {
				ret = prb_adec->write(&mbCtrl, pstStream->pu8Data, pstStream->u32Size, 1, u64PtsMs);
				if (RB_FAILURE == ret) {
					//printf("Ring Buffer Full : request %d\n", pstStream->u32Size);
					usleep(20*1000);
				} else {
					prb_adec->validate(&mbCtrl, RB_FLAG_WR, pstStream->u32Size);
					break;
				}
			}
		}

		return 0;
	
	} else {
	
		return Put_ADEC_Buffer(hAdec, pstStream, u64PtsMs, PtsValide, b_eos);
	}
#else

	return Put_ADEC_Buffer(hAdec, pstStream, u64PtsMs, PtsValide, b_eos);

#endif
}
  
mt_s32 MT_MPI_ADEC_ReceiveFrame (mt_handle hAdec, MT_UNF_AO_FRAMEINFO_S *pstAOFrame, ADEC_EXTFRAMEINFO_S *pstExtInfo)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_ReleaseFrame(mt_handle hAdec, const MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
	return 0;
}

static int adec_get_status_info(ADEC_STATUSINFO_S *status_info)
{
	adec_info_t *adec_info;
	adec_es_buf_t *es_buf;

	if (NULL == status_info)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)status_info);
		return -1;
	}
	ADEC_GET_USRER_SHARE_ADDR();
	adec_info = &g_aud_share_buf->adec_info;
	es_buf = &g_aud_share_buf->es_buf;

	status_info->u32BufferSize = es_buf->es_buf_size;
	if (es_buf->es_wt >= es_buf->es_rd)
	{
		status_info->u32BufferUsed = es_buf->es_wt - es_buf->es_rd;
	}
	else
	{
		status_info->u32BufferUsed = es_buf->es_buf_size - es_buf->es_rd + es_buf->es_wt;
	}
	status_info->u32BufferAvailable = es_buf->es_buf_size - status_info->u32BufferUsed;

	status_info->u32TotDecodeFrame = adec_info->decode_ok;
	if (0 != adec_info->sample_rate)
	{
		status_info->u32FrameDurationMs = 1000 * adec_info->sample_num / adec_info->sample_rate;
	}

	status_info->u32CodecID = adec_info->aud_type;
	status_info->u32Channels = adec_info->ch_num;
	status_info->enSampleRate = adec_info->sample_rate;
	status_info->enBitDepth = 16;	// ahren todo

	return 0;
}

static int adec_get_str_info(ADEC_STREAMINFO_S *str_info)
{
	adec_info_t *adec_info;

	if (NULL == str_info)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)str_info);
		return -1;
	}
	ADEC_GET_USRER_SHARE_ADDR();
	adec_info = &g_aud_share_buf->adec_info;

	str_info->u32CodecID = adec_info->aud_type;
	str_info->enSampleRate = adec_info->sample_rate;

	return 0;
}

static int adec_get_buf_status(ADEC_BUFSTATUS_S *buf_status)
{
	int ret = 0;
	adec_es_buf_t *es_buf;

	if (NULL == buf_status)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)buf_status);
		return -1;
	}
	
	ADEC_GET_USRER_SHARE_ADDR();
	es_buf = &g_aud_share_buf->es_buf;

	buf_status->u32BufferSize = es_buf->es_buf_size;

	ret = check_es_pkt(es_buf);
	if (0 != ret)
	{
		buf_status->u32BufferUsed = buf_status->u32BufferSize;
	} else {
		if (es_buf->es_wt >= es_buf->es_rd)
		{
			buf_status->u32BufferUsed = es_buf->es_wt - es_buf->es_rd;
		}
		else
		{
			buf_status->u32BufferUsed = es_buf->es_buf_size - es_buf->es_rd + es_buf->es_wt;
		}
	}
	
	buf_status->u32BufWritePos = es_buf->es_wt;
	buf_status->s32BufReadPos = es_buf->es_rd;
	buf_status->bEndOfFrame = 0;
	buf_status->u32TotDecodeFrame = g_aud_share_buf->adec_info.frm_idx;

	return 0;
}

static int adec_get_dbg_info(ADEC_DEBUGINFO_S *dbg_info)
{
	adec_info_t *adec_info;

	if (NULL == dbg_info)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)dbg_info);
		return -1;
	}
	ADEC_GET_USRER_SHARE_ADDR();

	adec_info = &g_aud_share_buf->adec_info;

	dbg_info->u32DecFrameNum = adec_info->decode_ok;
	dbg_info->u32ErrDecFrameNum = adec_info->decode_err;

	return 0;
}

static int adec_get_decoder_name(ADEC_SzNameINFO_S *info)
{
	adec_info_t *adec_info;

	if (NULL == info)
	{
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)info);
		return -1;
	}
	ADEC_GET_USRER_SHARE_ADDR();
	adec_info = &g_aud_share_buf->adec_info;
	memset(info->szHaCodecName, 0, sizeof(info->szHaCodecName));
	strncpy(info->szHaCodecName, adec_info->decoder_name, sizeof(info->szHaCodecName) - 1);

	return 0;
}

static int adec_get_meta_info(MT_UNF_AVPLAY_METARINFO_S *info)
{
#ifdef EXT_AUDIO_DECODER_ENABLE
	u16 i;
	AUDIO_META_INFO_T meta_info;
	
	if (NULL == info) {
		MT_ERR_ADEC("Invalid parameter %#lx\n", (ulong)info);
		return -1;
	}

	memset(info, 0, sizeof(MT_UNF_AVPLAY_METARINFO_S));

	if(a_type != HA_AUDIO_ID_VVID) 
		return -1;

	if(ext_dec)
		ext_dec->getmetainfo(&meta_info);

	info->obj_num = meta_info.obj_num;
	for(i=0; i< info->obj_num; i++) {
		info->obj_info[i].obj_id = meta_info.obj_info[i].obj_id;
		memcpy(info->obj_info[i].obj_name, meta_info.obj_info[i].obj_name, sizeof(info->obj_info[i].obj_name));
		info->obj_info[i].interact = meta_info.obj_info[i].interact;
	}
	memcpy(info->complementary_object_num, meta_info.complementary_object, sizeof(info->complementary_object_num));
	memcpy(info->complementary_object_id, meta_info.complementary_object_id, sizeof(info->complementary_object_id));

	return 0;
#else
	return -1;
#endif
}

mt_s32 MT_MPI_ADEC_GetInfo(mt_handle hAdec, MT_MPI_ADEC_INFO_E enAdecInfo, void *pstAdecInfo)
{
	mt_s32 retval;

	switch (enAdecInfo)
	{
	case MT_MPI_ADEC_STATUSINFO:
		MT_INFO_ADEC("MT_MPI_ADEC_GetAttrInfo CMD: ADEC_STATUSINFO");
		retval = adec_get_status_info((ADEC_STATUSINFO_S *)pstAdecInfo);
		break;
		
	case MT_MPI_ADEC_STREAMINFO:
		MT_INFO_ADEC("MT_MPI_ADEC_GetAttrInfo CMD: ADEC_STREAMINFO");
		retval = adec_get_str_info((ADEC_STREAMINFO_S *)pstAdecInfo);
		break;

	case MT_MPI_ADEC_BUFFERSTATUS:
		MT_INFO_ADEC("MT_MPI_ADEC_GetAttrInfo CMD: ADEC_BUFFERSTATUS");
		retval = adec_get_buf_status((ADEC_BUFSTATUS_S *)pstAdecInfo);
		break;

	case MT_MPI_ADEC_DEBUGINFO:
		MT_INFO_ADEC("MT_MPI_ADEC_GetAttrInfo CMD: ADEC_DEBUGINFO");
		retval = adec_get_dbg_info((ADEC_DEBUGINFO_S *)pstAdecInfo);
		break;

	case MT_MPI_ADEC_HaSzNameInfo:
		MT_INFO_ADEC("ADEC_GetHaSzNameInfo CMD: ADEC_HaSzNameInfo");
		retval = adec_get_decoder_name((ADEC_SzNameINFO_S *)pstAdecInfo);
		break;

	case MT_MPI_ADEC_METAINFO:
		retval = adec_get_meta_info((MT_UNF_AVPLAY_METARINFO_S *)pstAdecInfo);
		break;
		
	default:
		MT_ERR_ADEC(" MT_MPI_ADEC_GetAttrInfo  fail: INVALID PARAM = 0x%x\n", enAdecInfo);
		retval = MT_FAILURE;
	}

	return retval;
}

mt_s32 MT_MPI_ADEC_GetAudSpectrum(mt_handle hAdec, mt_u16 *pSpectrum ,mt_u32 u32BandNum)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_SetConfigDeoder( const mt_u32 enDstCodecID, mt_void *pstConfigStructure)
{
	return 0;
}


mt_s32 MT_MPI_ADEC_SetEosFlag(mt_handle hAdec)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_DropStream(mt_handle hAdec, mt_u64 u64SeekPts)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_SetCodecCmd(mt_handle hAdec, mt_void *pstCodecCmd)
{
	return 0;
}

static ADEC_EVENT_S adec_event = {0};
mt_s32 MT_MPI_ADEC_CheckNewEvent(mt_handle hAdec, ADEC_EVENT_S *pstNewEvent)
{
	u32 sample_rate, ch_num, bitdepth;
	adec_info_t *adec_info;
	MT_UNF_ACODEC_STREAMINFO_S *str_info;
	MT_UNF_AO_FRAMEINFO_S *frm_info;

	if ((NULL == pstNewEvent) || (NULL == g_aud_share_buf))
	{
		
		MT_ERR_ADEC("<ah><%s:%d> invalid parameter [%p/%p]\n", __func__, __LINE__, pstNewEvent, g_aud_share_buf);
		return -1;
	}
	ADEC_GET_USRER_SHARE_ADDR();

	adec_info = &g_aud_share_buf->adec_info;

	str_info = &adec_event.stStreamInfo;
	sample_rate = adec_info->sample_rate;
	ch_num = adec_info->ch_num;
	bitdepth = 16;		// ahren todo

	if(adec_info->status == ADEC_CLOSED || adec_info->decode_state == ADEC_READ)
	{
		return -1 ;
	}
	if ((sample_rate > 0) && (ch_num > 0))
	{
		if ((sample_rate != str_info->enSampleRate) || (ch_num != str_info->u32Channel) || (bitdepth != str_info->enBitDepth))
		{

			if ((0 != str_info->enSampleRate) || (0 != str_info->u32Channel) || (0 != str_info->enBitDepth))
			{
				adec_event.bFrameInfoChange = MT_TRUE;
			}
			str_info->enSampleRate = sample_rate; 
			str_info->u32Channel = ch_num; 
			str_info->enBitDepth = bitdepth; 
		}
		else
		{
			adec_event.bFrameInfoChange = MT_FALSE;
		}
	}

	adec_event.bUnSupportFormat = !adec_info->is_supported;

	/*if (frm_info->u32DecErrCnt > adec_info->decode_err)
	{
		adec_event.bStreamCorrupt = MT_TRUE;
		frm_info->u32DecErrCnt = adec_info->decode_err;
	}
	else
	{
		adec_event.bStreamCorrupt = MT_FALSE;
	}*/

	frm_info = &adec_event.AvplayAudFrm;
	if (frm_info->u32FrameIndex != adec_info->frm_idx)
	{
		frm_info->s32BitPerSample = 16;		// ahren todo
		frm_info->bInterleaved = MT_TRUE;
		frm_info->u32SampleRate = adec_info->sample_rate;
		frm_info->u32Channels = adec_info->ch_num;
		frm_info->u32PcmSamplesPerFrame = adec_info->sample_num;
		//frm_info->u32BitsBytesPerFrame = adec_info->;	// todo
		frm_info->u32FrameIndex = adec_info->frm_idx;
		frm_info->u32FrameCounter = adec_info->frm_idx;
		//frm_info->b_eos = adec_info->;	// todo
		//frm_info->bEac4TimeSampleRate = adec_info->;		// todo
		//frm_info->u32chan = adec_info->;
		frm_info->adectype = adec_info->aud_type;
		frm_info->u32ADFrameIndex = adec_info->ad_frm_idx;
		frm_info->u32DecErrCnt = adec_info->decode_err;
		//frm_info->u32DropCnt = adec_info->;	// todo
		//frm_info->stAudInfo = adec_info->;	// todo

		adec_event.bnewAudioFrame = MT_TRUE;
	}
	else
	{
		adec_event.bnewAudioFrame = MT_FALSE;
	}

	memcpy(pstNewEvent, &adec_event, sizeof(ADEC_EVENT_S));

	return 0;
}

mt_s32 MT_MPI_ADEC_AvcConfig(mt_handle hAdec,  MT_HADECODE_AVC_PARAM_S * pstAvcParam)
{
	return 0;
}

void ADEC_AD_PUT_PCM(mt_u8 *pcmdata,mt_u32 pcmlen)
{
	(void)pcmdata;
	(void)pcmlen;
}

mt_s32 ADEC_AD_SET_VOL_WEIGHT(mt_u32 *vol_weight)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf) {
		g_aud_share_buf_av->ad_vol_weight = *vol_weight;
		g_aud_share_buf_ta->ad_vol_weight = *vol_weight;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 ADEC_AD_GET_VOL_WEIGHT(mt_u32 *vol_weight)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf) {
		*vol_weight = g_aud_share_buf->ad_vol_weight;
		ret = MT_SUCCESS;
	}

	return ret;
}


void ADEC_AD_SET_DATATYPE(mt_handle hAdec, mt_u32 type)
{
	int ret = 0;
	
	ret = ioctl(hAdec, IO_ADEC_AD_ENABLE, type);
	if (ret != MT_SUCCESS)
	{
		MT_ERR_ADEC("IO_ADEC_AD_ENABLE error.\n");
	}

}

mt_u32 ADEC_AD_PCM_GETFREE(void)
{
	return 0;
}

mt_s32 MT_MPI_ADEC_Enable_HEAAC(mt_handle hAdec)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->aac_dec_sel = 0;
		g_aud_share_buf_ta->aac_dec_sel = 0;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_set_downmix_enable(mt_handle hAdec, mt_u32 enable)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->downmix_enable = enable;
		g_aud_share_buf_ta->downmix_enable = enable;
		ret = MT_SUCCESS;
	}

	ret |= ioctl(hAdec, IO_ADEC_SET_DOWNMIX_ENABLE, enable);
	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Downmix_Mode(mt_u32 *type)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		switch (*type) {
			case 0://LtRt
				g_aud_share_buf_av->ac4_configs.pcm_out_type = 1;
				g_aud_share_buf_av->ac4_configs.downmix_type = 0;

				g_aud_share_buf_ta->ac4_configs.pcm_out_type = 1;
				g_aud_share_buf_ta->ac4_configs.downmix_type = 0;
				break;
			case 1://LoRo
				g_aud_share_buf_av->ac4_configs.pcm_out_type = 1;
				g_aud_share_buf_av->ac4_configs.downmix_type = 1;

				g_aud_share_buf_ta->ac4_configs.pcm_out_type = 1;
				g_aud_share_buf_ta->ac4_configs.downmix_type = 1;
				break;

			case 2://PCM_5_1
				g_aud_share_buf_av->ac4_configs.pcm_out_type = 2;
				g_aud_share_buf_ta->ac4_configs.pcm_out_type = 2;
				break;

			case 3://PCM_RAW
				g_aud_share_buf_av->ac4_configs.pcm_out_type = 3;
				g_aud_share_buf_ta->ac4_configs.pcm_out_type = 3;
				break;

			default:
				MT_ERR_ADEC("unsupported ac4 downmix type.\n");
				break;
		}
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Dialogue_Enhancement(mt_u32 *val)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.de_value = *val;
		g_aud_share_buf_ta->ac4_configs.de_value = *val;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Encode_DD_DDP(mt_u32 *encode_type)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.ddp_out_type = *encode_type;
		g_aud_share_buf_ta->ac4_configs.ddp_out_type = *encode_type;

		g_aud_share_buf_av->ac4_configs.mat_enc_out_en = 0;
		g_aud_share_buf_ta->ac4_configs.mat_enc_out_en = 0;
		
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Encode_MAT(mt_u32 *enable)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.mat_enc_out_en = *enable;
		g_aud_share_buf_ta->ac4_configs.mat_enc_out_en = *enable;
		if(*enable == 1) {
			g_aud_share_buf_av->ac4_configs.ddp_out_type = 0;
			g_aud_share_buf_ta->ac4_configs.ddp_out_type = 0;
		}
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Encode_DAP(mt_u32 *encode_type)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.dap_out_type = *encode_type;
		g_aud_share_buf_ta->ac4_configs.dap_out_type = *encode_type;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_AD_OnOff(mt_u32 *enable)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.associated_audio_mixing = *enable;
		g_aud_share_buf_ta->ac4_configs.associated_audio_mixing = *enable;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_AD_Weight(mt_s32 *val)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.user_balance_adjustment = *val;
		g_aud_share_buf_ta->ac4_configs.user_balance_adjustment = *val;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Sel_AC4_AD_Type(mt_u32 *val)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.b_ac4_pref_assoc_type_over_lang = 1;
		g_aud_share_buf_ta->ac4_configs.b_ac4_pref_assoc_type_over_lang = 1;
		
		g_aud_share_buf_av->ac4_configs.ac4_associated_type = *val;
		g_aud_share_buf_ta->ac4_configs.ac4_associated_type = *val;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Sel_AC4_AD_Type_Over_Lang()
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.b_ac4_pref_assoc_type_over_lang = 0;
		g_aud_share_buf_ta->ac4_configs.b_ac4_pref_assoc_type_over_lang = 0;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Sel_AC4_Lang(MT_UNF_AVPLAY_AC4_LANG_S *ac4_lang)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		memcpy(g_aud_share_buf_av->ac4_configs.ac4_1st_pref_lang, ac4_lang->ac4_1st_lang, 3);
		memcpy(g_aud_share_buf_ta->ac4_configs.ac4_1st_pref_lang, ac4_lang->ac4_1st_lang, 3);

		memcpy(g_aud_share_buf_av->ac4_configs.ac4_2nd_pref_lang, ac4_lang->ac4_2nd_lang, 3);
		memcpy(g_aud_share_buf_ta->ac4_configs.ac4_2nd_pref_lang, ac4_lang->ac4_2nd_lang, 3);

		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_AC4_Pres_ID(mt_s32 *val)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->ac4_configs.ac4_pres_group_index = *val;
		g_aud_share_buf_ta->ac4_configs.ac4_pres_group_index = *val;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_Dolby_Force_MS12_Dec(mt_u32 *enable)
{
	mt_s32 ret = MT_FAILURE;
	
	if (NULL != g_aud_share_buf)
	{
		g_aud_share_buf_av->dolby_force_ms12_dec = *enable;
		g_aud_share_buf_ta->dolby_force_ms12_dec = *enable;
		ret = MT_SUCCESS;
	}

	return ret;
}

mt_s32 MT_MPI_ADEC_Set_VIVID_Pos(float x, float y, float z)
{
	
#ifdef EXT_AUDIO_DECODER_ENABLE
	if(a_type != HA_AUDIO_ID_VVID) 
		return -1;

	if(ext_dec)
		ext_dec->setpos(x,y,z);
	
	return 0;
#else
	return -1;
#endif
}

mt_s32 MT_MPI_ADEC_Sel_VIVID_Obj(mt_u16 id, mt_u16 on)
{
#ifdef EXT_AUDIO_DECODER_ENABLE
	if(a_type != HA_AUDIO_ID_VVID) 
		return -1;

	if(ext_dec)
		ext_dec->selobj(id, on);
	
	return 0;
#else
	return -1;
#endif
}

mt_s32 MT_MPI_ADEC_set_trickmode(mt_handle hAdec, u32 mode)
{
	mt_s32 ret = MT_FAILURE;

	ret = ioctl(hAdec, IO_ADEC_TRICK_MODE, mode);
	return ret;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
