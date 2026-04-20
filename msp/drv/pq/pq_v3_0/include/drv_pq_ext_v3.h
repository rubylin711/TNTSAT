/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_PQ_EXT_V3_H__
#define __DRV_PQ_EXT_V3_H__

#include "mt_type.h"
#include "mt_drv_video.h"
//#include "mt_reg_common.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


//typedef S_CAS_REGS_TYPE PQ_VPSS_CFG_REG_S;
//typedef S_VPSSWB_REGS_TYPE PQ_VPSS_WBC_REG_S ;


/* VPSS视频信息 */
typedef struct mtMT_VPSS_PQ_INFO_S
{
    MT_DRV_SOURCE_E     enInputSrc;         /* 信号源类型 */
    mt_u32              u32Width;           /* 图像宽度 */
    mt_u32              u32Height;          /* 图像高度 */
    mt_u32              u32FrameRate;       /* 输入帧率 */
    MT_BOOL             bInterlace;         /* 是否隔行信号 */
    MT_DRV_COLOR_SYS_E  enColorSys;         /* ATV信号的Color System */
} MT_VPSS_PQ_INFO_S;

/*VDP视频信息 */
typedef struct mtMT_VDP_PQ_INFO_S
{
    MT_DRV_SOURCE_E             enInputSrc;         /* 信号源类型 */
    mt_u32                      u32Width;           /* 图像宽度 */
    mt_u32                      u32Height;          /* 图像高度 */
    mt_u32                      u32FrameRate;       /* 输入帧率 */
    MT_BOOL                     bInterlace;         /* 是否隔行信号 */
    MT_BOOL                     b3dType;            /* 是否原生3D播放 */
    MT_BOOL                     bPcTiming;          /* 是否为PC Timing */
    MT_DRV_COLOR_SYS_E          enColorSys;         /* ATV信号的Color System */
    mt_u32                      u32OutWidth;        /* 输出图像宽度 */
    mt_u32                      u32OutHeight;       /* 输出图像高度 */
    MT_BOOL                     bSRState;           /* SR 开关 */
} MT_VDP_PQ_INFO_S;


/* PQ模块开关信息 */
typedef struct mtPQ_VPSS_MODULE_S
{
    MT_BOOL                     bFMD;         /* FMD开关 */
    MT_BOOL                     bTNR;         /* NR开关 */
    MT_BOOL                     bCCCL;        /* CCCL开关 */
} MT_PQ_VPSS_MODULE_S;

/* 色彩空间转换类型 */
typedef enum mtPQ_CSC_MODE_
{
    MT_PQ_CSC_YUV2RGB_601 = 0,          /* YCbCr_601 LIMIT-> RGB */
    MT_PQ_CSC_YUV2RGB_709,              /* YCbCr_709 LIMIT-> RGB */
    MT_PQ_CSC_RGB2YUV_601,              /* RGB->YCbCr_601 LIMIT */
    MT_PQ_CSC_RGB2YUV_709,              /* RGB->YCbCr_709 LIMIT */
    MT_PQ_CSC_YUV2YUV_709_601,          /* YCbCr_709 LIMIT->YCbCr_601 LIMIT */
    MT_PQ_CSC_YUV2YUV_601_709,          /* YCbCr_601 LIMIT->YCbCr_709 LIMIT */
    MT_PQ_CSC_YUV2YUV,                  /* YCbCr LIMIT->YCbCr LIMIT */
    MT_PQ_CSC_YUV2RGB_601_FULL,         /* YCbCr_601 FULL-> RGB */
    MT_PQ_CSC_YUV2RGB_709_FULL,         /* YCbCr_709 FULL-> RGB */
    MT_PQ_CSC_RGB2YUV_601_FULL,         /* RGB->YCbCr_601 FULL */
    MT_PQ_CSC_RGB2YUV_709_FULL,         /* RGB->YCbCr_709 FULL */
    MT_PQ_CSC_RGB2RGB,                  /* RGB->RGB */

    MT_PQ_CSC_BUTT
} MT_PQ_CSC_MODE_E;

/*VDP CSC ID*/
typedef enum mtPQ_CSC_ID_E
{
    MT_PQ_VDP_CSC_V0 = 0 ,
    MT_PQ_VDP_CSC_V1     ,
    MT_PQ_VDP_CSC_V2     ,
    MT_PQ_VDP_CSC_V3     ,
    MT_PQ_VDP_CSC_V4     ,
    //MT_PQ_VDP_CSC_VP0  ,
    //MT_PQ_VDP_CSC_VP1  ,

    MT_PQ_VDP_CSC_BUTT
} MT_PQ_CSC_ID_E;

/* VDP CSC 属性*/
typedef struct  mtPQ_VDP_CSC_S
{
    MT_BOOL  bCSCEn;
    MT_PQ_CSC_MODE_E  enCSC;
} MT_PQ_VDP_CSC_S;


/* 缩放YUV数据结构体444 422 420 */
typedef enum mtPQ_ZME_FORMAT_E
{
    MT_PQ_ALG_PIX_FORMAT_SP420 = 0,
    MT_PQ_ALG_PIX_FORMAT_SP422,
    MT_PQ_ALG_PIX_FORMAT_SP444,

    MT_PQ_ALG_PIX_FORMAT_BUTT,
} ZME_FORMAT_E;

/* SCALER 信息 */
typedef struct  mtPQ_SCALER_S
{
    MT_BOOL bHorizontal;
    MT_BOOL bYUV;
    MT_BOOL b3Dtype;
    mt_u32  u32YRatio;
    mt_u32  u32CRatio;
    ZME_FORMAT_E enInFmt;
    ZME_FORMAT_E enOutFmt;
} MT_PQ_SCALER_S;

/* DCI直方图结构 */
typedef struct mtPQ_HISTGRAM_S
{
    mt_s32 s32HistGram[32];
} MT_PQ_HISTGRAM_S;

/* CSC 矩阵系数结构 */
typedef struct  mtPQ_CSC_COEF_S
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
} MT_PQ_CSC_COEF_S;

typedef struct  mtPQ_CSC_DCCOEF_S
{
    mt_s32 csc_in_dc0;
    mt_s32 csc_in_dc1;
    mt_s32 csc_in_dc2;

    mt_s32 csc_out_dc0;
    mt_s32 csc_out_dc1;
    mt_s32 csc_out_dc2;
} MT_PQ_CSC_DCCOEF_S;

/* DCI统计窗口 */
typedef struct  mtPQ_DCI_WIN_S
{
    mt_u16      u16HStar;
    mt_u16      u16HEnd;
    mt_u16      u16VStar;
    mt_u16      u16VEnd;
} MT_PQ_DCI_WIN_S;

/* DEI 全局运动信息 */
typedef struct mtPQ_MOTION_INFO_S
{
    mt_u32 u32GlobalMotion;
    mt_u32 u32AdjustGain;
} MT_PQ_MOTION_INFO_S;

/*DEI API驱动传入的信息*/
typedef struct hiPQ_MOTION_INPUT_S
{
    mt_u32 u32Width;
    mt_u32 u32Height;
    mt_u32 u32HandleNo;
    //S_VPSSWB_REGS_TYPE* pstMotionReg;
} MT_PQ_MOTION_INPUT_S;


//tnr global motion api 输入参数结构
typedef struct
{
    mt_u32 u32HdlNo;                   /* 实例的ID */
    mt_u32 u32Width;                   /* 输入图像宽度 */
    mt_u32 u32Height;                  /* 输入图像高度 */
    //S_VPSSWB_REGS_TYPE* pstMotionReg;    /* 寄存器地址 */
} MT_PQ_TNR_MOTION_PARAM_IN_S;

/* Tnr global motion result */
typedef struct
{
    mt_u32 u32GlobalMotion;
    mt_u32 u32AdjustGain;
} MT_PQ_TNR_MOTION_RESULT_S;

/* DB强度值,对应VPSS_DB_WEIGHT寄存器 */
typedef struct
{
    mt_u8    u8HWeight;        // 6bits [0,63]
    mt_u8    u8VWeight;        // 6bits [0,63]
} MT_PQ_DB_WEIGHT_S;

/* image signal type of interlacing or non-interlacing */
typedef enum
{
    MT_PQ_DB_INTERLACE = 0,    /* 经过DEI ,隔行 */
    MT_PQ_DB_PROGRESSIVE       /* 不经过DEI ,逐行 */
} MT_PQ_DB_SIG_TYPE_E;


/* DB强度 API的输入参数结构 */
typedef struct
{
    mt_u8                u8SCDStr;  //根据SCD的api计算得出的一个值[0,255]
    mt_u32                u32HdlNo;    /* 实例的ID */
    MT_PQ_DB_SIG_TYPE_E   eType;       /* 数据源类型，隔行或逐行，是否做DEI */
    //S_VPSSWB_REGS_TYPE*  pstReg;     /* global motion 寄存器地址 */

} MT_PQ_DB_STR_PARAM_IN_S;


typedef struct
{
    mt_s32 IsProgressiveSeq;
    mt_s32 IsProgressiveFrm;
    mt_s32 RealFrmRate;

} PQ_VDEC_INFO_S;

/*IFMD软算法所需驱动传入参数，VPSS->PQ*/
typedef struct mt_PQ_IFMD_CALC_S
{
    mt_u32  u32HandleNo;
    mt_u32  u32WidthY;
    mt_u32  u32HeightY;
    mt_s32  s32FieldOrder;                 /* 顶底场序 顶场优先底场优先 */
    mt_s32  s32FieldMode;                  /* 顶底场标志 */
    mt_u32  u32FrameRate;
    MT_BOOL bPreInfo;                      /* DEI逻辑处理timeout，仍旧处理上一场 */
    PQ_VDEC_INFO_S stVdecInfo;
    /*S_VPSSWB_REGS_TYPE**/mt_void* pstIfmdHardOutReg; /* 读取ifmd的状态寄存器 */

} MT_PQ_IFMD_CALC_S;

typedef struct
{
    mt_s32  die_reset;
    mt_s32  die_stop;
    mt_s32  dir_mch;
    mt_s32  die_out_sel;
    mt_s32  jud_en;
    //mt_s32  ChromaVertFltEn;
    mt_s32  s32FieldOrder;    /* 顶地场序 */
    mt_s32  s32SceneChanged;  /* 场景切换信息 */
    mt_s32  s32FilmType;      /* 电影模式 */
    mt_u32  u32KeyValue;      /* 关键帧 */
    mt_u32  u32EdgeSmoothEn;

} MT_PQ_IFMD_PLAYBACK_S;


/* PFMD软算法所需驱动传入参数，VPSS->PQ */
typedef struct mt_PQ_PFMD_CALC_S
{
    mt_u32  u32HandleNo;
    mt_u32  u32WidthY;
    mt_u32  u32HeightY;
    mt_s32  s32FieldOrder; /* 顶底场序 顶场优先底场优先 */
    mt_s32  s32FieldMode;  /* 顶底场标志 */
    //S_VPSSWB_REGS_TYPE* pstpfmdHardOutReg; /* 读取pfmd的状态寄存器 */

} MT_PQ_PFMD_CALC_S;

typedef struct
{
    mt_s32  s32SceneChanged; /* 场景切换信息 */
    mt_s32  s32FilmType;     /* 电影模式 */
    mt_u32  u32KeyValue;     /* 关键帧 */
} MT_PQ_PFMD_PLAYBACK_S;

/*亮度/对比度/色调/饱和度设定*/
typedef struct hiPQ_PICTURE_SETTING_S
{
    mt_u16 u16Brightness;
    mt_u16 u16Contrast;
    mt_u16 u16Hue;
    mt_u16 u16Saturation;

    /*色温设定*/
    mt_s16 s16RedGain;
    mt_s16 s16GreenGain;
    mt_s16 s16BlueGain;
    mt_s16 s16RedOffset;
    mt_s16 s16GreenOffset;
    mt_s16 s16BlueOffset;
} MT_PQ_PICTURE_SETTING_S;


typedef struct
{
    mt_u32    u32ZmeFrmWIn;      /* zme  input frame width  */
    mt_u32    u32ZmeFrmHIn;      /* zme  input frame height */
    mt_u32    u32ZmeFrmWOut;     /* zme output frame width  */
    mt_u32    u32ZmeFrmHOut;     /* zme output frame height */

    mt_u8 u8ZmeYCFmtIn;       /*video format for zme input: 0-422; 1-420; 2-444*/
    mt_u8 u8ZmeYCFmtOut;      /*video format for zme Output: 0-422; 1-420; 2-444*/

    MT_BOOL   bZmeFrmFmtIn;      /* Frame format for zme  input: 0-field; 1-frame */
    MT_BOOL   bZmeFrmFmtOut;     /* Frame format for zme Output: 0-field; 1-frame */
    MT_BOOL   bZmeBFIn;          /* Input  field polar when input  is field format: 0-top field; 1-bottom field */
    MT_BOOL   bZmeBFOut;         /* Output field polar when Output is field format: 0-top field; 1-bottom field */

    mt_rect_s  stOriRect;
    mt_u32    u32InRate;         /* Vpss out Rate  RealRate*1000 */
    mt_u32    u32OutRate;        /* Disp Rate      RealRate*1000 */
    MT_BOOL   bDispProgressive;  /* 1:Progressive 0:Interlace */
    mt_u32    u32Fidelity;       /* rwzb info >0:is rwzb */
    /*
     1.OriRect
     2.InFrameRate
     3.OutRate
     4.Out I/P
     */
} MT_PQ_ZME_PARA_IN_S;

typedef struct
{
    MT_BOOL bZmeEnHL;    /* zme enable of horizontal luma: 0-off; 1-on */
    MT_BOOL bZmeEnHC;
    MT_BOOL bZmeEnVL;
    MT_BOOL bZmeEnVC;

    MT_BOOL bZmeMdHL;    /* zme mode of horizontal luma: 0-copy mode; 1-FIR filter mode */
    MT_BOOL bZmeMdHC;
    MT_BOOL bZmeMdVL;
    MT_BOOL bZmeMdVC;

    MT_BOOL bZmeMedHL;   /* zme Median filter enable of horizontal luma: 0-off; 1-on */
    MT_BOOL bZmeMedHC;
    MT_BOOL bZmeMedVL;
    MT_BOOL bZmeMedVC;

    mt_s32 s32ZmeOffsetHL;
    mt_s32 s32ZmeOffsetHC;
    mt_s32 s32ZmeOffsetVL;
    mt_s32 s32ZmeOffsetVC;
    mt_s32 s32ZmeOffsetVLBtm;
    mt_s32 s32ZmeOffsetVCBtm;

    mt_u32 u32ZmeWIn;
    mt_u32 u32ZmeHIn;
    mt_u32 u32ZmeWOut;
    mt_u32 u32ZmeHOut;

    mt_u32 u32ZmeRatioHL;
    mt_u32 u32ZmeRatioVL;
    mt_u32 u32ZmeRatioHC;
    mt_u32 u32ZmeRatioVC;

    MT_BOOL bZmeOrder;    /* zme order of hzme and vzme: 0-hzme first; 1-vzme first */
    MT_BOOL bZmeTapVC;    /* zme tap of vertical chroma: 0-4tap; 1-2tap */

    mt_u8 u8ZmeYCFmtIn;   /*video format for zme input: 0-422; 1-420; 2-444*/
    mt_u8 u8ZmeYCFmtOut;  /*video format for zme output: 0-422; 1-420; 2-444*/

    mt_u32 u32ZmeCoefAddrHL;
    mt_u32 u32ZmeCoefAddrHC;
    mt_u32 u32ZmeCoefAddrVL;
    mt_u32 u32ZmeCoefAddrVC;
} MT_PQ_ZME_RTL_S;

typedef struct
{
    MT_PQ_ZME_RTL_S stZmeRtlPara;
    mt_u32  u32PosXStart;
} MT_PQ_SPLIT_ZME_PARA_UNIT_S;

/*分块缩放的配置信息*/
typedef struct
{
    MT_PQ_SPLIT_ZME_PARA_UNIT_S stPQSplitZmePara[2]; /* 包含左右两个块的缩放配置信息*/
} MT_PQ_SPLIT_ZME_PARA_S;

typedef enum mtPQ_ZME_LAYER_E
{
    MT_PQ_DISP_V0_LAYER_ZME = 0,
    MT_PQ_DISP_V1_LAYER_ZME    ,
    MT_PQ_DISP_V2_LAYER_ZME    ,
    MT_PQ_DISP_V3_LAYER_ZME    ,
    MT_PQ_DISP_V4_LAYER_ZME    ,
    MT_PQ_DISP_WBC0_LAYER_ZME  ,
    MT_PQ_DISP_SR_LAYER_ZME    ,

    MT_PQ_DISP_LAYER_ZME_BUTT
} MT_PQ_ZME_LAYER_E;


/* 备注:Port的概念跟HSCL是不同的，VPSS的缩放应该区分两个概念，
   缩放的类型以及缩放的未知(PORT) */
typedef enum mtPQ_VPSS_ZME_LAYER_E
{
    MT_PQ_VPSS_PORT0_LAYER_ZME = 0 ,
    MT_PQ_VPSS_PORT1_LAYER_ZME     ,
    MT_PQ_VPSS_PORT2_LAYER_ZME     ,
    MT_PQ_VPSS_HSCL_LAYER_ZME      ,

    MT_PQ_VPSS_LAYER_ZME_BUTT
} MT_PQ_VPSS_ZME_LAYER_E;

#if 0
typedef enum mtPQ_VPSS_REG_PORT_E
{
    MT_PQ_VPSS_REG_HD = 0 ,
    MT_PQ_VPSS_REG_SD     ,
    MT_PQ_VPSS_REG_STR    ,

    MT_PQ_VPSS_REG_BUTT
} MT_PQ_VPSS_REG_PORT_E;
#endif

/* PORT缩放的窗口大小 */
typedef struct mtPQ_PORT_WIN_S
{
    mt_u32    u32Height;
    mt_u32    u32Width;
} MT_PQ_PORT_WIN_S;

/* ZME各Layer缩放窗口大小*/
typedef struct mtPQ_ZME_WIN_S
{
    MT_PQ_PORT_WIN_S    stPort0Win;
    MT_PQ_PORT_WIN_S    stPort1Win;
    MT_PQ_PORT_WIN_S    stPort2Win;
    MT_PQ_PORT_WIN_S    stHSCLWin;
} MT_PQ_ZME_WIN_S;



/* VPSS 驱动传入的信息 */
typedef struct mtPQ_WBC_INFO_S
{
    /* Common */
    mt_u32   u32HandleNo;
    mt_u32   u32Width;
    mt_u32   u32Height;
    MT_BOOL  bProgressive;              /* 隔逐行信息 */
    /*S_VPSSWB_REGS_TYPE**/mt_void* pstVPSSWbcReg;  /* ifmd 、Globlal Motion and DB的回写信息 */
    /* GlobalMotion */

    /* IFMD */
    mt_s32   s32FieldOrder;              /* 顶底场序 顶场优先底场优先 */
    mt_s32   s32FieldMode;               /* 顶底场标志 */
    mt_u32   u32FrameRate;               /* 帧率 */
    MT_BOOL  bPreInfo;                   /* DEI逻辑处理timeout，仍旧处理上一场 */
    PQ_VDEC_INFO_S stVdecInfo;           /* VDEC 传递的隔逐行信息 */

    /* DNR */

} MT_PQ_WBC_INFO_S;


/* PQ Calc 传给VPSS驱动的信息 */
typedef struct mtPQ_CFG_INFO_S
{
    /* GlobalMotion */

    /* IFMD */
    mt_s32  die_reset;
    mt_s32  die_stop;
    mt_s32  dir_mch;
    mt_s32  die_out_sel;
    mt_s32  jud_en;
    mt_s32  ChromaVertFltEn;
    mt_s32  s32FieldOrder;    /* 顶地场序 */
    mt_s32  s32SceneChanged;  /* 场景切换信息 */
    mt_s32  s32FilmType;      /* 电影模式 */
    mt_u32  u32KeyValue;      /* 关键帧 */
    /* DNR */

} MT_PQ_CFG_INFO_S;

/* VPSS WBC 传递给De-Blocking运算必须的信息 */
typedef struct mtPQ_DB_CALC_INFO_S
{

    mt_u32 u32HandleNo;
    mt_u32 u32Width;
    mt_u32 u32Height;
    /*S_VPSSWB_REGS_TYPE**/mt_void* pstDbCalcWbcReg;

} MT_PQ_DB_CALC_INFO_S;



typedef mt_s32 (*FN_PQ_GetPqParam)(mt_void);
typedef mt_s32 (*FN_PQ_UpdateVpssPQ)(mt_u32 u32HandleNo, MT_VPSS_PQ_INFO_S* pstTimingInfo, /*PQ_VPSS_CFG_REG_S**/mt_void* pstVPSSReg, /*PQ_VPSS_WBC_REG_S**/mt_void* pstWbcReg, MT_PQ_VPSS_MODULE_S* pstPQModule);
typedef mt_s32 (*FN_PQ_UpdateVdpPQ)(mt_u32 u32DisplayId, MT_VDP_PQ_INFO_S* pstTimingInfo, /*S_VDP_REGS_TYPE**/mt_void* pstVDPReg);
typedef mt_s32 (*FN_PQ_UpdateVdpCSC)(MT_PQ_CSC_ID_E enDisplayId, MT_PQ_VDP_CSC_S* pstCscMode);
typedef mt_s32 (*FN_PQ_UpdateDCIWin)(MT_PQ_DCI_WIN_S* pstWin, MT_BOOL bDciEnable);
typedef mt_s32 (*FN_PQ_GetVpssScalerCoef)(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);
typedef mt_s32 (*FN_PQ_GetScalerCoef)(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);
typedef mt_s32 (*FN_PQ_GetWbcScalerCoef)(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);
typedef mt_s32 (*FN_PQ_SetZme)(MT_PQ_ZME_LAYER_E u32LayerId, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL  bFirEnable);
typedef mt_s32 (*FN_PQ_SetVpssZme)(MT_PQ_VPSS_ZME_LAYER_E e32LayerId, /*S_CAS_REGS_TYPE**/mt_void* pstReg, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL  bFirEnable);
typedef mt_s32 (*FN_PQ_GetCSCCoef)(MT_PQ_CSC_MODE_E  enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef);
typedef mt_s32 (*FN_PQ_Get8BitCSCCoef)(MT_PQ_CSC_MODE_E  enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef);
typedef mt_s32 (*FN_PQ_DBCalcCfg)(MT_PQ_DB_CALC_INFO_S* pstDbCalcInfo);
typedef mt_s32 (*FN_PQ_GetWbcInfo)(MT_PQ_WBC_INFO_S* pstVpssWbcInfo);
typedef mt_s32 (*FN_PQ_SetAlgCalcCfg)(MT_PQ_WBC_INFO_S* pstVpssWbcInfo, MT_PQ_CFG_INFO_S* pstVpssCfgInfo);
typedef mt_s32 (*FN_PQ_GetTnrGlobalMotion)(MT_PQ_TNR_MOTION_PARAM_IN_S* pstParamIn, MT_PQ_TNR_MOTION_RESULT_S* pstMotionResult);
typedef mt_s32 (*FN_PQ_GetAdaptiveDBStrength)(MT_PQ_DB_STR_PARAM_IN_S* pstParamIn, MT_PQ_DB_WEIGHT_S* pstDBstr);
typedef mt_s32 (*FN_PQ_GetPfmdDetect)(MT_PQ_PFMD_CALC_S* pstPfmdCalc, MT_PQ_PFMD_PLAYBACK_S* pstPfmdResult);
typedef mt_s32 (*FN_PQ_IfmdDect)(MT_PQ_IFMD_CALC_S* pstIfmdCalc, MT_PQ_IFMD_PLAYBACK_S* pstIfmdResult);
typedef mt_s32 (*FN_PQ_GetDciHistgram)(MT_PQ_HISTGRAM_S* pstDciHist);
typedef mt_s32 (*FN_PQ_GetHDPictureSetting)(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);
typedef mt_s32 (*FN_PQ_GetSDPictureSetting)(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);
typedef mt_s32 (*FN_PQ_SetHDPictureSetting)(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);
typedef mt_s32 (*FN_PQ_SetSDPictureSetting)(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);


typedef struct tagPQ_EXPORT_FUNC_S
{
    FN_PQ_GetPqParam              pfnPQ_GetPqParam;
    FN_PQ_UpdateVpssPQ            pfnPQ_UpdateVpssPQ;
    FN_PQ_UpdateVdpPQ             pfnPQ_UpdateVdpPQ;
    FN_PQ_UpdateVdpCSC            pfnPQ_UpdateVdpCSC;
    FN_PQ_UpdateDCIWin            pfnPQ_UpdateDCIWin;
    FN_PQ_GetVpssScalerCoef       pfnPQ_GetVpssScalerCoef;
    FN_PQ_GetScalerCoef           pfnPQ_GetScalerCoef;
    FN_PQ_GetWbcScalerCoef        pfnPQ_GetWbcScalerCoef;
    FN_PQ_SetZme                  pfnPQ_SetZme;
    FN_PQ_SetVpssZme              pfnPQ_SetVpssZme;
    FN_PQ_GetCSCCoef              pfnPQ_GetCSCCoef;
    FN_PQ_Get8BitCSCCoef          pfnPQ_Get8BitCSCCoef;
    FN_PQ_DBCalcCfg               pfnPQ_DBCalcCfg;
    FN_PQ_GetWbcInfo              pfnPQ_GetWbcInfo;
    FN_PQ_SetAlgCalcCfg           pfnPQ_SetAlgCalcCfg;
    FN_PQ_GetTnrGlobalMotion      pfnPQ_GGetTnrGlobalMotion;
    FN_PQ_GetAdaptiveDBStrength   pfnPQ_GetAdaptiveDBStrength;
    FN_PQ_GetPfmdDetect           pfnPQ_PfmdDetect;
    FN_PQ_IfmdDect                pfnPQ_IfmdDect;
    FN_PQ_GetDciHistgram          pfnPQ_GetDciHistgram;
    FN_PQ_GetHDPictureSetting     pfnPQ_GetHDPictureSetting;
    FN_PQ_GetSDPictureSetting     pfnPQ_GetSDPictureSetting;
    FN_PQ_SetHDPictureSetting     pfnPQ_SetHDPictureSetting;
    FN_PQ_SetSDPictureSetting     pfnPQ_SetSDPictureSetting;

} PQ_EXPORT_FUNC_S;


/**
\brief Timming变化后进行VPSS PQ参数更新
\attention \n
无

\param[in] *u32HandleNo:VPSS通道号
\param[in] *pstTimingInfo: Timming Info
\param[in] *pstVPSSReg:VPSS 虚拟寄存器地址，如果为NULL，则表示销毁该路地址,
一个handleNo对应一个地址，若地址发生变化，才会重新赋初始值，否则不赋初始值。
\param[out] *pstPQModule:PQ传给驱动的开关参数

\retval ::MT_SUCCESS

*/

extern mt_s32 DRV_PQ_UpdateVpssPQ(mt_u32 u32HandleNo, MT_VPSS_PQ_INFO_S* pstTimingInfo, /*PQ_VPSS_CFG_REG_S**/mt_void* pstVPSSReg, /*PQ_VPSS_WBC_REG_S**/mt_void* pstWbcReg, MT_PQ_VPSS_MODULE_S* pstPQModule);

/**
 \brief Timming变化后进行VDP PQ参数更新
 \attention \n
无

 \param[in] u32DisplayId
 \param[in] pstTimingInfo: Timming Info
 \param[in] *pstVDPReg:VDP 虚拟寄存器地址

 \retval ::MT_SUCCESS

 */

extern mt_s32 DRV_PQ_UpdateVdpPQ(mt_u32 u32DisplayId, MT_VDP_PQ_INFO_S* pstTimingInfo, /*S_VDP_REGS_TYPE**/mt_void* pstVDPReg);

/**
 \brief 设置VDP各通道的CSC
 \attention \n
无

 \param[in] enDisplayId:
 \param[in] pstCscMode: 色彩空间

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_UpdateVdpCSC(MT_PQ_CSC_ID_E enDisplayId, MT_PQ_VDP_CSC_S* pstCscMode);

/**
 \brief 更新DCI直方图统计窗口
 \attention \n
无

 \param[in] *pstWin;

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_UpdateDCIWin(MT_PQ_DCI_WIN_S* pstWin, MT_BOOL bDciEnable);

/**
 \brief 设置VPSS scaler系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetVpssScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);
/**
 \brief 设置VDP scaler系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);
/**
 \brief 设置WBC scaler系数
 \attention \n
无

 \param[in] pstScalerInfo:缩放信息
 \param[out] pvCoefAddr:缩放系数地址

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetWbcScalerCoef(MT_PQ_SCALER_S* pstScalerInfo, mt_void* pvCoefAddrY, mt_void* pvCoefAddrC);

/**
 \brief 设置ZME
 \attention \n
无

 \param[in] u32LayerId:
 \param[in] pstZmeDrvPara:
 \param[in] bFirEnable

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetZme(MT_PQ_ZME_LAYER_E e32LayerId, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL bFirEnable);

/**
 \brief 设置ZME
 \attention \n
无

 \param[in] u32LayerId:
 \param[in] pstZmeDrvPara:
 \param[in] bFirEnable

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetVpssZme(MT_PQ_VPSS_ZME_LAYER_E e32LayerId, /*S_CAS_REGS_TYPE**/mt_void* pstReg, MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_BOOL bFirEnable);

/**
 \brief 获取CSC系数
 \attention \n
无

 \param[in] enCSCMode:
 \param[out] pstCSCCoef:
 \param[out] pstCSCDCCoef:

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetCSCCoef(MT_PQ_CSC_MODE_E enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef);

/**
 \brief 获取8bit CSC系数
 \attention \n
无

 \param[in] enCSCMode:
 \param[out] pstCSCCoef:
 \param[out] pstCSCDCCoef:

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_Get8BitCSCCoef(MT_PQ_CSC_MODE_E enCSCMode, MT_PQ_CSC_COEF_S* pstCSCCoef, MT_PQ_CSC_DCCOEF_S* pstCSCDCCoef);

#if 0
/**
 \brief 隔行从DEI获取的Global Motion信息
 \attention \n
无

 \param[in] *pstMotionHist:直方图
 \param[out] *pstMotionResult:运动信息

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetDeiGlobalMotion(MT_PQ_WBC_INFO_S* pstWbcInfo, MT_PQ_MOTION_INFO_S* pstMotionResult);
#endif


/**
 \brief 设置算法运算完之后的寄存器
 \attention \n
无

 \param[in]  *pstWbcInfo
 \param[out]
 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetAlgCalcCfg(MT_PQ_WBC_INFO_S* pstVpssWbcInfo, MT_PQ_CFG_INFO_S* pstVpssCfgInfo);

/**
 \brief 获取WbcInfo信息
 \attention \n
无

 \param[in]  MT_PQ_WBC_INFO_S* pstWbcInfo

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetWbcInfo(MT_PQ_WBC_INFO_S* pstVpssWbcInfo);

/**
\brief     :Tnr的Global Motion软算法API

\attention :
            本算法参考DEI模块进行移植
            算法提供：裴朝科 p00188942；移植&维护：吕明君 l00268071
            算法内已经将u32GlobalMotion和u32AdjustGain配到相应寄存器中

\param[in] :pstParamIn结构，包含如下元素
            u32HdlNo 实例的ID
            u32Width 输入图像宽度
            u32Height 输入图像高度
            pstMotionReg 寄存器地址

\param[out]:pstMotionResult结构，包含如下元素
            u32GlobalMotion
            u32AdjustGain

\retval    :MT_SUCCESS/MT_FAILURE

*/
extern mt_s32 DRV_PQ_GetTnrGlobalMotion(MT_PQ_TNR_MOTION_PARAM_IN_S* pstParamIn, MT_PQ_TNR_MOTION_RESULT_S* pstMotionResult);


/**
\brief     :根据Global Motion获取DB的自适应强度值API

\attention :函数内对多实例的ID u32HdlNo 不做活动性检测，仅判断阈值
            隔行或者逐行信号源所采用的Global Motion不同；
            算法内已经将输出参数u8HWeight和u8VWeight配到寄存器VPSS_DB_WEIGHT中
            算法提供：晏秀梅 y00224511；移植&维护：吕明君 l00268071

\param[in] :pstParamIn结构，包含如下元素
            u32HdlNo 实例的ID
            u8SCDStr 是根据SCD的api计算得出的一个值[0,255]
            pstReg   global motion 寄存器地址
            eType 数据源类型，隔行或逐行，是否做DEI

\param[out]:pstDBstr  DB强度值。对应寄存器VPSS_DB_WEIGHT，分别为u8HWeight和u8VWeight

\retval    :MT_SUCCESS/MT_FAILURE

*/
extern mt_s32 DRV_PQ_GetAdaptiveDBStrength(MT_PQ_DB_STR_PARAM_IN_S* pstParamIn, MT_PQ_DB_WEIGHT_S* pstDBstr);


/**
 \brief 逐行FMD模式检测
 \attention \n
无

 \param[in] pstVPSSReg: VPSS物理寄存器地址

 \retval ::MT_SUCCESS

 */

extern mt_s32 DRV_PQ_PfmdDetect(MT_PQ_PFMD_CALC_S* pstPfmdCalc, MT_PQ_PFMD_PLAYBACK_S* pstPfmdResult);

/**
 \brief 隔行FMD模式检测
 \attention \n
无

 \param[in] pstVPSSReg: VPSS物理寄存器地址

 \retval ::MT_SUCCESS

 */

//extern mt_s32 DRV_PQ_IFMDDetect(S_CAS_REGS_TYPE *pstVPSSReg);
extern mt_s32 DRV_PQ_IfmdDect(MT_PQ_IFMD_CALC_S* pstIfmdCalc, MT_PQ_IFMD_PLAYBACK_S* pstIfmdResult);

/**
 \brief 获取DCI直方图
 \attention \n
无

 \param[in] *pstDciHist:0-255

 \retval ::MT_SUCCESS

 */

extern mt_s32 DRV_PQ_GetDciHistgram(MT_PQ_HISTGRAM_S* pstDciHist);


/**
 \brief 获取高清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[out] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetHDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);

/**
 \brief 获取标清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[out] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_GetSDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);


/**
 \brief 设置高清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[in] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetHDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);

/**
 \brief 设置标清亮度/对比度/色调/饱和度/色温
 \attention \n
无

 \param[in] u32Hue   亮度/对比度/色调/饱和度/色温

 \retval ::MT_SUCCESS

 */
extern mt_s32 DRV_PQ_SetSDPictureSetting(MT_PQ_PICTURE_SETTING_S* pstPictureSetting);

/**
 \brief VPSS ZME二级缩放校验;
 \attention \n
无

 \param[in]
 u32InWitdh: 输入宽; u32InHeigh: 输入高; stZmeWin: 各层输出宽高;

 \param[out]
 pu32OutWitdh 输出宽; pu32OutHeigh 输出高;

 \retval :
MT_SUCCESS : 需要二级缩放
MT_FAILURE : 不需要二级缩放

 */
extern mt_s32 DRV_PQ_ZME_2L_Check(mt_u32 u32InWitdh, mt_u32 u32InHeigh, MT_PQ_ZME_WIN_S stZmeWin, mt_u32* pu32OutWitdh, mt_u32* pu32OutHeigh);


/**
 \brief 初始化PQ模块;
 \attention \n
无

 \param[in] pszPath: PQ Table文件路径, 如果pszPath参数为空指针, 会采用PQ SDK内部默认参数;

 \retval ::MT_SUCCESS

 */

extern mt_s32 MT_DRV_PQ_Init(mt_char* pszPath);

/**
 \brief 去初始化PQ模块;
 \attention \n
  无

 \param[in]

 \retval ::MT_SUCCESS

 */
extern mt_s32 MT_DRV_PQ_DeInit(mt_void);

extern mt_s32 PQ_DRV_ModInit(mt_void);
extern mt_void PQ_DRV_ModuleExit(mt_void);

extern mt_s32 DRV_PQ_SetSplitBlockZme(MT_PQ_ZME_PARA_IN_S* pstZmeDrvPara, MT_PQ_SPLIT_ZME_PARA_S* pstZmeSplitPara);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* End of #ifndef __DRV_PQ_EXT_H__ */
