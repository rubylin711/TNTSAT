/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:demux_comm header file
 * History:     Date        Author    Modification
 *   1.       2021-06-02  Montage      Create
 */
#ifndef __DEMUX_COMM_H_H__
#define __DEMUX_COMM_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mutil.h"

#if MT_DES("External API Declaration", 1)
static inline unsigned long long
    demux_parse_mpeg2_pts(const unsigned char *buf)
{
    return (((unsigned long long)(buf[0] & 0x0e) << 29)                     |
            ((unsigned long long)(MRD_BE16((uintptr_t)buf + 1) >> 1) << 15) |
             (unsigned long long)(MRD_BE16((uintptr_t)buf + 3) >> 1));
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __DEMUX_COMM_H_H__ */
