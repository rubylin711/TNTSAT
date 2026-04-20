/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_hal.h
* Description:TDE hal interface define
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/

#ifndef _TDE_HAL_H_
#define _TDE_HAL_H_

#include "mt_tde_type.h"
#include "tde_adp.h"
#include "tde_define.h"
//#include "gpe_hal.h"
#include "tde_hal_aria.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */


/****************************************************************************/
/*                             TDE hal types define                         */
/****************************************************************************/

/* TDE pixel format, accord with register, value cannot modified freely */
typedef enum mtTDE_DRV_COLOR_FMT_E
{
    TDE_DRV_COLOR_FMT_RGB444   = 0,
    TDE_DRV_COLOR_FMT_RGB555   = 1,
    TDE_DRV_COLOR_FMT_RGB565   = 2,
    TDE_DRV_COLOR_FMT_RGB888   = 3,
    TDE_DRV_COLOR_FMT_ARGB4444 = 4,
    TDE_DRV_COLOR_FMT_ARGB1555 = 5,
    TDE_DRV_COLOR_FMT_ARGB8565 = 6,
    TDE_DRV_COLOR_FMT_ARGB8888 = 7,
    TDE_DRV_COLOR_FMT_CLUT1   = 8,
    TDE_DRV_COLOR_FMT_CLUT2   = 9,
    TDE_DRV_COLOR_FMT_CLUT4   = 10,
    TDE_DRV_COLOR_FMT_CLUT8   = 11,
    TDE_DRV_COLOR_FMT_ACLUT44 = 12,
    TDE_DRV_COLOR_FMT_ACLUT88 = 13,
    TDE_DRV_COLOR_FMT_A1 = 16,
    TDE_DRV_COLOR_FMT_A8 = 17,
    TDE_DRV_COLOR_FMT_YCbCr888   = 18,
    TDE_DRV_COLOR_FMT_AYCbCr8888 = 19,
    TDE_DRV_COLOR_FMT_YCbCr422 = 20,
    TDE_DRV_COLOR_FMT_byte = 22,
    TDE_DRV_COLOR_FMT_halfword = 23,
    TDE_DRV_COLOR_FMT_YCbCr400MBP = 24,
    TDE_DRV_COLOR_FMT_YCbCr422MBH = 25,
    TDE_DRV_COLOR_FMT_YCbCr422MBV = 26,
    TDE_DRV_COLOR_FMT_YCbCr420MB = 27,
    TDE_DRV_COLOR_FMT_YCbCr444MB = 28,
    TDE_DRV_COLOR_FMT_RABG8888,
    TDE_DRV_COLOR_FMT_RGB233,
    TDE_DRV_COLOR_FMT_BUTT
} TDE_DRV_COLOR_FMT_E;

/* branch order*/
/* support 24 orders in ARGB, TDE driver can only expose 4 usual orders; if needed, can be added */
typedef enum mtTDE_DRV_ARGB_ORDER_E
{
    TDE_DRV_ORDER_ARGB = 0x0,    // suport in aria   0
    TDE_DRV_ORDER_ABGR = 0x5,    // suport in aria   4
	TDE_DRV_ORDER_RABG = 0x7,
    TDE_DRV_ORDER_RGBA = 0x9,    // suport in aria   1
    TDE_DRV_ORDER_BGRA = 0x14,  //  suport in aria   3
    TDE_DRV_ORDER_BUTT
}TDE_DRV_ARGB_ORDER_E;

/* TDE basic operate mode */
typedef enum mtTDE_DRV_BASEOPT_MODE_E
{
    /* Quick fill */
    TDE_QUIKE_FILL,

    /* Quick copy */
    TDE_QUIKE_COPY,

    /* Normal fill in single source */
    TDE_NORM_FILL_1OPT,

    /* Normal bilit in single source */
    TDE_NORM_BLIT_1OPT,

    /* Fill and Rop */
    TDE_NORM_FILL_2OPT,

    /* Normal bilit in double source */
    TDE_NORM_BLIT_2OPT,

    /* MB operation */
    TDE_MB_C_OPT,    /* MB chroma zoom */
    TDE_MB_Y_OPT,    /* MB brightness zoom */
    TDE_MB_2OPT,     /* MB combinate operation */

    /* Fill operate in single source mode  */
    TDE_SINGLE_SRC_PATTERN_FILL_OPT,

    /* Fill operate in double source mode */
    TDE_DOUBLE_SRC_PATTERN_FILL_OPT,

    TDE_TREE_SRC_OPT,

    
} TDE_DRV_BASEOPT_MODE_E;

/* Type definition in interrupted state */
typedef enum mtTDE_DRV_INT_STATS_E
{
    TDE_DRV_LINK_COMPLD_STATS = 0x1,
    TDE_DRV_NODE_COMPLD_STATS = 0x2,
    TDE_DRV_LINE_SUSP_STATS = 0x4,
    TDE_DRV_RDY_START_STATS = 0x8,
    TDE_DRV_SQ_UPDATE_STATS = 0x10,
    TDE_DRV_INT_ALL_STATS = 0x800F001F
} TDE_DRV_INT_STATS_E;

/* ColorKey mode is needed by hardware */
typedef enum mtTDE_DRV_COLORKEY_MODE_E
{
    TDE_DRV_COLORKEY_BACKGROUND = 0,          		/* color key in bkground bitmap */
    TDE_DRV_COLORKEY_FOREGROUND_BEFORE_CLUT = 2,  	/* color key in foreground bitmap,before CLUT */
    TDE_DRV_COLORKEY_FOREGROUND_AFTER_CLUT = 3    	/* color key in bkground bitmap, after CLUT */
} TDE_DRV_COLORKEY_MODE_E;

/* color key setting arguments*/
typedef struct mtTDE_DRV_COLORKEY_CMD_S
{
    TDE_DRV_COLORKEY_MODE_E enColorKeyMode;        	/* color key mode */
    TDE2_COLORKEY_U        unColorKeyValue;       	/* color key value */
} TDE_DRV_COLORKEY_CMD_S;

/* Deficker filting mode */
typedef enum mtTDE_DRV_FLICKER_MODE
{
    TDE_DRV_FIXED_COEF0 = 0,   /* Deficker by fixed coefficient: 0 */
    TDE_DRV_AUTO_FILTER,       /* Deficker by auto filter */
    TDE_DRV_TEST_FILTER        /* Deficker by test filter */
} TDE_DRV_FLICKER_MODE;

/* Block type, equipped register note in numerical value reference */
typedef enum mtTDE_SLICE_TYPE_E
{
    TDE_NO_BLOCK_SLICE_TYPE = 0,         /* No block */
    TDE_FIRST_BLOCK_SLICE_TYPE = 0x3,    /* First block */
    TDE_LAST_BLOCK_SLICE_TYPE = 0x5,     /* Last block */
    TDE_MID_BLOCK_SLICE_TYPE = 0x1       /* Middle block */
} TDE_SLICE_TYPE_E;

/* vertical/horizontal filt mode: available for zoom */
typedef enum mtTDE_DRV_FILTER_MODE_E
{
    TDE_DRV_FILTER_NONE = 0,    /* none filt*/
    TDE_DRV_FILTER_COLOR,       /* filt on color parameter */
    TDE_DRV_FILTER_ALPHA,       /* filt on Alpha value */
    TDE_DRV_FILTER_ALL          /* filt on Alpha and color value */
} TDE_DRV_FILTER_MODE_E;

/* Deflicker operate setting */
typedef struct mtTDE_DRV_FLICKER_CMD_S
{
    TDE_DRV_FLICKER_MODE enDfeMode;
    TDE_DRV_FILTER_MODE_E enFilterV;
    mt_u8            u8Coef0LastLine;
    mt_u8            u8Coef0CurLine;
    mt_u8            u8Coef0NextLine;
    mt_u8            u8Coef1LastLine;
    mt_u8            u8Coef1CurLine;
    mt_u8            u8Coef1NextLine;
    mt_u8            u8Coef2LastLine;
    mt_u8            u8Coef2CurLine;
    mt_u8            u8Coef2NextLine;
    mt_u8            u8Coef3LastLine;
    mt_u8            u8Coef3CurLine;
    mt_u8            u8Coef3NextLine;
    mt_u8            u8Threshold0;
    mt_u8            u8Threshold1;
    mt_u8            u8Threshold2;
    TDE2_DEFLICKER_MODE_E enDeflickerMode;
} TDE_DRV_FLICKER_CMD_S;


/* Zoom operate settings */
typedef struct mtTDE_DRV_RESIZE_CMD_S
{
    mt_u32            u32OffsetX;
    mt_u32            u32OffsetY;
    mt_u32            u32StepH;
    mt_u32            u32StepV;
    MT_BOOL           bCoefSym;
    MT_BOOL           bVfRing;
    MT_BOOL           bHfRing;
    TDE_DRV_FILTER_MODE_E enFilterV;
    TDE_DRV_FILTER_MODE_E enFilterH;
    MT_BOOL           bFirstLineOut;
    MT_BOOL           bLastLineOut;   
} TDE_DRV_RESIZE_CMD_S;

/* Clip Setting */
typedef struct mtTDE_DRV_CLIP_CMD_S
{
    mt_u16  u16ClipStartX;
    mt_u16  u16ClipStartY;
    mt_u16  u16ClipEndX;
    mt_u16  u16ClipEndY;
    MT_BOOL bInsideClip;
} TDE_DRV_CLIP_CMD_S;

/* clut mode */
typedef enum mtTDE_DRV_CLUT_MODE_E
{
    /* color expand */
    TDE_COLOR_EXP_CLUT_MODE = 0,

    /* color correct */
    TDE_COLOR_CORRCT_CLUT_MODE
} TDE_DRV_CLUT_MODE_E;

/* clut setting */
typedef struct mtTDE_DRV_CLUT_CMD_S
{
    TDE_DRV_CLUT_MODE_E enClutMode;
    phys_addr_t         pu8PhyClutAddr;
} TDE_DRV_CLUT_CMD_S;

/* MB Setting */
typedef enum mtTDE_DRV_MB_OPT_MODE_E
{
    TDE_MB_Y_FILTER = 0, 		/* brightness filt */
    TDE_MB_CbCr_FILTER = 2, 	/* chroma filt*/
    TDE_MB_UPSAMP_CONCA = 4,	/* first upsample then contact in chroma and brightness */
    TDE_MB_CONCA_FILTER = 6, 	/* first contact in chroma and brightness and then filt */
} TDE_DRV_MB_OPT_MODE_E;

/* MB Command Setting */
typedef struct mtTDE_DRV_MB_CMD_S
{
    TDE_DRV_MB_OPT_MODE_E enMbMode;        /* MB Operate Mode */
} TDE_DRV_MB_CMD_S;

/* plane mask command setting */
typedef struct mtTDE_DRV_PLMASK_CMD_S
{
    mt_u32 u32Mask;
} TDE_DRV_PLMASK_CMD_S;


/* Color zone convert setting */
typedef struct mtTDE_DRV_CONV_MODE_CMD_S
{
    /* Import Metrix used by color converted:graphic:0/video:1 */
    mt_u8 bInMetrixVid;

    /* Import standard in color convertion:IT-U601:0/ITU-709:1 */
    mt_u8 bInMetrix709;

    /* Export Metrix used by color converted:graphic:0/video:1 */
    mt_u8 bOutMetrixVid;

    /* Import standard in color conversion:IT-U601:0/ITU-709:1 */
    mt_u8 bOutMetrix709;

    /* Enable or unable conversion on importing color zone */
    mt_u8 bInConv;

    /* Enable or unable conversion on exporting color zone */
    mt_u8 bOutConv;
    mt_u8 bInSrc1Conv;

    /* import color conversion direction */
    mt_u8 bInRGB2YC;

    TDE_COLORFMT_CATEGORY_E src1_fmt;
    TDE_COLORFMT_CATEGORY_E src2_fmt;
    TDE_COLORFMT_CATEGORY_E src3_fmt;
    TDE_COLORFMT_CATEGORY_E dst_fmt;
    mt_u32 fmt_mask;   // 1, src1, 1 << 2, src2, 1<<3, src3, 1<<4src,
    
} TDE_DRV_CONV_MODE_CMD_S;

typedef enum mtTDE_DRV_CSC_TYPE_S
{
   NO_CSC = 0x0,
   CSC_RGB_YUV = 2,
   CSC_YUV_RGB = 3,
}TDE_DRV_CSC_TYPE_S;

typedef struct mtTDE_DRV_CSC_INFO_S
{
    TDE_DRV_CSC_TYPE_S  emSrc1ConvStatus;
    TDE_DRV_CSC_TYPE_S  emSrc2ConvStatus;
    TDE_DRV_CSC_TYPE_S  emSrc3ConvStatus;
    TDE_DRV_CSC_TYPE_S  emDstConvStatus;
  }TDE_DRV_CSC_INFO_S;

/* vertical scanning direction */
typedef enum mtTDE_DRV_VSCAN_E
{
    TDE_SCAN_UP_DOWN = 0,	/* form up to down */
    TDE_SCAN_DOWN_UP = 1 	/* form down to up */
} TDE_DRV_VSCAN_E;

/* horizontal scanning direction */
typedef enum mtTDE_DRV_HSCAN_E
{
    TDE_SCAN_LEFT_RIGHT = 0,	/* form left to right */
    TDE_SCAN_RIGHT_LEFT = 1 	/* form right to left */
} TDE_DRV_HSCAN_E;

/* Definition on scanning direction */
typedef struct mtTDE_SCANDIRECTION_S
{
    /* vertical scanning direction */
    TDE_DRV_VSCAN_E enVScan;

    /* horizontal scanning direction */
    TDE_DRV_HSCAN_E enHScan;
} TDE_SCANDIRECTION_S;

/*  Between bitmap info struct setted by driver , by user and hardware info is not all
	the same. eg, bitmap info can be divided into two bitmap info: src1 and src2, 
	which is hardware needs, when user set for MB.

	In MB mode(refer to TDE_INS register), pu8PhyCbCr is not used in driver, but divided into 
	head addr of src1 and src2.
*/

/* TDEV240 version:
1. In nonMB mod, you can support MB. Because adding two membet variables:u32CbCrPhyAddr、u32CbCrPitch,
which for Src1 and Src2 add one assistant channel by each.

2.Support component order in ARGB/RGB format(24 kinds in toal)
,add component order register and member variables:enRgbOrder.
*/

typedef struct mtTDE_DRV_SURFACE_S
{
    /* Bitmap head addr */
    mt_u32 u32PhyAddr;

    /* color format */
    TDE_DRV_COLOR_FMT_E enColorFmt;

    /* ARGB component order */
    TDE_DRV_ARGB_ORDER_E enRgbOrder;

    /* Position X at first */
    mt_u32 u32Xpos;

    /* Position Y at first */
    mt_u32 u32Ypos;

    /* Bitmap Height */
    mt_u32 u32Height;

    /* Bitmap Width */
    mt_u32 u32Width;

    /* Bitmap Pitch */
    mt_u32 u32Pitch;

    /* CbCr component addr */
    mt_u32 u32CbCrPhyAddr;

    /* CbCr pitch*/
    mt_u32 u32CbCrPitch;

    /* alpha max value is 255?or 128? */
    MT_BOOL bAlphaMax255;

    /* Vertical scanning direction */
    TDE_DRV_VSCAN_E enVScan;

    /* Horizontal scanning direction */
    TDE_DRV_HSCAN_E enHScan;
} TDE_DRV_SURFACE_S;

/* MB bitmap info */
typedef struct mtTDE_DRV_MB_S
{
    TDE_DRV_COLOR_FMT_E enMbFmt;
    mt_u32              u32YPhyAddr;
    mt_u32              u32YWidth;
    mt_u32              u32YHeight;
    mt_u32              u32YStride;
    mt_u32              u32CbCrPhyAddr;
    mt_u32              u32CbCrStride;
} TDE_DRV_MB_S;


/* ALU mode*/
typedef enum mtTDE_DRV_ALU_MODE_E
{
    TDE_SRC1_BYPASS = 0,
    TDE_ALU_ROP,
    TDE_ALU_BLEND,
    TDE_ALU_BLEND_SRC2,
    TDE_ALU_MASK_ROP1,
    TDE_ALU_MASK_BLEND,
    TDE_ALU_CONCA,
    TDE_SRC2_BYPASS,
    TDE_ALU_MASK_ROP2,
    
    TDE_ALU_GS_BLUR,              /* gaussian blur */
    TDE_ALU_GRADIENT,            /* Gradient paint  */
    TDE_ALU_MULTIPLY,            /*  image mutiply paint  */
    TDE_ALU_STENCIL,              /*  image stencil paint    */

    TDE_ALU_NONE 		/* register has no setting, used in flag */
} TDE_DRV_ALU_MODE_E;

#if 0
/* cofigure info of node,by TDE_UPDATE order */
typedef struct mtTDE_HWNode_S
{
    mt_u32 u32TDE_INS;
    mt_u32 u32TDE_S1_ADDR;
    mt_u32 u32TDE_S1_TYPE;
    mt_u32 u32TDE_S1_XY;
    mt_u32 u32TDE_S1_FILL;
    mt_u32 u32TDE_S2_ADDR;
    mt_u32 u32TDE_S2_TYPE;
    mt_u32 u32TDE_S2_XY;
    mt_u32 u32TDE_S2_SIZE;
    mt_u32 u32TDE_S2_FILL;
    mt_u32 u32TDE_TAR_ADDR;
    mt_u32 u32TDE_TAR_TYPE;
    mt_u32 u32TDE_TAR_XY;
    mt_u32 u32TDE_TS_SIZE;
    mt_u32 u32TDE_COLOR_CONV;
    mt_u32 u32TDE_CLUT_ADDR;
    mt_u32 u32TDE_2D_RSZ;
    mt_u32 u32TDE_HF_COEF_ADDR;
    mt_u32 u32TDE_VF_COEF_ADDR;
    mt_u32 u32TDE_RSZ_STEP;
    mt_u32 u32TDE_RSZ_Y_OFST;
    mt_u32 u32TDE_RSZ_X_OFST;
    mt_u32 u32TDE_DFE_COEF0;
    mt_u32 u32TDE_DFE_COEF1;
    mt_u32 u32TDE_DFE_COEF2;
    mt_u32 u32TDE_DFE_COEF3;
    mt_u32 u32TDE_ALU;
    mt_u32 u32TDE_CK_MIN;
    mt_u32 u32TDE_CK_MAX;
    mt_u32 u32TDE_CLIP_START;
    mt_u32 u32TDE_CLIP_STOP;
    mt_u32 u32TDE_Y1_ADDR;
    mt_u32 u32TDE_Y1_PITCH;
    mt_u32 u32TDE_Y2_ADDR;
    mt_u32 u32TDE_Y2_PITCH;
    mt_u32 u32TDE_RSZ_VSTEP;
    mt_u32 u32TDE_ARGB_ORDER;
    mt_u32 u32TDE_CK_MASK;
    mt_u32 u32TDE_COLORIZE;
    mt_u32 u32TDE_ALPHA_BLEND;
    mt_u32 u32TDE_ICSC_ADDR;
    mt_u32 u32TDE_OCSC_ADDR;	

  MT_BOOL no_dithering;
  MT_BOOL clip_en;
  mt_u32 dst_ds_mod;
  //0:按像素独立判断有效性
  // 1:相邻两个像素有一个无效，整个都无效
  mt_u32 dst_ds_choose;
  mt_u32 color_exp_mode;
  
  MT_BOOL src_img_en;
  MT_BOOL ex_img_en;
  MT_BOOL bg_img_en;
  MT_BOOL src1_sel;
  MT_BOOL src2_sel;
  MT_BOOL src3_sel; 

  gpe_img_t src_img;
  gpe_img_t dst_img;
  gpe_img_t ex_img;
  gpe_img_t bg_img;

  //csc
  MT_BOOL src1_rgb2yuv_en;
  MT_BOOL src1_yuv2rgb_en;
  MT_BOOL src2_rgb2yuv_en;
  MT_BOOL src2_yuv2rgb_en;
  MT_BOOL src3_rgb2yuv_en;
  MT_BOOL src3_yuv2rgb_en;
  MT_BOOL dst_rgb2yuv_en;
  MT_BOOL dst_yuv2rgb_en;
  
  //palette
  MT_BOOL src1_palt_load_en;
  MT_BOOL src3_palt_load_en;
  
  //tile
  MT_BOOL src_is_tile;
 // tile_cfg_t tile_cfg;
  
  //xylc
  MT_BOOL src_is_xylc;
  xylc_cfg_t xylc_cfg;

  //paint
  MT_BOOL paint_en;
  MT_BOOL paint_pattern_en;
  MT_BOOL paint_gradt_en;
  paint_cfg_t paint;
  mt_u16 * p_gradt_buf;
  
  //alpha map
  MT_BOOL alpha_map_en;
  alpha_map_mod_t alpha_map_mod;
  
  //blend
  MT_BOOL blend_en;
  blend_fact_t  src_blend_fact;
  blend_fact_t  dst_blend_fact;
  
  //rop
  MT_BOOL rop_en;
  rop_mod_t  rop_a_mod; //alpha
  rop_mod_t  rop_c_mod; //color
  mt_u32 rop_pattern;

  //compositor
  MT_BOOL comp_en; //0: rop and blend don't work, 1: one of rop and blend will work
  mt_u32 comp_bp;
  MT_BOOL comp_key_set; //0:keep dst value when colorkey math, 1: output 0xffffffff when colorkey match
  
  //scaler
  MT_BOOL scale_en;
  scale_cfg_t scale_cfg;

  //gaussian blur
  MT_BOOL gaussian_blur_en;
  blur_cfg_t blur_cfg;
  
  //draw image
  MT_BOOL is_draw_stencil;
  MT_BOOL is_draw_multiply;

  //rotator
  MT_BOOL rotator_en;
  aria_rotator_op_t rotator_op;
  aria_rop_ch_t rop_ch;

  //mask
  MT_BOOL src_with_mask;
  mt_u32 src0_buf;
  mt_u32 src0_pitch;
  MT_BOOL dst_with_mask;
  dst_mask_cfg_t dst_mask;

  //for cmdfifo
  MT_BOOL is_cmdfifo;  
  cmdfifo_node_t *p_node;

  mt_u32 cmyk_max;
  mt_u32 cmyk_coef;

} TDE_HWNode_S;

#endif





/* Zoom mode in subnode*/
typedef enum mtTDE_CHILD_SCALE_MODE_E
{
    TDE_CHILD_SCALE_NORM = 0,
    TDE_CHILD_SCALE_MBY,
    TDE_CHILD_SCALE_MBC,
    TDE_CHILD_SCALE_MB_CONCA_H,
    TDE_CHILD_SCALE_MB_CONCA_M,
    TDE_CHILD_SCALE_MB_CONCA_L,
    TDE_CHILD_SCALE_MB_CONCA_CUS,
} TDE_CHILD_SCALE_MODE_E;

/*  Info needed in MB format when Y/CbCr change */
typedef struct mtTDE_MBSTART_ADJ_INFO_S
{
    mt_u32 u32StartInX;     /* Start X,Y imported after MB adjust */
    mt_u32 u32StartInY;
    mt_u32 u32StartOutX;    /* Start X,Y exported after MB adjust */
    mt_u32 u32StartOutY;
    TDE_DRV_COLOR_FMT_E enFmt; /* color format, MB use it to renew position of Y and CbCr */
    TDE_CHILD_SCALE_MODE_E enScaleMode;
} TDE_MBSTART_ADJ_INFO_S;

/* Adjusting info when double source dispart */
typedef struct mtTDE_DOUBLESRC_ADJ_INFO_S
{
    MT_BOOL bDoubleSource;   
    mt_s32 s32DiffX;    /*  s32DiffX = S1x - Tx         */
    mt_s32 s32DiffY;    /*  s32DiffY = S1y - Ty         */ 
}TDE_DOUBLESRC_ADJ_INFO_S;
/*
 * Configure info when set child node 
 * u64Update :
 * _________________________________________
 * |    |    |    |    |    |    |    |    |
 * | ...| 0  | 0  | 1  | 1  | 1  | 1  |  1 |
 * |____|____|____|____|____|____|____|____|
 *                   |    |    |    |    |
 *                  \/   \/   \/   \/   \/
 *                u32Wo u32Xo HOfst u32Wi u32Xi
 *                u32Ho u32Yo VOfst u32Hi u32Yi
 */
typedef struct mtTDE_CHILD_INFO
{
    mt_u32 u32Xi;
    mt_u32 u32Yi;
    mt_u32 u32Wi;
    mt_u32 u32Hi;
    mt_u32 u32HOfst;
    mt_u32 u32VOfst;
    mt_u32 u32Xo;
    mt_u32 u32Yo;
    mt_u32 u32Wo;
    mt_u32 u32Ho;
    mt_u64 u64Update;
    TDE_MBSTART_ADJ_INFO_S stAdjInfo;
    TDE_DOUBLESRC_ADJ_INFO_S stDSAdjInfo;
    TDE_SLICE_TYPE_E enSliceType;
} TDE_CHILD_INFO;

typedef struct mtTDE_DRV_COLORFILL_S
{
    TDE_DRV_COLOR_FMT_E enDrvColorFmt;
    mt_u32              u32FillData;
} TDE_DRV_COLORFILL_S;

typedef enum mtTDE_DRV_INT_E
{
	TDE_DRV_INT_lIST_COMP_AQ = 0x10000,
	TDE_DRV_INT_lIST_COMP_SQ = 0x1,
	TDE_DRV_INT_NODE_COMP_AQ = 0x20000,
	TDE_DRV_INT_NODE_COMP_SQ = 0x2,
	TDE_DRV_INT_SUSPEND_LINE_AQ = 0x40000,
	TDE_DRV_INT_SUSPEND_LINE_SQ = 0x4,
 	TDE_DRV_INT_HEAD_UPDATE_SQ = 0x10,
 	TDE_DRV_INT_ERROR = 0x80000000,
}TDE_DRV_INT_E;

typedef TDE2_OUTALPHA_FROM_E TDE_DRV_OUTALPHA_FROM_E;

typedef enum mtTDE_DRV_SRC_E
{
    TDE_DRV_SRC_NONE = 0,
    TDE_DRV_SRC_S1 = 0x1,
    TDE_DRV_SRC_S2 = 0x2,
    TDE_DRV_SRC_T = 0x4,
}TDE_DRV_SRC_E;

typedef struct mtTDE_FILTER_OPT
{
    mt_u32  u32HStep;
    mt_u32  u32VStep;
    mt_s32  s32HOffset;
    mt_s32  s32VOffset;
    mt_u32  u32Bppi;
    mt_u32  u32WorkBufNum;
    mt_u32  bBadLastPix;        /* while blocking, last point of each block is if effective*/
    MT_BOOL bVRing;
    MT_BOOL bHRing;
    MT_BOOL bEvenStartInX;      /* when input need up_sample, bEvenStartInX is set to mt_TRUE */
    MT_BOOL bEvenStartOutX;     /* when input need drop_sample, bEvenStartInX is set to mt_TRUE */
    MT_BOOL bCoefSym;           /* coefficient is if symmetrical, use filed ground */
    MT_BOOL b2OptCbCr;
    TDE_SCANDIRECTION_S stSrcDire;
    TDE_SCANDIRECTION_S stDstDire;
    TDE_MBSTART_ADJ_INFO_S stAdjInfo;
    TDE_DOUBLESRC_ADJ_INFO_S stDSAdjInfo;
    TDE_DRV_FILTER_MODE_E enFilterMode;
    MT_BOOL bFirstLineOut;
    MT_BOOL bLastLineOut;
} TDE_FILTER_OPT;


/****************************************************************************/
/*                             TDE register macro definition                            */
/****************************************************************************/
#define TDE_RST 0x0804
#define TDE_CTRL 0x0808
#define TDE_AQ_ADDR 0x0810
#define TDE_STA 0x0814
#define TDE_INT 0x0818
#define TDE_AQ_CTRL 0x082C
#define TDE_AQ_NADDR 0x0830
#define TDE_AQ_UPDATE 0x0834
#define TDE_BUS_LIMITER 0x0844

#define TDE_AQ_UPDATE2 0x085c
#define TDE_REQ_TH 0x0868
#define TDE_AXI_ID 0x086c

#define TDE_AQ_COMP_NODE_MASK_EN 4 /*0100：Enable to interrupt when complete current node in AQ */
#define TDE_AQ_COMP_LIST_MASK_EN 8 /*1000：Enable to interrupt  in complete AQ */

/*Handle responsed with node */
/*
	Add 4 byte pointer in physical buffer header,to save software node;
	For need to consult current executing software node,but register can only
	give the physical addr of it.
*/
#define TDE_NODE_HEAD_BYTE 16

/* Next node addr、update info、occupied bytes */
#define TDE_NODE_TAIL_BYTE 12

/****************************************************************************/
/*                             TDE hal ctl functions define                 */
/****************************************************************************/

/*****************************************************************************
* Function:      TdeHalInit
* Description:   main used in mapping TDE basic addr
* Input:         u32BaseAddr:Register basic addr
* Output:        None
* Return:        Success/Failure
* Others:        None
*****************************************************************************/
mt_s32  TdeHalInit(mt_u32 u32BaseAddr);

/*****************************************************************************
* Function:      TdeHalOpen
* Description:   main used in initialize needed register 
* Input:         None
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalOpen(mt_void);

/*****************************************************************************
* Function:      TdeHalRelease
* Description:   main used in release TDE basic addr by mapping
* Input:         None
* Output:        None
* Return:        Success/Failure
* Others:        None
*****************************************************************************/
mt_void TdeHalRelease(mt_void);

/*****************************************************************************
* Function:      TdeHalCtlIsIdle
* Description:   Query if TDE is in IDLE state or not
* Input:         None
* Output:        None
* Return:        True: Idle/False: Busy
* Others:        None
*****************************************************************************/
MT_BOOL TdeHalCtlIsIdle(mt_void);

/*****************************************************************************
* Function:      TdeHalCtlIsIdleSafely
* Description:   cycle many times, to make sure TDE is in IDLE state
* Input:         None
* Output:        None
* Return:        True: Idle/False: Busy
* Others:        None
*****************************************************************************/
MT_BOOL TdeHalCtlIsIdleSafely(mt_void);


mt_void TdeHalClearInt(mt_u32 int_value);

/*****************************************************************************
* Function:      TdeHalCtlIntMask
* Description:   Get Sq/Aq interrupt state
* Input:         None
* Output:        None
* Return:        Sq/Aq interrupt state
* Others:        None
*****************************************************************************/
mt_u32  TdeHalCtlIntStats(mt_void);

/*****************************************************************************
* Function:      TdeHalCtlReset
* Description:   soft replace, reset interrupt state
* Input:         None
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalCtlReset(mt_void);

/*****************************************************************************
* Function:      TdeHalCtlIntClear
* Description:   Reset relevant interrupt state
* Input:         u32Stats: Reset relevant interrupt state
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalCtlIntClear(mt_u32 u32Stats);



/****************************************************************************/
/*                             TDE hal node functions define                */
/****************************************************************************/

/*****************************************************************************
* Function:      TdeHalNodeInitNd
* Description:   Initialize struct, TDE operate node is needed
* Input:         pstHWNode:Node struct pointer.
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeInitNd(TDE_HWNode_S** pstHWNode);

/*****************************************************************************
* Function:      TdeHalFreeNodeBuf
* Description:   Free TDE operate node buffer
* Input:         pstHWNode:Node struct pointer.
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalFreeNodeBuf(TDE_HWNode_S* pstHWNode);

/*****************************************************************************
* Function:      TdeHalNodeInitChildNd
* Description:   Initialize TDE child node 
* Input:         pstHWNode:Node struct pointer.
                     u32TDE_CLIP_START:The start position of the clip rect.
                     u32TDE_CLIP_STOP:The stop position of the clip rect
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeInitChildNd(TDE_HWNode_S* pHWNode, mt_u32 u32TDE_CLIP_START, mt_u32 u32TDE_CLIP_STOP);



/*****************************************************************************
* Function:      TdeHalNodeEnableCompleteInt
* Description:   Complete interrupt by using node's operate
* Input:         pBuf: Buffer need node be operated
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeEnableCompleteInt(mt_void* pBuf);



/*****************************************************************************
* Function:      TdeHalNodeSetSrc1
* Description:   Set Src1 bitmap info
* Input:         pHWNode: Node struct pointer used in cache by software
*                pDrvSurface: bitmap info used in setting
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetSrc1(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvSurface);

/*****************************************************************************
* Function:      TdeHalNodeSetSrc2
* Description:   Set Src2 bitmap info
* Input:         pHWNode: Node struct pointer used in cache by software
*                pDrvSurface: bitmap info used in setting
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetSrc2(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvSurface);

/*****************************************************************************
* Function:      TdeHalNodeSetSrc3
* Description:   Set Src2 bitmap info
* Input:         pHWNode: Node struct pointer used in cache by software
*                   pDrvSurface: bitmap info used in setting
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetSrc3(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvSurface);


/*****************************************************************************
* Function:      TdeHalNodeSetSrcMbY
* Description:   Set brightness information in MB source bitmap
* Input:         pHWNode: Node struct pointer used in cache by software
*                pDrvMbY: bitmap brightness information used in setting
*                enMbOpt: MB mode option
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetSrcMbY(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvMbY, TDE_DRV_MB_OPT_MODE_E enMbOpt);

/*****************************************************************************
* Function:      TdeHalNodeSetSrcMbCbCr
* Description:   set chroma info in MB source bitmap
* Input:         pHWNode: Node struct pointer used in cache by software
*                pDrvMbCbCr: CbCr info
*                enMbOpt: MB mode option
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetSrcMbCbCr(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvMbCbCr, TDE_DRV_MB_OPT_MODE_E enMbOpt);

/*****************************************************************************
* Function:      TdeHalNodeSetTgt
* Description:   Set target bitmap information
* Input:         pHWNode: Node struct pointer used in cache by software
*                pDrvSurface: bitmap information used in setting
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetTgt(TDE_HWNode_S* pHWNode, TDE_DRV_SURFACE_S* pDrvSurface, TDE_DRV_OUTALPHA_FROM_E enAlphaFrom);

/*****************************************************************************
* Function:      TdeHalNodeSetBaseOperate
* Description:   Set basic operate type
* Input:         pHWNode: Node struct pointer used in cache by software
*                enMode: basic operate mode
*                enAlu: ALU mode
*                u32FillData: if basic mode have fill operate ,read this value
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetBaseOperate(TDE_HWNode_S* pHWNode, TDE_DRV_BASEOPT_MODE_E enMode,
                                 TDE_DRV_ALU_MODE_E enAlu, TDE_DRV_COLORFILL_S *pstColorFill);

/*****************************************************************************
* Function:      TdeHalNodeSetGlobalAlpha
* Description:   Set Alpha mixed arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                u8Alpha: Alpha mixed setting value
*                       bEnable: Enable to use global alpha
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetGlobalAlpha(TDE_HWNode_S* pHWNode, mt_u8 u8Alpha, MT_BOOL bEnable);

/*****************************************************************************
* Function:      TdeHalNodeSetExpAlpha
* Description:   When expand Alpha in RGB5551. to alpha0 and alpha1
* Input:         pHWNode: Node struct pointer used in cache by software
*                u8Alpha: Alpha mixed setting value
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetExpAlpha(TDE_HWNode_S* pHWNode, TDE_DRV_SRC_E enSrc, mt_u8 u8Alpha0, mt_u8 u8Alpha1);
/*****************************************************************************
* Function:      TdeHalNodeSetAlphaBorder
* Description:   Enable to set Alpha can be bordered
* Input:         pHWNode: Node struct pointer used in cache by software
*                bEnable: Enanle to border Alpha
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetAlphaBorder(TDE_HWNode_S* pHWNode, MT_BOOL bVEnable, MT_BOOL bHEnable);

/*****************************************************************************
* Function:      TdeHalNodeSetRop
* Description:   Set ROP arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                enRopCode: ROP operator
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetRop(TDE_HWNode_S* pHWNode, TDE2_ROP_CODE_E enRgbRop, TDE2_ROP_CODE_E enAlphaRop);

/*****************************************************************************
* Function:      TdeHalNodeSetBlend
* Description:   Set blend operate arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                stBlendOpt:blend operate option
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetBlend(TDE_HWNode_S *pHWNode, TDE2_BLEND_OPT_S *pstBlendOpt);

/*****************************************************************************
* Function:      TdeHalNodeSetColorize
* Description:   Set blend operate arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                u32Colorize:Co
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetColorize(TDE_HWNode_S *pHWNode, mt_u32 u32Colorize);

/*****************************************************************************
* Function:      TdeHalNodeEnableAlphaRop
* Description:   Enable to blend Rop operate
* Input:         pHWNode: Node struct pointer used in cache by software
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeEnableAlphaRop(TDE_HWNode_S *pHWNode);

/*****************************************************************************
* Function:      TdeHalNodeSetColorExp
* Description:   Set color expand or adjust argument
* Input:         pHWNode: Node struct pointer used in cache by software
*                pClutCmd: Clut operate atguments
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetClutOpt(TDE_HWNode_S* pHWNode, TDE_DRV_CLUT_CMD_S* pClutCmd, MT_BOOL bReload);
mt_s32 TdeHalNodeSetClutOpt2(TDE_HWNode_S* pHWNode, TDE_DRV_CLUT_CMD_S* pClutCmd, MT_BOOL bReload);

/*****************************************************************************
* Function:      TdeHalNodeSetColorKey
* Description:   Set arguments needed by color key,according current color format
* Input:         pHWNode: Node struct pointer used in cache by software
*                enFmt: color format
*                pColorKey: color key pointer
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetColorKey(TDE_HWNode_S* pHWNode, TDE_COLORFMT_CATEGORY_E enFmtCat, 
                              TDE_DRV_COLORKEY_CMD_S* pColorKey);

/*****************************************************************************
* Function:      TdeHalNodeSetClipping
* Description:   Set rectangle's clip operated arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                pClip: Clip rectangle range
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetClipping(TDE_HWNode_S* pHWNode, TDE_DRV_CLIP_CMD_S* pClip);

/*****************************************************************************
* Function:      TdeHalNodeSetFlicker
* Description:   set deflicker fliter operate arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                pFlicker: Deflicker coefficient
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetFlicker(TDE_HWNode_S* pHWNode, TDE_DRV_FLICKER_CMD_S* pFlicker);

/*****************************************************************************
* Function:      TdeHalNodeSetResize
* Description:   set zoom fliter operate arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                pResize: zoom coefficient
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetResize(TDE_HWNode_S* pHWNode,TDE_FILTER_OPT* pstFilterOpt, TDE_NODE_SUBM_TYPE_E enNodeType);
/*****************************************************************************
* Function:      TdeHalNodeSetColorConvert
* Description:   set color zone conversion arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                pConv: color zone conversion argument
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeSetColorConvert(TDE_HWNode_S* pHWNode, TDE_DRV_CONV_MODE_CMD_S* pConv);

/*****************************************************************************
* Function:      TdeHalNodeAddChild
* Description:   Add child node when filter operate
* Input:         pHWNode: Node struct pointer used in cache by software
*                pChildInfo: add child node's configue information
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeAddChild(TDE_HWNode_S* pHWNode,TDE_CHILD_INFO* pChildInfo);

/*****************************************************************************
* Function:      TdeHalNodeSetMbMode
* Description:   set MB operate arguments
* Input:         pHWNode: Node struct pointer used in cache by software
*                pMbCmd: MB operate arguments
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalNodeSetMbMode(TDE_HWNode_S* pHWNode, TDE_DRV_MB_CMD_S* pMbCmd);


/*****************************************************************************
* Function:      TDeHalNodeSetCsc
* Description:  Set CSC's first optional argument
* Input:         pHWNode:Node struct pointer used in cache by software
			stCscOpt:CSC first optional argument
* Output:        None
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TDeHalNodeSetCsc(TDE_HWNode_S* pHWNode, TDE2_CSC_OPT_S stCscOpt);


/*****************************************************************************
* Function:      TdeHalSetDeflicerLevel
* Description:   set deflicker level
* Input:         eDeflickerLevel:deflicker level
* Output:        None
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalSetDeflicerLevel(TDE_DEFLICKER_LEVEL_E eDeflickerLevel);

/*****************************************************************************
* Function:      TdeHalGetDeflicerLevel
* Description:   Get deflicker level
* Input:        
* Output:        pDeflicerLevel:deflicker level
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalGetDeflicerLevel(TDE_DEFLICKER_LEVEL_E *pDeflicerLevel);

/*****************************************************************************
* Function:      TdeHalSetAlphaThreshold
* Description:   Set alpha threshold 
* Input:         u8ThresholdValue:alpha threshold
* Output:        
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalSetAlphaThreshold(mt_u8 u8ThresholdValue);

/*****************************************************************************
* Function:      TdeHalGetAlphaThreshold
* Description:   Get alpha threshold 
* Input:         
* Output:        pu8ThresholdValue:alpha threshold
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalGetAlphaThreshold(mt_u8 * pu8ThresholdValue);

/*****************************************************************************
* Function:      TdeHalGetAlphaThresholdState
* Description:   Set alpha threshold to judge if open or close
* Input:         bEnAlphaThreshold:alpha switch status
* Output:        None
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalSetAlphaThresholdState(MT_BOOL bEnAlphaThreshold);

/*****************************************************************************
* Function:      TdeHalGetAlphaThresholdState
* Description:   Get alpha threshold to judge if open or close
* Input:         None
* Output:        pbEnAlphaThreshold:alpha switch status
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalGetAlphaThresholdState(MT_BOOL *pbEnAlphaThreshold);

/*****************************************************************************
* Function:      TdeHalNodeExecute
* Description:   Start TDE list
* Input:   
*                u32NodePhyAddr: list first node address
*                u64Update: first node updating flag
*                bAqUseBuff: if use temporary buffer
* Output:        None
* Return:        Success / Fail
* Others:        None
*****************************************************************************/
mt_s32 TdeHalNodeExecute(mt_u32 u32NodePhyAddr, mt_u64 u64Update, MT_BOOL bAqUseBuff);


/*****************************************************************************
* Function:      TdeHalCurNode
* Description:   Get current node in register
* Input:         None
* Output:        node physical address
* Return:        None
* Others:        None
*****************************************************************************/
mt_u32 TdeHalCurNode(mt_void);
/*****************************************************************************
* Function:      TdeHalResumeInit
* Description:   Resume the hardware by software ,initialize the TDE device
* Input:         None
* Output:        node 
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalResumeInit(mt_void);

/*****************************************************************************
* Function:      TdeHalSuspend
* Description:   Suspend the hardware 
* Input:         None
* Output:        node 
* Return:        None
* Others:        None
*****************************************************************************/
mt_void TdeHalSuspend(mt_void);

//MT_BOOL gpe_set_parameter(TDE_HWNode_S* p_ctx, gpe_param_t *p_param);

//param: only_used :  only print the used 
mt_void TdeHalHwNodePrint(ARIA_TDE_HWNODE_t *p_hw_node, MT_BOOL only_used);  


//set scaler paramters 
mt_s32  TdeHalNodeSetScaler(TDE_HWNode_S* pHWNode,  TDE_DRV_SURFACE_S *p_src, TDE_DRV_SURFACE_S *p_dst);

mt_s32 TdeHalNodeSetBlurInfo(TDE_HWNode_S* pHWNode,  TDE2_GS_BLUR_S *p_blur_cfg);


mt_s32 TdeHalNodeSetRegularRotatorOpt(TDE_HWNode_S *pHWNode, TDE2_ROTATOR_TYPE_S rotator_type);


mt_u32 TdeHalReadReg(mt_u32 offset);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif  /* _TDE_HAL_H_ */
