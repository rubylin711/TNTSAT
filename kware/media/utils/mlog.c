/*
 * Montage Technology (Shanghai) Co., Ltd.
 * Montage Proprietary and Confidential
 * Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies
 *
 * Description:mlog definition
 * History:     Date        Author    Modification
 *   1.       2021-05-17  Montage      Create
 */
#define MODULE_TAG "MLOG"
#include "mlog.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include "mutil.h"

#ifdef _WIN_PC_
#include <Windows.h>
#endif

#if MT_DES("Macro Definition", 1)
// #define DEBUG_WITH_TIME
#define MAX_LOG_TAG_SIZE    32
#define MAX_LOG_TIME_SIZE   16
#ifdef _WIN_PC_
#define MAX_LOG_BUF_SIZE   (1024 * 1024)
#else
#define MAX_LOG_BUF_SIZE    1024
#endif

#define MAX_LOG_MSG_SIZE    (MAX_LOG_BUF_SIZE - MAX_LOG_TAG_SIZE - MAX_LOG_TIME_SIZE)
#endif

struct mlog_message {
    char time[MAX_LOG_TIME_SIZE];
    char tag[MAX_LOG_TAG_SIZE];
    char msg[MAX_LOG_MSG_SIZE];
};

int g_player_log_level = MLOG_INFO;
/* static mutex initialize */
static mlzp_mutex_t mutex = MLZP_MUTEX_INIT_VALUE;
#if MT_DES("Internal function", 1)
static mlog_ctrl_t *get_mlog_ctx(void);

static void inline output_message(
    struct mlog_message *log_msg)
{
    mlog_ctrl_t *ctx = get_mlog_ctx();

    if (ctx->fp != NULL) {
        fprintf(ctx->fp, "%s%s%s",
            log_msg->time, log_msg->tag, log_msg->msg);
    } else {
#ifdef _WIN_PC_
        char buf[MAX_LOG_BUF_SIZE] = {0};
        (void) snprintf(buf, MAX_LOG_BUF_SIZE, "%s%s%s",
            log_msg->time, log_msg->tag, log_msg->msg);
        OutputDebugStringA(buf);
#elif 1
        if (strstr(log_msg->msg, "%")) {
            char buf[MAX_LOG_BUF_SIZE] = {0};
            (void) snprintf(buf, MAX_LOG_BUF_SIZE, "%s%s%s",
                log_msg->time, log_msg->tag, log_msg->msg);
            puts(buf);
        } else {
            fprintf(stderr, "%s%s%s",
                log_msg->time, log_msg->tag, log_msg->msg);
        }
#else
    char buf[MAX_LOG_BUF_SIZE] = {0};
    (void) snprintf(buf, MAX_LOG_BUF_SIZE, "%s%s%s",
        log_msg->time, log_msg->tag, log_msg->msg);
    FILE *fp = fopen("/media/sda/mem.txt", "aw+");
    if (fp) {
        fprintf(fp, "%s", buf);
        fclose(fp);
    }
#endif
    }
}

static inline void record_time(struct mlog_message *msg)
{
    unsigned char hour, minute, second;
    unsigned short ms, us;

    mclock_get_clock(&hour, &minute, &second, &ms, &us);
    (void) snprintf(msg->time, MAX_LOG_TIME_SIZE,
        "[%02d:%02d:%02d %03d]", (int) hour, (int) minute, (int) second, (int) ms);
}

static inline void record_tag(
    struct mlog_message *msg, const char label, const char *module)
{

    (void) snprintf(msg->tag, MAX_LOG_TAG_SIZE,
        "[%c] [%s]", label, NULL == module ? "NULL" : module);
}

static void mlog_generic_logger(const char *module,
    const int level, const char *format, va_list arg)
{
    struct mlog_message log_msg = {0};
    const static char linfo[MLOG_MAX] =
        {'V', 'D', 'I', 'W', 'E',  'F', 'S'};

#ifdef DEBUG_WITH_TIME
    record_time(&log_msg);
#endif
    record_tag(&log_msg, linfo[level % MLOG_MAX], module);
    (void) vsnprintf((char *)(&log_msg.msg[0]), MAX_LOG_MSG_SIZE, format, arg);

    output_message(&log_msg);
}

static mlog_ctrl_t *get_mlog_ctx(void)
{
    static mlog_ctrl_t ctrl =
        {MLOG_VERBOSE, NULL, mlog_generic_logger};
    return &ctrl;
}

#endif

static int check_log_level(void)
{
#ifndef _WIN_PC_
    #define LOG_VAR_FILE_NAME "/tmp/player_log_level"
    if(access(LOG_VAR_FILE_NAME, F_OK) == 0) {

        FILE *fp = fopen(LOG_VAR_FILE_NAME, "r");
        if (!fp) {
            return 0;
        }
        /* at most 2digital number */
        char buf[3] = {0};
        fread(buf, sizeof(buf) - 1, 1, fp);
        fclose(fp);
        fp = NULL;

        int level = atoi(buf);
        system("rm -f "LOG_VAR_FILE_NAME);
        if (level >= MLOG_VERBOSE && level < MLOG_MAX) {
            g_player_log_level = level;
        } else if (level >= 16 && level <= 56) { /* AV_LOG_ERROR AV_LOG_TRACE for ffmpeg */
            return level;
        }
    }
#endif
    return 0;
}

#if MT_DES("External API Definition", 1)
int mlog_check_gobal_log_level(void)
{
    return check_log_level();
}

void mlog_printf(const char *module, const int level, const char *format, ...)
{
    mlog_ctrl_t *ctx = get_mlog_ctx();

    if (NULL == format || level < ctx->level) {
        return;
    }

    pthread_mutex_lock(&mutex);
    if (ctx->cb) {
        va_list arg;
        va_start(arg, format);
        ctx->cb(module, level, format, arg);
        va_end(arg);
    }
    pthread_mutex_unlock(&mutex);
}

void mlog_set_controller(mlog_ctrl_t *ctrl)
{
    mlog_ctrl_t *ctx = get_mlog_ctx();

    if (NULL == ctrl) {
        return;
    }

    pthread_mutex_lock(&mutex);
    *ctx = *ctrl;
    pthread_mutex_unlock(&mutex);
}

mlog_ctrl_t *mlog_get_controller(void)
{
    return get_mlog_ctx();
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
