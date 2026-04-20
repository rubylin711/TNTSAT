/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_disp.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/
/**
 * \file
 **\brief : define DISPLAY module information
 * \brief supply infor about display.
 */

#ifndef  __MT_UNF_DISP_H__
#define  __MT_UNF_DISP_H__

#include "mt_unf_common.h"
#include "mt_unf_hdmi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*************************** Structure Definition ****************************/
/** \addtogroup      DISPLAY */
/** @{ */  /** <!-- [DISPLAY] */

/**enum define about DISPLAY channel*/
/**CNcomment:定义DISPLAY通道枚举*/
typedef enum mtUNF_DISP_E
{
    MT_UNF_DISPLAY0 = 0x0,  /**< DISPLAY0,Compatible with SD */ /**<CNcomment:高清DISPLAY0，兼容标清 */
    MT_UNF_DISPLAY1,        /**< DISPLAY1 *//**<CNcomment: 高清DISPLAY1 */
    MT_UNF_DISPLAY2,        /**< DISPLAY2 *//**<CNcomment: 高清DISPLAY2 */
    MT_UNF_DISPLAY_BUTT
}MT_UNF_DISP_E;

/**enum define about DISPLAY layer zorder*/
/**CNcomment:定义DISPLAY 图层顺序*/
typedef enum mtLAYER_ZORDER_ABS_E
{
    MT_LAYER_ZORDER_ABS_OSD0_OSD1_SUB = 0,
    MT_LAYER_ZORDER_ABS_OSD1_OSD0_SUB ,
    MT_LAYER_ZORDER_ABS_SUB_OSD1_OSD0 ,
    MT_LAYER_ZORDER_ABS_OSD1_SUB_OSD0 ,
    MT_LAYER_ZORDER_ABS_SUB_OSD0_OSD1 ,
    MT_LAYER_ZORDER_ABS_OSD0_SUB_OSD1 ,
    MT_LAYER_ZORDER_ABS_BUTT
} MT_LAYER_ZORDER_ABS_E;

/**max DAC count*/
/**CNcomment:最大的DAC数目*/
#define MAX_DAC_NUM ( 6 )

/**define the LCD data format*/
/**CNcomment:定义LCD的数据格式*/
typedef enum mtMT_UNF_DISP_INTF_DATA_FMT
{
    MT_UNF_DISP_INTF_DATA_FMT_YUV422   = 0,         /**<YUV422,data width is 16*//**<CNcomment:YUV422，位宽为16*/
    MT_UNF_DISP_INTF_DATA_FMT_RGB565   = 0x8,       /**<RGB565,data width is 16*//**<CNcomment:RGB565，位宽为16*/
    MT_UNF_DISP_INTF_DATA_FMT_RGB444   = 0xa,       /**<RGB444,data width is 16*//**<CNcomment:RGB444，位宽为16*/
    MT_UNF_DISP_INTF_DATA_FMT_RGB666   = 0xb,       /**<RGB666,data width is 24*//**<CNcomment:RGB666，位宽为24*/
    MT_UNF_DISP_INTF_DATA_FMT_RGB888   = 0xc,       /**<RGB888,data widht is 24*//**<CNcomment:RGB888，位宽为24*/
    MT_UNF_DISP_INTF_DATA_FMT_BUTT
}MT_UNF_DISP_INTF_DATA_FMT_E;

/**define LCD output data width*/
/**CNcomment:定义LCD输出的数据位宽*/
typedef enum mtUNF_DISP_INTF_DATA_WIDTH_E
{
    MT_UNF_DISP_INTF_DATA_WIDTH8 = 0,       /**<8 bits*//**<CNcomment:8位*/
    MT_UNF_DISP_INTF_DATA_WIDTH16,          /**<16 bits*//**<CNcomment:16位*/
    MT_UNF_DISP_INTF_DATA_WIDTH24,          /**<24 bits*//**<CNcomment:24位*/
    MT_UNF_DISP_INTF_DATA_WIDTH_BUTT
}MT_UNF_DISP_INTF_DATA_WIDTH_E;

/**define LCD timing */
/**CNcomment:定义LCD的时序参数*/
typedef struct mtUNF_DISP_TIMING_S
{
    mt_u32                        VFB;            /**<vertical front blank*//**<CNcomment:垂直前消隐*/
    mt_u32                        VBB;            /**<vertical back blank*//**<CNcomment:垂直后消隐*/
    mt_u32                        VACT;           /**<vertical active area*//**<CNcomment:垂直有效区*/
    mt_u32                        HFB;            /**<horizonal front blank*//**<CNcomment:水平前消隐*/
    mt_u32                        HBB;            /**<horizonal back blank*//**<CNcomment:水平后消隐*/
    mt_u32                        HACT;           /**<horizonal active area*/ /**<CNcomment:水平有效区*/
    mt_u32                        VPW;            /**<vertical sync pluse width*//**<CNcomment:垂直脉冲宽度*/
    mt_u32                        HPW;            /**<horizonal sync pluse width*/ /**<CNcomment:水平脉冲宽度*/
    MT_BOOL                       IDV;            /**< flag of data valid signal is needed flip*//**<CNcomment:有效数据信号是否翻转*/
    MT_BOOL                       IHS;            /**<flag of horizonal sync pluse is needed flip*//**<CNcomment:水平同步脉冲信号是否翻转*/
    MT_BOOL                       IVS;            /**<flag of vertical sync pluse is needed flip*//**<CNcomment:垂直同步脉冲信号是否翻转*/
    MT_BOOL                       ClockReversal;  /**<flag of clock is needed flip*//**<CNcomment:时钟是否翻转*/
    MT_UNF_DISP_INTF_DATA_WIDTH_E DataWidth;      /**<data width*/ /**<CNcomment:数据位宽*/
    MT_UNF_DISP_INTF_DATA_FMT_E   ItfFormat;      /**<data format.*//**<CNcomment:数据格式.*/

    MT_BOOL DitherEnable;                           /**< flag of is enable Dither*//**<CNcomment:数据格式.*/
    mt_u32  ClkPara0;                               /**<PLL  register SC_VPLL1FREQCTRL0  value *//**<CNcomment:PLL  SC_VPLL1FREQCTRL0  寄存器*/
    mt_u32  ClkPara1;                               /**<PLL  register SC_VPLL1FREQCTRL1 value*//**<CNcomment:PLL   SC_VPLL1FREQCTRL1寄存器*/

    MT_BOOL bInterlace;                             /**<progressive or interlace*//**<CNcomment:逐行或者隔行*/
    mt_u32  PixFreq;                                /**<pixel clock*//**<CNcomment:像素时钟*/
    mt_u32  VertFreq;                               /**<display rate*//**<CNcomment:刷新率*/
    mt_u32  AspectRatioW;                           /**<width of screen*//**<CNcomment:屏幕宽度*/
    mt_u32  AspectRatioH;                           /**<height of screen*//**<CNcomment:屏幕高度*/
    MT_BOOL bUseGamma;                              /**<gamma modulation*//**<CNcomment:伽马调节*/
    mt_u32  Reserve0;                               /**<reserved byte*//**<CNcomment:保留位*/
    mt_u32  Reserve1;                               /**<reserved byte*//**<CNcomment:保留位*/
} MT_UNF_DISP_TIMING_S;

/**define video and graphics layers */
/**CNcomment:视频和图形层*/
typedef enum mtUNF_DISP_LAYER_E
{
    MT_UNF_DISP_LAYER_VIDEO = 0,                   /**<video layer *//**<CNcomment:视频层*/
    MT_UNF_DISP_LAYER_GFX,                  /**<graphics layer *//**<CNcomment:图形层*/
    MT_UNF_DISP_LAYER_BUTT
}MT_UNF_DISP_LAYER_E;

/**bt1120 interface ID */
/**CNcomment:bt1120 接口ID  */
typedef enum mtUNF_DISP_BT1120_E
{
    MT_UNF_DISP_BT1120_0,    /**<BT1120 interface 0 *//**<CNcomment:BT1120接口0 */
    MT_UNF_DISP_BT1120_BUTT,
}MT_UNF_DISP_BT1120_E;

/**BT656 interface ID */
/**CNcomment:BT656 接口ID  */
typedef enum mtUNF_DISP_BT656_E
{
    MT_UNF_DISP_BT656_0,    /**<BT656 interface 0 *//**<CNcomment:BT656接口0 */
    MT_UNF_DISP_BT656_BUTT,
}MT_UNF_DISP_BT656_E;

/**LCD interface ID */
/**CNcomment:LCD 接口ID  */
typedef enum mtUNF_LCD_E
{
    MT_UNF_DISP_LCD_0,    /**<LCD interface 0 *//**<CNcomment:LCD接口0 */
    MT_UNF_DISP_LCD_BUTT,
}MT_UNF_DISP_LCD_E;

/**define the type of interface*/
/**CNcomment:定义接口类型*/
typedef enum  mtUNF_DISP_INTF_TYPE_E
{
    MT_UNF_DISP_INTF_TYPE_HDMI,     /**<HDMI interface type *//**<CNcomment:HDMI接口类型*/
    MT_UNF_DISP_INTF_TYPE_LCD,      /**<LCD interface type *//**<CNcomment:LCD接口类型*/
    MT_UNF_DISP_INTF_TYPE_BT1120,   /**<bt1120 digital interface type *//**<CNcomment:BT1120数字接口类型*/
    MT_UNF_DISP_INTF_TYPE_BT656,    /**<bt656 digital interface type*//**<CNcomment:BT656数字接口类型*/
    MT_UNF_DISP_INTF_TYPE_YPBPR,    /**<YPBPR interface type*//**<CNcomment:YPBPR接口类型*/
    MT_UNF_DISP_INTF_TYPE_RGB,      /**<RGB interface type*//**<CNcomment:RGB接口类型*/
    MT_UNF_DISP_INTF_TYPE_CVBS,     /**<CVBS interface type*//**<CNcomment:CVBS接口类型*/
    MT_UNF_DISP_INTF_TYPE_SVIDEO,   /**<SVIDEO interface type*//**<CNcomment:SVIDEO接口类型*/
    MT_UNF_DISP_INTF_TYPE_VGA,      /**<VGA interface type*//**<CNcomment:VGA接口类型*/
    MT_UNF_DISP_INTF_TYPE_BUTT
}MT_UNF_DISP_INTF_TYPE_E;

/*!
@~english define the sigal type of DAC output
@~chinese 定义支持的DAC信号组合
*/
typedef enum 
{
    MT_UNF_DISP_DAC_CVBS_RGB = 0,           //CVBS + RGB输出,dac0:cvbs, dac1:sd G, dac2:sd B, dac3:sd R
    MT_UNF_DISP_DAC_SING_CVBS,           //!<@~english dac0:cvbs, dac1~dac3:off @~chinese dac0:cvbs, dac1~dac3:关闭
    MT_UNF_DISP_DAC_SING_CVBS_LOW_POWER,           //!<@~english dac3:cvbs, dac0~dac2:off @~chinese dac3:cvbs, dac0~dac2:关闭
    MT_UNF_DISP_DAC_DULE_CVBS,        //!<@~english dac1:cvbs, dac0~dac2:off, dac3:cvbs @~chinese dac1:cvbs, dac0~dac2:off, dac3:cvbs
    MT_UNF_DISP_DAC_CVBS_SVIDEO,  //CVBS + S_VIDEO, dac0:cvbs, dac1~2:s-video, dac3:off
    MT_UNF_DISP_DAC_SVIDEO_CVBS,  //S_VIDEO + CVBS, dac0:off, dac1~2:s-video, dac3:cvbs
    MT_UNF_DISP_DAC_DULE_CVBS_SVIDEO,  //S_VIDEO + 2CVBS, dac0:cvbs, dac1~2:s-video, dac3:cvbs
    MT_UNF_DISP_DAC_CVBS_YPBPR_SD,  //CVBS + sd YPBPR输出，dac0:cvbs, dac1:sd y, dac2:sd pb, dac3:sd pr
    MT_UNF_DISP_DAC_CVBS_YPBPR_HD,  //CVBS + hd YPBPR输出，dac0:cvbs, dac1:hd y, dac2:hd pb, dac3:hd pr
    MT_UNF_DISP_DAC_TYPE_BUTT
}MT_UNF_DISP_VDAC_TYPE_E;

/**define the YPBPR type struct of interface*/
/**CNcomment:定义YPBPR接口结构*/
typedef struct  mtUNF_DISP_INTF_YPBPR_S
{
    mt_u8 u8DacY;             /**<DAC num of Y  *//**<CNcomment:Y分量dac端口号*/
    mt_u8 u8DacPb;            /**<DAC num of Pb  *//**<CNcomment:Pb分量dac端口号*/
    mt_u8 u8DacPr;            /**<DAC num of Pr  *//**<CNcomment:Pr分量dac端口号*/
}MT_UNF_DISP_INTF_YPBPR_S;

/**define the RGB type struct of interface*/
/**CNcomment:定义RGB接口结构*/
typedef struct  mtUNF_DISP_INTF_RGB_S
{
    mt_u8  u8DacR;            /**<DAC num of R  *//**<CNcomment:R分量dac端口号*/
    mt_u8  u8DacG;            /**<DAC num of G  *//**<CNcomment:G分量dac端口号*/
    mt_u8  u8DacB;            /**<DAC num of B  *//**<CNcomment:B分量dac端口号*/
    MT_BOOL bDacSync;         /**<G without sync signal  *//**<CNcomment:G分量不带同步信号*/
}MT_UNF_DISP_INTF_RGB_S;

/**define the VGA type struct of interface*/
/**CNcomment:定义VGA接口结构*/
typedef struct  mtUNF_DISP_INTF_VGA_S
{
    mt_u8  u8DacR;            /**<DAC num of R  *//**<CNcomment:R分量dac端口号*/
    mt_u8  u8DacG;            /**<DAC num of G  *//**<CNcomment:G分量dac端口号*/
    mt_u8  u8DacB;            /**<DAC num of B  *//**<CNcomment:B分量dac端口号*/
}MT_UNF_DISP_INTF_VGA_S;

/**define the CVBS type struct of interface*/
/**CNcomment:定义CVBS接口结构*/
typedef struct  mtUNF_DISP_INTF_CVBS_S
{
    mt_u8 u8Dac;              /**<DAC num of CVBS  *//**<CNcomment:CVBS端子dac端口号*/
}MT_UNF_DISP_INTF_CVBS_S;

/**define the SVIDEO type struct of interface*/
/**CNcomment:定义SVIDEO接口结构*/
typedef struct  mtUNF_DISP_INTF_SVIDEO_S
{
    mt_u8 u8DacY;             /**<DAC num of Y  *//**<CNcomment:Y分量dac端口号*/
    mt_u8 u8DacC;             /**<DAC num of C   *//**<CNcomment:C分量dac端口号*/
}MT_UNF_DISP_INTF_SVIDEO_S;

/**define display interface struct*/
/**CNcomment:定义显示接口结构*/
typedef struct  mtUNF_DISP_INTF_S
{
    MT_UNF_DISP_INTF_TYPE_E enIntfType;         /**<interface type *//**<CNcomment:接口类型*/
    MT_UNF_DISP_VDAC_TYPE_E eDacMode;
    union
    {
        MT_UNF_HDMI_ID_E        enHdmi;         /**<hdmi id *//**<CNcomment:HDMI 序号*/
        MT_UNF_DISP_BT1120_E    enBT1120;       /**<bt1120 id *//**<CNcomment:BT1120序号*/
        MT_UNF_DISP_BT656_E     enBT656;        /**<bt656 id *//**<CNcomment:BT656序号*/
        MT_UNF_DISP_LCD_E       enLcd;          /**<lcd id *//**<CNcomment:LCD序号*/

        MT_UNF_DISP_INTF_YPBPR_S    stYPbPr;    /**<intf config of YPBPR  *//**<CNcomment:YPBPR接口配置*/
        MT_UNF_DISP_INTF_RGB_S      stRGB;      /**<intf config of RGB  *//**<CNcomment:RGB接口配置*/
        MT_UNF_DISP_INTF_VGA_S      stVGA;      /**<intf config of VGA  *//**<CNcomment:VGA接口配置*/
        MT_UNF_DISP_INTF_CVBS_S     stCVBS;     /**<intf config of CVBS  *//**<CNcomment:CVBS接口配置*/
        MT_UNF_DISP_INTF_SVIDEO_S   stSVideo;   /**<intf config of SVIDEO  *//**<CNcomment:SVIDEO接口配置*/
    }unIntf;
}MT_UNF_DISP_INTF_S;


/**Defines the device aspect ratio.*/
/**CNcomment: 定义设备宽高比枚举*/
typedef enum mtUNF_DISP_ASPECT_RATIO_E
{
    MT_UNF_DISP_ASPECT_RATIO_AUTO,              /**<aspect ratio as device Resolution*//**<CNcomment: 宽高比与设备分辨率一致*/
    MT_UNF_DISP_ASPECT_RATIO_4TO3,              /**<4:3*//**<CNcomment: 4比3*/
    MT_UNF_DISP_ASPECT_RATIO_16TO9,             /**<16:9*//**<CNcomment: 16比9*/
    MT_UNF_DISP_ASPECT_RATIO_1TO1,            /**<1:1*//**<CNcomment: 1比1*/
    MT_UNF_DISP_ASPECT_RATIO_USER,              /**<user define*//**<CNcomment: 用户定义*/

    MT_UNF_DISP_ASPECT_RATIO_BUTT
}MT_UNF_DISP_ASPECT_RATIO_E;

/**Defines the device aspect ratio struct.*/
/**CNcomment: 定义设备宽高比结构*/
typedef struct mtUNF_DISP_ASPECT_RATIO_S
{
    MT_UNF_DISP_ASPECT_RATIO_E enDispAspectRatio;   /**<aspect ratio type of device*//**<CNcomment: 设备宽高比类型*/
    mt_u32                     u32UserAspectWidth;  /**<user define width of device*//**<CNcomment: 用户定义设备宽度*/
    mt_u32                     u32UserAspectHeight; /**<user define height of device*//**<CNcomment: 用户定义设备高度*/
}MT_UNF_DISP_ASPECT_RATIO_S;

/**Defines algorithmic  control struct of display device.*/
/**CNcomment: 定义显示设备算法控制结构*/
typedef struct mtUNF_DISP_ALG_CFG_S
{
    MT_BOOL bAccEnable;                             /**<acc alg*//**<CNcomment: acc算法*/
    MT_BOOL bSharpEnable;                           /**<sharp alg*//**<CNcomment: sharp算法*/
}MT_UNF_DISP_ALG_CFG_S;

/**Defines VBI type.*/
/**CNcomment: 定义VBI数据类型*/

typedef enum mtUNF_DISP_VBI_TYPE_E
{
    MT_UNF_DISP_VBI_TYPE_TTX = 0,                   /**<teltext type*//**<CNcomment:teltext类型*/
    MT_UNF_DISP_VBI_TYPE_CC,                        /**<closed caption type*//**<CNcomment: 隐藏字幕类型*/
    MT_UNF_DISP_VBI_TYPE_VCHIP,                     /**<v-chip type*//**<CNcomment: v-chip类型*/
    MT_UNF_DISP_VBI_TYPE_WSS,                       /**<wide screen signal*//**<CNcomment:宽屏信令*/
    MT_UNF_DISP_VBI_TYPE_VPS,                       /**<video programme system*//**<CNcomment:视频节目系统*/   
    MT_UNF_DISP_VBI_TYPE_CGMS_A,                       /**<CGMS*//**<CNcomment:复制代次管理系统*/ 
    MT_UNF_DISP_VBI_TYPE_CC_PES,                     /**<closed caption type*//**<CNcomment: 隐藏字幕类型by pes to sdvenc*/
    MT_UNF_DISP_VBI_TYPE_TTX_ES,
    MT_UNF_DISP_VBI_TYPE_BUTT,
} MT_UNF_DISP_VBI_TYPE_E;

/**Defines VBI config struct.*/
/**CNcomment: 定义VBI配置结构*/
typedef struct mtUNF_DISP_VBI_CFG_S
{
    MT_UNF_DISP_VBI_TYPE_E  enType;                  /**<VBI type*//**<CNcomment: VBI 类型*/
    mt_u32                  u32InBufferSize;        /**<VBI data(pes) buffer size,more than 4K,suggest 4K*//**<CNcomment:VBI pes 数据输入缓冲大小,不小于4K，推荐值4K*/
    mt_u32                  u32WorkBufferSize;      /**<VBI data buffer size used in driver,more than 2K,suggest 2K*//**<CNcomment:VBI解析后数据在驱动中接收缓冲大小不小于2K，推荐值2K*/
} MT_UNF_DISP_VBI_CFG_S;

/** define VBI information structure*/
/**CNcomment: 定义VBI信息数据结构*/
typedef struct mtUNF_DISP_VBI_DATA_S
{
    MT_UNF_DISP_VBI_TYPE_E  enType;                  /**<VBI type*//**<CNcomment: VBI 类型*/
    mt_u8                   *pu8DataAddr;           /**<Vbi data buffer virtual address*//**<CNcomment:Vbi数据用户虚拟地址*/
    mt_u32                  u32DataLen;             /**<Vbi data lenght*//**<CNcomment:Vbi数据长度*/
} MT_UNF_DISP_VBI_DATA_S;

/**Defines the MAX buffer number.*/
/** CNcomment:定义最大的buffer 分配数*/
#define MT_DISP_CAST_BUFFER_MAX_NUMBER ( 16 )

/**define CAST config */
/**CNcomment:定义屏幕投影配置*/
typedef struct mtUNF_DISP_CAST_ATTR_S
{
    MT_UNF_VIDEO_FORMAT_E enFormat;                                        /**<the output video format.*//**<CNcomment:定义输出视频格式*/
    mt_u32                u32Width;                                       /**<the output video width.*//**<CNcomment:定义输出视频的宽*/
    mt_u32                u32Height;                                      /**<the output video height.*//**<CNcomment:定义输出视频的高*/
    MT_BOOL               bLowDelay;                                      /**<work at low delay mode.*//**<CNcomment:启用低延迟模式*/
    mt_u32                u32BufNum;                                      /**<the buffer number.*//**<CNcomment:定义分配buffer 数*/
    MT_BOOL               bUserAlloc;                                     /**<whether is user alloc memory*//**<CNcomment:定义是否用户分配内存*/
    mt_u32                u32BufSize;                                     /**<each the buffer size.*//**<CNcomment:定义用户分配每个buffer 大小*/
    mt_u32                u32BufStride;                                   /**<the horizonal stride.*//**<CNcomment:定义行对齐*/
    mt_u32                u32BufPhyAddr[MT_DISP_CAST_BUFFER_MAX_NUMBER];  /**<the buffer physics address.*//**<CNcomment:分配内存buffer 成员的物理地址*/
    MT_BOOL               bCrop;                                          /**<whether enable crop .*//**<CNcomment:定义是否进行crop*/
    MT_UNF_CROP_RECT_S    stCropRect;                                     /**<the crop wise .*//**<CNcomment:定义crop 范围*/
} MT_UNF_DISP_CAST_ATTR_S;

/**define color setting */
/**CNcomment:定义色彩设置*/
typedef struct mtUNF_DISP_COLOR_SETTING_S
{
    MT_BOOL bGammaEnable;         /**<whether Gamma enable.*//**<CNcomment:定义Gamma 使能*/
    MT_BOOL bUseCustGammaTable;   /**<whether use custom GammaTable.*//**<CNcomment:是否使用用户Gamma表*/
    MT_BOOL bColorCorrectEnable;  /**<whether Color Correct.*//**<CNcomment:是否色彩校正*/
    mt_s32  s32ColorTemp;         /**<color Temp.*//**<CNcomment:色温*/
    mt_u32  u32Reserve;           /**<Reserve.*//**<CNcomment:保留*/
    mt_void *pPrivate;            /**<Private.*//**<CNcomment:私有体指针*/
}MT_UNF_DISP_COLOR_SETTING_S;

/**define the struct about color */
/**CNcomment:定义显示颜色的结构体 */
typedef struct  mtUNF_DISP_BG_COLOR_S
{
    mt_u8 u8Red;                  /**<red *//**<CNcomment:红色分量*/
    mt_u8 u8Green;                /**<green*//**<CNcomment:绿色分量*/
    mt_u8 u8Blue;                 /**<blue*//**<CNcomment:蓝色分量*/
} MT_UNF_DISP_BG_COLOR_S;

/********************************ENCODER STRUCT********************************/
/** define the enum of Macrovision output type*/
/** CNcomment:显示输出Macrovision模式枚举定义*/
typedef enum mtUNF_DISP_MACROVISION_MODE_E
{
    MT_UNF_DISP_MACROVISION_MODE_TYPE0,     /**<type 0 *//**<CNcomment:典型配置0 */
    MT_UNF_DISP_MACROVISION_MODE_TYPE1,     /**<type 1 *//**<CNcomment:典型配置1 */
    MT_UNF_DISP_MACROVISION_MODE_TYPE2,     /**<type 2 *//**<CNcomment:典型配置2 */
    MT_UNF_DISP_MACROVISION_MODE_TYPE3,     /**<type 3 *//**<CNcomment:典型配置3 */
    MT_UNF_DISP_MACROVISION_MODE_CUSTOM0,    /**<type of configure by user *//**<CNcomment:用户自定义配置 */
    MT_UNF_DISP_MACROVISION_MODE_CUSTOM1,    /**<type of configure by user *//**<CNcomment:用户自定义配置 */
    MT_UNF_DISP_MACROVISION_MODE_BUTT
} MT_UNF_DISP_MACROVISION_MODE_E;

/** CGMS type select */
/**CNcomment:CGMS 类型选择*/
typedef enum mtUNF_DISP_CGMS_TYPE_E
{
    MT_UNF_DISP_CGMS_TYPE_A = 0x00,     /**<CGMS type  A*//**<CNcomment:CGMS 类型A*/
    MT_UNF_DISP_CGMS_TYPE_B,            /**<CGMS type  B*//**<CNcomment:CGMS 类型B*/

    MT_UNF_DISP_CGMS_TYPE_BUTT
}MT_UNF_DISP_CGMS_TYPE_E;

/** definition of CGMS mode */
/**CNcomment:定义CGMS 模式*/
typedef enum mtUNF_DISP_CGMS_MODE_E
{
    MT_UNF_DISP_CGMS_MODE_COPY_FREELY  = 0,     /**<copying is permitted without restriction *//**<CNcomment:无限制拷贝*/
    MT_UNF_DISP_CGMS_MODE_COPY_NO_MORE = 0x01,  /**<No more copies are allowed (one generation copy has been made)*//**<CNcomment:拷贝一次后不允许再被拷贝*/
    MT_UNF_DISP_CGMS_MODE_COPY_ONCE    = 0x02,  /**<One generation of copies may be made *//**<CNcomment:仅允许拷贝一次*/
    MT_UNF_DISP_CGMS_MODE_COPY_NEVER   = 0x03,  /**<No copying is permitted *//**<CNcomment:不允许拷贝*/

    MT_UNF_DISP_CGMS_MODE_BUTT
}MT_UNF_DISP_CGMS_MODE_E;

/** definition of CGMS configuration */
typedef struct mtUNF_DISP_CGMS_CFG_S
{
    MT_BOOL                 bEnable;            /**<MT_TRUE:CGMS is enabled; MT_FALSE:CGMS is disabled *//**<CNcomment:CGMS 使能选项*/
    MT_UNF_DISP_CGMS_TYPE_E enType;             /**<type-A or type-B or None(BUTT) *//**<CNcomment:CGMS 类型*/
    MT_UNF_DISP_CGMS_MODE_E enMode;             /**<CGMS mode. *//**<CNcomment:CGMS 模式*/
}MT_UNF_DISP_CGMS_CFG_S;

/**define WSS information structure*/
/**CNcomment:定义图文信息数据结构 */
typedef struct mtUNF_DISP_WSS_DATA_S
{
    MT_BOOL bEnable;                            /**<WSS configure enable MT_TRUE: enable,MT_FALSE: disnable*//**<CNcomment:WSS配置使能。MT_TRUE：使能；MT_FALSE：禁止*/
    mt_u16  u16Data;                            /**<Wss data */ /**<CNcomment:Wss数据*/
}MT_UNF_DISP_WSS_DATA_S;

/**define display attribute stucture*/
/**CNcomment:定义显示属性结构体 */
typedef struct mtUNF_DISP_ATTR_S
{
    MT_UNF_ENC_FMT_E       enEncodingFormat;   /**<format of display device *//**<CNcomment:显示设备制式*/
    MT_UNF_DISP_TIMING_S   stLcdPara;          /**<lcd para *//**<CNcomment:LCD参数*/
    mt_u32                 u32Brightness;      /**<Brightness *//**<CNcomment:亮度*/
    mt_u32                 u32Contrast;        /**<Contrast *//**<CNcomment:对比度*/
    mt_u32                 u32Saturation;      /**<Saturation *//**<CNcomment:饱和度*/
    mt_u32                 u32HuePlus;         /**<HuePlus *//**<CNcomment:色调*/
    MT_BOOL                bGammaEnable;       /**<gamma *//**<CNcomment:伽马*/
    MT_UNF_DISP_BG_COLOR_S stBgColor;          /**<background clor *//**<CNcomment:背景色*/
    mt_void                *pRevData;          /**<reserved data *//**<CNcomment:保留数据*/
}MT_UNF_DISP_ATTR_S;

/**define display 3D mode stucture*/
/**CNcomment:定义显示模式结构体 */
typedef enum mtUNF_DISP_3D_E
{
    MT_UNF_DISP_3D_NONE = 0,
    MT_UNF_DISP_3D_FRAME_PACKING,                   /**<3d type:Frame Packing*//**<CNcomment:3d 模式:帧封装*/
    MT_UNF_DISP_3D_SIDE_BY_SIDE_HALF,               /**<3d type:Side by side half*//**<CNcomment:3d 模式:并排式 左右半边*/
    MT_UNF_DISP_3D_TOP_AND_BOTTOM,                  /**<3d type:Top and Bottom*//**<CNcomment:3d 模式:上下模式*/
    MT_UNF_DISP_3D_FIELD_ALTERNATIVE,               /**<3d type:Field alternative*//**<CNcomment:3d 模式:场交错*/
    MT_UNF_DISP_3D_LINE_ALTERNATIVE,                /**<3d type:Field alternative*//**<CNcomment:3d 模式:行交错*/
    MT_UNF_DISP_3D_SIDE_BY_SIDE_FULL,               /**<3d type:Side by side full*//**<CNcomment:3d 模式:并排式 左右全场*/
    MT_UNF_DISP_3D_L_DEPTH,                         /**<3d type:L+depth*//**<CNcomment:3d 模式:L+DEPTH*/
    MT_UNF_DISP_3D_L_DEPTH_GRAPHICS_GRAPHICS_DEPTH, /**<3d type:L+depth+Graphics+Graphics-depth*//**<CNcomment:3d 模式:L+depth+Graphics+Graphics-depth*/
    MT_UNF_DISP_3D_BUTT
}MT_UNF_DISP_3D_E;

/**define display margin stucture*/
/**CNcomment:定义显示空白区域结构体 */
typedef struct mtUNF_DISP_OFFSET_S
{
    mt_u32 u32Left;    /**<left offset *//**<CNcomment:左侧偏移*/
    mt_u32 u32Top;     /**<top offset *//**<CNcomment:上方偏移*/
    mt_u32 u32Right;   /**<right offset *//**<CNcomment:右侧偏移*/
    mt_u32 u32Bottom;  /**<bottom offset *//**<CNcomment:下方偏移*/
}MT_UNF_DISP_OFFSET_S;

/**define display PP mode stucture*/
/**CNcomment:定义显示效果结构体 */
typedef enum mtUNF_DISP_PP_E
{
    MT_UNF_DISP_PP_NONE = 0, 
    MT_UNF_DISP_PP_TEST, 
    MT_UNF_DISP_PP_STANDARD,                   /**<PP mode:standard*//**<CNcomment:pass AE freq response*/
    MT_UNF_DISP_PP_DEFAULT,                     /**<PP mode:default*//**<CNcomment:montage prefered, ,look like mstar*/
    MT_UNF_DISP_PP_VIVID,                         /**<PP mode:vivid*//**<CNcomment:more like ali*/
    MT_UNF_DISP_PP_BUTT                           
}MT_UNF_DISP_PP_E;

/**define vdac index stucture*/
/**CNcomment:定义vdac index 结构体 */
typedef enum mtUNF_VDAC_INDEX_E
{
    MT_UNF_VDAC_0 = 0, 
    MT_UNF_VDAC_1, 
    MT_UNF_VDAC_2,                  
    MT_UNF_VDAC_3,                     
    MT_UNF_VDAC_BUTT                           
}MT_UNF_VDAC_INDEX_E;

/**define unblank mode enum*/
/**CNcomment:定义开屏模式枚举 */
typedef enum mtUNF_DISP_UNBLANK_MODE_E
{
    MT_UNF_DISP_UNBLANK_MODE_USER = 0, 
    MT_UNF_DISP_UNBLANK_MODE_SYNC, 
    MT_UNF_DISP_UNBLANK_MODE_STABLE,                  
    MT_UNF_DISP_UNBLANK_MODE_FAST,                     
    MT_UNF_DISP_UNBLANK_MODE_BUTT
}MT_UNF_DISP_UNBLANK_MODE_E;

/**define TV HDMI mode*/
/**CNcomment:定义HDMI 模式*/
typedef enum mtUNF_DISP_HDMI_MODE_E
{
    MT_UNF_DISP_HDMI_MODE_SDR = 0,                 /**<hdmi mode: SDR*//**<CNcomment: hdmi 模式: SDR*/
    MT_UNF_DISP_HDMI_MODE_HDR10,                   /**<hdmi mode: HDR10*//**<CNcomment: hdmi 模式: HDR10*/
    MT_UNF_DISP_HDMI_MODE_HLG,                       /**<hdmi mode: HLG*//**<CNcomment:hdmi 模式: HLG*/
    MT_UNF_DISP_HDMI_MODE_AUTO,                     /**<hdmi mode: AUTO*//**<CNcomment:hdmi 模式: AUTO*/
    MT_UNF_DISP_HDMI_MODE_BUTT                           
}MT_UNF_DISP_HDMI_MODE_E;

/**define sd enc pq item registers*/
/**CNcomment:定义CVBS  指标相关寄存器配置条目*/
typedef enum mtUNF_DISP_SD_ENC_CFG_E
{
    MT_UNF_DISP_SD_ENC_MODE = 0,
    MT_UNF_DISP_SD_ENC_CURVE,
    MT_UNF_DISP_SD_ENC_DELAY,
    MT_UNF_DISP_SD_ENC_BLANK_P,
    MT_UNF_DISP_SD_ENC_BLANK_N,
    MT_UNF_DISP_SD_ENC_CBLANK,
    MT_UNF_DISP_SD_ENC_VDAC_PCARRY,
    MT_UNF_DISP_SD_ENC_VDAC_NCARRY,
    MT_UNF_DISP_SD_ENC_CFIG1,
    MT_UNF_DISP_SD_ENC_CFIG2,
    MT_UNF_DISP_SD_ENC_CFIG3,
    MT_UNF_DISP_SD_ENC_CFIG4,
    MT_UNF_DISP_SD_ENC_CFIG5,
    MT_UNF_DISP_SD_ENC_CFIG6,
    MT_UNF_DISP_SD_ENC_CFIG7,
    MT_UNF_DISP_SD_ENC_SET,
    MT_UNF_DISP_SD_ENC_DAC0_PARA,
    MT_UNF_DISP_SD_ENC_DRCOEF,
    MT_UNF_DISP_SD_ENC_DBCOEF,
    MT_UNF_DISP_SD_ENC_CFIG8,
    MT_UNF_DISP_SD_ENC_SCARRYDR,
    MT_UNF_DISP_SD_ENC_SCARRYDB,
    MT_UNF_DISP_SD_ENC_PINCREMENT,
    MT_UNF_DISP_SD_ENC_NINCREMENT,
    MT_UNF_DISP_SD_ENC_GCONTROL,
    MT_UNF_DISP_SD_ENC_DACNUM,
    MT_UNF_DISP_SD_ENC_INSTM,
    MT_UNF_DISP_SD_ENC_COMPRESS,
    MT_UNF_DISP_SD_ENC_SET1,
    MT_UNF_DISP_SD_ENC_SET2,
    MT_UNF_DISP_SD_ENC_SET3,
    MT_UNF_DISP_SD_ENC_LUM_DLY_108M,
    MT_UNF_DISP_SD_ENC_COEF11,
    MT_UNF_DISP_SD_ENC_COEF10,
    MT_UNF_DISP_SD_ENC_COEF9,
    MT_UNF_DISP_SD_ENC_COEF8,
    MT_UNF_DISP_SD_ENC_COEF7,
    MT_UNF_DISP_SD_ENC_COEF6,
    MT_UNF_DISP_SD_ENC_COEF5,
    MT_UNF_DISP_SD_ENC_COEF4,
    MT_UNF_DISP_SD_ENC_COEF3,
    MT_UNF_DISP_SD_ENC_COEF2,
    MT_UNF_DISP_SD_ENC_COEF1,
    MT_UNF_DISP_SD_ENC_COEF0,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT3_U,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT2_U,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT1_U,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT0_U,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT3_V,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT2_V,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT1_V,
    MT_UNF_DISP_SD_ENC_COEF_C_LUT0_V,
    MT_UNF_DISP_SD_ENC_COEF_LUT3_Y,
    MT_UNF_DISP_SD_ENC_COEF_LUT2_Y,
    MT_UNF_DISP_SD_ENC_COEF_LUT1_Y,
    MT_UNF_DISP_SD_ENC_COEF_LUT0_Y,
    MT_UNF_DISP_SD_ENC_SIGN_CTL,
    MT_UNF_DISP_SD_ENC_DAC_OFFSET,
    MT_UNF_DISP_SD_ENC_DAC123_PARA,
    MT_UNF_DISP_SD_ENC_DAC0_ANTI_COEF1_0,
    MT_UNF_DISP_SD_ENC_DAC0_ANTI_COEF3_2,
    MT_UNF_DISP_SD_ENC_DAC123_ANTI_COEF1_0,
    MT_UNF_DISP_SD_ENC_DAC123_ANTI_COEF3_2,
    
    MT_UNF_DISP_SD_ENC_BUTT
}MT_UNF_DISP_SD_ENC_CFG_E;

/**define sd enc picture quality parameter stucture*/
/**CNcomment:定义cvbs 指标参数结构体*/
typedef struct mtUNF_SD_ENC_PQ_PARA_S
{
	mt_u32		item;
	mt_u32		val;
}MT_UNF_SD_ENC_PQ_PARA_S;

/**define dump scaler source enum*/
/**CNcomment:定义dump scaler源枚举*/
typedef enum mtUNF_DISP_DUMP_SCALER_SOURCE_E
{
    MT_UNF_DISP_DSCALER_IN_HD_SCREEN = 0,
    MT_UNF_DISP_DSCALER_IN_HD_VIDEO,
    MT_UNF_DISP_DSCALER_IN_SD_SCREEN,
    MT_UNF_DISP_DSCALER_IN_BUTT
}MT_UNF_DISP_DUMP_SCALER_SOURCE_E;

/**define scaler mode enum*/
/**CNcomment:定义scaler模式枚举*/
typedef enum mtUNF_DISP_SCALER_MODE_E
{
    MT_UNF_DISP_SCALER_MODE_FORCE_SD_DISABLE = 0,
    MT_UNF_DISP_SCALER_MODE_FORCE_SD_ENABLE,
    MT_UNF_DISP_SCALER_MODE_FORCE_AUTO,
    MT_UNF_DISP_SCALER_MODE_BUTT
}MT_UNF_DISP_SCALER_MODE_E;
    

typedef enum mtUNF_DISP_LAYER_ID_E
{
    /*!
      background layer
      */
    MT_UNF_DISP_LAYER_ID_BACKGROUND = 0,
    /*!
      still layer SD 1
      */
    MT_UNF_DISP_LAYER_ID_STILL_SD,  
    /*!
      still layer HD 2
      */
    MT_UNF_DISP_LAYER_ID_STILL_HD,  
    /*!
      video layer SD 3
      */
    MT_UNF_DISP_LAYER_ID_VIDEO_SD,
    /*!
      video layer HD 4
      */
    MT_UNF_DISP_LAYER_ID_VIDEO_HD,
    /*!
      osd layer 0  5
      */
    MT_UNF_DISP_LAYER_ID_OSD0,
    /*!
      osd layer 1  6
      */
    MT_UNF_DISP_LAYER_ID_OSD1,
    /*!
      sub layer 7
      */
    MT_UNF_DISP_LAYER_ID_SUBTITL,
    /*!
      max 
      */
    MT_UNF_DISP_LAYER_ID_MAX
} MT_UNF_DISP_LAYER_ID_E;


/**define dump scaler parameter stucture*/
/**CNcomment:定义dump scaler参数结构体*/
typedef struct mtUNF_DISP_DUMP_SCALER_PARA_S
{
    MT_BOOL                             b_enable;
    MT_UNF_DISP_DUMP_SCALER_SOURCE_E    source;
    MT_UNF_DISP_LAYER_ID_E              dst_layer;
    mt_u32		                        out_width;
    mt_u32		                        out_height;
}MT_UNF_DISP_DUMP_SCALER_PARA_S;


/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      DISPLAY */
/** @{ */  /** <!-- [DISPLAY] */


/**
   \brief Initiallization DISP module.CNcomment:初始化DISP模块 CNend
   \attention \n
   Please call this API function, before call anyother API of DISP module.
   CNcomment:调用DISP模块其它接口前要求首先调用本接口 CNend
   \param  none.CNcomment:无 CNend
   \retval ::MT_SUCCESS  operation success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_DEV_NOT_EXIST DISP device not exist.CNcomment:设备不存在 CNend
   \retval ::MT_ERR_DISP_NOT_DEV_FILE  DISP  not device file .CNcomment:非设备 CNend
   \retval ::MT_ERR_DISP_DEV_OPEN_ERR  DISP  open fail.CNcomment:打开失败 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Init( mt_void );


/**
   \brief deinital.CNcomment:去初始化DISP模块 CNend
   \attention \n
   please call API MT_UNF_DISP_Close and open all the DISP device, before call this API.
   CNcomment:在调用::MT_UNF_DISP_Close接口关闭所有打开的DISP后调用本接口 CNend
   \param none.CNcomment:无 CNend
   \retval ::MT_SUCCESS success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_DEV_CLOSE_ERR  DISP close fail.CNcomment:关闭失败 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_DeInit( mt_void );


/**
   \brief Attach one DISP channel to anoter.CNcomment:将两个DISP通道绑定 CNend
   \attention \n
   Please finish the attach operation before the DISP channel has been open, and currently we only support HD channel attach to SD channel.
   CNcomment:目前支持将高清DISP绑定到标清DISP通道上，绑定在打开DISP通道前必须完成绑定操作。 CNend
   \param[in] enDstDisp   Destination DISP channel.CNcomment:目标DISP通道号 CNend
   \param[in] enSrcDisp   source DISP channel.CNcomment:  源DISP通道号 CNend
   \retval ::MT_SUCCESS  operation success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT  display not be initialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA  invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT   invalid opeation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Attach( MT_UNF_DISP_E enDstDisp, MT_UNF_DISP_E enSrcDisp );


/**
   \brief dettach DISP.CNcomment:将两个DISP通道解绑定 CNend
   \attention \n
   should  close the DISP channels, before do detach operation.
   CNcomment:只有在关闭两个DISP通道后才能进行解绑定操作。 CNend
   \param[in] enDstDisp   Destination DISP channel.CNcomment:目标DISP通道号 CNend
   \param[in] enSrcDisp    source DISP channel.CNcomment: 源DISP通道号 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT   DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA  invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT   invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Detach( MT_UNF_DISP_E enDstDisp, MT_UNF_DISP_E enSrcDisp );


/**
   \brief open DISP channel.CNcomment:打开指定DISP通道 CNend
   \attention \n
   Please config the valid parameters before open DISP.
   CNcomment:在打开DISP之前，先完成对其的参数设置，避免画面闪烁 CNend
   \param[in] enDisp   DISP channel ID, please reference the define of MT_UNF_DISP_E.CNcomment:DISP通道号，请参见::MT_UNF_DISP_E CNend
   \retval ::MT_SUCCESS success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT   DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA  invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_CREATE_ERR    DISP create fail.CNcomment:DISP创建失败 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Open( MT_UNF_DISP_E enDisp );


/**
   \brief  coase DISP channel.CNcomment:关闭指定DISP CNend
   \attention \n
   none.
   CNcomment:无 CNend
   \param[in] enDisp DISP channel ID.CNcomment:DISP通道号 CNend
   \retval ::MT_SUCCESS success. CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Close( MT_UNF_DISP_E enDisp );


/**
   \brief  set DISP interface parameter.CNcomment:设置DISP的接口参数 CNend
   \attention \n
   The API is uesed for set  interface attach .If the interface has been attached,must be detach it ,or return MT_ERR_DISP_INVALID_OPT;\n
   There are 4 DAC provided by SOC 3712 v300. suggest, DAC 0/1/2 used by HD channel, DAC3 used by SD channel CVBS. \n
   There are 6 DAC provided by SOC 3716 v100/v200/v300. suggest, DAC 0/1/2 used by HD channel, DAC3/4/5 used by SD channel.
   CNcomment:该API 用来设置输出接口的绑定关系，如果设置已被绑定过的接口，则需要先进行Detach操作\n
   解除绑定关系，否则返回错误MT_ERR_DISP_INVALID_OPT;\n
   3712 v300 芯片共有4个DAC，推荐0，1，2用作高清输出，3用作标清CVBS输出。\n
   3716 v100/v200/v300 芯片共有6个DAC，推荐0，1，2用作高清输出，3，4，5用作标清输出。CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32IntfNum  DISP intf num of DISP channel.CNcomment:DISP通道上的接口数目 CNend
   \param[in] pstIntf  DISP intf para.CNcomment:DISP接口参数 CNend
   \retval ::MT_SUCCESS   success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT   DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR      Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA  invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT   invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_AttachIntf( MT_UNF_DISP_E enDisp, MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum );


/**
   \brief cancel DISP interface parameter.CNcomment:取消DISP的接口参数 CNend
   \attention \n
   The API is uesed to  detach  interface.
   CNcomment:这个API 用来解除接口的绑定关系。 CNend
   \param[in] enDisp        DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32IntfNum  DISP interface number of DISP channel.CNcomment:DISP通道上的接口数目 CNend
   \param[in] pstIntf  DISP interface para.CNcomment:DISP接口参数 CNend
   \retval ::MT_SUCCESS   success. CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT   DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR        Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA  invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT   invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_DetachIntf( MT_UNF_DISP_E enDisp, MT_UNF_DISP_INTF_S *pstIntf, mt_u32 u32IntfNum );


/**
   \brief set DISP sd enc picture quality parameters.CNcomment:设置CVBS 指标参数 CNend
   \attention \n
   only support DISP SD channel. \n
   CNcomment: CVBS 指标参数只针对SD channel设置          CNend
   \param[in] enDisp               DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstPara    pq paras array.CNcomment: 指标参数数组 CNend
   \param[in] cnt        number of items in array.CNcomment:指标参数个数 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetSdEncPqParams(MT_UNF_DISP_E enDisp, MT_UNF_SD_ENC_PQ_PARA_S *pstPara, mt_u32 cnt);

/**
   \brief set DISP output format.CNcomment:设置DISP的制式 CNend
   \attention \n
   for HD DISP channel please set HD display format, and for SD DISP channel please set SD display format.\n
   for scenario of HD,SD use same source, only support format which frame rate is 50Hz or 60Hz.
   CNcomment:对高清的DISP，只能设置高清的制式；对标清的DISP，只能设置标清的制式。\n
   在同源显示场景下，暂不支持刷新率非50Hz/60Hz的制式，如：暂不支持1080P24/1080P25/1080P30 CNend
   \param[in] enDisp               DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] enEncodingFormat    DISP format.CNcomment:DISP的制式 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetFormat( MT_UNF_DISP_E enDisp, MT_UNF_ENC_FMT_E enEncodingFormat );


/**
   \brief get DISP format.CNcomment:获取DISP的制式 CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp               DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] penEncodingFormat    poiner of DISP format.DCNcomment:ISP的制式指针 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetFormat( MT_UNF_DISP_E enDisp, MT_UNF_ENC_FMT_E *penEncodingFormat );


/**
   \brief set user define LCD clock parameter.CNcomment:设置用户定义的LCD时序参数 CNend
   \attention \n
   only 3716 v100 v200 is  supported LCD.
   CNcomment:只有3716 v100 v200 支持LCD CNend
   \param[in] enDisp               DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstTiming    poiner of DISP format.CNcomment:LCD时序参数 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetCustomTiming( MT_UNF_DISP_E enDisp, MT_UNF_DISP_TIMING_S *pstTiming );


/**
   \brief  get LCD parameter which had been config.CNcomment:获取DISP的LCD参数 CNend
   \attention \n
   only 3716 v100 v200 is  supported LCD.
   CNcomment:只有3716 v100 v200 支持LCD CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstTiming      pointer of LCD paramter.CNcomment:指针类型，LCD参数 CNend
   \retval ::MT_SUCCESS success.CNcomment: 成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetCustomTiming( MT_UNF_DISP_E enDisp, MT_UNF_DISP_TIMING_S *pstTiming );


/**
   \brief   set DISP layers Z order.CNcomment:设置DISP上叠加层的Z序 CNend
   \param[in] enDisp          DISP channel ID.CNcomment: DISP通道号 CNend
   \param[in] enZFlag        the way of overlay.CNcomment:Z序调节方式 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetLayerZorder( MT_UNF_DISP_E enDisp, MT_LAYER_ZORDER_ABS_E enZFlag );


/**
   \brief  get DISP overlay z order.CNcomment:获取DISP上叠加层的Z序 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.
   CNcomment:在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp            DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pu32Zorder        CNcomment:OSD layer oder CNend
   \retval ::MT_SUCCESS success.CNcomment: 成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation.CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetLayerZorder( MT_UNF_DISP_E enDisp, mt_u32 *pu32Zorder );


/**
   \brief set DISP back ground color.CNcomment:设置DISP背景色 CNend
   \attention \n
   none.
   CNcomment:无 CNend
   \param[in] enDisp       CNcomment:DISP通道号 CNend
   \param[in] pstBgColor   CNcomment:指针类型，待配置的显示输出背景色。请参见::MT_UNF_BG_COLOR_S CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetBgColor( MT_UNF_DISP_E enDisp, const MT_UNF_DISP_BG_COLOR_S *pstBgColor );


/**
   \brief  set DISP back ground color.CNcomment:获取DISP背景色 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.
   CNcomment:在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pstBgColor   pointer of back ground color.CNcomment: 指针类型，显示输出背景色 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetBgColor( MT_UNF_DISP_E enDisp, MT_UNF_DISP_BG_COLOR_S *pstBgColor );


/**
   \brief  set DISP brightness.CNcomment:设置DISP亮度 CNend
   \attention \n
   if the value seted more than 100, we clip it to 100.CNcomment:大于100的值按100处理 CNend
   \param[in] enDisp            DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Brightness   brightness value. the range is 0~100, 0 means the min brightness value.
   CNcomment:待设置的显示输出亮度值。取值范围为0～100。0：最小亮度；100：最大亮度 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetBrightness( MT_UNF_DISP_E enDisp, mt_u32 u32Brightness );


/**
   \brief   get the DISP brightness.CNcomment:获取DISP亮度 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.
   CNcomment:查询的默认亮度值为50。\n
   在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp              DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pu32Brightness    pointer of brightness. CNcomment:指针类型，显示输出亮度值 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetBrightness( MT_UNF_DISP_E enDisp, mt_u32 *pu32Brightness );


/**
   \brief  set DISP contrast value.CNcomment:设置DISP对比度 CNend
   \attention \n
   If the value is more than 100, we clip it to 100.
   CNcomment:大于100的值按100处理 CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Contrast    contrast value. the range is 0~100, 0 means the min contrast value. \n
   CNcomment:待设置的显示输出对比度值。取值范围为0～100。0：最小对比度；100：最大对比度 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetContrast( MT_UNF_DISP_E enDisp, mt_u32 u32Contrast );


/**
   \brief  get DISP contrast value.CNcomment:获取DISP对比度 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.\n
   CNcomment:查询的默认对比度值为50。\n
   在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pu32Contrast   pointer of contrast. CNcomment:指针类型，显示输出对比度值 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetContrast( MT_UNF_DISP_E enDisp, mt_u32 *pu32Contrast );


/**
   \brief   set DISP saturation.CNcomment:设置DISP饱和度 CNend
   \attention \n
   If the value is more than 100, we clip it to 100.
   CNcomment:大于100的值按100处理 CNend
   \param[in] enDisp             DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Saturation      saturation value. the range is 0~100, 0 means the min saturation value. \n
   CNcomment:待设置的显示输出饱和度值。取值范围为0～100。0：最小饱和度；100：最大饱和度 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
*/
mt_s32 MT_UNF_DISP_SetSaturation( MT_UNF_DISP_E enDisp, mt_u32 u32Saturation );


/**
   \brief   get DISP saturation.CNcomment:获取DISP饱和度 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.\n
   CNcomment:查询的默认饱和度值为50。\n
   在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp                 DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pu32Saturation    pointer of saturation. CNcomment:指针类型，显示输出饱和度值 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetSaturation( MT_UNF_DISP_E enDisp, mt_u32 *pu32Saturation );


/**
   \brief  set DISP hueplus.CNcomment:设置DISP色调 CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp              DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32HuePlus       hueplus value. the range is 0~100, 0 means the min hueplus value. \n
   CNcomment:显示输出色调增益值。范围为0～100。0：表示最小色调增益；100：表示最大色调增益 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetHuePlus( MT_UNF_DISP_E enDisp, mt_u32 u32HuePlus );


/**
   \brief get DISP hueplus.CNcomment:获取DISP色调 CNend
   \attention \n
   In the case of HD atach to SD. the Z order configuration of HD will auto sync to SD, also the configuration of SD will auto sync to HD.\n
   CNcomment:查询的默认色调值为50。\n
   在高标清绑定情况下，高清DISP的设置会自动同步到标清DISP；标清DISP的设置会自动同步到高清DISP。 CNend
   \param[in] enDisp          DISP channel ID.CNcomment: DISP通道号 CNend
   \param[out] pu32HuePlus     pointer of hueplus. CNcomment:指针类型，显示输出色调增益值 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetHuePlus( MT_UNF_DISP_E enDisp, mt_u32 *pu32HuePlus );


/**
   \brief  set Wss.CNcomment:直接设置WSS（Wide Screen Singnalling）数据 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstWssData        pointer of Wss data description structure.CNcomment:指针类型，指向WSS数据的指针 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetWss( MT_UNF_DISP_E enDisp, const MT_UNF_DISP_WSS_DATA_S *pstWssData );


/**
   \brief  set Macrovision mode.CNcomment:设置Macrovision模式 CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] enMode        Macrovision mode.  CNcomment:Macrovision模式，请参见::MT_UNF_DISP_MACROVISION_MODE_E CNend
   \param[in] pData          pointer of Macrovision control data CNcomment: 指针类型，自定义的Macrovision控制数据 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetMacrovision( MT_UNF_DISP_E enDisp, MT_UNF_DISP_MACROVISION_MODE_E enMode, const mt_void *pData );


/**
   \brief  set Macrovision nubmered register data.CNcomment:设置Macrovision 编号寄存器CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] index         index of macrovision reg_number to set.  CNcomment: Macrovision 寄存器编号1~21 CNend
   \param[in] data          Macrovision control data CNcomment: Macrovision 寄存器数据 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetMacrovisionRegN(MT_UNF_DISP_E enDisp, mt_u32 index, mt_u32 data);


/**
   \brief  get Macrovision numbered register data.CNcomment:获取Macrovision 编号寄存器CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp          DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] index         index of macrovision reg_number to set.  CNcomment: Macrovision 寄存器编号1~21 CNend
   \param[out] pData        pointer of Macrovision control data CNcomment: 指针类型 Macrovision 寄存器数据 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetMacrovisionRegN(MT_UNF_DISP_E enDisp, mt_u32 index, mt_u32 *pData);


/**
   \brief get Macrovision mode.CNcomment: 获取Macrovision模式 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] penMode    pointer of Macrovision mode. CNcomment:指针类型，MACROVISION模式 CNend
   \param[out] pData      a data pointer only valid whenpenMode=MT_MACROVISION_MODE_CUSTOM.\n
   CNcomment:指针类型，仅当penMode=MT_MACROVISION_MODE_CUSTOM时有效 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetMacrovision( MT_UNF_DISP_E enDisp, MT_UNF_DISP_MACROVISION_MODE_E *penMode, const mt_void *pData );


/**
   \brief   set CGMS data.CNcomment:设置CGMS（Copy Generation Management System）数据 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp                   DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstCgmsCfg             pointer of CGMS configuration  CNcomment:指针类型，指向CGMS配置数据的指针 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetCgms( MT_UNF_DISP_E enDisp, const MT_UNF_DISP_CGMS_CFG_S *pstCgmsCfg );


/**
   \brief chip do or not support Macrovision.CNcomment: 获取芯片是否支持Macrovision CNend
   \attention \n
   none.CNcomment:无 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pbSupport    pointer of Macrovision flag. CNcomment:指针类型，Macrovision 标志 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetMacrovisionSupport( MT_UNF_DISP_E enDisp, MT_BOOL *pbSupport );

/**
   \brief set aspect ratio attribute of display device.CNcomment: 设置设备宽高比 CNend
   \attention \n
   If set AspectRatio USER mode ,must be set u32UserAspectWidth and u32UserAspectHeight \n
   (theWidth/Height range is 1/64~64,if set zero means using current screen AspectRatio.) \n
   If not AspectRatio USER mode ,it will be set value by enumeration means;In this mode u32UserAspectWidth and u32UserAspectHeight are invalid ;\n
   If set AspectRatio AUTO mode , use current screen AspectRatio;
   CNcomment:如果设置USER模式则需设置u32UserAspectWidth 和u32UserAspectHeight的值，\n
   赋值范围为两者比值1/64~64,如果设置0则表示使用当前屏幕的宽高比；\n
   非USER模式则按照对应的比值起效u32UserAspectWidth 和u32UserAspectHeight的值无效\n
   其中设置AUTO模式则设备宽高比值自动设置为当前屏幕的宽高比。 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstDispAspectRatio    pointer of aspect ratio attribute of display device . CNcomment:指针类型，显示设备宽高比信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetAspectRatio( MT_UNF_DISP_E enDisp, MT_UNF_DISP_ASPECT_RATIO_S *pstDispAspectRatio );


/**
   \brief get aspect ratio attribute of display device.CNcomment: 获得设备宽高比属性 CNend
   \attention \n
   none.
   CNcomment:无 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pstDispAspectRatio    pointer of aspect ratio attribute of display device . CNcomment:指针类型，显示设备宽高比信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetAspectRatio( MT_UNF_DISP_E enDisp, MT_UNF_DISP_ASPECT_RATIO_S *pstDispAspectRatio );


/**
   \brief set algorithmic attribute of display device.CNcomment: 设置设备算法属性 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstAlg    pointer of algorithmic attribute of display device . CNcomment:指针类型，显示设备算法属性 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetAlgCfg( MT_UNF_DISP_E enDisp, MT_UNF_DISP_ALG_CFG_S *pstAlg );


/**
   \brief get algorithmic status of display device.CNcomment: 获取设备算法属性 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pstAlg    pointer of algorithmic attribute of display device . CNcomment:指针类型，显示设备算法属性 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetAlgCfg( MT_UNF_DISP_E enDisp, MT_UNF_DISP_ALG_CFG_S *pstAlg );


/**
   \brief create VBI data channel.CNcomment: 创建VBI数据通道 CNend
   \attention \n
   Each VBI type can not  created  repeatedly,if created more than one time,it will be return MT_ERR_DISP_CREATE_ERR;
   CNcomment:每种VBI类型只能创建一次，多次创建则返回MT_ERR_DISP_CREATE_ERR CNend
   \param[in] enDisp      DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstCfg    pointer of parameter of VBI channel . CNcomment:指针类型，VBI通道参数 CNend
   \param[out] phVbi    pointer of VBI handle . CNcomment:指针类型，VBI通道句柄 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \retval ::MT_ERR_DISP_CREATE_ERR    invalid operation. CNcomment:创建失败 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_CreateVBI( MT_UNF_DISP_E enDisp, MT_UNF_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi );


/**
   \brief destroy VBI data channel.CNcomment: 销毁VBI数据通道 CNend
   \attention \n
   none.
   CNcomment:无 CNend
   \param[in] hVbi      VBI handle ID.CNcomment:VBI 通道句柄 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_DestroyVBI( mt_handle hVbi );


/**
   \brief send data to vbi channel.CNcomment: 发送数据到VBI通道 CNend
   \attention \n
   After  call MT_UNF_DISP_CreateVBI creat VBI handle sucess,you can call API MT_UNF_DISP_SendVBIData send VBI data .
   CNcomment:调用MT_UNF_DISP_CreateVBI 创建VBI通道成功后，才能调用MT_UNF_DISP_SendVBIData  发送数据 CNend
   \param[in] hVbi      VBI handle ID.CNcomment:VBI 通路句柄 CNend
   \param[in] pstVbiData      pointer of VBI data.CNcomment:VBI数据指针 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SendVBIData( mt_handle hVbi, MT_UNF_DISP_VBI_DATA_S *pstVbiData );

/**
   \brief Get default attributes of screen share channel.CNcomment: 获取屏幕投影通道的默认属性 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[out]pstAttr     pointer of parameter .CNcomment:指针,配置参数 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetDefaultCastAttr(MT_UNF_DISP_E enDisp,MT_UNF_DISP_CAST_ATTR_S *pstAttr);


/**
   \brief create screen share channel.CNcomment: 创建屏幕投影通道 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[in] pstAttr      pointer of parameter .CNcomment:指针,配置参数 CNend
   \param[out] phCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_CreateCast( MT_UNF_DISP_E enDisp, MT_UNF_DISP_CAST_ATTR_S * pstAttr, mt_handle *phCast );


/**
   \brief destroy screen share channel.CNcomment: 销毁屏幕投影通道 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] phCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_DestroyCast( mt_handle hCast );


/**
   \brief enable screen share.CNcomment: 使能屏幕投影功能 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] phCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \param[in] bEnable      enable screen share .CNcomment:使能屏幕投影 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetCastEnable( mt_handle hCast, MT_BOOL bEnable );


/**
   \brief enable low delay frame skip.
   \attention \n
   none. CNcomment:无 CNend
   \param[in] phCast      handle of screen 
   \param[in] bEnable      enable low delay
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetLowDelayEnable(mt_handle hCast, MT_BOOL bEnable);
/**
   \brief get enable flag of screen share.CNcomment: 获取屏幕投影是否使能 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] phCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \param[out] bEnable     flag .CNcomment:标志 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetCastEnable( mt_handle hCast, MT_BOOL *pbEnable );


/**
   \brief get frame info of screen share.CNcomment: 获取屏幕投影帧信息 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] hCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \param[out] pstFrameInfo        frame info.CNcomment:帧信息 CNend
   \param[in] u32TimeoutMs wait time in ms.CNcomment:等待时间，毫秒为单位 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_AcquireCastFrame(mt_handle hCast, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo, mt_u32 u32TimeoutMs);


/**
   \brief release frame info of screen share.CNcomment: 释放屏幕投影帧信息 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] hCast      handle of screen share .CNcomment:屏幕投影句柄 CNend
   \param[in] pstFrameInfo     frame info.CNcomment:帧信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_ReleaseCastFrame( mt_handle hCast, MT_UNF_VIDEO_FRAME_INFO_S *pstFrameInfo );


/**
   \brief professional color modulation.CNcomment: 专业色彩调节 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[in] pstCS      struct of color modulation.CNcomment:色彩调节结构 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetColor( MT_UNF_DISP_E enDisp, MT_UNF_DISP_COLOR_SETTING_S *pstCS );


/**
   \brief get color modulation parameter.CNcomment: 获取色彩调节信息 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[in] pstCS      struct of color modulation.CNcomment:色彩调节结构 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetColor( MT_UNF_DISP_E enDisp, MT_UNF_DISP_COLOR_SETTING_S *pstCS );


/**
   \Snapshot truncation screen connection, intercepts the screen complete picture;CNcomment:截取屏幕完整画面; CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[in] pstFrameInfo     frame info.CNcomment:帧信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_AcquireSnapshot(MT_UNF_DISP_E enDisp, MT_UNF_VIDEO_FRAME_INFO_S * pstFrameInfo);

/**
   \Release Snapshot picture;CNcomment:释放截屏画面; CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \param[in] pstFrameInfo     frame info.CNcomment:帧信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_ReleaseSnapshot(MT_UNF_DISP_E enDisp, MT_UNF_VIDEO_FRAME_INFO_S * pstFrameInfo);


/**
   \brief get color modulation parameter.CNcomment:设置DISP的默认属性 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp      display channel.CNcomment:显示通路 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetDefaultPara( MT_UNF_DISP_E enDisp);


/**
   \brief set DISP 3D format.CNcomment:设置DISP的3D制式 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] en3D             DISP 3D mode.CNcomment:DISP 3D模式 CNend
   \param[in] enEncodingFormat DISP format.CNcomment:DISP的制式 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Set3DMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_3D_E en3D, MT_UNF_ENC_FMT_E enEncFormat);

/**
   \brief get DISP 3D format.CNcomment:查询DISP的3D制式 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pen3D           DISP 3D mode.CNcomment:DISP 3D模式 CNend
   \param[out] penEncFormat    DISP format.CNcomment:DISP的制式 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Get3DMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_3D_E *pen3D, MT_UNF_ENC_FMT_E *penEncFormat);

/**
   \brief set right eye first for 3D output.CNcomment:设置3D输出右眼优先 CNend
   \attention \n
   Only take effect in 3D output mode. CNcomment: 仅在3D输出时有效 CNend
   \param[in] enDisp     DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] bEnable   Right-Eye-First.CNcomment:右眼优先 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetRightEyeFirst(MT_UNF_DISP_E enDisp, MT_BOOL bEnable);

/**
   \brief set virtual screen size of display.CNcomment:设置显示通道的虚拟屏幕大小 CNend
   \attention \n
   if not set ,default value is 1280*720. CNcomment: 如果不设置此函数，默认的虚拟屏幕大小是720p. CNend
   \param[in] enDisp   DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Width  virtual screen width, the range is [480, 4096].CNcomment:虚拟屏幕宽度，取值范围为[480, 3840]. CNend
   \param[in] u32Height  virtual screen height, the range is [480, 4096].CNcomment:虚拟屏幕高度，取值范围为[480, 3840]. CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetVirtualScreen(MT_UNF_DISP_E enDisp, mt_u32 u32Width, mt_u32 u32Height);
mt_s32 MT_UNF_DISP_SetSmallWindow(MT_UNF_DISP_E enDisp, mt_s32 xstart, mt_s32 ystart,mt_u32 u32Width, mt_u32 u32Height);

/**
   \brief get virtual screen size of display.CNcomment:获取显示通道的虚拟屏幕大小 CNend
   \attention \n
   none. CNcomment: 无 CNend
   \param[in] enDisp   DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] u32Width  virtual screen width, the range is [480, 4096].CNcomment:虚拟屏幕宽度，取值范围为[480, 3840]. CNend
   \param[out] u32Height  virtual screen height, the range is [480, 4096].CNcomment:虚拟屏幕高度，取值范围为[480, 3840]. CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetVirtualScreen(MT_UNF_DISP_E enDisp, mt_u32 *u32Width, mt_u32 *u32Height);


/**
   \brief set offset of display area in real screen.CNcomment:设置显示区域在实体屏幕上的偏移量,为物理像素点 CNend
   \attention \n
   must be 4 pixels aligned,if not set, the default value is 0. CNcomment: offset 值必须是4对齐,如果不设置默认的值为0. CNend
   \param[in] enDisp   DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstOffset  offset in pixel, the range is [0, 200].CNcomment:偏移像素数目，取值范围为[0, 200]. CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetScreenOffset(MT_UNF_DISP_E enDisp, MT_UNF_DISP_OFFSET_S *pstOffset);

/**
   \brief get offset of display area in real screen.CNcomment:获取显示区域在实体屏幕上的偏移量,为物理像素点 CNend
   \attention \n
   none. CNcomment: 无 CNend
   \param[in] enDisp   DISP channel ID.CNcomment:DISP通道号 CNend
   \param[out] pstOffset  offset in pixel, the range is [0, 200].CNcomment:偏移像素数目，取值范围为[0, 200]. CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetScreenOffset(MT_UNF_DISP_E enDisp, MT_UNF_DISP_OFFSET_S *pstOffset);

/**
   \brief set DISP video/subvideo onoff.CNcomment:设置DISP的video / subvideo显示CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] pstOffset  offset in pixel.CNcomment:偏移像素数目 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetLayerShow(MT_UNF_DISP_E enDisp, MT_UNF_DISP_LAYER_ID_E enLayer, MT_BOOL bEnable);

/**
   \brief set DISP PP mode.CNcomment:设置DISP的显示效果CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] enMode             DISP PP mode.CNcomment:DISP PP模式 CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetPPMode(MT_UNF_DISP_E enDisp, MT_UNF_DISP_PP_E enMode);

mt_s32 MT_UNF_DISP_VidLayerShow(MT_BOOL bEnable);

mt_s32 MT_UNF_DISP_GetVidLayerEnable(MT_BOOL *pbEnable);

/**
   \brief set DISP PP mode.CNcomment:set the alpha value of given disp layer CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Alpha             DISP alpha value.CNcomment:视频层透明度，取值0~255；255表示不透明CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetAlpha(MT_UNF_DISP_E enDisp, MT_U32 u32Alpha);

/**
   \brief set DISP PP mode.CNcomment:get the alpha value of given disp layer CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] u32Alpha             DISP alpha value.CNcomment:视频层透明度，取值0~255；255表示不透明CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetAlpha(MT_UNF_DISP_E enDisp, mt_u32 *u32Alpha);

/**
   \brief set CSC on off.CNcomment:set CSC on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] u32Alpha             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetCscEnable(MT_BOOL bEnable);

/**
   \brief set denoise on off.CNcomment:set denoise on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetDenoiseEnable(MT_BOOL bEnable);

/**
   \brief set AFD on off.CNcomment:set AFD on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetAfdEnable(MT_BOOL bEnable);

/**
   \brief set Hd Video on off.CNcomment:set Hd video on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetHdVideoEnable(MT_BOOL bEnable);

/**
   \brief set Sd Video on off.CNcomment:set Sd video on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetSdVideoEnable(MT_BOOL bEnable);

/**
   \brief set Vdac on off.CNcomment:set Vdac on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] eDacId              vdac ID.CNcomment:CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_INDEX_E eDacId, MT_BOOL bEnable);

/**
   \brief set DI on off.CNcomment:set DI on off CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetDiOnOff(MT_BOOL bEnable);

/**
   \brief set tv capability.CNcomment:set tv capability CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enTvCapability  .CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetTvCapability(MT_UNF_DISP_HDMI_MODE_E enTvCapability);

/**
   \brief set Colorbar on off.CNcomment:使能 Colorbar 输出 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetColorbar(MT_UNF_DISP_E enDisp, MT_BOOL bEnable);

/**
   \brief set disp output on off.CNcomment:使能/关闭 HD，SD输出 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] enDisp           DISP channel ID.CNcomment:DISP通道号 CNend
   \param[in] bEnable             on or off.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetOutputEnable(MT_UNF_DISP_E enDisp, MT_BOOL bEnable);

/**
   \brief reset display hardware.CNcomment:reset display hardware with high speed or low speed clock CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] bHighSpeed true or false.CNcomment: 配置display clock为高频或低频CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_ResetHardware(MT_BOOL bHighSpeed);

/**
   \brief enable/disable sd scaler.CNcomment: 配置sd scaler使能 ---> force 3scaler/2scaler mode or auto mode CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] state: 0,1,or 2.CNcomment: 0 for 2scaler mode,1 for 3scaler mode, 2 for auto mode CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_SetSdScalerEnable(MT_UNF_DISP_SCALER_MODE_E state);

/**
   \brief dump hd_screen / video / sd_screen to OSD layer.CNcomment: dump hd输出、视频、sd输出 到OSD 显示buffer CNend
   \attention \n
   none.   CNcomment:无 CNend
   \param[in] pstParam      pointer of dump scaler parameter.CNcomment:dump scaler 配置信息 CNend
   \retval ::MT_SUCCESS CNcomment: success.成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_NULL_PTR          Input pointer is NULL.CNcomment:输入指针为空 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \retval ::MT_ERR_DISP_INVALID_OPT    invalid operation. CNcomment:操作非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_DumpScaler2OSD(MT_UNF_DISP_DUMP_SCALER_PARA_S *pstParam);

/**
   \brief set technicolor HDR parameters.CNcomment:set technicolor HDR parameters CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] display_brightness_hdr             hdr display brightness.CNcomment:CNend
   \param[in] display_brightness_sdr             sdr display brightness.CNcomment:CNend
   \param[in] tuning_level             target display adaptation tuning level.CNcomment:CNend
   \param[in] display_OETF             display OETF.CNcomment:CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_Set_Sl_Hdr(MT_UNF_DISP_E enDisp, mt_u32 transparent_mode, mt_u32 display_brightness_hdr, mt_u32 display_brightness_sdr, mt_u32 tuning_level, mt_u32 display_OETF);


/**
   \brief get sl-hdr lib version.CNcomment:获取sl-hdr版本号 CNend
   \attention \n
   none. CNcomment:无 CNend
   \param[in] pVer sl-hdr version. CNcomment:版本号CNend
   \retval ::MT_SUCCESS  success.CNcomment:成功 CNend
   \retval ::MT_ERR_DISP_NO_INIT    DISP uninitialization.CNcomment:DISP未初始化 CNend
   \retval ::MT_ERR_DISP_INVALID_PARA   invalid input parameter.CNcomment:输入参数非法 CNend
   \see \n
   none.CNcomment:无 CNend
 */
mt_s32 MT_UNF_DISP_GetSlHdrVersion(mt_u32 *pVer);

/** @} */  /** <!-- ==== API declaration end ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif
/************************************** The End Of File **************************************/
