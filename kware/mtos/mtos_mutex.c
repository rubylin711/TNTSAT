/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mtos_mutex.h"
#include "mtos_sem.h"
#include "mtos_printk.h"

#include "drv_os.h"

void *mtos_mutex_create(unsigned int prio)
{
    os_sem_t mutex = 0;
    MT_BOOL ret = FALSE;

#ifdef DRV_SEM_DEBUG
    ret = mtos_sem_create(&mutex, TRUE, (const u8 *)"mtos_mutex");
#else
    ret = mtos_sem_create(&mutex, TRUE);
#endif
    if (ret != TRUE) {
	mtos_printk("[mtos_mutex_create] create mutex ERROR!\n");
	return NULL;
    }

    return (void *)mutex;
}

MT_BOOL mtos_mutex_delete(void *p_mutex)
{
    MT_BOOL ret = FALSE;
    os_sem_t sem = (os_sem_t)p_mutex;

    if (p_mutex == NULL) {
	mtos_printk("[mtos_mutex_delete] mutex is NULL!\n");
	return FALSE;
    }

    ret = mtos_sem_destroy(&sem, 0);
    if (ret != TRUE) {
	mtos_printk("[mtos_mutex_delete] delete mutex ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_mutex_give(void *p_mutex)
{
    MT_BOOL ret = FALSE;
    os_sem_t sem = (os_sem_t)p_mutex;

    if (p_mutex == NULL) {
	mtos_printk("[mtos_mutex_give] mutex is NULL!\n");
	return FALSE;
    }

    ret = mtos_sem_give(&sem);
    if (ret != TRUE) {
	mtos_printk("[mtos_mutex_give] give mutex ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_mutex_take(void *p_mutex)
{
    MT_BOOL ret = FALSE;
    os_sem_t sem = (os_sem_t)p_mutex;

    if (p_mutex == NULL) {
	mtos_printk("[mtos_mutex_take] mutex is NULL!\n");
	return FALSE;
    }

    ret = mtos_sem_take(&sem, 0);
    if (ret != TRUE) {
	mtos_printk("[mtos_mutex_take] take mutex ERROR!\n");
	return FALSE;
    }

    return TRUE;
}
