/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __STREAM_FILTER_OS_SUPPORT_H_H__
#define __STREAM_FILTER_OS_SUPPORT_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include "mtlz_avfilter_stream.h"
#include "stream_filter/stream_filter.h"

MTAVSF_MEMCP_FUNC avfilter_get_memcp_func(
    unsigned int codec_id, unsigned int encrypted);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __STREAM_FILTER_OS_SUPPORT_H_H__ */
