/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __OPTM_HDMI_H__
#define __OPTM_HDMI_H__

#include "mt_error_mpi.h"
//#include "mpi_priv_hdmi.h"
#include "mt_drv_hdmi.h"
#include "mt_drv_disp.h"
#include "drv_hdmi_ioctl.h"
#include "hdmi20_drv/Tx_Common/driver/tx/si_drv_tx_api.h"
#include "mt_drv_disp.h"
#include "si_app_cec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define DRV_HDMI_NOTIFY_MAX_NUMBER 3
#define HDMI_AUDIO_CONFIG_TIMES  30

typedef mt_s32 (*hdmi_notify_t)(mt_u32 event, mt_u32 param, ulong context);

typedef enum {
	/*!
	  HDMI event nofity for hdmi driver
	  */
	HDMI_NOTIFY_ID_TYPE_HDMI_DRV,

	/*!
	  HDMI event nofity for audio driver
	  */
	HDMI_NOTIFY_ID_TYPE_AUDIO,

	/*!
	  HDMI event nofity for display/vidoe
	  */
	HDMI_NOTIFY_ID_TYPE_DISPLAY,

	/*!
	  HDMI event nofity for other
	  */
	HDMI_NOTIFY_ID_TYPE_OTHER,
} HDMI_NOTIFI_ID_TYPE_E;

typedef struct {
	/*!
	    HDMI event ids this notify should supported, see hdmi_event_id_t
	    */
	mt_u32 events;

	/*!
	    HDMI event notify function registered
	   */
	hdmi_notify_t notify;

	/*!
	    the context to be transfered back by calling event notify function
	   */
	ulong context;

	/*!
	   the context to be transfered back by calling event notify function
	   */
	HDMI_NOTIFI_ID_TYPE_E id_type;

} hdmi_notify_info_t;

/*!
  HDMI event need to be notified to up-layer
  */
typedef enum {
    /*!
    HDMI port connection event, param is 0 means Disconnected, 1 means Connected
    */
    HDMI_EVENT_CONNECTION_STATUS = 0x01,
    /*!
    cec message, for param see hdmi_ece_msg_t,
    */
    HDMI_EVENT_CEC_MESSAGE = 0x02,
    /*!
    Unsupported Native/Preffered video format, param: 16 LSB - standard, 16 MSB - resolution
    */
    HDMI_EVENT_UNSUPPORTED_FORMAT = 0x04,
    /*!
    hdcp status, param is 1 means authenticated fail, 0 means Authenticated pass
    */
    HDMI_EVENT_HDCP_AUTH_STATUS = 0x08,
    /*!
    HDMI video clock change
    */
    HDMI_EVENT_VIDEO_CLK_CHG = 0x10,
    /*!
    HDMI video format change
    */
    HDMI_EVENT_VIDEO_FMT_CHG = 0x20,
    /*!
     HDMI audio clock change
    */
    HDMI_EVENT_AUDIO_CLK_CHG = 0x40,
    /*!
	HDMI hdcp config change
	*/
    HDMI_EVENT_HDCP_CFG_CHG = 0x80,
    /*!
	HDMI hdcp config change, 0 not bksv data, 1 is valid bksv
	*/
    HDMI_EVENT_HDCP_CAPACITY_STATUS = 0x100,
    /*!
	HDMI cec receieve message ready
	*/
    HDMI_EVENT_CEC_MSG_READY = 0x200,
    /*!
    HDMI cec message status changed
    */
    HDMI_EVENT_CEC_STATUS_CHANGED = 0x400,
    /*!
    HDMI port inner connection event, param is 0 means Disconnected, 1 means Connected,  used for audio,display config
    */
    HDMI_EVENT_INNER_CONNECTION_STATUS = 0x800,

    /*!
    HDMI  used for notify video config
    */
    HMDI_VIDEO_CONFIG_EVENT = 0x1000,

    /*!
      HDMI  used for notify audio config
    */
    HMDI_AUDIO_CONFIG_EVENT = 0x2000,

} hdmi_event_id_t;

mt_u32 DRV_HDMI_Init(mt_u32 FromUserSpace);
mt_u32 DRV_HDMI_DeInit(mt_u32 FromUserSpace);
mt_u32 DRV_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi, HDMI_OPEN_S *pOpen, mt_u32 FromUserSpace, mt_u32 u32ProcID);
mt_u32 DRV_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi);
mt_u32 DRV_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkAttr);
mt_u32 DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_ATTR_S *pstAttr);
mt_u32 DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_ATTR_S *pstAttr);
mt_u32 DRV_HDMI_SetCECCommand(MT_UNF_HDMI_ID_E enHdmi, const MT_UNF_HDMI_CEC_CMD_S  *pCECCmd);
mt_u32 DRV_HDMI_GetCECCommand(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_CMD_S  *pCECCmd, mt_u32 timeout);
mt_u32 DRV_HDMI_CECStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_CEC_STATUS_S  *pStatus);
mt_u32 DRV_HDMI_SetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
mt_u32 DRV_HDMI_GetInfoFrame(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);
mt_u32 DRV_HDMI_ReadEvent(MT_UNF_HDMI_ID_E enHdmi, mt_u32 procID);
void DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_TYPE_E event);
mt_u32 DRV_HDMI_Start(MT_UNF_HDMI_ID_E enHdmi);
mt_u32 DRV_HDMI_Stop(MT_UNF_HDMI_ID_E enHdmi);
mt_u32 DRV_HDMI_SetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E enDeepColor);
mt_u32 DRV_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor);
mt_u32 DRV_HDMI_SetxvYCCMode(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bEnable);
mt_u32 DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bAvMute);
mt_u32 DRV_HDMI_PreFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enEncodingFormat);
mt_u32 DRV_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo);
mt_u32 DRV_HDMI_Force_GetEDID(HDMI_EDID_S *pEDID);
mt_u32 DRV_HDMI_GetPlayStatus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus);
mt_u32 DRV_HDMI_GetCECAddress(MT_U8 *pPhyAddr, MT_U8 *pLogicalAddr);
mt_u32 DRV_HDMI_LoadKey(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_LOAD_KEY_S *pstLoadKey);

mt_s32 DRV_HDMI_GetProcID(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32ProcID);
mt_s32 DRV_HDMI_ReleaseProcID(MT_UNF_HDMI_ID_E enHdmi, mt_u32 u32ProcID);

mt_s32 DRV_HDMI_AudioChange(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
mt_s32 DRV_HDMI_GetAOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr);
//mt_s32 DRV_HDMI_AdjustInfoFrame(MT_UNF_HDMI_ID_E enHdmi,MT_UNF_HDMI_INFOFRAME_S *pstInfoFrame);

mt_s32 DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 DRV_HDMI_GetProcNum(MT_UNF_HDMI_ID_E enHdmi);

mt_s32 DRV_HDMI_SetAPPAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstHDMIAppAttr, MT_BOOL UpdateFlag);
mt_s32 DRV_HDMI_SetAOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr, MT_BOOL UpdateFlag);
mt_s32 DRV_HDMI_SetVOAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_VIDEO_ATTR_S *pstHDMIVOAttr, MT_BOOL UpdateFlag);
//mt_s32 DRV_HDMI_SetHDMIAttr(MT_UNF_HDMI_ID_E enHdmi,MT_UNF_HDMI_ATTR_S *pstHDMIAttr);
//mt_s32 DRV_HDMI_ConfigAttr(MT_UNF_HDMI_ID_E enHdmi);

MT_UNF_ENC_FMT_E hdmi_Disp2EncFmt(MT_DRV_DISP_FMT_E SrcFmt);
MT_DRV_DISP_FMT_E hdmi_ENC2DispFmt(MT_UNF_ENC_FMT_E SrcFmt);

mt_s32 DRV_HDMI_GetStatus(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_STATUS_S *pHdmiStatus);

mt_s32 DRV_HDMI_GetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pHdmiDelay);
mt_s32 DRV_HDMI_SetDelay(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DELAY_S *pHdmiDelay);

mt_s32 DRV_HDMI_Register(mt_void);
mt_s32 DRV_HDMI_UnRegister(mt_void);

mt_void DRV_O5_HDMI_PutBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, mt_void* infor_ptr);
mt_void DRV_O5_HDMI_GetBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, mt_void *infor_ptr);

mt_u32 DRV_HDMI_Registers_Dump(MT_UNF_HDMI_ID_E enHdmi);
mt_u32 DRV_HDMI_Register_Read(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u8 *data);
mt_u32 DRV_HDMI_Register_Write(MT_UNF_HDMI_ID_E enHdmi, mt_u32 addr, mt_u32 data);
mt_s32 DRV_HDMI_Set_Video(mt_u32 *param, MT_UNF_HDMI_ID_E enHdmi);
mt_s32 DRV_HDMI_Output_Set(mt_u32 param);
mt_s32 DRV_HDMI_Set_Notify(mt_u32 *param);
mt_s32 DRV_HDMI_UserPacketRptEnable(MT_UNF_HDMI_ID_E hdmi_id, mt_u8 packet_id, MT_BOOL enable);
mt_s32 DRV_HDMI_Clk_Cfg(MT_UNF_HDMI_ID_E hdmi_id, HDMI_CLK_CFG_S* cfg_info);
mt_s32 DRV_HDMI_Emp_Cfg(MT_UNF_HDMI_ID_E hdmi_id, void* cfg_info, mt_u32 len);
mt_s32 DRV_HDMI_GetRawEdidInfo(MT_UNF_HDMI_ID_E enHdmi, mt_u8 *rawEdidInfo);
void DRV_HDMI_CECNotifyEvent(SiiCecEvent_E event);
mt_s32 DRV_HDMI_SetOsdName(MT_UNF_HDMI_ID_E hdmi_id, mt_u8 *osd_name);

typedef enum VIDEO_SAMPLE_TYPE_E_S {
	VIDEO_SAMPLE_TYPE_UNKNOWN,                  /**<Unknown*/ /**<CNcomment: 未知采样方式*/
	VIDEO_SAMPLE_TYPE_PROGRESSIVE,              /**<Progressive*/ /**<CNcomment: 采样方式为逐行*/
	VIDEO_SAMPLE_TYPE_INTERLACE,                /**<Interlaced*/ /**<CNcomment: 采样方式为隔行*/
	VIDEO_SAMPLE_TYPE_BUTT
} VIDEO_SAMPLE_TYPE_E;

enum hdmi_switch_state {
	STATE_PLUG_UNKNOWN = -1,
	STATE_HOTPLUGOUT = 0,
	STATE_HOTPLUGIN = 1
};

typedef struct {
	mt_u8 type;
	mt_u8 version;
	mt_u8 length;
	mt_u8 pb_byte[28];
} info_struct_t;

/*!
  select input pixel repeate times
  */
typedef enum {
	/*!
	pixel repeate 1 times
	*/
	PIXEL_RPT_1_TIMES = 1,
	/*!
	pixel repeate 2 times
	*/
	PIXEL_RPT_2_TIMES = 2,
	/*!
	pixel repeate 4 times
	*/
	PIXEL_RPT_4_TIMES = 4,
} hdmi_input_video_pixel_rpt_t;

/*!
  HDMI display output shape.
  */
typedef enum {
	/*!
	default setting
	*/
	HDMI_SHAPE_DEF,
	/*!
	4 X 3
	*/
	HDMI_SHAPE_4X3,
	/*!
	16 X 9
	*/
	HDMI_SHAPE_16X9,
} hdmi_disp_output_shape_t;

/*!
  User defined infoframe type
  */
typedef enum {
	/*!
	  unknown SPD type
	  */
	HDMI_USRDF_INFO_UNKNOWN     = 0x0,
	/*!
	  HDR type
	  */
	HDMI_USRDF_INFO_HDR         = 0x87,

	/*!
	  Maximum number of USRDF type
	  */
	HDMI_USRDF_INFO_MAX,
} hdmi_usrdf_info_type_t;

/*!
  User defined infoframe
  */
typedef struct {
	hdmi_usrdf_info_type_t type;
	mt_u8 version;
	mt_u8 length;
	u8 checksum;
	mt_u8 pb_byte[27];
} hdmi_usrdf_infoframe_t;

/*!
  vsif infoframe
  */
typedef struct {
	mt_u8 type;
	mt_u8 version;
	mt_u8 length;
	mt_u8 pb_byte[28];
} hdmi_vsif_infoframe_t;

/*!
  hdmi input video configuration
  */
typedef struct {
	SiiDrvClrSpc_t csc;
	SiiDrvConvStd_t std;
	SiiDrvBitDepth_t bitDepth;
	hdmi_input_video_pixel_rpt_t pixel_rpt;
	hdmi_disp_output_shape_t shape;
	mt_u8 hdr_onoff;
	hdmi_usrdf_infoframe_t usrdf_info[2];
	hdmi_vsif_infoframe_t vsif_infoframe;
	disp_sys_t fmt;
} hdmi_output_video_cfg_t;

/*!
  hdmi input video configuration
  */
typedef struct {
	SiiDrvClrSpc_t csc;
	SiiDrvConvStd_t std;
	SiiDrvBitDepth_t bitDepth;
	hdmi_input_video_pixel_rpt_t pixel_rpt;
	mt_u8 video_full_range;
} hdmi_input_video_cfg_t;

/*!
  HDMI display 3D output resolution.
  */
typedef enum {
	/*!
	Not use 3D resolution
	*/
	HDMI_3D_UNDEFINE,
	/*!
	Frame packing
	*/
	HDMI_3D_FRAME_PACKING,
	/*!
	Side-by-Side (Half)
	*/
	HDMI_3D_SIDE_BY_SIDE_HALF,
	/*!
	Top-and Bottom
	*/
	HDMI_3D_TOP_AND_BOTTOM,
	/*!
	Field alternative
	*/
	HDMI_3D_FIELD_ALT,
	/*!
	Line alternative
	*/
	HDMI_3D_LINE_ALT,
	/*!
	Side-by-Side (Full)
	*/
	HDMI_3D_SIDE_BY_SIDE_FULL,
	/*!
	L + depth
	*/
	HDMI_3D_L_DEPTH,
	/*!
	L + depth + graphics
	*/
	HDMI_3D_L_DEPTH_GRAPHICS,

} hdmi_3d_resolution_t;

/*!
  HDMI  3D ext data for Side-by-Side (Half).
  */
typedef enum {
	/*!
	Horizontal sub-sampling
	*/
	HDMI_3D_HORIZONTAL,
	/*!
	Odd/left Odd/right piecture
	*/
	HDMI_3D_ODD_LEFT_ODD_RIGHT,
	/*!
	Odd/left Eveen/right piecture
	*/
	HDMI_3D_ODD_LEFT_EVEEN_RIGHT,
	/*!
	Eveen/left Odd/right piecture
	*/
	HDMI_3D_EVEEN_LEFT_ODD_RIGHT,
	/*!
	Eveen/left Eveen/right piecture
	*/
	HDMI_3D_EVEEN_LEFT_EVEEN_RIGHT,
} hdmi_3d_ext_data_t;

/*!
  HDMI 3D configuration
  */
typedef struct {
	/*!
	3D resolution
	*/
	hdmi_3d_resolution_t hdmi_3d_res;
	/*!
	3D ext data
	*/
	hdmi_3d_ext_data_t hdmi_3d_ext;
	/*!
	3D Metadate type
	*/
	//mt_u8 metadata_type;
	/*!
	3D Metadate
	*/
	//mt_u8 metadata[8];
} hdmi_3d_config_t;

typedef struct hdmi20_video_cfg_s {
	/*!
	input video configuration
	*/
	hdmi_input_video_cfg_t input_v_cfg;
	/*!
	output video configuration
	*/
	hdmi_output_video_cfg_t output_v_cfg;
	/*!
	0: HDCP off, 1: HDCP on
	*/
	mt_u32 hdcp_on_off;
	/*!
	3D configuration
	*/
	hdmi_3d_config_t hdmi_3d_cfg;
} hdmi_video_config_t;

typedef struct EMP_S {
	uint8_t		hb[4];
	uint8_t		pb[28];
} EMP_T;
#define EMP_MD_NUM	(21)
typedef struct EMP_EM_S {
	bool_t	first;
	bool_t	last;
	uint8_t	seq_idx;
	bool_t	New;
	bool_t	end;
	uint8_t	DS_Type;
	bool_t	AFR;
	bool_t	VFR;
	bool_t	Sync;
	uint8_t	Organization_ID;
	uint8_t	Data_Set_Tag_MSB;
	uint8_t	Data_Set_Tag_LSB;
	uint8_t	Data_Set_Length_MSB;
	uint8_t	Data_Set_Length_LSB;
	uint8_t	MD[EMP_MD_NUM];
} EMP_EM_T;
typedef enum EMP_TYPE_E {
	EMP_TYPE_HDR_DYNAMIC,
	EMP_TYPE_COMP_VT,  //NOT supported
	EMP_TYPE_VT,  //NOT supported
	EMP_TYPE_MAX
} EMP_TYPE_T;

typedef enum EXTENDED_INFOFRAME_TYPE_E {
	EXT_INFO_TYPE_IDLE,
	EXT_INFO_TYPE_HDR_DYNAMIC_ANNEX_R = 0x0001,
	EXT_INFO_TYPE_HDR_DYNAMIC_SEI_ETSI = 0x0002,
	EXT_INFO_TYPE_HDR_DYNAMIC_SEI_H265 = 0x0003,
	EXT_INFO_TYPE_HDR_DYNAMIC_ANNEX_S = 0x0004,
	EXT_INFO_TYPE_RAPHICS_OVERLAY_ANNEX_T = 0x0100,
} EXTENDED_INFOFRAME_TYPE_T;

typedef enum HDR_DATA_TYPE_E {
	HDR_DATA_TYPE_NONE,
	HDR_DATA_TYPE_HDR,
	HDR_DATA_TYPE_HLG,
	HDR_DATA_TYPE_VSIF,
	HDR_DATA_TYPE_EMP
} HDR_DATA_TYPE_T;

typedef enum MTW_CFG_MODE_E {
	MTW_CFG_MODE_0,
	MTW_CFG_MODE_1,
	MTW_CFG_MODE_2,
	MTW_CFG_MODE_auto
} MTW_CFG_MODE_T;

typedef struct EMP_DATA_IN_S {
	HDR_DATA_TYPE_T	mode;
	void *	datap;
	uint32_t	datal;
	void *	data1p;
	uint32_t	data1l;
	EXTENDED_INFOFRAME_TYPE_T	itype;
	EMP_TYPE_T	emp_type;
	bool_t	end;
	bool_t	repeat;
	MTW_CFG_MODE_T mtw_mode;
	bool_t dma_bank_auto;
	uint8_t	emp_dma_start;
} EMP_DATA_IN_T;

typedef enum DISP_HDMI_PARAMS_CHECK_STATUS_E {
	DISP_HDMI_PARAMS_STS_SUCCESS = 0,
	DISP_HDMI_PARAMS_STS_FMT_ERR = (1<<1),
	DISP_HDMI_PARAMS_STS_3D_ERR = (1<<2),
	DISP_HDMI_PARAMS_STS_EMP_ERR = (1<<3),
	DISP_HDMI_PARAMS_STS_EXP_INFO_ERR = (1<<4),
	DISP_HDMI_PARAMS_STS_HDR_ERR = (1<<5),
	DISP_HDMI_PARAMS_STS_HDCP_ERR = (1<<6),
	DISP_HDMI_PARAMS_STS_STD_ERR = (1<<7),
	DISP_HDMI_PARAMS_STS_CSC_ERR = (1<<8),
	DISP_HDMI_PARAMS_STS_BIT_DEPTH_ERR = (1<<9),
	DISP_HDMI_PARAMS_STS_REPET_ERR = (1<<10),
	DISP_HDMI_PARAMS_STS_RATIO_ERR = (1<<11),
	DISP_HDMI_PARAMS_STS_EDID_ERR = (1<<12),
	DISP_HDMI_PARAMS_STS_PTR_ERR = (1<<13),
	DISP_HDMI_PARAMS_STS_MAXCLK_ERR = (1<<14),
} DISP_HDMI_PARAMS_CHECK_STATUS_T;

typedef struct DISP_HDMI_PARAM_S {
	hdmi_video_config_t	disp2hdmi_vcfg;
	HDR_DATA_TYPE_T	mode;
	EXTENDED_INFOFRAME_TYPE_T	itype;
	EMP_TYPE_T	emp_type;
	MT_DRV_DISP_FMT_E  drv_disp_fmt;
	MT_DRV_DISP_STEREO_E  drv_disp_stereo;
} DISP_HDMI_PARAM_T;

mt_s32 DRV_HDMI_Vcfg_Check(MT_UNF_HDMI_ID_E enHdmi, void *param_in, void *param_out);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __OPTM_HDMI_H__ */

/*------------------------------------END-------------------------------------*/
