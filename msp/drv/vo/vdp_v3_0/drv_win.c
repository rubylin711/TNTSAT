/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_win.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2017/01/20
  Description   :
  History       :
  1.Date        :
  Author        :
  Modification  : Created file

*******************************************************************************/

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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
#include <linux/smp_lock.h>
#endif
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_kernel_adapt.h"

#include "mt_drv_module.h"

#include "mt_drv_disp.h"
#include "drv_disp.h"
#include "drv_display.h"
#include "drv_disp_debug.h"
#include "drv_disp_ext.h"
#include "drv_disp_osal.h"
#include "drv_disp_debug.h"

#include "mt_drv_win.h"
#include "drv_win_ext.h"
#include "drv_win_ioctl.h"
#include "drv_window.h"
#include "mt_osal.h"
//#include "drv_win_hal.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

static atomic_t    g_VoCount = ATOMIC_INIT(0);
//WIN_GLOBAL_STATE_S   g_VoGlobalState;
WIN_STATE_S          g_VoModState;
MT_BOOL     g_VoSuspend = MT_FALSE;
mt_u32  g_bWinPrivateInformation = MT_FALSE;


MT_DECLARE_MUTEX(g_VoMutex);

mt_u8 *g_pAlgLocation[ALG_LOCATION_IN_BUTT] = {
    "In V0",
    "In VP",
};

mt_u8 *g_pWinTrueString[2] = {
    "False",
    "True ",
};

mt_u8 *g_pWinYNString[2] = {
    "N",
    "Y",
};

mt_u8 *g_pWinEnableString[2] = {
    "Disable",
    "Enable ",
};
mt_u8 *g_pWinRotateString[5] = {
    "Rotation_00",
    "Rotation_90 ",
    "Rotation_180 ",
    "Rotation_270 ",
    "Rotation_butt ",
};
//WIN_PROC_INFO_S s_stProcInfo;
//static mt_u32 s_WinProcId[MT_DRV_DISPLAY_BUTT][WINDOW_MAX_NUMBER]={WINDOW_INVALID_ID};
mt_u8 *g_pWinStateString[6] = {
    "Run        ",
    "Pause      ",
    "Resume     ",
    "FreezeLast ",
    "FreezeBlack",
};

mt_u8 *g_pWinDispModeString[DISP_STEREO_BUTT] = {
    "2D",
    "FPK",
    "SBS_HALF",
    "TAB",
    "FILED_ALTE",
    "LINE_ALTE",
    "SBS_FULL",
    "L_DEPTH",
    "LDEP_GDEP",
};

mt_u8 *g_pWinQucikString[2] = {
    " ",
    "+QuickMode",
};

mt_u8 *g_pWinStepString[2] = {
    " ",
    "+StepMode",
};


mt_u8 *g_pWinFreezeString[2] = {"LAST", "BLACK"};

mt_u8 *g_pWinTpyeString[MT_DRV_WIN_BUTT] = {
    "Display",
    "Virtual",
    "Main   ",
    "Slave  ",
};

mt_u8 *g_pWinAspectCvrsString[MT_DRV_ASP_RAT_MODE_BUTT] = {
    "Full      ",
    "LetterBox ",
    "PanAndScan",
    "Combined  ",
    "FullHori  ",
    "FullVert  ",
    "Customer  "
};

mt_u8 *g_pWinFrameTypetring[MT_DRV_FT_BUTT] = {
    "2D",
    "SideBySid",
    "TopAndBottom",
    "MVC",
};

mt_u8 *g_pWinFieldModeString[MT_DRV_FIELD_BUTT] = {
    "Top",
    "Bottom",
    "All",
};

mt_u8 *g_pWinMemTypeString[] = {
    "WinSupply",
    "UserSupply",
};

mt_u8 *g_pWinFieldTypeString[MT_DRV_FIELD_BUTT+1] = {
    "Top",
    "Bottom",
    "Frame",
    "BUTT",
};

mt_u8 *g_pPixFmtString[] = {
    "NV12       ",
    "NV21       ",
    "NV12CMP    ",
    "NV21CMP    ",
    "NV12TILE   ",
    "NV21TILE   ",
    "NV12TILECMP",
    "NV21TILECMP",
    "YUYV       ",
    "YYUV       ",
    "YVYU       ",
    "UYVY       ",
    "VYUY       ",
    "           ",
};

mt_u8 g_pPixFmtUnknown[20] = {0};


mt_u8 *g_pSourceString[] = {
    "Unknown",
    "AVPlay ",
    "VI     ",
};

mt_u8 *DrvWinGetPixfmtString(MT_DRV_PIX_FORMAT_E enPixfmt)
{
    switch(enPixfmt)
    {
        case MT_DRV_PIX_FMT_NV12:
            return g_pPixFmtString[0];
        case MT_DRV_PIX_FMT_NV21:
            return g_pPixFmtString[1];
        case MT_DRV_PIX_FMT_NV12_CMP:
            return g_pPixFmtString[2];
        case MT_DRV_PIX_FMT_NV21_CMP:
            return g_pPixFmtString[3];
        case MT_DRV_PIX_FMT_NV12_TILE:
            return g_pPixFmtString[4];
        case MT_DRV_PIX_FMT_NV21_TILE:
            return g_pPixFmtString[5];
        case MT_DRV_PIX_FMT_NV12_TILE_CMP:
            return g_pPixFmtString[6];
        case MT_DRV_PIX_FMT_NV21_TILE_CMP:
            return g_pPixFmtString[7];
        case MT_DRV_PIX_FMT_YUYV:
            return g_pPixFmtString[8];
        case MT_DRV_PIX_FMT_YYUV:
            return g_pPixFmtString[9];
        case MT_DRV_PIX_FMT_YVYU:
            return g_pPixFmtString[10];
        case MT_DRV_PIX_FMT_UYVY:
            return g_pPixFmtString[11];
        case MT_DRV_PIX_FMT_VYUY:
            return g_pPixFmtString[12];
        default:
            mt_osal_snprintf(g_pPixFmtUnknown, 10, "%d", enPixfmt);
            return g_pPixFmtUnknown;
    }

}

mt_s32 WIN_ProcParsePara(mt_char *pProcPara,mt_char **ppItem,mt_char **ppValue)
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

extern mt_u8 *g_pVDPColorSpaceString[MT_DRV_CS_BUTT];
mt_s32 DRV_WIN_ProcRead(struct seq_file *p, mt_void *v)
{
    mt_proc_entry_t *pProcItem;
    WINBUF_STATE_S *pstBuffer;
    MT_DRV_VIDEO_FRAME_S *pstNewFrame;
    MT_DRV_VIDEO_PRIVATE_S *pstPriv;
    WIN_PROC_INFO_S *pstProcInfo;
    mt_handle hWin;
    mt_s32 nRet;
    mt_u32 i;
    mt_u8 *pu8WinSate;

    nRet = down_interruptible(&g_VoMutex);
    pProcItem = p->private;

    hWin = (mt_handle)pProcItem->data;

    pstProcInfo = (WIN_PROC_INFO_S *)DISP_MALLOC(sizeof(WIN_PROC_INFO_S));
    if (!pstProcInfo)
    {
        PROC_PRINT(p,"-------- Malloc Proc Buffer Failed!--------\n");
        goto _ERR_EXIT_;
    }

    nRet = WinGetProcInfo(hWin, pstProcInfo);
    if (nRet)
    {
        MT_ERR_WIN("WinGetProcInfo FAILED!\n");
        goto _ERR_EXIT_;
    }

    PROC_PRINT(p,"------------------------------Win%04x[Z=%d]------------------------------------\n"
                      "------------Win Info-------------------|-------------Frame Info-------------------\n",
                        (mt_u32)(pstProcInfo->u32Index & 0xffffUL),
                        pstProcInfo->u32Zorder);

    switch(pstProcInfo->u32WinState)
    {
        case 1:
            pu8WinSate = g_pWinStateString[1];
            break;
        case 2:
            pu8WinSate = g_pWinStateString[2];
            break;
        case 3:
            if (pstProcInfo->enFreezeMode == MT_DRV_WIN_SWITCH_LAST)
            {
                pu8WinSate = g_pWinStateString[3];
            }
            else
            {
                pu8WinSate = g_pWinStateString[4];
            }
            break;
        default:
            pu8WinSate = g_pWinStateString[0];
            break;
    }

#if 0
    nRet = WinGetProcIndex(pstProcInfo->hSlvWin, &u32SlvWinIndex);
    if (nRet != MT_SUCCESS )
    {
        u32SlvWinIndex = 0xFFFFFFFFul
    }
#endif

    /*
    ------------------------------Win0100 [Z=1]-------------------------------------
    ------------Win Info-----------------|-------------Frame Info-------------------
    Enable              :TRUE            |Type/PixFmt         :NotStereo/NV12
    State               :Working         |Circurotate         :FALSE
    Type                :Main            |W/H(WvsH)           :1920/1080(16:9)
    LayerID             :0               |Disp(X/Y/W/H)       :0/0/1920/1080
    AspectRatioConvert  :Full            |FrameRate           :50
    CustAspectRatio     :MT_FALSE        |ColorSpace          :BT709_YUV_LIMITED
    Crop                :Enable          |Fieldmode(Origin)   :Frame(Bottom)
    Crop(L/T/R/B)       :0/0/0/0         |OriRect(X/Y/W/H)    :0/0/720/576
    In  (X/Y/W/H)       :0/0/0/0         |FrameIndex          :0x15eb5
    Out (X/Y/W/H)       :0/0/1280/720    |SrcPTS/PTS          :0x31a7345/0x31a7359
    DispMode/RightFirst :2D/MT_FALSE     |PlayTime            :1
    Masked              :FALSE           |FieldMode           :All
    AttachSource        :AVPLAY0         |Fidelity            :0
    CallBack            :Acquire(N)      |Y/CAddr             :0x23719000/0x23913400
                        :Release(Y)      |Y/CStride           :0x780/0x780
                        :SetAttr(Y)      |
    SlaveID             :win0000         |
    ----VirtualWindowInfo-----           |
    MemoryType          :NA              |
    BufferNum           :0               |
    PixelFormat         :NA              |
    */

    pstNewFrame = &pstProcInfo->stBufState.stCurrentFrame;
    pstPriv     = (MT_DRV_VIDEO_PRIVATE_S *)(&pstNewFrame->u32Priv[0]);
    PROC_PRINT(p, "%-19s:%-20s", "Enable", g_pWinTrueString[pstProcInfo->bEnable]);
    PROC_PRINT(p, "%-19s:%-10s/%-9s\n", "|Type/PixFmt", g_pWinFrameTypetring[pstNewFrame->eFrmType],
                                                            DrvWinGetPixfmtString(pstNewFrame->ePixFormat));
    PROC_PRINT(p, "%-19s:%-20s", "State", pu8WinSate);
    PROC_PRINT(p, "%-19s:%-20s\n", "|Rotation", g_pWinRotateString[pstNewFrame->u32Circumrotate > 0]);
    PROC_PRINT(p, "%-19s:%-20s", "Type", g_pWinTpyeString[pstProcInfo->enType]);
    PROC_PRINT(p, "%-19s:%4d/%4d(%4d:%4d)\n", "|W/H(Aspect W:H)", pstNewFrame->u32Width, pstNewFrame->u32Height,
                                                              (mt_u32)pstNewFrame->u32AspectWidth,
                                                              (mt_u32)pstNewFrame->u32AspectHeight);
    PROC_PRINT(p, "%-19s:%-20X", "*LayerID", pstProcInfo->u32LayerId);


    PROC_PRINT(p, "%-19s:%4d/%4d/%4d/%4d \n", "|Disp(X/Y/W/H)",
                                                    pstNewFrame->stDispRect.s32X,
                                                    pstNewFrame->stDispRect.s32Y,
                                                    pstNewFrame->stDispRect.s32Width,
                                                    pstNewFrame->stDispRect.s32Height);
    PROC_PRINT(p, "%-19s:%-20s", "AspectRatioConvert", g_pWinAspectCvrsString[pstProcInfo->stAttr.enARCvrs]);
    PROC_PRINT(p, "%-19s:%d.%d\n", "|FrameRate", pstNewFrame->u32FrameRate/100,
                                        pstNewFrame->u32FrameRate - (pstNewFrame->u32FrameRate - (pstNewFrame->u32FrameRate/100 * 100))
                                        );

    PROC_PRINT(p, "%-19s:%-4d:%-15d", "CustAspectRatio", (mt_u32)pstProcInfo->stAttr.stCustmAR.u32ARw,
                                                              (mt_u32)pstProcInfo->stAttr.stCustmAR.u32ARh);
    PROC_PRINT(p, "%-19s:%-20s\n", "|ColorSpace", g_pVDPColorSpaceString[pstPriv->eColorSpace]);

    PROC_PRINT(p, "%-19s:%-20s", "Crop", g_pWinTrueString[pstProcInfo->stAttr.bUseCropRect]);
    PROC_PRINT(p, "%-19s:%s(%s)\n", "|Fieldmode(Origin)", g_pWinFieldTypeString[pstNewFrame->enFieldMode],
                                                                    g_pWinFieldTypeString[pstPriv->eOriginField]);
    PROC_PRINT(p, "%-19s:%4x/%4x/%4x/%4x ", "Crop(L/T/R/B) ",  pstProcInfo->stAttr.stCropRect.u32LeftOffset,
                                                                    pstProcInfo->stAttr.stCropRect.u32TopOffset,
                                                                    pstProcInfo->stAttr.stCropRect.u32RightOffset,
                                                                    pstProcInfo->stAttr.stCropRect.u32BottomOffset);
    PROC_PRINT(p, "%-19s:%d/%d/%d/%d\n", "|OriRect(X/Y/W/H)",  0,
                                                               0,
                                                                    pstPriv->stVideoOriginalInfo.u32Width,
                                                                    pstPriv->stVideoOriginalInfo.u32Height);
    PROC_PRINT(p, "%-19s:%4d/%4d/%4d/%4d ", "In  (X/Y/W/H)",pstProcInfo->stAttr.stInRect.s32X,
                                                                pstProcInfo->stAttr.stInRect.s32Y,
                                                                pstProcInfo->stAttr.stInRect.s32Width,
                                                                pstProcInfo->stAttr.stInRect.s32Height);
    PROC_PRINT(p, "%-19s:0x%x\n", "|FrameIndex",  pstNewFrame->u32FrameIndex);
    PROC_PRINT(p, "%-19s:%4d/%4d/%4d/%4d ", "Out(X/Y/W/H)", pstProcInfo->stAttr.stOutRect.s32X,
                                                                pstProcInfo->stAttr.stOutRect.s32Y,
                                                                pstProcInfo->stAttr.stOutRect.s32Width,
                                                                pstProcInfo->stAttr.stOutRect.s32Height);
    PROC_PRINT(p, "%-19s:0x%x/0x%x\n", "|SrcPTS/PTS",  pstNewFrame->u32SrcPts, pstNewFrame->u32Pts);
    PROC_PRINT(p, "%-19s:%-10s/%-9s", "DispMode/RightFirst", g_pWinDispModeString[pstProcInfo->eDispMode],
                                                                 g_pWinTrueString[pstProcInfo->bRightEyeFirst]);
    PROC_PRINT(p, "%-19s:%d\n", "|PlayTime", pstPriv->u32PlayTime);
    PROC_PRINT(p, "%-19s:%-20s", "*Masked", g_pWinTrueString[pstProcInfo->bMasked]);
    PROC_PRINT(p, "%-19s:%-20s\n", "|FieldMode", g_pWinFieldModeString[pstNewFrame->enFieldMode]);
    PROC_PRINT(p, "%-19s:%-20s", "AttachSource", g_pWinTrueString[pstProcInfo->hSrc!= MT_NULL]);
    PROC_PRINT(p, "%-19s:%d\n", "|Fidelity", pstPriv->u32Fidelity);
    PROC_PRINT(p, "%-19s:%-20s", "*CallBack(Acquire)", g_pWinYNString[pstProcInfo->pfAcqFrame    !=MT_NULL]);
    PROC_PRINT(p, "%-19s:0x%lx/0x%x\n", "|YAddr/YStride", pstNewFrame->stBufAddr[0].u32PhyAddr_Y,pstNewFrame->stBufAddr[0].u32Stride_Y);
    PROC_PRINT(p, "%-19s:%-20s", "*CallBack(Release)", g_pWinYNString[pstProcInfo->pfRlsFrame    !=MT_NULL]);
    PROC_PRINT(p, "%-19s:0x%lx/0x%x\n", "|CAddr/CStride", pstNewFrame->stBufAddr[0].u32PhyAddr_C,pstNewFrame->stBufAddr[0].u32Stride_C);
    PROC_PRINT(p, "%-19s:%-20s|\n", "*CallBack(SetAttr)", g_pWinYNString[pstProcInfo->pfSendWinInfo !=MT_NULL]);



    if (pstProcInfo->hSlvWin)
    {
        PROC_PRINT(p, "%-19s:%04X                |\n", "SlaveWinID", (mt_u32)(pstProcInfo->hSlvWin & 0xFFFFul));
    }
    else
    {
        PROC_PRINT(p, "%-19s:%04X                |\n", "SlaveWinID", (mt_u32)0xFFFFul);
    }

    PROC_PRINT(p, "%-19s:%-4d                |\n", "bQuickoutMode", pstProcInfo->bQuickMode);
    PROC_PRINT(p, "%-19s:%-4d                |\n", "bVirtualCoordinate", pstProcInfo->bVirtualCoordinate);

    if (g_bWinPrivateInformation)
    {
        PROC_PRINT(p, "%-19s:%-20X \n", "*LayerRegionNum", pstProcInfo->u32LayerRegionNo);
        PROC_PRINT(p, "%-19s:%-20X \n", "*Zorder", pstProcInfo->u32Zorder);
        if (pstProcInfo->stWinInfoForDeveloper.stSrDciPhysicalInfo.bDci
            && pstProcInfo->stWinInfoForDeveloper.stSrDciPhysicalInfo.bSR)
        {

            PROC_PRINT(p, "%-19s\n", "-------------------SR And DCI Location Info----------------");

            PROC_PRINT(p, "%-19s:%-20s \n", "SR  Location:",  g_pAlgLocation[pstProcInfo->stWinInfoForDeveloper.stSrDciPhysicalInfo.eSrLocation]);
            PROC_PRINT(p, "%-19s:%-20s \n", "DCI Location:",  g_pAlgLocation[pstProcInfo->stWinInfoForDeveloper.stSrDciPhysicalInfo.eDciLocation]);
            PROC_PRINT(p, "%-19s:%-20s \n", "SR Behind DCI:", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.stSrDciPhysicalInfo.bSrBehindDci]);

            PROC_PRINT(p, "%-19s\n", "-------------------SR INFO---------------------------------------");

            PROC_PRINT(p, "%-19s:%-20s \n", "Hor SR Pre  Enable", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bHorSrOpenInPreProcess]);
            PROC_PRINT(p, "%-19s:%-20s \n", "Hor SR Post Enable", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bHorSrOpenInPostProcess]);
            PROC_PRINT(p, "%-19s:%-20s \n", "Ver SR Pre  Enable", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bVerSrOpenInPreProcess]);
            PROC_PRINT(p, "%-19s:%-20s \n", "Ver SR Post Enable", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bVerSrOpenInPostProcess]);

#if 0
            if (pstProcInfo->stWinInfoForDeveloper.bHorSrOpenInPreProcess
                && pstProcInfo->stWinInfoForDeveloper.bHorSrOpenInPostProcess
                && pstProcInfo->stWinInfoForDeveloper.bVerSrOpenInPreProcess
                && pstProcInfo->stWinInfoForDeveloper.bVerSrOpenInPostProcess)
#endif
            if (1)

            {

                PROC_PRINT(p, "%-25s%d \n", "Current Winnum:", pstProcInfo->stWinInfoForDeveloper.u32WinNum);
                PROC_PRINT(p, "%-25s%d/%d \n", "Current FMT(w/h):", pstProcInfo->stWinInfoForDeveloper.eCurrentFmt.s32Width,
                                                                   pstProcInfo->stWinInfoForDeveloper.eCurrentFmt.s32Height);
                PROC_PRINT(p, "%-25s%s \n", "bExistScaleDown_WhenRatioRevise:", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bExistScaleDown_WhenRatioRevise]);
                PROC_PRINT(p, "%-25s%s \n", "bIn3DMode:", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bIn3DMode]);

                PROC_PRINT(p, "%-25s%d/%d \n", "Original frame Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stOringinFrameSize.s32Width,
                                                                   pstProcInfo->stWinInfoForDeveloper.stOringinFrameSize.s32Height);

                PROC_PRINT(p, "%-25s%d/%d/%d/%d \n", "Window Output Coordinate(x/y/w/h):", pstProcInfo->stWinInfoForDeveloper.stFinalWinOutputSize.s32X,
                                                                   pstProcInfo->stWinInfoForDeveloper.stFinalWinOutputSize.s32Y,
                                                                   pstProcInfo->stWinInfoForDeveloper.stFinalWinOutputSize.s32Width,
                                                                   pstProcInfo->stWinInfoForDeveloper.stFinalWinOutputSize.s32Height);

                PROC_PRINT(p, "%-25s%d/%d/%d/%d \n", "V0'S Output Coordinate(x/y/w/h):",
                                                                   pstProcInfo->stWinInfoForDeveloper.stOutputSizeOfV0.s32X,
                                                                   pstProcInfo->stWinInfoForDeveloper.stOutputSizeOfV0.s32Y,
                                                                   pstProcInfo->stWinInfoForDeveloper.stOutputSizeOfV0.s32Width,
                                                                   pstProcInfo->stWinInfoForDeveloper.stOutputSizeOfV0.s32Height);

                PROC_PRINT(p, "%-25s%d/%d \n", "Vdp Require Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stVdpRequire.s32Width,
                                                       pstProcInfo->stWinInfoForDeveloper.stVdpRequire.s32Height);

                PROC_PRINT(p, "%-25s%d/%d \n", "Vpss Give Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stVpssGive.s32Width,
                                           pstProcInfo->stWinInfoForDeveloper.stVpssGive.s32Height);

                PROC_PRINT(p, "%-25s%d/%d \n", "Sr's output Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stSrOutputSize.s32Width,
                                           pstProcInfo->stWinInfoForDeveloper.stSrOutputSize.s32Height);

                PROC_PRINT(p, "%-25s%d/%d \n", "Sr's output Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stSrOutputSize.s32Width,
                                           pstProcInfo->stWinInfoForDeveloper.stSrOutputSize.s32Height);
            }

            PROC_PRINT(p, "%-25s\n", "-------------------DCI INFO-------------------------------------------");
            PROC_PRINT(p, "%-25s%-20s \n", "Dci Enable:", g_pWinTrueString[pstProcInfo->stWinInfoForDeveloper.bDciOpen]);

            if (pstProcInfo->stWinInfoForDeveloper.bDciOpen)
            {
                PROC_PRINT(p, "%-25s%d/%d \n", "Vpss Give Frame Size(w/h):", pstProcInfo->stWinInfoForDeveloper.stDciFrameSize.s32Width,
                           pstProcInfo->stWinInfoForDeveloper.stDciFrameSize.s32Height);

                PROC_PRINT(p, "%-25s%d/%d/%d/%d \n", "Vpss Give Dci Position(x/y/x'/y'):",
                                           pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32X,
                                           pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32Y,
                                           pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32Width + pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32X,
                                           pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32Height + pstProcInfo->stWinInfoForDeveloper.stOriginDCIPositionInFrame.s32Y);

                PROC_PRINT(p, "%-25s%d/%d/%d/%d \n", "DCI  input  effective content size(x/y/w/h):",
                                           pstProcInfo->stWinInfoForDeveloper.stDciEffecttiveContentInputSize.s32X,
                                           pstProcInfo->stWinInfoForDeveloper.stDciEffecttiveContentInputSize.s32Y,
                                           pstProcInfo->stWinInfoForDeveloper.stDciEffecttiveContentInputSize.s32Width,
                                           pstProcInfo->stWinInfoForDeveloper.stDciEffecttiveContentInputSize.s32Height);

                PROC_PRINT(p, "%-25s%d/%d/%d/%d \n", "DCI  Config   Position(x/y/x'/y'):",
                                           pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32X,
                                           pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32Y,
                                           pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32Width + pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32X,
                                           pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32Height + pstProcInfo->stWinInfoForDeveloper.stWinFinalPosition.s32Y);
            }
        }
    }

    if (pstProcInfo->stAttr.bVirtual)
    {
        PROC_PRINT(p, "----VirtualWindowInfo-----           |\n");
        PROC_PRINT(p, "%-19s:%-20s\n", "MemoryType", g_pWinMemTypeString[pstProcInfo->stAttr.bUserAllocBuffer==MT_TRUE]);
        PROC_PRINT(p, "%-19s:%-20d\n", "BufferNum", pstProcInfo->stAttr.u32BufNumber);
        PROC_PRINT(p, "%-19s:%-20s\n", "PixelFormat", DrvWinGetPixfmtString(pstProcInfo->stAttr.enDataFormat));
    }

/*
-----------------------Buffer State---------------------------------
Queue(Try/OK)       :179584/179581
Dequeue(Try/OK)     :179558/179558
Underload           :13750977
Discard             :4
UndispFrame(Q/DQ)   :5/5
FieldUnmatchCnt     :32
-------------------------------------------------------------------
BufferQueue[state, FrameID]
(State: 1,Empty[15]; 2,Write[0]; 3,ToDisp[0]; 4,Disp[1]; 5,Disped[0])
[1,0x5eaf] [1,0x5eb0] [1,0x5eb0] [1,0x5eb1]
[1,0x5eb1] [1,0x5eb2] [1,0x5eb2] [1,0x5eb3]
[1,0x5eb3] [1,0x5eb4] [1,0x5eb5] [1,0x5eb4]
[4,0x5eb5] [1,0x5eae] [1,0x5eae] [1,0x5eaf]
*/

    pstBuffer = &pstProcInfo->stBufState;

    PROC_PRINT(p,
        "-----------------------Buffer State---------------------------------\n"
        "Queue(Try/OK)       :%d/%d\n"
        "Release(Try/OK)     :%d/%d\n"
        "Config              :%d\n"
        "Underload           :%d\n"
        "Discard             :%d\n"
        "UndispFrame(Q/DQ)   :%d/%d\n"
        "*FieldUnmatchCnt    :%d\n",
        pstBuffer->stRecord.u32TryQueueFrame,pstBuffer->stRecord.u32QueueFrame,
        pstBuffer->stRecord.u32Release, pstBuffer->stRecord.u32Release,
        pstBuffer->stRecord.u32Config,
        pstProcInfo->u32UnderLoad,
        pstBuffer->stRecord.u32Disacard,
        pstProcInfo->u32ULSIn,pstProcInfo->u32ULSOut,
        pstProcInfo->u32TBNotMatchCount
    );

    {
        mt_s32 EmpNum, WrtNum, ToDispNum, DispNum, DispedNum;

        EmpNum    = 0;
        WrtNum    = 0;
        ToDispNum = 0;
        DispNum   = 0;
        DispedNum = 0;

        for (i=0; i<pstProcInfo->stBufState.u32Number; i++)
        {
            switch(pstProcInfo->stBufState.stNode[i].u32State)
            {
                case 0:
                case 1:
                    EmpNum++;
                    break;
                case 2:
                    WrtNum++;
                    break;
                case 3:
                    ToDispNum++;
                    break;
                case 4:
                    DispNum++;
                    break;
                case 5:
                    DispedNum++;
                    break;
            }
        }

        PROC_PRINT(p,
            "--------------------------------------------------------------------\n"
            "BufferQueue[state, FrameID]\n"
            "(State: 1,Empty[%d]; 2,Write[%d]; 3,ToDisp[%d]; 4,Disp[%d]; 5,Disped[%d])\n",
            EmpNum, WrtNum, ToDispNum, DispNum, DispedNum
        );
    }

    for (i=0; i<pstBuffer->u32Number;)
    {
        PROC_PRINT(p,"[%d,0x%x] ",
            pstBuffer->stNode[i].u32State,
            pstBuffer->stNode[i].u32FrameIndex
            );

        i++;
        if( (i%4) == 0)
        {
            PROC_PRINT(p, "\n");
        }
    }

    PROC_PRINT(p, "\n");

_ERR_EXIT_:
    if (pstProcInfo)
    {
        DISP_FREE(pstProcInfo);
    }
    up(&g_VoMutex);
    return nRet;
}

static mt_s32 WinProcParsePara(mt_char *pProcPara,mt_char **ppArg1,mt_char **ppArg2)
{
    mt_char *pChar = MT_NULL;

    if (strlen(pProcPara) == 0)
    {
        /* not fined arg1 and arg2, return failed */
        *ppArg1  = MT_NULL;
        *ppArg2  = MT_NULL;
        return MT_FAILURE;
    }

    /* find arg1 */
    pChar = pProcPara;
    while( (*pChar == ' ') && (*pChar != '\0') )
    {
        pChar++;
    }

    if (*pChar != '\0')
    {
        *ppArg1 = pChar;
    }
    else
    {
        *ppArg1  = MT_NULL;

        return MT_FAILURE;
    }

    /* ignor arg1 */
    while( (*pChar != ' ') && (*pChar != '\0') )
    {
        pChar++;
    }

    /* Not find arg2, return */
    if (*pChar == '\0')
    {
        *ppArg2 = MT_NULL;

        return MT_SUCCESS;
    }

    /* add '\0' for arg1 */
    *pChar = '\0';

    /* start to find arg2 */
    pChar = pChar + 1;
    while( (*pChar == ' ') && (*pChar != '\0') )
    {
        pChar++;
    }

    if (*pChar != '\0')
    {
        *ppArg2 = pChar;
    }
    else
    {
        *ppArg2 = MT_NULL;
    }

    return MT_SUCCESS;
}

mt_void WIN_ProcWriteHelp(mt_void)
{

}

#ifndef MT_ADVCA_FUNCTION_RELEASE
mt_s32 WinProcCmdProcee(mt_handle hWin, mt_char *pArg1,mt_char *pArg2)
{
    mt_s32 nRet = MT_SUCCESS;

    if (0 == mt_osal_strncmp(pArg1, "pause", strlen("pause")))
    {
        if (0 == mt_osal_strncmp(pArg2, "on", strlen("on")))
        {
            nRet = WIN_Pause(hWin, MT_TRUE);
        }
        else if (0 == mt_osal_strncmp(pArg2, "off", strlen("off")))
        {
            nRet = WIN_Pause(hWin, MT_FALSE);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "reset", strlen("reset")))
    {
        if (0 == mt_osal_strncmp(pArg2, "black", strlen("black")))
        {
            DISP_DEBUGK("[%s]line %d\n",__FUNCTION__,__LINE__);
            nRet = WIN_Reset(hWin, MT_DRV_WIN_SWITCH_BLACK);
        }
        else if (0 == mt_osal_strncmp(pArg2, "still", strlen("still")))
        {
            DISP_DEBUGK("[%s]line %d\n",__FUNCTION__,__LINE__);
            nRet = WIN_Reset(hWin, MT_DRV_WIN_SWITCH_LAST);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "freeze", strlen("freeze")))
    {
        if (0 == mt_osal_strncmp(pArg2, "black", strlen("black")))
        {
            nRet = WIN_Freeze(hWin, MT_TRUE, MT_DRV_WIN_SWITCH_BLACK);
        }
        else if (0 == mt_osal_strncmp(pArg2, "still", strlen("still")))
        {
            nRet = WIN_Freeze(hWin, MT_TRUE, MT_DRV_WIN_SWITCH_LAST);
        }
        else if (0 == mt_osal_strncmp(pArg2, "off", strlen("off")))
        {
            nRet = WIN_Freeze(hWin, MT_FALSE, MT_DRV_WIN_SWITCH_BLACK);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "order", strlen("order")))
    {
        if (0 == mt_osal_strncmp(pArg2, "up", strlen("up")))
        {
            nRet = WIN_SetZorder(hWin, MT_DRV_DISP_ZORDER_MOVEUP);
        }
        else if (0 == mt_osal_strncmp(pArg2, "down", strlen("down")))
        {
            nRet = WIN_SetZorder(hWin, MT_DRV_DISP_ZORDER_MOVEDOWN);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "quick", strlen("quick")))
    {
        if (0 == mt_osal_strncmp(pArg2, "on", strlen("on")))
        {
            nRet = WIN_SetQuick(hWin, MT_TRUE);
        }
        else if (0 == mt_osal_strncmp(pArg2, "off", strlen("off")))
        {
            nRet = WIN_SetQuick(hWin, MT_FALSE);
        }
        else
        {
            WIN_ProcWriteHelp();
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "rota", strlen("rota")))
    {

        MT_ERR_WIN("Not support set rotation now\n");
        return MT_SUCCESS;

#if 0
        if (0 == mt_osal_strncmp(pArg2, "0", strlen("0")))
        {
            nRet = WIN_SetRotation(hWin, MT_DRV_ROT_ANGLE_0);
        }
        else if (0 == mt_osal_strncmp(pArg2, "90", strlen("90")))
        {
            nRet = WIN_SetRotation(hWin, MT_DRV_ROT_ANGLE_90);
        }
        else if (0 == mt_osal_strncmp(pArg2, "180", strlen("180")))
        {
            nRet = WIN_SetRotation(hWin, MT_DRV_ROT_ANGLE_180);
        }
        else if (0 == mt_osal_strncmp(pArg2, "270", strlen("270")))
        {
            nRet = WIN_SetRotation(hWin, MT_DRV_ROT_ANGLE_270);
        }
        else
        {
            return MT_FAILURE;
        }
#endif
    }
    else if (0 == mt_osal_strncmp(pArg1, "flip", strlen("flip")))
    {
        MT_ERR_WIN("Not support set flip now\n");
        return MT_SUCCESS;

#if 0
        MT_BOOL bHoriFlip, bVertFlip;

        nRet = WIN_GetFlip(hWin, &bHoriFlip, &bVertFlip);
        if (nRet != MT_SUCCESS)
        {
            MT_ERR_WIN("WIN_GetFlip failed\n");
            return MT_SUCCESS;
        }

        if (0 == mt_osal_strncmp(pArg2, "hori", strlen("hori")))
        {
            nRet = WIN_SetFlip(hWin, MT_TRUE, bVertFlip);
        }
        else if (0 == mt_osal_strncmp(pArg2, "vert", strlen("vert")))
        {
            nRet = WIN_SetFlip(hWin, bHoriFlip, MT_TRUE);
        }
        else
        {
            return MT_FAILURE;
        }
#endif
    }
    else if (0 == mt_osal_strncmp(pArg1, "StereoDepth", strlen("StereoDepth")))
    {
#if 0
        mt_s32 s32Depth = 0;
        MT_ERR_WIN("Not support set 3d-depth now\n");


#define WIN_3D_VIDEO_MAX_DEPTH 50
#define WIN_3D_VIDEO_MIN_DEPTH -50

        s32Depth = simple_strtol(pArg2, NULL, 10);
        if ( (s32Depth > WIN_3D_VIDEO_MAX_DEPTH) || (s32Depth < WIN_3D_VIDEO_MIN_DEPTH))
        {
            MT_ERR_WIN("invalid 3d-depth value=%d, it must be in [-50, 50]\n", s32Depth);
            return MT_SUCCESS;
        }
#endif

        return MT_SUCCESS;

    }
    else if (0 == mt_osal_strncmp(pArg1, "capture", strlen("capture")))
    {
        mt_uchar *pChar = pArg2;
        MT_DRV_VIDEO_FRAME_S *pstCurFrame;
        mt_u32 pathlength = 0;


        pstCurFrame = (MT_DRV_VIDEO_FRAME_S*)DISP_MALLOC(sizeof(MT_DRV_VIDEO_FRAME_S));
        if (!pstCurFrame)
        {
            MT_ERR_WIN("alloc frame info memory failed\n");
            return nRet;
        }

        /* get currently displayed frame  */
        nRet = WinGetCurrentImg(hWin, pstCurFrame);
        if (nRet != MT_SUCCESS)
        {
            DISP_FREE(pstCurFrame);
            MT_ERR_WIN("call WinGetCurrentImg failed\n");
            return nRet;
        }

        /* calculate char nubmer of path string */
        while( (*pChar != ' ') && (*pChar != '\0') )
        {
            pChar++;
            pathlength++;
        }
        pathlength++;

        /* save yuv frame */
        nRet = vdp_DebugSaveYUVImg(pstCurFrame, pArg2, pathlength);

        DISP_FREE(pstCurFrame);
    }
    else if (0 == mt_osal_strncmp(pArg1, "rd_door", strlen("rd_door")))
    {
        if (0 == mt_osal_strncmp(pArg2, "start", strlen("start")))
        {
            g_bWinPrivateInformation = MT_TRUE;
        }
        else
        {
            g_bWinPrivateInformation = MT_FALSE;
        }
    }
    else if (0 == mt_osal_strncmp(pArg1, "sr_post_dci_test", strlen("sr_post_dci_test")))
    {
        WIN_Test(hWin);
    }
    else
    {
        return MT_FAILURE;
    }

    return nRet;
}
#endif

mt_char u8WinProcBuffer[256];
mt_s32 DRV_WIN_ProcWrite(struct file * file,
                   const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file *p = file->private_data;
    mt_proc_entry_t *pProcItem = p->private;
    mt_char *pArg1, *pArg2;
    mt_handle hWin;
    mt_s32 nRet;

    hWin = (mt_handle)(pProcItem->data);

    if(count >= sizeof(u8WinProcBuffer))
    {
        MT_ERR_WIN("your parameter string is too long!\n");
        return -EFAULT;
    }

    nRet = down_interruptible(&g_VoMutex);

    memset(u8WinProcBuffer, 0, sizeof(u8WinProcBuffer));
    if (copy_from_user(u8WinProcBuffer, buf, count))
    {
        up(&g_VoMutex);
        MT_ERR_WIN("MMZ: copy_from_user failed!\n");
        return -EFAULT;
    }
    u8WinProcBuffer[count] = 0;

    nRet = WinProcParsePara(u8WinProcBuffer, &pArg1, &pArg2);
    if(  (nRet != MT_SUCCESS)
        ||(0 == mt_osal_strncmp(pArg1, "help", strlen("help")))
        ||(pArg2 == MT_NULL) )
    {
        up(&g_VoMutex);
        MT_ERR_WIN("Invalid echo commond!\n");
        WIN_ProcWriteHelp();
        return -EFAULT;
    }

#ifndef MT_ADVCA_FUNCTION_RELEASE
    nRet = WinProcCmdProcee(hWin, pArg1, pArg2);
    if (nRet != MT_SUCCESS)
    {
        WIN_ProcWriteHelp();
    }
#endif

    up(&g_VoMutex);

    return count;
}

/***************************************************************/
mt_s32 WIN_AddToProc(mt_handle hWindow)
{
    mt_proc_entry_t  *pProcItem;
    mt_char           ProcName[12];
    mt_u32 u32Index;
    mt_s32 Ret;

    Ret = WinGetProcIndex(hWindow, &u32Index);
    if (Ret)
    {
        MT_ERR_WIN("WinGetProcInfo failed!\n");
        return MT_ERR_VO_ADD_PROC_ERR;
    }

    //printk("WIN_AddToProc win index=0x%x\n", u32Index);
    mt_osal_snprintf(ProcName, 12, "win%04x", (mt_u32)(u32Index & WINDOW_INDEX_MASK));

    pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);// MT_DRV_PROC_AddModule
    if (!pProcItem)
    {
        MT_ERR_WIN("Window add proc failed!\n");
        MT_ERR_WIN("WinGetProcInfo failed!\n");
        return MT_ERR_VO_ADD_PROC_ERR;
    }

    //s_WinProcId[WinGetDispId(u32Index)][WinGetId(u32Index)] = u32Index;
    pProcItem->data  = (mt_void *)hWindow;
    pProcItem->read  = DRV_WIN_ProcRead;
    pProcItem->write = DRV_WIN_ProcWrite;

    return MT_SUCCESS;
}

mt_s32 WIN_RemFromProc(mt_handle hWindow)
{
    mt_char           ProcName[12];
    mt_u32 u32Index;
    mt_s32 Ret;

    Ret = WinGetProcIndex(hWindow, &u32Index);
    //printk("WIN_RemFromProc win index=0x%x\n", u32Index);
    if (!Ret)
    {
        mt_osal_snprintf(ProcName, 12 ,"win%04x", (mt_u32)(u32Index & WINDOW_INDEX_MASK));
        mt_drv_proc_rm_module(ProcName);//MT_DRV_PROC_RemoveModule

        //s_WinProcId[WinGetDispId(u32Index)][WinGetId(u32Index)] = WINDOW_INVALID_ID;
    }
    else
    {
        MT_FATAL_WIN("WinGetProcInfo failed!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 WIN_AddToState(WIN_STATE_S *pst2WinState, MT_BOOL bVirtual, MT_DRV_DISPLAY_E enDisp, mt_handle hWin)
{
    mt_s32 i;

    if (bVirtual == MT_TRUE)
    {
        for(i=0; i<DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP; i++)
        {
            if (pst2WinState->hVirtualWin[i] == MT_NULL)
            {
                pst2WinState->hVirtualWin[i] = hWin;
                return MT_SUCCESS;
            }
        }
    }
    else
    {
        if (enDisp >= MT_DRV_DISPLAY_BUTT)
        {
            return MT_SUCCESS;
        }

        for(i=0; i<DEF_MAX_WIN_NUM_ON_SINGLE_DISP; i++)
        {
            if (pst2WinState->hWin[enDisp][i] == MT_NULL)
            {
                pst2WinState->hWin[enDisp][i] = hWin;
                return MT_SUCCESS;
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 WIN_RemFromState(WIN_STATE_S *pst2WinState,MT_BOOL bVirtual, mt_handle hWin)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 i;

    if (bVirtual)
    {
        for(i=0; i<DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP; i++)
        {
            if (pst2WinState->hVirtualWin[i] == hWin)
            {
                pst2WinState->hVirtualWin[i] = MT_NULL;
                return MT_SUCCESS;
            }
        }
    }
    else
    {
        for(enDisp=0; enDisp<MT_DRV_DISPLAY_BUTT; enDisp++)
        {
            for(i=0; i<DEF_MAX_WIN_NUM_ON_SINGLE_DISP; i++)
            {
                if (pst2WinState->hWin[enDisp][i] == hWin)
                {
                    pst2WinState->hWin[enDisp][i] = MT_NULL;
                    return MT_SUCCESS;
                }
            }
        }
    }

    return MT_SUCCESS;
}

typedef enum {
    VDP_MCE_STAGE = 0, /*mce stage*/
    VDP_NORMAL_STAGE,  /*kernel has boot already.*/
}VDP_MCE_STAGE_E;

VDP_MCE_STAGE_E sftware_stage = 0;

mt_s32 DRV_WIN_SetSftwareStage(mt_void)
{
    sftware_stage = VDP_NORMAL_STAGE;
    return MT_SUCCESS;
}

mt_s32 DRV_WIN_GetSftwareStage(mt_void)
{
    return sftware_stage;
}

mt_s32 WIN_CreateExt(WIN_CREATE_S *pVoWinCreate, WIN_STATE_S *pst2WinState)
{
    MT_DRV_WIN_INFO_S stWinInfo;
    mt_s32 Ret;
    WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);
    Ret = WIN_Create(&pVoWinCreate->WinAttr, &pVoWinCreate->hWindow, pVoWinCreate->bVirtScreen);

    if (Ret)
    {
        goto __ERR_EXIT__;
    }
    WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);

    if (DRV_WIN_GetSftwareStage())
    {
        // add proc
        Ret = WIN_AddToProc(pVoWinCreate->hWindow);
        if (Ret)
            goto __ERR_EXIT_DESTROY__;

        if (pVoWinCreate->WinAttr.bVirtual != MT_TRUE)
        {
            if (MT_SUCCESS == WIN_GetInfo(pVoWinCreate->hWindow, &stWinInfo))
            {
                if (stWinInfo.hSec)
                {
                    Ret = WIN_AddToProc(stWinInfo.hSec);
                    if (Ret)
                        goto __ERR_EXIT_REM_PORC__;
                }
            }
        }
    }

    WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);

    WIN_AddToState(pst2WinState, pVoWinCreate->WinAttr.bVirtual, pVoWinCreate->WinAttr.enDisp, pVoWinCreate->hWindow);
    return MT_SUCCESS;

__ERR_EXIT_REM_PORC__:
    WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);

    Ret = WIN_RemFromProc(pVoWinCreate->hWindow);

__ERR_EXIT_DESTROY__:
     WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);

    Ret = WIN_Destroy(pVoWinCreate->hWindow);

__ERR_EXIT__:
     WIN_DEBUGK("%s, %d\n",__FUNCTION__,__LINE__);

    return Ret;
}

mt_s32 WIN_DestroyExt(mt_handle hWindow, WIN_STATE_S *pstWinState)
{
    MT_DRV_WIN_INFO_S stWinInfo;
    mt_s32 Ret;
    WIN_DEBUGK("%s %d\n",__FUNCTION__,__LINE__);

    if (MT_SUCCESS != WIN_GetInfo(hWindow, &stWinInfo))
    {
        MT_ERR_WIN("Window dose not exist!\n");
        return MT_ERR_VO_WIN_NOT_EXIST;
    }

    if (stWinInfo.eType == MT_DRV_WIN_VITUAL_SINGLE)
    {
        // if virtual window, remove proc and destory.
        Ret = WIN_RemFromProc(hWindow);;
        if (Ret)
        {
            MT_ERR_WIN("WIN_RemFromProc failed!\n");;
        }

        Ret = WIN_Destroy(hWindow);
        if (Ret != MT_SUCCESS)
        {
            MT_FATAL_WIN("call WIN_Destroy failed.\n");
        }

        WIN_RemFromState(pstWinState, MT_TRUE, hWindow);
    }
    else
    {
        if (stWinInfo.hSec)
        {
            // if have slave window, remove slave window proc firstly
            Ret = WIN_RemFromProc(stWinInfo.hSec);
            if (Ret)
            {
                MT_ERR_WIN("WIN_RemFromProc slave failed!\n");
            }
        }

        // remove proc and destory window
        Ret = WIN_RemFromProc(hWindow);;
        if (Ret)
        {
            MT_ERR_WIN("WIN_RemFromProc failed!\n");;
        }

        Ret = WIN_Destroy(hWindow);
        if (Ret != MT_SUCCESS)
        {
            MT_FATAL_WIN("call WIN_Destroy failed.\n");
        }

        WIN_RemFromState(pstWinState, MT_FALSE, hWindow);
    }

    return Ret;
}

mt_s32 WIN_CheckHanlde(mt_handle hWindow, WIN_STATE_S *pstWinState)
{
    return MT_SUCCESS;
}

extern void MT_DRV_VDEC_Pause(mt_u32 Paused);
mt_s32 WIN_ProcessCmd(unsigned int cmd, mt_void *arg, WIN_STATE_S *pstWinState)
{
    mt_s32 Ret = MT_SUCCESS;
    //WIN_DEBUGK("%s %d  cmd %x\n",__FUNCTION__, __LINE__, cmd);
    switch (cmd)
    {
        case CMD_WIN_CREATE:
        {
            WIN_CREATE_S  *pVoWinCreate;

            pVoWinCreate = (WIN_CREATE_S *)arg;

            Ret = WIN_CreateExt(pVoWinCreate, pstWinState);

            WIN_DEBUGK("creat window ext \n");

            break;
        }

        case CMD_WIN_DESTROY:
        {
            Ret = WIN_CheckHanlde(*((mt_handle *)arg), pstWinState);

            if (MT_SUCCESS == Ret)
            {
                MT_BOOL bSinkAttached = MT_TRUE;
                MT_BOOL bSrcAttached = MT_TRUE;
                mt_handle hWindow = MT_INVALID_HANDLE;
                hWindow = *((mt_handle *)arg);
                Ret = WIN_CheckAttachState(hWindow,&bSrcAttached,&bSinkAttached);
                if (MT_SUCCESS != Ret)
                {
                    WIN_ERROR("Can't Get window's attachState\n");
                    return MT_ERR_VO_INVALID_OPT;
                }
                if (bSrcAttached == MT_TRUE || bSinkAttached == MT_TRUE)
                {
                    WIN_ERROR("Window is still attached,Can't be destoryed."
                              "bSrcAttached %d,bSinkAttached %d\n",bSrcAttached,bSinkAttached);
                    return MT_ERR_VO_INVALID_OPT;
                }
                Ret = WIN_DestroyExt(*((mt_handle *)arg), pstWinState);
            }

            break;
        }

        case CMD_WIN_SET_ENABLE:
        {
            WIN_ENABLE_S   *pWinEnable;

            pWinEnable = (WIN_ENABLE_S *)arg;

            Ret = WIN_CheckHanlde(pWinEnable->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
            	MT_DRV_VDEC_Pause(pWinEnable->bEnable);
                Ret = WIN_SetEnable(pWinEnable->hWindow, pWinEnable->bEnable);
            }
            WIN_DEBUGK("set enable window \n");

            break;
        }

        case CMD_WIN_GET_ENABLE:
        {
            WIN_ENABLE_S   *pVoWinEnable;

            pVoWinEnable = (WIN_ENABLE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinEnable->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetEnable(pVoWinEnable->hWindow, &pVoWinEnable->bEnable);
            }

            break;
        }

        case CMD_WIN_VIR_ACQUIRE:
        {
            WIN_FRAME_S   *pVoWinFrame;
            pVoWinFrame = (WIN_FRAME_S*)arg;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_AcquireFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
            }

            break;
        }

        case CMD_WIN_VIR_RELEASE:
        {
            WIN_FRAME_S   *pVoWinFrame;
            pVoWinFrame = (WIN_FRAME_S*)arg;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_ReleaseFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
            }

            break;
        }

        case CMD_WIN_SET_ATTR:
        {
            WIN_CREATE_S *pVoWinAttr;

            pVoWinAttr = (WIN_CREATE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinAttr->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetAttr(pVoWinAttr->hWindow, &pVoWinAttr->WinAttr);
            }
            WIN_DEBUGK("set attr window \n");


            break;
        }

        case CMD_WIN_GET_ATTR:
        {
            WIN_CREATE_S   *pVoWinAttr;

            pVoWinAttr = (WIN_CREATE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinAttr->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetAttr(pVoWinAttr->hWindow, &pVoWinAttr->WinAttr);
            }

            break;
        }

        case CMD_WIN_SET_ZORDER:
        {
            WIN_ZORDER_S *pVoWinZorder;

            pVoWinZorder = (WIN_ZORDER_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinZorder->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetZorder(pVoWinZorder->hWindow, pVoWinZorder->eZFlag);
            }

            break;
        }

        case CMD_WIN_GET_ORDER:
        {
            WIN_ORDER_S *pVoWinOrder;

            pVoWinOrder = (WIN_ORDER_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinOrder->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetZorder(pVoWinOrder->hWindow, &pVoWinOrder->Order);
            }

            break;
        }

        case CMD_WIN_SET_SOURCE:
        {
            WIN_SOURCE_S *pVoWinAttach;
            MT_DRV_WIN_SRC_INFO_S stWinSrc;
            pVoWinAttach = (WIN_SOURCE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinAttach->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetSource(pVoWinAttach->hWindow,&stWinSrc);
                if (MT_SUCCESS == Ret)
                {
                    if (stWinSrc.hSrc != pVoWinAttach->stSrc.hSrc)
                    {
                        Ret = WIN_SetSource(pVoWinAttach->hWindow,
                                            &pVoWinAttach->stSrc);
                    }
                    else
                    {
                        Ret = MT_ERR_VO_INVALID_OPT;
                    }
                }
            }
            WIN_DEBUGK("set source  window \n");

            break;
        }
        case CMD_WIN_GET_SOURCE:
        {
            WIN_SOURCE_S *pVoWinAttach;

            pVoWinAttach = (WIN_SOURCE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinAttach->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetSource(pVoWinAttach->hWindow, &pVoWinAttach->stSrc);
            }

            break;
        }

        case CMD_WIN_FREEZE:
        {
            WIN_FREEZE_S  *pVoWinFreeze;

            pVoWinFreeze = (WIN_FREEZE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinFreeze->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_Freeze(pVoWinFreeze->hWindow, pVoWinFreeze->bEnable, pVoWinFreeze->eMode);
            }

            break;
        }

        case CMD_WIN_GET_FREEZE_STATUS:
        {
            WIN_FREEZE_S  *pVoWinFreeze;

            pVoWinFreeze = (WIN_FREEZE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinFreeze->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetFreezeStatus(pVoWinFreeze->hWindow, &pVoWinFreeze->bEnable, &pVoWinFreeze->eMode);
            }

            break;
        }
        case CMD_WIN_SEND_FRAME:
        {
            WIN_FRAME_S   *pVoWinFrame;

            pVoWinFrame = (WIN_FRAME_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SendFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
            }
            WIN_DEBUGK("send frame  window \n");

            break;
        }

        case CMD_WIN_QU_FRAME:
        {
			//WIN_FRAME_S   *pVoWinFrame;
            WIN_CMD_FRAME_S   *pVoWinFrame;
            MT_DRV_VIDEO_FRAME_S *pstFrame;
			//pVoWinFrame = (WIN_FRAME_S *)arg;
			pVoWinFrame  = (WIN_CMD_FRAME_S *)arg;
			pstFrame = pVoWinFrame->pstFrame;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
				MT_DRV_VIDEO_FRAME_S stFrame;
				unsigned long res;

				res = copy_from_user((void*)&stFrame, (void __user*)pstFrame, sizeof(MT_DRV_VIDEO_FRAME_S));
				if (res != 0)
				{
					WIN_DEBUGK("copy from user frame failed!\n");
					BUG();
				}

				pstFrame = &stFrame;
#endif
				//Ret = WIN_QueueFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
                Ret = WIN_QueueFrame(pVoWinFrame->hWindow, pstFrame);
            }

            //printk("qu frame  window \n");

            break;
        }

        case CMD_WIN_QU_ULSFRAME:
        {
            WIN_FRAME_S   *pVoWinFrame;

            pVoWinFrame = (WIN_FRAME_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_QueueUselessFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
            }
            WIN_DEBUGK("qu useless frame  window \n");

            break;
        }

        case CMD_WIN_DQ_FRAME:
        {
            WIN_FRAME_S     *pVoWinFrame;

            pVoWinFrame = (WIN_FRAME_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinFrame->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_DequeueFrame(pVoWinFrame->hWindow, &pVoWinFrame->stFrame);
            }
            WIN_DEBUGK("dq frame  window \n");


            break;
        }

        case CMD_WIN_RESET:
        {
            WIN_RESET_S  *pVoWinReset;

            pVoWinReset = (WIN_RESET_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinReset->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                DISP_DEBUGK("[%s]line %d\n",__FUNCTION__,__LINE__);
                Ret = WIN_Reset(pVoWinReset->hWindow, pVoWinReset->eMode);
            }

            break;
        }

        case CMD_WIN_PAUSE:
        {
            WIN_PAUSE_S  *pVoWinPause;

            pVoWinPause = (WIN_PAUSE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinPause->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_Pause(pVoWinPause->hWindow, pVoWinPause->bEnable);
            }

            break;
        }

        case CMD_WIN_GET_PLAY_INFO:
        {
            WIN_PLAY_INFO_S  *pVoWinDelay;

            pVoWinDelay = (WIN_PLAY_INFO_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinDelay->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetPlayInfo(pVoWinDelay->hWindow, &pVoWinDelay->stPlayInfo);
            }

            break;
        }

        case CMD_WIN_GET_INFO:
        {
            WIN_PRIV_INFO_S  *pVoWinDelay;

            pVoWinDelay = (WIN_PRIV_INFO_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinDelay->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetInfo(pVoWinDelay->hWindow, &pVoWinDelay->stPrivInfo);
            }

            break;
        }


        case CMD_WIN_STEP_MODE:
        {
            WIN_STEP_MODE_S  *pVoWinStepMode;

            pVoWinStepMode = (WIN_STEP_MODE_S *)arg;

            Ret = WIN_CheckHanlde(pVoWinStepMode->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetStepMode(pVoWinStepMode->hWindow, pVoWinStepMode->bStep);
            }
            WIN_DEBUGK("step mode  window \n");


            break;
        }

        case CMD_WIN_STEP_PLAY:
        {
            Ret = WIN_CheckHanlde(*((mt_handle *)arg), pstWinState);

            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetStepPlay(*((mt_handle *)arg));
            }
            WIN_DEBUGK("step play  window \n");

        break;
        }

        case CMD_WIN_VIR_EXTERNBUF:
        {
            WIN_BUF_POOL_S*      winBufAttr = (WIN_BUF_POOL_S*)arg;

            Ret = WIN_CheckHanlde(winBufAttr->hwin, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetExtBuffer(winBufAttr->hwin, &winBufAttr->stBufPool);
            }
            break;
        }

        case CMD_WIN_SET_QUICK:
        {
            WIN_SET_QUICK_S * stQuickOutputAttr = (WIN_SET_QUICK_S*)arg;
            Ret = WIN_CheckHanlde(stQuickOutputAttr->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetQuick(stQuickOutputAttr->hWindow, stQuickOutputAttr->bQuickEnable);
            }

            break;
        }
        case CMD_WIN_GET_QUICK:
        {
            WIN_SET_QUICK_S * pstQuickOutputAttr = (WIN_SET_QUICK_S*)arg;
            Ret = WIN_CheckHanlde(pstQuickOutputAttr->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetQuick(pstQuickOutputAttr->hWindow, &pstQuickOutputAttr->bQuickEnable);
            }

            break;
        }

        case CMD_WIN_SUSPEND:
        {
            Ret = WIN_Suspend();
            break;
        }

        case CMD_WIN_RESUM:
        {
            Ret = WIN_Resume();
            break;
        }

        case CMD_WIN_GET_HANDLE:
        {
            WIN_HANDLE_ARRAY_S stWinArray;
            WIN_GET_HANDLE_S *pWin = (WIN_GET_HANDLE_S*)arg;

            Ret = Win_DebugGetHandle(pWin->enDisp, &stWinArray);
            if (Ret == MT_SUCCESS)
            {
                mt_s32 i;

                pWin->u32WinNumber = stWinArray.u32WinNumber;

                for (i=0; (i<pWin->u32WinNumber) && (i<DEF_MAX_WIN_NUM_ON_SINGLE_DISP); i++)
                {
                    pWin->ahWinHandle[i] = stWinArray.ahWinHandle[i];
                }
            }
            break;
        }

        case CMD_WIN_ATTACH:
        {
            WIN_ATTACH_S *pstAttach;
            pstAttach = (WIN_ATTACH_S *)arg;

            if (pstAttach->enType == ATTACH_TYPE_SINK)
            {
                Ret = WIN_AttachSink(pstAttach->hWindow, pstAttach->hMutual);
            }
            else
            {
                Ret = MT_FAILURE;
            }
            WIN_DEBUGK("attatch  window \n");

            break;
        }

        case CMD_WIN_DETACH:
        {
            WIN_ATTACH_S *pstAttach;
            pstAttach = (WIN_ATTACH_S *)arg;

            if (pstAttach->enType == ATTACH_TYPE_SINK)
            {
                Ret = WIN_DetachSink(pstAttach->hWindow, pstAttach->hMutual);
            }
            else
            {
                Ret = MT_FAILURE;
            }
            break;
        }

        case CMD_WIN_GET_INTF:
        {
            WIN_INTF_S *pstWinIntf;
            pstWinIntf = (WIN_INTF_S *)arg;

            pstWinIntf->pfAcqFrame = MT_NULL;
            pstWinIntf->pfRlsFrame = WIN_ReleaseFrame;
            pstWinIntf->pfSetWinAttr = WIN_SetVirtualAttr;
            Ret = MT_SUCCESS;

            break;
        }
        case CMD_WIN_GET_LATESTFRAME_INFO:
        {
            WIN_FRAME_S *frame_info;
            frame_info = (WIN_FRAME_S *)arg;
            Ret = WinGetCurrentImg(frame_info->hWindow, &frame_info->stFrame);
            WIN_DEBUGK("get lastframe info  window \n");

            break;
        }
        case CMD_VO_WIN_CAPTURE_START:
        {
            WIN_CAPTURE_S *capture_info = MT_NULL;
            MT_DRV_DISPLAY_E enDisp;
            mt_u32 u32WinIndex;

            capture_info = (WIN_CAPTURE_S *)arg;

            (mt_void)WinGetIndex(capture_info->hWindow, &enDisp, &u32WinIndex);
            pstWinState->hCapture[enDisp][u32WinIndex] = capture_info->hWindow;


            Ret = WinCapturePause(capture_info->hWindow, 1);
            if (MT_SUCCESS != Ret) {
                pstWinState->hCapture[enDisp][u32WinIndex] = 0;
                break;
            }

            Ret = WinCaptureFrame(capture_info->hWindow,
                                    &capture_info->CapPicture,
                                    &capture_info->driver_supply_addr.startPhyAddr,
                                    &capture_info->driver_supply_addr.length);

            if (MT_SUCCESS != Ret) {
                pstWinState->hCapture[enDisp][u32WinIndex] = 0;
                WinCapturePause(capture_info->hWindow, 0);
            }

            break;
        }
        case CMD_VO_WIN_CAPTURE_RELEASE:
        {
            WIN_CAPTURE_S *capture_info = MT_NULL;
            MT_DRV_DISPLAY_E enDisp;
            mt_u32 u32WinIndex;

            capture_info = (WIN_CAPTURE_S *)arg;
            (mt_void)WinGetIndex(capture_info->hWindow, &enDisp, &u32WinIndex);

            Ret = WinReleaseCaptureFrame(capture_info->hWindow, &capture_info->CapPicture);
            if (MT_SUCCESS != Ret)
                pstWinState->hCapture[enDisp][u32WinIndex] = 0;

            Ret = WinCapturePause(capture_info->hWindow, 0);
            if (MT_SUCCESS != Ret)
                pstWinState->hCapture[enDisp][u32WinIndex] = 0;

            break;
        }
        case CMD_VO_WIN_CAPTURE_FREE:
        {
            WIN_CAPTURE_S *capture_info = MT_NULL;
            MT_DRV_DISPLAY_E enDisp;
            mt_u32 u32WinIndex;

            capture_info = (WIN_CAPTURE_S *)arg;

            (mt_void)WinGetIndex(capture_info->hWindow, &enDisp, &u32WinIndex);
            pstWinState->hCapture[enDisp][u32WinIndex] = 0;

            Ret = WinFreeCaptureMMZBuf(capture_info->hWindow, &capture_info->CapPicture);
            break;
        }
        case CMD_WIN_SET_ROTATION:
        {
            WIN_ROTATION_S *pVoWinRotation;
            pVoWinRotation = (WIN_ROTATION_S *)arg;
            Ret = WIN_CheckHanlde(pVoWinRotation->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetRotation(pVoWinRotation->hWindow, pVoWinRotation->enRotation);
            }
            break;
        }
        case CMD_WIN_GET_ROTATION:
        {
            WIN_ROTATION_S *pVoWinRotation;
            pVoWinRotation = (WIN_ROTATION_S *)arg;
            Ret = WIN_CheckHanlde(pVoWinRotation->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetRotation(pVoWinRotation->hWindow, &(pVoWinRotation->enRotation));
            }
            break;
        }
        case CMD_WIN_SET_FLIP:
        {
            WIN_FLIP_S *pVoWinFlip;

            pVoWinFlip = (WIN_FLIP_S *)arg;
            Ret = WIN_CheckHanlde(pVoWinFlip->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_SetFlip(pVoWinFlip->hWindow, pVoWinFlip->bHoriFlip,pVoWinFlip->bVertFlip);
            }
            break;
        }
        case CMD_WIN_GET_FLIP:
        {
            WIN_FLIP_S *pVoWinFlip;
            pVoWinFlip = (WIN_FLIP_S *)arg;
            Ret = WIN_CheckHanlde(pVoWinFlip->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetFlip(pVoWinFlip->hWindow,
                                    &(pVoWinFlip->bHoriFlip),
                                    &(pVoWinFlip->bVertFlip));
            }
            break;
        }
        case CMD_WIN_GET_UNLOAD:
        {
            WIN_UNLOAD_S *pWinUnload;
            pWinUnload = (WIN_UNLOAD_S *)arg;
            Ret = WIN_CheckHanlde(pWinUnload->hWindow, pstWinState);
            if (MT_SUCCESS == Ret)
            {
                Ret = WIN_GetUnload(pWinUnload->hWindow,
                                    &(pWinUnload->u32Times));
            }
            break;
        }
        case CMD_WIN_SET_PARA  :
        {
            Ret = WIN_SetPara(arg);
            break;
        }
        case CMD_WIN_GET_PARA  :
        {
            Ret = WIN_GetPara(arg);
            break;
        }
        case CMD_WIN_CLEAN_ALLFRAME  :
        {
            Ret = WIN_ClearAllFrame(arg);
            break;
        }
        default:
        up(&g_VoMutex);
        return -ENOIOCTLCMD;
    }

    return Ret;
}

extern mt_u32 DISP_Get_CountStatus(mt_void);

mt_s32 DRV_WIN_Suspend(basedev_s *pdev, pm_message_t state)
{
    mt_s32  Ret;
    MT_DRV_WIN_PRIV_DATA *win_priv_data = dev_get_platdata(&pdev->dev);

    Ret = down_trylock(&g_VoMutex);
    if (Ret)
    {
        MT_FATAL_WIN("down g_VoMutex failed.\n");
        return -1;
    }

    /* just return if no task opened the vo device.*/
#if 0
    if( !atomic_read(&g_VoCount) )
    {
        up(&g_VoMutex);
        return 0;
    }
#endif

    g_VoSuspend = MT_TRUE;
    WIN_Suspend();

    msleep(50);

    MT_PRINT("VO suspend OK\n");

    up(&g_VoMutex);

    if (!IS_ERR_OR_NULL(win_priv_data->hdclk))
        clk_disable_unprepare(win_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data->hd_os_clk))
        clk_disable_unprepare(win_priv_data->hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_108m))
        clk_disable_unprepare(win_priv_data->sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_27m))
        clk_disable_unprepare(win_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data->vbiclk))
        clk_disable_unprepare(win_priv_data->vbiclk);

    return 0;
}

mt_s32 DRV_WIN_Resume(basedev_s *pdev)
{
    mt_s32  Ret;
    MT_DRV_WIN_PRIV_DATA *win_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(win_priv_data->hdclk))
        clk_prepare_enable(win_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data->hd_os_clk))
        clk_prepare_enable(win_priv_data->hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_108m))
        clk_prepare_enable(win_priv_data->sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_27m))
        clk_prepare_enable(win_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data->vbiclk))
        clk_prepare_enable(win_priv_data->vbiclk);

    Ret = down_trylock(&g_VoMutex);
    if (Ret)
    {
        MT_FATAL_WIN("down g_VoMutex failed.\n");
        return -1;
    }

#if 0
    if(!atomic_read(&g_VoCount))
    {
        up(&g_VoMutex);
        return 0;
    }
#endif

    WIN_Resume();
    g_VoSuspend = MT_FALSE;

    MT_PRINT("VO resume OK\n");

    up(&g_VoMutex);
    return 0;
}

mt_s32 DRV_WIN_Process(mt_u32 cmd, mt_void *arg)
{
    WIN_STATE_S   *pstWinState;
    mt_s32        Ret;

    Ret = down_interruptible(&g_VoMutex);

    pstWinState = &g_VoModState;

    Ret = WIN_ProcessCmd(cmd, arg, pstWinState);

    up(&g_VoMutex);
    return Ret;
}

/*add for mce interface*/
mt_s32  MT_DRV_WIN_Create(MT_DRV_WIN_ATTR_S *pWinAttr, mt_handle *phWindow)
{
    mt_s32 Ret;
    WIN_CREATE_S voWinCreate;

    voWinCreate.hWindow = *phWindow;
    voWinCreate.WinAttr = *pWinAttr;
    Ret = DRV_WIN_Process(CMD_WIN_CREATE, &voWinCreate);
    *phWindow = voWinCreate.hWindow;
    return Ret;
}

mt_s32 MT_DRV_WIN_Destroy(mt_handle hWindow)
{
    mt_s32 Ret;

    Ret = DRV_WIN_Process(CMD_WIN_DESTROY, &hWindow);
    return Ret;
}

mt_s32 MT_DRV_WIN_SetEnable(mt_handle hWindow, MT_BOOL bEnable)
{
    mt_s32 Ret;
    WIN_ENABLE_S   enVoWinEnable;

    enVoWinEnable.bEnable = bEnable;
    enVoWinEnable.hWindow = hWindow;
    Ret = DRV_WIN_Process(CMD_WIN_SET_ENABLE, &enVoWinEnable);
    return Ret;
}

mt_s32 MT_DRV_WIN_GetEnable(mt_handle hWindow, MT_BOOL *pbEnable)
{
    mt_s32 Ret;
    WIN_ENABLE_S   enVoWinEnable;

    memset(&enVoWinEnable, 0, sizeof(WIN_ENABLE_S));
   // enVoWinEnable.bEnable = bEnable;
    enVoWinEnable.hWindow = hWindow;

    Ret = DRV_WIN_Process(CMD_WIN_GET_ENABLE, &enVoWinEnable);
    if(!Ret)
    {
        *pbEnable = enVoWinEnable.bEnable;
    }
    return Ret;
}


mt_s32 MT_DRV_WIN_SetSource(mt_handle hWindow, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    mt_s32 Ret;
    WIN_SOURCE_S VoWinAttach;

    VoWinAttach.hWindow = hWindow;
    VoWinAttach.stSrc   = *pstSrc;


    Ret = DRV_WIN_Process(CMD_WIN_SET_SOURCE, &VoWinAttach);
    return Ret;
}

mt_s32 MT_DRV_WIN_GetSource(mt_handle hWin, MT_DRV_WIN_SRC_INFO_S *pstSrc)
{
    mt_s32 Ret;
    WIN_SOURCE_S   stWinSrc;

    memset(&stWinSrc, 0, sizeof(WIN_SOURCE_S));
    stWinSrc.hWindow = hWin;
    Ret = DRV_WIN_Process(CMD_WIN_GET_SOURCE, &stWinSrc);
    if(!Ret)
    {
        *pstSrc = stWinSrc.stSrc;
    }
    return Ret;
}

mt_s32 MT_DRV_WIN_Reset(mt_handle hWindow, MT_DRV_WIN_SWITCH_E enSwitch)
{
    mt_s32 Ret;
    WIN_RESET_S   VoWinReset;

    VoWinReset.hWindow = hWindow;
    VoWinReset.eMode   = enSwitch;
    Ret = DRV_WIN_Process(CMD_WIN_RESET, &VoWinReset);
    return Ret;
}

mt_s32 MT_DRV_WIN_Pause(mt_handle hWin, MT_BOOL bEnable)
{
    mt_s32 Ret;
    WIN_PAUSE_S   stWinPause;

    stWinPause.hWindow = hWin;
    stWinPause.bEnable = bEnable;
    Ret = DRV_WIN_Process(CMD_WIN_PAUSE, &stWinPause);
    return Ret;
}

mt_s32 MT_DRV_WIN_SetStepMode(mt_handle hWin, MT_BOOL bStepMode)
{
    mt_s32 Ret;
    WIN_STEP_MODE_S   stMode;

    stMode.hWindow = hWin;
    stMode.bStep = bStepMode;
    Ret = DRV_WIN_Process(CMD_WIN_STEP_MODE, &bStepMode);
    return Ret;
}

mt_s32 MT_DRV_WIN_SetStepPlay(mt_handle hWin)
{

    return DRV_WIN_Process(CMD_WIN_STEP_PLAY, &hWin);

}

mt_s32 MT_DRV_WIN_SetExtBuffer(mt_handle hWin, MT_DRV_VIDEO_BUFFER_POOL_S* pstBuf)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_VoMutex);

    Ret  = WIN_SetExtBuffer(hWin, pstBuf);

    up(&g_VoMutex);
    return Ret;
}

mt_s32 MT_DRV_WIN_AcquireFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_VoMutex);

    Ret  = WIN_AcquireFrame(hWin, pFrameinfo);

    up(&g_VoMutex);
    return Ret;
}

mt_s32 MT_DRV_WIN_ReleaseFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_VoMutex);

    Ret  = WIN_ReleaseFrame(hWin, pFrameinfo);

    up(&g_VoMutex);
    return Ret;
}

mt_s32 MT_DRV_WIN_SetQuick(mt_handle hWin, MT_BOOL bEnable)
{
    mt_s32 Ret;
    WIN_SET_QUICK_S   stQuik;

    stQuik.hWindow = hWin;
    stQuik.bQuickEnable = bEnable;
    Ret = DRV_WIN_Process(CMD_WIN_SET_QUICK, &stQuik);
    return Ret;
}

mt_s32 Make_CaptuerFrame(MT_DRV_VIDEO_FRAME_S *pFrameinfo,MT_DRV_VIDEO_FRAME_S *pCapFrameinfo)
{
    mt_u32 datalen = 0, y_stride = 0, height = 0 ;
    phys_addr_t u32StillFrameStartPhyAddr,u32FrameStartPhyAddr;
    ulong u32StillFrameStartVirAddr,u32FrameStartVirAddr;
    mt_s32 nRet = 0;
    DISP_MMZ_BUF_S  stMMZ_Frame;

    if (!pCapFrameinfo)
    {
        MT_ERR_WIN("para null!\n");
        return MT_FAILURE;
    }
    if (!pFrameinfo)
    {
        MT_ERR_WIN("para null!\n");
        return MT_FAILURE;
    }

    /*1:calculate data long*/
    y_stride = pFrameinfo->stBufAddr[0].u32Stride_Y;
    height   = pFrameinfo->u32Height;

    if ( MT_DRV_PIX_FMT_NV21 == pFrameinfo->ePixFormat)
        datalen = height * y_stride * 3 / 2 + height * 4;
    else
        datalen = height * y_stride * 2 + height * 4;

    /*2: creat still frame*/
    u32StillFrameStartPhyAddr = pCapFrameinfo->stBufAddr[0].u32PhyAddr_Y;
    u32FrameStartPhyAddr = pFrameinfo->stBufAddr[0].u32PhyAddr_Y;

    memset(&stMMZ_Frame,0,sizeof(DISP_MMZ_BUF_S));

    stMMZ_Frame.u32StartPhyAddr = pCapFrameinfo->stBufAddr[0].u32PhyAddr_Y;

    nRet = DISP_OS_MMZ_Map(&stMMZ_Frame);
    if (MT_SUCCESS != nRet)
    {
        return MT_ERR_VO_MALLOC_FAILED;
    }

    u32FrameStartVirAddr =(ulong) phys_to_virt (u32FrameStartPhyAddr);

    u32StillFrameStartVirAddr = stMMZ_Frame.u32StartVirAddr;

    memcpy(pCapFrameinfo,pFrameinfo,sizeof(MT_DRV_VIDEO_FRAME_S));
    memcpy((void *)u32StillFrameStartVirAddr, (void *)u32FrameStartVirAddr,datalen);

    /*3: calculate  frame addr*/
    pCapFrameinfo->stBufAddr[0].u32PhyAddr_YHead = u32StillFrameStartPhyAddr +(pFrameinfo->stBufAddr[0].u32PhyAddr_YHead -u32FrameStartPhyAddr );
    pCapFrameinfo->stBufAddr[0].u32Stride_Y =  pFrameinfo->stBufAddr[0].u32Stride_Y;

    pCapFrameinfo->stBufAddr[0].u32PhyAddr_Y = u32StillFrameStartPhyAddr +(pFrameinfo->stBufAddr[0].u32PhyAddr_Y - u32FrameStartPhyAddr );

    pCapFrameinfo->stBufAddr[0].u32PhyAddr_C = u32StillFrameStartPhyAddr + (pCapFrameinfo->u32Height*pCapFrameinfo->stBufAddr[0].u32Stride_Y);

    pCapFrameinfo->stBufAddr[0].u32PhyAddr_CrHead = u32StillFrameStartPhyAddr +( pFrameinfo->stBufAddr[0].u32PhyAddr_CrHead -u32FrameStartPhyAddr);
    pCapFrameinfo->stBufAddr[0].u32PhyAddr_Cr = u32StillFrameStartPhyAddr +( pFrameinfo->stBufAddr[0].u32PhyAddr_Cr -u32FrameStartPhyAddr );
    pCapFrameinfo->stBufAddr[0].u32Stride_Cr = pFrameinfo->stBufAddr[0].u32Stride_Cr;
    DISP_OS_MMZ_UnMap(&stMMZ_Frame);

    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_CapturePicture(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic)
{
    mt_s32 Ret;
    WIN_CAPTURE_S   stCapture;

    if (!pstPic)
    {
        MT_ERR_WIN("para null!\n");
        return MT_FAILURE;
    }

    stCapture.hWindow = hWin;

    Ret = DRV_WIN_Process(CMD_VO_WIN_CAPTURE_START, &stCapture);
    if (MT_SUCCESS != Ret)
    {
        WIN_ERROR("capture err!\n");
        return Ret;
    }

    stCapture.hWindow = hWin;
    pstPic->stBufAddr[0].u32PhyAddr_Y = stCapture.driver_supply_addr.startPhyAddr;

    (mt_void)Make_CaptuerFrame(&(stCapture.CapPicture),pstPic);

    stCapture.hWindow = hWin;
    Ret = DRV_WIN_Process(CMD_VO_WIN_CAPTURE_RELEASE, &stCapture);
    if (MT_SUCCESS != Ret)
    {
        WIN_ERROR("capture RELEASE err!\n");
    }
    return Ret;
}

mt_s32 MT_DRV_WIN_CapturePictureRelease(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pstPic)
{
    mt_s32 Ret;
    WIN_CAPTURE_S  stCapture;

    if (!pstPic)
    {
        MT_ERR_WIN("para null!\n");
        return MT_FAILURE;
    }

    stCapture.hWindow = hWin;
    stCapture.CapPicture = *pstPic;

    Ret = DRV_WIN_Process(CMD_VO_WIN_CAPTURE_FREE, &stCapture);

    return Ret;
}

mt_s32 MT_DRV_WIN_SetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E enRotation)
{
    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_GetRotation(mt_handle hWin, MT_DRV_ROT_ANGLE_E *penRotation)
{
    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_SetFlip(mt_handle hWin, MT_BOOL bHoriFlip, MT_BOOL bVertFlip)
{
    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_GetFlip(mt_handle hWin, MT_BOOL *pbHoriFlip, MT_BOOL *pbVertFlip)
{
    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_SendFrame(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *pFrameinfo)
{
    mt_s32 Ret;
    Ret = WIN_SendFrame(hWin,pFrameinfo);
    return Ret;
}

mt_s32 MT_DRV_WIN_SetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    mt_s32 Ret;
    WIN_CREATE_S   stWinAttr;

    stWinAttr.hWindow = hWin;
    stWinAttr.WinAttr = *pWinAttr;
    Ret = DRV_WIN_Process(CMD_WIN_SET_ATTR, &stWinAttr);
    return Ret;
}

mt_s32 MT_DRV_WIN_GetAttr(mt_handle hWin, MT_DRV_WIN_ATTR_S *pWinAttr)
{
    mt_s32 Ret;
    WIN_CREATE_S   stWinAttr;

    memset(&stWinAttr, 0, sizeof(WIN_CREATE_S));
    stWinAttr.hWindow = hWin;
   // stWinAttr.WinAttr = *pWinAttr;
    Ret = DRV_WIN_Process(CMD_WIN_GET_ATTR, &stWinAttr);
    if(!Ret)
    {
        *pWinAttr = stWinAttr.WinAttr;
    }
    return Ret;
}

mt_s32 MT_DRV_WIN_GetPlayInfo(mt_handle hWindow, MT_DRV_WIN_PLAY_INFO_S *pInfo)
{
    mt_s32 Ret;
    WIN_PLAY_INFO_S WinPlayInfo;

    memset(&WinPlayInfo, 0, sizeof(WIN_PLAY_INFO_S));
    WinPlayInfo.hWindow = hWindow;

    Ret = DRV_WIN_Process(CMD_WIN_GET_PLAY_INFO, &WinPlayInfo);
    *pInfo = WinPlayInfo.stPlayInfo;
    return Ret;
}

mt_s32 MT_DRV_WIN_GetInfo(mt_handle hWindow, MT_DRV_WIN_INFO_S *pInfo)
{
    mt_s32 Ret;
    WIN_PRIV_INFO_S WinPrivInfo;

    memset(&WinPrivInfo, 0, sizeof(WIN_PRIV_INFO_S));
    WinPrivInfo.hWindow = hWindow;

    Ret = DRV_WIN_Process(CMD_WIN_GET_INFO, &WinPrivInfo);
    *pInfo = WinPrivInfo.stPrivInfo;

    return Ret;
}

mt_s32 DRV_WIN_SendFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;
    if (!pFrame)
    {
        MT_ERR_WIN("para null!\n");
        return MT_FAILURE;
    }

    stWinFrame.hWindow = hWindow;
    stWinFrame.stFrame = *pFrame;

    Ret = DRV_WIN_Process(CMD_WIN_SEND_FRAME, &stWinFrame);

    return Ret;
}

mt_s32 MT_DRV_WIN_QFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;

    stWinFrame.hWindow = hWindow;
    stWinFrame.stFrame = *pFrame;

    Ret = DRV_WIN_Process(CMD_WIN_QU_FRAME, &stWinFrame);

    return Ret;
}

mt_s32 MT_DRV_WIN_QULSFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;

    stWinFrame.hWindow = hWindow;
    stWinFrame.stFrame = *pFrame;

    Ret = DRV_WIN_Process(CMD_WIN_QU_ULSFRAME, &stWinFrame);

    return Ret;
}

mt_s32 MT_DRV_WIN_DQFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;

    stWinFrame.hWindow = hWindow;


    Ret = DRV_WIN_Process(CMD_WIN_DQ_FRAME, &stWinFrame);
    if (Ret)
    {
        stWinFrame.stFrame = *pFrame;
    }

    return Ret;
}
mt_s32 MT_DRV_WIN_SetPARA(mt_handle hWindow, MT_DRV_WIN_PARA_S * para)
{
    mt_s32     Ret;

    Ret = DRV_WIN_Process(CMD_WIN_SET_PARA, para);

    return Ret;
}
mt_s32 MT_DRV_WIN_GetPARA(mt_handle hWindow, MT_DRV_WIN_PARA_S * para)
{
    mt_s32     Ret;

    Ret = DRV_WIN_Process(CMD_WIN_GET_PARA, para);

    return Ret;
}

mt_s32 DRV_WIN_AcquireFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;

    stWinFrame.hWindow = hWindow;
    stWinFrame.stFrame = *pFrame;

    Ret = DRV_WIN_Process(CMD_WIN_VIR_ACQUIRE, &stWinFrame);

    return Ret;
}

mt_s32 DRV_WIN_ReleaseFrame(mt_handle hWindow, MT_DRV_VIDEO_FRAME_S *pFrame)
{
    mt_s32     Ret;
    WIN_FRAME_S stWinFrame;

    stWinFrame.hWindow = hWindow;
    stWinFrame.stFrame = *pFrame;

    Ret = DRV_WIN_Process(CMD_WIN_VIR_RELEASE, &stWinFrame);
    if (Ret)
    {
        stWinFrame.stFrame = *pFrame;
    }

    return Ret;
}

mt_s32 MT_DRV_WIN_SetZorder(mt_handle hWin, MT_DRV_DISP_ZORDER_E ZFlag)
{
    mt_s32     Ret;
    WIN_ZORDER_S stWinZorder;

    stWinZorder.hWindow = hWin;
    stWinZorder.eZFlag = ZFlag;
    Ret = DRV_WIN_Process(CMD_WIN_SET_ZORDER, &stWinZorder);
    return Ret;
}

mt_s32 MT_DRV_WIN_GetZorder(mt_handle hWin, mt_u32 *pu32Zorder)
{
    mt_s32     Ret;
    WIN_ORDER_S stWinOrder;

    memset(&stWinOrder, 0, sizeof(WIN_ORDER_S));
    stWinOrder.hWindow = hWin;

    Ret = DRV_WIN_Process(CMD_WIN_SET_ZORDER, &stWinOrder);
    if(!Ret)
    {
        *pu32Zorder = stWinOrder.Order ;
    }
    return Ret;
}

mt_s32 MT_DRV_WIN_Freeze(mt_handle hWin, MT_BOOL bEnable,  MT_DRV_WIN_SWITCH_E eRst)
{
    mt_s32     Ret;
    WIN_FREEZE_S stWinFreeze;

    stWinFreeze.hWindow = hWin;
    stWinFreeze.bEnable = bEnable;
    stWinFreeze.eMode = eRst;

    Ret = DRV_WIN_Process(CMD_WIN_FREEZE, &stWinFreeze);

    return Ret;
}

mt_s32 MT_DRV_WIN_GetLatestFrameInfo(mt_handle hWin, MT_DRV_VIDEO_FRAME_S *frame_info)
{
    mt_s32     Ret;
    WIN_FRAME_S  frame_info_ioctl;

    memset(&frame_info_ioctl, 0, sizeof(WIN_FRAME_S));
    frame_info_ioctl.hWindow = hWin;
    Ret = DRV_WIN_Process(CMD_WIN_GET_LATESTFRAME_INFO, &frame_info_ioctl);

    if (Ret != MT_SUCCESS)
        return Ret;

    *frame_info = frame_info_ioctl.stFrame;
    return Ret;
}


mt_s32 WIN_DRV_DestroyAll(WIN_STATE_S *pst2WinState)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 i;

    for(enDisp=0; enDisp<MT_DRV_DISPLAY_BUTT; enDisp++)
    {
        for(i=0; i<DEF_MAX_WIN_NUM_ON_SINGLE_DISP; i++)
        {
            if (pst2WinState->hWin[enDisp][i] != MT_NULL)
            {
                WIN_DestroyExt(pst2WinState->hWin[enDisp][i], pst2WinState);
            }
        }
    }

    for(i=0; i<DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP; i++)
    {
        if (pst2WinState->hVirtualWin[i] != MT_NULL)
        {
            WIN_DestroyExt(pst2WinState->hVirtualWin[i], pst2WinState);
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_DRV_WIN_Init(mt_void)
{
    mt_s32 i, j, Ret;

    Ret = down_interruptible(&g_VoMutex);

    for (i=MT_DRV_DISPLAY_0; i<MT_DRV_DISPLAY_BUTT; i++)
    {
        for (j=0; j<DEF_MAX_WIN_NUM_ON_SINGLE_DISP; j++)
        {
            g_VoModState.hWin[i][j] = MT_NULL;
        }
    }

    for (j=0; j<DEF_MAX_WIN_NUM_ON_VIRTUAL_DISP; j++)
    {
        g_VoModState.hVirtualWin[j] = MT_NULL;
    }

    if (1 == atomic_inc_return(&g_VoCount))
    {
        Ret = WIN_Init();
        if (Ret != MT_SUCCESS)
        {
            MT_FATAL_WIN("call VO_Init failed.\n");
            atomic_dec(&g_VoCount);
            up(&g_VoMutex);
            return -1;
        }
    }

    up(&g_VoMutex);

    Ret = DRV_WIN_Register();
    if (Ret != MT_SUCCESS)
    {
        MT_FATAL_WIN("call DRV_WIN_Register failed.\n");
        return -1;
    }

    return 0;
}

mt_s32 MT_DRV_WIN_DeInit(mt_void)
{
    mt_s32        Ret;

    Ret = down_interruptible(&g_VoMutex);

    WIN_DRV_DestroyAll(&g_VoModState);

    if (atomic_dec_and_test(&g_VoCount))
    {
        WIN_DeInit();
    }

    up(&g_VoMutex);
    return 0;
}

static WIN_EXPORT_FUNC_S s_stWinExportFuncs = {
    .pfnWinInit      = MT_DRV_WIN_Init,
    .pfnWinDeInit    = MT_DRV_WIN_DeInit,
    .pfnWinCreate    = MT_DRV_WIN_Create,
    .pfnWinDestory   = MT_DRV_WIN_Destroy,
    .pfnWinSetAttr   = MT_DRV_WIN_SetAttr,
    .pfnWinGetAttr   = MT_DRV_WIN_GetAttr,
    .pfnWinGetInfo   = MT_DRV_WIN_GetInfo,
    .pfnWinSetSrc    = MT_DRV_WIN_SetSource,
    .pfnWinGetSrc    = MT_DRV_WIN_GetSource,
    .pfnWinSetEnable = MT_DRV_WIN_SetEnable,
    .pfnWinGetEnable = MT_DRV_WIN_GetEnable,
    .pfnWinQueueFrm  = MT_DRV_WIN_QFrame,
    .pWinQueueUselessFrm = MT_DRV_WIN_QULSFrame,
    .pfnWinDequeueFrm  = MT_DRV_WIN_DQFrame,
    .pfnWinGetPlayInfo = MT_DRV_WIN_GetPlayInfo,
    .pfnWinSetZorder = MT_DRV_WIN_SetZorder,
    .pfnWinGetZorder = MT_DRV_WIN_GetZorder,
    .pfnWinFreeze    = MT_DRV_WIN_Freeze,
    .pfnWinGetLatestFrameInfo = MT_DRV_WIN_GetLatestFrameInfo,
    .pfnWinReset     = MT_DRV_WIN_Reset,
    .pfnWinPause     = MT_DRV_WIN_Pause,
    .pfnWinSetStepMode = MT_DRV_WIN_SetStepMode,
    .pfnWinSetStepPlay = MT_DRV_WIN_SetStepPlay,
    .pfnWinSetExtBuffer= MT_DRV_WIN_SetExtBuffer,
    .pfnWinAcquireFrm  = MT_DRV_WIN_AcquireFrame,
    .pfnWinRlsFrm      = MT_DRV_WIN_ReleaseFrame,
//    .pfnWin3DMode      = MT_DRV_WIN_Set3DMode,
    .pfnWinSetQuik   = MT_DRV_WIN_SetQuick,
    .pfnWinCapturePic  = MT_DRV_WIN_CapturePicture,
    .pfnWinCapturePicRls = MT_DRV_WIN_CapturePictureRelease,
    .pfnWinSetRotation = MT_DRV_WIN_SetRotation,
    .pfnWinGetRotation = MT_DRV_WIN_GetRotation,
    .pfnWinSetFlip   = MT_DRV_WIN_SetFlip,
    .pfnWinGetFlip   = MT_DRV_WIN_GetFlip,
    .pfnWinSendFrm   = MT_DRV_WIN_SendFrame,
    .pfnWinResume = DRV_WIN_Resume,
    .pfnWinSuspend = DRV_WIN_Suspend,
};

mt_s32 DRV_WIN_Register(mt_void)
{
#if 1
    mt_s32  Ret;

    Ret = mt_drv_module_register((mt_u32)MT_ID_VO, "MT_VO", (mt_void *)(&s_stWinExportFuncs));//MT_DRV_MODULE_Register
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_WIN("MT_DRV_MODULE_Register VO failed\n");
        return Ret;
    }

    g_VoSuspend = MT_FALSE;
#endif
    return  0;
}

mt_void DRV_WIN_UnRegister(mt_void)
{
#if 1
    MT_DRV_WIN_DeInit();

    mt_drv_module_unregister(MT_ID_VO);//MT_DRV_MODULE_UnRegister
#endif
    return;
}

mt_s32 WIN_DRV_Open(struct inode *finode, struct file  *ffile)
{
    WIN_STATE_S *pWinState = MT_NULL;
    mt_s32     Ret;
    MT_S32 mt_idx = iminor(finode);
    MT_DRV_WIN_PRIV_DATA *win_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(win_priv_data->hdclk))
        clk_prepare_enable(win_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data->hd_os_clk))
        clk_prepare_enable(win_priv_data->hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_108m))
        clk_prepare_enable(win_priv_data->sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_27m))
        clk_prepare_enable(win_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data->vbiclk))
        clk_prepare_enable(win_priv_data->vbiclk);

    Ret = down_interruptible(&g_VoMutex);

    pWinState = MT_KMALLOC(MT_ID_VO, sizeof(WIN_STATE_S), GFP_KERNEL);
    if (!pWinState)
    {
        WIN_FATAL("malloc pWinState failed.\n");
        up(&g_VoMutex);
        return -1;
    }

    memset(pWinState, 0, sizeof(WIN_STATE_S));

    if (1 == atomic_inc_return(&g_VoCount))
    {
        Ret = WIN_Init();
        if (Ret != MT_SUCCESS)
        {
            MT_KFREE(MT_ID_VO, pWinState);
            WIN_FATAL("call VO_Init failed.\n");
            atomic_dec(&g_VoCount);
            up(&g_VoMutex);
            return -1;
        }
    }

    ffile->private_data = pWinState;

    up(&g_VoMutex);
    return 0;
}

mt_s32 WIN_DRV_Close(struct inode *finode, struct file  *ffile)
{
    MT_S32 mt_idx = iminor(finode);
    MT_DRV_WIN_PRIV_DATA *win_priv_data = get_mt_priv(mt_idx);
    WIN_STATE_S    *pVoState;
    mt_s32        Ret;
    mt_u32        i = 0, j = 0;

    WIN_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);

    Ret = down_interruptible(&g_VoMutex);
    pVoState = ffile->private_data;

    /*first close all the services belongs to the window.*/
    if (!pVoState)
    {
        WIN_FATAL(" pVoState is null.\n");
        return MT_FAILURE;
    }

    for (i = 0; i < MT_DRV_DISPLAY_BUTT; i++)
    {
        for (j = 0; j < DEF_MAX_WIN_NUM_ON_SINGLE_DISP; j++)
        {
            if (pVoState->hCapture[i][j] != 0)
                WinForceClearCapture(pVoState->hCapture[i][j]);
        }
    }
    WIN_DRV_DestroyAll(pVoState);

    if (atomic_dec_and_test(&g_VoCount))
        WIN_DeInit();

    MT_KFREE(MT_ID_VO, pVoState);

    up(&g_VoMutex);

    if (!IS_ERR_OR_NULL(win_priv_data->hdclk))
        clk_disable_unprepare(win_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(win_priv_data->hd_os_clk))
        clk_disable_unprepare(win_priv_data->hd_os_clk);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_108m))
        clk_disable_unprepare(win_priv_data->sdclk_108m);
    if (!IS_ERR_OR_NULL(win_priv_data->sdclk_27m))
        clk_disable_unprepare(win_priv_data->sdclk_27m);
    if (!IS_ERR_OR_NULL(win_priv_data->vbiclk))
        clk_disable_unprepare(win_priv_data->vbiclk);

    return 0;

   return 0;
}

mt_s32 DRV_WIN_Ioctl(struct inode *inode, struct file  *file, unsigned int cmd, mt_void *arg)
{
    WIN_STATE_S   *pstWinState;
    mt_s32        Ret;

    Ret = down_interruptible(&g_VoMutex);

    pstWinState = file->private_data;

    //printk("zzzzzzzzzzzzzzz WIN cmd = 0x%x\n", cmd);
    Ret = WIN_ProcessCmd(cmd, arg, pstWinState);

    up(&g_VoMutex);
    return Ret;
}

EXPORT_SYMBOL(g_VoMutex);

EXPORT_SYMBOL(DRV_WIN_Ioctl);
EXPORT_SYMBOL(WIN_DRV_Open);
EXPORT_SYMBOL(WIN_DRV_Close);
EXPORT_SYMBOL(DRV_WIN_Suspend);
EXPORT_SYMBOL(DRV_WIN_Resume);
//EXPORT_SYMBOL(DRV_WIN_ProcRegister);
//EXPORT_SYMBOL(DRV_WIN_ProcfUnRegister);
EXPORT_SYMBOL(WIN_AcquireFrame);
EXPORT_SYMBOL(WIN_ReleaseFrame);

EXPORT_SYMBOL(MT_DRV_WIN_SetEnable);
EXPORT_SYMBOL(DRV_WIN_Register);
EXPORT_SYMBOL(DRV_WIN_UnRegister);

EXPORT_SYMBOL(MT_DRV_WIN_Init);
EXPORT_SYMBOL(MT_DRV_WIN_DeInit);
EXPORT_SYMBOL(MT_DRV_WIN_QFrame);
EXPORT_SYMBOL(MT_DRV_WIN_CapturePicture);
EXPORT_SYMBOL(MT_DRV_WIN_QULSFrame);
EXPORT_SYMBOL(DRV_WIN_Process);
EXPORT_SYMBOL(MT_DRV_WIN_Create);
EXPORT_SYMBOL(MT_DRV_WIN_Destroy);
EXPORT_SYMBOL(MT_DRV_WIN_Freeze);
EXPORT_SYMBOL(MT_DRV_WIN_Pause);

EXPORT_SYMBOL(MT_DRV_WIN_SetSource);
EXPORT_SYMBOL(MT_DRV_WIN_Reset);
EXPORT_SYMBOL(MT_DRV_WIN_GetPlayInfo);
EXPORT_SYMBOL(DRV_WIN_SendFrame);
EXPORT_SYMBOL(MT_DRV_WIN_SetZorder);
EXPORT_SYMBOL(DRV_WIN_SetSftwareStage);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


