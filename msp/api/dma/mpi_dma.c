/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_drv_dma.h"
#include "mt_module_debug.h"

static int g_dma_fd = -1;

mt_s32 mt_dma_open(void)
{
	if (g_dma_fd < 0)
	{
		g_dma_fd = open("/dev/mt_dma" ,O_RDWR);
		if(g_dma_fd < 0)
		{
			MT_ERR_DMA("Fail to open!\n");
			return MT_FAILURE;
		}
	}

	return MT_SUCCESS;
}

void mt_dma_close(void)
{
	if (g_dma_fd >= 0)
	{
		close(g_dma_fd);
		g_dma_fd = -1;
	}
}

mt_s32 mt_dma_start(hal_dma_io_param_t *dma_param)
{
	if (mt_dma_open() != MT_SUCCESS)
		return MT_FAILURE;
	if(dma_param->chn_id == 255)
	{
		return ioctl(g_dma_fd, DMA_IOC_START, dma_param);
	}
	else
	{
		return ioctl(g_dma_fd, DMA_IOC_START_WITH_CH, dma_param);
	}	
}

mt_s32 mt_dma_stop(mt_s32 chn_id)
{
	if (mt_dma_open() != MT_SUCCESS)
		return MT_FAILURE;

	if (chn_id < 0 || chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("invalid ch id(%d)!\n",chn_id);
		return MT_FAILURE;
	}

	return ioctl(g_dma_fd, DMA_IOC_STOP, &chn_id);
}

dma_status_t mt_dma_status(mt_s32 chn_id)
{
	if (mt_dma_open() != MT_SUCCESS)
		return AVDMA_STATUS_LAST;

	if (chn_id < 0 || chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("invalid ch id(%d)!\n",chn_id);
		return AVDMA_STATUS_LAST;
	}

	return (dma_status_t)ioctl(g_dma_fd, DMA_IOC_CHECK, &chn_id);
}

mt_s32 mt_dma_request_chn_id(mt_s32 chn_id)
{
	if (mt_dma_open() != MT_SUCCESS)
		return AVDMA_STATUS_LAST;

	if (chn_id < 0 || chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("invalid ch id(%d)!\n",chn_id);
		return AVDMA_STATUS_LAST;
	}

	return ioctl(g_dma_fd, DMA_IOC_ACTIVE_CH, &chn_id);
}

mt_s32 mt_dma_release_chn_id(mt_s32 chn_id)
{
	if (mt_dma_open() != MT_SUCCESS)
		return AVDMA_STATUS_LAST;

	if (chn_id < 0 || chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("invalid ch id(%d)!\n",chn_id);
		return AVDMA_STATUS_LAST;
	}

	return ioctl(g_dma_fd, DMA_IOC_DEACTIVE_CH, &chn_id);
}

/*
 * @param[in] from/to shall be physical address.
 */
phys_addr_t mt_dma_memcpy(phys_addr_t to, phys_addr_t from, size_t n, unsigned int flags)
{
	mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;
	dma_addr_priv_t dma_addinfo;

	MT_INFO_DMA("dma_memcpy: from %p to %p, size %u\n",from,to,n);

	if (n == 0)
	{
		MT_ERR_DMA("Param error: from %p to %p, size %u\n",from,to,n);
		return 0;
	}

	if (mt_dma_open() != MT_SUCCESS)
	{
		return 0;
	}
	dma_addinfo.cpu_src = from;
	dma_addinfo.cpu_dst = to;
	dma_addinfo.dma_src_addr = 0;
	dma_addinfo.dma_dst_addr = 0;
	dma_addinfo.src_flag = 1;
	dma_addinfo.dst_flag = 1;

	ret = ioctl(g_dma_fd, DMA_IOC_ADDR_TRANSLATION, &dma_addinfo);
	if (ret != DMA_SUCCESS)
	{
		return 0;
	}

	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.flags = flags;
	dma_param.param.len = (mt_u32)n;
	dma_param.param.phy_src_addr = dma_addinfo.dma_src_addr;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = dma_addinfo.dma_dst_addr;
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

/* kernel allocate channel id automaticly */
/*
	dma_param.chn_id = hal_dma_get_free_channel();
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("dma get free channel failed!\n");
		return NULL;
	}
*/
	dma_param.chn_id = 255;
	ret = mt_dma_start(&dma_param);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) start failed!\n",dma_param.chn_id);
		return 0;
	}

	while (DMA_STATUS_STOP != mt_dma_status(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//MT_USLEEP(100);
		MT_USLEEP(1000);
		tmo --;
		if (tmo <= 0)
		{
			MT_ERR_DMA("dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = mt_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) stop failed!\n",dma_param.chn_id);
		return 0;
	}

	return to;
}

phys_addr_t mt_dma_memset(phys_addr_t to, unsigned int val, size_t n,unsigned char width)
{
	mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;
	dma_addr_priv_t dma_addinfo;

	MT_INFO_DMA("dma_memset: to %p, val %x, size %u width %d\n",to,val,n,width);

	if((0 == to) || (0 == n) || (width > 4))
	{
		MT_ERR_DMA("Param error: to %p, val %x, size %u width %d\n",to,val,n,width);
		return 0;
	}

	if (mt_dma_open() != MT_SUCCESS)
	{
		return 0;
	}
	dma_addinfo.cpu_src = 0;
	dma_addinfo.cpu_dst = to;
	dma_addinfo.dma_src_addr = 0;
	dma_addinfo.dma_dst_addr = 0;
	dma_addinfo.src_flag = 0;
	dma_addinfo.dst_flag = 1;

	ret = ioctl(g_dma_fd, DMA_IOC_ADDR_TRANSLATION, &dma_addinfo);
	if (ret != DMA_SUCCESS)
	{
		return 0;
	}

	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.param.len = (mt_u32)n;
	dma_param.param.phy_src_addr = 0;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = dma_addinfo.dma_dst_addr;
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
	switch(width){
		case 0:
		case 1:
			dma_param.param.alu_fill_width = 1;
			break;
		case 2:
			dma_param.param.alu_fill_width = 2;
			break;
		case 4:
			dma_param.param.alu_fill_width = 4;
			break;
		default:
			MT_ERR_DMA("Param error: to %p, val %x, size %u width %d\n",to,val,n,width);
			return 0;
	}
	dma_param.param.mode = DMA_MODE_ALU_FILL;
	dma_param.param.alu_fill_data = val;

	dma_param.param.config.dst_peripheral = 0xf; // memory
	dma_param.param.config.src_peripheral = 0xf; // memory
	dma_param.param.config.dst_endian = 0; // big endian
	dma_param.param.config.src_endian = 0; // big endian
	dma_param.param.config.dst_clk = 0; // AXI clock
	dma_param.param.config.src_clk = 0; // AXI clock
	dma_param.param.config.dst_i = DMA_ADDR_INC;
	dma_param.param.config.src_i = DMA_ADDR_FIX;
	dma_param.param.config.dst_usize = usize;
	dma_param.param.config.src_usize = usize;
	dma_param.param.config.dst_bsize = bnum;
	dma_param.param.config.src_bsize = bnum;
	dma_param.param.control.int_link_en = 1;
	dma_param.param.control.int_node_en = 1;
	dma_param.param.control.chn_param_reg_en = 0;

/* kernel allocate channel id automaticly */
/*
	dma_param.chn_id = hal_dma_get_free_channel();
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("dma get free channel failed!\n");
		return NULL;
	}
*/
	dma_param.chn_id = 255;
	ret = mt_dma_start(&dma_param);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) start failed!\n",dma_param.chn_id);
		return 0;
	}

	while (DMA_STATUS_STOP != mt_dma_status(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//MT_USLEEP(100);
		MT_USLEEP(1000);
		tmo --;
		if (tmo <= 0)
		{
			MT_ERR_DMA("dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = mt_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) stop failed!\n",dma_param.chn_id);
		return 0;
	}

	return to;
}

/*
 * Extra api
 */
phys_addr_t mt_dma_memcpy_ext(mt_u8 ch, phys_addr_t to, phys_addr_t from, size_t n, unsigned int flags)
{
	mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;
	dma_addr_priv_t dma_addinfo;

	MT_INFO_DMA("dma_memcpy: from %llx to %llx, size %u\n",(u64)from,(u64)to,n);

	if(n == 0)
	{
		MT_ERR_DMA("Param error: from %llx to %llx, size %u\n",(u64)from,(u64)to,n);
		return 0;
	}

	if (mt_dma_open() != MT_SUCCESS)
	{
		return 0;
	}

	dma_addinfo.cpu_src = from;
	dma_addinfo.cpu_dst = to;
	dma_addinfo.dma_src_addr = 0;
	dma_addinfo.dma_dst_addr = 0;
	dma_addinfo.src_flag = 1;
	dma_addinfo.dst_flag = 1;

	ret = ioctl(g_dma_fd, DMA_IOC_ADDR_TRANSLATION, &dma_addinfo);
	if (ret != DMA_SUCCESS)
	{
		return 0;
	}

	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.flags = flags;
	dma_param.param.len = (mt_u32)n;
	dma_param.param.phy_src_addr = dma_addinfo.dma_src_addr;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = dma_addinfo.dma_dst_addr;
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

	/* Specify channel id by user */
	dma_param.chn_id = ch;
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		MT_ERR_DMA("dma get free channel failed!\n");
		return 0;
	}

	ret = mt_dma_start(&dma_param);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) start failed!\n",dma_param.chn_id);
		return 0;
	}

	while (DMA_STATUS_STOP != mt_dma_status(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//MT_USLEEP(100);
		MT_USLEEP(1000);
		tmo --;
		if (tmo <= 0)
		{
			MT_ERR_DMA("dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = mt_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		MT_ERR_DMA("dma channel(%d) stop failed!\n",dma_param.chn_id);
		return 0;
	}

	return to;
}



