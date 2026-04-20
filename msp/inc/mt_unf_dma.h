/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __MT_UNF_DMA_H__
#define  __MT_UNF_DMA_H__

#include "mt_common.h"

typedef enum _EDMA_STATUS_E {
	MT_EDMA_STATUS_FREE,
	MT_EDMA_STATUS_BUSY,
	MT_EMDA_STATUS_UNKNOWN,
} MT_EDMA_STATUS_E;

typedef enum _EDMA_CH_E {
	MT_EDMA_CH_0,
	MT_EDMA_CH_1,
	MT_EDMA_CH_2,
	MT_EDMA_CH_3,
	MT_EDMA_CH_4,
	MT_EMDA_CH_MAX,
} MT_EDMA_CH_E;

mt_s32 mt_unf_dma_check(MT_EDMA_CH_E ch);
mt_s32 mt_unf_dma_request_channel(MT_EDMA_CH_E ch);
mt_s32 mt_unf_dma_release_channel(MT_EDMA_CH_E ch);

mt_s32 mt_unf_dma_memcpy(MT_EDMA_CH_E ch, phys_addr_t src, phys_addr_t dst, mt_u32 size);

#endif
