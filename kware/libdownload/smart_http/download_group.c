/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
//#include "sys_define.h"
#include "mt_common.h"
//#include "osal_mtos.h"
#include <pthread.h>

#include "download_group.h"
#include "download_api.h"

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

extern int g_is_live_broadcast;
//#define RECV_LEFT_DATA_WHEN_STOP
typedef struct DOWNLOAD_CONTEXT_T DOWNLOAD_CONTEXT;
typedef struct DOWNLOAD_GROUP_CONTEXT_T DOWNLOAD_GROUP_CONTEXT;

struct DOWNLOAD_CONTEXT_T{

  int task_prio;/*task0: 23  task1: 34  task2: 67*/
  int task_index;
  DOWNLOAD_TASK_STATUS task_status;/*task0:idle task1:idle  task2:busy*/
  DOWNLOAD_TASK_STATUS expect_status;
  DOWNLOAD_MODE download_mode;
  unsigned long task_param;//task param address
  int download_offset;
  int request_size;
  int content_len;
  char * p_stack;//only for release memory
  char *p_url;
  void *p_download_handle;
  void *queue;
  struct DOWNLOAD_GROUP_CONTEXT_T *dl_group;
};

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

struct DOWNLOAD_GROUP_CONTEXT_T{

 /*for schduler*/
  //long  begin_time;//time for first download task run
  //long  current_time;
  //long  expired_time;

  int  real_task_num;
#ifdef RECV_LEFT_DATA_WHEN_STOP
  char * tmp_buf;/*only recv rong data from lwip*/
#endif
  /* for all download thread*/
  DOWNLOAD_TASK_STATUS expect_status;

  /* for download group, create in Start_Download_Group, and destroy in Destroy_Download_Group*/
  struct DOWNLOAD_CONTEXT_T * p_dl_ctx[MAX_DL_TASK_NUM];

  /*get buf from uplayer, and set the address of buf point, and buf length*/
  MT_BOOL (*get_write_buf)(unsigned int cbParam, void *queue, int task_index, WRITE_POS_INFO * p_info);

  /*  update write buf for uplayer, tell uplayer we have written write_len data in this buffer*/
  void (*write_buf_update)(ulong cbParam, void *queue, int task_index, int len);

  /*  tell uplayer current task have downloaded the task already!*/
  void (*download_done)(ulong cbParam1, unsigned int cbParam2, void *queue, int task_index, MT_BOOL success);

  /*  tell uplayer current task the write buf has been full*/
  void (*write_buf_full)(ulong cbParam, void *queue, u32 begin_time, int buf_used_len);

  char *http_header;
};

static char *add_http_header_for_range(char *header, int offset, int seg_size)
{
    char range[64];
    char *p_new_header = NULL;
    int len = 0;
    int range_end = offset+seg_size-1;//must minus 1

    memset(range, 0, 64);

    if(range_end <=0)
        sprintf(range,"Range: bytes=%d-\r\n",offset);
    else
        sprintf(range,"Range: bytes=%d-%d\r\n",offset,range_end);

    if(header)
        len += strlen(header);
    len += strlen(range);

    p_new_header = (char*)malloc(len+4);
    if(!p_new_header)
        return NULL;
    memset(p_new_header,0,len+4);

    if(header)
        sprintf(p_new_header, "%s%s",header,range);
    else
        strcpy(p_new_header, range);
    return p_new_header;
}
/*****************************************************************************
*
*
*
*   3,   internal methods  for handling fetch ts data task
*
*
*
*********************************************************************************/
static MT_BOOL download_file(DOWNLOAD_GROUP_CONTEXT * p_dl_group_ctx,
    DOWNLOAD_CONTEXT * p_dl_ctx,
    int task_index,
    unsigned long *p_working_queue)
{
    HttpDownloadHeader downRespHeader;
    HttpDownloadResult result;
    void *pDownHandle = NULL;
    WRITE_POS_INFO info;
    int i;
    int total_read_len = 0;//download read length
    int buf_len;//write buf size
    int buf_len2;//write buf size
    int write_pos;//write pos in write buffer
    int request_size = p_dl_ctx->request_size;//like content length, for first job connection
    MT_BOOL buf_full = TRUE;//write buf full
    MT_BOOL ret;
    int recvCnt=0;
    u32 begin_time;
    char *p_header = NULL;

//OS_PRINTF("[%s]  task index %d , coming\n",__func__,task_index);
    begin_time = jobs_get_time_ms();
redo:
    //download start
    downRespHeader.Content_length = 0;
    downRespHeader.redirect_url = NULL;

    //mtos_printk("[download_group][%s]  off %d, len %d\n",__func__,p_dl_ctx->download_offset+total_read_len,request_size-total_read_len);
    p_header = add_http_header_for_range(p_dl_group_ctx->http_header, p_dl_ctx->download_offset+total_read_len, request_size-total_read_len);
    //if(p_header)
    //    mtos_printk("[download_group][%s]  p_header %s\n",__func__,p_header);
    //else
    //    mtos_printk("[download_group][%s]  p_header NULL\n",__func__);
    //mtos_printk("download task %d start\n",task_index);
    if(p_dl_ctx->p_download_handle)
    {
        OS_PRINTF("dgp [%s]  task index %d , use download handle, request_size %d\n",__func__,task_index,request_size);
        pDownHandle = p_dl_ctx->p_download_handle;
        #ifndef __LINUX__
        Nw_Http_Download_Add_Task_Prio(pDownHandle, p_dl_ctx->task_prio);
        #endif
    }
    else
    {
        if(!p_dl_ctx->p_url)
        {
            if(p_header)
            {
                free(p_header);
                p_header = NULL;
            }
            ERR_PRINTF("dgp [%s][WARNNIG]task index %d  url is null, return\n",__func__,task_index);
            return FALSE;
        }

        for(i=0; i<3;i++)
        {
            //OS_PRINTF("[%s]  task index %d  start 0000\n",__func__,task_index);
            pDownHandle = Nw_Http_Download_Start(p_dl_ctx->p_url, FALSE, NULL, 0, DOWNLOAD_GROUP_TIMEOUT, &downRespHeader, p_header);
            //OS_PRINTF("[%s]  task index %d  start done\n",__func__,task_index);
            if(pDownHandle)
            {
            	//ERR_PRINTF("[%s] %d fail to download_start!\n",__func__,__LINE__);
                break;
            }

            if(p_dl_group_ctx->expect_status != RUNNING || p_dl_ctx->expect_status != RUNNING)
            {
                if(pDownHandle)
                {
                    Nw_Http_Download_Stop(pDownHandle);
                    ERR_PRINTF("dgp stop00 download task %d\n",task_index);
                }

                p_dl_ctx->p_download_handle = NULL;
                if(p_header)
                {
                    free(p_header);
                    p_header = NULL;
                }
				//ERR_PRINTF("[%s] %d fail to download_start2222!\n",__func__,__LINE__);
                return FALSE;
            }
        }

        if(!pDownHandle)
        {
            ERR_PRINTF("dgp [%s][ERROR] resp %d\n",__func__,downRespHeader.resp_code);
            if(p_header)
            {
                free(p_header);
                p_header = NULL;
            }

            if(p_dl_ctx->download_mode == DOWNLOAD_MODE_ERROR_REDO)
            {
            	ERR_PRINTF("dgp [%s] redo download!!!!\n",__func__);
                goto redo;
            }

            return FALSE;
        }

        //OS_PRINTF("[download_group][%s]  task index %d, content_length %d, accept_range %d, http resp code %d\n",__func__,task_index,
         //   downRespHeader.Content_length, downRespHeader.accept_range, downRespHeader.resp_code);
    }

    if(p_header)
    {
        free(p_header);
        p_header = NULL;
    }

    //downlaod recv
    do
    {
        if(p_dl_group_ctx->expect_status != RUNNING || p_dl_ctx->expect_status != RUNNING)
        {

            	int cnt2 =0;
            if(pDownHandle)
            {
                #ifdef RECV_LEFT_DATA_WHEN_STOP
                do
                {//recv unwanted data from lwip
                    result = Nw_Http_Download_Recv(pDownHandle, (char*)(p_dl_group_ctx->tmp_buf), 128*1024);
                    cnt2 = cnt2 + result.readLen;

                    if(result.status == -2 || result.readLen == 0)
                    {
                        ERR_PRINTF("dgp [%s][%d] finish recv unwanted data,readLen:%d !!\n",__func__,__LINE__,result.readLen);
                        break;
                    }
                }while(result.status != -1);
                ERR_PRINTF("dgp [%s][%d] status:%d,cnt2:%d\n",__func__,__LINE__,result.status,cnt2);
                #endif

                Nw_Http_Download_Stop(pDownHandle);

                ERR_PRINTF("dgp [%s][%d]force to stop task[%d] !!!!!!\n",__func__,__LINE__,task_index);

                if(p_dl_ctx->p_download_handle)
                {
                    ERR_PRINTF("dgp stop first connection 02\n");
                }
            }

            //ERR_PRINTF("[%s] %d fail to expect_status != RUNNING cnt:%d\n",__func__,__LINE__,cnt);
            if(p_dl_ctx->p_download_handle)
            {
                p_dl_ctx->p_download_handle = NULL;
            }
            return FALSE;
        }

        if(buf_full)
        {
            OS_PRINTF("dgp [%s]  task_index %d, get buf!\n",__func__,task_index);
            ret = p_dl_group_ctx->get_write_buf(p_dl_ctx->task_param, p_dl_ctx->queue, task_index, &info);
            if(ret)
            {
                *p_working_queue = (unsigned long)(info.p_working_queue);

                buf_len = info.max_w_byte;
                buf_len2 = buf_len;
                write_pos = 0;
                buf_full = FALSE;
                if(!begin_time)
                    begin_time = jobs_get_time_ms();

                if(request_size && buf_len > (request_size-total_read_len))
                    buf_len = (request_size-total_read_len);
            }
            else// can not get buf
            {
                mtos_task_sleep(10);
				OS_PRINTF("dgp [%s][WARNING]=can not get buf!!!!!!\n",__func__);
                continue;
            }
        }
        result = Nw_Http_Download_Recv(pDownHandle, (char*)(info.w_start_addr+write_pos), buf_len);
        if(result.status == 0 || result.status == -1)//ok or done
        {
            buf_len -= result.readLen;
            total_read_len += result.readLen;
            //OS_PRINTF("[download_group][%s] task_index %d recv %d!,total recv %d,buf len %d, request_size %d, status %d\n",__func__,
            //    task_index,result.readLen,total_read_len,buf_len,request_size, result.status);
            p_dl_group_ctx->write_buf_update((ulong)(info.p_working_queue), p_dl_ctx->queue, task_index, result.readLen);
            write_pos += result.readLen;
            recvCnt++;

            if(buf_len == 0)//because sometimes will do recv funcion to recv 0 data and done status
            {
                buf_full = TRUE;
                p_dl_group_ctx->write_buf_full(p_dl_ctx->task_param, p_dl_ctx->queue, begin_time, buf_len2-buf_len);
                begin_time = 0;
            }

            if(result.status == -1 || (request_size && total_read_len >= request_size))//done
            {
                //mtos_printk("[download_group][%s] task_index %d recv -----------------------done,total read %d\n",__func__,task_index,total_read_len);
                Nw_Http_Download_Stop(pDownHandle);
                //ERR_PRINTF("dgp [%s][OK]task:%d,j:%d,status:%d\n",
                //    __func__,task_index,((DATA_SEGMENT_QUEUE*)(p_dl_ctx->queue))->job_id,result.status);
                if(p_dl_ctx->p_download_handle)
                {
                    ERR_PRINTF("dgp stop first connection 03\n");
                    p_dl_ctx->p_download_handle = NULL;
                }
                if(p_dl_ctx->download_offset == 0 && p_dl_ctx->request_size == 0 && p_dl_ctx->content_len <= 0 && g_is_live_broadcast==1){//ts live broadcast
                    ERR_PRINTF("dgp live mode reconnect hahaha!!!\n");
                    pDownHandle = NULL;
                    mtos_task_sleep(10);
                    goto redo;
                }
                return TRUE;
            }

        }
        else if(result.status == -2)//error
        {
            ERR_PRINTF("dgp [%s][WARNING]task_index:%d,job:%d,status:-2!\n",__func__,task_index,
				       ((DATA_SEGMENT_QUEUE*)(p_dl_ctx->queue))->job_id);
            p_dl_group_ctx->write_buf_full(p_dl_ctx->task_param, p_dl_ctx->queue, begin_time, buf_len2-buf_len);
            if(p_dl_ctx->download_mode != DOWNLOAD_MODE_ERROR_REDO)
                break;
            else
            {
                Nw_Http_Download_Stop(pDownHandle);
                pDownHandle = NULL;
                if(p_dl_ctx->p_download_handle)
                {
                    ERR_PRINTF("dgp stop first connection 04\n");
                    p_dl_ctx->p_download_handle = NULL;
                }
                ERR_PRINTF("dgp [%s] stop download task[%d]\n",__func__,task_index);
                goto redo;
            }
        }
        //OS_PRINTF("[%s]  task_index %d, recv status %d, task status %d, group status %d\n",__func__,task_index,
        //    result.status, p_dl_ctx->task_status,p_dl_group_ctx->expect_status);
        //if(recvCnt % 10 == 0)
        //{
            //mtos_task_sleep(5);
            //recvCnt = 0;
        //}
    }while(1);

    Nw_Http_Download_Stop(pDownHandle);
    if(p_dl_ctx->p_download_handle)
    {
        ERR_PRINTF("dgp stop first connection 05\n");
        p_dl_ctx->p_download_handle = NULL;
    }
    return FALSE;
}
static int get_index_from_prio(DOWNLOAD_GROUP_CONTEXT* p_dl_group_ctx, int prio)
{
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int index;

    for(index=0;index<p_dl_group_ctx->real_task_num;index++)
    {
        p_dl_ctx = p_dl_group_ctx->p_dl_ctx[index];
        //mtos_printk("index %d, task_prio %d,prio %d\n",index,p_dl_ctx->task_prio,prio);
        if(p_dl_ctx->task_prio == prio)
            break;
    }
    //mtos_printk("---index %d, real_task_num %d\n",index,p_dl_group_ctx->real_task_num);
    if(index >= p_dl_group_ctx->real_task_num)
        return -1;

    return index;
}
static void  download_task_entry(void * param)
{
    //u32 mtos_task_get_info(mtos_task_info_t *p_info)
    DOWNLOAD_CONTEXT * p_dl_ctx = (DOWNLOAD_CONTEXT*)param;
    DOWNLOAD_GROUP_CONTEXT * p_dl_group_ctx = NULL;
    int task_index = 0;
    unsigned long p_working_queue = 0;

    MT_BOOL ret = FALSE;
    mt_set_pthread_name(__FUNCTION__);
    if(!p_dl_ctx)
    {
        ERR_PRINTF("dgp [%s] %d error found ,DOWNLOAD_CONTEXT is null\n",__func__,__LINE__);
        mtos_task_exit();
        return;
    }

    p_dl_group_ctx = p_dl_ctx->dl_group;
    if(!p_dl_group_ctx)
    {
        ERR_PRINTF("dgp [%s] %d error found ,DOWNLOAD_GROUP_CONTEXT is null\n",__func__,__LINE__);
        mtos_task_exit();
        return;
    }

    //wait run_flag
    while(p_dl_group_ctx->expect_status == IDLE)
        mtos_task_sleep(10);

    task_index = p_dl_ctx->task_index;
    OS_PRINTF("dgp [%s]  task index %d , start to loop function\n",__func__,task_index);

    do
    {
        //OS_PRINTF("[download_group][%s]  task index %d , group expect status %d, cur status %d\n",__func__,p_dl_group_ctx->expect_status,
        //    p_dl_ctx->task_status);
        if(p_dl_group_ctx->expect_status == EXIT_STATUS)
        {
            OS_PRINTF("dgp [%s]  task index %d , group expect exit, so break!\n",__func__,task_index);
            if(p_dl_ctx->p_download_handle)
            {
                ERR_PRINTF("dgp stop first connection 00\n");
                Nw_Http_Download_Stop(p_dl_ctx->p_download_handle);
                p_dl_ctx->p_download_handle = NULL;
            }
            break;
        }

        if(p_dl_group_ctx->expect_status == IDLE || p_dl_ctx->expect_status == IDLE)
        {
            if(p_dl_ctx->task_status ==  RUNNING)//has already get a task.
            {
                ret = FALSE;
                p_working_queue = 0;
                goto do_nothing;
            }
            p_dl_ctx->task_status = IDLE;
        }

        if(p_dl_group_ctx->expect_status == RUNNING && p_dl_ctx->task_status == RUNNING)
        {
            //OS_PRINTF("[%s]  task index %d 0000\n",__func__,task_index);
            p_working_queue = 0;
            ret = download_file(p_dl_group_ctx, p_dl_ctx, task_index, &p_working_queue);

do_nothing:
            if(p_dl_ctx->p_url)
            {
                free(p_dl_ctx->p_url);
                p_dl_ctx->p_url = NULL;
            }

            if(p_dl_ctx->p_download_handle)
            {
                ERR_PRINTF("dgp stop first connection 01\n");
                Nw_Http_Download_Stop(p_dl_ctx->p_download_handle);
                p_dl_ctx->p_download_handle = NULL;
            }

            OS_PRINTF("dgp [%s] download done\n",__func__);
            p_dl_group_ctx->download_done(p_dl_ctx->task_param, p_working_queue, p_dl_ctx->queue, task_index, ret);
            p_dl_ctx->queue = NULL;
            p_dl_ctx->task_status = IDLE;
            //mtos_printk("task %d idle\n",task_index);
            //OS_PRINTF("[%s]  task index %d 1111\n",__func__,task_index);
        }
        else
        {
            mtos_task_sleep(5);
        }
    }while(1);


    p_dl_ctx->task_status = EXIT_STATUS;
    OS_PRINTF("dgp [%s] %d task index %d quit!!!!\n",__func__,__LINE__,task_index);
    mtos_task_exit();
}


MT_BOOL Find_Idle_Download_Task(void * pHandle,int * p_task_index)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int i = 0;
    if(!p_dl_group_ctx)
    {
        ERR_PRINTF("dgp [%s] %d error found ,DOWNLOAD_GROUP_CONTEXT is null\n",__func__,__LINE__);
        return FALSE;
    }

    if(p_dl_group_ctx->real_task_num <= 0)
    {
        ERR_PRINTF("dgp [%s] %d error found ,real_task_num is null\n",__func__,__LINE__);
        return FALSE;
    }

    for(i=0;i<p_dl_group_ctx->real_task_num;i++)
    {
        p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
        if(p_dl_ctx->task_status == IDLE)
        {
            *p_task_index = i;
            break;
        }
    }

    if(i >= p_dl_group_ctx->real_task_num)
    {
        //OS_PRINTF("[%s] %d not found idle task\n",__func__,__LINE__);
        return FALSE;
    }
    else
        return TRUE;
}
void static free_download_group(DOWNLOAD_GROUP_CONTEXT * p_dl_group_ctx)
{
    int i;
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;

    for(i=0; i<p_dl_group_ctx->real_task_num; i++)
    {
        p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
        if(p_dl_ctx)
        {
            if(p_dl_ctx->p_stack)
                free(p_dl_ctx->p_stack);

            if(p_dl_ctx->p_url)
                free(p_dl_ctx->p_url);

            free(p_dl_ctx);
        }
    }

#ifdef RECV_LEFT_DATA_WHEN_STOP
    if(p_dl_group_ctx->tmp_buf)
    {
        free(p_dl_group_ctx->tmp_buf);
        p_dl_group_ctx->tmp_buf = NULL;
    }
#endif

    free(p_dl_group_ctx);

    return;
}

void *Get_Queue_From_Download_Group(void  *pHandle, int task_index)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;

    if(task_index < 0 || task_index>MAX_DL_TASK_NUM)
        return NULL;

    if(!p_dl_group_ctx)
        return NULL;

    if(!p_dl_group_ctx->p_dl_ctx[task_index])
        return NULL;

    return p_dl_group_ctx->p_dl_ctx[task_index]->queue;
}
/**
*
*       init all download task
**
*
*/
void * Init_Download_Group(
	                                  MT_BOOL (*get_write_buf)(unsigned int cbParam, void *queue, int task_index, WRITE_POS_INFO * p_info),
	                                  void (*write_buf_update)(ulong cbParam, void *queue, int task_index, int len),
	                                  void (*download_done)(ulong cbParam1, unsigned int cbParam2, void *queue, int task_index, MT_BOOL success),
	                                  void (*write_buf_full)(ulong cbParam, void *queue, u32 begin_time, int buf_used_len),
	                                  int task_num,
	                                  int * p_task_priorty,
	                                  int stack_size)
{

    DOWNLOAD_GROUP_CONTEXT * p_dl_group_ctx =
        (DOWNLOAD_GROUP_CONTEXT *)malloc(sizeof(DOWNLOAD_GROUP_CONTEXT));
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int i=0;

    if(!p_dl_group_ctx)
    {
        ERR_PRINTF("dgp [%s] %d malloc group context error!\n",__func__,__LINE__);
        return NULL;
    }
    memset(p_dl_group_ctx,0,sizeof(DOWNLOAD_GROUP_CONTEXT));

    ERR_PRINTF("dgp [%s] %d task_num:%d!\n",__func__,__LINE__,task_num);

#ifdef RECV_LEFT_DATA_WHEN_STOP
    if(p_dl_group_ctx->tmp_buf == NULL)
        p_dl_group_ctx->tmp_buf = (char *)malloc(TMP_BUF_LEN);
#endif

    p_dl_group_ctx->expect_status = IDLE;

    if(task_num > 0 && p_task_priorty)
    {
        for(i=0;i<task_num;i++)
        {
            char taskname[128];
            memset(taskname,0,128);
            char * p_stack = (char *)malloc(stack_size);
            if(p_stack)
            {
                memset(p_stack,0,stack_size);
            }
            else
            {
                ERR_PRINTF("dgp [%s] %d malloc stack error!",__func__,__LINE__);
                break;
            }

            sprintf(taskname,"download executor:%d",i);
            ERR_PRINTF("dgp [%s] start:%s, prio %d\n",__func__,taskname,p_task_priorty[i]);

            p_dl_group_ctx->p_dl_ctx[i] = (DOWNLOAD_CONTEXT*)malloc(sizeof(DOWNLOAD_CONTEXT));
            if(!p_dl_group_ctx->p_dl_ctx[i])
            {
                ERR_PRINTF("dgp [%s] %d malloc download context error!",__func__,__LINE__);
                free(p_stack);
                break;
            }

            p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
            memset(p_dl_ctx,0,sizeof(DOWNLOAD_CONTEXT));
            p_dl_ctx->dl_group = p_dl_group_ctx;
            p_dl_ctx->task_index = i;

            MT_BOOL ret = mtos_task_create((u8 *)"download executor", download_task_entry,p_dl_ctx,
                p_task_priorty[i], (u32*)p_stack,stack_size);
            if(ret)
            {
                ERR_PRINTF("dgp [%s] success to start %s\n",__func__,taskname);
                p_dl_ctx->p_stack = p_stack;
                p_dl_ctx->task_prio = p_task_priorty[i];
                p_dl_group_ctx->real_task_num = i+1;
            }
            else
            {
                ERR_PRINTF("dgp [%s] %d create task %d error!, priority %d\n",__func__,__LINE__,i,p_task_priorty[i]);
                free(p_stack);
                free(p_dl_group_ctx->p_dl_ctx[i]);
                break;
            }
        }
    }

    if(!i)
    {
        ERR_PRINTF("dgp [%s] %d create all download tasks error!\n",__func__,__LINE__);
        goto fail;
    }

    p_dl_group_ctx->get_write_buf = get_write_buf;
    p_dl_group_ctx->write_buf_update = write_buf_update;
    p_dl_group_ctx->download_done = download_done;
    p_dl_group_ctx->write_buf_full = write_buf_full;

    p_dl_group_ctx->expect_status = RUNNING;
    return (void*)p_dl_group_ctx;

fail:
    free_download_group(p_dl_group_ctx);
    return NULL;
}

/**
*
*       setup all download task
**
*
*/
MT_BOOL Start_Download_Group(void  *pHandle, char *http_header)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_cxt = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    if(!p_dl_cxt)
        return FALSE;

    if(p_dl_cxt->http_header)
        free(p_dl_cxt->http_header);

    if(http_header)
        p_dl_cxt->http_header =  strdup(http_header);
    else
    {
        if(p_dl_cxt->http_header)
            free(p_dl_cxt->http_header);
        p_dl_cxt->http_header = NULL;
    }

    p_dl_cxt->expect_status = RUNNING;
    return TRUE;
}
/**
*
*       stop all download task
**
*
*/
void Stop_Download_Group(void  *pHandle)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int i;
    int prio;
    int all_idle = 0;

    if(!p_dl_group_ctx)
        return;


    OS_PRINTF("dgp [%s] %d start start===\n",__func__,__LINE__);

    p_dl_group_ctx->expect_status = IDLE;

#if 0
    for(i=0; i<p_dl_group_ctx->real_task_num; i++)
    {
        p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
        if(p_dl_ctx->task_status == RUNNING)
        {
            prio = p_dl_ctx->task_prio;
            if(prio)
            {
                OS_PRINTF("[%s] %d abort download task prio %d\n",__func__,__LINE__,prio);
                Abort_Download_Task(prio, TRUE);
            }
        }
    }
#endif

    //check download status, and wait until tasks really have stopped
    do
    {
        all_idle = 0;
        p_dl_group_ctx->expect_status = IDLE;
        for(i=0; i<p_dl_group_ctx->real_task_num; i++)
        {
            p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
            all_idle += (p_dl_ctx->task_status == IDLE?0:1);
            //OS_PRINTF("[download_group] real_task_num %d,i=%d,status %d\n",p_dl_group_ctx->real_task_num,i,p_dl_ctx->task_status);
        }

        if(all_idle == 0)
            break;

        mtos_task_sleep(5);

    }while(1);

    if(p_dl_group_ctx->http_header)
        free(p_dl_group_ctx->http_header);
    p_dl_group_ctx->http_header = NULL;

        OS_PRINTF("dgp [%s] %d end end=====\n",__func__,__LINE__);

    return;
}

void Stop_Download_Task_In_Group(void  *pHandle, int task_index)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int i;
    int prio;
    int all_idle = 0;

    if(!p_dl_group_ctx || task_index < 0)
        return;

    //mtos_printk("[download_group] stop download, task index %d start\n",task_index);

    p_dl_ctx = p_dl_group_ctx->p_dl_ctx[task_index];
    if(p_dl_ctx->task_status == RUNNING)
    {
        p_dl_ctx->expect_status = IDLE;
    }


    //check download status, and wait until tasks really have stopped
     OS_PRINTF("dgp [%s] ===start start=====\n",__func__);

    do
    {
        if(p_dl_ctx->task_status == IDLE)
        {
            break;
        }

        mtos_task_sleep(20);

    }while(1);

     OS_PRINTF("dgp [%s] ===end end====\n",__func__);

    return;
}

void Reset_Download_Group(void  *pHandle)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    DOWNLOAD_CONTEXT * p_dl_ctx = NULL;
    int i;

    if(!pHandle)
        return;

    Stop_Download_Group(pHandle);

    for(i=0;i<p_dl_group_ctx->real_task_num;i++)
    {
        p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
        if(p_dl_ctx)
        {
            if(p_dl_ctx->p_url)
            {
                free(p_dl_ctx->p_url);
                p_dl_ctx->p_url = NULL;
            }
        }
    }
OS_PRINTF("dgp [%s] %d done\n",__func__,__LINE__);
    return;
}

MT_BOOL Assign_Download_Task(void * pDownloadGroupHandle,unsigned long cbParam, int task_index, char *url,
    int download_offset, int download_size,int content_len,
    void *p_download, void *queue, DOWNLOAD_MODE download_mode)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pDownloadGroupHandle;
    DOWNLOAD_CONTEXT* p_dl_ctx = NULL;

    if(!p_dl_group_ctx || !url)
        return FALSE;
    OS_PRINTF("dgp [%s]  mode %d, dp handle %x,task_index %d\n",__func__,download_mode,p_download, task_index);
    if(task_index >= MAX_DL_TASK_NUM)
        return FALSE;

    p_dl_ctx = p_dl_group_ctx->p_dl_ctx[task_index];

    if(p_download)
        p_dl_ctx->p_download_handle = p_download;

    p_dl_ctx->p_url = strdup(url);

    p_dl_ctx->task_param = cbParam;
    p_dl_ctx->download_offset = download_offset;
    p_dl_ctx->request_size = download_size;
    p_dl_ctx->content_len = content_len;
    p_dl_ctx->download_mode = download_mode;
    p_dl_ctx->queue = queue;

    //set task running
    p_dl_ctx->expect_status = RUNNING;
    p_dl_ctx->task_status = RUNNING;
//mtos_printk("assign task %d running\n",task_index);

    return TRUE;
}
/**
*
*       destroy all download task
**
*
*/
void Destroy_Download_Group(void  *pHandle)
{
    DOWNLOAD_GROUP_CONTEXT *p_dl_group_ctx = (DOWNLOAD_GROUP_CONTEXT*)pHandle;
    DOWNLOAD_CONTEXT* p_dl_ctx = NULL;
    if(!p_dl_group_ctx)
        return;

    p_dl_group_ctx->expect_status = EXIT_STATUS;
    ERR_PRINTF("dgp [%s]  start\n",__func__);

    //wait exit done
    while(1)
    {
        if(p_dl_group_ctx->real_task_num)
        {
            int i;
            int status = 0;
            for(i=0;i<p_dl_group_ctx->real_task_num;i++)
            {
                p_dl_ctx = p_dl_group_ctx->p_dl_ctx[i];
                //OS_PRINTF("[download_group] real_task_num %d,i=%d,status %d\n",p_dl_group_ctx->real_task_num,i,p_dl_ctx->task_status);
                status += (p_dl_ctx->task_status == EXIT_STATUS)?0:1;
            }
            if(status == 0)
                break;
            mtos_task_sleep(20);
        }
        else
            break;
    }


    //free
    free_download_group(p_dl_group_ctx);




    ERR_PRINTF("dgp [%s] %d done\n",__func__,__LINE__);

    return;
}
