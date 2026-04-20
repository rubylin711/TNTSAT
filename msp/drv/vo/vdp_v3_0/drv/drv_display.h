
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_display.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#ifndef __DRV_DISPLAY_H__
#define __DRV_DISPLAY_H__

#include "mt_type.h"
#include "mt_drv_video.h"
#include "mt_drv_disp.h"

#include "vo_fw.h"
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
/*!
 *  disable cable detect 
 *     */
#define CABLE_DETECT  0 

/*!
 *   use oversample clock
 *     */
#define OVERSAMPLE OS_L
#endif

#if defined(CONFIG_MT_CHIP_ARIA)
#define IRQ_ARIA_HD_GROUP0_ID (32+38)
#define IRQ_ARIA_HD_GROUP1_ID (32+39)
#define IRQ_ARIA_SD_TOP_START_ID (32+40)
#define IRQ_ARIA_SD_BOT_START_ID (32+41)
#define IRQ_ARIA_PS_END_ID       (32+21)
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#define IRQ_ARIA_HD_GROUP0_ID IRQ_HD_BOT_START_ID
#define IRQ_ARIA_HD_GROUP1_ID IRQ_HD_BOT_START_ID
#define IRQ_ARIA_SD_TOP_START_ID IRQ_SD_VENC_ID
//#define IRQ_ARIA_SD_BOT_START_ID (32+41)
//#define IRQ_ARIA_PS_END_ID       (32+21)
#endif

#define MT_DRV_DISP_OFFSET_MAX 200
#define MT_DRV_DISP_OFFSET_HORIZONTAL_ALIGN 0xFFFFFFFEul
#define MT_DRV_DISP_OFFSET_VERTICAL_ALIGN   0xFFFFFFFCul

#define MT_DRV_DISP_VIRTSCREEN_MAX 4096
#define MT_DRV_DISP_VIRTSCREEN_MIN 480

#define ALIGN_SHIFT    (3)
/*!
   INT(x/y) * y
  */
#define ROUNDDOWN(x, y)         ((x) & ~((y) - 1))


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

mt_s32 DISP_Init(mt_void);
mt_s32 DISP_DeInit(mt_void);
mt_s32 DISP_Close_AllLayer(mt_void);

mt_s32 DISP_Suspend(mt_void);
mt_s32 DISP_Resume(mt_void);

mt_s32 DISP_Attach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);
mt_s32 DISP_Detach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave);

mt_s32 DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E enStereo, MT_DRV_DISP_FMT_E enEncFmt);
mt_s32 DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E *penStereo, MT_DRV_DISP_FMT_E *penEncFmt);

mt_s32 DISP_SetCustomTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming);
mt_s32 DISP_GetCustomTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming);

mt_s32 DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);
mt_s32 DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf);


mt_s32 DISP_Open(MT_DRV_DISPLAY_E enDisp);
mt_s32 DISP_Close(MT_DRV_DISPLAY_E enDisp);

mt_s32 DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable);

mt_s32 DISP_ExternSetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable);
mt_s32 DISP_SetVirtScreen(MT_DRV_DISPLAY_E enDisp, mt_rect_s virtscreen);
mt_s32 DISP_GetVirtScreen(MT_DRV_DISPLAY_E enDisp, mt_rect_s *virtscreen);
mt_s32 DISP_SetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_rect_s virtscreen);
mt_s32 DISP_GetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_rect_s *virtscreen);

mt_s32 DISP_SetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstScreenoffset);
mt_s32 DISP_GetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstScreenoffset);

mt_s32 DISP_SetBGColor(MT_DRV_DISPLAY_E eDisp, MT_DRV_DISP_COLOR_S *pstBGColor);
mt_s32 DISP_GetBGColor(MT_DRV_DISPLAY_E eDisp, MT_DRV_DISP_COLOR_S *pstBGColor);

#if ((!defined(__DISP_PLATFORM_BOOT__)) && defined(DAC_TYPE_SYNOPSYS))
//set gamma
mt_s32 DISP_UpdateGamma(GAMMA_CS_E   enGammaCsMode,PQ_GAMMA_RGB_MODE_S* pstPqGammaModeData);
mt_s32 DISP_SetGammaCtrl(MT_DRV_DISPLAY_E eDisp,GAMMA_MODE_E enGammaMode,MT_BOOL bEnable);

#endif

//set aspect ratio: 0 and 0 means auto
mt_s32 DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v);
mt_s32 DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v);

mt_s32 DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_ZORDER_ABS_E enZFlag);
mt_s32 DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder);

//miracast
mt_s32 DISP_CreateCast(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S * pstCfg, mt_handle *phCast);
mt_s32 DISP_DestroyCast(mt_handle hCast);
mt_s32 DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable);
mt_s32 DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable);

mt_s32 DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame);
mt_s32 DISP_External_Attach(mt_handle hCast, mt_handle hsink);
mt_s32 DISP_External_DeAttach(mt_handle hCast, mt_handle hsink);

mt_s32 DRV_DISP_SetCastAttr(mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *castAttr);
mt_s32 DRV_DISP_GetCastAttr(mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *castAttr);
mt_s32 DispGetCastHandle(MT_DRV_DISPLAY_E enDisp, mt_handle *phCast, mt_handle *phCast_ptr);

//snapshot
mt_s32 DISP_AcquireSnapshot(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S* pstSnapShotFrame, mt_handle *snapshotHandleOut);
mt_s32 DISP_ReleaseSnapshot(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S* pstSnapShotFrame, mt_handle snapshotHandle);
mt_s32 DISP_DestroySnapshot(mt_handle hSnapshot);

//Macrovision
mt_s32 DISP_TestMacrovisionSupport(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbSupport);
mt_s32 DISP_SetMacrovisionCustomer(MT_DRV_DISPLAY_E enDisp, mt_void *pData);
mt_s32 DISP_SetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E enMode);
mt_s32 DISP_GetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E *penMode);

//cgms-a
mt_s32 DISP_SetCGMS_A(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CGMSA_CFG_S *pstCfg);

//vbi
mt_s32 DISP_CreateVBIChannel(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi);
mt_s32 DISP_DestroyVBIChannel(mt_handle hVbi);
mt_s32 DISP_SendVbiData(mt_handle hVbi, MT_DRV_DISP_VBI_DATA_S *pstVbiData);
mt_s32 DISP_SetWss(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_WSS_DATA_S *pstWssData);
mt_s32 disp_set_bright(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_set_contrast(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_set_saturation(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_set_hue(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_set_colorbar(MT_DRV_DISPLAY_E enDisp, MT_BOOL bOn);
mt_s32 disp_set_output_enable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bOn);

//may be deleted
//setting
//mt_s32 DISP_SetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting);
//mt_s32 DISP_GetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting);
//mt_s32 DISP_ApplySetting(MT_DRV_DISPLAY_E enDisp);


/*****************************************************/
//internal state
mt_s32  DISP_GetInitFlag(MT_BOOL *pbInited);
mt_s32  DISP_GetVersion(MT_DRV_DISP_VERSION_S *pstVersion);
MT_BOOL DISP_IsOpened(MT_DRV_DISPLAY_E enDisp);
MT_BOOL DISP_IsFollowed(MT_DRV_DISPLAY_E enDisp);
MT_BOOL DISP_IsSameSource(MT_DRV_DISPLAY_E enDisp);

mt_s32 DISP_GetSlave(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penSlave);
mt_s32 DISP_GetMaster(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penMaster);
mt_s32 DISP_GetDisplayInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo);
mt_s32 DISP_GetDisplaySetting(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E *penFormat, MT_DRV_DISP_STEREO_E *peDispMode);
//isr
mt_s32 DISP_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                             MT_DRV_DISP_CALLBACK_S *pstCB);
mt_s32 DISP_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                             MT_DRV_DISP_CALLBACK_S *pstCB);

mt_u32 Disp_GetFastbootupFlag(mt_void);
mt_u32 Disp_SetFastbootupFlag(mt_u32 u32Value);
MT_BOOL DispCheckIntfExistByType(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S* pstIntf);


mt_s32 DISP_SetDACDetEn(MT_BOOL bDACDetEn);
mt_s32 DISP_GetDACAttr(MT_DRV_VDAC_ATTR_S *pDACAttr);
mt_s32 DISP_SetAllDacEn(MT_BOOL bDacEn);
#define DispCheckID(id)    \
    {                                \
        if ( (id >= MT_DRV_DISPLAY_BUTT) /*|| (id < MT_DRV_DISPLAY_0)*/)  \
        {                            \
            DISP_ERROR("DISP ERROR! Invalid display in %s!\n", __FUNCTION__); \
            return MT_ERR_DISP_INVALID_PARA;  \
        }                             \
    }

mt_void disp_set_dacmode(MT_DRV_DISPLAY_E enDisp, vdac_type_t dacMode);
mt_s32 disp_set_layershow(MT_DRV_DISPLAY_E enDisp, DISP_LAYERSHOW_S *arg);
mt_s32 disp_set_set_postprocess_mode(MT_DRV_DISPLAY_E enDisp, DISP_PP_S *arg);
mt_s32 disp_set_vid_layer_show(MT_BOOL bEnable);
mt_s32 disp_get_vid_layer_show(MT_BOOL *pbEnable);
mt_s32 disp_get_dce_percent(mt_s32 ChanID, mt_u32 *pDce);
mt_u32 disp_set_layer_alpha(MT_DRV_DISPLAY_E enDisp, mt_u32 alpha);
mt_u32 disp_get_layer_alpha(MT_DRV_DISPLAY_E enDisp, mt_u32 *alpha);
mt_u32 disp_set_csc_onoff(MT_BOOL  bEnable);
mt_u32 disp_set_denoise_onoff(MT_BOOL  bEnable);
mt_u32 disp_set_afd_onoff(MT_BOOL  bEnable);
mt_s32 disp_set_hd_video_onoff(MT_BOOL  bEnable);
mt_s32 disp_set_sd_video_onoff(MT_BOOL  bEnable);
mt_s32 disp_set_vac_onoff(dac_index_t dac_id, MT_BOOL bEnable);
mt_s32 disp_set_tv_capability(MT_DRV_DISP_HDMI_MODE_E enTvCap);
mt_u32 disp_hal_set_sd_venc_reg(DISP_SD_ENC_PQ_PARA_S* p_pq_para);
mt_s32 DispResetHardware(MT_BOOL b_clock_hi);
mt_s32 disp_tvsys_force_update(void);
mt_s32 disp_set_bright(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_get_bright(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent);
mt_s32 disp_set_contrast(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_get_contrast(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent);
mt_s32 disp_set_saturation(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_get_saturation(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent);
mt_s32 disp_set_hue(MT_DRV_DISPLAY_E enDisp, mt_u32 percent);
mt_s32 disp_get_hue(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent);
mt_s32 disp_set_1001_enable(MT_BOOL bEnable);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
mt_s32 disp_set_sl_hdr(MT_DRV_DISPLAY_E enDisp, mt_u32 transparent_mode, mt_u32 display_Brightness, mt_u32 tuning_level, mt_u32 display_OETF);
#endif

mt_void disp_st_vid_set_vdec_size(mt_u32 height, mt_u32 width);
mt_void disp_update_tde_video_size(mt_void);
mt_s32 disp_dump_scaler_to_osd(mt_handle pstDispBP, MT_DRV_DISP_DUMP_SCALER_PARA_S *pstParam);

#define VBI_DATA_BUF_LEN      65536

/**Defines VBI type.*/

typedef enum mt_DISP_VBI_TYPE_E
{
    MT_DISP_VBI_TYPE_TTX = 0,
    MT_DISP_VBI_TYPE_CC,
    MT_DISP_VBI_TYPE_VCHIP,
    MT_DISP_VBI_TYPE_WSS,
    MT_DISP_VBI_TYPE_VPS,
    MT_DISP_VBI_TYPE_CGMS_A,
    MT_DISP_VBI_TYPE_CC_PES,
    MT_DISP_VBI_TYPE_TTX_ES,
    MT_DISP_VBI_TYPE_BUTT,
} MT_DISP_VBI_TYPE_E;

typedef struct
{
    mt_char u8DispVbiBuffer[VBI_DATA_BUF_LEN];
    mt_u32 vbi_wp;
    mt_u32 vbi_rp;
    MT_DISP_VBI_TYPE_E vbi_type;
    mt_u32 vbi_int_mask0_set;
    MT_BOOL bEnable;
}disp_vbi_buffer_s;

extern disp_vbi_buffer_s disp_vbi_buffer;

#ifndef __DISP_PLATFORM_BOOT__

#define  Cast_BUFFER_NUM  20
typedef struct tagDISP_Cast_PROC_INFO_S
{
    MT_BOOL     bEnable;
    MT_BOOL     bLowDelay;
    MT_BOOL     bUserAllocate;
    MT_BOOL     bAttached;

    mt_u32      u32OutResolutionWidth;
    mt_u32      u32OutResolutionHeight;
    mt_u32      u32CastOutFrameRate;

    mt_u32      u32TotalBufNum;
    mt_u32      u32BufSize;
    mt_u32      u32BufStride;
    mt_u32      u32CastAcquireTryCnt;
    mt_u32      u32CastAcquireOkCnt;
    mt_u32      u32CastReleaseTryCnt;
    mt_u32      u32CastReleaseOkCnt;

    mt_u32      u32CastIntrCnt;

    mt_u32      u32CastEmptyBufferNum;
    mt_u32      u32CastFullBufferNum;
    /*be written by wbc.*/
    mt_u32      u32CastWriteBufferNum;
    /*be used by user, not returned back.*/
    mt_u32      u32CastUsedBufferNum;
    mt_u32     enState[Cast_BUFFER_NUM];
    mt_u32       u32FrameIndex[Cast_BUFFER_NUM];
}DISP_Cast_PROC_INFO_S;

typedef struct tagDISP_PROC_INFO_S
{
    MT_BOOL bEnable;
    MT_BOOL bMaster;
    MT_BOOL bSlave;
    MT_DRV_DISPLAY_E enAttachedDisp;

    //about encoding format
    MT_DRV_DISP_STEREO_E eDispMode;
    mt_rect_s stVirtaulScreen;
    MT_DRV_DISP_OFFSET_S stOffsetInfo;
    MT_BOOL bRightEyeFirst;
    MT_DRV_DISP_FMT_E eFmt;

    MT_BOOL bCustAspectRatio;
    mt_u32 u32AR_w;
    mt_u32 u32AR_h;

    mt_u32 u32Bright;
    mt_u32 u32Hue;
    mt_u32 u32Satur;
    mt_u32 u32Contrst;

    MT_DRV_DISP_LAYER_E enLayer[MT_DRV_DISP_LAYER_BUTT]; /* Z-order is from bottom to top */
    MT_DRV_COLOR_SPACE_E eDispColorSpace;

    // about color setting
    MT_DRV_DISP_COLOR_SETTING_S stColorSetting;
    MT_DRV_DISP_COLOR_S stBgColor;

    //interface
    mt_u32 u32IntfNumber;
    MT_DRV_DISP_INTF_S stIntf[MT_DRV_DISP_INTF_ID_MAX];
    mt_u32 u32Link[MT_DRV_DISP_INTF_ID_MAX];


    MT_DRV_DISP_TIMING_S stTiming;
    mt_u32 u32Underflow;
    mt_u32 u32StartTime;

    /*Cast infor.*/
    mt_handle pstCastInfor;
    DISP_Cast_PROC_INFO_S stCastInfor;
}DISP_PROC_INFO_S;

#define BIGL(A)        ((((unsigned int)(A) & 0xff000000) >> 24) | \
                                                       (((unsigned int)(A) & 0x00ff0000) >> 8) | \
                                                       (((unsigned int)(A) & 0x0000ff00) << 8) | \
                                                       (((unsigned int)(A) & 0x000000ff) << 24))

typedef struct
{
    mt_u8 u8Red;
    mt_u8 u8Green;
    mt_u8 u8Blue;

    mt_u8 u8Y;
    mt_u8 u8Cb;
    mt_u8 u8Cr;
}ALG_COLOR_S;

mt_s32 DISP_GetProcInto(MT_DRV_DISPLAY_E enDisp, DISP_PROC_INFO_S *pstInfo);
#endif


///////////////////////////////////////////////////////////////////////////////
// Display(VO) Firmware APIs

extern mt_s32 DF_Create(mt_handle* pstDispPool, mt_u32 u32BufNumber, MT_BOOL bCloseHdDI);
extern mt_s32 DF_Destroy(mt_handle pstDispPool);

extern mt_s32 DF_TransformDec2Disp_sinfo(mt_handle pstDispPool, MT_DRV_VIDEO_FRAME_S *pFrameInfo, MT_DF_VIDEO_FRAME_S *pstVideoFrame);
extern mt_s32 DF_TransformDec2Disp_sinfo1(IMAGE *pstImage, MT_DF_VIDEO_FRAME_S *pstVideoFrame);

extern mt_s32 DF_TSK_enQueue(mt_handle pstDispPool, MT_DF_VIDEO_FRAME_S *pstVideoFrame, MT_BOOL *pbNeedNewData);
extern mt_void DF_ISR_Update(mt_handle pstDispPool, DF_DRV_SETTING_S *pstDrvSetting);

extern MT_DF_STATUS DF_GetStatus(mt_handle pstDispPool);
extern mt_s32 DF_SetCmd(mt_handle pstDispPool, MT_DF_CMD eCMD, MT_VOID* pArgs);
extern mt_void DF_FlushDisp(mt_handle pstDispPool, MT_DF_FLUSH_TYPE_E eType);

extern mt_void DF_SetFreezeBuffer(mt_handle pstDispPool, phys_addr_t u32PhyAddr, MT_U32 u32Size);
extern mt_u32 DF_TestFreezeDone(mt_handle pstDispPool);
extern mt_void DF_Wait_For_FreezeCopy_Done(mt_handle pstDispPool);

extern mt_u32 DF_GetVideoCoeffTable(VIDEO_SCALE_COEFF_TABLE_E table);

extern mt_s32 DF_SetSource(mt_handle pstDispPool, DISP_SOURCE_INFO_S *pstSrc);

extern mt_s32 DF_AvsyncGetVptsInfo(mt_handle pstDispPool, DF_AVSYNC_PTS_S *pInfo);
extern mt_s32 DF_AvsyncGetInputRate(mt_handle pstDispPool, mt_u32 *pInputRate);
extern mt_s32 DF_AvsyncSetSkipFrame(mt_handle pstDispPool, mt_u32 skip_num);
extern mt_s32 DF_AvsyncSetRepeatFrame(mt_handle pstDispPool);
extern mt_s32 DF_AvsyncSetPause(mt_handle pstDispPool, MT_BOOL is_play);
extern mt_s32 DF_CheckFifoEmpty(mt_handle pstDispPool, MT_BOOL *bEmpty);

extern mt_s32 DF_GraScaler_Update(DF_DRV_SETTING_S *pDrvSetting);
extern mt_s32 DF_OsdScaler_Update(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height);
extern mt_s32 DF_StillScaler_Update(mt_handle pstDispPool,MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive);
extern mt_void DF_DumpScaler_Update(mt_handle pstDispPool, MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height);
extern mt_s32 DF_InitCoeffTable(void);

extern mt_s32 DF_GetCurrVideoInfo(mt_handle pstDispPool, MT_DF_VIDEO_INFO* vid_info);
extern mt_void DF_UpdateOsdCsc(mt_u32 dst_tv_mode);
extern mt_void DF_UpdateSdCsc(mt_u32 dst_tv_mode);
extern mt_u32 DF_GetSlHdrVersion(void);

////
//extern MT_RET PS_AddOrder(PRESCALE_ORDERINFO* pstOrderInfo);
extern int PS_AddOrder(PRESCALE_ORDERINFO* pstOrderInfo);
extern void PS_Loop(mt_void);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /*  __DRV_DISPLAY_H__  */

