/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <mach/hardware.h>

/* Unf headers */
#include "mt_error_mpi.h"
#include "mt_drv_mmz.h"
#include "mt_drv_stat.h"
#include "mt_drv_sys.h"
#include "mt_drv_proc.h"
#include "mt_module_debug.h"

/* Drv headers */
#include "mt_drv_ao.h"
#include "drv_ao_ioctl.h"
#include "drv_ao_ext.h"
#include "drv_ao_private.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */


static struct file_operations s_stDevFileOpts =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl   = AO_DRV_Ioctl,
    .open             = AO_DRV_Open,
    .release          = AO_DRV_Release,
};

static baseops_s s_stDrvOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = AO_DRV_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = AO_DRV_Resume,
};

static AO_REGISTER_PARAM_S s_stProcParam = {
    .pfnReadProc  = AO_DRV_ReadProc,
    .pfnWriteProc = AO_DRV_WriteProc,
};

/* the attribute struct of audio decode engine device */
static mt_device_s s_stAdeUmapDev;
static AO_PRIV_DATA_S ao_priv_data_info;

static __inline__ int  AO_DRV_RegisterDev(void)
{
    /*register aenc chn device*/
    snprintf(s_stAdeUmapDev.devfs_name, sizeof(s_stAdeUmapDev.devfs_name), UMAP_DEVNAME_AO);
    s_stAdeUmapDev.fops   = &s_stDevFileOpts;
    s_stAdeUmapDev.minor  = UMAP_MIN_MINOR_AO;
    s_stAdeUmapDev.owner  = THIS_MODULE;
    s_stAdeUmapDev.drvops = &s_stDrvOps;
    s_stAdeUmapDev.priv = &ao_priv_data_info;

    if (mt_drv_dev_register(&s_stAdeUmapDev) < 0)
    {
        MT_FATAL_AO("FATAL: vdec register device failed\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static __inline__ void AO_DRV_UnregisterDev(void)
{
    /*unregister aenc chn device*/
    mt_drv_dev_unregister(&s_stAdeUmapDev);
}


mt_s32 AO_DRV_ModInit(mt_void)
{
    int ret;

    ao_priv_data_info.audoutclk = clk_get(NULL, "audoutclk");
    ao_priv_data_info.audoutaxiclk = clk_get(NULL, "audoutaxiclk");
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutclk))
        clk_prepare_enable(ao_priv_data_info.audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutaxiclk))
        clk_prepare_enable(ao_priv_data_info.audoutaxiclk);

#ifndef MT_MCE_SUPPORT
    ret = AO_DRV_Init();
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_AO("Init ao drv fail!\n");
        return MT_FAILURE;
    }
#endif

    ret = AO_DRV_RegisterProc(&s_stProcParam);
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_AO("Reg proc fail!\n");
        return MT_FAILURE;
    }

    ret = AO_DRV_RegisterDev();
    if (MT_SUCCESS != ret)
    {
        AO_DRV_UnregisterProc();
        MT_FATAL_AO("Reg dev fail!\n");
        return MT_FAILURE;
    }

    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutclk))
        clk_disable_unprepare(ao_priv_data_info.audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutaxiclk))
        clk_disable_unprepare(ao_priv_data_info.audoutaxiclk);

    return MT_SUCCESS;
}

mt_void AO_DRV_ModExit(mt_void)
{
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutclk))
        clk_prepare_enable(ao_priv_data_info.audoutclk);
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutaxiclk))
        clk_prepare_enable(ao_priv_data_info.audoutaxiclk);

    AO_DRV_UnregisterDev();
    AO_DRV_UnregisterProc();

#ifndef MT_MCE_SUPPORT
    AO_DRV_Exit();

#endif

    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutclk)) {
        clk_disable_unprepare(ao_priv_data_info.audoutclk);
        clk_put(ao_priv_data_info.audoutclk);
    }
    if (!IS_ERR_OR_NULL(ao_priv_data_info.audoutaxiclk)) {
        clk_disable_unprepare(ao_priv_data_info.audoutaxiclk);
        clk_put(ao_priv_data_info.audoutaxiclk);
    }

    return;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
