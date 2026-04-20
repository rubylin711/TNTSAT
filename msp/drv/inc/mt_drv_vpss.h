/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_VPSS_H__
#define __MT_DRV_VPSS_H__

#include "mt_type.h"
#include "mt_drv_video.h"

/** Handle Definition.*/
/**<CNcomment: 句柄定义 \n
      句柄实际上是一个整数，用于标识chan和port时数值的定义有所不同，但都以-1作为无效句柄。\n
      1. VPSS实例句柄，其值为实例的索引号(ID) \n
      2. 端口句柄，其值要表示两个内容: 所属通道索引，以及端口本身的索引。\n
              = 实例索引*256 + 端口索引 */

typedef ulong VPSS_HANDLE;
#define VPSS_INVALID_HANDLE (-1)

#define PORTHANDLE_TO_VPSSID(hPort)    (hPort >> 8)
#define PORTHANDLE_TO_PORTID(hPort)    (hPort & 0xff)

/*************************** Macro definition ****************************/
#define DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER 40
#define DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_EXT_NUMBER 12
#define DEF_MT_DRV_VPSS_PORT_MAX_NUMBER 3

/** Last Frame Flag,indicate it is a valid frame.*/
/**<CNcomment: 前级传入的最后一帧标记，该帧有效*/
#define DEF_MT_DRV_VPSS_LAST_FRAME_FLAG 0xffee

/** Last Frame Flag,indicate it is an invalid frame.*/
/**<CNcomment: 前级传入的最后一帧标记，该帧无效*/
#define DEF_MT_DRV_VPSS_LAST_ERROR_FLAG 0xff00

/*************************** Structure Definition ****************************/

/**defines the vertical flip type.*/
/**CNcomment:定义是否打开垂直翻转功能*/
typedef enum
{
    MT_DRV_VPSS_HFLIP_DISABLE = 0,
    MT_DRV_VPSS_HFLIP_ENABLE,
    MT_DRV_VPSS_HFLIP_BUTT
}MT_DRV_VPSS_HFLIP_E;

/**defines the horizon flip type.*/
/**CNcomment:定义是否打开水平翻转功能*/
typedef enum
{
    MT_DRV_VPSS_VFLIP_DISABLE = 0,
    MT_DRV_VPSS_VFLIP_ENABLE,
    MT_DRV_VPSS_VFLIP_BUTT
}MT_DRV_VPSS_VFLIP_E;


/**defines the ROTATION type.*/
/**CNcomment:定义视频旋转角度*/
typedef enum
{
    MT_DRV_VPSS_ROTATION_DISABLE = 0,
    MT_DRV_VPSS_ROTATION_90,
    MT_DRV_VPSS_ROTATION_180,
    MT_DRV_VPSS_ROTATION_270,
    MT_DRV_VPSS_ROTATION_BUTT
}MT_DRV_VPSS_ROTATION_E;

/*Delete*/
typedef enum
{
    MT_DRV_VPSS_STEREO_DISABLE = 0,
    MT_DRV_VPSS_STEREO_SIDE_BY_SIDE,
    MT_DRV_VPSS_STEREO_TOP_AND_BOTTOM,
    MT_DRV_VPSS_STEREO_TIME_INTERLACED,
    MT_DRV_VPSS_STEREO_BUTT
}MT_DRV_VPSS_STEREO_E;


/**defines the Deinterlace mode.
    3FIELD，4FIELD，5FIELD is supported。
    Default is MT_DRV_VPSS_DIE_5FIELD*/
/**CNcomment:定义去隔行算法模式
    仅支持三、四、五场模式
    默认配置为五场模式*/
typedef enum
{
    MT_DRV_VPSS_DIE_DISABLE = 0,
    MT_DRV_VPSS_DIE_AUTO,
    MT_DRV_VPSS_DIE_2FIELD,
    MT_DRV_VPSS_DIE_3FIELD,
    MT_DRV_VPSS_DIE_4FIELD,
    MT_DRV_VPSS_DIE_5FIELD,
    MT_DRV_VPSS_DIE_6FIELD,
    MT_DRV_VPSS_DIE_7FIELD,
    MT_DRV_VPSS_DIE_BUTT
}MT_DRV_VPSS_DIE_MODE_E;

/*Delete*/
typedef enum
{
    MT_DRV_VPSS_ACC_DISABLE = 0,
    MT_DRV_VPSS_ACC_LOW,
    MT_DRV_VPSS_ACC_MIDDLE,
    MT_DRV_VPSS_ACC_HIGH,
    MT_DRV_VPSS_ACC_BUTT
}MT_DRV_VPSS_ACC_E;

/*Delete*/
typedef enum
{
    MT_DRV_VPSS_ACM_DISABLE = 0,
    MT_DRV_VPSS_ACM_BLUE,
    MT_DRV_VPSS_ACM_GREEN,
    MT_DRV_VPSS_ACM_BG,
    MT_DRV_VPSS_ACM_SKIN,
    MT_DRV_VPSS_ACM_BUTT
}MT_DRV_VPSS_ACM_E;

/*Delete*/
typedef enum
{
    MT_DRV_VPSS_CC_DISABLE = 0,
    MT_DRV_VPSS_CC_ENABLE,
    MT_DRV_VPSS_CC_AUTO,
    MT_DRV_VPSS_CC_BUTT
}MT_DRV_VPSS_CC_E;

/**defines the Sharpen type.*/
/**CNcomment:定义锐化模式*/
/*需要确认*/
typedef enum
{
    MT_DRV_VPSS_SHARPNESS_DISABLE = 0,
    MT_DRV_VPSS_SHARPNESS_ENABLE,
    MT_DRV_VPSS_SHARPNESS_AUTO,
    MT_DRV_VPSS_SHARPNESS_BUTT
}MT_DRV_VPSS_SHARPNESS_E;

/**defines the Dnr type.*/
/**CNcomment:定义去块去振铃算法模式*/
typedef enum
{
    MT_DRV_VPSS_DNR_DISABLE = 0,
    MT_DRV_VPSS_DNR_ENABLE,
    MT_DRV_VPSS_DNR_AUTO,
    MT_DRV_VPSS_DNR_BUTT
}MT_DRV_VPSS_DNR_E;

/*Delete ,will be MT_DRV_VPSS_DNR_E*/
typedef enum
{
    MT_DRV_VPSS_DB_DISABLE = 0,
    MT_DRV_VPSS_DB_ENABLE,
    MT_DRV_VPSS_DB_AUTO,
    MT_DRV_VPSS_DB_BUTT
}MT_DRV_VPSS_DB_E;
/*Delete ,will be MT_DRV_VPSS_DNR_E*/
typedef enum
{
    MT_DRV_VPSS_DR_DISABLE = 0,
    MT_DRV_VPSS_DR_ENABLE,
    MT_DRV_VPSS_DR_AUTO,
    MT_DRV_VPSS_DR_BUTT
}MT_DRV_VPSS_DR_E;
/*delete*/
/**defines the color space convert type.*/
/**CNcomment:定义色彩空间转换模式*/
typedef enum
{
    MT_DRV_VPSS_CSC_DISABLE = 0,
    MT_DRV_VPSS_CSC_ENABLE,
    MT_DRV_VPSS_CSC_AUTO,
    MT_DRV_VPSS_CSC_BUTT
}MT_DRV_VPSS_CSC_E;

/**defines the fidelity type.*/
/**CNcomment:定义保真处理模式*/
typedef enum
{
    MT_DRV_VPSS_FIDELITY_DISABLE = 0,
    MT_DRV_VPSS_FIDELITY_ENABLE,
    MT_DRV_VPSS_FIDELITY_AUTO,
    MT_DRV_VPSS_FIDELITY_BUTT
}MT_DRV_VPSS_FIDELITY_E;

typedef struct
{
    MT_DRV_VPSS_HFLIP_E  eHFlip;
    MT_DRV_VPSS_VFLIP_E  eVFlip;
    MT_DRV_VPSS_STEREO_E eStereo;
    MT_DRV_VPSS_ROTATION_E  eRotation;
    MT_DRV_VPSS_DIE_MODE_E eDEI;
    MT_DRV_VPSS_ACC_E eACC;
    MT_DRV_VPSS_ACM_E eACM;
    MT_DRV_VPSS_CC_E eCC;
    MT_DRV_VPSS_SHARPNESS_E eSharpness;
    MT_DRV_VPSS_DB_E eDB;
    MT_DRV_VPSS_DR_E eDR;

    MT_BOOL bIFMD;

    mt_rect_s stInRect;
    MT_BOOL   bUseCropRect;
    MT_DRV_CROP_RECT_S stCropRect;
}MT_DRV_VPSS_PROCESS_S;

typedef struct
{
    MT_DRV_VPSS_CSC_E eCSC;
    MT_DRV_VPSS_FIDELITY_E eFidelity;
}MT_DRV_VPSS_PORT_PROCESS_S;


/**defines the vpss instance attr.*/
/**CNcomment:定义逐隔行检测配置*/
typedef enum
{
    MT_DRV_VPSS_PRODETECT_PROGRESSIVE = 0,
    MT_DRV_VPSS_PRODETECT_INTERLACE,
    MT_DRV_VPSS_PRODETECT_AUTO,
    MT_DRV_VPSS_PRODETECT_BUTT
}MT_DRV_VPSS_PRODETECT_E;

/**defines the vpss IP attr.*/
/**CNcomment:定义VPSS绑定IP配置*/
typedef enum
{
    MT_DRV_VPSS_IPMODE_AUTO = 0,
    MT_DRV_VPSS_IPMODE_IP0,
    MT_DRV_VPSS_IPMODE_IP1,
    MT_DRV_VPSS_IPMODE_BUTT
}MT_DRV_VPSS_IPMODE_E;



/**defines the vpss instance attr.*/
/**CNcomment:定义实例配置*/
typedef struct
{
    mt_s32  s32Priority;  /**defines the instance priority.
                                default 16
                                0 is valid ,1 ~ 31 is more and more prior
                                */
                          /**CNcomment:0无效，1 ~ 31为正常优先级，数值越大优先级越高*/

    MT_BOOL bAlwaysFlushSrc;    /**whether process the newest frame.
                                        */
                                /**CNcomment:是否只处理最新一帧，在低延迟模式下置为TRUE
                                            这样每次都会读空Src buf，只留最新一帧处理 */

    MT_DRV_VPSS_PRODETECT_E enProgInfo;

    MT_BOOL bProgRevise;

    MT_DRV_COLOR_SPACE_E enSrcCS;

    MT_DRV_VPSS_PROCESS_S stProcCtrl;

}MT_DRV_VPSS_CFG_S;


/**defines the out buffer mode.*/
/**CNcomment:定义输出帧存管理模式*/
typedef enum hiDRV_VPSS_BUFFER_TYPE_E{
    MT_DRV_VPSS_BUF_VPSS_ALLOC_MANAGE = 0,
    MT_DRV_VPSS_BUF_USER_ALLOC_MANAGE,
    MT_DRV_VPSS_BUF_VDEC_ALLOC_MANAGE,
    MT_DRV_VPSS_BUF_USER_ALLOC_VPSS_MANAGE,
    MT_DRV_VPSS_BUF_TYPE_BUTT
}MT_DRV_VPSS_BUFFER_TYPE_E;

typedef struct hiDRV_VPSS_BUFFER_CFG_S
{
    MT_DRV_VPSS_BUFFER_TYPE_E eBufType;

    mt_u32 u32BufNumber;     /**bBufferNumber must be <= DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER */
    mt_u32 u32BufSize;       /** every buffer size in Byte */
    mt_u32 u32BufStride;     /**only for MT_DRV_VPSS_BUF_USER_ALLOC_VPSS_MANAGE */
    //mt_u32 u32BufPhyAddr[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_NUMBER]; /*128bit aligned */
    mt_u32 u32BufPhyAddr[DEF_MT_DRV_VPSS_PORT_BUFFER_MAX_EXT_NUMBER]; /*128bit aligned */
}MT_DRV_VPSS_BUFLIST_CFG_S;

/**defines the Port attr.*/
/**CNcomment:定义输出端口配置*/
typedef struct
{
    MT_DRV_COLOR_SPACE_E eDstCS;        /**Output color space*/

    MT_BOOL bOnlyKeyFrame;      /* 配合硬件FRC工作，电影模式源需要输出非重复帧，TV独有 */
    MT_BOOL bLBDCropEn;         /* 动态CROP已检测到的黑边，TV独有 */
    mt_rect_s stVideoRect;      /* TV LBX需求 */

    mt_rect_s stInRect;
    MT_BOOL   bUseCropRect;
    MT_DRV_CROP_RECT_S stCropRect;

    MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth;

    MT_DRV_VPSS_PORT_PROCESS_S stProcCtrl;

    /*display Info*/
    MT_DRV_ASPECT_RATIO_S stDispPixAR;
    MT_DRV_ASP_RAT_MODE_E eAspMode;
    MT_DRV_ASPECT_RATIO_S stCustmAR;

    mt_s32  s32OutputWidth;
    mt_s32  s32OutputHeight;

    MT_BOOL   bInterlaced;                /**display timing*/
    mt_rect_s stScreen;                   /**display screen resolution*/

    MT_DRV_PIX_FORMAT_E eFormat;            /**Output pix format*/
    MT_DRV_VPSS_BUFLIST_CFG_S stBufListCfg;
    mt_u32 u32MaxFrameRate;                 /* in 1/100 HZ  */

    MT_BOOL  bTunnelEnable;
    mt_s32  s32SafeThr;                 /**if Tunnel is  enabled, it is used to keep logic W/R order*/
                                        /**CNcomment:安全水线，0~100，为输出帧已完成的百分比.
                                                    0表示随时可给后级，100表示完全完成才能给后级 */
    MT_BOOL   b3Dsupport;

    MT_DRV_VPSS_ROTATION_E enRotation;
    MT_BOOL bVertFlip;
    MT_BOOL bHoriFlip;
}MT_DRV_VPSS_PORT_CFG_S;


typedef struct
{
    VPSS_HANDLE hPort;
    MT_BOOL bAvailable;
}MT_DRV_VPSS_PORT_AVAILABLE_S;

/**defines the user control command.*/
/**CNcomment:定义用户控制命令*/
typedef enum
{
    MT_DRV_VPSS_USER_COMMAND_IMAGEREADY = 0,
    MT_DRV_VPSS_USER_COMMAND_RESET,
    MT_DRV_VPSS_USER_COMMAND_CHECKALLDONE,
    MT_DRV_VPSS_USER_COMMAND_CHECKAVAILABLE,
    MT_DRV_VPSS_USER_COMMAND_START,
    MT_DRV_VPSS_USER_COMMAND_STOP,
    MT_DRV_VPSS_USER_COMMAND_CHANGEIP,
    MT_DRV_VPSS_USER_COMMAND_BUTT
}MT_DRV_VPSS_USER_COMMAND_E;

/**defines the out buffer state structure.*/
/**CNcomment:定义输出帧存队列状态*/
typedef struct
{
    mt_u32 u32TotalBufNumber;
    mt_u32 u32FulBufNumber;
}MT_DRV_VPSS_PORT_BUFLIST_STATE_S;


typedef enum
{
    MT_DRV_VPSS_BUFFUL_PAUSE = 0,
    MT_DRV_VPSS_BUFFUL_KEEPWORKING,
    MT_DRV_VPSS_BUFFUL_BUTT
}MT_DRV_VPSS_BUFFUL_STRATAGY_E;


/**defines the Pre-Module Mutual Mode.*/
/**CNcomment:定义与前级模块交互模式*/
typedef enum
{
    VPSS_SOURCE_MODE_USERACTIVE = 0,
    VPSS_SOURCE_MODE_VPSSACTIVE,
    VPSS_SOURCE_MODE_BUTT
}MT_DRV_VPSS_SOURCE_MODE_E;

typedef mt_s32 (*PFN_VPSS_SRC_FUNC)(VPSS_HANDLE hVPSS,MT_DRV_VIDEO_FRAME_S *pstImage);

typedef struct
{
    PFN_VPSS_SRC_FUNC VPSS_GET_SRCIMAGE;
    PFN_VPSS_SRC_FUNC VPSS_REL_SRCIMAGE;
}MT_DRV_VPSS_SOURCE_FUNC_S;

/**defines the user buffer structure.*/
/**CNcomment:定义VPSS向外部申请的BUFER结构*/
typedef struct
{
    VPSS_HANDLE hPort;
    ulong u32StartVirAddr;
    phys_addr_t u32StartPhyAddr;
    ulong u32Size;
    mt_u32 u32Stride;
    mt_u32 u32FrmH;
    mt_u32 u32FrmW;
}MT_DRV_VPSS_FRMBUF_S;

typedef struct
{
    VPSS_HANDLE hPort;
    MT_DRV_VIDEO_FRAME_S stFrame;
}MT_DRV_VPSS_FRMINFO_S;

typedef struct
{
    VPSS_HANDLE hPort;
    MT_BOOL bAvailable;
}MT_DRV_VPSS_BUFINFO_S;

/**defines the vpss process event.*/
/**CNcomment:定义VPSS处理事件*/
typedef enum
{
    VPSS_EVENT_BUFLIST_FULL,
    VPSS_EVENT_GET_FRMBUFFER,
    VPSS_EVENT_REL_FRMBUFFER,
    VPSS_EVENT_NEW_FRAME,
    VPSS_EVENT_CHECK_FRAMEBUFFER,
    VPSS_EVENT_BUTT,
}MT_DRV_VPSS_EVENT_E;

/**defines the vpss process event callback.*/
/**CNcomment:定义VPSS处理事件回调
    VPSS_EVENT_BUFLIST_FULL:输出帧存满，上报MT_DRV_VPSS_BUFFUL_STRATAGY_E结构体，用户返回处理策略
    VPSS_EVENT_GET_FRMBUFFER:获取输出帧存，上报MT_DRV_VPSS_FRMBUF_S结构体，用户赋值
    VPSS_EVENT_REL_FRMBUFFER:释放用户帧存，上报MT_DRV_VPSS_FRMBUF_S结构体，用户处理
    VPSS_EVENT_NEW_FRAME:新帧处理完成，上报MT_NULL，用户处理*/
typedef mt_s32 (*PFN_VPSS_CALLBACK)(mt_handle hDst, MT_DRV_VPSS_EVENT_E enEventID, mt_void *pstArgs);


/******************************* API declaration *****************************/

mt_s32 MT_DRV_VPSS_GlobalInit(mt_void);
mt_s32 MT_DRV_VPSS_GlobalDeInit(mt_void);

mt_s32  MT_DRV_VPSS_GetDefaultCfg(MT_DRV_VPSS_CFG_S *pstVpssCfg);

mt_s32  MT_DRV_VPSS_CreateVpss(MT_DRV_VPSS_CFG_S *pstVpssCfg,VPSS_HANDLE *phVPSS);
mt_s32  MT_DRV_VPSS_DestroyVpss(VPSS_HANDLE hVPSS);

mt_s32  MT_DRV_VPSS_SetVpssCfg(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg);
mt_s32  MT_DRV_VPSS_GetVpssCfg(VPSS_HANDLE hVPSS, MT_DRV_VPSS_CFG_S *pstVpssCfg);

mt_s32  MT_DRV_VPSS_GetDefaultPortCfg(MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);

mt_s32  MT_DRV_VPSS_CreatePort(VPSS_HANDLE hVPSS, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg, VPSS_HANDLE *phPort);
mt_s32  MT_DRV_VPSS_DestroyPort(VPSS_HANDLE hPort);

mt_s32  MT_DRV_VPSS_GetPortCfg(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);
mt_s32  MT_DRV_VPSS_SetPortCfg(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_CFG_S *pstVpssPortCfg);

mt_s32  MT_DRV_VPSS_EnablePort(VPSS_HANDLE hPort, MT_BOOL bEnable);

mt_s32  MT_DRV_VPSS_SendCommand(VPSS_HANDLE hVPSS, MT_DRV_VPSS_USER_COMMAND_E eCommand, mt_void *pArgs);

mt_s32  MT_DRV_VPSS_GetPortFrame(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame);
mt_s32  MT_DRV_VPSS_RelPortFrame(VPSS_HANDLE hPort, MT_DRV_VIDEO_FRAME_S *pstVpssFrame);

mt_s32  MT_DRV_VPSS_GetPortBufListState(VPSS_HANDLE hPort, MT_DRV_VPSS_PORT_BUFLIST_STATE_S *pstVpssBufListState);
MT_BOOL  MT_DRV_VPSS_CheckPortBufListFul(VPSS_HANDLE hPort);

mt_s32 MT_DRV_VPSS_SetSourceMode(VPSS_HANDLE hVPSS,
                          MT_DRV_VPSS_SOURCE_MODE_E eSrcMode,
                          MT_DRV_VPSS_SOURCE_FUNC_S* pstRegistSrcFunc);

mt_s32  MT_DRV_VPSS_PutImage(VPSS_HANDLE hVPSS, MT_DRV_VIDEO_FRAME_S *pstImage);
mt_s32  MT_DRV_VPSS_GetImage(VPSS_HANDLE hVPSS, MT_DRV_VIDEO_FRAME_S *pstImage);

mt_s32  MT_DRV_VPSS_RegistHook(VPSS_HANDLE hVPSS, mt_handle hDst, PFN_VPSS_CALLBACK pfVpssCallback);

#endif

