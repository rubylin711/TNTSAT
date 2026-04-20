/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __JOBS_H__
#define __JOBS_H__

#define MAX_JOB_DESCRIPTION    (2)

/***
*
* describe one job
*
*/
typedef struct{
 int seg_size;/*size of this ts segment*/
 int offset;   /*offset in the ts*/
 //int recv_data_size; /*size of received data*/
 MT_BOOL  is_assigned;  /*identify the job have beed assigned to downloader*/
 MT_BOOL complete_flag; /*the job is finished*/
 MT_BOOL success_flag;/*the job is success or failed to download*/
 //DOWNLOAD_TASK_STATUS  owner_status;/*the owner is the download excutor */
 u32   begin_time;/*ms*/
 u32   end_time;/*ms*/
 //TS_SEGMENT_QUEUE * p_output_queue;
 int job_index;/*index identify the job*/
 int job_id;/*increase identify the job*/

 void * p_jobs_desc;
 void *pDownloadHandle;/*download handle, if it has been created in another way*/
}DL_TS_SEG_JOB;

/***
*
*  status of all jobs (include the assigned and unassigned)
*
*/
typedef struct{
  DL_TS_SEG_JOB  **job_array;/*job handle array*/
  int job_total;/*total number of job*/
  int assigned_job_num;/*assigned job number*/
  int complete_job_num;/*complete job number*/
  MT_BOOL  finish_flag;/*all jobs has been completed!!*/

  int content_length;/*content-length of server response*/
  char *url;/*download url*/
  char *header;/*download header*/

  //int jobs_id;/*jobs_id*/
  MT_BOOL single_job_mode;/*only one job mode*/
  //MT_BOOL busy;/*if jobs handle is busy*/
  char *redirect_url;
  MT_BOOL reconnect;
}JOBS_DESCRIPTION;

/**
 * generate jobs of current url, which will download segment of url.
 * eg. url file is 100M, piece_buf_size is 1M, and this function will generate 100M/1M=100 blocks.
 *input para:
 *      url:download url
 *      header:download header, maybe user-agent will set by up-layer.
 *      piece_buf_size:segment size of file.
 *output:
 *      
 *
 *
 * @return handl of jobs managerment.
 */
JOBS_DESCRIPTION * Gen_Jobs(char *url,char *header, int piece_buf_size, MT_BOOL reconnect);

/**
 * get single mode of jobs management. if server not support "range", we can not segment file. so only one job exists, and this is single mode.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *output:
 *      
 *
 *
 * @return true means single mode, false means not.
 */
MT_BOOL Get_Jobs_Single_Mode(JOBS_DESCRIPTION* p_jobs_desc);

/**
 * get download handle if download handle has been created.
 * sometimes, first time we will create a download handle in main thread, and then we will use in download task, to avoid a same http connect.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *output:
 *      
 *
 *
 * @return download handle.
 */
void* Get_Job_Download_Handle(DL_TS_SEG_JOB *p_job_desc);

/**
 * get next valid job in jobs handle.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *output:
 *      
 *
 *
 * @return job handle.
 */
DL_TS_SEG_JOB* Get_Next_Job(JOBS_DESCRIPTION* p_jobs_desc);

/**
 * get indicated job handle.get job handle from jobs handle , using job_index.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index: index of job, which is from 0->(job_num-1)
 *output:
 *      
 *
 *
 * @return job handle.
 */
DL_TS_SEG_JOB* Get_Job_From_Index(JOBS_DESCRIPTION* p_jobs_desc, int job_index);

/**
 * get indicated job handle.get job handle from jobs handle , using job_id.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_id: id of job
 *output:
 *      
 *
 *
 * @return job handle.
 */
DL_TS_SEG_JOB* Get_Job_From_Id(JOBS_DESCRIPTION* p_jobs_desc, int job_id);

/**
 * get final job in jobs handle.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *output:
 *      
 *
 *
 * @return job handle.
 */
DL_TS_SEG_JOB* Get_Final_Job(JOBS_DESCRIPTION* p_jobs_desc);

/**
 * assign job to job management.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index: index of job, which is from 0->(job_num-1)
 *output:
 *      
 *
 *
 * @return true means assign success, false means assign failed.
 */
MT_BOOL Assign_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index);

/**
 * complete job to job management.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index: index of job, which is from 0->(job_num-1)
 *      result:true or false, true means job process successfully, false means job process failed.
 *output:
 *      
 *
 *
 * @return true means complete success, false means complete failed.
 */
MT_BOOL Complete_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index, MT_BOOL result);

/**
 * reactive one job, becasue seek back will use this feature.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index: index of job
 *output:
 *      
 *
 *
 * @return true means reactive success, false means reactive failed.
 */
MT_BOOL Reactive_Job(JOBS_DESCRIPTION* p_jobs_desc, int job_index);

/**
 * reactive several jobs, becasue seek back will use this feature.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index_start: start index of job
 *      job_index_end: end index of job
 *output:
 *      
 *
 *
 * @return true means reactive success, false means reactive failed.
 */
MT_BOOL Reactive_Jobs(JOBS_DESCRIPTION* p_jobs_desc, int job_index_start, int job_index_end);

/**
 * if current job is the final in jobs.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *      job_index: index of job, which is from 0->(job_num-1)
 *output:
 *      
 *
 *
 * @return true means the final job, false means not the final job.
 */
MT_BOOL Is_Final_Job_In_Jobs(JOBS_DESCRIPTION* p_jobs_desc, int job_index);

/**
 * whether all the jobs be finished.
 * 
 *input para:
 *      p_jobs_desc:jobs description, from Gen_Jobs function.
 *output:
 *      
 *
 *
 * @return true means finished, false means not finished.
 */
MT_BOOL Is_Jobs_Finished(JOBS_DESCRIPTION* p_jobs_desc);

/**
 * destroy jobs handle
 * 
 *input para:
 *      job_index: index of job, which is from 0->(job_num-1)
 *output:
 *      
 *
 *
 * @return null.
 */
void Destroy_Jobs(JOBS_DESCRIPTION * p_jobs_desc_hdl);



void Set_Job_Begin_Time(DL_TS_SEG_JOB* p_job_desc, u32 begin_time);
u32 Get_Job_Begin_Time(DL_TS_SEG_JOB* p_job_desc);
u32 jobs_get_time_ms();
#endif

