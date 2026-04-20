/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/atomic.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/clk.h>

#include "mt_module_debug.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "drv_sync_priv.h"
#include "drv_sync.h"
#include "drv_sync_intf.h"
#include "drv_sync_ext.h"
#include "mt_osal.h"
#include "mt_debug.h"
#include "drv_sync.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

extern mt_s32       SYNC_DRV_Init(mt_void);
extern mt_void      SYNC_DRV_Exit(mt_void);

static mt_device_s g_SyncRegisterData;

mt_u8 *g_pSyncStatusString[4] = {
    "STOP",
    "PLAY",
    "TPLAY",
    "PAUSE",
};

mt_u8 *g_pSyncRefString[5] = {
    "NONE",
    "AUDIO",
    "VIDEO",
    "PCR",
    "SCR",
};

mt_u8 *g_pSyncChnString[5] = {
    "AUD",
    "VID",
    "PCR",
    "SCR",
    "EXT",
};

extern avsync_priv_t  *g_p_avsync_private;

static mt_s32 SYNC_ProcRead(struct seq_file *p, mt_void *v)
{

	avsync_dump_state_proc(p);

    return MT_SUCCESS;
}

mt_s32 SYNC_ProcParsePara(mt_char *pProcPara,mt_char **ppItem,mt_char **ppValue)
{
    mt_char *pChar = MT_NULL;
    mt_char *pItem,*pValue;

    pChar = strchr(pProcPara,'=');
    if (MT_NULL == pChar)
    {
        return MT_FAILURE; /* Not Found '=' */
    }

    pItem = pProcPara;
    pValue = pChar + 1;
    *pChar = '\0';

    /* remove blank bytes from item tail */
    pChar = pItem;
    while(*pChar != ' ' && *pChar != '\0')
    {
        pChar++;
    }
    *pChar = '\0';

    /* remove blank bytes from value head */
    while(*pValue == ' ')
    {
        pValue++;
    }

    *ppItem = pItem;
    *ppValue = pValue;
    return MT_SUCCESS;
}

mt_void SYNC_ProcPrintHelp(mt_void)
{
    mt_drv_proc_echohelp("\necho Help > /proc/msp/syncxx to get help info\n");
    mt_drv_proc_echohelp("\necho SyncRef = none|audio|video > /proc/msp/syncxx\n");
    mt_drv_proc_echohelp("     SyncRef  define:\n");
    mt_drv_proc_echohelp("     none : av not adjust, av play free\n");
    mt_drv_proc_echohelp("     audio: auido not adjust, video adjusted by apts\n");
    mt_drv_proc_echohelp("     video: video not adjust, auido adjusted by vpts\n");
    mt_drv_proc_echohelp("     example: echo SyncRef = audio > /proc/msp/sync00\n");
    mt_drv_proc_echohelp("     SyncRef must be set\n");
    mt_drv_proc_echohelp("\necho SyncStart.bSmoothPlay = true|false > /proc/msp/syncxx\n");
    mt_drv_proc_echohelp("     true : video play, not wait av synced\n");
    mt_drv_proc_echohelp("     false: video not play until av synced\n");
    mt_drv_proc_echohelp("     example: echo SyncStart.bSmoothPlay = false > /proc/msp/sync00\n");
    mt_drv_proc_echohelp("\necho LogLevel = xxx > /proc/msp/syncxx\n");
    mt_drv_proc_echohelp("     LogLevel  define:\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_FRMAE_INFO    (0x00000001)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_FRMAE_INFO    (0x00000002)\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_PTS_INFO      (0x00000004)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_PTS_INFO      (0x00000008)\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_SYNCED_INFO   (0x00000010)\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_PAUSE_INFO    (0x00000020)\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_SKIP_INFO     (0x00000040)\n");
    mt_drv_proc_echohelp("     SYNC_VIDEO_FREE_INFO     (0x00000080)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_SYNCED_INFO   (0x00000100)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_PAUSE_INFO    (0x00000200)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_SKIP_INFO     (0x00000400)\n");
    mt_drv_proc_echohelp("     SYNC_AUDIO_FREE_INFO     (0x00000800)\n");
    mt_drv_proc_echohelp("     example: echo LogLevel = 0x0c > /proc/msp/sync00, av pts will be printed out\n");

}

extern avsync_priv_t                 g_avsync_private;

static mt_s32 SYNC_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)

{
    SYNC_S            *pSync;
    struct seq_file   *s = file->private_data;
    mt_proc_entry_t  *pProcItem = s->private;
    mt_u32            SyncId;
    mt_char           ProcPara[64]={0};
    mt_char           *pItem,*pValue;
    mt_s32            Ret;

    if (copy_from_user(ProcPara, buf, count))
    {
        return -EFAULT;
    }

    ProcPara[63] = '\0';

    Ret = SYNC_ProcParsePara(ProcPara,&pItem,&pValue);
    if (MT_SUCCESS != Ret)
    {
        SYNC_ProcPrintHelp();
        return count;//-EFAULT;
    }

    SyncId = (pProcItem->entry_name[4] - '0')*10 + (pProcItem->entry_name[5] - '0');

    pSync = SYNC_getInfoPtr(SyncId);
    if(pSync == NULL){
        MT_PRINT("inst %d not exist \n",SyncId);
        return MT_SUCCESS;
    }
    /* Don't use strlen("xxx")+1, SYNC_ProcParsePara add '\n' to every cmd */
    if (!mt_osal_strncmp(pItem, "SyncPrint", strlen("SyncPrint")))
    {
        pSync->bPrint = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncRef", strlen("SyncRef")))
    {
        if (0 == mt_osal_strncmp(pValue,"none", strlen("none")))
        {
            pSync->SyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
            SYNC_reference_config(pSync);
        }
        else if (0 == mt_osal_strncmp(pValue,"audio", strlen("audio")))
        {
            pSync->SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
            SYNC_reference_config(pSync);
        }
        else if (0 == mt_osal_strncmp(pValue,"video", strlen("video")))
        {
            pSync->SyncAttr.enSyncRef = MT_UNF_SYNC_REF_VIDEO;
            SYNC_reference_config(pSync);
        }
        else if (0 == mt_osal_strncmp(pValue,"pcr", strlen("pcr")))
        {
            pSync->SyncAttr.enSyncRef = MT_UNF_SYNC_REF_PCR;
            SYNC_reference_config(pSync);
        }
        else if (0 == mt_osal_strncmp(pValue,"scr", strlen("scr")))
        {
            /*
            not support scr sync ref
            */
            mt_drv_proc_echohelp("Input param error, please read help\n");
            SYNC_ProcPrintHelp();
        }
        else
        {
            mt_drv_proc_echohelp("Input param error, please read help\n");
            SYNC_ProcPrintHelp();
        }
    }
    else if (0 == mt_osal_strncmp(pItem,"SyncStart.VidPlusTime", strlen("SyncStart.VidPlusTime")))
    {
        pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem,"SyncStart.VidNegativeTime", strlen("SyncStart.VidNegativeTime")))
    {
        pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncStart.bSmoothPlay", strlen("SyncStart.bSmoothPlay")))
    {
        if (0 == mt_osal_strncmp(pValue,"true", strlen("true")))
        {
            pSync->SyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
            avsync_set_slow_sync_policy(MT_TRUE);
        }
        else if(0 == mt_osal_strncmp(pValue,"false", strlen("false")))
        {
            pSync->SyncAttr.stSyncStartRegion.bSmoothPlay = MT_FALSE;
            avsync_set_slow_sync_policy(MT_FALSE);
        }
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncNovel.VidPlusTime", strlen("SyncNovel.VidPlusTime")))
    {
        pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncNovel.VidNegativeTime", strlen("SyncNovel.VidNegativeTime")))
    {
        pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncNovel.bSmoothPlay", strlen("SyncNovel.bSmoothPlay")))
    {
        if (0 == mt_osal_strncmp(pValue,"true", strlen("true")))
        {
            pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay = MT_TRUE;
        }
        else if(0 == mt_osal_strncmp(pValue,"false", strlen("false")))
        {
            pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay = MT_FALSE;
        }
    }
    else if (0 == mt_osal_strncmp(pItem, "PreSyncTimeoutMs", strlen("PreSyncTimeoutMs")))
    {
        pSync->SyncAttr.u32PreSyncTimeoutMs = simple_strtol(pValue, NULL, 10);
    }
    else if (0 == mt_osal_strncmp(pItem, "bQuickOutput", strlen("bQuickOutput")))
    {
        if (0 == mt_osal_strncmp(pValue,"true", strlen("true")))
        {
            pSync->SyncAttr.bQuickOutput = MT_TRUE;
        }
        else if(0 == mt_osal_strncmp(pValue,"false", strlen("false")))
        {
            pSync->SyncAttr.bQuickOutput = MT_FALSE;
        }
    }
    else if (0 == mt_osal_strncmp(pItem, "LogLevel", strlen("LogLevel")))
    {
        if (g_p_avsync_private)
        {
            g_p_avsync_private->avsync_loglevel = simple_strtol(pValue, NULL, 0);
        }
        else
        {
            MT_ERR_SYNC("g_p_avsync_private null\n");
        }
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncSkipThrd", strlen("SyncSkipThrd")))
    {
        if (g_p_avsync_private)
        {
            g_p_avsync_private->avsync_cfg_info.avsync_skip_threhold = (simple_strtol(pValue, NULL, 0))*45;
        }
        else
        {
            MT_ERR_SYNC("g_p_avsync_private null\n");
        }
    }
    else if (0 == mt_osal_strncmp(pItem, "SyncPauseThrd", strlen("SyncPauseThrd")))
    {
        if (g_p_avsync_private)
        {
            g_p_avsync_private->avsync_cfg_info.avsync_pause_threhold = (simple_strtol(pValue, NULL, 0))*45;
        }
        else
        {
            MT_ERR_SYNC("g_p_avsync_private null\n");
        }
    }
    else
    {
        mt_drv_proc_echohelp("Input param error, please read help\n");
        SYNC_ProcPrintHelp();
        return -EFAULT;
    }

    return count;
}


static long SYNC_DRV_Ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    mt_s32 Ret;

    Ret = mt_drv_usercopy(ffile->f_path.dentry->d_inode, ffile, cmd, arg, SYNC_Ioctl);

    return Ret;
}

static struct file_operations SYNC_FOPS =
{
    .owner          = THIS_MODULE,
    .open           = SYNC_DRV_Open,
    .unlocked_ioctl = SYNC_DRV_Ioctl,
    .release        = SYNC_DRV_Close,
};


static baseops_s SYNC_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = SYNC_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = SYNC_Resume,
};


static SYNC_REGISTER_PARAM_S SYNC_ProcPara = {
    .rdproc = SYNC_ProcRead,
    .wtproc = SYNC_ProcWrite,
};

static SYNC_Priv_Data_S sync_priv_data_info;


mt_s32 __init SYNC_DRV_ModInit(mt_void)
{
    mt_s32  Ret;

    MT_INFO_SYNC("SYNC_DRV_ModInit: start \n");

    sync_priv_data_info.hdclk = clk_get(NULL, "hdclk");
    sync_priv_data_info.sdclk_27m = clk_get(NULL, "sdclk_27m");
    sync_priv_data_info.tsiclk = clk_get(NULL, "tsiclk");
    if (!IS_ERR_OR_NULL(sync_priv_data_info.hdclk))
        clk_prepare_enable(sync_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.sdclk_27m))
        clk_prepare_enable(sync_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.tsiclk))
        clk_prepare_enable(sync_priv_data_info.tsiclk);

#ifndef MT_MCE_SUPPORT
    Ret = SYNC_DRV_Init();
    if(MT_SUCCESS != Ret)
    {
        MT_FATAL_SYNC("register SYNC Intf failed.\n");
        return MT_FAILURE;
    }
#endif

    Ret = SYNC_IntfRegister(&SYNC_ProcPara);
    if (Ret != MT_SUCCESS)
    {
        MT_FATAL_SYNC("register SYNC Intf failed.\n");
        return MT_FAILURE;
    }

    mt_osal_snprintf(g_SyncRegisterData.devfs_name, sizeof(g_SyncRegisterData.devfs_name), UMAP_DEVNAME_SYNC);
    mt_osal_snprintf(g_SyncRegisterData.devfs_name, sizeof(g_SyncRegisterData.devfs_name), UMAP_DEVNAME_SYNC);
    g_SyncRegisterData.fops = &SYNC_FOPS;
    g_SyncRegisterData.minor = UMAP_MIN_MINOR_SYNC;
    g_SyncRegisterData.owner  = THIS_MODULE;
    g_SyncRegisterData.drvops = &SYNC_DRVOPS;
    g_SyncRegisterData.priv = (void *)&sync_priv_data_info;

    if (mt_drv_dev_register(&g_SyncRegisterData) < 0)
    {
        MT_FATAL_SYNC("register SYNC failed.\n");
        return MT_FAILURE;
    }

    if (!IS_ERR_OR_NULL(sync_priv_data_info.hdclk))
        clk_disable_unprepare(sync_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.sdclk_27m))
        clk_disable_unprepare(sync_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.tsiclk))
        clk_disable_unprepare(sync_priv_data_info.tsiclk);

    MT_INFO_SYNC("SYNC_DRV_ModInit: end \n");

    return  0;
}

mt_void __exit SYNC_DRV_ModExit(mt_void)
{
    if (!IS_ERR_OR_NULL(sync_priv_data_info.hdclk))
        clk_prepare_enable(sync_priv_data_info.hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.sdclk_27m))
        clk_prepare_enable(sync_priv_data_info.sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data_info.tsiclk))
        clk_prepare_enable(sync_priv_data_info.tsiclk);

    mt_drv_dev_unregister(&g_SyncRegisterData);

    SYNC_IntfUnRegister();

#ifndef MT_MCE_SUPPORT
    SYNC_DRV_Exit();
#endif

    if (!IS_ERR_OR_NULL(sync_priv_data_info.hdclk)) {
        clk_disable_unprepare(sync_priv_data_info.hdclk);
        clk_put(sync_priv_data_info.hdclk);
    }
    if (!IS_ERR_OR_NULL(sync_priv_data_info.sdclk_27m)) {
        clk_disable_unprepare(sync_priv_data_info.sdclk_27m);
        clk_put(sync_priv_data_info.sdclk_27m);
    }
    if (!IS_ERR_OR_NULL(sync_priv_data_info.tsiclk)) {
        clk_disable_unprepare(sync_priv_data_info.tsiclk);
        clk_put(sync_priv_data_info.tsiclk);
    }

}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


