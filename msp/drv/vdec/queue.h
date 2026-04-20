/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : queue.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2017/12/15
 * Description    : MT Queue(FIFO)
 * History        :
 * 1.Date         : 2017/12/15
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __MT_QUEUE_H__
#define __MT_QUEUE_H__

#ifndef __KERNEL__
#include "sys_define.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_QUEUE_NAME_LEN			8

#define CHECK_QUE(que)				do{MT_ASSERT(que!=NULL);}while(0)

/** queue definition */
struct queue_t
{
	char name[MAX_QUEUE_NAME_LEN];	//queue name
	unsigned int reserved[2];

	unsigned int rd_ptr;			//read pointer
	unsigned int wr_ptr;			//write pointer

	unsigned int item_size;			//each item's size in byte
	unsigned int item_count;		//total item count
	unsigned char item[0];			//item array pointer
};

//----------------------------------------------------------------------------//

#ifdef __KERNEL__
struct queue_t *queue_allocate(const char *name, unsigned int item_size, unsigned int item_count);
void queue_free(struct queue_t *ptr);
#endif

static __inline__ struct queue_t *queue_construct(void *ptr)
{
	CHECK_QUE(ptr);
	return (struct queue_t *)ptr;
}

static __inline__ void queue_destruct(struct queue_t *que)
{
	CHECK_QUE(que);
	//do nothing
}

//item count could read
static __inline__ unsigned int queue_size(struct queue_t *que)
{
	CHECK_QUE(que);
	if (que->wr_ptr >= que->rd_ptr)
		return (que->wr_ptr - que->rd_ptr);
	else
		return (que->wr_ptr + que->item_count - que->rd_ptr);
}

//get read pointer
static __inline__ unsigned int queue_read_ptr(struct queue_t *que)
{
	CHECK_QUE(que);
	return que->rd_ptr;
}

//set read pointer
static __inline__ void queue_set_read_ptr(struct queue_t *que, unsigned int rd_offset)
{
	CHECK_QUE(que);
	que->rd_ptr = rd_offset;
}

//get write pointer
static __inline__ unsigned int queue_write_ptr(struct queue_t *que)
{
	CHECK_QUE(que);
	return que->wr_ptr;
}

//set write pointer
static __inline__ void queue_set_write_ptr(struct queue_t *que, unsigned int wr_offset)
{
	CHECK_QUE(que);
	que->wr_ptr = wr_offset;
}

//write pointer address
static __inline__ void *queue_write_ptr_addr(struct queue_t *que)
{
	CHECK_QUE(que);
	return (void*)(que->item + que->wr_ptr * que->item_size);
}

//reset queue
static __inline__ void queue_reset(struct queue_t *que)
{
	CHECK_QUE(que);
	que->wr_ptr = 0;
	que->rd_ptr = 0;
	return;
}

//queue is full can not write anly more
static __inline__ bool queue_full(struct queue_t *que)
{
	CHECK_QUE(que);
	return (((que->wr_ptr+1)%que->item_count) == que->rd_ptr);
}

//queue is empty no item could read
static __inline__ bool queue_empty(struct queue_t *que)
{
	CHECK_QUE(que);
	return (que->rd_ptr == que->wr_ptr);
}

int queue_enqueue(struct queue_t *que, void *item, unsigned it_sz);
void *queue_dequeue(struct queue_t *que);

#ifdef __cplusplus
}
#endif

#endif //__MT_QUEUE_H__

