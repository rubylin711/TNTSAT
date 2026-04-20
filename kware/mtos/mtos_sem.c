/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mtos_sem.h"
#include "mtos_printk.h"

#include "drv_os.h"

#ifdef DRV_SEM_DEBUG
MT_BOOL mtos_sem_create(os_sem_t *p_sem, MT_BOOL mutex, const u8 *p_name)
#else
MT_BOOL mtos_sem_create(os_sem_t *p_sem, MT_BOOL mutex)
#endif
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if (p_sem == NULL) {
	mtos_printk("[mtos_sem_create] param is ERROR!\n");
	return FALSE;
    }

#ifdef DRV_SEM_DEBUG
    err = SYS_SemCreate((U32)mutex, (ulong *)p_sem, p_name);
#else
    err = SYS_SemCreate((U32)mutex, (ulong *)p_sem);
#endif
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("\n##[mtos_sem_create] create sem ERROR[%x]!\n", err);
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_sem_destroy(os_sem_t *p_sem, u8 opt)
{
    //check param
    if (p_sem == NULL) {
	mtos_printk("[mtos_sem_destroy] sem is NULL!\n");
	return FALSE;
    }

    (void)SYS_SemDel(*((ulong *)p_sem));

    return TRUE;
}

MT_BOOL mtos_sem_give(os_sem_t *p_sem)
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if (p_sem == NULL) {
	mtos_printk("[mtos_sem_give] sem is NULL!\n");
	return FALSE;
    }

    if ((*p_sem) == 0) {
	mtos_printk("[mtos_sem_give] param is ERROR!\n");
	return FALSE;
    }

    err = SYS_SemSend(*((ulong *)p_sem));
    if (err != ERROR_CODE_NO_ERROR) {
	mtos_printk("[mtos_sem_give] give sem ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_sem_take(os_sem_t *p_sem, u32 ms)
{
    S32 timeout = (S32)ms;
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if (p_sem == NULL) {
	mtos_printk("[mtos_sem_take] sem is NULL!\n");
	return FALSE;
    }

    if ((*p_sem) == 0) {
	mtos_printk("[mtos_sem_give] param is ERROR!\n");
	return FALSE;
    }

    if (ms == 0) {
	timeout = (S32)SYS_TIMEOUT_INFINITY;
    }

    err = SYS_SemWait(*((ulong *)p_sem), timeout);
    if (err != ERROR_CODE_NO_ERROR) {
	//mtos_printk("[mtos_sem_take] take sem ERROR!\n");
	return FALSE;
    }

    return TRUE;
}

MT_BOOL mtos_sem_query(os_sem_t *p_sem, u32 *p_cnt, MT_BOOL *p_pending)
{
    return TRUE;
}

void mtos_sem_debug(void)
{
#ifdef DRV_SEM_DEBUG
    SYS_SemDebug();
#endif
    return;
}

MT_BOOL mtos_sem_trytake(os_sem_t *p_sem)
{
    ErrorCode_t err = ERROR_CODE_ERROR_RESULT;

    //check param
    if (p_sem == NULL) {
		mtos_printk("[mtos_sem_trytake] sem is NULL!\n");
		return FALSE;
    }

    if ((*p_sem) == 0) {
		mtos_printk("[mtos_sem_trytake] param is ERROR!\n");
		return FALSE;
    }

    err = SYS_SemWait(*((ulong *)p_sem), 0);
    if (err != ERROR_CODE_NO_ERROR) {
		//mtos_printk("[mtos_sem_trytake] take sem ERROR!\n");
		return FALSE;
    }

    return TRUE;
}

