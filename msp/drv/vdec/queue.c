/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2017, Montage Technology Co., Ltd.
 *
 * File Name      : queue.c
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
#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/kernel.h>
#else
#include <string.h>
#include <stdbool.h>
#include "mt_type.h"
#endif
#include "mt_type.h"
#include "mt_module_debug.h"
#include "queue.h"
#ifdef __KERNEL__
#include "mt_drv_mmz.h"
#include "mt_drv_ampshm.h"
#endif

#ifdef __KERNEL__
#define QUE_ERROR				MT_ERR_VDEC
#else
#define QUE_ERROR				AV_PRINTF
#endif

/* mem type for queue */
//#define QUE_USE_MMZ
#define QUE_USE_AMPSHM
//#define QUE_USE_KMEM
//#define QUE_USE_VMEM

#define QUE_ALIGN_SIZE			32

#ifdef __KERNEL__
#ifdef QUE_USE_MMZ
static void *_que_malloc(const char *name, unsigned int size)
{
	mmz_buffer_s mmz_buf;
	mt_s32 ret;

	ret = mt_drv_mmz_alloc(name, NULL, size, QUE_ALIGN_SIZE, &mmz_buf);

	if (ret == MT_SUCCESS)
	{
		ret = mt_drv_mmz_map(&mmz_buf);
		if (ret == MT_SUCCESS)
		{
			return (void*)mmz_buf.startVirAddr;
		}
		else
		{
			mt_drv_mmz_release(&mmz_buf);
			QUE_ERROR("mt_drv_mmz_map failed!\n");
		}
	}
	else
	{
		QUE_ERROR("mt_drv_mmz_alloc failed!\n");
	}

	return NULL;
}

static void _que_free(struct queue_t *que)
{
	mmz_buffer_s mmz_buf;

	mmz_buf.startVirAddr = que;
	mt_drv_mmz_unmap(&mmz_buf);
	mt_drv_mmz_release(&mmz_buf);
}
#elif defined(QUE_USE_AMPSHM)
static void *_que_malloc(const char *name, unsigned int size)
{
	int fd;
	int ret;

	fd = mt_ampshm_open(name, 0, 0);
	if (fd < 0)
	{
		QUE_ERROR("mt_ampshm_open failed!\n");
		return NULL;
	}

	ret = mt_ampshm_ftruncate(fd, size);
	if (ret != 0)
	{
		QUE_ERROR("mt_ampshm_ftruncate failed!\n");
		mt_ampshm_unlink(name);
		return NULL;
	}

	return mt_ampshm_mmap(NULL, size, 0, 0, fd, 0);
}

static void _que_free(struct queue_t *que)
{
	unsigned int size = 0;

	size = sizeof(struct queue_t) + (que->item_size * que->item_count);

	mt_ampshm_munmap(que, size);

	mt_ampshm_unlink(que->name);
}
#elif defined(QUE_USE_KMEM)
//kmalloc
static void *_que_malloc(const char *name, unsigned int size)
{
	return kmalloc(size, GFP_KERNEL);
}

static void _que_free(struct queue_t *que)
{
	kfree((void*)que);
}
#elif defined(QUE_USE_VMEM)
//vmalloc
static void *_que_malloc(const char *name, unsigned int size)
{
	return vmalloc(size);
}

static void _que_free(struct queue_t *que)
{
	vfree((void*)que);
}
#else
#error "pls select a mem type in queue.c!"
#endif

struct queue_t *queue_allocate(const char *name, unsigned int item_size, unsigned int item_count)
{
	unsigned int size = 0;
	struct queue_t *que = NULL;

	MT_ASSERT(name != NULL);
	MT_ASSERT(item_size > 0);
	MT_ASSERT(item_count > 0);

	size = sizeof(struct queue_t) + (item_size * item_count);

	que = (struct queue_t *)_que_malloc(name, size);
	if (que != NULL)
	{
		memset(que, 0, size);
		strlcpy(que->name, name, MAX_QUEUE_NAME_LEN);
		que->item_size = item_size;
		que->item_count = item_count;
	}

	return que;
}

void queue_free(struct queue_t *que)
{
	CHECK_QUE(que);

	_que_free(que);
}
#endif

int queue_enqueue(struct queue_t *que, void *item, unsigned it_sz)
{
	void *ptr = NULL;

	CHECK_QUE(que);
	MT_ASSERT(item != NULL);
	MT_ASSERT(it_sz == que->item_size);

	if (!queue_full(que))
	{
		ptr = que->item + que->wr_ptr * it_sz;
		if (ptr != item)
			memcpy(ptr, item, it_sz);
		que->wr_ptr = (que->wr_ptr + 1) % que->item_count;
		return 0;
	}
	else
	{
		QUE_ERROR("que is full!\n");
		return -1;
	}
}

void *queue_dequeue(struct queue_t *que)
{
	void *item = NULL;

	CHECK_QUE(que);

	if (!queue_empty(que))
	{
		item = que->item + que->rd_ptr * que->item_size;
		que->rd_ptr = (que->rd_ptr + 1) % que->item_count;
		return (item);
	}
	else
	{
		//QUE_ERROR("%s: que is empty!\n",__FUNCTION__);
		return NULL;
	}
}

