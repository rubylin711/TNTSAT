/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FILE_SEQ_MISC_H__
#define __FILE_SEQ_MISC_H__

#include "mt_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
int run_memory(MT_BOOL flush_cache);
void open_es_dump_file(PLAYBACK_INTERNAL_T *pbi, STREAM_TYPE_E type);
void write_es_dump_file(PLAYBACK_INTERNAL_T *pbi,
    STREAM_TYPE_E type, const void *start_addr, unsigned int size);
void close_es_dump_file(PLAYBACK_INTERNAL_T *pbi, STREAM_TYPE_E type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
