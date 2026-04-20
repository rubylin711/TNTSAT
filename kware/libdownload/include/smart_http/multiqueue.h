/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef  __MULTI_QUEUE__
#define  __MULTI_QUEUE__

#include "mt_type.h"

#define  MULTI_QUEUE_BUF_LEN  (384*1024)

#define  MAX_QUEUE_NUM  (6)

typedef  enum{

   NORMAL_BUF_M  = 0,
   RING_BUF_M = 1,
}QUEUE_MODE;

typedef  enum{

   DOWNLOAD_SUCCESS  = 0,
   DOWNLOAD_ABORT = 1,
}DL_STATUS;

typedef struct{

  int start_pos;
  int end_pos;

}FILE_RANGE;

typedef struct{

	int  len;         /*len of the queue */
	int  read_pos;    /*offset of read pointer and "read_pos <= write_pos"*/
	int  write_pos;   /*offset of write pointer*/
	QUEUE_MODE mode;
	ulong * head_addr;   /**/
	ulong * tail_addr;
	int  ts_seg_num; /*ts segment number*/
	int  owner_id;    /*task priority: this task use the queue*/
	int  job_id;      /*job num: the queue is assigned to this job*/
	int  v_job_id;  /*only for ring buffer mode*/
	FILE_RANGE cur_range;
	int  id;/*id for queue*/
	MT_BOOL  used_flag;  /*identify whether the queue is now using*/
	MT_BOOL complete_job_flag; /*whether the job is finished: TRUE: finished  FALSE: not finish*/
	MT_BOOL abort_flag;/*whether the job is success or failed to download*/
	void * p_multiqueue;/*refer to multiqueue*/
}DATA_SEGMENT_QUEUE;



typedef struct{

	/* for  ts segment queue */
	DATA_SEGMENT_QUEUE * seg_queue[MAX_QUEUE_NUM];
	DATA_SEGMENT_QUEUE * cur_queue;
	MT_BOOL  seek_pending;
	int   seek_pos;
	int baseJobID;
	int  total;
	int cur_fake_job_id;
	MT_BOOL  eos;
	QUEUE_MODE mode; 

}MULTI_QUEUE_HANDLE;

typedef struct{

  ulong * w_start_addr;
  int  max_w_byte;
  DATA_SEGMENT_QUEUE * p_working_queue;

}WRITE_POS_INFO;


typedef struct{

  int id[MAX_QUEUE_NUM];
  int  totalNum;

}JOB_IDS_ARRAY;

/*****
*
*  init mutiqueue
*
*   
********/
MT_BOOL mutiQueue_init(MULTI_QUEUE_HANDLE** pp_multiqueue,
	                     int buf_len,int queue_total,QUEUE_MODE mode);

/*****
*
*  deinit mutiqueue
*
*   
********/
void mutiQueue_deinit(MULTI_QUEUE_HANDLE* p_multiqueue);


/*****
*
*  read data from mutiquue
*
*   return  real read byte
********/
int mutiQueue_read(MULTI_QUEUE_HANDLE* p_multiqueue, unsigned char *buf/*OUT*/, int size/*in*/);

/*****
*
*   config new work mode
*
*   return  real read byte
********/
MT_BOOL mutiQueue_refconfig(MULTI_QUEUE_HANDLE* p_multiqueue,QUEUE_MODE mode);

/*****
*
*  only reset r/w pointer,but free memory
*
*   
********/
void mutiQueue_reset(MULTI_QUEUE_HANDLE* p_multiqueue);

void mutiQueue_clear_queues(MULTI_QUEUE_HANDLE* p_multiqueue);

DATA_SEGMENT_QUEUE*  mutiQueue_get_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue, int job_id);
/*****
*
*  try to get a data queue resource from mutiqueue
*   only for scheduler
*   
********/
DATA_SEGMENT_QUEUE*  mutiQueue_take_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue);

int  mutiQueue_get_queue_exist_jobs(MULTI_QUEUE_HANDLE* p_multiqueue,
	 JOB_IDS_ARRAY * p_array);

int  mutiQueue_reuse_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue,
	 DATA_SEGMENT_QUEUE * p_queue);

int  mutiQueue_reset_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue,
	 DATA_SEGMENT_QUEUE * p_queue);

/*****
*
*  free a queue
*
*   
********/
int  mutiQueue_set_baseJobID(MULTI_QUEUE_HANDLE* p_multiqueue,int baseID);
MT_BOOL  mutiQueue_check_child_drained(MULTI_QUEUE_HANDLE* p_multiqueue,int job_id);
MT_BOOL  mutiQueue_check_queue_exist(MULTI_QUEUE_HANDLE* p_multiqueue,int job_id);
MT_BOOL mutiQueue_reconfig(MULTI_QUEUE_HANDLE* p_multiqueue,QUEUE_MODE mode);
int mutiQueue_end_seek(MULTI_QUEUE_HANDLE* p_multiqueue);

int mutiQueue_start_seek(MULTI_QUEUE_HANDLE* p_multiqueue,int pos);
MT_BOOL mutiQueue_is_seek_pending(MULTI_QUEUE_HANDLE* p_multiqueue);
//int  mutiQueue_free_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue,DATA_SEGMENT_QUEUE * p_queue);
/*****
*
*  write data to special  queue
*
*      download excutor use
********/
int  queue_query_wp( DATA_SEGMENT_QUEUE*  p_queue,WRITE_POS_INFO* p_info/*INFO*/);
int  update_queue_writePosition(DATA_SEGMENT_QUEUE* p_queue, int inc_num,ulong p_working_queue);
int  notify_queue_downloaderStatus(DATA_SEGMENT_QUEUE* p_queue,DL_STATUS status,ulong p_working_queue);
int  set_queue_job_id(DATA_SEGMENT_QUEUE* p_queue,int job_id);
MT_BOOL check_queue_drained(DATA_SEGMENT_QUEUE* p_queue);
int  set_queue_range( DATA_SEGMENT_QUEUE*  p_queue,FILE_RANGE range);
int  set_multiqueue_eos_flag(DATA_SEGMENT_QUEUE* p_queue,MT_BOOL is_eos);

/*****
*
*  get free space of current queue
*
*   
********/
//int  queue_get_freespace(DATA_SEGMENT_QUEUE*  p_queue);



#endif



