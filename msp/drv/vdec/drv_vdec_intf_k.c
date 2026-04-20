/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : drv_vdec_intf_k.c
  Version       : Initial Draft
  Author        : Montage MA-SW
  Created       : 2016/01/06
  Description   :
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file
******************************************************************************/

/******************************* Include Files *******************************/

/* Sys headers */
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
 #include <linux/timer.h>
#include <asm/io.h>
//#include <mach/hardware.h>

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#include "mt_mach/irq.h"
#endif
#include "mt_mach/chipinfo.h"

/* Unf headers */
#include "mt_error_mpi.h"
#include "mt_unf_common.h"
/* Drv headers */
#include "mt_kernel_adapt.h"
#include "drv_demux_ext.h"
#include "vfmw.h"
#include "drv_vdec_ext.h"
#include "drv_vpss_ext.h"
#include "mt_drv_vpss.h"
#include "mt_drv_disp.h"
//#include "../vo/vdp_v3_0/drv/drv_display.h"
#include "drv_disp_ext.h"

/* Local headers */
#include "drv_vdec_private.h"
#include "drv_vdec_pts_recv.h"
#include "drv_vdec_buf_mng.h"
#include "drv_vdec_usrdata.h"
#include "mt_drv_vdec.h"
#include "mt_mpi_vdec.h"
#include "mt_drv_stat.h"

#include "drv_vdec_frame_info.h"
#include "drv_vdec_ext.h"
#include "mach/irqs.h"
#include "vconfig.h"
#include "drv_vdec_debug.h"
#include "mt_module_debug.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dump.h"

//#include "VdecAPI.h"

#undef LOG_TAG
#define LOG_TAG				"VDEC_DRV"
#include "Log.h"

//Input from VFMW: debug dump read VFMW image frame
#define DEBUG_DUMP_VFMW_FRAME	0

//Output to userspace: debug dump receive vpss image frame
#define DEBUG_DUMP_VPSS_FRAME	0
//#define VDEC_4G_ADDR_TEST	1

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/
#define VDH_IRQ_NUM (95 + 32) /*interrupt vdector*/
#define MCE_INVALID_FILP (0xffffffff)
#define VDEC_NAME "MT_VDEC"
#define VDEC_IFRAME_MAX_READTIMES 2

#define MT_VMALLOC_BUFMNG(size) MT_VMALLOC(MT_ID_VDEC, size)
#define MT_VFREE_BUFMNG(addr) MT_VFREE(MT_ID_VDEC, addr)
#define MT_KMALLOC_ATOMIC_BUFMNG(size) MT_KMALLOC(MT_ID_VDEC, size, GFP_ATOMIC)
#define MT_KFREE_BUFMNG(addr) MT_KFREE(MT_ID_VDEC, addr)

#define VDEC_CHAN_STRMBUF_ATTACHED(pstChan) \
    (((MT_INVALID_HANDLE != pstChan->hStrmBuf) && (MT_INVALID_HANDLE == pstChan->hDmxVidChn)) || ((MT_INVALID_HANDLE == pstChan->hStrmBuf) && (MT_INVALID_HANDLE != pstChan->hDmxVidChn)))

#define VDEC_CHAN_TRY_USE_DOWN(pstEnt) \
    s32Ret = VDEC_CHAN_TRY_USE_DOWN_HELP((pstEnt));

#define VDEC_CHAN_USE_UP(pstEnt) \
    VDEC_CHAN_USE_UP_HELP((pstEnt));

#define VDEC_CHAN_RLS_DOWN(pstEnt, time) \
    s32Ret = VDEC_CHAN_RLS_DOWN_HELP((pstEnt), (time));

#define VDEC_CHAN_RLS_UP(pstEnt) \
    VDEC_CHAN_RLS_UP_HELP((pstEnt));

#define MT_VDEC_SCD_EXT_MEM (80000)
#define MT_VDEC_SVDEC_VDH_MEM (45 * 1024 * 1024)
#define MT_VDEC_REF_FRAME_MIN (4)
#define MT_VDEC_REF_FRAME_MAX (16)
#define MT_VDEC_DISP_FRAME_MIN (3)
#define MT_VDEC_DISP_FRAME_MAX (18)
#if (1 == MT_VDEC_HD_SIMPLE)
#define MT_VDEC_BUFFER_FRAME (1)
#else
#define MT_VDEC_BUFFER_FRAME (2)
#endif

#define MT_VDEC_TREEBUFFER_MIN (11)

#define MT_VDEC_RESOCHANGE_MASK (0x1)
#define MT_VDEC_CLOSEDEI_MASK (0x2) /* Close deinterlace */

#define MT_VDEC_CC_FROM_IMAGE (1)

#define MT_VDEC_FRAME_SIZE_1080P (3600 * 1024)
#define MT_VDEC_FRAME_SIZE_2160P (8100 * 1024)
#define MT_VDEC_FRAME_SIZE_4K (15400 * 1024)
mt_u32 vdec_descriptor_get_write_ptr(mt_u32 ch);
mt_u32 vdec_descriptor_get_read_ptr(mt_u32 ch);
SINT32 KERN_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs);
SINT32 KERN_VDEC_Exit(VOID);

/*************************** Structure Definition ****************************/

/* Channel entity */
typedef struct tagVDEC_CHAN_ENTITY_S
{
    VDEC_CHANNEL_S *pstChan;                            /* Channel structure pointer for vfmw*/
    VDEC_VPSSCHANNEL_S stVpssChan;                      /* vpss Channel structure */
    ulong u32File;                                     /* File handle */
    MT_BOOL bUsed;                                      /* Busy or free */
    atomic_t atmUseCnt;                                 /* Channel use count, support multi user */
    atomic_t atmRlsFlag;                                /* Channel release flag */
    wait_queue_head_t stRlsQue;                         /* Release queue */
    EventCallBack eCallBack;                            /*for opentv5*/
    GetDmxHdlCallBack DmxHdlCallBack;                   /*for opentv5*/
    mt_u32 u32DynamicFsEn; /*Dynamic frame store flag*/ //l00273086
} VDEC_CHAN_ENTITY_S;

/* Global parameter */
typedef struct
{
    mt_u32 u32ChanNum;                                          /* Record vfmw channel num */
    VDEC_CAP_S stVdecCap;                                       /* Vfmw capability */
    VDEC_CHAN_ENTITY_S astChanEntity[MT_VDEC_MAX_INSTANCE_NEW]; /* Channel parameter */
    struct semaphore stSem;                                     /* Mutex */
    struct timer_list stTimer;
    atomic_t atmOpenCnt;                 /* Open times */
    MT_BOOL bReady;                      /* Init flag */
    MT_UNF_VCODEC_ATTR_S stDefCfg;       /* Default channel config */
    VDEC_REGISTER_PARAM_S *pstProcParam; /* VDEC Proc functions */
    DEMUX_EXPORT_FUNC_S *pDmxFunc;       /* Demux extenal functions */
    VPSS_EXPORT_FUNC_S *pVpssFunc;       /*VPSS external functions*/
    DISP_EXPORT_FUNC_S *pDispFunc;       /*DISP external functions*/
    FN_VDEC_Watermark pfnWatermark;      /* Watermark function */
    VDEC_EXPORT_FUNC_S stExtFunc;        /* VDEC extenal functions */
    struct task_struct *pVdecTask;       //l00273086
} VDEC_GLOBAL_PARAM_S;

/***************************** Global Definition *****************************/

/* VDEC全局屏蔽控制字 bit0: DynamicFs Mask Ctrl; other: reserve*/
mt_u32 MaskCtrlWord = 0;

/***************************** Static Definition *****************************/

static mt_s32 RefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_1024;
static mt_s32 DispFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_1024;

//static mt_s32 EnVcmp = 1;
static mt_s32 EnVcmp = 0;
static mt_s32 En2d = 1;

VDEC_FW_FUNCTION_S vdec_decoder_irq_fun;

ulong g_reg_vdec_base;
ulong g_reg_tsi_base;
static mt_u32 g_VdecPause = 0;
static mt_u8 last_sl_hdr_enable = 0;

#if (CFG_VFMW_ON_AVCPU == 0)
extern irqreturn_t VDEC_Decoder_Irq(int irq_number, void *vdec_irq_handle);
extern void vdec_get_fw_input_param(inputParam_t *in_param);
extern void request_vdec_irq(mt_u32 irq_number, void *vdec_irq_handle);
extern void release_vdec_irq(mt_u32 irq_number);
#endif

/* for Symphony, VES VDH buffer static allocated and never free */
#if (1 == CFG_VDEC_VDH_STATIC_ALLOCATE)

#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
static mmz_buffer_s g_stStaticVDHMMZ[MT_VDEC_MAX_INSTANCE_NEW]= {{0, 0, 0},{0, 0, 0}};
static MT_BOOL g_bVdecStaticVDHMMZUsed[MT_VDEC_MAX_INSTANCE_NEW] = {0, 0};
#else
static mmz_buffer_s g_stStaticVDHMMZ[MT_VDEC_MAX_INSTANCE_NEW]= {{0, 0, 0}};
static MT_BOOL g_bVdecStaticVDHMMZUsed[MT_VDEC_MAX_INSTANCE_NEW] = {0};
#endif
#endif

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
static mmz_buffer_s g_stUsrDataMMZ[MT_VDEC_MAX_INSTANCE_NEW] = {{0, 0, 0},{0, 0, 0}};
#else
static mmz_buffer_s g_stUsrDataMMZ[MT_VDEC_MAX_INSTANCE_NEW] = {{0, 0, 0}};
#endif
#endif

#ifdef MT_DUMP_LCEVC
static void *mt_dump_handle = NULL;
#endif

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
ssize_t Lcevc_data_push(mt_handle hHandle, Lcevc_SEIpipe *dev, MT_VDEC_LCEVC_DATA_S *lcevc_data);
ssize_t Lcevc_data_pop(mt_handle hHandle, Lcevc_SEIpipe *dev, MT_VDEC_LCEVC_DATA_S *lcevc_data);
Lcevc_SEIpipe * gPLcevc_pipe[MT_VDEC_MAX_INSTANCE_NEW];
struct mutex Lcevc_pipe_lock[MT_VDEC_MAX_INSTANCE_NEW];

#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
static mmz_buffer_s g_stLecvcDataMMZ[MT_VDEC_MAX_INSTANCE_NEW] = {{0, 0, 0},{0, 0, 0}};

#else
static mmz_buffer_s g_stLecvcDataMMZ[MT_VDEC_MAX_INSTANCE_NEW] = {{0, 0, 0}};
#endif

#endif


#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
static mmz_buffer_s g_stSlHdrMMZ[MT_VDEC_MAX_INSTANCE_NEW] =  {{0, 0, 0},{0, 0, 0}};
#else
static mmz_buffer_s g_stSlHdrMMZ[MT_VDEC_MAX_INSTANCE_NEW] =  {{0, 0, 0}};
#endif

static mt_s32 VDEC_RegChanProc(mt_s32 s32Num);
static mt_s32 VDEC_VPU_RegChanProc(mt_s32 s32VpuNum);
static mt_void VDEC_UnRegChanProc(mt_s32 s32Num);
static mt_void VDEC_VPU_UnRegChanProc(mt_s32 s32Num_vpu);
static mt_s32 VDEC_VPU_PTS_Free(mt_handle hHandle, mt_handle hHandle_vpu);
static mt_s32 VDEC_Chan_VpssRecvFrmBuf(VPSS_HANDLE hVpss, MT_DRV_VIDEO_FRAME_S *pstFrm);
static mt_s32 VDEC_Chan_VpssRlsFrmBuf(VPSS_HANDLE hVpss, MT_DRV_VIDEO_FRAME_S *pstFrm);

static mt_s32 VDEC_Chan_RecvVpssFrmBuf(mt_handle hVpss, MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm);

#if 1
static mt_void VDEC_ConvertFrm(MT_UNF_VCODEC_TYPE_E enType, VDEC_CHANNEL_S *pstChan,
                               VDEC_CHAN_STATE_S *pstChanState, IMAGE *pstImage, MT_DRV_VIDEO_FRAME_S *pstFrame);
#else
static mt_void VDEC_ConvertFrm(MT_UNF_VCODEC_TYPE_E enType, VDEC_CHANNEL_S *pstChan,
                               IMAGE *pstImage, MT_DRV_VIDEO_FRAME_S *pstFrame);
#endif
static mt_s32 VDEC_FindVdecHandleByVpssHandle(VPSS_HANDLE hVpss, mt_handle *hVdec);
static mt_u32 VDEC_ConverColorSpace(mt_u32 u32ColorSpace);
static MT_UNF_VIDEO_FRAME_PACKING_TYPE_E VDEC_ConverFrameType(MT_DRV_FRAME_TYPE_E eFrmType);

extern ulong mt_get_tsi_base(void);
extern ulong mt_get_vdec_base(void);

extern mt_u32 dmx_get_avsync_flag(mt_u8 ch);
extern mt_u32 dmx_get_esbuff_id(mt_handle hDmxCh);
extern ulong dmx_get_esbuff_start_addr(mt_u32 buff_id);
extern ulong dmx_get_esbuff_end_addr(mt_u32 buff_id);
extern ulong dmx_get_desc_start_addr(mt_u32 buff_id);
extern mt_u8* dmx_get_desc_start_vaddr(mt_handle hDmxCh);

extern ulong dmx_get_desc_end_addr(mt_u32 buff_id);

static VDEC_GLOBAL_PARAM_S s_stVdecDrv =
    {
        .atmOpenCnt = ATOMIC_INIT(0),
        .bReady = MT_FALSE,
        .stDefCfg =
            {
	        .enType = MT_UNF_VCODEC_TYPE_H264,
	        .enMode = MT_UNF_VCODEC_MODE_NORMAL,
	        .u32ErrCover = 100,
	        .bOrderOutput = 0,
	        .u32Priority = 15,
	        .u32ForceFrameRateFlag = 0
	    },
        .pstProcParam = MT_NULL,
        .pDmxFunc = MT_NULL,
        .pVpssFunc = MT_NULL,
		.pDispFunc = MT_NULL,
        .pfnWatermark = MT_NULL,
        .stExtFunc =
            {
	        .pfnVDEC_Suspend = (mt_void *)VDEC_DRV_Suspend,
	        .pfnVDEC_Resume = (mt_void *)VDEC_DRV_Resume,
	    },
        .pVdecTask = MT_NULL //l00273086
    };

/*********************************** Code ************************************/

//Bug 111156: down_interruptible() return -EINTR!
static inline int DOWN_SEM(struct semaphore *sem, int line)
{
	int ret = 0;

	do {
		//ret = down_interruptible(sem);
		ret = down_killable(sem);
		if (ret == 0) {
			break;
		} else {
			MT_ERR_VDEC("[VDEC-%d]Semaphore is not acquired try again!\n",line);
			continue;
		}
	} while (1);

	return ret;
}

#if (VDEC_DEBUG == 1)
static mt_void VDEC_PrintImage(IMAGE *pstImg)
{
    MT_FATAL_VDEC("<0>top_luma_phy_addr = 0x%08x \n", pstImg->top_luma_phy_addr);
    MT_FATAL_VDEC("<0>top_chrom_phy_addr = 0x%08x \n", pstImg->top_chrom_phy_addr);
    MT_FATAL_VDEC("<0>btm_luma_phy_addr = 0x%08x \n", pstImg->btm_luma_phy_addr);
    MT_FATAL_VDEC("<0>btm_chrom_phy_addr = 0x%08x \n", pstImg->btm_chrom_phy_addr);
    MT_FATAL_VDEC("<0>disp_width = %d \n", pstImg->disp_width);
    MT_FATAL_VDEC("<0>disp_height = %d \n", pstImg->disp_height);
    MT_FATAL_VDEC("<0>disp_center_x = %d \n", pstImg->disp_center_x);
    MT_FATAL_VDEC("<0>disp_center_y = %d \n", pstImg->disp_center_y);
    MT_FATAL_VDEC("<0>error_level = %d \n", pstImg->error_level);
    MT_FATAL_VDEC("<0>seq_cnt = %d \n", pstImg->seq_cnt);
    MT_FATAL_VDEC("<0>seq_img_cnt = %d \n", pstImg->seq_img_cnt);
    MT_FATAL_VDEC("<0>PTS = %lld \n", pstImg->PTS);
}

static mt_void VDEC_PrintFrmInfo(MT_UNF_VIDEO_FRAME_INFO_S *pstFrame)
{
    MT_FATAL_VDEC("<0>u32Height = %d\n", pstFrame->u32Height);
    MT_FATAL_VDEC("<0>u32Width = %d\n", pstFrame->u32Width);
    MT_FATAL_VDEC("<0>u32DisplayWidth = %d\n", pstFrame->u32DisplayWidth);
    MT_FATAL_VDEC("<0>u32DisplayHeight = %d\n", pstFrame->u32DisplayHeight);
    MT_FATAL_VDEC("<0>u32DisplayCenterX = %d\n", pstFrame->u32DisplayCenterX);
    MT_FATAL_VDEC("<0>u32DisplayCenterY = %d\n", pstFrame->u32DisplayCenterY);
}
#endif

static void dump_channel(VDEC_CHANNEL_S *pCh)
{
	//TODO...
}

static void dump_hdrinfo(MT_UNF_VIDEO_DISP_HDR_INFO_S *pHdr)
{
	MT_ERR_VDEC("  Dump HDR Info(%p)::\n",pHdr);
    MT_ERR_VDEC("                  colour_range: %u\n",pHdr->colour_range);
    MT_ERR_VDEC("              colour_primaries: %u\n",pHdr->colour_primaries);
	MT_ERR_VDEC("      transfer_characteristics: %u\n",pHdr->transfer_characteristics);
	MT_ERR_VDEC("                  colour_space: %u\n",pHdr->colour_space);
    MT_ERR_VDEC("               chroma_location: %u\n",pHdr->chroma_location);
	MT_ERR_VDEC("               max_light_level: %u\n",pHdr->max_light_level);
	MT_ERR_VDEC("       max_pic_ave_light_level: %u\n",pHdr->max_pic_ave_light_level);
	MT_ERR_VDEC("                 has_primaries: %u\n",pHdr->has_primaries);
    MT_ERR_VDEC("                 has_luminance: %u\n",pHdr->has_luminance);
	MT_ERR_VDEC("      primary_r_chromaticity_x: %u\n",pHdr->primary_r_chromaticity_x);
    MT_ERR_VDEC("      primary_r_chromaticity_y: %u\n",pHdr->primary_r_chromaticity_y);
    MT_ERR_VDEC("      primary_g_chromaticity_x: %u\n",pHdr->primary_g_chromaticity_x);
    MT_ERR_VDEC("      primary_g_chromaticity_y: %u\n",pHdr->primary_g_chromaticity_y);
    MT_ERR_VDEC("      primary_b_chromaticity_x: %u\n",pHdr->primary_b_chromaticity_x);
    MT_ERR_VDEC("      primary_b_chromaticity_y: %u\n",pHdr->primary_b_chromaticity_y);
	MT_ERR_VDEC("    white_point_chromaticity_x: %u\n",pHdr->white_point_chromaticity_x);
    MT_ERR_VDEC("    white_point_chromaticity_y: %u\n",pHdr->white_point_chromaticity_y);
	MT_ERR_VDEC("                 max_luminance: %u\n",pHdr->max_luminance);
    MT_ERR_VDEC("                 min_luminance: %u\n",pHdr->min_luminance);
}

//local disp func wrapper
static mt_s32 disp_get_dce_percent_wrap(mt_s32 ChanID, mt_u32 *pDce)
{
	mt_s32 s32Ret = MT_SUCCESS;

	if (s_stVdecDrv.pDispFunc && s_stVdecDrv.pDispFunc->disp_get_dce_percent)
	{
		s32Ret = (s_stVdecDrv.pDispFunc->disp_get_dce_percent)(ChanID, pDce);
	}
	else
	{
		MLOGE("disp_get_dce_percent is null!\n");
	}

    return s32Ret;
}

static inline mt_s32 VDEC_CHAN_TRY_USE_DOWN_HELP(VDEC_CHAN_ENTITY_S *pstEnt)
{
    atomic_inc(&pstEnt->atmUseCnt);
    if (atomic_read(&pstEnt->atmRlsFlag) != 0) {
	atomic_dec(&pstEnt->atmUseCnt);
	if (atomic_read(&pstEnt->atmRlsFlag) != 1) {
	    MT_ERR_VDEC("Use lock err\n");
	    //while (1)
	    //{}
	}

	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static inline mt_s32 VDEC_CHAN_USE_UP_HELP(VDEC_CHAN_ENTITY_S *pstEnt)
{
    if (atomic_dec_return(&pstEnt->atmUseCnt) < 0) {
	MT_ERR_VDEC("Use unlock err\n");
	//while (1)
	//{}
    }
    return MT_SUCCESS;
}

static inline mt_s32 VDEC_CHAN_RLS_DOWN_HELP(VDEC_CHAN_ENTITY_S *pstEnt, mt_u32 time)
{
    mt_s32 s32Ret;

    /* Realse all */
    /* CNcomment:多个进行释放 */
    if (atomic_inc_return(&pstEnt->atmRlsFlag) != 1) {
	atomic_dec(&pstEnt->atmRlsFlag);
	return MT_FAILURE;
    }
    if (atomic_read(&pstEnt->atmUseCnt) != 0) {
	if (MT_INVALID_TIME == time) {
	    s32Ret = wait_event_interruptible(pstEnt->stRlsQue, (atomic_read(&pstEnt->atmUseCnt) == 0));
	} else {
	    s32Ret = wait_event_interruptible_hrtimeout(pstEnt->stRlsQue, (atomic_read(&pstEnt->atmUseCnt) == 0), ms_to_ktime(time));
	}

	if (s32Ret == 0) {
	    return MT_SUCCESS;
	} else {
	    if (s32Ret < 0) {
		return MT_SUCCESS;
	    }
	    atomic_dec(&pstEnt->atmRlsFlag);
	    return MT_FAILURE;
	}
    }

    return MT_SUCCESS;
}

static inline mt_s32 VDEC_CHAN_RLS_UP_HELP(VDEC_CHAN_ENTITY_S *pstEnt)
{
    if (atomic_dec_return(&pstEnt->atmRlsFlag) < 0) {
	//while (1)
	//{}
    }

    return MT_SUCCESS;
}

/* 初始化互斥锁*/
mt_s32 VDEC_InitSpinLock(VDEC_PORT_FRAME_LIST_LOCK_S *pIntrMutex)
{
    spin_lock_init(&pIntrMutex->irq_lock);
    pIntrMutex->isInit = MT_TRUE;
    return MT_SUCCESS;
}
/* 中断互斥加锁(关中断且加锁)*/
mt_s32 VDEC_SpinLockIRQ(VDEC_PORT_FRAME_LIST_LOCK_S *pIntrMutex)
{
    if (pIntrMutex->isInit == MT_FALSE) {
	spin_lock_init(&pIntrMutex->irq_lock);
	pIntrMutex->isInit = MT_TRUE;
    }
    spin_lock_irqsave(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);

    return MT_SUCCESS;
}
/* 中断互斥解锁(开中断且去锁)*/
mt_s32 VDEC_SpinUnLockIRQ(VDEC_PORT_FRAME_LIST_LOCK_S *pIntrMutex)
{
    if (pIntrMutex->isInit == MT_TRUE) {
	spin_unlock_irqrestore(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);
    }
    return MT_SUCCESS;
}

/* 初始化互斥锁*/
mt_s32 BUFMNG_InitSpinLock(BUFMNG_VPSS_IRQ_LOCK_S *pIntrMutex)
{
    spin_lock_init(&pIntrMutex->irq_lock);
    pIntrMutex->isInit = MT_TRUE;
    return MT_SUCCESS;
}
/* 中断互斥加锁(关中断且加锁)*/
mt_s32 BUFMNG_SpinLockIRQ(BUFMNG_VPSS_IRQ_LOCK_S *pIntrMutex)
{
    if (pIntrMutex->isInit == MT_FALSE) {
	spin_lock_init(&pIntrMutex->irq_lock);
	pIntrMutex->isInit = MT_TRUE;
    }
    spin_lock_irqsave(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);

    return MT_SUCCESS;
}
/* 中断互斥解锁(开中断且去锁)*/
mt_s32 BUFMNG_SpinUnLockIRQ(BUFMNG_VPSS_IRQ_LOCK_S *pIntrMutex)
{
    if (pIntrMutex->isInit == MT_TRUE) {
	spin_unlock_irqrestore(&pIntrMutex->irq_lock, pIntrMutex->irq_lockflags);
    }
    return MT_SUCCESS;
}
static mt_s32 BUFMNG_VPSS_Init(BUFMNG_VPSS_INST_S *pstBufVpssInst)
{
    mt_s32 i;
    BUFMNG_VPSS_NODE_S *pstBufNode;
    BUFMNG_VPSS_NODE_S *pstTarget;
    struct list_head *pos, *n;
    mt_s32 s32Ret = MT_SUCCESS;
    ENTER_FUNCTION;
    //s32Ret  = BUFMNG_InitSpinLock(&pstBufVpssInst->stAvailableListLock);
    memset(&pstBufVpssInst->stUnAvailableListLock, 0, sizeof(BUFMNG_VPSS_IRQ_LOCK_S));
    s32Ret = BUFMNG_InitSpinLock(&pstBufVpssInst->stUnAvailableListLock);
    INIT_LIST_HEAD(&pstBufVpssInst->stVpssBufAvailableList);
    INIT_LIST_HEAD(&pstBufVpssInst->stVpssBufUnAvailableList);
    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }
    if (MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer) {
	for (i = 0; i < pstBufVpssInst->u32BufNum; i++) {
	    pstBufNode = MT_VMALLOC_BUFMNG(sizeof(BUFMNG_VPSS_NODE_S));

	    if (MT_NULL == pstBufNode) {
		MT_VFREE_BUFMNG(pstBufNode);
		MT_ERR_VDEC("BUFMNG_VPSS_Init No memory.\n");
		return MT_ERR_BM_NO_MEMORY;
	    }
	    s32Ret = mt_drv_mmz_alloc_and_map("VDEC_VPSSBuf", MMZ_OTHERS, pstBufVpssInst->u32BufSize, 0, &pstBufNode->stMMZBuf);
	    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("BUFMNG_VPSS_Init Alloc MMZ fail:0x%x.\n", s32Ret);
		list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufAvailableList))
		{
		    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
		    mt_drv_mmz_unmap_and_release(&(pstTarget->stMMZBuf));
		    list_del_init(pos);
		    MT_VFREE_BUFMNG(pstTarget);
		}
		MT_VFREE_BUFMNG(pstBufNode);
		return MT_ERR_BM_NO_MEMORY;
	    }
	    BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	    pstBufNode->bBufUsed = MT_FALSE;
	    pstBufVpssInst->u32AvaiableFrameCnt = 0;
	    list_add_tail(&(pstBufNode->node), &(pstBufVpssInst->stVpssBufAvailableList));
	    BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	}
    } else if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer) {
#if 0
        pstBufVpssInst->stBufferAttr.u32BufNum = 10;
        for(i=0;i<pstBufVpssInst->stBufferAttr.u32BufNum;i++)
        {
            pstBufNode = MT_VMALLOC_BUFMNG(sizeof(BUFMNG_VPSS_NODE_S));
            if (MT_NULL == pstBufNode)
            {
                MT_VFREE_BUFMNG(pstBufNode);
                MT_ERR_VDEC("BUFMNG_VPSS_Init No memory.\n");
                return MT_ERR_BM_NO_MEMORY;
            }
#if 1
            s32Ret = mt_drv_mmz_alloc_and_map("VDEC_ALLOC_VPSSBuf", "VDEC", 3*1024*1024, 0, &pstBufNode->stMMZBuf);
#else
            pstBufNode->stMMZBuf.u32StartPhyAddr = pstBufVpssInst->stBufferAttr.u32PhyAddr[i];
            pstBufNode->stMMZBuf.u32StartVirAddr = pstBufVpssInst->stBufferAttr.u32UsrVirAddr[i];
            pstBufNode->stMMZBuf.u32Size         = pstBufVpssInst->stBufferAttr.u32BufSize;
#endif
            BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
            pstBufNode->bBufUsed = MT_FALSE;
            pstBufVpssInst->u32AvaiableFrameCnt = 0;
            list_add_tail(&(pstBufNode->node), &(pstBufVpssInst->stVpssBufAvailableList));
            BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
        }
#endif
#if 0
        VDEC_BUFFER_ATTR_S stBufferAttr;
        VDEC_Chan_SetExtBuffer(0,&stBufferAttr);
#endif
	BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	pstBufVpssInst->u32AvaiableFrameCnt = 0;
	pstBufVpssInst->u32BufNum = 0;
	pstBufVpssInst->enExtBufferState = VDEC_EXTBUFFER_STATE_START;
	BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
    }
    pstBufVpssInst->pstUnAvailableListPos = &pstBufVpssInst->stVpssBufUnAvailableList;
    return s32Ret;
}
static mt_s32 BUFMNG_VPSS_DeInit(BUFMNG_VPSS_INST_S *pstBufVpssInst)
{
    mt_s32 s32Ret = MT_SUCCESS;
    struct list_head *pos, *n;
    BUFMNG_VPSS_NODE_S *pstTarget;
    ENTER_FUNCTION;
    if (MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer) {
	list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufAvailableList))
	{
	    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	    mt_drv_mmz_unmap_and_release(&(pstTarget->stMMZBuf));
	    list_del_init(pos);
	    vfree(pstTarget);
	}

	list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufUnAvailableList))
	{
	    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	    mt_drv_mmz_unmap_and_release(&(pstTarget->stMMZBuf));
	    list_del_init(pos);
	    vfree(pstTarget);
	}
    }
    if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer) {
	BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	pstBufVpssInst->u32AvaiableFrameCnt = 0;
	pstBufVpssInst->u32BufNum = 0;
	pstBufVpssInst->u32BufSize = 0;
	pstBufVpssInst->enExtBufferState = VDEC_EXTBUFFER_STATE_START;
	BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
    }
    return s32Ret;
}

static mt_s32 BUFMNG_VPSS_Reset(BUFMNG_VPSS_INST_S *pstBufVpssInst)
{
    ENTER_FUNCTION;
    return MT_SUCCESS;
#if 0
    BUFMNG_VPSS_NODE_S* pstTarget;
    struct list_head *pos,*n;
    mt_s32 s32Ret = MT_SUCCESS;

    if(MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer)
    {
        BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
        list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufUnAvailableList))
        {
             pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
             list_del_init(pos);
             pstTarget->bBufUsed = MT_FALSE;
             list_add_tail(&(pstTarget->node), &(pstBufVpssInst->stVpssBufAvailableList));
        }
        pstBufVpssInst->u32AvaiableFrameCnt = 0;
        pstBufVpssInst->pstUnAvailableListPos = &pstBufVpssInst->stVpssBufUnAvailableList;
        BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
    }
    if(MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == pstBufVpssInst->enFrameBuffer)
    {
        BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
        list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufUnAvailableList))
        {
             pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
             if(pstTarget->enFrameBufferState != MT_DRV_VDEC_BUF_STATE_IN_USER)
             {
                 pstTarget->bBufUsed = MT_FALSE;
                 pstTarget->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_VDEC_EMPTY;
                 if(pstBufVpssInst->pstUnAvailableListPos == pos)
                 {
                    pstBufVpssInst->pstUnAvailableListPos = pos->prev;
                 }
                 list_del_init(pos);
                 list_add_tail(&(pstTarget->node), &(pstBufVpssInst->stVpssBufAvailableList));
             }
             else
             {
                 pstBufVpssInst->pstUnAvailableListPos = pos;
             }
        }
        pstBufVpssInst->u32AvaiableFrameCnt=0;
        BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
    }
    return s32Ret;
#endif
}
static mt_s32 BUFMNG_VPSS_CheckAvaibleBuffer(mt_handle hVpss, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort || MT_INVALID_HANDLE == hVpss) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_ERR_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	if (list_empty(&pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList)) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	    return MT_FAILURE;
	}
	BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}

static mt_s32 BUFMNG_VPSS_RecBuffer(mt_handle hVpss, mt_handle hPort, mmz_buffer_s *pstMMZ_Buffer)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    struct list_head *pos;
    BUFMNG_VPSS_NODE_S *pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort || MT_INVALID_HANDLE == hVpss) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    /*首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    /*查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    }

    /*此状态有上层控制当设置为STOP状态的时候意味不允许VPSS使用外部帧存*/
    if (VDEC_EXTBUFFER_STATE_START != pstVpssChan->stPort[0].stBufVpssInst.enExtBufferState) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    }
    /*获取帧存*/
    if (pstVpssChan->stPort[j].stBufVpssInst.u32BufSize != pstMMZ_Buffer->size) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
//	MT_WARN_VDEC("the frame size is change,before change:%d vpss want:%d\n", pstVpssChan->stPort[j].stBufVpssInst.u32BufSize, pstMMZ_Buffer->size);
	return MT_FAILURE;
    }
    BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
    if ((&pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList) == pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList.next) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	return MT_FAILURE;
    }
    pos = pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList.next;
    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
    pstTarget->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_VPSS;
    /*删除stVpssBufAvailableList上的pos节点*/
    list_del_init(pos);
    pstTarget->bBufUsed = MT_TRUE;
    /*将pos节点添加到stVpssBufUnAvailableList上去*/
    list_add_tail(pos, &(pstVpssChan->stPort[j].stBufVpssInst.stVpssBufUnAvailableList));
    memcpy(pstMMZ_Buffer, &pstTarget->stMMZBuf, sizeof(mmz_buffer_s));
    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}
static mt_s32 BUFMNG_VPSS_RelBuffer(mt_handle hVpss, mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstImage)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    struct list_head *pos, *n;
    BUFMNG_VPSS_NODE_S *pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort || MT_INVALID_HANDLE == hVpss) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    /*查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	/*释放帧存*/
	//pos从stVpssBufUnAvailableList的next开始
	BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	list_for_each_safe(pos, n, &(pstVpssChan->stPort[j].stBufVpssInst.stVpssBufUnAvailableList))
	{
	    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	    if (pstImage->stBufAddr[0].u32PhyAddr_Y == pstTarget->stMMZBuf.startPhyAddr) {
		//pstImage->stBufAddr[0].u32PhyAddr_Y,pstTarget->stMMZBuf.u32StartPhyAddr);
		pstTarget->bBufUsed = MT_FALSE;
		pstTarget->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_VDEC_EMPTY;
		/*删除stVpssBufUnAvailableList上的pos节点*/
		if (pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos == pos) {
		    pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos = pos->prev;
		}
		list_del_init(pos);
		/*将pos节点添加到stVpssBufAvailableList上去*/
		list_add_tail(&(pstTarget->node), &(pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList));
		break;
	    }
	}
	BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}
static mt_s32 VDEC_VpssNewImageEvent(mt_handle hVpss, mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstImage)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    struct list_head *pos, *n;
    BUFMNG_VPSS_NODE_S *pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort || MT_INVALID_HANDLE == hVpss) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    /*查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	list_for_each_safe(pos, n, &(pstVpssChan->stPort[j].stBufVpssInst.stVpssBufUnAvailableList))
	{
	    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	    if (pstTarget->stMMZBuf.startPhyAddr == pstImage->stBufAddr[0].u32PhyAddr_Y) {
		memcpy(&pstTarget->stVpssOutFrame, pstImage, sizeof(MT_DRV_VIDEO_FRAME_S));
		pstTarget->bBufUsed = MT_FALSE;
		pstTarget->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_VDEC_FULL;
		pstVpssChan->stPort[j].stBufVpssInst.u32AvaiableFrameCnt++;
		break;
	    }
	}
	BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}
static mt_s32 VDEC_FindVdecHandleByVpssHandle(VPSS_HANDLE hVpss, mt_handle *phVdec)
{
    mt_u32 i;

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    } else {
	if (MT_INVALID_HANDLE == s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec) {
	    /*MT_ERR_VDEC("Invalid hVdec!\n");*/
	    return MT_FAILURE;
	}
	*phVdec = s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec;
	return MT_SUCCESS;
    }
}

static mt_s32 VDEC_FindVdecHandleByPortHandle(mt_handle hPort, mt_handle *phVdec)
{
    mt_u32 i, j;

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	    if (hPort == s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].hPort &&
	        MT_TRUE == s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].bEnable) {
		break;
	    }
	}
	if (j < VDEC_MAX_PORT_NUM) {
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
		MT_ERR_VDEC("Can't find vdec :%s %d!\n", __FUNCTION__, __LINE__);
		return MT_FAILURE;
    } else {
	if (MT_INVALID_HANDLE == s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec) {
	    /*MT_ERR_VDEC("Invalid hVdec!\n");*/
	    return MT_FAILURE;
	}
	*phVdec = s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec;
	return MT_SUCCESS;
    }
}

mt_s32 VDEC_FindVpssHandleByVdecHandle(mt_handle hVdec, mt_handle *phVpss)
{
    mt_u32 i;
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVdec == s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    } else {
	*phVpss = s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss;
	return MT_SUCCESS;
    }
}

mt_s32 VDEC_FindVdecHandleByESBufferHandle(mt_handle hSteeamBufferHandle, mt_handle *phVdec)
{
    mt_u32 i;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	pstChan = s_stVdecDrv.astChanEntity[i].pstChan;
	if (MT_NULL == pstChan) {
	    continue;
	}
	if (hSteeamBufferHandle == pstChan->hStrmBuf) {
	    *phVdec = i;
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/* Find VFMW's Channel ID by ES Buffer Handle */
mt_s32 VDEC_FindChanIDByESBufferHandle(mt_handle hSteeamBufferHandle, mt_s32 *pChanID)
{
    mt_u32 i;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	pstChan = s_stVdecDrv.astChanEntity[i].pstChan;
	if (MT_NULL == pstChan) {
	    continue;
	}
	if (hSteeamBufferHandle == pstChan->hStrmBuf) {
	    *pChanID = (mt_s32)pstChan->hChan;
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 VDEC_CheckVpssPortOutFrameStatus(mt_handle hVdec)
{
    mt_s32 i = 0;
    mt_handle hMASTER = MT_INVALID_HANDLE;
    mt_handle hSLAVE = MT_INVALID_HANDLE;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    ENTER_FUNCTION;
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan);

    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[i].hPort) && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
	    if ((MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE == pstVpssChan->stPort[i].bufferType) && ((VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[i].enPortType) || (VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[i].enPortType))) {
		if (VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[i].enPortType) {
		    hMASTER = pstVpssChan->stPort[i].hPort;
		}

		if (VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[i].enPortType) {
		    hSLAVE = pstVpssChan->stPort[i].hPort;
		}
	    }
	}
    }

    if (hMASTER != MT_INVALID_HANDLE || hSLAVE != MT_INVALID_HANDLE) {
	MT_DRV_VPSS_PORT_AVAILABLE_S stCanGetFrm;

	if (hMASTER != MT_INVALID_HANDLE) {
	    stCanGetFrm.hPort = hMASTER;
	    stCanGetFrm.bAvailable = MT_FALSE;
	    s_stVdecDrv.pVpssFunc->pfnVpssSendCommand(pstVpssChan->hVpss,
	                                              MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE, &(stCanGetFrm));
	    if (stCanGetFrm.bAvailable == MT_FALSE) {
		return MT_FAILURE;
	    }
	}

	if (hSLAVE != MT_INVALID_HANDLE) {
	    stCanGetFrm.hPort = hSLAVE;
	    stCanGetFrm.bAvailable = MT_FALSE;
	    s_stVdecDrv.pVpssFunc->pfnVpssSendCommand(pstVpssChan->hVpss,
	                                              MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE, &(stCanGetFrm));
	    if (stCanGetFrm.bAvailable == MT_FALSE) {
		return MT_FAILURE;
	    }
	}
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_CheckPortFrameListStatus(mt_handle hVdec)
{
    mt_s32 i = 0;
    mt_handle hPort = MT_INVALID_HANDLE;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan);

    ENTER_FUNCTION;
    /*check list frame status, if the master port and slave port enable then must ensure there exists data in the framelist*/
    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[i].hPort) && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
	    if (MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE == pstVpssChan->stPort[i].bufferType) {
		if ((MT_TRUE == pstVpssChan->stPort[i].bEnable) &&
		    ((VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[i].enPortType) ||
		     (VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[i].enPortType))) {
		    hPort = pstVpssChan->stPort[i].hPort;
		    if (MT_INVALID_HANDLE != hPort) {
			if ((&(pstVpssChan->stPort[i].stPortList.stVdecPortFrameList)) ==
			    (pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next)) {
			    return MT_FAILURE;
			}
		    }
		} else {
		    if ((MT_TRUE == pstVpssChan->stPort[i].bEnable) && (VDEC_PORT_TYPE_VIRTUAL == pstVpssChan->stPort[i].enPortType)) {
			if ((&pstVpssChan->stPort[i].stPortList.stVdecPortFrameList) !=
			    (pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next)) {
			    return MT_SUCCESS;
			}
		    }
		}
	    }
	}
    }

    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_RecvVdecPortListFrame(mt_handle hVdec, MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_s32 i = 0;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    VDEC_PORT_FRAME_LIST_LOCK_S *pListLock = MT_NULL;
    VDEC_PORT_FRAME_LIST_NODE_S *pstListNode = MT_NULL;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVdec) || (MT_NULL == pstFrm)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    pstFrm->u32FrmNum = 0;
    s32Ret = VDEC_CheckPortFrameListStatus(hVdec);
    if (MT_SUCCESS == s32Ret) {
	pstVpssChan = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan);
	for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	    pListLock = &pstVpssChan->stPort[i].stPortList.stPortFrameListLock;
	    if ((MT_INVALID_HANDLE != pstVpssChan->stPort[i].hPort) && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
		VDEC_SpinLockIRQ(pListLock);
		if ((&pstVpssChan->stPort[i].stPortList.stVdecPortFrameList) != (pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next)) {
		    pstListNode = list_entry(pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next, VDEC_PORT_FRAME_LIST_NODE_S, node);
		    memcpy((void *)&(pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo), &(pstListNode->stPortOutFrame), sizeof(MT_DRV_VIDEO_FRAME_S));
		    pstFrm->stFrame[pstFrm->u32FrmNum].hport = pstVpssChan->stPort[i].hPort;
		    pstFrm->u32FrmNum++;
		    list_del_init(&(pstListNode->node));
		    MT_KFREE_BUFMNG(pstListNode);
		    VDEC_SpinUnLockIRQ(pListLock);
		} else {
		    VDEC_SpinUnLockIRQ(pListLock);
		    continue;
		}
	    }
	}
    } else {
	return MT_FAILURE;
    }

    if (pstFrm->u32FrmNum > 0) {
	return MT_SUCCESS;
    } else {
	return MT_FAILURE;
    }
}
static mt_s32 VDEC_InsertTmpListIntoPortList(mt_handle hVdec, mt_handle hPort)
{
    mt_s32 i = 0;
    mt_s32 j = 0;
    struct list_head *pstVdecPortFrameList;
    struct list_head *pnode;
    VDEC_PORT_FRAME_LIST_LOCK_S *pListLock = MT_NULL;
    VDEC_PORT_FRAME_LIST_NODE_S *pstVdecPortFrameListNode = MT_NULL;
    VDEC_PORT_FRAME_LIST_NODE_S *pastFrame[VDEC_MAX_PORT_FRAME] = { 0 };
    mt_u32 u32pastFrameIndex = 0;
    ENTER_FUNCTION;
    pListLock = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].stPortList.stPortFrameListLock);
    if ((MT_INVALID_HANDLE == hVdec) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    VDEC_SpinLockIRQ(pListLock);

    i = s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].s32PortTmpListPos - 1;
    if (i < 0) {
	VDEC_SpinUnLockIRQ(pListLock);
	return MT_FAILURE;
    }
    /*s32LastValidPos = s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].s32PortTmpListPos;  */
    pstVdecPortFrameList = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].stPortList.stVdecPortFrameList);

    for (; i >= 0; i--) {
	pstVdecPortFrameListNode = MT_KMALLOC_ATOMIC_BUFMNG(sizeof(VDEC_PORT_FRAME_LIST_NODE_S));
	if (MT_NULL == pstVdecPortFrameListNode) {
	    MT_ERR_VDEC("VDEC_InsertTmpListIntoPortList No memory.\n");
	    for (j = 0; j < u32pastFrameIndex; j++) {
		if (MT_NULL != pastFrame[j]) {
		    MT_KFREE_BUFMNG(pastFrame[j]);
		}
	    }
	    return MT_ERR_BM_NO_MEMORY;
	} else {
	    if (u32pastFrameIndex < VDEC_MAX_PORT_FRAME) {
		pastFrame[u32pastFrameIndex] = pstVdecPortFrameListNode;
		u32pastFrameIndex++;
	    }
	}
	memcpy(&(pstVdecPortFrameListNode->stPortOutFrame), &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].astPortTmpList[i].stPortOutFrame), sizeof(MT_DRV_VIDEO_FRAME_S));
	pnode = &(pstVdecPortFrameListNode->node);
	list_add_tail(pnode, pstVdecPortFrameList);
    }
    /*memcpy(&(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].astPortTmpList[0]),
           &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].astPortTmpList[s32LastValidPos]),
           sizeof(VDEC_PORT_FRAME_LIST_NODE_S));*/
    s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].s32PortTmpListPos = 0;

    VDEC_SpinUnLockIRQ(pListLock);

    return MT_SUCCESS;
}

static mt_s32 VDEC_InsertFrameIntoTmpList(mt_handle hVdec, mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Index = 0;
    //VDEC_PORT_FRAME_LIST_NODE_S *pstVdecPortFrameListNode = MT_NULL;
    VDEC_PORT_FRAME_LIST_LOCK_S *pListLock = MT_NULL;

    ENTER_FUNCTION;
    pListLock = &(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].stPortList.stPortFrameListLock);

    VDEC_SpinLockIRQ(pListLock);
    //pstVdecPortFrameListNode = MT_KMALLOC_ATOMIC_BUFMNG(sizeof(VDEC_PORT_FRAME_LIST_NODE_S));
    //if(MT_NULL == pstVdecPortFrameListNode)
    //{
    //    MT_ERR_VDEC("MT_KMALLOC_ATOMIC_BUFMNG err!\n");
    //    return MT_FAILURE;
    //}
    s32Index = s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].s32PortTmpListPos;
    if (s32Index >= VDEC_MAX_PORT_FRAME) {
	MT_ERR_VDEC("Invalid List Index :%d!\n", s32Index);
	VDEC_SpinUnLockIRQ(pListLock);
	return MT_FAILURE;
    }

    memcpy(&(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].astPortTmpList[s32Index].stPortOutFrame),
           pstFrame, sizeof(MT_DRV_VIDEO_FRAME_S));

    s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].s32PortTmpListPos++;

    VDEC_SpinUnLockIRQ(pListLock);

    return MT_SUCCESS;
}

static mt_s32 VDEC_RecvPortFrameFromVpss(mt_handle hVdec, mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hPortTmp = 0;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVdec) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    hPortTmp = s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[hPort].hPort;

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortFrame)(hPortTmp, pstFrame);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Get Port %d Frame err!\n", hPort);
	return MT_FAILURE;
    }

    return s32Ret;
}
static mt_s32 VDEC_EventHandle(mt_s32 s32ChanID, mt_s32 s32EventType, mt_void *pArgs)
{
    mt_s32 s32Ret;
    mt_handle hHandle;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    MT_VDEC_USRDAT_S *pstUsrData = MT_NULL;
    mt_u32 u32WriteID;
    mt_u32 u32ReadID;
    mt_u32 u32IStreamSize = 0;
    UNSUPPORT_SPEC_E e_SpecType;
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
#if (0 == MT_VDEC_CC_FROM_IMAGE)
    mt_u32 u32ID;
    mt_u8 u8Type;
#endif
#endif

    ENTER_FUNCTION;
    /* Find channel number */
    for (hHandle = 0; hHandle < MT_VDEC_MAX_INSTANCE_NEW; hHandle++) {
	if (s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    if (s_stVdecDrv.astChanEntity[hHandle].pstChan->hChan == s32ChanID) {
		break;
	    }
	}
    }
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("bad handle %d!\n", hHandle);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Event handle */
    switch (s32EventType) {
    case EVNT_DISP_EREA:
	/* !<Shall Implement>! */
    case EVNT_IMG_SIZE_CHANGE:
    	if (pstChan->enCurState == VDEC_CHAN_STATE_RUN)
    	{
			pstChan->bImageSizeChange = MT_TRUE;
			pstChan->bNoGetImage =MT_TRUE;
			MLOGD("%s: Image Size changed!\n",__FUNCTION__);
		}
		else
		{
			MLOGW("%s: Image Size changed but state is not RUN!\n",__FUNCTION__);
		}
	    break;
    case EVNT_FRMRATE_CHANGE:
    case EVNT_SCAN_CHANGE:
    case EVNT_ASPR_CHANGE:
	break;

    case EVNT_NEW_IMAGE:
	if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
#if 0
            MT_DRV_VIDEO_FRAME_S* pstLastFrm  = &(pstChan->stLastFrm);
            MT_DRV_VIDEO_FRAME_S stFrameInfo;
            memset(&stFrameInfo, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
            pstImg = (IMAGE*)(*(mt_u32*)pArgs);
            if (MT_NULL != pstImg)
            {
                VDEC_ConvertFrm(pstChan->stCurCfg.enType, pstChan,MT_NULL, pstImg, &stFrameInfo);

                /* Check norm change */
                if ((stFrameInfo.stDispRect.s32Height != pstLastFrm->stDispRect.s32Height) ||
                    (stFrameInfo.stDispRect.s32Width != pstLastFrm->stDispRect.s32Width) ||
                    (stFrameInfo.bProgressive != pstLastFrm->bProgressive))
                {
                    pstChan->bNormChange = MT_TRUE;
                    pstChan->stNormChangeParam.enNewFormat    = pstChan->enDisplayNorm;
                    pstChan->stNormChangeParam.u32ImageWidth  = stFrameInfo.u32Width;
                    pstChan->stNormChangeParam.u32ImageHeight = stFrameInfo.u32Height;
                    pstChan->stNormChangeParam.u32FrameRate = stFrameInfo.u32FrameRate;//stFrameRate.u32fpsInteger;
                    //pstChan->stNormChangeParam.stFrameRate.u32fpsDecimal = 0;//stFrameInfo.stFrameRate.u32fpsDecimal;
                    pstChan->stNormChangeParam.bProgressive = stFrameInfo.bProgressive;
                }
                //not same l00225186
                /* Check frame packing */
                //if (stFrameInfo.enFramePackingType != pstLastFrm->enFramePackingType)
                if (stFrameInfo.eFrmType != pstLastFrm->eFrmType)
                {
                    pstChan->bFramePackingChange = MT_TRUE;
                    //pstChan->enFramePackingType = stFrameInfo.enFramePackingType;
                    pstChan->enFramePackingType = stFrameInfo.eFrmType;
                }

                /* Save last frame */
                *pstLastFrm = stFrameInfo;
                //pstChan->bNewFrame = MT_TRUE;
            }
#endif
	    if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed < 0 || MT_TRUE == s_stVdecDrv.astChanEntity[hHandle].pstChan->bLowdelay) {
		(void)(s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_IMAGEREADY, NULL);
	    }
	}
	break;

    case EVNT_FIND_IFRAME:
	pstChan->stStatInfo.u32TotalVdecParseIFrame++;
	if (1 == pstChan->stStatInfo.u32TotalVdecParseIFrame) {
		if (pArgs != NULL)
		    u32IStreamSize = *(mt_u32 *)pArgs;
	    mt_drv_stat_event(STAT_EVENT_ISTREAMGET, u32IStreamSize);
	}
	break;

	case EVNT_LCEVCDAT_RDY:
	{
			MT_VDEC_LCEVC_DATA_S * lcevcData = (MT_VDEC_LCEVC_DATA_S*)((ulong *)pArgs);
			if(NULL != lcevcData)
			{
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
				Lcevc_data_push(hHandle, gPLcevc_pipe[hHandle], lcevcData);
#endif

#ifdef MT_DUMP_LCEVC
				mt_drv_dump_do(mt_dump_handle, lcevcData->data, lcevcData->data_size);
#endif
			}
			
	}
	break;
	
    case EVNT_USRDAT:
    if (pArgs != NULL)
    {
	pstUsrData = (MT_VDEC_USRDAT_S *)(*(ulong *)pArgs);

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
#if (0 == MT_VDEC_CC_FROM_IMAGE)
	if (pstUsrData->data_size > 5) {
	    u32ID = *((mt_u32 *)pstUsrData->data);
	    u8Type = pstUsrData->data[4];
	    if ((VDEC_USERDATA_IDENTIFIER_DVB1 == u32ID) && (VDEC_USERDATA_TYPE_DVB1_CC == u8Type)) {
#if (1 == VDEC_USERDATA_NEED_ARRANGE)
		USRDATA_Arrange(hHandle, pstUsrData);
#endif
		break;
	    }
	}
#endif
#endif

	if (MT_NULL == pstChan->pstUsrData) {
	    pstChan->pstUsrData = MT_KMALLOC_ATOMIC_VDEC(sizeof(VDEC_USRDATA_PARAM_S));
	    if (MT_NULL == pstChan->pstUsrData) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_ERR_VDEC("No memory\n");
		return MT_FAILURE;
	    }
	    memset(pstChan->pstUsrData, 0, sizeof(VDEC_USRDATA_PARAM_S));
	}

	/* Discard if the buffer of user data full */
	/* CNcomment: 如果用户数据buffer满就直接丢弃 */
	u32WriteID = pstChan->pstUsrData->u32WriteID;
	u32ReadID = pstChan->pstUsrData->u32ReadID;
	if ((u32WriteID + 1) % VDEC_UDC_MAX_NUM == u32ReadID) {
	    MT_INFO_VDEC("Chan %d drop user data\n", hHandle);
	    break;
	}

	pstChan->pstUsrData->stAttr[u32WriteID].enBroadcastProfile = MT_UNF_VIDEO_BROADCAST_DVB;
	pstChan->pstUsrData->stAttr[u32WriteID].enPositionInStream = MT_UNF_VIDEO_USER_DATA_POSITION_UNKNOWN;
	pstChan->pstUsrData->stAttr[u32WriteID].u32Pts = (mt_u32)pstUsrData->PTS;
	pstChan->pstUsrData->stAttr[u32WriteID].u32SeqCnt = 0;
	pstChan->pstUsrData->stAttr[u32WriteID].u32SeqFrameCnt = 0;
	pstChan->pstUsrData->stAttr[u32WriteID].bBufferOverflow = (pstUsrData->data_size > VDEC_KUD_MAX_LEN);
	pstChan->pstUsrData->stAttr[u32WriteID].pu8Buffer = pstChan->pstUsrData->au8Buf[u32WriteID];
	pstChan->pstUsrData->stAttr[u32WriteID].u32Length =
	    (pstUsrData->data_size > VDEC_KUD_MAX_LEN) ? MAX_USER_DATA_LEN : pstUsrData->data_size;
	memcpy(pstChan->pstUsrData->stAttr[u32WriteID].pu8Buffer, pstUsrData->data,
	       pstChan->pstUsrData->stAttr[u32WriteID].u32Length);
	pstChan->pstUsrData->u32WriteID = (u32WriteID + 1) % VDEC_UDC_MAX_NUM;
	MT_INFO_VDEC("Chan: %d get user data\n", hHandle);
	pstChan->bNewUserData = MT_TRUE;
	}
	break;

    case EVNT_VDM_ERR:
    case EVNT_SE_ERR:
	pstChan->stStatInfo.u32TotalStreamErrNum++;
	break;

    case EVNT_IFRAME_ERR:
	pstChan->bIFrameErr = MT_TRUE;
	break;

    /* Capture BTL over */
    case EVNT_CAPTURE_BTL_OVER: {
	IMAGE *pstImage;

	if ((MT_NULL != pstChan->stBTL.pstFrame) && (1 == atomic_read(&pstChan->stBTL.atmWorking))) {
		if (pArgs != NULL)
		{
		    pstImage = (IMAGE *)(*((ulong *)pArgs));
		    VDEC_ConvertFrm(pstChan->stCurCfg.enType, pstChan, MT_NULL, pstImage, pstChan->stBTL.pstFrame);
		}
	    atomic_dec(&pstChan->stBTL.atmWorking);
	    wake_up_interruptible(&(pstChan->stBTL.stWaitQue));
	}
	break;
    }

	/* !<Shall Implement>! */
    /* End frame */
    case EVNT_LAST_FRAME:
	/* *(mt_u32*)pArgs: 0 success, 1 fail,  2 report last frame image id */
	if (pArgs != NULL)
	{
		if (1 == *(mt_u32 *)pArgs) {
		    pstChan->u32EndFrmFlag = 1;
		} else if (2 <= *(mt_u32 *)pArgs) {
		    pstChan->u32EndFrmFlag = 2;
		    pstChan->u32LastFrmId = *(mt_u32 *)pArgs - 2;
		}
	}
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
	USRDATA_SetEosFlag(hHandle);
#endif
    pstChan->bIsLastFrame = MT_TRUE;
	pstChan->bEndOfStrm = MT_TRUE;
	MT_ALWAYS_PRINT( "%s: bIsLastFrame EOS reached!\n",__FUNCTION__);
	break;

    /* Resolution change */
    case EVNT_RESOLUTION_CHANGE:
	if (s_stVdecDrv.astChanEntity[hHandle].eCallBack)
	    s_stVdecDrv.astChanEntity[hHandle].eCallBack(hHandle, NULL, VIDDEC_EVT_STREAM_INFO_CHANGE);
	pstChan->u8ResolutionChange = 1;
	break;
    case EVNT_UNSUPPORT_SPEC: // UNSUPPORT类型的扩展，特殊处理后然后走EVNT_UNSUPPORT同样分支
	if (pArgs != NULL)
	{
		e_SpecType = ((mt_u32 *)pArgs)[0];
		switch (e_SpecType) {
		case SPEC_BIT_DEPTH:
		    pstChan->u32LastLumaBitdepth = ((mt_u32 *)pArgs)[1];
		    pstChan->u32LastChromaBitdepth = ((mt_u32 *)pArgs)[1];
		    break;
		default:
		    break;
		}
	}
	if (s_stVdecDrv.astChanEntity[hHandle].eCallBack)
	    s_stVdecDrv.astChanEntity[hHandle].eCallBack(hHandle, NULL, VIDDEC_EVT_UNSUPPORTED_STREAM_TYPE);
	pstChan->bUnSupportStream = 1;
	break;
    case EVNT_UNSUPPORT:
	if (s_stVdecDrv.astChanEntity[hHandle].eCallBack)
	    s_stVdecDrv.astChanEntity[hHandle].eCallBack(hHandle, NULL, VIDDEC_EVT_UNSUPPORTED_STREAM_TYPE);
	pstChan->bUnSupportStream = 1;
	break;
	case EVNT_RATIO_NOTZERO:
	{
		if (pArgs != NULL)
			pstChan->u32ErrRatio = *(mt_u32 *)pArgs;
		else
			pstChan->u32ErrRatio = 10; //sym2 can't get u32ErrRatio

	}
	break;
    case EVNT_NEED_ARRANGE: //获取码流信息,为帧存划分做准备.l00273086//
//FIX Kernel Info Log: No need this task!
#if 0
	wake_up_process(s_stVdecDrv.pVdecTask);
#endif
	pstChan->bNeedAlloc = MT_TRUE;
	if (pArgs != NULL)
	{
		pstChan->u32RefFrameNum = ((mt_u32 *)pArgs)[0];
		pstChan->u32FrameSize = ((mt_u32 *)pArgs)[1];
		//VPSS在对标清的流做去隔行操作时，会占用3帧
		if (((((mt_u32 *)pArgs)[2]) <= 1920) && ((((mt_u32 *)pArgs)[3]) <= 1088)) {
		    pstChan->u32RefFrameNum += 4;
		}
	}
	break;
    default:
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

//add by l00225186
/*vpss 回调函数*/
static mt_s32 VDEC_VpssEventHandle(mt_handle hVdec, MT_DRV_VPSS_EVENT_E enEventID, mt_void *pstArgs)
{
    mt_s32 s32Ret;
    mt_s32 i;
    mt_handle hHandle;
    mt_handle hPort = MT_INVALID_HANDLE;
    mt_handle hVpss = MT_INVALID_HANDLE;
    mmz_buffer_s stMMZ_Buffer;
    MT_DRV_VIDEO_FRAME_S stFrame;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = MT_NULL;
    MT_DRV_VPSS_PORT_BUFLIST_STATE_S stVpssBufListState;

    //ENTER_FUNCTION;
    /* Find channel number */
    for (hHandle = 0; hHandle < MT_VDEC_MAX_INSTANCE_NEW - 1; hHandle++) {
	if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVdec == hVdec) {
	    break;
	}
    }

    if (hHandle > MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("bad handle %d!\n", hHandle);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);

    /* Event handle */
    switch (enEventID) {
    case VPSS_EVENT_CHECK_FRAMEBUFFER:
	hPort = ((MT_DRV_VPSS_BUFINFO_S *)pstArgs)->hPort;
	hVpss = (mt_handle)PORTHANDLE_TO_VPSSID(hPort);
	s32Ret = BUFMNG_VPSS_CheckAvaibleBuffer(hVpss, hPort);
	if (MT_SUCCESS == s32Ret) {
	    ((MT_DRV_VPSS_BUFINFO_S *)pstArgs)->bAvailable = MT_TRUE;
	} else {
	    ((MT_DRV_VPSS_BUFINFO_S *)pstArgs)->bAvailable = MT_FALSE;
	}
	break;
    case VPSS_EVENT_GET_FRMBUFFER:
	hPort = ((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->hPort;
	hVpss = (mt_handle)PORTHANDLE_TO_VPSSID(hPort);
	memset(&stMMZ_Buffer, 0, sizeof(mmz_buffer_s));
	stMMZ_Buffer.size = ((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->u32Size;
	s32Ret = BUFMNG_VPSS_RecBuffer(hVpss, hPort, &stMMZ_Buffer);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_INFO_VDEC("Chan %d BUFMNG_VPSS_RecBuffer fail!\n", hHandle);
	    return MT_FAILURE;
	}
	((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->u32StartPhyAddr = stMMZ_Buffer.startPhyAddr;
	((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->u32StartVirAddr = (ulong) stMMZ_Buffer.startVirAddr;
	break;
    case VPSS_EVENT_REL_FRMBUFFER:
	hPort = ((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->hPort;
	hVpss = (mt_handle)PORTHANDLE_TO_VPSSID(hPort);
	stFrame.stBufAddr[0].u32PhyAddr_Y = ((MT_DRV_VPSS_FRMBUF_S *)pstArgs)->u32StartPhyAddr;
	BUFMNG_VPSS_RelBuffer(hVpss, hPort, &stFrame);
	break;
    case VPSS_EVENT_NEW_FRAME:
	s_stVdecDrv.astChanEntity[hHandle].pstChan->bNewFrame = MT_TRUE;
	if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed < 0) {
	    SINT32 s32VpssHaveFrame = 0;

	    s32VpssHaveFrame = VDEC_CheckVpssPortOutFrameStatus(hVdec);
	    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
		if (MT_SUCCESS == s32VpssHaveFrame) {
		    if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].hPort && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
			memset(&stFrame, 0, sizeof(stFrame));
			s32Ret = VDEC_RecvPortFrameFromVpss(hVdec, i, &stFrame);
			if (MT_SUCCESS == s32Ret) {
			    if (MT_TRUE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32DispOptimizeFlag) {
				if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].u32LastFrameIndex == stFrame.u32FrameIndex) {
				    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssRelPortFrame)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].hPort, &stFrame);
				    if (MT_SUCCESS != s32Ret) {
					MT_ERR_VDEC("vdec RelPortFrame err!\n");
				    }
				    continue;
				} else {
				    stFrame.u32FrameRate /= 2;
				}
			    }
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].u32LastFrameIndex = stFrame.u32FrameIndex;
			    pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(stFrame.u32Priv))->u32Reserve);
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame = 1;
			} else {
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame = 0;
			    continue;
			}
		    } else {
			continue;
		    }
		} else {
		    continue;
		}

		if (MT_FALSE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32BackwardOptimizeFlag) {
		    /*if get a new I frame then insert the tmpFrames to portFrameList from tail to head*/
		    if ((MT_FALSE != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstVpssFrameFlag) &&
		        (((0 == pstPrivInfo->s32FrameFormat) && (0 == pstPrivInfo->s32FieldFlag)) ||
		         (((0 == pstPrivInfo->s32BottomFieldFrameFormat) || (0 == pstPrivInfo->s32TopFieldFrameFormat)) && (1 == pstPrivInfo->s32FieldFlag)))) {
			VDEC_InsertTmpListIntoPortList(hHandle, i);
		    } else {
			if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame) {
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstVpssFrameFlag = MT_TRUE;
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.eLastFrameFormat = pstPrivInfo->s32FrameFormat;
			}
		    }
		} else {
		    hHandle &= 0x000000ff;
		    /*if get a new Gop_Num then insert the tmpFrames to portFrameList from tail to head*/
		    if ((MT_TRUE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32GetFirstVpssFrameFlag) &&
		        ((pstPrivInfo->s32GopNum) != (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32PortLastFrameGopNum)) &&
		        (0 < s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32PortTmpListPos)) {
			VDEC_InsertTmpListIntoPortList(hHandle, i);
		    }
		    /*insert the new frame into tmpFrames*/
		    //else
		    {
			if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame) {
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32GetFirstVpssFrameFlag = MT_TRUE;
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32PortLastFrameGopNum = pstPrivInfo->s32GopNum;
			}
		    }
		}

		if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame) {
		    VDEC_InsertFrameIntoTmpList(hHandle, i, &stFrame);
		    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame = 0;
		}
	    }
	} else {
	    if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.enFrameBuffer ||
	        MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.enFrameBuffer) {
		hPort = ((MT_DRV_VPSS_FRMINFO_S *)pstArgs)->hPort;
		for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
		    if (hPort == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].hPort) {
			break;
		    }
		}
		if (i >= VDEC_MAX_PORT_NUM) {
		    MT_ERR_VDEC("====ERR port handle\n");
		    break;
		}
		if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].bufferType) {
		    hPort = ((MT_DRV_VPSS_FRMINFO_S *)pstArgs)->hPort;
		    hVpss = (mt_handle)PORTHANDLE_TO_VPSSID(hPort);
		    s32Ret = VDEC_VpssNewImageEvent(hVpss, hPort, &((MT_DRV_VPSS_FRMINFO_S *)pstArgs)->stFrame);
		}
	    }
	}

	break;
#if 1
    case VPSS_EVENT_BUFLIST_FULL:
	for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	    if (VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[i].enPortType || VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[i].enPortType) {
		MT_DRV_VPSS_PORT_CFG_S stVpssPortCfg;
		s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortCfg)(pstVpssChan->stPort[i].hPort, &stVpssPortCfg);
		if (MT_SUCCESS == s32Ret) {
		    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == stVpssPortCfg.stBufListCfg.eBufType) {
			*(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_PAUSE;
			break;
		    } else {
			hPort = pstVpssChan->stPort[i].hPort;
			memset(&stVpssBufListState, 0, sizeof(MT_DRV_VPSS_PORT_BUFLIST_STATE_S));
			s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortBufListState)(hPort, &stVpssBufListState);
			if (MT_SUCCESS == s32Ret) {

			    /*master_port slave_port full:MT_DRV_VPSS_BUFFUL_PAUSE,master_port slave_port not full:MT_DRV_VPSS_BUFFUL_KEEPWORKING*/
			    if (stVpssBufListState.u32FulBufNumber > stVpssBufListState.u32TotalBufNumber - 2) {
				/*master_port slave_port full:MT_DRV_VPSS_BUFFUL_PAUSE*/
				*(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_PAUSE;
				break;
			    } else {
				/*master_port slave_port not full:MT_DRV_VPSS_BUFFUL_KEEPWORKING*/
				*(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_KEEPWORKING;
			    }
			}
		    }
		}
	    } else {
		//*(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_KEEPWORKING;
	    }
	}

	if (i >= VDEC_MAX_PORT_NUM) {
	    MT_WARN_VDEC("Port not exist!\n");
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    return MT_FAILURE;
	} else {
	}

	break;
#endif
#if 0
      case  VPSS_EVENT_BUFLIST_FULL:
        for(i = 0; i<VDEC_MAX_PORT_NUM; i++)
        {
            if(VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[i].enPortType
               || VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[i].enPortType)
            {
                hPort = pstVpssChan->stPort[i].hPort;
                break;
            }
            else
            {
                *(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_KEEPWORKING;
                break;
            }
        }

        if(i >= VDEC_MAX_PORT_NUM)
        {
            MT_WARN_VDEC("Port not exist!\n");
            VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
            return MT_FAILURE;
        }
        else
        {
            memset(&stVpssBufListState, 0, sizeof(MT_DRV_VPSS_PORT_BUFLIST_STATE_S));
            s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortBufListState)(hPort, &stVpssBufListState);
            if(MT_SUCCESS == s32Ret)
            {
                /*master_port slave_port full:MT_DRV_VPSS_BUFFUL_PAUSE,master_port slave_port not full:MT_DRV_VPSS_BUFFUL_KEEPWORKING*/
                if(stVpssBufListState.u32FulBufNumber == stVpssBufListState.u32TotalBufNumber)
                {
                     /*master_port slave_port full:MT_DRV_VPSS_BUFFUL_PAUSE*/
                     *(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_PAUSE;
                     break;
                }
                else
                {
                 /*master_port slave_port not full:MT_DRV_VPSS_BUFFUL_KEEPWORKING*/
                 *(MT_DRV_VPSS_BUFFUL_STRATAGY_E *)pstArgs = MT_DRV_VPSS_BUFFUL_KEEPWORKING;
                }
            }
        }

        break;
#endif
    default:
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

static VID_STD_E VDEC_CodecTypeUnfToFmw(MT_UNF_VCODEC_TYPE_E unfType)
{
    switch (unfType) {
    case MT_UNF_VCODEC_TYPE_MPEG2:
	return STD_MPEG2;
    case MT_UNF_VCODEC_TYPE_MPEG4:
	return STD_MPEG4;
    case MT_UNF_VCODEC_TYPE_AVS:
	return STD_AVS;
    case MT_UNF_VCODEC_TYPE_AVS2:
	return STD_AVS2;
    case MT_UNF_VCODEC_TYPE_H263:
	return STD_H263;
    case MT_UNF_VCODEC_TYPE_VP6:
	return STD_VP6;
    case MT_UNF_VCODEC_TYPE_VP6F:
	return STD_VP6F;
    case MT_UNF_VCODEC_TYPE_VP6A:
	return STD_VP6A;
    case MT_UNF_VCODEC_TYPE_VP8:
	return STD_VP8;
    case MT_UNF_VCODEC_TYPE_SORENSON:
	return STD_SORENSON;
    case MT_UNF_VCODEC_TYPE_H264:
	return STD_H264;
    case MT_UNF_VCODEC_TYPE_HEVC:
	return STD_HEVC;
    case MT_UNF_VCODEC_TYPE_REAL9:
	return STD_REAL9;
    case MT_UNF_VCODEC_TYPE_REAL8:
	return STD_REAL8;
    case MT_UNF_VCODEC_TYPE_VC1:
	return STD_VC1;
    case MT_UNF_VCODEC_TYPE_DIVX3:
	return STD_DIVX3;
    case MT_UNF_VCODEC_TYPE_MVC:
	return STD_MVC;
    case MT_UNF_VCODEC_TYPE_RAW:
	return STD_RAW;
    case MT_UNF_VCODEC_TYPE_MJPEG:
	return STD_USER;
    case MT_UNF_VCODEC_TYPE_VP9:
	return STD_VP9;
    default:
	return STD_END_RESERVED;
    }
}

static fw_video_coding_type_t VDEC_CodecTypeUnfToVfmw(MT_UNF_VCODEC_TYPE_E unfType)
{
    switch (unfType) {
    case MT_UNF_VCODEC_TYPE_MPEG2:
      return FW_VIDEO_TYPE_MPEG2;
  
    case MT_UNF_VCODEC_TYPE_MPEG4:
    case MT_UNF_VCODEC_TYPE_H263:
    case MT_UNF_VCODEC_TYPE_DIVX3:
      return FW_VIDEO_TYPE_MPEG4;

    case MT_UNF_VCODEC_TYPE_AVS:
      return FW_VIDEO_TYPE_AVS;
	  
	case MT_UNF_VCODEC_TYPE_AVS2:
		return FW_VIDEO_TYPE_AVS2;

    case MT_UNF_VCODEC_TYPE_VP8:
      return FW_VIDEO_TYPE_VP8;

    case MT_UNF_VCODEC_TYPE_H264:
      return FW_VIDEO_TYPE_H264;

    case MT_UNF_VCODEC_TYPE_HEVC:
      return FW_VIDEO_TYPE_HEVC;

    case MT_UNF_VCODEC_TYPE_REAL9:
    case MT_UNF_VCODEC_TYPE_REAL8:
      return FW_VIDEO_TYPE_RV34;

    case MT_UNF_VCODEC_TYPE_VC1:
      return FW_VIDEO_TYPE_VC1;

    case MT_UNF_VCODEC_TYPE_MVC:
      return FW_VIDEO_TYPE_MVC;

    case MT_UNF_VCODEC_TYPE_VP9:
      return FW_VIDEO_TYPE_VP9;

    default:
      return FW_VIDEO_TYPE_MPEG2;
    }
}

static mt_s32 VDEC_SetAttr(VDEC_CHANNEL_S *pstChan)
{
    mt_s32 s32Ret;
    VDEC_CHAN_CFG_S stVdecChanCfg;
    MT_UNF_VCODEC_ATTR_S *pstCfg = &pstChan->stCurCfg;
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    mt_u32 usrData_open_flag =0;
#endif

    ENTER_FUNCTION;

    memset(&stVdecChanCfg, 0, sizeof(VDEC_CHAN_CFG_S));
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_CFG, &stVdecChanCfg);
    if (VDEC_OK != s32Ret) {
    	MT_ERR_VDEC("VFMW GET_CHAN_CFG err!\n");
    	return MT_FAILURE;
    }
    stVdecChanCfg.s32ModuleLowlyEnable = pstChan->bLowdelay;
    stVdecChanCfg.s32LowdlyEnable = 0; //pstChan->bLowdelay;
    stVdecChanCfg.s32DecOrderOutput = pstCfg->bOrderOutput;

    stVdecChanCfg.eVidStd = VDEC_CodecTypeUnfToFmw(pstCfg->enType);
    if (MT_UNF_VCODEC_TYPE_VC1 == pstCfg->enType) {
    	if ((1 == pstChan->stOption.u32DynamicFrameStoreAllocEn) &&
    	    (0 == pstChan->stOption.u32SelfAdaptionDFS)) {
    	    pstChan->stOption.s32ExtraFrameStoreNum = 1;
    	}
    	stVdecChanCfg.StdExt.Vc1Ext.IsAdvProfile = pstCfg->unExtAttr.stVC1Attr.bAdvancedProfile;
    	stVdecChanCfg.StdExt.Vc1Ext.CodecVersion = (mt_s32)(pstCfg->unExtAttr.stVC1Attr.u32CodecVersion);
        if(!stVdecChanCfg.StdExt.Vc1Ext.IsAdvProfile)
            stVdecChanCfg.eVidStd = STD_VC1_SP_MAIN;
    } else if ((MT_UNF_VCODEC_TYPE_VP6 == pstCfg->enType) ||
               (MT_UNF_VCODEC_TYPE_VP6F == pstCfg->enType) ||
               (MT_UNF_VCODEC_TYPE_VP6A == pstCfg->enType)) {
	stVdecChanCfg.StdExt.Vp6Ext.bReversed = pstCfg->unExtAttr.stVP6Attr.bReversed;
    } else if (MT_UNF_VCODEC_TYPE_HEVC == pstCfg->enType) {
	if ((1 == pstChan->stOption.u32DynamicFrameStoreAllocEn) &&
	    (1 == pstChan->stOption.u32SelfAdaptionDFS)) {
	    //pstChan->stOption.s32ExtraFrameStoreNum += 1;
	}
    }
    stVdecChanCfg.s32ExtraFrameStoreNum = pstChan->stOption.s32ExtraFrameStoreNum;

    stVdecChanCfg.s32ChanPriority = pstCfg->u32Priority;
    stVdecChanCfg.s32ChanErrThr = pstCfg->u32ErrCover;
    switch (pstCfg->enMode) {
    case MT_UNF_VCODEC_MODE_NORMAL:
	stVdecChanCfg.s32DecMode = IPB_MODE;
	break;
    case MT_UNF_VCODEC_MODE_IP:
	stVdecChanCfg.s32DecMode = IP_MODE;
	break;
    case MT_UNF_VCODEC_MODE_I:
	stVdecChanCfg.s32DecMode = I_MODE;
	break;
    case MT_UNF_VCODEC_MODE_DROP_INVALID_B:
	stVdecChanCfg.s32DecMode = DISCARD_B_BF_P_MODE;
	break;
    case MT_UNF_VCODEC_MODE_BUTT:
    default:
	stVdecChanCfg.s32DecMode = IPB_MODE;
	break;
    }
    if (pstChan->bLowdelay) {
	stVdecChanCfg.s32DecMode = IP_MODE;
	pstCfg->enMode = MT_UNF_VCODEC_MODE_IP;
    }
    if (MT_TRUE == pstCfg->bOrderOutput) // normal/simple dpb mode
    {
	stVdecChanCfg.s32DecOrderOutput = pstCfg->bOrderOutput + pstCfg->s32CtrlOptions;
    } else // display mode
    {
	stVdecChanCfg.s32DecOrderOutput = 0;
    }

    stVdecChanCfg.s32ChanStrmOFThr = (pstChan->u32DmxBufSize * 95) / 100;
    stVdecChanCfg.s32DnrTfEnable = 0;
    stVdecChanCfg.s32DnrDispOutEnable = 0;

    /* MV300 COMPRESS PATCH */
/*
    if (pstChan->stOption.s32MaxWidth > 1920 || pstChan->stOption.s32MaxHeight > 1088) {
	EnVcmp = 0;
    } else {
	EnVcmp = 1;
    }
*/

    if (1) {
	/* Config decode compress attr */
	stVdecChanCfg.s32VcmpEn = EnVcmp;

	stVdecChanCfg.s32VcmpWmStartLine = 0;
	stVdecChanCfg.s32VcmpWmEndLine = 0;
    } else {
	if (MT_UNF_VCODEC_TYPE_VP8 == pstCfg->enType) {
	    MT_ERR_VDEC("Unsupport protocol: %d!\n", pstCfg->enType);
	    return MT_FAILURE;
	}

	/* Others do not compress */
	stVdecChanCfg.s32VcmpEn = 0;
    }

#if 1
    stVdecChanCfg.s32Btl1Dt2DEnable = pstChan->stOption.s32Btl1Dt2DEnable;
    stVdecChanCfg.s32BtlDbdrEnable = pstChan->stOption.s32BtlDbdrEnable;
//stVdecChanCfg.s32LowdlyEnable = 1;
//stVdecChanCfg.s32ModuleLowlyEnable = 0;
#endif

    MT_INFO_VDEC("StrmOFThr:%dK/%dK.\n", (stVdecChanCfg.s32ChanStrmOFThr / 1024), (pstChan->u32DmxBufSize / 1024));
    /* Only if pstCfg->orderOutput is 1 we do the judge */
    if (pstCfg->bOrderOutput) {
	if (pstCfg->s32CtrlOptions & MT_UNF_VCODEC_CTRL_OPTION_SIMPLE_DPB || pstChan->bLowdelay) {
	    /* set to 2 means both bOrderoutput and SIMPLE_DPB */
	    stVdecChanCfg.s32DecOrderOutput = 2;
	}
    }
    if (pstChan->bDPBFullCtrl) {
	    stVdecChanCfg.s32DecOrderOutput = 3;
    }

    switch (pstCfg->enUnBlank) {
    case MT_UNF_VCODEC_UNBLANK_USER:
	stVdecChanCfg.s32UnBlank = FW_MODE_UNBLANK_USER;
	break;
    case MT_UNF_VCODEC_UNBLANK_SYNC:
	stVdecChanCfg.s32UnBlank = FW_MODE_UNBLANK_SYNC;
	break;
    case MT_UNF_VCODEC_UNBLANK_STABLE:
	stVdecChanCfg.s32UnBlank = FW_MODE_UNBLANK_STABLE;
	break;
    case MT_UNF_VCODEC_UNBLANK_FAST:
	stVdecChanCfg.s32UnBlank = FW_MODE_UNBLANK_FAST;
	break;
    case MT_UNF_VCODEC_UNBLANK_BUTT:
    default:
	stVdecChanCfg.s32UnBlank = FW_MODE_UNBLANK_FAST;
	break;
    }

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    usrData_open_flag = 1;
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_USRDAT, &usrData_open_flag);
    if (MT_SUCCESS != s32Ret) {
    	MT_ERR_VDEC("VFMW VDEC_CID_START_USRDAT err!\n");
    	return MT_FAILURE;
    }
#endif

    /* Set to VFMW */
    vdec_dump_chcfg(&stVdecChanCfg);
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_CFG_CHAN, &stVdecChanCfg);
    if (VDEC_OK != s32Ret) {
	    MT_ERR_VDEC("VFMW CFG_CHAN err!\n");
	    return MT_FAILURE;
    }

    /* Set to VFMW Frame Rate */
    if (pstCfg->u32ForceFrameRateFlag)
    {
//    	MLOGI("%s: set Ch(%x) force frame rate(%u)\n",__FUNCTION__,pstChan->hChan,pstCfg->u32FrameRateInt);
		s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_FORCE_FRAME_RATE, &(pstCfg->u32FrameRateInt));
	    if (VDEC_OK != s32Ret) {
		    MT_ERR_VDEC("VFMW SET FPS err!\n");
		    return MT_FAILURE;
	    }
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_SeekPTS(mt_handle hHandle, mt_u64 *pu64SeekPts, mt_u32 u32Gap)
{
    mt_s32 s32Ret;
    mt_s32 s32RetStart = VDEC_OK;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    mt_u64 pTmpUserData[3] = { 0 };
    BUFMNG_STATUS_S stBMStatus = { 0 };
    mt_u32 u32RawBufferNum = 0;
    ENTER_FUNCTION;
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }
    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /*TS mode not support SeekPts*/
    if (pstChan->hDmxVidChn != MT_INVALID_HANDLE) {
	MT_ERR_VDEC("ERR: %d  VDEC do not support ts mode Now! \n", hHandle);
	return MT_FAILURE;
    }

    /* stop vpss and vfmw */
    if (pstChan->enCurState != VDEC_CHAN_STATE_STOP) {
	if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss) {
	    /*Stop VPSS*/
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_STOP, MT_NULL);
	    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("%s MT_DRV_VPSS_USER_COMMAND_STOP err!\n", __FUNCTION__);
		return MT_FAILURE;
	    }
	    /*Reset VPSS*/
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_RESET, MT_NULL);
	    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("%s MT_DRV_VPSS_USER_COMMAND_STOP err!\n", __FUNCTION__);
		return MT_FAILURE;
	    }
	}

	/* Stop VFMW */
	if (MT_INVALID_HANDLE != pstChan->hChan) {
	    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_STOP_CHAN, MT_NULL);
	    if (VDEC_OK != s32Ret) {
		MT_ERR_VDEC("Chan %d STOP_CHAN err!\n", pstChan->hChan);
		return MT_FAILURE;
	    }
	}
	/* Save state */
	pstChan->enCurState = VDEC_CHAN_STATE_STOP;
    }

    /*calc rawbuffer num*/
    s32Ret = BUFMNG_GetStatus(pstChan->hStrmBuf, &stBMStatus);
    if (MT_SUCCESS == s32Ret) {
	u32RawBufferNum = stBMStatus.u32PutOK - stBMStatus.u32RlsOK;
    }
    /*call vfmw seek pts*/
    pTmpUserData[0] = *(pu64SeekPts);
    pTmpUserData[1] = u32Gap;
    pTmpUserData[2] = u32RawBufferNum;
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_PTS_TO_SEEK, &pTmpUserData[0]);
    if (s32Ret != VDEC_OK) {
	MT_ERR_VDEC("VDEC_CID_SET_PTS_TO_SEEK err!\n");
	s32Ret = MT_FAILURE;
    }
    *(pu64SeekPts) = pTmpUserData[0];

    /*start VFMW*/
    //s32RetStart = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_CHAN, MT_NULL);
    if (VDEC_OK != s32RetStart) {
	MT_ERR_VDEC("Chan %d VDEC_CID_START_CHAN err!\n", pstChan->hChan);
	return MT_FAILURE;
    }
    /*start VPSS*/
    s32RetStart = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_START, MT_NULL);
    if (MT_SUCCESS != s32RetStart) {
	MT_ERR_VDEC("%s MT_DRV_VPSS_USER_COMMAND_START err!\n", __FUNCTION__);
	return MT_FAILURE;
    }
    /* Save state */
    pstChan->enCurState = VDEC_CHAN_STATE_RUN;

    return s32Ret;
}

static mt_s32 VDEC_GetCap(VDEC_CAP_S *pstCap)
{
    mt_s32 s32Ret = MT_SUCCESS;

    ENTER_FUNCTION;
    if (MT_NULL == pstCap) {
	return MT_FAILURE;
    }

	memset(pstCap, 0, sizeof(VDEC_CAP_S));
    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_CAPABILITY, pstCap);
    MLOGD("%s: Max Ch No(%d), BR(%d), Width(%d), Height(%d), Pix/s(%d)\n",__FUNCTION__,
    	pstCap->s32MaxChanNum,
    	pstCap->s32MaxBitRate,
    	pstCap->s32MaxFrameWidth,
    	pstCap->s32MaxFrameHeight,
    	pstCap->s32MaxPixelPerSec);
	dump_int_array("Supported STD", (int*)pstCap->SupportedStd, 32);

    if (MT_SUCCESS != s32Ret) {
	MLOGE("VFMW GET_CAPABILITY err:%d!\n", s32Ret);
	return MT_FAILURE;
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_GetFbMem(mmz_buffer_s *pstmmz)
{
    ENTER_FUNCTION;
    if (MT_NULL == pstmmz) {
	return MT_FAILURE;
    }
	memset(pstmmz, 0, sizeof(mmz_buffer_s));
	pstmmz->size = g_stStaticVDHMMZ[0].size;
	pstmmz->startVirAddr = g_stStaticVDHMMZ[0].startVirAddr;
	pstmmz->startPhyAddr = g_stStaticVDHMMZ[0].startPhyAddr;
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_CreateStrmBuf(MT_DRV_VDEC_STREAM_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    BUFMNG_INST_CONFIG_S stBufInstCfg;
	mt_handle hVdec = (pstBuf->hVdec) & 0xff;
	if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	    MT_ERR_VDEC("%s bad handle:%d!\n", __func__, hVdec);
	    return MT_ERR_VDEC_INVALID_PARA;
	}

    ENTER_FUNCTION;
    if (MT_NULL == pstBuf) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /* Create buffer manager instance */
    stBufInstCfg.enAllocType = BUFMNG_ALLOC_INNER;
    stBufInstCfg.u32PhyAddr = 0;
    stBufInstCfg.pu8UsrVirAddr = MT_NULL;
    stBufInstCfg.pu8KnlVirAddr = MT_NULL;
    stBufInstCfg.u32Size = pstBuf->u32Size;
    strncpy(stBufInstCfg.aszName, "VDEC_ESBuf", sizeof(stBufInstCfg.aszName)-1);
    stBufInstCfg.aszName[sizeof(stBufInstCfg.aszName)-1] = '\0';
	
    s32Ret = BUFMNG_Create(hVdec, &(pstBuf->hHandle), &stBufInstCfg, pstBuf->pip_en);
    if (s32Ret != MT_SUCCESS) {
	MT_ERR_VDEC("BUFMNG_Create err!\n");
	return MT_FAILURE;
    }

    pstBuf->u32PhyAddr = stBufInstCfg.u32PhyAddr;

	s32Ret = BUFMNG_AppendDescriptor(pstBuf->hHandle, "VDEC_DESCR", CFG_VDEC_VES_DESC_BUFF_SIZE);
    if (s32Ret != MT_SUCCESS) {
    	MLOGE("BUFMNG_AppendDescriptor err!\n");
	}

    return MT_SUCCESS;
}

static mt_s32 VDEC_StrmBuf_SetUserAddr(mt_handle hHandle, ulong u32Addr)
{
    ENTER_FUNCTION;
    return BUFMNG_SetUserAddr(hHandle, u32Addr);
}

static mt_s32 VDEC_DestroyStrmBuf(mt_handle hHandle)
{
    ENTER_FUNCTION;
    /* Destroy instance */
    if (MT_SUCCESS != BUFMNG_Destroy(hHandle)) {
	MT_ERR_VDEC("Destroy buf %ld err!\n", hHandle);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_AttachStrmBuf(mt_handle hHandle, mt_u32 u32BufSize, mt_handle hDmxVidChn, mt_handle hStrmBuf)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
      MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
      return MT_FAILURE;
    }

    //printk("ZZZZ 3333 hHandle = %x,hDmxVidChn= %x, u32BufSize = %x, ves_buffer_channel_id = %x \n",
    //  hHandle, hDmxVidChn, u32BufSize, ves_buffer_channel_id);
//	MLOGI("%s: h 0x%x, hDMX 0x%x, hStram 0x%x, Size 0x%x\n",__FUNCTION__,
  //  	hHandle,
    //	hDmxVidChn,
    //	hStrmBuf,
	//	u32BufSize);

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_ERR_VDEC_INVALID_CHANID;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Must attach buffer before start */
    if (pstChan->enCurState != VDEC_CHAN_STATE_STOP) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d state err:%d!\n", hHandle, pstChan->enCurState);
	return MT_ERR_VDEC_INVALID_STATE;
    }

    if (VDEC_CHAN_STRMBUF_ATTACHED(pstChan)) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d has strm buf:%d!\n", hHandle, pstChan->hStrmBuf);
	return MT_ERR_VDEC_BUFFER_ATTACHED;
    }

    if (MT_INVALID_HANDLE != hDmxVidChn) {
    	pstChan->hDmxVidChn = hDmxVidChn;
    	pstChan->u32DmxBufSize = u32BufSize;
    	pstChan->hStrmBuf = MT_INVALID_HANDLE;
    	pstChan->u32StrmBufSize = 0;
    } else {
    	pstChan->hStrmBuf = hStrmBuf;
    	pstChan->u32StrmBufSize = u32BufSize;
    	pstChan->hDmxVidChn = MT_INVALID_HANDLE;
    	pstChan->u32DmxBufSize = 0;
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_DetachStrmBuf(mt_handle hHandle)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Must stop channel first */
    if (pstChan->enCurState != VDEC_CHAN_STATE_STOP) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d state err:%d!\n", hHandle, pstChan->enCurState);
	return MT_FAILURE;
    }

    /* Clear handles */
    pstChan->hStrmBuf = MT_INVALID_HANDLE;
    pstChan->u32StrmBufSize = 0;
    pstChan->hDmxVidChn = MT_INVALID_HANDLE;
    pstChan->u32DmxBufSize = 0;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetEosFlag(mt_handle hHandle)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;


	
	printk(KERN_ERR "VFMW set VDEC_CID_SET_PVR_EOS !\n");
	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_PVR_EOS, NULL);
	if (VDEC_OK != s32Ret) {
	  VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	  MT_ERR_VDEC("VFMW set VDEC_CID_SET_PVR_EOS err!\n");
	  return MT_FAILURE;
	}

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    MT_INFO_VDEC("Chan %d STREAM_END OK\n", hHandle);
    return MT_SUCCESS;

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return s32Ret;
}
mt_s32 MT_DRV_VDEC_DiscardFrm(mt_handle hHandle, VDEC_DISCARD_FRAME_S *pstParam)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == pstParam) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Call vfmw */
    if (MT_INVALID_HANDLE != pstChan->hChan) {
	//s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_DISCARDPICS_PARAM, pstParam);
	if (VDEC_OK != s32Ret) {
	    MT_ERR_VDEC("Chan %d DISCARDPICS err!\n", pstChan->hChan);
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    return MT_FAILURE;
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    MT_INFO_VDEC("Chan %d DiscardFrm mode %d OK\n", hHandle, pstParam->enMode);
    return MT_SUCCESS;
}

/**
 * ES Play
 * 获取ES Buffer
 */
static mt_s32 VDEC_GetStrmBuf(mt_handle hHandle, VDEC_ES_BUF_S *pstEsBuf, MT_BOOL bUserSpace)
{
    mt_s32 s32Ret;
    BUFMNG_BUF_S stElem;
    mt_handle hVdec = MT_INVALID_HANDLE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    //ENTER_FUNCTION;
    if (MT_NULL == pstEsBuf) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

	s32Ret = VDEC_FindVdecHandleByESBufferHandle(hHandle, &hVdec);
    if (s32Ret != MT_SUCCESS) {
	    return s32Ret;
    }
    hVdec &= 0xff;
	pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
	if (MT_NULL == pstChan) {
	    return MT_FAILURE;
    }

	/* Bug: 007/rzdfc.mp2aac480x272.mkv video hold, for DESC_BUFF over write */
	/* Get descriptor buffer */
	if (pstChan->stCurCfg.u32UseDescInfoFlag)
	{
	    s32Ret = BUFMNG_GetDescWriteBuffer(hHandle, NULL);
	    if (s32Ret != MT_SUCCESS) {
		    return s32Ret;
	    }
	}

    /* Get buffer */
    stElem.u32Size = pstEsBuf->u32BufSize;
    stElem.u32ScrapSize = pstEsBuf->u32ScrapSize;
    s32Ret = BUFMNG_GetWriteBuffer(hHandle, &stElem);
    //printk("YYYYYYYYYY VDEC_GetStrmBuf s32Ret = %x, stElem.u32Size = %x, stElem.u32PhyAddr = %x \n",
    //  s32Ret, stElem.u32Size, stElem.u32PhyAddr);
    if (s32Ret != MT_SUCCESS) {
	    return s32Ret;
    }

    //pstEsBuf->u32BufSize = stElem.u32Size;
    pstEsBuf->u32BufSize = stElem.u32ESBufLeftSize;

    /* If invoked by user space, return user virtual address */
    if (bUserSpace) {
	    pstEsBuf->pu8Addr = stElem.pu8UsrVirAddr;
    }
    /* else, invoked by kernel space, return kernel virtual address */
    else {
	    pstEsBuf->pu8Addr = stElem.pu8KnlVirAddr;
    }
    pstEsBuf->u32PhyAddr = stElem.u32PhyAddr;

    return MT_SUCCESS;
}

/**
 * ES Play
 * 推送ES Buffer
 */
static mt_s32 VDEC_PutStrmBuf(mt_handle hHandle, VDEC_ES_BUF_S *pstEsBuf, MT_BOOL bUserSpace)
{
    mt_s32 s32Ret;
    BUFMNG_BUF_S stElem;
    mt_handle hVdec = MT_INVALID_HANDLE;
    //not used
    //VDEC_ES_BUF_S stEsBuf;

    //ENTER_FUNCTION;
    /* Check parameter */
    if (MT_NULL == pstEsBuf) {
	MT_INFO_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /* If user sapce, put by pu8UsrVirAddr */
    if (bUserSpace) {
	    stElem.pu8UsrVirAddr = pstEsBuf->pu8Addr;
	    stElem.pu8KnlVirAddr = 0;
    }
    /* If kernek sapce, put by pu8KnlVirAddr */
    else {
	    stElem.pu8KnlVirAddr = pstEsBuf->pu8Addr;
	    stElem.pu8UsrVirAddr = 0;
    }
    stElem.u64Pts = pstEsBuf->u64Pts;
    stElem.u32Marker = 0;
    if (!pstEsBuf->bEndOfFrame) {
	    stElem.u32Marker |= BUFMNG_NOT_END_FRAME_BIT;
    }
    if (pstEsBuf->bDiscontinuous) {
	    stElem.u32Marker |= BUFMNG_DISCONTINUOUS_BIT;
    }

    stElem.u32Size = pstEsBuf->u32BufSize;
    stElem.u32PtsValide = pstEsBuf->u32PtsValide;
    stElem.u32FrameFinsh = pstEsBuf->u32FrameFinsh;
    stElem.u32PreFrameFinsh = pstEsBuf->u32PreFrameFinsh;
    stElem.u32ScrapSize = pstEsBuf->u32ScrapSize;
    stElem.u32EosFlag = pstEsBuf->u32EosFlag;

    s32Ret = BUFMNG_PutWriteBuffer(hHandle, &stElem);
    pstEsBuf->u32ScrapSize = stElem.u32ScrapSize;
    pstEsBuf->u32PtsValide = stElem.u32PtsValide;

    if (s32Ret != MT_SUCCESS) {
	MT_ERR_VDEC("Buf %d put err!\n", hHandle);
	return MT_FAILURE;
    }

    /* save raw stream */
    (VOID) VDEC_FindVdecHandleByESBufferHandle(hHandle, &hVdec);
    BUFMNG_SaveRaw((hVdec & 0xff), stElem.pu8KnlVirAddr, stElem.u32Size);

    return MT_SUCCESS;
}

//add by l00225186
/*提供给vo收帧的函数*/
static mt_s32 VDEC_Chan_VOAcqFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame)
{
    mt_s32 s32Ret;
    s32Ret = MT_SUCCESS;
    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hPort) || (MT_NULL == pstVpssFrame)) {
	MT_ERR_VDEC("VDEC_Chan_VOAcqFrame Bad param!\n");
	return MT_FAILURE;
    }
    /*调用vpss的获取帧存函数,要确定vpss释放函数成功返回的是MT_SUCCESS*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortFrame)(hPort, pstVpssFrame);
    return s32Ret;
}

//Note: maybe called in Display ISR!
//      And should not call down/up!
/*提供给vo释放帧的函数*/
/**
 * Display显示完一帧之后,会在中断里调用该接口释放显示帧.
 * 此时即可通知VFMW和VPSS释放解码帧和后处理后的帧.
 */
static mt_s32 VDEC_Chan_VORlsFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame)
{
    mt_s32 s32Ret;
    //FIXME: stack might overflow!
    MT_DRV_VPSS_PORT_CFG_S stVpssPortCfg;
    mt_handle hVdec = MT_INVALID_HANDLE;
    VDEC_CHANNEL_S *pstChan;
    s32Ret = MT_SUCCESS;
    //ENTER_FUNCTION;

    if ((MT_INVALID_HANDLE == hPort) || (MT_NULL == pstVpssFrame)) {
		MT_ERR_VDEC("VDEC_Chan_VORlsFrame Bad param!\n");
		return MT_FAILURE;
    }

    s32Ret = VDEC_FindVdecHandleByPortHandle(hPort, &hVdec);

    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("VDEC_Chan_VORlsFrame call VDEC_FindVdecHandleByPortHandle ERROR!\n");
		return MT_FAILURE;
	}

    {
		mt_handle hVdecHandle;
		//VDEC_CHANNEL_S *pstChan = MT_NULL;

		hVdecHandle = pstVpssFrame->hVdecHandle;
		if (hVdecHandle == MT_INVALID_HANDLE || hVdecHandle >= MT_VDEC_MAX_INSTANCE_NEW)
		{
			MT_ERR_VDEC("VDEC_Chan_VORlsFrame invalid hVdecHandle(0x%x)!\n",hVdecHandle);
			return MT_FAILURE;
		}

//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
		VTRACE("\n[%s](%lu): frm_cnt %u, slot %u\n",__FUNCTION__,0/*isr*/,
				pstVpssFrame->slotInfo.frm_cnt,
				pstVpssFrame->slotInfo.filedInfoTop.slot_idx);
#endif

		pstChan = s_stVdecDrv.astChanEntity[hVdecHandle].pstChan;
		if (pstChan == MT_NULL)
		{
			MT_ERR_VDEC("VDEC_Chan_VORlsFrame pstChan is null!\n");
			return MT_FAILURE;
		}
    	//printk("\n CCCC 0000 slot_idx = %x \n", pstVpssFrame->slotInfo.filedInfoTop.slot_idx);
		if (pstChan->stImageIntf.release_image == MT_NULL)
		{
			//if VDEC already stopped, no need release to FW, for FW will do reset all.
			MT_WARN_VDEC("VDEC_Chan_VORlsFrame release_image is null!\n");
			return MT_FAILURE;
		}

		BUG_ON(sizeof(MT_DIS_FRAME_SLOT_INFO_T) != sizeof(MT_DIS_FRAME_SLOT_INFO_T));

    	pstChan->stImageIntf.release_image(pstChan->stImageIntf.image_provider_inst_id, (mt_void *)(&pstVpssFrame->slotInfo));
    }

    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("VDEC_Chan_VORlsFrame call VDEC_FindVdecHandleByPortHandle ERROR!\n");
		return MT_FAILURE;
    } else {
		pstChan = VDEC_DRV_GetChan(hVdec);
		if (MT_NULL != pstChan) {
		    pstChan->stStatInfo.u32AvplayRlsFrameTry++;
		} else {
		    MT_ERR_VDEC("VDEC_Chan_VORlsFrame call VDEC_DRV_GetChan ERROR!\n");
		    return MT_FAILURE;
		}
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortCfg)(hPort, &stVpssPortCfg);
    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("MT_DRV_VPSS_GetDefaultPortCfg err!\n");
		return MT_FAILURE;
    }
    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == stVpssPortCfg.stBufListCfg.eBufType) {
		s32Ret = BUFMNG_VPSS_RelBuffer((mt_handle)PORTHANDLE_TO_VPSSID(hPort), hPort, pstVpssFrame);
		if (MT_SUCCESS != s32Ret) {
		    MT_ERR_VDEC("BUFMNG_VPSS_RelBuffer err!\n");
		    return MT_FAILURE;
		}
    } else {
		//mt_handle hVdecHandle;
		//VDEC_CHANNEL_S *pstChan = MT_NULL;

		/*调用vpss的释放帧存函数*/
		s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssRelPortFrame)(hPort, pstVpssFrame);
        //hVdecHandle = pstVpssFrame->hVdecHandle;
        //pstChan = s_stVdecDrv.astChanEntity[hVdecHandle].pstChan;
        //pstChan->stImageIntf.release_image(pstChan->stImageIntf.image_provider_inst_id, (mt_void *)(&pstVpssFrame->slotInfo));
    }
    if (MT_SUCCESS == s32Ret) {
	    pstChan->stStatInfo.u32AvplayRlsFrameOK++;
      //printk("WWWW 1111 u32AvplayRlsFrameOK = %d, slot_idx = %d \n", pstChan->stStatInfo.u32AvplayRlsFrameOK, pstVpssFrame->slotInfo.filedInfoTop.slot_idx);
    }
    return s32Ret;
}

#if (CFG_VFMW_ON_AVCPU == 1)
/* acquire freeze frame buffer */
static mt_s32 VDEC_Chan_VOAcqFreezeFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstFzFrame)
{
    mt_s32 s32Ret;
    mt_handle hVdec;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;

	s32Ret = VDEC_FindVdecHandleByPortHandle(hPort, &hVdec);
    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("Port %d not found!\n", hPort);
		return MT_FAILURE;
    }

    hVdec = hVdec & 0xff;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
		MT_ERR_VDEC("%s %d hVdec:%d\n", __func__, __LINE__, hVdec);
		return MT_ERR_VDEC_INVALID_PARA;
    }

    if (pstFzFrame == NULL)
    {
		MT_ERR_VDEC("%s: invalid argument!\n", __func__);
		return MT_ERR_VDEC_INVALID_PARA;
    }
	memset(pstFzFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hVdec]);
    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("Chan %d lock fail!\n", hVdec);
		return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d not init!\n", hVdec);
		return MT_FAILURE;
    }

    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;

	if (pstChan->enCurState == VDEC_CHAN_STATE_INVALID)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d state invalid!\n", hVdec);
		return MT_FAILURE;
	}

	if (pstChan->stVDHMMZBuf.u32StartPhyAddr == MT_NULL)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d VDH MM is null!\n", hVdec);
		return MT_FAILURE;
	}

	if (MT_INVALID_HANDLE == pstChan->hChan)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("%s: VFMW Chan is invalid!\n", __FUNCTION__);
		return MT_FAILURE;
	}

    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_AC_FREEZE_BUFF, (void *)&pstChan->stFreezeFrame);
    if (VDEC_OK != s32Ret) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
    	MT_WARN_VDEC("Chan %d ACQ Freeze Frame buffer failed!\n", pstChan->hChan);
    	return MT_FAILURE;
    }

	//convert to MT_DRV_VIDEO_FRAME_S
	memcpy(&pstFzFrame->slotInfo, &pstChan->stFreezeFrame, sizeof(MT_DIS_FRAME_SLOT_INFO_T));
	pstFzFrame->hVdecHandle = hVdec;
	MLOGI("%s: Vdec %u, Port %u, acquired freeze frame buffer addr: 0x%x success.\n",__FUNCTION__,
		hVdec, hPort,
		pstChan->stFreezeFrame.filedInfoTop.addrLuma);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
    return MT_SUCCESS;
}

//Note: maybe called in Display ISR!
//      And should not call down/up!
/* release freeze frame buffer */
static mt_s32 VDEC_Chan_VORlsFreezeFrame(mt_handle hPort, MT_DRV_VIDEO_FRAME_S *pstFzFrame)
{
    mt_s32 s32Ret;
    mt_handle hVdec;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;

	s32Ret = VDEC_FindVdecHandleByPortHandle(hPort, &hVdec);
    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("Port %d not found!\n", hPort);
		return MT_FAILURE;
    }

    hVdec = hVdec & 0xff;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
		MT_ERR_VDEC("%s %d hVdec:%d\n", __func__, __LINE__, hVdec);
		return MT_ERR_VDEC_INVALID_PARA;
    }

    if (pstFzFrame == NULL)
    {
		MT_ERR_VDEC("%s: invalid argument!\n", __func__);
		return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hVdec]);
    if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("Chan %d lock fail!\n", hVdec);
		return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d not init!\n", hVdec);
		return MT_FAILURE;
    }

    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;

	if (pstChan->enCurState == VDEC_CHAN_STATE_INVALID)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d state invalid!\n", hVdec);
		return MT_FAILURE;
	}

	if (pstChan->stVDHMMZBuf.startPhyAddr == MT_NULL)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("Chan %d VDH MM is null!\n", hVdec);
		return MT_FAILURE;
	}

	if (MT_INVALID_HANDLE == pstChan->hChan)
	{
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
		MT_WARN_VDEC("%s: VFMW Chan is invalid!\n", __FUNCTION__);
		return MT_FAILURE;
	}

    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_RLS_FREEZE_BUFF, (void *)&pstFzFrame->slotInfo);
    if (VDEC_OK != s32Ret) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
    	MT_WARN_VDEC("Chan %d Rls Freeze Frame buffer failed!\n", pstChan->hChan);
    	return MT_FAILURE;
    }

	memset(&pstChan->stFreezeFrame, 0, sizeof(MT_DIS_FRAME_SLOT_INFO_T));

	MLOGI("%s: Vdec %u, Port %u, release freeze frame buffer addr: 0x%x success.\n",__FUNCTION__,
		hVdec, hPort,
		pstFzFrame->slotInfo.filedInfoTop.addrLuma);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
    return MT_SUCCESS;
}
#endif

static mt_s32 VDEC_ConvertWinInfo(mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg, MT_DRV_WIN_PRIV_INFO_S *pstWinInfo)
{
    mt_handle hVdec = MT_INVALID_HANDLE;
    mt_s32 s32Ret = MT_FAILURE;
    mt_s32 i;
    pstVpssPortCfg->s32OutputWidth = pstWinInfo->stOutRect.s32Width;
    pstVpssPortCfg->s32OutputHeight = pstWinInfo->stOutRect.s32Height;
    s32Ret = VDEC_FindVdecHandleByPortHandle(hPort, &hVdec);
    if (MT_SUCCESS != s32Ret) {
	return MT_FAILURE;
    }
    hVdec = hVdec & 0xff;
    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	if (s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[i].hPort == hPort) {
	    break;
	}
    }
    if (i >= VDEC_MAX_PORT_NUM) {
	return MT_FAILURE;
    }
    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stPort[i].bufferType) {
	pstVpssPortCfg->s32OutputWidth = 0;
	pstVpssPortCfg->s32OutputHeight = 0;
    }
    pstVpssPortCfg->u32MaxFrameRate = pstWinInfo->u32MaxRate;
    pstVpssPortCfg->eDstCS = MT_DRV_CS_BT709_YUV_LIMITED;
    pstVpssPortCfg->eFormat = pstWinInfo->ePixFmt;
    pstVpssPortCfg->eAspMode = pstWinInfo->enARCvrs;
    pstVpssPortCfg->stCustmAR = pstWinInfo->stCustmAR;

    pstVpssPortCfg->stDispPixAR = pstWinInfo->stScreenAR;

    pstVpssPortCfg->stScreen = pstWinInfo->stScreen;
    pstVpssPortCfg->b3Dsupport = pstWinInfo->bIn3DMode;
    switch (pstWinInfo->enRotation) {
    case MT_DRV_ROT_ANGLE_0:
	pstVpssPortCfg->enRotation = MT_DRV_VPSS_ROTATION_DISABLE;
	break;
    case MT_DRV_ROT_ANGLE_90:
	pstVpssPortCfg->enRotation = MT_DRV_VPSS_ROTATION_90;
	break;
    case MT_DRV_ROT_ANGLE_180:
	pstVpssPortCfg->enRotation = MT_DRV_VPSS_ROTATION_180;
	break;
    case MT_DRV_ROT_ANGLE_270:
	pstVpssPortCfg->enRotation = MT_DRV_VPSS_ROTATION_270;
	break;
    default:
	MT_ERR_VDEC("Invalid Rotation param %d !\n", pstWinInfo->enRotation);
	pstVpssPortCfg->enRotation = MT_DRV_VPSS_ROTATION_DISABLE;
	break;
    }
    pstVpssPortCfg->bHoriFlip = pstWinInfo->bHoriFlip;
    pstVpssPortCfg->bVertFlip = pstWinInfo->bVertFlip;
    pstVpssPortCfg->bTunnelEnable = pstWinInfo->bTunnelSupport;
    memcpy(&(pstVpssPortCfg->stInRect), &(pstWinInfo->stInRect), sizeof(mt_rect_s));
    pstVpssPortCfg->bUseCropRect = pstWinInfo->bUseCropRect;
    memcpy(&(pstVpssPortCfg->stCropRect), &(pstWinInfo->stCropRect), sizeof(MT_DRV_CROP_RECT_S));
    /*
    pstVpssPortCfg->stBufListCfg.eBufType        = 0;
    pstVpssPortCfg->stBufListCfg.u32BufNumber    = 6;
    pstVpssPortCfg->stBufListCfg.u32BufSize      = 2*(pstWinInfo->stOutRect.s32Width)*(pstWinInfo->stOutRect.s32Height);
    pstVpssPortCfg->stBufListCfg.u32BufStride    = pstVpssPortCfg->s32OutputWidth;
    */
    return MT_SUCCESS;
}
//add by l00225186
static mt_s32 VDEC_Chan_VOChangeWinInfo(mt_handle hPort, MT_DRV_WIN_PRIV_INFO_S *pstWinInfo)
{
    mt_s32 s32Ret;
    MT_DRV_VPSS_PORT_CFG_S stVpssPortCfg;
    MT_DRV_VPSS_CFG_S stVpssCfg;
    mt_handle hVpss;
    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hPort) || (MT_NULL == pstWinInfo)) {
	MT_ERR_VDEC("VDEC_Chan_VORlsFrame Bad param!\n");
	return MT_FAILURE;
    }
    hVpss = PORTHANDLE_TO_VPSSID(hPort);
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortCfg)(hPort, &stVpssPortCfg);

    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);

    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }

    if (pstWinInfo->bUseCropRect) {
	stVpssCfg.stProcCtrl.bUseCropRect = MT_TRUE;
	stVpssCfg.stProcCtrl.stCropRect.u32LeftOffset = pstWinInfo->stCropRect.u32LeftOffset;
	stVpssCfg.stProcCtrl.stCropRect.u32RightOffset = pstWinInfo->stCropRect.u32RightOffset;
	stVpssCfg.stProcCtrl.stCropRect.u32BottomOffset = pstWinInfo->stCropRect.u32BottomOffset;
	stVpssCfg.stProcCtrl.stCropRect.u32TopOffset = pstWinInfo->stCropRect.u32TopOffset;
    } else {
	stVpssCfg.stProcCtrl.bUseCropRect = MT_FALSE;
	stVpssCfg.stProcCtrl.stInRect.s32Height = pstWinInfo->stInRect.s32Height;
	stVpssCfg.stProcCtrl.stInRect.s32Width = pstWinInfo->stInRect.s32Width;
	stVpssCfg.stProcCtrl.stInRect.s32X = pstWinInfo->stInRect.s32X;
	stVpssCfg.stProcCtrl.stInRect.s32Y = pstWinInfo->stInRect.s32Y;
    }

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);

    VDEC_ConvertWinInfo(hPort, &stVpssPortCfg, pstWinInfo);
    /*重新设置PORT属性*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetPortCfg)(hPort, &stVpssPortCfg);
    return s32Ret;
}
//add by l00225185
/*创建vpss*/
static mt_s32 VDEC_Chan_CreateVpss(mt_handle hVdec, mt_handle *phVpss)
{
    mt_s32 s32Ret;
    MT_DRV_VPSS_CFG_S stVpssCfg;
    MT_DRV_VPSS_SOURCE_FUNC_S stRegistSrcFunc;

    ENTER_FUNCTION;

    if ((MT_INVALID_HANDLE == hVdec) || (MT_NULL == phVpss)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_Chan_CreateVpss err hvdec:%lx too large!\n", hVdec);
	return MT_FAILURE;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetDefaultCfg)(&stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_GetDefaultCfg err!\n");
	return MT_FAILURE;
    }

    /*向上返回创建的vpss句柄*/
#if 0	
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssCreateVpss)(&stVpssCfg, phVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_CreateVpss err!\n");
	return MT_FAILURE;
    }
#endif
	*phVpss =  hVdec; 

//    MLOGD("%s: hVdec 0x%x, hVpss 0x%x\n",__FUNCTION__,hVdec,*phVpss);

    s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
	if (s32Ret != 0)
	{
		MLOGE("%s: down_killable failed! return %d.\n",__FUNCTION__,s32Ret);
	}

    /*保存vpss句柄*/
    s_stVdecDrv.astChanEntity[hVdec].stVpssChan.hVpss = *phVpss;
    up(&s_stVdecDrv.stSem);

    s_stVdecDrv.astChanEntity[hVdec].stVpssChan.eFramePackType = MT_UNF_FRAME_PACKING_TYPE_BUTT;
    s_stVdecDrv.astChanEntity[hVdec].stVpssChan.enFrameBuffer = (int)MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE; //MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE;//;
    /*注册vpss回调函数*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssRegistHook)(*phVpss, hVdec, VDEC_VpssEventHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_RegistHook err!\n");
	return MT_FAILURE;
    }
    stRegistSrcFunc.VPSS_GET_SRCIMAGE = VDEC_Chan_VpssRecvFrmBuf;
    stRegistSrcFunc.VPSS_REL_SRCIMAGE = VDEC_Chan_VpssRlsFrmBuf;
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetSourceMode)(*phVpss, VPSS_SOURCE_MODE_VPSSACTIVE, &stRegistSrcFunc);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_SetSourceMode err!\n");
	return MT_FAILURE;
    }
#if 0
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.hVpss,MT_DRV_VPSS_USER_COMMAND_START, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_START err!\n");
        return MT_FAILURE;
    }
#endif

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
			if(0 == g_stLecvcDataMMZ[hVdec].startPhyAddr)
			{
				//#define LCEVCDATA_SLOT_NUM   32
				//#define LCEVCDATA_SIZE   20*1024
				//4k for share struct.
				mt_drv_mmz_alloc_and_map("LECVC", MMZ_OTHERS, 644*1024, 4096, &g_stLecvcDataMMZ[hVdec]);
				memset(g_stLecvcDataMMZ[hVdec].startVirAddr, 0, g_stLecvcDataMMZ[hVdec].size);
			}
#endif

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 VDEC_Chan_DestroyVpss(mt_handle hVdec)
{
    mt_s32 s32Ret = MT_SUCCESS;
    mt_handle hVpss;

    ENTER_FUNCTION;

    //s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
    hVpss = s_stVdecDrv.astChanEntity[hVdec].stVpssChan.hVpss;
 //   MLOGD("%s: hVdec 0x%x, hVpss 0x%x\n",__FUNCTION__,hVdec,hVpss);
    if (MT_INVALID_HANDLE != hVpss) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hVdec].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_STOP, NULL);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_STOP err!\n");
	    return MT_FAILURE;
	}

	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssDestroyVpss)(hVpss);
	if (MT_SUCCESS != s32Ret) {
	    //up(&s_stVdecDrv.stSem);
	    return s32Ret;
	}
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.hVpss = MT_INVALID_HANDLE;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stControlInfo.u32BackwardOptimizeFlag = MT_FALSE;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.stControlInfo.u32DispOptimizeFlag = MT_FALSE;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.s32GetFirstIFrameFlag = MT_FALSE;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.s32GetFirstVpssFrameFlag = MT_FALSE;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.eLastFrameFormat = VDEC_FRAME_BUTT;
	//s_stVdecDrv.astChanEntity[hVdec].stVpssChan.s32LastFrameGopNum = -1;
	s_stVdecDrv.astChanEntity[hVdec].stVpssChan.s32ImageDistance = 0;
    }
    //up(&s_stVdecDrv.stSem);
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
		if (g_stLecvcDataMMZ[hVdec].startPhyAddr != 0)
		{
			gPLcevc_pipe[hVdec] = NULL;
			mt_drv_mmz_unmap_and_release(&g_stLecvcDataMMZ[hVdec]);
			g_stLecvcDataMMZ[hVdec].startPhyAddr = 0;
		}
#endif

	LEAVE_FUNCTION;
    return s32Ret;
}

static mt_s32 VDEC_Chan_CreatePort(mt_handle hVpss, mt_handle *phPort, VDEC_PORT_ABILITY_E ePortAbility)
{
    mt_s32 s32Ret;
    MT_DRV_VPSS_PORT_CFG_S stVpssPortCfg;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;

    if ((MT_INVALID_HANDLE == hVpss) || (MT_NULL == phPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    /*get defualt port cfg*/
    /*CNcomment:创建port操作，调用vpss创建port函数，并在vdec端做记录*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetDefaultPortCfg)(&stVpssPortCfg);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_ERR_VDEC("MT_DRV_VPSS_GetDefaultPortCfg err!\n");
	return MT_FAILURE;
    }
    if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == pstVpssChan->enFrameBuffer || MT_DRV_VDEC_BUF_VDEC_ALLOC_MANAGE == pstVpssChan->enFrameBuffer) {
	stVpssPortCfg.stBufListCfg.eBufType = MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE;
    }
    /*create port*/
    /*CNcomment:创建port*/
    else if (MT_DRV_VDEC_BUF_VPSS_ALLOC_MANAGE == pstVpssChan->enFrameBuffer) {
	stVpssPortCfg.stBufListCfg.eBufType = MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE;
    }

	MLOGD("%s: PortAbility=%d\n",__FUNCTION__,ePortAbility);
    switch (ePortAbility) {
    case VDEC_PORT_HD:
	stVpssPortCfg.s32OutputHeight = 1080;
	stVpssPortCfg.s32OutputWidth = 1920;
	break;
    case VDEC_PORT_SD:
	stVpssPortCfg.s32OutputHeight = 576;
	stVpssPortCfg.s32OutputWidth = 720;
	break;
    case VDEC_PORT_STR:
	stVpssPortCfg.s32OutputHeight = 720;
	stVpssPortCfg.s32OutputWidth = 1280;
	break;
    default:
	break;
    }
    stVpssPortCfg.stBufListCfg.u32BufSize = stVpssPortCfg.s32OutputHeight * stVpssPortCfg.s32OutputWidth * 3 / 2;
    stVpssPortCfg.stBufListCfg.u32BufStride = stVpssPortCfg.s32OutputWidth;
    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == stVpssPortCfg.stBufListCfg.eBufType) {
	stVpssPortCfg.s32OutputWidth = 0; // 宽高设为0表示根据输入自适应配置
	stVpssPortCfg.s32OutputHeight = 0;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssCreatePort)(hVpss, &stVpssPortCfg, phPort);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_ERR_VDEC("MT_DRV_VPSS_CreatePort err!\n");
	return MT_FAILURE;
    }

    /*save port handle*/
    /*CNcomment:vdec端记录port句柄*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (MT_INVALID_HANDLE == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	MT_ERR_VDEC("Too many ports!\n");
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    } else {
	pstVpssChan->stPort[j].hPort = *phPort;
	pstVpssChan->stPort[j].bEnable = MT_TRUE;
	pstVpssChan->stPort[j].enPortType = VDEC_PORT_TYPE_BUTT;
	pstVpssChan->stPort[j].bufferType = stVpssPortCfg.stBufListCfg.eBufType;
//	MLOGD("%s: hVpss 0x%x, port %d, hPort 0x%x\n",__FUNCTION__,hVpss,j,*phPort);

	/*init the vpss buffer*/
	/*CNcomment:如果由vdec来管理vpss的帧存则进行帧存的初始化*/
	if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == stVpssPortCfg.stBufListCfg.eBufType) {
	    pstVpssChan->stPort[j].stBufVpssInst.enFrameBuffer = pstVpssChan->enFrameBuffer;
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_DestroyPort(mt_handle hVpss, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
//    MLOGD("%s: hVpss 0x%x, hPort 0x%x\n",__FUNCTION__,hVpss,hPort);

    if ((MT_INVALID_HANDLE == hVpss) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    /*find port*/
    /*CNcomment:查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	pstVpssChan->stPort[j].hPort = MT_INVALID_HANDLE;
	pstVpssChan->stPort[j].bEnable = MT_FALSE;
	pstVpssChan->stPort[j].enPortType = VDEC_PORT_TYPE_BUTT;
	pstVpssChan->stPort[j].s32PortTmpListPos = 0;

	/*call vpss funtion to destory port*/
	/*CNcomment:调用vpss接口销毁port*/
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssDestroyPort)(hPort);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    MT_WARN_VDEC("MT_DRV_VPSS_DestroyPort err!\n");
	    return MT_FAILURE;
	}

	if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == pstVpssChan->stPort[j].bufferType) {
	    BUFMNG_VPSS_DeInit(&pstVpssChan->stPort[j].stBufVpssInst);
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_EnablePort(mt_handle hVpss, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
//	MLOGD("%s: hVpss 0x%x, hPort 0x%x\n",__FUNCTION__,hVpss,hPort);

    if ((MT_INVALID_HANDLE == hVpss) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    /*enable port*/
    /*CNcomment:查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	pstVpssChan->stPort[j].bEnable = MT_TRUE;
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssEnablePort)(hPort, MT_TRUE);
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_DisablePort(mt_handle hVpss, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
//    MLOGD("%s: hVpss 0x%x, hPort 0x%x\n",__FUNCTION__,hVpss,hPort);

    if ((MT_INVALID_HANDLE == hVpss) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    /*disenable port,find port*/
    /*CNcomment:查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	pstVpssChan->stPort[j].bEnable = MT_FALSE;
	pstVpssChan->stPort[j].s32PortLastFrameGopNum = -1;
	pstVpssChan->stPort[j].u32LastFrameIndex = -1;
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssEnablePort)(hPort, MT_FALSE);
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

#ifdef CONFIG_MT_HAS_HW_VPASS
/* receive/release VPSS frames to clean all of them while state is running */
static mt_s32 VDEC_Chan_CleanVpssFrame(mt_handle hHandle, mt_handle hVpss, mt_s32 try_times)
{
    mt_s32 i = 0;
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    MT_DRV_VIDEO_FRAME_PACKAGE_S *pstVpssFramePack = NULL;

    ENTER_FUNCTION;

	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
	if (pstChan != NULL && pstChan->enCurState == VDEC_CHAN_STATE_RUN)
	{
		pstVpssFramePack = &pstChan->stVpssFramePack;
		do
		{
			s32Ret = VDEC_Chan_RecvVpssFrmBuf(hVpss, pstVpssFramePack);
			if (s32Ret == MT_SUCCESS)
			{
				for (i=0; i<pstVpssFramePack->u32FrmNum; i++)
				{
					//FIXME: maybe conflict with Display's ISR calling VDEC_Chan_VORlsFrame!
					s32Ret |= VDEC_Chan_VORlsFrame(pstVpssFramePack->stFrame[i].hport,
													&pstVpssFramePack->stFrame[i].stFrameVideo);
					if (s32Ret == MT_SUCCESS)
					{
						MLOGI("%s: release slot_idx %u success.\n",__FUNCTION__,
								pstVpssFramePack->stFrame[i].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
					}
					else
					{
						MLOGE("%s: release slot_idx %u failed!\n",__FUNCTION__,
								pstVpssFramePack->stFrame[i].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
						break;
					}
				}
			}

			//msleep(1);

		} while (--try_times > 0);
	}

	return s32Ret;
}

/* receive/release VPSS frames to clean all of them while state is running */
static mt_s32 VDEC_Chan_CleanAllVpssFrame(mt_handle hHandle, mt_handle hVpss)
{
    mt_s32 s32Ret = MT_FAILURE;
	MT_BOOL bAllDone = MT_TRUE;

    ENTER_FUNCTION;

	do
	{
		if (s_stVdecDrv.astChanEntity[hHandle].pstChan != MT_NULL
			&& s_stVdecDrv.astChanEntity[hHandle].pstChan->enCurState == VDEC_CHAN_STATE_RUN)
		{
			VDEC_Chan_CleanVpssFrame(hHandle, hVpss, 1);
		}
		else
		{
			break;
		}

		msleep(1);

		if (s_stVdecDrv.pVpssFunc != NULL && s_stVdecDrv.pVpssFunc->pfnVpssSendCommand != NULL)
		{
			bAllDone = MT_TRUE;
			s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(hVpss, MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE, (void*)&bAllDone);
		}
		else
		{
			break;
		}

	//till all released!
	} while (s32Ret == MT_SUCCESS && !bAllDone);

    LEAVE_FUNCTION;
	return s32Ret;
}
#endif

static mt_s32 VDEC_Chan_ResetVpss(mt_handle hVpss)
{
    mt_s32 i = 0;
    mt_s32 s32Ret;
    mt_u32 u32Index;
    mt_handle hHandle = MT_INVALID_HANDLE;
    VDEC_PORT_FRAME_LIST_LOCK_S *pListLock = MT_NULL;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    VDEC_PORT_FRAME_LIST_NODE_S *pstListNode = MT_NULL;

    ENTER_FUNCTION;
    hVpss = hVpss & 0xff;

    if (MT_INVALID_HANDLE == hVpss) {
    	MT_ERR_VDEC("VDEC_Chan_ResetVpss Bad param!\n");
    	return MT_FAILURE;
    }

    s32Ret = VDEC_FindVdecHandleByVpssHandle(hVpss, &hHandle);
    if (MT_SUCCESS != s32Ret) {
    	MT_ERR_VDEC("VDEC_FindVdecHandleByVpssHandle err!\n");
    	return MT_FAILURE;
    }

//	MLOGD("%s: hHandle 0x%x, hVpss 0x%x\n",__FUNCTION__,hHandle,hVpss);

    /*Reset pvr tmplist and portlist data*/
    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
    	pListLock = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].stPortList.stPortFrameListLock);
    	pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);

    	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[i].hPort) && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
    	    VDEC_SpinLockIRQ(pListLock);

			MLOGD("%s: s32PortTmpListPos = %x \n", __FUNCTION__,pstVpssChan->stPort[i].s32PortTmpListPos);

	    	if (0 < pstVpssChan->stPort[i].s32PortTmpListPos) {
	    		for (u32Index = 0; u32Index < pstVpssChan->stPort[i].s32PortTmpListPos; u32Index++) {
	    		    s32Ret = VDEC_Chan_VORlsFrame(pstVpssChan->stPort[i].hPort, &(pstVpssChan->stPort[i].astPortTmpList[u32Index].stPortOutFrame));
		            MT_INFO_VDEC("WWWW 8888 VDEC_Chan_ResetVpss\n");
	    		    if (s32Ret != MT_SUCCESS) {
		    			MT_ERR_VDEC("VDEC pvr mode rls vpss port %d frame error.\n", pstVpssChan->stPort[i].hPort);
		    		    }
		    		}
		      }

			pstVpssChan->stPort[i].s32PortTmpListPos = 0;

			while ((&pstVpssChan->stPort[i].stPortList.stVdecPortFrameList) != (pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next)) {
				pstListNode = list_entry(pstVpssChan->stPort[i].stPortList.stVdecPortFrameList.next, VDEC_PORT_FRAME_LIST_NODE_S, node);
				s32Ret = VDEC_Chan_VORlsFrame(pstVpssChan->stPort[i].hPort, &(pstListNode->stPortOutFrame));
				if (s32Ret != MT_SUCCESS) {
				    MT_ERR_VDEC("VDEC pvr mode rls vpss port %d frame error.\n", pstVpssChan->stPort[i].hPort);
				}
				MT_INFO_VDEC("YYYY 1111 VDEC_Chan_ResetVpss \n");
				list_del_init(&(pstListNode->node));
				MT_KFREE_BUFMNG(pstListNode);
			}
			VDEC_SpinUnLockIRQ(pListLock);
    	}
    }

#ifdef CONFIG_MT_HAS_HW_VPASS
	//Patch:
	//when flush, VPSS may store some frames processed or not processed,
	//and can not do release internally(can only reset internally),
	//so here to receive/release them all.
	s32Ret = VDEC_Chan_CleanAllVpssFrame(hHandle, hVpss);
#endif

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(hVpss, MT_DRV_VPSS_USER_COMMAND_RESET, MT_NULL);

	LEAVE_FUNCTION;
    return s32Ret;
}

static mt_s32 VDEC_Chan_SetPortType(mt_handle hVpss, mt_handle hPort, VDEC_PORT_TYPE_E enPortType)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    MT_DRV_VPSS_PORT_CFG_S stVpssPortCfg;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (MT_INVALID_HANDLE == hPort)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    /*set main port,find port*/
    /*CNcomment:查找port是否存在*/
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }
    if (j >= VDEC_MAX_PORT_NUM) {
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	return MT_FAILURE;
    } else {
	pstVpssChan->stPort[j].enPortType = enPortType;
//	MLOGD("%s: hVpss 0x%x, hPort 0x%x, Port Type %d\n",__FUNCTION__,hVpss,hPort,enPortType);
	/*set MaxFrameRate 30pfs when port is virtual port*/
	if (VDEC_PORT_TYPE_VIRTUAL == enPortType) {
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortCfg)(hPort, &stVpssPortCfg);
	    if (MT_SUCCESS != s32Ret) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		MT_ERR_VDEC("pfnVpssGetPortCfg err!\n");
		return MT_FAILURE;
	    }

	    stVpssPortCfg.u32MaxFrameRate = 30;
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetPortCfg)(hPort, &stVpssPortCfg);
	    if (MT_SUCCESS != s32Ret) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		MT_ERR_VDEC("pfnVpssSetPortCfg err!\n");
		return MT_FAILURE;
	    }
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_SetVpssAttr(mt_handle hVpss, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort) {
	MT_ERR_VDEC("hPort is invalid!\n");
	return MT_FAILURE;
    }

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }

    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }

    if (j >= VDEC_MAX_PORT_NUM) {
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    } else {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetPortCfg)(hPort, pstPortCfg);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("pfnVpssSetPortCfg ERR, Ret=%#x\n", s32Ret);
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    return s32Ret;
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_GetVpssAttr(mt_handle hVpss, mt_handle hPort, MT_DRV_VPSS_PORT_CFG_S *pstPortCfg)
{
    mt_s32 s32Ret;
    mt_s32 i, j;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hPort) {
	MT_ERR_VDEC("hPort is invalid!\n");
	return MT_FAILURE;
    }

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	return MT_FAILURE;
    }

    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if (hPort == pstVpssChan->stPort[j].hPort) {
	    break;
	}
    }

    if (j >= VDEC_MAX_PORT_NUM) {
	MT_WARN_VDEC("Port %d not exist!\n", hPort);
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    } else {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortCfg)(hPort, pstPortCfg);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("pfnVpssGetPortCfg ERR, Ret=%#x\n", s32Ret);
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    return MT_FAILURE;
	}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_SetExtBuffer(mt_handle hVpss, VDEC_BUFFER_ATTR_S *pstBufferAttr)
{
    mt_s32 i;
    BUFMNG_VPSS_NODE_S *pstBufNode;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    BUFMNG_VPSS_INST_S *pstBufVpssInst;
    BUFMNG_VPSS_NODE_S *pstTarget;
    struct list_head *pos, *n;
    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (MT_NULL == pstBufferAttr)) {
	MT_ERR_VDEC("Bad param eFramePackType!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    pstBufVpssInst = &(pstVpssChan->stPort[0].stBufVpssInst);
    if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[i].stVpssChan.enFrameBuffer) {
	if (0 != pstBufVpssInst->u32BufNum && pstBufferAttr->u32BufSize != pstBufVpssInst->u32BufSize) {
	    MT_ERR_VDEC("^^^^^^^^^^^^^^^^^^^^^^^^the buffer size if not same! the size should :%d but you put :%d  u32BufNum:%d\n", pstBufVpssInst->u32BufSize, pstBufferAttr->u32BufSize, pstBufVpssInst->u32BufNum);
	    return MT_FAILURE;
	}
	if (0 == pstBufVpssInst->u32BufNum) {
	    pstBufVpssInst->u32BufSize = pstBufferAttr->u32BufSize;
	}
	for (i = 0; i < pstBufferAttr->u32BufNum; i++) {
	    pstBufNode = MT_VMALLOC_BUFMNG(sizeof(BUFMNG_VPSS_NODE_S));
	    if (MT_NULL == pstBufNode) {
		list_for_each_safe(pos, n, &(pstBufVpssInst->stVpssBufUnAvailableList))
		{
		    pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
		    list_del_init(pos);
		    MT_VFREE_BUFMNG(pstTarget);
		}
		MT_ERR_VDEC("BUFMNG_VPSS_Init No memory.\n");
		return MT_ERR_BM_NO_MEMORY;
	    }
	    pstBufNode->stMMZBuf.startPhyAddr = pstBufferAttr->u32PhyAddr[i];
	    pstBufNode->stMMZBuf.startVirAddr = (void*)pstBufferAttr->u32UsrVirAddr[i];
	    pstBufNode->stMMZBuf.size = pstBufferAttr->u32BufSize;
	    pstBufNode->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_VDEC_EMPTY;
	    BUFMNG_SpinLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	    pstBufNode->bBufUsed = MT_FALSE;
	    pstBufVpssInst->u32BufNum++;
	    list_add_tail(&(pstBufNode->node), &(pstBufVpssInst->stVpssBufAvailableList));
	    BUFMNG_SpinUnLockIRQ(&pstBufVpssInst->stUnAvailableListLock);
	}
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_SetFrameBufferMode(mt_handle hVpss, VDEC_FRAMEBUFFER_MODE_E enFrameBufferMode)
{
    mt_s32 i;
    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (VDEC_BUF_TYPE_BUTT == enFrameBufferMode)) {
	MT_ERR_VDEC("Bad param eFramePackType!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }
    if (VDEC_BUF_VPSS_ALLOC_MANAGE == enFrameBufferMode) {
	s_stVdecDrv.astChanEntity[i].stVpssChan.enFrameBuffer = MT_DRV_VDEC_BUF_VPSS_ALLOC_MANAGE;
    }
    if (VDEC_BUF_USER_ALLOC_MANAGE == enFrameBufferMode) {
	s_stVdecDrv.astChanEntity[i].stVpssChan.enFrameBuffer = MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[0].bufferType = MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE;
	BUFMNG_VPSS_Init(&(s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[0].stBufVpssInst));
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_CheckAndDelBuffer(mt_handle hVpss, VDEC_BUFFER_INFO_S stBufInfo)
{
    mt_s32 i;
    struct list_head *pos, *n;
    BUFMNG_VPSS_NODE_S *pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    BUFMNG_VPSS_INST_S *pstBufVpssInst;
    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (MT_NULL == stBufInfo.penBufState)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[0].stBufVpssInst.stUnAvailableListLock);
    pstBufVpssInst = &(pstVpssChan->stPort[0].stBufVpssInst);

    list_for_each_safe(pos, n, &(pstVpssChan->stPort[0].stBufVpssInst.stVpssBufAvailableList))
    {
	pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	if (stBufInfo.u32PhyAddr == pstTarget->stMMZBuf.startPhyAddr) {
	    *(stBufInfo.penBufState) = VDEC_BUF_STATE_EMPTY;
	    list_del_init(pos);
	    pstBufVpssInst->u32BufNum--;
	    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[0].stBufVpssInst.stUnAvailableListLock);
	    return MT_SUCCESS;
	}
    }
    list_for_each_safe(pos, n, &(pstVpssChan->stPort[0].stBufVpssInst.stVpssBufUnAvailableList))
    {
	pstTarget = list_entry(pos, BUFMNG_VPSS_NODE_S, node);
	if (stBufInfo.u32PhyAddr == pstTarget->stMMZBuf.startPhyAddr) {
	    if (pstTarget->enFrameBufferState == MT_DRV_VDEC_BUF_STATE_IN_VPSS) {
		*(stBufInfo.penBufState) = VDEC_BUF_STATE_IN_USE;
		BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[0].stBufVpssInst.stUnAvailableListLock);
		return MT_FAILURE;
	    } else {
		*(stBufInfo.penBufState) = VDEC_BUF_STATE_FULL;
		if (pstVpssChan->stPort[0].stBufVpssInst.pstUnAvailableListPos == pos) {
		    pstVpssChan->stPort[0].stBufVpssInst.pstUnAvailableListPos = pos->prev;
		}
		if (pstTarget->enFrameBufferState == MT_DRV_VDEC_BUF_STATE_IN_VDEC_FULL) {
		    pstBufVpssInst->u32AvaiableFrameCnt--;
		}
		list_del_init(pos);
		pstBufVpssInst->u32BufNum--;
		BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[0].stBufVpssInst.stUnAvailableListLock);
		return MT_SUCCESS;
	    }
	}
    }
    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[0].stBufVpssInst.stUnAvailableListLock);
    return MT_FAILURE;
}

static mt_s32 VDEC_Chan_SetExtBufferState(mt_handle hVpss, VDEC_EXTBUFFER_STATE_E enExtBufferState)
{
    mt_s32 i;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    BUFMNG_VPSS_INST_S *pstBufVpssInst;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (enExtBufferState >= VDEC_EXTBUFFER_STATE_BUTT)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    pstBufVpssInst = &(pstVpssChan->stPort[0].stBufVpssInst);
    pstBufVpssInst->enExtBufferState = enExtBufferState;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_SetResolution(mt_handle hVpss, VDEC_RESOLUTION_ATTR_S stResolution)
{
    mt_s32 i;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVpss) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[i].pstChan;
    pstChan->stLastFrm.stDispRect.s32Height = stResolution.s32Height;
    pstChan->stLastFrm.stDispRect.s32Width = stResolution.s32Width;
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_SetFrmPackingType(mt_handle hVpss, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E eFramePackType)
{
    mt_s32 s32Ret;
    mt_s32 i;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (MT_UNF_FRAME_PACKING_TYPE_BUTT == eFramePackType)) {
	MT_ERR_VDEC("Bad param eFramePackType!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    s_stVdecDrv.astChanEntity[i].stVpssChan.eFramePackType = eFramePackType;
    if (MT_NULL != s_stVdecDrv.astChanEntity[i].pstChan) {
	s_stVdecDrv.astChanEntity[i].pstChan->eFramePackType = eFramePackType;
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}
mt_s32 MT_DRV_VDEC_SetFrmPackingType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E eFramePackType)
{
    mt_s32 s32Ret;
    mt_handle hVpss = MT_INVALID_HANDLE;
    ENTER_FUNCTION;
    hVdec = hVdec & 0xff;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle in function:%s %d\n", __func__, __LINE__);
	return MT_FAILURE;
    }
    s32Ret = VDEC_Chan_SetFrmPackingType(hVpss, eFramePackType);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_Chan_SetFrmPackingType in function:%s %d\n", __func__, __LINE__);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_GetFrmPackingType(mt_handle hVpss, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *penFramePackType)
{
    mt_s32 s32Ret;
    mt_s32 i;

    ENTER_FUNCTION;
    if ((MT_INVALID_HANDLE == hVpss) || (MT_NULL == penFramePackType)) {
	MT_ERR_VDEC("Bad param penFramePackType!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    *penFramePackType = s_stVdecDrv.astChanEntity[i].stVpssChan.eFramePackType;

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetFrmPackingType(mt_handle hVdec, MT_UNF_VIDEO_FRAME_PACKING_TYPE_E *penFramePackType)
{
    mt_s32 s32Ret;
    mt_handle hVpss = MT_INVALID_HANDLE;
    ENTER_FUNCTION;
    hVdec = hVdec & 0xff;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle in function:%s %d\n", __func__, __LINE__);
	return MT_FAILURE;
    }
    s32Ret = VDEC_Chan_GetFrmPackingType(hVpss, penFramePackType);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_Chan_SetFrmPackingType in function:%s %d\n", __func__, __LINE__);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static MT_UNF_VIDEO_FRAME_PACKING_TYPE_E VDEC_ConverFrameType(MT_DRV_FRAME_TYPE_E eFrmType)
{
    switch (eFrmType) {
    case MT_DRV_FT_NOT_STEREO:
	return MT_UNF_FRAME_PACKING_TYPE_NONE;
    case MT_DRV_FT_SBS:
	return MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE;
    case MT_DRV_FT_TAB:
	return MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM;
    case MT_DRV_FT_FPK:
	return MT_UNF_FRAME_PACKING_TYPE_FRAME_PACKING;
    case MT_DRV_FT_TILE:
	return MT_UNF_FRAME_PACKING_TYPE_3D_TILE;
    default:
	return MT_UNF_FRAME_PACKING_TYPE_BUTT;
    }
}
mt_s32 MT_DRV_VDEC_GetVideoFrameInfo(mt_handle hVdec, MT_UNF_AVPLAY_VIDEO_FRAME_INFO_S *pstVideoFrameInfo)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    hVdec = hVdec & 0xff;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("Chan %d not init!\n", hVdec);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    pstVideoFrameInfo->u32Width = pstChan->stLastFrm.u32Width;
    pstVideoFrameInfo->u32Height = pstChan->stLastFrm.u32Height;
    pstVideoFrameInfo->u32AspectWidth = pstChan->stLastFrm.u32AspectWidth;
    pstVideoFrameInfo->u32AspectHeight = pstChan->stLastFrm.u32AspectHeight;
    //pstVideoFrameInfo->u32fpsInteger = pstChan->stLastVpssFrm.u32FrameRate / 1000;
    pstVideoFrameInfo->u32fpsInteger = pstChan->stLastFrm.u32FrameRate / 1000;
    pstVideoFrameInfo->u32fpsDecimal = 0;
    //pstVideoFrameInfo->bProgressive = pstChan->stLastVpssFrm.bProgressive;
    //pstVideoFrameInfo->enFramePackingType = VDEC_ConverFrameType(pstChan->stLastVpssFrm.eFrmType);
    pstVideoFrameInfo->bProgressive = pstChan->stLastFrm.bProgressive;
    pstVideoFrameInfo->enFramePackingType = VDEC_ConverFrameType(pstChan->stLastFrm.eFrmType);
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_GetPortParam(mt_handle hVpss, mt_handle hPort, VDEC_PORT_PARAM_S *pstPortParam)
{
    mt_s32 s32Ret;
    mt_u32 i;

    ENTER_FUNCTION;
//    MLOGD("%s: hVpss 0x%x, hPort 0x%x\n",__FUNCTION__,hVpss,hPort);

    if ((MT_INVALID_HANDLE == hVpss) || (MT_INVALID_HANDLE == hPort) || (MT_NULL == pstPortParam)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }

    /*主要是给vo提供获取/释放帧存的函数和wininfo改变的处理函数,获取帧存的函数只是为了和释放配对*/
    pstPortParam->pfVOAcqFrame = VDEC_Chan_VOAcqFrame;
    pstPortParam->pfVORlsFrame = VDEC_Chan_VORlsFrame;
    pstPortParam->pfVOSendWinInfo = VDEC_Chan_VOChangeWinInfo;

	//Extend Freeze interfaces
#if (CFG_VFMW_ON_AVCPU == 1)
    pstPortParam->pfAcqFreezeFrame = VDEC_Chan_VOAcqFreezeFrame;
    pstPortParam->pfRlsFreezeFrame = VDEC_Chan_VORlsFreezeFrame;
#else
    pstPortParam->pfAcqFreezeFrame = NULL;
    pstPortParam->pfRlsFreezeFrame = NULL;
#endif

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
    LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_GetPortState(mt_handle hHandle, MT_BOOL *bAllPortComplete)
{
    mt_s32 s32Ret;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    /* check input parameters */
    if ((MT_INVALID_HANDLE == hHandle) || (MT_NULL == bAllPortComplete)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);

    /*get vpss status*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(pstVpssChan->hVpss, MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE, bAllPortComplete);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Vpss:%d MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE error!\n", pstVpssChan->hVpss);
	return MT_FAILURE;
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_SendEos(mt_handle hVdec)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    hVdec = hVdec & 0xff;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("%s %d hVdec:%d\n", __func__, __LINE__, hVdec);
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hVdec]);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Chan %d lock fail!\n", hVdec);
	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
	MT_WARN_VDEC("Chan %d not init!\n", hVdec);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    ((MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstChan->stLastFrm.u32Priv))->u32Reserve))->u8EndFrame = 1;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_GetFrmStatusInfo(mt_handle hVdec, mt_handle hPort, VDEC_FRMSTATUSINFO_S *pstVdecFrmStatusInfo)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    MT_DRV_VPSS_PORT_BUFLIST_STATE_S stVpssBufListState;
    //VDEC_CHAN_FRMSTATUSINFO_S stVdecChanFrmStatusInfo;
    VDEC_CHAN_STATE_S stChanState;
    //ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstVdecFrmStatusInfo) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }
    memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hVdec]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hVdec);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
	MT_ERR_VDEC("Chan %d not init!\n", hVdec);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;

    /*get vfmw frame status*/
    //TODO: add function in vfmw VDEC_CID_GET_CHAN_STATE
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &stChanState);
    if (VDEC_OK != s32Ret) {
	MT_FATAL_VDEC("Chan %d GET_CHAN_STATE err\n", pstChan->hChan);
    } else {
	pstVdecFrmStatusInfo->u32DecodedFrmNum = stChanState.decoded_1d_frame_num + stChanState.wait_disp_frame_num;
	pstVdecFrmStatusInfo->u32StrmSize = stChanState.buffered_stream_size; //u32StrmSize;
    }

    /*get vpss port frame status*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortBufListState)(hPort, &stVpssBufListState);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);
	MT_ERR_VDEC("Get Port %d status error!\n", hPort);

	return MT_FAILURE;
    }

    /*port used_buffer number*/
    pstVdecFrmStatusInfo->u32OutBufFrmNum = stVpssBufListState.u32FulBufNumber;

    /*copy from VDEC_Chan_GetStatusInfo*/
    pstVdecFrmStatusInfo->u32StrmInBps = pstChan->stStatInfo.u32AvrgVdecInBps;

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hVdec]);

    return s32Ret;
}

//not used
#if 0
static mt_s32 VDEC_Chan_AllPortHaveDate(mt_handle hVpss)
{
    mt_s32 i, j = 0;
    BUFMNG_VPSS_NODE_S *pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    //ENTER_FUNCTION;
    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);

    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[j].hPort) && (MT_TRUE == pstVpssChan->stPort[j].bEnable)) {
	    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == pstVpssChan->stPort[j].bufferType) {
		BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		/*首先判断是不是所有的有效port队列上都有数据，只要有一个没有则退出*/
		if (pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next != &pstVpssChan->stPort[j].stBufVpssInst.stVpssBufUnAvailableList && (pstVpssChan->stPort[j].stBufVpssInst.u32AvaiableFrameCnt > 0)) {
		    pstTarget = list_entry(pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next, BUFMNG_VPSS_NODE_S, node);
		    if ((NULL == pstTarget) || (MT_DRV_VDEC_BUF_STATE_IN_USER == pstTarget->enFrameBufferState)) {
			BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
			return MT_FAILURE;
		    }
		    if (MT_TRUE == pstTarget->bBufUsed) {
			BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
			continue;
		    } else {
			if (VDEC_PORT_TYPE_VIRTUAL != pstVpssChan->stPort[j].enPortType) {
			    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
			    return MT_FAILURE;
			} else {
			    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
			    continue;
			}
		    }
		} else {
		    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		    return MT_FAILURE;
		}
	    }
	}
    }
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_OnePortHaveDate(mt_handle hVpss, mt_handle hPort)
{
    mt_s32 i=0;
    BUFMNG_VPSS_NODE_S* pstTarget;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    for(i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++)
    {
       if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss)
       {
            break;
       }
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        MT_ERR_VDEC("Can't find vpss :%s %d!\n",__FUNCTION__,__LINE__);
        return MT_FAILURE;
    }
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    {
        if((MT_INVALID_HANDLE != pstVpssChan->stPort[hPort].hPort) && (MT_TRUE == pstVpssChan->stPort[hPort].bEnable))
        {
            if(MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == pstVpssChan->stPort[hPort].bufferType)
            {
                if(pstVpssChan->stPort[hPort].stBufVpssInst.pstUnAvailableListPos->next
                    != &pstVpssChan->stPort[hPort].stBufVpssInst.stVpssBufUnAvailableList)
                {
                    pstTarget = list_entry(pstVpssChan->stPort[hPort].stBufVpssInst.pstUnAvailableListPos->next, BUFMNG_VPSS_NODE_S, node);
                    if (NULL == pstTarget)
                    {
                        return MT_FAILURE;
                    }
                    else if (MT_TRUE == pstTarget->bBufUsed)
                    {
                        return MT_SUCCESS;
                    }
               }
               else
               {
                    return MT_FAILURE;
               }
            }
        }
    }
    return MT_SUCCESS;
}
#endif

//add by l00225186
/*Avplay收帧，vdec直接在vpss中取*/
/**
 * VFMW解码后的帧,VPSS线程会通过VDEC DRV接口VDEC_Chan_VpssRecvFrmBuf
 * 读取并做后处理并缓冲.
 * UserSpace的AVPlay MPI线程再调用VDEC DRV的此接口,从VPSS获取处理后的帧,
 * 送到Win和Display进行显示.
 */
static mt_s32 VDEC_Chan_RecvVpssFrmBuf(mt_handle hVpss, MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_s32 i, j = 0;
    //BUFMNG_VPSS_NODE_S *pstTarget;	//not used
    mt_handle hMASTER = MT_INVALID_HANDLE;
    mt_handle hSLAVE = MT_INVALID_HANDLE;

    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_CHAN_STATINFO_S *pstStatInfo = MT_NULL;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    mt_handle hHandle = MT_INVALID_HANDLE;

    static mt_u32 slot_index_pre;
    mt_u32 slot_index_tmp;

    //ENTER_FUNCTION;
//For Debug only
//	dump_stack();

    if ((MT_INVALID_HANDLE == hVpss) || (MT_NULL == pstFrm)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    s32Ret = VDEC_FindVdecHandleByVpssHandle(hVpss, &hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("ERROR:func:%s line:%d!\n", __func__, __LINE__);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    pstStatInfo = &pstChan->stStatInfo;
    pstFrm->u32FrmNum = 0;

    /*find astChanEntity by vpss handle*/
    /*CNcomment:首先应该找到hVpss对应的是第几个astChanEntity*/
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (hVpss == s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    break;
	}
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Can't find vpss :%s %d!\n", __FUNCTION__, __LINE__);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[i]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", i);
	return MT_FAILURE;
    }
    pstStatInfo->u32AvplayRcvFrameTry++;
    pstVpssChan = &(s_stVdecDrv.astChanEntity[i].stVpssChan);
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[j].hPort) && (MT_TRUE == pstVpssChan->stPort[j].bEnable)) {
	    if ((MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE == pstVpssChan->stPort[j].bufferType) && ((VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[j].enPortType) || (VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[j].enPortType))) {
		if (VDEC_PORT_TYPE_MASTER == pstVpssChan->stPort[j].enPortType) {
		    hMASTER = pstVpssChan->stPort[j].hPort;
		}

		if (VDEC_PORT_TYPE_SLAVE == pstVpssChan->stPort[j].enPortType) {
		    hSLAVE = pstVpssChan->stPort[j].hPort;
		}
	    }
	}
    }

    if (hMASTER != MT_INVALID_HANDLE || hSLAVE != MT_INVALID_HANDLE) {
	MT_DRV_VPSS_PORT_AVAILABLE_S stCanGetFrm;

	if (hMASTER != MT_INVALID_HANDLE) {
	    stCanGetFrm.hPort = hMASTER;
	    stCanGetFrm.bAvailable = MT_FALSE;
	    s_stVdecDrv.pVpssFunc->pfnVpssSendCommand(pstVpssChan->hVpss,
	                                              MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE, &(stCanGetFrm));
	    if (stCanGetFrm.bAvailable == MT_FALSE) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		//printk("SSSS 0000 0x%x 0x%x\n", hMASTER, hSLAVE);
		return MT_FAILURE;
	    }
	}

	if (hSLAVE != MT_INVALID_HANDLE) {
	    stCanGetFrm.hPort = hSLAVE;
	    stCanGetFrm.bAvailable = MT_FALSE;
	    s_stVdecDrv.pVpssFunc->pfnVpssSendCommand(pstVpssChan->hVpss,
	                                              MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE, &(stCanGetFrm));
	    if (stCanGetFrm.bAvailable == MT_FALSE) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		return MT_FAILURE;
	    }
	}
    }

    /*调用vpss接口获取主port数据*/
    if (hMASTER != MT_INVALID_HANDLE) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortFrame)(hMASTER, &pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    //MT_ERR_VDEC("Get MainPort Frame err!\n");
	    return MT_FAILURE;
	}

	//printk("\n JJJJ 1111 slot_idx = %d \n", pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo.slotInfo.filedInfoTop.slot_idx);
	slot_index_tmp = pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo.slotInfo.filedInfoTop.slot_idx;
  if(slot_index_tmp == slot_index_pre)
  {
    //printk("YYYY 5555 slot_index_tmp = %d, slot_index_pre = %d \n",slot_index_tmp, slot_index_pre);
  }
  slot_index_pre = slot_index_tmp;

	pstFrm->stFrame[pstFrm->u32FrmNum].hport = hMASTER;
	//memcpy(&(pstChan->stLastVpssFrm), &pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo, sizeof(MT_DRV_VIDEO_FRAME_S));
	pstFrm->u32FrmNum++;
    }

//20180820
#if 0
    if (hSLAVE != MT_INVALID_HANDLE) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortFrame)(hSLAVE, &pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	    MT_ERR_VDEC("Get SlavePort Frame err!\n");
	    return MT_FAILURE;
	}
	pstFrm->stFrame[pstFrm->u32FrmNum].hport = hSLAVE;
	pstFrm->u32FrmNum++;
    }

    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[j].hPort) && (1 == pstVpssChan->stPort[j].bEnable)) {
	    if ((MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE == pstVpssChan->stPort[j].bufferType) && (VDEC_PORT_TYPE_VIRTUAL == pstVpssChan->stPort[j].enPortType)) {
		/*调用vpss接口获取从port数据*/
		s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortFrame)(pstVpssChan->stPort[j].hPort, &pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo);
		if (MT_SUCCESS == s32Ret) {
		    pstFrm->stFrame[pstFrm->u32FrmNum].hport = pstVpssChan->stPort[j].hPort;
		    pstFrm->u32FrmNum++;
		}
	    }
	}
    }

    if (MT_SUCCESS != VDEC_Chan_AllPortHaveDate(hVpss)) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    }

    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	if ((MT_INVALID_HANDLE != pstVpssChan->stPort[j].hPort) && (MT_TRUE == pstVpssChan->stPort[j].bEnable)) {
	    if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == pstVpssChan->stPort[j].bufferType) {
		/*从vdec维护的队列中获取数据*/
		BUFMNG_SpinLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		if (pstVpssChan->stPort[j].stBufVpssInst.u32AvaiableFrameCnt <= 0) {
		    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		    return MT_FAILURE;
		}
		if ((pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next) == &(pstVpssChan->stPort[j].stBufVpssInst.stVpssBufAvailableList)) {
		    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		    return MT_FAILURE;
		}
		pstTarget = list_entry(pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next, BUFMNG_VPSS_NODE_S, node);
		if (MT_NULL == pstTarget) {
		    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		    return MT_FAILURE;
		}

		if (MT_DRV_VDEC_BUF_STATE_IN_USER == pstTarget->enFrameBufferState) {
		    BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
		    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
		    return MT_FAILURE;
		}
		pstTarget->enFrameBufferState = MT_DRV_VDEC_BUF_STATE_IN_USER;
		memcpy(&(pstFrm->stFrame[pstFrm->u32FrmNum].stFrameVideo), &(pstTarget->stVpssOutFrame), sizeof(MT_DRV_VIDEO_FRAME_S));
		pstVpssChan->stPort[j].stBufVpssInst.u32AvaiableFrameCnt--;
		//if(pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next
		//    != &pstVpssChan->stPort[j].stBufVpssInst.stVpssBufUnAvailableList)
		{
		    pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos = pstVpssChan->stPort[j].stBufVpssInst.pstUnAvailableListPos->next;
		}
		pstFrm->stFrame[pstFrm->u32FrmNum].hport = pstVpssChan->stPort[pstFrm->u32FrmNum].hPort;
		pstFrm->u32FrmNum++;
		BUFMNG_SpinUnLockIRQ(&pstVpssChan->stPort[j].stBufVpssInst.stUnAvailableListLock);
	    }
	}
    }
#endif

    if (0 == pstFrm->u32FrmNum) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);
	return MT_FAILURE;
    }

    pstStatInfo->u32AvplayRcvFrameOK++;
    //printk("WWWW 2222 u32AvplayRcvFrameOK = %d, slot_index = %d \n", pstStatInfo->u32AvplayRcvFrameOK, slot_index_tmp);
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[i]);

	if (pstFrm->stFrame[0].stFrameVideo.slotInfo.progressive_frame) {
		pstFrm->stFrame[0].stFrameVideo.bProgressive = MT_TRUE;
	} else {
		pstFrm->stFrame[0].stFrameVideo.bProgressive = MT_FALSE;
	}

	//debug
	if (DEBUG_DUMP_VPSS_FRAME)
	{
		vdec_dump_vpss_frame(pstFrm);
	}
//Trace Image Flow
//AP MPI Image start point::
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("\n[%s](%lu): frm_cnt %u, slot %u, pts %llu, eos %u\n",__FUNCTION__,gettid(),
			pstFrm->stFrame[0].stFrameVideo.slotInfo.frm_cnt,
			pstFrm->stFrame[0].stFrameVideo.slotInfo.filedInfoTop.slot_idx,
			pstFrm->stFrame[0].stFrameVideo.slotInfo.pts,
			pstFrm->stFrame[0].stFrameVideo.slotInfo.end_of_stream_flag);
#endif

    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetEsBuf(mt_handle hHandle, VDEC_ES_BUF_S *pstEsBuf)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == pstEsBuf) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Get */
    s32Ret = VDEC_GetStrmBuf(pstChan->hStrmBuf, pstEsBuf, MT_FALSE);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return s32Ret;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_PutEsBuf(mt_handle hHandle, VDEC_ES_BUF_S *pstEsBuf)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == pstEsBuf) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    s32Ret = VDEC_PutStrmBuf(pstChan->hStrmBuf, pstEsBuf, MT_FALSE);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return s32Ret;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

static mt_void VDEC_ConvertFrm(MT_UNF_VCODEC_TYPE_E enType, VDEC_CHANNEL_S *pstChan,
                               VDEC_CHAN_STATE_S *pstChanState, IMAGE *pstImage, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    UINT32 u32fpsInteger, u32fpsDecimal;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32Reserve);
    MT_DRV_VIDEO_PRIVATE_S *pstVideoPriv = (MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv);
		MT_DIS_FRAME_SLOT_INFO_T *pstSlot = (MT_DIS_FRAME_SLOT_INFO_T*)&pstImage->slotInfo;

    //ENTER_FUNCTION;
    memset(pstFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));	//!!! move to here

    BUG_ON(sizeof(MT_DIS_FRAME_SLOT_INFO_T) != sizeof(MT_DIS_FRAME_SLOT_INFO_T));

    memcpy(&pstFrame->slotInfo, &pstImage->slotInfo, sizeof(MT_DIS_FRAME_SLOT_INFO_T));

	///// Image Begin /////

	/* PATCH: some fields of IMAGE are not assigned and invalid,
     * but DIS_FRAME_SLOT_INFO_T is valid,
     * so do assign IMAGE from DIS_FRAME_SLOT_INFO_T firstly.
	 */
	pstImage->SrcPts				= pstSlot->pts;
	pstImage->PTS					= pstSlot->pts;
	//pstImage->Usertag
	//pstImage->DispTime
	//pstImage->luma_vir_addr
	//pstImage->chrom_vir_addr
	//pstImage->luma_tf_vir_addr
	//pstImage->chrom_tf_vir_addr
	//pstImage->luma_2d_vir_addr
	//pstImage->chrom_2d_vir_addr
	//pstImage->line_num_vir_addr

	if (pstSlot->aspect_ratio == 1) {
		pstImage->u32AspectWidth	= 16;
		pstImage->u32AspectHeight	= 9;
	} else if (pstSlot->aspect_ratio == 2) {
		pstImage->u32AspectWidth	= 4;
		pstImage->u32AspectHeight	= 3;
	} else {
	}

	//pstImage->DispEnableFlag
	//pstImage->DispFrameDistance
	//pstImage->DistanceBeforeFirstFrame
	//pstImage->GopNum
	pstImage->u32RepeatCnt          = pstSlot->repeat_frm_num;

	pstImage->top_luma_phy_addr 	= pstSlot->filedInfoTop.addrLuma;
	pstImage->top_chrom_phy_addr 	= pstSlot->filedInfoTop.addrChroma;
	pstImage->btm_luma_phy_addr 	= pstSlot->filedInfoBot.addrLuma;
	pstImage->btm_chrom_phy_addr 	= pstSlot->filedInfoBot.addrLuma;

	//pstImage->luma_phy_addr
	//pstImage->chrom_phy_addr
	//pstImage->luma_tf_phy_addr
	//pstImage->chrom_tf_phy_addr
	//pstImage->luma_2d_phy_addr
	//pstImage->chrom_2d_phy_addr

	pstImage->is_fld_save			= pstSlot->frame_pic_flag?0:1;
	//pstImage->top_fld_type
	//pstImage->bottom_fld_type

	//pstImage->format
	pstImage->image_width			= pstSlot->pic_width;
	pstImage->image_height			= pstSlot->pic_height;
	pstImage->disp_width			= pstSlot->pic_width;
	pstImage->disp_height			= pstSlot->pic_height;
	//pstImage->disp_center_x
	//pstImage->disp_center_y
	//in Q10
	pstImage->frame_rate			= (pstSlot->input_rate * 1024);

	pstImage->image_stride			= pstSlot->filedInfoTop.stride_data_luma * pstSlot->pic_height
										+ pstSlot->filedInfoTop.stride_data_chroma * pstSlot->pic_height / 2;

	//pstImage->image_id
	pstImage->error_level			= pstSlot->dirty_flag;	//???
	//pstImage->seq_cnt
	pstImage->seq_img_cnt			= pstSlot->frm_cnt;
	//pstImage->p_usrdat

	//pstImage->chroma_idc
	pstImage->bit_depth_luma		= pstSlot->filedInfoTop.bit_depth_luma;
	pstImage->bit_depth_chroma		= pstSlot->filedInfoTop.bit_depth_chroma;
	pstImage->frame_idx             = pstSlot->frm_cnt;
	//pstImage->last_frame
	//pstImage->view_id
	//pstImage->image_id_1
	pstImage->is_3D 				= pstSlot->is_3D_flag;

	pstImage->top_luma_phy_addr_1 	= pstSlot->filedInfoTopRight.addrLuma;
	pstImage->top_chrom_phy_addr_1 	= pstSlot->filedInfoTopRight.addrChroma;
	//pstImage->btm_luma_phy_addr_1
	//pstImage->btm_chrom_phy_addr_1
	//pstImage->line_num_phy_addr
	//pstImage->BTLInfo_1

#ifdef VFMW_BVT_SUPPORT
	pstImage->luma_sum_h
	pstImage->luma_sum_l
	pstImage->luma_historgam
#endif

	//pstImage->is_1Dcompress
	pstImage->eFramePackingType		= pstSlot->packing_type;
	//pstImage->ImageDnr
	//pstImage->BTLInfo
	//pstImage->optm_inf

	///// Image End /////

	//FIXME: not support 'format'
    //if ((pstImage->format & 0x3000) != 0) {
	if (pstSlot->top_field_first) {
		pstFrame->bTopFieldFirst = MT_TRUE;
    } else {
		pstFrame->bTopFieldFirst = MT_FALSE;
    }

/* Don't use 0x1C000 bits. */

/* Image compress flag to frame flag */
#if 1
    pstPrivInfo->stCompressInfo.u32CompressFlag = (1 == pstImage->BTLInfo.u32IsCompress) ? 1 : 0;
#else
    pstPrivInfo->stCompressInfo.u32CompressFlag = (1 == pstImage->ImageDnr.s32VcmpEn) ? 1 : 0;
#endif

    if (0 == (pstImage->ImageDnr.s32VcmpFrameHeight % 16)) {
	pstPrivInfo->stCompressInfo.s32CompFrameHeight = pstImage->ImageDnr.s32VcmpFrameHeight;
    } else {
	MT_WARN_VDEC("s32CompFrameHeight err!\n");
	pstPrivInfo->stCompressInfo.s32CompFrameHeight = 0;
    }

    if (0 == (pstImage->ImageDnr.s32VcmpFrameWidth % 16)) {
	pstPrivInfo->stCompressInfo.s32CompFrameWidth = pstImage->ImageDnr.s32VcmpFrameWidth;
    } else {
	MT_WARN_VDEC("s32CompFrameWidth err!\n");
	pstPrivInfo->stCompressInfo.s32CompFrameWidth = 0;
    }

//FIXME: not support Pixel Format
#if 0
    if (MT_UNF_VCODEC_TYPE_H263 == enType || MT_UNF_VCODEC_TYPE_SORENSON == enType) {
	pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
    } else if (MT_UNF_VCODEC_TYPE_MJPEG == enType) {
	switch (pstImage->BTLInfo.YUVFormat) {
	case SPYCbCr400:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV08;
	    break;
	case SPYCbCr411:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV12_411;
	    break;
	case SPYCbCr422_1X2:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV16;
	    break;
	case SPYCbCr422_2X1:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV61_2X1;
	    break;
	case SPYCbCr444:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV24;
	    break;
	case PLNYCbCr400:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV400;
	    break;
	case PLNYCbCr411:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV411;
	    break;
	case PLNYCbCr420:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV420p;
	    break;
	case PLNYCbCr422_1X2:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV422_1X2;
	    break;
	case PLNYCbCr422_2X1:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV422_2X1;
	    break;
	case PLNYCbCr444:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV_444;
	    break;
	case PLNYCbCr410:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_YUV410p;
	    break;
	case SPYCbCr420:
	default:
	    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
	    break;
	}
    } else {
	if (0 == EnVcmp) {
	    switch ((pstImage->format >> 2) & 7) {
	    case 0:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE;
		break;
	    case 1:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE;
		break;
	    default:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE;
		break;
	    }
	} else {
	    switch ((pstImage->format >> 2) & 7) {
	    case 0:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE_CMP;
		break;
	    case 1:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE_CMP;
		break;
	    default:
		pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21_TILE_CMP;
		break;
	    }
	}
    }
#else
	//HW VDEC YUV output is MT_DRV_PIX_FMT_NV12_TILE in fact,
	//but software code all place only supports MT_DRV_PIX_FMT_NV21,
	//so hack it to MT_DRV_PIX_FMT_NV21.
	//pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV12_TILE;
	pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
#endif

//FIXME: not support Display Norm
#if 0
    switch (pstImage->format & 0xE0) {
    case 0x20:
	pstChan->enDisplayNorm = MT_UNF_ENC_FMT_PAL;
	break;
    case 0x40:
	pstChan->enDisplayNorm = MT_UNF_ENC_FMT_NTSC;
	break;
    default:
	pstChan->enDisplayNorm = MT_UNF_ENC_FMT_BUTT;
	break;
    }
#else
    switch(pstSlot->video_format) {
    case VIDEO_FORMAT_PAL:
        pstChan->enDisplayNorm = MT_UNF_ENC_FMT_PAL;
        break;
    case VIDEO_FORMAT_NTSC:
        pstChan->enDisplayNorm = MT_UNF_ENC_FMT_NTSC;
        break;
    default:
	pstChan->enDisplayNorm = MT_UNF_ENC_FMT_BUTT;
	break;
    }
#endif

//FIXME: not support 'format'
#if 0
    switch (pstImage->format & 0x300) {
    case 0x0: /* PROGRESSIVE */
	pstFrame->bProgressive = MT_TRUE;
	pstChan->stStatInfo.u32FrameType[1]++;
	break;
    case 0x100: /* INTERLACE */
    case 0x200: /* INFERED_PROGRESSIVE */
    case 0x300: /* INFERED_INTERLACE */
    default:
	pstFrame->bProgressive = MT_FALSE;
	pstChan->stStatInfo.u32FrameType[0]++;
	break;
    }

    switch (pstImage->format & 0x300) {
    case 0x0: /* PROGRESSIVE */
	pstVideoPriv->eSampleType = MT_DRV_SAMPLE_TYPE_PROGRESSIVE;
	break;
    case 0x100: /* INTERLACE */
	pstVideoPriv->eSampleType = MT_DRV_SAMPLE_TYPE_INTERLACE;
	break;
    case 0x200: /* INFERED_PROGRESSIVE */
    case 0x300: /* INFERED_INTERLACE */
    default:
	pstVideoPriv->eSampleType = MT_DRV_SAMPLE_TYPE_UNKNOWN;

	break;
    }
#else
	if (pstSlot->progressive_frame) {
		pstFrame->bProgressive = MT_TRUE;
		pstChan->stStatInfo.u32FrameType[1]++;
		pstVideoPriv->eSampleType = MT_DRV_SAMPLE_TYPE_PROGRESSIVE;
	} else {
		pstFrame->bProgressive = MT_FALSE;
		pstChan->stStatInfo.u32FrameType[0]++;
		pstVideoPriv->eSampleType = MT_DRV_SAMPLE_TYPE_INTERLACE;
	}
#endif

//FIXME: not support 'format'
#if 0
    switch (pstImage->format & 0xC00) {
    case 0x400:
	pstFrame->enFieldMode = MT_DRV_FIELD_TOP;
	break;
    case 0x800:
	pstFrame->enFieldMode = MT_DRV_FIELD_BOTTOM;
	break;
    case 0xC00:
	pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
	break;
    default:
	pstFrame->enFieldMode = MT_DRV_FIELD_BUTT;
	break;
    }
#else
	if (pstSlot->frame_pic_flag) {
		pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
	} else {
		if (pstSlot->isHalf == 1) {
			pstFrame->enFieldMode = MT_DRV_FIELD_TOP;
		} else if (pstSlot->isHalf == 2) {
			pstFrame->enFieldMode = MT_DRV_FIELD_BOTTOM;
		} else if (pstSlot->isHalf == 3) {
			pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
		} else {
			pstFrame->enFieldMode = MT_DRV_FIELD_BUTT;
		}
	}
#endif

    if (MT_UNF_VCODEC_TYPE_MJPEG == enType || MT_UNF_VCODEC_TYPE_SORENSON == enType || MT_UNF_VCODEC_TYPE_H263 == enType) {
	pstFrame->stBufAddr[0].u32PhyAddr_Y = pstImage->top_luma_phy_addr;
	pstFrame->stBufAddr[0].u32Stride_Y = pstImage->image_stride;
	pstFrame->stBufAddr[0].u32PhyAddr_YHead = pstImage->BTLInfo.u32YHeadAddr;
	pstFrame->stBufAddr[0].u32PhyAddr_C = pstImage->top_chrom_phy_addr;
	pstFrame->stBufAddr[0].u32Stride_C = pstFrame->stBufAddr[0].u32Stride_Y;
	pstFrame->stBufAddr[0].u32PhyAddr_CHead = pstImage->BTLInfo.u32CHeadAddr;
	pstFrame->stBufAddr[0].u32PhyAddr_Cr = pstImage->BTLInfo.u32CrAddr;
	pstFrame->stBufAddr[0].u32Stride_Cr = pstImage->BTLInfo.u32CrStride;
	pstFrame->stBufAddr[0].u32PhyAddr_CrHead = pstImage->BTLInfo.u32CHeadAddr;
    } else {
	if (0 == EnVcmp) {
	    pstFrame->stBufAddr[0].u32PhyAddr_Y = pstImage->top_luma_phy_addr;
	    if (MT_UNF_VCODEC_TYPE_H263 == enType) {
		pstFrame->stBufAddr[0].u32Stride_Y = pstImage->image_stride;
	    } else {
		pstFrame->stBufAddr[0].u32Stride_Y = pstImage->image_stride / 16;	//???
	    }
	    pstFrame->stBufAddr[0].u32PhyAddr_C = pstImage->top_chrom_phy_addr;
	    pstFrame->stBufAddr[0].u32Stride_C = pstFrame->stBufAddr[0].u32Stride_Y;
	} else {
	    pstFrame->stBufAddr[0].u32PhyAddr_YHead = pstImage->top_luma_phy_addr;
	    pstFrame->stBufAddr[0].u32PhyAddr_Y = pstImage->top_luma_phy_addr + (pstImage->image_height + 31) / 32 * 32 * 16;
	    pstFrame->stBufAddr[0].u32Stride_Y = pstImage->image_stride / 16;
	    pstFrame->stBufAddr[0].u32PhyAddr_CHead = pstImage->top_chrom_phy_addr;
	    pstFrame->stBufAddr[0].u32PhyAddr_C = pstImage->top_chrom_phy_addr + (pstImage->image_height + 31) / 32 * 32 * 8;
	    pstFrame->stBufAddr[0].u32Stride_C = pstFrame->stBufAddr[0].u32Stride_Y;
	}
    }

    //FOR MVC DEBUG
    if (pstImage->is_3D) {

	pstFrame->stBufAddr[1].u32PhyAddr_YHead = pstImage->top_luma_phy_addr_1;
	pstFrame->stBufAddr[1].u32PhyAddr_Y = pstImage->top_luma_phy_addr_1 + (pstImage->image_height + 31) / 32 * 32 * 16;
	pstFrame->stBufAddr[1].u32Stride_Y = pstImage->image_stride / 16;

	pstFrame->stBufAddr[1].u32PhyAddr_CHead = pstImage->top_chrom_phy_addr_1;
	pstFrame->stBufAddr[1].u32PhyAddr_C = pstImage->top_chrom_phy_addr_1 + (pstImage->image_height + 31) / 32 * 32 * 8;
	pstFrame->stBufAddr[1].u32Stride_C = pstFrame->stBufAddr[1].u32Stride_Y;
	if (0 == EnVcmp) {
	    pstFrame->stBufAddr[1].u32PhyAddr_Y = pstImage->top_luma_phy_addr_1;
	    pstFrame->stBufAddr[1].u32PhyAddr_C = pstImage->top_chrom_phy_addr_1;
	}
    }

    if ((MT_UNF_VCODEC_TYPE_VP6 == enType) || (MT_UNF_VCODEC_TYPE_VP6F == enType) || (MT_UNF_VCODEC_TYPE_VP6A == enType)) {

	if (MT_UNF_VCODEC_TYPE_VP6A == enType) {
	    pstFrame->u32Circumrotate = pstChan->stCurCfg.unExtAttr.stVP6Attr.bReversed & 0x1;
	} else {
	    pstFrame->u32Circumrotate = !(pstChan->stCurCfg.unExtAttr.stVP6Attr.bReversed & 0x1);
	}
    } else {
	pstFrame->u32Circumrotate = 0;
    }

    pstFrame->u32FrameIndex = pstImage->frame_idx;

    pstFrame->u32AspectWidth = pstImage->u32AspectWidth;
    pstFrame->u32AspectHeight = pstImage->u32AspectHeight;
    pstFrame->u32Width = (mt_u32)pstImage->image_width;
    pstFrame->u32Height = (mt_u32)pstImage->image_height;
    pstFrame->stDispRect.s32Width = (mt_s32)pstImage->disp_width;
    pstFrame->stDispRect.s32Height = (mt_s32)pstImage->disp_height;
    pstFrame->stDispRect.s32X = 0;
    pstFrame->stDispRect.s32Y = 0;
    pstFrame->u32ErrorLevel = pstImage->error_level;
    pstFrame->u32SrcPts = (mt_u32)pstImage->SrcPts;
    pstFrame->u32Pts = (mt_u32)pstImage->PTS;
    pstFrame->s64OmxPts = pstImage->PTS;
    pstFrame->u64Pts = pstImage->PTS;
    //pstFrame->u32FrmCnt                        = pstImage->seq_img_cnt;
    pstChan->u32DecodeAspectWidth = pstImage->u32AspectWidth;
    pstChan->u32DecodeAspectHeight = pstImage->u32AspectHeight;
    pstChan->u32LastLumaBitdepth = pstImage->bit_depth_luma;
    pstChan->u32LastChromaBitdepth = pstImage->bit_depth_chroma;

    /* vfmw解码出来的FP信息不外透，统一由上层设置
   MVC传递的是两帧地址，还是保留 */
    if (MT_UNF_VCODEC_TYPE_MVC == enType) {
	switch (pstImage->eFramePackingType) {
	case DISP_FRAME_PACKING_TYPE_NONE:
	    pstFrame->eFrmType = MT_DRV_FT_NOT_STEREO;
	    break;
	case DISP_FRAME_PACKING_TYPE_SIDE_BY_SIDE:
	    pstFrame->eFrmType = MT_DRV_FT_SBS;
	    break;
	case DISP_FRAME_PACKING_TYPE_TOP_BOTTOM:
	    pstFrame->eFrmType = MT_DRV_FT_TAB;
	    break;
	case DISP_FRAME_PACKING_TYPE_TIME_INTERLACED:
	    pstFrame->eFrmType = MT_DRV_FT_FPK;
	    break;
	default:
	    pstFrame->eFrmType = MT_DRV_FT_BUTT;
	    break;
	}
    }

    pstPrivInfo->image_id = pstImage->image_id;
    pstPrivInfo->image_id_1 = pstImage->image_id_1;
    pstPrivInfo->u32SeqFrameCnt = pstImage->seq_img_cnt;
    pstPrivInfo->u8Repeat = !(pstImage->format & 0x80000); /* control vo discard frame bit19 */
    pstPrivInfo->u8TestFlag = pstImage->optm_inf.Rwzb;
    pstPrivInfo->u8EndFrame = pstImage->last_frame;
    //pstPrivInfo->s32FrameFormat = pstImage->format & 0x3;			/* not support 'format' */
    pstPrivInfo->s32FrameFormat = pstSlot->picture_coding_type;
    pstPrivInfo->s32TopFieldFrameFormat = pstImage->top_fld_type & 0x03;
    pstPrivInfo->s32BottomFieldFrameFormat = pstImage->bottom_fld_type & 0x03;
    pstPrivInfo->s32FieldFlag = pstImage->is_fld_save;

    pstPrivInfo->s32GopNum = pstImage->GopNum;

    /* For VC1 */
    if (MT_UNF_VCODEC_TYPE_VC1 == enType) {
	pstPrivInfo->u32BeVC1 = MT_TRUE;
	pstPrivInfo->stVC1RangeInfo.u8PicStructure = pstImage->ImageDnr.pic_structure;
	pstPrivInfo->stVC1RangeInfo.u8PicQPEnable = pstImage->ImageDnr.use_pic_qp_en;
	pstPrivInfo->stVC1RangeInfo.s32QPY = pstImage->ImageDnr.QP_Y;
	pstPrivInfo->stVC1RangeInfo.s32QPU = pstImage->ImageDnr.QP_U;
	pstPrivInfo->stVC1RangeInfo.s32QPV = pstImage->ImageDnr.QP_V;
	pstPrivInfo->stVC1RangeInfo.u8ChromaFormatIdc = pstImage->ImageDnr.chroma_format_idc;
	pstPrivInfo->stVC1RangeInfo.u8VC1Profile = pstImage->ImageDnr.vc1_profile;
	pstPrivInfo->stVC1RangeInfo.s32RangedFrm = pstImage->ImageDnr.Rangedfrm;
	pstPrivInfo->stVC1RangeInfo.u8RangeMapYFlag = pstImage->ImageDnr.Range_mapy_flag;
	pstPrivInfo->stVC1RangeInfo.u8RangeMapY = pstImage->ImageDnr.Range_mapy;
	pstPrivInfo->stVC1RangeInfo.u8RangeMapUVFlag = pstImage->ImageDnr.Range_mapuv_flag;
	pstPrivInfo->stVC1RangeInfo.u8RangeMapUV = pstImage->ImageDnr.Range_mapuv;
	pstPrivInfo->stVC1RangeInfo.u8BtmRangeMapYFlag = pstImage->ImageDnr.bottom_Range_mapy_flag;
	pstPrivInfo->stVC1RangeInfo.u8BtmRangeMapY = pstImage->ImageDnr.bottom_Range_mapy;
	pstPrivInfo->stVC1RangeInfo.u8BtmRangeMapUVFlag = pstImage->ImageDnr.bottom_Range_mapuv_flag;
	pstPrivInfo->stVC1RangeInfo.u8BtmRangeMapUV = pstImage->ImageDnr.bottom_Range_mapuv;
    } else {
	pstPrivInfo->u32BeVC1 = MT_FALSE;
    }

    //pstFrame->stVideoFrameAddr[0].u32CrAddr = pstImage->BTLInfo.u32CrAddr;
    //pstFrame->stVideoFrameAddr[0].u32CrStride = pstImage->BTLInfo.u32CrStride;
    pstPrivInfo->stCompressInfo.u32HeadOffset = pstImage->BTLInfo.u32HeadOffset;
    pstPrivInfo->stCompressInfo.u32YHeadAddr = pstImage->BTLInfo.u32YHeadAddr;
    pstPrivInfo->stCompressInfo.u32CHeadAddr = pstImage->BTLInfo.u32CHeadAddr;
    pstPrivInfo->stCompressInfo.u32HeadStride = pstImage->BTLInfo.u32HeadStride;
    pstPrivInfo->stBTLInfo.u32BTLImageID = pstImage->BTLInfo.btl_imageid;
    pstPrivInfo->stBTLInfo.u32Is1D = pstImage->BTLInfo.u32Is1D;
    pstPrivInfo->stBTLInfo.u32IsCompress = pstImage->BTLInfo.u32IsCompress;
    pstPrivInfo->stBTLInfo.u32DNROpen = pstImage->BTLInfo.u32DNROpen;
    pstPrivInfo->stBTLInfo.u32DNRInfoAddr = pstImage->BTLInfo.u32DNRInfoAddr;
    pstPrivInfo->stBTLInfo.u32DNRInfoStride = pstImage->BTLInfo.u32DNRInfoStride;

    if (pstFrame->u32Height <= 288) {
	pstFrame->bProgressive = MT_TRUE;
    }

    switch (pstChan->stFrameRateParam.enFrmRateType) {
    case MT_UNF_AVPLAY_FRMRATE_TYPE_USER:
	u32fpsInteger = pstChan->stFrameRateParam.stSetFrmRate.u32fpsInteger;
	u32fpsDecimal = pstChan->stFrameRateParam.stSetFrmRate.u32fpsDecimal;
	//pstFrame->stFrameRate.u32fpsInteger = pstChan->stFrameRateParam.stSetFrmRate.u32fpsInteger;
	//pstFrame->stFrameRate.u32fpsDecimal = pstChan->stFrameRateParam.stSetFrmRate.u32fpsDecimal;stSetFrmRate.u32fpsDecimal;

	break;

    case MT_UNF_AVPLAY_FRMRATE_TYPE_PTS:
    case MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS:
    case MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM:
    default:
	u32fpsInteger = pstImage->frame_rate / 1024;
	u32fpsDecimal = pstImage->frame_rate % 1024;

	//pstFrame->stFrameRate.u32fpsInteger = pstImage->frame_rate/1024;
	//pstFrame->stFrameRate.u32fpsDecimal = (pstImage->frame_rate*1000/1024)%1000;

	break;
    }
    pstFrame->u32FrameRate = u32fpsInteger * 1000 + u32fpsDecimal;
    if (pstFrame->u32FrameRate < 1000) {
	pstFrame->u32FrameRate = 25000;
    }
    if (MT_TRUE == pstChan->bLowdelay) {
	pstFrame->u32TunnelPhyAddr = pstImage->line_num_phy_addr;
    } else {
	pstFrame->u32TunnelPhyAddr = 0;
    }
    pstFrame->enBitWidth = MT_DRV_PIXEL_BITWIDTH_8BIT;
    pstPrivInfo->entype = enType;
    if (0 == pstImage->is_fld_save) {
	pstVideoPriv->ePictureMode = MT_DRV_PICTURE_FRAME;
    } else if (1 == pstImage->is_fld_save) {
	pstVideoPriv->ePictureMode = MT_DRV_PICTURE_FIELD;
    } else {
	pstVideoPriv->ePictureMode = MT_DRV_PICTURE_BUTT;
    }

    ((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32PrivDispTime = pstImage->DispTime;
    ((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32PlayTime = pstImage->u32RepeatCnt;
    ((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->eColorSpace = MT_DRV_CS_BT601_YUV_LIMITED;
    pstVideoPriv->stVideoOriginalInfo.enSource = MT_DRV_SOURCE_DTV;
    pstVideoPriv->stVideoOriginalInfo.u32Width = pstImage->disp_width;
    pstVideoPriv->stVideoOriginalInfo.u32Height = pstImage->disp_height;
    pstVideoPriv->stVideoOriginalInfo.u32FrmRate = pstFrame->u32FrameRate;
    pstVideoPriv->stVideoOriginalInfo.en3dType = pstFrame->eFrmType;
    pstVideoPriv->stVideoOriginalInfo.enSrcColorSpace = MT_DRV_CS_BT601_YUV_LIMITED;
    pstVideoPriv->stVideoOriginalInfo.enColorSys = MT_DRV_COLOR_SYS_AUTO;
    pstVideoPriv->stVideoOriginalInfo.bGraphicMode = MT_FALSE;
    //pstVideoPriv->stVideoOriginalInfo.bInterlace = pstFrame->bProgressive;	//???
    pstVideoPriv->stVideoOriginalInfo.bInterlace = !pstFrame->bProgressive;
}

static mt_s32 VDEC_RlsFrm(VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = VDEC_OK;
    IMAGE stImage;	//stack might overflow!
    //not used
    //IMAGE_INTF_S *pstImgInft = &pstChan->stImageIntf;
    VDEC_CHAN_STATINFO_S *pstStatInfo = &pstChan->stStatInfo;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32Reserve);

    //ENTER_FUNCTION;
    memset(&stImage, 0, sizeof(stImage));
    stImage.image_stride = pstFrame->stBufAddr[0].u32Stride_Y;
    stImage.image_height = pstFrame->u32Height;
    stImage.image_width = pstFrame->u32Width;
    stImage.luma_phy_addr = pstFrame->stBufAddr[0].u32PhyAddr_Y;
    stImage.top_luma_phy_addr = pstFrame->stBufAddr[0].u32PhyAddr_Y;
    stImage.image_id = pstPrivInfo->image_id;
    stImage.image_id_1 = pstPrivInfo->image_id_1;
    stImage.BTLInfo.btl_imageid = pstPrivInfo->stBTLInfo.u32BTLImageID;
    stImage.BTLInfo.u32Is1D = pstPrivInfo->stBTLInfo.u32Is1D;

    pstStatInfo->u32VdecRlsFrameTry++;
    //s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, &stImage);
    if (VDEC_OK != s32Ret) {
	pstStatInfo->u32VdecRlsFrameFail++;
	return MT_FAILURE;
    } else {
	pstStatInfo->u32VdecRlsFrameOK++;
	return MT_SUCCESS;
    }
}

//if drv read image success, but check fail, shall release it.
static mt_s32 VDEC_Do_RlsFrm(VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = VDEC_OK;
    IMAGE_INTF_S *pstImgInft = &pstChan->stImageIntf;
    VDEC_CHAN_STATINFO_S *pstStatInfo = &pstChan->stStatInfo;

    ENTER_FUNCTION;

    pstStatInfo->u32VdecRlsFrameTry++;
	if (pstImgInft != MT_NULL && pstImgInft->release_image != MT_NULL && pstFrame != MT_NULL)
	{
	    s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, (void*)&pstFrame->slotInfo);
	    if (VDEC_OK != s32Ret) {
			pstStatInfo->u32VdecRlsFrameFail++;
			return MT_FAILURE;
	    } else {
			pstStatInfo->u32VdecRlsFrameOK++;
			return MT_SUCCESS;
	    }
	}
	return MT_FAILURE;
}

static mt_s32 VDEC_CheckConvertFrm(VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    if (0 == pstFrame->u32Height || 0 == pstFrame->u32Width) {
		MLOGW("%s: bad frame(slot %u) width/height (%u, %u)!\n",__FUNCTION__,
			pstFrame->slotInfo.filedInfoTop.slot_idx,pstFrame->u32Width,pstFrame->u32Height);
		(mt_void)VDEC_Do_RlsFrm(pstChan, pstFrame);
		return MT_FAILURE;
    }
    return MT_SUCCESS;
}

static mt_s32 VDEC_RecvFastBackFrm(mt_handle hHandle, IMAGE_INTF_S *pstImgInft, IMAGE *pstImage)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    /*receive an effective image from vfmw*/
    s32Ret = pstImgInft->read_image(pstImgInft->image_provider_inst_id, pstImage);

    if (VDEC_OK == s32Ret) {
	if (MT_FALSE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32BackwardOptimizeFlag) {
	    if (MT_FALSE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstIFrameFlag) {
		if (((pstImage->is_fld_save == 0) && (0 == (pstImage->format & 0x03))) ||
		    ((pstImage->is_fld_save == 1) && ((0 == (pstImage->top_fld_type & 0x03)) || (0 == (pstImage->bottom_fld_type & 0x03))))) {
		    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstIFrameFlag = MT_TRUE;
		    return MT_SUCCESS;
		} else {
		    //s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, pstImage);
		    return MT_FAILURE;
		}
	    } else {
		return MT_SUCCESS;
	    }
	} else {
	    if (MT_TRUE == pstImage->DispEnableFlag) {
		if (MT_FALSE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstIFrameFlag) {
		    //if (pstImage->GopNum !=  s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32LastFrameGopNum)
		    {
			s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32GetFirstIFrameFlag = MT_TRUE;
			//s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32LastFrameGopNum = pstImage->GopNum;
			s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance = pstImage->DistanceBeforeFirstFrame;
			if (0 != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance) {
			    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance--;
			    //s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, pstImage);
			    return MT_FAILURE;
			} else {
			    return MT_SUCCESS;
			}
		    }
		    /*else
                     {
                         can go here?
                         s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, pstImage);
                         return MT_FAILURE;
                     }*/
		} else {
		    if (0 != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance) {
			s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance--;
			//s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, pstImage);
			return MT_FAILURE;
		    } else {
			s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32ImageDistance = pstImage->DispFrameDistance;
			return MT_SUCCESS;
		    }
		}
	    } else {
		//s32Ret = pstImgInft->release_image(pstImgInft->image_provider_inst_id, pstImage);
		return MT_FAILURE;
	    }
	}
    } else {
	return MT_FAILURE;
    }

    return MT_FAILURE;
}

static mt_s32 VDEC_RecvFrm(mt_handle hHandle, VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32UserdataId;
	/* FIXME: avoid stack overflow, but not thread safe! */
    //IMAGE stImage;	//stack might overflow!
    IMAGE *pstImage = &pstChan->stImage;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    VDEC_CHAN_STATE_S stChanState = { 0 };
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
#if (1 == MT_VDEC_CC_FROM_IMAGE)
    mt_u32 u32ID;
    mt_u8 u8Type;
    mt_u32 u32Index;
    MT_VDEC_USRDAT_S *pstUsrData = MT_NULL;
#endif
#endif
    IMAGE_INTF_S *pstImgInft = &pstChan->stImageIntf;
    VDEC_CHAN_STATINFO_S *pstStatInfo = &pstChan->stStatInfo;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32Reserve);
    //MT_VDEC_PRIV_FRAMEINFO_S *pstLastFrmPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstChan->stLastDispFrameInfo.u32Priv))->u32Reserve);
    MT_VDEC_PRIV_FRAMEINFO_S *pstLastFrmPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstChan->stLastFrm.u32Priv))->u32Reserve);
    MT_DRV_VIDEO_PRIVATE_S *pstVideoPriv = (MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv);
	//unused
    //mt_s32 s32FrameRate;
    //ENTER_FUNCTION;
    //memset(pstImage, 0, sizeof(IMAGE));	//!!! no need

    if (pstLastFrmPrivInfo->u8EndFrame == 1) {
      pstPrivInfo->u8EndFrame = 2;
      pstChan->u32EndFrmFlag = 0;
      pstChan->u32LastFrmTryTimes = 0;
      pstChan->u32LastFrmId = -1;
      pstFrame->bProgressive = MT_FALSE;
      pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
      pstVideoPriv->u32LastFlag = DEF_MT_DRV_VPSS_LAST_ERROR_FLAG;

    	return MT_SUCCESS;
    }

    /*VDEC Receive frame from VFMW */
    pstStatInfo->u32VdecRcvFrameTry++;
    if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed < 0) {
    	s32Ret = VDEC_RecvFastBackFrm(hHandle, pstImgInft, pstImage);
    	if (MT_SUCCESS != s32Ret) {
    	    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    	    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);
    	    //(void)(s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(pstVpssChan->hVpss,MT_DRV_VPSS_USER_COMMAND_IMAGEREADY, NULL);
    	}
    } else {
      if(!pstChan->bNoGetImage){
		mt_u32 tk_in = (mt_u32)ktime_to_ms(ktime_get_boottime());
        s32Ret = pstImgInft->read_image(pstImgInft->image_provider_inst_id, pstImage);
		mt_u32 tk_out = (mt_u32)ktime_to_ms(ktime_get_boottime());
		
		if((pstChan->last_recv_time != 0) && ((tk_out - pstChan->last_recv_time) > 40) ) 
		{
			printk(KERN_INFO "read_image   tk_diff = %d ms tk %d ms pre %d ms  cost %d  ms  \n", 
				(tk_out - pstChan->last_recv_time), tk_out, pstChan->last_recv_time, (tk_out - tk_in) );
		}
		pstChan->last_recv_time = tk_out;
		
        if (VDEC_OK == s32Ret)
        {
			if (DEBUG_DUMP_VFMW_FRAME)
			{
				vdec_dump_image(pstImage);
			}

          pstImage->PTS = pstImage->slotInfo.pts;
          pstImage->SrcPts = pstImage->slotInfo.pts;
          pstImage->frame_rate = pstImage->slotInfo.input_rate;
          //printk("WWWW 0000 %x, %x, %x\n", stImage.slotInfo.pic_height, stImage.slotInfo.filedInfoBot.addrLuma, stImage.slotInfo.is_3D_flag);
        }

        //printk("VVVV 0000 inst_id = %x, s32Ret = %d, slot_idx = %d \n", pstImgInft->image_provider_inst_id, s32Ret,
        //stImage.slotInfo.filedInfoTop.slot_idx);
      	//printk("11 %x, %x, %x\n", stImage.slotInfo.pic_height, stImage.slotInfo.filedInfoBot.addrLuma, stImage.slotInfo.is_3D_flag);
      }
    }

    if (VDEC_OK != s32Ret) {
#if 0 //Rock_hu  以后打开
            /* If last frame decode fail, retry 5 times */
        if (((pstChan->u32EndFrmFlag == 1) && (pstChan->u32LastFrmTryTimes++ >= 4)) ||
            /* If report last frame id after this frame had been outputed, check last frame id */
            ((pstChan->u32EndFrmFlag == 2) && (pstLastFrmPrivInfo->image_id%100 == pstChan->u32LastFrmId)) ||//(MT_VDEC_PRIV_FRAMEINFO_S*)pstFrame->u32Priv
            /* For user space decode mode, the first fail means receive over. */
            (pstChan->u32EndFrmFlag == 3))
        {
            /* Last frame is the end frame */
            pstPrivInfo->u8EndFrame = 2;
            pstChan->u32EndFrmFlag = 0;
            pstChan->u32LastFrmTryTimes = 0;
            pstChan->u32LastFrmId = -1;
            pstFrame->bProgressive = MT_FALSE;
            pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
            pstVideoPriv->u32LastFlag = DEF_MT_DRV_VPSS_LAST_ERROR_FLAG;

            return MT_SUCCESS;
        }
#endif
	    //printk("@@@@ 9999 read_image fail\n");

	    return MT_FAILURE;
    }
    pstChan->u32LastFrmTryTimes = 0;

    pstStatInfo->u32VdecRcvFrameOK++;
    pstStatInfo->u32TotalVdecOutFrame++;

#if 0 // Rock_hu
    /* Calculate PTS */
    PTSREC_CalcStamp(hHandle, pstChan->stCurCfg.enType, &stImage);
    s32FrameRate = stImage.frame_rate / 1024;
    if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed < 0) {
    	s32FrameRate = 60;
    	//s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_FRAME_RATE, &s32FrameRate);
    }
    if (s32FrameRate > 30 && s32FrameRate > (pstChan->stLastDispFrameInfo.u32FrameRate / 1000)) {
    	//s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_FRAME_RATE, &s32FrameRate);
    }

    /*interleaved source, VPSS module swtich field to frame, need to adjust pts*/
    pstPrivInfo->s32InterPtsDelta = PTSREC_GetInterPtsDelta(hHandle);
#endif

    /* Save user data for watermark */
    for (u32UserdataId = 0; u32UserdataId < 4; u32UserdataId++) {
    	pstChan->pu8UsrDataForWaterMark[u32UserdataId] = pstImage->p_usrdat[u32UserdataId];
    }

    /* Get channel state */
    /*s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &stChanState);
    if (VDEC_OK != s32Ret) {
    	MT_FATAL_VDEC("Chan %d GET_CHAN_STATE err\n", pstChan->hChan);
    }*/
#if (0 == VDEC_USERDATA_NEED_ARRANGE)
    if (MT_NULL != pstImage->p_usrdat) {
    	USRDATA_Arrange(hHandle, pstImage->p_usrdat[0]);
    }
#endif

    /* Convert VFMW-IMAGE to VO-MT_UNF_VIDEO_FRAME_INFO_S */
    VDEC_ConvertFrm(pstChan->stCurCfg.enType, pstChan, &stChanState, pstImage, pstFrame);
    s32Ret = VDEC_CheckConvertFrm(pstChan, pstFrame);
    if (MT_SUCCESS != s32Ret) {
    	return MT_FAILURE;
    }
#if 1
    if ((pstFrame->stDispRect.s32Height != pstChan->stLastFrm.stDispRect.s32Height) ||
        (pstFrame->stDispRect.s32Width != pstChan->stLastFrm.stDispRect.s32Width)) {
    	pstChan->bNormChange = MT_TRUE;
    	pstChan->stNormChangeParam.enNewFormat = pstChan->enDisplayNorm;
    	pstChan->stNormChangeParam.u32ImageWidth = pstFrame->stDispRect.s32Width & 0xfffffffe;
    	pstChan->stNormChangeParam.u32ImageHeight = pstFrame->stDispRect.s32Height & 0xfffffffc;
    	pstChan->stNormChangeParam.u32FrameRate = pstFrame->u32FrameRate;
    	pstChan->stNormChangeParam.bProgressive = pstFrame->bProgressive;
    }
    /*set framePackingType for pstFrame*/
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);
    if (MT_UNF_FRAME_PACKING_TYPE_BUTT != pstVpssChan->eFramePackType) {
    	switch (pstVpssChan->eFramePackType) {
    	case MT_UNF_FRAME_PACKING_TYPE_NONE: /**< Normal frame, not a 3D frame */
    	    pstFrame->eFrmType = MT_DRV_FT_NOT_STEREO;
    	    break;
    	case MT_UNF_FRAME_PACKING_TYPE_3D_TILE: /**< Tile frame, for vpss to convert 3D */
    	    pstFrame->eFrmType = MT_DRV_FT_TILE;
    	    break;
      case MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE: /**< Side by side */
    	    pstFrame->eFrmType = MT_DRV_FT_SBS;
    	    if (pstImage->disp_width / 2 > pstImage->disp_height) {
    		pstFrame->u32AspectWidth = pstImage->disp_width / 2;
    		pstFrame->u32AspectHeight = pstImage->disp_height;
    	    } else {
    		pstFrame->u32AspectWidth = pstImage->disp_width;
    		pstFrame->u32AspectHeight = pstImage->disp_height;
    	    }
    	    s_stVdecDrv.astChanEntity[hHandle].pstChan->u32UserSetAspectWidth = pstFrame->u32AspectWidth;
    	    s_stVdecDrv.astChanEntity[hHandle].pstChan->u32UserSetAspectHeight = pstFrame->u32AspectHeight;
    	    break;
    	case MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM: /**< Top and bottom */
    	    pstFrame->eFrmType = MT_DRV_FT_TAB;
    	    if (pstImage->disp_width < pstImage->disp_height) {
    		pstFrame->u32AspectWidth = pstImage->disp_width;
    		pstFrame->u32AspectHeight = pstImage->disp_height / 2;
    	    } else {
    		pstFrame->u32AspectWidth = pstImage->disp_width;
    		pstFrame->u32AspectHeight = pstImage->disp_height;
    	    }
    	    s_stVdecDrv.astChanEntity[hHandle].pstChan->u32UserSetAspectWidth = pstFrame->u32AspectWidth;
    	    s_stVdecDrv.astChanEntity[hHandle].pstChan->u32UserSetAspectHeight = pstFrame->u32AspectHeight;
    	    break;
    	case MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED: /**< Time interlaced: one frame for left eye, the next frame for right eye */
    	    //pstFrame->eFrmType = MT_DRV_FT_FPK;
    	    break;
    	default:
    	    pstFrame->eFrmType = MT_DRV_FT_BUTT;
    	    break;
    	}
    }
    if (pstFrame->eFrmType != pstChan->stLastFrm.eFrmType) {
	pstChan->bFramePackingChange = MT_TRUE;
	pstChan->enFramePackingType = VDEC_ConverFrameType(pstFrame->eFrmType);
    }
    memcpy(&(pstChan->stLastFrm), pstFrame, sizeof(MT_DRV_VIDEO_FRAME_S));
#endif
    pstFrame->hTunnelSrc = (MT_ID_VDEC << 16) | hHandle;
    if (pstImage->image_id % 100 == pstChan->u32LastFrmId) {
	pstPrivInfo->u8EndFrame = 1;
	pstVideoPriv->u32LastFlag = DEF_MT_DRV_VPSS_LAST_FRAME_FLAG;
    }
    /* Count err frame */
    pstStatInfo->u32VdecErrFrame = stChanState.error_frame_num;
    //w00271806  vdec proc ErrFrame count
    pstStatInfo->u32VdecDecErrFrame = stChanState.dec_error_frame_num;

    pstFrame->bIsFirstIFrame = MT_FALSE;

    /*Record the interval of I frames and the output time of the first I frame*/
    /*CNcomment: 记录I帧间隔和换台后第一个I帧解码输出时间 */
    if (0 == (pstImage->format & 0x3)) /* I frame */
    {
	mt_drv_stat_event(STAT_EVENT_IFRAMEINTER, pstFrame->u32Pts);
	if (1 == pstStatInfo->u32TotalVdecOutFrame) {
	    pstFrame->bIsFirstIFrame = MT_TRUE;
	    mt_drv_stat_event(STAT_EVENT_IFRAMEOUT, 0);
	}
    }

    if (pstChan->bIsIFrameDec) {
	//pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
	pstFrame->bProgressive = MT_TRUE;
	pstPrivInfo->u8Marker |= 0x2;
    } else {
	pstPrivInfo->u8Marker &= 0xfd;
    }
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)

#if (1 == MT_VDEC_CC_FROM_IMAGE)
    for (u32Index = 0; u32Index < 4; u32Index++) {
	pstUsrData = ((pstImage->p_usrdat))[u32Index];
	if (MT_NULL != pstUsrData) {
//	    if (pstUsrData->data_size > 5) {
	    if (pstUsrData->data_size > 0) {
		pstUsrData->PTS = pstImage->PTS;
		u32ID = *((mt_u32 *)pstUsrData->data);
		u8Type = pstUsrData->data[4];
//		if ((VDEC_USERDATA_IDENTIFIER_DVB1 == u32ID) && (VDEC_USERDATA_TYPE_DVB1_CC == u8Type))
		{
		    USRDATA_Put(hHandle, pstUsrData, MT_UNF_VIDEO_USERDATA_DVB1_CC);
		}
	    }
	}
    }
#endif
#endif

//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("[%s](%lu): frm_cnt %u, slot %u\n",__FUNCTION__,gettid(),
			pstFrame->slotInfo.frm_cnt,
			pstFrame->slotInfo.filedInfoTop.slot_idx);
#endif

    return MT_SUCCESS;
}

static mt_s32 VDEC_RecvVPUFrm(mt_handle hHandle, VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = MT_SUCCESS;
    struct list_head *pos;
    VDEC_VPU_FRAME_LIST_NODE_S *pstTarget;
    IMAGE stImage;
    VDEC_CHAN_STATINFO_S *pstStatInfo = &pstChan->stStatInfo;
    ENTER_FUNCTION;
    memset(&stImage, 0, sizeof(IMAGE));
    pstStatInfo->u32VdecRcvFrameTry++;
    if ((&(pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList)) == pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList.next) {
	return MT_FAILURE;
    }

    BUFMNG_SpinLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
    pos = pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList.next;
    pstTarget = list_entry(pos, VDEC_VPU_FRAME_LIST_NODE_S, node);
    if (MT_NULL != pstTarget) {
	memcpy(pstFrame, &(pstTarget->stPortOutFrame), sizeof(MT_DRV_VIDEO_FRAME_S));
	list_del_init(pos);
	BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
	MT_VFREE_BUFMNG(pstTarget);
    } else {
	BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
	s32Ret = MT_FAILURE;
    }
    //    BUFMNG_SaveYuv(0, pstFrame,MT_UNF_VCODEC_TYPE_H264);
    stImage.SrcPts = (mt_u64)pstFrame->u32Pts;
    stImage.PTS = (mt_u64)pstFrame->u32Pts;
    stImage.image_width = pstFrame->u32Width;
    stImage.image_height = pstFrame->u32Height;
    PTSREC_CalcStamp(hHandle, pstChan->stCurCfg.enType, &stImage);
    pstFrame->u32Pts = (mt_u32)stImage.PTS;
    pstFrame->u32FrameRate = (stImage.frame_rate / 1024) * 1000 + (stImage.frame_rate % 1024);
    memcpy(&(pstChan->stLastFrm), pstFrame, sizeof(MT_DRV_VIDEO_FRAME_S));
    if (MT_SUCCESS == s32Ret) {
	pstStatInfo->u32VdecRcvFrameOK++;
    }
    return s32Ret;
}
static mt_s32 VDEC_RlsVPUFrm(VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Index;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32Reserve);
    VDEC_CHAN_STATINFO_S *pstStatInfo = &pstChan->stStatInfo;
    ENTER_FUNCTION;
    pstStatInfo->u32VdecRlsFrameTry++;
    BUFMNG_SpinLockIRQ(&(pstChan->stVPUParam.stVPURlsParam.stVPURlsFrameListLock));
    s32Index = pstChan->stVPUParam.stVPURlsParam.s32AvailableNum;
    pstChan->stVPUParam.stVPURlsParam.RlsFrameIDArray[s32Index] = pstPrivInfo->image_id;
    pstChan->stVPUParam.stVPURlsParam.s32AvailableNum++;
    BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPURlsParam.stVPURlsFrameListLock));
    pstStatInfo->u32VdecRlsFrameOK++;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetCrcBuf(mt_handle hHandle, MT_DRV_VDEC_CRC_BUF_S *pstCrc)
{
	VDEC_CHANNEL_S *pstChan = MT_NULL;
	mt_s32 s32Ret;

	ENTER_FUNCTION;

	if (MT_NULL == pstCrc)
	{
		MT_ERR_VDEC("Bad param!\n");
		return MT_ERR_VDEC_INVALID_PARA;
	}

	hHandle = hHandle & 0xff;
	if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
	{
		return MT_ERR_VDEC_INVALID_PARA;
	}

	/* Lock */
	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
	if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
		return MT_FAILURE;
	}

	/* Check and get pstChan pointer */
	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_WARN_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
	}

	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

	pstCrc->u32Size = pstChan->stCrcBuf.size;
	pstCrc->u32PhyAddr = pstChan->stCrcBuf.startPhyAddr;
#if 0
	{
		mt_u32 *ptr = pstChan->stCrcBuf.u32StartVirAddr;
		printk("\r\n crc0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5] );
	}

	printk("\r\n ~~~~~~%s, %d, u32PhyAddr:0x%08x, size:%d", __FUNCTION__, __LINE__, pstCrc->u32PhyAddr, pstCrc->u32Size);
#endif

//	MLOGI("%s: hHandle 0x%08x, PhyAddr 0x%08x, SZ %u\n",__FUNCTION__,
//		hHandle, pstCrc->u32PhyAddr, pstCrc->u32Size);

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_SUCCESS;
}

static mt_s32 VDEC_GetLcevcMem(mt_handle hHandle,mmz_buffer_s *pstmmz)
{
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	mt_s32 s32Ret;

	if (MT_NULL == pstmmz)
	{
		MT_ERR_VDEC("Bad param!\n");
		return MT_ERR_VDEC_INVALID_PARA;
	}
	hHandle = hHandle & 0xff;
	if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
	{
		return MT_ERR_VDEC_INVALID_PARA;
	}
	/* Lock */
	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
	if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
		return MT_FAILURE;
	}

	/* Check and get pstChan pointer */
	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_WARN_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
	}
	memset(pstmmz, 0, sizeof(mmz_buffer_s));
	pstmmz->size = g_stLecvcDataMMZ[hHandle].size;
	pstmmz->startVirAddr = g_stLecvcDataMMZ[hHandle].startVirAddr;
	pstmmz->startPhyAddr = g_stLecvcDataMMZ[hHandle].startPhyAddr;

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_SUCCESS;
#else
	return MT_FAILURE;
#endif
}

mt_s32 MT_DRV_VDEC_GetLcevcData(mt_handle hHandle, MT_VDEC_LCEVC_DATA_S *pdata)
{
#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	VDEC_CHANNEL_S *pstChan = MT_NULL;
	mt_s32 s32Ret;

	if (MT_NULL == pdata)
	{
		MT_ERR_VDEC("Bad param!\n");
		return MT_ERR_VDEC_INVALID_PARA;
	}

	hHandle = hHandle & 0xff;
	if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
	{
		return MT_ERR_VDEC_INVALID_PARA;
	}

	/* Lock */
	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
	if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
		return MT_FAILURE;
	}

	/* Check and get pstChan pointer */
	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_WARN_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
	}

	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
	
	s32Ret = Lcevc_data_pop(hHandle, gPLcevc_pipe[hHandle], pdata);
	//jace

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

	if(0 == s32Ret)
		return MT_ERR_VDEC_RECEIVE_FAILED;
	else
		return MT_SUCCESS;
#else
		return MT_FAILURE;
#endif
}

mt_s32 MT_DRV_VDEC_GetFrmBuf(mt_handle hHandle, MT_DRV_VDEC_FRAME_BUF_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_s32 as8TmpBuf[16];
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == pstFrm) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    memset(as8TmpBuf, 0, sizeof(as8TmpBuf));

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Get from VFMW */
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_USRDEC_FRAME, as8TmpBuf);
    if (VDEC_OK != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d GET_USRDEC_FRAME err!\n", pstChan->hChan);
	return MT_FAILURE;
    }

    pstFrm->u32PhyAddr = (phys_addr_t)(as8TmpBuf[0]);
    pstFrm->u32Size = (mt_u32)(as8TmpBuf[1]);
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetVOBufClear(mt_handle hHandle, MT_BOOL buffClearFlag)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    //mt_handle hVpss = MT_NULL;	//not used

    ENTER_FUNCTION;
//    MLOGD("%s: Handle = %x\n", __FUNCTION__,hHandle);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
    	return MT_ERR_VDEC_INVALID_PARA;
    }

//20181027: optimize 1st frame's EVNT_IMG_SIZE_CHANGE,
//			for FAKE VPSS, no need clean vpss's all frames.
#ifdef CONFIG_MT_HAS_HW_VPASS
	//Added for EVNT_IMG_SIZE_CHANGE begin:
	//clean all vpss frames
	//Note: NO Lock, for VDEC_Chan_CleanAllVpssFrame -> VDEC_Chan_CleanVpssFrame -> VDEC_Chan_RecvVpssFrmBuf
	//      will Lock!
	hVpss = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss;
	if (hVpss != MT_INVALID_HANDLE)
	{
		MLOGI("%s: EVNT_IMG_SIZE_CHANGE, clean vpss.\n",__FUNCTION__);
		VDEC_Chan_CleanAllVpssFrame(hHandle, hVpss);

		//reset vpss
		if (s_stVdecDrv.pVpssFunc != NULL && s_stVdecDrv.pVpssFunc->pfnVpssSendCommand != NULL)
		{
			MLOGI("%s: EVNT_IMG_SIZE_CHANGE, reset vpss.\n",__FUNCTION__);
			s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(hVpss, MT_DRV_VPSS_USER_COMMAND_RESET, MT_NULL);
			if (VDEC_OK != s32Ret)
			{
				MT_WARN_VDEC("Chan %x Reset Vpss err!\n", hHandle);
			}
		}
	}
	else
	{
		MLOGW("%s: invalid vpss handle!\n",__FUNCTION__);
	}
	//Added end.
#endif

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Get from VFMW */
    if (pstChan->hChan != MT_INVALID_HANDLE)
    {
	    MLOGD("%s: call KERN_VDEC_Control(VDEC_CID_SET_VDEC_RESOLUTION_DONE)\n",__FUNCTION__);
	    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_VDEC_RESOLUTION_DONE, (void *)&buffClearFlag);
	    if (VDEC_OK != s32Ret) {
	    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    	MT_WARN_VDEC("Chan %d GET_USRDEC_FRAME err!\n", pstChan->hChan);
	    	return MT_FAILURE;
	    }

	    MLOGI("%s: KERN_VDEC_Control(VDEC_CID_SET_VDEC_RESOLUTION_DONE) return %d\n",__FUNCTION__,s32Ret);
	    pstChan->bNoGetImage = MT_FALSE;
    }
    else
    {
	    MLOGW("%s: VFMW Chan Handle invalid!\n",__FUNCTION__);
    }

    /* Reset & dump ES Stat */
    vdec_dump_es_stat(MT_INVALID_HANDLE, NULL);
    vdec_dump_es_stat(hHandle, pstChan);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_PutFrmBuf(mt_handle hHandle, MT_DRV_VDEC_USR_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    USRDEC_FRAME_DESC_S stFrameDesc;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == pstFrm) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Convert color format */
    switch (pstFrm->enFormat) {
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_422:
	stFrameDesc.enFmt = COLOR_FMT_422_2x1;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_420:
	stFrameDesc.enFmt = COLOR_FMT_420;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_400:
	stFrameDesc.enFmt = COLOR_FMT_400;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_411:
	stFrameDesc.enFmt = COLOR_FMT_411;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2:
	stFrameDesc.enFmt = COLOR_FMT_422_1x2;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_444:
	stFrameDesc.enFmt = COLOR_FMT_444;
	stFrameDesc.s32IsSemiPlanar = MT_TRUE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_400:
	stFrameDesc.enFmt = COLOR_FMT_400;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_411:
	stFrameDesc.enFmt = COLOR_FMT_411;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_420:
	stFrameDesc.enFmt = COLOR_FMT_420;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_422_1X2:
	stFrameDesc.enFmt = COLOR_FMT_422_1x2;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_422_2X1:
	stFrameDesc.enFmt = COLOR_FMT_422_2x1;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_444:
	stFrameDesc.enFmt = COLOR_FMT_444;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PLANAR_410:
	stFrameDesc.enFmt = COLOR_FMT_410;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    case MT_UNF_FORMAT_YUV_PACKAGE_UYVY:
    case MT_UNF_FORMAT_YUV_PACKAGE_YUYV:
    case MT_UNF_FORMAT_YUV_PACKAGE_YVYU:
    default:
	stFrameDesc.enFmt = COLOR_FMT_BUTT;
	stFrameDesc.s32IsSemiPlanar = MT_FALSE;
	break;
    }

    stFrameDesc.Pts = pstFrm->u32Pts;
    stFrameDesc.s32YWidth = pstFrm->s32YWidth;
    stFrameDesc.s32YHeight = pstFrm->s32YHeight;
    stFrameDesc.s32LumaPhyAddr = pstFrm->s32LumaPhyAddr;
    stFrameDesc.s32LumaStride = pstFrm->s32LumaStride;
    stFrameDesc.s32CbPhyAddr = pstFrm->s32CbPhyAddr;
    stFrameDesc.s32CrPhyAddr = pstFrm->s32CrPhyAddr;
    stFrameDesc.s32ChromStride = pstFrm->s32ChromStride;
    stFrameDesc.s32ChromCrStride = pstFrm->s32ChromCrStride;
    stFrameDesc.s32IsFrameValid = pstFrm->bFrameValid;

    /* Last frame is the end frame */
    if (pstFrm->bEndOfStream) {
	pstChan->u32EndFrmFlag = 3;
    }

    /* Put */
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_PUT_USRDEC_FRAME, &stFrameDesc);
    if (VDEC_OK != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d PUT_USRDEC_FRAME err!\n", pstChan->hChan);
	return MT_FAILURE;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_RecvFrmBuf(mt_handle hHandle, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    //MT_DRV_VIDEO_FRAME_S* pstLastFrm = MT_NULL;

    //ENTER_FUNCTION;
    if ((MT_NULL == pstFrm) || (MT_INVALID_HANDLE == hHandle)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    //memset(pstFrm, 0, sizeof(MT_DRV_VIDEO_FRAME_S));	//!!! not here

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_WARN_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (VDEC_CHAN_STATE_RUN != pstChan->enCurState) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d isn't runnig!\n", hHandle);
	return MT_FAILURE;
    }

    /*VPSS Read a frame from VDEC */
    pstChan->stStatInfo.u32UserAcqFrameTry++;
    if (pstChan->bVPU) {
	s32Ret = VDEC_RecvVPUFrm(hHandle, pstChan, pstFrm);
    } else {
	s32Ret = VDEC_RecvFrm(hHandle, pstChan, pstFrm);
	pstFrm->hVdecHandle = hHandle;
    }
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }
    pstChan->stStatInfo.u32UserAcqFrameOK++;

    //pstChan->stLastDispFrameInfo = *pstFrm;

//Display call VDEC_Chan_VORlsFrame to release the freeze frame buffer,
//not call VDEC_Chan_VORlsFreezeFrame anymore!
//so no need check here.
#if 0
	//compare check with freeze frame buffer
	if (pstChan->stFreezeFrame.filedInfoTop.addrLuma != 0
		&& pstChan->stFreezeFrame.filedInfoTop.addrLuma ==
			pstFrm->slotInfo.filedInfoTop.addrLuma)
	{
		MLOGE("%s: Error, Received frame's Luma addr(0x%x) equals to Freeze Frame buffer!\n",
			__FUNCTION__,
			pstFrm->slotInfo.filedInfoTop.addrLuma);
		//BUG();
		WARN_ON(1);
	}
#endif

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    /* save yuv data */
    BUFMNG_SaveYuv(hHandle, pstFrm, pstChan->stCurCfg.enType);

//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	VTRACE("[%s](%lu): frm_cnt %u, slot %u\n",__FUNCTION__,gettid(),
			pstFrm->slotInfo.frm_cnt,
			pstFrm->slotInfo.filedInfoTop.slot_idx);
#endif

    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_RlsFrmBuf(mt_handle hHandle, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    //ENTER_FUNCTION;
    if ((MT_NULL == pstFrm) || (MT_INVALID_HANDLE == hHandle)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    pstChan->stStatInfo.u32UserRlsFrameTry++;
    if (pstChan->bVPU) {
	s32Ret = VDEC_RlsVPUFrm(pstChan, pstFrm);
    } else {
	//printk("ADD MT_DRV_VDEC_RlsFrmBuf \n");
	s32Ret = VDEC_RlsFrm(pstChan, pstFrm);
    }
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("VDEC_RlsFrm err!\n");
	return MT_FAILURE;
    }

    pstChan->stStatInfo.u32UserRlsFrameOK++;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

/**
 * VPSS 通过VDEC DRV从VFMW Read Image进行后处理.
 * 此时VPSS应该另外再分配一帧的Buffer保存Read的Image进行后处理.
 * 但Montage的硬件架构,并不支持这种VPSS处理机制.
 * VPSS仅仅做缓冲.
 */
static mt_s32 VDEC_Chan_VpssRecvFrmBuf(VPSS_HANDLE hVpss, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_handle hVdec;

    //ENTER_FUNCTION;
    if (MT_NULL == pstFrm) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    s32Ret = VDEC_FindVdecHandleByVpssHandle(hVpss, &hVdec);
    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }
    s32Ret = MT_DRV_VDEC_RecvFrmBuf(hVdec, pstFrm);

//Trace Image Flow
#ifdef CONFIG_MT_DEBUG_V_IMG_FLOW
	if (s32Ret == MT_SUCCESS)
	{
		VTRACE("[%s](%lu): frm_cnt %u, slot %u, pts %llu, eos %u\n",__FUNCTION__,gettid(),
				pstFrm->slotInfo.frm_cnt,
				pstFrm->slotInfo.filedInfoTop.slot_idx,
				pstFrm->slotInfo.pts,
				pstFrm->slotInfo.end_of_stream_flag);
	}
#endif

    return s32Ret;
}

/**
 * VPSS 通过VDEC DRV从VFMW Read Image之后再调用该接口通知VDEC DRV和VFMW可以释放该Image.
 * 但Montage的硬件架构,从VFMW解码Image之后,到VDEC DRV/VPSS,再到Display,都是不能释放该帧Image的.
 * 直到Display显示完该解码帧之后,Display通知VDEC DRV/VPSS/VFMW才能释放该帧数据.
 */
static mt_s32 VDEC_Chan_VpssRlsFrmBuf(VPSS_HANDLE hVpss, MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_handle hVdec;
    //ENTER_FUNCTION;
    if (MT_NULL == pstFrm) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    s32Ret = VDEC_FindVdecHandleByVpssHandle(hVpss, &hVdec);
    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }
    s32Ret = MT_DRV_VDEC_RlsFrmBuf(hVdec, pstFrm);
    return s32Ret;
}
mt_s32 MT_DRV_VDEC_RlsFrmBufWithoutHandle(MT_DRV_VIDEO_FRAME_S *pstFrm)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Yaddr = 0;

    ENTER_FUNCTION;
    if (MT_NULL == pstFrm) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Get handle value */
    u32Yaddr = pstFrm->stBufAddr[0].u32PhyAddr_Y;
    //s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_CHAN_ID_BY_MEM, &u32Yaddr);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("VMFW GET_CHAN_ID_BY_MEM err!\n");
	return MT_FAILURE;
    }

    return MT_DRV_VDEC_RlsFrmBuf(u32Yaddr & 0xff, pstFrm);
}

#if 0
static inline mt_void VDEC_YUVFormat_UNF2VFMW(MT_UNF_VIDEO_FORMAT_E enUNF, YUV_FORMAT_E* penVFMW)
{
    switch (enUNF)
    {
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_422:
        *penVFMW = SPYCbCr422_2X1;
        break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_420:
        *penVFMW = SPYCbCr420;
        break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_400:
        *penVFMW = SPYCbCr400;
        break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_411:
        *penVFMW = SPYCbCr411;
        break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_422_1X2:
        *penVFMW = SPYCbCr422_1X2;
        break;
    case MT_UNF_FORMAT_YUV_SEMIPLANAR_444:
        *penVFMW = SPYCbCr444;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_400:
        *penVFMW = PLNYCbCr400;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_411:
        *penVFMW = PLNYCbCr411;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_420:
        *penVFMW = PLNYCbCr420;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_422_1X2:
        *penVFMW = PLNYCbCr422_1X2;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_422_2X1:
        *penVFMW = PLNYCbCr422_2X1;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_444:
        *penVFMW = PLNYCbCr444;
        break;
    case MT_UNF_FORMAT_YUV_PLANAR_410:
        *penVFMW = PLNYCbCr410;
        break;
    case MT_UNF_FORMAT_YUV_PACKAGE_UYVY:
    case MT_UNF_FORMAT_YUV_PACKAGE_YUYV:
    case MT_UNF_FORMAT_YUV_PACKAGE_YVYU:
    default:
        *penVFMW = SPYCbCr420;
        break;
    }
}
#endif

static inline mt_void VDEC_YUVFormat_UNF2VFMW(MT_DRV_PIX_FORMAT_E enVideo, YUV_FORMAT_E *penVFMW)
{
    switch (enVideo) {
    case MT_DRV_PIX_FMT_NV08:
	*penVFMW = SPYCbCr400;
	break;
    case MT_DRV_PIX_FMT_NV12_411:
	*penVFMW = SPYCbCr411;
	break;
    case MT_DRV_PIX_FMT_NV16:
	*penVFMW = SPYCbCr422_1X2;
	break;
    case MT_DRV_PIX_FMT_NV16_2X1:
	*penVFMW = SPYCbCr422_2X1;
	break;
    case MT_DRV_PIX_FMT_NV24:
	*penVFMW = SPYCbCr444;
	break;
    case MT_DRV_PIX_FMT_YUV400:
	*penVFMW = PLNYCbCr400;
	break;
    case MT_DRV_PIX_FMT_YUV411:
	*penVFMW = PLNYCbCr411;
	break;
    case MT_DRV_PIX_FMT_YUV420p:
	*penVFMW = PLNYCbCr420;
	break;
    case MT_DRV_PIX_FMT_YUV422_1X2:
	*penVFMW = PLNYCbCr422_1X2;
	break;
    case MT_DRV_PIX_FMT_YUV422_2X1:
	*penVFMW = PLNYCbCr422_2X1;
	break;
    case MT_DRV_PIX_FMT_YUV_444:
	*penVFMW = PLNYCbCr444;
	break;
    case MT_DRV_PIX_FMT_YUV410p:
	*penVFMW = PLNYCbCr410;
	break;
    case MT_DRV_PIX_FMT_NV21:
    default:
	*penVFMW = SPYCbCr420;
	break;
    }
}

mt_s32 MT_DRV_VDEC_BlockToLine(mt_s32 hHandle, MT_DRV_VDEC_BTL_S *pstBTL)
{
/* Only hx3712 support this interface now. */
#if 1
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    struct
    {
	IMAGE astImage[2];
	mt_u32 u32Size;
    } stBTLParam;
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = MT_NULL;

    if ((MT_NULL == pstBTL) || (MT_NULL == pstBTL->pstInFrame) ||
        (MT_NULL == pstBTL->pstOutFrame) || (MT_INVALID_HANDLE == hHandle)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)pstBTL->pstInFrame->u32Priv;

    /* Set input image data */
    stBTLParam.astImage[0].top_luma_phy_addr = pstBTL->pstInFrame->stBufAddr[0].u32PhyAddr_Y;
    stBTLParam.astImage[0].top_chrom_phy_addr = pstBTL->pstInFrame->stBufAddr[0].u32PhyAddr_C;
    stBTLParam.astImage[0].luma_phy_addr = pstBTL->pstInFrame->stBufAddr[0].u32PhyAddr_Y;
    stBTLParam.astImage[0].chrom_phy_addr = pstBTL->pstInFrame->stBufAddr[0].u32PhyAddr_C;
    stBTLParam.astImage[0].image_width = pstBTL->pstInFrame->u32Width;
    stBTLParam.astImage[0].image_height = pstBTL->pstInFrame->u32Height;
    stBTLParam.astImage[0].image_stride = pstBTL->pstInFrame->stBufAddr[0].u32Stride_Y;
    //stBTLParam.astImage[0].image_id = pstBTL->pstInFrame->u32FrameIndex;
    stBTLParam.astImage[0].BTLInfo.u32Is1D = pstPrivInfo->stBTLInfo.u32Is1D;
    stBTLParam.astImage[0].BTLInfo.u32IsCompress = pstPrivInfo->stBTLInfo.u32IsCompress;
    stBTLParam.astImage[0].BTLInfo.u32HeadStride = pstPrivInfo->stCompressInfo.u32HeadStride;
    stBTLParam.astImage[0].BTLInfo.u32HeadOffset = pstPrivInfo->stCompressInfo.u32HeadOffset;
    stBTLParam.astImage[0].BTLInfo.u32YHeadAddr = pstPrivInfo->stCompressInfo.u32YHeadAddr;
    stBTLParam.astImage[0].BTLInfo.u32CHeadAddr = pstPrivInfo->stCompressInfo.u32CHeadAddr;
    stBTLParam.astImage[0].BTLInfo.u32CrStride = pstBTL->pstInFrame->stBufAddr[0].u32Stride_Cr;
    stBTLParam.astImage[0].BTLInfo.u32CrAddr = pstBTL->pstInFrame->stBufAddr[0].u32PhyAddr_Cr;
    //stBTLParam.astImage[0].BTLInfo.u32Reversed = pstBTL->pstInFrame->u32Circumrotate;
    VDEC_YUVFormat_UNF2VFMW(pstBTL->pstInFrame->ePixFormat, &(stBTLParam.astImage[0].BTLInfo.YUVFormat));

    /* Set output image data */
    stBTLParam.astImage[1].luma_2d_phy_addr = pstBTL->u32PhyAddr;

    /* Set buffer size */
    stBTLParam.u32Size = pstBTL->u32Size;

    pstChan->stBTL.pstFrame = pstBTL->pstOutFrame;
    atomic_inc(&pstChan->stBTL.atmWorking);

    /* VDEC_CID_FRAME_BTL */
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_FRAME_BTL, &stBTLParam);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("Chan %d VDEC_CID_FRAME_BTL err!\n", pstChan->hChan);
	goto err;
    }

    /* Wait for over */
    if (0 == wait_event_interruptible_hrtimeout(pstChan->stBTL.stWaitQue,
                                              (atomic_read(&pstChan->stBTL.atmWorking) == 0), ms_to_ktime(pstBTL->u32TimeOutMs))) {
	MT_ERR_VDEC("Chan %d BlockToLine time out!\n", pstChan->hChan);
	goto err;
    }

    pstChan->stBTL.pstFrame = MT_NULL;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;

err:
    pstChan->stBTL.pstFrame = MT_NULL;
    atomic_set(&pstChan->stBTL.atmWorking, 0);
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_FAILURE;
#else
    return MT_FAILURE;
#endif
}

static VDEC_CHAN_CAP_LEVEL_E VDEC_CapLevelUnfToFmw(MT_UNF_AVPLAY_OPEN_OPT_S *pstVdecCapParam)
{
    if (MT_UNF_VCODEC_DEC_TYPE_ISINGLE == pstVdecCapParam->enDecType) {
	return CAP_LEVEL_SINGLE_IFRAME_FHD;
    } else if (MT_UNF_VCODEC_DEC_TYPE_NORMAL == pstVdecCapParam->enDecType) {
	if (MT_UNF_VCODEC_PRTCL_LEVEL_H264 == pstVdecCapParam->enProtocolLevel) {
	    switch (pstVdecCapParam->enCapLevel) {
	    case MT_UNF_VCODEC_CAP_LEVEL_QCIF:
		return CAP_LEVEL_H264_QCIF;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_CIF:
		return CAP_LEVEL_H264_CIF;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_D1:
		return CAP_LEVEL_H264_D1;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_720P:
		return CAP_LEVEL_H264_720;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_FULLHD:
		return CAP_LEVEL_H264_FHD;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x800:
		return CAP_LEVEL_1280x800;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_800x1280:
		return CAP_LEVEL_800x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1488x1280:
		return CAP_LEVEL_1488x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x1488:
		return CAP_LEVEL_1280x1488;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x1280:
		return CAP_LEVEL_2160x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x2160:
		return CAP_LEVEL_1280x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x2160:
		return CAP_LEVEL_2160x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x2160:
		return CAP_LEVEL_4096x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x4096:
		return CAP_LEVEL_2160x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x4096:
		return CAP_LEVEL_4096x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_8192x4096:
		return CAP_LEVEL_8192x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x8192:
		return CAP_LEVEL_4096x8192;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_8192x8192:
		return CAP_LEVEL_8192x8192;
		break;
	    default:
		return CAP_LEVEL_H264_FHD;
		break;
	    }
	} else if (MT_UNF_VCODEC_PRTCL_LEVEL_MVC == pstVdecCapParam->enProtocolLevel) {
	    return CAP_LEVEL_MVC_FHD;
	} else {
	    switch (pstVdecCapParam->enCapLevel) {
	    case MT_UNF_VCODEC_CAP_LEVEL_QCIF:
		return CAP_LEVEL_MPEG_QCIF;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_CIF:
		return CAP_LEVEL_MPEG_CIF;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_D1:
		return CAP_LEVEL_MPEG_D1;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_720P:
		return CAP_LEVEL_MPEG_720;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_FULLHD:
		return CAP_LEVEL_MPEG_FHD;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x800:
		return CAP_LEVEL_1280x800;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_800x1280:
		return CAP_LEVEL_800x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1488x1280:
		return CAP_LEVEL_1488x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x1488:
		return CAP_LEVEL_1280x1488;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x1280:
		return CAP_LEVEL_2160x1280;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_1280x2160:
		return CAP_LEVEL_1280x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x2160:
		return CAP_LEVEL_2160x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x2160:
		return CAP_LEVEL_4096x2160;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_2160x4096:
		return CAP_LEVEL_2160x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x4096:
		return CAP_LEVEL_4096x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_8192x4096:
		return CAP_LEVEL_8192x4096;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_4096x8192:
		return CAP_LEVEL_4096x8192;
		break;
	    case MT_UNF_VCODEC_CAP_LEVEL_8192x8192:
		return CAP_LEVEL_8192x8192;
		break;
	    default:
		return CAP_LEVEL_MPEG_FHD;
		break;
	    }
	}
    } else {
	return CAP_LEVEL_BUTT;
    }
}

static mt_s32 VDEC_Chan_InitParam(mt_handle hHandle, MT_UNF_AVPLAY_OPEN_OPT_S *pstCapParam)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_CHAN_CAP_LEVEL_E enCapToFmw;
    mt_sys_mem_config_s stMemConfig = { 0 };

    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    /* check input parameters */
    if (MT_NULL == pstCapParam) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }
    /* Allocate resource */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	pstChan = MT_VMALLOC_VDEC(sizeof(VDEC_CHANNEL_S));
	if (MT_NULL == pstChan) {
	    MT_ERR_VDEC("No memory\n");
	    goto err0;
	} else {
	    /* Initialize the channel attributes */
	    memset(pstChan, 0, sizeof(VDEC_CHANNEL_S));
	    s_stVdecDrv.astChanEntity[hHandle].pstChan = pstChan;
	}
    } else {
	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    }

    pstChan->hVdec = hHandle;
    pstChan->hChan = MT_INVALID_HANDLE;
    pstChan->hStrmBuf = MT_INVALID_HANDLE;
    pstChan->u32StrmBufSize = 0;
    pstChan->hDmxVidChn = MT_INVALID_HANDLE;
    pstChan->u32DmxBufSize = 0;
    pstChan->bNormChange = MT_FALSE;
    pstChan->stNormChangeParam.enNewFormat = MT_UNF_ENC_FMT_BUTT;
    pstChan->stNormChangeParam.u32ImageWidth = 0;
    pstChan->stNormChangeParam.u32ImageHeight = 0;
    pstChan->stNormChangeParam.u32FrameRate = 0;
    pstChan->stNormChangeParam.bProgressive = MT_FALSE;
    pstChan->stIFrame.st2dBuf.size = 0;
    pstChan->bNewFrame = MT_FALSE;
    pstChan->bFramePackingChange = MT_FALSE;
    pstChan->bNewSeq = MT_FALSE;
    pstChan->bNewUserData = MT_FALSE;
    pstChan->bIFrameErr = MT_FALSE;
    pstChan->bUnSupportStream = MT_FALSE;
    pstChan->pstUsrData = MT_NULL;
    pstChan->stFrameRateParam.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
    pstChan->stFrameRateParam.stSetFrmRate.u32fpsInteger = 25;
    pstChan->stFrameRateParam.stSetFrmRate.u32fpsDecimal = 0;
    pstChan->bProcRegister = MT_FALSE;
    pstChan->bVPUProcRegister = MT_FALSE;
    pstChan->u8ResolutionChange = 0;
    pstChan->u32DiscontinueCount = 0;
    pstChan->s32Speed = 1024;
    pstChan->bVPU = MT_FALSE;
    pstChan->u32FrameCnt = 0;
    atomic_set(&pstChan->stBTL.atmWorking, 0);
    init_waitqueue_head(&pstChan->stBTL.stWaitQue);
    pstChan->stBTL.pstFrame = MT_NULL;

    pstChan->bIsIFrameDec = MT_FALSE;
    pstChan->u32ErrRatio = 0;
    /* Get proper buffer size */
    enCapToFmw = VDEC_CapLevelUnfToFmw(pstCapParam);
    MLOGD("%s: FW CAP %d\n",__FUNCTION__,enCapToFmw);
    pstChan->enCapToFmw = enCapToFmw;
    pstChan->stOption.eAdapterType = ADAPTER_TYPE_VDEC;
    pstChan->stOption.Purpose = PURPOSE_DECODE;
    pstChan->stOption.MemAllocMode = MODE_PART_BY_SDK;
    pstChan->stOption.u32DynamicFrameStoreAllocEn = ((MaskCtrlWord & 0x1) == 1) ? 0 : s_stVdecDrv.astChanEntity[hHandle].u32DynamicFsEn; //l00273086 使能动态分配功能
    pstChan->stOption.s32ExtraFrameStoreNum = PLUS_FS_NUM + 1;                                                                           //l00273086 PLUS_FS_NUM + 1:流畅播放防止阻塞
    pstChan->stOption.s32DelayTime = 0;                                                                                                  //l00273086 动态分配帧存时，若分配不到内存vfmw等待的时间
    pstChan->stOption.u32SelfAdaptionDFS = 1;                                                                                            //l00273086 自适应帧存
    pstChan->stOption.u32CfgFrameNum = 0;                                                                                                //l00273086 配置帧数
    pstChan->stOption.u32NeedMMZ = 1;                                                                                                    //l00273086是否使用MMZ做为预分配不足时的补充
    pstChan->stOption.u32MaxMemUse = -1;                                                                                                 //l00273086最大可用帧存
    pstChan->bNeedAlloc = MT_FALSE;                                                                                                      //l00273086 记录该通道是否已经分配过帧存了

    pstChan->stOption.s32MaxSliceNum = 136;
    pstChan->stOption.s32MaxSpsNum = 32;
    pstChan->stOption.s32MaxPpsNum = 256;
    pstChan->stOption.s32SupportBFrame = 1;
    pstChan->stOption.s32SupportH264 = 1;
    pstChan->stOption.s32ReRangeEn = 1; /* Support rerange frame buffer when definition change */
    pstChan->stOption.s32SlotWidth = 0;
    pstChan->stOption.s32SlotHeight = 0;

    //pstChan->stOption.s32SCDBufSize = g_stSCDMMZ.u32Size - MT_VDEC_SCD_EXT_MEM;

    /*calculate max width and height*/
    switch (enCapToFmw) {
    case CAP_LEVEL_MPEG_QCIF:
    case CAP_LEVEL_H264_QCIF:
	pstChan->stOption.s32MaxWidth = 176;
	pstChan->stOption.s32MaxHeight = 144;
	break;
    case CAP_LEVEL_MPEG_CIF:
    case CAP_LEVEL_H264_CIF:
	pstChan->stOption.s32MaxWidth = 352;
	pstChan->stOption.s32MaxHeight = 288;
	break;
    case CAP_LEVEL_MPEG_D1:
    case CAP_LEVEL_H264_D1:
	pstChan->stOption.s32MaxWidth = 720;
	pstChan->stOption.s32MaxHeight = 576;
	break;
    case CAP_LEVEL_MPEG_720:
    case CAP_LEVEL_H264_720:
	pstChan->stOption.s32MaxWidth = 1280;
	pstChan->stOption.s32MaxHeight = 736;
	break;
    case CAP_LEVEL_MPEG_FHD:
    case CAP_LEVEL_H264_FHD:
    case CAP_LEVEL_MVC_FHD:
	pstChan->stOption.s32MaxWidth = 1920;
	pstChan->stOption.s32MaxHeight = 1088;
	break;
    case CAP_LEVEL_H264_BYDHD:
	pstChan->stOption.s32MaxWidth = 5632;
	pstChan->stOption.s32MaxHeight = 4224;
	break;
    case CAP_LEVEL_SINGLE_IFRAME_FHD:
	pstChan->stOption.s32MaxWidth = 1920;
	pstChan->stOption.s32MaxHeight = 1088;
	break;
    case CAP_LEVEL_1280x800:
	pstChan->stOption.s32MaxWidth = 1280;
	pstChan->stOption.s32MaxHeight = 800;
	break;
    case CAP_LEVEL_800x1280:
	pstChan->stOption.s32MaxWidth = 800;
	pstChan->stOption.s32MaxHeight = 1280;
	break;
    case CAP_LEVEL_1488x1280:
	pstChan->stOption.s32MaxWidth = 1488;
	pstChan->stOption.s32MaxHeight = 1280;
	break;
    case CAP_LEVEL_1280x1488:
	pstChan->stOption.s32MaxWidth = 1280;
	pstChan->stOption.s32MaxHeight = 1488;
	break;
    case CAP_LEVEL_2160x1280:
	pstChan->stOption.s32MaxWidth = 2160;
	pstChan->stOption.s32MaxHeight = 1280;
	break;
    case CAP_LEVEL_1280x2160:
	pstChan->stOption.s32MaxWidth = 1280;
	pstChan->stOption.s32MaxHeight = 2176;
	break;
    case CAP_LEVEL_2160x2160:
	pstChan->stOption.s32MaxWidth = 2160;
	pstChan->stOption.s32MaxHeight = 2176;
	break;
    case CAP_LEVEL_4096x2160:
	pstChan->stOption.s32MaxWidth = 4096;
	pstChan->stOption.s32MaxHeight = 2304;
	break;
    case CAP_LEVEL_2160x4096:
	pstChan->stOption.s32MaxWidth = 2304;
	pstChan->stOption.s32MaxHeight = 4096;
	break;
    case CAP_LEVEL_4096x4096:
	pstChan->stOption.s32MaxWidth = 4096;
	pstChan->stOption.s32MaxHeight = 4096;
	break;
    case CAP_LEVEL_8192x4096:
	pstChan->stOption.s32MaxWidth = 8192;
	pstChan->stOption.s32MaxHeight = 4096;
	break;
    case CAP_LEVEL_4096x8192:
	pstChan->stOption.s32MaxWidth = 4096;
	pstChan->stOption.s32MaxHeight = 8192;
	break;
    case CAP_LEVEL_8192x8192:
	pstChan->stOption.s32MaxWidth = 8192;
	pstChan->stOption.s32MaxHeight = 8192;
	break;
    default:
	pstChan->stOption.s32MaxWidth = 1920;
	pstChan->stOption.s32MaxHeight = 1088;
	break;
    }

    /* calculate refrence frame number and display frame number */
    s32Ret = mt_drv_sys_getmemconfig(&stMemConfig);
    if (MT_SUCCESS != s32Ret) {
	stMemConfig.u32TotalSize = 1024;
    }

    MLOGI("%s: Total Memory Size = %d \n", __FUNCTION__,stMemConfig.u32TotalSize);

    switch (stMemConfig.u32TotalSize) {
    case 512:
	pstChan->stOption.s32MaxRefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_512;
	pstChan->stOption.s32DisplayFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_512 - MT_VDEC_BUFFER_FRAME;
	RefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_512;
	DispFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_512;
	pstChan->stOption.s32ExtraFrameStoreNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_512;
	pstChan->stOption.u32MaxMemUse = MT_VIDEO_MAX_VDH_BUF_IN_512 * 1024 * 1024;
	break;

    case 2048:
	pstChan->stOption.s32MaxRefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_2048;
	pstChan->stOption.s32DisplayFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_2048 - MT_VDEC_BUFFER_FRAME;
	RefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_2048;
	DispFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_2048;
	pstChan->stOption.s32ExtraFrameStoreNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_2048;
	pstChan->stOption.u32MaxMemUse = MT_VIDEO_MAX_VDH_BUF_IN_2048 * 1024 * 1024;
	break;

    case 1024:
    default:
	pstChan->stOption.s32MaxRefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_1024;
	pstChan->stOption.s32DisplayFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_1024 - MT_VDEC_BUFFER_FRAME;
	RefFrameNum = MT_VIDEO_MAX_REF_FRAME_NUM_IN_1024;
	DispFrameNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_1024;
	pstChan->stOption.s32ExtraFrameStoreNum = MT_VIDEO_MAX_DISP_FRAME_NUM_IN_1024;
	pstChan->stOption.u32MaxMemUse = MT_VIDEO_MAX_VDH_BUF_IN_1024 * 1024 * 1024;
    }

    /* 1: Enable 1D to 2D, the data got by VOU is 2D; 0: Disable */
    /* Can't support 1D->2D switching. Because 2D must allocate memory when VDEC_CID_CREATE_CHAN. */
    pstChan->stOption.s32Btl1Dt2DEnable = En2d; /* Default 1D */
    /* 1: Enable DbDr info calculation, used by DNR in VOU; 0: Disable */
    pstChan->stOption.s32BtlDbdrEnable = 1; /* DNR Enable */
#if (1 == MT_VDEC_HD_SIMPLE)
    pstChan->stOption.s32TreeFsEnable = 0;
#else
#if 0
    if (pstChan->stOption.s32MaxRefFrameNum +
        pstChan->stOption.s32DisplayFrameNum +
        MT_VDEC_BUFFER_FRAME
          >= MT_VDEC_TREEBUFFER_MIN)
    {
        pstChan->stOption.s32TreeFsEnable = 1;      /* Support tree buffer */
    }
    else
    {
        pstChan->stOption.s32TreeFsEnable = 0;
    }
#else
    pstChan->stOption.s32TreeFsEnable = 0; /* Support tree buffer */
#endif
#endif

    /* Register proc file of this chan */
    if (MT_FALSE == pstChan->bProcRegister) {
	s32Ret = VDEC_RegChanProc(hHandle);
	if (MT_SUCCESS == s32Ret) {
	    pstChan->bProcRegister = MT_TRUE;
	} else {
	    pstChan->bProcRegister = MT_FALSE;
	}
    }
    /* Set default config */
    pstChan->stCurCfg = s_stVdecDrv.stDefCfg;

    pstChan->enDisplayNorm = MT_UNF_ENC_FMT_BUTT;
    pstChan->stLastFrm.eFrmType = MT_DRV_FT_BUTT;
    pstChan->u32ValidPtsFlag = 0;

    /* Alloc pts recover channel */
    PTSREC_Alloc(hHandle);

    /* Update information of VDEC device */
    // s_stVdecDrv.astChanEntity[hHandle].pstChan = pstChan;
    pstChan->stUserCfgCap = *pstCapParam;
    MLOGD("%s: DecType=%d, CapLevel=%d, Protocol=%d\n",__FUNCTION__,
    	pstCapParam->enDecType,
    	pstCapParam->enCapLevel,
    	pstCapParam->enProtocolLevel);

    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVdec = hHandle;

#ifdef TEST_VDEC_SAVEFILE
    VDEC_Dbg_OpenSaveFile(hHandle);
#endif

 //   MT_INFO_VDEC("Chan %d alloc OK!\n", hHandle);
//    MLOGI("%s: Chan %d alloc OK!\n", __FUNCTION__,hHandle);
    dump_channel(pstChan);

    LEAVE_FUNCTION;
    return MT_SUCCESS;

err0:
    return MT_FAILURE;
}

static mt_s32 VDEC_Chan_AllocHandle(mt_handle *phHandle, struct file *pstFile)
{
    mt_u32 i, j;
    MT_UNF_AVPLAY_OPEN_OPT_S stCapParam;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_NULL == phHandle) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    memset(&stCapParam, 0, sizeof(MT_UNF_AVPLAY_OPEN_OPT_S));

    /* Check ready flag */
    if (s_stVdecDrv.bReady != MT_TRUE) {
	MT_ERR_VDEC("Need open first!\n");
	return MT_ERR_VDEC_NOT_OPEN;
    }

    /* Lock */
    if (DOWN_SEM(&s_stVdecDrv.stSem, __LINE__)) {
	MT_ERR_VDEC("Global lock err!\n");
	return MT_FAILURE;
    }

    /* Check channel number */
    if ((s_stVdecDrv.u32ChanNum >= MT_VDEC_MAX_INSTANCE_NEW) || (s_stVdecDrv.u32ChanNum >= s_stVdecDrv.stVdecCap.s32MaxChanNum)) {
	MT_ERR_VDEC("Too many chans:%d!\n", s_stVdecDrv.u32ChanNum);
	goto err0;
    }

    /* Allocate new channel */
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (MT_FALSE == s_stVdecDrv.astChanEntity[i].bUsed) {
	    s_stVdecDrv.astChanEntity[i].bUsed = MT_TRUE;
	    s_stVdecDrv.astChanEntity[i].pstChan = MT_NULL;
	    s_stVdecDrv.astChanEntity[i].eCallBack = 0;
	    s_stVdecDrv.astChanEntity[i].DmxHdlCallBack = 0;
	    s_stVdecDrv.astChanEntity[i].u32File = (mt_u32)MT_NULL;
	    atomic_set(&s_stVdecDrv.astChanEntity[i].atmUseCnt, 0);
	    atomic_set(&s_stVdecDrv.astChanEntity[i].atmRlsFlag, 0);
	    break;
	}
    }

    if (i >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Too many chans!\n");
	goto err0;
    }

    s_stVdecDrv.astChanEntity[i].pstChan = MT_NULL;
    s_stVdecDrv.astChanEntity[i].u32File = (ulong)pstFile;

    s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec = i;
    s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss = MT_INVALID_HANDLE;
    s_stVdecDrv.astChanEntity[i].stVpssChan.stControlInfo.u32BackwardOptimizeFlag = MT_FALSE;
    s_stVdecDrv.astChanEntity[i].stVpssChan.stControlInfo.u32DispOptimizeFlag = MT_FALSE;
    s_stVdecDrv.astChanEntity[i].stVpssChan.s32GetFirstIFrameFlag = MT_FALSE;
    s_stVdecDrv.astChanEntity[i].stVpssChan.eLastFrameFormat = VDEC_FRAME_BUTT;
    //s_stVdecDrv.astChanEntity[i].stVpssChan.s32LastFrameGopNum = -1;
    s_stVdecDrv.astChanEntity[i].stVpssChan.s32Speed = 1024;
    s_stVdecDrv.astChanEntity[i].stVpssChan.s32ImageDistance = 0;
    for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].hPort = MT_INVALID_HANDLE;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].bEnable = MT_FALSE;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].enPortType = VDEC_PORT_TYPE_BUTT;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].s32PortTmpListPos = 0;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].s32GetFirstVpssFrameFlag = MT_FALSE;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].s32RecvNewFrame = MT_FALSE;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].s32PortLastFrameGopNum = -1;
	s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].u32LastFrameIndex = -1;
	INIT_LIST_HEAD(&s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].stPortList.stVdecPortFrameList);
	VDEC_InitSpinLock(&s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].stPortList.stPortFrameListLock);
    }

#if 1
	if (MT_NULL == s_stVdecDrv.astChanEntity[i].pstChan) {
		pstChan = MT_VMALLOC_VDEC(sizeof(VDEC_CHANNEL_S));
		if (MT_NULL == pstChan) {
			MT_ERR_VDEC("No memory\n");
			s_stVdecDrv.astChanEntity[i].bUsed = MT_FALSE;
			goto err0;
		} else {
			/* Initialize the channel attributes */
			memset(pstChan, 0, sizeof(VDEC_CHANNEL_S));
			pstChan->hVdec = MT_INVALID_HANDLE;
			pstChan->hChan = MT_INVALID_HANDLE;
		}
	}
	s_stVdecDrv.astChanEntity[i].pstChan = pstChan;
#endif

    s_stVdecDrv.u32ChanNum++;
    *phHandle = (MT_ID_VDEC << 16) | i;
//    MLOGD("%s: hHandle 0x%x, ChanNum %u\n",__FUNCTION__,*phHandle,s_stVdecDrv.u32ChanNum);
    up(&s_stVdecDrv.stSem);

    LEAVE_FUNCTION;
    return MT_SUCCESS;

err0:
    up(&s_stVdecDrv.stSem);
    return MT_FAILURE;
}

static mt_s32 VDEC_Chan_FreeHandle(mt_handle hHandle)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    /* Clear global parameter */
    s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
	if (s32Ret != 0)
	{
		MLOGE("%s: down_killable failed! return %d.\n",__FUNCTION__,s32Ret);
	}

    if (s_stVdecDrv.u32ChanNum > 0) {
	s_stVdecDrv.u32ChanNum--;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    s_stVdecDrv.astChanEntity[hHandle].bUsed = MT_FALSE;
    s_stVdecDrv.astChanEntity[hHandle].pstChan = MT_NULL;
    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVdec = MT_INVALID_HANDLE;
    s_stVdecDrv.astChanEntity[hHandle].u32File = (mt_u32)MT_NULL;
    MT_VFREE_VDEC(pstChan);
    up(&s_stVdecDrv.stSem);

    LEAVE_FUNCTION;
    return MT_SUCCESS;
}
static mt_s32 VDEC_Chan_GetMemSize(VDEC_CHANNEL_S *pstChan)
{
    mt_s32 s32Ret;
    mt_s32 s32DispSlotCnt;
    MT_BOOL bDynamicResSupport;
    MT_BOOL bHevcSupport;
    MT_BOOL bHevc10BitSupport;
    MT_BOOL bLossyCompress;
    MT_BOOL bVp9Support;
	MT_BOOL b4KSupport;
#if CONFIG_SUPPORT_VDEC_DVIEW
	MT_BOOL	  down_scaler_enable;
	mt_s32	  ds_width;
	mt_s32	  ds_height;
#endif
    ulong as8TmpBuf[32];

    ENTER_FUNCTION;
    memset(as8TmpBuf, 0, sizeof(as8TmpBuf));

    /* Hevc计算内存公式不一样，在GetMemSize调用前，通过传递一个HEVC的能力集类型来识别 */
    if (MT_UNF_VCODEC_TYPE_HEVC == pstChan->stCurCfg.enType) {
      switch (pstChan->enCapToFmw) {
      case CAP_LEVEL_MPEG_QCIF:
      case CAP_LEVEL_H264_QCIF:
      case CAP_LEVEL_HEVC_QCIF:
          pstChan->stOption.s32MaxWidth = 176;
          pstChan->stOption.s32MaxHeight = 144;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_QCIF;
          break;
      case CAP_LEVEL_MPEG_CIF:
      case CAP_LEVEL_H264_CIF:
      case CAP_LEVEL_HEVC_CIF:
          pstChan->stOption.s32MaxWidth = 352;
          pstChan->stOption.s32MaxHeight = 288;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_CIF;
          break;
      case CAP_LEVEL_MPEG_D1:
      case CAP_LEVEL_H264_D1:
      case CAP_LEVEL_HEVC_D1:
          pstChan->stOption.s32MaxWidth = 720;
          pstChan->stOption.s32MaxHeight = 576;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_D1;
          break;
      case CAP_LEVEL_MPEG_720:
      case CAP_LEVEL_H264_720:
      case CAP_LEVEL_HEVC_720:
          pstChan->stOption.s32MaxWidth = 1280;
          pstChan->stOption.s32MaxHeight = 736;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_720;
          break;
      case CAP_LEVEL_MPEG_FHD:
      case CAP_LEVEL_H264_FHD:
      case CAP_LEVEL_MVC_FHD:
      case CAP_LEVEL_HEVC_FHD:
          pstChan->stOption.s32MaxWidth = 1920;
          pstChan->stOption.s32MaxHeight = 1088;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_FHD;
          break;
      default:
          pstChan->stOption.s32MaxWidth = 4096;
          pstChan->stOption.s32MaxHeight = 2304;
          pstChan->enCapToFmw = CAP_LEVEL_HEVC_UHD;
          break;
      }
    }

    as8TmpBuf[0] = (ulong)pstChan->enCapToFmw;
    as8TmpBuf[1] = (ulong) & pstChan->stOption;
#if	VFMW_PARAMETER_SUPPORT_4K
	s32DispSlotCnt = 3;
#else
    s32DispSlotCnt = VFMW_PARAMETER_DISP_SLOT_COUNT;
#endif
    as8TmpBuf[2] = (ulong) & s32DispSlotCnt;

    bDynamicResSupport = VFMW_PARAMETER_DYNAMIC_RES;
    as8TmpBuf[3] = (ulong) & bDynamicResSupport;

    bHevcSupport = VFMW_PARAMETER_HEVC;
    as8TmpBuf[4] = (ulong) & bHevcSupport;

    bHevc10BitSupport = VFMW_PARAMETER_HEVC_10BIT;
    as8TmpBuf[5] = (ulong) & bHevc10BitSupport;

    bLossyCompress = VFMW_PARAMETER_LOSSY_COMPRESS;
    as8TmpBuf[6] = (ulong) & bLossyCompress;

    bVp9Support = VFMW_PARAMETER_VP9;
    as8TmpBuf[7] = (ulong) & bVp9Support;

#if CONFIG_SUPPORT_VDEC_DVIEW

#if	VFMW_PARAMETER_SUPPORT_4K
	down_scaler_enable	= 1;
	as8TmpBuf[8] = (ulong) & down_scaler_enable;
	ds_width			= 1280 ;
	as8TmpBuf[9] = (ulong) & ds_width;
	ds_height			= 720;
	as8TmpBuf[10] = (ulong) & ds_height;

#else
	down_scaler_enable	= 1;
	as8TmpBuf[8] = (ulong) & down_scaler_enable;
	ds_width			= 640 ;
	as8TmpBuf[9] = (ulong) & ds_width;
	ds_height			= 360;
	as8TmpBuf[10] = (ulong) & ds_height;
#endif
	
#endif

	b4KSupport =VFMW_PARAMETER_SUPPORT_4K;

#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
	if(1 == pstChan->stCurCfg.pip_en)
	{
#ifndef CONFIG_MT_VDEC_4KPIP_SUPPORT
		b4KSupport = 0 ; // VFMW_PARAMETER_SUPPORT_4K;
#if CONFIG_SUPPORT_VDEC_DVIEW
		down_scaler_enable	= 1;
		as8TmpBuf[8] = (ulong) & down_scaler_enable;
		ds_width			= 640 ;
		as8TmpBuf[9] = (ulong) & ds_width;
		ds_height			= 360;
		as8TmpBuf[10] = (ulong) & ds_height;
#endif	
#else
    bDynamicResSupport = 0;
    as8TmpBuf[3] = (ulong) & bDynamicResSupport;
	
	bVp9Support = 0;
	as8TmpBuf[7] = (ulong) & bVp9Support;
#endif

	}

#endif
	
    as8TmpBuf[11] = (ulong) & b4KSupport;

    MLOGI("%s: DispSlotCnt=%d\n",__FUNCTION__,s32DispSlotCnt);
    MLOGI("%s: DynamicResSupport=%d\n",__FUNCTION__,bDynamicResSupport);
    MLOGI("%s: HevcSupport=%d\n",__FUNCTION__,bHevcSupport);
    MLOGI("%s: Hevc10BitSupport=%d\n",__FUNCTION__,bHevc10BitSupport);
    MLOGI("%s: LossyCompress=%d\n",__FUNCTION__,bLossyCompress);
    MLOGI("%s: Vp9Support=%d\n",__FUNCTION__,bVp9Support);

    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_CHAN_DETAIL_MEMSIZE_WITH_OPTION, as8TmpBuf);
    if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("VDEC_CID_GET_CHAN_DETAIL_MEMSIZE_WITH_OPTION failed!\n");
	    return MT_FAILURE;
    }
	

    pstChan->stMemSize = *(DETAIL_MEM_SIZE *)as8TmpBuf;
	if(pstChan->stMemSize.VdhDetailMem> 263*1024*1024)
	{
		pstChan->stMemSize.VdhDetailMem = 263*1024*1024;
	}
	
    //MLOGI("%s: ChanCtxDetailMem=%x, ScdDetailMem=%x, VdhDetailMem=%x\n",__FUNCTION__,
    //       pstChan->stMemSize.ChanCtxDetailMem, pstChan->stMemSize.ScdDetailMem, pstChan->stMemSize.VdhDetailMem);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_void * VDEC_phys_to_virt(phys_addr_t address)
{
	void * va_vir = MT_NULL;
	mt_s32 i = 0;
	for(i=0; i< MT_VDEC_MAX_INSTANCE_NEW; i++)
	{
		if((address >= g_stStaticVDHMMZ[i].startPhyAddr) 
			&& address < (g_stStaticVDHMMZ[i].startPhyAddr + g_stStaticVDHMMZ[i].size))
		{
			va_vir = mmz_va(&g_stStaticVDHMMZ[i], address);
			break;
		}
	}
		
	return va_vir;
}
static mt_s32 VDEC_Chan_Create(mt_handle hHandle)
{
    mt_s32 s32Ret;
    ulong u32VDHSize = 0;
    ulong as8TmpBuf[32] = {0};
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    IMAGE_INTF_S *pstImageIntf = MT_NULL;
    //unused
    //STREAM_INTF_S *pstStreamIntf = MT_NULL;
    ChannelInfo_t ChannelInfo;
    mt_u32 es_channel_id;
	mt_u32 mmz_handle = 0 ;
	unsigned char cName[24] = {0};
    ENTER_FUNCTION;

    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Context memory allocated by VFMW */
    /* CNcomment: 这部分由vfmw自己进行分配，scd和vdh的内存由vdec进行分配*/
    pstChan->stOption.MemDetail.ChanMemCtx.Length = 0;
    pstChan->stOption.MemDetail.ChanMemCtx.PhyAddr = 0;
    pstChan->stOption.MemDetail.ChanMemCtx.VirAddr = MT_NULL;
	pstChan->last_recv_time = 0;

//mzhu
	if(pstChan->stCurCfg.pip_en)
	{
		mmz_handle =  1;
	}
	else
	{
		mmz_handle =  0;
	}

    u32VDHSize = pstChan->stMemSize.VdhDetailMem;
    //#endif
    if (u32VDHSize > 0) {
	pstChan->stMemSize.VdhDetailMem = u32VDHSize;
#if (1 == CFG_VDEC_VDH_STATIC_ALLOCATE)
	/* VES VDH buffer static allocated and never free */
	if (1)
	{
		//if u32VDHSize changed, need re-allocate
		if (g_stStaticVDHMMZ[mmz_handle].startPhyAddr != 0
			&& g_stStaticVDHMMZ[mmz_handle].size != u32VDHSize)
		{
			if (g_bVdecStaticVDHMMZUsed[mmz_handle])
			{
				MT_ERR_VDEC("VDH MMZ size changed(%x -> %x), and in usage!\n",g_stStaticVDHMMZ[mmz_handle].size,u32VDHSize);
				goto errA;
			}

			MLOGD("VDH MMZ size changed(%lx -> %lx)\n",g_stStaticVDHMMZ[mmz_handle].size,u32VDHSize);
			mt_drv_mmz_unmap_and_release(&g_stStaticVDHMMZ[mmz_handle]);
			memset(&g_stStaticVDHMMZ[mmz_handle], 0, sizeof(g_stStaticVDHMMZ[mmz_handle]));
		}

		if (g_stStaticVDHMMZ[mmz_handle].startPhyAddr == 0
			|| g_stStaticVDHMMZ[mmz_handle].startVirAddr == 0
			|| g_stStaticVDHMMZ[mmz_handle].size == 0)
		{
		
			snprintf(cName, sizeof(cName), "VFMW_VDH%02d", (int)mmz_handle);
			s32Ret = mt_drv_mmz_alloc_and_map("VFMW_VDH", cName, u32VDHSize, 0, &g_stStaticVDHMMZ[mmz_handle]);
			if (MT_SUCCESS != s32Ret) {
				MT_ERR_VDEC("Chan %d alloc VDH MMZ err!\n", mmz_handle);
				goto errA;
			}
			else
			{ 
				memset(g_stStaticVDHMMZ[mmz_handle].startVirAddr, 0, g_stStaticVDHMMZ[mmz_handle].size);
			}
		}

		//freed
		//such as case: stop -> free -> start
		if (pstChan->stVDHMMZBuf.startPhyAddr == 0)
		{
			if (!g_bVdecStaticVDHMMZUsed[mmz_handle])
			{
				pstChan->stVDHMMZBuf.size = g_stStaticVDHMMZ[mmz_handle].size;
				pstChan->stVDHMMZBuf.startPhyAddr = g_stStaticVDHMMZ[mmz_handle].startPhyAddr;
				pstChan->stVDHMMZBuf.startVirAddr = g_stStaticVDHMMZ[mmz_handle].startVirAddr;
				g_bVdecStaticVDHMMZUsed[mmz_handle] = MT_TRUE;
			} else {
				MT_ERR_VDEC("VDH MMZ in usage!\n");
				goto errA;
			}
		}
		else
		{
			//not freed yet!
			//such as case: stop -> start
		}
	}
#endif
	
	pstChan->stOption.MemDetail.ChanMemVdh.Length = pstChan->stVDHMMZBuf.size;
	pstChan->stOption.MemDetail.ChanMemVdh.PhyAddr = pstChan->stVDHMMZBuf.startPhyAddr;
	pstChan->stOption.MemDetail.ChanMemVdh.VirAddr = (mt_void *)pstChan->stVDHMMZBuf.startVirAddr;
	MT_INFO_VDEC("%s: VDH Buffer allocate %u!\n", __FUNCTION__, pstChan->stVDHMMZBuf.size);
    }

    //ES only, move to line 6624
    memset(&ChannelInfo, 0, sizeof(ChannelInfo));
    //ChannelInfo.es_buffer.hBuf = ves_buffer_inst[es_channel_id].hBuf;
    //ChannelInfo.es_buffer.pu8UsrVirAddr = ves_buffer_inst[es_channel_id].pu8UsrVirAddr;
    ChannelInfo.insert_pts_mode = PTS_NOT_IN_ES;
    ChannelInfo.h264_low_delay_mode = pstChan->bLowdelay;
    //MLOGD("%s: h264_low_delay_mode=%u\n",__FUNCTION__,ChannelInfo.h264_low_delay_mode);

	if (pstChan->hDmxVidChn != MT_INVALID_HANDLE) {
    	mt_u32 av_sync_flage;
    	mt_u32 esBuffStartAddr;
    	mt_u32 esBuffEndAddr;
    	mt_u32 esBuffLen;
    	mt_u32 dscBuffStartAddr;
    	mt_u32 dscBuffEndAddr;

		//av_sync_flage = dmx_get_avsync_flag();
    	es_channel_id = dmx_get_esbuff_id(pstChan->hDmxVidChn);
    	esBuffStartAddr = dmx_get_esbuff_start_addr(es_channel_id);
    	esBuffEndAddr = dmx_get_esbuff_end_addr(es_channel_id);
    	esBuffLen = esBuffEndAddr + 1 - esBuffStartAddr;
		
		av_sync_flage = dmx_get_avsync_flag(es_channel_id);

      MT_INFO_VDEC("%s: [TS] av_sync_flag = %x, es_channel_id = %x, esBuffStartAddr = %x, esBuffLen = %x\n",
      		__FUNCTION__, av_sync_flage, es_channel_id, esBuffStartAddr, esBuffLen);
      MLOGI("%s: [TS] av_sync_flag=%x, es_channel_id=%x, esBuffStartAddr=0x%x, esBuffLen=0x%x\n",
      		__FUNCTION__, av_sync_flage, es_channel_id, esBuffStartAddr, esBuffLen);

      if (2 == av_sync_flage) {
          ChannelInfo.insert_pts_mode = PTS_IN_PIC_HEADER;
      }

      ChannelInfo.es_buffer.u32PhyAddr = esBuffStartAddr;
      ChannelInfo.es_buffer.u32Size = esBuffLen;
      dscBuffStartAddr = dmx_get_desc_start_addr(es_channel_id);
      dscBuffEndAddr = dmx_get_desc_end_addr(es_channel_id);
 //     ChannelInfo.es_buffer.pu8KnlVirDescAddr = (unsigned char*)phys_to_virt(dscBuffStartAddr);
      ChannelInfo.es_buffer.pu8KnlVirDescAddr = (unsigned char*)dmx_get_desc_start_vaddr(pstChan->hDmxVidChn);
      ChannelInfo.es_buffer.u32KnlVirDescBufSize = dscBuffEndAddr + 1 - dscBuffStartAddr;
      ChannelInfo.es_mode = VES_HARDWARE_POINTER;
      ChannelInfo.es_buffer.u32UseDescInfoFlag = 1;

	  ChannelInfo.es_buffer.chan_id = es_channel_id;

      //MLOGI("%s: [TS] KnlVirAddr=%p, KnlVirDescAddr=%p, DescBuffSize=0x%x, insert_pts_mode=%d\n", __FUNCTION__,
      //  ChannelInfo.es_buffer.pu8KnlVirAddr, ChannelInfo.es_buffer.pu8KnlVirDescAddr, ChannelInfo.es_buffer.u32KnlVirDescBufSize, ChannelInfo.insert_pts_mode);
      MLOGI("%s: [TS] KnlVirDescAddr=%p, DescBuffSize=0x%x, insert_pts_mode=%d\n", __FUNCTION__,
        ChannelInfo.es_buffer.pu8KnlVirDescAddr, ChannelInfo.es_buffer.u32KnlVirDescBufSize, ChannelInfo.insert_pts_mode);
	} else {
      //Move to here:
      //ChannelInfo.es_buffer.hBuf = ves_buffer_inst[es_channel_id].hBuf;
      //ChannelInfo.es_buffer.pu8UsrVirAddr = ves_buffer_inst[es_channel_id].pu8UsrVirAddr;

	  BUFMNG_INST_CONFIG_S bufCfg;
	  if (BUFMNG_Get(pstChan->hStrmBuf, &bufCfg) != MT_SUCCESS)
	  {
	  	MLOGE("BUFMNG_Get failed!\n");
	  	BUG();
	  }

	  es_channel_id = pstChan->hStrmBuf;

      ChannelInfo.es_buffer.u32PhyAddr = bufCfg.u32PhyAddr;
      ChannelInfo.es_buffer.u32Size = bufCfg.u32Size;

      ChannelInfo.es_buffer.u32UseDescInfoFlag = pstChan->stCurCfg.u32UseDescInfoFlag;
      if(ChannelInfo.es_buffer.u32UseDescInfoFlag)
        ChannelInfo.insert_pts_mode = PTS_IN_DESCRIPTOR;

      ChannelInfo.es_buffer.pu8KnlVirDescAddr = (unsigned char*)bufCfg.pu8KnlVirDescAddr;
      ChannelInfo.es_buffer.u32KnlVirDescBufSize = bufCfg.u32KnlVirDescBufSize;

      ChannelInfo.es_mode = VES_SOFTWARE_POINTER;

	  ChannelInfo.es_buffer.chan_id = es_channel_id | 0x20;

//      MLOGI("%s: [ES] hBuf=%x, esBuffStartAddr=0x%x, esBuffLen=0x%x\n", __FUNCTION__,
    //  		pstChan->hStrmBuf, ChannelInfo.es_buffer.u32PhyAddr, ChannelInfo.es_buffer.u32Size);
  //    MLOGI("%s: [ES] UseDescInfoFlag=%x, VirDescAddr=%p, VirDescBufSize=0x%x\n", __FUNCTION__,
     // 		ChannelInfo.es_buffer.u32UseDescInfoFlag, ChannelInfo.es_buffer.pu8KnlVirDescAddr, ChannelInfo.es_buffer.u32KnlVirDescBufSize);
    }

    ChannelInfo.mem_buffer.u32StartPhyAddr = pstChan->stVDHMMZBuf.startPhyAddr;
    ChannelInfo.mem_buffer.u32StartVirAddr = (ulong)pstChan->stVDHMMZBuf.startVirAddr;
    ChannelInfo.mem_buffer.u32Size = pstChan->stVDHMMZBuf.size;

    ChannelInfo.res_dynamic_enable = VFMW_PARAMETER_DYNAMIC_RES;
    //MLOGD("%s: res_dynamic_enable=%d\n",__FUNCTION__,ChannelInfo.res_dynamic_enable);

	//FIX
    ChannelInfo.dec_type = VDEC_CodecTypeUnfToVfmw(pstChan->stCurCfg.enType);

    ChannelInfo.user_disable_timeout = (unsigned int)pstChan->stCurCfg.bForceDisableTimeout;

	ChannelInfo.support_hevc        = VFMW_PARAMETER_HEVC;
	ChannelInfo.support_hevc_10b    = VFMW_PARAMETER_HEVC_10BIT;
	ChannelInfo.support_lossy       = VFMW_PARAMETER_LOSSY_COMPRESS;
	ChannelInfo.lossy_quality_level = VFMW_PARAMETER_LOSSY_QUALITY;
	ChannelInfo.support_vp9         = VFMW_PARAMETER_VP9;
	ChannelInfo.support_4k			  = VFMW_PARAMETER_SUPPORT_4K;  
	ChannelInfo.hdr10p_enable		  = 1;
	ChannelInfo.hdr_common_info_base_address = 0;

#if CONFIG_SUPPORT_VDEC_DVIEW
#if	VFMW_PARAMETER_SUPPORT_4K
	ChannelInfo.down_scaler_enable	= 1;
	ChannelInfo.ds_width			= 1280;
	ChannelInfo.ds_height			= 720;
#else
	ChannelInfo.down_scaler_enable	= 1;
	ChannelInfo.ds_width			= 640;
	ChannelInfo.ds_height			= 360;
#endif
#endif

#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
		if(1 == pstChan->stCurCfg.pip_en)
		{
#ifndef CONFIG_MT_VDEC_4KPIP_SUPPORT
			ChannelInfo.support_4k = 0;
#if CONFIG_SUPPORT_VDEC_DVIEW
			ChannelInfo.down_scaler_enable	= 1;
			ChannelInfo.ds_width			= 640;
			ChannelInfo.ds_height			= 360;
#endif		
#endif
		}
#endif


	if(pstChan->stCurCfg.u32DebugCrcBufSize > 0)
	{
		if(pstChan->stCrcBuf.startPhyAddr == 0)
		{
			
			s32Ret = mt_drv_mmz_alloc_and_map("CRC_BUF", MMZ_OTHERS, pstChan->stCurCfg.u32DebugCrcBufSize, 0, &pstChan->stCrcBuf);
			if (s32Ret != MT_SUCCESS)
			{
				MLOGE("%s: malloc CRC buffer failed!\n",__FUNCTION__);
			}
		}

		ChannelInfo.crc_buf_addr = (ulong)pstChan->stCrcBuf.startVirAddr;
		ChannelInfo.crc_buf_size = pstChan->stCurCfg.u32DebugCrcBufSize;
	}
	else
	{
		ChannelInfo.crc_buf_addr = 0;
		ChannelInfo.crc_buf_size = 0;
	}

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    s32Ret = mt_drv_mmz_alloc_and_map("CC", MMZ_OTHERS, CFG_VDEC_USERDATA_CC_BUFFER_SIZE, 4096, &g_stUsrDataMMZ[hHandle]);
    ChannelInfo.usrdat_addr = (ulong) g_stUsrDataMMZ[hHandle].startVirAddr;
	memset(g_stUsrDataMMZ[hHandle].startVirAddr, 0, g_stUsrDataMMZ[hHandle].size);
#endif

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
		ChannelInfo.lcevcdat_addr = (ulong) g_stLecvcDataMMZ[hHandle].startVirAddr;
		if(0 != ChannelInfo.lcevcdat_addr)
		{
			memset(g_stLecvcDataMMZ[hHandle].startVirAddr, 0, g_stLecvcDataMMZ[hHandle].size);
			
			gPLcevc_pipe[hHandle] = (Lcevc_SEIpipe * )(g_stLecvcDataMMZ[hHandle].startVirAddr + 640*1024);
			mutex_init(& Lcevc_pipe_lock[hHandle]);
			gPLcevc_pipe[hHandle]->buffer = gPLcevc_pipe[hHandle]->ring;
			gPLcevc_pipe[hHandle]->freesize = LCEVC_RING_SIZE;
			gPLcevc_pipe[hHandle]->end =gPLcevc_pipe[hHandle]->buffer + LCEVC_RING_SIZE;
			gPLcevc_pipe[hHandle]->rp = gPLcevc_pipe[hHandle]->buffer;
			gPLcevc_pipe[hHandle]->wp = gPLcevc_pipe[hHandle]->buffer;
			gPLcevc_pipe[hHandle]->push_cnt = 0;
			gPLcevc_pipe[hHandle]->pop_cnt = 0;
		}
#endif

#ifdef MT_DUMP_LCEVC
	if (NULL == mt_dump_handle)
	{
		 mt_dump_handle = mt_drv_dump_create(E_DUMP_TYPE_FILE, "/mnt/lecvc.data", 0);
		 if (NULL == mt_dump_handle)
		 {
				 printk(KERN_ERR "BUFMNG_Create mt_drv_dump_create error!\n");
				 return -1;
		 }
	}
#endif
    s32Ret = mt_drv_mmz_alloc_and_map("SL_HDR", MMZ_OTHERS, 33 * 4096, 4096, &g_stSlHdrMMZ[hHandle]);
    ChannelInfo.hdr_common_info_base_address = (ulong)g_stSlHdrMMZ[hHandle].startVirAddr;

    as8TmpBuf[0] = (ulong)pstChan->enCapToFmw;
    as8TmpBuf[1] = (ulong) & pstChan->stOption;
    as8TmpBuf[2] = (ulong) & ChannelInfo;

	vdec_dump_chopt(&ChannelInfo);
    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_CREATE_CHAN_WITH_OPTION, as8TmpBuf);
    if (MT_SUCCESS != s32Ret) {
    	MT_ERR_VDEC("VFMW CREATE_CHAN_WITH_OPTION err!\n");
    	goto err2;
    }

    //hw_set_es_buffer_write_point_channel(pstChan->ves_buffer_channel_id, 0);

    /* Record hHandle */
    pstChan->hChan =  hHandle; //*(mt_u32 *)as8TmpBuf;
    pstChan->enCurState = VDEC_CHAN_STATE_STOP;
  //  MT_INFO_VDEC("Create channel success:%d!\n", pstChan->hChan);
//    MLOGI("%s: Create channel(%u) success.\n", __FUNCTION__, pstChan->hChan);

    /* Set interface of read/release stream buffer */
    /*pstStreamIntf = &pstChan->stStrmIntf;
    pstStreamIntf->stream_provider_inst_id = hHandle;
    pstStreamIntf->read_stream = VDEC_Chan_RecvStrmBuf;
    pstStreamIntf->release_stream = VDEC_Chan_RlsStrmBuf;
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_STREAM_INTF, pstStreamIntf);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_VDEC("Chan %d SET_STREAM_INTF err!\n", hHandle);
        goto err3;
    }*/

    /* Get interface of read/release image */
    pstImageIntf = &pstChan->stImageIntf;
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_IMAGE_INTF, pstImageIntf);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Chan %d GET_IMAGE_INTF err!\n", hHandle);
	goto err3;
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;

err3:
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_DESTROY_CHAN_WITH_OPTION, MT_NULL);
    pstChan->hChan = MT_INVALID_HANDLE;

err2:
#if (1 == CFG_VDEC_VDH_STATIC_ALLOCATE)
	if (1)
	{
		memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
		g_bVdecStaticVDHMMZUsed[mmz_handle] = MT_FALSE;
	} else
#endif
    {
		if (1 != pstChan->stOption.u32DynamicFrameStoreAllocEn) //如果使用动态帧存的方案,也就没有mmz的操作l00273086//
		{
		    mt_drv_mmz_unmap_and_release(&pstChan->stVDHMMZBuf);
		    memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
		}
    }

errA:

//not used
//err1:
    //MT_VFREE_VDEC(pstChan);

    return MT_FAILURE;
}

static mt_s32 VDEC_Chan_Destroy(mt_handle hHandle)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    /* Stop channel first */
    MT_DRV_VDEC_ChanStop(hHandle);

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_RLS_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }

    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    if (MT_NULL == pstChan) {
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    /* Destroy VFMW decode channel */
    if (MT_INVALID_HANDLE != pstChan->hChan) {
	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_DESTROY_CHAN_WITH_OPTION, MT_NULL);
	if (VDEC_OK != s32Ret) {
	    MT_ERR_VDEC("Chan %d DESTROY_CHAN err!\n", hHandle);
	}

/* Free vfmw memory */
#if (1 == CFG_VDEC_VDH_STATIC_ALLOCATE)
	if (1)
	{
		mt_u32 mmz_handle = 0;
		if(pstChan->stCurCfg.pip_en)
		{
			mmz_handle =  1;
		}
		else
		{
			mmz_handle =  0;
		}
		memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
		
		g_bVdecStaticVDHMMZUsed[mmz_handle] = MT_FALSE;
	} else
#endif
	{
      MT_INFO_VDEC("XXXX 9999 u32DynamicFrameStoreAllocEn = %d \n", pstChan->stOption.u32DynamicFrameStoreAllocEn);
	    if (1 != pstChan->stOption.u32DynamicFrameStoreAllocEn) //l00273086
	    {
		   mt_drv_mmz_unmap_and_release(&pstChan->stVDHMMZBuf);
		   memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
	    }
	}

    }

    /* Free I frame 2d buffer */
    if (0 != pstChan->stIFrame.st2dBuf.size) {
		mt_drv_mmz_unmap_and_release(&pstChan->stIFrame.st2dBuf);
		pstChan->stIFrame.st2dBuf.size = 0;
    }

	pstChan->hChan = MT_INVALID_HANDLE;

    /* Free CRC buffer */
	if (0 != pstChan->stCrcBuf.size) {
		mt_drv_mmz_unmap_and_release(&pstChan->stCrcBuf);
		pstChan->stCrcBuf.startPhyAddr = 0;
		pstChan->stCrcBuf.size = 0;
	}

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_Free(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    //mt_s32 i,j;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);
    /* Stop channel first */
    s32Ret = MT_DRV_VDEC_ChanStop(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("INFO: VDEC_Chan_Free call MT_DRV_VDEC_ChanStop %ld err !\n", hHandle);
	return MT_FAILURE;
    }
    VDEC_CHAN_RLS_DOWN(&s_stVdecDrv.astChanEntity[hHandle], MT_INVALID_TIME);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("INFO: %ld use too long !\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_RLS_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    if (MT_NULL == pstChan) {
	VDEC_CHAN_RLS_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	return MT_FAILURE;
    }
    /* Remove proc interface */
    if (MT_TRUE == pstChan->bProcRegister) {
	VDEC_UnRegChanProc(hHandle);
	pstChan->bProcRegister = MT_FALSE;
    }

    VDEC_VPU_PTS_Free(hHandle, pstChan->u32VPUhandle);
    /* Free I frame 2d buffer */
    if (0 != pstChan->stIFrame.st2dBuf.size) {
	mt_drv_mmz_unmap_and_release(&pstChan->stIFrame.st2dBuf);
	pstChan->stIFrame.st2dBuf.size = 0;
    }

//Symphony: Free @ Channel Destroy/Free
#if (1 == CFG_VDEC_VDH_STATIC_ALLOCATE)
	if (1)
	{
		mt_u32 mmz_handle = 0;
		if(pstChan->stCurCfg.pip_en)
		{
			mmz_handle =  1;
		}
		else
		{
			mmz_handle = 0;
		}
	
		memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
		g_bVdecStaticVDHMMZUsed[mmz_handle] = MT_FALSE;
	} else
#endif
	{
	    /* Free VDHM MMZ */
	    if (0 != pstChan->stVDHMMZBuf.size) {
	  //  	MLOGI("%s: Free VDH Memory(%x, %x).\n",__FUNCTION__,
	    //		pstChan->stVDHMMZBuf.u32StartPhyAddr,pstChan->stVDHMMZBuf.u32Size);
			mt_drv_mmz_unmap_and_release(&pstChan->stVDHMMZBuf);
			memset(&pstChan->stVDHMMZBuf, 0, sizeof(mmz_buffer_s));
	    }
	}

    /* Free pts recover channel */
    PTSREC_Free(hHandle);

/* Free user data */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_Free(hHandle);
#endif

/* Clear global parameter */
#if 0
    s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
    s_stVdecDrv.astChanEntity[hHandle].pstChan = MT_NULL;
    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVdec = MT_INVALID_HANDLE;
    s_stVdecDrv.astChanEntity[hHandle].u32File = (mt_u32)MT_NULL;
    up(&s_stVdecDrv.stSem);
#endif

    /* Free resource */
    if (pstChan) {
	if (pstChan->pstUsrData) {
	    MT_KFREE_VDEC(pstChan->pstUsrData);
	}
#if 0
        MT_VFREE_VDEC(pstChan);
#endif
    }

#ifdef MT_DUMP_LCEVC
	mt_drv_dump_destroy(mt_dump_handle);
#endif

    VDEC_CHAN_RLS_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    MT_INFO_VDEC("Chan %ld free OK!\n", hHandle);
    MLOGI("%s: Chan %ld free OK!\n", __FUNCTION__, hHandle);
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_ChanStart(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    MEM_DESC_S stChanMemVdh;
    mmz_buffer_s stMMZBuffer;
    //mt_u32 u32VDHSize = 0;
    MT_BOOL u32StartWithOptionFlag = MT_FALSE;
    mt_u32 dcePercent = 0;

    ENTER_FUNCTION;
//    MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);
    memset(&stChanMemVdh, 0, sizeof(MEM_DESC_S));
    memset(&stMMZBuffer, 0, sizeof(mmz_buffer_s));
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	MT_ERR_VDEC("Chan %ld Start err , not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    s32Ret = VDEC_Chan_GetMemSize(pstChan);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Chan %ld GetMemSize err!\n", hHandle);
	return MT_FAILURE;
    }

    if (MT_INVALID_HANDLE == s_stVdecDrv.astChanEntity[hHandle].pstChan->hChan) {
	pstChan->enCurState = VDEC_CHAN_STATE_STOP;
	s32Ret = VDEC_Chan_Create(hHandle);
	if (MT_SUCCESS != s32Ret) {
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_STOP, NULL);
	    MT_ERR_VDEC("Chan %ld Start err ,VDEC_Chan_Create err!\n", hHandle);
	    return MT_FAILURE;
	}
	u32StartWithOptionFlag = MT_FALSE;
    }
	
    s32Ret = VDEC_SetAttr(pstChan);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Chan %ld SetAttr err!\n", hHandle);
	return MT_FAILURE;
    }
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Lock %ld err!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) ||
        (MT_INVALID_HANDLE == s_stVdecDrv.astChanEntity[hHandle].pstChan->hChan)) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	return MT_FAILURE;
    }

    /* Already running, retrun MT_SUCCESS */
    if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_SUCCESS;
    }

    disp_get_dce_percent_wrap(pstChan->hChan, &dcePercent);
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_DCE_PERCENT, &dcePercent);
//    MLOGD("%s: dcePercent %ld\n", __FUNCTION__,dcePercent);

    /* Initialize status information*/
    memset(&(pstChan->stStatInfo), 0, sizeof(VDEC_CHAN_STATINFO_S));
    pstChan->bEndOfStrm = MT_FALSE;
    pstChan->u32EndFrmFlag = 0;
    pstChan->u32LastFrmId = -1;
    pstChan->u32LastFrmTryTimes = 0;
    pstChan->u8ResolutionChange = 0;
    pstChan->bImageSizeChange = MT_FALSE;
    pstChan->u32DiscontinueCount = 0;
    pstChan->s32Speed = 1024;
    //Fix: Bug 125857
	pstChan->bNoGetImage = MT_FALSE;

/* Start VFMW channel */
#if 0
    if(u32StartWithOptionFlag)
    {
        s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_CHAN_WITH_OPTION, &stChanMemVdh);
        if (VDEC_OK != s32Ret)
        {
            VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
            MT_ERR_VDEC("Chan %d VDEC_CID_START_CHAN_WITH_OPTION err!\n", pstChan->hChan);
            return MT_FAILURE;
        }
    }
    else
#endif
    {
	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_CHAN, &stChanMemVdh);
	//MLOGI("%s: ChanMemVdh PhyAddr=%x, VirAddr=%p, Length=%x\n", __FUNCTION__, stChanMemVdh.PhyAddr, stChanMemVdh.VirAddr, stChanMemVdh.Length);
	if (VDEC_OK != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("Chan %ld VDEC_CID_START_CHAN err!\n", pstChan->hChan);
	    return MT_FAILURE;
	}
    }

    /* Start pts recover channel */
    PTSREC_Start(hHandle);

/* Start user data channel */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_Start(hHandle);
#endif
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_START, NULL);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_START err!\n");
	return MT_FAILURE;
    }

	/* start debug */
	vdec_debug_start(hHandle, pstChan);

    /* Save state */
    pstChan->enCurState = VDEC_CHAN_STATE_RUN;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld start OK\n", hHandle);
    MLOGI("%s: Chan %ld start OK\n", __FUNCTION__,hHandle);

    last_sl_hdr_enable = 0;
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_ChanStop(mt_handle hHandle)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
	g_VdecPause = 0;

    ENTER_FUNCTION;
 //   MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    BUFMNG_Reset(hHandle);
    last_sl_hdr_enable = 0;

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %ld lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Already stop, retrun MT_SUCCESS */
    if (pstChan->enCurState == VDEC_CHAN_STATE_STOP) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_SUCCESS;
    }

    if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_STOP, NULL);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_STOP err!\n");
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    return MT_FAILURE;
	}
    }

	/* avoid Resolution Change ACK */
    pstChan->bImageSizeChange = MT_FALSE;

    /* Stop VFMW */
    if (MT_INVALID_HANDLE != pstChan->hChan) {
      //printk("BBBB 0000 MT_DRV_VDEC_ChanStop \n");
	    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_STOP_CHAN, MT_NULL);
	    if (VDEC_OK != s32Ret) {
	    MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_STOP err!\n");
	    }
    }

	
#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
	if (MT_INVALID_HANDLE != pstChan->hDmxVidChn)
	{
		mt_u32 es_channel_id = dmx_get_esbuff_id(pstChan->hDmxVidChn);
		ulong pti_base_addr = mt_get_tsi_base() + 0x6002c;
		mt_u32 val = readl((volatile void*)pti_base_addr);
		val |= (0x0F << (es_channel_id*4));
		if(hHandle != 0)
		{
			writel(val, (volatile u32 *) pti_base_addr);
		}
 		printk(KERN_ERR " vdec stop hHandle = %ld, demux_es_channel=%d, reg(0xbf26002c)=0x%x\n",  (ulong)hHandle, es_channel_id, val);
	}
#endif

    /* Destroy VFMW decode channel */
    if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[hHandle].pstChan->hChan) {
	pstChan->hChan = MT_INVALID_HANDLE;
    }
/* Stop user data channel */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_Stop(hHandle);

	/* free CC buffer when stopped */
	if (g_stUsrDataMMZ[hHandle].startPhyAddr != 0)
	{
   		mt_drv_mmz_unmap_and_release(&g_stUsrDataMMZ[hHandle]);
   		g_stUsrDataMMZ[hHandle].startPhyAddr = 0;
   	}
#endif

	/* free SL_HDR buffer when stopped */
	if (g_stSlHdrMMZ[hHandle].startPhyAddr != 0)
	{
   		mt_drv_mmz_unmap_and_release(&g_stSlHdrMMZ[hHandle]);
   		g_stSlHdrMMZ[hHandle].startPhyAddr = 0;
   	}
	

    /* Free CRC buffer */
	if (0 != pstChan->stCrcBuf.size) {
		mt_drv_mmz_unmap_and_release(&pstChan->stCrcBuf);
		pstChan->stCrcBuf.startPhyAddr = 0;
		pstChan->stCrcBuf.size = 0;
	}

    /* Stop pts recover channel */
    PTSREC_Stop(hHandle);

    /* Save state */
    pstChan->enCurState = VDEC_CHAN_STATE_STOP;
    pstChan->bEndOfStrm = MT_FALSE;
    pstChan->u32EndFrmFlag = 0;
    pstChan->u32LastFrmId = -1;
    pstChan->u32LastFrmTryTimes = 0;
    pstChan->u32ValidPtsFlag = 0;
    pstChan->u8ResolutionChange = 0;
    pstChan->u32DiscontinueCount = 0;
    pstChan->s32Speed = 1024;
    //Fix: Bug 125857
	pstChan->bNoGetImage = MT_FALSE;

    mt_drv_ld_stop_statistics();

	/* stop debug */
	vdec_debug_stop();

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
	if(gPLcevc_pipe[hHandle] != NULL)
	{
		gPLcevc_pipe[hHandle]->buffer = gPLcevc_pipe[hHandle]->ring;
		gPLcevc_pipe[hHandle]->freesize = LCEVC_RING_SIZE;
		gPLcevc_pipe[hHandle]->end =gPLcevc_pipe[hHandle]->buffer + LCEVC_RING_SIZE;
		gPLcevc_pipe[hHandle]->rp = gPLcevc_pipe[hHandle]->buffer;
		gPLcevc_pipe[hHandle]->wp = gPLcevc_pipe[hHandle]->buffer;
		gPLcevc_pipe[hHandle]->push_cnt = 0;
		gPLcevc_pipe[hHandle]->pop_cnt = 0;
		gPLcevc_pipe[0]->m_frame_id = 0;
		gPLcevc_pipe[0]->m_last_display_pts = 0;
	}
#endif

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld stop ret:%x\n", hHandle, s32Ret);
    MLOGI("%s: Chan %ld stop ret:%x\n", __FUNCTION__,hHandle, s32Ret);

	LEAVE_FUNCTION;
    return s32Ret;
}

static mt_s32 VDEC_Chan_Reset(mt_handle hHandle, VDEC_CMD_RESET_S *pArgs)
{
    mt_s32 s32Ret, i;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;

    ENTER_FUNCTION;
    MLOGD("%s: hHandle 0x%lx\n",__FUNCTION__,hHandle);

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %ld lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	    return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);

	/* 20171226: RUN state still could reset */
    /* Must stop channel before reset */
//    if (pstChan->enCurState != VDEC_CHAN_STATE_STOP) {
    if (pstChan->enCurState == VDEC_CHAN_STATE_INVALID) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld state err:%d!\n", hHandle, pstChan->enCurState);
	return MT_FAILURE;
    }

    if (MT_INVALID_HANDLE != pstChan->hStrmBuf) {
	s32Ret = BUFMNG_Reset(pstChan->hStrmBuf);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("Chan %ld strm buf reset err!\n", hHandle);
	}
    }

    /* Reset vfmw */
    if (MT_INVALID_HANDLE != pstChan->hChan) {
        if(pArgs != MT_NULL)
		    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_RESET_CHAN, (VOID *)&pArgs->resetDQ);
        else
            s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_RESET_CHAN, MT_NULL);
		//VFMW ask call VDEC_CID_SET_VDEC_SEEK_DONE after RESET
		s32Ret |= KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_VDEC_SEEK_DONE, MT_NULL);

		if (VDEC_OK != s32Ret) {
		    MT_ERR_VDEC("Chan %ld RESET_CHAN err!\n", pstChan->hChan);
		}
    }

    if (MT_DRV_VDEC_BUF_USER_ALLOC_MANAGE == pstVpssChan->enFrameBuffer) {
	for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	    if ((MT_INVALID_HANDLE != pstVpssChan->stPort[i].hPort) && (MT_TRUE == pstVpssChan->stPort[i].bEnable)) {
		(mt_void) BUFMNG_VPSS_Reset(&(pstVpssChan->stPort[i].stBufVpssInst));
	    }
	}
    }

    /* Reset end frame flag */
    pstChan->bIsLastFrame = MT_FALSE;
    pstChan->bEndOfStrm = MT_FALSE;
    pstChan->u32EndFrmFlag = 0;
    pstChan->u32LastFrmId = -1;
    pstChan->u32LastFrmTryTimes = 0;

    pstChan->u32ValidPtsFlag = 0;
    pstChan->u8ResolutionChange = 0;
    pstChan->u32DiscontinueCount = 0;
    pstChan->s32Speed = 1024;

    pstChan->bUnSupportStream = MT_FALSE;
    pstChan->u32FrameCnt = 0;
    //Fix: Bug 125857
	pstChan->bNoGetImage = MT_FALSE;

    /* Reset pts recover channel */
    PTSREC_Reset(hHandle);

/* Reset user data channel */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_Reset(hHandle);
#endif

    /* Reset & dump ES Stat */
    vdec_dump_es_stat(MT_INVALID_HANDLE, NULL);
    vdec_dump_es_stat(hHandle, pstChan);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld reset OK\n", hHandle);
    MLOGI("%s: Chan %ld reset OK\n", __FUNCTION__, hHandle);

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_Chan_CheckCfg(VDEC_CHANNEL_S *pstChan, MT_UNF_VCODEC_ATTR_S *pstCfgParam)
{
    mt_s32 s32Level = 0;
    MT_UNF_VCODEC_ATTR_S *pstCfg = &pstChan->stCurCfg;

    if (pstCfgParam->enType >= MT_UNF_VCODEC_TYPE_BUTT) {
	MT_ERR_VDEC("Bad type:%d!\n", pstCfgParam->enType);
	return MT_FAILURE;
    }

    if (pstCfgParam->enMode >= MT_UNF_VCODEC_MODE_BUTT) {
	MT_ERR_VDEC("Bad mode:%d!\n", pstCfgParam->enMode);
	return MT_FAILURE;
    }

    if (pstCfgParam->u32ErrCover > 100) {
	MT_ERR_VDEC("Bad err_cover:%d!\n", pstCfgParam->u32ErrCover);
	return MT_FAILURE;
    }

    if (pstCfgParam->u32Priority > MT_UNF_VCODEC_MAX_PRIORITY) {
	MT_ERR_VDEC("Bad priority:%d!\n", pstCfgParam->u32Priority);
	return MT_FAILURE;
    }

    /* enVdecType can't be set dynamically */
    if (pstCfg->enType != pstCfgParam->enType) {
	s32Level |= 1;
    } else if ((MT_UNF_VCODEC_TYPE_VC1 == pstCfg->enType) && ((pstCfg->unExtAttr.stVC1Attr.bAdvancedProfile != pstCfgParam->unExtAttr.stVC1Attr.bAdvancedProfile) || (pstCfg->unExtAttr.stVC1Attr.u32CodecVersion != pstCfgParam->unExtAttr.stVC1Attr.u32CodecVersion))) {
	s32Level |= 1;
    }

    /* priority can't be set dynamically */
    if (pstCfg->u32Priority != pstCfgParam->u32Priority) {
	s32Level |= 1;
    }

    return s32Level;
}

mt_s32 MT_DRV_VDEC_SetChanAttr(mt_handle hHandle, MT_UNF_VCODEC_ATTR_S *pstCfgParam)
{
    mt_s32 s32Ret;
    mt_s32 s32Level;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
   // MLOGD("%s: hHandle 0x%x\n",__FUNCTION__,hHandle);

    /* check input parameters */
    if (MT_NULL == pstCfgParam) {
		MT_ERR_VDEC("Bad param!\n");
		return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %ld lock fail!\n", hHandle);
		return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
		return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Check parameter */
    s32Level = VDEC_Chan_CheckCfg(pstChan, pstCfgParam);
    if (s32Level < 0) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		return MT_FAILURE;
    }

    /* Some parameter can't be set when channel is running */
    if ((pstChan->enCurState != VDEC_CHAN_STATE_STOP) && (s32Level)) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_ERR_VDEC("Chan %ld state err:%d!\n", hHandle, pstChan->enCurState);
		return MT_FAILURE;
    }

    /* Set config */
    pstChan->stCurCfg = *pstCfgParam;
    vdec_dump_chattr(pstCfgParam);

    if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
		//FIX: bug 122203
		//     FW not support set attr while running!
		//s32Ret = VDEC_SetAttr(pstChan);
		/*if (MT_SUCCESS != s32Ret)*/ {
		    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		    MT_ERR_VDEC("Chan %ld SetAttr err(not support SetAttr while running)!\n", hHandle);
		    return MT_FAILURE;
		}
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld SetAttr OK\n", hHandle);
    LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetChanAttr(mt_handle hHandle, MT_UNF_VCODEC_ATTR_S *pstCfgParam)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstCfgParam) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }

    *pstCfgParam = s_stVdecDrv.astChanEntity[hHandle].pstChan->stCurCfg;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %d GetAttr OK\n", hHandle);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetChanStatusInfo(mt_handle hHandle, VDEC_STATUSINFO_S *pstStatus)
{
    mt_s32 i;
    mt_u32 freeSize;
    mt_u32 busySize;
    mt_s32 s32Ret;
    mt_handle hMasterPortHandle = MT_INVALID_HANDLE;
    mt_handle hVirPortHandle = MT_INVALID_HANDLE;
    mt_handle hPortHandle = MT_INVALID_HANDLE;
    MT_BOOL bAllPortCompleteFrm = MT_FALSE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    VDEC_CHAN_STATE_S stChanState;
    BUFMNG_STATUS_S stStatus;
    MT_DRV_VPSS_PORT_BUFLIST_STATE_S stVpssBufListState;
	mt_u32 rd = 0;
	mt_u32 wt = 0;
	mt_u32 tmp = 0;

    //ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstStatus) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }
    //memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	//MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);

    if (MT_INVALID_HANDLE != pstChan->hStrmBuf) {
	s32Ret = BUFMNG_GetStatus(pstChan->hStrmBuf, &stStatus);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("Chan %d get strm buf status err!\n", hHandle);
	    return MT_FAILURE;
	}

	freeSize = stStatus.u32Free;
	busySize = stStatus.u32Used;
	pstStatus->u32BufferSize = pstChan->u32StrmBufSize;
	pstStatus->u32BufferUsed = busySize;
	pstStatus->u32BufferAvailable = freeSize;
	pstStatus->u32BufRptr = stStatus.u32RdPtr;
	pstStatus->u32BufWptr = stStatus.u32WrPtr;
    }

    pstStatus->u32StrmInBps = pstChan->stStatInfo.u32AvrgVdecInBps;
    pstStatus->u32TotalDecFrmNum = pstChan->stStatInfo.u32TotalVdecOutFrame;
    //pstStatus->u32TotalErrFrmNum = pstChan->stStatInfo.u32VdecErrFrame;
    pstStatus->u32TotalErrStrmNum = pstChan->stStatInfo.u32TotalStreamErrNum;

    /* judge if reach end of stream */
    memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));		//move to here


	if((pstChan->enCurState == VDEC_CHAN_STATE_RUN) && (pstChan->hChan >=0)) {
		s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &stChanState);
	}
    pstStatus->u32TotalErrFrmNum = stChanState.dec_error_frame_num;
	pstStatus->u32TotalDecFrmNum = stChanState.total_frame_num;

    pstStatus->u32FrameBufNum = stChanState.wait_disp_frame_num;

    /* Get frame num and stream size vfmw holded */
    pstStatus->u32VfmwFrmNum = stChanState.decoded_1d_frame_num;
    pstStatus->u32VfmwStrmSize = stChanState.buffered_stream_size;

    //pstStatus->stVfmwFrameRate.u32fpsInteger = stChanState.frame_rate / 10;
    //pstStatus->stVfmwFrameRate.u32fpsDecimal = stChanState.frame_rate % 10 * 100;
    pstStatus->stVfmwFrameRate.u32fpsInteger = stChanState.frame_rate;
    pstStatus->stVfmwFrameRate.u32fpsDecimal = 0;
    pstStatus->u32VfmwStrmNum = stChanState.buffered_stream_num;
    //pstStatus->u32VfmwTotalDispFrmNum = stChanState.total_disp_frame_num;
    pstStatus->u32FieldFlag = stChanState.is_field_flg;
    if (pstChan->bEndOfStrm) {
	pstStatus->bEndOfStream = MT_TRUE;
	MLOGD("%s: EOS reached!\n",__FUNCTION__);
    } else {
	pstStatus->bEndOfStream = MT_FALSE;
    }
	
	if (MT_INVALID_HANDLE == pstChan->hDmxVidChn)
	{
		rd = vdec_descriptor_get_read_ptr(pstChan->hStrmBuf);
		wt = vdec_descriptor_get_write_ptr(pstChan->hStrmBuf);
		tmp = wt >= rd ? (wt - rd) : (CFG_VDEC_VES_DESC_BUFF_SIZE - rd + wt);
	}

	pstStatus->u32VideoESFrameNumber = tmp/32 ; // one frame use 32 bit
    /*get vpss status*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(pstVpssChan->hVpss, MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE, &bAllPortCompleteFrm);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Vpss:%d MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE error!\n", pstVpssChan->hVpss);
	return MT_FAILURE;
    }
    pstStatus->bAllPortCompleteFrm = bAllPortCompleteFrm;

    for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	if (VDEC_PORT_TYPE_MASTER == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].enPortType) {
	    hMasterPortHandle = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].hPort;
	    break;
	} else if (VDEC_PORT_TYPE_VIRTUAL == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].enPortType) {
	    hVirPortHandle = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].hPort;
	}
    }
    if (MT_INVALID_HANDLE != hMasterPortHandle) {
	hPortHandle = hMasterPortHandle;
    } else if (MT_INVALID_HANDLE != hVirPortHandle) {
	hPortHandle = hVirPortHandle;
    } else {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("VDEC_GetChanStatusInfo get INVALID Porthandle\n");
	return MT_FAILURE;
    }

    if (MT_INVALID_HANDLE != hPortHandle) {
	if (i == VDEC_MAX_PORT_NUM) {
	    i = VDEC_MAX_PORT_NUM - 1;
	}
	if (MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].bufferType) {
	    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetPortBufListState)(hPortHandle, &stVpssBufListState);
	    if (MT_SUCCESS == s32Ret) {
		pstStatus->u32VfmwTotalDispFrmNum = stVpssBufListState.u32TotalBufNumber;
	    } else {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		return MT_FAILURE;
	    }
	} else if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].bufferType) {
	    pstStatus->u32VfmwTotalDispFrmNum = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].stBufVpssInst.u32AvaiableFrameCnt;
	}
    } else {
	pstStatus->u32VfmwTotalDispFrmNum = 0;
    }
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

extern void AO_TRACK_FillAudioCiTestInfo(MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo);
extern void avsync_fill_avsync_ci_test_info(MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo);
mt_s32 MT_DRV_VDEC_GetCiTestInfo(mt_handle hHandle, MT_UNF_AVPLAY_CI_TEST_INFO_S *pstInfo)
{
	mt_s32 s32Ret;
	VDEC_CHAN_STATE_S stChanState;
	VDEC_CHANNEL_S *pstChan = MT_NULL;
	MT_UNF_AVPLAY_CI_TEST_INFO_S ciInfo;

	memset(&ciInfo, 0, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S));
	/* check input parameters */
	if (MT_NULL == pstInfo) {
		MT_ERR_VDEC("Bad param!\n");
		return MT_ERR_VDEC_INVALID_PARA;
	}

	hHandle = hHandle & 0xff;
	if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
	}
	//memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));

	/* Lock */
	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
	if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
		return MT_FAILURE;
	}

	/* Check and get pstChan pointer */
	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		//MT_ERR_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
	}
	
	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
	if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
		s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &stChanState);
		if (VDEC_OK != s32Ret) {
			MT_FATAL_VDEC("Chan %d GET_CHAN_STATE err\n", pstChan->hChan);
		}else{
			ciInfo.VFrmRate = stChanState.frame_rate;
			ciInfo.VDecErrFrmCnt = stChanState.dec_error_frame_num;
			//fill ci test audio info
			AO_TRACK_FillAudioCiTestInfo(&ciInfo);
			//fill ci test avsync info
			avsync_fill_avsync_ci_test_info(&ciInfo);
			memcpy(pstInfo, &ciInfo, sizeof(MT_UNF_AVPLAY_CI_TEST_INFO_S));
		}
	}

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetChanStreamInfo(mt_handle hHandle, MT_UNF_VCODEC_STREAMINFO_S *pstStreamInfo)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_CHAN_STATE_S stChanState;

    ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstStreamInfo) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }
//	MLOGD("%s: hHandle=0x%08x\n", __FUNCTION__, hHandle);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }
    //memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %ld lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %ld not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    memset(&stChanState, 0, sizeof(VDEC_CHAN_STATE_S));		//move to here
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &stChanState);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("Chan %ld GET_CHAN_STATE err\n", pstChan->hChan);
    }

    pstStreamInfo->enVCodecType = pstChan->stCurCfg.enType;
    pstStreamInfo->enSubStandard = MT_UNF_VIDEO_SUB_STANDARD_UNKNOWN;
    pstStreamInfo->u32SubVersion = 0;
    pstStreamInfo->u32Profile = stChanState.profile;
    pstStreamInfo->u32Level = stChanState.level;
    pstStreamInfo->enDisplayNorm = pstChan->enDisplayNorm;
    pstStreamInfo->bProgressive = (pstChan->stLastFrm.bProgressive);
    pstStreamInfo->u32AspectWidth = pstChan->stLastFrm.u32AspectWidth;
    pstStreamInfo->u32AspectHeight = pstChan->stLastFrm.u32AspectHeight;
    pstStreamInfo->u32bps = stChanState.bit_rate;
    //pstStreamInfo->u32fpsInteger = pstChan->stLastFrm.stFrameRate.u32fpsInteger;
    //pstStreamInfo->u32fpsDecimal = pstChan->stLastFrm.stFrameRate.u32fpsDecimal;
    pstStreamInfo->u32fpsInteger = pstChan->stLastFrm.u32FrameRate / 1000;
    pstStreamInfo->u32fpsDecimal = pstChan->stLastFrm.u32FrameRate % 1000;
    pstStreamInfo->u32Width = pstChan->stLastFrm.u32Width;
    pstStreamInfo->u32Height = pstChan->stLastFrm.u32Height;
    pstStreamInfo->u32DisplayWidth = pstChan->stLastFrm.stDispRect.s32Width;
    pstStreamInfo->u32DisplayHeight = pstChan->stLastFrm.stDispRect.s32Height;
    pstStreamInfo->u32DisplayCenterX = pstChan->stLastFrm.stDispRect.s32Width / 2;
    pstStreamInfo->u32DisplayCenterY = pstChan->stLastFrm.stDispRect.s32Height / 2;
	if((16 == pstChan->stLastFrm.slotInfo.transfer_characteristics) || (18 == pstChan->stLastFrm.slotInfo.transfer_characteristics))
	{
		pstStreamInfo->u32IsHDR = pstChan->stLastFrm.slotInfo.transfer_characteristics;
	}
	else
	{
		pstStreamInfo->u32IsHDR = 0;
	}
	
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

extern mt_s32 CheckWinEvent(void);
extern mt_s32 DRV_DISP_Is_SLHDR_Enable(void);
mt_s32 MT_DRV_VDEC_CheckNewEvent(mt_handle hHandle, VDEC_EVENT_S *pstEvent)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    mt_u8 cur_sl_hdr_enable = 0;    
    //ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstEvent) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

	if(CheckWinEvent())
	{
		pstEvent->bLastFrameShowed = MT_TRUE;
	}
	else
	{
		pstEvent->bLastFrameShowed = MT_FALSE;
	}
    /* Check norm change event */
    if (pstChan->bNormChange) {
	pstChan->bNormChange = MT_FALSE;
	pstEvent->bNormChange = MT_TRUE;
	pstEvent->stNormChangeParam = pstChan->stNormChangeParam;
    } else {
	pstEvent->bNormChange = MT_FALSE;
    }

    /* Check frame packing event */
    if (pstChan->bFramePackingChange) {
	pstChan->bFramePackingChange = MT_FALSE;
	pstEvent->bFramePackingChange = MT_TRUE;
	pstEvent->enFramePackingType = pstChan->enFramePackingType;
    } else {
	pstEvent->bFramePackingChange = MT_FALSE;
	pstEvent->enFramePackingType = MT_UNF_FRAME_PACKING_TYPE_NONE;
    }

    /* Check new frame event */
    if (pstChan->bNewFrame) {
	pstChan->bNewFrame = MT_FALSE;
	pstEvent->bNewFrame = MT_TRUE;
    } else {
	pstEvent->bNewFrame = MT_FALSE;
    }

    /* Check new seq event */
    if (pstChan->bNewSeq) {
	pstChan->bNewSeq = MT_FALSE;
	pstEvent->bNewSeq = MT_TRUE;
    } else {
	pstEvent->bNewSeq = MT_FALSE;
    }

    /* Check new user data event */
    if (pstChan->bNewUserData) {
	pstChan->bNewUserData = MT_FALSE;
	pstEvent->bNewUserData = MT_TRUE;
    } else {
	pstEvent->bNewUserData = MT_FALSE;
    }

    /* Check I frame err event */
    if (pstChan->bIFrameErr) {
	pstChan->bIFrameErr = MT_FALSE;
	pstEvent->bIFrameErr = MT_TRUE;
    } else {
	pstEvent->bIFrameErr = MT_FALSE;
    }

    if(pstChan->bImageSizeChange
    	&& pstChan->enCurState == VDEC_CHAN_STATE_RUN){
		MLOGI("%s: Image Size Changed!\n", __FUNCTION__);
		pstChan->bImageSizeChange = MT_FALSE;
		pstEvent->bImageSizeChange = MT_TRUE;
    }else{
      pstEvent->bImageSizeChange = MT_FALSE;
    }

    if (pstChan->bFirstValidPts) {
	pstChan->bFirstValidPts = MT_FALSE;
	pstEvent->bFirstValidPts = MT_TRUE;
	pstEvent->u32FirstValidPts = pstChan->u32FirstValidPts;
    }

    if (pstChan->bSecondValidPts) {
	pstChan->bSecondValidPts = MT_FALSE;
	pstEvent->bSecondValidPts = MT_TRUE;
	pstEvent->u32SecondValidPts = pstChan->u32SecondValidPts;
    }

    if (pstChan->bUnSupportStream) {
	pstChan->bUnSupportStream = MT_FALSE;
	pstEvent->bUnSupportStream = MT_TRUE;

    } else {
	pstEvent->bUnSupportStream = MT_FALSE;
    }
    if (0 != pstChan->u32ErrRatio) {
	pstEvent->u32ErrRatio = pstChan->u32ErrRatio;
	pstChan->u32ErrRatio = 0;
    } else {
	pstEvent->u32ErrRatio = 0;
    }

    if (0 != pstChan->bIsLastFrame) {
        pstChan->bIsLastFrame = MT_FALSE;
	    pstEvent->bLastFrameDecoded = MT_TRUE;	    
    } else {
	   pstEvent->bLastFrameDecoded = MT_FALSE;
    }

    cur_sl_hdr_enable = DRV_DISP_Is_SLHDR_Enable();
    if(last_sl_hdr_enable != cur_sl_hdr_enable) {
        pstEvent->bSlHdrEnableChange = MT_TRUE;
        pstEvent->bSlHdrEnable = cur_sl_hdr_enable;
        last_sl_hdr_enable = cur_sl_hdr_enable;
    }
    else {
        pstEvent->bSlHdrEnableChange = 0;
    }    
    
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetUsrData(mt_handle hHandle, MT_UNF_VIDEO_USERDATA_S *pstUsrData)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstUsrData) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* If pstUsrData is null, it must be none user data */
    if (MT_NULL == pstChan->pstUsrData) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d none user data!\n", hHandle);
	return MT_FAILURE;
    }

    /* Copy usrdata data */
    s32Ret = copy_to_user(pstUsrData->pu8Buffer,
                          pstChan->pstUsrData->au8Buf[pstChan->pstUsrData->u32ReadID],
                          pstChan->pstUsrData->stAttr[pstChan->pstUsrData->u32ReadID].u32Length);
    if (s32Ret != 0) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d copy_to_user err!\n", hHandle);
	return MT_FAILURE;
    }

    /* copy usrdata attribute */
    //FIXME: what pstUsrData->pu8Buffer?
    memcpy(pstUsrData,
           &pstChan->pstUsrData->stAttr[pstChan->pstUsrData->u32ReadID],
           sizeof(MT_UNF_VIDEO_USERDATA_S));
    pstChan->pstUsrData->u32ReadID = (pstChan->pstUsrData->u32ReadID + 1) % VDEC_UDC_MAX_NUM;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetProgressive(mt_handle hHandle, MT_BOOL pProgressive)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    MT_DRV_VPSS_CFG_S stVpssCfg;
    ENTER_FUNCTION;
    /* check input parameters
    if (MT_NULL == pProgressive)
    {
        MT_ERR_VDEC("Bad param!\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }*/
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hVpss = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss;

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec call Vpss %d VpssGetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    if (pProgressive) {
	stVpssCfg.bProgRevise = MT_FALSE;
    } else {
	stVpssCfg.enProgInfo = MT_DRV_VPSS_PRODETECT_AUTO;
	stVpssCfg.bProgRevise = MT_TRUE;
    }
    MLOGI("%s: Progressive=%d, bProgRevise=%d, enProgInfo=%d\n",__FUNCTION__,pProgressive,
    		stVpssCfg.bProgRevise,stVpssCfg.enProgInfo);

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec call Vpss %d VpssSetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetDPBFullCtrl(mt_handle hHandle, MT_BOOL bDPBFullCtrl)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hHandle) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hHandle &= 0xFF;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay err hvdec:%d too large!\n", hHandle);
	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	MT_ERR_VDEC("Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    pstChan->bDPBFullCtrl = bDPBFullCtrl;
    MLOGI("%s: DPBFullCtrl=%d\n",__FUNCTION__,bDPBFullCtrl);
    return MT_SUCCESS;
}
static mt_u32 VDEC_ConverColorSpace(mt_u32 u32ColorSpace)
{
    switch (u32ColorSpace) {
    case MT_UNF_COLOR_SPACE_UNKNOWN:
	return MT_DRV_CS_UNKNOWN;

    case MT_UNF_COLOR_SPACE_BT601_YUV_LIMITED:
	return MT_DRV_CS_BT601_YUV_LIMITED;

    case MT_UNF_COLOR_SPACE_BT601_YUV_FULL:
	return MT_DRV_CS_BT601_YUV_FULL;

    case MT_UNF_COLOR_SPACE_BT601_RGB_LIMITED:
	return MT_DRV_CS_BT601_RGB_LIMITED;

    case MT_UNF_COLOR_SPACE_BT601_RGB_FULL:
	return MT_DRV_CS_BT601_RGB_FULL;

    case MT_UNF_COLOR_SPACE_NTSC1953:
	return MT_DRV_CS_NTSC1953;

    case MT_UNF_COLOR_SPACE_BT470_SYSTEM_M:
	return MT_DRV_CS_BT470_SYSTEM_M;

    case MT_UNF_COLOR_SPACE_BT470_SYSTEM_BG:
	return MT_DRV_CS_BT470_SYSTEM_BG;

    case MT_UNF_COLOR_SPACE_BT709_YUV_LIMITED:
	return MT_DRV_CS_BT709_YUV_LIMITED;

    case MT_UNF_COLOR_SPACE_BT709_YUV_FULL:
	return MT_DRV_CS_BT709_YUV_FULL;

    case MT_UNF_COLOR_SPACE_BT709_RGB_LIMITED:
	return MT_DRV_CS_BT709_RGB_LIMITED;

    case MT_UNF_COLOR_SPACE_BT709_RGB_FULL:
	return MT_DRV_CS_BT709_RGB_FULL;

    case MT_UNF_COLOR_SPACE_REC709:
	return MT_DRV_CS_REC709;

    case MT_UNF_COLOR_SPACE_SMPT170M:
	return MT_DRV_CS_SMPT170M;

    case MT_UNF_COLOR_SPACE_SMPT240M:
	return MT_DRV_CS_SMPT240M;

    case MT_UNF_COLOR_SPACE_BT878:
	return MT_DRV_CS_BT878;

    case MT_UNF_COLOR_SPACE_XVYCC:
	return MT_DRV_CS_XVYCC;

    case MT_UNF_COLOR_SPACE_JPEG:
	return MT_DRV_CS_JPEG;

    default:
	MT_ERR_VDEC("MT_DRV_VDEC_SetColorSpace Unknow type %d,use default!\n");
	return MT_DRV_CS_DEFAULT;
    }
    return MT_SUCCESS;
}
mt_s32 MT_DRV_VDEC_SetColorSpace(mt_handle hHandle, mt_u32 u32UsrColorSpace)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    MT_DRV_VPSS_CFG_S stVpssCfg;

    ENTER_FUNCTION;
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("MT_DRV_VDEC_SetColorSpace Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hVpss = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss;

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec SetColorSpace call Vpss %d VpssGetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    stVpssCfg.enSrcCS = VDEC_ConverColorSpace(u32UsrColorSpace);
    MLOGI("%s: UsrColorSpace=%d, SrcCS=%d\n",__FUNCTION__,u32UsrColorSpace,stVpssCfg.enSrcCS);
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec SetColorSpace call Vpss %d VpssSetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetDEI(mt_handle hHandle, MT_BOOL bDEI)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    MT_DRV_VPSS_CFG_S stVpssCfg;
    ENTER_FUNCTION;
    /* check input parameters
    if (MT_NULL == pProgressive)
    {
        MT_ERR_VDEC("Bad param!\n");
        return MT_ERR_VDEC_INVALID_PARA;
    }*/
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hVpss = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss;

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec call Vpss %d VpssGetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    if (bDEI) {
	stVpssCfg.enProgInfo = MT_DRV_VPSS_PRODETECT_PROGRESSIVE;
    } else {
	stVpssCfg.enProgInfo = MT_DRV_VPSS_PRODETECT_AUTO;
    }
    MLOGI("%s: DEI=%d, enProgInfo=%d\n",__FUNCTION__,bDEI,stVpssCfg.enProgInfo);

    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("vdec call Vpss %d VpssSetVpssCfg err\n", hVpss);
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}

//not used now
mt_s32 MT_DRV_VDEC_SetTrickMode(mt_handle hHandle, MT_UNF_AVPLAY_TPLAY_OPT_S *pstOpt)
{
    mt_s32 i = 0;
    mt_s32 s32Ret = 0;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    mt_s32 s32Speed;
    mt_handle hVpss;
    MT_DRV_VPSS_CFG_S stVpssCfg;

    MT_BOOL bProgressive = MT_FALSE;
    ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstOpt) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (VDEC_CHAN_STATE_RUN != pstChan->enCurState) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d isn't runnig!\n", hHandle);
	return MT_FAILURE;
    }

    s32Speed = (pstOpt->u32SpeedInteger * 1000 + pstOpt->u32SpeedDecimal) * 1024 / 1000;
    if (MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD == pstOpt->enTplayDirect) {
	s32Speed = -s32Speed;
    }
    MLOGI("%s: SpeedInteger=%d, SpeedDecimal=%d, Direct=%d, Speed=%d\n",__FUNCTION__,pstOpt->u32SpeedInteger,pstOpt->u32SpeedDecimal,
    		pstOpt->enTplayDirect,s32Speed);
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_TRICK_MODE, &s32Speed);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("VFMW Chan %d set trick mode err\n", pstChan->hChan);
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }

    if (s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed != s32Speed) {
	for (i = 0; i < VDEC_MAX_PORT_NUM; i++) {
	    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32RecvNewFrame = MT_FALSE;
	    //s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32PortTmpListPos = 0;
	    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32GetFirstVpssFrameFlag = 0;
	    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].s32PortLastFrameGopNum = -1;
	    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stPort[i].u32LastFrameIndex = -1;
	}
    }
    if (1024 == s32Speed) {
	hVpss = s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss;
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("vdec call Vpss %d VpssGetVpssCfg err\n", hVpss);
	    return MT_FAILURE;
	}
	stVpssCfg.enProgInfo = MT_DRV_VPSS_PRODETECT_AUTO;
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("vdec call Vpss %d VpssSetVpssCfg err\n", hVpss);
	    return MT_FAILURE;
	}
    }
    pstChan->s32Speed = s32Speed;
    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.s32Speed = s32Speed;
#if 1
    if ((s32Speed > 1024) && (MT_TRUE == s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32DispOptimizeFlag)) {
	bProgressive = MT_FALSE;
	s32Ret = MT_DRV_VDEC_SetDEI(hHandle, bProgressive);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("VDEC %d MT_DRV_VDEC_SetDEI err\n", pstChan->hChan);
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    return MT_FAILURE;
	}
    }
#endif
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_SetCtrlInfo(mt_handle hHandle, MT_UNF_AVPLAY_CONTROL_INFO_S *pstCtrlInfo)
{
    mt_s32 s32Ret = 0;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VFMW_CONTROLINFO_S stInfo;
    MT_BOOL bProgressive = MT_FALSE;
    ENTER_FUNCTION;
    /* check input parameters */
    if (MT_NULL == pstCtrlInfo) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_ERR_VDEC_INVALID_PARA;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (VDEC_CHAN_STATE_RUN != pstChan->enCurState) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d isn't runnig!\n", hHandle);
	return MT_FAILURE;
    }

    stInfo.u32IDRFlag = pstCtrlInfo->u32IDRFlag;
    stInfo.u32BFrmRefFlag = pstCtrlInfo->u32BFrmRefFlag;
    stInfo.u32ContinuousFlag = pstCtrlInfo->u32ContinuousFlag;
    stInfo.u32BackwardOptimizeFlag = pstCtrlInfo->u32BackwardOptimizeFlag;
    stInfo.u32DispOptimizeFlag = pstCtrlInfo->u32DispOptimizeFlag;
    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32BackwardOptimizeFlag = pstCtrlInfo->u32BackwardOptimizeFlag;
    s_stVdecDrv.astChanEntity[hHandle].stVpssChan.stControlInfo.u32DispOptimizeFlag = pstCtrlInfo->u32DispOptimizeFlag;
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_CTRL_INFO, &stInfo);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("VFMW Chan %d set ctrl info err\n", pstChan->hChan);
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }
    if (!pstCtrlInfo->u32DispOptimizeFlag) {
	bProgressive = MT_TRUE;
    } else {
	bProgressive = MT_FALSE;
    }

    s32Ret = MT_DRV_VDEC_SetDEI(hHandle, bProgressive);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC %d MT_DRV_VDEC_SetDEI err\n", pstChan->hChan);
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

static mt_s32 VDEC_IFrame_GetStrm(mt_s32 hHandle, STREAM_DATA_S *pstPacket)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_NULL == pstPacket) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }

    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("bad handle %d!\n", hHandle);
	return MT_FAILURE;
    }

    s32Ret = VDEC_CHAN_TRY_USE_DOWN_HELP(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (pstChan->stIFrame.u32ReadTimes >= VDEC_IFRAME_MAX_READTIMES) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	return MT_FAILURE;
    }

    pstPacket->PhyAddr = pstChan->stIFrame.stMMZBuf.startPhyAddr;
    pstPacket->VirAddr = (mt_u8 *)pstChan->stIFrame.stMMZBuf.startVirAddr;
    pstPacket->Length = pstChan->stIFrame.stMMZBuf.size;
    pstPacket->Pts = 0;
    pstPacket->Index = 0;
    pstPacket->is_not_last_packet_flag = 0;
    pstPacket->UserTag = 0;
    pstPacket->discontinue_count = 0;
    pstPacket->is_stream_end_flag = 0;
//    MLOGI("%s: PhyAddr=%llx, VirAddr=%p, Length=%x\n",__FUNCTION__,
//    		pstPacket->PhyAddr,pstPacket->VirAddr,pstPacket->Length);

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    pstChan->stIFrame.u32ReadTimes++;
    return MT_SUCCESS;
}

static mt_s32 VDEC_IFrame_PutStrm(mt_s32 hHandle, STREAM_DATA_S *pstPacket)
{
    ENTER_FUNCTION;
    return MT_SUCCESS;
}

static mt_s32 VDEC_IFrame_SetAttr(VDEC_CHANNEL_S *pstChan, MT_UNF_VCODEC_TYPE_E type)
{
    mt_s32 s32Ret;
    VDEC_CHAN_CFG_S stVdecChanCfg = { 0 };

    ENTER_FUNCTION;
    stVdecChanCfg.eVidStd = VDEC_CodecTypeUnfToFmw(type);
    stVdecChanCfg.s32ChanPriority = 18;
    stVdecChanCfg.s32ChanErrThr = 100;
    stVdecChanCfg.s32DecMode = I_MODE;
    stVdecChanCfg.s32DecOrderOutput = 1;
    
    if ((stVdecChanCfg.eVidStd == STD_VC1) && !pstChan->stCurCfg.unExtAttr.stVC1Attr.bAdvancedProfile)
        stVdecChanCfg.eVidStd = STD_VC1_SP_MAIN;
    
#if 1
    stVdecChanCfg.s32Btl1Dt2DEnable = pstChan->stOption.s32Btl1Dt2DEnable;
    stVdecChanCfg.s32BtlDbdrEnable = pstChan->stOption.s32BtlDbdrEnable;
#endif

	MLOGI("%s: type=%d, Btl1Dt2DEnable=%d, BtlDbdrEnable=%d\n",__FUNCTION__,
			type,stVdecChanCfg.s32Btl1Dt2DEnable,stVdecChanCfg.s32BtlDbdrEnable);

    /* Do not use compress in i frame decode mode*/
    EnVcmp = 0;
    stVdecChanCfg.s32VcmpEn = EnVcmp;
    s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_CFG_CHAN, &stVdecChanCfg);
    if (VDEC_OK != s32Ret) {
	MT_ERR_VDEC("VFMW CFG_CHAN err!\n");
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_SetPause(mt_handle hHandle, MT_BOOL bPauseFlag)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }
	
	g_VdecPause = bPauseFlag;
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
		MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
		return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_ERR_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (VDEC_CHAN_STATE_RUN != pstChan->enCurState) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_WARN_VDEC("Chan %d isn't runnig!\n", hHandle);
		return MT_FAILURE;
    }

	if(bPauseFlag)
	{
		s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_PAUSE_CHAN, NULL);
	}
	else
	{
		s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_RESUME_CHAN, NULL);
	}

    if (VDEC_OK != s32Ret) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("VFMW SetPause err!\n");
	    return MT_FAILURE;
    }

    MLOGI("%s: pause flag %d\n", __FUNCTION__, bPauseFlag);

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_GetVdecCapability(mt_handle hHandle, FW_VDEC_CAPABILITY_INFO_S *pCapability)
{
	mt_s32 s32Ret;
	VDEC_CHANNEL_S *pstChan = MT_NULL;

//	MLOGD("MT_DRV_VDEC_GetVdecCapability hHandle:0x%08x\n", hHandle);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
    	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_DECODING_CAPABILITY, pCapability);
	if (VDEC_OK != s32Ret) {
	  VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	  MT_ERR_VDEC("VFMW GET CAPABILITY err!\n");
	  return MT_FAILURE;
	}

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

	MLOGD("capability: %u, %u, %u, 0x%x\n", pCapability->u32Width, pCapability->u32Height,
									pCapability->u32FrameRate, pCapability->u32VdecCapability);
    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_SetFrameRate(mt_handle hHandle, mt_u32 u32FrameRateInt, mt_u32 u32FrameRateDec)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
	mt_u32  my_rate = 0;
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
    	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Set config */
    pstChan->stCurCfg.u32FrameRateInt = u32FrameRateInt;
    pstChan->stCurCfg.u32FrameRateDec = u32FrameRateDec;
    pstChan->stCurCfg.u32ForceFrameRateFlag = 1;

    if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
#ifndef JQW
	//fix bug 114865: vfmw support frame_rate=0.
	//                frame_rate=0, means frame_rate by stream;
	//                frame_rate>0, means frame_rate force by user.
	//if ((pstChan->stCurCfg.u32FrameRateInt != 0) || (pstChan->stCurCfg.u32FrameRateDec != 0))
	{
      /* Set to VFMW Frame Rate */

	  if(pstChan->stCurCfg.u32FrameRateDec == 97)
	  {
		  my_rate = pstChan->stCurCfg.u32FrameRateInt * 100 + pstChan->stCurCfg.u32FrameRateDec;
	  }
	  else
	  {
		  my_rate = pstChan->stCurCfg.u32FrameRateInt * 100;
	  }
	  s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_FORCE_FRAME_RATE, &(my_rate));
      if (VDEC_OK != s32Ret) {
	  	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
  	    MT_ERR_VDEC("VFMW SET FPS err!\n");
  	    return MT_FAILURE;
      }
	}
#endif

    	/*s32Ret = VDEC_SetAttr(pstChan);
    	if (MT_SUCCESS != s32Ret) {
    	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	    MT_ERR_VDEC("Chan %d SetAttr err!\n", hHandle);
    	    return MT_FAILURE;
    	}*/
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld MT_DRV_VDEC_SetFrameRate %u.%03u OK\n", hHandle, u32FrameRateInt, u32FrameRateDec);
    MLOGI("%s: Ch(%lu) set FPS %u.%03u success.\n",__FUNCTION__,hHandle,u32FrameRateInt, u32FrameRateDec);
    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_SetFrameRateDeclear(mt_handle hHandle, mt_u32 u32FrameRateInt, mt_u32 u32FrameRateDec)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
	mt_u32  my_rate = 0;
    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }
    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
    	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    /* Set config */
    pstChan->stCurCfg.u32FrameRateInt = u32FrameRateInt;
    pstChan->stCurCfg.u32FrameRateDec = u32FrameRateDec;
    pstChan->stCurCfg.u32ForceFrameRateFlag = 0;

    if (pstChan->enCurState == VDEC_CHAN_STATE_RUN) {
#ifndef JQW
	//fix bug 114865: vfmw support frame_rate=0.
	//                frame_rate=0, means frame_rate by stream;
	//                frame_rate>0, means frame_rate force by user.
	//if ((pstChan->stCurCfg.u32FrameRateInt != 0) || (pstChan->stCurCfg.u32FrameRateDec != 0))
	{
      /* Set to VFMW Frame Rate */

	  if(pstChan->stCurCfg.u32FrameRateDec == 97)
	  {
		  my_rate = pstChan->stCurCfg.u32FrameRateInt * 100 + pstChan->stCurCfg.u32FrameRateDec;
	  }
	  else
	  {
		  my_rate = pstChan->stCurCfg.u32FrameRateInt * 100;
	  }
  	  s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_DECLARE_FRAME_RATE, &(my_rate));
      if (VDEC_OK != s32Ret) {
	  	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
  	    MT_ERR_VDEC("VFMW SET FPS err!\n");
  	    return MT_FAILURE;
      }
	}
#endif

    	/*s32Ret = VDEC_SetAttr(pstChan);
    	if (MT_SUCCESS != s32Ret) {
    	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	    MT_ERR_VDEC("Chan %d SetAttr err!\n", hHandle);
    	    return MT_FAILURE;
    	}*/
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    MT_INFO_VDEC("Chan %ld MT_DRV_VDEC_SetFrameRate %u.%03u OK\n", hHandle, u32FrameRateInt, u32FrameRateDec);
    MLOGI("%s: Ch(%lu) set FPS %u.%03u success.\n",__FUNCTION__,hHandle,u32FrameRateInt, u32FrameRateDec);
    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_SetDecFrmType(mt_handle hHandle, mt_u32 u32DecFrameType)
{
	mt_s32 s32Ret;
	VDEC_CHANNEL_S *pstChan = MT_NULL;

	MLOGI("%s: hHandle 0x%08lx, u32DecFrameType %u\n", __FUNCTION__,hHandle,u32DecFrameType);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
    	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if ((pstChan->enCurState != VDEC_CHAN_STATE_RUN)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Invalid Chan %d state %d!\n", hHandle, pstChan->enCurState);
    	return MT_FAILURE;
    }

	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_DEC_MODE, &u32DecFrameType);
	if (VDEC_OK != s32Ret) {
	  VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	  MT_ERR_VDEC("VFMW GET CAPABILITY err!\n");
	  return MT_FAILURE;
	}

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    return MT_SUCCESS;
}

static mt_s32 MT_DRV_VDEC_SetTrickCfg(mt_handle hHandle, MT_BOOL is_incomplete_stream, MT_UNF_DEC_TRICK_MODE_E trickmode)
{
	mt_s32 s32Ret;
	VDEC_CHANNEL_S *pstChan = MT_NULL;
	fw_trick_mode_info_t stTrick = {0};

	MLOGI("MT_DRV_VDEC_SetTrickCfg: hHandle 0x%08lx, trickmode %d, stream %d\n",
		hHandle, trickmode, is_incomplete_stream);

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
		return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
    	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
    	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if ((MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
    	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if ((pstChan->enCurState != VDEC_CHAN_STATE_RUN)) {
    	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    	MT_ERR_VDEC("Invalid Chan %d state %d!\n", hHandle, pstChan->enCurState);
    	return MT_FAILURE;
    }

	stTrick.not_original_stream_flag = is_incomplete_stream;
	if(trickmode == MT_UNF_DEC_TM_FFWD)
		stTrick.trick_mode = FW_VDEC_TM_FFWD;
	else if(trickmode == MT_UNF_DEC_TM_FREV)
		stTrick.trick_mode = FW_VDEC_TM_FREV;
	else if(trickmode == MT_UNF_DEC_TM_SFWD)
		stTrick.trick_mode = FW_VDEC_TM_SFWD;
	else if(trickmode == MT_UNF_DEC_TM_SREV)
		stTrick.trick_mode = FW_VDEC_TM_SREV;
	else
		stTrick.trick_mode = FW_VDEC_TM_NORMAL;

	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_TRICK_MODE, &stTrick);
	if (VDEC_OK != s32Ret) {
	  VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	  MT_ERR_VDEC("VFMW GET CAPABILITY err!\n");
	  return MT_FAILURE;
	}

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    return MT_SUCCESS;
}

static mt_s32 VDEC_IFrame_BufInit(mt_u32 u32BufSize, mmz_buffer_s *pstMMZBuf)
{
    ENTER_FUNCTION;
#if defined(CFG_ANDROID_TOOLCHAIN)
    return mt_drv_mmz_alloc_and_map("VDEC_IFrame", MMZ_OTHERS, u32BufSize, 0, pstMMZBuf);
#else
    return mt_drv_mmz_alloc_and_map("VDEC_IFrame", MMZ_OTHERS, u32BufSize, 0, pstMMZBuf);
#endif
}

static mt_void VDEC_IFrame_BufDeInit(mmz_buffer_s *pstMMZBuf)
{
    ENTER_FUNCTION;
    mt_drv_mmz_unmap_and_release(pstMMZBuf);
}

mt_s32 MT_DRV_VDEC_DecodeIFrame(mt_handle hHandle, MT_UNF_AVPLAY_I_FRAME_S *pstStreamInfo,
                                MT_DRV_VIDEO_FRAME_S *pstFrameInfo, MT_BOOL bCapture, MT_BOOL bUserSpace)
{
    mt_s32 s32Ret, i;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    STREAM_INTF_S stSteamIntf;
#if 0
    mt_s32 s322DbufferSize;
    MT_DRV_VDEC_BTL_S stBtl;
    MT_DRV_VIDEO_FRAME_S st2dFrame;
#endif

    ENTER_FUNCTION;
    /*parameter check*/
    if ((MT_NULL == pstStreamInfo) || (MT_NULL == pstFrameInfo)) {
	MT_ERR_VDEC("bad param\n");
	return MT_FAILURE;
    }

    hHandle = hHandle & 0xff;

    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("bad handle %d!\n", hHandle);
	return MT_FAILURE;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (pstChan->enCurState != VDEC_CHAN_STATE_STOP) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d state err:%d!\n", hHandle, pstChan->enCurState);
	return MT_FAILURE;
    }

    if (!VDEC_CHAN_STRMBUF_ATTACHED(pstChan)) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d bad strm buf!\n", hHandle);
	return MT_FAILURE;
    }

	MLOGI("%s: Handle=%lx, Capture=%d, UserSpace=%d, Type=%d, Addr=%px, BufSize=%u\n",__FUNCTION__,
			hHandle,bCapture,bUserSpace,
			pstStreamInfo->enType,pstStreamInfo->pu8Addr,pstStreamInfo->u32BufSize);

    if (MT_INVALID_HANDLE == pstChan->hChan) {
	s32Ret = VDEC_Chan_Create(hHandle);
	if (MT_SUCCESS != s32Ret) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("IFrame VDEC_Chan_Create err\n");
	    return MT_FAILURE;
	}
    }
    /* Modify the decoder's attributes */
    s32Ret = VDEC_IFrame_SetAttr(pstChan, pstStreamInfo->enType);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("IFrame_SetAttr err\n");
	return MT_FAILURE;
    }

    pstChan->bIsIFrameDec = MT_TRUE;

    /* Init I frame buffer */
    pstChan->stIFrame.u32ReadTimes = 0;
    /* malloc the memory to save the stream from user,unmalloc below (OUT1) until read the frame from vfmw successly */
    s32Ret = VDEC_IFrame_BufInit(pstStreamInfo->u32BufSize, &(pstChan->stIFrame.stMMZBuf));
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Alloc IFrame buf err\n");
	goto OUT0;
    }

    if (!bUserSpace) {
	/*not from user space : from kernel ddr ,use memcpy is ok*/
	memcpy((mt_u8 *)pstChan->stIFrame.stMMZBuf.startVirAddr,
	       pstStreamInfo->pu8Addr, pstStreamInfo->u32BufSize);
    } else {
	/*I MODE : the stream from user is one IFrame stream, copy the stream from user space , should use function : copy_from_user*/
	if (0 != copy_from_user((mt_u8 *)pstChan->stIFrame.stMMZBuf.startVirAddr,
	                        pstStreamInfo->pu8Addr, pstStreamInfo->u32BufSize)) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    MT_ERR_VDEC("Chan %d IFrame copy %dB %px->%px err!\n",
	                hHandle, pstStreamInfo->u32BufSize, pstStreamInfo->pu8Addr,
	                (mt_u8 *)pstChan->stIFrame.stMMZBuf.startVirAddr);
	    goto OUT1;
	}
	MT_INFO_VDEC("Chan %d IFrame copy %dB %px->%px success!\n",
	             hHandle, pstStreamInfo->u32BufSize, pstStreamInfo->pu8Addr,
	             (mt_u8 *)pstChan->stIFrame.stMMZBuf.startVirAddr);
    }

    /* Set IFrame stream read functions */
    stSteamIntf.stream_provider_inst_id = hHandle;
    stSteamIntf.read_stream = VDEC_IFrame_GetStrm;
    stSteamIntf.release_stream = VDEC_IFrame_PutStrm;
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_CHAN, MT_NULL);
    if (VDEC_OK != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d SET_STREAM_INTF err!\n", hHandle);
	goto OUT1;
    }

    /* Start decode */
    memset(&pstChan->stStatInfo, 0, sizeof(VDEC_CHAN_STATINFO_S));
    //s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_START_CHAN, MT_NULL);
    if (VDEC_OK != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Chan %d START_CHAN err!\n", hHandle);
	goto OUT2;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_START, NULL);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_START err in DecodeIFrame!\n");
	return MT_FAILURE;
    }
    pstChan->enCurState = VDEC_CHAN_STATE_RUN;

    /* Start PTS recover channel */
    PTSREC_Start(hHandle);

    /* Here we invoke USE_UP function to release the atomic number, so that the VFMW can decode
      by invoking the interface of get stream, the VDEC_Event_Handle function can also hHandle
      EVNT_NEW_IMAGE */
    /*CNcomment: 此处调用USE_UP释放锁 以使 VFMW 可以调用获取码流接口 进行解码
      VDEC_Event_Handle 可以处理 EVNT_NEW_IMAGE 事件 */
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

    /* wait for vdec process complete */
    for (i = 0; i < 10; i++) {
	msleep(10);
    }

#if 0
    /* Waiting for decode complete */
    for (i = 0; i < 40; i++)
    {
        s32Ret = MT_FAILURE;
        msleep(5);
        s32Ret = MT_DRV_VDEC_RecvFrmBuf(hHandle, pstFrameInfo);
        if (MT_SUCCESS == s32Ret)
        {
            /* Elude 4+64 EVNT_RESOLUTION_CHANGE event
             * In this case, MT_DRV_VDEC_RecvFrmBuf will return MT_SUCCESS but give an invalid frame,
             * only send the flag.
             */
            if (0 != pstFrameInfo->u32Width)
            {
                break;
            }
        }
    }

    if (i >= 40)
    {
        MT_ERR_VDEC("IFrame decode timeout\n");
        goto OUT3;
    }
    /*OSD just need 1D data ,vdec malloc the memory to save the 1D data from vfmw and release it, user use IFrameRelease to release the memory malloc by vdec*/
    if ((bCapture) && (0 == pstChan->stOption.s32Btl1Dt2DEnable))
    {
        s322DbufferSize = pstFrameInfo->u32Width * pstFrameInfo->u32Height * 3;
#if defined(CFG_ANDROID_TOOLCHAIN)
        s32Ret = mt_drv_mmz_alloc_and_map("VDEC_IFrame_2d", "vdec", s322DbufferSize, 0, &pstChan->stIFrame.st2dBuf);
#else
        s32Ret = mt_drv_mmz_alloc_and_map("VDEC_IFrame_2d", MT_NULL, s322DbufferSize, 0, &pstChan->stIFrame.st2dBuf);
#endif
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_VDEC("Alloc 2d buffer fail:%d.\n", s322DbufferSize);
            goto OUT4;
        }

        stBtl.u32PhyAddr = pstChan->stIFrame.st2dBuf.u32StartPhyAddr;
        stBtl.u32Size = s322DbufferSize;
        stBtl.u32TimeOutMs = 1000;
        stBtl.pstInFrame = pstFrameInfo;
        stBtl.pstOutFrame = &st2dFrame;
        s32Ret = MT_DRV_VDEC_BlockToLine(hHandle, &stBtl);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_VDEC("BTL fail.\n");
            mt_drv_mmz_unmap_and_release(&pstChan->stIFrame.st2dBuf);
            pstChan->stIFrame.st2dBuf.u32Size = 0;
            goto OUT4;
        }

        st2dFrame.stDispRect.s32Width = pstFrameInfo->stDispRect.s32Width;
        st2dFrame.stDispRect.s32Height = pstFrameInfo->stDispRect.s32Height;
        st2dFrame.stDispRect.s32X = pstFrameInfo->stDispRect.s32X;
        st2dFrame.stDispRect.s32Y = pstFrameInfo->stDispRect.s32Y;

OUT4:
        /* For capture, release vfmw frame here. For VO, release in vo */
        MT_DRV_VDEC_RlsFrmBuf(hHandle, pstFrameInfo);
        *pstFrameInfo = st2dFrame;
    }

OUT3:
#endif
    /* Stop channel */
    PTSREC_Stop(hHandle);
#if 1
    KERN_VDEC_Control(pstChan->hChan, VDEC_CID_STOP_CHAN, MT_NULL);
    pstChan->enCurState = VDEC_CHAN_STATE_STOP;
#endif
OUT2:
#if 0
    /* Resume stream interface */
    KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_STREAM_INTF, &pstChan->stStrmIntf);
#endif

OUT1:

    /* Free MMZ buffer */
    VDEC_IFrame_BufDeInit(&(pstChan->stIFrame.stMMZBuf));
    pstChan->stIFrame.u32ReadTimes = 0;

OUT0:
    /* Resume channel attribute */
    s32Ret = VDEC_SetAttr(pstChan);
    s32Ret |= VDEC_Chan_Destroy(hHandle);

    pstChan->bIsIFrameDec = MT_FALSE;

    return s32Ret;
}

mt_s32 MT_DRV_VDEC_ReleaseIFrame(mt_handle hHandle, MT_DRV_VIDEO_FRAME_S *pstFrameInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;
    // VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    return s32Ret;
#if 0
    if (MT_NULL == pstFrameInfo)
    {
        MT_ERR_VDEC("Bad param!\n");
        return MT_FAILURE;
    }

    hHandle = hHandle & 0xff;
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        return MT_ERR_VDEC_INVALID_PARA;
    }

    /* Lock */
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret)
    {
        MT_INFO_VDEC("Chan %d lock fail!\n", hHandle);
        return MT_FAILURE;
    }

    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan)
    {
        VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
        MT_ERR_VDEC("Chan %d not init!\n", hHandle);
        return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

    if (0 != pstChan->stIFrame.st2dBuf.u32Size)
    {
        mt_drv_mmz_unmap_and_release(&pstChan->stIFrame.st2dBuf);
        pstChan->stIFrame.st2dBuf.u32Size = 0;
    }

    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
#endif
}

mt_s32 MT_DRV_VDEC_SetLowdelay(mt_handle hVdec, MT_BOOL bLowdelay)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    mt_handle hVpss;
    MT_DRV_VPSS_CFG_S stVpssCfg;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    /* Check and get pstChan pointer */
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    pstChan->bLowdelay = bLowdelay;
    MLOGI("%s: Handle=%lx, Lowdelay=%d\n",__FUNCTION__,hVdec,bLowdelay);
    if (bLowdelay) {
	pstChan->stCurCfg.enMode = MT_UNF_VCODEC_MODE_IP;
	pstChan->stCurCfg.bOrderOutput = 1;
	pstChan->stCurCfg.s32CtrlOptions = 1;
    } else {
	pstChan->stCurCfg.enMode = MT_UNF_VCODEC_MODE_NORMAL;
	pstChan->stCurCfg.bOrderOutput = 0;
	pstChan->stCurCfg.s32CtrlOptions = 0;
    }
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec, &hVpss);
    if (MT_SUCCESS == s32Ret) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGetVpssCfg)(hVpss, &stVpssCfg);
	if (MT_SUCCESS == s32Ret) {
	    stVpssCfg.bAlwaysFlushSrc = bLowdelay;
	} else {
	    MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay call pfnVpssGetVpssCfg err.\n");
	    return MT_FAILURE;
	}
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSetVpssCfg)(hVpss, &stVpssCfg);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay call pfnVpssSetVpssCfg err.\n");
	    return MT_FAILURE;
	}
    } else {
	MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay call VDEC_FindVpssHandleByVdecHandle err.\n");
	return MT_FAILURE;
    }

    s32Ret = MT_DRV_VDEC_SetDEI(hVdec, MT_TRUE);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VDEC_SetLowdelay call MT_DRV_VDEC_SetDEI err.\n");
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 VDEC_DRV_VPU_CreateFrameList(mt_handle hVdec)
{
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_DRV_VPU_CreateFrameList err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    memset(&pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock, 0, sizeof(BUFMNG_VPSS_IRQ_LOCK_S));
    s32Ret = BUFMNG_InitSpinLock(&pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock);
    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }
    INIT_LIST_HEAD(&pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList);
    return MT_SUCCESS;
}
static mt_s32 VDEC_DRV_VPU_ReleaseFrameList(mt_handle hVdec)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    struct list_head *pos, *n;
    VDEC_VPU_FRAME_LIST_NODE_S *pstTarget;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_DRV_VPU_ReleaseFrameList err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    if ((&(pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList)) == pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList.next) {
	return MT_SUCCESS;
    }
    list_for_each_safe(pos, n, &(pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList))
    {
	BUFMNG_SpinLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
	pstTarget = list_entry(pos, VDEC_VPU_FRAME_LIST_NODE_S, node);
	if (MT_NULL != pstTarget) {
	    list_del_init(pos);
	    BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
	    MT_VFREE_BUFMNG(pstTarget);
	} else {
	    BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
	    s32Ret = MT_FAILURE;
	}
    }
    return s32Ret;
}

//stVPUFrame[in]
//pstChan[in]
//pstFrame[out]
static mt_void VDEC_ConvertUserFrame(MT_DRV_VDEC_USR_FRAME_S stVPUFrame, VDEC_CHANNEL_S *pstChan, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    MT_VDEC_PRIV_FRAMEINFO_S *pstPrivInfo = (MT_VDEC_PRIV_FRAMEINFO_S *)(((MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv))->u32Reserve);
    MT_DRV_VIDEO_PRIVATE_S *pstVideoPriv = (MT_DRV_VIDEO_PRIVATE_S *)(pstFrame->u32Priv);
    memset(pstFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    pstPrivInfo->image_id = stVPUFrame.s32FrameID;
    pstFrame->u32FrameIndex = pstChan->u32FrameCnt;
    if (pstChan->u32FrameCnt >= 0xFFFFFFFE) {
	pstChan->u32FrameCnt = 0;
    } else {
	pstChan->u32FrameCnt++;
    }
    pstFrame->u32Pts = stVPUFrame.u32Pts;
    pstFrame->u32SrcPts = stVPUFrame.u32Pts;
    pstFrame->stBufAddr[0].u32PhyAddr_YHead = stVPUFrame.s32LumaPhyAddr;
    pstFrame->stBufAddr[0].u32PhyAddr_Y = stVPUFrame.s32LumaPhyAddr;
    pstFrame->stBufAddr[0].u32Stride_Y = stVPUFrame.s32LumaStride;
    pstFrame->stBufAddr[0].u32PhyAddr_CHead = stVPUFrame.s32CbPhyAddr;
    pstFrame->stBufAddr[0].u32PhyAddr_C = stVPUFrame.s32CbPhyAddr;
    pstFrame->stBufAddr[0].u32Stride_C = stVPUFrame.s32ChromCrStride;
    pstFrame->stBufAddr[0].u32PhyAddr_CrHead = stVPUFrame.s32CrPhyAddr;
    pstFrame->stBufAddr[0].u32PhyAddr_Cr = stVPUFrame.s32CrPhyAddr;
    pstFrame->stBufAddr[0].u32Stride_Cr = stVPUFrame.s32ChromCrStride;
    pstFrame->u32Width = stVPUFrame.s32YWidth;
    pstFrame->u32Height = stVPUFrame.s32YHeight;
    pstFrame->ePixFormat = MT_DRV_PIX_FMT_NV21;
    pstFrame->enFieldMode = MT_DRV_FIELD_ALL;
    pstVideoPriv->eColorSpace = MT_DRV_CS_BT601_YUV_LIMITED;
    pstVideoPriv->stVideoOriginalInfo.enSource = MT_DRV_SOURCE_DTV;
    pstVideoPriv->stVideoOriginalInfo.u32Width = pstFrame->u32Width;
    pstVideoPriv->stVideoOriginalInfo.u32Height = pstFrame->u32Height;
    pstVideoPriv->stVideoOriginalInfo.u32FrmRate = 25;
    pstVideoPriv->stVideoOriginalInfo.en3dType = MT_DRV_FT_NOT_STEREO;
    pstVideoPriv->stVideoOriginalInfo.enSrcColorSpace = MT_DRV_CS_BT601_YUV_LIMITED;
    pstVideoPriv->stVideoOriginalInfo.enColorSys = MT_DRV_COLOR_SYS_AUTO;
    pstVideoPriv->stVideoOriginalInfo.bGraphicMode = MT_FALSE;
    pstVideoPriv->stVideoOriginalInfo.bInterlace = 0;
    return;
}
static mt_s32 VDEC_DRV_VPU_PutFrame(mt_handle hVdec, MT_DRV_VDEC_USR_FRAME_S stVPUFrame)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    MT_DRV_VIDEO_FRAME_S stFrame;
    VDEC_VPU_FRAME_LIST_NODE_S *pstBufNode;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_DRV_VPU_PutFrame err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    memset(&stFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    VDEC_ConvertUserFrame(stVPUFrame, pstChan, &stFrame);
    pstBufNode = MT_VMALLOC_BUFMNG(sizeof(VDEC_VPU_FRAME_LIST_NODE_S));
    if (MT_NULL == pstBufNode) {
	MT_ERR_VDEC("VDEC_DRV_VPU_PutFrame No memory.\n");
	return MT_ERR_BM_NO_MEMORY;
    }
    memcpy(&(pstBufNode->stPortOutFrame), &stFrame, sizeof(MT_DRV_VIDEO_FRAME_S));
    BUFMNG_SpinLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
    list_add_tail(&(pstBufNode->node), &(pstChan->stVPUParam.stVPUFrameList.stVdecVPUFrameList));
    BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPUFrameList.stVPUFrameListLock));
    return MT_SUCCESS;
}
// add by w00278582
static mt_s32 VDEC_DRV_VPU_Status(mt_handle hVdec, MT_DRV_VDEC_VPU_STATUS_S *stVPUStatus)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VPU_STATUS err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("Chan not create!\n");
	return MT_FAILURE;
    }

    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;

    if (MT_NULL == pstChan) {
	return MT_FAILURE;
    }

    memcpy(&(pstChan->stVdecVpuStatus), stVPUStatus, sizeof(MT_DRV_VDEC_VPU_STATUS_S));
    return MT_SUCCESS;
}
static mt_s32 VDEC_DRV_VPU_SetAttr(mt_handle hVdec, VDEC_VPU_ATTR_S stVPUAttr)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_DRV_VPU_SetAttr err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("VDEC_DRV_VPU_SetAttr Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    pstChan->bVPU = stVPUAttr.bVPU;
    return MT_SUCCESS;
}
static mt_s32 VDEC_DRV_VPU_CheckRlsFrameID(mt_handle hVdec, mt_s32 *pID, mt_s32 *ps32Count)
{
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hVdec) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    hVdec &= 0xFF;
    if (hVdec >= MT_VDEC_MAX_INSTANCE_NEW) {
	MT_ERR_VDEC("VDEC_DRV_VPU_SetAttr err hvdec:%d too large!\n", hVdec);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hVdec].pstChan) {
	MT_ERR_VDEC("VDEC_DRV_VPU_SetAttr Chan not create!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
    BUFMNG_SpinLockIRQ(&(pstChan->stVPUParam.stVPURlsParam.stVPURlsFrameListLock));
    memcpy(pID, pstChan->stVPUParam.stVPURlsParam.RlsFrameIDArray, sizeof(mt_s32) * MT_VDEC_MAX_VPU_FRAME_NUM);
    *ps32Count = pstChan->stVPUParam.stVPURlsParam.s32AvailableNum;
    pstChan->stVPUParam.stVPURlsParam.s32AvailableNum = 0;
    BUFMNG_SpinUnLockIRQ(&(pstChan->stVPUParam.stVPURlsParam.stVPURlsFrameListLock));
    return MT_SUCCESS;
}
static mt_s32 MT_DRV_VDEC_VPU_ChanStart(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_BOOL bVPU = MT_FALSE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    if (MT_TRUE == pstChan->bVPU) {
	bVPU = MT_TRUE;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_CHANGEIP, &bVPU);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_CHANGEIP err!\n");
	return MT_FAILURE;
    }
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_START, NULL);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_START err!\n");
	return MT_FAILURE;
    }
    pstChan->enCurState = VDEC_CHAN_STATE_RUN;
    return MT_SUCCESS;
}
static mt_s32 MT_DRV_VDEC_VPU_ChanStop(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss) {
	s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(s_stVdecDrv.astChanEntity[hHandle].stVpssChan.hVpss, MT_DRV_VPSS_USER_COMMAND_STOP, NULL);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("MT_DRV_VPSS_USER_COMMAND_STOP err!\n");
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    return MT_FAILURE;
	}
    }
    pstChan->enCurState = VDEC_CHAN_STATE_STOP;
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_PTS_Alloc(mt_handle hHandle, mt_handle hHandle_vpu)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    s32Ret = PTSREC_Alloc(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_VPU_PTS_Alloc err!\n");
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle & 0xff].pstChan;
    pstChan->stCurCfg.enType = MT_UNF_VCODEC_TYPE_HEVC;
    pstChan->hDmxVidChn = MT_INVALID_HANDLE;
    if (MT_FALSE == pstChan->bProcRegister) {
	s32Ret = VDEC_RegChanProc(hHandle);
	if (MT_SUCCESS == s32Ret) {
	    pstChan->bProcRegister = MT_TRUE;
	} else {
	    pstChan->bProcRegister = MT_FALSE;
	}
	s32Ret = VDEC_VPU_RegChanProc(hHandle_vpu);
	if (MT_SUCCESS == s32Ret) {
	    pstChan->bVPUProcRegister = MT_TRUE;
	} else {
	    pstChan->bVPUProcRegister = MT_FALSE;
	}
	pstChan->u32VPUhandle = hHandle_vpu;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_PTS_Free(mt_handle hHandle, mt_handle hHandle_vpu)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    s32Ret = PTSREC_Free(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_VPU_PTS_Free err!\n");
	return MT_FAILURE;
    }
    if (MT_TRUE == pstChan->bProcRegister) {
	VDEC_UnRegChanProc(hHandle);
	pstChan->bProcRegister = MT_FALSE;
    }
    if (MT_TRUE == pstChan->bVPUProcRegister) {
	VDEC_VPU_UnRegChanProc(hHandle_vpu);
	pstChan->bVPUProcRegister = MT_FALSE;
	pstChan->u32VPUhandle = 0xffffffff;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_PTS_Start(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    ENTER_FUNCTION;
    s32Ret = PTSREC_Start(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_VPU_PTS_Start err!\n");
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_PTS_Stop(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    ENTER_FUNCTION;
    s32Ret = PTSREC_Stop(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_VPU_PTS_Stop err!\n");
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_PTS_Reset(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    ENTER_FUNCTION;
    s32Ret = PTSREC_Reset(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_VPU_PTS_Reset err!\n");
	return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_Get_FrameRateForNewFrm(mt_handle hHandle, MT_DRV_VIDEO_FRAME_S *pstFrame)
{
    mt_s32 s32Ret = MT_SUCCESS;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    ENTER_FUNCTION;
    hHandle &= 0xFF;
    if ((MT_NULL == pstFrame) || (MT_INVALID_HANDLE == hHandle)) {
	MT_ERR_VDEC("Bad param!\n");
	return MT_FAILURE;
    }
    memset(pstFrame, 0, sizeof(MT_DRV_VIDEO_FRAME_S));
    if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	return MT_ERR_VDEC_INVALID_PARA;
    }
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    if (MT_SUCCESS != s32Ret) {
	MT_WARN_VDEC("Chan %d lock fail!\n", hHandle);
	return MT_FAILURE;
    }
    if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_WARN_VDEC("Chan %d not init!\n", hHandle);
	return MT_FAILURE;
    }
    pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;
    //memcpy(pstFrame, &(pstChan->stLastDispFrameInfo), sizeof(MT_DRV_VIDEO_FRAME_S));
    memcpy(pstFrame, &(pstChan->stLastFrm), sizeof(MT_DRV_VIDEO_FRAME_S));
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return MT_SUCCESS;
}
static mt_s32 VDEC_VPU_Get_Vpss_StatusInfo(mt_handle hHandle, MT_BOOL *bAllPortCompleteFrm)
{
    mt_s32 s32Ret;
    MT_BOOL bPortCompleteFrm = MT_FALSE;
    VDEC_VPSSCHANNEL_S *pstVpssChan = MT_NULL;
    ENTER_FUNCTION;
    hHandle &= 0xFF;
    VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
    pstVpssChan = &(s_stVdecDrv.astChanEntity[hHandle].stVpssChan);
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssSendCommand)(pstVpssChan->hVpss, MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE, &bPortCompleteFrm);
    if (MT_SUCCESS != s32Ret) {
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	MT_ERR_VDEC("Vpss:%d VDEC_VPU_Get_Vpss_StatusInfo error!\n", pstVpssChan->hVpss);
	return MT_FAILURE;
    }
    *bAllPortCompleteFrm = bPortCompleteFrm;
    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    return s32Ret;
}

/* Set HDR Information */
static mt_s32 VDEC_Chan_SetHdrInfo(mt_handle hHandle, MT_UNF_VIDEO_DISP_HDR_INFO_S stHdrInfo)
{
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    ENTER_FUNCTION;
    hHandle &= 0xFF;

	//debug
	dump_hdrinfo(&stHdrInfo);

	BUG_ON(sizeof(MT_UNF_VIDEO_DISP_HDR_INFO_S) != sizeof(FW_HDR_INFO_S));

	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);

	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
		MT_WARN_VDEC("Chan %d not init!\n", hHandle);
		return MT_FAILURE;
	}
	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

	s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_SET_HDR_INFO, (void*)&stHdrInfo);
	if (MT_SUCCESS != s32Ret) {
		MT_ERR_VDEC("VDEC[%u]: VDEC_CID_SET_HDR_INFO error(%d)!\n", hHandle, s32Ret);
	}

	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);

	return s32Ret;
}

mt_s32 VDEC_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, void *arg)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hHandle = MT_INVALID_HANDLE;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
    unsigned long res;
#endif

    //ENTER_FUNCTION;
    /* Check parameter in this switch */
    switch (cmd) {

    case UMAPC_VDEC_GETCAP:
	case UMAPC_VDEC_GETFB_MEM:
    case UMAPC_VDEC_ALLOCHANDLE:
    case UMAPC_VDEC_CREATE_ESBUF:
    case UMAPC_VDEC_DESTROY_ESBUF:
    case UMAPC_VDEC_GETBUF:
    case UMAPC_VDEC_PUTBUF:
    case UMAPC_VDEC_SETUSERADDR:
    case UMAPC_VDEC_RCVBUF:
    case UMAPC_VDEC_RLSBUF:
    case UMAPC_VDEC_RESET_ESBUF:
    case UMAPC_VDEC_GET_ESBUF_STATUS:
    case UMAPC_VDEC_CHAN_CREATE_VPU_BUF:
    case UMAPC_VDEC_CHAN_VPU_REVERT_FRAME_BUF:
    case UMAPC_VDEC_CHAN_VPU_CREATE_FRAMELIST:
    case UMAPC_VDEC_CHAN_VPU_RELEASE_FRAMELIST:
    case UMAPC_VDEC_CHAN_VPU_PUT_FRAME:
    case UMAPC_VDEC_CHAN_VPU_ATTR:
    case UMAPC_VDEC_CHAN_VPU_CHECK_RLSFRM:
    case UMAPC_VDEC_VPU_PTS_Alloc:
    case UMAPC_VDEC_VPU_PTS_Free:
    case UMAPC_VDEC_VPU_PROC:
	if (MT_NULL == arg) {
	    MT_ERR_VDEC("CMD %p Bad arg!\n", cmd);
	    return MT_ERR_VDEC_INVALID_PARA;
	}
	break;

    case UMAPC_VDEC_FREEHANDLE:
    case UMAPC_VDEC_CHAN_ALLOC:
    case UMAPC_VDEC_CHAN_FREE:
    case UMAPC_VDEC_CHAN_START:
    case UMAPC_VDEC_CHAN_STOP:
    case UMAPC_VDEC_CHAN_VPU_START:
    case UMAPC_VDEC_CHAN_VPU_STOP:
    case UMAPC_VDEC_VPU_PTS_Start:
    case UMAPC_VDEC_VPU_PTS_Stop:
    case UMAPC_VDEC_VPU_PTS_Reset:
    case UMAPC_VDEC_VPU_GET_FRAME_RATE:
	case UMAPC_VDEC_GET_DECODING_CAPABILITY:
    case UMAPC_VDEC_VPU_GET_VPSS_STATUSINFO:
    case UMAPC_VDEC_CHAN_RESET:
    case UMAPC_VDEC_CHAN_SETATTR:
    case UMAPC_VDEC_CHAN_GETATTR:
    case UMAPC_VDEC_CHAN_ATTACHBUF:
    case UMAPC_VDEC_CHAN_DETACHBUF:
    case UMAPC_VDEC_CHAN_SETEOSFLAG:
    case UMAPC_VDEC_CHAN_DISCARDFRM:
    case UMAPC_VDEC_CHAN_USRDATA:
    case UMAPC_VDEC_CHAN_STATUSINFO:
	case UMAPC_VDEC_GET_CI_TEST_INFO:
    case UMAPC_VDEC_CHAN_STREAMINFO:
    case UMAPC_VDEC_CHAN_CHECKEVT:
    case UMAPC_VDEC_CHAN_EVNET_NEWFRAME:
    case UMAPC_VDEC_CHAN_SETVOBUFCLEARFLAE:
    case UMAPC_VDEC_CHAN_IFRMDECODE:
    case UMAPC_VDEC_CHAN_IFRMRELEASE:
    case UMAPC_VDEC_CHAN_SETFRMRATE:
	case UMAPC_VDEC_CHAN_SETFRMRATE_DECLEAR:
    case UMAPC_VDEC_CHAN_GETFRMRATE:
    case UMAPC_VDEC_CHAN_GETFRM:
    case UMAPC_VDEC_CHAN_PUTFRM:
    case UMAPC_VDEC_CHAN_RLSFRM:
    case UMAPC_VDEC_CHAN_RCVFRM:
    case UMAPC_VDEC_CHAN_SETTRICKMODE:
    case UMAPC_VDEC_CHAN_PAUSE:
	case UMAPC_VDEC_CHAN_SETDECFRMTYPE:
	case UMAPC_VDEC_CHAN_SETTRICKCFG:
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    case UMAPC_VDEC_CHAN_USERDATAINITBUF:
    case UMAPC_VDEC_CHAN_USERDATASETBUFADDR:
    case UMAPC_VDEC_CHAN_ACQUSERDATA:
    case UMAPC_VDEC_CHAN_RLSUSERDATA:
    case UMAPC_VDEC_CHAN_RSTUSERDATABUF:
#endif
    case UMAPC_VDEC_CHAN_SEEKPTS:
    case UMAPC_VDEC_CHAN_SETCTRLINFO:
    case UMAPC_VDEC_CHAN_GETFRMSTATUSINFO:
    case UMAPC_VDEC_CHAN_SENDEOS:
    case UMAPC_VDEC_CHAN_GETPORTSTATE:
    case UMAPC_VDEC_CHAN_PROGRSSIVE:
    case UMAPC_VDEC_CHAN_LOWDELAY:
    case UMAPC_VDEC_CHAN_DPBFULL:
    case UMAPC_VDEC_CHAN_SETCOLORSPACE:
	case UMAPC_VDEC_CHAN_SETHDRINFO:
    case UMAPC_VDEC_CHAN_GETCRCBUF:
	case UMAPC_VDEC_GET_LCEVC_DATA:
	case UMAPC_VDEC_GETLCEVC_MEM:
	if (MT_NULL == arg) {
	    MT_ERR_VDEC("CMD %p Bad arg!\n", cmd);
	    return MT_ERR_VDEC_INVALID_PARA;
	}
	hHandle = (*((mt_handle *)arg)) & 0xff;
	if (hHandle >= MT_VDEC_MAX_INSTANCE_NEW) {
	    MT_ERR_VDEC("CMD %px bad handle:%d!\n", cmd, hHandle);
	    return MT_ERR_VDEC_INVALID_PARA;
	}
	break;
    case UMAPC_VDEC_CHAN_CREATEVPSS:
    case UMAPC_VDEC_CHAN_DESTORYVPSS:
	if (MT_NULL == arg) {
	    MT_ERR_VDEC("CMD %p Bad arg!\n", cmd);
	    return MT_ERR_VDEC_INVALID_PARA;
	}
	hHandle = (*((mt_handle *)arg)) & 0xff;
	break;
    //add by l00225186
    /*调用vpss接口的时候，使用的是vpss的句柄，因此需要将句柄参数传递过来，并提取*/
    case UMAPC_VDEC_CHAN_GETPORTPARAM:
    case UMAPC_VDEC_CHAN_RCVVPSSFRM:
    case UMAPC_VDEC_CHAN_CREATEPORT:
    case UMAPC_VDEC_CHAN_DESTROYPORT:
    case UMAPC_VDEC_CHAN_ENABLEPORT:
    case UMAPC_VDEC_CHAN_DISABLEPORT:
    case UMAPC_VDEC_CHAN_SETPORTTYPE:
    case UMAPC_VDEC_CHAN_CANCLEMAINPORT:
    case UMAPC_VDEC_CHAN_SETFRMPACKTYPE:
    case UMAPC_VDEC_CHAN_GETFRMPACKTYPE:
    case UMAPC_VDEC_CHAN_RESETVPSS:
    case UMAPC_VDEC_CHAN_SETPORTATTR:
    case UMAPC_VDEC_CHAN_GETPORTATTR:
    case UMAPC_VDEC_CHAN_RLSPORTFRM:
    case UMAPC_VDEC_CHAN_SETEXTBUFFER:
    case UMAPC_VDEC_CHAN_SETBUFFERMODE:
    case UMAPC_VDEC_CHAN_CHECKANDDELBUFFER:
    case UMAPC_VDEC_CHAN_SETEXTBUFFERSTATE:
    case UMAPC_VDEC_CHAN_SETRESOLUTION:
	if (MT_NULL == arg) {
	    MT_ERR_VDEC("CMD %p Bad arg!\n",cmd);
	    return MT_ERR_VDEC_INVALID_PARA;
	}
	hHandle = *((mt_handle *)arg); //vpss的句柄是什么生成规则未知，不做判断
	break;
    default:
	MT_ERR_VDEC("CMD %p unsupport now!\n", cmd);
	return MT_ERR_VDEC_NOT_SUPPORT;
    }

    /* Call function in this switch */
    switch (cmd) {
    case UMAPC_VDEC_CHAN_USRDATA: {
	s32Ret = MT_DRV_VDEC_GetUsrData(hHandle, &(((VDEC_CMD_USERDATA_S *)arg)->stUserData));
	break;
    }

    case UMAPC_VDEC_CHAN_IFRMDECODE: {
	VDEC_CMD_IFRAME_DEC_S *pstIFrameDec = (VDEC_CMD_IFRAME_DEC_S *)arg;
	s32Ret = MT_DRV_VDEC_DecodeIFrame((pstIFrameDec->hHandle) & 0xff, &pstIFrameDec->stIFrame,
	                                  &(pstIFrameDec->stVoFrameInfo), pstIFrameDec->bCapture, MT_TRUE);
	s32Ret |= VDEC_Chan_Reset((pstIFrameDec->hHandle) & 0xff, MT_NULL);
	break;
    }

    case UMAPC_VDEC_CHAN_IFRMRELEASE: {
	s32Ret = MT_DRV_VDEC_ReleaseIFrame(hHandle, &(((VDEC_CMD_IFRAME_RLS_S *)arg)->stVoFrameInfo));
	break;
    }

    case UMAPC_VDEC_CHAN_ALLOC: {
	//s32Ret = VDEC_Chan_Alloc(hHandle, &(((VDEC_CMD_ALLOC_S*)arg)->stOpenOpt));
	if (((VDEC_CMD_ALLOC_S *)arg)->u32DFSEnable > 1) //l00273086
	{
	    ((VDEC_CMD_ALLOC_S *)arg)->u32DFSEnable = 0;
	}
	s_stVdecDrv.astChanEntity[hHandle].u32DynamicFsEn = ((VDEC_CMD_ALLOC_S *)arg)->u32DFSEnable; //l00273086
	s32Ret = VDEC_Chan_InitParam(hHandle, &(((VDEC_CMD_ALLOC_S *)arg)->stOpenOpt));
	*((mt_handle *)arg) = hHandle;
	break;
    }
    case UMAPC_VDEC_CHAN_FREE: {
	MLOGD("%s: UMAPC_VDEC_CHAN_FREE \n",__FUNCTION__);
	s32Ret = VDEC_Chan_Free(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_START: {
	s32Ret = MT_DRV_VDEC_ChanStart(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_STOP: {
	s32Ret = MT_DRV_VDEC_ChanStop(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_RESET: {
	s32Ret = VDEC_Chan_Reset(hHandle, (VDEC_CMD_RESET_S *)arg);
	break;
    }
    case UMAPC_VDEC_CHAN_SETATTR: {
	s32Ret = MT_DRV_VDEC_SetChanAttr(hHandle, &(((VDEC_CMD_ATTR_S *)arg)->stAttr));
	break;
    }
    case UMAPC_VDEC_CHAN_GETATTR: {
	s32Ret = MT_DRV_VDEC_GetChanAttr(hHandle, &(((VDEC_CMD_ATTR_S *)arg)->stAttr));
	break;
    }
    case UMAPC_VDEC_CREATE_ESBUF: {
	s32Ret = VDEC_CreateStrmBuf((MT_DRV_VDEC_STREAM_BUF_S *)arg);
	break;
    }
    case UMAPC_VDEC_DESTROY_ESBUF: {
	s32Ret = VDEC_DestroyStrmBuf(*((mt_handle *)arg));
	break;
    }
    case UMAPC_VDEC_GETBUF: {
	VDEC_CMD_BUF_S *pstBuf = (VDEC_CMD_BUF_S *)arg;
	s32Ret = VDEC_GetStrmBuf(pstBuf->hHandle, &(pstBuf->stBuf), MT_TRUE);
	break;
    }
    case UMAPC_VDEC_PUTBUF: {
	VDEC_CMD_BUF_S *pstBuf = (VDEC_CMD_BUF_S *)arg;
	s32Ret = VDEC_PutStrmBuf(pstBuf->hHandle, &(pstBuf->stBuf), MT_TRUE);
	break;
    }
    case UMAPC_VDEC_SETUSERADDR: {
	VDEC_CMD_BUF_USERADDR_S *pstUserAddr = (VDEC_CMD_BUF_USERADDR_S *)arg;
	s32Ret = VDEC_StrmBuf_SetUserAddr(pstUserAddr->hHandle, pstUserAddr->u32UserAddr);
	break;
    }
    case UMAPC_VDEC_RCVBUF: {
	BUFMNG_BUF_S stEsBuf;
	VDEC_CMD_BUF_S *pstBuf = (VDEC_CMD_BUF_S *)arg;

	s32Ret = BUFMNG_AcqReadBuffer(pstBuf->hHandle, &stEsBuf);
	if (MT_SUCCESS == s32Ret) {
	    pstBuf->stBuf.pu8Addr = stEsBuf.pu8UsrVirAddr;
	    pstBuf->stBuf.u32PhyAddr = stEsBuf.u32PhyAddr;
	    pstBuf->stBuf.u32BufSize = stEsBuf.u32Size;
	    pstBuf->stBuf.u64Pts = stEsBuf.u64Pts;
	    pstBuf->stBuf.bEndOfFrame = !(stEsBuf.u32Marker & BUFMNG_NOT_END_FRAME_BIT);
	    pstBuf->stBuf.bDiscontinuous = (stEsBuf.u32Marker & BUFMNG_DISCONTINUOUS_BIT) ? 1 : 0;
	}
	break;
    }
    case UMAPC_VDEC_RLSBUF: {
	BUFMNG_BUF_S stEsBuf;
	VDEC_CMD_BUF_S *pstBuf = (VDEC_CMD_BUF_S *)arg;

	stEsBuf.u32PhyAddr = 0;
	stEsBuf.pu8UsrVirAddr = pstBuf->stBuf.pu8Addr;
	stEsBuf.pu8KnlVirAddr = MT_NULL;
	stEsBuf.u32Size = pstBuf->stBuf.u32BufSize;
	stEsBuf.u64Pts = pstBuf->stBuf.u64Pts;
	/* Don't care stEsBuf.u32Marker here. */
	s32Ret = BUFMNG_RlsReadBuffer(pstBuf->hHandle, &stEsBuf);
	break;
    }
    case UMAPC_VDEC_RESET_ESBUF: {
    	s32Ret = BUFMNG_Reset((*((mt_handle *)arg)));
    	break;
        }
    case UMAPC_VDEC_GET_ESBUF_STATUS: {
    	BUFMNG_STATUS_S stBMStatus;
    	VDEC_CMD_BUF_STATUS_S *pstStatus = (VDEC_CMD_BUF_STATUS_S *)arg;
    	s32Ret = BUFMNG_GetStatus(pstStatus->hHandle, &stBMStatus);
    	if (MT_SUCCESS == s32Ret) {
    	    pstStatus->stStatus.u32Size = stBMStatus.u32Used + stBMStatus.u32Free;
    	    pstStatus->stStatus.u32Available = stBMStatus.u32Free;
    	    pstStatus->stStatus.u32Used = stBMStatus.u32Used;
    	    pstStatus->stStatus.u32DataNum = stBMStatus.u32DataNum;
    	}
    	break;
    }
    case UMAPC_VDEC_CHAN_RLSFRM: {
    	s32Ret = MT_DRV_VDEC_RlsFrmBuf(hHandle, &(((VDEC_CMD_VO_FRAME_S *)arg)->stFrame));
    	break;
        }
    case UMAPC_VDEC_CHAN_RCVFRM: {
    	s32Ret = MT_DRV_VDEC_RecvFrmBuf(hHandle, &(((VDEC_CMD_VO_FRAME_S *)arg)->stFrame));
    	break;
        }
    case UMAPC_VDEC_CHAN_STATUSINFO: {
    	s32Ret = MT_DRV_VDEC_GetChanStatusInfo(hHandle, &(((VDEC_CMD_STATUS_S *)arg)->stStatus));
    	break;
        }
	case UMAPC_VDEC_GET_CI_TEST_INFO: {
		s32Ret = MT_DRV_VDEC_GetCiTestInfo(hHandle, &(((VDEC_CMD_CI_TEST_INFO_S *)arg)->stInfo));
		break;
		}
    case UMAPC_VDEC_CHAN_STREAMINFO: {
    	s32Ret = MT_DRV_VDEC_GetChanStreamInfo(hHandle, &(((VDEC_CMD_STREAM_INFO_S *)arg)->stInfo));
    	break;
      }
    case UMAPC_VDEC_CHAN_ATTACHBUF: {
    	VDEC_CMD_ATTACH_BUF_S stParam = *((VDEC_CMD_ATTACH_BUF_S *)arg);
    	s32Ret = VDEC_Chan_AttachStrmBuf(hHandle, stParam.u32BufSize, stParam.hDmxVidChn, stParam.hStrmBuf);
    	break;
        }
    case UMAPC_VDEC_CHAN_DETACHBUF: {
    	s32Ret = VDEC_Chan_DetachStrmBuf(hHandle);
    	break;
        }
    case UMAPC_VDEC_CHAN_SETEOSFLAG: {
	s32Ret = MT_DRV_VDEC_SetEosFlag(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_DISCARDFRM: {
	s32Ret = MT_DRV_VDEC_DiscardFrm(hHandle, &(((VDEC_CMD_DISCARD_FRAME_S *)arg)->stDiscardOpt));
	break;
    }
    case UMAPC_VDEC_CHAN_CHECKEVT: {
    	s32Ret = MT_DRV_VDEC_CheckNewEvent(hHandle, &(((VDEC_CMD_EVENT_S *)arg)->stEvent));
    	break;
    }
    case UMAPC_VDEC_CHAN_SETVOBUFCLEARFLAE:{
      s32Ret = MT_DRV_VDEC_SetVOBufClear(hHandle, (((VDEC_CMD_BUFCLEAR_S *)arg)->stBuffClearFlag));
	    break;
    }
    case UMAPC_VDEC_CHAN_EVNET_NEWFRAME: {
	VDEC_CMD_FRAME_S *pstFrameCmd = (VDEC_CMD_FRAME_S *)arg;
	pstFrameCmd->stFrame = s_stVdecDrv.astChanEntity[pstFrameCmd->hHandle & 0xff].pstChan->stLastFrm;
	s32Ret = MT_SUCCESS;
	break;
    }
    case UMAPC_VDEC_CHAN_GETFRM: {
	s32Ret = MT_DRV_VDEC_GetFrmBuf(hHandle, &(((VDEC_CMD_GET_FRAME_S *)arg)->stFrame));
	break;
    }
    case UMAPC_VDEC_CHAN_PUTFRM: {
	s32Ret = MT_DRV_VDEC_PutFrmBuf(hHandle, &(((VDEC_CMD_PUT_FRAME_S *)arg)->stFrame));
	break;
    }

    case UMAPC_VDEC_CHAN_SETFRMRATE: {
	MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstParam = &(((VDEC_CMD_FRAME_RATE_S *)arg)->stFrameRate);
	if (MT_NULL != s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    s_stVdecDrv.astChanEntity[hHandle].pstChan->stFrameRateParam = *pstParam;
	    s32Ret = PTSREC_SetFrmRate(hHandle, pstParam);
		s32Ret |= MT_DRV_VDEC_SetFrameRate(hHandle, pstParam->stSetFrmRate.u32fpsInteger, pstParam->stSetFrmRate.u32fpsDecimal);
	} else {
	    s32Ret = MT_FAILURE;
	}
	break;
    }
    case UMAPC_VDEC_CHAN_SETFRMRATE_DECLEAR: {
	MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstParam = &(((VDEC_CMD_FRAME_RATE_S *)arg)->stFrameRate);
	if (MT_NULL != s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    s_stVdecDrv.astChanEntity[hHandle].pstChan->stFrameRateParam = *pstParam;
	    s32Ret = PTSREC_SetFrmRate(hHandle, pstParam);
		s32Ret |= MT_DRV_VDEC_SetFrameRateDeclear(hHandle, pstParam->stSetFrmRate.u32fpsInteger, pstParam->stSetFrmRate.u32fpsDecimal);
	} else {
	    s32Ret = MT_FAILURE;
	}
	break;
    }
	
	case UMAPC_VDEC_GET_DECODING_CAPABILITY:
	{
	FW_VDEC_CAPABILITY_INFO_S *pstCapability = &(((VDEC_CMD_CAPABILITY_S *)arg)->stCapability);
	s32Ret = MT_DRV_VDEC_GetVdecCapability(hHandle, (FW_VDEC_CAPABILITY_INFO_S *)pstCapability);
	break;
	}

    case UMAPC_VDEC_CHAN_GETFRMRATE: {
	MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstParam = &(((VDEC_CMD_FRAME_RATE_S *)arg)->stFrameRate);
	if (MT_NULL != s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    *pstParam = s_stVdecDrv.astChanEntity[hHandle].pstChan->stFrameRateParam;
	    s32Ret = MT_SUCCESS;
	} else {
	    s32Ret = MT_FAILURE;
	}
	break;
    }

#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    case UMAPC_VDEC_CHAN_USERDATAINITBUF: {
	s32Ret = USRDATA_Alloc(hHandle, &(((VDEC_CMD_USERDATABUF_S *)arg)->stBuf));
	break;
    }

    case UMAPC_VDEC_CHAN_USERDATASETBUFADDR: {
	s32Ret = USRDATA_SetUserAddr(hHandle, ((VDEC_CMD_BUF_USERADDR_S *)arg)->u32UserAddr);
	break;
    }

    case UMAPC_VDEC_CHAN_ACQUSERDATA: {
	s32Ret = USRDATA_Acq(hHandle, &(((VDEC_CMD_USERDATA_ACQMODE_S *)arg)->stUserData),
	                     &(((VDEC_CMD_USERDATA_ACQMODE_S *)arg)->enType));
	break;
    }

    case UMAPC_VDEC_CHAN_RLSUSERDATA: {
	s32Ret = USRDATA_Rls(hHandle, &(((VDEC_CMD_USERDATA_S *)arg)->stUserData));
	break;
    }

    case UMAPC_VDEC_CHAN_RSTUSERDATABUF: {
	s32Ret = USRDATA_Reset(hHandle);
	break;
    }
#endif
    case UMAPC_VDEC_CHAN_SEEKPTS: {
	s32Ret = VDEC_SeekPTS(hHandle, ((VDEC_CMD_SEEK_PTS_S *)(arg))->pu64SeekPts, ((VDEC_CMD_SEEK_PTS_S *)(arg))->u32Gap);
	break;
    }
    case UMAPC_VDEC_GETCAP: {
	s32Ret = VDEC_GetCap((VDEC_CAP_S *)arg);
	break;
    }
    case UMAPC_VDEC_GETFB_MEM: {
	s32Ret = VDEC_GetFbMem((mmz_buffer_s *)arg);
	break;
    }
    case UMAPC_VDEC_ALLOCHANDLE: {
	s32Ret = VDEC_Chan_AllocHandle((mt_handle *)arg, filp);
	break;
    }

    case UMAPC_VDEC_FREEHANDLE: {
	s32Ret = VDEC_Chan_FreeHandle(hHandle);
	break;
    }

    case UMAPC_VDEC_CHAN_SETTRICKMODE: {
	s32Ret = MT_DRV_VDEC_SetTrickMode(hHandle, &(((VDEC_CMD_TRICKMODE_OPT_S *)arg)->stTPlayOpt));
	break;
    }

    case UMAPC_VDEC_CHAN_SETCTRLINFO: {
	s32Ret = MT_DRV_VDEC_SetCtrlInfo(hHandle, &(((VDEC_CMD_SET_CTRL_INFO_S *)arg)->stCtrlInfo));
	break;
    }
    case UMAPC_VDEC_CHAN_PROGRSSIVE: {
	s32Ret = MT_DRV_VDEC_SetProgressive(hHandle, ((VDEC_CMD_SET_PROGRESSIVE_S *)arg)->bProgressive);
	break;
    }
    case UMAPC_VDEC_CHAN_DPBFULL: {
	s32Ret = MT_DRV_VDEC_SetDPBFullCtrl(hHandle, ((VDEC_CMD_SET_DPBFULL_CTRL_S *)arg)->bDPBFullCtrl);
	break;
    }
    case UMAPC_VDEC_CHAN_LOWDELAY: {
	s32Ret = MT_DRV_VDEC_SetLowdelay(hHandle, ((VDEC_CMD_SET_LOWDELAY_S *)arg)->bLowdelay);
	break;
    }
    case UMAPC_VDEC_CHAN_SETCOLORSPACE: {
	s32Ret = MT_DRV_VDEC_SetColorSpace(hHandle, ((VDEC_CMD_SET_COLORSPACE_S *)arg)->u32ColorSpace);
	break;
    }
    case UMAPC_VDEC_CHAN_GETCRCBUF:{
		s32Ret = MT_DRV_VDEC_GetCrcBuf(hHandle, &(((VDEC_CMD_BUF_CRC_S *)arg)->stCrcBuf));
		break;
	}
	case UMAPC_VDEC_GETLCEVC_MEM:
	{
		s32Ret = VDEC_GetLcevcMem(hHandle, &(((VDEC_CMD_GET_LEVCMMZ_S *)arg)->mmz_buf));
		break;
	}
	case UMAPC_VDEC_GET_LCEVC_DATA:
	{
		s32Ret = MT_DRV_VDEC_GetLcevcData(hHandle, &(((VDEC_CMD_GET_LEVCDATA_S *)arg)->lcevcData));
		break;
	}
    case UMAPC_VDEC_CHAN_PAUSE:{
	    s32Ret = MT_DRV_VDEC_SetPause(hHandle, ((VDEC_CMD_SET_PAUSE_S *)arg)->bPauseFlag);
	    break;
    }
	case UMAPC_VDEC_CHAN_SETDECFRMTYPE:
	{
		s32Ret = MT_DRV_VDEC_SetDecFrmType(hHandle, ((VDEC_CMD_SET_DECFRMTYPE_S *)arg)->decFrmType);
		break;
	}
	case UMAPC_VDEC_CHAN_SETTRICKCFG:
	{
		s32Ret = MT_DRV_VDEC_SetTrickCfg(hHandle, ((VDEC_CMD_SET_TRICKCFG_S *)arg)->is_incomplete_stream,
			((VDEC_CMD_SET_TRICKCFG_S *)arg)->trick_mode);
		break;
	}

    //add by l00225186
    case UMAPC_VDEC_CHAN_RCVVPSSFRM: {
	//hHandle表示vpss的handle
	mt_handle hVdec = MT_INVALID_HANDLE;
	VDEC_CMD_VPSS_FRAME_S *pstVpssFrmTmp = (VDEC_CMD_VPSS_FRAME_S *)arg;
	MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrame = pstVpssFrmTmp->pstFrame;

	s32Ret = VDEC_FindVdecHandleByVpssHandle(hHandle, &hVdec);
	if (MT_SUCCESS == s32Ret) {

#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
		VDEC_CHANNEL_S *pstChan = s_stVdecDrv.astChanEntity[hVdec].pstChan;
		BUG_ON(pstChan == NULL);
		if (pstChan == NULL)
		{
			MLOGE("invalid channel(%lx)!\n",hHandle);
			s32Ret = MT_FAILURE;
			break;
		}
		pstFrame = &pstChan->stVpssFramePack;	//temply use this 'stVpssFramePack', avoid stack overflow!
#endif

	    if (s_stVdecDrv.astChanEntity[hVdec].stVpssChan.s32Speed < 0) {
			//s32Ret = VDEC_Chan_RecvVdecPortListFrame(hVdec, &(pstVpssFrmTmp->stFrame));
			s32Ret = VDEC_Chan_RecvVdecPortListFrame(hVdec, pstFrame);
	    } else {
			//s32Ret = VDEC_Chan_RecvVpssFrmBuf(hHandle, &(pstVpssFrmTmp->stFrame));
			s32Ret = VDEC_Chan_RecvVpssFrmBuf(hHandle, pstFrame);
	    }
#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
		if (MT_SUCCESS == s32Ret)
		{
			res = copy_to_user((void __user*)pstVpssFrmTmp->pstFrame, (void*)pstFrame, sizeof(MT_DRV_VIDEO_FRAME_PACKAGE_S));
			if (res != 0)
			{
				MLOGE("copy to user frame failed!\n");
				BUG();
				s32Ret = MT_FAILURE;
			}
		}
#endif
	}
	break;
    }
    case UMAPC_VDEC_CHAN_RLSPORTFRM:
    {
		VDEC_CMD_VPSS_FRAME_S *pstVpssFrmTmp = (VDEC_CMD_VPSS_FRAME_S *)arg;
		MT_DRV_VIDEO_FRAME_S *pVideoFrame = pstVpssFrmTmp->pVideoFrame;

#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
		static MT_DRV_VIDEO_FRAME_S stVidFrm;

		res = copy_from_user((void*)&stVidFrm, (void __user*)pVideoFrame, sizeof(MT_DRV_VIDEO_FRAME_S));
		if (res != 0)
		{
			MLOGE("copy frome user frame failed!\n");
			BUG();
			s32Ret = MT_FAILURE;
			break;
		}

		pVideoFrame = &stVidFrm;
#endif

		s32Ret = VDEC_Chan_VORlsFrame(hHandle, pVideoFrame);
		if (MT_SUCCESS != s32Ret) {
			MT_ERR_VDEC("VDEC release port %d frame ERR:%x!\n", s32Ret);
		}
	}
	break;
    case UMAPC_VDEC_CHAN_CREATEVPSS: {
	ulong hOuthandle = MT_INVALID_HANDLE;
	s32Ret = VDEC_Chan_CreateVpss(hHandle, &hOuthandle);
	if (s32Ret == MT_SUCCESS) {
	    *((mt_handle *)arg) = hOuthandle; //将vpss的句柄返回，供用户态代码中使用
	}
	break;
    }
    case UMAPC_VDEC_CHAN_DESTORYVPSS: {
	s32Ret = VDEC_Chan_DestroyVpss(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_CREATEPORT: {
	s32Ret = VDEC_Chan_CreatePort(hHandle, &((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, ((VDEC_CMD_VPSS_FRAME_S *)arg)->ePortAbility);
	break;
    }
    case UMAPC_VDEC_CHAN_DESTROYPORT: {
	s32Ret = VDEC_Chan_DestroyPort(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort);
	break;
    }
    case UMAPC_VDEC_CHAN_ENABLEPORT: {
	s32Ret = VDEC_Chan_EnablePort(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort);
	break;
    }
    case UMAPC_VDEC_CHAN_DISABLEPORT: {
	s32Ret = VDEC_Chan_DisablePort(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort);
	break;
    }
    case UMAPC_VDEC_CHAN_SETPORTTYPE: {
	s32Ret = VDEC_Chan_SetPortType(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, ((VDEC_CMD_VPSS_FRAME_S *)arg)->enPortType);
	break;
    }
    case UMAPC_VDEC_CHAN_CANCLEMAINPORT: {
	break;
    }
    case UMAPC_VDEC_CHAN_SETFRMPACKTYPE: {
	s32Ret = VDEC_Chan_SetFrmPackingType(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->eFramePackType);
	break;
    }
    case UMAPC_VDEC_CHAN_GETFRMPACKTYPE: {
	s32Ret = VDEC_Chan_GetFrmPackingType(hHandle, &((VDEC_CMD_VPSS_FRAME_S *)arg)->eFramePackType);
	break;
    }
    case UMAPC_VDEC_CHAN_GETPORTPARAM: {
	s32Ret = VDEC_Chan_GetPortParam(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, &((VDEC_CMD_VPSS_FRAME_S *)arg)->stPortParam);
	break;
    }
    case UMAPC_VDEC_CHAN_RESETVPSS: {
	s32Ret = VDEC_Chan_ResetVpss(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_SENDEOS: {
	s32Ret = VDEC_Chan_SendEos(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_GETPORTSTATE: {
	s32Ret = VDEC_Chan_GetPortState(hHandle, &((VDEC_CMD_VPSS_FRAME_S *)arg)->bAllPortComplete);
	break;
    }
    case UMAPC_VDEC_CHAN_GETFRMSTATUSINFO: {
	s32Ret = VDEC_Chan_GetFrmStatusInfo(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, &((VDEC_CMD_VPSS_FRAME_S *)arg)->stVdecFrmStatusInfo);
	break;
    }
    case UMAPC_VDEC_CHAN_SETPORTATTR: {
	s32Ret = VDEC_Chan_SetVpssAttr(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, &((VDEC_CMD_VPSS_FRAME_S *)arg)->stPortCfg);
	break;
    }
    case UMAPC_VDEC_CHAN_GETPORTATTR: {
	s32Ret = VDEC_Chan_GetVpssAttr(hHandle, ((VDEC_CMD_VPSS_FRAME_S *)arg)->hPort, &((VDEC_CMD_VPSS_FRAME_S *)arg)->stPortCfg);
	break;
    }
    case UMAPC_VDEC_CHAN_CREATE_VPU_BUF: {
//	s32Ret = VDEC_VPU_CreatBuffer((VDEC_CMD_VPU_BUF_CREATE_S *)arg);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_REVERT_FRAME_BUF: {
//	s32Ret = VDEC_VPU_RevertFrameBuffer((phys_addr_t *)arg);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_CREATE_FRAMELIST: {
	s32Ret = VDEC_DRV_VPU_CreateFrameList(((VDEC_CMD_CREATE_FRAME_LIST_S *)arg)->hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_RELEASE_FRAMELIST: {
	s32Ret = VDEC_DRV_VPU_ReleaseFrameList(((VDEC_CMD_RELEASE_FRAME_LIST_S *)arg)->hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_PUT_FRAME: {
	s32Ret = VDEC_DRV_VPU_PutFrame(((VDEC_CMD_VPU_PUT_FRAME_S *)arg)->hHandle, ((VDEC_CMD_VPU_PUT_FRAME_S *)arg)->stFrame);
	break;
    }
    case UMAPC_VDEC_VPU_PROC: {
	s32Ret = VDEC_DRV_VPU_Status((((VDEC_CMD_VPU_PROC_STATUS_S *)arg)->hVdecHandle), &(((VDEC_CMD_VPU_PROC_STATUS_S *)arg)->stVPUStatus));
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_ATTR: {
	s32Ret = VDEC_DRV_VPU_SetAttr(((VDEC_CMD_VPU_ATTR_S *)arg)->hHandle, ((VDEC_CMD_VPU_ATTR_S *)arg)->stVPUAttr);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_CHECK_RLSFRM: {
	s32Ret = VDEC_DRV_VPU_CheckRlsFrameID(((VDEC_CMD_VPU_CHECK_RLSFRAME_S *)arg)->hHandle, ((VDEC_CMD_VPU_CHECK_RLSFRAME_S *)arg)->as32FrameID, &(((VDEC_CMD_VPU_CHECK_RLSFRAME_S *)arg)->s32Count));
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_START: {
	s32Ret = MT_DRV_VDEC_VPU_ChanStart(hHandle);
	break;
    }
    case UMAPC_VDEC_CHAN_VPU_STOP: {
	s32Ret = MT_DRV_VDEC_VPU_ChanStop(hHandle);
	break;
    }
    case UMAPC_VDEC_VPU_PTS_Alloc: {
	s32Ret = VDEC_VPU_PTS_Alloc((((VDEC_CMD_VPU_PROC_HANDLE_S *)arg)->hVdecHandle & 0xff), ((VDEC_CMD_VPU_PROC_HANDLE_S *)arg)->hVpuHandle);
	break;
    }
    case UMAPC_VDEC_VPU_PTS_Free: {
	s32Ret = VDEC_VPU_PTS_Free((((VDEC_CMD_VPU_PROC_HANDLE_S *)arg)->hVdecHandle & 0xff), ((VDEC_CMD_VPU_PROC_HANDLE_S *)arg)->hVpuHandle);
	break;
    }
    case UMAPC_VDEC_VPU_PTS_Start: {
	s32Ret = VDEC_VPU_PTS_Start(hHandle);
	break;
    }
    case UMAPC_VDEC_VPU_PTS_Stop: {
	s32Ret = VDEC_VPU_PTS_Stop(hHandle);
	break;
    }
    case UMAPC_VDEC_VPU_PTS_Reset: {
	s32Ret = VDEC_VPU_PTS_Reset(hHandle);
	break;
    }
    case UMAPC_VDEC_VPU_GET_FRAME_RATE: {
	s32Ret = VDEC_VPU_Get_FrameRateForNewFrm(((VDEC_CMD_VO_FRAME_S *)arg)->hHandle, &(((VDEC_CMD_VO_FRAME_S *)arg)->stFrame));
	break;
    }
    case UMAPC_VDEC_VPU_GET_VPSS_STATUSINFO: {
	s32Ret = VDEC_VPU_Get_Vpss_StatusInfo(((VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S *)arg)->hHandle, &(((VDEC_CMD_VPU_GET_VPSS_STATUSINFO_S *)arg)->bAllPortCompleteFrm));
	break;
    }
    case UMAPC_VDEC_CHAN_SETEXTBUFFER: {
	s32Ret = VDEC_Chan_SetExtBuffer(hHandle, &((VDEC_CMD_VPSS_FRAME_S *)arg)->stBufferAttr);
	break;
    }
    case UMAPC_VDEC_CHAN_SETBUFFERMODE: {
	s32Ret = VDEC_Chan_SetFrameBufferMode(hHandle, ((VDEC_CMD_SET_BUFFERMODE_S *)arg)->enFrameBufferMode);
	break;
    }
    case UMAPC_VDEC_CHAN_CHECKANDDELBUFFER: {
	s32Ret = VDEC_Chan_CheckAndDelBuffer(hHandle, ((VDEC_CMD_CHECKANDDELBUFFER_S *)arg)->stBufInfo);
	break;
    }
    case UMAPC_VDEC_CHAN_SETEXTBUFFERSTATE: {
	s32Ret = VDEC_Chan_SetExtBufferState(hHandle, ((VDEC_CMD_SETEXTBUFFERTATE_S *)arg)->enExtBufferState);
	break;
    }
    case UMAPC_VDEC_CHAN_SETRESOLUTION: {
	s32Ret = VDEC_Chan_SetResolution(hHandle, ((VDEC_CMD_SETRESOLUTION_S *)arg)->stResolution);
	break;
    }
    case UMAPC_VDEC_CHAN_SETHDRINFO: {
	s32Ret = VDEC_Chan_SetHdrInfo(hHandle, ((VDEC_CMD_SET_HDRINFO_S *)arg)->stHdrInfo);
	break;
    }
    default:
	s32Ret = MT_FAILURE;
	break;
    }

    //LEAVE_FUNCTION;
    return s32Ret;
}

static mt_s32 VDEC_RegChanProc(mt_s32 s32Num)
{
    mt_char aszBuf[16];
    mt_proc_entry_t *pstItem;

    ENTER_FUNCTION;
    MLOGD("%s: Num %d\n",__FUNCTION__,s32Num);
    /* Check parameters */
    if (MT_NULL == s_stVdecDrv.pstProcParam) {
	return MT_FAILURE;
    }

    /* Create proc */
    if (-1 == s32Num) {
	snprintf(aszBuf, sizeof(aszBuf), "vdec_ctrl");
    } else {
	snprintf(aszBuf, sizeof(aszBuf), "vdec%02d", s32Num);
    }
    pstItem = mt_drv_proc_add_module(aszBuf, MT_NULL, MT_NULL);
    if (!pstItem) {
	MT_FATAL_VDEC("Create vdec proc entry fail!\n");
	return MT_FAILURE;
    }

    /* Set functions */
    if (-1 == s32Num) {
	pstItem->read = s_stVdecDrv.pstProcParam->pfnCtrlReadProc;
	pstItem->write = s_stVdecDrv.pstProcParam->pfnCtrlWriteProc;
    } else {
	pstItem->read = s_stVdecDrv.pstProcParam->pfnReadProc;
	pstItem->write = s_stVdecDrv.pstProcParam->pfnWriteProc;
    }

    MT_INFO_VDEC("Create proc entry for vdec%d OK!\n", s32Num);
    return MT_SUCCESS;
}

static mt_s32 VDEC_VPU_RegChanProc(mt_s32 s32VpuNum)
{
    mt_char aszBuf[16];
    mt_proc_entry_t *pstItem;
    ENTER_FUNCTION;
    if (MT_NULL == s_stVdecDrv.pstProcParam) {
	return MT_FAILURE;
    }
    if (-1 < s32VpuNum) {
	snprintf(aszBuf, sizeof(aszBuf), "vdec_vpu%02d", s32VpuNum);
	pstItem = mt_drv_proc_add_module(aszBuf, MT_NULL, MT_NULL);
	if (!pstItem) {
	    MT_FATAL_VDEC("Create vdec_vpu proc entry fail!\n");
	    return MT_FAILURE;
	}
	pstItem->read = s_stVdecDrv.pstProcParam->pfnVpuReadProc;
	pstItem->write = s_stVdecDrv.pstProcParam->pfnVpuWriteProc;
    }
    MT_INFO_VDEC("Create proc entry for vdec_vpu%02d OK!\n", s32VpuNum);
    return MT_SUCCESS;
}

static mt_void VDEC_UnRegChanProc(mt_s32 s32Num)
{
    mt_char aszBuf[16];

    ENTER_FUNCTION;
    if (-1 == s32Num) {
	snprintf(aszBuf, sizeof(aszBuf), "vdec_ctrl");
    } else {
	snprintf(aszBuf, sizeof(aszBuf), "vdec%02d", s32Num);
    }
    mt_drv_proc_rm_module(aszBuf);
    return;
}
static mt_void VDEC_VPU_UnRegChanProc(mt_s32 s32VpuNum)
{
    mt_char aszBuf[16];
    ENTER_FUNCTION;
    if (-1 < s32VpuNum) {
	snprintf(aszBuf, sizeof(aszBuf), "vdec_vpu%02d", s32VpuNum);
	mt_drv_proc_rm_module(aszBuf);
    }

    return;
}

//static mt_void VDEC_TimerFunc(mt_length_t value)
static mt_void VDEC_TimerFunc(struct timer_list * value)
{
    mt_handle hHandle;
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan = MT_NULL;
    VDEC_CHAN_STATINFO_S *pstStatInfo = MT_NULL;

    //ENTER_FUNCTION;

    for (hHandle = 0; hHandle < MT_VDEC_MAX_INSTANCE_NEW; hHandle++) {
	/* Lock */
	VDEC_CHAN_TRY_USE_DOWN(&s_stVdecDrv.astChanEntity[hHandle]);
	if (MT_SUCCESS != s32Ret) {
	    continue;
	}

	/* Check and get pstChan pointer */
	if (MT_NULL == s_stVdecDrv.astChanEntity[hHandle].pstChan) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    continue;
	}
	pstChan = s_stVdecDrv.astChanEntity[hHandle].pstChan;

	if (pstChan->enCurState != VDEC_CHAN_STATE_RUN) {
	    VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
	    continue;
	}

	pstStatInfo = &pstChan->stStatInfo;
	pstStatInfo->u32TotalVdecTime++;
	pstStatInfo->u32CalcBpsVdecTime++;
	pstStatInfo->u32AvrgVdecFpsLittle = (mt_u32)((pstStatInfo->u32TotalVdecOutFrame * 100) / pstStatInfo->u32TotalVdecTime);

	pstStatInfo->u32AvrgVdecFps = (mt_u32)(pstStatInfo->u32TotalVdecOutFrame / pstStatInfo->u32TotalVdecTime);
	pstStatInfo->u32AvrgVdecFpsLittle -= (pstStatInfo->u32AvrgVdecFps * 100);
	pstStatInfo->u32AvrgVdecInBps = (mt_u32)(pstStatInfo->u32TotalVdecInByte / pstStatInfo->u32CalcBpsVdecTime * 8);
	//pstStatInfo->u32AvrgVdecInBps = (mt_u32)(pstStatInfo->u32TotalVdecInByte / 1024);
	//pstStatInfo->u32TotalVdecInByte = 0;
	if (pstStatInfo->u32TotalVdecInByte > 0xFEFFFFFFUL) {
	    pstStatInfo->u32TotalVdecInByte = 0;
	    pstStatInfo->u32CalcBpsVdecTime = 0;
	}

	/* monitor and/or dump vdec es buffer stat */
	if(0 == g_VdecPause)
	{
		vdec_dump_es_stat(hHandle, pstChan);
	}
	
	VDEC_CHAN_USE_UP(&s_stVdecDrv.astChanEntity[hHandle]);
    }

	mod_timer(&s_stVdecDrv.stTimer, jiffies + HZ);
    //LEAVE_FUNCTION;
    return;
}
mt_s32 VDEC_VPSS_Init(mt_void)
{
    //mt_u32 i,j;
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    /*init vpss*/
    /*CNcomment:对vpss初始化*/
    s32Ret = (s_stVdecDrv.pVpssFunc->pfnVpssGlobalInit)();

    LEAVE_FUNCTION;
    return s32Ret;
}
//l00273086 start//
//static UINT32 s_VdecTaskSleepTimems = 2;	//not used
static UINT32 u32TaskRunFlag = 0;


extern phys_addr_t vdec_phy2dma_addr(phys_addr_t phy_addr);


static mt_s32 VDEC_PRE_Alloc_VHD_Mem(mt_u32 vchannel)
{
    mt_s32 s32Ret;
    mt_s32 s32DispSlotCnt;
    MT_BOOL bDynamicResSupport;
    MT_BOOL bHevcSupport;
    MT_BOOL bHevc10BitSupport;
    MT_BOOL bLossyCompress;
    MT_BOOL bVp9Support;
    ulong as8TmpBuf[32];
    DETAIL_MEM_SIZE         stMemSize;
    MT_BOOL b4KSupport;
#if CONFIG_SUPPORT_VDEC_DVIEW
	MT_BOOL	  down_scaler_enable;
	mt_s32	  ds_width;
	mt_s32	  ds_height;
#endif
    ENTER_FUNCTION;
    memset(as8TmpBuf, 0, sizeof(as8TmpBuf));

	if(vchannel >= MT_VDEC_MAX_INSTANCE_NEW)
	{
	    MT_ERR_VDEC("vchannel %d > MT_VDEC_MAX_INSTANCE_NEW %d !\n", vchannel , MT_VDEC_MAX_INSTANCE_NEW );
	    return MT_FAILURE;
	}
	
#if	VFMW_PARAMETER_SUPPORT_4K
	s32DispSlotCnt = 3;
#else
	s32DispSlotCnt = VFMW_PARAMETER_DISP_SLOT_COUNT;
#endif
    as8TmpBuf[2] = (ulong) & s32DispSlotCnt;

    bDynamicResSupport = VFMW_PARAMETER_DYNAMIC_RES;
    as8TmpBuf[3] = (ulong) & bDynamicResSupport;

    bHevcSupport = VFMW_PARAMETER_HEVC;
    as8TmpBuf[4] = (ulong) & bHevcSupport;

    bHevc10BitSupport = VFMW_PARAMETER_HEVC_10BIT;
    as8TmpBuf[5] = (ulong) & bHevc10BitSupport;

    bLossyCompress = VFMW_PARAMETER_LOSSY_COMPRESS;
    as8TmpBuf[6] = (ulong) & bLossyCompress;

    bVp9Support = VFMW_PARAMETER_VP9;
    as8TmpBuf[7] = (ulong) & bVp9Support;

#if CONFIG_SUPPORT_VDEC_DVIEW
#if	VFMW_PARAMETER_SUPPORT_4K
	down_scaler_enable	= 1;
	as8TmpBuf[8] = (ulong) & down_scaler_enable;
	ds_width			= 1280 ;
	as8TmpBuf[9] = (ulong) & ds_width;
	ds_height			= 720;
	as8TmpBuf[10] = (ulong) & ds_height;
#else
	down_scaler_enable	= 1;
	as8TmpBuf[8] = (ulong) & down_scaler_enable;
	ds_width			= 640 ;
	as8TmpBuf[9] = (ulong) & ds_width;
	ds_height			= 360;
	as8TmpBuf[10] = (ulong) & ds_height;
#endif
#endif
	b4KSupport = VFMW_PARAMETER_SUPPORT_4K;

	if(0 == vchannel)
	{
		b4KSupport =VFMW_PARAMETER_SUPPORT_4K;
	}
	else
	{
#ifndef CONFIG_MT_VDEC_4KPIP_SUPPORT
		b4KSupport = 0 ; 
	#if CONFIG_SUPPORT_VDEC_DVIEW
		down_scaler_enable	= 1;
		as8TmpBuf[8] = (ulong) & down_scaler_enable;
		ds_width			= 640 ;
		as8TmpBuf[9] = (ulong) & ds_width;
		ds_height			= 360;
		as8TmpBuf[10] = (ulong) & ds_height;
	#endif		
#else
    bDynamicResSupport = 0;
    as8TmpBuf[3] = (ulong) & bDynamicResSupport;
	
	bVp9Support = 0;
	as8TmpBuf[7] = (ulong) & bVp9Support;
#endif
	}

    as8TmpBuf[11] = (ulong) & b4KSupport;

    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_CHAN_DETAIL_MEMSIZE_WITH_OPTION, as8TmpBuf);
    if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("VDEC_CID_GET_CHAN_DETAIL_MEMSIZE_WITH_OPTION failed!\n");
	    return MT_FAILURE;
    }
    stMemSize = *(DETAIL_MEM_SIZE *)as8TmpBuf;

	if(stMemSize.VdhDetailMem> 263*1024*1024)
	{
		stMemSize.VdhDetailMem = 263*1024*1024;
	}

//	for(i=0; i<MT_VDEC_MAX_INSTANCE_NEW; i++)
	{
		if (g_stStaticVDHMMZ[vchannel].startPhyAddr == 0
			|| g_stStaticVDHMMZ[vchannel].startVirAddr == 0
			|| g_stStaticVDHMMZ[vchannel].size == 0)
		{
			phys_addr_t dma_addr = 0;
			unsigned char cName[24] = {0};
			snprintf(cName, sizeof(cName), "VFMW_VDH%02d", vchannel);
#ifdef VDEC_4G_ADDR_TEST
			s32Ret = mt_drv_mmz_alloc_and_map(cName, "av_4g", stMemSize.VdhDetailMem, 0, &g_stStaticVDHMMZ[vchannel]);
#else			
			if(0 == vchannel)
			{
				s32Ret = mt_drv_mmz_alloc_and_map(cName, MMZ_ZONE_AV, stMemSize.VdhDetailMem, 0, &g_stStaticVDHMMZ[vchannel]);
			}
			else
			{
				s32Ret = mt_drv_mmz_alloc_and_map(cName, MMZ_ZONE_PIP, stMemSize.VdhDetailMem, 0, &g_stStaticVDHMMZ[vchannel]);
			}
#endif
			if (MT_SUCCESS != s32Ret) {
				MT_ERR_VDEC("VDEC INIT alloc VDH MMZ err!\n");
			}
		
	    	 dma_addr = vdec_phy2dma_addr(g_stStaticVDHMMZ[vchannel].startPhyAddr);
	    	
			MT_ERR_VDEC("jace vchannel %d   ddr_size %d byte\n",  vchannel, stMemSize.VdhDetailMem);
			g_stStaticVDHMMZ[vchannel].startPhyAddr = dma_addr;
			
		}
	}
	LEAVE_FUNCTION;
    return MT_SUCCESS;
}
SINT32 KERN_VDEC_InitWithOperation(VDEC_OPERATION_S *pArgs);

static mt_s32 VDEC_OpenDev(mt_void)
{
    mt_u32 i;
    mt_s32 s32Ret;
    VDEC_OPERATION_S stOpt;

    MT_INFO_VDEC("VDEC_OpenDev: Enter\n");
    ENTER_FUNCTION;

    /* Init global parameter */
    MT_INIT_MUTEX(&s_stVdecDrv.stSem);

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
    	atomic_set(&s_stVdecDrv.astChanEntity[i].atmUseCnt, 0);
    	atomic_set(&s_stVdecDrv.astChanEntity[i].atmRlsFlag, 0);
    	init_waitqueue_head(&s_stVdecDrv.astChanEntity[i].stRlsQue);
    	s_stVdecDrv.astChanEntity[i].stVpssChan.hVdec = MT_INVALID_HANDLE;
    	s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss = MT_INVALID_HANDLE;
    	s_stVdecDrv.astChanEntity[i].pstChan = MT_NULL;  //l00273086
    	s_stVdecDrv.astChanEntity[i].u32DynamicFsEn = 0; //l00273086
    }

    /* Init buffer manager */
    s32Ret = BUFMNG_Init();
    if (MT_SUCCESS != s32Ret) {
    	MT_ERR_VDEC("BUFMNG_Init err!\n");
    	goto err0;
    }

    /* Init vfmw */
    memset(&stOpt, 0, sizeof(VDEC_OPERATION_S));
    stOpt.eAdapterType = ADAPTER_TYPE_VDEC;

    stOpt.mem_malloc = MT_NULL;
    stOpt.mem_free = MT_NULL;

#if (CFG_VFMW_ON_AVCPU == 0)

	vdec_get_fw_input_param(&stOpt.inputParam);

	stOpt.inputParam.VdecCallback = VDEC_EventHandle;

#else
    stOpt.inputParam.VdecCallback = VDEC_EventHandle;
#endif

    s32Ret = KERN_VDEC_InitWithOperation(&stOpt);

    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("Init vfmw err:%d!\n", s32Ret);
	goto err1;
    }

    /* Get vfmw capabilite */
    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_CAPABILITY, &s_stVdecDrv.stVdecCap);
    if (MT_SUCCESS != s32Ret) {
	MT_FATAL_VDEC("VFMW GET_CAPABILITY err:%d!\n", s32Ret);
	goto err2;
    }

	VDEC_PRE_Alloc_VHD_Mem(0);
#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
	VDEC_PRE_Alloc_VHD_Mem(1);
#endif
    /* Get vfmw irq */
    s32Ret = KERN_VDEC_Control(MT_INVALID_HANDLE, VDEC_CID_GET_VDEC_ISR, &vdec_decoder_irq_fun);
    if (MT_SUCCESS != s32Ret) {
	MT_FATAL_VDEC("VFMW GET_CAPABILITY err:%d!\n", s32Ret);
	goto err2;
    }

#if (CFG_VFMW_ON_AVCPU == 0)
	request_vdec_irq(VDEC_IRQ_NUM, (void *)VDEC_Decoder_Irq);
#endif

    /* Init pts recover function */
    PTSREC_Init();

/* Init CC user data function */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_Init();
#endif

    VDEC_VPSS_Init();
    /* Set global timer */
	
	timer_setup(&s_stVdecDrv.stTimer, VDEC_TimerFunc, 0);
    s_stVdecDrv.stTimer.expires = jiffies + (HZ);
    add_timer(&s_stVdecDrv.stTimer);
	
    /* Set ready flag */
    s_stVdecDrv.bReady = MT_TRUE;

//FIX Kernel Info Log: No need this task!
#if 0
    /**********************Creat Vdec task //l00273086 start*************************/
    if (MT_NULL == s_stVdecDrv.pVdecTask) //l00273086
    {
	s_stVdecDrv.pVdecTask = kthread_create(VdecTaskFunc, (VOID *)NULL, "VdecKernelThread");
	if (NULL == s_stVdecDrv.pVdecTask) {
	    MT_FATAL_VDEC("VDEC can not create thread!\n");
	    goto err2;
	}
		/* created but not run */
		u32TaskRunFlag = 2;
    }
    MT_INFO_VDEC("VDEC_OpenDev OK 0x%x.\n", s_stVdecDrv.pVdecTask);
    MLOGI("VDEC_OpenDev: task %p ready.\n", s_stVdecDrv.pVdecTask);
    /**********************Creat Vdec task //l00273086 end**************************/
#endif

    LEAVE_FUNCTION;
    return MT_SUCCESS;

err2:
    KERN_VDEC_Exit();
err1:
    BUFMNG_DeInit();
err0:
    LEAVE_FUNCTION;
    return MT_FAILURE;
}

static mt_s32 VDEC_CloseDev(mt_void)
{
    mt_u32 i, j;
    mt_s32 s32Ret = MT_SUCCESS;

    ENTER_FUNCTION;

    /* Reentrant */
    if (s_stVdecDrv.bReady == MT_FALSE) {
	return MT_SUCCESS;
    }

    /* Set ready flag */
    s_stVdecDrv.bReady = MT_FALSE;

    /* Delete timer */
    del_timer_sync(&s_stVdecDrv.stTimer);

    /*stop task*/
    if ((MT_NULL != s_stVdecDrv.pVdecTask)
    	&& (1 == u32TaskRunFlag || 2 == u32TaskRunFlag/*created but not run*/))
    {
		kthread_stop(s_stVdecDrv.pVdecTask);
		msleep(1);
		s_stVdecDrv.pVdecTask = MT_NULL;
		u32TaskRunFlag = 0;
    } //l00273086
    /* Free all channels */
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
	if (s32Ret != 0)
	{
		MLOGE("%s: down_killable failed! return %d.\n",__FUNCTION__,s32Ret);
	}

	for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
	    if (s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].hPort != MT_INVALID_HANDLE) {
		if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].bufferType) {
		    BUFMNG_VPSS_DeInit(&s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].stBufVpssInst);
		}
	    }
	}

	if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
	    VDEC_Chan_DestroyVpss(i);
	}
	up(&s_stVdecDrv.stSem);

	if (s_stVdecDrv.astChanEntity[i].bUsed) {
	    if (s_stVdecDrv.astChanEntity[i].pstChan) {
		VDEC_Chan_Free(i);
	    }
	    VDEC_Chan_FreeHandle(i);
	}
    }

#if (CFG_VFMW_ON_AVCPU == 0)
	release_vdec_irq(VDEC_IRQ_NUM);
#endif

    /* Vfmw exit */
    KERN_VDEC_Exit();

    /* Buffer manager exit  */
    BUFMNG_DeInit();

    /* Pts recover exit */
    PTSREC_DeInit();
    (s_stVdecDrv.pVpssFunc->pfnVpssGlobalDeInit)();

/* CC user data exit */
#if (1 == MT_VDEC_USERDATA_CC_SUPPORT)
    USRDATA_DeInit();
#endif

    LEAVE_FUNCTION;

    return MT_SUCCESS;
}

mt_s32 VDEC_DRV_Open(struct inode *inode, struct file *filp)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    if (atomic_inc_return(&s_stVdecDrv.atmOpenCnt) == 1) {
	s_stVdecDrv.pDmxFunc = MT_NULL;
	s_stVdecDrv.pVpssFunc = MT_NULL;

	/* Get demux functions */
	s32Ret = mt_drv_module_getfunction(MT_ID_DEMUX, (mt_void **)&s_stVdecDrv.pDmxFunc);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("Get demux function err:%#x!\n", s32Ret);
	}

	/*Get vpss functions*/
	s32Ret = mt_drv_module_getfunction(MT_ID_VPSS, (mt_void **)&s_stVdecDrv.pVpssFunc);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("Get vpss function err:%#x!\n", s32Ret);
	    goto err;
	}

	/*Get disp functions*/
	s32Ret = mt_drv_module_getfunction(MT_ID_DISP, (mt_void **)&s_stVdecDrv.pDispFunc);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("Get disp function err:%#x!\n", s32Ret);
	    goto err;
	}

	/* Init device */
	if (MT_SUCCESS != VDEC_OpenDev()) {
	    MT_FATAL_VDEC("VDEC_OpenDev err!\n");
	    goto err;
	}
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;

err:
    atomic_dec(&s_stVdecDrv.atmOpenCnt);
    return MT_FAILURE;
}

#ifdef CONFIG_MT_VDEC_LCEVC_SUPPORT
void update_lcevc_displayed_pts(mt_u32 frame_pts, mt_u32 frame_id)
{
	if(NULL == gPLcevc_pipe[0])
	{
		return ;
	}
	
	gPLcevc_pipe[0]->m_frame_id = frame_id;
	gPLcevc_pipe[0]->m_last_display_pts = frame_pts;
}
ssize_t Lcevc_data_pop (mt_handle hHandle, Lcevc_SEIpipe *dev, MT_VDEC_LCEVC_DATA_S *lcevc_data)
{
	size_t count = 32;
	
	if (mutex_lock_interruptible(&Lcevc_pipe_lock[hHandle]))
		return -ERESTARTSYS;

	if(LCEVC_RING_SIZE == dev->freesize)
	{
		mutex_unlock(&Lcevc_pipe_lock[hHandle]);
		return 0;
	}
	
	memcpy((void*)((ulong)lcevc_data),(void*)dev->rp, count);
		 
	dev->rp += count;
	dev->freesize += count;
	
	dev->pop_cnt++;
	if (dev->rp == dev->end)
		dev->rp = dev->buffer; /* wrapped */
	mutex_unlock (&Lcevc_pipe_lock[hHandle]);

	return count;
}

ssize_t Lcevc_data_push(mt_handle hHandle, Lcevc_SEIpipe *dev, MT_VDEC_LCEVC_DATA_S *lcevc_data)
{
	size_t count = 32;
	int result;

	if (mutex_lock_interruptible(&Lcevc_pipe_lock[hHandle]))
		return -ERESTARTSYS;

	/* Make sure there's space to write */
	if (0 == dev->freesize)
	{
		mutex_unlock(&Lcevc_pipe_lock[hHandle]);
		printk(KERN_ERR "error Lcevc_data_push pts %ld  dev->freesize %d  \n", 
				 (ulong)lcevc_data->PTS, dev->freesize);
		return result;
	}

	memcpy((void*)(dev->wp),(void*)(ulong)lcevc_data, count);
	
	dev->wp += count;
	dev->push_cnt++;
	dev->freesize -= count;
	dev->numReorderPics = lcevc_data->numReorderPics;
	if (dev->wp == dev->end)
		dev->wp = dev->buffer; /* wrapped */
	mutex_unlock(&Lcevc_pipe_lock[hHandle]);
	return count;
}
#else
void update_lcevc_displayed_pts(mt_u32 frame_pts, mt_u32 frame_id)
{
	return ;
}
#endif

mt_s32 VDEC_DRV_Release(struct inode *inode, struct file *filp)
{
    mt_s32 i, j;
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    //i = readl(0);
    /* Not the last close, only close the channel match with the 'filp' */
    if (atomic_dec_return(&s_stVdecDrv.atmOpenCnt) != 0) {
	for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	    if (s_stVdecDrv.astChanEntity[i].u32File == ((ulong)filp)) {
		s32Ret = DOWN_SEM(&s_stVdecDrv.stSem, __LINE__);
		if (s32Ret != 0)
		{
			MLOGE("%s: down_killable failed! return %d.\n",__FUNCTION__,s32Ret);
		}

		for (j = 0; j < VDEC_MAX_PORT_NUM; j++) {
		    if (s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].hPort != MT_INVALID_HANDLE) {
			if (MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE == s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].bufferType) {
			    BUFMNG_VPSS_DeInit(&s_stVdecDrv.astChanEntity[i].stVpssChan.stPort[j].stBufVpssInst);
			}
		    }
		}
		if (MT_INVALID_HANDLE != s_stVdecDrv.astChanEntity[i].stVpssChan.hVpss) {
		    VDEC_Chan_DestroyVpss(i);
		}
		up(&s_stVdecDrv.stSem);

		if (s_stVdecDrv.astChanEntity[i].bUsed) {
		    if (s_stVdecDrv.astChanEntity[i].pstChan) {
			if (MT_SUCCESS != VDEC_Chan_Free(i)) {
			    atomic_inc(&s_stVdecDrv.atmOpenCnt);
			    return MT_FAILURE;
			}
		    }
		    VDEC_Chan_FreeHandle(i);
		}
	    }
	}
    }
    /* Last close */
    else {
	VDEC_CloseDev();
    }

    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
		if (MT_TRUE == s_stVdecDrv.astChanEntity[i].bUsed) {
		    break;
		}
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_s32 VDEC_DRV_RegWatermarkFunc(FN_VDEC_Watermark pfnFunc)
{
    ENTER_FUNCTION;
    /* Check parameters */
    if (MT_NULL == pfnFunc) {
	return MT_FAILURE;
    }

    s_stVdecDrv.pfnWatermark = pfnFunc;
    return MT_SUCCESS;
}

mt_void VDEC_DRV_UnRegWatermarkFunc(mt_void)
{
    ENTER_FUNCTION;
    s_stVdecDrv.pfnWatermark = MT_NULL;
    return;
}

mt_s32 VDEC_DRV_RegisterProc(VDEC_REGISTER_PARAM_S *pstParam)
{
    mt_s32 i;
    mt_s32 s32Ret = MT_FAILURE;

    ENTER_FUNCTION;

    s_stVdecDrv.pstProcParam = pstParam;

    /* Create ctrl proc */
    s32Ret = VDEC_RegChanProc(-1);
    if (MT_SUCCESS != s32Ret) {
	MT_INFO_VDEC("VDEC_RegChanProc Err!\n");
    }

    /* Create proc */
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (s_stVdecDrv.astChanEntity[i].pstChan) {
	    if (MT_FALSE == s_stVdecDrv.astChanEntity[i].pstChan->bProcRegister) {
		s32Ret = VDEC_RegChanProc(i);
		if (MT_SUCCESS == s32Ret) {
		    s_stVdecDrv.astChanEntity[i].pstChan->bProcRegister = MT_TRUE;
		} else {
		    s_stVdecDrv.astChanEntity[i].pstChan->bProcRegister = MT_FALSE;
		}
	    }
	}
    }

    return MT_SUCCESS;
}

mt_void VDEC_DRV_UnregisterProc(mt_void)
{
    mt_s32 i;

    ENTER_FUNCTION;

    /* Unregister ctrl proc*/
    VDEC_UnRegChanProc(-1);

    /* Unregister */
    for (i = 0; i < MT_VDEC_MAX_INSTANCE_NEW; i++) {
	if (s_stVdecDrv.astChanEntity[i].pstChan) {
	    if (MT_TRUE == s_stVdecDrv.astChanEntity[i].pstChan->bProcRegister) {
		VDEC_UnRegChanProc(i);
		s_stVdecDrv.astChanEntity[i].pstChan->bProcRegister = MT_FALSE;
	    }
	}
    }

    /* Clear param */
    s_stVdecDrv.pstProcParam = MT_NULL;
    return;
}

mt_s32 VDEC_DRV_Suspend(basedev_s *pdev, pm_message_t state)
{
    ENTER_FUNCTION;
    //if (VDEC_Suspend())
    if (0) {
	MT_FATAL_VDEC("Suspend err!\n");
	return MT_FAILURE;
    }

    MT_PRINT("VDEC suspend OK\n");
    return MT_SUCCESS;
}

mt_s32 VDEC_DRV_Resume(basedev_s *pdev)
{
    ENTER_FUNCTION;
    //if (VDEC_Resume())
    if (0) {
	MT_FATAL_VDEC("Resume err!\n");
	return MT_FAILURE;
    }

    MT_PRINT("VDEC resume OK\n");
    return MT_SUCCESS;
}

/*this function is the interface of controlling by proc file system*/
/*CNcomment: 通过proc文件系统进行控制的函数入口*/
mt_s32 VDEC_DRV_DebugCtrl(mt_u32 u32Para1, mt_u32 u32Para2)
{
    ENTER_FUNCTION;
    MT_INFO_VDEC("Para1=0x%x, Para2=0x%x\n", u32Para1, u32Para2);
    //VCTRL_SetDbgOption(u32Para1, (mt_u8 *)&u32Para2);

    return MT_SUCCESS;
}

VDEC_CHANNEL_S *VDEC_DRV_GetChan(mt_handle hHandle)
{
    //ENTER_FUNCTION;
  if (s_stVdecDrv.bReady) {
	  if (hHandle < MT_VDEC_MAX_INSTANCE_NEW) {
	    if (s_stVdecDrv.astChanEntity[hHandle].pstChan) {
		    return s_stVdecDrv.astChanEntity[hHandle].pstChan;
	    }
	  }
  }

  MLOGW("%s: VdecDrv.bReady = %d, Handle = %ld\n", __FUNCTION__,s_stVdecDrv.bReady,hHandle);
  return MT_NULL;
}

mt_s32 VDEC_DRV_Init(mt_void)
{
    mt_s32 ret;

    ENTER_FUNCTION;

	g_reg_vdec_base = mt_get_vdec_base();
	g_reg_tsi_base = mt_get_tsi_base();

    BUFMNG_SaveInit();

    ret = mt_drv_module_register(MT_ID_VDEC, VDEC_NAME, (mt_void *)&s_stVdecDrv.stExtFunc);
    if (MT_SUCCESS != ret) {
    	MT_FATAL_VDEC("Reg module fail:%#x!\n", ret);
    	return ret;
    }

	LEAVE_FUNCTION;
    return MT_SUCCESS;
}

mt_void VDEC_DRV_Exit(mt_void)
{
    ENTER_FUNCTION;
    mt_drv_module_unregister(MT_ID_VDEC);
    return;
}

mt_s32 MT_DRV_VDEC_Init(mt_void)
{
    ENTER_FUNCTION;
    return VDEC_DRV_Init();
}

mt_void MT_DRV_VDEC_DeInit(mt_void)
{
    ENTER_FUNCTION;
    return VDEC_DRV_Exit();
}

mt_s32 MT_DRV_VDEC_Open(mt_void)
{
    ENTER_FUNCTION;
    return VDEC_DRV_Open(MT_NULL, MT_NULL);
}

mt_s32 MT_DRV_VDEC_Close(mt_void)
{
    ENTER_FUNCTION;
    return VDEC_DRV_Release(MT_NULL, MT_NULL);
}

mt_s32 MT_DRV_VDEC_AllocChan(mt_handle *phHandle, MT_UNF_AVPLAY_OPEN_OPT_S *pstCapParam)
{
    mt_s32 s32Ret;
    mt_handle hVpss = MT_INVALID_HANDLE;

    ENTER_FUNCTION;
    s32Ret = VDEC_Chan_AllocHandle(phHandle, (struct file *)MCE_INVALID_FILP);
    if (MT_SUCCESS != s32Ret) {
	return s32Ret;
    }

    s32Ret = VDEC_Chan_CreateVpss((*phHandle) & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_Chan_CreateVpss err!\n");
	return MT_FAILURE;
    }
    //return VDEC_Chan_Alloc((*phHandle)&0xff, pstCapParam);
    return VDEC_Chan_InitParam((*phHandle) & 0xff, pstCapParam);
}

mt_s32 MT_DRV_VDEC_FreeChan(mt_handle hHandle)
{
    mt_s32 s32Ret;

    ENTER_FUNCTION;
    s32Ret = VDEC_Chan_Free(hHandle & 0xff);
    s32Ret |= VDEC_Chan_FreeHandle(hHandle & 0xff);
    s32Ret |= VDEC_Chan_DestroyVpss(hHandle & 0xff);

    return s32Ret;
}
mt_s32 MT_DRV_VDEC_CreateStrmBuf(mt_handle hVdec, MT_DRV_VDEC_STREAM_BUF_S *pstBuf)
{
    mt_s32 s32Ret;
    ENTER_FUNCTION;
    s32Ret = VDEC_CreateStrmBuf(pstBuf);
    VDEC_Chan_AttachStrmBuf(hVdec, pstBuf->u32Size, MT_INVALID_HANDLE, pstBuf->hHandle);
    return s32Ret;
}
mt_s32 MT_DRV_VDEC_ChanBufferInit(mt_handle hHandle, mt_u32 u32BufSize, mt_handle hDmxVidChn)
{
    mt_s32 s32Ret;
    MT_DRV_VDEC_STREAM_BUF_S stBuf;

    ENTER_FUNCTION;
    if (MT_INVALID_HANDLE == hDmxVidChn) {
    	stBuf.u32Size = u32BufSize;
    	s32Ret = MT_DRV_VDEC_CreateStrmBuf(hHandle & 0xff, &stBuf);
    	if (MT_SUCCESS != s32Ret) {
    	    return MT_FAILURE;
    	}
    } else {
    	s32Ret = VDEC_Chan_AttachStrmBuf(hHandle & 0xff, u32BufSize, hDmxVidChn, MT_INVALID_HANDLE);
    }
    return s32Ret;
}

mt_s32 MT_DRV_VDEC_ChanBufferDeInit(mt_handle hHandle)
{
    mt_s32 s32Ret = MT_FAILURE;
    mt_handle hDemuxHandle;
    ENTER_FUNCTION;
    if ((hHandle & 0xff) > MT_VDEC_MAX_INSTANCE_NEW - 1) {
	MT_ERR_VDEC("MT_DRV_VDEC_ChanBufferDeInit error hHandle:%d wrong!\n", hHandle);
	return s32Ret;
    }
    hDemuxHandle = s_stVdecDrv.astChanEntity[hHandle & 0xff].pstChan->hDmxVidChn;

    if (MT_INVALID_HANDLE == hDemuxHandle) {
	s32Ret = VDEC_DestroyStrmBuf(s_stVdecDrv.astChanEntity[hHandle & 0xff].pstChan->hStrmBuf);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_VDEC("VDEC_DestroyStrmBuf error!\n");
	    return s32Ret;
	}
    }
    s32Ret = VDEC_Chan_DetachStrmBuf(hHandle & 0xff);
    return s32Ret;
}

mt_s32 MT_DRV_VDEC_ResetChan(mt_handle hHandle)
{
    ENTER_FUNCTION;
    return VDEC_Chan_Reset(hHandle & 0xff, MT_NULL);
}

mt_s32 MT_DRV_VDEC_SetPortType(mt_handle hVdec, mt_handle hPort, VDEC_PORT_TYPE_E enPortType)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_SetPortType(hVpss, hPort, enPortType);
}

mt_s32 MT_DRV_VDEC_EnablePort(mt_handle hVdec, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_EnablePort(hVpss, hPort);
}

mt_s32 MT_DRV_VDEC_CreatePort(mt_handle hVdec, mt_handle *phPort, VDEC_PORT_ABILITY_E ePortAbility)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_CreatePort(hVpss, phPort, ePortAbility);
}

mt_s32 MT_DRV_VDEC_DestroyPort(mt_handle hVdec, mt_handle hPort)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_DestroyPort(hVpss, hPort);
}

mt_s32 MT_DRV_VDEC_GetPortParam(mt_handle hVdec, mt_handle hPort, VDEC_PORT_PARAM_S *pstPortParam)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_GetPortParam(hVpss, hPort, pstPortParam);
}
mt_s32 MT_DRV_VDEC_Chan_RecvVpssFrmBuf(mt_handle hVdec, MT_DRV_VIDEO_FRAME_PACKAGE_S *pstFrm)
{
    mt_s32 s32Ret;
    mt_handle hVpss;
    ENTER_FUNCTION;
    s32Ret = VDEC_FindVpssHandleByVdecHandle(hVdec & 0xff, &hVpss);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle err!\n");
	return MT_FAILURE;
    }
    return VDEC_Chan_RecvVpssFrmBuf(hVpss, pstFrm);
}
mt_s32 MT_DRV_VDEC_SetChanFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    ENTER_FUNCTION;
    return PTSREC_GetFrmRate(hHandle, pstFrmRate);
}
mt_s32 MT_DRV_VDEC_GetChanFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    ENTER_FUNCTION;
    return PTSREC_SetFrmRate(hHandle, pstFrmRate);
}
mt_void MT_DRV_VDEC_GetVcmpFlag(MT_BOOL *pbVcmpFlag)
{
    ENTER_FUNCTION;
    if (1 == EnVcmp) {
	*pbVcmpFlag = MT_TRUE;
    } else {
	*pbVcmpFlag = MT_FALSE;
    }
    MLOGI("%s: VCMP Flag = %d\n",__FUNCTION__,*pbVcmpFlag);
}

mt_s32 MT_DRV_VDEC_DestroyStrmBuf(mt_handle hHandle)
{
    mt_s32 s32Ret;
    ENTER_FUNCTION;
    s32Ret = VDEC_DestroyStrmBuf(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_DestroyStrmBuf err!\n");
	return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_Chan_AttachStrmBuf(mt_handle hHandle, mt_u32 u32BufSize, mt_handle hDmxVidChn, mt_handle hStrmBuf)
{
    mt_s32 s32Ret;
    ENTER_FUNCTION;
    s32Ret = VDEC_Chan_AttachStrmBuf(hHandle, u32BufSize, hDmxVidChn, hStrmBuf);
    if (MT_SUCCESS != s32Ret) {
    	MT_ERR_VDEC("VDEC_Chan_AttachStrmBuf err!\n");
    	return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_Chan_DetachStrmBuf(mt_handle hHandle)
{
    mt_s32 s32Ret;
    s32Ret = VDEC_Chan_DetachStrmBuf(hHandle);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_Chan_DetachStrmBuf err!\n");
	return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_GetCap(VDEC_CAP_S *pstCap)
{
    mt_s32 s32Ret;
    ENTER_FUNCTION;
    s32Ret = VDEC_GetCap(pstCap);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_VDEC("VDEC_GetCap err!\n");
	return s32Ret;
    }
    return MT_SUCCESS;
}

VDEC_CHANNEL_S *MT_DRV_VDEC_DRV_GetChan(mt_handle hHandle)
{
    VDEC_CHANNEL_S *stVdecChn = MT_NULL;
    ENTER_FUNCTION;
    stVdecChn = VDEC_DRV_GetChan(hHandle);
    if (MT_NULL == stVdecChn) {
	MT_ERR_VDEC("VDEC_DRV_GetChan err!\n");
	return MT_NULL;
    }
    return stVdecChn;
}
mt_s32 MT_DRV_VDEC_RegisterEventCallback(mt_handle hHandle, EventCallBack fCallback)
{
    mt_handle hHnd = hHandle & 0xff;
    ENTER_FUNCTION;
    if (hHnd >= MT_VDEC_MAX_INSTANCE) {
	MT_ERR_VDEC("bad handle %d!\n", hHnd);
	return MT_FAILURE;
    }
    s_stVdecDrv.astChanEntity[hHnd].eCallBack = fCallback;
    return MT_SUCCESS;
}

mt_s32 MT_DRV_VDEC_RegisterDmxHdlCallback(mt_u32 hHandle, GetDmxHdlCallBack fCallback, mt_u32 dmxID)
{
    mt_handle hHnd = hHandle & 0xff;
    ENTER_FUNCTION;
    if (hHnd >= MT_VDEC_MAX_INSTANCE) {
	MT_ERR_VDEC("bad handle %d!\n", hHnd);
	return MT_FAILURE;
    }
    s_stVdecDrv.astChanEntity[hHnd].pstChan->u32DmxID = dmxID;
    s_stVdecDrv.astChanEntity[hHnd].DmxHdlCallBack = fCallback;
    return MT_SUCCESS;
}

void MT_DRV_VDEC_Pause(mt_u32 Paused)	 
{	 
    if(Paused)	 
		g_VdecPause = 0;	 
    else	 
		g_VdecPause = 1;	 

}

module_param(RefFrameNum, int, S_IRUGO);
module_param(DispFrameNum, int, S_IRUGO);
module_param(EnVcmp, int, S_IRUGO);
module_param(En2d, int, S_IRUGO);

EXPORT_SYMBOL(MT_DRV_VDEC_GetChanStatusInfo);
EXPORT_SYMBOL(MT_DRV_VDEC_Open);
EXPORT_SYMBOL(MT_DRV_VDEC_ResetChan);
EXPORT_SYMBOL(MT_DRV_VDEC_GetEsBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_PutEsBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_AllocChan);
EXPORT_SYMBOL(MT_DRV_VDEC_SetChanAttr);
EXPORT_SYMBOL(MT_DRV_VDEC_DestroyPort);
EXPORT_SYMBOL(MT_DRV_VDEC_GetPortParam);
EXPORT_SYMBOL(MT_DRV_VDEC_CreatePort);
EXPORT_SYMBOL(MT_DRV_VDEC_Chan_RecvVpssFrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_GetChanStreamInfo);
EXPORT_SYMBOL(MT_DRV_VDEC_ChanStop);
EXPORT_SYMBOL(MT_DRV_VDEC_ChanStart);
EXPORT_SYMBOL(MT_DRV_VDEC_FreeChan);
EXPORT_SYMBOL(MT_DRV_VDEC_RecvFrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_GetChanAttr);
EXPORT_SYMBOL(MT_DRV_VDEC_SetPortType);
EXPORT_SYMBOL(MT_DRV_VDEC_EnablePort);
EXPORT_SYMBOL(MT_DRV_VDEC_Close);
EXPORT_SYMBOL(MT_DRV_VDEC_CreateStrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_ChanBufferInit);
EXPORT_SYMBOL(MT_DRV_VDEC_ChanBufferDeInit);
EXPORT_SYMBOL(MT_DRV_VDEC_DestroyStrmBuf);
//EXPORT_SYMBOL(VDEC_CreateStrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_Chan_AttachStrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_Chan_DetachStrmBuf);
EXPORT_SYMBOL(MT_DRV_VDEC_GetCap);
EXPORT_SYMBOL(MT_DRV_VDEC_DRV_GetChan);
EXPORT_SYMBOL(MT_DRV_VDEC_RegisterEventCallback);
EXPORT_SYMBOL(MT_DRV_VDEC_RegisterDmxHdlCallback);
EXPORT_SYMBOL(VDEC_phys_to_virt);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
