/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h> 

#include "mt_type.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "mt_drv_module.h"
#include "mt_drv_dma.h"
#include "mt_cache.h"
#include "mt_common.h"
#include "mt_error_mpi.h"
#include "adec/adec_k.h"
#include "adec/adec_api.h"
#include "mt_drv_dump.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_audio_codec.h"
#include "mt_kernel_adapt.h"

#define  AV_CPU_ENABLE  1

extern mt_s32 hal_dma_active_channel(mt_s32 id);
extern mt_s32 hal_dma_deactive_channel(mt_s32 id);
extern mt_s32 config_downmix_mode(void);
MT_DECLARE_MUTEX(g_adec_es_mutex);

#define TMP_INPUT_ES_SIZE 4096
u32 g_adec_prt_level = (1 << ADEC_PRT_ERR) + (1 << ADEC_PRT_INFO) + (1 << ADEC_PRT_DGB);
//u32 g_adec_prt_level = (1 << ADEC_PRT_ERR);
adec_dev_t *g_adev = NULL;

#define ADEC_GET_SHARE_ADDR()\
do{\
    if (NULL == g_adev)\
    {\
        adec_err("g_adev is null \n");\
        return -1;\
    }\
    if(1 == g_adev->is_ac3or4)\
    {\
        g_adev->share_buf = g_adev->share_buf_mmz_ta.startVirAddr;\
    }\
    else\
    {\
		g_adev->share_buf = g_adev->share_buf_mmz_av.startVirAddr;\
    }\
    g_adev->es_buf = &g_adev->share_buf->es_buf;\
	g_adev->ad_es_buf = &g_adev->share_buf->ad_es_buf;\
	g_adev->adec_info = &g_adev->share_buf->adec_info;\
   }while(0)

mt_s32 use_adec_ta(aud_share_buf_t **share_buf)
{
	if (NULL == g_adev)
	{
		adec_err("g_adev is null \n");
		return -1;
	}
		
    if(1 == g_adev->is_ac3or4)
    {
        *share_buf  = g_adev->share_buf_mmz_ta.startVirAddr;
    }
    else
    {
		*share_buf  = g_adev->share_buf_mmz_av.startVirAddr;
    }
	return g_adev->is_ac3or4;
}

mt_s32 get_sharebuf_av(aud_share_buf_t **share_buf)
{
	if (NULL == g_adev) {
		adec_err("g_adev is null \n");
		return MT_FAILURE;
	}

	*share_buf  = g_adev->share_buf_mmz_av.startVirAddr;
	return MT_SUCCESS;
}

mt_s32 get_sharebuf_ta(aud_share_buf_t **share_buf)
{
	if (NULL == g_adev) {
		adec_err("g_adev is null \n");
		return MT_FAILURE;
	}

    *share_buf  = g_adev->share_buf_mmz_ta.startVirAddr;
	return MT_SUCCESS;
}


int dump_data(const char *data, int size)
{
	int ret;
	static void *handle = NULL;

	if (NULL == handle)
	{
		handle = mt_drv_dump_create(E_DUMP_TYPE_FILE, "/media/sda1/audio.bin", 0);
		if (NULL == handle)
		{
			adec_err("mt_drv_dump_create error!\n");
			return -1;
		}
	}

	ret = mt_drv_dump_do(handle, (const mt_u8 *)data, size);
	if (ret != size)
	{
		adec_err("dump data error!!! size:%d\n", size);
		return -1;
	}

	return 0;
}

int get_adec_es_buf_info(int *data_size, int *buf_size)
{
	adec_es_buf_t *es_buf;

	if (NULL == g_adev)
	{
		adec_err("g_adev is NULL!");
		return -1;
	}
	ADEC_GET_SHARE_ADDR();

	es_buf = g_adev->es_buf;
	if (NULL == es_buf)
	{
		adec_err("es_buf is NULL!");
		return -1;
	}

	if (es_buf->es_wt >= es_buf->es_rd)
	{
		*data_size = es_buf->es_wt - es_buf->es_rd;
	}
	else
	{
		*data_size = es_buf->es_buf_size - es_buf->es_rd + es_buf->es_wt;
	}

	*buf_size = es_buf->es_buf_size;

	return 0;
}

#ifndef CONFIG_MT_FPGA
#if 0
static int adec_task(void *param)
{
	//u8 *data;
	//int ret;
	adec_dev_t *adev = (adec_dev_t *)param;
	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}
	
	while (1)
	{
		if (kthread_should_stop())
		{
			//adec_info("exit adec task!\n");
			break;
		}

		msleep_interruptible(20);
	}

	return 0;
}
#endif
extern int alloc_snd_sw_buf(aud_share_buf_t *share_buf, aud_share_buf_t *share_buf_av, aud_buf_param_t *buf_param);
static int alloc_aud_memory(adec_dev_t *adev)
{
	int ret;
	unsigned char buf_name[32] = {0};
	aud_share_buf_t * share_buf_av = NULL;
	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}

#ifdef CONFIG_MT_FPGA	
	ret = mt_drv_mmz_alloc("tmp_es_input_mmz", MMZ_ZONE_DDR,  TMP_INPUT_ES_SIZE, CACHE_LINE_SIZE, &adev->tmp_es_input_mmz);
	if (0 != ret)
	{
		adec_err("drv_mmz_alloc_and_map tmp_es_input_mmz error!");
		return -1;
	}
	ret = mt_drv_mmz_map(&adev->tmp_es_input_mmz);		// for avcpu
	if (0 != ret)
	{
		adec_err("drv_mmz_map_cache adec tmp_es_input_mmz error!");
		mt_drv_mmz_release(&adev->tmp_es_input_mmz);
		return -1;
	}
#endif
	snprintf(buf_name, sizeof(buf_name), "adec_shared_ta");
	ret = mt_drv_mmz_alloc(buf_name, MMZ_ZONE_DDR, sizeof(aud_share_buf_t), CACHE_LINE_SIZE, &adev->share_buf_mmz_ta);
	if (0 != ret)
	{
		adec_err("drv_mmz_alloc_and_map adec share buffer error!");
		return -1;
	}
	adev->aud_buf_param.share_buf_phy = adev->share_buf_mmz_ta.startPhyAddr;
	adec_err("share phy:%lx\n", (ulong)adev->share_buf_mmz_ta.startPhyAddr);
	ret = mt_drv_mmz_map_cache(&adev->share_buf_mmz_ta);	// for tee   mzhu
	if (0 != ret)
	{
		adec_err("drv_mmz_map_cache adec share buffer error!");
		mt_drv_mmz_release(&adev->share_buf_mmz_ta);
		return -1;
	}
	memset(adev->share_buf_mmz_ta.startVirAddr, 0, sizeof(aud_share_buf_t));

	snprintf(buf_name, sizeof(buf_name), "adec_shared_av");
	ret = mt_drv_mmz_alloc(buf_name, MMZ_ZONE_DDR, sizeof(aud_share_buf_t), CACHE_LINE_SIZE, &adev->share_buf_mmz_av);
	if (0 != ret)
	{
		adec_err("drv_mmz_alloc_and_map adec share buffer error!");
		return -1;
	}
	ret = mt_drv_mmz_map(&adev->share_buf_mmz_av);	// for av   mzhu
	if (0 != ret)
	{
		adec_err("drv_mmz_map_cache adec share buffer error!");
		mt_drv_mmz_release(&adev->share_buf_mmz_av);
		return -1;
	}
	
	memset(adev->share_buf_mmz_av.startVirAddr, 0, sizeof(aud_share_buf_t));
	

	snprintf(buf_name, sizeof(buf_name), "adec_es");
	ret = mt_drv_mmz_alloc(buf_name, MMZ_ZONE_PCM, ADEC_ES_BUF_SIZE * 2, CACHE_LINE_SIZE, &adev->es_buf_mmz);
	if (0 != ret)
	{
		adec_err("drv_mmz_alloc_and_map adec es buffer error!");
		return -1;
	}
	adev->aud_buf_param.es_buf_phy = adev->es_buf_mmz.startPhyAddr;
	ret = mt_drv_mmz_map(&adev->es_buf_mmz);	// for avcpu
	if (0 != ret)
	{
		adec_err("drv_mmz_map_cache adec es share buffer error!");
		mt_drv_mmz_release(&adev->share_buf_mmz_ta);
		mt_drv_mmz_release(&adev->share_buf_mmz_av);
		mt_drv_mmz_release(&adev->es_buf_mmz);
		return -1;
	}
	
	adec_dbg("alloc adec es buf success! size:%ld vir_addr:%lx phy_addr:%llx\n", adev->es_buf_mmz.size,
		(ulong)adev->es_buf_mmz.startVirAddr, adev->es_buf_mmz.startPhyAddr);


	//init ta share_buf ta
	adev->share_buf = (aud_share_buf_t *)adev->share_buf_mmz_ta.startVirAddr;
	
	adev->share_buf->es_buf.data_phy = adev->es_buf_mmz.startPhyAddr;
	adev->share_buf->es_buf.data_kern_vir = (u64)((ulong)adev->es_buf_mmz.startVirAddr);
	adev->share_buf->es_buf.pkt_cnt = ADEC_ES_HEADER_CNT;
	adev->share_buf->es_buf.es_buf_size = ADEC_ES_BUF_SIZE;
	
	adev->share_buf->ad_es_buf.data_phy = adev->es_buf_mmz.startPhyAddr + ADEC_ES_BUF_SIZE;
	adev->share_buf->ad_es_buf.data_kern_vir = (u64)((ulong)(adev->es_buf_mmz.startVirAddr + ADEC_ES_BUF_SIZE));
	adev->share_buf->ad_es_buf.pkt_cnt = ADEC_ES_HEADER_CNT;
	adev->share_buf->ad_es_buf.es_buf_size = ADEC_ES_BUF_SIZE;
	
	adev->share_buf->pcm_dump.size = sizeof(adev->share_buf->pcm_dump.data);
	adev->share_buf->es_dump.size = sizeof(adev->share_buf->es_dump.data);
	adev->share_buf->log_level_flag = 4;	// default output error log
	adev->share_buf->downmix_enable = 1;	// default enable audio downmix
	adev->share_buf->aac_dec_sel = 2;		// default disable heaac

	adev->share_buf->ac4_configs.pcm_out_type = 0xff;
	adev->share_buf->ac4_configs.ddp_out_type = 0xff;
	adev->share_buf->ac4_configs.de_value = 0xff;
	adev->share_buf->ac4_configs.downmix_type = 0xff;
	adev->share_buf->ac4_configs.mat_enc_out_en = 0xff;
	adev->share_buf->ac4_configs.associated_audio_mixing = 0xff;
	adev->share_buf->ac4_configs.user_balance_adjustment = 0xff;
	adev->share_buf->ac4_configs.ac4_associated_type = 0xff;
	adev->share_buf->ac4_configs.b_ac4_pref_assoc_type_over_lang = 0xff;
	adev->share_buf->ac4_configs.ac4_pres_group_index = 0xff;
	adev->share_buf->ac4_configs.dap_out_type = 0xff;
	adev->share_buf->dolby_force_ms12_dec = 0;

	adev->aud_buf_param.aud_buf_size_map.es_buf_size = ADEC_ES_BUF_SIZE;
	adev->aud_buf_param.aud_buf_size_map.ad_es_buf_size = ADEC_ES_BUF_SIZE;
	memset(adev->share_buf->ac4_configs.ac4_1st_pref_lang, 0, sizeof(adev->share_buf->ac4_configs.ac4_1st_pref_lang));
	memset(adev->share_buf->ac4_configs.ac4_2nd_pref_lang, 0, sizeof(adev->share_buf->ac4_configs.ac4_2nd_pref_lang));
	//adev->aud_buf_param.aud_buf_size_map.ad_es_buf_size = 0;

	// call snd alloc function
#ifdef AV_CPU_ENABLE
	//init  share_buf  av
	share_buf_av = (aud_share_buf_t *)adev->share_buf_mmz_av.startVirAddr;
	adev->aud_buf_param.share_buf_phy_av = adev->share_buf_mmz_av.startPhyAddr;
	share_buf_av->es_buf.data_phy = adev->es_buf_mmz.startPhyAddr;
	share_buf_av->es_buf.data_kern_vir = (u64)((ulong)adev->es_buf_mmz.startVirAddr);
	share_buf_av->es_buf.pkt_cnt = ADEC_ES_HEADER_CNT;
	share_buf_av->es_buf.es_buf_size = ADEC_ES_BUF_SIZE;
	
	share_buf_av->ad_es_buf.data_phy = adev->es_buf_mmz.startPhyAddr + ADEC_ES_BUF_SIZE;
	share_buf_av->ad_es_buf.data_kern_vir = (u64)((ulong)(adev->es_buf_mmz.startVirAddr + ADEC_ES_BUF_SIZE));
	share_buf_av->ad_es_buf.pkt_cnt = ADEC_ES_HEADER_CNT;
	share_buf_av->ad_es_buf.es_buf_size = ADEC_ES_BUF_SIZE;
	
	share_buf_av->pcm_dump.size = sizeof(adev->share_buf->pcm_dump.data);
	share_buf_av->es_dump.size = sizeof(adev->share_buf->es_dump.data);
	share_buf_av->log_level_flag = 4;	// default output error log
	share_buf_av->downmix_enable = 1;	// default enable audio downmix
	share_buf_av->aac_dec_sel = 2;		// default disable heaac

	share_buf_av->ac4_configs.pcm_out_type = 0xff;
	share_buf_av->ac4_configs.ddp_out_type = 0xff;
	share_buf_av->ac4_configs.de_value = 0xff;
	share_buf_av->ac4_configs.downmix_type = 0xff;
	share_buf_av->ac4_configs.mat_enc_out_en = 0xff;
	share_buf_av->ac4_configs.associated_audio_mixing = 0xff;
	share_buf_av->ac4_configs.user_balance_adjustment = 0xff;
	share_buf_av->ac4_configs.ac4_associated_type = 0xff;
	share_buf_av->ac4_configs.b_ac4_pref_assoc_type_over_lang = 0xff;
	share_buf_av->ac4_configs.ac4_pres_group_index = 0xff;
	share_buf_av->ac4_configs.dap_out_type = 0xff;
	share_buf_av->dolby_force_ms12_dec = 0;
	
	memset(share_buf_av->ac4_configs.ac4_1st_pref_lang, 0, sizeof(share_buf_av->ac4_configs.ac4_1st_pref_lang));
	memset(share_buf_av->ac4_configs.ac4_2nd_pref_lang, 0, sizeof(share_buf_av->ac4_configs.ac4_2nd_pref_lang));

#endif
	ret = alloc_snd_sw_buf(adev->share_buf, share_buf_av,  &adev->aud_buf_param);

	return ret;
}

extern int release_snd_sw_buf(void);
static int release_aud_memory(adec_dev_t *adev)
{
	mt_drv_mmz_unmap(&adev->share_buf_mmz_ta);
	mt_drv_mmz_release(&adev->share_buf_mmz_ta);
	mt_drv_mmz_unmap(&adev->share_buf_mmz_av);
	mt_drv_mmz_release(&adev->share_buf_mmz_av);
	mt_drv_mmz_unmap(&adev->es_buf_mmz);
	mt_drv_mmz_release(&adev->es_buf_mmz);
#ifdef CONFIG_MT_FPGA	
	mt_drv_mmz_unmap(&adev->tmp_es_input_mmz);
	mt_drv_mmz_release(&adev->tmp_es_input_mmz);
#endif
	return release_snd_sw_buf();
}

static int adec_send_cmd2ta(adec_dev_t *adev, adec_ree_to_tee_cmd_t *adec_cmd, int cmd)
{
	int n = 0;
	adec_cmd->cmd = cmd;
	adec_cmd->exec = 0;

	if(2 == adev->aud_buf_param.ta_enabled)   //tee not exists 
	{
		return 0;
	}

	while (0 == adec_cmd->exec)
	{
		msleep_interruptible(1);
		if (++n > ADEC_CMD_TIMEOUT)
		{
			adec_err("wait cmd:%d exec timeout!\n", cmd);
			return -1;
		}
	}
	
	return adec_cmd->ret;
}

static int adec_send_cmd2av(adec_dev_t *adev, adec_ree_to_tee_cmd_t *adec_cmd, int cmd)
{
	int ret = 0;
	mt_u8 ack;	
	ipc_msg_t msg_send;
	
	msg_send.msg_id = cmd;
	msg_send.param1 = 0;
	msg_send.param2 = 0;
	msg_send.time_out_ms = ADEC_CMD_TIMEOUT;
	
	ack = 1;
	ret = ap_send_to_av(AP_SYS_DEV_AUD, &msg_send, ack);
	if (0 != ret)
	{
		adec_err("ap_send_to_av error, ret:%d\n", ret);
	}
	
	return adec_cmd->ret;
}

static int adec_exec_cmd(adec_dev_t *adev, adec_ree_to_tee_cmd_t *adec_cmd, int cmd)
{
	if(TA_ADEC_CLOSE == cmd || TA_ADEC_AD_ENABLE == cmd) {
		
#ifdef CONFIG_TEE
		aud_share_buf_t * cmd_ta = g_adev->share_buf_mmz_ta.startVirAddr;
#endif
		aud_share_buf_t * cmd_av = g_adev->share_buf_mmz_av.startVirAddr;
		adec_cmd->ret = adec_send_cmd2av(adev, &cmd_av->adec_cmd, cmd);
#ifdef CONFIG_TEE
		adec_cmd->ret |= adec_send_cmd2ta(adev, &cmd_ta->adec_cmd, cmd);
#endif
	} else {
		if(g_adev->is_ac3or4)
		{
			adec_cmd->ret = adec_send_cmd2ta(adev, adec_cmd, cmd);
		}
		else
		{
			adec_cmd->ret = adec_send_cmd2av(adev, adec_cmd, cmd);
		}
	}
	
	return adec_cmd->ret;
}

static int adec_stop(adec_dev_t *adev, ulong param)
{
	int ret = 0;

	adec_dbg("\n");
	adev->status = ADEC_STOPED;
	
	ADEC_GET_SHARE_ADDR();
	
	//adev->share_buf->adec_cmd.param0 = (int)param;
	hal_dma_deactive_channel(DMA_CHANNEL_SECURE_AUDIO_0);
	
#ifdef CONFIG_MT_EXT_VVID_SUPPORT
	if(g_adev->atype != HA_AUDIO_ID_VVID)
		ret = adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_STOP);
#else
	ret = adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_STOP);
#endif

	adec_es_buf_clear(&adev->share_buf->es_buf);
	adec_es_buf_clear(&adev->share_buf->ad_es_buf);

	adev->share_buf->pcm_dump.rd = 0;
	adev->share_buf->pcm_dump.wt = 0;
	adev->share_buf->es_dump.rd = 0;
	adev->share_buf->es_dump.wt = 0;

	if((mt_u32)param == 0 ) {
		adev->share_buf->ac4_configs.pcm_out_type = 0xff;
		adev->share_buf->ac4_configs.ddp_out_type = 0xff;
		adev->share_buf->ac4_configs.de_value = 0xff;
		adev->share_buf->ac4_configs.downmix_type = 0xff;
		adev->share_buf->ac4_configs.mat_enc_out_en = 0xff;
		adev->share_buf->ac4_configs.associated_audio_mixing = 0xff;
		adev->share_buf->ac4_configs.user_balance_adjustment = 0xff;
		adev->share_buf->ac4_configs.ac4_associated_type = 0xff;
		adev->share_buf->ac4_configs.b_ac4_pref_assoc_type_over_lang = 0xff;
		adev->share_buf->ac4_configs.ac4_pres_group_index = 0xff;
		adev->share_buf->ac4_configs.dap_out_type = 0xff;
		adev->share_buf->dolby_force_ms12_dec = 0;
		memset(adev->share_buf->ac4_configs.ac4_1st_pref_lang, 0, sizeof(adev->share_buf->ac4_configs.ac4_1st_pref_lang));
		memset(adev->share_buf->ac4_configs.ac4_2nd_pref_lang, 0, sizeof(adev->share_buf->ac4_configs.ac4_2nd_pref_lang));
	}
	
	memset(adev->adec_info, 0, sizeof(adec_info_t));
	mt_dcache_flush(adev->share_buf, sizeof(aud_share_buf_t));
	
	return ret;
}

static int adec_start(adec_dev_t *adev, ulong param)
{
	int ret;
	MT_HADECODE_OPENPARAM_S dec_param = {0};
	adec_ree_to_tee_cmd_t *adec_cmd;
	adec_es_buf_clear(&adev->share_buf->es_buf);
	adec_es_buf_clear(&adev->share_buf->ad_es_buf);
	
	hal_dma_active_channel(DMA_CHANNEL_SECURE_AUDIO_0);

	ret = copy_from_user(&dec_param, (char *)param, sizeof(MT_HADECODE_OPENPARAM_S));
	if (0 != ret)
	{
		adec_err("copy_from_user error, only copy %d byte, need copy %ld byte!\n", ret, (ulong)sizeof(MT_HADECODE_OPENPARAM_S));
		return ret;
	}

	g_adev->atype = dec_param.u32CodecId;
	if((dec_param.u32CodecId ==  HA_AUDIO_ID_TRUEHD) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_AC3PASSTHROUGH) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_EAC3PASSTHROUGH) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_DOLBY_PLUS) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_DOLBY_TRUEHD) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_DOLBY_CONVERT) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_VVID) ||
		(dec_param.u32CodecId ==  HA_AUDIO_ID_DOLBY_AC4))// ||
		//(dec_param.u32CodecId ==  HA_AUDIO_ID_AAC))
	{
		g_adev->is_ac3or4 = 1;
	}
	else
	{
		g_adev->is_ac3or4 = 0;
		
	}
	
#ifndef AV_CPU_ENABLE
	g_adev->is_ac3or4 = 1;   // allways on TEE
#endif
	
#ifndef CONFIG_TEE
	 g_adev->is_ac3or4 = 0;  // allways on av_cpu
#endif

	if(	2 == g_adev->aud_buf_param.ta_enabled)  //no ta exists
	{
		g_adev->is_ac3or4 = 0;	// allways on av_cpu
	}

//retry:
	ADEC_GET_SHARE_ADDR();

	adec_cmd = &adev->share_buf->adec_cmd;

	if ((NULL != dec_param.pCodecPrivateData) && (dec_param.u32CodecPrivateDataSize > 0))
	{
		if (sizeof(adec_cmd->param1) < dec_param.u32CodecPrivateDataSize)
		{
			adec_err("param buf too small, need:%d actual:%ld\n", dec_param.u32CodecPrivateDataSize, (ulong)sizeof(adec_cmd->param1));
			return -1;
		}

		ret = copy_from_user(adec_cmd->param1, dec_param.pCodecPrivateData, dec_param.u32CodecPrivateDataSize);
		if (0 != ret)
		{
			adec_err("copy_from_user error, only copy %d byte, need copy %d byte!\n", ret, dec_param.u32CodecPrivateDataSize);
			return ret;
		}
		adec_cmd->param1_size = dec_param.u32CodecPrivateDataSize;
	}

	adec_cmd->param0 = dec_param.u32CodecId;

#ifdef CONFIG_MT_EXT_VVID_SUPPORT
	if(g_adev->atype != HA_AUDIO_ID_VVID)
		ret = adec_exec_cmd(adev, adec_cmd, TA_ADEC_START);
	else
		g_adev->is_ac3or4 = 0;	// allways on av_cpu
#else
	ret = adec_exec_cmd(adev, adec_cmd, TA_ADEC_START);
/*
	if(g_adev->is_ac3or4 == 1 && dec_param.u32CodecId == HA_AUDIO_ID_AAC) {
		if(adev->share_buf->adec_info.is_supported == 0) {
			pr_err("try aac on avcpu!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			adec_stop(adev, param);
			g_adev->is_ac3or4 = 0;
			goto retry;
			//ret = adec_exec_cmd(adev, adec_cmd, TA_ADEC_START);
		}
	}
*/
#endif

	adev->status = ADEC_STARTED;

	return ret;
}

static int adec_ad_enable(adec_dev_t *adev, ulong param)
{
	adec_dbg("\n");
	ADEC_GET_SHARE_ADDR();
	adev->ad_enable = (int)param;
	//adev->share_buf->adec_cmd.param0 = (int)param;
	
#ifdef CONFIG_TEE
	aud_share_buf_t * cmd_ta = g_adev->share_buf_mmz_ta.startVirAddr;
	cmd_ta->adec_cmd.param0 = (int)param;
#endif
	aud_share_buf_t * cmd_av = g_adev->share_buf_mmz_av.startVirAddr;
	cmd_av->adec_cmd.param0 = (int)param;

	return adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_AD_ENABLE);
}

static int adec_pause(adec_dev_t *adev)
{
	if (NULL == adev)
	{
		adec_err("invalid parameter [%p]\n", adev);
		return -1;
	}

	if (ADEC_STARTED != adev->status)
	{
		adec_err("adec not start, status:%d\n", adev->status);
		return 0;
	}
	
	ADEC_GET_SHARE_ADDR();
	adev->status = ADEC_PAUSED;
	adev->share_buf->adec_cmd.param0 = 0;
	return adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_PAUSE);
}

static int adec_resume(adec_dev_t *adev)
{
	int ret;
	
	if (NULL == adev)
	{
		adec_err("invalid parameter [%p]\n", adev);
		return -1;
	}

	if (ADEC_PAUSED != adev->status)
	{
		adec_err("adec not pause, status:%d\n", adev->status);
		return 0;
	}

	ADEC_GET_SHARE_ADDR();
	adev->share_buf->adec_cmd.param0 = 0;
	ret = adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_RESUME);
	adev->status = ADEC_STARTED;

	return ret;
}

void adec_flush_snd(void);

static int adec_flush_es_buf(adec_dev_t *adev)
{
	int ret;
	
	if (NULL == adev)
	{
		adec_err("invalid parameter [%p]\n", adev);
		return -1;
	}

	ret = adec_pause(adev);
	if (0 != ret)
	{
		adec_err("adec_pause error!\n");
		return -1;
	}

	ADEC_GET_SHARE_ADDR();
	adec_es_buf_clear(&adev->share_buf->es_buf);
	adec_es_buf_clear(&adev->share_buf->ad_es_buf);

	mt_dcache_flush(adev->share_buf, sizeof(aud_share_buf_t));
	
	adec_flush_snd();
	
	ret = adec_resume(adev);
	if (0 != ret)
	{
		adec_err("adec_resume error!\n");
		return -1;
	}

	return 0;
}

void adec_flush_all(void)
{
#ifndef CONFIG_MT_FPGA	
	(void)down_interruptible(&g_adec_es_mutex);
	adec_flush_es_buf(g_adev);
	up(&g_adec_es_mutex);
#endif
}
mt_u32 adec_flush_status(void)
{
	return g_adev->flush_flag;
}
EXPORT_SYMBOL(adec_flush_status);
EXPORT_SYMBOL(adec_flush_all);

static int adec_enable_heaac(adec_dev_t *adev)
{	
	if (NULL == adev)
	{
		adec_err("invalid parameter [%p]\n", adev);
		return -1;
	}
	ADEC_GET_SHARE_ADDR();
	adev->share_buf->adec_cmd.param0 = 0;
	return adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_ENABLE_HEAAC);
}
#endif

mt_s32 MT_DRV_ADEC_Write(mt_u32 pid, phys_addr_t addr, mt_u32 len, mt_u64 pts, mt_u32 pts_valid)
{
	int ret;
	adec_dev_t *adev = g_adev;
	ADEC_GET_SHARE_ADDR();

	static u32 last_pts = 0;
	if ((NULL == adev) || (0 == addr) || (0 == len))
	{
		adec_err("invalid parameter [0x%p/%#llx/%d]\n", adev, addr, len);
		return -1;
	}

	if (ADEC_STARTED != adev->status)
	{
		return -1;
	}

	//drop es in trick mode
	if(adev->trick_mode)
		return 0;

	//u8 *p = (u8 *)addr;
	//adec_err("[%x/%x/%x/%x/%x/%x/%x/%x/%x]\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
	//dump_data((char *)addr, len);

	if (last_pts == (u32)pts)
	{
		pts_valid = 0;
	}
	else
	{
		pts_valid = 1;
	}
	//adec_err("pts:%#x valid:%d diff:%d \n", pts, pts_valid, pts-last_pts);
	(void)down_interruptible(&g_adec_es_mutex);
	ret = adec_es_write(adev->es_buf,  addr, len, pts, pts_valid);
	up(&g_adec_es_mutex);
	if (ret != len)
	{
		//adec_err("write data to es_buf error!\n");
		return MT_ERR_ADEC_IN_BUF_FULL;
	}
	adev->es_size += len;
	adev->es_cnt++;

	last_pts = pts;
	
	return 0;
}
EXPORT_SYMBOL(MT_DRV_ADEC_Write);

mt_s32 MT_DRV_ADEC_Write_AD(mt_u32 pid, phys_addr_t addr, mt_u32 len, mt_u64 pts, mt_u32 pts_valid)
{
	int ret;
	adec_dev_t *adev = g_adev;
	ADEC_GET_SHARE_ADDR();

	if ((NULL == adev) || (0 == addr) || (0 == len))
	{
		adec_err("invalid parameter [0x%p/%#llx/%d]\n", adev, addr, len);
		return -1;
	}

	if (ADEC_STARTED != adev->status)
	{
		return 0;
	}

	//drop es in trick mode
	if(adev->trick_mode)
		return 0;
	
	ret = adec_es_write(adev->ad_es_buf, addr, len, pts, pts_valid);
	if (ret != len)
	{
		//adec_err("write data to es_buf error!\n");
		return MT_ERR_ADEC_IN_BUF_FULL;
	}
	adev->ad_es_size += len;
	adev->ad_es_cnt++;
	
	return 0;
}
EXPORT_SYMBOL(MT_DRV_ADEC_Write_AD);

#ifndef CONFIG_MT_FPGA
#ifdef AV_CPU_ENABLE	
static int ipc_init(adec_dev_t *adev);
#endif
static int adec_drv_open(struct inode *inode, struct file *filp)
{
#ifdef AV_CPU_ENABLE	
	int ret;
	ipc_msg_t msg_send;
	u8 ack;
#endif

	adec_dev_t *adev = container_of(inode->i_cdev, adec_dev_t, dev);
	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}
	filp->private_data = adev;

    if (atomic_inc_return(&adev->atmOpenCnt) > 1) {
		adec_dbg("open adev atmOpenCnt:%d\n", atomic_read(&adev->atmOpenCnt));
		return 0;
    }
	
#ifdef AV_CPU_ENABLE	
	if(g_adev->ipc_inited == 0)
	{
		ipc_init(g_adev);
	}
	msg_send.msg_id = TA_ADEC_OPEN;
	msg_send.param1 = g_adev->share_buf_mmz_av.startPhyAddr;
	msg_send.param2 = sizeof(aud_share_buf_t);
	msg_send.time_out_ms = 1000;
	
	ack = 1;
	adec_err("g_adev->share_buf_mmz_av.startPhyAddr %lx \n", (ulong)g_adev->share_buf_mmz_av.startPhyAddr);
	ret = ap_send_to_av(AP_SYS_DEV_AUD, &msg_send, ack);
	if (0 != ret)
	{
		adec_err("ap_send_to_av error, ret:%d\n", ret);
	}
#endif

	adev->status = ADEC_OPENED;

	return 0;
}

static int adec_drv_release(struct inode *inode, struct file *filp)
{
	adec_dev_t *adev = filp->private_data;

	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}
	
	ADEC_GET_SHARE_ADDR();

	atomic_dec_return(&adev->atmOpenCnt);
	adec_err("close adev atmOpenCnt:%d\n",  atomic_read(&adev->atmOpenCnt));
#ifndef CONFIG_TEE
	if(atomic_read(&adev->atmOpenCnt) == 0)
	{
		if (ADEC_STARTED == adev->share_buf->adec_info.status)
		{
			adec_stop(adev, 0);
			adev->trick_mode = 0;
		}
	}	
	if(atomic_read(&adev->atmOpenCnt) == 0)
	{
		adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_CLOSE);
		adev->status = ADEC_CLOSED;
		if(2 != adev->aud_buf_param.ta_enabled)  // for no ta case
		{
			adev->aud_buf_param.ta_enabled = 0;
		}
	}
#else

	//tee case have a deamon on background not exit
	if(atomic_read(&adev->atmOpenCnt) == 1)
	{
	#ifdef CONFIG_MT_EXT_VVID_SUPPORT
		if (ADEC_STARTED == adev->status)
	#else
		if (ADEC_STARTED == adev->share_buf->adec_info.status)
	#endif
		{
			adec_stop(adev, 0);
		}
	}	

	if(atomic_read(&adev->atmOpenCnt) == 0)
	{
		adec_exec_cmd(adev, &adev->share_buf->adec_cmd, TA_ADEC_CLOSE);
		adev->status = ADEC_CLOSED;
		if(2 != adev->aud_buf_param.ta_enabled)  // for no ta case
		{
			adev->aud_buf_param.ta_enabled = 0;
		}
	}
#endif
	return 0;
}

static ssize_t adec_drv_read(struct file *filp, char __user *buffer, size_t count, loff_t *ppos)
{
	adec_dbg("adec read!\n");

	return 0;
}

static ssize_t adec_drv_write(struct file *filp, const char __user *buffer, size_t count, loff_t* ppos)
{
	int size = 0;
	return size;
}

static long adec_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long param)
{
	int ret = 0;
	adec_dev_t *adev = filp->private_data;

	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}

	switch (cmd)
	{
	case IO_ADEC_START:
		if((adev->status == ADEC_STARTED) || (adev->status == ADEC_PAUSED) )
		{
			adec_err("adec allready started adev->status = %d!\n", adev->status);
			return -1;
		}
		else
		{
			ret = adec_start(adev, param);
		}
		break;
	case IO_ADEC_STOP:
		ret = adec_stop(adev, param);
		break;
	case IO_ADEC_AD_ENABLE:
		ret = adec_ad_enable(adev, param);
		break;
	case IO_ADEC_GET_AUD_MEM:
		if (copy_to_user((void*)param, &adev->aud_buf_param, sizeof(aud_buf_param_t)))
		{
			adec_err("copy_to_user error!\n");
			ret = -1;
		}
		break;
	case IO_ADEC_TEE_ENABLE:
		adev->aud_buf_param.ta_enabled = 1;
		break;
	case IO_ADEC_TEE_DISABLE:
			adev->aud_buf_param.ta_enabled = 2;  //no ta
			break;
	case IO_ADEC_SET_DOWNMIX_ENABLE:
		ADEC_GET_SHARE_ADDR();
		adev->share_buf->downmix_enable = (u32)param;
		ret = config_downmix_mode();
		break;
	case IO_ADEC_PAUSE:
		ret = adec_pause(adev);
		break;
	case IO_ADEC_RESUME:
		ret = adec_resume(adev);
		break;
	case IO_ADEC_FLUSH:
		ret = adec_flush_es_buf(adev);
		break;
	case IO_ADEC_ENABLE_HEAAC:
		adec_enable_heaac(adev);
		break;
	case IO_ADEC_TRICK_MODE:
		adev->trick_mode = (u32)param;
		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}

static int adec_drv_mmap(struct file*filp, struct vm_area_struct *vma)
{
	adec_dev_t *adev = filp->private_data;
	ADEC_GET_SHARE_ADDR();
	if (remap_pfn_range(vma, vma->vm_start, virt_to_phys(adev->share_buf)>>PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot))
	{
		return -1;
	}
	
	//vma->vm_ops = &sln_remap_vm_ops;
	//sln_vma_open(vma);
	
	return 0;
}

static u32 calc_percent(u32 rd, u32 wt, u32 cnt)
{
	u64 percent = 0, n;
	
	if (rd <= wt)
	{
		n = wt - rd;
	}
	else
	{
		n = cnt - rd + wt;
	}

	if (cnt > 0)
	{
		percent = div_u64(10000 * n, cnt);
	}
	
	return percent;
}

static char *get_adec_status_string(u32 status)
{
	char *s;

	switch (status)
	{
	case ADEC_CLOSED:
		s = "closed";
		break;
	case ADEC_OPENED:
		s = "opened";
		break;
	case ADEC_ATTACHED:
		s = "attached";
		break;
	case ADEC_INITED:
		s = "inited";
		break;
	case ADEC_STARTED:
		s = "started";
		break;
	case ADEC_STOPED:
		s = "stoped";
		break;
	case ADEC_PAUSED:
		s = "paused";
		break;
	default:
		s = "error";
		break;
	}

	return s;
}

static char *get_adec_decode_state_string(u32 state)
{
	char *s;

	switch (state)
	{
	case ADEC_READ:
		s = "read";
		break;
	case ADEC_PARSE:
		s = "parse";
		break;
	case ADEC_DECODE:
		s = "decode";
		break;
	case ADEC_POST_PROCESS:
		s = "post_process";
		break;
	case ADEC_OUTPUT:
		s = "output";
		break;
	default:
		s = "error";
		break;
	}

	return s;
}

static mt_s32 adec_proc_read(struct seq_file* p, mt_void* v)
{
	adec_dev_t *adev = g_adev;
	u32 rd = 0, wt = 0, cnt = 0, percent = 0;
	u32 rd_ad = 0, wt_ad = 0, cnt_ad = 0, percent_ad = 0;
	ADEC_GET_SHARE_ADDR();

	seq_printf(p, "------------------------------------------ adec info -----------------------------------------\n");
	seq_printf(p, "WorkState\t\t: %s [%d]\n", get_adec_status_string(adev->adec_info->status), adev->adec_info->status);
	seq_printf(p, "decode state\t\t: %s [%d]\n", get_adec_decode_state_string(adev->adec_info->decode_state), adev->adec_info->decode_state);
	seq_printf(p, "decoder name\t\t: %s [0x%x]\n", adev->adec_info->decoder_name, adev->adec_info->aud_type);
	seq_printf(p, "parse ok/err\t\t: %d/%d\n", adev->adec_info->parse_ok, adev->adec_info->parse_err);
	seq_printf(p, "decoder ok/err\t\t: %d/%d\n", adev->adec_info->decode_ok, adev->adec_info->decode_err);
	seq_printf(p, "FrameIndex \t\t: %d\n", adev->adec_info->frm_idx);
	seq_printf(p, "sample rate\t\t: %d\n", adev->adec_info->sample_rate);
	seq_printf(p, "channel number\t\t: %d\n", adev->adec_info->ch_num);
	seq_printf(p, "sample number\t\t: %d\n", adev->adec_info->sample_num);
	
	seq_printf(p, "AD enable\t\t: %d\n", adev->adec_info->ad_enable);
	seq_printf(p, "AD volume\t\t: %d\n", adev->adec_info->ad_volume);
	seq_printf(p, "ADFrameIndex\t\t: %d\n", adev->adec_info->ad_frm_idx);
	seq_printf(p, "AD parse ok/err\t\t: %d/%d\n", adev->adec_info->ad_parse_ok, adev->adec_info->ad_parse_err);
	seq_printf(p, "AD decoder ok/err\t: %d/%d\n", adev->adec_info->ad_decode_ok, adev->adec_info->ad_decode_err);
	seq_printf(p, "DroppedFrames\t\t: %#x\n", 0);
	seq_printf(p, "DecErrCnt\t\t: %#x\n", adev->adec_info->decode_err);
	seq_printf(p, "log_level_flag\t\t: %#x\n", adev->share_buf->log_level_flag);

	if (NULL != adev->es_buf)
	{
		rd = adev->es_buf->pkt_rd;
		wt = adev->es_buf->pkt_wt;
		cnt = adev->es_buf->pkt_cnt;
	}

	if (NULL != adev->ad_es_buf)
	{
		rd_ad = adev->ad_es_buf->pkt_rd;
		wt_ad = adev->ad_es_buf->pkt_wt;
		cnt_ad = adev->ad_es_buf->pkt_cnt;
	}
	percent = calc_percent(rd, wt, cnt);
	percent_ad = calc_percent(rd_ad, wt_ad, cnt_ad);

	seq_printf(p, "+--------+--------+--------+--------+---------+\n");
	seq_printf(p, "| es pkt |   rd   |   wt   |  size  | percent |\n");
	seq_printf(p, "|--------+--------+--------+--------+---------|\n");
	seq_printf(p, "|  main  |%-8d|%-8d|%-8d|%5d.%02d%%|\n", rd, wt, cnt, percent / 100, percent % 100);
	seq_printf(p, "|   AD   |%-8d|%-8d|%-8d|%5d.%02d%%|\n", rd_ad, wt_ad, cnt_ad, percent_ad / 100, percent_ad % 100);
	seq_printf(p, "+--------+--------+--------+--------+---------+\n\n");

	if (NULL != adev->es_buf)
	{
		rd = adev->es_buf->es_rd;
		wt = adev->es_buf->es_wt;
		cnt = adev->es_buf->es_buf_size;
	}
	else
	{
		rd = wt = cnt = 0;
	}

	if (NULL != adev->ad_es_buf)
	{
		rd_ad = adev->ad_es_buf->es_rd;
		wt_ad = adev->ad_es_buf->es_wt;
		cnt_ad = adev->ad_es_buf->es_buf_size;
	}
	else
	{
		rd_ad = wt_ad = cnt_ad = 0;
	}

	percent = calc_percent(rd, wt, cnt);
	percent_ad = calc_percent(rd_ad, wt_ad, cnt_ad);

	seq_printf(p, "+--------+--------+--------+--------+---------+--------+---------+\n");
	seq_printf(p, "| es buf |   rd   |   wt   |  size  | percent |recv_cnt|recv_size|\n");
	seq_printf(p, "|--------+--------+--------+--------+---------+--------+---------|\n");
	seq_printf(p, "|  main  |%-8d|%-8d|%-8d|%5d.%02d%%|%-8d|%-9d|\n", rd, wt, cnt, percent / 100, percent % 100, adev->es_cnt, adev->es_size);
	seq_printf(p, "|   AD   |%-8d|%-8d|%-8d|%5d.%02d%%|%-8d|%-9d|\n", rd_ad, wt_ad, cnt_ad, percent_ad / 100, percent % 100, adev->ad_es_cnt, adev->ad_es_size);
	seq_printf(p, "+--------+--------+--------+--------+---------+--------+---------+\n");

	return MT_SUCCESS;
}

static int adec_proc_parse_param(char *buf, char param[][64], int max_num)
{
	int i = 0, j = 0;
	
	if ((NULL == buf) || (NULL == param))
	{
		adec_err("invalid parameter!\n");
		return -1;
	}

	while ((' ' == *buf) || ('\n' == *buf))
	{
		buf++;
	}

	if ('\0' == *buf)
	{
		return 0;
	}

	while ('\0' != *buf)
	{
		if ('\n' == *buf)
		{
			buf++;
		}
		
		if (' ' == *buf)
		{
			i++;
			j = 0;
			buf++;
			if (i >= max_num)
			{
				adec_err("param number large than %d\n", max_num);
				return -1;
			}
		}

		if (j < 64)
		{
			param[i][j++] = *buf;
		}
		buf++;
	}

	return (i + 1);
}

static void adec_proc_show_help(void)
{
	mt_drv_proc_echohelp("================ AUDIO DBG CMD INFO ================\n");
	mt_drv_proc_echohelp("log_level:\n");
	mt_drv_proc_echohelp("1:debug 2:info 4:error\n");
	mt_drv_proc_echohelp("eg. \"echo log_level 7 > /proc/msp/audio00\" will output log include debug/info/error!\n\n");

	mt_drv_proc_echohelp("save audio es:\n");
	mt_drv_proc_echohelp("echo save_es fielpath/filename.es > /proc/msp/audio00\n");
	mt_drv_proc_echohelp("echo start > /proc/msp/audio00\n");
	mt_drv_proc_echohelp("echo stop > /proc/msp/audio00\n\n");

	mt_drv_proc_echohelp("save audio pcm:\n");
	mt_drv_proc_echohelp("echo save_pcm filepath/filename.pcm > /proc/msp/audio00\n");
	mt_drv_proc_echohelp("echo start > /proc/msp/audio00\n");
	mt_drv_proc_echohelp("echo stop > /proc/msp/audio00\n");
}

static int write_data_to_file(void *handle, adec_dump_t *buf)
{
	int ret, size;
	u32 rd, wt;

	if ((NULL == handle) || (NULL == buf))
	{
		adec_err("invalid parameter, [%#lx/%#lx]\n", (ulong)handle, (ulong)buf);
		return -1;
	}

	rd = buf->rd;
	wt = buf->wt;
	//pr_err("rd:%d wt:%d\n", rd, wt);
	if (rd == wt)
	{
		return 0;
	}


	if (rd < wt)
	{
		size = wt - rd;
		ret = mt_drv_dump_do(handle, buf->data + rd, size);
		if (ret != size)
		{
			adec_err("write to file error, ret:%d size:%d\n", ret, size);
			return -1;
		}
		buf->rd = wt;
	}
	else
	{
		size = buf->size - rd;
		ret = mt_drv_dump_do(handle, buf->data + rd, size);
		if (ret != size)
		{
			adec_err("write to file error, ret:%d size:%d\n", ret, size);
			return -1;
		}
		buf->rd = 0;

		size = wt;
		ret = mt_drv_dump_do(handle, buf->data, size);
		if (ret != size)
		{
			adec_err("write to file error, ret:%d size:%d\n", ret, size);
			return -1;
		}
		buf->rd = wt;
	}

	buf->rd %= buf->size;

	return 0;
}

static int adec_dump_task(void *param)
{
	//u8 *data;
	//int ret;
	adec_dev_t *adev = (adec_dev_t *)param;
	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}
	
	ADEC_GET_SHARE_ADDR();
	while (1)
	{
		if (kthread_should_stop())
		{
			adec_err("exit adec dump task!\n");
			break;
		}

		if (adev->share_buf->pcm_dump.enable)
		{
			write_data_to_file(adev->pcm_dump_handle, &adev->share_buf->pcm_dump);
		}

		if (adev->share_buf->es_dump.enable)
		{
			write_data_to_file(adev->es_dump_handle, &adev->share_buf->es_dump);
		}

		//msleep_interruptible(20);
	}

	return 0;
}

static int adec_dump_data_enable(adec_dev_t *adev)
{
	if (NULL == adev)
	{
		adec_err("adev is NULL!\n");
		return -1;
	}
	ADEC_GET_SHARE_ADDR();

	if (strlen(adev->share_buf->pcm_dump.path))
	{
		adev->share_buf->pcm_dump.enable = 1;
		adev->pcm_dump_handle = mt_drv_dump_create(E_DUMP_TYPE_FILE, adev->share_buf->pcm_dump.path, 0);
		adec_err("adec start dump pcm data!\n");
	}

	if (strlen(adev->share_buf->es_dump.path))
	{
		adev->share_buf->es_dump.enable = 1;
		adev->es_dump_handle = mt_drv_dump_create(E_DUMP_TYPE_FILE, adev->share_buf->es_dump.path, 0);
		adec_err("adec start dump es data!\n");
	}

	if (adev->share_buf->pcm_dump.enable || adev->share_buf->es_dump.enable)
	{
		adev->dump_task_hanlde = kthread_create(adec_dump_task, adev, "adec_dump_task");
		if (IS_ERR(adev->dump_task_hanlde))
		{
			adec_err("create adec_task error!");
			adev->dump_task_hanlde = NULL;
			return -1;
		}
		
		wake_up_process(adev->dump_task_hanlde); 
	}

	return 0;
}

static int adec_dump_data_disable(adec_dev_t *adev)
{
	if (NULL == adev)
	{
		adec_err("adev is NULL!\n");
		return -1;
	}

	kthread_stop(adev->dump_task_hanlde);
	
	ADEC_GET_SHARE_ADDR();
	if(NULL != adev->pcm_dump_handle)
	{
		mt_drv_dump_destroy(adev->pcm_dump_handle);
		adev->pcm_dump_handle = NULL;
		memset(adev->share_buf->pcm_dump.path, 0, sizeof(adev->share_buf->pcm_dump.path));
		adec_err("adec stop dump pcm data!\n");
	}

	if(NULL != adev->es_dump_handle)
	{
		mt_drv_dump_destroy(adev->es_dump_handle);
		adev->es_dump_handle = NULL;
		memset(adev->share_buf->es_dump.path, 0, sizeof(adev->share_buf->es_dump.path));
		adec_err("adec stop dump es data!\n");
	}

	adev->share_buf->pcm_dump.enable = 0;
	adev->share_buf->es_dump.enable = 0;

	return 0;
}

static mt_s32 adec_proc_write(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
	int param_cnt;
	char tmp[512] = {0};
	char param[8][64] = {0};
	adec_dev_t *adev = g_adev;

	if (count > sizeof(tmp))
	{
	//	adec_err("proc write data need little than %ld\n", sizeof(tmp));
		return -1;
	}
	
	if (copy_from_user(tmp, buf, count))
	{
		adec_err("copy from user FAIL\n");
		return -1;
	}
	tmp[count] = '\0';
	ADEC_GET_SHARE_ADDR();

	param_cnt = adec_proc_parse_param(tmp, param, 8);

#if 0		// just for debug
	int i;
	adec_err("param_cnt:%d count0:%d\n", param_cnt, count);
	for (i = 0; i < 8; i++)
	{
		adec_err("param[%d]:%s\n", i, param[i]);
	}
#endif

	if (0 == param_cnt)
	{
		adec_proc_show_help();
		return count;
	}

	if (0 == strcmp(param[0], "log_level"))
	{
		char *after;
		adev->share_buf->log_level_flag = simple_strtoul(param[1], &after, 10);
		adec_err("log_level:%s %#x\n", param[1], g_adev->share_buf->log_level_flag);
	}
	else if (0 == strcmp(param[0], "save_pcm"))
	{
		memset(adev->share_buf->pcm_dump.path, 0, sizeof(adev->share_buf->pcm_dump.path));
		snprintf(adev->share_buf->pcm_dump.path, sizeof(adev->share_buf->pcm_dump.path), "%s", param[1]);
	}
	else if (0 == strcmp(param[0], "save_es"))
	{
		memset(adev->share_buf->es_dump.path, 0, sizeof(adev->share_buf->es_dump.path));
		snprintf(adev->share_buf->es_dump.path, sizeof(adev->share_buf->es_dump.path), "%s", param[1]);
	}
	else if (0 == strcmp(param[0], "start"))	// start dump data
	{
		adec_dump_data_enable(adev);
	}
	else if (0 == strcmp(param[0], "stop"))		// stop dump data
	{
		adec_dump_data_disable(adev);
	}
	else
	{
		adec_proc_show_help();
	}

	return count;
}

static adec_proc_param_t g_adec_proc_param = {
	.proc_read  = adec_proc_read,
	.proc_write = adec_proc_write,
};

static mt_s32 adec_proc_add(const mt_char * name, adec_proc_param_t * proc_param)
{
    mt_char entryName[16];
    mt_proc_entry_t*  pProcItem;

    if (MT_NULL == proc_param)
	{
		adec_err("PARAM pstParam is NULL\n");
        return MT_FAILURE;
    }

    /* Create proc */
    snprintf(entryName, sizeof(entryName), "%s", name);
    pProcItem = mt_drv_proc_add_module(entryName, MT_NULL, MT_NULL);
    if (!pProcItem){
        adec_err("Create %s proc entry FAIL\n", name);
        return MT_FAILURE;
    }

    /* Set functions */
    pProcItem->read  = proc_param->proc_read;
    pProcItem->write = proc_param->proc_write;

	adec_err("DRV adec proc[%s] add SUCCESS\n", name);
    return MT_SUCCESS;
}


static struct file_operations adec_fops =
{
	.owner =			THIS_MODULE,
	.open =				adec_drv_open,
	.read =				adec_drv_read,
	.write =			adec_drv_write,
	.unlocked_ioctl =	adec_drv_ioctl,
	.release =			adec_drv_release,
	.mmap = 			adec_drv_mmap,
};

#ifdef AV_CPU_ENABLE	
static int ipc_init(adec_dev_t *adev)
{
	int ret;
	mt_s32 s32Ret;
	void * func = NULL;
	int i = 0;
	
	if (NULL == adev)
	{
		adec_err("invalid parameter!\n");
		return -1;
	}

	for(i=0; i<50; i++)
	{
  		s32Ret = mt_drv_module_getfunction(MT_ID_AMPSHM, &func);
		if(	MT_SUCCESS == s32Ret)
		{
			printk(KERN_ERR " %s %d MT_ID_AMPSHM init done i=%d\n", __func__, __LINE__, i);
			break;
		}
		else
		{
			printk(KERN_ERR " %s %d MT_ID_AMPSHM init err i=%d\n", __func__, __LINE__, i);
		}
		msleep_interruptible(2);
	}
	adev->pipe_recv.msg_id_mask = 0xFFFFFFFF;
	adev->pipe_recv.p_sid = AV_SYS_DEV_AUD;
	adev->pipe_recv.p_did = AP_SYS_DEV_AUD;

	//create pipe for msg send/recv
	ret = ap_ipc_pipe_create(&adev->pipe_recv, IPC_MSG_AUD_FW_MSG_DEPTH);
	if (0 != ret)
	{
		adec_err("ap_ipc_pipe_create error!\n");
	}
	g_adev->ipc_inited  = 1;

	return ret;
}
#endif
#endif

int __init adec_drv_init(void)
{
	int ret = 0;
#ifndef CONFIG_MT_FPGA	
	/*static struct proc_dir_entry *ent;

	if (proc_mkdir("msp", NULL))
	{
		adec_err("proc_mkdir error!\n");
	}

	if (proc_create("msp/adec", 0660, NULL, &proc_ops) == NULL) {
		ret = -1;
		adec_err("create /proc/%s failed.\n", "msp/adec");
	} else {
		adec_err("create /proc/%s sucess.\n", "msp/adec");
	}*/

	adec_proc_add("audio00", &g_adec_proc_param);

	g_adev = kmalloc(sizeof(adec_dev_t), GFP_KERNEL);
	if (!g_adev)
	{
		adec_err("alloc mem_dev error!\n");
		return - ENOMEM;
	}
	memset(g_adev, 0, sizeof(adec_dev_t));
	g_adev->status = ADEC_CLOSED;

	ret = alloc_chrdev_region(&g_adev->devno, 0, 1, ADEC_DEVICE_NAME);
	if (ret)
	{
		adec_err("alloc adev failed.\n");
		kfree(g_adev);
		g_adev = NULL;
		return ret;
	}

	cdev_init(&g_adev->dev, &adec_fops);
	g_adev->dev.owner = THIS_MODULE;
	g_adev->dev.ops = &adec_fops;
	ret = cdev_add(&g_adev->dev, g_adev->devno, 1);
	if (ret)
	{
		unregister_chrdev_region(g_adev->devno, 1);
		kfree(g_adev);
		g_adev = NULL;
		return ret;
	}

	g_adev->devclass = class_create("adec_class");
	if (IS_ERR(g_adev->devclass))
	{
		adec_err("class_create failed.\n");
		cdev_del(&g_adev->dev);
		kfree(g_adev);
		g_adev = NULL;
		ret = -EIO;
		return ret;
	}

	device_create(g_adev->devclass, NULL, g_adev->devno, NULL, ADEC_DEVICE_NAME);

	alloc_aud_memory(g_adev);
	
#ifndef CONFIG_TEE
	g_adev->aud_buf_param.ta_enabled = 2;
#endif

	atomic_set(&g_adev->atmOpenCnt, 0);
	g_adev->ipc_inited = 0;
	adec_dbg("adec dev init!\n");
#endif
	return ret;
}

void __exit adec_drv_exit(void)
{
#ifndef CONFIG_MT_FPGA
	release_aud_memory(g_adev);
	
	device_destroy(g_adev->devclass, g_adev->devno);
	class_destroy(g_adev->devclass);
	cdev_del(&g_adev->dev);
	unregister_chrdev_region(g_adev->devno, 1);
	kfree(g_adev);
	
	adec_dbg("adec drv exit!\n");
#endif
}


