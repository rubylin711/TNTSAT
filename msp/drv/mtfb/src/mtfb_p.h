/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTFB_P_H__
#define __MTFB_P_H__


/*********************************add include here******************************/
#include "mt_type.h"
#include <linux/fb.h>
#include "mtfb_drv.h"
#include "mtfb_drv_common.h"
#include "mtfb_scrolltext.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/




#define MAX_FB  32          /* support 32 layers most, the limit is from linux fb*/

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
/* define the value of default set of each layer */
#define MTFB_HD_DEF_WIDTH    1920     /* unit: pixel */
#define MTFB_HD_DEF_HEIGHT   1080     /* unit: pixel */
#define MTFB_HD_DEF_STRIDE   (MTFB_HD_DEF_WIDTH*4)    /* unit: byte */
#define MTFB_HD_DEF_VRAM     16200  //(0x1c20)   /* unit:KB 1280*720*4*2*/
#else
/* define the value of default set of each layer */
#define MTFB_HD_DEF_WIDTH    1280     /* unit: pixel */
#define MTFB_HD_DEF_HEIGHT   720     /* unit: pixel */
#define MTFB_HD_DEF_STRIDE   (MTFB_HD_DEF_WIDTH*4)    /* unit: byte */
#define MTFB_HD_DEF_VRAM     7200  //(0x1c20)   /* unit:KB 1280*720*4*2*/
#endif


#define MTFB_SD_DEF_WIDTH    1280//720
#define MTFB_SD_DEF_HEIGHT   720
#define MTFB_SD_DEF_STRIDE   (MTFB_SD_DEF_WIDTH*4)
#define MTFB_SD_DEF_VRAM     7200   //(0xca8)   /* unit:KB 720*576*4*2*/


#define MTFB_AD_DEF_WIDTH    1280
#define MTFB_AD_DEF_HEIGHT   80
#define MTFB_AD_DEF_STRIDE   (MTFB_AD_DEF_WIDTH*4)
#define MTFB_AD_DEF_VRAM     800   //(0x230)   /*unit:KB 1280*80*4*2*/

#define MTFB_CURSOR_DEF_WIDTH    128
#define MTFB_CURSOR_DEF_HEIGHT   128
#define MTFB_CURSOR_DEF_STRIDE   (MTFB_CURSOR_DEF_WIDTH*4)
#define MTFB_CURSOR_DEF_VRAM     128   //(0x80)   /*unit:KB 128*128*4*2*/


#define MTFB_DEF_DEPTH    32      /* unit: bits */
#define MTFB_DEF_XSTART   0
#define MTFB_DEF_YSTART   0
#define MTFB_DEF_ALPHA    0xff
#define MTFB_DEF_PIXEL_FMT    MTFB_FMT_ARGB8888

#define MTFB_IS_CLUTFMT(eFmt)  (MTFB_FMT_1BPP <= (eFmt) && (eFmt) <= MTFB_FMT_ACLUT88)
#define MTFB_ALPHA_OPAQUE    0xff
#define MTFB_ALPHA_TRANSPARENT 0x00

#define MTFB_DEFLICKER_LEVEL_MAX 5   /* support level 5 deflicker most */

#define MTFB_MAX_LAYER_ID (MTFB_LAYER_ID_BUTT-1)
#define MTFB_MAX_LAYER_NUM MTFB_LAYER_ID_BUTT



/*************************** Structure Definition ****************************/

typedef enum
{
	MTFB_ANTIFLICKER_NONE,	/* no antiflicker.If scan mode is progressive, mtfb will set antiflicker mode to none */
	MTFB_ANTIFLICKER_TDE,	/* tde antiflicker mode, it's effect for 1buf or 2buf only */
	MTFB_ANTIFLICKER_VO,	/* vo antiflicker mode, need hardware supprot */
	MTFB_ANTIFLICKER_BUTT
} MTFB_LAYER_ANTIFLICKER_MODE_E;

/*only use in logo transition*/
typedef enum
{
    MTFB_STATE_LOGO_IN = 0x1,    /*boot start with logo*/
    MTFB_STATE_PUT_VSCREENINFO = 0x2, /*ioctl:FBIOPUT_VSCREENINFO*/
    MTFB_STATE_PAN_DISPLAY = 0x4, /*call function mtfb_pan_display*/
    MTFB_STATE_REFRESH = 0x8, /*refresh app*/
    MTFB_STATE_BUTT
}MTFB_STATE_E;

#define MTFB_MAX_FLIPBUF_NUM 2

/*frame info*/
typedef struct
{
    mt_u32  u32RefreshFrame;
    mt_u32  u32StartTimeMs;
    mt_u32  u32Fps;
}MTFB_FRAME_INFO_S;

/* 3D MEM INFO STRUCT*/
typedef struct
{
    mt_u32 u32StereoMemStart;
    mt_u32 u32StereoMemLen;
    struct mutex stStereoMemLock;    
}MTFB_3DMEM_INFO_S;


/* 3D PAR INFO STRUCT*/
typedef struct
{
	mt_s32                 s32StereoDepth;	
	mt_u32                 u32rightEyeAddr;     /**<  right eye address */
	mt_u32                 u32DisplayAddr[MTFB_MAX_FLIPBUF_NUM];
    MTFB_STEREO_MODE_E     enInStereoMode;
    MTFB_STEREO_MODE_E     enOutStereoMode; 	
	MTFB_RECT              st3DUpdateRect;
	MTFB_SURFACE_S         st3DSurface;
    MTFB_3DMEM_INFO_S      st3DMemInfo;
}MTFB_3D_PAR_S;


/* N3D PAR INFO STRUCT*/
typedef struct
{
	MTFB_RECT       stCmpRect;
	MTFB_RECT       stUpdateRect;
	mt_u32          u32DisplayAddr[MTFB_MAX_FLIPBUF_NUM]; /** 加载KO分配的两个块内存 **/
	MTFB_SURFACE_S  stCanvasSur;        /**
	                                      ** canvas surface allocated for user，单buffer就给硬件 
	                                      ** 双buffer， canvas buffer blit display buffer
	                                      **/
	MTFB_BUFFER_S   stUserBuffer;       /** 备份用户信息，可以重新刷新用户的最后一帧数据，要是没有
	                                      ** 备份，重新刷新可能刷新异常
	                                      ** backup usr's refresmtng buffer data, 
										  ** using when refresh again or refresh all
										  **/
}MTFB_DISP_INFO_S;


/** 运行相关的 **/
typedef struct 
{    
	MT_BOOL   bModifying;
	mt_u32    u32ParamModifyMask; 
    MT_BOOL   bNeedFlip;       /* when tde blit job completed, we need to flip buffer, only using in pandisplay and 2buf*/
    MT_BOOL   bFliped;	       /* a flag to record buf has been swithed no not in vo isr, effect only in 2 buf mode*/
    mt_u32    u32IndexForInt;  /* index of screen buf*/
	mt_u32    u32BufNum;       /* count of flip buffer*/	
	mt_u32    u32ScreenAddr;   /* screen buf addr */
	mt_s32    s32RefreshHandle;/* job handle of tde blit*/
}MTFB_RTIME_INFO_S;


/** 显示相关的 **/
typedef struct
{
	MT_BOOL                         bOpen;               /* open status*/
	MT_BOOL                         bShow;               /* show status */
    MTFB_COLOR_FMT_E                enColFmt;            /* color format */
	MTFB_LAYER_BUF_E                enBufMode;           /* refresh mode*/
	mt_u32                          u32DisplayWidth;     /* width  of layer's display buffer*/
    mt_u32                          u32DisplayHeight;    /* height of layer's display buffer*/
    mt_u32                          u32ScreenWidth;      /* 无用 width  of layer's  show    area*/
    mt_u32                          u32ScreenHeight;     /* 无用 height of layer's  show    area*/
	MTFB_POINT_S                    stPos;               /* beginning position of layer*/ 
    MTFB_ALPHA_S                    stAlpha;             /* alpha attribution */
    MTFB_COLORKEYEX_S               stCkey;              /* colorkey attribution */  	
}MTFB_EXTEND_INFO_S;

typedef struct
{
	mt_u32             u32LayerID;       /* layer id */
	atomic_t           ref_count;        /* framebuffer reference count */
	spinlock_t         lock;             /* using in 2buf refresh */
	MT_BOOL            bPreMul;
	MT_BOOL            bNeedAntiflicker;
	mt_u32             u32HDflevel;      /* horizontal deflicker level */
	mt_u32             u32VDflevel;      /* vertical deflicker level */
	mt_uchar           ucHDfcoef[MTFB_DEFLICKER_LEVEL_MAX - 1];/* horizontal deflicker coefficients */
	mt_uchar           ucVDfcoef[MTFB_DEFLICKER_LEVEL_MAX - 1];/* vertical deflicker coefficients */
	MTFB_LAYER_ANTIFLICKER_LEVEL_E  enAntiflickerLevel; /* antiflicker level */
	MTFB_LAYER_ANTIFLICKER_MODE_E   enAntiflickerMode; /* antiflicker mode */ 
}MTFB_BASE_INFO_S;

#ifdef CFG_MTFB_PROC_SUPPORT 
typedef struct
{
    MT_BOOL bWbcProc;             /* whether wbc proc or not*/
	MT_BOOL bCreatedProc;         /* have created proc */
	mt_u32          u32MasterLayerNum; /* the master layer num of slavery layer */
	MTFB_LAYER_ID_E enWbcLayerID; /* id of the layer attached to master layer */
}MTFB_PROC_S;
#endif

typedef struct
{   	
	MT_BOOL             bSetStereoMode;
	MT_BOOL             bPanFlag;
	MT_BOOL             bSetVar;
	MT_BOOL             bPanReady;
    MT_BOOL             bVblank;
    MT_BOOL             bHwcRefresh;
    MTFB_BASE_INFO_S    stBaseInfo;
    MTFB_EXTEND_INFO_S  stExtendInfo;
    
    MTFB_3D_PAR_S       st3DInfo;
    MTFB_DISP_INFO_S    stDispInfo; 

	MTFB_RTIME_INFO_S   stRunInfo;      /**run time info for N3D and 3D*/
    MTFB_FRAME_INFO_S   stFrameInfo;
#ifdef CFG_MTFB_PROC_SUPPORT 	
	MTFB_PROC_S         stProcInfo;
#endif	
}MTFB_PAR_S;

typedef struct 
{
    struct fb_info *pstInfo;
    mt_u32    u32LayerSize;     /*u32LayerSize = fb.smem_len*/
} MTFB_LAYER_S;



typedef enum 
{
    MTFB_LAYER_TYPE_HD,
    MTFB_LAYER_TYPE_SD,
    MTFB_LAYER_TYPE_AD,
    MTFB_LAYER_TYPE_CURSOR,
    MTFB_LAYER_TYPE_BUTT,    
}MTFB_LAYER_TYPE_E;


typedef struct
{
    struct fb_bitfield stRed;     /* bitfield in fb mem if true color, */
    struct fb_bitfield stGreen;   /* else only length is significant */
    struct fb_bitfield stBlue;
    struct fb_bitfield stTransp;  /* transparency	*/
} MTFB_ARGB_BITINFO_S;


/********************** Global Variable declaration **************************/

/**extern for scrolltext.c*/
/* the interface to operate the cmtp */
extern MTFB_DRV_OPS_S s_stDrvOps;
extern MTFB_DRV_TDEOPS_S s_stDrvTdeOps;

/* to save layer id and layer size */
extern MTFB_LAYER_S s_stLayer[MTFB_MAX_LAYER_NUM];

#ifdef CFG_MTFB_SCROLLTEXT_SUPPORT
extern MTFB_SCROLLTEXT_INFO_S s_stTextLayer[MTFB_LAYER_ID_BUTT];
#endif


/******************************* API declaration *****************************/

#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_P_H__ */


