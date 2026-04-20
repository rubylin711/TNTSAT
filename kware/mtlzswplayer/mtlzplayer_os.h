/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : mtlzplayer_os.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player OS porting functions.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/

#ifndef __INC_MTLZPLAYER_OS_H__
#define __INC_MTLZPLAYER_OS_H__

#ifdef __cplusplus
#if __cplusplus
	 extern "C"{
#endif
#endif /* __cplusplus */

void *MTLZ_MALLOC(size_t size);
void MTLZ_FREE(void *ptr);
//void MTLZ_ASSERT_DEBUG(bool expr, const char *function, int line);
int mtlz_msleep(unsigned int msec);
unsigned long mtlz_get_tick(void);
int mtlz_thread_set_name(const char *thread_name);
int mtlz_thread_create(unsigned int *thread_id, void *(*start_routine)(void *), void *arg);
int mtlz_thread_join(unsigned int thread_id);
int mtlz_mutex_init(void **mutex);
int mtlz_mutex_destroy(void *mutex);
int mtlz_mutex_lock(void *mutex);
int mtlz_mutex_unlock(void *mutex);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

