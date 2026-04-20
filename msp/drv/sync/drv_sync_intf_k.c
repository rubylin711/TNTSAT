/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
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
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <asm/atomic.h>

#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "mt_module_debug.h"

#include "drv_sync.h"
#include "drv_sync_intf.h"
#include "mt_drv_module.h"
#include "mt_module.h"
#include "drv_sync_ext.h"
#include "mt_kernel_adapt.h"
#include "mt_osal.h"
#include "drv_sync.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define SYNC_NAME   "MT_SYNC"

static atomic_t    g_SyncCount = ATOMIC_INIT(0);

SYNC_GLOBAL_STATE_S  g_SyncGlobalState;

MT_DECLARE_MUTEX(g_SyncMutex);

SYNC_REGISTER_PARAM_S  *g_pSyncProcPara = NULL;
extern mt_void avsync_aria_init(void);

SYNC_S  *SYNC_getInfoPtr(mt_u32 SyncId)
{
    if(SyncId >= g_SyncGlobalState.SyncCount)
    {
        return NULL;
    }
    return g_SyncGlobalState.SyncInfo[SyncId].pSync;
}



static mt_s32 SYNC_InitUsedProc(mt_void)
{
    mt_u32            i, j;
    mt_char           ProcName[16];
    mt_proc_entry_t  *pProcItem = NULL;

    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].pSync)
        {
            mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_SYNC, i);
            pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);
            if (!pProcItem)
            {
                MT_FATAL_SYNC("add %s proc failed.\n", ProcName);

                for(j=0; j<i; j++)
                {
                    if (g_SyncGlobalState.SyncInfo[j].pSync)
                    {
                        mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_SYNC, j);
                        mt_drv_proc_rm_module(ProcName);
                    }
                }

                return MT_FAILURE;
            }
            pProcItem->read  = g_pSyncProcPara->rdproc;
            pProcItem->write = g_pSyncProcPara->wtproc;
        }

      MT_INFO_SYNC("SYNC_InitUsedProc: [%s] [%d]\n",ProcName, i);
    }

    return MT_SUCCESS;
}

static mt_void SYNC_DeInitUsedProc(mt_void)
{
    mt_u32  i;
    mt_char ProcName[16];

    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].pSync)
        {
            mt_osal_snprintf(ProcName, sizeof(ProcName),  "%s%02d", MT_MOD_SYNC, i);
            mt_drv_proc_rm_module(ProcName);
        }
    }

    return;
}


mt_s32 SYNC_IntfRegister(SYNC_REGISTER_PARAM_S *SyncProcPara)
{
    mt_s32  Ret;

    g_pSyncProcPara = SyncProcPara;

    Ret = SYNC_InitUsedProc();
    if (Ret != MT_SUCCESS)
    {
        g_pSyncProcPara = MT_NULL;
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


mt_void SYNC_IntfUnRegister(mt_void)
{
    SYNC_DeInitUsedProc();
    g_pSyncProcPara = MT_NULL;

    return;
}


mt_s32 SYNC_Create(SYNC_CREATE_S *pSyncCreate, struct file *file)
{
    mt_proc_entry_t  *pProcItem;
    mt_char           ProcName[12];
    mmz_buffer_s  MemBuf;
    mt_s32            Ret;
    mt_u32            i;
    mt_char           BufName[32];

    if (SYNC_MAX_NUM == g_SyncGlobalState.SyncCount)
    {
        MT_ERR_SYNC("the sync num is max.\n");
        return MT_ERR_SYNC_CREATE_ERR;
    }

    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        if (MT_NULL == g_SyncGlobalState.SyncInfo[i].pSync)
        {
            break;
        }
    }

    mt_osal_snprintf(BufName, sizeof(BufName), "SYNC_Inst%02d", i);

	if (sizeof(SYNC_S) > 0x1000)
	{
		BUG_ON(1);
	}

    Ret = mt_drv_mmz_alloc_and_map(BufName, MMZ_OTHERS, 0x1000, 0, &MemBuf);
    if (Ret != MT_SUCCESS)
    {
        MT_FATAL_SYNC("malloc %s mmz failed.\n", BufName);
        return Ret;
    }

    mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_SYNC, i);

    if (g_pSyncProcPara)
    {
        pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);
        if (!pProcItem)
        {
            MT_FATAL_SYNC("add %s proc failed.\n", ProcName);
            return MT_FAILURE;
        }
        pProcItem->read  = g_pSyncProcPara->rdproc;
        pProcItem->write = g_pSyncProcPara->wtproc;
    }

    g_SyncGlobalState.SyncInfo[i].pSync = (SYNC_S *)MemBuf.startVirAddr;
    g_SyncGlobalState.SyncInfo[i].SyncPhyAddr = MemBuf.startPhyAddr;
    g_SyncGlobalState.SyncInfo[i].File = (ulong)file;

    pSyncCreate->SyncId = i;
    pSyncCreate->SyncPhyAddr = MemBuf.startPhyAddr;
    g_SyncGlobalState.SyncCount++;
	if (g_SyncGlobalState.AddSyncIns)
	{
	    g_SyncGlobalState.AddSyncIns((void*)&i);
	}

    return MT_SUCCESS;
}

mt_void avsync_get_current_psync(SYNC_S *pSync);

mt_s32 SYNC_Destroy(mt_u32 SyncId)
{
    mt_char   ProcName[15];
    mmz_buffer_s      MemBuf;

    if (MT_NULL == g_SyncGlobalState.SyncInfo[SyncId].pSync)
    {
        MT_ERR_SYNC("this is invalid handle.\n");
        return MT_ERR_SYNC_DESTROY_ERR;
    }

    if (g_pSyncProcPara)
    {
        memset(ProcName, 0, sizeof(ProcName));
        mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_SYNC, SyncId);
        mt_drv_proc_rm_module(ProcName);
    }

	avsync_get_current_psync(NULL);

    MemBuf.startVirAddr = (void*)g_SyncGlobalState.SyncInfo[SyncId].pSync;
    MemBuf.startPhyAddr = g_SyncGlobalState.SyncInfo[SyncId].SyncPhyAddr;
    MemBuf.size = 0x1000;

    mt_drv_mmz_unmap_and_release(&MemBuf);

    g_SyncGlobalState.SyncInfo[SyncId].pSync = MT_NULL;
    g_SyncGlobalState.SyncInfo[SyncId].SyncPhyAddr = MT_NULL;
    g_SyncGlobalState.SyncInfo[SyncId].File = MT_NULL;
    g_SyncGlobalState.SyncInfo[SyncId].SyncUsrAddr = MT_NULL;

    g_SyncGlobalState.SyncCount--;
	if (g_SyncGlobalState.DelSyncIns)
	{
		g_SyncGlobalState.DelSyncIns((void*)&SyncId);
	}

    return MT_SUCCESS;
}


mt_s32 SYNC_SetUsrAddr(SYNC_USR_ADDR_S *pSyncUsrAddr)
{
    g_SyncGlobalState.SyncInfo[pSyncUsrAddr->SyncId].SyncUsrAddr = (ulong)(pSyncUsrAddr->SyncUsrAddr);
    return MT_SUCCESS;
}


mt_s32 SYNC_CheckId(SYNC_USR_ADDR_S *pSyncUsrAddr, struct file *file)
{
    if (g_SyncGlobalState.SyncInfo[pSyncUsrAddr->SyncId].File != ((ulong)file))
    {
        MT_ERR_SYNC("this is invalid handle.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    if (MT_NULL == g_SyncGlobalState.SyncInfo[pSyncUsrAddr->SyncId].pSync)
    {
        MT_ERR_SYNC("this is invalid handle.\n");
        return MT_ERR_SYNC_INVALID_PARA;
    }

    pSyncUsrAddr->SyncUsrAddr = g_SyncGlobalState.SyncInfo[pSyncUsrAddr->SyncId].SyncUsrAddr;

    return MT_SUCCESS;
}


mt_s32 SYNC_CheckNum(mt_u32 *pSyncNum, struct file *file)
{
    mt_u32   i;

    *pSyncNum = 0;
    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        /*multi-thread process in MCE mode*/
        /*CNcomment:此时mce方式类多线程处理即可 */
        if (g_SyncGlobalState.SyncInfo[i].File == ((ulong)file))
        {
            (*pSyncNum)++;
        }
    }

    return MT_SUCCESS;
}


mt_s32 SYNC_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    mt_s32           Ret = MT_SUCCESS;

    Ret = down_interruptible(&g_SyncMutex);

    switch (cmd)
    {
        case CMD_SYNC_CREATE:
        {
            SYNC_CREATE_S  *pSyncCreate;

            pSyncCreate = (SYNC_CREATE_S *)arg;

            Ret = SYNC_Create(pSyncCreate, file);

            break;
        }

        case CMD_SYNC_DESTROY:
        {
            Ret = SYNC_Destroy(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_SET_USRADDR:
        {
            SYNC_USR_ADDR_S *pSyncUsrAddr;

            pSyncUsrAddr = (SYNC_USR_ADDR_S *)arg;

            Ret = SYNC_SetUsrAddr(pSyncUsrAddr);

            break;
        }

        case CMD_SYNC_CHECK_ID:
        {
            SYNC_USR_ADDR_S *pSyncUsrAddr;

            pSyncUsrAddr = (SYNC_USR_ADDR_S *)arg;

            Ret = SYNC_CheckId(pSyncUsrAddr, file);

            break;
        }

        case CMD_SYNC_CHECK_NUM:
        {
            Ret = SYNC_CheckNum((mt_u32 *)arg, file);

            break;
        }

        case CMD_SYNC_START_SYNC:
        {
            Ret = SYNC_StartSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_AUD_JUDGE:
        {
            SYNC_AUD_JUDGE_S  *pAudJudge;

            pAudJudge = (SYNC_AUD_JUDGE_S *)arg;

            SYNC_AudProc(pAudJudge->hSync, &pAudJudge->AudInfo, &pAudJudge->AudOpt);

            Ret = MT_SUCCESS;

            break;
        }

        case CMD_SYNC_VID_JUDGE:
        {
            SYNC_VID_JUDGE_S  *pVidJudge;

            pVidJudge = (SYNC_VID_JUDGE_S *)arg;

            SYNC_VidProc(pVidJudge->hSync, &pVidJudge->VidInfo, &pVidJudge->VidOpt);

            Ret = MT_SUCCESS;

            break;
        }

        case CMD_SYNC_PAUSE_SYNC:
        {
            Ret = SYNC_PauseSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_RESUME_SYNC:
        {
            Ret = SYNC_ResumeSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_GET_TIME:
        {
            SYNC_GET_TIME_S    *pSyncTime;

            pSyncTime = (SYNC_GET_TIME_S *)arg;
            Ret = SYNC_GetTime(pSyncTime->SyncId, &pSyncTime->LocalTime, &pSyncTime->PlayTime);

            break;
        }

        case CMD_SYNC_GET_TIME_INFO:
        {
            SYNC_GET_TIME_INFO_S    *pSyncTimeInfo;

            pSyncTimeInfo = (SYNC_GET_TIME_INFO_S *)arg;
            Ret = avsync_get_time_info(pSyncTimeInfo->SyncId, pSyncTimeInfo);
            //printk("D: [%x]\n", (mt_u32)pSyncTimeInfo);
            break;
        }
		case CMD_SYNC_GET_VFRM_CNT:
		{
			mt_s32 * cnt = (mt_s32 * )arg;
			*cnt = avsync_video_get_play_cnt();
			break;
		}
        case CMD_SYNC_PUSH_VPTS:
        {
            break;
        }

        case CMD_SYNC_PUSH_APTS:
        {
            SYNC_PUSH_APTS_S *pSyncApts = NULL;

            pSyncApts = (SYNC_PUSH_APTS_S *)arg;

            Ret = (mt_s32)avsync_audio_push_pts(NULL, pSyncApts);
            break;
        }

        case CMD_SYNC_INIT_AUD:
        {
            mt_u32 *a_fmt;
            
            a_fmt = (mt_u32 *)arg;
            
            avsync_audio_init(*a_fmt);
            Ret = MT_SUCCESS;
            break;
        }

        case CMD_SYNC_DEINIT_AUD:
        {
              avsync_audio_deinit();
               Ret = MT_SUCCESS;
			   break;
        }
        
        case CMD_SYNC_INIT_VID:
        {
             mt_u32 *v_fmt;
            
            v_fmt = (mt_u32 *)arg;
            
            avsync_video_init(*v_fmt);
            Ret = MT_SUCCESS;
            break;
        }

        case CMD_SYNC_DEINIT_VID:
        {
              avsync_video_deinit();
              Ret = MT_SUCCESS;
			  break;
        }
        
        case CMD_SYNC_ADEC_PTS:
        {
            Ret = MT_SUCCESS;
            break;
        }


        case CMD_SYNC_APTS_ADJUST:
        {
        	Ret = avsync_apts_adjust(*((mt_u32 *)arg));
            break;
        }
		
        case CMD_SYNC_AUD_TRACK:
        {
            mt_u32 track = 0;

            track = *((mt_u32 *)arg);
            Ret = avsync_audio_do_track(&track);
			
            break;
        }

        case CMD_SYNC_GET_AVSYNC_INFO:
        {
            Ret = avsync_get_avsync_info((SYNC_GET_AVSYNC_INFO_S *)arg);
            if (MT_SUCCESS != Ret)
            {
                MT_ERR_SYNC("CMD_SYNC_GET_AVSYNC_INFO failed Ret = %x\n", Ret);
            }
            break;
        }

        case CMD_SYNC_CFG_PLAY_INFO:
        {
            SYNC_PLAY_INFO_S play_info;
			
            memset(&play_info, 0, sizeof(play_info));
            memcpy(&play_info, arg, sizeof(play_info));
            Ret = avsync_play_info_cfg((mt_u32 *)(&play_info));
            break;
        }

        case CMD_SYNC_SET_AVSYNC_REF_MODE:
        {
            SYNC_S  *pSync;
            SYNC_SET_AVSYNC_MODE_S *p_set_ref_mode = NULL;

            p_set_ref_mode = (SYNC_SET_AVSYNC_MODE_S *)arg;
            pSync = g_SyncGlobalState.SyncInfo[p_set_ref_mode->SyncId].pSync;       
            pSync->SyncAttr.enSyncRef = p_set_ref_mode->sync_ref_mode;
            Ret = SYNC_reference_config(pSync);

            if (MT_SUCCESS != Ret)
            {
                MT_ERR_SYNC("CMD_SYNC_SET_AVSYNC_REF_MODE failed Ret = %x\n", Ret);
            }

            break;
        }

        case CMD_SYNC_VFRM_INFO_CAP_ENABLE:
        {
            MT_BOOL enable = *((MT_BOOL *)arg);
            
            MT_INFO_SYNC("on off video frame info capture %d\n", enable);
            if(enable)
            {
                avsync_video_frame_info_capture_create(60);
            }
            else
            {
                avsync_video_frame_info_capture_destroy();
            }
            Ret = MT_SUCCESS;
            break;
        }

        case CMD_SYNC_VFRM_INFO_CAP_READ:
        {
            Ret = avsync_video_frame_info_capture_read((SYNC_VOUT_FRAME_INFO *)arg);
            break;
        }

        case CMD_SYNC_SET_AVSYNC_DATA_SOURCE:
        {
            MT_SYNC_DATA_SOURCE_E *pdata_source = NULL;

            pdata_source = (MT_SYNC_DATA_SOURCE_E *)arg;
            Ret = avsync_data_source_cfg(pdata_source);

            if (MT_SUCCESS != Ret)
            {
                MT_ERR_SYNC("CMD_SYNC_SET_AVSYNC_DATA_SOURCE failed Ret = %x\n", Ret);
            }

            break;
        }

        default:
            up(&g_SyncMutex);
            return -ENOIOCTLCMD;
    }

    up(&g_SyncMutex);
    return Ret;
}

mt_s32 SYNC_DRV_Open(struct inode *finode, struct file  *ffile)
{
    mt_s32            Ret;
    MT_S32 mt_idx = iminor(finode);
    SYNC_Priv_Data_S *sync_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(sync_priv_data->hdclk))
        clk_prepare_enable(sync_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data->sdclk_27m))
        clk_prepare_enable(sync_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data->tsiclk))
        clk_prepare_enable(sync_priv_data->tsiclk);

    Ret = down_interruptible(&g_SyncMutex);

    if (1 == atomic_inc_return(&g_SyncCount))
    {
    }

    up(&g_SyncMutex);
    return 0;
}


mt_s32 SYNC_DRV_Close(struct inode *finode, struct file  *ffile)
{
    mt_s32           i;
    mt_s32           Ret;
    MT_S32 mt_idx = iminor(finode);
    SYNC_Priv_Data_S *sync_priv_data = get_mt_priv(mt_idx);

    Ret = down_interruptible(&g_SyncMutex);

    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].File == ((ulong)ffile))
        {
            Ret = SYNC_Destroy(i);
            if (Ret != MT_SUCCESS)
            {
                up(&g_SyncMutex);
                return -1;
            }
        }
    }

    if (atomic_dec_and_test(&g_SyncCount))
    {
    }

    up(&g_SyncMutex);

    if (!IS_ERR_OR_NULL(sync_priv_data->hdclk))
        clk_disable_unprepare(sync_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data->sdclk_27m))
        clk_disable_unprepare(sync_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data->tsiclk))
        clk_disable_unprepare(sync_priv_data->tsiclk);

    return 0;
}

mt_s32 avsync_int_register(void);
mt_s32 avsync_int_unregister(void);

mt_s32 SYNC_Suspend(basedev_s *pdev, pm_message_t state)
{
    mt_s32  Ret;
    mt_u32  i;
    SYNC_Priv_Data_S *sync_priv_data = dev_get_platdata(&pdev->dev);

    Ret = down_trylock(&g_SyncMutex);
    if (Ret)
    {
        MT_FATAL_SYNC("down g_SyncMutex failed.\n");
        return -1;
    }

    /* if SYNC device has never opened by any process,return directly */
    if (!atomic_read(&g_SyncCount))
    {
        up(&g_SyncMutex);
        return 0;
    }

    /* it needs to pause when standby,because system time keep running*/
    for (i = 0; i < SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].pSync)
        {
            SYNC_PauseSync(i);
        }
    }

    MT_PRINT("SYNC suspend OK\n");

    up(&g_SyncMutex);
	avsync_int_unregister();
    if (!IS_ERR_OR_NULL(sync_priv_data->hdclk))
        clk_disable_unprepare(sync_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data->sdclk_27m))
        clk_disable_unprepare(sync_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data->tsiclk))
        clk_disable_unprepare(sync_priv_data->tsiclk);

    return 0;
}

mt_s32 avsync_module_resume(mt_void);

mt_s32 SYNC_Resume(basedev_s *pdev)
{
    mt_s32  Ret;
    mt_u32  i;
    SYNC_Priv_Data_S *sync_priv_data = dev_get_platdata(&pdev->dev);
	avsync_module_resume();

    if (!IS_ERR_OR_NULL(sync_priv_data->hdclk))
        clk_prepare_enable(sync_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(sync_priv_data->sdclk_27m))
        clk_prepare_enable(sync_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(sync_priv_data->tsiclk))
        clk_prepare_enable(sync_priv_data->tsiclk);

    Ret = down_trylock(&g_SyncMutex);
    if (Ret)
    {
        MT_FATAL_SYNC("down g_SyncMutex failed.\n");
        return -1;
    }

    if (!atomic_read(&g_SyncCount))
    {
        up(&g_SyncMutex);
        return 0;
    }

    for (i = 0; i < SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].pSync)
        {
            SYNC_ResumeSync(i);
        }
    }

    MT_PRINT("SYNC resume OK\n");

    up(&g_SyncMutex);
    return 0;
}


EXPORT_SYMBOL(SYNC_getInfoPtr);
EXPORT_SYMBOL(SYNC_IntfRegister);
EXPORT_SYMBOL(SYNC_IntfUnRegister);
EXPORT_SYMBOL(SYNC_Ioctl);
EXPORT_SYMBOL(SYNC_DRV_Open);
EXPORT_SYMBOL(SYNC_DRV_Close);
EXPORT_SYMBOL(SYNC_Suspend);
EXPORT_SYMBOL(SYNC_Resume);
/*Temporary useage*/
EXPORT_SYMBOL(SYNC_VidProc);
EXPORT_SYMBOL(SYNC_GetLocalTime);


static SYNC_EXPORT_FUNC_S g_SyncExportFuncs =
{
    .pfnSYNC_VidProc        = (mt_void *)SYNC_VidProc,
    .pfnSYNC_PcrProc        = (mt_void *)SYNC_PcrProc,
    .pfnSYNC_VerifyHandle   = (mt_void *)SYNC_VerifyHandle
};

mt_s32 SYNC_DRV_Init(mt_void)
{
    mt_s32    ret;
    mt_u32    i;
    
    MT_INFO_SYNC("SYNC_DRV_Init: start\n");
    
    ret = mt_drv_module_register(MT_ID_SYNC, SYNC_NAME, (mt_void*)&g_SyncExportFuncs);
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_SYNC("ERR: MT_DRV_MODULE_Register!\n");
        return ret;
    }

    g_pSyncProcPara = MT_NULL;

    g_SyncGlobalState.SyncCount = 0;
	g_SyncGlobalState.AddSyncIns = 0;
	g_SyncGlobalState.DelSyncIns = 0;
    g_SyncGlobalState.SyncCount = 0;
    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        g_SyncGlobalState.SyncInfo[i].pSync = MT_NULL;
        g_SyncGlobalState.SyncInfo[i].SyncPhyAddr = MT_NULL;
        g_SyncGlobalState.SyncInfo[i].File = MT_NULL;
        g_SyncGlobalState.SyncInfo[i].SyncUsrAddr = MT_NULL;
    }

    avsync_module_init();

    MT_INFO_SYNC("SYNC_DRV_Init: end\n");
    
    return 0;
}

mt_void SYNC_DRV_Exit(mt_void)
{
    avsync_module_deinit();
    mt_drv_module_unregister(MT_ID_SYNC);
    return;
}


mt_s32 SYNC_MOD_Init(mt_void)
{
    mt_s32     Ret;

    Ret = down_interruptible(&g_SyncMutex);

    if (1 == atomic_inc_return(&g_SyncCount))
    {
    }

    up(&g_SyncMutex);

    return 0;
}

mt_s32 SYNC_MOD_DeInit(mt_void)
{
    mt_s32           i;
    mt_s32           Ret;

    Ret = down_interruptible(&g_SyncMutex);

    for (i=0; i<SYNC_MAX_NUM; i++)
    {
        if (g_SyncGlobalState.SyncInfo[i].pSync && (0xffffffff == g_SyncGlobalState.SyncInfo[i].File))
        {
            Ret = SYNC_Destroy(i);
            if (Ret != MT_SUCCESS)
            {
                up(&g_SyncMutex);
                return -1;
            }
        }
    }

    if (atomic_dec_and_test(&g_SyncCount))
    {
    }

    up(&g_SyncMutex);

    return 0;
}

mt_s32 SYNC_MOD_Ioctl(unsigned int cmd, mt_void *arg)
{
    mt_s32           Ret;

    Ret = down_interruptible(&g_SyncMutex);

    switch (cmd)
    {
        case CMD_SYNC_CREATE:
        {
            SYNC_CREATE_S  *pSyncCreate;

            pSyncCreate = (SYNC_CREATE_S *)arg;

            Ret = SYNC_Create(pSyncCreate, (struct file *)0xffffffff);

            break;
        }

        case CMD_SYNC_DESTROY:
        {
            Ret = SYNC_Destroy(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_START_SYNC:
        {
            Ret = SYNC_StartSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_AUD_JUDGE:
        {
            SYNC_AUD_JUDGE_S  *pAudJudge;

            pAudJudge = (SYNC_AUD_JUDGE_S *)arg;

            SYNC_AudProc(pAudJudge->hSync, &pAudJudge->AudInfo, &pAudJudge->AudOpt);

            Ret = MT_SUCCESS;

            break;
        }

        case CMD_SYNC_VID_JUDGE:
        {
            SYNC_VID_JUDGE_S  *pVidJudge;

            pVidJudge = (SYNC_VID_JUDGE_S *)arg;

            SYNC_VidProc(pVidJudge->hSync, &pVidJudge->VidInfo, &pVidJudge->VidOpt);

            Ret = MT_SUCCESS;

            break;
        }

        case CMD_SYNC_PAUSE_SYNC:
        {
            Ret = SYNC_PauseSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_RESUME_SYNC:
        {
            Ret = SYNC_ResumeSync(*((mt_u32 *)arg));

            break;
        }

        case CMD_SYNC_GET_TIME:
        {
            SYNC_GET_TIME_S    *pSyncTime;

            pSyncTime = (SYNC_GET_TIME_S *)arg;

            Ret = SYNC_GetTime(pSyncTime->SyncId, &pSyncTime->LocalTime, &pSyncTime->PlayTime);

            break;
        }

        default:
            up(&g_SyncMutex);
            return -ENOIOCTLCMD;
    }

    up(&g_SyncMutex);
    return Ret;
}


//#ifdef HI_MCE_SUPPORT
/*the following interface is only for mce now*/
/*CNCommont: 下述接口均为SYNC模块驱动下对外接口，目前仅提供给MCE模块使用*/
static mt_s32 DRV_SYNC_CheckHandle(mt_handle hSync, SYNC_S **ppSync, mt_u32 *pSyncID)
{
    *pSyncID = hSync & 0xff;

    if(*pSyncID >= SYNC_MAX_NUM)
    {
        MT_ERR_SYNC("ERR: invalid handle!");
        return MT_FAILURE;
    }

    if(MT_NULL == g_SyncGlobalState.SyncInfo[*pSyncID].pSync)
    {
        MT_ERR_SYNC("ERR: pSync = null!");
        return MT_FAILURE;
    }

    *ppSync = g_SyncGlobalState.SyncInfo[*pSyncID].pSync;

    return MT_SUCCESS;
}

static mt_void DRV_SYNC_ResetStatInfo(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidFirstCome = MT_FALSE;
        pSync->VidFirstSysTime = MT_INVALID_TIME_U64;
        pSync->VidFirstPts = MT_INVALID_PTS_U64;
        pSync->VidLastPts = MT_INVALID_PTS_U64;
        pSync->VidPreSyncTargetInit = MT_FALSE;
        pSync->VidPreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->VidLocalTimeFlag = MT_FALSE;
        pSync->VidLastSysTime = MT_INVALID_TIME_U64;
        pSync->VidLastLocalTime = MT_INVALID_TIME_U64;
        pSync->VidPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->VidPtsSeriesCnt = 0;

        pSync->CrtBufStatus.VidBufState = SYNC_BUF_STATE_NORMAL;
        pSync->CrtBufStatus.VidBufPercent = 0;
        pSync->CrtBufStatus.bOverflowDiscFrm= MT_FALSE;
        pSync->VidSyndAdjust = MT_FALSE;
        pSync->VidDisPlayCnt = 0;
        pSync->VidDiscardCnt = 0;
        pSync->VidRepPlayCnt = 0;
        pSync->VidRepeatCnt = 0;
        pSync->VidFirstPlay = MT_FALSE;
        pSync->VidFirstPlayTime = MT_INVALID_TIME_U64;
        pSync->VidInfo.SrcPts = MT_INVALID_PTS_U64;
        pSync->VidInfo.Pts = MT_INVALID_PTS_U64;
        pSync->VidInfo.FrameTime = 40;
        pSync->VidInfo.DelayTime = 20;

        pSync->VidFirstValidCome = MT_FALSE;
        pSync->VidFirstValidPts = MT_INVALID_PTS_U64;
        pSync->VidLastSrcPts = MT_INVALID_PTS_U64;
        pSync->VidPtsLoopBack = MT_FALSE;

        pSync->VidFirstDecPts = MT_INVALID_PTS_U64;
        pSync->VidSecondDecPts = MT_INVALID_PTS_U64;
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudFirstCome = MT_FALSE;
        pSync->AudFirstSysTime = MT_INVALID_TIME_U64;
        pSync->AudFirstPts = MT_INVALID_PTS_U64;
        pSync->AudLastPts = MT_INVALID_PTS_U64;
        pSync->AudLastBufTime = 0;
        pSync->AudPreSyncTargetInit = MT_FALSE;
        pSync->AudPreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->AudLocalTimeFlag = MT_FALSE;
        pSync->AudLastSysTime = MT_INVALID_TIME_U64;
        pSync->AudLastLocalTime = MT_INVALID_TIME_U64;
        pSync->AudPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->AudPtsSeriesCnt = 0;

        pSync->CrtBufStatus.AudBufState = SYNC_BUF_STATE_NORMAL;
        pSync->CrtBufStatus.AudBufPercent = 0;

        pSync->AudReSync = MT_TRUE;
        pSync->AudReBufFund = MT_TRUE;
        pSync->AudPlayCnt = 0;
        pSync->AudDiscardCnt = 0;
        pSync->AudRepeatCnt = 0;
        pSync->AudInfo.SrcPts = MT_INVALID_PTS_U64;
        pSync->AudInfo.Pts = MT_INVALID_PTS_U64;
        pSync->AudInfo.FrameTime = 24;
        pSync->AudInfo.BufTime = 0;
        pSync->AudInfo.FrameNum = 0;
        pSync->AudFirstPlay = MT_FALSE;
        pSync->AudFirstPlayTime = MT_INVALID_TIME_U64;

        pSync->AudFirstValidCome = MT_FALSE;
        pSync->AudFirstValidPts = MT_INVALID_PTS_U64;
        pSync->AudLastSrcPts = MT_INVALID_PTS_U64;
        pSync->AudPtsLoopBack = MT_FALSE;
    }

    if ((!pSync->VidEnable)
      &&(!pSync->AudEnable)
       )
    {
        pSync->CrtStatus = SYNC_STATUS_STOP;
        pSync->PreSyncStartSysTime = MT_INVALID_TIME_U64;
        pSync->PreSyncEndSysTime = MT_INVALID_TIME_U64;
        pSync->PreSyncFinish = MT_FALSE;
        pSync->BufFundEndSysTime = MT_INVALID_TIME_U64;
        pSync->BufFundFinish = MT_FALSE;
        pSync->PreSyncTarget = SYNC_CHAN_VID;
        pSync->PreSyncTargetTime = MT_INVALID_TIME_U64;
        pSync->PreSyncTargetInit = MT_FALSE;

        pSync->PcrSyncInfo.PcrFirstCome = MT_FALSE;
        pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_TRUE;
        pSync->PcrSyncInfo.PcrFirstSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrFirst = MT_INVALID_PTS_U64;
        pSync->PcrSyncInfo.PcrLast = MT_INVALID_PTS_U64;
        pSync->PcrSyncInfo.PcrLastSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrLastLocalTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrPauseLocalTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrLocalTimeFlag = MT_FALSE;
        pSync->PcrSyncInfo.PcrSeriesCnt = 0;
        pSync->PcrSyncInfo.PcrSyncStartSysTime = MT_INVALID_TIME_U64;
        pSync->PcrSyncInfo.PcrDelta = 0;
        pSync->PcrSyncInfo.enPcrAdjust = SYNC_SCR_ADJUST_BUTT;
        pSync->PcrSyncInfo.PcrLoopBack = MT_FALSE;

        pSync->ScrInitFlag = MT_FALSE;
        pSync->ScrFirstLocalTime = MT_INVALID_TIME_U64;
        pSync->ScrFirstSysTime = MT_INVALID_TIME_U64;
        pSync->ScrLastLocalTime = MT_INVALID_TIME_U64;
        pSync->ScrLastSysTime = MT_INVALID_TIME_U64;

        pSync->AudReSync = MT_FALSE;
        pSync->AudReBufFund = MT_FALSE;

        pSync->SyncEvent.bAudPtsJump = MT_FALSE;
        pSync->SyncEvent.bVidPtsJump = MT_FALSE;
        pSync->SyncEvent.bStatChange = MT_FALSE;		
		pSync->SyncEvent.bEos_back = MT_FALSE;
        pSync->LoopBackTime = MT_INVALID_TIME_U64;
        pSync->LoopBackFlag = MT_FALSE;
    }

    return;
}

mt_s32 MT_DRV_SYNC_Init(mt_void)
{
    mt_s32          Ret;

    /*KO加载前半部*/
    Ret = SYNC_DRV_Init();

    /*本模块基本初始化*/
    Ret |= SYNC_MOD_Init();

    return Ret;
}

mt_s32 MT_DRV_SYNC_DeInit(mt_void)
{
    mt_s32          Ret;

    Ret = SYNC_MOD_DeInit();

    SYNC_DRV_Exit();

    return Ret;
}

mt_s32 MT_DRV_SYNC_Create(MT_UNF_SYNC_ATTR_S *pstSyncAttr, mt_handle *phSync)
{
    mt_s32          Ret;
    SYNC_S          *pSync = MT_NULL;
    SYNC_CREATE_S   SyncCreate;

    if(MT_NULL == pstSyncAttr || MT_NULL == phSync)
    {
        return MT_FAILURE;
    }


	Ret = SYNC_Create(&SyncCreate, (struct file *)0xffffffff);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: SYNC_MOD_Ioctl!");
        return Ret;
    }

    pSync = g_SyncGlobalState.SyncInfo[SyncCreate.SyncId].pSync;

    pSync->SyncAttr  = *pstSyncAttr;
    pSync->VidEnable = MT_FALSE;
    pSync->AudEnable = MT_FALSE;
    pSync->CrtStatus = SYNC_STATUS_STOP;
    pSync->bPrint = MT_TRUE;
    pSync->bPrint = MT_FALSE;
    pSync->bUseStopRegion = MT_TRUE;

    DRV_SYNC_ResetStatInfo(pSync, SYNC_CHAN_VID);
    DRV_SYNC_ResetStatInfo(pSync, SYNC_CHAN_AUD);

    *phSync = (MT_ID_SYNC << 16) | SyncCreate.SyncId;

    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_Destroy(mt_handle hSync)
{
    mt_s32              Ret;
    mt_u32              SyncID;

    SyncID = hSync & 0xff;

    Ret = SYNC_Destroy(SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: SYNC_Destroy.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS ;
}

mt_s32 MT_DRV_SYNC_Start(mt_handle hSync, SYNC_CHAN_E enChn)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    MT_ERR_SYNC("%s: [%d] [%d]\n", __FUNCTION__, __LINE__, enChn);
     
    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidEnable = MT_TRUE;
        Ret = SYNC_StartSync(SyncID);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_SYNC("SYNC_StartSync err.\n");
            return MT_FAILURE;
        }

        //avsync_video_init(NULL,  0);
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudEnable = MT_TRUE;
        Ret = SYNC_StartSync(SyncID);
        if (Ret != MT_SUCCESS)
        {
            MT_ERR_SYNC("SYNC_StartSync err.\n");
            return MT_FAILURE;
        }

        //avsync_audio_init(0);
    }

    MT_ERR_SYNC("%s: [%d] [%d]\n", __FUNCTION__, __LINE__, enChn);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_Stop(mt_handle hSync, SYNC_CHAN_E enChn)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    MT_ERR_SYNC("%s: [%d] [%d]\n", __FUNCTION__, __LINE__, enChn);
    
    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    if (enChn == SYNC_CHAN_VID)
    {
        pSync->VidEnable = MT_FALSE;
        DRV_SYNC_ResetStatInfo(pSync, SYNC_CHAN_VID);

        //avsync_video_deinit();
    }

    if (enChn == SYNC_CHAN_AUD)
    {
        pSync->AudEnable = MT_FALSE;
        DRV_SYNC_ResetStatInfo(pSync, SYNC_CHAN_AUD);
        //avsync_audio_deinit();
    }

    MT_ERR_SYNC("%s: [%d] [%d]\n", __FUNCTION__, __LINE__, enChn);
    
    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_Play(mt_handle hSync)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    pSync->CrtStatus = SYNC_STATUS_PLAY;

    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_AudJudge(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    SYNC_AudProc(hSync, pAudInfo, pAudOpt);

    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_VidJudge(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    SYNC_VidProc(hSync, pVidInfo, pVidOpt);

    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_GetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pstSyncAttr)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    memcpy(pstSyncAttr, &pSync->SyncAttr, sizeof(MT_UNF_SYNC_ATTR_S));

    return MT_SUCCESS;
}

mt_s32 MT_DRV_SYNC_SetAttr(mt_handle hSync, MT_UNF_SYNC_ATTR_S *pSyncAttr)
{
    mt_s32      Ret;
    SYNC_S      *pSync = MT_NULL;
    mt_u32      SyncID;

    Ret = DRV_SYNC_CheckHandle(hSync, &pSync, &SyncID);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_SYNC("ERR: DRV_SYNC_CheckHandle.\n");
        return MT_FAILURE;
    }

    memcpy(&pSync->SyncAttr, pSyncAttr, sizeof(MT_UNF_SYNC_ATTR_S));

    return MT_SUCCESS;
}
//#endif

mt_s32 MT_DRV_SYNC_RegCallback(SyncManage synAddCallBack,SyncManage synDelCallBack)
{
	g_SyncGlobalState.AddSyncIns = synAddCallBack;
	g_SyncGlobalState.DelSyncIns = synDelCallBack;
	return MT_SUCCESS;
}

EXPORT_SYMBOL(MT_DRV_SYNC_Init);
EXPORT_SYMBOL(MT_DRV_SYNC_DeInit);
EXPORT_SYMBOL(MT_DRV_SYNC_Stop);
EXPORT_SYMBOL(MT_DRV_SYNC_Start);
EXPORT_SYMBOL(MT_DRV_SYNC_GetAttr);
EXPORT_SYMBOL(MT_DRV_SYNC_SetAttr);
EXPORT_SYMBOL(MT_DRV_SYNC_Create);
EXPORT_SYMBOL(MT_DRV_SYNC_Destroy);
EXPORT_SYMBOL(MT_DRV_SYNC_VidJudge);
EXPORT_SYMBOL(MT_DRV_SYNC_Play);
EXPORT_SYMBOL(MT_DRV_SYNC_RegCallback);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


