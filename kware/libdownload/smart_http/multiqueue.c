/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
//#include "sys_define.h"
#include "mt_common.h"
#include <sys/time.h>

//#include "osal_mtos.h"

#include  "multiqueue.h"

#define  MAX_JOB_TOTAL  (100)
#define INVALID_JOB_ID  (-1)
#define  INVALID_SEEK_POS  (-9999)


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

static void mtos_task_sleep(unsigned int ms)
{
    MT_USLEEP(ms * 1000);
}

static DATA_SEGMENT_QUEUE * mutiQueue_find_dataready(MULTI_QUEUE_HANDLE* p_multiqueue);
static MT_BOOL check_all_queues_drained(MULTI_QUEUE_HANDLE * p_hdl);
static DATA_SEGMENT_QUEUE * creat_queue(int len)
{

	if(len <= 0){
		ERR_PRINTF("[%s][ERROR]====!!!\n",len);
		return NULL;
	}


	DATA_SEGMENT_QUEUE * p_queue = (DATA_SEGMENT_QUEUE *)malloc(sizeof(DATA_SEGMENT_QUEUE));
	if(p_queue){
		memset(p_queue,0,sizeof(DATA_SEGMENT_QUEUE));
		if(len > 0){
			p_queue->head_addr = malloc(len);
			if(p_queue->head_addr){

				memset(p_queue->head_addr,0,len);
				p_queue->job_id = INVALID_JOB_ID;
				p_queue->len = len;
				p_queue->tail_addr = p_queue->head_addr + p_queue->len - 1;
				p_queue->read_pos = 0;
				p_queue->write_pos = 0;

			}
			else{
				free(p_queue);
				p_queue = NULL;
			}
		}
	}
	return p_queue;
}

static void destroy_queue(DATA_SEGMENT_QUEUE * p_queue){

	if(p_queue){

		if(p_queue->head_addr){
			free(p_queue->head_addr);
			p_queue->head_addr = NULL;
		}

		free(p_queue);
	}

}
/*
 *
 *  return size of ts segment
 *
 */
static int queue_get_data_size(DATA_SEGMENT_QUEUE * p_queue)
{
	return p_queue->write_pos - p_queue->read_pos;
}


/*
 *
 *  read data from one queue
 *
 *    if no data ,return 0;
 *
 */
static int queue_read(DATA_SEGMENT_QUEUE * p_queue,
		char * buf/*OUT*/,int buf_size/*IN*/)
{
	int read_size = 0;
	int seg_size = p_queue->write_pos - p_queue->read_pos;
	MULTI_QUEUE_HANDLE * p_multiqueue = p_queue->p_multiqueue;
	int tmp_read_pos = 0;
	static int print_cnt = 0;
	static int print_cnt2 = 0;


      if( (p_multiqueue->seek_pos != INVALID_SEEK_POS)&&(buf_size > 0))
      {

	      /*seeking data is in this queue*/
          if(p_multiqueue->seek_pos >= p_queue->cur_range.start_pos
                && p_multiqueue->seek_pos <= p_queue->cur_range.end_pos)
          {
          		tmp_read_pos = p_multiqueue->seek_pos - p_queue->cur_range.start_pos;

          		if(tmp_read_pos >= 0
					&& tmp_read_pos < p_queue->write_pos)
          		{
	                p_queue->read_pos = tmp_read_pos;
	                seg_size = p_queue->write_pos - p_queue->read_pos;

					ERR_PRINTF("[%s]ok seek to:%d\n",__func__,p_multiqueue->seek_pos);

	                p_multiqueue->seek_pos = INVALID_SEEK_POS;


          		}
				else
				{
					if(print_cnt%10 == 0){

						ERR_PRINTF("[%s]seeking data not come in!!!:tmp_r:%d w:%d\n",
						__func__,p_multiqueue->seek_pos,
						tmp_read_pos,
						p_queue->write_pos);
						print_cnt = 0;
					}
					print_cnt++;

					return 0;
				}

          }
          else/*seeking data not in this queue*/
          {

		  		if(print_cnt2%10 == 0){

                	ERR_PRINTF("skip q(%d j:%d)\n",p_multiqueue->seek_pos,p_queue->job_id);
                //ERR_PRINTF("[%s]s_pos:%d e_pos:%d\n",
				//	__func__,p_queue->cur_range.start_pos,p_queue->cur_range.end_pos);
					print_cnt2 = 0;
		  		}

				print_cnt2++;

                p_queue->read_pos += seg_size;
                return 0;
          }
      }


    /**move data to buffer**/
	if(seg_size > 0 && buf_size > 0)
	{
		if(buf_size < seg_size)
		{
			OS_PRINTF("[%s] line %d,buf_size %d,seg_size %d,jobid %d,len %d,rdpos %d,wrpos %d,queid %d\n",
				__FUNCTION__,__LINE__,buf_size,seg_size,p_queue->job_id,
				p_queue->len,p_queue->read_pos,p_queue->write_pos,p_queue->id);

			memcpy(buf,(char *)(p_queue->head_addr+p_queue->read_pos),buf_size);
			p_queue->read_pos += buf_size;
			read_size = buf_size;

		}
		else
		{
			OS_PRINTF("[%s] line %d,buf_size %d,seg_size %d,jobid %d,len %d,rdpos %d,wrpos %d,queid %d\n",
				__FUNCTION__,__LINE__,buf_size,seg_size,p_queue->job_id,
				p_queue->len,p_queue->read_pos,p_queue->write_pos,p_queue->id);

				memcpy(buf,(char *)(p_queue->head_addr+p_queue->read_pos),seg_size);
				p_queue->read_pos += seg_size;
				read_size = seg_size;

		}

	}

	return read_size;
}
/*
 *
 *   note: don't reset the multiqueue handler of the queue and the queue 'id'
 *
 *
 *
 */
static void queue_reset(DATA_SEGMENT_QUEUE * p_queue)
{
    if(!p_queue)
        return;

    p_queue->abort_flag = FALSE;
    p_queue->complete_job_flag = FALSE;
    p_queue->job_id = INVALID_JOB_ID;
	p_queue->v_job_id = 0;
    p_queue->owner_id = -1;
    p_queue->read_pos = 0;
	p_queue->write_pos = 0;
    p_queue->ts_seg_num = 0;
    p_queue->used_flag = FALSE;
	p_queue->cur_range.end_pos = INVALID_SEEK_POS;
	p_queue->cur_range.start_pos = INVALID_SEEK_POS;

    return;
}

/******************************************************************
*
*
*
*
 *
 *
 *    the following api for smart_http and scheudler
 *
 *
 *********************************************************************/


MT_BOOL mutiQueue_init(MULTI_QUEUE_HANDLE** pp_multiqueue,
	                     int queue_size,int queue_total,QUEUE_MODE mode)

{
	int i=0;
	ERR_PRINTF("[%s] ====start start=====\n", __FUNCTION__);
	ERR_PRINTF("[%s] ====queue_size:%d=====\n", __FUNCTION__,queue_size);
	ERR_PRINTF("[%s] ====queue_total:%d=====\n", __FUNCTION__,queue_total);
	ERR_PRINTF("[%s] ====mode:%d=====\n", __FUNCTION__,mode);

	if(queue_total > MAX_QUEUE_NUM){
		ERR_PRINTF("[%s][ERROR][ERROR]==queue_total:%d\n", __FUNCTION__,queue_total);
		ERR_PRINTF("[%s][ERROR][ERROR]==MAX_QUEUE_NUM:%d\n", __FUNCTION__,MAX_QUEUE_NUM);
		return FALSE;
	}

	//dctx = Init_Download_Group(fetch_ts_segment_task_entry,NULL,num,p_task_priorty,stack_size);
	MULTI_QUEUE_HANDLE * p_multiqueue = (MULTI_QUEUE_HANDLE *)malloc(sizeof(MULTI_QUEUE_HANDLE));
	if(p_multiqueue){
		memset(p_multiqueue,0,sizeof(MULTI_QUEUE_HANDLE));
	}
	else{
		ERR_PRINTF("[%s] ====ERROR!!!====\n", __FUNCTION__);
		return FALSE;
	}


	for(i=0;i < queue_total;i++)
	{
		p_multiqueue->seg_queue[i]=creat_queue(queue_size);
		if(p_multiqueue->seg_queue[i])
		{
			p_multiqueue->seg_queue[i]->id=i;
			p_multiqueue->seg_queue[i]->mode = mode;
			p_multiqueue->seg_queue[i]->cur_range.end_pos = INVALID_SEEK_POS;
			p_multiqueue->seg_queue[i]->cur_range.start_pos = INVALID_SEEK_POS;
			p_multiqueue->seg_queue[i]->p_multiqueue = (void *)p_multiqueue;

		}
		else
		{
			ERR_PRINTF("[%s] ===ERROR ERROR=====\n", __FUNCTION__);
			ERR_PRINTF("[%s] ===seg_queue[i]:0x%x====\n", __FUNCTION__,p_multiqueue->seg_queue[i]);
			free(p_multiqueue);
			*pp_multiqueue = p_multiqueue = NULL;
			return FALSE;
		}
	}

	   p_multiqueue->total = queue_total;
	   p_multiqueue->baseJobID = 0;
       p_multiqueue->mode = mode;
	   p_multiqueue->cur_fake_job_id = 0;
	   p_multiqueue->seek_pending = FALSE;
	   p_multiqueue->seek_pos = INVALID_SEEK_POS;

	*pp_multiqueue = p_multiqueue;

	ERR_PRINTF("[%s] ====end end=====\n", __FUNCTION__);

	return TRUE;

}


void mutiQueue_deinit(MULTI_QUEUE_HANDLE* p_multiqueue)
{
	ERR_PRINTF("[%s] ====start start=====\n", __FUNCTION__);

	int i = 0;

	for(i=0;i<p_multiqueue->total;i++)
	{
		destroy_queue(p_multiqueue->seg_queue[i]);
	}

	free(p_multiqueue);

	ERR_PRINTF("[%s] ====end end=====\n", __FUNCTION__);
}

static DATA_SEGMENT_QUEUE * mutiQueue_find_dataready(MULTI_QUEUE_HANDLE* p_multiqueue)
{
	int i=0;
	int job_id = 0;
	int v_job_id = 0;
	DATA_SEGMENT_QUEUE * p_queue = NULL;

	if(p_multiqueue->mode == NORMAL_BUF_M)
	{
		job_id = p_multiqueue->seg_queue[0]->job_id + p_multiqueue->baseJobID + MAX_JOB_TOTAL;


		for(i=0;i < p_multiqueue->total; i++)
		{
			/*find the minimum job id*/
			if(p_multiqueue->seg_queue[i]->used_flag
	                       &&  p_multiqueue->seg_queue[i]->job_id != INVALID_JOB_ID//invalid value
				  &&  p_multiqueue->seg_queue[i]->job_id < job_id)
			{
				job_id = p_multiqueue->seg_queue[i]->job_id;
				p_queue = p_multiqueue->seg_queue[i];

			}
		}

	}
	else{//RING_BUF_M

		v_job_id = p_multiqueue->seg_queue[0]->v_job_id +  MAX_JOB_TOTAL;

		for(i=0;i < p_multiqueue->total; i++)
		{
			/*find the minimum job id*/
			if(p_multiqueue->seg_queue[i]->used_flag
	              &&  p_multiqueue->seg_queue[i]->v_job_id != INVALID_JOB_ID//invalid value
				  &&  p_multiqueue->seg_queue[i]->v_job_id < v_job_id)
			{
				v_job_id = p_multiqueue->seg_queue[i]->v_job_id;
				p_queue = p_multiqueue->seg_queue[i];

			}
		}
	}

	return p_queue;

}

int  mutiQueue_set_baseJobID(MULTI_QUEUE_HANDLE* p_multiqueue,int baseID)
{
     p_multiqueue->baseJobID = baseID;
	 return 0;
}


MT_BOOL mutiQueue_reconfig(MULTI_QUEUE_HANDLE* p_multiqueue,QUEUE_MODE mode)
{
	int i = 0;

       if(!p_multiqueue)
            return FALSE;

       if(mode == p_multiqueue->mode)//need not to change mode
            return TRUE;

	for(i=0;i < p_multiqueue->total;i++)
	{
		//p_multiqueue->seg_queue[i]=creat_queue(queue_size);
		if(p_multiqueue->seg_queue[i]){
			//p_multiqueue->seg_queue[i]->id=i;
			p_multiqueue->seg_queue[i]->mode = mode;
			p_multiqueue->seg_queue[i]->cur_range.end_pos = INVALID_SEEK_POS;
			p_multiqueue->seg_queue[i]->cur_range.start_pos = INVALID_SEEK_POS;
			p_multiqueue->seg_queue[i]->p_multiqueue = p_multiqueue;
		}
	}

	p_multiqueue->baseJobID = 0;
	p_multiqueue->cur_fake_job_id = 0;
	p_multiqueue->seek_pending = FALSE;
	p_multiqueue->seek_pos = INVALID_SEEK_POS;
	p_multiqueue->mode = mode;
	return TRUE;

}

int mutiQueue_start_seek(MULTI_QUEUE_HANDLE* p_multiqueue,int pos)
{

	if(p_multiqueue->seek_pending){

		ERR_PRINTF("[%s][WARNING]do nothing because mutiqueue is in seek_pending=====\n", __FUNCTION__);
		return -1;
	}

	int loopcnt = 0;
	p_multiqueue->seek_pending = TRUE;
	p_multiqueue->seek_pos = pos;

	ERR_PRINTF("[mtq]start seek_pos:%d\n",p_multiqueue->seek_pos);

	do{

		mtos_task_sleep(10);
		loopcnt++;
		if(loopcnt%20 == 0)
		{
			ERR_PRINTF("[mtq]====wait seek 1 second====\n");
		}

	}while(p_multiqueue->seek_pending);

	ERR_PRINTF("[mtq] end\n");

	return 0;

}

int mutiQueue_end_seek(MULTI_QUEUE_HANDLE* p_multiqueue)
{

	if(p_multiqueue->seek_pending == FALSE){

		OS_PRINTF("[%s][WARNING]do nothing because mutiqueue is not in seek pending=====\n", __FUNCTION__);
		return -1;
	}

	p_multiqueue->seek_pending = FALSE;

	return 0;

}
MT_BOOL mutiQueue_is_seek_pending(MULTI_QUEUE_HANDLE* p_multiqueue)
{
    return p_multiqueue->seek_pending;
}

/*********
*
*
*   smart_http use this method to read data
*
*
*
*
*
*
****/
int mutiQueue_read(MULTI_QUEUE_HANDLE* p_multiqueue, unsigned char *buf/*out*/, int size)
{
#define MAX_REDO_CNT    (3)
	int read_size = 0;
       int redo_cnt = 0;


	if(p_multiqueue->seek_pending){

		ERR_PRINTF("[%s][WARNING]do nothing because mutiqueue is in seek_pending=====\n", __FUNCTION__);
		return 0;
	}


	do
	{
		if (is_file_seq_exit()) {
			OS_PRINTF("[%s] %d exit exit!!!!!!!!! \n", __func__, __LINE__);
			return 0;
		}

		if(p_multiqueue->eos){
			OS_PRINTF("[%s][WARNNIG] line %d EOS EOS EOS!!!!\n", __FUNCTION__,__LINE__);
			return -1;
		}

		if(p_multiqueue->cur_queue == NULL){
			p_multiqueue->cur_queue = mutiQueue_find_dataready(p_multiqueue);
			if(p_multiqueue->cur_queue == NULL){
                            if(redo_cnt >= MAX_REDO_CNT)
                                return -1;

				mtos_task_sleep(10);
                redo_cnt++;
				ERR_PRINTF("no ready queue!\n");
				continue;
			}
		}

		//check queue is complete!!!!!

		read_size = queue_read(p_multiqueue->cur_queue,buf,size);
		if(read_size == 0)
		{
			mtos_task_sleep(10);
		}

		if(p_multiqueue->cur_queue->complete_job_flag
		 && p_multiqueue->cur_queue->read_pos == p_multiqueue->cur_queue->write_pos
		 &&p_multiqueue->cur_queue->len){

			//ERR_PRINTF("j:%d,q:%d is ok\n",
			//	p_multiqueue->cur_queue->job_id,p_multiqueue->cur_queue->id);

			p_multiqueue->cur_queue->used_flag = FALSE;
			p_multiqueue->cur_queue = NULL;


	   }

	}while(!read_size);

	//mtos_printk("%s %d exit \n", __func__, __LINE__);

	return read_size;

}

void mutiQueue_reset(MULTI_QUEUE_HANDLE* p_multiqueue)
{
    ERR_PRINTF("[%s] start ...\n", __func__);
    int i=0;
    if(!p_multiqueue)
        return;

    for(i=0;i < p_multiqueue->total;i++)
    {
        if(p_multiqueue->seg_queue[i])
            queue_reset(p_multiqueue->seg_queue[i]);
    }

    p_multiqueue->baseJobID = 0;
    p_multiqueue->cur_queue = NULL;
	p_multiqueue->cur_fake_job_id = 0;
    p_multiqueue->eos = FALSE;
    p_multiqueue->seek_pending = FALSE;
    p_multiqueue->seek_pos = INVALID_SEEK_POS;

	ERR_PRINTF("[%s] end ...\n", __func__);
    return;
}
void mutiQueue_clear_queues(MULTI_QUEUE_HANDLE* p_multiqueue)
{
    int i=0;
    ERR_PRINTF("%s coming\n", __func__);
    if(!p_multiqueue)
        return;

    for(i=0;i < p_multiqueue->total;i++)
    {
        if(p_multiqueue->seg_queue[i])
            queue_reset(p_multiqueue->seg_queue[i]);
    }

    p_multiqueue->cur_queue = NULL;
    p_multiqueue->cur_fake_job_id = 0;
    p_multiqueue->eos = FALSE;
    return;
}

DATA_SEGMENT_QUEUE*  mutiQueue_get_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue, int job_id)
{
    int i=0;
    for(i=0; i < p_multiqueue->total; i++)
    {
        if( (p_multiqueue->seg_queue[i]->job_id) == job_id)
        {
            return p_multiqueue->seg_queue[i];
        }
    }

	return NULL;
}
/*
 *  this fucn only is called by sheduler
 *
 *    scheduler call this function to get a free queue from mutiqueue for downloader
 *
 *  return a free queue
 *   if no available queue, return NULL
 */
DATA_SEGMENT_QUEUE * mutiQueue_take_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue)
{
	DATA_SEGMENT_QUEUE * p_queue = NULL;
	int i=0;
	for(i=0;i < p_multiqueue->total; i++)
	{
		p_queue = p_multiqueue->seg_queue[i];
		if(!p_queue->used_flag)
		{
			//p_queue[i]->job_id = -1;
			OS_PRINTF("[%s] jobid %d,len %d,rdpos %d,wrpos %d,queid %d\n",
			__FUNCTION__,p_queue->job_id,
			p_queue->len,p_queue->read_pos,
			p_queue->write_pos,p_queue->id);

			p_queue->used_flag = TRUE;
			p_queue->write_pos = 0;
			p_queue->read_pos = 0;
			p_queue->complete_job_flag = FALSE;
			p_queue->abort_flag = FALSE;
			p_queue->cur_range.end_pos = INVALID_SEEK_POS;
			p_queue->cur_range.start_pos = INVALID_SEEK_POS;

			p_queue->v_job_id = p_multiqueue->cur_fake_job_id+1;
			p_multiqueue->cur_fake_job_id = p_queue->v_job_id;

			return p_queue;
		}
	}
	return NULL;
}

int  mutiQueue_reset_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue,
	 DATA_SEGMENT_QUEUE * p_queue)
{

	if(p_multiqueue&&p_queue){

		queue_reset(p_queue);
	}

	return 0;
}

int  mutiQueue_reuse_one_queue(MULTI_QUEUE_HANDLE* p_multiqueue,
	 DATA_SEGMENT_QUEUE * p_queue)
{

	if(p_multiqueue&&p_queue){

		p_queue->read_pos = 0;
		p_queue->used_flag= TRUE;
	}

	return 0;
}

int  mutiQueue_get_queue_exist_jobs(MULTI_QUEUE_HANDLE* p_multiqueue,
	 JOB_IDS_ARRAY * p_array)
{

	int i = 0;
	int j=0;

	for(i=0;i<p_multiqueue->total;i++)
	{

		if(p_multiqueue->seg_queue[i]->used_flag){

			p_array->id[j] = p_multiqueue->seg_queue[i]->job_id;
			j++;

		}

	}

	p_array->totalNum = j;
	return 0;
}
MT_BOOL  mutiQueue_check_child_drained(MULTI_QUEUE_HANDLE* p_multiqueue,int job_id)
{
	int i=0;
	for(i=0; i < p_multiqueue->total; i++){

		if( (p_multiqueue->seg_queue[i]->job_id) == job_id){
			break;
		}
	}

	if(i >= p_multiqueue->total){
		ERR_PRINTF("[%s][ERROR] NOT  FIND THE JOBID:%d!!!!!\n",__func__,job_id);
		return FALSE;
	}
	else{
		OS_PRINTF("[%s][OK]find queue for job_id:%d\n",__func__,job_id);
	}

	DATA_SEGMENT_QUEUE*  p_queue = p_multiqueue->seg_queue[i];

	if(p_queue->mode == NORMAL_BUF_M  && p_queue->complete_job_flag){

		OS_PRINTF("[%s] write_pos:%d read_pos:%d abort_flag:%d!!!!!!\n",
				 __func__,p_queue->write_pos,p_queue->read_pos,p_queue->abort_flag);

		if(p_queue->write_pos == p_queue->read_pos)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if(p_queue->mode == RING_BUF_M)
	{
		return check_all_queues_drained(p_multiqueue);
	}
	else
	{
		OS_PRINTF("[%s] p_queue has es data,wait!!!!!!\n",__func__);
		return FALSE;
	}



}

MT_BOOL  mutiQueue_check_queue_exist(MULTI_QUEUE_HANDLE* p_multiqueue,int job_id)
{
	int i=0;
	for(i=0; i < p_multiqueue->total; i++){

		if( (p_multiqueue->seg_queue[i]->job_id) == job_id){
			break;
		}
	}

	if(i >= p_multiqueue->total){
		ERR_PRINTF("[%s][ERROR] NOT  FIND THE JOBID:%d!!!!!\n",__func__,job_id);
		return FALSE;
	}
	else{
		OS_PRINTF("[%s][OK]find queue for job_id:%d\n",__func__,job_id);
	}

	DATA_SEGMENT_QUEUE*  p_queue = p_multiqueue->seg_queue[i];

	if(p_queue->mode == NORMAL_BUF_M){

		OS_PRINTF("[%s] write_pos:%d read_pos:%d abort_flag:%d!!!!!!\n",
				 __func__,p_queue->write_pos,p_queue->read_pos,p_queue->abort_flag);

		if(p_queue->used_flag && p_queue->write_pos > p_queue->read_pos)//queue not read done
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if(p_queue->mode == RING_BUF_M)
	{
		return FALSE;
	}
	else
	{
		OS_PRINTF("[%s] p_queue has es data,wait!!!!!!\n",__func__);
		return FALSE;
	}



}
/*
*   only called by download excutor !!!
*
*   return 0: OK  -1:fail
*
*     downloader call this function to get the right write positon of queue
*
*/
int queue_query_wp( DATA_SEGMENT_QUEUE*  p_queue,WRITE_POS_INFO* p_info/*IN*/)
{
   if(p_info){

		memset(p_info,0,sizeof(WRITE_POS_INFO));

		if( (p_queue->head_addr + p_queue->write_pos) <=  p_queue->tail_addr)
		{
			p_info->w_start_addr = p_queue->head_addr + p_queue->write_pos;
			p_info->max_w_byte = p_queue->len - p_queue->write_pos;
			p_info->p_working_queue = p_queue;
			OS_PRINTF("[%s] this queue is 0x%x ,size is %d!!!!!\n",__func__,p_queue,p_info->max_w_byte);
			return 0;
		}
		else if( (p_queue->mode == RING_BUF_M) &&
			  (p_queue->head_addr + p_queue->write_pos) >  p_queue->tail_addr)
	    {
			OS_PRINTF("[%s] RING_BUF_M : selecet brother queue!!!!\n",__func__);
			MULTI_QUEUE_HANDLE * p_multiq = p_queue->p_multiqueue;
			DATA_SEGMENT_QUEUE*  p_newqueue = mutiQueue_take_one_queue(p_multiq);
			if(p_newqueue){
				p_info->w_start_addr = p_newqueue->head_addr + p_newqueue->write_pos;
				p_info->max_w_byte = p_newqueue->len - p_newqueue->write_pos;
				p_info->p_working_queue = p_newqueue;
				OS_PRINTF("[%s] this queue is 0x%x ,new queue addr %x,id %d,size is %d!!!!!\n",__func__,p_queue,
                                p_newqueue,p_newqueue->id,p_info->max_w_byte);
				return 0;
			}
			else{
				OS_PRINTF("[%s][ERROR][ERROR] not found available queue!!!!!\n",__func__);
				return -1;
			}

		}
		else
		{
			ERR_PRINTF("[%s][WARNING] NORMAL_BUF_M : no free space==!!!!\n",__func__);
			ERR_PRINTF("[%s][WARNING]=MODE:%d==!!!!job id %d\n",__func__,p_queue->mode,
                            p_queue->job_id);
			//OS_PRINTF("[%s][ERROR]=write_pos[%d] len[%d]==!!!!\n",__func__,p_queue->write_pos,p_queue->len);
			p_info->w_start_addr = NULL;
			p_info->max_w_byte = 0;
			return -1;
		}
   }

   return -1;

}


/*
*  this functiong is  only called by download excutor !!!
*
*
*  after finish writing some data to queue ,downloader should call the fuction
*     to update write position of queue
*
*/
int update_queue_writePosition(DATA_SEGMENT_QUEUE* p_queue,
                int inc_byte/*increase bytes*/,ulong p_working_queue)
{
	DATA_SEGMENT_QUEUE* tmp_queue = NULL;

   if(p_working_queue && p_queue != (DATA_SEGMENT_QUEUE*)p_working_queue)
   {
		tmp_queue = (DATA_SEGMENT_QUEUE*)p_working_queue;
   }
   else
   {
		tmp_queue =p_queue;
   }

   if( (tmp_queue->write_pos + inc_byte) <=  tmp_queue->len)
   {
		tmp_queue->write_pos += inc_byte;
              if(tmp_queue->mode == RING_BUF_M && tmp_queue->write_pos >= tmp_queue->len)
              {
                    tmp_queue->complete_job_flag = TRUE;
		      tmp_queue->abort_flag = FALSE;
              }
		return 0;
   }
   else
   {
		ERR_PRINTF("[%s][ERROR][ERROR] can't update write position==!!!!!!!!!!!!!!\n",__func__);
		ERR_PRINTF("[%s][ERROR][ERROR] write_pos[%d] inc_byte[%d] len[%d]!!!!!!!!!!!!!!\n",
			  __func__,tmp_queue->write_pos,inc_byte,tmp_queue->len);

		return -1;
   }

}

int set_queue_job_id(DATA_SEGMENT_QUEUE* p_queue,int job_id)
{
	return p_queue->job_id = job_id;
}

/*
*   this functiong is  only called by download excutor  !!!
*
*    download excutor notify the queue that it has successed or failed to downloaded ts data
*
*
*/
int  notify_queue_downloaderStatus(DATA_SEGMENT_QUEUE* p_queue,
      DL_STATUS status,ulong p_working_queue)
{

   DATA_SEGMENT_QUEUE* tmp_queue = NULL;

   if(p_working_queue && p_queue != (DATA_SEGMENT_QUEUE*)p_working_queue)
   {
		tmp_queue = (DATA_SEGMENT_QUEUE*)p_working_queue;
   }
   else
   {
		tmp_queue =p_queue;
   }

	if(tmp_queue)
	{
		if(status == DOWNLOAD_SUCCESS)
		{
			tmp_queue->complete_job_flag = TRUE;
			tmp_queue->abort_flag = FALSE;
			//ERR_PRINTF("[notify]j:%d,q:%d\n",tmp_queue->job_id,tmp_queue->id);

		}
		else{//DOWNLOAD_ABORT

			tmp_queue->complete_job_flag = TRUE;
			tmp_queue->abort_flag = TRUE;
			ERR_PRINTF("[notify][abort]j:%d],q:%d\n",tmp_queue->job_id,tmp_queue->id);
		}
	}

  return 0;
}

int  set_queue_range( DATA_SEGMENT_QUEUE*  p_queue,FILE_RANGE range)
{

	if(p_queue)
	{
		p_queue->cur_range.start_pos = range.start_pos;
		p_queue->cur_range.end_pos = range.end_pos;
		OS_PRINTF("[%s]=start_pos:%d  end_pos:%d====\n",__func__,range.start_pos,range.end_pos);
	}
  return 0;
}

/***********************************************************************************
*
*
*
*
*
*
*
**********************************************************************************/
static MT_BOOL check_all_queues_drained(MULTI_QUEUE_HANDLE * p_hdl)
{
	ERR_PRINTF("[%s]==========start start====\n",__func__);

	int i = 0;
	int not_drained_cnt = 0;
	DATA_SEGMENT_QUEUE * p_queue = NULL;
	MT_BOOL drained = TRUE;

	for(i=0;i<p_hdl->total;i++)
	{
		p_queue = p_hdl->seg_queue[i];

		if(p_queue && p_queue->used_flag)
		{

			if(p_queue->write_pos >= p_queue->len
				&& p_queue->len > 0
				&& p_queue->read_pos >= p_queue->write_pos)
			{
				drained = TRUE;
			}
			else if(p_queue->complete_job_flag
				 && p_queue->read_pos == p_queue->write_pos)
			{

				drained = TRUE;
			}
			else
			{

				drained = FALSE;

				ERR_PRINTF("[%s][WARNING]this queue[%d] not drained!!!!!!!!\n",__func__,i);
				ERR_PRINTF("[%s][WARNING]id:%d read:%d write:%d!!!!\n",__func__,i,p_queue->read_pos,p_queue->write_pos);
				ERR_PRINTF("[%s][WARNING]complete_job_flag:%d !!!\n",__func__,p_queue->complete_job_flag);

				break;


			}

		}

	}


	OS_PRINTF("[%s]======drained:%d====end end====\n",__func__,drained);
	return drained;

}

/************************************************************************************
*
*  check whether the queue has been drained
*    if the queue work in ring buffer mode , should check all queues have drained
*
**********************************************************************************/
MT_BOOL  check_queue_drained(DATA_SEGMENT_QUEUE* p_queue)
{
    if(!p_queue)
        return FALSE;

	if(p_queue->mode == NORMAL_BUF_M && p_queue->complete_job_flag){

		if(p_queue->write_pos == p_queue->read_pos)
		{
			return TRUE;
		}
		else
		{
			OS_PRINTF("[%s] write_pos:%d read_pos:%d abort_flag:%d!!!!!!\n",
				 __func__,p_queue->write_pos,p_queue->read_pos,p_queue->abort_flag);
			return FALSE;
		}
	}
	else if(p_queue->mode == RING_BUF_M){

		return check_all_queues_drained(p_queue->p_multiqueue);
	}
	else
	{
		//OS_PRINTF("[%s] p_queue has es data,wait!!!!!!\n",__func__);
		return FALSE;
	}
}

int set_multiqueue_eos_flag(DATA_SEGMENT_QUEUE* p_queue,MT_BOOL is_eos)
{
	if(p_queue && p_queue->p_multiqueue)
	{
		MULTI_QUEUE_HANDLE* p_multiqueue = p_queue->p_multiqueue;
		p_multiqueue->eos = is_eos;
	}
	else{
		ERR_PRINTF("[%s][ERROR]!!!p_queue:0x%x!!!\n",__func__,p_queue);
		ERR_PRINTF("[%s][ERROR]!!!p_multiqueue:0x%x!!!\n",__func__,p_queue->p_multiqueue);
	}
	return 0;
}

