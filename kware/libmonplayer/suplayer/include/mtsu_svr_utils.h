/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*
 * mt_svr_logger.h
 *
 *  Created on: 2017Äê11ÔÂ10ÈÕ
 *      Author: ztq
 */

#ifndef MT_SVR_UTILS_H_
#define MT_SVR_UTILS_H_

#include <stdarg.h>
#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef SVR_LOG_TAG
#define SVR_LOG_TAG NULL
#endif

#ifndef SVR_LOG_PREFIX
#define SVR_LOG_PREFIX NULL
#endif

typedef enum tagSVR_LOG_LEVEL_E {
    SVR_LOG_QUIET = 0,
    SVR_LOG_FATAL,
    SVR_LOG_ERROR,
    SVR_LOG_WARN,
    SVR_LOG_INFO,
    SVR_LOG_DEBUG,
    SVR_LOG_VERBOSE,
    SVR_LOG_BUTT,
} SVR_LOG_LEVEL_E;

#define SVR_LOGV(...)                                             \
    do {                                                          \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_VERBOSE, __VA_ARGS__); \
    } while (0)

#define SVR_LOGD(...)                                           \
    do {                                                        \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_DEBUG, __VA_ARGS__); \
    } while (0)

#define SVR_LOGI(...)                                          \
    do {                                                       \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_INFO, __VA_ARGS__); \
    } while (0)

#define SVR_LOGW(...)                                          \
    do {                                                       \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_WARN, __VA_ARGS__); \
    } while (0)

#define SVR_LOGE(...)                                           \
    do {                                                        \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_ERROR, __VA_ARGS__); \
    } while (0)

#define SVR_LOGF(...)                                           \
    do {                                                        \
	SVR_LOG_Write(SVR_LOG_TAG, SVR_LOG_FATAL, __VA_ARGS__); \
    } while (0)

typedef struct tagSVR_LOG_WRITER_S
{
    MT_VOID (*write)(MT_VOID *opaque, const MT_CHAR *tag, mt_s32 level,
                     MT_CHAR *fmt, va_list var);
    MT_VOID *opaque;
} SVR_LOG_WRITER_S;

MT_VOID SVR_LOG_Write(const MT_CHAR *tag, mt_s32 level, MT_CHAR *fmt, ...);

MT_VOID SVR_LOG_RegisterWriter(SVR_LOG_WRITER_S *writer);

/*if the level setted by SVR_LOG_SetDebugLevel is valid, use this debug level first,
  else use the level setted by SVR_LOG_SetLevel*/
mt_s32 SVR_LOG_SetLevel(mt_s32 s32Level);

mt_s32 SVR_LOG_GetLevel();

mt_s32 SVR_LOG_SetDebugLevel(mt_s32 level);

mt_s32 SVR_LOG_GetDebugLevel();

mt_s32 SVR_SYS_SetProperty(const MT_CHAR *pszKey, const MT_CHAR *pszVal);

mt_s32 SVR_SYS_GetProperty(const MT_CHAR *pszKey, MT_CHAR *pszVal, mt_s32 s32Len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* MT_SVR_UTILS_H_ */
