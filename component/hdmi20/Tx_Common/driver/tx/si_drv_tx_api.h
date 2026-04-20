/******************************************************************************
*
* Copyright 2013, Deco, Inc.  All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of
* Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
*
*****************************************************************************/
/**
* @file si_drv_tx_api.h
*
* @brief Tx API
*
*****************************************************************************/
#ifndef __SI_DRV_TX_API_H__
#define __SI_DRV_TX_API_H__

/***** #include statements ***************************************************/
#include "si_lib_obj_api.h"
#include "si_drv_cra_api.h"
#include "si_mod_tx_videopath_api.h"
#include "si_mod_tx_hdmi_api.h"
#include "si_mod_tx_hdcp_api.h"
#include "si_lib_edid_api.h"

/***** public macro definitions **********************************************/
#define SII_INFOFRAME_MAX_LEN           31
#define SII_EDID_BLOCK_SIZE             128
#define SII_EDID_TOTAL_BLOCKS           4
#define SII_EDID_TOTAL_BLOCKS_EEODB           8
#define SII_EDID_MAX_LEN                (SII_EDID_TOTAL_BLOCKS_EEODB * SII_EDID_BLOCK_SIZE)
#define SII_BKSV_LIST_BYTES             5
#define SII_HDCP2X_RCVID_LENGTH         5

/***** public type definitions ***********************************************/
typedef uint32_t SiiDrvTxEvent_t;
typedef void (*TxcbFunc)(SiiDrvTxEvent_t);

/**
* Event Types
*/
typedef enum {
	SII_DRV_TX_EVENT_TYPE__GENERAL,
	SII_DRV_TX_EVENT_TYPE__HDCP,
	SII_DRV_TX_EVENT_TYPE__VIDPATH,
	SII_DRV_TX_EVENT_TYPE__CPI,
	SII_DRV_TX_EVENT_TYPE__SCDC,
} SiiDrvTxEventType_t;

/**
* General Events
*/
typedef enum {
	SII_DRV_TX_EVENT__HOT_PLUG_CHNG				= 0x00000001,		//!< Hot-Plug status is changed
	SII_DRV_TX_EVENT__RSEN_CHNG					= 0x00000002,		//!< Rsen status is changed
	SII_DRV_TX_EVENT__EDID_CHNG					= 0x00000004,		//!< Downstream EDID is changed
	SII_DRV_TX_EVENT__HDMI_STATE_CHNG			= 0x00000008,       //!< TMDS Mode is changed/updated
	SII_DRV_TX_EVENT__HDCP_STATE_CHNG			= 0x00000010,       //!< HDCP status changed
	SII_DRV_TX_EVENT__CEC_CMD_RECEIVED			= 0x00000020,		//!< CEC Received //Todo
	SII_DRV_TX_EVENT__SCDC_EVENT				= 0x00000040,		//!< SCDC Event
} SiiDrvTxGenEvent_t;

/**
* Tx SCDCS events
*/
typedef enum {
	SII_DRV_TX_EVENT__SCDCS_READ_REQST              = 0x00000001, //!< Scrambling status changed in SCDCS
} SiiDrvTxHdmiScdcsEvent_t;

/**
* DDC bus access error codes
*/
typedef enum {
	SII_DDC_ERROR_CODE_NO_ERROR    = 0x00, //!< Success
	SII_DDC_ERROR_CODE_TIMEOUT     = 0x01, //!< DDC bus is not granted within timeout
	SII_DDC_ERROR_CODE_NO_ACK      = 0x02, //!< No ACK from DDC device
	SII_DDC_ERROR_CODE_BUSY        = 0x03, //!< DDC bus is busy
	SII_DDC_ERROR_CODE_TX_HW       = 0x04,
	SII_DDC_ERROR_CODE_LIM_EXCEED  = 0x05
} SiiDdcComErr_t;

/**
* @brief TMDS Modes
*/
typedef enum {
	SII_TMDS_MODE__NONE,
	SII_TMDS_MODE__DVI,
	SII_TMDS_MODE__HDMI1,
	SII_TMDS_MODE__HDMI2,
	SII_TMDS_MODE__AUTO
} SiiTmdsMode_t;

/**
* @brief Horizontal and Vertical Sync Polarities
*/
typedef enum {
	SII_HV_SYNC_POL__HPVP,
	SII_HV_SYNC_POL__HPVN,
	SII_HV_SYNC_POL__HNVP,
	SII_HV_SYNC_POL__HNVN
} SiiHvSyncPol_t;

typedef enum {
	QUANTIZATION_VIDEO_LEVELS_MT, // 16 - 235
	QUANTIZATION_PC_LEVELS_MT, // 0 - 255
	QUANTIZATION_BUTT
} SiiQuantLevel_t;

/**
* @brief Info Frame IDs
*/
typedef enum {
	SII_INFO_FRAME_ID__AVI,
	SII_INFO_FRAME_ID__AUDIO,
	SII_INFO_FRAME_ID__VS,
	SII_INFO_FRAME_ID__SPD,
	SII_INFO_FRAME_ID__GBD,
	SII_INFO_FRAME_ID__MPEG,
	SII_INFO_FRAME_ID__ISRC,
	SII_INFO_FRAME_ID__ISRC2,
	SII_INFO_FRAME_ID__GCP,
	SII_INFO_FRAME_ID__ACP,
	SII_INFO_FRAME_ID__HDR
} SiiInfoFrameId_t;

/**
* @brief Audio Sampling Frequrency
*/
typedef enum {
	SII_AUDIO_FS__22_05KHZ,
	SII_AUDIO_FS__24KHZ,
	SII_AUDIO_FS__32KHZ,
	SII_AUDIO_FS__44_1KHZ,
	SII_AUDIO_FS__48KHZ,
	SII_AUDIO_FS__88_2KHZ,
	SII_AUDIO_FS__96KHZ,
	SII_AUDIO_FS__176_4KHZ,
	SII_AUDIO_FS__192KHZ,
	SII_AUDIO_FS__768KHZ,
	SII_AUDIO_FS__64KHZ,
	SII_AUDIO_FS__128KHZ
} SiiAudioFs_t;

/**
* @brief Audio Channel Layout
*/
typedef enum {
	AUDIO_FORMAT__2CH = 0x02,
	AUDIO_FORMAT__3CH,
	AUDIO_FORMAT__4CH,
	AUDIO_FORMAT__5CH,
	AUDIO_FORMAT__6CH,
	AUDIO_FORMAT__7CH,
	AUDIO_FORMAT__8CH
} SiiAudioCh_t;

/**
* @brief HDCP Status
*/
typedef enum {
	SII_DRV_HDCP_STATUS__OFF,                //!< Authentication is not enabled
	SII_DRV_HDCP_STATUS__SUCCESS_1X,         //!< Authentication succeeded for HDCP 1.X
	SII_DRV_HDCP_STATUS__SUCCESS_22,         //!< Authentication succeeded for HDCP 2.2
	SII_DRV_HDCP_STATUS__AUTHENTICATING,     //!< Authentication is in progress
	SII_DRV_HDCP_STATUS__FAILED,             //!< Authentication failed and does not re-try
} SiiDrvHdcpStatus_t;

/**
* @brief HDCP Version
*/
typedef enum {
	SII_DRV_DS_HDCP_VER__1X,                 //!< HDCP 1.4 Compliance
	SII_DRV_DS_HDCP_VER__22,                  //!< HDCP 2.2 Compliance
	SII_DRV_DS_HDCP_VER__NONE                  //!< HDCP 2.2 Compliance
} SiiDrvDsHdcpVersion_t;

/**
* @brief Downstream HDCP Failure Reason
*/
typedef enum {
	SII_DRV_HDCP_FAILURE__NONE,              //!< No failure detected so far
	SII_DRV_HDCP_FAILURE__NACK,              //!< Downstream device does not acknowledge HDCP registers; firmware continues trying
	SII_DRV_HDCP_FAILURE__DEV_EXC,           //!< Too many devices; firmware does not try until HPD Low to High transition
	SII_DRV_HDCP_FAILURE__CAS_EXC,           //!< Cascade exceeded error; firmware does not try until HPD Low to High transition
	SII_DRV_HDCP_FAILURE__V,                 //!< V verification failed; firmware continues trying
	SII_DRV_HDCP_FAILURE__TIMEOUT,           //!< Authentication timed out; firmware continues trying
	SII_DRV_HDCP_FAILURE__OTHER,             //!< Other authentication errors; firmware continues trying
} SiiDrvHdcpFailure_t;

/**
* @brief HDCP 2.2 content type
*/
typedef enum {
	SII_DRV_HDCP_CONTENT_TYPE__0,            //!< HDCP for content type 0
	SII_DRV_HDCP_CONTENT_TYPE__1,            //!< HDCP for content type 1
	SII_DRV_HDCP_CONTENT_TYPE__UNKNOWN,      //!< Unknown content type
} SiiDrvHdcpContentType_t;

/**
* @brief HDCP2x core code update status
*/
typedef enum {
	SII_DRV_HDCP2X_CUPD_CHK__DONE,		     //!< Code Update Done
	SII_DRV_HDCP2X_CUPD_CHK__FAIL,		     //!< Code Updated Failed
	SII_DRV_HDCP2X_CUPD_CHK__ERROR,	         //!< Code Udated Timedout
} SiiDrvHdcp2xCupdChkStat_t;

/**
* @brief Error type returned by BKSV/RxID reading functions
*/
typedef enum {
	SII_DRV_HDCP_KSV_LOAD__OK,               //!< Success
	SII_DRV_HDCP_KSV_LOAD__NOT_AVAILABLE,    //!< BKSV/RxID list was attempted to be read before authentication or after failure
	SII_DRV_HDCP_KSV_LOAD__BUFFER_ERROR,     //!< Buffer loading error
} SiiDrvHdcpKsvLoadError_t;

/**
* @brief Video Color formats
*/
typedef enum {
	SII_DRV_CLRSPC__PASSTHRU,

	SII_DRV_CLRSPC__YC444_601,
	SII_DRV_CLRSPC__YC422_601,
	SII_DRV_CLRSPC__YC420_601,

	SII_DRV_CLRSPC__YC444_709,
	SII_DRV_CLRSPC__YC422_709,
	SII_DRV_CLRSPC__YC420_709,

	SII_DRV_CLRSPC__XVYCC444_601,
	SII_DRV_CLRSPC__XVYCC422_601,
	SII_DRV_CLRSPC__XVYCC420_601,

	SII_DRV_CLRSPC__XVYCC444_709,
	SII_DRV_CLRSPC__XVYCC422_709,
	SII_DRV_CLRSPC__XVYCC420_709,

	SII_DRV_CLRSPC__YC444_2020,
	SII_DRV_CLRSPC__YC422_2020,
	SII_DRV_CLRSPC__YC420_2020,

	SII_DRV_CLRSPC__RGB_FULL,
	SII_DRV_CLRSPC__RGB_LIMITED
} SiiDrvClrSpc_t;

/**
* @brief Video Conversion Standard
*/
typedef enum {
	SII_DRV_CONV_STD__BT_709,
	SII_DRV_CONV_STD__BT_601,
	SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS,
	SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS
} SiiDrvConvStd_t;

/**
* @brief Video bit depth
*/
typedef enum {
	SII_DRV_BIT_DEPTH__PASSTHOUGH,
	SII_DRV_BIT_DEPTH__8_BIT,
	SII_DRV_BIT_DEPTH__10_BIT,
	SII_DRV_BIT_DEPTH__12_BIT,
	SII_DRV_BIT_DEPTH__16_BIT
} SiiDrvBitDepth_t;

/**
* @brief EDID buffer
*/
typedef struct {
	uint8_t          b[SII_EDID_MAX_LEN];
} SiiEdid_t;

/**
* @brief EDID's LipSync Info
*/
typedef struct {
	bool_t latencyPresent;
	bool_t ILatencyPresent;
	uint8_t videoLatency;
	uint8_t audioLatency;
	uint8_t IVideoLatency;
	uint8_t IAudioLatency;
} SiiLipSyncInfo_t;

/**
* @brief Info Frames
*/
typedef struct {
	SiiInfoFrameId_t ifId;
	uint8_t          b[SII_INFOFRAME_MAX_LEN];
} SiiInfoFrame_t;

/**
* @brief Channle Status
*/
typedef struct {
	uint8_t          i2s_chst0;
	uint8_t          i2s_chst1;
	uint8_t          i2s_chst2;
	uint8_t          i2s_chst3;
	uint8_t          i2s_chst4;
	uint8_t          i2s_chst5;
	uint8_t          i2s_chst6;
} SiiChannelStatus_t;

/**
* @brief Audio Format type
*/
typedef struct {
	SiiAudioFs_t   audioFs;
	uint8_t        layout1;
	uint8_t        dsd;
	uint8_t        hbrA;
	uint8_t        spdif;
	uint8_t        i2s;
	uint8_t        downSample;
} SiiAudioFormat_t;

/**
* @brief KSV data object
*/
typedef struct {
	uint8_t  d[SII_BKSV_LIST_BYTES];
} SiiDrvHdcpKsv_t;

/**
* @brief KSV data object
*/
typedef struct {
	uint16_t        length;
	uint8_t*  		pList;
	uint8_t*		pListStart;
} SiiDrvHdcpKsvList_t;

/**
* @brief Downstream device HDCP topology information
*/
typedef struct {
	uint8_t         deviceCount;		//!< Total number of attached HDCP devices.
	uint8_t         depth;				//!< Number of attached repeater levels.
	uint8_t         maxDevsExceeded;	//!< \c 1: more than supported number of devices attached.
	//!< \c 0: \c deviceCount is valid.
	uint8_t         maxCascadeExceeded; //!< \c 1: more than supported number of levels attached.
	//!< \c 0: \c depth is valid.
	uint8_t         hdcp20RepeaterDs;	//!< \c 1: there is an HDCP 2.0 compliant repeater in the topology.
	//!< \c 0: there is no HDCP 2.0 compliant repeaters in the topology.
	uint8_t         hdcp1xRepeaterDs;	//!< \c 1: there is an HDCP 1.x compliant repeater in the topology.
	//!< \c 0: there is no HDCP 1.x compliant repeaters in the topology.
} SiiDrvHdcpTopology_t;

/**
* @brief Video path Color Info configuration data
*/
typedef struct {
	SiiDrvConvStd_t inputClrConvStd;
	SiiDrvClrSpc_t inputClrSpc;
	SiiDrvBitDepth_t inputVidDcDepth;
} SiiDrvTxColorInfoCfg_t;

/**
* @brief Peer's Manufacturer Status
*/
typedef struct {
	uint8_t oui_3;
	uint8_t oui_2;
	uint8_t oui_1;
	uint8_t dev_id_str;
	uint8_t dev_id_hw_rev;
	uint8_t dev_id_sw_major_rev;
	uint8_t dev_id_sw_minor_rev;
} SiiDrvTxScdcManufacturerStatus_t;

/**
* @brief Peer's SCDC reg Status
*/
typedef struct {
	uint8_t sink_ver;
	uint8_t source_ver;
	uint8_t update_0;
	uint8_t update_1;
	uint8_t tmds_config;
	uint8_t tmds_status;
	uint8_t config_0;
	uint8_t status_flag_0;
	uint8_t test_config_0;
	uint16_t chnl0_error_cnt;
	uint16_t chnl1_error_cnt;
	uint16_t chnl2_error_cnt;
} SiiDrvTxScdcRegisterStatus_t;

/**
* @brief  SCDC scramble and clock Status
*/
typedef struct {
	uint8_t sink_clk_on_off;
	uint8_t source_clk_on_off;
	uint8_t sink_scramble_on_off;
	uint8_t source_scramble_on_off;
} SiiDrvTxScdcScrmbleclkStatus_t;

/**
* @brief Static constructor configuration
*/
typedef struct {
	SiiDrvCraAddr_t		baseAddr;
	SiiInst_t           instCra;
	bool_t				bHdcp2xEn;
	bool_t				bVidPathEn;
	bool_t				bCpiEn;
	bool_t				bScdcEn;
	bool_t				bHdcpEn;
	SiiInst_t			scdcInst;
} SiiDrvTxConfig_t;

typedef struct {
	bool_t   b3DOsdDisparity;        //!<
	bool_t   bDualView;              //!<
	bool_t   bIndependentView;       //!<
	bool_t   bLTE340MscsScramble;    //!<
	bool_t   bReadReqCapable;        //!<
	bool_t   bScdcPresent;           //!<
	bool_t   bDc30bit420;            //!<
	bool_t   bDc36bit420;            //!<
	bool_t   bDc48bit420;            //!<
	uint32_t vclk_mb;                //!< video clock supported by sink in MB
	bool_t   bUHD_VIC;
} SiiDrvTxScdcSinKCaps_t;

/***** call-back functions ***************************************************/

/*****************************************************************************/
/**
* @brief Tx Driver notification call-back register function
* @note  This function is called by application to register Tx driver
*        notification call-back function
*
* @param[in]  inst        Handle to instance
* @param[in]  cbFunc      Pointer to call back function
*
*****************************************************************************/
void SiiDrvTxRegisterCallBack(SiiInst_t instTx, TxcbFunc cbFunc);

/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief Tx driver constructor
*
* @param[in]  pNameStr   Name of instance
* @param[in]  pConfig    Static configuration parameters
*
* @retval                Handle to instance
*
*****************************************************************************/
SiiInst_t SiiDrvTxCreate(char *pNameStr, SiiDrvTxConfig_t *pConfig);
SiiInst_t SiiDrvTxCreate_uboot2main(char *pNameStr, SiiDrvTxConfig_t *pConfig);

/*****************************************************************************/
/**
* @brief Tx driver destructor
*
* @param[in]  inst       Handle to instance
*
*****************************************************************************/
void SiiDrvTxDelete(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief Downstream Edid interrogation.
*
* @param[in]  inst       Handle to instance
* @param[out] pEdid      256 byte of EDID data
*
*****************************************************************************/
void SiiDrvTxEdidGet(SiiInst_t inst, SiiEdid_t* pEdid);
void SiiDrvTxEdidParseGet(SiiInst_t inst, SiiLibEdidPar_t* pEdid);

/*****************************************************************************/
/**
* @brief Downstream Edid's LipSync interrogation.
*
* @param[in]  inst			Handle to instance
* @param[out] lipSync		Edid'd Lipsync Info
*
*****************************************************************************/
void SiiDrvTxLipSyncInfoGet(SiiInst_t inst, SiiLipSyncInfo_t* lipSync);

/*****************************************************************************/
/**
* @brief Hot-plug status.
*
* @param[in]  inst       Handle to instance
*
* @retval     #true      Hot-Plug is active
* @retval     #false     Hot-Plug is not active
*
*****************************************************************************/
bool_t SiiDrvTxHotPlugStatusGet(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief RSen (TMDS input impedance) status.
*
* @param[in]  inst       Handle to instance
*
* @retval     #true      Activate impedance.
* @retval     #false     Enable high impedance.
*
*****************************************************************************/
bool_t SiiDrvTxRsenStatusGet(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief TMDS mode control.
*
* @param[in]  inst       Handle to instance
* @param[in]  tmdsMode   Can be chanaged to the following TMDS modes:
*                        - @ref SII_TMDS_MODE__OFF      Turn off TMDS signal off.
*                        - @ref SII_TMDS_MODE__AUTO     Enable TMDS output signal. Mode is automatically selected based on downstream EDID.
*                        - @ref SII_TMDS_MODE__DVI      Enable DVI compliant TMDS signal.
*                        - @ref SII_TMDS_MODE__HDMI1    Enable HDMI1 compliant TMDS signal.
*                        - @ref SII_TMDS_MODE__HDMI2    Enable HDMI2 compliant TMDS signal.
*
*****************************************************************************/
void SiiDrvTxTmdsModeSet(SiiInst_t inst, SiiTmdsMode_t tmdsMode);

/*****************************************************************************/
/**
* @brief Current used TMDS mode status.
*
* @param[in]  inst      Handle to instance
*
* @retval               #SII_TMDS_MODE__OFF      TMDS output signal is turned off.
* @retval               #SII_TMDS_MODE__DVI      TMDS output signal is DVI compliant.
* @retval               #SII_TMDS_MODE__HDMI1    TMDS output signal is HDMI1 compliant.
* @retval               #SII_TMDS_MODE__HDMI2    TMDS output signal is HDMI2 compliant.
*
*****************************************************************************/
SiiTmdsMode_t SiiDrvTxTmdsModeStatusGet(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief CEC Physical Address.
*
* @param[in]  inst       Handle to instance
*
* @retval                CEC Physical Address
*
*****************************************************************************/
uint16_t SiiDrvTxCecPhysicalAddrGet(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief AV-Mute control.
*
* @param[in]  inst    Handle to instance
* @param[in]  onOff   Boolean to control AV-Mute request for downstream device
*                        - @ref true   Requests downstream device to mute audio/video.
*                        - @ref false  Requests downstream device to unmute audio/video.
*
*****************************************************************************/
void SiiDrvTxAvMuteSet(SiiInst_t inst, bool_t onOff);

/*****************************************************************************/
/**
* @brief Info-frame control.
*
* @param[in]  inst        Handle to instance
* @param[in]  ifId        Info-Frame/Packet type
* @param[in]  pInfoFrame  Info-Frame/Packet content
*
*****************************************************************************/
void SiiDrvTxInfoframeSet(SiiInst_t inst, const SiiInfoFrame_t *pInfoFrame);

/*****************************************************************************/
/**
* @brief Info-frame enable/disable control.
*
* @param[in]  inst        Handle to instance
* @param[in]  ifId        Info-Frame/Packet type
* @param[in]  onOff       Boolean to control enable/disable info-frames/packets
*                         - @ref true       Enable Info-frame/packet transmission.
*                         - @ref false      Disable Info-frame/packet transmission.
*
*****************************************************************************/
void SiiDrvTxInfoframeOnOffSet(SiiInst_t inst, SiiInfoFrameId_t ifId, bool_t onOff);
/*****************************************************************************/
/**
* @brief Info-frame enable/disable control.
*
* @param[in]  inst        Handle to instance
* @retval     uint8_t *   All infoframe on/off data array
*
*****************************************************************************/
void SiiDrvTxInfoframeOnOffGet(SiiInst_t inst, uint8_t *OutD);

/*****************************************************************************/
/**
* @brief Audio channel-Status header control.
*
* @param[in]  inst            Handle to instance
* @param[in]  pChannelStatus  Channel-status header content
*
*****************************************************************************/
void SiiDrvTxChannelStatusSet(SiiInst_t inst, const SiiChannelStatus_t *pChannelStatus);

/*****************************************************************************/
/**
* @brief Audio format set.
*
* @param[in]  inst            Handle to instance
* @param[in]  pAudioFormat    Audio format data structure
*
*****************************************************************************/
void SiiDrvTxAudioFormatSet(SiiInst_t inst, const SiiAudioFormat_t *pAudioFormat);
/*****************************************************************************/
/**
* @brief Audio format get.
*
* @param[in]  inst              Handle to instance
*
* @retval     SiiAudioFormat_t  Audio Format Info
*
*****************************************************************************/
void SiiDrvTxAudioFormatStatusGet(SiiInst_t inst, SiiAudioFormat_t *audioFormat);

/*****************************************************************************/
/**
* @brief Content protection control.
*
* @param[in]  inst        Handle to instance
* @param[in]  onOff       Boolean to enable/disable content protection
*                         - @ref true       Enable audio/video content protection.
*                         - @ref false      Disable audio/video content protection.
*
* @retval     #true       Outgoing audio/video signal in protected.
* @retval     #false      Outgoing audio/video signal is not protected.
*
*****************************************************************************/
void SiiDrvTxHdcpProtectionSet(SiiInst_t inst, bool_t onOff);

/*****************************************************************************/
/**
* @brief HDCP status.
*
* @param[in]  inst            Handle to instance
* @param[out] pHdcpStatus     Audio format parameters.
*                             - @ref SII_DRV_HDCP_STATUS__OFF              Authentication is not enabled
*                             - @ref SII_DRV_HDCP_STATUS__SUCCESS_1X       Authentication succeeded for HDCP 1.X
*                             - @ref SII_DRV_HDCP_STATUS__SUCCESS_22       Authentication succeeded for HDCP 2.2
*                             - @ref SII_DRV_HDCP_STATUS__AUTHENTICATING   Authentication is in progress
*                             - @ref SII_DRV_HDCP_STATUS__FAILED           Authentication failed and does not re-try
*
*****************************************************************************/
void SiiDrvTxHdcpStateStatusGet(SiiInst_t inst, SiiDrvHdcpStatus_t *pHdcpStatus);
//void SiiDrvTxHdcpFailureReasonStatusGet(SiiInst_t inst, SiiDrvHdcpFailureStatus_t *pFailure);	//TODO : Implement Later

/*****************************************************************************/
/**
* @brief Interrogates downstream BKSV list from downstream device
*        (for downstream repeater application only)
*
* @param[in]  inst            Handle to instance
* @param[out  pBksvList       List of downstream BKSVs.
*
*****************************************************************************/
void SiiDrvTxHdcpKsvListGet(SiiInst_t inst, SiiDrvHdcpKsvList_t *pBksvList);

/*****************************************************************************/
/**
* @brief User acknowledge for KSV list approval
*
* @param[in]  inst            Handle to instance
* @param[out  bApproved       KSV list approval.
*                             - @ref TRUE      List is approved
*                             - @ref FALSE     List is rovocated
*
*****************************************************************************/
void SiiDrvTxHdcpKsvListApprovalSet(SiiInst_t inst, bool_t bApproved);

/*****************************************************************************/
/**
* @brief HDCP topology configuration status.
*
* @param[in]  inst            Handle to instance
* @param[out] pTopology       Topology data structure.
*
*****************************************************************************/
void SiiDrvTxHdcpTopologyGet(SiiInst_t inst,  SiiDrvHdcpTopology_t *pTopology);

/*****************************************************************************/
/**
* @brief HDCP2.2 Content type configuration status.
*
* @param[in]  inst            Handle to instance
* @param[in] pContentType    Content indicator.
*                             - @ref SII_DRV_HDCP_CONTENT_TYPE__0
*                             - @ref SII_DRV_HDCP_CONTENT_TYPE__1
*
*****************************************************************************/
void SiiDrvTxHdcp2ContentTypeSet(SiiInst_t inst, SiiDrvHdcpContentType_t *pContentType);
//void SiiDrvTxHdcp2ContentTypeGet(SiiInst_t inst, SiiDrvHdcpContentType_t *pContentType);	//TODO : Implement Later

/*****************************************************************************/
/**
* @brief HDCP2x Code Update Status.
*
* @param[in]  inst            Handle to instance
* @param[in]  pHdcp2xCupdStat        hdcp2x Code update Status
*                             - @ref SII_DRV_HDCP2X_CUPD_CHK__ERROR
*                             - @ref SII_DRV_HDCP2X_CUPD_CHK__FAIL
*                             - @ref SII_DRV_HDCP2X_CUPD_CHK__DONE
*
*****************************************************************************/
void SiiDrvTxHdcp2xCupdStatusGet(SiiInst_t inst, SiiDrvHdcp2xCupdChkStat_t *pHdcp2xCupdStat);

void SiiDrvTxHdcpCapGet(SiiInst_t inst, SiiDrvDsHdcpVersion_t *pHdcpCap);

/*****************************************************************************/
/**
* @brief H/V-Sync Polarity Set.
*
* @param[in]  inst		Handle to instance
* @param[in]  hvSyncPol	Can be changed to the following polarities:
*
* @retval     #SII_HV_SYNC_POL__HPVP   Positive hor. sync / Positive ver. sync.
* @retval     #SII_HV_SYNC_POL__HPVN   Positive hor. sync / Negative ver. sync.
* @retval     #SII_HV_SYNC_POL__HNVP   Negative hor. sync / Positive ver. sync.
* @retval     #SII_HV_SYNC_POL__HNVN   Negative hor. sync / Negative ver. sync.
*
*****************************************************************************/
void SiiDrvTxHvSyncPolaritySet(SiiInst_t inst, SiiHvSyncPol_t *hvSyncPol);

/*****************************************************************************/
/**
* @brief Output video bit depth control.
* @note  Enables TMDS deep color mode if 10 or 12 bit is requested for both 4:2:0 and 4:4:4 signals.
*
* @param[in]  inst            Handle to instance
* @param[in]  bitDepth        Bit Depth of tmds output signal
*                             - @ref SII_DRV_BIT_DEPTH__8_BIT
*                             - @ref SII_DRV_BIT_DEPTH__10_BIT
*                             - @ref SII_DRV_BIT_DEPTH__12_BIT
*
*****************************************************************************/
void SiiDrvTxOutputBitDepthSet(SiiInst_t inst, SiiDrvBitDepth_t bitDepth);

/*****************************************************************************/
/**
* @brief Configure Video Path input Color Space, Bit depth and Color conversion standard.
*
* @param[in]  inst    Handle to instance
* @param[in]  pClrInfo Video path Input Color Information data.
*
*****************************************************************************/
void SiiDrvTxColorInfoConfig(SiiInst_t inst, SiiDrvTxColorInfoCfg_t *clrInfo);
void SiiDrvTxInputQuantSet(SiiInst_t inst, SiiQuantLevel_t *Quant);

/*****************************************************************************/
/**
* @brief Output color space format.
* @note  Input color space format is detemined by AVI info-frame.
* @note  Use @ref SII_DRV_TX_CLRSPC__PASSTHRU to remain output color space unchanged.
*
* @param[in]  inst     Handle to instance
* @param[in]  clrSpc   Color Space format.
*                      - @ref SII_DRV_TX_CLRSPC__PASSTHRU
*                      - @ref SII_DRV_TX_CLRSPC__YC444_601
*                      - @ref SII_DRV_TX_CLRSPC__YC422_601
*                      - @ref SII_DRV_TX_CLRSPC__YC420_601
*                      - @ref SII_DRV_TX_CLRSPC__YC444_709
*                      - @ref SII_DRV_TX_CLRSPC__YC422_709
*                      - @ref SII_DRV_TX_CLRSPC__YC420_709
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC444_601
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC422_601
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC420_601
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC444_709
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC422_709
*                      - @ref SII_DRV_TX_CLRSPC__XVYCC420_709
*                      - @ref SII_DRV_TX_CLRSPC__YC444_2020
*                      - @ref SII_DRV_TX_CLRSPC__YC422_2020
*                      - @ref SII_DRV_TX_CLRSPC__YC420_2020
*                      - @ref SII_DRV_TX_CLRSPC__RGB_FULL
*                      - @ref SII_DRV_TX_CLRSPC__RGB_LIMITED
*
*****************************************************************************/
void SiiDrvTxOutputColorSpaceSet(SiiInst_t inst, SiiDrvClrSpc_t *clrSpc);

/*****************************************************************************/
/**
* @brief reads and prints the Manufacturer SCDC registers
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcManufacturerRegisterstatus(SiiInst_t inst, SiiDrvTxScdcManufacturerStatus_t *manfStatus);

/*****************************************************************************/
/**
* @brief reads and prints the peer SCDC registers
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcPeerRegisterstatus(SiiInst_t inst, SiiDrvTxScdcRegisterStatus_t *peerStatus);

/*****************************************************************************/
/**
* @brief reads scramble and clock status
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcScrambleAndClockstatus(SiiInst_t inst, SiiDrvTxScdcScrmbleclkStatus_t *scrambleclkStatus);

/*****************************************************************************/
/**
* @brief Reset peer update status registers
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcResetUpdateRegisters(SiiInst_t inst);

/*****************************************************************************/
/**
* @brief enables SCDC auto polling
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcAutopollEnable(SiiInst_t inst, bool_t value);

/*****************************************************************************/
/**
* @brief enables SCDC Test Read request test
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcReadRequestTest(SiiInst_t inst, bool_t enable);

/*****************************************************************************/
/**
* @brief enables SCDC scrambling
*
* @param[in]  inst       Handle to instance
*
* @retval                None
*
*****************************************************************************/
void SiiDrvTxScdcScrambleenable(SiiInst_t inst, SiiDrvTxScdcSinKCaps_t *scramble_enable);
void SiiDrvTxScdcScrambleDisable(SiiInst_t inst, SiiDrvTxScdcSinKCaps_t *scramble_enable);
void SiiDrvTxScdcSrcCrSet(SiiInst_t inst, bool_t cr);
void SiiDrvTxScdcSrcStreamTypeSet(SiiInst_t inst, uint8_t st);
void SiiDrvTxHdcpReauth(SiiInst_t inst, bool_t onOff);
mt_u32 SiiDrvTxHdcpSetProtectionDone(SiiInst_t inst);
bool_t SiiDrvTxHdcpProtectionGet(SiiInst_t inst);
void SiiDrvTxTmdsMute(SiiInst_t inst, bool_t mute);
void SiiDrvTxOutputColorimetrySet(SiiInst_t inst, SiiDrvConvStd_t *std);
void SiiDrvTxOutputCscMtxSet(SiiInst_t inst, void *mtx);
void SiiDrvTxHdcpMute(SiiInst_t inst, bool_t onOff);
void SiiDrvTxReadEDIDFromSink(SiiInst_t inst, uint8_t *OutD);
void SiiDrvTxHdcpVerGet(SiiInst_t inst, SiiDrvDsHdcpVersion_t *pHdcpCap);
void SiiDrvTx_Get_Plug_Status(SiiInst_t inst, uint8_t *status);

#endif // __SI_DRV_TX_API_H__
