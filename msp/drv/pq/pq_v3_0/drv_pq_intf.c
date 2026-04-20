/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "drv_pq.h"
#include "mt_drv_dev.h"
#include "drv_pq_define.h"
#include "mt_module_debug.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

DEFINE_SEMAPHORE(g_stPQSemaphore, 1);

extern mt_s32  DRV_PQ_Suspend(basedev_s *pdev, pm_message_t state);
extern mt_s32  DRV_PQ_Resume(basedev_s *pdev);
extern mt_s32 MT_DRV_PQ_Init(MT_CHAR* pszPath);
extern mt_s32 MT_DRV_PQ_DeInit(mt_void);

static mt_s32 PQ_Open(struct inode* node, struct file* filp)
{
    return MT_SUCCESS;
}

static mt_s32 PQ_Close(struct inode* node, struct file* filp)
{
    return MT_SUCCESS;
}

static mt_s32 PQIoctl(struct inode* inode, struct file* filp, unsigned int cmd, MT_VOID* arg)
{
    mt_s32 s32Ret = MT_SUCCESS;

    s32Ret = down_interruptible(&g_stPQSemaphore);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_PQ("Acquire PQ mutex failed!\n");
        return s32Ret;
    }

    switch (cmd)
    {
        case MTIOC_PQ_S_PQ_PATH:
        {
            MT_PQ_PATE_S* pstAttr;
            pstAttr = (MT_PQ_PATE_S*)arg;

            s32Ret = DRV_PQ_Comsumer_DeInit();
            s32Ret |= DRV_PQ_Comsumer_Init(pstAttr->cPqPath);

            break;
        }

        case MTIOC_PQ_S_COLORTEMP:
        {
            MT_PQ_COLOR_TEMP_S* pstAttr;
            pstAttr = (MT_PQ_COLOR_TEMP_S*)arg;

            s32Ret = DRV_PQ_SetColorTemp(pstAttr);

            break;
        }
        case MTIOC_PQ_G_COLORTEMP:
        {
            MT_PQ_COLOR_TEMP_S* pstAttr;

            pstAttr = (MT_PQ_COLOR_TEMP_S*)arg;
            s32Ret = DRV_PQ_GetColorTemp(pstAttr);

            break;
        }

        case MTIOC_PQ_S_REGISTER:
        {
            MT_PQ_REGISTER_S* pstReg;

            pstReg = (MT_PQ_REGISTER_S*)arg;
            s32Ret = DRV_PQ_SetReg(pstReg);

            break;
        }
        case MTIOC_PQ_G_REGISTER:
        {
            MT_PQ_REGISTER_S* pstReg;

            pstReg = (MT_PQ_REGISTER_S*)arg;
            s32Ret = DRV_PQ_GetReg(pstReg);

            break;
        }

        case MTIOC_PQ_S_ACM_LUMA:
        {
            MT_PQ_ACM_LUT_S* pstAttr;
            pstAttr = (MT_PQ_ACM_LUT_S*)arg;

            s32Ret = DRV_PQ_SetAcmLuma(pstAttr);
            break;
        }
        case MTIOC_PQ_G_ACM_LUMA:
        {
#if 0 //janny
            COLOR_LUT_S* pstAttr;
            pstAttr = (COLOR_LUT_S*)arg;

            s32Ret = PQ_HAL_GetACMLumaTbl(pstAttr);
#endif
            break;
        }
        case MTIOC_PQ_S_ACM_HUE:
        {
            MT_PQ_ACM_LUT_S* pstAttr;
            pstAttr = (MT_PQ_ACM_LUT_S*)arg;

            s32Ret = DRV_PQ_SetAcmHue(pstAttr);
            break;
        }
        case MTIOC_PQ_G_ACM_HUE:
        {
#if 0 //janny
            COLOR_LUT_S* pstAttr;
            pstAttr = (COLOR_LUT_S*)arg;

            s32Ret = PQ_HAL_GetACMHueTbl(pstAttr);
#endif
            break;
        }
        case MTIOC_PQ_S_ACM_SAT:
        {
            MT_PQ_ACM_LUT_S* pstAttr;
            pstAttr = (MT_PQ_ACM_LUT_S*)arg;

            s32Ret = DRV_PQ_SetAcmSat(pstAttr);
            break;
        }
        case MTIOC_PQ_G_ACM_SAT:
        {
#if 0 //janny
            COLOR_LUT_S* pstAttr;
            pstAttr = (COLOR_LUT_S*)arg;

            s32Ret = PQ_HAL_GetACMSatTbl(pstAttr);
#endif
            break;
        }

#if 0
        case MTIOC_PQ_S_DCI:
        {
            DCI_WGT_S* pstAttr;
            pstAttr = (DCI_WGT_S*)arg;
            s32Ret = DRV_PQ_SetDCIWgtLut(pstAttr);

            break;
        }
        case MTIOC_PQ_G_DCI:
        {
            DCI_WGT_S* pstAttr;

            pstAttr = (DCI_WGT_S*)arg;
            s32Ret = PQ_HAL_GetDCIWgtLut(pstAttr);

            break;
        }

        case MTIOC_PQ_G_DCI_HIST:
        {
            DCI_HISTGRAM_S* pstAttr;

            pstAttr = (DCI_HISTGRAM_S*)arg;
            s32Ret = PQ_HAL_GetDCIHistgram(pstAttr);

            break;
        }
        case MTIOC_PQ_S_DCI_LEVEL:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetDCILevelGain(u32Level);

            break;
        }
        case MTIOC_PQ_G_DCI_LEVEL:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*) arg;
            s32Ret = PQ_MNG_GetDCILevelGain(pu32Level);

            break;
        }
#endif

        case MTIOC_PQ_S_SD_BRIGHTNESS:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSDBrightness(u32Level);

            break;
        }
        case MTIOC_PQ_G_SD_BRIGHTNESS:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSDBrightness(pu32Level);

            break;
        }

        case MTIOC_PQ_S_SD_CONTRAST:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSDContrast(u32Level);

            break;
        }
        case MTIOC_PQ_G_SD_CONTRAST:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSDContrast(pu32Level);

            break;
        }

        case MTIOC_PQ_S_SD_SATURATION:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSDSaturation(u32Level);

            break;
        }
        case MTIOC_PQ_G_SD_SATURATION:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSDSaturation(pu32Level);

            break;
        }

        case MTIOC_PQ_S_SD_HUE:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSDHue(u32Level);

            break;
        }
        case MTIOC_PQ_G_SD_HUE:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSDHue(pu32Level);

            break;
        }

        case MTIOC_PQ_S_HD_BRIGHTNESS:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetHDBrightness(u32Level);

            break;
        }
        case MTIOC_PQ_G_HD_BRIGHTNESS:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetHDBrightness(pu32Level);

            break;
        }

        case MTIOC_PQ_S_HD_CONTRAST:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetHDContrast(u32Level);

            break;
        }
        case MTIOC_PQ_G_HD_CONTRAST:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetHDContrast(pu32Level);

            break;
        }

        case MTIOC_PQ_S_HD_SATURATION:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetHDSaturation(u32Level);

            break;
        }
        case MTIOC_PQ_G_HD_SATURATION:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetHDSaturation(pu32Level);

            break;
        }

        case MTIOC_PQ_S_HD_HUE:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetHDHue(u32Level);

            break;
        }
        case MTIOC_PQ_G_HD_HUE:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetHDHue(pu32Level);

            break;
        }

        case MTIOC_PQ_S_NR:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            //s32Ret = DRV_PQ_SetNRLevel(u32Level);

            break;
        }

        case MTIOC_PQ_G_NR:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            //s32Ret = DRV_PQ_GetNRLevel(pu32Level);

            break;
        }

        case MTIOC_PQ_S_SHARPNESS:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSharpness(u32Level);

            break;
        }
        case MTIOC_PQ_G_SHARPNESS:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSharpness(pu32Level);

            break;
        }

        case MTIOC_PQ_S_DB:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            //s32Ret = DRV_PQ_SetDeBlocking(u32Level);

            break;
        }
        case MTIOC_PQ_G_DB:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            //s32Ret = DRV_PQ_GetDeBlocking(pu32Level);

            break;
        }

        case MTIOC_PQ_S_DR:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            //s32Ret = DRV_PQ_SetDeRinging(u32Level);

            break;
        }
        case MTIOC_PQ_G_DR:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            //s32Ret = DRV_PQ_GetDeRinging(pu32Level);

            break;
        }

        case MTIOC_PQ_S_COLORGAIN:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetColorEhance(u32Level);

            break;
        }
        case MTIOC_PQ_G_COLORGAIN:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetColorEhance(pu32Level);

            break;
        }

        case MTIOC_PQ_S_FLESHTONE:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetFleshToneLevel(u32Level);

            break;
        }
        case MTIOC_PQ_G_FLESHTONE:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetFleshToneLevel(pu32Level);

            break;
        }

#if 0
        case MTIOC_PQ_S_SIXBASECOLOR:
        {
            MT_PQ_SIX_BASE_S* pstAttr;

            pstAttr = (MT_PQ_SIX_BASE_S*)arg;
            s32Ret = DRV_PQ_SetSixBaseColorLevel(pstAttr);

            break;
        }

        case MTIOC_PQ_G_SIXBASECOLOR:
        {
            SIX_BASE_COLOR_OFFSET_S* pstAttr;
            pstAttr = (SIX_BASE_COLOR_OFFSET_S*)arg;

            s32Ret = PQ_MNG_GetSixBaseColorLevel(pstAttr);

            break;
        }

        case MTIOC_PQ_S_COLOR_ENHANCE_MODE:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetColorEnhanceMode(u32Level);

            break;
        }

        case MTIOC_PQ_G_COLOR_ENHANCE_MODE:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = PQ_MNG_GetColorEnhanceMode(pu32Level);

            break;
        }
#endif

        case MTIOC_PQ_S_MODULE:
        {
            MT_PQ_MODULE_S* pstAttr;
            pstAttr = (MT_PQ_MODULE_S*)arg;

            s32Ret = DRV_PQ_SetPQModule(pstAttr->enModule, pstAttr->u32OnOff);

            break;
        }
        case MTIOC_PQ_G_MODULE:
        {
            MT_PQ_MODULE_S* pstAttr;
            pstAttr = (MT_PQ_MODULE_S*)arg;

            s32Ret = DRV_PQ_GetPQModule(pstAttr->enModule, &(pstAttr->u32OnOff));
            break;
        }

        case MTIOC_PQ_S_DEMO:
        {
            MT_PQ_DEMO_S* pstAttr;
            pstAttr = (MT_PQ_DEMO_S*)arg;

            s32Ret = DRV_PQ_SetDemoMode(pstAttr->enModule, pstAttr->bOnOff);

            break;
        }


        case MTIOC_PQ_S_SR_DEMO:
        {
            mt_u32 u32Level;

            u32Level = *(mt_u32*)arg;
            s32Ret = DRV_PQ_SetSRMode(u32Level);

            break;
        }

        case MTIOC_PQ_G_SR_DEMO:
        {
            mt_u32* pu32Level;

            pu32Level = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetSRMode(pu32Level);

            break;
        }



        case MTIOC_PQ_G_BIN_ADDR:
        {
            mt_u32* pu32Addr;

            pu32Addr = (mt_u32*)arg;
            s32Ret = DRV_PQ_GetBinPhyAddr(pu32Addr);

            break;
        }

        default:
        {
            MT_ERR_PQ("No Such IOCTL Command: %d\n", cmd);
            up(&g_stPQSemaphore);
            return -ENOIOCTLCMD;
        }
    }

    up(&g_stPQSemaphore);

    return s32Ret;
}

long PQ_Ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    return (long)mt_drv_usercopy(filp->f_path.dentry->d_inode, filp, cmd, arg, PQIoctl);
}

mt_s32 PQ_DRV_Suspend(basedev_s* pdev, pm_message_t state)
{
    return 0;
}

mt_s32 PQ_DRV_Resume(basedev_s* pdev)
{
    return 0;
}

static struct file_operations s_stPQOps =
{
    .owner          = THIS_MODULE,
    .open           = PQ_Open,
    .release        = PQ_Close,
    .unlocked_ioctl = PQ_Ioctl,
};

static baseops_s  s_stPQDrvOps =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = DRV_PQ_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = DRV_PQ_Resume,
};

static mt_device_s s_stPQDev =
{
    .owner      = THIS_MODULE,
    .minor      = UMAP_MIN_MINOR_PQ,
    .fops       = &s_stPQOps,
    .drvops     = &s_stPQDrvOps,
};

mt_s32 __init PQ_DRV_ModInit(MT_VOID)
{
    mt_s32 s32Ret;
    mt_proc_entry_t* pstPqProc = NULL;
    mt_char szPath[128] = "../bin/";

    mt_osal_snprintf(s_stPQDev.devfs_name, MT_DEVICE_NAME_MAX_LEN, "%s", UMAP_DEVNAME_PQ);
    s32Ret = mt_drv_dev_register(&s_stPQDev);
    if (s32Ret != MT_SUCCESS)
    {
        MT_ERR_DEV("PQ Device Register Fail!\n");
        return s32Ret;
    }

    pstPqProc = mt_drv_proc_add_module("pq", NULL, NULL);
    if (NULL == pstPqProc)
    {
        MT_ERR_DEV("PQ Proc Register Fail!\n");
        mt_drv_dev_unregister(&s_stPQDev);

        return s32Ret;
    }
    pstPqProc->read = DRV_PQ_ProcRead;
    pstPqProc->write = NULL;
    pstPqProc->ioctl = NULL;

#ifndef MT_MCE_SUPPORT
    s32Ret = MT_DRV_PQ_Init(szPath);
    if (s32Ret != MT_SUCCESS)
    {
        mt_drv_dev_unregister(&s_stPQDev);
        mt_drv_proc_rm_module("pq");
        MT_ERR_DEV("PQ Hal Init Fail!\n");

        return s32Ret;
    }
#endif

    return MT_SUCCESS;
}

MT_VOID __exit PQ_DRV_ModuleExit(MT_VOID)
{
#ifndef MT_MCE_SUPPORT
    MT_DRV_PQ_DeInit();
#endif
    mt_drv_proc_rm_module("pq");
    mt_drv_dev_unregister(&s_stPQDev);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

