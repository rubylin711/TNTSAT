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
#include "mt_unf_rsa.h"
#include "pkcs1_internel.h"

/* EB = 00+ BT+PS +00+ D */
int PKCS1_V1_5_encode(mt_u8 *to, mt_u32 tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 mode)
{
    mt_u32 i, j;
    mt_u8 *p = to;
    mt_u8 data;

    j = tlen - 3 - flen;

    /* 0 */
    *p++ = 0;
    if (mode == MT_RSA_CRYPT)
    {
        /* Public Key BT (Block Type) */
        *p++ = MT_RSA_CRYPT;

        /* PS: pad out with non-zero random data */
        for (i = 0; i < j; i++) 
        {
            do {
                mt_unf_cipher_get_random_number(1, &data);
            } while (data == 0);

            *p++ = data;
        }
    }
    else
    {
        /* Private Key BT (Block Type) */
        *p++ = MT_RSA_SIGN;

        /* PS: pad out with 0xFF */
        for (i = 0; i < j; i++) 
            *p++ = 0xFF;
    }

    /* 0 */
    *p++ = 0;

    /* D */
    memcpy(p, from, (unsigned int)flen);

    return MT_SUCCESS;
}

int PKCS1_V1_5_decode(mt_u8 *to, mt_u32 *tlen,
    mt_u8 *from, mt_u32 flen, mt_u32 mode)
{
    mt_u32 i;
    mt_u8 *p = from;
    mt_u8 bad = 0;

    /* 0 */
    bad |= p[0];

    if (mode == MT_RSA_CRYPT)
    {
        /* BT */
        bad |= p[1] ^ MT_RSA_CRYPT;

        /* PS: pad out with non-zero random data */
        for (i=2; i<flen; i++)
        {
            if (p[i] == 0)
                break;
        }
    }
    else
    {
        /* BT */
        bad |= p[1] ^ MT_RSA_SIGN;

        /* PS: pad out with 0xFF */
        for (i=2; i<flen; i++)
        {
            if (p[i] != 0xFF)
            {
                if (p[i] == 0)
                    break;

                bad = 1;
                break;
            }
        }
    }

    if (i == flen)
        bad = 1;
    if (i < 10)
        bad = 1;

    if (bad == 0)
    {
        /* 0 */
        i++;

        /* D */
        *tlen = flen - i;
	if (((&p[i] >= to) && (&p[i] < to + *tlen)) || ((to >= &p[i]) && (to < &p[i] + *tlen)))
       	    memmove(to, &p[i], *tlen);   //overlap
	else
            memcpy(to, &p[i], *tlen);
    }

    if (bad)
        return MT_RSA_ERR_BAD_PADDING;
    else
        return MT_SUCCESS;
}

