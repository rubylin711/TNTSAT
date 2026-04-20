/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
#ifndef __MT_TDE_TYPE_H__
#define __MT_TDE_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include "mt_tde_errcode.h"

/****************************************************************************/
/*                             TDE2 types define                             */
/****************************************************************************/

/*************************** Structure Definition ****************************/
/** \addtogroup      TDE */
/** @{ */  /** <!-- 【 TDE】 */  


/**Definition of the TDE handle*/
/**CNcomment:TDE句柄定义 */
typedef ulong TDE_HANDLE;

/**TDE callback functions*/
/**CNcomment:TDE中断回调函数 */
typedef mt_void (* TDE_FUNC_CB) (mt_void *pParaml, mt_void *pParamr);

typedef mt_void (* TDE_TRIG_SEL)(mt_u32);

/**RGB and packet YUV formats*/
/**CNcomment:RGB和Packet YUV 颜色格式 */
typedef enum mtTDE2_COLOR_FMT_E
{
    TDE2_COLOR_FMT_RGB233 = 0,          /**<RGB233 format*//**<CNcomment:RGB233格式 */
    TDE2_COLOR_FMT_RGB444,              /**<For the RGB444 format, red occupies four bits, green occupies four bits, and blue occupies four bits. Other formats may be deduced by analogy.*//**<CNcomment:RGB444格式, Red占4bits Green占4bits, Blue占4bits,其余格式依此类推 */
    TDE2_COLOR_FMT_BGR444,              /**<BGR444 format*//**<CNcomment:BGR444格式 */    
    TDE2_COLOR_FMT_RGB555,              /**<RGB555 format*//**<CNcomment:RGB555格式 */
    TDE2_COLOR_FMT_BGR555,              /**<BGR555 format*//**<CNcomment:BGR555格式 */
    TDE2_COLOR_FMT_RGB565,              /**<RGB565 format*//**<CNcomment:RGB565格式 */
    TDE2_COLOR_FMT_BGR565,              /**<BGR565 format*//**<CNcomment:BGR565格式 */
    TDE2_COLOR_FMT_RGB888,              /**<ARGB0888 format*//**<CNcomment:ARGB0888格式 */
    TDE2_COLOR_FMT_BGR888,              /**<ABGR0888 format*//**<CNcomment:ABGR0888格式 */
    TDE2_COLOR_FMT_ARGB4444,            /**<ARGB4444 format*//**<CNcomment:ARGB4444格式 */
    TDE2_COLOR_FMT_ABGR4444,            /**<ABGR4444 format*//**<CNcomment:ABGR4444格式 */
    TDE2_COLOR_FMT_RGBA4444,            /**<RGBA4444 format*//**<CNcomment:RGBA4444格式 */
    TDE2_COLOR_FMT_BGRA4444,            /**<BGRA4444 format*//**<CNcomment:BGRA4444格式 */
    TDE2_COLOR_FMT_ARGB1555,            /**<ARGB1555 format*//**<CNcomment:ARGB1555格式 */
    TDE2_COLOR_FMT_ABGR1555,            /**<ABGR1555 format*//**<CNcomment:ABGR1555格式 */
    TDE2_COLOR_FMT_RGBA1555,            /**<RGBA1555 format*//**<CNcomment:RGBA1555格式 */
    TDE2_COLOR_FMT_BGRA1555,            /**<BGRA1555 format*//**<CNcomment:BGRA1555格式 */
    TDE2_COLOR_FMT_ARGB8565,            /**<ARGB8565 format*//**<CNcomment:ARGB8565格式 */
    TDE2_COLOR_FMT_ABGR8565,            /**<ABGR8565 format*//**<CNcomment:ABGR8565格式 */
    TDE2_COLOR_FMT_RGBA8565,            /**<RGBA8565 format*//**<CNcomment:RGBA8565格式 */
    TDE2_COLOR_FMT_BGRA8565,            /**<BGRA8565 format*//**<CNcomment:BGRA8565格式 */
    TDE2_COLOR_FMT_ARGB8888,            /**<ARGB8888 format*//**<CNcomment:ARGB8888格式 */
    TDE2_COLOR_FMT_ABGR8888,            /**<ABGR8888 format*//**<CNcomment:ABGR8888格式 */
    TDE2_COLOR_FMT_RGBA8888,            /**<RGBA8888 format*//**<CNcomment:RGBA8888格式 */
    TDE2_COLOR_FMT_BGRA8888,            /**<BGRA8888 format*//**<CNcomment:BGRA8888格式 */
    TDE2_COLOR_FMT_RABG8888,            /**<RABG8888 format*//**<CNcomment:RABG8888格式 */
    TDE2_COLOR_FMT_CLUT1,               /**<1-bit palette format without alpha component. Each pixel occupies one bit.*//**<CNcomment:无Alpha分量,调色板1bit格式,每个像用1个bit表示 */
    TDE2_COLOR_FMT_CLUT2,               /**<2-bit palette format without alpha component. Each pixel occupies two bits.*//**<CNcomment:无Alpha分量,调色板2bit格式,每个像用2个bit表示 */
    TDE2_COLOR_FMT_CLUT4,               /**<4-bit palette format without alpha component. Each pixel occupies four bits.*//**<CNcomment:无Alpha分量,调色板4bit格式,每个像用4个bit表示 */
    TDE2_COLOR_FMT_CLUT8,               /**<8-bit palette format without alpha component. Each pixel occupies eight bits.*//**<CNcomment:无Alpha分量,调色板8bit格式,每个像用8个bit表示 */
    TDE2_COLOR_FMT_ACLUT44,             /**<1-bit palette format with alpha component. Each pixel occupies one bit.*//**<CNcomment:有Alpha分量,调色板1bit格式,每个像用1个bit表示 */
    TDE2_COLOR_FMT_ACLUT88,             /**<1-bit palette format with alpha component. Each pixel occupies one bit.*//**<CNcomment:有Alpha分量,调色板1bit格式,每个像用1个bit表示 */
    TDE2_COLOR_FMT_A1,                  /**<Alpha format. Each pixel occupies one bit.*//**<CNcomment:alpha格式，每个点用1bit */
    TDE2_COLOR_FMT_A8,                  /**<Alpha format. Each pixel occupies eight bits.*//**<CNcomment:alpha格式，每个点用8bit */
    TDE2_COLOR_FMT_YCbCr888,            /**<YUV packet format without alpha component*//**<CNcomment:YUV packet格式，无alpha分量*/
    TDE2_COLOR_FMT_CbCrY888,            /**<UVY packet format without alpha component*//**<CNcomment:UVY packet格式，无alpha分量*/
    TDE2_COLOR_FMT_AYCbCr8888,          /**<YUV packet format with alpha component*//**<CNcomment:YUV packet格式，有alpha分量*/
    TDE2_COLOR_FMT_CbY0CrY1,            /**<YUV packet422 format*//**<CNcomment:YUV packet422格式 */
    TDE2_COLOR_FMT_YCbCr422,            /**<YUV packet422 format*//**<CNcomment:YUV packet422格式 */
    TDE2_COLOR_FMT_byte,                /**<Only for fast copy*//**<CNcomment:仅用于数据快速copy*/
    TDE2_COLOR_FMT_halfword,            /**<Only for fast copy*//**<CNcomment:仅用于数据快速copy*/
    TDE2_COLOR_FMT_JPG_YCbCr400MBP,     /**<Semi-planar YUV400 format, for JPG decoding*//**<CNcomment:Semi-planar YUV400格式 ,对应于JPG解码*/
    TDE2_COLOR_FMT_JPG_YCbCr422MBHP,    /**<Semi-planar YUV422 format, horizontal sampling, for JPG decoding*//**<CNcomment:Semi-planar YUV422格式,水平方向采样，对应于JPG解码 */
    TDE2_COLOR_FMT_JPG_YCbCr422MBVP,    /**<Semi-planar YUV422 format, vertical sampling, for JPG decoding*//**<CNcomment:Semi-planar YUV422格式,垂直方向采样，对应于JPG解码 */
    TDE2_COLOR_FMT_MP1_YCbCr420MBP,     /**<Semi-planar YUV420 format*//**<CNcomment:Semi-planar YUV420格式 */
    TDE2_COLOR_FMT_MP2_YCbCr420MBP,     /**<Semi-planar YUV420 format*//**<CNcomment:Semi-planar YUV420格式 */
    TDE2_COLOR_FMT_MP2_YCbCr420MBI,     /**<Semi-planar YUV400 format*//**<CNcomment:Semi-planar YUV400格式 */
    TDE2_COLOR_FMT_JPG_YCbCr420MBP,     /**<Semi-planar YUV400 format, for JPG decoding*//**<CNcomment:Semi-planar YUV400格式,对于应于JPG */
    TDE2_COLOR_FMT_JPG_YCbCr444MBP,     /**<Semi-planar YUV444 format*//**<CNcomment:Semi-planar YUV444格式 */
    TDE2_COLOR_FMT_XY,
    TDE2_COLOR_FMT_XYL,
    TDE2_COLOR_FMT_XYC,
    TDE2_COLOR_FMT_XYLC,
    TDE2_COLOR_FMT_TILE,
    TDE2_COLOR_FMT_JPG_SP_CMYK,  			/**<Semi-planar cmyk format*//**<CNcomment:Semi-planar cmky格式 */
    TDE2_COLOR_FMT_RGB24,              /**<RGB888 format*//**<CNcomment:RGB888格式 */
    TDE2_COLOR_FMT_BGR24,              /**<BGR888 format*//**<CNcomment:RGB888格式 */
    TDE2_COLOR_FMT_BUTT                 /**<End of enumeration*//**<CNcomment: 枚举量结束*/
} TDE2_COLOR_FMT_E;

/**Definition of the semi-planar YUV format*/
/**CNcomment:Semi-planar YUV 格式定义 */
typedef enum mtTDE2_MB_COLORFMT_E
{
    TDE2_MB_COLOR_FMT_JPG_YCbCr400MBP = 0,/**<Semi-planar YUV400 format, for JPG decoding*//**<CNcomment:Semi-planar YUV400格式 ,对应于JPG解码*/
    TDE2_MB_COLOR_FMT_JPG_YCbCr422MBHP,   /**<Semi-planar YUV422 format, horizontal sampling, for JPG decoding*//**<CNcomment:Semi-planar YUV422格式,水平方向采样，对应于JPG解码 */
    TDE2_MB_COLOR_FMT_JPG_YCbCr422MBVP,   /**<Semi-planar YUV422 format, vertical sampling, for JPG decoding*//**<CNcomment:Semi-planar YUV422格式,垂直方向采样，对应于JPG解码 */
    TDE2_MB_COLOR_FMT_MP1_YCbCr420MBP,    /**<Semi-planar YUV420 format*//**<CNcomment:Semi-planar YUV420格式 */
    TDE2_MB_COLOR_FMT_MP2_YCbCr420MBP,    /**<Semi-planar YUV420 format*//**<CNcomment:Semi-planar YUV420格式 */
    TDE2_MB_COLOR_FMT_MP2_YCbCr420MBI,    /**<Semi-planar YUV400 format*//**<CNcomment:Semi-planar YUV400格式 */
    TDE2_MB_COLOR_FMT_JPG_YCbCr420MBP,    /**<Semi-planar YUV400 format, for JPG pictures*//**<CNcomment:Semi-planar YUV400格式,对于应于JPG */
    TDE2_MB_COLOR_FMT_JPG_YCbCr444MBP,    /**<Semi-planar YUV444 format, for JPG pictures*//**<CNcomment:Semi-planar YUV444格式,对于应于JPG */
	TDE2_MB_COLOR_FMT_JPG_SP_CMYK,
	TDE2_MB_COLOR_FMT_TILE,   //for fpga
	TDE2_MB_COLOR_FMT_BUTT
} TDE2_MB_COLOR_FMT_E;

/**Structure of the bitmap information set by customers*/
/**CNcomment:用户设置的位图信息结构 */
typedef struct mtTDE2_SURFACE_S
{
    phys_addr_t u32PhyAddr;              /**<Header address of a bitmap or the Y component*//**<CNcomment:位图首地址或Y分量地址 */
	ulong u32VirAddr;
    TDE2_COLOR_FMT_E enColorFmt;    /**<Color format*//**<CNcomment:颜色格式 */

    mt_u32 u32Height;               /**<Bitmap height*//**<CNcomment:位图高度 */

    mt_u32 u32Width;                /**<Bitmap width*//**<CNcomment:位图宽度 */

    mt_u32 u32Stride;               /**<Stride of a bitmap or the Y component*//**<CNcomment:位图跨度或Y分量跨度 */
    phys_addr_t  pu8ClutPhyAddr;          /**<Address of the color look-up table (CLUT), for color extension or color correction*//**<CNcomment:Clut表地址,用作颜色扩展或颜色校正*/
	mt_u8* pu8ClutVirAddr;
    MT_BOOL bYCbCrClut;             /**<Whether the CLUT is in the YCbCr space.*//**<CNcomment:Clut表是否位于YCbCr空间 */

    MT_BOOL bAlphaMax255;           /**<The maximum alpha value of a bitmap is 255 or 128.*//**<CNcomment:位图alpha最大值为255还是128 */

    MT_BOOL bAlphaExt1555;          /*<Whether to enable the alpha extension of an ARGB1555 bitmap.*//**<CNcomment:是否使能1555的Alpha扩展 */
    mt_u8 u8Alpha0;                 /**<Values of alpha0 and alpha1, used as the ARGB1555 format*//**<CNcomment:Alpha0、Alpha1值，用作ARGB1555格式 */
    mt_u8 u8Alpha1;                 /**<Values of alpha0 and alpha1, used as the ARGB1555 format*//**<CNcomment:Alpha0、Alpha1值，用作ARGB1555格式 */
    phys_addr_t u32CbCrPhyAddr;          /**<Address of the CbCr component, pilot*//**<CNcomment:CbCr分量地址,pilot */    
    ulong u32CbCrVirAddr;
    mt_u32 u32CbCrStride;           /**<Stride of the CbCr component, pilot*//**<CNcomment:CbCr分量跨度,pilot */
	
 
} TDE2_SURFACE_S;

/**Definition of the semi-planar YUV data*/
/**CNcomment:Semi-planar YUV格式数据定义 */
typedef struct mtTDE2_MB_S
{
    TDE2_MB_COLOR_FMT_E enMbFmt;        /**<YUV format*//**<CNcomment:YUV格式 */
    phys_addr_t              u32YPhyAddr;    /**<Physical address of the Y component*//**<CNcomment:Y分量物理地址 */
    ulong 				u32YVirAddr;
    mt_u32              u32YWidth;      /**<Width of the Y component*//**<CNcomment:Y分量宽度 */
    mt_u32              u32YHeight;     /**<Height of the Y component*//**<CNcomment:Y分量高度 */
    mt_u32              u32YStride;     /**< Stride of the Y component, indicating bytes in each row*//**<CNcomment:Y分量跨度，每行字节数 */
    phys_addr_t              u32CbCrPhyAddr; /**<Width of the UV component*//**<CNcomment:UV分量宽度 */
    ulong				u32CbCrVirAddr;
    mt_u32              u32CbCrStride;  /**<Stride of the UV component, indicating the bytes in each row*//**<CNcomment:UV分量跨度,每行字节数 */
} TDE2_MB_S;

/**Definition of the TDE rectangle*/
/**CNcomment:TDE矩形定义*/
typedef struct mtTDE2_RECT_S
{
    mt_s32 s32Xpos;     /**<Horizontal coordinate*//**<CNcomment:x坐标 */
    mt_s32 s32Ypos;     /**<Vertical coordinate*//**<CNcomment:y坐标 */
    mt_u32 u32Width;    /**<Width*//**<CNcomment:宽度 */
    mt_u32 u32Height;   /**<Height*//**<CNcomment:高度 */
} TDE2_RECT_S;

/**Logical operation type*/
/**CNcomment:逻辑运算方式 */
typedef enum mtTDE2_ALUCMD_E
{
    TDE2_ALUCMD_NONE = 0x0,         /**<No alpha and raster of operation (ROP) blending*//**<CNcomment:不进行alpha和rop叠加 */    
    TDE2_ALUCMD_BLEND = 0x1,        /**<Alpha blending*//**<CNcomment:Alpha混合*/
    TDE2_ALUCMD_ROP = 0x2,          /**<ROP blending*//**<CNcomment:进行rop叠加 */
    TDE2_ALUCMD_COLORIZE = 0x4,     /**<Colorize operation*//**<CNcomment:进行Colorize操作 */
    TDE2_ALUCMD_BUTT = 0x8          /**<End of enumeration*//**<CNcomment:枚举结束 */
} TDE2_ALUCMD_E;


typedef enum mtTDE2_IMAGE_MULTIPLY_TYPE_E
{
   TDE_IMAGE_MULTIPLY = 0,
   TDE_IMAGE_STENCIL, 
}TDE2_IMAGE_MULTIPLY_TYPE_E;


/**Definition of ROP codes*/
/**CNcomment:ROP操作码定义 */
typedef enum mtTDE2_ROP_CODE_E
{
    TDE2_ROP_BLACK = 0,     /**<Blackness*/
    TDE2_ROP_NOTMERGEPEN,   /**<~(S2 | S1)*/
    TDE2_ROP_MASKNOTPEN,    /**<~S2&S1*/
    TDE2_ROP_NOTCOPYPEN,    /**< ~S2*/
    TDE2_ROP_MASKPENNOT,    /**< S2&~S1 */
    TDE2_ROP_NOT,           /**< ~S1 */
    TDE2_ROP_XORPEN,        /**< S2^S1 */
    TDE2_ROP_NOTMASKPEN,    /**< ~(S2 & S1) */
    TDE2_ROP_MASKPEN,       /**< S2&S1 */
    TDE2_ROP_NOTXORPEN,     /**< ~(S2^S1) */
    TDE2_ROP_NOP,           /**< S1 */
    TDE2_ROP_MERGENOTPEN,   /**< ~S2|S1 */
    TDE2_ROP_COPYPEN,       /**< S2 */
    TDE2_ROP_MERGEPENNOT,   /**< S2|~S1 */
    TDE2_ROP_MERGEPEN,      /**< S2|S1 */
    TDE2_ROP_WHITE,         /**< Wmtteness */

     //  aria added 
    TDE2_ROP_PATINVERT,  /*p ^ d*/
    TDE2_ROP_MERGEPAINT, /*s & p*/
    TDE2_ROP_PATCOPY, 	/*p*/
    TDE2_ROP_PATPAINT,  /*~s | p | d*/

    TDE2_ROP_BUTT
} TDE2_ROP_CODE_E;

/**Definition of the blit mirror*/
/**CNcomment:blit镜像定义 */
typedef enum mtTDE2_MIRROR_E
{
    TDE2_MIRROR_NONE = 0,       /**<No mirror*//**<CNcomment:不进行镜像 */
    TDE2_MIRROR_HORIZONTAL,     /**<Horizontal mirror*//**<CNcomment:水平镜像 */
    TDE2_MIRROR_VERTICAL,       /**<Vertical mirror*//**<CNcomment:垂直镜像 */
    TDE2_MIRROR_BOTH,           /**<Horizontal and vertical mirror*//**<CNcomment:垂直和水平镜像 */
    TDE2_MIRROR_BUTT
} TDE2_MIRROR_E;

typedef enum mtTDE2_ROTATE_E
{
    TDE2_ROTATE_NONE = 0,
    TDE2_ROTATE_90,     /**<Rotate 90 degrees clockwise*//**<CNcomment: 顺时针旋转90度 */
    TDE2_ROTATE_180,    /**<Rotate 180 degrees clockwise*//**<CNcomment: 顺时针旋转180度 */
    TDE2_ROTATE_270,    /**<Rotate 270 degrees clockwise*//**<CNcomment: 顺时针旋转270度 */

    TDE2_ROTATE_BUTT
} TDE2_ROTATE_E;


/**Clip operation type*/
/**CNcomment:Clip操作类型*/
typedef enum mtTDE2_CLIPMODE_E
{
    TDE2_CLIPMODE_NONE = 0, /**<No clip*//**<CNcomment:无clip操作 */
    TDE2_CLIPMODE_INSIDE,   /**<Clip the data within the rectangle to output and discard others*//**<CNcomment:剪切矩形范围内的数据输出,其余扔掉*/
    TDE2_CLIPMODE_OUTSIDE,  /**<Clip the data outside the rectangle to output and discard others*//**<CNcomment:剪切矩形范围外的数据输出，其余扔掉*/
    TDE2_CLIPMODE_BUTT
} TDE2_CLIPMODE_E;

/**Scaling mode for the macroblock*/
/**CNcomment:宏块格式缩放类型*/
typedef enum mtTDE2_MBRESIZE_E
{
    TDE2_MBRESIZE_NONE = 0,         /**<No scaling*//**<CNcomment:不做缩放 */
    TDE2_MBRESIZE_QUALITY_LOW,      /**<Low-quality scaling*//**<CNcomment:低质量缩放 */
    TDE2_MBRESIZE_QUALITY_MIDDLE,   /**<Medium-quality scaling*//**<CNcomment:中质量缩放 */
    TDE2_MBRESIZE_QUALITY_HIGH,     /**<High-quality scaling*//**<CNcomment:高质量缩放 */
    TDE2_MBRESIZE_BUTT
} TDE2_MBRESIZE_E;

/**Definition of fill colors*/
/**CNcomment:填充色定义 */
typedef struct mtTDE2_FILLCOLOR_S
{
    TDE2_COLOR_FMT_E enColorFmt;    /**<TDE pixel format*//**<CNcomment:TDE像素格式 */
    mt_u32           u32FillColor;  /**<Fill colors that vary according to pixel formats*//**<CNcomment:填充颜色，根据像素格式而不同 */
} TDE2_FILLCOLOR_S;

/**Definition of colorkey modes*/
/**CNcomment:colorkey选择方向定义 */
typedef enum mtTDE2_COLORKEY_MODE_E
{
    TDE2_COLORKEY_MODE_NONE = 0,     /**<No colorkey*//**<CNcomment:不做color key */
    TDE2_COLORKEY_MODE_FOREGROUND,   /**<When performing the colorkey operation on the foreground bitmap, you need to perform this operation before the CLUT for color extension and perform this operation after the CLUT for color correction.*//**<CNcomment:对前景位图进行color key，说明:对于颜色扩展，在CLUT前做color key；对于颜色校正:在CLUT后做color key */
    TDE2_COLORKEY_MODE_BACKGROUND,   /**<Perform the colorkey operation on the background bitmap*//**<CNcomment:对背景位图进行color key*/
    TDE2_COLORKEY_MODE_EX,			/*src3带colorkey*/
    TDE2_COLORKEY_MODE_CONFIG,			/*colorkey同时来自以上两个或三个，由stColorKeyCfg配置*/
    TDE2_COLORKEY_MODE_BUTT
} TDE2_COLORKEY_MODE_E;


typedef enum mtTDE2_COLORKEY_SELECT_E
{
    TDE2_MASK_KEY_MATCH = 0,
    TDE2_MASK_KEY_MISMATCH = 1
}TDE2_COLORKEY_SELECT_E;

typedef enum mtTDE2_COLOEKEY_RANGE_E
{
    TDE2_KEY_MATCH_NONE = 0,
    //MIN <=C <= MAX
    TDE2_KEY_MATCH_INSIDE_MIN_MAX = 1,
    //C <MIN  || C > MAX
    TDE2_KEY_MATCH_OUTSIDE_MIN_MAX = 2,
    TDE2_KEY_MATCH_ALL = 3
}TDE2_COLOEKEY_RANGE_E;

/**Definition of colorkey range*/
/**CNcomment:colorkey范围定义 */
typedef struct mtTDE2_COLORKEY_COMP_S
{
    mt_u8 u8CompMin;           /**<Minimum value of a component*//**<CNcomment:分量最小值*/
    mt_u8 u8CompMax;           /**<Maximum value of a component*//**<CNcomment:分量最大值*/
    mt_u8 bCompOut;            /**<The colorkey of a component is within or beyond the range.*//**<CNcomment:分量关键色在范围内/范围外*/
    mt_u8 bCompIgnore;         /**<Whether to ignore a component.*//**<CNcomment:分量是否忽略*/
    mt_u8 u8CompMask;          /**<Component mask*//**<CNcomment:分量掩码*/
    mt_u8 u8Reserved;
    mt_u8 u8Reserved1;
    mt_u8 u8Reserved2;
} TDE2_COLORKEY_COMP_S;

/**Definition of colorkey values*/
/**CNcomment:colorkey值定义 */
typedef union mtTDE2_COLORKEY_U
{
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;   /**<Alpha component*//**<CNcomment:alpha 分量信息 */
        TDE2_COLORKEY_COMP_S stRed;     /**<Red component*//**<CNcomment:红色分量信息 */
        TDE2_COLORKEY_COMP_S stGreen;   /**<Green component*//**<CNcomment:绿色分量信息 */
        TDE2_COLORKEY_COMP_S stBlue;    /**<Blue component*//**<CNcomment:蓝色分量信息 */
    } struCkARGB;
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;   /**<Alpha component*//**<CNcomment:alpha 分量信息 */
        TDE2_COLORKEY_COMP_S stY;       /**<Y component*//**<CNcomment:Y分量信息 */
        TDE2_COLORKEY_COMP_S stCb;      /**<Cb component*//**<CNcomment:Cb分量信息 */
        TDE2_COLORKEY_COMP_S stCr;      /**<Cr component*//**<CNcomment:Cr分量信息 */
    } struCkYCbCr;
    struct
    {
        TDE2_COLORKEY_COMP_S stAlpha;   /**<Alpha component*//**<CNcomment:alpha 分量信息 */
        TDE2_COLORKEY_COMP_S stClut;    /**<Palette component*//**<CNcomment:调色板分量信息 */
    } struCkClut;
} TDE2_COLORKEY_U;

/*Definition of alpha output sources*/
/**CNcomment:输出alpha定义 */
typedef enum mtTDE2_OUTALPHA_FROM_E
{
    TDE2_OUTALPHA_FROM_NORM = 0,    /**<Output from the result of alpha blending or anti-flicker*//**<CNcomment:来源于alpha blending的结果或者抗闪烁的结果 */
    TDE2_OUTALPHA_FROM_BACKGROUND,  /**<Output from the background bitmap*//**<CNcomment:来源于背景位图 */
    TDE2_OUTALPHA_FROM_FOREGROUND,  /**<Output from the foreground bitmap*//**<CNcomment:来源于前景位图 */
    TDE2_OUTALPHA_FROM_GLOBALALPHA, /**<Output from the global alpha*//**<CNcomment:来源于全局alpha */
    TDE2_OUTALPHA_FROM_BUTT
} TDE2_OUTALPHA_FROM_E;

/**Definition of filtering*/
/**CNcomment:缩放滤波定义 */
typedef enum mtTDE2_FILTER_MODE_E
{
    TDE2_FILTER_MODE_COLOR = 0, /**<Filter the color*//**<CNcomment:对颜色进行滤波 */
    TDE2_FILTER_MODE_ALPHA,     /**<Filter the alpha channel*//**<CNcomment:对alpha通道滤波 */
    TDE2_FILTER_MODE_BOTH,      /**<Filter the color and alpha channel*//**<CNcomment:对颜色和alpha通道同时滤波 */
    TDE2_FILTER_MODE_NONE,      /**<No filter *//**<CNcomment:不进行滤波 */
    TDE2_FILTER_MODE_BUTT
} TDE2_FILTER_MODE_E;

/**Configuration of the anti-flicker channel*/
/**CNcomment:抗闪烁处理通道配置 */
typedef enum mtTDE2_DEFLICKER_MODE_E
{
    TDE2_DEFLICKER_MODE_NONE = 0,   /*<No anti-flicker*//**<CNcomment:不做抗闪 */
    TDE2_DEFLICKER_MODE_RGB,        /**<Perform anti-flicker on the RGB component*//**<CNcomment:RGB分量抗闪 */
    TDE2_DEFLICKER_MODE_BOTH,       /**<Perform anti-flicker on the alpha component*//**<CNcomment:alpha分量抗闪 */
    TDE2_DEFLICKER_MODE_BUTT
}TDE2_DEFLICKER_MODE_E;

/* blend mode */
typedef enum mtTDE2_BLEND_MODE_E
{
    TDE2_BLEND_ZERO = 0x0,
    TDE2_BLEND_ONE,
    TDE2_BLEND_SRC2COLOR,
    TDE2_BLEND_INVSRC2COLOR,
    TDE2_BLEND_SRC2ALPHA,
    TDE2_BLEND_INVSRC2ALPHA,
    TDE2_BLEND_SRC1COLOR,
    TDE2_BLEND_INVSRC1COLOR,
    TDE2_BLEND_SRC1ALPHA,
    TDE2_BLEND_INVSRC1ALPHA,
    TDE2_BLEND_SRC2ALPHASAT,
    TDE2_BLEND_BUTT
}TDE2_BLEND_MODE_E;

/**Alpha blending command. You can set parameters or select Porter or Duff.*/
/**CNcomment:alpha混合命令,可以选择自己配置参数，也可以选择Porter/Duff中的一种 */
/* pixel = (source * fs + destination * fd),
   sa = source alpha,
   da = destination alpha */
typedef enum mtTDE2_BLENDCMD_E
{
    TDE2_BLENDCMD_NONE = 0x0,     /**< fs: sa      fd: 1.0-sa */
    TDE2_BLENDCMD_CLEAR,    /**< fs: 0.0     fd: 0.0 */
    TDE2_BLENDCMD_SRC,      /**< fs: 1.0     fd: 0.0 */
    TDE2_BLENDCMD_SRCOVER,  /**< fs: 1.0     fd: 1.0-sa */
    TDE2_BLENDCMD_DSTOVER,  /**< fs: 1.0-da  fd: 1.0 */
    TDE2_BLENDCMD_SRCIN,    /**< fs: da      fd: 0.0 */
    TDE2_BLENDCMD_DSTIN,    /**< fs: 0.0     fd: sa */
    TDE2_BLENDCMD_SRCOUT,   /**< fs: 1.0-da  fd: 0.0 */
    TDE2_BLENDCMD_DSTOUT,   /**< fs: 0.0     fd: 1.0-sa */
    TDE2_BLENDCMD_SRCATOP,  /**< fs: da      fd: 1.0-sa */
    TDE2_BLENDCMD_DSTATOP,  /**< fs: 1.0-da  fd: sa */
    TDE2_BLENDCMD_ADD,      /**< fs: 1.0     fd: 1.0 */
    TDE2_BLENDCMD_XOR,      /**< fs: 1.0-da  fd: 1.0-sa */
    TDE2_BLENDCMD_DST,      /**< fs: 0.0     fd: 1.0 */
    TDE2_BLENDCMD_CONFIG,   /**<You can set the parameteres.*//**<CNcomment:用户自己配置参数*/
    TDE2_BLENDCMD_BUTT
}TDE2_BLENDCMD_E;


typedef enum meTDE2_EDGE_OPT_E
{
    EDGE_CFG_AUTO_EXP_COPY = 0,   //  auto exp and copy
    EDGE_CFG_AUTO_EXP_FILL = 1,    // auto exp and fill 0
    EDGE_CFG_NOEXP_COPY = 2,         // no exp and only copy
}TDE2_EDGE_OPT_E;

/**Options for the alpha blending operation*/
/**CNcomment:alpha混合操作选项 */
typedef struct mtTDE2_BLEND_OPT_S
{
    MT_BOOL  bGlobalAlphaEnable;        /**<Global alpha enable*//**<CNcomment:是否使能全局alpha */
    MT_BOOL  bPixelAlphaEnable;         /**<not used, Pixel alpha enable*//**<CNcomment:是否使能象素alpha */
    MT_BOOL bSrc1AlphaPremulti;         /**<not support, Src1(dst) alpha premultiply enable*//**<CNcomment:是否使能Src1 alpha预乘 */
    MT_BOOL bSrc2AlphaPremulti;         /**<Src2(src) alpha premultiply enable*//**<CNcomment:是否使能Src2 alpha预乘 */
    MT_BOOL bBlendModeAlphaEnable;         /**<It is valid when eBlendCmd is set to TDE2_BLENDCMD_CONFIG，set blend mode for color and alpha respectively*//**<CNcomment:在eBlendCmd = TDE2_BLENDCMD_CONFIG时，单独分开配置alpha和color的模式*/
    TDE2_BLENDCMD_E eBlendCmd;          /**<Alpha blending command*//**<CNcomment:alpha混合命令*/    
    TDE2_BLEND_MODE_E eSrc1BlendMode;   /**<Src1(dst) blending mode select. It is valid when eBlendCmd is set to TDE2_BLENDCMD_CONFIG.*//**<CNcomment:Src1 blend模式选择,在eBlendCmd = TDE2_BLENDCMD_CONFIG时有效 */
    TDE2_BLEND_MODE_E eSrc2BlendMode;   /**<Src2(src) blending mode select. It is valid when eBlendCmd is set to TDE2_BLENDCMD_CONFIG.*//**<CNcomment:Src2 blend模式选择,在eBlendCmd = TDE2_BLENDCMD_CONFIG时有效 */
    TDE2_BLEND_MODE_E eSrc1BlendModeAlpha;   /**<Src1(dst) blending mode select for alpha. It is valid when eBlendCmd is set to TDE2_BLENDCMD_CONFIG and bBlendModeAlphaEnable is true.*//**<CNcomment:alpha的Src1 blend模式选择,在eBlendCmd = TDE2_BLENDCMD_CONFIG和bBlendModeAlphaEnable为true时有效 */
    TDE2_BLEND_MODE_E eSrc2BlendModeAlpha;   /**<Src2(src) blending mode select for alpha. It is valid when eBlendCmd is set to TDE2_BLENDCMD_CONFIG and bBlendModeAlphaEnable is true.*//**<CNcomment:alpha的Src2 blend模式选择,在eBlendCmd = TDE2_BLENDCMD_CONFIG和bBlendModeAlphaEnable为true时有效 */
    MT_BOOL demultiply_en;
}TDE2_BLEND_OPT_S;

/**CSC parameter option*/
/**CNcomment:CSC参数选项*/
typedef struct mtTDE2_CSC_OPT_S
{
    MT_BOOL bICSCUserEnable;		/**User-defined ICSC parameter enable*//**CNcomment:用户自定义ICSC参数使能*/
    MT_BOOL bICSCParamReload;	/**User-defined ICSC parameter reload enable*//**CNcomment:重新加载用户自定义ICSC参数使能*/
    MT_BOOL bOCSCUserEnable;		/**User-defined OCSC parameter enable*//**CNcomment:用户自定义OCSC参数使能*/
    MT_BOOL bOCSCParamReload;	/**User-defined OCSC parameter reload enable*//**CNcomment:重新加载用户自定义OCSC参数使能*/
    mt_u32 u32ICSCParamAddr;		/**ICSC parameter address. The address must be 128-bit aligned.*//**CNcomment:ICSC参数地址，要求128bit对齐*/
    mt_u32 u32OCSCParamAddr;	/**OCSC parameter address. The address must be 128-bit aligned.*//**CNcomment:OCSC参数地址，要求128bit对齐*/
}TDE2_CSC_OPT_S;


/// tde gaussian tap,  0 --- 26
#define  TDE2_MIN_GS_BLUR_LEVEL     (0)
#define  TDE2_MAX_GS_BLUR_LEVEL    (26)

typedef struct mtTDE2_GS_BLUR_S      //  gaussian blur
{
   mt_u8                        blur_level;                     // 0 -- 26,     the blur number =   (blur_level * 2 + 3) 
   TDE2_EDGE_OPT_E    edge_opt; 
}TDE2_GS_BLUR_S;


/*!
indicate concerto rotator operation
*/
typedef enum mtTDE2_ROTATOR_TYPE_S
{
    ROTATOR_COPY_OP = 0,
    ROTATOR_HORI_MIRROR = 1,
    ROTATOR_VERT_MIRROR = 2,
    ROTATOR_HORI_VERT_MIRROR = 3,
    ROTATOR_TRANS = 4,    
    ROTATOR_HORI_MIRROR_TRANS = 5,    
    ROTATOR_VERT_MIRROR_TRANS = 6,    
    ROTATOR_HORI_VERT_MIRROR_TRANS = 7,
 }TDE2_ROTATOR_TYPE_S;

typedef struct mtTDE2_ROTATOR_S      //  gaussian blur
{
   TDE2_ROTATOR_TYPE_S        rotator_type;                    
}TDE2_ROTATOR_S;

typedef enum mtTDE2_MASK_OPT_E
{
  TDE2_CLEAR_MASK,//!< D = 0
  TDE2_FILL_MASK, //!< D = 1
  TDE2_SET_MASK, //!< D = S
  TDE2_UNION_MASK,//!< D = 1 - (1 - S) * (1 - D)
  TDE2_INTERSECT_MASK,//!< D = S * D
  TDE2_SUBTRACT_MASK, //!< D = D * (1 - S)

  TDE2_MASK_BUTT,//!< BUTT
} TDE2_MASK_OPT_E;


typedef struct mtTDE2_POS_S
{
  /*!
    X pos
    */
  mt_u32 x;
  /*!
    Y pos
    */
  mt_u32 y;
} TDE2_POS_S;

typedef struct mtTDE2_POS_GROUP_S
{
    /*!
      top left point
      */
    TDE2_POS_S pos00;
    /*!
      top right point
      */    
    TDE2_POS_S pos10; 
    /*!
      bottom left point
      */    
    TDE2_POS_S pos01; 
    /*!
      bottom right point
      */    
    TDE2_POS_S pos11;
}TDE2_POS_GROUP_S;

/*!
@~english Paint mode
@~chinese Paint操作模式
*/
typedef enum mtTDE2_PAINT_TYPE_S
{
    /*!
    @~english flat color paint, no input src data in this mode, the generated data can be used as src to do rop or blending operation with dst
    @~chinese 该模式没有src，产生的纯色图片可当成src和dst做rop或blending叠加
    */
   TDE2_PAINT_TYPE_COLOR,
    /*!
    @~english linear gradient paint, no input src data in this mode, the generated data can be used as src to do rop or blending operation with dst
    @~chinese 该模式没有src，产生的线性渐变图片可当成src和dst做rop或blending叠加
    */
    TDE2_PAINT_TYPE_LINEAR_GRADIENT,
    /*!
    @~english radial gradient paint, no input src data in this mode, the generated data can be used as src to do rop or blending operation with dst
    @~chinese 该模式没有src，产生的放射性渐变图片可当成src和dst做rop或blending叠加
    */
    TDE2_PAINT_TYPE_RADIAL_GRADIENT,
    /*!
    @~english ellipse gradient paint, no input src data in this mode, the generated data can be used as src to do rop or blending operation with dst
    @~chinese 该模式没有src，产生的椭圆形放射性渐变图片可当成src和dst做rop或blending叠加
    */    
    TDE2_PAINT_TYPE_ELLIPSE_GRADIENT,    
    /*!
    @~english pattern paint based on an input src image,  the generated data will be copyed to dst directly
    @~chinese 该模式有src，生成的新图片会直接被硬件拷贝到dst
    */
    TDE2_PAINT_TYPE_PATTERN,

    /*!
    BUTT
    */
    TDE2_PAINT_TYPE_BUTT
} TDE2_PAINT_TYPE_S;


/*!
@~english
@brief Tiling mode for pattern paint
@details The tiling mode defines possible methods for defining colors for source pixels that lie outside the bounds of the source image

@~chinese
@brief Pattern paint的tiling模式
@details 定义了源图以外区域像素值的生成方式
*/
typedef enum mtTDE2_TILE_S
{
    /*!
    @~english The pixel outside the bounds of source image will be taken as a constant ARGB8888 color setted by user
    @~chinese 源图以外区域像素值填充成用户设置的某个ARGB8888色彩值
    */
    TDE2_TILE_FILL,
     /*!
    @~english The pixel outside the bounds of source image will be taken as the edge pixel of the source image
    @~chinese 源图以外区域像素值填充成源图的边界值
    */
    TDE2_TILE_PAD,
    /*!
    @~english The source image will be repeated to fill the dst region
    @~chinese 源图不断复制来填充目标区域
    */
    TDE2_TILE_REPEAT,
    /*!
    @~english The source image will be reflected to fill the dst region
    @~chinese 源图不断反射性复制来填充目标区域
    */
    TDE2_TILE_REFLECT,
    /*!
    BUTT
    */
    TDE2_TILE_BUTT
} TDE2_TILE_S;

/*!
@~english Spread modes for liner gradient paint and radial gradient paint
@~chinese 线性渐变和放射性渐变的展开模式
*/
typedef enum mtTDE2_SPREAD_S
{
    COLOR_RAMP_SPREAD_PAD, //!<extend stops
    COLOR_RAMP_SPREAD_REPEAT, //!<repeat stops
    COLOR_RAMP_SPREAD_REFLECT, //!<repeat stops in reflected order
    COLOR_RAMP_SPREAD_BUTT, //!<BUTT
} TDE2_SPREAD_S;

/*!
  comments
  */
  typedef enum mtTDE2_GMASK_MODE_t
  {
    GRADIENT_MASK_0,
    GRADIENT_MASK_1,
    GRADIENT_MASK_2,

    GRADIENT_MASK_MODE_FORCE_SIZE,
  } TDE2_GMASK_MODE_t;

/*!
  comments
  */
  typedef struct mtTDE2_GSTOP_S
  {
    /*!
      comments
      */
    mt_u32 argb;
    /*!
      offset= offset_org <<12,offset_org: 0~1
      */
    mt_u32 offset;
  }TDE2_GSTOP_S;


/*!
  comments
  */
  typedef struct mtTDE2_PAINT_LGRADT_S
  {
    /*!
      comments
      */
    TDE2_POS_S begin;
    /*!
      comments
      */
    TDE2_POS_S end;
    /*!
      comments
      */
    TDE2_GSTOP_S stop0;
    /*!
      comments
      */
    TDE2_GSTOP_S stop1;
    /*!
      comments
      */
    TDE2_GSTOP_S stop2;
    /*!
      comments
      */
    TDE2_GSTOP_S stop3;
    /*!
      VGColorRampSpreadMode
      */
    TDE2_SPREAD_S spread_mod;
    /*!
      comments
      */
    TDE2_GMASK_MODE_t mask_mod;
  }TDE2_PAINT_LGRADT_S;
/*!
  comments
  */
  typedef struct mtTDE2_PAINT_RGRADT_S
  {
    /*!
      comments
      */
    TDE2_POS_S center;
    /*!
      comments
      */
    TDE2_POS_S focus;
    /*!
      comments
      */
    mt_u32 radius;
    /*!
      comments
      */
    TDE2_GSTOP_S stop0;
    /*!
      comments
      */
    TDE2_GSTOP_S stop1;
    /*!
      comments
      */
    TDE2_GSTOP_S stop2;
    /*!
      comments
      */
    TDE2_GSTOP_S stop3;
    /*!
      VGColorRampSpreadMode
      */
    TDE2_SPREAD_S spread_mod;
    /*!
      comments
      */
    TDE2_GMASK_MODE_t mask_mod;
  }TDE2_PAINT_RGRADT_S;
/*!
  comments
  */
  typedef struct mtTDE2_PAINT_EGRADT_S
  {
    /*!
      comments
      */
    TDE2_POS_S center;
    /*!
      comments
      */
    TDE2_POS_S focus;
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
    TDE2_GSTOP_S stop0;
    /*!
      comments
      */
    TDE2_GSTOP_S stop1;
    /*!
      comments
      */
    TDE2_GSTOP_S stop2;
    /*!
      comments
      */
    TDE2_GSTOP_S stop3;
    /*!
      VGColorRampSpreadMode
      */
    TDE2_SPREAD_S spread_mod;
    /*!
      comments
      */
    TDE2_GMASK_MODE_t mask_mod;
    /*!
      comments
      */
    mt_u32 stop_num;
  }TDE2_PAINT_EGRADT_S;
/*!
  comments
  */
  typedef struct mtTDE2_PAINT_PATTERN_S
  {
    /*!
      comments
      */
    TDE2_POS_S pat_beg;
    /*!
      comments
      */
    mt_u32 fill_color;
    /*!
      VGTilingMode
      */
    TDE2_TILE_S tiling_mod;
  }TDE2_PAINT_PATTERN_S;

  typedef struct mtTDE2_PAINT_CFG_S
  {
    /*!
      VGPaintType
      */
    TDE2_PAINT_TYPE_S paint_type;
    /*!
      paint color config
      */
    mt_u32 paint_color;
    /*!
      paint liner gradient config
      */
    TDE2_PAINT_LGRADT_S paint_liner_gradt;
    /*!
      paint radial gradient config
      */
    TDE2_PAINT_RGRADT_S paint_radial_gradt;
    /*!
      paint ellipse gradient config
      */
    TDE2_PAINT_EGRADT_S paint_ellipse_gradt;

    /*!
      paint pattern config
      */
    TDE2_PAINT_PATTERN_S paint_pattern;

  }TDE2_PAINT_CFG_S;

typedef struct mtTDE2_COLORKEY_CFG_S
{
	MT_BOOL enColorKey_fg;
	MT_BOOL enColorKey_bg;
	MT_BOOL enColorKey_ex;
	TDE2_COLORKEY_SELECT_E enColorKeySelect_fg;
	TDE2_COLORKEY_SELECT_E enColorKeySelect_bg;
	TDE2_COLORKEY_SELECT_E enColorKeySelect_ex;
	TDE2_COLORKEY_U unColorKeyValue_fg;
	TDE2_COLORKEY_U unColorKeyValue_bg;
	TDE2_COLORKEY_U unColorKeyValue_ex;
}TDE2_COLORKEY_CFG_S;

typedef struct mtTDE2_SURFACE_CFG_S
{
	MT_BOOL enSrcGlobalAlpha;
	MT_BOOL enExGlobalAlpha;
    MT_BOOL enDstGlobalAlpha;    
    MT_BOOL enDstPreMult;
	MT_BOOL enSrcPreMult;
	MT_BOOL enExPreMult;
	MT_BOOL disSrcAlpha;
	MT_BOOL disExAlpha;
	MT_BOOL disDstAlpha;	
	MT_BOOL enSrcFlip;
	MT_BOOL enDstFlip;
	MT_BOOL enExFlip;
}TDE2_SURFACE_CFG_S;
typedef enum
{
    CLIP_MODE_INSIDE = 0,
    CLIP_MODE_OUTSIDE = 1
}clip_mode_t;
typedef struct
{
    clip_mode_t clip_mode;
    TDE2_RECT_S  clip_rect; 
}clip_cfg_t;  


/**Definition of blit operation options*/
/**CNcomment:blit操作选项定义 */
typedef struct mtTDE2_OPT_S
{
    TDE2_ALUCMD_E enAluCmd;                 /**<Logical operation type*//**<CNcomment:逻辑运算类型*/

    TDE2_ROP_CODE_E enRopCode_Color;        /**<ROP type of the color space*//**<CNcomment:颜色空间ROP类型*/

    TDE2_ROP_CODE_E enRopCode_Alpha;        /**<ROP type of the alpha component*//**<CNcomment:Alpha的ROP类型*/

    TDE2_COLORKEY_MODE_E enColorKeyMode;    /**<Colorkey mode*//**<CNcomment:color key方式*/
	
	TDE2_COLORKEY_SELECT_E enColorKeySelect; /*关键色匹配反转，满足匹配条件的做colorkey或不满足匹配条件的做colorkey*/

    TDE2_COLORKEY_U unColorKeyValue;        /**<Colorkey value*//**<CNcomment:color key设置值*/

	TDE2_COLORKEY_CFG_S stColorKeyCfg;		/*当有多个colorkey时，用这个配置*/

    TDE2_CLIPMODE_E enClipMode;             /**<Perform the clip operation within or beyond the area*//**<CNcomment:区域内作clip还是区域外作clip*/

    TDE2_RECT_S stClipRect;                 /**<Definition of the clipping area*//**<CNcomment:clip区域定义*/

    TDE2_DEFLICKER_MODE_E enDeflickerMode;  /**<Anti-flicker mode*//**<CNcomment:抗闪烁模式*/

    MT_BOOL bResize;                        /**<Whether to scale*//**<CNcomment:是否缩放*/

    TDE2_FILTER_MODE_E enFilterMode;        /**<Filtering mode during scaling*//**< CNcomment:缩放时使用的滤波模式 */

    TDE2_MIRROR_E enMirror;                 /**<Mirror type*//**<CNcomment:镜像类型*/

    MT_BOOL bClutReload;                    /**<Whether to reload the CLUT*//**<CNcomment:是否重新加载Clut表*/

    mt_u8   u8GlobalAlpha;                  /**<(src) Global alpha value*//**<CNcomment:src surface 全局Alpha值*/

    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;    /**<Source of the output alpha*//**<CNcomment:输出alpha来源*/
    
    mt_u32 u32Colorize;                     /**<Colorize value*//**<CNcomment:Colorize值 */

    TDE2_BLEND_OPT_S stBlendOpt;

    TDE2_CSC_OPT_S stCscOpt;	

    MT_BOOL  enGsBlur;             /* enable gaussian blur */
    
    TDE2_GS_BLUR_S  stBlurOpt;  /* gaussian blur  option   */

    MT_BOOL enPaint;	/*enable paint*/
	TDE2_PAINT_CFG_S stPaintOpt;

	TDE2_ROTATE_E enRotator;

    MT_BOOL  enMultiply;
    
    MT_BOOL  enStencil;
    /*!
    dst = Aex ? ROP/BLD(src, dst):dst, \n
    @~english ex is a gray-8 picture, and only the bit0 is valid, if bit0 is equal 1, then the operation result between src and dst will be the output for dst; 
    otherwise, dst will keep it's origial value
    @~chinese 源3为8位灰度图，只有最低位有效，即用8位表示的二值图,当源3的最低位为1时，src和dst
    的叠加结果将被输出到dst；否则dst保持不变
    */
	MT_BOOL enAMapLogical;
    /*!
    [ARGB]dst = [ARGB]src.*[AAAA]ex \n
    @~english ex is a gray-8 picture, it will used to change the A,R,G,B components of the src picture, and then the new generated picture will be used as src to do 
    rop or blending operation with dst
    @~chinese 源3为8位灰度图，它先和源的A,R,G,B分量分别相乘，得到的结果再作为src与dst做叠加操作
    */
	MT_BOOL enAMapMix;
    /*!
    [ARGB]dst = [ARGB]src.*[A111]ex \n
    @~english ex is a gray-8 picture, it will used to change the A component of the src picture, and then the new generated picture will be used as src to do 
    rop or blending operation with dst
    @~chinese 源3为8位灰度图，它先和源的A分量相乘，得到的结果再作为src与dst做叠加操作
    */

	MT_BOOL enAMapMixEx;

    MT_BOOL b3dResize;
    TDE2_POS_GROUP_S Scale3dDst;

    mt_u32 xylc_cmd_num;
    mt_u32 xylc_color;

	mt_u8 u8ExGlobalAlpha;			/*ex surface的global alpha*/
	
	TDE2_SURFACE_CFG_S stSurfaceCfg;/*surfaceCfg作用于全部情况，不仅仅在blend时起作用*/
    MT_BOOL enableClip;
    clip_cfg_t clipCfg;
    MT_BOOL enableColorize;
    mt_u32 color;
    mt_u8   u8DstGlobalAlpha; //dst surface alpha
} TDE2_OPT_S;

/**Definition of macroblock operation options*/
/**CNcomment:宏块操作选项定义 */
typedef struct mtTDE2_MBOPT_S
{
    TDE2_CLIPMODE_E enClipMode;     /**<Clip mode*//**<CNcomment:Clip模式选择*/

    TDE2_RECT_S stClipRect;         /**<Definition of the clipping area*//**<CNcomment:clip区域定义*/

    MT_BOOL bDeflicker;             /**<Whether to perform anti-flicker*//**<CNcomment:是否抗闪烁*/

    TDE2_MBRESIZE_E enResize;       /**<Scaling information*//**<CNcomment:缩放信息*/

    MT_BOOL bSetOutAlpha;           /**<If the alpha value is not set, the maximum alpha value is output by default.*//**<CNcomment:如果不设置Alpha,则默认输出最大Alpha值*/
    
    mt_u8   u8OutAlpha;             /**<Global alpha for operation*//**<CNcomment:参加运算的全局alpha */
#ifdef CONFIG_MT_FPGA_GPE
    TDE2_COLORKEY_MODE_E enColorKeyMode;    /**<Colorkey mode*//**<CNcomment:color key方式*/

    TDE2_COLORKEY_U unColorKeyValue;        /**<Colorkey value*//**<CNcomment:color key值*/
#endif	
} TDE2_MBOPT_S;

/**Definition of the pattern filling operation*/
/**CNcomment:模式填充操作信息定义 */
typedef struct mtTDE2_PATTERN_FILL_OPT_S
{
    TDE2_ALUCMD_E enAluCmd;                 /**<Logical operation type*//**<CNcomment:逻辑运算类型*/

    TDE2_ROP_CODE_E enRopCode_Color;        /**<ROP type of the color space*//**<CNcomment:颜色空间ROP类型*/

    TDE2_ROP_CODE_E enRopCode_Alpha;        /**<ROP type of the alpha component*//**<CNcomment:alpha的ROP类型*/

    TDE2_COLORKEY_MODE_E enColorKeyMode;    /**<Colorkey mode*//**<CNcomment:color key方式*/

    TDE2_COLORKEY_U unColorKeyValue;        /**<Colorkey value*//**<CNcomment:color key值*/

    TDE2_CLIPMODE_E enClipMode;             /**<Clip mode*//**<CNcomment:clip模式*/

    TDE2_RECT_S stClipRect;                 /**<Clipping area*//**<CNcomment:clip区域*/

    MT_BOOL bClutReload;                    /**<Whether to reload the CLUT*//**<CNcomment:是否重载clut表*/

    mt_u8 u8GlobalAlpha;                    /**<Global alpha*//**<CNcomment:全局alpha*/

    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;    /**<Source of the output alpha*//**<CNcomment:输出alpha来源*/

    mt_u32 u32Colorize;                     /**<Colorize value*//**<CNcomment:Colorize值*/

    TDE2_BLEND_OPT_S stBlendOpt;           /**<Options of the blending operation*//**<CNcomment:Blend操作选项 */

    TDE2_CSC_OPT_S stCscOpt;		/**<CSC parameter option*//**<CNcomment:Csc参数选项*/
    
}TDE2_PATTERN_FILL_OPT_S;



/**Definition of the pattern filling operation*/
/**CNcomment:模式填充操作信息定义 */
typedef struct mtTDE2_MULTIPLY_OPT_S
{
    TDE2_IMAGE_MULTIPLY_TYPE_E  multiplyType;
    
    TDE2_ALUCMD_E enAluCmd;                 /**<Logical operation type*//**<CNcomment:逻辑运算类型*/

    TDE2_ROP_CODE_E enRopCode_Color;        /**<ROP type of the color space*//**<CNcomment:颜色空间ROP类型*/

    TDE2_ROP_CODE_E enRopCode_Alpha;        /**<ROP type of the alpha component*//**<CNcomment:alpha的ROP类型*/

    TDE2_COLORKEY_MODE_E enColorKeyMode;    /**<Colorkey mode*//**<CNcomment:color key方式*/

    TDE2_COLORKEY_U unColorKeyValue;        /**<Colorkey value*//**<CNcomment:color key值*/

    TDE2_CLIPMODE_E enClipMode;             /**<Clip mode*//**<CNcomment:clip模式*/

    TDE2_RECT_S stClipRect;                 /**<Clipping area*//**<CNcomment:clip区域*/

    MT_BOOL bClutReload;                    /**<Whether to reload the CLUT*//**<CNcomment:是否重载clut表*/

    mt_u8 u8GlobalAlpha;                    /**<Global alpha*//**<CNcomment:全局alpha*/

    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;    /**<Source of the output alpha*//**<CNcomment:输出alpha来源*/

    mt_u32 u32Colorize;                     /**<Colorize value*//**<CNcomment:Colorize值*/

    TDE2_BLEND_OPT_S stBlendOpt;           /**<Options of the blending operation*//**<CNcomment:Blend操作选项 */

    TDE2_CSC_OPT_S stCscOpt;		/**<CSC parameter option*//**<CNcomment:Csc参数选项*/
}TDE2_MULTIPLY_OPT_S;

/*!
  This structure defines the trapez for driver usage
  */
typedef struct mtTDE2_TRAPEZ_S
{
  /*!
    top left pos
    */
  mt_u32 top_start_x;
  /*!
    bottom left pos
    */
  mt_u32 bottom_start_x;
  /*!
    top  w
    */
  mt_u32 top_len;
  /*!
    bottom w
    */
  mt_u32 bottom_len;
 
 }TDE2_TRAPEZ_S;

/**Definition of the trapez operation*/
typedef struct mtTDE2_TRAPEZ_OPT_S
{
    TDE2_ALUCMD_E enAluCmd;                 /**<Logical operation type*//**<CNcomment:逻辑运算类型*/

    TDE2_ROP_CODE_E enRopCode_Color;        /**<ROP type of the color space*//**<CNcomment:颜色空间ROP类型*/

    TDE2_ROP_CODE_E enRopCode_Alpha;        /**<ROP type of the alpha component*//**<CNcomment:alpha的ROP类型*/

    TDE2_COLORKEY_MODE_E enColorKeyMode;    /**<Colorkey mode*//**<CNcomment:color key方式*/

	TDE2_COLORKEY_SELECT_E enColorKeySelect; /*关键色匹配反转，满足匹配条件的做colorkey或不满足匹配条件的做colorkey*/	

    TDE2_COLORKEY_U unColorKeyValue;        /**<Colorkey value*//**<CNcomment:color key值*/

	TDE2_COLORKEY_CFG_S stColorKeyCfg;		/*当有多个colorkey时，用这个配置*/	

    mt_u8 u8GlobalAlpha;                    /**<Global alpha*//**<CNcomment:全局alpha*/

    TDE2_OUTALPHA_FROM_E enOutAlphaFrom;    /**<Source of the output alpha*//**<CNcomment:输出alpha来源*/

    TDE2_BLEND_OPT_S stBlendOpt;           /**<Options of the blending operation*//**<CNcomment:Blend操作选项 */

    /*!
    direction, 
    0:              1: 
    |\              /|
    | |            ||
    |/              \|
    */
    mt_u32 u32Direction;
    /*!
    dert factor denominator
    */
//    mt_u32 u32Dert_deno;
     /*!
    dert factor numerator
    */
//    mt_u32 u32Dert_num;

    TDE2_TRAPEZ_S stTrapez;
	mt_u32 u32Colorize;
}TDE2_TRAPEZ_OPT_S;


/**Definition of rotation directions*/
/**CNcomment:旋转方向定义 */
typedef enum mtTDE_ROTATE_ANGLE_E
{
    TDE_ROTATE_CLOCKWISE_90 = 0,    /**<Rotate 90° clockwise*//**< CNcomment:顺时针旋转90度 */
    TDE_ROTATE_CLOCKWISE_180,       /**<Rotate 180° clockwise*//**< CNcomment:顺时针旋转180度 */
    TDE_ROTATE_CLOCKWISE_270,       /**<Rotate 270° clockwise*//**< CNcomment:顺时针旋转270度 */
    TDE_ROTATE_BUTT
} TDE_ROTATE_ANGLE_E;

/**Definition of anti-flicker levels*/
/**CNcomment:抗闪烁级别定义 */
typedef enum mtTDE_DEFLICKER_LEVEL_E
{
    TDE_DEFLICKER_AUTO = 0, /**<Adaptation. The anti-flicker coefficient is selected by the TDE.*//**<CNcomment:自适应，由TDE选择抗闪烁系数*/
    TDE_DEFLICKER_LOW,      /**<Low-level anti-flicker*//**<CNcomment:低级别抗闪烁*/
    TDE_DEFLICKER_MIDDLE,   /**<Medium-level anti-flicker*//**CNcomment:中级别抗闪烁*/
    TDE_DEFLICKER_HIGH,     /**High-level anti-flicker*//**CNcomment:高级别抗闪烁*/
    TDE_DEFLICKER_BUTT
}TDE_DEFLICKER_LEVEL_E;

/* composed surface info */
typedef struct mtTDE_COMPOSOR_S
{
    TDE2_SURFACE_S stSrcSurface;
    TDE2_RECT_S stInRect;
    TDE2_RECT_S stOutRect;
    TDE2_OPT_S stOpt;
}TDE_COMPOSOR_S;

/* composed surface list */
typedef struct mtTDE_SURFACE_LIST_S
{
	mt_u32 u32SurfaceNum;
	TDE2_SURFACE_S *pDstSurface;
	TDE_COMPOSOR_S *pstComposor;
}TDE_SURFACE_LIST_S;

/* batch blit info */
typedef struct mtTDE_BLIT_INFO_S
{
    TDE2_SURFACE_S *pstSrcSur;
    TDE2_RECT_S *pstInRect;
    TDE2_SURFACE_S *pstBgSur;
    TDE2_RECT_S *pstBgRect;
    TDE2_SURFACE_S *pstMaskSur;
    TDE2_RECT_S *pstMaskRect;
    TDE2_RECT_S *pstOutRect;
    TDE2_OPT_S *pstOpt;
}TDE_BLIT_INFO_S;

/* batch blit list */
typedef struct mtTDE_BLIT_LIST_S
{
	mt_u32 u32BlitNum;
	TDE2_SURFACE_S *pDstSurface;
	TDE_BLIT_INFO_S *pstComposor;
}TDE_BLIT_LIST_S;
/*!
    rotator
   */
typedef struct mtTDE2_ROTATOR_ANGLE_S
{
  /*!
  when concerto:
    alpha = tan(A/2) * 2048
  when symphony:
    alpha = cos(A) * 2048
  */
  mt_u32 alpha;  
  /*!
      sin(A) * 2048
  */
  mt_u32 beta;  
  /*!
  only used in symphony
  
  if input angle larger than 45, A should be changed to the value in range of (0,45]
  angle_plus=0: orginal angle in range of (0, 45]
  angle_plus=1: orginal angle in range of (45, 90)
  angle_plus=2: orginal angle in range of (90, 135]
  angle_plus=3: orginal angle in range of (135, 180)
  angle_plus=4: orginal angle in range of (180, 225]
  angle_plus=5: orginal angle in range of (225, 270)
  angle_plus=6: orginal angle in range of (270, 315]
  angle_plus=7: orginal angle in range of (315, 360)
  */
  mt_u32 angle_plus;
}TDE2_ROTATOR_ANGLE_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_TDE_TYPE_H__ */


