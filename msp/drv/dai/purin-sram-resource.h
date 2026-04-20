/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _PURIN_SRAM_RESOURCE_H
#define _PURIN_SRAM_RESOURCE_H

void check_if_size_available_for_sram(void);
void *alloc_audio_dma(dma_addr_t* phy_addr, int stream);

#endif
