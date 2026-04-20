/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* drv_snd_ao.c <2022-10-24>																*/
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <uapi/linux/sched/types.h>
#include "mt_drv_mmz.h"
#include "mt_common.h"
#include "mt_drv_dev.h"
#include "mt_drv_struct.h"
#include "mt_drv_module.h"
#include "mt_drv_dma.h"
#include "drv_snd_reg.h"
#include "drv_snd_ao.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_dump.h"
#include "snd_fpga_test.h"
#include "drv_sync_ioctl.h"
#include "mt_cache.h"
#include "mt_kernel_adapt.h"
//#include "drv_sync.h"
#include "../../sync/drv_sync.h"
#include "mach/irqs.h"
#include "mt_unf_sound.h"
#include "mt_drv_analog.h"

/*****************************************************************************
[GLOBAL]
******************************************************************************/
MT_DECLARE_MUTEX(g_drv_snd_mutex);
/* the attribute struct of mt_snd device */
static mt_device_s gst_snd_dev;
static SND_PRIV_DATA gst_snd_priv_data;
static SND_CARD gst_snd_card[DRV_SND_CARD_NUM];
static struct task_struct *gpst_snd_task = NULL;
static struct task_struct *gtts_snd_task = NULL;
//static AUD_SND_DATA_PKT_QUE gst_snd_pkt_que;
static mt_u8 g_dolby_volume_ctrl = 0;
static atomic_t atm_open_cnt = ATOMIC_INIT(0);	/* Open times */
static mt_u32 g_hdmi_afmt = 0;
static mt_u32 g_hdmi_inf = 0;
static mt_u32 g_hdmi_chan = 0;
static mt_u32 g_hdmi_rate = 0;
static mt_u32 g_hdmi_depth = 0;
static mt_u32 g_hdmi_mat = 0;
static MT_BOOL g_aout_tts_stop = MT_FALSE;
static mt_u32 g_aout_tts_state = 0;

#define TTS_FRM_SAMPLE 1024
static __attribute__((aligned(32))) mt_u16 tts_buffer[TTS_FRM_SAMPLE];
//static __attribute__((aligned(32))) mt_u32 tts_outbuffer[TTS_FRM_SAMPLE];

extern ulong reg_aout_base_ioremap;
extern ulong reg_aout_clk_base_ioremap;
extern ulong reg_hdmi_audio_ctrl_ioremap;

mt_s32 use_adec_ta(aud_share_buf_t **share_buf);
mt_s32 get_sharebuf_av(aud_share_buf_t **share_buf);
mt_s32 get_sharebuf_ta(aud_share_buf_t **share_buf);

aud_share_buf_t * tmp_share_buf = NULL; 

#define SND_GET_SHARE_PKT()\
do{\
    if(use_adec_ta(&tmp_share_buf) < 0)\
    {\
       return -1;\
    }\
	gst_snd_card[0].share_buf = tmp_share_buf;\
	gst_snd_card[0].pPkt = &tmp_share_buf->snd_pkt;\
	gst_snd_card[0].swBuff[0] = &tmp_share_buf->snd_sw_buf[0];\
	gst_snd_card[0].swBuff[1] = &tmp_share_buf->snd_sw_buf[1];\
	gst_snd_card[0].swBuff[2] = &tmp_share_buf->snd_sw_buf[2];\
   }while(0)


/*****************************************************************************
[DEBUG]
******************************************************************************/
//mt_u32 g_drv_snd_log_level = LOG_LEVEL_FUNC_LINE | (LOG_LEVEL_ERR<<LOG_LEVEL_BIT) | (LOG_LEVEL_DBG<<LOG_LEVEL_BIT);
mt_u32 g_drv_snd_log_level = LOG_LEVEL_FUNC_LINE | (LOG_LEVEL_ERR << LOG_LEVEL_BIT);
mt_u32 g_snd_log_level = (0x1 << SND_LOG_POS_FUNC_LINE) | (0x1 << SND_LOG_POS_FLOW) | (0x1 << SND_LOG_POS_ERR);

/*****************************************************************************
[FUNCTION]
******************************************************************************/
static mt_s32 drv_snd_str2u32(char *str, mt_u32 *pdata)
{
	mt_s32 i = 0;
	mt_s32 d = 0;
	mt_u32 dat = 0;
	mt_u32 weight = 16;

	dat = 0;
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		i = 2;
		weight = 16;
	}
	else
	{
		i = 0;
		weight = 10;
	}

	for (; i < 10; i++)
	{
		if (str[i] < 0x20)
		{
			break;
		}
		else if (weight == 16 && str[i] >= 'a' && str[i] <= 'f')
		{
			d = str[i] - 'a' + 10;
		}
		else if (weight == 16 && str[i] >= 'A' && str[i] <= 'F')
		{
			d = str[i] - 'A' + 10;
		}
		else if (str[i] >= '0' && str[i] <= '9')
		{
			d = str[i] - '0';
		}
		else
		{
			return MT_FAILURE;
		}
		dat = dat * weight + d;
	}

	*pdata = dat;
	return MT_SUCCESS;
}

/*static void *drv_snd_dma_copy(void *dest, const void *src, size_t n)
{
	mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;
	unsigned int ch_id = DMA_CHANNEL_SECURE_AUDIO_1;
	void *from = (void *)virt_to_phys(src);
	void *to = (void *)virt_to_phys(dest);

	if(n == 0)
	{
		SND_LOG(SND_LOG_POS_TODO, "Param error: from %p to %p, size %ld\n",from,to,n);
		return NULL;
	}

	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.param.len = (mt_u32)n;
	dma_param.param.phy_src_addr = (phys_addr_t)from;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = (phys_addr_t)to;
	dma_param.param.vir_dst_addr = 0;

	//align 128
	if ((dma_param.param.phy_src_addr & (128-1)) == 0
		&& (dma_param.param.phy_dst_addr & (128-1)) == 0
		&& (dma_param.param.len & (128-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM16;
	}
	//align 64
	else if ((dma_param.param.phy_src_addr & (64-1)) == 0
		&& (dma_param.param.phy_dst_addr & (64-1)) == 0
		&& (dma_param.param.len & (64-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM8;
	}
	//align 32
	else if ((dma_param.param.phy_src_addr & (32-1)) == 0
		&& (dma_param.param.phy_dst_addr & (32-1)) == 0
		&& (dma_param.param.len & (32-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM4;
	}
	//align 16
	else if ((dma_param.param.phy_src_addr & (16-1)) == 0
		&& (dma_param.param.phy_dst_addr & (16-1)) == 0
		&& (dma_param.param.len & (16-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM2;
	}
	//align 8
	else if ((dma_param.param.phy_src_addr & (8-1)) == 0
		&& (dma_param.param.phy_dst_addr & (8-1)) == 0
		&& (dma_param.param.len & (8-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 4
	else if ((dma_param.param.phy_src_addr & (4-1)) == 0
		&& (dma_param.param.phy_dst_addr & (4-1)) == 0
		&& (dma_param.param.len & (4-1)) == 0)
	{
		usize = DMA_USIZE_32BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 2
	else if ((dma_param.param.phy_src_addr & (2-1)) == 0
		&& (dma_param.param.phy_dst_addr & (2-1)) == 0
		&& (dma_param.param.len & (2-1)) == 0)
	{
		usize = DMA_USIZE_16BIT;
		bnum = DMA_BURST_NUM1;
	}
	else
	{
		usize = DMA_USIZE_8BIT;
		bnum = DMA_BURST_NUM1;
	}

	dma_param.param.config.dst_peripheral = 0xf; // memory
	dma_param.param.config.src_peripheral = 0xf; // memory
	dma_param.param.config.dst_endian = 0; // big endian
	dma_param.param.config.src_endian = 0; // big endian
	dma_param.param.config.dst_clk = 0; // AXI clock
	dma_param.param.config.src_clk = 0; // AXI clock
	dma_param.param.config.dst_i = DMA_ADDR_INC;
	dma_param.param.config.src_i = DMA_ADDR_INC;
	dma_param.param.config.dst_usize = usize;
	dma_param.param.config.src_usize = usize;
	dma_param.param.config.dst_bsize = bnum;
	dma_param.param.config.src_bsize = bnum;
	dma_param.param.control.int_link_en = 1;
	dma_param.param.control.int_node_en = 1;
	dma_param.param.control.chn_param_reg_en = 0;

	dma_param.chn_id = ch_id;
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		SND_LOG(SND_LOG_POS_TODO, "dma get free channel failed!\n");
		return NULL;
	}

	ret = hal_dma_start(dma_param.chn_id, (mt_void *)&dma_param.param, NULL);
	if (ret != DMA_SUCCESS)
	{
		SND_LOG(SND_LOG_POS_TODO, "dma channel(%d) start failed!\n",dma_param.chn_id);
		return NULL;
	}

	while (DMA_STATUS_STOP != hal_dma_check(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//usleep_range(100, 100);
		usleep_range(100, 1000);
		tmo --;
		if (tmo <= 0)
		{
			SND_LOG(SND_LOG_POS_TODO, "dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = hal_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		SND_LOG(SND_LOG_POS_TODO, "dma channel(%d) stop failed!\n",dma_param.chn_id);
		return NULL;
	}

	return to;
}*/

static const int srcflt_coef[256] =
{
//table_2A
	0xffffff0a, 0xfffff548, 0xffffffb4, 0x0000029e, 0xfffffade, 0x000008ba,
	0xfffff26a, 0x000013e7, 0xffffe43c, 0x00002566, 0xffffcf0e,
	0x00003ea9, 0xffffb12d, 0x000061df, 0xffff878b, 0x000093ae, 0xffff4a8a,
	0x0000e17e, 0xfffee073, 0x00018315, 0xfffdb39f, 0x000513f2,
	0x000e18a1, 0xfffd74f2, 0x00013cf0, 0xffff4647, 0x0000731b, 0xffffb8cc,
	0x000029d2, 0xffffea92, 0x00000726, 0x000002bd, 0xfffff6ab,
	0x00000d6a, 0xfffff071, 0x00001040, 0xfffff02b, 0x00000eae, 0xfffff2fb,
	0x00000b30, 0xfffff67f, 0x000008aa, 0xfffff588, 0xfffffafa,
//table_2B
	0x00000562, 0x0000550b, 0xfffe58eb, 0x00058622, 0x000c9794, 0xffff5694,
	0xffffb935, 0x00002af1,
//table_3A
	0xffffff78, 0xfffff6cc, 0xfffffede, 0x000005fb, 0xfffff9d5, 0x00000c53,
	0xfffff0cf, 0x0000179c, 0xffffe2a8, 0x0000285d, 0xffffcec4,
	0x00003f42, 0xffffb44f, 0x00005d63, 0xffff91a8, 0x00008567, 0xffff6230,
	0x0000bfc8, 0xffff1548, 0x000130c3, 0xfffe4b1e, 0x0003680a,
	0x000ee8da, 0xfffe3617, 0x0000c598, 0xffff9d44, 0x0000317f, 0xffffee1c,
	0x00000025, 0x00000da8, 0xffffec21, 0x000019db, 0xffffe5bf,
	0x00001c27, 0xffffe6ad, 0x00001924, 0xffffeb60, 0x000013b2, 0xfffff171,
	0x00000de8, 0xfffff6ec, 0x00000a2a, 0xfffff7c9, 0xfffffa7b,
	0xfffffdb7, 0xfffff51a, 0x000006cb, 0xfffffd10, 0x00000489, 0xffffff32,
	0x0000007b, 0x0000055b, 0xfffff727, 0x00001219, 0xffffe5e7,
	0x0000286a, 0xffffc924, 0x00004cd2, 0xffff9ac6, 0x00008824, 0xffff4d00,
	0x0000f1f6, 0xfffeb2dd, 0x0001ee02, 0xfffcac9b, 0x000a2a83,
	0x000a2a83, 0xfffcac9b, 0x0001ee02, 0xfffeb2dd, 0x0000f1f6, 0xffff4d00,
	0x00008824, 0xffff9ac6, 0x00004cd2, 0xffffc924, 0x0000286a,
	0xffffe5e7, 0x00001219, 0xfffff727, 0x0000055b, 0x0000007b, 0xffffff32,
	0x00000489, 0xfffffd10, 0x000006cb, 0xfffff51a, 0xfffffdb7,
//table_3B
	0xffffff12, 0xffffec99, 0x00002a60, 0xffffcbb0, 0x000019a0, 0x00006ba9,
	0xfffe07d3, 0x000c3e47, 0x000721e5, 0xfffd5c3f,
	0x00013893, 0xffff8531, 0x00001fc2, 0x000006a1, 0xfffff3c7, 0xfffffb4a,
	0xffffefe7, 0x00003e56, 0xffff7a0b, 0x0000e902,
	0xfffeb6ec, 0x00019578, 0x000e5137, 0x00019578, 0xfffeb6ec, 0x0000e902,
	0xffff7a0b, 0x00003e56, 0xffffefe7, 0xfffffb4a,
//table_4A
	0x0000017e, 0x000050ce, 0xfffeae4e, 0x00037fef, 0x000d900b, 0x000052b8,
	0xffff6fce, 0x000035d6,
	0x00000855, 0x00004d4c, 0xfffe3116, 0x000785da, 0x000b48b4, 0xfffe92aa,
	0x00000759, 0x00001994,
//table_4B
	0xfffffb77, 0xffffb68c, 0x000a3582, 0x00069ff0, 0xffff8a6e,
	0xffffe685, 0x0000c89d, 0x000c82d3, 0x0003277b, 0xffffb86b,
//table_5A
	0x00000310, 0x00036ad3, 0x000abbe4, 0x0001d7a4,
	0x00001473, 0x0005792d, 0x00099e4f, 0x0000d590,
	0x00004d6d, 0x0007b34d, 0x0007b34d, 0x00004d6d,
	0xfffff68f, 0x00000000, 0x0000071b, 0x00000000, 0xfffff64a, 0x00000000,
//table_2D
	0x00000cea, 0x00000000, 0xffffef35, 0x00000000, 0x0000157a, 0x00000000,
	0xffffe4e5, 0x00000000, 0x000021e2, 0x00000000, 0xffffd5ef, 0x00000000,
	0x00003413, 0x00000000, 0xffffbf83, 0x00000000, 0x0000504d, 0x00000000,
	0xffff9ace, 0x00000000, 0x00008267, 0x00000000, 0xffff512e, 0x00000000,
	0x0000fc7e, 0x00000000, 0xfffe5267, 0x00000000, 0x00051620, 0x00080000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

static mt_u32 regWtCmd_table[AUD_SND_HW_BUFF_NUM] =
{
	REG_AUD_PP_BUF_WRCMD,	REG_AUD_OUT_BUF_WRCMD,
	REG_AUD_MIX_BUF_WR_CMD,	REG_AUD_SPD_BUF_WRCMD
};

static mt_u32 regBuffCnt_table[AUD_SND_HW_BUFF_NUM] =
{
	REG_AUD_PP_BUF0_CNT,	REG_AUD_OUT_BUF_CNT,
	REG_AUD_MIX_BUF_CNT,	REG_AUD_SPD_BUF_CNT
};

static mt_u8 drv_snd_samplerate2index(mt_u32 sample)
{
	mt_u8 index = 0;

	if (sample <= 8000)
	{
		index = 0xa;
	}
	else if (sample <= 11025)
	{
		index = 0x8;
	}
	else if (sample <= 12000)
	{
		index = 0x9;
	}
	else if (sample <= 16000)
	{
		index = 0x2;
	}
	else if (sample <= 22050)
	{
		index = 0x0;
	}
	else if (sample <= 24000)
	{
		index = 0x1;
	}
	else if (sample <= 32000)
	{
		index = 0x6;
	}
	else if (sample <= 44100)
	{
		index = 0x4;
	}
	else if (sample <= 48000)
	{
		index = 0x5;
	}
	else if (sample <= 64000)
	{
		index = 0xe;
	}
	else if (sample <= 88200)
	{
		index = 0xc;
	}
	else if (sample <= 96000)
	{
		index = 0xd;
	}
	else if (sample <= 128000)
	{
		index = 0xf;
	}
	else if (sample <= 176400)
	{
		index = 0x3;
	}
	else if (sample <= 192000)
	{
		index = 0x7;
	}
	else
	{
		index = 0x5;
	}

	return index;
}

static mt_u8 drv_snd_chan_mode(mt_u32 chan)
{
	mt_u8 mode = 0;
	mt_u8 idx = chan;

	if (8 < idx)
	{
		idx = 8;
	}

	while (idx)
	{
		mode = (mode << 1) | 1;
		idx--;
	}

	return mode;
}

extern void avsync_reset(void);
static int reset_hw_buf(void)
{
	mt_u32 tmp = 0;

	//audio_sync/mixbuf/spdbuf reset
	tmp = 10;
	drv_snd_reg_wt(REG_AUD_SYNC_RESET, 0x13);
	while (tmp--)
	{
		msleep_interruptible(1);
		if (0x13 == (0x13 & (drv_snd_reg_rd(REG_AUD_SYNC_RESET) >> 24)))
		{
			break;
		}
	}
	drv_snd_reg_wt(REG_AUD_SYNC_RESET, 0x0);
	avsync_reset();
	return 0;
}

extern void ao_all_skip_avsync_reset(void);
static int ao_all_skip_reset_hw_buf(void)
{
	mt_u32 tmp = 0;

	//audio_sync/mixbuf/spdbuf reset
	tmp = 10;
	drv_snd_reg_wt(REG_AUD_SYNC_RESET, 0x13);
	while (tmp--)
	{
		msleep_interruptible(1);
		if (0x13 == (0x13 & (drv_snd_reg_rd(REG_AUD_SYNC_RESET) >> 24)))
		{
			break;
		}
	}
	drv_snd_reg_wt(REG_AUD_SYNC_RESET, 0x0);
	ao_all_skip_avsync_reset();
	return 0;
}

#if 1
static int downmix_config(SND_CARD *snd_card, MT_UNF_TRACK_MODE_E track_mode)
{
	u32 left, right, bass, center, sl, sr, rsl, rsr;

	if (NULL == snd_card)
	{
		AO_LOG_ERR("snd_card is NULL!\n");
		return -1;
	}

	if (0 == snd_card->stCfg.channel)
	{
		AO_LOG_ERR("channel number is 0!!!\n");
		return -1;
	}

	if (1 == snd_card->stCfg.channel)
	{
		if(MT_UNF_TRACK_MODE_MUTED == track_mode)
		{
			left = 0;
			right = bass = center = sl = sr = rsl = rsr = 0;
		}
		else 
		{
			left = 0x7ff07ff0;
			right = bass = center = sl = sr = rsl = rsr = 0;
		}
	}
	else if (2 == snd_card->stCfg.channel)
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x3ff03ff0;
			right = 0x3ff03ff0;
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x7ff07ff0;
			right = 0;
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0;
			right = 0x7ff07ff0;
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x00007ff0;
			right = 0X7ff00000;
		}
		else if(MT_UNF_TRACK_MODE_EXCHANGE == track_mode)
		{
			left = 0x7ff00000;
			right = 0X00007ff0;
		}
		else if(MT_UNF_TRACK_MODE_ONLY_RIGHT == track_mode)
		{
			left = 0;
			right = 0x7ff00000;
		}
		else if(MT_UNF_TRACK_MODE_ONLY_LEFT == track_mode)
		{
			left = 0x00007ff0;
			right = 0;
		}
		else if(MT_UNF_TRACK_MODE_MUTED == track_mode)
		{
			left = 0;
			right = 0;
		}
		bass = center = sl = sr = rsl = rsr = 0;
	}
	else if ((3 <= snd_card->stCfg.channel) && (snd_card->stCfg.channel <= 6))
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x15501550; //l
			right = 0x15501550; //r
			bass = 0x15501550; //lfe
			center = 0x15501550; //c
			sl = 0x15501550; //sl
			sr = 0x15501550; //sr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x20002000; //l
			right = 0; //r
			bass = 0x20002000; //lfe
			center = 0x20002000; //c
			sl = 0x20002000; //sl
			sr = 0; //sr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0; //l
			right = 0x20002000; //r
			bass = 0x20002000; //lfe
			center = 0x20002000; //c
			sl = 0; //sl
			sr = 0x20002000; //sr
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x2000; //l
			right = 0x20000000; //r
			bass = 0x20002000; //lfe
			center = 0x20002000; //c
			sl = 0x2000; //sl
			sr = 0x20000000; //sr
		}
		else if(MT_UNF_TRACK_MODE_MUTED == track_mode)
		{
			left = 0; //l
			right = 0; //r
			bass = 0; //lfe
			center = 0; //c
			sl = 0; //sl
			sr = 0; //sr
		}
		rsl = rsr = 0;
	}
	else if (snd_card->stCfg.channel > 6)
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x10001000; //l
			right = 0x10001000; //r
			bass = 0x10001000; //lfe
			center = 0x10001000; //c
			sl = 0x10001000; //sl
			sr = 0x10001000; //sr
			rsl = 0x10001000; //rsl
			rsr = 0x10001000; //rsr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x20002000; //l
			right = 0; //r
			bass = 0x10001000; //lfe
			center = 0x10001000; //c
			sl = 0x20002000; //sl
			sr = 0; //sr
			rsl = 0x20002000; //rsl
			rsr = 0; //rsr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0; //l
			right = 0x20002000; //r
			bass = 0x10001000; //lfe
			center = 0x10001000; //c
			sl = 0; //sl
			sr = 0x20002000; //sr
			rsl = 0; //rsl
			rsr = 0x20002000; //rsr
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x2000; //l
			right = 0x20000000; //r
			bass = 0x10001000; //lfe
			center = 0x10001000; //c
			sl = 0x2000; //sl
			sr = 0x20000000; //sr
			rsl = 0; //rsl
			rsr = 0x20000000; //rsr
		}
		else if(MT_UNF_TRACK_MODE_MUTED == track_mode)
		{
			left = 0; //l
			right = 0; //r
			bass = 0; //lfe
			center = 0; //c
			sl = 0; //sl
			sr = 0; //sr
			rsl = 0; //rsl
			rsr = 0; //rsr
		}
	}

	snd_reg_60h_set_coef_left_all(left);
	snd_reg_64h_set_coef_right_all(right);
	snd_reg_68h_set_coef_bass_all(bass);
	snd_reg_6ch_set_coef_center_all(center);
	snd_reg_70h_set_coef_sl_all(sl);
	snd_reg_74h_set_coef_sr_all(sr);
	snd_reg_78h_set_coef_rsl_all(rsl);
	snd_reg_7ch_set_coef_rsr_all(rsr);

	return 0;
}
#else 
static int downmix_config(SND_CARD *snd_card, MT_UNF_TRACK_MODE_E track_mode)
{
	u32 left, right, bass, center, sl, sr, rsl, rsr;

	if (NULL == snd_card)
	{
		AO_LOG_ERR("snd_card is NULL!\n");
		return -1;
	}

	if (0 == snd_card->stCfg.channel)
	{
		AO_LOG_ERR("channel number is 0!!!\n");
		return -1;
	}

	if (1 == snd_card->stCfg.channel)
	{
		left = 0x7ff07ff0;
		right = bass = center = sl = sr = rsl = rsr = 0;
	}
	else if (2 == snd_card->stCfg.channel)
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x3ff03ff0;
			right = 0x3ff03ff0;
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x7ff07ff0;
			right = 0;
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0;
			right = 0x7ff07ff0;
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x00007ff0;
			right = 0X7ff00000;
		}
		else if(MT_UNF_TRACK_MODE_EXCHANGE == track_mode)
		{
			left = 0x7ff00000;
			right = 0X00007ff0;
		}
		else if(MT_UNF_TRACK_MODE_ONLY_RIGHT == track_mode)
		{
			left = 0;
			right = 0x7ff00000;
		}
		else if(MT_UNF_TRACK_MODE_ONLY_LEFT == track_mode)
		{
			left = 0x00007ff0;
			right = 0;
		}
		else if(MT_UNF_TRACK_MODE_MUTED == track_mode)
		{
			left = 0;
			right = 0;
		}
		bass = center = sl = sr = rsl = rsr = 0;
	}
	else if ((3 <= snd_card->stCfg.channel) && (snd_card->stCfg.channel <= 6))
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x5a825a82; //l
			right = 0x5a825a82; //r
			bass = 0; //lfe
			center = 0x7ff07ff0; //c
			sl = 0x40004000; //sl
			sr = 0x40004000; //sr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x7ff07ff0; //l
			right = 0; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0x5a825a82; //sl
			sr = 0; //sr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0; //l
			right = 0x7ff07ff0; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0; //sl
			sr = 0x5a825a82; //sr
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x7ff0; //l
			right = 0x7ff00000; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0x5a82; //sl
			sr = 0x5a820000; //sr
		}
		rsl = rsr = 0;
	}
	else if (snd_card->stCfg.channel > 6)
	{
		if(MT_UNF_TRACK_MODE_DOUBLE_MONO == track_mode)
		{
			left = 0x20002000; //l
			right = 0x20002000; //r
			bass = 0; //lfe
			center = 0x7ff07ff0; //c
			sl = 0x10001000; //sl
			sr = 0x10001000; //sr
			rsl = 0x10001000; //rsl
			rsr = 0x10001000; //rsr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_LEFT == track_mode)
		{
			left = 0x7ff07ff0; //l
			right = 0; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0x20002000; //sl
			sr = 0; //sr
			rsl = 0x20002000; //rsl
			rsr = 0; //rsr
		}
		else if(MT_UNF_TRACK_MODE_DOUBLE_RIGHT == track_mode)
		{
			left = 0; //l
			right = 0x7ff07ff0; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0; //sl
			sr = 0x20002000; //sr
			rsl = 0; //rsl
			rsr = 0x20002000; //rsr
		}
		else if(MT_UNF_TRACK_MODE_STEREO == track_mode)
		{
			left = 0x7ff0; //l
			right = 0x7ff00000; //r
			bass = 0; //lfe
			center = 0x5a825a82; //c
			sl = 0x2000; //sl
			sr = 0x20000000; //sr
			rsl = 0x2000; //rsl
			rsr = 0x20000000; //rsr
		}
	}

	snd_reg_60h_set_coef_left_all(left);
	snd_reg_64h_set_coef_right_all(right);
	snd_reg_68h_set_coef_bass_all(bass);
	snd_reg_6ch_set_coef_center_all(center);
	snd_reg_70h_set_coef_sl_all(sl);
	snd_reg_74h_set_coef_sr_all(sr);
	snd_reg_78h_set_coef_rsl_all(rsl);
	snd_reg_7ch_set_coef_rsr_all(rsr);

	return 0;
}

#endif
static mt_u32 update_src_sample_num(mt_u32 sample_num, mt_u32 sample_rate, mt_u8 src_mode)
{
	mt_u32 num = 0;

	if (src_mode)
	{
		/*old src mode */
		num = sample_num * 48000 / sample_rate;
	}
	else
	{
		switch(sample_rate)
		{
		case 8000:
		case 11025:
		case 12000:
			num = sample_num << 2;
			break;
		case 16000:
		case 22050:
		case 24000:
			num = sample_num << 1;
			break;
		default:
			num = sample_num;
			break;
		}
	}
	
	return num;
}

static mt_u32 update_src_sample_rate(mt_u32 sample_rate, mt_u8 src_mode)
{
	mt_u32 src_rate = 0;
	
	if(src_mode)
	{
		/*old src mode */
		src_rate = 48000;
	}
	else
	{
		switch(sample_rate)
		{
		case 8000:
		case 16000:
			src_rate = 32000;
			break;
		case 11025:
		case 22050:
			src_rate = 44100;
			break;
		case 12000:
		case 24000:
			src_rate = 48000;
			break;
		case 32000:
			src_rate = 64000;
			break;
		case 44100:
			src_rate = 88200;
			break;
		case 48000:
			src_rate = 96000;
			break;
		default:
			src_rate = 48000;
			break;
		}
	}
	
	return src_rate;
}

static void get_src_and_sample_cfg(AUD_SND_DATA_PKT *pCfg, mt_u32 *sample_num, mt_u32 *resample_en, mt_u32 *src_mode)
{
	if(!pCfg || !sample_num || !resample_en || !src_mode)
		return;
	
	if ((pCfg->sampleRate >= 32000) && (pCfg->sampleRate != 64000)){
		*sample_num = pCfg->frameSample;
		*resample_en = 0;
		*src_mode = 0;
	} else {
		*resample_en = 1;
		if (64000 == pCfg->sampleRate) {
			*src_mode = 1;
		}
		*sample_num = update_src_sample_num(pCfg->frameSample, pCfg->sampleRate, *src_mode);
	}
}

static u32 clca_aout_div(u32 sample_rate)
{
	u32 aout_clksel = 0, tmp = 0, n = 10;
	u64 div = 0;
	u64 div_tmp;
	u64 pll_no_div_freq = 2880000000; //must u64, 2880M
	u32 u32_pll_no_div_freq = (u32)pll_no_div_freq;

	aout_clksel = (drv_aout_clk_reg_rd(REG_AOUT_CLKSEL_REG) & 0x3);
	div = sample_rate;
	div <<= 33;	// sample_rate * 2^25 * 256

	if (0 == aout_clksel)	// 288M
	{
		n = 10;
	}
	else if (1 == aout_clksel)	// 262M
	{
		n = 11;
	}
	else if (2 == aout_clksel)	// 206M
	{
		n = 14;
	}
	else if (3 == aout_clksel)	// 131M
	{
		n = 22;
	}

	div *= n;
	div_tmp = div;
	do_div(div_tmp, u32_pll_no_div_freq);
	tmp = div_tmp;

	/* The value of tmp affects the output precision of AO, so next checking
	 * is import, tmp maybe not the best value, maybe tmp + 1.
	 */
	if ((div - pll_no_div_freq*tmp) > (pll_no_div_freq*(tmp + 1) - div))
	{
		tmp += 1;
	}

	return tmp;
}

mt_s32 config_downmix_mode(void)
{
	SND_CARD *snd_card = &gst_snd_card[0];

	if (NULL == snd_card->share_buf) {
		//snd is not opend now, will config it later.
		return 0;
	}

	if (snd_card->share_buf->downmix_enable &&
		(snd_card->share_buf->adec_info.aud_type != HA_AUDIO_ID_DTSHD &&
		(0 == g_hdmi_mat))) {
		snd_reg_0CH_set_downmix_en(1);
		snd_reg_0CH_set_dmx_hdmi_en(1);
	} else {
		snd_reg_0CH_set_downmix_en(0);
		snd_reg_0CH_set_dmx_hdmi_en(0);
	}
	
	snd_card->hdmi_need_cfg = 1;

	return 0;
}

EXPORT_SYMBOL(config_downmix_mode);

static mt_s32 drv_snd_aout_config(SND_CARD *snd_card, AUD_SND_DATA_PKT *pCfg)
{
	mt_u32 idx = 0;
	mt_u32 tmp = 0, src_mode = 0, samplenum = 0, resample_en = 0, sample_rate = 0;
	SND_BUFF *pBuff = NULL;
	mt_s32 ret = MT_SUCCESS;

	snd_card->aout_init = 1;
	if (AUD_SND_DATA_FMT_MAT == pCfg->stData[2].fmt)
		g_hdmi_mat = 1;
	else
		g_hdmi_mat = 0;
	
	//sample rate check
	if (snd_card->stCfg.sampleRate != pCfg->sampleRate ||
// for bug 25546	snd_card->stCfg.frameSample != pCfg->frameSample ||
		snd_card->stCfg.dolby_type != pCfg->dolby_type)
	{
		snd_card->aout_need_cfg = 1;
	}

	//pcm channel check
	if (snd_card->stCfg.channel != pCfg->channel)
	{
		snd_card->aout_need_cfg = 1;
	}

	if (snd_card->stream_info.aud_type != snd_card->share_buf->adec_info.aud_type)
	{
		snd_card->aout_need_cfg = 1;
		snd_card->stream_info.aud_type = snd_card->share_buf->adec_info.aud_type;
	}

	if(HA_AUDIO_ID_DOLBY_PLUS != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_DOLBY_TRUEHD != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_DOLBY_CONVERT != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_AC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_EAC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_DOLBY_AC4 != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_DTSHD != snd_card->share_buf->adec_info.aud_type &&
		HA_AUDIO_ID_DTSPASSTHROUGH != snd_card->share_buf->adec_info.aud_type) {
		if(MT_UNF_SND_HDMI_MODE_LPCM != snd_card->hdmi_mode ||
			MT_UNF_SND_SPDIF_MODE_LPCM != snd_card->spdif_mode) {
			SND_LOG(SND_LOG_POS_FLOW, "atype:0x%x is not support BS, force pcm!\n",snd_card->share_buf->adec_info.aud_type);
			snd_card->hdmi_mode = MT_UNF_SND_HDMI_MODE_LPCM;
			snd_card->spdif_mode = MT_UNF_SND_SPDIF_MODE_LPCM;
			snd_card->aout_need_cfg = 1;
		}
	} else {
		if(snd_card->hdmi_mode != snd_card->user_hdmi_mode ||
			snd_card->spdif_mode != snd_card->user_spdif_mode) {
			SND_LOG(SND_LOG_POS_FLOW, "atype:0x%x use user output mode!\n",snd_card->share_buf->adec_info.aud_type);
			snd_card->hdmi_mode = snd_card->user_hdmi_mode;
			snd_card->spdif_mode = snd_card->user_spdif_mode;
			snd_card->aout_need_cfg = 1;
		}
	}

	//buff data format check
	/*for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		if ((AUD_SND_DATA_FMT_NULL != pCfg->stData[idx].fmt) && (snd_card->stCfg.stData[idx].fmt != pCfg->stData[idx].fmt))
		{
			pr_err("<ah> fmt:%d/%d\n", snd_card->stCfg.stData[idx].fmt, pCfg->stData[idx].fmt);
			break;
		}
	}
	if (AUD_SND_SW_BUFF_NUM > idx)
	{
		snd_card->aout_need_cfg = 1;
	}*/

	if (0 == snd_card->aout_need_cfg)
	{
		if(snd_card->stCfg.frameSample != pCfg->frameSample) {
			get_src_and_sample_cfg(pCfg, &samplenum, &resample_en, &src_mode);
			drv_snd_reg_wt(REG_AUD_SAMP_NUM_FRM, samplenum);
			snd_card->stCfg.frameSample = pCfg->frameSample;
		}
		
		return 0;
	}

	snd_card->aout_need_cfg = 1;
	snd_card->hdmi_need_cfg = 1;
	SND_LOG(SND_LOG_POS_FLOW, "************AOUT-CONFIG************\n");

	memcpy((void *)&snd_card->stCfg, (void *)pCfg, sizeof(AUD_SND_DATA_PKT));

	//aout clock reset
	if (0 != reg_aout_clk_base_ioremap)
	{
		drv_aout_clk_reg_wt(REG_AOUT_SRSTN_REG, drv_aout_clk_reg_rd(REG_AOUT_SRSTN_REG) & (~(REG_AOUT_SRSTN_REG_MASK << REG_AOUT_SRSTN_REG_SHIFT)));
		msleep_interruptible(4);
		drv_aout_clk_reg_wt(REG_AOUT_SRSTN_REG, drv_aout_clk_reg_rd(REG_AOUT_SRSTN_REG) | (REG_AOUT_SRSTN_REG_MASK << REG_AOUT_SRSTN_REG_SHIFT));
		drv_aout_clk_reg_wt(REG_AOUT_CLKEN_REG, drv_aout_clk_reg_rd(REG_AOUT_CLKEN_REG) | (REG_AOUT_CLKEN_REG_MASK << REG_AOUT_CLKEN_REG_SHIFT));
	}

	//aout stop
	snd_reg_DCH_set_pcm_play_stop(1);
	//snd_reg_d0h_set_mix_func_en(0);

	reset_hw_buf();

	//src coef
	for (idx = 0; idx < 256; idx++)
	{
		//TODO... calculate the value of srcflt_coef to fix&final one
		tmp = srcflt_coef[idx] & 0x001fffff; //low 21bits
		tmp |= idx << 21;		//wr_coef_addr
		tmp |= 0x80000000;  	//wr_coef_en
		drv_snd_reg_wt(REG_AUD_SRC_CFG, tmp);
	}

	//hw buffer wt reset
	for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
	{
		pBuff = &snd_card->hwBuff[idx];
		pBuff->rd = 0;
		pBuff->wt = 0;
		pBuff->wtCnt = 0;
		pBuff->wtByte = 0;
		pBuff->regWtCmd = regWtCmd_table[idx];
		pBuff->regBuffCnt = regBuffCnt_table[idx];
	}

	//hw buffer config
	//pp buff
	snd_reg_28H_set_pp_buf_base(snd_card->hw_buf_mmz[0].startPhyAddr >> 3);
	snd_reg_2cH_set_pp_buf_len(snd_card->hwBuff[0].size >> 3);
	snd_reg_30H_set_pp_buf_full_thd(0);

	get_src_and_sample_cfg(pCfg, &samplenum, &resample_en, &src_mode);

	//always output 48k for eaa
	if(HA_AUDIO_ID_DTSHD != snd_card->share_buf->adec_info.aud_type && g_hdmi_mat == 0) {
		resample_en = 1;
		src_mode = 1;
		samplenum = update_src_sample_num(pCfg->frameSample, pCfg->sampleRate, 1);
		snd_card->buffer_thd = 10;
	} else {
		resample_en = 0;
		snd_card->buffer_thd = 3;
	}
	
	snd_reg_0CH_set_src_en(resample_en);
	snd_reg_0CH_set_old_src_flag(src_mode);
	
	ret = avsync_is_ddp_avsync_verfication();
	if ((MT_SUCCESS == ret) &&
        ((1536 == samplenum) ||
         (256 == samplenum) ||
         (1792 == samplenum) ||
		 (HA_AUDIO_ID_DOLBY_AC4 == snd_card->share_buf->adec_info.aud_type)))//a frame is divided into 4 frames for dolby test
	{
		avsync_set_aud_frame_sample_num(samplenum);
		drv_snd_reg_wt(REG_AUD_SAMP_NUM_FRM, samplenum>>2);
	}
	else
	{
		// set sample number
		drv_snd_reg_wt(REG_AUD_SAMP_NUM_FRM, samplenum);
	}

	/*if (pCfg->channel > 2)
	{
		snd_reg_0CH_set_chan_mod_pcmbuf(0);
	}
	else
	{
		snd_reg_0CH_set_chan_mod_pcmbuf(1);
	}*/

	if (pCfg->bitDepth > 16)
	{
		snd_reg_00H_set_pcm_32b_flag(1);
	}
	else
	{
		snd_reg_00H_set_pcm_32b_flag(0);
	}

	if (snd_card->share_buf->downmix_enable &&
		(snd_card->share_buf->adec_info.aud_type != HA_AUDIO_ID_DTSHD &&
		g_hdmi_mat == 0))
	{
		snd_reg_0CH_set_downmix_en(1);
		snd_reg_0CH_set_dmx_hdmi_en(1);
	}
	else
	{
		snd_reg_0CH_set_downmix_en(0);
		snd_reg_0CH_set_dmx_hdmi_en(0);
	}

	downmix_config(snd_card, snd_card->track_mode);

	//aout buff
	//snd_reg_18H_set_audio_out_buf_base_address(pBuff->base >> 3);
	snd_reg_18H_set_audio_out_buf_base_address(snd_card->hw_buf_mmz[1].startPhyAddr >> 3);
	snd_reg_1CH_set_audio_out_buf_length(snd_card->hwBuff[1].size >> 3);
	snd_reg_20H_set_audout_buf_full_thd(0);

	//mix buff
	snd_reg_c0H_set_mix_buf_base_address(snd_card->hw_buf_mmz[2].startPhyAddr >> 3);
	snd_reg_c4H_set_mix_buf_length(snd_card->hwBuff[2].size >> 3);
	snd_reg_c8H_set_mix_buf_full_thd(0);

	//spd buff
	snd_reg_38H_set_spd_buf_base_address(snd_card->hw_buf_mmz[3].startPhyAddr >> 3);
	snd_reg_3CH_set_spd_buf_length(snd_card->hwBuff[3].size >> 3);
	snd_reg_40H_set_spd_buf_full_thd(0);

	//clk div
	if (0 == resample_en)
	{
		sample_rate = pCfg->sampleRate;
	}
	else
	{
		sample_rate = update_src_sample_rate(pCfg->sampleRate, src_mode);
	}
	tmp = clca_aout_div(sample_rate);
	snd_reg_10H_set_clk_divider_factor(tmp);
	if ((AC3 == pCfg->dolby_type) || (AUD_SND_DATA_FMT_DTS == pCfg->stData[2].fmt))
	{
		snd_reg_88H_set_clk_divider_factor(tmp);
		tmp = drv_snd_samplerate2index(pCfg->sampleRate);
		snd_reg_00H_set_spdbuf_sample_rate(tmp);
	}
	else
	{
		snd_reg_88H_set_clk_divider_factor(tmp * 4);
		tmp = drv_snd_samplerate2index(pCfg->sampleRate*4);
		snd_reg_00H_set_spdbuf_sample_rate(tmp);
	}
	snd_reg_00H_set_spdbuf_sample_rate_sel(1);
	//ch-srt-configs
	tmp = drv_snd_samplerate2index(pCfg->sampleRate);
	snd_reg_00H_set_sample_rate(tmp);

	tmp = drv_snd_chan_mode(pCfg->channel);
	snd_reg_00H_set_ch_input_mode(tmp);

	snd_reg_04H_set_spdif_audout_buf(0);
/*
	//i2s-spdif-configs
	if (AUD_SND_DATA_FMT_PCM == pCfg->stData[0].fmt || 
		AUD_SND_DATA_FMT_DTS_HD == pCfg->stData[2].fmt ||
		AUD_SND_DATA_FMT_MAT == pCfg->stData[2].fmt)
	{
		snd_reg_04H_set_spdif_audout_buf(0);
	}
	else
	{
		snd_reg_04H_set_spdif_audout_buf(1);
	}
*/
	/*if (AUD_SND_DATA_FMT_NULL == pCfg->stData[1].fmt)
	{
		snd_reg_d0h_set_mix_func_en(0);
	}
	else*/
	{	// mix buf output dd data only
		//snd_reg_d4h_set_mix_hdmi_alpha(0);
		//snd_reg_d4h_set_mix_adac_alpha(0);
		//snd_reg_d0h_set_mix_func_en(1);
	}

	if (AUD_SND_DATA_FMT_PCM == pCfg->stData[2].fmt)
	{
		snd_reg_04H_set_spdif_spd_buf(0);
	}
	else
	{
		snd_reg_04H_set_spdif_spd_buf(1);
	}

	//Three outputs must enable register 20cH bit1
	if ((AUD_SND_DATA_FMT_NULL != pCfg->stData[1].fmt) && (AUD_SND_DATA_FMT_NULL != pCfg->stData[2].fmt))
	{
		snd_reg_20ch_set_pcm_buf_data_type(1);
	}
	else
	{
		snd_reg_20ch_set_pcm_buf_data_type(0);
	}

	if((MT_UNF_SND_SPDIF_MODE_RAW == snd_card->spdif_mode) && 
		(AUD_SND_DATA_FMT_DTS == pCfg->stData[2].fmt || AUD_SND_DATA_FMT_AC3 == pCfg->stData[2].fmt)) 
		snd_reg_04H_set_spdif_path_sel_spdif(1);	// spdif output from spd buf
	else
		snd_reg_04H_set_spdif_path_sel_spdif(0);	// spdif output from aout/mix buf

	if(AUD_SND_DATA_FMT_DTS == pCfg->stData[2].fmt)
		snd_reg_04H_set_spdif_path_sel_spdif(1);	//bypass spdif from spd buf always
		
	if ((MT_UNF_SND_SPDIF_MODE_RAW == snd_card->spdif_mode) && (AUD_SND_DATA_FMT_AC3 == pCfg->stData[1].fmt))
	{
		// mix buffer output dd data
		snd_reg_b0h_set_spdif_pcm_vol_ctrl(0);		// pcm_spdif mute
		snd_reg_b0h_set_mix_spdif_alpha(0x8000);	// mix_spdif unmute
	}
	else
	{
		snd_reg_b0h_set_spdif_pcm_vol_ctrl(0x8000);		// pcm_spdif unmute
		snd_reg_b0h_set_mix_spdif_alpha(0);				// mix_spdif mute
	}

	if (((MT_UNF_SND_HDMI_MODE_FORCE_DD == snd_card->hdmi_mode) && (AUD_SND_DATA_FMT_AC3 == pCfg->stData[1].fmt))/* ||
		((AUD_SND_OUTPUT_MODE_RAW == snd_card->hdmi_mode) && (AC3 == pCfg->dolby_type))*/)
	{
		// mix buffer output dd data
		snd_reg_b0h_set_spdif_pcm_vol_ctrl(0);		// pcm_spdif mute
		snd_reg_b0h_set_mix_spdif_alpha(0x8000);	// mix_spdif unmute

		// hdmi output DD data from mix buf
		snd_reg_04H_set_spdif_audout_buf(1);
		snd_reg_04H_set_spdif_mixbuf(1);

		snd_reg_04H_set_hdmi_manu_mute(1);
		snd_reg_04H_set_adac_manu_mute(1);

		snd_reg_0CH_set_dmx_manu_close(1);
		snd_reg_0CH_set_chmode_manu_2ch(1);

		snd_reg_a8h_set_hdmi_vol_ctrl(0);
		snd_reg_d4h_set_mix_hdmi_alpha(0x8000);
		snd_reg_04H_set_spdif_path_sel_hdmi(0);
	}
	else if (((MT_UNF_SND_HDMI_MODE_RAW == snd_card->hdmi_mode) && (AUD_SND_DATA_FMT_AC3 == pCfg->stData[2].fmt || AUD_SND_DATA_FMT_EAC3 == pCfg->stData[2].fmt)) ||
			(AUD_SND_DATA_FMT_DTS == pCfg->stData[2].fmt))
	{
		// hdmi output DD+/DTS data from spdif buf
		snd_reg_04H_set_spdif_path_sel_hdmi(1);
	}
	else
	{
		snd_reg_04H_set_spdif_path_sel_hdmi(0);
		snd_reg_04H_set_justified_mode(2);		//for pcm to HDMI unmute
		snd_reg_a8h_set_hdmi_vol_ctrl(0x8000);
	}

	drv_snd_reg_wt(0x8, 0);

	if (((MT_UNF_SND_HDMI_MODE_FORCE_DD == snd_card->hdmi_mode) && (AUD_SND_DATA_FMT_AC3 == pCfg->stData[1].fmt)))
		snd_reg_08H_set_gainq_manu_zero(1);

	if (snd_card->adac_mute)
	{
		set_volume(snd_card, 0);
	}
	else
	{
		set_volume(snd_card, snd_card->volume);
	}
	snd_reg_08H_set_spdif_hdmi_mute(snd_card->hdmi_mute);
	snd_reg_08H_set_spdif_coax_mute(snd_card->spdif_mute);

	//for tts
	snd_reg_a8h_set_hdmi_vol_ctrl(0x8000);
	snd_reg_a8h_set_adac_vol_ctrl(0x8000);
	snd_reg_b0h_set_spdif_pcm_vol_ctrl(0x8000);
	snd_reg_b0h_set_mix_spdif_alpha(0x8000);
	snd_reg_d4h_set_mix_hdmi_alpha(0x8000);
	snd_reg_d4h_set_mix_adac_alpha(0x8000);

	//hw buffer config
	if(HA_AUDIO_ID_DTSHD == snd_card->share_buf->adec_info.aud_type ||
		g_hdmi_mat == 1) {
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_0].used = 0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_0].swBuffIdx = 0xff;//AUD_SND_SW_BUFF_IDX_0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_1].used = 1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_1].swBuffIdx = AUD_SND_SW_BUFF_IDX_2;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_2].used = 0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_2].swBuffIdx = 0xff;//AUD_SND_SW_BUFF_IDX_1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_3].used = 0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_3].swBuffIdx = 0xff;//AUD_SND_SW_BUFF_IDX_2;
	} else {
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_0].used = 1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_0].swBuffIdx = AUD_SND_SW_BUFF_IDX_0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_1].used = 0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_1].swBuffIdx = 0xff;//AUD_SND_SW_BUFF_IDX_0;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_2].used = 1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_2].swBuffIdx = AUD_SND_SW_BUFF_IDX_1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_3].used = 1;
		snd_card->hwBuff[AUD_SND_HW_BUFF_IDX_3].swBuffIdx = AUD_SND_SW_BUFF_IDX_2;
	}

	tmp = 10;
	drv_snd_reg_wt(0x80, 0x13);
	while (tmp--)
	{
		msleep_interruptible(1);
		if (0x13 == (0x13 & (drv_snd_reg_rd(0x80) >> 24)))
		{
			break;
		}
	}
	drv_snd_reg_wt(0x80, 0x0);

	// enable snd_isr
	drv_snd_reg_wt(REG_AUD_INTR_SET, 0x211e00);		// enable snd_isr

	snd_card->play_cnt = 0;
	snd_card->aout_need_cfg = 0;
	SND_LOG(SND_LOG_POS_FLOW, "audio config aout SUCCESS \n");

	return MT_SUCCESS;
}

static mt_s32 drv_snd_hdmi_config(SND_CARD *snd_card, AUD_SND_DATA_PKT *pCfg)
{
	mt_s32 ret = MT_SUCCESS;
	HDMI_AUDIO_ATTR_S stHDMIAttr;
	AUD_SND_DATA_FMT fmt;

	if (0 == snd_card->hdmi_need_cfg)
	{
		return 0;
	}

	//get hdmi function pointer
	if (NULL == snd_card->pstHdmiFunc)
	{
		if (MT_SUCCESS != mt_drv_module_getfunction(MT_ID_HDMI, (mt_void**)&snd_card->pstHdmiFunc))
		{
			SND_LOG(SND_LOG_POS_ERR, "get hdmi func pointer FAIL\n");
			return MT_FAILURE;
		}
	}

	//audio to hdmi config
	if ((NULL == snd_card->pstHdmiFunc) || (NULL == snd_card->pstHdmiFunc->pfnHdmiAudioChange))
	{
		SND_LOG(SND_LOG_POS_ERR, "hdmi call back is null \n");
		return MT_FAILURE;
	}
	
	memset(&stHDMIAttr, 0, sizeof(HDMI_AUDIO_ATTR_S));
	if (snd_card->pstHdmiFunc && snd_card->pstHdmiFunc->pfnHdmiGetAoAttr)
	{
		(snd_card->pstHdmiFunc->pfnHdmiGetAoAttr)(MT_UNF_HDMI_ID_0, &stHDMIAttr);
	}

	stHDMIAttr.enBitDepth = snd_card->stCfg.bitDepth;
	stHDMIAttr.u32Channels = snd_card->stCfg.channel;

	if (MT_UNF_SND_HDMI_MODE_LPCM == snd_card->hdmi_mode)
	{
		fmt = snd_card->stCfg.stData[0].fmt;
		if(snd_card->share_buf->downmix_enable)
			stHDMIAttr.u32Channels = 2;
	}
	else if (MT_UNF_SND_HDMI_MODE_RAW == snd_card->hdmi_mode)
	{
		fmt = snd_card->stCfg.stData[2].fmt;
		stHDMIAttr.u32Channels = 1;
	}
	else if (MT_UNF_SND_HDMI_MODE_FORCE_DD == snd_card->hdmi_mode)
	{
		fmt = snd_card->stCfg.stData[1].fmt;
		stHDMIAttr.u32Channels = 1;
	}

	if(AUD_SND_DATA_FMT_DTS == pCfg->stData[2].fmt ||
		AUD_SND_DATA_FMT_DTS_HD == pCfg->stData[2].fmt) {//bypass dts always
		fmt = snd_card->stCfg.stData[2].fmt;
	}

	if(g_hdmi_mat == 1)
		fmt = snd_card->stCfg.stData[2].fmt;
	
	//data format
	switch (fmt)
	{
	case AUD_SND_DATA_FMT_PCM:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
		break;
	case AUD_SND_DATA_FMT_AC3:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_AC3;
		break;
	case AUD_SND_DATA_FMT_EAC3:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP;
		break;
	case AUD_SND_DATA_FMT_DTS:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS;
		stHDMIAttr.u32Channels = 1;
		break;
	case AUD_SND_DATA_FMT_DTS_HD:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD;
		stHDMIAttr.u32Channels = 8;
		break;
	case AUD_SND_DATA_FMT_MAT:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT;
		stHDMIAttr.u32Channels = 8;
		break;

	default:
		stHDMIAttr.enAudioCode = MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM;
		stHDMIAttr.u32Channels = snd_card->stCfg.channel;
		if(snd_card->share_buf->downmix_enable)
			stHDMIAttr.u32Channels = 2;
		break;
	}

	//source interface
	if (MT_UNF_EDID_AUDIO_FORMAT_CODE_PCM == stHDMIAttr.enAudioCode) {
		stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_I2S;
	} else if (MT_UNF_EDID_AUDIO_FORMAT_CODE_DTS_HD == stHDMIAttr.enAudioCode) {
		stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_HBR;
	} else if (MT_UNF_EDID_AUDIO_FORMAT_CODE_MAT == stHDMIAttr.enAudioCode) {
		stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_HBR;
	} else {
		stHDMIAttr.enSoundIntf = HDMI_AUDIO_INTERFACE_SPDIF;
	}
	
	//sample rate
	if (MT_UNF_EDID_AUDIO_FORMAT_CODE_DDP == stHDMIAttr.enAudioCode)
	{
		stHDMIAttr.enSampleRate = 4 * snd_card->stCfg.sampleRate;
	}
	else 
	{
		if(snd_reg_0CH_get_src_en())
			stHDMIAttr.enSampleRate = update_src_sample_rate(snd_card->stCfg.sampleRate, snd_reg_0CH_get_old_src_flag());
		else 
			stHDMIAttr.enSampleRate = snd_card->stCfg.sampleRate;
	}
	
	//others
	if (2 < stHDMIAttr.u32Channels)
	{
		stHDMIAttr.bIsMultiChannel = MT_TRUE;
	}
	else
	{
		stHDMIAttr.bIsMultiChannel = MT_FALSE;
	}
	stHDMIAttr.u8DownSampleParm = 0;
	stHDMIAttr.u8I2SCtlVbit = 0;

	g_hdmi_afmt = stHDMIAttr.enAudioCode;
	g_hdmi_inf = stHDMIAttr.enSoundIntf;
	g_hdmi_rate = stHDMIAttr.enSampleRate;
	g_hdmi_chan = stHDMIAttr.u32Channels;
	g_hdmi_depth = stHDMIAttr.enBitDepth;
	SND_LOG(SND_LOG_POS_FLOW, "CFG: fmt<%x>, intf<%d>, samplerate<%d>, channel<%d>, bitdepth<%d>\n",
			stHDMIAttr.enAudioCode, stHDMIAttr.enSoundIntf, stHDMIAttr.enSampleRate, stHDMIAttr.u32Channels, stHDMIAttr.enBitDepth);
	//config implement
	if (MT_SUCCESS == (snd_card->pstHdmiFunc->pfnHdmiAudioChange)(MT_UNF_HDMI_ID_0, &stHDMIAttr))
	{
		ret = MT_SUCCESS;
		snd_card->hdmi_need_cfg = 0;
	}
	else
	{
		ret = MT_FAILURE;
		//TODO...max reconfig check
	}

	SND_LOG(SND_LOG_POS_FLOW, "audio config hdmi %s\n", MT_SUCCESS == ret ? "SUCCESS" : "FAIL");
	return ret;
}

u32 avsync_get_ao_data_time(void)
{
	SND_CARD *snd_card = &gst_snd_card[0];

	u32 num, ao_data_size, ao_data_time = 0, ch_in_aout, pp_num, pp_data_size = 0;
	u32 ao_resample = snd_reg_0CH_get_src_en();
	u32 ao_src_mode = snd_reg_0CH_get_old_src_flag();
	u32 ao_sample_rate = 0;

	pp_data_size = drv_snd_reg_rd(REG_AUD_PP_BUF0_CNT) << 3;
	if (snd_reg_00H_get_pcm_32b_flag())
	{
		pp_num = pp_data_size / 4;
	}
	else
	{
		pp_num = pp_data_size / 2;
	}

	if ((0 != snd_card->stCfg.sampleRate) && (0 != snd_card->stCfg.channel))
	{
		pp_num = pp_num/snd_card->stCfg.channel;
		ao_data_time = 45 * (1000 * pp_num / snd_card->stCfg.sampleRate);
	}

	if (snd_reg_0CH_get_chan_mod_pcmbuf())
	{
		ch_in_aout = 2;
	}
	else
	{
		ch_in_aout = 8;
	}

	ao_data_size = drv_snd_reg_rd(REG_AUD_OUT_BUF_CNT) << 3;
	num = ao_data_size / ch_in_aout / 2;
	if(0 == ao_resample) {
		if ((0 != snd_card->stCfg.sampleRate) && (0 != snd_card->stCfg.channel))
		{
			ao_data_time += 45 * (1000 * num / snd_card->stCfg.sampleRate);
		}
	} else {
		ao_sample_rate = update_src_sample_rate(snd_card->stCfg.sampleRate, ao_src_mode);
		ao_data_time += 45 * (1000 * num / ao_sample_rate);
	}

	return ao_data_time;
}
EXPORT_SYMBOL(avsync_get_ao_data_time);

extern mt_void __weak avsync_audio_loop(struct avsync_async_info_t *pdata, AVSYNC_FRAME_SYNCFLAG_E *psync_flg);
extern int __weak get_adec_es_buf_info(int *data_size, int *buf_size);
static AVSYNC_FRAME_SYNCFLAG_E drv_snd_avsync_check(SND_BUFF *sw_buf, AUD_SND_DATA_PKT *data_pkt, mt_u32 *pAO_data_time)
{
	u32 num, data_size;
	struct avsync_async_info_t async_info = {0};
	AVSYNC_FRAME_SYNCFLAG_E sync_flg;
	SND_CARD *snd_card = &gst_snd_card[0];

	async_info.dolby_type = data_pkt->dolby_type;	
	async_info.apts = data_pkt->apts;
	async_info.apts_id = data_pkt->frmIdx;
	if (0 != data_pkt->sampleRate)
	{
		async_info.apts_step = 45000 * data_pkt->frameSample / data_pkt->sampleRate;
	}

	get_adec_es_buf_info(&async_info.inbuf_es_size, &async_info.inbuf_size);

	async_info.ao_data_size = drv_snd_reg_rd(REG_AUD_OUT_BUF_CNT) << 3;
	async_info.ao_data_time = avsync_get_ao_data_time();
	
	if (sw_buf->wt >= sw_buf->rd)
	{
		data_size = sw_buf->wt - sw_buf->rd;
	}
	else
	{
		data_size = sw_buf->size - sw_buf->rd + sw_buf->wt;
	}
	if ((0 != data_pkt->sampleRate) && (0 != data_pkt->channel))
	{
		num = data_size / data_pkt->channel / 2;
		async_info.pcm_temp_time = 45000 * num / data_pkt->sampleRate;
	}

	//----------------check the audio is bypass or pcm, 1:bypass, 0:pcm----------------
	async_info.audio_bypass = 0;
	
	/* HA_AUDIO_ID_DTSHD/HA_AUDIO_ID_DTSPASSTHROUGH: dts always use bypass mode
	 * HA_AUDIO_ID_AC3PASSTHROUGH/HA_AUDIO_ID_EAC3PASSTHROUGH: dolby select bypass mode*/	 
	if (HA_AUDIO_ID_DTSHD == snd_card->share_buf->adec_info.aud_type ||
	    HA_AUDIO_ID_DTSPASSTHROUGH == snd_card->share_buf->adec_info.aud_type ||
	    HA_AUDIO_ID_AC3PASSTHROUGH == snd_card->share_buf->adec_info.aud_type ||
	    HA_AUDIO_ID_EAC3PASSTHROUGH == snd_card->share_buf->adec_info.aud_type) 
	{
		async_info.audio_bypass = 1;
	} 
	else if (HA_AUDIO_ID_DOLBY_PLUS == snd_card->share_buf->adec_info.aud_type ||
			 HA_AUDIO_ID_DOLBY_TRUEHD == snd_card->share_buf->adec_info.aud_type ||
			 HA_AUDIO_ID_DOLBY_CONVERT == snd_card->share_buf->adec_info.aud_type) 
	{
		// if dolby not select bypass mode, but there is no dolby lib, set to bypass
		if (AUD_SND_DATA_FMT_NULL == data_pkt->stData[0].fmt)
		{
			async_info.audio_bypass = 1;
		}
	}			
	//--------------------------------------------------------------------------------
				
	avsync_audio_loop(&async_info, &sync_flg);
	*pAO_data_time = async_info.ao_data_time;

	return sync_flg;
}

static int drv_snd_drop_data(SND_CARD *snd_card, AUD_SND_DATA_PKT *pdata_pkt)
{
	int i;
	SND_BUFF *pSwBuff = NULL;
	AUD_SND_BUFF_ATTR *pPktBuff = NULL;

	if ((NULL == snd_card) || (NULL == pdata_pkt))
	{
		AO_LOG_ERR("invalid parameter [%p/%p]\n", snd_card, pdata_pkt);
		return MT_FAILURE;
	}

	for (i = 0; i < AUD_SND_SW_BUFF_NUM; i++)
	{
		pPktBuff = &pdata_pkt->stData[i];
		pSwBuff = snd_card->swBuff[i];
		if(pPktBuff->len)
			pSwBuff->rd = ((ulong)pPktBuff->addr + pPktBuff->len)%pSwBuff->size;
	}

	return 0;
}

static mt_s32 drv_snd_data_pkt_pop(SND_CARD *snd_card, AUD_SND_DATA_PKT *pdata_pkt)
{
	mt_u32 tmp = 0;
	AUD_SND_DATA_PKT_QUE *pPkt = NULL;

	if ((NULL == snd_card) || (NULL == pdata_pkt))
	{
		AO_LOG_ERR("invalid parameter [%p/%p]\n", snd_card, pdata_pkt);
		return MT_FAILURE;
	}

	pPkt = snd_card->pPkt;
	if (NULL == pPkt)
	{
		AO_LOG_ERR("pPkt is NULL!\n");
		return MT_FAILURE;
	}

	//data package queue node check
	if (pPkt->wt >= pPkt->rd)
	{
		tmp = pPkt->wt - pPkt->rd;
	}
	else
	{
		tmp = pPkt->num - pPkt->rd + pPkt->wt;
	}
	if (!tmp)
	{
		//AO_LOG_DBG("snd data package queue is empty [%d/%d/%d]\n", pPkt->rd, pPkt->wt, pPkt->num);
		return MT_FAILURE;
	}
	mt_dcache_invalid(pPkt, sizeof(AUD_SND_DATA_PKT_QUE));

	//pkt info copy
	memcpy((void *)pdata_pkt, (void *)&pPkt->stQue[pPkt->rd], sizeof(AUD_SND_DATA_PKT));

	if (AUD_SND_DATA_FMT_PCM == pdata_pkt->stData[0].fmt)
	{
		//pr_err("%d/%d/%d\n", (NULL != snd_card->crc_buf_vir), (pdata_pkt->frmIdx >= 1), (pdata_pkt->frmIdx <= (snd_card->crc_buf_size >> 2)));
		if ((NULL != snd_card->crc_buf_vir) && (pdata_pkt->frmIdx >= 1) && (pdata_pkt->frmIdx <= (snd_card->crc_buf_size >> 2)))
		{
			if (1 == pdata_pkt->frmIdx)
			{
				memset(snd_card->crc_buf_vir, 0, snd_card->crc_buf_size);
			}
			snd_card->crc_buf_vir[pdata_pkt->frmIdx - 1] = pdata_pkt->stData[0].crc;
			//pr_err("frm_id:%d crc:%#x\n", pdata_pkt->frmIdx, pdata_pkt->stData[0].crc);
		}
	}

	snd_card->stream_info.dolby_dd_ddp = pdata_pkt->dolby_type;
	snd_card->stream_info.dolby_dualmono = pdata_pkt->dualmono;
	snd_card->stream_info.dolby_acmod = pdata_pkt->acmod;
	snd_card->stream_info.dolby_bsid = pdata_pkt->bsid;
	snd_card->stream_info.dolby_lfeon = pdata_pkt->lfeon;
	snd_card->stream_info.origin_m_channum = pdata_pkt->channelOriginal;
	snd_card->stream_info.hdmi_mode = snd_card->hdmi_mode; 
	snd_card->aud_ci_test_info.AMute = (snd_card->hdmi_mute << 2) | (snd_card->spdif_mute << 1) | snd_card->adac_mute;
	snd_card->aud_ci_test_info.AVolume = snd_card->volume;
	snd_card->aud_ci_test_info.ADecFrmCnt = pdata_pkt->frmIdx;
	snd_card->aud_ci_test_info.AErrFrmCnt = snd_card->share_buf->adec_info.decode_err;
	snd_card->aud_ci_test_info.ADropFrmCnt = snd_card->drop_cnt;
	snd_card->aud_ci_test_info.ADescriptionFrmCnt = snd_card->share_buf->adec_info.ad_frm_idx;

	//pkt queue rd update
	if (1) //MT_SUCCESS == ret){
	{
		pPkt->rd = (pPkt->rd + 1) % pPkt->num;
		AO_LOG_DBG("sw buff pop SUCCESS, rd/wt/num[%d/%d/%d/0x%llx]\n", pPkt->rd, pPkt->wt, pPkt->num, pPkt->stQue[0].stData[0].addr);
	}
	else
	{
		AO_LOG_DBG("sw buff pop FAIL, rd/wt/num[%d/%d/%d/0x%llx]\n", pPkt->rd, pPkt->wt, pPkt->num, pPkt->stQue[0].stData[0].addr);
	}

	return MT_SUCCESS;
}

static mt_s32 drv_snd_data_pkt_push(SND_CARD *snd_card, AUD_SND_DATA_PKT *pdata_pkt)
{
	mt_u32 idx = 0;
	mt_u32 len = 0;
	mt_s32 ret[AUD_SND_SW_BUFF_NUM];
	SND_BUFF *pBuff = NULL;
	AUD_SND_BUFF_ATTR *pAttr = NULL;
	AUD_SND_DATA_PKT_QUE *pPkt = snd_card->pPkt;

	//ret flag init
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		ret[idx] = MT_SUCCESS;
	}

	//pkt info copy
	memcpy((void *)&pPkt->stQue[pPkt->wt], (void *)pdata_pkt, sizeof(AUD_SND_DATA_PKT));

	//data copy
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		if (pdata_pkt->stData[idx].fmt && (AUD_SND_DATA_FMT_NULL != pdata_pkt->stData[idx].fmt))
		{
			pBuff = snd_card->swBuff[idx];
			pAttr = &pPkt->stQue[pPkt->wt].stData[idx];
			len = pdata_pkt->stData[idx].len;
			ret[idx] = MT_FAILURE;
			if (pBuff->rd > pBuff->wt)
			{
				if (pBuff->wt + len < pBuff->rd)
				{
					memcpy((void*)(ulong)(pBuff->base_kern_vir + pBuff->wt), (void *)(ulong)pdata_pkt->stData[idx].addr, len);
					pAttr->addr = pBuff->wt;
					pBuff->wt = (pBuff->wt + len) % pBuff->size;
					ret[idx] = MT_SUCCESS;
				}
			}
			else
			{
				if (pBuff->wt + len < pBuff->size)
				{
					memcpy((void*)(ulong)(pBuff->base_kern_vir + pBuff->wt), (void *)(ulong)pdata_pkt->stData[idx].addr, len);
					pAttr->addr = pBuff->wt;
					pBuff->wt = (pBuff->wt + len) % pBuff->size;
					ret[idx] = MT_SUCCESS;
				}
				else if (len < pBuff->rd)
				{
					memcpy((void*)(ulong)pBuff->base_kern_vir, (void *)(ulong)pdata_pkt->stData[idx].addr, len);
					pAttr->addr = 0;
					pBuff->wt = len % pBuff->size;
					ret[idx] = MT_SUCCESS;
				}
			}
			if (MT_SUCCESS == ret[idx])
			{
				pAttr->len = len;
				pBuff->wtCnt++;
				pBuff->wtByte += len;
				//pBuff->wt = (pBuff->wt + len) % pBuff->size;
				AO_LOG_DBG("SBuff-push[%d|%x/%x]\n", idx, pBuff->rd, pBuff->wt);
			}
		}
	}

	//pkt queue wt update
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		if (MT_SUCCESS != ret[idx])
		{
			break;
		}
	}
	if (AUD_SND_SW_BUFF_NUM <= idx)
	{
		pPkt->wt_cnt++;
		pPkt->wt = (pPkt->wt + 1) % pPkt->num;
		AO_LOG_DBG("sw buff push SUCCESS, rd/wt/num[%d/%d/%d]\n", pPkt->rd, pPkt->wt, pPkt->num);
	}
	else
	{
		AO_LOG_DBG("sw buff push FAIL, rd/wt/num[%d/%d/%d]\n", pPkt->rd, pPkt->wt, pPkt->num);
	}

	return MT_SUCCESS;
}

static mt_s32 drv_snd_hw_buff_check(SND_BUFF *pBuff, mt_u32 len)
{
	mt_u32 buff_space = 0;

	buff_space = pBuff->size - (drv_snd_reg_rd(pBuff->regBuffCnt) << 3);
	AO_LOG_DBG("BUFF[%d/%d/%d, %#x/%d]\n", buff_space, len, pBuff->size,
	           pBuff->regBuffCnt, drv_snd_reg_rd(pBuff->regBuffCnt));

	if (buff_space > len)
	{
		return MT_SUCCESS;
	}
	else
	{
		return MT_FAILURE;
	}
}

static int g_dump_enable = 0;
static void *g_dump_handle = NULL;
int snd_dump_data(const char *data, int size)
{
	int ret;

	if (!g_dump_enable)
	{
		return 0;
	}

	mt_dcache_invalid((void *)data, size);
	ret = mt_drv_dump_do(g_dump_handle, (const mt_u8 *)data, size);
	if (ret != size)
	{
		pr_err("dump data error!!! size:%d\n", size);
		return -1;
	}

	return 0;
}

extern void *AO_MEMCPY(void *dest, const void *src, size_t n);
static int write_data_to_hw_buf(void *dst, u32 dst_wt, u32 dst_buf_size, void *data, u32 data_size)
{
	u32 tmp;

	if ((NULL == dst) || (NULL == data))
	{
		AO_LOG_ERR("invalid parameter [%#lx/%#lx]\n", (ulong)dst, (ulong)data);
		return -1;
	}
	
	if (dst_wt + data_size > dst_buf_size)
	{
		tmp = dst_buf_size - dst_wt;
		AO_MEMCPY(dst + dst_wt, data, tmp);

		AO_MEMCPY(dst, data + tmp, data_size - tmp);
	}
	else
	{
		AO_MEMCPY(dst + dst_wt, data, data_size);
	}

	return 0;
}

static mt_s32 drv_snd_hw_buff_write(SND_CARD *snd_card, AUD_SND_DATA_PKT *pdata_pkt)
{
	mt_u32 idx = 0, i;
//	mt_u32 tmp = 0;
	void *dst = NULL, *src = NULL;
	long ret = MT_FAILURE;
	SND_BUFF *pHwBuff = NULL;
	SND_BUFF *pSwBuff = NULL;
	AUD_SND_BUFF_ATTR *pPktBuff = NULL;
	mt_u32 data_len = 0;

	if (DRV_SND_CARD_NUM <= snd_card->idx)
	{
		AO_LOG_ERR("card index[%d] ERROR\n", snd_card->idx);
		return ret;
	}
	if (SND_CARD_TYPE_MASTER != snd_card->type)
	{
		AO_LOG_ERR("card type[%d] isn't master\n", snd_card->type);
		return ret;
	}

	//data package hw buffer space check
	for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
	{
		pHwBuff = &snd_card->hwBuff[idx];
		if (pHwBuff->used && (pHwBuff->swBuffIdx < AUD_SND_SW_BUFF_NUM))
		{
			data_len = pdata_pkt->stData[pHwBuff->swBuffIdx].len;
			if(0 == idx && pdata_pkt->channel)
				data_len /= pdata_pkt->channel;
			if(0 >= pdata_pkt->channel || 8 < pdata_pkt->channel) {
				if( (AC3 == pdata_pkt->dolby_type || EAC3 == pdata_pkt->dolby_type) && (0 == pdata_pkt->channel))
					AO_LOG_INFO("maybe it's normal due to just output raw data!\n");
				else
					AO_LOG_ERR("pdata_pkt->channel is %d, len:%d, need check!\n", pdata_pkt->channel,data_len);
			}
			if (MT_SUCCESS != drv_snd_hw_buff_check(pHwBuff, data_len))
			{
				AO_LOG_DBG("snd data package hw buffer[%d] no space\n", idx);
				return ret;
			}
		}
	}

	//hw buff data copy and wt update
	for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
	{
		pHwBuff = &snd_card->hwBuff[idx];
		if (pHwBuff->used && (pHwBuff->swBuffIdx < AUD_SND_SW_BUFF_NUM))
		{
			pPktBuff = &pdata_pkt->stData[pHwBuff->swBuffIdx];
			pSwBuff = snd_card->swBuff[pHwBuff->swBuffIdx];

			if (0 == pPktBuff->len)
			{
				continue;
			}
			
			//pr_err("<ah>L:%d %llx/%llx\n", __LINE__, (pSwBuff->base_kern_vir + (ulong)pPktBuff->addr), pPktBuff->len);
			if (pSwBuff->base_kern_vir & 0xfff)
			{
				AO_LOG_ERR("invalid sw addr:%llx\n", pSwBuff->base_kern_vir);
			}
			else
			{
				mt_dcache_invalid((void *)((ulong)(pSwBuff->base_kern_vir)), pSwBuff->size);
			}

			if (0 == idx)	// pp buf
			{
				u32 one_pp_buf_size = pHwBuff->size;
				u32 one_ch_size = (pdata_pkt->frameSample << 2);
				for (i = 0; i < 8; i++)
				{
					if (i < pdata_pkt->channel)
					{
						//pr_err("i:%d hw_wt:%x ch_size:%x pp_size:%x sw_rd:%x\n", i, pHwBuff->wt, one_ch_size, one_pp_buf_size, pPktBuff->addr);
						dst = (void *)(ulong)(pHwBuff->base_phy + i * one_pp_buf_size);
						src = (void *)(ulong)(pSwBuff->base_phy + (ulong)pPktBuff->addr + i * one_ch_size);
						write_data_to_hw_buf(dst, pHwBuff->wt, one_pp_buf_size, src, one_ch_size);
						if (0 == i)
						{
							snd_dump_data((char*)(ulong)(pSwBuff->base_kern_vir + (ulong)pPktBuff->addr + i * one_ch_size), one_ch_size);
						}
						snd_reg_34H_set_pp_buf_wr_cmd(i, one_ch_size >> 3);
					}
				}
				pHwBuff->wt += one_ch_size;
				pHwBuff->wt %= one_pp_buf_size;
			}
			else if (idx != 2)//drop mix buffer
			{
				src = (void *)(ulong)(pSwBuff->base_phy + (ulong)pPktBuff->addr);
				write_data_to_hw_buf((void *)(ulong)pHwBuff->base_phy, pHwBuff->wt, pHwBuff->size, src, pPktBuff->len);
				snd_dump_data((void *)(ulong)(pSwBuff->base_kern_vir + pPktBuff->addr), pPktBuff->len);

				pHwBuff->wt += pPktBuff->len;
				pHwBuff->wt %= pHwBuff->size;
				drv_snd_reg_wt(pHwBuff->regWtCmd, (pPktBuff->len >> 3) | 0x80000000);
				pHwBuff->wtCnt++;
				pHwBuff->wtByte += pPktBuff->len;
			}
			
			pSwBuff->rd = ((ulong)pPktBuff->addr + pPktBuff->len)%pSwBuff->size;
		}
		AO_LOG_DBG("hw buff[#%d]: rd/wt/size[%x/%x/%x], wtCnt/wtByte[%x/%x]-TMP[%#x]\n",
				idx, pHwBuff->rd, pHwBuff->wt, pHwBuff->size,  pHwBuff->wtCnt, pHwBuff->wtByte, pHwBuff->regWtCmd);
	}

	snd_card->stCfg.frmIdx = pdata_pkt->frmIdx;
	snd_card->stCfg.apts = pdata_pkt->apts;
	return MT_SUCCESS;
}

static mt_s32 drv_snd_sw_buff_check(SND_BUFF *pBuff, mt_u32 len)
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

long drv_snd_sw_buff_write(SND_CARD *snd_card, AUD_SND_DATA_PKT *pdata_pkt)
{
	mt_u32 idx = 0;
	mt_u32 tmp = 0;
	long ret = MT_FAILURE;
	AUD_SND_DATA_PKT_QUE *pPkt = NULL;

	if (NULL == snd_card)
	{
		AO_LOG_ERR("snd_card is NULL\n");
		return ret;
	}
	pPkt = snd_card->pPkt;

	if (DRV_SND_CARD_NUM <= snd_card->idx)
	{
		AO_LOG_ERR("card index[%d] ERROR\n", snd_card->idx);
		return ret;
	}
	if (SND_CARD_TYPE_MASTER != snd_card->type)
	{
		AO_LOG_ERR("card type[%d] isn't master\n", snd_card->type);
		return ret;
	}

	//data package queue node check
	tmp = pPkt->rd > pPkt->wt ? (pPkt->rd - pPkt->wt) : (pPkt->num - pPkt->wt + pPkt->rd);
	if (!tmp)
	{
		AO_LOG_DBG("snd data package queue no space\n");
		return ret;
	}

	//data package sw buffer space check
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		if (pdata_pkt->stData[idx].fmt && (AUD_SND_DATA_FMT_NULL != pdata_pkt->stData[idx].fmt))
		{
			if (MT_SUCCESS != drv_snd_sw_buff_check(snd_card->swBuff[idx], pdata_pkt->stData[idx].len))
			{
				AO_LOG_DBG("snd data package sw buffer[%d] no space, pPkt.wt_cnt[%d]\n", idx, pPkt->wt_cnt);
				return ret;
			}
		}
	}

	//data copy and pkt queue update
	if (MT_SUCCESS != drv_snd_data_pkt_push(snd_card, pdata_pkt))
	{
		AO_LOG_DBG("snd data package sw buffer push FAIL\n");
	}

	return MT_SUCCESS;
}

long drv_adec_output_to_snd(AUD_SND_DATA_PKT *pdata_pkt)
{
	mt_u32 idx = 0;
	SND_CARD *snd_card = NULL;
	SND_GET_SHARE_PKT();

	for (idx = 0; idx < DRV_SND_CARD_NUM; idx++)
	{
		if ((SND_CARD_TYPE_MASTER == gst_snd_card[idx].type) && (SND_CARD_STA_IDLE != gst_snd_card[idx].sta))
		{
			break;
		}
	}
	if (DRV_SND_CARD_NUM <= idx)
	{
		SND_LOG(SND_LOG_POS_TODO, "no master snd card is opened\n");
		return MT_FAILURE;
	}
	else
	{
		snd_card = &gst_snd_card[idx];
	}

	if (SND_CARD_STA_STOP == snd_card->sta)
	{
		return MT_FAILURE;
	}

	SND_LOG(SND_LOG_POS_TODO, "frmIdx[%d], pkt[%d<%llx/%d>]\n", pdata_pkt->frmIdx, pdata_pkt->stData[0].fmt,
	        pdata_pkt->stData[0].addr, pdata_pkt->stData[0].len);
	return drv_snd_sw_buff_write(snd_card, pdata_pkt);
}
EXPORT_SYMBOL(drv_adec_output_to_snd);

extern ulong mt_get_audioout_base(void);
extern ulong mt_get_clk_base(void);
extern ulong mt_get_hdmi_base(void);
extern ulong mt_get_analog_base(void);
static int snd_task(void *data);
static int tts_snd_task(void *data);

static long drv_snd_init(SND_CARD *snd_card, AUD_SND_INIT_PARAM *pParam)
{
	mt_u32 idx = 0;
	//long ret = MT_FAILURE;
	//struct sched_param param;
	AUD_SND_DATA_PKT_QUE *pPkt = snd_card->pPkt;
	struct sched_param param;
	int ret = 0;
	mmz_buffer_s ttsbuffer_mmz;
	mmz_buffer_s ttstemp_mmz;
	
	//snd card type check
	for (idx = 0; idx < DRV_SND_CARD_NUM; idx++)
	{
		if (pParam->type == gst_snd_card[idx].type)
		{
			break;
		}
	}
	if (DRV_SND_CARD_NUM > idx)
	{
		SND_LOG(SND_LOG_POS_ERR, "type[%d] is already occupied in card #%d\n", pParam->type, idx);
		return MT_FAILURE;
	}
	snd_card->type = pParam->type;

	//resource allocation
	if (SND_CARD_TYPE_MASTER == pParam->type)
	{
		//HW buff: PP/AOUT/MIX/SPD buffer
		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("SndPpBuff", MMZ_ZONE_PCM/*MMZ_ZONE_PCM*/, SND_HW_BUFF_PP_SIZE, SND_BUFF_ADDR_ALIGN, &snd_card->hw_buf_mmz[0]))
		{
			SND_LOG(SND_LOG_POS_ERR, "SndPpBuff malloc FAIL\n");
			goto INIT_ERR_EXIT1;
		}
		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("SndAoutBuff", MMZ_ZONE_PCM/*MMZ_ZONE_PCM*/, SND_HW_BUFF_AOUT_SIZE, SND_BUFF_ADDR_ALIGN, &snd_card->hw_buf_mmz[1]))
		{
			SND_LOG(SND_LOG_POS_ERR, "SndAoutBuff malloc FAIL\n");
			goto INIT_ERR_EXIT2;
		}
		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("SndMixBuff", MMZ_ZONE_PCM/*MMZ_ZONE_PCM*/, SND_HW_BUFF_MIX_SIZE, SND_BUFF_ADDR_ALIGN, &snd_card->hw_buf_mmz[2]))
		{
			SND_LOG(SND_LOG_POS_ERR, "SndMixBuff malloc FAIL\n");
			goto INIT_ERR_EXIT3;
		}
		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("SndSpdBuff", MMZ_ZONE_PCM/*MMZ_ZONE_PCM*/, SND_HW_BUFF_SPD_SIZE, SND_BUFF_ADDR_ALIGN, &snd_card->hw_buf_mmz[3]))
		{
			SND_LOG(SND_LOG_POS_ERR, "SndSpdBuff malloc FAIL\n");
			goto INIT_ERR_EXIT4;
		}

		//buff Rd/Wt
		for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
		{
			//memset(&pCard->hwBuff[idx], 0, sizeof(SND_BUFF));
			snd_card->hwBuff[idx].rd = 0;
			snd_card->hwBuff[idx].wt = 0;
			snd_card->hwBuff[idx].wtCnt = 0;
			snd_card->hwBuff[idx].wtByte = 0;
			snd_card->hwBuff[idx].swBuffIdx = 0xff;
			snd_card->hwBuff[idx].base_phy = (u64)snd_card->hw_buf_mmz[idx].startPhyAddr;
			snd_card->hwBuff[idx].base_kern_vir = (u64)(ulong)snd_card->hw_buf_mmz[idx].startVirAddr;
			
			if(0 == idx)
				snd_card->hwBuff[idx].size = snd_card->hw_buf_mmz[idx].size >> 3;
			else 
				snd_card->hwBuff[idx].size = snd_card->hw_buf_mmz[idx].size;

			//SND_LOG(SND_LOG_POS_FLOW, "hwBuff[%d]=<%#llx,%#x>\n", idx, snd_card->hwBuff[idx].base_kern_vir, snd_card->hwBuff[idx].size);
		}

		for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
		{
			snd_card->swBuff[idx]->rd = 0;
			snd_card->swBuff[idx]->wt = 0;
			snd_card->swBuff[idx]->wtCnt = 0;
			snd_card->swBuff[idx]->wtByte = 0;
			snd_card->swBuff[idx]->used = 1;
			snd_card->swBuff[idx]->swBuffIdx = 0xff;
			//SND_LOG(SND_LOG_POS_FLOW, "swBuff[%d]=<%#llx,%#x>\n", idx, snd_card->swBuff[idx]->base_kern_vir, snd_card->swBuff[idx]->size);
		}

		pPkt->rd = 0;
		pPkt->wt = 0;
		pPkt->wt_cnt = 0;
		pPkt->num = AUD_SND_DATA_PKT_NUM;
		memset((void *)pPkt->stQue, 0, sizeof(AUD_SND_DATA_PKT) * AUD_SND_DATA_PKT_NUM);

		//cfg init
		snd_card->volume = 100;
		snd_card->aout_need_cfg = 1;
		snd_card->hdmi_need_cfg = 1;
		snd_card->type = SND_CARD_TYPE_MASTER;
		//snd_card->sta = SND_CARD_STA_PKT_POP;

		//get aout/clock reg base address and ioremap
		reg_aout_base_ioremap = mt_get_audioout_base();
		//SND_LOG(SND_LOG_POS_FLOW, "reg_aout_base_ioremap=0x%lx\n", reg_aout_base_ioremap);
		reg_aout_clk_base_ioremap = mt_get_clk_base();
		//SND_LOG(SND_LOG_POS_FLOW, "reg_aout_clk_base_ioremap=0x%lx\n", reg_aout_clk_base_ioremap);
		reg_hdmi_audio_ctrl_ioremap = mt_get_hdmi_base();
		//SND_LOG(SND_LOG_POS_FLOW, "reg_hdmi_audio_ctrl_ioremap=0x%lx\n", reg_hdmi_audio_ctrl_ioremap);

		//task thread
		gpst_snd_task = kthread_create(snd_task, (void *)snd_card, "snd_task");
		if (IS_ERR_OR_NULL(gpst_snd_task))
		{
			SND_LOG(SND_LOG_POS_ERR, "DRV SND task create FAIL\n");
			gpst_snd_task = NULL;
			goto INIT_ERR_EXIT4;
		}
		
		param.sched_priority = 99;
		sched_setscheduler(gpst_snd_task, SCHED_RR, &param);
		wake_up_process(gpst_snd_task);

		snd_card->tts_buffer_size = 1024*4*36;
		memset(&ttsbuffer_mmz, 0, sizeof(mmz_buffer_s));
		ret = mt_drv_mmz_alloc("tts_buf", MMZ_ZONE_DDR, snd_card->tts_buffer_size, 64, &ttsbuffer_mmz);
		if (MT_SUCCESS != ret) {
			SND_LOG(SND_LOG_POS_ERR, "tts_buf mt_drv_mmz_alloc error\n");
		}
		ret = mt_drv_mmz_map(&ttsbuffer_mmz);
		if (MT_SUCCESS != ret) {
			SND_LOG(SND_LOG_POS_ERR, "tts_buf mt_drv_mmz_map error\n");
		}
		snd_card->tts_buffer_phy = ttsbuffer_mmz.startPhyAddr;
		snd_card->tts_buffer_vir = (u8 *)ttsbuffer_mmz.startVirAddr;

		snd_card->tts_temp_size = TTS_FRM_SAMPLE*4;
		memset(&ttstemp_mmz, 0, sizeof(mmz_buffer_s));
		ret = mt_drv_mmz_alloc("tts_temp", MMZ_ZONE_DDR, snd_card->tts_temp_size, 64, &ttstemp_mmz);
		if (MT_SUCCESS != ret) {
			SND_LOG(SND_LOG_POS_ERR, "tts_temp mt_drv_mmz_alloc error\n");
		}
		ret = mt_drv_mmz_map(&ttstemp_mmz);
		if (MT_SUCCESS != ret) {
			SND_LOG(SND_LOG_POS_ERR, "tts_temp mt_drv_mmz_map error\n");
		}
		snd_card->tts_temp_phy = ttstemp_mmz.startPhyAddr;
		snd_card->tts_temp_vir = (u32 *)ttstemp_mmz.startVirAddr;

		gtts_snd_task = kthread_create(tts_snd_task, (void *)snd_card, "tts_snd_task");
		if (IS_ERR_OR_NULL(gtts_snd_task))
		{
			SND_LOG(SND_LOG_POS_ERR, "DRV TTS SND task create FAIL\n");
			gtts_snd_task = NULL;
		}
		
		param.sched_priority = 99;
		sched_setscheduler(gtts_snd_task, SCHED_RR, &param);
		wake_up_process(gtts_snd_task);
	}
	else if (SND_CARD_TYPE_SLAVE == snd_card->type)
	{
		SND_LOG(SND_LOG_POS_FLOW, "TODO...<not support currently>\n");
	}
	else if (SND_CARD_TYPE_VIRTUAL == snd_card->type)
	{
		SND_LOG(SND_LOG_POS_FLOW, "TODO...<not support currently>\n");
	}

	SND_LOG(SND_LOG_POS_FLOW, "snd card[idx=%d, type=%d] init SUCCESS\n", snd_card->idx, snd_card->type);
	return MT_SUCCESS;

INIT_ERR_EXIT4:
	mt_drv_mmz_unmap_and_release(&snd_card->hw_buf_mmz[2]);
INIT_ERR_EXIT3:
	mt_drv_mmz_unmap_and_release(&snd_card->hw_buf_mmz[1]);
INIT_ERR_EXIT2:
	mt_drv_mmz_unmap_and_release(&snd_card->hw_buf_mmz[0]);
INIT_ERR_EXIT1:

	return MT_FAILURE;
}

static long drv_snd_uninit(SND_CARD *snd_card)
{
	mt_u32 idx = 0;
	AUD_SND_DATA_PKT_QUE *pPkt = snd_card->pPkt;
	mmz_buffer_s ttsbuffer_mmz;
	mmz_buffer_s ttstemp_mmz;

	//aout stop and buff reset
	snd_reg_DCH_set_pcm_play_stop(1);

	reset_hw_buf();

	//resource release
	if (SND_CARD_TYPE_MASTER == snd_card->type)
	{
		//snd task stop
		if (!IS_ERR_OR_NULL(gpst_snd_task))
		{
			kthread_stop(gpst_snd_task);
			SND_LOG(SND_LOG_POS_FLOW, "DRV SND task deinit SUCCESS\n");
		}
		//TODO...wait thread exit
		//while(1)
		gpst_snd_task = NULL;

		//tts task stop
		if (!IS_ERR_OR_NULL(gtts_snd_task))
		{
			kthread_stop(gtts_snd_task);
			SND_LOG(SND_LOG_POS_FLOW, "DRV tts task deinit SUCCESS\n");
		}
		gtts_snd_task = NULL;
		
		if(snd_card->tts_buffer_vir){
			ttsbuffer_mmz.size = snd_card->tts_buffer_size;
			ttsbuffer_mmz.startPhyAddr = snd_card->tts_buffer_phy;
			ttsbuffer_mmz.startVirAddr = snd_card->tts_buffer_vir;
			mt_drv_mmz_unmap_and_release(&ttsbuffer_mmz);
			snd_card->tts_buffer_vir = NULL;
		}

		if(snd_card->tts_temp_vir){
			ttstemp_mmz.size = snd_card->tts_temp_size;
			ttstemp_mmz.startPhyAddr = snd_card->tts_temp_phy;
			ttstemp_mmz.startVirAddr = snd_card->tts_temp_vir;
			mt_drv_mmz_unmap_and_release(&ttsbuffer_mmz);
			snd_card->tts_temp_vir = NULL;
		}

		//buff free
		for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
		{
			snd_card->hwBuff[idx].used = 0;
			snd_card->hwBuff[idx].rd = 0;
			snd_card->hwBuff[idx].wt = 0;
			snd_card->hwBuff[idx].wtCnt = 0;
			snd_card->hwBuff[idx].wtByte = 0;
			snd_card->hwBuff[idx].swBuffIdx = 0xff;
			snd_card->hwBuff[idx].size = 0;
			snd_card->hwBuff[idx].base_phy = 0;
			snd_card->hwBuff[idx].base_kern_vir = 0;
			if (snd_card->hw_buf_mmz[idx].startVirAddr)
			{
				mt_drv_mmz_unmap_and_release(&snd_card->hw_buf_mmz[idx]);
			}
		}

		//data package queue
		//TODO... malloc queue
		pPkt->rd = 0;
		pPkt->wt = 0;
		pPkt->wt_cnt = 0;
		pPkt->num = 0;
		memset((void *)pPkt->stQue, 0, sizeof(AUD_SND_DATA_PKT) * AUD_SND_DATA_PKT_NUM);

		//card status release
		snd_card->type = SND_CARD_TYPE_NULL;
		snd_card->sta = SND_CARD_STA_IDLE;
	}

	SND_LOG(SND_LOG_POS_FLOW, "deinit exit<<<\n");
	return 0;
}

static int drv_snd_start(SND_CARD *snd_card)
{
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx]\n", (ulong)snd_card);
		return -1;
	}

	snd_card->stop_flag = 0;
	snd_card->play_cnt = 0;
	snd_card->drop_cnt = 0;
	snd_card->sta = SND_CARD_STA_PKT_POP;
	snd_card->pause_flag = 0;
	pr_err("<ah><%s:%d>\n", __func__, __LINE__);
	
	return 0;
}

static int drv_snd_stop(SND_CARD *snd_card)
{
	mt_u32 idx = 0, cnt = 0;
	AUD_SND_DATA_PKT_QUE *pPkt;
	
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx]\n", (ulong)snd_card);
		return -1;
	}

	if (SND_CARD_STA_STOP == snd_card->sta)
	{
		AO_LOG_ERR("snd already stoped!\n");
		return 0;
	}

	snd_reg_DCH_set_pcm_play_stop(1);

	snd_card->stop_flag = 1;
	while (snd_card->stop_flag)
	{
		msleep_interruptible(1);
		if (cnt++ > 500)
		{
			SND_LOG(SND_LOG_POS_ERR, "snd_task stop error!!!\n");
			//return -1;
		}
	}

	pPkt = snd_card->pPkt;

	reset_hw_buf();

	//buff Rd/Wt
	for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
	{
		//memset(&pCard->hwBuff[idx], 0, sizeof(SND_BUFF));
		snd_card->hwBuff[idx].rd = 0;
		snd_card->hwBuff[idx].wt = 0;
		snd_card->hwBuff[idx].wtCnt = 0;
		snd_card->hwBuff[idx].wtByte = 0;
		drv_snd_reg_wt(snd_card->hwBuff[idx].regWtCmd, 0);
	}

	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		snd_card->swBuff[idx]->rd = 0;
		snd_card->swBuff[idx]->wt = 0;
		snd_card->swBuff[idx]->wtCnt = 0;
		snd_card->swBuff[idx]->wtByte = 0;
		snd_card->swBuff[idx]->used = 1;
	}

	pPkt->rd = 0;
	pPkt->wt = 0;
	pPkt->wt_cnt = 0;
	pPkt->num = AUD_SND_DATA_PKT_NUM;
	memset((void *)pPkt->stQue, 0, sizeof(AUD_SND_DATA_PKT) * AUD_SND_DATA_PKT_NUM);
    memset((void *)&snd_card->stCfg, 0, sizeof(AUD_SND_DATA_PKT));

	pr_err("<ah><%s:%d>\n", __func__, __LINE__);

	return 0;
}

static int drv_snd_pause(SND_CARD *snd_card)
{
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx]\n", (ulong)snd_card);
		return -1;
	}

	snd_card->pause_flag = 1;
	//snd_reg_DCH_set_pcm_play_stop(1);

	return 0;
}

static int drv_snd_resume(SND_CARD *snd_card)
{
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx]\n", (ulong)snd_card);
		return -1;
	}

	//snd_reg_DCH_set_pcm_play_stop(0);
	snd_card->pause_flag = 0;

	return 0;
}

static int drv_snd_flush(SND_CARD *snd_card)
{
	int i;
	u32 stop_bit = 0;

	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx]\n", (ulong)snd_card);
		return -1;
	}

	stop_bit = 0x1 & snd_reg_DCH_get_pcm_play_stop();
	snd_reg_DCH_set_pcm_play_stop(1);
	mdelay(1);
	reset_hw_buf();
	mdelay(1);
	for (i = 0; i < AUD_SND_HW_BUFF_NUM; i++)
	{
		//memset(&pCard->hwBuff[idx], 0, sizeof(SND_BUFF));
		snd_card->hwBuff[i].rd = 0;
		snd_card->hwBuff[i].wt = 0;
		snd_card->hwBuff[i].wtCnt = 0;
		snd_card->hwBuff[i].wtByte = 0;
	}
	mdelay(1);
	snd_reg_DCH_set_pcm_play_stop(stop_bit);

	return 0;
}

void adec_flush_snd(void)
{
	drv_snd_flush(&gst_snd_card[0]);
}

EXPORT_SYMBOL(adec_flush_snd);

int alloc_snd_sw_buf(aud_share_buf_t *share_buf, aud_share_buf_t *share_buf_av, aud_buf_param_t *buf_param)
{
	int ret, i;
	u32 sw_buf_size = 0;
	SND_CARD *snd_card = NULL;

	if ((NULL == share_buf) || (NULL == buf_param))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter [%lx/%lx]\n", (ulong)share_buf, (ulong)buf_param);
		return -1;
	}

	snd_card = &gst_snd_card[0];
	sw_buf_size = SND_SW_BUFF_0_SIZE + SND_SW_BUFF_1_SIZE + SND_SW_BUFF_2_SIZE;

	ret = mt_drv_mmz_alloc("swBuff", MMZ_ZONE_PCM, sw_buf_size, SND_BUFF_ADDR_ALIGN, &snd_card->sw_buf_mmz);

	if (MT_SUCCESS != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "swBuff mt_drv_mmz_alloc error\n");
		return -1;
	}

	buf_param->snd_buf_phy = snd_card->sw_buf_mmz.startPhyAddr;
	SND_LOG(SND_LOG_POS_ERR, "sw buf phy:%#lx\n", (ulong)buf_param->snd_buf_phy);

	ret = mt_drv_mmz_map(&snd_card->sw_buf_mmz);	// for avcpu
	if (0 != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "mt_drv_mmz_map_cache snd sw buffer error!");
		mt_drv_mmz_release(&snd_card->sw_buf_mmz);
		return -1;
	}

	share_buf->snd_sw_buf[0].base_phy = (u64)snd_card->sw_buf_mmz.startPhyAddr;
	share_buf->snd_sw_buf[0].base_kern_vir = (u64)(ulong)snd_card->sw_buf_mmz.startVirAddr;
	share_buf->snd_sw_buf[0].size = SND_SW_BUFF_0_SIZE;

	share_buf->snd_sw_buf[1].base_phy = share_buf->snd_sw_buf[0].base_phy + SND_SW_BUFF_0_SIZE;
	share_buf->snd_sw_buf[1].base_kern_vir = share_buf->snd_sw_buf[0].base_kern_vir + SND_SW_BUFF_0_SIZE;
	share_buf->snd_sw_buf[1].size = SND_SW_BUFF_1_SIZE;

	share_buf->snd_sw_buf[2].base_phy = share_buf->snd_sw_buf[1].base_phy + SND_SW_BUFF_1_SIZE;
	share_buf->snd_sw_buf[2].base_kern_vir = share_buf->snd_sw_buf[1].base_kern_vir + SND_SW_BUFF_1_SIZE;
	share_buf->snd_sw_buf[2].size = SND_SW_BUFF_2_SIZE;

	for (i = 0; i < AUD_SND_SW_BUFF_NUM; i++)
	{
		//memset(&pCard->swBuff[idx], 0, sizeof(SND_BUFF));
		share_buf->snd_sw_buf[i].rd = 0;
		share_buf->snd_sw_buf[i].wt = 0;
		share_buf->snd_sw_buf[i].wtCnt = 0;
		share_buf->snd_sw_buf[i].wtByte = 0;
		share_buf->snd_sw_buf[i].used = 1;
		share_buf->snd_sw_buf[i].swBuffIdx = 0xff;

		snd_card->swBuff[i] = &share_buf->snd_sw_buf[i];
	}

	snd_card->share_buf = share_buf;
	snd_card->pPkt = &share_buf->snd_pkt;

	buf_param->aud_buf_size_map.snd_sw_buf0_size = SND_SW_BUFF_0_SIZE;
	buf_param->aud_buf_size_map.snd_sw_buf1_size = SND_SW_BUFF_1_SIZE;
	buf_param->aud_buf_size_map.snd_sw_buf2_size = SND_SW_BUFF_2_SIZE;

	snd_card->pPkt->rd = 0;
	snd_card->pPkt->wt = 0;
	snd_card->pPkt->wt_cnt = 0;
	snd_card->pPkt->num = AUD_SND_DATA_PKT_NUM;
	memset((void *)snd_card->pPkt->stQue, 0, sizeof(AUD_SND_DATA_PKT) * AUD_SND_DATA_PKT_NUM);

//for av_cpu case:
	if(NULL != share_buf_av)
	{	
		share_buf_av->snd_sw_buf[0].base_phy = (u64)snd_card->sw_buf_mmz.startPhyAddr;
		share_buf_av->snd_sw_buf[0].base_kern_vir = (u64)(ulong)snd_card->sw_buf_mmz.startVirAddr;
		share_buf_av->snd_sw_buf[0].size = SND_SW_BUFF_0_SIZE;
		
		share_buf_av->snd_sw_buf[1].base_phy = share_buf_av->snd_sw_buf[0].base_phy + SND_SW_BUFF_0_SIZE;
		share_buf_av->snd_sw_buf[1].base_kern_vir = share_buf_av->snd_sw_buf[0].base_kern_vir + SND_SW_BUFF_0_SIZE;
		share_buf_av->snd_sw_buf[1].size = SND_SW_BUFF_1_SIZE;
		
		share_buf_av->snd_sw_buf[2].base_phy = share_buf_av->snd_sw_buf[1].base_phy + SND_SW_BUFF_1_SIZE;
		share_buf_av->snd_sw_buf[2].base_kern_vir = share_buf_av->snd_sw_buf[1].base_kern_vir + SND_SW_BUFF_1_SIZE;
		share_buf_av->snd_sw_buf[2].size = SND_SW_BUFF_2_SIZE;
	
		for (i = 0; i < AUD_SND_SW_BUFF_NUM; i++)
		{
			//memset(&pCard->swBuff[idx], 0, sizeof(SND_BUFF));
			share_buf_av->snd_sw_buf[i].rd = 0;
			share_buf_av->snd_sw_buf[i].wt = 0;
			share_buf_av->snd_sw_buf[i].wtCnt = 0;
			share_buf_av->snd_sw_buf[i].wtByte = 0;
			share_buf_av->snd_sw_buf[i].used = 1;
			share_buf_av->snd_sw_buf[i].swBuffIdx = 0xff;
		}
		share_buf_av->snd_pkt.rd = 0;
		share_buf_av->snd_pkt.wt = 0;
		share_buf_av->snd_pkt.wt_cnt = 0;
		share_buf_av->snd_pkt.num = AUD_SND_DATA_PKT_NUM;
		memset((void *)share_buf_av->snd_pkt.stQue, 0, sizeof(AUD_SND_DATA_PKT) * AUD_SND_DATA_PKT_NUM);
	}
	return 0;
}
EXPORT_SYMBOL(alloc_snd_sw_buf);

int release_snd_sw_buf(void)
{
	int i;
	SND_CARD *snd_card = NULL;
	snd_card = &gst_snd_card[0];

	mt_drv_mmz_unmap(&snd_card->sw_buf_mmz);
	mt_drv_mmz_release(&snd_card->sw_buf_mmz);

	for (i = 0; i < AUD_SND_SW_BUFF_NUM; i++)
	{
		snd_card->swBuff[i] = NULL;
	}

	snd_card->pPkt = NULL;

	return 0;
}
EXPORT_SYMBOL(release_snd_sw_buf);
void adec_flush_all(void);
mt_u32 adec_flush_status(void);
mt_s32 is_liveplay(void);

static int snd_task(void *data)
{
	//MT_BOOL retry = MT_FALSE;
	mt_u32 aout_cfg_retry = 0;
	mt_u32 data_send_retry = 0;
	mt_u32 ao_time = 0;
	mt_u32 i = 0;
	AUD_SND_DATA_PKT data_pkt;
	SND_CARD *snd_card = (SND_CARD *)data;
	AVSYNC_FRAME_SYNCFLAG_E avsync_flag = AVSYNC_FRAME_PLAY;
	SND_GET_SHARE_PKT();

	while (1)
	{
		if (kthread_should_stop())
		{
			SND_LOG(SND_LOG_POS_FLOW, "<SND-TASK-EXIT>\n");
			break;
		}

		if (1 == snd_card->stop_flag)
		{
			snd_card->stop_flag = 0;
			snd_card->sta = SND_CARD_STA_STOP;
			snd_card->last_pts = 0;
			pr_err("<%s:%d> snd_task stop!\n", __func__, __LINE__);
		}

		if (SND_CARD_STA_STOP == snd_card->sta || snd_card->pause_flag == 1)
		{
			msleep_interruptible(10);
			continue;
		}

		switch (snd_card->sta)
		{
		case SND_CARD_STA_PKT_POP:
			if (MT_SUCCESS != drv_snd_data_pkt_pop(snd_card, &data_pkt))
			{
				msleep_interruptible(1);
				break;
			}
			aout_cfg_retry = 0;
			snd_card->hdmi_cfg_retry = 0;
			data_send_retry = 0;
			snd_card->sta = SND_CARD_STA_AOUT_CFG ;
			break;
			
		case SND_CARD_STA_AOUT_CFG:
			if (MT_SUCCESS != drv_snd_aout_config(snd_card, &data_pkt))
			{
				//if(SND_AOUT_CFG_RETRY_MAX <= ++aout_cfg_retry)
				//	pCard->sta = SND_CARD_STA_PKT_POP;
				msleep_interruptible(1);
				break;
			}
			snd_card->sta = SND_CARD_STA_HDMI_CFG;			
			break;
				
		case SND_CARD_STA_HDMI_CFG:
			if (MT_SUCCESS != drv_snd_hdmi_config(snd_card, &data_pkt))
			{
				if (++snd_card->hdmi_cfg_retry >= SND_HDMI_CFG_RETRY_MAX)
				{
					snd_card->hdmi_need_cfg = 0;
				}
				msleep_interruptible(1);
				break;
			}
			snd_card->sta = SND_CARD_STA_AVSYNC ; 		
			break;

		case SND_CARD_STA_AVSYNC:
			avsync_flag = drv_snd_avsync_check(snd_card->swBuff[0], &data_pkt, &ao_time);
			if (AVSYNC_FRAME_PAUSE == avsync_flag)
			{
				msleep_interruptible(1);
				break;
			}
			if (AVSYNC_FRAME_SKIP == avsync_flag)
			{
				snd_card->drop_cnt++;
				snd_card->sta = SND_CARD_STA_PKT_POP;
				drv_snd_drop_data(snd_card, &data_pkt);
				break;
			}

			if (AVSYNC_FRAME_SKIP_AOUT == avsync_flag)//all ao data is dropped
			{
				snd_card->drop_cnt++;
				snd_card->sta = SND_CARD_STA_PKT_POP;
				drv_snd_drop_data(snd_card, &data_pkt);
				if(ao_time > 50*45)
				{
					ao_all_skip_reset_hw_buf();
					for (i = 0; i < AUD_SND_HW_BUFF_NUM; i++)
					{
						snd_card->hwBuff[i].rd = 0;
						snd_card->hwBuff[i].wt = 0;
						snd_card->hwBuff[i].wtCnt = 0;
						snd_card->hwBuff[i].wtByte = 0;
					}
				}//if ao data is enouth , drop ao and flush pts queue
				break;
			}
			snd_card->sta = SND_CARD_STA_HW_BUFF_WRITE;
			break;

		case SND_CARD_STA_HW_BUFF_WRITE:
			//bug 26583 change stream or stop/start play on  stream server, detect it
			if(avsync_is_ddp_avsync_verfication() == MT_SUCCESS)
			{
				if ((0xffffffff - data_pkt.apts > 22500) && (data_pkt.apts + 22500 < snd_card->last_pts)) 
				{
					snd_card->play_cnt = 0;
					snd_card->share_buf->stream_loop = 1;
					snd_card->share_buf->stream_loop_flush = 1;
					snd_reg_DCH_set_pcm_play_stop(1);
					//snd_reg_d0h_set_mix_func_en(0);
					SND_LOG(SND_LOG_POS_FLOW, "StreamLoop/Switch-OK!  dolby cf, cur pts:0x%x, last pts:0x%x\n",
						data_pkt.apts, snd_card->last_pts);
				}
			}
			else
			{
				if(is_liveplay())
				{					
					if ((0xffffffff - data_pkt.apts > 450000) && (data_pkt.apts + 450000 < snd_card->last_pts)) {
						snd_card->play_cnt = 0;
						snd_card->share_buf->stream_loop = 1;
						snd_card->share_buf->stream_loop_flush = 1;
#ifndef CONFIG_MT_FPGA	
							//adec_flush_all();
#endif
						snd_reg_DCH_set_pcm_play_stop(1);
						//snd_reg_d0h_set_mix_func_en(0);
						SND_LOG(SND_LOG_POS_FLOW, "StreamLoop/Switch-OK! flush! cur pts:0x%x, last pts:0x%x\n",
							data_pkt.apts, snd_card->last_pts);
					}
				}
			}
			// end bug 26583	
			
			snd_card->play_cnt++;
			snd_card->last_pts = data_pkt.apts;
			if (snd_card->buffer_thd == snd_card->play_cnt)
			{
				snd_reg_DCH_set_pcm_play_stop(0);
				//snd_reg_d0h_set_mix_func_en(1);
				SND_LOG(SND_LOG_POS_FLOW, "<SND-PLAY>\n");
			}

			if(avsync_get_ao_data_time() > 45*100) {
				msleep_interruptible(4);
				break;
			}

			if (MT_SUCCESS != drv_snd_hw_buff_write(snd_card, &data_pkt))
			{
				msleep_interruptible(4);
				/*if (SND_DATA_SEND_RETRY_MAX <= ++data_send_retry)
				{
					snd_card->sta = SND_CARD_STA_PKT_POP;
					pr_err("snd drop data!!!!!!\n");
				}*/
			}
			else
			{
				snd_card->sta = SND_CARD_STA_PKT_POP;
			}
			
			break;
		default:
			break;
		}

		//msleep_interruptible(1);
	}

	SND_LOG(SND_LOG_POS_FLOW, "thread exit\n");
	return 0;
}

static mt_u32 cal_mix_valid_ms(void)
{
	mt_u32 data_size = 0;
	mt_u32 samples = 0;
	mt_u32 time_ms = 0;
	
	data_size = drv_snd_reg_rd(REG_AUD_MIX_BUF_CNT) << 3;
	samples = data_size >> 2;
	time_ms = samples/48;

	return time_ms;
}

static int tts_snd_task(void *data)
{
	mt_u32 wp, rp, len, valid= 0;
	mt_u8 * srcptr = NULL;
	mt_u32 i = 0;
	mt_u8 wait_cnt = 0;
	SND_BUFF *pHwBuff = NULL;
	AUD_SND_DATA_PKT data_pkt;
	SND_CARD *snd_card = (SND_CARD *)data;

	pr_err("aout tts thread start\n");

	while (!kthread_should_stop()) {
		if(g_aout_tts_stop) {
			snd_card->tts_buffer_rd = snd_card->tts_buffer_wt;
			//reg_sym_linux_mix_func_cfg_mix_en_bit(0);
			g_aout_tts_state = 1;
		}
		
		wp = snd_card->tts_buffer_wt;
		rp = snd_card->tts_buffer_rd;
		len = snd_card->tts_buffer_size;
		srcptr = snd_card->tts_buffer_vir;

		if (wp < rp) {
			valid = (wp + len - rp);
		} else {
			valid = (wp - rp);
		}
		
		if (valid < TTS_FRM_SAMPLE*2) {
			msleep_interruptible(10); 
			continue;
		}

		if(len-rp >= TTS_FRM_SAMPLE*2) {
			memcpy((void *) tts_buffer, (void *)(srcptr + rp), TTS_FRM_SAMPLE*2);
		} else {
			memcpy((void *) tts_buffer, (void *)(srcptr + rp), len-rp);
			memcpy((void *) ((u8 *)tts_buffer+len-rp), (void *)(srcptr), TTS_FRM_SAMPLE*2-(len-rp));
		}

		for(i=0; i<TTS_FRM_SAMPLE; i++) {
			snd_card->tts_temp_vir[i] = tts_buffer[i] & 0xffff;
		}

		if(snd_card->aout_init == 0) {
			memset(&data_pkt, 0, sizeof(AUD_SND_DATA_PKT));
			data_pkt.bitDepth = 32;
			data_pkt.channel = 2;
			data_pkt.frameSample = 1024;
			data_pkt.sampleRate = 48000;
			if (MT_SUCCESS != drv_snd_aout_config(snd_card, &data_pkt)) {
				pr_err("tts init aout failed!\n");
			}
				
			if (MT_SUCCESS != drv_snd_hdmi_config(snd_card, &data_pkt)) {
				pr_err("tts init hdmi failed!\n");
			}
			snd_reg_DCH_set_pcm_play_stop(0);
		}

		//mt_dcache_flush((phys_addr_t)snd_card->tts_temp_vir, TTS_FRM_SAMPLE*sizeof(mt_u32));
		while(cal_mix_valid_ms() > 100 && wait_cnt <= 5) {
			msleep_interruptible(20); 
			wait_cnt++;
		}
		
		if(wait_cnt > 5)
			pr_err("wait timeout, mix:0x%x!!!\n",snd_reg_d0h_get_mix_func_en());
		
		wait_cnt = 0;

		
		pHwBuff = &snd_card->hwBuff[2];//mix buffer
		write_data_to_hw_buf((void *)(ulong)pHwBuff->base_phy, pHwBuff->wt, pHwBuff->size, 
							(void *)(ulong)snd_card->tts_temp_phy, snd_card->tts_temp_size);
		pHwBuff->wt += snd_card->tts_temp_size;
		pHwBuff->wt %= pHwBuff->size;
		drv_snd_reg_wt(pHwBuff->regWtCmd, (snd_card->tts_temp_size >> 3) | 0x80000000);

		rp = (rp + TTS_FRM_SAMPLE*2) % len;
		snd_card->tts_buffer_rd = rp;
	}

	pr_err("aout tts thread exit\n");
	return 0;
}


static void get_volume_gainq(u8 *p_gainq, u16 *p_scale, u8 volume)
{
	u8 gainQ = 0;
	u32 scale = 0;
	u8 step;

	// percent_l = 0; vol is 0,do nothing; else do something
	if (volume)
	{
		//100~150
		if (volume > SND_VOLUME_MAX_SAFE)
		{
			scale = SND_VOLUME_MAX_VALUE;
			step = SND_VOLUME_MAX - volume;

			while (step--)
			{
				scale *= 1000;
				scale /= SND_VOLUME_STEP_COEF_100_150_50_ARIA_INT;
			}

		}
		else //1~100
		{
			scale = SND_VOLUME_MAX_VALUE_SAFE;
			step = SND_VOLUME_MAX_SAFE - volume;

			while (step--)
			{
				scale *= SND_VOLUME_EF;
				scale /= SND_VOLUME_STEP_COEF_1_100_99_ARIA_INT;
			}
		}

#if 1 //for bug131757: Loudness Matching
		//dolby policy
		if (g_dolby_volume_ctrl)
		{
			//scale = scale * 7096 / 10000; //AUD_VOLUME_DOLBY_COEF: 0.7096
			scale = scale * 9297 / 10000; //30627:for ms12 loudness testcase
		}
#endif

		//get gainq and volume
		while (scale > 0xffff)
		{
			gainQ++;
			scale >>= 1;
		}
	}

	*p_gainq = gainQ;
	*p_scale = (u16)scale;

	return;
}

long set_volume(SND_CARD *snd_card, ulong volume)
{
	mt_u8 gainq = 0;
	mt_u16 scale = 0;

	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	if (volume > SND_VOLUME_MAX_SAFE)
	{
		volume = SND_VOLUME_MAX_SAFE;
	}

	if(snd_card->share_buf) {
		if(HA_AUDIO_ID_DOLBY_PLUS == snd_card->share_buf->adec_info.aud_type ||
			HA_AUDIO_ID_DOLBY_TRUEHD == snd_card->share_buf->adec_info.aud_type ||
			HA_AUDIO_ID_DOLBY_CONVERT == snd_card->share_buf->adec_info.aud_type ) {
				g_dolby_volume_ctrl = 1;
			} else {
				g_dolby_volume_ctrl = 0;
			}
	}

	get_volume_gainq(&gainq, &scale, volume);

	snd_reg_08H_set_volume_scale(scale);
	if(snd_card->share_buf) {
		if(HA_AUDIO_ID_DTSHD == snd_card->share_buf->adec_info.aud_type ||
			(1 == g_hdmi_mat)) {
			snd_reg_08H_set_volume_scale(0x8000);
			snd_reg_08H_set_adac_2ch_mute(1);
		}
	}

	snd_reg_08H_set_gainq(gainq);

	return 0;
}

static int get_volume(SND_CARD *snd_card, void *arg)
{
	int ret = 0;
	ulong volume = 0;

	if ((NULL == snd_card) || (NULL == arg))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	volume = (ulong)snd_card->volume;
	if (0 != copy_to_user((void *)arg, &volume, sizeof(ulong)))
	{
		ret = MT_FAILURE;
	}

	return ret;
}

static int set_mute(SND_CARD *snd_card, void *arg)
{
	snd_mute_parame_t mute_param;

	if ((NULL == snd_card) || (NULL == arg))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	if (0 != copy_from_user((void*)&mute_param, (void*)arg, sizeof(snd_mute_parame_t)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_from_user error!\n");
		return -1;
	}

	if (MT_UNF_SND_OUTPUTPORT_ALL == mute_param.port)
	{
		if (mute_param.mute)
		{
			set_volume(snd_card, 0);
			snd_reg_08H_set_spdif_hdmi_mute(1);
			snd_reg_08H_set_spdif_coax_mute(1);
		}
		else
		{
			set_volume(snd_card, snd_card->volume);
			snd_reg_08H_set_spdif_hdmi_mute(0);
			snd_reg_08H_set_spdif_coax_mute(0);
		}

		snd_card->hdmi_mute = mute_param.mute;
		snd_card->spdif_mute = mute_param.mute;
		snd_card->adac_mute = mute_param.mute;
	}
	else if (MT_UNF_SND_OUTPUTPORT_HDMI0 == mute_param.port)
	{
		if (mute_param.mute)
		{
			snd_reg_08H_set_spdif_hdmi_mute(1);
		}
		else
		{
			snd_reg_08H_set_spdif_hdmi_mute(0);
		}

		snd_card->hdmi_mute = mute_param.mute;
	}
	else if (MT_UNF_SND_OUTPUTPORT_SPDIF0 == mute_param.port)
	{
		if (mute_param.mute)
		{
			snd_reg_08H_set_spdif_coax_mute(1);
		}
		else
		{
			snd_reg_08H_set_spdif_coax_mute(0);
		}

		snd_card->spdif_mute = mute_param.mute;
	}
	else //MT_UNF_SND_OUTPUTPORT_DAC0
	{
		if (mute_param.mute)
		{
			//drv_gpio_set_value(GPIO_7, GPIO_VALUE_LOW_LEVEL);
			set_volume(snd_card, 0);
		}
		else
		{
			set_volume(snd_card, snd_card->volume);
			//drv_gpio_set_value(GPIO_7, GPIO_VALUE_HIGH_LEVEL);
		}
		snd_card->adac_mute = mute_param.mute;
	}

	return 0;
}

static int get_mute(SND_CARD *snd_card, void *param)
{
	int ret = 0;
	snd_mute_parame_t mute_param;
	if ((NULL == snd_card) || (NULL == param))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	if (0 != copy_from_user((void*)&mute_param, (void*)param, sizeof(snd_mute_parame_t)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_from_user error!\n");
		return -1;
	}

	if (MT_UNF_SND_OUTPUTPORT_HDMI0 == mute_param.port)
	{
		mute_param.mute = snd_card->hdmi_mute;
	}
	else if (MT_UNF_SND_OUTPUTPORT_SPDIF0 == mute_param.port)
	{
		mute_param.mute = snd_card->spdif_mute;
	}
	else //MT_UNF_SND_OUTPUTPORT_DAC0
	{
		mute_param.mute = snd_card->adac_mute;
	}

	if (0 != copy_to_user((void*)param, (void*)&mute_param, sizeof(snd_mute_parame_t)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}

	return ret;
}

static int set_hdmi_mode(SND_CARD *snd_card, MT_UNF_SND_HDMI_MODE_E mode)
{
	if (NULL == snd_card || MT_UNF_SND_HDMI_MODE_LPCM > mode || MT_UNF_SND_HDMI_MODE_BUTT <= mode)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	if(MT_UNF_SND_HDMI_MODE_FORCE_DD == mode)
		return -1;
	
	snd_card->user_hdmi_mode = mode;
	if (SND_CARD_STA_STOP != snd_card->sta && SND_CARD_STA_IDLE != snd_card->sta) {
		if(HA_AUDIO_ID_DOLBY_PLUS != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_TRUEHD != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_CONVERT != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_AC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_EAC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_AC4 != snd_card->share_buf->adec_info.aud_type) {
			if(MT_UNF_SND_HDMI_MODE_LPCM != mode)
				return 0;
		}
	}

	if (snd_card->hdmi_mode != mode)
	{
		snd_card->hdmi_mode = mode;
		snd_card->aout_need_cfg = 1;
		snd_card->hdmi_need_cfg = 1;
		snd_card->hdmi_cfg_retry = 0;
	}

	return 0;
}

static int get_hdmi_mode(SND_CARD *snd_card, MT_UNF_SND_HDMI_MODE_E *mode)
{
	ulong hdmi_mode = 0;
	if ((NULL == snd_card) || (NULL == mode))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%p/%p]\n", snd_card, mode);
		return -1;
	}

	hdmi_mode = (ulong)snd_card->hdmi_mode;
	if (0 != copy_to_user((void*)mode, (void*)&hdmi_mode, sizeof(MT_UNF_SND_HDMI_MODE_E)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}
	return 0;
}

static int set_spdif_mode(SND_CARD *snd_card, MT_UNF_SND_SPDIF_MODE_E mode)
{
	if (NULL == snd_card || MT_UNF_SND_SPDIF_MODE_LPCM > mode || MT_UNF_SND_SPDIF_MODE_BUTT <= mode)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	snd_card->user_spdif_mode = mode;
	if (SND_CARD_STA_STOP != snd_card->sta && SND_CARD_STA_IDLE != snd_card->sta) {
		if(HA_AUDIO_ID_DOLBY_PLUS != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_TRUEHD != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_CONVERT != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_AC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_EAC3PASSTHROUGH != snd_card->share_buf->adec_info.aud_type &&
			HA_AUDIO_ID_DOLBY_AC4 != snd_card->share_buf->adec_info.aud_type) {
			if(MT_UNF_SND_SPDIF_MODE_LPCM != mode)
				return 0;
		}
	}

	if (snd_card->spdif_mode != mode)
	{
		snd_card->spdif_mode = mode;
		snd_card->aout_need_cfg = 1;
		snd_card->hdmi_need_cfg = 1;
		snd_card->hdmi_cfg_retry = 0;
	}

	return 0;
}

static int get_spdif_mode(SND_CARD *snd_card, MT_UNF_SND_SPDIF_MODE_E *mode)
{
	ulong spdif_mode = 0;
	if ((NULL == snd_card) || (NULL == mode))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%p/%p]\n", snd_card, mode);
		return -1;
	}

	spdif_mode = (ulong)snd_card->spdif_mode;
	if (0 != copy_to_user((void*)mode, (void*)&spdif_mode, sizeof(MT_UNF_SND_SPDIF_MODE_E)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}

	return 0;
}

static int release_crc_dbg_buf(SND_CARD *snd_card)
{
	aud_share_buf_t *share_buf = NULL;
	
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%p]\n", snd_card);
		return -1;
	}

	if (0 != snd_card->crc_buf_size)
	{
		mt_drv_mmz_unmap(&snd_card->crc_buf_mmz);
		mt_drv_mmz_release(&snd_card->crc_buf_mmz);
		snd_card->crc_buf_size = 0;
		snd_card->crc_buf_phy = 0;
		snd_card->crc_buf_vir = NULL;

		if(MT_SUCCESS == get_sharebuf_av(&share_buf))
			share_buf->crc_enable = 0;
		if(MT_SUCCESS == get_sharebuf_ta(&share_buf))
			share_buf->crc_enable = 0;
	}

	return 0;
}

static int alloc_crc_dbg_buf(SND_CARD *snd_card, ulong size)
{
	int ret;
	aud_share_buf_t *share_buf = NULL;

	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%p]\n", snd_card);
		return -1;
	}

	if (0 != snd_card->crc_buf_size)
	{
		release_crc_dbg_buf(snd_card);
	}

	if (0 == size)
	{
		return 0;
	}

	ret = mt_drv_mmz_alloc("crc_buf", MMZ_ZONE_DDR, size, SND_BUFF_ADDR_ALIGN, &snd_card->crc_buf_mmz);
	if (MT_SUCCESS != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "crc_buf mt_drv_mmz_alloc error\n");
		return -1;
	}

	ret = mt_drv_mmz_map_cache(&snd_card->crc_buf_mmz);
	if (MT_SUCCESS != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "crc_buf mt_drv_mmz_map_cache error\n");
		return -1;
	}

	snd_card->crc_buf_size = size;
	snd_card->crc_buf_phy = snd_card->crc_buf_mmz.startPhyAddr;
	snd_card->crc_buf_vir = (u32 *)snd_card->crc_buf_mmz.startVirAddr;

	if(MT_SUCCESS == get_sharebuf_av(&share_buf))
		share_buf->crc_enable = 1;
	if(MT_SUCCESS == get_sharebuf_ta(&share_buf))
		share_buf->crc_enable = 1;
	return 0;
}

void AO_TRACK_FillAudioCiTestInfo(MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo)
{
	SND_CARD *snd_card = &gst_snd_card[0];

	if ((NULL == snd_card) || (NULL == pstInfo))
	{
		return;
	}

	pstInfo->AMute = snd_card->aud_ci_test_info.AMute;
	pstInfo->AVolume = snd_card->aud_ci_test_info.AVolume;
	pstInfo->ADecFrmCnt = snd_card->aud_ci_test_info.ADecFrmCnt;
	pstInfo->AErrFrmCnt = snd_card->aud_ci_test_info.AErrFrmCnt;
	pstInfo->ADropFrmCnt = snd_card->aud_ci_test_info.ADropFrmCnt;
	pstInfo->ADescriptionFrmCnt = snd_card->aud_ci_test_info.ADescriptionFrmCnt;
}
EXPORT_SYMBOL(AO_TRACK_FillAudioCiTestInfo);

mt_s32 AO_Track_GetAudioStreamInfo(MT_HA_AUDIO_STREAM_INFO_S *pInfo)
{
	SND_CARD *snd_card = &gst_snd_card[0];

	if ((NULL == snd_card) || (NULL == pInfo))
	{
		return -1;
	}

	if(snd_card->share_buf)
		snd_card->stream_info.aud_type = snd_card->share_buf->adec_info.aud_type;

	memcpy(pInfo, &snd_card->stream_info, sizeof(MT_HA_AUDIO_STREAM_INFO_S));

	if(g_hdmi_mat == 1)
		pInfo->hdmi_mode = MT_UNF_SND_HDMI_MODE_MAT;
 
	return 0;
}
EXPORT_SYMBOL(AO_Track_GetAudioStreamInfo);

static int get_snd_attr(SND_CARD *snd_card, void *param)
{
	MT_UNF_AUDIOTRACK_ATTR_S snd_attr = {0};

	if ((NULL == snd_card) || (NULL == param))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%lx/%lx]\n", (ulong)snd_card, (ulong)param);
		return -1;
	}

	snd_attr.u32DebugCrcSize = snd_card->crc_buf_size;
	snd_attr.u32DebugCrc_phy = snd_card->crc_buf_phy;
	snd_attr.u32DebugCrc_vir = (ulong)snd_card->crc_buf_vir;

	snd_attr.dolby_dd_ddp = snd_card->stream_info.dolby_dd_ddp;
	snd_attr.dolby_dualmono = snd_card->stream_info.dolby_dualmono;
	
	if (0 != copy_to_user((void*)param, (void*)&snd_attr, sizeof(MT_UNF_AUDIOTRACK_ATTR_S)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}

	return 0;
}

static int set_track_mode(SND_CARD *snd_card, MT_UNF_TRACK_MODE_E track_mode)
{
	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%lx]\n", (ulong)snd_card);
		return -1;
	}

	if (snd_card->track_mode != track_mode)
	{
		snd_card->track_mode = track_mode;
		downmix_config(snd_card, track_mode);
	}

	return 0;
}

static int get_track_mode(SND_CARD *snd_card, MT_UNF_TRACK_MODE_E *track_mode)
{
	if ((NULL == snd_card) || (NULL == track_mode))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%lx/%lx]\n", (ulong)snd_card, (ulong)track_mode);
		return -1;
	}

	if (0 != copy_to_user((void*)track_mode, (void*)&snd_card->track_mode, sizeof(MT_UNF_TRACK_MODE_E)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}

	return 0;
}

static int get_citest_info(SND_CARD *snd_card, void *param)
{
	MT_UNF_AVPLAY_CI_TEST_INFO_S ci_test_info;
	
	if ((NULL == snd_card) || (NULL == param))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter![%lx/%lx]\n", (ulong)snd_card, (ulong)param);
		return -1;
	}
	
	memset(&ci_test_info, 0, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S));
	if (0 != copy_from_user((void*)&ci_test_info, (void*)param, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S))) {
		SND_LOG(SND_LOG_POS_ERR, "copy_from_user error!\n");
		return -1;
	}

	snd_card->aud_ci_test_info.ASndPcmBufCnt = drv_snd_reg_rd(REG_AUD_OUT_BUF_CNT) << 3;
	snd_card->aud_ci_test_info.ASndPlayStatus = snd_reg_DCH_get_pcm_play_stop();

	ci_test_info.ASndPcmBufCnt = snd_card->aud_ci_test_info.ASndPcmBufCnt;
	ci_test_info.ASndPlayStatus = snd_card->aud_ci_test_info.ASndPlayStatus;
	if (0 != copy_to_user((void*)param, (void*)&ci_test_info, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
		return -1;
	}

	return 0;
}

static int set_fader(SND_CARD *snd_card, MT_UNF_SND_FADER_ATTR_S *fader_attr)
{
	MT_UNF_SND_FADER_ATTR_S fader_cfg;

	if ((NULL == snd_card) || (NULL == fader_attr))
	{
		SND_LOG(SND_LOG_POS_ERR, "invalid parameter!\n");
		return -1;
	}

	if (0 != copy_from_user((void*)&fader_cfg, (void*)fader_attr, sizeof(MT_UNF_SND_FADER_ATTR_S)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_from_user error!\n");
		return -1;
	}

	snd_reg_5CH_set_fader_en(fader_cfg.fader_en);
	snd_reg_5CH_set_fader_in(fader_cfg.fade_in);
	
	if(0 < fader_cfg.fade_step && 3 >= fader_cfg.fade_step)
		snd_reg_5CH_set_fader_step(fader_cfg.fade_step);
	
	if(0 < fader_cfg.target_gain && 0xfff >= fader_cfg.target_gain)
		snd_reg_5CH_set_fader_target_gain(fader_cfg.target_gain);
	
	if(0 < fader_cfg.time_step && 0xffff >= fader_cfg.time_step)
		snd_reg_5CH_set_fader_time_step(fader_cfg.time_step);

	return 0;
}

static void get_snd_buf_info(MT_UNF_SND_BUF_INFO_S *buf_info)
{
	u32 pp_size = drv_snd_reg_rd(REG_AUD_PP_BUF_LEN) << 3;
	u32 pcm_size = drv_snd_reg_rd(REG_AUD_OUT_BUF_LEN) << 3;
	u32 pp_valid = drv_snd_reg_rd(REG_AUD_PP_BUF0_CNT) << 3;
	u32 pcm_valid = drv_snd_reg_rd(REG_AUD_OUT_BUF_CNT) << 3;
	MT_UNF_SND_BUF_INFO_S snd_buf_info;

	if (NULL == buf_info)
	{
		SND_LOG(SND_LOG_POS_ERR, "get_snd_buf_info invalid parameter!\n");
		return;
	}

	memset(&snd_buf_info, 0, sizeof(MT_UNF_SND_BUF_INFO_S));
	snd_buf_info.pcm_size = pp_size + pcm_size;
	snd_buf_info.pcm_valid = pp_valid + pcm_valid;

	snd_buf_info.spd_size = drv_snd_reg_rd(REG_AUD_SPD_BUF_LEN) << 3;
	snd_buf_info.spd_valid = drv_snd_reg_rd(REG_AUD_SPD_BUF_CNT) << 3;

	if (0 != copy_to_user((void*)buf_info, (void*)&snd_buf_info, sizeof(MT_UNF_SND_BUF_INFO_S)))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy_to_user error!\n");
	}
}

static int start_tts(SND_CARD *snd_card)
{
    if(NULL == snd_card){
        return MT_FAILURE;
    }

	snd_reg_d0h_set_mix_func_en(1);
	g_aout_tts_stop = MT_FALSE;
	pr_err("start tts!!!!!!!!\n");
	
	return MT_SUCCESS;
}

static int stop_tts(SND_CARD *snd_card)
{
	mt_u8 wait_cnt = 0;
	
    if(NULL == snd_card){
        return MT_FAILURE;
    }

	g_aout_tts_state = 0;
	g_aout_tts_stop = MT_TRUE;
	//while(reg_sym_linux_reg_get_mix_en_bit() == 1 && wait_cnt < 10) {
	while(g_aout_tts_state == 0 && wait_cnt < 10) {
		msleep_interruptible(5);
		wait_cnt++;
	}
	pr_err("stop tts!!!!!!!!\n");

	return MT_SUCCESS;
}

static long drv_snd_ioctl(struct file *file, unsigned int cmd, unsigned long param)
{
	//mt_u32 idx = 0;
	long ret = MT_FAILURE;
	AUD_SND_DATA_PKT data_pkt;
	//AUD_SND_INIT_PARAM init_param;
	SND_CARD *snd_card = (SND_CARD *)file->private_data;

	ret = down_interruptible(&g_drv_snd_mutex);
	if (!snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "filp->private_data is NULL\n");
		up(&g_drv_snd_mutex);
		return MT_FAILURE;
	}
	
	SND_GET_SHARE_PKT();
	//command implement
	switch (cmd)
	{
	case SND_SEND_DATA:
		memset(&data_pkt, 0, sizeof(AUD_SND_DATA_PKT));
		if (0 != copy_from_user((void*)&data_pkt, (void*)param, sizeof(AUD_SND_DATA_PKT)))
		{
			break;
		}

		ret = drv_snd_sw_buff_write(snd_card, &data_pkt);
		break;
	case SND_SET_VOL:
		ret = set_volume(snd_card, param);
		snd_card->volume = (u8)param;
		break;
	case SND_GET_VOL:
		ret = get_volume(snd_card, (void *)param);
		break;
	case SND_SET_MUTE:
		ret = set_mute(snd_card, (void *)param);
		break;
	case SND_GET_MUTE:
		ret = get_mute(snd_card, (void *)param);
		break;
	case SND_INIT:
		ret = 0;
		break;
	case SND_UNINIT:
#if 0
		ret = drv_snd_uninit(snd_card);
#else
		ret = 0;
#endif
		break;
	case SND_START:
		ret = drv_snd_start(snd_card);
		break;
	case SND_STOP:
		ret = drv_snd_stop(snd_card);
		break;
	case SND_PAUSE:
		ret = drv_snd_pause(snd_card);
		break;
	case SND_RESUME:
		ret = drv_snd_resume(snd_card);
		break;
	case SND_FLUSH:
		ret = drv_snd_flush(snd_card);
		break;
	case SND_RESET:
		break;
	case SND_ADAC_ONOFF:
		ret = board_adac_onoff(param);
		break;
	case SND_SET_HDMI_MODE:
		ret = set_hdmi_mode(snd_card, (MT_UNF_SND_HDMI_MODE_E)param);
		break;
	case SND_GET_HDMI_MODE:
		ret = get_hdmi_mode(snd_card, (MT_UNF_SND_HDMI_MODE_E *)param);
		break;
	case SND_SET_SPDIF_MODE:
		ret = set_spdif_mode(snd_card, (MT_UNF_SND_SPDIF_MODE_E)param);
		break;
	case SND_GET_SPDIF_MODE:
		ret = get_spdif_mode(snd_card, (MT_UNF_SND_SPDIF_MODE_E *)param);
		break;
	case SND_SET_CRC_DBG_BUF_SIZE:
		ret = alloc_crc_dbg_buf(snd_card, param);
		break;
	case SND_GET_ATTR:
		ret = get_snd_attr(snd_card, (void *)param);
		break;
	case SND_SET_TRACK_MODE:
		ret = set_track_mode(snd_card, (MT_UNF_TRACK_MODE_E)param);
		break;
	case SND_GET_TRACK_MODE:
		ret = get_track_mode(snd_card, (MT_UNF_TRACK_MODE_E *)param);
		break;
	case SND_GET_CITEST_INFO:
		ret = get_citest_info(snd_card, (void *)param);
		break;
	case SND_SET_EOS_FLAG:
		snd_card->eos = (u32)param;
		ret = 0;
		break;
#ifdef AOUT_FPGA_TEST
	case SND_CFG_HDMI:
	{
		aud_param_t aud_param;
		if (0 != copy_from_user((void*)&aud_param, (void*)param, sizeof(aud_param_t)))
		{
			pr_err("<%s:%d> copy_from_user error!\n", __func__, __LINE__);
			break;
		}
		ret = fpga_test_hdmi_config(&aud_param);
		break;
	}
#endif
	case SND_SET_FADER:
		ret = set_fader(snd_card, (MT_UNF_SND_FADER_ATTR_S *)param);
		break;
	case SND_GET_BUF_INFO:
		get_snd_buf_info((MT_UNF_SND_BUF_INFO_S *)param);
		break;
	case SND_START_TTS:
		start_tts(snd_card);
		break;
	case SND_STOP_TTS:
		stop_tts(snd_card);
		break;
	default:
		SND_LOG(SND_LOG_POS_ERR, "Invalid cmd[%#x]arg[%#lx]\n", cmd, param);
		break;
	}

	up(&g_drv_snd_mutex);
	//SND_LOG(SND_LOG_POS_FLOW, "DRV SND ioctl exit\n");
	return ret;
}

SND_CARD *g_snd_card_muti_open = NULL;

static int drv_snd_open(struct inode * inode, struct file * filp)
{
	mt_u32 idx = 0;
	mt_s32 ret = MT_FAILURE;
	AUD_SND_INIT_PARAM param;
	SND_CARD *snd_card = &gst_snd_card[idx];
	//SND_PRIV_DATA *pPrivData = NULL;

	if (atomic_inc_return(&atm_open_cnt) > 1)
	{
		SND_LOG(SND_LOG_POS_ERR, "open snd multiple times:%d\n", atomic_read(&atm_open_cnt));
		filp->private_data = (void *)g_snd_card_muti_open;
		return 0;
	}

	ret = down_interruptible(&g_drv_snd_mutex);

	//snd card index
	for (idx = 0; idx < DRV_SND_CARD_NUM; idx++)
	{
		if (SND_CARD_STA_IDLE == gst_snd_card[idx].sta)
		{
			break;
		}
	}
	if (DRV_SND_CARD_NUM <= idx)
	{
		SND_LOG(SND_LOG_POS_ERR, "snd device open too much, total: %d\n", DRV_SND_CARD_NUM);
		up(&g_drv_snd_mutex);
		return MT_FAILURE;
	}

	//snd card init
	snd_card = &gst_snd_card[idx];
	memset(snd_card, 0, sizeof(SND_CARD));

	SND_GET_SHARE_PKT();
	
	snd_card->idx = idx;
	snd_card->sta = SND_CARD_STA_STOP;
	snd_card->type = SND_CARD_TYPE_NULL;
	filp->private_data = (void *)snd_card;

	param.type = SND_CARD_TYPE_MASTER;
	drv_snd_init(snd_card, &param);

	snd_card->track_mode = MT_UNF_TRACK_MODE_STEREO;
	g_snd_card_muti_open = snd_card;

	up(&g_drv_snd_mutex);
	SND_LOG(SND_LOG_POS_FLOW, "DRV SND[%d] open SUCCESS\n", snd_card->idx);
	return 0;
}

static int drv_snd_release(struct inode * inode, struct file * filp)
{
	int ret = 0;
	SND_CARD *snd_card = (SND_CARD *)filp->private_data;

	if (NULL == snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "filp->private_data is NULL\n");
		return MT_FAILURE;
	}

	if (atomic_dec_return(&atm_open_cnt) != 0)
	{
		SND_LOG(SND_LOG_POS_ERR, "snd atm_open_cnt is not 0 ! \n");
		return 0;
	}

	ret = down_interruptible(&g_drv_snd_mutex);
	if (!snd_card)
	{
		SND_LOG(SND_LOG_POS_ERR, "filp->private_data is NULL\n");
		up(&g_drv_snd_mutex);
		return MT_FAILURE;
	}

	//uninit
	drv_snd_uninit(snd_card);

	release_crc_dbg_buf(snd_card);
	memset((void *)snd_card, 0, sizeof(SND_CARD));
	g_snd_card_muti_open = NULL;

	up(&g_drv_snd_mutex);
	SND_LOG(SND_LOG_POS_FLOW, "DRV SND release SUCCESS\n");
	return MT_SUCCESS;
}

static mt_s32 snd_drv_suspend(basedev_s *pdev, pm_message_t state)
{
	SND_CARD *snd_card = &gst_snd_card[0];

	//for eaa no output
	snd_card->aout_init = 0;
	snd_card->aout_need_cfg = 1;
	board_adac_onoff(0);
	
    return MT_SUCCESS;
}

static mt_s32 snd_drv_resume(basedev_s *pdev)
{
	board_adac_onoff(1);
    return MT_SUCCESS;
}

ssize_t drv_snd_write(struct file *filp, const char __user *buffer, size_t count, loff_t* ppos)
{
	mt_u32 rd, wt, size, free = 0;
	mt_u8 *dst = NULL;
	mt_u32 copylen = 0;
	SND_CARD *snd_card = (SND_CARD *)filp->private_data;

	if (NULL == snd_card) {
		SND_LOG(SND_LOG_POS_ERR, "filp->private_data is NULL\n");
		return MT_FAILURE;
	}

	rd = snd_card->tts_buffer_rd;
	wt = snd_card->tts_buffer_wt;
	size = snd_card->tts_buffer_size;
	dst = snd_card->tts_buffer_vir;
	
	if(wt < rd) {
		free = rd - wt;
	} else {
		free = size - wt + rd;
	}
	
	if(count >= free)
		return 0;
	
    if(wt + count > size) {
        copylen = size - wt;
        copy_from_user(dst+wt, buffer, copylen);
        copy_from_user(dst, buffer+copylen, count-copylen);        
    } else {
        copy_from_user(dst + wt, buffer, count);
    }
    wt = (wt + count)%size;
	snd_card->tts_buffer_wt = wt;

	return count;
}

/*****************************************************************************
[DEVICE & PROC]
******************************************************************************/

static struct file_operations gst_snd_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl   = drv_snd_ioctl,
	.open             = drv_snd_open,
	.release          = drv_snd_release,
	.write            = drv_snd_write,
};

static baseops_s gst_snd_drvops =
{
	.probe        = NULL,
	.remove       = NULL,
	.shutdown     = NULL,
	.prepare      = NULL,
	.complete     = NULL,
	.suspend      = snd_drv_suspend,
	.suspend_late = NULL,
	.resume_early = NULL,
	.resume       = snd_drv_resume,
};

static __inline__ int  drv_snd_device_register(void)
{
	/*register snd device*/
	snprintf(gst_snd_dev.devfs_name, sizeof(gst_snd_dev.devfs_name), DRV_SND_DEV_NAME);
	gst_snd_dev.fops   = &gst_snd_fops;
	gst_snd_dev.minor  = DRV_SND_MIN_MINOR;
	gst_snd_dev.owner  = THIS_MODULE;
	gst_snd_dev.drvops = &gst_snd_drvops;
	gst_snd_dev.priv = &gst_snd_priv_data;

	if (mt_drv_dev_register(&gst_snd_dev) < 0)
	{
		SND_LOG(SND_LOG_POS_ERR, "DRV SND device register FAIL\n");
		return MT_FAILURE;
	}

	SND_LOG(SND_LOG_POS_FLOW, "DRV SND device register SUCCESS\n");
	return MT_SUCCESS;
}

static __inline__ void drv_snd_device_unregister(void)
{
	/*unregister snd device*/
	mt_drv_dev_unregister(&gst_snd_dev);
}

static void drv_snd_get_percent(mt_u32 size, mt_u32 rd, mt_u32 wt, mt_u8 *integer, mt_u8 *decimal)
{
	mt_u32 tmp = 0;

	if (wt >= rd)
	{
		tmp =  wt - rd;
	}
	else
	{
		tmp =  size - rd + wt;
	}
	*integer = tmp * 100 / size;
	*decimal = (tmp * 10000 / size) % 100;
}

static mt_s32 drv_snd_proc_read(struct seq_file* p, mt_void* v)
{
	mt_u8 integer = 0;
	mt_u8 decimal = 0;
	mt_u32 idx = 0;
	mt_u32 tmp = 0;
	SND_BUFF *pBuff = NULL;
	SND_BUFF *sw_pBuff = NULL;
	SND_CARD *snd_card = &gst_snd_card[0];
	AUD_SND_DATA_PKT_QUE *pPkt = snd_card->pPkt;
	char *sw_idx_str[3] = {"0", "1", "2"};

	//SND_BUFF *pSwBuff = &pCard->swBuff[0];
	//SND_BUFF *phwBuff = &pCard->hwBuff[1];
	char swBuffName[AUD_SND_SW_BUFF_NUM][4] = {"SW0", "SW1", "SW2"};
	char hwBuffName[AUD_SND_HW_BUFF_NUM][4] = {" PP", "PCM", "MIX", "SPD"};
	char dataFormat[AUD_SND_DATA_FMT_INVALID][6] = {"NULL", " PCM", " AC3", "EAC3", " DTS", "DTSHD", "MAT"};
	char hdmimode[MT_UNF_SND_HDMI_MODE_BUTT][5] = {"PCM", "RAW", "HBR", "AUTO", "FDD"};
	char spdmode[MT_UNF_SND_SPDIF_MODE_BUTT][4] = {"PCM", "RAW"};
	SND_GET_SHARE_PKT();

	//seq_printf(p, TEXT_COLOR_RED "\n------------------------------------- SND State ------------------------------------\n\n" TEXT_COLOR_END);
	if(pPkt == NULL)
	{
		return 0;
	}
	//pkt info
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE1"\n" CONTENT_PKT_INFO"\n" ITEM_BTM_LINE1"\n" TEXT_COLOR_END);
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END);
	drv_snd_get_percent(pPkt->num, pPkt->rd, pPkt->wt, &integer, &decimal);
	seq_printf(p, "   %-8.4d  %-7.5x  %-7.5x  %-8.5x  %2d.%02d%% ",
	           pPkt->num, pPkt->rd, pPkt->wt, pPkt->wt_cnt, integer, decimal);
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");

	//sw buff
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE2"\n" CONTENT_SW_BUFF"\n" ITEM_BTM_LINE2"\n" TEXT_COLOR_END);
	for (idx = 0; idx < AUD_SND_SW_BUFF_NUM; idx++)
	{
		sw_pBuff = snd_card->swBuff[idx];
		drv_snd_get_percent(sw_pBuff->size, sw_pBuff->rd, sw_pBuff->wt, &integer, &decimal);
		seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END);
		seq_printf(p, "    %-7.3s  %-7.5x  %-7.5x  %-7.5x  %-8.5x %-10.8x  %2d.%02d%% ",
		           &swBuffName[idx][0], sw_pBuff->size, sw_pBuff->rd, sw_pBuff->wt, sw_pBuff->wtCnt, sw_pBuff->wtByte, integer, decimal);
		seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	}

	//hw buff
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3"\n" CONTENT_HW_BUFF"\n" ITEM_BTM_LINE3"\n" TEXT_COLOR_END);
	for (idx = 0; idx < AUD_SND_HW_BUFF_NUM; idx++)
	{
		pBuff = &snd_card->hwBuff[idx];
		tmp = drv_snd_reg_rd(regBuffCnt_table[idx]);
		drv_snd_get_percent(pBuff->size >> 3, 0, tmp, &integer, &decimal);
		seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END);
		seq_printf(p, "    %-7.3s  %-7.5x  %-7.5x  %-8.5x %-10.8x   %-5.1s  %-8.5x  %2d.%02d%% ",
		           &hwBuffName[idx][0], pBuff->size, pBuff->wt, pBuff->wtCnt, pBuff->wtByte,
		           AUD_SND_SW_BUFF_NUM > pBuff->swBuffIdx ? sw_idx_str[pBuff->swBuffIdx] : "*", tmp, integer, decimal);
		seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	}
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n\n");

	//snd info
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3"\n" TEXT_COLOR_END);

	tmp = drv_snd_reg_rd(REG_AUD_VOL_CFG);
	seq_printf(p, TEXT_COLOR_PINK "| State: " TEXT_COLOR_END);
	seq_printf(p, "%-4.4s ", (0x1 & drv_snd_reg_rd(REG_AUD_PLAY_STOP)) ? "STOP" : "PLAY");
	
	seq_printf(p, TEXT_COLOR_PINK "| SND_State: " TEXT_COLOR_END);
	seq_printf(p, "%d ", snd_card->sta);

	seq_printf(p, TEXT_COLOR_PINK "| Volme(Gain/Scale): " TEXT_COLOR_END);
	seq_printf(p, "%-2.2x / %-4.4x ", (0x7 & (tmp >> 16)), (0xffff & tmp));
	seq_printf(p, TEXT_COLOR_PINK "| Apts: " TEXT_COLOR_END);
	seq_printf(p, "%-8.8x ", snd_card->stCfg.apts);
	seq_printf(p, TEXT_COLOR_PINK "| FrmIdx: " TEXT_COLOR_END);
	seq_printf(p, "%-5.5x ", snd_card->stCfg.frmIdx);
	seq_printf(p, TEXT_COLOR_PINK "|" "\n" ITEM_BTM_LINE3 "\n" TEXT_COLOR_END);

	tmp = drv_snd_reg_rd(REG_AUD_VOL_CFG);
	seq_printf(p, TEXT_COLOR_PINK "| Mute(Hdmi8Ch/Adac2Ch/SpdCoax/SpdHdmi/I2S): " TEXT_COLOR_END);
	seq_printf(p, "%-2.2x / %1x / %1x / %1x / %1x ",
	           0xff & (tmp >> 24), 0x1 & (tmp >> 23), 0x1 & (tmp >> 22), 0x1 & (tmp >> 21), 0x1 & (tmp >> 20));
	seq_printf(p, TEXT_COLOR_PINK "|    HDMI: " TEXT_COLOR_END);
	if (0 != reg_hdmi_audio_ctrl_ioremap)
	{
		tmp = HAL_GET_U32((volatile u32 *)reg_hdmi_audio_ctrl_ioremap + 0x50);
	}
	seq_printf(p, "%2.2x ", 0xff & (tmp >> 8));
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n");

	seq_printf(p, TEXT_COLOR_PINK "| CON | Samplerate: " TEXT_COLOR_END);
	seq_printf(p, "%6d ", snd_card->stCfg.sampleRate);
	seq_printf(p, TEXT_COLOR_PINK "| Channel: " TEXT_COLOR_END);
	seq_printf(p, "%1x ", snd_card->stCfg.channel);
	seq_printf(p, TEXT_COLOR_PINK "| FrameSamples: " TEXT_COLOR_END);
	seq_printf(p, "%6d ", snd_card->stCfg.frameSample);
	seq_printf(p, TEXT_COLOR_PINK "| BitDepth: " TEXT_COLOR_END);
	seq_printf(p, "%2d ", snd_card->stCfg.bitDepth);
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");

	tmp = snd_card->stCfg.stData[0].fmt;
	seq_printf(p, TEXT_COLOR_PINK "| FIG | Fmt0: " TEXT_COLOR_END);
	seq_printf(p, "%4.4s ", AUD_SND_DATA_FMT_INVALID > tmp ? &dataFormat[tmp][0] : "****");
	tmp = snd_card->stCfg.stData[1].fmt;
	seq_printf(p, TEXT_COLOR_PINK "| Fmt1: " TEXT_COLOR_END);
	seq_printf(p, "%4.4s ", AUD_SND_DATA_FMT_INVALID > tmp ? &dataFormat[tmp][0] : "****");
	tmp = snd_card->stCfg.stData[2].fmt;
	seq_printf(p, TEXT_COLOR_PINK "| Fmt2: " TEXT_COLOR_END);
	seq_printf(p, "%4.4s ", AUD_SND_DATA_FMT_INVALID > tmp ? &dataFormat[tmp][0] : "****");
	seq_printf(p, TEXT_COLOR_PINK "| Interleaved: " TEXT_COLOR_END);
	seq_printf(p, "%2d ", snd_card->stCfg.interleaved);
	seq_printf(p, TEXT_COLOR_PINK "| ValidBit: " TEXT_COLOR_END);
	seq_printf(p, "%2d ", snd_card->stCfg.validBit);
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n");

	tmp = drv_snd_reg_rd(REG_AUD_CH_SRT_CFG);
	seq_printf(p, TEXT_COLOR_PINK "| REG(00H):   " TEXT_COLOR_END);
	seq_printf(p, "%8.8x   ", tmp);
	tmp = drv_snd_reg_rd(REG_AUD_I2S_SPDIF_CFG);
	seq_printf(p, "%8.8x   ", tmp);
	tmp = drv_snd_reg_rd(REG_AUD_VOL_CFG);
	seq_printf(p, "%8.8x   ", tmp);
	tmp = drv_snd_reg_rd(REG_AUD_PP_EN_CFG);
	seq_printf(p, "%8.8x   ", tmp);
	tmp = drv_snd_reg_rd(REG_AUD_CLK_DIV_CFG);
	seq_printf(p, "%8.8x   ", tmp);
	tmp = drv_snd_reg_rd(REG_AUD_CLK_DIV_CFG_2);
	seq_printf(p, "%8.8x ", tmp);
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n");

	tmp = snd_card->hdmi_mode;
	seq_printf(p, TEXT_COLOR_PINK "| HDMI MODE:   " TEXT_COLOR_END);
	seq_printf(p, "%4.4s ", MT_UNF_SND_HDMI_MODE_BUTT > tmp ? &hdmimode[tmp][0] : "****");
	tmp = snd_card->spdif_mode;
	seq_printf(p, TEXT_COLOR_PINK "| SPD MODE:   " TEXT_COLOR_END);
	seq_printf(p, "%4.4s ", MT_UNF_SND_SPDIF_MODE_BUTT > tmp ? &spdmode[tmp][0] : "****");
	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n");

	seq_printf(p, TEXT_COLOR_PINK "| HDMI AFMT: " TEXT_COLOR_END);
	seq_printf(p, "%2d    ", g_hdmi_afmt);

	seq_printf(p, TEXT_COLOR_PINK "| INF: " TEXT_COLOR_END);
	seq_printf(p, "%2d    ", g_hdmi_inf);

	seq_printf(p, TEXT_COLOR_PINK "| RATE: " TEXT_COLOR_END);
	seq_printf(p, "%6d    ", g_hdmi_rate);

	seq_printf(p, TEXT_COLOR_PINK "| CHAN: " TEXT_COLOR_END);
	seq_printf(p, "%2d    ", g_hdmi_chan);

	seq_printf(p, TEXT_COLOR_PINK "| DEPT: " TEXT_COLOR_END);
	seq_printf(p, "%2d    ", g_hdmi_depth);

	seq_printf(p, TEXT_COLOR_PINK "|" TEXT_COLOR_END "\n");
	seq_printf(p, TEXT_COLOR_PINK ITEM_BTM_LINE3 TEXT_COLOR_END "\n\n");

	return MT_SUCCESS;
}

//echo reg //show all aout register value
static void drv_snd_cmd_reg(void)
{
	mt_u32 idx = 0;
	if (0 != reg_aout_base_ioremap)
	{
		printk(KERN_ERR "[AUD]dump aout reg<num=%d>\n", AOUT_REG_NUM);
		for (idx = 0; idx < AOUT_REG_NUM; idx += 4)
		{
			printk(KERN_ERR "[0x%03x]: %08x %08x %08x %08x\n", idx * 4,
			       HAL_GET_U32((volatile u32 *)reg_aout_base_ioremap + idx), HAL_GET_U32((volatile u32 *)reg_aout_base_ioremap + (idx + 1)),
			       HAL_GET_U32((volatile u32 *)reg_aout_base_ioremap + (idx + 2)), HAL_GET_U32((volatile u32 *)reg_aout_base_ioremap + (idx + 3)));
		}
		printk(KERN_ERR "[AUD]dump aout reg END\n");
	}
}

//echo log > /proc/msp/snd	//show log level and help info(default as 0x3)
//echo log 0x******** > /proc/msp/snd	//log level setting and show log level
static void drv_snd_cmd_log(mt_char *pcmd, mt_u8 len)
{
	mt_char *pcCmdLog = "log";
	mt_char	*pcBuf = pcmd;
	mt_char *bufEnd = pcmd + len - 1;

	pcBuf = strstr(pcmd, pcCmdLog);
	pcBuf += strlen(pcCmdLog) - 1;
	do
	{
		pcBuf++;
	}
	while (*pcBuf == ' ' && pcBuf <= bufEnd);
	if (MT_SUCCESS != drv_snd_str2u32(pcBuf, &g_snd_log_level))
	{
		printk(KERN_ERR "snd log level set input ERR: %s\n", pcBuf);
	}
	printk(KERN_ERR "current snd_log_level = [%#x]\n", g_snd_log_level);
}

static void drv_snd_cmd_dbg(mt_char *pcmd, mt_u8 len)
{
	//echo dbg -freerun on / off
	//echo dbg -stat apts / data / ...
	//echo dbg -flushbuff pp / aout /spd / mix
	SND_LOG(SND_LOG_POS_FLOW, "\n");
}


static void drv_snd_cmd_dump(mt_char *pcmd, mt_u8 len)
{
	g_dump_enable = !g_dump_enable;
	if (g_dump_enable)
	{
		if (NULL == g_dump_handle)
		{
			g_dump_handle = mt_drv_dump_create(E_DUMP_TYPE_FILE, "/mnt/snd.bin", 0);
			if (NULL == g_dump_handle)
			{
				pr_err("mt_drv_dump_create error!\n");
				return;
			}
		}
	}
	else
	{
		mt_drv_dump_destroy(g_dump_handle);
		g_dump_handle = NULL;
	}
	pr_err("snd dump enable:%d\n", g_dump_enable);
	SND_LOG(SND_LOG_POS_FLOW, "\n");
}

static mt_s32 drv_snd_proc_write(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
	mt_char szBuf[64];
	mt_char* pcCmdReg = "reg";
	mt_char* pcCmdLog = "log";
	mt_char* pcCmdDbg = "dbg";
	mt_char* pcCmdDump = "dump";

	if (count <= 0)
	{
		SND_LOG(SND_LOG_POS_ERR, "PARAM count is 0\n");
		return MT_FAILURE;
	}
	else if (count >= sizeof(szBuf))
	{
//		SND_LOG(SND_LOG_POS_ERR, "cmd too long, max=%ld\n", sizeof(szBuf));
		return MT_FAILURE;
	}

	memset(szBuf, 0, sizeof(szBuf));
	if (copy_from_user(szBuf, buf, count))
	{
		SND_LOG(SND_LOG_POS_ERR, "copy from user FAIL\n");
		return MT_FAILURE;
	}
	szBuf[strlen(szBuf) - 1] = '\0';
	SND_LOG(SND_LOG_POS_FLOW, "str{ %d: %s }\n", (int)strlen(szBuf), szBuf);

	//sub cmd
	if (strstr(szBuf, pcCmdReg))
	{
		drv_snd_cmd_reg();

	}
	else if (strstr(szBuf, pcCmdLog))
	{
		drv_snd_cmd_log(szBuf, count);

	}
	else if (strstr(szBuf, pcCmdDbg))
	{
		drv_snd_cmd_dbg(szBuf, count);

	}
	else if (strstr(szBuf, pcCmdDump))
	{
		drv_snd_cmd_dump(szBuf, count);
	}

	return count;
}


static SND_PROC_PARAM gst_snd_proc =
{
	.pfnReadProc  = drv_snd_proc_read,
	.pfnWriteProc = drv_snd_proc_write,
};

static mt_s32 drv_snd_proc_add(const mt_char * name, mt_u32 idx, SND_PROC_PARAM * pstParam)
{
	mt_char entryName[16];
	mt_proc_entry_t*  pProcItem;

	if (MT_NULL == pstParam)
	{
		SND_LOG(SND_LOG_POS_ERR, "PARAM pstParam is NULL\n");
		return MT_FAILURE;
	}

	/* Create proc */
	if (!idx)
	{
		snprintf(entryName, sizeof(entryName), "%s", name);
	}
	else
	{
		snprintf(entryName, sizeof(entryName), "%s%01d", name, idx);
	}
	pProcItem = mt_drv_proc_add_module(entryName, MT_NULL, MT_NULL);
	if (!pProcItem)
	{
		SND_LOG(SND_LOG_POS_ERR, "Create %s proc entry FAIL\n", name);
		return MT_FAILURE;
	}

	/* Set functions */
	pProcItem->read  = pstParam->pfnReadProc;
	pProcItem->write = pstParam->pfnWriteProc;

	SND_LOG(SND_LOG_POS_FLOW, "DRV SND proc[%s] add SUCCESS\n", name);
	return MT_SUCCESS;
}

static void drv_snd_proc_remove(const mt_char * name, mt_u32 idx)
{
	mt_char entryName[16];

	if (!idx)
	{
		snprintf(entryName, sizeof(entryName), "%s", name);
	}
	else
	{
		snprintf(entryName, sizeof(entryName), "%s%01d", name, idx);
	}
	mt_drv_proc_rm_module(entryName);
}

irqreturn_t snd_isr(mt_s32 irq, mt_void *dev_id)
{
	mt_u32 audio_reg = 0;

	audio_reg = drv_snd_reg_rd(REG_AUD_INTR_SET);     //AUDIO_INTR_SET
	drv_snd_reg_wt(REG_AUD_INTR_SET, audio_reg);

	//pr_err("snd_isr:%#x\n", audio_reg);
	if(audio_reg & 0x1)
	{
		// do isr
	}

	if(audio_reg & 0x2)      // bb_buf_w_hang_up
	{
		// do isr
	}

	if(audio_reg & 0x4)     // buf_rw_intr
	{
		// do isr
	}

	if(audio_reg & 0x8)    // pcm_fifo_empty
	{
		// do isr
	}

	if(audio_reg & 0x10)    // pcm_fifo_cnt_diff bigger
	{
		// do isr
	}

	// this intr adding in B0
	if(audio_reg & 0x20)
	{

	}

	return IRQ_HANDLED;
}

static atomic_t g_snd_inited = ATOMIC_INIT(0);

int drv_snd_ao_module_init(void)
{
	int ret = MT_FAILURE;

    if (atomic_inc_return(&g_snd_inited) > 1) {
		return 0;
	}

	ret = drv_snd_proc_add(DRV_SND_PROC_NAME, 0, &gst_snd_proc);
	if (MT_SUCCESS != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "SND proc add FAIL\n");
		return MT_FAILURE;
	}

	ret = drv_snd_device_register();
	if (MT_SUCCESS != ret)
	{
		//drv_snd_proc_unregister();
		SND_LOG(SND_LOG_POS_ERR, "SND device register FAIL\n");
		return MT_FAILURE;
	}

	//set_adac_onoff(1);

	atomic_set(&atm_open_cnt, 0);

#ifdef AOUT_FPGA_TEST
	fpga_test_request_irq();
#else
	ret = request_irq((unsigned char)IRQ_AOUT_ID, snd_isr, IRQF_TRIGGER_HIGH | IRQF_SHARED, "snd_out_isr", MT_NULL);
	if (0 != ret)
	{
		SND_LOG(SND_LOG_POS_ERR, "register snd_isr error!\n");
	}
#endif

	SND_LOG(SND_LOG_POS_FLOW, "Module init SUCCESS\n");
	return ret;
}

void drv_snd_ao_module_exit(void)
{
	//snd proc remove
	drv_snd_proc_remove(DRV_SND_PROC_NAME, 0);

	//snd card device unregister
	drv_snd_device_unregister();

	SND_LOG(SND_LOG_POS_FLOW, "Module exit SUCCESS\n");
	AO_LOG_INFO();
}

