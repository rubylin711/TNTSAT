
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
 ******************************************************************************
 File Name      : drv_window.h
Version          : Initial Draft
Author          : Montage multimedia software group
Created          : 2017/01/30
Last Modified :
Description   :
Function List :
History          :
 ******************************************************************************/
//#include "MT_DF_disp.h"
//#include "MT_DF_PreScale.h"
#include "vo_fw.h"

#include "drv_display.h"

#include "drv_window.h"
#include "drv_win_policy.h"
#include "drv_win_priv.h"
#include "mt_drv_sys.h"
#include "mt_drv_stat.h"
//#include "drv_vdec_ext.h"
#include "drv_disp_hal.h"
//#include "drv_disp_alg_service.h"
#include "mt_drv_module.h"
#include "drv_pq_ext.h"
#include "drv_disp_priv.h"
#include "mt_module_debug.h"

#include "hd_enc_aria_reg.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "drv_disp_Symphony_reg.h"

DEFINE_SPINLOCK(g_threadIsr_Lock);


/******************************************************************************
  global object
 ******************************************************************************/
static volatile mt_s32 s_s32WindowGlobalFlag = WIN_DEVICE_STATE_CLOSE;
static DISPLAY_WINDOW_S stDispWindow;
static VIRTUAL_WINDOW_S stVirWindow;
static MT_DRV_VIDEO_FRAME_S stDrvVideo;
IMAGE g_testImagInfo;
MT_BOOL b_disp_coeff_update = 0;
WINDOW_S *g_pstWin[MAX_WIN_NUM];

atomic_t bCFGVideoFlag[MAX_WIN_NUM];
atomic_t bISRProcessFlag;
atomic_t bCleanFifoFlag;
/* Flush Display type
 * flush FIFO, or Display, or both of them
 */
MT_DRV_WIN_FLUSH_TYPE_E g_FlushDispType;

DF_DRV_SETTING_S g_stDrvSetting[MAX_WIN_NUM];


//maximum size the freeze buffer needed to retain tile format frame data
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#define MAX_FREEZE_BUFFER_SIZE 0x380000
#elif defined CONFIG_MT_CHIP_SYMPHONY2
#define MAX_FREEZE_BUFFER_SIZE 0x418000
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) 
#define MAX_FREEZE_BUFFER_SIZE 0x454000
#elif defined(CONFIG_MT_CHIP_SYMPHONY6) //add CONFIG_MT_CHIP_SYMPHONY6 config tmp for sym6 compile
#define MAX_FREEZE_BUFFER_SIZE 0x400000
#elif defined CONFIG_MT_CHIP_ARIA
#define MAX_FREEZE_BUFFER_SIZE ((MT_UHD_WIDTH_LARGEST*MT_UHD_HEIGHT*2))
#endif


static unsigned int freeze_buf_addr = 0;
static phys_addr_t g_freeze_buf_addr = 0;
module_param(freeze_buf_addr, uint, S_IRUGO | S_IWUSR);

int fcnt_vir;
int rcnt_vir;
int rls_cnt_vir =0;
//hmc
//FIXME: how about multi-thread?
MT_DF_VIDEO_FRAME_S g_stVideoFrame[MAX_WIN_NUM]; //used for display queue frame

//Added to avoid stack overflow!
//FIXME: how about multi-thread?
static MT_DRV_VIDEO_FRAME_S g_stRlsFrm[MAX_WIN_NUM];

extern disp_priv_t *get_disp_priv_handle(void);

#if defined(DAC_TYPE_SYNOPSYS)

typedef struct hiDRV_GAMMA_S
{
    mt_u32 u32Gamma;
    mt_u32 u32ParaIndex;
    MT_BOOL bGammaChange;
} MT_DRV_GAMMA_S;

#define GAMMA_UP_DELAY 4
#define GAMMA_PARA_NO_USE 3
MT_DRV_GAMMA_S g_stGamma[MT_DRV_DISPLAY_BUTT] =
{
    { 0, GAMMA_PARA_NO_USE, MT_FALSE },
    { 0, GAMMA_PARA_NO_USE, MT_FALSE },
    { 0, GAMMA_PARA_NO_USE, MT_FALSE }
};
#endif

mt_u32 Win_GetVideoWinIndex(mt_void)
{
    mt_u32 index = 0;
    //get video layer window index
    for(index = 0; index < MAX_WIN_NUM; index++)
    {
        if(g_pstWin[index] != NULL && (0 == g_pstWin[index]->stCfg.stAttr.bUseSubLayer))
        {
            break;
        }
    } 
    return index;
}

mt_u32 Win_GetStillWinIndex(mt_void)
{
    mt_u32 index = 0;
    //get video layer window index
    for(index = 0; index < MAX_WIN_NUM; index++)
    {
        if(g_pstWin[index] != NULL && (1 == g_pstWin[index]->stCfg.stAttr.bUseSubLayer))
        {
            break;
        }
    } 
    return index;
}

MT_BOOL WinGetRegWinManageStatus(mt_void)
{
    return stDispWindow.bWinManageStatus;
}

mt_void WinSetRegWinManageStatus(MT_BOOL bRegStatus)
{
    stDispWindow.bWinManageStatus = bRegStatus;
    return;
}

VIRTUAL_S *WinGetVirWindow(mt_u32 u32WinIndex)
{

    if (!stVirWindow.u32WinNumber)
    {
        WIN_WARN("Not found this window!\n");
        return MT_NULL;
    }

    if (WinGetPrefix(u32WinIndex) != WIN_INDEX_PREFIX)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    if (WinGetDispId(u32WinIndex) != WIN_INDEX_VIRTUAL_CHANNEL)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    if (WinGetId(u32WinIndex) >= WIN_VIRTAUL_MAX_NUMBER)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    return stVirWindow.pstWinArray[WinGetId(u32WinIndex)];
}

mt_void ISR_CallbackForDispModeChange(mt_handle hDst, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
mt_void ISR_CallbackForWinProcess(mt_handle hDst, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo);
mt_void WinUpdateDispInfo(WINDOW_S *pstWin, MT_DISP_DISPLAY_INFO_S *pstDsipInfo);
mt_void WinUpdateDispInfo_InitialFmt(WINDOW_S *pstWin, MT_DISP_DISPLAY_INFO_S *pstDsipInfo);

/******************************************************************************
  Win device function
 ******************************************************************************/

mt_s32 Check_WinFunc(WINDOW_S *pWin)
{
#if 0
    if (!pWin)
        return MT_ERR_VO_NULL_PTR;

    if ((!pWin->stVLayerFunc.PF_ReleaseLayer)
            ||(!pWin->stVLayerFunc.PF_SetEnable)
            ||(!pWin->stVLayerFunc.PF_Update)
            ||(!pWin->stVLayerFunc.PF_VP0ParaUpd)
            ||(!pWin->stVLayerFunc.PF_AcquireLayerByDisplay)
            ||(!pWin->stVLayerFunc.PF_MovBottom)
            ||(!pWin->stVLayerFunc.PF_MovTop)
            ||(!pWin->stVLayerFunc.PF_AcquireLayerByDisplay)
       )
    {
        return MT_ERR_VO_NO_INIT;

    }
    else
    {
        return MT_SUCCESS;
    }
#endif
    return MT_SUCCESS;
}

extern MT_BOOL LayerEnable[];
mt_s32 WinUpdatPara(WINDOW_S *pWin)
{
#if 0
    if (MT_SUCCESS == Check_WinFunc(pWin))
    {

#if defined(DAC_TYPE_SYNOPSYS)
        if ((!g_stGamma[MT_DRV_DISPLAY_0].u32Gamma) && (!g_stGamma[MT_DRV_DISPLAY_1].u32Gamma) )
#endif
        {
            pWin->stVLayerFunc.PF_VP0ParaUpd(pWin->u32VideoLayer);
        }

        pWin->stVLayerFunc.PF_Update(pWin->u32VideoLayer);
        return MT_SUCCESS;
    }
#endif
    if(!LayerEnable[DISP_LAYER_ID_VIDEO_HD] && (drv_reg_4k_disp_get_video_ctrl_1_video_sel() != LayerEnable[DISP_LAYER_ID_VIDEO_HD]))
    {
        drv_reg_4k_disp_set_video_ctrl_1_video_sel(0);
    }

    if(!LayerEnable[DISP_LAYER_ID_STILL_HD] && (drv_reg_4k_disp_get_still_control_still_select() != LayerEnable[DISP_LAYER_ID_STILL_HD]))
    {
        drv_reg_4k_disp_set_still_control_still_select(0);
    }

    return MT_SUCCESS;
}

mt_void WinDisableAllLayerRegion(MT_DRV_DISPLAY_E enDisp)
{
#if 0
    mt_u32 i = 0;
    WINDOW_S *pstWin = MT_NULL;

    /*because video-window remmaping, so make a status resetting.*/
    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        pstWin = stDispWindow.pstWinArray[enDisp][i];
        if ((pstWin) && (pstWin->u32VideoLayer != VDP_LAYER_VID_BUTT))
        {
            pstWin->stVLayerFunc.PF_SetEnable(pstWin->u32VideoLayer, pstWin->u32VideoRegionNo, MT_FALSE);
            //pstWin->stVLayerFunc.PF_Update(pstWin->u32VideoLayer);
            WinUpdatPara(pstWin);
        }

        /*set window's layer invalid.*/
        if (pstWin)
            pstWin->u32VideoLayer = VDP_LAYER_VID_BUTT;
    }
#endif
    return;
}

mt_void WinLoadNewMapping(MT_DRV_DISPLAY_E enDisp)
{
#if 0
    mt_u32 i = 0;

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if (stDispWindow.pstWinArray[enDisp][i]
                && (stDispWindow.pstWinArray[enDisp][i]->bEnable))
        {
            stDispWindow.pstWinArray[enDisp][i]->u32VideoLayer
                = stDispWindow.pstWinArray[enDisp][i]->u32VideoLayerNew;

            stDispWindow.pstWinArray[enDisp][i]->u32VideoRegionNo
                = stDispWindow.pstWinArray[enDisp][i]->u32VideoRegionNoNew;
        }
        else if (stDispWindow.pstWinArray[enDisp][i]
                && (!stDispWindow.pstWinArray[enDisp][i]->bEnable))
        {
            stDispWindow.pstWinArray[enDisp][i]->u32VideoLayer
                = VDP_LAYER_VID_BUTT;
            stDispWindow.pstWinArray[enDisp][i]->u32VideoRegionNo
                = VDP_LAYER_VID_BUTT;
        }
    }
#endif
    return;
}

mt_void ISR_CallbackForWinManage(mt_handle hDst, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
#if 0
    mt_u32 i = 0;
    WINDOW_S *pWin = MT_NULL;

    if (spin_trylock(&g_threadIsr_Lock))
    {
        /*judge whether the internal hareware-cfg is changed*/
        if (atomic_read(&stDispWindow.bWindowSytemUpdate[hDst]))
        {
            /*video-window remmaping may occurs, so make a status resetting.
             *and load new mapping.*/
            WinDisableAllLayerRegion(hDst);
            WinLoadNewMapping(hDst);

            /*to adjust the order, becasue win-layermapping changed.*/
            for (i = 0; i < WINDOW_MAX_NUMBER; i++)
            {
                /*check wether the layer is set  or not.*/
                pWin = stDispWindow.pstWinArray[hDst][i];
                if( (pWin) && (!pWin->stVLayerFunc.PF_ChckLayerInit(pWin->u32VideoLayer)))
                    (mt_void)pWin->stVLayerFunc.PF_SetDefault(pWin->u32VideoLayer);

                if ((pWin) && (pWin->stWinLayerOpt.bEffective)
                        && (pWin->stWinLayerOpt.layerOptType == LAYER_OPT_ZORDER_ADJUST))
                {
                    if (((mt_u32)pWin->stWinLayerOpt.pParam) == MT_DRV_DISP_ZORDER_MOVETOP)
                    {

                        pWin->stVLayerFunc.PF_MovTop(pWin->u32VideoLayer);
                    }
                    else if (((mt_u32)pWin->stWinLayerOpt.pParam) == MT_DRV_DISP_ZORDER_MOVEBOTTOM)
                    {
                        pWin->stVLayerFunc.PF_MovBottom(pWin->u32VideoLayer);
                    }

                    /*clear the flag, to avoid taking effect next period.*/
                    pWin->stWinLayerOpt.bEffective = MT_FALSE;
                    pWin->stWinLayerOpt.layerOptType = LAYER_OPT_BUTT;
                    pWin->stWinLayerOpt.pParam = MT_NULL;
                }
                if (pWin)
                {
                    atomic_set(&pWin->stCfg.bNewAttrFlag, 1);
                    pWin->bDispInfoChange = MT_TRUE;
                }
            }

            atomic_set(&stDispWindow.bWindowSytemUpdate[hDst], 0);
        }

        spin_unlock(&g_threadIsr_Lock);
    }
#endif
    return;
}

mt_s32 WinRegWinManageCallback(MT_DRV_DISPLAY_E enDisp)
{
    MT_DRV_DISP_CALLBACK_S stCB;
    mt_s32 nRet = MT_SUCCESS;

    stCB.hDst = (mt_handle)enDisp;
    stCB.pfDISP_Callback = ISR_CallbackForWinManage;
    nRet = DISP_RegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);
    if (nRet)
    {
        WIN_WARN("WIN register callback failed in %s!\n", __FUNCTION__);
    }

    return nRet;
}

mt_s32 WinUnRegWinManageCallback(MT_DRV_DISPLAY_E enDisp)
{
    MT_DRV_DISP_CALLBACK_S stCB;
    mt_s32 nRet = MT_SUCCESS;

    stCB.hDst = (mt_handle)enDisp;
    stCB.pfDISP_Callback = ISR_CallbackForWinManage;
    nRet = DISP_UnRegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);
    if (nRet)
    {
        WIN_ERROR("WIN unregister callback failed in %s!\n", __FUNCTION__);
    }
    return nRet;
}

mt_s32 WIN_DestroyStillFrame(MT_DRV_VIDEO_FRAME_S *pstReleaseFrame)
{
    mt_u32 i;
    WIN_RELEASE_FRM_S *pstWinRelFrame = &stDispWindow.stWinRelFrame;

    for (i = 0; i < MAX_RELEASE_NO; i++)
    {
        if (!pstWinRelFrame->pstNeedRelFrmNode[i])
        {
            pstWinRelFrame->pstNeedRelFrmNode[i] = pstReleaseFrame;
            pstWinRelFrame->enThreadEvent = EVENT_RELEASE;
            //printk(" wake up !\n");
            wake_up(&pstWinRelFrame->stWaitQueHead);
            return MT_SUCCESS;
        }
    }

    WIN_ERROR("Release still frame failed ,buff is full. %s!\n", __FUNCTION__);
    return MT_FAILURE;
}
mt_s32 WinReleaseFrameThreadProcess(mt_void *pArg)
{
    mt_u32 i;
    WIN_RELEASE_FRM_S *pstWinRelFrame = pArg;

    /*if stop refush release frame buffer*/
    while (!kthread_should_stop())
    {
        for (i = 0; i < MAX_RELEASE_NO; i++)
        {
            if (pstWinRelFrame->pstNeedRelFrmNode[i])
            {
                if (pstWinRelFrame->pstNeedRelFrmNode[i]->bStillFrame)
                {
                    WinReleaseStillFrame(pstWinRelFrame->pstNeedRelFrmNode[i]);
                    pstWinRelFrame->pstNeedRelFrmNode[i] = MT_NULL;
                }
            }
        }

        pstWinRelFrame->enThreadEvent = EVENT_BUTT;

        wait_event_hrtimeout(pstWinRelFrame->stWaitQueHead, (EVENT_RELEASE == pstWinRelFrame->enThreadEvent), ms_to_ktime(1000));
    }

    return MT_SUCCESS;
}

mt_s32 WinCreatReleaseFrameThread(mt_void)
{
    WIN_RELEASE_FRM_S *pstWinRelFrame = &stDispWindow.stWinRelFrame;

    memset(pstWinRelFrame, 0, sizeof(WIN_RELEASE_FRM_S));
    pstWinRelFrame->hThread =
        kthread_create(WinReleaseFrameThreadProcess, (mt_void *)(&stDispWindow.stWinRelFrame), "MT_WIN_ReleaseFrameProcess");

    init_waitqueue_head(&(pstWinRelFrame->stWaitQueHead));
    if (IS_ERR(pstWinRelFrame->hThread))
    {
        WIN_FATAL("Can not create thread.\n");
        return MT_ERR_VO_CREATE_ERR;
    }

    wake_up_process(pstWinRelFrame->hThread);
    return MT_SUCCESS;
}

mt_s32 WinDestroyReleaseFrameThread(mt_void)
{
    mt_s32 s32Ret;
    mt_s32 s32Times = 0;
    WIN_RELEASE_FRM_S *pstWinRelFrame = &stDispWindow.stWinRelFrame;

    /*reflush release buffer */
    pstWinRelFrame->enThreadEvent = EVENT_RELEASE;
    wake_up(&pstWinRelFrame->stWaitQueHead);

    for (s32Times = 0; s32Times < 10; s32Times++)
    {
        if (EVENT_BUTT == pstWinRelFrame->enThreadEvent)
        {
            pstWinRelFrame->enThreadEvent = EVENT_RELEASE;
            s32Ret = kthread_stop(pstWinRelFrame->hThread);

            if (s32Ret != MT_SUCCESS)
            {
                WIN_FATAL("Destory Thread Error.\n");
            }
            return MT_SUCCESS;
        }
        msleep(100);
    }

    return MT_FAILURE;
}

mt_s32 WIN_Init(mt_void)
{
    MT_BOOL bDispInitFlag;
    mt_s32 s32Ret;

    if (WIN_DEVICE_STATE_CLOSE != s_s32WindowGlobalFlag)
    {
        WIN_INFO("VO has been inited!\n");
        return MT_SUCCESS;
    }

    DISP_GetInitFlag(&bDispInitFlag);
    if (MT_TRUE != bDispInitFlag)
    {
        WIN_ERROR("Display is not inited! %s\n", __FUNCTION__);
        return MT_ERR_VO_DEPEND_DEVICE_NOT_READY;
    }

    if (MT_SUCCESS != BP_CreateBlackFrame())
    {
        WIN_ERROR("Create Black Frame failed!\n");
        return MT_ERR_VO_MALLOC_FAILED;
    }

    DISP_MEMSET(&stDispWindow, 0, sizeof(DISPLAY_WINDOW_S));
#if 0 //disp_hal
    VideoLayer_Init();
#endif

    DISP_MEMSET(&stVirWindow, 0, sizeof(VIRTUAL_WINDOW_S));
    s32Ret = WinCreatReleaseFrameThread();
    if (MT_SUCCESS != s32Ret)
    {
        WIN_ERROR("win Create Release Frame Thread failed! %s\n", __FUNCTION__);
        return MT_ERR_VO_MALLOC_FAILED;
    }
    s_s32WindowGlobalFlag = WIN_DEVICE_STATE_OPEN;

    return MT_SUCCESS;
}

mt_s32 WIN_DeInit(mt_void)
{
    mt_s32 i, j, s32Ret;
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);

    if (WIN_DEVICE_STATE_CLOSE == s_s32WindowGlobalFlag)
    {
        WIN_INFO("VO is not inited!\n");
        return MT_SUCCESS;
    }

    /*close all the windows.*/
    for (i = 0; i < MT_DRV_DISPLAY_BUTT; i++)
    {
        stDispWindow.u32WinNumber[i] = 0;

        for (j = 0; j < WINDOW_MAX_NUMBER; j++)
        {
            if (stDispWindow.pstWinArray[i][j])
            {
                WIN_Destroy(stDispWindow.pstWinArray[i][j]->u32Index);
                stDispWindow.pstWinArray[i][j] = MT_NULL;
            }
        }
    }

    /*close all the virtual windows.*/
    for (i = 0; i < MT_DRV_DISPLAY_BUTT; i++)
    {
        if (stVirWindow.pstWinArray[i])
        {
            WIN_VIR_Destroy(stVirWindow.pstWinArray[i]);
            stVirWindow.pstWinArray[i] = MT_NULL;
        }
    }

    stVirWindow.u32WinNumber = 0;

    if (WinGetRegWinManageStatus())
    {
        WinUnRegWinManageCallback(MT_DRV_DISPLAY_1);
        WinUnRegWinManageCallback(MT_DRV_DISPLAY_0);
        WinSetRegWinManageStatus(MT_FALSE);
    }

    s32Ret = WinDestroyReleaseFrameThread();
    if (MT_SUCCESS != s32Ret)
    {
        WIN_ERROR("win Destroy Release Frame Thread failed!\n");
        //return MT_ERR_VO_MALLOC_FAILED;
    }
#if 0 //disp_hal
    VideoLayer_DeInit();
#endif

    BP_DestroyBlackFrame();
    s_s32WindowGlobalFlag = WIN_DEVICE_STATE_CLOSE;

    WIN_INFO("VO has been DEinited!\n");
    return MT_SUCCESS;
}

WINDOW_S *WinGetWindow(mt_u32 u32WinIndex);
#if 0
static mt_s32 WIN_SetWinState(mt_void)
{
    WINDOW_S *pstWin = MT_NULL;
    mt_u32 i = 0, j = 0;

    /*close all the windows.*/
    for (i = 0; i < MT_DRV_DISPLAY_BUTT; i++)
    {
        for (j = 0; j < WINDOW_MAX_NUMBER; j++)
        {
            if (stDispWindow.pstWinArray[i][j])
            {
                WinCheckWindow(stDispWindow.pstWinArray[i][j]->u32Index, pstWin);
                pstWin->bNeedResume = MT_TRUE;
            }
        }
    }

    return MT_SUCCESS;
}
#endif

mt_s32 WIN_Suspend(mt_void)
{
    WinCheckDeviceOpen();

    s_s32WindowGlobalFlag = WIN_DEVICE_STATE_SUSPEND;

    return MT_SUCCESS;
}

mt_s32 WIN_Resume(mt_void)
{
#if 0
    (mt_void)WIN_SetWinState();

    if (s_s32WindowGlobalFlag == WIN_DEVICE_STATE_SUSPEND)
    {
        VIDEO_LAYER_FUNCTIONG_S *pF = VideoLayer_GetFunctionPtr();

        s_s32WindowGlobalFlag = WIN_DEVICE_STATE_OPEN;
        if (pF)
            pF->PF_SetAllLayerDefault();
    }
#endif
    return MT_SUCCESS;
}

mt_s32 WinTestAddWindow(MT_DRV_DISPLAY_E enDisp)
{
    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        WIN_ERROR("Invalid Disp = 0x%x\n", enDisp);
        return MT_ERR_VO_INVALID_PARA;
    }

#ifdef MT_GFX_USE_G3

    /*DTS2013110807439
3716cv200 :g3 and v4 can not be used at the same time.
Attach mode : GFX DISP0 channel is writeback form DISP1,so  G3 is not use.
Detach mode : GFX DISP0  G3 is  used.
*/
    if (!DISP_IsSameSource(enDisp) && MT_DRV_DISPLAY_0 == enDisp)
    {
        if (stDispWindow.u32WinNumber[MT_DRV_DISPLAY_0] >= 1)
            return MT_ERR_VO_WIN_UNSUPPORT;
    }
#endif

    if (stDispWindow.u32WinNumber[enDisp] < WINDOW_MAX_NUMBER)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_VO_INVALID_OPT;
    }
}

mt_u32 WinGetPrefix(mt_u32 u32WinIndex)
{
    return (mt_u32)(u32WinIndex & WIN_INDEX_PREFIX_MASK);
}

mt_u32 WinGetDispId(mt_u32 u32WinIndex)
{
    return (mt_u32)((u32WinIndex >> WIN_INDEX_DISPID_SHIFT_NUMBER) & WIN_INDEX_DISPID_MASK);
}

mt_u32 WinGetId(mt_u32 u32WinIndex)
{
    return (mt_u32)(u32WinIndex & WINDOW_INDEX_NUMBER_MASK);
}

mt_u32 WinMakeVirIndex(mt_u32 u32WinIndex)
{
    /*
     *    0x12               34                         56                78
     *         WIN_INDEX_PREFIX  WIN_INDEX_VIRTUAL_CHANNEL   u32WinIndex
     *
     */
    return (mt_u32)(WIN_INDEX_PREFIX | ((WIN_INDEX_VIRTUAL_CHANNEL & WIN_INDEX_DISPID_MASK)
                << WIN_INDEX_DISPID_SHIFT_NUMBER) |
            (u32WinIndex & WINDOW_INDEX_NUMBER_MASK));
}

mt_u32 WinMakeIndex(MT_DRV_DISPLAY_E enDisp, mt_u32 u32WinIndex)
{
    return (mt_u32)((WIN_INDEX_PREFIX) | (((mt_u32)enDisp & WIN_INDEX_DISPID_MASK)
                << WIN_INDEX_DISPID_SHIFT_NUMBER) |
            (u32WinIndex & WINDOW_INDEX_NUMBER_MASK));
}

mt_u32 WinGetIndex(mt_handle hWin, MT_DRV_DISPLAY_E *enDisp, mt_u32 *u32WinIndex)
{

    *enDisp = (hWin & 0xff00) >> WIN_INDEX_DISPID_SHIFT_NUMBER;
    *u32WinIndex = hWin & 0xff;

    return MT_SUCCESS;
}

mt_s32 WinAddVirWindow(VIRTUAL_S *pstWin)
{
    mt_s32 i;

    for (i = 0; i < WIN_VIRTAUL_MAX_NUMBER; i++)
    {
        if (!stVirWindow.pstWinArray[i])
        {
            pstWin->u32Index = WinMakeVirIndex((mt_u32)i);
            stVirWindow.pstWinArray[i] = pstWin;
            stVirWindow.u32WinNumber++;

            return MT_SUCCESS;
        }
    }
    return MT_ERR_VO_CREATE_ERR;
}

mt_s32 WinDelVirWindow(mt_u32 u32WinIndex)
{
    if (WinGetPrefix(u32WinIndex) != WIN_INDEX_PREFIX)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_FAILURE;
    }

    if (WinGetDispId(u32WinIndex) != WIN_INDEX_VIRTUAL_CHANNEL)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_FAILURE;
    }

    if (WinGetId(u32WinIndex) >= WIN_VIRTAUL_MAX_NUMBER)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_FAILURE;
    }

    if (!stVirWindow.u32WinNumber)
    {
        WIN_ERROR("Not found this window!\n");
        return MT_FAILURE;
    }

    if (stVirWindow.pstWinArray[WinGetId(u32WinIndex)])
    {
        stVirWindow.pstWinArray[WinGetId(u32WinIndex)] = MT_NULL;
        stVirWindow.u32WinNumber--;
    }
    else
    {
        WIN_ERROR("Not found this window!\n");
    }

    return MT_SUCCESS;
}

mt_s32 WinAddWindow(MT_DRV_DISPLAY_E enDisp, WINDOW_S *pstWin)
{
    mt_s32 i;

    if (enDisp >= MT_DRV_DISPLAY_BUTT)
    {
        return MT_ERR_VO_CREATE_ERR;
    }

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if (!stDispWindow.pstWinArray[(mt_u32)enDisp][i])
        {
            pstWin->u32Index = WinMakeIndex(enDisp, (mt_u32)i);

            stDispWindow.pstWinArray[(mt_u32)enDisp][i] = pstWin;
            stDispWindow.u32WinNumber[(mt_u32)enDisp]++;

            return MT_SUCCESS;
        }
    }

    return MT_ERR_VO_CREATE_ERR;
}

mt_s32 WinDelWindow(mt_u32 u32WinIndex)
{
    if (WinGetPrefix(u32WinIndex) != WIN_INDEX_PREFIX)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (WinGetDispId(u32WinIndex) >= MT_DRV_DISPLAY_BUTT)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (WinGetId(u32WinIndex) >= WINDOW_MAX_NUMBER)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (!stDispWindow.u32WinNumber[WinGetDispId(u32WinIndex)])
    {
        WIN_ERROR("Not found this window!\n");
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (stDispWindow.pstWinArray[WinGetDispId(u32WinIndex)][WinGetId(u32WinIndex)])
    {
        stDispWindow.pstWinArray[WinGetDispId(u32WinIndex)][WinGetId(u32WinIndex)] = MT_NULL;
        stDispWindow.u32WinNumber[WinGetDispId(u32WinIndex)]--;
    }
    else
    {
        WIN_ERROR("Not found this window!\n");
    }

    return MT_SUCCESS;
}

WINDOW_S *WinGetWindow(mt_u32 u32WinIndex)
{
    if (WinGetPrefix(u32WinIndex) != WIN_INDEX_PREFIX)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    if (WinGetDispId(u32WinIndex) >= MT_DRV_DISPLAY_BUTT)
    {
        WIN_WARN("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    if (!stDispWindow.u32WinNumber[WinGetDispId(u32WinIndex)])
    {
        WIN_WARN("Not found this window!\n");
        return MT_NULL;
    }
    if (WinGetId(u32WinIndex) >= WINDOW_MAX_NUMBER)
    {
        WIN_ERROR("Invalid window index = 0x%x\n", u32WinIndex);
        return MT_NULL;
    }

    return stDispWindow.pstWinArray[WinGetDispId(u32WinIndex)][WinGetId(u32WinIndex)];
}

/******************************************************************************
  internal function
 ******************************************************************************/
mt_u32 WinParamAlignUp(mt_u32 x, mt_u32 a)
{
    if (!a)
    {
        return x;
    }
    else
    {
        return (((x + (a - 1)) / a) * a);
    }
}

mt_u32 WinParamAlignDown(mt_u32 x, mt_u32 a)
{
    if (!a)
    {
        return x;
    }
    else
    {
        return ((x / a) * a);
    }
}

mt_void window_revise(mt_rect_s *pstToBeRevisedRect_tmp,
        const mt_rect_s *ptmp_virtscreen)
{
    /*give a basic    process of width and height.*/
    if (pstToBeRevisedRect_tmp->s32Width < WIN_OUTRECT_MIN_WIDTH)
    {
        pstToBeRevisedRect_tmp->s32Width = WIN_OUTRECT_MIN_WIDTH;
    }

    /*here we delete the width max branch, because we support tc 2-screen,
     *even 3-screen display.
     * here we do not make a limit,  all the behavior depends on the zme performance.
     */

    if (pstToBeRevisedRect_tmp->s32Height < WIN_OUTRECT_MIN_HEIGHT)
    {
        pstToBeRevisedRect_tmp->s32Height = WIN_OUTRECT_MIN_HEIGHT;
    }

    return;
}

/*previously, we do division first, then do multiplication
 *so we lose precision. now we invert the  order.
 */
mt_s32 Win_Caculate(mt_s32 ratioA, mt_s32 ratioB, mt_s32 virtualValue)
{
    return ((virtualValue * ratioA) * 100) / ratioB;
}

/*this func is for both graphics and video    virtual screen deal, it's a common function.*/
mt_s32 WinOutRectSizeConversion(const mt_rect_s *pstCanvas,
        const MT_DRV_DISP_OFFSET_S *pstOffsetInfo,
        const mt_rect_s *pstFmtResolution,
        mt_rect_s *pstToBeRevisedRect,
        mt_rect_s *pstRevisedRect)
{
    mt_u32 zmeDestWidth = 0, zmeDestHeight = 0;
    MT_DRV_DISP_OFFSET_S tmp_offsetInfo;
    mt_rect_s stToBeRevisedRect_tmp = *pstToBeRevisedRect;

    tmp_offsetInfo = *pstOffsetInfo;

    /*for browse mode, revise in virt screen .*/
    window_revise(&stToBeRevisedRect_tmp, pstCanvas);

    zmeDestWidth = (pstFmtResolution->s32Width - tmp_offsetInfo.u32Left - tmp_offsetInfo.u32Right);
    zmeDestHeight = (pstFmtResolution->s32Height - tmp_offsetInfo.u32Top - tmp_offsetInfo.u32Bottom);

    if (pstCanvas->s32Width != stToBeRevisedRect_tmp.s32Width)
    {
        pstRevisedRect->s32Width = Win_Caculate(zmeDestWidth,
                pstCanvas->s32Width,
                stToBeRevisedRect_tmp.s32Width) /
            100;
    }
    else
    {
        pstRevisedRect->s32Width = zmeDestWidth;
    }

    if (pstCanvas->s32Height != stToBeRevisedRect_tmp.s32Height)
    {
        pstRevisedRect->s32Height = Win_Caculate(zmeDestHeight,
                pstCanvas->s32Height,
                stToBeRevisedRect_tmp.s32Height) /
            100;
    }
    else
    {
        pstRevisedRect->s32Height = zmeDestHeight;
    }

    pstRevisedRect->s32X = Win_Caculate(zmeDestWidth,
            pstCanvas->s32Width,
            stToBeRevisedRect_tmp.s32X) /
        100 +
        tmp_offsetInfo.u32Left;
    pstRevisedRect->s32Y = Win_Caculate(zmeDestHeight,
            pstCanvas->s32Height,
            stToBeRevisedRect_tmp.s32Y) /
        100 +
        tmp_offsetInfo.u32Top;

    pstRevisedRect->s32X = WinParamAlignUp(pstRevisedRect->s32X, 2);
    pstRevisedRect->s32Y = WinParamAlignUp(pstRevisedRect->s32Y, 2);

    pstRevisedRect->s32Width = WinParamAlignUp(pstRevisedRect->s32Width, 2);
    pstRevisedRect->s32Height = WinParamAlignUp(pstRevisedRect->s32Height, 2);

    /*for browse mode, revise in virt screen .*/
    window_revise(pstRevisedRect, pstFmtResolution);
    return MT_SUCCESS;
}

mt_s32 WinOutRectSizeConversionByType(const MT_DISP_DISPLAY_INFO_S *pstInfo,
        mt_rect_s *pstToBeRevisedRect,
        mt_rect_s *pstRevisedRect,
        WINDOW_S *pstWin)
{
    mt_rect_s stCanvas;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;

    memset((void *)&stCanvas, 0, sizeof(mt_rect_s));
    memset((void *)&stOffsetInfo, 0, sizeof(MT_DRV_DISP_OFFSET_S));

    if (pstWin->bVirtScreenMode)
    {
        (mt_void) DISP_GetVirtScreen(pstWin->enDisp, &stCanvas);
        stOffsetInfo = pstInfo->stOffsetInfo;

#ifndef MT_VO_OFFSET_EFFECTIVE_WHEN_WIN_FULL
        if (((pstWin->stCfg.stAttrBuf.stOutRect.s32Width == 0) || (pstWin->stCfg.stAttrBuf.stOutRect.s32Height == 0)) ||
                ((pstWin->stCfg.stAttrBuf.stOutRect.s32Width == stCanvas.s32Width) && (pstWin->stCfg.stAttrBuf.stOutRect.s32Height == stCanvas.s32Height)))
        {
            memset((void *)&stOffsetInfo, 0, sizeof(MT_DRV_DISP_OFFSET_S));
        }
#endif
    }
    else
    {
        stOffsetInfo.u32Left = 0;
        stOffsetInfo.u32Top = 0;
        stOffsetInfo.u32Right = 0;
        stOffsetInfo.u32Bottom = 0;
        stCanvas = pstWin->stDispInfo.stWinInitialFmt;
#if 0
        printk("111conversion: lstw:%d, lsth:%d,curw:%d, curh:%d!\n", stCanvas.s32Width,
                stCanvas.s32Height,
                pstInfo->stFmtResolution.s32Width,
                pstInfo->stFmtResolution.s32Height);
#endif
    }

#if 0
    printk("111conversion: lstw:%d, lsth:%d, offset:%d,%d,%d,%d,curw:%d, curh:%d, tow:%d, toh:%d,sedw:%d,sedh:%d!\n",
            stCanvas.s32Width,
            stCanvas.s32Height,
            stOffsetInfo.u32Left,
            stOffsetInfo.u32Top,
            stOffsetInfo.u32Right,
            stOffsetInfo.u32Bottom,
            pstInfo->stFmtResolution.s32Width,
            pstInfo->stFmtResolution.s32Height,
            pstToBeRevisedRect->s32Width,
            pstToBeRevisedRect->s32Height,
            pstRevisedRect->s32Width,
            pstRevisedRect->s32Height);
#endif

    (mt_void) WinOutRectSizeConversion(&stCanvas,
            &stOffsetInfo,
            &pstInfo->stFmtResolution,
            pstToBeRevisedRect,
            pstRevisedRect);

#if 0
    printk("222conversion: lstw:%d, lsth:%d, offset:%d,%d,%d,%d,curw:%d, curh:%d, tow:%d, toh:%d,sedw:%d,sedh:%d!\n",
            stCanvas.s32Width,
            stCanvas.s32Height,
            stOffsetInfo.u32Left,
            stOffsetInfo.u32Top,
            stOffsetInfo.u32Right,
            stOffsetInfo.u32Bottom,
            pstInfo->stFmtResolution.s32Width,
            pstInfo->stFmtResolution.s32Height,
            pstToBeRevisedRect->s32Width,
            pstToBeRevisedRect->s32Height,
            pstRevisedRect->s32Width,
            pstRevisedRect->s32Height);
#endif

    return MT_SUCCESS;
}

mt_s32 WindowRedistributeProcess(WINDOW_S *tmpWindow);
mt_s32 WindowRedistributeProcess_Wait(WINDOW_S *tmpWindow);

#if 0
static mt_s32 WinZorderAdd(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0, i = 0;

    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt])
        {
            i++;
        }
    }

    pstWin->u32Zorder = i;
    return WindowRedistributeProcess(pstWin);
}
#endif

static mt_void WinZorderSave(mt_u32 *pU32Array, MT_DRV_DISPLAY_E enDisp)
{
    mt_u32 u32Cnt = 0, i = 0;

    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if (stDispWindow.pstWinArray[enDisp][u32Cnt])
        {
            i++;
        }
    }

    for (u32Cnt = 0; u32Cnt < i; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[enDisp][u32Cnt]))
        {
            pU32Array[u32Cnt] = stDispWindow.pstWinArray[enDisp][u32Cnt]->u32Zorder;
        }
    }

    return;
}

static mt_void WinZorderRestore(mt_u32 *pU32Array, MT_DRV_DISPLAY_E enDisp)
{
    mt_u32 u32Cnt = 0, i = 0;

    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if (stDispWindow.pstWinArray[enDisp][u32Cnt])
        {
            i++;
        }
    }

    for (u32Cnt = 0; u32Cnt < i; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[enDisp][u32Cnt]))
        {
            stDispWindow.pstWinArray[enDisp][u32Cnt]->u32Zorder = pU32Array[u32Cnt];
        }
    }

    return;
}
static mt_s32 WinZorderDelete(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0;

    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]) &&
                (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder > pstWin->u32Zorder))
        {
            stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder--;
        }
    }

    return MT_SUCCESS;
}

static mt_s32 WinZorderMoveTop(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0, i = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32ZorderBak[WINDOW_MAX_NUMBER] = { 0 };

    WinZorderSave(u32ZorderBak, pstWin->enDisp);
    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt])
        {
            i++;
        }
    }
    if (pstWin->u32Index)
    {
        for (u32Cnt = 0; u32Cnt < i; u32Cnt++)
        {
            if ((stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]) &&
                    (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder > pstWin->u32Zorder))
            {
                stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder--;
            }
        }

        pstWin->u32Zorder = i - 1;
    }
    else
    {
        pstWin->u32Zorder = i;
    }

    ret = WindowRedistributeProcess(pstWin);
    (mt_void) WindowRedistributeProcess_Wait(pstWin);
    if (ret)
    {
        WinZorderRestore(u32ZorderBak, pstWin->enDisp);
    }

    return ret;
}

static mt_s32 WinZorderMoveBottom(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0, i = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32ZorderBak[WINDOW_MAX_NUMBER] = { 0 };

    WinZorderSave(u32ZorderBak, pstWin->enDisp);
    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt])
        {
            i++;
        }
    }

    for (u32Cnt = 0; u32Cnt < i; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]) &&
                (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder < pstWin->u32Zorder))
        {
            stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder++;
        }
    }

    pstWin->u32Zorder = 0;

    ret = WindowRedistributeProcess(pstWin);
    (mt_void) WindowRedistributeProcess_Wait(pstWin);

    if (ret)
        WinZorderRestore(u32ZorderBak, pstWin->enDisp);

    return ret;
}

static mt_s32 WinZorderMoveUp(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32ZorderBak[WINDOW_MAX_NUMBER] = { 0 };

    WinZorderSave(u32ZorderBak, pstWin->enDisp);
    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]) &&
                (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder == (pstWin->u32Zorder + 1)))
        {
            pstWin->u32Zorder++;
            stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder--;
            break;
        }
    }

    ret = WindowRedistributeProcess(pstWin);
    (mt_void) WindowRedistributeProcess_Wait(pstWin);
    if (ret)
        WinZorderRestore(u32ZorderBak, pstWin->enDisp);

    return ret;
}

static mt_s32 WinZorderMoveDown(WINDOW_S *pstWin)
{
    mt_u32 u32Cnt = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32ZorderBak[WINDOW_MAX_NUMBER] = { 0 };

    if (pstWin->u32Zorder == 0)
        return MT_SUCCESS;

    WinZorderSave(u32ZorderBak, pstWin->enDisp);
    for (u32Cnt = 0; u32Cnt < WINDOW_MAX_NUMBER; u32Cnt++)
    {
        if ((stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]) &&
                (stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder == (pstWin->u32Zorder - 1)))
        {
            pstWin->u32Zorder--;
            stDispWindow.pstWinArray[pstWin->enDisp][u32Cnt]->u32Zorder++;
            break;
        }
    }

    ret = WindowRedistributeProcess(pstWin);
    (mt_void) WindowRedistributeProcess_Wait(pstWin);
    if (ret)
        WinZorderRestore(u32ZorderBak, pstWin->enDisp);

    return ret;
}

/*check whether the layout is valid.*/
mt_void WinSetLocationInfor(WINDOW_S **pstWin, /*WindowInfor_S */ mt_void *pstWinInfo, mt_u32 numofWindow)
{
#if 0
    mt_u32 i = 0;
    mt_rect_s stCanvas;

    for (i = 0; i < numofWindow; i++)
    {
        stCanvas = pstWin[i]->stDispInfo.stWinCurrentFmt;

        /*if the window 's outrect is zero, just set it to vscreen size.*/
        if (!pstWin[i]->stUsingAttr.stOutRect.s32Width
                || !pstWin[i]->stUsingAttr.stOutRect.s32Height)
        {
            pstWin[i]->stUsingAttr.stOutRect.s32Width
                =  stCanvas.s32Width;

            pstWin[i]->stUsingAttr.stOutRect.s32Height
                =  stCanvas.s32Height;
        }

        pstWinInfo[i].x_start = pstWin[i]->stUsingAttr.stOutRect.s32X;

        /*if window is outof screen, just pull it in.*/
        if (pstWinInfo[i].x_start < 0)
        {
            pstWinInfo[i].x_start = 0;
        }
        else if (pstWinInfo[i].x_start >  stCanvas.s32Width)
        {
            pstWinInfo[i].x_start = stCanvas.s32Width;
        }

        /*compare x_end.*/
        pstWinInfo[i].x_end = pstWin[i]->stUsingAttr.stOutRect.s32X
            +  pstWin[i]->stUsingAttr.stOutRect.s32Width;
        if (pstWinInfo[i].x_end < 0)
        {
            pstWinInfo[i].x_end = 0;
        }
        else if (pstWinInfo[i].x_end > stCanvas.s32Width)
        {
            pstWinInfo[i].x_end = stCanvas.s32Width;
        }

        /*compare y_start*/
        pstWinInfo[i].y_start = pstWin[i]->stUsingAttr.stOutRect.s32Y;

        if (pstWinInfo[i].y_start < 0)
        {
            pstWinInfo[i].y_start = 0;
        }
        else if (pstWinInfo[i].y_start >  stCanvas.s32Height)
        {
            pstWinInfo[i].y_start = stCanvas.s32Height;
        }

        /*compare y_end.*/
        pstWinInfo[i].y_end = pstWin[i]->stUsingAttr.stOutRect.s32Y
            +  pstWin[i]->stUsingAttr.stOutRect.s32Height;

        if (pstWinInfo[i].y_end < 0)
        {
            pstWinInfo[i].y_end = 0;
        }
        else if (pstWinInfo[i].y_end > stCanvas.s32Height)
        {
            pstWinInfo[i].y_end = stCanvas.s32Height;
        }

        /*set the window zorder.*/
        pstWinInfo[i].zorder = pstWin[i]->u32Zorder;
    }
#endif
    return;
}

mt_void Win_GetAllEnabledWindows(WINDOW_S **ppstWin, mt_u32 *WinNum, WINDOW_S *tmpWindow)
{
    mt_u32 k = 0, i = 0;

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if (stDispWindow.pstWinArray[tmpWindow->enDisp][i])
        {
            if ((stDispWindow.pstWinArray[tmpWindow->enDisp][i]->u32Index != tmpWindow->u32Index) &&
                    (stDispWindow.pstWinArray[tmpWindow->enDisp][i]->bEnable == MT_TRUE))
            {
                ppstWin[k] = stDispWindow.pstWinArray[tmpWindow->enDisp][i];
                k++;
            }
        }
    }

    *WinNum = k;
    return;
}

mt_void Win_InsertNodeIntoEnabledWindows(WINDOW_S **ppstWin, mt_u32 *WinNum, WINDOW_S *tmpWindow)
{
    mt_u32 i = 0;

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if (stDispWindow.pstWinArray[tmpWindow->enDisp][i])
        {
            /*select enabled window which does not equal to tmpWindow
              and insert them to the window ptr array.*/
            if ((stDispWindow.pstWinArray[tmpWindow->enDisp][i]->u32Index == tmpWindow->u32Index) &&
                    (tmpWindow->bEnable == MT_TRUE))
            {
                ppstWin[*WinNum] = tmpWindow;
                *WinNum = *WinNum + 1;
            }
        }
    }

    return;
}

/*update the new layer and region number to window.*/
mt_s32 WinUpdateMappingInfor(WINDOW_S *pstWin, /*WindowMappingResult_S */ mt_void *pstWinMapResult, MT_DRV_DISPLAY_E enDisp)
{
#if 0
    mt_u32 i = 0;

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if ((stDispWindow.pstWinArray[pstWin->enDisp][i])
                && (stDispWindow.pstWinArray[pstWin->enDisp][i]->u32Index
                    == pstWin->u32Index))
        {
            if (enDisp == MT_DRV_DISPLAY_1)
            {
                stDispWindow.pstWinArray[pstWin->enDisp][i]->u32VideoLayerNew = pstWinMapResult->videoLayer;
            }
            else if (enDisp == MT_DRV_DISPLAY_0)
            {
                /*the sd channel is v3/v4*/
                stDispWindow.pstWinArray[pstWin->enDisp][i]->u32VideoLayerNew = pstWinMapResult->videoLayer + 3;
            }

            stDispWindow.pstWinArray[pstWin->enDisp][i]->u32VideoRegionNoNew = pstWinMapResult->regionNo;

#if 0
            if (pstWin->enDisp == MT_DRV_DISPLAY_1)
            {
                printk("win handle:%x, layer:%d, regiono:%d!\n", pstWin->u32Index,
                        stDispWindow.pstWinArray[pstWin->enDisp][i]->u32VideoLayerNew,
                        stDispWindow.pstWinArray[pstWin->enDisp][i]->u32VideoRegionNoNew);
            }

#endif

            //printk("!!!!!!!!!!!!!!:%x, index:%x !\n", stDispWindow.pstWinArray[pstWin->enDisp][i], stDispWindow.pstWinArray[pstWin->enDisp][i]->u32Index);
        }
    }

    if (enDisp == MT_DRV_DISPLAY_1)
        pstWin->u32VideoLayerNew = pstWinMapResult->videoLayer;
    else if (enDisp == MT_DRV_DISPLAY_0)
        /*the sd channel is v3/v4*/
        pstWin->u32VideoLayerNew = pstWinMapResult->videoLayer + 3;

    pstWin->u32VideoRegionNoNew = pstWinMapResult->regionNo;
#endif
    return MT_SUCCESS;
}

/*window adjust, move/zme/create/destroy may cause the layout
 *change. because the  adjust in thread-suituation may be half complete,
 *and then interruptted by isr, so the change should be located in    isr func.
 */
mt_s32 WindowRedistributeProcess(WINDOW_S *tmpWindow)
{
#if 0
    WINDOW_S *pstWin[WINDOW_MAX_NUMBER] = {MT_NULL}, *pstWinTmp = MT_NULL;
    WindowInfor_S winLocationInfor[WINDOW_MAX_NUMBER] = {{0}};
    WindowMappingResult_S winMappingResult[WINDOW_MAX_NUMBER] = {{0}};
    WindowZorderInfor_S  winZorderInfor[WINDOW_MAX_NUMBER] = {{0}};

    mt_u32    numofWindow = 0, i = 0, k = 0, overlapWinCnt = 0;
    mt_s32    ret = MT_SUCCESS;
    MT_DRV_DISPLAY_E enDisp  = MT_DRV_DISPLAY_BUTT;

    spin_lock(&g_threadIsr_Lock);


    /*first,get all the enabled window ptrs.*/
    Win_GetAllEnabledWindows(pstWin, &numofWindow, tmpWindow);
    Win_InsertNodeIntoEnabledWindows(pstWin, &numofWindow, tmpWindow);

    if (numofWindow == 0)
    {
        DISP_WARN("No enabled window exist.!\n");
        goto _REDIS_SUCCESS;
    }

    /*Second,  set the coordinate and locations value from windows.*/
    WinSetLocationInfor(pstWin, winLocationInfor, numofWindow);
#if 0
    for (i = 0; i < numofWindow; i++)
    {
        printk("x_start:%d, x_end:%d, y_start:%d, y_end:%d!\n",
                winLocationInfor[i].x_start,
                winLocationInfor[i].x_end,
                winLocationInfor[i].y_start,
                winLocationInfor[i].y_end);
    }
#endif

    /*third:  generate win-layer remapping.*/
    ret =  WindowAndLayerMapping(winLocationInfor, winMappingResult, numofWindow);
    if (ret)
    {
        WIN_ERROR("UI disobey the windows strict, the coordinate is as follows:!\n");
        for (i = 0; i < numofWindow; i++)
        {
            WIN_ERROR("win x:%d, y:%d, x_end:%d, y_end:%d, zorder %d !\n",
                    winLocationInfor[i].x_start,
                    winLocationInfor[i].y_start,
                    winLocationInfor[i].x_end,
                    winLocationInfor[i].y_end,
                    winLocationInfor[i].zorder);
        }

        spin_unlock(&g_threadIsr_Lock);
        return MT_ERR_VO_OPERATION_DENIED;
    }

    /*update new window-layer mapping.*/
    enDisp = tmpWindow->enDisp;
    for (i = 0 ; i < numofWindow; i++)
    {
        WinUpdateMappingInfor(pstWin[i], &winMappingResult[i], enDisp);
    }

    for (i = 0; i < numofWindow; i++)
    {
        if (winMappingResult[i].u32OverlapCnt)
        {
            winZorderInfor[overlapWinCnt].layerNumber = pstWin[i]->u32VideoLayerNew;
            winZorderInfor[overlapWinCnt].winZorder =    pstWin[i]->u32Zorder;

            (mt_void)pstWin[i]->stVLayerFunc.PF_GetZorder(pstWin[i]->u32VideoLayerNew,
                    &winZorderInfor[overlapWinCnt].layerZorder);


            overlapWinCnt ++;
        }
    }

    /*check wether the window's order matches the layer's order.*/
    ret = layerZorderMatch(winZorderInfor, overlapWinCnt);
    if (ret)
    {
        WIN_ERROR(" the windows zorder is not right, and can't allocate layers.!\n");
        for (i = 0; i < overlapWinCnt; i++)
        {
            WIN_ERROR("win zorder:%d, layerorder:%d, allocated layer:%d !\n",
                    winZorderInfor[i].winZorder,
                    winZorderInfor[i].layerZorder,
                    winZorderInfor[i].layerNumber);
        }
        spin_unlock(&g_threadIsr_Lock);
        return MT_ERR_VO_OPERATION_DENIED;
    }

    for (i = 0; i < overlapWinCnt; i++)
    {
        if ((winZorderInfor[i].layerNumber == VDP_RM_LAYER_VID0)
                || (winZorderInfor[i].layerNumber == VDP_RM_LAYER_VID3))
        {
            k = i;
            break;
        }
    }

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        pstWinTmp  = stDispWindow.pstWinArray[tmpWindow->enDisp][i];

        if (pstWinTmp
                && ((pstWinTmp->u32VideoLayerNew == VDP_RM_LAYER_VID3)
                    || (pstWinTmp->u32VideoLayerNew == VDP_RM_LAYER_VID0)))
        {
            pstWinTmp->stWinLayerOpt.layerOptType = LAYER_OPT_ZORDER_ADJUST;

            if (MT_DRV_DISP_ZORDER_BUTT == winZorderInfor[k].matchOperation)
            {
                pstWinTmp->stWinLayerOpt.bEffective = MT_FALSE;
            }
            else
            {
                pstWinTmp->stWinLayerOpt.bEffective = MT_TRUE;
            }

            pstWinTmp->stWinLayerOpt.pParam = (mt_void*)winZorderInfor[k].matchOperation;
        }
    }
#if 0
    for (i = 0; i < numofWindow; i++)
    {
        if ((pstWin[i]->u32VideoLayerNew == VDP_RM_LAYER_VID3)
                || (pstWin[i]->u32VideoLayerNew == VDP_RM_LAYER_VID0))
        {
            pstWin[i]->stWinLayerOpt.layerOptType = LAYER_OPT_ZORDER_ADJUST;

            if (MT_DRV_DISP_ZORDER_BUTT == winZorderInfor[k].matchOperation)
                pstWin[i]->stWinLayerOpt.bEffective = MT_FALSE;
            else
                pstWin[i]->stWinLayerOpt.bEffective = MT_TRUE;

            pstWin[i]->stWinLayerOpt.pParam = (mt_void*)winZorderInfor[k].matchOperation;
        }
    }
#endif

_REDIS_SUCCESS:
    /*this setting is  atomic, and update the flag, so isr func can do something.*/
    atomic_set(&stDispWindow.bWindowSytemUpdate[tmpWindow->enDisp], 1);
    spin_unlock(&g_threadIsr_Lock);
    return MT_SUCCESS;

#endif
    return MT_SUCCESS;
}
mt_s32 WindowRedistributeProcess_Wait(WINDOW_S *tmpWindow)
{
    unsigned int t = 0;
    while (atomic_read(&stDispWindow.bWindowSytemUpdate[tmpWindow->enDisp]))
    {
        DISP_MSLEEP(5);
        t++;

        if (t > 10)
        {
            WIN_WARN("wait for redistribtue end time out!\n");
            break;
        }
    }

    return MT_SUCCESS;
}
mt_u32 mmz_phy_addr = 0;

mt_s32 DF_Init_TestIMG(mt_void)
{
    IMAGE *pstImage = &g_testImagInfo;
    //DISP_MMZ_BUF_S *p_stMem = BP_GetHDTestMemInfo();
    //common info
    //left field related info
    //top
    //printk("[%s]line %d\n",__FUNCTION__,__LINE__);

    pstImage->slotInfo.frm_cnt = 0;
    pstImage->slotInfo.filedInfoTop.slot_idx = 0;
    pstImage->slotInfo.pts = 0;
    pstImage->slotInfo.filed_storage_mode = 0;
    pstImage->slotInfo.filedInfoTop.addrLuma = mmz_phy_addr;
    pstImage->slotInfo.filedInfoTop.addrChroma = mmz_phy_addr + 0x300000;
    //bot
    pstImage->slotInfo.filedInfoBot.addrLuma = mmz_phy_addr;
    pstImage->slotInfo.filedInfoBot.addrChroma = mmz_phy_addr + 0x300000;

    //Frame related info
    pstImage->slotInfo.pic_height = 1080;
    pstImage->slotInfo.pic_width = 1920;
    pstImage->slotInfo.input_rate = 25;
    pstImage->slotInfo.row_jump_value = 0xf000000;
    pstImage->slotInfo.row_jump_offset = 4;
    pstImage->slotInfo.col_size = 3;
    pstImage->slotInfo.aspect_ratio = 0;
    pstImage->slotInfo.tile_cfg = 0;
    pstImage->slotInfo.frame_pic_flag = 1;
    pstImage->slotInfo.picture_coding_type = 0;
    //dec mode parser
    pstImage->slotInfo.display_order_mode_valid = 0;
    return MT_SUCCESS;
}
extern mt_u32 p_disp_addr;

/*!
  Get 32 bits register value

  \param[in] p_addr register address

  \return register value
  */
static inline unsigned long hal_get_u32(ulong p_addr)
{
    /*!
      Get 32 bits register value
      */
    return (unsigned long)HAL_GET_U32((volatile u32 *)p_addr);
}

/*!
  Write 32 bits register

  \param[in] addr register address
  \param[in] p_addr data to write
  */
static inline void hal_put_u32(ulong p_addr, mt_u32 data)
{
    /*!
      Write 32 bits register
      */
    HAL_PUT_U32((volatile u32 *)p_addr, (u32)data);
}

mt_s32 DF_cfg_TestIMG(mt_void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    mt_u32 dtmp = 0;
#endif
    //IMAGE *pstImage = &g_testImagInfo;
    //DISP_MMZ_BUF_S *p_stMem = BP_GetHDTestMemInfo();
    //mt_u32 *pbuf = (mt_u32 *)p_stMem->u32StartVirAddr;
    //common info
    //left field related info
    //top
    MT_INFO_VO("mmz_phy_addr %x\n",mmz_phy_addr);
#if defined(CONFIG_MT_CHIP_ARIA)
    //tile para
    hal_put_u32(p_disp_addr + 0x2e0, 0x1103);
    //row jump 01
    hal_put_u32(p_disp_addr + 0x2e8, 0xf000000);
    //burst info
    hal_put_u32(p_disp_addr + 0x24, 0x11);
    //window x
    hal_put_u32(p_disp_addr + 0x88, 0x10780);
    //window y
    hal_put_u32(p_disp_addr + 0x8c, 0x10438);

    //luma addr0
    hal_put_u32(p_disp_addr + 0x200, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x204, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x208, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x20c, mmz_phy_addr);

    //chroma addr0
    hal_put_u32(p_disp_addr + 0x240, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x244, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x248, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x24c, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x250, mmz_phy_addr + 0x300000);

    //luma addr1
    hal_put_u32(p_disp_addr + 0x220, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x224, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x228, mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x22c, mmz_phy_addr);

    //chroma addr1
    hal_put_u32(p_disp_addr + 0x260, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x264, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x268, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x26c, mmz_phy_addr + 0x300000);
    hal_put_u32(p_disp_addr + 0x270, mmz_phy_addr + 0x300000);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    //tile cfg
    dtmp = hal_get_u32(SYMPHONY_IO_VA(0xbf44108c));
    dtmp &= ~(0x3 << 16);
    hal_put_u32(SYMPHONY_IO_VA(0xbf44108c), dtmp);
    //col_size_mode
    dtmp = hal_get_u32(SYMPHONY_IO_VA(0xbf000204));
    dtmp |= 0x3 << 8;
    hal_put_u32(SYMPHONY_IO_VA(0xbf000204), dtmp);
    //hd map mode
    dtmp = hal_get_u32(SYMPHONY_IO_VA(0xbf441090));
    dtmp |= 0x1;
    hal_put_u32(SYMPHONY_IO_VA(0xbf441090), dtmp);
    //field mode
    dtmp = hal_get_u32(SYMPHONY_IO_VA(0xbf44108c));
    dtmp |= 0x1;
    hal_put_u32(SYMPHONY_IO_VA(0xbf44108c), dtmp);

    reg_symphony_disp_drv_set_display_ctrl(0x01000100);
    hal_put_u32(p_disp_addr + 0xa8, 0x0);
    hal_put_u32(p_disp_addr + 0x3000, 0x121);


    //row jump 01
    reg_symphony_disp_drv_set_row_jump_01(0xf000000);
    //burst info
    //hal_put_u32(p_disp_addr + 0x24, 0x11);
    //window x
    reg_symphony_disp_drv_set_vid_window_x_hd(0x07810001);
    //window y
    reg_symphony_disp_drv_set_vid_window_y_hd(0x04390001);

    //luma addr0
    reg_symphony_disp_drv_set_luma_pre_addr(mmz_phy_addr/8);
    reg_symphony_disp_drv_set_luma_cur_addr(mmz_phy_addr/8);
    reg_symphony_disp_drv_set_luma_next_addr(mmz_phy_addr/8);

    //chroma addr0
    reg_symphony_disp_drv_set_chroma_ppre_addr((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_pre_addr((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_cur_addr((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_next_addr((mmz_phy_addr + 0x300000)/8);

    //luma addr1
    reg_symphony_disp_drv_set_luma_pre_addr2(mmz_phy_addr/8);
    reg_symphony_disp_drv_set_luma_cur_addr2(mmz_phy_addr/8);
    reg_symphony_disp_drv_set_luma_next_addr2(mmz_phy_addr/8);

    //chroma addr1
    reg_symphony_disp_drv_set_chroma_ppre_addr2((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_pre_addr2((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_cur_addr2((mmz_phy_addr + 0x300000)/8);
    reg_symphony_disp_drv_set_chroma_next_addr2((mmz_phy_addr + 0x300000)/8);

#endif
    //printk("[%s][%08x][%08x][%08x][%08x]\n",__FUNCTION__,pbuf[0],pbuf[1],pbuf[2],pbuf[3]);
#if 0
    {
        struct file *filp;
        mm_segment_t fs;
        struct inode *inode1;
        mt_u32 read_len = 0;
        off_t fsize;
        filp=filp_open("/mnt/y.bin", O_RDWR, 0644);
        if(IS_ERR(filp))
        {
            printk("[%s]open y err\n",__FUNCTION__);
            return;
        }
        inode1 = filp->f_path.dentry->d_inode;
        fsize = inode1->i_size;
        printk("[%s]fsize[%d]\n",__FUNCTION__,(mt_u32)fsize);
        //filp->f_pos = 0;
        fs=get_fs();
        set_fs(KERNEL_DS);
        read_len=filp->f_op->read(filp,(mt_u8*)(p_stMem->u32StartVirAddr),fsize,&(filp->f_pos));
        set_fs(fs);
        filp_close(filp,NULL);
        printk("[%s]read ydata ok,len[%d]\n",__FUNCTION__,read_len);
        filp=filp_open("/mnt/uv.bin", O_RDWR, 0644);
        if(IS_ERR(filp))
        {
            printk("[%s]open uv err\n",__FUNCTION__);
            return;
        }
        inode1 = filp->f_path.dentry->d_inode;
        fsize = inode1->i_size;
        printk("[%s]fsize[%d]\n",__FUNCTION__,(mt_u32)fsize);
        fs=get_fs();
        set_fs(KERNEL_DS);
        read_len=filp->f_op->read(filp,(mt_u8*)(p_stMem->u32StartVirAddr+0x300000),fsize,&(filp->f_pos));
        set_fs(fs);
        printk("[%s]read uvdata ok,len[%d]\n",__FUNCTION__,read_len);
        filp_close(filp,NULL);
    }
#endif
    //hal_put_u32(p_disp_addr + 0, hal_get_u32(p_disp_addr + 0) | 1);

    return MT_SUCCESS;
}

mt_s32 DF_cfg_TestIMG_SD(mt_void)
{
    //IMAGE *pstImage = &g_testImagInfo;
    //DISP_MMZ_BUF_S *p_stMem = BP_GetHDTestMemInfo();
    //mt_u32 *pbuf = (mt_u32 *)p_stMem->u32StartVirAddr;
    //common info
    //left field related info
    //top
    hal_put_u32(p_disp_addr + 0x2e0, 0x1103);
    hal_put_u32(p_disp_addr + 0x2e8, 0xf000000);
    hal_put_u32(p_disp_addr + 0x24, 0x21);

    MT_INFO_VO("mmz_phy_addr %x\n",mmz_phy_addr);
    hal_put_u32(p_disp_addr + 0x1c0, 0);
    hal_put_u32(p_disp_addr + 0x1c4, 0x111);
    hal_put_u32(p_disp_addr + 0x180, 0);
    hal_put_u32(p_disp_addr + 0x80, 0x10001000);
    hal_put_u32(p_disp_addr + 0x8, 0x07800438);
    hal_put_u32(p_disp_addr + 0x88, 0x10781);
    hal_put_u32(p_disp_addr + 0x8c, 0x10439);
    hal_put_u32(p_disp_addr + 0x2c, 0xf0);
    hal_put_u32(p_disp_addr + 0x8000, 0x0780021c);

    hal_put_u32(p_disp_addr + 0x8000, 0x0780021c);

    //hal_put_u32(p_disp_addr+0x94,0x2aaa2400);
    hal_put_u32(p_disp_addr + 0x94, 0x2aaa1e00);
    hal_put_u32(p_disp_addr + 0x98, 0xa00);
    hal_put_u32(p_disp_addr + 0x9c, 0x102d0);
    //hal_put_u32(p_disp_addr+0xa0,0x100f1);
    hal_put_u32(p_disp_addr + 0xa0, 0x10121);
    //hal_put_u32(p_disp_addr+0x8004,0x02d000f0);
    hal_put_u32(p_disp_addr + 0x8004, 0x02d00120);
    //hal_put_u32(p_disp_addr+0xa8,0xb);

    hal_put_u32(p_disp_addr + 0x6000, 0x10);
    hal_put_u32(p_disp_addr + 0x602c, 0x0780021c);
    hal_put_u32(p_disp_addr + 0x6048, 0xaaa02);
    //hal_put_u32(p_disp_addr+0x604c,0x00040002);
    hal_put_u32(p_disp_addr + 0x604c, 0x000E0001);
    hal_put_u32(p_disp_addr + 0x6054, 0);
    hal_put_u32(p_disp_addr + 0x6040, 0x4033700);

    hal_put_u32(p_disp_addr + 0x7000, 0x01001032);

    return MT_SUCCESS;
}
mt_s32 WinBufferReset(WIN_BUFFER_S *pstBuffer);
mt_void ISR_WinReleaseUSLFrame(WINDOW_S *pstWin);
extern mt_void disp_st_vid_get_vout_size(disp_channel_t ch, mt_u32 *p_height, mt_u32 *p_width);
extern mt_void disp_st_vid_get_vdec_size(mt_u32 *p_height, mt_u32 *p_width);

static MT_U32 g_last_vid_event = 0;

mt_s32 CheckWinEvent(void)
{
    if(g_last_vid_event)
    {
        g_last_vid_event = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

void DF_Event_func(MT_U32 event)
{
    printk(KERN_ERR "%s %d \n", __func__, __LINE__); 
    if(MT_DF_EVENT_LAST_FRMAE == event)
    {
        g_last_vid_event = 1;
    }
}
mt_s32 WinCreateDisplayWindow(MT_DRV_WIN_ATTR_S *pWinAttr, WINDOW_S **ppstWin, mt_u32 u32BufNum)
{
    WINDOW_S *pstWin;
    mt_s32 nRet;
    //DISP_MMZ_BUF_S *p_stBuf;
    MT_BOOL hd_close_di_flag;

    if (WinTestAddWindow(pWinAttr->enDisp))
    {
        WIN_ERROR("Reach max window number,can not create!\n");
        return MT_ERR_VO_CREATE_ERR;
    }

    pstWin = (WINDOW_S *)DISP_MALLOC(sizeof(WINDOW_S));
    if (!pstWin)
    {
        WIN_ERROR("Malloc WINDOW_S failed in %s!\n", __FUNCTION__);
        return MT_ERR_VO_CREATE_ERR;
    }
    MT_INFO_VO("---------------\n");

    DISP_MEMSET(pstWin, 0, sizeof(WINDOW_S));

    /* attribute */
    pstWin->bEnable = MT_FALSE;
    pstWin->bMasked = MT_FALSE;

    pstWin->enState = WIN_STATE_WORK;
    //pstWin->enStateBackup = pstWin->enState;
    pstWin->bUpState = MT_FALSE;

    pstWin->enDisp = pWinAttr->enDisp;
    pstWin->enType = MT_DRV_WIN_ACTIVE_SINGLE;

    pstWin->stCfg.stAttrBuf = *pWinAttr;
    pstWin->stCfg.stAttr = *pWinAttr;
    atomic_set(&pstWin->stCfg.bNewAttrFlag, 1);

    pstWin->stCfg.enFrameCS = MT_DRV_CS_UNKNOWN;
    pstWin->stCfg.u32Fidelity = 0;
    pstWin->stCfg.enOutCS = MT_DRV_CS_UNKNOWN;

    pstWin->stCfg.stSource.hSrc = MT_INVALID_HANDLE;
    pstWin->stCfg.stSource.pfAcqFrame = MT_NULL;
    pstWin->stCfg.stSource.pfRlsFrame = MT_NULL;
    pstWin->stCfg.stSource.pfSendWinInfo = MT_NULL;
#if 0 //disp_hal
    nRet = VideoLayer_GetFunction(&pstWin->stVLayerFunc);
    if (nRet)
    {
        WIN_ERROR("VideoLayer_GetFunction failed in %s!\n", __FUNCTION__);
        goto __ERR_GET_FUNC__;
    }

    pstWin->u32VideoLayer = VDP_LAYER_VID_BUTT;
    pstWin->u32VideoLayerNew = VDP_LAYER_VID_BUTT;
#endif

    pstWin->u32VideoRegionNo = 0xffffffff;
    pstWin->u32VideoRegionNoNew = 0xffffffff;

    pstWin->u32LastInLowDelayIdx = 0xffffffff;
    pstWin->u32LastOutLowDelayIdx = 0xffffffff;
    /*Pay attention please: here  the  func of zorder adjust should be modified.
     * do not forget it.
     */
    WinZorderMoveTop(pstWin);
    //pstWin->stBuffer.stDispBP.stSrcInfo.pfRlsFrame=stWBSrcInfo.pfRlsFrame;

    //#ifdef CONFIG_MT_DISP_HD_DI //dean fpga debug
#if 1
    hd_close_di_flag = MT_FALSE;
#else
    hd_close_di_flag = MT_TRUE;
#endif
    MT_INFO_VO("hd_close_di_flag=%d\n", hd_close_di_flag);

    //call Disp Firm create
    nRet = DF_Create(&pstWin->stBuffer.stDispBP, u32BufNum, hd_close_di_flag);
    MT_INFO_VO("Create disp buffer pool done\n");
    nRet = BP_CreateHDTestMem();
    if (nRet)
    {
        MT_INFO_VO("Create HDTestMem fail\n");
    }
    DF_Init_TestIMG();

    nRet = WinBuf_Create(u32BufNum, WIN_BUF_MEM_SRC_SUPPLY, MT_NULL, &pstWin->stBuffer.stWinBP);
    MT_INFO_VO("Create window  buffer pool done\n");
    if (nRet)
    {
        WIN_ERROR("Create buffer pool failed\n");
        goto __ERR_GET_FUNC__;
    }

    WinBufferReset(&pstWin->stBuffer);

    // initial reset
    pstWin->bReset = MT_FALSE;
    pstWin->bConfigedBlackFrame = MT_FALSE;

    // initial quickmode
    pstWin->bQuickMode = MT_FALSE;

    // initial stepmode flag
    pstWin->bStepMode = MT_FALSE;

    //init delay info
    pstWin->stDelayInfo.u32DispRate = 5000;
    pstWin->stDelayInfo.bTBMatch = MT_TRUE;
    pstWin->stDelayInfo.u32DisplayTime = 20;
    pstWin->bInInterrupt = MT_FALSE;

    //init rotation
    pstWin->enRotation = MT_DRV_ROT_ANGLE_0;
    //init flip
    pstWin->bVertFlip = MT_FALSE;
    pstWin->bHoriFlip = MT_FALSE;
    *ppstWin = pstWin;

    MT_INFO_VO("---------------\n");

    return MT_SUCCESS;

__ERR_GET_FUNC__:

    DISP_FREE(pstWin);

    return MT_ERR_VO_CREATE_ERR;
}
extern volatile mt_u32 *p_intcnt;
extern atomic_t smallwindow_flag[];
extern mt_rect_s smallwindow_rect[];
extern atomic_t cropwindow_flag[];
extern mt_rect_s cropwindow_rect[];
extern atomic_t tvsyschange_flag;
extern MT_DRV_DISP_PPMODE_E PPMode;
extern MT_BOOL LayerEnable[];
extern MT_BOOL AfdEnable;
extern MT_BOOL SdScalerEnable;
extern MT_DRV_ASPECT_RATIO_E out_ar;
extern disp_sys_t SdVidSys;
extern disp_sys_t HdVidSys;
extern MT_BOOL HdOutEnable;
extern MT_BOOL SdOutEnable;

extern mt_void avsync_video_loop(mt_void *);
extern MT_BOOL avsync_is_finished(mt_void);
mt_u32 table_frmrate[8]={5000,2400,2500,3000,5000,6000,12000,6000};
mt_u32 table_frmrate_1001[8]={5000,2400,2500,2997,5000,5994,12000,6000};

mt_void ISR_FUNCTION(int index)
{
    mt_u32 frame_rate = 0;
    mt_u32 w, h;
    rect_vsb_t vout_rect_hd = {0};
    mt_u32 interlace = 0;
    mt_u32 field_type = 0;
    mt_bool avsync_status = 0;
    
    if(!atomic_read(&bCFGVideoFlag[index]))
        return;

    if(index >= MAX_WIN_NUM)
        return;

    atomic_set(&bISRProcessFlag,1);
    p_intcnt[20]= 1;
    //    if(atomic_read(&bCleanFifoFlag))
    //    {
    //        printk("[%s]line %d\n", __FUNCTION__, __LINE__);
    //        DF_SetCmd(DF_CMD_STOP);
    //        g_stDrvSetting.eDisplayStatus = DF_STATUS_STOP;
    //    }

    if(atomic_read(&tvsyschange_flag))
    {
        disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &h, &w);
        if (w == 0 || h == 0)
        {
            g_stDrvSetting[index].window_xy_HD.u32Xstart = 0;
            g_stDrvSetting[index].window_xy_HD.u32Xend = MT_HD_WIDTH-1;
            g_stDrvSetting[index].window_xy_HD.u32Ystart = 0;
            g_stDrvSetting[index].window_xy_HD.u32Yend = MT_HD_HEIGHT-1;
        }
        else
        {
            g_stDrvSetting[index].window_xy_HD.u32Xstart = 0;
            g_stDrvSetting[index].window_xy_HD.u32Xend = w-1;
            g_stDrvSetting[index].window_xy_HD.u32Ystart = 0;
            g_stDrvSetting[index].window_xy_HD.u32Yend = h-1;
        }
        atomic_set(&tvsyschange_flag,0);
        DISP_DEBUGK("[%s %d]reset window[%d %d %d %d]\n", __FUNCTION__, __LINE__,
            g_stDrvSetting[index].window_xy_HD.u32Xstart,
            g_stDrvSetting[index].window_xy_HD.u32Ystart,
            g_stDrvSetting[index].window_xy_HD.u32Xend,
            g_stDrvSetting[index].window_xy_HD.u32Yend);
    }


    if(atomic_read(&smallwindow_flag[index]))
    {
        g_stDrvSetting[index].window_xy_HD.u32Xstart = smallwindow_rect[index].s32X;
        g_stDrvSetting[index].window_xy_HD.u32Xend= smallwindow_rect[index].s32X+smallwindow_rect[index].s32Width-1;
        g_stDrvSetting[index].window_xy_HD.u32Ystart = smallwindow_rect[index].s32Y;
        g_stDrvSetting[index].window_xy_HD.u32Yend= smallwindow_rect[index].s32Y+smallwindow_rect[index].s32Height-1;
        atomic_set(&smallwindow_flag[index],0);
        WIN_DEBUGK("[%s]set smallwindow index:%d,x,y,w,h[%d,%d,%d,%d]\n", __FUNCTION__, index,
            smallwindow_rect[index].s32X,
            smallwindow_rect[index].s32Y,
            smallwindow_rect[index].s32Width,
            smallwindow_rect[index].s32Height);
    }


    if(atomic_read(&cropwindow_flag[index]))
    {
        g_stDrvSetting[index].crop_xy.u32Xstart = cropwindow_rect[index].s32X;
        g_stDrvSetting[index].crop_xy.u32Xend= cropwindow_rect[index].s32X+cropwindow_rect[index].s32Width-1;
        g_stDrvSetting[index].crop_xy.u32Ystart = cropwindow_rect[index].s32Y;
        g_stDrvSetting[index].crop_xy.u32Yend= cropwindow_rect[index].s32Y+cropwindow_rect[index].s32Height-1;
        disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &(vout_rect_hd.h), &(vout_rect_hd.w));
        if((cropwindow_rect[index].s32X == vout_rect_hd.x) &&
                (cropwindow_rect[index].s32Y == vout_rect_hd.y) &&
                (cropwindow_rect[index].s32Width == vout_rect_hd.w) &&
                (cropwindow_rect[index].s32Height == vout_rect_hd.h))
        {
            g_stDrvSetting[index].crop_xy.bEnable = MT_FALSE;
        }
        else
            g_stDrvSetting[index].crop_xy.bEnable = MT_TRUE;

        atomic_set(&cropwindow_flag[index],0);            
        WIN_DEBUGK("[%s]set cropwindow index:%d,x,y,w,h[%d,%d,%d,%d]\n", __FUNCTION__, index,
            cropwindow_rect[index].s32X,
            cropwindow_rect[index].s32Y,
            cropwindow_rect[index].s32Width,
            cropwindow_rect[index].s32Height);
    }                



    interlace = reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    if (interlace == 0)
    {
        g_stDrvSetting[index].eIntPolarity = MT_INT_PRO;
    }
    else
    {
        field_type = hal_get_u32((ulong)REG_ARIA_HD_ENCODER_HD_ENCODER_STATUS) & 0x2;

        if (field_type == 0)
        {
            g_stDrvSetting[index].eIntPolarity = MT_INT_TOP;
        }
        else
        {
            g_stDrvSetting[index].eIntPolarity = MT_INT_BOT;
        }
    }

    frame_rate = drv_reg_4k_disp_get_video_ctrl_2_frame_rate();
    if(frame_rate>=6 )
    {
        frame_rate = 0;
    }


    g_stDrvSetting[index].eFrameRate = (HAL_GET_U32((volatile u32 *)R_INTP_CTRL_REG1) & 0x1) ? table_frmrate[frame_rate] : table_frmrate_1001[frame_rate];
    g_stDrvSetting[index].e_mode = (int)PPMode;
    
    g_stDrvSetting[index].out_ar = (int)out_ar;
    g_stDrvSetting[index].bAfdOpen = AfdEnable;
    g_stDrvSetting[index].bUse3Scaler = SdScalerEnable;
    g_stDrvSetting[index].eSdVidSys = SdVidSys;
    g_stDrvSetting[index].eHdVidSys = HdVidSys;
    g_stDrvSetting[index].bHdOutEnable = HdOutEnable;
    g_stDrvSetting[index].bSdOutEnable = SdOutEnable;
    if((NULL != g_pstWin[index]) && g_pstWin[index]->stCfg.stAttr.bUseSubLayer)
    {
        g_stDrvSetting[index].b_layerEnable = LayerEnable[DISP_LAYER_ID_STILL_HD];
    }
    else
    {
        g_stDrvSetting[index].b_layerEnable = LayerEnable[DISP_LAYER_ID_VIDEO_HD];
    }        
    // check top
    if((NULL != g_pstWin[index])
        && (0 == g_pstWin[index]->stCfg.stAttr.bUseSubLayer))
    { //only one video for avsync
        avsync_video_loop((mt_void *)g_pstWin[index]->stBuffer.stDispBP);
        avsync_status = avsync_is_finished();
        if(g_stDrvSetting[index].bAvsyncFinished != avsync_status)
        {
            WIN_DEBUGK("%s avsync_status:%d\n", __FUNCTION__, avsync_status);
            g_stDrvSetting[index].bAvsyncFinished = avsync_status;
        }
    }


    if(g_pstWin[index] != NULL)
    {
        DF_ISR_Update(g_pstWin[index]->stBuffer.stDispBP, &g_stDrvSetting[index]);
    }

    if(atomic_read(&bCleanFifoFlag))
    {
        //hmc, clean display fifo when resolution change

        if(g_pstWin[index] != NULL)
        {
            DF_FlushDisp(g_pstWin[index]->stBuffer.stDispBP, (int)g_FlushDispType);
        }

        //tangbai, disp should keep freeze state
        //DF_SetCmd(DF_CMD_RESUME);
        //g_stDrvSetting.eDisplayStatus = DF_STATUS_PLAY;
        atomic_set(&bCleanFifoFlag, 0);
        MT_INFO_VO("---------------\n");
    }
    atomic_set(&bISRProcessFlag, 0);

    //FIXME:
    disp_update_tde_video_size();

    ///TODO::  check skiped frame
    p_intcnt[21] = 1;
    //reg_aria1_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(0);
}

mt_s32 WinDestroyDisplayWindow(WINDOW_S *pstWin)
{
    MT_DRV_VIDEO_FRAME_S *pstFrame;
    MT_DRV_DISPLAY_E enDisp;
    mt_u32 u32WinIndex;

    WinGetIndex(pstWin->u32Index, &enDisp, &u32WinIndex);    

    g_pstWin[u32WinIndex] = MT_NULL;
    g_stDrvSetting[u32WinIndex].crop_xy.bEnable = MT_FALSE;

    DF_Destroy(pstWin->stBuffer.stDispBP);
    BP_DestroyHDTestMem();
    DISP_DEBUGK("[%s]line %d win_index:%d bUseSubLayer:%d\n", __FUNCTION__, __LINE__, 
        u32WinIndex, pstWin->stCfg.stAttr.bUseSubLayer);

    WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);
    ISR_WinReleaseUSLFrame(pstWin);

    // flush frame in full buffer pool
    pstFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
    WinBuf_FlushWaitingFrame(&pstWin->stBuffer.stWinBP, pstFrame);

    // release current frame
    WinBuf_ForceReleaseFrame(&pstWin->stBuffer.stWinBP, pstFrame);

    // s1 derstoy buffer
    WinBuf_Destroy(&pstWin->stBuffer.stWinBP);

    /*when delete the win, adjust the zorder.*/
    WinZorderDelete(pstWin);
    
    if(pstWin->stCfg.stAttr.bUseSubLayer)
    {
        drv_reg_4k_disp_set_still_control_still_select(0);        
    }
    else
    {
        drv_reg_4k_disp_set_video_ctrl_1_video_sel(0);
    }

    DISP_FREE(pstWin);

    return MT_SUCCESS;
}

mt_s32 WinRegCallback(WINDOW_S *pstWin)
{
    MT_DRV_DISP_CALLBACK_S stCB;
    mt_s32 nRet = MT_SUCCESS;

    stCB.hDst = (mt_handle)pstWin;
    stCB.pfDISP_Callback = ISR_CallbackForWinProcess;
    nRet = DISP_RegCallback(WinGetDispID(pstWin), MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);
    if (nRet)
    {
        WIN_DEBUGK("WIN register callback failed in %s!\n", __FUNCTION__);
        return MT_ERR_VO_CREATE_ERR;
    }

    return MT_SUCCESS;
}

mt_s32 WinUnRegCallback(WINDOW_S *pstWin)
{
    MT_DRV_DISP_CALLBACK_S stCB;
    mt_s32 nRet = MT_SUCCESS;

    stCB.hDst = (mt_handle)pstWin;
    stCB.pfDISP_Callback = ISR_CallbackForWinProcess;
    nRet = DISP_UnRegCallback(WinGetDispID(pstWin), MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);

    return MT_SUCCESS;
}

mt_void WinSetBlackFrameFlag(WINDOW_S *pstWin)
{
    if (pstWin)
    {
        pstWin->bConfigedBlackFrame = MT_TRUE;
    }
}

mt_void WinClearBlackFrameFlag(WINDOW_S *pstWin)
{
    if (pstWin)
    {
        pstWin->bConfigedBlackFrame = MT_FALSE;
    }
}

MT_BOOL WinTestBlackFrameFlag(WINDOW_S *pstWin)
{
    if (pstWin)
    {
        return pstWin->bConfigedBlackFrame;
    }
    return MT_FALSE;
}

mt_s32 WinCheckFixedAttr(MT_DRV_WIN_ATTR_S *pOldAttr, MT_DRV_WIN_ATTR_S *pNewAttr)
{
    if ((pOldAttr->enDisp != pNewAttr->enDisp) || (pOldAttr->bVirtual != pNewAttr->bVirtual))
    {
        WIN_ERROR("the fixed attr is not right.!\n");
        return MT_ERR_VO_OPERATION_DENIED;
    }

    if (pOldAttr->bVirtual)
    {
        if ((pOldAttr->bUserAllocBuffer != pNewAttr->bUserAllocBuffer) || (pOldAttr->u32BufNumber != pNewAttr->u32BufNumber))
        {
            WIN_ERROR("the fixed attr of virtual window error!\n");
            return MT_ERR_VO_OPERATION_DENIED;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WinCheckAttr(MT_DRV_WIN_ATTR_S *pstAttr, MT_BOOL bVirtScreen)
{
    mt_rect_s drv_resolution;
    mt_s32 ret = 0;
    MT_DISP_DISPLAY_INFO_S stInfo;

    if (pstAttr->enDisp > MT_DRV_DISPLAY_1)
    {
        WIN_DEBUGK("WIN only support MT_DRV_DISPLAY_0! %s\n", __FUNCTION__);
        return MT_ERR_VO_INVALID_PARA;
    }

    memset((void *)&drv_resolution, 0, sizeof(mt_rect_s));
    memset((void *)&stInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));

    if (bVirtScreen)
    {
        ret = DISP_GetVirtScreen(pstAttr->enDisp, &drv_resolution);
    }
    else
    {
        ret = DISP_GetDisplayInfo(pstAttr->enDisp, &stInfo);
        drv_resolution = stInfo.stFmtResolution;
    }

    if (ret != MT_SUCCESS)
    {
        WIN_DEBUGK("Get Virtual SCREEN error! %s\n", __FUNCTION__);
        return MT_ERR_VO_INVALID_PARA;
    }

    /*delete the window size limit.*/
#if 0
    if (pstAttr->bVirtual == MT_TRUE)
    {
        if (   (pstAttr->stOutRect.s32Height > WIN_VIRTUAL_OUTRECT_MAX_HEIGHT)
                || (pstAttr->stOutRect.s32Width  > WIN_VIRTUAL_OUTRECT_MAX_WIDTH))
        {
            MT_ERR_WIN("Virtual win outrect is larger max width and height!\n");
            return MT_ERR_VO_INVALID_PARA;
        }
    }
    else
    {
        if (   (pstAttr->stOutRect.s32Height > drv_resolution.s32Height)
                || (pstAttr->stOutRect.s32Width > drv_resolution.s32Width))
        {
            MT_ERR_WIN("Win outrect is larger than virtual screen!\n");
            return MT_ERR_VO_INVALID_PARA;
        }
    }
#endif

    if (((pstAttr->stOutRect.s32Height == 0) && (pstAttr->stOutRect.s32Width != 0)) ||
            ((pstAttr->stOutRect.s32Height != 0) && (pstAttr->stOutRect.s32Width == 0)))
    {
        WIN_DEBUGK("win outrect error, one of w/h is zero.! %s\n", __FUNCTION__);
        return MT_ERR_VO_INVALID_PARA;
    }
    else if ((!(pstAttr->stOutRect.s32Height | pstAttr->stOutRect.s32Width)) &&
            (pstAttr->stOutRect.s32X | pstAttr->stOutRect.s32Y))
    {
        WIN_DEBUGK("when w/h is zero, x/y should be zero too.! %s\n", __FUNCTION__);
        return MT_ERR_VO_INVALID_PARA;
    }
    else if ((pstAttr->stOutRect.s32Height | pstAttr->stOutRect.s32Width) &&
            ((pstAttr->stOutRect.s32Width < WIN_OUTRECT_MIN_WIDTH) ||
             (pstAttr->stOutRect.s32Height < WIN_OUTRECT_MIN_HEIGHT)))
    {
        WIN_DEBUGK("The Min WIN OutRect supported is 64*64 ! %s [%d %d]\n", __FUNCTION__, 
            pstAttr->stOutRect.s32Width,pstAttr->stOutRect.s32Height);
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((pstAttr->stCustmAR.u32ARw > (pstAttr->stCustmAR.u32ARh * WIN_MAX_ASPECT_RATIO)) ||
            ((pstAttr->stCustmAR.u32ARw * WIN_MAX_ASPECT_RATIO) < pstAttr->stCustmAR.u32ARh))
    {
        WIN_DEBUGK("bUserDefAspectRatio  error! %s\n", __FUNCTION__);
        return MT_ERR_VO_INVALID_PARA;
    }

    if (pstAttr->bUseCropRect)
    {
        if ((pstAttr->stCropRect.u32TopOffset > WIN_CROPRECT_MAX_OFFSET_TOP) ||
                (pstAttr->stCropRect.u32LeftOffset > WIN_CROPRECT_MAX_OFFSET_LEFT) ||
                (pstAttr->stCropRect.u32BottomOffset > WIN_CROPRECT_MAX_OFFSET_BOTTOM) ||
                (pstAttr->stCropRect.u32RightOffset > WIN_CROPRECT_MAX_OFFSET_RIGHT))
        {
            WIN_DEBUGK("WIN CropRec support less than 128! %s\n", __FUNCTION__);
            return MT_ERR_VO_INVALID_PARA;
        }
    }
    else
    {
        if (!pstAttr->stInRect.s32Height || !pstAttr->stInRect.s32Width)
        {
            DISP_MEMSET(&pstAttr->stInRect, 0, sizeof(mt_rect_s));
        }
        else if ((pstAttr->stInRect.s32Width < WIN_FRAME_MIN_WIDTH) ||
                (pstAttr->stInRect.s32Height < WIN_FRAME_MIN_HEIGHT) ||
                (pstAttr->stInRect.s32Width > WIN_FRAME_MAX_WIDTH) ||
                (pstAttr->stInRect.s32Height > WIN_FRAME_MAX_HEIGHT))
        {
            WIN_DEBUGK("WIN InRect support 64*64 ~ 2560*1600! %s\n", __FUNCTION__);
            return MT_ERR_VO_INVALID_PARA;
        }
    }

    /* may change when window lives */
    pstAttr->stInRect.s32X = pstAttr->stInRect.s32X & MT_WIN_IN_RECT_X_ALIGN;
    pstAttr->stInRect.s32Y = pstAttr->stInRect.s32Y & MT_WIN_IN_RECT_Y_ALIGN;
    pstAttr->stInRect.s32Width = pstAttr->stInRect.s32Width & MT_WIN_IN_RECT_WIDTH_ALIGN;
    pstAttr->stInRect.s32Height = pstAttr->stInRect.s32Height & MT_WIN_IN_RECT_HEIGHT_ALIGN;

    pstAttr->stCropRect.u32LeftOffset = pstAttr->stCropRect.u32LeftOffset & MT_WIN_IN_RECT_X_ALIGN;
    pstAttr->stCropRect.u32RightOffset = pstAttr->stCropRect.u32RightOffset & MT_WIN_IN_RECT_X_ALIGN;
    pstAttr->stCropRect.u32TopOffset = pstAttr->stCropRect.u32TopOffset & MT_WIN_IN_RECT_Y_ALIGN;
    pstAttr->stCropRect.u32BottomOffset = pstAttr->stCropRect.u32BottomOffset & MT_WIN_IN_RECT_Y_ALIGN;

    pstAttr->stOutRect.s32X = pstAttr->stOutRect.s32X & MT_WIN_OUT_RECT_X_ALIGN;
    pstAttr->stOutRect.s32Y = pstAttr->stOutRect.s32Y & MT_WIN_OUT_RECT_Y_ALIGN;
    pstAttr->stOutRect.s32Width = pstAttr->stOutRect.s32Width & MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pstAttr->stOutRect.s32Height = pstAttr->stOutRect.s32Height & MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    return MT_SUCCESS;
}

mt_s32 WinCheckSourceInfo(MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    if (!pstSrc->pfAcqFrame && !pstSrc->pfRlsFrame && !pstSrc->pfSendWinInfo)
    {
        return MT_ERR_VO_NULL_PTR;
    }
    return MT_SUCCESS;
}

static mt_void Win_Calculate_New_DeviceAspectRatio(MT_DRV_ASPECT_RATIO_S *pstDeviceAspectRatio,
        MT_BOOL bHorSrEnable,
        MT_BOOL bVerSrEnable,
        MT_DRV_ASPECT_RATIO_S *pstNewDeviceAspectRatio)
{
    mt_u32 tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;

    if (bHorSrEnable)
    {
        tmp1 = 2;
        tmp2 = 1;
    }
    else
    {
        tmp1 = 1;
        tmp2 = 1;
    }

    if (bVerSrEnable)
    {
        tmp3 = 1;
        tmp4 = 2;
    }
    else
    {
        tmp3 = 1;
        tmp4 = 1;
    }

    pstNewDeviceAspectRatio->u32ARw = pstDeviceAspectRatio->u32ARw * tmp1 * tmp3;
    pstNewDeviceAspectRatio->u32ARh = pstDeviceAspectRatio->u32ARh * tmp2 * tmp4;

#if 0
    if (haha_i == 1)
        printk("#####----%d, %d!\n", pstNewDeviceAspectRatio->u32ARw,pstNewDeviceAspectRatio->u32ARh);
#endif

    return;
}

mt_s32 WinSendAttrToSource(WINDOW_S *pstWin, MT_DISP_DISPLAY_INFO_S *pstDispInfo)
{
    VIDEO_LAYER_CAPABILITY_S stVideoLayerCap;
    mt_rect_s stOutRectTmp;
    MT_BOOL bHorSrEnable = MT_FALSE;
    MT_BOOL bVerSrEnable = MT_FALSE;
    MT_DRV_DISP_STEREO_E enStereo = pstDispInfo->eDispMode;

    memset((void *)&stOutRectTmp, 0, sizeof(mt_rect_s));

    if (pstWin->stCfg.stSource.pfSendWinInfo)
    {
        MT_DRV_WIN_PRIV_INFO_S stInfo;
        MT_DRV_WIN_ATTR_S *pstAttr;
        MT_DRV_ASPECT_RATIO_S stScreenARTmp;
        PFN_GET_WIN_INFO_CALLBACK pfTmpSendWinInfo = MT_NULL;

        DISP_MEMSET(&stInfo, 0, sizeof(MT_DRV_WIN_PRIV_INFO_S));

        pstAttr = &pstWin->stUsingAttr;

        stInfo.ePixFmt = MT_DRV_PIX_FMT_NV21;
        stInfo.bUseCropRect = pstAttr->bUseCropRect;
        stInfo.stInRect = pstAttr->stInRect;
        stInfo.stCropRect = pstAttr->stCropRect;

        if (!pstAttr->stOutRect.s32Width || !pstAttr->stOutRect.s32Height)
            stInfo.stOutRect = pstDispInfo->stFmtResolution;
        else
            stInfo.stOutRect = pstAttr->stOutRect;

#if 0
        if (pstWin->enDisp == 1)
            haha_i = 1;

        if (haha_i == 1)
        {
            printk("111--set source:%d, %d, %d,%d!\n", stInfo.stOutRect.s32X,
                    stInfo.stOutRect.s32Y,
                    stInfo.stOutRect.s32Width,
                    stInfo.stOutRect.s32Height);
        }
#endif
        stInfo.stScreenAR = pstDispInfo->stAR;
        stInfo.stCustmAR = pstAttr->stCustmAR;
        stInfo.enARCvrs = pstAttr->enARCvrs;

        stInfo.bUseExtBuf = pstAttr->bUserAllocBuffer;

        stInfo.u32MaxRate = (pstDispInfo->u32RefreshRate > WIN_TRANSFER_CODE_MAX_FRAME_RATE) ? (pstDispInfo->u32RefreshRate / 2) : pstDispInfo->u32RefreshRate;

        stInfo.bInterlaced = pstDispInfo->bInterlace;
        stInfo.enRotation = pstWin->enRotation;
        stInfo.bHoriFlip = pstWin->bHoriFlip;
        stInfo.bVertFlip = pstWin->bVertFlip;

        // stInfo.stScreen     = pstDispInfo->stFmtResolution;
        if ((pstDispInfo->eDispMode != DISP_STEREO_NONE) && (pstDispInfo->eDispMode < DISP_STEREO_BUTT))
        {
            stInfo.bIn3DMode = MT_TRUE;
        }
        else
        {
            stInfo.bIn3DMode = MT_FALSE;
        }

        stInfo.bTunnelSupport = MT_TRUE;

        if (!pstWin->stVLayerFunc.PF_Get3DOutRect)
        {
            //WIN_FATAL("PF_Get3DOutRect is null\n");
            return MT_SUCCESS;
        }

        /*when 3d mode ,set screen and OutRect as one eys*/
        pstWin->stVLayerFunc.PF_Get3DOutRect(pstDispInfo->eDispMode, &pstDispInfo->stFmtResolution, &stInfo.stScreen);
        pstWin->stVLayerFunc.PF_Get3DOutRect(pstDispInfo->eDispMode, &stInfo.stOutRect, &stInfo.stOutRect);

        pstWin->stWinInfoForDeveloper.stFinalWinOutputSize = stInfo.stOutRect;
        /*If we need a 2-class zme, to avoid the distortion of content,
          we should let every zme do the same aspect ratio of zoom in or out.*/
        Win_Pre_ScalerDistribute(pstWin,
                &stInfo.stOutRect,
                &stInfo.stOutRect,
                stDispWindow.u32WinNumber[pstWin->enDisp],
                &bHorSrEnable,
                &bVerSrEnable,
                &pstDispInfo->stFmtResolution,
                enStereo);

        Win_Calculate_New_DeviceAspectRatio(&stInfo.stScreenAR, bHorSrEnable, bVerSrEnable, &stScreenARTmp);

        stInfo.stScreenAR = stScreenARTmp;
        DISP_PRINT(">>>>>>>>>>>>>>>>>>>>>>>>>>>will send info to source .............\n");

        pstWin->stWinInfoForDeveloper.u32WinNum = stDispWindow.u32WinNumber[pstWin->enDisp];
        pstWin->stWinInfoForDeveloper.bIn3DMode = stInfo.bIn3DMode;

        pstWin->stWinInfoForDeveloper.bHorSrOpenInPreProcess = bHorSrEnable;
        pstWin->stWinInfoForDeveloper.bVerSrOpenInPreProcess = bVerSrEnable;

        pstWin->stWinInfoForDeveloper.stVdpRequire = stInfo.stOutRect;
        pstWin->stWinInfoForDeveloper.eCurrentFmt = pstDispInfo->stFmtResolution;

        if (!pstWin->stVLayerFunc.PF_GetCapability)
        {
            WIN_FATAL("PF_GetCapability is null\n");
            return MT_SUCCESS;
        }

        /*as a result of winlayermapping, we switch vpss  compress on or off according
          the layer capability.*/
        if (!pstWin->stVLayerFunc.PF_GetCapability(pstWin->u32VideoLayer, &stVideoLayerCap) && (stVideoLayerCap.bDcmp == 1))
            stInfo.bCompressFlag = 1;
        else
            stInfo.bCompressFlag = 0;

        pfTmpSendWinInfo = pstWin->stCfg.stSource.pfSendWinInfo;
        if (pfTmpSendWinInfo)
        {
            (pfTmpSendWinInfo)(pstWin->stCfg.stSource.hSrc, &stInfo);
        }
    }

    return MT_SUCCESS;
}

mt_s32 WinCalcDispRectBaseRefandRel(mt_rect_s *pRef, mt_rect_s *pRel,
        mt_rect_s *pI, mt_rect_s *pO)
{
    pO->s32X = (pI->s32X * pRel->s32Width) / pRef->s32Width;
    pO->s32Width = (pI->s32Width * pRel->s32Width) / pRef->s32Width;
    pO->s32Y = (pI->s32Y * pRel->s32Height) / pRef->s32Height;
    pO->s32Height = (pI->s32Height * pRel->s32Height) / pRef->s32Height;

    pO->s32X = pO->s32X & MT_WIN_OUT_RECT_X_ALIGN;
    pO->s32Width = pO->s32Width & MT_WIN_OUT_RECT_WIDTH_ALIGN;
    pO->s32Y = pO->s32Y & MT_WIN_OUT_RECT_Y_ALIGN;
    pO->s32Height = pO->s32Height & MT_WIN_OUT_RECT_HEIGHT_ALIGN;

    if (pO->s32Width && (pO->s32Width < WIN_OUTRECT_MIN_WIDTH))
    {
        pO->s32Width = WIN_OUTRECT_MIN_WIDTH;
    }

    if (pO->s32Height && (pO->s32Height < WIN_OUTRECT_MIN_HEIGHT))
    {
        pO->s32Height = WIN_OUTRECT_MIN_HEIGHT;
    }

    return MT_SUCCESS;
}

mt_s32 WinCalcCropRectBaseRefandRel(mt_rect_s *pRef, mt_rect_s *pRel,
        MT_DRV_CROP_RECT_S *pI,
        MT_DRV_CROP_RECT_S *pO)
{
    pO->u32LeftOffset = (pI->u32LeftOffset * pRel->s32Width) / pRef->s32Width;
    pO->u32RightOffset = (pI->u32RightOffset * pRel->s32Width) / pRef->s32Width;
    pO->u32TopOffset = (pI->u32TopOffset * pRel->s32Height) / pRef->s32Height;
    pO->u32BottomOffset = (pI->u32BottomOffset * pRel->s32Height) / pRef->s32Height;

    pO->u32LeftOffset = pO->u32LeftOffset & MT_WIN_IN_RECT_X_ALIGN;
    pO->u32RightOffset = pO->u32RightOffset & MT_WIN_IN_RECT_X_ALIGN;
    pO->u32TopOffset = pO->u32TopOffset & MT_WIN_IN_RECT_Y_ALIGN;
    pO->u32BottomOffset = pO->u32BottomOffset & MT_WIN_IN_RECT_Y_ALIGN;

    return MT_SUCCESS;
}

mt_s32 WinGetSlaveWinAttr(MT_DRV_WIN_ATTR_S *pWinAttr,
        MT_DRV_WIN_ATTR_S *pSlvWinAttr,
        MT_BOOL bVirtScreen)
{
    mt_s32 nRet;
    MT_DISP_DISPLAY_INFO_S stDispInfo, stSlaveDispInfo;
    mt_rect_s stCanvas;
    mt_rect_s stMastOutRect;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;

    DISP_MEMSET(pSlvWinAttr, 0, sizeof(MT_DRV_WIN_ATTR_S));
    DISP_MEMSET((void *)&stDispInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));
    DISP_MEMSET((void *)&stSlaveDispInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));

    // s1 get slave display
    nRet = DISP_GetSlave(pWinAttr->enDisp, &pSlvWinAttr->enDisp);
    if (nRet)
    {
        WIN_ERROR("Get slave Display failed\n");
        return nRet;
    }

    if (!DISP_IsOpened(pSlvWinAttr->enDisp))
    {
        WIN_ERROR("Slave Display is not open\n");
        return MT_ERR_DISP_NOT_OPEN;
    }

    // s2 get master and slave display info
    pSlvWinAttr->bVirtual = MT_FALSE;

    /* may change when window lives */
    pSlvWinAttr->stCustmAR = pWinAttr->stCustmAR;
    pSlvWinAttr->enARCvrs = pWinAttr->enARCvrs;

    pSlvWinAttr->bUseCropRect = pWinAttr->bUseCropRect;
    pSlvWinAttr->stInRect = pWinAttr->stInRect;

    pSlvWinAttr->stCropRect = pWinAttr->stCropRect;

    if (!bVirtScreen)
    {
        (mt_void) DISP_GetDisplayInfo(pWinAttr->enDisp, &stDispInfo);
        (mt_void) DISP_GetDisplayInfo(pSlvWinAttr->enDisp, &stSlaveDispInfo);

        stCanvas = stDispInfo.stFmtResolution;
        stOffsetInfo.u32Left = 0;
        stOffsetInfo.u32Top = 0;
        stOffsetInfo.u32Right = 0;
        stOffsetInfo.u32Bottom = 0;

        stMastOutRect = pWinAttr->stOutRect;
        if ((stMastOutRect.s32Width == 0) || (stMastOutRect.s32Height == 0))
        {
            stMastOutRect.s32Width = stCanvas.s32Width;
            stMastOutRect.s32Height = stCanvas.s32Height;
        }

        (mt_void) WinOutRectSizeConversion(&stCanvas,
                &stOffsetInfo,
                &stSlaveDispInfo.stFmtResolution,
                &stMastOutRect,
                &pSlvWinAttr->stOutRect);
#if 0
        printk("--stCanvas:%d,%d, offset:%d,%d,%d,%d; fmt:%d, %d; mout:%d,%d,%d,%d, slv:%d,%d,%d,%d!\n",
                stCanvas.s32Width,
                stCanvas.s32Height,
                stOffsetInfo.u32Left,
                stOffsetInfo.u32Top,
                stOffsetInfo.u32Right,
                stOffsetInfo.u32Bottom,
                stSlaveDispInfo.stFmtResolution.s32Width,
                stSlaveDispInfo.stFmtResolution.s32Height,
                pWinAttr->stOutRect.s32X,
                pWinAttr->stOutRect.s32Y,
                pWinAttr->stOutRect.s32Width,
                pWinAttr->stOutRect.s32Height,
                pSlvWinAttr->stOutRect.s32X,
                pSlvWinAttr->stOutRect.s32Y,
                pSlvWinAttr->stOutRect.s32Width,
                pSlvWinAttr->stOutRect.s32Height);
#endif

    }
    else
    {
        pSlvWinAttr->stOutRect = pWinAttr->stOutRect;
    }

    return MT_SUCCESS;
}

mt_s32 WinGetSlaveWinAttr2(WINDOW_S *pstWin,
        MT_DRV_WIN_ATTR_S *pWinAttr,
        MT_DRV_WIN_ATTR_S *pSlvWinAttr)
{
    MT_DISP_DISPLAY_INFO_S stDispInfo, stSlaveDispInfo;
    mt_rect_s stCanvas;
    mt_rect_s stMastOutRect;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;

    memset((void *)&stDispInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));
    memset((void *)&stSlaveDispInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));

    /* may change when window lives */
    pSlvWinAttr->stCustmAR = pWinAttr->stCustmAR;
    pSlvWinAttr->enARCvrs = pWinAttr->enARCvrs;

    pSlvWinAttr->bUseCropRect = pWinAttr->bUseCropRect;
    pSlvWinAttr->stInRect = pWinAttr->stInRect;
    pSlvWinAttr->stCropRect = pWinAttr->stCropRect;

    if (pstWin->bVirtScreenMode)
    {
        pSlvWinAttr->stOutRect = pWinAttr->stOutRect;
    }
    else
    {
        (mt_void) DISP_GetDisplayInfo(pWinAttr->enDisp, &stDispInfo);
        (mt_void) DISP_GetDisplayInfo(pSlvWinAttr->enDisp, &stSlaveDispInfo);

        stCanvas = stDispInfo.stFmtResolution;
        stOffsetInfo.u32Left = 0;
        stOffsetInfo.u32Top = 0;
        stOffsetInfo.u32Right = 0;
        stOffsetInfo.u32Bottom = 0;

        stMastOutRect = pWinAttr->stOutRect;
        if ((stMastOutRect.s32Width == 0) || (stMastOutRect.s32Height == 0))
        {
            stMastOutRect.s32Width = stCanvas.s32Width;
            stMastOutRect.s32Height = stCanvas.s32Height;
        }

        (mt_void) WinOutRectSizeConversion(&stCanvas,
                &stOffsetInfo,
                &stSlaveDispInfo.stFmtResolution,
                &stMastOutRect,
                &pSlvWinAttr->stOutRect);

#if 0
        printk("111111--stCanvas:%d,%d, offset:%d,%d,%d,%d; fmt:%d, %d; mout:%d,%d,%d,%d, slv:%d,%d,%d,%d!\n",
                stCanvas.s32Width,
                stCanvas.s32Height,
                stOffsetInfo.u32Left,
                stOffsetInfo.u32Top,
                stOffsetInfo.u32Right,
                stOffsetInfo.u32Bottom,
                stSlaveDispInfo.stFmtResolution.s32Width,
                stSlaveDispInfo.stFmtResolution.s32Height,
                pWinAttr->stOutRect.s32X,
                pWinAttr->stOutRect.s32Y,
                pWinAttr->stOutRect.s32Width,
                pWinAttr->stOutRect.s32Height,
                pSlvWinAttr->stOutRect.s32X,
                pSlvWinAttr->stOutRect.s32Y,
                pSlvWinAttr->stOutRect.s32Width,
                pSlvWinAttr->stOutRect.s32Height);
#endif
    }

    return MT_SUCCESS;
}

mt_s32 WinTestZero(volatile mt_u32 *pLock, mt_u32 u32MaxTimeIn10ms)
{
    volatile mt_u32 nLockState;
    mt_u32 u = 0;

    while (u < u32MaxTimeIn10ms)
    {
        nLockState = *pLock;
        if (!nLockState)
        {
            return MT_SUCCESS;
        }

        DISP_MSLEEP(10);
        u++;
    }

    return MT_ERR_VO_TIMEOUT;
}

mt_s32 WinCheckFrame(MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    MT_DRV_VIDEO_PRIVATE_S *pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pFrameInfo->u32Priv[0]);

    pstPriv->u32PlayTime = 1;

    if (pFrameInfo->eFrmType > MT_DRV_FT_BUTT)
    {
        WIN_FATAL("Q Frame type error : %d\n", pFrameInfo->eFrmType);
        return MT_ERR_VO_INVALID_PARA;
    }

    if (!((MT_DRV_PIX_FMT_NV12 == pFrameInfo->ePixFormat) || (MT_DRV_PIX_FMT_NV21 == pFrameInfo->ePixFormat)))
    {
        WIN_FATAL("Q Frame pixformat error : %d\n", pFrameInfo->ePixFormat);
        return MT_ERR_VO_INVALID_PARA;
    }

    if ((pFrameInfo->u32Width < WIN_FRAME_MIN_WIDTH) || (pFrameInfo->u32Width > WIN_FRAME_MAX_WIDTH) ||
            (pFrameInfo->u32Height < WIN_FRAME_MIN_HEIGHT) || (pFrameInfo->u32Height > WIN_FRAME_MAX_HEIGHT))
    {
        WIN_FATAL("Q Frame resolution error : w=%d,h=%d\n",
                pFrameInfo->u32Width, pFrameInfo->u32Height);
        return MT_ERR_VO_INVALID_PARA;
    }

    pFrameInfo->stDispRect.s32X = 0;
    pFrameInfo->stDispRect.s32Y = 0;
    pFrameInfo->stDispRect.s32Width = pFrameInfo->u32Width;
    pFrameInfo->stDispRect.s32Height = pFrameInfo->u32Height;

    /*
       if (   (pFrameInfo->stDispRect.s32X < 0)
       || (pFrameInfo->stDispRect.s32Width <  0)
       || ((pFrameInfo->stDispRect.s32Width + pFrameInfo->stDispRect.s32X) >  pFrameInfo->u32Width)
       || (pFrameInfo->stDispRect.s32Y < 0)
       || (pFrameInfo->stDispRect.s32Height < 0)
       || ((pFrameInfo->stDispRect.s32Height + pFrameInfo->stDispRect.s32Y) >    pFrameInfo->u32Height)
       )
       {
       return MT_ERR_VO_INVALID_PARA;
       }

       if (  (pFrameInfo->stDispAR.u8ARh > (pFrameInfo->stDispAR.u8ARw * WIN_MAX_ASPECT_RATIO))
       ||(pFrameInfo->stDispAR.u8ARw > (pFrameInfo->stDispAR.u8ARh  * WIN_MAX_ASPECT_RATIO))
       )
       {
       return MT_ERR_VO_INVALID_PARA;
       }

       if (pFrameInfo->u32FrameRate >    WIN_MAX_FRAME_RATE)
       {
       return MT_ERR_VO_INVALID_PARA;
       }

       if (pFrameInfo->eColorSpace >  MT_DRV_CS_SMPT240M)
       {
       return MT_ERR_VO_INVALID_PARA;
       }

       if (pFrameInfo->u32PlayTime >  WIN_MAX_FRAME_PLAY_TIME)
       {
       return MT_ERR_VO_INVALID_PARA;
       }

    // stBufAddr[1] is right eye for stereo video
    if (    (pFrameInfo->stBufAddr[0].u32Stride_Y < pFrameInfo->stDispRect.s32Width)
    ||    (pFrameInfo->stBufAddr[0].u32Stride_C < pFrameInfo->stDispRect.s32Width)
    )
    {
    return MT_ERR_VO_INVALID_PARA;
    }
    */

    return MT_SUCCESS;
}

/* window buffer manager */
mt_s32 WinBufferReset(WIN_BUFFER_S *pstBuffer)
{

    DISP_MEMSET(&pstBuffer->stUselessFrame, 0,
            sizeof(MT_DRV_VIDEO_FRAME_S) * WIN_USELESS_FRAME_MAX_NUMBER);

    pstBuffer->u32ULSRdPtr = 0;
    pstBuffer->u32ULSWtPtr = 0;

    pstBuffer->u32ULSIn = 0;
    pstBuffer->u32ULSOut = 0;
    pstBuffer->u32UnderLoad = 0;

    return MT_SUCCESS;
}

mt_s32 WinBufferPutULSFrame(WIN_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_u32 WP1;

    WP1 = (pstBuffer->u32ULSWtPtr + 1) % WIN_USELESS_FRAME_MAX_NUMBER;

    if (WP1 == pstBuffer->u32ULSRdPtr)
    {
        WIN_ERROR("usl full\n");
        return MT_ERR_VO_BUFQUE_FULL;
    }

    pstBuffer->stUselessFrame[pstBuffer->u32ULSWtPtr] = *pstFrame;

    pstBuffer->u32ULSWtPtr = WP1;
    pstBuffer->u32ULSIn++;

    return MT_SUCCESS;
}

mt_s32 WinBufferGetULSFrame(WIN_BUFFER_S *pstBuffer, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    if (pstBuffer->u32ULSWtPtr == pstBuffer->u32ULSRdPtr)
    {
        return MT_FAILURE;
    }

    *pstFrame = pstBuffer->stUselessFrame[pstBuffer->u32ULSRdPtr];
    pstBuffer->u32ULSRdPtr = (pstBuffer->u32ULSRdPtr + 1) % WIN_USELESS_FRAME_MAX_NUMBER;
    pstBuffer->u32ULSOut++;

    return MT_SUCCESS;
}

static mt_s32 WIN_Create_Seperate(MT_DRV_WIN_ATTR_S *pWinAttr,
        mt_handle *phWin,
        mt_u32 u32BufNum,
        WINDOW_S **pWindowReturn)
{
    WINDOW_S *pWindow = MT_NULL;
    mt_s32 nRet = MT_SUCCESS;
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    int win_index = 0;
    MT_DRV_DISPLAY_E enDisp;
     
    WIN_DEBUGK("%s %d, pWinAttr 0x%lx  phWin 0x%lx  %d pWindowReturn 0x%lx\n", __FUNCTION__, __LINE__, (ulong)pWinAttr, (ulong)phWin, u32BufNum, (ulong)pWindowReturn);
    if (DISP_IsOpened(pWinAttr->enDisp) != MT_TRUE)
    {
        WIN_DEBUGK("DISP is not opened! %s\n", __FUNCTION__);
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = WinCreateDisplayWindow(pWinAttr, &pWindow, u32BufNum);
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);
    if (nRet)
    {
        WIN_DEBUGK("WinCreateDisplayWindow failed! %s\n", __FUNCTION__);

        return nRet;
    }

    (mt_void) DISP_GetDisplayInfo(pWinAttr->enDisp, &stDispInfo);
    WinUpdateDispInfo_InitialFmt(pWindow, &stDispInfo);
    WinUpdateDispInfo(pWindow, &stDispInfo);
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);

    nRet = WinRegCallback(pWindow);
    if (nRet)
    {
        goto __ERR_RET_DESTROY__;
    }
    nRet = WinAddWindow(pWindow->enDisp, pWindow);
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);
    if (nRet)
    {
        WIN_DEBUGK("WinAddWindow failed! %s\n", __FUNCTION__);

        goto __ERR_RET_UNREG_;
    }
 
    WinGetIndex(pWindow->u32Index, &enDisp, &win_index); 
    if(win_index >= MAX_WIN_NUM)
    {
        WIN_DEBUGK("WinAddWindow failed! %s\n", __FUNCTION__);
        nRet = MT_ERR_DISP_CREATE_ERR;
        goto __ERR_RET_UNREG_;
    }
    WIN_DEBUGK("%s %d win_index:%d, bUseSubLayer:%d, bSetVideoBot:%d\n", __FUNCTION__, __LINE__, 
    win_index, pWinAttr->bUseSubLayer, pWinAttr->bSetVideoBot);

    if(pWinAttr->bUseSubLayer && pWinAttr->bSetVideoBot) //set sub video on top
    {
        drv_reg_4k_disp_set_hd_video_bot_flag(1);
        for(int i = 0; i < MAX_WIN_NUM; i++)
            g_stDrvSetting[i].b_set_sub_video_layer_top = 1; 
    }

    g_pstWin[win_index] = pWindow ;
    {
        mt_u32 w, h;
        disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &h, &w);
        g_stDrvSetting[win_index].eDisplayStatus = DF_STATUS_PLAY;
        memset(&g_stDrvSetting[win_index].window_xy_HD, 0, sizeof(stWindow_XY));
        memset(&g_stDrvSetting[win_index].window_xy_SD, 0, sizeof(stWindow_XY));
        g_stDrvSetting[win_index].crop_xy.bEnable = MT_FALSE;
        DF_SetCmd(g_pstWin[win_index]->stBuffer.stDispBP, DF_CMD_REGISTER_EVENT_CB, DF_Event_func);
        if (w == 0 || h == 0)
        {
            g_stDrvSetting[win_index].window_xy_HD.u32Xstart = 0;
            g_stDrvSetting[win_index].window_xy_HD.u32Xend = MT_HD_WIDTH-1;
            g_stDrvSetting[win_index].window_xy_HD.u32Ystart = 0;
            g_stDrvSetting[win_index].window_xy_HD.u32Yend = MT_HD_HEIGHT-1;

        }
        else
        {
            g_stDrvSetting[win_index].window_xy_HD.u32Xstart = 0;
            g_stDrvSetting[win_index].window_xy_HD.u32Xend = w-1;
            g_stDrvSetting[win_index].window_xy_HD.u32Ystart = 0;
            g_stDrvSetting[win_index].window_xy_HD.u32Yend = h-1;
        }

        g_stDrvSetting[win_index].b_use_subvideo_layer = pWinAttr->bUseSubLayer;
        g_stDrvSetting[win_index].b_set_sub_video_layer_top = pWinAttr->bSetVideoBot;
    }

    atomic_set(&bCFGVideoFlag[win_index], 1);
    b_disp_coeff_update = 1;
    WIN_DEBUGK("%s %d set b_disp_coeff_update \n", __FUNCTION__, __LINE__);

    *pWindowReturn = pWindow;
    return MT_SUCCESS;
__ERR_RET_UNREG_:
    WinUnRegCallback(pWindow);
__ERR_RET_DESTROY__:
    WinDestroyDisplayWindow(pWindow);
    return nRet;
}

__attribute__((unused)) static mt_s32 WIN_Destroy_Seperate(WINDOW_S *pWindow)
{
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);
    
    WinUnRegCallback(pWindow);
    WinDelWindow(pWindow->u32Index);
    WinDestroyDisplayWindow(pWindow);

    return MT_SUCCESS;
}

/******************************************************************************
  apply function
 ******************************************************************************/
mt_s32 WIN_Create(MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWin, MT_BOOL bVirtScreen)
{
    mt_s32 nRet = MT_SUCCESS;
    WINDOW_S *pWindow = MT_NULL;
    mt_u32 index_video = Win_GetVideoWinIndex();
    mt_u32 index_still = Win_GetStillWinIndex();
        
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);

    WinCheckDeviceOpen();
    WinCheckNullPointer(pWinAttr);
    WinCheckNullPointer(phWin);

    if(((index_video < MAX_WIN_NUM) && (pWinAttr->bUseSubLayer == 0))
        || ((index_still < MAX_WIN_NUM) && (pWinAttr->bUseSubLayer == 1)))
    {
        WIN_DEBUGK("WIN already exists! %s index[%d %d] bUseSubLayer[%d]\n", 
            __FUNCTION__, index_video, index_still, pWinAttr->bUseSubLayer);
        return MT_ERR_VO_DEV_CLOSE_ERR;
    }

    // s1 check attribute
    nRet = WinCheckAttr(pWinAttr, bVirtScreen);
    if (nRet)
    {
        WIN_DEBUGK("WinAttr is invalid!\n");
        return nRet;
    }

    if (!WinGetRegWinManageStatus())
    {
        WinRegWinManageCallback(MT_DRV_DISPLAY_0);
        WinRegWinManageCallback(MT_DRV_DISPLAY_1);
        WinSetRegWinManageStatus(MT_TRUE);
    }
    fcnt_vir=0;
    rcnt_vir=0;
    if (pWinAttr->bVirtual != MT_TRUE)
    {
        nRet = WIN_Create_Seperate(pWinAttr, phWin, WIN_IN_FB_DEFAULT_NUMBER, &pWindow);
        if (nRet)
        {
            WIN_DEBUGK("WWIN_Create_Seperate failed!\n");

            return nRet;
        }

        if (DISP_IsFollowed(pWinAttr->enDisp))
        {
#if 0 //not_need
            MT_DRV_WIN_ATTR_S stSlvWinAttr;
            WINDOW_S *pSlaveWindow = MT_NULL;
            nRet = WinGetSlaveWinAttr(pWinAttr, &stSlvWinAttr, bVirtScreen);
            if (nRet)
            {
                WIN_DEBUGK("WinAttr is invalid!\n");
                goto __ERR_RET_UNREG_CB__;
            }

            nRet = WIN_Create_Seperate(&stSlvWinAttr,
                    phWin,
                    WIN_IN_FB_DEFAULT_NUMBER + 4,
                    &pSlaveWindow);
            if (nRet)
            {
                WIN_DEBUGK("Creat slave window failed!\n");
                goto __ERR_RET_UNREG_CB__;
            }

            /*pointer to each other.*/
            pWindow->hSlvWin = (mt_handle)(pSlaveWindow->u32Index);
            pSlaveWindow->pstMstWin = (mt_handle)pWindow;

            /*give a value of the type.*/
            pWindow->enType = MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE;
            pSlaveWindow->enType = MT_DRV_WIN_ACTIVE_SLAVE ;

            pWindow->bVirtScreenMode = bVirtScreen;
            pSlaveWindow->bVirtScreenMode = bVirtScreen;
#endif
        }
        else
        {
            pWindow->enType = MT_DRV_WIN_ACTIVE_SINGLE;
            pWindow->bVirtScreenMode = bVirtScreen;
        }

        *phWin = (mt_handle)(pWindow->u32Index);
        return MT_SUCCESS;
    }
    else
    {
        VIRTUAL_S *pstVirWindow = MT_NULL;

        nRet = WIN_VIR_Create(pWinAttr, &pstVirWindow);

        MT_INFO_VO("create vir win ret %d\n", nRet);
        if (nRet)
            return nRet;

        if(pstVirWindow)
            pstVirWindow->enType = MT_DRV_WIN_VITUAL_SINGLE;
        rls_cnt_vir = 0;
        // add to virtual window array, if win_deinit, auto destory it
        nRet = WinAddVirWindow(pstVirWindow);

        MT_INFO_VO("add vir win ret %d\n", nRet);
        if (nRet)
        {
            WIN_VIR_Destroy(pstVirWindow);
            return nRet;
        }
        else
        {
            if(pstVirWindow)
                *phWin = (mt_handle)(pstVirWindow->u32Index);
            
            return MT_SUCCESS;
        }
    }
}

mt_s32 WIN_CheckAttachState(mt_handle hWin, MT_BOOL *pbSrcAttached, MT_BOOL *pbSinkAttached)
{
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WINDOW_S *pstWin;

        WinCheckWindow(hWin, pstWin);

        if (pstWin->stCfg.stSource.hSrc == MT_INVALID_HANDLE) {
            *pbSrcAttached = MT_FALSE;
        }
        else
        {
            *pbSrcAttached = MT_TRUE;
        }

        *pbSinkAttached = MT_FALSE;
    }
    else
    {
        VIRTUAL_S *pstVirWin;
        WinCheckVirWindow(hWin, pstVirWin);

        if (pstVirWin->hSink == MT_INVALID_HANDLE)
        {
            *pbSinkAttached = MT_FALSE;
        }
        else
        {
            *pbSinkAttached = MT_TRUE;
        }

        if (pstVirWin->stSrcInfo.hSrc == MT_INVALID_HANDLE)
        {
            *pbSrcAttached = MT_FALSE;
        }
        else
        {
            *pbSrcAttached = MT_TRUE;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WIN_Destroy(mt_handle hWin)
{
    WINDOW_S *pstWin;
    mt_s32 nRet = MT_SUCCESS;
    mt_s32 t = 0;
    MT_BOOL bVirtual;
    int win_index = 0;
    MT_DRV_DISPLAY_E enDisp;    
    WIN_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);

    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    atomic_read(&bISRProcessFlag);
    t = 0;
    while (atomic_read(&bISRProcessFlag))
    {
        DISP_MSLEEP(10);
        t++;
        if (t > 10)
        {
            break;
        }
    }
    
    mmz_phy_addr = 0;

    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);

        WinGetIndex(hWin, &enDisp, &win_index); 
        WIN_DEBUGK("%s %d bUseSubLayer:%d, win_index:%d\n", __FUNCTION__, __LINE__, 
            pstWin->stCfg.stAttr.bUseSubLayer, win_index);
        
        atomic_set(&bCFGVideoFlag[win_index], 0);
        if(pstWin->stCfg.stAttr.bUseSubLayer)
        {
            drv_reg_4k_disp_set_hd_video_bot_flag(0);  //fix black screen issue, hardware limitation
            for(int i = 0; i < MAX_WIN_NUM; i++)
                g_stDrvSetting[i].b_set_sub_video_layer_top = 0; 
        }
        if (pstWin->bEnable == MT_TRUE)
        {
            nRet = WIN_SetEnable(hWin, MT_FALSE);
        }

        if (pstWin->enType != MT_DRV_WIN_VITUAL_SINGLE)
        {

            if (pstWin->hSlvWin)
            {
                WIN_Destroy(pstWin->hSlvWin);
            }

            WinUnRegCallback(pstWin);
            WinDelWindow(pstWin->u32Index);
            WinDestroyDisplayWindow(pstWin);
        }
    }
    else
    {
        VIRTUAL_S *pstVirWin;

        WinCheckVirWindow(hWin, pstVirWin);
        WinGetIndex(pstVirWin->u32Index, &enDisp, &win_index); 
        atomic_set(&bCFGVideoFlag[win_index], 0);

        WinDelVirWindow(hWin);

        nRet = WIN_VIR_Destroy(pstVirWin);
    }

    return MT_SUCCESS;
}
mt_s32 WIN_Set_DF_Window(int index ,mt_rect_s *src_rec,mt_rect_s *dst_rec)
{
    mt_u32 w,h;
    DISP_DEBUGK(" func [%s] lin %d \n", __FUNCTION__, __LINE__);
    disp_st_vid_get_vout_size(DISP_CHANNEL_HD,&h,&w);
    //memset(&g_stDrvSetting.window_xy_HD , 0 , sizeof(stWindow_XY));
    //memset(&g_stDrvSetting.window_xy_SD , 0 , sizeof(stWindow_XY));
    g_stDrvSetting[index].crop_xy.bEnable=MT_FALSE;
    if(w==0 || h==0)
    {
        g_stDrvSetting[index].window_xy_HD.u32Xstart = 0;
        g_stDrvSetting[index].window_xy_HD.u32Xend = MT_HD_WIDTH-1;
        g_stDrvSetting[index].window_xy_HD.u32Ystart = 0;
        g_stDrvSetting[index].window_xy_HD.u32Yend = MT_HD_HEIGHT-1;
    }
    else
    {
        g_stDrvSetting[index].window_xy_HD.u32Xstart = 0;
        g_stDrvSetting[index].window_xy_HD.u32Xend = w-1;
        g_stDrvSetting[index].window_xy_HD.u32Ystart = 0;
        g_stDrvSetting[index].window_xy_HD.u32Yend = h-1;
        if(dst_rec->s32Width <= 0 ||dst_rec->s32Height <= 0 || dst_rec->s32X < 0 || dst_rec->s32Y < 0)
            return MT_FAILURE;
        if(dst_rec->s32X > 0 && dst_rec->s32X < w)
            g_stDrvSetting[index].window_xy_HD.u32Xstart = dst_rec->s32X;
        if((dst_rec->s32X + dst_rec->s32Width <= w) && (dst_rec->s32X + dst_rec->s32Width > 2))
            g_stDrvSetting[index].window_xy_HD.u32Xend = dst_rec->s32X + dst_rec->s32Width;
        if(dst_rec->s32Y > 0 ||dst_rec->s32Y < h)
            g_stDrvSetting[index].window_xy_HD.u32Ystart = dst_rec->s32Y;
        if((dst_rec->s32Y + dst_rec->s32Height <= h) && (dst_rec->s32Y + dst_rec->s32Height > 2))
            g_stDrvSetting[index].window_xy_HD.u32Yend = dst_rec->s32Y + dst_rec->s32Height;
    }
    
    return MT_SUCCESS;

}
#if 0
mt_s32 WIN_Set_ScalerAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{

}
#endif

mt_s32 WIN_SetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    WINDOW_S *pstWin;
    MT_DRV_WIN_ATTR_S stSlvWinAttr;
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    mt_s32 nRet = MT_SUCCESS;
    mt_s32 t = 0;
    MT_BOOL bVirtual;
    WINDOW_S *pstWinTmp = MT_NULL;
    mt_rect_s stOutRectOrigin, stOutRectRevised;
    mt_s32 index = 0;
    MT_DRV_DISPLAY_E enDisp;
    WinCheckDeviceOpen();
    WinCheckNullPointer(pWinAttr);
    
    WinGetIndex(hWin, &enDisp, &index);

    memset((void *)&stSlvWinAttr, 0, sizeof(MT_DRV_WIN_ATTR_S));
    bVirtual = WinCheckVirtual(hWin);

    WIN_DEBUGK("%s In_ori [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stInRect.s32X, 
            pWinAttr->stInRect.s32Y, 
            pWinAttr->stInRect.s32Width, 
            pWinAttr->stInRect.s32Height);
    WIN_DEBUGK("%s Out_ori [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stOutRect.s32X, 
            pWinAttr->stOutRect.s32Y, 
            pWinAttr->stOutRect.s32Width, 
            pWinAttr->stOutRect.s32Height);

    WIN_DEBUGK("%s Crop_ori [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stCropRect.u32LeftOffset, 
            pWinAttr->stCropRect.u32RightOffset, 
            pWinAttr->stCropRect.u32TopOffset, 
            pWinAttr->stCropRect.u32BottomOffset);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        nRet = WinCheckFixedAttr(&pstWin->stCfg.stAttrBuf, pWinAttr);
        if (nRet)
        {
            return nRet;
        }

        nRet = WinCheckAttr(pWinAttr, pstWin->bVirtScreenMode);
        if (nRet)
        {
            return nRet;
        }

        nRet = DISP_GetDisplayInfo(pstWin->enDisp, &stDispInfo);
        if (nRet)
        {
            WIN_ERROR("DISP_GetDisplayInfo failed in %s!\n", __FUNCTION__);
            return MT_ERR_VO_CREATE_ERR;
        }       

        pstWinTmp = (WINDOW_S *)DISP_MALLOC(sizeof(WINDOW_S));
        if (!pstWinTmp)
        {
            WIN_DEBUGK("%s %d DISP_MALLOC ! size:0x%x \n", __FUNCTION__, __LINE__, (mt_u32)sizeof(WINDOW_S));
            return MT_ERR_VO_MALLOC_FAILED;
        }

        /*let a tmp window to join the calculate.*/
        *pstWinTmp = *pstWin;
        pstWinTmp->stCfg.stAttrBuf = *pWinAttr;
        /*update the temporary window's initial fmt.*/
        WinUpdateDispInfo_InitialFmt(pstWinTmp, &stDispInfo);
        WinUpdateDispInfo(pstWinTmp, &stDispInfo);

        stOutRectOrigin = pstWinTmp->stCfg.stAttrBuf.stOutRect;
        if ((stOutRectOrigin.s32Width == 0) || (stOutRectOrigin.s32Height == 0))
        {
            if (pstWinTmp->bVirtScreenMode)
                stOutRectOrigin = stDispInfo.stVirtaulScreen;
            else
                stOutRectOrigin = stDispInfo.stFmtResolution;
        }

        WinOutRectSizeConversionByType(&stDispInfo,
                &stOutRectOrigin,
                &stOutRectRevised,
                pstWinTmp);

        /*to change the tmp window's using outrect.*/
        pstWinTmp->stUsingAttr.stOutRect = stOutRectRevised;

        /*to judge the layout valid or not.*/
        nRet = WindowRedistributeProcess(pstWinTmp);
        if (nRet)
        {
            DISP_FREE(pstWinTmp);
            WIN_ERROR("reallocate video layer failed in %s!\n", __FUNCTION__);
            return nRet;
        }

        /*update the formal window's initial fmt.*/
        WinUpdateDispInfo_InitialFmt(pstWin, &stDispInfo);
        WinUpdateDispInfo(pstWin, &stDispInfo);

        atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
        pstWin->stCfg.stAttrBuf = *pWinAttr;
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 1);

        /*wait for the hal resources:such as physical layers, zorder
          to take effect.*/
        (mt_void) WindowRedistributeProcess_Wait(pstWin);
        /*wait for software vars to take effect.*/
        while (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            DISP_MSLEEP(5);
            t++;

            if (t > 10)
            {
                break;
            }
        }

        if (pstWin->hSlvWin)
        {
            WIN_GetAttr(pstWin->hSlvWin, &stSlvWinAttr);

            WinGetSlaveWinAttr2(pstWin, pWinAttr, &stSlvWinAttr);

            nRet = WIN_SetAttr(pstWin->hSlvWin, &stSlvWinAttr);
        }

        if (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
            WIN_ERROR("WIN Set Attr timeout in %s\n", __FUNCTION__);
            DISP_FREE(pstWinTmp);
            return MT_ERR_VO_TIMEOUT;
        }

        DISP_FREE(pstWinTmp);

        if((pWinAttr->stOutRect.s32Width == 0) || (pWinAttr->stOutRect.s32Height == 0))
        {
            pWinAttr->stOutRect = stDispInfo.stFmtResolution;
        }
        if((pWinAttr->stInRect.s32Width == 0) || (pWinAttr->stInRect.s32Height == 0))
        {
            pWinAttr->stInRect = stDispInfo.stFmtResolution;
        }

        smallwindow_rect[index] =  pWinAttr->stOutRect;
        barrier();
        atomic_set(&smallwindow_flag[index], 1);

        cropwindow_rect[index] = pWinAttr->stInRect;
        barrier();
        atomic_set(&cropwindow_flag[index], 1);

        switch(pWinAttr->enARCvrs)
        {
        case MT_DRV_ASP_RAT_MODE_FULL:
            g_stDrvSetting[index].out_ar_mode = DF_ASPECT_MODE_AUTO;
            break;
        case MT_DRV_ASP_RAT_MODE_LETTERBOX:
            g_stDrvSetting[index].out_ar_mode = DF_ASPECT_MODE_LETTERBOX;
            break;
        case MT_DRV_ASP_RAT_MODE_PANANDSCAN:
            g_stDrvSetting[index].out_ar_mode = DF_ASPECT_MODE_PANSCAN;
            break;
        case MT_DRV_ASP_RAT_MODE_COMBINED:
            g_stDrvSetting[index].out_ar_mode = DF_ASPECT_MODE_ORIG;
            break;
        default:
            g_stDrvSetting[index].out_ar_mode = DF_ASPECT_MODE_AUTO;
            break;
        }
		WIN_DEBUGK("%s subvideo:%d video_bot:%d\n", __FUNCTION__, pWinAttr->bUseSubLayer, pWinAttr->bSetVideoBot);
       
        g_stDrvSetting[index].b_use_subvideo_layer = pWinAttr->bUseSubLayer;
        g_stDrvSetting[index].b_set_sub_video_layer_top = pWinAttr->bSetVideoBot;
        if(pWinAttr->bUseSubLayer && pWinAttr->bSetVideoBot) //set sub video on top
        {
            drv_reg_4k_disp_set_hd_video_bot_flag(1);
            for(int i = 0; i < MAX_WIN_NUM; i++)
                g_stDrvSetting[i].b_set_sub_video_layer_top = 1; 
        }

        t = 0;
        while(atomic_read(&smallwindow_flag[index]) || atomic_read(&cropwindow_flag[index]))
        {
            msleep(5);
            t++;

            if (t > 10)
            {
                break;
            }
        }
    }
    else
    {
        //VIRTUAL_S *pstVirWindow;
        //WinCheckVirWindow(hWin, pstVirWindow);
        //nRet = WIN_VIR_SetAttr(pstVirWindow, pWinAttr);
        return MT_ERR_VO_INVALID_OPT;
    }

    WIN_DEBUGK("%s In [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stInRect.s32X, 
            pWinAttr->stInRect.s32Y, 
            pWinAttr->stInRect.s32Width, 
            pWinAttr->stInRect.s32Height);
    WIN_DEBUGK("%s Out [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stOutRect.s32X, 
            pWinAttr->stOutRect.s32Y, 
            pWinAttr->stOutRect.s32Width, 
            pWinAttr->stOutRect.s32Height);

    WIN_DEBUGK("%s Crop [%d %d %d %d]\n", __FUNCTION__,  
            pWinAttr->stCropRect.u32LeftOffset, 
            pWinAttr->stCropRect.u32RightOffset, 
            pWinAttr->stCropRect.u32TopOffset, 
            pWinAttr->stCropRect.u32BottomOffset);
    return nRet;
}

mt_s32 WIN_GetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pWinAttr);
    bVirtual = WinCheckVirtual(hWin);

    if (!bVirtual)
    {
        WINDOW_S *pstWin;

        WinCheckWindow(hWin, pstWin);
        *pWinAttr = pstWin->stCfg.stAttr;
    }
    else
    {
        VIRTUAL_S *pstVirWindow;

        WinCheckVirWindow(hWin, pstVirWindow);
        *pWinAttr = pstVirWindow->stAttrBuf;
    }

    return MT_SUCCESS;
}

//get info for source
mt_s32 WIN_GetInfo(mt_handle hWin, MT_DRV_WIN_INFO_S *pstInfo)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstInfo);

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        pstInfo->eType = WinGetType(pstWin);
        pstInfo->hPrim = (mt_handle)(pstWin->u32Index);
        pstInfo->hSec = (mt_handle)(pstWin->hSlvWin);
    }
    else
    {
        VIRTUAL_S *pstVirWin;
        WinCheckVirWindow(hWin, pstVirWin);

        pstInfo->eType = pstVirWin->enType;
        pstInfo->hPrim = (mt_handle)(pstVirWin->u32Index);
        pstInfo->hSec = MT_INVALID_HANDLE;
    }

    return MT_SUCCESS;
}

mt_s32 WIN_SetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    DISP_SOURCE_INFO_S stSrc2Buf;
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    mt_s32 nRet = MT_SUCCESS;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstSrc);

    if (pstSrc->hSrc == MT_INVALID_HANDLE)
    {
        MT_BOOL bEnable = MT_FALSE;
        (mt_void) WIN_GetEnable(hWin, &bEnable);
        if (bEnable == MT_TRUE)
        {
            DISP_ERROR("Window is still working,can't be detached,please disable it first\n");
            return MT_ERR_VO_INVALID_OPT;
        }
        else
        {
            nRet = WIN_Reset(hWin, MT_DRV_WIN_SWITCH_BLACK);
            if (MT_SUCCESS != nRet)
            {
                DISP_ERROR("Reset Window Failed\n");
                return nRet;
            }
        }
    }

    bVirtual = WinCheckVirtual(hWin);

    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);

        nRet = DISP_GetDisplayInfo(WinGetDispID(pstWin), &stDispInfo);
        if (nRet)
        {
            return nRet;
        }

        if (pstWin->stCfg.stSource.hSrc == pstSrc->hSrc)
        {
            if (pstSrc->hSrc)
                WIN_ERROR("Attach repeately!\n");
            else
                WIN_ERROR("Detach repeately!\n");

            return MT_ERR_VO_OPERATION_DENIED;
        }

        pstWin->stCfg.stSource = *pstSrc;

        stSrc2Buf.hSrc = pstSrc->hSrc;
        stSrc2Buf.pfAcqFrame = (PFN_DF_GET_FRAME_CALLBACK)pstSrc->pfAcqFrame;
        stSrc2Buf.pfRlsFrame = (PFN_DF_PUT_FRAME_CALLBACK)pstSrc->pfRlsFrame;
        //hmc
        stSrc2Buf.pfAcqFreezeFrame = (PFN_DF_GET_FRAME_CALLBACK)pstSrc->pfAcqFreezeFrame;
        stSrc2Buf.pfRlsFreezeFrame = (PFN_DF_PUT_FRAME_CALLBACK)pstSrc->pfRlsFreezeFrame;

        //nRet =  WinBuf_SetSource(&pstWin->stBuffer.stWinBP, &stSrc2Buf);
        nRet = DF_SetSource(pstWin->stBuffer.stDispBP, &stSrc2Buf);

        if (nRet)
        {
            return nRet;
        }

        // send attr to source
        WinSendAttrToSource(pstWin, &stDispInfo);

        // use DF_SetFreezeBuffer instead of DF_FreezeMalloc
#if 0
        //Patch: for global freeze buffer, should allocate at first! or may has memory holes issue!
        if (stSrc2Buf.pfAcqFreezeFrame == MT_NULL && pstSrc->hSrc != MT_INVALID_HANDLE/*Attach*/)
        {
            DISP_PRINT("WIN_SetSource: do global freeze buffer allocation.\n");
            DF_FreezeMalloc(pstWin->stBuffer.stDispBP);
        }
#endif
 //       WIN_DEBUGK("WIN_SetSource :s=0x%lx, info=0x%llx, pfAcqFrame=0x%llx,pfRlsFrame=0x%llx\n",
 //               pstSrc->hSrc, (mt_u64)pstSrc->pfSendWinInfo,
 //               (mt_u64)pstSrc->pfAcqFrame, (mt_u64)pstSrc->pfRlsFrame);
    }
    else
    {
        VIRTUAL_S *pstVirWin;
        WinCheckVirWindow(hWin, pstVirWin);

        if (pstVirWin->stSrcInfo.hSrc == pstSrc->hSrc)
        {
            if (pstSrc->hSrc)
                WIN_ERROR("Attach repeately!\n");
            else
                WIN_ERROR("Detach repeately!\n");

            return MT_ERR_VO_OPERATION_DENIED;
        }

        pstVirWin->stSrcInfo = *pstSrc;

        WIN_VIR_SendAttrToSource(pstVirWin);
    }
    return MT_SUCCESS;
}

mt_s32 WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstSrc);
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        *pstSrc = pstWin->stCfg.stSource;
    }
    else
    {
        VIRTUAL_S *pstVirWin;
        WinCheckVirWindow(hWin, pstVirWin);
        *pstSrc = pstVirWin->stSrcInfo;
    }

    return MT_SUCCESS;
}

mt_s32 WIN_SetEnable_Seperate(WINDOW_S *pstWin, MT_BOOL bEnable)
{
    mt_s32 ret = MT_SUCCESS;
    WINDOW_S *pstWinTmp = MT_NULL;
    mt_rect_s stOutRectOrigin, stOutRectRevised;
    MT_DISP_DISPLAY_INFO_S stDispInfo;

    memset((void *)&stDispInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));
    pstWinTmp = (WINDOW_S *)DISP_MALLOC(sizeof(WINDOW_S));
    if (!pstWinTmp)
        return MT_FAILURE;

    *pstWinTmp = *pstWin;
    pstWinTmp->bEnable = bEnable;

    ret = DISP_GetDisplayInfo(pstWin->enDisp, &stDispInfo);
    if (ret)
    {
        stDispInfo.stFmtResolution.s32X = 0;
        stDispInfo.stFmtResolution.s32Y = 0;
        stDispInfo.stFmtResolution.s32Width = 1280;
        stDispInfo.stFmtResolution.s32Height = 720;
        memset((void *)&stDispInfo.stOffsetInfo, 0, sizeof(MT_DRV_DISP_OFFSET_S));
        WIN_WARN("when call set_enable, may ctrl+c, display closed\n");
    }

    /*here we convert all the coordinate to phisical cooridate,
     * we do in phisical coordinate to judge whether the window  overlapped.
     */
    stOutRectOrigin = pstWinTmp->stCfg.stAttrBuf.stOutRect;

    if ((stOutRectOrigin.s32Width == 0) || (stOutRectOrigin.s32Height == 0))
    {
        if (pstWinTmp->bVirtScreenMode)
            stOutRectOrigin = stDispInfo.stVirtaulScreen;
        else
            stOutRectOrigin = stDispInfo.stFmtResolution;
    }

    WinOutRectSizeConversionByType(&stDispInfo,
            &stOutRectOrigin,
            &stOutRectRevised,
            pstWinTmp);

    /*to change the tmp window's using outrect.*/
    pstWinTmp->stUsingAttr.stOutRect = stOutRectRevised;

    /* when enable,may cause win-layer remapping.*/
    ret = WindowRedistributeProcess(pstWinTmp);
    if (ret)
    {
        DISP_FREE(pstWinTmp);
        return ret;
    }

    pstWin->bEnable = bEnable;
    (mt_void) WindowRedistributeProcess_Wait(pstWinTmp);

    DISP_FREE(pstWinTmp);
    return MT_SUCCESS;
}

mt_s32 WIN_SetEnable(mt_handle hWin, MT_BOOL bEnable)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32SleepTime = 0;

    WinCheckDeviceOpen();

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);

        if ((ret = WIN_SetEnable_Seperate(pstWin, bEnable)))
        {
            WIN_ERROR("main window enable error:%x\n", ret);
            return ret;
        }
#if 0 //not_need
        if ((MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin))
                && (pstWin->hSlvWin))
        {
            WINDOW_S *pstSlvWin;
            WinCheckWindow(pstWin->hSlvWin, pstSlvWin);

            if ((ret = WIN_SetEnable_Seperate(pstSlvWin, bEnable)))
            {
                WIN_ERROR("slave window enable error:%x\n",ret);
                return ret;
            }
        }
#endif
        /*when disable we should sleep, the func is synchronous.*/
        u32SleepTime = pstWin->stDelayInfo.T ? (2 * pstWin->stDelayInfo.T) : (2 * 20);
        if (pstWin->bEnable == MT_FALSE) {
            DISP_MSLEEP(u32SleepTime);
        }
    }
    else
    {
        VIRTUAL_S *pstVirWindow;
        WinCheckVirWindow(hWin, pstVirWindow);
        pstVirWindow->bEnable = bEnable;
    }

    return MT_SUCCESS;
}

mt_s32 WIN_GetEnable(mt_handle hWin, MT_BOOL *pbEnable)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pbEnable);

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        *pbEnable = pstWin->bEnable;
    }
    else
    {
        VIRTUAL_S *pstVirWindow;

        WinCheckVirWindow(hWin, pstVirWindow);

        *pbEnable = pstVirWindow->bEnable;
    }
    return MT_SUCCESS;
}

#define WIN_PROCESS_CALC_TIME_THRESHOLD 10
#define WIN_DELAY_TIME_MAX_CIRCLE 50
mt_s32 WIN_CalcDelayTime(WINDOW_S *pstWin, mt_u32 *pu32BufNum, mt_u32 *pu32DelayMs)
{
#if 0
    mt_u32 u32Num, u32NumNew, T, Dt, Ct, LstCt, Delay;
    mt_u32 L = 0;

__WIN_CALC_DELAY__:
    L++;
    if (L > WIN_PROCESS_CALC_TIME_THRESHOLD)
    {
        goto __WIN_CALC_DELAY_ERROR__;
    }

    LstCt = pstWin->stDelayInfo.u32CfgTime;
    T = pstWin->stDelayInfo.T;

    WinBuf_GetFullBufNum(&pstWin->stBuffer.stWinBP, &u32Num);
    Dt = pstWin->stDelayInfo.u32DisplayTime;

    MT_DRV_SYS_GetTimeStampMs(&Ct);

    if (Ct >= LstCt)
    {
        Delay = Ct - LstCt;
    }
    else
    {
        // circle happen, Ct across zero
        Delay = 0xFFFFFFFFul - LstCt + Ct;
    }

#if 0
    if (Delay > ((T*3)/2) )
    {
        printk("Delay=%d, T = %d\n", Delay, T);
        goto __WIN_CALC_DELAY__;
    }
#endif
    //printk("Delay=%d, Dt = %d\n", Delay, Dt);
    if (T <= (Dt + Delay))
    {
        Delay = 0;
    }
    else
    {
        Delay = T - Dt - Delay;
    }

    Delay = (u32Num + 1) * T + Delay;

    WinBuf_GetFullBufNum(&pstWin->stBuffer.stWinBP, &u32NumNew);
    if (  (LstCt != pstWin->stDelayInfo.u32CfgTime)
            ||(u32NumNew != u32Num)
            ||(MT_TRUE == pstWin->bInInterrupt)
       )
    {
        udelay(100);
        goto __WIN_CALC_DELAY__;
    }

    if (Delay > (T * WIN_DELAY_TIME_MAX_CIRCLE))
    {
        DISP_ASSERT(!Delay);
        goto __WIN_CALC_DELAY_ERROR__;
    }

    *pu32BufNum = u32Num;
    *pu32DelayMs= Delay;
    return MT_SUCCESS;

__WIN_CALC_DELAY_ERROR__:

    *pu32BufNum = 0;
    *pu32DelayMs= 0;
#endif
    return MT_SUCCESS;
}

MT_BOOL WinGetTBMatchInfo(mt_handle hWin)
{
    WINDOW_S *pstWin;

    WinCheckWindow(hWin, pstWin);

    return pstWin->stDelayInfo.bTBMatch;
}
mt_s32 WIN_GetPlayInfo(mt_handle hWin, MT_DRV_WIN_PLAY_INFO_S *pstInfo)
{
    WINDOW_S *pstWin;
    MT_BOOL bFifoEmpty = MT_TRUE;
    //IMAGE *pstImage = &g_testImagInfo;
    //DISP_MMZ_BUF_S *p_stMem = BP_GetHDTestMemInfo();
    //   DISP_DEBUGK("[%s]line %d\n", __FUNCTION__, __LINE__);
    WinCheckDeviceOpen();
    WinCheckNullPointer(pstInfo);
    WinCheckWindow(hWin, pstWin);

    if (!pstWin->bEnable || !pstWin->stDelayInfo.u32DispRate)
    {
        WIN_WARN("window is not ready! state:%d dispRate:%d\n",
                pstWin->bEnable,
                pstWin->stDelayInfo.u32DispRate);
        return MT_ERR_VO_INVALID_OPT;
    }

    WIN_CalcDelayTime(pstWin, &(pstInfo->u32FrameNumInBufQn), &(pstInfo->u32DelayTime));
    //mmz_phy_addr=pstInfo->u32DelayTime;
    pstInfo->u32DelayTime = pstWin->stDelayInfo.u32DisplayTime;
    //pstInfo->u32DelayTime = p_stMem->u32StartPhyAddr;
    pstInfo->u32DispRate = pstWin->stDelayInfo.u32DispRate;
    if(MT_FAILURE == DF_CheckFifoEmpty(pstWin->stBuffer.stDispBP, &bFifoEmpty))
    {
        WIN_WARN("can not get fifoEmpty status!!\n");
        pstInfo->u32FrameNumInBufQn = 0x01; /*0x01 = invalid value*/
        return MT_ERR_VO_OPERATION_DENIED;
    }
    pstInfo->u32FrameNumInBufQn = (bFifoEmpty == MT_TRUE)? 0 : 0xFF;
    return MT_SUCCESS;
}


mt_u32 disp_get_vid_input_info(disp_priv_t *p_dp, MT_DIS_FRAME_SLOT_INFO_T *slotInfo)
{
    //p_dp->cur_3d_flag = slotInfo->is_3D_flag; //3D
    //p_dp->cur_mode_3d = packing_type_to_3d_mode(slotInfo->is_3D_flag, slotInfo->packing_type); //3D
    //p_dp->cur_extpara_3d = slotInfo->is_3D_flag; //3D
    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w = slotInfo->pic_width;
    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h = slotInfo->pic_height;
    p_dp->disp_in_info.b_frame = slotInfo->frame_pic_flag;
    p_dp->disp_in_info.ar = slotInfo->aspect_ratio;
    //get video progressive or interleaved
    p_dp->disp_in_info.b_progressive = slotInfo->progressive_frame;
#if 0
    //b_frameл??
    if(p_dp->disp_in_info.b_frame == 0)
    {
        if(p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w <= DISP_VID_FULLSCR_SD_WIDTH)
        {
            p_dp->disp_in_info.b_frame = 1;
            p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h <<= 1;
        }
    }
    if(load_flag != 0)
    {
        mt_u32 table_sel = reg_aria_disp_get_vscaler_table_sel() & (~load_flag);
        reg_aria_disp_set_vscaler_table_sel(table_sel);
    }
    //clear the load table status
    load_flag = reg_aria_disp_get_vscaler_table_sel() & 0x7ff;
#endif

    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_SD].h = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h;
    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_SD].w = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w;
#if 0
    p_dp->csc_info.cur_transfer_characteristics = slotInfo->transfer_characteristics;
    p_dp->csc_info.cur_colour_primaries = slotInfo->colour_primaries;

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#ifdef CONFIG_MT_VO_SL_HDR
    p_dp->sl_hdr_para.sl_hdr_info.sl_hdr_metadata_flag = slotInfo->sl_hdr_metadata_flag;
    p_dp->sl_hdr_para.sl_hdr_info.sl_hdr_metadata_recovery_flag = slotInfo->sl_hdr_metadata_recovery_flag;
    p_dp->sl_hdr_para.sl_hdr_info.sl_hdr_yuv_range = slotInfo->sl_hdr_yuv_range;
    p_dp->sl_hdr_para.sl_hdr_info.colour_primaries = slotInfo->colour_primaries;
    p_dp->sl_hdr_para.sl_hdr_info.transfer_characteristics = slotInfo->transfer_characteristics;
    p_dp->sl_hdr_para.sl_hdr_info.sl_hdr_metadata_addr_virt = slotInfo->sl_hdr_metadata_addr;
#endif
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)||defined(CONFIG_MT_CHIP_SYMPHONY2)
    p_dp->hdr_info.colour_primaries = slotInfo->colour_primaries;
    p_dp->hdr_info.transfer_characteristics = slotInfo->transfer_characteristics;
#ifdef SUPPORT_HDR
    p_dp->hdr_info.colour_volume.colour_volume_enable = slotInfo->colour_volume.colour_volume_enable;
    p_dp->hdr_info.colour_volume.white_point_x = slotInfo->colour_volume.white_point_x;
    p_dp->hdr_info.colour_volume.white_point_y = slotInfo->colour_volume.white_point_y;
    p_dp->hdr_info.colour_volume.max_luminance = slotInfo->colour_volume.max_luminance;
    p_dp->hdr_info.colour_volume.min_luminance = slotInfo->colour_volume.min_luminance;
    p_dp->hdr_info.colour_volume.display_primaries_x[0] = slotInfo->colour_volume.display_primaries_x[0];
    p_dp->hdr_info.colour_volume.display_primaries_x[1] = slotInfo->colour_volume.display_primaries_x[1];
    p_dp->hdr_info.colour_volume.display_primaries_x[2] = slotInfo->colour_volume.display_primaries_x[2];
    p_dp->hdr_info.colour_volume.display_primaries_y[0] = slotInfo->colour_volume.display_primaries_y[0];
    p_dp->hdr_info.colour_volume.display_primaries_y[1] = slotInfo->colour_volume.display_primaries_y[1];
    p_dp->hdr_info.colour_volume.display_primaries_y[2] = slotInfo->colour_volume.display_primaries_y[2];
    p_dp->hdr_info.light_level.light_level_info_enable = slotInfo->light_level.light_level_info_enable;
    p_dp->hdr_info.light_level.max_light_level = slotInfo->light_level.max_light_level;
    p_dp->hdr_info.light_level.max_pic_ave_light_level = slotInfo->light_level.max_pic_ave_light_level;
#endif
#endif
#endif
    return MT_SUCCESS;
}

#if 0

#include "mt_drv_dma.h"

hal_dma_io_param_t g_dma_param1 = {0};

mt_s32 win_DMA_Test_init(MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    DF_TransformDec2Disp_sinfo(pstFrame, &g_stVideoFrame);

    g_dma_param1.param.len = g_stVideoFrame.u32WidthIn *g_stVideoFrame.u32HeightIn;
    //malloc phy&vir src addr
    //if(g_dma_param1.param.phy_src_addr == 0)
    {
        g_dma_param1.param.phy_src_addr  = g_stVideoFrame.stFrameInfo[MT_DF_BUF_ADDR_LEFT].stField[MT_TOP_FIELD].u32PhyAddr_Y;
    }
    //malloc phy&vir dst addr

    g_dma_param1.param.phy_dst_addr  = pstFrame->stLBufAddr[0].u32YAddr;
    pstFrame->stBufAddr[0].u32Stride_Y = (g_stVideoFrame.u32WidthIn + 31) / 32 * 32;
    pstFrame->stBufAddr[0].u32Stride_C = (g_stVideoFrame.u32WidthIn + 31) / 32 * 32;
    pstFrame->stLBufAddr[0].u32YStride = (g_stVideoFrame.u32WidthIn + 31) / 32 * 32;
    pstFrame->stLBufAddr[0].u32CStride = (g_stVideoFrame.u32WidthIn + 31) / 32 * 32;
    pstFrame->u32Height = g_stVideoFrame.u32HeightIn;
    pstFrame->u32Width= g_stVideoFrame.u32WidthIn;

    return 0;
}

mt_s32 win_DMA_Test(mt_void)
{
    mt_s32            Ret = 0;

    //memset((mt_void *)g_dma_param.param.vir_src_addr, 0x11, g_dma_param.param.len);
    //memset((mt_void *)g_dma_param.param.vir_dst_addr, 0x22, g_dma_param.param.len);

    g_dma_param1.param.config.dst_peripheral = 0xf; // memory
    g_dma_param1.param.config.src_peripheral = 0xf; // memory
    g_dma_param1.param.config.dst_endian = 0; // big endian
    g_dma_param1.param.config.src_endian = 0; // big endian
    g_dma_param1.param.config.dst_clk = 0; // AXI clock
    g_dma_param1.param.config.src_clk = 0; // AXI clock
    g_dma_param1.param.config.dst_i = DMA_ADDR_INC;
    g_dma_param1.param.config.src_i = DMA_ADDR_INC;
    g_dma_param1.param.config.dst_usize = DMA_USIZE_64BIT;
    g_dma_param1.param.config.src_usize = DMA_USIZE_64BIT;
    g_dma_param1.param.config.dst_bsize = DMA_BURST_NUM16;
    g_dma_param1.param.config.src_bsize = DMA_BURST_NUM16;
    g_dma_param1.param.control.int_link_en = 1;
    g_dma_param1.param.control.int_node_en = 1;
    g_dma_param1.param.control.chn_param_reg_en = 0;

    MT_INFO_VO("dma start ...\n");
    g_dma_param1.chn_id = hal_dma_get_free_channel(0);
    hal_dma_start(g_dma_param1.chn_id, (mt_void *)&g_dma_param1.param, (mt_void *)g_dma_param1.p_notify);
    while (DMA_STATUS_STOP != hal_dma_check(g_dma_param1.chn_id));


    hal_dma_stop(g_dma_param1.chn_id);
    MT_INFO_VO("dma over ...\n");

    return Ret;
}


#endif


mt_s32 WinConvertTileFrmToLinear(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    PRESCALE_ORDERINFO TempOrderInfo;
    mt_s32 ret;
    mt_u32 index = 0;
    MT_DRV_DISPLAY_E enDisp;

    WinGetIndex(hWin, &enDisp, &index);
    memset(&TempOrderInfo,0,sizeof(PRESCALE_ORDERINFO));
    DF_TransformDec2Disp_sinfo(MT_NULL, pstFrame, &g_stVideoFrame[index]);

    TempOrderInfo.eMode=FRAMER_FRAMEW_ONEFRAME;
    TempOrderInfo.eOrderPriority=PS_ORDER_PRI_LOW;//PS_ORDER_PRI_LOW;
    TempOrderInfo.eOrderStatus=PS_ORDER_STATUS_NEW;
    TempOrderInfo.u32LumaAddrRead=g_stVideoFrame[index].stFrameInfo[MT_DF_BUF_ADDR_LEFT].stField[MT_TOP_FIELD].u32PhyAddr_Y;
    TempOrderInfo.u32LumaAddrRead2=g_stVideoFrame[index].stFrameInfo[MT_DF_BUF_ADDR_LEFT].stField[MT_BOT_FIELD].u32PhyAddr_Y;
    //TempOrderInfo.u32LumaAddrWrite=pstFrame->stBufAddr[0].u32PhyAddr_Y;
    TempOrderInfo.u32LumaAddrWrite=pstFrame->stLBufAddr[0].u32YAddr;
    TempOrderInfo.u32ChromaAddrRead=g_stVideoFrame[index].stFrameInfo[MT_DF_BUF_ADDR_LEFT].stField[MT_TOP_FIELD].u32PhyAddr_C;
    TempOrderInfo.u32ChromaAddrRead2=g_stVideoFrame[index].stFrameInfo[MT_DF_BUF_ADDR_LEFT].stField[MT_BOT_FIELD].u32PhyAddr_C;
    //TempOrderInfo.u32ChromaAddrWrite=pstFrame->stBufAddr[0].u32PhyAddr_C;
    TempOrderInfo.u32ChromaAddrWrite=pstFrame->stLBufAddr[0].u32CAddr;
    TempOrderInfo.u32SrcWidth=g_stVideoFrame[index].u32WidthIn;
    TempOrderInfo.u32SrcHeight=g_stVideoFrame[index].u32HeightIn;

    TempOrderInfo.u32DstWidth=  g_stVideoFrame[index].u32WidthIn;//stVideoFrame.u32PreSclWidthOut;
    TempOrderInfo.u32DstHeight= g_stVideoFrame[index].u32HeightIn;//stVideoFrame.u32PreSclHeightOut;
    TempOrderInfo.u32DstStride= (g_stVideoFrame[index].u32WidthIn + 31) / 32 * 32;//stVideoFrame.u32PreSclStrideOut;

    TempOrderInfo.row_jump_value=g_stVideoFrame[index].row_jump_value;
    TempOrderInfo.row_jump_offset=g_stVideoFrame[index].row_jump_offset;
    TempOrderInfo.u8TileCfg=g_stVideoFrame[index].u8TileCfg;
    TempOrderInfo.u8ColSize=g_stVideoFrame[index].u8ColSize;
    TempOrderInfo.u8FieldPicture=g_stVideoFrame[index].u8FieldPicture;


    TempOrderInfo.u8InputEndian = 0x8;
    TempOrderInfo.u8OutputEndian = 0xf;
    TempOrderInfo.bIsTileMode = 1;
    TempOrderInfo.u8UVChange = 0x1;

    //printk("0 WinConvertTileFrmToLinear over Y 0x%x ,C 0x%x\n", TempOrderInfo.u32LumaAddrWrite, TempOrderInfo.u32ChromaAddrWrite);

    //make the order
    ret = PS_AddOrder(&TempOrderInfo);

    PS_Loop();

    while(1)
    {
        if(TempOrderInfo.eOrderStatus == PS_ORDER_STATUS_DONE)
            break;
        else
        {
            MT_INFO_VO("wait %d\n",TempOrderInfo.eOrderStatus);

            msleep(5);
        }
    }

    pstFrame->stBufAddr[0].u32Stride_Y = TempOrderInfo.u32DstStride;
    pstFrame->stBufAddr[0].u32Stride_C = TempOrderInfo.u32DstStride;
    pstFrame->stLBufAddr[0].u32YStride = TempOrderInfo.u32DstStride;
    pstFrame->stLBufAddr[0].u32CStride = TempOrderInfo.u32DstStride;

    pstFrame->u32Height = g_stVideoFrame[index].u32HeightIn;
    pstFrame->u32Width= g_stVideoFrame[index].u32WidthIn;

    //printk("WinConvertTileFrmToLinear over Y 0x%x ,C 0x%x\n", TempOrderInfo.u32LumaAddrWrite, TempOrderInfo.u32ChromaAddrWrite);

    return MT_SUCCESS;
}

mt_s32 WinQueueFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    WINDOW_S *pstWin;
    MT_DRV_WIN_SRC_INFO_S *pstSource;
    //FIXME: stack size might overflow!
    //MT_DF_VIDEO_FRAME_S stVideoFrame;
    MT_BOOL bNeedNewData = MT_TRUE;
    //FIXME: stack size might overflow!
    //IMAGE stDpbInfo;
    mt_s32 i = 0;
    MT_DF_STATUS eDisplayStatus;
    mt_s32 index = 0;
    MT_DRV_DISPLAY_E enDisp;

    mt_s32 nRet = MT_SUCCESS;
    MT_BOOL bVirtual;
    disp_priv_t *p_dp = get_disp_priv_handle();
    fcnt_vir++;

    WinGetIndex(hWin, &enDisp, &index);
    WinCheckNullPointer(pFrameInfo);

    //printk("TTTT 0000 slot_idx = %d, %s line %d\n",pFrameInfo->slotInfo.filedInfoTop.slot_idx,__FUNCTION__,__LINE__);//yihua and binxuan
    //printk("[%s]line %d,frame cnt %d,slot %d,total %d,pts %lld,eos %d\n",__FUNCTION__,__LINE__,pFrameInfo->slotInfo.frm_cnt ,pFrameInfo->slotInfo.filedInfoTop.slot_idx,fcnt_vir,pFrameInfo->slotInfo.pts,pFrameInfo->slotInfo.end_of_stream_flag);
    if(pFrameInfo->slotInfo.frm_cnt == 500)
    {
        //WIN_ClearAllFrame(NULL);
    }

    WinCheckDeviceOpen();

    // s2 get state
    nRet = WinCheckFrame(pFrameInfo);
    if (nRet)
    {
        WIN_ERROR("win frame parameters invalid\n");
        return MT_ERR_VO_FRAME_INFO_ERROR;
    }

    //Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
    VTRACE("[%s](%lu): frm_cnt %u, slot %u, pts %llu, eos %u\n",__FUNCTION__,gettid(),
            pFrameInfo->slotInfo.frm_cnt,
            pFrameInfo->slotInfo.filedInfoTop.slot_idx,
            pFrameInfo->slotInfo.pts,
            pFrameInfo->slotInfo.end_of_stream_flag);
#endif

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        pstSource = &pstWin->stCfg.stSource;

        if ((pstWin->enType == MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE || pstWin->enType == MT_DRV_WIN_ACTIVE_SINGLE) &&
                (pFrameInfo->u32FrameIndex != pstWin->u32LastInLowDelayIdx))
        {
            mt_ld_event_s evt;
            mt_u32 TmpTime = 0;
            //MT_DRV_SYS_GetTimeStampMs(&TmpTime);
            evt.evt_id = EVENT_VO_FRM_IN;
            evt.frame = pFrameInfo->u32FrameIndex;
            evt.handle = pFrameInfo->hTunnelSrc;
            evt.time = TmpTime;
            mt_drv_ld_notify_event(&evt);
            pstWin->u32LastInLowDelayIdx = pFrameInfo->u32FrameIndex;
        }

        if (pstWin->bEnable == MT_TRUE)
        {
            //connie put new frame tap

#if 0
            mt_u32 TmpTime_cur;
            static mt_u32 last_tmp_time_1;
            static mt_u32 last_tmp_time_2;
            static WINDOW_S *pstWin_1 = 0;
            static WINDOW_S *pstWin_2 = 0;

            if(pstWin_1 == 0 && pstWin_2 == 0)
            {
                pstWin_1 = pstWin;
            }
            else if(pstWin_2 == 0 && pstWin_1 != pstWin)
            {
                pstWin_2 = pstWin;
            }


            if(pstWin_1 == pstWin)
            {
                MT_DRV_SYS_GetTimeStampMs(&TmpTime_cur);

                printk("win1_put_tap [%d]ms \n",TmpTime_cur - last_tmp_time_1);
                last_tmp_time_1 = TmpTime_cur;
            }
            else if(pstWin_2 == pstWin)
            {
                MT_DRV_SYS_GetTimeStampMs(&TmpTime_cur);

                printk("win2_put_tap [%d]ms \n",TmpTime_cur - last_tmp_time_2);
                last_tmp_time_2 = TmpTime_cur;
            }
#endif
            eDisplayStatus = DF_GetStatus(pstWin->stBuffer.stDispBP);
            if(eDisplayStatus == DF_STATUS_STOP)
            {
                //MT_INFO_VO("rls frame cnt %d,is3d %d\n",pFrameInfo->slotInfo.frm_cnt ,pFrameInfo->slotInfo.is_3D_flag);
                //       pstWin->stBuffer.stDispBP.stSrcInfo.pfRlsFrame(pstWin->stBuffer.stDispBP.stSrcInfo.hSrc, pFrameInfo);//our rls func may use slotindex
                return MT_ERR_VO_DEPEND_DEVICE_NOT_READY;
            }
#define WIN_WHILE_JUMPOUT 10
            if (bNeedNewData == MT_TRUE)
            {
                //unused
                //memset(&stDpbInfo, 0, sizeof(IMAGE));

                //transform to display video frame info
                if (mmz_phy_addr)
                {
                    disp_get_vid_input_info(p_dp, (MT_DIS_FRAME_SLOT_INFO_T *)&(g_testImagInfo.slotInfo));
#ifndef SYMPHONY_LINUX
                    DF_TransformDec2Disp_sinfo1(&g_testImagInfo, &g_stVideoFrame[index]);
#endif
                }
                else
                {
                    disp_get_vid_input_info(p_dp, (MT_DIS_FRAME_SLOT_INFO_T *)&(pFrameInfo->slotInfo));
                    DF_TransformDec2Disp_sinfo(pstWin->stBuffer.stDispBP, pFrameInfo, &g_stVideoFrame[index]);
                }
                //repeat first field and repeat frame should be guaranteed not to be enabled at the same time

                //
                bNeedNewData = MT_FALSE;
            }

            //the status could change
            g_pstWin[index] = pstWin;

            //FIX: Bug 106447
            //while (i < WIN_WHILE_JUMPOUT)
            {
                i++;

                //FIX: Bug 106447
                //if (bNeedNewData == MT_TRUE)
                //    break;

                //enqueue
                //need add tsk sleep logic
                //in each time the queue is added, success or fail, we should give the win tsk the oppotunity to change the status
                nRet = DF_TSK_enQueue(pstWin->stBuffer.stDispBP, &g_stVideoFrame[index], (MT_BOOL *)&bNeedNewData); //
                //	printk("%s %d pstWin->stBuffer.stDispBP %lx index %x \n", __func__, __LINE__, (ulong)pstWin->stBuffer.stDispBP, index);
                //nRet = WinBuf_PutNewFrame(&pstWin->stBuffer.stWinBP, pFrameInfo);
                if (nRet)
                {
                    return MT_ERR_VO_BUFQUE_FULL;
                }

                disp_st_vid_set_vdec_size(pFrameInfo->u32Height, pFrameInfo->u32Width);
            }
            //fixbug:100723,avoid memoryleak
            if(g_stVideoFrame[index].u32CurEnqueueTimes == 0)
            {
                return MT_ERR_VO_BUFQUE_FULL;
            }
        }
        else
        {
            WIN_WARN("Window is disabled\n");
            return MT_ERR_VO_INVALID_OPT;
        }
    }
    else
    {
        VIRTUAL_S *pstVirWin;

        WinCheckVirWindow(hWin, pstVirWin);
#ifdef CONFIG_EMU
        MT_INFO_VO("minnan kennel vitual frameno %d\n", pFrameInfo->u32FrameIndex);
#endif
        // update sink acquire frame count
        pstVirWin->stFrameStat.u32SrcQTry++;
        //WinConvertTileFrmToLinear(pFrameInfo);
        nRet = WIN_VIR_AddNewFrm(pstVirWin, pFrameInfo);
        if (nRet)
        {
            return MT_ERR_VO_BUFQUE_FULL;
        }

        pstVirWin->stFrameStat.u32SrcQOK++;
    }

    return MT_SUCCESS;
}

mt_s32 WIN_QueueFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    WinCheckNullPointer(pFrameInfo);
    // WIN_DEBUGK("LLLL 7777 %x ,frm cnt %d\n", pFrameInfo->stBufAddr[0].u32PhyAddr_Y, pFrameInfo->slotInfo.frm_cnt);
    DF_Init_TestIMG();
    pFrameInfo->bStillFrame = MT_FALSE;
    return WinQueueFrame(hWin, pFrameInfo);
}

mt_s32 WIN_QueueUselessFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    WINDOW_S *pstWin;
    mt_s32 nRet = MT_SUCCESS;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();

    WinCheckNullPointer(pFrameInfo);

    bVirtual = WinCheckVirtual(hWin);

    if (!bVirtual)
    {
        WIN_DEBUGK("%s %d \n", __FUNCTION__, __LINE__);
        WinCheckWindow(hWin, pstWin);
        nRet = WinCheckFrame(pFrameInfo);
        if (nRet)
        {
            WIN_ERROR("win frame parameters invalid\n");
            return MT_ERR_VO_FRAME_INFO_ERROR;
        }

        if (pstWin->bEnable == MT_TRUE)
        {
            nRet = WinBufferPutULSFrame(&pstWin->stBuffer, pFrameInfo);
            if (nRet)
            {
                WIN_WARN("quls failed\n");
                return MT_ERR_VO_BUFQUE_FULL;
            }
        }
        else
        {
            WIN_ERROR("Window is disabled\n");
            return MT_ERR_VO_INVALID_OPT;
        }
    }
    else
    {
        VIRTUAL_S *pstVirWindow;
        WinCheckVirWindow(hWin, pstVirWindow);

        nRet = WinCheckFrame(pFrameInfo);
        if (nRet)
        {
            WIN_ERROR("win frame parameters invalid\n");
            return MT_ERR_VO_FRAME_INFO_ERROR;
        }

        nRet = WIN_VIR_AddUlsFrm(pstVirWindow, pFrameInfo);
        if (nRet)
        {
            WIN_WARN("quls failed\n");
            return MT_ERR_VO_BUFQUE_FULL;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WIN_DequeueFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    return MT_SUCCESS;
}

mt_s32 WIN_SetZorder(mt_handle hWin, MT_DRV_DISP_ZORDER_E enZFlag)
{
    WINDOW_S *pstWin;
    mt_s32 nRet = MT_SUCCESS;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();

    if (enZFlag >= MT_DRV_DISP_ZORDER_BUTT)
    {
        WIN_FATAL("MT_DRV_DISP_ZORDER_E invalid!\n");
        return MT_ERR_VO_INVALID_PARA;
    }

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        // s1 ?
        WinCheckWindow(hWin, pstWin);

        switch (enZFlag)
        {
        case MT_DRV_DISP_ZORDER_MOVETOP:
            nRet = WinZorderMoveTop(pstWin);
            break;
        case MT_DRV_DISP_ZORDER_MOVEUP:
            nRet = WinZorderMoveUp(pstWin);
            break;
        case MT_DRV_DISP_ZORDER_MOVEBOTTOM:
            nRet = WinZorderMoveBottom(pstWin);
            break;
        case MT_DRV_DISP_ZORDER_MOVEDOWN:
            nRet = WinZorderMoveDown(pstWin);
            break;
        default:
            nRet = MT_ERR_VO_INVALID_OPT;
            break;
        }

        if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin))
        {
            if (pstWin->hSlvWin)
            {
                WIN_SetZorder(pstWin->hSlvWin, enZFlag);
            }
        }
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return nRet;
}

mt_s32 WIN_GetZorder(mt_handle hWin, mt_u32 *pu32Zorder)
{
    WINDOW_S *pstWin;
    mt_s32 nRet = MT_SUCCESS;
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    WinCheckNullPointer(pu32Zorder);

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        *pu32Zorder = pstWin->u32Zorder;
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return nRet;
}

mt_s32 WIN_Freeze(mt_handle hWin, MT_BOOL bEnable, MT_DRV_WIN_SWITCH_E enFrz)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;
    mt_u32 u = 0;

    WinCheckDeviceOpen();

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        // s1 ?
        WinCheckWindow(hWin, pstWin);
        //WinCheckSlaveWindow(pstWin);

        if (enFrz >= MT_DRV_WIN_SWITCH_BUTT)
        {
            WIN_ERROR("Freeze mode is invalid!\n");
            return MT_ERR_VO_INVALID_PARA;
        }

        // s2 set enable
        if (pstWin->bUpState && pstWin->bEnable)
        {
            WIN_ERROR("Window is changing, can't set pause now!\n");
            return MT_ERR_VO_INVALID_OPT;
        }

        if (!pstWin->bEnable || pstWin->bReset)
        {
            WIN_ERROR("Window is DISABLE, can't set pause now!\n");
            return MT_ERR_VO_INVALID_OPT;
        }

        pstWin->bUpState = MT_FALSE;

        pstWin->enStateNew = bEnable ? WIN_STATE_FREEZE : WIN_STATE_UNFREEZE;
        if (bEnable)
            pstWin->stFrz.enFreezeMode = enFrz;
        else
            pstWin->stFrz.enFreezeMode = MT_DRV_WIN_SWITCH_BUTT;

        WIN_DEBUGK("old status %d,enState %d,freeze %d\n",DF_GetStatus(pstWin->stBuffer.stDispBP), pstWin->enStateNew, pstWin->stFrz.enFreezeMode);

        // use DF_SetFreezeBuffer instead of DF_FreezeMalloc
#if 0
        if(pstWin->stFrz.enFreezeMode == MT_DRV_WIN_SWITCH_LAST)
        {
            DF_FreezeMalloc(pstWin->stBuffer.stDispBP);
        }
#endif

        pstWin->bUpState = MT_TRUE;

        if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin))
        {
            if (pstWin->hSlvWin)
            {
                WIN_Freeze(pstWin->hSlvWin, bEnable, enFrz);
            }
        }

        u = 0;
        while (pstWin->bUpState && (u < 10))
        {
            DISP_MSLEEP(5);
            u++;
        }

        if (u >= 10)
        {
            DISP_WARN("############ freeze TIMEOUT#########\n");
        }

        //FIX: Bug 120990,
        // but if User control Screen ON/OFF, it might has bug show vid layer here.
#if 0	
        if (!bEnable)
        {
            (void)disp_set_vid_layer_show(MT_TRUE);
        }
#endif
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_GetFreezeStatus(mt_handle hWin, MT_BOOL *pbEnable, MT_DRV_WIN_SWITCH_E *penFrz)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        *pbEnable = (pstWin->enStateNew == WIN_STATE_FREEZE) ? MT_TRUE : MT_FALSE;
        *penFrz = pstWin->stFrz.enFreezeMode;
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return MT_SUCCESS;
}

static mt_u32 WIN_Delay(WINDOW_S *pstWin)
{
    mt_u32 u = 0;

    if (pstWin)
    {
        while ((pstWin->bReset) && (u < 400))
        {
            usleep_range(100, 1000);
            u++;
        }
    }

    return u;
}

mt_s32 WIN_ResetSeperate(mt_handle hWin, MT_DRV_WIN_SWITCH_E enRst)
{
    WINDOW_S *pstWin = MT_NULL;
    MT_DRV_VIDEO_FRAME_S *pstFrame;
    MT_BOOL bVirtual;

    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        if (pstWin->bEnable)
        {
            pstWin->stRst.enResetMode = enRst;
            // use DF_SetFreezeBuffer instead of DF_FreezeMalloc
#if 0
            //hmc, just run before freeze, must finish malloc here
            if(enRst==0)
                //not open freeze on 128M chip
                DF_FreezeMalloc(pstWin->stBuffer.stDispBP);
#endif
            pstWin->bReset = MT_TRUE;
        }
        else
        {
            MT_WARN_VO(" win is not Enabled\n");
            WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);
            ISR_WinReleaseUSLFrame(pstWin);
            WinBuf_DiscardDisplayedFrame(&pstWin->stBuffer.stWinBP);
            /* flush frame in full buffer pool */
            pstFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
            DISP_DEBUGK("[%s]line %d\n", __FUNCTION__, __LINE__);

            WinBuf_FlushWaitingFrame(&pstWin->stBuffer.stWinBP, pstFrame);
            pstWin->bReset = MT_FALSE;
        }
    }
    else
    {
        VIRTUAL_S *pstVirWindow;
        mt_s32 s32Ret;

        WinCheckVirWindow(hWin, pstVirWindow);

        s32Ret = WIN_VIR_Reset(pstVirWindow);
        if (MT_SUCCESS != s32Ret)
        {
            DISP_ERROR("Reset Virtual Window Failed\n");
            return s32Ret;
        }
    }
    return MT_SUCCESS;
}

mt_s32 WIN_Reset(mt_handle hWin, MT_DRV_WIN_SWITCH_E enRst)
{
    WINDOW_S *pstWin = MT_NULL, *pstWinSlv = MT_NULL;
    mt_u32 u = 0;
    MT_BOOL bVirtual;
    mt_s32 ret = 0;
    mt_s32 counter = 0;
    //hmc
    MT_DF_STATUS cur_status;
    MT_DRV_WIN_SWITCH_E en_swith = enRst;
    //MT_ERR_VO("[%s]line %d\n", __FUNCTION__, __LINE__);
#ifdef CONFIG_MT_DISP_FREEZE_DISABLE
    en_swith = MT_DRV_WIN_SWITCH_BLACK;
#endif

    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        if (pstWin->bEnable == MT_FALSE) {
			printk(KERN_ERR "pstWin->bEnable == MT_FALSE  set MT_DRV_WIN_SWITCH_BLACK  en_swith %d return !\n", en_swith);
			en_swith = MT_DRV_WIN_SWITCH_BLACK;
			return MT_SUCCESS;
        }		

        if (enRst >= MT_DRV_WIN_SWITCH_BUTT)
        {
            WIN_ERROR("Reset mode is invalid!\n");
            return MT_ERR_VO_INVALID_PARA;
        }

        if(MT_DRV_WIN_SWITCH_LAST == en_swith)
        {
            if(0 == freeze_buf_addr)
            {
                DISP_MMZ_BUF_S m_freeze_buf;
                mt_s32  nRet = DISP_OS_MMZ_AllocAndMap((const MT_S8 *)"DISP_FreezeCurLeftMemstatic", MMZ_ZONE_AV, MAX_FREEZE_BUFFER_SIZE, 32, &m_freeze_buf);	

                if(0 == nRet)
                {
                    g_freeze_buf_addr = m_freeze_buf.u32StartPhyAddr;
                    freeze_buf_addr = (unsigned int)m_freeze_buf.u32StartPhyAddr;
                }
                else
                {
                    WIN_ERROR("alloc FreezeCurLeft static failed %s\n",__FUNCTION__);
                }
            }
            else
            {
                g_freeze_buf_addr = (phys_addr_t)freeze_buf_addr;
            }
            DF_SetFreezeBuffer(pstWin->stBuffer.stDispBP, g_freeze_buf_addr, MAX_FREEZE_BUFFER_SIZE);
            printk(KERN_ERR "DF_SetFreezeBuffer addr %x len %x \n", freeze_buf_addr, MAX_FREEZE_BUFFER_SIZE);
        }

        if (pstWin->bReset || pstWin->bUpState)
        {
            MT_ERR_VO("Last reset is not finished!\n");
            return MT_ERR_VO_INVALID_OPT;
        }

        if ((ret = WIN_ResetSeperate(hWin, enRst)))
            return ret;

        /*now we do main and slave window reset in drv,
          not in mpi/unf level.*/
        if (pstWin->hSlvWin)
        {
            WinCheckWindow(pstWin->hSlvWin, pstWinSlv);
            if ((ret = WIN_ResetSeperate(pstWin->hSlvWin, enRst)))
                return ret;
        }

        u += WIN_Delay(pstWin);
        u += WIN_Delay(pstWinSlv);

        if (u >= 80)
        {
            DISP_WARN("############ RESET TIMEOUT#########\n");
        }
    }
    else
    {
        return WIN_ResetSeperate(hWin, enRst);
    }
    //hmc, wait for dma done
    if(enRst==MT_DRV_WIN_SWITCH_LAST)
    {
        while(1)
        {
            cur_status=DF_GetStatus(pstWin->stBuffer.stDispBP);
            if(DF_TestFreezeDone(pstWin->stBuffer.stDispBP)==1)
                break;
            else
            {
                counter ++;
                if ((counter % 5000) == 0)
                {
#ifdef DEBUG_DISPLAY_FREEZE
                    MT_INFO_DISP("\nWIN_Reset: wait DF_TestFreezeDone %d times. cur_status=%d\n",counter,cur_status);
#endif
                }
                DISP_MSLEEP(1);
            }
        }
    }


    return MT_SUCCESS;
}

mt_s32 WIN_Pause(mt_handle hWin, MT_BOOL bEnable)
{
    WINDOW_S *pstWin;
    mt_u32 u;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);

    if (!bVirtual)
    {
        // s1 ?
        WinCheckWindow(hWin, pstWin);
        //WinCheckSlaveWindow(pstWin);

        // s2 set enable
        if (pstWin->bUpState && pstWin->bEnable)
        {
            WIN_ERROR("Window is changing, can't set pause now!\n");
            return MT_ERR_VO_INVALID_OPT;
        }

        pstWin->bUpState = MT_FALSE;

        pstWin->enStateNew = bEnable ? WIN_STATE_PAUSE : WIN_STATE_RESUME;

        pstWin->bUpState = MT_TRUE;

        u = 0;
        while (pstWin->bUpState && (u < 10))
        {
            DISP_MSLEEP(5);
            u++;
        }

        if (u >= 10)
        {
            DISP_WARN("############ PAUSE TIMEOUT#########\n");
        }
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_SetStepMode(mt_handle hWin, MT_BOOL bStepMode)
{
    WINDOW_S *pstWin;
    MT_BOOL bVirtual;

    WinCheckDeviceOpen();

    bVirtual = WinCheckVirtual(hWin);

    if (!bVirtual)
    {
        // s1 ?
        WinCheckWindow(hWin, pstWin);

        // s2 set enable
        if (pstWin->bUpState && pstWin->bEnable)
        {
            WIN_ERROR("Window is changing, can't set pause now!\n");
            return MT_ERR_VO_INVALID_OPT;
        }

        // set stepmode flag
        pstWin->bStepMode = bStepMode;

        if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin))
        {
            if (pstWin->hSlvWin)
            {
                WIN_SetStepMode(pstWin->hSlvWin, bStepMode);
            }
        }
    }
    else
    {
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_SetStepPlay(mt_handle hWin)
{
    return MT_SUCCESS;
}

mt_s32 WIN_SetQuick(mt_handle hWin, MT_BOOL bEnable)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();

    // s1 ?
    WinCheckWindow(hWin, pstWin);
    //WinCheckSlaveWindow(pstWin);

    // s2 set enable
    if (pstWin->bUpState && pstWin->bEnable)
    {
        WIN_ERROR("Window is changing, can't set pause now!\n");
        return MT_ERR_VO_INVALID_OPT;
    }

    // initial quickmode
    pstWin->bQuickMode = bEnable;

    if (MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE == WinGetType(pstWin))
    {
        if (pstWin->hSlvWin)
        {
            WIN_SetQuick(pstWin->hSlvWin, bEnable);
        }
    }

    return MT_SUCCESS;
}

mt_s32 WIN_GetQuick(mt_handle hWin, MT_BOOL *pbEnable)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();
    WinCheckWindow(hWin, pstWin);

    // initial quickmode
    *pbEnable = pstWin->bQuickMode;
    return MT_SUCCESS;
}

/* only for virtual window */
mt_s32 WIN_SetExtBuffer(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S *pstBuf)
{

    return MT_SUCCESS;
}

mt_s32 WIN_AttachSink(mt_handle hWin, mt_handle hSink)
{
    VIRTUAL_S *pstVirWin;
    mt_s32 s32Ret;

    WinCheckVirWindow(hWin, pstVirWin);

    s32Ret = WIN_VIR_AttachSink(pstVirWin, hSink);

    return s32Ret;
}

mt_s32 WIN_DetachSink(mt_handle hWin, mt_handle hSink)
{
    VIRTUAL_S *pstVirWin;
    mt_s32 s32Ret;

    WinCheckVirWindow(hWin, pstVirWin);

    s32Ret = WIN_VIR_DetachSink(pstVirWin, hSink);

    return s32Ret;
}

mt_s32 WIN_SetVirtualAttr(mt_handle hWin, mt_u32 u32Width, mt_u32 u32Height)
{
    VIRTUAL_S *pstVirWin;
    mt_s32 s32Ret;

    WinCheckVirWindow(hWin, pstVirWin);

    s32Ret = WIN_VIR_SetSize(pstVirWin, u32Width, u32Height);

    return s32Ret;
}

mt_s32 WIN_AcquireFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    VIRTUAL_S *pstVirWin;
    mt_s32 s32Ret;

    WinCheckVirWindow(hWin, pstVirWin);

    s32Ret = WIN_VIR_GetFrm(pstVirWin, &stDrvVideo);

    // update sink acquire frame count
    pstVirWin->stFrameStat.u32SinkAcqTry++;
    if (s32Ret == MT_SUCCESS)
    {
        memcpy(&(stDrvVideo.stLBufAddr[0]),&(pFrameinfo->stLBufAddr[0]),2*sizeof(MT_DRV_LINEAR_FRAME_ADDR_S));
        //win_DMA_Test_init(&stDrvVideo);
        //win_DMA_Test();
        WinConvertTileFrmToLinear(hWin, &stDrvVideo);
        rcnt_vir++;
        memcpy(pFrameinfo,&stDrvVideo,sizeof(MT_DRV_VIDEO_FRAME_S));
        pstVirWin->stFrameStat.u32SinkAcqOK++;
        //printk("%s line %d,fcnt_vir %d,rcnt_vir %d,frm %d,slot %d,w %d,h %d\n", __FUNCTION__, __LINE__,fcnt_vir,rcnt_vir,pFrameinfo->slotInfo.frm_cnt ,pFrameinfo->slotInfo.filedInfoTop.slot_idx,pFrameinfo->u32Width,pFrameinfo->u32Height);
    }

    return s32Ret;
}

mt_s32 WIN_ReleaseFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    VIRTUAL_S *pstVirWin;
    mt_s32 s32Ret;

    WinCheckVirWindow(hWin, pstVirWin);

    //printk("%s line %d,fcnt_vir %d,rcnt_vir %d,frm %d,slot %d,w %d,h %d\n", __FUNCTION__, __LINE__,fcnt_vir,rcnt_vir,pFrameinfo->slotInfo.frm_cnt ,pFrameinfo->slotInfo.filedInfoTop.slot_idx,pFrameinfo->u32Width,pFrameinfo->u32Height);
    s32Ret = WIN_VIR_RelFrm(pstVirWin, pFrameinfo);

    // update sink release frame count
    pstVirWin->stFrameStat.u32SinkRlsTry++;
    if (s32Ret == MT_SUCCESS)
    {
        pstVirWin->stFrameStat.u32SinkRlsOK++;
    }

    return s32Ret;
}

mt_s32 WIN_CreatStillFrame(MT_DRV_VIDEO_FRAME_S *pFrameinfo, MT_DRV_VIDEO_FRAME_S *pStillFrameInfo)
{
#if 0
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = MT_NULL;
    mt_u32 datalen = 0, y_stride = 0, height = 0 ;
    DISP_MMZ_BUF_S    stMMZ_StillFrame;
    DISP_MMZ_BUF_S    stMMZ_Frame;
    mt_s32            nRet = 0;

    pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(pFrameinfo->u32Priv);

    memset(&stMMZ_StillFrame,0,sizeof(DISP_MMZ_BUF_S));
    memset(&stMMZ_Frame,0,sizeof(DISP_MMZ_BUF_S));

    /*1:calculate alloc mem*/
    y_stride = pFrameinfo->stBufAddr[0].u32Stride_Y;
    height     = (MT_TRUE == pstPrivInfo->stCompressInfo.u32CompressFlag)
        ? pstPrivInfo->stCompressInfo.s32CompFrameHeight : pFrameinfo->u32Height;

    if ( MT_DRV_PIX_FMT_NV21 == pFrameinfo->ePixFormat)
        datalen = height * y_stride * 3 / 2 + height * 4;
    else
        datalen = height * y_stride * 2 + height * 4;

    if(MT_SUCCESS != DISP_OS_MMZ_Alloc("VDP_StillFrame", MT_NULL, datalen, 16, &stMMZ_StillFrame))
    {
        WIN_ERROR(" Alloc StillFrame  failid(%x)\n",datalen);
        return MT_ERR_VO_MALLOC_FAILED;
    }

    /*2: creat still frame*/
    /*not support  Compress info*/
    stMMZ_Frame.u32StartPhyAddr = pFrameinfo->stBufAddr[0].u32PhyAddr_Y;

    nRet = DISP_OS_MMZ_Map(&stMMZ_StillFrame);
    if (MT_SUCCESS != nRet)
        goto __FAILURE_RELEASE;

    nRet = DISP_OS_MMZ_Map(&stMMZ_Frame);
    if (MT_SUCCESS != nRet)
        goto __EXIT_UNMAP;


    memcpy(pStillFrameInfo,pFrameinfo,sizeof(MT_DRV_VIDEO_FRAME_S));
    memcpy((void *)stMMZ_StillFrame.u32StartVirAddr, (void *)stMMZ_Frame.u32StartVirAddr,datalen);


    /*3: calculate still frame addr*/
    pStillFrameInfo->stBufAddr[0].u32PhyAddr_YHead = stMMZ_StillFrame.u32StartPhyAddr +(pFrameinfo->stBufAddr[0].u32PhyAddr_YHead - stMMZ_Frame.u32StartPhyAddr );
    pStillFrameInfo->stBufAddr[0].u32Stride_Y =  pFrameinfo->stBufAddr[0].u32Stride_Y;

    pStillFrameInfo->stBufAddr[0].u32PhyAddr_Y = stMMZ_StillFrame.u32StartPhyAddr +(stMMZ_Frame.u32StartPhyAddr - pFrameinfo->stBufAddr[0].u32PhyAddr_Y );

    pStillFrameInfo->stBufAddr[0].u32PhyAddr_C = stMMZ_StillFrame.u32StartPhyAddr + (pStillFrameInfo->u32Height*pStillFrameInfo->stBufAddr[0].u32Stride_Y);

    pStillFrameInfo->stBufAddr[0].u32PhyAddr_CrHead = stMMZ_StillFrame.u32StartPhyAddr +( pFrameinfo->stBufAddr[0].u32PhyAddr_CrHead - stMMZ_Frame.u32StartPhyAddr);
    pStillFrameInfo->stBufAddr[0].u32PhyAddr_Cr = stMMZ_StillFrame.u32StartPhyAddr +( pFrameinfo->stBufAddr[0].u32PhyAddr_Cr - stMMZ_Frame.u32StartPhyAddr );
    pStillFrameInfo->stBufAddr[0].u32Stride_Cr = pFrameinfo->stBufAddr[0].u32Stride_Cr;

    pStillFrameInfo->bStillFrame = MT_TRUE;

    DISP_OS_MMZ_UnMap(&stMMZ_StillFrame);
    DISP_OS_MMZ_UnMap(&stMMZ_Frame);

    return MT_SUCCESS;

__EXIT_UNMAP:
    DISP_OS_MMZ_UnMap(&stMMZ_StillFrame);

__FAILURE_RELEASE:
    DISP_OS_MMZ_Release(&stMMZ_StillFrame);
    return nRet;
#endif
    return MT_SUCCESS;
}

mt_s32 WinReleaseStillFrame(MT_DRV_VIDEO_FRAME_S *pStillFrameInfo)
{
#if 0
    DISP_MMZ_BUF_S    stMMZ_StillFrame;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = MT_NULL;

    WinCheckNullPointer(pStillFrameInfo);
    if (pStillFrameInfo->bStillFrame)
    {
        memset((void*)&stMMZ_StillFrame, 0, sizeof(DISP_MMZ_BUF_S));
        pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(pStillFrameInfo->u32Priv);
        if (MT_TRUE == pstPrivInfo->stCompressInfo.u32CompressFlag)
        {
            stMMZ_StillFrame.u32StartPhyAddr = pStillFrameInfo->stBufAddr[0].u32PhyAddr_YHead;
        }
        else
        {
            stMMZ_StillFrame.u32StartPhyAddr = pStillFrameInfo->stBufAddr[0].u32PhyAddr_Y;
        }
        //printk("release 0x%x\n",stMMZ_StillFrame.u32StartPhyAddr);
        DISP_OS_MMZ_Release(&stMMZ_StillFrame);
        return MT_SUCCESS;
    }

    return MT_FAILURE;
#endif
    return MT_SUCCESS;
}

mt_s32 WIN_SendFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameInfo)
{
    mt_s32 nRet;
    MT_DRV_VIDEO_FRAME_S StillFrameInfo;
    WinCheckNullPointer(pFrameInfo);

    nRet = WIN_CreatStillFrame(pFrameInfo, &StillFrameInfo);
    if (MT_SUCCESS != nRet)
    {
        WIN_ERROR(" WIN_CreatStillFrame  failid(%x)\n", nRet);
        return nRet;
    }

    return WinQueueFrame(hWin, &StillFrameInfo);
}

mt_s32 Win_DebugGetHandle(MT_DRV_DISPLAY_E enDisp, WIN_HANDLE_ARRAY_S *pstWin)
{
    mt_s32 i;

    WinCheckDeviceOpen();

    // s1 ?
    WinCheckNullPointer(pstWin);

    DISP_MEMSET(pstWin, 0, sizeof(WIN_HANDLE_ARRAY_S));

    pstWin->u32WinNumber = 0;

    for (i = 0; i < WINDOW_MAX_NUMBER; i++)
    {
        if (stDispWindow.pstWinArray[(mt_u32)enDisp][i])
        {
            pstWin->ahWinHandle[pstWin->u32WinNumber] = (mt_handle)(stDispWindow.pstWinArray[(mt_u32)enDisp][i]->u32Index);
            pstWin->u32WinNumber++;
        }
    }

    return MT_SUCCESS;
}

/*
   mt_s32 WinUpdatePlayInfo(MT_DRV_WIN_PLAY_INFO_S *ptPlay, mt_u32 u32Rate)
   {
   ptPlay->u32DispRate = u32Rate;
   return MT_SUCCESS;
   }
   */

mt_s32 WinAcquireFrame(WINDOW_S *pstWin)
{
#if 0
    MT_DRV_VIDEO_FRAME_S stNewFrame;
    mt_u32 u32BufId;
    mt_s32 nRet = MT_SUCCESS;

    nRet = BQ_GetWriteNode(&pstWin->stBuffer.stBP, &u32BufId);
    if (nRet)
    {
        return MT_ERR_VO_BUFQUE_FULL;
    }

    if (pstWin->stSource.pfAcqFrame)
    {
        nRet = pstWin->stSource.pfAcqFrame(pstWin->stSource.hSrc, &stNewFrame);
        if (nRet)
        {
            WIN_ERROR("WIN Release Frame failid\n");
            return MT_ERR_VO_FRAME_RELEASE_FAILED;
        }
    }

    // s2 get state
    nRet = WinCheckFrame(&stNewFrame);
    if (nRet)
    {
        return MT_ERR_VO_FRAME_INFO_ERROR;
    }

    if (!stNewFrame.u32PlayTime)
    {
        if (pstWin->stSource.pfRlsFrame && stNewFrame.bToRelease)
        {
            nRet = pstWin->stSource.pfRlsFrame(pstWin->stSource.hSrc, &stNewFrame);
            if (nRet)
            {
                WIN_ERROR("WIN Release Frame failid\n");
            }

            BQ_ReleaseRecoder(&pstWin->stBufQue, BQ_RELEASE_DISCARD);
        }
        else
        {
            // todo
        }
    }

    nRet = BQ_PutWriteNode(&pstWin->stBufQue, &stNewFrame, BUF_FRAME_SRC_NORMAL);
#endif

    return MT_SUCCESS;
}

//#define WIN_DEBUG_PRINT_RELEASE 1
mt_s32 s_ResetPrint = 0;

mt_void ISR_WinReleaseUSLFrame(WINDOW_S *pstWin)
{
    MT_DRV_WIN_SRC_INFO_S *pstSource = &pstWin->stCfg.stSource;
    //FIXME: stack might overflow!
    //MT_DRV_VIDEO_FRAME_S stRlsFrm;
    mt_s32 nRet;
    mt_s32 index = 0; 
    MT_DRV_DISPLAY_E enDisp;
    
    WinGetIndex(pstWin->u32Index, &enDisp, &index);
    /* release useless frame */

    nRet = WinBufferGetULSFrame(&pstWin->stBuffer, &g_stRlsFrm[index]);
    while (!nRet)
    {
        if (g_stRlsFrm[index].bStillFrame)
        {
            WIN_DestroyStillFrame(&g_stRlsFrm[index]);           
        }
        else if (pstSource->pfRlsFrame)
        {
            pstSource->pfRlsFrame(pstSource->hSrc, &g_stRlsFrm[index]);
#ifdef WIN_DEBUG_PRINT_RELEASE
            if (s_ResetPrint)
            {
                MT_INFO_VO("Rel 006 fid=%d, addr=0x%x\n",
                        stRlsFrm.u32FrameIndex,
                        stRlsFrm.stBufAddr[0].u32PhyAddr_Y);
            }
#endif
        }

        nRet = WinBufferGetULSFrame(&pstWin->stBuffer, &g_stRlsFrm[index]);
    }

    return;
}

MT_DRV_VIDEO_FRAME_S *ISR_SlaveWinGetConfigFrame(WINDOW_S *pstWin)
{
    WINDOW_S *pstMstWin = (WINDOW_S *)(pstWin->pstMstWin);
    MT_DRV_VIDEO_FRAME_S *pstDstF, *pstRefF, *pstNew;

    // get master window crrent configed frame
    pstDstF = WinBuf_GetConfigedFrame(&pstMstWin->stBuffer.stWinBP);
    if (!pstDstF)
    {
        return MT_NULL;
    }

    // get slave window displayed frame
    pstRefF = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);

    //pstNew = WinBuf_GetFrameByMaxID(&pstWin->stBuffer.stWinBP, pstRefF, RefID, enDstField);
    pstNew = WinBuf_GetFrameByDstFrame(&pstWin->stBuffer.stWinBP, pstDstF, pstRefF);

#if 0
    if (pstNew)
    {
        pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstNew->u32Priv[0]);
        printk("S:d=%d,f=%d,F:id =%d,f=%d\n",  RefID, enDstField, pstNew->u32FrameIndex, pstPriv->eOriginField);
    }
#endif

    return pstNew;
}

mt_void WinUpdateDispInfo(WINDOW_S *pstWin, MT_DISP_DISPLAY_INFO_S *pstDsipInfo)
{
    pstWin->stDispInfo.u32RefreshRate = pstDsipInfo->u32RefreshRate;
    pstWin->stDispInfo.bIsInterlace = pstDsipInfo->bInterlace;
    pstWin->stDispInfo.bIsBtm = pstDsipInfo->bIsBottomField;
    pstWin->stDispInfo.stWinCurrentFmt = pstDsipInfo->stFmtResolution;

    return;
}

mt_void WinUpdateDispInfo_InitialFmt(WINDOW_S *pstWin, MT_DISP_DISPLAY_INFO_S *pstDsipInfo)
{
    pstWin->stDispInfo.stWinInitialFmt = pstDsipInfo->stFmtResolution;
    return;
}

WIN_DISP_INFO_S *WinGetDispInfoByHandle(mt_handle hWin)
{
    WINDOW_S *pstWin;

    pstWin = WinGetWindow(hWin);

    if (pstWin)
    {
        return &pstWin->stDispInfo;
    }

    return MT_NULL;
}

mt_void WinTestFrameMatch(WINDOW_S *pstWin, MT_DRV_VIDEO_FRAME_S *pstFrame, MT_DISP_DISPLAY_INFO_S *pstDsipInfo)
{
    //MT_DRV_WIN_ATTR_S *pstAttr = &pstWin->stUsingAttr;
    MT_DRV_VIDEO_PRIVATE_S *pstPriv = (MT_DRV_VIDEO_PRIVATE_S *)&(pstFrame->u32Priv[0]);

    // if display work at interlace mode and frame rate equale to display refresh rate,
    // dectect whether top-field video frame output at top-field time
    if (pstDsipInfo->bInterlace == MT_TRUE)
    {
        if (((pstPriv->eOriginField == MT_DRV_FIELD_TOP) && (pstDsipInfo->bIsBottomField == MT_FALSE)) ||
                ((pstPriv->eOriginField == MT_DRV_FIELD_BOTTOM) && (pstDsipInfo->bIsBottomField == MT_TRUE)))
        {
            pstWin->u32TBNotMatchCount++;
#if 0
            if (pstWin->enDisp == MT_DRV_DISPLAY_0)
            {
                printk(">>>>>>>Disp=%d, fid=%d, f=%d, btm=%d\n",
                        pstWin->enDisp,
                        pstFrame->u32FrameIndex,
                        pstPriv->eOriginField,
                        pstDsipInfo->bIsBottomField);
            }
#endif
        }
#if 0
        else
        {
            if (pstWin->enDisp == MT_DRV_DISPLAY_0)
            {
                printk("Disp=%d, fid=%d, f=%d, btm=%d\n",
                        pstWin->enDisp,
                        pstFrame->u32FrameIndex,
                        pstPriv->eOriginField,
                        pstDsipInfo->bIsBottomField);
            }
        }
#endif
    }

    return;
}

#define FIDELITY_033 1
#define FIDELITY_18 2
#define FIDELITY_576I_YPBPR 3
#define FIDELITY_576P_YPBPR 3
#define FIDELITY_480I_YPBPR 4
#define FIDELITY_480P_YPBPR 4
#define FIDELITY_720P_YPBPR 5
#define FIDELITY_1080I_P_YPBPR 7
#define FIDELITY_CBAR_75 8
#define FIDELITY_MX625 10
#define FIDELITY_BOWTIE 18
#define FIDELITY_SKN 20
#define FIDELITY_ZDN 22
#define FIDELITY_MATRIX525 23
#define FIDELITY_MOTO_CVBS 25
#define FIDELITY_MOTO_75_COLORBARS 32
#define FIDELITY_MOTO_COMBINATION 33
#define FIDELITY_MOTO_CVBS_MATRIX 36

#define VIDEO_TEST_SATURATION_OFFSET (3)
#define VIDEO_TEST_SATURATION_OFFSET_DEC (2)

mt_void ISR_WinConfigFrameMute(WINDOW_S *pstWin, MT_DRV_VIDEO_FRAME_S *pstFrame, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    int index = 0;
    MT_DRV_DISPLAY_E enDisp;
    DISP_LAYERSHOW_S arg = {0};
    
    if (!pstWin)
    {
        WIN_DEBUGK("WIN Input null pointer in %s!\n", __FUNCTION__);
        return;
    }

    WinGetIndex(pstWin->u32Index, &enDisp, &index);
    WIN_DEBUGK("%s %d subvideo:%d\n",__FUNCTION__,__LINE__, g_stDrvSetting[index].b_use_subvideo_layer);
    
    arg.b_on = 0;
    arg.enDisp = MT_DRV_DISPLAY_1;

    if(g_stDrvSetting[index].b_use_subvideo_layer)
    {
        arg.elayer = DISP_LAYER_ID_STILL_HD;            
    }            
    else
    {
        arg.elayer = DISP_LAYER_ID_VIDEO_HD;  
    }
    
    disp_set_layershow(MT_DRV_DISPLAY_1, &arg);

    return;
}

mt_void Win_GenerateConfigInfo(WINDOW_S *pstWin,
        MT_DRV_VIDEO_FRAME_S *pstFrame,
        const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo,
        WIN_HAL_PARA_S *pstLayerPara)
{

}

extern mt_u32 s_u32VdpBaseAddr;
mt_void Win_PqProcess(WINDOW_S *pstWin, MT_DRV_VIDEO_FRAME_S *pstFrame, WIN_HAL_PARA_S *pstLayerPara)
{
#if 0
    MT_VDP_PQ_INFO_S stTimingInfo;
    mt_rect_s stVpssGive;
    MT_DRV_WIN_ATTR_S *pstAttr = &pstWin->stUsingAttr;
    MT_BOOL bSrChange = MT_FALSE;
    VIDEO_LAYER_CAPABILITY_S  stVideoLayerCap;


    (mt_void)pstWin->stVLayerFunc.PF_GetCapability(pstWin->u32VideoLayer,&stVideoLayerCap);

    /*this is the current value.*/
    stTimingInfo.u32Width = ((MT_DRV_VIDEO_PRIVATE_S *)&pstFrame->u32Priv[0])->stVideoOriginalInfo.u32Width;
    stTimingInfo.u32Height = ((MT_DRV_VIDEO_PRIVATE_S *)&pstFrame->u32Priv[0])->stVideoOriginalInfo.u32Height;
    stTimingInfo.u32OutWidth = pstAttr->stOutRect.s32Width;
    stTimingInfo.u32OutHeight = pstAttr->stOutRect.s32Height;
    stTimingInfo.bSRState     = pstWin->stMiscInfor.bWinSrEnableCurrent;


    bSrChange  =  (stTimingInfo.u32Width != pstWin->stMiscInfor.stFrameOriginalRect.s32Width)
        || (stTimingInfo.u32Height != pstWin->stMiscInfor.stFrameOriginalRect.s32Height)
        || (stTimingInfo.u32OutWidth != pstWin->stMiscInfor.stWinOutRect.s32Width)
        || (stTimingInfo.u32OutHeight != pstWin->stMiscInfor.stWinOutRect.s32Height)
        || (stTimingInfo.bSRState != pstWin->stMiscInfor.bWinSrEnableLast);

    if ((( bSrChange || pstWin->bNeedResume)) && (stVideoLayerCap.bSR))
    {
        DRV_PQ_UpdateVdpPQ(1, &stTimingInfo, (S_VDP_REGS_TYPE *)s_u32VdpBaseAddr);
        pstWin->bNeedResume = MT_FALSE;
    }

    /*save the value ,for next judgement.*/
    pstWin->stMiscInfor.stFrameOriginalRect.s32Width  = stTimingInfo.u32Width;
    pstWin->stMiscInfor.stFrameOriginalRect.s32Height  = stTimingInfo.u32Height;
    pstWin->stMiscInfor.stWinOutRect.s32Width  =  stTimingInfo.u32OutWidth;
    pstWin->stMiscInfor.stWinOutRect.s32Height = stTimingInfo.u32OutHeight;
    pstWin->stMiscInfor.bWinSrEnableLast       =  stTimingInfo.bSRState;

    /*next ,we process coordinate  for dci.*/
    stVpssGive.s32X = 0;
    stVpssGive.s32Y = 0;
    stVpssGive.s32Width = pstFrame->u32Width;
    stVpssGive.s32Height = pstFrame->u32Height;

    (mt_void)Win_DciEnable_Policy(pstWin,
            &pstFrame->stLbxInfo,
            &pstLayerPara->stVideo,
            &stVpssGive,
            stDispWindow.u32WinNumber[pstWin->enDisp]);
#endif
}

mt_void ISR_WinConfigFrameNormal(WINDOW_S *pstWin, MT_DRV_VIDEO_FRAME_S *pstFrame, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    return;
}

mt_void ISR_WinConfigFrame(WINDOW_S *pstWin, MT_DRV_VIDEO_FRAME_S *pstFrame, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    if (pstFrame == MT_NULL)
        ISR_WinConfigFrameMute(pstWin, pstFrame, pstInfo);
    else
        ISR_WinConfigFrameNormal(pstWin, pstFrame, pstInfo);

    return;
}

mt_void ISR_WinUpdatePlayInfo(WINDOW_S *pstWin, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
#if 0
    // calc delay time in buffer queue
    mt_u32 T;

    DISP_ASSERT(pstInfo->stDispInfo.u32RefreshRate);
    DISP_ASSERT(pstInfo->stDispInfo.stFmtResolution.s32Height);

    pstWin->stDelayInfo.u32DispRate = pstInfo->stDispInfo.u32RefreshRate;

    /*in fact DISP_ASSERT has solves this ,this is for codecc warning.*/
    if (pstInfo->stDispInfo.u32RefreshRate)
    {
        pstWin->stDelayInfo.T = (1*1000*100)/pstInfo->stDispInfo.u32RefreshRate;
    }
    else
    {
        WIN_ERROR("Invalid display fresh rate:[%d]!\n", __LINE__);
    }

    pstWin->stDelayInfo.bInterlace = pstInfo->stDispInfo.bInterlace;
    T = pstWin->stDelayInfo.T;

    if (pstInfo->stDispInfo.bInterlace)
    {
        pstWin->stDelayInfo.u32DisplayTime = (pstInfo->stDispInfo.u32Vline *2*T)/pstInfo->stDispInfo.stPixelFmtResolution.s32Height;
    }
    else
    {
        pstWin->stDelayInfo.u32DisplayTime = (pstInfo->stDispInfo.u32Vline *T)/pstInfo->stDispInfo.stPixelFmtResolution.s32Height;
    }

    MT_DRV_SYS_GetTimeStampMs((mt_u32 *)&pstWin->stDelayInfo.u32CfgTime);
    return;
#endif
}

mt_void ISR_WinStateTransfer(WINDOW_S *pstWin)
{
    if (pstWin->enState == WIN_STATE_WORK)
    {
        switch (pstWin->enStateNew)
        {
        case WIN_STATE_PAUSE:
        case WIN_STATE_FREEZE:
            {
                pstWin->enState = pstWin->enStateNew;
                pstWin->bUpState = MT_FALSE;
                MT_INFO_VO("enState %d\n",pstWin->enState);
                return;
            }
        case WIN_STATE_WORK:
        case WIN_STATE_RESUME:
        case WIN_STATE_UNFREEZE:
        default:
            pstWin->bUpState = MT_FALSE;
            return;
        }
    }
    else if (pstWin->enState == WIN_STATE_PAUSE)
    {
        switch (pstWin->enStateNew)
        {
        case WIN_STATE_RESUME:
        case WIN_STATE_FREEZE:
            {
                pstWin->enState = pstWin->enStateNew;
                pstWin->bUpState = MT_FALSE;
                return;
            }
        case WIN_STATE_PAUSE:
        case WIN_STATE_WORK:
        case WIN_STATE_UNFREEZE:
        default:
            pstWin->bUpState = MT_FALSE;
            return;
        }
    }
    else if (pstWin->enState == WIN_STATE_FREEZE)
    {
        switch (pstWin->enStateNew)
        {
        case WIN_STATE_UNFREEZE:
            {
                pstWin->enState = pstWin->enStateNew;
                pstWin->bUpState = MT_FALSE;
                return;
            }
        case WIN_STATE_PAUSE:
        case WIN_STATE_FREEZE:
        case WIN_STATE_WORK:
        case WIN_STATE_RESUME:
        default:
            pstWin->bUpState = MT_FALSE;
            return;
        }
    }

    return;
}

mt_void ISR_WinResetState(WINDOW_S *pstWin)
{
    pstWin->enState = WIN_STATE_WORK;
    pstWin->bReset = MT_FALSE;

    return;
}

MT_DRV_VIDEO_FRAME_S *WinSearchMatchFrame(WINDOW_S *pstWin)
{
    WIN_DISP_INFO_S *pstDispInfo;
    MT_DRV_FIELD_MODE_E enDstField;
    MT_DRV_VIDEO_FRAME_S *pstDispFrame, *pstNewFrame;

    // if display work at interlace mode
    pstDispInfo = &pstWin->stDispInfo;
    if (!pstDispInfo)
    {
        WIN_ERROR("WIN  null pointer in %s!\n", __FUNCTION__);
        return MT_NULL;
    }

#if 0
    if (pstDispInfo->bIsInterlace != MT_TRUE)
    {
        if(pstWin->enType == MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE)
        {
            pstDispInfo = WinGetDispInfoByHandle(pstWin->hSlvWin);
        }
    }
#else

    if (pstDispInfo->bIsInterlace != MT_TRUE)
    {
        // if master window output progressive frame, get slave window info
        if (pstWin->enType == MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE)
        {
            WIN_DISP_INFO_S *pstSlvDispInfo;

            pstSlvDispInfo = WinGetDispInfoByHandle(pstWin->hSlvWin);
            if (pstSlvDispInfo)
            {
                if ((pstSlvDispInfo->bIsInterlace == MT_TRUE) && (pstDispInfo->u32RefreshRate == pstSlvDispInfo->u32RefreshRate))
                {
                    pstDispInfo = pstSlvDispInfo;
                }
            }
        }
    }
#endif

    //printk("bIsInterlace %d \n",pstDispInfo->bIsInterlace);
    if (!pstDispInfo || (pstDispInfo->bIsInterlace != MT_TRUE))
    {
        // if output progressive picture, need not to search top/bottom field
        return WinBuf_GetConfigFrame(&pstWin->stBuffer.stWinBP);
        ;
    }

    // if display work at interlace mode and frame rate equale to display refresh rate,
    // dectect whether top-field video frame output at top-field time
    pstDispFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
    enDstField = pstDispInfo->bIsBtm ? MT_DRV_FIELD_BOTTOM : MT_DRV_FIELD_TOP;

    pstNewFrame = WinBuf_GetFrameByDisplayInfo(&pstWin->stBuffer.stWinBP,
            pstDispFrame,
            pstDispInfo->u32RefreshRate,
            enDstField);
    return pstNewFrame;
}

MT_DRV_VIDEO_FRAME_S *WinGetFrameToConfig(WINDOW_S *pstWin, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    MT_DRV_VIDEO_FRAME_S *pstFrame;

    if (pstWin->enType == MT_DRV_WIN_ACTIVE_SLAVE)
    {

        pstFrame = ISR_SlaveWinGetConfigFrame(pstWin);
        if (pstFrame == MT_NULL && pstWin->bQuickMode)
        {
            MT_DRV_VIDEO_FRAME_S *pstDispFrame;

            pstDispFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);

            WinBuf_FlushWaitingFrame(&pstWin->stBuffer.stWinBP, pstDispFrame);
            MT_INFO_VO("slavewin quickmode get frame \n");
        }

    }
    else
    {
        if (pstWin->bQuickMode)
        {
            MT_DRV_VIDEO_FRAME_S *pstDispFrame;

            pstDispFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
            pstFrame = WinBuf_GetNewestFrame(&pstWin->stBuffer.stWinBP, pstDispFrame);
            MT_INFO_VO("mastewin quickmode get frame \n");
        }
        else
        {
            pstFrame = WinSearchMatchFrame(pstWin);
        }
    }

    if (!pstFrame)
    {
        /*if not  receive frame*/
        pstWin->stBuffer.u32UnderLoad++;

        if (WinTestBlackFrameFlag(pstWin) != MT_TRUE)
        {
            /*
1 :  if first config frame ,config nothing.
2 :  if last config is not black frame, repeat last config frame*/
            WinBuf_RepeatDisplayedFrame(&pstWin->stBuffer.stWinBP);
            pstFrame = WinBuf_GetConfigedFrame(&pstWin->stBuffer.stWinBP);
        }
        else
        {
            /*if last config is black frame, repeat black frame*/
            WinSetBlackFrameFlag(pstWin);
        }
    }
    else
    {
        /*if received frame*/
        WinClearBlackFrameFlag(pstWin);
    }

    /*
       if(!pstFrame)
       {
       pstFrame = BP_GetBlackFrameInfo();
       WinSetBlackFrameFlag(pstWin);
       }
       */

    return pstFrame;
}

mt_void ISR_CallbackForWinProcess(mt_handle hDst, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    WINDOW_S *pstWin;
    MT_BOOL bUpDispInof = MT_FALSE;
    int index = 0;
    MT_DRV_DISPLAY_E enDisp;
    
    //WIN_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);
    if (!hDst || !pstInfo)
    {
        WIN_ERROR("WIN Input null pointer in %s!\n", __FUNCTION__);
        return;
    }

    pstWin = (WINDOW_S *)hDst;

    WinGetIndex(pstWin->u32Index, &enDisp, &index);
    if (pstInfo->eEventType != MT_DRV_DISP_C_VT_INT)
    {
        DISP_PRINT("@@@@@@@ DISP MT_DRV_DISP_C_event= %d, disp=%d\n", pstInfo->eEventType, pstWin->enDisp);
    }

    if ((WIN_DEVICE_STATE_SUSPEND == s_s32WindowGlobalFlag) || (pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_CLOSE) ||
            (pstInfo->eEventType == MT_DRV_DISP_C_PREPARE_TO_PEND))
    {
        DISP_PRINT(">>>>>>>>> mask\n");
        pstWin->bMasked = MT_TRUE;
    }
    else
    {
        pstWin->bMasked = MT_FALSE;
    }

    if ((pstInfo->eEventType == MT_DRV_DISP_C_DISPLAY_SETTING_CHANGE) || (pstInfo->eEventType == MT_DRV_DISP_C_OPEN) ||
            (pstInfo->eEventType == MT_DRV_DISP_C_RESUME))
    {
        pstWin->bDispInfoChange = MT_TRUE;
        bUpDispInof = MT_TRUE;
    }

#if 0
    if (pstWin->u32VideoLayer == VDP_LAYER_VID_BUTT)
        printk("%s,layer:%d, new:%d,zorder:%d!\n", __func__,pstWin->u32VideoLayer,
                pstWin->u32VideoLayerNew, pstWin->u32Zorder);
#endif

    /*judge whether the window cfg is changed.
     * and reload the attr buffer to the buffer using.*/
    if (atomic_read(&pstWin->stCfg.bNewAttrFlag))
    {
        pstWin->stCfg.stAttr = pstWin->stCfg.stAttrBuf;
        pstWin->stUsingAttr = pstWin->stCfg.stAttrBuf;
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
        bUpDispInof = MT_TRUE;
        pstWin->bDispInfoChange = MT_TRUE;
    }

    if (bUpDispInof)
    {
        mt_rect_s stOutRectRevised, stOutRectOrigin;
        memset((void *)&stOutRectRevised, 0, sizeof(mt_rect_s));
        memset((void *)&stOutRectOrigin, 0, sizeof(mt_rect_s));

        stOutRectOrigin = pstWin->stCfg.stAttrBuf.stOutRect;

        if (!stOutRectOrigin.s32Width || !stOutRectOrigin.s32Height)
        {
            if (pstWin->bVirtScreenMode)
                stOutRectOrigin = pstInfo->stDispInfo.stVirtaulScreen;
            else
                stOutRectOrigin = pstInfo->stDispInfo.stFmtResolution;
        }

#if 0
        printk("0000VS=%d,%d, FR=%d,%d, PFR=%d,%d, O=%d,%d, U=%d,%d, offset: %d, %d,%d, %d\n",
                pstInfo->stDispInfo.stVirtaulScreen.s32Width,
                pstInfo->stDispInfo.stVirtaulScreen.s32Height,
                pstInfo->stDispInfo.stFmtResolution.s32Width,
                pstInfo->stDispInfo.stFmtResolution.s32Height,
                pstInfo->stDispInfo.stPixelFmtResolution.s32Width,
                pstInfo->stDispInfo.stPixelFmtResolution.s32Height,
                stOutRectOrigin.s32Width,
                stOutRectOrigin.s32Height,
                pstWin->stUsingAttr.stOutRect.s32Width,
                pstWin->stUsingAttr.stOutRect.s32Height,
                pstInfo->stDispInfo.stOffsetInfo.u32Left,
                pstInfo->stDispInfo.stOffsetInfo.u32Top,
                pstInfo->stDispInfo.stOffsetInfo.u32Right,
                pstInfo->stDispInfo.stOffsetInfo.u32Bottom);
#endif

        /*give a size conversion from:
         * 1) user setting(virtscreen)-----> virtual screen---->actual format;
         * 2) user setting(physical coordinate)--------> actual format
         * in fact, you can choose only one through the flag pstWin->bVirtScreenMode.
         */

        WinOutRectSizeConversionByType(&pstInfo->stDispInfo,
                &stOutRectOrigin,
                &stOutRectRevised,
                pstWin);

        pstWin->stUsingAttr.stOutRect = stOutRectRevised;
#if 0
        if (!pstWin->bVirtScreenMode)
        {
            pstWin->stCfg.stAttr.stOutRect = pstWin->stUsingAttr.stOutRect;
        }
#endif

#if 0
        printk("11111VS=%d,%d, FR=%d,%d, PFR=%d,%d, O=%d,%d, U=%d,%d, offset: %d, %d,%d, %d\n",
                pstInfo->stDispInfo.stVirtaulScreen.s32Width,
                pstInfo->stDispInfo.stVirtaulScreen.s32Height,
                pstInfo->stDispInfo.stFmtResolution.s32Width,
                pstInfo->stDispInfo.stFmtResolution.s32Height,
                pstInfo->stDispInfo.stPixelFmtResolution.s32Width,
                pstInfo->stDispInfo.stPixelFmtResolution.s32Height,
                stOutRectOrigin.s32Width,
                stOutRectOrigin.s32Height,
                pstWin->stUsingAttr.stOutRect.s32Width,
                pstWin->stUsingAttr.stOutRect.s32Height,
                pstInfo->stDispInfo.stOffsetInfo.u32Left,
                pstInfo->stDispInfo.stOffsetInfo.u32Top,
                pstInfo->stDispInfo.stOffsetInfo.u32Right,
                pstInfo->stDispInfo.stOffsetInfo.u32Bottom);
#endif
    }

    WinSendAttrToSource(pstWin, (MT_DISP_DISPLAY_INFO_S *)&pstInfo->stDispInfo);

    DISP_PRINT("Display info>> M=%d,S=%d,att=%d, 3d=%d, R=%d, I=%d, w=%d, h=%d, %dvs%d, rate=%d, cs=%d\n",
            pstInfo->stDispInfo.bIsMaster,
            pstInfo->stDispInfo.bIsSlave,
            pstInfo->stDispInfo.enAttachedDisp,
            pstInfo->stDispInfo.eDispMode,
            pstInfo->stDispInfo.bRightEyeFirst,
            pstInfo->stDispInfo.bInterlace,
            pstInfo->stDispInfo.stPixelFmtResolution.s32Width,
            pstInfo->stDispInfo.stPixelFmtResolution.s32Height,
            pstInfo->stDispInfo.stAR.u32ARw,
            pstInfo->stDispInfo.stAR.u32ARh,
            pstInfo->stDispInfo.u32RefreshRate,
            pstInfo->stDispInfo.eColorSpace);

    if (!pstWin->bEnable || pstWin->bMasked)
    {
#if 0 //disp_hal
        pstWin->stVLayerFunc.PF_SetEnable(pstWin->u32VideoLayer, pstWin->u32VideoRegionNo, MT_FALSE);
        //pstWin->stVLayerFunc.PF_Update(pstWin->u32VideoLayer);
#endif
        WinUpdatPara(pstWin);
        return;
    }

    pstWin->bInInterrupt = MT_TRUE;
    // window process
    if ((MT_DRV_DISP_C_VT_INT == pstInfo->eEventType) || (MT_DRV_DISP_C_DISPLAY_SETTING_CHANGE == pstInfo->eEventType))
    {
        MT_DRV_VIDEO_FRAME_S *pstFrame = MT_NULL;

        //printk("win003,");
        if (pstWin->bReset)
        {
            WIN_DEBUGK("%s %d reset Mode=%d\n", __FUNCTION__, __LINE__, pstWin->stRst.enResetMode);
            // release displayed and configed frame
            s_ResetPrint = 1;

            WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);

            ISR_WinReleaseUSLFrame(pstWin);

            // flush frame in full buffer pool
            //ISR_WinReleaseFullFrame(pstWin);
            pstFrame = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
            WinBuf_FlushWaitingFrame(&pstWin->stBuffer.stWinBP, pstFrame);

            if (pstWin->stRst.enResetMode == MT_DRV_WIN_SWITCH_BLACK)
            {
                //20180627
                //Bug: show logo -> still stop -> reset black screen -> play av program,
                //     will show the last logo for a while.
                //after reset to black screen, display need receive new frame to open screen.
                //different with freeze -> unfreeze, if unfreeze, then the screen will open at once;
                //and show the last freezed frame, that will be a bug.
                //if (DF_GetStatus() != DF_STATUS_PLAY)
                {
                    DF_SetCmd(pstWin->stBuffer.stDispBP, DF_CMD_START, MT_NULL);
                    MT_ALWAYS_PRINT("Reset Black, set Disp %d -> PLAY.\n",DF_GetStatus(pstWin->stBuffer.stDispBP));
                }

                /*do not use blackframe,just set the flag and mute the layer.*/
                WinSetBlackFrameFlag(pstWin);

                /*pass NULL frame-ptr into the func, just mute it internally.*/
                ISR_WinConfigFrame(pstWin, MT_NULL, pstInfo);

                WinClearBlackFrameFlag(pstWin);

#if 0
                //20180628, after clear logo, show black screen, should release the freeze buffer.
                //clear logo is not unfreeze/resume, but show black screen, and set Display to PLAY state.
                //so need release the freeze buffer here, instead of unfreeze to release it.
                DF_FreezeRelease(pstWin->stBuffer.stDispBP);
#endif

                //Bug: black screen channel switch, will show last channel's frames, do flush all frames
                DF_FlushDisp(pstWin->stBuffer.stDispBP, (int)MT_DRV_WIN_FLUSH_BOTH);
            }
            else
            {
                //                DF_SetCmd(DF_CMD_FREEZE);
                DF_SetCmd(pstWin->stBuffer.stDispBP, DF_CMD_FREEZE_COPY, MT_NULL);
                g_stDrvSetting[index].eDisplayStatus = DF_STATUS_STOP_FREEZE;

#if 0
                WinBuf_RepeatDisplayedFrame(&pstWin->stBuffer.stWinBP);
                pstFrame = WinBuf_GetConfigedFrame(&pstWin->stBuffer.stWinBP);

                if (!pstFrame)
                {
                    /*no frame to display, mute it.*/
                    WinSetBlackFrameFlag(pstWin);
                    ISR_WinConfigFrame(pstWin, MT_NULL, pstInfo);
                    WinClearBlackFrameFlag(pstWin);
                }
                else
                {
                    ISR_WinConfigFrame(pstWin, pstFrame, pstInfo);
                    WinClearBlackFrameFlag(pstWin);
                }
#endif

            }

            s_ResetPrint = 0;
            pstWin->enState = WIN_STATE_WORK;
            pstWin->bReset = MT_FALSE;
            pstWin->stRst.enResetMode = MT_DRV_WIN_SWITCH_BUTT;
        }
        else
        {
            // window state transfer
            if (pstWin->bUpState)
            {
                WIN_DEBUGK("%s %d status %d pstWin->enState %d\n", __FUNCTION__, __LINE__,
                    pstWin->enState, DF_GetStatus(pstWin->stBuffer.stDispBP));
                ISR_WinStateTransfer(pstWin);
                switch (pstWin->enState)
                {
                case WIN_STATE_RESUME:
                case WIN_STATE_UNFREEZE:
                case WIN_STATE_WORK:
                    //hmcccccccccccccccccccccccccccccccccccccccc
                    //DF_SetCmd(DF_CMD_START);
                    //disp_set_vid_layer_show(MT_TRUE);
                    DF_SetCmd(pstWin->stBuffer.stDispBP, DF_CMD_RESUME, MT_NULL);
                    g_stDrvSetting[index].eDisplayStatus = DF_STATUS_PLAY;
                    break;
                case WIN_STATE_PAUSE:
                    DF_SetCmd(pstWin->stBuffer.stDispBP, DF_CMD_PAUSE, MT_NULL);
                    g_stDrvSetting[index].eDisplayStatus = DF_STATUS_PAUSE;
                    break;
                case WIN_STATE_FREEZE:
                    DF_SetCmd(pstWin->stBuffer.stDispBP, DF_CMD_FREEZE, MT_NULL);
                    g_stDrvSetting[index].eDisplayStatus = DF_STATUS_FREEZE;
                    break;
                default:
                    break;
                }
                MT_INFO_VO("status %d\n",DF_GetStatus(pstWin->stBuffer.stDispBP));
            }
            ISR_FUNCTION(index);
            
            switch (pstWin->enState)
            {
            case WIN_STATE_RESUME:
            case WIN_STATE_UNFREEZE:
            case WIN_STATE_WORK:
                {
                    pstWin->enState = WIN_STATE_WORK;
                    //ISR_WinReleaseDisplayedFrame(pstWin);
                    //connie win_isr tap
#if 0
                    mt_u32 TmpTime_cur;
                    static mt_u32 last_tmp_time_master;
                    static mt_u32 last_tmp_time_slaver;


                    if(pstInfo->stDispInfo.bIsMaster)
                    {
                        MT_DRV_SYS_GetTimeStampMs(&TmpTime_cur);

                        printk("m_tap [%d]ms \n",TmpTime_cur - last_tmp_time_master);
                        last_tmp_time_master = TmpTime_cur;
                    }
                    else
                    {
                        MT_DRV_SYS_GetTimeStampMs(&TmpTime_cur);

                        printk("s_tap [%d]ms \n",TmpTime_cur - last_tmp_time_slaver);
                        last_tmp_time_slaver = TmpTime_cur;
                    }
#endif
                    ////////////////////////////////////////

                    WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);

                    ISR_WinReleaseUSLFrame(pstWin);

                    pstFrame = WinGetFrameToConfig(pstWin, pstInfo);

                    if (pstFrame)
                    {
                        if ((pstWin->enType == MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE || pstWin->enType == MT_DRV_WIN_ACTIVE_SINGLE) &&
                                (pstFrame->u32FrameIndex != pstWin->u32LastOutLowDelayIdx))
                        {

                            mt_ld_event_s evt;
                            mt_u32 TmpTime = 0;
                            //  MT_DRV_SYS_GetTimeStampMs(&TmpTime);

                            // printk("time 0x%x\n",TmpTime);

                            evt.evt_id = EVENT_VO_FRM_OUT;
                            evt.frame = pstFrame->u32FrameIndex;
                            evt.handle = pstFrame->hTunnelSrc;
                            evt.time = TmpTime;
                            mt_drv_ld_notify_event(&evt);
                            pstWin->u32LastOutLowDelayIdx = pstFrame->u32FrameIndex;
                        }
                        ISR_WinConfigFrame(pstWin, pstFrame, pstInfo);

                    }
                    else if (WinTestBlackFrameFlag(pstWin) == MT_TRUE)
                    {
                        /*no frame to display, just mute the layer.*/
                        ISR_WinConfigFrame(pstWin, MT_NULL, pstInfo);
                    }

                    /* last branch , no frame and it's the start of playing,
                     * just return.
                     */

                    break;
                }
            case WIN_STATE_PAUSE:
                {
                    WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);
                    ISR_WinReleaseUSLFrame(pstWin);

                    WinBuf_RepeatDisplayedFrame(&pstWin->stBuffer.stWinBP);

                    pstFrame = WinBuf_GetConfigedFrame(&pstWin->stBuffer.stWinBP);
                    if (!pstFrame)
                    {
                        WinSetBlackFrameFlag(pstWin);
                        //ISR_WinConfigFrame(pstWin, MT_NULL, pstInfo);
                        WinClearBlackFrameFlag(pstWin);
                    }
                    else if (pstFrame)
                    {
                        ISR_WinConfigFrame(pstWin, pstFrame, pstInfo);
                        WinClearBlackFrameFlag(pstWin);
                    }

                    break;
                }
            case WIN_STATE_FREEZE:
                {
                    mt_s32 nRet;

                    WinBuf_RlsAndUpdateUsingFrame(&pstWin->stBuffer.stWinBP);
                    ISR_WinReleaseUSLFrame(pstWin);

                    /*freeze mode, the buffer should flows as normal,
                      so we should let the buffer rounds.*/
                    WinBuf_RepeatDisplayedFrame(&pstWin->stBuffer.stWinBP);
                    pstFrame = WinBuf_GetConfigedFrame(&pstWin->stBuffer.stWinBP);

                    nRet = WinBuf_ReleaseOneFrame(&pstWin->stBuffer.stWinBP, pstFrame);
                    if (nRet != MT_SUCCESS)
                    {
                        /* u32UnderLoad happened */
                        pstWin->stBuffer.u32UnderLoad++;
                    }

                    if (pstWin->stFrz.enFreezeMode == MT_DRV_WIN_SWITCH_BLACK)
                    {
                        WinSetBlackFrameFlag(pstWin);
                        ISR_WinConfigFrame(pstWin, MT_NULL, pstInfo);
                        WinClearBlackFrameFlag(pstWin);
                    }
                    else if (pstFrame)
                    {
                        ISR_WinConfigFrame(pstWin, pstFrame, pstInfo);
                        WinClearBlackFrameFlag(pstWin);
                    }

                    break;
                }
            default:
                break;
            }
        }
    }

    ISR_WinUpdatePlayInfo(pstWin, pstInfo);

    pstWin->bInInterrupt = MT_FALSE;

    return;
}

mt_s32 WinGetProcIndex(mt_handle hWin, mt_u32 *p32Index)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();

    WinCheckNullPointer(p32Index);

    // s1 get window pointer
    pstWin = WinGetWindow(hWin);
    if (pstWin)
    {
        // return active windwo index
        *p32Index = pstWin->u32Index;
    }
    else
    {
        VIRTUAL_S *pstVirWin;

        pstVirWin = WinGetVirWindow(hWin);
        if (pstVirWin)
        {
            //return virtual window index
            *p32Index = pstVirWin->u32Index;
        }
        else
        {
            return MT_ERR_VO_WIN_NOT_EXIST;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WinGetProcInfo(mt_handle hWin, WIN_PROC_INFO_S *pstInfo)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();

    WinCheckNullPointer(pstInfo);

    DISP_MEMSET(pstInfo, 0, sizeof(WIN_PROC_INFO_S));

    // s1 get active window pointer
    pstWin = WinGetWindow(hWin);
    if (pstWin)
    {
        // get window proc info
        pstInfo->enType = pstWin->enType;
        pstInfo->u32Index = pstWin->u32Index;

        pstInfo->u32LayerId = (mt_u32)pstWin->u32VideoLayer;
        pstInfo->u32LayerRegionNo = (mt_u32)pstWin->u32VideoRegionNo;

        pstInfo->u32Zorder = pstWin->u32Zorder;
        pstInfo->bEnable = (mt_u32)pstWin->bEnable;
        pstInfo->bMasked = (mt_u32)pstWin->bMasked;
        pstInfo->u32WinState = (mt_u32)pstWin->enState;

        pstInfo->bReset = pstWin->bReset;
        pstInfo->enResetMode = pstWin->stRst.enResetMode;
        pstInfo->enFreezeMode = pstWin->stFrz.enFreezeMode;

        pstInfo->bQuickMode = pstWin->bQuickMode;
        pstInfo->bStepMode = pstWin->bStepMode;
        pstInfo->bVirtualCoordinate = pstWin->bVirtScreenMode;

        pstInfo->hSrc = (mt_u32)pstWin->stCfg.stSource.hSrc;
        pstInfo->pfAcqFrame = (ulong)pstWin->stCfg.stSource.pfAcqFrame;
        pstInfo->pfRlsFrame = (ulong)pstWin->stCfg.stSource.pfRlsFrame;
        pstInfo->pfSendWinInfo = (ulong)pstWin->stCfg.stSource.pfSendWinInfo;

        pstInfo->stAttr = pstWin->stCfg.stAttr;

        pstInfo->u32TBNotMatchCount = pstWin->u32TBNotMatchCount;

        pstInfo->eDispMode = pstWin->stCfg.eDispMode;
        pstInfo->bRightEyeFirst = pstWin->stCfg.bRightEyeFirst;

        pstInfo->hSlvWin = (mt_u32)pstWin->hSlvWin;
        pstInfo->bDebugEn = (mt_u32)pstWin->bDebugEn;

        pstInfo->u32ULSIn = pstWin->stBuffer.u32ULSIn;
        pstInfo->u32ULSOut = pstWin->stBuffer.u32ULSOut;
        pstInfo->u32UnderLoad = pstWin->stBuffer.u32UnderLoad;

        pstInfo->stWinInfoForDeveloper = pstWin->stWinInfoForDeveloper;

        //BP_GetBufState(&pstWin->stBuffer.stBP, (BUF_STT_S *)&(pstInfo->stBufState));
        WinBuf_GetStateInfo(&pstWin->stBuffer.stWinBP, (WB_STATE_S *)&(pstInfo->stBufState));

    }
    else
    {
        VIRTUAL_S *pstVirWin;

        pstVirWin = WinGetVirWindow(hWin);
        if (pstVirWin)
        {
            // get virtual proc info
            pstInfo->enType = pstVirWin->enType;
            pstInfo->u32Index = pstVirWin->u32Index;
            pstInfo->u32LayerId = 0xFFFFul; // not use video layer
            pstInfo->bEnable = (mt_u32)pstVirWin->bEnable;
            pstInfo->bMasked = (mt_u32)pstVirWin->bMasked;
            pstInfo->u32WinState = 0; // 0 means work

            pstInfo->hSrc = (mt_u32)pstVirWin->stSrcInfo.hSrc;
            pstInfo->pfAcqFrame = (ulong)pstVirWin->stSrcInfo.pfAcqFrame;
            pstInfo->pfRlsFrame = (ulong)pstVirWin->stSrcInfo.pfRlsFrame;
            pstInfo->pfSendWinInfo = (ulong)pstVirWin->stSrcInfo.pfSendWinInfo;

            pstInfo->stAttr = pstVirWin->stAttrBuf;

            pstInfo->stBufState.stRecord.u32TryQueueFrame = pstVirWin->stFrameStat.u32SrcQTry;
            pstInfo->stBufState.stRecord.u32QueueFrame = pstVirWin->stFrameStat.u32SrcQOK;

            pstInfo->stBufState.stRecord.u32Release = pstVirWin->stFrameStat.u32SinkRlsOK;
        }
        else
        {
            WIN_ERROR("WIN is not exist!\n");
            return MT_ERR_VO_WIN_NOT_EXIST;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WinGetCurrentImg(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    WINDOW_S *pstWin;
    MT_DRV_VIDEO_FRAME_S *pstFrmTmp = NULL;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstFrame);
    MT_INFO_VO("%x, 0000000000000000\n", hWin);
    WinCheckWindow(hWin, pstWin);
    DISP_MEMSET(pstFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    pstFrmTmp = WinBuf_GetDisplayedFrame(&pstWin->stBuffer.stWinBP);
    if (NULL == pstFrmTmp)
    {
        if (pstWin->latest_display_frame_valid)
        {
            *pstFrame = pstWin->latest_display_frame;
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else
    {
        *pstFrame = *(pstFrmTmp);
        pstWin->latest_display_frame = *(pstFrmTmp);
        pstWin->latest_display_frame_valid = 1;
    }

    return MT_SUCCESS;
}

mt_s32 WinCaptureFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame, mt_u32 *stMMZPhyAddr, mt_u32 *stMMZlen)
{
#if 0
    WINDOW_S *pstWin;
    MT_DRV_VIDEO_FRAME_S *pstFrmTmp = NULL;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = MT_NULL;
    mt_u32 datalen = 0, y_stride = 0, height = 0;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstFrame);
    WinCheckNullPointer(stMMZPhyAddr);
    WinCheckNullPointer(stMMZlen);
    WinCheckWindow(hWin, pstWin);
    DISP_MEMSET(pstFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    if (pstWin->enType == MT_DRV_WIN_VITUAL_SINGLE)
        return MT_ERR_VO_WIN_UNSUPPORT;

    if ( ((WIN_STATE_FREEZE == pstWin->enState)
                && (MT_DRV_WIN_SWITCH_BLACK == pstWin->stFrz.enFreezeMode))
            || ((MT_TRUE == pstWin->bReset)
                && (MT_DRV_WIN_SWITCH_BLACK == pstWin->stRst.enResetMode)))
    {
        pstFrmTmp = BP_GetBlackFrameInfo();
    }
    else
    {
        if(MT_SUCCESS != WinBuf_SetCaptureFrame(&pstWin->stBuffer.stWinBP, pstWin->u32WinCapMMZvalid))
            return MT_ERR_VO_INVALID_OPT;

        pstFrmTmp = WinBuf_GetCapturedFrame(&pstWin->stBuffer.stWinBP);
        if (MT_NULL == pstFrmTmp)
        {
            pstFrmTmp = BP_GetBlackFrameInfo();
        }
    }

    if (MT_NULL == pstFrmTmp)
    {
        WIN_ERROR("get null frame ptr\n");
        return MT_ERR_VO_BUFQUE_EMPTY;
    }
    *pstFrame = *(pstFrmTmp);
    pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(pstFrame->u32Priv);

    y_stride = pstFrame->stBufAddr[0].u32Stride_Y;
    height   = (MT_TRUE == pstPrivInfo->stCompressInfo.u32CompressFlag)
        ? pstPrivInfo->stCompressInfo.s32CompFrameHeight : pstFrame->u32Height;

    /*don't know why there exists these two branches, do we condidered gstreamer?*/
    if ( MT_DRV_PIX_FMT_NV21 == pstFrame->ePixFormat)
        datalen = height * y_stride * 3 / 2 + height * 4;
    else
        datalen = height * y_stride * 2 + height * 4;


    if(MT_SUCCESS != DISP_OS_MMZ_Alloc("VDP_Cpefun", MT_NULL, datalen, 16, &pstWin->stWinCaptureMMZ))
    {
        WinBuf_ReleaseCaptureFrame(&pstWin->stBuffer.stWinBP,pstFrame,MT_FALSE);
        return MT_ERR_VO_MALLOC_FAILED;
    }

    pstWin->u32WinCapMMZvalid  = 1;
    *stMMZPhyAddr = pstWin->stWinCaptureMMZ.u32StartPhyAddr;
    *stMMZlen      = pstWin->stWinCaptureMMZ.u32Size;
#endif
    return MT_SUCCESS;
}

mt_s32 WinReleaseCaptureFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();
    WinCheckNullPointer(pstFrame);
    WinCheckWindow(hWin, pstWin);

    if (pstWin->enType == MT_DRV_WIN_VITUAL_SINGLE)
        return MT_ERR_VO_WIN_UNSUPPORT;

    if (MT_SUCCESS != WinBuf_ReleaseCaptureFrame(&pstWin->stBuffer.stWinBP, pstFrame, MT_FALSE))
        return MT_ERR_VO_INVALID_OPT;

    return MT_SUCCESS;
}

mt_s32 WinFreeCaptureMMZBuf(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *cap_frame)
{
    WINDOW_S *pstWin;

    WinCheckDeviceOpen();
    WinCheckWindow(hWin, pstWin);

    if (cap_frame->stBufAddr[0].u32PhyAddr_Y != pstWin->stWinCaptureMMZ.u32StartPhyAddr)
    {
        WIN_ERROR("capture release addr:%x,%x!\n", cap_frame->stBufAddr[0].u32PhyAddr_Y,
                pstWin->stWinCaptureMMZ.u32StartPhyAddr);
        return MT_ERR_VO_INVALID_OPT;
    }

    if (pstWin->u32WinCapMMZvalid == 1)
    {
        DISP_OS_MMZ_Release(&pstWin->stWinCaptureMMZ);
        pstWin->stWinCaptureMMZ.u32StartPhyAddr = 0;
        pstWin->u32WinCapMMZvalid = 0;
    }
    else
    {
        WIN_WARN("warning: when free mmz, null refcnt occurs.!\n");
        return MT_ERR_VO_INVALID_OPT;
    }

    return MT_SUCCESS;
}

mt_s32 WinFreeCaptureMMZBuf2(WINDOW_S *pstWin)
{

    if (pstWin->u32WinCapMMZvalid == 1)
    {
        DISP_OS_MMZ_Release(&pstWin->stWinCaptureMMZ);
        pstWin->stWinCaptureMMZ.u32StartPhyAddr = 0;
        pstWin->u32WinCapMMZvalid = 0;
    }
    else
    {
        WIN_WARN("warning: when free mmz, null refcnt occurs.!\n");
    }

    return MT_SUCCESS;
}

mt_s32 WinForceClearCapture(mt_handle hWin)
{
    MT_DRV_VIDEO_FRAME_S *pstFrmTmp = NULL;
    WINDOW_S *pstWin = MT_NULL;

    WinCheckDeviceOpen();
    WinCheckWindow(hWin, pstWin);

    pstFrmTmp = WinBuf_GetCapturedFrame(&pstWin->stBuffer.stWinBP);
    if (NULL != pstFrmTmp)
    {
        if (MT_SUCCESS != WinBuf_ReleaseCaptureFrame(&pstWin->stBuffer.stWinBP, pstFrmTmp, MT_TRUE))
            return MT_ERR_VO_INVALID_OPT;
    }
    if (MT_SUCCESS != WinFreeCaptureMMZBuf2(pstWin))
        return MT_FAILURE;

    return MT_SUCCESS;
}

mt_s32 WinCapturePause(mt_handle hWin, MT_BOOL bCaptureStart)
{
    WINDOW_S *pstWin = MT_NULL, *pstWinAttach = MT_NULL;
    mt_s32 Ret = 0;
    mt_handle attach_handle = 0;

    pstWin = WinGetWindow(hWin);
    if (!pstWin)
    {
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (bCaptureStart && ((pstWin->enState == WIN_STATE_PAUSE) || (pstWin->enState == WIN_STATE_FREEZE)))
    {
        pstWin->bRestoreFlag = 0;
        return MT_SUCCESS;
    }

    if (pstWin->enType == MT_DRV_WIN_ACTIVE_MAIN_AND_SLAVE)
    {
        /*attach mode , need  pasue salve window too*/
        attach_handle = pstWin->hSlvWin;
        pstWinAttach = WinGetWindow(attach_handle);

        if (!pstWinAttach)
        {
            attach_handle = 0;
        }
    }
    else if (pstWin->enType == MT_DRV_WIN_ACTIVE_SINGLE)
    {
        /*not attach mode only need  pasue self window*/
        attach_handle = 0;
    }
    else if (pstWin->enType == MT_DRV_WIN_ACTIVE_SLAVE)
    {
        /*attach mode only    mce capture slave window*/
        attach_handle = 0;
    }

    if (bCaptureStart)
    {
        Ret = WIN_Pause(hWin, bCaptureStart);
        if (Ret != MT_SUCCESS)
            return Ret;

        if (attach_handle)
        {
            Ret = WIN_Pause(attach_handle, bCaptureStart);
            if (Ret != MT_SUCCESS)
            {
                (mt_void) WIN_Pause(hWin, !bCaptureStart);
                return Ret;
            }
        }

        pstWin->bRestoreFlag = 1;
    }
    else
    {
        if (pstWin->bRestoreFlag)
        {

            Ret = WIN_Pause(hWin, 0);
            if (attach_handle)
            {
                Ret |= WIN_Pause(attach_handle, 0);
            }
            pstWin->bRestoreFlag = 0;

            if (Ret != MT_SUCCESS)
                return Ret;
        }
    }

    return MT_SUCCESS;
}

mt_s32 WIN_SetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation)
{
    WINDOW_S *pstWin;
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    mt_s32 nRet = MT_SUCCESS;
    mt_s32 t;
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        nRet = DISP_GetDisplayInfo(pstWin->enDisp, &stDispInfo);
        if (nRet)
        {
            WIN_ERROR("DISP_GetDisplayInfo failed in %s!\n", __FUNCTION__);
            return MT_ERR_VO_CREATE_ERR;
        }
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
        pstWin->enRotation = enRotation;
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 1);
        t = 0;
        while (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            DISP_MSLEEP(5);
            t++;
            if (t > 10)
            {
                break;
            }
        }
        if (pstWin->hSlvWin)
        {
            nRet = WIN_SetRotation(pstWin->hSlvWin, enRotation);
        }
        if (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
            WIN_ERROR("WIN Set Attr timeout in %s\n", __FUNCTION__);
            return MT_ERR_VO_TIMEOUT;
        }
    }
    else
    {
        VIRTUAL_S *pstVirWindow;

        WinCheckVirWindow(hWin, pstVirWindow);

        pstVirWindow->enRotation = enRotation;

        WIN_VIR_SendAttrToSource(pstVirWindow);
    }
    return MT_SUCCESS;
}

mt_s32 WIN_GetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation)
{
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WINDOW_S *pstWin;
        WinCheckWindow(hWin, pstWin);
        *penRotation = pstWin->enRotation;
    }
    else
    {
        VIRTUAL_S *pstVirWindow;
        WinCheckVirWindow(hWin, pstVirWindow);
        *penRotation = pstVirWindow->enRotation;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_SetFlip(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip)
{
    WINDOW_S *pstWin;
    MT_DISP_DISPLAY_INFO_S stDispInfo;
    mt_s32 nRet = MT_SUCCESS;
    mt_s32 t;
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WinCheckWindow(hWin, pstWin);
        nRet = DISP_GetDisplayInfo(pstWin->enDisp, &stDispInfo);
        if (nRet)
        {
            WIN_ERROR("DISP_GetDisplayInfo failed in %s!\n", __FUNCTION__);
            return MT_ERR_VO_CREATE_ERR;
        }
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
        pstWin->bHoriFlip = bHoriFlip;
        pstWin->bVertFlip = bVertFlip;
        atomic_set(&pstWin->stCfg.bNewAttrFlag, 1);
        t = 0;
        while (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            DISP_MSLEEP(5);
            t++;
            if (t > 10)
            {
                break;
            }
        }
        if (pstWin->hSlvWin)
        {
            nRet = WIN_SetFlip(pstWin->hSlvWin, bHoriFlip, bVertFlip);
        }
        if (atomic_read(&pstWin->stCfg.bNewAttrFlag))
        {
            atomic_set(&pstWin->stCfg.bNewAttrFlag, 0);
            WIN_ERROR("WIN Set Attr timeout in %s\n", __FUNCTION__);
            return MT_ERR_VO_TIMEOUT;
        }
    }
    else
    {
        VIRTUAL_S *pstVirWindow;

        WinCheckVirWindow(hWin, pstVirWindow);

        pstVirWindow->bHoriFlip = bHoriFlip;
        pstVirWindow->bVertFlip = bVertFlip;

        WIN_VIR_SendAttrToSource(pstVirWindow);
    }
    return MT_SUCCESS;
}

mt_s32 WIN_GetFlip(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip)
{
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WINDOW_S *pstWin;
        WinCheckWindow(hWin, pstWin);
        *pbHoriFlip = pstWin->bHoriFlip;
        *pbVertFlip = pstWin->bVertFlip;
    }
    else
    {
        VIRTUAL_S *pstVirWindow;
        WinCheckVirWindow(hWin, pstVirWindow);
        *pbHoriFlip = pstVirWindow->bHoriFlip;
        *pbVertFlip = pstVirWindow->bVertFlip;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_Test(mt_handle hWin)
{
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        Win_TestSR();
        Win_TestPost();
        Win_TestDci();
    }

    return MT_SUCCESS;
}

mt_s32 WIN_GetUnload(mt_handle hWin, mt_u32 *pu32Times)
{
    MT_BOOL bVirtual;
    WinCheckDeviceOpen();
    bVirtual = WinCheckVirtual(hWin);
    if (!bVirtual)
    {
        WINDOW_S *pstWin;
        WinCheckWindow(hWin, pstWin);
        *pu32Times = pstWin->stBuffer.u32UnderLoad;
    }
    else
    {
        *pu32Times = 0;
        return MT_ERR_VO_WIN_UNSUPPORT;
    }
    return MT_SUCCESS;
}

mt_s32 WIN_SetPara(MT_DRV_WIN_PARA_S *arg)
{
    WINDOW_S *pstWin;
    MT_DRV_WIN_PARA_S  *para = arg;
    mt_handle hWin;
    mt_rect_s src_rec={0},dst_rec={0};
    if(!para)
        return MT_ERR_VO_INVALID_OPT;
    hWin = para->hwin;
    int index = 0;
    MT_DRV_DISPLAY_E enDisp;
    WinCheckDeviceOpen();

    WinCheckWindow(hWin, pstWin);
    if(para->u32ctlcmd & 0xff)
    {        
        WinGetIndex(hWin, &enDisp, &index);
        g_stDrvSetting[index].eDisplayStatus=DF_STATUS_PLAY;
        if(para->u32ctlcmd != 1)
        {
            dst_rec.s32X = para->send[0]>>16;
            dst_rec.s32Y = para->send[1]>>16;
            dst_rec.s32Width= para->send[0]&0xffff;
            dst_rec.s32Height= para->send[1]&0xffff;
            WIN_Set_DF_Window(index, &src_rec,&dst_rec);
        }

#if 1
        //DF_ISR_Update(&pstWin->stBuffer.stDispBP, &g_stDrvSetting);
        //DF_ISR_Update(&pstWin->stBuffer.stDispBP, &g_stDrvSetting);
        //DF_ISR_Update(&pstWin->stBuffer.stDispBP, &g_stDrvSetting);
#if defined(CONFIG_MT_CHIP_ARIA)
        hal_put_u32(p_disp_addr + 0, hal_get_u32(p_disp_addr + 0) | 1);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        //#ifdef DEBUG_SCREEN_OP
        MT_ALWAYS_PRINT("call reg_symphony_disp_drv_set_display_ctrl_vid_sel(%d)\n",1);
        //#endif
        reg_symphony_disp_drv_set_display_ctrl_vid_sel(1);
#endif

        MT_INFO_VO("---------------\n");

#else
        DF_cfg_TestIMG();
        DF_cfg_TestIMG_SD();
        hal_put_u32(p_disp_addr + 0, hal_get_u32(p_disp_addr + 0) | 1);
        printk("[%s]line %d,start\n", __FUNCTION__, __LINE__);
#endif
    }
    if (para->u32ctlcmd & 0xff00)
    {
        mmz_phy_addr = para->send[0];
        MT_INFO_VO("phyaddr %x\n",mmz_phy_addr);
    }
    DF_cfg_TestIMG();
    return MT_SUCCESS;
}

mt_s32 WIN_GetPara(MT_DRV_WIN_PARA_S *arg)
{
    if (!arg)
        return MT_ERR_VO_INVALID_OPT;

    return MT_SUCCESS;
}

mt_s32 WIN_ClearAllFrame(MT_DRV_WIN_PARA_S *arg)
{
    //WINDOW_S *pstWin;
    mt_s32 t=0;
    MT_DRV_WIN_PARA_S  *para = arg;
    int index = 0;
    MT_DRV_DISPLAY_E enDisp;
    
    WinGetIndex(arg->hwin, &enDisp, &index);
    if(atomic_read(&bCleanFifoFlag))
        return MT_FAILURE;

    g_FlushDispType = (MT_DRV_WIN_FLUSH_TYPE_E)para->send[0];
#if !defined(CONFIG_MT_DDR_SIZE_128)
    if (((g_FlushDispType&MT_DRV_WIN_FLUSH_DISPLAY_QUEUE) != 0) && (g_pstWin[index] != MT_NULL))
    {
        //FIXME
        //For 128M solution, when "Resolution Change Event" call WIN_ClearAllFrame(),
        //maybe can NOT call DF_FreezeMalloc() to allocate freeze buffer from AV CPU's VFMW.
        //If call DF_FreezeMalloc(), it's timing is not correct with "Resolution Change Done Ack"!
        //DF_FreezeMalloc();

        DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_COPY, MT_NULL);
    }
#endif

    atomic_set(&bCleanFifoFlag, 1);

#if !defined(CONFIG_MT_DDR_SIZE_128)
    if (((g_FlushDispType&MT_DRV_WIN_FLUSH_DISPLAY_QUEUE) != 0) && (g_pstWin[index] != MT_NULL))
    {
        DF_Wait_For_FreezeCopy_Done(g_pstWin[index]->stBuffer.stDispBP);

        DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_RESUME, MT_NULL);

    }
#endif

    MT_INFO_VO("---------------\n");
    while (atomic_read(&bCleanFifoFlag))
    {
        DISP_MSLEEP(5);
        t++;

        if (t > 10)
        {
            break;
        }
    }
    if(atomic_read(&bCleanFifoFlag))
        return MT_FAILURE;
    return MT_SUCCESS;
}

mt_s32 WIN_Para_Init(void)
{
    int index = 0;
    for(index=0; index<MAX_WIN_NUM; index++)
    {
        g_pstWin[index] = MT_NULL;
        memset(&g_stDrvSetting[index], 0 , sizeof(DF_DRV_SETTING_S));
    }
    return MT_SUCCESS;
}

