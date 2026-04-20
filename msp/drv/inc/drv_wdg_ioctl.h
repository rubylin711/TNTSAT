/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_WDG_IOCTL_H__
#define __DRV_WDG_IOCTL_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#include "mt_unf_wdg.h"
#include "mt_drv_wdg.h"

typedef struct _tag_wdg_timeout_s
{
	mt_u32	u32WdgIndex;
	mt_s32	s32Timeout;
}wdg_timeout_s;

typedef struct _tagwdg_option_s
{
	mt_u32	u32WdgIndex;
	mt_s32	s32Option;
}wdg_option_s;

#define WATCHDOG_IOCTL_BASE 'W'

#define WDIOC_SET_OPTIONS _IOR(WATCHDOG_IOCTL_BASE, 4, wdg_option_s)
#define WDIOC_KEEP_ALIVE _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SET_TIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 6, wdg_timeout_s)
#define WDIOC_GET_TIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 7, wdg_timeout_s)
#define WDIOF_UNKNOWN -1  /* Unknown flag error */
#define WDIOS_UNKNOWN -1  /* Unknown status error */


#define WDIOS_DISABLECARD 0x0001  /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD 0x0002  /* Turn on the watchdog timer */
#define WDIOS_RESET_BOARD 0x0008  /* reset the board */

#define WDIOS_ENABLECARD_IRQ 0x0010  /* Turn on the watchdog irq */
#define WDIOS_DISABLECARD_IRQ 0x0020  /* Turn on the watchdog irq */


#endif  /* ifndef __DRV_WDG_IOCTL_H__ */
