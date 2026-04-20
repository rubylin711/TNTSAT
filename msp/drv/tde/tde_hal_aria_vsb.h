/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TDE_HAL_ARIA_VSB_H__
#define __TDE_HAL_ARIA_VSB_H__

#if 0
/*!
  This structure defines the pixfmt
  */
typedef enum
{
  /*!
    1-bit RGB Palette index  argb palette  = 0
    */
  PIX_FMT_RGBPALETTE1,      
  /*!
    2-bit RGB Palette index  argb palette  = 1
    */
  PIX_FMT_RGBPALETTE2,       
  /*!
    4-bit RGB Palette index  argb palette  = 2
    */
  PIX_FMT_RGBPALETTE4,      
  /*!
    8-bit RGB Palette index  argb palette  = 3
    */
  PIX_FMT_RGBPALETTE8,       
  /*!
    1-bit YUV Palette index  ayuv palette  = 4
    */
  PIX_FMT_YUVPALETTE1,      
  /*!
    2-bit YUV Palette index  ayuv palette  = 5
    */
  PIX_FMT_YUVPALETTE2,     
  /*!
    4-bit YUV Palette index  ayuv palette  = 6
    */
  PIX_FMT_YUVPALETTE4,     
  /*!
    8-bit YUV Palette index  ayuv palette  = 7
    */
  PIX_FMT_YUVPALETTE8,       
  /*!
    A1 and 1-bit RGB Palette index  argb palette  = 8
    */
  PIX_FMT_ARGBPALETTE11,     
  /*!
    A2 and 2-bit RGB Palette index  argb palette  = 9
    */
  PIX_FMT_ARGBPALETTE22,    
  /*!
    A4 and 4-bit RGB Palette index  argb palette  = 10
    */
  PIX_FMT_ARGBPALETTE44,     
  /*!
    A8 and 8-bit RGB Palette index  argb palette  = 11
    */
  PIX_FMT_ARGBPALETTE88,     
  /*!
    A1 and 1-bit YUV Palette index  ayuv palette  = 12
    */
  PIX_FMT_AYUVPALETTE11,    
  /*!
    A2 and 2-bit YUV Palette index  ayuv palette  = 13
    */
  PIX_FMT_AYUVPALETTE22,     
  /*!
    A4 and 4-bit YUV Palette index  ayuv palette  = 14
    */
  PIX_FMT_AYUVPALETTE44,     
  /*!
    A8 and 8-bit YUV Palette index  ayuv palette  = 15
    */
  PIX_FMT_AYUVPALETTE88,    
  /*!
    8-bit, no per-pixel alpha        = 16
    */
  PIX_FMT_RGB233,         
  /*!
    16-bit, no per-pixel alpha       = 17
    */
  PIX_FMT_RGB565,         
  /*!
    16-bit                           = 18
    */
  PIX_FMT_ARGB1555,     
  /*!
    16-bit                           = 19
    */
  PIX_FMT_RGBA5551,       
  /*!
    16-bit                           = 20
    */
  PIX_FMT_ARGB4444,       
  /*!
    16-bit                           = 21
    */
  PIX_FMT_RGBA4444,       
  /*!
    32-bit                           = 22
    */
  PIX_FMT_ARGB8888,       
  /*!
    32-bit                           = 23
    */
  PIX_FMT_RGBA8888,      
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 24    
    */
  PIX_FMT_Y0CBY1CR8888,  
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 25
    */
  PIX_FMT_Y0CRY1CB8888,  
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 26
    */
  PIX_FMT_Y1CBY0CR8888,  
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 27
    */
  PIX_FMT_Y1CRY0CB8888,   
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 28
    */
  PIX_FMT_CBY0CRY18888,  
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 29
    */
  PIX_FMT_CBY1CRY08888,   
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 30
    */
  PIX_FMT_CRY1CBY08888,  
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 31
    */
  PIX_FMT_CRY0CBY18888,  
  /*!
    32-bit for 1 pixel,  YCbCr444, 10-bit values   = 32
    */
  PIX_FMT_X2C10Y10CB10,  
  /*!
    YCbCr444, 8-bit values   ayuv8888              = 33
    */
  PIX_FMT_AYCBCR8888,    
  /*!
    YCbCr444, 8-bit values   vuya8888              = 34
    */
  PIX_FMT_CRCBYA8888,        
  /*!
    YCbCr444, 8-bit values   yuva8888              = 35
    */
  PIX_FMT_YCBCRA8888,    
  /*!
    YCbCr444, 8-bit values                         = 36
    */
  PIX_FMT_ACRCBY8888,    
  /*!
    YCbCr444, 24-bit values                        = 37
    */
  PIX_FMT_YCBCR444,       
  /*!
    YCbCr422, 16-bit values                        = 38
    */
  PIX_FMT_YCBCR422,      
  /*!
    YCbCr420, 12-bit values                        = 39
    */
  PIX_FMT_YCBCR420,  
  /*!
    2-bit RGB Palette index  bgra palette  = 40
    */
  PIX_FMT_RGBPALETTE2_PALETTE_BGRA,       
  /*!
    4-bit RGB Palette index  bgra palette  = 41
    */
  PIX_FMT_RGBPALETTE4_PALETTE_BGRA,      
  /*!
    8-bit RGB Palette index  bgra palette  = 42
    */
  PIX_FMT_RGBPALETTE8_PALETTE_BGRA,       
  /*!
    2-bit YUV Palette index  vuya palette  = 43
    */
  PIX_FMT_YUVPALETTE2_PALETTE_VUYA,     
  /*!
    4-bit YUV Palette index  vuya palette  = 44
    */
  PIX_FMT_YUVPALETTE4_PALETTE_VUYA,     
  /*!
    8-bit YUV Palette index  vuya palette  = 45
    */
  PIX_FMT_YUVPALETTE8_PALETTE_VUYA,       
  /*!
    A4 and 4-bit RGB Palette index   bgra palette   = 46
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_BGRA,     
  /*!
    A8 and 8-bit RGB Palette index  bgra palette  = 47
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_BGRA,     
  /*!
    8-bit RGB and A8 Paletteindex   argb palette   = 48
    */
  PIX_FMT_RGBAPALETTE88,     
  /*!
     8-bit RGB and A8 Palette index  bgra palette  = 49
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_BGRA,     
  /*!
    A4 and 4-bit YUV Palette index  vuya palette  = 50
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_VUYA,     
  /*!
    A8 and 8-bit YUV Palette index  vuya palette  = 51
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_VUYA,     
  /*!
    8-bit YUV and A8 Palette index  ayuv palette  = 52
    */
  PIX_FMT_YUVAPALETTE88,     
  /*!
     8-bit YUV and A8 Palette index  vuya palette  = 53
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_VUYA,     
  /*!
    16-bit, no per-pixel alpha       = 54
    */
  PIX_FMT_RGB565_SMALL_ENDIAN,         
  /*!
    16-bit                           = 55
    */
  PIX_FMT_ARGB1555_SMALL_ENDIAN,     
  /*!
    16-bit                           = 56
    */
  PIX_FMT_ARGB4444_SMALL_ENDIAN,       
  /*!
    32-bit                           = 57
    */
  PIX_FMT_ARGB8888_SMALL_ENDIAN,       
  /*!
    32-bit                           = 58
    */
  PIX_FMT_RGBA8888_SMALL_ENDIAN,      
  /*!
    16-bit                           = 59
    */
  PIX_FMT_RGBA5551_SMALL_ENDIAN,     
  /*!
    16-bit                           = 60
    */
  PIX_FMT_RGBA4444_SMALL_ENDIAN,       
  /*!
    2-bit RGB Palette index  rgba palette  = 61
    */
  PIX_FMT_RGBPALETTE2_PALETTE_RGBA,       
  /*!
    2-bit RGB Palette index  abgr palette  = 62
    */
  PIX_FMT_RGBPALETTE2_PALETTE_ABGR,       
  /*!
    4-bit RGB Palette index  rgba palette  = 63
    */
  PIX_FMT_RGBPALETTE4_PALETTE_RGBA,       
  /*!
    4-bit RGB Palette index  abgr palette  = 64
    */
  PIX_FMT_RGBPALETTE4_PALETTE_ABGR,       
  /*!
    8-bit RGB Palette index  rgba palette  = 65
    */
  PIX_FMT_RGBPALETTE8_PALETTE_RGBA,       
  /*!
    8-bit RGB Palette index  abgr palette  = 66
    */
  PIX_FMT_RGBPALETTE8_PALETTE_ABGR,       
  /*!
    2-bit YUV Palette index  yuva palette  = 67
    */
  PIX_FMT_YUVPALETTE2_PALETTE_YUVA,     
  /*!
    2-bit YUV Palette index  avuy palette  = 68
    */
  PIX_FMT_YUVPALETTE2_PALETTE_AVUY,     
  /*!
    4-bit YUV Palette index  yuva palette  = 69
    */
  PIX_FMT_YUVPALETTE4_PALETTE_YUVA,     
  /*!
    4-bit YUV Palette index  avuy palette  = 70
    */
  PIX_FMT_YUVPALETTE4_PALETTE_AVUY,     
  /*!
    8-bit YUV Palette index  yuva palette  = 71
    */
  PIX_FMT_YUVPALETTE8_PALETTE_YUVA,     
  /*!
    8-bit YUV Palette index  avuy palette  = 72
    */
  PIX_FMT_YUVPALETTE8_PALETTE_AVUY,     
  /*!
    A4 and 4-bit RGB Palette index   rgba palette   = 73
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_RGBA,     
  /*!
    A4 and 4-bit RGB Palette index   abgr palette   = 74
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_ABGR,     
  /*!
    A8 and 8-bit RGB Palette index   rgba palette   = 75
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_RGBA,     
  /*!
    A8 and 8-bit RGB Palette index   abgr palette   = 76
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_ABGR,     
  /*!
    8-bit RGB and A8 Paletteindex   rgba palette   = 77
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_RGBA,     
  /*!
     8-bit RGB and A8 Palette index  abgr palette  = 78
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_ABGR,     
  /*!
    A4 and 4-bit YUV Palette index  yuva palette  = 79
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_YUVA,     
  /*!
    A4 and 4-bit YUV Palette index  avuy palette  = 80
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_AVUY,     
  /*!
    A8 and 8-bit YUV Palette index  yuva palette  = 81
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_YUVA,     
  /*!
    A8 and 8-bit YUV Palette index  avuy palette  = 82
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_AVUY,     
  /*!
    8-bit YUV and A8 Palette index  yuva palette  = 83
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_YUVA,     
  /*!
     8-bit YUV and A8 Palette index  avuy palette  = 84
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_AVUY, 
  /*!
      = 85
    */ 
  PIX_FMT_GRAY_8,   
  /*!
    used by concerto = 86
  */
  PIX_FMT_RGBPALETTE1_PALETTE_BGRA, 
  /*!
    used by concerto = 87
  */
  PIX_FMT_XY,
  /*!
    used by concerto = 88
  */
  PIX_FMT_XYL,
  /*!
    used by concerto = 89
  */
  PIX_FMT_XYC,
  /*!
    used by concerto = 90
  */
  PIX_FMT_XYLC,
  /*!
    used by concerto = 91
  */
  PIX_FMT_XY_SMALL,
  /*!
    used by concerto = 92
  */
  PIX_FMT_XYL_SMALL,
  /*!
    used by concerto = 93
  */
  PIX_FMT_XYC_SMALL,
  /*!
    used by concerto = 94
  */
  PIX_FMT_XYLC_SMALL,
  /*!
    used by concerto = 95
  */
  PIX_FMT_TILE,
  /*!
    rgb888 24bits = 96 
  */
  PIX_FMT_RGB888,
  /*! 
    bgr8888 24bits = 97
  */
  PIX_FMT_BGR888,
  /*! 
    gray16 16bits = 98
  */
  PIX_FMT_GRAY_16,
  /*! 
    semi-planner yuv = 99
  */
  PIX_FMT_SP_YUV444,
  /*! 
    semi-planner yuv = 100
  */
  PIX_FMT_SP_YUV422,
  /*! 
    semi-planner yuv = 101
  */
  PIX_FMT_SP_YUV420,
  /*! 
    semi-planner yuv = 102
  */
  PIX_FMT_SP_YUV422_1x2,
  /*! 
    semi-planner yuv = 103
  */
  PIX_FMT_SP_YUV422_2x1,
  /*! 
    semi-planner yuv = 104
  */
  PIX_FMT_SP_YUV444_UVSWAP,
  /*! 
    semi-planner yuv = 105
  */
  PIX_FMT_SP_YUV422_UVSWAP,
  /*! 
    semi-planner yuv = 106
  */
  PIX_FMT_SP_YUV420_UVSWAP,  
  /*! 
    semi-planner yuv = 107
  */
  PIX_FMT_SP_YUV422_1x2_UVSWAP,
  /*! 
    semi-planner yuv = 108
  */
  PIX_FMT_SP_YUV422_2x1_UVSWAP,
  /*!
    cmyk = 109
  */
  PIX_FMT_CMYK,
  /*!
    kymc = 110
  */  
  PIX_FMT_KYMC,
  /*!
    semi-planner cmyk = 111
  */  
  PIX_FMT_SP_CMYK,    
  /*!
    semi-planner cmyk = 112
  */  
  PIX_FMT_SP_CMYK_SWAP,      
  /*!
    max
    */
  PIX_FMT_MAX
} pix_fmt_t;
#endif

/*!
  The command of mix with region
  valid after warriors chip
  */
typedef enum
{
  /*!
    comment
    */
  RGB_COLOR_SPACE,
  /*!
    comment
    */
  YUV_COLOR_SPACE,
  /*!
    comment
    */
  GRAY_COLOR_SPACE,

}color_space_t;

/*!
  comments
  */
typedef enum
{
  CLUT_1 = 0x0,
  CLUT_2 = 0x1,
  CLUT_4 = 0x2,
  CLUT_8 = 0x3,
  ALUT44 = 0x4,
  ALUT88 = 0x5,
  GRAY_8 = 0x11,
  GRAY_16 = 0x12,
  RGB233 = 0x20,
  RGB565 = 0x21,
  ARGB1555 = 0x22,
  RGBA5551 = 0x23,
  ARGB4444 = 0x24,
  RGBA4444 = 0x25,
  ARGB8888 = 0x26,
  RGBA8888 = 0x27,
  RGB888 = 0x28,
  BGR888 = 0x29,  
  Y1VY0U = 0x2a,
  AYUV8888 = 0x2b,
  YUVA8888 = 0x2c,
  CMYK8888 = 0x2d,
  KYMC8888 = 0x2e,
  XY = 0x30,
  XYC = 0x31,
  XYL = 0x32,  
  XYLC = 0x33,   
  SP_YUV420_Y = 0x40, 
  SP_YUV422_Y = 0x41, 
  SP_YUV444_Y = 0x42, 
  SP_CMYK8888_CM = 0x43,
  SP_YUV420_C = 0x48,
  SP_YUV422_C = 0x49,
  SP_YUV444_C = 0x4a,
  SP_CMYK8888_YK = 0x4b,
  TILE_Y = 0x50,
  TILE_C = 0x58,   
  SP_YUV422_Y2 = 0xff,
  COLOR_FORMAT_MAX,
}color_format_t;

/*!
  indicate symphony palette format
  */
typedef enum
{
  GPE_SPN_PALT_ARGB8888           = 0,
  GPE_SPN_PALT_RGBA8888           = 1,
  GPE_SPN_PALT_AYUV8888           = 2,
  GPE_SPN_PALT_YUVA8888           = 3,

  PALETTE_FORMAT_MAX              = 4,
}palette_format_t;



/*!
  indicate symphony color ramp spread mode for gradient paint
  */
typedef enum
{
  GPE_SPN_SPREAD_PAD                 = 0,
  GPE_SPN_SPREAD_REPEAT              = 1,
  GPE_SPN_SPREAD_REFLECT             = 2,
  GPE_SPN_FLAT_COLOR_FILL            = 3,
  
  SPREAD_MOD_MAX,
}gradt_spread_mod_t;

/*!
  indicate symphony mask mode for gradient paint
  */
typedef enum
{
  GPE_SPN_MASK_0                 = 0,
  GPE_SPN_MASK_1              = 1,
  GPE_SPN_MASK_2             = 2,
  
  MASK_MOD_MAX,
}gradt_mask_mod_t;

/*!
  indicate symphony tiling mod for pattern paint
  */
typedef enum
{
  GPE_SPN_TILE_FILL              = 0,
  GPE_SPN_TILE_PAD               = 1,
  GPE_SPN_TILE_REPEAT            = 2,
  GPE_SPN_TILE_REFLECT           = 3,

  TILE_MOD_MAX                   = 4,
}tiling_mod_t;


/*!
  comments
  */
  typedef enum {
    VG_CLEAR_MASK,
    VG_FILL_MASK,
    VG_SET_MASK,
    VG_UNION_MASK,
    VG_INTERSECT_MASK,
    VG_SUBTRACT_MASK,

    VG_MASK_OPERATION_FORCE_SIZE,
  } vg_mask_operation_t;


/*!
  comments
  */
  typedef enum {
    VG_COLOR_RAMP_SPREAD_PAD,
    VG_COLOR_RAMP_SPREAD_REPEAT,
    VG_COLOR_RAMP_SPREAD_REFLECT,

    VG_COLOR_RAMP_SPREAD_MODE_FORCE_SIZE,
  } vg_color_ramp_spread_mode_t;


/*!
  comments
  */
  typedef enum {
    VG_GRADIENT_MASK_0,
    VG_GRADIENT_MASK_1,
    VG_GRADIENT_MASK_2,

    VG_GRADIENT_MASK_MODE_FORCE_SIZE,
  } vg_gradient_mask_mode_t;


/*!
  comments
  */
  typedef enum {
    VG_TILE_FILL,
    VG_TILE_PAD,
    VG_TILE_REPEAT,
    VG_TILE_REFLECT,

    VG_TILING_MODE_FORCE_SIZE,
  } vg_tiling_mode_t;


/*!
  comments
  */
  typedef enum {
    VG_PAINT_TYPE_COLOR,
    VG_PAINT_TYPE_LINEAR_GRADIENT,
    VG_PAINT_TYPE_RADIAL_GRADIENT,
    VG_PAINT_TYPE_ELLIPSE_GRADIENT,
    VG_PAINT_TYPE_PATTERN,

    VG_PAINT_TYPE_FORCE_SIZE,
  } vg_paint_type_t;


typedef enum
{
    /*!
    Blackness
    */
    ROP_BLACK       = 0x00,     
    /*!
    ~(S | D)
    */
    ROP_NOTMERGEPEN = 0x11,     
    /*!
    ~S&D
    */
    ROP_MASKNOTPEN  = 0x22,    
    /*!
    ~S
    */
    ROP_NOTCOPYPEN  = 0x33,    
    /*!
    S&~D
    */
    ROP_MASKPENNOT  = 0x44,     
    /*!
    ~D
    */
    ROP_NOT         = 0x55,     
    /*!
    S^D
    */
    ROP_XORPEN      = 0x66,     
    /*!
    ~(S & D)
    */
    ROP_NOTMASKPEN  = 0x77,    
    /*!
    S&D
    */
    ROP_MASKPEN     = 0x88,    
    /*!
    ~(S^D)
    */
    ROP_NOTXORPEN   = 0x99,     
    /*!
    D
    */
    ROP_NOP         = 0xaa,     
    /*!
    ~S|D
    */
    ROP_MERGENOTPEN = 0xbb,    
    /*!
    S
    */
    ROP_COPYPEN     = 0xcc,     
    /*!
    S|~D
    */
    ROP_MERGEPENNOT = 0xdd,     
    /*!
    S|D
    */
    ROP_MERGEPEN    = 0xee,     
    /*!
    Whiteness
    */
    ROP_WHITE       = 0xff,     
    /*!
    p ^ d
    */
    ROP_PATINVERT   = 0x5a,     
    /*!
    s & p
    */
    ROP_MERGEPAINT  = 0xc0,     
    /*!
    p
    */
    ROP_PATCOPY     = 0xf0,   
    /*!
    ~s | p | d
    */
    ROP_PATPAINT    = 0xfb     
  }rop_mod_t;


/*!
  indicate symphony blend factor
  */
typedef enum
{
  GPE_SPN_GL_ZERO                 = 0,
  GPE_SPN_GL_ONE                  = 1,
  GPE_SPN_GL_SRC_COLOR            = 2,
  GPE_SPN_GL_DST_COLOR            = 3,
  GPE_SPN_GL_ONE_MINUS_SRC_COLOR  = 4,
  GPE_SPN_GL_ONE_MINUS_DST_COLOR  = 5,
  GPE_SPN_GL_DST_ALPHA            = 6,
  GPE_SPN_GL_ONE_MINUS_DST_ALPHA  = 7,
  GPE_SPN_GL_SRC_ALPHA            = 8,
  GPE_SPN_GL_ONE_MINUS_SRC_ALPHA  = 9,
  GPE_SPN_GL_SRC_ALPHA_SATURATE   = 10,
  GPE_SPN_ST_COLOR                = 11,
  GPE_SPN_ST_ONE_MINUS_COLOR      = 12,
  GPE_SPN_ST_ALPHA                = 13,
  GPE_SPN_ST_ONE_MINUS_ALPHA      = 14,
  GPE_SPN_ST_ALPHA_SATURATE       = 15,
  
  BLEND_MOD_MAX                   = 16,
}blend_fact_t;




/*!
  indicate symphony bit per pixel
  */
typedef enum
{
  GPE_SPN_BPP_1BIT                = 0,
  GPE_SPN_BPP_2BIT                = 1,
  GPE_SPN_BPP_4BIT                = 2,
  GPE_SPN_BPP_8BIT                = 3,
  GPE_SPN_BPP_16BIT               = 4,
  GPE_SPN_BPP_32BIT               = 5,
  GPE_SPN_BPP_24BIT               = 6,
}spn_bpp_t ;

/*!
  indicate symphony gradt_type
  */
typedef enum
{
  GPE_SPN_LINER_GRADT             = 0,
  GPE_SPN_RADIAL_GRADT            = 1,
  GPE_SPN_ELLIPSE_GRADT            = 2,
}gradt_type_t;


/*!
  comments
  */
typedef enum
{
  SPN_DST,
  SPN_SRC1,
  SPN_SRC2,
  SPN_SRC3,
}src_ch_t;

/*!
indicate concerto rotator operation
*/
typedef enum
{
    GPE_SPN_NO_OP = 0,
    GPE_SPN_HORI_MIRROR = 1,
    GPE_SPN_VERT_MIRROR = 2,
    GPE_SPN_HORI_VERT_MIRROR = 3,
    GPE_SPN_TRANS = 4,    
    GPE_SPN_HORI_MIRROR_TRANS = 5,    
    GPE_SPN_VERT_MIRROR_TRANS = 6,    
    GPE_SPN_HORI_VERT_MIRROR_TRANS = 7,
  }spn_rotator_op_t;

typedef enum
{
  DS_MIX_UV = 3,
  DS_EVEN_UV = 0,
  DS_ODD_UV = 2,
  DS_MEDIA_UV = 1,
}ds_mod_t;

typedef enum
{
    SCALE_HORI_BLK_OUT = 0,
    SCALE_VERT_BLK_OUT = 1,
    SCALE_HORI_LINE_OUT = 2
}scale_mod_t;

typedef enum
{
    SCALE_ONLY_HORI_RECT = 0,
    SCALE_ONLY_VERT_RECT,
    SCALE_RECT,
    SCALE_TRAPZ,
    SCALE_ONLY_HORI_TRAPZ,
    SCALE_TRANS_TRAPZ,
    SCAL_TYPE_BOTN,
}scale_type_t;

typedef enum
{
    EXP_PAD_LOW_BIT = 0,
    EXP_PAD_ZERO = 2,
    EXP_PAD_OLD_MODE = 3
}exp_mode_t;

/*!
  comments
  */
typedef struct
{
  MT_BOOL               little_endian;
  mt_u32                  palt_buf;
  mt_u32                 palt_size;
  palette_format_t    palt_format;
}palt_info_t;



/*!
  comments
  */
typedef struct
{
  color_format_t        color_fmt;  
  color_space_t        color_space;
  MT_BOOL               alpha_ch_en;
  MT_BOOL               alpha_pre_mult_en;
  MT_BOOL               little_endian;
  mt_u32                  bpp;
  MT_BOOL               is_pix_alpha;
}color_info_t;

/*!
  comments
  */
typedef struct
{
  mt_u32                chroma_addr;
  mt_u32                luma_addr;
}tile_cfg_t;
/*!
  comments
  */
typedef struct
{
  mt_u32                xylc_num;
  mt_u32                xylc_color;
}xylc_cfg_t;

/*!
  comments
  */
typedef struct
{
  phys_addr_t         buf;
  mt_u32                pitch; //unit: byte
  mt_u32                width; //unit: pixel
  mt_u32                height; //unit:pixel
  MT_BOOL             negative_stride; //only used in src1
  rect_vsb_t            rect;
  color_info_t          color_info;
  //plane alpha
  MT_BOOL             plane_alpha_en; //only for src1 and src3
  mt_u32                plane_alpha; //only for src1 and src3
  //color key
  MT_BOOL             ck_en;
  mt_u32                ck_min;
  mt_u32                ck_max;
  mt_u32                ck_mod;
  mt_u32                ck_select;
  
  //palette
  MT_BOOL               with_palette;
  MT_BOOL               palt_little_endian;
  palette_format_t     palt_format;
  mt_u32                  palt_size;                
  phys_addr_t           palt_buf;

/*
  MT_BOOL               is_xylc;
  mt_u32                  xylc_num;
  mt_u32                  xylc_color;
  
  MT_BOOL               is_tile;
  mt_u32                  chroma_addr;
  mt_u32                  chroma_pitch;
*/ 
}gpe_img_t;




/*!
  comments
  */
  typedef struct
  {
    /*!
      comments
      */
    mt_u32 argb;
    /*!
      offset= offset_org <<12,offset_org: 0~1
      */
    mt_u32 offset;
  }gradt_stop_t;


/*!
  comments
  */
typedef struct
{
  MT_BOOL               is_pattern_paint; //gradient paint or pattern paint
  //gradient paint
  MT_BOOL               true_liner_gradt; //used to make flat color paint 
                                       //different from liner gradient paint
  gradt_type_t       gradt_type;//hardware theat flat color paint as liner gradient paint
  gradt_spread_mod_t spread_mod;
  gradt_mask_mod_t mask_mod;
  
  //flat color paint or fill color for pattern paint
  mt_u32                paint_color;
  //liner gradient paint
  pos_t              begin;
  pos_t              end;
  mt_s32                gradt_start;
  mt_s32                step_x;
  mt_s32                step_y;
  //radial gradient paint
  pos_t              center;
  pos_t              focus;
  mt_u32                radius;
  //ellipse gradient paint
  mt_u32                long_a;
  mt_u32                short_b;
  mt_u32                stop_num;
  
  gradt_stop_t         stop0;
  gradt_stop_t         stop1;
  gradt_stop_t         stop2;
  gradt_stop_t         stop3;
  mt_s32                stop_fact0;
  mt_s32                stop_fact1;
  mt_s32                stop_fact2;
  
  //pattern paint
  tiling_mod_t         tiling_mod;
  pos_t                   pat_beg;
  mt_u32                pat_remd1;
  mt_u32                pat_quot1;
  mt_u32                pat_remd2;
  mt_u32                pat_quot2;
  mt_u32                pat_remd3;
  mt_u32                pat_quot3;
  mt_u32                pat_remd4;
  mt_u32                pat_quot4;
 
}paint_cfg_t;

/*!
  comments
  */
typedef struct
{
  mt_u32    coef_addr;
  MT_BOOL disable_alpha_filter;
  MT_BOOL disable_color_filter;
  MT_BOOL disable_alpha_anti_flicker;
  MT_BOOL disable_color_anti_flicker;
  mt_u32    init_phase;
  scale_mod_t scale_mod;
  mt_s32      coef[6];   
}scale_cfg_t;


typedef struct 
{
    //mt_u8 src3_aplha_en;
    //mt_u8 src3_aplha;
    //mt_u8 src3_pre_mutil_en;
    mt_u8 src3_mult_mod;

    //mt_u8 src1_aplha_en;
    //mt_u8 src1_aplha;
    //mt_u8 src1_pre_mutil_en;
    mt_u8 src1_mult_mod;
}multip_cfg_t;


typedef struct
{
   spn_rotator_op_t rotator_type;
}rotator_cfg_t;


/*!
  comments
  */
typedef struct
{
    mt_u32 blur_tap;
}blur_cfg_t;



typedef enum
{
    KEY_MATCH_NONE = 0,
    //MIN <=C <= MAX
    KEY_MATCH_INSIDE_MIN_MAX = 1,
    //C <MIN  || C > MAX
    KEY_MATCH_OUTSIDE_MIN_MAX = 2,
    KEY_MATCH_ALL = 3
}ck_mod_t;

typedef enum
{
    MASK_KEY_MATCH = 0,
    MASK_KEY_MISMATCH = 1
}ck_select_t;



typedef struct
  {
    /*!
      comments
      */
    phys_addr_t buf;
    /*!
      unit:byte
      */
    u32 pitch;
    /*!
      unit:pixel
      */
    u32 width;
    /*!
      unit:pixel
      */
    u32 height;
    /*!
      comments
      */
    MT_BOOL negative_stride;
    /*!
      comments
      */
    rect_vsb_t rect;
    /*!
      comments
      */
    pix_fmt_t pix_format;
    /*!
      alpha
      */
    MT_BOOL alpha_pre_multed;
    /*!
      comments
      */
    MT_BOOL alpha_ch_en;
    /*!
      comments
      */
    MT_BOOL plane_alpha_en;
    /*!
      comments
      */
    u32 plane_alpha;
    /*!
      color key
      */
    MT_BOOL ck_en;
    /*!
      comments
      */
    u32 key_color;
    /*!
      tile
      */
    u8 field_flag;
    /*!
      comments
      */
    phys_addr_t chroma_addr;
    /*!
      added in symphony, for semi-planner and tile yuv format
      */    
    u32 chroma_pitch;
    /*!
      comments
      */
    u32 luma_addr;
    /*!
      xylc
      */
    u32 xylc_num;
    /*!
      comments
      */
    u32 xylc_color;
        /*!
     palette
      */
    phys_addr_t palette_base;
    /*!
      comments
      */
    u32 palette_size;
    /*!
      added in symphony, new colorkey rule
      */       
    u32               key_color_mod;
    /*!
      added in symphony, new colorkey rule
      */       
    u32 key_color_select;
  }image_info_t;


  typedef enum
  {
    /*!
      Asrc3 ? ROP/BLD(src1,src2):src2
      */
    GPE_CCT_ALPHA_MAP_LOGICAL,
    /*!
      [ARGB]src1.*[AAAA]src3
      */
    GPE_CCT_ALPHA_MAP_MIX_NORMAL,
    /*!
      [ARGB]src1.*[A111]src3
      */
    GPE_CCT_ALPHA_MAP_MIX_EX,

    ALPHA_MAP_MOD_MAX,
  }alpha_map_mod_t;



typedef struct
{
  /*!
     alpha
      */
    rop_mod_t  rop_a_id;
    /*!
     color
      */
    rop_mod_t  rop_c_id;
    /*!
      comments
      */
    u32 rop_pattern;
}rop_cfg_t;


typedef enum
{
    GL_ZERO = 0,
    GL_ONE = 1,
    GL_SRC_COLOR = 2,
    GL_DST_COLOR = 3,
    GL_ONE_MINUS_SRC_COLOR = 4,
    GL_ONE_MINUS_DST_COLOR = 5,
    GL_DST_ALPHA = 6,
    GL_ONE_MINUS_DST_ALPHA = 7,
    GL_SRC_ALPHA = 8,
    GL_ONE_MINUS_SRC_ALPHA = 9,
    GL_SRC_ALPHA_SATURATE = 10,
}bld_fact_t;



typedef struct
{
  /*!
      comments
      */
    bld_fact_t  src_blend_fact;
    /*!
      comments
      */
    bld_fact_t  dst_blend_fact;
}blend_cfg_t;


typedef struct
  {
    /*!
      comments
      */
    pos_t begin;
    /*!
      comments
      */
    pos_t end;
    /*!
      comments
      */
    gradt_stop_t stop0;
    /*!
      comments
      */
    gradt_stop_t stop1;
    /*!
      comments
      */
    gradt_stop_t stop2;
    /*!
      comments
      */
    gradt_stop_t stop3;
    /*!
      VGColorRampSpreadMode
      */
    u32 spread_mod;
    /*!
      comments
      */
    u32 mask_mod;
  }paint_liner_gradt_cfg_t;


  typedef struct
  {
    /*!
      comments
      */
    pos_t center;
    /*!
      comments
      */
    pos_t focus;
    /*!
      comments
      */
    u32 radius;
    /*!
      comments
      */
    gradt_stop_t stop0;
    /*!
      comments
      */
    gradt_stop_t stop1;
    /*!
      comments
      */
    gradt_stop_t stop2;
    /*!
      comments
      */
    gradt_stop_t stop3;
    /*!
      VGColorRampSpreadMode
      */
    u32 spread_mod;
    /*!
      comments
      */
    u32 mask_mod;
  }paint_radial_gradt_cfg_t;
  

  typedef struct
  {
    /*!
      comments
      */
    pos_t center;
    /*!
      comments
      */
    pos_t focus;
    /*!
      comments
      */
    u32 long_a;
    /*!
      comments
      */
    u32 short_b;
    /*!
      comments
      */
    gradt_stop_t stop0;
    /*!
      comments
      */
    gradt_stop_t stop1;
    /*!
      comments
      */
    gradt_stop_t stop2;
    /*!
      comments
      */
    gradt_stop_t stop3;
    /*!
      VGColorRampSpreadMode
      */
    u32 spread_mod;
    /*!
      comments
      */
    u32 mask_mod;
    /*!
      comments
      */
    u32 stop_num;
  }paint_ellipse_gradt_cfg_t;


  typedef struct
  {
    /*!
      comments
      */
    pos_t pat_beg;
    /*!
      comments
      */
    u32 fill_color;
    /*!
      VGTilingMode
      */
    u32 tiling_mod;
  }paint_pattern_cfg_t;
  


 typedef struct
 {
    /*!
      VGPaintType
      */
    u32 paint_type;
    /*!
      paint color config
      */
    u32 paint_color;
    /*!
      paint liner gradient config
      */
    paint_liner_gradt_cfg_t paint_liner_gradt;
    /*!
      paint radial gradient config
      */
    paint_radial_gradt_cfg_t paint_radial_gradt;
    /*!
      paint ellipse gradient config
      */
    paint_ellipse_gradt_cfg_t paint_ellipse_gradt;

    /*!
      paint pattern config
      */
    paint_pattern_cfg_t paint_pattern;

  }paint_info_t;


typedef enum
{
     GPE_OP_NONE                = 0x00000000,  
     GPE_OP_ROP                  = 0x000000001,
     GPE_OP_BLEND              = 0x000000002,
     GPE_OP_ALPHAMAP       = 0x000000004,
     GPE_OP_SCALE               = 0x000000008,
     GPE_OP_ROTATE            = 0x000000010,
     GPE_OP_PAINT               = 0x000000020,
     GPE_OP_MASK                = 0x000000040,
     GPE_OP_DMULT              = 0x000000080,
     GPE_OP_DSTEN              = 0x000000100,
     GPE_OP_SCALE_TRAPZ  = 0x000000200,
     GPE_OP_SCALE_HORI    = 0x000000400,
     GPE_OP_SCALE_VERT    = 0x000000800,
     GPE_OP_BLUR                = 0x000001000,
} gpe_ops_t;
  

  typedef enum
  {
    GPE_CCT_NO_OP = 0,
    GPE_CCT_TRANS = 1,
    GPE_CCT_HORI_MIRROR = 2,
    GPE_CCT_HORI_MIRROR_TRANS = 3,
    GPE_CCT_VERT_MIRROR = 4,
    GPE_CCT_VERT_MIRROR_TRANS = 5,
    GPE_CCT_HORI_VERT_MIRROR = 6,
    GPE_CCT_HORI_VERT_MIRROR_TRANS = 7,
  }rotator_op_t;



typedef enum
{
    HD_FIELD_START = 0,
    HD_TOP_START = 1,
    HD_BOT_START = 2,
    SD_FIELD_START = 4,
    SD_TOP_START = 5,
    SD_BOT_START = 6
}sync_signal_t;

typedef enum
{
    CONF_WAIT_NODE_FINISH = 0,
    CONF_SWITCH_TO_SYNC = 1
}conf_mod_t;

typedef enum
{
    CONF_CONTINUE_RUN = 0,
    CONF_STOP_RUN = 1
}halt_mod_t;

typedef enum
{
    CONF_HW_RST = 0,
    CONF_SW_RST = 1
}rst_mod_t;

typedef enum
{
    CONF_AUTO_RECOVERY = 0,
    CONF_CPU_RECOVERY = 1
}recovery_mod_t;




typedef struct
{
   mt_u8   bpp;
   mt_u8   swap_mod;
   mt_u8   fmt;
   mt_u8   exp_mod;
   mt_u8   mb_type;
   
   mt_u32 pic_addr;
   mt_u32 pic_stride;
}ARIA_MB_INFO_S;


typedef enum 
{
  MB_TYPE_Y,
  MT_TYPE_CbCr,
  MT_TYPE_NONE,
    
}MB_INFO_TYPE_E;





#define CF_SYNC_LIST_EXIT (1 << 6)
#define CF_ASYNC_LIST_EXIT (1 << 18)
#define GRA_ALL_DONE (1 << 31)


/*!
  comments
  */
typedef struct
{
  MT_BOOL src_in;
  MT_BOOL dst_in;
}spn_rop_ch_t;



typedef struct
{
  /*!
  buffer for output mask data
  */
  phys_addr_t mbuf_addr;
  /*!
  8bytes aligned
  */
  u32 mbuf_pitch;
  /*!
  0: store mask to mask buf, 
  1: store mask to alpha, no mask buf needed, 
  2: store src alpha to mask buf
  */
  u32 mask2alpha; 
}dst_mask_cfg_t;

  
/*!
  struct for common use
  */
  typedef struct
  {
    MT_BOOL comp_key_set; //0:keep dst value when colorkey math, 1: output 0xffffffff when colorkey match
    MT_BOOL clip_en;
    mt_u32 dst_ds_mod;  
    //0:按像素独立判断有效性
    // 1:相邻两个像素有一个无效，整个都无效
    mt_u32 dst_ds_choose;     
    /*!
      comments
      */
    MT_BOOL no_dithering;
    /*!
      comments
      */
    MT_BOOL src_img_en;
    /*!
      comments
      */
    MT_BOOL ex_img_en;
    /*!
      comments
      */
    MT_BOOL bg_img_en;
    /*!
      comments
      */
    image_info_t src_img;
    /*!
      src2(background) image
      */
    image_info_t dst_img;
    /*!
      comments
      */
    image_info_t ex_img;
    /*!
      comments
      */
    image_info_t bg_img;    

   /*!
      comments
   */
    gpe_ops_t gpe_op;
    /*!
      comments
      */
    paint_info_t paint;
    /*!
      comments
      */
    alpha_map_mod_t alpha_map_mod;
    /*!
      blend cfg
      */
    blend_cfg_t blend;
    /*!
     rop cfg
      */
    rop_cfg_t rop;
    /*!
     scaler coef
      */
    mt_s32 coef[6];
    /*!
    scale output mode
    */
    scale_mod_t scale_mod;
    /*!
     if mask enable, src1 and src2 must be GRAY-8??
      */
    mt_u32 mask_mod;
    /*!
      comments
      */
    rotator_op_t rotator_op;
   /*!
     comments
   */
   MT_BOOL src_with_mask;
   mt_u32 src_mask_buf;
   mt_u32 src_mask_pitch;
   /*!
     comments
   */
   MT_BOOL dst_with_mask; // true in 3d scaler 
   /*!
     comments
   */
   dst_mask_cfg_t dst_mask;
   MT_BOOL need_suspend;
   
     //gaussian blur
   mt_u32 blur_tap;
  }spn_param_t;

/*
used inner
*/
typedef struct
{
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
  u16 * p_gradt_buf;
  
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
  spn_rotator_op_t rotator_op;
  spn_rop_ch_t rop_ch;

  //mask
  MT_BOOL src_with_mask;
  mt_u32 src0_buf;
  mt_u32 src0_pitch;
  MT_BOOL dst_with_mask;
  dst_mask_cfg_t dst_mask;

  //for cmdfifo
  MT_BOOL is_cmdfifo;  
  //cmdfifo_node_t *p_node;

  mt_u32 cmyk_max;
  mt_u32 cmyk_coef;
}spn_gpe_context_t;

/*!
  default a mrco to read reg  
  */
//#define GET_REG(A)      (*((volatile unsigned mt_s32 *)(A))) 

/*!
  default a mrco to write reg  
  */
//#define PUT_REG(A,V)   (*((volatile unsigned mt_s32 *)(A))) = ((unsigned mt_s32)V) 


/*!
  indicate the hardware isr on/off
  */
//#define GPE_SPN_HARDWARE_ISR_ON


/*!
  The graphics engine isr no.
  */
//#define GPE_SPN_ISR_NO           17

/*!
  The graphics engine cmd fifo isr no.
  */
//#define GPE_SPN_CMD_FIFO_ISR_NO  18

/*!
  this macro definition will set a timer to check the cycle when draw image   
  */
//#define GPE_SPN_TEST_CYCLE

/*!
  check graphics engine command is sync or not 
  this mod is sync opertion 
  */
//#define GPE_SPN_SYNC_COMMAND
/*!
  indicate the print function enable for debug
  */
//#define GPE_SYMPHONY_PRINT_ON


/*!
  indicate assert
  */
//#define TDE_ASSERT_ON

/*!
  indicate assert
  */
//#define TDE_ASSERT  MT_ASSERT

/*!
  comments
  */
//#define GPE_SPN_MEMCPY memcpy

//#define OLD_EXP_MODE

#define PARA_FRA  14
#define PARA_FRA_22  14
#define PARA_FRA_23_33  22
#define SOR_FRA 18



bld_fact_t tde_hal_convert_bld_mod_to_fact(TDE2_BLEND_MODE_E bld_mod);
rop_mod_t tde_hal_convert_tde_rop_id_to_aria_rop_mod(TDE2_ROP_CODE_E rop);
MT_BOOL tde_hal_converto_surface_to_pix_fmt(TDE_DRV_SURFACE_S *p_surface, pix_fmt_t *p_fmt);
mt_u8  tde_hal_get_img_swap(gpe_img_t *p_img);
MT_BOOL tde_hal_convert_surface_to_img(TDE_DRV_SURFACE_S *p_surface, gpe_img_t  *p_img, src_ch_t ch);
MT_BOOL  tde_hal_get_MB_info(TDE_DRV_SURFACE_S *p_MBSurface,  MB_INFO_TYPE_E mbType,  ARIA_MB_INFO_S *p_info);

mt_void tde_hal_get_scale_coeff(rect_vsb_t *p_src , 
                                                                      pos_t *p_dst00,
                                                                      pos_t *p_dst10, 
                                                                      pos_t *p_dst01, 
                                                                      pos_t * p_dst11,
                                                                      mt_s32 *p_coeff,
                                                                      scale_type_t *scale_type);
mt_void TdeHalStart(mt_void);
mt_void TdeHalCfStart(mt_void);
mt_void TdeHalCfSyncStart(mt_void);

#endif     //  __TDE_HAL_ARIA_VSB_H__






