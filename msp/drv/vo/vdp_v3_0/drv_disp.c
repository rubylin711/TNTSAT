/******************************************************************************

  Copyright (C), 2017, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : drv_disp.c
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
#include <linux/seq_file.h>
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
#include "drv_disp_ioctl.h"
#include "drv_display.h"
#include "drv_disp_debug.h"
#include "drv_disp_ext.h"
#include "drv_disp_osal.h"
#include "mt_osal.h"
#include "drv_win_ext.h"
#include "drv_disp_priv.h"
#include "drv_win_priv.h"
#include "mt_drv_stat.h"
#include "mt_module_debug.h"
#include "vo_fw.h"
#include "MT_DF_video.h"
#include "drv_disp_Symphony_reg.h"
#include "hd_enc_aria_reg.h"
#include "sd_enc_aria_reg.h"
/*
#include "drv_hdmi_ext.h"
*/
#include "mt_error_mpi.h"
#include "mt_drv_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

DRV_DISP_STATE_S         g_DispModState;
DRV_DISP_GLOBAL_STATE_S  g_DispUserCountState;
DRV_DISP_GLOBAL_STATE_S  g_DispKernelCountState;
DRV_DISP_GLOBAL_STATE_S  g_DispAllCountState;
MT_BOOL g_DispSuspend = MT_FALSE;
mt_s32 g_s32DispAttachCount = 0;
mt_u32 g_bDispPrivateInformation = 0;

static atomic_t        g_DispCount = ATOMIC_INIT(0);
static atomic_t        g_DispLayerCount = ATOMIC_INIT(0);
MT_DECLARE_MUTEX(g_DispMutex);

extern WINDOW_S *g_pstWin[];
extern MT_BOOL SdScalerEnable;

mt_s32 DRV_DISP_ProcessCmd(unsigned int cmd, mt_void *arg, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser);
mt_s32 DISP_ExtOpen(MT_DRV_DISPLAY_E enDisp, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser);
mt_s32 DISP_ExtClose(MT_DRV_DISPLAY_E enDisp, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser);
extern mt_void disp_st_vid_get_vdec_size(u32 *p_height, u32 *p_width);

/* ================================================ */
#define DEF_DRV_DISP_PROC_FUNCTION_START_FROM_HERE
mt_u8 *g_pDispLayerString[MT_DRV_DISP_LAYER_BUTT] = {
    "NONE",
    "VIDEO",
    "GFX",
};

//TODO
mt_u8 *g_pVDPDispFmtString[MT_DRV_DISP_FMT_BUTT] = {
    "1080P60",
    "1080P50",
    "1080P30",
    "1080P25",
    "1080P24",
    "1080i60",
    "1080i50",
    "720P60",
    "720P50",

    "576P50",
    "480P60",

    "PAL",
    "PAL_B",
    "PAL_B1",
    "PAL_D",
    "PAL_D1",
    "PAL_G",
    "PAL_H",
    "PAL_K",
    "PAL_I",
    "PAL_N",
    "PAL_Nc",
    
    "PAL_M",
    "PAL_60",
    "NTSC",
    "NTSC_J",
    "NTSC_443",

    "SECAM_SIN",
    "SECAM_COS",
    "SECAM_L",
    "SECAM_B",
    "SECAM_G",
    "SECAM_D",
    "SECAM_K",
    "SECAM_H",

    "1440x576i",
    "1440x480i",

    "1080P24_FP",
    "720P60_FP",
    "720P50_FP",

    "640x480",
    "800x600",
    "1024x768",
    "1280x720",
    "1280x800",
    "1280x1024",
    "1360x768",
    "1366x768",
    "1400x1050",
    "1440x900",
    "1440x900_RB",

    "1600x900_RB",
    "1600x1200",
    "1680x1050",
    "1680x1050_RB",
    "1920x1080",
    "1920x1200",
    "1920x1440",
    "2048x1152",
    "2560x1440_RB",
     "2560x1600_RB",

    "3840X2160_24",
    "3840X2160_25",
    "3840X2160_30",
    "3840X2160_50",
    "3840X2160_60",
    "4096X2160_24",
    "4096X2160_25",
    "4096X2160_30",
    "4096X2160_50",
    "4096X2160_60",


    "CustomerTiming"
};

mt_u8 *g_pVDPDispModeString[DISP_STEREO_BUTT] = {
    "2D",
    "FPK",
    "SBS_HALF",
    "TAB",
    "FILED_ALTE",
    "LINE_ALTE",
    "SBS_FULL",
    "L_DEPTH",
    "L_DEPTH_G_DEPTH",
};


mt_u8 *g_pVDPColorSpaceString[MT_DRV_CS_BUTT] = {
    "Unknown",
    "Default",
    "BT601_YUV_LIMITED",
    "BT601_YUV_FULL",
    "BT601_RGB_LIMITED",
    "BT601_RGB_FULL",
    "NTSC1953",
    "BT470_M",
    "BT470_BG",
    "BT709_YUV_LIMITED",
    "BT709_YUV_FULL",
    "BT709_RGB_LIMITED",
    "BT709_RGB_FULL",
    "REC709",
    "SMPT170M",
    "SMPT240M",
    "BT878",
    "XVYCC",
    "JPEG",
};


mt_u8 *g_pVDPCInterfaceString[MT_DRV_DISP_INTF_ID_MAX] = {
    "YPbPr0",
      "RGB0",
    "S_VIDEO0",
    "CVBS0",
    "VGA0",
    "HDMI0",
    "HDMI1",
    "HDMI2",
    "BT656_0",
    "BT656_1",
    "BT656_2",
    "BT1120_0",
    "BT1120_1",
    "BT1120_2",
    "LCD_0",
    "LCD_1",
    "LCD_2",
};

mt_u8 *g_pDispMacrovisionString[4] = {"TYPE0", "TYPE1", "TYPE2", "TYPE3"};
#define DISPLAY_INVALID_ID 0xFFFFFFFFul
static mt_u32 s_DispProcId[MT_DRV_DISPLAY_BUTT]={DISPLAY_INVALID_ID};

static mt_s32 DISP_ProcRead(struct seq_file *p, mt_void *v);
static mt_s32 DISP_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos);


static mt_s32 DRV_DISP_ProcInit(mt_void)
{
#if 1
    mt_u32 u;

    for(u=0; u<(mt_u32)MT_DRV_DISPLAY_BUTT; u++)
    {
        s_DispProcId[u] = DISPLAY_INVALID_ID;
    }
#endif

    return MT_SUCCESS;
}

static mt_void DRV_DISP_ProcDeInit(mt_void)
{
#if 0
    mt_char ProcName[12];
    mt_u32 u;

    for(u=0; u<(mt_u32)MT_DRV_DISPLAY_BUTT; u++)
    {
        if (s_DispProcId[u] != DISPLAY_INVALID_ID)
        {
            mt_osal_snprintf(ProcName, 12, "%s%d", MT_MOD_DISP, u);
            MT_DRV_PROC_RemoveModule(ProcName);
        }
    }
#endif

    return;
}

static mt_s32 DRV_DISP_ProcAdd(MT_DRV_DISPLAY_E enDisp)
{

    mt_proc_entry_t  *pProcItem;
    mt_char           ProcName[12];

    /* register HD-display PROC*/
    mt_osal_snprintf(ProcName, 12, "%s%d", MT_MOD_DISP, enDisp);
    pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);
    if (!pProcItem)
    {
        MT_FATAL_DISP("add %s proc failed.\n", ProcName);
        DISP_Close(enDisp);

        return MT_ERR_DISP_CREATE_ERR;
    }

    pProcItem->read  = DISP_ProcRead;
    pProcItem->write = DISP_ProcWrite;
    pProcItem->data  = (mt_void *)enDisp;

    s_DispProcId[enDisp] = enDisp;

    return MT_SUCCESS;
}

static mt_s32 DRV_DISP_ProcDel(MT_DRV_DISPLAY_E enDisp)
{
    mt_char ProcName[12];

    /* register HD-display PROC*/
    mt_osal_snprintf(ProcName, 12, "%s%d", MT_MOD_DISP, enDisp);
    mt_drv_proc_rm_module(ProcName);

    s_DispProcId[enDisp] = DISPLAY_INVALID_ID;

    return MT_SUCCESS;
}

#if 0
mt_u8 disp_attr[256];
mt_u8 *Disp_GetAttr(mt_void)
{
    mt_u8 *pBuffer;
    memset(disp_attr, 0x0, 256);

#ifdef  MT_DISP_TTX_SUPPORT
    pBuffer = "ttx ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_DISP_ATTACH_OSD_SUPPORT
    pBuffer = "attach_osd";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef  MT_DISP_MODE_TC
    pBuffer = "tc ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_WRAP_SUPPORT
    pBuffer = "vo_wrap";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_DISP_CGMS_SUPPOR
    pBuffer = "cgms ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_DISP_LCD_SUPPORT
    MT_OSAL_Strncat(disp_attr, "lcd ", 256);
#endif

#ifdef MT_VO_MOSAIC_SUPPORT
    pBuffer = "mosaic ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_SINGLE_VIDEO_SUPPORT
    pBuffer = "signle_vid ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_STILLFRAME_SUPPORT
    pBuffer = "vo_still ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_DUMPFRAME_SUPPORT
    pBuffer = "vo_dump ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_MOSAIC_SUPPORT
    pBuffer = "mosaic ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_SHARPNESS_SUPPORT
    pBuffer = "sharp ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif

#ifdef MT_VO_HD_VIDEO_DO_DEI
    pBuffer = "vo_not_dei ";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
#endif
    pBuffer = "\r\n";
    MT_OSAL_Strncat(disp_attr, pBuffer, strlen(pBuffer)+1);
    return disp_attr;
}
#endif

#ifndef MT_ADVCA_FUNCTION_RELEASE
mt_char *g_pVDPDispName[MT_DRV_DISPLAY_BUTT+1] = {"display0", "display1","display2","invalid  display"};
mt_char *g_pVDPDispState[2] = {"Close", "Open"};
mt_char *g_pVDPDispState_1[2] = {"Disable", "Enable"};
mt_char *g_pVDPDispARMode[2] = {"Auto", "Custmer Setting"};
mt_char *g_pVDPDispCastAllocMode[2] = {"DispAllocate", "UserAllocate"};
mt_u32  g_pVDPDispCastBufferState[5] = {1, 4, 2, 3, 4};
static DISP_PROC_INFO_S  s_DispAttr;
#endif


static inline void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
    *(p_addr) = data;
}
static inline MT_U32 hal_get_u32(volatile MT_U32 *p_addr)
{
    return *(volatile MT_U32 *)(p_addr);
}

static void Proc_Dump_Register(struct seq_file *p, char *module, ulong reg_base, unsigned int range)
{
	unsigned int reg_offset;

    PROC_PRINT(p, "--------------------------Dump %s Registers-------------------------", module);
	for (reg_offset=0; reg_offset<range; reg_offset+=4)
	{
		if ((reg_offset%16) == 0)
		{
			PROC_PRINT(p, "\n0x%08lx : ", reg_base+reg_offset);
		}
		PROC_PRINT(p, "%08x ", hal_get_u32((volatile MT_U32 *)(reg_base+reg_offset)));
	}
    PROC_PRINT(p, "\n");
}

static mt_s32 DISP_ProcRead(struct seq_file *p, mt_void *v)
{
#ifndef MT_ADVCA_FUNCTION_RELEASE
    mt_proc_entry_t  *pProcItem;
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 Ret, i;

    pProcItem = p->private;
    enDisp = (MT_DRV_DISPLAY_E)pProcItem->data;

    /* Disp PROC*/
    Ret = DISP_GetProcInto(enDisp, &s_DispAttr);
    if (Ret)
    {
        PROC_PRINT(p,"---------Get Montage DISP %d Out Info Failed!---------\n", enDisp);
        return MT_SUCCESS;
    }
    PROC_PRINT(p,"---------Montage DISP %d State---------\n", enDisp);

    PROC_PRINT(p, "%-20s:%s\n", "State",g_pVDPDispState[s_DispAttr.bEnable]);
    PROC_PRINT(p, "%-20s:%s/%s\n", "Format/DispMode",g_pVDPDispFmtString[s_DispAttr.eFmt], g_pVDPDispModeString[s_DispAttr.eDispMode]);
    if (MT_DRV_DISP_FMT_CUSTOM == s_DispAttr.eFmt)
    {
        PROC_PRINT(p, "%-20s:\n", "------Custom Timing Para List------");
        PROC_PRINT(p, "       %-20s: %d/%d/%d\n","HACT/VACT/VFreq" ,s_DispAttr.stTiming.u32HACT,s_DispAttr.stTiming.u32VACT,s_DispAttr.stTiming.u32VertFreq);
        PROC_PRINT(p, "       %-20s: %d/%d/%d\n","HBB/HFB/HPW " ,s_DispAttr.stTiming.u32HBB,s_DispAttr.stTiming.u32HFB,s_DispAttr.stTiming.u32HPW);
        PROC_PRINT(p, "       %-20s: %d/%d/%d\n","VBB/VFB/VPW" ,s_DispAttr.stTiming.u32VBB,s_DispAttr.stTiming.u32VFB,s_DispAttr.stTiming.u32VPW);
        PROC_PRINT(p, "       %-20s: %d/%d/%d/%d\n","Inter/IDV/IHS/IVS" ,s_DispAttr.stTiming.bInterlace,s_DispAttr.stTiming.bIDV,s_DispAttr.stTiming.bIHS,s_DispAttr.stTiming.bIVS);
        PROC_PRINT(p, "       %-20s: %d/%d/%d\n","PixFreq/ARW/ARH",s_DispAttr.stTiming.u32PixFreq, s_DispAttr.stTiming.u32AspectRatioW,s_DispAttr.stTiming.u32AspectRatioH);
        PROC_PRINT(p, "       %-20s: 0x%x/0x%x\n","ClkPara0/ClkPara1/" ,s_DispAttr.stTiming.u32ClkPara0,s_DispAttr.stTiming.u32ClkPara1);
    }
    PROC_PRINT(p, "%-20s:%s\n", "RightEyeFirst",g_pVDPDispState_1[s_DispAttr.bRightEyeFirst]);
    PROC_PRINT(p, "%-20s:%d/%d\n", "VirtualScreen",s_DispAttr.stVirtaulScreen.s32Width,s_DispAttr.stVirtaulScreen.s32Height);
    PROC_PRINT(p, "%-20s:%d/%d/%d/%d\n", "Offset(L/T/R/B)",s_DispAttr.stOffsetInfo.u32Left,
                                                  s_DispAttr.stOffsetInfo.u32Top,
                                                  s_DispAttr.stOffsetInfo.u32Right,
                                                  s_DispAttr.stOffsetInfo.u32Bottom);

    PROC_PRINT(p, "%-20s:%s\n", "AspectRatioMode",g_pVDPDispARMode[(mt_u32)s_DispAttr.bCustAspectRatio]);
    PROC_PRINT(p, "%-20s:%d:%d\n", "AspectRatio",s_DispAttr.u32AR_w,s_DispAttr.u32AR_h);

    PROC_PRINT(p, "%-20s:%s->%s\n", "ColorSpace",g_pVDPColorSpaceString[(mt_u32)s_DispAttr.eDispColorSpace],
                                                g_pVDPColorSpaceString[(mt_u32)s_DispAttr.eDispColorSpace]);
    PROC_PRINT(p, "%-20s:%d\n", "Bright",s_DispAttr.u32Bright);
    PROC_PRINT(p, "%-20s:%d\n", "Contrast",s_DispAttr.u32Contrst);
    PROC_PRINT(p, "%-20s:%d\n", "Saturation",s_DispAttr.u32Satur);
    PROC_PRINT(p, "%-20s:%d\n", "Hue",s_DispAttr.u32Hue);
    PROC_PRINT(p, "%-20s:0x%x/0x%x/0x%x\n", "Background (R/G/B)",s_DispAttr.stBgColor.u8Red,
                                                     s_DispAttr.stBgColor.u8Green,
                                                     s_DispAttr.stBgColor.u8Blue);
    PROC_PRINT(p, "%-20s:%s->%s\n", "Zorder(Bot->Top)",g_pDispLayerString[s_DispAttr.enLayer[0]],
                                                        g_pDispLayerString[s_DispAttr.enLayer[1]]);

    if (s_DispAttr.bMaster == MT_TRUE)
        PROC_PRINT(p, "%-20s:%s\n", "AttachRole","source");

    if (s_DispAttr.bSlave == MT_TRUE)
        PROC_PRINT(p, "%-20s:%s\n", "AttachRole","destination");

    if ((s_DispAttr.bMaster != MT_TRUE) && (s_DispAttr.bSlave != MT_TRUE))
        PROC_PRINT(p, "%-20s:%s\n", "AttachRole","single running.");

    PROC_PRINT(p, "%-20s:%s\n", "AttachDisp",g_pVDPDispName[s_DispAttr.enAttachedDisp]);

    PROC_PRINT(p, "%-20s:", "Interface");
    for(i=0; i<s_DispAttr.u32IntfNumber;i++)
    {
        PROC_PRINT(p, "%s ", g_pVDPCInterfaceString[(mt_s32)s_DispAttr.stIntf[i].eID]);

        if (s_DispAttr.stIntf[i].eID <= MT_DRV_DISP_INTF_VGA0)
            PROC_PRINT(p, "(%3d/%3d/%3d) ",
                s_DispAttr.stIntf[i].u8VDAC_Y_G,
                s_DispAttr.stIntf[i].u8VDAC_Pb_B,
                s_DispAttr.stIntf[i].u8VDAC_Pr_R);
    }
    PROC_PRINT(p, "\n");

    PROC_PRINT(p, "%-20s:%d\n", "InitCount", atomic_read(&g_DispCount));
    PROC_PRINT(p, "%-20s:%d/%d\n", "OpenCnt[User/Kernel]", g_DispUserCountState.DispOpenNum[enDisp], g_DispKernelCountState.DispOpenNum[enDisp]);
    PROC_PRINT(p, "%-20s:%d\n", "LowbandCount", s_DispAttr.u32Underflow);

    if (s_DispAttr.pstCastInfor)
    {
        PROC_PRINT(p,"------------------CAST Info---------------------------------\n");
        PROC_PRINT(p, "%-20s:%s\n", "State", g_pVDPDispState_1[s_DispAttr.stCastInfor.bEnable]);
        PROC_PRINT(p, "%-20s:%s\n", "Crop", "False");
        PROC_PRINT(p, "%-20s:%d/%d/%d/%d\n", "CropRect(L/T/R/B)", 0,0,0,0);
        PROC_PRINT(p, "%-20s:%d/%d\n", "Resolution", s_DispAttr.stCastInfor.u32OutResolutionWidth,
                                                  s_DispAttr.stCastInfor.u32OutResolutionHeight);

        PROC_PRINT(p, "%-20s:%s\n", "PixelFormat","NV21");
        PROC_PRINT(p, "%-20s:%d\n", "FrameRate", s_DispAttr.stCastInfor.u32CastOutFrameRate/2);
        PROC_PRINT(p, "%-20s:%s\n", "LowDelay", g_pVDPDispState_1[s_DispAttr.stCastInfor.bLowDelay]);

        PROC_PRINT(p, "%-20s:%s\n", "MemoryType", g_pVDPDispCastAllocMode[s_DispAttr.stCastInfor.bUserAllocate]);
        PROC_PRINT(p, "%-20s:%d\n", "BufferNumber", s_DispAttr.stCastInfor.u32TotalBufNum);

        PROC_PRINT(p, "%-20s:%d/%d\n", "BufferWidth/Height", s_DispAttr.stCastInfor.u32OutResolutionWidth,
                                                             s_DispAttr.stCastInfor.u32OutResolutionHeight);

        PROC_PRINT(p, "%-20s:0x%x\n", "BufferSize", s_DispAttr.stCastInfor.u32BufSize);
        PROC_PRINT(p, "%-20s:%d\n", "BufferStride", s_DispAttr.stCastInfor.u32BufStride);

        PROC_PRINT(p, "%-20s:%d\n", "bAttached", s_DispAttr.stCastInfor.bAttached);
        if (g_bDispPrivateInformation)
        {
           PROC_PRINT(p, "%-20s:%d\n", "CastIntrCnt", s_DispAttr.stCastInfor.u32CastIntrCnt);
        }

        PROC_PRINT(p,"------------------Buffer---------------------------------\n");
        PROC_PRINT(p, "%-20s:%d/%d\n", "Acquire(Try/OK)", s_DispAttr.stCastInfor.u32CastAcquireTryCnt,
                                                          s_DispAttr.stCastInfor.u32CastAcquireOkCnt);

        PROC_PRINT(p, "%-20s:%d/%d\n", "Release(Try/OK)", s_DispAttr.stCastInfor.u32CastReleaseTryCnt,
                                                          s_DispAttr.stCastInfor.u32CastReleaseOkCnt);


        PROC_PRINT(p,"---------------------------------------------------------\n");
        PROC_PRINT(p,"BufferQueue: [state, FrameID]\n");
        PROC_PRINT(p,"(State: 1,Empty[%d]; 2,Write[%d]; 3,Full[%d]; 4,Use[%d]\n",
                                    s_DispAttr.stCastInfor.u32CastEmptyBufferNum,
                                    s_DispAttr.stCastInfor.u32CastWriteBufferNum,
                                    s_DispAttr.stCastInfor.u32CastFullBufferNum,
                                    s_DispAttr.stCastInfor.u32CastUsedBufferNum);

        for (i = 0; i < s_DispAttr.stCastInfor.u32TotalBufNum; i++)
        {
            if ((i != 0) && ((i%4) ==0))
                PROC_PRINT(p,"\n");

            PROC_PRINT(p,"[%d,0x%x] ", g_pVDPDispCastBufferState[s_DispAttr.stCastInfor.enState[i]],
                                           s_DispAttr.stCastInfor.u32FrameIndex[i]);
        }

        PROC_PRINT(p,"\n");
    }

    //dump vdec registers
    Proc_Dump_Register(p, "DISP1-part1", REG_SYMPHONY_DISP_DRV_BASE, 0x800);
    Proc_Dump_Register(p, "DISP1-part2", REG_SYMPHONY_DISP_DRV_BASE+0x1000, 0x7c);
    Proc_Dump_Register(p, "DISP1-part3", REG_SYMPHONY_DISP_DRV_BASE+0x2000, 0x3c0);
    Proc_Dump_Register(p, "DISP1-part4", REG_SYMPHONY_DISP_DRV_BASE+0x3000, 0x128);
    Proc_Dump_Register(p, "DISP1-part5", REG_SYMPHONY_DISP_DRV_BASE+0x4000, 0x1cc);
    Proc_Dump_Register(p, "DISP1-part6", REG_SYMPHONY_DISP_DRV_BASE+0x6000, 0x2e8);
    Proc_Dump_Register(p, "DISP1-part7", REG_SYMPHONY_DISP_DRV_BASE+0x7000, 0x158);
    Proc_Dump_Register(p, "DISP1-part8", REG_SYMPHONY_DISP_DRV_BASE+0x8000, 0x64);
    Proc_Dump_Register(p, "DISP1-part9", REG_SYMPHONY_DISP_DRV_BASE+0x9090, 0x34);
    Proc_Dump_Register(p, "DISP1-part9", REG_SYMPHONY_DISP_DRV_BASE+0x9300, 0xd8);
    Proc_Dump_Register(p, "DISP1-part9", REG_SYMPHONY_DISP_DRV_BASE+0xc000, 0xF);
    
    Proc_Dump_Register(p, "HDVENC", REG_ARIA_HD_ENCODER_BASE, 0x100);
    Proc_Dump_Register(p, "SDVENC", REG_ARIA_SD_ENCODER_BASE, 0x1B0);
    Proc_Dump_Register(p, "VBI", REG_SYMPHONY_VBI_BASE, 0xB0);

    PROC_PRINT(p, "=======================================================================\n");

#endif

    return MT_SUCCESS;
}

#if 0
static mt_s32 DispProcParsePara(mt_char *pProcPara,mt_char **ppArg1,mt_char **ppArg2)
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
#endif


MT_DRV_DISP_FMT_E DispGetFmtbyString(mt_char *pFmtString)
{
#if 0
    if (0 == MT_OSAL_Strncmp(pFmtString, "1080p60", strlen("1080p60")))
    {
        return MT_DRV_DISP_FMT_1080P_60;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080p50", strlen("1080p50")))
    {
        return MT_DRV_DISP_FMT_1080P_50;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080p30", strlen("1080p30")))
    {
        return MT_DRV_DISP_FMT_1080P_30;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080p25", strlen("1080p25")))
    {
        return MT_DRV_DISP_FMT_1080P_25;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080p24", strlen("1080p24")))
    {
        return MT_DRV_DISP_FMT_1080P_24;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080i60", strlen("1080i60")))
    {
        return MT_DRV_DISP_FMT_1080i_60;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080i50", strlen("1080i50")))
    {
        return MT_DRV_DISP_FMT_1080i_50;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "720p60", strlen("720p60")))
    {
        return MT_DRV_DISP_FMT_720P_60;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "720p50", strlen("720p50")))
    {
        return MT_DRV_DISP_FMT_720P_50;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "576p50", strlen("576p50")))
    {
        return MT_DRV_DISP_FMT_576P_50;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "480p60", strlen("480p60")))
    {
        return MT_DRV_DISP_FMT_480P_60;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "pal", strlen("pal")))
    {
        return MT_DRV_DISP_FMT_PAL;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "ntsc", strlen("ntsc")))
    {
        return MT_DRV_DISP_FMT_NTSC;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "1080p24fp", strlen("1080p24fp")))
    {
        return MT_DRV_DISP_FMT_1080P_24_FP;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "720p60fp", strlen("720p60fp")))
    {
        return MT_DRV_DISP_FMT_720P_60_FP;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "720p50fp", strlen("720p50fp")))
    {
        return MT_DRV_DISP_FMT_720P_50_FP;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "3840*2160_24", strlen("3840*2160_24")))
    {
        return MT_DRV_DISP_FMT_3840X2160_24;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "3840*2160_25", strlen("3840*2160_25")))
    {
        return MT_DRV_DISP_FMT_3840X2160_25;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "3840*2160_30", strlen("3840*2160_30")))
    {
        return MT_DRV_DISP_FMT_3840X2160_30;
    }
    else if (0 == MT_OSAL_Strncmp(pFmtString, "4096*2160_24", strlen("4096*2160_24")))
    {
        return MT_DRV_DISP_FMT_4096X2160_24;
    }

    else
    {
        return MT_DRV_DISP_FMT_BUTT;
    }
#endif

return MT_DRV_DISP_FMT_BUTT;

}

//mt_s32 DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E enStereo, MT_DRV_DISP_FMT_E enEncFmt);
mt_s32 DispProcCmdProcee(MT_DRV_DISPLAY_E enDisp, mt_char *pArg1,mt_char *pArg2)
{
    mt_s32 nRet = MT_SUCCESS;
#if 0
    mt_handle  hCommonHandle;

    if (0 == MT_OSAL_Strncmp(pArg1, "fmt", strlen("fmt")))
    {
        MT_DRV_DISP_FMT_E fmt;

        fmt = DispGetFmtbyString(pArg2);

        if (fmt == MT_DRV_DISP_FMT_BUTT)
        {
            return MT_ERR_DISP_NOT_SUPPORT_FMT;
        }
        else if( (fmt >= MT_DRV_DISP_FMT_1080P_24_FP) && (fmt <= MT_DRV_DISP_FMT_720P_50_FP))
        {
            nRet = DISP_SetFormat(enDisp, MT_DRV_DISP_STEREO_FRAME_PACKING, fmt);;
        }
        else
        {
            MT_DRV_DISP_FMT_E fmtold;
            MT_DRV_DISP_STEREO_MODE_E enStereoold;

            nRet = DISP_GetFormat(enDisp, &enStereoold, &fmtold);
            if (nRet == MT_SUCCESS)
            {
                nRet = DISP_SetFormat(enDisp, enStereoold, fmt);
            }
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "3d", strlen("3d")))
    {
        MT_DRV_DISP_FMT_E fmtold;
        MT_DRV_DISP_STEREO_MODE_E enStereoold;

        nRet = DISP_GetFormat(enDisp, &enStereoold, &fmtold);
        if (nRet != MT_SUCCESS)
        {
            return MT_FAILURE;
        }

        if (0 == MT_OSAL_Strncmp(pArg2, "2d", strlen("2d")))
        {
            nRet = DISP_SetFormat(enDisp, MT_DRV_DISP_STEREO_NONE, fmtold);
        }

        if (0 == MT_OSAL_Strncmp(pArg2, "sbs_hf", strlen("sbs_hf")))
        {
            nRet = DISP_SetFormat(enDisp, MT_DRV_DISP_STEREO_SBS_HALF, fmtold);
        }

        if (0 == MT_OSAL_Strncmp(pArg2, "tab", strlen("tab")))
        {
            nRet = DISP_SetFormat(enDisp, MT_DRV_DISP_STEREO_TAB, fmtold);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "rf", strlen("rf")))
    {
        if (0 == MT_OSAL_Strncmp(pArg2, "on", strlen("on")))
        {
            nRet = DISP_SetRightEyeFirst(enDisp, MT_TRUE);
        }
        else if (0 == MT_OSAL_Strncmp(pArg2, "off", strlen("off")))
        {
            nRet = DISP_SetRightEyeFirst(enDisp, MT_FALSE);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "bright", strlen("bright")))
    {
        return MT_ERR_DISP_NOT_SUPPORT;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "contrast", strlen("contrast")))
    {
        return MT_ERR_DISP_NOT_SUPPORT;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "hue", strlen("hue")))
    {
        return MT_ERR_DISP_NOT_SUPPORT;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "satu", strlen("satu")))
    {
        return MT_ERR_DISP_NOT_SUPPORT;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "left", strlen("left")))
    {
        MT_DRV_DISP_OFFSET_S screenoffset;

        nRet = DISP_GetScreenOffset(enDisp, &screenoffset);
        if (nRet == MT_SUCCESS)
        {
            screenoffset.u32Left= (mt_u32)simple_strtol(pArg2, NULL, 10);
            nRet = DISP_SetScreenOffset(enDisp, &screenoffset);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "right", strlen("right")))
    {
        MT_DRV_DISP_OFFSET_S screenoffset;

        nRet = DISP_GetScreenOffset(enDisp, &screenoffset);
        if (nRet == MT_SUCCESS)
        {
            screenoffset.u32Right = (mt_u32)simple_strtol(pArg2, NULL, 10);
            nRet = DISP_SetScreenOffset(enDisp, &screenoffset);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "bottom", strlen("bottom")))
    {
        MT_DRV_DISP_OFFSET_S screenoffset;

        nRet = DISP_GetScreenOffset(enDisp, &screenoffset);
        if (nRet == MT_SUCCESS)
        {
            screenoffset.u32Bottom = (mt_u32)simple_strtol(pArg2, NULL, 10);
            nRet = DISP_SetScreenOffset(enDisp, &screenoffset);;
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "video", strlen("video")))
    {
        if (0 == MT_OSAL_Strncmp(pArg2, "up", strlen("up")))
        {
            nRet = DISP_SetLayerZorder(enDisp, MT_DRV_DISP_LAYER_VIDEO, MT_DRV_DISP_ZORDER_MOVEUP);
        }
        else if (0 == MT_OSAL_Strncmp(pArg2, "down", strlen("down")))
        {
            nRet = DISP_SetLayerZorder(enDisp, MT_DRV_DISP_LAYER_VIDEO, MT_DRV_DISP_ZORDER_MOVEDOWN);
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "reset", strlen("reset")))
    {

        MT_ERR_DISP("Not support set rotation now\n");
        return MT_SUCCESS;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "dac_detect", strlen("dac_detect")))
    {
        mt_u32 u32Data = (mt_u32)simple_strtol(pArg2, NULL, 10);
        if(0 == u32Data)
        {
            MT_DRV_PROC_EchoHelper("dac detect close\n");
            DISP_SetDACDetEn(MT_FALSE);
        }
        else if (1 == u32Data)
        {
            MT_DRV_PROC_EchoHelper("dac detect open\n");
            DISP_SetDACDetEn(MT_TRUE);
        }
        else  //do nothing
        {

        }

        return MT_SUCCESS;
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "cat", strlen("cat")))
    {
        MT_UCHAR *pChar = pArg2;
        MT_DRV_VIDEO_FRAME_S *pstCurFrame;
        mt_u32 pathlength = 0;

        pstCurFrame = (MT_DRV_VIDEO_FRAME_S*)DISP_MALLOC(sizeof(MT_DRV_VIDEO_FRAME_S));
        if (!pstCurFrame)
        {
            MT_ERR_DISP("alloc frame info memory failed\n");
            return nRet;
        }

        /* get currently displayed frame */
        nRet = DISP_AcquireSnapshot(enDisp, pstCurFrame, &hCommonHandle);
        if (nRet != MT_SUCCESS)
        {
            MT_ERR_DISP("catpure screen failed\n");
            DISP_FREE(pstCurFrame);
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

        DISP_ReleaseSnapshot(enDisp, pstCurFrame, hCommonHandle);

        DISP_FREE(pstCurFrame);
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "low_delay_stat", strlen("low_delay_stat")))
    {
        mt_handle hCast;
        mt_handle hCast_ptr;

        if (MT_DRV_DISPLAY_1 != enDisp)
        {
            return MT_FAILURE;
        }

        (mt_void)DispGetCastHandle(enDisp, &hCast, &hCast_ptr);
        if (0 == MT_OSAL_Strncmp(pArg2, "start", strlen("start")))
        {
            if ((hCast) &&(hCast_ptr))
            {
                MT_DRV_LD_Start_Statistics(SCENES_VID_CAST, (mt_void*)hCast);
            }
        }
        else if (0 == MT_OSAL_Strncmp(pArg2, "stop", strlen("stop")))
        {
            MT_DRV_LD_Stop_Statistics();
        }
        else
        {
            return MT_FAILURE;
        }
    }
    else if (0 == MT_OSAL_Strncmp(pArg1, "rd_door", strlen("rd_door")))
    {
        if (0 == MT_OSAL_Strncmp(pArg2, "start", strlen("start")))
        {
            g_bDispPrivateInformation = MT_TRUE;
        }
        else if (0 == MT_OSAL_Strncmp(pArg2, "stop", strlen("stop")))
        {
            g_bDispPrivateInformation = MT_FALSE;
        }
    }
    else
    {
        return MT_FAILURE;
    }
#endif
    return nRet;
}

mt_void DISP_ProcPrintHelp(mt_void)
{
/*
    DISP_DEBUGK("Please input these commands:\n"
           "echo enfromat = 0(1080P60)|1(1080P50)|2(1080p30)|3(1080p25)|4(1080p24)|\n"
                "5(1080i60)|6(1080i50)|7(720p60)|8(720p50)|9(576p50)|10(480p60)\n "
                "11(pal)|12(pal_n)|13(pal_nc)|14(ntsc)|15(ntsc_j)|16(ntsc_pal_m)\n "
                "17(secam_sin)|18(secam_cos) > /proc/msp/dispX\n"
           "echo bright = 0~100 > /proc/msp/dispxxx\n"
           "echo contrast = 0~100 > /proc/msp/dispxxx\n"
           "echo saturation = 0~100 > /proc/msp/dispxxx\n");
*/
#if 0
    MT_DRV_PROC_EchoHelper("echo help                            > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo fmt 1080i50/720p50/pal/ntsc/... > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo 3d fp/sbs_hf/tab                > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo rf on/off                       > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo bright   X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo contrast X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo hue      X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo satu     X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo left     X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo top      X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo right    X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo bottom   X                      > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo video up/down                   > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo reset                           > /proc/msp/dispX\n");
    MT_DRV_PROC_EchoHelper("echo cat AbsolutePath                > /proc/msp/dispX\n");
#endif
}

mt_char u8DispProcBuffer[256];
static mt_s32 DISP_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
#if 0
    struct seq_file   *p = file->private_data;
    mt_proc_entry_t  *pProcItem = p->private;
    mt_char *pArg1, *pArg2;
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;

    pProcItem = p->private;
    enDisp = (MT_DRV_DISPLAY_E)pProcItem->data;

    if(count >= sizeof(u8DispProcBuffer))
    {
        MT_ERR_DISP("your parameter string is too long!\n");
        return -EFAULT;
    }

    memset(u8DispProcBuffer, 0, sizeof(u8DispProcBuffer));
    if (copy_from_user(u8DispProcBuffer, buf, count))
    {
        MT_ERR_DISP("MMZ: copy_from_user failed!\n");
        return -EFAULT;
    }
    u8DispProcBuffer[count] = 0;

    nRet = DispProcParsePara(u8DispProcBuffer, &pArg1, &pArg2);
    if(  (nRet != MT_SUCCESS)
        ||(0 == MT_OSAL_Strncmp(pArg1, "help", strlen("help")))
        ||(pArg2 == MT_NULL) )
    {
        DISP_ProcPrintHelp();
        return -EFAULT;
    }

    MT_PRINT("====================echo debug disp%d\n", enDisp);
    nRet = DispProcCmdProcee(enDisp, pArg1, pArg2);
    if (nRet != MT_SUCCESS)
    {
        DISP_ProcPrintHelp();
    }
#endif
    return count;
}



/***************************************************************/
#define DEF_DRV_DISP_FILE_FUNCTION_START_FROM_HERE

mt_s32 DISP_CheckPara(MT_DRV_DISPLAY_E enDisp, DRV_DISP_STATE_S *pDispState)
{
    if ((enDisp < MT_DRV_DISPLAY_BUTT) && pDispState->bDispOpen[enDisp])
    {
        return MT_SUCCESS;
    }

    return MT_ERR_DISP_INVALID_PARA;
}

mt_s32 DISP_FileOpen(struct inode *finode, struct file  *ffile)
{
    MT_S32 mt_idx = iminor(finode);
    disp_priv_data *disp_priv_data = get_mt_priv(mt_idx);
    DRV_DISP_STATE_S *pDispState = MT_NULL;
    MT_DRV_DISPLAY_E u;
    mt_s32 Ret;
    DISP_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);
    if (!IS_ERR_OR_NULL(disp_priv_data->disclk))
        clk_prepare_enable(disp_priv_data->disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->diclk))
        clk_prepare_enable(disp_priv_data->diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->osdclk))
        clk_prepare_enable(disp_priv_data->osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->presclk))
        clk_prepare_enable(disp_priv_data->presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->disaxiclk))
        clk_prepare_enable(disp_priv_data->disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->hdclk))
        clk_prepare_enable(disp_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->sdclk_27m))
        clk_prepare_enable(disp_priv_data->sdclk_27m);

    Ret = down_interruptible(&g_DispMutex);

    pDispState = MT_KMALLOC(MT_ID_DISP, sizeof(DRV_DISP_STATE_S), GFP_KERNEL);
    if (!pDispState)
    {
        MT_FATAL_DISP("malloc pDispState failed.\n");
        up(&g_DispMutex);
        return -1;
    }

    if (1 == atomic_inc_return(&g_DispCount))
    {
        /* for configuration such as start clock, pins re-use, etc  */
        Ret = DISP_Init();
        if (Ret != MT_SUCCESS)
        {
            MT_KFREE(MT_ID_DISP, pDispState);
            MT_FATAL_DISP("call DISP_Init failed.\n");
            atomic_dec(&g_DispCount);
            up(&g_DispMutex);
            return -1;
        }
    }

    if (1 == atomic_inc_return(&g_DispLayerCount))
    {
        DISP_Close_AllLayer();
    }

    for(u=0; u<MT_DRV_DISPLAY_BUTT; u++)
    {
        pDispState->bDispOpen[u] = MT_FALSE;
        pDispState->hCastHandle[u] = MT_NULL;
        pDispState->hSnapshot[u] = MT_NULL;
    }

    ffile->private_data = pDispState;

    up(&g_DispMutex);
    return 0;
}


mt_s32 DISP_FileClose(struct inode *finode, struct file  *ffile)
{
    MT_S32 mt_idx = iminor(finode);
    disp_priv_data *disp_priv_data = get_mt_priv(mt_idx);
    DRV_DISP_STATE_S *pDispState;
    MT_DRV_DISPLAY_E u;
    mt_s32 Ret;
    DISP_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);

    Ret = down_interruptible(&g_DispMutex);

    pDispState = ffile->private_data;

    for(u=0; u<MT_DRV_DISPLAY_BUTT; u++)
    {
        /*to close cast service, no matter disp open or not.*/
        if (pDispState->hCastHandle[u])
        {
            Ret = DISP_DestroyCast(pDispState->hCastHandle[u]);
            if (Ret != MT_SUCCESS)
                MT_ERR_DISP("destroy cast  %d  failed!\n", u);
        }

        /*to close snapshot service, for ctrl+c condition.*/
        if (pDispState->hSnapshot[u])
        {
            Ret = DISP_DestroySnapshot(pDispState->hSnapshot[u]);
            if (Ret != MT_SUCCESS)
                MT_ERR_DISP("destroy cast  %d  failed!\n", u);
        }

        if (pDispState->bDispOpen[u])
        {
            Ret = DISP_ExtClose(u, pDispState, MT_TRUE);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_DISP("Display %d close failed!\n", u);
            }
        }
    }

    if (atomic_dec_and_test(&g_DispCount))
    {
        /* for close of clock */
        DISP_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);
        DISP_DeInit();

        //DEBUG_PRINTK("DISP_FileClose DISP_DeInit\n");

        // add for multiple process
        g_s32DispAttachCount = 0;
    }

    MT_KFREE(MT_ID_DISP, ffile->private_data);

    up(&g_DispMutex);

    if (!IS_ERR_OR_NULL(disp_priv_data->disclk))
        clk_disable_unprepare(disp_priv_data->disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->diclk))
        clk_disable_unprepare(disp_priv_data->diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->osdclk))
        clk_disable_unprepare(disp_priv_data->osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->presclk))
        clk_disable_unprepare(disp_priv_data->presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->disaxiclk))
        clk_disable_unprepare(disp_priv_data->disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->hdclk))
        clk_disable_unprepare(disp_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->sdclk_27m))
        clk_disable_unprepare(disp_priv_data->sdclk_27m);

    return 0;
}


mt_s32 DRV_DISP_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    DRV_DISP_STATE_S *pDispState;
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    pDispState = file->private_data;

    Ret = DRV_DISP_ProcessCmd(cmd, arg, pDispState, MT_TRUE);

    up(&g_DispMutex);
    return Ret;
}


/***************************************************************/
#define DEF_DRV_DISP_DRV_FUNCTION_START_FROM_HERE

mt_s32 MT_DRV_DISP_Process(mt_u32 cmd, mt_void *arg)
{
    DRV_DISP_STATE_S    *pDispState;
    mt_s32          Ret;

    Ret = down_interruptible(&g_DispMutex);

    pDispState = &g_DispModState;

    Ret = DRV_DISP_ProcessCmd(cmd, arg, pDispState, MT_FALSE);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_Process_Intr(mt_u32 cmd, mt_void *arg)
{
    DRV_DISP_STATE_S    *pDispState;
    mt_s32          Ret;
    pDispState = &g_DispModState;
    Ret = DRV_DISP_ProcessCmd(cmd, arg, pDispState, MT_FALSE);
    return Ret;
}

mt_u32 DISP_ResetCountStatus(void)
{
    MT_DRV_DISPLAY_E u;

    for(u=0; u<MT_DRV_DISPLAY_BUTT; u++)
    {
        g_DispAllCountState.DispOpenNum[u] = 0;
        g_DispUserCountState.DispOpenNum[u] = 0;
        g_DispKernelCountState.DispOpenNum[u] = 0;
        g_DispModState.bDispOpen[u] = MT_FALSE;
        g_DispModState.hCastHandle[u] = MT_NULL;
        g_DispModState.hSnapshot[u] = MT_NULL;
    }

    g_DispSuspend = MT_FALSE;
    g_s32DispAttachCount = 0;

    return MT_SUCCESS;
}

mt_u32 DISP_Get_CountStatus(void)
{
    MT_DRV_DISPLAY_E u;
    for(u=0; u<MT_DRV_DISPLAY_BUTT; u++)
    {
        if (g_DispAllCountState.DispOpenNum[u] > 0)
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

mt_s32 DISP_ExtOpen(MT_DRV_DISPLAY_E enDisp, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser)
{
    mt_s32            Ret;

    /* create DISP for the first time */
    DISP_DEBUGK("%s %d endisp %d pDispState0x%lx\n",__FUNCTION__, __LINE__,enDisp,(ulong)pDispState);

    if (!pDispState->bDispOpen[enDisp])
    {
        /* call basic interface for the first time creating DISP globally*/
        if (!g_DispAllCountState.DispOpenNum[enDisp])
        {
            Ret = DISP_Open(enDisp);
            if (Ret != MT_SUCCESS)
            {
                MT_ERR_DISP(" Error number is: %d.\n", Ret);
                return Ret;
            }
        }
        DISP_DEBUGK("%s %d endisp %d pDispState0x%lx\n",__FUNCTION__, __LINE__,enDisp,(ulong)pDispState);


        pDispState->bDispOpen[enDisp] = MT_TRUE;

        g_DispAllCountState.DispOpenNum[enDisp]++;

        if (MT_TRUE == bUser)
        {
            g_DispUserCountState.DispOpenNum[enDisp]++;
        }
        else
        {
            g_DispKernelCountState.DispOpenNum[enDisp]++;
        }
    }

    return MT_SUCCESS;
}

mt_s32 DISP_ExtClose(MT_DRV_DISPLAY_E enDisp, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser)
{
    mt_s32       Ret = MT_SUCCESS;

    /* destroy DISP */
    if (pDispState->bDispOpen[enDisp])
    {
        if (MT_TRUE == bUser)
        {
            if (g_DispUserCountState.DispOpenNum[enDisp] == 0)
            {
                MT_WARN_DISP("Already Close User display%d =0\n", enDisp);
                return 0;
            }

            g_DispUserCountState.DispOpenNum[enDisp]--;  /* User count --   */
        }
        else
        {
            if (g_DispKernelCountState.DispOpenNum[enDisp] == 0)
            {
            MT_WARN_DISP("Already Close kernel display%d =0\n", enDisp);
            return 0;
            }
            g_DispKernelCountState.DispOpenNum[enDisp]--;

        }

        g_DispAllCountState.DispOpenNum[enDisp]--;  /* Global count -- */

        if (!g_DispAllCountState.DispOpenNum[enDisp])
        {
            Ret = DISP_Close(enDisp);
            if (Ret != MT_SUCCESS)
            {
                MT_FATAL_DISP("call DISP_Close failed.\n");
            }

            g_s32DispAttachCount = 0;
        }

        pDispState->bDispOpen[enDisp] = MT_FALSE;
    }

    return Ret;
}


mt_s32 DISP_ExtAttach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32 nRet = MT_SUCCESS;

    if ( (enMaster != MT_DRV_DISPLAY_1) || (enSlave != MT_DRV_DISPLAY_0))
    {
        MT_FATAL_DISP("Attach parameters invalid.\n");
        return MT_ERR_DISP_INVALID_OPT;
    }

    if(   (0 == g_DispAllCountState.DispOpenNum[enMaster])
        &&(0 == g_DispAllCountState.DispOpenNum[enSlave]) )

    {
        nRet = DISP_Attach(enMaster, enSlave);
    }

    return nRet;
}

mt_s32 DISP_ExtDetach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32 nRet = MT_SUCCESS;

    if ( (enMaster != MT_DRV_DISPLAY_1) || (enSlave != MT_DRV_DISPLAY_0))
    {
        MT_FATAL_DISP("Attach parameters invalid.\n");
        return MT_ERR_DISP_INVALID_OPT;
    }

    if(   (0 == g_DispAllCountState.DispOpenNum[enMaster])
        &&(0 == g_DispAllCountState.DispOpenNum[enSlave]) )

    {
        nRet = DISP_Detach(enMaster, enSlave);
       DISP_DEBUGK(" DISP_ExtDetach  003 = 0x%x %s\n", nRet,__FUNCTION__);
    }

    return nRet;
}

//DIPS_DEBUG : change order
mt_s32 MT_DRV_DISP_Open(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32 Ret;

    Ret = MT_DRV_DISP_Process(CMD_DISP_OPEN, &enDisp);
    return Ret;
}

mt_s32 MT_DRV_DISP_Close(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32 Ret;

    Ret = MT_DRV_DISP_Process(CMD_DISP_CLOSE, &enDisp);
    return Ret;
}

mt_s32 MT_DRV_DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E enFormat)
{
    mt_s32 Ret;
    DISP_FORMAT_S  enDispFormat;

    enDispFormat.enDisp = enDisp;
    enDispFormat.enFormat = enFormat;
    enDispFormat.enStereo = MT_DRV_DISP_STEREO_NONE;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_FORMAT,  &enDispFormat);
    return Ret;
}

/* DRV_DISP_XXX */
mt_s32 MT_DRV_DISP_Attach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32 Ret;
    DISP_ATTACH_S  enDispAttach;

    enDispAttach.enMaster = enMaster;
    enDispAttach.enSlave  = enSlave;
    Ret = MT_DRV_DISP_Process(CMD_DISP_ATTACH, &enDispAttach);
    return Ret;
}

mt_s32 MT_DRV_DISP_Detach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    mt_s32 Ret;
    DISP_ATTACH_S  enDispAttach;

    enDispAttach.enMaster = enMaster;
    enDispAttach.enSlave  = enSlave;
    Ret = MT_DRV_DISP_Process(CMD_DISP_DETACH, &enDispAttach);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E *penFormat)
{
    mt_s32 Ret;
    DISP_FORMAT_S  enDispFormat;

    if (!penFormat)
    {
        return MT_ERR_DISP_NULL_PTR;
    }
    memset(&enDispFormat, 0, sizeof(DISP_FORMAT_S));
    enDispFormat.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_FORMAT,  &enDispFormat);
    if (!Ret)
    {
        *penFormat = enDispFormat.enFormat;
    }
    return Ret;
}

mt_s32 MT_DRV_DISP_SetCustomTiming(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming)
{
    mt_s32 Ret;
    DISP_TIMING_S  DispTiming;
    DispTiming.enDisp = enDisp;
    memcpy(&DispTiming.stTimingPara, pstTiming, sizeof(MT_DRV_DISP_TIMING_S));
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_TIMING, &DispTiming);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetCustomTiming(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_TIMING_S *pstTiming)
{
    mt_s32 Ret;
    DISP_TIMING_S  DispTiming;
    memset(&DispTiming, 0, sizeof(DISP_TIMING_S));
    DispTiming.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_TIMING, &DispTiming);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }
    memcpy(pstTiming, &DispTiming.stTimingPara, sizeof(MT_DRV_DISP_TIMING_S));
    return Ret;
}

mt_s32 MT_DRV_DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
    mt_s32          Ret;
    DISP_SET_INTF_S DispIntf;

    DispIntf.enDisp = enDisp;

    memcpy(&DispIntf.stIntf, pstIntf, sizeof(MT_DRV_DISP_INTF_S));

    Ret = MT_DRV_DISP_Process(CMD_DISP_ADD_INTF, &DispIntf);
    return Ret;
}

mt_s32 MT_DRV_DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
    mt_s32          Ret;
    DISP_SET_INTF_S DispIntf;

    DispIntf.enDisp = enDisp;

    memcpy(&DispIntf.stIntf, pstIntf, sizeof(MT_DRV_DISP_INTF_S));

    Ret = MT_DRV_DISP_Process(CMD_DISP_DEL_INTF, &DispIntf);
    return Ret;
}

mt_s32 DRV_DISP_AttachVDAC(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_ID_E eVdacId, MT_DRV_DISP_VDAC_SIGNAL_E eSignal)
{
    return MT_ERR_DISP_NOT_SUPPORT;
}

mt_s32 MT_DRV_DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    mt_s32 Ret;
    DISP_ENABLE_S  stDispEnable;

    stDispEnable.bEnable = bEnable;
    stDispEnable.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_ENABLE, &stDispEnable);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable)
{
    mt_s32 Ret;
    DISP_ENABLE_S  stDispEnable;

    memset(&stDispEnable, 0, sizeof(DISP_ENABLE_S));
    stDispEnable.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_ENABLE, &stDispEnable);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }
    *pbEnable = stDispEnable.bEnable;
    return Ret;
}

mt_s32 MT_DRV_DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    mt_s32 Ret;
    DISP_R_EYE_FIRST_S  stREFirst;

    stREFirst.bREFirst = bEnable;
    stREFirst.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_R_E_FIRST, &stREFirst);
    return Ret;
}

mt_s32 MT_DRV_DISP_SetBgColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor)
{
    mt_s32 Ret;
    DISP_BGC_S  stDispBgc;

    stDispBgc.stBgColor = *pstBgColor;
    stDispBgc.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_BGC, &stDispBgc);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetBgColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBgColor)
{
    mt_s32 Ret;
    DISP_BGC_S  stDispBgc;

    memset(&stDispBgc, 0, sizeof(DISP_BGC_S));
    stDispBgc.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_BGC, &stDispBgc);
    if (MT_SUCCESS != Ret)
    {
        return Ret;
    }

    *pstBgColor = stDispBgc.stBgColor ;
    return Ret;
}

mt_s32 MT_DRV_DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v)
{
    mt_s32 Ret;
    DISP_ASPECT_RATIO_S stDispRatio;

    stDispRatio.enDisp = enDisp;
    stDispRatio.u32ARHori = u32Ratio_h;
    stDispRatio.u32ARVert = u32Ratio_v;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_DEV_RATIO, &stDispRatio);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v)
{
    mt_s32 Ret;
    DISP_ASPECT_RATIO_S stDispRatio;

    memset(&stDispRatio, 0, sizeof(DISP_ASPECT_RATIO_S));
    stDispRatio.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_DEV_RATIO, &stDispRatio);
    if(!Ret)
    {
        *pu32Ratio_h = stDispRatio.u32ARHori;
        *pu32Ratio_v = stDispRatio.u32ARVert;
    }
    return Ret;
}

mt_s32 MT_DRV_DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag)
{
    mt_s32 Ret;
    DISP_ZORDER_S stDispZorder;

    stDispZorder.enDisp = enDisp;
    stDispZorder.ZFlag  = enZFlag;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_ZORDER, &stDispZorder);
    return Ret;
}

mt_s32 MT_DRV_DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder)
{
    mt_s32 Ret;
    DISP_ZORDER_S stDispZorder;

    memset(&stDispZorder, 0, sizeof(DISP_ZORDER_S));
    stDispZorder.enDisp = enDisp;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_ZORDER, &stDispZorder);
    *pu32Zorder = stDispZorder.ZFlag;
    return Ret;
}

mt_s32 MT_DRV_DISP_CreateCast (MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S * pstCfg, mt_handle *phCast)
{
    mt_s32 Ret;
    DISP_CAST_CREATE_S stCastCreate;

    stCastCreate.enDisp = enDisp;
    stCastCreate.hCast = *phCast;
    stCastCreate.stCfg = *pstCfg;
    Ret = MT_DRV_DISP_Process(CMD_DISP_CREATE_CAST, &stCastCreate);
    return Ret;
}

mt_s32  MT_DRV_DISP_DestroyCast(mt_handle hCast)
{
    mt_s32 Ret;
    DISP_CAST_DESTROY_S stCastDestroy;

    stCastDestroy.hCast = hCast;
    Ret = MT_DRV_DISP_Process(CMD_DISP_DESTROY_CAST, &stCastDestroy);
    return Ret;
}

mt_s32 MT_DRV_DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
    mt_s32 Ret;
    DISP_CAST_ENABLE_S stCastEnable;

    stCastEnable.hCast = hCast;
    stCastEnable.bEnable = bEnable;
    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_CAST_ENABLE, &stCastEnable);
    return Ret;
}

mt_s32 MT_DRV_DISP_SetLowDelayEnable(mt_handle hCast, MT_BOOL bEnable)
{
   mt_s32 Ret = MT_FAILURE;
   mt_u32 index = Win_GetVideoWinIndex();
   if(index < MAX_WIN_NUM)
   {
        Ret = DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_SET_LOW_DELAY_MODE, (mt_void*)((ulong)bEnable));
   }
   
   return Ret;
}
mt_s32  MT_DRV_DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    mt_s32 Ret;
    DISP_CAST_ENABLE_S stCastEnable;

    memset(&stCastEnable, 0, sizeof(DISP_CAST_ENABLE_S));
    stCastEnable.hCast = hCast;
    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_CAST_ENABLE, &stCastEnable);
    if(!Ret)
    {
        *pbEnable = stCastEnable.bEnable;
    }
    return Ret ;
}

mt_s32  MT_DRV_DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    mt_s32 Ret;
    DISP_CAST_FRAME_S stCastFrame;

    memset(&stCastFrame, 0, sizeof(DISP_CAST_FRAME_S));
    stCastFrame.hCast = hCast;
    Ret = MT_DRV_DISP_Process(CMD_DISP_ACQUIRE_CAST_FRAME, &stCastFrame);
    if(!Ret)
    {
        *pstCastFrame = stCastFrame.stFrame;
    }
    return Ret ;
}

mt_s32 MT_DRV_DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    mt_s32 Ret;
    DISP_CAST_FRAME_S stCastFrame;

    stCastFrame.hCast = hCast;
    stCastFrame.stFrame = *pstCastFrame;

    Ret = MT_DRV_DISP_Process_Intr(CMD_DISP_RELEASE_CAST_FRAME, &stCastFrame);

    return Ret ;
}

mt_s32 MT_DRV_DISP_ExternlAttach(mt_handle hCast, mt_handle hSink)
{
    mt_s32 Ret;
    DISP_EXT_ATTACH_S disp_attach_info;

    disp_attach_info.hCast = hCast;
    disp_attach_info.hMutual = hSink;
    disp_attach_info.enType = EXT_ATTACH_TYPE_SINK;

    Ret = MT_DRV_DISP_Process(CMD_DISP_EXT_ATTACH, &disp_attach_info);

    return Ret ;
}

mt_s32 MT_DRV_DISP_SetCastAttr(mt_handle hCast, mt_u32 u32Width,mt_u32 u32Height)
{
    mt_s32 Ret;
    DISP_CAST_EXT_ATTR_S disp_cast_attr;

    disp_cast_attr.hCast = hCast;
    disp_cast_attr.castAttr.s32Width =  u32Width;
    disp_cast_attr.castAttr.s32Height = u32Height;

    Ret = MT_DRV_DISP_Process(CMD_DISP_SET_CASTATTR, &disp_cast_attr);
    return Ret ;
}

mt_s32 MT_DRV_DISP_GetCastAttr(mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *pstCastAttr)
{
    mt_s32 Ret;
    DISP_CAST_EXT_ATTR_S disp_cast_attr;
    memset((void*)&disp_cast_attr, 0, sizeof(DISP_CAST_EXT_ATTR_S));

    disp_cast_attr.hCast = hCast;

    Ret = MT_DRV_DISP_Process(CMD_DISP_GET_CASTATTR, &disp_cast_attr);
    if (MT_FAILURE ==  Ret)
        return  Ret;

    *pstCastAttr = disp_cast_attr.castAttr;
    return Ret ;
}

mt_s32 MT_DRV_DISP_ExternlDetach(mt_handle hCast, mt_handle hSink)
{
    mt_s32 Ret;
    DISP_EXT_ATTACH_S disp_attach_info;

    disp_attach_info.hCast = hCast;
    disp_attach_info.hMutual = hSink;
    disp_attach_info.enType = EXT_ATTACH_TYPE_SINK;

    Ret = MT_DRV_DISP_Process(CMD_DISP_EXT_DEATTACH, &disp_attach_info);

    return Ret ;
}

mt_s32 MT_DRV_DISP_GetInitFlag(MT_BOOL *pbInited)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_GetInitFlag(pbInited);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_GetVersion(MT_DRV_DISP_VERSION_S *pstVersion)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_GetVersion(pstVersion);

    up(&g_DispMutex);

    return Ret;
}

MT_BOOL MT_DRV_DISP_IsOpened(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_IsOpened(enDisp);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_GetSlave(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penSlave)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_GetSlave(enDisp, penSlave);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_GetMaster(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penMaster)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_GetMaster(enDisp, penMaster);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_GetDisplayInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);

    Ret = DISP_GetDisplayInfo(enDisp, pstInfo);

    up(&g_DispMutex);

    return Ret;
}

mt_s32 MT_DRV_DISP_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                            MT_DRV_DISP_CALLBACK_S *pstCallback)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);
    Ret = DISP_RegCallback(enDisp, eType, pstCallback);
    up(&g_DispMutex);
    return Ret;
}

mt_s32 MT_DRV_DISP_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                              MT_DRV_DISP_CALLBACK_S *pstCallback)
{
    mt_s32 Ret;

    Ret = down_interruptible(&g_DispMutex);
    Ret = DISP_UnRegCallback(enDisp, eType, pstCallback);
    up(&g_DispMutex);
    return Ret;
}

mt_s32 DRV_DISP_Suspend(basedev_s *pdev, pm_message_t state)
{
    disp_priv_data *disp_priv_data = dev_get_platdata(&pdev->dev);
#if 0
    mt_s32 Ret;

    HDMI_EXPORT_FUNC_S* pstHDMIFunc = MT_NULL;

    Ret = MT_DRV_MODULE_GetFunction(MT_ID_HDMI, (mt_void**)&pstHDMIFunc);

    if ((Ret != MT_SUCCESS) || (pstHDMIFunc == MT_NULL))
    {
        DISP_ERROR("DISP_get HDMI func failed!");
        return MT_FAILURE;
    }

    pstHDMIFunc->pfnHdmiSuspend(NULL,state);


    Ret = down_trylock(&g_DispMutex);
    if (Ret)
    {
        MT_FATAL_DISP("down g_DispMutex failed.\n");
        return -1;
    }

    /* no process opened the equipment, return directly */
    if (!atomic_read(&g_DispCount))
    {
        up(&g_DispMutex);
        return 0;
    }

    DISP_Suspend();

    msleep(50);

    g_DispSuspend = MT_TRUE;

    MT_PRINT("DISP suspend OK\n");

    up(&g_DispMutex);
#endif

    DISP_Suspend();

    if (!IS_ERR_OR_NULL(disp_priv_data->disclk))
        clk_disable_unprepare(disp_priv_data->disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->diclk))
        clk_disable_unprepare(disp_priv_data->diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->osdclk))
        clk_disable_unprepare(disp_priv_data->osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->presclk))
        clk_disable_unprepare(disp_priv_data->presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->disaxiclk))
        clk_disable_unprepare(disp_priv_data->disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->hdclk))
        clk_disable_unprepare(disp_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->sdclk_27m))
        clk_disable_unprepare(disp_priv_data->sdclk_27m);

return 0;
}

mt_s32 DRV_DISP_Resume(basedev_s *pdev)
{
    disp_priv_data *disp_priv_data = dev_get_platdata(&pdev->dev);

    if (!IS_ERR_OR_NULL(disp_priv_data->disclk))
        clk_prepare_enable(disp_priv_data->disclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->diclk))
        clk_prepare_enable(disp_priv_data->diclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->osdclk))
        clk_prepare_enable(disp_priv_data->osdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->presclk))
        clk_prepare_enable(disp_priv_data->presclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->disaxiclk))
        clk_prepare_enable(disp_priv_data->disaxiclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->hdclk))
        clk_prepare_enable(disp_priv_data->hdclk);
    if (!IS_ERR_OR_NULL(disp_priv_data->sdclk_27m))
        clk_prepare_enable(disp_priv_data->sdclk_27m);

    DISP_Resume();
#if 0
    mt_s32  Ret;
    MT_DRV_DISP_INTF_S stIntf;
    MT_DRV_DISP_FMT_E enFormat;
    MT_DRV_DISP_STEREO_E eDispMode;

    HDMI_EXPORT_FUNC_S* pstHDMIFunc = MT_NULL;

    Ret = down_trylock(&g_DispMutex);
    if (Ret)
    {
        MT_FATAL_DISP("down g_DispMutex failed.\n");
        return -1;
    }

    /* no process opened the equipment, return directly */
    if (!atomic_read(&g_DispCount))
    {
        up(&g_DispMutex);
        return 0;
    }

    DISP_Resume();

    g_DispSuspend = MT_FALSE;

    MT_PRINT("DISP resume OK\n");

    up(&g_DispMutex);

    Ret = MT_DRV_MODULE_GetFunction(MT_ID_HDMI, (mt_void**)&pstHDMIFunc);

    if ((Ret != MT_SUCCESS) || (pstHDMIFunc == MT_NULL))
    {
        DISP_ERROR("DISP_get HDMI func failed!");
        return MT_FAILURE;
    }

    if (Disp_GetFastbootupFlag() == DISP_FASTBOOTUP_FLAG)
    {
        stIntf.eID = MT_DRV_DISP_INTF_HDMI0;

        if (DispCheckIntfExistByType(MT_DRV_DISPLAY_0, &stIntf))
        {
            DISP_GetDisplaySetting(MT_DRV_DISPLAY_0, &enFormat,&eDispMode);
        }
        else if (DispCheckIntfExistByType(MT_DRV_DISPLAY_1, &stIntf))
        {
            DISP_GetDisplaySetting(MT_DRV_DISPLAY_1, &enFormat,&eDispMode);
        }

        pstHDMIFunc->pfnHdmiSoftResume(enFormat, eDispMode);
        Disp_SetFastbootupFlag(0);
    }
    else
    {
        pstHDMIFunc->pfnHdmiResume(NULL);
    }

 #endif
    return 0;
}

mt_s32 DRV_DISP_ProcessCmd(unsigned int cmd, mt_void *arg, DRV_DISP_STATE_S *pDispState, MT_BOOL bUser)
{
    mt_s32       Ret = MT_FAILURE;

    if(cmd != CMD_DISP_SEND_VBI)
        DISP_DEBUGK("ioctrl_ cmd %x  %s\n",cmd,__FUNCTION__);

    switch (cmd)
    {
        case CMD_DISP_ATTACH:
            {
                DISP_ATTACH_S  *pDispAttach;

                pDispAttach = (DISP_ATTACH_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_ATTACH enMaster:%d enSlave:%d\n", __FUNCTION__, __LINE__,
                    pDispAttach->enMaster, pDispAttach->enSlave);

                Ret = DISP_ExtAttach(pDispAttach->enMaster, pDispAttach->enSlave);

                break;
            }

        case CMD_DISP_DETACH:
            {
                DISP_ATTACH_S  *pDispAttach;

                pDispAttach = (DISP_ATTACH_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_DETACH enMaster:%d enSlave:%d\n", __FUNCTION__, __LINE__,
                    pDispAttach->enMaster, pDispAttach->enSlave);


                Ret = DISP_ExtDetach(pDispAttach->enMaster, pDispAttach->enSlave);

                break;
            }

        case CMD_DISP_OPEN:
            {
                DISP_DEBUGK("%s %d CMD_DISP_OPEN bUser:%d\n", __FUNCTION__, __LINE__, bUser);            
                Ret = DISP_ExtOpen(*((MT_DRV_DISPLAY_E *)arg), pDispState, bUser);

                break;
            }

        case CMD_DISP_CLOSE:
            {
                DISP_DEBUGK("%s %d CMD_DISP_CLOSE bUser:%d\n", __FUNCTION__, __LINE__, bUser);  
                Ret = DISP_CheckPara(*((MT_DRV_DISPLAY_E *)arg), pDispState);
                if (MT_SUCCESS == Ret)
                {
                    Ret = DISP_ExtClose(*((MT_DRV_DISPLAY_E *)arg), pDispState, bUser);
                }
                else
                {
                    Ret = MT_SUCCESS;
                }

                break;
            }
#if 0
        case CMD_DISP_AttachOsd:
            {
                DISP_OSD_S  *pDispOsd;

                pDispOsd = (DISP_OSD_S *)arg;

                Ret = DISP_AttachOsd(pDispOsd->enDisp, pDispOsd->enLayer);

                break;
            }

        case CMD_DISP_DetachOsd:
            {
                DISP_OSD_S  *pDispOsd;

                pDispOsd = (DISP_OSD_S *)arg;

                Ret = DISP_DetachOsd(pDispOsd->enDisp, pDispOsd->enLayer);

                break;
            }
#endif
        case CMD_DISP_SET_ENABLE:
            {
                DISP_ENABLE_S  *pDispEnable;

                pDispEnable = (DISP_ENABLE_S *)arg;

                Ret = DISP_CheckPara(pDispEnable->enDisp, pDispState);

                DISP_DEBUGK("%s %d CMD_DISP_SET_ENABLE enDisp:%d: bEnable:%d\n", __FUNCTION__, __LINE__, 
                    pDispEnable->enDisp, pDispEnable->bEnable);  
                if (MT_SUCCESS == Ret)
                {
                    Ret = DISP_ExternSetEnable(pDispEnable->enDisp, pDispEnable->bEnable);
                }

                break;
            }

        case CMD_DISP_GET_ENABLE:
            {
                DISP_ENABLE_S  *pDispEnable;

                pDispEnable = (DISP_ENABLE_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_ENABLE\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetEnable(pDispEnable->enDisp, &pDispEnable->bEnable);

                break;
            }

        case CMD_DISP_ADD_INTF:
            {
                DISP_SET_INTF_S  *pDispIntf;

                pDispIntf = (DISP_SET_INTF_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_ADD_INTF\n", __FUNCTION__, __LINE__);

                Ret = DISP_AddIntf(pDispIntf->enDisp, &pDispIntf->stIntf);

                break;
            }
        case CMD_DISP_DEL_INTF:
            {
                DISP_SET_INTF_S  *pDispIntf;
                DISP_DEBUGK("%s %d CMD_DISP_DEL_INTF\n", __FUNCTION__, __LINE__);

                pDispIntf = (DISP_SET_INTF_S *)arg;
                Ret = DISP_DelIntf(pDispIntf->enDisp, &pDispIntf->stIntf);

                break;

            }

#if 0
        case CMD_DISP_GET_INTF:
            {
                DISP_GET_INTF_S  *pDispIntf;

                pDispIntf = (DISP_GET_INTF_S *)arg;

                Ret = DISP_GetIntf(pDispIntf->enDisp, &(pDispIntf->u32IntfNum), (pDispIntf->stIntf));

                break;
            }
#endif
        case CMD_DISP_SET_FORMAT:
            {
                DISP_FORMAT_S  *pDispFormat;

                pDispFormat = (DISP_FORMAT_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_FORMAT enStereo:%d enFormat:%d\n", __FUNCTION__, __LINE__, 
                    pDispFormat->enStereo, pDispFormat->enFormat);

                Ret = DISP_SetFormat(pDispFormat->enDisp, pDispFormat->enStereo, pDispFormat->enFormat);

                break;
            }

        case CMD_DISP_GET_FORMAT:
            {
                DISP_FORMAT_S  *pDispFormat;

                pDispFormat = (DISP_FORMAT_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_FORMAT\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetFormat(pDispFormat->enDisp, &pDispFormat->enStereo,&pDispFormat->enFormat);

                break;
            }

        case CMD_DISP_SET_R_E_FIRST:
            {
                DISP_R_EYE_FIRST_S  *pREFirst;

                pREFirst = (DISP_R_EYE_FIRST_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_R_E_FIRST bREFirst:%d\n", __FUNCTION__, __LINE__, pREFirst->bREFirst);

                Ret = DISP_SetRightEyeFirst(pREFirst->enDisp, pREFirst->bREFirst);

                break;
            }
        case CMD_DISP_SET_VIRTSCREEN:
            {
                DISP_VIRTSCREEN_S  pVirtScreen;
                pVirtScreen = *((DISP_VIRTSCREEN_S *)arg);
                DISP_DEBUGK("%s %d CMD_DISP_SET_VIRTSCREEN\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetVirtScreen(pVirtScreen.enDisp, pVirtScreen.stVirtScreen);

                break;
            }
        case CMD_DISP_GET_VIRTSCREEN:
            {
                DISP_VIRTSCREEN_S  *pVirtScreen;
                pVirtScreen = (DISP_VIRTSCREEN_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_VIRTSCREEN\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetVirtScreen(pVirtScreen->enDisp, &pVirtScreen->stVirtScreen);
                break;
            }
        case CMD_DISP_SET_SMALLWINDOW:
            {
                DISP_VIRTSCREEN_S  pVirtScreen;
                pVirtScreen = *((DISP_VIRTSCREEN_S *)arg);
                DISP_DEBUGK("%s %d CMD_DISP_SET_SMALLWINDOW\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetSmallWindow(pVirtScreen.enDisp, pVirtScreen.stVirtScreen);
                break;
            }
        case CMD_DISP_GET_SMALLWINDOW:
            {
                DISP_VIRTSCREEN_S  *pVirtScreen;
                pVirtScreen = (DISP_VIRTSCREEN_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_SMALLWINDOW\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetSmallWindow(pVirtScreen->enDisp, &pVirtScreen->stVirtScreen);
                break;
            }
        case CMD_DISP_SET_SCREENOFFSET:
            {
                DISP_SCREENOFFSET_S  *pOffset;
                pOffset = (DISP_SCREENOFFSET_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_SCREENOFFSET\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetScreenOffset(pOffset->enDisp, &pOffset->stScreenOffset);
                break;
            }
        case CMD_DISP_GET_SCREENOFFSET:
            {
                DISP_SCREENOFFSET_S  *pOffset;
                pOffset = (DISP_SCREENOFFSET_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_SCREENOFFSET\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetScreenOffset(pOffset->enDisp, &pOffset->stScreenOffset);
                break;
            }

        case CMD_DISP_SET_TIMING:
            {
                DISP_TIMING_S  *pDispTiming;

                pDispTiming = (DISP_TIMING_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_TIMING\n", __FUNCTION__, __LINE__);
                Ret = DISP_SetCustomTiming(pDispTiming->enDisp, &pDispTiming->stTimingPara);
                break;
            }
        case CMD_DISP_GET_TIMING:
            {
                DISP_TIMING_S  *pDispTiming;
                pDispTiming = (DISP_TIMING_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_TIMING\n", __FUNCTION__, __LINE__);
                Ret = DISP_GetCustomTiming(pDispTiming->enDisp, &pDispTiming->stTimingPara);
                break;
            }

        case CMD_DISP_SET_ZORDER:
            {
                DISP_ZORDER_S   *pDispZorder;

                pDispZorder = (DISP_ZORDER_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_ZORDER\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetLayerZorder(pDispZorder->enDisp,  pDispZorder->ZFlag);

                break;
            }

        case CMD_DISP_GET_ORDER:
            {
                DISP_ORDER_S    *pDispOrder;

                pDispOrder = (DISP_ORDER_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_ORDER\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetLayerZorder(pDispOrder->enDisp, &pDispOrder->Order);

                break;
            }
        case CMD_DISP_SET_DEV_RATIO:
            {
                DISP_ASPECT_RATIO_S *pDispAspectRatio;

                pDispAspectRatio = (DISP_ASPECT_RATIO_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_DEV_RATIO u32ARHori:%d u32ARVert:%d\n", __FUNCTION__, __LINE__,
                     pDispAspectRatio->u32ARHori, pDispAspectRatio->u32ARVert);
                Ret = DISP_SetAspectRatio(pDispAspectRatio->enDisp, pDispAspectRatio->u32ARHori, pDispAspectRatio->u32ARVert);

                break;
            }
        case CMD_DISP_GET_DEV_RATIO:
            {
                DISP_ASPECT_RATIO_S *pDispAspectRatio;

                pDispAspectRatio = (DISP_ASPECT_RATIO_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_DEV_RATIO\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetAspectRatio(pDispAspectRatio->enDisp, &pDispAspectRatio->u32ARHori, &pDispAspectRatio->u32ARVert);
                break;
            }
        case CMD_DISP_SET_BGC:
            {
                DISP_BGC_S  *pDispBgc;

                pDispBgc = (DISP_BGC_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_BGC\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetBGColor(pDispBgc->enDisp, &pDispBgc->stBgColor);

                break;
            }

        case CMD_DISP_GET_BGC:
            {
                DISP_BGC_S  *pDispBgc;

                pDispBgc = (DISP_BGC_S *)arg;

                Ret = DISP_GetBGColor(pDispBgc->enDisp, &pDispBgc->stBgColor);
                DISP_DEBUGK("%s %d CMD_DISP_GET_BGC\n", __FUNCTION__, __LINE__);
                break;
            }
#if 0
        case CMD_DISP_SEND_TTX:
            {
                DISP_TTX_S  *pDispTtx;

                pDispTtx = (DISP_TTX_S *)arg;

                Ret = DISP_SendTtxData(pDispTtx->enDisp, &pDispTtx->TtxData);

                break;
            }
#endif

        case CMD_DISP_CREATE_VBI_CHANNEL:
            {
                DISP_VBI_CREATE_CHANNEL_S  *pstDispVbiCrtChanl;

                pstDispVbiCrtChanl = (DISP_VBI_CREATE_CHANNEL_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_CREATE_VBI_CHANNEL\n", __FUNCTION__, __LINE__);

                Ret = DISP_CreateVBIChannel(pstDispVbiCrtChanl->enDisp,  &pstDispVbiCrtChanl->stCfg, &pstDispVbiCrtChanl->hVbi);


                break;
            }

        case CMD_DISP_DESTROY_VBI_CHANNEL:
            {
                mt_handle  *pnVbiHandle;

                pnVbiHandle = (mt_handle *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_DESTROY_VBI_CHANNEL\n", __FUNCTION__, __LINE__);

                Ret = DISP_DestroyVBIChannel(*pnVbiHandle);

                break;
            }

        case CMD_DISP_SEND_VBI:
            {
                DISP_VBI_S  *pDispVbi;

                pDispVbi = (DISP_VBI_S *)arg;

                Ret = DISP_SendVbiData(pDispVbi->hVbi, &pDispVbi->stVbiData);

                break;
            }

        case CMD_DISP_SET_WSS:
            {
                DISP_WSS_S  *pDispWss;

                pDispWss = (DISP_WSS_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_WSS\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetWss(pDispWss->enDisp, &pDispWss->WssData);

                break;
            }

        case CMD_DISP_SET_MCRVSN:
            {
                DISP_MCRVSN_S  *pDispMcrvsn;

                pDispMcrvsn = (DISP_MCRVSN_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_MCRVSN\n", __FUNCTION__, __LINE__);

                if (pDispMcrvsn->pPriv)
                {
                    Ret = DISP_SetMacrovisionCustomer(pDispMcrvsn->enDisp, pDispMcrvsn->pPriv);
                }

                Ret = DISP_SetMacrovision(pDispMcrvsn->enDisp, pDispMcrvsn->eMcrvsn);

                break;
            }

        case CMD_DISP_GET_MCRVSN:
            {
                DISP_MCRVSN_S  *pDispMcrvsn;

                pDispMcrvsn = (DISP_MCRVSN_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_MCRVSN\n", __FUNCTION__, __LINE__);

                Ret = DISP_GetMacrovision(pDispMcrvsn->enDisp, &pDispMcrvsn->eMcrvsn);

                break;
            }
#if 0
        case CMD_DISP_GET_HDMI_INTF:
            {
                DISP_HDMIINF_S  *pDispHdmiIntf;

                pDispHdmiIntf = (DISP_HDMIINF_S *)arg;

                Ret = DISP_GetHdmiIntf(pDispHdmiIntf->enDisp, &pDispHdmiIntf->HDMIInf);

                break;
            }

        case CMD_DISP_SET_HDMI_INTF:
            {
                DISP_HDMIINF_S  *pDispHdmiIntf;

                pDispHdmiIntf = (DISP_HDMIINF_S *)arg;

                Ret = DISP_SetHdmiIntf(pDispHdmiIntf->enDisp, &pDispHdmiIntf->HDMIInf);

                break;
            }
#endif
        case CMD_DISP_SET_CGMS:
            {
                DISP_CGMS_S  *pDispCgms;

                pDispCgms = (DISP_CGMS_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_CGMS\n", __FUNCTION__, __LINE__);

                Ret = DISP_SetCGMS_A(pDispCgms->enDisp, &pDispCgms->stCgmsCfg);

                break;
            }
         case CMD_DISP_CREATE_CAST:
            {
                DISP_CAST_CREATE_S *pstC = (DISP_CAST_CREATE_S *)arg;

                /*if cast already open , return failed*/
                if (pDispState->hCastHandle[pstC->enDisp] != MT_NULL)
                    return MT_ERR_DISP_INVALID_OPT;

                DISP_DEBUGK("%s %d CMD_DISP_CREATE_CAST\n", __FUNCTION__, __LINE__);
                Ret = DISP_CreateCast(pstC->enDisp, &pstC->stCfg, &pstC->hCast);
                if (!Ret)
                {
                    pDispState->hCastHandle[pstC->enDisp] = pstC->hCast;
                }
                break;
            }
        case CMD_DISP_DESTROY_CAST:
            {
                DISP_CAST_DESTROY_S *pstC = (DISP_CAST_DESTROY_S *)arg;
                mt_s32 i;
                DISP_DEBUGK("%s %d CMD_DISP_DESTROY_CAST\n", __FUNCTION__, __LINE__);

                for (i=0; i<MT_DRV_DISPLAY_BUTT; i++)
                {
                    if (pDispState->hCastHandle[i] == pstC->hCast)
                    {
                        Ret = DISP_DestroyCast(pstC->hCast);
                        pDispState->hCastHandle[i] = MT_NULL;
                    }
                }

                break;
            }
        case CMD_DISP_SET_CAST_ENABLE:
            {
                DISP_CAST_ENABLE_S *pstC = (DISP_CAST_ENABLE_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_CAST_ENABLE\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_SetCastEnable(pstC->hCast, pstC->bEnable);
                break;
            }

        case CMD_DISP_GET_CAST_ENABLE:
            {
                DISP_CAST_ENABLE_S *pstC = (DISP_CAST_ENABLE_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_CAST_ENABLE\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_GetCastEnable(pstC->hCast, &pstC->bEnable);
                break;
            }

        case CMD_DISP_ACQUIRE_CAST_FRAME:
            {
                DISP_CAST_FRAME_S *pstC = (DISP_CAST_FRAME_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_ACQUIRE_CAST_FRAME\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_AcquireCastFrame(pstC->hCast, &pstC->stFrame);
                break;
            }

        case CMD_DISP_RELEASE_CAST_FRAME:
            {
                DISP_CAST_FRAME_S *pstC = (DISP_CAST_FRAME_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_RELEASE_CAST_FRAME\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_ReleaseCastFrame(pstC->hCast, &pstC->stFrame);
                break;
            }

        case CMD_DISP_ACQUIRE_SNAPSHOT:
            {
                DISP_SNAPSHOT_FRAME_S *pstFrame = (DISP_SNAPSHOT_FRAME_S*)arg;
                mt_handle snapshotHandleOut = 0;
                DISP_DEBUGK("%s %d CMD_DISP_ACQUIRE_SNAPSHOT\n", __FUNCTION__, __LINE__);

                /*does not support continuous snapshot.*/
                if (pDispState->hSnapshot[pstFrame->enDispLayer] != 0)
                    break;

                                Ret =  DISP_AcquireSnapshot(pstFrame->enDispLayer,
                                            &pstFrame->stFrame,
                                            &snapshotHandleOut);
                if (!Ret)
                    pDispState->hSnapshot[pstFrame->enDispLayer] = snapshotHandleOut;
                else
                    pDispState->hSnapshot[pstFrame->enDispLayer] = 0;

                break;
            }
        case CMD_DISP_RELEASE_SNAPSHOT:
            {
                DISP_SNAPSHOT_FRAME_S *pstFrame = (DISP_SNAPSHOT_FRAME_S*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_RELEASE_SNAPSHOT\n", __FUNCTION__, __LINE__);

                /*for released snapshot, just break.*/
                if (pDispState->hSnapshot[pstFrame->enDispLayer] == 0)
                    break;

                Ret = DISP_ReleaseSnapshot(pstFrame->enDispLayer, &pstFrame->stFrame, pDispState->hSnapshot[pstFrame->enDispLayer]);
                pDispState->hSnapshot[pstFrame->enDispLayer] = 0;
                break;
            }

        case CMD_DISP_EXT_ATTACH:
            {
                DISP_EXT_ATTACH_S *pstC = (DISP_EXT_ATTACH_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_EXT_ATTACH\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_External_Attach(pstC->hCast, pstC->hMutual);
                break;
            }
        case CMD_DISP_EXT_DEATTACH:
            {
                DISP_EXT_ATTACH_S *pstC = (DISP_EXT_ATTACH_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_EXT_DEATTACH\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DISP_External_DeAttach(pstC->hCast, pstC->hMutual);
                break;
            }
        case CMD_DISP_SET_CASTATTR:
            {
                DISP_CAST_EXT_ATTR_S *pstC = (DISP_CAST_EXT_ATTR_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_CASTATTR\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DRV_DISP_SetCastAttr(pstC->hCast, &pstC->castAttr);
                break;
            }
        case CMD_DISP_GET_CASTATTR:
            {
                DISP_CAST_EXT_ATTR_S *pstC = (DISP_CAST_EXT_ATTR_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_GET_CASTATTR\n", __FUNCTION__, __LINE__);

                /*To do*/
                Ret = DRV_DISP_GetCastAttr(pstC->hCast, &pstC->castAttr);
                break;
            }
        case CMD_DISP_SUSPEND:
            {
                DISP_DEBUGK("%s %d CMD_DISP_SUSPEND\n", __FUNCTION__, __LINE__);
                /*To do*/
                Ret = DISP_Suspend();
                break;
            }
        case CMD_DISP_RESUME:
            {
                DISP_DEBUGK("%s %d CMD_DISP_RESUME\n", __FUNCTION__, __LINE__);
                /*To do*/
                Ret = DISP_Resume();
                break;
            }
        case CMD_DISP_FORCESET_DAC_ENABLE:
            {
                MT_BOOL bDacEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_FORCESET_DAC_ENABLE bDacEnable:%d \n", __FUNCTION__, __LINE__, bDacEnable);
                if (bDacEnable == MT_TRUE)
                {
                    Ret  = DISP_SetAllDacEn(MT_TRUE);
                    Ret |= DISP_SetDACDetEn(MT_FALSE);
                }
                else
                {
                    Ret = DISP_SetDACDetEn(MT_TRUE);
                }
                break;
            }
        case CMD_DISP_SET_COLORBAR:
            {
                DISP_ENABLE_S *pst = (DISP_ENABLE_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_COLORBAR\n", __FUNCTION__, __LINE__);
                Ret = disp_set_colorbar(pst->enDisp, pst->bEnable);

                break;
            }      
        case CMD_DISP_SET_OUTPUT_ENABLE:
            {
                DISP_ENABLE_S *pst = (DISP_ENABLE_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_OUTPUT_ENABLE bEnable:%d\n", __FUNCTION__, __LINE__, pst->bEnable);
                Ret = disp_set_output_enable(pst->enDisp, pst->bEnable);

                break;
            } 
        
        case CMD_DISP_SET_BRIGHT:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_BRIGHT CscValue:%d\n", __FUNCTION__, __LINE__, pst->CscValue);
                Ret = disp_set_bright(pst->enDisp, pst->CscValue);

                break;
            }
        case CMD_DISP_SET_CONTRAST:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_CONTRAST CscValue:%d\n", __FUNCTION__, __LINE__, pst->CscValue);
                Ret = disp_set_contrast(pst->enDisp, pst->CscValue);

                break;
            }
        case CMD_DISP_SET_SATURATION:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_SATURATION CscValue:%d\n", __FUNCTION__, __LINE__, pst->CscValue);
                Ret = disp_set_saturation(pst->enDisp, pst->CscValue);

                break;
            }
        case CMD_DISP_SET_HUE:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_HUE CscValue:%d\n", __FUNCTION__, __LINE__, pst->CscValue);
                Ret = disp_set_hue(pst->enDisp, pst->CscValue);

                break;
            }
        case CMD_DISP_GET_BRIGHT:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                Ret = disp_get_bright(pst->enDisp, &pst->CscValue);
                DISP_DEBUGK("%s %d CMD_DISP_GET_BRIGHT\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_GET_CONTRAST:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                Ret = disp_get_contrast(pst->enDisp, &pst->CscValue);
                DISP_DEBUGK("%s %d CMD_DISP_GET_CONTRAST\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_GET_SATURATION:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                Ret = disp_get_saturation(pst->enDisp, &pst->CscValue);
                DISP_DEBUGK("%s %d CMD_DISP_GET_SATURATION\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_GET_HUE:
            {
                DISP_CSC_S *pst = (DISP_CSC_S *)arg;
                Ret = disp_get_hue(pst->enDisp, &pst->CscValue);
                DISP_DEBUGK("%s %d CMD_DISP_GET_HUE\n", __FUNCTION__, __LINE__);

                break;
            }

        
        case CMD_DISP_SET_LAYER_SHOW:
            {
                DISP_LAYERSHOW_S *pst = (DISP_LAYERSHOW_S *)arg;
                Ret = disp_set_layershow(pst->enDisp, pst);
                DISP_DEBUGK("%s %d CMD_DISP_SET_LAYER_SHOW\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_SET_PPMODE:
            {
                DISP_PP_S *pst = (DISP_PP_S *)arg;
                Ret = disp_set_set_postprocess_mode(pst->enDisp, pst);
                DISP_DEBUGK("%s %d CMD_DISP_SET_PPMODE\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_SET_LAYER_ENABLE:
            {
                MT_BOOL bLayerEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_LAYER_ENABLE bLayerEnable:%d \n", __FUNCTION__, __LINE__, bLayerEnable);
                Ret = disp_set_vid_layer_show(bLayerEnable);

                break;
            }
        case CMD_DISP_GET_LAYER_ENABLE:
            {
                MT_BOOL *pbLayerEnable = (MT_BOOL*)arg;
                Ret = disp_get_vid_layer_show(pbLayerEnable);
                DISP_DEBUGK("%s %d CMD_DISP_GET_LAYER_ENABLE\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_GET_VIDEO_SIZE:
            {
                DISP_VIDEO_SIZE_S *psize = (DISP_VIDEO_SIZE_S *)arg;
                disp_st_vid_get_vdec_size(&psize->u32Height, &psize->u32Width);
                DISP_DEBUGK("%s %d CMD_DISP_GET_VIDEO_SIZE\n", __FUNCTION__, __LINE__);
                Ret = MT_SUCCESS;
                break;
            }
        case CMD_DISP_SET_LAYER_ALPHA:
            {
                DISP_ALPHA_S *pst = (DISP_ALPHA_S *)arg;
                Ret = disp_set_layer_alpha(pst->enDisp, pst->u32Alpha);
                DISP_DEBUGK("%s %d CMD_DISP_SET_LAYER_ALPHA\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_GET_LAYER_ALPHA:
            {
                DISP_ALPHA_S *pst = (DISP_ALPHA_S *)arg;
                Ret = disp_get_layer_alpha(pst->enDisp, &pst->u32Alpha);
                DISP_DEBUGK("%s %d CMD_DISP_GET_LAYER_ALPHA\n", __FUNCTION__, __LINE__);

                break;
            }
        case CMD_DISP_SET_CSC_ENABLE:
            {
                MT_BOOL bCscEnable = *(MT_BOOL*)arg;
                Ret = disp_set_csc_onoff(bCscEnable);
                DISP_DEBUGK("%s %d CMD_DISP_SET_CSC_ENABLE bCscEnable:%d\n", __FUNCTION__, __LINE__, bCscEnable);

                break;
            }
        case CMD_DISP_SET_DENOISE_ENABLE:
            {
                MT_BOOL bDenoiseEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_DENOISE_ENABLE bDenoiseEnable:%d\n", __FUNCTION__, __LINE__, bDenoiseEnable);
                Ret = disp_set_denoise_onoff(bDenoiseEnable);

                break;
            }
        case CMD_DISP_SET_AFD_ENABLE:
            {
                MT_BOOL bAfdEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_AFD_ENABLE bAfdEnable:%d\n", __FUNCTION__, __LINE__, bAfdEnable);
                Ret = disp_set_afd_onoff(bAfdEnable);

                break;
            }

        case CMD_DISP_SET_HDVIDEO_ENABLE:
            {
                MT_BOOL bHdVideoEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_HDVIDEO_ENABLE bHdVideoEnable:%d\n", __FUNCTION__, __LINE__, bHdVideoEnable);
                Ret = disp_set_hd_video_onoff(bHdVideoEnable);

                break;
            }

        case CMD_DISP_SET_SDVIDEO_ENABLE:
            {
                MT_BOOL bSdVideoEnable = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_SDVIDEO_ENABLE bSdVideoEnable:%d\n", __FUNCTION__, __LINE__, bSdVideoEnable);
                Ret = disp_set_sd_video_onoff(bSdVideoEnable);

                break;
            }

         case CMD_DISP_SET_VDAC_ONOFF:
        {
                DISP_VDACOUTPUT_S *pst = (DISP_VDACOUTPUT_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_VDAC_ONOFF b_enable:%d\n", __FUNCTION__, __LINE__, pst->b_enable);
                Ret = disp_set_vac_onoff(pst->dac_id, pst->b_enable);

            break;
        }
         case CMD_DISP_SET_LOWDELAY_ENABLE:
            {
            DISP_LOWDELAY_ENABLE_S *pstC = (DISP_LOWDELAY_ENABLE_S *)arg;
            DISP_DEBUGK("%s %d CMD_DISP_SET_LOWDELAY_ENABLE b_enable:%d\n", __FUNCTION__, __LINE__, pstC->bEnable);
            Ret = MT_DRV_DISP_SetLowDelayEnable(pstC->hCast, pstC->bEnable);
            break;
            }
		case CMD_DISP_SET_TRICK_MODE:
		{
          DISP_TRICK_MODE_E  trickmode = *(DISP_TRICK_MODE_E *)arg;
          MT_DF_Trick_Mode_Info_T info;
          mt_u32 index = Win_GetVideoWinIndex();
          
          DISP_DEBUGK("%s CMD_DISP_SET_TRICK_MODE=%d win_index=%d\n", __FUNCTION__, trickmode, index);
          if(index < MAX_WIN_NUM)
          {
              if(trickmode == TM_FFWD_X2)
              {
                info.trick_mode = DF_TRICK_MODE_FAST_FORWARD;
                info.trick_sr = MT_DF_PLAY_SPEED_X2;
              }
              else if(trickmode == TM_SFWD_X2)
              {
                info.trick_mode = DF_TRICK_MODE_SLOW_FORWARD;
                info.trick_sr = MT_DF_PLAY_SPEED_X2;
              }
              else if(trickmode == TM_FREV)
              {
                info.trick_mode = DF_TRICK_MODE_FAST_BACKWARD;
                info.trick_sr = MT_DF_PLAY_SPEED_X2;
              }
              else if(trickmode == TM_FFWD_X2_MORE)
              {
                info.trick_mode = DF_TRICK_MODE_FAST_FORWARD;
                info.trick_sr = MT_DF_PLAY_SPEED_X4;
              }
              else if(trickmode == TM_SFWD_X2_MORE)
              {
                info.trick_mode = DF_TRICK_MODE_SLOW_FORWARD;
                info.trick_sr = MT_DF_PLAY_SPEED_X4;
              }
              else
              {
                info.trick_mode = DF_TRICK_MODE_NORMAL;
                info.trick_sr = MT_DF_PLAY_SPEED_NORMAL;
              }
              
              Ret = DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_SET_TRICK_MODE, (mt_void*)&info);          
          }

          if(index == MAX_WIN_NUM)
          {
              Ret = MT_FAILURE;
          }

          break;
        }
        
        case CMD_DISP_SET_UNBLANK_MODE:
        {
          MT_DRV_DISP_UNBLANK_MODE_E  disp_unblank_mode = *(MT_DRV_DISP_UNBLANK_MODE_E *)arg;
          DF_START_MODE_E df_unblank_mode;
          mt_u32 i = 0;
          Ret = 0;
          DISP_DEBUGK("%s CMD_DISP_SET_UNBLANK_MODE=%d \n", __FUNCTION__, disp_unblank_mode);

          if(disp_unblank_mode == DISP_UNBLANK_MODE_USER)
          {
              df_unblank_mode = DF_UNBLANK_USER;                
          }
          else if(disp_unblank_mode == DISP_UNBLANK_MODE_SYNC)
          {
              df_unblank_mode = DF_UNBLANK_SYNC; 
          }
          else if(disp_unblank_mode == DISP_UNBLANK_MODE_STABLE)
          {
              df_unblank_mode = DF_UNBLANK_STABLE; 
          }
          else if(disp_unblank_mode == DISP_UNBLANK_MODE_FAST)
          {
              df_unblank_mode = DF_UNBLANK_FAST;
          }
          else
          {
              df_unblank_mode = DF_UNBLANK_SYNC;
          }

          for(i = 0; i < MAX_WIN_NUM; i++)
          {                    
              if(g_pstWin[i] == NULL)
              {                        
                  continue;
              }                    
              Ret |= DF_SetCmd(g_pstWin[i]->stBuffer.stDispBP, DF_CMD_SET_UNBLANK_MODE, (mt_void*)df_unblank_mode);
          }
                
          break;
        }

        case CMD_DISP_SET_DI_ENABLE:
            {
                MT_BOOL bDiEnable = *(MT_BOOL*)arg;
                MT_U32 i = 0;

                for(i = 0; i < MAX_WIN_NUM; i ++)
                {                    
                    if(g_pstWin[i] == NULL)
                    {                        
                        continue;
                    }                
                    Ret = DF_SetCmd(g_pstWin[i]->stBuffer.stDispBP, DF_CMD_FORCE_CLOSE_DI, (mt_void*)((ulong)!bDiEnable));
                    DISP_DEBUGK("%s CMD_DISP_SET_DI_ENABLE index:%d subvideo:%d\n", __FUNCTION__, i,
                        g_pstWin[i]->stCfg.stAttr.bUseSubLayer);
                }
                break;
            }
		
        case CMD_FORCE_SHOW_DS_PIC:
            {
                DISP_SHOW_DS_S *pst = (DISP_SHOW_DS_S*)arg;
                MT_U32 index_video = Win_GetVideoWinIndex();
                MT_U32 index_still = Win_GetStillWinIndex();
                MT_BOOL bDS = pst->bShowDsPic;

                if (pst->bStillLayer)
                {                    
                    Ret = DF_SetCmd(g_pstWin[index_still]->stBuffer.stDispBP, DF_CMD_FORCE_SHOW_DS_PIC, (mt_void*)((ulong)bDS));
                    DISP_DEBUGK("%s CMD_FORCE_SHOW_DS_PIC index:%d subvideo:%d bDS:%d\n", __FUNCTION__, index_still,
                        g_pstWin[index_still]->stCfg.stAttr.bUseSubLayer, bDS);
                }
                else
                {
                    Ret = DF_SetCmd(g_pstWin[index_video]->stBuffer.stDispBP, DF_CMD_FORCE_SHOW_DS_PIC, (mt_void*)((ulong)bDS));
                    DISP_DEBUGK("%s CMD_FORCE_SHOW_DS_PIC index:%d subvideo:%d bDS:%d\n", __FUNCTION__, index_video,
                        g_pstWin[index_video]->stCfg.stAttr.bUseSubLayer, bDS);
                }
                break;
            }

        case CMD_DISP_SET_TVCAP:
            {
                MT_DRV_DISP_HDMI_MODE_E enTvCap = *(MT_DRV_DISP_HDMI_MODE_E*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_TVCAP enTvCap:%d\n", __FUNCTION__, __LINE__, enTvCap);
                Ret = disp_set_tv_capability(enTvCap);
                break;
            }

        case CMD_DISP_SET_SL_HDR_PARA:
            {

                DISP_SL_HDR_PARA_S *pst = (DISP_SL_HDR_PARA_S *)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_SL_HDR_PARA \n", __FUNCTION__, __LINE__);

                Ret = disp_set_sl_hdr(pst->enDisp, pst->transparent_mode,pst->u32Display_Brightness, pst->u32Tuning_level, pst->u32Display_OETF);
                break;
            }
        case CMD_DISP_SET_SD_ENC_PQ_PARA:
            {
                DISP_SD_ENC_PQ_PARA_S *pst = (DISP_SD_ENC_PQ_PARA_S*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_SET_SD_ENC_PQ_PARA \n", __FUNCTION__, __LINE__);
                Ret = disp_hal_set_sd_venc_reg(pst);
                break;
            }
        case CMD_DISP_RESET_HARDWARE:
            {
                MT_BOOL b_clock_hi = *(MT_BOOL*)arg;
                DISP_DEBUGK("%s %d CMD_DISP_RESET_HARDWARE \n", __FUNCTION__, __LINE__);
                Ret = DispResetHardware(b_clock_hi);
                break;
            }
        case CMD_DISP_SET_SD_SCALER_ENABLE:
            {
                MT_BOOL sd_scaler_state = *(MT_U32 *)arg;
                mt_u32 index = 0;
                DISP_DEBUGK("%s %d CMD_DISP_RESET_HARDWARE sd_scaler_state:%d\n", __FUNCTION__, __LINE__,sd_scaler_state);
                for(index = 0; index < MAX_WIN_NUM; index++)
                {
                    if(g_pstWin[index] != NULL)
                    {
                        if(sd_scaler_state == 2) // auto mode, determined by Fw policy
                        {
                            SdScalerEnable = MT_FALSE;
                            Ret = DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_SET_VIDEO_SCALER, (mt_void*)DF_VIDEO_SCALER_AUTO);
                        }
                        else if(sd_scaler_state == 1)  // force 3scaler mode
                        {
                            SdScalerEnable = MT_TRUE;
                            Ret = DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_SET_VIDEO_SCALER, (mt_void*)DF_VIDEO_FORCE_3SCALER);
                        }
                        else if(sd_scaler_state == 0)  //// force 2scaler mode
                        {
                            SdScalerEnable = MT_FALSE;
                            Ret = DF_SetCmd(g_pstWin[index]->stBuffer.stDispBP, DF_CMD_SET_VIDEO_SCALER, (mt_void*)DF_VIDEO_FORCE_2SCALER);
                        }
      			    }
      	        }                
                
                break;
            }

       case CMD_DISP_DUMP_SCALER_TO_OSD:
           {
                MT_DRV_DISP_DUMP_SCALER_PARA_S *pst = (MT_DRV_DISP_DUMP_SCALER_PARA_S*)arg;
                mt_u32 index = Win_GetVideoWinIndex();
                DISP_DEBUGK("%s %d CMD_DISP_DUMP_SCALER_TO_OSD\n", __FUNCTION__, __LINE__);
                if(index < MAX_WIN_NUM)
                {
                    Ret = disp_dump_scaler_to_osd(g_pstWin[index]->stBuffer.stDispBP, pst);
                }                
            
                break;
           }

        case CMD_DISP_GET_SlHDR_VER:
            {
                mt_u32 *pVer = (mt_u32 *)arg;
                *pVer = DF_GetSlHdrVersion();
                if(*pVer == 0)
                {
                    Ret = MT_FAILURE;
                }
                else
                {
                    Ret = MT_SUCCESS;
                }                
                
                DISP_DEBUGK("%s %d CMD_DISP_GET_SlHDR_VER ver:0x%x\n", __FUNCTION__, __LINE__, *pVer);

                break;
            }       
      default:
            // ????
            //up(&g_DispMutex);
            return -ENOIOCTLCMD;
    }

    return Ret;
}


mt_s32 DRV_DISP_Init2(mt_void)
{
    atomic_set(&g_DispCount, 1);

    DRV_DISP_ProcInit();

    DRV_DISP_ProcAdd(MT_DRV_DISPLAY_0);
    DRV_DISP_ProcAdd(MT_DRV_DISPLAY_1);

    (mt_void)DISP_Init();

    MT_DRV_DISP_Open(MT_DRV_DISPLAY_0);
    MT_DRV_DISP_Open(MT_DRV_DISPLAY_1);


    return MT_SUCCESS;
}

mt_s32 DRV_DISP_DeInit2(mt_void)
{
    DRV_DISP_ProcDel(MT_DRV_DISPLAY_1);
    DRV_DISP_ProcDel(MT_DRV_DISPLAY_0);

    DRV_DISP_ProcDeInit();

    MT_DRV_DISP_Close(MT_DRV_DISPLAY_0);
    MT_DRV_DISP_Close(MT_DRV_DISPLAY_1);

    /* closing clock */
    DISP_DeInit();

    atomic_set(&g_DispCount, 0);

    return MT_SUCCESS;
}

//may be delete
mt_s32 MT_DRV_DISP_Init(mt_void)
{
    mt_s32          Ret;

    Ret = down_interruptible(&g_DispMutex);

    if (1 == atomic_inc_return(&g_DispCount))
    {
        /* for configuration such as start clock, re-use pins, etc */
        Ret = DISP_Init();
        if (Ret != MT_SUCCESS)
        {
            MT_FATAL_DISP("call DISP_Init failed.\n");
            atomic_dec(&g_DispCount);
            up(&g_DispMutex);
            return -1;
        }
    }

    up(&g_DispMutex);

    Ret = DRV_DISP_Register();

    return Ret;
}

//mt_s32 MT_DRV_DISP_ModDeinit(mt_void)
mt_s32 MT_DRV_DISP_DeInit(mt_void)
{
    mt_s32 Ret;
    MT_DRV_DISPLAY_E u;

    Ret = down_interruptible(&g_DispMutex);

    //MT_INFO_DISP("come to close HD/SD g_DispModState.bDisp1Open:%d, g_DispModState.bDisp0Open:%d\n",
    //        g_DispModState.bDisp1Open, g_DispModState.bDisp0Open);

    for(u=0; u<MT_DRV_DISPLAY_BUTT; u++)
    {
        if (g_DispModState.bDispOpen[u])
        {
            MT_INFO_DISP("DISP_MOD_ExtClose HD0\n");
            Ret = DISP_ExtClose(u, &g_DispModState, MT_FALSE);
            if (Ret != MT_SUCCESS)
            {
                MT_FATAL_DISP("DISP_MOD_ExtClose Display %d failed!\n", u);
            }
        }
    }

    MT_INFO_DISP("MT_DRV_DISP_Deinit:atomic g_DispCount:%d\n", atomic_read(&g_DispCount));

    if (atomic_dec_and_test(&g_DispCount))
    {
        MT_INFO_DISP("close clock\n");

        /* closing clock */
        DISP_DeInit();
    }

    up(&g_DispMutex);
    return 0;
}

mt_s32 DRV_DISP_AvsyncGetVptsInfo(void *pstDispBP, void *pInfo)
{
	return (mt_s32)DF_AvsyncGetVptsInfo((mt_handle)pstDispBP, (DF_AVSYNC_PTS_S*)pInfo);
}

mt_s32 DRV_DISP_AvsyncGetInputRate(void *pstDispBP, MT_U32 *pInputRate)
{
	return (mt_s32)DF_AvsyncGetInputRate((mt_handle)pstDispBP, pInputRate);
}

mt_s32 DRV_DISP_AvsyncSetSkipFrame(MT_U32 skip_num)
{
    mt_s32 Ret = MT_FAILURE;
    mt_u32 index = Win_GetVideoWinIndex();
    if(index < MAX_WIN_NUM)
    {
        return (mt_s32)DF_AvsyncSetSkipFrame(g_pstWin[index]->stBuffer.stDispBP, skip_num);        
    }

    return Ret; 
}

mt_s32 DRV_DISP_AvsyncSetRepeatFrame(void)
{
    mt_s32 Ret = MT_FAILURE;
    mt_u32 index = Win_GetVideoWinIndex();
    if(index < MAX_WIN_NUM)
    {
        return (mt_s32)DF_AvsyncSetRepeatFrame(g_pstWin[index]->stBuffer.stDispBP);        
    }

    return Ret;     
}

// only for fpga verify
mt_s32 DRV_DISP_StillScalerUpdate(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive)
{
    mt_s32 Ret = MT_FAILURE;
    mt_u32 index = Win_GetVideoWinIndex();
    if(index < MAX_WIN_NUM)
    {
        return 
            (mt_s32)DF_StillScaler_Update(g_pstWin[index]->stBuffer.stDispBP, src_width, src_height, dst_width, dst_height, bProgressive);
    }

    return Ret;       
}

static DISP_EXPORT_FUNC_S s_stDispExportFuncs = {
    .pfnDispInit             = MT_DRV_DISP_Init            ,
    .pfnDispDeInit           = MT_DRV_DISP_DeInit          ,
    .pfnDispAttach           = MT_DRV_DISP_Attach          ,
    .pfnDispDetach           = MT_DRV_DISP_Detach          ,
    .pfnDispSetFormat        = MT_DRV_DISP_SetFormat       ,
    .pfnDispGetFormat        = MT_DRV_DISP_GetFormat       ,
    .pfnDispSetCustomTiming  = MT_DRV_DISP_SetCustomTiming ,
    .pfnDispGetCustomTiming  = MT_DRV_DISP_GetCustomTiming ,
    .pfnDispAddIntf          = MT_DRV_DISP_AddIntf         ,
    .pfnDispDeIntf           = MT_DRV_DISP_DelIntf         ,

    .pfnDispOpen             = MT_DRV_DISP_Open            ,
    .pfnDispClose            = MT_DRV_DISP_Close           ,
    .pfnDispSetEnable        = MT_DRV_DISP_SetEnable       ,
    .pfnDispGetEnable        = MT_DRV_DISP_GetEnable       ,
    .pfnDispSetRightEyeFirst = MT_DRV_DISP_SetRightEyeFirst,
    .pfnDispSetBgColor       = MT_DRV_DISP_SetBgColor      ,
    .pfnDispGetBgColor       = MT_DRV_DISP_GetBgColor      ,
    .pfnDispSetAspectRatio   = MT_DRV_DISP_SetAspectRatio  ,
    .pfnDispGetAspectRatio   = MT_DRV_DISP_GetAspectRatio  ,
    .pfnDispSetLayerZorder   = MT_DRV_DISP_SetLayerZorder  ,
    .pfnDispGetLayerZorder   = MT_DRV_DISP_GetLayerZorder  ,

    .pfnDispCreatCast        = MT_DRV_DISP_CreateCast      ,
    .pfnDispDestoryCast      = MT_DRV_DISP_DestroyCast     ,
    .pfnDispSetCastEnable    = MT_DRV_DISP_SetCastEnable   ,
    .pfnDispGetCastEnable    = MT_DRV_DISP_GetCastEnable   ,
    .pfnDispAcquireCastFrm   = MT_DRV_DISP_AcquireCastFrame,
    .pfnDispRlsCastFrm       = MT_DRV_DISP_ReleaseCastFrame,

    .pfnDispExtAttach        = MT_DRV_DISP_ExternlAttach,
    .pfnDispExtDeAttach      = MT_DRV_DISP_ExternlDetach,
    .pfnDispSetCastAttr      = MT_DRV_DISP_SetCastAttr,
    .pfnDispGetCastAttr      = MT_DRV_DISP_GetCastAttr,

    .pfnDispGetInitFlag      = MT_DRV_DISP_GetInitFlag     ,
    .pfnDispGetVersion       = MT_DRV_DISP_GetVersion      ,
    .pfnDispIsOpen           = MT_DRV_DISP_IsOpened        ,
    .pfnDispGetSlave         = MT_DRV_DISP_GetSlave        ,
    .pfnDispGetMaster        = MT_DRV_DISP_GetMaster       ,
    .pfnDispGetDispInfo      = MT_DRV_DISP_GetDisplayInfo  ,

    .pfnDispIoctl            = MT_DRV_DISP_Process,
    .pfnDispRegCallback      = MT_DRV_DISP_RegCallback,
    .pfnDispUnRegCallback    = MT_DRV_DISP_UnRegCallback,
    .pfnDispSuspend = DRV_DISP_Suspend,
    .pfnDispResume = DRV_DISP_Resume,

    .disp_set_hd_video_onoff       = disp_set_hd_video_onoff,
    .disp_get_dce_percent          = disp_get_dce_percent,
    .DRV_DISP_AvsyncGetVptsInfo    = DRV_DISP_AvsyncGetVptsInfo,
    .DRV_DISP_AvsyncGetInputRate   = DRV_DISP_AvsyncGetInputRate,
    .DRV_DISP_AvsyncSetSkipFrame   = DRV_DISP_AvsyncSetSkipFrame,
    .DRV_DISP_AvsyncSetRepeatFrame = DRV_DISP_AvsyncSetRepeatFrame,
    .reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode,
    .reg_aria_hd_encoder_get_basic_cfg_interlace_mode   = reg_aria_hd_encoder_get_basic_cfg_interlace_mode,
    .pfnDispStillScalerUpdate      = DRV_DISP_StillScalerUpdate,   // only for fpga verify
    .pfnDispTvsysForceUpdate      = disp_tvsys_force_update,
};

mt_s32 DRV_DISP_Register(mt_void)
{

    mt_s32 Ret;

    // add for multiple process
    DISP_ResetCountStatus();

    DRV_DISP_ProcInit();

    Ret = mt_drv_module_register((mt_u32)MT_ID_DISP, "MT_DISP", (mt_void *)(&s_stDispExportFuncs));
    if (MT_SUCCESS != Ret)
    {
        MT_FATAL_DISP("MT_DRV_MODULE_Register DISP failed\n");
        return Ret;
    }

    return  0;
}

mt_void DRV_DISP_UnRegister(mt_void)
{

    mt_drv_module_unregister((mt_u32)MT_ID_DISP);

    DRV_DISP_ProcDeInit();

    return;
}

/* ======================================================================= */
#if 0
mt_s32 DRV_DISP_SetBrightness(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Brightness)
{
    mt_s32 Ret;
    DISP_CSC_S  enDispCsc;

    enDispCsc.CscValue = u32Brightness;
    enDispCsc.enDisp = enDisp;
    Ret = DRV_DISP_Process(CMD_DISP_SET_BRIGHT,  &enDispCsc);
    return Ret;
}

mt_s32 DRV_DISP_SetContrast(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Contrast)
{
    mt_s32 Ret;
    DISP_CSC_S  enDispCsc;

    enDispCsc.CscValue = u32Contrast;
    enDispCsc.enDisp = enDisp;
    Ret = DRV_DISP_Process(CMD_DISP_SET_CONTRAST,  &enDispCsc);
    return Ret;
}

mt_s32 DRV_DISP_SetSaturation(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Saturation)
{
    mt_s32 Ret;
    DISP_CSC_S  enDispCsc;

    enDispCsc.CscValue = u32Saturation;
    enDispCsc.enDisp = enDisp;
    Ret = DRV_DISP_Process(CMD_DISP_SET_SATURATION,  &enDispCsc);
    return Ret;
}

mt_s32 DRV_DISP_SetHuePlus(MT_DRV_DISPLAY_E enDisp, mt_u32 u32HuePlus)
{
    mt_s32 Ret;
    DISP_CSC_S  enDispCsc;

    enDispCsc.CscValue = u32HuePlus;
    enDispCsc.enDisp = enDisp;
    Ret = DRV_DISP_Process(CMD_DISP_SET_HUE,  &enDispCsc);
    return Ret;
}


mt_s32 DRV_DISP_SetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E enMode, mt_void *pData)
{
    mt_s32 Ret;
    DISP_MCRVSN_S  enDispMcrvsn;

    enDispMcrvsn.enDisp = enDisp;
    enDispMcrvsn.eMcrvsn = enMode;
    Ret = DRV_DISP_Process(CMD_DISP_SET_MCRVSN,  &enDispMcrvsn);
    return Ret;
}
#endif


EXPORT_SYMBOL(MT_DRV_DISP_SetFormat);

#if 0

/* for intf */
EXPORT_SYMBOL(DRV_DISP_Open);
EXPORT_SYMBOL(DRV_DISP_Close);
EXPORT_SYMBOL(DRV_DISP_Attach);
EXPORT_SYMBOL(DRV_DISP_Detach);
EXPORT_SYMBOL(DRV_DISP_AddIntf);
EXPORT_SYMBOL(DRV_DISP_SetFormat);
EXPORT_SYMBOL(DRV_DISP_SetTiming);
//EXPORT_SYMBOL(DRV_DISP_SetHuePlus);
//EXPORT_SYMBOL(DRV_DISP_SetSaturation);
//EXPORT_SYMBOL(DRV_DISP_SetContrast);
//EXPORT_SYMBOL(DRV_DISP_SetBrightness);

EXPORT_SYMBOL(DRV_DISP_SetBgColor);
//EXPORT_SYMBOL(DRV_DISP_SetMacrovision);

EXPORT_SYMBOL(DRV_DISP_Suspend);
EXPORT_SYMBOL(DRV_DISP_Resume);

EXPORT_SYMBOL(DRV_DISP_Register);
EXPORT_SYMBOL(DRV_DISP_UnRegister);

EXPORT_SYMBOL(DRV_DISP_Ioctl);

EXPORT_SYMBOL(DRV_DISP_Init);
EXPORT_SYMBOL(DRV_DISP_DeInit);

EXPORT_SYMBOL(DRV_DISP_ProcRegister);
EXPORT_SYMBOL(DRV_DISP_ProcUnRegister);

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
