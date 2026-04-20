/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/interrupt.h>
#include <linux/clk.h>

#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"

#include "mt_module.h"
#include "mt_drv_mmz.h"
#include "mt_drv_sys.h"
#include "mt_drv_module.h"
#include "mt_drv_mem.h"
#include "mt_error_mpi.h"

#include "mt_drv_ai.h"
#include "drv_ai_private.h"


#define AI_PMOC

/**************************** global variables ****************************/
static mt_device_s g_stAIDev;
static atomic_t g_AIModInitFlag = ATOMIC_INIT(0);

static struct file_operations AI_DRV_Fops =
{
    .owner   = THIS_MODULE,
    .open    = AI_DRV_Open,
    .unlocked_ioctl   = AI_DRV_Ioctl,
    .release = AI_DRV_Release,
};

static baseops_s ai_drvops =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = AI_DRV_Suspend,               /*TODO  AI_Suspend*/
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = AI_DRV_Resume,               /*TODO  AI_Resume*/
};

static AI_REGISTER_PARAM_S s_stProcParam = {
    .pfnReadProc  = AI_DRV_ReadProc,
    .pfnWriteProc = AI_DRV_WriteProc,
};

static AI_PRIV_DATA_S ai_priv_data_info;

static mt_s32  AIRegisterDevice(mt_void)
{
    snprintf(g_stAIDev.devfs_name, sizeof(g_stAIDev.devfs_name), UMAP_DEVNAME_AI);
    g_stAIDev.fops  = &AI_DRV_Fops;
    g_stAIDev.minor = UMAP_MIN_MINOR_AI;
#ifdef AI_PMOC
    g_stAIDev.owner  = THIS_MODULE;
    g_stAIDev.drvops = &ai_drvops;
#endif
    g_stAIDev.priv = &ai_priv_data_info;

    if(mt_drv_dev_register(&g_stAIDev) < 0)
    {
        MT_FATAL_AI("Unable to register ai dev\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 AIUnregisterDevice(mt_void)
{    
    mt_drv_dev_unregister(&g_stAIDev);
    return MT_SUCCESS;
}


mt_s32 AI_DRV_ModInit(mt_void)
{
    mt_s32 Ret;

    ai_priv_data_info.audinclk = clk_get(NULL, "audinclk");
    ai_priv_data_info.audinaxiclk = clk_get(NULL, "audinaxiclk");
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinclk)))
        clk_prepare_enable(ai_priv_data_info.audinclk);
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinaxiclk)))
        clk_prepare_enable(ai_priv_data_info.audinaxiclk);
    
    Ret = AI_DRV_Init();
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AI("Init ai drv fail!\n");
        return MT_FAILURE;
    }

    Ret = AI_DRV_RegisterProc(&s_stProcParam);
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_AI("Reg proc fail!\n");
        return MT_FAILURE;
    }
    
#ifdef MT_AI_PROC_SUPPORT
    mt_proc_entry_t *item;
#endif

    Ret = AIRegisterDevice();
    if(MT_SUCCESS != Ret)
    {
        MT_FATAL_AI("Unable to register ai dev\n");
        return MT_FAILURE;
    }

#ifdef MT_AI_PROC_SUPPORT
    item = mt_drv_proc_add_module("ai", AI_DRV_ProcRead, NULL);
    if (!item)
    {
        MT_ERR_AI("add proc ai failed\n");
    }
#endif
    atomic_inc(&g_AIModInitFlag);

    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinclk)))
        clk_disable_unprepare(ai_priv_data_info.audinclk);
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinaxiclk)))
        clk_disable_unprepare(ai_priv_data_info.audinaxiclk);

    return MT_SUCCESS;
}


mt_void AI_DRV_ModExit(mt_void)
{
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinclk)))
        clk_prepare_enable(ai_priv_data_info.audinclk);
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinaxiclk)))
        clk_prepare_enable(ai_priv_data_info.audinaxiclk);

    AIUnregisterDevice();
    
    AI_DRV_Exit();
    
    AI_DRV_UnregisterProc();
    atomic_dec(&g_AIModInitFlag);

    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinclk))) {
        clk_disable_unprepare(ai_priv_data_info.audinclk);
        clk_put(ai_priv_data_info.audinclk);
    }
    if (!(IS_ERR_OR_NULL(ai_priv_data_info.audinaxiclk))) {
        clk_disable_unprepare(ai_priv_data_info.audinaxiclk);
        clk_put(ai_priv_data_info.audinaxiclk);
    }
}


