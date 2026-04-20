/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "pkcs1_internel.h"

mt_s32 MGF1(mt_u8 *mask, mt_u32 len, mt_u8 *seed, mt_u32 seedlen, mt_u32 hashtype)
{
    mt_handle hash_handle = MT_INVALID_HANDLE;
    mt_u32 i, outlen = 0;
    mt_u8 cnt[4];
    mt_u8 md[64]; // 20 32  only support sha1 sha256
    mt_u32 mdlen;

    mdlen = (MT_CIPHER_HASH_TYPE_SHA1 == hashtype) ? 20 : 32;

    for (i = 0; outlen < len; i++) {
	cnt[0] = (unsigned char)((i >> 24) & 255);
	cnt[1] = (unsigned char)((i >> 16) & 255);
	cnt[2] = (unsigned char)((i >> 8) & 255);
	cnt[3] = (unsigned char)(i & 255);

	mt_unf_cipher_hash_create(hashtype, NULL, &hash_handle);

	mt_unf_cipher_hash_update(hash_handle, seed, seedlen);
	mt_unf_cipher_hash_update(hash_handle, cnt, 4);

	if (outlen + mdlen <= len) {
	    mt_unf_cipher_hash_final(hash_handle, mask + outlen);
	    outlen += mdlen;
	} else {
	    mt_unf_cipher_hash_final(hash_handle, md);
	    memcpy(mask + outlen, md, (mt_u32)(len - outlen));
	    outlen = len;
	}
    }

    return MT_SUCCESS;
}

