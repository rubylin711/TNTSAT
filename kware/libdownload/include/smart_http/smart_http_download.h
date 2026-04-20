/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SMART_HTTP_DOWNLOAD_H__
#define __SMART_HTTP_DOWNLOAD_H__

#include "download_group.h"
#include "jobs.h"
#include  "multiqueue.h"

typedef enum{
  SCHEDULE_IDLE = 0,
  SCHEDULE_RUNNING = 1,
  SCHEDULE_PREPARE_EXIT = 2,
  SCHEDULE_EXIT_DONE = 3,
  SCHEDULE_PREPARE_IDLE = 4,
  SCHEDULE_SEEK = 5,
}SCHEDULE_TASK_STATUS;

typedef struct {
    MULTI_QUEUE_HANDLE* gp_multiqueue;
    void *gp_download_group;
    int g_prio[5];
    int mem_block_num;
    int mem_block_size;
    int seek_off;//absolute offset in file
    SCHEDULE_TASK_STATUS schedule_task_status;
    JOBS_DESCRIPTION *p_jobs_desc;
    char *p_schedule_task_stack;
    char *http_extra_header;
    MT_BOOL init_flag;//TRUE- init ok  FALSE-- not init
    MT_BOOL speed_calc;
    MT_BOOL live_stream;
    MT_BOOL reconnect;
    u32 max_speed;
    u32 real_speed;
    u32 count_start_time;

    int filesize;
    int off;
    char seekable;
} SmartHTTPGlobalContext;

typedef enum
{
    SMART_HTTP_DOWNLOAD_SEEK_SIZE,//get file size,
    SMART_HTTP_DOWNLOAD_SEEK_CUR,//seek cur
    SMART_HTTP_DOWNLOAD_SEEK_ABS,//absolute position
    SMART_HTTP_DOWNLOAD_SEEK_END,// seek end
}SMART_HTTP_DOWNLOAD_SEEK_MODE;

#define SMART_HTTP_DOWNLOAD_EOF     -123

SmartHTTPGlobalContext* Smart_Http_Download_Init(int prio[5]);

void Smart_Http_Download_Deinit(SmartHTTPGlobalContext *handle);

int Smart_Http_Download_Open(SmartHTTPGlobalContext *handle, const char *uri, char *extra_header, MT_BOOL live_stream,
    MT_BOOL reconnect);

int Smart_Http_Download_Read(SmartHTTPGlobalContext *handle, unsigned char *buf, int size);

int Smart_Http_Download_Seek(SmartHTTPGlobalContext *handle, int off, SMART_HTTP_DOWNLOAD_SEEK_MODE seek_mode);

int Smart_Http_Download_Close(SmartHTTPGlobalContext *handle);

MT_BOOL Smart_Http_Download_Get_Speed(SmartHTTPGlobalContext *handle, u32 *speed_max_Bps, u32 * speed_real_Bps);

#endif

