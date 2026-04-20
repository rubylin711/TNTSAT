/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"

#include "mtos_event.h"
#include "mtos_sem.h"
#include "mtos_printk.h"

#include "drv_os.h"

MT_BOOL mtos_event_create(mtos_evt_t *p_event)
{
    os_sem_t sem = 0;
    MT_BOOL ret = FALSE;

    //check param
    if (p_event == NULL) {
	mtos_printk("[mtos_event_create] param is ERROR!\n");
	return FALSE;
    }

#ifdef DRV_SEM_DEBUG
    ret = mtos_sem_create(&sem, TRUE, (const u8 *)"mtos_event");
#else
    ret = mtos_sem_create(&sem, TRUE);
#endif
    if (ret != TRUE) {
	mtos_printk("[mtos_event_create] create event ERROR!\n");
	return FALSE;
    }

    (*p_event) = (mtos_evt_t)sem;

    return TRUE;
}

MT_BOOL mtos_event_destroy(mtos_evt_t *p_event)
{
    os_sem_t sem;

    //check param
    if (p_event == NULL) {
	mtos_printk("[mtos_event_destroy] event is NULL!\n");
	return FALSE;
    }

    sem = (os_sem_t)(*p_event);

    (void)mtos_sem_destroy(&sem, 0);

    return TRUE;
}

MT_BOOL mtos_event_wait(mtos_evt_t *p_event,
                     unsigned int type,
                     unsigned int flag,
                     unsigned int *p_value,
                     unsigned int timeout)
{
    u32 ms = 0;
    MT_BOOL ret = FALSE;
    os_sem_t sem;

    //check param
    if (p_event == NULL) {
	mtos_printk("[mtos_event_wait] event is NULL!\n");
	return FALSE;
    }

    sem = (os_sem_t)(*p_event);

    if (timeout == MTOS_WAIT_FOREVER) {
	ms = 0;
    } else {
	ms = (u32)timeout;
    }

    ret = mtos_sem_take(&sem, ms);
    if (ret != TRUE) {
	mtos_printk("[mtos_event_wait] wait event ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_event_set(mtos_evt_t *p_event, unsigned int mask)
{
    MT_BOOL ret = FALSE;
    os_sem_t sem;

    //check param
    if (p_event == NULL) {
	mtos_printk("[mtos_event_set] event is NULL!\n");
	return FALSE;
    }

    sem = (os_sem_t)(*p_event);

    ret = mtos_sem_give(&sem);
    if (ret != TRUE) {
	mtos_printk("[mtos_sem_give] give sem ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_event_query(mtos_evt_t *p_event, unsigned int *mask)
{
    return TRUE;
}
