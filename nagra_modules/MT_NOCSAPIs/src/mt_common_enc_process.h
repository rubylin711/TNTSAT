/*
 * @file mt_common_enc_process.h
 * @brief Montage implemantation of Common Encryption Scheme(ISO/IEC 23001-7)
 *
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#ifndef __MT_COMMON_ENC_PROCESS_H__
#define __MT_COMMON_ENC_PROCESS_H__
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"

//#define __CEN_DEBUG__

typedef enum _MT_CENC_SCHEME_E
{
	MT_CENC_SCH_CENC,	/* 'cenc' */
	MT_CENC_SCH_CBC1,	/* 'cbc1' */
	MT_CENC_SCH_CENS,	/* 'cens' */
	MT_CENC_SCH_CBCS,	/* 'cbcs' */
} MT_CENC_SCHEME;

typedef struct _MT_CENC_SUBSAMPLE_INFO_S
{
	mt_u8 *sub_in; //shall be phyaddr
	mt_u8 *sub_out;//shall be phyaddr
	mt_u32 clear_size;
	mt_u32 protect_size;
	mt_u8 *iv;
	mt_u32 iv_size;
} MT_CENC_SUBSAMPLE_INFO_S;

typedef struct _MT_CENC_SAMPLE_INFO_S
{
	mt_u8 *input;
	mt_u32 input_size;
	MT_CENC_SCHEME sch;		/*TODO:not used now */
	mt_u32 sub_count;		/* should >= 1. If full-encryption, =1 */
	MT_BOOL	is_pattern_enc;	/* if TRUE, should specify params below */
	mt_u32 crypt_blocks;	/* encrypted 16-byte blocks inside encryption pattern mode */
	mt_u32 clear_blocks;	/* unencrypted 16-byte blocks inside encryption pattern mode */
	MT_BOOL incremental;	/* Always set it FALSE, or leave it alone. (NOTE:Not support, reserve for future use) */
} MT_CENC_SAMPLE_INFO_S;

mt_s32 mt_cenc_create(mt_handle *cenc_handle, MT_CIPHER_CTRL_S *ctrl, mt_u32 key_slot, MT_CENC_SAMPLE_INFO_S *sample_info);

mt_s32 mt_cenc_chain(mt_handle cenc_handle, MT_CENC_SUBSAMPLE_INFO_S *sub_info);

mt_s32 mt_cenc_process(mt_handle cenc_handle);



#endif //__MT_COMMON_ENC_PROCESS_H__
