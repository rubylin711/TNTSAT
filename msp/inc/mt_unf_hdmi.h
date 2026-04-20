/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_HDMI_H__
#define __MT_UNF_HDMI_H__

/* add include here */
#include "mt_unf_common.h"
#include "mt_unf_sound.h"
#include "mt_unf_edid.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*************************** Structure Definition ****************************/
/** \addtogroup      HDMI */
/** @{ */ /** <!-- [HDMI] */

/**the max infoframe length*/ /**CNcomment:最大信息帧长度 */
#define MT_UNF_HDMI_MAX_INFOFRAME_LEN 0X20

/**HDMI default value*/ /**CNcomment:HDMI 缺省值 */
#define MT_UNF_HDMI_DEFAULT_SETTING 0x00

/**HDMI interface ID */ /**CNcomment:HDMI接口ID  */
typedef enum mtUNF_HDMI_ID_E {
    MT_UNF_HDMI_ID_0 = 0,
    /**<HDMI interface 0*/ /**<CNcomment:HDMI接口0 */
    MT_UNF_HDMI_ID_BUTT
} MT_UNF_HDMI_ID_E;

/**HDMI event type*/ /**CNcomment: HDMI事件类型 */
typedef enum mtUNF_HDMI_EVENT_TYPE_E {
    MT_UNF_HDMI_EVENT_HOTPLUG = 0x10,
    /**<HDMI HotPlug event type*/ /**<CNcomment:<HDMI热插拔事件 */
    MT_UNF_HDMI_EVENT_NO_PLUG,
    /**<HDMI unplug event type*/ /**<CNcomment:HDMI Cable没有连接 事件 */
    MT_UNF_HDMI_EVENT_EDID_FAIL,
    /**<HDMI read edid fail event type*/ /**<CNcomment:HDMI EDID读取失败事件 */
    MT_UNF_HDMI_EVENT_HDCP_FAIL,
    /**<HDCP authentication fail event type */ /**<CNcomment:HDCP验证失败事件 */
    MT_UNF_HDMI_EVENT_HDCP_SUCCESS,
    /**<HDCP authentication succeed event type*/ /**<CNcomment:HDCP验证成功 */
    MT_UNF_HDMI_EVENT_RSEN_CONNECT,
    /**<TMDS link is connected*/ /**<CNcomment:TMDS链接成功 */
    MT_UNF_HDMI_EVENT_RSEN_DISCONNECT,
    /**<TMDS link is disconnected*/ /**<CNcomment:TMDS未链接 */
    MT_UNF_HDMI_EVENT_HDCP_USERSETTING,
    /**<HDMI CEC msg tv send standby msg*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_OK,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_UP,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_DOWN,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_LEFT,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_RIGHT,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_UP,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_DOWN,
    /**<HDMI CEC msg notify*/
    MT_UNF_HDMI_EVENT_TV_SEND_CEC_MUTE,


    /**<HDMI CEC statuc changed msg notify:  CEC_INIT_STATUS */
    MT_UNF_HDMI_EVENT_CEC_STATUS_INIT,

    /**<HDMI CEC statuc changed msg notify:  CEC_READY_STATUS */
    MT_UNF_HDMI_EVENT_CEC_STATUS_READY,

    /**<HDMI CEC statuc changed msg notify:  CEC_POWER_ON_STATUS */
    MT_UNF_HDMI_EVENT_CEC_STATUS_POWER_ON,

    /**<HDMI CEC statuc changed msg notify:  CEC_STANDBY_STATUS */
    /* MT_UNF_HDMI_EVENT_CEC_STATUS_STANDBY replaced by MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY  */
    /*  MT_UNF_HDMI_EVENT_CEC_STATUS_STANDBY */

    /**<HDMI CEC statuc changed msg notify:  HDMI_PORT_SWTICH_OUT */
    MT_UNF_HDMI_EVENT_CEC_STATUS_PORT_SWTICH_OUT,

	MT_UNF_HDMI_EVENT_CEC_GET_MENU_LANGE_MSG,
   	MT_UNF_HDMI_EVENT_CEC_SET_MENU_LANGE_MSG,

    /**<HDMI Reset */ /**<CNcomment:HDCP 复位*/
    MT_UNF_HDMI_EVENT_BUTT,
    /**<HDMI max_event */
    MT_UNF_HDMI_EVENT_MAX
} MT_UNF_HDMI_EVENT_TYPE_E;

/*Video color space mode*/ /**CNcomment:视频颜色空间类型*/
typedef enum mtUNF_HDMI_VIDEO_MODE {
    MT_UNF_HDMI_VIDEO_MODE_RGB444,
    /**<RGB444 output mode*/ /**<CNcomment:RGB444输出模式 */
    MT_UNF_HDMI_VIDEO_MODE_YCBCR422,
    /**<YCBCR422 output mode*/ /**<CNcomment:YCBCR422输出模式 */
    MT_UNF_HDMI_VIDEO_MODE_YCBCR444,
    /**<YCBCR444 output mode*/ /**<CNcomment:YCBCR444输出模式 */
	MT_UNF_HDMI_VIDEO_MODE_YCBCR420,
	/**<YCBCR420 output mode*/ /**<CNcomment:YCBCR420输出模式 */
    MT_UNF_HDMI_VIDEO_MODE_AUTO,
    /**<Auto RGB4444 or YCBCR444 output mode*/ /**<CNcomment: auto select the RGB444 or YCBCR444输出模式 */
    MT_UNF_HDMI_VIDEO_MODE_BUTT
} MT_UNF_HDMI_VIDEO_MODE_E;

/*HDMI Output Aspect Ratio*/ /**CNcomment:HDMI输出宽高比*/
typedef enum mtUNF_HDMI_ASPECT_RATIO_E {
    MT_UNF_HDMI_ASPECT_RATIO_NO_DATA,
    /**<Aspect Ratio unknown */ /**<CNcomment:未知宽高比 */
    MT_UNF_HDMI_ASPECT_RATIO_4TO3,
    /**<Aspect Ratio 4:3  */ /**<CNcomment:宽高比4:3*/
    MT_UNF_HDMI_ASPECT_RATIO_16TO9,
    /**<Aspect Ratio 16:9 */ /**<CNcomment:宽高比16:9 */
    MT_UNF_HDMI_ASPECT_RATIO_FUTURE,
    MT_UNF_HDMI_ASPECT_RATIO_BUTT
} MT_UNF_HDMI_ASPECT_RATIO_E;

/**HDMI Deep color mode*/ /**CNcomment: HDMI 深色模式 */
typedef enum mtUNF_HDMI_DEEP_COLOR_E {
    MT_UNF_HDMI_DEEP_COLOR_24BIT = 0x00,
    /**<HDMI Deep color 24bit mode*/ /**<CNcomment:HDMI 24bit 深色模式  */
    MT_UNF_HDMI_DEEP_COLOR_30BIT,
    /**<HDMI Deep color 30bit mode*/ /**<CNcomment:HDMI 30bit 深色模式  */
    MT_UNF_HDMI_DEEP_COLOR_36BIT,
    /**<HDMI Deep color 36bit mode*/ /**<CNcomment:HDMI 36bit 深色模式  */
    MT_UNF_HDMI_DEEP_COLOR_48BIT,
    /**<HDMI Deep color 48bit mode. NOT SUPPORT*/ /**<CNcomment:HDMI 48bit 深色模式. 不支持*/
    MT_UNF_HDMI_DEEP_DC_Y444,
    /**<HDMI Deep color YUV mode,internal use*/ /**<CNcomment:HDMI YUV 深色模式, 内部使用*/
    MT_UNF_HDMI_DEEP_COLOR_OFF = 0xff,
    MT_UNF_HDMI_DEEP_COLOR_BUTT
} MT_UNF_HDMI_DEEP_COLOR_E;

/**HDMI AVI infoframe BarInfo enum*/ /**CNcomment: HDMI AVI信息帧 BarInfo 枚举 */
typedef enum mtUNF_HDMI_BARINFO_E {
    HDMI_BAR_INFO_NOT_VALID,
    /**<Bar Data not valid */ /**<CNcomment:无效Bar数据  */
    HDMI_BAR_INFO_V,
    /**<Vertical bar data valid */ /**<CNcomment:垂直Bar数据有效  */
    HDMI_BAR_INFO_H,
    /**<Horizental bar data valid */                               /**<CNcomment:水平bar数据有效  */
    HDMI_BAR_INFO_VH /**<Horizental and Vertical bar data valid */ /**<CNcomment:水平垂直Bar数据同时有效 */
} MT_UNF_HDMI_BARINFO_E;

/**HDMI AVI infofram ScanInfo enum*/ /**CNcomment: HDMI AVI信息帧 ScanInfo 枚举 */
typedef enum mtUNF_HDMI_SCANINFO_E {
    HDMI_SCAN_INFO_NO_DATA = 0,
    /**< No Scan information*/ /**<CNcomment:无扫描信息  */
    HDMI_SCAN_INFO_OVERSCANNED = 1,
    /**< Scan information, Overscanned (for television) */ /**<CNcomment:扫描信息:全画面扫描  */
    HDMI_SCAN_INFO_UNDERSCANNED = 2,
    /**< Scan information, Underscanned (for computer) */ /**<CNcomment:扫描信息: 非全画面扫描  */
    HDMI_SCAN_INFO_FUTURE
} MT_UNF_HDMI_SCANINFO_E;

/**HDMI AVI InfoFrame picture scale enum*/ /**CNcomment: HDMI AVI信息帧 Picture scale 枚举 */
typedef enum mtUNF_HDMI_PICTURE_SCALING_E {
    HDMI_PICTURE_NON_UNIFORM_SCALING,
    /**< No Known, non-uniform picture scaling  */ /**<CNcomment:统一图像坐标  */
    HDMI_PICTURE_SCALING_H,
    /**< Picture has been scaled horizentally */ /**<CNcomment:图像水平坐标化  */
    HDMI_PICTURE_SCALING_V,
    /**< Picture has been scaled Vertically */                                            /**<CNcomment:图像垂直坐标化  */
    HDMI_PICTURE_SCALING_HV /**< Picture has been scaled Horizentally and Vertically   */ /**<CNcomment:图像水平垂直坐标化  */
} MT_UNF_HDMI_PICTURE_SCALING_E;

/**HDMI AVI InfoFrame colorimetry enum*/ /**CNcomment: HDMI AVI信息帧 色度空间 枚举 */
typedef enum mtUNF_HDMI_COLORSPACE_E {
    HDMI_COLORIMETRY_NO_DATA,
    /**<Colorimetry No Data option*/ /**<CNcomment:Colorimetry No Data选项 */
    HDMI_COLORIMETRY_ITU601,
    /**<Colorimetry ITU601 option*/ /**<CNcomment:Colorimetry ITU601色度空间选项 */
    HDMI_COLORIMETRY_ITU709,
    /**<Colorimetry ITU709 option*/ /**<CNcomment:Colorimetry ITU709色度空间选项 */
    HDMI_COLORIMETRY_EXTENDED,
    /**<Colorimetry extended option*/ /**<CNcomment:Colorimetry 扩展选项 */
    HDMI_COLORIMETRY_XVYCC_601,
    /**<Colorimetry xvYCC601 extened option*/                            /**<CNcomment:Colorimetry xvYCC601扩展选项 */
    HDMI_COLORIMETRY_XVYCC_709, /**<Colorimetry xvYCC709 extened option*/ /**<CNcomment:Colorimetry xvYCC709扩展选项 */
	HDMI_COLORIMETRY_BT2020_C,
	/**<Colorimetry BT2020 Constant option*/ /**<CNcomment:Colorimetry BT2020 Constant色度空间选项 */
	HDMI_COLORIMETRY_BT2020_NC,
	/**<Colorimetry BT2020 Non-Constant option*/ /**<CNcomment:Colorimetry BT2020 Non-Constant色度空间选项 */
} MT_UNF_HDMI_COLORSPACE_E;

/**HDMI AVI InfoFrame RGB range enum*/ /**CNcomment: HDMI AVI信息帧 RGB色度范围 枚举 */
typedef enum mtUNF_HDMI_RGB_QUAN_RAGE_E {
    HDMI_RGB_QUANTIZATION_DEFAULT_RANGE,
    /**< Defaulr range, it depends on the video format */ /**<CNcomment:默认色度范围，依赖于视频制式 */
    HDMI_RGB_QUANTIZATION_LIMITED_RANGE,
    /**< Limited quantization range of 220 levels when receiving a CE video format*/                                /**<CNcomment:受限色度范围16-234 */
    HDMI_RGB_QUANTIZATION_FULL_RANGE /**< Full quantization range of 256 levels when receiving an IT video format*/ /**<CNcomment:全色度范围 0-255 */
} MT_UNF_HDMI_RGB_QUAN_RAGE_E;

/**HDMI AVI InfoFrame YCC quantization range enum */ /**CNcomment:HDMI AVI信息帧 YCC色度范围 枚举 */
typedef enum mtUNF_HDMI_YCC_QUAN_RAGE_E {
    HDMI_YCC_QUANTIZATION_LIMITED_RANGE,
    /**< Limited quantization range of 220 levels when receiving a CE video format*/                                /**<CNcomment:受限色度范围16-234 */
    HDMI_YCC_QUANTIZATION_FULL_RANGE /**< Full quantization range of 256 levels when receiving an IT video format*/ /**<CNcomment:全色度范围 0-255 */
} MT_UNF_HDMI_YCC_QUAN_RAGE_E;

/**HDMI AVI InfoFrame AVI video content type enum*/ /**CNcomment:HDMI AVI信息帧 AVI视频内容的类型 枚举 */
typedef enum mtUNF_HDMI_CONTENT_TYPE_E {
    HDMI_CONTNET_GRAPHIC,
    /**< Graphics type*/ /**<CNcomment:图像 */
    HDMI_CONTNET_PHOTO,
    /**< Photo type*/ /**<CNcomment:照片 */
    HDMI_CONTNET_CINEMA,
    /**< Cinema type*/                 /**<CNcomment:电影院 */
    HDMI_CONTNET_GAME /**< Game type*/ /**<CNcomment:游戏 */
} MT_UNF_HDMI_CONTENT_TYPE_E;

/**HDMI Priority judgments strategy enum*/ /**CNcomment:HDMI 优先判断策略 枚举*/
typedef enum mtUNF_HDMI_DEFAULT_ACTION_E {
    MT_UNF_HDMI_DEFAULT_ACTION_NULL,
    /**<Default action null*/ /**<CNcomment:无默认优先策略 */
    MT_UNF_HDMI_DEFAULT_ACTION_HDMI,
    /**<Default action HDMI*/ /**<CNcomment:优先判断HDMI */
    MT_UNF_HDMI_DEFAULT_ACTION_DVI,
    /**<Default action DVI*/ /**<CNcomment:优先判断DVI */
    MT_UNF_HDMI_DEFAULT_ACTION_BUTT
} MT_UNF_HDMI_DEFAULT_ACTION_E;

/**The hotplug callback function interface */
/**CNcomment: 接口热插拔回调函数 */
typedef void (*MT_UNF_HDMI_CALLBACK)(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);

/**HDMI Callback Struct*/
/**CNcomment: HDMI回调结构 */
typedef struct mtUNF_HDMI_CALLBACK_FUNC_S
{
    MT_UNF_HDMI_CALLBACK pfnHdmiEventCallback; /**<callback function pointer*/ /**<CNcomment:回调函数指针*/
    mt_void *pPrivateData; /**<callback funtion param*/                        /**<CNcomment:回调函数参数*/
} MT_UNF_HDMI_CALLBACK_FUNC_S;


/*!
  HDMI init parameter need set  by up-layer,else use default
  */
typedef enum {
    /*!
    HDMI already initialized
    */
    HDMI_INITIALIZED = 0x01,
    /*!
    hdmi open need init cec enable
    */
    HDMI_INIT_CEC_ENABLE = 0x02,
     /*!
    hdmi  use output key
    */
    HDMI_INIT_OUT_HDCP_KEY = 0x04,
     /*!
    hdmi  use inner key
    */
    HDMI_INIT_INNER_HDCP_KEY = 0x08,

} MT_UNF_HDMI_INIT_PARAM;
/**HDMI Open Param*/
/**CNcomment: HDMI打开参数 */
typedef struct mtUNF_HDMI_OPEN_PARA_S
{
    MT_UNF_HDMI_DEFAULT_ACTION_E enDefaultMode; /**<HDMI Priority judgments strategy enum*/ /**<CNcomment:HDMI 优先判断策略 枚举*/
    MT_UNF_HDMI_INIT_PARAM u32InitParam;/*hdmi open init param */
} MT_UNF_HDMI_OPEN_PARA_S;

/**HDMI status*/
/**CNcomment: HDMI状态 */
typedef struct mtUNF_HDMI_STATUS_S
{
    MT_BOOL bConnected; /**<The Device is connected or disconnected */ /**<CNcomment:设备是否连接 */
    MT_BOOL bSinkPowerOn; /**<The sink is PowerOn or not*/             /**<CNcomment:Sink设备是否上电 */
    MT_BOOL bAuthed; /**<HDCP Authentication */                        /**<CNcomment:HDCP 是否握手完成 */
    mt_u8 u8Bksv[5]; /**<Bksv of sink 40bits*/                         /**<CNcomment:接收端的Bksv */
    mt_u32 sink_cpower_status; //sink power status                         /**<CNcomment:接收端的端口状态 */
} MT_UNF_HDMI_STATUS_S;

/**the config parameter of HDMI interface*/
/**CNcomment:HDMI 接口参数配置 */
typedef struct mtUNF_HDMI_ATTR_S
{
    mt_u8 bEnableHdmi; /**<0: DVI,1: HDMI,2:HDMI and DVI,the value must set before MT_UNF_HDMI_Start or behind MT_UNF_HDMI_Stop, 4:force DVI, 5:force HDMI,*/ /**<CNcomment:强制HDMI或者强制DVI或者HDMI和DVI都支持，该值必须在 MT_UNF_HDMI_Start之前或者MT_UNF_HDMI_Stop之后设置  */

    MT_BOOL bEnableVideo; /**<parameter must set MT_TRUE,or the HDMI diver will force to set MT_TRUE*/ /**<CNcomment:必须是MT_TRUE, 如果是MT_FALSE:HDMI驱动会强制设置为MT_TRUE */

    MT_UNF_HDMI_VIDEO_MODE_E enVidOutMode; /**<HDMI output vedio mode VIDEO_MODE_YCBCR,VIDEO_MODE_YCBCR444,VIDEO_MODE_YCBCR422,VIDEO_MODE_RGB444 */ /**<CNcomment:HDMI输出视频模式，VIDEO_MODE_YCBCR444，VIDEO_MODE_YCBCR422，VIDEO_MODE_RGB444 */
    MT_UNF_HDMI_DEEP_COLOR_E enDeepColorMode; /**<Deep Color output mode,defualt: MT_UNF_HDMI_DEEP_COLOR_24BIT */                                   /**<CNcomment:DeepColor输出模式, 默认为MT_UNF_HDMI_DEEP_COLOR_24BIT */
    MT_BOOL bxvYCCMode; /**<the xvYCC output mode,default:MT_FALSE*/                                                                                /**<CNcomment:< xvYCC输出模式，默认为MT_FALSE */

    MT_BOOL bEnableAudio; /**<Enable flag of Audio*/ /**CNcomment:是否Enable音频 */

    MT_BOOL bEnableAviInfoFrame; /**<Enable flag of AVI InfoFrame,suggestion:enable */    /**<CNcomment:是否使能 AVI InfoFrame，建议使能 */
    MT_BOOL bEnableAudInfoFrame; /**<Enable flag of Audio InfoFrame,suggestion:enable*/   /**<CNcomment:是否使能 AUDIO InfoFrame，建议使能 */
    MT_BOOL bEnableSpdInfoFrame; /**<Enable flag of SPD info frame,suggestion:disable*/   /**<CNcomment:是否使能 SPD InfoFrame， 建议关闭 */
    MT_BOOL bEnableMpegInfoFrame; /**<Enable flag of MPEG info frame,suggestion:disable*/ /**<CNcomment:是否使能 MPEG InfoFrame， 建议关闭 */

    MT_BOOL bHDCPEnable; /**<0:HDCP disable mode,1:eable HDCP mode*/ /**<CNcomment:< 0:HDCP不激活，1:HDCP模式打开 */
} MT_UNF_HDMI_ATTR_S;

/**HDMI infoFrame type definition*/
/**CNcomment: HDMI 信息帧类型定义 */
typedef enum tagMT_UNF_HDMI_INFOFRAME_TYPE_E {
    MT_INFOFRAME_TYPE_AVI,
    /**<HDMI AVI InfoFrame type defintion*/ /**<CNcomment:HDMI AVI InfoFrame 类型定义 */
    MT_INFOFRAME_TYPE_SPD,
    /**<HDMI SPD InfoFrame type defintion*/ /**<CNcomment:HDMI SPD InfoFrame 类型定义 */
    MT_INFOFRAME_TYPE_AUDIO,
    /**<HDMI AUDIO InfoFrame type defintion*/ /**<CNcomment:HDMI AUDIO InfoFrame 类型定义 */
    MT_INFOFRAME_TYPE_MPEG,
    /**<HDMI MPEG InfoFrame type defintion*/ /**<CNcomment:HDMI MPEG InfoFrame 类型定义 */
    MT_INFOFRAME_TYPE_VENDORSPEC,
    /**<HDMI Specific InfoFrame type defintion*/ /**<CNcomment:HDMI Vendor Specific InfoFrame 类型定义 */
    MT_INFOFRAME_TYPE_BUTT
} MT_UNF_HDMI_INFOFRAME_TYPE_E;

/**HDMI AVI InfoFrame parameter struct,please reference EIA-CEA-861-D*/
/**CNcomment: HDMI AVI 信息帧参数结构, 请参考EIA-CEA-861-D */
typedef struct mtUNF_HDMI_AVI_INFOFRAME_VER2_S
{
    MT_UNF_ENC_FMT_E enTimingMode; /**<AVI video timing format*/ /**<CNcomment:AVI视频timing格式 */
    /*hdmi_video_color_space_t   csc */
    MT_UNF_HDMI_VIDEO_MODE_E enOutputType; /**<AVI video output color space*/ /**<CNcomment:AVI视频输出颜色格式 */

    MT_BOOL bActive_Infor_Present; /**<AVI video Active_Infor_Present flag*/                /**<CNcomment:AVI视频Active_Infor_Present标志位 */
    MT_UNF_HDMI_BARINFO_E enBarInfo; /**<AVI video BarInfo type*/                           /**<CNcomment:AVI视频BarInfo类型 */
    MT_UNF_HDMI_SCANINFO_E enScanInfo; /**<AVI video ScanInfo type*/                        /**<CNcomment:AVI视频ScanInfo类型 */
    MT_UNF_HDMI_COLORSPACE_E enColorimetry; /**<AVI video Colorimetry type*/                /**<CNcomment:AVI视频Colorimetry类型 */
    MT_UNF_HDMI_ASPECT_RATIO_E enAspectRatio; /**<AVI video AspectRatio type*/              /**<CNcomment:AVI视频宽高比格式 */
    MT_UNF_HDMI_ASPECT_RATIO_E enActiveAspectRatio; /**<AVI video Active AspectRatio type*/ /**<CNcomment:AVI视频有效宽高比格式 */
    MT_UNF_HDMI_PICTURE_SCALING_E enPictureScaling; /**<AVI video picture scaling type*/    /**<CNcomment:AVI视频scaling格式 */
    MT_UNF_HDMI_RGB_QUAN_RAGE_E enRGBQuantization; /**<AVI video RGB Quantization*/         /**<CNcomment:AVI视频RGB色度范围 */
    MT_BOOL bIsITContent; /**<AVI video ITContent flag*/                                    /**<CNcomment:AVI视频ITcontent */
    mt_u32 u32PixelRepetition; /**<AVI video Pixel Repetition flag*/                        /**<CNcomment:AVI视频像素重传标志位 */

    MT_UNF_HDMI_CONTENT_TYPE_E enContentType; /**<AVI video content type*/          /**<CNcomment:AVI视频内容的类型 */
    MT_UNF_HDMI_YCC_QUAN_RAGE_E enYCCQuantization; /**<AVI video YCC Quantization*/ /**CNcomment:*< AVI视频YCC色度范围 */

    mt_u32 u32LineNEndofTopBar; /**<AVI video EndofTopBar coordinate,defualt:0 */         /**<CNcomment:AVI视频EndofTopBar坐标，缺省为0 */
    mt_u32 u32LineNStartofBotBar; /**<AVI video StartofBotBar coordinate,defualt:0*/      /**<CNcomment:AVI视频StartofBotBar坐标，缺省为0 */
    mt_u32 u32PixelNEndofLeftBar; /**<AVI video EndofLeft coordinate,defualt:0*/          /**<CNcomment:AVI视频EndofLeft坐标，缺省为0 */
    mt_u32 u32PixelNStartofRightBar; /**<AVI video StartofRightBar coordinate,defualt:0*/ /**<CNcomment:AVI视频StartofRightBar坐标，缺省为0 */
} MT_UNF_HDMI_AVI_INFOFRAME_VER2_S;

/**HDMI AUDIO InfoFrame parameter struct ,please reference EIA-CEA-861-D*/
/**CNcomment: HDMI 音频信息帧参数结构, 请参考EIA-CEA-861-D */
typedef struct mtUNF_HDMI_AUD_INFOFRAME_VER1_S
{
    mt_u32 u32ChannelCount; /**<audio frequency channel count*/                                                      /**<CNcomment:音频 声道数 */
    MT_UNF_EDID_AUDIO_FORMAT_CODE_E enCodingType; /**<audio frequency coding type,default 0;Refer to Stream Header*/ /**<CNcomment:音频 编码类型，缺省为0：Refer to Stream Header */
    mt_u32 u32SampleSize; /**<audio frequency sample size,default 0,Refer to Stream Header*/                         /**<CNcomment:音频 采样大小，缺省为0：Refer to Stream Header */
    mt_u32 u32SamplingFrequency; /**<audio frequency sampling frequency ,default 0,Refer to Stream Header*/          /**<CNcomment:音频 采样频率，缺省为0：Refer to Stream Header */
    mt_u32 u32ChannelAlloc; /**<audio frequency channel allocable ,default 0,Refer to Stream Header*/                /**<CNcomment:音频 声道分配，缺省为0：Refer to Stream Header */
    mt_u32 u32LevelShift; /**<audio frequency Levelshift ,default 0,Refer to Stream Header*/                         /**<CNcomment:音频 Levelshift，缺省为0：Refer to Stream Header */
    MT_BOOL u32DownmixInhibit; /**<audio frequency DownmixInhibit ,default 0,Refer to Stream Header*/                /**<CNcomment:音频 DownmixInhibit，缺省为0：Refer to Stream Header */
} MT_UNF_HDMI_AUD_INFOFRAME_VER1_S;

typedef enum
{
   HDMI_SRC_DEVICE_INFO_UNKNOWN,
   HDMI_SRC_DEVICE_INFO_DIG_STB,
   HDMI_SRC_DEVICE_INFO_DVD,
   HDMI_SRC_DEVICE_INFO_DVHS,
   HDMI_SRC_DEVICE_INFO_HDD_VIDEO,
   HDMI_SRC_DEVICE_INFO_DVC,
   HDMI_SRC_DEVICE_INFO_DSC,
   HDMI_SRC_DEVICE_INFO_VIDEO_CD,
   HDMI_SRC_DEVICE_INFO_GAME,
   HDMI_SRC_DEVICE_INFO_PC_GENERAL,
   HDMI_SRC_DEVICE_INFO_BLU_RAY_DISC,
   HDMI_SRC_DEVICE_INFO_SACD
} HDMI_SrcDeviceInfoSPD;


/**HDMI SPD InfoFrame parameter struct,please reference EIA-CEA-861-D*/
/**CNcomment: HDMI SPD信息帧参数结构 , 请参考EIA-CEA-861-D */
typedef struct mtUNF_HDMI_SPD_INFOFRAME_S
{
	mt_u8 uiVersion;
    mt_u8 u8VendorName[8]; /**<vendor name*/                  /**<CNcomment:卖方名称 */
    mt_u8 u8ProductDescription[16]; /**<product Description*/ /**<CNcomment:产品描述符 */
	HDMI_SrcDeviceInfoSPD eSrcDeviceInfo;
} MT_UNF_HDMI_SPD_INFOFRAME_S;

/**HDMI Source HDMI MPEG InfoFrame parameter struct,please reference EIA-CEA-861-D*/
/**CNcomment: HDMI MPEG信息帧参数结构 , 请参考EIA-CEA-861-D */
typedef struct mtUNF_HDMI_MPEGSOURCE_INFOFRAME_S
{
    mt_u32 u32MPEGBitRate; /**<MPEG bit Rate*/         /**<CNcomment:MPEG位率 */
    MT_BOOL bIsFieldRepeated; /**<FieldRepeater flag*/ /**<CNcomment:FieldRepeater标志位 */
} MT_UNF_HDMI_MPEGSOURCE_INFOFRAME_S;

/**HDMI Vendor Specific InfoFrame parameter struct,please reference EIA-CEA-861-D*/
/**CNcomment: HDMI VSIF信息帧 参数结构 , 请参考EIA-CEA-861-D */
typedef struct mtUNF_HDMI_VENDORSPEC_INFOFRAME_S
{
   mt_u8  uiVersion;       /* Version of the VS InfoFrame. Only 0x01 is defined in CEA-861-C */
   mt_u8  ucIEEERegistrationId[3]; /* 24 bit IEEE registration ID (LSB 1st) also called "company_id" or OUI */
   mt_u8  uiSpecificPayloadLength; /* Length (in bytes) of the vendor specific payload */
   mt_u8  pucSpecificPayload[24];   /* Pointer to vendor specific payload data */
} MT_UNF_HDMI_VENDORSPEC_INFOFRAME_S;


/**HDMI InfoFrame unit struct*/
/**CNcomment: HDMI 信息帧联合定义体 */
typedef union mtUNF_HDMI_INFOFRAME_UNIT_U
{
    MT_UNF_HDMI_AVI_INFOFRAME_VER2_S stAVIInfoFrame; /**<AVI FrameInfo*/                 /**<CNcomment:AVI信息帧 */
    MT_UNF_HDMI_AUD_INFOFRAME_VER1_S stAUDInfoFrame; /**<Audio FrameInfo*/               /**<CNcomment:AUD信息帧 */
    MT_UNF_HDMI_SPD_INFOFRAME_S stSPDInfoFrame; /**<SPD FrameInfo*/                      /**<CNcomment:SPD信息帧 */
    MT_UNF_HDMI_MPEGSOURCE_INFOFRAME_S stMPEGSourceInfoFrame; /**<MPEGSource FrameInfo*/ /**<CNcomment:MPEGSource信息帧 */
    MT_UNF_HDMI_VENDORSPEC_INFOFRAME_S stVendorSpecInfoFrame; /**<VS FrameInfo*/         /**<CNcomment:VS信息帧 */
} MT_UNF_HMDI_INFORFRAME_UNIT_U;

/**HDMI InfoFrame struct */
/**CNcomment: HDMI 信息帧数据结构 */
typedef struct mtUNF_HDMI_INFOFRAME_S
{
    MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType; /**<InfoFrame type*/   /**CNcomment:<InfoFrame类型 */
    MT_UNF_HMDI_INFORFRAME_UNIT_U unInforUnit; /**<InfoFrame unit data*/ /**CNcomment:<InfoFrame数据 */
} MT_UNF_HDMI_INFOFRAME_S;

/* CEC */
/** CEC interrelated Opcode:Please refer to CEC 15 Message Descriptions */
/** CNcomment: CEC 相关的操作码 */

/* General Protocol messages */

/**"Feature Abort" Used as a response to indicate that the device does not support the requested message type, or that it cannot execute it at the present time. */
/**CNcomment:"Feature Abort"消息表明设备不支持该信息的回复，或当前没法处理*/
#define CEC_OPCODE_FEATURE_ABORT 0X00
/**"Abort" Message This message is reserved for testing purposes.*/
/**CNcomment:"Abort"消息是专为测试保留的*/
#define CEC_OPCODE_ABORT_MESSAGE 0XFF
#define CEC_OPCODE_USER_DEFINE_MSG 0XFF
/* One Touch Play Feature*/

/**"Active Source" Used by a new source to indicate that it has started to transmit a stream OR used in response to a "Request Active Source"*/
/**CNcomment:"Active Source"消息说明源端设备正在发送一条码流或者回复"Request Active Source"消息*/
#define CEC_OPCODE_ACTIVE_SOURCE 0X82
/**"Image View On" Sent by a source device to the TV whenever it enters the active state (alternatively it may send "Text View On").*/
/**CNcomment:当源端设备进入激活状态时，会给TV发送一次"Image View On"消息*/
#define CEC_OPCODE_IMAGE_VIEW_ON 0X04
/**"Text View On" As "Image View On", but should also remove any text, menus and PIP windows from the TV's display.*/
/**CNcomment:"Text View On" 和"Image View On"指令相似，但同时会从TV画面上关闭文字，菜单和画中画窗口*/
#define CEC_OPCODE_TEXT_VIEW_ON 0X0D

/* Routing Control Feature*/

/**"Inactive Source" Used by the currently active source to inform the TV that it has no video to be presented to the user, or is going into standby as the result of a local user command on the device. */
/**CNcomment: "Inactive Source"消息表明当前无码流播放或者因用户操作，设备要进入待机状态*/
#define CEC_OPCODE_INACTIVE_SOURCE 0X9D
/**"Request Active Source" Used by a new device to discover the status of the system.*/
/**CNcomment: "Request Active Source"消息用于通知系统添加了一个新设备*/
#define CEC_OPCODE_REQUEST_ACTIVE_SOURCE 0X85
/**"Routing Change" Sent by a CEC Switch when it is manually switched to inform all other devices on the network that the active route below the switch has changed. */
/**CNcomment: 当CEC转接器检测到子设备列表发生变化时，会通知所有活动子设备 "Routing Change"消息*/
#define CEC_OPCODE_ROUTING_CHANGE 0X80
/**"Routing Information" Sent by a CEC Switch to indicate the active route below the switch.*/
/**CNcomment: CEC转接器发送"Routing Information"消息来检测子网络下活动子设备*/
#define CEC_OPCODE_ROUTING_INFORMATION 0X81
/**"Set Stream Path" Used by the TV to request a streaming path from the specified physical address.*/
/**CNcomment: TV从一个特定的设备上获取码流路径 */
#define CEC_OPCODE_SET_STREAM_PATH 0X86

/* Standby Feature*/

/**"Standby" Switches one or all devices into standby mode. Can be used as a broadcast message or be addressed to a specific device.See section CEC 13.3 for important notes on the use of this message */
/**CNcomment: "Standby"消息能使一个或多个设备进入待机状态。可用广播或者给特定的设备单独发送*/
#define CEC_OPCODE_STANDBY 0X36

/* One Touch Record Feature*/

/**"Record Off" Requests a device to stop a recording. */
/**CNcomment: "Record Off"能使一个设备停止录像*/
#define CEC_OPCODE_RECORD_OFF 0X0B
/**"Record On" Attempt to record the specified source. */
/**CNcomment: "Record On"消息用于尝试让一个特定的源录像*/
#define CEC_OPCODE_RECORD_ON 0X09
/**"Record Status" Used by a Recording Device to inform the initiator of the message "Record On" about its status. */
/**CNcomment: 可录像设备发送"Record Status"消息和状态给发起"Record On"消息的的设备*/
#define CEC_OPCODE_RECORD_STATUS 0X0A
/**"Record TV Screen" Request by the Recording Device to record the presently displayed source. */
/**CNcomment: "Record TV Screen"消息用于请求录像设备记录当前显示的资源*/
#define CEC_OPCODE_RECORD_TV_SCREEN 0X0F

/* Timer Programming Feature*/

/**"Clear Analogue Timer" Used to clear an Analogue timer block of a device. */
/**CNcomment: 清空模拟定时器设备*/
#define CEC_OPCODE_CLEAR_ANALOGUE_TIMER 0X33
/**"Clear Digital Timer" Used to clear a Digital timer block of a device. */
/**CNcomment: 清空数字定时器设备*/
#define CEC_OPCODE_CLEAR_DIGITAL_TIMER 0X99
/**"Clear External Timer" Used to clear an External timer block of a device. */
/**CNcomment: 清空外部定时器设备*/
#define CEC_OPCODE_CLEAR_EXTERNAL_TIMER 0XA1
/**"Set Analogue Timer" Used to set a single timer block on an Analogue Recording Device. */
/**CNcomment: 在模拟定时器设备上设置定时器*/
#define CEC_OPCODE_SET_ANALOGUE_TIMER 0X34
/**"Set Digital Timer" Used to set a single timer block on a Digital Recording Device. */
/**CNcomment: 在数字定时器设备上设置定时器*/
#define CEC_OPCODE_SET_DIGITAL_TIMER 0X97
/**"Set External Timer" Used to set a single timer block to record from an external device. */
/**CNcomment: 在外部定时器设备上设置定时器*/
#define CEC_OPCODE_SET_EXTERNAL_TIMER 0XA2
/**"Set Timer Program Title" Used to set the name of a program associated with a timer block. Sent directly after sending a "Set Analogue Timer" or "Set Digital Timer" message. The name is then associated with that timer block. */
/**CNcomment: 当发送完"Set Analogue Timer" 或"Set Digital Timer"消息后发送一个名称用于程序和定时器模块关联 */
#define CEC_OPCODE_SET_TIMER_PROGRAM_TITLE 0X67
/**"Timer Cleared Status" Used to give the status of a "Clear Analogue Timer", "Clear Digital Timer" or "Clear External Timer" message. */
/**CNcomment: "Timer Cleared Status"消息用于发送"Clear Analogue Timer"，"Clear Digital Timer"，"Clear External Timer"之后的状态*/
#define CEC_OPCODE_TIMER_CLEARED_STATUS 0X43
/**"Timer Status" Used to send timer status to the initiator of a "Set Timer" message. */
/**CNcomment: "Timer Status"消息用于给Set Timer"消息的发起者发送定时器状态*/
#define CEC_OPCODE_TIMER_STATUS 0X35

/* System Information Feature*/

/**"CEC Version" Used to indicate the supported CEC version, in response to a "Get CEC Version" */
/**CNcomment: "CEC Version"消息用于发送设备上的CEC版本信息来回复"Get CEC Version"消息*/
#define CEC_OPCODE_CEC_VERSION 0X9E
/**"Get CEC Version" Used by a device to enquire which version of CEC the target supports */
/**CNcomment: "Get CEC Version"用于一个设备获取从设备的CEC版本信息*/
#define CEC_OPCODE_GET_CEC_VERSION 0X9F
/**"Give Physical Address" A request to a device to return its physical address. */
/**CNcomment: 向一个设备请求获取该设备的物理地址*/
#define CEC_OPCODE_GIVE_PHYSICAL_ADDRESS 0X83
/**"Report Physical Address" Used to inform all other devices of the mapping between physical and logical address of the initiator.*/
/**CNcomment: 向同一网络下其他所有设备发送物理地址和逻辑地址*/
#define CEC_OPCODE_REPORT_PHYSICAL_ADDRESS 0X84
/**"Get Menu Language" Sent by a device capable of character generation (for OSD and Menus) to a TV in order to discover the currently selected Menu language.Also used by a TV during installation to discover the currently set menu language of other devices.*/
/**CNcomment: "Get Menu Language"用于获取Tv端能力集，用于替换当前的菜单语言类型；也可用于TV设备启动时，设置其它设备的菜单语言*/
#define CEC_OPCODE_GET_MENU_LANGUAGE 0X91
/**"Set Menu Language" Used by a TV or another device to indicate the menu language. */
/**CNcomment: "Set Menu Language"用于Tv或其他设备，设置菜单语言*/
#define CEC_OPCODE_SET_MENU_LANGUAGE 0X32

/*  Deck Control Feature*/

/**"Deck Control" Used to control a device's media functions. */
/**CNcomment: "Deck control"消息用于控制一个设备的多媒体功能*/
#define CEC_OPCODE_DECK_CONTROL 0X42
/**"Deck Status" Used to provide a deck's status to the initiator of the "Give Deck Status" message. */
/**CNcomment: "Deck Status "消息用于回复"Give Deck Status"消息的发起者Deck的状态*/
#define CEC_OPCODE_DECK_STATUS 0X1B
/**"Give Deck Status" Used to request the status of a device, regardless of whether or not it is the current active source. */
/**CNcomment: "Give Deck Status"消息请求获取目标设备的状态，而不管目标设备目前是不是激活的*/
#define CEC_OPCODE_GIVE_DECK_STATUS 0X1A
/**"Play" Used to control the playback behaviour of a source device. */
/**CNcomment: "Play"消息用于控制源设备播放*/
#define CEC_OPCODE_PLAY 0X41

/* Tuner Control Feature*/

/**"Give Tuner Device Status" Used to request the status of a tuner device. */
/**CNcomment: "Give Tuner Device Status"用于获取电视调谐器的状态*/
#define CEC_OPCODE_GIVE_TUNER_DEVICE_STATUS 0X08
/**"Select Analogue Service" Directly selects an Analogue TV service */
/**CNcomment: "Select Analogue Service"消息用于直接选择一个模拟电视的服务项*/
#define CEC_OPCODE_SELECT_ANALOGUE_SERVICE 0X92
/**"Select Digital Service" Directly selects a Digital TV, Radio or Data Broadcast Service */
/**CNcomment: "Select Digital Service"消息用于直接选择一个数字电视的服务项*/
#define CEC_OPCODE_SELECT_DIGITAL_SERVICE 0X93
/**"Tuner Device Status" Use by a tuner device to provide its status to the initiator of the "Give Tuner Device Status" message. */
/**CNcomment: "Tuner Device Status"消息用于电视调谐器给" Give Tuner Device Status"消息的发起端回复*/
#define CEC_OPCODE_TUNER_DEVICE_STATUS 0X07
/**"Tuner Step Decrement" Used to tune to next lowest service in a tuner's service list. Can be used for PIP. */
/**CNcomment: "Tuner Step Decrement"消息用于把次低的服务项放入调谐器的服务列表，能用于画中画*/
#define CEC_OPCODE_TUNER_STEP_DECREMENT 0X06
/**"Tuner Step Increment" Used to tune to next highest service in a tuner's service list. Can be used for PIP. */
/**CNcomment: "Tuner Step Decrement"消息用于把次高的服务项放入调谐器的服务列表，能用于画中画*/
#define CEC_OPCODE_TUNER_STEP_INCREMENT 0X05

/* Vendor Specific Command*/

/**"Device Vendor ID" Reports the vendor ID of this device. */
/**CNcomment: "Device Vendor ID"消息用于报告此设备的供应商ID*/
#define CEC_OPCODE_DEVICE_VENDOR_ID 0X87
/**"Give Device Vendor ID" Requests the Vendor ID from a device. */
/**CNcomment: "Give Device Vendor ID"消息用于请求此设备的供应商ID*/
#define CEC_OPCODE_GIVE_DEVICE_VENDOR_ID 0X8C
/**"Vendor Command" Allows vendor specific commands to be sent between two devices. */
/**CNcomment: "Vendor Command"用于两个设备之间互相发送供应商定制消息*/
#define CEC_OPCODE_VENDOR_COMMAND 0X89
/**"Vendor Command With ID" Allows vendor specific commands to be sent between two devices or broadcast. */
/**CNcomment: "Vendor Command With ID"用于两个设备之间或广播形式发送供应商定制消息*/
#define CEC_OPCODE_VENDOR_COMMAND_WITH_ID 0XA0
/**"Vendor Remote Button Down" Indicates that a remote control button has been depressed. */
/**CNcomment: "Vendor Remote Button Down"消息表示，遥控器有按键按下来*/
#define CEC_OPCODE_VENDOR_REMOTE_BUTTON_DOWN 0X8A
/**"Vendor Remote Button Up" Indicates that a remote control button (the last button pressed indicated by the Vendor Remote Button Down message) has been released. */
/**CNcomment: "Vendor Remote Button Up"消息表示 "Vendor Remote Button Down"消息最后一次表示过的按键被释放了*/
#define CEC_OPCODE_VENDOR_REMOTE_BUTTON_UP 0X8B

/* OSD Display Feature*/

/**"Set OSD String" Used to send a text message to output on a TV. */
/**CNcomment: "Set OSD String"消息可以发送一段文字消息让它显示在电视机上*/
#define CEC_OPCODE_SET_OSD_STRING 0X64
/**"Give OSD Name" Used to request the preferred OSD name of a device for use in menus associated with that device. */
/**CNcomment:请求首选的关联设备菜单显示的名字 */
#define CEC_OPCODE_GIVE_OSD_NAME 0X46
/**"Set OSD Name" Used to set the preferred OSD name of a device for use in menus associated with that device. */
/**CNcomment:设置首选关联设备菜单显示的名字 */
#define CEC_OPCODE_SET_OSD_NAME 0X47

/* Device Menu Control Feature*/

/**"Menu Request" A request from the TV for a device to show/remove a menu or to query if a device is currently showing a menu. */
/**CNcomment: "Menu Request"消息用于电视请求一个设备显示/关闭菜单或询问该设备当前有没有菜单显示*/
#define CEC_OPCODE_MENU_REQUEST 0X8D
/**"Menu Status" Used to indicate to the TV that the device is showing/has removed a menu and requests the remote control keys to be passed though. */
/**CNcomment: "Menu Status"消息用于回复电视端当前正显示/移除了菜单，并透传遥控器指令*/
#define CEC_OPCODE_MENU_STATUS 0X8E
/**"User Control Pressed" Used to indicate that the user pressed a remote control button or switched from one remote control button to another. */
/**CNcomment: "User Control Pressed"表示用户按了一个遥控器按键或从某一个按键换到另一个按键.也可以用于用户间接发起的指令*/
#define CEC_OPCODE_USER_CONTROL_PRESSED 0X44
/**"User Control Released" Indicates that user released a remote control button (the last one indicated by the "User Control Pressed" message) */
/**CNcomment: "User Control Released"表示用户释放了"User Control Released"消息指定的遥控器按键.也可以用于用户间接发起的指令*/
#define CEC_OPCODE_USER_CONTROL_RELEASED 0X45

/* Power Status Feature*/

/**"Give Device Power Status" Used to determine the current power status of a target device */
/**CNcomment: "Give Device Power Status"消息用于获取目标设备当前的电源状态*/
#define CEC_OPCODE_GIVE_DEVICE_POWER_STATUS 0X8F
/**"Report Power Status" Used to inform a requesting device of the current power status */
/**CNcomment: "Report Power Status"消息用于发送当前的电源状态回复请求设备*/
#define CEC_OPCODE_REPORT_POWER_STATUS 0X90

/* System Audio Control Feature*/

/**"Give Audio Status" Requests an amplifier to send its volume and mute status */
/**CNcomment: "Give Audio Status"消息请求获取扩音器的音量和静音状态*/
#define CEC_OPCODE_GIVE_AUDIO_STATUS 0X71
/**"Give System Audio Mode Status" Requests the status of the System Audio Mode */
/**CNcomment: "Give System Audio Mode Status"消息请求系统音频的状态*/
#define CEC_OPCODE_GIVE_SYSTEM_AUDIO_MODE_STATUS 0x7D
/**"Report Audio Status" Reports an amplifier's volume and mute status */
/**CNcomment: "Report Audio Status"消息用于发送扩音器的音量和静音状态*/
#define CEC_OPCODE_REPORT_AUDIO_STATUS 0X7A

#define CEC_OPCODE_REPORT_SHORT_AUDIO_DESCRIPTOR 0X73

#define CEC_OPCODE_REQUEST_SHORT_AUDIO_DESCRIPTOR 0X74

/**"Set System Audio Mode" Turns the System Audio Mode On or Off. */
/**CNcomment: "Set System Audio Mode"消息用于打开/关闭系统音频功能*/
#define CEC_OPCODE_SET_SYSTEM_AUDIO_MODE 0X72
/**"System Audio Mode Request" A device implementing System Audio Control and which has volume control RC buttons (eg TV or STB) requests to use System Audio Mode to the amplifier */
/**CNcomment: 使用音量控制按钮(stb或者tv)需要扬声器进入 System Audio Mode*/
#define CEC_OPCODE_SYSTEM_AUDIO_MODE_REQUEST 0X70
/**"System Audio Mode Status" Reports the current status of the System Audio Mode */
/**CNcomment: "System Audio Mode Status"消息用于发送系统音频当前的状态*/
#define CEC_OPCODE_SYSTEM_AUDIO_MODE_STATUS 0X7E

/* Audio Rate Control Feature*/

/**"Set Audio Rate" Used to control audio rate from Source Device. */
/**CNcomment: "Set Audio Rate"消息用于控制源端设备的音频采样率*/
#define CEC_OPCODE_SET_AUDIO_RATE 0X9A


/**"Initiate ARC" Used by an ARC RX device to activate the ARC functionality in an ARC TX Device. */
/**CNcomment: "Initiate ARC"消息被ARC RX用于激活ARC TX设备的ARC功能所用*/
#define CEC_OPCODE_INITIATE_ARC 0XC0

#define CEC_OPCODE_REPORT_ARC_INITIATED 0XC1

#define CEC_OPCODE_REPORT_ARC_TERMINATED 0XC2

#define CEC_OPCODE_REQUEST_ARC_INITIATION 0XC3

#define CEC_OPCODE_REQUEST_ARC_TERMINATION 0XC4

#define CEC_OPCODE_TERMINATE_ARC 0XC5


/**POLL message have no opcode, So, we just use this value */
/**CNcomment: "POLL"消息没有操作码，我们仅仅只是使用该值*/
#define CEC_OPCODE_POLLING_MESSAGE 0XFE

/**HDMI CEC logical address,please reference  HDMI specification 1.4a*/
/**CNcomment: HDMI CEC逻辑地址，请参考HDMI 1.4a协议 */
typedef enum mtUNF_CEC_LOGICALADD_S {
    MT_UNF_CEC_LOGICALADD_TV = 0X00,
    /**<TV*/ /**CNcomment:<电视 */
    MT_UNF_CEC_LOGICALADD_RECORDDEV_1 = 0X01,
    /**<Record device 1*/ /**CNcomment:<录像机 1 */
    MT_UNF_CEC_LOGICALADD_RECORDDEV_2 = 0X02,
    /**<Record device 2*/ /**CNcomment:<录像机 2 */
    MT_UNF_CEC_LOGICALADD_TUNER_1 = 0X03,
    /**<Tuner 1*/ /**CNcomment:<高频头 1*/
    MT_UNF_CEC_LOGICALADD_PLAYDEV_1 = 0X04,
    /**<play device 1*/ /**CNcomment:<播放设备 1 */
    MT_UNF_CEC_LOGICALADD_AUDIOSYSTEM = 0X05,
    /**<audio system*/ /**CNcomment:<音频系统 */
    MT_UNF_CEC_LOGICALADD_TUNER_2 = 0X06,
    /**<tuner 2*/ /**CNcomment:<高频头 2 */
    MT_UNF_CEC_LOGICALADD_TUNER_3 = 0X07,
    /**<tuner 3*/ /**CNcomment:<高频头 3 */
    MT_UNF_CEC_LOGICALADD_PLAYDEV_2 = 0X08,
    /**<play device 2*/ /**CNcomment:<播放设备 2 */
    MT_UNF_CEC_LOGICALADD_RECORDDEV_3 = 0X09,
    /**<Record device 3*/ /**CNcomment:<录像机 3 */
    MT_UNF_CEC_LOGICALADD_TUNER_4 = 0X0A,
    /**<tuner 4*/ /**CNcomment:<高频头 4 */
    MT_UNF_CEC_LOGICALADD_PLAYDEV_3 = 0X0B,
    /**<play device 3*/ /**CNcomment:<播放设备 3 */
    MT_UNF_CEC_LOGICALADD_RESERVED_1 = 0X0C,
    /**<reserved 1*/ /**CNcomment:<保留项 1 */
    MT_UNF_CEC_LOGICALADD_RESERVED_2 = 0X0D,
    /**<reserved 2*/ /**CNcomment:<保留项 2 */
    MT_UNF_CEC_LOGICALADD_SPECIALUSE = 0X0E,
    /**<special use*/ /**CNcomment:<特殊用途 */
    MT_UNF_CEC_LOGICALADD_BROADCAST = 0X0F,
    /**<broadcast*/ /**CNcomment:<广播 */
    MT_UNF_CEC_LOGICALADD_BUTT
} MT_UNF_CEC_LOGICALADD_S;

/**HDMI CEC command type,please reference  HDMI specification 1.4a*/
/**CNcomment: HDMI CEC命令类型，请参考HDMI 1.4a协议 */
typedef enum mtUNF_CEC_CMDTYPE_E {
    MT_UNF_CEC_STRUCTCOMMAND,
    /**<CEC struct command*/ /**<CNcomment:CEC 结构命令 */
    MT_UNF_CEC_RAWCOMMAND,
    /**<CEC raw command*/ /**<CNcomment:CEC 原始命令 */
    MT_UNF_CEC_BUTT
} MT_UNF_CEC_CMDTYPE_E;

/**HDMI CEC Raw Data struct,please reference  HDMI specification 1.4a*/
/**CNcomment: HDMI CEC原始数据结构体，请参考HDMI 1.4a协议 */
typedef struct mtUNF_CEC_RAWDATA_S
{
    mt_u8 u8Length; /**<CEC raw data lengh*/ /**<CNcomment:cec 有效参数个数 */
    mt_u8 u8Data[15]; /**<CEC raw data*/     /**<CNcomment:CEC 参数结构体 */
} MT_UNF_CEC_RAWDATA_S;

/**HDMI CEC user Interface Command Opcode,please reference  HDMI specification 1.4a*/
/**CNcomment: HDMI CEC 用户接口操作指令，请参考HDMI 1.4a协议 */
typedef enum mtUNF_CEC_UICMD_E {
    MT_UNF_CEC_UICMD_SELECT = 0x00,
    MT_UNF_CEC_UICMD_UP = 0x01,
    MT_UNF_CEC_UICMD_DOWN = 0x02,
    MT_UNF_CEC_UICMD_LEFT = 0x03,
    MT_UNF_CEC_UICMD_RIGHT = 0x04,
    MT_UNF_CEC_UICMD_RIGHT_UP = 0x05,
    MT_UNF_CEC_UICMD_RIGHT_DOWN = 0x06,
    MT_UNF_CEC_UICMD_LEFT_UP = 0x07,
    MT_UNF_CEC_UICMD_LEFT_DOWN = 0x08,
    MT_UNF_CEC_UICMD_ROOT_MENU = 0x09,
    MT_UNF_CEC_UICMD_SETUP_MENU = 0x0A,
    MT_UNF_CEC_UICMD_CONTENTS_MENU = 0x0B,
    MT_UNF_CEC_UICMD_FAVORITE_MENU = 0x0C,
    MT_UNF_CEC_UICMD_EXIT = 0x0D,
    MT_UNF_CEC_UICMD_NUM_0 = 0x20,
    MT_UNF_CEC_UICMD_NUM_1 = 0x21,
    MT_UNF_CEC_UICMD_NUM_2 = 0x22,
    MT_UNF_CEC_UICMD_NUM_3 = 0x23,
    MT_UNF_CEC_UICMD_NUM_4 = 0x24,
    MT_UNF_CEC_UICMD_NUM_5 = 0x25,
    MT_UNF_CEC_UICMD_NUM_6 = 0x26,
    MT_UNF_CEC_UICMD_NUM_7 = 0x27,
    MT_UNF_CEC_UICMD_NUM_8 = 0x28,
    MT_UNF_CEC_UICMD_NUM_9 = 0x29,
    MT_UNF_CEC_UICMD_DOT = 0x2A,
    MT_UNF_CEC_UICMD_ENTER = 0x2B,
    MT_UNF_CEC_UICMD_CLEAR = 0x2C,
    MT_UNF_CEC_UICMD_NEXT_FAVORITE = 0x2F,
    MT_UNF_CEC_UICMD_CHANNEL_UP = 0x30,
    MT_UNF_CEC_UICMD_CHANNEL_DOWN = 0x31,
    MT_UNF_CEC_UICMD_PREVIOUS_CHANNEL = 0x32,
    MT_UNF_CEC_UICMD_SOUND_SELECT = 0x33,
    MT_UNF_CEC_UICMD_INPUT_SELECT = 0x34,
    MT_UNF_CEC_UICMD_DISPLAY_INFORMATION = 0x35,
    MT_UNF_CEC_UICMD_HELP = 0x36,
    MT_UNF_CEC_UICMD_PAGE_UP = 0x37,
    MT_UNF_CEC_UICMD_PAGE_DOWN = 0x38,
    MT_UNF_CEC_UICMD_POWER = 0x40,
    MT_UNF_CEC_UICMD_VOLUME_UP = 0x41,
    MT_UNF_CEC_UICMD_VOLUME_DOWN = 0x42,
    MT_UNF_CEC_UICMD_MUTE = 0x43,
    MT_UNF_CEC_UICMD_PLAY = 0x44,
    MT_UNF_CEC_UICMD_STOP = 0x45,
    MT_UNF_CEC_UICMD_PAUSE = 0x46,
    MT_UNF_CEC_UICMD_RECORD = 0x47,
    MT_UNF_CEC_UICMD_REWIND = 0x48,
    MT_UNF_CEC_UICMD_FAST_FORWARD = 0x49,
    MT_UNF_CEC_UICMD_EJECT = 0x4A,
    MT_UNF_CEC_UICMD_FORWARD = 0x4B,
    MT_UNF_CEC_UICMD_BACKWARD = 0x4C,
    MT_UNF_CEC_UICMD_STOP_RECORD = 0x4D,
    MT_UNF_CEC_UICMD_PAUSE_RECORD = 0x4E,
    MT_UNF_CEC_UICMD_ANGLE = 0x50,
    MT_UNF_CEC_UICMD_SUBPICTURE = 0x51,
    MT_UNF_CEC_UICMD_VIDEO_ON_DEMAND = 0x52,
    MT_UNF_CEC_UICMD_ELECTRONIC_PROGRAM_GUIDE = 0x53,
    MT_UNF_CEC_UICMD_TIMER_PROGRAMMING = 0x54,
    MT_UNF_CEC_UICMD_INITIAL_CONFIGURATION = 0x55,
    MT_UNF_CEC_UICMD_PLAY_FUNCTION = 0x60,
    MT_UNF_CEC_UICMD_PAUSE_PLAY_FUNCTION = 0x61,
    MT_UNF_CEC_UICMD_RECORD_FUNCTION = 0x62,
    MT_UNF_CEC_UICMD_PAUSE_RECORD_FUNCTION = 0x63,
    MT_UNF_CEC_UICMD_STOP_FUNCTION = 0x64,
    MT_UNF_CEC_UICMD_MUTE_FUNCTION = 0x65,
    MT_UNF_CEC_UICMD_RESTORE_VOLUME_FUNCTION = 0x66,
    MT_UNF_CEC_UICMD_TUNE_FUNCTION = 0x67,
    MT_UNF_CEC_UICMD_SELECT_MEDIA_FUNCTION = 0x68,
    MT_UNF_CEC_UICMD_SELECT_AV_INPUT_FUNCTION = 0x69,
    MT_UNF_CEC_UICMD_SELECT_AUDIO_INPUT_FUNCTION = 0x6A,
    MT_UNF_CEC_UICMD_POWER_TOGGLE_FUNCTION = 0x6B,
    MT_UNF_CEC_UICMD_POWER_OFF_FUNCTION = 0x6C,
    MT_UNF_CEC_UICMD_POWER_ON_FUNCTION = 0x6D,
    MT_UNF_CEC_UICMD_F1_BLUE = 0x71,
    MT_UNF_CEC_UICMD_F2_RED = 0x72,
    MT_UNF_CEC_UICMD_F3_GREEN = 0x73,
    MT_UNF_CEC_UICMD_F4_YELLOW = 0x74,
    MT_UNF_CEC_UICMD_F5 = 0x75,
    MT_UNF_CEC_UICMD_DATA = 0x76
} MT_UNF_CEC_UICMD_E;

/**HDMI CEC operand command,please reference  HDMI specification 1.4a*/
/**CNcomment: HDMI CEC 操作指令，请参考HDMI 1.4a协议 */
typedef struct mtUNF_CEC_Operand_t
{
    MT_UNF_CEC_RAWDATA_S stRawData; /**<CEC raw date*/              /**<CNcomment:CEC 原始命令 */
    MT_UNF_CEC_UICMD_E stUIOpcode; /**<CEC user interface command*/ /**<CNcomment:CEC用户自定义操作 */
} MT_UNF_CEC_Operand_t;

/**HDMI CEC struct command*/
/**CNcomment: HDMI CEC 命令结构 */
typedef struct mtUNF_HDMI_CEC_CMD_S
{
    MT_UNF_CEC_LOGICALADD_S enSrcAdd; /**<logical address of source */     /**<CNcomment:源端设备地址 */
    MT_UNF_CEC_LOGICALADD_S enDstAdd; /**<logical address of destination*/ /**<CNcomment:目标设备地址 */
    mt_u8 u8Opcode; /**<opration code*/                                    /**<CNcomment:操作码*/
    MT_UNF_CEC_Operand_t unOperand; /**<operand*/                          /**<CNcomment:操作数*/
	//ld : cecsetdata not use in v2.0, it should be delete
	mt_u8 cecSetData[17]; /*set cec data for send data test*/
} MT_UNF_HDMI_CEC_CMD_S;

/**HDMI CEC status struct*/
/**CNcomment: HDMI CEC 状态结构 */
typedef struct mtUNF_HDMI_CEC_STATUS_S
{
    MT_BOOL bEnable; /**<the flag of CEC work,MT_TRUE:CEC work enable,MT_FALSE:CEC no work ,other parameter no effect*/ /**<CNcomment:CEC 正常工作标记位，如果为MT_TRUE,CEC可以正常工作，如果为MT_FASLE,CEC的其他参数无效，并且不能工作 */
    mt_u8 u8PhysicalAddr[4]; /**<CEC physics address*/                                                                  /**<CNcomment:CEC 物理地址 */
    mt_u8 u8LogicalAddr; /**<CEC logic address,defualt 0x03*/                                                           /**<CNcomment:CEC 逻辑地址，默认为：0x03. */
    mt_u8 u8Network[MT_UNF_CEC_LOGICALADD_BUTT]; /**<CEC network struct ,1:the device can response CEC command*/        /**<CNcomment:CEC 构建的网路结构，为1表示该设备能够响应CEC命令 */
} MT_UNF_HDMI_CEC_STATUS_S;

/**HDMI CEC Regcallback param struct */
/**CNcomment: HDMI CEC 回调函数参数结构 */
typedef mt_void (*MT_UNF_HDMI_CECCALLBACK)(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pstCECCmd, mt_void *pData);

/**HDMI HDCP key struct*/
/**CNcomment: HDMI HDCP 密钥结构 */
typedef struct mtUNF_HDMI_LOAD_KEY_S
{
    mt_u8 *pu8InputEncryptedKey; /**<Encrypted key pointer */ /**<CNcomment:加密的密钥数组头地址 */
    mt_u32 u32KeyLength; /**<Encrypted key length*/           /**<CNcomment:加密的密钥长度 */
} MT_UNF_HDMI_LOAD_KEY_S;

/**HDMI Delay struct*/
/**CNcomment: HDMI 延时结构 */
typedef struct mtUNF_HDMI_DELAY_S
{
    mt_u32 u32MuteDelay; /**<delay for avmute */              /**<CNcomment:avmute的延时 */
    mt_u32 u32FmtDelay; /**<delay for setformat */            /**<CNcomment:设置制式的延时 */
    MT_BOOL bForceFmtDelay; /**<force setformat delay mode */ /**<CNcomment:切换制式强制延时模式 */
    MT_BOOL bForceMuteDelay; /**<force avmute delay mode */   /**<CNcomment:mute强制延时模式 */
} MT_UNF_HDMI_DELAY_S;


/*!
     set the hdmi blank ctrl mode by  HDMI_BLANK_LINE_CTRL & HDMI_BLANK_LINE_SET

   */
typedef enum
{
  /*!
        disable the blank ctrl mode
     */
  HDMI_BLACK_CTRL_MODE_DISABLE,

  /*!
         the blank ctrl mode 0,
     */
  HDMI_BLACK_CTRL_MODE_0,

  /*!
         the blank ctrl mode 1
     */
  HDMI_BLACK_CTRL_MODE_1,

  /*!
         the blank ctrl mode 2. after blank line number 16 line
         if mode 2 or mode 3, need set a blank line number as a start line number, use HDMI_BLANK_LINE_SET
     */
  HDMI_BLACK_CTRL_MODE_2,

  /*!
        the blank ctrl mode 3, after blank line number 32 line
        if mode 2 or mode 3, need set a blank line number as a start line number, use HDMI_BLANK_LINE_SET
     */
  HDMI_BLACK_CTRL_MODE_3,
}BLACK_CTRL_MODE_E;


/*!
  HDMI io control command
  */
typedef enum
{
  /*!
    power control, parameter is TRUE means power down or FALSE means power up
    Deprecated Functions, unimplemented,  no used now
    */
  HDMI_POWER_DOWN,

  /*!
    HDMI transit enable or disable,   screent will be mute when disable output
    data output control, parameter is TRUE means enable output or FALSE means disable
    */
  HDMI_OUTPUT_ENABLE,

  /*!
    setting Source Product Description infoframe, for parameter please see hdmi_spd_info_t
    Deprecated Functions, unimplemented, no used now
    */
  HDMI_SPD_INFO_SET,

  /*!
    setting phy pll configuration, for parameter please see hdmi_phy_pll_t
    Deprecated Functions, unimplemented, no used now
    */
  HDMI_FHY_PLL_SET,

  /*!
    getting phy pll configuration, for parameter please see hdmi_phy_pll_t
    Deprecated Functions, unimplemented, no used now
    */
  HDMI_FHY_PLL_GET,

  /*!
    getting EDID information, for parameter please see hdmi_edid_result_t
    This EDID info is parsed and saved when hotplug asserted
    if want to force get EDID from ddc channel, use HDMI_EDID_FORCE_PARSE
    */
  HDMI_EDID_GET,

  /*!
    force to get and parse EDID information from ddc channel
    This EDID info is parsed and saved when hotplug asserted
    unimplemented, no used now
    */
  HDMI_EDID_FORCE_PARSE,

  /*!
    force to get the edid raw data,  need 256 bytes buf
    unimplemented, no used now
    */
  HDMI_EDID_RAW_DATA_GET,

  /*!
    assumed that hotplug is asserted
    Deprecated Functions,
    */
  HDMI_FORCE_PLUG_IN,

  /*!
    copy right protection enable/disable, param is TRUE means enable, FALSE means disable
    Deprecated Functions, unimplemented,  no used now
    */
  HDMI_HDCP_CP_ENABLE,

  /*!
    get currrent hdcp authentication status,  it's parameter please see hdmi_hdcp_auth_status_t
    hdcp authentication process will be repeated  every about per 2 seconds.
    check the HDCP authentication  pass or failed at present.
    Enums = 10
    */
  HDMI_HDCP_STATUS_GET,

  /*!
    get hdcp capacity of the tv: return param: 1 tv support hdcp, 0 tv unsupport hdcp
    this status come from checking the tv's BKSV data, which getted from tv by hdmi ddc/i2c bus when hotpluging and after reading the edid data.
    */
  HDMI_HDCP_CAPACITY_GET,

  /*!
     dump the HDMI controller's register vaule,
     this cmd will print page0 and page1 register value, so only used at debug lib
     you can direct to read hdmi register and print it as well.
     the Page0: base addr=0xBF480000: offset: 00--FF,
     the Page1: base addr=0xBF480100: offset: 00--FF,
     see the drvtest code.
    */
  HDMI_REGISTER_VALUE_DUMP,

  /*!
    set mute status, parameter is TRUE means mute output or FALSE means unmute
    THis will mute the HDMI tmds clock, lead to the screen no concent to show
    */
  HDMI_AV_MUTE,

  /*!
    notify register, for parameter please see hdmi_notify_info_t
    Deprecated Functions, unimplemented,  no used now
    used the interface hdmi_notify_register to replace it
    */
  HDMI_NOTIFY_REGISTER,

  /*!
    notify register, for parameter please see hdmi_event_id_t
    Deprecated Functions, unimplemented,  no used now
    used the interface hdmi_notify_unregister to replace it
    */
  HDMI_NOTIFY_UNREGISTER,

  /*!
    notify register, for parameter please see hdmi_event_id_t
    Deprecated Functions, unimplemented,  no used now
    hdmi events will be notifyed the callback function registered in the interface hdmi_notify_register
    */
  HDMI_NOTIFY_GET_EVENTS,

  /*!
    open config
    Deprecated Functions, unimplemented,  no used now
    */
  HDMI_OPEN_CONFIG,

  /*!
    get HDMI sink status, 1 sink connected, 0 not connected
    check the hdmi sink device is connected or not
    */
  HDMI_IS_SINK_CONNECTED,

  /*!
    get HDMI audio source
    */
  HDMI_GET_AUD_SOURCE,

  /*!
    mute HDMI audio,  ture make hdmi audio mute, false make hdmi audio unmute,
    Enums = 20
    */
  HDMI_SET_AUD_MUTE,

  /*!
    create and start HDMI main service thread
    */
  HDMI_START_MAIN_SERVICE,

  /*!
     set hdcp function onoff, this cmd will lead hdmi transaction reset, tv screen will mute a monent
    */
  HDMI_SET_HDCP,

  /*!
       set hdcp function onoff but no hdmi mute , hdmi tmds will not reset, tv screent  continuously show content
       Deprecated Functions, unimplemented,  in symphony chip, this has bug, will lead screen show a mosaic video frame
      */
  HDMI_SET_HDCP_ON_OFF,

  /*!
    get the status of the  hdcp function on or off
    param: MT_BOOL hdcp_on_off
    */
  HDMI_GET_HDCP,

  /*!
    cfg audio for fastlogo mode
    */
  HDMI_CFG_AUDIO_4_FASTLOGO,

  /*!
    // Deprecated Functions,  this function had been removed
    // 0xFF no_user setting,hdcp/encryption is default: when hdcp_auth pass,encryption enable ,
    // 1: for nor_tv: encryption always disable, for no_hdcp_tv: hdcp is disabled
    // 0: for nor_tv: encryption enable when hdcp_auth pass, for no_hdcp_tv: hdcp able
    // must setting in maincode
   */
  HDMI_FORCE_DIS_ENCRYPTION,

  /*!
      set cec logitcal address, cec logical address value range ref:  HDMI_GET_CEC_LOGICAL_ADDR
      this value generally getted by HDMI_GET_CEC_LOGICAL_ADDR
      do not change it arbitrary
      you must not set the locigcal addr, because drv will auto allocate the logical  address  afer hotplug in
      This logic addr must not changed by uplatyer
      this iocmd now is deserted, if for debug, replace by HDMI_SET_LA
   */
  HDMI_SET_CEC_LOGICAL_ADDR,

  /*!
      cec wakeup tv msg
      send a image view on msg to wakeup tv
      pls see the drv testcase code
   */
  HDMI_CEC_TV_WAKEUP,

  /*!
      cec  active source
      send active source cmd to tv.
   */
  HDMI_CEC_TV_ACTIVE_SOURCE,

  /*!
      cec standby tv
      send a standby cmd to tv
      tv will do standby,  tv maybe sendback a standby cmd to STB,
      notify STB to enter the stb-standby flow
      pls see the drv testcase code
      Enums = 30
   */
  HDMI_CEC_TV_STANDBY,

  /*!
      cec volume +
      send a volume + cmd to tv
      tv will do volume +
      pls see the drv testcase code
   */
  HDMI_CEC_TV_VOLINCREASE,

  /*!
      cec volume -
      send a volume - cmd to tv
      tv will do volume -
      pls see the drv testcase code
   */
  HDMI_CEC_TV_VOLDECREASE,

  /*!
      cec  volume mute key to tv
      tv will do mute or unmute
      pls see the drv testcase code
   */
  HDMI_CEC_TV_VOL_MUTE,

  /*!
      cec  send a user control code, defined in  HDMI 1.4a spec: cec table 30 user control codes, page 348
      Deprecated Functions, maybe has some bug
   */
  HDMI_CEC_USER_CTROL_CODE,

  /*!
      cec  send a message data to tv
      Deprecated Functions, maybe has some bug
   */
  HDMI_CEC_MESSAGE_SEND,

  /*!
      set cec enable or disable, if disabled, drv will do not received the cec msg from tv
      you can use this fucntion to disable cec, if you do not want to use the cec
      if you want to enable cec after disabled this function, set HDMI_CEC_RECV_ENABLE enable, and then call HDMI_START_CEC_SERVICE to restart the cec servce
   */
  HDMI_CEC_RECV_ENABLE,

  /*!
      get logical address of cec device
      this logical is allocation in drv
      cec logical valid value range is 0x01---0x0F,  0x00 used for tv
      we only alternativly used the 0x03, 0x06, 0x07, 0x0a.
   */
  HDMI_GET_CEC_LOGICAL_ADDR,

  /*!
        poll the event,  fix bug 99011 end
        Deprecated Functions
        no implemented
     */
  HDMI_POLL_EVENT,

  /*!
      cec  get logtical address
   */
  HDMI_USRDF_INFOFRAME_SET,

  /*!
      cec  get logtical address
      Enums = 40
   */
  HDMI_USRDF_INFOFRAME_CLEAR,

  /*!
      set hdmi warm reset enable or disable
      default is disable, param = true indicate to enable warm reset
      This function is deserted now
   */
  HDMI_WARM_RESET_ENABLE_SET,

  /*!
      check if the hotplug in or out status by hdmi hw
   */
  HDMI_CHECK_HOTPLUG_STATUS,

  /*!
      check if the TV active sense status by hdmi hw
   */
  HDMI_CHECK_TV_ACTVIE_SENSE_STATUS,

  /*!
      check if the encryption onoff  status
   */
  HDMI_CHECK_ENCRYPTION_STATUS,

  /*!
      check if the cec is ready or not,  you can send cec message just until cec ready
      this param is E_CEC_STATUS_T
    */
  HDMI_CHECK_CEC_STATUS,

  /*!
      set the hdmi phy mute or unmute
   */
  HDMI_PHY_MUTE_SET,

  /*!
      set the hdmi tmds mute or unmute
      Deprecated Functions,  pls do'not  mute tmds
   */
  HDMI_TMDS_MUTE_SET,

  /*!
      set the hdmi bg color enable or disable, when hdcp verify failed
      you need call the HDMI_BGCOLORE_DATA_SET first
   */
  HDMI_BGCOLORE_ENABLE_SET,

  /*!
      set the hdmi bg color data value,  a u32 data for rgb888, yuv
      when hdmi av-mute or hdmi video mute, tmds data will filled by this color
      color format: a u32 data, contain 4 bytes: 0x-cs-c1-c2-c3:  cs = hdmi_video_color_space_t, color data c1,c2,c3 is one byte according to it's format
   */
  HDMI_BGCOLORE_DATA_SET,

  /*!
      set  the function disable or enable:  check edid parse err such as  checksum, head data
      if set ture,  disable this check function , hdmi dirver don't check the edid parse err.
      0 is closed, 1 will dont check minor error.
      Enums = 50
     */
  HMDI_EDID_PARSE_ERR_CHECK_DISABLE,

  /*!
        set the hdmi av-mute,  send  av-mute ctrl packet  once
        HDMI_SET_AUD_MUTE, will send av-mute ctrl packet consecutively,
        HDMI_SET_AV_MUTE_ONCE only send a av-mute ctrl packet
        this cmd will lead some tv av-mute 2-3 seconds, and than the tv will auto av-unmute .
     */
  HDMI_SET_AV_MUTE_ONCE,

  /*!
        set the hdmi audio transfer enable or disable, ture is enable
     */
  HDMI_SET_AUD_ENABLE,

  /*!
        set the hdmi video transfer enable or disable, ture is enable
     */
  HDMI_SET_VIDEO_ENABLE,

  /*!
      set the hdmi video mute or disable, ture is mute, false is unmute
      this will lead hdmi transfer the "blue screen color" to tv
   */
  HDMI_SET_VIDEO_MUTE,

  /*!
      set the hdmi hotplog out process policy,   54
      if 1,  when hotplug out, hdmi will ouput normal
      if 0,  when hotplug out, hdmi will stop output
   */
  HDMI_SET_HDP_OUT_POLICY,

  /*!
     start cec service
   */
  HDMI_START_CEC_SERVICE,

  /*!
     send a cec ping tv msg
   */
  HDMI_CEC_PING_TV_MSG,

  /*!
     check cec is rx idle and tx idle status
     before send view on, active source ... you need check cec idle
   */
   HDMI_CEC_IDLE_CHECK,

  /*!
      set the hdmi tx enable flag,  when hdcp auth failed
      ture, enable tx, when auth failed
      false, disable tx, when auth failed
     */
  HDMI_TX_ENABLE_AUTH_FAILED,

  /*!
      cec  inactive source
      send inactive source cmd to tv.
      no param
      Enums = 60
   */
  HDMI_CEC_TV_INACTIVE_SOURCE,

  /*!
      set the cec  power status
      param: HDMI_PW_STATUS_E
   */
  HDMI_CEC_STB_POWER_STATUS_SET,

  /*!
     get hdmi port  captured by cec set stream path msg
     */
  HDMI_GET_TV_HDMI_PORT,

  /*!
        set the hdmi phy reset
     */
  HDMI_PHY_RESET,

  /*!
      set the hdmi av mute check feature supported flag, default value is disable,
      if set to enable, hdmi-av-mute cmd will not be sent to tv
      if the TV shows a bad effect for the av-mute cmd, you can set HDMI_AVMUTE_CHECK_SET enable
      and than call HDMI_AVMUTE_CHECK_EDID_ADD , addng the tv's EDID
      param: TRUE-->enable, FALSE-->disable,  defualt is FALSE, donot check
   */
  HDMI_AVMUTE_CHECK_SET,

  /*!
      set the tv'edid, byte 8-18, total 10 byyes, if the tv will not support the hdmi av-mute
      param: a 10  byte array
   */
  HDMI_AVMUTE_TV_EDID_ADD,

  /*!
      remove all edid setting: HDMI_AVMUTE_TV_EDID_ADD
   */
  HDMI_AVMUTE_TV_EDID_RM_ALL,

  /*!
    check the edid is some special TV
    param is HDMI_SPECIAL_TV_E
    iocmd return SUCCESS, means it is the tv
   */
  HDMI_IS_SPECIAL_TV_CHECK,

  /*!
      set osd name showed on tv.
      If hdmi is already in, only change g_hdmi_cfg.osd_name, do nothing else.
      The tv must send give osd name, stb send the new osd name to tv, tv shows new osd name.
   */
  HDMI_CEC_SET_OSD_NAME,

  /*!
      hdmi colorbar
   */
  HDMI_VEDIO_COLORBAR,


  /*!
      flowing hdmi ioctrl cmd used for debug
      Enums = 100
   */
  HDMI_DEUB_CMD_START = 100,

  /*!
      set  enable or disable for detecting  hotplug in/out , used  for debug those tv, which have no hotplug isr signal
      set ture, for disable hotplug detecting, set false  to enable hotplug dectecting
   */
  HDMI_HOTPLUG_DET_DISABLE,

  /*!
      set  enable or disable for detecting  Rx sense active , used  for debug those tv, which have no Rx sense active isr signal
      set ture, for disable Rx sense active detecting, set false  to enable Rx sense active dectecting
   */
  HDMI_RX_SENSE_ACTIVE_DET_DISABLE,

  /*!
      set  enable or disable for debug , default is enable
   */
  HDMI_SET_882E_ENABLE,

   /*!
      set  la, like HDMI_SET_CEC_LOGICAL_ADDR, for debug
   */
  HDMI_SET_LA,

  /*!
      set  pa
   */
  HDMI_SET_PA,

/*!
      set  hdmi device type:  0 is dvi,  1 is hdmi
   */
  HDMI_DEVICE_TYPE,
  /*!
      set  30 bit enable
   */
  HDMI_30BIT_ENABLE,

  /*!
      set  36 bit enable in edid
   */
  HDMI_36BIT_ENABLE,

 /*!
      set  48 bit enable in edid
   */
  HDMI_48BIT_ENABLE,

  /*!
      set  HDR10  enable in edid
      Enums = 110
   */
  HDMI_HDR10_ENABLE,

  /*!
      set HLG enable in edid
   */
  HDMI_HLG_ENABLE,

   /*!
      disable hdmi isr
   */
  HDMI_ISR_DISABLE,

  /*!
      encryption disable
   */
  HDMI_ENCRYPTION_DISABLE,

  /*!
      set the normal strength of analog signal
   */
  HDMI_ANALOG_NORMAL_STRENGTH,

  /*!
      set the high strength of analog signal
   */
  HDMI_ANALOG_HIGH_STRENGTH,

  /*!
      disable audio mute operation
   */
  HDMI_DISABLE_AUDIO_MUTE,

  /*!
      disable hdmi audio tx operation
   */
  HDMI_DISABLE_AUDIO_TX,

  /*!
      do  cec msg receive
   */
  HDMI_CEC_RCV_MSG_START,

  /*!
      enable ddc error check process
   */
  HDMI_DDC_ERR_PROCESS,

  /*!
      ddc reset
      reset the ddc/i2c of hdmi , used for the ddc err Exception:such as Short Circuit the SDA/SCL of i2c line.
      no param
      Enums = 120
   */
  HDMI_DDC_RESET,

  /*!
     make the hdcp re-auth
     used for auth failed in the case of the ddc err
     param 1 : is reauth
   */
  HDMI_HDCP_REAUTH,

  /*!
       for fpga cmd
     */
  HDMI_FORCE_HOTPLUG,

  /*!
      for fpga cmd
     */
  HDMI_FORCE_CEC_RVC,

  /*!
      for frontporch param is turning on or not
     */
  HDMI_FP_ON,

  /*!
      for hdmi soft reset, it will rise a hotplug isr
     */
  HDMI_SOFT_RESET,


  /*!
      flowing new hdmi ioctrl cmd used for new sym4 chip
      Enums = 100
   */
  HDMI_NEW_CMD_START = 200,

  /*!
        for sym4 a1 eco test, set blank line
       */
  HDMI_BLANK_LINE_SET,

  /*!
        for sym4 a1 eco test, set blank line ctrl:  4 mode ,  ref BLACK_CTRL_MODE_E

       */
  HDMI_BLANK_LINE_CTRL,

 /*!
      for sym4 a1 eco test, set phy mute auto enable
     */
  HDMI_PHY_MUTE_AUTO_EN,

 /*!
    for sym4 a1 eco test, set prbs ctrl enable or disable
    it can be combination with BLACK_CTRL_MODE_E
   */
  HDMI_PRBS_CTRL_EN,

  /*!
      cmd end
   */
  HDMI_IO_CMD_MAX,
}hdmi_io_cmd_t;



/*!
  HDMI timing id defined in hdmi spec , don't change the enum value, it used for phy reg value
  */
typedef enum
{
    /*!
           HDMI  timing 640x480p@60     0
       */
    HDMI_TIMING_ID_640x480p_60Hz = 0,

    /*!
           HDMI  timing 1280x720p@60    1
       */
    HDMI_TIMING_ID_1280x720p_60Hz,

    /*!
           HDMI  timing 1920x1080i@60   2
       */
    HDMI_TIMING_ID_1920x1080i_60Hz,

    /*!
           HDMI  timing 720x480p@60     3
       */
    HDMI_TIMING_ID_720x480p_60Hz,

    /*!
           HDMI  timing 720x480i@60      4
       */
    HDMI_TIMING_ID_720x480i_60Hz,

    /*!
           HDMI  timing 1920x1080p@60  5
       */
    HDMI_TIMING_ID_1920x1080p_60Hz,

    /*!
           HDMI  timing 1920x1080p@50  6
       */
    HDMI_TIMING_ID_1920x1080p_50Hz,

    /*!
           HDMI  timing 1280x720p@50    7
       */
    HDMI_TIMING_ID_1280x720p_50Hz,

    /*!
           HDMI  timing 1920x1080i@50   8
       */
    HDMI_TIMING_ID_1920x1080i_50Hz,

    /*!
           HDMI  timing 720x576p@50     9
       */
    HDMI_TIMING_ID_720x576p_50Hz,

    /*!
           HDMI  timing 720x576i@50      10
       */
    HDMI_TIMING_ID_720x576i_50Hz,

    /*!
           HDMI  timing 1920x1080p@24  11
       */
    HDMI_TIMING_ID_1920x1080p_24Hz,

    /*!
           HDMI  timing 1920x1080p@25  12
       */
    HDMI_TIMING_ID_1920x1080p_25Hz,

    /*!
           HDMI  timing  1920x1080p@30  13
       */
    HDMI_TIMING_ID_1920x1080p_30Hz,


    /*!
           HDMI  timing  max id
       */
    HDMI_TIMING_MAX_ID,
}HDMI_TIMING_ID;




/*!
  HDMI audio format max number
  */
#define HDMI_AUDIO_FORMAT_MAX_NUM 10
typedef enum D3_STRUCT_E {
	D3_INFO_IDLE = 0x0,
	D3_INFO_FRAME_PACKING = 0x1,
	D3_INFO_TOP_AND_BOTTOM = 0x2,
	D3_INFO_SIDE_BY_SIDE_HALF = 0x4
} D3_STRUCT_T;

/*!
  HDMI edid information
  */
typedef struct {
	/*!
	the flag to indicate is the device is HDMI supported
	*/
	mt_u32 is_hdmi : 1;
	/*!
	the flag to indicate is the device support YCbCr444
	*/
	mt_u32 ycbcr444_supported : 1;
	/*!
	the flag to indicate is the device support YCbCr422
	*/
	mt_u32 ycbcr422_supported : 1;
	/*!
	the flag to indicate is the device support YCbCr420
	*/
	mt_u32 ycbcr420_supported : 1;
	/*!
	the flag to indicate is the device support 3840x2160p 24Hz
	*/
	mt_u32 supported_3840x2160p_24Hz : 1;
	/*!
	the flag to indicate is the device support 3840x2160p 25Hz
	*/
	mt_u32 supported_3840x2160p_25Hz : 1;
	/*!
	the flag to indicate is the device support 3840x2160p 30Hz
	*/
	mt_u32 supported_3840x2160p_30Hz : 1;
	/*!
	the flag to indicate is the device support 3840x2160p 50Hz
	*/
	mt_u32 supported_3840x2160p_50Hz : 2;
	/*!
	the flag to indicate is the device support 3840x2160p 60Hz
	*/
	mt_u32 supported_3840x2160p_60Hz : 2;
	/*!
	the flag to indicate is the device support 4096x2160p 24Hz
	*/
	mt_u32 supported_4096x2160p_24Hz : 1;
	/*!
	the flag to indicate is the device support 4096x2160p 25Hz
	*/
	mt_u32 supported_4096x2160p_25Hz : 1;
	/*!
	the flag to indicate is the device support 4096x2160p 30Hz
	*/
	mt_u32 supported_4096x2160p_30Hz : 1;
	/*!
	the flag to indicate is the device support 4096x2160p 50Hz
	*/
	mt_u32 supported_4096x2160p_50Hz : 2;
	/*!
	the flag to indicate is the device support 4096x2160p 60Hz
	*/
	mt_u32 supported_4096x2160p_60Hz : 2;
	/*!
	the flag to indicate is the device support 1080p 60Hz
	*/
	mt_u32 supported_1080p_60Hz : 1;
	/*!
	the flag to indicate is the device support 1080p 50Hz
	*/
	mt_u32 supported_1080p_50Hz : 1;
	/*!
	the flag to indicate is the device support 1080i 60Hz
	*/
	mt_u32 supported_1080i_60Hz : 1;
	/*!
	the flag to indicate is the device support 1080i 50Hz
	*/
	mt_u32 supported_1080i_50Hz : 1;
	/*!
	the flag to indicate is the device support 720p 60Hz
	*/
	mt_u32 supported_720p_60Hz : 1;
	/*!
	the flag to indicate is the device support 720p 50Hz
	*/
	mt_u32 supported_720p_50Hz : 1;
	/*!
	the flag to indicate is the device support 576p 50Hz
	*/
	mt_u32 supported_576p_50Hz : 1;
	/*!
	the flag to indicate is the device support 576i 50Hz
	*/
	mt_u32 supported_576i_50Hz : 1;
	/*!
	the flag to indicate is the device support 640X480p 60Hz
	*/
	mt_u32 supported_640x480p_60Hz : 1;
	/*!
	the flag to indicate is the device support 720X480p 60Hz
	*/
	mt_u32 supported_720x480p_60Hz : 1;
	/*!
	the flag to indicate is the device support 720X480i 60Hz
	*/
	mt_u32 supported_720x480i_60Hz : 1;
	/*!
	the flag to indicate is the device support deep color rgb 30bit
	*/
	mt_u32 rgb30bit : 1;
	/*!
	the flag to indicate is the device support deep color rgb 36bit
	*/
	mt_u32 rgb36bit : 1;
	/*!
	the flag to indicate is the device support deep color rgb 48bit
	*/
	mt_u32 rgb48bit : 1;
	/*!
	the flag to indicate is the device support deep color y444
	*/
	mt_u32 dc_y444 : 1;
	/*!
	the flag to indicate is the device support deep color yuv420 30bit
	*/
	mt_u32 y420_30bit : 1;
	/*!
	the flag to indicate is the device support deep color yuv420 36bit
	*/
	mt_u32 y420_36bit : 1;
	/*!
	the flag to indicate is the device support deep color yuv420 48bit
	*/
	mt_u32 y420_48bit : 1;
	/*!
	the flag to indicate is the device support xvycc601
	*/
	mt_u32 supported_xvycc601 : 1;
	/*!
	the flag to indicate is the device support xvycc709
	*/
	mt_u32 supported_xvycc709 : 1;
	/*!
	the flag to indicate is the device support bt2020cycc
	*/
	mt_u32 supported_bt2020cycc : 1;
	/*!
	the flag to indicate is the device support bt2020ycc
	*/
	mt_u32 supported_bt2020ycc : 1;
	/*!
	the flag to indicate is the device support bt2020rgb
	*/
	mt_u32 supported_bt2020rgb : 1;
	/*!
	the flag to indicate is the device support hdr10
	*/
	mt_u32 supported_hdr10: 1;
	/*!
	the flag to indicate is the device support hlg
	*/
	mt_u32 supported_hlg: 1;
	/*!
	the flag to indicate is the device support hlg
	*/
	mt_u32 supported_hdr10p_vsif: 1;
	/*!
	the flag to indicate is the device support vivid
	*/
	mt_u32 supported_hdr10p_vivid: 1;
	/*!
	the flag to indicate is the device support hlg
	*/
	mt_u32 supported_4k2k_30 : 1;
	/*!
	the flag to indicate is the device support 4k2k 25
	*/
	mt_u32 supported_4k2k_25 : 1;
	/*!
	the flag to indicate is the device support 4k2k 24
	*/
	mt_u32 supported_4k2k_24 : 1;
	/*!
	the flag to indicate is the device support 4k2k smpte 24
	*/
	mt_u32 supported_4k2k_smpte_24 : 1;
	/*!
	the flag to indicate is the device 480i support 3d structure
	*/
	mt_u32 supported_3d_struc_480i_60Hz : 3;
	/*!
	the flag to indicate is the device 480p support 3d structure
	*/
	mt_u32 supported_3d_struc_480p_60Hz : 3;
	/*!
	the flag to indicate is the device 576i support 3d structure
	*/
	mt_u32 supported_3d_struc_576i_50Hz : 3;
	/*!
	the flag to indicate is the device 576p support 3d structure
	*/
	mt_u32 supported_3d_struc_576p_50Hz : 3;
	/*!
	the flag to indicate is the device 720p50 support 3d structure
	*/
	mt_u32 supported_3d_struc_720p_50Hz : 3;
	/*!
	the flag to indicate is the device 720p60 support 3d structure
	*/
	mt_u32 supported_3d_struc_720p_60Hz : 3;
	/*!
	the flag to indicate is the device 1080i50 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080i_50Hz : 3;
	/*!
	the flag to indicate is the device 1080i60 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080i_60Hz : 3;
	/*!
	the flag to indicate is the device 1080p24 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080p_24Hz : 3;
	/*!
	the flag to indicate is the device 1080p25 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080p_25Hz : 3;
	/*!
	the flag to indicate is the device 1080p30 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080p_30Hz : 3;
	/*!
	the flag to indicate is the device 1080p50 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080p_50Hz : 3;
	/*!
	the flag to indicate is the device 1080p60 support 3d structure
	*/
	mt_u32 supported_3d_struc_1080p_60Hz : 3;
	/*!
	the flag to indicate is the device 3840x2160@24 support 3d structure
	*/
	mt_u32 supported_3d_struc_3840x2160p_24Hz : 3;
	/*!
	the flag to indicate is the device 3840x2160@25 support 3d structure
	*/
	mt_u32 supported_3d_struc_3840x2160p_25Hz : 3;
	/*!
	the flag to indicate is the device 3840x2160@30 support 3d structure
	*/
	mt_u32 supported_3d_struc_3840x2160p_30Hz : 3;
	/*!
	the flag to indicate is the device 3840x2160@50 support 3d structure
	*/
	mt_u32 supported_3d_struc_3840x2160p_50Hz : 3;
	/*!
	the flag to indicate is the device 3840x2160@60 support 3d structure
	*/
	mt_u32 supported_3d_struc_3840x2160p_60Hz : 3;
	/*!
	the flag to indicate is the device 4096x2160@24 support 3d structure
	*/
	mt_u32 supported_3d_struc_4096x2160p_24Hz : 3;
	/*!
	the flag to indicate is the device 4096x2160@25 support 3d structure
	*/
	mt_u32 supported_3d_struc_4096x2160p_25Hz : 3;
	/*!
	the flag to indicate is the device 4096x2160@30 support 3d structure
	*/
	mt_u32 supported_3d_struc_4096x2160p_30Hz : 3;
	/*!
	the flag to indicate is the device 4096x2160@50 support 3d structure
	*/
	mt_u32 supported_3d_struc_4096x2160p_50Hz : 3;
	/*!
	the flag to indicate is the device 4096x2160@60 support 3d structure
	*/
	mt_u32 supported_3d_struc_4096x2160p_60Hz : 3;
	/*!
	device id info
	*/
	mt_u8 tv_id_info[10];
	/*!
	supported audio format count
	*/
	mt_u8 audioformat_cnt;
	/*!
	supported audio format
	*/
	mt_u8 audioformat[HDMI_AUDIO_FORMAT_MAX_NUM];
	/*!
	supported audio channel
	*/
	mt_u8 audiochannel[HDMI_AUDIO_FORMAT_MAX_NUM];
	/*!
	supported audio sample rate
	*/
	mt_u8 audiofs[HDMI_AUDIO_FORMAT_MAX_NUM];
	/*!
	supported audio sample size
	*/
	mt_u8 audiolength[HDMI_AUDIO_FORMAT_MAX_NUM];
	/*!
	supported speaker placement
	*/
	mt_u8 speakerformat;
	/*!
	edid paser error code
	*/
	//hdmi_edid_err_code_t edid_errcode;
	/*!
	cec physical address
	*/
	mt_u16 cec_phy_addr;
	/*!
	  Manufacturer Info
	  */
	mt_u8 Manufacturer[3];
	/*!
	ProductCode
	*/
	mt_u16 ProductCode;
	/*!
	SerialID
	*/
	mt_u32 SerialID;
	/*!
	week of product
	*/
	mt_u32 ProductWeek;
	/*!
	data of product by year
	*/
	mt_u32 ProductYear;
	/*!
	indicate the edid data parse ready
	*/
	MT_BOOL edid_parse_ready;
	/*!
	supported HDR10P EMP
	*/
	mt_u8 hdr10p_emp;
	/*!
	sink supported max clock
	*/
	mt_u32 maxclk;
	/*!
	sink supported max lum
	*/
	mt_u32 maxlum;
	/*!
	sink supported max lum
	*/
	mt_u32 maxlum_vivid;
	/*!
	sink supported eotf
	*/
	mt_u8 hdr_et;
	/*!
	sink supported DD48
	*/
	mt_u8 DdMat48K;
} mt_unf_hdmi_edid_t;

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API declaration *****************************/
/** \addtogroup      HDMI */
/** @{ */ /** <!-- [HDMI] */

/**
\brief the whole initialization of the hdmi. CNcomment:HDMI接口驱动软件初始化 CNend
\attention  this func should be called before vo_init and after disp_init. CNcomment:必须在DISP驱动Setup之后和VO驱动Setup之前打开 CNend
\param CNcomment:无 CNend
\retval MT_SUCCESS  success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Init(mt_void);

/**
\brief deinit the hdmi. CNcomment:HDMI接口去初始化 CNend
\attention  this must be called after vo exited and before disp  exited . CNcomment:必须在VO驱动Exit之后和DISP驱动Exit之前打开 CNend
\param CNcomment:无 CNend
\retval MT_SUCCESS      success.CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_DeInit(mt_void);

/**
\brief create hdmi device. CNcomment:创建HDMI设备 CNend
\attention \n
\param[in] enHdmi  hdmi device id. CNcomment:要打开的HDMI设备 CNend
\param[in] pstOpenPara  When get GetSinkCapability failed,defalut set sink device to DVI/HDMI mode.CNcomment:未获取到对端能力集时，默认把对端当成(DVI/HDMI)设备 CNend
\retval MT_SUCCESS     success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_OPEN_PARA_S *pstOpenPara);

/**
\brief close the handler created by  MT_UNF_HDMI_Open. CNcomment:销毁由MT_UNF_HDMI_Open创建的句柄 CNend
\attention \n
\param[in] enHdmi  hdmi device id. CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS     success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi);

/**
\brief get current hdmi status. CNcomment:获取hdmi当前的状态 CNend
\attention \n
\param[in] enHdmi  hdmi device id. CNcomment:HDMI设备ID CNend
\param[out] pHdmiStatus hdmi cuttent status. CNcomment:HDMI当前状态 CNend
\retval MT_SUCCESS     success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus);

/**
\brief to get the capability of sink connect to stbbox. CNcomment:查询获取HDMI Sink设备的能力集 CNend
\attention \n
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[out] pCapability  the capability of the sink .CNcomment:SINK能力集 CNend
\retval MT_SUCCESS       success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi. CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_EDID_BASE_INFO_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pCapability);

/**
\brief set the attr of given hdmi interface. CNcomment:设置HDMI接口属性。 CNend
\attention \n
\param[in] enHdmi      hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] pstAttr     the attr of given hdmi interface.CNcomment:HDMI接口属性 CNend
\retval MT_SUCCESS     success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_ATTR_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr);

/**
\brief get the current attr of the give hdmi interface. CNcomment:查询HDMI接口当前属性 CNend
\attention \n
\param[in] enHdmi hdmi device id.CNcomment: HDMI设备ID CNend
\param[out] pstAttr     the attr of given hdmi interface. CNcomment:HDMI接口属性 CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see MT_UNF_HDMI_ATTR_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_ATTR_S *pstAttr);

/**
\brief get the cec working status. CNcomment:获取CEC状态 CNend
\attention  user can get the cec working status through this func. CNcomment:客户可以通过该接口获取CEC工作状态\n CNend
\param[in] enHdmi       hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] pStatus     get cec working status. CNcomment:CEC 状态 CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_CEC_CMD_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S *pStatus);

/**
\brief send the cec data. CNcomment:发送CEC 命令 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] pCECCmd      the cec cmd data.CNcomment:Cec Command 内容 CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_CEC_CMD_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd);

/**
\brief get the cec data received. CNcomment:接收的CEC 命令 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] pCECCmd    the cec cmd data. CNcomment:Cec Command 内容 CNend
\param[in] timeout    timeout for getting cec cmd, unit: 10ms;
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_CEC_CMD_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S *pCECCmd, mt_u32 timeout);

/**
\brief enable the cec func. CNcomment:CEC 使能打开 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/

/**
\brief register CEC callback function. CNcomment:注册获取CEC回调函数 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] pCECCallback    the cec callback handle. CNcomment:回调函数句柄 CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_CEC_CMD_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_RegCECCallBackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CECCALLBACK pCECCallback);

/**
\brief unregister CEC callback function. CNcomment:反注册获取CEC回调函数 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] pCECCallback  the cec callback handle. CNcomment:回调函数句柄 CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see ::MT_UNF_HDMI_CEC_CMD_S\n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_UnRegCECCallBackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CECCALLBACK pCECCallback);

/**
 \brief enable the cec func. CNcomment:CEC 使能 CNend
 \attention \n
 \param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
 \retval MT_SUCCESS      success. CNcomment:成功 CNend
 \retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
 \see \n
 CNcomment:无 CNend

 */
mt_s32 MT_UNF_HDMI_CEC_Enable(MT_UNF_HDMI_ID_E enHdmi);

/**
\brief disable the cec func. CNcomment:CEC 使能关闭 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_CEC_Disable(MT_UNF_HDMI_ID_E enHdmi);


/**
 \brief enable the hdcp func. CNcomment:CEC 使能 CNend
 \attention \n
 \param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
 \retval MT_SUCCESS      success. CNcomment:成功 CNend
 \retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
 \see \n
 CNcomment:无 CNend

 */
mt_s32 MT_UNF_HDMI_HDCP_Enable(MT_UNF_HDMI_ID_E enHdmi);

/**
\brief disable the hdcp func. CNcomment:CEC 使能关闭 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_HDCP_Disable(MT_UNF_HDMI_ID_E enHdmi);


/**
\brief  set and send infoframe. CNcomment:设置并发送InfoFrame CNend
\attention \n
\param[in] enHdmi       hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] pstInfoFrame the inforframe content.CNcomment:InfoFrame内容 CNend
\retval MT_SUCCESS      success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);

/**
\brief get the infoframe infor. CNcomment:获取正在发送的InfoFrame的信息 CNend
\attention \n
\param[in] enHdmi       hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] enInfoFrameType the info frame type such as avi or audio or gcp etc. CNcomment:InfoFrame类型 CNend
\param[out] pstInfoFrame   the inforframe content.CNcomment:InfoFrame内容 CNend
\retval MT_SUCCESS      success.CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);

/**
\brief  start the hdmi works. CNcomment:HDMI开始 CNend
\attention \n
this should be called after MT_UNF_HDMI_SetAttr.
CNcomment:启动HDMI接口。一般在MT_UNF_HDMI_SetAttr之后调用 CNend
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi);

/**
\brief stop  the hdmi. CNcomment:hdmi停止 CNend
\attention \n
this func should be called  when hdmi plug out.
CNcomment:当HDMI线被拔除后，HDMI回调函数应该调用该函数。 CNend
\param[in] enHdmi  hdmi device id. CNcomment:HDMI设备ID CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi);

/**
\brief set the deep color mode. CNcomment:设置HDMI DeepColor模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] enDeepColor deep color mode,please refer to the MT_UNF_HDMI_DEEP_COLOR_E definiton.CNcomment:DeepColor模式，请参考::MT_UNF_HDMI_DEEP_COLOR_E  CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor);

/**
\brief get the deep color mode. CNcomment:获取HDMI DeepColor模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] penDeepColor deep color mode,please refer to the MT_UNF_HDMI_DEEP_COLOR_E definiton.CNcomment:DeepColor模式，请参考::MT_UNF_HDMI_DEEP_COLOR_E  CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor);


/**
\brief set the hdmi color space mode. CNcomment:设置HDMI Color Space模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] enColorSpace  color space mode,please refer to the MT_UNF_HDMI_VIDEO_MODE_E definiton.CNcomment:DeepColor模式，请参考::MT_UNF_HDMI_VIDEO_MODE_E  CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E enColorSpace);


/**
\brief get the hdmi color space mode. CNcomment:获取HDMI Color Space 模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id. CNcomment:HDMI设备ID CNend
\param[in] penDeepColor  color space mode,please refer to the MT_UNF_HDMI_VIDEO_MODE_E definiton.CNcomment:color space 模式，请参考::MT_UNF_HDMI_VIDEO_MODE_E  CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/

mt_s32 MT_UNF_HDMI_GetColorSpace(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_VIDEO_MODE_E *penColorSpace);


/**
\brief switch the xvycc on or off. CNcomment:设置HDMI xvYCC 模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] bEnalbe   whether to enable xvycc mode or not .CNcomment:是否使能xvYCC模式 CNend
\retval MT_SUCCESS    success.  CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnalbe);

/**
\brief switch the mute of av  on or off. CNcomment:设置HDMI AV mute 模式 CNend
\attention \n
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] bAvMute   whether to mute the av.CNcomment:是否mute AV CNend
\retval MT_SUCCESS     success. CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute);

/**
\brief get the edid information forcelly. CNcomment:强制获取EDID原始数据,该接口为特定平台需要获取原始EDID的接口 CNend
\attention \n
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] *u8Edid  the buffer allocated externally, buffer size must be 512. CNcomment:获取原始EDID数据buffer。EDID版本不同，读出来的数据不同，取上限buffer 大小为512,用户需自己分配好; CNend
\param[in] *u32EdidLength  the data lenth of  original edid. CNcomment:获取原始EDID数据长度 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Force_GetEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8Edid, mt_u32 *u32EdidLength);

/**
\brief register callback function. CNcomment:注册回调函数 CNend
\attention \n
this func should be called before MT_UNF_HDMI_Open and after MT_UNF_HDMI_Init
because MT_UNF_HDMI_Open will trigger hotplug event.
And this function can only register one function
If call this func two times,then the 2nd callback function will cover 1st one.
CNcomment:建议在MT_UNF_HDMI_Init之后和MT_UNF_HDMI_Open之前调用 \n
因为Open时如果连着接收端设备上会触发一次hotplug消息 \n
该函数只能注册一个回调函数，第二次注册的回调函数会覆盖前面的回调函数 CNend
\param CNcomment:无 CNend
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] *MT_UNF_HDMI_CALLBACK_FUNC_S  callback function CNcomment:回调函数 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_RegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);

/**
\brief unregister CallbackFunc. CNcomment:注销回调函数 CNend
\attention \n
this func should be called before MT_UNF_HDMI_DeInit and after MT_UNF_HDMI_Close
CNcomment:建议在MT_UNF_HDMI_Close之后和MT_UNF_HDMI_DeInit之前调用 CNend
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] *MT_UNF_HDMI_CALLBACK_FUNC_S  callback function CNcomment:回调函数 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_UnRegCallbackFunc(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CALLBACK_FUNC_S *pstCallbackFunc);

/**
\brief Load HDCP key. CNcomment:导入hdcpkey CNend
\attention \n
\param[in] enHdmi   hdmi device id.CNcomment:HDMI设备ID CNend
\param[in] *pstLoadKey  key struct length and point CNcomment:key 的结构 长度和指针 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey);

mt_s32 MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL enable);
/**
\brief get the edid information forcelly. CNcomment:从EEPROM获取EDID数据 CNend
\attention \n
Difference with two interface for get EDID
MT_UNF_HDMI_Force_GetEDID : Reread EDID from Sink,and write it to EEPROM
MT_UNF_HDMI_ReadEDID : read edid from EEPROM,not Real-time read from Sink
CNcomment:两个获取EDID的接口差别
MT_UNF_HDMI_Force_GetEDID 是重新从接收端读取Edid，并写到EEPROM
MT_UNF_HDMI_ReadEDID 是从EEPROM读取Edid，而不是实时的接收端EDID CNend
\param[in] *pstLoadKey  key struct length and point CNcomment:key 的结构 长度和指针 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_ReadEDID(mt_u8 *u8Edid, mt_u32 *u32EdidLength);

/** @deprecate */
/**
\brief Get HDMI runtime delay. CNcomment:获取hdmi运行时延时 CNend
\attention \n
CNend
\param[in] *pstDelay  delay struct delay time and mode CNcomment:延时结构体 延时的时长和模式 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay);

/** @deprecate */
/**
\brief Set HDMI runtime delay. CNcomment:设置hdmi运行时延时 CNend
\attention \n
if not use this interface, then use hdmi inner delay
CNcomment:默认情况下调用该接口前会使用内部延时,调用后根据delay模式使用延时 CNend
\param[in] *pstDelay  delay struct delay time and mode CNcomment:延时结构体 延时的时长和模式 CNend
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pstDelay);


/**
\brief dump HDMI module register
\attention \n
hdmi driver need turn on the print function
CNcomment
\param[in]
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi);


/**
\brief write the HDMI module register
\attention \n
CNcomment
\param[in] addr  page0, 0x00---0xff, page1: 0x100 --- 0x1ff
\param[in] data  it's a u8 data
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data);

/**
\brief read the HDMI module register
\attention \n
CNcomment
\param[in] addr  page0, 0x00---0xff, page1: 0x100 --- 0x1ff
\param[in] pstData  it's a address off u8 data to save the result
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 *pstData);

/**
\brief set the HDMI osd name
\param[in] osdName set osd name
\retval MT_SUCCESS success
*/
mt_s32 MT_UNF_HDMI_Set_Osdname(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *osdName);

/**
\brief get the HDMI sink 's edid raw data
\attention \n
CNcomment
\param[in] enHdmi  hdmi device id
\param[in] u8RawEdid  it's a address off u8 to save the edid raw data, max 512 bytes
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_ReadRawEDID(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *u8RawEdid);


/**
\brief read the HDMI module register
\attention \n
CNcomment
\param[in] enHdmi  hdmi device id
\param[in] cmd
\param[in] data1
\param[in] data2
\retval MT_SUCCESS   success.   CNcomment:成功 CNend
\retval please refer to the err code definitino of mpi.CNcomment:请参考MPI错误码 CNend
\see \n
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_Docmd(MT_UNF_HDMI_ID_E enHdmi, hdmi_io_cmd_t cmd, mt_u32 data1, mt_u32 data2);
/**
\brief test api
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E Fmt);
/**
\brief test api
CNcomment:无 CNend
*/
mt_s32 MT_UNF_HDMI_PreSetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_ENC_FMT_E Fmt);

/** @} */ /** <!-- ==== API declaration end ==== */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MT_UNF_HDMI_H__ */
