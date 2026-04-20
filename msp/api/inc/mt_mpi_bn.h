/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_MPI_BN_H__
#define __MT_MPI_BN_H__
#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

mt_u32 mt_bn_len(mt_u8 *data, mt_u32 length);
mt_s32 mt_bn_ucmp(mt_u8 *d1, mt_u32 len1, mt_u8 *d2, mt_u32 len2);
mt_s32 mt_bn_is_odd(mt_u8 *bn, mt_u32 len);

mt_s32 mt_bn_mod(int ce_fd, mt_handle handle, mt_u8 *r, mt_u8 *d, mt_u8 *m, mt_u32 dl, mt_u32 ml);

unsigned int mt_bn_rshift1(unsigned char *out, const unsigned char *data, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif //__MT_MPI_BN_H__

