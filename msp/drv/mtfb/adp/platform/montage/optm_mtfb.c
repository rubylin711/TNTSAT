/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef MT_BUILD_IN_BOOT
#include <linux/string.h>
#include <linux/fb.h>

#include <linux/time.h>
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
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/workqueue.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_module.h"
#include "mt_drv_module.h"
#include "drv_disp_ext.h"
#include "drv_display.h"
#else
#include "mtfb_debug.h"
#ifndef MT_PQ_V1_0
#include "mt_drv_pq.h"
#endif
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "drv_disp_Symphony6_reg.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "optm_hal_sym.h"
#endif
#include "hd_enc_aria_reg.h"
#include "optm_mtfb.h"
#include "optm_define.h"
#include "mt_drv_mmz.h"
#include "mt_drv_disp.h"
#include "mt_drv_sys.h"
#include "../../../../osd/drv_osd_intf.h"

/******************************************************************
  definitions of functional switches
 ******************************************************************/
#ifndef MT_BUILD_IN_BOOT
static DISP_EXPORT_FUNC_S *ps_DispExportFuncs = MT_NULL;
#endif

#ifdef CFG_MTGO_PROC_SUPPORT

extern MT_MTGO_PROC_INFO_S g_stMtgoProcInfo;
#endif

#define CFG_MTFB_S40V2_PALNTSC_BUG

//#define  OPTM_MTFB_DEBUG

#if 0
#ifdef MT_BUILD_IN_BOOT

#ifdef OPTM_MTFB_DEBUG
#define MTFB_ERROR printf
#define MTFB_WARNING printf
#define MTFB_INFO(fmt...)
#define MTFB_FATAL printf
#else
#define MTFB_ERROR(fmt...)
#define MTFB_WARNING(fmt...)
#define MTFB_INFO(fmt...)
#define MTFB_FATAL(fmt...)
#endif

#else

#ifdef OPTM_MTFB_DEBUG
#define MTFB_ERROR printk
#define MTFB_WARNING printk
//#define MTFB_INFO(fmt...)
#define MTFB_INFO printk
#define MTFB_FATAL printk
#else
#define MTFB_ERROR(fmt...)
#define MTFB_WARNING(fmt...)
#define MTFB_INFO(fmt...)
#define MTFB_FATAL(fmt...)
#endif

#endif
#endif

#ifdef OPTM_MTFB_DEBUG
#define OPTM_FUN_IN printk("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define OPTM_FUN_OUT printk("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define OPTM_LOG printk
#define OPTM_LINE printk("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#else
#define DUMP_LOG \
    do {         \
    } while (0);
#define OPTM_FUN_IN DUMP_LOG
#define OPTM_FUN_OUT DUMP_LOG
#define OPTM_LOG(...) DUMP_LOG
#define OPTM_LINE DUMP_LOG
#endif

#ifndef MT_BUILD_IN_BOOT
/* wait v blanking */
#define OPTM_MTFB_WVM_ENABLE 1

/*  call-back after registers' update */
#define OPTM_MTFB_GFXRR_ENABLE 1
#endif

#define OPTM_MAX_LOGIC_MTFB_LAYER ((mt_u32)MTFB_LAYER_ID_BUTT)

#define DispWidth_HD 1280
#define DispHeight_HD 720

#define DispWidth_SD 720
#define DispHeight_SD 576

#define OPTM_ENABLE 1
#define OPTM_DISABLE 0

#define OPTM_COLOR_DEFVALUE 50

#define OPTM_EXTRACTLINE_RATIO 4
#define OPTM_EXTRACTLINE_WIDTH 1080
#define OPTM_MASTER_GPID OPTM_GFX_GP_0
#define OPTM_SLAVER_GPID OPTM_GFX_GP_1
#define OPTM_SLAVER_LAYERID MTFB_LAYER_SD_0
#define OPTM_CURSOR_LAYERID MTFB_LAYER_SD_1

#ifndef MT_BUILD_IN_BOOT
#define OPTM_WBCBUFFER_NUM 2
#else
#define OPTM_WBCBUFFER_NUM 1
#endif

#define OPTM_MMZ_ZONE NULL
/******************************************************************
  definitions of data format
 ******************************************************************/
typedef union _OPTM_GFX_UP_FLAG_U
{
    /*  Define the struct bits */
    struct
    {
        unsigned int RegUp : 1;     /*  [0] */
        unsigned int Enable : 1;    /*  [1] */
        unsigned int InRect : 1;    /*  [2] */
        unsigned int OutRect : 1;   /*  [3] */
        unsigned int Alpha : 1;     /*  [4] */
        unsigned int PreMute : 1;   /*  [5] */
        unsigned int WbcMode : 1;   /*  [6] */
        unsigned int Reserved : 25; /*  [31...7] */
    } bits;

    /*  Define an unsigned member */
    unsigned int u32;
} OPTM_GFX_UP_FLAG_U;

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
typedef enum tagOPTM_CMP_STATUS_E {
    OPTM_CMP_STATUS_STANDBY = 0x0,     /** hardware was ready to work*/
    OPTM_CMP_STATUS_PARALYSED,         /** hardware unable to work*/
    OPTM_CMP_STATUS_COMPRESSIONABLE,   /** hardware can be setted to compress*/
    OPTM_CMP_STATUS_COMPRESSING,       /** hardware was compressing*/
    OPTM_CMP_STATUS_COMPRESSFINISHED,  /** hardware has finished compression*/
    OPTM_CMP_STATUS_DECOMPRESSIONABLE, /** hardware can be setted to decompress*/
    OPTM_CMP_STATUS_DECOMPRESSING,     /** hardware was decompressing*/
    OPTM_CMP_STATUS_BUTT,
} OPTM_CMP_STATUS_E;

typedef struct tagOPTM_GFX_CMP_S
{
    MT_BOOL bUseCompress;
    MTFB_CMP_MODE_E enCMPMode;
    OPTM_CMP_STATUS_E enStatus;
    mmz_buffer_s stCMPBuffer_A;
    mmz_buffer_s stCMPBuffer_R;
    mmz_buffer_s stCMPBuffer_G;
    mmz_buffer_s stCMPBuffer_B;

} OPTM_GFX_CMP_S;
#endif
typedef struct tagOPTM_GFX_LAYER_S
{
    MT_BOOL bOpened;
    MT_BOOL bMaskFlag;
    MT_BOOL bSharpEnable;
    MT_BOOL bExtractLine;
    /*******backup hardware data of gfx*********/
    MT_BOOL bEnable;
    MT_BOOL b3DEnable;
    mt_s32 s32Depth;
    MT_BOOL bPreMute;
    mt_u32 u32ZOrder;
    MT_BOOL bCmpOpened;
    MT_BOOL bCmpCfg; /*use compression*/
    MT_BOOL bBufferChged;
    mt_u32 s32BufferChgCount;
    mt_u32 NoCmpBufAddr;
    mt_u32 u32TriDimAddr;
    mt_u16 Stride;    /* no compression mode stride*/
    mt_u16 CmpStride; /* compression mode stride     */
    MTFB_COLOR_FMT_E enDataFmt;
    MTFB_RECT stInRect; /*Inres of gfx*/
    MTFB_ALPHA_S stAlpha;
    MTFB_COLORKEYEX_S stColorkey;
    MTFB_STEREO_MODE_E enTriDimMode;
    OPTM_VDP_BKG_S stBkg;
    OPTM_VDP_GFX_BITEXTEND_E enBitExtend;
    OPTM_VDP_DATA_RMODE_E enReadMode;
    OPTM_VDP_DATA_RMODE_E enUpDateMode;
    /*****************end**********************/

    OPTM_VDP_LAYER_GFX_E enGfxHalId; /*the gfx's hal id*/
    OPTM_GFX_GP_E enGPId;            /*which gp the gfx belong to*/

    OPTM_CSC_STATE_E CscState;

    volatile mt_u32 vblflag;
    wait_queue_head_t vblEvent;
    mmz_buffer_s stCluptTable;
    mmz_buffer_s stRgnHeaderBuf;
    OPTM_OSD_HEADER_S stRgnHeader;

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
    OPTM_GFX_CMP_S stCmpInfo;
#endif
} OPTM_GFX_LAYER_S;

typedef enum tagOPTM_WBC_MODE_E {
    OPTM_WBC_MODE_MONO = 0x0,
    OPTM_WBC_MODE_LFET_EYE = 0x2,
    OPTM_WBC_MODE_RIGHT_EYE = 0x3,
    OPTM_WBC_MODE_BUTT,
} OPTM_WBC_MODE_E;

typedef enum tagOPTM_VDP_CONNECT_E {
    OPTM_VDP_CONNECT_G3_DHD0 = 0,
    OPTM_VDP_CONNECT_G3_DHD1,
    OPTM_VDP_CONNECT_BUTT,
} OPTM_VDP_CONNECT_E;

typedef enum tagOPTM_WBC_BUFFER_STATUS_E {
    OPTM_WBC_BUFFER_RELEASED = 0,
    OPTM_WBC_BUFFER_ACQUIRED
} OPTM_WBC_BUFFER_STATUS_E;

typedef struct tagOPTM_GFX_WBC_S
{
    MT_BOOL bOpened;
    MT_BOOL bEnable;

    OPTM_VDP_LAYER_WBC_E enWbcHalId;
    /* setting */
    mt_s32 s32BufferWidth;
    mt_s32 s32BufferHeight;
    mt_u32 u32BufferStride;
    mt_u32 u32BufIndex;
    mt_s32 s32WbcCnt;
    mt_u32 u32WBCBuffer[OPTM_WBCBUFFER_NUM];
    OPTM_WBC_BUFFER_STATUS_E enWBCBufferStatus[OPTM_WBCBUFFER_NUM];
    mt_u32 u32WriteBufAddr;
    mt_u32 u32ReadBufAddr;
    mmz_buffer_s stFrameBuffer;

    mt_u32 u32DataPoint; /* 0, feeder; others, reserve */

    MTFB_COLOR_FMT_E enDataFmt;

    MTFB_RECT stInRect;
    MT_BOOL bInProgressive;
    MTFB_RECT stOutRect;
    MT_BOOL bOutProgressive;
    mt_u32 u32BtmOffset;
    MT_BOOL bHdDispProgressive;
    OPTM_VDP_DITHER_E enDitherMode;
    OPTM_VDP_WBC_OFMT_E stWBCFmt;
    OPTM_VDP_DATA_RMODE_E enReadMode;
    OPTM_WBC_MODE_E enWbcMode;
    OPTM_VDP_INTMSK_E enWbcInt;
} OPTM_GFX_WBC_S;

/* display ID */
typedef enum tagOPTM_DISPCHANNEL_E {
    OPTM_DISPCHANNEL_0 = 0, //gfx4,gfx5
    OPTM_DISPCHANNEL_1,     //gfx0,gfx1,gfx2,gfx3
    OPTM_DISPCHANNEL_BUTT
} OPTM_DISPCHANNEL_E;

#ifndef MT_BUILD_IN_BOOT
typedef struct tagOPTM_GFX_WORK_S
{
    mt_u32 u32Data;
    struct work_struct work;
} OPTM_GFX_WORK_S;
#endif

/*!
  This structure defines coeff tables.
  */
typedef struct
{
    /*!
      The coeff table physical address.
      */
    phys_addr_t coeff_phy_addr;
    /*!
      The coeff table kernel vitual address.
      */
    ulong coeff_kvir_addr;
    /*!
      The coeff table size.
      */
    mt_u32 coeff_size;
} OPTM_COEFF_S;

typedef struct tagOPTM_GFX_GP_S
{
    /*Frame format of Output: 0-field; 1-frame*/
    MT_BOOL bOpen;     //the flag of gp initial
    MT_BOOL b3DEnable; // 3D flag
    MT_BOOL bMaskFlag;
    MT_BOOL bBGRState;
    MT_BOOL bInterface;
    MT_BOOL bGpClose;
    MT_BOOL bRecoveryInNextVT;
    /*wether need to extract line or not*/
    MT_BOOL bNeedExtractLine;
    /*gp_in size setted by usr*/
    MT_BOOL bGPInSetbyusr;
    /*gp_in size got initial by the first opened layer */
    MT_BOOL bGPInInitial;
    /*disp initial*/
    MT_BOOL bDispInitial;

    mt_rect_s stInRect;
    mt_rect_s stOutRect;

    MTFB_STEREO_MODE_E enTriDimMode;

    /*pq parameter*/
    mt_u32 u32ZmeDeflicker;

    /*  about color  */
    OPTM_COLOR_SPACE_E enInputCsc;
    OPTM_COLOR_SPACE_E enOutputCsc;
    OPTM_GFX_CSC_PARA_S stCscPara;

    OPTM_VDP_LAYER_GP_E enGpHalId;
    OPTM_DISPCHANNEL_E enDispCh;

    OPTM_GFX_UP_FLAG_U unUpFlag;
    /*declare work queue to open slv layer*/
#ifndef MT_BUILD_IN_BOOT
    struct workqueue_struct *queue;
    OPTM_GFX_WORK_S stOpenSlvWork;
    OPTM_GFX_WORK_S st3DModeChgWork;
#endif

    /***save for disp change and suspend***/
    mt_u32 u32Prior;
    mt_u32 u32Alpha;
    OPTM_VDP_DATA_RMODE_E enReadMode;
    OPTM_VDP_BKG_S stBkg;
    OPTM_VDP_CBM_MIX_E enMixg;

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    /*  The coeff tables.  */
    mmz_buffer_s stCoeffTableBuf;
    OPTM_COEFF_S stCoeffTable[SCALE_COEFF_TABLE_MAX];
#endif
} OPTM_GFX_GP_S;

typedef struct tagOPTM_GFX_IRQ_S
{
    ulong u32Param0;
    ulong u32Param1;
    IntCallBack pFunc;
} OPTM_GFX_IRQ_S;

typedef struct tagOPTM_GFX_CALLBACK_S
{
    /*each bit: 0---no irq,1---irq
      0---MTFB_CALLBACK_TYPE_VO
      1---MTFB_CALLBACK_TYPE_3DMode_CHG
      2---MTFB_CALLBACK_TYPE_REGUP
      3---MTFB_CALLBACK_TYPE_FRAME_START
      4---MTFB_CALLBACK_TYPE_FRAME_END
      */
    mt_u32 u32CTypeFlag;
    OPTM_GFX_IRQ_S stGfxIrq[MTFB_CALLBACK_TYPE_BUTT];
} OPTM_GFX_CALLBACK_S;

#define OPTM_GP_MAXGFXCOUNT (MTFB_LAYER_ID_BUTT)
typedef struct tagOPTM_GP_IRQ_S
{
    /*Gp only need to register callback func to disp once*/
    MT_BOOL bRegistered[MT_DRV_DISP_C_TYPE_BUTT];

    OPTM_GFX_CALLBACK_S stGfxCallBack[OPTM_GP_MAXGFXCOUNT];
} OPTM_GP_IRQ_S;

typedef struct
{
    mt_u32  refreshSyncFlag;
    wait_queue_head_t    refreshSyncEvent;

     mt_u32  vsync_flag;
    wait_queue_head_t    vsync_wait;
}OPTM_SYNC_INFO_S;

static OPTM_SYNC_INFO_S s_Refresh;

/******************************************************************
  capacity set definitions
 ******************************************************************/
const MTFB_CAPABILITY_S g_stGfxCap[OPTM_MAX_LOGIC_MTFB_LAYER] =
{
    /* BG */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_FALSE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },


    /* OSD0 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 1,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 1,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* OSD1 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 1,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 1,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* SUB */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 1,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 1,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* STILL */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0 },
        .bVoScale = MT_FALSE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* HD0 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 1,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 0,
        .bStereo = 1,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* HD1 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* HD2 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* HD3 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* SD0 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 1,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_TRUE,
        .bCompression = 0,
        .bStereo = 1,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* SD1 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 1,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_FALSE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 1,
        .u32MaxWidth = 2560,
        .u32MaxHeight = 1600,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* SD2 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_FALSE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* SD3 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_FALSE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 0,
        .bStereo = 0,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },
#ifndef MT_BUILD_IN_BOOT
    /* AD0 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 1,
        .bStereo = 1,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* AD1 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 1,
        .bStereo = 1,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* AD2 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 1,
        .bStereo = 1,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* AD3 */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 1,
        .bStereo = 1,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },

    /* CURSOR */
    {
        .bKeyAlpha = 1,
        .bGlobalAlpha = 1,
        .bCmap = 0,
        .bHasCmapReg = 0,
        .bColFmt = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 27
        .bVoScale = MT_TRUE,
        .bLayerSupported = MT_FALSE,
        .bCompression = 1,
        .bStereo = 1,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32MinWidth = 0,
        .u32MinHeight = 0,
        .u32VDefLevel = 0, /* not surpport */
        .u32HDefLevel = 0, /* not surpport */
    },
#endif
};

/******************************************************************
  definitions of global variables
 ******************************************************************/
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
static mt_u8 scale_coeff_table[SCALE_COEFF_TABLE_MAX * SINGLE_SCALE_COEFF_TABLE_SIZE] =
{
#include "disp_scale_coef_aria.h"
};
#endif
#if 0
static mt_u8 g_TempScaleFilter[SINGLE_SCALE_COEFF_TABLE_SIZE];
static mt_u8 * p16ByteSrc = MT_NULL;
static mt_u8 * p16ByteDst = MT_NULL;
#define SCALE_FILTER_ENDIAN_SWITCH_SIZE 16
#endif

ulong p_optm_addr = 0;


static MT_BOOL b_osd_force_update = FALSE;
static mt_u32 g_u32GFXInitFlag = 0;
//static mt_u32 g_u32SlvLayerInitFlag = 0;
//static mt_u32 g_u32DispInitFlag[OPTM_GFX_GP_BUTT];

static MT_BOOL g_bTcWBCFlag = MT_FALSE;
static MT_BOOL g_bGpCoefChangeFlag = MT_FALSE;

/* WORKMODE */
static MTFB_GFX_MODE_EN g_enOptmGfxWorkMode = MTFB_GFX_MODE_NORMAL;

/* gfx0,gfx1,gfx2,gfx3,gfx4,gfx5 */
static OPTM_GFX_LAYER_S g_stGfxDevice[OPTM_MAX_LOGIC_MTFB_LAYER];

/*graphics process device gp0 and gp1*/
/*gp0: process gfx0,gfx1,gfx2,gfx3*/
/*gp1: process --------gfx4,gfx5*/
static OPTM_GFX_GP_S g_stGfxGPDevice[OPTM_GFX_GP_BUTT];
/*save irq info of each gfx*/
static OPTM_GP_IRQ_S g_stGfxGPIrq[OPTM_GFX_GP_BUTT];
static OPTM_GFX_WBC_S g_stGfxWbc2;
//static OPTM_GFX_WBC_S  g_stGfxWbc3;
#define OPTM_GP0_GFX_COUNT 5
#define OPTM_GP1_GFX_COUNT 0

#define OPTM_GFX_WBC_WIDTH (g_bTcWBCFlag ? 1920 : 720)
#define OPTM_GFX_WBC_HEIGHT (g_bTcWBCFlag ? 1200 : 576)

#define OPTM_GFXCLUT_LENGTH 256
#define OPTM_GFXDATA_DEFAULTBYTES 4

#define OPTM_CMAP_SIZE 0x400 /*unit:KB 256*4*/

#define OPTM_HEAD_SIZE 64

#define OPTM_COMPRESS_SIZE (((1280 * 720 * 17)+15) / 16)


/******************************************************************
  macro definitions
 ******************************************************************/

#define D_OPTM_MTFB_CheckGfxOpen(enLayerId)                    \
    \
do                                                      \
{                                                          \
    if (enLayerId >= MTFB_LAYER_ID_BUTT) {                 \
        MTFB_ERROR("no suppout Gfx%d!\n", enLayerId);      \
        return MT_FAILURE;                                 \
    }                                                      \
    if (g_stGfxDevice[enLayerId].bOpened != MT_TRUE) {     \
        MTFB_ERROR("Error! Gfx%d not open!\n", enLayerId); \
        return MT_FAILURE;                                 \
    }                                                      \
    \
}                                                       \
while (0)

/**check gp mask flag,return success when it's true, or else continue*/
#define OPTM_CheckGPMask_BYLayerID(u32LayerID)                             \
    do \
{                                                                \
    if (g_stGfxGPDevice[g_stGfxDevice[u32LayerID].enGPId].bMaskFlag) { \
        return MT_SUCCESS;                                             \
    }                                                                  \
    \
} while (0)

#define OPTM_CheckGPMask_BYGPID(enGPId)          \
    do \
{                                      \
    if (g_stGfxGPDevice[enGPId].bMaskFlag) { \
        return MT_SUCCESS;                   \
    }                                        \
    \
} while (0)

#define IS_MASTER_GP(enGpId) ((g_enOptmGfxWorkMode == MTFB_GFX_MODE_HD_WBC) && (enGpId == OPTM_GFX_GP_0))
#define IS_SLAVER_GP(enGpId) ((g_enOptmGfxWorkMode == MTFB_GFX_MODE_HD_WBC) && (enGpId == OPTM_GFX_GP_1))

/******************************************************************
  function definitions
 ******************************************************************/
#ifdef OPTM_MTFB_WVM_ENABLE
mt_void OPTM_GfxWVBCallBack(mt_u32 enLayerId, mt_u32 u32Param1);
#endif
mt_void OPTM_GfxWaitRRCallBack(mt_u32 enLayerId, mt_u32 u32Param1);
mt_void OPTM_Wbc2Isr(mt_void *pParam0, mt_void *pParam1);
mt_s32 OPTM_GFX_OpenWbc2(OPTM_GFX_WBC_S *pstWbc2);
mt_s32 OPTM_GFX_CloseWbc2(OPTM_GFX_WBC_S *pstWbc2);

/*  in WBC mode, call-back function of switching for SD display system */
mt_void OPTM_DispInfoCallbackUnderWbc(mt_u32 u32Param0, mt_u32 u32Param1);
#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_SetCallbackToDisp(OPTM_GFX_GP_E enGPId, IntCallBack pCallBack, MT_DRV_DISP_CALLBACK_TYPE_E eType, MT_BOOL bFlag);
mt_void OPTM_DispCallBack(mt_void *u32Param0, mt_void *u32Param1);
mt_s32 OPTM_GFX_SetStereoDepth(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth);
mt_s32 OPTM_Distribute_Callback(mt_void *u32Param0, mt_void *u32Param1);
mt_void OPTM_FrameEndCallBack(mt_void *u32Param0, mt_void *u32Param1);
mt_void OPTM_SlaverProcess(mt_void *u32Param0, mt_void *u32Param1);
#endif

mt_s32 OPTM_GfxSetSrcFromWbc2(MT_BOOL bFromWbc2);
mt_s32 OPTM_GPMask(OPTM_VDP_LAYER_GP_E enGPId, MT_BOOL bFlag);

mt_s32 OPTM_GPRecovery(OPTM_VDP_LAYER_GP_E enGPId);
mt_s32 OPTM_GfxCloseLayer(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_GfxSetDispFMTSize(OPTM_GFX_GP_E enGpId, const mt_rect_s *pstOutRect);
mt_s32 OPTM_GfxSetEnable(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable);
mt_s32 OPTM_GfxGetLayerRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect);
mt_s32 OPTM_GfxSetLayerRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect);
mt_s32 OPTM_GfxSetGpRect(OPTM_GFX_GP_E enGpId, const MTFB_RECT *pstInputRect);
OPTM_VDP_GFX_IFMT_E OPTM_PixerFmtTransferToHalFmt(MTFB_COLOR_FMT_E enDataFmt);
mt_s32 OPTM_GfxConfigSlvLayer(MTFB_LAYER_ID_E enLayerId, mt_rect_s *pstRect);
mt_s32 OPTM_GfxSetCsc(OPTM_GFX_GP_E enGfxGpId, OPTM_GFX_CSC_PARA_S *pstCscPara, MT_BOOL bIsBGRIn);
mt_s32 OPTM_GfxOpenSlvLayer(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, mmz_buffer_s *psMBuf);
mt_void OPTM_UnmapAndRelease(mmz_buffer_s *psMBuf);
mt_s32 OPTM_Adapt_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, mmz_buffer_s *psMBuf);
mt_s32 OPTM_GfxSetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr);
mt_s32 OPTM_GfxSetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride);
OPTM_COLOR_SPACE_E OPTM_AdaptCscTypeFromDisp(MT_DRV_COLOR_SPACE_E enHiDrvCsc);
mt_s32 OPTM_JudgeWbcEnable(mt_void);
//static mt_s32 OPTM_GfxCloseSlvLayer(MTFB_LAYER_ID_E enLayerId);
#ifdef CFG_MTFB_COMPRESSION_SUPPORT
mt_void OPTM_GFX_CMP_Clean(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_GFX_CMP_Open(MTFB_LAYER_ID_E enLayerId);
mt_s32 OPTM_GFX_CMP_Close(MTFB_LAYER_ID_E enLayerId);
#endif
mt_s32 OPTM_GFX_CMP_DECMP_Process(MTFB_LAYER_ID_E enLayerId, mt_u32 u32RdAddr,
        mt_u32 pic_width, mt_u32 pic_height, mt_u32 u32Stride, mt_u32 u32WrAddr);

/******************************************************************
  function definitions
 ******************************************************************/
mt_u32 cmp_isr(int irq, void *dev_id)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    mt_u32 u32CmpIrqStatus;

    OPTM_FUN_IN;

    u32CmpIrqStatus = drv_reg_4k_disp_get_osdc_irq();
    MTFB_DEBUGK("[OSDC_IRQ]=0x%x\n", u32CmpIrqStatus);
    /* clear irq */
    drv_reg_4k_disp_set_osdc_irq(u32CmpIrqStatus);
#endif
    return MT_SUCCESS;
}

mt_u32 OPTM_AlignUp(mt_u32 x, mt_u32 a)
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

#ifdef MT_BUILD_IN_BOOT
extern mt_s32 Win_ReviseOutRect(const mt_rect_s *tmp_virtscreen,
        const MT_DRV_DISP_OFFSET_S *stOffsetInfo,
        const mt_rect_s *stFmtResolution,
        const mt_rect_s *stPixelFmtResolution,
        mt_rect_s *stToBeRevisedRect,
        mt_rect_s *stRevisedRect);
#define IO_ADDRESS(addr) (addr)

MTFB_GFX_MODE_EN OPTM_Get_GfxWorkMode(mt_void)
{
    return g_enOptmGfxWorkMode;
}

mt_s32 OPTM_GpInitFromDisp(OPTM_GFX_GP_E enGPId)
{
    MT_DRV_DISPLAY_E enDisp;
    MT_DISP_DISPLAY_INFO_S pstInfo;
    OPTM_COLOR_SPACE_E enGpCsc;

    if (!g_stGfxGPDevice[enGPId].bOpen)
    {
        return MT_FAILURE;
    }

    if (enGPId == OPTM_GFX_GP_0)
    {
        enDisp = MT_DRV_DISPLAY_1;
    }
    else
    {
        enDisp = MT_DRV_DISPLAY_0;
    }

    memset(&pstInfo, 0, sizeof(MT_DISP_DISPLAY_INFO_S));

    DISP_GetDisplayInfo(enDisp, &pstInfo);
    g_stGfxGPDevice[enGPId].bInterface = pstInfo.bInterlace;

    Win_ReviseOutRect(&pstInfo.stVirtaulScreen, &pstInfo.stOffsetInfo,
            &pstInfo.stFmtResolution, &pstInfo.stPixelFmtResolution,
            &pstInfo.stVirtaulScreen, &g_stGfxGPDevice[enGPId].stOutRect);

    if (pstInfo.bIsMaster && enGPId == OPTM_GFX_GP_0)
    {
        g_enOptmGfxWorkMode = MTFB_GFX_MODE_HD_WBC;
    }

    // Debug("DISP_GetDisplayInfo:GpId=%d,DisplayWidth=%d,DisplayHeight=%d\n",enGPId,pstInfo.stOrgRect.s32Width,pstInfo.stOrgRect.s32Height);
    OPTM_GPRecovery(enGPId);

    return MT_SUCCESS;
}
#endif

mt_s32 OPTM_GFX_GetDevCap(const MTFB_CAPABILITY_S **pstCap)
{
    *pstCap = &g_stGfxCap[0];

    return MT_SUCCESS;
}

OPTM_VDP_LAYER_GFX_E OPTM_GetGfxHalId(MTFB_LAYER_ID_E enLayerId)
{
    return OPTM_VDP_LAYER_GFX0;
    // in aria, there is only one GFX
#if 0
    if (MTFB_LAYER_SD_1 == enLayerId)
    {
        return OPTM_VDP_LAYER_GFX_BUTT;
    }
    else if (MTFB_LAYER_SD_1 > enLayerId)
    {
        return (OPTM_VDP_LAYER_GFX_E)enLayerId;
    }
    else
    {
        return OPTM_VDP_LAYER_GFX_BUTT;
    }
#endif
}

OPTM_COLOR_SPACE_E OPTM_AdaptCscTypeFromDisp(MT_DRV_COLOR_SPACE_E enHiDrvCsc)
{
    switch (enHiDrvCsc)
    {
    case MT_DRV_CS_BT601_YUV_LIMITED:
        return OPTM_CS_BT601_YUV_LIMITED;
    case MT_DRV_CS_BT601_YUV_FULL:
        return OPTM_CS_BT601_YUV_FULL;
    case MT_DRV_CS_BT709_YUV_LIMITED:
        return OPTM_CS_BT709_YUV_LIMITED;
    case MT_DRV_CS_BT709_YUV_FULL:
        return OPTM_CS_BT709_YUV_FULL;
    case MT_DRV_CS_BT709_RGB_FULL:
        return OPTM_CS_BT601_RGB_FULL;
    default:
        return OPTM_CS_BUTT;
    }
}
#ifndef MT_BUILD_IN_BOOT
MTFB_STEREO_MODE_E OPTM_AdaptTriDimModeFromDisp(OPTM_VDP_DISP_MODE_E enDispStereo)
{
    switch (enDispStereo)
    {
    case VDP_DISP_MODE_2D:
        return MTFB_STEREO_MONO;
    case VDP_DISP_MODE_SBS:
        return MTFB_STEREO_SIDEBYSIDE_HALF;
    case VDP_DISP_MODE_TAB:
        return MTFB_STEREO_TOPANDBOTTOM;
    case VDP_DISP_MODE_FP:
        return MTFB_STEREO_FRMPACKING;
    default:
        return MTFB_STEREO_BUTT;
    }

    return MTFB_STEREO_BUTT;
}
#endif

mt_void OPTM_GPDATA_Init(mt_void)
{
    memset(&(g_stGfxDevice[0]), 0, sizeof(OPTM_GFX_LAYER_S) * OPTM_MAX_LOGIC_MTFB_LAYER);
    memset(&(g_stGfxGPDevice[0]), 0, sizeof(OPTM_GFX_GP_S) * OPTM_GFX_GP_BUTT);
    memset(&(g_stGfxGPIrq[0]), 0, sizeof(OPTM_GP_IRQ_S) * OPTM_GFX_GP_BUTT);
    memset(&g_stGfxWbc2, 0, sizeof(OPTM_GFX_WBC_S));
}

/* physical base address of VOU registers' list */
#define OPTM_REGS_BASE_ADDR 0xf8cc0000

#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_Aapt_Module_GetFunction(mt_u32 u32ModuleID, mt_void **ppFunc)
{
    if (MT_NULL == ppFunc)
    {
        return MT_FAILURE;
    }

    if (MT_SUCCESS != mt_drv_module_getfunction(u32ModuleID, ppFunc))
    {
        return MT_FAILURE;
    }

    if (MT_NULL == *ppFunc)
    {
        return MT_FAILURE;
    }
    else
    {
        return MT_SUCCESS;
    }
}
#endif

mt_s32 OPTM_GfxInit(mt_void)
{
    //    mt_s32 s32ret;
    //    mt_u32 u32Phyaddr;

    OPTM_FUN_IN;

    if (OPTM_ENABLE == g_u32GFXInitFlag)  /** 保证只初始化一次 **/
    {
        return MT_SUCCESS;
    }

    /**
     **初始化gfx gp gpirq wbc全局变量
     **/
    OPTM_GPDATA_Init();

    /** has been initial **/
    g_u32GFXInitFlag = OPTM_ENABLE;

#ifdef CFG_MTFB_COMPRESSION_SUPPORT_OSD0
    g_stGfxDevice[MTFB_LAYER_OSD0].bCmpCfg = MT_TRUE;

#endif

#ifdef CFG_MTFB_COMPRESSION_SUPPORT_OSD1
    g_stGfxDevice[MTFB_LAYER_OSD1].bCmpCfg = MT_TRUE;
#endif

#ifdef CFG_MTFB_COMPRESSION_SUPPORT_SUB
    g_stGfxDevice[MTFB_LAYER_SUB].bCmpCfg = MT_TRUE;
#endif

    s_Refresh.vsync_flag = 0;
    init_waitqueue_head(&s_Refresh.vsync_wait);

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxDeInit(mt_void)
{
#ifndef MT_BUILD_IN_BOOT
    mt_s32 i;

    OPTM_FUN_IN;

    if (OPTM_DISABLE == g_u32GFXInitFlag)
    {
        return MT_SUCCESS;
    }

    for (i = MTFB_LAYER_BACKGROUND; i < MTFB_LAYER_ID_BUTT; i++)
    {
        if (g_stGfxDevice[i].bOpened != MT_FALSE)
        {
            OPTM_GfxCloseLayer(i);
        }
#ifdef CFG_MTFB_COMPRESSION_SUPPORT
        OPTM_GFX_CMP_Clean(i);
#endif
    }

    g_u32GFXInitFlag = OPTM_DISABLE;
    ps_DispExportFuncs = MT_NULL;
    g_bTcWBCFlag = MT_FALSE;

    OPTM_FUN_OUT;
#endif
    return MT_SUCCESS;
}

#if 0
#ifndef MT_BUILD_IN_BOOT
static mt_void OPTM_WorkQueueToOpenWbc(struct work_struct *data)
{
}
#endif

#ifndef MT_BUILD_IN_BOOT
static mt_void OPTM_3DMode_Callback(struct work_struct *data)
{
}
#endif
#endif

mt_void OPTM_ALG_Init(OPTM_GFX_GP_E enGPId)
{
}

static mt_s32 OPTM_GPOpen(OPTM_GFX_GP_E enGPId)
{
    mt_u32 i;
    OPTM_VDP_BKG_S stBkg;
    mt_u32 u32InitLayerID;
    mt_u32 u32MaxLayerCount;
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_u32 data;
    phys_addr_t coeff_phy_addr = 0;    
    ulong coeff_vir_addr = 0;
#endif
    OPTM_FUN_IN;
    MTFB_DEBUGK("%s: %d\n", __FUNCTION__, enGPId);

    if (g_stGfxGPDevice[enGPId].bOpen)  /** 已经打开，不用重新打开 **/
    {
        return MT_SUCCESS;
    }

#ifndef MT_BUILD_IN_BOOT
    if (MT_NULL == ps_DispExportFuncs)
    {
        /**
         **从disp中获取函数指针
         **/
        if (MT_SUCCESS != OPTM_Aapt_Module_GetFunction(MT_ID_DISP, (mt_void **)&ps_DispExportFuncs))
        {
            MTFB_ERROR("Fail to get disp export functions!\n");
            return MT_FAILURE;
        }
    }
#endif

    /**
     **初始化GP
     **/
    memset(&g_stGfxGPIrq[enGPId], 0, sizeof(OPTM_GP_IRQ_S));

    memset(&stBkg, 0, sizeof(OPTM_VDP_BKG_S));
    stBkg.u32BkgA = 0x0;

    g_stGfxGPDevice[enGPId].u32Alpha = 0xff;
    g_stGfxGPDevice[enGPId].enReadMode = VDP_RMODE_PROGRESSIVE;
    g_stGfxGPDevice[enGPId].stBkg = stBkg;
    g_stGfxGPDevice[enGPId].enInputCsc = OPTM_CS_BT709_RGB_FULL;
    g_stGfxGPDevice[enGPId].enOutputCsc = OPTM_CS_UNKNOWN;
    g_stGfxGPDevice[enGPId].bBGRState = MT_FALSE;
    /**
     ** set recovery true to make sure when gp was open initially,
     ** dispsize will be set to hardware
     **/
    g_stGfxGPDevice[enGPId].bGpClose = MT_FALSE;
    g_stGfxGPDevice[enGPId].bRecoveryInNextVT = MT_TRUE; /** 下次中断是否需要重新设置所有相关寄存器 **/
    g_stGfxGPDevice[enGPId].bDispInitial = MT_FALSE;

    /**
     ** 0:HIFB_LAYER_HD_0;  1:HIFB_LAYER_HD_1;  2:HIFB_LAYER_HD_2;  3:HIFB_LAYER_HD_3
     ** 0:HIFB_LAYER_SD_0;  1:HIFB_LAYER_SD_1
     **/
    if (OPTM_GFX_GP_0 == enGPId)
    {
        g_stGfxGPDevice[enGPId].enMixg = VDP_CBM_MIXG0;
        g_stGfxGPDevice[enGPId].enGpHalId = OPTM_VDP_LAYER_GP0;
        g_stGfxGPDevice[enGPId].enDispCh = OPTM_DISPCHANNEL_1;

        //OPTM_VDP_GP_SetLayerGalpha(enGPId, g_stGfxGPDevice[enGPId].u32Alpha);
        //OPTM_VDP_GP_SetReadMode   (enGPId, g_stGfxGPDevice[enGPId].enReadMode);
        //OPTM_VDP_CBM_SetMixerBkg  (g_stGfxGPDevice[enGPId].enMixg, g_stGfxGPDevice[enGPId].stBkg);
        u32InitLayerID = (mt_u32)MTFB_LAYER_HD_0;
        u32MaxLayerCount = (mt_u32)(OPTM_GP0_GFX_COUNT + u32InitLayerID - 1);
    }
    else if (OPTM_GFX_GP_1 == enGPId)
    {
        g_stGfxGPDevice[enGPId].enMixg = VDP_CBM_MIXG1;
        g_stGfxGPDevice[enGPId].enGpHalId = OPTM_VDP_LAYER_GP1;
        g_stGfxGPDevice[enGPId].enDispCh = OPTM_DISPCHANNEL_0;

        //OPTM_VDP_GP_SetLayerGalpha(enGPId, g_stGfxGPDevice[enGPId].u32Alpha);
        //OPTM_VDP_GP_SetReadMode   (enGPId, g_stGfxGPDevice[enGPId].enReadMode);
        //OPTM_VDP_CBM_SetMixerBkg  (g_stGfxGPDevice[enGPId].enMixg, g_stGfxGPDevice[enGPId].stBkg);
        u32InitLayerID = (mt_u32)MTFB_LAYER_SD_0;
        u32MaxLayerCount = (mt_u32)(OPTM_GP1_GFX_COUNT + u32InitLayerID - 1);
    }
    else
    {
        return MT_SUCCESS;
    }

    for (i = u32InitLayerID; i <= u32MaxLayerCount; i++)
    {
        g_stGfxDevice[i].enGfxHalId = OPTM_GetGfxHalId(i);
    }

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    //load the coeff table
    /**
     **分配coeff 内存
     **/
    {
        mt_char name_coeff[32];
        snprintf(name_coeff, sizeof(name_coeff), "MTFB_GP%d_Coeff", enGPId);
        /**
         ** apply coeff buffer
         **/
        if (OPTM_Adapt_AllocAndMap(name_coeff, OPTM_MMZ_ZONE, SCALE_COEFF_TABLE_MAX * SINGLE_SCALE_COEFF_TABLE_SIZE,
                    0, &g_stGfxGPDevice[enGPId].stCoeffTableBuf) != MT_SUCCESS) {
            MTFB_ERROR("GP Get coeff buffer failed!\n");
            return MT_FAILURE;
        }

        coeff_phy_addr = g_stGfxGPDevice[enGPId].stCoeffTableBuf.startPhyAddr;
        coeff_vir_addr = (ulong)g_stGfxGPDevice[enGPId].stCoeffTableBuf.startVirAddr;

        for (i = 0; i < SCALE_COEFF_TABLE_MAX; i++)
        {
            g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_phy_addr = coeff_phy_addr + (i * SINGLE_SCALE_COEFF_TABLE_SIZE);
            g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_kvir_addr = coeff_vir_addr + (i * SINGLE_SCALE_COEFF_TABLE_SIZE);
            g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_size = SINGLE_SCALE_COEFF_TABLE_SIZE;

#if 0
            memcpy(&g_TempScaleFilter,
                    &scale_coeff_table[i * SINGLE_SCALE_COEFF_TABLE_SIZE],
                    g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_size);

            //On Aria, the filter endian mode is changed, every 16 byte should be switched
            for(j=0;j<SINGLE_SCALE_COEFF_TABLE_SIZE;j+=SCALE_FILTER_ENDIAN_SWITCH_SIZE)
            {
                p16ByteSrc=&g_TempScaleFilter[j];
                p16ByteDst=((mt_u8 *)g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_kvir_addr)+j;
                //switch every 8 bytes
                memcpy(p16ByteDst,p16ByteSrc+SCALE_FILTER_ENDIAN_SWITCH_SIZE/2,SCALE_FILTER_ENDIAN_SWITCH_SIZE/2);
                memcpy(p16ByteDst+SCALE_FILTER_ENDIAN_SWITCH_SIZE/2,p16ByteSrc,SCALE_FILTER_ENDIAN_SWITCH_SIZE/2);
            }
#else
            memcpy((void *)g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_kvir_addr,
                    &scale_coeff_table[i * SINGLE_SCALE_COEFF_TABLE_SIZE],
                    g_stGfxGPDevice[enGPId].stCoeffTable[i].coeff_size);
#endif
        }
    }
#endif
    //    if (0 != request_irq(CMP_INTNUM, (irq_handler_t)cmp_isr,
    //                         IRQF_TRIGGER_HIGH, "mt_cmp_irq", MT_NULL))
    //    {
    //        MTFB_ERROR("request_irq for CMP failure!\n");
    //        return MT_FAILURE;
    //    }

#if defined(CONFIG_MT_CHIP_SYMPHONY4) 
    /**
     **adjust GP clk
     **/
    data = HAL_GET_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa104))); 
    data &= ~0x03;
    data |= 0x01;
    HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa104)), data); 
#endif
    /**
     **GP设备打开
     **/
    g_stGfxGPDevice[enGPId].bOpen = MT_TRUE;
    g_bGpCoefChangeFlag = MT_TRUE;
    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

static mt_s32 OPTM_GPClose(OPTM_GFX_GP_E enGPId)
{
    OPTM_FUN_IN;
    if (MT_FALSE == g_stGfxGPDevice[enGPId].bOpen)
    {
        return MT_SUCCESS;
    }

    g_enOptmGfxWorkMode = MTFB_GFX_MODE_NORMAL;

#ifndef MT_BUILD_IN_BOOT
    OPTM_SetCallbackToDisp(enGPId, (IntCallBack)OPTM_DispCallBack, MT_DRV_DISP_C_INTPOS_90_PERCENT, MT_FALSE);

    OPTM_SetCallbackToDisp(enGPId, (IntCallBack)OPTM_FrameEndCallBack, MT_DRV_DISP_C_INTPOS_100_PERCENT, MT_FALSE);
    if (g_stGfxGPDevice[enGPId].queue)
    {
        //destroy_workqueue(g_stGfxGPDevice[enGPId].queue);
        g_stGfxGPDevice[enGPId].queue = MT_NULL;
    }
#endif

    g_stGfxGPDevice[enGPId].bOpen = MT_FALSE;
    g_stGfxGPDevice[enGPId].bGPInInitial = MT_FALSE;
    g_stGfxGPDevice[enGPId].bGPInSetbyusr = MT_FALSE;
    g_stGfxGPDevice[enGPId].bDispInitial = MT_FALSE;
    g_stGfxGPDevice[enGPId].bNeedExtractLine = MT_FALSE;
    g_stGfxGPDevice[enGPId].bMaskFlag = MT_FALSE;
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    /*  release coeff buffer */
    if(g_stGfxGPDevice[enGPId].stCoeffTableBuf.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxGPDevice[enGPId].stCoeffTableBuf));
        g_stGfxGPDevice[enGPId].stCoeffTableBuf.startVirAddr = 0;
        g_stGfxGPDevice[enGPId].stCoeffTableBuf.startPhyAddr = 0;
    }
#endif

    //free_irq(CMP_INTNUM);
    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

static mt_s32 OPTM_GfxSetLayerReadMode(MTFB_LAYER_ID_E enLayerId, OPTM_VDP_DATA_RMODE_E enReadMode)
{
    OPTM_GFX_GP_E enGPId;

    enGPId = g_stGfxDevice[enLayerId].enGPId;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        drv_reg_4k_disp_set_osdl_osd0_cmd_force_progressive_mode(enReadMode);
        break;
    case MTFB_LAYER_OSD1:
        drv_reg_4k_disp_set_osdl_osd1_cmd_force_progressive_mode(enReadMode);
        break;
    case MTFB_LAYER_SUB:
        drv_reg_4k_disp_set_osdl_sub_cmd_force_progressive_mode(enReadMode);
        break;
    case MTFB_LAYER_STILL:
        drv_reg_4k_disp_set_still_control_progressive_mode(enReadMode);
        break;

    default:
        break;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        reg_symphony_optm_disp_set_osd0_cmd_hd_force_progressive(enReadMode);
        break;
    case MTFB_LAYER_OSD1:
        reg_symphony_optm_disp_set_osd1_cmd_hd_force_progressive(enReadMode);
        break;
    case MTFB_LAYER_SUB:
        reg_symphony_optm_disp_set_sub_cmd_hd_force_progressive(enReadMode);
        break;
    default:
        break;
    }
#endif

    return MT_SUCCESS;
}

static mt_s32 OPTM_GfxInitLayer(MTFB_LAYER_ID_E enLayerId)
{
    OPTM_VDP_BKG_S stBkg;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d\n", __FUNCTION__, enLayerId);

    g_stGfxDevice[enLayerId].enGfxHalId = OPTM_GetGfxHalId(enLayerId);
    g_stGfxDevice[enLayerId].enGPId = OPTM_GFX_GP_0; // (g_stGfxDevice[enLayerId].enGfxHalId > OPTM_VDP_LAYER_GFX3) ? OPTM_GFX_GP_1 : OPTM_GFX_GP_0;
    g_stGfxDevice[enLayerId].CscState = OPTM_CSC_SET_PARA_RGB;

    memset(&stBkg, 0, sizeof(stBkg));
    stBkg.u32BkgA = 0x0;
    g_stGfxDevice[enLayerId].stBkg = stBkg;

    /** 由制式决定的 **/
    g_stGfxDevice[enLayerId].enReadMode = VDP_RMODE_PROGRESSIVE;

    /**
     **设置读模式，这里包括图层和GP
     **读模式设置为逐行，GP和G设置一样的，和制式相关
     **/
    OPTM_GfxSetLayerReadMode(enLayerId, g_stGfxDevice[enLayerId].enReadMode);
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#if 0  //nothing need to do
    //enable osd alpha and scaler filter and also mix sub first
    drv_reg_4k_disp_set_osds_cmd_osd_vert_no_filter(0);
    drv_reg_4k_disp_set_osds_cmd_osd_vert_no_filter_alpha(0);
    drv_reg_4k_disp_set_osds_cmd_osd_hori_no_filter(0);
    drv_reg_4k_disp_set_osds_cmd_osd_hori_no_filter_alpha(0);
    drv_reg_4k_disp_set_osdm_cmd_osd_sub_mix_first(1);

    //osd latch at top start
    drv_reg_4k_disp_set_osdl_cmd_osd_latch_or_not(0);
    drv_reg_4k_disp_set_osdl_cmd_osd_latch_3d_2nd(0);
    drv_reg_4k_disp_set_osdl_cmd_osd_latch_3d_1st(0);
    drv_reg_4k_disp_set_osdl_cmd_osd_latch_bot(0);
    drv_reg_4k_disp_set_osdl_cmd_osd_latch_top(1);

    //still latch at top start
    drv_reg_4k_disp_set_still_latch_command_still_latch_or_not(2);
    drv_reg_4k_disp_set_still_latch_command_still_latch_3d_2nd(0);
    drv_reg_4k_disp_set_still_latch_command_still_latch_3d_1st(0);
    drv_reg_4k_disp_set_still_latch_command_still_latch_bot(0);
    drv_reg_4k_disp_set_still_latch_command_still_latch_top(1);
#endif
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_symphony_optm_disp_set_osd_alpha_osd_alpha_filter(0);
    reg_symphony_optm_disp_set_osd_alpha_border_cfg(1);
    reg_symphony_optm_disp_set_osd_alpha_osd_no_filter(0);
    reg_symphony_optm_disp_set_osd_alpha_osd_sub_mix_first(1);

#endif

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

static mt_s32 OPTM_GfxDeInitLayer(MTFB_LAYER_ID_E enLayerId)
{
#ifndef MT_BUILD_IN_BOOT
    OPTM_FUN_IN;

    if ((enLayerId == MTFB_LAYER_OSD0) || (enLayerId == MTFB_LAYER_OSD1) || (enLayerId == MTFB_LAYER_SUB))
    {
        /*  release control word and CLUT TABLE buffer */
        if(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr != 0)
        {
            OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stRgnHeaderBuf));
            g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr = 0;
            g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr = 0;
        }
    }

    OPTM_FUN_OUT;
#endif
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetCsc(OPTM_GFX_GP_E enGfxGpId, OPTM_GFX_CSC_PARA_S *pstCscPara, MT_BOOL bIsBGRIn)
{

    OPTM_FUN_IN;

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

#ifndef MT_BUILD_IN_BOOT
static MTFB_LAYER_ID_E CallBackLayerId[MTFB_LAYER_ID_BUTT];

mt_s32 OPTM_GfxSetCallback(MTFB_LAYER_ID_E enLayerId, IntCallBack pCallBack, MTFB_CALLBACK_TPYE_E eCallbackType)
{
    mt_u32 u32GfxIndex;
    OPTM_GFX_GP_E enGPId;

    OPTM_FUN_IN;

    if (eCallbackType >= MTFB_CALLBACK_TYPE_BUTT)
    {
        MTFB_ERROR("Fail to set callback func!\n");
        return MT_FAILURE;
    }

    /***back up layer's id in the global array***/
    CallBackLayerId[enLayerId] = enLayerId;

    enGPId = g_stGfxDevice[enLayerId].enGPId; // enGPId is 0
    //u32GfxIndex = (enLayerId > MTFB_LAYER_HD_3) ? (enLayerId - MTFB_LAYER_HD_3 - 1) : enLayerId;
    u32GfxIndex = enLayerId;

    MTFB_DEBUGK("%s: %d, %x, %lx, %lx\n", __FUNCTION__, enGPId, (mt_u32)enLayerId, (ulong)pCallBack, (ulong)eCallbackType);

    if (u32GfxIndex >= OPTM_GP_MAXGFXCOUNT)
    {
        MTFB_ERROR("Fail to set callback func!\n");
        return MT_FAILURE;
    }

    if (MT_NULL != pCallBack)
    {
        g_stGfxGPIrq[enGPId].stGfxCallBack[u32GfxIndex].stGfxIrq[eCallbackType].pFunc = pCallBack;
        g_stGfxGPIrq[enGPId].stGfxCallBack[u32GfxIndex].stGfxIrq[eCallbackType].u32Param0 = (ulong) &CallBackLayerId[enLayerId];
        g_stGfxGPIrq[enGPId].stGfxCallBack[u32GfxIndex].u32CTypeFlag |= eCallbackType;
    }
    else
    {
        g_stGfxGPIrq[enGPId].stGfxCallBack[u32GfxIndex].u32CTypeFlag &= ~((mt_u32)eCallbackType);
        g_stGfxGPIrq[enGPId].stGfxCallBack[u32GfxIndex].stGfxIrq[eCallbackType].pFunc = MT_NULL;
    }

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}
mt_s32 OPTM_CheckGfxCallbackReg(OPTM_GFX_GP_E enGPId, MTFB_CALLBACK_TPYE_E eCallbackType)
{
    return MT_SUCCESS;
}
#endif

mt_s32 OPTM_GfxSetWbcAddr(OPTM_VDP_LAYER_WBC_E enWbcHalId, mt_u32 u32Addr, mt_u32 u32Stride)
{
    return MT_SUCCESS;
}

/*Open  sd0 layer when working int wbc mode*/
mt_s32 OPTM_GfxOpenSlvLayer(MTFB_LAYER_ID_E enLayerId)
{
    return MT_SUCCESS;
}

#if 0
/*Close  sd0 layer when working int wbc mode*/
static mt_s32 OPTM_GfxCloseSlvLayer(MTFB_LAYER_ID_E enLayerId)
{
    return MT_SUCCESS;
}
#endif

#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_GetScreenRectFromDispInfo(const mt_rect_s *tmp_virtscreen,
        const OPTM_GFX_OFFSET_S *stOffsetInfo,
        const mt_rect_s *stFmtResolution,
        const mt_rect_s *stPixelFmtResolution,
        mt_rect_s *stScreenRect)
{
    OPTM_GFX_OFFSET_S tmp_offsetInfo;

    tmp_offsetInfo = *stOffsetInfo;

    if (tmp_virtscreen->s32Width == 0 || tmp_virtscreen->s32Height == 0)
    {
        return MT_FAILURE;
    }

    if ((stFmtResolution->s32Width * 2) == stPixelFmtResolution->s32Width)
    {
        tmp_offsetInfo.u32Left *= 2;
        tmp_offsetInfo.u32Right *= 2;
    }

    stScreenRect->s32X = tmp_offsetInfo.u32Left;
    stScreenRect->s32Y = tmp_offsetInfo.u32Top;

    stScreenRect->s32Width = (stPixelFmtResolution->s32Width - tmp_offsetInfo.u32Left - tmp_offsetInfo.u32Right);
    stScreenRect->s32Height = (stPixelFmtResolution->s32Height - tmp_offsetInfo.u32Top - tmp_offsetInfo.u32Bottom);

    stScreenRect->s32X = OPTM_AlignUp(stScreenRect->s32X, 2);
    stScreenRect->s32Y = OPTM_AlignUp(stScreenRect->s32Y, 2);
    stScreenRect->s32Width = OPTM_AlignUp(stScreenRect->s32Width, 2);
    stScreenRect->s32Height = OPTM_AlignUp(stScreenRect->s32Height, 2);

    tmp_offsetInfo = *stOffsetInfo;

    return MT_SUCCESS;
}
#endif
#ifndef MT_BUILD_IN_BOOT
mt_void OPTM_SlaverProcess(mt_void *u32Param0, mt_void *u32Param1)
{
}

mt_void OPTM_Wbc2Process(mt_void *u32Param0, mt_void *u32Param1)
{
}


mt_void OPTM_FrameEndCallBack(mt_void *u32Param0, mt_void *u32Param1)
{

    mt_u32 u32CTypeFlag;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;
    mt_u32 Ct;
    mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;


    if(s_Refresh.refreshSyncFlag == 0)
    {
        s_Refresh.refreshSyncFlag = 1;
        wake_up_interruptible(&s_Refresh.refreshSyncEvent);
    }

    if (MT_NULL == pEnGpHalId || MT_NULL == pstDispInfo)
    {
        MTFB_WARNING("unable to handle null point in dispcallback\n");
        return;
    }

    if (!g_stGfxGPDevice[*pEnGpHalId].bOpen)
    {
        return;
    }

    if(s_Refresh.vsync_flag== 0)
    {
        s_Refresh.vsync_flag= 1;
        wake_up_interruptible(&s_Refresh.vsync_wait);
    }

    if (*pEnGpHalId == OPTM_VDP_LAYER_GP0)
    {
        if (pstDispInfo->stDispInfo.bInterlace &&
                !pstDispInfo->stDispInfo.bIsBottomField)
        {
            return;
        }
        OPTM_Wbc2Process(u32Param0, u32Param1);

        u32CTypeFlag = g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[MTFB_LAYER_HD_0].u32CTypeFlag;
        if (u32CTypeFlag & MTFB_CALLBACK_TYPE_FRAME_END)
        {
            g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[MTFB_LAYER_HD_0].stGfxIrq[MTFB_CALLBACK_TYPE_FRAME_END].pFunc(
                    (mt_void *)g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[MTFB_LAYER_HD_0].stGfxIrq[MTFB_CALLBACK_TYPE_FRAME_END].u32Param0,
                    MT_NULL);
        }
    }
    else if (*pEnGpHalId == OPTM_VDP_LAYER_GP1)
    {
        //MTFB_INFO("==========frame end==========\n");
        //OPTM_SlaverProcess(u32Param0, u32Param1);
    }

    return;
}
#if 0
static MT_BOOL OPTM_DispInfoProcess(OPTM_VDP_LAYER_GP_E eGpId, MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo)
{
    MT_BOOL bDispInfoChange = MT_FALSE;

    return bDispInfoChange;
}
#endif

/*!
  Refresh OSD rect.

  \param[in] enLayerId .
  */
static mt_u32 disp_aria_osd_rect_refresh(MTFB_LAYER_ID_E enLayerId)
{
    mt_u32 nRet = MT_SUCCESS;
    MTFB_RECT Rect;

    /** 判断该图层是否打开 **/
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    OPTM_GfxGetLayerRect(enLayerId, &Rect);
    nRet = OPTM_GfxSetLayerRect(enLayerId, &Rect);

    return nRet;
}


/*!
  Osd horizotal scale update.

  \param[in] p_dp priv handle.
  */
static mt_u32 u32Left = 0;
static mt_u32 u32Top = 0;
static mt_u32 u32Right  = 0;
static mt_u32 u32Bottom = 0;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
static void disp_symphony_osd_scale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    static mt_bool b_first_update = 1;
    rect_size_t osd_ori_size;
    static rect_size_t osd_orig_size_old;
    rect_vsb_t vout_hd_rect_cur = { 0 };
    static rect_vsb_t vout_hd_rect_old = { 0 };
    mt_u32 out_interleave = 0;
    static mt_u32 out_interleave_old = 0xff;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    static mt_u8 hd_reinterlace_en_old = 0;
    mt_u8 hd_reinterlace_en = drv_reg_4k_disp_get_hd_reinterlace_ctrl_reinterlace_en();
    mt_u8 sd_reinterlace_en = drv_reg_4k_disp_get_sd_reinterlace_ctrl_reinterlace_en();

    if (ps_DispExportFuncs && ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode)
    {
        out_interleave = ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    }
    else
    {
        MTFB_DEBUGK("disp func is null!\n");
    }

    if((hd_reinterlace_en || sd_reinterlace_en)
      && (out_interleave_old != out_interleave)
	  && (drv_reg_4k_disp_get_video_ctrl_1_video_sel() == 0)
	  )
    {
        hd_reinterlace_en = 0;
        sd_reinterlace_en = 0;
        drv_reg_4k_disp_set_hd_reinterlace_ctrl_reinterlace_en(hd_reinterlace_en);
        drv_reg_4k_disp_set_sd_reinterlace_ctrl_reinterlace_en(sd_reinterlace_en);
    }

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    OPTM_GetScreenRectFromDispInfo(&pstDispInfo->stDispInfo.stVirtaulScreen,
            (OPTM_GFX_OFFSET_S *)&pstDispInfo->stDispInfo.stOffsetInfo,
            &pstDispInfo->stDispInfo.stFmtResolution,
            &pstDispInfo->stDispInfo.stPixelFmtResolution,
            &g_stGfxGPDevice[*pEnGpHalId].stOutRect);

    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Width = 1280;
    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Height = 720;

    u32Left = pstDispInfo->stDispInfo.stOffsetInfo.u32Left;
    u32Right = pstDispInfo->stDispInfo.stOffsetInfo.u32Right;
    osd_ori_size.w =  g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Width + pstDispInfo->stDispInfo.stOffsetInfo.u32Left + pstDispInfo->stDispInfo.stOffsetInfo.u32Right;


    u32Top = pstDispInfo->stDispInfo.stOffsetInfo.u32Top;
    u32Bottom = pstDispInfo->stDispInfo.stOffsetInfo.u32Bottom;
    osd_ori_size.h = g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Height + pstDispInfo->stDispInfo.stOffsetInfo.u32Top + pstDispInfo->stDispInfo.stOffsetInfo.u32Bottom;

    //get hd output size
    vout_hd_rect_cur.w = pstDispInfo->stDispInfo.stFmtResolution.s32Width;
    vout_hd_rect_cur.h = pstDispInfo->stDispInfo.stFmtResolution.s32Height;

    if(osd_ori_size.w != 0 && osd_ori_size.h != 0)
    {
        if ((vout_hd_rect_old.w != vout_hd_rect_cur.w) 
                || (osd_orig_size_old.w != osd_ori_size.w)
                || (vout_hd_rect_old.h != vout_hd_rect_cur.h)                 
                || (osd_orig_size_old.h != osd_ori_size.h)
                || (out_interleave_old != out_interleave) 
                || (hd_reinterlace_en_old != hd_reinterlace_en)
                || b_osd_force_update
           )
        {
            MTFB_DEBUGK("%s %d, s_w:%d s_h:%d d_w:%d d_h:%d out_interleave:%d hd_reinterlace_en:%d\n", 
                __FUNCTION__, __LINE__, 
                osd_ori_size.w, osd_ori_size.h, vout_hd_rect_cur.w, vout_hd_rect_cur.h,
                out_interleave, hd_reinterlace_en);
            
            if(!drv_reg_4k_disp_get_video_ctrl_1_video_sel()
                || (osd_ori_size.w != drv_reg_4k_disp_get_osds_hsize_osd_ori_hsize())
                || (osd_ori_size.h != drv_reg_4k_disp_get_osds_vsize_osd_ori_vsize())
                || b_first_update)
            {
                DF_OsdScaler_Update(osd_ori_size.w, osd_ori_size.h, vout_hd_rect_cur.w, vout_hd_rect_cur.h);
                b_first_update = 0;
            }
                
            if(osd_orig_size_old.w != osd_ori_size.w)
            {
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD0);
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD1);
                disp_aria_osd_rect_refresh(MTFB_LAYER_SUB);
            }

            vout_hd_rect_old.w = vout_hd_rect_cur.w;
            osd_orig_size_old.w = osd_ori_size.w;
            vout_hd_rect_old.h = vout_hd_rect_cur.h;
            out_interleave_old = out_interleave;
            osd_orig_size_old.h = osd_ori_size.h;
            hd_reinterlace_en_old = hd_reinterlace_en;
            b_osd_force_update = FALSE;
        }
    }

}


static void disp_symphony_still_scale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    rect_vsb_t still_in_rect_cur = { 0 };
    rect_vsb_t still_out_rect_cur = { 0 };
    static rect_vsb_t still_in_rect_old = { 0 };
    static rect_vsb_t still_out_rect_old = { 0 };
    mt_u32 out_interleave = 0;
    static mt_u32 out_interleave_old = 0;

    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    if (ps_DispExportFuncs && ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode)
    {
        out_interleave = ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    }
    else
    {
        MTFB_ERROR("disp func is null!\n");
    }


  //  g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w = 1280;
  //  g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h = 720;
    
    still_in_rect_cur.w = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w;
    still_in_rect_cur.h = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h;
    still_out_rect_cur.w = pstDispInfo->stDispInfo.stFmtResolution.s32Width;
    still_out_rect_cur.h = pstDispInfo->stDispInfo.stFmtResolution.s32Height;
    
    if ((still_in_rect_cur.w != 0) && (still_in_rect_cur.h != 0) 
        && ((still_in_rect_old.w != still_in_rect_cur.w) 
             || (still_in_rect_old.h != still_in_rect_cur.h)
             || (still_out_rect_old.w != still_out_rect_cur.w)
             || (still_out_rect_old.h != still_out_rect_cur.h)
             || (out_interleave_old != out_interleave) )
        )
    {
        MTFB_DEBUGK("%s %d, s_w:%d s_h:%d d_w:%d d_h:%d\n", __FUNCTION__, __LINE__, 
            still_in_rect_cur.w, still_in_rect_cur.h, still_out_rect_cur.w, still_out_rect_cur.h);
        
        if (ps_DispExportFuncs && ps_DispExportFuncs->pfnDispStillScalerUpdate)
        {
            ps_DispExportFuncs->pfnDispStillScalerUpdate(still_in_rect_cur.w, 
                still_in_rect_cur.h, 
                still_out_rect_cur.w, 
                still_out_rect_cur.h,
                1);
        }
        else
        {
            MTFB_ERROR("disp func is null!\n");
        }        
        
        still_in_rect_old.w = still_in_rect_cur.w;
        still_out_rect_old.w = still_out_rect_cur.w;
        still_in_rect_old.h = still_in_rect_cur.h;
        still_out_rect_old.h = still_out_rect_cur.h; 
        out_interleave_old = out_interleave;
    }
}

#else

/*!
  Set osd horizontal scaler paramters.

  \param[in] p_dp priv handle.
  \param[in] src_w The width of the source osd
  \param[in] dst_w The width of the target osd
  */
static void disp_aria_osd_hscale_ratio_set(
        mt_u16 src_w,
        mt_u16 dst_w)
{
    mt_u32 hratio_int = 0;
    mt_u32 hratio_fra = 0;

    MT_ASSERT(0 != src_w * dst_w);

    // Please refer to Spec
    hratio_int = (src_w / dst_w) & 0xF;
    hratio_fra = (src_w % dst_w) * 4096 / dst_w;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    drv_reg_4k_disp_set_osds_hratio_osd_hori_ratio_int(hratio_int);
    drv_reg_4k_disp_set_osds_hratio_osd_hori_ratio_fra(hratio_fra);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_symphony_optm_disp_set_osd_scale_ratio_osd_ratio_int(hratio_int);
    reg_symphony_optm_disp_set_osd_scale_ratio_osd_ratio_fra(hratio_fra);

#endif
}

/*!
  Set osd vertical scaler paramters.

  \param[in] p_dp priv handle.
  \param[in] src_h The width of the source osd
  \param[in] dst_h The width of the target osd
  */
static void disp_aria_osd_vscale_ratio_set(
        mt_u16 src_h,
        mt_u16 dst_h,
        mt_u32 filter_phase_type)
{
    mt_u32 vratio_int = 0;
    mt_u32 vratio_fra = 0;
    mt_u32 out_interleave = 0;
    mt_u32 odd_start_line = 0;
    mt_u32 even_start_line = 0;
    mt_u32 odd_start_phase = 0;
    mt_u32 even_start_phase = 0;
    mt_u32 temp1 = 0;
    mt_u32 temp2 = 0;
    mt_u32 temp_fra = 0;
    mt_u32 temp2_fra = 0;
    mt_u32 m = 0;

    MT_ASSERT(0 != src_h * dst_h);

    /* calculate scale ratio */
    vratio_int = (src_h / dst_h) & 0xF;
    vratio_fra = (src_h % dst_h) * 4096 / dst_h;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    drv_reg_4k_disp_set_osds_vratio_osd_vert_ratio_int(vratio_int);
    drv_reg_4k_disp_set_osds_vratio_osd_vert_ratio_fra(vratio_fra);

    /* calculate start_line and start_phase */
    /*
       if(p->p)                              //progressive -> progressive
       start_line = 0x11;
       else if(p->i)                        // progressive -> interlace
       start_line = m<<4 + 0x01;

       tmp1 = int(1/2 * v_ratio);
       tmp_fra = fra(1/2 * v_ratio) * 0x1000;
       tmp2 = int((0x800 + tmp_fra) >> 12)
       tmp2_fra = fra(0x800 + tmp_fra);

       if(filter_phase_type == 1)
       m = 0x1 + tmp1 + tmp2;
       else if(filter_phase_type == 0)
       m = 0x1 + tmp1;

       n = {(p->p), filter_phase_type}
       case(n)
       2'b00: start_phase = tmp_fra << 16 + 0x0;
       2'b01: start_phase = tmp2_fra << 16 + 0x800;
       2'b10: start_phase = 0x00;
       2'b11: start_phase = 0x0800_0800
       */
    //out_interleave = reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    out_interleave = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    if (out_interleave == 0 ||
            drv_reg_4k_disp_get_osdl_osd0_cmd_force_progressive_mode() == 0)
    {
        /* p */
        even_start_line = 0x1;
        even_start_phase = 0x800 * filter_phase_type;
    }
    else
    {
        /* i */
        temp1 = (src_h / (2 * dst_h)) & 0xF;
        temp_fra = (src_h % (2 * dst_h)) * 4096 / (2 * dst_h);
        temp2 = (0x800 + temp_fra) >> 12;
        temp2_fra = (0x800 + temp_fra) & 0xFFF;
        m = 0x1 + temp1 + filter_phase_type * temp2;
        even_start_line = m;
        if (filter_phase_type)
        {
            even_start_phase = temp2_fra;
        }
        else
        {
            even_start_phase = temp_fra;
        }
    }
    odd_start_line = 0x1;
    odd_start_phase = 0x800 * filter_phase_type;
    drv_reg_4k_disp_set_osds_v_start_line_osd_odd_start_line(odd_start_line);
    drv_reg_4k_disp_set_osds_v_start_line_osd_even_start_line(even_start_line);
    drv_reg_4k_disp_set_osds_v_start_fra_osd_vert_start_fra_odd(odd_start_phase);
    drv_reg_4k_disp_set_osds_v_start_fra_osd_vert_start_fra_even(even_start_phase);

    //enable filter
    drv_reg_4k_disp_set_osds_cmd_osd_vert_no_filter(0);

    drv_reg_4k_disp_set_osds_cmd_osd_vphase_type(0);

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_symphony_optm_disp_set_osd_vertical_ratio_osd_vratio_int(vratio_int);
    reg_symphony_optm_disp_set_osd_vertical_ratio_osd_vratio_fra(vratio_fra);

    /* calculate start_line and start_phase */
    /*
       if(p->p)                //progressive -> progressive
       start_line = 0x11;
       else if(p->i)              // progressive -> interlace
       start_line = m<<4 + 0x01;

       tmp1 = int(1/2 * v_ratio);
       tmp_fra = fra(1/2 * v_ratio) * 0x1000;
       tmp2 = int((0x800 + tmp_fra) >> 12)
       tmp2_fra = fra(0x800 + tmp_fra);

       if(filter_phase_type == 1)
       m = 0x1 + tmp1 + tmp2;
       else if(filter_phase_type == 0)
       m = 0x1 + tmp1;

       n = {(p->p), filter_phase_type}
       case(n)
       2'b00: start_phase = tmp_fra << 16 + 0x0;
       2'b01: start_phase = tmp2_fra << 16 + 0x800;
       2'b10: start_phase = 0x00;
       2'b11: start_phase = 0x0800_0800
       */

    //out_interleave = reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    if (ps_DispExportFuncs && ps_DispExportFuncs->reg_aria_hd_encoder_get_basic_cfg_interlace_mode)
    {
        out_interleave = ps_DispExportFuncs->reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    }
    else
    {
        MTFB_ERROR("disp func is null!\n");
    }

    if(out_interleave == 0 ||
            reg_symphony_optm_disp_get_osd0_cmd_hd_force_progressive() == 0)
    {
        /* p */
        even_start_line = 0x1;
        even_start_phase = 0x800 * filter_phase_type;
    }
    else
    {
        /* i */
        temp1 = (src_h / (2 * dst_h)) & 0xF;
        temp_fra = (src_h % (2 * dst_h)) * 4096 / (2 *dst_h);
        temp2 = (0x800 + temp_fra) >> 12;
        temp2_fra = (0x800 + temp_fra) & 0xFFF;
        m = 0x1 + temp1 + filter_phase_type * temp2;
        even_start_line = m;
        if(filter_phase_type)
        {
            even_start_phase = temp2_fra;
        }
        else
        {
            even_start_phase = temp_fra;
        }
    }
    odd_start_line = 0x1;
    odd_start_phase = 0x800 * filter_phase_type;
    reg_symphony_optm_disp_set_osd_vert_start_line_osd_odd_start_line(odd_start_line);
    reg_symphony_optm_disp_set_osd_vert_start_line_osd_even_start_line(even_start_line);
    reg_symphony_optm_disp_set_osd_v_start_fra_osd_v_start_fra_odd(odd_start_phase);
    reg_symphony_optm_disp_set_osd_v_start_fra_osd_v_start_fra_even(even_start_phase);

    //enable filter
    reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_no_filter(0);

    reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_phase_type(0);

#endif
}

/*!
  Select osd vertical scaler coeffcient table.

  \param[in] p_dp priv handle.
  \param[in] src_h The height of the source osd
  \param[in] dst_h The height of the target osd
  \param[out] filter phase type
  */
static mt_u32 disp_aria_osd_vscale_coeff_table_sel(mt_void *u32Param0,
        mt_u16 src_h,
        mt_u16 dst_h)
{
    mt_u32 tap_num = 0;
    mt_u32 table_index = 0;
    mt_u32 filter_phase_type = 0;
    mt_u32 ratio_tenfold = 0;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;

    MT_ASSERT(0 != src_h * dst_h);

    ratio_tenfold = 10 * src_h / dst_h;
    if (ratio_tenfold <= 10)
    {
        tap_num = 3;
        filter_phase_type = 1;
        table_index = SCALE_COEFF_TABLE_4_V_GRAPHIC_SHARP;
    }
    else if (ratio_tenfold <= 12)
    {
        tap_num = 3;
        filter_phase_type = 1;
        table_index = SCALE_COEFF_TABLE_3_TAP_2;
    }
    else if (ratio_tenfold <= 15)
    {
        tap_num = 4;
        filter_phase_type = 0;
        table_index = SCALE_COEFF_TABLE_4_TAP_1;
    }
    else if (ratio_tenfold <= 20)
    {
        tap_num = 4;
        filter_phase_type = 0;
        table_index = SCALE_COEFF_TABLE_4_TAP_2;
    }
    else
    {
        tap_num = 5;
        filter_phase_type = 1;
        table_index = SCALE_COEFF_TABLE_5_TAP_0;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    //load osd vscaler table
    drv_reg_4k_disp_set_osds_v_tap_osd_vert_tap_num(tap_num);
    drv_reg_4k_disp_set_osds_vf_coeff_addr(g_stGfxGPDevice[*pEnGpHalId].stCoeffTable[table_index].coeff_phy_addr);
    drv_reg_4k_disp_set_coeff_table_sel_osd_vf_coeff_load_en(1);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    //load osd vscaler table
    reg_symphony_optm_disp_set_osd_v_tap_num_tap_num(tap_num);
    reg_symphony_optm_disp_set_osd_vf_coef_addr(g_stGfxGPDevice[*pEnGpHalId].stCoeffTable[table_index].coeff_phy_addr >> 3);
    reg_symphony_optm_disp_set_vscaler_table_sel_osd_vert_table_sel(1);

#endif

    return filter_phase_type;
}

/*!
  Select osd horizontal scaler coeffcient table.

  \param[in] p_dp priv handle.
  \param[in] src_w The width of the source osd
  \param[in] dst_w The width of the target osd
  */
static void disp_aria_osd_hscale_coeff_table_sel(mt_void *u32Param0,
        mt_u16 src_w,
        mt_u16 dst_w)
{
    mt_u32 table_index = 0;
    mt_u32 ratio_tenfold = 0;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;

    MT_ASSERT(0 != src_w * dst_w);

    ratio_tenfold = 10 * src_w / dst_w;
    if (ratio_tenfold <= 10)
    {
        table_index = SCALE_COEFF_TABLE_4_TAP_3;
    }
    else if (ratio_tenfold <= 15)
    {
        table_index = SCALE_COEFF_TABLE_4_TAP_1;
    }
    else
    {
        table_index = SCALE_COEFF_TABLE_4_TAP_2;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    //osd scale coeff table always table1_4
    drv_reg_4k_disp_set_osds_hf_coeff_addr(g_stGfxGPDevice[*pEnGpHalId].stCoeffTable[table_index].coeff_phy_addr);
    //load osd hscaler table
    drv_reg_4k_disp_set_coeff_table_sel_osd_hf_coeff_load_en(1);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    //osd scale coeff table always table1_4
    reg_symphony_optm_disp_set_osd_hf_coef_addr(g_stGfxGPDevice[*pEnGpHalId].stCoeffTable[table_index].coeff_phy_addr >> 3);
    //load osd hscaler table
    reg_symphony_optm_disp_set_vscaler_table_sel_osd_hori_table_sel(1);

#endif
}

/*!
  Scale the osd in horizontal.

  \param[in] src_w The src width in source osd layer.
  \param[in] dst_w The dst width.
  */
static void disp_aria_osd_hscale_set(mt_u16 src_w, mt_u16 dst_w)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    drv_reg_4k_disp_set_osds_hsize_osd_ori_hsize(src_w);
    drv_reg_4k_disp_set_osds_hsize_osd_dst_hsize(dst_w);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_symphony_optm_disp_set_osd_scale_hsize_osd_src_hsize(src_w);
    reg_symphony_optm_disp_set_osd_scale_hsize_osd_dst_hsize(dst_w);

#endif
}

/*!
  Scale the osd in vertical.

  \param[in] src_h The src height in source osd layer.
  \param[in] dst_h The dst height.
  */
static void disp_aria_osd_vscale_set(mt_u16 src_h, mt_u16 dst_h)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    drv_reg_4k_disp_set_osds_vsize_osd_ori_vsize(src_h);
    drv_reg_4k_disp_set_osds_vsize_osd_dst_vsize(dst_h);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_symphony_optm_disp_set_osd_vertical_size_osd_ori_vsize(src_h);
    reg_symphony_optm_disp_set_osd_vertical_size_osd_dst_vsize(dst_h);

#endif
}


static void disp_aria_osd_hscale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    rect_size_t osd_ori_size;
    static rect_size_t osd_orig_size_old;
    rect_vsb_t vout_hd_rect_cur = { 0 };
    static rect_vsb_t vout_hd_rect_old = { 0 };
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    OPTM_GetScreenRectFromDispInfo(&pstDispInfo->stDispInfo.stVirtaulScreen,
            (OPTM_GFX_OFFSET_S *)&pstDispInfo->stDispInfo.stOffsetInfo,
            &pstDispInfo->stDispInfo.stFmtResolution,
            &pstDispInfo->stDispInfo.stPixelFmtResolution,
            &g_stGfxGPDevice[*pEnGpHalId].stOutRect);

    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Width = 1280;
    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Height = 720;

    u32Left = pstDispInfo->stDispInfo.stOffsetInfo.u32Left;
    u32Right = pstDispInfo->stDispInfo.stOffsetInfo.u32Right;
    osd_ori_size.w =  g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Width + pstDispInfo->stDispInfo.stOffsetInfo.u32Left + pstDispInfo->stDispInfo.stOffsetInfo.u32Right;

    //get hd output size
    vout_hd_rect_cur.w = pstDispInfo->stDispInfo.stFmtResolution.s32Width;
    vout_hd_rect_cur.h = pstDispInfo->stDispInfo.stFmtResolution.s32Height;

    if(osd_ori_size.w != 0 || g_bGpCoefChangeFlag == MT_TRUE)
    {
        if ((vout_hd_rect_old.w != vout_hd_rect_cur.w) 
                || (osd_orig_size_old.w != osd_ori_size.w)
                || (g_bGpCoefChangeFlag == MT_TRUE))
        {
            disp_aria_osd_hscale_set(osd_ori_size.w, vout_hd_rect_cur.w);
            disp_aria_osd_hscale_coeff_table_sel(u32Param0, osd_ori_size.w, vout_hd_rect_cur.w);
            disp_aria_osd_hscale_ratio_set(osd_ori_size.w, vout_hd_rect_cur.w);

            if(osd_orig_size_old.w != osd_ori_size.w)
            {
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD0);
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD1);
                disp_aria_osd_rect_refresh(MTFB_LAYER_SUB);
            }

            vout_hd_rect_old.w = vout_hd_rect_cur.w;
            osd_orig_size_old.w = osd_ori_size.w;
        }
    }
}

/*!
  Osd vertical scale update.

  \param[in] p_dp priv handle.
  */
static void disp_aria_osd_vscale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    rect_size_t osd_ori_size;
    static rect_size_t osd_orig_size_old;
    rect_vsb_t vout_hd_rect_cur = { 0 };
    static rect_vsb_t vscale_rect_old = { 0 };
    mt_u32 in_scale_h = 0;
    mt_u32 out_scale_h = 0;
    mt_u32 out_interleave = 0;
    static mt_u32 out_interleave_old = 0;
    mt_u32 filter_phase_type = 0;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    OPTM_GetScreenRectFromDispInfo(&pstDispInfo->stDispInfo.stVirtaulScreen,
            (OPTM_GFX_OFFSET_S *)&pstDispInfo->stDispInfo.stOffsetInfo,
            &pstDispInfo->stDispInfo.stFmtResolution,
            &pstDispInfo->stDispInfo.stPixelFmtResolution,
            &g_stGfxGPDevice[*pEnGpHalId].stOutRect);

    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Width = 1280;
    //    g_stGfxGPDevice[*pEnGpHalId].stInRect.s32Height = 720;

    u32Top = pstDispInfo->stDispInfo.stOffsetInfo.u32Top;
    u32Bottom = pstDispInfo->stDispInfo.stOffsetInfo.u32Bottom;
    osd_ori_size.h = g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Height + pstDispInfo->stDispInfo.stOffsetInfo.u32Top + pstDispInfo->stDispInfo.stOffsetInfo.u32Bottom;

    //get hd output size
    vout_hd_rect_cur.w = pstDispInfo->stDispInfo.stFmtResolution.s32Width;
    vout_hd_rect_cur.h = pstDispInfo->stDispInfo.stFmtResolution.s32Height;

    //out_interleave = reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    //out_interleave = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    if (ps_DispExportFuncs && ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode)
    {
        out_interleave = ps_DispExportFuncs->reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    }
    else
    {
        MTFB_ERROR("disp func is null!\n");
    }

    if(osd_ori_size.h != 0)
    {
        if ((g_bGpCoefChangeFlag == MT_TRUE) || (vscale_rect_old.h != vout_hd_rect_cur.h) || (out_interleave_old != out_interleave) || (osd_orig_size_old.h != osd_ori_size.h))
        {
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
            if ((1 == drv_reg_4k_disp_get_osdl_osd0_cmd_force_progressive_mode()) || (1 == drv_reg_4k_disp_get_osdl_osd1_cmd_force_progressive_mode())
                    || (1 == drv_reg_4k_disp_get_osdl_sub_cmd_force_progressive_mode()))
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
                if((1 == reg_symphony_optm_disp_get_osd0_cmd_hd_force_progressive())
                        || (1 == reg_symphony_optm_disp_get_osd1_cmd_hd_force_progressive())
                        || (1 == reg_symphony_optm_disp_get_sub_cmd_hd_force_progressive()))
#endif
                {
                    //force input as progressive
                    in_scale_h = osd_ori_size.h;
                    if (1 == out_interleave)
                    {
                        //i out
                        out_scale_h = vout_hd_rect_cur.h / 2;
                    }
                    else
                    {
                        //p out
                        out_scale_h = vout_hd_rect_cur.h;
                    }

                }
                else
                {
                    //ratio calc as output, i/i or p/p
                    if (1 == out_interleave)
                    {
                        //i to i
                        in_scale_h = osd_ori_size.h / 2;
                        out_scale_h = vout_hd_rect_cur.h / 2;
                    }
                    else
                    {
                        //p to p
                        in_scale_h = osd_ori_size.h;
                        out_scale_h = vout_hd_rect_cur.h;
                    }
                }

            disp_aria_osd_vscale_set(in_scale_h, out_scale_h);
            filter_phase_type = disp_aria_osd_vscale_coeff_table_sel(u32Param0,
                    in_scale_h, out_scale_h);
            disp_aria_osd_vscale_ratio_set(in_scale_h, out_scale_h, filter_phase_type);
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
            if (in_scale_h == out_scale_h)
            {
                drv_reg_4k_disp_set_osds_cmd_osd_vert_bypass_en(1);
            }
            else
            {
                drv_reg_4k_disp_set_osds_cmd_osd_vert_bypass_en(0);
            }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
            if(in_scale_h == out_scale_h)
            {
                reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_bypass(1);
            }
            else
            {
                reg_symphony_optm_disp_set_osd_vertical_ctrl_osd_vert_bypass(0);
            }
#endif

            if(osd_orig_size_old.h != osd_ori_size.h)
            {
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD0);
                disp_aria_osd_rect_refresh(MTFB_LAYER_OSD1);
                disp_aria_osd_rect_refresh(MTFB_LAYER_SUB);
            }

            vscale_rect_old.h = vout_hd_rect_cur.h;
            out_interleave_old = out_interleave;
            osd_orig_size_old.h = osd_ori_size.h;
        }
    }
}

/*!
  Set still horizontal scaler paramters.

  \param[in] p_dp priv handle.
  \param[in] src_w The width of the source osd
  \param[in] dst_w The width of the target osd
  */
static void disp_aria_still_hscale_ratio_set(
        mt_u16 src_w,
        mt_u16 dst_w)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    mt_u32 hratio_int = 0;
    mt_u32 hratio_fra = 0;

    MT_ASSERT(0 != src_w * dst_w);

    // Please refer to Spec
    hratio_int = (src_w / dst_w) & 0xF;
    hratio_fra = (src_w % dst_w) * 4096 / dst_w;

    reg_aria_disp_set_still_scale_h_ratio_h_ratio_int(hratio_int);
    reg_aria_disp_set_still_scale_h_ratio_h_ratio_fra(hratio_fra);
#endif
}

/*!
  Set still vertical scaler paramters.

  \param[in] p_dp priv handle.
  \param[in] src_h The width of the source osd
  \param[in] dst_h The width of the target osd
  */
static void disp_aria_still_vscale_ratio_set(
        mt_u16 src_h,
        mt_u16 dst_h,
        mt_u32 v_phase_type)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    mt_u32 vratio_int = 0;
    mt_u32 vratio_fra = 0;
    mt_u32 vratio_int_1 = 0;
    mt_u32 vratio_fra_1 = 0;
    mt_u32 out_interleave = 0;
    mt_u32 input_p_i = 0;
    mt_u32 downsample_en = 0;
    mt_u32 temp, temp1, temp2, temp3;

    if ((src_h / dst_h) < 4)
    {
        downsample_en = 0;
        vratio_int = (src_h / dst_h) & 0xF;
        vratio_fra = (src_h % dst_h) * 4096 / dst_h;
    }
    else
    {
        downsample_en = 1;
        vratio_int = (src_h / (2 * dst_h)) & 0xF;
        vratio_fra = (src_h % (2 * dst_h)) * 4096 / (2 * dst_h);
    }

    vratio_int_1 = vratio_int / 2;
    vratio_fra_1 = vratio_fra / 2;

    //out_interleave = reg_aria_hd_encoder_get_basic_cfg_interlace_mode();
    out_interleave = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
    input_p_i = reg_aria_disp_get_still_control_progressive_mode();
    if ((input_p_i == 1) && (out_interleave == 1)) // p->i
    {
        temp = 0x2000 + (v_phase_type << 8) + (downsample_en << 4);
        temp1 = 2 + vratio_int_1;
        temp = temp + (temp1 << 16);
        reg_aria_disp_set_still_scale_ctrl(temp);

        temp = vratio_fra_1 << 16;
        reg_aria_disp_set_still_scale_v_start_fra(temp);
    }
    else if ((input_p_i == 1) && (out_interleave == 0)) // p->p
    {
        temp = 0x22000 + (v_phase_type << 8) + (downsample_en << 4);
        reg_aria_disp_set_still_scale_ctrl(temp);
        reg_aria_disp_set_still_scale_v_start_fra(0);
    }
    else if ((input_p_i == 0) && (out_interleave == 0)) //i->p
    {
        temp = 0x23000 + (v_phase_type << 8) + (downsample_en << 4);
        reg_aria_disp_set_still_scale_ctrl(temp);
        reg_aria_disp_set_still_scale_v_start_fra(0x8000000);
    } else //i->i
    {
        temp3 = (vratio_int << 12) + vratio_fra;
        temp3 = temp3 / 2;
        if (temp3 > 0x800)
        {
            temp = (temp3 - 0x800);
            temp1 = temp >> 12;   //int
            temp2 = temp & 0xfff; //fra
            temp = ((0x2 + temp1) << 16) + 0x2000 + (v_phase_type << 8) + (downsample_en << 4);
            reg_aria_disp_set_still_scale_ctrl(temp);

            temp2 = temp2 << 16;
            reg_aria_disp_set_still_scale_v_start_fra(temp2);
        }
        else
        {
            temp = (temp3 + 0x800);
            temp1 = temp >> 12;   //int
            temp2 = temp & 0xfff; //fra
            temp = ((0x2 + temp1) << 16) + 0x3000 + (v_phase_type << 8) + (downsample_en << 4);
            reg_aria_disp_set_still_scale_ctrl(temp);

            temp2 = temp2 << 16;
            reg_aria_disp_set_still_scale_v_start_fra(temp2);
        }
    }

    temp = (vratio_fra << 8) + vratio_int;
    reg_aria_disp_set_still_scale_v_ratio(temp);

    reg_aria_disp_set_still_scale_h_start_fra(0x800);

    reg_aria_disp_set_still_x_config(0);
    reg_aria_disp_set_still_y_config(0);
#endif
}

/*!
  Select still vertical scaler coeffcient table.

  \param[in] p_dp priv handle.
  \param[in] src_h The height of the source osd
  \param[in] dst_h The height of the target osd
  \param[out] filter phase type
  */
static mt_u32 disp_aria_still_vscale_coeff_table_sel(mt_void *u32Param0,
        mt_u16 src_h,
        mt_u16 dst_h)
{
    mt_u32 filter_phase_type = 0;

    return filter_phase_type;
}

/*!
  Select still horizontal scaler coeffcient table.

  \param[in] p_dp priv handle.
  \param[in] src_w The width of the source osd
  \param[in] dst_w The width of the target osd
  */
static void disp_aria_still_hscale_coeff_table_sel(mt_void *u32Param0,
        mt_u16 src_w,
        mt_u16 dst_w)
{
}

/*!
  Scale the still in horizontal.

  \param[in] src_w The src width in source still layer.
  \param[in] dst_w The dst width.
  */
static void disp_aria_still_hscale_set(mt_u16 src_w, mt_u16 dst_w)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    reg_aria_disp_set_still_scale_hsize(dst_w);
#endif
}

/*!
  Scale the still in vertical.

  \param[in] src_h The src height in source still layer.
  \param[in] dst_h The dst height.
  */
static void disp_aria_still_vscale_set(mt_u16 src_h, mt_u16 dst_h)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    reg_aria_disp_set_still_scale_vsize(dst_h);
#endif
}

/*!
  Still horizotal scale update.

  \param[in] p_dp priv handle.
  */
static void disp_aria_still_hscale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    rect_vsb_t still_in_rect_cur = { 0 };
    rect_vsb_t still_out_rect_cur = { 0 };
    static rect_vsb_t still_in_rect_old = { 0 };
    static rect_vsb_t still_out_rect_old = { 0 };

    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w = 1280;
    g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h = 720;

    still_in_rect_cur.w = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w;
    still_in_rect_cur.h = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h;
    still_out_rect_cur.w = 720;
    still_out_rect_cur.h = 576;
    if ((still_in_rect_old.w != still_in_rect_cur.w) || (still_out_rect_old.w != still_out_rect_cur.w))
    {
        disp_aria_still_hscale_set(still_in_rect_cur.w, still_out_rect_cur.w);
        disp_aria_still_hscale_coeff_table_sel(u32Param0,
                still_in_rect_cur.w,
                still_out_rect_cur.w);
        disp_aria_still_hscale_ratio_set(still_in_rect_cur.w, still_out_rect_cur.w);
        still_in_rect_old.w = still_in_rect_cur.w;
        still_out_rect_old.w = still_out_rect_cur.w;
    }
}

/*!
  Still vertical scale update.

  \param[in] p_dp priv handle.
  */
static void disp_aria_still_vscale_update(mt_void *u32Param0, mt_void *u32Param1)
{
    rect_vsb_t still_in_rect_cur = { 0 };
    rect_vsb_t still_out_rect_cur = { 0 };
    static rect_vsb_t still_in_rect_old = { 0 };
    static rect_vsb_t still_out_rect_old = { 0 };

    mt_u32 v_phase_type = 0;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w = 1280;
    g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h = 720;

    still_in_rect_cur.w = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.w;
    still_in_rect_cur.h = g_stGfxDevice[MTFB_LAYER_STILL].stInRect.h;
    still_out_rect_cur.w = 720;
    still_out_rect_cur.h = 576;
    if ((still_in_rect_old.h != still_in_rect_cur.h) || (still_out_rect_old.h != still_out_rect_cur.h))
    {
        disp_aria_still_vscale_set(still_in_rect_cur.h, still_out_rect_cur.h);
        v_phase_type = disp_aria_still_vscale_coeff_table_sel(u32Param0,
                still_in_rect_cur.h, still_out_rect_cur.h);
        disp_aria_still_vscale_ratio_set(still_in_rect_cur.h, still_out_rect_cur.h, v_phase_type);
        still_in_rect_old.h = still_in_rect_cur.h;
        still_out_rect_old.h = still_out_rect_cur.h;
    }
}
#endif

mt_s32 OPTM_DispInfoUpdate(OPTM_VDP_LAYER_GP_E enGPId);
mt_void OPTM_DispCallBack(mt_void *u32Param0, mt_void *u32Param1)
{
    //    OPTM_COLOR_SPACE_E enGpCsc;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;
    mt_u32 Ct;
    mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

    if(s_Refresh.refreshSyncFlag == 0)
    {
        s_Refresh.refreshSyncFlag = 1;
        wake_up_interruptible(&s_Refresh.refreshSyncEvent);
    }
    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    /**
     **u32Param0: GP ID
     **u32Param1: display information
     **/
    if (MT_NULL == pEnGpHalId || MT_NULL == pstDispInfo)
    {
        MTFB_WARNING("unable to handle null point in dispcallback\n");
        return;
    }

    if (!g_stGfxGPDevice[*pEnGpHalId].bOpen)  /** 判断GP是否已经打开，要是没有打开返回 **/
    {
        return;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    disp_symphony_osd_scale_update(u32Param0, u32Param1);
    disp_symphony_still_scale_update(u32Param0, u32Param1);
#else
    disp_aria_osd_hscale_update(u32Param0, u32Param1);

    disp_aria_osd_vscale_update(u32Param0, u32Param1);

    disp_aria_still_hscale_update(u32Param0, u32Param1);

    disp_aria_still_vscale_update(u32Param0, u32Param1);

#endif



    if(g_bGpCoefChangeFlag == MT_TRUE)
    {
        g_bGpCoefChangeFlag = MT_FALSE;
    }

    //if (MT_DRV_DISP_C_VT_INT == pstDispInfo->eEventType)     // in aria display ,no preprocess
    {
        OPTM_Distribute_Callback(u32Param0, u32Param1);
    }

    return;
}
#endif

#ifndef MT_BUILD_IN_BOOT
MT_DRV_DISPLAY_E OPTM_GfxChn2DispChn(OPTM_DISPCHANNEL_E enDispCh)
{
    if (OPTM_DISPCHANNEL_0 == enDispCh)
    {
        return MT_DRV_DISPLAY_0;
    }
    else if (OPTM_DISPCHANNEL_1 == enDispCh)
    {
        return MT_DRV_DISPLAY_1;
    }
    else
    {
        return MT_DRV_DISPLAY_BUTT;
    }
}
#endif
#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_SetCallbackToDisp(OPTM_GFX_GP_E enGPId, IntCallBack pCallBack, MT_DRV_DISP_CALLBACK_TYPE_E eType, MT_BOOL bFlag)
{
    mt_s32 s32Ret = MT_FAILURE;
    MT_DRV_DISPLAY_E enDisp;
    MT_DRV_DISP_CALLBACK_S stCallback;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %lx, %x, %x\n", __FUNCTION__, enGPId, (ulong)pCallBack, (mt_u32)eType, (mt_u32)bFlag);

    if (bFlag == g_stGfxGPIrq[enGPId].bRegistered[eType])  /** 判断是否注册 **/
    {
        return MT_SUCCESS;
    }

    if (eType >= MT_DRV_DISP_C_TYPE_BUTT)
    {
        MTFB_ERROR("Fail to set callback func!\n");
        return MT_FAILURE;
    }

    if (bFlag == g_stGfxGPIrq[enGPId].bRegistered[eType])
    {
        return MT_SUCCESS;
    }

    if (MT_NULL == pCallBack)
    {
        MTFB_ERROR("Unable to handle the null func point!\n");
        return MT_FAILURE;
    }

    enDisp = OPTM_GfxChn2DispChn(g_stGfxGPDevice[enGPId].enDispCh);

    stCallback.hDst = (mt_handle)(&g_stGfxGPDevice[enGPId].enGpHalId);
    stCallback.pfDISP_Callback = (mt_void *)pCallBack;

    //MTFB_INFO("[%s:%d]flag:%d\n", __FUNCTION__, __LINE__, bFlag);
    if (bFlag)
    {
        s32Ret = ps_DispExportFuncs->pfnDispRegCallback(enDisp, eType, &stCallback);
    }
    else
    {
        s32Ret = ps_DispExportFuncs->pfnDispUnRegCallback(enDisp, eType, &stCallback);
    }
    //MTFB_INFO("[%s:%d]flag:%d\n", __FUNCTION__, __LINE__, bFlag);

    if (MT_SUCCESS == s32Ret)
    {
        g_stGfxGPIrq[enGPId].bRegistered[eType] = bFlag;
    }

    OPTM_FUN_OUT;

    return s32Ret;
}
#endif

static mt_void OPTM_GfxSync_init(mt_void)
{
    s_Refresh.refreshSyncFlag = 0;
    init_waitqueue_head(&s_Refresh.refreshSyncEvent);
}

mt_s32 OPTM_GfxWaitSync(mt_void)
{ 
    mt_u32 Ct;
    mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

    s_Refresh.refreshSyncFlag = 0;
    wait_event_interruptible_hrtimeout(s_Refresh.refreshSyncEvent, s_Refresh.refreshSyncFlag, ms_to_ktime(1000));
    mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxOpenLayer(MTFB_LAYER_ID_E enLayerId,mt_u32 bEnableOsdc)
{
    mt_s32 s32Ret;
    OPTM_GFX_GP_E enGPId;

    OPTM_FUN_IN;
    MTFB_DEBUGK("%s: %d\n", __FUNCTION__, enLayerId);

    if (g_stGfxCap[enLayerId].bLayerSupported != MT_TRUE)
    {
        MTFB_ERROR("Gfx%d was not supported!\n", enLayerId);
        return MT_FAILURE;
    }

    if (g_stGfxDevice[enLayerId].bOpened == MT_TRUE)
    {
        MTFB_WARNING("info:Gfx%d was opened!\n", enLayerId);
        return MT_SUCCESS;
    }

#if defined(CONFIG_MT_CHIP_ARIA)
    p_optm_addr = (mt_u32)mt_get_display_base();
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    p_optm_addr = REG_SYMPHONY_OPTM_DISP_BASE;
#endif

    OPTM_GfxSync_init();

    s32Ret = OPTM_GfxInitLayer(enLayerId);
    if (s32Ret != MT_SUCCESS)
    {
        MTFB_ERROR("fail to init GFX%d!\n", enLayerId);
        return MT_FAILURE;
    }

    enGPId = g_stGfxDevice[enLayerId].enGPId;

    /**
     **打开GP设备
     **/
    s32Ret = OPTM_GPOpen(enGPId);
    if (MT_SUCCESS != s32Ret)
    {
        return MT_FAILURE;
    }

    /**
     **向disp注册两个回调函数
     **/
#ifndef MT_BUILD_IN_BOOT
    s32Ret = OPTM_SetCallbackToDisp(enGPId, (IntCallBack)OPTM_DispCallBack, MT_DRV_DISP_C_INTPOS_90_PERCENT, MT_TRUE);
    if (MT_SUCCESS != s32Ret)
    {
        MTFB_ERROR("Disp was not ready, open gfx%d failure!\n", enLayerId);
        return MT_FAILURE;
    }
    s32Ret = OPTM_SetCallbackToDisp(enGPId, (IntCallBack)OPTM_FrameEndCallBack, MT_DRV_DISP_C_INTPOS_100_PERCENT, MT_TRUE);
    if (MT_SUCCESS != s32Ret)
    {
        MTFB_ERROR("fail to register FrameEndCallBack\n");
        return MT_FAILURE;
    }

#endif

    /**
     **分配OSD控制字内存and 调色板内存
     **/
    {
        if ((enLayerId == MTFB_LAYER_OSD0) || (enLayerId == MTFB_LAYER_OSD1) || (enLayerId == MTFB_LAYER_SUB))
        {
            mt_char name[32];
            snprintf(name, sizeof(name), "MTFB_Fb%d_RgnHeader", enLayerId);
            /**
             ** apply region header buffer，存放控制字数据的
             **/
            if (OPTM_Adapt_AllocAndMap(name, OPTM_MMZ_ZONE, OPTM_HEAD_SIZE + OPTM_CMAP_SIZE, 0, &g_stGfxDevice[enLayerId].stRgnHeaderBuf) != MT_SUCCESS)
            {
                MTFB_ERROR("GFX Get header buffer failed!\n");
                return MT_FAILURE;
            }

            {
                ulong virAddr = 0;
                phys_addr_t phyAddr = 0;
                mt_u32 size = 0;
                virAddr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;
                phyAddr = g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr;
                size = g_stGfxDevice[enLayerId].stRgnHeaderBuf.size;
                MTFB_DEBUGK("OPTM_GfxOpenLayer: [virAddr:0x%lx][phyAddr0x%llx][size:0x%x][osdc:%d]\n", virAddr, phyAddr, size, bEnableOsdc);
#ifdef CFG_MTGO_PROC_SUPPORT
            	g_stMtgoProcInfo.headerAddr[enLayerId] = (phys_addr_t)phyAddr;
#endif
            }
            memset((mt_u8 *)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr, 0x0, g_stGfxDevice[enLayerId].stRgnHeaderBuf.size);

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
            /**
             ** apply compress output buffer
             **/
            if(bEnableOsdc)
            {
                g_stGfxDevice[enLayerId].bCmpCfg = MT_TRUE;
            }
            else
            {
                g_stGfxDevice[enLayerId].bCmpCfg = MT_FALSE;
            }                
            g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress = MT_FALSE; 
            if(g_stGfxDevice[enLayerId].bCmpCfg == MT_TRUE)   //for fpga verification, osd0 use compress
            {
                s32Ret = 0;

                snprintf(name, sizeof(name), "MTFB_Fb%d_OSD_CMPBuffer_A", enLayerId);
                s32Ret = OPTM_Adapt_AllocAndMap(name, OPTM_MMZ_ZONE, OPTM_COMPRESS_SIZE, 0, &g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A);

                snprintf(name, sizeof(name), "MTFB_Fb%d_OSD_CMPBuffer_R", enLayerId);
                s32Ret |= OPTM_Adapt_AllocAndMap(name, OPTM_MMZ_ZONE, OPTM_COMPRESS_SIZE, 0, &g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R);

                snprintf(name, sizeof(name), "MTFB_Fb%d_OSD_CMPBuffer_G", enLayerId);
                s32Ret |= OPTM_Adapt_AllocAndMap(name, OPTM_MMZ_ZONE, OPTM_COMPRESS_SIZE, 0, &g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G);
                
                snprintf(name, sizeof(name), "MTFB_Fb%d_OSD_CMPBuffer_B", enLayerId);
                s32Ret |= OPTM_Adapt_AllocAndMap(name, OPTM_MMZ_ZONE, OPTM_COMPRESS_SIZE, 0, &g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B);

                if (s32Ret != MT_SUCCESS)
                {
                    MTFB_ERROR("GFX Get compress output failed!\n");
                    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr != 0)
                    {
                        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A));
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startPhyAddr = 0;
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr = 0;
                    }
                    
                    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr != 0)
                    {
                        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R));
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startPhyAddr = 0;
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr = 0;
                    }

                    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr != 0)
                    {
                        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G));
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startPhyAddr = 0;
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr = 0;
                    }

                    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr != 0)
                    {
                        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B));
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startPhyAddr = 0;
                        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr = 0;
                    }

                    g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress = MT_FALSE;                
                }            
                else                
                {
                    ulong virAddr = 0;
                    phys_addr_t phyAddr = 0;
                    mt_u32 size = 0;
                    virAddr = (ulong)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr;
                    phyAddr = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startPhyAddr;
                    size = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.size;
                    MTFB_DEBUGK("OSD_CMPBuffer_A: [%lx][%llx][%d]\n", virAddr, phyAddr, size);

                    virAddr = (ulong)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr;
                    phyAddr = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startPhyAddr;
                    size = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.size;
                    MTFB_DEBUGK("OSD_CMPBuffer_R: [%lx][%llx][%d]\n", virAddr, phyAddr, size);
                    
                    virAddr = (ulong)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr;
                    phyAddr = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startPhyAddr;
                    size = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.size;
                    MTFB_DEBUGK("OSD_CMPBuffer_G: [%lx][%llx][%d]\n", virAddr, phyAddr, size);

                    virAddr = (ulong)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr;
                    phyAddr = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startPhyAddr;
                    size = g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.size;
                    MTFB_DEBUGK("OSD_CMPBuffer_B: [%lx][%llx][%d]\n", virAddr, phyAddr, size);

                    g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress = MT_TRUE;
                }
            }
#endif
            
        }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        switch (enLayerId)
        {
        case MTFB_LAYER_OSD0:
            drv_reg_4k_disp_set_osdl_osd0_ini_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr);
            break;
        case MTFB_LAYER_OSD1:
            drv_reg_4k_disp_set_osdl_osd1_ini_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr);
            break;
        case MTFB_LAYER_SUB:
            drv_reg_4k_disp_set_osdl_sub_ini_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr);
            break;
        case MTFB_LAYER_STILL:
            break;
        default:
            break;
        }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        switch (enLayerId)
        {
        case MTFB_LAYER_OSD0:
            reg_symphony_optm_disp_set_osd0_start_addr_hd_osd0_header_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr >> ALIGN_SHIFT);
            break;
        case MTFB_LAYER_OSD1:
            reg_symphony_optm_disp_set_osd1_start_addr_hd_osd1_header_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr >> ALIGN_SHIFT);
            break;
        case MTFB_LAYER_SUB:
            reg_symphony_optm_disp_set_sub_start_addr_hd_sub_header_addr(g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr >> ALIGN_SHIFT);
            break;
        case MTFB_LAYER_STILL:
            break;
        default:
            break;
        }

#endif
    }

    /**
     ** set layer open flag true 图层已经打开
     **/
    g_stGfxDevice[enLayerId].bOpened = MT_TRUE;

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

static mt_s32 OPTM_CheckGpState(OPTM_GFX_GP_E enGPId)
{
    mt_u32 i;
    mt_u32 u32LayerIdSta, u32LayerIdEnd;

    if (OPTM_GFX_GP_0 == enGPId)
    {
        u32LayerIdSta = MTFB_LAYER_BACKGROUND;
        u32LayerIdEnd = MTFB_LAYER_SUB;
    }
    else if (OPTM_GFX_GP_1 == enGPId)
    {
        u32LayerIdSta = MTFB_LAYER_SD_0;
        u32LayerIdEnd = MTFB_LAYER_SD_1;
    }
    else
    {
        return OPTM_DISABLE;
    }

    for (i = u32LayerIdSta; i <= u32LayerIdEnd; i++)
    {
        if (g_stGfxDevice[i].bOpened)
        {
            return OPTM_ENABLE;
        }
    }

    return OPTM_DISABLE;
}

mt_s32 OPTM_GfxCloseLayer(MTFB_LAYER_ID_E enLayerId)
{
    OPTM_GFX_GP_E enGPId;

    OPTM_FUN_IN;

    if (g_stGfxDevice[enLayerId].bOpened == MT_FALSE)
    {
        return MT_SUCCESS;
    }
    /* set layer disable, confirm hardware close */
    OPTM_GfxSetEnable(enLayerId, MT_FALSE);

    g_stGfxDevice[enLayerId].bOpened = MT_FALSE;

    enGPId = g_stGfxDevice[enLayerId].enGPId;

    OPTM_GfxDeInitLayer(enLayerId);

    g_stGfxDevice[enLayerId].bExtractLine = MT_FALSE;

    if (!OPTM_CheckGpState(enGPId))
    {
        OPTM_GPClose(enGPId);
    }

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
    OPTM_GFX_CMP_Close(enLayerId);
    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr = 0;
    }

    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr = 0;
    }

    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr = 0;
    }

    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr = 0;
    }

#endif

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_JudgeWbcEnable(mt_void)
{
    mt_u32 i;
    for (i = 0; i < MTFB_LAYER_SD_0; i++)
    {
        if (g_stGfxDevice[i].bEnable)
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

mt_s32 OPTM_GfxSetEnable(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
    OPTM_OSD_HEADER_S *p_header;
    mt_u32 cw_1;
    ulong rgn_addr;
    static MT_BOOL b_last_osd_state[MTFB_LAYER_ID_BUTT] = {0};
    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %x\n", __FUNCTION__, enLayerId, (mt_u32)bEnable);
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);
    g_stGfxDevice[enLayerId].bEnable = bEnable;
    p_header = &g_stGfxDevice[enLayerId].stRgnHeader;

    if(bEnable)
    {
        if(b_last_osd_state[enLayerId] != bEnable)
            b_osd_force_update = TRUE;
    }
    b_last_osd_state[enLayerId] = bEnable;
    
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:
        p_header->m_enable = bEnable;

        cw_1 = (p_header->m_truemode << 28) |
            (p_header->m_clutmode << 24) |
            (p_header->m_colormode << 20) |
            (p_header->m_alpha_pos << 19) |
            (p_header->m_alphamode << 16) | //0 use plane alpha, 1 use region alpha
            ((p_header->m_odd_only ? 1 : 0) << 12) |
            (p_header->m_semi_format << 10) |
            ((p_header->m_semi_enable ? 1 : 0) << 9) |
            ((p_header->m_follow ? 1 : 0) << 8) |
            (p_header->m_palette_endianmode << 5) |
            ((p_header->m_palette ? 1 : 0) << 4) |
            ((p_header->m_uv_change ? 1 : 0) << 3) |
            (p_header->m_endianmode << 1) |
            (p_header->m_enable ? 1 : 0);

        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        *((mt_u32 *)(rgn_addr + (4 * 0))) = cw_1;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#ifdef CFG_MTFB_COMPRESSION_SUPPORT
        if(g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress && bEnable)
        {
            OPTM_GFX_CMP_Open(enLayerId);            
        }
        else
        {
            OPTM_GFX_CMP_Close(enLayerId);
        }
#endif

        if (enLayerId == MTFB_LAYER_OSD0) {
            drv_reg_4k_disp_set_osdl_osd0_cmd_osd_layer_en(bEnable);
        } else if (enLayerId == MTFB_LAYER_OSD1) {
            drv_reg_4k_disp_set_osdl_osd1_cmd_osd_layer_en(bEnable);
        } else if (enLayerId == MTFB_LAYER_SUB) {
            drv_reg_4k_disp_set_osdl_sub_cmd_osd_layer_en(bEnable);
        }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        if (enLayerId == MTFB_LAYER_OSD0) {
            reg_symphony_optm_disp_set_osd0_cmd_hd_osd0_sel(bEnable);
        } else if (enLayerId == MTFB_LAYER_OSD1) {
            reg_symphony_optm_disp_set_osd1_cmd_hd_osd1_sel(bEnable);
        } else if (enLayerId == MTFB_LAYER_SUB) {
            reg_symphony_optm_disp_set_sub_cmd_hd_sub_sel(bEnable);
        }
#endif
        break;

    case MTFB_LAYER_STILL:
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        drv_reg_4k_disp_set_still_control_still_select(bEnable);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        reg_symphony_optm_disp_set_graphic_ctrl_still_sel_hd(bEnable);
#endif

        break;
    default:
        break;
    }

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetEnableWithoutWbcReg(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
    return MT_SUCCESS;
}
mt_s32 OPTM_GfxSetLayerAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Addr)
{
    OPTM_OSD_HEADER_S *p_header;
    mt_u32 cw_5, cw_7;
    ulong rgn_addr;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %x\n", __FUNCTION__, enLayerId,  u32Addr);
    /**check layer is opened**/                                                                                                                                                                          
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    g_stGfxDevice[enLayerId].NoCmpBufAddr = u32Addr;
    p_header = &g_stGfxDevice[enLayerId].stRgnHeader;

    OPTM_CheckGPMask_BYLayerID(enLayerId);

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:
        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        p_header->m_start_addr = u32Addr;
        /*!
          Control word 5
          ______________________________
          !                             |
          !          31:0               |
          !---------------------------- |
          !      osd_start_addr         |
          !_____________________________|
          */
        cw_5 = p_header->m_start_addr; 
        *((mt_u32 *)(rgn_addr + (4 * 4))) = cw_5;
        
        if(p_header->m_semi_enable)
        {
            p_header->m_uv_start_addr = u32Addr + p_header->m_pitch * (p_header->m_bottom - p_header->m_top);
            cw_7 = p_header->m_uv_start_addr;
            *((mt_u32 *)(rgn_addr + (4 * 6))) = cw_7;
        }

        break;

    case MTFB_LAYER_STILL:
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        drv_reg_4k_disp_set_still_luma_baseaddr(u32Addr>>4);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        reg_symphony_optm_disp_set_still_y_start_addr_hd_hd_still_y_start_addr(u32Addr >> ALIGN_SHIFT);
#endif

        break;
    default:
        break;
    }
    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetLayerStride(MTFB_LAYER_ID_E enLayerId, mt_u32 u32Stride)
{
    OPTM_OSD_HEADER_S *p_header;
    mt_u32 cw_4;
    ulong rgn_addr;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %x\n", __FUNCTION__, enLayerId, (mt_u32)u32Stride);
    /**check layer is opened**/                                                                                                                                                                           
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    g_stGfxDevice[enLayerId].Stride = (mt_u16)u32Stride;
    p_header = &g_stGfxDevice[enLayerId].stRgnHeader;

    OPTM_CheckGPMask_BYLayerID(enLayerId);

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:

        p_header->m_pitch = g_stGfxDevice[enLayerId].Stride;
        /*!
          Control word 4
          ____________________________________________________
          !                                                   |
          !   26:16    |     8              |      7:0        |
          !---------------------------------------------------|
          ! osd_pitch | pre_mul_alpha_blend | region_alpha_0  |
          !___________________________________________________|
          */
        //pitch is different, it is in pixel
        cw_4 = p_header->m_pitch;
        cw_4 <<= 8;
        cw_4 |= p_header->m_alpha_blendmode;
        cw_4 <<= 8;
        cw_4 |= p_header->m_alpha0;

        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        *((mt_u32 *)(rgn_addr + (4 * 3))) = cw_4;

        break;
    case MTFB_LAYER_STILL:
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        drv_reg_4k_disp_set_still_stride_still_stride(u32Stride);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        reg_symphony_optm_disp_set_still_stride_hd_hd_still_stride(u32Stride);
#endif

        break;
    default:
        break;
    }

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}
#ifndef MT_BUILD_IN_BOOT
MTFB_COLOR_FMT_E OPTM_HalFmtTransferToPixerFmt(OPTM_VDP_GFX_IFMT_E enDataFmt)
{
    if (enDataFmt >= VDP_GFX_IFMT_BUTT)
    {
        return MTFB_FMT_BUTT;
    }

    switch (enDataFmt)
    {
    case VDP_GFX_IFMT_CLUT_1BPP:
        return MTFB_FMT_1BPP;
    case VDP_GFX_IFMT_CLUT_2BPP:
        return MTFB_FMT_2BPP;
    case VDP_GFX_IFMT_CLUT_4BPP:
        return MTFB_FMT_4BPP;
    case VDP_GFX_IFMT_CLUT_8BPP:
        return MTFB_FMT_8BPP;
    case VDP_GFX_IFMT_ACLUT_44:
        return MTFB_FMT_ACLUT44;
    case VDP_GFX_IFMT_RGB_444:
        return MTFB_FMT_KRGB444;
    case VDP_GFX_IFMT_RGB_555:
        return MTFB_FMT_KRGB555;
    case VDP_GFX_IFMT_RGB_565:
        return MTFB_FMT_RGB565;
    case VDP_GFX_IFMT_PKG_UYVY:
        return MTFB_FMT_PUYVY;
    case VDP_GFX_IFMT_PKG_YUYV:
        return MTFB_FMT_PYUYV;
    case VDP_GFX_IFMT_PKG_YVYU:
        return MTFB_FMT_PYVYU;
    case VDP_GFX_IFMT_ACLUT_88:
        return MTFB_FMT_ACLUT88;
    case VDP_GFX_IFMT_RGB_888:
        return MTFB_FMT_RGB888;
    case VDP_GFX_IFMT_YCBCR_888:
        return MTFB_FMT_YUV888;
    case VDP_GFX_IFMT_ARGB_8565:
        return MTFB_FMT_ARGB8565;
    case VDP_GFX_IFMT_KRGB_888:
        return MTFB_FMT_KRGB888;
    case VDP_GFX_IFMT_ARGB_8888:
        return MTFB_FMT_ARGB8888;
    case VDP_GFX_IFMT_ARGB_4444:
        return MTFB_FMT_ARGB4444;
    case VDP_GFX_IFMT_ARGB_1555:
        return MTFB_FMT_ARGB1555;
    case VDP_GFX_IFMT_AYCBCR_8888:
        return MTFB_FMT_AYUV8888;
    case VDP_GFX_IFMT_RGBA_4444:
        return MTFB_FMT_RGBA4444;
    case VDP_GFX_IFMT_RGBA_5551:
        return MTFB_FMT_RGBA5551;
    case VDP_GFX_IFMT_RGBA_5658:
        return MTFB_FMT_RGBA5658;
    case VDP_GFX_IFMT_RGBA_8888:
        return MTFB_FMT_RGBA8888;
    case VDP_GFX_IFMT_YCBCRA_8888:
        return MTFB_FMT_YUVA8888;

    default:
        return MTFB_FMT_BUTT;
    }
}
#endif

OPTM_VDP_GFX_IFMT_E OPTM_PixerFmtTransferToHalFmt(MTFB_COLOR_FMT_E enDataFmt)
{
    if (enDataFmt >= MTFB_FMT_BUTT)
    {
        return VDP_GFX_IFMT_BUTT;
    }

    switch (enDataFmt)
    {
    case MTFB_FMT_1BPP:
        return VDP_GFX_IFMT_CLUT_1BPP;
    case MTFB_FMT_2BPP:
        return VDP_GFX_IFMT_CLUT_2BPP;
    case MTFB_FMT_4BPP:
        return VDP_GFX_IFMT_CLUT_4BPP;
    case MTFB_FMT_8BPP:
        return VDP_GFX_IFMT_CLUT_8BPP;
    case MTFB_FMT_ACLUT44:
        return VDP_GFX_IFMT_ACLUT_44;
    case MTFB_FMT_KRGB444:
        return VDP_GFX_IFMT_RGB_444;
    case MTFB_FMT_KRGB555:
        return VDP_GFX_IFMT_RGB_555;
    case MTFB_FMT_RGB565:
        return VDP_GFX_IFMT_RGB_565;
    case MTFB_FMT_PUYVY:
        return VDP_GFX_IFMT_PKG_UYVY;
    case MTFB_FMT_PYUYV:
        return VDP_GFX_IFMT_PKG_YUYV;
    case MTFB_FMT_PYVYU:
        return VDP_GFX_IFMT_PKG_YVYU;
    case MTFB_FMT_ACLUT88:
        return VDP_GFX_IFMT_ACLUT_88;
    case MTFB_FMT_RGB888:
        return VDP_GFX_IFMT_RGB_888;
    case MTFB_FMT_YUV888:
        return VDP_GFX_IFMT_YCBCR_888;
    case MTFB_FMT_ARGB8565:
        return VDP_GFX_IFMT_ARGB_8565;
    case MTFB_FMT_KRGB888:
        return VDP_GFX_IFMT_KRGB_888;
    case MTFB_FMT_ARGB8888:
        return VDP_GFX_IFMT_ARGB_8888;
    case MTFB_FMT_ARGB4444:
        return VDP_GFX_IFMT_ARGB_4444;
    case MTFB_FMT_ARGB1555:
        return VDP_GFX_IFMT_ARGB_1555;
    case MTFB_FMT_AYUV8888:
        return VDP_GFX_IFMT_AYCBCR_8888;
    case MTFB_FMT_RGBA4444:
        return VDP_GFX_IFMT_RGBA_4444;
    case MTFB_FMT_RGBA5551:
        return VDP_GFX_IFMT_RGBA_5551;
    case MTFB_FMT_RGBA5658:
        return VDP_GFX_IFMT_RGBA_5658;
    case MTFB_FMT_RGBA8888:
        return VDP_GFX_IFMT_RGBA_8888;
    case MTFB_FMT_YUVA8888:
        return VDP_GFX_IFMT_YCBCRA_8888;
    case MTFB_FMT_BGR565:
        return VDP_GFX_IFMT_RGB_565;
    case MTFB_FMT_BGR888:
        return VDP_GFX_IFMT_RGB_888;
    case MTFB_FMT_ABGR4444:
        return VDP_GFX_IFMT_ARGB_4444;
    case MTFB_FMT_ABGR1555:
        return VDP_GFX_IFMT_ARGB_1555;
    case MTFB_FMT_ABGR8888:
        return VDP_GFX_IFMT_ABGR_8888;
    case MTFB_FMT_ABGR8565:
        return VDP_GFX_IFMT_ARGB_8565;
    case MTFB_FMT_KBGR444:
        return VDP_GFX_IFMT_RGB_444;
    case MTFB_FMT_KBGR555:
        return VDP_GFX_IFMT_RGB_555;
    case MTFB_FMT_KBGR888:
        return VDP_GFX_IFMT_KRGB_888;
    default:
        return VDP_GFX_IFMT_BUTT;
    }

    return VDP_GFX_IFMT_BUTT;
}

mt_u32 OPTM_GetBppFromPixelFmt(MTFB_COLOR_FMT_E enDataFmt)
{
    switch (enDataFmt)
    {
    case MTFB_FMT_RGB565:
    case MTFB_FMT_KRGB444:
    case MTFB_FMT_KRGB555:
    case MTFB_FMT_ARGB4444:
    case MTFB_FMT_ARGB1555:
    case MTFB_FMT_RGBA4444:
    case MTFB_FMT_RGBA5551:
    case MTFB_FMT_ACLUT88:
    case MTFB_FMT_BGR565:
    case MTFB_FMT_ABGR1555:
    case MTFB_FMT_ABGR4444:
    case MTFB_FMT_KBGR444:
    case MTFB_FMT_KBGR555:
        {
            return 16;
        }
    case MTFB_FMT_RGB888:
    case MTFB_FMT_ARGB8565:
    case MTFB_FMT_RGBA5658:
    case MTFB_FMT_ABGR8565:
    case MTFB_FMT_BGR888:
        {
            return 24;
        }
    case MTFB_FMT_KRGB888:
    case MTFB_FMT_ARGB8888:
    case MTFB_FMT_RGBA8888:
    case MTFB_FMT_ABGR8888:
    case MTFB_FMT_KBGR888:
        {
            return 32;
        }
    case MTFB_FMT_1BPP:
        {
            return 1;
        }
    case MTFB_FMT_2BPP:
        {
            return 2;
        }
    case MTFB_FMT_4BPP:
        {
            return 4;
        }
    case MTFB_FMT_8BPP:
    case MTFB_FMT_ACLUT44:
        {
            return 8;
        }
    default:
        return 0;
    }
}

mt_s32 OPTM_GfxSetLayerDataFmt(MTFB_LAYER_ID_E enLayerId, MTFB_COLOR_FMT_E enDataFmt)
{
    OPTM_GFX_GP_E enGPId;
    OPTM_OSD_HEADER_S *p_header;
    mt_u32 cw_1;
    ulong rgn_addr;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %x\n", __FUNCTION__, enLayerId, (mt_u32)enDataFmt);
    /** check layer is opened **/                                                                                                                                                                          
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    if (!g_stGfxCap[enLayerId].bColFmt[enDataFmt])
    {
        MTFB_ERROR("unSupport color format.\n");
        return MT_FAILURE;
    }

    enGPId = g_stGfxDevice[enLayerId].enGPId;
    g_stGfxDevice[enLayerId].enDataFmt = enDataFmt;
    p_header = &g_stGfxDevice[enLayerId].stRgnHeader;

    OPTM_CheckGPMask_BYGPID(enGPId);

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:

        p_header->m_follow = MT_FALSE;
        p_header->m_rgnaddr_next = 0;

        if ((MTFB_LAYER_OSD0 == enLayerId) ||
                (MTFB_LAYER_OSD1 == enLayerId))
        {
            p_header->m_alpha0 = DISP_OSD_ALPHA0_DEFAULT;
            p_header->m_alphamode = DISP_OSD_PLANE_ALPHA_MODE;
        }
        else if (MTFB_LAYER_SUB == enLayerId)
        {
            p_header->m_alpha0 = DISP_SUB_ALPHA0_DEFAULT;
            p_header->m_alphamode = DISP_SUB_PLANE_ALPHA_MODE;
        }

        p_header->m_odd_only = MT_FALSE;
        p_header->m_semi_enable = MT_FALSE;
        p_header->m_uv_change = MT_FALSE;

        //set default endian mode, alpha pos, alpha blend
        p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_0;
        p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_0;
        p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_MSB;
        p_header->m_alpha_blendmode = DISP_OSD_ALPHA_BLEND_NO_PRE_MUL;

        switch (enDataFmt)
        {
        case MTFB_FMT_2BPP_ABGR:
        case MTFB_FMT_2BPP_RGBA:
            if(enDataFmt == MTFB_FMT_2BPP_ABGR)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_2BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_2BPP_BGRA:
        case MTFB_FMT_2BPP_ARGB:
            if(enDataFmt == MTFB_FMT_2BPP_BGRA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_2BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_2BPP_AVUY:
        case MTFB_FMT_2BPP_YUVA:
            if(enDataFmt == MTFB_FMT_2BPP_AVUY)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_2BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_2BPP_VUYA:
        case MTFB_FMT_2BPP_AYUV:
            if(enDataFmt == MTFB_FMT_2BPP_VUYA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_2BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_4BPP_ABGR:
        case MTFB_FMT_4BPP_RGBA:
            if(enDataFmt == MTFB_FMT_4BPP_ABGR)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_4BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_4BPP_BGRA:
        case MTFB_FMT_4BPP_ARGB:
            if(enDataFmt == MTFB_FMT_4BPP_BGRA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_4BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_4BPP_AVUY:
        case MTFB_FMT_4BPP_YUVA:
            if(enDataFmt == MTFB_FMT_4BPP_AVUY)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_4BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_4BPP_VUYA:
        case MTFB_FMT_4BPP_AYUV:
            if(enDataFmt == MTFB_FMT_4BPP_VUYA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_4BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_8BPP_ABGR:
        case MTFB_FMT_8BPP_RGBA:
            if(enDataFmt == MTFB_FMT_8BPP_ABGR)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_8BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_8BPP_BGRA:
        case MTFB_FMT_8BPP_ARGB:
            if(enDataFmt == MTFB_FMT_8BPP_BGRA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_8BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_8BPP_AVUY:
        case MTFB_FMT_8BPP_YUVA:
            if(enDataFmt == MTFB_FMT_8BPP_AVUY)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_8BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_8BPP_VUYA:
        case MTFB_FMT_8BPP_AYUV:
            if(enDataFmt == MTFB_FMT_8BPP_VUYA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_8BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT44_ABGR:
        case MTFB_FMT_ACLUT44_RGBA:
            if(enDataFmt == MTFB_FMT_ACLUT44_ABGR)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT44;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT44_BGRA:
        case MTFB_FMT_ACLUT44_ARGB:
        case MTFB_FMT_ACLUT44:
            if(enDataFmt == MTFB_FMT_ACLUT44_BGRA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                  
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT44;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT44_AVUY:
        case MTFB_FMT_ACLUT44_YUVA:
            if(enDataFmt == MTFB_FMT_ACLUT44_AVUY)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                   
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT44;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT44_VUYA:
        case MTFB_FMT_ACLUT44_AYUV:
            if(enDataFmt == MTFB_FMT_ACLUT44_VUYA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                      
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT44;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT88_ABGR:
        case MTFB_FMT_ACLUT88_RGBA:
            if(enDataFmt == MTFB_FMT_ACLUT88_ABGR)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                     
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT88;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT88_BGRA:
        case MTFB_FMT_ACLUT88_ARGB:
        case MTFB_FMT_ACLUT88:
            if(enDataFmt == MTFB_FMT_ACLUT88_BGRA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                      
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT88;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT88_AVUY:
        case MTFB_FMT_ACLUT88_YUVA:
            if(enDataFmt == MTFB_FMT_ACLUT88_AVUY)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                    
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT88;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_ACLUT88_VUYA:
        case MTFB_FMT_ACLUT88_AYUV:
            if(enDataFmt == MTFB_FMT_ACLUT88_VUYA)
                p_header->m_palette_endianmode = DISP_OSD_PALETTE_ENDIAN_MODE_3;                    
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_LUT88;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_RGB233:
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB233;
            break;
        case MTFB_FMT_RGB565_SMALL_ENDIAN:
        case MTFB_FMT_RGB565:
            if(enDataFmt == MTFB_FMT_RGB565_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;                    
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB565;
            break;
        case MTFB_FMT_ARGB1555_SMALL_ENDIAN:
        case MTFB_FMT_ARGB1555:
            if(enDataFmt == MTFB_FMT_ARGB1555_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;                   
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB1555;
            break;
        case MTFB_FMT_RGBA5551_SMALL_ENDIAN:
        case MTFB_FMT_RGBA5551:
            if(enDataFmt == MTFB_FMT_RGBA5551_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;                      
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB1555;
            break;
        case MTFB_FMT_ARGB4444_SMALL_ENDIAN:
        case MTFB_FMT_ARGB4444:
            if(enDataFmt == MTFB_FMT_ARGB4444_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;                   
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB4444;
            break;
        case MTFB_FMT_RGBA4444_SMALL_ENDIAN:
        case MTFB_FMT_RGBA4444:
            if(enDataFmt == MTFB_FMT_RGBA4444_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;                   
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB4444;
            break;
        case MTFB_FMT_PYVYU:
        case MTFB_FMT_PUYVY:
            if(enDataFmt == MTFB_FMT_PYVYU)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_3;                   
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_UYVY;
            break;
        case MTFB_FMT_PYUYV:
            p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_1;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_UYVY;
            break;
        case MTFB_FMT_PVYUY:
            p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_2;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_UYVY;
            break;
        case MTFB_FMT_ARGB8888_SMALL_ENDIAN:
        case MTFB_FMT_ARGB8888:
            if(enDataFmt == MTFB_FMT_ARGB8888_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_3;                  
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB8888;
            break;
        case MTFB_FMT_RGBA8888_SMALL_ENDIAN:
        case MTFB_FMT_RGBA8888:
            if(enDataFmt == MTFB_FMT_RGBA8888_SMALL_ENDIAN)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_3;                      
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGBTRUE;
            p_header->m_truemode = DISP_OSD_TRUECODE_RGB8888;
            break;
        case MTFB_FMT_VUYA8888:
        case MTFB_FMT_AYUV8888:
            if(enDataFmt == MTFB_FMT_VUYA8888)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_3;                   
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_AYUV8888;
            break;
        case MTFB_FMT_AVUY8888:
        case MTFB_FMT_YUVA8888:
            if(enDataFmt == MTFB_FMT_AVUY8888)
                p_header->m_endianmode = DISP_OSD_ENDIAN_MODE_3;                   
            p_header->m_alpha_pos = DISP_OSD_ALPHA_POS_LSB;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_AYUV8888;
            break;
        case MTFB_FMT_SP_YUV420:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV420_1x1;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            break;
        case MTFB_FMT_SP_YUV422_1x2:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV422_1x2;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            break;
        case MTFB_FMT_SP_YUV422_2x1:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV422_2x1;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            break;
        case MTFB_FMT_SP_YUV420_UVSWAP:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV420_1x1;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            p_header->m_uv_change = MT_TRUE;
            break;
        case MTFB_FMT_SP_YUV422_1x2_UVSWAP:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV422_1x2;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            p_header->m_uv_change = MT_TRUE;
            break;
        case MTFB_FMT_SP_YUV422_2x1_UVSWAP:
            p_header->m_semi_enable = MT_TRUE;
            p_header->m_semi_format = DISP_SP_YUV422_2x1;
            p_header->m_colormode = DISP_OSD_COLORSPACE_YUV422_444;
            p_header->m_truemode = DISP_OSD_TRUECODE_SP;
            p_header->m_uv_change = MT_TRUE;
            break;
        case MTFB_FMT_1BPP:
            break;
        case MTFB_FMT_2BPP:
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_2BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_4BPP:
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_4BIT;
            p_header->m_palette = MT_TRUE;
            break;
        case MTFB_FMT_8BPP:
            p_header->m_colormode = DISP_OSD_COLORSPACE_RGB;
            p_header->m_clutmode = DISP_OSD_CLUTCODE_8BIT;
            p_header->m_palette = MT_TRUE;
            break;
        default:
            break;
        }

        /*!
          Control word 1
          _____________________________________________________________________________________________________________________
          !                                       |
          ! 30:28  | 26:24  | 21:20 | 19   |  16  |    12  |   11:10   |   9   |   8  |   6:5     | 4  |     3   |  2:1   | 0  |
          !---------------------------------- -------------------------------------------------------------------------------- |
          !true_mod|clut_mod|col_mod|aa_pos|aa_mod|odd only|semi_format|semi_en|follow|pale endian|pale|uv_change|osd endian|en|
          !____________________________________________________________________________________________________________________

*/
        cw_1 = (p_header->m_truemode << 28) |
            (p_header->m_clutmode << 24) |
            (p_header->m_colormode << 20) |
            (p_header->m_alpha_pos << 19) |
            (p_header->m_alphamode << 16) | //0 use plane alpha, 1 use region alpha
            ((p_header->m_odd_only ? 1 : 0) << 12) |
            (p_header->m_semi_format << 10) |
            ((p_header->m_semi_enable ? 1 : 0) << 9) |
            ((p_header->m_follow ? 1 : 0) << 8) |
            (p_header->m_palette_endianmode << 5) |
            ((p_header->m_palette ? 1 : 0) << 4) |
            ((p_header->m_uv_change ? 1 : 0) << 3) |
            (p_header->m_endianmode << 1) |
            (p_header->m_enable ? 1 : 0);

        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        *((mt_u32 *)(rgn_addr + (4 * 0))) = cw_1;

        break;

    case MTFB_LAYER_STILL:
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        switch (enDataFmt)
        {
        case MTFB_FMT_PUYVY:
            drv_reg_4k_disp_set_still_control_still_format(0);
            break;
        case MTFB_FMT_AYUV8888:
            drv_reg_4k_disp_set_still_control_still_format(1);
            break;
        case MTFB_FMT_SP_YUV420:
            drv_reg_4k_disp_set_still_control_still_format(2);
            break;
        case MTFB_FMT_SP_YUV422_1x2:
            drv_reg_4k_disp_set_still_control_still_format(3);
            break;
        case MTFB_FMT_SP_YUV422_2x1:
            drv_reg_4k_disp_set_still_control_still_format(4);
            break;
        default:
            break;
        }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        switch (enDataFmt)
        {
        case MTFB_FMT_PUYVY:
            reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(0);
            reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(3);
            reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(1);
            reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(2);
            break;
        case MTFB_FMT_AYUV8888:
            reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(1);
            reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(1);
            reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(0);
            reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(2);
            break;
        case MTFB_FMT_SP_YUV420:
            reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(2);
            reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(1);
            reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(0);
            reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(2);
            reg_symphony_optm_disp_set_still_stride_hd_cr_first(1);
            break;
        case MTFB_FMT_SP_YUV422_1x2:
            reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(3);
            reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(1);
            reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(0);
            reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(2);
            reg_symphony_optm_disp_set_still_stride_hd_cr_first(1);
            break;
        case MTFB_FMT_SP_YUV422_2x1:
            reg_symphony_optm_disp_set_graphic_ctrl_still_hd_format(4);
            reg_symphony_optm_disp_set_still_stride_hd_cb_byte_sel(1);
            reg_symphony_optm_disp_set_still_stride_hd_cr_byte_sel(0);
            reg_symphony_optm_disp_set_still_stride_hd_y_byte_sel(2);
            reg_symphony_optm_disp_set_still_stride_hd_cr_first(1);
            break;
        default:
            break;
        }

#endif
        break;
    default:
        break;
    }

    OPTM_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetColorReg(MTFB_LAYER_ID_E enLayerId, mt_u32 u32OffSet, mt_u32 u32Color, mt_u32 UpFlag)
{
    ulong rgn_addr;

    //OPTM_FUN_IN;
    /** check layer is opened **/                                                                                                                                                                          
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;
    *((mt_u32 *)(rgn_addr + (4 * 16) + (4 *u32OffSet))) = u32Color;

    //OPTM_FUN_OUT;
    return MT_SUCCESS;
}

#ifdef OPTM_MTFB_WVM_ENABLE
mt_void OPTM_GfxWVBCallBack(mt_u32 enLayerId, mt_u32 u32Param1)
{

    return;
}

mt_s32 OPTM_GfxWaitVBlank(MTFB_LAYER_ID_E enLayerId)
{
    mt_s32 ret = 0;     
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:  
        s_Refresh.vsync_flag = 0;
        ret = wait_event_interruptible_timeout(s_Refresh.vsync_wait,
            s_Refresh.vsync_flag  != 0,
            HZ/10);
        
        if (ret < 0)   
        {
            MTFB_DEBUGK("GFX ERROR! <%s> : <%d> : enLayerId (%d) invalid\n", __FUNCTION__, __LINE__, enLayerId);
            return ret;
         }
        if (ret == 0)
        {
            MTFB_DEBUGK("GFX ERROR! <%s> : <%d> : enLayerId (%d) wait v blank timeout\n", __FUNCTION__, __LINE__, enLayerId);
           return -ETIMEDOUT;
         }
        break;
    default:        
        MTFB_DEBUGK("GFX ERROR! <%s> : <%d> : enLayerId (%d) invalid\n", __FUNCTION__, __LINE__, enLayerId);
        ret = -1;
        break;    
    }
    return ret;  
}

#else
mt_s32 OPTM_GfxWaitVBlank(MTFB_LAYER_ID_E enLayerId)
{
    MTFB_ERROR("GFX ERROR! NOT enable wait v blank\n");
    return MT_FAILURE;
}
#endif

mt_s32 OPTM_GfxSetLayerDeFlicker(MTFB_LAYER_ID_E enLayerId, MTFB_DEFLICKER_S *pstDeFlicker)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetLayerAlpha(MTFB_LAYER_ID_E enLayerId, MTFB_ALPHA_S *pstAlpha)
{
    OPTM_GFX_GP_E enGpId;
    mt_u8 alpha;
    MT_BOOL b_on;
    OPTM_OSD_HEADER_S *p_header;
    mt_u32 cw_1;
    mt_u32 cw_4;
    ulong rgn_addr;
    mt_u8 rgn_alpha;
    MT_BOOL b_rgn_on;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d %lx\n", __FUNCTION__, enLayerId, (ulong)pstAlpha);
    /** check layer is opened **/                                                                                                                                                                          
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    enGpId = g_stGfxDevice[enLayerId].enGPId;
    alpha = pstAlpha->u8GlobalAlpha;
    b_on = pstAlpha->bAlphaEnable;
    rgn_alpha = pstAlpha->u8RegionAlpha;
    b_rgn_on = pstAlpha->bRegionAlphaEnable;

    p_header = &g_stGfxDevice[enLayerId].stRgnHeader;

    memcpy(&g_stGfxDevice[enLayerId].stAlpha, pstAlpha, sizeof(MTFB_ALPHA_S));

    /** 是否已经开机过渡完 **/
    OPTM_CheckGPMask_BYLayerID(enLayerId);


    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:

        p_header->m_alphamode = b_rgn_on;
        p_header->m_alpha0 = rgn_alpha;


        cw_1 = (p_header->m_truemode << 28) |
            (p_header->m_clutmode << 24) |
            (p_header->m_colormode << 20) |
            (p_header->m_alpha_pos << 19) |
            (p_header->m_alphamode << 16) | //0 use plane alpha, 1 use region alpha
            ((p_header->m_odd_only ? 1 : 0) << 12) |
            (p_header->m_semi_format << 10) |
            ((p_header->m_semi_enable ? 1 : 0) << 9) |
            ((p_header->m_follow ? 1 : 0) << 8) |
            (p_header->m_palette_endianmode << 5) |
            ((p_header->m_palette ? 1 : 0) << 4) |
            ((p_header->m_uv_change ? 1 : 0) << 3) |
            (p_header->m_endianmode << 1) |
            (p_header->m_enable ? 1 : 0);

        /*!
          Control word 4
          ____________________________________________________
          !                                                   |
          !   26:16    |     8              |      7:0        |
          !---------------------------------------------------|
          ! osd_pitch | pre_mul_alpha_blend | region_alpha_0  |
          !___________________________________________________|
          */
        //pitch is different, it is in pixel
        cw_4 = p_header->m_pitch;
        cw_4 <<= 8;
        cw_4 |= p_header->m_alpha_blendmode;
        cw_4 <<= 8;
        cw_4 |= p_header->m_alpha0;

        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        *((mt_u32 *)(rgn_addr + (4 * 0))) = cw_1;

        *((mt_u32 *)(rgn_addr + (4 * 3))) = cw_4;

        break;
    case MTFB_LAYER_STILL:
        break;
    default:
        break;

    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        drv_reg_4k_disp_set_osdl_osd0_cmd_osd_plane_alpha_en(b_on);
        drv_reg_4k_disp_set_osdl_osd0_cmd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_OSD1:
        drv_reg_4k_disp_set_osdl_osd1_cmd_osd_plane_alpha_en(b_on);
        drv_reg_4k_disp_set_osdl_osd1_cmd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_SUB:
        drv_reg_4k_disp_set_osdl_sub_cmd_osd_plane_alpha_en(b_on);
        drv_reg_4k_disp_set_osdl_sub_cmd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_STILL:
        break;
    default:
        break;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        reg_symphony_optm_disp_set_osd0_cmd_hd_plane_alpha_en(b_on);
        reg_symphony_optm_disp_set_osd0_cmd_hd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_OSD1:
        reg_symphony_optm_disp_set_osd1_cmd_hd_plane_alpha_en(b_on);
        reg_symphony_optm_disp_set_osd1_cmd_hd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_SUB:
        reg_symphony_optm_disp_set_sub_cmd_hd_plane_alpha_en(b_on);
        reg_symphony_optm_disp_set_sub_cmd_hd_plane_alpha(alpha);
        break;
    case MTFB_LAYER_STILL:
        break;
    default:
        break;
    }

#endif
    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxGetLayerRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect)
{
    OPTM_FUN_IN;

    memcpy(pstRect, &g_stGfxDevice[enLayerId].stInRect, sizeof(MTFB_RECT));

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxConfigCursorRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetLayerRect(MTFB_LAYER_ID_E enLayerId, const MTFB_RECT *pstRect)
{
    //    OPTM_VDP_DISP_RECT_S stGfxRect;
    mt_s32 left, right, top, bottom;
    mt_u32 cw_2, cw_3;
    ulong rgn_addr;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d, %d, [%d][%d][%d][%d]\n", __FUNCTION__, __LINE__, enLayerId, (mt_u32)pstRect->x, (mt_u32)pstRect->y, (mt_u32)pstRect->w, (mt_u32)pstRect->h);
    /** check layer is opened **/                                                                                                                                                                          
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    /**
     **保存输入矩形
     **/
    g_stGfxDevice[enLayerId].stInRect.x = pstRect->x;
    g_stGfxDevice[enLayerId].stInRect.y = pstRect->y;
    g_stGfxDevice[enLayerId].stInRect.w = pstRect->w;
    g_stGfxDevice[enLayerId].stInRect.h = pstRect->h;
    g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32X = pstRect->x;
    g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Y = pstRect->y;
    g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Width = pstRect->w;
    g_stGfxGPDevice[OPTM_GFX_GP_0].stInRect.s32Height = pstRect->h;

    /**
     **是否完成开机过渡
     **/
    OPTM_CheckGPMask_BYLayerID(enLayerId);

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
    case MTFB_LAYER_OSD1:
    case MTFB_LAYER_SUB:

        g_stGfxDevice[enLayerId].stRgnHeader.m_left = pstRect->x + u32Left + 1;
        g_stGfxDevice[enLayerId].stRgnHeader.m_right = g_stGfxDevice[enLayerId].stRgnHeader.m_left + pstRect->w  - 1;
        g_stGfxDevice[enLayerId].stRgnHeader.m_top = pstRect->y + u32Top + 1;
        g_stGfxDevice[enLayerId].stRgnHeader.m_bottom = g_stGfxDevice[enLayerId].stRgnHeader.m_top + pstRect->h  - 1;

        /*!
          Control word 2
          ____________________________
          !                           |
          !    26:16    |     10:0    |
          !---------------------------|
          ! osd_start_y | osd_start_x |
          !___________________________|
          */
        cw_2 = g_stGfxDevice[enLayerId].stRgnHeader.m_top;
        cw_2 <<= 16;
        cw_2 |= g_stGfxDevice[enLayerId].stRgnHeader.m_left;

        /*!
          Control word 3
          ________________________
          !                       |
          !   26:16   |    10:0   |
          !-----------------------|
          ! osd_end_y | osd_end_x |
          !_______________________|
          */
        cw_3 = g_stGfxDevice[enLayerId].stRgnHeader.m_bottom;
        cw_3 <<= 16;
        cw_3 |= g_stGfxDevice[enLayerId].stRgnHeader.m_right;

        rgn_addr = (ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;

        *((mt_u32 *)(rgn_addr + (4 * 1))) = cw_2;
        *((mt_u32 *)(rgn_addr + (4 * 2))) = cw_3;

        *((mt_u32 *)(rgn_addr + (4 * 7))) = 0;

        break;

    case MTFB_LAYER_STILL:
        left = pstRect->x;
        top = pstRect->y;

        right = left + pstRect->w;
        bottom = top + pstRect->h;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        drv_reg_4k_disp_set_still_read_x_cfg_still_read_x_start(left);
        drv_reg_4k_disp_set_still_read_x_cfg_still_read_x_end(right - 1);
        drv_reg_4k_disp_set_still_read_y_cfg_still_read_y_start(top);
        drv_reg_4k_disp_set_still_read_y_cfg_still_read_y_end(bottom - 1);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
        reg_symphony_optm_disp_set_still_x_hd_hd_still_startx(left + 1);
        reg_symphony_optm_disp_set_still_x_hd_hd_still_endx(right);
        reg_symphony_optm_disp_set_still_y_hd_hd_still_starty(top + 1);
        reg_symphony_optm_disp_set_still_y_hd_hd_still_endy(bottom);
#endif
        break;
    default:
        break;
    }

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}
#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_GfxSetGpInPutSize(OPTM_GFX_GP_E enGpId, mt_u32 u32Width, mt_u32 u32Height)
{
    g_stGfxGPDevice[enGpId].stInRect.s32Width = u32Width;
    g_stGfxGPDevice[enGpId].stInRect.s32Height = u32Height;
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxGetDispFMTSize(OPTM_GFX_GP_E enGpId, MTFB_RECT *pstOutRect)
{
    pstOutRect->x = g_stGfxGPDevice[enGpId].stOutRect.s32X;
    pstOutRect->y = g_stGfxGPDevice[enGpId].stOutRect.s32Y;
    pstOutRect->w = g_stGfxGPDevice[enGpId].stOutRect.s32Width;
    pstOutRect->h = g_stGfxGPDevice[enGpId].stOutRect.s32Height;

    if (pstOutRect->w == 0 || pstOutRect->h == 0)
    {
        pstOutRect->x = 0;
        pstOutRect->y = 0;
        pstOutRect->w = 1280;
        pstOutRect->h = 720;
    }

    return MT_SUCCESS;
}
#endif
mt_s32 OPTM_GfxSetDispFMTSize(OPTM_GFX_GP_E enGpId, const mt_rect_s *pstOutRect)
{
    MTFB_RECT stInputRect;

    OPTM_FUN_IN;

    if (pstOutRect->s32X < 0 || pstOutRect->s32Y < 0)
    {
        return MT_FAILURE;
    }

    if (pstOutRect->s32Width <= 0 || pstOutRect->s32Height <= 0)
    {
        return MT_FAILURE;
    }

    g_stGfxGPDevice[enGpId].stOutRect.s32X = pstOutRect->s32X;
    g_stGfxGPDevice[enGpId].stOutRect.s32Y = pstOutRect->s32Y;

    g_stGfxGPDevice[enGpId].stOutRect.s32Width = pstOutRect->s32Width;
    g_stGfxGPDevice[enGpId].stOutRect.s32Height = pstOutRect->s32Height;

    //MTFB_INFO("===OPTM_GfxSetDispFMTSize==ID %d,w %d,h %d===.\n",enGpId,pstOutRect->s32Width,pstOutRect->s32Height);

    OPTM_CheckGPMask_BYGPID(enGpId);

    stInputRect.x = g_stGfxGPDevice[enGpId].stInRect.s32X;
    stInputRect.y = g_stGfxGPDevice[enGpId].stInRect.s32Y;
    stInputRect.w = g_stGfxGPDevice[enGpId].stInRect.s32Width;
    stInputRect.h = g_stGfxGPDevice[enGpId].stInRect.s32Height;

    if (stInputRect.w && stInputRect.h)
    {
        OPTM_GfxSetGpRect(enGpId, &stInputRect);
    }

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}
#define SHARPEN_RATIO 3
#define SHARPEN_MAX_WIDTH 1920
mt_s32 OPTM_GfxSetGpRect(OPTM_GFX_GP_E enGpId, const MTFB_RECT *pstInputRect)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetGpDeflicker(OPTM_GFX_GP_E enGpId, MT_BOOL bDeflicker)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxGetOutRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstOutputRect)
{
    OPTM_GFX_GP_E enGpId;
    OPTM_FUN_IN;

    pstOutputRect->x = 0;
    pstOutputRect->y = 0;
    enGpId = g_stGfxDevice[enLayerId].enGPId;
    pstOutputRect->w = g_stGfxGPDevice[enGpId].stInRect.s32Width;
    pstOutputRect->h = g_stGfxGPDevice[enGpId].stInRect.s32Height;

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetLayKeyMask(MTFB_LAYER_ID_E enLayerId, const MTFB_COLORKEYEX_S *pstColorkey)
{
    OPTM_VDP_GFX_CKEY_S ckey_info;
    OPTM_VDP_GFX_MASK_S ckey_mask;

    OPTM_FUN_IN;

    MTFB_DEBUGK("%s: %d %lx\n", __FUNCTION__, enLayerId, (ulong)pstColorkey);

    /** 判断该图层是否打开 **/
    D_OPTM_MTFB_CheckGfxOpen(enLayerId);

    memset(&ckey_info, 0, sizeof(ckey_info));
    memset(&ckey_mask, 0, sizeof(ckey_mask));

    memcpy(&g_stGfxDevice[enLayerId].stColorkey, pstColorkey, sizeof(MTFB_COLORKEYEX_S));

    /** 是否已经开机logo过渡完 **/
    OPTM_CheckGPMask_BYLayerID(enLayerId);
#if defined(CONFIG_MT_CHIP_SYMPHONY6)

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        drv_reg_4k_disp_set_osdm_osd0_ckey_osd0_colorkey_en(pstColorkey->bKeyEnable);
        drv_reg_4k_disp_set_osdm_osd0_ckey_colorkey_value(pstColorkey->u32Key);
        break;
    case MTFB_LAYER_OSD1:
        drv_reg_4k_disp_set_osdm_osd1_ckey_osd1_colorkey_en(pstColorkey->bKeyEnable);
        drv_reg_4k_disp_set_osdm_osd1_ckey_colorkey_value(pstColorkey->u32Key);
        break;
    case MTFB_LAYER_SUB:
        break;
    case MTFB_LAYER_STILL:
        break;
    default:
        break;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        reg_symphony_optm_disp_set_osd0_ck_hd_ck_en(pstColorkey->bKeyEnable);
        reg_symphony_optm_disp_set_osd0_ck_hd_ck_yuv(pstColorkey->u32Key);
        break;
    case MTFB_LAYER_OSD1:
        reg_symphony_optm_disp_set_osd1_ck_hd_ck_en(pstColorkey->bKeyEnable);
        reg_symphony_optm_disp_set_osd1_ck_hd_ck_yuv(pstColorkey->u32Key);
        break;
    case MTFB_LAYER_SUB:
        break;
    case MTFB_LAYER_STILL:
        break;
    default:
        break;
    }

#endif

    OPTM_FUN_OUT;

    return MT_SUCCESS;
}

/*  superposition */
mt_s32 OPTM_GfxSetLayerPreMult(MTFB_LAYER_ID_E enLayerId, MT_BOOL bEnable)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetClutAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32PhyAddr)
{
    return MT_SUCCESS;
}

#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_GFX_GetSlvLayerInfo(MTFB_SLVLAYER_DATA_S *pstLayerInfo)
{
    pstLayerInfo->enLayerID = OPTM_SLAVER_LAYERID;

    return MT_SUCCESS;
}

mt_s32 OPTM_GfxGetOSDData(MTFB_LAYER_ID_E enLayerId, MTFB_OSD_DATA_S *pstLayerData)
{
    return MT_SUCCESS;
}
#endif

mt_s32 OPTM_GfxUpLayerReg(MTFB_LAYER_ID_E enLayerId)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_OpenWbc2(OPTM_GFX_WBC_S *pstWbc2)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_CloseWbc2(OPTM_GFX_WBC_S *pstWbc2)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxConfigSlvLayer(MTFB_LAYER_ID_E enLayerId,
        mt_rect_s *pstRect)
{
    return MT_SUCCESS;
}

mt_void OPTM_Wbc2Isr(mt_void *u32Param0, mt_void *u32Param1)
{
    return;
}

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
mt_s32 OPTM_GfxSetTriDimEnable(MTFB_LAYER_ID_E enLayerId, mt_u32 bEnable)
{
    return MT_SUCCESS;
}

OPTM_VDP_DISP_MODE_E OPTM_GfxGetHalTriDimMode(MTFB_STEREO_MODE_E enMode)
{
    switch (enMode)
    {
    case MTFB_STEREO_MONO:
        return VDP_DISP_MODE_2D;
    case MTFB_STEREO_SIDEBYSIDE_HALF:
        return VDP_DISP_MODE_SBS;
    case MTFB_STEREO_TOPANDBOTTOM:
        return VDP_DISP_MODE_TAB;
    case MTFB_STEREO_FRMPACKING:
        return VDP_DISP_MODE_FP;
    default:
        return VDP_DISP_MODE_BUTT;
    }

    return VDP_DISP_MODE_BUTT;
}

mt_s32 OPTM_GfxSetTriDimMode(MTFB_LAYER_ID_E enLayerId, MTFB_STEREO_MODE_E enMode)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxSetTriDimAddr(MTFB_LAYER_ID_E enLayerId, mt_u32 u32TriDimAddr)
{
    return MT_SUCCESS;
}
#endif

/*set the priority of layer in gp*/
/*CNcomment:设置图层在GP 中的优先级*/
mt_s32 OPTM_GfxGetLayerPriority(MTFB_LAYER_ID_E enLayerId, mt_u32 *pU32Priority)
{
    return MT_FAILURE;
}

/*set the priority of layer in gp*/
/*CNcomment:设置图层在GP 中的优先级*/
mt_s32 OPTM_GfxSetLayerPriority(MTFB_LAYER_ID_E enLayerId, MTFB_ZORDER_E enZOrder)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GPMask(OPTM_VDP_LAYER_GP_E enGPId, MT_BOOL bFlag)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_DispInfoUpdate(OPTM_VDP_LAYER_GP_E enGPId)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GPRecovery(OPTM_VDP_LAYER_GP_E enGPId)
{
    return MT_SUCCESS;
}

#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_VO_Callback(mt_void *u32Param0, mt_void *u32Param1)
{
    mt_u32 i;
    mt_u32 u32CTypeFlag;
    mt_u32 u32LayerCount;
    MTFB_LAYER_ID_E enInitLayerId;
    MTFB_LAYER_ID_E enLayerId;
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;
    OPTM_VDP_LAYER_GP_E EnGpHalId = OPTM_VDP_LAYER_GP0;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    if (pEnGpHalId == MT_NULL || pstDispInfo == MT_NULL)
    {
        return MT_FAILURE;
    }

    //u32LayerCount = (OPTM_VDP_LAYER_GP0 == *pEnGpHalId) ? OPTM_GP0_GFX_COUNT : OPTM_GP1_GFX_COUNT;
    //enInitLayerId   = (OPTM_VDP_LAYER_GP0 == *pEnGpHalId) ? MTFB_LAYER_HD_0 : MTFB_LAYER_SD_0;
    u32LayerCount = OPTM_GP_MAXGFXCOUNT;
    pEnGpHalId = &EnGpHalId; // only to set  *pEngpHalID  = 0;
    enInitLayerId = MTFB_LAYER_STILL;

    for (i = 0; i < u32LayerCount; i++)
    {
        u32CTypeFlag = g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[i].u32CTypeFlag;

        if (!u32CTypeFlag) { /** 顶场不更新 **/
            continue;
        }

        enLayerId = enInitLayerId + i;

        if (u32CTypeFlag & MTFB_CALLBACK_TYPE_VO)
        {
            if (pstDispInfo->stDispInfo.bInterlace &&
                    !pstDispInfo->stDispInfo.bIsBottomField /* && MTFB_LAYER_HD_0 == i */)
            {
                continue;
            }

            MT_ASSERT(g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[i].stGfxIrq[MTFB_CALLBACK_TYPE_VO].pFunc != NULL);
            {
                g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[i].stGfxIrq[MTFB_CALLBACK_TYPE_VO].pFunc(
                        (mt_void *)g_stGfxGPIrq[*pEnGpHalId].stGfxCallBack[i].stGfxIrq[MTFB_CALLBACK_TYPE_VO].u32Param0,
                        MT_NULL);
            }

            /**when v sync , wake up V Block*/
#ifdef OPTM_MTFB_WVM_ENABLE
            OPTM_GfxWVBCallBack(enLayerId, MT_NULL);
#endif
        }

        if (u32CTypeFlag & MTFB_CALLBACK_TYPE_REGUP) {
            /*callback function*/
            /*define here for extending*/
        }
    }

    return MT_SUCCESS;
}

mt_s32 OPTM_Distribute_Callback(mt_void *u32Param0, mt_void *u32Param1)
{
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    MTFB_STEREO_MODE_E enTriDimMode;
#endif
    OPTM_VDP_LAYER_GP_E *pEnGpHalId;
    MT_DRV_DISP_CALLBACK_INFO_S *pstDispInfo;

    pEnGpHalId = (OPTM_VDP_LAYER_GP_E *)u32Param0;
    pstDispInfo = (MT_DRV_DISP_CALLBACK_INFO_S *)u32Param1;

    if (pEnGpHalId == MT_NULL || pstDispInfo == MT_NULL)
    {
        return MT_FAILURE;
    }

    OPTM_VO_Callback(u32Param0, u32Param1);

    return MT_SUCCESS;
}
#endif

mt_s32 OPTM_GFX_SetGpInUsrFlag(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag)
{
    g_stGfxGPDevice[enGpId].bGPInSetbyusr = bFlag;
    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_GetGpInUsrFlag(OPTM_GFX_GP_E enGpId)
{

    return g_stGfxGPDevice[enGpId].bGPInSetbyusr;
}

mt_s32 OPTM_GFX_SetGpInInitFlag(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag)
{
    g_stGfxGPDevice[enGpId].bGPInInitial = bFlag;
    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_GetGpInInitFlag(OPTM_GFX_GP_E enGpId)
{

    return g_stGfxGPDevice[enGpId].bGPInInitial;
}

#ifndef MT_BUILD_IN_BOOT
mt_s32 OPTM_GFX_SetGfxMask(OPTM_GFX_GP_E enGpId, MT_BOOL bFlag)
{
    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_GetGfxMask(OPTM_GFX_GP_E enGpId)
{
    return g_stGfxGPDevice[enGpId].bMaskFlag;
}

mt_s32 OPTM_GFX_ClearLogoOsd(MTFB_LAYER_ID_E enLayerId)
{

    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_SetStereoDepth(MTFB_LAYER_ID_E enLayerId, mt_s32 s32Depth)
{
    return MT_SUCCESS;
}
#endif

mt_s32 OPTM_GFX_SetTCFlag(MT_BOOL bFlag)
{
    g_bTcWBCFlag = bFlag;
    return MT_SUCCESS;
}

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
mt_s32 OPTM_GFX_CMP_Open(MTFB_LAYER_ID_E enLayerId)
{
    mt_u32 temp,temp1;
    mt_u32 osdc_data_rd_addr,osdc_data_wr_addr_a,osdc_data_wr_addr_r,osdc_data_wr_addr_g,osdc_data_wr_addr_b;
    mt_u16 osdc_osd_width,osdc_osd_height,osdc_osd_width_stride;
    mt_u8 osdc_endian,osdc_a_position,osdc_premulti_en,osdc_semi_uv_change,osdc_format;
    mt_u8 osd_format,osd_semi_en,osd_semi_format,osd_endian,osd_uv_change,osd_a_position;
    void* osdc_cw_rd_addr = NULL;
    mt_u32 cnt = 0;
    mt_u8 alpha_bypass = 0;
    mt_u32 cw1,cw2,cw3,cw4,cw5;
    mt_u32 osdc_compress_bit_a_length,osdc_compress_bit_r_length,osdc_compress_bit_g_length,osdc_compress_bit_b_length;
    //mt_u32 osdc_data_a_reserved_size,osdc_data_r_reserved_size,osdc_data_g_reserved_size,osdc_data_b_reserved_size,osdc_data_total_reserved_size;
    mt_u32 osdc_bits_max = 0xFFFFFFFF;
    osdc_cw_rd_addr = g_stGfxDevice[enLayerId].stRgnHeaderBuf.startVirAddr;
          
    MTFB_DEBUGK("%s osd layer:%d, header vir_addr:%p, phy_addr:%p\n", __FUNCTION__, 
        enLayerId, osdc_cw_rd_addr,  (void *)(ulong)g_stGfxDevice[enLayerId].stRgnHeaderBuf.startPhyAddr);

    if(osdc_cw_rd_addr == NULL )
    {
       MTFB_DEBUGK("osdc_cw_rd_addr is invalid!!!\n");
    }

    if(g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress != MT_TRUE)
    {
        MTFB_DEBUGK("osd compress can not be use now!!!\n");
        return MT_FAILURE;
    }
    cw1 = readl(osdc_cw_rd_addr);
    cw2 = readl(osdc_cw_rd_addr+0x4);
    cw3 = readl(osdc_cw_rd_addr+0x8);
    cw4 = readl(osdc_cw_rd_addr+0xc);
    cw5 = readl(osdc_cw_rd_addr+0x10);
    
    temp = cw2&0x7ff;
    temp1 = cw3&0x7ff;
    osdc_osd_width = temp1 - temp + 1;
    temp = (cw2>>16)&0x7ff;
    temp1 = (cw3>>16)&0x7ff;
    osdc_osd_height = temp1 - temp + 1;
    osdc_osd_width_stride = (cw4>>16)&0x7ff;
    MTFB_DEBUGK("\n osd width:%d,height:%d,stride:%d\n",osdc_osd_width,osdc_osd_height,osdc_osd_width_stride);
    
    osdc_premulti_en = (cw4>>8)&0x01;
    osdc_data_rd_addr = cw5;
    
    temp1 = cw1&0x60006;//endian10[2:1],endian32[18:17]
    osd_endian = ((temp1>>17<<2) | (temp1>>1))&0xf;
    osdc_endian = osd_endian;
    MTFB_DEBUGK("\n osd endian:%d", osd_endian);
    
    osd_a_position = (cw1>>19)&0x1;
    osdc_a_position = osd_a_position;
    MTFB_DEBUGK("\n osd a position:%d", osd_a_position);
    
    osd_uv_change = (cw1>>3)&0x1;
    osdc_semi_uv_change = osd_uv_change;
    
    osd_semi_en = (cw1>>9)&0x1;
    osd_semi_format = (cw1>>10)&0x3;
    osd_format = (cw1>>28)&0x7;
    
    switch(osd_format)
    {
        case 1:
          osdc_format = 2;//rgb565
          break;
        case 2:
          osdc_format = 0;//rgb4444
          break;
        case 3:
          osdc_format = 1;//rgb1555
          alpha_bypass = 1;
          break;
        case 4:
          {
            osdc_format = 10;//argb8888
          }
          break;
        case 6:
          {
            osdc_format = 6;//uy0vy1
          }
          break;
        case 7:
          {
            osdc_format = 13;//ayuv8888
          }
          break;
        default:
          MTFB_DEBUGK("\n osd format:%d not supported by osd compress!\n",osd_format);
          return MT_FAILURE;
      }
      MTFB_DEBUGK("\n osdc format:%d",osdc_format);
    
      //0x3000
      drv_reg_4k_disp_set_osdc_cmd_osdc_osd_width(osdc_osd_width);
      drv_reg_4k_disp_set_osdc_cmd_osdc_osd_height(osdc_osd_height);
      drv_reg_4k_disp_set_osdc_cmd_osdc_endian(osdc_endian);
    
      //0x301c
      drv_reg_4k_disp_set_osdc_width_stride_osdc_osd_width_stride(osdc_osd_width_stride);
    
      drv_reg_4k_disp_set_osdc_ctrl3_osdc_semi_uv_change(osdc_semi_uv_change);
      drv_reg_4k_disp_set_osdc_ctrl3_osdc_alpha_bypass(alpha_bypass);
      drv_reg_4k_disp_set_osdc_ctrl3_osdc_osd_format(osdc_format);
      drv_reg_4k_disp_set_osdc_ctrl3_osdc_alpha_pos(osdc_a_position);
      drv_reg_4k_disp_set_osdc_ctrl3_osdc_premulti_en(osdc_premulti_en);
      drv_reg_4k_disp_set_osdc_ddr_rd_addr_uv(osdc_data_rd_addr);
      drv_reg_4k_disp_set_osdc_ddr_rd_addr(osdc_data_rd_addr);
#if 0    
    //calculate reserved size
      temp = (osdc_osd_width_stride + 7)>>3<<3;//width should be 8byte aligned
      switch(osdc_format) //every 8 pix add 4 bit
      {
        case 2://rgb565
          osdc_data_a_reserved_size = 0;
          osdc_data_r_reserved_size = ((((temp * osdc_osd_height * 5) / 8) * 11)+9) / 10;
          osdc_data_g_reserved_size = ((((temp * osdc_osd_height * 6) / 8) * 13)+11) / 12;
          osdc_data_b_reserved_size = osdc_data_r_reserved_size;
          break;
        case 0://argb4444
          osdc_data_a_reserved_size = ((((temp * osdc_osd_height * 4) / 8) * 9)+7) / 8;
          osdc_data_r_reserved_size = osdc_data_a_reserved_size;
          osdc_data_g_reserved_size = osdc_data_a_reserved_size;
          osdc_data_b_reserved_size = osdc_data_a_reserved_size;
          break;
        case 1://argb1555
          osdc_data_a_reserved_size = ((((temp * osdc_osd_height * 1) / 8) * 3)+1) / 2;
          osdc_data_r_reserved_size = ((((temp * osdc_osd_height * 5) / 8) * 11)+9) / 10;
          osdc_data_g_reserved_size = osdc_data_r_reserved_size;
          osdc_data_b_reserved_size = osdc_data_r_reserved_size;
          break;
        case 10://argb8888
          osdc_data_a_reserved_size = ((temp * osdc_osd_height * 17)+15) / 16;
          osdc_data_r_reserved_size = osdc_data_a_reserved_size;
          osdc_data_g_reserved_size = osdc_data_a_reserved_size;
          osdc_data_b_reserved_size = osdc_data_a_reserved_size;
          break;
        case 6://uy0vy1
          temp = (temp + 15)>>4<<4;//yuv format should be 16 byte aligned
          osdc_data_a_reserved_size = 0;
          osdc_data_r_reserved_size = ((temp * osdc_osd_height * 17)+15) / 16;
          osdc_data_g_reserved_size = ((((temp * osdc_osd_height * 4) / 8) * 9)+7) / 8;
          osdc_data_b_reserved_size = osdc_data_g_reserved_size;
          break;
        case 13://ayuv8888
          osdc_data_a_reserved_size = ((temp * osdc_osd_height * 17)+15) / 16;
          osdc_data_r_reserved_size = osdc_data_a_reserved_size;
          osdc_data_g_reserved_size = osdc_data_a_reserved_size;
          osdc_data_b_reserved_size = osdc_data_a_reserved_size;
          break;
        default:
          MTFB_DEBUGK("\n osd format:%d not supported by osd compress!\n",osd_format);
          return;
          break;
      }
    
      osdc_data_a_reserved_size = (osdc_data_a_reserved_size+15)>>4<<4;
      osdc_data_r_reserved_size = (osdc_data_r_reserved_size+15)>>4<<4;
      osdc_data_g_reserved_size = (osdc_data_g_reserved_size+15)>>4<<4;
      osdc_data_b_reserved_size = (osdc_data_b_reserved_size+15)>>4<<4;
      MTFB_DEBUGK("reserved size bit num:%x,%x,%x,%x\n",osdc_data_a_reserved_size*8,osdc_data_r_reserved_size*8,
        osdc_data_g_reserved_size*8,osdc_data_b_reserved_size*8);
    
      osdc_data_total_reserved_size = osdc_data_a_reserved_size+osdc_data_r_reserved_size+osdc_data_g_reserved_size+osdc_data_b_reserved_size;

      osdc_data_wr_addr_a = osdc_data_wr_addr_base;
      osdc_data_wr_addr_r = osdc_data_wr_addr_a + osdc_data_a_reserved_size;
      osdc_data_wr_addr_g = osdc_data_wr_addr_r + osdc_data_r_reserved_size;
      osdc_data_wr_addr_b = osdc_data_wr_addr_g + osdc_data_g_reserved_size;
#else
      osdc_data_wr_addr_a = ((mt_u32)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startPhyAddr+15)>>4<<4;
      osdc_data_wr_addr_r = ((mt_u32)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startPhyAddr+15)>>4<<4;;
      osdc_data_wr_addr_g = ((mt_u32)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startPhyAddr+15)>>4<<4;
      osdc_data_wr_addr_b = ((mt_u32)g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startPhyAddr+15)>>4<<4;
#endif      
      drv_reg_4k_disp_set_osdc_ddr_wr_addr_a(osdc_data_wr_addr_a);//0x3020
      drv_reg_4k_disp_set_osdc_ddr_wr_addr_r(osdc_data_wr_addr_r);//0x3024
      drv_reg_4k_disp_set_osdc_ddr_wr_addr_g(osdc_data_wr_addr_g);//0x302c
      drv_reg_4k_disp_set_osdc_ddr_wr_addr_b(osdc_data_wr_addr_b);//0x3028
    
      drv_reg_4k_disp_set_osdc_bits_max_a(osdc_bits_max);
      drv_reg_4k_disp_set_osdc_bits_max_r(osdc_bits_max);
      drv_reg_4k_disp_set_osdc_bits_max_g(osdc_bits_max);
      drv_reg_4k_disp_set_osdc_bits_max_b(osdc_bits_max);

      drv_reg_4k_disp_set_osdc_cg_ctrl_cg_sw_bypass(1); //clock gate - enable clock
      drv_reg_4k_disp_set_osdc_rst_osdc_rst_h(1);//software reset
      drv_reg_4k_disp_set_osdc_rst_osdc_axi_w_limit(0xf);
      drv_reg_4k_disp_set_osdc_rst_osdc_axi_r_limit(0x7);
      
      MTFB_DEBUGK("\n osdc status:0x%x,wait osdc reset...",drv_reg_4k_disp_get_osdc_status());
      cnt = 0;
      while((!drv_reg_4k_disp_get_osdc_status_osdc_soft_rst_done())
        ||(!drv_reg_4k_disp_get_osdc_status_osdc_axi_w_done())
        ||(!drv_reg_4k_disp_get_osdc_status_osdc_axi_r_done())
        ||drv_reg_4k_disp_get_osdc_irq_osdc_end_irq())
      {
        MTFB_DEBUGK("wait until osdc reset finished!");
        msleep(1);
        cnt++;
        if(cnt>0x100)
        {
          MTFB_DEBUGK("\n osdc reset fail!");
          return MT_FAILURE;
        }        
      };
      MTFB_DEBUGK("\n osdc reset finished!\n");
    
      drv_reg_4k_disp_set_osdc_rst_osdc_rst_h(0);//software reset release
      drv_reg_4k_disp_set_osdc_cg_ctrl_cg_sw_bypass(0); //clock gate - auto clock gate
      
      drv_reg_4k_disp_set_osdc_cmd_osdc_pre_judge(0);
      drv_reg_4k_disp_set_osdc_cmd_osdc_osdcomp_start(1);//start osd compress
      MTFB_DEBUGK("\n start compression...");      
      
      cnt = 0;
      while(!drv_reg_4k_disp_get_osdc_irq_osdc_end_irq())
      {
        msleep(1);
        cnt++;
        if(cnt > 2000)
        {
          MTFB_DEBUGK("\n osd compression fail!");
          return MT_FAILURE;
        }
      }
      //drv_reg_4k_disp_set_osdc_irq_en_osdc_end_irq_en(1); //enable osdc_irq --- only for fpga irq test
    
      MTFB_DEBUGK("\n compression finished! status 0x%x",drv_reg_4k_disp_get_osdc_status());
      osdc_compress_bit_a_length = drv_reg_4k_disp_get_osdc_compress_bit_a();
      osdc_compress_bit_r_length = drv_reg_4k_disp_get_osdc_compress_bit_r();
      osdc_compress_bit_g_length = drv_reg_4k_disp_get_osdc_compress_bit_g();
      osdc_compress_bit_b_length = drv_reg_4k_disp_get_osdc_compress_bit_b();
      MTFB_DEBUGK("\n a channel compression bits num:0x%x", osdc_compress_bit_a_length);
      MTFB_DEBUGK("\n r channel compression bits num:0x%x", osdc_compress_bit_r_length);
      MTFB_DEBUGK("\n g channel compression bits num:0x%x", osdc_compress_bit_g_length);
      MTFB_DEBUGK("\n b channel compression bits num:0x%x", osdc_compress_bit_b_length);
    
    //set osd decompress
      if(MTFB_LAYER_OSD0 == enLayerId)
      {
        MTFB_DEBUGK("\n osd0 decompression setting ...");
        drv_reg_4k_disp_set_osdd_osd0_length_a(osdc_compress_bit_a_length);
        drv_reg_4k_disp_set_osdd_osd0_length_r(osdc_compress_bit_r_length);
        drv_reg_4k_disp_set_osdd_osd0_length_g(osdc_compress_bit_g_length);
        drv_reg_4k_disp_set_osdd_osd0_length_b(osdc_compress_bit_b_length);
        drv_reg_4k_disp_set_osdd_osd0_addr_a(drv_reg_4k_disp_get_osdc_ddr_wr_addr_a());
        drv_reg_4k_disp_set_osdd_osd0_addr_r(drv_reg_4k_disp_get_osdc_ddr_wr_addr_r());
        drv_reg_4k_disp_set_osdd_osd0_addr_g(drv_reg_4k_disp_get_osdc_ddr_wr_addr_g());
        drv_reg_4k_disp_set_osdd_osd0_addr_b(drv_reg_4k_disp_get_osdc_ddr_wr_addr_b());
        drv_reg_4k_disp_set_osdd_osd0_margin_th0_a(drv_reg_4k_disp_get_osdc_margin_th0_a());
        drv_reg_4k_disp_set_osdd_osd0_margin_th0_y(drv_reg_4k_disp_get_osdc_margin_th0_y());
        drv_reg_4k_disp_set_osdd_osd0_margin_th0_u(drv_reg_4k_disp_get_osdc_margin_th0_u());
        drv_reg_4k_disp_set_osdd_osd0_margin_th0_v(drv_reg_4k_disp_get_osdc_margin_th0_v());
        drv_reg_4k_disp_set_osdd_osd0_margin_th1_a(drv_reg_4k_disp_get_osdc_margin_th1_a());
        drv_reg_4k_disp_set_osdd_osd0_margin_th1_y(drv_reg_4k_disp_get_osdc_margin_th1_y());
        drv_reg_4k_disp_set_osdd_osd0_margin_th1_u(drv_reg_4k_disp_get_osdc_margin_th1_u());
        drv_reg_4k_disp_set_osdd_osd0_margin_th1_v(drv_reg_4k_disp_get_osdc_margin_th1_v());
        drv_reg_4k_disp_set_osdd_osd0_margin_th2_a(drv_reg_4k_disp_get_osdc_margin_th2_a());
        drv_reg_4k_disp_set_osdd_osd0_margin_th2_y(drv_reg_4k_disp_get_osdc_margin_th2_y());
        drv_reg_4k_disp_set_osdd_osd0_margin_th2_u(drv_reg_4k_disp_get_osdc_margin_th2_u());
        drv_reg_4k_disp_set_osdd_osd0_margin_th2_v(drv_reg_4k_disp_get_osdc_margin_th2_v());
        drv_reg_4k_disp_set_osdd_osd0_margin_th3_a(drv_reg_4k_disp_get_osdc_margin_th3_a());
        drv_reg_4k_disp_set_osdd_osd0_margin_th3_y(drv_reg_4k_disp_get_osdc_margin_th3_y());
        drv_reg_4k_disp_set_osdd_osd0_margin_th3_u(drv_reg_4k_disp_get_osdc_margin_th3_u());
        drv_reg_4k_disp_set_osdd_osd0_margin_th3_v(drv_reg_4k_disp_get_osdc_margin_th3_v());
        drv_reg_4k_disp_set_osdd_osd0_margin_th4_a(drv_reg_4k_disp_get_osdc_margin_th4_a());
        drv_reg_4k_disp_set_osdd_osd0_margin_th4_y(drv_reg_4k_disp_get_osdc_margin_th4_y());
        drv_reg_4k_disp_set_osdd_osd0_margin_th4_u(drv_reg_4k_disp_get_osdc_margin_th4_u());
        drv_reg_4k_disp_set_osdd_osd0_margin_th4_v(drv_reg_4k_disp_get_osdc_margin_th4_v());
        drv_reg_4k_disp_set_osdd_osd0_bit_qp_th0_osdd_osd0_bit_qp_th0_a(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_a());
        drv_reg_4k_disp_set_osdd_osd0_bit_qp_th0_osdd_osd0_bit_qp_th0_y(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_y());
        drv_reg_4k_disp_set_osdd_osd0_bit_qp_th0_osdd_osd0_bit_qp_th0_u(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_u());
        drv_reg_4k_disp_set_osdd_osd0_bit_qp_th0_osdd_osd0_bit_qp_th0_v(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_v());
        drv_reg_4k_disp_set_osdd_osd0_expect_buffer_size_0_expect_buffer_size_a(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_a());
        drv_reg_4k_disp_set_osdd_osd0_expect_buffer_size_0_expect_buffer_size_y(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_y());
        drv_reg_4k_disp_set_osdd_osd0_expect_buffer_size_1_expect_buffer_size_u(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_u());
        drv_reg_4k_disp_set_osdd_osd0_expect_buffer_size_1_expect_buffer_size_v(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_v());
        drv_reg_4k_disp_set_osdd_osd0_expect_code_bit_a(drv_reg_4k_disp_get_osdc_expect_code_bit_a());
        drv_reg_4k_disp_set_osdd_osd0_expect_code_bit_y(drv_reg_4k_disp_get_osdc_expect_code_bit_y());
        drv_reg_4k_disp_set_osdd_osd0_expect_code_bit_u(drv_reg_4k_disp_get_osdc_expect_code_bit_u());
        drv_reg_4k_disp_set_osdd_osd0_expect_code_bit_v(drv_reg_4k_disp_get_osdc_expect_code_bit_v());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_max_qp_a(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_a());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_max_qp_y(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_y());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_max_qp_u(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_u());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_max_qp_v(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_v());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_encode_mode_a(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_a());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_encode_mode_y(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_y());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_encode_mode_u(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_u());
        drv_reg_4k_disp_set_osdd_osd0_max_qp_encmode_osd0_encode_mode_v(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_v());
    
        drv_reg_4k_disp_set_osdd_osd0_cmd_compress_csc_en(drv_reg_4k_disp_get_osdc_ctrl3_osdc_csc_en());
        drv_reg_4k_disp_set_osdd_osd0_cmd_alpha_bypass(alpha_bypass);
        drv_reg_4k_disp_set_osdd_osd0_cmd_compress_en_osd0(1);//enable osd0 decompression
      }
      else if(MTFB_LAYER_OSD1 == enLayerId)
      {
        MTFB_DEBUGK("\n osd1 decompression setting ...");
        drv_reg_4k_disp_set_osdd_osd1_length_a(osdc_compress_bit_a_length);
        drv_reg_4k_disp_set_osdd_osd1_length_r(osdc_compress_bit_r_length);
        drv_reg_4k_disp_set_osdd_osd1_length_g(osdc_compress_bit_g_length);
        drv_reg_4k_disp_set_osdd_osd1_length_b(osdc_compress_bit_b_length);
        drv_reg_4k_disp_set_osdd_osd1_addr_a(drv_reg_4k_disp_get_osdc_ddr_wr_addr_a());
        drv_reg_4k_disp_set_osdd_osd1_addr_r(drv_reg_4k_disp_get_osdc_ddr_wr_addr_r());
        drv_reg_4k_disp_set_osdd_osd1_addr_g(drv_reg_4k_disp_get_osdc_ddr_wr_addr_g());
        drv_reg_4k_disp_set_osdd_osd1_addr_b(drv_reg_4k_disp_get_osdc_ddr_wr_addr_b());
        drv_reg_4k_disp_set_osdd_osd1_margin_th0_a(drv_reg_4k_disp_get_osdc_margin_th0_a());
        drv_reg_4k_disp_set_osdd_osd1_margin_th0_y(drv_reg_4k_disp_get_osdc_margin_th0_y());
        drv_reg_4k_disp_set_osdd_osd1_margin_th0_u(drv_reg_4k_disp_get_osdc_margin_th0_u());
        drv_reg_4k_disp_set_osdd_osd1_margin_th0_v(drv_reg_4k_disp_get_osdc_margin_th0_v());
        drv_reg_4k_disp_set_osdd_osd1_margin_th1_a(drv_reg_4k_disp_get_osdc_margin_th1_a());
        drv_reg_4k_disp_set_osdd_osd1_margin_th1_y(drv_reg_4k_disp_get_osdc_margin_th1_y());
        drv_reg_4k_disp_set_osdd_osd1_margin_th1_u(drv_reg_4k_disp_get_osdc_margin_th1_u());
        drv_reg_4k_disp_set_osdd_osd1_margin_th1_v(drv_reg_4k_disp_get_osdc_margin_th1_v());
        drv_reg_4k_disp_set_osdd_osd1_margin_th2_a(drv_reg_4k_disp_get_osdc_margin_th2_a());
        drv_reg_4k_disp_set_osdd_osd1_margin_th2_y(drv_reg_4k_disp_get_osdc_margin_th2_y());
        drv_reg_4k_disp_set_osdd_osd1_margin_th2_u(drv_reg_4k_disp_get_osdc_margin_th2_u());
        drv_reg_4k_disp_set_osdd_osd1_margin_th2_v(drv_reg_4k_disp_get_osdc_margin_th2_v());
        drv_reg_4k_disp_set_osdd_osd1_margin_th3_a(drv_reg_4k_disp_get_osdc_margin_th3_a());
        drv_reg_4k_disp_set_osdd_osd1_margin_th3_y(drv_reg_4k_disp_get_osdc_margin_th3_y());
        drv_reg_4k_disp_set_osdd_osd1_margin_th3_u(drv_reg_4k_disp_get_osdc_margin_th3_u());
        drv_reg_4k_disp_set_osdd_osd1_margin_th3_v(drv_reg_4k_disp_get_osdc_margin_th3_v());
        drv_reg_4k_disp_set_osdd_osd1_margin_th4_a(drv_reg_4k_disp_get_osdc_margin_th4_a());
        drv_reg_4k_disp_set_osdd_osd1_margin_th4_y(drv_reg_4k_disp_get_osdc_margin_th4_y());
        drv_reg_4k_disp_set_osdd_osd1_margin_th4_u(drv_reg_4k_disp_get_osdc_margin_th4_u());
        drv_reg_4k_disp_set_osdd_osd1_margin_th4_v(drv_reg_4k_disp_get_osdc_margin_th4_v());
        drv_reg_4k_disp_set_osdd_osd1_bit_qp_th0_osdd_osd1_bit_qp_th0_a(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_a());
        drv_reg_4k_disp_set_osdd_osd1_bit_qp_th0_osdd_osd1_bit_qp_th0_y(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_y());
        drv_reg_4k_disp_set_osdd_osd1_bit_qp_th0_osdd_osd1_bit_qp_th0_u(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_u());
        drv_reg_4k_disp_set_osdd_osd1_bit_qp_th0_osdd_osd1_bit_qp_th0_v(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_v());
        drv_reg_4k_disp_set_osdd_osd1_expect_buffer_size_0_expect_buffer_size_a(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_a());
        drv_reg_4k_disp_set_osdd_osd1_expect_buffer_size_0_expect_buffer_size_y(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_y());
        drv_reg_4k_disp_set_osdd_osd1_expect_buffer_size_1_expect_buffer_size_u(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_u());
        drv_reg_4k_disp_set_osdd_osd1_expect_buffer_size_1_expect_buffer_size_v(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_v());
        drv_reg_4k_disp_set_osdd_osd1_expect_code_bit_a(drv_reg_4k_disp_get_osdc_expect_code_bit_a());
        drv_reg_4k_disp_set_osdd_osd1_expect_code_bit_y(drv_reg_4k_disp_get_osdc_expect_code_bit_y());
        drv_reg_4k_disp_set_osdd_osd1_expect_code_bit_u(drv_reg_4k_disp_get_osdc_expect_code_bit_u());
        drv_reg_4k_disp_set_osdd_osd1_expect_code_bit_v(drv_reg_4k_disp_get_osdc_expect_code_bit_v());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_max_qp_a(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_a());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_max_qp_y(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_y());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_max_qp_u(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_u());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_max_qp_v(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_v());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_encode_mode_a(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_a());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_encode_mode_y(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_y());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_encode_mode_u(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_u());
        drv_reg_4k_disp_set_osdd_osd1_max_qp_encmode_osd1_encode_mode_v(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_v());
    
        drv_reg_4k_disp_set_osdd_osd1_cmd_compress_csc_en(drv_reg_4k_disp_get_osdc_ctrl3_osdc_csc_en());
        drv_reg_4k_disp_set_osdd_osd1_cmd_alpha_bypass(alpha_bypass);
        drv_reg_4k_disp_set_osdd_osd1_cmd_compress_en_osd1(1);//enable osd1 decompression
      }
      else if(MTFB_LAYER_SUB == enLayerId)
      {
        MTFB_DEBUGK("\n sub decompression setting ...");
        drv_reg_4k_disp_set_osdd_sub_length_a(osdc_compress_bit_a_length);
        drv_reg_4k_disp_set_osdd_sub_length_r(osdc_compress_bit_r_length);
        drv_reg_4k_disp_set_osdd_sub_length_g(osdc_compress_bit_g_length);
        drv_reg_4k_disp_set_osdd_sub_length_b(osdc_compress_bit_b_length);
        drv_reg_4k_disp_set_osdd_sub_addr_a(drv_reg_4k_disp_get_osdc_ddr_wr_addr_a());
        drv_reg_4k_disp_set_osdd_sub_addr_r(drv_reg_4k_disp_get_osdc_ddr_wr_addr_r());
        drv_reg_4k_disp_set_osdd_sub_addr_g(drv_reg_4k_disp_get_osdc_ddr_wr_addr_g());
        drv_reg_4k_disp_set_osdd_sub_addr_b(drv_reg_4k_disp_get_osdc_ddr_wr_addr_b());
        drv_reg_4k_disp_set_osdd_sub_margin_th0_a(drv_reg_4k_disp_get_osdc_margin_th0_a());
        drv_reg_4k_disp_set_osdd_sub_margin_th0_y(drv_reg_4k_disp_get_osdc_margin_th0_y());
        drv_reg_4k_disp_set_osdd_sub_margin_th0_u(drv_reg_4k_disp_get_osdc_margin_th0_u());
        drv_reg_4k_disp_set_osdd_sub_margin_th0_v(drv_reg_4k_disp_get_osdc_margin_th0_v());
        drv_reg_4k_disp_set_osdd_sub_margin_th1_a(drv_reg_4k_disp_get_osdc_margin_th1_a());
        drv_reg_4k_disp_set_osdd_sub_margin_th1_y(drv_reg_4k_disp_get_osdc_margin_th1_y());
        drv_reg_4k_disp_set_osdd_sub_margin_th1_u(drv_reg_4k_disp_get_osdc_margin_th1_u());
        drv_reg_4k_disp_set_osdd_sub_margin_th1_v(drv_reg_4k_disp_get_osdc_margin_th1_v());
        drv_reg_4k_disp_set_osdd_sub_margin_th2_a(drv_reg_4k_disp_get_osdc_margin_th2_a());
        drv_reg_4k_disp_set_osdd_sub_margin_th2_y(drv_reg_4k_disp_get_osdc_margin_th2_y());
        drv_reg_4k_disp_set_osdd_sub_margin_th2_u(drv_reg_4k_disp_get_osdc_margin_th2_u());
        drv_reg_4k_disp_set_osdd_sub_margin_th2_v(drv_reg_4k_disp_get_osdc_margin_th2_v());
        drv_reg_4k_disp_set_osdd_sub_margin_th3_a(drv_reg_4k_disp_get_osdc_margin_th3_a());
        drv_reg_4k_disp_set_osdd_sub_margin_th3_y(drv_reg_4k_disp_get_osdc_margin_th3_y());
        drv_reg_4k_disp_set_osdd_sub_margin_th3_u(drv_reg_4k_disp_get_osdc_margin_th3_u());
        drv_reg_4k_disp_set_osdd_sub_margin_th3_v(drv_reg_4k_disp_get_osdc_margin_th3_v());
        drv_reg_4k_disp_set_osdd_sub_margin_th4_a(drv_reg_4k_disp_get_osdc_margin_th4_a());
        drv_reg_4k_disp_set_osdd_sub_margin_th4_y(drv_reg_4k_disp_get_osdc_margin_th4_y());
        drv_reg_4k_disp_set_osdd_sub_margin_th4_u(drv_reg_4k_disp_get_osdc_margin_th4_u());
        drv_reg_4k_disp_set_osdd_sub_margin_th4_v(drv_reg_4k_disp_get_osdc_margin_th4_v());
        drv_reg_4k_disp_set_osdd_sub_bit_qp_th0_osdd_sub_bit_qp_th0_a(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_a());
        drv_reg_4k_disp_set_osdd_sub_bit_qp_th0_osdd_sub_bit_qp_th0_y(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_y());
        drv_reg_4k_disp_set_osdd_sub_bit_qp_th0_osdd_sub_bit_qp_th0_u(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_u());
        drv_reg_4k_disp_set_osdd_sub_bit_qp_th0_osdd_sub_bit_qp_th0_v(drv_reg_4k_disp_get_osdc_bit_qp_th0_osdc_bit_qp_th0_v());
        drv_reg_4k_disp_set_osdd_sub_expect_buffer_size_0_expect_buffer_size_a(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_a());
        drv_reg_4k_disp_set_osdd_sub_expect_buffer_size_0_expect_buffer_size_y(drv_reg_4k_disp_get_osdc_expect_buffer_size_0_osdc_expect_buffer_size_y());
        drv_reg_4k_disp_set_osdd_sub_expect_buffer_size_1_expect_buffer_size_u(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_u());
        drv_reg_4k_disp_set_osdd_sub_expect_buffer_size_1_expect_buffer_size_v(drv_reg_4k_disp_get_osdc_expect_buffer_size_1_osdc_expect_buffer_size_v());
        drv_reg_4k_disp_set_osdd_sub_expect_code_bit_a(drv_reg_4k_disp_get_osdc_expect_code_bit_a());
        drv_reg_4k_disp_set_osdd_sub_expect_code_bit_y(drv_reg_4k_disp_get_osdc_expect_code_bit_y());
        drv_reg_4k_disp_set_osdd_sub_expect_code_bit_u(drv_reg_4k_disp_get_osdc_expect_code_bit_u());
        drv_reg_4k_disp_set_osdd_sub_expect_code_bit_v(drv_reg_4k_disp_get_osdc_expect_code_bit_v());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_max_qp_a(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_a());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_max_qp_y(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_y());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_max_qp_u(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_u());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_max_qp_v(drv_reg_4k_disp_get_osdc_max_qp_osdc_max_qp_v());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_encode_mode_a(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_a());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_encode_mode_y(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_y());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_encode_mode_u(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_u());
        drv_reg_4k_disp_set_osdd_sub_max_qp_encmode_sub_encode_mode_v(drv_reg_4k_disp_get_osdc_ctrl3_osdc_encode_mode_v());
    
        drv_reg_4k_disp_set_osdd_sub_cmd_compress_csc_en(drv_reg_4k_disp_get_osdc_ctrl3_osdc_csc_en());
        drv_reg_4k_disp_set_osdd_sub_cmd_alpha_bypass(alpha_bypass);
        drv_reg_4k_disp_set_osdd_sub_cmd_compress_en_sub(1);//enable sub decompression
      }

      g_stGfxDevice[enLayerId].bCmpOpened = MT_TRUE;

    return MT_SUCCESS;
}

mt_void OPTM_GFX_CMP_Clean(MTFB_LAYER_ID_E enLayerId)
{

    MTFB_DEBUGK("%s %d layer:%d\n", __FUNCTION__, __LINE__, enLayerId);
    
    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_A.startVirAddr = 0;
    }
    
    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_R.startVirAddr = 0;
    }
    
    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_G.startVirAddr = 0;
    }
    
    if(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr != 0)
    {
        OPTM_UnmapAndRelease(&(g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B));
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startPhyAddr = 0;
        g_stGfxDevice[enLayerId].stCmpInfo.stCMPBuffer_B.startVirAddr = 0;
    }
    
    g_stGfxDevice[enLayerId].stCmpInfo.bUseCompress = MT_FALSE;      
}

mt_s32 OPTM_GFX_CMP_Close(MTFB_LAYER_ID_E enLayerId)
{
    MTFB_DEBUGK("%s layer:%d \n", __FUNCTION__, enLayerId); 

    if(MTFB_LAYER_OSD0 == enLayerId)
    {
        drv_reg_4k_disp_set_osdd_osd0_cmd_compress_en_osd0(0);
    }
    else if(MTFB_LAYER_OSD1 == enLayerId)
    {
        drv_reg_4k_disp_set_osdd_osd1_cmd_compress_en_osd1(0);

    }
    else if(MTFB_LAYER_SUB == enLayerId)
    {
        drv_reg_4k_disp_set_osdd_sub_cmd_compress_en_sub(0);
    } 
    OPTM_GFX_CMP_Clean(enLayerId); //gavins change   
    g_stGfxDevice[enLayerId].bCmpOpened = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 OPTM_GFX_CMP_CheckInt(MTFB_LAYER_ID_E enLayerId)
{
    mt_u32 bCmpFinish = 0;

    return bCmpFinish;
}


mt_s32 OPTM_GFX_CMP_GetSwitch(MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxDevice[enLayerId].bCmpOpened;
}

mt_s32 OPTM_GFX_SetCmpRect(MTFB_LAYER_ID_E enLayerId, MTFB_RECT *pstRect)
{


    return MT_SUCCESS;
}

OPTM_CMP_STATUS_E OPTM_GFX_CMP_GetStatus(MTFB_LAYER_ID_E enLayerId)
{
    OPTM_GFX_CMP_S *pstCmpInfo;

    pstCmpInfo = &(g_stGfxDevice[enLayerId].stCmpInfo);

    return pstCmpInfo->enStatus;
}

/*Set compression Mode*/
mt_s32 OPTM_GFX_SetCmpMode(MTFB_LAYER_ID_E enLayerId, MTFB_CMP_MODE_E enCMPMode)
{
    g_stGfxDevice[enLayerId].stCmpInfo.enCMPMode = enCMPMode;
    return MT_SUCCESS;
}
/*Get compression Mode*/
MTFB_CMP_MODE_E OPTM_GFX_GetCmpMode(MTFB_LAYER_ID_E enLayerId)
{
    return g_stGfxDevice[enLayerId].stCmpInfo.enCMPMode;
}

#endif

mt_s32 OPTM_GFX_CMP_DECMP_Process(MTFB_LAYER_ID_E enLayerId, mt_u32 u32RdAddr,
        mt_u32 pic_width, mt_u32 pic_height, mt_u32 u32Stride, mt_u32 u32WrAddr)
{
#if defined(CONFIG_MT_CHIP_ARIA)

    mt_u32 temp, temp1;
    mt_u32 wrAddr_gap;

    OPTM_FUN_IN;

    wrAddr_gap = pic_width * pic_height;

    temp = (pic_height << 16) + pic_width;
    reg_aria_disp_set_osdc_cmd(temp);

    reg_aria_disp_set_osdc_rst(0xff00);

    //set arithmetic: self-adaption
    reg_aria_disp_set_osdc_ctl2(0xf);

    reg_aria_disp_set_osdc_ddr_rd_addr(u32RdAddr);
    reg_aria_disp_set_osdc_width_stride(u32Stride);
    reg_aria_disp_set_osdc_ddr_wr_addr_a(u32WrAddr);
    reg_aria_disp_set_osdc_ddr_wr_addr_r(u32WrAddr + wrAddr_gap);
    reg_aria_disp_set_osdc_ddr_wr_addr_g(u32WrAddr + wrAddr_gap * 2);
    reg_aria_disp_set_osdc_ddr_wr_addr_b(u32WrAddr + wrAddr_gap * 3);

    reg_aria_disp_set_osdc_bits_max_a(0xffffffff);
    reg_aria_disp_set_osdc_bits_max_r(0xffffffff);
    reg_aria_disp_set_osdc_bits_max_g(0xffffffff);
    reg_aria_disp_set_osdc_bits_max_b(0xffffffff);

    reg_aria_disp_set_osdc_rst(0xff01);
    reg_aria_disp_set_osdc_rst(0xff00);

    reg_aria_disp_set_osdc_irq_en(1);

    temp = reg_aria_disp_get_osdc_cmd();
    temp |= 0x80000000;
    reg_aria_disp_set_osdc_cmd(temp);

    while (1)
    {
        temp = reg_aria_disp_get_osdc_irq();

        if (temp & 0x2)
        {
            OPTM_LOG("\n ERROR, Result number is bigger than expected !!!! \n");
            reg_aria_disp_set_osdc_rst(0xff01);
            while (1)
                ;
            {
                temp1 = reg_aria_disp_get_osdc_status();
                if ((temp1 & 0xc) == 0xc)
                {
                    OPTM_LOG("\n axi release success!!!! \n");
                    break;
                }
            }
            reg_aria_disp_set_osdc_rst(0xff00);
            break;
        }

        if (temp & 0x1)
        {
            OPTM_LOG("\n OSD compression successfully Done !!!! \n");
            break;
        }
    }

    switch (enLayerId)
    {
    case MTFB_LAYER_OSD0:
        temp = reg_aria_disp_get_osdl_cmd();
        reg_aria_disp_set_osdl_cmd(temp | 0x1);
        reg_aria_disp_set_osdd_osd0_cmd(0x1);
        break;

    case MTFB_LAYER_OSD1:
        temp = reg_aria_disp_get_osdl_cmd();
        reg_aria_disp_set_osdl_cmd(temp | 0x2);
        reg_aria_disp_set_osdd_osd1_cmd(0x1);
        break;

    case MTFB_LAYER_SUB:
        temp = reg_aria_disp_get_osdl_cmd();
        reg_aria_disp_set_osdl_cmd(temp | 0x4);
        reg_aria_disp_set_osdd_sub_cmd(0x1);
        break;

    case MTFB_LAYER_STILL:
        break;

    default:
        break;
    }

    OPTM_FUN_OUT;
#endif
    return MT_SUCCESS;
}

mt_s32 OPTM_GfxGetOsdHeader(MTFB_LAYER_ID_E enLayerId, osd_header_info_s *p_info)
{
    MTFB_DEBUGK("%s %d layer:%d\n", __FUNCTION__, __LINE__, enLayerId);
    switch (enLayerId)
    {
        case MTFB_LAYER_OSD0:
        case MTFB_LAYER_OSD1:
        case MTFB_LAYER_SUB:
           p_info->m_left = g_stGfxDevice[enLayerId].stRgnHeader.m_left;
           p_info->m_right = g_stGfxDevice[enLayerId].stRgnHeader.m_right;
           p_info->m_top = g_stGfxDevice[enLayerId].stRgnHeader.m_top;
           p_info->m_bottom = g_stGfxDevice[enLayerId].stRgnHeader.m_bottom;
           p_info->m_alpha0 = g_stGfxDevice[enLayerId].stRgnHeader.m_alpha0;
           p_info->m_alpha1 = g_stGfxDevice[enLayerId].stRgnHeader.m_alpha1;
           p_info->m_pitch = g_stGfxDevice[enLayerId].stRgnHeader.m_pitch;
           p_info->m_enable = g_stGfxDevice[enLayerId].stRgnHeader.m_enable;
           p_info->m_palette = g_stGfxDevice[enLayerId].stRgnHeader.m_palette;
           p_info->m_alphamode = g_stGfxDevice[enLayerId].stRgnHeader.m_alphamode;
           p_info->m_follow = g_stGfxDevice[enLayerId].stRgnHeader.m_follow;
           p_info->m_start_addr = g_stGfxDevice[enLayerId].stRgnHeader.m_start_addr;
           p_info->m_start_addr_vs = g_stGfxDevice[enLayerId].stRgnHeader.m_start_addr_vs;
           p_info->m_uv_start_addr = g_stGfxDevice[enLayerId].stRgnHeader.m_uv_start_addr;
           p_info->m_truemode = g_stGfxDevice[enLayerId].stRgnHeader.m_truemode;
           p_info->m_clutmode = g_stGfxDevice[enLayerId].stRgnHeader.m_clutmode;
           p_info->m_colormode = g_stGfxDevice[enLayerId].stRgnHeader.m_colormode;
           p_info->m_endianmode = g_stGfxDevice[enLayerId].stRgnHeader.m_endianmode;
           p_info->m_palette_endianmode = g_stGfxDevice[enLayerId].stRgnHeader.m_palette_endianmode;
           p_info->m_alpha_pos = g_stGfxDevice[enLayerId].stRgnHeader.m_alpha_pos;
           p_info->m_alpha_blendmode = g_stGfxDevice[enLayerId].stRgnHeader.m_alpha_blendmode;
           p_info->m_odd_only = g_stGfxDevice[enLayerId].stRgnHeader.m_odd_only;
           p_info->m_rgnaddr_next = g_stGfxDevice[enLayerId].stRgnHeader.m_rgnaddr_next;
           p_info->m_stride = g_stGfxDevice[enLayerId].stRgnHeader.m_stride;
           p_info->m_semi_enable = g_stGfxDevice[enLayerId].stRgnHeader.m_semi_enable;
           p_info->m_uv_change = g_stGfxDevice[enLayerId].stRgnHeader.m_uv_change;
           p_info->m_semi_format = g_stGfxDevice[enLayerId].stRgnHeader.m_semi_format;

        default:
            break;

    }
    return MT_SUCCESS;

}

mt_void OPTM_GFX_GetOps(OPTM_GFX_OPS_S *ops)
{
    ops->OPTM_GfxCloseLayer = OPTM_GfxCloseLayer;
    ops->OPTM_GfxDeInit = OPTM_GfxDeInit;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GfxGetLayerPriority = OPTM_GfxGetLayerPriority;
    ops->OPTM_GfxGetOSDData = OPTM_GfxGetOSDData;
#endif
    ops->OPTM_GfxInit = OPTM_GfxInit;
    ops->OPTM_GfxOpenLayer = OPTM_GfxOpenLayer;
    ops->OPTM_GfxWaitSync = OPTM_GfxWaitSync;
    ops->OPTM_GfxGetOsdHeader= OPTM_GfxGetOsdHeader;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GfxSetCallback = OPTM_GfxSetCallback;
#endif
    ops->OPTM_GfxSetClutAddr = OPTM_GfxSetClutAddr;
    ops->OPTM_GfxSetColorReg = OPTM_GfxSetColorReg;
    ops->OPTM_GfxSetEnable = OPTM_GfxSetEnable;
    ops->OPTM_GfxSetGpRect = OPTM_GfxSetGpRect;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GfxSetGpInPutSize = OPTM_GfxSetGpInPutSize;
#endif
    ops->OPTM_GfxSetLayerAddr = OPTM_GfxSetLayerAddr;
    ops->OPTM_GfxSetLayerAlpha = OPTM_GfxSetLayerAlpha;
    ops->OPTM_GfxSetLayerDataFmt = OPTM_GfxSetLayerDataFmt;
    ops->OPTM_GfxSetLayerDeFlicker = OPTM_GfxSetLayerDeFlicker;
    ops->OPTM_GfxSetLayerPreMult = OPTM_GfxSetLayerPreMult;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GfxSetLayerPriority = OPTM_GfxSetLayerPriority;
#endif
    ops->OPTM_GfxSetLayerRect = OPTM_GfxSetLayerRect;
    ops->OPTM_GfxSetLayerStride = OPTM_GfxSetLayerStride;
    ops->OPTM_GfxSetLayKeyMask = OPTM_GfxSetLayKeyMask;
    ops->OPTM_GfxUpLayerReg = OPTM_GfxUpLayerReg;
    ops->OPTM_GfxWaitVBlank = OPTM_GfxWaitVBlank;
    ops->OPTM_GFX_GetDevCap = OPTM_GFX_GetDevCap;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GfxGetOutRect = OPTM_GfxGetOutRect;
#endif
    ops->OPTM_GfxGetLayerRect = OPTM_GfxGetLayerRect;
    ops->OPTM_GFX_SetGpInUsrFlag = OPTM_GFX_SetGpInUsrFlag;
    ops->OPTM_GFX_GetGpInUsrFlag = OPTM_GFX_GetGpInUsrFlag;
    ops->OPTM_GFX_SetGpInInitFlag = OPTM_GFX_SetGpInInitFlag;
    ops->OPTM_GFX_GetGpInInitFlag = OPTM_GFX_GetGpInInitFlag;
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GFX_SetGfxMask = OPTM_GFX_SetGfxMask;
    ops->OPTM_GFX_GetGfxMask = OPTM_GFX_GetGfxMask;
    ops->OPTM_GfxGetDispFMTSize = OPTM_GfxGetDispFMTSize;
    ops->OPTM_GFX_GetSlvLayerInfo = OPTM_GFX_GetSlvLayerInfo;
#endif
    ops->OPTM_GFX_SetTCFlag = OPTM_GFX_SetTCFlag;

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
    /***compression****/
    ops->OPTM_GFX_CMP_Open = OPTM_GFX_CMP_Open;
    ops->OPTM_GFX_CMP_Close = OPTM_GFX_CMP_Close;
    ops->OPTM_GFX_CMP_GetSwitch = OPTM_GFX_CMP_GetSwitch;
    ops->OPTM_GFX_SetCmpRect = OPTM_GFX_SetCmpRect;
    ops->OPTM_GFX_SetCmpMode = OPTM_GFX_SetCmpMode;
    ops->OPTM_GFX_GetCmpMode = OPTM_GFX_GetCmpMode;
#endif
    ops->OPTM_GFX_CMP_DECMP_Process = OPTM_GFX_CMP_DECMP_Process;
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    /***3D****/
    ops->OPTM_GfxSetTriDimEnable = OPTM_GfxSetTriDimEnable;
    ops->OPTM_GfxSetTriDimMode = OPTM_GfxSetTriDimMode;
    ops->OPTM_GfxSetTriDimAddr = OPTM_GfxSetTriDimAddr;

    ops->OPTM_GFX_SetStereoDepth = OPTM_GFX_SetStereoDepth;
#endif
#ifndef MT_BUILD_IN_BOOT
    ops->OPTM_GFX_ClearLogoOsd = OPTM_GFX_ClearLogoOsd;
    ops->OPTM_GfxSetGpDeflicker = OPTM_GfxSetGpDeflicker;
#endif
}
EXPORT_SYMBOL(OPTM_GFX_GetOps);

/***********************************************************/
/*                         adapt system function                                                 */
/***********************************************************/
mt_s32 OPTM_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, mmz_buffer_s *psMBuf)
{
#ifndef MT_BUILD_IN_BOOT
    return mt_drv_mmz_alloc_and_map(bufname, zone_name, size, align, psMBuf);
#else
    if (MT_SUCCESS == MT_DRV_PDM_AllocReserveMem(bufname, size, &psMBuf->startPhyAddr))
    {
        psMBuf->startVirAddr = psMBuf->startPhyAddr;
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
#endif
}

mt_void OPTM_UnmapAndRelease(mmz_buffer_s *psMBuf)
{
#ifdef MT_BUILD_IN_BOOT
    return;
#else
    mt_drv_mmz_unmap_and_release(psMBuf);
#endif
}

mt_s32 OPTM_Adapt_AllocAndMap(const char *bufname, char *zone_name, mt_u32 size, int align, mmz_buffer_s *psMBuf)
{
#ifndef MT_BUILD_IN_BOOT
    return mt_drv_mmz_alloc_and_map(bufname, zone_name, size, align, psMBuf);
#else
    psMBuf->startPhyAddr = (mt_u32)malloc(size);
    if (MT_NULL == psMBuf->startPhyAddr)
    {
        MTFB_ERROR("fail to alloc buffer.\n");
        return MT_FAILURE;
    }

    psMBuf->startVirAddr = psMBuf->startPhyAddr;
    return MT_SUCCESS;
#endif
}

EXPORT_SYMBOL(p_optm_addr);
