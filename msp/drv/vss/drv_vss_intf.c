/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <mt_mach/symphony_regs.h>

#include "mt_drv_dev.h"
#include "mt_debug.h"
#include "drv_vss_intf.h"

int vss_setup(void)
{
    vss_base_init();
    vss_mb_init();

    MT_PRINT("vss init success\n");
    return MT_SUCCESS;
}

void vss_cleanup(void)
{
    vss_base_exit();
    vss_mb_exit();

    MT_PRINT("vss cleanup success\n");
}
