/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef MT_VIDEO_H
#define MT_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vo_fw.h"
#include "MT_DF_common.h"

#ifndef DISP_ON_AP_LINUX
#include "MT_DF_prj_adapt.h"
#endif

#define DISP_POOL_MAX_TRASH_NODES_NUM 32

#define DF_DISP_BUF_NODE_MAX_NUMBER  32
#define DF_DISP_BUF_DATA_SIZE        ((sizeof(MT_DF_VIDEO_FRAME_S)+sizeof(ulong)-1)/sizeof(ulong))
#define DF_DISP_BUF_USER_DATA_SIZE   32

#define MAX_RATE_CONVERSION_TABLE_LENGTH  6000//150

typedef enum
{
    MT_SCANTIMMING_I =0,
    MT_SCANTIMMING_P
}MT_SCAN_TIMMING;

/*!
  This structure defines the behavior of video display when video decoding is ready.
  */
typedef enum
{
  /*!
    The state of video layer is handled by user
    */
  DF_UNBLANK_USER,
  /*!
    The video layer will be displayed When AV is sync
    */
  DF_UNBLANK_SYNC,
  /*!
    The video layer will be displayed when video decoding is ready for display.
    */
  DF_UNBLANK_STABLE,
  /*!
    The video layer will be displayed when video get first I image.
  */
  DF_UNBLANK_FAST
}DF_START_MODE_E;

typedef struct
{
    /*!
    The coeff table physical address.
    */
    phys_addr_t coeff_phy_addr;
    /*!
    The coeff table kernel vitual address.
    */
    ulong coeff_kvir_addr;
    /*!
    The coeff table size.
    */
    MT_U32 coeff_size;
} VIDEO_SCALAR_COEFF_S;

typedef struct tagDF_DISP_BUF_NODE_S
{
    /* bit[31~16] resevered, zero; bit[15~8], buf id; bit[7~0], node id */
    volatile MT_U32 u32ID;

    ulong u32Data[DF_DISP_BUF_DATA_SIZE];	/* see: MT_DF_VIDEO_FRAME_S */
    ulong u32UserData[DF_DISP_BUF_USER_DATA_SIZE];
}DF_DISP_BUF_NODE_S;

typedef struct tagDF_DISP_NODE_ARRAY_S
{
  MT_U32 u32Nodes[DF_DISP_BUF_NODE_MAX_NUMBER];
  volatile MT_U32 u32ReaddPos;
  volatile MT_U32 u32WritePos;
}DF_DISP_NODE_ARRAY_S;

typedef struct tagDF_DISP_BUF_S
{
    MT_U32 u32BufID;
    MT_U32 u32Number;
    DF_DISP_BUF_NODE_S *pstBufArray[DF_DISP_BUF_NODE_MAX_NUMBER];

    DF_DISP_NODE_ARRAY_S DispEmptyArray;
    DF_DISP_NODE_ARRAY_S DispFullArray;
}DF_DISP_BUF_S;

typedef struct tagDispCfgDIenable
{
    DF_DISP_BUF_NODE_S *pstNodePre;
    MT_FIELDPOLARITY eFieldPolarityPre;//polarity flag is used in LastConfigNode configuration when repeat happened
    DF_DISP_BUF_NODE_S *pstNodeCur;
    MT_FIELDPOLARITY eFieldPolarityCur;
    DF_DISP_BUF_NODE_S *pstNodeNxt;
    MT_FIELDPOLARITY eFieldPolarityNxt;
}DISP_CFG_DI_ENABLE;

typedef struct
{
  DF_START_MODE_E start_mode;
  MT_U32 need_skip_frm_num;
  MT_U32 act_skip_frm_num; // output para
  MT_U8  repeat_one_frm;
  MT_U8  user_force_di_close_flag;
  MT_DF_BOOL force_show_ds_pic;
  MT_DF_VIDEO_SCALER_MODE_E vid_scaler_mode;
} DF_USER_CTRL_S;

typedef struct
{
  MT_U8 tag; // 1: set for drop in rate_conversion drop or set for repeat in rate_conversion_repeat  0: normal frame for display
  MT_U8 valid; // 1: not need to update current tag  0: need to update current tag
} DISP_FRC_TABLE;

typedef struct tagMTDispFRC
{
    MT_U32 u32InRate;
    MT_U32 u32OutRate;
    MT_U32 u32InRate_old;
    MT_U32 u32OutRate_old;
    MT_U32 u32InputCount;
    MT_U32 flag;   // 1 drop, 2 repeat
    DISP_FRC_TABLE RCTagBuf[MAX_RATE_CONVERSION_TABLE_LENGTH]; //DisplayFrameTagBuf
    MT_U32 RCTableLen; //RateConversionTableLen
    MT_U32 u32RepeatFlag; //0 : normal; 1: repeat
}MT_DF_FRC;

//for display control info
//set by drv for control the output video parameter
typedef struct tagDispControlInfo
{
    //Drv Control Setting
    MT_DF_STATUS eDisplayStatus;

    DF_USER_CTRL_S stUserCtrl;
    //Display Firmware use inside
    MT_DI_STATUS eDIStatus;

    MT_INT_POLARITY eInterruptPolarity;

    MT_DF_BOOL bBotIsrProc;

    //window
    stWindow_XY window_xy_HD;
    stWindow_XY window_xy_HD_new;
    stWindow_XY window_xy_SD;
    //crop
    stCrop_XY crop_xy;

    //AFD
    MT_DF_BOOL bAfdOpen;

    //FRC info&ctl struct
    MT_DF_FRC stFRCInfo;

    //AVSyncInfo

    //display mode
    DF_DRV_DISP_PPMODE_E eMode;

    //open/dark screen info
    MT_DF_BOOL b_layerEnable;

    DF_ASPECT_RATIO_E eOutAspectRatio;
    DF_ASPECT_MODE_E eOutAspectMode;

    // 3 scaler
    MT_DF_BOOL bUseSdScaler;
    MT_DF_BOOL bSdScalerFilterEnable;

    MT_DF_VIDEO_SCALER_MODE_E  video_scaler_mode;

    MT_DF_BOOL bUseDs;
    MT_DF_BOOL bUseSubVideo;
    MT_DF_BOOL bVideoLayerTop;

    MT_U8 u3ScalerStatus;  //for symphon1, 0:2 scaler; 1:3 scaler not ready; 2:3 scaler worked

    disp_sys_t eHdVidSys;
    disp_sys_t eSdVidSys;

    MT_DF_BOOL bHdOutEnable;
    MT_DF_BOOL bSdOutEnable;
    //HD/SD video out can only turn on/off when switch is enable
    MT_DF_BOOL bVideoOutSwitchEnable;

    MT_DF_BOOL bHdReInterlaceEnable;
    MT_DF_BOOL bSdReInterlaceEnable;

    MT_DF_BOOL ShowBlackCmd;

    MT_U8 ChipRev;

    MT_U8 DispExcptFlag;

    MT_DF_TRICK_MODE_E eTrickMode;
    MT_DF_PLAY_SPEED_E trick_sr;

    MT_DF_BOOL bLowDelayFlag;

    MT_DF_BOOL hd_close_di_flag;

    MT_U32 u32DispRptCount;

    stHdrInfoFromDrv hdr_info_from_drv;

    MT_U32 u32DisplayBrightness; // current display brightness

     //Bug 14188: check whether diplay repeat, not include rate convetion
    MT_DF_BOOL bDispRepeat;

    //avsync status
    MT_DF_BOOL bAvsyncFinished;

    //Bug 16389
    DF_EVENT_CALLBACK_FUNC event_cb;

    MT_DF_BOOL bEnablePDD;  //enable pull down detection
    MT_DF_BOOL bMovieMode;
}DF_CONTROL_INFO_S;

typedef struct tagDISP_POOL_S
{
    MT_DF_BOOL bValidStatus;

    MT_U32 u32BufNumber;
    MT_BOOL hd_close_di_flag; //true : close DI for HD videos

    DF_DISP_BUF_S stBuffer;
    // source info
    DISP_SOURCE_INFO_S stSrcInfo;

    MT_DF_VIDEO_INFO stCurrVideoInfo;

    //lastest display and config.//
    DF_DISP_BUF_NODE_S *pstDispNxt;//next disp buffer, used for DI
    DF_DISP_BUF_NODE_S *pstDispCur;//cur disp buffer
    DF_DISP_BUF_NODE_S *pstDispPre;//previous disp buffer, used for DI
    DF_DISP_BUF_NODE_S *pstDispPrePre;//prepre disp buffer, used for DI , data in pre node may be singular,
    DF_DISP_BUF_NODE_S *apstDispTrash[DISP_POOL_MAX_TRASH_NODES_NUM];//node could be released

    //cur config nodes, may be invalid
    DISP_CFG_DI_ENABLE stCurCfg_DI_Enable;
    DISP_CFG_DI_ENABLE stLastCfg_DI_Enable;
    DISP_CFG_DI_ENABLE stFreezeCfg_DI_Enable;

    MT_U32 pMotionData;
    MT_U8  u8DispChanId;
    MT_U32 gTransformFrameIndex;

    MT_DF_VIDEO_FRAME_S g_stFrame_TempField;
    MT_DF_VIDEO_FRAME_S g_stFrame_CurEnqueue;

    DF_CONTROL_INFO_S g_Disp_Ctl_Info;
    MT_DF_CSC_INFO_T  last_csc_info;
    tv_mode_t last_tv_mode;
    MT_U32 last_disp_brightness;

    DISP_MMZ_BUF_S stHdr10pOotfTable;
    DISP_MMZ_BUF_S stHdr10pCommonAddr;
    DISP_MMZ_BUF_S stSlHdrLutAddr;

    DISP_MMZ_BUF_S stFreezeFrameBuffer;
    MT_U8 u8FreezeBufferType;
    MT_U8 u8freezeBufferStatus;
}DISP_POOL_S;

#ifdef __cplusplus
}
#endif
#endif

