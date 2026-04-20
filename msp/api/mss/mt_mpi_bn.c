/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_module_debug.h"
#include "mt_unf_cipher_v2.h"
#include "mt_mpi_cipher_v2.h"
#include "mt_mpi_bn.h"

mt_u32 mt_bn_len(mt_u8 *data, mt_u32 length)
{
    mt_u32 i;

    for (i=0; i<length; i++) {
        if (data[i] != 0)
            break;
    }

    return (length-i);
}

mt_s32 mt_bn_ucmp(mt_u8 *d1, mt_u32 len1, mt_u8 *d2, mt_u32 len2)
{
    mt_u32 i;
    mt_u32 d1_vl, d2_vl;
    mt_u8 *p1, *p2;

    d1_vl = mt_bn_len(d1, len1);
    d2_vl = mt_bn_len(d2, len2);

    if (d1_vl > d2_vl)
        return 1;

    if (d1_vl < d2_vl)
        return -1;

    p1 = d1 + len1 - d1_vl;
    p2 = d2 + len2 - d2_vl;

    for (i = 0; i<d1_vl-1; i++) {
        if (p1[i] > p2[i])
            return 1;
        if (p1[i] < p2[i])
            return -1;
    }

    return 0;
}

mt_s32 mt_bn_is_odd(mt_u8 *bn, mt_u32 len)
{
    mt_s32 ret = -1;

    if (bn[len-1] & 1)
        ret = 0;

    return ret;
}

/*
d = dn-1 << ml*8*(n-1) | ... | d1 << ml*8 | d0
len(d0) = len(d1) = ... = len(dn-2) = ml
len(dn-1) = dl - ml * (n - 1)

r = d mod m
  = [(dn-1 * 2^ml ...* 2^ml) + ... d1 *  2^ml + d0] mod m
  = ({[(dn-1 * 2^ml) mod m] ...* 2^ml} mod m + ... (d1 *  2^ml) mod m + d0 mod m) mod m
*/
mt_s32 mt_bn_mod(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 dl, mt_u32 ml)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 n;
    mt_u8 *di;
    mt_u32 di_len;
    mt_u32 i, j;
    mt_u8 tmp[256];
    mt_u8 e[260];
    mt_u32 el;

    if (r == NULL || d == NULL || m == NULL)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (d[0] == 0 || m[0] == 0)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    if (ml > 256)
        return CE_BN_MOD_MUL_INPUT_ERROR;

    /* e =  2^ml;   el = len(e) */
    el = ml + 1;
    memset(e, 0, el);
    e[0] = 1;    

    n = (dl + ml - 1) / ml;

    memset(r, 0, ml);
    for (i = 0; i < n; i++) {
        if (dl > ml * (i + 1)) {
            di = &d[dl - ml * (i + 1)];
            di_len = ml;
        }
        else {
            di = &d[0];
            di_len = dl - ml * i;
        }

        memset(tmp, 0, ml);
        memcpy(tmp + ml - di_len, di, di_len);

        for (j = 0; j < i; j++) {
            /* tmp = (tmp * 2^ml) mod m */
            ret = mt_mpi_crypto_bn_mod_mul(ce_fd, handle, tmp, tmp, e, m, ml, el, ml);
            if (ret != MT_SUCCESS)
                return ret;
        }

        /* r = (r + tmp) mod m */
        ret = mt_mpi_crypto_bn_mod_add(ce_fd, handle, r, r, tmp, m, ml, ml, ml);
        if (ret != MT_SUCCESS)
            return ret;
    }

    return ret;
}

unsigned int mt_bn_rshift1(unsigned char *out, const unsigned char *data, unsigned int len)
{
    unsigned int c;
    unsigned int i, j;

    /* Skip leading zero's. */
    for ( ; len > 0 && *data == 0; data++, len--)
        continue;

    i = 0;
    c = 0;
    if ((data[0] >> 1) == 0) {
        i++;
        c = (data[0] & 1) ? 0x80 : 0;
    }

    for (j=0 ; i < len; i++, j++) {
        out[j] = (data[i] >> 1) | (unsigned char)c;
        c = (data[i] & 1) ? 0x80 : 0;
    }

    return j;
}

