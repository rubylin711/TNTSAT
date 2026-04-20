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
* @file si_lib_edid_api.h
*
* @brief EDID parser
*
*****************************************************************************/
#ifndef __SI_LIB_EDID_API_H__
#define __SI_LIB_EDID_API_H__

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "si_lib_video_api.h"
#if ( __HDMI_OS_KERNEL__ )
	#include "mt_unf_video.h"
#endif

/***** public macro definitions **********************************************/
#define SII_LIB_EDID__SVD_MAX                 50
#define SII_LIB_EDID__SPD_MAX                  1
#define SII_LIB_EDID__SAD_MAX                 50
#define SII_LIB_EDID__DTD_MAX                  1
#define SII_LIB_EDID__3DDB_MAX                10
#define SII_LIB_EDID__YUV420CMDB_MAX                 4
#define SII_LIB_EDID__DMDB_MAX                 10
#define SII_LIB_EDID__DMDB_OPT_FLG_MAX                 2

#define SII_LIB_EDID_PAR__VERSION              1
#define SII_LIB_EDID_PAR__MONITOR_NAME         1
#define SII_LIB_EDID_PAR__DISPLAY_AR           0
#define SII_LIB_EDID_PAR__CEC_ADDR             1
#define SII_LIB_EDID_PAR__VDB                  1
#define SII_LIB_EDID_PAR__SADB                 1
#define SII_LIB_EDID_PAR__ADB                  1
#define SII_LIB_EDID_PAR__DTDB                 1
#define SII_LIB_EDID_PAR__AUD_FRM              1
#define SII_LIB_EDID_PAR__MAX_TMDS             1
#define SII_LIB_EDID_PAR__3D                   1
#define SII_LIB_EDID_PAR__DEEP_CLR             1
#define SII_LIB_EDID_PAR__CLR_SPACE            1
#define SII_LIB_EDID_PAR__IEEE		           1
#define SII_LIB_EDID_PAR__SCDC   	           1
#define SII_LIB_EDID_PAR__LIPSYNC	           1

/***** public type definitions ***********************************************/

typedef uint8_t  SiiLibEdidRaw_t;

/* EDID parsing error codes */
typedef enum {
	SII_LIB_EDID_ERR_CODE__NO_ERROR,
	SII_LIB_EDID_ERR_CODE__BAD_HEADER,
	SII_LIB_EDID_ERR_CODE__CHECKSUM,
	SII_LIB_EDID_ERR_CODE__NOEXTENSIONS,
	SII_LIB_EDID_ERR_CODE__CEA_TAG,
	SII_LIB_EDID_ERR_CODE__NO_861B,
	SII_LIB_EDID_ERR_CODE__UNKNOWN_TAG,
	SII_LIB_EDID_ERR_CODE__VENDERBLOCK
} SiiLibEdidErrCode_t;

typedef struct {
	uint8_t               sub[4];
} SiiLibEdidCecAddr_t;

typedef struct {
	uint16_t              cecAddrPtr;
	SiiLibEdidCecAddr_t   cecAddr;
} SiiLibEdidVsdb_t;

typedef struct {
	uint8_t               p[1];
} SiiLibEdidSvd_t;

typedef struct {
	uint8_t               size;
	SiiLibEdidSvd_t       svd[SII_LIB_EDID__SVD_MAX];
} SiiLibEdidVdb_t;

typedef struct {
	uint8_t               p[3];
} SiiLibEdidSpD_t;

typedef struct {
	uint8_t               size;
	SiiLibEdidSpD_t       SpD[SII_LIB_EDID__SPD_MAX];
} SiiLibEdidSadb_t;

typedef struct {
	uint8_t               p[3];
} SiiLibEdidSad_t;

typedef struct {
	uint8_t               size;
	SiiLibEdidSad_t       sad[SII_LIB_EDID__SAD_MAX];
} SiiLibEdidAdb_t;

typedef struct {
	uint8_t               size;
	SiiLibVideoTiming_t   vidTim[SII_LIB_EDID__DTD_MAX];
} SiiLibEdidDtdb_t;

typedef struct EDID_DB3D_s {
	uint8_t               size;
	uint8_t               b[SII_LIB_EDID__3DDB_MAX];
} SiiLibEdidDb3d_t;

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
} SiiLibScdcSinKCaps_t;

/* CTA-861-G Table 70 */
typedef struct {
	bool_t   xvYCC601;        //!<
	bool_t   xvYCC709;              //!<
	bool_t   sYCC601;       //!<
	bool_t   AdobeYCC601;    //!<
	bool_t   AdobeRGB;        //!<
	bool_t   BT2020cYCC;           //!<
	bool_t   BT2020YCC;            //!<
	bool_t   BT2020RGB;            //!<
	bool_t   DCI_P3;            //!<
} SiiLibColorimetry_t;

/* HDR Static Metadata Data Block CTA-861-G Table 85/86 */
typedef struct {
	bool_t   Gamma_SDR;        //!<
	bool_t   Gamma_HDR;              //!<
	bool_t   Smpte2084;       //!<
	bool_t   HLG;    //!<
	bool_t   SmType;    //!<
	uint32_t   MaxLum;    //!<
	uint32_t   MaxAvgLum;    //!<
	uint32_t   MinLum;    //!<
} SiiLibHDRInfo_t;

/**
* @brief EDID's HDR DMDB Info
*/
typedef struct {
	uint16_t DMType;
	uint8_t Flags;
	uint8_t OptFields[SII_LIB_EDID__DMDB_OPT_FLG_MAX];
} SiiLibEdidDmTypeInfo_t;

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
} SiiLibEdidLipSyncInfo_t;

typedef struct SiiLibEdid3DInfoVicOrder_s {
	uint8_t	vic_order;
	uint8_t	struc_3d;
} SiiLibEdid3DInfoVicOrder_t;

/**
* @brief EDID's 3D Info
*/
typedef struct {
	bool_t supported_3d;
	uint8_t supported_3d_multi;
	uint8_t Image_Size;
	uint16_t Struc_all_3d;
	uint16_t mask_3d;
	uint8_t Edid3DVicOrderCnt;
	SiiLibEdid3DInfoVicOrder_t vic_ord[16];
	bool_t supported_4k2k_30;
	bool_t supported_4k2k_25;
	bool_t supported_4k2k_24;
	bool_t supported_4k2k_smpte_24;
	bool_t supported_3d_frmpack;
	bool_t supported_3d_tb;
	bool_t supported_3d_sbys;
} SiiLibEdid3DInfo_t;

#define HDMI_AUDIO_FORMAT_MAX_NUM_EDID 16
/**
* @brief Audio Info
*/
typedef struct {
	uint8_t audformat_cnt;
	uint8_t audformat[HDMI_AUDIO_FORMAT_MAX_NUM_EDID];
	uint8_t audchannel[HDMI_AUDIO_FORMAT_MAX_NUM_EDID];
	uint8_t audfs[HDMI_AUDIO_FORMAT_MAX_NUM_EDID];
	uint8_t audlen[HDMI_AUDIO_FORMAT_MAX_NUM_EDID];
	uint16_t speakerformat;
} SiiLibAudInfo_t;

/**
* @brief Audio Info
*/
typedef struct {
	uint8_t HdPhonePb;
	uint8_t HeightSpeaker;
	uint8_t SurroundSpeaker;
	uint8_t CenterSpeaker;
	uint8_t DdVsadbVer;
	uint8_t SinkCap;
} SiiLibDdVsAudInfo_t;

typedef struct {
	uint8_t man_name[4];
	uint32_t product_code;
	uint32_t serialnum;
	uint32_t week;
	uint32_t year;
} SiiLibManInfo_t;

/* HDR Static Metadata Data Block CTA-861-G Table 85/86 */
typedef struct {
	bool_t   vivid;        //!<
	uint8_t   ststem_start_code;              //!<
	bool_t   version_code;       //!<
	uint32_t   MaxLum;    //!<
	uint32_t   MinLum;    //!<
	bool_t   monitor_mode_support;        //!<
	bool_t   rx_mode_support;        //!<
} SiiLibVividInfo_t;

typedef struct {
	#if SII_LIB_EDID_PAR__VERSION
	uint16_t              version;
	#endif

	SiiLibManInfo_t			maninfo;
	#if SII_LIB_EDID_PAR__MONITOR_NAME
	char                  pMonitorNameStr[14];
	#endif

	#if SII_LIB_EDID_PAR__DISPLAY_AR
	VidFrm_AspRat_t       fDisplayAR;
	#endif

	#if SII_LIB_EDID_PAR__VDB
	SiiLibEdidVdb_t       vdb;
	#endif

	#if SII_LIB_EDID_PAR__SADB
	SiiLibEdidSadb_t      sadb;
	#endif

	#if SII_LIB_EDID_PAR__ADB
	SiiLibEdidAdb_t       adb;
	#endif

	#if SII_LIB_EDID_PAR__DTDB
	SiiLibEdidDtdb_t      dtdb;
	#endif

	#if SII_LIB_EDID_PAR__MAX_TMDS
	uint32_t              maxTmds;
	#endif

	#if SII_LIB_EDID_PAR__3D
	SiiLibEdidDb3d_t      db3d;
	#endif

	#if SII_LIB_EDID_PAR__CEC_ADDR
	uint16_t              cecAddrPtr;
	SiiLibEdidCecAddr_t   cecAddr;
	#endif

	#if SII_LIB_EDID_PAR__AUD_FRM
	bit_fld_t             bBasicAudio;
	bit_fld_t             bSupportsAI;
	#endif

	#if SII_LIB_EDID_PAR__DEEP_CLR
	bit_fld_t             b444DC10;
	bit_fld_t             b444DC12;
	bit_fld_t             b444DC16;
	#endif

	#if SII_LIB_EDID_PAR__CLR_SPACE
	bit_fld_t             bY444;
	#endif

	#if SII_LIB_EDID_PAR__SCDC
	SiiLibScdcSinKCaps_t  scdc;
	#endif

	#if SII_LIB_EDID_PAR__IEEE
	int						ieee_id;
	#endif
	#if SII_LIB_EDID_PAR__LIPSYNC
	SiiLibEdidLipSyncInfo_t		lipSync;
	#endif
	SiiLibColorimetry_t                 colorimetry;
	SiiLibHDRInfo_t                     hdrInfo;
	uint8_t					yuv420CMDB[SII_LIB_EDID__YUV420CMDB_MAX];
	SiiLibEdidVdb_t			yuv420Vdb;
	int  bHfScdb;
	bit_fld_t             			bUnderScan;
	bit_fld_t             			bAudio;
	bit_fld_t             			Yuv444;
	bit_fld_t             			Yuv422;
	SiiLibEdidDmTypeInfo_t		DMDBInfo[SII_LIB_EDID__DMDB_MAX];
	bit_fld_t					hdr10p_vsif;
	uint32_t					hdr10p_emp;
	SiiLibVividInfo_t					hdr10p_vivid;
	SiiLibEdid3DInfo_t	d3Info;
	SiiLibAudInfo_t		audInfo;
	uint8_t				vsifs;
	SiiLibDdVsAudInfo_t	DdVsadb;
} SiiLibEdidPar_t;

/***** call-back functions ***************************************************/

/***** public functions ******************************************************/
uint8_t SiiLibEdidExtentionsGet(const SiiLibEdidRaw_t* piEdidRaw);
SiiLibEdidErrCode_t SiiLibEdidParse(SiiLibEdidPar_t* poEdidPar, const SiiLibEdidRaw_t* piEdidRaw);
void SiiLibEdidPrintEn( uint8_t en);
void sParseEdidErrInit(SiiLibEdidPar_t* poEdidPar);
#if ( __HDMI_OS_KERNEL__ )
	uint8_t Transfer_VideoTimingFromat_to_VModeTablesIndex(MT_UNF_ENC_FMT_E unfFmt);
#endif

#endif // __SI_LIB_EDID_API_H__

/***** end of file ***********************************************************/
