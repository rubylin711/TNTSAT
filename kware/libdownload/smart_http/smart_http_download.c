/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include "mt_common.h"
#include "smart_http_download.h"

//#define DEBUG_LOG

#ifndef __LINUX__
#ifdef DEBUG_LOG
#define OS_PRINTF(format, ...)              mt_dbg_printf(MT_DBG_SYS_DEF,format, ##__VA_ARGS__)
#else
#define OS_PRINTF(format, ...)
#endif
#define ERR_PRINTF(format, ...)              mt_dbg_printf(MT_DBG_ALWAYS_ON,format, ##__VA_ARGS__)
#else
#ifdef DEBUG_LOG
#define OS_PRINTF(format, ...)              printf(format, ##__VA_ARGS__)
#else
#define OS_PRINTF(format, ...)
#endif
#define ERR_PRINTF(format, ...)              printf(format, ##__VA_ARGS__)
#endif



typedef struct {
    DATA_SEGMENT_QUEUE *p_queue;
    DL_TS_SEG_JOB *p_job_desc;
    int task_index;
} SmartHTTPCurrentTaskContext;





#define SEGMENT_DOWNLOAD_STACK_SIZE     (64*1024)
#define DELAY_TIME  (10)
void Smart_Http_Download_Enable_Speed_Calculate(SmartHTTPGlobalContext *handle, MT_BOOL enable);
static MT_BOOL smart_http_download_gen_speed(SmartHTTPGlobalContext *handle, int job_index, u32 begin_time, int interval_len);
static void smart_http_download_init_speed(SmartHTTPGlobalContext *handle);

static MT_BOOL mtos_task_create(u8   *p_taskname,
                     void (*p_taskproc)(void *p_param),
                     void  *p_param,
                     u32    nprio,
                     u32   *pstk,
                     u32    nstksize)
{

    pthread_t              thread;
    int                   ret = 0;
    pthread_attr_t attribs;
    struct sched_param taskschedparam;


    if(nstksize < 16*1024)
    {
      nstksize = 16*1024;
    }

    ret = pthread_attr_init(&attribs);
    if (ret != 0)
    {
      return FALSE;
    }

    ret = pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
    if (ret != 0)
    {
      return FALSE;
    }


    if( strcmp("initT1",(const char*)p_taskname)==0 )
    {

      ret = pthread_attr_setinheritsched(&attribs, PTHREAD_EXPLICIT_SCHED);
      if (ret != 0)
      {
        return FALSE;
      }

      ret = pthread_attr_setstacksize(&attribs, nstksize);
      if (ret != 0)
      {
        return FALSE;
      }

      /*set scheduling policy*/
      ret = pthread_attr_setschedpolicy(&attribs, SCHED_FIFO);
      if (ret != 0)
      {
        return FALSE;
      }

      /*Set task priority*/
      taskschedparam.sched_priority =  100-nprio;


      ret = pthread_attr_setschedparam(&attribs, &taskschedparam);
      if (ret != 0)
      {
        return FALSE;
      }
    }

    ret = pthread_create(&thread, &attribs, (void *)p_taskproc, p_param);
    if (ret != 0)
    {
      exit(-1);
    }

    return TRUE;

}
static unsigned int mtos_task_exit(void)
{
   pthread_exit(NULL);
	return 0;
}
static void mtos_task_sleep(u32 ms)
{
  MT_USLEEP(ms * 1000);
}

static MT_BOOL get_write_buf(unsigned int cbParam, void *queue, int task_index, WRITE_POS_INFO * p_info)
{
    int ret;
    DATA_SEGMENT_QUEUE *p_queue = (DATA_SEGMENT_QUEUE*)queue;
    OS_PRINTF("smh get buf start, task %d, job id %d, q %d,  w %d, r %d\n",task_index,p_queue->id, p_queue->job_id,
        p_queue->write_pos,p_queue->read_pos);
    ret = queue_query_wp(p_queue, p_info);
    if(ret)
        return FALSE;
//OS_PRINTF("get_write_buf %x %d, task_index %d, job_index %d, job_id %d\n",p_info->w_start_addr,p_info->max_w_byte,
//    task_index, g_Smart_Http.schedule[task_index].p_job_desc->job_index,
//    g_Smart_Http.schedule[task_index].p_job_desc->job_id);
    return TRUE;
}

static void write_buf_update(ulong cbParam, void *queue,  int task_index, int len)
{
    ulong p_working_queue = (ulong)cbParam;
    DATA_SEGMENT_QUEUE *p_queue = (DATA_SEGMENT_QUEUE*)queue;
//OS_PRINTF("write_buf_update task_index %d,q %d, working queue %x\n",task_index,p_queue->id,p_working_queue);
    update_queue_writePosition(p_queue, len, p_working_queue);
//    OS_PRINTF("write done, wp %d,rp %d,len %d\n",
//        g_Smart_Http.schedule[task_index].p_queue->write_pos,
//        g_Smart_Http.schedule[task_index].p_queue->read_pos,
//        g_Smart_Http.schedule[task_index].p_queue->len);
    return;
}
static void write_buf_full(ulong cbParam, void *queue, u32 begin_time, int buf_used_len)
{
    SmartHTTPGlobalContext *handle = (SmartHTTPGlobalContext*)cbParam;
    DATA_SEGMENT_QUEUE *p_queue = (DATA_SEGMENT_QUEUE*)queue;
    if(!p_queue)
        return;

    DL_TS_SEG_JOB *p_job_desc = NULL;
    p_job_desc = Get_Job_From_Index(handle->p_jobs_desc, p_queue->job_id);

    Set_Job_Begin_Time(p_job_desc, begin_time);
    if(handle->p_jobs_desc && Get_Jobs_Single_Mode(handle->p_jobs_desc))
        smart_http_download_gen_speed(handle, p_queue->job_id, begin_time, buf_used_len);
}

extern u32 jobs_get_time_ms();
static void download_done(ulong cbParam1, unsigned int cbParam2, void *queue, int task_index, MT_BOOL success)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    DL_STATUS status;
    int job_id;
    u32 p_working_queue = (u32)cbParam2;
    SmartHTTPGlobalContext *handle = (SmartHTTPGlobalContext*)cbParam1;
    DATA_SEGMENT_QUEUE *p_queue = (DATA_SEGMENT_QUEUE*)queue;

OS_PRINTF("smh download_done .................................task_index %d, success %d\n",task_index,success);
    if(success)
        status = DOWNLOAD_SUCCESS;
    else
        status = DOWNLOAD_ABORT;


    job_id = p_queue->job_id;//should save it before change queue status,

    OS_PRINTF("smh download_done ...............................queue id %d,job_id %d,r pos %d, w pos %d, len %d\n",
        p_queue->id,p_queue->job_id,p_queue->read_pos,
        p_queue->write_pos,p_queue->len);
    notify_queue_downloaderStatus(p_queue, status, p_working_queue);

    p_job_desc = Get_Job_From_Id(handle->p_jobs_desc, job_id);
    if(!p_job_desc)
        return;

    Complete_Job(handle->p_jobs_desc, p_job_desc->job_index, success);

    if(handle->p_jobs_desc && Get_Jobs_Single_Mode(handle->p_jobs_desc) == FALSE)
    {
        smart_http_download_gen_speed(handle, job_id, 0, 0);
    }

    if(handle->p_jobs_desc->finish_flag)
    {
        smart_http_download_gen_speed(handle, handle->p_jobs_desc->job_total-1, 0, 0);
    }
    return;
}
static void reset_multiqueue_and_download_group(SmartHTTPGlobalContext *handle)
{
    Reset_Download_Group(handle->gp_download_group);
    mutiQueue_reset(handle->gp_multiqueue);
    return;
}
static MT_BOOL assign_job_and_download(SmartHTTPGlobalContext *handle, DATA_SEGMENT_QUEUE *p_queue, int task_index, DL_TS_SEG_JOB *p_job_desc)
{
    FILE_RANGE seg_range;

    //////////////////////////////////set queue
    OS_PRINTF("----set %d,size %d\n",p_job_desc->job_id,p_job_desc->seg_size);
    set_queue_job_id(p_queue,p_job_desc->job_id);
    seg_range.start_pos = p_job_desc->offset;
    seg_range.end_pos = p_job_desc->offset+p_job_desc->seg_size-1;
    set_queue_range(p_queue,seg_range);
    OS_PRINTF("---set queue[id=%d] job id %d,range %d-%d\n",p_queue->id,p_queue->job_id,seg_range.start_pos,seg_range.end_pos);

    //////////////////////////////////assign job//////////////////////////////////////////
    if(Assign_Job(handle->p_jobs_desc, p_job_desc->job_index) == FALSE)
    {
        return FALSE;
    }

    OS_PRINTF("[smart_http]assign job %d done\n",p_job_desc->job_id);
    //////////////////////////////////assign task//////////////////////////////////////////
    JOBS_DESCRIPTION * p_jobs_d = NULL;
	p_jobs_d = (JOBS_DESCRIPTION *)p_job_desc->p_jobs_desc;
	int cont_len = 0;
	cont_len = p_jobs_d->content_length;
    if(Assign_Download_Task(handle->gp_download_group, (unsigned long)handle, task_index, handle->p_jobs_desc->url,
        p_job_desc->offset, p_job_desc->seg_size,cont_len ,
        Get_Job_Download_Handle(p_job_desc), (void*)p_queue,
        handle->p_jobs_desc->reconnect ? DOWNLOAD_MODE_ERROR_REDO:DOWNLOAD_MODE_ERROR_SKIP) == FALSE)
    {
        Assign_Job_Reset(handle->p_jobs_desc, p_job_desc->job_index);
        return FALSE;
    }

    return TRUE;
}


#define MIN(a,b) ((a) < (b) ? (a) : (b))
typedef struct{
    int  kill_jobs[MAX_QUEUE_NUM];/*these jobs should be removed from downloader*/
    int  kill_total;
    int  new_jobs[MAX_QUEUE_NUM];/*append new jobs*/
    int  new_total;
    int  save_jobs[MAX_QUEUE_NUM];/*these jobs should be saved in queue before read operation, to avoid change queue status when take one queue*/
    int  save_total;
    MT_BOOL reset;
}SEEK_POLICY_T;

static MT_BOOL sort_job_id_in_array(JOB_IDS_ARRAY *job_id_array, int *job_id_min, int *job_id_max)
{
    int i;
    if(!job_id_array || job_id_array->totalNum <= 0)
        return FALSE;

    *job_id_min = (int)65536;
    *job_id_max = 0;

    for(i=0; i<job_id_array->totalNum; i++)
    {
        if(*job_id_min > job_id_array->id[i])
            *job_id_min = job_id_array->id[i];

        if(*job_id_max < job_id_array->id[i])
            *job_id_max = job_id_array->id[i];
    }

    return TRUE;
}
static int look_for_task_in_schedule(SmartHTTPGlobalContext *handle, int job_id)
{
    int i;
    DATA_SEGMENT_QUEUE *p_queue = NULL;
    DL_TS_SEG_JOB *p_job_desc = NULL;

    for(i=0; i<MAX_DL_TASK_NUM; i++)
    {
        p_queue = Get_Queue_From_Download_Group(handle->gp_download_group, i);
        //if(p_queue)
        //    mtos_printk("task %d, q id %d, job id %d\n",i,p_queue->id,p_queue->job_id);
        if(p_queue && p_queue->job_id == job_id)
        {
            return i;
        }
    }

    return -1;
}
/*************************************************************************************************************************************************************
**********************************************************************************************************************************
*                  *            *J,J+1,J+2,.....J+n*
**********************************************************************************************************************************

*********************************************************
*                  *       new     *    save        *     kill       *                   *
*********************************************************
**************************************************************************************************************************************************************/
static MT_BOOL  gen_seek_policy(int dst_job_id/*int*/,
	                          JOB_IDS_ARRAY *job_id_array,
	                          SEEK_POLICY_T * p_seek_policy/*out*/)
{
    int min_job_id =0;
    int max_job_id =0;
    int i = 0;

    //OS_PRINTF("[%s] in\n",__func__);
    if(job_id_array->totalNum == 0 || !sort_job_id_in_array(job_id_array,&min_job_id, &max_job_id))
    {
        p_seek_policy->reset = TRUE;
        return TRUE;
    }

    p_seek_policy->reset = FALSE;
    ERR_PRINTF("smh in, dst %d, min %d, max %d\n",dst_job_id, min_job_id, max_job_id);


    if(dst_job_id >= min_job_id && dst_job_id <= max_job_id )
    {
        //do nothing
        p_seek_policy->kill_total = 0;
        p_seek_policy->new_total = 0;
        p_seek_policy->save_total = 0;
        #if 0
        p_seek_policy->save_total = job_id_array->totalNum;
        for(i=0; i<job_id_array->totalNum; i++)
        {
            p_seek_policy->save_jobs[i] = job_id_array->id[i];
            OS_PRINTF("[%s] save %d\n",__func__,p_seek_policy->save_jobs[i]);
        }
        #endif
    }
    else if(dst_job_id > max_job_id)
    {
        //todo:
        #if 0
        p_seek_policy->new_total = 0;
        p_seek_policy->save_total = 0;
        p_seek_policy->kill_total = job_id_array->totalNum;
        for(i=0; i<job_id_array->totalNum; i++)
        {
            p_seek_policy->kill_jobs[i] = job_id_array->id[i];
            OS_PRINTF("[%s] kill %d\n",__func__,p_seek_policy->kill_jobs[i]);
        }
        #else
        p_seek_policy->reset = TRUE;
		ERR_PRINTF("smh[%s] reset 111\n",__func__);
        return TRUE;
        #endif
    }
    else if(dst_job_id < min_job_id && dst_job_id >= (min_job_id - (MAX_DL_TASK_NUM-1)))
    {
        //todo:
        p_seek_policy->new_total = min_job_id - dst_job_id;
        p_seek_policy->save_total = MIN(MAX_DL_TASK_NUM-p_seek_policy->new_total, job_id_array->totalNum);
        p_seek_policy->kill_total = job_id_array->totalNum-p_seek_policy->save_total;
        ERR_PRINTF("smh[%s] number new %d, save %d, kill %d\n",__func__,p_seek_policy->new_total,p_seek_policy->save_total,p_seek_policy->kill_total);

        for(i=0;i<p_seek_policy->new_total; i++)
        {
            p_seek_policy->new_jobs[i] = dst_job_id+i;
            OS_PRINTF("smh[%s] new %d\n",__func__,p_seek_policy->new_jobs[i]);
        }

        for(i=0;i<p_seek_policy->save_total; i++)
        {
            p_seek_policy->save_jobs[i] = min_job_id+i;
            OS_PRINTF("smh[%s] save %d\n",__func__,p_seek_policy->save_jobs[i]);
        }

        for(i=0;i<p_seek_policy->kill_total; i++)
        {
            p_seek_policy->kill_jobs[i] = min_job_id+p_seek_policy->save_total+i;
            OS_PRINTF("smh[%s] kill:%d\n",__func__,p_seek_policy->kill_jobs[i]);
        }


    }
    else //if(dst_job_id < (min_job_id - (MAX_DL_TASK_NUM-1)))
    {
        //todo:
        p_seek_policy->reset = TRUE;
        ERR_PRINTF("smh[%s] reset222\n",__func__);
    }

    return TRUE;
}


static void excute_seek_policy(SmartHTTPGlobalContext *handle, int dst_job_id/*in*/, int dst_job_index/*in*/, SEEK_POLICY_T * p_seek_policy/*in*/)
{
    int i;
    int curr_valid_job_index = 0;
    int task_index;
    DL_TS_SEG_JOB *p_job_desc = NULL;
    DATA_SEGMENT_QUEUE *p_queue = NULL;

    if(!p_seek_policy)
        return;

    if(p_seek_policy->reset)
    {
        Reset_Download_Group(handle->gp_download_group);
        mutiQueue_clear_queues(handle->gp_multiqueue);

        p_job_desc = Get_Next_Job(handle->p_jobs_desc);
        if(p_job_desc)
            curr_valid_job_index = p_job_desc->job_index;
        else
            curr_valid_job_index = handle->p_jobs_desc->job_total-1;

        //OS_PRINTF("smart_http.c [%s] seek job_index %d, current job_index %d\n",__func__,dst_job_index,curr_valid_job_index);
        if(dst_job_index < curr_valid_job_index)
        {
            Reactive_Jobs(handle->p_jobs_desc, dst_job_index,
                p_job_desc?curr_valid_job_index-1:curr_valid_job_index);
            OS_PRINTF("smh seek_process reactive0 %d-%d\n",dst_job_index,
                p_job_desc?curr_valid_job_index-1:curr_valid_job_index);
        }
        else if(dst_job_index > curr_valid_job_index)
        {
            for(i=curr_valid_job_index; i<dst_job_index;i++)
            {
                Complete_Job(handle->p_jobs_desc, i, FALSE);
            }
            OS_PRINTF("smh seek_process complete %d-%d\n",curr_valid_job_index,
                dst_job_index);
            Reactive_Job(handle->p_jobs_desc, dst_job_index);
            OS_PRINTF("smh seek_process reactive1 %d\n",dst_job_index);
        }
        else
        {
            Reactive_Job(handle->p_jobs_desc, dst_job_index);
            OS_PRINTF("smh seek_process reactive2 %d\n",dst_job_index);
        }

        Start_Download_Group(handle->gp_download_group, handle->p_jobs_desc->header);
    }
    else
    {
        if(p_seek_policy->kill_total)
        {
            for(i=0; i<p_seek_policy->kill_total; i++)
            {
                p_job_desc = Get_Job_From_Id(handle->p_jobs_desc, p_seek_policy->kill_jobs[i]);
                //mtos_printk("kill  job id %d,i=%d, addr %x\n",p_seek_policy->kill_jobs[i],i,p_job_desc);
                //kill download
                task_index = look_for_task_in_schedule(handle, p_seek_policy->kill_jobs[i]);
                //mtos_printk("kill task index %d\n",task_index);
                if(task_index >= 0)
                {
                	OS_PRINTF("smh [%s]kill task index:%d\n",__func__,task_index);
                    Stop_Download_Task_In_Group(handle->gp_download_group, task_index);
                }

                p_queue = mutiQueue_get_one_queue(handle->gp_multiqueue, p_seek_policy->kill_jobs[i]);
                //kill multiqueue
                //FIX: compile error
                //mutiQueue_reset_one_queue(handle->p_jobs_desc,p_queue);
                mutiQueue_reset_one_queue(handle->gp_multiqueue,p_queue);

                //kill job management
                Reactive_Job(handle->p_jobs_desc, p_job_desc->job_index);
                OS_PRINTF("smh seek_process reactive3 %d\n",p_job_desc->job_id);

                OS_PRINTF("smh [%s] kill done %d, q id %d\n",__func__,p_seek_policy->kill_jobs[i],
                    p_queue->id);
            }
        }

        if(p_seek_policy->save_total)
        {
            for(i=0; i<p_seek_policy->save_total; i++)
            {
                p_queue = mutiQueue_get_one_queue(handle->gp_multiqueue, p_seek_policy->save_jobs[i]);
                mutiQueue_reuse_one_queue(handle->gp_multiqueue, p_queue);
                OS_PRINTF("smh [%s] save done %d,q id %d\n",__func__,p_seek_policy->save_jobs[i],p_queue->id);
            }
        }

        if(p_seek_policy->new_total)
        {
            for(i=0; i<p_seek_policy->new_total; i++)
            {
                p_job_desc = Get_Job_From_Id(handle->p_jobs_desc, p_seek_policy->new_jobs[i]);
                Reactive_Job(handle->p_jobs_desc, p_job_desc->job_index);
                OS_PRINTF("smh seek_process reactive4 %d\n",p_job_desc->job_index);

                if(Find_Idle_Download_Task(handle->gp_download_group, &task_index) == FALSE)
                {
                    ERR_PRINTF("smh [%s] job_id %d, look for task error\n",__func__,p_seek_policy->new_jobs[i]);
                }
                else
                {
                    p_queue = mutiQueue_take_one_queue(handle->gp_multiqueue);
                    if(p_queue)
                    {
                        assign_job_and_download(handle, p_queue, task_index, p_job_desc);
                        OS_PRINTF("smh seek_process  assign task_index %d, job_index %d,job_id %d,q id %d\n",
                            task_index, p_job_desc->job_index, p_job_desc->job_id,p_queue->id);
                    }
                    else
                        OS_PRINTF("smh seek_process  assign task_index %d, job_index %d,job_id %d,q null\n",
                            task_index, p_job_desc->job_index, p_job_desc->job_id);
                }

            }
        }
    }
}

static void seek(SmartHTTPGlobalContext *handle, int off)
{
    int job_index;
    int job_id;
    int off_in_job;
    int i;
    int curr_valid_job_index = 0;
    SmartHTTPCurrentTaskContext *p_schedule = NULL;
    DL_TS_SEG_JOB *p_job_desc = NULL;
    JOB_IDS_ARRAY job_id_array;
    SEEK_POLICY_T policy;

    //OS_PRINTF("smart_http.c [%s] start, off %d\n",__func__,(int)off);
    //look for job index
    job_index = off/MULTI_QUEUE_BUF_LEN;
    off_in_job = off - job_index*MULTI_QUEUE_BUF_LEN;
    OS_PRINTF("smh [%s] job_index %d, off_in_job %d\n",__func__,job_index,off_in_job);
    p_job_desc = Get_Job_From_Index(handle->p_jobs_desc,job_index);
    job_id = p_job_desc->job_id;
OS_PRINTF("smh [%s] job_index = %d, job id %d\n",__func__,job_index, job_id);

    //wait mutiqueue seek pending status
    while(!mutiQueue_is_seek_pending(handle->gp_multiqueue))
         mtos_task_sleep(DELAY_TIME);

    job_id_array.totalNum = 0;
    mutiQueue_get_queue_exist_jobs(handle->gp_multiqueue, &job_id_array);

    if(gen_seek_policy(job_id, &job_id_array, &policy))
    {
        excute_seek_policy(handle, job_id, job_index, &policy);
    }

    mutiQueue_end_seek(handle->gp_multiqueue);
    OS_PRINTF("smh [%s] done, job total %d, complete num %d, assgined num %d\n",__func__,
        handle->p_jobs_desc->job_total,handle->p_jobs_desc->complete_job_num,
        handle->p_jobs_desc->assigned_job_num);
}
static void  download_job_queue_schedule(void * param)
{
    int task_index;
    DATA_SEGMENT_QUEUE *p_queue = NULL;
    DL_TS_SEG_JOB *p_job_desc = NULL;
    SmartHTTPGlobalContext *handle = (SmartHTTPGlobalContext*)param;
    MT_BOOL ret;
    mt_set_pthread_name(__FUNCTION__);
    while(1)
    {
        if(handle->schedule_task_status == SCHEDULE_PREPARE_EXIT)
        {
            ERR_PRINTF("[%s] SCHEDULE_PREPARE_EXIT !!!\n",__func__);
            break;
        }

        //check status
        if(handle->schedule_task_status == SCHEDULE_PREPARE_IDLE)
            handle->schedule_task_status = SCHEDULE_IDLE;

        if(handle->schedule_task_status == SCHEDULE_IDLE)
        {
            mtos_task_sleep(DELAY_TIME);

            ////////////set null, because next time will get next job handle
            p_job_desc = NULL;
            p_queue = NULL;

            continue;
        }

        //seek process
        if(handle->schedule_task_status == SCHEDULE_SEEK)
        {
            if(p_job_desc)//process current job
            {
                Assign_Job_Reset(handle->p_jobs_desc, p_job_desc->job_index);
                p_job_desc = NULL;
            }

            seek(handle, handle->seek_off);
            handle->schedule_task_status = SCHEDULE_RUNNING;
        }

        if(handle->schedule_task_status != SCHEDULE_RUNNING)
        {
            mtos_task_sleep(DELAY_TIME);
            continue;
        }

        ///////////////////////////////////look for task/////////////////////////////////////////
        ret = Find_Idle_Download_Task(handle->gp_download_group, &task_index);
        if(!ret)
        {
            mtos_task_sleep(DELAY_TIME);
            continue;
        }
        OS_PRINTF("get task index %d\n",task_index);

        //////////////////////////look for job//////////////////////////////////////////////
        if(!p_job_desc)
            p_job_desc = Get_Next_Job(handle->p_jobs_desc);
        if(!p_job_desc)
        {
            mtos_task_sleep(DELAY_TIME);
            continue;
        }
        OS_PRINTF("schedule_task get job id %d, index %d,offset %d,seg size %d\n",
            p_job_desc->job_id,p_job_desc->job_index,p_job_desc->offset,p_job_desc->seg_size);
        ///////////////////////////////look for queue//////////////////////////////////////////
        if(!p_queue)
        {
            p_queue = mutiQueue_take_one_queue(handle->gp_multiqueue);
        }

        if(!p_queue)
        {
            mtos_task_sleep(DELAY_TIME);
            continue;
        }

        OS_PRINTF("schedule_task get queue id %d, job id %d, len %d\n",
            p_queue->id,p_queue->job_id,p_queue->len);
        ///////////////////////////////////assign /////////////////////////////
        if(assign_job_and_download(handle, p_queue, task_index, p_job_desc) == FALSE)
        {
            mtos_task_sleep(DELAY_TIME);
            continue;
        }

        OS_PRINTF("smh schedule_task  assign task_index %d,job_id %d, q id %d\n",task_index, p_job_desc->job_id, p_queue->id);
        ////////////////////////////////////release ////////////////////////////////

        ////////////set null, because next time will get next job handle
        p_job_desc = NULL;
        p_queue = NULL;

        //mtos_task_sleep(DELAY_TIME);
    }

    handle->schedule_task_status = SCHEDULE_EXIT_DONE;
    mtos_task_exit();
    return;
}

SmartHTTPGlobalContext* Smart_Http_Download_Init(int prio[5])
{
    MT_BOOL ret = FALSE;
    int i;
    ERR_PRINTF("smh [%s] start\n",__func__);

    SmartHTTPGlobalContext *smart_http_download_handle = (SmartHTTPGlobalContext*)malloc(sizeof(SmartHTTPGlobalContext));
    if(!smart_http_download_handle)
        return NULL;

    memset(smart_http_download_handle, 0, sizeof(SmartHTTPGlobalContext));
    smart_http_download_handle->speed_calc = FALSE;

    Smart_Http_Download_Enable_Speed_Calculate(smart_http_download_handle, TRUE);//test

    //save prio
    for(i=0;i<5;i++)
        smart_http_download_handle->g_prio[i] = prio[i];

    //save memory config
    smart_http_download_handle->mem_block_num = MAX_QUEUE_NUM;
    smart_http_download_handle->mem_block_size = MULTI_QUEUE_BUF_LEN;

    //check download group
    if(smart_http_download_handle->gp_download_group)//reset
    {
        Reset_Download_Group(smart_http_download_handle->gp_download_group);
        ret = TRUE;
    }
    else//create
    {
        smart_http_download_handle->gp_download_group = Init_Download_Group(get_write_buf, write_buf_update,
            download_done, write_buf_full,MAX_DL_TASK_NUM, &(smart_http_download_handle->g_prio[1]), SEGMENT_DOWNLOAD_STACK_SIZE);
        if(!smart_http_download_handle->gp_download_group)
        {
            free(smart_http_download_handle);
            return NULL;
        }
        ret = TRUE;
    }

    if(ret)
    {
        //check multi queue
        if(smart_http_download_handle->gp_multiqueue)//reset
        {
            mutiQueue_reset(smart_http_download_handle->gp_multiqueue);
            ret = TRUE;
        }
        else//create
        {
            ret = mutiQueue_init(&(smart_http_download_handle->gp_multiqueue), smart_http_download_handle->mem_block_size,
                smart_http_download_handle->mem_block_num, NORMAL_BUF_M);
        }
    }

    if(ret)
    {
        //malloc stack for schedule thread
        smart_http_download_handle->p_schedule_task_stack = (char *)malloc(SEGMENT_DOWNLOAD_STACK_SIZE);
        if(!smart_http_download_handle->p_schedule_task_stack)
        {
            ret = FALSE;
        }

        ///create schedule task
        ret = mtos_task_create((u8 *)"download schedule", download_job_queue_schedule,smart_http_download_handle,
                    smart_http_download_handle->g_prio[0], (u32*)smart_http_download_handle->p_schedule_task_stack, SEGMENT_DOWNLOAD_STACK_SIZE);
        if(!ret)
        {
            ERR_PRINTF("smh [%s] create download job schedule task failed, prio %d\n",__func__,
                smart_http_download_handle->g_prio[0]);
        }
        else
            OS_PRINTF("smh [%s] create download job schedule task success, prio %d\n",__func__,
                smart_http_download_handle->g_prio[0]);
    }

    //false
    if(!ret)
    {
        Smart_Http_Download_Deinit(smart_http_download_handle);
        return NULL;
    }
    else
    {
        smart_http_download_handle->init_flag = TRUE;
    }

    return smart_http_download_handle;
}
MT_BOOL Smart_Http_Download_Is_Enable(SmartHTTPGlobalContext *handle)
{
    if(!handle)
        return FALSE;

    if(handle->gp_download_group && handle->gp_multiqueue)
        return TRUE;
    else
        return FALSE;
}

void Smart_Http_Download_Enable_Speed_Calculate(SmartHTTPGlobalContext *handle, MT_BOOL enable)
{
    handle->speed_calc = enable;
    smart_http_download_init_speed(handle);
    return;
}
MT_BOOL Smart_Http_Download_Is_Speed_Calculate_Enable(SmartHTTPGlobalContext *handle)
{
    return handle->speed_calc;
}

static void calculate_speed(SmartHTTPGlobalContext *handle, int job_id, u32 *speed)
{
    int i;
    int j_index;
    int begin_time,end_time;//ms
    u32 total_len = 0;
    int count = 0;
    DL_TS_SEG_JOB *p_job_desc = NULL;

    if(!handle)
        return ;

    /*calulate single job speed, becasue sometimes reading performance will affect download performance. so we will check single job speed*/
    int speed_current_job = 0;

    /*calulate average speed of 4 valid taks*/
    begin_time = 0;
    end_time = 0;
    for(i=0;i<MAX_DL_TASK_NUM*2;i++)
    {
        p_job_desc = Get_Job_From_Id(handle->p_jobs_desc, job_id-i);
        if(!p_job_desc)
            continue;
        j_index = p_job_desc->job_index;
        if(Is_Job_Complete(handle->p_jobs_desc, j_index) && Is_Job_Success(handle->p_jobs_desc, j_index))
        {
            OS_PRINTF("j %d,size %d, begin %d, end %d\n",job_id-i,p_job_desc->seg_size, p_job_desc->begin_time,p_job_desc->end_time);
            count++;
            total_len += p_job_desc->seg_size;
            if(begin_time == 0 || begin_time>p_job_desc->begin_time)
                begin_time = p_job_desc->begin_time;

            if(end_time == 0 || end_time<p_job_desc->end_time)
                end_time = p_job_desc->end_time;

            if(speed_current_job == 0)
            {
                int duration = (p_job_desc->end_time-p_job_desc->begin_time);
                if(duration)
                    speed_current_job = p_job_desc->seg_size*1000/duration;
            }
        }
        if(count == MAX_DL_TASK_NUM)
            break;
    }

    if(end_time-begin_time)
    {
        *speed = total_len*1000/(end_time-begin_time);
    }

    if(speed_current_job > *speed)
        *speed = speed_current_job;
    return;
}
char* Smart_Http_Download_Get_Redirect_Url(SmartHTTPGlobalContext *handle)
{
    if(!handle->p_jobs_desc)
        return NULL;

    //if redirect , use redirect url
   /* if(handle->p_jobs_desc->redirect_url)
        return handle->p_jobs_desc->redirect_url;*/

    return NULL;
}
extern u32 jobs_get_time_ms();
static void smart_http_download_init_speed(SmartHTTPGlobalContext *handle)
{
    handle->count_start_time = jobs_get_time_ms();
    handle->max_speed = 0;
    handle->real_speed = 0;
}

static MT_BOOL smart_http_download_gen_speed(SmartHTTPGlobalContext *handle, int job_index, u32 begin_time, int interval_len)
{
#define DURATION_TO_SAVE_SPEED_MS          (30*1000)
    u32 speed = 0, max_speed = 0;
    int i;
    u32 current_time = jobs_get_time_ms();

    if(!handle->speed_calc || !handle->p_jobs_desc)
        return FALSE;

    if(current_time - handle->count_start_time >= DURATION_TO_SAVE_SPEED_MS)
    {
        handle->count_start_time = current_time;
        handle->max_speed = 0;
        handle->real_speed = 0;
    }

    //single mode to calculate speed
    if(Get_Jobs_Single_Mode(handle->p_jobs_desc))
    {
        u32 end_time = jobs_get_time_ms();
        if(handle->max_speed == 0)
        {
            handle->max_speed = 1;//not zero ,to use next time ,but not valid speed.
            return FALSE;
        }

        if(end_time - begin_time)
        {
            speed = interval_len*1000/(end_time-begin_time);
        }

        if(handle->max_speed < speed)
            handle->max_speed = speed;
        handle->real_speed = speed;

        OS_PRINTF("[%s]---single j %d ,max speed %d, real speed %d\n",__func__,job_index,handle->max_speed,handle->real_speed);
        return TRUE;
    }

    max_speed = 0;
    ///current job
    if(Is_Job_Success(handle->p_jobs_desc, job_index))
    {
        DL_TS_SEG_JOB *p_job_desc = NULL;
        p_job_desc = Get_Job_From_Index(handle->p_jobs_desc, job_index);

        if(p_job_desc->end_time && p_job_desc->begin_time && (p_job_desc->end_time-p_job_desc->begin_time))
        {
            max_speed = p_job_desc->seg_size/(p_job_desc->end_time-p_job_desc->begin_time);
            //mtos_printk("current job spped %d\n",max_speed);
        }
    }


    max_speed = 0;
    calculate_speed(handle, job_index, &max_speed);

    handle->real_speed = max_speed;

    if(handle->real_speed > handle->max_speed)
    {
        handle->max_speed = handle->real_speed;
    }


    OS_PRINTF("[%s]---j %d ,max speed %d, real speed %d\n",__func__,job_index,handle->max_speed,handle->real_speed);
    return TRUE;
}
MT_BOOL Smart_Http_Download_Get_Speed(SmartHTTPGlobalContext *handle, u32 *speed_max_Bps, u32 * speed_real_Bps)
{
    if(!handle->speed_calc || !speed_max_Bps || !speed_real_Bps)
        return FALSE;

    *speed_max_Bps = handle->max_speed;

    *speed_real_Bps = handle->real_speed;

    OS_PRINTF("[%s]---max speed %d, real speed %d\n",__func__,*speed_max_Bps,*speed_real_Bps);
    return TRUE;
}
void Smart_Http_Download_Deinit(SmartHTTPGlobalContext *handle)
{
    ERR_PRINTF("smh [%s] start\n",__func__);
    if(!handle)
        return;

    if(handle->init_flag == FALSE)
        return;

    handle->schedule_task_status = SCHEDULE_PREPARE_EXIT;
    while(handle->schedule_task_status != SCHEDULE_EXIT_DONE)
        mtos_task_sleep(5);

    if(handle->gp_download_group)
    {
        Stop_Download_Group(handle->gp_download_group);

        Destroy_Download_Group(handle->gp_download_group);

        handle->gp_download_group = NULL;
    }

    if(handle->gp_multiqueue)
    {
        mutiQueue_deinit(handle->gp_multiqueue);
        handle->gp_multiqueue = NULL;
    }

    if(handle->p_schedule_task_stack)
    {
        free(handle->p_schedule_task_stack);
        handle->p_schedule_task_stack = NULL;
    }

    handle->max_speed = 0;
    handle->init_flag = FALSE;

    free(handle);
ERR_PRINTF("smh [%s] end\n",__func__);
    return;
}

//extern char *mediaplay_http_extraheader;
//extern char *mediaplay_http_header_useragent;

int Smart_Http_Download_Open(SmartHTTPGlobalContext *handle, const char *uri, char *extra_header, MT_BOOL live_stream,
    MT_BOOL reconnect)
{
    MT_BOOL ret;
    MT_BOOL single_job_mode;
    OS_PRINTF("smh [%s] url:%s\n",__func__,uri);

    if(!uri)
        return -1;

    handle->schedule_task_status = SCHEDULE_IDLE;

    if(!handle->gp_multiqueue || !handle->gp_download_group)
        return -1;

    //gen jobs
    handle->p_jobs_desc = Gen_Jobs(uri,
        extra_header,
        MULTI_QUEUE_BUF_LEN,
        reconnect);
    if(!handle->p_jobs_desc)
    {
        reset_multiqueue_and_download_group(handle);
        return -1;
    }

    //check single job mode or multi jobs mode..
    single_job_mode = Get_Jobs_Single_Mode(handle->p_jobs_desc);
    if(single_job_mode)
    {
        OS_PRINTF("smh [%s] single_job_mode\n",__func__);
        ret = mutiQueue_reconfig(handle->gp_multiqueue, RING_BUF_M);
        if(ret)
        {
            handle->mem_block_num = 1;
            handle->mem_block_size = MULTI_QUEUE_BUF_LEN;
        }
    }
    else
    {
        OS_PRINTF("smh [%s] not single_job_mode\n",__func__);
        ret = mutiQueue_reconfig(handle->gp_multiqueue,NORMAL_BUF_M);
        if(ret)
        {
            handle->mem_block_num = MAX_QUEUE_NUM;
            handle->mem_block_size = MULTI_QUEUE_BUF_LEN;
        }
    }
    if(!ret)
    {
        reset_multiqueue_and_download_group(handle);
        return -1;
    }

    Smart_Http_Download_Enable_Speed_Calculate(handle, TRUE);

    //start download group, set running status
    ret = Start_Download_Group(handle->gp_download_group, handle->p_jobs_desc->header);
    if(!ret)
    {
        reset_multiqueue_and_download_group(handle);

        Destroy_Jobs(handle->p_jobs_desc);
        return -1;
    }

    handle->seekable = single_job_mode?0:1;
    handle->filesize = (handle->p_jobs_desc->content_length > 0)?handle->p_jobs_desc->content_length:-1;
    handle->live_stream = live_stream;
    handle->reconnect = reconnect;
    if(extra_header)
        handle->http_extra_header = strdup(extra_header);
    else
        handle->http_extra_header = NULL;

    handle->schedule_task_status = SCHEDULE_RUNNING;
    return 0;
}

int Smart_Http_Download_Seek(SmartHTTPGlobalContext *handle, int off, SMART_HTTP_DOWNLOAD_SEEK_MODE seek_mode)
{

    OS_PRINTF("[%s] coming 00 off = %d,whence %d\n",__func__,(int)off, seek_mode);
    if(!handle || !handle->seekable)
        return -1;
    OS_PRINTF("[%s] coming 11, s->off %d\n",__func__,(int)handle->off);
    if (seek_mode == SMART_HTTP_DOWNLOAD_SEEK_SIZE)
    {
        OS_PRINTF("smh [%s] AVSEEK_SIZE filesize=%d\n",__func__,handle->filesize);
        return handle->filesize;
    }
    else if ((handle->filesize == -1 && seek_mode == SMART_HTTP_DOWNLOAD_SEEK_END) )
    {
        ERR_PRINTF("smh [%s] SEEK_END filesize=%d  error\n",__func__,handle->filesize);
        return -1;
    }


    if (seek_mode == SMART_HTTP_DOWNLOAD_SEEK_CUR)
        off += handle->off;
    else if (seek_mode == SMART_HTTP_DOWNLOAD_SEEK_END)
        off += handle->filesize;
OS_PRINTF("smh [%s] seek pos=%d, cur pos %d, filesize %d\n",__func__,(int)off,(int)handle->off,handle->filesize);

    if(handle->off == (int)off)
        return (int)off;


    if(handle->filesize && off < handle->filesize)
    {
        handle->off = off;
        handle->seek_off = off;
        handle->schedule_task_status = SCHEDULE_SEEK;
        OS_PRINTF("smh [%s] queue seek start\n",__func__);
        mutiQueue_start_seek(handle->gp_multiqueue, off);
        OS_PRINTF("smh [%s] queue seek end\n",__func__);
    }
    else
    {
        ERR_PRINTF("smh [%s] seek failed filesize=%d  error\n",__func__,handle->filesize);
        return -1;
    }


    //OS_PRINTF("[%s] seek done s->off=%d,  off %d\n",__func__,(int)s->off,(int)off);
    return (int)off;
}
static void reopen_pocess(SmartHTTPGlobalContext *handle)
{
    char *url = NULL;

    if(!handle->p_jobs_desc)
        return;

    if(!handle->p_jobs_desc->url)
        return;

    url = strdup(handle->p_jobs_desc->url);
    Smart_Http_Download_Close(handle);
    if(url)
    {
        while(1)
        {
            //if(is_file_seq_exit())
            //    break;

            if(Smart_Http_Download_Open(handle, url, handle->http_extra_header,
                    handle->live_stream, handle->reconnect) == 0)
                break;
        }
        free(url);
    }

    if(handle->http_extra_header)
    {
        free(handle->http_extra_header);
        handle->http_extra_header = NULL;
    }

    return;
}
int Smart_Http_Download_Read(SmartHTTPGlobalContext *handle, unsigned char *buf, int size)
{
    int ret = 0;
    MT_BOOL download_finished = FALSE;
    //OS_PRINTF("[%s] start,buf size %d\n",__func__,size);
    if(!handle)
    {
        OS_PRINTF("[%s] return1 %d\n",__func__,-1);
        return -1;
    }

redo:
    if(!handle->p_jobs_desc)
    {
        OS_PRINTF("[%s] return2 %d\n",__func__,-1);
        return -1;
    }

    download_finished = Is_Jobs_Finished(handle->p_jobs_desc);
    //OS_PRINTF("[%s] download_finished %d\n",__func__,download_finished);
    if(download_finished)
    {
        DL_TS_SEG_JOB *p_job_desc = Get_Final_Job(handle->p_jobs_desc);
        //OS_PRINTF("[%s] job_id %d,\n",__func__,p_job_desc->job_id);
        DATA_SEGMENT_QUEUE *p_queue = mutiQueue_get_one_queue(handle->gp_multiqueue, p_job_desc->job_id);
        if(!p_queue)
            OS_PRINTF("smh [%s] not queue\n",__func__);
        else
        {
            ret = check_queue_drained(p_queue);
            OS_PRINTF("[%s]queue w %d,r %d, ret %d\n",__func__,p_queue->write_pos,p_queue->read_pos,ret);
            if(ret)
            {
                OS_PRINTF("g_is_live_broadcast %d,reconnect %d\n",handle->live_stream,handle->p_jobs_desc->reconnect);
                //live stream and need reconnect, and single mode in jobs, we need reopen this connection.
                if(handle->live_stream && Get_Jobs_Single_Mode(handle->p_jobs_desc) && handle->p_jobs_desc->reconnect)
                {
                    OS_PRINTF("smh [%s] reopen 00\n",__func__);
                    reopen_pocess(handle);
                    OS_PRINTF("[%s] return1 %d\n",__func__,0);
                    return 0;
                }
                OS_PRINTF("smh [%s] return eof\n",__func__);
                return SMART_HTTP_DOWNLOAD_EOF;
            }
        }
    }

    //OS_PRINTF("[%s] start to read queue, buf addr %x,size %d\n",__func__,buf,size);
    ret = mutiQueue_read(handle->gp_multiqueue, buf, size);
    if(ret == -1)
    {
        OS_PRINTF("smh [%s] 222 return error , no data, g_is_live_broadcast %d, reconnect %d\n",__func__,
            handle->live_stream,handle->p_jobs_desc->reconnect);
        //live stream and need reconnect
        if(handle->live_stream && handle->p_jobs_desc && handle->p_jobs_desc->reconnect)
        {
            //if single mode ,we should reopen ,because download has be done, sometimes server close the connection.
            if(Get_Jobs_Single_Mode(handle->p_jobs_desc))
            {
                OS_PRINTF("smh [%s] reopen 11\n",__func__);
                reopen_pocess(handle);
            }
            else
            {
                OS_PRINTF("smh [%s] reread\n",__func__);
                mtos_task_sleep(DELAY_TIME);
                goto redo;
            }
        }
        OS_PRINTF("[%s] return2 %d\n",__func__,0);
        return 0;
    }


    if(ret > 0)
        handle->off += ret;

    return ret;
}

int Smart_Http_Download_Close(SmartHTTPGlobalContext *handle)
{
    ERR_PRINTF("smh [%s] start\n",__func__);
    if(!handle)
        return -1;

    Smart_Http_Download_Enable_Speed_Calculate(handle, FALSE);

    handle->schedule_task_status = SCHEDULE_PREPARE_IDLE;
    while(handle->schedule_task_status != SCHEDULE_IDLE)
        mtos_task_sleep(5);

    if(handle->gp_download_group)
    {
        Reset_Download_Group(handle->gp_download_group);
    }

    if(handle->p_jobs_desc)
    {
        Destroy_Jobs(handle->p_jobs_desc);
        handle->p_jobs_desc = NULL;
    }

    if(handle->gp_multiqueue)
        mutiQueue_reset(handle->gp_multiqueue);

    handle->max_speed = 0;
    handle->real_speed = 0;
    ERR_PRINTF("smh [%s] done\n",__func__);
    return 0;
}


void TestSmartHttp()
{
  int prio[5];
  ne_sock_init();
  SmartHTTPGlobalContext* pCtx = Smart_Http_Download_Init(prio);

  Smart_Http_Download_Open(pCtx, "http://120.78.53.135/iptv/test.php", NULL, FALSE,TRUE);
  char buffer[4096];
  FILE *fp = fopen("eeeeee.ts","wb");
  int ret = 0;
  int len = 0;
  while (1)
  {
    ret = Smart_Http_Download_Read(pCtx, buffer,  4096);
    if (ret <= 0) break;
    fwrite(buffer, ret,1, fp);
    len += ret;
  }
  printf("recv len: %d\n", len);
  Smart_Http_Download_Close(pCtx);
  fclose(fp);
  ne_sock_exit();
  Smart_Http_Download_Deinit(pCtx);
}



