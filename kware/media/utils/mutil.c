/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:mutil definition
 * History:     Date        Author    Modification
 *   1.       2021-04-29  Montage      Create
 */
#define MODULE_TAG "MUTIL"
#include <math.h>
#include "mutil.h"
#include "mlog.h"
#include "string.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if defined(MT_DRM_SUPPORT)
#include "MTDrmApi.h"
#endif

#if MT_DES("Internal API Definition", 1)
#ifndef _WIN_PC_
#define CMD_LINE_LEN    256
static char *read_cmd_line(FILE *f, char *p_cmd_line)
{
    char c = 0;
    int len = 0;

    while ((c = fgetc(f)) != EOF && c != '\n') {
        p_cmd_line[len++] = c;
        p_cmd_line[len] = '\0';
        if (len > (CMD_LINE_LEN - 1)) {
            MLOGE("[%s_%d] Overflow...........\n", __func__, __LINE__);
            return NULL;
        }
    }

    return p_cmd_line;
}

static int mt_run_memory(int isflushCache, char *p_cmd_line,
                   int *pMemAvailable, int *pMemTotal, int *pTotalFree, int *pMemNowFree, int *pNCached)
{
    int mem_free    = 0;
    int nBuffers    = 0;
    int newCached   = 0 ;
    int nSwapCached = 0;
    FILE *mem_fp    = NULL;

    if (NULL == mem_fp) {
        mem_fp = fopen("/proc/meminfo", "r");
    }

    if (NULL == mem_fp) {
        MLOGE("%s: open /proc/meminfo error!!!!!!!!!!!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    while (read_cmd_line(mem_fp, p_cmd_line)) {
        if (strncmp(p_cmd_line, "MemTotal", strlen("MemTotal")) == 0) {
            sscanf(p_cmd_line, "MemTotal: %d", pMemTotal);
        } else if (strncmp(p_cmd_line, "MemFree", strlen("MemFree")) == 0) {
            sscanf(p_cmd_line, "MemFree: %d", pMemNowFree);
        } else if (strncmp(p_cmd_line, "MemAvailable", strlen("MemAvailable")) == 0) {
            sscanf(p_cmd_line, "MemAvailable: %d", pMemAvailable);
        } else if (strncmp(p_cmd_line, "Buffers", strlen("Buffers")) == 0) {
            sscanf(p_cmd_line, "Buffers: %d", &nBuffers);
        } else if (strncmp(p_cmd_line, "newCached", strlen("newCached")) == 0) {
            sscanf(p_cmd_line, "newCached: %d", &newCached);
        } else if (strncmp(p_cmd_line, "Cached", strlen("Cached")) == 0) {
            sscanf(p_cmd_line, "Cached: %d", pNCached);
        } else if (strncmp(p_cmd_line, "SwapCached", strlen("SwapCached")) == 0) {
            sscanf(p_cmd_line, "SwapCached: %d", &nSwapCached);
            break;
        } else {
            continue;
        }
    }

    *pTotalFree = (*pMemNowFree + *pNCached + nSwapCached + nBuffers);
    if (mem_fp) {
        fclose(mem_fp);
        mem_fp = NULL;
    }

    return mem_free;
}
#endif
#endif

#if MT_DES("External API Definition", 1)
#ifdef _WIN_PC_
void show_sys_memory_info(sys_mem_debug_t* start, sys_mem_debug_t* stop, int show_diff)
{
}
#else
int mlzp_is_stream_uri(const char *url)
{
    int i;
    static const char *stream_uris[] = {
        "http://", "https://", "mms://" , "rtsp://", "rtp://" , "udp://",
        "mmsh://", "mmsu://" , "mmst://", "fd://"  , "myth://", "ssh://",
        "ftp://" , "sftp://" , "rtmp://",  NULL
    };

    for (i = 0; stream_uris[i]; i++) {
        if (!strncasecmp(url, stream_uris[i], strlen(stream_uris[i]))) {
            return 1;
        }
    }
    return 0;
}

void mlzp_show_encryption_init_infomation(int drm_index, void *arg)
{
#if defined(MT_DRM_SUPPORT)
    unsigned int idx;

    MTDRM_PROTECT_INFO *info = arg;
    MLOGI("[%d]Encryption Init Information, pssh %d\n", drm_index, info->is_pssh);
    if (info->init_data) {
        MLOGI("Init len %d, Data:\n{", info->init_data_len);
        for (idx = 0; idx < info->init_data_len; idx++) {
            printf("0x%02x%s", info->init_data[idx], (idx + 1) == info->init_data_len ? "}\n" : ", ");
        }
    }
    if (info->kids) {
        MLOGI("KID conut:%d\n", info->kid_count);
        for (int j = 0; j < info->kid_count; j++) {
            MLOGI("ID[%d]:\n{", j);
            for (idx = 0; idx < 16; idx++) {
                printf("0x%02x%s", info->kids[j][idx], (idx + 1) == 16 ? "}\n" : ", ");
            }
        }
    }
    MLOGI("License : %s\n", info->license_url ? info->license_url : "NULL");
    if (info->init_data) {
        MLOGI("Customer len %d, Data:{\n", info->customer_data_len);
        for (idx = 0; idx < info->customer_data_len; idx++) {
            printf("0x%02x%s", info->customer_data[idx], (idx + 1) == info->customer_data_len ? "}\n" : ", ");
        }
    }
#endif
}

void mlzp_show_encryption_sample_infomation(int index, void *pDataIn)
{
#if defined(MT_DRM_SUPPORT)
    int i;
    DMTRM_BUFFER_IN *input = pDataIn;

    MLOGI("[%d] Decrypt sample len:%d crypt_byte_block:%d skip_byte_block:%d iv:%p region:%d\n",
        index, input->data_length, input->patternEncrypt, input->patternClear, input->iv, input->region_count);

    MLOGI("Data:\n{");
    for (i = 0; i < input->data_length; i++) {
        printf("0x%02x, ", input->data[i]);
    }
    MLOGI("}\n");

    if (input->iv) {
        MLOGI("iv:16\n{");
        for (i = 0; i < 16; i++) {
            printf("0x%02x, ", input->iv[i]);
        }
        MLOGI("}\n");
    }
    if (input->region_count) {
        MLOGI("subsample_count:%d\n", input->region_count);
        for (i = 0; i < input->region_count; i++) {
            printf("[%d] clear:%d  encrypted:%d\n", i,
            input->clear[i], input->encrypt[i]);
        }
        MLOGI("\n");
    }
#endif
}

void show_sys_memory_info(sys_mem_debug_t *start, sys_mem_debug_t *stop, int show_diff)
{
    char cmd_line[CMD_LINE_LEN] = {0};

    if (NULL == start || (MT_FALSE != show_diff && NULL == stop)) {
        return;
    }

    if (MT_FALSE == show_diff) {
        /**for debug memory info start start*/
        mt_run_memory(0, cmd_line,
                        &(start->avaibleMem),
                        &(start->totalMem),
                        &(start->totalFree),
                        &(start->nFree),
                        &(start->cachedMem));

        MLOGD("Avaible:%d,total:%d,Free:%d,nFree%d,cached:%d\n",
            start->avaibleMem, start->totalMem, start->totalFree, start->nFree, start->cachedMem);
    } else {
        mt_run_memory(0, cmd_line,
                      &(stop->avaibleMem),
                      &(stop->totalMem),
                      &(stop->totalFree),
                      &(stop->nFree),
                      &(stop->cachedMem));

        printf("Memory delta, avaible:%d nFree:%d totalFree:%d, nCached:%d\n",
               stop->avaibleMem - start->avaibleMem, stop->nFree    - start->nFree,
               stop->totalFree  - start->totalFree, stop->cachedMem - start->cachedMem);
    }
}
#endif

#ifdef _WIN_PC_
#include <windows.h>
mt_s64 mclock_get_utime(void)
{
    mt_s64 time_us;
    /* unit:100ns */
    FILETIME file_time;
    /* 1970-01-01 00:00:00 and 1601-01-01 00:00:00, unit:us*/
    const static mt_s64 diff = 11644473600000000;
    /* For china UTC+08:00, file_time will lag 8 hours */
    GetSystemTimeAsFileTime(&file_time);
    time_us = (mt_s64) (((mt_u64)file_time.dwHighDateTime) << 32 | file_time.dwLowDateTime);
    time_us = (time_us / 10) - diff;

    return time_us;
}
#else
#include <sys/time.h>
mt_s64 mclock_get_utime(void)
{
    mt_s64 time_us;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_us = (mt_s64) tv.tv_sec * 1000000 + tv.tv_usec;

    return time_us;
}
#endif

void mclock_get_clock(mt_u8 *hour, mt_u8 *minute,
                      mt_u8 *second, mt_u16 *ms, mt_u16 *us)
{
    mt_s64 time_reamin;
    const static mt_s64 US_TO_DAY_DEN    = (mt_s64) 24 * 60 * 60 * 1000 * 1000;
    const static mt_s64 US_TO_HOUR_DEN   = (mt_s64) 1000 * 1000 * 60 * 60;
    const static mt_s64 US_TO_MINUTE_DEN = (mt_s64) 1000 * 1000 * 60;
    const static mt_s64 US_TO_SECOND_DEN = (mt_s64) 1000 * 1000;
    mt_s64 time_us = mclock_get_utime();

    /* convert time to that of a day */
    time_reamin = time_us % US_TO_DAY_DEN;
    if (NULL != hour) {
        *hour = (mt_u8) (time_reamin / US_TO_HOUR_DEN);
    }

    time_reamin = time_reamin % US_TO_HOUR_DEN;
    if (NULL != minute) {
        *minute = (mt_u8) (time_reamin / US_TO_MINUTE_DEN);
    }

    time_reamin = time_reamin % US_TO_MINUTE_DEN;
    if (NULL != second) {
        *second = (mt_u8) (time_reamin / US_TO_SECOND_DEN);
    }

    time_reamin = time_reamin % US_TO_SECOND_DEN;
    if (NULL != ms) {
        *ms = (mt_u16) (time_reamin / 1000);
    }

    time_reamin = time_reamin % 1000;
    if (NULL != us) {
        *us = (mt_u16) (time_reamin / 1000);
    }
}

double m_q2d(m_rational_t q)
{
    if (0 == q.den || 0 == q.num) {
        return 0.0;
    }
    return (double) q.num / q.den;
}

double m_round(double x)
{
    return (x > 0) ? floor(x + 0.5) : ceil(x - 0.5);
}

int mlzp_mutex_init(mlzp_mutex_t **mutex)
{
    if (!mutex) {
        return -1;
    }
    mlzp_mutex_t *m = mlzp_malloc(sizeof(mlzp_mutex_t));
    if (!m) {
        return -1;
    }
    memset(m, 0, sizeof(mlzp_mutex_t));
    int ret = pthread_mutex_init(m, NULL);
    if (0 != ret) {
        mlzp_free(m);
        return ret;
    }
    *mutex = m;
    return 0;
}

int mlzp_mutex_destroy(mlzp_mutex_t **mutex)
{
    if (!mutex || !*mutex) {
        return -1;
    }

    int ret = pthread_mutex_destroy(*mutex);
    mlzp_zfree((void **)mutex);
    return ret;
}

int mlzp_mutex_lock(mlzp_mutex_t *mutex)
{
    MLOGA("[%p] lock\n", mutex);
    if (!mutex) {
        return -1;
    }
   return pthread_mutex_lock(mutex);
}

int mlzp_mutex_trylock(mlzp_mutex_t *mutex)
{
    if (!mutex) {
        return -1;
    }
    return pthread_mutex_trylock(mutex);
}

int mlzp_mutex_unlock(mlzp_mutex_t *mutex)
{
    MLOGA("[%p] unlock\n", mutex);
    if (!mutex) {
        return -1;
    }
    return pthread_mutex_unlock(mutex);
}

int mlzp_cond_init(mlzp_cond_t **cond)
{
    if (!cond) {
        return -1;
    }

    mlzp_cond_t *c = mlzp_malloc(sizeof(mlzp_cond_t));
    if (!c) {
        return -1;
    }
    memset(c, 0, sizeof(mlzp_cond_t));
    int ret = pthread_cond_init(c, NULL);
    if (0 != ret) {
        mlzp_free(c);
        return ret;
    }

    *cond = c;
    return 0;
}

int mlzp_cond_destroy(mlzp_cond_t **cond)
{
    if (!cond || !*cond) {
        return -1;
    }
    (void) pthread_cond_destroy(*cond);
    mlzp_zfree((void**) cond);
    return 0;
}

int mlzp_cond_wait(mlzp_cond_t *cond, mlzp_mutex_t *mutex)
{
    int ret;

    MLOGA("[%p] wait\n", mutex);
    if (!cond || !mutex) {
        return -1;
    }
    ret = pthread_cond_wait(cond, mutex);
    MLOGA("[%p] wait return\n", mutex);

    return ret;
}

int mlzp_cond_timedwait(mlzp_cond_t *cond, mlzp_mutex_t *mutex, const int time_ms)
{
    int ret;
    struct timespec abstime =
        {time_ms / 1000, (time_ms % 1000) * 1000 * 1000};

    MLOGA("[%p] timed wait\n", mutex);
    if (!cond || !mutex) {
        return -1;
    }
    ret = pthread_cond_timedwait(cond, mutex, &abstime);
    MLOGA("[%p] timed wait return\n", mutex);

    return ret;
}

int mlzp_cond_signal(mlzp_cond_t *cond, mlzp_mutex_t *mutex)
{
    int ret;

    (void) mutex;
    MLOGA("[%p] signal\n", mutex);
    if (!cond || !mutex) {
        return -1;
    }
    ret = pthread_cond_signal(cond);
    MLOGA("[%p] signal return\n", mutex);
    return ret;

}

int mlzp_cond_broadcast(mlzp_cond_t *cond, mlzp_mutex_t *mutex)
{
    int ret;

    (void) mutex;
    MLOGA("[%p] broadcast\n", mutex);
    if (!cond || !mutex) {
        return -1;
    }
    ret = pthread_cond_broadcast(cond);
    MLOGA("[%p] broadcast return\n", mutex);
    return ret;
}

int mlzp_thread_create(mlzp_thread_t *tid,
    const char *tname, void *(*entry)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr = {0};

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    ret = pthread_create(tid, &attr, entry, arg);
    MLOGD("[%s] Start thread, id:0x%x\n", tname, *tid);

    return ret;
}

int mlzp_thread_join(mlzp_thread_t tid, const char *tname)
{
    int ret;

    MLOGD("[%s] Join thread, id:0x%x\n", tname, tid);
    ret = pthread_join(tid, NULL);
    MLOGD("[%s] Join thread, id:0x%x sucess!\n", tname, tid);

    return ret;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
