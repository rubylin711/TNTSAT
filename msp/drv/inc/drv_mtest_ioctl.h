/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_MTEST_IOCTL_H__
#define __DRV_MTEST_IOCTL_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#include "mt_type.h"



typedef struct _tagmtest_option_s
{
	mt_u32	phyaddr;
	mt_u32	offset;
	mt_u32	pattern;
	mt_s32	len;
}mtest_option_s;

#define MTEST_IOCTL_BASE 'T'

#define MTEST_SET_OPTIONS _IOWR(MTEST_IOCTL_BASE, 4, mtest_option_s)
#define MTEST_KEEP_ALIVE _IOR(MTEST_IOCTL_BASE, 5, int)
#define MTEST_SET_CHECK_PATTERN _IOWR(MTEST_IOCTL_BASE, 6, mtest_option_s)
#define MTEST_GET_TIMEOUT _IOR(MTEST_IOCTL_BASE, 7, mtest_option_s)
#define WDIOF_UNKNOWN -1  /* Unknown flag error */
#define WDIOS_UNKNOWN -1  /* Unknown status error */

#endif  /* ifndef __DRV_WDG_IOCTL_H__ */
