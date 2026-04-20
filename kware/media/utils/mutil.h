/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MUTIL_H_H__
#define __MUTIL_H_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifdef _WIN_PC_
#define _TIMESPEC_DEFINED
#endif

#include "mt_type.h"
#include "pthread.h"
#include "string.h"
#include "stdlib.h"

#if MT_DES("Macrofunction Definition", 1)
#define SECOND_TO_MICROSECOND_BASE        (1000 * 1000)
#define MLZP_MUTEX_INIT_VALUE             PTHREAD_MUTEX_INITIALIZER

/* sleep millisecond */
#ifdef _WIN_PC_
#include <Windows.h>
#define mlzp_msleep(ms)          Sleep((ms))
#else
#include <unistd.h>
#define mlzp_msleep(ms)          usleep((ms) * 1000)
#endif

#endif

#if MT_DES("Inline Function Definition", 1)
static inline void *mlzp_malloc(unsigned int size)
{
    if (0 == size) {
        return NULL;
    }
    return malloc(size);
}

static inline void mlzp_free(void *ptr)
{
    if (NULL == ptr) {
        return;
    }

    free(ptr);
}

static inline void mlzp_zfree(void **ptr)
{
    if (NULL == ptr || NULL == *ptr) {
        return;
    }

    free(*ptr);
    *ptr = NULL;
}

static inline char *mlzp_strdup(char *str)
{
#ifdef _WIN_PC_
    return _strdup(str);
#else
    return strdup(str);
#endif
}

#define IS_STREAM_URL(url)   mlzp_is_stream_uri(url)

#endif

#if MT_DES("Macro Definition", 1)

#ifndef mlzp_mutex_t
typedef pthread_mutex_t              mlzp_mutex_t;
#endif

#ifndef mlzp_cond_t
typedef pthread_cond_t               mlzp_cond_t;
#endif

#ifndef mlzp_thread_t
typedef pthread_t                    mlzp_thread_t;
#endif

#ifndef NO_OPT
#define NO_OPT __attribute__ ((__used__))
#endif

#ifndef DBG_TO_RED
#define DBG_TO_RED(str)     "\033[31m"str"\033[0m"
#endif

#ifndef DBG_TO_GREEN
#define DBG_TO_GREEN(str)   "\033[32m"str"\033[0m"
#endif

#ifndef DBG_TO_YELLOW
#define DBG_TO_YELLOW(str)  "\033[33m"str"\033[0m"
#endif

#ifndef DBG_TO_BLUE
#define DBG_TO_BLUE(str)    "\033[34m"str"\033[0m"
#endif

#ifndef DBG_TO_PURPLE
#define DBG_TO_PURPLE(str)  "\033[35m"str"\033[0m"
#endif

#ifndef ARRAY_CNT
#define ARRAY_CNT(a) sizeof((a))/sizeof((a)[0])
#endif

#define MRD_LE16(p)                                                    \
    (((unsigned short)(((const unsigned char *)(p))[0]))      |        \
     ((unsigned short)(((const unsigned char *)(p))[1]) << 8))         \

#define MRD_LE32(p)                                                    \
    (((unsigned int) MRD_LE16(p))                             |        \
    (((unsigned int) MRD_LE16((uintptr_t)(p) + 2)) << 16))

#define MRD_LE64(p)                                                    \
    (((unsigned int) MRD_LE32(p))                             |        \
    (((unsigned int) MRD_LE32((uintptr_t)(p) + 4)) << 32))

#define MRD_BE16(p)                                                    \
    (((unsigned short)(((const unsigned char *)(p))[0]) << 8) |        \
      (unsigned short)(((const unsigned char *)(p))[1]))

#define MRD_BE32(p)                                                    \
    (((unsigned int) MRD_BE16(p) << 16)                       |        \
     ((unsigned int) MRD_BE16((uintptr_t)(p) + 2)))

#define MRD_BE64(p)                                                    \
    (((unsigned long long) MRD_BE32(p) << 32)                 |        \
     ((unsigned long long) MRD_BE32((uintptr_t)(p) + 4)))

#define MWR_LE16(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)((val) & 0xff);         \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 8) & 0xff);  \
}

#define MWR_LE32(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)((val) & 0xff);         \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 8) & 0xff);  \
    ((unsigned char *)(p))[2] = (unsigned char)(((val) >> 16) & 0xff); \
    ((unsigned char *)(p))[3] = (unsigned char)(((val) >> 24) & 0xff); \
}

#define MWR_LE64(p, val)    {                                          \
    MWR_LE32(p, val);                                                  \
    MWR_LE32((uintptr_t)(p) + 4, (val) >> 32);                         \
}

#define MWR_BE16(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)(((val) >> 8) & 0xff);  \
    ((unsigned char *)(p))[1] = (unsigned char)((val) & 0xff);         \
}

#define MWR_BE32(p, val)    {                                          \
    ((unsigned char *)(p))[0] = (unsigned char)(((val) >> 24) & 0xff); \
    ((unsigned char *)(p))[1] = (unsigned char)(((val) >> 16) & 0xff); \
    ((unsigned char *)(p))[2] = (unsigned char)(((val) >> 8) & 0xff);  \
    ((unsigned char *)(p))[3] = (unsigned char)((val) & 0xff);         \
}

#define MWR_BE64(p, val)    {                                          \
    MWR_BE32((uintptr_t)(p) + 4, (val) >> 32);                         \
    MWR_BE32(p, val);                                                  \
}

#define MK_TAG32(a0, a1, a2, a3)                                       \
    ((unsigned int)(a0)        |                                       \
    ((unsigned int)(a1) << 8)  |                                       \
    ((unsigned int)(a2) << 16) |                                       \
    ((unsigned int)(a3) << 24))

#endif

#if MT_DES("Struct Definition", 1)
/* rational */
typedef struct m_rational {
    int num; /* numerator */
    int den; /* denominator */
} m_rational_t;

typedef struct sys_mem_debug {
    int avaibleMem;
    int totalMem;
    int totalFree;
    int nFree;
    int cachedMem;
 } sys_mem_debug_t;
#endif

#if MT_DES("External API Declaration", 1)
int mlzp_is_stream_uri(const char *url);

void mlzp_show_encryption_init_infomation(int drm_index, void *arg);
void mlzp_show_encryption_sample_infomation(int index, void *pDataIn);
void show_sys_memory_info(sys_mem_debug_t *start, sys_mem_debug_t *stop, int show_diff);

mt_s64 mclock_get_utime(void);
void mclock_get_clock(mt_u8 *hour, mt_u8 *minute,
                      mt_u8 *second, mt_u16 *ms, mt_u16 *us);

double m_q2d(m_rational_t q);
double m_round(double x);

int mlzp_mutex_init(mlzp_mutex_t **mutex);
int mlzp_mutex_destroy(mlzp_mutex_t **mutex);
int mlzp_mutex_lock(mlzp_mutex_t *mutex);
int mlzp_mutex_trylock(mlzp_mutex_t *mutex);
int mlzp_mutex_unlock(mlzp_mutex_t *mutex);

int mlzp_cond_init(mlzp_cond_t **cond);
int mlzp_cond_destroy(mlzp_cond_t **cond);
/* lock before wait and unlock will take place in mlzp_cond_wait function */
int mlzp_cond_wait(mlzp_cond_t *cond, mlzp_mutex_t *mutex);
int mlzp_cond_timedwait(mlzp_cond_t *cond, mlzp_mutex_t *mutex, const int time_ms);
int mlzp_cond_signal(mlzp_cond_t *cond, mlzp_mutex_t* mutex);
int mlzp_cond_broadcast(mlzp_cond_t *cond, mlzp_mutex_t* mutex);

int mlzp_thread_create(mlzp_thread_t *tid,
    const char *tname, void *(*entry)(void *), void *arg);
int mlzp_thread_join(mlzp_thread_t tid, const char *tname);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MUTIL_H_H__ */
