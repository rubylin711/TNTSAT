/******************************************************************************
*
* Copyright 2014, Deco, Inc.  All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of
* Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
*
*****************************************************************************/
/**
* @file si_drv_tx_hdmi.c
*
* @brief HDMI Tx Driver
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "si_drv_tx_api.h"
#include "si_lib_edid_api.h"
#include "si_lib_log_api.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "si_lib_time_api.h"
#include "si_drv_tx_regs.h"
#include "si_mod_tx_scdc_api.h"
#if ((0==__HDMI_UBOOT__) && (0==__HDMI_OS_RTOS__))
	#include "mt_common.h"
#elif __HDMI_OS_RTOS__
#include "../mta_hdmi/mta_hdmi.h"
#include "hdmi.h"
#else
	#include <asm/arch-symphony6/mt_common.h>
#endif
/***** Register Module name **************************************************/

SII_LIB_OBJ_MODULE_DEF(drv_tx_hdmi);

/***** Macro definitions *****************************************************/

#define LEN_TPI__DDC_FIFO_SIZE              16

#define TIMER_START__HW_UPDATE              200//1000 tq@20220315 for speed up to run hdcp on/off
#define TIMER_START__HW_UPDATE_PRI			254
#define TIMER_START__HW_UPDATE_200MS_WAIT	200
#define TIMER_START__HW_UPDATE_50MS_WAIT	50
#define TIMER_START__HW_UPDATE_5MS_WAIT		5
#define TIMER_START__HW_UPDATE_100MS_WAIT   100

#define LEN_TPI__IF_BUFFER_LENGTH           31

//Page 6:0xBF - REG_ADDR__TPI_INFO_FSEL
#define BIT_ENUM__TPI_INFO_FSEL__AVI                  0x00
#define BIT_ENUM__TPI_INFO_FSEL__GBD                  0x01
#define BIT_ENUM__TPI_INFO_FSEL__AUDIO                0x02
#define BIT_ENUM__TPI_INFO_FSEL__SPD                  0x03
#define BIT_ENUM__TPI_INFO_FSEL__MPEG                 0x04
#define BIT_ENUM__TPI_INFO_FSEL__VSIF                 0x05
#define BIT_ENUM__TPI_INFO_FSEL__GEN1                 0x06  // use this for ISRC
#define BIT_ENUM__TPI_INFO_FSEL__GEN2                 0x07  // use this for ISRC2
#define BIT_ENUM__TPI_INFO_FSEL__GEN3                 0x08  // use this for ACP
#define BIT_ENUM__TPI_INFO_FSEL__GEN4                 0x09
#define BIT_ENUM__TPI_INFO_FSEL__GEN5                 0x0A

//Page 0:0xF3 - REG_ADDR__DDC_CMD
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_READ				0x02
#define BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ			0x04
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_WRITE				0x06
#define BIT_ENUM__DDC_CMD__CLEAR_FIFO					0x09
#define BIT_ENUM__DDC_CMD__ABORT_TRANSACTION			0x0F

//Page A:0x61 - REG_ADDR__TPI_DOWN_SMPL_CTRL
#define BIT_ENUM__TPI_DOWN_SMPL_CTRL__AUDIO_DOWNSAMP_EN	0x02
#define BIT_ENUM__TPI_DOWN_SMPL_CTRL__AUDIO_PASS_Fs  	0x03

//page_2:0x24 - REG_ADDR__TX_ZONEL_CTL4
#define BIT_MSK__TX_ZONEL_CTL4__REG_HDMI_CLK_RATIO_DEFAULT 0x04

/***** local type definitions ************************************************/
/**
* @brief tx states
*/
typedef enum {
	SII_MOD_TX_HDMI_EVENT__TMDS_OFF,
	SII_MOD_TX_HDMI_EVENT__TMDS_ON,
	SII_MOD_TX_HDMI_EVENT__HDCP_STATE,
	SII_MOD_TX_HDMI_EVENT__HDCP_ON,
	SII_MOD_TX_HDMI_EVENT__HDCP_OFF,
	SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS,
} SiiModTxHdmiInternalState_t;

/**
* Interrupt Status Registers
*/
typedef struct {
	uint8_t              reg0;
	uint8_t              reg1;
} IntStat_t;

//Hdmi Tx Driver Data
typedef struct {
	SiiModTxHdmiConfig_t     *pConfig;

	/*--------------------------------*/
	/* User request states            */
	/*--------------------------------*/
	SiiTmdsMode_t		tmdsMode;
	SiiTmdsMode_t		prevTmdsMode;
	SiiHvSyncPol_t		hvSyncPol;

	bool_t				bIsHdcpOn;
	uint8_t				bWasHdcpOn;
	uint8_t				bIsScdc;
	bool_t				bAvMute;

	uint8_t				bIfOnAvi;
	uint8_t				bIfOnAudio;
	uint8_t				bIfOnVs;
	uint8_t				bIfOnSpd;
	uint8_t				bIfOnGbd;
	uint8_t				bIfOnMpeg;
	uint8_t				bIfOnIsrc;
	uint8_t				bIfOnIsrc2;
	uint8_t				bIfOnAcp;
	uint8_t       bIfOnHdr;

	SiiInfoFrame_t      ifAvi;
	SiiInfoFrame_t      ifAudio;
	SiiInfoFrame_t      ifVs;
	SiiInfoFrame_t      ifSpd;
	SiiInfoFrame_t		ifGbd;
	SiiInfoFrame_t		ifMpeg;
	SiiInfoFrame_t		ifIsrc;
	SiiInfoFrame_t		ifIsrc2;
	SiiInfoFrame_t		ifAcp;
	SiiInfoFrame_t    ifHdr;

	SiiChannelStatus_t   channelStatus;
	SiiAudioFormat_t     audioFormat;
	bool_t				 audioOverTdm;

	SiiDrvBitDepth_t	outputBitDepth;
	SiiDrvClrSpc_t		inputClrSpc;
	SiiDrvClrSpc_t		outputClrSpc;

	/*--------------------------------*/
	/* User status                    */
	/*--------------------------------*/
	SiiEdid_t			edid;
	bool_t				bHotPlug;
	bool_t				bRsen;
	uint8_t				bInitHotPlug;

	/*--------------------------------*/
	/* Internal states                */
	/*--------------------------------*/
	SiiInst_t			timerHwUpdate;     //!< Timer for hw updates
	SiiInst_t			hdcpInst;          //!< hdcp instance
	SiiInst_t			scdcInst;
	IntStat_t			intStat;
	SiiDrvHdcpStatus_t	hdcpStatus;

	SiiModTxHdmiInternalState_t		iState;
	SiiModTxHdmiState_t			eState;
	SiiLibEdidPar_t				parseEdid;

	HdmiTxEventNotifyCallBack	cbFunc;

	uint32_t    tx_scdc_events;
} HdmiTxObj_t;

/***** local prototypes ******************************************************/
static void sTxHwUpdateHandler(SiiInst_t inst);
static void sVirtualIsrHandler(HdmiTxObj_t* pObj);
static void sPlugStatusUpdate(HdmiTxObj_t* pObj);
static void sPlugStatusUpdatePseudo(HdmiTxObj_t* pObj, uint8_t flg);
static uint8_t sPlugStatus(HdmiTxObj_t* pObj);
static SiiDdcComErr_t sUpdateEdid(HdmiTxObj_t* pObj, uint8_t segmentIndex, uint8_t regAddr, uint8_t *pBuf, uint16_t length, uint16_t mode);
static bool_t sWaitForDdcBus(HdmiTxObj_t* pObj);
static SiiLibEdidErrCode_t sParseEdid(HdmiTxObj_t* pObj);
static void sApplyInfoFrame(HdmiTxObj_t* pObj, uint8_t bApply);
static void sInfoframesSet(HdmiTxObj_t* pObj);
static void sInfoframeTypeSet(HdmiTxObj_t* pObj, SiiInfoFrame_t *pInfoFrame);
static void sInfoframeOnOffSet(HdmiTxObj_t* pObj, SiiInfoFrameId_t ifId, uint8_t onOff);
static void sUpdateInfoframes(HdmiTxObj_t* pObj);
static void sAudioformatSet(HdmiTxObj_t* pObj);
static void sUpdateAudioformat(HdmiTxObj_t* pObj);
static void sChannelStateSet(HdmiTxObj_t* pObj);
static void sUpdateChannelState(HdmiTxObj_t* pObj);
static void sPutTMDSOnOff(HdmiTxObj_t* pObj, bool_t onOff);
static void sUpdateHdmiMode(HdmiTxObj_t* pObj);
static void sTmdsModeSet(HdmiTxObj_t* pObj);
//static void sUpdateAvMute(HdmiTxObj_t* pObj);
static void sUpdateHotPlugRsen(HdmiTxObj_t* pObj);
static void sUpdateOutputBitDepth(HdmiTxObj_t* pObj);
static SiiDrvCraAddr_t sBaseAddrGet(HdmiTxObj_t *pObj);
static SiiInst_t sCraInstGet(HdmiTxObj_t *pObj);
static void sTxLog(uint8_t *pData, uint16_t len);
static void sClearInfoFrame(SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame);

//Notify functions
static void sNotifyHdmiState(HdmiTxObj_t *pObj);
static void sTxHdmiScdcCallBack(SiiInst_t inst, uint32_t scdcEvents);
static void sPutTMDSMute(HdmiTxObj_t* pObj, bool_t onOff);
void sSetDelayEDIDRead(uint32_t delay);
void sSetEDIDPrintEn(uint32_t en);

/***** local data objects ****************************************************/

static SiiModTxScdcConfig_t sScdcConfig = {
	0x0,			//!<  baseAddr   : Base Addrress of SCDC-Tx register space
	true,			//!<  bReadReq   : If true then use read-request if Sink support read request as well.
	1,				//!<  srcVersion : Source version
};
uint8_t g_edata_back[SII_EDID_MAX_LEN];
uint8_t g_edata_hpdren = 0;
static uint8_t g_mthpd_state_bk = 0;

/***** public functions ******************************************************/
#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	#include "mt_unf_edid.h"
	#include "drv_global.h"
	#include "mt4si/linux_k/si_mod_tx_hdmi_mt.h"
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize HDMI TX module
//-------------------------------------------------------------------------------------------------
SiiInst_t SiiModTxHdmiCreate(char *pNameStr, SiiModTxHdmiConfig_t *pConfig)
{
	HdmiTxObj_t*	hdmiObj = NULL;
	SiiDrvCraAddr_t	baseAddr;
	SiiInst_t		craInst;

	/* Allocate memory for hdmi_tx object */
	hdmiObj = (HdmiTxObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(HdmiTxObj_t));
	SII_PLATFORM_DEBUG_ASSERT(hdmiObj);

	hdmiObj->pConfig = (SiiModTxHdmiConfig_t*)SiiLibMallocCreate(sizeof(SiiModTxHdmiConfig_t));
	if ( hdmiObj->pConfig ) {
		memcpy(hdmiObj->pConfig, pConfig, sizeof(SiiModTxHdmiConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiModTxHdmiCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	baseAddr = sBaseAddrGet(hdmiObj);
	craInst	 = sCraInstGet(hdmiObj);
	hdmiObj->cbFunc = hdmiObj->pConfig->cbFunc;

	/*--------------------------------*/
	/* Initialize user request states */
	/*--------------------------------*/
	//hdmiObj->primLink        = pConfig->primLink;
	//hdmiObj->bDualLink       = false;
	#if ( 1 == __HDMI_UBOOT__)
	hdmiObj->tmdsMode			= SII_TMDS_MODE__HDMI1;
	hdmiObj->prevTmdsMode		= SII_TMDS_MODE__HDMI1;
	#else
	hdmiObj->tmdsMode			= SII_TMDS_MODE__NONE;
	hdmiObj->prevTmdsMode		= SII_TMDS_MODE__NONE;
	#endif
	hdmiObj->hvSyncPol			= SII_HV_SYNC_POL__HPVP;
	hdmiObj->eState				= SII_MOD_TX_HDMI_STATUS__TMDS_OFF;

	hdmiObj->bInitHotPlug		= false;  // set this to false, so it will check the h/w status

	hdmiObj->bIsHdcpOn			= hdmiObj->pConfig->bHdcpEn;//true;
	hdmiObj->bWasHdcpOn			= false;
	hdmiObj->bAvMute			= false;
	hdmiObj->bIfOnAvi			= true;
	hdmiObj->bIfOnAudio			= true;
	hdmiObj->bIfOnVs			= true;
	hdmiObj->bIfOnSpd			= false;
	hdmiObj->bIfOnGbd			= false;
	hdmiObj->bIfOnMpeg			= false;
	hdmiObj->bIfOnIsrc			= false;
	hdmiObj->bIfOnIsrc2			= false;
	hdmiObj->bIfOnAcp			= false;

	/*--------------------------------*/
	/* Initialize user status         */
	/*--------------------------------*/
	hdmiObj->bHotPlug	= false;
	hdmiObj->bRsen		= false;

	/*--------------------------------*/
	/* Initialize interrupts Status    */
	/*--------------------------------*/
	hdmiObj->intStat.reg0 = 0x00;
	hdmiObj->intStat.reg1 = 0x00;

	//Clear Infoframes
	sClearInfoFrame(SII_INFO_FRAME_ID__AVI,    &hdmiObj->ifAvi);
	sClearInfoFrame(SII_INFO_FRAME_ID__AUDIO,  &hdmiObj->ifAudio);
	sClearInfoFrame(SII_INFO_FRAME_ID__VS,     &hdmiObj->ifVs);
	sClearInfoFrame(SII_INFO_FRAME_ID__SPD,    &hdmiObj->ifSpd);
	sClearInfoFrame(SII_INFO_FRAME_ID__GBD,    &hdmiObj->ifGbd);
	sClearInfoFrame(SII_INFO_FRAME_ID__MPEG,   &hdmiObj->ifMpeg);
	sClearInfoFrame(SII_INFO_FRAME_ID__ISRC,   &hdmiObj->ifIsrc);
	sClearInfoFrame(SII_INFO_FRAME_ID__ISRC2,  &hdmiObj->ifIsrc2);
	sClearInfoFrame(SII_INFO_FRAME_ID__ACP,    &hdmiObj->ifAcp);
	sClearInfoFrame(SII_INFO_FRAME_ID__HDR,     &hdmiObj->ifHdr);

	//Set Audio Infoframes
	hdmiObj->ifAudio.b[0] = 0x84;
	hdmiObj->ifAudio.b[1] = 0x01;
	hdmiObj->ifAudio.b[2] = 0x0a;
	hdmiObj->ifAudio.b[3] = 0x70;
	hdmiObj->ifAudio.b[4] = 0x01;	// 2-ch
	hdmiObj->ifAudio.b[5] = 0x00;
	hdmiObj->ifAudio.b[6] = 0x00;
	hdmiObj->ifAudio.b[7] = 0x00;
	hdmiObj->ifAudio.b[8] = 0x00;
	hdmiObj->ifAudio.b[9] = 0x00;
	hdmiObj->ifAudio.b[10] = 0x00;
	hdmiObj->ifAudio.b[11] = 0x00;
	hdmiObj->ifAudio.b[12] = 0x00;
	hdmiObj->ifAudio.b[13] = 0x00;

	//Set Audio Channel Status
	hdmiObj->channelStatus.i2s_chst0 = 0x00;
	hdmiObj->channelStatus.i2s_chst1 = 0x00;
	hdmiObj->channelStatus.i2s_chst2 = 0x00;
	hdmiObj->channelStatus.i2s_chst3 = 0x02;
	hdmiObj->channelStatus.i2s_chst4 = 0x0b;
	hdmiObj->channelStatus.i2s_chst5 = 0x00;
	hdmiObj->channelStatus.i2s_chst6 = 0x00;

	//Set Audio Format
	//hdmiObj->audioFormat.spdif = true;
	hdmiObj->audioFormat.i2s = true;
	hdmiObj->audioFormat.layout1 = AUDIO_FORMAT__2CH;
	hdmiObj->audioFormat.audioFs = SII_AUDIO_FS__48KHZ;

	//Set TDM Audio
	hdmiObj->audioOverTdm = false;

	// Create a timer to update Tx states
	hdmiObj->timerHwUpdate = SII_LIB_SEQ_TIMER_CREATE("Hardware_Update", sTxHwUpdateHandler, SII_LIB_OBJ_INST(hdmiObj), TIMER_START__HW_UPDATE_PRI);
	SII_PLATFORM_DEBUG_ASSERT(hdmiObj->timerHwUpdate);

	//Creating SCDC
	if (pConfig->bScdcEn) {
		sScdcConfig.instTxCra = pConfig->instTxCra;
		sScdcConfig.baseAddr = pConfig->baseAddrTx;
		sScdcConfig.cbFunc = sTxHdmiScdcCallBack;
		hdmiObj->scdcInst = SiiModTxScdcCreate("tx_scdc", SII_LIB_OBJ_INST(hdmiObj), &sScdcConfig);
		pConfig->scdcInst = hdmiObj->scdcInst;
		SII_PLATFORM_DEBUG_ASSERT(hdmiObj->scdcInst);

		hdmiObj->pConfig->scdcInst = hdmiObj->scdcInst;
		pConfig->scdcInst = hdmiObj->pConfig->scdcInst;
	}
	/*--------------------------------*/
	/* Static hardware configuration  */
	/*--------------------------------*/
	//Tx IP initialization

	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__ASRC		, 0x00);
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__PKT_FILTER_0		, BIT_MSK__PKT_FILTER_0__REG_DROP_CTS_PKT);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__CLKPWD		, BIT_MSK__CLKPWD__REG_PDIDCK_N);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__FUNC_SEL		, BIT_MSK__FUNC_SEL__REG_HDMI_EN);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DIPT_CNTL		, BIT_MSK__DIPT_CNTL__REG_AUD_SPLIT_EN | BIT_MSK__DIPT_CNTL__REG_AUD_BYP_MODE);
	//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__P2T_CTRL		, BIT_MSK__P2T_CTRL__REG_DC_PKT_EN);
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__P2T_CTRL		, BIT_MSK__P2T_CTRL__REG_NULL_PKT_EN_VS_HI);
	SI_HDMI20_PRINT("\n[%s_%d] 0x%x=0x%x\n", __func__, __LINE__, baseAddr | REG_ADDR__P2T_CTRL, SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__P2T_CTRL));
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TEST_TXCTRL	, BIT_MSK__TEST_TXCTRL__REG_HDMI_MODE);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TX_ZONEL_CTL4	, 0x04);
	//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__MHL_TOP_CTL	, BIT_MSK__MHL_TOP_CTL__REG_MHL3_DOC_SEL | BIT_MSK__MHL_TOP_CTL__REG_HDMI_EN);
	//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__MHL_CBUS_CTL0, BIT_MSK__MHL_CBUS_CTL0__REG_CBUS_DRV_SEL, 0x01);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__MHL_TOP_CTL, BIT_MSK__MHL_TOP_CTL__REG_MHL3_DOC_SEL | BIT_MSK__MHL_TOP_CTL__REG_HDMI_EN);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__MHL_DP_CTL0, 0x10);
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__CLKRATIO	, BIT_MSK__CLKRATIO__REG_CLKRATIO_SW_EN);    //enabling HW decoded clock ratio

	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__AIP_RST, BIT_MSK__AIP_RST__REG_RST4AUDIO_FIFO | BIT_MSK__AIP_RST__REG_RST4AUDIO);
	// Tx IP - audio related initialization
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__I2S_CHST3, 0x02);             // set AIP channel Sampling frequency to 48 KHz
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__I2S_CHST4, 0x0b);             // set AIP channel status word length to 24 bits
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__ACR_CTRL, 0x0e);              // configure AIP cts generation
	//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__I2S_IN_SIZE, 0x0b);           // set AIP i2s word length to 24 bits
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__I2S_IN_SIZE, 0x02);           // set AIP i2s word length to 24 bits
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__I2S_IN_CTRL, 0x60);           // configure AIP i2s input
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xF0, 0x10);       // enable sd0-3 input
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT); // layout 0
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_EN, BIT_MSK__AUD_EN__REG_AUD_IN_EN, BIT_MSK__AUD_EN__REG_AUD_IN_EN);// enable audio
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_EN, BIT_MSK__AUD_EN__REG_AUD_SEL_OWRT, BIT_MSK__AUD_EN__REG_AUD_SEL_OWRT);// enable audio
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AIP_HDMI2MHL, SII_BIT3);

	//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_DOWN_SMPL_CTRL, BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUDIO_LOOKUP_EN);// enable LUT
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_AUD_CONFIG, 0x00);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x02);// fs = 48kHz
	//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, BIT_MSK__TPI_AUD_FS__REG_TPI_AUD_SF_OVRD, BIT_MSK__TPI_AUD_FS__REG_TPI_AUD_SF_OVRD);// enable fs override

	// reset AIP and AFIFO
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__AIP_RST, 0x00);

	// Force I2C SDA/SCL output '0'
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__DDC_MANUAL, BIT_MSK__DDC_MANUAL__REG_MAN_DDC | BIT_MSK__DDC_MANUAL__REG_DSDA | BIT_MSK__DDC_MANUAL__REG_DSCL);

	//Enable Intr1 Interrupts.
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__INTR1_MASK, BIT_MSK__INTR1_MASK__REG_INTR1_MASK5 | BIT_MSK__INTR1_MASK__REG_INTR1_MASK6);
	//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__INTR1_MASK, BIT_MSK__INTR1_MASK__REG_INTR1_MASK6);
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	//Enable EMP intr
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__INTR_MASK, BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK | \
					BIT_MSK_REG__EMP_ERR_HDMI_MASK | BIT_MSK_REG__EMP_ERR_CPU_MASK | \
					BIT_MSK_REG__DMA_DONE_MASK);
	#endif

	//Set Internal State to TMDS_OFF
	hdmiObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;

	return SII_LIB_OBJ_INST(hdmiObj);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Delete HDMI TX module
//-------------------------------------------------------------------------------------------------
void SiiModTxHdmiDelete(SiiInst_t inst)
{
	HdmiTxObj_t* hdmiObj = (HdmiTxObj_t*)SII_LIB_OBJ_PNTR(inst);
	if (hdmiObj->pConfig->bScdcEn) {
		SiiModTxScdcDelete(hdmiObj->scdcInst);
	}
	SiiLibSeqTimerDelete(hdmiObj->timerHwUpdate);

	SiiLibMallocDelete(hdmiObj->pConfig);
	SII_LIB_OBJ_DELETE(hdmiObj);
}

//-------------------------------------------------------------------------------------------------
//! @brief      HDMI TX Interrupt handler.
//!
//!             Check for HDMI TX related interrupts. If found, clear pending hardware interrupt
//!             bits and change the status to indicate pending interrupt.
//!
//!             This function is to be called from the Device Interrupt manager upon receiving
//!             a hardware interrupt from TX.
//!
//! @param[in]  inst  - instance of the notification addressee
//-------------------------------------------------------------------------------------------------
void SiiModTxHdmiTpiInterruptHandler(SiiInst_t inst)
{
	HdmiTxObj_t*	pObj		= (HdmiTxObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr	= sBaseAddrGet(pObj);
	SiiInst_t		craInst		= sCraInstGet(pObj);
	IntStat_t       intStat = {0};
	uint8_t         mask;

	// Capture and mask HPD(STAT6) and RSEN(STAT5) interrupt status bits.
	mask = BIT_MSK__INTR1__REG_INTR1_STAT6 | BIT_MSK__INTR1__REG_INTR1_STAT5;
	//mask = BIT_MSK__INTR1__REG_INTR1_STAT6;

	intStat.reg0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR1);
	intStat.reg0 &= mask;
	SI_HDMI20_PRINT("\n0x308f=0x%x __ 0x%x\n", (uint32_t)intStat.reg0, (uint32_t)pObj->intStat.reg0);

	if (SII_MEMCMP(&pObj->intStat, &intStat, sizeof(IntStat_t))) {
		uint8_t intreg = 0;

		// Find any interrupt status bit that changed to '1'
		intreg |= ((pObj->intStat.reg0 ^ intStat.reg0) & intStat.reg0);

		pObj->intStat = intStat;
		if (intreg || pObj->bInitHotPlug) {
			// Call derived interupt handler
			sVirtualIsrHandler(pObj);
			pObj->bInitHotPlug = false;
		}
	}
}

//Todo : Try to return ErrorStatus
//-------------------------------------------------------------------------------------------------
//! @brief      HDMI TX module Set API
//-------------------------------------------------------------------------------------------------
bool_t SiiModTxHdmiSet(SiiInst_t inst, SiiModTxHdmiOpcode_t opcode, const void *inData)
{
	HdmiTxObj_t* pObj = (HdmiTxObj_t*)SII_LIB_OBJ_PNTR(inst);
	switch (opcode) {
		case SII_MOD_TX_HDMI_OPCODE__TMDS_MODE: {
			SII_MEMCPY(&pObj->tmdsMode, inData, sizeof(SiiTmdsMode_t));
			sTmdsModeSet(pObj);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__INFOFRAME_TYPE: {
			SiiInfoFrame_t *pInfoframe = (SiiInfoFrame_t*)inData;
			sInfoframeTypeSet(pObj, pInfoframe);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__INFOFRAME_ONOFF: {
			uint8_t *inp = (uint8_t *)inData;
			sInfoframeOnOffSet(pObj, (SiiInfoFrameId_t)inp[0], inp[1]);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__CHANNEL_STATUS: {
			SII_MEMCPY(&pObj->channelStatus, inData, sizeof(SiiChannelStatus_t));
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0); //Make this call for all Set cases
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__AUDIO_FORMAT: {
			SII_MEMCPY(&pObj->audioFormat, inData, sizeof(SiiAudioFormat_t));
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__OUTPUT_BIT_DEPTH: {
			SII_MEMCPY(&pObj->outputBitDepth, inData, sizeof(SiiDrvBitDepth_t));
			sUpdateOutputBitDepth(pObj);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__HDCP_PROTECTION: {
			SII_MEMCPY(&pObj->bIsHdcpOn, inData, sizeof(bool_t));
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_100MS_WAIT, 0);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__HDCP_STATUS: {
			SII_MEMCPY(&pObj->hdcpStatus, inData, sizeof(SiiDrvHdcpStatus_t));
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__HW_UPDATE_START:
			//SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_RESET_PEER_UPDATE_REG:
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__PEER_SCDC_STATUS, NULL);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_AUTOPOLL_MODE:
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__ENABLE_AUTOPOLL_MODE, NULL);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_ENABLE_READ_REQ_TEST: {
			uint8_t *requestenable = (uint8_t *)inData;
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__ENABLE_READ_REQ_TEST, (void *)requestenable);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_ENABLE: {
			uint8_t *scrambleclk = (uint8_t *)inData;
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_ENABLE, scrambleclk);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_DISABLE: {
			uint8_t *scrambleclk = (uint8_t *)inData;
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_DISABLE, scrambleclk);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__TMDS_MUTE: {
			bool_t *mute = (bool_t *)inData;
			sPutTMDSMute(pObj, *mute);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_CR: {
			bool_t *crAbove340 = (bool_t *)inData;
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SRC_CR, (void *)crAbove340);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__SCDC_SRC_STREAM_TYPE: {
			uint8_t *st = (uint8_t *)inData;
			SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SRC_STREAM_TYPE, (void *)st);
			break;
		}
		case SII_MOD_TX_HDMI_OPCODE__READ_EDID_FROM_SINK: {
			uint8_t *st = (uint8_t *)inData;
			sPlugStatusUpdatePseudo(pObj, *st);
			break;
		}
		default:
			SI_HDMI20_PRINT("HDMI set err opcode:%d", opcode);
			break;
	}

	return true;
}

//Todo : Try to return ErrorStatus
//-------------------------------------------------------------------------------------------------
//! @brief      HDMI TX module Get API
//-------------------------------------------------------------------------------------------------
bool_t SiiModTxHdmiGet(SiiInst_t inst, SiiModTxHdmiOpcode_t opcode, void *outData)
{
	HdmiTxObj_t* pObj = (HdmiTxObj_t*)SII_LIB_OBJ_PNTR(inst);
	uint16_t cecAddr = 0;
	switch (opcode) {
		case SII_MOD_TX_HDMI_OPCODE__EDID:
			//*outData = pObj->edid;
			SII_MEMCPY(outData, &(pObj->edid), sizeof(SiiEdid_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__PARSED_EDID:
			SII_MEMCPY(outData, &(pObj->parseEdid), sizeof(SiiLibEdidPar_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__HOTPLUG:
			SII_MEMCPY(outData, &(pObj->bHotPlug), sizeof(bool_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__RSEN:
			SII_MEMCPY(outData, &(pObj->bRsen), sizeof(bool_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__HDMI_STATE:
			SII_MEMCPY(outData, &(pObj->eState), sizeof(SiiModTxHdmiState_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__TMDS_MODE:
			SII_MEMCPY(outData, &(pObj->tmdsMode), sizeof(SiiTmdsMode_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__CEC_PHY_ADDR:
			cecAddr = ((pObj->parseEdid.cecAddr.sub[0] << 12) | (pObj->parseEdid.cecAddr.sub[1] << 8)
					   | (pObj->parseEdid.cecAddr.sub[2] << 4) | pObj->parseEdid.cecAddr.sub[3]);
			SII_MEMCPY(outData, &(cecAddr), sizeof(uint16_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__AUDIO_FORMAT:
			SII_MEMCPY(outData, &(pObj->audioFormat), sizeof(SiiAudioFormat_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__EDID_LIPSYNC:
			SII_MEMCPY(outData, &(pObj->parseEdid.lipSync), sizeof(SiiLipSyncInfo_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_MANF_STATUS:
			SiiModTxScdcGet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__PEER_MANF_STATUS, outData);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_PEER_REG_STATUS:
			SiiModTxScdcGet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__PEER_SCDC_STATUS, outData);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_SCRAMBLE_CLOCK_STATUS:
			SiiModTxScdcGet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCRAMBLE_CLOCK_STATUS, outData);
			break;
		case SII_MOD_TX_HDMI_OPCODE__SCDC_EVENTS:
			memcpy(outData, &pObj->tx_scdc_events, sizeof(uint32_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__SM_UPDATE_STS:
			*((mt_u32 *)outData) = 0;
			*((mt_u32 *)outData) |= false == pObj->bWasHdcpOn && false == pObj->bIsHdcpOn && (pObj->iState == SII_MOD_TX_HDMI_EVENT__TMDS_OFF || pObj->iState == SII_MOD_TX_HDMI_EVENT__TMDS_ON);
			*((mt_u32 *)outData) |= (true == pObj->bWasHdcpOn && true == pObj->bIsHdcpOn && (pObj->iState == SII_MOD_TX_HDMI_EVENT__TMDS_OFF || pObj->iState == SII_MOD_TX_HDMI_EVENT__TMDS_ON)) << 1;
			#if 0
			if (SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS == pObj->iState) {
				*((mt_u32 *)outData) = 1;
			} else if (SII_MOD_TX_HDMI_EVENT__HDCP_STATE == pObj->iState) {
				*((mt_u32 *)outData) = 2;
			} else if (SII_MOD_TX_HDMI_EVENT__TMDS_ON == pObj->iState ||
					   SII_MOD_TX_HDMI_EVENT__TMDS_OFF == pObj->iState) {
				*((mt_u32 *)outData) = 4;
			} else {
				*((mt_u32 *)outData) = 0;
			}
			#endif
			SII_LIB_LOG_DEBUG1(pObj, ("\nGetSts:%d,%d,%d,%d\n", pObj->bWasHdcpOn,pObj->bIsHdcpOn,(mt_u32)pObj->iState, (mt_u32)(*((mt_u32 *)outData))));
			break;
		case SII_MOD_TX_HDMI_OPCODE__HDCP_PROTECTION:
			SI_HDMI20_PRINT("\n.... %s ...\n", pObj->bIsHdcpOn ? "HDCP_ON_USER" : "HDCP_OFF_USER");
			memcpy(outData, &pObj->bIsHdcpOn, sizeof(bool_t));
			break;
		case SII_MOD_TX_HDMI_OPCODE__INFOFRAME_ONOFF:
			((uint8_t*)outData)[0] = pObj->bIfOnAvi;
			((uint8_t*)outData)[1] = pObj->bIfOnGbd;
			((uint8_t*)outData)[2] = pObj->bIfOnAudio;
			((uint8_t*)outData)[3] = pObj->bIfOnSpd;
			((uint8_t*)outData)[4] = pObj->bIfOnMpeg;
			((uint8_t*)outData)[5] = pObj->bIfOnVs;
			((uint8_t*)outData)[6] = pObj->bIfOnIsrc;
			((uint8_t*)outData)[7] = pObj->bIfOnIsrc2;
			((uint8_t*)outData)[8] = pObj->bIfOnHdr;
			break;
		case SII_MOD_TX_HDMI_OPCODE__TPI_PLUG_STATUS:
			((uint8_t*)outData)[0] = sPlugStatus(pObj);
			break;
		default:
			SI_HDMI20_PRINT("HDMI get err opcode:%d", opcode);
			break;
	}
	return true;
}
/***** local functions *******************************************************/

static void sClearInfoFrame(SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame)
{
	pInfoFrame->ifId = ifId;
	SII_MEMSET(&pInfoFrame->b[0], 0, SII_INFOFRAME_MAX_LEN);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Hardware Update handler
//!
//!             update hardware registers based on the guidance from integration layer.
//!
//! @param[in]  inst  - instance of the notification addressee
//-------------------------------------------------------------------------------------------------
static void sTxHwUpdateHandler(SiiInst_t inst)
{
	HdmiTxObj_t* pObj = (HdmiTxObj_t*)SII_LIB_OBJ_PNTR(inst);

	SII_LIB_LOG_DEBUG1(pObj, ("sTxHwUpdateHandler TM:%02X,hdcp:%s_%s,sm:%d,sts:%d\n", (uint32_t)pObj->tmdsMode, \
							  (uint32_t)pObj->bWasHdcpOn ? "ON" : "OFF",(uint32_t)pObj->bIsHdcpOn ? "ON" : "OFF", (uint32_t)pObj->iState, (uint32_t)pObj->hdcpStatus));

	switch (pObj->iState) {
		case SII_MOD_TX_HDMI_EVENT__TMDS_OFF:

			SII_LIB_LOG_DEBUG1(pObj, ("TMDS_OFF\n"));
			if (pObj->bWasHdcpOn) {
				pObj->bWasHdcpOn = false;
				pObj->iState = SII_MOD_TX_HDMI_EVENT__HDCP_OFF;
			} else {
				pObj->iState = SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS;
			}
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			break;

		case SII_MOD_TX_HDMI_EVENT__TMDS_ON:
			SII_LIB_LOG_DEBUG1(pObj, ("TMDS_ON\n"));
			if ( ( pObj->tmdsMode == SII_TMDS_MODE__HDMI1 ) || ( pObj->tmdsMode == SII_TMDS_MODE__HDMI2 ) ) {
				sUpdateInfoframes(pObj);
				sUpdateAudioformat(pObj);
				sUpdateChannelState(pObj);
				//SiiDrvCraWrReg8((ulong)NULL, REG_ADDR__AIP_RST, 0x00);
				SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__AUDP_TXCTRL, 0x80);
			}

			if ( pObj->bIsHdcpOn && !pObj->bWasHdcpOn) {
				pObj->bWasHdcpOn = pObj->bIsHdcpOn;
				pObj->iState = SII_MOD_TX_HDMI_EVENT__HDCP_ON;
				SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			} else if ( !pObj->bIsHdcpOn && pObj->bWasHdcpOn ) {
				pObj->bWasHdcpOn = pObj->bIsHdcpOn;
				pObj->iState = SII_MOD_TX_HDMI_EVENT__HDCP_OFF;
				SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			} else {
				pObj->iState = SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS;
				SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
			}

			break;

		case SII_MOD_TX_HDMI_EVENT__HDCP_ON:
			if ( pObj->bIsHdcpOn ) {
			SII_LIB_LOG_DEBUG1(pObj, ("HDCP_ON\n"));
			sUpdateHdmiMode(pObj);
			pObj->prevTmdsMode = pObj->tmdsMode;
			pObj->eState = SII_MOD_TX_HDMI_STATUS__HDCP_ON;
			sNotifyHdmiState(pObj);
			pObj->iState = SII_MOD_TX_HDMI_EVENT__HDCP_STATE;
			} else {
				SII_LIB_LOG_DEBUG1(pObj, ("HDCP_OFF\n"));
				// stop HDCP
				pObj->bWasHdcpOn = pObj->bIsHdcpOn;
				pObj->eState = SII_MOD_TX_HDMI_STATUS__HDCP_OFF;
				sNotifyHdmiState(pObj);
				pObj->iState = SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS;
				SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_100MS_WAIT, 0);
			}
			break;

		case SII_MOD_TX_HDMI_EVENT__HDCP_OFF:
			SII_LIB_LOG_DEBUG1(pObj, ("HDCP_OFF\n"));
			// stop HDCP
			pObj->eState = SII_MOD_TX_HDMI_STATUS__HDCP_OFF;
			sNotifyHdmiState(pObj);
			pObj->iState = SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS;
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_100MS_WAIT, 0);
			break;

		case SII_MOD_TX_HDMI_EVENT__HDCP_STATE:
			if ( !pObj->bIsHdcpOn && pObj->bWasHdcpOn ) {
				pObj->bWasHdcpOn = pObj->bIsHdcpOn;
				pObj->iState = SII_MOD_TX_HDMI_EVENT__HDCP_OFF;
				SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
				break;
			}
			switch (pObj->hdcpStatus) {
				case SII_DRV_HDCP_STATUS__SUCCESS_1X:
				case SII_DRV_HDCP_STATUS__SUCCESS_22: {
					/*if(!pObj->bBksvListApproved)
					{
					return;
					}*/
					pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
					break;
				}
				case SII_DRV_HDCP_STATUS__FAILED:
					return;

				default:
					return;
			}
			break;
		case SII_MOD_TX_HDMI_EVENT__UPDATE_TMDS:
			SII_LIB_LOG_DEBUG1(pObj, ("UPDATE_TMDS\n"));
			if (pObj->tmdsMode) {
				pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
			} else {
				pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;
			}
			pObj->prevTmdsMode = pObj->tmdsMode;
			if ( ( pObj->tmdsMode == SII_TMDS_MODE__HDMI1 ) || ( pObj->tmdsMode == SII_TMDS_MODE__HDMI2 ) ) {
				sUpdateInfoframes(pObj);
				sUpdateAudioformat(pObj);
				sUpdateChannelState(pObj);
				//SiiDrvCraWrReg8((ulong)NULL, REG_ADDR__AIP_RST, 0x00);
				SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__AUDP_TXCTRL, 0x80);
			}
			sUpdateHdmiMode(pObj);
			break;

		default:
			SI_HDMI20_PRINT("HDMI Update err state:%d", pObj->iState);
			break;
	}
}

static void sVirtualIsrHandler(HdmiTxObj_t* pObj)
{
	// Clear all pending hardware TX interrupts
	if (pObj->intStat.reg0) {
		SiiDrvCraWrReg8(sCraInstGet(pObj), sBaseAddrGet(pObj) | REG_ADDR__INTR1, pObj->intStat.reg0);
		pObj->intStat.reg0 = 0x00;
	}

	sPlugStatusUpdate(pObj);

}

static void sSetHwTpiBit(HdmiTxObj_t* pObj)
{
	SII_LIB_LOG_DEBUG1(pObj, ("Set HW TPI bit!!\n"));
	SiiDrvCraSetBit8(sCraInstGet(pObj), sBaseAddrGet(pObj) |
					 REG_ADDR__LM_DDC, BIT_MSK__LM_DDC__REG_SW_TPI_EN );
}

static void sResetTpiStateMachine(HdmiTxObj_t* pObj)
{
	SII_LIB_LOG_DEBUG1(pObj, ("Resetting TPI State Machine!!!!!\n"));

	/* workaround for hardare ddc issue with scdc module */
	SiiDrvCraClrBit8(sCraInstGet(pObj), sBaseAddrGet(pObj) | REG_ADDR__SCDC_CTL,
					 BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY);

	SiiDrvCraSetBit8(sCraInstGet(pObj), sBaseAddrGet(pObj) |
					 REG_ADDR__TPI_HW_OPT0, BIT_MSK__TPI_HW_OPT0__REG_HW_TPI_SM_RST );
	SiiDrvCraClrBit8(sCraInstGet(pObj), sBaseAddrGet(pObj) |
					 REG_ADDR__TPI_HW_OPT0, BIT_MSK__TPI_HW_OPT0__REG_HW_TPI_SM_RST );

	/* workaround for hardare ddc issue with scdc module */
	SiiDrvCraSetBit8(sCraInstGet(pObj), sBaseAddrGet(pObj) | REG_ADDR__SCDC_CTL,
					 BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY);
}

static uint32_t g_EDIDReadDelaynMs = 100;
uint32_t g_Hdmi_CommonDebug = 0;
void sSetDelayEDIDRead(uint32_t delay)
{
	g_EDIDReadDelaynMs = delay;
}

void sSetEDIDPrintEn(uint32_t en)
{
	SiiLibEdidPrintEn((uint8_t)en);
}

void sSetHdmi_CommonDebug(uint32_t en)
{
	g_Hdmi_CommonDebug=en;
}

void sSetDDCManual(HdmiTxObj_t* pObj, uint8_t man, uint8_t sda, uint8_t scl)
{
	SiiInst_t		craInst 	= sCraInstGet(pObj);
	uint8_t rval = 0;

	if ( man ) {
		// Force I2C SDA/SCL output '0'
		rval = BIT_MSK__DDC_MANUAL__REG_MAN_DDC;
		if ( sda ) {
			rval |= BIT_MSK__DDC_MANUAL__REG_DSDA;
		}
		if ( scl ) {
			rval |= BIT_MSK__DDC_MANUAL__REG_DSCL;
		}
		SiiDrvCraSetBit8(craInst, sBaseAddrGet(pObj) | REG_ADDR__DDC_MANUAL, rval);
	} else {
		SiiDrvCraClrBit8(craInst, sBaseAddrGet(pObj) | REG_ADDR__DDC_MANUAL, BIT_MSK__DDC_MANUAL__REG_MAN_DDC | BIT_MSK__DDC_MANUAL__REG_DSDA | BIT_MSK__DDC_MANUAL__REG_DSCL);
	}
}

static SiiDdcComErr_t sReadEdid(HdmiTxObj_t* pObj, SiiEdid_t *edid) {
	SiiDdcComErr_t			ddcErr;
	uint8_t extensions			= 0;
	ddcErr = sUpdateEdid(pObj, 0, 0, &(edid->b[0]), SII_EDID_BLOCK_SIZE, BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);
	if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR) {
		//find how many extension blocks are there, read them and store.
		extensions = edid->b[0x7E];

		if (extensions >= SII_EDID_TOTAL_BLOCKS) {
			SII_LIB_LOG_DEBUG1(pObj, ("****Warning****:: EDID: DS supports %d Extensions. But Tx IP Supports %d extensions !!\n", extensions, (SII_EDID_TOTAL_BLOCKS - 1) ));
			extensions = (SII_EDID_TOTAL_BLOCKS - 1);
		}

		if (extensions) {
			if (extensions >= 1) {
				ddcErr = sUpdateEdid( pObj, 0, SII_EDID_BLOCK_SIZE, &(edid->b[SII_EDID_BLOCK_SIZE]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);
				if ( ddcErr == SII_DDC_ERROR_CODE_NO_ERROR ) {
					uint8_t *bk1 = &(edid->b[SII_EDID_BLOCK_SIZE]);
					if ( (bk1[4] >> 5) == 7 && bk1[5] == 0x78 ) {
						extensions = bk1[6];
						SII_LIB_LOG_DEBUG1(pObj, ("NOTE: encounter EEODB %d extensions !!\n", extensions));
						if ( extensions > 7 ) {
							extensions = 7;
						}
					}
				}
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 2) {
				ddcErr = sUpdateEdid( pObj, 1, 0, &(edid->b[SII_EDID_BLOCK_SIZE * 2]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 3) {
				ddcErr = sUpdateEdid( pObj, 1, SII_EDID_BLOCK_SIZE, &(edid->b[SII_EDID_BLOCK_SIZE * 3]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 4) {
				ddcErr = sUpdateEdid( pObj, 2, 0, &(edid->b[SII_EDID_BLOCK_SIZE * 4]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 5) {
				ddcErr = sUpdateEdid( pObj, 2, SII_EDID_BLOCK_SIZE, &(edid->b[SII_EDID_BLOCK_SIZE * 5]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 6) {
				ddcErr = sUpdateEdid( pObj, 3, 0, &(edid->b[SII_EDID_BLOCK_SIZE * 6]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
			if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR && extensions >= 7) {
				ddcErr = sUpdateEdid( pObj, 3, SII_EDID_BLOCK_SIZE, &(edid->b[SII_EDID_BLOCK_SIZE * 7]), (SII_EDID_BLOCK_SIZE), BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ);
			}
		}
	#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
		EdidSetExtBlkNum(extensions);
	#endif
	}
	return ddcErr;
}

static uint8_t sPlugStatus(HdmiTxObj_t* pObj)
{
	SiiInst_t		craInst		= sCraInstGet(pObj);
	uint8_t	sysStatus			= 0;
	sysStatus   = SiiDrvCraRdReg8(craInst, sBaseAddrGet(pObj) | REG_ADDR__TPI_HPD_RSEN);
	//printk(KERN_EMERG "plug status=%x\n",sysStatus);
	return sysStatus;
}

static SiiDrvDsHdcpVersion_t g_hdmi_hdcp_ver = SII_DRV_DS_HDCP_VER__NONE;
static void sPlugStatusUpdatePseudo(HdmiTxObj_t* pObj, uint8_t flg)
{
#define			EDID_RD_CNT_MAX			(3)
	bool_t          bHotPlug	= pObj->bHotPlug;
	SiiInst_t		craInst		= sCraInstGet(pObj);
	uint8_t extensions			= 0;
	uint8_t	sysStatus			= 0;
	SiiDdcComErr_t			ddcErr;
	SiiEdid_t			edid;
	bool_t				bSendUpdate;
	SiiLibEdidErrCode_t edidError;
	uint8_t hpd_mask = 0;
	uint8_t hpd_pair_check = 0;
	uint32_t rd_edid_timeout = EDID_RD_CNT_MAX;
	#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	EdidCapInit();
	#endif

	bSendUpdate = false;
	hpd_mask = ((BIT_MSK__TPI_HPD_RSEN__HPD_STATE & SII_BIT2) | BIT_MSK__TPI_HPD_RSEN__HPD);

	sysStatus   = SiiDrvCraRdReg8(craInst, sBaseAddrGet(pObj) | REG_ADDR__TPI_HPD_RSEN);
	SI_HDMI20_PRINT("\n0x363b=0x%x\n", sysStatus);

	#if 1
	hpd_pair_check  = (true == pObj->bHotPlug) \
					  && ((sysStatus & hpd_mask) == hpd_mask);//plug in twice, missed one unplug
	#endif

HDMI_PLUG_IN:
	if ((sysStatus & hpd_mask) == hpd_mask && hpd_pair_check == 0) {
		g_mthpd_state_bk = 7;
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG ASSERTED:%d\n", (uint32_t)pObj->bHotPlug));
		bHotPlug	= true;
		pObj->bRsen = true;

		sSetDDCManual(pObj, 0, 0, 0);
		SiiLibTimeMilliDelay(g_EDIDReadDelaynMs);
		SII_LIB_LOG_DEBUG1(pObj, ("Hotplug:%d,%d\n", (sysStatus & hpd_mask) == hpd_mask, g_EDIDReadDelaynMs));

	EDID_READ_AGAIN:
		// read EDID
		SII_MEMSET(&pObj->edid, 0, sizeof(SiiEdid_t));
		if ( pObj->bInitHotPlug == false ) {
			sSetHwTpiBit(pObj);
			sResetTpiStateMachine(pObj);
		}
		// read block# 0
		memset(&edid, 0, sizeof(SiiEdid_t));
		ddcErr = sReadEdid(pObj, (SiiEdid_t *)(&edid));

		if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR) {
			SII_LIB_LOG_DEBUG1(pObj, ("EDID has been read successfully!!\n"));
			if ( SII_MEMCMP(&pObj->edid, &edid, sizeof(SiiEdid_t)) ) {
				pObj->edid = edid;
				bSendUpdate = true;
			}

			sTxLog(&pObj->edid.b[0], (1 + extensions) * SII_EDID_BLOCK_SIZE);
#if __HDMI_OS_RTOS__
			memset(&g_hdmi_edid_result, 0, sizeof(hdmi_edid_result_t));
#endif

			edidError = sParseEdid(pObj);
#if __HDMI_OS_RTOS__
			g_hdmi_edid_result.edid_errcode = edidError;
#endif
			if ( edidError ) {
				SII_LIB_LOG_DEBUG1(pObj, ("sParseEdid error:%d\n", (uint32_t)edidError));
			}

			if (pObj->pConfig->bScdcEn) {
				//---- for testing : delete this ------------
				//pObj->parseEdid.scdc.bScdcPresent = 1;
				//pObj->parseEdid.scdc.bReadReqCapable = 1;
				//pObj->parseEdid.scdc.bLTE340MscsScramble =1;
				//pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
				//--------------------------------------------
				// Start SCDC module with the Sink Capablities read from Sink EDID

				if (pObj->parseEdid.scdc.bScdcPresent ) {
					if (pObj->parseEdid.scdc.vclk_mb >= 340) {
						pObj->parseEdid.maxTmds = pObj->parseEdid.scdc.vclk_mb * 1000000;
					}
					SII_LIB_LOG_DEBUG1(pObj, ("xxx %d,%d \n", pObj->parseEdid.scdc.vclk_mb, pObj->parseEdid.maxTmds));
				}
			}
			if (pObj->parseEdid.ieee_id == 0xC45DD8 || pObj->parseEdid.yuv420Vdb.size || pObj->parseEdid.bHfScdb) { /* HDMIForum VSDB */
				pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			} else if ( pObj->parseEdid.ieee_id == 0x000C03 ) { /* HDMI1.4 VSDB */
				pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
				//HDMI1 supports upto 300 MHz and HDMI2 supports upto 600 MHz
				//	if(pObj->parseEdid.maxTmds > 340000000)
				//		pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			} else {
				pObj->tmdsMode = SII_TMDS_MODE__DVI;
			}
#if __HDMI_OS_RTOS__
	 #if 1 //sym6 should check sync from sym4
			if(pObj->parseEdid.ieee_id == 0xC45DD8 || pObj->parseEdid.ieee_id == 0x000C03 || edidError != SII_LIB_EDID_ERR_CODE__NO_ERROR)
            {
                 //pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
                 g_hdmi_edid_result.is_hdmi = 1;
                 g_hdmi_edid_result.ycbcr444_supported = 1; //hdmi allways support ycbcr444.
            }
			else
			{
				pObj->tmdsMode = SII_TMDS_MODE__DVI;
                g_hdmi_edid_result.is_hdmi = 0;
			}
     #endif
#endif

			pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
		} else {
			//SII_LIB_LOG_DEBUG1(pObj, ("Error happened during EDID read!!"));
			SII_LIB_LOG_PRINT2(("Error happened during EDID read!!"));
			sTxLog(&(edid.b[0]), (1 + extensions) * SII_EDID_BLOCK_SIZE);
			if ( rd_edid_timeout-- ) {
				SII_LIB_LOG_DEBUG1(pObj, ("Read EDID again:delay=%dms start!!", g_EDIDReadDelaynMs * (EDID_RD_CNT_MAX - rd_edid_timeout + 1)));
				SiiLibTimeMilliDelay(g_EDIDReadDelaynMs * (EDID_RD_CNT_MAX - rd_edid_timeout + 1));
				goto EDID_READ_AGAIN;
			}
			sParseEdidErrInit(&pObj->parseEdid);
			pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
			pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
#if __HDMI_OS_RTOS__
			g_hdmi_edid_result.is_hdmi = 1;
            g_hdmi_edid_result.ycbcr444_supported = 1;
            g_hdmi_edid_result.edid_errcode = ddcErr;
#endif
		}

		if (pObj->pConfig->bScdcEn) {
			//---- for testing : detlete this ------------
			//pObj->parseEdid.scdc.bScdcPresent = 1;
			//pObj->parseEdid.scdc.bReadReqCapable = 1;
			//pObj->parseEdid.scdc.bLTE340MscsScramble =1;
			//pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			//--------------------------------------------
			// Start SCDC module with the Sink Capablities read from Sink EDID

			if (pObj->parseEdid.scdc.bScdcPresent) {
				pObj->parseEdid.scdc.vclk_mb = pObj->parseEdid.maxTmds / 1000000;
				SII_LIB_LOG_DEBUG1(pObj, ("xxx %d,%d \n", pObj->parseEdid.scdc.vclk_mb, pObj->parseEdid.maxTmds));
				//sScdcSinkCapsSet(pObj->scdcInst, &pObj->parseEdid.scdc);
				SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SINK_CAPABILITY, &pObj->parseEdid.scdc);
			}
		}
		memcpy(g_edata_back, edid.b, SII_EDID_MAX_LEN);
#if __HDMI_OS_RTOS__
		g_hdmi_edid_result.edid_parse_ready = MT_TRUE;
        //mta_hdmi_edid_result_dump();
#endif
		if ( flg ) {
			SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_50MS_WAIT, 0);
		}
	} else {
		sSetDDCManual(pObj, 1, 1, 1);
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG DE-ASSERTED:%d\n", (uint32_t)pObj->bHotPlug));
		bHotPlug	= false;
		pObj->bRsen = false;

		if (pObj->pConfig->bScdcEn) {
			if (pObj->parseEdid.scdc.bScdcPresent) {
				//sScdcReset(pObj->scdcInst);
				SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_RESET_CAPABILITY, NULL);

			}
		}

		//Clear Raw EDID Buffer and Parsed EDID Buffer
		SII_MEMSET(&pObj->edid, 0, sizeof(SiiEdid_t));
		SII_MEMSET(&pObj->parseEdid, 0, sizeof(SiiLibEdidPar_t));

		//TODO:: remove this later on when RX drivers updates TMDS mode appropriately
		#if ( 1 == __HDMI_UBOOT__ )
		pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
		#else
		pObj->tmdsMode = SII_TMDS_MODE__NONE;
		#endif
		pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;
		SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);

		//Stop HDCP immideatly
		pObj->eState = SII_MOD_TX_HDMI_STATUS__HDCP_OFF;
		pObj->bWasHdcpOn = false;
		sNotifyHdmiState(pObj);
#if __HDMI_OS_RTOS__
		g_hdmi_edid_result.edid_parse_ready = MT_FALSE;
#endif
	}

	if ((bHotPlug != pObj->bHotPlug) || bSendUpdate) {
		pObj->bHotPlug = bHotPlug;
		sUpdateHotPlugRsen(pObj);
	}
	SiiDrvTxHdcpVerGet(pObj->pConfig->instTx, &g_hdmi_hdcp_ver);
	if (hpd_pair_check) {
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG UNPAIR:%d\n", (uint32_t)pObj->bHotPlug));
		hpd_pair_check =  0;
		goto HDMI_PLUG_IN;
	}
}

static bool_t sPlugEdidDataCheck(HdmiTxObj_t* pObj, uint8_t *old, uint8_t *neww, uint16_t len)
{
	bool_t ret = true; //true : diff, false: same
	ret = memcmp(old, neww, len) != 0;
	if ( ret == false ) {
		SII_LIB_LOG_DEBUG1(pObj, ("Edid is the same as last\n"));
		SII_LIB_LOG_DEBUG1(pObj, ("Data:%x %x %x %x\n",old[0],old[1],old[2],old[127]));
	} else {
		SII_LIB_LOG_DEBUG1(pObj, ("Old:%x %x %x %x\n",old[0],old[1],old[2],old[127]));
		SII_LIB_LOG_DEBUG1(pObj, ("New:%x %x %x %x\n",neww[0],neww[1],neww[2],neww[127]));
	}
	return ret;
}
static bool_t sHdmiHdcpChgCheck(HdmiTxObj_t* pObj)
{
	bool_t ret = false;
	SiiDrvDsHdcpVersion_t hdcpverNew = SII_DRV_DS_HDCP_VER__NONE;
	SiiDrvTxHdcpCapGet(pObj->pConfig->instTx, &hdcpverNew); //Get Hdcp Status from hdcp module
	if ( g_hdmi_hdcp_ver != hdcpverNew ) {
		ret = true;
	} else {
		ret = false;
	}
	SII_LIB_LOG_DEBUG1(pObj, ("sHdmiHdcpChgCheck, hdcp old:%d,new,%d\n",g_hdmi_hdcp_ver,hdcpverNew));
	return ret;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Update HPD and RxSense status
//-------------------------------------------------------------------------------------------------
static void sPlugStatusUpdate(HdmiTxObj_t* pObj)
{
#define			EDID_RD_CNT_MAX			(3)
	bool_t          bHotPlug	= pObj->bHotPlug;
	SiiInst_t		craInst		= sCraInstGet(pObj);
	uint8_t extensions			= 0;
	uint8_t	sysStatus			= 0;
	SiiDdcComErr_t			ddcErr;
	SiiEdid_t			edid;
	bool_t				bSendUpdate;
	SiiLibEdidErrCode_t edidError;
	uint8_t hpd_mask = 0;
	uint8_t rsen_mask = 0;
	uint8_t do_hpd_process = 0;
	uint8_t hpd_pair_check = 0;
	uint32_t rd_edid_timeout = EDID_RD_CNT_MAX;
	uint8_t mthpd_state = 0;

	memset(&edid, 0, sizeof(SiiEdid_t));
	bSendUpdate = false;
	hpd_mask = ((BIT_MSK__TPI_HPD_RSEN__HPD_STATE & SII_BIT2) | BIT_MSK__TPI_HPD_RSEN__HPD);
	rsen_mask = ((BIT_MSK__TPI_HPD_RSEN__RSEN_STATE & SII_BIT6) | BIT_MSK__TPI_HPD_RSEN__RSEN);

	sysStatus   = SiiDrvCraRdReg8(craInst, sBaseAddrGet(pObj) | REG_ADDR__TPI_HPD_RSEN);
	mthpd_state = ((true == pObj->bHotPlug)<<2) | (((sysStatus&rsen_mask)==rsen_mask) << 1) | (((sysStatus&hpd_mask)==hpd_mask) << 0);
	SII_LIB_LOG_DEBUG1(pObj, ("sPlugStatusUpdate:%d->%d, sts:0x%x, hdcp %d,g_hdmi_hdcp_ver:%d\n", g_mthpd_state_bk,(uint32_t)mthpd_state,sysStatus,pObj->bIsHdcpOn,g_hdmi_hdcp_ver));
	switch (mthpd_state) {
		case 0:
		case 2:
#if __HDMI_OS_KERNEL__
			{
			extern void SiiTxMtHpdPatchCallBack(uint8_t sts);
			SiiTxMtHpdPatchCallBack(0);
			}
#endif
			g_mthpd_state_bk = mthpd_state;
			return;
			break;
		case 1:
#if __HDMI_OS_KERNEL__
			if(0){//do it at Hdmi_KThread_CEC
			extern void SiiTxMtHpdPatchCallBack(uint8_t sts);
			SiiTxMtHpdPatchCallBack(1);
			}
#endif
			g_mthpd_state_bk = mthpd_state;
			return;
			break;
		case 3:
			do_hpd_process = 3;
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
			EdidCapInit();
#endif
			break;
		case 4:
		case 6:
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
			EdidCapInit();
#endif
			do_hpd_process = 2;
			break;
		case 5:
			if ( g_mthpd_state_bk == 7 || g_mthpd_state_bk == 3) {
				extern void SiiTxMtHpdPatchCallBack(uint8_t sts);
				//SiiTxMtHpdPatchCallBack(1);
				g_mthpd_state_bk = mthpd_state;
				return;
			}
		case 7:
			{
				// double on
				uint8_t tout = 20;
				//SII_LIB_LOG_DEBUG1(pObj, ("HLine sts 1 1:%d\n", mtsi_hpd));
				do {
					ddcErr = sReadEdid(pObj, (SiiEdid_t *)(&edid));
					if ( ddcErr == SII_DDC_ERROR_CODE_TIMEOUT ) {
						SiiLibTimeMilliDelay(50);
					}
				} while (tout-- > 0 && ddcErr == SII_DDC_ERROR_CODE_TIMEOUT);
				if ( ddcErr == SII_DDC_ERROR_CODE_TIMEOUT ) {
					g_mthpd_state_bk = mthpd_state;
					return;
				}
				if ( sPlugEdidDataCheck(pObj, g_edata_back, edid.b, SII_EDID_MAX_LEN) ) {
					do_hpd_process = 2;
					//SII_LIB_LOG_DEBUG1(pObj, ("Edid change %x %x %x %x!!!\n",g_edata_back[1],g_edata_back[127],edid.b[1],edid.b[127]));
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
					EdidCapInit();
#endif
				} else {
					if ( sHdmiHdcpChgCheck(pObj) ) {
						do_hpd_process = 2;
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
						EdidCapInit();
#endif
					} else {
						if ( g_mthpd_state_bk == 5 ) {
							#if __HDMI_OS_KERNEL__
							extern void SiiTxMtHpdPatchCallBack(uint8_t sts);
							SiiTxMtHpdPatchCallBack(1);
							#endif
						}
						return;
					}
				}
			}
			break;
		default:
			//Error state
			break;
	}

	g_mthpd_state_bk = mthpd_state;
	#if 1
	hpd_pair_check  = (true == pObj->bHotPlug) \
					  && ((sysStatus & hpd_mask) == hpd_mask);//plug in twice, missed one unplug
	#endif

HDMI_PLUG_IN:
	//if ((sysStatus & hpd_mask) == hpd_mask && hpd_pair_check == 0 && (sysStatus & rsen_mask) == rsen_mask) {
	if ( do_hpd_process == 1 || do_hpd_process == 3 ) {
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG ASSERTED:%d\n", (uint32_t)pObj->bHotPlug));
		bHotPlug	= true;
		pObj->bRsen = true;

		sSetDDCManual(pObj, 0, 0, 0);
		SiiLibTimeMilliDelay(g_EDIDReadDelaynMs);
		SII_LIB_LOG_DEBUG1(pObj, ("Hotplug:%d,%d\n", (sysStatus & hpd_mask) == hpd_mask, g_EDIDReadDelaynMs));

	EDID_READ_AGAIN:
		// read EDID
		SII_MEMSET(&pObj->edid, 0, sizeof(SiiEdid_t));
		if ( pObj->bInitHotPlug == false ) {
			sSetHwTpiBit(pObj);
			sResetTpiStateMachine(pObj);
		}
		// read block# 0
		memset(&edid, 0, sizeof(SiiEdid_t));
		ddcErr = sReadEdid(pObj, (SiiEdid_t *)(&edid));
		if (ddcErr == SII_DDC_ERROR_CODE_NO_ERROR) {
			SII_LIB_LOG_DEBUG1(pObj, ("EDID has been read successfully!!\n"));
			if ( SII_MEMCMP(&pObj->edid, &edid, sizeof(SiiEdid_t)) ) {
				pObj->edid = edid;
				bSendUpdate = true;
			}

			sTxLog(&pObj->edid.b[0], (1 + extensions) * SII_EDID_BLOCK_SIZE);
#if __HDMI_OS_RTOS__
			memset(&g_hdmi_edid_result, 0, sizeof(hdmi_edid_result_t));
#endif

			edidError = sParseEdid(pObj);
#if __HDMI_OS_RTOS__
			g_hdmi_edid_result.edid_errcode = edidError;
#endif
			if ( edidError ) {
				SII_LIB_LOG_DEBUG1(pObj, ("sParseEdid error:%d\n", (uint32_t)edidError));
			}

			if (pObj->pConfig->bScdcEn) {
				//---- for testing : delete this ------------
				//pObj->parseEdid.scdc.bScdcPresent = 1;
				//pObj->parseEdid.scdc.bReadReqCapable = 1;
				//pObj->parseEdid.scdc.bLTE340MscsScramble =1;
				//pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
				//--------------------------------------------
				// Start SCDC module with the Sink Capablities read from Sink EDID

				if (pObj->parseEdid.scdc.bScdcPresent ) {
					if (pObj->parseEdid.scdc.vclk_mb >= 340) {
						pObj->parseEdid.maxTmds = pObj->parseEdid.scdc.vclk_mb * 1000000;
					} else if ( pObj->parseEdid.scdc.bDc30bit420 || pObj->parseEdid.scdc.bDc36bit420 || pObj->parseEdid.scdc.bDc48bit420 ) {
						pObj->parseEdid.maxTmds = 446 * 1000000;
					}
					SII_LIB_LOG_DEBUG1(pObj, ("xxx %d,%d \n", pObj->parseEdid.scdc.vclk_mb, pObj->parseEdid.maxTmds));
				}
			}
			if (pObj->parseEdid.ieee_id == 0xC45DD8 || pObj->parseEdid.yuv420Vdb.size || pObj->parseEdid.bHfScdb) { /* HDMIForum VSDB */
				pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			} else if ( pObj->parseEdid.ieee_id == 0x000C03 ) { /* HDMI1.4 VSDB */
				pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
				//HDMI1 supports upto 300 MHz and HDMI2 supports upto 600 MHz
				//	if(pObj->parseEdid.maxTmds > 340000000)
				//		pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			} else {
				pObj->tmdsMode = SII_TMDS_MODE__DVI;
			}
#if __HDMI_OS_RTOS__
	 #if 1 //sym6 should check sync from sym4
			if(pObj->parseEdid.ieee_id == 0xC45DD8 || pObj->parseEdid.ieee_id == 0x000C03 || edidError != SII_LIB_EDID_ERR_CODE__NO_ERROR)
            {
                 //pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
                 g_hdmi_edid_result.is_hdmi = 1;
                 g_hdmi_edid_result.ycbcr444_supported = 1; //hdmi allways support ycbcr444.
            }
			else
			{
				pObj->tmdsMode = SII_TMDS_MODE__DVI;
                g_hdmi_edid_result.is_hdmi = 0;
			}
     #endif
#endif

			pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
		} else {
			//SII_LIB_LOG_DEBUG1(pObj, ("Error happened during EDID read!!"));
			SII_LIB_LOG_PRINT2(("Error happened during EDID read!!"));
			sTxLog(&(edid.b[0]), (1 + extensions) * SII_EDID_BLOCK_SIZE);
			if ( rd_edid_timeout-- ) {
				SII_LIB_LOG_DEBUG1(pObj, ("Read EDID again:delay=%dms start!!", g_EDIDReadDelaynMs * (EDID_RD_CNT_MAX - rd_edid_timeout + 1)));
				SiiLibTimeMilliDelay(g_EDIDReadDelaynMs * (EDID_RD_CNT_MAX - rd_edid_timeout + 1));
				goto EDID_READ_AGAIN;
			}
			sParseEdidErrInit(&pObj->parseEdid);
			pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
			pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
#if __HDMI_OS_RTOS__
			g_hdmi_edid_result.is_hdmi = 1;
            g_hdmi_edid_result.ycbcr444_supported = 1;
            g_hdmi_edid_result.edid_errcode = ddcErr;
#endif
		}

		if (pObj->pConfig->bScdcEn) {
			//---- for testing : detlete this ------------
			//pObj->parseEdid.scdc.bScdcPresent = 1;
			//pObj->parseEdid.scdc.bReadReqCapable = 1;
			//pObj->parseEdid.scdc.bLTE340MscsScramble =1;
			//pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
			//--------------------------------------------
			// Start SCDC module with the Sink Capablities read from Sink EDID

			if (pObj->parseEdid.scdc.bScdcPresent) {
				pObj->parseEdid.scdc.vclk_mb = pObj->parseEdid.maxTmds / 1000000;
				SII_LIB_LOG_DEBUG1(pObj, ("xxx %d,%d \n", pObj->parseEdid.scdc.vclk_mb, pObj->parseEdid.maxTmds));
				//sScdcSinkCapsSet(pObj->scdcInst, &pObj->parseEdid.scdc);
				SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_SINK_CAPABILITY, &pObj->parseEdid.scdc);
			}
		}
		memcpy(g_edata_back, edid.b, SII_EDID_MAX_LEN);
#if __HDMI_OS_RTOS__
		g_hdmi_edid_result.edid_parse_ready = MT_TRUE;
        //mta_hdmi_edid_result_dump();
#endif
		SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_50MS_WAIT, 0);
	} else if ( do_hpd_process == 2 ) {
		sSetDDCManual(pObj, 1, 1, 1);
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG DE-ASSERTED:%d\n", (uint32_t)pObj->bHotPlug));
		bHotPlug	= false;
		pObj->bRsen = false;

		if (pObj->pConfig->bScdcEn) {
			if (pObj->parseEdid.scdc.bScdcPresent) {
				//sScdcReset(pObj->scdcInst);
				SiiModTxScdcSet(pObj->scdcInst, SII_MOD_TX_SCDC_OPCODE__SCDC_RESET_CAPABILITY, NULL);

			}
		}

		//Clear Raw EDID Buffer and Parsed EDID Buffer
		SII_MEMSET(&pObj->edid, 0, sizeof(SiiEdid_t));
		SII_MEMSET(&pObj->parseEdid, 0, sizeof(SiiLibEdidPar_t));

		//TODO:: remove this later on when RX drivers updates TMDS mode appropriately
		#if ( 1 == __HDMI_UBOOT__ )
		pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
		#else
		pObj->tmdsMode = SII_TMDS_MODE__NONE;
		#endif
		pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;
		SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);

		//Stop HDCP immideatly
		pObj->eState = SII_MOD_TX_HDMI_STATUS__HDCP_OFF;
		pObj->bWasHdcpOn = false;
		sNotifyHdmiState(pObj);
#if __HDMI_OS_RTOS__
		g_hdmi_edid_result.edid_parse_ready = MT_FALSE;
#endif
		memset(g_edata_back, 0, SII_EDID_MAX_LEN);
	}

	if ((bHotPlug != pObj->bHotPlug) || bSendUpdate) {
		pObj->bHotPlug = bHotPlug;
		//if (sPlugEdidDataCheck(pObj, g_edata_back, edid.b, SII_EDID_MAX_LEN) || pObj->bHotPlug == false ) {
		if ( 1 ) {
		sUpdateHotPlugRsen(pObj);
	}
	}
	SII_LIB_LOG_DEBUG1(pObj, ("HPD :%d, 0x%x e[128]=0x%x\n", (uint32_t)pObj->bHotPlug,g_edata_hpdren,edid.b[127]));
	SiiDrvTxHdcpVerGet(pObj->pConfig->instTx, &g_hdmi_hdcp_ver);
	SII_LIB_LOG_DEBUG1(pObj, ("g_hdmi_hdcp_ver:%d\n", (uint32_t)g_hdmi_hdcp_ver));
	if (hpd_pair_check) {
		SII_LIB_LOG_DEBUG1(pObj, ("HOT PLUG UNPAIR:%d 0x%x\n", (uint32_t)pObj->bHotPlug,sysStatus));
		if ((sysStatus & 0x55) == 0x55 && pObj->bHotPlug == false) {
			hpd_pair_check =  0;
			do_hpd_process = 1;
			SII_LIB_LOG_DEBUG1(pObj, ("UUU IN\n"));
			goto HDMI_PLUG_IN;
		} else if (pObj->bHotPlug == true) {
			hpd_pair_check =  0;
			do_hpd_process = 2;
			SII_LIB_LOG_DEBUG1(pObj, ("UUU OUT\n"));
			goto HDMI_PLUG_IN;
	}
}
}

static SiiDdcComErr_t sUpdateEdidErrCheck(HdmiTxObj_t* pObj, uint8_t segmentIndex, uint8_t regAddr, uint8_t *pBuf, uint16_t length)
{
#define EDID_DDC_CHECK_BYTES	(16)
	SiiDdcComErr_t	dsDdcError  = SII_DDC_ERROR_CODE_NO_ERROR;
	uint8_t i;

	if ( segmentIndex > 3 || (regAddr != 0 && regAddr != SII_EDID_BLOCK_SIZE) || (length != SII_EDID_BLOCK_SIZE && length != 0) ) {
		dsDdcError = SII_DDC_ERROR_CODE_NO_ERROR;
	}
	for (i = 0; i < EDID_DDC_CHECK_BYTES; i++) {
		if ((*(pBuf - i - 1)) != 0xFF) {
			break;
		}
	}
	if ( i == EDID_DDC_CHECK_BYTES ) {
		dsDdcError = SII_DDC_ERROR_CODE_TIMEOUT;
	}
	return dsDdcError;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read from downstream DDC device.
//!
//!             The function can be used for downstream EDID or HDCP DDC reading.
//!
//! @param[in]  segmentIndex - EDID segment number; 0 for HDCP DDC
//! @param[in]  regAddr      - HDCP register offset or EDID data offset
//! @param[out] pBuf         - pointer to the buffer
//! @param[in]  length       - number of bytes to read
//!
//! @return     Error code, @see SiiDdcComErr_t
//-------------------------------------------------------------------------------------------------
static SiiDdcComErr_t sUpdateEdid(HdmiTxObj_t* pObj, uint8_t segmentIndex, uint8_t regAddr, uint8_t *pBuf, uint16_t length, uint16_t mode)
{
	SiiDrvCraAddr_t	baseAddr	= sBaseAddrGet(pObj);
	SiiInst_t		craInst		= sCraInstGet(pObj);
	SiiDdcComErr_t	dsDdcError  = SII_DDC_ERROR_CODE_NO_ERROR;
	uint16_t        fifoSize;
	uint16_t        timeOutMs = 0;
	bool_t			loop = false;
	uint32_t        timego = 0;
	uint32_t        timego1 = 0;

	do {
		if ( length == 0 ) {
			break;
		}

		if ( !pBuf ) {
			break;
		}

		if (!sWaitForDdcBus(pObj)) {
			SII_LIB_LOG_DEBUG1(pObj, ("DDC Error:: Busy. Try Later\n"));
			dsDdcError = SII_DDC_ERROR_CODE_BUSY;
			return dsDdcError;
		}

		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
		if ( SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
			SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
		}
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_ADDR, (0xFF != segmentIndex) ? 0xA0 : 0x74 );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_SEGM, (0xFF != segmentIndex) ? segmentIndex : 0x00 );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_OFFSET, regAddr);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT2, ((length >> 8) & 0xFF) );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT1,  (length & 0xFF) );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, segmentIndex ? BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ : BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, mode);

		timeOutMs = length + 3; // timeout is proportional to length

		// wait until the FIFO is filled with several bytes
		SiiLibTimeMilliDelay(2); // also makes time aligning

		do {
			fifoSize = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__DDC_DOUT_CNT) & BIT_MSK__DDC_DOUT_CNT__DDC_DATA_OUT_CNT;

			if ( fifoSize ) {
				timeOutMs = length + 20;
				// if the FIFO has some bytes
				if ( fifoSize > length ) {
					SII_LIB_LOG_DEBUG1(pObj, ("DDC Error: FIFO Size Exceeded Length\n"));
					dsDdcError = SII_DDC_ERROR_CODE_TX_HW;
					break;
				} else if ( fifoSize > LEN_TPI__DDC_FIFO_SIZE ) {
					SII_LIB_LOG_DEBUG1(pObj, ("DDC Error: FIFO Size Exceeded Max FIFO Size\n"));
					dsDdcError = SII_DDC_ERROR_CODE_LIM_EXCEED;
					break;
				} else {
					// read fifo_size bytes
					SiiDrvCraFifoRead8(craInst, baseAddr | REG_ADDR__DDC_DATA, pBuf, fifoSize);
					length -= fifoSize;
					pBuf += fifoSize;
				}
			} else {
				//SII_LIB_LOG_DEBUG1(pObj, ("DDC Error: FifoEmpty%d_%d +++\n",timeOutMs,length));
				timego = SiiLibTimeMilliGet();
				SiiLibTimeMilliDelay(1); // note, the time is aligned
				timego1 = SiiLibTimeMilliGet();
				if ( timego1 > timego ) {
					timego = timego1 - timego;
				} else {
					timego = 1;
				}
				//SII_LIB_LOG_DEBUG1(pObj, ("DDC Error: FifoEmpty%d_%d_%d ---\n",timeOutMs,length,timego));
				if ( timeOutMs > timego ) {
					timeOutMs -= timego;
				} else {
					timeOutMs = 0;
				}
			}
		} while (length && timeOutMs);

		if ( dsDdcError ) {
			break;
		}

		if ( 0 == timeOutMs ) {
			SII_LIB_LOG_DEBUG1(pObj, ("DDC Error:: Timedout:%d\n", (uint32_t)segmentIndex));
			dsDdcError = SII_DDC_ERROR_CODE_TIMEOUT;
		}
	} while (loop);

	if ( dsDdcError ) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION);
	} else {
		dsDdcError = sUpdateEdidErrCheck(pObj, segmentIndex, regAddr, pBuf, length);
	}

	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	//SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__DDC_CMD, 0x7F);

	return dsDdcError;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Wait for DS DDC operation to finish.
//-------------------------------------------------------------------------------------------------
static bool_t sWaitForDdcBus(HdmiTxObj_t* pObj)
{
	uint8_t          val;
	SiiDrvCraAddr_t  baseAddr	= sBaseAddrGet(pObj);
	SiiInst_t		 craInst	= sCraInstGet(pObj);
	uint8_t          time_out   = LEN_TPI__DDC_FIFO_SIZE + 1;

	// time_out is time in ms, which is proportional to the FIFO size.
	// Since the time required to transmit one byte is 100 uS, the time_out
	// is as much as 10 times longer. +1 is additional time due to SI_Sleep() function precision
	// allows error +0-1 ms.
	do {
		val = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__DDC_STATUS) & BIT_MSK__DDC_STATUS__DDC_I2C_IN_PROG;

		if (0 == val) {
			return true;
		} else {
			SiiLibTimeMilliDelay(1);
		}
	} while (--time_out);

	return false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Parse Edid after reading edid.
//!
//!             The function can be used for parsing downstream EDID.
//!
//! @return     Error code, @see SiiLibEdidErrCode_t
//-------------------------------------------------------------------------------------------------
static SiiLibEdidErrCode_t sParseEdid(HdmiTxObj_t* pObj)
{
	return SiiLibEdidParse(&pObj->parseEdid, (const SiiLibEdidRaw_t*)&pObj->edid);
}

static void sApplyInfoFrame(HdmiTxObj_t* pObj, uint8_t bApply)
{
	SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);
	SiiInst_t		craInst	 = sCraInstGet(pObj);

	if (bApply) {
		//start inforframes
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_EN, 0xC0, 0xC0);
	} else {
		//stop inforframes
		//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_EN, 0xC0, 0x00);
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_RPT);
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_EN, BIT_MSK__TPI_INFO_EN__REG_TPI_INFO_EN);
	}
}

static void sInfoframesSet(HdmiTxObj_t* pObj)
{
	SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);
	SiiInst_t		craInst	 = sCraInstGet(pObj);

	//AVI Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AVI);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifAvi.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnAvi);

	//AUDIO Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__AUDIO);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifAudio.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnAudio);

	//VSIF Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__VSIF);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifVs.b, LEN_TPI__IF_BUFFER_LENGTH);
	//vsif ctrl
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__VSIF_CTRL, BIT_MSK_REG__VSIF_WR_DONE, BIT_MSK_REG__VSIF_WR_DONE);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnVs);

	//SPD Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__SPD);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifSpd.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnSpd);

	//GBD Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GBD);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifGbd.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnGbd);

	//MPEG Infoframe
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__MPEG);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifMpeg.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnMpeg);

	//HDR Infoframe
	//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN4);
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN3);
	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifHdr.b, LEN_TPI__IF_BUFFER_LENGTH);
	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnHdr);

	/*
	// TODO:: We need to understand how to use these buffers, they seem to create problems with DDC line

	// use GEN1 buffer for ISRC
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN1);

	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifIsrc.b, LEN_TPI__IF_BUFFER_LENGTH);

	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnIsrc);

	// use GEN1 buffer for ISRC2
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN2);

	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifIsrc2.b, LEN_TPI__IF_BUFFER_LENGTH);

	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnIsrc2);

	// use GEN3 buffer for ACP
	txCraTpiAipPutBit8(pObj->priBaseAddr, REG_ADDR__TPI_INFO_FSEL, BIT_MSK__TPI_INFO_FSEL__REG_TPI_INFO_SEL, BIT_ENUM__TPI_INFO_FSEL__GEN3);

	// InfoFrame Data Update
	SiiDrvCraBlockWrite8(craInst, baseAddr | REG_ADDR__TPI_INFO_B0, (uint8_t*)pObj->ifAcp.b, LEN_TPI__IF_BUFFER_LENGTH);

	//make the changes effective
	sApplyInfoFrame(pObj, pObj->bIfOnAcp);
	*/
}

static void sTxHdmiScdcCallBack(SiiInst_t inst, uint32_t scdcEvents)
{
	HdmiTxObj_t *pObj = (HdmiTxObj_t *)SII_LIB_OBJ_PNTR(inst);
	pObj->tx_scdc_events = scdcEvents;
	pObj->tx_scdc_events |= SII_DRV_TX_EVENT__SCDC_EVENT;
}

static void sInfoframeTypeSet(HdmiTxObj_t* pObj, SiiInfoFrame_t *pInfoFrame)
{
	uint8_t *ptr = (uint8_t*)pInfoFrame;

	switch (pInfoFrame->ifId) {
		case SII_INFO_FRAME_ID__AVI:
			SII_MEMCPY(&pObj->ifAvi, pInfoFrame, sizeof(SiiInfoFrame_t));
			SII_LIB_LOG_DEBUG2(("IF-AVI	:"));
			break;

		case SII_INFO_FRAME_ID__AUDIO:
			SII_MEMCPY(&pObj->ifAudio, pInfoFrame, sizeof(pObj->ifAudio));
			SII_LIB_LOG_DEBUG2(("IF-AUDIO	:"));
			break;

		case SII_INFO_FRAME_ID__VS:
			SII_MEMCPY(&pObj->ifVs, pInfoFrame, sizeof(pObj->ifVs));
			SII_LIB_LOG_DEBUG2(("IF-VS	:"));
			break;

		case SII_INFO_FRAME_ID__SPD:
			SII_MEMCPY(&pObj->ifSpd, pInfoFrame, sizeof(pObj->ifSpd));
			SII_LIB_LOG_DEBUG2(("IF-SPD	:"));
			break;

		case SII_INFO_FRAME_ID__GBD:
			SII_MEMCPY(&pObj->ifGbd, pInfoFrame, sizeof(pObj->ifGbd));
			SII_LIB_LOG_DEBUG2(("IF-GBD	:"));
			break;

		case SII_INFO_FRAME_ID__MPEG:
			SII_MEMCPY(&pObj->ifMpeg, pInfoFrame, sizeof(pObj->ifMpeg));
			SII_LIB_LOG_DEBUG2(("IF-MPEG	:"));
			break;

		case SII_INFO_FRAME_ID__ISRC:
			SII_MEMCPY(&pObj->ifIsrc, pInfoFrame, sizeof(pObj->ifIsrc));
			SII_LIB_LOG_DEBUG2(("IF-ISRC	:"));
			break;

		case SII_INFO_FRAME_ID__ISRC2:
			SII_MEMCPY(&pObj->ifIsrc2, pInfoFrame, sizeof(pObj->ifIsrc2));
			SII_LIB_LOG_DEBUG2(("IF-ISRC2	:"));
			break;

		case SII_INFO_FRAME_ID__ACP:
			SII_MEMCPY(&pObj->ifAcp, pInfoFrame, sizeof(pObj->ifAcp));
			SII_LIB_LOG_DEBUG2(("IF-ACP	:"));
			break;

		case SII_INFO_FRAME_ID__HDR:
			SII_MEMCPY(&pObj->ifHdr, pInfoFrame, sizeof(pObj->ifHdr));
			SII_LIB_LOG_DEBUG2(("IF-HDR	:"));
			break;
		default :
			break;
	}
	sTxLog((uint8_t*)++ptr, SII_INFOFRAME_MAX_LEN);
	SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
}

static void sInfoframeOnOffSet(HdmiTxObj_t* pObj, SiiInfoFrameId_t ifId, uint8_t onOff)
{
	switch (ifId) {
		case SII_INFO_FRAME_ID__AVI:
			pObj->bIfOnAvi = onOff;
			break;

		case SII_INFO_FRAME_ID__AUDIO  :
			pObj->bIfOnAudio = onOff;
			break;

		case SII_INFO_FRAME_ID__VS     :
			pObj->bIfOnVs = onOff;
			break;

		case SII_INFO_FRAME_ID__SPD    :
			pObj->bIfOnSpd = onOff;
			break;

		case SII_INFO_FRAME_ID__GBD    :
			pObj->bIfOnGbd = onOff;
			break;

		case SII_INFO_FRAME_ID__ISRC   :
			pObj->bIfOnIsrc = onOff;
			break;

		case SII_INFO_FRAME_ID__ISRC2  :
			pObj->bIfOnIsrc2 = onOff;
			break;

		case SII_INFO_FRAME_ID__ACP    :
			pObj->bIfOnAcp = onOff;
			break;

		case SII_INFO_FRAME_ID__MPEG:
			pObj->bIfOnMpeg = onOff;
			break;

		case SII_INFO_FRAME_ID__HDR:
			pObj->bIfOnHdr = onOff;
			break;

		default :
			break;
	}
	SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
}
static void sUpdateInfoframes(HdmiTxObj_t* pObj)
{
	sInfoframesSet(pObj);
}

static void sAudioSetChannelSts(HdmiTxObj_t* pObj)
{
	//60958-3
	if (pObj->audioFormat.i2s && pObj->audioFormat.hbrA == 0) {
		pObj->channelStatus.i2s_chst0 = 0x4;
		pObj->channelStatus.i2s_chst1 = 0x01;
	} else if (pObj->audioFormat.spdif || pObj->audioFormat.hbrA) {
		pObj->channelStatus.i2s_chst0 = 0x6;
		pObj->channelStatus.i2s_chst1 = 0x19;
	} else {
		pObj->channelStatus.i2s_chst0 = 0x4;
		pObj->channelStatus.i2s_chst1 = 0x01;
	}
	pObj->channelStatus.i2s_chst2 = 0x00;
	//pObj->channelStatus.i2s_chst3 = 0x00;
	if (pObj->audioFormat.hbrA) {
		pObj->channelStatus.i2s_chst3 = 0x09;
		pObj->channelStatus.i2s_chst4 = 0xE2;
	} else {
		mt_u32 o_fs = 0;
		pObj->channelStatus.i2s_chst4 = 0x02;
		o_fs = 16 - (pObj->channelStatus.i2s_chst3 & 0xf);
		pObj->channelStatus.i2s_chst4 |= ((o_fs & 0xf) << 4);
	}
	pObj->channelStatus.i2s_chst5 = 0x01;
	pObj->channelStatus.i2s_chst6 = 0x00;
	//SII_LIB_LOG_DEBUG1(pObj, ("Setting Channel status:%d %d\n",pObj->audioFormat.i2s,pObj->audioFormat.hbrA));
}

static void sAudioformatSet(HdmiTxObj_t* pObj)
{
	SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);
	SiiInst_t		craInst	 = sCraInstGet(pObj);
	uint8_t		audiFsMask = 0;

	switch (pObj->audioFormat.audioFs) {
		case SII_AUDIO_FS__22_05KHZ:
			audiFsMask = 0x04;
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x04);
			break;

		case SII_AUDIO_FS__24KHZ:
			audiFsMask = 0x06;
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x06);
			break;

		case SII_AUDIO_FS__32KHZ:
			audiFsMask = 0x03;
			if (pObj->audioFormat.hbrA) { //128
				audiFsMask = 0x2b;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x03);
			break;

		case SII_AUDIO_FS__44_1KHZ:
			audiFsMask = 0x00;
			if (pObj->audioFormat.hbrA) { //176.4
				audiFsMask = 0x0C;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x00);
			break;

		case SII_AUDIO_FS__48KHZ:
			audiFsMask = 0x02;
			if (pObj->audioFormat.hbrA) { //192
				audiFsMask = 0x0E;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x02);
			break;

		case SII_AUDIO_FS__88_2KHZ:
			audiFsMask = 0x08;
			if (pObj->audioFormat.hbrA) { //352.8
				audiFsMask = 0x0d;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x08);
			break;

		case SII_AUDIO_FS__96KHZ:
			audiFsMask = 0x0A;
			if (pObj->audioFormat.hbrA) { //384
				audiFsMask = 0x05;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x0A);
			break;

		case SII_AUDIO_FS__176_4KHZ:
			audiFsMask = 0x0C;
			if (pObj->audioFormat.hbrA) { //705.6
				audiFsMask = 0x2d;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x0C);
			break;

		case SII_AUDIO_FS__192KHZ:
			audiFsMask = 0x0E;
			if (pObj->audioFormat.hbrA) { //768
				audiFsMask = 0x09;
			}
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x0E);
			break;

		case SII_AUDIO_FS__768KHZ:
			audiFsMask = 0x09;
			//SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x0F, 0x09);
			break;
		case SII_AUDIO_FS__64KHZ:
			audiFsMask = 0x0b;
			break;
		case SII_AUDIO_FS__128KHZ:
			audiFsMask = 0x2b;
			break;

		default :
			break;
	}
	SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, 0x3F, audiFsMask);

	switch (pObj->audioFormat.layout1) {
		case AUDIO_FORMAT__2CH:
			//txCraTpiAipClrBit8(baseAddr, REG_ADDR__I2S_CHST3, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
			break;
		case AUDIO_FORMAT__3CH:
		case AUDIO_FORMAT__4CH:
		case AUDIO_FORMAT__5CH:
		case AUDIO_FORMAT__6CH:
		case AUDIO_FORMAT__7CH:
		case AUDIO_FORMAT__8CH:
			//txCraTpiAipSetBit8(baseAddr, REG_ADDR__I2S_CHST3, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
			SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
			break;
		default :
			break;
	}

	if (pObj->audioFormat.i2s) {
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, SII_BIT1);                // enable spdif
		pObj->channelStatus.i2s_chst3 = audiFsMask;

		switch (pObj->audioFormat.layout1) {
			case AUDIO_FORMAT__2CH:
				SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, SII_BIT4);
				break;
			case AUDIO_FORMAT__3CH:
			case AUDIO_FORMAT__4CH:
				SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, (SII_BIT5 | SII_BIT4) );
				break;
			case AUDIO_FORMAT__5CH:
			case AUDIO_FORMAT__6CH:
				SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, (SII_BIT6 | SII_BIT5 | SII_BIT4) );
				break;
			case AUDIO_FORMAT__7CH:
			case AUDIO_FORMAT__8CH:
				SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, (SII_BIT7 | SII_BIT6 | SII_BIT5 | SII_BIT4) );
				break;
			default :
				break;
		}
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__I2S_IN_CTRL, 0x70, BIT_MSK__I2S_IN_CTRL__REG_SCK_EDGE);

		if (pObj->audioFormat.hbrA) {
			SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, SII_BIT2, SII_BIT2); // enable hbrA
			SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__I2S_IN_CTRL, 0x70, \
							 BIT_MSK__I2S_IN_CTRL__REG_SCK_EDGE | BIT_MSK__I2S_IN_CTRL__REG_CBIT_ORDER | BIT_MSK__I2S_IN_CTRL__REG_VBIT);
			\
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
		}
	} else if (pObj->audioFormat.dsd) {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, SII_BIT3);                // enable dsd
	} else if (pObj->audioFormat.hbrA) {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, SII_BIT2);                // enable hbrA
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__I2S_IN_CTRL, 0x70, \
						 BIT_MSK__I2S_IN_CTRL__REG_SCK_EDGE | BIT_MSK__I2S_IN_CTRL__REG_CBIT_ORDER | BIT_MSK__I2S_IN_CTRL__REG_VBIT);
		\
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_LAYOUT);
	} else if (pObj->audioFormat.spdif) {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, SII_BIT1);                // enable spdif
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__I2S_IN_CTRL, 0x70, BIT_MSK__I2S_IN_CTRL__REG_SCK_EDGE | BIT_MSK__I2S_IN_CTRL__REG_VBIT);
	} else {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__AUD_MODE, 0xFF, 0x00);                    // enable spdiff/pkt pass thru
	}

	// Enable/Disable Down sampling
	if (pObj->audioFormat.downSample) {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_DOWN_SMPL_CTRL, BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUD_HNDL, BIT_ENUM__TPI_DOWN_SMPL_CTRL__AUDIO_DOWNSAMP_EN );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_AUD_FS, CLEAR_BITS);
	} else {
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_DOWN_SMPL_CTRL, BIT_MSK__TPI_DOWN_SMPL_CTRL__REG_TPI_AUD_HNDL, BIT_ENUM__TPI_DOWN_SMPL_CTRL__AUDIO_PASS_Fs );
	}

	sAudioSetChannelSts(pObj);
}

static void sUpdateAudioformat(HdmiTxObj_t* pObj)
{
	sAudioformatSet(pObj);
}

static void sChannelStateSet(HdmiTxObj_t* pObj)
{
	SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);
	SiiInst_t		craInst	 = sCraInstGet(pObj);

	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST0, pObj->channelStatus.i2s_chst0);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST1, pObj->channelStatus.i2s_chst1);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST2, pObj->channelStatus.i2s_chst2);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST3, pObj->channelStatus.i2s_chst3);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST4, pObj->channelStatus.i2s_chst4);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST6, pObj->channelStatus.i2s_chst5);
	SiiDrvCraWrReg8(craInst, baseAddr |  REG_ADDR__I2S_CHST7, pObj->channelStatus.i2s_chst6);
}

static void sUpdateChannelState(HdmiTxObj_t* pObj)
{
	sChannelStateSet(pObj);
}

static void sPutTMDSOnOff(HdmiTxObj_t* pObj, bool_t onOff)
{
#if 0//__HDMI_OS_RTOS__
     #define  HDMI_PHY_MUTE_REG               SYMPHONY_IO_VA(0xbf5d01c4)  // hdmi tx reg2

      SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);

	  SiiInst_t		craInst	 = sCraInstGet(pObj);

      u32 phy_mute_reg_data = 0;

      if(onOff)
      {
        phy_mute_reg_data = hal_get_u32(HDMI_PHY_MUTE_REG);
        phy_mute_reg_data  &= 0x0fffffff;
        hal_put_u32(HDMI_PHY_MUTE_REG, phy_mute_reg_data);
      }

      // temp cfg the avi info data
      //SiiDrvCraWrReg32(craInst, baseAddr | 0x60c,  0x00a85063);
      //SiiDrvCraWrReg32(craInst, baseAddr | 0x610,  0x00000014);
      //SiiDrvCraWrReg32(craInst, baseAddr | 0x618,  0xb2010000);
#endif
	return;
}

static void sPutTMDSMute(HdmiTxObj_t* pObj, bool_t mute)
{
	#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#if defined(CONFIG_MT_CHIP_ETUDE2)
	uint32_t val[4];
	int32_t ret;

	SII_LIB_LOG_DEBUG1(pObj, ("Setting TMDS MUTE:: %s\n", mute ? "ON" : "OFF"));
	memset(val, 0, sizeof(val));
	ret = 0;
	if (FALSE == mute) {
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d006cUL), &(val[0]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d0244UL), &(val[1]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d0248UL), &(val[2]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d024cUL), &(val[3]));
		if ( (val[0] & (1 << 24)) || (val[1] & (1 << 24)) || (val[2] & (1 << 24)) || (val[3] & (1 << 8)) ) {
			val[0] &= ~(1 << 24);
			val[0] &= ~(3 << 22);

			val[1] &= ~(1 << 24);
			val[1] &= ~(3 << 22);

			val[2] &= ~(1 << 24);
			val[2] &= ~(3 << 22);

			val[3] &= ~(1 << 8);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d006cUL), val[0]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d0244UL), val[1]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d0248UL), val[2]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d024cUL), val[3]);
		}
	} else {
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d006cUL), &(val[0]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d0244UL), &(val[1]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d0248UL), &(val[2]));
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0x1f5d024cUL), &(val[3]));
		if ( !((val[0] & (1 << 24)) && (val[1] & (1 << 24)) && (val[2] & (1 << 24)) && (val[3] & (1 << 8))) ) {
			val[0] |= (1 << 24);
			val[0] &= ~(3 << 22);
			val[0] |= (2 << 22); //constant 0.  0b0x:data, 0b10:0, 0b11:1

			val[1] |= (1 << 24);
			val[1] &= ~(3 << 22);
			val[1] |= (2 << 22); //constant 0.  0b0x:data, 0b10:0, 0b11:1

			val[2] |= (1 << 24);
			val[2] &= ~(3 << 22);
			val[2] |= (2 << 22); //constant 0.  0b0x:data, 0b10:0, 0b11:1

			val[3] |= (1 << 8);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d006cUL), val[0]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d0244UL), val[1]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d0248UL), val[2]);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0x1f5d024cUL), val[3]);
		}
	}
	#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	uint32_t val;
	int32_t ret;

	SII_LIB_LOG_DEBUG1(pObj, ("Setting TMDS MUTE:: %s\n", mute ? "ON" : "OFF"));
	ret = 0;
	if (FALSE == mute) {
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d01c4UL), &val);
		if ( val & ((0xf << 28) | (0x3 << 22))) {
			val &= ~(0xf << 28);
			val &= ~(3 << 22);
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0xbf5d01c4UL), val);
		}
	} else {
		ret = mt_sys_read_register(SYMPHONY_IO_PA(0xbf5d01c4UL), &val);
		if ( (val & (0xf << 28)) != (0xf << 28) || (val & (3 << 22)) != (2 << 22)) {
			val |= (0xf << 28);
			val &= ~(3 << 22);
			val |= (2 << 22); //constant 0.  0b0x:data, 0b10:0, 0b11:1
			ret = mt_sys_write_register(SYMPHONY_IO_PA(0xbf5d01c4UL), val);
		}
	}
	#else
	int32_t ret = 0;
	#endif
	if (ret ) {
		return;
	}
	#endif
}

static void sUpdateHdmiMode(HdmiTxObj_t* pObj)
{
	SiiDrvCraAddr_t baseAddr = sBaseAddrGet(pObj);
	SiiInst_t		craInst	 = sCraInstGet(pObj);
	bool_t tmdsOn = true;
	bool_t updateDone = true;

	do {
		SII_LIB_LOG_DEBUG1(pObj, ("sUpdateHdmiMode:: Setting TMDS mode: %02X\n", pObj->tmdsMode));
		updateDone = true;

		switch (pObj->tmdsMode) {
			case SII_TMDS_MODE__NONE:
				tmdsOn = false;
				break;

			case SII_TMDS_MODE__AUTO:

				if ((pObj->parseEdid.ieee_id == 0x000C03)/* HDMI1.4 VSDB */ || (pObj->parseEdid.ieee_id == 0xC45DD8)/* HDMIForum VSDB */) {
					pObj->tmdsMode = SII_TMDS_MODE__HDMI1;
					//HDMI1 supports upto 300 MHz and HDMI2 supports upto 600 MHz
					if (pObj->parseEdid.maxTmds > 300000000 || pObj->parseEdid.yuv420Vdb.size || pObj->parseEdid.ieee_id == 0xC45DD8) {
						pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
					}
				} else if (pObj->parseEdid.bHfScdb) {
					pObj->tmdsMode = SII_TMDS_MODE__HDMI2;
				} else {
					pObj->tmdsMode = SII_TMDS_MODE__DVI;
				}

				updateDone = false;

				break;

			case SII_TMDS_MODE__HDMI2:
				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_HDMI2_ON);

				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0);
				break;
			case SII_TMDS_MODE__HDMI1:
				SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_HDMI2_ON);

				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0);
				break;
			case SII_TMDS_MODE__DVI:
				SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_OUTPUT_MODE_B0);
				break;

			default:
				tmdsOn = false;
				break;
		}

		sPutTMDSOnOff(pObj, tmdsOn);

	} while (false == updateDone);

	return;
}

static void sTmdsModeSet(HdmiTxObj_t* pObj)
{
	SII_LIB_LOG_DEBUG1(pObj, ("Setting TMDS mode to: %02X", pObj->tmdsMode));
	if (pObj->tmdsMode) {
		pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_ON;
	} else {
		pObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;
	}
	SiiLibSeqTimerStart(pObj->timerHwUpdate, TIMER_START__HW_UPDATE_5MS_WAIT, 0);
}

static void sUpdateHotPlugRsen(HdmiTxObj_t* pObj)
{
	if (pObj->cbFunc) {
		pObj->cbFunc(pObj->pConfig->instTx, SII_DRV_TX_EVENT__HOT_PLUG_CHNG | SII_DRV_TX_EVENT__RSEN_CHNG);
	}
}

static void sOutputBitDepthSet(HdmiTxObj_t* pObj, SiiDrvCraAddr_t baseAddr)
{
	uint8_t reg = 0;
	switch (pObj->outputBitDepth) {
		case SII_DRV_BIT_DEPTH__8_BIT:
			reg = 0x00;
			break;
		case SII_DRV_BIT_DEPTH__10_BIT:
			reg = 0x81;
			break;
		case SII_DRV_BIT_DEPTH__12_BIT:
			reg = 0x82;
			break;
		case SII_DRV_BIT_DEPTH__16_BIT:
			reg = 0x83;
			break;
		default:
			reg = 0x82;
			break;
	}
	//SiiDrvCraPutBit8(pObj->pConfig->instTxCra,  baseAddr | REG_ADDR__P2T_CTRL, (uint8_t)BIT_MSK__P2T_CTRL__REG_PACK_MODE, (uint8_t)(pObj->outputBitDepth - 1));
	SiiDrvCraPutBit8(pObj->pConfig->instTxCra, baseAddr | REG_ADDR__P2T_CTRL, (uint8_t)BIT_MSK__P2T_CTRL__REG_PACK_MODE | BIT_MSK__P2T_CTRL__REG_DC_PKT_EN, reg);
}

static void sUpdateOutputBitDepth(HdmiTxObj_t* pObj)
{
	if (SII_DRV_BIT_DEPTH__PASSTHOUGH != pObj->outputBitDepth) {
		SII_LIB_LOG_DEBUG1(pObj, ("Setting Output Bit Depth to: %02X\n", (pObj->outputBitDepth - 1)));
		sOutputBitDepthSet(pObj, sBaseAddrGet(pObj));
	}
}
static SiiDrvCraAddr_t sBaseAddrGet(HdmiTxObj_t *pObj)
{
	return pObj->pConfig->baseAddrTx;
}

static SiiInst_t sCraInstGet(HdmiTxObj_t *pObj)
{
	return pObj->pConfig->instTxCra;
}

static void sTxLog(uint8_t *pData, uint16_t len)
{
	int i = 0;
	SII_LIB_LOG_DEBUG2(("\n"));
	while (len--) {
		SII_LIB_LOG_DEBUG2((" %02X", *pData));
		pData++;
		if (++i == 0x10) {
			SII_LIB_LOG_DEBUG2(("\n"));
			i = 0;
		}

	}
	SII_LIB_LOG_DEBUG2(("\n"));
}

static void sNotifyHdmiState(HdmiTxObj_t *pObj)
{
	if (pObj->cbFunc) {
		pObj->cbFunc(pObj->pConfig->instTx, SII_DRV_TX_EVENT__HDMI_STATE_CHNG);
	}
}

/***** end of file ***********************************************************/
