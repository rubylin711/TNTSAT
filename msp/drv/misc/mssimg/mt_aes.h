/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __INC_MT_AES_H__
#define __INC_MT_AES_H__

# define MT_AES_ENCRYPT     1
# define MT_AES_DECRYPT     0


#define MT_AES_CIPHER_KEYSLOT_INVALID 255

typedef struct
{
	unsigned int keyslot;

	//TODO: add your AES Key fields here:
} MT_AES_KEY;

int MT_AES_set_decrypt_keyslot(const unsigned int keyslot,
                               MT_AES_KEY *key);

int MT_AES_cbc_encrypt(const unsigned char *in, unsigned char *out,
                       size_t length, const MT_AES_KEY *key,
                       unsigned char *ivec, const int enc);

#endif

