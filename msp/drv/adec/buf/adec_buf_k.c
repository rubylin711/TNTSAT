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

#include "mt_cache.h"
#include "mt_type.h"
#include "mt_drv_dma.h"
#include "adec/adec_api.h"
#include "adec/buf/adec_buf_k.h"
#include "adec/adec_k.h"

extern void *audio_dma_memcpy(void *to, const void *from, size_t n, unsigned int ch_id);
static inline void *MT_DRV_ADEC_MEMCPY(void *dest, const void *src, size_t n)
{
	return audio_dma_memcpy(dest, src, n, DMA_CHANNEL_SECURE_AUDIO_0);
}

static int check_es_pkt(adec_es_buf_t *es_buf)
{
	u32 remain, rd, wt;

	if (NULL == es_buf)
	{
		adec_err("invalid parameter! [0x%#lx]\n", (ulong)es_buf);
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
		adec_err("invalid parameter! [0x%#lx/%d]\n", (ulong)es_buf, size);
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
				//pr_err("<ah>es buf loop, wt:%d\n", es_buf->es_wt);
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

int adec_es_write(adec_es_buf_t* es_buf,  phys_addr_t data, int size, uint32_t pts, uint32_t pts_valid)
{
	int ret;
	uint32_t es_rd, es_wt, left_cp, right_cp;
	adec_es_pkt_t *es_pkt;
	char *es_data, *es_data_vir;
	
	if ((NULL == es_buf) || (0 == data) || (size <= 0))
	{
	//	pr_err("<%s:%d> invalid parameter! [0x%p/0x%p/%d]\n", __func__, __LINE__, es_buf, data, size);
		pr_err("<%s:%d> invalid parameter! \n", __func__, __LINE__);
		return -1;
	}

	es_data = (char *)((ulong)es_buf->data_phy);
	es_data_vir = (char *)((ulong)es_buf->data_kern_vir);
	if (NULL == es_data)
	{
		pr_err("<%s:%d> es_data is NULL!\n", __func__, __LINE__);
		return -1;
	}

	ret = check_es_pkt(es_buf);
	if (0 != ret)
	{
		return -1;	// no pkt
	}

	ret = check_es_buf(es_buf, size);
	if (0 != ret)
	{
		return -1;	// no space
	}

	es_rd = es_buf->es_rd;
	es_wt = es_buf->es_wt;
	if (es_wt >= es_rd)
	{
		right_cp = es_buf->es_buf_size - es_wt;
		if (right_cp >= size)
		{
			right_cp = size;
			left_cp = 0;
		}
		else
		{
			left_cp = size - right_cp;
		}

		if (0 == left_cp)
		{
			MT_DRV_ADEC_MEMCPY(es_data + es_wt, (void*)(ulong) data, size);
			//mt_dcache_invalid(es_data_vir + es_wt, size);
			es_buf->es_wt += size;
		}
		else
		{
			MT_DRV_ADEC_MEMCPY(es_data + es_wt, (void*)(ulong)data, right_cp);
			//mt_dcache_invalid(es_data_vir + es_wt, right_cp);

			MT_DRV_ADEC_MEMCPY(es_data, (void*)(ulong)data + right_cp, left_cp);
			//mt_dcache_invalid(es_data_vir, left_cp);
			
			es_buf->es_wt = left_cp;
		}
	}
	else
	{
		MT_DRV_ADEC_MEMCPY(es_data + es_wt, (void*)(ulong)data, size);
		//mt_dcache_invalid(es_data_vir + es_wt, size);
		es_buf->es_wt += size;
	}

	mt_dcache_invalid(es_data_vir, es_buf->es_buf_size);
	es_buf->es_wt %= es_buf->es_buf_size;

	es_pkt = &es_buf->es_pkt[es_buf->pkt_wt];
	es_pkt->pts = pts;
	es_pkt->valid = pts_valid;
	es_pkt->es_rd = es_wt;
	es_pkt->size = size;
	es_buf->pkt_wt++;
	es_buf->pkt_wt %= es_buf->pkt_cnt;
	es_pkt->es_ts = 1; //  0 ES,  1 TS

	//pr_err("<ah-k> es [%d/%d][%d/%d]\n", es_pkt->es_rd, es_wt, es_buf->pkt_wt, es_wt);
	mt_dcache_flush(es_buf, sizeof(adec_es_buf_t));

	return size;
}

int adec_es_buf_data_remain(adec_es_buf_t* es_buf)
{
	int remain_data;
	
	if (NULL == es_buf)
	{
		pr_err("<%s:%d> invalid parameter! [0x%p]\n", __func__, __LINE__, es_buf);
		return -1;
	}

	if (es_buf->es_rd <= es_buf->es_wt)
	{
		remain_data = es_buf->es_wt - es_buf->es_rd;
	}
	else
	{
		remain_data = es_buf->es_buf_size - es_buf->es_rd + es_buf->es_wt;
	}

	return remain_data;
}

int adec_es_buf_clear(adec_es_buf_t* es_buf)
{
	if (NULL == es_buf)
	{
		pr_err("<%s:%d> invalid parameter! [%p]\n", __func__, __LINE__, es_buf);
		return -1;
	}

	es_buf->es_rd = 0;
	es_buf->es_wt = 0;
	es_buf->pkt_rd = 0;
	es_buf->pkt_wt = 0;
	memset(&es_buf->es_pkt, 0, sizeof(es_buf->es_pkt));

	return 0;
}

