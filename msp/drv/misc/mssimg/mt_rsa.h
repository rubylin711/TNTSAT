/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __INC_MT_RSA_H__
#define __INC_MT_RSA_H__

typedef struct
{
	unsigned char *pub_key;
	unsigned int pub_key_length;

	//TODO: add your fields here:

} MT_RSA;

MT_RSA *MT_RSA_new(void);
void MT_RSA_delete(MT_RSA *rsa);

int MT_RSA_set_public_key(MT_RSA *r,
                          const unsigned char *key, unsigned int length);

int MT_RSA_verify(int type, const unsigned char *m, unsigned int m_length,
                  const unsigned char *sigbuf, unsigned int siglen, MT_RSA *rsa);

#endif

