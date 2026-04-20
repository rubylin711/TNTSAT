/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_VSS_INTF_H__
#define __DRV_VSS_INTF_H__
#include <linux/types.h>

int vss_base_init(void);
void vss_base_exit(void);

#ifdef CONFIG_MT_VSS_MB
extern int vss_mb_init(void);
extern void vss_mb_exit(void);
#else
static int vss_mb_init(void) {return 0;}
static void vss_mb_exit(void) {}
#endif

#endif	/*__DRV_VSS_INTF_H__*/
