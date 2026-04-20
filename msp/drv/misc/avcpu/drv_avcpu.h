/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __INC_DRV_AVCPU_H__
#define __INC_DRV_AVCPU_H__

#define AVCPU_MTD_PART_NAME					"avcpu.img"

MT_BOOL mt_avcpu_get_encrypt_status(void);

u32 mt_avcpu_decrypt(u32 ddr_addr);

u32 avcpu_decompress(u32 ddr_addr);

void misc_avcpu_resume(void);

#endif

