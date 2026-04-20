/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _MT_UNF_CC_H_
#define _MT_UNF_CC_H_

#include "mt_type.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
*                    Macro Definitions
*****************************************************************************/
#define ARIBCC_MAX_LANGUAGE 2
/*************************** Structure Definition ****************************/
/** \addtogroup      CC */
/** @{*/  /** <!-- [CC]*/

/**CC data type*//** CNcomment:CC数据类型分类 */
typedef enum mtUNF_CC_DATA_TYPE_E
{
    MT_UNF_CC_DATA_TYPE_608,  /**<CC608*//**<CNcomment:CC608数据 */
    MT_UNF_CC_DATA_TYPE_708,        /**<CC708*//**<CNcomment:CC708数据 */
    MT_UNF_CC_DATA_TYPE_ARIB,       /**<ARIB CC*//**<CNcomment:ARIB CC数据 */
    MT_UNF_CC_DATA_TYPE_BUTT
}MT_UNF_CC_DATA_TYPE_E;

/**708CC service channel*//** CNcomment:708CC的服务通道号*/
typedef enum mtUNF_CC_708_SERVICE_NUM_E
{
    MT_UNF_CC_708_SERVICE1 = 0x1, /**<708CC service 1*//**<CNcomment:708CC服务1 */
    MT_UNF_CC_708_SERVICE2,       /**<708CC service 2*//**<CNcomment:708CC服务2 */
    MT_UNF_CC_708_SERVICE3,       /**<708CC service 3*//**<CNcomment:708CC服务3 */
    MT_UNF_CC_708_SERVICE4,       /**<708CC service 4*//**<CNcomment:708CC服务4 */
    MT_UNF_CC_708_SERVICE5,       /**<708CC service 5*//**<CNcomment:708CC服务5 */
    MT_UNF_CC_708_SERVICE6,       /**<708CC service 6*//**<CNcomment:708CC服务6 */
    MT_UNF_CC_708_SERVICE_BUTT
}MT_UNF_CC_708_SERVICE_NUM_E;

/**CC608 data type*//** CNcomment:CC608在line 21的数据类型分类 */
typedef enum mtUNF_CC_608_DATATYPE_E
{
    MT_UNF_CC_608_DATATYPE_CC1,       /**<CC1*//**<CNcomment:CC1 */
    MT_UNF_CC_608_DATATYPE_CC2,       /**<CC2*//**<CNcomment:CC2 */
    MT_UNF_CC_608_DATATYPE_CC3,       /**<CC3*//**<CNcomment:CC3 */
    MT_UNF_CC_608_DATATYPE_CC4,       /**<CC4*//**<CNcomment:CC4 */
    MT_UNF_CC_608_DATATYPE_TEXT1,     /**<TEXT1*//**<CNcomment:TEXT1 */
    MT_UNF_CC_608_DATATYPE_TEXT2,     /**<TEXT2*//**<CNcomment:TEXT2 */
    MT_UNF_CC_608_DATATYPE_TEXT3,     /**<TEXT3*//**<CNcomment:TEXT3 */
    MT_UNF_CC_608_DATATYPE_TEXT4,     /**<TEXT4*//**<CNcomment:TEXT4 */
    MT_UNF_CC_608_DATATYPE_BUTT
}MT_UNF_CC_608_DATATYPE_E;

/**standard color*//** CNcomment:标准颜色 */
typedef enum mtUNF_CC_COLOR_E
{
    MT_UNF_CC_COLOR_DEFAULT=0x00000000,       /**<default color*//**<CNcomment:默认颜色 */
    MT_UNF_CC_COLOR_BLACK=0xff000000,         /**<black*//**<CNcomment:黑色 */
    MT_UNF_CC_COLOR_WHITE=0xffffffff,         /**<white*//**<CNcomment:白色 */
    MT_UNF_CC_COLOR_RED=0xffff0000,           /**<red*//**<CNcomment:红色 */
    MT_UNF_CC_COLOR_GREEN=0xff00ff00,         /**<green*//**<CNcomment:绿色 */
    MT_UNF_CC_COLOR_BLUE=0xff0000ff,          /**<blue*//**<CNcomment:蓝色 */
    MT_UNF_CC_COLOR_YELLOW=0xffffff00,        /**<yellow*//**<CNcomment:黄色 */
    MT_UNF_CC_COLOR_MAGENTA=0xffff00ff,       /**<magenta*//**<CNcomment:品红 */
    MT_UNF_CC_COLOR_CYAN=0xff00ffff,          /**<cyan*//**<CNcomment:青色 */
}MT_UNF_CC_COLOR_E;

/**opacity*//** CNcomment:透明度 */
typedef enum mtUNF_CC_OPACITY_E
{
    MT_UNF_CC_OPACITY_DEFAULT,         /**<default*//**<CNcomment:默认透明度 */
    MT_UNF_CC_OPACITY_SOLID,           /**<opaque*//**<CNcomment:不透明 */
    MT_UNF_CC_OPACITY_FLASH,           /**<flash*//**<CNcomment:闪烁 */
    MT_UNF_CC_OPACITY_TRANSLUCENT,     /**<translucent*//**<CNcomment:半透明 */
    MT_UNF_CC_OPACITY_TRANSPARENT,     /**<transparent*//**<CNcomment:透明 */
    MT_UNF_CC_OPACITY_BUTT
}MT_UNF_CC_OPACITY_E;

/**justify*//** CNcomment:排版*/
typedef enum mtUNF_CC_JUSTIFY_E
{
    MT_UNF_CC_JUSTIFY_LEFT,        /**<left*//**<CNcomment:居左 */
    MT_UNF_CC_JUSTIFY_RIGHT,       /**<rigth*//**<CNcomment:居右 */
    MT_UNF_CC_JUSTIFY_CENTER,      /**<center*//**<CNcomment:居中 */
    MT_UNF_CC_JUSTIFY_FULL,        /**<full*//**<CNcomment:两端对齐 */
    MT_UNF_CC_JUSTIFY_BUTT
}MT_UNF_CC_JUSTIFY_E;

/**word wrap*//** CNcomment:自动换行*/
typedef enum mtUNF_CC_WORDWRAP_E
{
    MT_UNF_CC_WW_DISABLE,         /**<disable*//**<CNcomment:不自动换行 */
    MT_UNF_CC_WW_ENABLE,          /**<enable*//**<CNcomment:自动换行 */
    MT_UNF_CC_WW_BUTT
}MT_UNF_CC_WORDWRAP_E;

/**font style*//** CNcomment:字体风格 */
typedef enum mtUNF_CC_FONTSTYLE_E
{
    MT_UNF_CC_FONTSTYLE_DEFAULT,         /**<default*//**<CNcomment:默认字体风格 */
    MT_UNF_CC_FONTSTYLE_NORMAL,          /**<normal*//**<CNcomment:正常 */
    MT_UNF_CC_FONTSTYLE_ITALIC,          /**<italic*//**<CNcomment:斜体 */
    MT_UNF_CC_FONTSTYLE_UNDERLINE,       /**<underline*//**<CNcomment:下划线 */
    MT_UNF_CC_FONTSTYLE_ITALIC_UNDERLINE,/**<italic&underline*//**<CNcomment:斜体并且带下滑线 */
    MT_UNF_CC_FONTSTYLE_BUTT
}MT_UNF_CC_FONTSTYLE_E;

/**font size*//** CNcomment:字体大小 */
typedef enum mtUNF_CC_FONTSIZE_E
{
    MT_UNF_CC_FONTSIZE_DEFAULT,       /**<default font size *//**<CNcomment:默认字体大小 */
    MT_UNF_CC_FONTSIZE_SMALL,         /**<small*//**<CNcomment:小 */
    MT_UNF_CC_FONTSIZE_STANDARD,      /**<standard*//**<CNcomment:标准 */
    MT_UNF_CC_FONTSIZE_LARGE,         /**<large*//**<CNcomment:大 */
    MT_UNF_CC_FONTSIZE_BUTT
} MT_UNF_CC_FONTSIZE_E;

/**font name*//** CNcomment:字体样式 */
typedef enum  mtUNF_CC_FONTNAME_E
{
    MT_UNF_CC_FN_DEFAULT,                  /**<default *//**<CNcomment:默认字体样式 */
    MT_UNF_CC_FN_MONOSPACED,               /**<monospaced*//**<CNcomment:monospaced字体 */
    MT_UNF_CC_FN_PROPORT,                  /**<proport*//**<CNcomment:proport字体 */
    MT_UNF_CC_FN_MONOSPACED_NO_SERIAFS,    /**<monospaced with no seriafs*//**<CNcomment:monospaced字体(无衬线) */
    MT_UNF_CC_FN_PROPORT_NO_SERIAFS,       /**<proport with no seriafs*//**<CNcomment:proport字体(无衬线) */
    MT_UNF_CC_FN_CASUAL,                   /**<casual*//**<CNcomment:casual字体 */
    MT_UNF_CC_FN_CURSIVE,                  /**<cursive*//**<CNcomment:cursive字体 */
    MT_UNF_CC_FN_SMALL_CAPITALS,           /**<small capitals*//**<CNcomment:小写字体 */
    MT_UNF_CC_FN_BUTT
}MT_UNF_CC_FONTNAME_E;

/**font edge type*//** CNcomment:字体边缘类型 */
typedef enum mtUNF_CC_EdgeType_E
{
    MT_UNF_CC_EDGETYPE_DEFAULT,            /**<default *//**<CNcomment:默认字体边缘类型 */
    MT_UNF_CC_EDGETYPE_NONE,               /**<none edge type *//**<CNcomment:没有边缘 */
    MT_UNF_CC_EDGETYPE_RAISED,             /**<raised *//**<CNcomment:边缘突起 */
    MT_UNF_CC_EDGETYPE_DEPRESSED,          /**<depressed *//**<CNcomment:边缘凹下 */
    MT_UNF_CC_EDGETYPE_UNIFORM,            /**<uniform *//**<CNcomment:边缘统一 */
    MT_UNF_CC_EDGETYPE_LEFT_DROP_SHADOW,   /**<left drop shadow *//**<CNcomment:左下阴影 */
    MT_UNF_CC_EDGETYPE_RIGHT_DROP_SHADOW,  /**<right drop shadow *//**<CNcomment:右下阴影 */
    MT_UNF_CC_EDGETYPE_BUTT
}MT_UNF_CC_EdgeType_E;

/**display format of caption display screen*//** CNcomment:显示窗口的显示模式 */
typedef enum mtUNF_CC_DF_E
{
    MT_UNF_CC_DF_720X480,       /**<caption display screen is 720*480*//**<CNcomment:显示窗口的宽高是720*480 */
    MT_UNF_CC_DF_720X576,       /**<caption display screen is 720*576*//**<CNcomment:显示窗口的宽高是720*576 */
    MT_UNF_CC_DF_960X540,       /**<caption display screen is 960*540*//**<CNcomment:显示窗口的宽高是960*540 */
    MT_UNF_CC_DF_1280X720,      /**<caption display screen is 1280*720*//**<CNcomment:显示窗口的宽高是1280*720 */
    MT_UNF_CC_DF_1920X1080,     /**<caption display screen is 1920*1080*//**<CNcomment:显示窗口的宽高是1920*1080 */
    MT_UNF_CC_DF_BUTT
} MT_UNF_CC_DF_E;

/**CC display operation*//** CNcomment:CC显示操作 */
typedef enum mtUNF_CC_OPT_E
{
     MT_UNF_CC_OPT_DRAWTEXT = 0x1, /**<draw text *//**<CNcomment:绘制文本 */
     MT_UNF_CC_OPT_DRAWBITMAP,     /**<draw bitmap *//**<CNcomment:绘制位图 */
     MT_UNF_CC_OPT_FILLRECT,       /**<fill rect *//**<CNcomment:填充矩形区域 */
     MT_UNF_CC_OPT_BUTT
} MT_UNF_CC_OPT_E;

/**Arib CC rollup mode*//** CNcomment:arib CC rollup模式*/
typedef enum mtUNF_CC_ARIB_ROLLUP_E
{
    MT_UNF_CC_ARIB_NON_ROLLUP,        /**<Non roll-up *//**<CNcomment:非rollup模式*/
    MT_UNF_CC_ARIB_ROLLUP,      /**<roll-up*//**<CNcomment:rollup模式*/
    MT_UNF_CC_ARIB_ROLLUP_BUTT
}MT_UNF_CC_ARIB_ROLLUP_E;


/**Arib CC character coding*//** CNcomment:arib CC 字符编码方式*/
typedef enum mtUNF_CC_ARIB_TCS_E
{
    MT_UNF_CC_ARIB_TCS_8BIT,         /**<the type of character codig is 8bit-code*//**<CNcomment:字符编码方式(8位)*/
    MT_UNF_CC_ARIB_TCS_BUTT
}MT_UNF_CC_ARIB_TCS_E;

/**Arib CC display format*//** CNcomment:arib CC显示方式*/
typedef enum mtUNF_CC_ARIB_DF_E
{
    MT_UNF_CC_ARIB_DF_HORIZONTAL_SD,                /**<horizontal writing in stardard density*/    /**<CNcomment:水平显示，标清*/
    MT_UNF_CC_ARIB_DF_VERTICAL_SD,                  /**<vertical writing in standard density*/      /**<CNcomment:垂直显示，标清*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_HD,                /**<horizontal writing in high density*/        /**<CNcomment:水平显示，高清*/
    MT_UNF_CC_ARIB_DF_VERTICAL_HD,                  /**<vertical writing in high density*/          /**<CNcomment:垂直显示，高清*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_WESTERN,           /**<horizontal writing of western language*/    /**<CNcomment:水平显示，西文*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_1920X1080,         /**<horizontal writing in 1920X1080*/           /**<CNcomment:水平显示，屏幕1920X1080*/
    MT_UNF_CC_ARIB_DF_VERTICAL_1920X1080,           /**<vertical writing in 1920X1080*/             /**<CNcomment:垂直显示，屏幕1920X1080*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_960X540,           /**<horizontal writing in 960X540*/             /**<CNcomment:水平显示，屏幕960X540*/
    MT_UNF_CC_ARIB_DF_VERTICAL_960X540,             /**<vertical writing in 960X540*/               /**<CNcomment:垂直显示，屏幕960X540*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_1280X720,          /**<horizontal writing in 1280X720*/            /**<CNcomment:水平显示，屏幕1280X720*/
    MT_UNF_CC_ARIB_DF_VERTICAL_1280X720,            /**<vertical writing in 1280X720*/              /**<CNcomment:垂直显示，屏幕1280X720*/
    MT_UNF_CC_ARIB_DF_HORIZONTAL_720X480,           /**<horizontal writing in 720X480*/             /**<CNcomment:水平显示，屏幕720X480*/
    MT_UNF_CC_ARIB_DF_VERTICAL_720X480,             /**<vertical writing in 720X480*/               /**<CNcomment:垂直显示，屏幕720X480*/
    MT_UNF_CC_ARIB_DF_BUTT
}MT_UNF_CC_ARIB_DF_E;

/**Arib CC display mode*//** CNcomment:arib CC显示模式*/
typedef enum mtUNF_CC_ARIB_DMF_E
{
    MT_UNF_CC_ARIB_DMF_AUTO_AND_AUTO=0x0,           /**<atomatic display when received ,automatic display when recording and playback */                    /**<CNcomment:接收后自动播放，录制和回放时自动播放*/
    MT_UNF_CC_ARIB_DMF_AUTO_AND_NOT,                /**<atomatic display when received ,Non-displayed automatically when recording and playback*/           /**<CNcomment:接收后自动播放，录制和回放时不自动播放*/
    MT_UNF_CC_ARIB_DMF_AUTO_AND_SELECT,             /**<atomatic display when received ,Selectable display when recording and playback*/                    /**<CNcomment:接收后自动播放，录制和回放时可选择播放*/
    MT_UNF_CC_ARIB_DMF_NON_AND_AUTO=0x4,            /**<non-displayed automatically when received, automatic display when recording and playback */         /**<CNcomment:接收时不自动播放，录制和回放时自动播放*/
    MT_UNF_CC_ARIB_DMF_NON_AND_NON,                 /**<non-displayed automatically when received,Non-displayed automatically when recording and playback*/ /**<CNcomment:接收时不自动播放，录制和回放时不自动播放*/
    MT_UNF_CC_ARIB_DMF_NON_AND_SELECT,              /**<non-displayed automatically when received,Selectable display when recording and playback*/          /**<CNcomment:接收时不自动播放，录制和回放时可选择播放*/
    MT_UNF_CC_ARIB_DMF_SELECT_AND_AUTO=0x8,         /**<selectable display when received, automatic display when recording and playback */                  /**<CNcomment:接收时可选择播放，录制和回放时可自动播放*/
    MT_UNF_CC_ARIB_DMF_SELECT_AND_NON,              /**<selectable display when received,Non- displayed automatically when recording and playback*/         /**<CNcomment:接收时可选择播放，录制和回放时不自动播放*/
    MT_UNF_CC_ARIB_DMF_SELECT_AND_SELECT,           /**<selectable display when received,Selectable display when recording and playback*/                   /**<CNcomment:接收时可选择播放，录制和回放时可选择播放*/
    MT_UNF_CC_ARIB_DMF_SPECIAL_AND_AUTO=0xc,        /**<automatic display/non-display under specific condition when received,automatic display when recording and playback */           /**<CNcomment:接收时特定情况下自动/非自动播放，录制和回放时自动播放*/
    MT_UNF_CC_ARIB_DMF_SPECIAL_AND_NON,             /**<automatic display/non-display under specific condition when received,Non-displayed automatically when recording and playback*/  /**<CNcomment:接收时特定情况下自动/非自动播放，录制和回放时不自动播放*/
    MT_UNF_CC_ARIB_DMF_SPECIAL_AND_SELECT,          /**<automatic display/non-display under specific condition when received,Selectable display when recording and playback*/           /**<CNcomment:接收时特定情况下自动/非自动播放，录制和回放时可选择播放*/
    MT_UNF_CC_ARIB_DMF_BUTT
}MT_UNF_CC_ARIB_DMF_E;

/**Arib CC time control mode*//** CNcomment:arib CC时间控制模式*/
typedef enum mtUNF_CC_ARIB_TMD_E
{
    MT_UNF_CC_ARIB_TMD_FREE,            /**<playback time is not restricted to synchronize to the clock*//**<CNcomment:重放时间和时钟无关*/
    MT_UNF_CC_ARIB_TMD_REAL_TIME,       /**<playback time is given by PTS*//**<CNcomment:重放时间由PTS给出*/
    MT_UNF_CC_ARIB_TMD_OFFSET_TIME,     /**<playback time added with offset time should be the new playback time*//**<CNcomment:重放时间加上偏移时间将是新的重放时间*/
    MT_UNF_CC_ARIB_TMD_BUTT
}MT_UNF_CC_ARIB_TMD_E;

/**CC608 VBI data*//** CNcomment:CC608 VBI数据 */
typedef struct mtUNF_CC_VBI_DADA_S
{
    mt_u8 u8FieldParity; /**<parity field *//**<CNcomment:奇偶场 */
    mt_u8 u8Data1;       /**<first byte *//**<CNcomment:第一个字节 */
    mt_u8 u8Data2;       /**<second byte *//**<CNcomment:第二个字节 */
}MT_UNF_CC_VBI_DADA_S;

/**CC rect*//** CNcomment:定义CC矩形区域*/
typedef struct mtUNF_CC_RECT_S
{
    mt_u16 x;           /**<x cordinate *//**<CNcomment:矩形左上角的x坐标 */
    mt_u16 y;           /**<y cordinate *//**<CNcomment:矩形左上角的y坐标 */
    mt_u16 width;       /**<rect width *//**<CNcomment:矩形的宽度 */
    mt_u16 height;      /**<rect heigth *//**<CNcomment:矩形的高度 */
}MT_UNF_CC_RECT_S;

/**CC color components*//** CNcomment:CC像素颜色结构 */
typedef struct mtUNF_CC_COLOR_S
{
    mt_u8 u8Blue;       /**<blue component *//**<CNcomment:B分量颜色值 */
    mt_u8 u8Green;      /**<green component *//**<CNcomment:G分量颜色值 */
    mt_u8 u8Red;        /**<red component *//**<CNcomment:R分量颜色值 */
    mt_u8 u8Alpha;      /**<alpha component *//**<CNcomment:透明度，值为0为透明，0xFF为不透明 */
} MT_UNF_CC_COLOR_S;

/**CC data with text format*//** CNcomment:文本格式的CC数据 */
typedef struct mtUNF_CC_TEXT_S
{
    mt_u16                 *pu16Text;    /**<cc data,2 bytes,unicode *//**<CNcomment:cc数据，unicode编码，2个字节 */
    mt_u8                  u8TextLen;    /**<cc data length *//**<CNcomment:要显示的cc数据长度 */

    MT_UNF_CC_COLOR_S      stFgColor;    /**<cc foregroud color*//**<CNcomment:要显示的cc数据前景色 */
    MT_UNF_CC_COLOR_S      stBgColor;    /**<cc backgroud color*//**<CNcomment:要显示的cc数据字体背景色 */
    MT_UNF_CC_COLOR_S      stEdgeColor;   /**<cc edge color*//**<CNcomment:要显示的cc数据字体边缘颜色*/

    mt_u8                  u8Justify;    /*see MT_UNF_CC_JUSTIFY_E,used when decode cc708*//**<CNcomment:cc数据显示时的排版方式 */
    mt_u8                  u8WordWrap;   /*see MT_UNF_CC_WORDWRAP_E,used when decode cc708*//**<CNcomment:自动换行 */
    MT_UNF_CC_FONTSTYLE_E  enFontStyle;  /*font style ,see MT_UNF_CC_FONTSTYLE_E*//**<CNcomment:要显示的cc数据的字体风格 */
    MT_UNF_CC_FONTSIZE_E   enFontSize;   /*font size ,see MT_UNF_CC_FONTSIZE_E*//**<CNcomment:要显示的cc数据的字体大小 */
    MT_UNF_CC_EdgeType_E    enEdgetype;  /**<cc edge type*//**<CNcomment:要显示的cc数据字体边缘类型*/
} MT_UNF_CC_TEXT_S;

/**CC data with bitmap format*//** CNcomment:位图格式的CC数据 */
typedef struct mtUNF_CC_BITMAP_S
{
    mt_s32                 s32BitWidth;         /**<bit width, is 2/4/8/16/24/32 *//**<CNcomment:位宽,值是2/4/8/16/24/32 */
    mt_u8                  *pmt_u8BitmapData;      /**<pixel data *//**<CNcomment:图像数据 */
    mt_u32                 u32BitmapDataLen;    /**<length of pixel data *//**<CNcomment:图像数据长度 */
    MT_UNF_CC_COLOR_S      astPalette[256];     /**<palette data *//**<CNcomment:调色板数据 */
    mt_u32                 u32PaletteLen;       /**<length of palette data *//**<CNcomment:调色板数据长度 */
} MT_UNF_CC_BITMAP_S;

/**parameter of fill rect*//** CNcomment:填充矩形区域的参数 */
typedef struct mtUNF_CC_FILLRECT_S
{
    MT_UNF_CC_COLOR_S      stColor;      /**<color *//**<CNcomment:颜色值 */
} MT_UNF_CC_FILLRECT_S;

/**CC display param*//** CNcomment:定义CC数据显示时的参数 */
typedef struct mtUNF_CC_DISPLAY_PARAM_S
{
    MT_UNF_CC_OPT_E        enOpt;
    mt_u32                 u32DisplayWidth; /**<display screen width*//**<CNcomment:画布宽度 */
    mt_u32                 u32DisplayHeight;/**<display screen height*//**<CNcomment:画布高度 */
    MT_UNF_CC_RECT_S       stRect;          /**<cc display location*//**<CNcomment:要显示的cc数据的位置信息 */
    union
    {
        MT_UNF_CC_TEXT_S   stText;          /**<CC data with text format*//**<CNcomment:文本格式的CC数据 */
        MT_UNF_CC_BITMAP_S stBitmap;        /**<CC data with bitmap format*//**<CNcomment:位图格式的CC数据 */
        MT_UNF_CC_FILLRECT_S stFillRect;    /**<parameter of fill rect*//**<CNcomment:填充矩形区域的参数 */
    } unDispParam;
} MT_UNF_CC_DISPLAY_PARAM_S;

/**
\brief cc get text size callback function. CNcomment: cc用于获得字体大小(宽高)的回调函数。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in] mt_u32UserData user data. CNcomment: MT_UNF_CC_Create传入的用户私有数据。CNend
\param[in] mt_u16Str  string. CNcomment: 数据地址。CNend
\param[in] StrNum  number of character. CNcomment: 数据长度。CNend
\param[out] width  total width of character. CNcomment: 得到的字体宽。CNend
\param[out] heigth heigth of character. CNcomment: 得到的字体高。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
none. CNcomment: 无。CNend
*/
typedef mt_s32 (*MT_UNF_CC_GETTEXTSIZE_CB_FN)(ulong mt_u32Userdata, mt_u16 *mt_u16Str,mt_s32 mt_s32StrNum, mt_s32 *pmt_s32Width, mt_s32 *pmt_s32Heigth);


/**
\brief cc display callback function. CNcomment: cc显示数据回调。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in] mt_u32UserData user data. CNcomment: MT_UNF_CC_Create传入的用户私有数据。CNend
\param[in] pstCCdisplayParam parameter of display. CNcomment: cc显示数据结构。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
none. CNcomment: 无。CNend
*/
typedef mt_s32 (*MT_UNF_CC_DISPLAY_CB_FN)(ulong mt_u32Userdata, MT_UNF_CC_DISPLAY_PARAM_S *pstCCdisplayParam);


/**
\brief cc get avplay pts callback function. CNcomment: cc获取当前节目的PTS值的回调。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in] mt_u32UserData user data. CNcomment: MT_UNF_CC_Create传入的用户私有数据。CNend
\param[out] pmt_s64CurrentPts current pts. CNcomment: 存储当前PTS值的变量指针。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
none. CNcomment: 无。CNend
*/
typedef mt_s32 (*MT_UNF_CC_GETPTS_CB_FN)(ulong mt_u32UserData, mt_s64 *pmt_s64CurrentPts);

/**
\brief cc data blit callback function. CNcomment: cc数据搬移回调。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in] mt_u32UserData user data. CNcomment: MT_UNF_CC_Create传入的用户私有数据。CNend
\param[in] SrcRect source rect. CNcomment: 需要搬移的原区域。CNend
\param[in] DstRect destination rect. CNcomment: 需要搬移的目标区域。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
none. CNcomment: 无。CNend
*/
typedef mt_s32 (*MT_UNF_CC_BLIT_CB_FN)(ulong mt_u32UserData, MT_UNF_CC_RECT_S *pstSrcRect, MT_UNF_CC_RECT_S *pstDstRect);

/**
\brief cc data blit callback function. CNcomment: cc vbi 数据输出回调。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in] mt_u32UserData user data. CNcomment: MT_UNF_CC_Create传入的用户私有数据。CNend
\param[out] pstVBIData VBI data. CNcomment: VBI数据结构。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
none. CNcomment: 无。CNend
*/
typedef mt_s32 (*MT_UNF_CC_VBI_CB_FN)(ulong mt_u32UserData, MT_UNF_CC_VBI_DADA_S *pstVBIData);

/**
\brief output XDS packets in CC608. CNcomment: 输出CC608中的XDS包数据。CNend
\attention \n
XDS packet is a third data service intended to supply program related and other information.
CNcomment: XDS包是第三方数据服务，用来提供节目或者其他相关信息。CNend
\param[in] mt_u8XDSClass XDS class. CNcomment: XDS的分类。CNend
\param[in] mt_u8XDSPacketType  XDS packet type. CNcomment: XDS数据包的类型。CNend
\param[in] pmt_u8Data  XDS data. CNcomment: XDS数据。CNend
\param[in] mt_u8DataLen  XDS data length. CNcomment: XDS数据长度。CNend
\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\see \n
Decoding of XDS packet data, see sample/cc/sample_cc_xds.c.
CNcomment: XDS数据包的解析，请参看sdk包sample/cc/sample_cc_xds.c。CNend
*/
typedef mt_s32 (*MT_UNF_CC_XDS_CB_FN)(mt_u8 mt_u8XDSClass, mt_u8 mt_u8XDSPacketType, mt_u8 *pmt_u8Data, mt_u8 mt_u8DataLen);


/**user data in MPEG*//** CNcomment:MPEG中的USERDATA数据 */
typedef struct mtUNF_CC_USERDATA_S
{
    mt_u8       *pu8userdata;  /**<cc user data in mpeg *//**<CNcomment:mpeg视频帧中的cc用户数据 */
    mt_u32      u32dataLen;    /**<cc user data length *//**<CNcomment:cc数据长度 */
    MT_BOOL     bTopFieldFirst;
} MT_UNF_CC_USERDATA_S;

/**CC608 config param *//** CNcomment:CC608 配置信息参数 */
/**CC608 just support 8 stardard colors*//**CNcomment:CC608只支持8种标准颜色*/
typedef struct mtUNF_CC_608_CONFIGPARAM_S
{
    MT_UNF_CC_608_DATATYPE_E    enCC608DataType;        /**<CC608 data type *//**<CNcomment:配置cc608数据类型 */
    mt_u32                      u32CC608TextColor;      /**<CC608 text color *//**<CNcomment:配置cc608字体颜色 */
    MT_UNF_CC_OPACITY_E         enCC608TextOpac;        /**<CC608 text opacity *//**<CNcomment:配置cc608字体透明度 */
    mt_u32                      u32CC608BgColor;        /**<CC608 background color *//**<CNcomment:配置cc608背景色 */
    MT_UNF_CC_OPACITY_E         enCC608BgOpac;          /**<CC608 background opacity *//**<CNcomment:配置cc608背景透明度 */
    MT_UNF_CC_FONTSTYLE_E       enCC608FontStyle;       /**<CC608 font style *//**<CNcomment:配置cc608字体风格 */
    MT_UNF_CC_DF_E              enCC608DispFormat;      /**<CC608 display format of caption display screen *//**<CNcomment:配置cc608显示模式 */
    MT_BOOL                     bLeadingTailingSpace;   /**< CC608 leading/tailing space flag*//**<CNcomment: 是否显示leading/tailing space*/
} MT_UNF_CC_608_CONFIGPARAM_S;

/**CC708 config param *//** CNcomment:CC708 配置信息参数 */
/**CC708 color specification CEA-708-B-1999.pdf section 9.20 Color Representation*//**CNcomment:CC708颜色说明请参考协议CEA-708-B-1999.pdf 9.20 颜色呈现 章节*/ 
/**CC708 support 64 colors.one of RGB color components can be 0x00,0x5f,0xaf,0xff*//**CNcomment:CC708支持64种颜色,一个RGB分量能取的值有0x00,0x5f,0xaf,0xff共4种*/
typedef struct mtUNF_CC_708_CONFIGPARAM_S
{
    MT_UNF_CC_708_SERVICE_NUM_E enCC708ServiceNum;      /**<CC708 service number *//**<CNcomment:配置cc708服务通道号 */
    MT_UNF_CC_FONTNAME_E        enCC708FontName;        /**<CC708 font name *//**<CNcomment:配置cc708字体 */
    MT_UNF_CC_FONTSTYLE_E       enCC708FontStyle;       /**<CC708 font style *//**<CNcomment:配置cc708字体风格 */
    MT_UNF_CC_FONTSIZE_E        enCC708FontSize;        /**<CC708 font size *//**<CNcomment:配置cc708字体大小 */
    mt_u32                      u32CC708TextColor;      /**<CC708 text color *//**<CNcomment:配置cc708字体颜色 */
    MT_UNF_CC_OPACITY_E         enCC708TextOpac;        /**<CC708 text opacity *//**<CNcomment:配置cc708字体透明度 */
    mt_u32                      u32CC708BgColor;        /**<CC708 background color *//**<CNcomment:配置cc708背景颜色 */
    MT_UNF_CC_OPACITY_E         enCC708BgOpac;          /**<CC708 background opacity *//**<CNcomment:配置cc708背景透明度 */
    mt_u32                      u32CC708WinColor;       /**<CC708 window color *//**<CNcomment:配置cc708窗口颜色 */
    MT_UNF_CC_OPACITY_E         enCC708WinOpac;         /**<CC708 window opacity *//**<CNcomment:配置cc708窗口透明度 */
    MT_UNF_CC_EdgeType_E        enCC708TextEdgeType;    /**<CC708 text egde type *//**<CNcomment:配置cc708字体边缘类型 */
    mt_u32                      u32CC708TextEdgeColor;  /**<CC708 text edge color *//**<CNcomment:配置cc708字体边缘颜色 */
    MT_UNF_CC_DF_E              enCC708DispFormat;      /**<CC708 display format of caption display screen *//**<CNcomment:配置cc708显示模式 */
} MT_UNF_CC_708_CONFIGPARAM_S;

/**ARIB CC config param *//** CNcomment:ARIB CC 配置信息参数 */
typedef struct mtUNF_CC_ARIB_CONFIGPARAM_S
{
    mt_u32      mt_u32BufferSize;   /**<size of buffer which used to cache pes data,Recommends its value is 64K ~ 512K.note:This value can only be set when created,does not support dynamic setting*/
                                 /**<CNcomment:缓存PES数据的缓冲区大小，取值为64k~512K。注意:这个值只能在创建时设置，不支持动态设置*/
} MT_UNF_CC_ARIB_CONFIGPARAM_S;

/**CC data attribution *//** CNcomment:CC属性信息 */
typedef struct mtUNF_CC_ATTR_S
{
    MT_UNF_CC_DATA_TYPE_E enCCDataType;   /**<cc data type *//**<CNcomment:cc数据类型 */
    union
    {
        MT_UNF_CC_608_CONFIGPARAM_S  stCC608ConfigParam;   /**<CC608 config param *//**<CNcomment:CC608 配置信息参数 */
        MT_UNF_CC_708_CONFIGPARAM_S  stCC708ConfigParam;   /**<CC708 config param *//**<CNcomment:CC708 配置信息参数 */
        MT_UNF_CC_ARIB_CONFIGPARAM_S stCCARIBConfigParam;  /**<ARIB CC config param *//**<CNcomment:ARIB CC 配置信息参数 */
    } unCCConfig;
} MT_UNF_CC_ATTR_S;

/**CC instance param *//** CNcomment:创建实例时需要的参数信息 */
typedef struct mtUNF_CC_PARAM_S
{
    MT_UNF_CC_ATTR_S             stCCAttr;           /**<cc attribution *//**<CNcomment:cc属性信息 */

    MT_UNF_CC_GETPTS_CB_FN       pfnCCGetPts;        /**<get current pts callback function *//**<CNcomment:获取当前pts的回调函数 */
    MT_UNF_CC_DISPLAY_CB_FN      pfnCCDisplay;       /**<cc display callback function *//**<CNcomment:cc显示的回调函数 */
    MT_UNF_CC_GETTEXTSIZE_CB_FN  pfnCCGetTextSize;   /**<cc get text size callback function *//**<CNcomment:cc用于获得字体大小(宽高)的回调函数  */
    MT_UNF_CC_BLIT_CB_FN         pfnBlit;            /**<cc data blit callback function *//**<CNcomment:用于cc数据在屏幕上搬移的回调函数 */
    MT_UNF_CC_VBI_CB_FN          pfnVBIOutput;       /**<output VBI data callback function *//**<CNcomment:输出VBI数据回调函数 */
    MT_UNF_CC_XDS_CB_FN          pfnXDSOutput;       /**<output XDS packets function *//**<CNcomment:输出CC608中的XDS包数据 */
    ulong                        u32UserData;        /**<user data,used in callback function *//**<CNcomment:用户私有数据，用于回调函数 */
} MT_UNF_CC_PARAM_S;

/*ARIB CC info node*//** CNcomment:arib cc字幕信息节点 */
typedef struct mtUNF_CC_ARIB_INFONODE_S
{
    mt_u8                       u8LanguageTag;            /**<identification of language*//**<CNcomment:字幕语言标记 */
    MT_UNF_CC_ARIB_DMF_E        enCCAribDMF;              /**<display mode *//**<CNcomment:显示模式 */
    mt_char                     acISO639LanguageCode[4];  /**<language code *//**<CNcomment:字幕语言代码 */
    MT_UNF_CC_ARIB_DF_E         enCCAribDF;               /**<display format *//**<CNcomment:显示方式 */
    MT_UNF_CC_ARIB_TCS_E        enCCAribTCS;              /**<character coding *//**<CNcomment:字符编码 */
    MT_UNF_CC_ARIB_ROLLUP_E     enCCAribRollup;           /**<roll-up mode *//**<CNcomment:roll-up模式 */
}MT_UNF_CC_ARIB_INFONODE_S;

/*ARIB CC info struct*//** CNcomment:arib cc字幕信息结构体 */
typedef struct mtUNF_CC_ARIB_INFO_S
{
    MT_UNF_CC_ARIB_TMD_E enCCAribTMD;   /**<time control mode*//**<CNcomment:时间控制模式 */
    mt_u32 u32NumLanguage;       /**<number of languages *//**<CNcomment:字幕语言个数 */
    MT_UNF_CC_ARIB_INFONODE_S stCCAribInfonode[ARIBCC_MAX_LANGUAGE];     /**<array of arib cc info *//**<CNcomment:arib cc字幕信息数组 */
}MT_UNF_CC_ARIB_INFO_S;


/** @} */  /** <!-- ==== Structure Definition end ==== */


/******************************* API Declaration *****************************/
/**
\brief Initialize cc module. CNcomment: 初始化CC模块。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_SUCCESS initialize success. CNcomment: 初始化成功。CNend
\retval ::MT_FAILURE initialize failure. CNcomment: 初始化失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Init(mt_void);

/**
\brief DeInitialize cc module. CNcomment: 去初始化CC模块。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_SUCCESS deinitialize success. CNcomment: 去初始化成功。CNend
\retval ::MT_FAILURE deinitialize failure. CNcomment: 去初始化失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_DeInit(mt_void);

/**
\brief Get default attribution in cc module. CNcomment: 获取CC模块的默认属性值。CNend
\attention \n
none. CNcomment: 无。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_GetDefaultAttr(MT_UNF_CC_ATTR_S *pstDefaultAttr);

/**
\brief open cc module. CNcomment: 创建cc实例。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  pstAttr  cc attribution. CNcomment: 创建时传入的解码器属性。CNend
\param[out]  phCC  cc handle. CNcomment: cc模块句柄。CNend
\retval ::MT_SUCCESS success. CNcomment: 创建成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 创建失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Create(MT_UNF_CC_PARAM_S *pstCCParam, MT_HANDLE *phCC);

/**
\brief close cc module. CNcomment: 销毁cc实例。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_SUCCESS success. CNcomment: 销毁成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 销毁失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Destroy(MT_HANDLE hCC);

void MT_NUF_CC_Set_showmode(int show_num);

/**
\brief start cc module. CNcomment: 开始cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Start(MT_HANDLE hCC);

/**
\brief stop cc module. CNcomment: 结束cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Stop(MT_HANDLE hCC);

/**
\brief reset cc module. CNcomment: 复位cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_Reset(MT_HANDLE hCC);

/**
\brief inject mpeg userdata to  cc module. CNcomment: 注入mpeg用户数据到cc模块。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\param[in]  pstUserData  cc userdata structure used in inject cc data. CNcomment: 用户数据结构体。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_InjectUserData(MT_HANDLE hCC, MT_UNF_CC_USERDATA_S *pstUserData);

/**
\brief inject cc pes data to cc module. CNcomment: 注入pes数据到cc模块(暂不支持)。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment:模块句柄。CNend
\param[in]  pmt_u8PesData  pes data address. CNcomment: pes数据首地址。CNend
\param[in]  mt_u32DataLen  pes data length. CNcomment: pes数据长度。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_InjectPESData(MT_HANDLE hCC, mt_u8 *pmt_u8PesData, mt_u32 mt_u32DataLen);

/**
\brief get cc attribution. CNcomment: 获取cc属性信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment: 模块句柄。CNend
\param[out]  pstCCAttr  cc attribution structure. CNcomment: 属性信息结构体。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_GetAttr(MT_HANDLE hCC, MT_UNF_CC_ATTR_S *pstCCAttr);

/**
\brief set cc attribution. CNcomment:设置cc属性信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  cc handle. CNcomment. CNcomment: 模块句柄。CNend
\param[in]  pstCCAttr  cc attribution structure. CNcomment:属性信息结构体。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_SetAttr(MT_HANDLE hCC, MT_UNF_CC_ATTR_S *pstCCAttr);


/**
\brief get cc arib info. CNcomment: 获取arib cc字幕信息。CNend
\attention \n
none. CNcomment: 无。CNend
\param[in]  hCC  arib cc handle. CNcomment: 模块句柄。CNend
\param[out]  pstCCAttr  arib cc info structure. CNcomment:arib cc字幕信息结构体。CNend
\retval ::MT_SUCCESS success. CNcomment: 成功。CNend
\retval ::MT_FAILURE failure. CNcomment: 失败。CNend
\see \n
none. CNcomment: 无。CNend
*/
mt_s32 MT_UNF_CC_GetARIBCCInfo(MT_HANDLE hCC,MT_UNF_CC_ARIB_INFO_S *pstCCAribInfo);

/** @} */  /** <!-- ==== API declaration end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
