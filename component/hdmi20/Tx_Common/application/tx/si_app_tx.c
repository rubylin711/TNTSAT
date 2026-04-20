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
* @file
*
* @brief Tx API
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/
#include "mt_hdmi20_cfg.h"
#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#include "conio.h"
#endif
#include "si_datatypes.h"
#include "si_drv_tx_api.h"
#include "si_drv_tpg_api.h"
#include "si_drv_cra_api.h"
#include "si_lib_seq_api.h"
#include "si_drv_vtg_api.h"
#include "si_lib_video_api.h"
#include "si_drv_pll_vo_api.h"
#include "si_lib_log_api.h"
#include "si_tx_api_testing_api.h"
#include "si_drv_pebbles_api.h"
#include "si_app_cec.h"
#include "si_drv_cpi_api.h"
#include "si_cec_enums.h"
#include "si_drv_tx_api.h"

SII_LIB_OBJ_MODULE_DEF_NOCREATE(app_tx);

/***** Application Data ******************************************************/
const char lSignonMsg [] = "Tx IP Firmware v";
const char lCopyrightMsg [] = "Copyright Deco Inc, 2013-2014";
int8_t *buildVersion = (int8_t *)"0.94.02";

/***** local macro definitions ***********************************************/
#define SII_TPG_BASE_ADDR		0x3E40

/***** local type definitions ************************************************/
typedef struct {
	bool_t		bHdcp2xEn;
	bool_t		bVideopathEn;
	bool_t		bCecEn;
	bool_t		bApiTestingEn;
	bool_t		bSiIMonHandlingEn;
	bool_t		bTpgEn;
	bool_t		bProgramPebbels;
	bool_t      bScDcEn;
	bool_t      bHdcpEn;
	uint16_t	baseAddr;

} SiiTxConfig_t;

typedef enum {
	SII_TPG_FORMAT_480P_60,
	SII_TPG_FORMAT_720P_50,
	SII_TPG_FORMAT_720P_60,
	SII_TPG_FORMAT_2610P_50,
	SII_TPG_FORMAT_2610P_60
} SiiTpgTiming_t;

/***** local prototypes ******************************************************/
TestModulesInstInfo_t testModInstinfo = {0};
static SiiInst_t sInstSiiMonHandler = SII_INST_NULL;
static SiiInst_t sInstTx			= SII_INST_NULL;
static SiiInst_t sInstCra			= SII_INST_NULL;

static SiiInst_t sInstTpg = SII_INST_NULL;
static uint8_t   tpgEnable  = 0;
static uint8_t   tpgPattern = 0;
static uint8_t   tpgFormat  = 0;

static bool_t craDebugAssertEn = false;
static SiiTxConfig_t txCfg;

//static SiiDrvRxConfig_t   sRxConfig  = {BASE_ADDRESS_RX0, BASE_ADDRESS_RX1, SII_DRV_RX_PRIM_LINK__RX0};
static SiiDrvTxConfig_t   sTxConfig = {0};
static SiiDrvTpgConfig_t  sTpgConfig = {0};

//#if SII_INC_CEC
#if 0
uint8_t  cecSourceLaList[] = { CEC_LOGADDR_PLAYBACK1, CEC_LOGADDR_PLAYBACK2, CEC_LOGADDR_PLAYBACK3,
							   CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC,
							   CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC
							 };
#else
uint8_t  cecSourceLaList[] = { CEC_LOGADDR_TUNER1, CEC_LOGADDR_TUNER2, CEC_LOGADDR_TUNER3,
							   CEC_LOGADDR_TUNER4, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC,
							   CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC
							 };
#endif
//#endif
/***** local functions *******************************************************/
bool_t SiiHostInfoframeOnOffGet(SiiInfoFrameId_t ifId);
void SiiHostInfoframeGet(SiiInst_t inst, SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame);
static void sUpdateEdid(void);
void SiiTxTpgEnable(SiiInst_t inst, uint8_t enable);
void SiiTxTpgFormatSet(SiiInst_t inst, uint8_t format);
void SiiTxTpgPatternSet(SiiInst_t inst, uint8_t pattern);
void SiiDrvCraFailureCallback(void);
void SiiTxCallBack(SiiDrvTxEvent_t eventFlags);
bool_t SiiTxCreate(SiiPlatformInterface_t *pInterfaceInfo);
void SiiTxDelete(void);
bool_t SiiTxReCreate(SiiPlatformInterface_t *pInterfaceInfo);
bool_t getScdcEnable(void);
SiiInst_t getTxCraInstance(void);
SiiInst_t getTxInstance(void);

bool_t SiiHostInfoframeOnOffGet(SiiInfoFrameId_t ifId)
{
	bool_t bIfOn = false;

	switch (ifId) {
		case SII_INFO_FRAME_ID__AVI    :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__AUDIO  :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__VS     :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__SPD    :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__GBD    :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__MPEG   :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__ISRC   :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__ISRC2  :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__ACP    :
			bIfOn = false;
			break;
		case SII_INFO_FRAME_ID__HDR    :
			bIfOn = false;
			break;
		default :
			break;
	}
	return bIfOn;
}

void SiiHostInfoframeGet(SiiInst_t inst, SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame)
{
	pInfoFrame->ifId = ifId;

	SII_MEMSET(pInfoFrame->b, 0x00, SII_INFOFRAME_MAX_LEN);

	switch (ifId) {
		case SII_INFO_FRAME_ID__AVI    :
			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__AVI, pInfoFrame);
			break;

		case SII_INFO_FRAME_ID__AUDIO  :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__AUDIO, pInfoFrame);
			/*
			//pInfoFrame = &infoFrame;
			pInfoFrame->ifId = ifId;
			pInfoFrame->b[0] = 0x02;	// audio infoframe
			pInfoFrame->b[1] = 0x84;
			pInfoFrame->b[2] = 0x01;
			pInfoFrame->b[3] = 0x0a;
			pInfoFrame->b[4] = 0x70;
			pInfoFrame->b[5] = 0x01;	// 2-ch
			pInfoFrame->b[6] = 0x00;
			pInfoFrame->b[7] = 0x00;
			pInfoFrame->b[8] = 0x00;
			pInfoFrame->b[9] = 0x00;
			pInfoFrame->b[10] = 0x00;
			pInfoFrame->b[11] = 0x00;
			pInfoFrame->b[12] = 0x00;
			pInfoFrame->b[13] = 0x00;
			pInfoFrame->b[14] = 0x00;
			pInfoFrame->b[15] = 0xc0;	// enable & repeat
			*/
			break;
		case SII_INFO_FRAME_ID__VS     :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__VS, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__SPD    :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__SPD, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__GBD    :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__GBD, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__MPEG   :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__MPEG, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__ISRC   :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__ISRC, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__ISRC2  :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__ISRC2, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__ACP    :

			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__ACP, pInfoFrame);
			break;
		case SII_INFO_FRAME_ID__HDR    :
			SiiPebblesInfoframeGet(inst, SII_INFO_FRAME_ID__HDR, pInfoFrame);
			break;
		default :
			break;
	}
}

#if 0
static void sInfoFrameRepeat(SiiInfoFrameId_t ifId)
{
	bool_t ifOnOff = SiiHostInfoframeOnOffGet(ifId);

	if (ifOnOff) {
		SiiInfoFrame_t infoFrame;

		SiiHostInfoframeGet(sTxConfig.instCra, ifId, &infoFrame);
		infoFrame.ifId = ifId;
		SiiDrvTxInfoframeSet(sInstTx, &infoFrame);
	}
	SiiDrvTxInfoframeOnOffSet(sInstTx, ifId, ifOnOff);
}
#endif

static void sUpdateEdid(void)
{
	SiiEdid_t   edid;

	SiiDrvTxEdidGet(sInstTx, &edid);

	//    SiiDrvRxEdidSet(sInstRx, &edid);

}

static void sUpdateHotPlug(void)
{
	bool_t hotPlug = SiiDrvTxHotPlugStatusGet(sInstTx);

	if ( hotPlug ) {
		//set_tvsys_default();
		sUpdateEdid();
		if (txCfg.bCecEn) {
			SiiCecUpdatePhysicalAdress(SiiDrvTxCecPhysicalAddrGet(sInstTx));
			//SiiCecSetSourceActive( true );
			SiiCecUpdatePowerState((SiiCecPowerstatus_t)CEC_POWERSTATUS_ON);

			SiiCecEnumerateDevices(0, hotPlug, cecSourceLaList);
			SiiCecEnumerateDeviceLa(cecSourceLaList );
			SiiCecSetDevicePA( SiiDrvTxCecPhysicalAddrGet(sInstTx) );
		}
	} else if (txCfg.bCecEn) {
		//SiiCecSetSourceActive( false );
		SiiCecUpdatePowerState((SiiCecPowerstatus_t)CEC_POWERSTATUS_STANDBY);
		SiiCecEnumerateDevices(0, hotPlug, cecSourceLaList);
		SiiCecReset();
	}
}

static void sUpdateRepeater(void)
{
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__AVI	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__AUDIO	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__VS	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__SPD	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__GBD	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__MPEG	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ISRC	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ISRC2	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ACP	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__HDR	, true);

	//SiiDrvTxAvMuteSet(sInstTx, false); // TODO: set false after HDCP authentication success
	SiiDrvTxTmdsModeSet(sInstTx, SII_TMDS_MODE__HDMI1);
}

static void sClearInfoFrame(SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame)
{
	pInfoFrame->ifId = ifId;
	SII_MEMSET(&pInfoFrame->b[0], 0, SII_INFOFRAME_MAX_LEN);
}

static void sUpdateTpg(void)
{
	SiiInfoFrame_t infoFrame;

	sClearInfoFrame(SII_INFO_FRAME_ID__AVI, &infoFrame);
	infoFrame.b[0] = 0x82;
	infoFrame.b[1] = 0x02;
	infoFrame.b[2] = 0x0D;
	infoFrame.b[3] = 0xBF;
	infoFrame.b[4] = 0x40;
	infoFrame.b[5] = 0x6A;
	infoFrame.b[6] = 0x04;
	infoFrame.b[7] = 0x02;

	infoFrame.ifId = SII_INFO_FRAME_ID__AVI;
	SiiDrvTxInfoframeSet(sInstTx, &infoFrame);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__AVI	, true);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__AUDIO	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__VS	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__SPD	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__GBD	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__MPEG	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ISRC	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ISRC2	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__ACP	, false);
	SiiDrvTxInfoframeOnOffSet(sInstTx, SII_INFO_FRAME_ID__HDR, false);
}

static void sTpgFormat(SiiInst_t inst, SiiDrvTpgVM_t vm)
{
	SiiDrvTpgTimingSet(inst, vm);
}

static void sUdateEnablePattern(SiiInst_t inst)
{
	if (tpgEnable == 1) {
		SiiDrvTpgModeSet(inst, SII_DRV_TPG_MODE__PATTERN);
		//SiiDrvBsbConnectTpg(true);
		//SiiDrvCraWrReg16(0x0014,0x16d);       // all three outputs select Video test pattern generator
		SiiDrvTpgPatternSet(inst, (SiiDrvTpgPattern_t) tpgPattern);
		sUpdateTpg();
	} else {
		SiiDrvTpgModeSet(inst, SII_DRV_TPG_MODE__NONE);
		//SiiDrvBsbConnectTpg(false);
		//SiiDrvCraWrReg16(0x0014,0x00);       // all three outputs select normal video
		sUpdateRepeater();
	}
}

void SiiTxTpgEnable(SiiInst_t inst, uint8_t enable)
{
	tpgEnable = enable;
	sUdateEnablePattern(inst);
}

void SiiTxTpgFormatSet(SiiInst_t inst, uint8_t format)
{
	tpgFormat = format;
	sTpgFormat(inst, (SiiDrvTpgVM_t)tpgFormat);
}

void SiiTxTpgPatternSet(SiiInst_t inst, uint8_t pattern)
{
	tpgPattern = pattern;
	sUdateEnablePattern(inst);
}

static void deblank(char *str)
{
	#if (MT_SDK_COMPILE_HDMI20 == 0)
	for (; *str != '\0'; ++str) {
		if (*str != ' ') {
			*str = *str++;
		}
	}
	#else
	int i, j = 0;
	for (i = 0;; i++) {
		if (str[i] != ' ' && str[i] != '\0') {
			str[j] = str[i], j++;    //����ͬ��ֵ����j�ۼ�
		} else if (str[i] == '\0') {
			break;    //��\0������ѭ��
		}
	}
	#endif
}

static uint8_t getValIndex(char *line)
{
	char *equalIndex;
	equalIndex = strstr(line, "=");
	if (equalIndex) {
		return (uint8_t)(equalIndex - line);
	} else {
		printf("\nerror in config file line: %s\n", line);
		while ( !_kbhit() ) {
			;
		}
		exit(1);
	}
}

static void ParseConfig(SiiTxConfig_t *pTxCfg)
{
	FILE *fin;
	char line[100];
	uint8_t valIndex;
	#if 0
	printf("\nPlease make sure Tx_Config.txt is placed along side executable and press any key\n");
	while ( !_kbhit() ) {
		;
	}
	#endif
	printf("\nParsing Tx IP Configuration file\n");
	fopen_s(&fin, "Tx_Config.txt", "r");
	if (!fin) {
		printf("\n");
		printf("No Configuration File found\n");
		return;
	}

	while (fgets(line, 100, fin)) {
		deblank(line);
		if (strstr(line, "API_TESTING_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bApiTestingEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "CEC_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bCecEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "VIDPATH_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bVideopathEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "SIIMON_HANDLING_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bSiIMonHandlingEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "HDCP2X_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bHdcp2xEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "CRA_FAILURE_DEBUG_ASSERT_EN")) {
			valIndex = getValIndex(line);
			craDebugAssertEn = (char)atoi(&line[valIndex + 1]);
		} else if (strstr(line, "BASE_ADDR")) {
			valIndex = getValIndex(line);
			pTxCfg->baseAddr = (uint16_t)atoi(line + valIndex + 1);
		} else if (strstr(line, "TPG_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bTpgEn = (char)atoi(line + valIndex + 1);
		} else if (strstr(line, "PEBBELS_CONFIG_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bProgramPebbels = (char)atoi(line + valIndex + 1);
		} else if (strstr(line, "SCDC_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bScDcEn = (char)atoi(line + valIndex + 1);
		} else if (strstr(line, "HDCP_EN")) {
			valIndex = getValIndex(line);
			pTxCfg->bHdcpEn = (char)atoi(line + valIndex + 1);
		}
	}
	fclose(fin);
}

/***** call-back functions ***************************************************/
void SiiDrvCraFailureCallback(void)
{
	printf("\n\n  Cra Failure Notified");
	if (craDebugAssertEn) {
		printf("   Platform Debug Asserting, Restart the Apllication");
		SII_PLATFORM_DEBUG_ASSERT(0);
	}
	printf("\n");
}

void SiiTxCallBack(SiiDrvTxEvent_t eventFlags)
{
	SiiDrvHdcpStatus_t 		hdcpStatus;
	SiiDrvHdcpKsvList_t 	bksvList;
	if ( tpgEnable ) {
		return;
	}

	if (eventFlags & SII_DRV_TX_EVENT__HOT_PLUG_CHNG) {
		sUpdateHotPlug();
	}
	if (eventFlags & SII_DRV_TX_EVENT__RSEN_CHNG) {
		//SiiDrvRxRsenSet(sInstRx, SiiDrvTxRsenStatusGet(instTx));
	}
	if (eventFlags & SII_DRV_TX_EVENT__HDMI_STATE_CHNG) {
		;
	}
	if (eventFlags & SII_DRV_TX_EVENT__HDCP_STATE_CHNG) {
		SiiDrvTxHdcpStateStatusGet(sInstTx, &hdcpStatus);
		if ((hdcpStatus == SII_DRV_HDCP_STATUS__SUCCESS_1X) || (hdcpStatus == SII_DRV_HDCP_STATUS__SUCCESS_22)) {
			SiiDrvTxHdcpKsvListGet(sInstTx, &bksvList);
			// Compare with revocation list and approve/reject the KSV list.
			// Here approving by default
			SiiDrvTxHdcpKsvListApprovalSet(sInstTx, true);
		} else {
			SiiDrvTxHdcpKsvListApprovalSet(sInstTx, false);
		}
	}

	if (eventFlags & SII_DRV_TX_EVENT__CEC_CMD_RECEIVED) {
		SiiDrvCecInterruptHandler(SiiGetCecInst());
	}
}

//********************************************************************
bool_t SiiTxCreate(SiiPlatformInterface_t *pInterfaceInfo)
{
	SiiDrvCraConfig_t craConfig;
	SiiDrvHdcp2xCupdChkStat_t cupdChkStat;
	bool_t ret = 0;

	SII_LIB_LOG_PRINT2(("\n|-------------------------------------------| "));
	SII_LIB_LOG_PRINT2(("\n|                                           |\n"));
	SII_LIB_LOG_PRINT2(("| %s%s			|\n", lSignonMsg, buildVersion));
	SII_LIB_LOG_PRINT2(("| %s    |\n\n", lCopyrightMsg));
	SII_LIB_LOG_PRINT2(("\n|                                           |"));
	SII_LIB_LOG_PRINT2(("\n|-------------------------------------------|\n "));
	SII_LIB_LOG_PRINT2(("\n "));
	SII_LIB_LOG_PRINT2(("\n "));

	ParseConfig(&txCfg);

	//Configure Cra
	craConfig.baseAddr = txCfg.baseAddr;
	craConfig.pInterfaceInfo = pInterfaceInfo;
	if (pInterfaceInfo->interfaceHardware == SII_PLATFORM_HARDWARE__AARDVARK) {
		craConfig.intrPin  = SII_PLATFORM_GPIO__2;
		craConfig.rstPin   = SII_PLATFORM_GPIO__5;
	} else if (pInterfaceInfo->interfaceHardware == SII_PLATFORM_HARDWARE__BB) {
		craConfig.intrPin  = SII_PLATFORM_GPIO__0;
		craConfig.rstPin   = SII_PLATFORM_GPIO__1;
	}

	craConfig.callBack = SiiDrvCraFailureCallback;
	sInstCra = SiiDrvCraCreate(&craConfig);

	SII_LIB_LOG_DEBUG2(("Resetting Tx...  baseAddr=%d\n", craConfig.baseAddr));
	ret = SiiDrvCraHardwareReset(sInstCra);
	if (!ret) {
		SII_LIB_LOG_DEBUG2(("Reset Tx...FAILED\n"));
	}
	SII_LIB_LOG_DEBUG2(("Reset Tx...Done\n"));
	//Configure & Create Tx
	sTxConfig.baseAddr		= txCfg.baseAddr;
	sTxConfig.instCra		= sInstCra;
	sTxConfig.bHdcp2xEn		= txCfg.bHdcp2xEn;
	sTxConfig.bVidPathEn	= txCfg.bVideopathEn;
	sTxConfig.bCpiEn		= txCfg.bCecEn ? true : false;
	sTxConfig.bScdcEn	    = txCfg.bScDcEn;
	sTxConfig.bHdcpEn	    = txCfg.bHdcpEn;

	sInstTx = SiiDrvTxCreate("TX_IP", &sTxConfig);
	SII_PLATFORM_DEBUG_ASSERT(sInstTx);
	SiiDrvTxRegisterCallBack(sInstTx, SiiTxCallBack);

	//Checking HDCP2x core update status
	if (txCfg.bHdcp2xEn) {
		SiiDrvTxHdcp2xCupdStatusGet(sInstTx, &cupdChkStat);
		switch (cupdChkStat) {
			case SII_DRV_HDCP2X_CUPD_CHK__ERROR:
				SII_LIB_LOG_DEBUG1(NULL, ("HDCP2x Code Update Time Expired... Exit the aplication\n"));
				SII_LIB_LOG_DEBUG1(NULL, ("Press any Key to Exit\n"));
				_getch();
				SiiDrvTxDelete(sInstTx);
				SiiDrvCraDelete(sInstCra);
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__FAIL:
				SII_LIB_LOG_DEBUG1(NULL, ("HDCP2x Code Update Failed... Exit the aplication\n"));
				SII_LIB_LOG_DEBUG1(NULL, ("Press any Key to Exit\n"));
				_getch();
				SiiDrvTxDelete(sInstTx);
				SiiDrvCraDelete(sInstCra);
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__DONE:
				SII_LIB_LOG_DEBUG1(NULL, ("HDCP2x Code Update Successful\n"));
				break;
		}
	}

	//Create CEC
	if (txCfg.bCecEn) {
		SiiCecCreate(sTxConfig.instCra);
	}

	if (txCfg.bTpgEn) {
		sTpgConfig.instTxCra = sTxConfig.instCra;
		sTpgConfig.baseAddr = SII_TPG_BASE_ADDR;
		sInstTpg = SiiDrvTpgCreate(&sTpgConfig);
	}

	#if (MT_SDK_COMPILE_HDMI20 == 0)
	if (txCfg.bSiIMonHandlingEn) {
		sInstSiiMonHandler = SiIMonHandleCreate(sTxConfig.instCra);
		SII_PLATFORM_DEBUG_ASSERT(sInstSiiMonHandler);
	}
	#endif

	if (txCfg.bApiTestingEn) {
		testModInstinfo.instTx = sInstTx;
		testModInstinfo.bCecEn = txCfg.bCecEn;
		testModInstinfo.bProgrammPebbelsEn = txCfg.bProgramPebbels;
		testModInstinfo.bSimmonHandlingEn = txCfg.bSiIMonHandlingEn;
		testModInstinfo.bTpgEn = txCfg.bTpgEn;
		testModInstinfo.bScDcEn = txCfg.bScDcEn;

		if (txCfg.bSiIMonHandlingEn) {
			testModInstinfo.instSiIMonHandle = sInstSiiMonHandler;
		}

		if (txCfg.bTpgEn) {
			testModInstinfo.instTpg = sInstTpg;
		}

		testModInstinfo.instTxCra = sTxConfig.instCra;
		SiiApiTestModuleCreate(pInterfaceInfo, &testModInstinfo);
	}

	return true;
}

void SiiTxDelete(void)
{
	if (txCfg.bApiTestingEn) {
		SiiApiTestModuleDelete();
	}

	#if (MT_SDK_COMPILE_HDMI20 == 0)
	if (txCfg.bSiIMonHandlingEn) {
		SiIMonHandleDelete(sInstSiiMonHandler);
	}
	#endif

	if (txCfg.bTpgEn) {
		SiiDrvTpgDelete(sInstTpg);
	}

	if (txCfg.bCecEn) {
		SiiCecDelete();
	}

	SiiDrvTxDelete(sInstTx);

	SiiDrvCraDelete(sInstCra);
}

SiiInst_t getTxCraInstance(void) //get the App instance and try
{
	return sTxConfig.instCra;
}

bool_t SiiTxReCreate(SiiPlatformInterface_t *pInterfaceInfo)
{
	SiiTxDelete();
	return SiiTxCreate(pInterfaceInfo);
}

SiiInst_t getTxInstance(void) //get the App instance and try
{
	return sInstTx;
}

bool_t getScdcEnable(void)
{
	return sTxConfig.bScdcEn;
}
/***** end of file ***********************************************************/
