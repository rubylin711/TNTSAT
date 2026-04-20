/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _VDP_DEFINE_H_
#define _VDP_DEFINE_H_

#include "mt_type.h"

// test switch

#define OPTM_FPGA_TEST 1

#define OPTM_VID_MAX 4
#define OPTM_GFX_MAX 6
#define OPTM_WBC_MAX 1
#define OPTM_CHN_MAX 2
#define OPTM_GP_MAX  2
#define OPTM_VP_MAX  2
#define OPTM_CBM_MAX  (OPTM_VP_MAX + OPTM_GP_MAX)

#define OPTM_MAX_LAYER OPTM_VID_MAX + OPTM_GFX_MAX
#define OPTM_MIX_PARA  8


#define OPTM_VID_OFFSET 0x800 
#define OPTM_VP_OFFSET  0x800 
#define OPTM_GFX_OFFSET 0x800 
#define OPTM_GP_OFFSET  0x800 
#define OPTM_MIX_OFFSET 0x100 
#define OPTM_CHN_OFFSET 0x400

#define OPTM_WBC_OFFSET 0x400

#define OPTM_WBC_GP0_SEL 1 

#define OPTM_MAX_VALUE 255

#define MT_TYPE   1


#define HD_LOGO_LAYE_ADDR 0xf8cc6800
#define SD_LOGO_LAYE_ADDR 0xf8cc7800
#define SD_LAYE_ADDR   0xf8cc8000
#define WBC_LAYE_ADDR  0xf8cca800
#if !(OPTM_FPGA_TEST)
#define MyAssert(x) { if (!(x)) { printf("\nErr @ (%s, %d)\n", __FILE__, __LINE__); \
                                                         exit(-__LINE__); } }
#else
#define MyAssert(x) 
#endif

/*! 
  OSD endian mode 0
  00: {byte3, byte2, byte1, byte0} (no byte order change)
  */
#define DISP_OSD_ENDIAN_MODE_0           0x0

/*! 
  OSD endian mode 1
  01: {byte2, byte3, byte0, byte1}
  */
#define DISP_OSD_ENDIAN_MODE_1           0x1

/*! 
  OSD endian mode 2
  10: {byte1, byte0, byte3, byte2}
  */
#define DISP_OSD_ENDIAN_MODE_2           0x2

/*! 
  OSD endian mode 3
  11: {byte0, byte1, byte2, byte3}
  */
#define DISP_OSD_ENDIAN_MODE_3           0x3

/*! 
  OSD palette endian mode 0
  00: {byte3, byte2, byte1, byte0} (no byte order change)
  */
#define DISP_OSD_PALETTE_ENDIAN_MODE_0           0x0

/*! 
  OSD pallete endian mode 1
  01: {byte2, byte3, byte0, byte1}
  */
#define DISP_OSD_PALETTE_ENDIAN_MODE_1           0x1

/*! 
  OSD pallete endian mode 2
  10: {byte1, byte0, byte3, byte2}
  */
#define DISP_OSD_PALETTE_ENDIAN_MODE_2           0x2

/*! 
  OSD pallete endian mode 3
  11: {byte0, byte1, byte2, byte3}
  */
#define DISP_OSD_PALETTE_ENDIAN_MODE_3           0x3

/*! 
  OSD alpha position MSB
  alpha is on MSB (byte 3). For example, aRGB, aYUV
  */
#define DISP_OSD_ALPHA_POS_MSB           0x0

/*! 
  OSD alpha position LSB
  alpha is on LSB (byte 0). For example, RGBa, YUVa, etc
  */
#define DISP_OSD_ALPHA_POS_LSB           0x1

/*! 
  OSD alpha blending NOT pre multiply
  Pixel data have NOT been multiplied by their corresponding alpha values
  */
#define DISP_OSD_ALPHA_BLEND_NO_PRE_MUL          0x0

/*! 
  OSD alpha blending pre multiply
  Pixel data have been multiplied by their corresponding alpha values
  */
#define DISP_OSD_ALPHA_BLEND_PRE_MUL          0x1


/*! 
  OSD defualt level of alpha 0
  */
#define DISP_OSD_ALPHA0_DEFAULT           0xFF
/*!
  OSD defualt level of alpha 1
  */
#define DISP_OSD_ALPHA1_DEFAULT           0xFF

/*! 
  OSD region alpha mode 
  */
#define DISP_OSD_REGION_ALPHA_MODE        0x1
/*!
  OSD plane alpha mode
  */
#define DISP_OSD_PLANE_ALPHA_MODE         0x0

/*! 
  YUV color space. 
  */
#define DISP_OSD_COLORSPACE_YUV           0x00
/*!
  RGB color space
  */
#define DISP_OSD_COLORSPACE_RGB           0x01
/*!
  YUV 422  444 color space
  */
#define DISP_OSD_COLORSPACE_YUV422_444    0x02
/*!
  RGB true color space
  */
#define DISP_OSD_COLORSPACE_RGBTRUE       0x03

/*!
  2bit indexed colors. 
  */
#define DISP_OSD_CLUTCODE_2BIT            0x00
/*!
  4bit indexed color
  */
#define DISP_OSD_CLUTCODE_4BIT            0x01
/*!
  8bit indexed color
  */
#define DISP_OSD_CLUTCODE_8BIT            0x02
/*!
  CLUT44 indexed color
  */
#define DISP_OSD_CLUTCODE_LUT44           0x05
/*!
  CLUT88 indexed color
  */
#define DISP_OSD_CLUTCODE_LUT88           0x06

/*! 
  RGB233 true folor. 
  */
#define DISP_OSD_TRUECODE_RGB233          0x00
/*!
  RGB565 true color
  */
#define DISP_OSD_TRUECODE_RGB565          0x01
/*!
  RGB4444 true color
  */
#define DISP_OSD_TRUECODE_RGB4444         0x02
/*!
  RGB1555 true color
  */
#define DISP_OSD_TRUECODE_RGB1555         0x03
/*!
  aRGB8888 or RGBa true color
  */
#define DISP_OSD_TRUECODE_RGB8888         0x04
/*!
  semi-planar true color
  */
#define DISP_OSD_TRUECODE_SP            0x05
/*!
  uyvy true color
  */
#define DISP_OSD_TRUECODE_UYVY            0x06
/*!
  ayuv8888 or yuva true color
  */
#define DISP_OSD_TRUECODE_AYUV8888        0x07

/*! 
  Sub default level of alpha 0. 
  */
#define DISP_SUB_ALPHA0_DEFAULT           0xFF
/*!
  Sub default level of alpha 1
  */
#define DISP_SUB_ALPHA1_DEFAULT           0xFF

/*! 
  Region alpha mode. 
  */
#define DISP_SUB_REGION_ALPHA_MODE        0x1
/*!
  Plane alpha mode
  */
#define DISP_SUB_PLANE_ALPHA_MODE         0x0

/*
  Semi-Planar format, yuv422_1x2.
*/
#define DISP_SP_YUV422_1x2       0x0

/*
  Semi-Planar format, yuv422_2x1.
*/
#define DISP_SP_YUV422_2x1       0x1

/*
  Semi-Planar format, yuv420_1x1.
*/
#define DISP_SP_YUV420_1x1       0x2

/*!
  The scaler coeff table size.
  */
#define SINGLE_SCALE_COEFF_TABLE_SIZE           0x100

typedef enum tagVDP_LAYER_GFX_E
{
    OPTM_VDP_LAYER_GFX0  = 0,
    OPTM_VDP_LAYER_GFX1  = 1,
    OPTM_VDP_LAYER_GFX2  = 2,
    OPTM_VDP_LAYER_GFX3  = 3,
    OPTM_VDP_LAYER_GFX4  = 4,
    OPTM_VDP_LAYER_GFX5  = 5,
    
    OPTM_VDP_LAYER_GFX_BUTT

} OPTM_VDP_LAYER_GFX_E;

typedef enum tagVDP_LAYER_GP_E
{
    OPTM_VDP_LAYER_GP0   = 0,
    OPTM_VDP_LAYER_GP1   = 1,
    
    OPTM_VDP_LAYER_GP_BUTT

} OPTM_VDP_LAYER_GP_E;

typedef enum tagVDP_LAYER_WBC_E
{
    OPTM_VDP_LAYER_WBC_GP0  = 0,
    OPTM_VDP_LAYER_WBC_HD0  = 1,
    OPTM_VDP_LAYER_WBC_G0   = 2,
    OPTM_VDP_LAYER_WBC_G4   = 3,
    OPTM_VDP_LAYER_WBC_BUTT

} OPTM_VDP_LAYER_WBC_E;

typedef enum tagOPTM_VDP_INTMSK_E
{
    OPTM_VDP_INTMSK_NONE      = 0,
	OPTM_VDP_INTMSK_WBC_GP0_INT = 0x100,
    OPTM_VDP_INTMSK_WBC_G0_INT  = 0x400,
    OPTM_VDP_INTMSK_WBC_G4_INT  = 0x800,
    OPTM_VDP_INTMSK_BUTT,
}OPTM_VDP_INTMSK_E;


typedef enum tagVDP_CHN_E
{
    OPTM_VDP_CHN_DHD0    = 0,
    OPTM_VDP_CHN_DHD1    = 1,
    OPTM_VDP_CHN_WBC0    = 2,
    OPTM_VDP_CHN_WBC1    = 3,
    OPTM_VDP_CHN_WBC2    = 4,
    OPTM_VDP_CHN_WBC3    = 5,
    OPTM_VDP_CHN_NONE    = 6,
    OPTM_VDP_CHN_BUTT

} OPTM_VDP_CHN_E;


typedef enum tagVDP_VID_IFMT_E
{
    OPTM_VDP_VID_IFMT_SP_400      = 0x1,
    OPTM_VDP_VID_IFMT_SP_420      = 0x3,
    OPTM_VDP_VID_IFMT_SP_422      = 0x4,
    OPTM_VDP_VID_IFMT_SP_444      = 0x5,
    OPTM_VDP_VID_IFMT_PKG_UYVY    = 0x9,
    OPTM_VDP_VID_IFMT_PKG_YUYV    = 0xa,
    OPTM_VDP_VID_IFMT_PKG_YVYU    = 0xb,

    OPTM_VDP_VID_IFMT_BUTT        
    
}OPTM_VDP_VID_IFMT_E;
 
typedef enum tagVDP_GFX_IFMT_E
{
    VDP_GFX_IFMT_CLUT_1BPP   = 0x00,
    VDP_GFX_IFMT_CLUT_2BPP   = 0x10,
    VDP_GFX_IFMT_CLUT_4BPP   = 0x20,
    VDP_GFX_IFMT_CLUT_8BPP   = 0x30,

    VDP_GFX_IFMT_ACLUT_44    = 0x38,

    VDP_GFX_IFMT_RGB_444     = 0x40,
    VDP_GFX_IFMT_RGB_555     = 0x41,
    VDP_GFX_IFMT_RGB_565     = 0x42,

    VDP_GFX_IFMT_PKG_UYVY    = 0x43,
    VDP_GFX_IFMT_PKG_YUYV    = 0x44,
    VDP_GFX_IFMT_PKG_YVYU    = 0x45,

    VDP_GFX_IFMT_ACLUT_88    = 0x46,
    VDP_GFX_IFMT_ARGB_4444   = 0x48,
    VDP_GFX_IFMT_ARGB_1555   = 0x49,

    VDP_GFX_IFMT_RGB_888     = 0x50,//24bpp
    VDP_GFX_IFMT_YCBCR_888   = 0x51,//24bpp
    VDP_GFX_IFMT_ARGB_8565   = 0x5a,//24bpp

    VDP_GFX_IFMT_KRGB_888    = 0x60,
    VDP_GFX_IFMT_ARGB_8888   = 0x68,
    VDP_GFX_IFMT_AYCBCR_8888 = 0x69,

    VDP_GFX_IFMT_RGBA_4444   = 0xc8,
    VDP_GFX_IFMT_RGBA_5551   = 0xc9,
    VDP_GFX_IFMT_RGBA_5658   = 0xda,//24bpp
    VDP_GFX_IFMT_RGBA_8888   = 0xe8,
    VDP_GFX_IFMT_YCBCRA_8888 = 0xe9,
    VDP_GFX_IFMT_ABGR_8888   = 0xef,

    VDP_GFX_IFMT_BUTT        
    
}OPTM_VDP_GFX_IFMT_E;
 
typedef enum tagVDP_PROC_FMT_E
{
    VDP_PROC_FMT_SP_422      = 0x0,
    VDP_PROC_FMT_SP_420      = 0x1,
    VDP_PROC_FMT_SP_444      = 0x2,

    VDP_PROC_FMT_BUTT        
    
}OPTM_VDP_PROC_FMT_E;

typedef enum tagVDP_WBC_FMT_E
{
    VDP_WBC_OFMT_PKG_UYVY = 0,
    VDP_WBC_OFMT_PKG_YUYV = 1,
    VDP_WBC_OFMT_PKG_YVYU = 2,
    VDP_WBC_OFMT_ARGB8888 = 3,
    VDP_WBC_OFMT_SP420   = 4,
    VDP_WBC_OFMT_SP422   = 5,
    
    VDP_WBC_OFMT_BUUT

}OPTM_VDP_WBC_OFMT_E;



typedef enum tagVDP_DATA_RMODE_E
{
    VDP_RMODE_INTERLACE = 0,
    VDP_RMODE_PROGRESSIVE,
    VDP_RMODE_TOP,
    VDP_RMODE_BOTTOM,
    VDP_RMODE_BUTT

} OPTM_VDP_DATA_RMODE_E;

typedef enum 
{
    VDP_ZME_MODE_HOR = 0,
    VDP_ZME_MODE_VER,

    VDP_ZME_MODE_HORL,  
    VDP_ZME_MODE_HORC,  
    VDP_ZME_MODE_VERL,
    VDP_ZME_MODE_VERC,

    VDP_ZME_MODE_ALPHA,
    VDP_ZME_MODE_ALPHAV,
    VDP_ZME_MODE_VERT,
    VDP_ZME_MODE_VERB,

    VDP_ZME_MODE_ALL,
    VDP_ZME_MODE_NONL,
    VDP_ZME_MODE_BUTT
      
}OPTM_VDP_ZME_MODE_E;

typedef enum 
{
    VDP_TI_MODE_LUM = 0,  
    VDP_TI_MODE_CHM,  

    VDP_TI_MODE_ALL,
    VDP_TI_MODE_NON,
    VDP_TI_MODE_BUTT
      
}OPTM_VDP_TI_MODE_E;



typedef enum tagVDP_ZME_ORDER_E
{
    VDP_ZME_ORDER_HV = 0x0,
    VDP_ZME_ORDER_VH = 0x1,

    VDP_ZME_ORDER_BUTT
} OPTM_VDP_ZME_ORDER_E;

typedef enum tagVDP_GP_ORDER_E
{
    VDP_GP_ORDER_NULL     = 0x0,
    VDP_GP_ORDER_CSC      = 0x1,
    VDP_GP_ORDER_ZME      = 0x2,
    VDP_GP_ORDER_CSC_ZME  = 0x3,
    VDP_GP_ORDER_ZME_CSC  = 0x4,

    VDP_GP_ORDER_BUTT
} OPTM_VDP_GP_ORDER_E;


typedef enum tagVDP_DITHER_E
{
    VDP_DITHER_DROP_10   = 0,
    VDP_DITHER_TMP_10    = 1,
    VDP_DITHER_SPA_10    = 2,
    VDP_DITHER_TMP_SPA_8 = 3,
    VDP_DITHER_ROUND_10  = 4,
    VDP_DITHER_ROUND_8   = 5,
    VDP_DITHER_DISEN     = 6,
    VDP_DITHER_BUTT
} OPTM_VDP_DITHER_E;

typedef struct
{
    mt_u32 dither_coef0;
    mt_u32 dither_coef1;
    mt_u32 dither_coef2;
    mt_u32 dither_coef3;

    mt_u32 dither_coef4;
    mt_u32 dither_coef5;
    mt_u32 dither_coef6;
    mt_u32 dither_coef7;
} OPTM_VDP_DITHER_COEF_S;

typedef struct tagVDP_DISP_RECT_S
{
    mt_u32 u32SX;   // source horizontal start position
    mt_u32 u32SY;   // source vertical start position
    
    mt_u32 u32DXS;  // dispaly horizontal start position
    mt_u32 u32DYS;  // display vertical start position

    mt_u32 u32DXL;  // dispaly horizontal end position
    mt_u32 u32DYL;  // display vertical end position
    
    mt_u32 u32VX;   // video horizontal start position
    mt_u32 u32VY;   // video vertical start position

    mt_u32 u32VXL;  // video horizontal start position
    mt_u32 u32VYL;  // video vertical start position
    
    mt_u32 u32IWth; // input width
    mt_u32 u32IHgt; // input height
    mt_u32 u32OWth; // output width
    mt_u32 u32OHgt; // output height

} OPTM_VDP_DISP_RECT_S;


typedef struct
{
    mt_u32 u32X;
    mt_u32 u32Y;

    mt_u32 u32Wth;
    mt_u32 u32Hgt;
    
} OPTM_VDP_RECT_S;

typedef enum tagVDP_GP_PARA_E
{
    VDP_GP_PARA_ZME_HOR = 0,
    VDP_GP_PARA_ZME_VER   ,
    
    VDP_GP_PARA_ZME_HORL  ,
    VDP_GP_PARA_ZME_HORC  ,
    VDP_GP_PARA_ZME_VERL  ,
    VDP_GP_PARA_ZME_VERC  ,

    VDP_GP_GTI_PARA_ZME_HORL  ,
    VDP_GP_GTI_PARA_ZME_HORC  ,
    VDP_GP_GTI_PARA_ZME_VERL  ,
    VDP_GP_GTI_PARA_ZME_VERC  ,
    
    VDP_GP_PARA_BUTT
} OPTM_VDP_GP_PARA_E;

typedef enum tagVDP_WBC_PARA_E
{
    VDP_WBC_PARA_ZME_HOR = 0,
    VDP_WBC_PARA_ZME_VER   ,
    
    VDP_WBC_PARA_ZME_HORL  ,
    VDP_WBC_PARA_ZME_HORC  ,
    VDP_WBC_PARA_ZME_VERL  ,
    VDP_WBC_PARA_ZME_VERC  ,

    VDP_WBC_GTI_PARA_ZME_HORL  ,
    VDP_WBC_GTI_PARA_ZME_HORC  ,
    VDP_WBC_GTI_PARA_ZME_VERL  ,
    VDP_WBC_GTI_PARA_ZME_VERC  ,
    
    VDP_WBC_PARA_BUTT
} OPTM_VDP_WBC_PARA_E;

//---------------------------------------
// Modules
//---------------------------------------

// bkg color patern fill
typedef struct tagVDP_BKG_S
{
    mt_u32 u32BkgY;
    mt_u32 u32BkgU;
    mt_u32 u32BkgV;

    mt_u32 u32BkgA;
    
    MT_BOOL bBkType;

} OPTM_VDP_BKG_S;

//-------------------
// CSC
//-------------------
typedef enum tagVDP_CSC_MODE_E
{
    VDP_CSC_YUV2RGB_601 = 0,
    VDP_CSC_YUV2RGB_709    ,
    VDP_CSC_RGB2YUV_601    ,
    VDP_CSC_RGB2YUV_709    ,
    VDP_CSC_YUV2YUV_709_601,
    VDP_CSC_YUV2YUV_601_709,
    VDP_CSC_YUV2YUV,
    VDP_CSC_YUV2YUV_MAX,
    VDP_CSC_YUV2YUV_MIN,
    VDP_CSC_YUV2YUV_RAND,
   
    VDP_CSC_BUTT
} OPTM_VDP_CSC_MODE_E;

//-------------------
// CSC
//-------------------
typedef struct 
{
    mt_s32 csc_coef00;
    mt_s32 csc_coef01;
    mt_s32 csc_coef02;

    mt_s32 csc_coef10;
    mt_s32 csc_coef11;
    mt_s32 csc_coef12;

    mt_s32 csc_coef20;
    mt_s32 csc_coef21;
    mt_s32 csc_coef22;

} OPTM_VDP_CSC_COEF_S;

typedef struct
{
    mt_s32 csc_in_dc0;
    mt_s32 csc_in_dc1;
    mt_s32 csc_in_dc2;

    mt_s32 csc_out_dc0;
    mt_s32 csc_out_dc1;
    mt_s32 csc_out_dc2;
} OPTM_VDP_CSC_DC_COEF_S;

//for rm
typedef struct 
{
    mt_s32 csc_coef00;
    mt_s32 csc_coef01;
    mt_s32 csc_coef02;

    mt_s32 csc_coef10;
    mt_s32 csc_coef11;
    mt_s32 csc_coef12;

    mt_s32 csc_coef20;
    mt_s32 csc_coef21;
    mt_s32 csc_coef22;

    mt_s32 csc_in_dc0;
    mt_s32 csc_in_dc1;
    mt_s32 csc_in_dc2;

    mt_s32 csc_out_dc0;
    mt_s32 csc_out_dc1;
    mt_s32 csc_out_dc2;
} OPTM_VDP_CSC_CFG_S;

typedef enum tagOPTM_VDP_DISP_MODE_E
{
    VDP_DISP_MODE_2D  = 0,
    VDP_DISP_MODE_SBS = 1,
    VDP_DISP_MODE_TAB = 4,
    VDP_DISP_MODE_FP  = 5,

    VDP_DISP_MODE_BUTT
}OPTM_VDP_DISP_MODE_E;
//-------------------
// vou graphic layer data extend mode
//-------------------
typedef enum tagVDP_GFX_BITEXTEND_E
{
    VDP_GFX_BITEXTEND_1ST =   0,  
    VDP_GFX_BITEXTEND_2ND = 0x2,
    VDP_GFX_BITEXTEND_3RD = 0x3,

    VDP_GFX_BITEXTEND_BUTT
}OPTM_VDP_GFX_BITEXTEND_E;

typedef enum tagVDP_CBM_MIX_E
{
    VDP_CBM_MIXV0 = 0,
    VDP_CBM_MIXV1 = 1,
    VDP_CBM_MIXG0 = 2,
    VDP_CBM_MIXG1 = 3,
    VDP_CBM_MIX0  = 4,
    VDP_CBM_MIX1  = 5,

    VDP_CBM_MIX_BUTT 
}OPTM_VDP_CBM_MIX_E;

typedef enum 
{
    VDP_DISP_COEFMODE_HOR  = 0,  
    VDP_DISP_COEFMODE_VER,
    VDP_DISP_COEFMODE_LUT,
    VDP_DISP_COEFMODE_GAM,
    VDP_DISP_COEFMODE_ACC,
    VDP_DISP_COEFMODE_ABC,
    VDP_DISP_COEFMODE_ACM,
    VDP_DISP_COEFMODE_NR,
    VDP_DISP_COEFMODE_SHARP,
    VDP_DISP_COEFMODE_DIM,
    VDP_DISP_COEFMODE_DIMZMEH,
    VDP_DISP_COEFMODE_DIMZMEV,

    VDP_DISP_COEFMODE_ALL
}OPTM_VDP_DISP_COEFMODE_E;

typedef struct tagVDP_GFX_CKEY_S
{
    mt_u32 u32Key_r_min;
    mt_u32 u32Key_g_min;
    mt_u32 u32Key_b_min;

    mt_u32 u32Key_r_max;
    mt_u32 u32Key_g_max;
    mt_u32 u32Key_b_max;    

    mt_u32 u32Key_r_msk;
    mt_u32 u32Key_g_msk;
    mt_u32 u32Key_b_msk;

    mt_u32 bKeyMode;

    mt_u32 u32KeyAlpha;

} OPTM_VDP_GFX_CKEY_S;

typedef struct tagVDP_GFX_MASK_S
{
    mt_u32 u32Mask_r;
    mt_u32 u32Mask_g;
    mt_u32 u32Mask_b;

} OPTM_VDP_GFX_MASK_S;

typedef struct tagOSD_HEADER_S
{
  /*! 
    The left x coordinate. 
    */
  mt_u16 m_left;
  /*! 
    The right x coordinate. 
    */
  mt_u16 m_right;
  /*! 
    The top y coordinate. 
    */
  mt_u16 m_top;
  /*! 
    The bottom y coordinate. 
    */
  mt_u16 m_bottom;
  /*! 
    Region alpha 0. 
    */
  mt_u8 m_alpha0;
  /*! 
    Region alpha 1. ONLY aRGB1555 can select alpha 1. 
    */
  mt_u8 m_alpha1;
  /*! 
    The width of the region. 
    */
  mt_u16 m_pitch;
  /*! 
    TRUE to display the region. otherwise to hide it. 
    */
  MT_BOOL m_enable;
  /*! 
    TRUE means this region contains a palette
    */
  MT_BOOL m_palette;
  /*! 
    The alpha mode of this region: region or plane. 
    */
  mt_u8 m_alphamode;
  /*!
    If current region is followed by another region.
    */
  MT_BOOL m_follow;
  /*!
    The start address of the data
    */
  mt_u32 m_start_addr;
  /*!
    The start address of the data of vertical scaler in graphic engine
    */
  mt_u32 m_start_addr_vs;
  /*!
    The start address of the uv data
    */
  mt_u32 m_uv_start_addr;
  /*! 
    The mode of RGB true color. 
    */
  mt_u8 m_truemode;
  /*! 
    The mode for indexed color. 
    */
  mt_u8 m_clutmode;
  /*! 
    The color space. 
    */
  mt_u8 m_colormode;
  /*! 
    The osd endian mode . 
    */
  mt_u8 m_endianmode;
  /*! 
    The palette endian mode . 
    */
  mt_u8 m_palette_endianmode;
  /*! 
    The alpha position. 
    */
  mt_u8 m_alpha_pos;
  /*! 
    The alpha blending mode. 
    */
  mt_u8 m_alpha_blendmode;
  /*! 
    odd duplication trigger
    */
  MT_BOOL m_odd_only;
  /*! 
    The start address of next region if it exist.
    */
  mt_u32 m_rgnaddr_next;
  /*! 
    The line stride in memory, in pixel
    */
  mt_u16 m_stride;
  /*! 
    TRUE to enable the semi-planar. otherwise to disable it.  
    */
  MT_BOOL m_semi_enable;
  /*! 
    UV exchange. 
    */
  MT_BOOL m_uv_change;
  /*! 
    The semi-planar format . 
    */
  mt_u8 m_semi_format;
} OPTM_OSD_HEADER_S;

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
/*!
  The scale coef table index
  */
typedef enum
{
  /*!
    table 1_4
    */
  SCALE_COEFF_TABLE_1_4 = 0,
  /*!
    table 1_7
    */
  SCALE_COEFF_TABLE_1_7,
  /*!
    table 2_4
    */
  SCALE_COEFF_TABLE_2_4,
  /*!
    table 2_7
    */
  SCALE_COEFF_TABLE_2_7,
  /*!
    table 3_5
    */
  SCALE_COEFF_TABLE_3_5,
  /*!
    table 3_7
    */
  SCALE_COEFF_TABLE_3_7,
  /*!
    table 4_6
    */
  SCALE_COEFF_TABLE_4_6,
  /*!
    table 4_7
    */
  SCALE_COEFF_TABLE_4_7,
  /*!
    table 5_7
    */
  SCALE_COEFF_TABLE_5_7,
  /*!
    table 7_3 horizontal
    */
  SCALE_COEFF_TABLE_7_3_H,
  /*!
    table 7_3 vertical
    */
  SCALE_COEFF_TABLE_7_3_V,
  /*!
    table 4 bicubic
    */
  SCALE_COEFF_TABLE_4_BICUBIC,
  /*!
    table 7 bicubic
    */
  SCALE_COEFF_TABLE_7_BICUBIC,
  /*!
    table 3 tap 1
    */
  SCALE_COEFF_TABLE_3_TAP_1,
  /*!
    table 3 tap 2
    */
  SCALE_COEFF_TABLE_3_TAP_2,
  /*!
    table 4 tap 1
    */
  SCALE_COEFF_TABLE_4_TAP_1,
  /*!
    table 4 tap 2
    */
  SCALE_COEFF_TABLE_4_TAP_2,
  /*!
    table 4 tap 3
    */
  SCALE_COEFF_TABLE_4_TAP_3,
  /*!
    table 4 tap 4
    */
  SCALE_COEFF_TABLE_4_TAP_4,
  /*!
    table 5 tap 0
    */
  SCALE_COEFF_TABLE_5_TAP_0,
  /*!
    table 4 vertical_graphic_scale_sharp
    */
  SCALE_COEFF_TABLE_4_V_GRAPHIC_SHARP,
    /*!
    table 7 horizontal sharp
    */
  SCALE_COEFF_TABLE_7_H_SHARP,
  /*!
    table 7 vertical sharp
    */
  SCALE_COEFF_TABLE_7_V_SHARP,
  /*!
    table 5 horizontal soft
    */
  SCALE_COEFF_TABLE_5_H_SOFT,
  /*!
    table 5 vertical softs
    */
  SCALE_COEFF_TABLE_5_V_SOFT,
    /*!
    table horizontal chroma down scale
    */
  SCALE_COEFF_TABLE_CHROMA_H_DOWNSCALE,
  /*!
    DCE no eco 0
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_0,
  /*!
    DCE no eco 1
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_1,
  /*!
    DCE no eco 2
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_2,
  /*!
    DCE no eco 3
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_3,
  /*!
    DCE no eco 4
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_4,
  /*!
    DCE no eco 5
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_5,
  /*!
    DCE no eco 6
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_6,
  /*!
    DCE no eco 7
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_7,
  /*!
    DCE no eco vivid 0 
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_0,
  /*!
    DCE no eco vivid 1
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_1,
  /*!
    DCE no eco vivid 2
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_2,
  /*!
    DCE no eco vivid 3
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_3,
  /*!
    DCE no eco vivid 4
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_4,
  /*!
    DCE no eco vivid 5
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_5,
  /*!
    DCE no eco vivid 6
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_6,
  /*!
    DCE no eco vivid 7
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID_7,
  /*
  DCE no eco
  */
  SCALE_COEFF_TABLE_DCE_NO_ECO,
  /*!
    DCE no eco vivid
    */
  SCALE_COEFF_TABLE_DCE_NO_ECO_VIVID,
  /*!
    table max
    */
  SCALE_COEFF_TABLE_MAX
}COEFF_TABLE_E;
  
#endif

//-----------------------------------
//define of EDA
//-----------------------------------
#endif

