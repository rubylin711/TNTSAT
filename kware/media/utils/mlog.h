/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:mlog header file
 * History:     Date        Author    Modification
 *   1.       2021-05-17   Montage      Create
 */
#ifndef __MLOG_H_H__
#define __MLOG_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include <stdio.h>
#include <stdarg.h>
#include "mt_type.h"
#include "mutil.h"

extern int g_player_log_level;
#if MT_DES("Macro Definition", 1)
#ifndef MODULE_TAG
#define MODULE_TAG NULL
#endif

typedef enum {
    MLOG_VERBOSE = 0,
    MLOG_DEBUG,
    MLOG_INFO,
    MLOG_WARN,
    MLOG_ERROR,
    MLOG_FATAL,
    MLOG_SILENCE,
    MLOG_MAX,
} MLOG_LEVEL_E;

#ifndef MLOG_LEVEL
#define MLOG_LEVEL g_player_log_level
#endif

#ifndef MLOGA
#define MLOGA(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_VERBOSE >= MLOG_LEVEL) {                                            \
        mlog_printf(MODULE_TAG, MLOG_VERBOSE, fmt, ##__VA_ARGS__);               \
    }                                                                            \
} while (0)
#endif

#ifndef MLOGD
#define MLOGD(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_DEBUG >= MLOG_LEVEL) {                                              \
        mlog_printf(MODULE_TAG, MLOG_DEBUG, fmt, ##__VA_ARGS__);                 \
    }                                                                            \
} while (0)

#endif

#ifndef MLOGI
#define MLOGI(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_INFO >= MLOG_LEVEL) {                                               \
        mlog_printf(MODULE_TAG, MLOG_INFO, fmt, ##__VA_ARGS__);                  \
    }                                                                            \
} while (0)
#endif

#ifndef MLOGW
#define MLOGW(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_WARN >= MLOG_LEVEL) {                                               \
        mlog_printf(MODULE_TAG, MLOG_WARN, DBG_TO_GREEN(fmt), ##__VA_ARGS__);    \
    }                                                                            \
} while (0)
#endif

#ifndef MLOGE
#define MLOGE(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_ERROR >= MLOG_LEVEL) {                                              \
        mlog_printf(MODULE_TAG, MLOG_ERROR, DBG_TO_YELLOW(fmt), ##__VA_ARGS__);  \
    }                                                                            \
} while (0)
#endif

#ifndef MLOGF
#define MLOGF(fmt, ...)                                                          \
do {                                                                             \
    if (MLOG_FATAL >= MLOG_LEVEL) {                                              \
        mlog_printf(MODULE_TAG, MLOG_FATAL, DBG_TO_RED(fmt), ##__VA_ARGS__);     \
    }                                                                            \
} while (0)
#endif


#endif

#if MT_DES("Struct Definition", 1)
typedef struct mlog_ctrol {
    int level;
    FILE *fp;
    void (*cb)(const char *, const int, const char *, va_list);
} mlog_ctrl_t;
#endif

#if MT_DES("External API Declaration", 1)
/* 0:set player,other:set ffmpeg */
int mlog_check_gobal_log_level(void);
void mlog_printf(const char *module, const int level, const char *format, ...);
void mlog_set_controller(mlog_ctrl_t *ctrl);

mlog_ctrl_t *mlog_get_controller(void);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MLOG_H_H__ */
