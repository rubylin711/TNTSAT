/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_WDG_H__
#define __DRV_WDG_H__

#ifdef __KERNEL__

#ifndef WATCHDOG_NOWAYOUT
#ifdef CONFIG_WATCHDOG_NOWAYOUT
#define WATCHDOG_NOWAYOUT	1
#else
#define WATCHDOG_NOWAYOUT	0
#endif
#endif

#endif	/* __KERNEL__ */


#endif


