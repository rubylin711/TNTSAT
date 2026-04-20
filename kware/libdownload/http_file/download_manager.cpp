/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mtos_mem.h"
#include "mtos_sem.h"
#include "mtos_printk.h"
#include "mtos_task.h"

#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/file.h>
#include "stdio.h"
#define mtos_printk printf
#define OS_PRINTF


#include "mt_type.h"

#include <assert.h>

#include "download_manager.h"
#include "download_api.h"

#define MAX_TASK_PRO 256

#define   DOWNLOAD_MANAGER_LOG(format, ...)                 OS_PRINTF(format, ##__VA_ARGS__)
#define   DOWNLOAD_MANAGER_DEBUG(format, ...)            OS_PRINTF(format, ##__VA_ARGS__)
#define   DOWNLOAD_MANAGER_ERROR(format, ...)           mtos_printk(format, ##__VA_ARGS__)

static DownloadManager download_manager[MAX_TASK_PRO];
static unsigned int dl_mgr_lock = 0;

#undef MT_ASSERT
#define MT_ASSERT(__x)	\
	do{	\
		if(!(__x)) {	\
			printf("%s(%s, %d ): ASSERT(%s) failed\n", __FUNCTION__, __FILE__, __LINE__, #__x);	\
			while(1);	\
		}	\
	}while(0)

static void take_the_lock()
{
#if 0
    static int prio_is_lock_init = 0;
    MT_BOOL ret = FALSE;
    if(prio_is_lock_init == 0)
    {
        ret = mtos_sem_create(&dl_mgr_lock, TRUE);
        MT_ASSERT(ret == TRUE);
        prio_is_lock_init = 1;
    }
    mtos_sem_take((os_sem_t *)(&dl_mgr_lock), 0);
#endif
}
static void give_the_lock()
{
    //mtos_sem_give((os_sem_t *)(&dl_mgr_lock));
}

//extern void   Nw_Download_Abort(http_download_mini * instance);

void http_download_mamager_init()
{
#if 0
    int i;

    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);
    for(i = 0; i < MAX_TASK_PRO; i++)
    {
        download_manager[i].abort_flag = FALSE;
        download_manager[i].task_pro = -1;
    }

    DOWNLOAD_MANAGER_LOG("[%s] end end... ...\n", __func__);
	#endif
}

void   add_task_to_manager(int task_prio, MT_BOOL abort_flag, http_download_mini * instance)
{
#if 0
    take_the_lock();
    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);

    download_manager[task_prio].abort_flag = abort_flag;
    download_manager[task_prio].task_pro = task_prio;
    download_manager[task_prio].instance= instance;

    DOWNLOAD_MANAGER_LOG("[%s] end end... ...\n", __func__);
    give_the_lock();
#endif
}

void   remove_task_from_manager(int task_prio)
{
#if 0
    take_the_lock();
    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);

    download_manager[task_prio].abort_flag = FALSE;
    download_manager[task_prio].task_pro = -1;

    DOWNLOAD_MANAGER_LOG("[%s] end end... ...\n", __func__);
    give_the_lock();
#endif
}

MT_BOOL   check_task_download_running(int task_prio)
{
#if 0
    take_the_lock();
    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);

    if(download_manager[task_prio].task_pro > 0)
    {
        DOWNLOAD_MANAGER_LOG("[%s] TRUE done done ... ...\n", __func__);
        give_the_lock();
        return TRUE;
    }
    else
    {
        DOWNLOAD_MANAGER_LOG("[%s] FALSE done done ... ...\n", __func__);
        give_the_lock();
        return FALSE;
    }
#endif
    return 0;
}

void get_running_tasks_from_manager(int * task_list)
{
#if 0
    take_the_lock();
    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);

    int i = 0, running_tasks[MAX_TASK_PRO] = {0};

    for(i = 0; i < MAX_TASK_PRO; i++)
    {
        if(download_manager[i].task_pro > 0)
        {
            *task_list++ = download_manager[i].task_pro;
        }
    }
    DOWNLOAD_MANAGER_LOG("[%s] end end... ..\n", __func__);

    give_the_lock();
#endif
}

void   abort_download_task(int task_prio, MT_BOOL abort_flag)
{
#if 0
    take_the_lock();

    DOWNLOAD_MANAGER_LOG("[%s] start start... ...\n", __func__);
    if(abort_flag && download_manager[task_prio].task_pro > 0)
    {
        Nw_Download_Abort(download_manager[task_prio].instance);

        //remove the task
        download_manager[task_prio].abort_flag = FALSE;
        download_manager[task_prio].task_pro = -1;
    }
    DOWNLOAD_MANAGER_LOG("[%s] end end... ...\n", __func__);

    give_the_lock();
#endif
}


