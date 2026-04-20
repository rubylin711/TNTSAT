/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __PKCS1_INTERNEL_H__
#define __PKCS1_INTERNEL_H__

#define MT_RSA_SIGN      1
#define MT_RSA_CRYPT     2

mt_s32 MGF1(mt_u8 *mask, mt_u32 len, mt_u8 *seed, mt_u32 seedlen, mt_u32 hashtype);

int PKCS1_OAEP_mgf1_encode(mt_u8 *to, mt_u32 tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 hashtype);

int PKCS1_OAEP_mgf1_decode(mt_u8 *to, mt_u32 *tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 num, mt_u32 hashtype);

int PKCS1_V1_5_encode(mt_u8 *to, mt_u32 tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 mode);

int PKCS1_V1_5_decode(mt_u8 *to, mt_u32 *tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 mode);

int PKCS1_PSS_sign(mt_u8 *msg, mt_u32 mlen, mt_u32 emBits,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype);

int PKCS1_PSS_verify(mt_u8 *msg, mt_u32 mlen, mt_u32 emBits,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype);

int PKCS1_V1_5_sign(mt_u8 *msg, mt_u32 mlen,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype);

int PKCS1_V1_5_verify(mt_u8 *msg, mt_u32 mlen,
        mt_u8 *sign, mt_u32 slen, mt_u32 hashtype);

#endif

