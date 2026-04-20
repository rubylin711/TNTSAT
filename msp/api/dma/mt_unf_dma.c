/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_common.h"
#include "mt_drv_dma.h"
#include "mt_unf_dma.h"

mt_s32 mt_unf_dma_request_channel(MT_EDMA_CH_E ch)
{
	if (ch != mt_dma_request_chn_id((mt_u8)ch)) 
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_s32 mt_unf_dma_release_channel(MT_EDMA_CH_E ch)
{
	if (MT_SUCCESS != mt_dma_release_chn_id((mt_u8)ch)) 
	{
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_s32 mt_unf_dma_memcpy(MT_EDMA_CH_E ch, phys_addr_t src, phys_addr_t dst, mt_u32 size)
{
	if (0 == mt_dma_memcpy_ext((mt_u8)ch, dst, src, size, 0)) {
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

mt_s32 mt_unf_dma_check(MT_EDMA_CH_E ch)
{
	if (DMA_STATUS_STOP != mt_dma_status((mt_u8)ch)) {
		//busy
		return MT_EDMA_STATUS_BUSY;
	}

	return MT_EDMA_STATUS_FREE;
}
