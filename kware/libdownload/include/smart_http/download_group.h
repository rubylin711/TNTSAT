/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __MT_DOWNLOAD_GROUP__
#define  __MT_DOWNLOAD_GROUP__

#include "multiqueue.h"

#define  MAX_DL_TASK_NUM (4)

#define DOWNLOAD_GROUP_TIMEOUT    (5)
#define  TMP_BUF_LEN (128*1024+8)

typedef enum{
  IDLE = 0,
  RUNNING = 1,
  EXIT_STATUS = 2,
}DOWNLOAD_TASK_STATUS;


typedef enum{
  DOWNLOAD_MODE_ERROR_SKIP = 0,//if error found when downloading, will skip current download
  DOWNLOAD_MODE_ERROR_REDO = 1,//if error found when downloading, will redo this download
}DOWNLOAD_MODE;



void *Get_Queue_From_Download_Group(void  *pHandle, int task_index);

/**
 * init download group
 *input para:
 *      get_write_buf:callback,when download task will write data, it will get write buf address from this callback func.
 *      write_buf_update: callback,when download task write part of data in write buf ,it will tell the uplayer the size.
 *      download_done:callback,when download task complete current download task.
 *      task_num:will create tasks number.
 *      p_task_priorty:task priority which will be created.
 *      stack_size:stack size of task.
 *
 * @return download group handle.
 */
	void * Init_Download_Group(
																			MT_BOOL (*get_write_buf)(unsigned int cbParam, void *queue, int task_index, WRITE_POS_INFO * p_info),
																			void (*write_buf_update)(ulong cbParam, void *queue, int task_index, int len),
																			void (*download_done)(ulong cbParam1, unsigned int cbParam2, void *queue, int task_index, MT_BOOL success),
																			void (*write_buf_full)(ulong cbParam, void *queue, u32 begin_time, int buf_used_len),
																			int task_num,
																			int * p_task_priorty,
																			int stack_size);



/**
 * find the idle task
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *output:
 *      p_task_index:task index, from 0 to (real_task_num-1)
 *
 *
 * @return result, TRUE means find, FALSE means not find.
 */
MT_BOOL Find_Idle_Download_Task(void * pHandle,int * p_task_index);

/**
 * start download task, after this operation, all the download tasks will be running status
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *
 *
 * @return result, TRUE means start ok, FALSE means start failed.
 */
MT_BOOL Start_Download_Group(void  *pHandle, char *http_header);

/**
 * stop download task, after this operation, all download tasks will be idle status
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *
 *
 * @return null.
 */
void Stop_Download_Group(void  *pHandle);

/**
 * stop indicated download task, after this operation, this download task will be idle status
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *      p_task_index:task index, from 0 to (real_task_num-1)
 *
 * @return null.
 */
void Stop_Download_Task_In_Group(void  *pHandle, int task_index);

/**
 * reset download group, after this operation, all download tasks will be idle status, and free resources, but not destroy tasks.
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *
 *
 * @return null.
 */
void Reset_Download_Group(void  *pHandle);

/**
 * assign job to download task.
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *      cbParam:parameters address about this job.
 *      task_index: task index ,which is from 0 to (real_task_num-1).
 *      url:download url
 *      download_offset:download offset in file, for range feature
 *      download_size:download size 
 *      p_download:download handle ,if it has been created in another place, if null means has not been created in another place.
 *      download_mode:mode, decide whether to reconnect.
 *
 * @return TRUE or FALSE.
 */
MT_BOOL Assign_Download_Task(void * pDownloadGroupHandle,unsigned long cbParam, int task_index, char *url,
    int download_offset, int download_size,int content_len,
    void *p_download, void *queue, DOWNLOAD_MODE download_mode);

/**
 * destroy download task, after this operation, all the download tasks will destroy, including exit task, release all the resources about task.
 *input para:
 *      pHandle:download group handle, which is generated in Init_Download_Group.
 *
 *
 * @return null.
 */
void Destroy_Download_Group(void  *pHandle);




#endif
