/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mt_common.h"
#include "mt_see_misc.h"

void see_mcpu_mailbox_init(void)
{
    //Do nothing
}

int see_mb_take_cryptolock(void)
{
    mt_u32 val;

    /* get lock before access shared memory */
    while (1)
    {
        mt_sys_read_register(MECPU_MB_MEMLOCK_REG(T_CRYPTOLOCK_SPINLOCK) & 0x1FFFFFFF, &val);
        if (1  != (val & 0x1))
            break;
        MT_USLEEP(1000);
    }

    return 0;
}

int see_mb_give_cryptolock(void)
{
    mt_sys_write_register(MECPU_MB_MEMLOCK_REG(T_CRYPTOLOCK_SPINLOCK) & 0x1FFFFFFF, 1);
    return 0;
}

#if 0
int see_mb_take_printflock(void)
{
    mt_u32 val;

    /* get lock before access shared memory */
    while (1)
    {
        mt_sys_read_register(MECPU_MB_MEMLOCK_REG(T_MSGLOCK_SPINLOCK) & 0x1FFFFFFF, &val);
        if (1  != (val & 0x1))
            break;
        MT_USLEEP(1000);
    }

    return 0;
}

int see_mb_give_printflock(void)
{
    mt_sys_write_register(MECPU_MB_MEMLOCK_REG(T_MSGLOCK_SPINLOCK) & 0x1FFFFFFF, 1);
    return 0;
}

unsigned int see_check_seccpu_ready(void)
{
    mt_u32 val;

    mt_sys_read_register(E2MCPU_MB_INIT & 0x1FFFFFFF, &val);

    return (val == CPU_READY);
}

/*
 * once this function is called, the peer cpu will know it's ready
 * this is the only way the peer cpu know it's ready
 * so, this function must be called only after all the necessay work has been done by the local cpu
 */
unsigned int see_set_cpu_ready(void)
{
    mt_sys_write_register(M2ECPU_MB_INIT & 0x1FFFFFFF, CPU_READY);

    return 0;
}

unsigned int see_set_cpu_unready(void)
{
    mt_sys_write_register(M2ECPU_MB_INIT & 0x1FFFFFFF, CPU_NOT_READY);

    return 0;
}
#endif

