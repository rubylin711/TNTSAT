#ifndef _GPE_H_
#define _GPE_H_

#include "mt_drv_disp.h"
#include "mt_tde_type.h"
#include "tde_config.h"
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif /* __cplusplus */
#endif  /* __cplusplus */


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
  indicate symphony color ramp spread mode for gradient paint
  */
typedef enum
{
  GPE_ARIA_SPREAD_PAD                 = 0,
  GPE_ARIA_SPREAD_REPEAT              = 1,
  GPE_ARIA_SPREAD_REFLECT             = 2,
  GPE_ARIA_FLAT_COLOR_FILL            = 3,
  
  SPREAD_MOD_MAX,
}gradt_spread_mod_t;

/*!
  indicate symphony mask mode for gradient paint
  */
typedef enum
{
  GPE_ARIA_MASK_0                 = 0,
  GPE_ARIA_MASK_1              = 1,
  GPE_ARIA_MASK_2             = 2,
  
  MASK_MOD_MAX,
}gradt_mask_mod_t;

/*!
  indicate symphony tiling mod for pattern paint
  */
typedef enum
{
  GPE_ARIA_TILE_FILL              = 0,
  GPE_ARIA_TILE_PAD               = 1,
  GPE_ARIA_TILE_REPEAT            = 2,
  GPE_ARIA_TILE_REFLECT           = 3,

  TILE_MOD_MAX                   = 4,
}tiling_mod_t;

/*!
  indicate symphony blend factor
  */
typedef enum
{
  GPE_ARIA_GL_ZERO                 = 0,
  GPE_ARIA_GL_ONE                  = 1,
  GPE_ARIA_GL_SRC_COLOR            = 2,
  GPE_ARIA_GL_DST_COLOR            = 3,
  GPE_ARIA_GL_ONE_MINUS_SRC_COLOR  = 4,
  GPE_ARIA_GL_ONE_MINUS_DST_COLOR  = 5,
  GPE_ARIA_GL_DST_ALPHA            = 6,
  GPE_ARIA_GL_ONE_MINUS_DST_ALPHA  = 7,
  GPE_ARIA_GL_SRC_ALPHA            = 8,
  GPE_ARIA_GL_ONE_MINUS_SRC_ALPHA  = 9,
  GPE_ARIA_GL_SRC_ALPHA_SATURATE   = 10,
  GPE_ARIA_ST_COLOR                = 11,
  GPE_ARIA_ST_ONE_MINUS_COLOR      = 12,
  GPE_ARIA_ST_ALPHA                = 13,
  GPE_ARIA_ST_ONE_MINUS_ALPHA      = 14,
  GPE_ARIA_ST_ALPHA_SATURATE       = 15,
  
  BLEND_MOD_MAX                   = 16,
}blend_fact_t;

/*!
  indicate symphony palette format
  */
typedef enum
{
  GPE_ARIA_PALT_ARGB8888           = 0,
  GPE_ARIA_PALT_RGBA8888           = 1,
  GPE_ARIA_PALT_AYUV8888           = 2,
  GPE_ARIA_PALT_YUVA8888           = 3,

  PALETTE_FORMAT_MAX              = 4,
}palette_format_t;


/*!
  indicate symphony bit per pixel
  */
typedef enum
{
  GPE_ARIA_BPP_1BIT                = 0,
  GPE_ARIA_BPP_2BIT                = 1,
  GPE_ARIA_BPP_4BIT                = 2,
  GPE_ARIA_BPP_8BIT                = 3,
  GPE_ARIA_BPP_16BIT               = 4,
  GPE_ARIA_BPP_32BIT               = 5,
  GPE_ARIA_BPP_24BIT               = 6,
}aria_bpp_t ;

/*!
  indicate symphony gradt_type
  */
typedef enum
{
  GPE_ARIA_LINER_GRADT             = 0,
  GPE_ARIA_RADIAL_GRADT            = 1,
  GPE_ARIA_ELLIPSE_GRADT            = 2,
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
indicate aria rotator operation
*/
typedef enum
{
  GPE_ARIA_NO_OP = 0,
  GPE_ARIA_HORI_MIRROR = 1,
  GPE_ARIA_VERT_MIRROR = 2,
  GPE_ARIA_HORI_VERT_MIRROR = 3,
  GPE_ARIA_TRANS = 4,    
  GPE_ARIA_HORI_MIRROR_TRANS = 5,    
  GPE_ARIA_VERT_MIRROR_TRANS = 6,    
  GPE_ARIA_HORI_VERT_MIRROR_TRANS = 7,
}aria_rotator_op_t;

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
    SCALE_ONLY_HORI_RECT,
    SCALE_ONLY_VERT_RECT,
    SCALE_RECT,
    SCALE_TRAPZ,
    SCALE_ONLY_HORI_TRAPZ,
    SCALE_TRANS_TRAPZ,
    SCALE_TYPE_MAX
}scale_type_t;

typedef enum
{
    EXP_PAD_LOW_BIT = 0,
    EXP_PAD_ZERO = 2,
    EXP_PAD_OLD_MODE = 3
}exp_mode_t;

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

  /*!
indicate concerto rotator operation
*/
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

/*!
    gpe operation
   */
typedef enum {
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

/*!
  comments
  */
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

/*!
  comments
  */
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

#define CF_SYNC_LIST_EXIT (1 << 6)
#define CF_ASYNC_LIST_EXIT (1 << 18)
#define GRA_ALL_DONE (1 << 31)
#define CF_ASYNC_LIST_FINISH (1 << 8)
#define CF_ASYNC_NODE_FINISH (1 << 9)

#define CF_SYNC_LIST_FINISH (1 << 0)
#define CF_SYNC_NODE_FINISH (1 << 1)

/*!
rop modes
*/
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

typedef enum mtGPE_COLORFMT_CATEGORY_E {
    GPE_COLORFMT_CATEGORY_ARGB,
    GPE_COLORFMT_CATEGORY_CLUT,
    GPE_COLORFMT_CATEGORY_An,
    GPE_COLORFMT_CATEGORY_YCbCr,
    GPE_COLORFMT_CATEGORY_BYTE,
    GPE_COLORFMT_CATEGORY_HALFWORD,
    GPE_COLORFMT_CATEGORY_MB,
    GPE_COLORFMT_CATEGORY_BUTT
} GPE_COLORFMT_CATEGORY_E;

/*!
  comments
  */
typedef struct
{
  MT_BOOL               little_endian;
  mt_u32                palt_buf;
  mt_u32                palt_size;
  palette_format_t   palt_format;
}palt_info_t;

/*!
  comments
  */
typedef struct
{
  color_format_t     color_fmt;  
  MT_BOOL               alpha_ch_en;
  MT_BOOL               alpha_pre_mult_en;
  MT_BOOL               little_endian;
  mt_u32                bpp;
//  mt_u32                sub_fmt;
//  symphony_space_t   aria_space;
  color_space_t  color_space;
  MT_BOOL               is_pix_alpha;
}color_info_t;
/*!
  comments
  */
typedef struct
{
  ulong                chroma_addr;
  ulong                luma_addr;
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
  MT_BOOL               negative_stride; //only used in src1
  rect_vsb_t         rect;
  color_info_t       color_info;
  //plane alpha
  MT_BOOL               plane_alpha_en; //only for src1 and src3
  mt_u32                plane_alpha; //only for src1 and src3
  //color key
  MT_BOOL               ck_en;
  mt_u32                ck_min;
  mt_u32               ck_max;
  mt_u32               ck_mod;
  mt_u32            ck_select;
  
  //palette
  MT_BOOL               with_palette;
  MT_BOOL               palt_little_endian;
  palette_format_t   palt_format;
  mt_u32 palt_size;                
  phys_addr_t palt_buf;
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
  
  gradt_stop_t       stop0;
  gradt_stop_t       stop1;
  gradt_stop_t       stop2;
  gradt_stop_t       stop3;
  mt_s32                stop_fact0;
  mt_s32                stop_fact1;
  mt_s32                stop_fact2;
  
  //pattern paint
  tiling_mod_t       tiling_mod;
  pos_t              pat_beg;
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
  mt_s32 *coef;
  MT_BOOL disable_alpha_filter;
  MT_BOOL disable_color_filter;
  MT_BOOL disable_alpha_filter_h;
  MT_BOOL disable_color_filter_h;  
  MT_BOOL disable_alpha_anti_flicker;
  MT_BOOL disable_color_anti_flicker;
  mt_u32 init_phase;
  mt_u32 init_phase_x;
  mt_u32 init_phase_y;  
  scale_mod_t scale_mod;
}scale_cfg_t;

/*!
  comments
  */
typedef struct
{
    mt_u32 blur_tap;
}blur_cfg_t;

typedef struct cmdfifo_node
{
    ulong p_node_addr_vir;
    phys_addr_t p_node_addr_phy;
    mt_u32 node_size;
    struct cmdfifo_node *p_prev;
    struct cmdfifo_node *p_next;
    MT_BOOL need_suspend;
#ifdef CONFIG_MT_FPGA_GPE
    ulong dst_addr;
    mt_u32 dst_pitch;
    rect_vsb_t dst_rect;    
#endif
}cmdfifo_node_t;

/*!
  comments
  */
  typedef struct
  {
    /*!
      comments
      */
    phys_addr_t buf;
    /*!
      unit:byte
      */
    mt_u32 pitch;
    /*!
      unit:pixel
      */
    mt_u32 width;
    /*!
      unit:pixel
      */
    mt_u32 height;
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
    mt_u32 plane_alpha;
    /*!
      color key
      */
    MT_BOOL ck_en;
    /*!
      comments
      */
    //mt_u32 key_color;
    mt_u32 ck_min;
    mt_u32 ck_max;
    /*!
      tile
      */
    mt_u8 field_flag;
    /*!
      comments
      */
    phys_addr_t chroma_addr;
    /*!
      added in symphony, for semi-planner and tile yuv format
      */    
    mt_u32 chroma_pitch;
    /*!
      comments
      */
    phys_addr_t luma_addr;
    /*!
      xylc
      */
    mt_u32 xylc_num;
    /*!
      comments
      */
    mt_u32 xylc_color;
        /*!
     palette
      */
    phys_addr_t  palette_base;
    /*!
      comments
      */
    mt_u32 palette_size;
    /*!
      added in symphony, new colorkey rule
      */       
    mt_u32               key_color_mod;
    /*!
      added in symphony, new colorkey rule
      */       
    mt_u32 key_color_select;
  }image_info_t;


/*!
  comments
  */
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
    mt_u32 spread_mod;
    /*!
      comments
      */
    mt_u32 mask_mod;
  }paint_liner_gradt_cfg_t;
/*!
  comments
  */
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
    mt_u32 radius;
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
    mt_u32 spread_mod;
    /*!
      comments
      */
    mt_u32 mask_mod;
  }paint_radial_gradt_cfg_t;
/*!
  comments
  */
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
    mt_u32 long_a;
    /*!
      comments
      */
    mt_u32 short_b;
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
    mt_u32 spread_mod;
    /*!
      comments
      */
    mt_u32 mask_mod;
    /*!
      comments
      */
    mt_u32 stop_num;
  }paint_ellipse_gradt_cfg_t;
/*!
  comments
  */
  typedef struct
  {
    /*!
      comments
      */
    pos_t pat_beg;
    /*!
      comments
      */
    mt_u32 fill_color;
    /*!
      VGTilingMode
      */
    mt_u32 tiling_mod;
  }paint_pattern_cfg_t;

  typedef struct
  {
    /*!
      VGPaintType
      */
    mt_u32 paint_type;
    /*!
      paint color config
      */
    mt_u32 paint_color;
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


/*!
  This structure defines blend factor
  */
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
    MT_BOOL demultiply_en;
}blend_cfg_t;

/*!
  comments
*/
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
    mt_u32 rop_pattern;
}rop_cfg_t;

/*!
    output mask
   */
typedef struct
{
  /*!
  buffer for output mask data
  */
  phys_addr_t mbuf_addr;
  /*!
  8bytes aligned
  */
  mt_u32 mbuf_pitch;
  /*!
  0: store mask to mask buf, 
  1: store mask to alpha, no mask buf needed, 
  2: store src alpha to mask buf
  */
  mt_u32 mask2alpha; 
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
    aria_rotator_op_t rotator_op;
   /*!
     comments
   */
   MT_BOOL src_with_mask;
   phys_addr_t src_mask_buf;
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
    /*!
      blend cfg for alpha
      */
    blend_cfg_t blend_alpha;
    clip_cfg_t clip;
    MT_BOOL colorize_en;
    mt_u32 color;   
  }aria_param_t;

/*!
  comments
  */
typedef struct
{
  MT_BOOL src_in;
  MT_BOOL dst_in;
}aria_rop_ch_t;

/*!
    cmdfifo_cfg_t
   */
typedef struct
{
    /*!
      the cmdfifo is round or not
      */
    MT_BOOL is_round;    
    /*!
      for sync list, the triggle signal
      */    
    mt_u32 sync_signal;
    /*!
      for sync list, the triggle signal number
      */       
    mt_u32 sync_num;
    /*!
      for sync list, the delay counter for triggle signal
      */       
    mt_u32 sync_delay_counter;
    /*!
      for sync list, the halt mode when conflict
      */    
    mt_u32 halt_mode;
    /*!
      for async list, the reset mode when conflict
      */     
    mt_u32 async_conf_rst;
    /*!
      for async list, the recovery mode when conflict
      */       
    mt_u32 async_conf_recovery;
    /*!
      for async list, the conflict mode
      */        
    mt_u32 async_conf_mod;   
    /*!
      the cmdfifo is sync list or not
      */      
    MT_BOOL is_sync;
}cmdfifo_cfg_t;

/*!
  max cmdfifo node number, can be larger
  */
#define MAX_NODE_NUM 256
/*!
    cmdfifo_attr_t
   */
typedef struct
{
    /*!
      the cmdfifo node number
      */ 
    mt_u32 node_num;
    /*!
      the cmdfifo node addr
      */     
    ulong node_addr[MAX_NODE_NUM];    
    /*!
      the cmdfifo is sync
      */     
    MT_BOOL is_sync;
#ifdef CONFIG_MT_FPGA_GPE
    /*!
    for verification
      dst buffer pitch
      */  
    mt_u32 dst_pitch[MAX_NODE_NUM];
    /*!
    for verification
      dst buffer addr
      */  
    ulong dst_addr[MAX_NODE_NUM];
    /*!
    for verification
      dst buffer operate rectangle
      */      
    rect_vsb_t dst_rect[MAX_NODE_NUM];    
#endif
}cmdfifo_attr_t;

typedef struct
{
    mt_u32 *p_list;
    cmdfifo_cfg_t cmdfifo_cfg;    
    cmdfifo_attr_t cmdfifo_attr;
}cmdfifo_hdl_t;

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
  phys_addr_t p_gradt_buf;
  
  //alpha map
  MT_BOOL alpha_map_en;
  alpha_map_mod_t alpha_map_mod;
  
  //blend
  MT_BOOL blend_en;
  blend_fact_t  src_blend_fact;
  blend_fact_t  dst_blend_fact;
  blend_fact_t  asrc_blend_fact;
  blend_fact_t  adst_blend_fact;   
  MT_BOOL demultiply_en;
  
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
  rotator_op_t rotator_op;
  aria_rop_ch_t rop_ch;

  //mask
  MT_BOOL src_with_mask;
  phys_addr_t src0_buf;
  mt_u32 src0_pitch;
  MT_BOOL dst_with_mask;
  dst_mask_cfg_t dst_mask;

  //for cmdfifo
  MT_BOOL is_cmdfifo;  
  cmdfifo_node_t *p_node;

  mt_u32 cmyk_max;
  mt_u32 cmyk_coef;

  clip_cfg_t clip;
  MT_BOOL colorize_en;
  mt_u32 color;  
  ulong src0_buf_vir;
  mt_u32 *p_scale_tab;
}aria_gpe_context_t;

/*!
  This structure defines the type of the private data in a GPE device.
  */
typedef struct hdl_gpe
{
  MT_BOOL bInUse;    /* is used now? */
  /*!
  The pointer to the private variables and structures.
  */
  aria_gpe_context_t gpe_ctx;  
}hdl_gpe_t;

#ifdef CONFIG_MT_FPGA_GPE
typedef struct
{
    MT_BOOL print_reg0;
    MT_BOOL print_reg1;
    MT_BOOL compare_with_sw;
    MT_BOOL show_sw_result;
    MT_BOOL key_set;
    MT_BOOL key_msk;
    u32 zero_edge;
    MT_BOOL pause_on;
    MT_BOOL reset_on;
    MT_BOOL clip_en;
    u32 rotator_op;
    u32 no_rst;
    u32 cmyk_max;
    u32 src1_bitswap;
    u32 src3_bitswap;
    MT_BOOL use_new_expmode;
    MT_BOOL dst_premult_en;
    MT_BOOL mirror;
    MT_BOOL tmp_mem;
    u32 ds_mod;
    MT_BOOL mask_test_en;
    MT_BOOL new_blend_en;
    MT_BOOL wr_last_mod;
    MT_BOOL old_pfm_mod;
     MT_BOOL demultiply_en;

    ulong chroma_vir_addr;

    
    u32 comset[10];
    u32 int_mode_en;
    u32 fast2d_flt_val;
}spn_debug_t;

void gpe_symphony_set_debug(void *p_info);
void gpe_symphony_get_debug(void *p_info);


mt_s32 spn_cmdfifo_hang_req(mt_void);
mt_s32 spn_cmdfifo_hang_release(mt_void);
mt_s32 spn_cmdfifo_cpu_unsuspend(ulong p_cf_hdl, MT_BOOL suspend_immediate);
mt_s32 spn_cmdfifo_cpu_suspend(ulong p_cf_hdl, MT_BOOL suspend_immediate);
mt_s32 spn_cmdfifo_stop(ulong p_cf_hdl);
mt_s32 spn_cmdfifo_get_info(ulong p_cf_hdl, cmdfifo_attr_t *p_attr);
mt_s32 GpeOsiVideoScreenCaptureFpga(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_RECT_S  *pstSrcRect, TDE2_OPT_S* pstOpt);
mt_s32 gpe_gaussian_blur(TDE_HANDLE s32Handle, 
        TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
        TDE2_SURFACE_S* pstExGround, TDE2_RECT_S  *pstExGroundRect,
        TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);
mt_s32 GpeOsiImageDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
    void *pData,void *p_palette,mt_s32 pal_entris,mt_s32 s32Stride,mt_u32 in_size,
    mt_s32 image_w,mt_s32 image_h,TDE2_COLOR_FMT_E color_fmt,TDE2_RECT_S *pstSrcRect,TDE2_OPT_S *pstOpt);
#endif

mt_s32 GpeOsiBeginJob(TDE_HANDLE *ps32Handle);
mt_s32 GpeOsiEndJob(TDE_HANDLE s32Handle, MT_BOOL bBlock, mt_u32 u32TimeOut,
                        MT_BOOL bSync, TDE_FUNC_CB pFuncComplCB, mt_void *pFuncPara);

mt_s32 GpeOsiBlit(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                          TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, TDE2_SURFACE_S* pstDst,
                          TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pOpt);
mt_s32 GpeOsiBlit_3src(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
                      TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, 
                      TDE2_SURFACE_S* pstExGround, TDE2_RECT_S  *pstExGroundRect, 
                      TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);
mt_s32 GpeOsiQuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect,
                           mt_u32 u32FillData);
mt_s32 GpeOsiQuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect);
mt_s32 GpeOsiQuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                       TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect);
mt_void GpeOsiScaleCoeff_New(mt_void);
mt_void GpeOsiScaleCoeff_Del(mt_void) ;
mt_s32 GpeOsiVideoScreenCapture(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt);
mt_s32 GpeClipMask(TDE_HANDLE s32Handle,
                                       TDE2_SURFACE_S *pstForeGround,
                                       TDE2_SURFACE_S *pstDst,
                                       TDE2_RECT_S *pstForeGroundRect,
                                       TDE2_RECT_S *pstDstRect,
                                       TDE2_MASK_OPT_E enMaskOpt);
mt_s32 GpeOsiRotateAribtraryAngle(TDE_HANDLE s32Handle,
                                        TDE2_SURFACE_S *pstSrc,
                                        TDE2_SURFACE_S *pstDst,
                                        TDE2_RECT_S *pstSrcRect,
                                        TDE2_POS_S *pDstPos,
                                        TDE2_ROTATOR_ANGLE_S *pAngle,
                                        TDE2_TRAPEZ_OPT_S *pOpt);
mt_s32 gpe_create_mid_memory(mt_u32 mem_size);
mt_s32 gpe_destroy_mid_memory(mt_void);
mt_s32 GpeOsiDraw_3d_trapez(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
                         TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect, TDE2_TRAPEZ_OPT_S *pstOpt);


#define OLD_EXP_MODE

#define PARA_FRA  14
#define PARA_FRA_22  14
#define PARA_FRA_23_33  22
#define SOR_FRA 18

MT_BOOL is_lut(mt_u32 pix_fmt);
mt_u32 aria_color_expend(pix_fmt_t fmt, mt_u32 pk, MT_BOOL ck_exp);
mt_u32 aria_lut_patterncolor(pix_fmt_t fmt, mt_u32 color);
mt_u32 colorkey_expend(TDE2_COLOR_FMT_E enColorFmt, mt_u32 pk);
mt_s32 getColorkeyMinMax(mt_u32 colorkey,TDE2_COLOR_FMT_E enColorFmt, TDE2_COLORKEY_U *pKeyValue);

#ifdef SUPPORT_CMDFIFO
mt_s32 gpe_create_cmdfifo_memory(mt_void);
mt_s32 gpe_destroy_cmdfifo_memory(mt_void);
mt_s32 get_cmdfifo_buf(ulong *p_viraddr, phys_addr_t *p_phyaddr);
mt_s32 spn_cmdfifo_create_list_begin(TDE_HANDLE *p_cf_hdl, cmdfifo_cfg_t *p_cmdfifo_cfg);
mt_s32 spn_cmdfifo_destroy(TDE_HANDLE p_cf_hdl);
mt_s32 spn_cmdfifo_create_list_end(TDE_HANDLE p_cf_hdl);
mt_s32 spn_cmdfifo_run(TDE_HANDLE p_cf_hdl);
mt_u32 spn_cmdfifo_get_node_cnt(mt_void);
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */

#endif  /* _GPE_H_ */

