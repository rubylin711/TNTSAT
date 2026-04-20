/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:stream_misc header file
 * History:     Date        Author    Modification
 *   1.       2020-12-29   ChenZhimou      Create
 */
#ifndef __STREAM_MISC_H_H__
#define __STREAM_MISC_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "mt_type.h"
#include "mt_module_debug.h"

#if MT_DES("MACRO Declaration", 1)
#ifndef ARRAY_CNT
#define ARRAY_CNT(a) sizeof(a)/sizeof((a)[0])
#endif
#define MT_LOG_FMT(level, fmt)           "["level"] ["MODULE_TAG"] "fmt
#define MT_LOGF(fmt, ...)                MT_FATAL_AVPLAY(MT_LOG_FMT("F", fmt), ##__VA_ARGS__)
#define MT_LOGE(fmt, ...)                MT_ERR_AVPLAY  (MT_LOG_FMT("E", fmt), ##__VA_ARGS__)
#define MT_LOGW(fmt, ...)                MT_WARN_AVPLAY (MT_LOG_FMT("W", fmt), ##__VA_ARGS__)
#define MT_LOGI(fmt, ...)                MT_INFO_AVPLAY (MT_LOG_FMT("I", fmt), ##__VA_ARGS__)
#define MT_LOGD(fmt, ...)                MT_DBG_AVPLAY  (MT_LOG_FMT("D", fmt), ##__VA_ARGS__)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __STREAM_MISC_H_H__ */