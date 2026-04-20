/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2018, Montage Technology Co., Ltd.
 *
 * File Name      : drv_vpss_intf_k.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2018/04/12
 * Description    : Fake VPSS module driver.
 * History        :
 * 1.Date         : 2018/04/12
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "mt_osal.h"
#include "mt_reg_common.h"	//for g_pstRegCrg!!!
#include "vfmw.h"
//#include "mt_reg_vpss.h"
#include "mt_drv_vpss.h"
//#include "vpss_ctrl.h"
#include "drv_vpss_ext.h"
#include "vpss_common.h"
//#include "vpss_instance.h"
#include "mt_drv_module.h"
//#include "vpss_src.h"
/*for MT_DECLARE_MUTEX*/
#include "mt_kernel_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

//#define BUF_DBG_OUT 0
//#define BUF_DBG_IN 0

////////////////////////////////////////////////////////////////////////////////
//Fake VPASS definition
#define VPSS_FAKE_HANDLE		0
#define VPSS_FAKE_PORT_HANDLE	0

//hal_cipher.c/... related global register!
//bad code!
static MT_REG_CRG_S   g_stRegCrg = {0};				/* FIXME: Fake Registers!!! */
volatile MT_REG_CRG_S *g_pstRegCrg = &g_stRegCrg;
EXPORT_SYMBOL(g_pstRegCrg);

////////////////////////////////////////////////////////////////////////////////


//no use
#if 0
MT_DECLARE_MUTEX(g_VpssMutex);
#endif

static const mt_char    g_VpssDevName[] = "/dev/"UMAP_DEVNAME_VPSS;

static VPSS_EXPORT_FUNC_S s_VpssExportFuncs =
{
    .pfnVpssGlobalInit = MT_DRV_VPSS_GlobalInit,
    .pfnVpssGlobalDeInit = MT_DRV_VPSS_GlobalDeInit,

    .pfnVpssGetDefaultCfg = MT_DRV_VPSS_GetDefaultCfg,
    .pfnVpssCreateVpss = MT_DRV_VPSS_CreateVpss,
    .pfnVpssDestroyVpss = MT_DRV_VPSS_DestroyVpss,
    .pfnVpssSetVpssCfg = MT_DRV_VPSS_SetVpssCfg,
    .pfnVpssGetVpssCfg = MT_DRV_VPSS_GetVpssCfg,

    .pfnVpssGetDefaultPortCfg = MT_DRV_VPSS_GetDefaultPortCfg,
    .pfnVpssCreatePort = MT_DRV_VPSS_CreatePort,
    .pfnVpssDestroyPort = MT_DRV_VPSS_DestroyPort,
    .pfnVpssGetPortCfg = MT_DRV_VPSS_GetPortCfg,
    .pfnVpssSetPortCfg = MT_DRV_VPSS_SetPortCfg,
    .pfnVpssEnablePort = MT_DRV_VPSS_EnablePort,

    .pfnVpssSendCommand = MT_DRV_VPSS_SendCommand,

    .pfnVpssGetPortFrame = MT_DRV_VPSS_GetPortFrame,
    .pfnVpssRelPortFrame = MT_DRV_VPSS_RelPortFrame,

    .pfnVpssGetPortBufListState = MT_DRV_VPSS_GetPortBufListState,
    .pfnVpssCheckPortBufListFul = MT_NULL,

    .pfnVpssSetSourceMode = MT_DRV_VPSS_SetSourceMode,
    .pfnVpssPutImage = MT_DRV_VPSS_PutImage,
    .pfnVpssGetImage = MT_DRV_VPSS_GetImage,

    .pfnVpssRegistHook = MT_DRV_VPSS_RegistHook,

};

static mt_device_s g_VpssRegisterData;

static struct file_operations s_VpssFileOps =
{
    .owner          = THIS_MODULE,
    .open           = NULL,
    .unlocked_ioctl = NULL,
    .release        = NULL,
};

mt_s32 MT_DRV_VPSS_Suspend(basedev_s *pdev, pm_message_t state)
{
	VPSS_WARN("%s: function not implemented!\n",__FUNCTION__);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VPSS_Resume(basedev_s *pdev)
{
	VPSS_WARN("%s: function not implemented!\n",__FUNCTION__);
    return MT_SUCCESS;
}

static baseops_s  s_VpssBasicOps = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = MT_DRV_VPSS_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = MT_DRV_VPSS_Resume,
};

//Source Function Pointer
static MT_DRV_VPSS_SOURCE_FUNC_S s_VpssSrcFn = {MT_NULL};

mt_s32 MT_DRV_VPSS_Init(mt_void)
{
    mt_s32 s32Ret;

	VPSS_INFO("Fake %s\n",__FUNCTION__);

    s32Ret = mt_drv_module_register(MT_ID_VPSS,VPSS_NAME,(mt_void*)&s_VpssExportFuncs);
    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("Regist MT_ID_VPSS failed\n");
        return MT_FAILURE;
    }
//no use
#if 0
    s32Ret = VPSS_CTRL_Init();
    if (s32Ret != MT_SUCCESS)
    {
        goto VPSS_Init_UnRegist_Module;
    }

    VPSS_CTRL_SetMceFlag(MT_TRUE);
#endif
    return MT_SUCCESS;

//VPSS_Init_UnRegist_Module:
    mt_drv_module_unregister(MT_ID_VPSS);

    return MT_FAILURE;
}

mt_void MT_DRV_VPSS_Exit(mt_void)
{
    VPSS_FATAL("Can't be supported\n");
}

mt_s32 VPSS_DRV_Init(mt_void)
{
    mt_s32 s32Ret = MT_SUCCESS;

	VPSS_INFO("Fake %s\n",__FUNCTION__);

    mt_osal_snprintf(g_VpssRegisterData.devfs_name, 64, UMAP_DEVNAME_VPSS);

    g_VpssRegisterData.fops   = &s_VpssFileOps;
    g_VpssRegisterData.minor  = UMAP_MIN_MINOR_VPSS;
    g_VpssRegisterData.owner  = THIS_MODULE;
    g_VpssRegisterData.drvops = &s_VpssBasicOps;

    if (mt_drv_dev_register(&g_VpssRegisterData) < 0)
    {
        VPSS_FATAL("register VPSS failed.\n");
        return MT_FAILURE;
    }

    s32Ret = mt_drv_module_register(MT_ID_VPSS,VPSS_NAME,(mt_void*)&s_VpssExportFuncs);
    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("Regist MT_ID_VPSS failed\n");
        goto DRV_Dev_UnRegister;
    }

//no use
#if 0
    s32Ret = VPSS_CTRL_Init();
    if (s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("VPSS_CTRL_Init Failed\n");
        goto DRV_Dev_MODULE_UnRegist;
    }

    VPSS_CTRL_SetMceFlag(MT_FALSE);
#endif
    return MT_SUCCESS;

//DRV_Dev_MODULE_UnRegist:
    mt_drv_module_unregister(MT_ID_VPSS);

DRV_Dev_UnRegister:
    mt_drv_dev_unregister(&g_VpssRegisterData);

    return MT_FAILURE;

}

mt_void VPSS_DRV_Exit(mt_void)
{
	VPSS_INFO("Fake %s\n",__FUNCTION__);
//no use
#if 0
    (mt_void)VPSS_CTRL_DelInit();
#endif
    (mt_void)mt_drv_module_unregister(MT_ID_VPSS);
    (mt_void)mt_drv_dev_unregister(&g_VpssRegisterData);
}

mt_s32 __init VPSS_DRV_ModInit(mt_void)
{
    VPSS_DRV_Init();

    return MT_SUCCESS;
}

mt_void __exit VPSS_DRV_ModExit(mt_void)
{
    VPSS_DRV_Exit();
}

mt_s32 MT_DRV_VPSS_GlobalInit(mt_void)
{
//no use
#if 0
    mt_s32 s32Ret;

    s32Ret = VPSS_CTRL_Init();
    if (s32Ret == MT_FAILURE)
    {
        VPSS_FATAL("GlobalInit Error.\n");
    }
    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);

	return MT_SUCCESS;
#endif
}

mt_s32 MT_DRV_VPSS_GlobalDeInit(mt_void)
{
//no use
#if 0
    mt_s32 s32Ret;

    s32Ret = VPSS_CTRL_DelInit();
    if (s32Ret == MT_FAILURE)
    {
        VPSS_FATAL("GlobalDeInit Error.\n");
    }

    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_GetDefaultCfg(MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
//no use
#if 0
    VPSS_INST_GetDefInstCfg(pstVpssCfg);
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
#endif
    return MT_SUCCESS;
}

mt_s32  MT_DRV_VPSS_CreateVpss(MT_DRV_VPSS_CFG_S *pstVpssCfg,VPSS_HANDLE *hVPSS)
{
    VPSS_HANDLE hInst = VPSS_INVALID_HANDLE;
//no use
#if 0
    hInst = VPSS_CTRL_CreateInstance(pstVpssCfg);
#else
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	hInst = VPSS_FAKE_HANDLE;
#endif
    if(hInst != VPSS_INVALID_HANDLE && hVPSS != NULL)
    {
        *hVPSS = hInst;
        return MT_SUCCESS;
    }
    else
    {
		VPSS_ERROR("%s: invalid param!\n",__FUNCTION__);
        return MT_FAILURE;
    }
}

mt_s32  MT_DRV_VPSS_DestroyVpss(VPSS_HANDLE hVPSS)
{
//no use
#if 0
    mt_s32 s32Ret;

    s32Ret = down_interruptible(&g_VpssMutex);
    s32Ret = VPSS_CTRL_DestoryInstance(hVPSS);
	up(&g_VpssMutex);

    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_SetVpssCfg(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);
    if(pstInstance)
    {
        s32Ret = VPSS_INST_SetInstCfg(pstInstance, pstVpssCfg);
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_GetVpssCfg(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);
    if(pstInstance)
    {
        VPSS_INST_GetInstCfg(pstInstance, pstVpssCfg);
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_GetDefaultPortCfg(MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg)
{
//no use
#if 0
    VPSS_INST_GetDefPortCfg(pstVpssPortCfg);
#endif

	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
    return MT_SUCCESS;
}

mt_s32  MT_DRV_VPSS_CreatePort(VPSS_HANDLE hVPSS,MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg,VPSS_HANDLE *phPort)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(pstInstance)
    {
        VPSS_CTRL_Pause(hVPSS);
        s32Ret = VPSS_INST_CreatePort(pstInstance, pstVpssPortCfg, phPort);
        VPSS_CTRL_Resume(hVPSS);
        return s32Ret;
    }
    else
    {
        return MT_FAILURE;
    }
#else
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	if (phPort != NULL)
	{
		*phPort = hVPSS;
	}
	else
	{
		VPSS_ERROR("%s: invalid param!\n",__FUNCTION__);
	}

	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_DestroyPort(VPSS_HANDLE hPort)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);
    if(!pstInstance)
    {
        return MT_FAILURE;
    }

	VPSS_CTRL_Pause(hVPSS);
    s32Ret = VPSS_INST_DestoryPort(pstInstance, hPort);
    VPSS_CTRL_Resume(hVPSS);

    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_GetPortCfg(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance= VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    s32Ret = VPSS_INST_GetPortCfg(pstInstance, hPort,pstVpssPortCfg);

    return s32Ret;
#else
	//TODO...
	if (pstVpssPortCfg != NULL)
	{
		//only support MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE!
		pstVpssPortCfg->stBufListCfg.eBufType = MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE;
	}
	else
	{
		VPSS_ERROR("%s: invalid param!\n",__FUNCTION__);
	}

	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_SetPortCfg(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    s32Ret = VPSS_INST_CheckPortCfg(pstInstance, hPort,pstVpssPortCfg);

    if(s32Ret != MT_SUCCESS)
    {
        VPSS_FATAL("SetPortCfg Error.\n");
        return MT_FAILURE;
    }
    s32Ret = VPSS_INST_SetPortCfg(pstInstance, hPort,pstVpssPortCfg);

    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_EnablePort(VPSS_HANDLE hPort, MT_BOOL bEnable)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    //VPSS_OSAL_DownLock(&(pstInstance->stInstLock));
    s32Ret = VPSS_INST_EnablePort(pstInstance, hPort,bEnable);
    //VPSS_OSAL_UpLock(&(pstInstance->stInstLock));
    return s32Ret;
#else
	//do nothing
	VPSS_INFO("Fake %s\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_SendCommand(VPSS_HANDLE hVPSS, MT_DRV_VPSS_USER_COMMAND_E eCommand, mt_void *pArgs)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret = MT_SUCCESS;

    if (eCommand == MT_DRV_VPSS_USER_COMMAND_IMAGEREADY)
    {
        s32Ret = VPSS_CTRL_WakeUpThread();
        goto CMD_OUT;
    }

    s32Ret = down_interruptible(&g_VpssMutex);
    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        up(&g_VpssMutex);
        return MT_FAILURE;
    }

    s32Ret = VPSS_INST_ReplyUserCommand(pstInstance,eCommand,pArgs);


	up(&g_VpssMutex);

CMD_OUT:
    return s32Ret;
#else
    MT_BOOL *pbAllDone;
    MT_DRV_VPSS_PORT_AVAILABLE_S *pstAvailable;

    switch ( eCommand )
    {
        case MT_DRV_VPSS_USER_COMMAND_RESET:
		case MT_DRV_VPSS_USER_COMMAND_START:
		case MT_DRV_VPSS_USER_COMMAND_STOP:
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE:
            pbAllDone = (MT_BOOL *)pArgs;
            *pbAllDone = MT_TRUE;
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE:
			//TODO...
            pstAvailable = (MT_DRV_VPSS_PORT_AVAILABLE_S *)pArgs;
            pstAvailable->bAvailable = MT_TRUE;	//???
            break;
        case MT_DRV_VPSS_USER_COMMAND_CHANGEIP:
        default:
			VPSS_WARN("%s: cmd(%d) not implemented!\n",__FUNCTION__,eCommand);
            break;
    }
	return MT_SUCCESS;
#endif
}

/**
 * @brief post process frame info
 *  post process frame info for Display when received one frame from VDEC DRV/FW.
 *
 * @param[in][out] pstFrm frame information
 */
static MT_VOID VPSS_PostProcessFrame(MT_DRV_VIDEO_FRAME_S *pstFrm)
{
	MT_DRV_VIDEO_PRIVATE_S *pstPriv;

	//TODO...
    //pstFrm->u32AspectHeight = pstPort->stDispPixAR.u32ARh;
    //pstFrm->u32AspectWidth = pstPort->stDispPixAR.u32ARw;

	//HW VDEC YUV output is MT_DRV_PIX_FMT_NV12_TILE in fact,
	//but software code all place only supports MT_DRV_PIX_FMT_NV21,
	//so hack it to MT_DRV_PIX_FMT_NV21.
    //pstFrm->ePixFormat = pstPort->eFormat;
    //pstFrm->ePixFormat = MT_DRV_PIX_FMT_NV12_TILE;
    pstFrm->ePixFormat = MT_DRV_PIX_FMT_NV21;

    pstFrm->bProgressive = MT_TRUE;
    pstFrm->enFieldMode = MT_DRV_FIELD_ALL;

    //pstFrm->enBitWidth = pstPort->enOutBitWidth;
    pstFrm->enBitWidth = MT_DRV_PIXEL_BITWIDTH_8BIT;

	pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrm->u32Priv[0]);
	pstPriv->eOriginField = pstFrm->enFieldMode;

	if(/*pstPort->b3Dsupport &&*/ (pstFrm->eFrmType != MT_DRV_FT_NOT_STEREO))
	{
		pstFrm->eFrmType = MT_DRV_FT_FPK;
	}
	else
	{
		pstFrm->eFrmType = MT_DRV_FT_NOT_STEREO;
	}

	pstFrm->u32Circumrotate = 0;
	pstFrm->bToFlip_V = 0;

	//:TODO:先简单实现stLbxInfo
	pstFrm->stLbxInfo.s32X = 0;
	pstFrm->stLbxInfo.s32Y = 0;
    //pstFrm->stLbxInfo.s32Width  = u32DstW;
    //pstFrm->stLbxInfo.s32Height = u32DstH;
	pstFrm->stLbxInfo.s32Width	= pstFrm->u32Width;
	pstFrm->stLbxInfo.s32Height = pstFrm->u32Height;

	/* 填充地址信息 */
	//TODO...
}

mt_s32  MT_DRV_VPSS_GetPortFrame(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if (!pstInstance)
    {
        return MT_FAILURE;
    }

    s32Ret = VPSS_INST_GetPortFrame(pstInstance,hPort,pstVpssFrame);

    if (s32Ret == MT_SUCCESS)
    {
        #if BUF_DBG_OUT
            MT_PRINT("%s h %#x Get %d addr %#x\n",
                    __func__,
                    hPort,
                    pstVpssFrame->u32FrameIndex,
                    pstVpssFrame->stBufAddr[0].u32PhyAddr_Y);
        #endif
        VPSS_INFO("\n Port = %d GetPortFrame %d Success",hPort,pstVpssFrame->u32FrameIndex);
	}
    else
    {
        VPSS_INFO("\n Port = %d GetPortFrame Failed",hPort);
    }
    return s32Ret;
#else
    mt_s32 s32Ret = MT_FAILURE;
	if (s_VpssSrcFn.VPSS_GET_SRCIMAGE != NULL && pstVpssFrame != NULL)
	{
		//call VDEC DRV to get image directly
		s32Ret = s_VpssSrcFn.VPSS_GET_SRCIMAGE(hPort, pstVpssFrame);

		if (s32Ret == MT_SUCCESS)
		{
			VPSS_PostProcessFrame(pstVpssFrame);
		}
	}
	else
	{
		VPSS_ERROR("%s: invalid param (%p, %p)\n",__FUNCTION__,
				s_VpssSrcFn.VPSS_GET_SRCIMAGE, pstVpssFrame);
	}

	return s32Ret;
#endif
}

mt_s32  MT_DRV_VPSS_RelPortFrame(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }
    #if BUF_DBG_IN
        MT_PRINT("%s h %#x Rel %d addr %#x\n",
                    __func__,
                    hPort,
                    pstVpssFrame->u32FrameIndex,
                    pstVpssFrame->stBufAddr[0].u32PhyAddr_Y);
    #endif
    s32Ret = VPSS_INST_RelPortFrame(pstInstance,hPort,pstVpssFrame);

    if(s32Ret != MT_SUCCESS)
    {
        //printk("\n WWWW 6666 Port = %d RelPortFrame %d Failed\n",hPort,pstVpssFrame->u32FrameIndex);//yihua error
    }
    else
    {
        //printk("\n  WWWW 7777 Port = %d RelPortFrame u32FrameIndex = %d Success \n",hPort,pstVpssFrame->u32FrameIndex);//yihua error
    }

    return s32Ret;
#else
	//do nothing
	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_RegistHook(VPSS_HANDLE hVPSS, mt_handle hDst, PFN_VPSS_CALLBACK pfVpssCallback)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }
    s32Ret = VPSS_INST_SetCallBack(pstInstance, hDst, pfVpssCallback);

    return s32Ret;
#else
	VPSS_WARN("%s: function not implemented!\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32 MT_DRV_VPSS_PutImage(VPSS_HANDLE hVPSS,MT_DRV_VIDEO_FRAME_S *pstImage)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    return VPSS_SRCIN_SendImage(&pstInstance->stSrcIn, pstImage);
#else
	VPSS_WARN("%s: function not implemented!\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32 MT_DRV_VPSS_GetImage(VPSS_HANDLE hVPSS,MT_DRV_VIDEO_FRAME_S *pstImage)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    return VPSS_SRCIN_CallImage(&pstInstance->stSrcIn, pstImage);
#else
	VPSS_WARN("%s: function not implemented!\n",__FUNCTION__);
	return MT_SUCCESS;
#endif
}

mt_s32 MT_DRV_VPSS_SetSourceMode(VPSS_HANDLE hVPSS,
                          MT_DRV_VPSS_SOURCE_MODE_E eSrcMode,
                          MT_DRV_VPSS_SOURCE_FUNC_S* pstRegistSrcFunc)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);

    if(!pstInstance)
    {
        return MT_FAILURE;
    }

    switch(eSrcMode)
    {
        case VPSS_SOURCE_MODE_USERACTIVE:
            return VPSS_INST_SetUserActiveMode(pstInstance);
            break;
        case VPSS_SOURCE_MODE_VPSSACTIVE:
            if(pstRegistSrcFunc == MT_NULL)
            {
                VPSS_FATAL("pstRegistSrcFunc is NULL.\n");
                return MT_FAILURE;
            }
            else
            {
                pstInstance->eSrcImgMode = VPSS_SOURCE_MODE_VPSSACTIVE;
                if (pstRegistSrcFunc->VPSS_GET_SRCIMAGE == MT_NULL
                    || pstRegistSrcFunc->VPSS_REL_SRCIMAGE== MT_NULL)
                {
                    VPSS_FATAL("VPSS_GET_SRCIMAGE || VPSS_REL_SRCIMAGE is NULL.\n");
                    return MT_FAILURE;
                }
                else
                {
                    VPSS_IN_SOURCE_S stSrcInfo;
                    pstInstance->stSrcFuncs.VPSS_GET_SRCIMAGE =
                                    pstRegistSrcFunc->VPSS_GET_SRCIMAGE;
                    pstInstance->stSrcFuncs.VPSS_REL_SRCIMAGE =
                                    pstRegistSrcFunc->VPSS_REL_SRCIMAGE;

                    stSrcInfo.pfnAcqCallback = (PFN_IN_RlsCallback)pstInstance->stSrcFuncs.VPSS_GET_SRCIMAGE;
                    stSrcInfo.pfnRlsCallback = (PFN_IN_RlsCallback)pstInstance->stSrcFuncs.VPSS_REL_SRCIMAGE;
                    stSrcInfo.enMode = VPSS_SOURCE_MODE_VPSSACTIVE;
                    stSrcInfo.hSource = pstInstance->ID;
                    VPSS_IN_SetSrcMode(&(pstInstance->stInEntity), stSrcInfo);
                }
            }
            return MT_SUCCESS;
            break;
        default:
            VPSS_FATAL("SourceMode is invalid.\n");
            return MT_FAILURE;
            break;
    }
#else
	if (eSrcMode == VPSS_SOURCE_MODE_VPSSACTIVE
		&& pstRegistSrcFunc != NULL)
	{
		s_VpssSrcFn.VPSS_GET_SRCIMAGE = pstRegistSrcFunc->VPSS_GET_SRCIMAGE;
		s_VpssSrcFn.VPSS_REL_SRCIMAGE = pstRegistSrcFunc->VPSS_REL_SRCIMAGE;
	}
	else
	{
		VPSS_ERROR("%s: invalid param (%d, %p)!\n",__FUNCTION__,eSrcMode,pstRegistSrcFunc);
	}

	return MT_SUCCESS;
#endif
}

mt_s32  MT_DRV_VPSS_GetPortBufListState(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_BUFLIST_STATE_S *pstVpssBufListState)
{
//no use
#if 0
    VPSS_INSTANCE_S * pstInstance;
    mt_s32 s32Ret;
    VPSS_HANDLE hVPSS;
    hVPSS = PORTHANDLE_TO_VPSSID(hPort);

    pstInstance = VPSS_CTRL_GetInstance(hVPSS);
    if(pstInstance)
    {
        s32Ret = VPSS_INST_GetPortListState(pstInstance,hPort,pstVpssBufListState);
        return s32Ret;
    }
    else
    {
        return MT_FAILURE;
    }
#else
	//TODO...
	if (pstVpssBufListState != NULL)
	{
		pstVpssBufListState->u32TotalBufNumber 	= 0;
		pstVpssBufListState->u32FulBufNumber 	= 0;
		VPSS_VERB("Fake %s: u32TotalBufNumber %u, u32FulBufNumber %u\n",__FUNCTION__,
				pstVpssBufListState->u32TotalBufNumber,
				pstVpssBufListState->u32FulBufNumber);
	}
	else
	{
		VPSS_ERROR("%s: invalid param!\n",__FUNCTION__);
	}

	return MT_SUCCESS;
#endif
}

EXPORT_SYMBOL(MT_DRV_VPSS_GlobalInit);
EXPORT_SYMBOL(MT_DRV_VPSS_GlobalDeInit);
EXPORT_SYMBOL(MT_DRV_VPSS_GetDefaultCfg);
EXPORT_SYMBOL(MT_DRV_VPSS_CreateVpss);
EXPORT_SYMBOL(MT_DRV_VPSS_DestroyVpss);

EXPORT_SYMBOL(MT_DRV_VPSS_GetDefaultPortCfg);
EXPORT_SYMBOL(MT_DRV_VPSS_CreatePort);
EXPORT_SYMBOL(MT_DRV_VPSS_EnablePort);
EXPORT_SYMBOL(MT_DRV_VPSS_DestroyPort);
EXPORT_SYMBOL(MT_DRV_VPSS_GetPortCfg);
EXPORT_SYMBOL(MT_DRV_VPSS_SetPortCfg);
EXPORT_SYMBOL(MT_DRV_VPSS_RegistHook);
EXPORT_SYMBOL(MT_DRV_VPSS_GetPortFrame);
EXPORT_SYMBOL(MT_DRV_VPSS_RelPortFrame);
EXPORT_SYMBOL(MT_DRV_VPSS_SendCommand);
EXPORT_SYMBOL(MT_DRV_VPSS_SetSourceMode);
EXPORT_SYMBOL(MT_DRV_VPSS_PutImage);
EXPORT_SYMBOL(MT_DRV_VPSS_GetImage);


#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

