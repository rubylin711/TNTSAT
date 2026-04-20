/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/ 
#ifndef __MTFB_H__
#define __MTFB_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */



#include "linux/fb.h"

#include "mt_type.h"
#include "mt_common.h"



/*************************** Macro Definition ****************************/
/** \addtogroup           MTFB   */
/** @{ */  /** <!-- [MTFB] */
#define IOC_TYPE_MTFB       'F'

/**Obtains the colorkey of a graphics layer.*/
/**CNcomment:获取图层colorkey信息 */
#define FBIOGET_COLORKEY_MTFB       _IOR(IOC_TYPE_MTFB, 90, MTFB_COLORKEY_S)

/**Sets the colorkey of a graphics layer.*/
/**CNcomment:设置图层colorkey信息 */
#define FBIOPUT_COLORKEY_MTFB       _IOW(IOC_TYPE_MTFB, 91, MTFB_COLORKEY_S)

/**Obtains the alpha values of a graphics layer, including the pixel alpha and global alpha.*/
/**CNcomment:获取图层alpha信息,包括像素和全局alpha */
#define FBIOGET_ALPHA_MTFB          _IOR(IOC_TYPE_MTFB, 92, MTFB_ALPHA_S)

/**Sets the alpha values of a graphics layer, including the pixel alpha and global alpha.*/
/**CNcomment:设置图层alpha信息,包括像素和全局alpha */
#define FBIOPUT_ALPHA_MTFB          _IOW(IOC_TYPE_MTFB, 93, MTFB_ALPHA_S)

/**Obtains the origin of a graphics layer.*/
/**CNcomment:获取图层屏幕原点信息 */
#define FBIOGET_SCREEN_ORIGIN_MTFB  _IOR(IOC_TYPE_MTFB, 94, MTFB_POINT_S)

/**Sets the origin of a graphics layer.*/
/**CNcomment:设置图层屏幕原点信息 */
#define FBIOPUT_SCREEN_ORIGIN_MTFB  _IOW(IOC_TYPE_MTFB, 95, MTFB_POINT_S)

/**Obtains the anti-flicker status of a graphics layer.*/
/**CNcomment:获取图层抗闪烁*/
#define FBIOGET_DEFLICKER_MTFB       _IOR(IOC_TYPE_MTFB, 98, MTFB_DEFLICKER_S)

/**Sets the anti-flicker status of a graphics layer.*/
/**CNcomment:设置图层抗闪烁 */
#define FBIOPUT_DEFLICKER_MTFB       _IOW(IOC_TYPE_MTFB, 99, MTFB_DEFLICKER_S)

/**Waits for the vertical blanking interrupt of a graphics layer.*/
/**CNcomment:等待图层垂直水隐中断 */
#define FBIOGET_VBLANK_MTFB         _IO(IOC_TYPE_MTFB, 100)

/**Sets to display a graphics layer.*/
/**CNcomment:设置图层显示 */
#define FBIOPUT_SHOW_MTFB           _IOW(IOC_TYPE_MTFB, 101, MT_BOOL)

/**Obtains the display information about a graphics layer.*/
/**CNcomment:获取图层显示显示信息 */
#define FBIOGET_SHOW_MTFB           _IOR(IOC_TYPE_MTFB, 102, MT_BOOL)

/**Obtains the capability of a graphics layer.*/
/**CNcomment:获取图层能力 */
#define FBIOGET_CAPABILITY_MTFB    _IOR(IOC_TYPE_MTFB, 103, MTFB_CAPABILITY_S)

/* crusor handle */
/* Attention:surface in cursor will be released by user*/




/** cursor will be separated from attached layer automatically if you attach cursor to another layer,that means
cursor can be attached to only one layer at any time*/


/**Sets the information about a graphics layer.*/
/**CNcomment:设置图层信息 */
#define FBIOPUT_LAYER_INFO                _IOW(IOC_TYPE_MTFB, 120, MTFB_LAYER_INFO_S*)

/**Obtains the information about a graphics layer.*/
/**CNcomment:获取图层信息 */
#define FBIOGET_LAYER_INFO                _IOR(IOC_TYPE_MTFB, 121, MTFB_LAYER_INFO_S*)

/**Obtains a canvas buffer.*/
/**CNcomment:获取canvas buf */
#define FBIOGET_CANVAS_BUFFER             _IOR(IOC_TYPE_MTFB, 123, MTFB_BUFFER_S*) 

/**Refreshes a graphics layer.*/
/**CNcomment:刷新图层 */
#define FBIO_REFRESH                      _IOW(IOC_TYPE_MTFB, 124, MTFB_BUFFER_S*) 

/**sync refresh*/
#define FBIO_WAITFOR_FREFRESH_DONE        _IO(IOC_TYPE_MTFB, 125)

/** set Encoder picture delivery method*/
#define FBIOPUT_ENCODER_PICTURE_FRAMING   _IOW(IOC_TYPE_MTFB, 126, MTFB_STEREO_MODE_E*)
/** get Encoder picture delivery method */
#define FBIOGET_ENCODER_PICTURE_FRAMING   _IOR(IOC_TYPE_MTFB, 127, MTFB_STEREO_MODE_E*)

/*set 3D Stereo work mode*/
#define FBIOPUT_STEREO_MODE _IOW(IOC_TYPE_MTFB, 128, MTFB_STEREO_WORKMODE_E*)
/*get 3D Stereo work mode*/
#define FBIOGET_STEREO_MODE _IOW(IOC_TYPE_MTFB, 129, MTFB_STEREO_WORKMODE_E*)


/**Sets the output range of the screen.*/
/**CNcomment:设置屏幕输出范围*/
#define FBIOPUT_SCREENSIZE          _IOW(IOC_TYPE_MTFB, 130, MTFB_SIZE_S*)

/**Obtains the output range of the screen.*/
/**CNcomment:获取屏幕输出范围*/
#define FBIOGET_SCREENSIZE          _IOR(IOC_TYPE_MTFB, 131, MTFB_SIZE_S*)

/*set compression mode, only support pixel format argb8888, and stereo mode not support top and bottom*/
#define FBIOPUT_COMPRESSION  _IOW(IOC_TYPE_MTFB, 132, MT_BOOL*)
/*get compression mode */
#define FBIOGET_COMPRESSION  _IOR(IOC_TYPE_MTFB, 133, MT_BOOL*)

/**create scroll text*/
#define FBIO_SCROLLTEXT_CREATE   _IOW(IOC_TYPE_MTFB, 134, MTFB_SCROLLTEXT_CREATE_S*) 
/**send to scroll text buffer*/
#define FBIO_SCROLLTEXT_FILL     _IOW(IOC_TYPE_MTFB, 135, MTFB_SCROLLTEXT_DATA_S*) 
/**pause scroll text*/
#define FBIO_SCROLLTEXT_PAUSE    _IOW(IOC_TYPE_MTFB, 136, mt_u32*) 
/**resume scroll text*/
#define FBIO_SCROLLTEXT_RESUME   _IOW(IOC_TYPE_MTFB, 137, mt_u32*) 
/**destory scroll text*/
#define FBIO_SCROLLTEXT_DESTORY  _IOW(IOC_TYPE_MTFB, 138, mt_u32*)

/*set fb stereo depth*/
#define FBIOPUT_STEREO_DEPTH  _IOW(IOC_TYPE_MTFB, 139, mt_s32*)
/*get fb stereo depth*/
#define FBIOGET_STEREO_DEPTH  _IOR(IOC_TYPE_MTFB, 140, mt_s32*)

/**set the priority of layer in gp*/
#define FBIOPUT_ZORDER          _IOW(IOC_TYPE_MTFB, 141, MTFB_ZORDER_E *)
/**gett the priority of layer in gp*/
/**default setting:
	HD0: G0(mtfb0) ---- zorder == 1  -->bottom
	HD0: G1(mtfb1) ---- zorder == 2
	HD0: G2(mtfb2) ---- zorder == 3 -->top
	--------------------------
	HD1: G4(mtfb4) ---- zorder == 1*/
#define FBIOGET_ZORDER          _IOW(IOC_TYPE_MTFB, 142, mt_u32 *)

/**free logo*/
#define FBIO_FREE_LOGO          _IO(IOC_TYPE_MTFB, 143)

/*set compression update mode*/
#define FBIOPUT_COMPRESSIONMODE  _IOW(IOC_TYPE_MTFB, 144, MTFB_CMP_MODE_E*)
/*get compression mode */
#define FBIOGET_COMPRESSIONMODE  _IOR(IOC_TYPE_MTFB, 145, MTFB_CMP_MODE_E*)

#define FBIO_HWC_REFRESH _IOR(IOC_TYPE_MTFB, 146, MTFB_HWC_LAYERINFO_S*)

#define FBIOGET_DMABUF _IOR('F', 0x21, struct fb_dmabuf_export)


/** @} */  /** <!-- ==== Macro Definition end ==== */



/*************************** Structure Definition ****************************/
/** \addtogroup           MTFB   */
/** @{ */  /** <!-- [MTFB] */

struct fb_dmabuf_export
{
	mt_u32 fd;
	mt_u32 flags;
};

/**Definition of the range type*/
/**CNcomment:范围类型定义*/
typedef struct
{
    mt_u32  u32Width;         /**<Output width of the screen*//**<CNcomment:屏幕上输出的宽度 */
    mt_u32  u32Height;        /**<Output height of the screen*//**<CNcomment:屏幕上输出的高度 */
}MTFB_SIZE_S;

/**layer ID */
/**CNcomment:图层ID定义*/
typedef enum 
{
    MTFB_LAYER_BACKGROUND = 0x0,
    MTFB_LAYER_OSD0,
    MTFB_LAYER_OSD1,
    MTFB_LAYER_SUB,
    MTFB_LAYER_STILL,
    MTFB_LAYER_HD_0,
    MTFB_LAYER_HD_1,
    MTFB_LAYER_HD_2,
    MTFB_LAYER_HD_3,
    MTFB_LAYER_SD_0,    
    MTFB_LAYER_SD_1,    
    MTFB_LAYER_SD_2,     
    MTFB_LAYER_SD_3, 
    MTFB_LAYER_AD_0,  
    MTFB_LAYER_AD_1,
    MTFB_LAYER_AD_2,
    MTFB_LAYER_AD_3,
    MTFB_LAYER_CURSOR,
    MTFB_LAYER_ID_BUTT
} MTFB_LAYER_ID_E;


/**Obtains the extended 32-bit color value based on color components. The upper bits are padded with lower bits during color extension.*/
/**CNcomment:根据颜色分量，获取其扩展到32位后值。扩展规则:低位补其高位*/
static inline mt_u8  mtfb_rgb(const struct fb_bitfield* pBit, mt_s32 color)
{
    return ((mt_u8)((((mt_u32)color)>>pBit->offset) << (8-pBit->length)) +
             ((mt_u8)(((mt_u32)(color)>>pBit->offset) << (8-pBit->length)) >> pBit->length));
}

/**Converts a colorkey to a 32-bit colorkey based on the pixel format. A lower-bit color (such as 16 bits) is extended to an upper-bit color (such as 32 bits) as follows:
   For each chip, upper bits can be padded with lower bits in multiple mode. The padding mode of the chip must be consistent with the preceding extension mode. Otherwise, the colorkey of the graphics layer is invalid.*/
/**CNcomment:根据像素格式信息，将一个颜色值，转换成一个32位的颜色key值,将低色深如16bit扩展为高色深如32bit的策略是:
   低位补高几位，芯片有多种方式。芯片的方式必须如这个方式一致，否则图层colorkey无效*/
static inline mt_s32 mtfb_color2key(const struct fb_var_screeninfo* pVar, mt_s32 color)
{
   if (pVar->bits_per_pixel <= 8)
   {
       return color; 
   }
   else
   {
      mt_u8 r, g, b;
      r = mtfb_rgb(&pVar->red, color);
      g = mtfb_rgb(&pVar->green, color);      
      b = mtfb_rgb(&pVar->blue, color);
      return (r<<16) + (g<<8) + b;
   }
}


typedef struct
{
    MT_BOOL bKeyEnable;         /**<Colorkey enable*//**<CNcomment:colorkey 是否使能 */
    mt_u32 u32Key;              /**<The value is obtained by calling ::mtfb_color2key.*//**<CNcomment:该值应该通过::mtfb_color2key得到*/
}MTFB_COLORKEY_S;

/**Rectangle information*/
/**CNcomment:矩形信息 */
typedef struct
{
    mt_s32 x, y;    /**<x: horizontal coordinate of the upper left point of the rectangle; y: vertical coordinate of the upper left point of the rectangle*//**<CNcomment: x:矩形左上点横坐标 y:矩形左上点纵坐标*/
    mt_s32 w, h;    /**< w: rectangle width; h: rectangle height*//**<CNcomment: w:矩形宽度 h:矩形高度*/
} MTFB_RECT;

typedef struct
{
    mt_s32 l;
    mt_s32 t;
    mt_s32 r;
    mt_s32 b;
}MTFB_REGION;

/**Coordinate of a point*/
/**CNcomment:点坐标信息 */
typedef struct
{
    mt_s32 s32XPos;         /**<  horizontal position *//**<CNcomment:水平位置 */
    mt_s32 s32YPos;         /**<  vertical position *//**<CNcomment:垂直位置 */
}MTFB_POINT_S;

/**Anti-flicker information*/
/**CNcomment:抗闪烁信息 */
typedef struct hiMTFB_DEFLICKER_S
{
    mt_u32  u32HDfLevel;    /**<  horizontal deflicker level *//**<CNcomment:水平抗闪烁级别 */     
    mt_u32  u32VDfLevel;    /**<  vertical deflicker level *//**<CNcomment:垂直抗闪烁级别 */
    mt_u8   *pu8HDfCoef;    /**<  horizontal deflicker coefficient *//**<CNcomment:水平抗闪烁系数 */
    mt_u8   *pu8VDfCoef;    /**<  vertical deflicker coefficient *//**<CNcomment:垂直抗闪烁系数 */
}MTFB_DEFLICKER_S;

/**Alpha information*/
/**CNcomment:Alpha信息 */
typedef struct
{
    MT_BOOL bAlphaEnable;   /**<  alpha enable flag *//**<CNcomment:alpha使能标识*/
    MT_BOOL bAlphaChannel;  /**<  alpha channel enable flag *//**<CNcomment:alpha通道使能*/
    mt_u8 u8Alpha0;         /**<  alpha0 value, used in ARGB1555 *//**CNcomment:alpha0取值，ARGB1555格式时使用*/
    mt_u8 u8Alpha1;         /**<  alpha1 value, used in ARGB1555 *//**CNcomment:alpha1取值，ARGB1555格式时使用*/
    mt_u8 u8GlobalAlpha;    /**<  global alpha value *//**<CNcomment:全局alpha取值*/
    mt_u8 u8Reserved;
    MT_BOOL bRegionAlphaEnable;   /**<  region alpha enable flag *//**<CNcomment: region alpha使能标识*/
    mt_u8 u8RegionAlpha;    /**<  region alpha value *//**<CNcomment: region alpha取值*/
}MTFB_ALPHA_S;

/**Pixel format*/
/**CNcomment:像素格式枚举 */
typedef enum
{        
    MTFB_FMT_RGB565 = 0,    /**<  RGB565 16bpp */   
    MTFB_FMT_RGB565_SMALL_ENDIAN,
    MTFB_FMT_RGB888,		  /**<  RGB888 24bpp */
    MTFB_FMT_KRGB444,       /**<  RGB444 16bpp */
    MTFB_FMT_KRGB555,       /**<  RGB555 16bpp */
    
    MTFB_FMT_KRGB888,       /**<  RGB888 32bpp */
    MTFB_FMT_ARGB4444,      /**< ARGB4444 */  
    MTFB_FMT_ARGB4444_SMALL_ENDIAN,
    MTFB_FMT_ARGB1555,      /**< ARGB1555 */
    MTFB_FMT_ARGB1555_SMALL_ENDIAN,
    MTFB_FMT_ARGB8888,      /**< ARGB8888 */
    MTFB_FMT_ARGB8888_SMALL_ENDIAN,
  
    MTFB_FMT_ARGB8565,      /**< ARGB8565 */
    MTFB_FMT_RGBA4444,      /**< ARGB4444 */
    MTFB_FMT_RGBA4444_SMALL_ENDIAN,
    MTFB_FMT_RGBA5551,      /**< RGBA5551 */
    MTFB_FMT_RGBA5551_SMALL_ENDIAN,
    MTFB_FMT_RGBA5658,      /**< RGBA5658 */
    
    MTFB_FMT_RGBA8888,      /**< RGBA8888 */
    MTFB_FMT_RGBA8888_SMALL_ENDIAN,
    MTFB_FMT_BGR565,        /**< BGR565 */
    MTFB_FMT_BGR888,        /**< BGR888 */
    MTFB_FMT_ABGR4444,      /**< ABGR4444 */
    
    MTFB_FMT_ABGR1555,      /**< ABGR1555 */
    MTFB_FMT_ABGR8888,      /**< ABGR8888 */
    MTFB_FMT_ABGR8565,      /**< ABGR8565 */
    MTFB_FMT_KBGR444,       /**< BGR444 16bpp */
    
    MTFB_FMT_KBGR555,       /**< BGR555 16bpp */
    MTFB_FMT_KBGR888,       /**< BGR888 32bpp */
    MTFB_FMT_1BPP,          /**<  clut1 */
    MTFB_FMT_2BPP,          /**<  clut2 */
    
    MTFB_FMT_4BPP,          /**<  clut4 */ 
    MTFB_FMT_8BPP,          /**< clut8 */
    MTFB_FMT_ACLUT44,       /**< AClUT44*/
    MTFB_FMT_ACLUT88,       /**< ACLUT88 */
    
    MTFB_FMT_PUYVY,         /**< UYVY */
    MTFB_FMT_PYUYV,         /**< YUYV */
    MTFB_FMT_PYVYU,         /**< YVYU */
    MTFB_FMT_PVYUY,
    MTFB_FMT_YUV888,        /**< YUV888 */

    MTFB_FMT_VUYA8888,    
    MTFB_FMT_AYUV8888,      /**< AYUV8888 */
    MTFB_FMT_AVUY8888,
    MTFB_FMT_YUVA8888,      /**< YUVA8888 */

    MTFB_FMT_2BPP_ABGR,
    MTFB_FMT_2BPP_RGBA,
    MTFB_FMT_2BPP_BGRA,
    MTFB_FMT_2BPP_ARGB,
    MTFB_FMT_2BPP_AVUY,
    MTFB_FMT_2BPP_YUVA,
    MTFB_FMT_2BPP_VUYA,
    MTFB_FMT_2BPP_AYUV,

    MTFB_FMT_4BPP_ABGR,
    MTFB_FMT_4BPP_RGBA,
    MTFB_FMT_4BPP_BGRA,
    MTFB_FMT_4BPP_ARGB,
    MTFB_FMT_4BPP_AVUY,
    MTFB_FMT_4BPP_YUVA,
    MTFB_FMT_4BPP_VUYA,
    MTFB_FMT_4BPP_AYUV,

    MTFB_FMT_8BPP_ABGR,
    MTFB_FMT_8BPP_RGBA,
    MTFB_FMT_8BPP_BGRA,
    MTFB_FMT_8BPP_ARGB,
    MTFB_FMT_8BPP_AVUY,
    MTFB_FMT_8BPP_YUVA,
    MTFB_FMT_8BPP_VUYA,
    MTFB_FMT_8BPP_AYUV,

    MTFB_FMT_ACLUT44_ABGR,
    MTFB_FMT_ACLUT44_RGBA,
    MTFB_FMT_ACLUT44_BGRA,
    MTFB_FMT_ACLUT44_ARGB,
    MTFB_FMT_ACLUT44_AVUY,
    MTFB_FMT_ACLUT44_YUVA,
    MTFB_FMT_ACLUT44_VUYA,
    MTFB_FMT_ACLUT44_AYUV,

    MTFB_FMT_ACLUT88_ABGR,
    MTFB_FMT_ACLUT88_RGBA,
    MTFB_FMT_ACLUT88_BGRA,
    MTFB_FMT_ACLUT88_ARGB,
    MTFB_FMT_ACLUT88_AVUY,
    MTFB_FMT_ACLUT88_YUVA,
    MTFB_FMT_ACLUT88_VUYA,
    MTFB_FMT_ACLUT88_AYUV,

    MTFB_FMT_RGB233,

    MTFB_FMT_SP_YUV420,
    MTFB_FMT_SP_YUV422_1x2,
    MTFB_FMT_SP_YUV422_2x1,
    MTFB_FMT_SP_YUV420_UVSWAP,
    MTFB_FMT_SP_YUV422_1x2_UVSWAP,
    MTFB_FMT_SP_YUV422_2x1_UVSWAP,

	MTFB_FMT_SP_CMYK,
    MTFB_FMT_BUTT
}MTFB_COLOR_FMT_E;

/**MtFB capability set*/
/**CNcomment:mtfb能力集 */
typedef struct
{
    MT_BOOL bKeyAlpha;      /**< whether support colorkey alpha */
    MT_BOOL bGlobalAlpha;   /**< whether support global alpha */
    MT_BOOL bCmap;          /**< whether support color map */
    MT_BOOL bHasCmapReg;    /**< whether has color map register*/
    MT_BOOL bColFmt[MTFB_FMT_BUTT]; /**< support which color format */
    MT_BOOL bVoScale;       /**< support vo scale*/
    MT_BOOL bLayerSupported;    /**< whether support a certain layer, for example:x5 HD support MTFB_SD_0 not support MTFB_SD_1*/
    MT_BOOL bCompression;    /**< whether support compression */
    MT_BOOL bStereo;        /**< whether support 3D Stereo*/
    mt_u32  u32MaxWidth;    /**< the max pixels per line */
    mt_u32  u32MaxHeight;   /**< the max lines */
    mt_u32  u32MinWidth;    /**< the min pixels per line */
    mt_u32  u32MinHeight;   /**< the min lines */ 
    mt_u32  u32VDefLevel;   /**< vertical deflicker level, 0 means vertical deflicker is unsupported */
    mt_u32  u32HDefLevel;   /**< horizontal deflicker level, 0 means horizontal deflicker is unsupported  */
}MTFB_CAPABILITY_S;

/*refresh mode*/
typedef enum 
{
    MTFB_LAYER_BUF_DOUBLE = 0x0,         /**< 2 display buf in fb */
    MTFB_LAYER_BUF_ONE    = 0x1,         /**< 1 display buf in fb */
    MTFB_LAYER_BUF_NONE   = 0x2,         /**< no display buf in fb,the buf user refreshed will be directly set to VO*/    
    MTFB_LAYER_BUF_DOUBLE_IMMEDIATE=0x3, /**< 2 display buf in fb, each refresh will be displayed*/
    MTFB_LAYER_BUF_STANDARD = 0x4,       /**< standard refresh*/
    MTFB_LAYER_BUF_BUTT
} MTFB_LAYER_BUF_E;

/* surface info */
typedef struct
{
    phys_addr_t u32PhyAddr;     /**<  start physical address */
    mt_u32  u32Width;       /**<  width pixels */
    mt_u32  u32Height;      /**<  height pixels */   
    mt_u32  u32Pitch;       /**<  line pixels */         
    MTFB_COLOR_FMT_E enFmt; /**<  color format */     
}MTFB_SURFACE_S;


/* refresh surface info */
typedef struct
{
    MTFB_SURFACE_S stCanvas;    
    MTFB_RECT UpdateRect;       /**< refresh region*/
}MTFB_BUFFER_S;

/* cursor info */
typedef struct
{
    MTFB_SURFACE_S stCursor;
    MTFB_POINT_S stHotPos;
} MTFB_CURSOR_S;

/** Encoder picture delivery method */
typedef enum
{
    MTFB_STEREO_MONO   = 0x0,             /**< Normal output to non-stereoscopic (3D) TV. No L/R content provided to TV*/
    MTFB_STEREO_SIDEBYSIDE_HALF,          /**< L/R frames are downscaled horizontally by 2 andpacked side-by-side into a single frame, left on lefthalf of frame*/
    MTFB_STEREO_TOPANDBOTTOM,             /**< L/R frames are downscaled vertically by 2 andpacked into a single frame, left on top*/
    MTFB_STEREO_FRMPACKING,               /**< one frames are copyed */    
    MTFB_STEREO_BUTT
}MTFB_STEREO_MODE_E;

/** 3D stereo mode*/
typedef enum
{
    MTFB_STEREO_WORKMODE_HW_FULL = 0x0,      /**< 3d stereo function use hardware and transfer full frame to vo, note: hardware doesn't support this mode if encoder picture delivery method is top and bottom */   
    MTFB_STEREO_WORKMODE_HW_HALF,            /**< 3d stereo function use hardware and transfer half frame to vo*/
    MTFB_STEREO_WORKMODE_SW_EMUL,            /**< 3d stereo function use software emulation */
    MTFB_STEREO_WORKMODE_BUTT
}MTFB_STEREO_WORKMODE_E;

/**antiflicker level*/
/**Auto means fb will choose a appropriate antiflicker level automatically according to the color info of map*/
typedef enum
{
    MTFB_LAYER_ANTIFLICKER_NONE = 0x0,	/**< no antiflicker*/
    MTFB_LAYER_ANTIFLICKER_LOW = 0x1,	/**< low level*/
    MTFB_LAYER_ANTIFLICKER_MIDDLE = 0x2,/**< middle level*/
    MTFB_LAYER_ANTIFLICKER_HIGH = 0x3,  /**< high level*/
    MTFB_LAYER_ANTIFLICKER_AUTO = 0x4,  /**< auto*/
    MTFB_LAYER_ANTIFLICKER_BUTT
}MTFB_LAYER_ANTIFLICKER_LEVEL_E;

/*layer info maskbit*/
typedef enum
{
    MTFB_LAYERMASK_BUFMODE = 0x1,           /**< Whether the buffer mode in MTFB_LAYER_INFO_S is masked when the graphics layer information is set.*//**<CNcomment:设置图层信息时，MTFB_LAYER_INFO_S中buf模式是否有效掩码 */
    MTFB_LAYERMASK_ANTIFLICKER_MODE = 0x2,  /**< Whether the anti-flicker mode is masked.*//**<CNcomment:抗闪烁模式是否有效掩码 */
    MTFB_LAYERMASK_POS = 0x4,               /**< Whether the graphics layer position is masked.*//**<CNcomment:图层位置是否有效掩码 */
    MTFB_LAYERMASK_CANVASSIZE = 0x8,        /**< Whether the canvas size is masked.*//**<CNcomment:canvassize是否有效掩码 */
    MTFB_LAYERMASK_DISPSIZE = 0x10,         /**< Whether the display size is masked.*//**<CNcomment:displaysize是否有效掩码 */
    MTFB_LAYERMASK_SCREENSIZE = 0x20,       /**< Whether the screen size is masked.*//**<CNcomment:screensize是否有效掩码 */
    MTFB_LAYERMASK_BMUL = 0x40,             /**< Whether the premultiplexed mode is masked.*//**<CNcomment:是否预乘是否有效掩码 */
    MTFB_LAYERMASK_BUTT
}MTFB_LAYER_INFO_MASKBIT;


/**layer info*/
typedef struct
{
    MTFB_LAYER_BUF_E BufMode;
    MTFB_LAYER_ANTIFLICKER_LEVEL_E eAntiflickerLevel;	
    mt_s32 s32XPos;               /**<  the x pos of origion point in screen */
    mt_s32 s32YPos;	               /**<  the y pos of origion point in screen */
    mt_s32 u32CanvasWidth;        /**<  the width of canvas buffer */
    mt_s32 u32CanvasHeight;       /**<  the height of canvas buffer */
    mt_u32 u32DisplayWidth;	   /**<  the width of display buf in fb.for 0 buf ,there is no display buf in fb, so it's effectless*/
    mt_u32 u32DisplayHeight;     /**<  the height of display buf in fb. */
    mt_u32 u32ScreenWidth;        /**<  the width of screen，现在没有用 */
    mt_u32 u32ScreenHeight;       /**<  the height of screen，现在没有用 */
    MT_BOOL bPreMul;               /**<  The data drawed in buf is premul data or not*/
    MT_BOOL bUseNewScreen;        /**<  whether use new screen*/
    mt_u32 u32Mask;			        /**<  param modify mask bit*/
}MTFB_LAYER_INFO_S;

/** scrolltext attibute */
/** CNcomment:滚动字幕属性 */
typedef struct 
{
    MTFB_RECT           stRect;        /**< the position you wanted to show on the screen */
    MTFB_COLOR_FMT_E    ePixelFmt;     /**< the color format of scrolltext content */
    mt_u16              u16CacheNum;   /**< The cached buffer number for store scrolltext content */
    mt_u16              u16RefreshFreq;/**< The refresh frequency you wanted */
    MT_BOOL             bDeflicker;    /**< Whether enable antificker */
}MTFB_SCROLLTEXT_ATTR_S;

typedef struct
{
    MTFB_SCROLLTEXT_ATTR_S stAttr; /**< The scrolltext attribution */
    mt_u32  u32Handle;             /**< The output ID for the scrolltext，将MTGO属性中的图层移到这里变成ID */
}MTFB_SCROLLTEXT_CREATE_S;

/** scrolltext data */
/** CNcomment:滚动字幕数据，地址和stride */
typedef struct
{
    mt_u32  u32Handle;     /**< ID of the scrolltext,这里比MTGO多了个句柄 */
    phys_addr_t  u32PhyAddr;    /**< The physical address of the scrolltext content buffer */
    mt_u8   *pu8VirAddr;   /**< The virtual address of the scrolltext content buffer */
    mt_u32  u32Stride;     /**< The stride of the scrolltext content buffer */
}MTFB_SCROLLTEXT_DATA_S;


#define GP_LAYER_NUM 4
/*virtual window info*/
typedef enum
{
    MTFB_ZORDER_MOVETOP = 0,  /**< Move to the top*//**<CNcomment:移到最顶部*/
    MTFB_ZORDER_MOVEUP,       /**< Move upwards*//**<CNcomment:向上移*/
    MTFB_ZORDER_MOVEBOTTOM,   /**< Move to the bottom*//**<CNcomment:移到最底部*/
    MTFB_ZORDER_MOVEDOWN,     /**< Move downwards*//**<CNcomment:向下移*/
    MTFB_ZORDER_BUTT
}MTFB_ZORDER_E;

/*compression mode*/
typedef enum
{
    MTFB_CMP_REGION = 0,   /**< fb support region compression, default mode*//**<CNcomment:支持局部压缩,默认模式*/
    MTFB_CMP_GLOBAL,       /**< fb support global compression only*//**<CNcomment:仅支持全局压缩*/
    MTFB_CMP_BUTT
}MTFB_CMP_MODE_E;


typedef struct
{
    MTFB_POINT_S stPos;
    MTFB_RECT    stInRect;
    mt_u32       u32LayerAddr;
    mt_u32       u32Stride;
    mt_u32       u32Alpha;  
    MT_BOOL      bPreMul;
    MTFB_COLOR_FMT_E eFmt;
    mt_s32 s32AcquireFenceFd;
    mt_s32 s32ReleaseFenceFd;
}MTFB_HWC_LAYERINFO_S;

/** @} */  /** <!-- ==== Structure Definition end ==== */


#if 0
/******************************* API declaration *****************************/
/** \addtogroup     MtFB */
/** @{ */  /** <!-- 【MtFB】 */

/**
\brief Obtains the variable information about the screen. CNcomment:获取屏幕的可变信息.CNend
\attention \n
By default, the resolution and pixel format of the standard-definition (SD) graphics layer are 720x576 and ARGB1555 respectively; the resolution and pixel format of the high-definition (HD) graphics layer are 1280x720 and ARGB888 respectively;
CNcomment:系统默认的标清图层分辨率为720×576，象素格式为ARGB1555。系统默认的高清图层分辨率为1280×720，象素格式为ARGB888。CNend

\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_VSCREENINFO  ioctl number. CNcomment:ioctl号CNend
\param[out] var  Pointer to the variable information. CNcomment:可变信息结构体指针CNend
\retval 0  SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_VSCREENINFO
\par example
\code
struct fb_var_screeninfo vinfo;
if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
{
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOGET_VSCREENINFO, struct fb_var_screeninfo *var);


/**
\brief Sets the screen resolution and pixel format of the FB. CNcomment:设置Framebuffer的屏幕分辨率和象素格式等。CNend
\attention \n
The resolution must be supported by each overlay layer. You can query the maximum resolution and minimum resolution supported by each overlay layer by calling FBIOGET_CAPABILITY_MTFB.\n
The sum of the actual resolution and offset value must be within the range of the virtual resolution. Otherwise, the system automatically adjusts the actual resolution.
CNcomment:分辨率的大小必须在各叠加层支持的分辨率范围内，各叠加层支持的最大分辨率和最小分辨率可通过FBIOGET_CAPABILITY_MTFB获取。\n
必须保证实际分辨率与偏移的和在虚拟分辨率范围内，否则系统会自动调整实际分辨率的大小让其在虚拟分辨率范围内。CNend

\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_VSCREENINFO ioctl number. CNcomment:ioctl号CNend
\param[in] var Pointer to the variable information. CNcomment:可变信息结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_VSCREENINFO
\par example
\code
//Assume that the actual resolution is 720x576, the virtual resolution is 720x576, the offset value is (0, 0), and the pixel format is ARGB8888. In this case, the sample code is as follows:
//CNcommnet:设置实际分辨率为720×576，虚拟分辨率为720×576，偏移为（0，0），象素格式为ARGB8888的示例代码如下：CNend
struct fb_bitfield r32 = {16, 8, 0};
struct fb_bitfield g32 = {8, 8, 0};
struct fb_bitfield b32 = {0, 8, 0}; 
struct fb_bitfield a32 = {24, 8, 0};
struct fb_var_screeninfo vinfo;
if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
{
    return -1;
}
vinfo.xres_virtual = 720;
vinfo.yres_virtual = 576;
vinfo.xres = 720;
vinfo.yres = 576;
vinfo.activate = FB_ACTIVATE_NOW;
vinfo.bits_per_pixel = 32;
vinfo.xoffset = 0;
vinfo.yoffset = 0;
vinfo.red = r32;
vinfo.green = g32;
vinfo.blue = b32;
vinfo.transp= a32;
if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
{
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOPUT_VSCREENINFO, struct fb_var_screeninfo *var);



/**
\brief Obtains the fixed information about the FB. CNcomment:获取Framebuffer的固定信息。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_FSCREENINFO  ioctl number. CNcomment:ioctl号CNend
\param[out] fix Pointer to the fixed information. CNcomment:固定信息结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
N/A
*/
int ioctl (int fd, FBIOGET_FSCREENINFO, struct fb_fix_screeninfo *fix);



/**
\brief Sets to display contents at different offset positions of the virtual resolution. CNcommnet:设置从虚拟分辨率中的不同偏移处开始显示。CNend
\attention \n
The sum of the actual resolution and offset value must be within the range of the virtual resolution. Otherwise, the setting is unsuccessful.
CNcomment:必须保证实际分辨率与偏移的和在虚拟分辨率范围内，否则设置不成功。CNend

\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPAN_DISPLAY  ioctl number. CNcomment:ioctl号CNend
\param[out] var  Pointer to the variable information. CNcomment:可变信息结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid.  CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
N/A
\par example
\code
//Assume that the actual resolution is 300x300, the virtual resolution is 720x576, the start offset value is (50, 50), and the display position is (300, 0). In this case, the code for the PAN setting is as follows:
//CNcomment:设置实际分辨率为300×300，虚拟分辨率为720×576，起始偏移为（50，50），然后偏移到（300，0）处开始显示的PAN设置代码如下：CNend
struct fb_bitfield r32 = {16, 8, 0};
struct fb_bitfield g32 = {8, 8, 0};
struct fb_bitfield b32 = {0, 8, 0};
struct fb_bitfield a32 = {24, 8, 0};
struct fb_var_screeninfo vinfo;

vinfo.xres_virtual = 720;
vinfo.yres_virtual = 576;
vinfo.xres = 300;
vinfo.yres = 300;
vinfo.activate = FB_ACTIVATE_NOW;
vinfo.bits_per_pixel = 32;
vinfo.xoffset = 50;
vinfo.yoffset = 50;
vinfo.red = r32;
vinfo.green = g32;
vinfo.blue = b32;
vinfo.transp= a32;
if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
{
    return -1;
}
vinfo.xoffset = 300;
vinfo.yoffset = 0;
if (ioctl(fd, FBIOPAN_DISPLAY, &vinfo) < 0)
{
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOPAN_DISPLAY, struct fb_var_screeninfo *var);



/**
\brief Obtains the information about the palette. CNcommnet:获取调色板信息。CNend
\attention \n
None

\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGETCMAP  ioctl number. CNcomment:ioctl号CNend
\param[out] cmap Pointer to the palette. CNcomment:调色板结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUTCMAP
\par example
\code
//only low 8bits valid 
unsigned short Red256[256];   
//only low 8bits valid 
unsigned short Green256[256];  
// only low 8bits valid 
unsigned short Blue256[256];   
struct fb_cmap cmap;

cmap.start = 0;
cmap.len = 256;
cmap.red = Red256;
cmap.green = Green256;
cmap.blue = Blue256;
cmap.transp = 0;

if (ioctl(fd, FBIOGETCMAP, &cmap) < 0)
{
    printf("fb ioctl get cmap err!\n");
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOGETCMAP, struct fb_cmap *cmap);



/**
\brief Sets the information about the palette. CNcomment:设置调色板信息。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUTCMAP  ioctl number. CNcomment:ioctl号CNend
\param[in] cmap Pointer to the palette. CNcomment:调色板结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGETCMAP
\par example
\code
//only low 8bits valid 
unsigned short Red256[256];     
//only low 8bits valid 
unsigned short Green256[256]; 
//only low 8bits valid 
unsigned short Blue256[256];    
struct fb_cmap cmap;

//create a palette which contains 256 color 
Palette_Create(Red256, Green256, Blue256);

cmap.start = 0;
cmap.len = 256;
cmap.red = Red256;
cmap.green = Green256;
cmap.blue = Blue256;
cmap.transp = 0;

if (ioctl(fd, FBIOPUTCMAP, &cmap) < 0)
{
    printf("fb ioctl put cmap err!\n");
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOPUTCMAP, struct fb_cmap *cmap);



/**-----Extended Functions--------*/
/**
\brief Obtains the capability of an overlay layer. CNcomment:获取叠加层的支持能力。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_CAPABILITY_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] pstCap Pointer to the capability. CNcomment:支持能力结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
N/A
*/
int ioctl (int fd, FBIOGET_CAPABILITY_MTFB, MTFB_CAPABILITY_S *pstCap);



/**
\brief Obtains the coordinates of the start display point of an overlay layer on the screen. CNcomment:获取叠加层在屏幕上显示的起始点坐标。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_SCREEN_ORIGIN_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] pstPoint Pointer to the coordinate origin. CNcomment:坐标原点结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_SCREEN_ORIGIN_MTFB
*/
int ioctl (int fd, FBIOGET_SCREEN_ORIGIN_MTFB, MTFB_POINT_S *pstPoint);



/**
\brief Sets the coordinates of the start display point of an overlay layer on the screen. CNcomment:设置叠加层在屏幕上显示的起始点坐标。CNend
\attention \n
If the coordinate origin of an overlay layer is beyond the range, the coordinate origin is set to (u32MaxWidth - u32MinWidth, u32MaxHeight - u32MinHeight) by default.
CNcomment:如果叠加层坐标原点超出了范围，默认将坐标原点设置为（u32MaxWidth –u32MinWidth,，u32MaxHeight –u32MinHeight）。CNend
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_SCREEN_ORIGIN_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[in] pstPoint Pointer to the coordinate origin. CNcomment:坐标原点结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_SCREEN_ORIGIN_MTFB
*/
int ioctl (int fd, FBIOPUT_SCREEN_ORIGIN_MTFB, MTFB_POINT_S *pstPoint);



/**
\brief Obtains the display status of the current overlay layer. CNcomment:获取当前叠加层的显示状态。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_SHOW_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] bShow Status of the current overlay layer. If *bShow is set to MT_TRUE, it indicates that the current overlay layer is shown; if *bShow is set to MT_FALSE, it indicates that the current overlay layer is hidden. CNcomment:指示当前叠加层的状态：*bShow = MT_TRUE：当前叠加层处于显示状态；*bShow = MT_FALSE：当前叠加层处于隐藏状态CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_SHOW_MTFB
*/
int ioctl (int fd, FBIOGET_SHOW_MTFB, MT_BOOL *bShow);



/**
\brief Shows or hides the current overlay layer. CNcomment:显示或隐藏该叠加层。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_SHOW_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[in] bShow Display status of the current overlay layer. If *bShow is set to MT_TRUE, it indicates that the current overlay layer is shown; if *bShow is set to MT_FALSE, it indicates that the current overlay layer is hidden. CNcomment:该叠加层的显示状态：*bShow = MT_TRUE：显示当前叠加层；*bShow = MT_FALSE：隐藏当前叠加层CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_SHOW_MTFB
*/
int ioctl (int fd, FBIOPUT_SHOW_MTFB, MT_BOOL *bShow);



/**
\brief Obtains the colorkey of an overlay layer. CNcomment:获取叠加层的colorkey。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_COLORKEY_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] pstColorKey Pointer to the colorkey. CNcomment:colorkey结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_COLORKEY_MTFB
*/
int ioctl (int fd, FBIOGET_COLORKEY_MTFB, MTFB_COLORKEY_S *pstColorKey);



/**
\brief Sets the colorkey of an overlay layer. CNcomment:设置叠加层的colorkey。CNend
\attention \n
None
\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_COLORKEY_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[in] pstColorKey  Pointer to the colorkey. CNcomment:colorkey结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_COLORKEY_MTFB
\par example
\code
//Assume that the pixel format is ARGB8888. To filter the red color, perform the following setting:
//CNcomment:假设当前象素格式为ARGB8888，则要过滤掉红色,具体设置如下：CNend
MTFB_COLORKEY_S stColorKey;

stColorKey.bKeyEnable = MT_TRUE;
stColorKey.u32Key = 0xFF0000;
if (ioctl(fd, FBIOPUT_COLORKEY_MTFB, &stColorKey) < 0)
{
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOPUT_COLORKEY_MTFB, MTFB_COLORKEY_S *pstColorKey);



/**
\brief Obtains the alpha value of an overlay layer. CNcomment:获取叠加层Alpha。CNend
\attention \n
For details, see the description of ::MTFB_ALPHA_S.
CNcomment:请参见::MTFB_ALPHA_S的说明。CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_ALPHA_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] pstAlpha  Pointer to the alpha value. CNcomment:Alpha结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_ALPHA_MTFB
*/
int ioctl (int fd, FBIOGET_ALPHA_MTFB, MTFB_ALPHA_S *pstAlpha);




/**
\brief Sets the alpha value of an overlay layer. CNcomment:设置叠加层的Alpha。CNend
\attention \n
For details, see the description of ::MTFB_ALPHA_S.
CNcomment:请参见::MTFB_ALPHA_S的说明CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_ALPHA_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[in] pstAlpha  Pointer to the alpha value. CNcomment:Alpha结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_ALPHA_MTFB
*/
int ioctl (int fd, FBIOPUT_ALPHA_MTFB, MTFB_ALPHA_S *pstAlpha);



/**
\brief Obtains the anti-flicker setting of an overlay layer. The HD platform does not support this function. CNcomment:获取叠加层的抗闪烁设置。高清平台不支持设置。CNend
\attention \n
Before obtaining the values of anti-flicker parameters, you must set the maximum anti-flicker level that can be obtained, and allocate sufficient memory for storing anti-flicker coefficients. This API is invalid for the HD platform currently.
CNcomment:在获取抗闪烁参数时，必须设置能够获取抗闪烁的最大级别，且为抗闪烁系数分配足够的内存。目前该接口在高清无效。CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_DEFLICKER_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[out] pstDeflicker  Pointer to the anti-flicker setting. CNcomment:抗闪烁结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_DEFLICKER_MTFB
\par example
\code
//Assume that the maximum horizontal and vertical anti-flicker levels that can be obtained are 2. In this case, the sample code is as follows:
//CNcomment:获取水平和垂直抗闪烁最大级别为2的示例代码如下：CNend
mt_u8 u8HDefCoef;
mt_u8 u8VDefCoef;
MTFB_DEFLICKER_S stDeflicker;

stDeflicker.u32HDfLevel = 2;
stDeflicker.u32VDfLevel = 2;
stDeflicker.pu8HDfCoef = &u8HDefCoef;
stDeflicker.pu8VDfCoef = &u8VDefCoef;

if (ioctl(fd, FBIOGET_DEFLICKER_MTFB, &stDeflicker) < 0)
{
    return -1;
}

\endcode
*/
int ioctl (int fd, FBIOGET_DEFLICKER_MTFB, MTFB_DEFLICKER_S *pstDeflicker);



/**
\brief Sets the anti-flicker function of an overlay layer. CNcomment:设置叠加层的抗闪烁功能。CNend
\attention \n
For details, see the description of ::MTFB_DEFLICKER_S. This API is invalid for the HD platform currently.
CNcomment:请参见::MTFB_DEFLICKER_S的说明。目前该接口在高清无效。CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_DEFLICKER_MTFB  ioctl number. CNcomment:ioctl号CNend
\param[in] pstDeflicker  Pointer to the anti-flicker setting. CNcomment:抗闪烁结构体指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_DEFLICKER_MTFB
\par example
\code
//Assume that the 2-tap horizontal and vertical anti-flicker functions are required. In this case, the sample code is as follows:
//CNcomment:设置水平和垂直2阶抗闪烁的代码如下：CNend
mt_u8 u8HDefCoef = 0x80;
mt_u8 u8VDefCoef = 0x80;
MTFB_DEFLICKER_S stDeflicker;

stDeflicker.u32HDfLevel = 2;
stDeflicker.u32VDfLevel = 2;
stDeflicker.pu8HDfCoef = &u8HDefCoef;
stDeflicker.pu8VDfCoef = &u8VDefCoef;

if (ioctl(fd, FBIOPUT_DEFLICKER_MTFB, &stDeflicker) < 0)
{
    return -1;
}
\endcode
*/
int ioctl (int fd, FBIOPUT_DEFLICKER_MTFB, MTFB_DEFLICKER_S *pstDeflicker);



/**
\brief Waits for the vertical blanking region of an overlay layer. To operate the display buffer without tearing, you can operate it in the vertical blanking region. CNcomment:为了操作显存时而不引起撕裂现象，一般可以在该叠加层的垂直消隐区对显存进行操作，通过该接口可以等待该叠加层垂直消隐区的到来。CNend
\attention \n
None
\param[in] fd ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_VBLANK_MTFB  ioctl number. CNcomment:ioctl号CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
N/A
*/
int ioctl (int fd, FBIOGET_VBLANK_MTFB);


/**
\brief Obtains the stereo frame encode format. CNcomment:获取Stereo帧编码格式功能。CNend
\attention \n
For details, see the description of ::MTFB_STEREO_MODE_E. 

\param[in] fd  Framebuffer设备号CNend
\param[in] FBIOGET_STEREO_MODE  ioctl号CNend
\param[out] penStereoMode  stereo模式枚举指针CNend
\retval  0 SUCCESS 成功CNend
\retval ::EPERM  1,不支持该操作CNend
\retval ::ENOMEM  12,内存不够CNend
\retval ::EFAULT  14,传入参数指针地址无效CNend
\retval ::EINVAL  22,传入参数无效CNend
\see \n
::FBIOPUT_STEREO_MODE
*/
int ioctl (int fd, FBIOGET_STEREO_MODE, MTFB_STEREO_MODE_E *penStereoMode);


/**
\brief 设置Stereo模式功能。CNend
\attention \n
请参见::MTFB_STEREO_MODE_E的说明。CNend

\param[in] fd  Framebuffer设备号CNend
\param[in] FBIOPUT_STEREO_MODE  ioctl号CNend
\param[in] penStereoMode  stereo模式枚举指针CNend
\retval  0 SUCCESS 成功CNend
\retval ::EPERM  1,不支持该操作CNend
\retval ::ENOMEM  12,内存不够CNend
\retval ::EFAULT  14,传入参数指针地址无效CNend
\retval ::EINVAL  22,传入参数无效CNend
\see \n
::FBIOGET_STEREO_MODE
*/
int ioctl (int fd, FBIOPUT_STEREO_MODE, MTFB_STEREO_MODE_E *penStereoMode);


/**
\brief 获取Stereo帧编码格式功能。CNend
\attention \n
CNcomment:请参见::MTFB_ENCODER_PICTURE_FRAMING_E的说明。CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOGET_STEREO_MODE  ioctl number. CNcomment:ioctl号CNend
\param[out] penStereoMode  Pointer to the frame encode format. CNcomment:帧编码格式枚举指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOPUT_ENCODER_PICTURE_FRAMING
*/
int ioctl (int fd, FBIOGET_ENCODER_PICTURE_FRAMING, MTFB_STEREO_MODE_E *penEncPicFrm);


/**
\brief Sets the stereo frame encode format. CNcomment:设置Stereo帧编码格式功能。CNend
\attention \n
For details, see the description of ::MTFB_STEREO_MODE_E. 
CNcomment:请参见::MTFB_ENCODER_PICTURE_FRAMING_E的说明。CNend

\param[in] fd  ID of an FB device. CNcomment:Framebuffer设备号CNend
\param[in] FBIOPUT_STEREO_MODE  ioctl number. CNcomment:ioctl号CNend
\param[in] penStereoMode  Pointer to the frame encode format. CNcomment:帧编码格式枚举指针CNend
\retval  0 SUCCESS Success. CNcomment:成功CNend
\retval ::EPERM  1. The operation is not supported. CNcomment:1,不支持该操作CNend
\retval ::ENOMEM  12. The memory is insufficient. CNcomment:12,内存不够CNend
\retval ::EFAULT  14. The input pointer is invalid. CNcomment:14,传入参数指针地址无效CNend
\retval ::EINVAL  22. The input parameter is invalid. CNcomment:22,传入参数无效CNend
\see \n
::FBIOGET_ENCODER_PICTURE_FRAMING
*/
int ioctl (int fd, FBIOPUT_ENCODER_PICTURE_FRAMING, MTFB_STEREO_MODE_E *penEncPicFrm);


/** @} */  /** <!-- ==== API declaration end ==== */
#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MTFB_H__ */

