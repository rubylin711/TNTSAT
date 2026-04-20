/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
//#include "sys_define.h"
#include <sys/time.h>

//#include "osal_mtos.h"
#include "mtos_misc.h"

#include "jobs.h"
#include "download_api.h"
#include "download_group.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define ERR_PRINTF                          printf
#endif


#define USE_SMART_HTTP_WHEN_NO_RANGE    //single job mode, if server do not support range mode, we will use single job mode

enum{
    INVALID_DW_HANDLE = 0,
    ACCEPT_RANGE_BYTE = 1,
    NO_ACCETP_RANGE_BYTE = 2,
    SUCCESS_GET_HEADER_INFO   = 3,

};

/**************************************************************************************
*
*
*
*
*
*
*****************************************************************************************/

u32 jobs_get_time_ms()
{
    return mtos_ticks_get()*10;
}
/**************************************************************************************
*
*
*   2,   internal methods  for handling jobs
*
*
*
*****************************************************************************************/
//return result of download ,success or failed.
static int get_jobs_content_length(char *url,char *header, void **pDownload, int *content_length, JOBS_DESCRIPTION *pJobsDesp,int *acceptRange)
{
    HttpDownloadHeader downRespHeader;
    void *pDownHandle = NULL;
    int i;

    downRespHeader.Content_length = 0;
    downRespHeader.redirect_url = NULL;
    for(i=0; i<3;i++)
    {
        pDownHandle = Nw_Http_Download_Start(url, FALSE, NULL, 0, DOWNLOAD_GROUP_TIMEOUT, &downRespHeader, header);
        if(pDownHandle)
            break;
        ERR_PRINTF("get_jobs_content_length resp %d\n",downRespHeader.resp_code);
        if(downRespHeader.resp_code == 404 || downRespHeader.resp_code == 403)
            break;
    }
    if(!pDownHandle)
    {
        ERR_PRINTF("jobs [%s] %d error!\n",__func__,__LINE__);
        return INVALID_DW_HANDLE;
    }
    if(downRespHeader.redirect_url)
    {
        pJobsDesp->url = strdup(downRespHeader.redirect_url);
    }
    else
    {
      pJobsDesp->url = strdup(url);
    }
    OS_PRINTF("jobs [%s]  content_length %d, accept_range %d, http resp code %d\n",__func__,
        downRespHeader.Content_length, downRespHeader.accept_range, downRespHeader.resp_code);

    *pDownload = pDownHandle;


   *acceptRange = downRespHeader.accept_range;
   if(downRespHeader.Content_length <= 0)
    {
        *content_length = -1;
        //Nw_Http_Download_Stop(pDownHandle);
        return SUCCESS_GET_HEADER_INFO;
    }

    *content_length = downRespHeader.Content_length;
    //Nw_Http_Download_Stop(pDownHandle);
    return SUCCESS_GET_HEADER_INFO;
}
static void destroy_job_array(DL_TS_SEG_JOB  **job_array, int number)
{
    int i;

    if(job_array)
    {
        if(number)
        {
            for(i=0; i<number;i++)
            {
                if(job_array[i])
                {
                    free(job_array[i]);
                    job_array[i] = NULL;
                }
            }
        }

        free(job_array);
    }
}

static MT_BOOL Is_Job_Complete(JOBS_DESCRIPTION *jobs, int job_index)
{
    DL_TS_SEG_JOB *job = NULL;
    if(!jobs)
        return FALSE;

    if(job_index >= jobs->job_total)
        return FALSE;

    job = jobs->job_array[job_index];
    if(!job)
        return FALSE;

    return job->complete_flag;
}

static MT_BOOL Is_Job_Success(JOBS_DESCRIPTION *jobs, int job_index)
{
    DL_TS_SEG_JOB *job = NULL;
    if(!jobs)
        return FALSE;

    if(job_index >= jobs->job_total)
        return FALSE;

    job = jobs->job_array[job_index];
    if(!job)
        return FALSE;

    return job->success_flag;
}

static void release_jobs_mem_except_job_array(JOBS_DESCRIPTION *pJobsDesp)
{
    if(!pJobsDesp)
        return;

    if(pJobsDesp->url)
        free(pJobsDesp->url);

    if(pJobsDesp->header)
        free(pJobsDesp->header);

    if(pJobsDesp->redirect_url)
        free(pJobsDesp->redirect_url);

    free(pJobsDesp);

    return;
}
/**************************************************************************************
*
*
*
*
*
*
*****************************************************************************************/

static char *add_http_header_for_range_dl(char *header, int offset, int seg_size)
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
/**************************************************************************************
*
*
*
*
*
*
*****************************************************************************************/

static int is_partcial_get_valid(char *url, char *header, void **pDownload)
{
    HttpDownloadHeader downRespHeader;
    void *pDownHandle = NULL;
    int i = 0;
    int ret = INVALID_DW_HANDLE;

	memset(&downRespHeader,0,sizeof(HttpDownloadHeader));
    downRespHeader.Content_length = 0;
    downRespHeader.redirect_url = NULL;

    for(i=0; i<3;i++)
    {
        pDownHandle = Nw_Http_Download_Start(url, FALSE, NULL, 0, DOWNLOAD_GROUP_TIMEOUT, &downRespHeader, header);
        if(pDownHandle)
            break;
        ERR_PRINTF("get_jobs_content_length resp %d\n",downRespHeader.resp_code);
        if(downRespHeader.resp_code == 404 || downRespHeader.resp_code == 403)
            break;
    }

    if(!pDownHandle)
    {
        ERR_PRINTF("jobs [%s] %d error!\n",__func__,__LINE__);
        ret = INVALID_DW_HANDLE;
    }
    else if(downRespHeader.resp_code == 206){
        ret = ACCEPT_RANGE_BYTE;
    }
    else{
        ret = NO_ACCETP_RANGE_BYTE;
    }

    *pDownload = pDownHandle;
    return ret;

}
/**************************************************************************************
*
*
*
*
*
*
*****************************************************************************************/
#define RANGE_CON_MAX (3)

JOBS_DESCRIPTION * Gen_Jobs(char *url,char *header, int piece_buf_size, MT_BOOL reconnect)
{

    //TODO:look for jobs
    JOBS_DESCRIPTION *pJobsDesp = NULL;
    void * pDownHandle = NULL;
    void * pDownHandle_tmp[RANGE_CON_MAX];
    DL_TS_SEG_JOB *p_job = NULL;
    MT_BOOL single_job_mode = FALSE;//because server not support range mode ,or we can not get content-length from server
    int gJob_id = 0;
    int check_job_num = 0;

    if(!url)
    {
        ERR_PRINTF("jobs [%s] %d url is null !\n",__func__,__LINE__);
        return NULL;
    }

    pJobsDesp = (JOBS_DESCRIPTION*)malloc(sizeof(JOBS_DESCRIPTION));
    if(!pJobsDesp)
    {
        ERR_PRINTF("jobs [%s] %d error!\n",__func__,__LINE__);
        return NULL;
    }

    memset(pJobsDesp,0,sizeof(JOBS_DESCRIPTION));
    OS_PRINTF("jobs [%s] %d,url1 %s\n",__func__,__LINE__,url);

    /*************************get content-length***********************/
    char *p_new_range_header = NULL;
    int length = 0;
    int jobs;
    int ssize = 0;
    int soffset = 0;
    int last_piece_size;
    int part_check_num = 2;
    int ret = 0;
    int  accept_range = 0;

    //if(part_check_num < RANGE_CON_MAX);
    //{
    ERR_PRINTF("jobs [%s] %d error part_check_num!\n",__func__,__LINE__);
    //}

    memset(pDownHandle_tmp, 0, sizeof(pDownHandle_tmp));

	/***************************************************************************
	*
	*
	*
	*  check whether server accept request in range mode  really
	*
	*
	*         for example:  iqiyi server work in fake 'accept range byte'
    *
	*
	*
	***************************************************************************/
    do
    {
        if(check_job_num == 0)
		{
            //first job
            ssize = 0;
            soffset = 0;
        }
		else
		{
            //second job
            if(jobs == 2 && last_piece_size)
                ssize = last_piece_size;
            else
                ssize = piece_buf_size;
            soffset = piece_buf_size;
        }

        p_new_range_header = add_http_header_for_range_dl(header, soffset, ssize);
        if(!p_new_range_header)
        {
            ERR_PRINTF("[%s][ERROR] fail to malloc range header!\n",__func__);
            release_jobs_mem_except_job_array(pJobsDesp);
            return NULL;
        }


		//get info about contentLen and 'accept range bytes' from response header
        if(check_job_num == 0)
		{
            ret = get_jobs_content_length(url,p_new_range_header,&pDownHandle, &length, pJobsDesp,&accept_range);
        }
        else//try to check whether server can really support accept range bytes
		{
            ret = is_partcial_get_valid(url, p_new_range_header, &pDownHandle);
        }


        //ERR_PRINTF("jobs [%s][ERROR] download start failed dl=0x%x, cnt=%d\n",__func__,pDownHandle,check_job_num);
        if(check_job_num < RANGE_CON_MAX)
            pDownHandle_tmp[check_job_num] = pDownHandle;


        if(check_job_num == 0)
		{

            if(ret == INVALID_DW_HANDLE)
            {
                ERR_PRINTF("jobs [%s][ERROR] download start failed\n",__func__);
                free(p_new_range_header);
                p_new_range_header = NULL;
                release_jobs_mem_except_job_array(pJobsDesp);
                return NULL;
            }

            if(accept_range == 0 || length <= 0)
            {
                OS_PRINTF("jobs [%s] can not get content_length!\n",__func__);
                single_job_mode = TRUE;

                free(p_new_range_header);
                #ifndef USE_SMART_HTTP_WHEN_NO_RANGE
                release_jobs_mem_except_job_array(pJobsDesp);
                Nw_Http_Download_Stop(pDownHandle_tmp[check_job_num]);
                return NULL;
                #endif
            }
            else{
                free(p_new_range_header);
            }

    //set single job mode
    pJobsDesp->single_job_mode = single_job_mode;

    //save url and header
	if(url)
    	pJobsDesp->url = strdup(url);
    if(header)
        pJobsDesp->header = strdup(header);

            pJobsDesp->content_length = length;

            //get total jobs number
            jobs = single_job_mode?1: length/piece_buf_size;
            last_piece_size = single_job_mode ? 0:(length-jobs*piece_buf_size);
            if(last_piece_size)
                jobs++;

            if(jobs > 1)
			{
                Nw_Http_Download_Remove_Curr_Task_Prio();

            }
			else
			{
                break;
            }

        }
		else
		{//dounble check whether work in single job mode!!!

			if(ret == ACCEPT_RANGE_BYTE)
			{
                ERR_PRINTF("jobs [%s][GOOD] Particial start Success!!!\n",__func__);
            }
			else
			{
                ERR_PRINTF("jobs [%s][ERROR] Particial start failed cnt=%d\n",__func__,  check_job_num);
                single_job_mode = TRUE;
                pJobsDesp->single_job_mode = single_job_mode;
                jobs = 1;
                last_piece_size = 0;
			}


            if(ACCEPT_RANGE_BYTE != ret && pDownHandle_tmp[check_job_num])
			{
                Nw_Http_Download_Stop(pDownHandle_tmp[check_job_num]);
            }

            free(p_new_range_header);
            p_new_range_header = NULL;

        }

        check_job_num++;

    }while(check_job_num < part_check_num);



    //malloc jobs step1
    pJobsDesp->job_array = (DL_TS_SEG_JOB**)malloc(jobs*sizeof(DL_TS_SEG_JOB*));
    if(!pJobsDesp->job_array)
    {
        ERR_PRINTF("jobs [%s] %d error!\n",__func__,__LINE__);
        release_jobs_mem_except_job_array(pJobsDesp);

        Nw_Http_Download_Stop(pDownHandle_tmp[0]);
        return NULL;
    }
    memset(pJobsDesp->job_array, 0, jobs*sizeof(DL_TS_SEG_JOB*));


    //malloc jobs step2
    int i = 0;
    for(i=0;i<jobs;i++)
    {
        pJobsDesp->job_array[i] = (DL_TS_SEG_JOB*)malloc(sizeof(DL_TS_SEG_JOB));
        if(!pJobsDesp->job_array[i])
        {
            ERR_PRINTF("jobs [%s] %d error!\n",__func__,__LINE__);
            destroy_job_array(pJobsDesp->job_array,i);
            release_jobs_mem_except_job_array(pJobsDesp);

            if(i < RANGE_CON_MAX)
                Nw_Http_Download_Stop(pDownHandle_tmp[i]);

            return NULL;
        }
    }

    pJobsDesp->job_total = jobs;
    ERR_PRINTF("[%s] job_total[%d] consume %ld bytes\n",__FUNCTION__,jobs,jobs*sizeof(DL_TS_SEG_JOB));

    //set jobs value
    for(i=0;i<jobs;i++)
    {
        p_job = pJobsDesp->job_array[i];

        memset(p_job,0, sizeof(DL_TS_SEG_JOB));
        p_job->job_id = gJob_id++;
        p_job->job_index = i;
        p_job->offset = single_job_mode ? 0: i*piece_buf_size;

        if(last_piece_size && i==(jobs-1))
        {
            p_job->seg_size = last_piece_size;
        }
        else
        {
            p_job->seg_size = single_job_mode ? 0 : piece_buf_size;
        }

        p_job->p_jobs_desc = pJobsDesp;
        if(i < part_check_num && pDownHandle_tmp[i])//first job will save download handle, to avoid new connect...
            p_job->pDownloadHandle = pDownHandle_tmp[i];

        //OS_PRINTF("[%s] job_index %d, job_id %d, offset %d, seg size %d\n",__func__,
        //    p_job->job_index,p_job->job_id,p_job->offset,p_job->seg_size);
    }

    pJobsDesp->reconnect = reconnect;
    #ifndef __LINUX__
    Nw_Http_Download_Remove_Curr_Task_Prio();
    #endif
    //pJobsDesp->busy = TRUE;
    return pJobsDesp;
}

MT_BOOL Get_Jobs_Single_Mode(JOBS_DESCRIPTION* p_jobs_desc)
{
    if(!p_jobs_desc)
        return FALSE;

    return p_jobs_desc->single_job_mode;
}
void* Get_Job_Download_Handle(DL_TS_SEG_JOB *p_job_desc)
{
    if(!p_job_desc)
    {
        ERR_PRINTF("jobs [%s] p_job_desc is null\n",__func__);
        return NULL;
    }

    return p_job_desc->pDownloadHandle;
}
static void clear_job_download_handle(DL_TS_SEG_JOB *p_job_desc)
{
    if(!p_job_desc)
    {
        ERR_PRINTF("jobs [%s] p_job_desc is null\n",__func__);
        return;
    }

    if(p_job_desc->pDownloadHandle)
    {
        Nw_Http_Download_Stop(p_job_desc->pDownloadHandle);
        p_job_desc->pDownloadHandle = NULL;
        ERR_PRINTF("jobs [%s] close first connection\n",__func__);
    }
    return;
}
DL_TS_SEG_JOB* Get_Next_Job(JOBS_DESCRIPTION* p_jobs_desc)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    int i;
    if(!p_jobs_desc)
    {
        ERR_PRINTF("jobs [%s] p_jobs_desc is null\n",__func__);
        return NULL;
    }

    if(!p_jobs_desc->job_array)
    {
        ERR_PRINTF("jobs [%s] job_array is null\n",__func__);
        return NULL;
    }

    if(p_jobs_desc->job_total <= 0 || p_jobs_desc->job_total == p_jobs_desc->assigned_job_num
        || p_jobs_desc->job_total == p_jobs_desc->complete_job_num
        || p_jobs_desc->finish_flag)
    {
        OS_PRINTF("jobs [%s] job_total %d,assigned_job_num %d,complete_job_num %d,finish_flag %d, return null\n",__func__,
           p_jobs_desc->job_total,p_jobs_desc->assigned_job_num,
            p_jobs_desc->complete_job_num,p_jobs_desc->finish_flag);
        return NULL;
    }

    for(i=0; i<p_jobs_desc->job_total;i++)
    {
        p_job_desc = p_jobs_desc->job_array[i];
        if(p_job_desc && !p_job_desc->is_assigned && !p_job_desc->complete_flag)
        {
            OS_PRINTF("jobs [%s] job_index %d, assigned %d,complete %d\n",__func__,i,p_job_desc->is_assigned,p_job_desc->complete_flag);
            return p_job_desc;
        }
    }

    return NULL;//not find
}

DL_TS_SEG_JOB* Get_Job_From_Index(JOBS_DESCRIPTION* p_jobs_desc, int job_index)
{
    if(!p_jobs_desc)
    {
        ERR_PRINTF("jobs [%s] p_jobs_desc is null\n",__func__);
        return NULL;
    }

    if(!p_jobs_desc->job_array)
    {
        ERR_PRINTF("jobs [%s] job_array is null\n",__func__);
        return NULL;
    }

    if(job_index >= p_jobs_desc->job_total)
        return NULL;

    return p_jobs_desc->job_array[job_index];
}

DL_TS_SEG_JOB* Get_Job_From_Id(JOBS_DESCRIPTION* p_jobs_desc, int job_id)
{
    int i = 0;
    DL_TS_SEG_JOB *p_job_desc = NULL;

    if(job_id < 0)
        return NULL;

    if(!p_jobs_desc)
    {
        ERR_PRINTF("jobs [%s] p_jobs_desc is null\n",__func__);
        return NULL;
    }

    if(!p_jobs_desc->job_array)
    {
        ERR_PRINTF("jobs [%s] job_array is null\n",__func__);
        return NULL;
    }

    for(i=0; i<p_jobs_desc->job_total; i++)
    {
        p_job_desc = p_jobs_desc->job_array[i];
        if(p_job_desc && p_job_desc->job_id == job_id)
        {
            OS_PRINTF("jobs [%s] get job handle with id=%d\n",__func__,job_id);
            return p_job_desc;
        }
    }

    OS_PRINTF("jobs [%s] can not get job handle with id=%d\n",__func__,job_id);
    return NULL;
}

DL_TS_SEG_JOB* Get_Final_Job(JOBS_DESCRIPTION* p_jobs_desc)
{
    if(!p_jobs_desc)
    {
        ERR_PRINTF("jobs [%s] p_jobs_desc is null\n",__func__);
        return NULL;
    }

    if(!p_jobs_desc->job_array)
    {
        ERR_PRINTF("jobs [%s] job_array is null\n",__func__);
        return NULL;
    }

    return p_jobs_desc->job_array[p_jobs_desc->job_total-1];
}

static MT_BOOL Assign_Job_Reset(JOBS_DESCRIPTION* p_jobs_desc, int job_index)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;

    p_job_desc = p_jobs_desc->job_array[job_index];
    if(!p_job_desc)
        return FALSE;

    if(p_job_desc->is_assigned)
        p_jobs_desc->assigned_job_num--;

    //set value
    p_job_desc->is_assigned = FALSE;
    p_job_desc->complete_flag = FALSE;
    p_job_desc->success_flag = FALSE;

    p_job_desc->begin_time = 0;
    p_job_desc->end_time = 0;

    OS_PRINTF("jobs [%s] job_index %d, job_id %d\n",__func__,p_job_desc->job_index,p_job_desc->job_id);
    return TRUE;
}

void Set_Job_Begin_Time(DL_TS_SEG_JOB* p_job_desc, u32 begin_time)
{
    if(!p_job_desc)
        return;

    p_job_desc->begin_time = begin_time;

    return;
}
u32 Get_Job_Begin_Time(DL_TS_SEG_JOB* p_job_desc)
{
    if(!p_job_desc)
        return 0;

    return p_job_desc->begin_time;
}
MT_BOOL Assign_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;

    p_job_desc = p_jobs_desc->job_array[job_index];
    if(!p_job_desc)
        return FALSE;

    //set value
    p_job_desc->is_assigned = TRUE;
    p_job_desc->complete_flag = FALSE;
    p_job_desc->success_flag = FALSE;

    p_jobs_desc->assigned_job_num++;

    OS_PRINTF("jobs [%s] job_index %d, job_id %d\n",__func__,p_job_desc->job_index,p_job_desc->job_id);
    return TRUE;
}

MT_BOOL Complete_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index, MT_BOOL result)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;

    p_job_desc = p_jobs_desc->job_array[job_index];
    if(!p_job_desc)
        return FALSE;

    if(p_job_desc->complete_flag)
        return TRUE;

    p_job_desc->complete_flag = TRUE;
    p_job_desc->success_flag = result;
    p_job_desc->pDownloadHandle = NULL;
    p_job_desc->end_time = jobs_get_time_ms();

    p_jobs_desc->complete_job_num++;
    if(p_jobs_desc->complete_job_num == p_jobs_desc->job_total)
        p_jobs_desc->finish_flag = TRUE;

    OS_PRINTF("jobs [%s] job_index %d, job_id %d, job_total %d,complete_job_num %d, finished %d\n",__func__,
        p_job_desc->job_index,p_job_desc->job_id,p_jobs_desc->job_total,p_jobs_desc->complete_job_num,
        p_jobs_desc->finish_flag);
    return TRUE;
}
MT_BOOL Reactive_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index < 0 || job_index >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;


    p_job_desc = p_jobs_desc->job_array[job_index];
    if(p_job_desc)
    {
        if(p_job_desc->complete_flag)
            p_jobs_desc->complete_job_num--;
        if(p_job_desc->is_assigned)
            p_jobs_desc->assigned_job_num--;

        p_job_desc->complete_flag = FALSE;
        p_job_desc->is_assigned = FALSE;
        p_job_desc->success_flag = FALSE;
        p_job_desc->begin_time = 0;
        p_job_desc->end_time = 0;

        p_jobs_desc->finish_flag = FALSE;

        OS_PRINTF("jobs [%s] job_index %d, job_id %d, job_total %d,complete_job_num %d, finished %d\n",__func__,
            p_job_desc->job_index,p_job_desc->job_id,p_jobs_desc->job_total,p_jobs_desc->complete_job_num,
            p_jobs_desc->finish_flag);
    }

    return TRUE;
}
MT_BOOL Reactive_Jobs(JOBS_DESCRIPTION* p_jobs_desc, int job_index_start, int job_index_end)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    int i;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index_start >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;

    if(job_index_end >= p_jobs_desc->job_total || job_index_end <= 0)
        job_index_end = p_jobs_desc->job_total-1;

    for(i=job_index_start; i<=job_index_end; i++)
    {
        p_job_desc = p_jobs_desc->job_array[i];
        if(p_job_desc && (p_job_desc->complete_flag || p_job_desc->is_assigned))
        {
            if(p_job_desc->complete_flag)
                p_jobs_desc->complete_job_num--;
            if(p_job_desc->is_assigned)
                p_jobs_desc->assigned_job_num--;

            p_job_desc->complete_flag = FALSE;
            p_job_desc->is_assigned = FALSE;
            p_job_desc->success_flag = FALSE;
            p_job_desc->begin_time = 0;
            p_job_desc->end_time = 0;
            p_job_desc->pDownloadHandle = NULL;
            OS_PRINTF("jobs [%s] job_index %d, job_id %d, job_total %d,complete_job_num %d, finished %d\n",__func__,
                p_job_desc->job_index,p_job_desc->job_id,p_jobs_desc->job_total,p_jobs_desc->complete_job_num,
                p_jobs_desc->finish_flag);
        }
    }

    p_jobs_desc->finish_flag = FALSE;

    return TRUE;
}
MT_BOOL Is_Final_Job_In_Jobs(JOBS_DESCRIPTION* p_jobs_desc, int job_index)
{
    DL_TS_SEG_JOB *p_job_desc = NULL;
    if(!p_jobs_desc)
        return FALSE;

    if(job_index >= p_jobs_desc->job_total)
        return FALSE;

    if(!p_jobs_desc->job_array)
        return FALSE;

    p_job_desc = p_jobs_desc->job_array[job_index];
    if(!p_job_desc)
        return FALSE;

    if(p_job_desc->job_index == (p_jobs_desc->job_total-1))
        return TRUE;

    return FALSE;
}

MT_BOOL Is_Jobs_Finished(JOBS_DESCRIPTION* p_jobs_desc)
{
    if(!p_jobs_desc)
        return FALSE;

    if(p_jobs_desc->single_job_mode)
    {
        if(p_jobs_desc->job_array && p_jobs_desc->job_array[0])
        {
            return p_jobs_desc->job_array[0]->success_flag;
        }
        else
            return FALSE;
    }
    else
        return p_jobs_desc->finish_flag;
}
/*
*
*
*
*
***/
void Destroy_Jobs(JOBS_DESCRIPTION * p_jobs_desc_hdl)
{
    //TODO:distroy all jobs
    if(!p_jobs_desc_hdl)
        return;


    //if not assign job to download group ,so we should close the first connection...
    if(p_jobs_desc_hdl->assigned_job_num == 0 && p_jobs_desc_hdl->job_array)
    {
        DL_TS_SEG_JOB *pJob = p_jobs_desc_hdl->job_array[0];
        if(pJob)
            clear_job_download_handle(pJob);
    }

    if(p_jobs_desc_hdl->job_array)
    {
        destroy_job_array(p_jobs_desc_hdl->job_array, p_jobs_desc_hdl->job_total);
    }

    release_jobs_mem_except_job_array(p_jobs_desc_hdl);
ERR_PRINTF("jobs [%s] %d done\n",__func__,__LINE__);
    return;
}


