/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __ADEC_BUF_K_H__
#define __ADEC_BUF_K_H__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/semaphore.h>		// semaphore
#include <linux/wait.h>				// wait_queue_head_t
#include <linux/sched.h>
#include "adec/adec_api.h"

int adec_es_write(adec_es_buf_t* es_buf,  phys_addr_t data, int size, u32 pts, u32 pts_valid);
int adec_es_buf_data_remain(adec_es_buf_t* es_buf);
int adec_es_buf_clear(adec_es_buf_t* es_buf);

#endif

