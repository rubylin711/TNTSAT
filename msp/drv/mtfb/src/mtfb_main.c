/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

/*********************************add include here******************************/
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/file.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/timekeeping.h>

#include <linux/fb.h>
#include <linux/uaccess.h>

#include <asm/types.h>
#include <asm/stat.h>
#include <asm/fcntl.h>

#include <linux/interrupt.h>
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_proc.h"
#include "mt_module_debug.h"
#include "mt_cache.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mtfb_drv.h"

#include "mtfb.h"
#include "mtfb_p.h"
#include "mtfb_comm.h"
#include "drv_pdm_ext.h"

#ifdef CFG_MTFB_FENCE_SUPPORT
#include "mtfb_fence.h"
#endif

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
#include "mtfb_scrolltext.h"
#endif

#ifdef CONFIG_DMA_SHARED_BUFFER
#include "mtfb_dmabuf.h"
#endif

/**
 **所有发生变化的所在的头文件
 **/
#include "mtfb_config.h"
#include "mt_gfx_comm_k.h"


int dfb_mem = 0;
EXPORT_SYMBOL(dfb_mem);
int __init dfb_mem_config(char *str)
{
	dfb_mem = memparse(str, &str);
    return 1;
}
__setup("dfb_mem=", dfb_mem_config);
module_param(dfb_mem, int, S_IRUSR);

/***************************** Macro Definition ******************************/

//#define CFG_MTFB_SUPPORT_CONSOLE
//#define CFG_MTFB_PROC_DEBUG


/**
 **mod init tmts var
 **/
#define MTFB_MAX_WIDTH(u32LayerId)     g_pstCap[u32LayerId].u32MaxWidth
#define MTFB_MAX_HEIGHT(u32LayerId)    g_pstCap[u32LayerId].u32MaxHeight
#define MTFB_MIN_WIDTH(u32LayerId)     g_pstCap[u32LayerId].u32MinWidth
#define MTFB_MIN_HEIGHT(u32LayerId)    g_pstCap[u32LayerId].u32MinHeight

#define IS_STEREO_SBS(par)  ((par->st3DInfo.enOutStereoMode == MTFB_STEREO_SIDEBYSIDE_HALF))
#define IS_STEREO_TAB(par)  ((par->st3DInfo.enOutStereoMode == MTFB_STEREO_TOPANDBOTTOM))
#define IS_STEREO_FPK(par)  ((par->st3DInfo.enOutStereoMode == MTFB_STEREO_FRMPACKING))

#define IS_2BUF_MODE(par)  ((par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_DOUBLE || par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_DOUBLE_IMMEDIATE))
#define IS_1BUF_MODE(par)  ((par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_ONE))


#ifdef CFG_MTFB_LOGO_SUPPORT
	#define MTFB_HD_LOGO_LAYER_ID          MTFB_LAYER_HD_1
	#define MTFB_SD_LOGO_LAYER_ID          CONFIG_MTFB_SD_LOGO_LAYER_ID
#endif


#define FBNAME "MT_FB"

#if 0
#define MTFB_FUN_IN        printk("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define MTFB_FUN_OUT     printk("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define MTFB_LOG              printk
#define MTFB_INFO            printk
#define  MTFB_LINE            printk("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#else
#define  DUMP_LOG            do{}while(0);
#define MTFB_FUN_IN        DUMP_LOG
#define MTFB_FUN_OUT     DUMP_LOG
#define MTFB_LOG(...)         DUMP_LOG
#define  MTFB_LINE            DUMP_LOG
#endif


/*************************** Structure Definition ****************************/

#ifdef CFG_MTFB_LOGO_SUPPORT
	typedef enum tagMTFB_LOGO_CHANNEL_E
	{
		MTFB_LOGO_CHN_HD = 0,
		MTFB_LOGO_CHN_SD = 1,
		MTFB_LOGO_CHN_BUTT,
	}MTFB_LOGO_CHANNEL_E;

	typedef struct tagMTFB_LOGO_INFO_S
	{
		MT_BOOL          bShow;
		MT_BOOL          bTransitted;
		MTFB_LAYER_ID_E  enLogoID;
	    struct work_struct freeLogoMemWork;
	}MTFB_LOGO_INFO_S;

	typedef struct tagMTFB_LOGO_S
	{
		mt_u32           u32LogoNum;
		MTFB_LOGO_INFO_S stLogoInfo[MTFB_LOGO_CHN_BUTT];
	}MTFB_LOGO_S;

	static MTFB_LOGO_S g_sLogo;
#endif


#ifdef CFG_MTFB_FENCE_SUPPORT
	static MTFB_SYNC_INFO_S s_SyncInfo;
#endif

#ifndef MT_ADVCA_FUNCTION_RELEASE
	static MT_BOOL  g_bProcDebug = MT_FALSE;
#endif

/** mem size of layer.mtfb will allocate mem: **/
static char* video = "";
module_param(video, charp, S_IRUGO);

static char* tc_wbc = "off";
module_param(tc_wbc, charp, S_IRUGO);


MTFB_DRV_OPS_S s_stDrvOps;
MTFB_DRV_TDEOPS_S s_stDrvTdeOps;

/* to save layer id and layer size */
MTFB_LAYER_S s_stLayer[MTFB_MAX_LAYER_NUM];

const static MTFB_CAPABILITY_S *g_pstCap;

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
	/*define tmts array to save the private info of scrolltext layer*/
	MTFB_SCROLLTEXT_INFO_S s_stTextLayer[MTFB_LAYER_ID_BUTT];
#endif


/*config layer size
	1: config layer memory size from video params
	2: config layer memory size from cfg.mak
	3: config layer memory size when usr opened layer*/
mt_u32 g_u32LayerSize[MTFB_MAX_LAYER_NUM] =
{
	/***********HD0**************/
	#ifdef CFG_MT_HD0_FB_VRAM_SIZE
		CFG_MT_HD0_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********HD1**************/
	#ifdef CFG_MT_HD1_FB_VRAM_SIZE
		CFG_MT_HD1_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********HD2**************/
	#ifdef CFG_MT_HD2_FB_VRAM_SIZE
		CFG_MT_HD2_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********HD3**************/
	#ifdef CFG_MT_HD3_FB_VRAM_SIZE
		CFG_MT_HD3_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********SD0**************/
	#ifdef CFG_MT_SD0_FB_VRAM_SIZE
		CFG_MT_SD0_FB_VRAM_SIZE,
	#else
        0,
	#endif
	/***********SD1**************/
	#ifdef CFG_MT_SD1_FB_VRAM_SIZE
		CFG_MT_SD1_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********SD2**************/
	#ifdef CFG_MT_SD2_FB_VRAM_SIZE
		CFG_MT_SD2_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********SD3**************/
	#ifdef CFG_MT_SD3_FB_VRAM_SIZE
		CFG_MT_SD3_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********AD0**************/
	#ifdef CFG_MT_AD0_FB_VRAM_SIZE
		CFG_MT_AD0_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********AD1**************/
	#ifdef CFG_MT_AD1_FB_VRAM_SIZE
		CFG_MT_AD1_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********AD2**************/
	#ifdef CFG_MT_AD2_FB_VRAM_SIZE
		CFG_MT_AD2_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********AD3**************/
	#ifdef CFG_MT_AD3_FB_VRAM_SIZE
		CFG_MT_AD3_FB_VRAM_SIZE,
	#else
		0,
	#endif
	/***********SOFT CURSOR******/
	#ifdef CFG_MT_CURSOR_FB_VRAM_SIZE
		CFG_MT_CURSOR_FB_VRAM_SIZE,
	#else
		0,
	#endif
};

/********************** Global Variable declaration **************************/

/* default fix information */
static struct fb_fix_screeninfo s_stDefFix[MTFB_LAYER_TYPE_BUTT] =
{
    {
        .id          = "mtfb",
        .type        = FB_TYPE_PACKED_PIXELS,
        .visual      = FB_VISUAL_TRUECOLOR,
        .xpanstep    = 1,
        .ypanstep    = 1,
        .ywrapstep   = 0,
        .line_length = MTFB_HD_DEF_STRIDE,
        .accel       = FB_ACCEL_NONE,
        .mmio_len    = 0,
        .mmio_start  = 0,
    },
    {
        .id          = "mtfb",
        .type        = FB_TYPE_PACKED_PIXELS,
        .visual      = FB_VISUAL_TRUECOLOR,
        .xpanstep    = 1,
        .ypanstep    = 1,
        .ywrapstep   = 0,
        .line_length = MTFB_SD_DEF_STRIDE,
        .accel       = FB_ACCEL_NONE,
        .mmio_len    = 0,
        .mmio_start  = 0,
    },
    {
        .id          = "mtfb",
        .type        = FB_TYPE_PACKED_PIXELS,
        .visual      = FB_VISUAL_TRUECOLOR,
        .xpanstep    = 1,
        .ypanstep    = 1,
        .ywrapstep   = 0,
        .line_length = MTFB_AD_DEF_STRIDE,
        .accel       = FB_ACCEL_NONE,
        .mmio_len    = 0,
        .mmio_start  = 0,
    },
    {
	    .id          = "mtfb",
	    .type        = FB_TYPE_PACKED_PIXELS,
	    .visual      = FB_VISUAL_TRUECOLOR,
	    .xpanstep    = 1,
	    .ypanstep    = 1,
	    .ywrapstep   = 0,
	    .line_length = MTFB_AD_DEF_STRIDE,
	    .accel       = FB_ACCEL_NONE,
	    .mmio_len    = 0,
	    .mmio_start  = 0,
    }
};


/* default variable information */
static struct fb_var_screeninfo s_stDefVar[MTFB_LAYER_TYPE_BUTT] =
{
    /*for HD layer*/
    {
        .xres			= MTFB_HD_DEF_WIDTH,
        .yres			= MTFB_HD_DEF_HEIGHT,
        .xres_virtual	= MTFB_HD_DEF_WIDTH,
        .yres_virtual	= MTFB_HD_DEF_HEIGHT * 2,
        .xoffset        = 0,
        .yoffset        = 0,
        .bits_per_pixel = MTFB_DEF_DEPTH,
        .red			= {16, 8, 0},
        .green			= {8, 8, 0},
        .blue			= {0, 8, 0},
        .transp			= {24, 8, 0},
        .activate		= FB_ACTIVATE_NOW,
        .pixclock		= -1, /* pixel clock in ps (pico seconds) */
        .left_margin	= -1, /* time from sync to picture	*/
        .right_margin	= -1, /* time from picture to sync	*/
        .upper_margin	= -1, /* time from sync to picture	*/
        .lower_margin	= -1,
        .hsync_len		= -1, /* length of horizontal sync	*/
        .vsync_len		= -1, /* length of vertical sync	*/
    },
    /*for SD layer*/
    {
        .xres			= MTFB_SD_DEF_WIDTH,
        .yres			= MTFB_SD_DEF_HEIGHT,
        .xres_virtual	= MTFB_SD_DEF_WIDTH,
        .yres_virtual	= MTFB_SD_DEF_HEIGHT,
        .xoffset        = 0,
        .yoffset        = 0,
        .bits_per_pixel = MTFB_DEF_DEPTH,
        .red			= {16, 8, 0},
        .green			= {8, 8, 0},
        .blue			= {0, 8, 0},
        .transp			= {24, 8, 0},
        .activate		= FB_ACTIVATE_NOW,
        .pixclock		= -1, /* pixel clock in ps (pico seconds) */
        .left_margin	= -1, /* time from sync to picture	*/
        .right_margin	= -1, /* time from picture to sync	*/
        .upper_margin	= -1, /* time from sync to picture	*/
        .lower_margin	= -1,
        .hsync_len		= -1, /* length of horizontal sync	*/
        .vsync_len		= -1, /* length of vertical sync	*/
    },
    /*for AD layer*/
    {
        .xres			= MTFB_AD_DEF_WIDTH,
        .yres			= MTFB_AD_DEF_HEIGHT,
        .xres_virtual	= MTFB_AD_DEF_WIDTH,
        .yres_virtual	= MTFB_AD_DEF_HEIGHT,
        .xoffset        = 0,
        .yoffset        = 0,
        .bits_per_pixel = MTFB_DEF_DEPTH,
        .red			= {16, 8, 0},
        .green			= {8, 8, 0},
        .blue			= {0, 8, 0},
        .transp			= {24, 8, 0},
        .activate		= FB_ACTIVATE_NOW,
        .pixclock		= -1, /* pixel clock in ps (pico seconds) */
        .left_margin	= -1, /* time from sync to picture	*/
        .right_margin	= -1, /* time from picture to sync	*/
        .upper_margin	= -1, /* time from sync to picture	*/
        .lower_margin	= -1,
        .hsync_len		= -1, /* length of horizontal sync	*/
        .vsync_len		= -1, /* length of vertical sync	*/
    },
     /*for CURSOR layer*/
    {
        .xres			= MTFB_CURSOR_DEF_WIDTH,
        .yres			= MTFB_CURSOR_DEF_HEIGHT,
        .xres_virtual	= MTFB_CURSOR_DEF_WIDTH,
        .yres_virtual	= MTFB_CURSOR_DEF_HEIGHT,
        .xoffset        = 0,
        .yoffset        = 0,
        .bits_per_pixel = MTFB_DEF_DEPTH,
        .red			= {16, 8, 0},
        .green			= {8, 8, 0},
        .blue			= {0, 8, 0},
        .transp			= {24, 8, 0},
        .activate		= FB_ACTIVATE_NOW,
        .pixclock		= -1, /* pixel clock in ps (pico seconds) */
        .left_margin	= -1, /* time from sync to picture	*/
        .right_margin	= -1, /* time from picture to sync	*/
        .upper_margin	= -1, /* time from sync to picture	*/
        .lower_margin	= -1,
        .hsync_len		= -1, /* length of horizontal sync	*/
        .vsync_len		= -1, /* length of vertical sync	*/
    }
};

/* bit filed info of color fmt, the order must be the same as MTFB_COLOR_FMT_E */
static MTFB_ARGB_BITINFO_S s_stArgbBitField[] =
{   /*RGB233*/
    {
        .stRed    = {6, 2, 0},
        .stGreen  = {3, 3, 0},
        .stBlue   = {0, 3, 0},
        .stTransp = {0, 0, 0},
    },
    /*RGB565*/
    {
        .stRed    = {11, 5, 0},
        .stGreen  = {5, 6, 0},
        .stBlue   = {0, 5, 0},
        .stTransp = {0, 0, 0},
    },
    /*RGB888*/
    {
        .stRed    = {16, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {0, 8, 0},
        .stTransp = {0, 0, 0},
    },
    /*KRGB444*/
    {
        .stRed    = {8, 4, 0},
        .stGreen  = {4, 4, 0},
        .stBlue   = {0, 4, 0},
        .stTransp = {0, 0, 0},
    },
    /*KRGB555*/
    {
        .stRed    = {10, 5, 0},
        .stGreen  = {5, 5, 0},
        .stBlue   = {0, 5, 0},
        .stTransp = {0, 0, 0},
    },
    /*KRGB888*/
    {
        .stRed    = {16,8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {0, 8, 0},
        .stTransp = {0, 0, 0},
    },
    /*ARGB4444*/
    {
        .stRed    = {8, 4, 0},
        .stGreen  = {4, 4, 0},
        .stBlue   = {0, 4, 0},
        .stTransp = {12, 4, 0},
    },
    /*ARGB1555*/
    {
        .stRed    = {10, 5, 0},
        .stGreen  = {5, 5, 0},
        .stBlue   = {0, 5, 0},
        .stTransp = {15, 1, 0},
    },
    /*ARGB8888*/
    {
        .stRed    = {16, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {0, 8, 0},
        .stTransp = {24, 8, 0},
    },
    /*ARGB8565*/
    {
        .stRed    = {11, 5, 0},
        .stGreen  = {5, 6, 0},
        .stBlue   = {0, 5, 0},
        .stTransp = {16, 8, 0},
    },
    /*RGBA4444*/
    {
        .stRed    = {12, 4, 0},
        .stGreen  = {8, 4, 0},
        .stBlue   = {4, 4, 0},
        .stTransp = {0, 4, 0},
    },
    /*RGBA5551*/
    {
        .stRed    = {11, 5, 0},
        .stGreen  = {6, 5, 0},
        .stBlue   = {1, 5, 0},
        .stTransp = {0, 1, 0},
    },
    /*RGBA5658*/
    {
        .stRed    = {19, 5, 0},
        .stGreen  = {13, 6, 0},
        .stBlue   = {8, 5, 0},
        .stTransp = {0, 8, 0},
    },
    /*RGBA8888*/
    {
        .stRed    = {24, 8, 0},
        .stGreen  = {16, 8, 0},
        .stBlue   = {8, 8, 0},
        .stTransp = {0, 8, 0},
    },
    /*BGR565*/
    {
        .stRed    = {0, 5, 0},
        .stGreen  = {5, 6, 0},
        .stBlue   = {11, 5, 0},
        .stTransp = {0, 0, 0},
    },
    /*BGR888*/
    {
        .stRed    = {0, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {16, 8, 0},
        .stTransp = {0, 0, 0},
    },
    /*ABGR4444*/
    {
        .stRed    = {0, 4, 0},
        .stGreen  = {4, 4, 0},
        .stBlue   = {8, 4, 0},
        .stTransp = {12, 4, 0},
    },
    /*ABGR1555*/
    {
        .stRed    = {0, 5, 0},
        .stGreen  = {5, 5, 0},
        .stBlue   = {10, 5, 0},
        .stTransp = {15, 1, 0},
    },
    /*ABGR8888*/
    {
        .stRed    = {0, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {16, 8, 0},
        .stTransp = {24, 8, 0},
    },
    /*ABGR8565*/
    {
        .stRed    = {0, 5, 0},
        .stGreen  = {5, 6, 0},
        .stBlue   = {11, 5, 0},
        .stTransp = {16, 8, 0},
    },
    /*KBGR444 16bpp*/
    {
        .stRed    = {0, 4, 0},
        .stGreen  = {4, 4, 0},
        .stBlue   = {8, 4, 0},
        .stTransp = {0, 0, 0},
    },
    /*KBGR555 16bpp*/
    {
        .stRed    = {0, 5, 0},
        .stGreen  = {5, 5, 0},
        .stBlue   = {10, 5, 0},
        .stTransp = {0, 0, 0},
    },
    /*KBGR888 32bpp*/
    {
        .stRed    = {0, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {16, 8, 0},
        .stTransp = {0, 0, 0},
    },

    /*1bpp*/
    {
        .stRed    = {0, 1, 0},
        .stGreen  = {0, 1, 0},
        .stBlue   = {0, 1, 0},
        .stTransp = {0, 0, 0},
    },
    /*2bpp*/
    {
        .stRed    = {0, 2, 0},
        .stGreen  = {0, 2, 0},
        .stBlue   = {0, 2, 0},
        .stTransp = {0, 0, 0},
    },
    /*4bpp*/
    {
        .stRed    = {0, 4, 0},
        .stGreen  = {0, 4, 0},
        .stBlue   = {0, 4, 0},
        .stTransp = {0, 0, 0},
    },
    /*8bpp*/
    {
        .stRed    = {0, 8, 0},
        .stGreen  = {0, 8, 0},
        .stBlue   = {0, 8, 0},
        .stTransp = {0, 0, 0},
    },
    /*ACLUT44*/
    {
        .stRed    = {4, 4, 0},
        .stGreen  = {4, 4, 0},
        .stBlue   = {4, 4, 0},
        .stTransp = {0, 4, 0},
    },
    /*ACLUT88*/
    {
        .stRed    = {8, 8, 0},
        .stGreen  = {8, 8, 0},
        .stBlue   = {8, 8, 0},
        .stTransp = {0, 8, 0},
    }
};

static mt_u32 s_stArgbBitFieldfmt[] =
{
    MTFB_FMT_RGB233,
    MTFB_FMT_RGB565,
    MTFB_FMT_RGB888,
    MTFB_FMT_ARGB4444,
    MTFB_FMT_KRGB555,
    MTFB_FMT_KRGB888,
    MTFB_FMT_ARGB4444,
    MTFB_FMT_ARGB1555,
    MTFB_FMT_ARGB8888,
    MTFB_FMT_ARGB8565,
    MTFB_FMT_RGBA4444,
    MTFB_FMT_RGBA5551,
    MTFB_FMT_RGBA5658,
    MTFB_FMT_RGBA8888,
    MTFB_FMT_BGR565,
    MTFB_FMT_BGR888,
    MTFB_FMT_ABGR4444,
    MTFB_FMT_ABGR1555,
    MTFB_FMT_ABGR8888,
    MTFB_FMT_ABGR8565,
    MTFB_FMT_KBGR444,
    MTFB_FMT_KBGR555,
    MTFB_FMT_KBGR888,
    MTFB_FMT_1BPP,
    MTFB_FMT_2BPP,
    MTFB_FMT_4BPP,
    MTFB_FMT_8BPP,
    MTFB_FMT_ACLUT44,
    MTFB_FMT_ACLUT88,

    MTFB_FMT_BUTT
};

/******************************* API declaration *****************************/
static mt_s32 mtfb_refresh(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf, MTFB_LAYER_BUF_E enBufMode);
static mt_void mtfb_select_antiflicker_mode(MTFB_PAR_S *pstPar);
static mt_s32 mtfb_refreshall(struct fb_info *info);
static void mtfb_tde_callback(mt_void *pParaml, mt_void *pParamr);
static mt_s32 mtfb_alloccanbuf(struct fb_info *info, MTFB_LAYER_INFO_S * pLayerInfo);
static mt_void mtfb_3DMode_callback(mt_void * pParaml,mt_void * pParamr);
static mt_void mtfb_assign_dispbuf(mt_u32 u32LayerId);
static mt_s32 mtfb_setcolreg(unsigned regno, unsigned red, unsigned green,unsigned blue, unsigned transp, struct fb_info *info);
#ifdef CFG_MTFB_PROC_SUPPORT
static mt_s32 mtfb_read_proc(struct seq_file *p, mt_void *v);
static mt_s32 mtfb_write_proc(struct file * file, const char __user * buf, size_t count, loff_t *ppos);
#endif

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
static mt_s32 mtfb_clearallstereobuf(struct fb_info *info);
static mt_s32 mtfb_checkandalloc_3dmem(MTFB_LAYER_ID_E enLayerId, mt_u32 u32BufferSize);
#endif



extern mt_s32 mtfb_init_module_k(mt_void);
/******************************* API realization *****************************/


/***************************************************************************************
* func          : mtfb_freelogomem_work
* description   : CNcomment: 清logo内存 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_LOGO_SUPPORT
static mt_void mtfb_freelogomem(MTFB_LOGO_CHANNEL_E enLogoChn)
{
    PDM_EXPORT_FUNC_S *ps_PdmExportFuncs = MT_NULL;

    if (MT_SUCCESS != MT_DRV_MODULE_GetFunction(MT_ID_PDM, (mt_void**)&ps_PdmExportFuncs))
    {
        return;
    }

    if(MT_NULL == ps_PdmExportFuncs)
    {
        return;
    }

	 /*msleep 80ms to asure wbc closed or screen addr switched*/
	msleep(80);
    if (MTFB_LOGO_CHN_HD == enLogoChn)
    {
        //ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(DISPLAY_BUFFER_HD);
        //ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(OPTM_GFX_WBC2_BUFFER);
        MTFB_DEBUGK("<<<<<<<<freen mem %s>>>>>>>>>\n", DISPLAY_BUFFER_HD);
        MTFB_DEBUGK("<<<<<<<<freen mem %s>>>>>>>>>\n", OPTM_GFX_WBC2_BUFFER);
    }
    else
    {
        //ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(DISPLAY_BUFFER_SD);
        MTFB_DEBUGK("<<<<<<<<freen mem %s>>>>>>>>>\n", DISPLAY_BUFFER_SD);
    }

    if (0 == g_sLogo.u32LogoNum)
    {
        //ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(MTFB_ZME_COEF_BUFFER);
        MTFB_DEBUGK("<<<<<<<<freen mem %s>>>>>>>>>\n", MTFB_ZME_COEF_BUFFER);
    }

    return;
}


static mt_void mtfb_freelogomem_work(struct work_struct *work)
{
	MTFB_LOGO_INFO_S *pstLogoInfo = NULL;
	MTFB_LOGO_CHANNEL_E enLogoChn;

	pstLogoInfo = (MTFB_LOGO_INFO_S *)container_of(work, MTFB_LOGO_INFO_S, freeLogoMemWork);

	if (IS_HD_LAYER(pstLogoInfo->enLogoID) || IS_MINOR_HD_LAYER(pstLogoInfo->enLogoID))
	{
		enLogoChn = MTFB_LOGO_CHN_HD;
	}
	else
	{
		enLogoChn = MTFB_LOGO_CHN_SD;
	}

	mtfb_freelogomem(enLogoChn);

    return;
}



/***************************************************************************************
* func          : mtfb_logo_init
* description   : CNcomment: logo初始化，这里只是设置一下掩码和对应的图层打开标记 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_logo_init(mt_void)
{
    mt_s32 s32Ret;
	MTFB_LOGO_CHANNEL_E enLogoChn;
    MTFB_OSD_DATA_S pstLogoLayerData;

	g_sLogo.u32LogoNum = 0;
	/**
	 ** 这个参数在去初始化logo中的处理还是有点问题
	 ** 高清通道，标清通道，两个通道都用是在TC双显中
	 **/
	for (enLogoChn = 0; enLogoChn < MTFB_LOGO_CHN_BUTT; enLogoChn++)
	{
		/**
		 **bTransitted是否完成开机过渡
		 **/
	    g_sLogo.stLogoInfo[enLogoChn].bTransitted = MT_FALSE;
		/**
		 **98M用HD1和SD0做为开机logo
		 **/
		g_sLogo.stLogoInfo[enLogoChn].enLogoID = (MTFB_LOGO_CHN_HD == enLogoChn) ? MTFB_HD_LOGO_LAYER_ID : MTFB_SD_LOGO_LAYER_ID;
		/**
		 ** judge whether has logo
		 ** 这里只想获取图层是否使能
		 **/
		s32Ret = s_stDrvOps.MTFB_DRV_GetOSDData(g_sLogo.stLogoInfo[enLogoChn].enLogoID, &pstLogoLayerData);
	    if (s32Ret != MT_SUCCESS)
	    {
	        MTFB_DEBUGK("failed to Get OSDData%d !\n",g_sLogo.stLogoInfo[enLogoChn].enLogoID);
	        return s32Ret;
	    }
		if (pstLogoLayerData.eState == MTFB_LAYER_STATE_ENABLE)
	    {/** 只有在有开机logo得情况下才能进入 **/
	    	g_sLogo.u32LogoNum++;
	        g_sLogo.stLogoInfo[enLogoChn].bShow = MT_TRUE;
			/** 开机logo没有过渡完 **/
	        s_stDrvOps.MTFB_DRV_SetLayerMaskFlag(g_sLogo.stLogoInfo[enLogoChn].enLogoID, MT_TRUE);
			MTFB_DEBUGK("<<<<<<<<<<<<<start with boot logo, ID:%d>>>>>>>>>>>>>>>\n", g_sLogo.stLogoInfo[enLogoChn].enLogoID);
	    }
	}

    return MT_SUCCESS;

}


/***************************************************************************************
* func          : mtfb_clear_logo
* description   : CNcomment: 清logo CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_clear_logo(mt_u32 u32LayerID, MT_BOOL bModExit)
{

	MTFB_LAYER_ID_E enLogoLayerId;
	MTFB_LOGO_CHANNEL_E enLogoChn;

	if (IS_HD_LAYER(u32LayerID) || IS_MINOR_HD_LAYER(u32LayerID))
	{
		enLogoChn = MTFB_LOGO_CHN_HD;
	}
	else
	{
		enLogoChn = MTFB_LOGO_CHN_SD;
	}

	if (!g_sLogo.stLogoInfo[enLogoChn].bShow)
	{/** 没有开机logo不需要处理，保证只进来一次g_sLogo.u32LogoNum = 1有问题**/
		return;
	}

	/** 这里用G1 **/
	enLogoLayerId = g_sLogo.stLogoInfo[enLogoChn].enLogoID;
	/** 过渡完可以切换显示地址了 **/
    s_stDrvOps.MTFB_DRV_SetLayerMaskFlag(enLogoLayerId, MT_FALSE);
    s_stDrvOps.MTFB_DRV_ClearLogo(enLogoLayerId);
    s_stDrvOps.MTFB_DRV_UpdataLayerReg(enLogoLayerId);
    /*wait for logo closed ,so we can free logo buffer*/
    //s_stDrvOps.MTFB_DRV_WaitVBlank(u32LayerID);

    //mtfb_freelogomem_work(enLogoChn);
    if (bModExit)
	{
		mtfb_freelogomem(enLogoChn);
	}
	else
	{
		INIT_WORK(&(g_sLogo.stLogoInfo[enLogoChn].freeLogoMemWork), mtfb_freelogomem_work);
    	schedule_work(&(g_sLogo.stLogoInfo[enLogoChn].freeLogoMemWork));
	}

    g_sLogo.stLogoInfo[enLogoChn].bShow = MT_FALSE;
    g_sLogo.stLogoInfo[enLogoChn].bTransitted = MT_TRUE;

    MTFB_DEBUGK("<<<<<<<<<<<<<mtfb_clear_logo>>>>>>>>>>>>>>>\n");
}
#endif

/***************************************************************************************
* func          : mtfb_getfmtbyargb
* description   : CNcomment: 从argb中判断像素格式 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static MTFB_COLOR_FMT_E mtfb_getfmtbyargb(
                                          struct fb_bitfield *red,
                                          struct fb_bitfield *green,
                                          struct fb_bitfield *blue,
                                          struct fb_bitfield *transp,
                                          mt_u32 u32ColorDepth)
{

    mt_u32 i = 0;
    mt_u32 u32Bpp = 0;
    mt_u32 pxfmt = 0;

    MTFB_FUN_IN;

    /* not support color palette low than 8bit*/
//    if (u32ColorDepth < 8)
//    {
//        return  MTFB_FMT_BUTT;
//    }

//    if (u32ColorDepth == 8)
//    {
//        return MTFB_FMT_8BPP;
//    }

    for (i = 0; i < sizeof(s_stArgbBitField)/sizeof(MTFB_ARGB_BITINFO_S); i++)
    {
        if (  (mtfb_bitfieldcmp(*red, s_stArgbBitField[i].stRed)        == 0)
            && (mtfb_bitfieldcmp(*green, s_stArgbBitField[i].stGreen)   == 0)
            && (mtfb_bitfieldcmp(*blue, s_stArgbBitField[i].stBlue)     == 0)
            && (mtfb_bitfieldcmp(*transp, s_stArgbBitField[i].stTransp) == 0))
        {
            pxfmt = s_stArgbBitFieldfmt[i];
            u32Bpp = mtfb_getbppbyfmt(pxfmt);

             //printk("for[%d] [%d] [%d]: [%d][%d][%d] [%d][%d][%d] [%d][%d][%d] [%d][%d][%d] [%d]\n", i, pxfmt, u32Bpp, s_stArgbBitField[i].stRed.length, s_stArgbBitField[i].stRed.offset, s_stArgbBitField[i].stRed.msb_right, s_stArgbBitField[i].stGreen.length, s_stArgbBitField[i].stGreen.offset, s_stArgbBitField[i].stGreen.msb_right, s_stArgbBitField[i].stBlue.length, s_stArgbBitField[i].stBlue.offset, s_stArgbBitField[i].stBlue.msb_right, s_stArgbBitField[i].stTransp.length, s_stArgbBitField[i].stTransp.offset, s_stArgbBitField[i].stTransp.msb_right, u32ColorDepth);

            if (u32Bpp == u32ColorDepth)
            {
                return pxfmt;
            }
        }
    }
    pxfmt = MTFB_FMT_BUTT;

    switch(u32ColorDepth)
    {
        case 16:
            pxfmt = MTFB_FMT_RGB565;
            break;
        case 24:
            pxfmt = MTFB_FMT_RGB888;
            break;
        case 32:
            pxfmt = MTFB_FMT_ARGB8888;
            break;
        default:
            pxfmt = MTFB_FMT_BUTT;
            break;
    }

    if(MTFB_FMT_BUTT != pxfmt)
    {
        for (i = 0; i < sizeof(s_stArgbBitField)/sizeof(MTFB_ARGB_BITINFO_S); i++)
        {
            if(s_stArgbBitFieldfmt[i] == pxfmt)
            {
              break;
            }
        }

        *red    = s_stArgbBitField[i].stRed;
        *green  = s_stArgbBitField[i].stGreen;
        *blue   = s_stArgbBitField[i].stBlue;
        *transp = s_stArgbBitField[i].stTransp;
    }

    MTFB_FUN_OUT;

    return pxfmt;
}


/***************************************************************************************
* func          : mtfb_realloc_layermem
* description   : CNcomment: 要是内存不够重新分配内存 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 mtfb_realloc_layermem(struct fb_info *info,mt_u32 u32BufSize)
{
	mt_char name[32];
	MTFB_PAR_S *par;

   MTFB_FUN_IN;

	if (MT_NULL == info)
	{
		return MT_FAILURE;
	}

	par = (MTFB_PAR_S *)info->par;

	if (0 == u32BufSize)
	{
		return MT_SUCCESS;
	}

    if (info->screen_base != MT_NULL)
    {
        mtfb_buf_ummap(info->screen_base);
    }

    if (info->fix.smem_start != 0)
    {
        mtfb_buf_freemem(info->fix.smem_start);
		MTFB_DEBUGK("free the video memory, phyaddr: 0x%lx!\n", info->fix.smem_start);
    }

    /**
     ** Modify 16 to 32, preventing out of bound
     ** initialize the fix screen info
     **/
	snprintf(name, sizeof(name),"MTFB_Fb%d", par->stBaseInfo.u32LayerID);
	name[sizeof(name) - 1] = '\0';
    info->fix.smem_start = mtfb_buf_allocmem(name, u32BufSize);
    if (0 == info->fix.smem_start)
    {
        MTFB_DEBUGK("%s:failed to malloc the video memory, size: %d KBtyes!\n", name, u32BufSize/1024);
        return MT_FAILURE;
    }
    else
    {
        info->fix.smem_len = u32BufSize;
        /**
         ** initialize the virtual address and clear memory
         **/
        info->screen_base = mtfb_buf_map(info->fix.smem_start);
        if (MT_NULL == info->screen_base)
        {
            MTFB_WARNING("Failed to call map video memory, ""size:%d KBytes, start: 0x%lx\n",
                                                             info->fix.smem_len/1024,
                                                             info->fix.smem_start);
        }
        else
        {
            memset(info->screen_base, 0x00, info->fix.smem_len);
        }

		MTFB_DEBUGK("%s:success to malloc the video memory, size: %d KBtyes!\n", name, u32BufSize/1024);
    }

   MTFB_FUN_OUT;

	return MT_SUCCESS;

}



/***************************************************************************************
* func          : mtfb_checkmem_enough
* description   : CNcomment: 判断内存是否足够 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 mtfb_checkmem_enough(struct fb_info *info,mt_u32 u32Pitch,mt_u32 u32Height)
{
    mt_u32 u32BufferNum = 0;
    mt_u32 u32Buffersize = 0;
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    switch(par->stExtendInfo.enBufMode)
    {
        case MTFB_LAYER_BUF_DOUBLE:
        case MTFB_LAYER_BUF_DOUBLE_IMMEDIATE:
        {
            u32BufferNum = 2;
            break;
        }
        case MTFB_LAYER_BUF_ONE:
		case MTFB_LAYER_BUF_STANDARD:
        {
            u32BufferNum = 1;
            break;
        }
        default:
            return MT_SUCCESS;
    }

    u32Buffersize = u32BufferNum * u32Pitch * u32Height;


    if(info->fix.smem_len >= u32Buffersize)
    {
        MTFB_FUN_OUT;

        return MT_SUCCESS;
    }

	MTFB_DEBUGK("memory is not enough!  now is %d u32Pitch %d u32Height %d expect %d\n",info->fix.smem_len,u32Pitch, u32Height,u32Buffersize);

    MTFB_FUN_OUT;

    return MT_FAILURE;

}


/***************************************************************************************
* func          : mtfb_check_fmt
* description   : CNcomment: 判断像素格式是否合法 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_check_fmt(struct fb_var_screeninfo *var, struct fb_info *info)
{

    mt_u32 u32MaxXRes = 0;
    mt_u32 u32MaxYRes = 0;
	MTFB_PAR_S *par    = NULL;
	mt_u32 u32LayerID  = 0;
    MTFB_COLOR_FMT_E enFmt = MTFB_FMT_BUTT;

    MTFB_FUN_IN;

	par = (MTFB_PAR_S *)info->par;
	u32LayerID = par->stBaseInfo.u32LayerID;

	/**
	 **用户会设置可变屏幕信息
	 **/
    enFmt = mtfb_getfmtbyargb(&var->red,
                                &var->green,
                                &var->blue,
                                &var->transp,
                                var->bits_per_pixel);
	if (enFmt == MTFB_FMT_BUTT)
    {
        MTFB_DEBUGK("Unknown fmt(offset, length) \
			         r:(%d,%d,%d) ,              \
			         g:(%d,%d,%d),               \
			         b(%d,%d,%d),                \
			         a(%d,%d,%d),                \
			         bpp:%d!\n",                 \
                    var->red.offset,    var->red.length,    var->red.msb_right,   \
                    var->green.offset,  var->green.length,  var->green.msb_right, \
                    var->blue.offset,   var->blue.length,   var->blue.msb_right,  \
                    var->transp.offset, var->transp.length, var->transp.msb_right,\
                    var->bits_per_pixel);

        MTFB_FUN_OUT;

        return -EINVAL;
    }

	/**
	 **初始化的时候已经获取到g_pstCap的值信息
	 **/
    if (  (!g_pstCap[par->stBaseInfo.u32LayerID].bColFmt[enFmt])
        || (!s_stDrvTdeOps.MTFB_DRV_TdeSupportFmt(enFmt) && par->stExtendInfo.enBufMode != MTFB_LAYER_BUF_STANDARD))
    {
        MTFB_DEBUGK("Unsupported PIXEL FORMAT!\n");
        MTFB_FUN_OUT;
        return -EINVAL;
    }

    /**
     ** virtual resolution must be no less than minimal resolution
     **/
    if (var->xres_virtual < MTFB_MIN_WIDTH(u32LayerID))
    {
        var->xres_virtual = MTFB_MIN_WIDTH(u32LayerID);
    }
    if (var->yres_virtual < MTFB_MIN_HEIGHT(u32LayerID))
    {
        var->yres_virtual = MTFB_MIN_HEIGHT(u32LayerID);
    }

    /**
     ** just needed to campare display resolution with virtual resolution,
     ** because VO grapmtc layer can do scaler,display resolution >current
     ** standard resolution
     **/
    u32MaxXRes = var->xres_virtual;
    if (var->xres > u32MaxXRes)
    {
        var->xres = u32MaxXRes;
    }
    else if (var->xres < MTFB_MIN_WIDTH(u32LayerID))
    {
        var->xres = MTFB_MIN_WIDTH(u32LayerID);
    }

    u32MaxYRes = var->yres_virtual;
    if (var->yres > u32MaxYRes)
    {
        var->yres = u32MaxYRes;
    }
    else if (var->yres < MTFB_MIN_HEIGHT(u32LayerID))
    {
        var->yres = MTFB_MIN_HEIGHT(u32LayerID);
    }

//    MTFB_DEBUGK("xres:%d,   yres:%d,    xres_virtual:%d,    yres_virtual:%d\n",
//                var->xres, var->yres, var->xres_virtual,  var->yres_virtual);

    /**
     ** check if the offset is valid
     **/
    if (   (var->xoffset > var->xres_virtual)
        || (var->yoffset > var->yres_virtual)
        || (var->xoffset + var->xres > var->xres_virtual)
        || (var->yoffset + var->yres > var->yres_virtual))
    {
        MTFB_DEBUGK("offset is invalid! xoffset:%d, yoffset:%d\n", var->xoffset, var->yoffset);
        MTFB_FUN_OUT;
        return -EINVAL;
    }

   MTFB_FUN_OUT;

	return MT_SUCCESS;

}



/***************************************************************************************
* func          : mtfb_check_var
* description   : CNcomment: 判断参数是否合法 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    if (pstPar->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
    {
        MTFB_DEBUGK("cursor layer doesn't support tmts operation!\n");
        return MT_FAILURE;
    }

    MTFB_FUN_OUT;
    return mtfb_check_fmt(var, info);
}


/***************************************************************************************
* func          : mtfb_3DData_Config
* description   : CNcomment: 3D数据配置 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
static mt_s32 mtfb_3DData_Config(MTFB_LAYER_ID_E enLayerId, MTFB_BUFFER_S *pstBuffer, MTFB_BLIT_OPT_S *pstBlitOpt)
{
	MTFB_PAR_S *pstPar;
	struct fb_info *info;
	mt_s32 s32Ret;
	unsigned long lockflag;

	MTFB_BUFFER_S st3DBuf;

	info   = s_stLayer[enLayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)(info->par);

	spin_lock_irqsave(&pstPar->stBaseInfo.lock,lockflag);
	pstPar->stRunInfo.bNeedFlip        = MT_FALSE; /** 内容还没有赋值，不需要刷新 **/
	pstPar->stRunInfo.s32RefreshHandle = 0;        /** 这对TDE任务的              **/
	spin_unlock_irqrestore(&pstPar->stBaseInfo.lock,lockflag);

	/**
	 ** config 3D buffer wmtch set to hardware
	 **/
	memcpy(&st3DBuf.stCanvas, &pstPar->st3DInfo.st3DSurface, sizeof(MTFB_SURFACE_S));

	/**
	 ** Left Eye Region
	 **/
	if (MTFB_STEREO_SIDEBYSIDE_HALF == pstPar->st3DInfo.enOutStereoMode)
	{
		st3DBuf.stCanvas.u32Width >>= 1;
	}
	else if (MTFB_STEREO_TOPANDBOTTOM == pstPar->st3DInfo.enOutStereoMode)
	{
		st3DBuf.stCanvas.u32Height >>= 1;
	}

	st3DBuf.UpdateRect.x = 0;
	st3DBuf.UpdateRect.y = 0;
	st3DBuf.UpdateRect.w = st3DBuf.stCanvas.u32Width;
	st3DBuf.UpdateRect.h = st3DBuf.stCanvas.u32Height;

	/**
	 **局部操作有TDE重新计算dst rect，第一个参数src 第二个参数dst
	 **/
	s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(pstBuffer, &st3DBuf, pstBlitOpt, MT_TRUE);
	if (s32Ret < 0)
	{
	    MTFB_DEBUGK("tde blit error!\n");
	    return MT_FAILURE;
	}

	pstPar->stRunInfo.bModifying          = MT_TRUE;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
	pstPar->stRunInfo.bModifying          = MT_FALSE;

    /** TDE刷新句柄 **/
	pstPar->stRunInfo.s32RefreshHandle = s32Ret;

	return MT_SUCCESS;

}
#endif


/***************************************************************************************
* func          : mtfb_assign_dispbuf
* description   : CNcomment: 分配display buffer CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_assign_dispbuf(mt_u32 u32LayerId)
{

    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)(info->par);
    mt_u32 u32BufSize = 0;
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	mt_u32 u32InSize  = 0;
#endif
//	u32InSize = u32InSize;

    MTFB_FUN_IN;

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    if (pstPar->bSetStereoMode)
    {

    	mt_u32 u32Stride;
        mt_u32 u32StartAddr;
        /**
         ** there's a limit from hardware that screen buf shoule be 16 bytes
         ** aligned,maybe it's proper to get tmts info from drv adapter
         **/
        u32InSize = info->var.xres * info->var.bits_per_pixel >> 3;
		MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);
		/**
		 **info->var.yres 双buffer切换地址使用
		 **/
        u32BufSize = ((u32Stride * info->var.yres)+0xf)&0xfffffff0;

		/**
		 ** in 2buf and 1buf refresh mode, we can use N3D buffer to save 3D data
		 **/
		if (IS_2BUF_MODE(pstPar) || IS_1BUF_MODE(pstPar))
		{
			u32StartAddr = info->fix.smem_start;
		}
		else if ( (0 == pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart)
                || (0 == pstPar->st3DInfo.st3DMemInfo.u32StereoMemLen)
                || (0 == pstPar->stRunInfo.u32BufNum))
		{
			return;
		}
		else
		{
			u32StartAddr = pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart;
		}

		if (1 == pstPar->stRunInfo.u32BufNum)
        {
            pstPar->st3DInfo.u32DisplayAddr[0] = u32StartAddr;
			pstPar->st3DInfo.u32DisplayAddr[1] = u32StartAddr;
        }
        else if (2 == pstPar->stRunInfo.u32BufNum)
        {/** 使用双buferr **/
            pstPar->st3DInfo.u32DisplayAddr[0] = u32StartAddr;
            pstPar->st3DInfo.u32DisplayAddr[1] = u32StartAddr + u32BufSize;
        }
    }
    else
#endif
    {
        /**
         ** there's a limit from hardware that screen buf shoule be 16 bytes
         ** aligned,maybe it's proper to get tmts info from drv adapter
         **/
        u32BufSize = ((info->fix.line_length * info->var.yres)+ 0xf) & 0xfffffff0;

        if (info->fix.smem_len == 0)
        {
            return;
        }
        else if ((info->fix.smem_len >= u32BufSize) && (info->fix.smem_len < u32BufSize * 2))
        {
            pstPar->stDispInfo.u32DisplayAddr[0] = info->fix.smem_start;
            pstPar->stDispInfo.u32DisplayAddr[1] = info->fix.smem_start;
        }
        else if (info->fix.smem_len >= u32BufSize * 2)
        {/** 这里使用双buffer **/
            pstPar->stDispInfo.u32DisplayAddr[0] = info->fix.smem_start;
            pstPar->stDispInfo.u32DisplayAddr[1] = info->fix.smem_start + u32BufSize;
        }
    }

    MTFB_FUN_OUT;

    return;

}

/***************************************************************************************
* func          : mtfb_getupdate_rect
* description   : CNcomment: 获取图层的更新区域 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_getupdate_rect(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf, MTFB_RECT *pstUpdateRect)
{
	MTFB_PAR_S *pstPar;
    struct fb_info *info;

	TDE2_RECT_S SrcRect   = {0};
	TDE2_RECT_S DstRect   = {0};
	TDE2_RECT_S InSrcRect = {0};
	TDE2_RECT_S InDstRect = {0};

	info     = s_stLayer[u32LayerId].pstInfo;
	pstPar   = (MTFB_PAR_S *)info->par;

   MTFB_FUN_IN;

	memset(&InDstRect, 0, sizeof(TDE2_RECT_S));

	SrcRect.u32Width  = pstCanvasBuf->stCanvas.u32Width;
	SrcRect.u32Height = pstCanvasBuf->stCanvas.u32Height;
	if (pstPar->st3DInfo.enOutStereoMode == MTFB_STEREO_SIDEBYSIDE_HALF)
	{
		DstRect.u32Width  = pstPar->stExtendInfo.u32DisplayWidth >> 1;
		DstRect.u32Height = pstPar->stExtendInfo.u32DisplayHeight;
	}
	else if (pstPar->st3DInfo.enOutStereoMode == MTFB_STEREO_TOPANDBOTTOM)
	{
		DstRect.u32Width  = pstPar->stExtendInfo.u32DisplayWidth;
		DstRect.u32Height = pstPar->stExtendInfo.u32DisplayHeight >> 1;
	}
	else
	{
		DstRect.u32Width  = pstPar->stExtendInfo.u32DisplayWidth;
		DstRect.u32Height = pstPar->stExtendInfo.u32DisplayHeight;
	}

	if(    SrcRect.u32Width  != DstRect.u32Width
		|| SrcRect.u32Height != DstRect.u32Height)
	{
		memcpy(&InSrcRect, &pstCanvasBuf->UpdateRect, sizeof(MTFB_RECT));
	    s_stDrvTdeOps.MTFB_DRV_CalScaleRect(&SrcRect, &DstRect, &InSrcRect, &InDstRect);
		memcpy(pstUpdateRect, &InDstRect, sizeof(MTFB_RECT));
	}
	else
	{
		memcpy(pstUpdateRect, &pstCanvasBuf->UpdateRect, sizeof(MTFB_RECT));
	}

    MTFB_FUN_OUT;

	return MT_SUCCESS;

}

/***************************************************************************************
* func			: mtfb_backup_forebuf
* description	: CNcomment: 更新前景数据CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
static mt_s32 mtfb_backup_forebuf(mt_u32 u32LayerId, MTFB_BUFFER_S *pstBackBuf)
{
	mt_s32 s32Ret;
	mt_u32 u32ForePhyAddr;
	MTFB_PAR_S *pstPar;
	MTFB_RECT  *pstForeUpdateRect;
    struct fb_info *info;
	MTFB_BUFFER_S stForeBuf;
	MTFB_BUFFER_S stBackBuf;
	MTFB_BLIT_OPT_S stBlitTmp;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

	memcpy(&stBackBuf, pstBackBuf, sizeof(MTFB_BUFFER_S));

	if (   pstPar->st3DInfo.enOutStereoMode != MTFB_STEREO_MONO
		&& pstPar->st3DInfo.enOutStereoMode != MTFB_STEREO_BUTT)
	{
		pstForeUpdateRect = &pstPar->st3DInfo.st3DUpdateRect;
		u32ForePhyAddr= pstPar->st3DInfo.u32DisplayAddr[1-pstPar->stRunInfo.u32IndexForInt];
	}
	else
	{
		pstForeUpdateRect = &pstPar->stDispInfo.stUpdateRect;
		u32ForePhyAddr= pstPar->stDispInfo.u32DisplayAddr[1-pstPar->stRunInfo.u32IndexForInt];
	}

	if (pstPar->st3DInfo.enOutStereoMode == MTFB_STEREO_SIDEBYSIDE_HALF)
	{
		stBackBuf.stCanvas.u32Width  = stBackBuf.stCanvas.u32Width >> 1;
	}
	else if (pstPar->st3DInfo.enOutStereoMode == MTFB_STEREO_TOPANDBOTTOM)
	{
		stBackBuf.stCanvas.u32Height = stBackBuf.stCanvas.u32Height >> 1;
	}

    /** backup fore buffer **/
    if (!mtfb_iscontain(&stBackBuf.UpdateRect, pstForeUpdateRect))
    {
    	memcpy(&stForeBuf, &stBackBuf, sizeof(MTFB_BUFFER_S));
	    stForeBuf.stCanvas.u32PhyAddr = u32ForePhyAddr;
	    memcpy(&stForeBuf.UpdateRect, pstForeUpdateRect, sizeof(MTFB_RECT));
	    memcpy(&stBackBuf.UpdateRect, &stForeBuf.UpdateRect , sizeof(MTFB_RECT));
	    memset(&stBlitTmp, 0x0, sizeof(stBlitTmp));

        s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(&stForeBuf, &stBackBuf, &stBlitTmp, MT_TRUE);
        if (s32Ret <= 0)
        {
            MTFB_FUN_OUT;
            MTFB_DEBUGK("2buf  blit err 4!\n");
            return MT_FAILURE;
        }
    }

    MTFB_FUN_OUT;

	return MT_SUCCESS;
}
/***************************************************************************************
* func          : mtfb_wait_regconfig_work
* description   : CNcomment: 等待寄存器配置完成 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_wait_regconfig_work(MTFB_LAYER_ID_E enLayerId)
{
    MTFB_FUN_IN;

	s_stDrvOps.MTFB_DRV_WaitVBlank(enLayerId);

    MTFB_FUN_OUT;

	return MT_SUCCESS;
}
/***************************************************************************************
* func          : mtfb_disp_setdispsize
* description   : CNcomment: set display size CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_disp_setdispsize(mt_u32 u32LayerId, mt_u32 u32Width, mt_u32 u32Height)
{
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;
    mt_u32 u32Pitch;

    MTFB_FUN_IN;
    if ((pstPar->stExtendInfo.u32DisplayWidth == u32Width) && (pstPar->stExtendInfo.u32DisplayHeight == u32Height))
    {
        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    u32Pitch = u32Width * info->var.bits_per_pixel >> 3;
    u32Pitch = (u32Pitch + 0xf) & 0xfffffff0;

	if(MT_FAILURE == mtfb_checkmem_enough(info, u32Pitch, u32Height))
	{
	   MTFB_FUN_OUT;
	   return MT_FAILURE;
	}

    pstPar->stExtendInfo.u32DisplayWidth  = u32Width;
    pstPar->stExtendInfo.u32DisplayHeight = u32Height;

    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_INRECT;

    /** here we need to tmtnk about how to resist flicker again,
     ** we use VO do flicker resist before , but now if the display H size is the
     ** same as the screen, VO will not do flicker resist, so should choose TDE
     ** to do flicker resist
     **/
    mtfb_select_antiflicker_mode(pstPar);

    MTFB_FUN_OUT;

    return MT_SUCCESS;

}


  /* we handle it by two case:
      case 1 : if VO support Zoom, we only change screeen size, display size keep not change
      case 2: if VO can't support zoom, display size should keep the same as screen size*/
static mt_s32 mtfb_disp_setscreensize(mt_u32 u32LayerId, mt_u32 u32Width, mt_u32 u32Height)
{
#ifndef CFG_MTFB_VIRTUAL_COORDINATE_SUPPORT
	MTFB_RECT stDispRect;
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    if (0 == u32Width || 0 == u32Height)
    {
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }

    pstPar->stExtendInfo.u32ScreenWidth  = u32Width;
    pstPar->stExtendInfo.u32ScreenHeight = u32Height;

    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_OUTRECT;

    /* Here  we need to tmtnk about how to resist flicker again, we use VO do flicker resist before , but now if the display H size is the
    	     same as the screen, VO will not do flicker resist, so should choose TDE to do flicker resist*/
    mtfb_select_antiflicker_mode(pstPar);
#endif
    MTFB_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 mtfb_freeccanbuf(MTFB_PAR_S *par)
{
    MTFB_FUN_IN;

    if (MT_NULL != par->stDispInfo.stCanvasSur.u32PhyAddr)
    {
        mtfb_buf_freemem(par->stDispInfo.stCanvasSur.u32PhyAddr);
    }

    par->stDispInfo.stCanvasSur.u32PhyAddr = 0;

    MTFB_FUN_OUT;

    return MT_SUCCESS;
}


/***************************************************************************************
* func          : mtfb_allocstereobuf
* description   : CNcomment: 分配3D buffer CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
mt_s32 mtfb_freestereobuf(MTFB_PAR_S *par)
{
   MTFB_FUN_IN;

	/**
	 **清楚旧的3D buffer
	 **/
    if (MT_NULL != par->st3DInfo.st3DMemInfo.u32StereoMemStart)
    {
        mtfb_buf_freemem(par->st3DInfo.st3DMemInfo.u32StereoMemStart);
    }

    par->st3DInfo.st3DMemInfo.u32StereoMemStart = 0;
    par->st3DInfo.st3DMemInfo.u32StereoMemLen   = 0;
    par->st3DInfo.st3DSurface.u32PhyAddr        = 0;

    MTFB_FUN_OUT;

    return MT_SUCCESS;
}

/***************************************************************************************
* func          : mtfb_clearstereobuf
* description   : CNcomment: 清楚3D buffer CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_clearstereobuf(struct fb_info *info)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    if(par->st3DInfo.st3DMemInfo.u32StereoMemStart && par->st3DInfo.st3DMemInfo.u32StereoMemLen)
    {
        MTFB_BLIT_OPT_S stOpt;
        memset(&stOpt, 0x0, sizeof(stOpt));
        par->st3DInfo.st3DSurface.u32PhyAddr = par->st3DInfo.st3DMemInfo.u32StereoMemStart;
		/**
		 **清3D surface
		 **/
		/**
		 **这里软件memset和tde填充buffer内容性能差异有多大?要是性能相当是否可以用
		 **memset
		 **/
		s_stDrvTdeOps.MTFB_DRV_ClearRect(&(par->st3DInfo.st3DSurface), &stOpt);
    }

    MTFB_FUN_IN;

    return;

}
/***************************************************************************************
* func          : mtfb_allocstereobuf
* description   : CNcomment: 分配3D buffer CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_allocstereobuf(struct fb_info *info, mt_u32 u32BufSize)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;
    mt_char name[32] = "";

     MTFB_FUN_IN;

    if (0 == u32BufSize)
    {
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }

    if (u32BufSize == par->st3DInfo.st3DMemInfo.u32StereoMemLen)
    {
        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    /**
     ** with old stereo buffer
     **/
    if (par->st3DInfo.st3DMemInfo.u32StereoMemStart)
    {
        /** free old buffer*/
        MTFB_DEBUGK("free old stereo buffer\n");
        mtfb_freestereobuf(par);
    }


    /**
     **alloc new stereo buffer
     **/
    snprintf(name, sizeof(name), "MTFB_StereoBuf%d", par->stBaseInfo.u32LayerID);
    par->st3DInfo.st3DMemInfo.u32StereoMemStart = mtfb_buf_allocmem(name, u32BufSize);
    if (0 == par->st3DInfo.st3DMemInfo.u32StereoMemStart)
    {
        MTFB_DEBUGK("alloc stereo buffer no mem, u32BufSize:%d\n", u32BufSize);
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }
    /**
     **3d buffer大小
     **/
    par->st3DInfo.st3DMemInfo.u32StereoMemLen = u32BufSize;

    MTFB_DEBUGK("alloc new memory for stereo buffer success\n");

    MTFB_FUN_OUT;

    return MT_SUCCESS;

}
#endif


/***************************************************************************************
* func          : mtfb_set_par
* description   : CNcomment: 配置参数 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_set_par(struct fb_info *info)
{
	mt_s32 s32Ret     = MT_SUCCESS;
	mt_u32 u32Stride  = 0;
	mt_u32 u32BufSize = 0;
	mt_u32 u32InSize  = 0;
    MTFB_PAR_S *pstPar = NULL;
	mt_u32 u32StartAddr = 0;

    MTFB_COLOR_FMT_E enFmt;
    pstPar = (MTFB_PAR_S *)info->par;

   MTFB_FUN_IN;

	/**
	 **根据屏幕可变信息来获取像素格式
	 **/
	enFmt = mtfb_getfmtbyargb(&info->var.red, &info->var.green, &info->var.blue, &info->var.transp, info->var.bits_per_pixel);

	u32InSize = info->var.xres_virtual * info->var.bits_per_pixel >> 3;
	MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);

	/**
	 **open中初始化图层设置为false,pandisplay之后会设置为true，之后为true
	 **/
	if (!pstPar->bPanFlag)
	{/** 该配置的参数已经刷新过了，则重新配置参数 **/
		u32BufSize = u32Stride * info->var.yres_virtual;

       MTFB_LINE;

		/**
		 **info->fix.smem_len初始化配置的，要考虑双buffer
		 **/
    	if (u32BufSize > info->fix.smem_len)
    	{
    		s32Ret = mtfb_realloc_layermem(info, u32BufSize);
			if (MT_FAILURE == s32Ret)
			{
			    MTFB_FUN_OUT;
				return MT_FAILURE;
			}

           MTFB_LINE;

			/**
			 **使用第一块内存
			 **/
			pstPar->stRunInfo.u32IndexForInt = 0;

			mtfb_assign_dispbuf(pstPar->stBaseInfo.u32LayerID);

			pstPar->stRunInfo.bModifying          = MT_TRUE;
			pstPar->stRunInfo.u32ScreenAddr       = info->fix.smem_start;
        	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
        	pstPar->stRunInfo.bModifying          = MT_FALSE;
    	}
	}
	else
	{
	    MTFB_LINE;

		s32Ret = mtfb_checkmem_enough(info, u32Stride, info->var.yres_virtual);
		if (MT_FAILURE == s32Ret)
		{
		    MTFB_FUN_OUT;
			return MT_FAILURE;
		}
	}

	/**
	 **这两个参数原先是为了开机LOGO过渡使用的，现在
	 **不需要了，通过掩码来控制
	 **/
	if (!pstPar->bSetVar)
	{
	    MTFB_LINE;
		pstPar->bSetVar   = MT_TRUE;
		pstPar->bPanReady = MT_FALSE;
	}
	else
	{
	   /** 参数设置完可以pandisplay**/
		pstPar->bPanReady = MT_TRUE;
       MTFB_LINE;
	}

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    if(   (pstPar->bSetStereoMode)
        && (MTFB_LAYER_BUF_STANDARD == pstPar->stExtendInfo.enBufMode))
    {
    	mt_u32 u32BufferSize;
		/**
		 ** the stride of 3D buffer
		 **/
		u32InSize = info->var.xres * info->var.bits_per_pixel >> 3;
		MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);

        u32BufferSize = u32Stride * info->var.yres;
		u32BufferSize *= pstPar->stRunInfo.u32BufNum; /** 内存个数 **/

		/*config 3D surface par*/
		pstPar->st3DInfo.st3DSurface.enFmt    = enFmt;
		pstPar->st3DInfo.st3DSurface.u32Width = info->var.xres;
		pstPar->st3DInfo.st3DSurface.u32Height= info->var.yres;

		pstPar->stRunInfo.bModifying          = MT_TRUE;
		pstPar->st3DInfo.st3DSurface.u32Pitch = u32Stride;
	    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;
		pstPar->stRunInfo.bModifying          = MT_FALSE;

		s32Ret = mtfb_checkandalloc_3dmem(pstPar->stBaseInfo.u32LayerID, u32BufferSize);
		if (MT_SUCCESS != s32Ret)
		{
			MTFB_DEBUGK("fail to alloc 3d memory, set_fb_par failure.\n ");
           MTFB_FUN_OUT;
			return s32Ret;
		}

		/*the stride of N3D buffer*/
	   u32InSize = info->var.xres_virtual * info->var.bits_per_pixel >> 3;
	   MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);
       info->fix.line_length = u32Stride;

    }
    else
#endif
    {
        /* set the stride if stride change */
		u32InSize = info->var.xres_virtual * info->var.bits_per_pixel >> 3;
	    MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);

        u32StartAddr = info->fix.smem_start ;

        MTFB_LINE;

        if(  u32Stride != info->fix.line_length ||(info->var.yres != pstPar->stExtendInfo.u32DisplayHeight))
        {
        	pstPar->stRunInfo.bModifying     = MT_TRUE;
			info->fix.line_length            = u32Stride;
            mtfb_assign_dispbuf(pstPar->stBaseInfo.u32LayerID);
            pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;
			pstPar->stRunInfo.bModifying      = MT_FALSE;

           MTFB_LINE;
        }
    }

    MTFB_LINE;

    if ((pstPar->stExtendInfo.enColFmt != enFmt))
    {/** 像素格式改变，初始化有设置一个pstPar->stExtendInfo.enColFmt **/

       MTFB_LINE;

		mtfb_freeccanbuf(pstPar);

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
		if(s_stTextLayer[pstPar->stBaseInfo.u32LayerID].bAvailable)
		{
			mt_u32 i;
			for (i = 0; i < SCROLLTEXT_NUM; i++)
			{
				if (s_stTextLayer[pstPar->stBaseInfo.u32LayerID].stScrollText[i].bAvailable)
				{
					mtfb_freescrolltext_cachebuf(&(s_stTextLayer[pstPar->stBaseInfo.u32LayerID].stScrollText[i]));
					memset(&s_stTextLayer[pstPar->stBaseInfo.u32LayerID].stScrollText[i],0,sizeof(MTFB_SCROLLTEXT_S));
				}
			}

		   MTFB_LINE;
			s_stTextLayer[pstPar->stBaseInfo.u32LayerID].bAvailable      = MT_FALSE;
			s_stTextLayer[pstPar->stBaseInfo.u32LayerID].u32textnum      = 0;
			s_stTextLayer[pstPar->stBaseInfo.u32LayerID].u32ScrollTextId = 0;
		}
#endif
		pstPar->stRunInfo.bModifying   = MT_TRUE;
        pstPar->stExtendInfo.enColFmt = enFmt;
        pstPar->stRunInfo.u32ParamModifyMask  |= MTFB_LAYER_PARAMODIFY_FMT;
		pstPar->stRunInfo.bModifying = MT_FALSE;

    }

    MTFB_LINE;

    /* If xres or yres change */
    if (   info->var.xres != pstPar->stExtendInfo.u32DisplayWidth
        || info->var.yres != pstPar->stExtendInfo.u32DisplayHeight)
    {
        MTFB_LINE;

        if ((0 == info->var.xres) || (0 == info->var.yres))
        {
            MTFB_LINE;

            if (MT_TRUE == pstPar->stExtendInfo.bShow)
            {
                MTFB_LINE;
            	  pstPar->stRunInfo.bModifying          = MT_TRUE;
                pstPar->stExtendInfo.bShow            = MT_FALSE;
                pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_SHOW;
				  pstPar->stRunInfo.bModifying          = MT_FALSE;
            }
        }

       MTFB_LINE;
       mtfb_disp_setdispsize  (pstPar->stBaseInfo.u32LayerID, info->var.xres, info->var.yres);
		mtfb_assign_dispbuf(pstPar->stBaseInfo.u32LayerID);
        MTFB_LINE;
    }

    MTFB_FUN_OUT;

    return 0;

}



/***************************************************************************************
* func          : mtfb_pan_display
* description   : CNcomment: 标准刷新流程，android单板起来会调用一次，其它情况下
                             要是非3D不会调用 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;
    mt_u32 u32DisplayAddr = 0;
    mt_u32 u32StartAddr   = info->fix.smem_start ;

    MTFB_FUN_IN;

	if(MTFB_LAYER_BUF_STANDARD != par->stExtendInfo.enBufMode)
	{/** 不是标准刷新就不操作了 **/
	    MTFB_FUN_OUT;
		return MT_SUCCESS;
	}

    if (!par->bPanReady)
    {
        MTFB_LINE;
        par->bPanReady = MT_TRUE;
		#ifdef CFG_MTFB_LOGO_TO_APP
		return MT_SUCCESS;
		#endif
    }

    /*stereo 3d  mode*/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    if(par->bSetStereoMode && par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD)
    {/** 标准刷新，3D模式 **/
    	mt_s32 s32Ret     = 0;
    	mt_u32 u32TmpAddr = 0;
		mt_u32 u32Stride  = 0;
		mt_u32 u32InSize  = 0;
		MTFB_BLIT_OPT_S stBlitOpt;
		MTFB_BUFFER_S stDispBuf;
		mt_u32 u32BufferSize = 0;

		/**
		 ** the stride of 3D buffer
		 **/
		u32InSize = info->var.xres * info->var.bits_per_pixel >> 3;
	    MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);

       MTFB_LINE;

		u32BufferSize = u32Stride * info->var.yres;
		/** buffer数，初始化中设置，两个 **/
		u32BufferSize *= par->stRunInfo.u32BufNum;

	    /**
	     ** N3D display buffer address
	     ** 非3D
	     **/
		if (info->var.bits_per_pixel >= 8)
		{
		    MTFB_LINE;
			u32TmpAddr =    info->fix.smem_start
				          + info->fix.line_length * info->var.yoffset
		                  + info->var.xoffset * (info->var.bits_per_pixel >> 3);
		}
		else
		{
		    MTFB_LINE;
			u32TmpAddr = (   info->fix.smem_start
				           + info->fix.line_length * info->var.yoffset
		                   + info->var.xoffset * info->var.bits_per_pixel / 8);
		}

	    if((info->var.bits_per_pixel == 24)&&((info->var.xoffset != 0)||(info->var.yoffset != 0)))
	    {
	        mt_u32 TmpData;

            MTFB_LINE;

	        TmpData = (   info->fix.smem_start
				        + info->fix.line_length * info->var.yoffset
	                    + info->var.xoffset * (info->var.bits_per_pixel >> 3))/16/3;
	        u32TmpAddr = TmpData * 16 * 3;
	    }

       MTFB_LINE;

		/**config N3D display buffer that we blit to 3D buffer*/
		memset(&stDispBuf, 0x0, sizeof(stDispBuf));
		stDispBuf.stCanvas.enFmt      = par->stExtendInfo.enColFmt;
		stDispBuf.stCanvas.u32Pitch   = info->fix.line_length;
		stDispBuf.stCanvas.u32PhyAddr = u32TmpAddr;
		stDispBuf.stCanvas.u32Width   = info->var.xres;
		stDispBuf.stCanvas.u32Height  = info->var.yres;

		stDispBuf.UpdateRect.x = 0;
		stDispBuf.UpdateRect.y = 0;
		stDispBuf.UpdateRect.w = info->var.xres;
		stDispBuf.UpdateRect.h = info->var.yres;
		/**end*/

		/**config N3D display buffer that we blit to 3D buffer*/
		memcpy(&par->st3DInfo.st3DSurface, &stDispBuf.stCanvas, sizeof(MTFB_SURFACE_S));
		par->st3DInfo.st3DSurface.u32Pitch = u32Stride;

       MTFB_LINE;

		s32Ret = mtfb_checkandalloc_3dmem(par->stBaseInfo.u32LayerID, u32BufferSize);
		if (MT_SUCCESS != s32Ret)
		{
		    MTFB_LINE;
			MTFB_DEBUGK("fail to alloc 3d memory, pandisplay failure.\n ");
			return s32Ret;
		}

       if(   MTFB_LAYER_BUF_STANDARD == par->stExtendInfo.enBufMode
		  && MTFB_STEREO_FRMPACKING  == par->st3DInfo.enOutStereoMode)
        {
            par->stRunInfo.bModifying          = MT_TRUE;
    		par->stRunInfo.u32ScreenAddr       = u32TmpAddr;
    		par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
            par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
            par->stRunInfo.bModifying          = MT_FALSE;

            MTFB_LINE;
            return MT_SUCCESS;
        }
		par->st3DInfo.st3DSurface.u32PhyAddr = par->st3DInfo.u32DisplayAddr[par->stRunInfo.u32IndexForInt];
		/**end*/

		memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));

		stBlitOpt.bScale = MT_TRUE;
		stBlitOpt.bRegionDeflicker = MT_TRUE;
		if (par->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
		{
		    MTFB_LINE;
			stBlitOpt.enAntiflickerLevel = par->stBaseInfo.enAntiflickerLevel;
		}

       MTFB_LINE;

		stBlitOpt.bCallBack   = MT_TRUE;
		stBlitOpt.pfnCallBack = mtfb_tde_callback; /** 这里面会更新内存计数**/
		stBlitOpt.pParam      = &(par->stBaseInfo.u32LayerID);

		/*blit display buffer to 3D buffer*/
        s32Ret = mtfb_3DData_Config(par->stBaseInfo.u32LayerID, &stDispBuf, &stBlitOpt);
		if (MT_SUCCESS != s32Ret)
		{
			MTFB_DEBUGK("pandisplay config stereo data failure!");
           MTFB_LINE;
			return MT_FAILURE;
		}
    }
    else//mono mode
 #endif
    {
        /**
         ** set the stride and display start address
         **/

        MTFB_LINE;

        if (var->bits_per_pixel >= 8)
        {
             MTFB_LINE;
            u32DisplayAddr = (u32StartAddr + info->fix.line_length * var->yoffset
                           + var->xoffset * (var->bits_per_pixel >> 3))&0xfffffff0;
        }
        else
        {
            MTFB_LINE;
            u32DisplayAddr = (u32StartAddr + info->fix.line_length * var->yoffset
                           + var->xoffset * var->bits_per_pixel / 8) & 0xfffffff0;
        }

        if((info->var.bits_per_pixel == 24)&&((info->var.xoffset !=0)||(info->var.yoffset !=0)))
        {
            mt_u32 TmpData;
            MTFB_LINE;
            TmpData = (u32StartAddr + info->fix.line_length * var->yoffset
                           + var->xoffset * (var->bits_per_pixel >> 3))/16/3;
            u32DisplayAddr = TmpData * 16 * 3;

        }

        par->stRunInfo.bModifying          = MT_TRUE;
		 par->stRunInfo.u32ScreenAddr       = u32DisplayAddr;
		 par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
        par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
        par->stRunInfo.bModifying          = MT_FALSE;

        MTFB_LINE;

        /*if the flag "FB_ACTIVATE_VBL" has been set, we should wait forregister update finish*/
        if ((var->activate & FB_ACTIVATE_VBL) && par->bVblank)
        {
            MTFB_LINE;
            mtfb_wait_regconfig_work(par->stBaseInfo.u32LayerID);
        }
    }

    MTFB_LINE;

#ifdef CFG_MTFB_LOGO_SUPPORT
	mtfb_clear_logo(par->stBaseInfo.u32LayerID, MT_FALSE);
#endif

 	if (!par->bPanFlag)
 	{
 	    MTFB_LINE;
 		par->bPanFlag = MT_TRUE;
 	}

    MTFB_LINE;
    par->bHwcRefresh = MT_FALSE;

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}



/***************************************************************************************
* func          : mtfb_checkandalloc_3dmem
* description   : CNcomment: 判断3D内存是否足够，这个接口在set par 和 0buffer
                             刷新以及pan display的时候会被调用   CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
static mt_s32 mtfb_checkandalloc_3dmem(MTFB_LAYER_ID_E enLayerId, mt_u32 u32BufferSize)
{

	mt_s32 s32Ret;
    MTFB_PAR_S *pstPar;
	struct fb_info *info;

	info = s_stLayer[enLayerId].pstInfo;
    pstPar = (MTFB_PAR_S *)info->par;

	MTFB_FUN_IN;

	if(MTFB_STEREO_MONO == pstPar->st3DInfo.enOutStereoMode)
	{
	   /** 非3D **/
       MTFB_FUN_OUT;
		return MT_SUCCESS;
	}

	if(   pstPar->stExtendInfo.enBufMode != MTFB_LAYER_BUF_NONE
	   && pstPar->stExtendInfo.enBufMode != MTFB_LAYER_BUF_STANDARD)
	{
	   MTFB_FUN_OUT;
		return MT_SUCCESS;
	}
    if(    MTFB_LAYER_BUF_STANDARD == pstPar->stExtendInfo.enBufMode
		&& MTFB_STEREO_FRMPACKING  == pstPar->st3DInfo.enOutStereoMode)
    {
        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    mutex_lock(&pstPar->st3DInfo.st3DMemInfo.stStereoMemLock);

	/**
	 ** 1: allocate 3D buffer
	 ** 初始化pstPar->st3DInfo.st3DMemInfo.u32StereoMemLen为0，调这里mtfb_allocstereobuf
	 ** 就赋值了
	 **/
	if (u32BufferSize > pstPar->st3DInfo.st3DMemInfo.u32StereoMemLen)
	{
		s32Ret = mtfb_allocstereobuf(info, u32BufferSize);
        if (s32Ret != MT_SUCCESS)
        {
            MTFB_DEBUGK("alloc 3D buffer failure!, expect mem size: %d\n", u32BufferSize);
            mutex_unlock(&pstPar->st3DInfo.st3DMemInfo.stStereoMemLock);
            MTFB_FUN_OUT;
            return s32Ret;
        }
		/**
		 **左眼和右眼地址
		 **/
		pstPar->st3DInfo.st3DSurface.u32PhyAddr = pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart;
		pstPar->st3DInfo.u32rightEyeAddr        = pstPar->st3DInfo.st3DSurface.u32PhyAddr;
        /** 使用哪块buffer **/
		pstPar->stRunInfo.u32IndexForInt        = 0;

		mtfb_clearstereobuf  (info);
		mtfb_assign_dispbuf(pstPar->stBaseInfo.u32LayerID);
	}

    mutex_unlock(&pstPar->st3DInfo.st3DMemInfo.stStereoMemLock);

   MTFB_FUN_OUT;

	return MT_SUCCESS;

}
#endif


/***************************************************************************************
* func          : mtfb_tde_callback
* description   : CNcomment:  CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static void mtfb_tde_callback(mt_void *pParaml, mt_void *pParamr)
{

    mt_u32 u32LayerId = *(mt_u32 *)pParaml;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)(s_stLayer[u32LayerId].pstInfo->par);

    MTFB_FUN_IN;

#ifdef CFG_MTFB_FENCE_SUPPORT
    #ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    	if(pstPar->bSetStereoMode)
    	{
    	   /** 切换地址 **/
    	    mt_u32 u32Index;
    	    u32Index = pstPar->stRunInfo.u32IndexForInt;
           MTFB_LINE;
    		s_stDrvOps.MTFB_DRV_SetLayerAddr(u32LayerId, pstPar->st3DInfo.u32DisplayAddr[u32Index]);
    		pstPar->stRunInfo.u32ScreenAddr  = pstPar->st3DInfo.u32DisplayAddr[u32Index];
    		pstPar->st3DInfo.u32rightEyeAddr = pstPar->stRunInfo.u32ScreenAddr;
    		s_stDrvOps.MTFB_DRV_SetTriDimAddr(u32LayerId, pstPar->st3DInfo.u32rightEyeAddr);
            s_stDrvOps.MTFB_DRV_UpdataLayerReg(pstPar->stBaseInfo.u32LayerID);
            pstPar->stRunInfo.u32IndexForInt = (++u32Index) % pstPar->stRunInfo.u32BufNum;

    	}
    #endif
#else
    {
        MTFB_LINE;
        pstPar->stRunInfo.bNeedFlip = MT_TRUE;
    }
#endif
    MTFB_FUN_OUT;
}

static mt_void mtfb_disp_setlayerpos(mt_u32 u32LayerId, mt_s32 s32XPos, mt_s32 s32YPos)
{
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

	pstPar->stRunInfo.bModifying          = MT_TRUE;

	pstPar->stExtendInfo.stPos.s32XPos = s32XPos;
    pstPar->stExtendInfo.stPos.s32YPos = s32YPos;

    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_INRECT;
	 pstPar->stRunInfo.bModifying          = MT_FALSE;

    MTFB_FUN_OUT;
    return;
}

static mt_void mtfb_buf_setbufmode(mt_u32 u32LayerId, MTFB_LAYER_BUF_E enLayerBufMode)
{
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    /* in 0 buf mode ,maybe the stride or fmt will be changed! */
    if ((pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
        && (pstPar->stExtendInfo.enBufMode != enLayerBufMode))
    {
        pstPar->stRunInfo.bModifying = MT_TRUE;

        pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;

        pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_FMT;

        pstPar->stRunInfo.bModifying = MT_FALSE;
    }

    pstPar->stExtendInfo.enBufMode = enLayerBufMode;

    MTFB_FUN_OUT;
}


/***************************************************************************************
* func          : mtfb_disp_setdispsize
* description   : CNcomment: choose the module to do  flicker resiting,
                             TDE or VOU ? the rule is as tmts ,the moudle
                             should do flicker resisting who has do scaling CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_select_antiflicker_mode(MTFB_PAR_S *pstPar)
{
	MTFB_RECT stOutputRect;

    MTFB_FUN_IN;
    /**
     ** if the usr's configuration is no needed to do flicker resisting,
     ** so no needed to do it
     **/
   if (pstPar->stBaseInfo.enAntiflickerLevel == MTFB_LAYER_ANTIFLICKER_NONE)
   {
       pstPar->stBaseInfo.enAntiflickerMode = MTFB_ANTIFLICKER_NONE;
   }
   else
   {
       /**
        ** current standard no needed to do flicker resisting
        **/
       if (!pstPar->stBaseInfo.bNeedAntiflicker)
       {
           pstPar->stBaseInfo.enAntiflickerMode = MTFB_ANTIFLICKER_NONE;
       }
       else
       {
       		s_stDrvOps.MTFB_DRV_GetLayerOutRect(pstPar->stBaseInfo.u32LayerID, &stOutputRect);
           /**
            ** VO has don scaling , so should do flicker resisting at the same time
            **/
           if ( (pstPar->stExtendInfo.u32DisplayWidth  != stOutputRect.w)
             || (pstPar->stExtendInfo.u32DisplayHeight != stOutputRect.h))
           {
               pstPar->stBaseInfo.enAntiflickerMode = MTFB_ANTIFLICKER_VO;
           }
           else
           {
               pstPar->stBaseInfo.enAntiflickerMode = MTFB_ANTIFLICKER_TDE;
           }
       }
   }

   MTFB_FUN_OUT;
}


/***************************************************************************************
* func          : mtfb_disp_setantiflickerlevel
* description   : CNcomment: 设置抗闪级别 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_disp_setantiflickerlevel(mt_u32 u32LayerId, MTFB_LAYER_ANTIFLICKER_LEVEL_E enAntiflickerLevel)
{
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
    pstPar->stBaseInfo.enAntiflickerLevel = enAntiflickerLevel;
    mtfb_select_antiflicker_mode(pstPar);

    MTFB_FUN_OUT;
    return;
}

#define MTFB_CHECK_LAYERID(u32LayerId) do\
{\
    if (!g_pstCap[u32LayerId].bLayerSupported)\
    {\
        MTFB_DEBUGK("not support layer %d\n", u32LayerId);\
        return MT_FAILURE;\
    }\
}while(0);

#ifdef CFG_MTFB_CURSOR_SUPPORT
#define MTFB_CHECK_CURSOR_LAYERID(u32LayerId) do\
{\
 if (u32LayerId != MTFB_LAYER_CURSOR)\
    {\
        MTFB_DEBUGK("layer %d is not cursor layer!\n", u32LayerId);\
    	return MT_FAILURE;\
    }\
}while(0)
#endif


/***************************************************************************************
* func          : mtfb_flip_screenaddr
* description   : CNcomment: 刷新屏幕地址 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_flip_screenaddr(mt_u32 u32LayerId)
{

	mt_u32 u32Index;
    struct fb_info *info;
    MTFB_PAR_S *pstPar;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)(info->par);

    u32Index = pstPar->stRunInfo.u32IndexForInt;

    MTFB_FUN_IN;

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	if (pstPar->bSetStereoMode)
	{
		s_stDrvOps.MTFB_DRV_SetLayerAddr(u32LayerId, pstPar->st3DInfo.u32DisplayAddr[u32Index]);
		pstPar->stRunInfo.u32ScreenAddr  = pstPar->st3DInfo.u32DisplayAddr[u32Index];
		pstPar->st3DInfo.u32rightEyeAddr = pstPar->stRunInfo.u32ScreenAddr;
		s_stDrvOps.MTFB_DRV_SetTriDimAddr(u32LayerId, pstPar->st3DInfo.u32rightEyeAddr);
	}
	else
#endif
	{
		s_stDrvOps.MTFB_DRV_SetLayerAddr(u32LayerId, pstPar->stDispInfo.u32DisplayAddr[u32Index]);
		pstPar->stRunInfo.u32ScreenAddr  = pstPar->stDispInfo.u32DisplayAddr[u32Index];
	}

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
	if (s_stDrvOps.MTFB_DRV_GetCmpSwitch(u32LayerId))
	{
		memcpy(&(pstPar->stDispInfo.stCmpRect), &pstPar->stDispInfo.stUpdateRect, sizeof(MTFB_RECT));
	}
#endif

	/**
	 **bufer 1 and buffer 2在不停的交换
	 **/
    pstPar->stRunInfo.u32IndexForInt = (++u32Index) % pstPar->stRunInfo.u32BufNum;
    pstPar->stRunInfo.bFliped   = MT_TRUE;
    pstPar->stRunInfo.bNeedFlip = MT_FALSE;

    MTFB_FUN_OUT;
	return MT_SUCCESS;

}


/***************************************************************************************
* func          : mtfb_frame_end_callback
* description   : CNcomment: 帧结束call back CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_FENCE_SUPPORT
static mt_void mtfb_frame_end_callback(mt_void)
{
   MTFB_FUN_IN;
//   printk("\r\n ~~~~~~~~~~~~%s", __FUNCTION__);
	while(atomic_read(&s_SyncInfo.s32RefreshCnt) > 0)
    {
    	/** 更新一帧数据 **/
		atomic_dec(&s_SyncInfo.s32RefreshCnt);
        if(s_SyncInfo.pstTimeline)
        {/** 时间轴不为空 **/
		   /**
			** timeline value 加1，这里刷几帧就创建多少个fence，timeline就要加多少个
			**/
		    sw_sync_timeline_inc(s_SyncInfo.pstTimeline, 1);
            s_SyncInfo.u32Timeline++;
        }
	}
	/** 帧结束中断唤醒，可以刷新了 **/
    s_SyncInfo.FrameEndFlag = 1;
    wake_up_interruptible(&s_SyncInfo.FrameEndEvent);

    MTFB_FUN_OUT;
    return;

}
#endif


/***************************************************************************************
* func          : mtfb_vo_callback
* description   : CNcomment: vo中断处理 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static void mtfb_vo_callback(mt_void *pParaml, mt_void *pParamr)
{
    mt_u32 *pu32LayerId = (mt_u32 *)pParaml;
    struct fb_info *info = s_stLayer[*pu32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)(info->par);
    struct timespec64 tv;
    mt_u32 u32NowTimeMs;
	mt_u32 pixStride = 0;

    //MTFB_FUN_IN;

    /**
     ** 帧率统计
     **/
	  ktime_get_real_ts64(&tv);
	  u32NowTimeMs = (((mt_u32)tv.tv_sec)*1000+((mt_u32)tv.tv_nsec)/1000000);  
    if ((u32NowTimeMs - pstPar->stFrameInfo.u32StartTimeMs) >= 1000)
    {/** 刷新帧率 **/
        pstPar->stFrameInfo.u32StartTimeMs = u32NowTimeMs;
        pstPar->stFrameInfo.u32Fps =  pstPar->stFrameInfo.u32RefreshFrame;
        pstPar->stFrameInfo.u32RefreshFrame = 0;

        //MTFB_LINE;
    }

    if (!pstPar->stRunInfo.bModifying)
    {
       //MTFB_DEBUGK("%s %d: [0x%x][0x%x]\n", __FUNCTION__, __LINE__, pstPar->stRunInfo.u32ParamModifyMask, *pu32LayerId);

    	if(pstPar->stRunInfo.u32ParamModifyMask)
    	{/** 有发生变化就更新寄存器 **/
    		s_stDrvOps.MTFB_DRV_UpdataLayerReg(*pu32LayerId);
    	}
        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_SHOW)
        {
            s_stDrvOps.MTFB_DRV_EnableLayer(*pu32LayerId, pstPar->stExtendInfo.bShow);
            pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_SHOW;
        }
        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_ALPHA)
        {
            s_stDrvOps.MTFB_DRV_SetLayerAlpha(*pu32LayerId, &pstPar->stExtendInfo.stAlpha);
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_ALPHA;
        }
        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_COLORKEY)
        {
            s_stDrvOps.MTFB_DRV_SetLayerKeyMask(*pu32LayerId, &pstPar->stExtendInfo.stCkey);
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_COLORKEY;
        }
        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_BMUL)
        {
            s_stDrvOps.MTFB_DRV_SetLayerPreMult(*pu32LayerId, pstPar->stBaseInfo.bPreMul);
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_BMUL;
        }
        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_ANTIFLICKERLEVEL)
        {
        	/**
        	 **水平抗闪系数和级别
        	 **垂直抗闪系数和级别
        	 **底层实现为空
        	 **/
            MTFB_DEFLICKER_S stDeflicker;

            stDeflicker.pu8HDfCoef  = pstPar->stBaseInfo.ucHDfcoef;
            stDeflicker.pu8VDfCoef  = pstPar->stBaseInfo.ucVDfcoef;
            stDeflicker.u32HDfLevel = pstPar->stBaseInfo.u32HDflevel;
            stDeflicker.u32VDfLevel = pstPar->stBaseInfo.u32VDflevel;

            s_stDrvOps.MTFB_DRV_SetLayerDeFlicker(*pu32LayerId, &stDeflicker);
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_ANTIFLICKERLEVEL;
        }
        if (   pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_INRECT
			|| pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_OUTRECT)
        {
            MTFB_RECT stInRect   = {0};

        	  stInRect.x = pstPar->stExtendInfo.stPos.s32XPos;
            stInRect.y = pstPar->stExtendInfo.stPos.s32YPos;

            stInRect.w = (mt_s32)pstPar->stExtendInfo.u32DisplayWidth;
            stInRect.h = (mt_s32)pstPar->stExtendInfo.u32DisplayHeight;
            //MTFB_LINE;
           s_stDrvOps.MTFB_DRV_SetLayerInRect(*pu32LayerId, &stInRect);
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_INRECT;
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_OUTRECT;
		}
		/**
		 ** color format,stride,display address take effect only when user refresmtng
		 ** 像素格式，行间距，显示地址只有在刷新的时候生效，否则中断更新的时候会显示异常
		 ** 上面的东西第一次设置后面几乎不会变
		 **/
		if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_REFRESH)
		{
		   //MTFB_LINE;
			if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_FMT)
	        {
	            //MTFB_DEBUGK("%s %d: 0x%x\n", __FUNCTION__, __LINE__, pstPar->stRunInfo.u32ParamModifyMask);

	            if (  (pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
	                && pstPar->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr)
	            {/** 单buffer,使用canvas信息 **/
	                s_stDrvOps.MTFB_DRV_SetLayerDataFmt(*pu32LayerId, pstPar->stDispInfo.stUserBuffer.stCanvas.enFmt);
	            }
	            else
	            {/** 非单buffer **/
	                s_stDrvOps.MTFB_DRV_SetLayerDataFmt(*pu32LayerId, pstPar->stExtendInfo.enColFmt);
	            }
				pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_FMT;
	        }
	        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_STRIDE)
	        {
	           //MTFB_DEBUGK("%s %d: 0x%x\n", __FUNCTION__, __LINE__, pstPar->stRunInfo.u32ParamModifyMask);
				#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
		        	if (//(IS_STEREO_SBS(pstPar) || IS_STEREO_TAB(pstPar)))
		        		pstPar->bSetStereoMode)
		        	{
		        	      pixStride = calc_pixel_stride_by_pitch_fmt(pstPar->st3DInfo.st3DSurface.u32Pitch, pstPar->st3DInfo.st3DSurface.enFmt, __LINE__);
		                //s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, pstPar->st3DInfo.st3DSurface.u32Pitch);
		                s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, pixStride);
		        	}
					else
				#endif
				{
				   //MTFB_LINE;
					if ((pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
	                	&& pstPar->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr)
		            {
		                pixStride = calc_pixel_stride_by_pitch_fmt(pstPar->stDispInfo.stUserBuffer.stCanvas.u32Pitch, pstPar->stDispInfo.stUserBuffer.stCanvas.enFmt, __LINE__);
		                //s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, pstPar->stDispInfo.stUserBuffer.stCanvas.u32Pitch);
		                s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, pixStride);
		            }
		            else
		            {

                       pixStride = calc_pixel_stride_by_pitch_fmt(info->fix.line_length, pstPar->stExtendInfo.enColFmt, __LINE__);
						  //s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, info->fix.line_length);
                       s_stDrvOps.MTFB_DRV_SetLayerStride(*pu32LayerId, pixStride);
		            }
				}

				pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_STRIDE;
              //MTFB_LINE;
	        }

	        if (pstPar->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_DISPLAYADDR)
	        {
	            //MTFB_DEBUGK("%s %d: 0x%x\n", __FUNCTION__, __LINE__, pstPar->stRunInfo.u32ParamModifyMask);
                pstPar->stFrameInfo.u32RefreshFrame++;
	            s_stDrvOps.MTFB_DRV_SetLayerAddr(*pu32LayerId, pstPar->stRunInfo.u32ScreenAddr);
				pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_DISPLAYADDR;

				#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
				/**
	 			 **图形层3D 格式时右眼数据存放地址寄存器
	 			 **/
				if (pstPar->bSetStereoMode)
				{
				   //MTFB_LINE;
					pstPar->st3DInfo.u32rightEyeAddr = pstPar->stRunInfo.u32ScreenAddr;
					s_stDrvOps.MTFB_DRV_SetTriDimAddr(*pu32LayerId, pstPar->st3DInfo.u32rightEyeAddr);
				}
         		#endif

	        }
		 	#ifdef CFG_MTFB_COMPRESSION_SUPPORT
			if (s_stDrvOps.MTFB_DRV_GetCmpSwitch(*pu32LayerId))
			{
			   //MTFB_LINE;

				if (   pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD
					|| pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
				{
					pstPar->stDispInfo.stCmpRect.x = 0;
					pstPar->stDispInfo.stCmpRect.y = 0;
					pstPar->stDispInfo.stCmpRect.w = pstPar->stExtendInfo.u32DisplayWidth;
					pstPar->stDispInfo.stCmpRect.h = pstPar->stExtendInfo.u32DisplayHeight;
				}
			}
		    #endif
			pstPar->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_REFRESH;
          //MTFB_LINE;
		}

        //MTFB_LINE;
    }

    //MTFB_LINE;

    if(  (pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_DOUBLE)
       &&(pstPar->stRunInfo.bNeedFlip == MT_TRUE))
    {
       //MTFB_LINE;
       /** 双buffer模式，需要刷新 **/
		mtfb_flip_screenaddr(*pu32LayerId);
		/**
		 **更新寄存器
		 **/
		s_stDrvOps.MTFB_DRV_UpdataLayerReg(*pu32LayerId);
    }
    else if (pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_ONE)
    {
        //MTFB_LINE;
        /** 单buffer模式，通知WBC回写pGfxGp->unUpFlag.bits.RegUp **/
        s_stDrvOps.MTFB_DRV_UpdataLayerReg(*pu32LayerId);
    }


#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    else if ((pstPar->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD)
        	&& pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart
        	&& pstPar->bSetStereoMode
        	&& (pstPar->stRunInfo.bNeedFlip == MT_TRUE))
    {
       //MTFB_LINE;
		mtfb_flip_screenaddr(*pu32LayerId);
		s_stDrvOps.MTFB_DRV_UpdataLayerReg(*pu32LayerId);
    }
#endif


#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
    //MTFB_LINE;
	mtfb_scrolltext_blit(*pu32LayerId);
#endif

    //MTFB_FUN_OUT;
}


/***************************************************************************************
* func          : mtfb_refresh_0buf
* description   : CNcomment: no display buffer refresh CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_0buf(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{

	mt_u32 u32StartAddr;
	MTFB_PAR_S *pstPar;
    struct fb_info *info;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)info->par;

   MTFB_FUN_IN;

	/**
	 **显示起始地址为canvas 地址
	 **/
	u32StartAddr = pstCanvasBuf->stCanvas.u32PhyAddr;

	/**
	 ** when you change para, the register not be change
	 **/
    pstPar->stRunInfo.bModifying = MT_TRUE;
	pstPar->stRunInfo.u32ScreenAddr       = u32StartAddr;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;

	pstPar->stDispInfo.stUserBuffer.stCanvas.u32Pitch = pstCanvasBuf->stCanvas.u32Pitch;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;

	pstPar->stDispInfo.stUserBuffer.stCanvas.enFmt = pstCanvasBuf->stCanvas.enFmt;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_FMT;

    mtfb_disp_setdispsize(u32LayerId, pstCanvasBuf->stCanvas.u32Width, pstCanvasBuf->stCanvas.u32Height);

	memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;

	pstPar->stRunInfo.bModifying = MT_FALSE;

    mtfb_wait_regconfig_work(u32LayerId);

    MTFB_FUN_OUT;

    return MT_SUCCESS;

}

/***************************************************************************************
* func          : mtfb_refresh_1buf
* description   : CNcomment: one canvas buffer,one display buffer refresh CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_1buf(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
	mt_s32 s32Ret;
    MTFB_PAR_S *pstPar;
    MTFB_BUFFER_S stDisplayBuf;
	struct fb_info *info;
	MTFB_OSD_DATA_S stOsdData;
	MTFB_BLIT_OPT_S stBlitOpt;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
    memset(&stBlitOpt,    0, sizeof(MTFB_BLIT_OPT_S));
    memset(&stDisplayBuf, 0, sizeof(MTFB_BUFFER_S));

    stDisplayBuf.stCanvas.enFmt      = pstPar->stExtendInfo.enColFmt;
    stDisplayBuf.stCanvas.u32Height  = pstPar->stExtendInfo.u32DisplayHeight;
    stDisplayBuf.stCanvas.u32Width   = pstPar->stExtendInfo.u32DisplayWidth;
    stDisplayBuf.stCanvas.u32Pitch   = info->fix.line_length;
    stDisplayBuf.stCanvas.u32PhyAddr = pstPar->stDispInfo.u32DisplayAddr[0];

    s_stDrvOps.MTFB_DRV_GetOSDData(u32LayerId, &stOsdData);

    /**
     ** if display address is not the same as inital address,
     ** please config it use old address
     **/
    if (   stOsdData.u32RegPhyAddr != pstPar->stDispInfo.u32DisplayAddr[0]
		&& pstPar->stDispInfo.u32DisplayAddr[0])
    {
        pstPar->stRunInfo.bModifying = MT_TRUE;
        pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
		pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
        pstPar->stRunInfo.u32ScreenAddr       = pstPar->stDispInfo.u32DisplayAddr[0];
        memset(info->screen_base, 0x00, info->fix.smem_len);
        pstPar->stRunInfo.bModifying = MT_FALSE;
    }

    if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
    {
        stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
    }

    if (   pstCanvasBuf->stCanvas.u32Height != pstPar->stExtendInfo.u32DisplayHeight
        || pstCanvasBuf->stCanvas.u32Width != pstPar->stExtendInfo.u32DisplayWidth)
    {
        stBlitOpt.bScale          = MT_TRUE;
		/** 只做为TDE内部参数检查使用**/
        stDisplayBuf.UpdateRect.x = 0;
        stDisplayBuf.UpdateRect.y = 0;
        stDisplayBuf.UpdateRect.w = stDisplayBuf.stCanvas.u32Width;
        stDisplayBuf.UpdateRect.h = stDisplayBuf.stCanvas.u32Height;
    }
    else
    {
        stDisplayBuf.UpdateRect = pstCanvasBuf->UpdateRect;
    }

    stBlitOpt.bRegionDeflicker = MT_TRUE;

    s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(pstCanvasBuf, &stDisplayBuf, &stBlitOpt, MT_TRUE);
    if (s32Ret <= 0)
    {
        MTFB_DEBUGK("mtfb_refresh_1buf blit err 5!\n");
        return MT_FAILURE;
    }

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
	memcpy(&(pstPar->stDispInfo.stCmpRect), &stDisplayBuf.UpdateRect, sizeof(MTFB_RECT));
#endif

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}

/***************************************************************************************
* func          : mtfb_refresh_2buf
* description   : CNcomment: 异步刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_2buf(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
	mt_s32 s32Ret;
	mt_u32 u32Index;
	MTFB_PAR_S *pstPar;
    struct fb_info *info;
	unsigned long lockflag;
    MTFB_BUFFER_S stForeBuf;
    MTFB_BUFFER_S stBackBuf;
	MTFB_BLIT_OPT_S stBlitOpt;
    MTFB_OSD_DATA_S stOsdData;

	s32Ret   = 0;
	info     = s_stLayer[u32LayerId].pstInfo;
	pstPar   = (MTFB_PAR_S *)info->par;
	u32Index = pstPar->stRunInfo.u32IndexForInt;

    MTFB_FUN_IN;

    memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
    memset(&stForeBuf, 0, sizeof(MTFB_BUFFER_S));
    memset(&stBackBuf, 0, sizeof(MTFB_BUFFER_S));

    stBlitOpt.bCallBack = MT_TRUE;
	stBlitOpt.pfnCallBack = mtfb_tde_callback;
    stBlitOpt.pParam = &(pstPar->stBaseInfo.u32LayerID);

    spin_lock_irqsave(&pstPar->stBaseInfo.lock,lockflag);
    pstPar->stRunInfo.bNeedFlip = MT_FALSE;
    pstPar->stRunInfo.s32RefreshHandle = 0;
    spin_unlock_irqrestore(&pstPar->stBaseInfo.lock,lockflag);

    s_stDrvOps.MTFB_DRV_GetOSDData(u32LayerId, &stOsdData);

    stBackBuf.stCanvas.enFmt      = pstPar->stExtendInfo.enColFmt;
	stBackBuf.stCanvas.u32Width   = pstPar->stExtendInfo.u32DisplayWidth;
    stBackBuf.stCanvas.u32Height  = pstPar->stExtendInfo.u32DisplayHeight;
    stBackBuf.stCanvas.u32Pitch   = info->fix.line_length;
    stBackBuf.stCanvas.u32PhyAddr = pstPar->stDispInfo.u32DisplayAddr[u32Index];

    /**
     ** according to the hw arithemetic, calculate source and Dst fresh rectangle
     **/
    if (  (pstCanvasBuf->stCanvas.u32Height != pstPar->stExtendInfo.u32DisplayHeight)
        ||(pstCanvasBuf->stCanvas.u32Width  != pstPar->stExtendInfo.u32DisplayWidth))
    {

        stBlitOpt.bScale = MT_TRUE;
    }

	mtfb_getupdate_rect(u32LayerId, pstCanvasBuf, &stBackBuf.UpdateRect);


    /**
     ** We should check is address changed, for make sure that the address
     ** configed to the hw reigster is in effect
     **/
    if (   (pstPar->stRunInfo.bFliped)
		&& (stOsdData.u32RegPhyAddr == pstPar->stDispInfo.u32DisplayAddr[1-u32Index]))
    {
    	/**
    	 ** when fill background buffer, we need to backup fore buffer first
    	 **/
		mtfb_backup_forebuf(u32LayerId, &stBackBuf);
        /**
         ** clear union rect
         **/
        memset(&(pstPar->stDispInfo.stUpdateRect), 0, sizeof(MTFB_RECT));
        pstPar->stRunInfo.bFliped = MT_FALSE;
    }

    /* update union rect */
    if ((pstPar->stDispInfo.stUpdateRect.w == 0) || (pstPar->stDispInfo.stUpdateRect.h == 0))
    {
        memcpy(&pstPar->stDispInfo.stUpdateRect, &stBackBuf.UpdateRect, sizeof(MTFB_RECT));
    }
    else
    {
        MTFB_UNITE_RECT(pstPar->stDispInfo.stUpdateRect, stBackBuf.UpdateRect);
    }

    if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
    {
        stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
    }

    if (stBlitOpt.bScale == MT_TRUE)
    {
        /*actual area, calculate by TDE, here is just use for let pass the test */
        stBackBuf.UpdateRect.x = 0;
        stBackBuf.UpdateRect.y = 0;
        stBackBuf.UpdateRect.w = stBackBuf.stCanvas.u32Width;
        stBackBuf.UpdateRect.h = stBackBuf.stCanvas.u32Height;
    }
    else
    {
        stBackBuf.UpdateRect = pstCanvasBuf->UpdateRect;
    }

    stBlitOpt.bRegionDeflicker = MT_TRUE;
    /**
     ** blit with refresh rect
     **/
    s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(pstCanvasBuf, &stBackBuf,&stBlitOpt, MT_TRUE);
    if (s32Ret <= 0)
    {
        MTFB_DEBUGK("2buf blit err7!\n");
        goto RET;
    }

    pstPar->stRunInfo.s32RefreshHandle = s32Ret;

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

RET:

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}
/***************************************************************************************
* func          : mtfb_refresh_2buf_immediate_display
* description   : CNcomment: 同步刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_2buf_immediate_display(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{

	mt_s32 s32Ret    = 0;
	mt_u32 u32Index  = 0;
	MTFB_PAR_S *pstPar = NULL;
	struct fb_info *info;
	unsigned long lockflag = 0;
	MTFB_BUFFER_S stForeBuf;
	MTFB_BUFFER_S stBackBuf;
	MTFB_BLIT_OPT_S stBlitOpt;
	MTFB_OSD_DATA_S stOsdData;

	s32Ret	 = 0;
	info	 = s_stLayer[u32LayerId].pstInfo;
	pstPar	 = (MTFB_PAR_S *)info->par;
	u32Index = pstPar->stRunInfo.u32IndexForInt;

   MTFB_FUN_IN;

	memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
	memset(&stForeBuf, 0, sizeof(MTFB_BUFFER_S));
	memset(&stBackBuf, 0, sizeof(MTFB_BUFFER_S));

	stBlitOpt.bCallBack = MT_FALSE;
	stBlitOpt.pParam = &(pstPar->stBaseInfo.u32LayerID);

	spin_lock_irqsave(&pstPar->stBaseInfo.lock,lockflag);
	pstPar->stRunInfo.bNeedFlip = MT_FALSE;
	pstPar->stRunInfo.s32RefreshHandle = 0;
	spin_unlock_irqrestore(&pstPar->stBaseInfo.lock,lockflag);

	s_stDrvOps.MTFB_DRV_GetOSDData(u32LayerId, &stOsdData);

	stBackBuf.stCanvas.enFmt	  = pstPar->stExtendInfo.enColFmt;
	stBackBuf.stCanvas.u32Width   = pstPar->stExtendInfo.u32DisplayWidth;
	stBackBuf.stCanvas.u32Height  = pstPar->stExtendInfo.u32DisplayHeight;
	stBackBuf.stCanvas.u32Pitch   = info->fix.line_length;
	stBackBuf.stCanvas.u32PhyAddr = pstPar->stDispInfo.u32DisplayAddr[u32Index];

	/* according to the hw arithemetic, calculate  source and Dst fresh rectangle */
	if (   (pstCanvasBuf->stCanvas.u32Height != pstPar->stExtendInfo.u32DisplayHeight)
		|| (pstCanvasBuf->stCanvas.u32Width  != pstPar->stExtendInfo.u32DisplayWidth))
	{
		stBlitOpt.bScale = MT_TRUE;
	}

	mtfb_getupdate_rect(u32LayerId, pstCanvasBuf, &stBackBuf.UpdateRect);

	/**
	 ** when fill background buffer, we need to backup fore buffer first
	 **/
	mtfb_backup_forebuf(u32LayerId, &stBackBuf);


	/**
	 ** update union rect
	 **/
	memcpy(&pstPar->stDispInfo.stUpdateRect, &stBackBuf.UpdateRect, sizeof(MTFB_RECT));

	if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
	{
		stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
	}

	if (stBlitOpt.bScale == MT_TRUE)
	{
		/**
		 ** actual area, calculate by TDE, here is just use for let pass the test
		 **/
		stBackBuf.UpdateRect.x = 0;
		stBackBuf.UpdateRect.y = 0;
		stBackBuf.UpdateRect.w = stBackBuf.stCanvas.u32Width;
		stBackBuf.UpdateRect.h = stBackBuf.stCanvas.u32Height;
	}
	else
	{
		stBackBuf.UpdateRect = pstCanvasBuf->UpdateRect;
	}

	stBlitOpt.bRegionDeflicker = MT_TRUE;
	stBlitOpt.bBlock           = MT_TRUE;
	/**
	 ** blit with refresh rect
	 **/
	s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(pstCanvasBuf, &stBackBuf,&stBlitOpt, MT_TRUE);
	if (s32Ret <= 0)
	{
		MTFB_DEBUGK("2buf blit err 0x%x!\n",s32Ret);
		goto RET;
	}

    /**
     **set the backup buffer to register and show it
     **/
    pstPar->stRunInfo.bModifying = MT_TRUE;
    pstPar->stRunInfo.u32ScreenAddr       = pstPar->stDispInfo.u32DisplayAddr[u32Index];
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
    pstPar->stRunInfo.bModifying = MT_FALSE;

	pstPar->stRunInfo.u32IndexForInt = 1 - u32Index;

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
	if (s_stDrvOps.MTFB_DRV_GetCmpSwitch(u32LayerId))
	{
		memcpy(&(pstPar->stDispInfo.stCmpRect), &pstPar->stDispInfo.stUpdateRect, sizeof(MTFB_RECT));
	}
#endif

    /**
     ** wait the address register's configuration take effect before return
     **/
    mtfb_wait_regconfig_work(u32LayerId);

RET:

   MTFB_FUN_OUT;

	return MT_SUCCESS;

}


/***************************************************************************************
* func          : mtfb_refresh_panbuf
* description   : CNcomment: 给android使用 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
static mt_s32 mtfb_refresh_panbuf(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
    mt_s32 s32Ret;
	mt_u32 u32Stride;
	mt_u32 u32TmpAddr;
	MTFB_RECT UpdateRect;
	MTFB_BLIT_OPT_S stBlitOpt;
    MTFB_BUFFER_S stCanvasBuf;
    MTFB_BUFFER_S stDisplayBuf;

	MTFB_PAR_S *par;
    struct fb_info *info;
    struct fb_var_screeninfo *var;

    MTFB_FUN_IN;

	info = s_stLayer[u32LayerId].pstInfo;
	par = (MTFB_PAR_S *)info->par;
	var = &(info->var);

	UpdateRect = pstCanvasBuf->UpdateRect;

    if ((UpdateRect.x >=  par->stExtendInfo.u32DisplayWidth)
        || (UpdateRect.y >= par->stExtendInfo.u32DisplayHeight)
        || (UpdateRect.w == 0) || (UpdateRect.h == 0))
    {
        MTFB_DEBUGK("mtfb_refresh_panbuf upate rect invalid\n");
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }

    if (par->bSetStereoMode)//(IS_STEREO_SBS(par) || IS_STEREO_TAB(par))
    {
        if (MT_NULL == par->st3DInfo.st3DMemInfo.u32StereoMemStart)
        {
            MTFB_DEBUGK("you should pan first\n");
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        u32Stride = par->st3DInfo.st3DSurface.u32Pitch;

        memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
        stBlitOpt.bScale = MT_TRUE;

        if (MTFB_ANTIFLICKER_TDE == par->stBaseInfo.enAntiflickerMode)
        {
            stBlitOpt.enAntiflickerLevel = par->stBaseInfo.enAntiflickerLevel;
        }

        stBlitOpt.bBlock = MT_TRUE;
        stBlitOpt.bRegionDeflicker = MT_TRUE;

        if (var->bits_per_pixel >= 8)
        {
            u32TmpAddr = info->fix.smem_start + info->fix.line_length * var->yoffset
                           + var->xoffset* (var->bits_per_pixel >> 3);
        }
        else
        {
            u32TmpAddr = (info->fix.smem_start + info->fix.line_length * var->yoffset
                           + var->xoffset * var->bits_per_pixel / 8);
        }

		if((var->bits_per_pixel == 24)&&((var->xoffset !=0)||(var->yoffset !=0)))
	    {
	        mt_u32 TmpData;

	        TmpData = (info->fix.smem_start + info->fix.line_length * var->yoffset
	                       + var->xoffset * (var->bits_per_pixel >> 3))/16/3;
	        u32TmpAddr = TmpData*16*3;
	    }

		/********************config pan buffer*******************/
        memset(&stCanvasBuf, 0, sizeof(MTFB_BUFFER_S));
        stCanvasBuf.stCanvas.enFmt      = par->stExtendInfo.enColFmt;
        stCanvasBuf.stCanvas.u32Pitch   = info->fix.line_length;
        stCanvasBuf.stCanvas.u32PhyAddr = u32TmpAddr;
		stCanvasBuf.stCanvas.u32Width   = par->stExtendInfo.u32DisplayWidth;
        stCanvasBuf.stCanvas.u32Height  = par->stExtendInfo.u32DisplayHeight;
        stCanvasBuf.UpdateRect          = UpdateRect;
		/***********************end**************************/

		/*******************config 3D buffer********************/
        memset(&stDisplayBuf, 0, sizeof(MTFB_BUFFER_S));
        stDisplayBuf.stCanvas.enFmt      = par->st3DInfo.st3DSurface.enFmt;
        stDisplayBuf.stCanvas.u32Pitch   = par->st3DInfo.st3DSurface.u32Pitch;
        stDisplayBuf.stCanvas.u32PhyAddr = par->stRunInfo.u32ScreenAddr;
        stDisplayBuf.stCanvas.u32Width   = par->st3DInfo.st3DSurface.u32Width;
        stDisplayBuf.stCanvas.u32Height  = par->st3DInfo.st3DSurface.u32Height;
		/***********************end**************************/

        if (MTFB_STEREO_SIDEBYSIDE_HALF == par->st3DInfo.enOutStereoMode)
        {
            stDisplayBuf.stCanvas.u32Width >>= 1;
        }
        else if (MTFB_STEREO_TOPANDBOTTOM == par->st3DInfo.enOutStereoMode)
        {
            stDisplayBuf.stCanvas.u32Height >>= 1;
        }

		stDisplayBuf.UpdateRect.x = 0;
		stDisplayBuf.UpdateRect.y = 0;
		stDisplayBuf.UpdateRect.w = stDisplayBuf.stCanvas.u32Width;
		stDisplayBuf.UpdateRect.h = stDisplayBuf.stCanvas.u32Height;


        s32Ret = s_stDrvTdeOps.MTFB_DRV_Blit(&stCanvasBuf, &stDisplayBuf, &stBlitOpt, MT_TRUE);
        if (s32Ret < 0)
        {
            MTFB_DEBUGK("stereo blit error!\n");
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

    }

    MTFB_FUN_OUT;
    return MT_SUCCESS;
}

/***************************************************************************************
* func          : mtfb_refresh_0buf_3D
* description   : CNcomment: 单buffer刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_0buf_3D(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{

	mt_s32 s32Ret;
	MTFB_PAR_S *pstPar;
	mt_u32 u32BufferSize;
    struct fb_info *info;
	MTFB_BLIT_OPT_S stBlitOpt;

   MTFB_FUN_IN;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)info->par;

	/**
	 ** config 3D surface par
	 **/
	pstPar->st3DInfo.st3DSurface.enFmt     = pstCanvasBuf->stCanvas.enFmt;
	pstPar->st3DInfo.st3DSurface.u32Pitch  = pstCanvasBuf->stCanvas.u32Pitch;
	pstPar->st3DInfo.st3DSurface.u32Width  = pstCanvasBuf->stCanvas.u32Width;
	pstPar->st3DInfo.st3DSurface.u32Height = pstCanvasBuf->stCanvas .u32Height;

	/**
	 ** allocate 3D memory,stride 16字节对齐
	 **/
	u32BufferSize = pstCanvasBuf->stCanvas.u32Height * ((pstCanvasBuf->stCanvas.u32Pitch + 0xf) & 0xfffffff0);

	/**
	 ** 分配3Dbuffer
	 **/
	s32Ret = mtfb_checkandalloc_3dmem(u32LayerId, u32BufferSize);
	if (MT_SUCCESS != s32Ret)
	{
		MTFB_DEBUGK("fail to alloc 3d memory, refresh failure.\n ");
       MTFB_FUN_OUT;
		return s32Ret;
	}

	/**
	 ** config 3D surface par，使用display buffer0
	 **/
	pstPar->st3DInfo.st3DSurface.u32PhyAddr= pstPar->st3DInfo.u32DisplayAddr[0];

	/**
	 ** config 3D buffer
	 **/
	memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
	stBlitOpt.bRegionDeflicker = MT_TRUE;
	stBlitOpt.bScale           = MT_TRUE;

	/**
	 **要是逐行处理就需要抗闪，使得行与行之间的像素差别不会那么大，防止
	 **那种条纹出现
	 **/
	if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
	{
		stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
	}

	mtfb_3DData_Config(u32LayerId, pstCanvasBuf, &stBlitOpt);

	/** 更新数据 **/
    pstPar->stRunInfo.bModifying = MT_TRUE;
    /** 更新显示地址 **/
	pstPar->stRunInfo.u32ScreenAddr       = pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
	/** 更新stride **/
	pstPar->stDispInfo.stUserBuffer.stCanvas.u32Pitch = pstCanvasBuf->stCanvas.u32Pitch;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;
	/** 更新像素格式 **/
	pstPar->stDispInfo.stUserBuffer.stCanvas.enFmt = pstCanvasBuf->stCanvas.enFmt;
    pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_FMT;

    mtfb_disp_setdispsize(u32LayerId,
		                    pstCanvasBuf->stCanvas.u32Width,
                            pstCanvasBuf->stCanvas.u32Height);

	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;

    pstPar->stRunInfo.bModifying = MT_FALSE;

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

    mtfb_wait_regconfig_work(u32LayerId);

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}
/***************************************************************************************
* func          : mtfb_refresh_1buf_3D
* description   : CNcomment: 双buffer刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_1buf_3D(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
	MTFB_PAR_S *pstPar;
	struct fb_info *info;
	MTFB_BLIT_OPT_S stBlitOpt;
	MTFB_OSD_DATA_S stOsdData;

	info   = s_stLayer[u32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
	/**
	 **1.获取上一帧显示的OSD数据
	 **/
	s_stDrvOps.MTFB_DRV_GetOSDData(u32LayerId, &stOsdData);

	/**2
	 ** if display address is not the same as inital address,
	 ** please config it use old address,如果显示的地址不是初始化的地址则切换显示地址
	 **/
	if( (stOsdData.u32RegPhyAddr != pstPar->stDispInfo.u32DisplayAddr[0]) &&
		(pstPar->stDispInfo.u32DisplayAddr[0]))
	{
		pstPar->stRunInfo.bModifying = MT_TRUE;
		pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
		pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
		pstPar->stRunInfo.u32ScreenAddr = pstPar->stDispInfo.u32DisplayAddr[0];
		memset(info->screen_base, 0x00, info->fix.smem_len);
		pstPar->stRunInfo.bModifying = MT_FALSE;
	}

	/**
	 ** no need to allocate 3D buffer, displaybuf[0] will be setted to be 3D buffer
	 **/
	pstPar->st3DInfo.st3DSurface.enFmt     = pstPar->stExtendInfo.enColFmt;
	pstPar->st3DInfo.st3DSurface.u32Pitch  = ((((pstPar->stExtendInfo.u32DisplayWidth * info->var.bits_per_pixel) >> 3) + 0xf) & 0xfffffff0);
	pstPar->st3DInfo.st3DSurface.u32Width  = pstPar->stExtendInfo.u32DisplayWidth;
	pstPar->st3DInfo.st3DSurface.u32Height = pstPar->stExtendInfo.u32DisplayHeight;
	pstPar->st3DInfo.st3DSurface.u32PhyAddr= pstPar->st3DInfo.u32DisplayAddr[0];

	memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));

	/**
	 **TDE内部做局部操作。会根据src srcrect dst自己计算dstrect,传入的dstrect无效
	 **/
	stBlitOpt.bRegionDeflicker = MT_TRUE;
	stBlitOpt.bScale           = MT_TRUE;
	if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
	{
		stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
	}

	mtfb_3DData_Config(u32LayerId, pstCanvasBuf, &stBlitOpt);

	/*backup usr buffer*/
	memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

   MTFB_FUN_OUT;

	return MT_SUCCESS;

}

/***************************************************************************************
* func          : mtfb_refresh_2buf_3D
* description   : CNcomment: 3 buffer刷新 异步，刷新不等更新完，允许丢帧 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh_2buf_3D(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
	mt_s32 s32Ret;
	mt_u32 u32Index;
	MTFB_PAR_S *pstPar;
    struct fb_info *info;
	unsigned long lockflag;
    MTFB_BUFFER_S stBackBuf;
	MTFB_BLIT_OPT_S stBlitOpt;
    MTFB_OSD_DATA_S stOsdData;

    MTFB_FUN_IN;
	s32Ret   = 0;
	info     = s_stLayer[u32LayerId].pstInfo;
	pstPar   = (MTFB_PAR_S *)info->par;
	u32Index = pstPar->stRunInfo.u32IndexForInt;

    memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
    memset(&stBackBuf, 0, sizeof(MTFB_BUFFER_S));

	/**
	 **TDE操作完之后回调TDE注册的回调函数mtfb_tde_callback
	 **/
    stBlitOpt.bCallBack = MT_TRUE;
	stBlitOpt.pfnCallBack = mtfb_tde_callback;
    stBlitOpt.pParam = &(pstPar->stBaseInfo.u32LayerID);

    spin_lock_irqsave(&pstPar->stBaseInfo.lock,lockflag);
    pstPar->stRunInfo.bNeedFlip        = MT_FALSE;
    pstPar->stRunInfo.s32RefreshHandle = 0;
    spin_unlock_irqrestore(&pstPar->stBaseInfo.lock,lockflag);

    s_stDrvOps.MTFB_DRV_GetOSDData(u32LayerId, &stOsdData);

	/**
	 ** no need to allocate 3D buffer, displaybuf[0] will be setted to be 3D buffer
	 **/
	/**
	 ** config 3D surface par
	 **/
	pstPar->st3DInfo.st3DSurface.enFmt     = pstPar->stExtendInfo.enColFmt;
	pstPar->st3DInfo.st3DSurface.u32Pitch  = ((((pstPar->stExtendInfo.u32DisplayWidth * info->var.bits_per_pixel) >> 3) + 0xf) & 0xfffffff0);
	pstPar->st3DInfo.st3DSurface.u32Width  = pstPar->stExtendInfo.u32DisplayWidth;
	pstPar->st3DInfo.st3DSurface.u32Height = pstPar->stExtendInfo.u32DisplayHeight;
	pstPar->st3DInfo.st3DSurface.u32PhyAddr= pstPar->st3DInfo.u32DisplayAddr[u32Index];

	memcpy(&stBackBuf.stCanvas, &pstPar->st3DInfo.st3DSurface, sizeof(MTFB_SURFACE_S));

    /**
     ** according to the hw arithemetic, calculate  source and Dst fresh rectangle
     **/
    mtfb_getupdate_rect(u32LayerId, pstCanvasBuf, &stBackBuf.UpdateRect);

    /**
     ** We should check is address changed, for make sure
     ** that the address configed to the hw reigster is in effect
     **/
    if(pstPar->stRunInfo.bFliped && (stOsdData.u32RegPhyAddr== pstPar->st3DInfo.u32DisplayAddr[1-u32Index]))
    {
		/**
		 ** when fill background buffer, we need to backup fore buffer first
		 **/
		mtfb_backup_forebuf(u32LayerId, &stBackBuf);
        /** clear union rect **/
        memset(&(pstPar->st3DInfo.st3DUpdateRect), 0, sizeof(MTFB_RECT));
        pstPar->stRunInfo.bFliped = MT_FALSE;
    }

    /* update union rect */
    if ((pstPar->st3DInfo.st3DUpdateRect.w == 0) || (pstPar->st3DInfo.st3DUpdateRect.h == 0))
    {
        memcpy(&pstPar->st3DInfo.st3DUpdateRect, &stBackBuf.UpdateRect, sizeof(MTFB_RECT));
    }
    else
    {
        MTFB_UNITE_RECT(pstPar->st3DInfo.st3DUpdateRect, stBackBuf.UpdateRect);
    }

	stBlitOpt.bScale = MT_TRUE;
	stBlitOpt.bRegionDeflicker = MT_TRUE;
	if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
	{
		stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
	}

	mtfb_3DData_Config(u32LayerId, pstCanvasBuf, &stBlitOpt);

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}


/**
 ** In tmts function we should wait the new contain has
 ** been show on the screen before return, and the operations
 ** such as address configuration no needed do in interrupt handle
 **/
/***************************************************************************************
* func			: mtfb_refresh_2buf_immediate_display_3D
* description	: CNcomment: 3 buffer 同步，刷新等待更新完 CNend\n
* param[in] 	: mt_void
* retval		: NA
* others:		: NA
***************************************************************************************/
static mt_s32 mtfb_refresh_2buf_immediate_display_3D(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf)
{
	mt_s32 s32Ret = MT_SUCCESS;
	mt_u32 u32Index = 0;
	MTFB_PAR_S *pstPar;
	struct fb_info *info;
	unsigned long lockflag;
	MTFB_BUFFER_S stBackBuf;
	MTFB_BLIT_OPT_S stBlitOpt;

    MTFB_FUN_IN;
	s32Ret	 = 0;
	info	 = s_stLayer[u32LayerId].pstInfo;
	pstPar	 = (MTFB_PAR_S *)info->par;
	u32Index = pstPar->stRunInfo.u32IndexForInt;

	memset(&stBlitOpt, 0, sizeof(MTFB_BLIT_OPT_S));
	memset(&stBackBuf, 0, sizeof(MTFB_BUFFER_S));


	stBlitOpt.bCallBack = MT_FALSE; /** TDE工作完不需要回调注册的TDE CALLBACK函数 **/
	stBlitOpt.pParam = &(pstPar->stBaseInfo.u32LayerID);

	spin_lock_irqsave(&pstPar->stBaseInfo.lock,lockflag);
	pstPar->stRunInfo.bNeedFlip 	   = MT_FALSE;
	pstPar->stRunInfo.s32RefreshHandle = 0;
	spin_unlock_irqrestore(&pstPar->stBaseInfo.lock,lockflag);

	/**
	 ** no need to allocate 3D buffer, displaybuf[0] will be setted to be 3D buffer
	 **/
	pstPar->st3DInfo.st3DSurface.enFmt	   = pstPar->stExtendInfo.enColFmt;
	pstPar->st3DInfo.st3DSurface.u32Pitch  = ((((pstPar->stExtendInfo.u32DisplayWidth * info->var.bits_per_pixel) >> 3) + 0xf) & 0xfffffff0);
	pstPar->st3DInfo.st3DSurface.u32Width  = pstPar->stExtendInfo.u32DisplayWidth;
	pstPar->st3DInfo.st3DSurface.u32Height = pstPar->stExtendInfo.u32DisplayHeight;
	pstPar->st3DInfo.st3DSurface.u32PhyAddr= pstPar->st3DInfo.u32DisplayAddr[u32Index];

	memcpy(&stBackBuf.stCanvas, &pstPar->st3DInfo.st3DSurface, sizeof(MTFB_SURFACE_S));

	/**
	 ** according to the hw arithemetic, calculate  source and Dst fresh rectangle
	 **/
	mtfb_getupdate_rect(u32LayerId, pstCanvasBuf, &stBackBuf.UpdateRect);

	/**
	 ** when fill background buffer, we need to backup fore buffer first
	 **/
	mtfb_backup_forebuf(u32LayerId, &stBackBuf);

	/* update union rect */
	memcpy(&pstPar->st3DInfo.st3DUpdateRect, &stBackBuf.UpdateRect, sizeof(MTFB_RECT));


	stBlitOpt.bScale = MT_TRUE;
	stBlitOpt.bBlock = MT_TRUE;
	stBlitOpt.bRegionDeflicker = MT_TRUE;
	if (pstPar->stBaseInfo.enAntiflickerMode == MTFB_ANTIFLICKER_TDE)
	{
		stBlitOpt.enAntiflickerLevel = pstPar->stBaseInfo.enAntiflickerLevel;
	}

	mtfb_3DData_Config(u32LayerId, pstCanvasBuf, &stBlitOpt);

    pstPar->stRunInfo.bModifying = MT_TRUE;
    pstPar->stRunInfo.u32ScreenAddr       = pstPar->st3DInfo.u32DisplayAddr[u32Index];
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
    pstPar->stRunInfo.bModifying = MT_FALSE;

	pstPar->stRunInfo.u32IndexForInt = 1 - u32Index;

    memcpy(&(pstPar->stDispInfo.stUserBuffer), pstCanvasBuf, sizeof(MTFB_BUFFER_S));

    /**
     ** wait the address register's configuration take effect before return
     **/

    mtfb_wait_regconfig_work(u32LayerId);

    MTFB_FUN_OUT;

	return MT_SUCCESS;

}
#endif



/***************************************************************************************
* func          : mtfb_refresh
* description   : CNcomment: 刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_refresh(mt_u32 u32LayerId, MTFB_BUFFER_S *pstCanvasBuf, MTFB_LAYER_BUF_E enBufMode)
{
    mt_s32 s32Ret;
	MTFB_PAR_S *par;
	struct fb_info *info;

	s32Ret = MT_FAILURE;
	info   = s_stLayer[u32LayerId].pstInfo;
	par    = (MTFB_PAR_S *)(info->par);

   MTFB_FUN_IN;

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	if (par->bSetStereoMode)
	{
	    MTFB_LINE;
		switch (enBufMode)
	    {
	        case MTFB_LAYER_BUF_DOUBLE:
	            s32Ret = mtfb_refresh_2buf_3D(u32LayerId, pstCanvasBuf);
	            break;
	        case MTFB_LAYER_BUF_ONE:
	            s32Ret = mtfb_refresh_1buf_3D(u32LayerId, pstCanvasBuf);
	            break;
	        case MTFB_LAYER_BUF_NONE:
	           s32Ret = mtfb_refresh_0buf_3D(u32LayerId, pstCanvasBuf);
	           break;
	        case MTFB_LAYER_BUF_DOUBLE_IMMEDIATE:
	            s32Ret = mtfb_refresh_2buf_immediate_display_3D(u32LayerId, pstCanvasBuf);
	            break;
	        default:
	            break;
	    }
	}
	else
#endif
	{
	   MTFB_LINE;
		switch (enBufMode)
	    {
	        case MTFB_LAYER_BUF_DOUBLE:
	            s32Ret = mtfb_refresh_2buf(u32LayerId, pstCanvasBuf);
	            break;
	        case MTFB_LAYER_BUF_ONE:
	            s32Ret = mtfb_refresh_1buf(u32LayerId, pstCanvasBuf);
	            break;
	        case MTFB_LAYER_BUF_NONE:
	           s32Ret = mtfb_refresh_0buf(u32LayerId, pstCanvasBuf);
	           break;
	        case MTFB_LAYER_BUF_DOUBLE_IMMEDIATE:
	            s32Ret = mtfb_refresh_2buf_immediate_display(u32LayerId, pstCanvasBuf);
	            break;
	        default:
	            break;
	    }
	}

#ifdef CFG_MTFB_LOGO_SUPPORT
	mtfb_clear_logo(u32LayerId, MT_FALSE);
#endif

    MTFB_FUN_OUT;
    return s32Ret;

}

static mt_s32 mtfb_alloccanbuf(struct fb_info *info, MTFB_LAYER_INFO_S * pLayerInfo)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
    if (!(pLayerInfo->u32Mask & MTFB_LAYERMASK_CANVASSIZE))
    {
        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    /** if  with old canvas buffer **/
    if (par->stDispInfo.stCanvasSur.u32PhyAddr)
    {
        /* if  old is the sampe with new , then return, else free the old buffer*/
        if ((pLayerInfo->u32CanvasWidth == par->stDispInfo.stCanvasSur.u32Width) &&
             (pLayerInfo->u32CanvasHeight == par->stDispInfo.stCanvasSur.u32Height))
        {
            MTFB_DEBUGK("mem is the sampe , no need alloc new memory");
            MTFB_FUN_OUT;
            return MT_SUCCESS;
        }

        /** free new old buffer **/
        MTFB_DEBUGK("free old canvas buffer\n");
        mtfb_freeccanbuf(par);
    }

    /** new canvas buffer **/
    if ((pLayerInfo->u32CanvasWidth >=  0) && (pLayerInfo->u32CanvasHeight >=  0))
    {
        mt_u32 u32LayerSize;
        mt_u32 u32Pitch;
        mt_char *pBuf;

        /*Modify 16 to 32, preventing out of bound.*/
        mt_char name[32];

        /*16 bytes aligmn*/
        u32Pitch = ((pLayerInfo->u32CanvasWidth * info->var.bits_per_pixel >> 3) + 15)>>4;
        u32Pitch = u32Pitch << 4;

        u32LayerSize = u32Pitch * pLayerInfo->u32CanvasHeight;
        /** alloc new buffer*/
		snprintf(name, sizeof(name), "MTFB_Canvas%d", par->stBaseInfo.u32LayerID);
        par->stDispInfo.stCanvasSur.u32PhyAddr = mtfb_buf_allocmem(name, u32LayerSize);
        //MTFB_DEBUGK("canvas surface addr:0x%x\n", par->CanvasSur.u32PhyAddr);
        if (par->stDispInfo.stCanvasSur.u32PhyAddr == 0)
        {
            MTFB_DEBUGK("alloc canvas buffer no mem, expect size: 0x%x, cavh:%d\n", u32LayerSize, pLayerInfo->u32CanvasHeight);
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        pBuf = (mt_char *)mtfb_buf_map(par->stDispInfo.stCanvasSur.u32PhyAddr);
        if (pBuf == MT_NULL)
        {
            MTFB_DEBUGK("map canvas buffer failed!\n");
            mtfb_buf_freemem(par->stDispInfo.stCanvasSur.u32PhyAddr);
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        memset(pBuf, 0, u32LayerSize);
        mtfb_buf_ummap(pBuf);

        MTFB_DEBUGK("alloc new memory for canvas buffer success\n");
        par->stDispInfo.stCanvasSur.u32Width  = pLayerInfo->u32CanvasWidth;
        par->stDispInfo.stCanvasSur.u32Height = pLayerInfo->u32CanvasHeight;
        par->stDispInfo.stCanvasSur.enFmt     =  mtfb_getfmtbyargb(&info->var.red, &info->var.green, &info->var.blue, &info->var.transp, info->var.bits_per_pixel);
        par->stDispInfo.stCanvasSur.u32Pitch  = u32Pitch;

        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    MTFB_FUN_OUT;
	return MT_SUCCESS;
}


/***************************************************************************************
* func          : mtfb_pan_display
* description   : CNcomment: hwc中hwc_set调用，android使用的是这个接口，
                             要是有hwc库的情况下 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_hwcrefresh(MTFB_PAR_S* par, mt_void __user *argp)
{

#ifdef CFG_MTFB_FENCE_SUPPORT
    mt_s32 s32Ret;
    struct fb_info *info;
    MTFB_HWC_LAYERINFO_S stLayerInfo;
    mt_u32 pixStride = 0;

    MTFB_FUN_IN;
    if (copy_from_user(&stLayerInfo, argp, sizeof(MTFB_HWC_LAYERINFO_S)))
    {
        MTFB_FUN_OUT;
        return -EFAULT;
    }
	/**
	 **创建mtfb fence，返回文件节点给stLayerInfo.s32ReleaseFenceFd
	 **/
//	 printk("\r\n %d, %d", s_SyncInfo.u32Timeline, s_SyncInfo.u32FenceValue);
	s32Ret = mtfb_create_fence(s_SyncInfo.pstTimeline, "mtfb_fence", ++s_SyncInfo.u32FenceValue);
	if (s32Ret < 0)
	{
		MTFB_DEBUGK("mtfb_create_fence failed! s32Ret = 0x%x\n", s32Ret);
	}
	/**
	 **fence设备节点，每一个layer都有一个acquire 和release fence
	 **禁止显示一个buffer的内容直到该fence被触发，而它是在HW 被set up 前被发送的
	 **这个意味着属于这个layer的buffer已经不在被读取了，在一个buffer不在被读取的时候将会触发这个fence
	 **/
    stLayerInfo.s32ReleaseFenceFd = s32Ret;
    if (stLayerInfo.s32AcquireFenceFd >= 0)
    {/**
      **禁止显示一个buffer的内容直到该fence被触发,也就是s32AcquireFenceFd被释放
      **才能进行更新显示
      **/
        mtfb_fence_wait(stLayerInfo.s32AcquireFenceFd, 1000);
    }
//	printk("\r\nxxxxx %d, %d", s_SyncInfo.u32Timeline, s_SyncInfo.u32FenceValue);

	/**
	 **测试帧率
	 **/
    par->stFrameInfo.u32RefreshFrame++;

	/** GPU已经绘制完，可以刷新了 **/
    atomic_inc(&s_SyncInfo.s32RefreshCnt);

    if(par->bSetStereoMode)
    {/** 3d模式情况下 **/
        info = s_stLayer[par->stBaseInfo.u32LayerID].pstInfo;
        if (atomic_read(&s_SyncInfo.s32RefreshCnt) > 1)
        {
            s_SyncInfo.FrameEndFlag = 0;
            wait_event_interruptible_hrtimeout(s_SyncInfo.FrameEndEvent, s_SyncInfo.FrameEndFlag, ms_to_ktime(1000));
        }
        info->var.yoffset = (stLayerInfo.u32LayerAddr - info->fix.smem_start)/stLayerInfo.u32Stride;
		mtfb_pan_display(&info->var, info);
    }
	else
	{	/** 非3d模式情况下，修改3D问题 **/
		/**
		 **这里是三buffer进行切换，这个地址由HWC切换
		 **/
	    par->stRunInfo.u32ScreenAddr  = stLayerInfo.u32LayerAddr;
	    s_stDrvOps.MTFB_DRV_SetLayerAddr(par->stBaseInfo.u32LayerID, par->stRunInfo.u32ScreenAddr);
        pixStride = calc_pixel_stride_by_pitch_fmt(stLayerInfo.u32Stride, par->stExtendInfo.enColFmt, __LINE__);
	    //s_stDrvOps.MTFB_DRV_SetLayerStride(par->stBaseInfo.u32LayerID, stLayerInfo.u32Stride);
       s_stDrvOps.MTFB_DRV_SetLayerStride(par->stBaseInfo.u32LayerID, pixStride);

		par->stExtendInfo.stPos.s32XPos    = stLayerInfo.stInRect.x;
	    par->stExtendInfo.stPos.s32YPos    = stLayerInfo.stInRect.y;
	    par->stExtendInfo.u32DisplayWidth  = stLayerInfo.stInRect.w;
	    par->stExtendInfo.u32DisplayHeight = stLayerInfo.stInRect.h;
		/**
		 **输入的制式，微调是影响到输出的stOutRect
		 **/
	    s_stDrvOps.MTFB_DRV_SetLayerInRect(par->stBaseInfo.u32LayerID, &stLayerInfo.stInRect);
		if (par->stExtendInfo.enColFmt != stLayerInfo.eFmt)
		{
			par->stExtendInfo.enColFmt = stLayerInfo.eFmt;
			par->stRunInfo.u32ParamModifyMask  |= MTFB_LAYER_PARAMODIFY_FMT;
		}
		if (par->stRunInfo.u32ParamModifyMask & MTFB_LAYER_PARAMODIFY_FMT)
		{
			s_stDrvOps.MTFB_DRV_SetLayerDataFmt(par->stBaseInfo.u32LayerID, par->stExtendInfo.enColFmt);
			par->stRunInfo.u32ParamModifyMask &= ~MTFB_LAYER_PARAMODIFY_FMT;
		}

	    s_stDrvOps.MTFB_DRV_UpdataLayerReg(par->stBaseInfo.u32LayerID);
	}

    if (copy_to_user(argp,&stLayerInfo,sizeof(MTFB_HWC_LAYERINFO_S)))
    {/**
      **释放没有用的fence
      **/
		 put_unused_fd(stLayerInfo.s32ReleaseFenceFd);
        MTFB_FUN_OUT;
        return -EFAULT;
	}

#ifdef CFG_MTFB_LOGO_SUPPORT
	mtfb_clear_logo(par->stBaseInfo.u32LayerID, MT_FALSE);
#endif

    par->bHwcRefresh = MT_TRUE;
#endif
    MTFB_FUN_OUT;
    return MT_SUCCESS;

}


/***************************************************************************************
* func          : mtfb_onrefresh
* description   : CNcomment: 刷新 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_onrefresh(MTFB_PAR_S* par, mt_void __user *argp)
{

    mt_s32 s32Ret;
    MTFB_BUFFER_S stCanvasBuf;

    MTFB_FUN_IN;
    if (par->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
    {
        MTFB_WARNING("you shouldn't refresh cursor layer!");
        MTFB_FUN_OUT;
        return MT_SUCCESS;
    }

    if (copy_from_user(&stCanvasBuf, argp, sizeof(MTFB_BUFFER_S)))
    {
        MTFB_FUN_OUT;
        return -EFAULT;
    }

    /**
     ** when user data  update in 3d mode ,
     ** blit pan buffer to 3D buffer to config 3d data
     **/
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    if (  (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD)
         &&((par->st3DInfo.st3DMemInfo.u32StereoMemStart != 0) && (par->bSetStereoMode)))
    {

        MTFB_FUN_OUT;
        /**
          ** 标准刷新模式，3D模式，起始地址不为0
          **/
        return mtfb_refresh_panbuf(par->stBaseInfo.u32LayerID, &stCanvasBuf);
    }

	/**
	 ** when user refresh in pan display , just return
	 **/
    if (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD)
    {
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }
#endif

	if (   (0 == stCanvasBuf.stCanvas.u32Width)
		|| (0 == stCanvasBuf.stCanvas.u32Height))
	{
		MTFB_DEBUGK("canvas buffer's width or height can't be zero.\n");
       MTFB_FUN_OUT;
		return MT_FAILURE;
	}

	if (stCanvasBuf.stCanvas.enFmt >= MTFB_FMT_BUTT)
	{
		MTFB_DEBUGK("color format of canvas buffer unsupported.\n");
       MTFB_FUN_OUT;
		return MT_FAILURE;
	}

    if (  (stCanvasBuf.UpdateRect.x >=  stCanvasBuf.stCanvas.u32Width)
        ||(stCanvasBuf.UpdateRect.y >= stCanvasBuf.stCanvas.u32Height)
        ||(stCanvasBuf.UpdateRect.w == 0) || (stCanvasBuf.UpdateRect.h == 0))
    {
        MTFB_DEBUGK("rect error: update rect:(%d,%d,%d,%d), canvas range:(%d,%d)\n",
                  stCanvasBuf.UpdateRect.x, stCanvasBuf.UpdateRect.y,
                  stCanvasBuf.UpdateRect.w, stCanvasBuf.UpdateRect.h,
                  stCanvasBuf.stCanvas.u32Width, stCanvasBuf.stCanvas.u32Height);
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }

    if (stCanvasBuf.UpdateRect.x + stCanvasBuf.UpdateRect.w > stCanvasBuf.stCanvas.u32Width)
    {
        stCanvasBuf.UpdateRect.w = stCanvasBuf.stCanvas.u32Width - stCanvasBuf.UpdateRect.x;
    }
    if (stCanvasBuf.UpdateRect.y + stCanvasBuf.UpdateRect.h > stCanvasBuf.stCanvas.u32Height)
    {
        stCanvasBuf.UpdateRect.h =  stCanvasBuf.stCanvas.u32Height - stCanvasBuf.UpdateRect.y;
    }

    if (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
    {
       MTFB_LINE;
       /** 只有canvas buffer **/
        /**
         ** there's a limit from hardware that the start address of screen buf
         ** should be 16byte aligned!
         **/
        if ((stCanvasBuf.stCanvas.u32PhyAddr & 0xf) || (stCanvasBuf.stCanvas.u32Pitch & 0xf))
        {
            MTFB_DEBUGK("addr 0x%llx or pitch: 0x%x is not 16 bytes align !\n",
                stCanvasBuf.stCanvas.u32PhyAddr,
                stCanvasBuf.stCanvas.u32Pitch);
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }
    }

    s32Ret = mtfb_refresh(par->stBaseInfo.u32LayerID, &stCanvasBuf, par->stExtendInfo.enBufMode);

    MTFB_FUN_OUT;
    return s32Ret;

}

static mt_s32 mtfb_onputlayerinfo(struct fb_info *info, MTFB_PAR_S* par, mt_void __user *argp)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MTFB_LAYER_INFO_S stLayerInfo;
    mt_u32 u32Pitch;

    MTFB_FUN_IN;
    if (par->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
    {
       MTFB_WARNING("you shouldn't put cursor layer info!");
       MTFB_FUN_OUT;
       return MT_SUCCESS;
    }

    if (copy_from_user(&stLayerInfo, argp, sizeof(MTFB_LAYER_INFO_S)))
    {
      MTFB_FUN_OUT;
       return -EFAULT;
    }

    s32Ret = mtfb_alloccanbuf(info, &stLayerInfo);
    if (s32Ret != MT_SUCCESS)
    {
       MTFB_DEBUGK("alloc canvas buffer failed\n");
       MTFB_FUN_OUT;
       return MT_FAILURE;
    }

    MTFB_LINE;
    if (stLayerInfo.u32Mask & MTFB_LAYERMASK_DISPSIZE)
    {
        u32Pitch = stLayerInfo.u32DisplayWidth* info->var.bits_per_pixel >> 3;
        u32Pitch = (u32Pitch + 0xf) & 0xfffffff0;

		if (stLayerInfo.u32DisplayWidth == 0 || stLayerInfo.u32DisplayHeight == 0)
        {
            MTFB_DEBUGK("display witdh/height shouldn't be 0!\n");
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        if(MT_FAILURE == mtfb_checkmem_enough(info, u32Pitch, stLayerInfo.u32DisplayHeight))
        {
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }
    }

    if (stLayerInfo.u32Mask & MTFB_LAYERMASK_SCREENSIZE)
    {
       MTFB_LINE;
       if ((stLayerInfo.u32ScreenWidth == 0) || (stLayerInfo.u32ScreenHeight == 0))
       {
           MTFB_DEBUGK("screen width/height shouldn't be 0\n");
           MTFB_FUN_OUT;
           return MT_FAILURE;
       }
    }

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
     if ( ((stLayerInfo.u32Mask & MTFB_LAYERMASK_DISPSIZE)
	 		&& par->bSetStereoMode))
     {
        MTFB_LINE;
        mtfb_clearallstereobuf(info);
     }
#endif

	if (stLayerInfo.u32Mask & MTFB_LAYERMASK_BUFMODE)
	{
        mt_u32 u32LayerSize;

        MTFB_LINE;

		if (stLayerInfo.BufMode == MTFB_LAYER_BUF_ONE)
		{
		   u32LayerSize = info->fix.line_length * info->var.yres;
		}
		else if ((stLayerInfo.BufMode == MTFB_LAYER_BUF_DOUBLE)
		    || (stLayerInfo.BufMode == MTFB_LAYER_BUF_DOUBLE_IMMEDIATE))
		{
		   u32LayerSize = 2 * info->fix.line_length * info->var.yres;
		}
		else
		{
		   u32LayerSize = 0;
		}

		if (u32LayerSize > info->fix.smem_len)
		{
		   MTFB_DEBUGK("No enough mem! layer real memory size:%d KBytes, expected:%d KBtyes\n",
		       info->fix.smem_len/1024, u32LayerSize/1024);
          MTFB_FUN_OUT;
		   return MT_FAILURE;
		}
    }

    /*if x>width or y>height ,how to deal with: see notmtng in screen or return failure?*/
    if ((stLayerInfo.u32Mask & MTFB_LAYERMASK_POS)
       && ((stLayerInfo.s32XPos < 0) || (stLayerInfo.s32YPos < 0)))
    {
       MTFB_DEBUGK("Pos err!\n");
       MTFB_FUN_OUT;
       return MT_FAILURE;
    }

    if ((stLayerInfo.u32Mask & MTFB_LAYERMASK_BMUL) && par->stExtendInfo.stCkey.bKeyEnable)
    {
       MTFB_DEBUGK("Colorkey and premul couldn't take effect at same time!\n");
       MTFB_FUN_OUT;
       return MT_FAILURE;
    }

    /*avoid modifying register in vo isr before all params has benn recorded! In vo irq,
       flag bModifying will be checked.*/
    par->stRunInfo.bModifying = MT_TRUE;

    MTFB_DEBUGK("%s %d: 0x%x\n", __FUNCTION__, __LINE__,  stLayerInfo.u32Mask);
    if (stLayerInfo.u32Mask & MTFB_LAYERMASK_BMUL)
    {
        par->stBaseInfo.bPreMul            = stLayerInfo.bPreMul;
        par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_BMUL;
    }


    if (stLayerInfo.u32Mask & MTFB_LAYERMASK_BUFMODE)
    {
        mtfb_buf_setbufmode(par->stBaseInfo.u32LayerID, stLayerInfo.BufMode);
    }

	if (stLayerInfo.u32Mask & MTFB_LAYERMASK_POS)
	{
	    mtfb_disp_setlayerpos(par->stBaseInfo.u32LayerID, stLayerInfo.s32XPos, stLayerInfo.s32YPos);
	}

	if (stLayerInfo.u32Mask & MTFB_LAYERMASK_ANTIFLICKER_MODE)
	{
	    mtfb_disp_setantiflickerlevel(par->stBaseInfo.u32LayerID, stLayerInfo.eAntiflickerLevel);
	}

	if (stLayerInfo.u32Mask & MTFB_LAYERMASK_SCREENSIZE)
	{
	    s32Ret = mtfb_disp_setscreensize(par->stBaseInfo.u32LayerID, stLayerInfo.u32ScreenWidth, stLayerInfo.u32ScreenHeight);
		if (MT_SUCCESS == s32Ret)
		{
			s_stDrvOps.MTFB_DRV_SetScreenFlag(par->stBaseInfo.u32LayerID, MT_TRUE);
		}
	}

	if (stLayerInfo.u32Mask & MTFB_LAYERMASK_DISPSIZE)
	{
		if (stLayerInfo.u32DisplayWidth <= info->var.xres_virtual
			&& stLayerInfo.u32DisplayHeight <= info->var.yres_virtual)
		{
			s32Ret = mtfb_disp_setdispsize(par->stBaseInfo.u32LayerID, stLayerInfo.u32DisplayWidth, stLayerInfo.u32DisplayHeight);
			if (s32Ret == MT_SUCCESS)
			{
			    info->var.xres = stLayerInfo.u32DisplayWidth;
				info->var.yres = stLayerInfo.u32DisplayHeight;
				mtfb_assign_dispbuf(par->stBaseInfo.u32LayerID);
			}

			mtfb_refreshall(info);
		}
	}

    par->stRunInfo.bModifying = MT_FALSE;

    MTFB_FUN_OUT;
    return s32Ret;
}

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
static mt_s32 mtfb_clearallstereobuf(struct fb_info *info)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
    if (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_STANDARD || par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
    {
        mtfb_clearstereobuf(info);
    }
    else
    {
        MTFB_BLIT_OPT_S stOpt;
        MTFB_SURFACE_S Surface;
		mt_u32 u32InSize  = 0;
		mt_u32 u32Stride  = 0;
        memset(&stOpt, 0x0, sizeof(stOpt));

        Surface.enFmt     = par->stExtendInfo.enColFmt;
        Surface.u32Height = par->stExtendInfo.u32DisplayHeight;
        Surface.u32Width  = par->stExtendInfo.u32DisplayWidth;

		u32InSize = (par->stExtendInfo.u32DisplayWidth * info->var.bits_per_pixel) >> 3;
	    MT_MTFB_GetStride(u32InSize,&u32Stride,CONFIG_MTFB_STRIDE_16ALIGN);
        Surface.u32Pitch  = u32Stride;

        if (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_DOUBLE
                ||par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_DOUBLE_IMMEDIATE)
        {
            Surface.u32PhyAddr = par->st3DInfo.u32DisplayAddr[par->stRunInfo.u32IndexForInt];
        }
        else
        {
            Surface.u32PhyAddr = par->st3DInfo.u32DisplayAddr[0];
        }

		if (MT_NULL == Surface.u32PhyAddr)
		{
			MTFB_DEBUGK("fail to clear stereo rect.\n");
           MTFB_FUN_OUT;
			return MT_FAILURE;
		}
        s_stDrvTdeOps.MTFB_DRV_ClearRect(&Surface, &stOpt);
    }

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}
#endif

static mt_s32 mtfb_refreshuserbuffer(mt_u32 u32LayerId)
{
	MTFB_PAR_S *par;
	struct fb_info *info;

    MTFB_FUN_IN;

	info = s_stLayer[u32LayerId].pstInfo;
	par = (MTFB_PAR_S *)info->par;

	if (par->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr)
    {
        MTFB_BUFFER_S stCanvas;
        stCanvas = par->stDispInfo.stUserBuffer;
        stCanvas.UpdateRect.x = 0;
        stCanvas.UpdateRect.y = 0;
        stCanvas.UpdateRect.w = stCanvas.stCanvas.u32Width;
        stCanvas.UpdateRect.h = stCanvas.stCanvas.u32Height;

        mtfb_refresh(par->stBaseInfo.u32LayerID, &stCanvas, par->stExtendInfo.enBufMode);
    }

    MTFB_FUN_OUT;
	return MT_SUCCESS;

}

static mt_s32 mtfb_refreshall(struct fb_info *info)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    if (par->bSetStereoMode)//(IS_STEREO_SBS(par) || IS_STEREO_TAB(par))
    {
       MTFB_LINE;
    	if (MTFB_LAYER_BUF_STANDARD == par->stExtendInfo.enBufMode)
    	{
        	mtfb_pan_display(&info->var, info);
    	}

		if (MTFB_LAYER_BUF_NONE == par->stExtendInfo.enBufMode)
		{
			mtfb_refreshuserbuffer(par->stBaseInfo.u32LayerID);
		}
    }
#endif

    if (MTFB_LAYER_BUF_STANDARD != par->stExtendInfo.enBufMode
		 && MTFB_LAYER_BUF_NONE != par->stExtendInfo.enBufMode)
    {
       MTFB_LINE;
		mtfb_refreshuserbuffer(par->stBaseInfo.u32LayerID);
    }

    MTFB_FUN_OUT;
    return MT_SUCCESS;
}




/***************************************************************************************
* func          : mtfb_ioctl
* description   : CNcomment: API 函数 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_ioctl(struct fb_info *info, mt_u32 cmd, unsigned long arg)
{

    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;
    mt_void __user *argp = (mt_void __user *)arg;

    mt_s32 s32Ret = MT_SUCCESS;

#ifndef CONFIG_MTFB_CURSOR_LAYER_NEGATIVE_SUPPORT
    mt_u32 u32Bpp, u32Addr;
    MTFB_RECT   stOutputRect;
    mt_u32 u32LayerId;
#endif

    MTFB_FUN_IN;

    if ((argp == NULL) && (cmd != FBIOGET_VBLANK_MTFB) && (cmd != FBIO_WAITFOR_FREFRESH_DONE)
		&& (cmd != FBIO_FREE_LOGO))
    {
        MTFB_FUN_OUT;
        return -EINVAL;
    }

    if (  (!g_pstCap[par->stBaseInfo.u32LayerID].bLayerSupported)
        &&(par->stBaseInfo.u32LayerID != MTFB_LAYER_CURSOR))
    {
        MTFB_DEBUGK("not supprot layer %d!\n", par->stBaseInfo.u32LayerID);
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }

    MTFB_DEBUGK("%s %d: 0x%x\n", __FUNCTION__, __LINE__,  cmd);
    switch (cmd)
    {
        case FBIO_HWC_REFRESH:
        {
            MTFB_LINE;
            s32Ret = mtfb_hwcrefresh(par, argp);
            break;
        }
        case FBIO_REFRESH:
        {
            MTFB_LINE;
            s32Ret = mtfb_onrefresh(par, argp);
            break;
        }
        case FBIOGET_CANVAS_BUFFER:
        {
            MTFB_LINE;
            if (copy_to_user(argp, &(par->stDispInfo.stCanvasSur), sizeof(MTFB_BUFFER_S)))
            {
                MTFB_FUN_OUT;
                return -EFAULT;
            }
            return MT_SUCCESS;
        }
    	case FBIOPUT_LAYER_INFO:
    	{
            MTFB_LINE;
            s32Ret= mtfb_onputlayerinfo(info, par, argp);
            break;
    	}
    	case FBIOGET_LAYER_INFO:
    	{
            MTFB_LAYER_INFO_S stLayerInfo = {0};
            MTFB_LINE;
			  mtfb_wait_regconfig_work(par->stBaseInfo.u32LayerID);

            stLayerInfo.bPreMul           = par->stBaseInfo.bPreMul;
            stLayerInfo.BufMode           = par->stExtendInfo.enBufMode;
            stLayerInfo.eAntiflickerLevel = par->stBaseInfo.enAntiflickerLevel;
            stLayerInfo.s32XPos           = par->stExtendInfo.stPos.s32XPos;
            stLayerInfo.s32YPos           = par->stExtendInfo.stPos.s32YPos;
            stLayerInfo.u32DisplayWidth   = par->stExtendInfo.u32DisplayWidth;
            stLayerInfo.u32DisplayHeight  = par->stExtendInfo.u32DisplayHeight;
			stLayerInfo.u32ScreenWidth    = par->stExtendInfo.u32ScreenWidth;
			stLayerInfo.u32ScreenHeight   = par->stExtendInfo.u32ScreenHeight;

            return copy_to_user(argp, &stLayerInfo, sizeof(MTFB_LAYER_INFO_S));
    	}
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
        case FBIOGET_ENCODER_PICTURE_FRAMING:
        {
             MTFB_LINE;
			#ifdef MTFB_FOR_TEST
	            if (copy_to_user(argp, &par->st3DInfo.enOutStereoMode, sizeof(MTFB_STEREO_MODE_E)))
	            {
	                MTFB_FUN_OUT;
	                return -EFAULT;
	            }
			#endif
            break;
        }
        case FBIOPUT_ENCODER_PICTURE_FRAMING:
        {
			#ifdef MTFB_FOR_TEST
				MTFB_STEREO_MODE_E epftmp;
               MTFB_LINE;
	            if (copy_from_user(&epftmp, argp, sizeof(MTFB_STEREO_MODE_E)))
	            {
	                return -EFAULT;
	            }
				mtfb_3DMode_callback(&par->stBaseInfo.u32LayerID, &epftmp);
			#endif
            break;
        }
        case FBIOPUT_STEREO_MODE:
        {
           MTFB_LINE;
			break;
        }
        case FBIOGET_STEREO_MODE:
        {
            MTFB_LINE;
            break;
        }
#endif

#ifdef CFG_MTFB_COMPRESSION_SUPPORT
        case FBIOPUT_COMPRESSION:
        {
            MT_BOOL bComp = MT_FALSE;
			  MT_BOOL bComp_pre = MT_FALSE;
            MTFB_LINE;
            if (copy_from_user(&bComp, argp, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

           MTFB_LINE;
			bComp_pre = s_stDrvOps.MTFB_DRV_GetCmpSwitch(par->stBaseInfo.u32LayerID);

			if (bComp == bComp_pre)
			{
				return MT_SUCCESS;
			}


            if (bComp == MT_TRUE)
            {
              MTFB_LINE;
            	 if (!g_pstCap[par->stBaseInfo.u32LayerID].bCompression)
		        {
		        	MTFB_DEBUGK("mtfb% don't support compression\n",par->stBaseInfo.u32LayerID);
		            return MT_FAILURE;
		        }

                if (par->stExtendInfo.enColFmt != MTFB_FMT_ARGB8888)
                {
                    MTFB_DEBUGK("compression only support pixel format (ARGB8888)\n");
                    return MT_FAILURE;
                }

				if (par->bSetStereoMode)
				{
					  MTFB_DEBUGK("not support compression in 3d mode\n");
                    return MT_FAILURE;
				}
            }

			 s_stDrvOps.MTFB_DRV_SetCmpSwitch(par->stBaseInfo.u32LayerID, bComp);
            break;
        }
        case FBIOGET_COMPRESSION:
        {
			MT_BOOL bComp = MT_FALSE;
           MTFB_LINE;
			bComp = s_stDrvOps.MTFB_DRV_GetCmpSwitch(par->stBaseInfo.u32LayerID);

            if (copy_to_user(argp, &bComp, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

            break;
        }
		case FBIOPUT_COMPRESSIONMODE:
		{
			MTFB_CMP_MODE_E enMode;
            MTFB_LINE;
            if (copy_from_user(&enMode, argp, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

			if (enMode < MTFB_CMP_BUTT)
			{
			    MTFB_LINE;
				s_stDrvOps.MTFB_DRV_SetCmpMode(par->stBaseInfo.u32LayerID, enMode);
			}

			break;
		}
		case FBIOGET_COMPRESSIONMODE:
		{
			MTFB_CMP_MODE_E enMode;
           MTFB_LINE;
			enMode = s_stDrvOps.MTFB_DRV_GetCmpMode(par->stBaseInfo.u32LayerID);

            if (copy_to_user(argp, &enMode, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

            break;
		}
#endif
        case FBIOGET_ALPHA_MTFB:
        {
            MTFB_LINE;
            if (copy_to_user(argp, &par->stExtendInfo.stAlpha, sizeof(MTFB_ALPHA_S)))
            {
                MTFB_FUN_OUT;
                return -EFAULT;
            }

            break;
        }

        case FBIOPUT_ALPHA_MTFB:
        {
            MTFB_ALPHA_S stAlpha = {0};

            MTFB_LINE;
            if (copy_from_user(&par->stExtendInfo.stAlpha, argp, sizeof(MTFB_ALPHA_S)))
            {
                return -EFAULT;
            }

            stAlpha = par->stExtendInfo.stAlpha;
            if (!par->stExtendInfo.stAlpha.bAlphaChannel)
            {
                stAlpha.u8GlobalAlpha |= 0xff;
                par->stExtendInfo.stAlpha.u8GlobalAlpha |= 0xff;
            }

            s_stDrvOps.MTFB_DRV_SetLayerAlpha(par->stBaseInfo.u32LayerID, &stAlpha);
            break;
        }

        case FBIOGET_DEFLICKER_MTFB:
        {
            MTFB_DEFLICKER_S deflicker;
            MTFB_LINE;
            if (!g_pstCap[par->stBaseInfo.u32LayerID].u32HDefLevel
                && !g_pstCap[par->stBaseInfo.u32LayerID].u32VDefLevel)
            {
                MTFB_WARNING("deflicker is not supported!\n");
                return -EPERM;
            }

            if (copy_from_user(&deflicker, argp, sizeof(MTFB_DEFLICKER_S)))
            {
                return -EFAULT;
            }

            deflicker.u32HDfLevel = par->stBaseInfo.u32HDflevel;
            deflicker.u32VDfLevel = par->stBaseInfo.u32VDflevel;
            if (par->stBaseInfo.u32HDflevel > 1)
            {
                if (NULL == deflicker.pu8HDfCoef)
                {
                    return -EFAULT;
                }

                if (copy_to_user(deflicker.pu8HDfCoef, par->stBaseInfo.ucHDfcoef, par->stBaseInfo.u32HDflevel - 1))
                {
                    return -EFAULT;
                }
            }

            if (par->stBaseInfo.u32VDflevel > 1)
            {
                if (NULL == deflicker.pu8VDfCoef)
                {
                    return -EFAULT;
                }

                if (copy_to_user(deflicker.pu8VDfCoef, par->stBaseInfo.ucVDfcoef, par->stBaseInfo.u32VDflevel - 1))
                {
                    return -EFAULT;
                }
            }

            if (copy_to_user(argp, &deflicker, sizeof(deflicker)))
            {
                return -EFAULT;
            }

            break;
        }

        case FBIOPUT_DEFLICKER_MTFB:
        {
            MTFB_DEFLICKER_S deflicker;
             MTFB_LINE;
            if (!g_pstCap[par->stBaseInfo.u32LayerID].u32HDefLevel
                && !g_pstCap[par->stBaseInfo.u32LayerID].u32VDefLevel)
            {
                MTFB_WARNING("deflicker is not supported!\n");
                return -EPERM;
            }

            par->stRunInfo.bModifying = MT_TRUE;

            if (copy_from_user(&deflicker, argp, sizeof(MTFB_DEFLICKER_S)))
            {
                return -EFAULT;
            }

            par->stBaseInfo.u32HDflevel = MTFB_MIN(deflicker.u32HDfLevel, g_pstCap[par->stBaseInfo.u32LayerID].u32HDefLevel);
            if ((par->stBaseInfo.u32HDflevel > 1))
            {
                if (NULL == deflicker.pu8HDfCoef)
                {
                    return -EFAULT;
                }

                if (copy_from_user(par->stBaseInfo.ucHDfcoef, deflicker.pu8HDfCoef, par->stBaseInfo.u32HDflevel - 1))
                {
                    return -EFAULT;
                }
            }

            par->stBaseInfo.u32VDflevel = MTFB_MIN(deflicker.u32VDfLevel, g_pstCap[par->stBaseInfo.u32LayerID].u32VDefLevel);
            if (par->stBaseInfo.u32VDflevel > 1)
            {
                if (NULL == deflicker.pu8VDfCoef)
                {
                    return -EFAULT;
                }

                if (copy_from_user(par->stBaseInfo.ucVDfcoef, deflicker.pu8VDfCoef, par->stBaseInfo.u32VDflevel - 1))
                {
                    return -EFAULT;
                }
            }

            par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_ANTIFLICKERLEVEL;

            par->stRunInfo.bModifying = MT_FALSE;

            break;
        }

        case FBIOGET_COLORKEY_MTFB:
        {
            MTFB_COLORKEY_S ck;

            MTFB_LINE;
            ck.bKeyEnable = par->stExtendInfo.stCkey.bKeyEnable;
            ck.u32Key = par->stExtendInfo.stCkey.u32Key;
            if (copy_to_user(argp, &ck, sizeof(MTFB_COLORKEY_S)))
            {
                return -EFAULT;
            }

            break;
        }

        case FBIOPUT_COLORKEY_MTFB:
        {
            MTFB_COLORKEY_S ckey;
            MTFB_LINE;
            if (copy_from_user(&ckey, argp, sizeof(MTFB_COLORKEY_S)))
            {
                return -EFAULT;
            }

            if (ckey.bKeyEnable && par->stBaseInfo.bPreMul)
            {
                MTFB_DEBUGK("colorkey and premul couldn't take effect at the same time!\n");
                return MT_FAILURE;
            }

			par->stRunInfo.bModifying = MT_TRUE;

            par->stExtendInfo.stCkey.u32Key = ckey.u32Key;
            par->stExtendInfo.stCkey.bKeyEnable = ckey.bKeyEnable;


            if (info->var.bits_per_pixel <= 8)
            {
                MTFB_LINE;
                if (ckey.u32Key >= (2 << info->var.bits_per_pixel))
                {
                    MTFB_DEBUGK("The key :%d is out of range the palette: %d!\n",
                                ckey.u32Key, 2 << info->var.bits_per_pixel);
                    return MT_FAILURE;
                }

                par->stExtendInfo.stCkey.u8BlueMax  = par->stExtendInfo.stCkey.u8BlueMin = info->cmap.blue[ckey.u32Key];
                par->stExtendInfo.stCkey.u8GreenMax = par->stExtendInfo.stCkey.u8GreenMin = info->cmap.green[ckey.u32Key];
                par->stExtendInfo.stCkey.u8RedMax   = par->stExtendInfo.stCkey.u8RedMin = info->cmap.red[ckey.u32Key];
            }
            else
            {
                mt_u8 u8RMask, u8GMask, u8BMask;
                MTFB_LINE;
                s_stDrvOps.MTFB_DRV_ColorConvert(&info->var, &par->stExtendInfo.stCkey);

                u8BMask  = (0xff >> s_stArgbBitField[par->stExtendInfo.enColFmt].stBlue.length);
                u8GMask  = (0xff >> s_stArgbBitField[par->stExtendInfo.enColFmt].stGreen.length);
                u8RMask  = (0xff >> s_stArgbBitField[par->stExtendInfo.enColFmt].stRed.length);

                par->stExtendInfo.stCkey.u8BlueMin  = (par->stExtendInfo.stCkey.u32Key & (~u8BMask));
                par->stExtendInfo.stCkey.u8GreenMin = ((par->stExtendInfo.stCkey.u32Key >> 8) & (~u8GMask));
                par->stExtendInfo.stCkey.u8RedMin   = ((par->stExtendInfo.stCkey.u32Key >> 16) & (~u8RMask));

                par->stExtendInfo.stCkey.u8BlueMax  = par->stExtendInfo.stCkey.u8BlueMin | u8BMask;
                par->stExtendInfo.stCkey.u8GreenMax = par->stExtendInfo.stCkey.u8GreenMin | u8GMask;
                par->stExtendInfo.stCkey.u8RedMax   = par->stExtendInfo.stCkey.u8RedMin | u8RMask;
            }

			par->stExtendInfo.stCkey.u8RedMask   = 0xff;
			par->stExtendInfo.stCkey.u8BlueMask  = 0xff;
			par->stExtendInfo.stCkey.u8GreenMask = 0xff;

            par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_COLORKEY;
            par->stRunInfo.bModifying          = MT_FALSE;
            MTFB_LINE;
            break;
        }

        case FBIOPUT_SCREENSIZE:
        {
#ifndef CFG_MTFB_VIRTUAL_COORDINATE_SUPPORT
            MTFB_SIZE_S stScreenSize;
            MTFB_LINE;
            if (par->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
            {
                MTFB_WARNING("you shouldn't set cursor origion!");
                return MT_SUCCESS;
            }

            if (copy_from_user(&stScreenSize, argp, sizeof(MTFB_SIZE_S)))
            {
                return -EFAULT;
            }

            s32Ret = mtfb_disp_setscreensize(par->stBaseInfo.u32LayerID, stScreenSize.u32Width, stScreenSize.u32Height);

			if (MT_SUCCESS == s32Ret)
			{
				s_stDrvOps.MTFB_DRV_SetScreenFlag(par->stBaseInfo.u32LayerID, MT_TRUE);
			}
#endif
            break;
        }

        case FBIOGET_SCREENSIZE:
        {
#ifndef CFG_MTFB_VIRTUAL_COORDINATE_SUPPORT
            MTFB_SIZE_S stScreenSize;
            MTFB_LINE;
			 mtfb_wait_regconfig_work(par->stBaseInfo.u32LayerID);

			stScreenSize.u32Width  = par->stExtendInfo.u32ScreenWidth;
			stScreenSize.u32Height = par->stExtendInfo.u32ScreenHeight;

            if (copy_to_user(argp, &stScreenSize, sizeof(MTFB_SIZE_S)))
            {
                return -EFAULT;
            }
#else
            MTFB_SIZE_S stScreenSize;
            MTFB_RECT   stOutputRect;
            s_stDrvOps.MTFB_DRV_GetLayerOutRect(par->stBaseInfo.u32LayerID, &stOutputRect);
            stScreenSize.u32Width = stOutputRect.w;
            stScreenSize.u32Height = stOutputRect.h;
            if (copy_to_user(argp, &stScreenSize, sizeof(MTFB_SIZE_S)))
            {
                return -EFAULT;
            }
#endif
            break;
        }

        case FBIOGET_SCREEN_ORIGIN_MTFB:
        {
            MTFB_LINE;
            if (copy_to_user(argp, &par->stExtendInfo.stPos, sizeof(MTFB_POINT_S)))
            {
                return -EFAULT;
            }

            break;
        }

        case FBIOPUT_SCREEN_ORIGIN_MTFB:
        {
            MTFB_POINT_S origin;
		     MTFB_LINE;
            if (copy_from_user(&origin, argp, sizeof(MTFB_POINT_S)))
            {
                return -EFAULT;
            }
        #ifdef CONFIG_MTFB_CURSOR_LAYER_NEGATIVE_SUPPORT
			/** 鼠标层支持负坐标处理 **/
			if (par->stBaseInfo.u32LayerID != MTFB_LAYER_HD_3)
			{/** 非鼠标层不支持负坐标处理 **/
				if (origin.s32XPos < 0 || origin.s32YPos < 0)
		        {
		            MTFB_DEBUGK("It's not supported to set start pos of layer to negative!\n");
		            return MT_FAILURE;
		        }
			}
            par->stRunInfo.bModifying = MT_TRUE;
            par->stExtendInfo.stPos.s32XPos = origin.s32XPos;
            par->stExtendInfo.stPos.s32YPos = origin.s32YPos;
        #else
            u32LayerId = par->stBaseInfo.u32LayerID;
			if (par->stExtendInfo.enBufMode != MTFB_LAYER_BUF_NONE)
			{
				if (origin.s32XPos < 0 || origin.s32YPos < 0)
		        {
		            MTFB_DEBUGK("It's not supported to set start pos of layer to negative!\n");
		            return MT_FAILURE;
		        }
			}

			s_stDrvOps.MTFB_DRV_GetLayerOutRect(u32LayerId, &stOutputRect);

            par->stRunInfo.bModifying = MT_TRUE;
            par->stExtendInfo.stPos.s32XPos  = origin.s32XPos;
            par->stExtendInfo.stPos.s32YPos  = origin.s32YPos;

            if (origin.s32XPos > stOutputRect.w - MTFB_MIN_WIDTH(u32LayerId))
            {
                par->stExtendInfo.stPos.s32XPos = stOutputRect.w - MTFB_MIN_WIDTH(u32LayerId);
            }

            if (origin.s32YPos > stOutputRect.h - MTFB_MIN_HEIGHT(u32LayerId))
            {
                par->stExtendInfo.stPos.s32YPos = stOutputRect.h - MTFB_MIN_HEIGHT(u32LayerId);
            }

			if (origin.s32XPos < 0 || origin.s32YPos < 0)
			{
				mt_u32 u32XPos, u32YPos;

				u32Bpp = mtfb_getbppbyfmt(par->stDispInfo.stUserBuffer.stCanvas.enFmt);
				u32Addr= par->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr;
				if (origin.s32XPos < 0)
				{
					u32XPos = 0-origin.s32XPos;
					if (u32XPos > par->stDispInfo.stUserBuffer.stCanvas.u32Width)
					{
						u32XPos = par->stDispInfo.stUserBuffer.stCanvas.u32Width;
					}
					par->stExtendInfo.u32DisplayWidth = par->stDispInfo.stUserBuffer.stCanvas.u32Width-u32XPos;
					u32Addr +=  (u32XPos*u32Bpp/8);
					par->stExtendInfo.stPos.s32XPos = 0;
				}

				if (origin.s32YPos < 0)
				{
					u32YPos = 0-origin.s32YPos;
					if (u32YPos > par->stDispInfo.stUserBuffer.stCanvas.u32Height)
					{
						u32YPos = par->stDispInfo.stUserBuffer.stCanvas.u32Height;
					}
					par->stExtendInfo.u32DisplayHeight = par->stDispInfo.stUserBuffer.stCanvas.u32Height-u32YPos;
					u32Addr +=  par->stDispInfo.stUserBuffer.stCanvas.u32Pitch*u32YPos;
					par->stExtendInfo.stPos.s32YPos = 0;
				}

				par->stRunInfo.u32ScreenAddr = (u32Addr&0xfffffff0);
				par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
				par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
			}

			if (par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
			{
				if (origin.s32XPos >= 0)
				{
					par->stExtendInfo.u32DisplayWidth = par->stDispInfo.stUserBuffer.stCanvas.u32Width;
				}

				if (origin.s32YPos >= 0)
				{
					par->stExtendInfo.u32DisplayHeight= par->stDispInfo.stUserBuffer.stCanvas.u32Height;
				}

				if (origin.s32XPos >= 0 && origin.s32YPos >= 0)
				{
					par->stRunInfo.u32ScreenAddr = par->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr;
					par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
					par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_REFRESH;
				}

			}
		#endif
            par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_INRECT;
            par->stRunInfo.bModifying = MT_FALSE;

            break;
        }

        case FBIOGET_VBLANK_MTFB:
        {
            MTFB_LINE;
            if (s_stDrvOps.MTFB_DRV_WaitVBlank(par->stBaseInfo.u32LayerID) < 0)
            {
                MTFB_WARNING("It is not support VBL!\n");
                return -EPERM;
            }

            break;
        }

        case FBIOPUT_SHOW_MTFB:
        {
            MT_BOOL bShow;
            MTFB_LINE;
            if (par->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
            {
                MTFB_WARNING("you shouldn't show cursor by tmts cmd!");
                return MT_SUCCESS;
            }

            if (copy_from_user(&bShow, argp, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

            /* reset the same status */
            if (bShow == par->stExtendInfo.bShow)
            {
                MTFB_DEBUGK("The layer is show(%d) now!\n", par->stExtendInfo.bShow);
                return 0;
            }

            par->stRunInfo.bModifying          = MT_TRUE;
            par->stExtendInfo.bShow            = bShow;
            par->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_SHOW;
            par->stRunInfo.bModifying          = MT_FALSE;

            break;
        }

        case FBIOGET_SHOW_MTFB:
        {
            MTFB_LINE;
            if (copy_to_user(argp, &par->stExtendInfo.bShow, sizeof(MT_BOOL)))
            {
                return -EFAULT;
            }

            break;
        }

        case FBIO_WAITFOR_FREFRESH_DONE:
        {
            MTFB_LINE;
            if (par->stRunInfo.s32RefreshHandle
               && par->stExtendInfo.enBufMode != MTFB_LAYER_BUF_ONE)
            {
                s32Ret = s_stDrvTdeOps.MTFB_DRV_WaitForDone(par->stRunInfo.s32RefreshHandle, 1000);
                if (s32Ret < 0)
                {
                    MTFB_DEBUGK("MTFB_DRV_WaitForDone failed!ret=%x\n", s32Ret);
                    return MT_FAILURE;
                }
            }

            break;
        }

        case FBIOGET_CAPABILITY_MTFB:
        {
            MTFB_LINE;
            if (copy_to_user(argp, (mt_void *)&g_pstCap[par->stBaseInfo.u32LayerID], sizeof(MTFB_CAPABILITY_S)))
            {
                MTFB_DEBUGK("FBIOGET_CAPABILITY_MTFB error\n");
                return -EFAULT;
            }

            break;
        }
#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
        case FBIO_SCROLLTEXT_CREATE:
        {
            MTFB_SCROLLTEXT_CREATE_S stScrollText;
            MTFB_LINE;
            if (copy_from_user(&stScrollText, argp, sizeof(MTFB_SCROLLTEXT_CREATE_S)))
            {
                return -EFAULT;
            }
			/** 判断像素格式是否支持 **/
			if (stScrollText.stAttr.ePixelFmt >= MTFB_FMT_BUTT)
			{
				MTFB_DEBUGK("Invalid attributes.\n");
				return MT_FAILURE;
			}

			if (stScrollText.stAttr.stRect.w < 0 || stScrollText.stAttr.stRect.h < 0)
			{
				MTFB_DEBUGK("Invalid attributes.\n");
				return MT_FAILURE;
			}

			s32Ret = mtfb_create_scrolltext(par->stBaseInfo.u32LayerID, &stScrollText);
            if (MT_SUCCESS != s32Ret)
            {
                return -EFAULT;
            }

            return copy_to_user(argp, &stScrollText, sizeof(MTFB_SCROLLTEXT_CREATE_S));

        }
        case FBIO_SCROLLTEXT_FILL:
        {
            MTFB_SCROLLTEXT_DATA_S stScrollTextData;
            MTFB_LINE;
            if (copy_from_user(&stScrollTextData, argp, sizeof(MTFB_SCROLLTEXT_DATA_S)))
            {
                return -EFAULT;
            }

            if(    MT_NULL == stScrollTextData.u32PhyAddr
                && MT_NULL == stScrollTextData.pu8VirAddr)
            {
                MTFB_DEBUGK("invalid usr data!\n");
                return -EFAULT;
            }
			s32Ret = mtfb_fill_scrolltext(&stScrollTextData);
            if (MT_SUCCESS != s32Ret)
            {
                MTFB_DEBUGK("failed to fill data to scroll text !\n");
                return -EFAULT;
            }

            break;
        }
		case FBIO_SCROLLTEXT_DESTORY:
        {
            mt_u32 u32LayerId, u32ScrollTextID, u32Handle;
            MTFB_LINE;
            if (copy_from_user(&u32Handle, argp, sizeof(mt_u32)))
            {
                return -EFAULT;
            }

			s32Ret = mtfb_parse_scrolltexthandle(u32Handle,&u32LayerId,&u32ScrollTextID);
			if (MT_SUCCESS != s32Ret)
			{
				MTFB_DEBUGK("invalid scrolltext handle!\n");
                return -EFAULT;
			}

			s32Ret = mtfb_destroy_scrolltext(u32LayerId,u32ScrollTextID);
			if (MT_SUCCESS != s32Ret)
			{
				MTFB_DEBUGK("failed to destroy scrolltext!\n");
                return -EFAULT;
			}

            break;
        }
        case FBIO_SCROLLTEXT_PAUSE:
        {
            mt_u32 u32LayerId, u32ScrollTextID, u32Handle;
			 MTFB_SCROLLTEXT_S  *pstScrollText;
            MTFB_LINE;
            if (copy_from_user(&u32Handle, argp, sizeof(mt_u32)))
            {
                return -EFAULT;
            }

			s32Ret = mtfb_parse_scrolltexthandle(u32Handle,&u32LayerId,&u32ScrollTextID);
			if (MT_SUCCESS != s32Ret)
			{
				MTFB_DEBUGK("invalid scrolltext handle!\n");
                return -EFAULT;
			}

			pstScrollText = &(s_stTextLayer[u32LayerId].stScrollText[u32ScrollTextID]);
			pstScrollText->bPause = MT_TRUE;

            break;
        }
		case FBIO_SCROLLTEXT_RESUME:
        {
            mt_u32 u32LayerId, u32ScrollTextID, u32Handle;
			 MTFB_SCROLLTEXT_S  *pstScrollText;
            MTFB_LINE;
            if (copy_from_user(&u32Handle, argp, sizeof(mt_u32)))
            {
                return -EFAULT;
            }

			s32Ret = mtfb_parse_scrolltexthandle(u32Handle,&u32LayerId,&u32ScrollTextID);
			if (MT_SUCCESS != s32Ret)
			{
				MTFB_DEBUGK("invalid scrolltext handle!\n");
                return -EFAULT;
			}

			pstScrollText = &(s_stTextLayer[u32LayerId].stScrollText[u32ScrollTextID]);
			pstScrollText->bPause = MT_FALSE;

            break;
        }
#endif
		case FBIOGET_ZORDER:
		{
			mt_u32  u32Zorder;
           MTFB_LINE;
			 s_stDrvOps.MTFB_DRV_GetLayerPriority(par->stBaseInfo.u32LayerID, &u32Zorder);
            return copy_to_user(argp, &(u32Zorder), sizeof(mt_u32));
		}
		case FBIOPUT_ZORDER:
		{
            MTFB_ZORDER_E enZorder;
             MTFB_LINE;
            if (copy_from_user(&enZorder, argp, sizeof(MTFB_ZORDER_E)))
            {
                return -EFAULT;
            }

			if (enZorder >= MTFB_ZORDER_BUTT)
			{
				MTFB_DEBUGK("invalid operation.\n");
				return MT_FAILURE;
			}

			s_stDrvOps.MTFB_DRV_SetLayerPriority(par->stBaseInfo.u32LayerID, enZorder);
			break;
		}
#ifdef CFG_MTFB_LOGO_SUPPORT
        case FBIO_FREE_LOGO:
        {
            MTFB_LINE;
            mtfb_clear_logo(par->stBaseInfo.u32LayerID, MT_FALSE);
            break;
        }
#endif
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	    case FBIOPUT_STEREO_DEPTH:
        {
			mt_s32 s32StereoDepth;
          MTFB_LINE;
			if (copy_from_user(&s32StereoDepth, argp, sizeof(mt_s32)))
            {
                return -EFAULT;
            }

			if (!par->bSetStereoMode)
			{
				MTFB_DEBUGK("u need to set disp stereo mode first.\n");
				return MT_FAILURE;
			}

			s_stDrvOps.MTFB_DRV_SetStereoDepth(par->stBaseInfo.u32LayerID, s32StereoDepth);

			par->st3DInfo.s32StereoDepth = s32StereoDepth;

            break;
        }

		case FBIOGET_STEREO_DEPTH:
        {
           MTFB_LINE;
			if (!par->bSetStereoMode)
			{
				MTFB_DEBUGK("u need to set disp stereo mode first.\n");
				return MT_FAILURE;
			}

			if (copy_to_user(argp, &(par->st3DInfo.s32StereoDepth), sizeof(mt_s32)))
            {
                return -EFAULT;
            }

            break;
        }
#endif
#ifdef CONFIG_DMA_SHARED_BUFFER
        case FBIOGET_DMABUF:
        {
            struct dma_buf *dma_buf = NULL;
            struct fb_dmabuf_export buf = {0};            
            int fd = -1;
            dma_buf = mtfb_memblock_export(info->fix.smem_start, info->fix.smem_len, O_CLOEXEC | O_RDWR);
            /* get fd for buf */
            fd = dma_buf_fd(dma_buf, O_CLOEXEC);
            buf.fd = fd;
            buf.flags = 0;
            if (copy_to_user(argp, &buf, sizeof(struct fb_dmabuf_export)))
            {
                return -EFAULT;
            }
            break;
        }
#endif

        default:
        {
            MTFB_LINE;
            MTFB_DEBUGK("the command:0x%x is unsupported!\n", cmd);
            return -EINVAL;
        }
    }

    MTFB_FUN_OUT;
    return s32Ret;
}

static mt_void mtfb_3DMode_callback(mt_void * pParaml,mt_void * pParamr)
{
#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
	mt_u32 *pu32LayerId;
	MTFB_STEREO_MODE_E *penStereoMode;
	struct fb_info *info;
    MTFB_PAR_S *pstPar;

    MTFB_FUN_IN;
	pu32LayerId   = (mt_u32 *)pParaml;
	penStereoMode = (MTFB_STEREO_MODE_E *)pParamr;

	info   = s_stLayer[*pu32LayerId].pstInfo;
	pstPar = (MTFB_PAR_S *)(info->par);

	if (MTFB_STEREO_MONO == *penStereoMode)
	{
		pstPar->bSetStereoMode = MT_FALSE;
		pstPar->st3DInfo.enInStereoMode  = MTFB_STEREO_MONO;
        if (pstPar->st3DInfo.st3DMemInfo.u32StereoMemStart)
        {
            /** free old buffer*/
            MTFB_DEBUGK("free old stereo buffer\n");
            mtfb_freestereobuf(pstPar);
        }
	}
	else
	{
		pstPar->bSetStereoMode = MT_TRUE;
		s_stDrvOps.MTFB_DRV_SetTriDimAddr(*pu32LayerId, pstPar->st3DInfo.u32rightEyeAddr);
	}

	pstPar->st3DInfo.enOutStereoMode = *penStereoMode;
	s_stDrvOps.MTFB_DRV_SetTriDimMode(*pu32LayerId, *penStereoMode);

	/**
	 ** these parameters will take effect after refresh
	 **/
    pstPar->stRunInfo.bModifying          = MT_TRUE;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_STRIDE;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_INRECT;
	pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_DISPLAYADDR;
	pstPar->stRunInfo.bModifying          = MT_FALSE;

    mtfb_assign_dispbuf(pstPar->stBaseInfo.u32LayerID);
    mtfb_clearallstereobuf(info);

    if (MTFB_LAYER_BUF_STANDARD == pstPar->stExtendInfo.enBufMode)
	{
    	mtfb_pan_display(&info->var, info);
	}
	else
	{
		mtfb_refreshuserbuffer(pstPar->stBaseInfo.u32LayerID);
	}
#endif

   MTFB_FUN_OUT;
	return;

}


/***************************************************************************************
* func          : mtfb_layer_init
* description   : CNcomment: 图层初始化 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_layer_init(mt_u32 u32LayerID)
{
	struct fb_info *info;
    MTFB_PAR_S *par;
	MTFB_COLOR_FMT_E enColorFmt;
	MTFB_RECT stInRect;
    mt_u32 pixStride = 0;

    MTFB_FUN_IN;
	info   = s_stLayer[u32LayerID].pstInfo;
	par = (MTFB_PAR_S *)(info->par);

	if (IS_HD_LAYER(u32LayerID))
    {
        info->var = s_stDefVar[MTFB_LAYER_TYPE_HD];
    }
    else if (IS_SD_LAYER(u32LayerID))
    {
        info->var = s_stDefVar[MTFB_LAYER_TYPE_SD];
    }
    else if(  IS_AD_LAYER(u32LayerID)
		    || IS_MINOR_HD_LAYER(u32LayerID)
			|| IS_MINOR_SD_LAYER(u32LayerID))
    {/** G1G2G5是给AD使用的 **/
        info->var = s_stDefVar[MTFB_LAYER_TYPE_AD];
    }
    else
    {
       MTFB_FUN_OUT;
		return MT_FAILURE;
    }

	enColorFmt = mtfb_getfmtbyargb(&info->var.red, &info->var.green, &info->var.blue, &info->var.transp, info->var.bits_per_pixel);

	memset(&(par->stDispInfo.stUserBuffer), 0, sizeof(MTFB_BUFFER_S));
	memset(&(par->stDispInfo.stCanvasSur),  0, sizeof(MTFB_SURFACE_S));

	/**
	 **自动选择抗闪
	 **/
	par->stBaseInfo.bNeedAntiflicker = MT_FALSE;
	mtfb_disp_setantiflickerlevel(par->stBaseInfo.u32LayerID, MTFB_LAYER_ANTIFLICKER_AUTO);


	par->stRunInfo.bModifying = MT_FALSE;

	par->stRunInfo.u32ParamModifyMask = 0;

	info->var.xoffset = 0;
	info->var.yoffset = 0;

	par->stExtendInfo.stAlpha.bAlphaEnable  = MT_TRUE;
	par->stExtendInfo.stAlpha.bAlphaChannel = MT_FALSE;
	par->stExtendInfo.stAlpha.u8Alpha0      = MTFB_ALPHA_TRANSPARENT;
	par->stExtendInfo.stAlpha.u8Alpha1      = MTFB_ALPHA_OPAQUE;
	par->stExtendInfo.stAlpha.u8GlobalAlpha = MTFB_ALPHA_OPAQUE;
	s_stDrvOps.MTFB_DRV_SetLayerAlpha(par->stBaseInfo.u32LayerID, &par->stExtendInfo.stAlpha);

	memset(&(par->stExtendInfo.stCkey), 0, sizeof(MTFB_COLORKEYEX_S));
	par->stExtendInfo.stCkey.u8RedMask   = 0xff;
	par->stExtendInfo.stCkey.u8GreenMask = 0xff;
	par->stExtendInfo.stCkey.u8BlueMask  = 0xff;
	s_stDrvOps.MTFB_DRV_SetLayerKeyMask(par->stBaseInfo.u32LayerID, &par->stExtendInfo.stCkey);

	par->stExtendInfo.enColFmt = enColorFmt;
	s_stDrvOps.MTFB_DRV_SetLayerDataFmt(par->stBaseInfo.u32LayerID, par->stExtendInfo.enColFmt);

	memset(&par->stExtendInfo.stPos, 0, sizeof(MTFB_POINT_S));

	info->fix.line_length = info->var.xres_virtual * (info->var.bits_per_pixel >> 3);

	par->stExtendInfo.u32DisplayWidth       = info->var.xres;
	par->stExtendInfo.u32DisplayHeight      = info->var.yres;

	par->st3DInfo.st3DSurface.u32Pitch      = info->fix.line_length;
	par->st3DInfo.st3DSurface.enFmt         = par->stExtendInfo.enColFmt;
	par->st3DInfo.st3DSurface.u32Width      = info->var.xres;
	par->st3DInfo.st3DSurface.u32Height     = info->var.yres;
	par->st3DInfo.st3DMemInfo.u32StereoMemLen   = MT_NULL;
	par->st3DInfo.st3DMemInfo.u32StereoMemStart = MT_NULL;

	stInRect.x = 0;
	stInRect.y = 0;
	stInRect.w = info->var.xres;
	stInRect.h = info->var.yres;
	/**
	 **set layer's inrect the same as outrect when initial
	 **/
	s_stDrvOps.MTFB_DRV_SetLayerInRect(par->stBaseInfo.u32LayerID, &stInRect);
   pixStride = calc_pixel_stride_by_pitch_fmt(info->fix.line_length, par->st3DInfo.st3DSurface.enFmt, __LINE__);
	//s_stDrvOps.MTFB_DRV_SetLayerStride(par->stBaseInfo.u32LayerID, info->fix.line_length);
   s_stDrvOps.MTFB_DRV_SetLayerStride(par->stBaseInfo.u32LayerID, pixStride);

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
    mutex_init(&par->st3DInfo.st3DMemInfo.stStereoMemLock);
	s_stDrvOps.MTFB_DRV_SetTriDimMode(par->stBaseInfo.u32LayerID, MTFB_STEREO_MONO);
#endif

	/**
	 **标准刷新
	 **/
	par->stExtendInfo.enBufMode               = MTFB_LAYER_BUF_STANDARD;

	par->stRunInfo.u32BufNum = MTFB_MAX_FLIPBUF_NUM;

	par->bPanFlag  = MT_FALSE;
	par->bPanReady = MT_TRUE;
	par->bSetVar   = MT_FALSE;
	spin_lock_init(&par->stBaseInfo.lock);

    MTFB_FUN_OUT;
	return MT_SUCCESS;
}



/***************************************************************************************
* func          : mtfb_createproc
* description   : CNcomment: 创建proc CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#ifdef CFG_MTFB_PROC_SUPPORT
static mt_s32 mtfb_createproc(MTFB_LAYER_ID_E enLayerID)
{
    MTFB_PAR_S *par;
	mt_char entry_name[16];
	GFX_PROC_ITEM_S item;
	struct fb_info *info;

	info = s_stLayer[enLayerID].pstInfo;
	par  = (MTFB_PAR_S *)(info->par);

	if (par->stProcInfo.bCreatedProc)
	{
	    return MT_FAILURE;
	}

	/* create a proc entry in 'mtfb' for the layer */
	snprintf(entry_name, sizeof(entry_name), "mtfb%d", enLayerID);
	entry_name[sizeof(entry_name) - 1] = '\0';
	item.fnRead   = mtfb_read_proc;
	item.fnWrite  = mtfb_write_proc;
	item.fnIoctl  = MT_NULL;
	MT_GFX_PROC_AddModule(entry_name, &item, (mt_void *)s_stLayer[enLayerID].pstInfo);
    MTFB_DEBUGK("success to create %s proc!\n", entry_name);
	par->stProcInfo.bCreatedProc = MT_TRUE;
	par->stProcInfo.bWbcProc     = MT_FALSE;
	par->stProcInfo.enWbcLayerID = MTFB_LAYER_ID_BUTT;

    return MT_SUCCESS;

}

/***************************************************************************************
* func          : mtfb_createwbcproc
* description   : CNcomment: 创建wbc proc CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_createwbcproc(MTFB_PAR_S *pMasterpar)
{
	MTFB_PAR_S *par;
	struct fb_info *info;
	mt_char entry_name[16];
	GFX_PROC_ITEM_S item;
	MTFB_SLVLAYER_DATA_S stLayerInfo;

	mt_s32 s32Ret;

	s32Ret = s_stDrvOps.MTFB_DRV_GetSlvLayerInfo(&stLayerInfo);

	if(MT_FAILURE == s32Ret)
	{
	    return MT_FAILURE;
	}

	if (stLayerInfo.enLayerID >= MTFB_LAYER_ID_BUTT)
	{
	    return MT_FAILURE;
	}

	info = s_stLayer[stLayerInfo.enLayerID].pstInfo;
	par  = (MTFB_PAR_S *)(info->par);

	pMasterpar->stProcInfo.enWbcLayerID = stLayerInfo.enLayerID;
	par->stProcInfo.u32MasterLayerNum++;
	MTFB_DEBUGK("create wbc proc, master layerid %d, masterlayernum %d\n",
		   pMasterpar->stBaseInfo.u32LayerID, par->stProcInfo.u32MasterLayerNum);

	if (par->stProcInfo.bCreatedProc)
	{
	    return MT_SUCCESS;
	}

	/* create a proc entry in 'mtfb' for the layer */
	snprintf(entry_name, sizeof(entry_name), "mtfb%d", stLayerInfo.enLayerID);
	item.fnRead = mtfb_read_proc;
	item.fnWrite= mtfb_write_proc;
	item.fnIoctl= MT_NULL;
	MT_GFX_PROC_AddModule(entry_name, &item, (mt_void *)s_stLayer[stLayerInfo.enLayerID].pstInfo);
	MTFB_DEBUGK("success to create %s proc!\n", entry_name);

	par->stProcInfo.bCreatedProc   = MT_TRUE;
	par->stProcInfo.bWbcProc       = MT_TRUE;

    return MT_SUCCESS;

}

static mt_s32 mtfb_removewbcproc(MTFB_LAYER_ID_E enWbcLayerID)
{
    MTFB_PAR_S *par;
	struct fb_info *info;
	mt_char entry_name[16];

	info = s_stLayer[enWbcLayerID].pstInfo;
	par  = (MTFB_PAR_S *)(info->par);

	if (!par->stProcInfo.bWbcProc)
	{
	    return MT_FAILURE;
	}

    snprintf(entry_name, sizeof(entry_name), "mtfb%d", enWbcLayerID);
    MT_GFX_PROC_RemoveModule(entry_name);
	MTFB_DEBUGK("success to remove mtfb%d proc!\n", enWbcLayerID);

	par->stProcInfo.bCreatedProc = MT_FALSE;
	par->stProcInfo.bWbcProc     = MT_FALSE;
	par->stProcInfo.u32MasterLayerNum = 0;

    return MT_SUCCESS;
}
#endif

#ifdef CFG_MTFB_LOGO_SUPPORT

/***************************************************************************************
* func          : mtfb_convertbootfmt2fbfmt
* description   : CNcomment: 将boot的像素格式转成HiFB的像素格式 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
#define MTFB_BOOT_FMT_1555 5
#define MTFB_BOOT_FMT_8888 9
static MTFB_COLOR_FMT_E mtfb_convertbootfmt2fbfmt(mt_u32 enBootFmt)
{
	switch(enBootFmt)
	{
		case MTFB_BOOT_FMT_1555:
			return MTFB_FMT_ARGB1555;
		case MTFB_BOOT_FMT_8888:
			return MTFB_FMT_ARGB8888;
		default:
			return MTFB_FMT_BUTT;
	}
}
/***************************************************************************************
* func          : mtfb_convertlogochn2dispchn
* description   : CNcomment: 将logo通道转成disp通道 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static MT_UNF_DISP_E mtfb_convertlogochn2dispchn(MTFB_LOGO_CHANNEL_E enLogoChn)
{
	if (enLogoChn >= MTFB_LOGO_CHN_BUTT)
	{
		return MT_UNF_DISPLAY_BUTT;
	}
	switch(enLogoChn)
	{
		case MTFB_LOGO_CHN_HD:
			return MT_UNF_DISPLAY1;
		case MTFB_LOGO_CHN_SD:
			return MT_UNF_DISPLAY0;
		default:
			return MT_UNF_DISPLAY_BUTT;
	}
}

/***************************************************************************************
* func          : mtfb_set_logosd
* description   : CNcomment: 设置logo数据信息 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_set_logosd(mt_u32 u32LayerID)
{
	mt_s32 s32Ret;
    MTFB_PAR_S *par;
	struct fb_info *info;
	MTFB_LOGO_CHANNEL_E enLogoChn;
	MTFB_RECT         stInRect;
	MTFB_COLOR_FMT_E  enHifbFmt;
	MT_DISP_PARAM_S   stDispParam;
	PDM_EXPORT_FUNC_S *pstPdmFuncs  = MT_NULL;
   mt_u32 pixStride = 0;
	info   = s_stLayer[u32LayerID].pstInfo;
	par = (MTFB_PAR_S *)(info->par);

	if (IS_HD_LAYER(u32LayerID) || IS_MINOR_HD_LAYER(u32LayerID))
	{
		enLogoChn = MTFB_LOGO_CHN_HD;
	}
	else if(IS_SD_LAYER(u32LayerID) || IS_MINOR_SD_LAYER(u32LayerID))
	{
		enLogoChn = MTFB_LOGO_CHN_SD;
	}
	else
	{
		return MT_SUCCESS;
	}
#if 0
	if (g_sLogo.stLogoInfo[enLogoChn].bTransitted)
	{
		return MT_SUCCESS;
	}
#endif
    s32Ret = MT_DRV_MODULE_GetFunction(MT_ID_PDM, (mt_void**)&pstPdmFuncs);
    if(   s32Ret == MT_FAILURE
		|| NULL == pstPdmFuncs
		|| NULL == pstPdmFuncs->pfnPDM_GetDispParam)
    {
        MTFB_WARNING("get pdm module function failed\r\n");
		s32Ret = MT_FAILURE;
		goto FINISHED;
    }
	s32Ret = pstPdmFuncs->pfnPDM_GetDispParam(mtfb_convertlogochn2dispchn(enLogoChn), &stDispParam);
	if (s32Ret == MT_FAILURE)
    {/** 没有开机logo的情况下这里就没有参数了，直接退出 **/
        MTFB_WARNING("PDM_GetDispParam failed\r\n");
		goto FINISHED;
    }

	if (   stDispParam.u32VirtScreenWidth == 0
		|| stDispParam.u32VirtScreenHeight == 0)
	{
		s32Ret = MT_FAILURE;
		goto FINISHED;
	}

	enHifbFmt = mtfb_convertbootfmt2fbfmt((mt_u32)stDispParam.enPixelFormat);
	if (enHifbFmt >= MTFB_FMT_PUYVY)
	{
		s32Ret = MT_FAILURE;
		goto FINISHED;
	}

	info->var.bits_per_pixel = mtfb_getbppbyfmt(enHifbFmt);
	if (info->var.bits_per_pixel == 0)
	{
		MTFB_WARNING("unsupported fmt received from boot!\n");
		s32Ret = MT_FAILURE;
		goto FINISHED;
	}

	info->var.red    = s_stArgbBitField[enHifbFmt].stRed;
    info->var.green  = s_stArgbBitField[enHifbFmt].stGreen;
    info->var.blue   = s_stArgbBitField[enHifbFmt].stBlue;
    info->var.transp = s_stArgbBitField[enHifbFmt].stTransp;

	info->var.xres = stDispParam.u32VirtScreenWidth;
	info->var.yres = stDispParam.u32VirtScreenHeight;
	info->var.xres_virtual = info->var.xres;
	info->var.yres_virtual = info->var.yres;
	info->fix.line_length  = ((((info->var.xres_virtual * info->var.bits_per_pixel) >> 3) + 0xf) & 0xfffffff0);

    par->stExtendInfo.enColFmt         = enHifbFmt;
	par->stExtendInfo.stPos.s32XPos    = 0;
	par->stExtendInfo.stPos.s32YPos    = 0;
	par->stExtendInfo.u32DisplayWidth  = info->var.xres;
	par->stExtendInfo.u32DisplayHeight = info->var.yres;

	stInRect.x = par->stExtendInfo.stPos.s32XPos;
	stInRect.y = par->stExtendInfo.stPos.s32YPos;
	stInRect.w = par->stExtendInfo.u32DisplayWidth;
	stInRect.h = par->stExtendInfo.u32DisplayHeight;

	/**
	 ** 更新的几个参数
	 **/
    s_stDrvOps.MTFB_DRV_SetLayerInRect (u32LayerID, &stInRect);
    pixStride = calc_pixel_stride_by_pitch_fmt(info->fix.line_length, par->stExtendInfo.enColFmt, __LINE__);
	//s_stDrvOps.MTFB_DRV_SetLayerStride (u32LayerID, info->fix.line_length);
	s_stDrvOps.MTFB_DRV_SetLayerStride (u32LayerID, pixStride);    // changed by Jack
	s_stDrvOps.MTFB_DRV_SetLayerDataFmt(u32LayerID, par->stExtendInfo.enColFmt);

	MTFB_DEBUGK("<<<<<<<<<<<<<get logo data width %d, height %d>>>>>>>>>>>>>>>>>\n", info->var.xres, info->var.yres);

	s32Ret = MT_SUCCESS;

FINISHED:
	g_sLogo.stLogoInfo[enLogoChn].bTransitted = MT_TRUE;
	return s32Ret;
}
#endif


/***************************************************************************************
* func          : mtfb_sync_init
* description   : CNcomment: fence同步 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_void mtfb_sync_init(mt_u32 u32LayerID)
{
#ifdef CFG_MTFB_FENCE_SUPPORT

    MTFB_FUN_IN;

    /**
	 **android只用fb0,而且只在android里调用
	 **/
//    if (u32LayerID == MTFB_LAYER_HD_0)
    {
    	/** 这个不在这里初始化，这里只是赋个初值 **/
        atomic_set(&s_SyncInfo.s32RefreshCnt, 0);
        s_SyncInfo.u32FenceValue = 1;
        s_SyncInfo.u32Timeline   = 0;
        s_SyncInfo.FrameEndFlag  = 0;
		/**
		 **初始化帧结束中断
		 **/
        init_waitqueue_head(&s_SyncInfo.FrameEndEvent);
		/**
		 **创建mtfb流程的时间轴，每个流程都有自己的时间轴
		 **/
        s_SyncInfo.pstTimeline = sw_sync_timeline_create("mtfb");
    }
#endif
    MTFB_FUN_OUT;
    return;
}

static mt_void mtfb_sync_deinit(mt_u32 u32LayerID)
{
    MTFB_FUN_IN;
#ifdef CFG_MTFB_FENCE_SUPPORT
    if (s_SyncInfo.pstTimeline && u32LayerID == MTFB_LAYER_HD_0)
    {
        sync_timeline_destroy((struct sync_timeline*)s_SyncInfo.pstTimeline);
        s_SyncInfo.pstTimeline = NULL;
    }
#endif
    MTFB_FUN_OUT;
    return;
}


/***************************************************************************************
* func          : mtfb_open
* description   : CNcomment: 打开设备 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_open (struct fb_info *info, mt_s32 user)
{
    mt_s32 cnt;
    mt_s32 s32Ret;
	mt_u32 u32InSize = 0;
	MTFB_PAR_S *par;
	mt_u32 u32BufSize, u32Pitch;
	par = (MTFB_PAR_S *)info->par;

   MTFB_FUN_IN;
	/** 几个进程同时操作的时候，打开过了就不再调用 **/
	cnt = atomic_read(&par->stBaseInfo.ref_count);

	/**
	 **初始化的时候对每个fb设备都设置了par->stBaseInfo.u32LayerID
	 **/
    if (!g_pstCap[par->stBaseInfo.u32LayerID].bLayerSupported)
    {
        MTFB_DEBUGK("gfx%d is not supported!\n", par->stBaseInfo.u32LayerID);
        MTFB_FUN_OUT;
        return MT_FAILURE;
    }


    /****************** open the layer first time **************/
    if (!cnt)
    {/**
      **第一次打开设备
      **/
	    /**
	     **获取TDE导出函数
	     **/
		s32Ret = s_stDrvTdeOps.MTFB_DRV_TdeOpen();
		if (s32Ret != MT_SUCCESS)
        {
            MTFB_DEBUGK("tde was not avaliable!\n");
        }

		/**
      	 **打开图层
         **/
        s32Ret = s_stDrvOps.MTFB_DRV_OpenLayer(par->stBaseInfo.u32LayerID, 0);
        if (s32Ret != MT_SUCCESS)
        {
            MTFB_DEBUGK("failed to open layer%d !\n", par->stBaseInfo.u32LayerID);
            MTFB_FUN_OUT;
            return s32Ret;
        }

		/** fence同步 **/
        mtfb_sync_init(par->stBaseInfo.u32LayerID);

		/***********layer parameters initial***************/
		mtfb_layer_init(par->stBaseInfo.u32LayerID);

#ifdef CFG_MTFB_LOGO_SUPPORT
		/***********config layer with osd data***************/
       	/**
	 	 **base参数配置
	     **/
		mtfb_set_logosd(par->stBaseInfo.u32LayerID);
#endif

		/***********alloc disp buffer, set disp address******/
		u32InSize = (info->var.xres_virtual * info->var.bits_per_pixel) >> 3;
		MT_MTFB_GetStride(u32InSize,&u32Pitch,CONFIG_MTFB_STRIDE_16ALIGN);

		u32BufSize  = info->var.yres_virtual * u32Pitch;
		if(u32BufSize < dfb_mem)
			u32BufSize = dfb_mem;
		MT_INFO_MTFB("~~~~~~~~~~~~~dfb_mem:0x%08x", dfb_mem);
        if(info->fix.smem_len < u32BufSize)
        {
			mtfb_realloc_layermem(info, u32BufSize);
		}

		/**
		 **使用第几块内存
		 **/
		par->stRunInfo.u32IndexForInt = 0;

		mtfb_assign_dispbuf(par->stBaseInfo.u32LayerID);

		/**
		 ** clear fb memory if it's the first time to open layer
		 **/
		memset(info->screen_base, 0, info->fix.smem_len);
        mt_dcache_flush_all();
        msleep(1);        
		s_stDrvOps.MTFB_DRV_SetLayerAddr(par->stBaseInfo.u32LayerID, info->fix.smem_start);
		par->stRunInfo.u32ScreenAddr  = info->fix.smem_start;
		par->st3DInfo.u32rightEyeAddr = par->stRunInfo.u32ScreenAddr;

		/***********set callback function to hard ware*********/
		s32Ret = s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_VO, (IntCallBack)mtfb_vo_callback, par->stBaseInfo.u32LayerID);
		if (s32Ret != MT_SUCCESS)
		{
			MTFB_DEBUGK("failed to set vo callback function, open layer%d failure\n", par->stBaseInfo.u32LayerID);
          MTFB_FUN_OUT;
			return s32Ret;
		}

		s32Ret = s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_3DMode_CHG, (IntCallBack)mtfb_3DMode_callback, par->stBaseInfo.u32LayerID);
		if (s32Ret != MT_SUCCESS)
		{
			MTFB_DEBUGK("failed to set stereo mode change callback function, open layer%d failure\n", par->stBaseInfo.u32LayerID);
           MTFB_FUN_OUT;
			return s32Ret;
		}

#ifdef CFG_MTFB_FENCE_SUPPORT

//        if (par->stBaseInfo.u32LayerID == MTFB_LAYER_HD_0)
        {
    	//	s32Ret = s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_FRAME_END, (IntCallBack)mtfb_frame_end_callback, par->stBaseInfo.u32LayerID);
			s32Ret = s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_FRAME_END, (IntCallBack)mtfb_frame_end_callback, MTFB_LAYER_HD_0);
    		if (s32Ret != MT_SUCCESS)
    		{
    			MTFB_DEBUGK("failed to set frame end callback function, open layer%d failure\n", par->stBaseInfo.u32LayerID);
              MTFB_FUN_OUT;
    			return s32Ret;
    		}
        }
#endif

#ifdef CFG_MTFB_PROC_SUPPORT

		if (MT_SUCCESS == mtfb_createproc(par->stBaseInfo.u32LayerID))
		{
		    mtfb_createwbcproc(par);
		}
#endif

        s_stDrvOps.MTFB_DRV_EnableLayer(par->stBaseInfo.u32LayerID, MT_TRUE);

        par->stExtendInfo.bShow = MT_TRUE;
        par->bVblank            = MT_TRUE;
    }

    /* increase reference count */
    atomic_inc(&par->stBaseInfo.ref_count);
    par->stExtendInfo.bOpen = MT_TRUE;

    MTFB_FUN_OUT;
    return MT_SUCCESS;

}


/***************************************************************************************
* func          : mtfb_release
* description   : CNcomment: 关闭设备 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_release (struct fb_info *info, mt_s32 user)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;
    mt_u32 cnt = atomic_read(&par->stBaseInfo.ref_count);

    MTFB_FUN_IN;
    if (!cnt)
    {
        MTFB_FUN_OUT;
        return -EINVAL;
    }

    /**
     ** only one user
     **/
    if (cnt == 1 )
    {
#ifdef CFG_MTFB_PROC_SUPPORT
        mt_char entry_name[16];
#endif
        par->stExtendInfo.bShow = MT_FALSE;
        if (par->stBaseInfo.u32LayerID != MTFB_LAYER_CURSOR)
        {
            s_stDrvOps.MTFB_DRV_EnableLayer(par->stBaseInfo.u32LayerID, MT_FALSE);
            s_stDrvOps.MTFB_DRV_UpdataLayerReg(par->stBaseInfo.u32LayerID);

			/*************unRegister callback function************************/
	        s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_VO,         MT_NULL, par->stBaseInfo.u32LayerID);
			s_stDrvOps.MTFB_DRV_SetIntCallback(MTFB_CALLBACK_TYPE_3DMode_CHG, MT_NULL, par->stBaseInfo.u32LayerID);

            memset(info->screen_base, 0, info->fix.smem_len);
			/**
			 **free canvas buffer
			 **/
			mtfb_freeccanbuf(par);

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
			if (s_stTextLayer[par->stBaseInfo.u32LayerID].bAvailable)
			{
			    mt_u32 i;
				for (i = 0; i < SCROLLTEXT_NUM; i++)
				{
				    if (s_stTextLayer[par->stBaseInfo.u32LayerID].stScrollText[i].bAvailable)
				    {
				        mtfb_freescrolltext_cachebuf(&(s_stTextLayer[par->stBaseInfo.u32LayerID].stScrollText[i]));
						memset(&s_stTextLayer[par->stBaseInfo.u32LayerID].stScrollText[i],0,sizeof(MTFB_SCROLLTEXT_S));
				    }
				}
				s_stTextLayer[par->stBaseInfo.u32LayerID].bAvailable = MT_FALSE;
				s_stTextLayer[par->stBaseInfo.u32LayerID].u32textnum = 0;
				s_stTextLayer[par->stBaseInfo.u32LayerID].u32ScrollTextId = 0;
			}
#endif

#ifdef CFG_MTFB_STEREO3D_HW_SUPPORT
            mtfb_freestereobuf(par);
            par->st3DInfo.enInStereoMode  = MTFB_STEREO_MONO;
			par->st3DInfo.enOutStereoMode = MTFB_STEREO_MONO;
            par->bSetStereoMode           = MT_FALSE;

			s_stDrvOps.MTFB_DRV_SetTriDimMode(par->stBaseInfo.u32LayerID, MTFB_STEREO_MONO);
			s_stDrvOps.MTFB_DRV_SetTriDimAddr(par->stBaseInfo.u32LayerID, MT_NULL);
			s_stDrvOps.MTFB_DRV_SetLayerAddr (par->stBaseInfo.u32LayerID, MT_NULL);
#endif
            s_stDrvOps.MTFB_DRV_CloseLayer(par->stBaseInfo.u32LayerID);
        }

#ifdef CFG_MTFB_PROC_SUPPORT
        /* remove a proc entry in 'mtfb' for the layer */
        if (par->stProcInfo.bCreatedProc)
        {
            MTFB_PAR_S *WbcLayerPar;
            struct fb_info *WbcLayerInfo;
            MTFB_LAYER_ID_E enWbcLayerID;

            snprintf(entry_name, sizeof(entry_name), "mtfb%d", par->stBaseInfo.u32LayerID);
            MT_GFX_PROC_RemoveModule(entry_name);
			par->stProcInfo.bCreatedProc = MT_FALSE;
			MTFB_DEBUGK("success to remove %s proc!\n", entry_name);

			enWbcLayerID = par->stProcInfo.enWbcLayerID;
			if (enWbcLayerID < MTFB_LAYER_ID_BUTT)
			{
			    WbcLayerInfo = s_stLayer[enWbcLayerID].pstInfo;
		        WbcLayerPar  = (MTFB_PAR_S *)(WbcLayerInfo->par);
				WbcLayerPar->stProcInfo.u32MasterLayerNum--;
				if (0 == WbcLayerPar->stProcInfo.u32MasterLayerNum)
				{
				    mtfb_removewbcproc(enWbcLayerID);
				}
			}
        }
#endif
      mtfb_sync_deinit(par->stBaseInfo.u32LayerID);
      par->stExtendInfo.bOpen = MT_FALSE;
    }

#ifdef CFG_MTFB_LOGO_SUPPORT
    mtfb_clear_logo(par->stBaseInfo.u32LayerID, MT_FALSE);
#endif

    /* decrease the reference count */
    atomic_dec(&par->stBaseInfo.ref_count);

    MTFB_FUN_OUT;
    return 0;
}


static mt_s32 mtfb_dosetcolreg(unsigned regno, unsigned red, unsigned green,
                          unsigned blue, unsigned transp, struct fb_info *info, MT_BOOL bUpdateReg)
{
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;
    //mt_u32 *pCmap;

    mt_u32 argb = ((transp & 0xff) << 24) | ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);

    if (regno > 255)
    {
        MTFB_WARNING("regno: %d, larger than 255!\n", regno);
        return MT_FAILURE;
    }

    s_stDrvOps.MTFB_DRV_SetColorReg(par->stBaseInfo.u32LayerID, regno, argb, bUpdateReg);
    return MT_SUCCESS;
}


/***************************************************************************************
* func          : _setcolreg
* description   : CNcomment: 设置调色板信息 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 _setcolreg(unsigned regno, unsigned red, unsigned green,
                          unsigned blue, unsigned transp, struct fb_info *info)
{
    mt_s32 Ret = MT_SUCCESS;

    MTFB_FUN_IN;
    switch (info->var.bits_per_pixel)
    {
        case 8:
            Ret = mtfb_dosetcolreg(regno, red, green, blue, transp, info, MT_TRUE);
            break;
        case 16:
            /*ACLUT88*/
            if (  (mtfb_bitfieldcmp(info->var.red, s_stArgbBitField[28].stRed)        == 0)
                && (mtfb_bitfieldcmp(info->var.green, s_stArgbBitField[28].stGreen)   == 0)
                && (mtfb_bitfieldcmp(info->var.blue, s_stArgbBitField[28].stBlue)     == 0)
                && (mtfb_bitfieldcmp(info->var.transp, s_stArgbBitField[28].stTransp) == 0))
            {
                Ret = mtfb_dosetcolreg(regno, red, green, blue, transp, info, MT_TRUE);
                break;
            }
            else
            {
                if (regno >= 16)
                {
                    break;
                }
                if (info->var.red.offset == 10)
                {
                    /* 1:5:5:5 */
                    ((u32*) (info->pseudo_palette))[regno] =
                    ((red   & 0xf800) >>  1) |
                    ((green & 0xf800) >>  6) |
                    ((blue  & 0xf800) >> 11);
                }
                else
                {
                    /* 0:5:6:5 */
                    ((u32*) (info->pseudo_palette))[regno] =
                    ((red   & 0xf800)      ) |
                    ((green & 0xfc00) >>  5) |
                    ((blue  & 0xf800) >> 11);
                }
                break;
            }
        case 24:
        case 32:
            red   >>= 8;
            green >>= 8;
            blue  >>= 8;
            transp >>= 8;
            ((u32 *)(info->pseudo_palette))[regno] =
             (red   << info->var.red.offset)   |
             (green << info->var.green.offset) |
             (blue  << info->var.blue.offset)  |
             (transp  << info->var.transp.offset) ;
            break;
    }

    MTFB_FUN_OUT;
    return Ret;
}

static mt_s32 mtfb_setcolreg(unsigned regno, unsigned red, unsigned green,
                          unsigned blue, unsigned transp, struct fb_info *info)
{

    MTFB_FUN_IN;
    return _setcolreg(regno, red, green, blue, transp, info);

}


/***************************************************************************************
* func          : mtfb_setcmap
* description   : CNcomment: 设置调色板 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_setcmap(struct fb_cmap *cmap, struct fb_info *info)
{
    mt_s32 i = 0, start = 0;
    unsigned short *red, *green, *blue, *transp;
    unsigned short hred, hgreen, hblue, htransp = 0xffff;
    MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

    MTFB_FUN_IN;

    if (par->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
    {
        return -EINVAL;
    }

    if (!g_pstCap[par->stBaseInfo.u32LayerID].bCmap)
    {
        /* AE6D03519, delete tmts color map warning! */
        MTFB_DEBUGK("Layer%d is not support color map!\n", par->stBaseInfo.u32LayerID);
        return -EPERM;
    }

    red    = cmap->red;
    green  = cmap->green;
    blue   = cmap->blue;
    transp = cmap->transp;
    start  = cmap->start;

    for (i = 0; i < cmap->len; i++)
    {
        hred   = *red++;
        hgreen = *green++;
        hblue  = *blue++;
        htransp = (transp != NULL)?*transp++:0xffff;
        _setcolreg(start++, hred, hgreen, hblue, htransp, info);
    }

    MTFB_FUN_OUT;
    return 0;
}

#ifdef CFG_MTFB_SUPPORT_CONSOLE
void mtfb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
    cfb_fillrect(p, rect);
}
void mtfb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
    cfb_copyarea(p, area);
}
void mtfb_imageblit(struct fb_info *p, const struct fb_image *image)
{
    cfb_imageblit(p, image);
}
#endif

#ifdef CONFIG_DMA_SHARED_BUFFER
struct dma_buf * mtfb_dmabuf_export(struct fb_info *info)
{
     return mtfb_memblock_export(info->fix.smem_start, info->fix.smem_len, 0);
}
#endif

static struct fb_ops s_stmtfbops =
{
    .owner			= THIS_MODULE,
    .fb_open		= mtfb_open,
    .fb_release		= mtfb_release,
    .fb_check_var	= mtfb_check_var,
    .fb_set_par		= mtfb_set_par,
    .fb_pan_display = mtfb_pan_display,
    .fb_ioctl		= mtfb_ioctl,
    .fb_setcolreg	= mtfb_setcolreg,
    .fb_setcmap		= mtfb_setcmap,
#ifdef CFG_MTFB_SUPPORT_CONSOLE
   	.fb_fillrect	= mtfb_fillrect,
	.fb_copyarea	= mtfb_copyarea,
	.fb_imageblit	= mtfb_imageblit,
#endif
#ifdef CONFIG_DMA_SHARED_BUFFER
    .fb_dmabuf_export	= mtfb_dmabuf_export,
#endif
};

/******************************************************************************
 Function        : mtfb_overlay_cleanup
 Description     : releae the resource for certain framebuffer
 Data Accessed   :
 Data Updated    :
 Output          : None
 Input           : mt_s32 wmtch_layer
                   mt_s32 need_unregister
 Return          : static
 Others          : 0
******************************************************************************/

static mt_void mtfb_overlay_cleanup(mt_u32 u32LayerId, MT_BOOL bUnregister)
{
    struct fb_info* info = NULL;
    struct fb_cmap* cmap = NULL;

    MTFB_FUN_IN;

    /* get framebuffer info structure pointer */
    info = s_stLayer[u32LayerId].pstInfo;
    if (info != NULL)
    {
        cmap = &info->cmap;

        if (cmap->len != 0)
        {
            /* free color map */
            fb_dealloc_cmap(cmap);
        }

        if (info->screen_base != MT_NULL)
        {
            mtfb_buf_ummap(info->screen_base);
        }

        if (info->fix.smem_start != 0)
        {
            mtfb_buf_freemem(info->fix.smem_start);
        }

        if (bUnregister)
        {
            unregister_framebuffer(info);
        }

        s_stLayer[u32LayerId].pstInfo = NULL;

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
		if (s_stTextLayer[u32LayerId].bAvailable)
		{
		    mt_u32 i;
			for (i = 0; i < SCROLLTEXT_NUM; i++)
			{
			    if (s_stTextLayer[u32LayerId].stScrollText[i].bAvailable)
			    {
			        mtfb_freescrolltext_cachebuf(&(s_stTextLayer[u32LayerId].stScrollText[i]));
			    	 memset(&s_stTextLayer[u32LayerId].stScrollText[i],0,sizeof(MTFB_SCROLLTEXT_S));

			    }
			}
			s_stTextLayer[u32LayerId].bAvailable = MT_FALSE;
			s_stTextLayer[u32LayerId].u32textnum = 0;
			s_stTextLayer[u32LayerId].u32ScrollTextId = 0;
		}
#endif
    }

    MTFB_FUN_IN;
    return;
}


/***************************************************************************************
* func          : mtfb_overlay_probe
* description   : CNcomment: 注册图层 CNend\n
                  info->fix.smem_start  物理地址
                  info->screen_base     虚拟地址
                  info->fix.smem_len    buffer大小
                  info->fix.line_length 行间距
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_overlay_probe(mt_u32 u32LayerId, mt_u32 u32VramSize)
{
    mt_s32 s32Ret = 0;
    struct fb_info * info = NULL;
    MTFB_PAR_S *pstPar = NULL;

    MTFB_FUN_IN;

    /**
     ** Creates a new frame buffer info structure.
     ** reserves mtfb_par for driver private data (info->par)
     ** PAR + 调色板
     **/
    info = framebuffer_alloc((sizeof(MTFB_PAR_S)+sizeof(u32) * 256), NULL);
    if (!info)
    {
        MTFB_DEBUGK("failed to malloc the fb_info!\n");
        MTFB_FUN_OUT;
        return -ENOMEM;
    }

    info->pseudo_palette = ((mt_u8*)(info->par) + sizeof(MTFB_PAR_S));
    /**
     ** save the info pointer in global pointer array, otherwise the
     ** info will be lost in cleanup if the following code has error
     **/
    s_stLayer[u32LayerId].pstInfo = info;

	snprintf(info->fix.id,sizeof(info->fix.id),"ovl%d", u32LayerId);
	info->fix.id[sizeof(info->fix.id) - 1] = '\0';

	info->flags = FBINFO_HWACCEL_YPAN | FBINFO_HWACCEL_XPAN;

    /**
     ** initialize file operations
     **/
    info->fbops = &s_stmtfbops;

	/**
	 **初始化自己维护变量的值，保存图层ID和地址
	 **/
    pstPar                         = (MTFB_PAR_S *)(info->par);
    pstPar->stBaseInfo.u32LayerID  = u32LayerId;
    pstPar->stDispInfo.stCanvasSur.u32PhyAddr = 0;

    if (IS_HD_LAYER(u32LayerId))
    {
        info->fix = s_stDefFix[MTFB_LAYER_TYPE_HD];
        info->var = s_stDefVar[MTFB_LAYER_TYPE_HD];
    }
    else if (IS_SD_LAYER(u32LayerId))
    {
        info->fix = s_stDefFix[MTFB_LAYER_TYPE_SD];
        info->var = s_stDefVar[MTFB_LAYER_TYPE_SD];
    }
    else if(  IS_AD_LAYER(u32LayerId)
		    || IS_MINOR_HD_LAYER(u32LayerId)
			|| IS_MINOR_SD_LAYER(u32LayerId))
    {/** 做字幕用的图层 **/
        info->fix = s_stDefFix[MTFB_LAYER_TYPE_AD];
        info->var = s_stDefVar[MTFB_LAYER_TYPE_AD];
    }
    else
    {
       MTFB_FUN_OUT;
		return MT_FAILURE;
    }

    /**
     ** it's not need to alloc mem for cursor layer
     **/
    if (u32VramSize != 0)
    {
        /**
         ** Modify 16 to 32, preventing out of bound.
         **/
        mt_char name[32];
        /**
         ** initialize the fix screen info
         **/
        snprintf(name, sizeof(name), "MTFB_Fb%d", u32LayerId);
		name[sizeof(name) -1] = '\0';

        info->fix.smem_start = mtfb_buf_allocmem(name, (u32VramSize)* 1024);
        if (0 == info->fix.smem_start)
        {
            MTFB_DEBUGK("%s:failed to malloc the video memory, size: %d KBtyes!\n", name, (u32VramSize));
            goto ERR;
        }
        else
        {
        	/**
        	 **M大小
        	 **/
            info->fix.smem_len = u32VramSize * 1024;

            /**
             ** initialize the virtual address and clear memory
             **/
            info->screen_base = mtfb_buf_map(info->fix.smem_start);
            if (MT_NULL == info->screen_base)
            {
                MTFB_WARNING("Failed to call map video memory, "
                         "size:0x%x, start: 0x%lx\n",
                         info->fix.smem_len, info->fix.smem_start);
            }
            else
            {
                memset(info->screen_base, 0x00, info->fix.smem_len);
            }
        }

        /**
         ** alloc color map，调色板结构体
         **/
        if (g_pstCap[u32LayerId].bCmap)
        {
            if (fb_alloc_cmap(&info->cmap, 256, 1) < 0)
            {
                MTFB_WARNING("fb_alloc_cmap failed!\n");
            }
            else
            {
                info->cmap.len = 256;
            }
        }
    }

	 /**
     ** 向标准fb中注册
     **/
    if ((s32Ret = register_framebuffer(info)) < 0)
    {
        MTFB_DEBUGK("failed to register_framebuffer!\n");
        s32Ret = -EINVAL;
        goto ERR;
    }


    MTFB_DEBUGK("succeed in registering the fb%d: %s frame buffer device\n",
              info->node, info->fix.id);

    MTFB_FUN_OUT;
    return MT_SUCCESS;

ERR:
    mtfb_overlay_cleanup(u32LayerId, MT_FALSE);
    MTFB_FUN_OUT;
    return s32Ret;

}


/***************************************************************************************
* func          : mtfb_get_vram_size
* description   : CNcomment: 获取参数大小 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static unsigned long mtfb_get_vram_size(char* pstr)
{
    mt_s32 str_is_valid = MT_TRUE;
    unsigned long vram_size = 0;
    char* ptr = pstr;

    MTFB_FUN_IN;
    if ((ptr == NULL) || (*ptr == '\0'))
    {
        MTFB_FUN_OUT;
        return 0;
    }

    /*check if the string is valid*/
    while (*ptr != '\0')
    {
        if (*ptr == ',')
        {
            break;
        }
        else if ((!isdigit(*ptr)) && ('X' != *ptr) && ('x' != *ptr)
            && ((*ptr > 'f' && *ptr <= 'z') || (*ptr > 'F' && *ptr <= 'Z')))
        {
            str_is_valid = MT_FALSE;
            break;
        }

        ptr++;
    }

    if (str_is_valid)
    {
        vram_size = simple_strtoul(pstr, (char **)NULL, 0);
        /*make the size PAGE_SIZE align*/
        vram_size = ((vram_size * 1024 + PAGE_SIZE - 1) & PAGE_MASK)/1024;
    }

    MTFB_FUN_OUT;
    return vram_size;
}


/***************************************************************************************
* func          : mtfb_parse_cfg
* description   : CNcomment: 参数解析 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
static mt_s32 mtfb_parse_cfg(mt_void)
{
    mt_char *pscstr = NULL;
    mt_char number[4] = {0};
    mt_u32 i = 0;
    mt_u32 u32LayerId;
    mt_u32 u32LayerSize;

    MTFB_FUN_IN;
    /**
     ** find the first 'varm' position in \arg video
     **/

    /**
     ** get the string before next varm
     **/
    pscstr = strstr(video, "vram");

    MTFB_DEBUGK("video:%s\n", video);

    while (pscstr != NULL)
    {
        /**
         ** parse the layer id and save it in a string
         **/
        i = 0;

        /**
         ** skip "vram"
         **/
        pscstr += 4;
        while (*pscstr != '_')
        {
            /* i>1 means layer id is bigger than 100, it's obviously out of range!*/
            if (i > 1)
            {
                MTFB_DEBUGK("layer id is out of range!\n");
                return -1;
            }

            number[i] = *pscstr;
            i++;
            pscstr++;
        }

        number[i] = '\0';

        /**
         ** change the layer id string into digital and assure it's legal
         **/
        u32LayerId = simple_strtoul(number, (char **)NULL, 10);
        if (u32LayerId > MTFB_MAX_LAYER_ID)
        {
            MTFB_DEBUGK("layer id is out of range!\n");
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        if ((!g_pstCap[u32LayerId].bLayerSupported)
            &&(u32LayerId != MTFB_LAYER_CURSOR))
        {
            MTFB_DEBUGK("cmtp doesn't support layer %d!\n", u32LayerId);
            MTFB_FUN_OUT;
            return MT_FAILURE;
        }

        /* get the layer size string and change it to digital */
        pscstr += sizeof("size") + i;
        u32LayerSize = mtfb_get_vram_size(pscstr);
        if ((u32LayerSize < s_stLayer[u32LayerId].u32LayerSize / 2) && (u32LayerId != MTFB_LAYER_CURSOR)
            && (u32LayerSize > 0))
        {
            u32LayerSize = s_stLayer[u32LayerId].u32LayerSize / 2;
        }
		/**
		 **KO带入的值，kbyte
		 **/
        s_stLayer[u32LayerId].u32LayerSize = u32LayerSize;
        /* get next layer string */
        pscstr = strstr(pscstr, "vram");

    }

    MTFB_FUN_OUT;
    return 0;
}



#ifdef CFG_MTFB_PROC_SUPPORT
static const mt_char* s_pszFmtName[] = {
    "RGB565",
    "RGB888",
    "KRGB444",
    "KRGB555",
    "KRGB888",
    "ARGB4444",
    "ARGB1555",
    "ARGB8888",
    "ARGB8565",
    "RGBA4444",
    "RGBA5551",
    "RGBA5658",
    "RGBA8888",
    "BGR565",
    "BGR888",
    "ABGR4444",
    "ABGR1555",
    "ABGR8888",
    "ABGR8565",
    "KBGR444",
    "KBGR555",
    "KBGR888",
    "1BPP",
    "2BPP",
    "4BPP",
    "8BPP",
    "ACLUT44",
    "ACLUT88",
    "PUYVY",
    "PYUYV",
    "PYVYU",
    "YUV888",
    "AYUV8888",
    "YUVA8888",
    "BUTT"};

const static mt_char* s_pszLayerName[] = {"layer_hd_0", "layer_hd_1", "layer_hd_2", "layer_hd_3",
                                          "layer_sd_0", "layer_sd_1", "layer_sd_2", "layer_sd_3",
                                          "layer_ad_0", "layer_ad_1", "layer_ad_2", "layer_ad_3",
                                          "layer_cursor"};
mt_s32 mtfb_print_slvlayer_proc(struct fb_info * info, struct seq_file *p, mt_void *v)
{
    MTFB_PAR_S *par;
	const mt_char* pLayerName = NULL;
	MTFB_SLVLAYER_DATA_S stSlvLayerData;

	par = (MTFB_PAR_S *)info->par;

	if (par->stBaseInfo.u32LayerID >= MTFB_LAYER_ID_BUTT)
	{
		MTFB_DEBUGK("wrong layer id.\n");
		return MT_FAILURE;
	}

    if (par->stBaseInfo.u32LayerID >= sizeof(s_pszLayerName)/sizeof(*s_pszLayerName))
    {
        pLayerName = "unknow layer";
    }
    else
    {
        pLayerName = s_pszLayerName[par->stBaseInfo.u32LayerID];
    }

	if (s_stDrvOps.MTFB_DRV_GetSlvLayerInfo(&stSlvLayerData))
	{
	    MTFB_DEBUGK("fail to get layer%d info!\n", par->stBaseInfo.u32LayerID);
		return MT_FAILURE;
	}

    PROC_PRINT(p,  "LayerId                    \t :%s\n", pLayerName);
	PROC_PRINT(p,  "ShowState                  \t :%s\n", stSlvLayerData.bShow? "ON" : "OFF");
	PROC_PRINT(p,  "ColorFormat                \t :%s\n", s_pszFmtName[stSlvLayerData.eFmt]);
	PROC_PRINT(p,  "Stride                     \t :%d\n", stSlvLayerData.u32Stride);
	PROC_PRINT(p,  "AttachRole                 \t :%s\n", "destination");
	PROC_PRINT(p,  "MasterLayerNum             \t :%d\n", par->stProcInfo.u32MasterLayerNum);

	PROC_PRINT(p,  "WriteBackResolution(Source/Dst/Max)\t :(%d, %d)/(%d, %d)/(%d, %d)\n", stSlvLayerData.stSrcBufRect.w, stSlvLayerData.stSrcBufRect.h,
				stSlvLayerData.stCurWBCBufRect.w, stSlvLayerData.stCurWBCBufRect.h, stSlvLayerData.stMaxWbcBufRect.w, stSlvLayerData.stMaxWbcBufRect.h);

	PROC_PRINT(p,  "Screenregion               \t :(%d, %d, %d, %d)\n", stSlvLayerData.stScreenRect.x, stSlvLayerData.stScreenRect.y,
		                                                                  (stSlvLayerData.stScreenRect.x + stSlvLayerData.stScreenRect.w),
		                                                                  (stSlvLayerData.stScreenRect.y + stSlvLayerData.stScreenRect.h));

	PROC_PRINT(p,  "WbcBufNum                  \t :%d \n",stSlvLayerData.u32WbcBufNum);
	PROC_PRINT(p,  "Mem size                   \t :%d KB\n\n",stSlvLayerData.u32WbcBufSize/1024);
    return MT_SUCCESS;
}

mt_s32 mtfb_print_layer_proc(struct fb_info * info, struct seq_file *p, mt_void *v)
{
	mt_u32 u32Stride;
    MTFB_PAR_S *par;
	MTFB_RECT   stOutputRect;
	MTFB_RECT   stDispRect;
    const mt_char* pszBufMode[] = {"triple", "double ", "single", "triple( no dicard frame)", "standard", "unknow"};
    const mt_char* pszAntiflicerLevel[] =  {"NONE", "LOW" , "MIDDLE", "MTGH", "AUTO" ,"ERROR"};
    const mt_char* pszAntiMode[] =  {"NONE", "TDE" , "VOU" , "BUTT"};
    const mt_char* pszStereoMode[] =  {"Mono", "Side by Side" , "Top and Bottom", "Frame packing", "unknow mode"};
    const mt_char* pLayerName = NULL;

    par = (MTFB_PAR_S *)info->par;

	if (par->stBaseInfo.u32LayerID >= MTFB_LAYER_ID_BUTT)
	{
		MTFB_DEBUGK("wrong layer id.\n");
		return MT_FAILURE;
	}

	s_stDrvOps.MTFB_DRV_GetLayerOutRect(par->stBaseInfo.u32LayerID, &stOutputRect);
	s_stDrvOps.MTFB_DRV_GetDispSize(par->stBaseInfo.u32LayerID, &stDispRect);
    if (par->stBaseInfo.u32LayerID >= sizeof(s_pszLayerName)/sizeof(*s_pszLayerName))
    {
        pLayerName = "unknow layer";
    }
    else
    {
        pLayerName = s_pszLayerName[par->stBaseInfo.u32LayerID];
    }

    if (par->stBaseInfo.enAntiflickerMode > MTFB_ANTIFLICKER_BUTT)
    {
        par->stBaseInfo.enAntiflickerMode = MTFB_ANTIFLICKER_BUTT;
    }

    if (par->stBaseInfo.enAntiflickerLevel > MTFB_LAYER_ANTIFLICKER_BUTT)
    {
        par->stBaseInfo.enAntiflickerLevel = MTFB_LAYER_ANTIFLICKER_BUTT;
    }

	if (par->bSetStereoMode)
	{
		u32Stride = par->st3DInfo.st3DSurface.u32Pitch;
	}
	else
	{
		if ((par->stExtendInfo.enBufMode == MTFB_LAYER_BUF_NONE)
	         && par->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr)
		{
			u32Stride = par->stDispInfo.stUserBuffer.stCanvas.u32Pitch;
		}
		else
		{
			u32Stride = info->fix.line_length;
		}
	}

	PROC_PRINT(p,  "LayerId                     \t :%s\n", pLayerName);
    PROC_PRINT(p,  "Fps                         \t :%d\n", par->stFrameInfo.u32Fps);
	PROC_PRINT(p,  "ShowState                   \t :%s\n", par->stExtendInfo.bShow ? "ON" : "OFF");
    PROC_PRINT(p,  "SyncType                    \t :%s\n", par->bHwcRefresh ? "fence" : "vblank");
#ifdef CFG_MTFB_FENCE_SUPPORT
    if (par->bHwcRefresh)
    {
        PROC_PRINT(p,  "Fence                       \t :%d\n", s_SyncInfo.u32FenceValue);
        PROC_PRINT(p,  "Timeline                    \t :%d\n", s_SyncInfo.u32Timeline);
    }
#endif
	PROC_PRINT(p,  "ColorFormat:                \t :%s\n", s_pszFmtName[par->stExtendInfo.enColFmt]);
	PROC_PRINT(p,  "Stride                      \t :%d\n", u32Stride);
	PROC_PRINT(p,  "Offset                      \t :(%d, %d)\n", info->var.xoffset, info->var.yoffset);
	PROC_PRINT(p,  "Resolution(real/virtual/max)\t :(%d, %d)/(%d, %d)/(%d, %d)\n", info->var.xres, info->var.yres,
				info->var.xres_virtual, info->var.yres_virtual,stOutputRect.w,  stOutputRect.h);
	PROC_PRINT(p,  "MemSize:                    \t :%d KB\n\n",info->fix.smem_len / 1024);

	PROC_PRINT(p,  "StartPosition               \t :(%d, %d)\n", par->stExtendInfo.stPos.s32XPos, par->stExtendInfo.stPos.s32YPos);
	PROC_PRINT(p,  "BufferMode                  \t :%s\n",pszBufMode[par->stExtendInfo.enBufMode]);
	PROC_PRINT(p,  "PixelAlpha                  \t :enable(%s), alpha0(0x%x), alpha1(0x%x)\n",
				par->stExtendInfo.stAlpha.bAlphaEnable ? "true" : "false",
				par->stExtendInfo.stAlpha.u8Alpha0, par->stExtendInfo.stAlpha.u8Alpha1);
	PROC_PRINT(p,  "GlobalAlpha                 \t :0x%x\n", par->stExtendInfo.stAlpha.u8GlobalAlpha);
	PROC_PRINT(p,  "Colorkey                    \t :enable(%s), value(0x%x)\n", par->stExtendInfo.stCkey.bKeyEnable ? "true" : "false",
				par->stExtendInfo.stCkey.u32Key);
	PROC_PRINT(p,  "Deflicker                   \t :enable(%s), mode(%s), level(%s)\n",par->stBaseInfo.bNeedAntiflicker ? "true" : "false",
					pszAntiMode[par->stBaseInfo.enAntiflickerMode], pszAntiflicerLevel[par->stBaseInfo.enAntiflickerLevel]);
	PROC_PRINT(p,  "3DMode                      \t :input(%s), output(%s)\n", pszStereoMode[par->st3DInfo.enInStereoMode], pszStereoMode[par->st3DInfo.enOutStereoMode]);
	PROC_PRINT(p,  "DisplayResolution           \t :(%d, %d)\n",par->stExtendInfo.u32DisplayWidth, par->stExtendInfo.u32DisplayHeight);

	PROC_PRINT(p,  "CanavasAddr                 \t :0x%lx\n",(ulong)par->stDispInfo.stUserBuffer.stCanvas.u32PhyAddr);
	PROC_PRINT(p,  "CanavasUpdateRect           \t :(%d,%d,%d,%d) \n", par->stDispInfo.stUserBuffer.UpdateRect.x, par->stDispInfo.stUserBuffer.UpdateRect.y,
					par->stDispInfo.stUserBuffer.UpdateRect.w, par->stDispInfo.stUserBuffer.UpdateRect.h);
	PROC_PRINT(p,  "CanvasResolution            \t :(%d,%d)\n",par->stDispInfo.stCanvasSur.u32Width, par->stDispInfo.stCanvasSur.u32Height);
	PROC_PRINT(p,  "CanvasPitch                 \t :%d\n",par->stDispInfo.stCanvasSur.u32Pitch);
	PROC_PRINT(p,  "CanvasFormat                \t :%s\n",s_pszFmtName[par->stDispInfo.stCanvasSur.enFmt]);

	if (g_bProcDebug)
	{
		PROC_PRINT(p,  "\nReferecceCount              \t :%d\n", atomic_read(&par->stBaseInfo.ref_count));
		PROC_PRINT(p,  "DeviceMaxResolution         \t :%d, %d\n",stDispRect.w,	stDispRect.h);
		PROC_PRINT(p,  "DisplayingAddr(register)    \t :0x%x\n",par->stRunInfo.u32ScreenAddr);
		PROC_PRINT(p,  "DisplayBuf[0] addr          \t :0x%x\n",par->stDispInfo.u32DisplayAddr[0]);
		PROC_PRINT(p,  "DisplayBuf[1] addr          \t :0x%x\n",par->stDispInfo.u32DisplayAddr[1]);
		PROC_PRINT(p,  "IsNeedFlip(2buf)            \t :%s\n",par->stRunInfo.bNeedFlip? "YES" : "NO");
		PROC_PRINT(p,  "BufferIndexDisplaying(2buf) \t :%d\n",1-par->stRunInfo.u32IndexForInt);
		PROC_PRINT(p,  "UnionRect(2buf)             \t :(%d,%d,%d,%d)\n",par->stDispInfo.stUpdateRect.x, par->stDispInfo.stUpdateRect.y, par->stDispInfo.stUpdateRect.w, par->stDispInfo.stUpdateRect.h);
	}

    return 0;
}

static mt_s32 mtfb_read_proc(struct seq_file *p, mt_void *v)
{
    mt_proc_entry_t * item = (mt_proc_entry_t *)(p->private);
    struct fb_info* info    = (struct fb_info *)(item->data);
	MTFB_PAR_S *par = (MTFB_PAR_S *)info->par;

   MTFB_FUN_IN;
	if (par->stProcInfo.bWbcProc)
	{
	    MTFB_FUN_OUT;
	    return mtfb_print_slvlayer_proc(info, p, v);
	}
	else
	{
	    MTFB_FUN_OUT;
	    return mtfb_print_layer_proc(info, p, v);
	}

}

extern mt_void mtfb_captureimage_fromdevice(mt_u32 u32LayerID, MT_BOOL bAlphaEnable);

static mt_void mtfb_parse_proccmd(struct seq_file* p, mt_u32 u32LayerId, mt_char *pCmd)
{
    struct fb_info *info = s_stLayer[u32LayerId].pstInfo;
    MTFB_PAR_S *pstPar = (MTFB_PAR_S *)info->par;
    mt_s32 cnt = atomic_read(&pstPar->stBaseInfo.ref_count);


    if (strncmp("show", pCmd, 4) == 0)
    {
        if (cnt == 0)
        {
            MTFB_DEBUGK("err:device no open!\n");
            return;
        }

        if (pstPar->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
        {
            MTFB_DEBUGK("cursor layer doesn't support this cmd!\n");
            return;
        }

		if (pstPar->stProcInfo.bWbcProc)
        {
            MTFB_DEBUGK("write back layer doesn't support this cmd!\n");
            return;
        }

        if (!pstPar->stExtendInfo.bShow)
        {
            pstPar->stRunInfo.bModifying = MT_TRUE;
            pstPar->stExtendInfo.bShow = MT_TRUE;
            pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_SHOW;
            pstPar->stRunInfo.bModifying = MT_FALSE;
        }
    }
    else if (strncmp("hide", pCmd, 4) == 0)
    {
        if (cnt == 0)
        {
            MTFB_DEBUGK("err:device not open!\n");
            return;
        }

        if (pstPar->stBaseInfo.u32LayerID == MTFB_LAYER_CURSOR)
        {
            PROC_PRINT(p, "cursor layer doesn't support this cmd!\n");
            return;
        }

		if (pstPar->stProcInfo.bWbcProc)
        {
            MTFB_DEBUGK("write back layer doesn't support this cmd!\n");
            return;
        }

        if (pstPar->stExtendInfo.bShow)
        {
            pstPar->stRunInfo.bModifying = MT_TRUE;
            pstPar->stExtendInfo.bShow = MT_FALSE;
            pstPar->stRunInfo.u32ParamModifyMask |= MTFB_LAYER_PARAMODIFY_SHOW;
            pstPar->stRunInfo.bModifying = MT_FALSE;
        }
    }
    else if (strncmp("help", pCmd, 4) == 0)
    {
        mt_drv_proc_echohelp("help info:\n");
        mt_drv_proc_echohelp("echo cmd > proc file\n");
        mt_drv_proc_echohelp("mtfb support cmd:\n");
        mt_drv_proc_echohelp("show:show layer\n");
        mt_drv_proc_echohelp("hide:hide layer\n");
		mt_drv_proc_echohelp("alpha=255:set layer's global alpha\n");
#if 0
		mt_drv_proc_echohelp("capture:capture image from frame buffer\n");
		mt_drv_proc_echohelp("vblank on :vblank on\n");
		mt_drv_proc_echohelp("vblank off:vblank off\n");
#endif
		mt_drv_proc_echohelp("debug on :debug on\n");
		mt_drv_proc_echohelp("debug off:debug off\n");
        mt_drv_proc_echohelp("For example, if you want to mtde layer 1,you can input:\n");
        mt_drv_proc_echohelp("echo mtde > /proc/msp/mtfb1\n");
    }
	else if (strncmp("debug on", pCmd, 8) == 0)
    {
		g_bProcDebug = MT_TRUE;
		MTFB_DEBUGK("set proc debug on.\n");
    }
	else if (strncmp("debug off", pCmd, 9) == 0)
    {
		g_bProcDebug = MT_FALSE;
		MTFB_DEBUGK("set proc debug off.\n");
    }
#if 0
	else if (strncmp("vblank on", pCmd, 9) == 0)
    {
		pstPar->bVblank = MT_TRUE;
		//MTFB_DEBUGK("set proc vblank on.\n");
    }
	else if (strncmp("vblank off", pCmd, 10) == 0)
    {
		pstPar->bVblank = MT_FALSE;
		//MTFB_DEBUGK("set proc vblank off.\n");
    }
#endif
	else if (strncmp("alpha", pCmd, 5) == 0)
	{
		mt_u32  u32Alpha;
		mt_char TmpCmd[MTFB_FILE_NAME_MAX_LEN]={0};
		MT_BOOL bIsStrValid = MT_FALSE;
		mt_char *pStr = MT_NULL;
		mt_char *pStrTmp = MT_NULL;

		if (pstPar->stProcInfo.bWbcProc)
        {
            MTFB_DEBUGK("write back layer doesn't support this cmd!\n");
            return;
        }

		strncpy(TmpCmd,pCmd,(MTFB_FILE_NAME_MAX_LEN-1));
		TmpCmd[MTFB_FILE_NAME_MAX_LEN-1] = '\0';

		pStr = strstr(TmpCmd, "=");

		if (MT_NULL == pStr)
		{
			return;
		}

		pStr++;

		while(*pStr != '\0')
		{
			if (MT_FALSE == bIsStrValid)
			{
				if (isdigit(*pStr) || ('X' == *pStr) || ('x' == *pStr)
					|| (*pStr >= 'a' && *pStr <= 'f') || (*pStr >= 'A' && *pStr <= 'F'))
				{
					bIsStrValid = MT_TRUE;
					pStrTmp = pStr;
				}
			}
			else
			{
				if ((!isdigit(*pStr)) && ('X' != *pStr) && ('x' != *pStr)
						&& (!(*pStr >= 'a' && *pStr <= 'f') && !(*pStr >= 'A' && *pStr <= 'F')))
				{
					*pStr = '\0';
					break;
				}
			}

			pStr++;
		}

		if (!bIsStrValid)
		{
			MTFB_DEBUGK("cmd is invalid\n");
			return;
		}

		u32Alpha = simple_strtoul(pStrTmp, (char **)NULL, 0);

		if (u32Alpha > 255)
		{
			u32Alpha = 255;
		}

	   	pstPar->stExtendInfo.stAlpha.bAlphaChannel = MT_TRUE;
        pstPar->stExtendInfo.stAlpha.u8GlobalAlpha = u32Alpha;

        s_stDrvOps.MTFB_DRV_SetLayerAlpha(pstPar->stBaseInfo.u32LayerID, &pstPar->stExtendInfo.stAlpha);
		MTFB_DEBUGK("set gfx global alpha 0x%x.\n", u32Alpha);
	}
#if 0
	else if (strncmp("capture", pCmd, 7) == 0)
	{
		mt_s32 cnt;
		cnt = atomic_read(&pstPar->stBaseInfo.ref_count);
		if (cnt < 1 && !pstPar->stProcInfo.bWbcProc)
		{
			MTFB_DEBUGK("Unsupported to capture a closed layer.\n");
			return;
		}

		mtfb_captureimage_fromdevice(pstPar->stBaseInfo.u32LayerID, MT_FALSE);
	}
	else if (strncmp("excapture", pCmd, 9) == 0)
	{
		mt_s32 cnt;
		cnt = atomic_read(&pstPar->stBaseInfo.ref_count);
		if (cnt < 1 && !pstPar->stProcInfo.bWbcProc)
		{
			MTFB_DEBUGK("Unsupported to capture a closed layer.\n");
			return;
		}

		mtfb_captureimage_fromdevice(pstPar->stBaseInfo.u32LayerID, MT_TRUE);
	}
#endif
    else
    {
        MTFB_DEBUGK("unsupported cmd:%s ", pCmd);
        MTFB_DEBUGK("you can use help cmd to show help info!\n");
    }

    return;
}

static mt_s32 mtfb_write_proc(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct fb_info *info;
    MTFB_PAR_S *pstPar;
    mt_char buffer[128];

    struct seq_file *seq = file->private_data;
    mt_proc_entry_t *item = seq->private;
    info = (struct fb_info *)(item->data);
    pstPar = (MTFB_PAR_S *)(info->par);


    if (count > sizeof(buffer))
    {
        MTFB_DEBUGK("The command string is out of buf space :%d bytes !\n", (unsigned int)sizeof(buffer));
        return 0;
    }

    if (copy_from_user(buffer, buf, count))
    {
        MTFB_DEBUGK("failed to call copy_from_user !\n");
        return 0;
    }

    mtfb_parse_proccmd(seq, pstPar->stBaseInfo.u32LayerID, (mt_char*)buffer);

    return count;
}
#endif


/***************************************************************************************
* func          : MTFB_DRV_ModExit
* description   : CNcomment: 驱动去初始化 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_void __exit MTFB_DRV_ModExit(mt_void)
{
    mt_s32 i;

    MTFB_FUN_IN;

	 MT_GFX_MODULE_UnRegister(MTGFX_FB_ID);
    //mt_drv_module_unregister(MTGFX_FB_ID);

    s_stDrvTdeOps.MTFB_DRV_SetTdeCallBack(NULL);

    s_stDrvOps.MTFB_DRV_GfxDeInit();

    for(i = 0; i <= MTFB_LAYER_SD_1; i++)
    {
#ifdef CFG_MTFB_LOGO_SUPPORT
     	mtfb_clear_logo(i,MT_TRUE);
#endif
        mtfb_overlay_cleanup(i, MT_TRUE);
    }

	s_stDrvTdeOps.MTFB_DRV_TdeClose();

   MTFB_FUN_OUT;
}

/***************************************************************************************
* func          : MTFB_DRV_ModInit
* description   : CNcomment: 加载KO的初始化 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 __init MTFB_DRV_ModInit(mt_void)
{

    mt_u32 i = 0;
#ifdef CFG_MTFB_LOGO_SUPPORT
    mt_s32 s32Ret = 0;
#endif

    MTFB_FUN_IN;

    memset(&s_stLayer, 0x00, sizeof(s_stLayer));

	/**
	 ** get drv and tde property
	 **/
	MTFB_DRV_GetDevOps(&s_stDrvOps);
    MTFB_DRV_GetTdeOps(&s_stDrvTdeOps);

    /**
     ** inital adoption layer
     **/
    if (MT_SUCCESS != s_stDrvOps.MTFB_DRV_GfxInit())
    {
        MTFB_DEBUGK("drv init failed\n");
        goto ERR;
    }

#ifdef CFG_MTFB_LOGO_SUPPORT

    /**
     **这个函数里面只做一件事情，设置掩码信息和标记图层已经打开在有logo的情况下
     **/
    s32Ret = mtfb_logo_init();

    if (s32Ret != MT_SUCCESS)
    {
        MTFB_DEBUGK("mtfb logo init failed\n");
    }
#endif

	/**
     ** 获取图层属性，这个保存在全局变量里optm_mtfb.c中
     **/
	if (s_stDrvOps.MTFB_DRV_GetGFXCap(&g_pstCap) < 0)
	{
		MTFB_DEBUGK("Gfx get device capability failed!\n");
       MTFB_FUN_OUT;
		return MT_FAILURE;
	}

	 /**
     ** 在KO里设置要是支持TC回写则设置标记为true
     **/
	if (!strncmp("on", tc_wbc, 2))
	{
		s_stDrvOps.MTFB_DRV_SetTCFlag(MT_TRUE);
	}

    /**
     ** parse the \arg video string
     **/
    if (mtfb_parse_cfg() < 0)
    {
        /* mtnt info */
        MTFB_DEBUGK("Usage:insmod mtfb.ko video=\"mtfb:vrami_size:xxx,vramj_size:xxx,...\"\n");
        MTFB_DEBUGK("i,j means layer id, xxx means layer size in kbytes!\n");
        MTFB_DEBUGK("example:insmod mtfb.ko video=\"mtfb:vram0_size:810,vram1_size:810\"\n\n");
        return MT_FAILURE;
    }

    /**
     ** inital fb file according the config
	 **/
//	 for(i = MTFB_LAYER_OSD0; i <= MTFB_LAYER_STILL; i++)  //for(i = 0; i <= MTFB_LAYER_SD_1; i++)
	 for(i = MTFB_LAYER_OSD0; i <= MTFB_LAYER_SD_0; i++)
    {
        /**
         ** if hw not support, we modify memory to 0, so in the path
         ** of /dev/ there is only one device name
         **/
		if (!strcmp("", video))
		{/** 要是没有带参数，则使用宏开关 **/
			s_stLayer[i].u32LayerSize = g_u32LayerSize[i];
		}
        /**
         ** register the layer
         **/
        if (mtfb_overlay_probe(i, s_stLayer[i].u32LayerSize) != MT_SUCCESS)
        {
           MT_ERR_MTFB("[%d][%d]\n",i, s_stLayer[i].u32LayerSize);
        	return MT_FAILURE;
        }

        MT_INFO_MTFB("[%d][%d]\n",i, s_stLayer[i].u32LayerSize);

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
		memset(&s_stTextLayer[i], 0, sizeof(MTFB_SCROLLTEXT_INFO_S));
#endif
    }

        MTFB_DEBUGK("layersize mtfb0:%d, mtfb1:%d, mtfb2:%d, mtfb3:%d, mtfb4:%d, mtfb5:%d, mtfb_cursor:%d\n",
        s_stLayer[MTFB_LAYER_HD_0].u32LayerSize, s_stLayer[MTFB_LAYER_HD_1].u32LayerSize,
        s_stLayer[MTFB_LAYER_HD_2].u32LayerSize, s_stLayer[MTFB_LAYER_HD_3].u32LayerSize,
        s_stLayer[MTFB_LAYER_SD_0].u32LayerSize, s_stLayer[MTFB_LAYER_SD_1].u32LayerSize,
        s_stLayer[MTFB_LAYER_CURSOR].u32LayerSize);

	/** 这句是废代码 **/
    s_stDrvTdeOps.MTFB_DRV_SetTdeCallBack(mtfb_tde_callback);

#ifndef MT_MCE_SUPPORT
    mtfb_init_module_k();
#endif

	/**
	 ** show version
	 ** tmts use GFX common function
	 **/
	mtfb_version();

    MTFB_FUN_OUT;
    return 0;

ERR:

    for(i = 0; i <= MTFB_LAYER_SD_1; i++)
    {
         MT_INFO_MTFB("------------\n");
        mtfb_overlay_cleanup(i, MT_TRUE);
    }

    MTFB_FUN_OUT;
    return MT_FAILURE;

}

