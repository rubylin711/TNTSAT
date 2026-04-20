/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

// A file of this name is needed on Windows
#ifndef _LZ_LINUX_
#include "sys_define.h"
#include "sys_types.h"
#include "mtos_printk.h"
#include "mtos_task.h"
#else
#define OS_PRINTF printf
#define mtos_task_sleep  usleep
#endif
 
