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
* @file si_drv_tx_hdcp.c
*
* @brief Tx HDCP API
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "sii_time.h"
#include "si_drv_cra_api.h"
#include "si_drv_tx_api.h"
#include "si_lib_log_api.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "si_lib_time_api.h"
#include "si_drv_tx_regs.h"
#include "si_vidpath_regs.h"
#if !defined(CONFIG_TARGET_SYMPHONY6_LITE)
#include "si_mt_mod_tx_hdcp_pram.h"
#endif

/***** Register Module name **************************************************/

SII_LIB_OBJ_MODULE_DEF(drv_tx_hdcp);

/***** public macro definitions **********************************************/
#define HDCP2X_CUPD_MAX_TIMEOUT					3000
#define HDCP2X_RETRY_THRESHOLD					7
#define HDCP2X_CONTENT_TYPE_SET_MAX_TIMEOUT		10
#define LEN_TPI__DDC_FIFO_SIZE                  16

#define TIMER_START__TX_HDCP__INTR          	150
#define TIMER_START__TX_HDCP__STATE         	40
#define TIMER_START__TX_HDCP__INTR_INTVAL     	10
#define TIMER_START__TX_HDCP__STATE_INTVAL      20
#define FIX_XMTV_NOSOUND		(0)

//! HDCP control flags
#define HDCP_CTRL_MODE ( 0 \
						 | BIT_MSK__TPI_COPP_DATA2__REG_COPP_PROTLEVEL \
						 | BIT_MSK__TPI_COPP_DATA2__REG_DDC_SHORT_RI_RD \
						 | BIT_MSK__TPI_COPP_DATA2__REG_DOUBLE_RI_CHECK \
						 | BIT_MSK__TPI_COPP_DATA2__REG_KSV_FORWARD \
					   )

//! HDCP Part 2 is successfully done
#define PART2_DONE (BIT_MSK__TPI_COPP_DATA1__REG_COPP_GPROT | BIT_MSK__TPI_COPP_DATA1__REG_COPP_LPROT)

#define SII_BKSV_LIST_BYTES             5
#define SII_HDCP2X_RCVID_LENGTH         5

//Page 8:0x0C - REG_ADDR__TX_HDCP2X_AUTH_STAT
#define BIT_ENUM__HDCP2X_AUTH_STAT__AUTH_DONE			0x01
#define BIT_ENUM__HDCP2X_AUTH_STAT__AUTH_FAIL			0x02
#define BIT_ENUM__HDCP2X_AUTH_STAT__CCHK_DONE			0x10
#define BIT_ENUM__HDCP2X_AUTH_STAT__CCHK_FAIL			0x20
#define HDCP2X_CCHK_FAIL								( BIT_ENUM__HDCP2X_AUTH_STAT__CCHK_DONE | BIT_ENUM__HDCP2X_AUTH_STAT__CCHK_FAIL )

//Page 6:0x29 - REG_ADDR__TPI_COPP_DATA1
#define BIT_ENUM__TPI_HDCP_QUERY__STATUS_NORMAL        0x00
#define BIT_ENUM__TPI_HDCP_QUERY__STATUS_LOST          0x10
#define BIT_ENUM__TPI_HDCP_QUERY__STATUS_FAILED        0x20
#define BIT_ENUM__TPI_HDCP_QUERY__STATUS_SUSPENDED     0x30

//Page 0:0xF3 - REG_ADDR__DDC_CMD
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_READ				0x02
#define BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ			0x04
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_WRITE				0x06
#define BIT_ENUM__DDC_CMD__CLEAR_FIFO					0x09
#define BIT_ENUM__DDC_CMD__ABORT_TRANSACTION			0x0F
#define GET_HDCP22CAP_INPROGRESS						(0)

/***** local type definitions ************************************************/
/**
* @brief HDCP state
*/
typedef enum {
	SII_MOD_TX_HDCP_EVENT__OFF,
	SII_MOD_TX_HDCP_EVENT__WAIT_FOR_START,
	SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE,
	SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READY,
	SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READ_DONE,
	SII_MOD_TX_HDCP_EVENT__AUTHENTICATED,
	SII_MOD_TX_HDCP_EVENT__FAILED,
} SiiModTxHdcpInternalState_t;

/**
* Interrupt Status Registers
*/
typedef struct {
	uint8_t              reg0;
	uint8_t              reg1;
} IntStat_t;

typedef struct {
	SiiModTxHdcpConfig_t			*pConfig;
	IntStat_t                       hdcp1xIntStat;
	IntStat_t						hdcp2xIntStat;
	IntStat_t						EcmIntStat;
	uint8_t							authFailCounter;
	bool_t							bAvMute;

	/*--------------------------------*/
	/* User request states            */
	/*--------------------------------*/
	bool_t                          isAuthRequested;            //!< set if downstream authentication is requested
	SiiDrvHdcpStatus_t				hdcpStatus;
	bool_t							bBksvListApproved;
	bool_t							isTxHpdAsserted;
	bool_t							isDsHdcp_2_2_Cap_Read;
	/*--------------------------------*/
	/* Internal states                */
	/*--------------------------------*/
	SiiModTxHdcpInternalState_t     authState;
	SiiModTxHdcpInternalState_t     prevAuthState;              //!< previous value of authState
	SiiDrvHdcpKsvList_t  			ksvList;
	SiiDrvHdcpTopology_t			hdcpTopology;
	//#if SII_INC_HDCP2XCORE
	uint8_t                         dsHdcp_2_2_Supported;       //!< down stream HDCP2.2 capability
	SiiDrvHdcpContentType_t	   	    hdcpContentType;			//!< hdcp2.2 content type
	SiiDrvHdcp2xCupdChkStat_t		hdcp2xCupdStat;
	//#endif
	SiiInst_t			 	        instTimerIsrPoll;			//!< Timer for interrupts
	SiiInst_t			 	        instTimerStatePoll;			//!< Timer for state machine

	HdcpEventNotifyCallBack			cbFunc;
	uint32_t			hdcp2x_seq_num_m;
	bool_t				hdcp2x_repeater_ready;
	bool_t				hdcp_ignore_mute;
	uint8_t                         dsHdcp_1_x_Supported;
} HdcpObj_t;

/***** local prototypes ******************************************************/

static void sUpdateHdcpState(HdcpObj_t *hdcpObj);
static void sHdcp2ContentTypeSet(HdcpObj_t *hdcpObj, SiiDrvHdcpContentType_t pContentType);
static SiiDrvDsHdcpVersion_t sDsHdcpVerGet(HdcpObj_t *hdcpObj);
static SiiDrvDsHdcpVersion_t sDsHdcpCapGet(HdcpObj_t *hdcpObj);

static SiiDrvCraAddr_t sbaseAddrGet(HdcpObj_t* hdcpObj);
static SiiInst_t sCraInstGet(HdcpObj_t* hdcpObj);
static void sResetKsvFifo(HdcpObj_t* hdcpObj);
#if TPG_RECOVERY_UNUSED_FUNC
	static void sToggleHwTpiBit(HdcpObj_t* hdcpObj);
#endif
static void sAvMuteSet(HdcpObj_t* pObj, bool_t bAvMute);
static void sStartHdcp(HdcpObj_t* hdcpObj, bool_t bEnable);
static void sTpiHdcpProtectionEnable(HdcpObj_t* hdcpObj, bool_t isEnabled);
static bool_t sTpiIsDownstreamHdcpAvailable(HdcpObj_t* hdcpObj);
static uint8_t sTpiHdcpStatusGet(HdcpObj_t* hdcpObj);
static bool_t sTpiHdcpIsPart2Done(HdcpObj_t* hdcpObj);
static void sTpiBksvGet(HdcpObj_t* hdcpObj, uint8_t* pBksv);
static bool_t sTpiKsvListPortionSizeGet(HdcpObj_t* hdcpObj, uint8_t *pBytesToRead);
static void sTpiKsvListGet(HdcpObj_t* hdcpObj, uint8_t *pBuffer, uint8_t length);
static SiiModTxHdcpInternalState_t sTpiGetKSVList(HdcpObj_t* hdcpObj, uint8_t dsBstatus[2]);
static void sTpiBStatusGet(HdcpObj_t* hdcpObj, uint8_t *pDsBStatus);
static void sPrintKsvList(HdcpObj_t* hdcpObj);
static void sPrintHdcpStatus(HdcpObj_t* hdcpObj);
static void sPrintHdcpQueryStatus(HdcpObj_t* hdcpObj, uint8_t query);
static void sHdcpStateMachineHandler(SiiInst_t inst);
static void sVirtualIsrHandler(HdcpObj_t* hdcpObj);
#if TPG_RECOVERY_UNUSED_FUNC
	static void sTpiHdcpEncriptionEnable(HdcpObj_t* hdcpObj, bool_t isEnable);
	static void sTpiHdcpDynamicAuthenticationEnable(HdcpObj_t* hdcpObj, bool_t isEnabled);
	static bool_t sTpiHdcpIsAuthenticationGood(HdcpObj_t* hdcpObj);
#endif
static void sHdcp2xIntrHandler(HdcpObj_t *hdcpObj);
static void sHdcp2xCodeUpdate(HdcpObj_t* hdcpObj);
static void Hdcp22AuthStop(HdcpObj_t* hdcpObj);
static bool_t sWaitForDdcBus(HdcpObj_t* hdcpObj);
#if GET_HDCP22CAP_INPROGRESS
static SiiDdcComErr_t sReadDsHdcp22Capability(HdcpObj_t* hdcpObj);
#endif
static void sTpiHdcp2ProtectionEnable(HdcpObj_t* hdcpObj, bool_t isEnabled);
static bool_t DsHdcp22SupportGet(HdcpObj_t* hdcpObj);
static void Hdcp22AuthStart(HdcpObj_t* hdcpObj);
static bool_t IsDSdeviceHDCP2Repeater(HdcpObj_t *hdcpObj);
static void sRcvIdListGet(HdcpObj_t* hdcpObj);
static void sUpdateHdcpTopology(HdcpObj_t* hdcpObj);
static void sHdcp2xCodeUpdatePatch(HdcpObj_t* hdcpObj);

//Notify functions
static void sNotifyHdcpStatus(HdcpObj_t* hdcpObj);
static void sHdcpReauth(HdcpObj_t* hdcpObj);
void SI_SetOverrideVidVal(HdcpObj_t* pObj, uint8_t c0, uint8_t c1, uint8_t c2);
static SiiDdcComErr_t sReadDsHdcpRead(HdcpObj_t* hdcpObj, uint8_t regAddr, uint8_t *pBuf, uint16_t length);
SiiLibTimeMilli_t g_tvformat_time[2] = {0,0};

/***** public functions ******************************************************/
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	#if __HDMI_OS_KERNEL__
	#include "linux/uaccess.h"
	#endif
	#include "mt4si/linux_k/si_mod_tx_hdcp_mt.h"
#endif

/*****************************************************************************/
/**
* @brief Tx HDCP driver constructor
*
* @param[in]  pNameStr   Name of instance
* @param[in]  pConfig    Static configuration parameters
*
* @retval                Handle to instance
*
*****************************************************************************/
SiiInst_t SiiModTxHdcpCreate(char *pNameStr, SiiModTxHdcpConfig_t *pConfig)
{
	HdcpObj_t*	hdcpObj = NULL;
	SiiDrvCraAddr_t	baseAddr;
	SiiInst_t		craInst;

	/* Allocate memory for object */
	hdcpObj = (HdcpObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(HdcpObj_t));
	SII_PLATFORM_DEBUG_ASSERT(hdcpObj);

	hdcpObj->pConfig = (SiiModTxHdcpConfig_t*)SiiLibMallocCreate(sizeof(SiiModTxHdcpConfig_t));
	if ( hdcpObj->pConfig ) {
		SII_MEMCPY(hdcpObj->pConfig, pConfig, sizeof(SiiModTxHdcpConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiModTxHdcpCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	//Register Callback Function
	hdcpObj->cbFunc = hdcpObj->pConfig->cbFunc;

	baseAddr = sbaseAddrGet(hdcpObj);
	craInst	 = sCraInstGet(hdcpObj);

	/*--------------------------------*/
	/* Initialize user request states */
	/*--------------------------------*/
	hdcpObj->isAuthRequested	= false;
	hdcpObj->bBksvListApproved	= false;
	hdcpObj->isTxHpdAsserted	= false;
	hdcpObj->bAvMute			= false;
	hdcpObj->isDsHdcp_2_2_Cap_Read = false;
	hdcpObj->hdcp2x_seq_num_m = 0;
	hdcpObj->hdcpContentType = SII_DRV_HDCP_CONTENT_TYPE__0;
	hdcpObj->hdcp2x_repeater_ready = false;
	hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__OFF;
	if ( pConfig->maxDsDev ) {
		hdcpObj->ksvList.pListStart = hdcpObj->ksvList.pList = (uint8_t*)SiiLibMallocCreate(pConfig->maxDsDev * sizeof(SiiDrvHdcpKsv_t));
		hdcpObj->ksvList.length  = 0;
		SII_PLATFORM_DEBUG_ASSERT(hdcpObj->ksvList.pList);
	}

	/*--------------------------------*/
	/* Initialize user status         */
	/*--------------------------------*/
	hdcpObj->authState				= SII_MOD_TX_HDCP_EVENT__OFF;
	hdcpObj->dsHdcp_2_2_Supported	= 0;
	hdcpObj->dsHdcp_1_x_Supported	= 0;
	hdcpObj->hdcp2xIntStat.reg0		= 0;
	hdcpObj->hdcp2xIntStat.reg1		= 0;
	hdcpObj->hdcp1xIntStat.reg0		= 0;
	hdcpObj->hdcp1xIntStat.reg1		= 0;
	hdcpObj->EcmIntStat.reg0		= 0;
	hdcpObj->EcmIntStat.reg1		= 0;
	hdcpObj->authFailCounter		= 0;

	/*-------Set HDCP Topology to default----------*/
	hdcpObj->hdcpTopology.depth				= 0;
	hdcpObj->hdcpTopology.deviceCount		= 0;
	hdcpObj->hdcpTopology.hdcp1xRepeaterDs	= 0;
	hdcpObj->hdcpTopology.hdcp20RepeaterDs	= 0;
	hdcpObj->hdcpTopology.maxCascadeExceeded = 0;
	hdcpObj->hdcpTopology.maxDevsExceeded	= 0;

/* Mini Uboot supports HDCP, Lite Uboot NOT support HDCP */
#if !defined(CONFIG_TARGET_SYMPHONY6_LITE)
	if (hdcpObj->pConfig->bHdcp2xEn) {

		sHdcp2xCodeUpdatePatch(hdcpObj);//Add this according to IC's suggestion, or HDCP2x will NOT send AKE_init

		//HDCP2x Patch Update
		sHdcp2xCodeUpdate(hdcpObj);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_1		, BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_REAUTH_SW);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP0		, 0x02);  // HDCP2X TP0=2
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP1		, 0x8C);  // HDCP2X TP1=24, 20MHz->98, 24MHz->117 27MHz->140
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP2		, 0x01);  // HDCP2X TP2=1
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP3		, 0x32);  // HDCP2X TP3=50
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP4		, 0x1E);  // HDCP2X TP4=30
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP5		, 0x78);  // HDCP2X TP5=120
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP6		, 0x02);  // HDCP2X TP6=2
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP7		, 0x0A);  // HDCP2X TP7=10
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP8		, 0x0A);  // HDCP2X TP8=10 Cert read timeout
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP9		, 0x14);  // HDCP2X TP9=20
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP10		, 0x16);  // HDCP2X TP10=22
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP11		, 0xC8);  // HDCP2X TP11=200
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP12		, 0x10);  // HDCP2X TP12=150
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP13		, 0x10);  // HDCP2X TP13=16
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP14		, 0xC8);  // HDCP2X TP14=200
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP15		, 0x00);  // HDCP2X TP15=0

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN0		, 0x00);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN1		, 0x22);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN2		, 0x80);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN3		, 0x00);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_POLL_VAL0	, 0x05); // DDC polling interval
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_POLL_VAL1	, 0xe4); // DDC polling interval

		/* Apply AES reset when authdone=0 */
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_AESCTL, BIT_MSK__HDCP2X_AESCTL__RI_AES_RST_AUTHDONE);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0		, 0xFF);	//Clearing Intr0
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1		, 0xFF);	// clearing Intr1
	}
#endif

	//SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_HW_OPT0, BIT_MSK__TPI_HW_OPT0__REG_R0_ABSOLUTE);  //Vsync
	//SiiDrvCraPutBit8(craInst,  baseAddr | REG_ADDR__TPI_HW_OPT1, BIT_MSK__TPI_HW_OPT1__REG_TPI_R0_CALC_TIME_B3_B0, 0x01);  //200+ -> 100+ms
	//Disabling HDCP Encryption
	SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);

	// Create a timer to update hdcp states
	hdcpObj->instTimerStatePoll = SII_LIB_SEQ_TIMER_CREATE("HDCP_State_Machine_Handler", sHdcpStateMachineHandler, SII_LIB_OBJ_INST(hdcpObj), 252);
	SII_PLATFORM_DEBUG_ASSERT(hdcpObj->instTimerStatePoll);

	return SII_LIB_OBJ_INST(hdcpObj);
}

void SiiModTxHdcpDelete(SiiInst_t inst)
{
	HdcpObj_t* hdcpObj = (HdcpObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiLibSeqTimerDelete(hdcpObj->instTimerStatePoll);
	SiiLibMallocDelete(hdcpObj->ksvList.pListStart);
	SiiLibMallocDelete(hdcpObj->pConfig);
	SII_LIB_OBJ_DELETE(hdcpObj);
}

static void sVirtualIsrHandlerMt(HdcpObj_t *hdcpObj);

/* Mini Uboot supports HDCP, Lite Uboot NOT support HDCP */
#if !defined(CONFIG_TARGET_SYMPHONY6_LITE)
void SiiModTxHdcpInterruptHandler(SiiInst_t inst)
{
	HdcpObj_t*		hdcpObj		= (HdcpObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	IntStat_t       intStat		= {0};

	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if (1) {
		uint8_t intreg = 0;
		uint8_t 		mask;

		mask = BIT_MSK_REG__EMP_SUCCESS_HDMI | BIT_MSK_REG__EMP_ERR_HDMI | BIT_MSK_REG__EMP_ERR_CPU;
		intStat.reg0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_STATUS);
		SI_HDMI20_PRINT("\n0x3688:0x%x\n", (uint32_t)intStat.reg0);
		intStat.reg0 &= mask;
		// Find any interrupt status bit that changed to '1'
		intreg = 0;
		intreg |= ((hdcpObj->EcmIntStat.reg0 ^ intStat.reg0) & intStat.reg0);

		hdcpObj->EcmIntStat = intStat;
		if (intreg) {
			// Call derived interupt handler
			sVirtualIsrHandlerMt(hdcpObj);
		}
		memset(&intStat, 0, sizeof(IntStat_t));
	}
	#endif

	if (hdcpObj->pConfig->bHdcp2xEn) {
		if (DsHdcp22SupportGet(hdcpObj)) {
			sHdcp2xIntrHandler(hdcpObj);
			return;
		}
	}
	// Capture and mask interrupt status bits. (ignore status bits and non-serving interrupts)
	intStat.reg0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0) & BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST7;
	intStat.reg1 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0) & BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST3;

	SI_HDMI20_PRINT("\n0x363d:0x%x\n", (uint32_t)SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0));
	if (SII_MEMCMP(&hdcpObj->hdcp1xIntStat, &intStat, sizeof(IntStat_t))) {
		uint8_t intreg = 0;

		// Find any interrupt status bit that changed to '1'
		intreg |= ((hdcpObj->hdcp1xIntStat.reg0 ^ intStat.reg0) & intStat.reg0);
		intreg |= ((hdcpObj->hdcp1xIntStat.reg1 ^ intStat.reg1) & intStat.reg1);

		hdcpObj->hdcp1xIntStat = intStat;

		// Clear all pending HDCP interrupts
		if (hdcpObj->hdcp1xIntStat.reg0) {
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, hdcpObj->hdcp1xIntStat.reg0);
		}

		if (hdcpObj->hdcp1xIntStat.reg1) {
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, hdcpObj->hdcp1xIntStat.reg1);
		}
		if (intreg) {
			// Call derived interupt handler
			sVirtualIsrHandler(hdcpObj);
		}
		// Clear all pending HDCP interrupts
		if (hdcpObj->hdcp1xIntStat.reg0) {
			hdcpObj->hdcp1xIntStat.reg0 = 0x00;
		}

		if (hdcpObj->hdcp1xIntStat.reg1) {
			hdcpObj->hdcp1xIntStat.reg1 = 0x00;
		}
	}
}
#endif

//Todo : Try to return ErrorStatus
//-------------------------------------------------------------------------------------------------
//! @brief      HDMI TX module Set API
//-------------------------------------------------------------------------------------------------
bool_t SiiModTxHdcpSet(SiiInst_t inst, SiiModTxHdcpOpcode_t opcode, void *inData)
{
	HdcpObj_t* hdcpObj = (HdcpObj_t*)SII_LIB_OBJ_PNTR(inst);

	switch (opcode) {
		case SII_MOD_TX_HDCP_OPCODE__HDCP_ENABLE: {
			uint8_t hdcpEnable = *(uint8_t *)inData;
			sStartHdcp(hdcpObj, hdcpEnable);
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__HDCP_CONTENT_TYPE: {
			SiiDrvHdcpContentType_t contentType = *(SiiDrvHdcpContentType_t*)inData;
			sHdcp2ContentTypeSet(hdcpObj, contentType);
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST_APPROVAL: {
			SII_MEMCPY(&hdcpObj->bBksvListApproved, inData, sizeof(hdcpObj->bBksvListApproved));
			break;
		}

		case SII_MOD_TX_HDCP_OPCODE__TX_HPD_STATUS: {
			SII_MEMCPY(&hdcpObj->isTxHpdAsserted, inData, sizeof(hdcpObj->isTxHpdAsserted));
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__AVMUTE: {
			//SII_MEMCPY(&hdcpObj->bAvMute, inData, sizeof(bool_t));
			//sAvMuteSet(hdcpObj, hdcpObj->bAvMute);
			sAvMuteSet(hdcpObj, *(bool_t *)inData);
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__HDCP_REAUTH: {
			hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
			sHdcpReauth(hdcpObj);
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__HDCP_MUTE: {
			hdcpObj->hdcp_ignore_mute = *(bool_t *)inData;
			break;
		}
		default:
			break;
	}
	return true;
}

bool_t SiiModTxHdcpGet(SiiInst_t inst, SiiModTxHdcpOpcode_t opcode, void *outData)
{
	HdcpObj_t* hdcpObj = (HdcpObj_t*)SII_LIB_OBJ_PNTR(inst);

	switch (opcode) {
		case SII_MOD_TX_HDCP_OPCODE__HDCP_STATUS:
			sUpdateHdcpState(hdcpObj);
			SII_MEMCPY(outData, &(hdcpObj->hdcpStatus), sizeof(hdcpObj->hdcpStatus));
			break;
		case SII_MOD_TX_HDCP_OPCODE__HDCP_VERSION: {
			SiiDrvDsHdcpVersion_t dsHdcpVer = sDsHdcpVerGet(hdcpObj);
			SII_MEMCPY(outData, &dsHdcpVer, sizeof(dsHdcpVer));
			break;
		}
		case SII_MOD_TX_HDCP_OPCODE__HDCP_BKSV_LIST:
			SII_MEMCPY(outData, &(hdcpObj->ksvList), sizeof(hdcpObj->ksvList));
			break;
		case SII_MOD_TX_HDCP_OPCODE__HDCP_TOPOLOGY:
			sUpdateHdcpTopology(hdcpObj);
			SII_MEMCPY(outData, &(hdcpObj->hdcpTopology), sizeof(hdcpObj->hdcpTopology));
			break;
		case SII_MOD_TX_HDCP_OPCODE__HDCP2X_CUPD_STAT:
			SII_MEMCPY(outData, &(hdcpObj->hdcp2xCupdStat), sizeof(hdcpObj->hdcp2xCupdStat));
			break;
		case SII_MOD_TX_HDCP_OPCODE__HDCP_CAP: {
			SiiDrvDsHdcpVersion_t dsHdcpVer = sDsHdcpCapGet(hdcpObj);
			SII_MEMCPY(outData, &dsHdcpVer, sizeof(dsHdcpVer));
			break;
		}
		default:
			SII_LIB_LOG_PRINT1(hdcpObj, ("HDCP get error opcode:%d", opcode));
			break;
	}
	return true;
}

/***** local functions *******************************************************/
static void sUpdateHdcpState(HdcpObj_t *hdcpObj)
{
	switch (hdcpObj->authState) {
		case SII_MOD_TX_HDCP_EVENT__OFF:
			hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__OFF;
			break;
		case SII_MOD_TX_HDCP_EVENT__WAIT_FOR_START:
		case SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE:
		case SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READY:
			hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__AUTHENTICATING;
			break;
		case SII_MOD_TX_HDCP_EVENT__AUTHENTICATED:
			if (SII_DRV_DS_HDCP_VER__22 == sDsHdcpVerGet(hdcpObj)) {
				hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__SUCCESS_22;
			} else {
				hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__SUCCESS_1X;
			}
			{
				SiiLibTimeMilli_t tdiff = 0;
				tdiff = SiiLibTimeMilliGet() - g_tvformat_time[0];
				if ( g_tvformat_time[0] != 0 ) {
				  SII_LIB_LOG_PRINT1(hdcpObj, ("hon tvsys switch, time: %d (ms)\n",(uint32_t)tdiff));
				}
				g_tvformat_time[1] = 0;
				g_tvformat_time[0] = 0;
			}
			break;
		case SII_MOD_TX_HDCP_EVENT__FAILED:
			hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__FAILED;
			break;
		default:
			SII_LIB_LOG_PRINT1(hdcpObj, ("HDCP get error state:%d", hdcpObj->authState));
			break;
	}
}

static SiiDrvDsHdcpVersion_t sDsHdcpVerGet(HdcpObj_t *hdcpObj)
{
	if(DsHdcp22SupportGet(hdcpObj))
		return SII_DRV_DS_HDCP_VER__22;
	else if (hdcpObj->dsHdcp_1_x_Supported)
		return SII_DRV_DS_HDCP_VER__1X;
	else
		return SII_DRV_DS_HDCP_VER__NONE;
}

static SiiDrvDsHdcpVersion_t sDsHdcpCapGet(HdcpObj_t *hdcpObj)
{
	uint8_t data[5];
	SiiDrvDsHdcpVersion_t ret = SII_DRV_DS_HDCP_VER__NONE;
	//SII_LIB_LOG_PRINT1(hdcpObj, ("sDsHdcpVerGet\n"));

	if ( hdcpObj->isTxHpdAsserted ) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP asserted\n"));
		if (SII_DDC_ERROR_CODE_NO_ERROR == sReadDsHdcpRead(hdcpObj, 0x50, data, 1)) {
			if ( data[0] & 0x04 ) {
				SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP 2x\n"));
				ret = SII_DRV_DS_HDCP_VER__22;
				hdcpObj->dsHdcp_2_2_Supported = 0x04;
				hdcpObj->dsHdcp_1_x_Supported	= 1;
			}
		}

		if ( ret == SII_DRV_DS_HDCP_VER__NONE ) {
			hdcpObj->dsHdcp_2_2_Supported = 0x0;
			hdcpObj->dsHdcp_1_x_Supported	= 0;
			if ( SII_DDC_ERROR_CODE_NO_ERROR == sReadDsHdcpRead(hdcpObj, 0x00, data, 5) ) {
				uint8_t i;
				uint8_t onecnt = 0;
				for (i=0;i<8;i++) {
					onecnt += (uint8_t)((data[0] & ( 1<<i )) > 0);
					onecnt += (uint8_t)((data[1] & ( 1<<i )) > 0);
					onecnt += (uint8_t)((data[2] & ( 1<<i )) > 0);
					onecnt += (uint8_t)((data[3] & ( 1<<i )) > 0);
					onecnt += (uint8_t)((data[4] & ( 1<<i )) > 0);
				}
				SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP 1x [%x %x %x %x %x] %d ones\n",data[0],data[1],data[2],data[3],data[4],onecnt));
				if ( onecnt == 20 ) {
					ret = SII_DRV_DS_HDCP_VER__1X;
					hdcpObj->dsHdcp_1_x_Supported	= 1;
				}
			}
		}
	}else {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP de-asserted\n"));
		hdcpObj->isDsHdcp_2_2_Cap_Read = false;
		hdcpObj->dsHdcp_2_2_Supported = 0x00;
		hdcpObj->dsHdcp_1_x_Supported	= 0;
	}
	return ret;
}

static void sHdcp2ContentTypeSet(HdcpObj_t *hdcpObj, SiiDrvHdcpContentType_t pContentType)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	uint8_t timeout;

	if (SII_DRV_DS_HDCP_VER__22 == sDsHdcpVerGet(hdcpObj)) {
		hdcpObj->hdcp2x_seq_num_m = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0);
		hdcpObj->hdcp2x_seq_num_m = (hdcpObj->hdcp2x_seq_num_m << 8) |
									SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1);
		hdcpObj->hdcp2x_seq_num_m = (hdcpObj->hdcp2x_seq_num_m << 8) |
									SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2);

		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RPT_SMNG_K,     0x01);		// k -> always 0x01/*REG_ADDR__HDCP2X_RPT_SMNG_K*/
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR_START);	// smng_wr_start=1
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_TX_RPT_SMNG_IN, pContentType);		// smng_in (stream ID)
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=0
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_TX_RPT_SMNG_IN, 0x00);		// smng_in (type)
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=0
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START);	// smng_xfer_start=1-rising edge will start transfer
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START);
		hdcpObj->hdcp2x_seq_num_m++;

		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0, (uint8_t)hdcpObj->hdcp2x_seq_num_m);		// seq_num_m0
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1, (uint8_t)hdcpObj->hdcp2x_seq_num_m >> 8);		// seq_num_m1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2, (uint8_t)hdcpObj->hdcp2x_seq_num_m >> 16);		// seq_num_m2
		SiiLibTimeMilliDelay(5);
		timeout = HDCP2X_CONTENT_TYPE_SET_MAX_TIMEOUT;
		while (--timeout && !( SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_STATUS) & BIT_MSK__HDCP2X_TX_STATUS__RO_HDCP2TX_RPT_SMNG_XFER_DONE)) { // Wait smng_xfer_done
			SiiLibTimeMilliDelay(1);
		}
		if (!timeout) {
			SII_LIB_LOG_PRINT1(hdcpObj, ("Unable to set contentType... Stream Manage Transfer failed"));
		} else {
			SII_LIB_LOG_PRINT1(hdcpObj, ("ContentType set to %s", pContentType ? "SII_DRV_HDCP_CONTENT_TYPE__1" : "SII_DRV_HDCP_CONTENT_TYPE__0"));
		}
	} else {
		SII_LIB_LOG_PRINT1(hdcpObj, ("Unable to set contentType... DS Device is not HDCP2.2 Capable\n"));
	}

}
/***** local functions *******************************************************/
static void sHdcp2streamManageMessageSet(HdcpObj_t *hdcpObj, SiiDrvHdcpContentType_t pContentType)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	uint32_t hdcp2x_seq_num_m_tmp = 0;

	if (SII_DRV_DS_HDCP_VER__22 == sDsHdcpVerGet(hdcpObj)) {
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0, BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN);
		hdcp2x_seq_num_m_tmp = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0);
		hdcp2x_seq_num_m_tmp = (hdcp2x_seq_num_m_tmp << 8) |
									SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1);
		hdcp2x_seq_num_m_tmp = (hdcp2x_seq_num_m_tmp << 8) |
									SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2);
		if ( hdcp2x_seq_num_m_tmp ) {
			hdcpObj->hdcp2x_seq_num_m = hdcp2x_seq_num_m_tmp;
		}

		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RPT_SMNG_K,     0x01);		// k -> always 0x01/*REG_ADDR__HDCP2X_RPT_SMNG_K*/
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR_START);	// smng_wr_start=1
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_TX_RPT_SMNG_IN, pContentType);		// smng_in (stream ID)
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=0
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_TX_RPT_SMNG_IN, 0x00);		// smng_in (type)
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_WR);	// smng_wr=0
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START);	// smng_xfer_start=1-rising edge will start transfer
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0,      BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START);
		hdcpObj->hdcp2x_seq_num_m++;
		SII_LIB_LOG_PRINT1(hdcpObj, ("Sending stream message... :%d\n",hdcpObj->hdcp2x_seq_num_m));

		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0, (uint8_t)hdcpObj->hdcp2x_seq_num_m);		// seq_num_m0
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1, (uint8_t)hdcpObj->hdcp2x_seq_num_m >> 8);		// seq_num_m1
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2, (uint8_t)hdcpObj->hdcp2x_seq_num_m >> 16);		// seq_num_m2
	} else {
		SII_LIB_LOG_PRINT1(hdcpObj, ("Unable to set contentType... DS Device is not HDCP2.2 Capable\n"));
	}

}

static void sUpdateHdcpTopology(HdcpObj_t* hdcpObj)
{
	uint8_t hdcpMiscStatus;
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	if (hdcpObj->pConfig->bHdcp2xEn && DsHdcp22SupportGet(hdcpObj)) {
		hdcpObj->hdcpTopology.depth = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DEPTH);
		hdcpObj->hdcpTopology.deviceCount = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DEVCNT);

		hdcpMiscStatus = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DETAIL);
		hdcpObj->hdcpTopology.maxDevsExceeded = (hdcpMiscStatus & BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_DEVS_EXC) ? true : false;
		hdcpObj->hdcpTopology.maxCascadeExceeded = (hdcpMiscStatus & BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_CASC_EXC) ? true : false;
		hdcpObj->hdcpTopology.hdcp20RepeaterDs = (hdcpMiscStatus & BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_HDCP20RPT_DSTRM) ? true : false;
		hdcpObj->hdcpTopology.hdcp1xRepeaterDs = (hdcpMiscStatus & BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_HDCP1DEV_DSTRM) ? true : false;
	} else {
		uint8_t bStatus[2] = {0};
		SiiDrvCraBlockRead8(craInst, baseAddr | REG_ADDR__TPI_BSTATUS1, bStatus, 2);
		hdcpObj->hdcpTopology.depth = bStatus[1] & BIT_MSK__TPI_BSTATUS2__REG_DS_DEPTH;
		hdcpObj->hdcpTopology.deviceCount = bStatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_CNT;
		hdcpObj->hdcpTopology.maxDevsExceeded =	(bStatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_EXCEED) ? true : false;
		hdcpObj->hdcpTopology.maxCascadeExceeded = (bStatus[1] & BIT_MSK__TPI_BSTATUS2__REG_DS_CASC_EXCEED) ? true : false;
		hdcpObj->hdcpTopology.hdcp20RepeaterDs = false;
		hdcpObj->hdcpTopology.hdcp1xRepeaterDs = false; //Todo: Update using device count

	}
}

static SiiDrvCraAddr_t sbaseAddrGet(HdcpObj_t* hdcpObj)
{
	return hdcpObj->pConfig->baseAddrTx;
}

static SiiInst_t sCraInstGet(HdcpObj_t* hdcpObj)
{
	return hdcpObj->pConfig->instTxCra;
}

static void sResetKsvFifo(HdcpObj_t* hdcpObj)
{
	SII_LIB_LOG_DEBUG1(hdcpObj, ("Resetting KSV fifo!!\n"));
	hdcpObj->ksvList.length = 0;
	hdcpObj->ksvList.pList = hdcpObj->ksvList.pListStart;
}

static void sSetHwTpiBit(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Set HW TPI bit!!\n"));
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__LM_DDC, BIT_MSK__LM_DDC__REG_SW_TPI_EN );
}

static void sClearHwTpiBit(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Clear HW TPI bit!!\n"));
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__LM_DDC, BIT_MSK__LM_DDC__REG_SW_TPI_EN );
}

#if TPG_RECOVERY_UNUSED_FUNC
static void sToggleHwTpiBit(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Toggle HW TPI bit!!\n"));
	//Toggle h/w TPI bit:: necessary as it triggers the HDCP interrupts, without it HDCP interrupts may not be triggered by h/w
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__LM_DDC, BIT_MSK__LM_DDC__REG_SW_TPI_EN );
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__LM_DDC, BIT_MSK__LM_DDC__REG_SW_TPI_EN );
}
#endif

static void sResetTpiStateMachine(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	SII_LIB_LOG_DEBUG1(hdcpObj, ("Resetting TPI State Machine!!!!!\n"));

	/* workaround for hardare ddc issue with scdc module */
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__SCDC_CTL,
					 BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY); // enable SCDC auto reply read request from slave
	//for SCDC registers up_flag0 and up_flag1
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT0,
					 BIT_MSK__TPI_HW_OPT0__REG_HW_TPI_SM_RST );
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT0,
					 BIT_MSK__TPI_HW_OPT0__REG_HW_TPI_SM_RST );

	/* workaround for hardare ddc issue with scdc module */
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__SCDC_CTL,
					 BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY); // enable SCDC auto reply read request from slave
	//for SCDC registers up_flag0 and up_flag1
}

void SI_SetOverrideVidVal(HdcpObj_t* pObj, uint8_t c0, uint8_t c1, uint8_t c2)
{
	uint16_t c0_12;
	uint16_t c1_12;
	uint16_t c2_12;
	#if 1
	uint16_t cms0 = 0;
	uint16_t cms1 = 0;

	cms0 = (uint16_t)SiiDrvCraRdReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG);
	cms0 |= ((uint16_t)SiiDrvCraRdReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG + 1)) << 8;

	cms1 = (uint16_t)SiiDrvCraRdReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG);
	cms1 |= ((uint16_t)SiiDrvCraRdReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG + 1)) << 8;

	//SII_LIB_LOG_DEBUG1(pObj, ("cms0 0x%x, cms1 0x%x\n",cms0,cms1));
	if ( cms1 & BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__ENABLE ) {
		if ( cms1 & BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_RGB ) {
			c0 = 0x0;
			c1 = 0x0;
			c2 = 0x0;
		} else {
			c0 = 0x10;
			c1 = 0x80;
			c2 = 0x80;
		}
	} else if ( cms0 & BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE ) {
		if ( cms0 & BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_RGB ) {
			c0 = 0x0;
			c1 = 0x0;
			c2 = 0x0;
		} else {
			c0 = 0x10;
			c1 = 0x80;
			c2 = 0x80;
		}
	} else {
		c0 = 0x10;
		c1 = 0x80;
		c2 = 0x80;
	}
	#endif
	c0_12 = ((uint16_t)c0) << 4;
	c1_12 = ((uint16_t)c1) << 4;
	c2_12 = ((uint16_t)c2) << 4;
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_Y, c0_12 & 0xFF);
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_Y + 1, (c0_12 >> 8) & 0x0F);
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_CB, c1_12 & 0xFF);
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_CB + 1, (c1_12 >> 8) & 0x0F);
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_CR, c2_12 & 0xFF);
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_ACTIVE_CR + 1, (c2_12 >> 8) & 0x0F);
}

static void SI_SetVideo(HdcpObj_t* pObj, bool_t bAvMute)
{
	if ( bAvMute ) {
		SI_SetOverrideVidVal(pObj, 0, 0, 0);
		SiiDrvCraSetBit8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE);
	} else {
		SiiDrvCraClrBit8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE);
	}
}

#if 0//FIX_XMTV_NOSOUND
static void SI_SetAudio(HdcpObj_t* pObj, bool_t bAvMute)
{
	if (bAvMute) {
		SiiDrvCraSetBit8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_AUD_MUTE_EN);
	} else {
		SiiDrvCraClrBit8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_AUD_MUTE_EN);
	}
}
#elif 1
static void SI_SetAudio(HdcpObj_t* pObj, bool_t bAvMute)
{
#define		AUD_MUTE_MODE		(1)
	uint8_t regc = 0;
	regc = SiiDrvCraRdReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__SYS_CTRL1);
	if (bAvMute) {
		if ( AUD_MUTE_MODE == 0 ) {
			regc &= ~(1 << 0);
			regc |= (1 << 6) | (1 << 4) | (1 << 1); //No Audio
		} else {
			regc &= ~(1 << 1);
			regc |= (1 << 6) | (1 << 4) | (1 << 0); //0 Audio
		}
		SiiDrvCraWrReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__SYS_CTRL1, regc);
		SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_AUD_MUTE_EN);
		SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__TPI_AUD_CONFIG, BIT_MSK__TPI_AUD_CONFIG__REG_TPI_AUD_MUTE);
	} else {
		SiiDrvCraWrReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__SYS_CTRL1, 0x00);
		SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__AUDP_TXCTRL, BIT_MSK__AUDP_TXCTRL__REG_AUD_MUTE_EN);
		SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__TPI_AUD_CONFIG, BIT_MSK__TPI_AUD_CONFIG__REG_TPI_AUD_MUTE);
	}
}
#else
static void SI_SetAudio(HdcpObj_t* pObj, bool_t bAvMute)
{
	uint8_t regc = 0;
	regc = SiiDrvCraRdReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__AIP_RST);
	if (bAvMute) {
		regc &= ~(3 << 0);
		regc |= 3;
	} else {
		regc &= ~(3 << 0);
	}
	SiiDrvCraWrReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__AIP_RST, regc);
}
#endif

static void sAvMuteSet(HdcpObj_t* pObj, bool_t bAvMute)
{
	uint8_t	regc = 0;

	if ( pObj->bAvMute == bAvMute ) {
		return;
	}
	pObj->bAvMute = bAvMute;
	SII_LIB_LOG_DEBUG1(pObj, ("Setting AVMUTE:: %s,%d\n", bAvMute ? "ON" : "OFF",pObj->isAuthRequested));

	SI_SetVideo(pObj, bAvMute);
	SI_SetAudio(pObj, bAvMute);
#if FIX_XMTV_NOSOUND
	if ( bAvMute == false ) {
		SiiLibTimeMilliDelay(50);
		SI_SetAudio(pObj, 1);
		SiiLibTimeMilliDelay(50);
		SI_SetAudio(pObj, 0);
	}
#endif
	regc = SiiDrvCraRdReg8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__TPI_SC);
	if ( bAvMute != ((regc & BIT_MSK__TPI_SC__REG_TPI_AV_MUTE) >> 3)) {
		if ( bAvMute ) {
			SiiDrvCraSetBit8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_AV_MUTE);
		} else {
			SiiDrvCraClrBit8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_AV_MUTE);
		}
	}
	//SiiLibTimeMilliDelay(50);
}

static void sAvMuteSetDONE(HdcpObj_t* pObj, bool_t bAvMute, bool_t xm)
{
	if ( pObj->bAvMute == bAvMute ) {
		return;
	}
	pObj->bAvMute = bAvMute;
	SII_LIB_LOG_DEBUG1(pObj, ("Setting AVMUTE:: %s\n", bAvMute ? "ON" : "OFF"));
	SiiDrvCraClrBit8(pObj->pConfig->instTxCra, REG_ADDR__VP__OUTPUT_BLANK_CONFIG, BIT_MSK__VP__OUTPUT_BLANK_CONFIG__ENABLE_ACTIVE_OVERRIDE);
	SI_SetAudio(pObj, bAvMute);
#if FIX_XMTV_NOSOUND
	if ( bAvMute == false && xm == true ) {
		SiiLibTimeMilliDelay( 100 );
		SI_SetAudio(pObj, 1);
		SiiLibTimeMilliDelay( 100 );
		SI_SetAudio(pObj, 0);
	}
#endif
	SiiDrvCraClrBit8(pObj->pConfig->instTxCra, pObj->pConfig->baseAddrTx | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_AV_MUTE);
}

static void sStopHdcpOne(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	if ( DsHdcp22SupportGet(hdcpObj) == 0) { //Switch HDCP2x to HDCP1x
		uint8_t hdcp2x_ctrl0 = 0;
		uint8_t hdcp2x_ctrl1 = 0;
		hdcp2x_ctrl0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK);
		hdcp2x_ctrl1 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK);
		if ( hdcp2x_ctrl0 && hdcp2x_ctrl1) {
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			sTpiHdcp2ProtectionEnable(hdcpObj, false);
			//hdcpObj->dsHdcp_2_2_Supported = 0;

			//Disable HDCP2x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, 0x0); //HDCP2x Interrupt0 Mask Register
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, 0x0); //HDCP2x Interrupt1 Mask Register
		}
	} else {
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_TPI_HDCP_PREP_EN);
			//Disabling HDCP Encryption
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN, SET_BITS);
			//SiiLibTimeMilliDelay(50);
			sTpiHdcpProtectionEnable(hdcpObj, false);

			//DisableHDCP1x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_EN, 0x0);
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      Start HDCP
//!
//! @param[in]  Start authentication
//-------------------------------------------------------------------------------------------------
static void sStartHdcp(HdcpObj_t* hdcpObj, bool_t bEnable)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	hdcpObj->authState			= SII_MOD_TX_HDCP_EVENT__OFF;
	HDMI_SI_LOCK();
	hdcpObj->isAuthRequested	= bEnable;
	HDMI_SI_UNLOCK();

	if (bEnable) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP ON!!\n"));
		if (hdcpObj->isTxHpdAsserted) {
			if ( hdcpObj->hdcp_ignore_mute == false ) {
			sAvMuteSet(hdcpObj, true);
			} else {
				hdcpObj->hdcp_ignore_mute = false;
			}
			SiiLibSeqTimerStart(hdcpObj->instTimerStatePoll, TIMER_START__TX_HDCP__STATE, TIMER_START__TX_HDCP__STATE_INTVAL);
		}
	} else {
		SiiLibSeqTimerStop(hdcpObj->instTimerStatePoll);

		{	//clear all interrupt
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, 0xff);
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0, 0xff);
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1, 0xff);
		}
		if (DsHdcp22SupportGet(hdcpObj)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nStop HDCP2x...\n"));
			sResetKsvFifo(hdcpObj);
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			SiiLibTimeMilliDelay(50);
			Hdcp22AuthStop(hdcpObj);
			sTpiHdcp2ProtectionEnable(hdcpObj, false);
			//hdcpObj->dsHdcp_2_2_Supported = 0;
			//sResetKsvFifo(hdcpObj);

			//Disable HDCP2x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, 0x0); //HDCP2x Interrupt0 Mask Register
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, 0x0); //HDCP2x Interrupt1 Mask Register
		} else {
			sResetKsvFifo(hdcpObj);
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_TPI_HDCP_PREP_EN);
			//Disabling HDCP Encryption
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN, SET_BITS);
			SiiLibTimeMilliDelay(50);
			sTpiHdcpProtectionEnable(hdcpObj, false);

			//DisableHDCP1x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_EN, 0x0);
		}
		if ( ! hdcpObj->isTxHpdAsserted ) {
			hdcpObj->isDsHdcp_2_2_Cap_Read = false;
		}

		hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__OFF;
		sPrintHdcpStatus(hdcpObj);
		sNotifyHdcpStatus(hdcpObj);
		if ( hdcpObj->hdcp_ignore_mute == false ) {
			sAvMuteSet(hdcpObj, false);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      Start HDCP
//!
//! @param[in]  Start authentication
//-------------------------------------------------------------------------------------------------
static void sHdcpReauth(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t craInst			= sCraInstGet(hdcpObj);
	hdcpObj->authState			= SII_MOD_TX_HDCP_EVENT__OFF;
	HDMI_SI_LOCK();
	if ( hdcpObj->isAuthRequested == false ) {
		return;
	}
	//hdcpObj->isAuthRequested	= true;
	HDMI_SI_UNLOCK();

	//HDCP off
	{

		SiiLibSeqTimerStop(hdcpObj->instTimerStatePoll);
		{	//clear all interrupt
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, 0xff);
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0, 0xff);
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1, 0xff);
		}

		if (DsHdcp22SupportGet(hdcpObj)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nReauth debug ...\n"));
			sResetKsvFifo(hdcpObj);
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			SiiLibTimeMilliDelay(50); //wait at least 1 frame to make sure encrytion is really disabled
			Hdcp22AuthStop(hdcpObj);
			sTpiHdcp2ProtectionEnable(hdcpObj, false);
			//hdcpObj->dsHdcp_2_2_Supported = 0;
			//sResetKsvFifo(hdcpObj);

			//Disable HDCP2x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, 0x0); //HDCP2x Interrupt0 Mask Register
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, 0x0); //HDCP2x Interrupt1 Mask Register
		} else {
			sResetKsvFifo(hdcpObj);
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_TPI_HDCP_PREP_EN);
			//Disabling HDCP Encryption
			SiiDrvCraSetBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN, SET_BITS);
			SiiLibTimeMilliDelay(50); //wait at least 1 frame to make sure encrytion is really disabled
			sTpiHdcpProtectionEnable(hdcpObj, false);

			//DisableHDCP1x Intr Masks
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_EN, 0x0);
		}
		if ( ! hdcpObj->isTxHpdAsserted ) {
			hdcpObj->isDsHdcp_2_2_Cap_Read = false;
		}

		hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__OFF;
		sPrintHdcpStatus(hdcpObj);
		sNotifyHdcpStatus(hdcpObj);
		//sAvMuteSet(hdcpObj, false);
	}

	//HDCP on
	{
		SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP ON!!\n"));
		if (hdcpObj->isTxHpdAsserted) {
			sAvMuteSet(hdcpObj, true);
			SiiLibSeqTimerStart(hdcpObj->instTimerStatePoll, TIMER_START__TX_HDCP__STATE, TIMER_START__TX_HDCP__STATE_INTVAL);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable HDCP protection.
//!
//! @param[in]  isEnabled - true, if HDCP protection has to be enabled.
//-------------------------------------------------------------------------------------------------

static void sTpiHdcpProtectionEnable(HdcpObj_t* hdcpObj, bool_t isEnabled)
{
	SiiDrvCraPutBit8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_COPP_DATA2, HDCP_CTRL_MODE, isEnabled ? HDCP_CTRL_MODE : CLEAR_BITS);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return HDCP Status byte.
//-------------------------------------------------------------------------------------------------

static uint8_t sTpiHdcpStatusGet(HdcpObj_t* hdcpObj)
{
	uint8_t hdcpStatus = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_COPP_DATA1);
	return hdcpStatus;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return true if Sink HDCP is available, false otherwise.
//-------------------------------------------------------------------------------------------------

static bool_t sTpiIsDownstreamHdcpAvailable(HdcpObj_t* hdcpObj)
{
	uint8_t hdcpStatus = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_COPP_DATA1);
	return ((hdcpStatus & BIT_MSK__TPI_COPP_DATA1__REG_COPP_PROTYPE) != 0);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Check if HDCP authentication Part 2 is successfully done.
//-------------------------------------------------------------------------------------------------

static bool_t sTpiHdcpIsPart2Done(HdcpObj_t* hdcpObj)
{
	uint8_t hdcp_status = sTpiHdcpStatusGet(hdcpObj);
	return (PART2_DONE == (hdcp_status & PART2_DONE));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read BKSV that is 8*5 = 40 bits.
//!
//! @param[in]  pBksv - pointer to an array to store the BKSV.
//-------------------------------------------------------------------------------------------------

static void sTpiBksvGet(HdcpObj_t* hdcpObj, uint8_t* pBksv)
{
	SiiDrvCraBlockRead8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_WR_BKSV_1, pBksv, SII_BKSV_LIST_BYTES);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return a number of bytes in the KSV list FIFO ready to be read and Done flag.
//!
//!             To be used in repeater operation when DS KSV list is copied from DS to US.
//!
//! @param[out] pBytesToRead - pointer to a variable holding the number of bytes to read,
//!
//! @return     KSV List reading done flag.
//! @retval     true - if a final portion of the KSV list is awaiting to be read.
//-------------------------------------------------------------------------------------------------

static bool_t sTpiKsvListPortionSizeGet(HdcpObj_t* hdcpObj, uint8_t *pBytesToRead)
{
	bool_t isDone;
	uint8_t fifoStatus = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_KSV_FIFO_STAT);

	*pBytesToRead = fifoStatus & BIT_MSK__TPI_KSV_FIFO_STAT__KSV_FIFO_BYTES;
	isDone = ((fifoStatus & BIT_MSK__TPI_KSV_FIFO_STAT__KSV_FIFO_LAST) != 0);

	return isDone;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get DS BKSV list.
//!
//! @param[in]  pBuffer - pointer to a buffer for the KSV list storage,
//! @param[in]  length  - number of bytes to read.
//-------------------------------------------------------------------------------------------------

static void sTpiKsvListGet(HdcpObj_t* hdcpObj, uint8_t *pBuffer, uint8_t length)
{
	// Note: this FIFO register is a special case. While reading from it in burst mode
	// the slave I2C interface don't increment the offset after every single reading.
	// Content of the register gets immediately updated by data waiting in the FIFO
	// after every reading.
	SiiDrvCraFifoRead8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_KSV_FIFO_FORW, pBuffer, length);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Send BSTATUS to US device FIFO.
//-------------------------------------------------------------------------------------------------

static SiiModTxHdcpInternalState_t sTpiGetKSVList(HdcpObj_t* hdcpObj, uint8_t dsBstatus[2])
{
#define KSV_MAX_COUNT 	(2000)
	uint8_t bytesToRead;
	uint8_t dsCount = dsBstatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_CNT;
	uint8_t dsDepth = dsBstatus[1] & BIT_MSK__TPI_BSTATUS2__REG_DS_DEPTH;
	int16_t fifoByteCounter;
	bool_t isDone;
	uint32_t timeout_cnt = KSV_MAX_COUNT; //5s timeout

	if ((dsCount > 0x7F) || (dsDepth > 0x07)) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Error:: DS device count: %02X, DS device depth: %02X\n", dsCount, dsDepth));
		return SII_MOD_TX_HDCP_EVENT__FAILED;
	}

	hdcpObj->ksvList.length = (dsCount * SII_BKSV_LIST_BYTES);
	fifoByteCounter = hdcpObj->ksvList.length;
	isDone = sTpiKsvListPortionSizeGet(hdcpObj, &bytesToRead);

	while ((fifoByteCounter > 0) && ((bytesToRead != 0) || (isDone == 0)) && (timeout_cnt > 0)) {
		if (bytesToRead > fifoByteCounter) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("KSV list Exp : %d, Act: %d\n", fifoByteCounter, bytesToRead));
		}
		// get DS BKSV list
		if ( bytesToRead ) {
			sTpiKsvListGet(hdcpObj, hdcpObj->ksvList.pList, bytesToRead);
		} else {
			SiiLibTimeMilliDelay(10); // note, the time is aligned
			timeout_cnt--;
		}
		hdcpObj->ksvList.pList += bytesToRead;
		fifoByteCounter -= bytesToRead;
		isDone = sTpiKsvListPortionSizeGet(hdcpObj, &bytesToRead); //No of bytes left in fifo
	}

	return SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READ_DONE;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get DS BSTATUS information.
//!
//! @param[out] pDsBStatus - pointer to a buffer of length 2
//-------------------------------------------------------------------------------------------------

static void sTpiBStatusGet(HdcpObj_t* hdcpObj, uint8_t *pDsBStatus)
{
	SiiDrvCraBlockRead8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_BSTATUS1, pDsBStatus, 2);
}

static void sPrintKsvList(HdcpObj_t* hdcpObj)
{
	uint8_t		*ptrTemp;
	uint16_t	length;

	length = hdcpObj->ksvList.length;
	ptrTemp = hdcpObj->ksvList.pListStart;
	while (length--) {
		SII_LIB_LOG_DEBUG2((" %02X", *ptrTemp));
		ptrTemp++;
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      Show HDCP authentication phase and error messages, if any.
//-------------------------------------------------------------------------------------------------

static void sPrintHdcpStatus(HdcpObj_t* hdcpObj)
{
	if (hdcpObj->prevAuthState != hdcpObj->authState) {
		switch (hdcpObj->authState) {
			case SII_MOD_TX_HDCP_EVENT__OFF:
				SII_LIB_LOG_PRINT1(hdcpObj, ("DS HDCP: OFF\n"));
				break;
			case SII_MOD_TX_HDCP_EVENT__WAIT_FOR_START:
				SII_LIB_LOG_DEBUG1(hdcpObj, ("DS HDCP: WAIT FOR START\n"));
				break;
			case SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE:
				SII_LIB_LOG_DEBUG1(hdcpObj, ("DS HDCP: WAIT FOR DONE\n"));
				break;
			case SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READY:
				SII_LIB_LOG_DEBUG1(hdcpObj, ("DS HDCP: WAIT FOR FIFO READY\n"));
				break;
			case SII_MOD_TX_HDCP_EVENT__AUTHENTICATED:
				SII_LIB_LOG_PRINT1(hdcpObj, ("DS HDCP: AUTHENTICATED\n"));
				break;
			case SII_MOD_TX_HDCP_EVENT__FAILED:
				SII_LIB_LOG_PRINT1(hdcpObj, ("DS HDCP: FAILED\n"));
				break;
			default:
				SII_LIB_LOG_PRINT1(hdcpObj, ("DS HDCP: err state:%d\n", hdcpObj->authState));
				break;
		}
		hdcpObj->prevAuthState = hdcpObj->authState;
	}
}

static void Read_Ri_Debug(HdcpObj_t* hdcpObj, int mode)
{
	SiiInst_t       craInst     = sCraInstGet(hdcpObj);
	uint8_t         reg;
	uint8_t         reg0;

	return;
	reg = SiiDrvCraRdReg8(craInst, REG_ADDR__DDC_MANUAL);
	if (mode) { //read Ri from tx
		reg |= BIT_MSK__DDC_MANUAL__VP_SEL;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Tx Ri:                 "));
	} else { //read Ri from rx
		reg &= ~BIT_MSK__DDC_MANUAL__VP_SEL;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Rx Ri:                 "));
	}
	(void)SiiDrvCraWrReg8(craInst,  REG_ADDR__DDC_MANUAL, reg);
	reg = SiiDrvCraRdReg8(craInst, REG_ADDR__RI_1);
	reg0 = SiiDrvCraRdReg8(craInst, REG_ADDR__RI_2);
	SII_LIB_LOG_DEBUG1(hdcpObj, ("reg[0x52]=%4x, reg[0x53]=%4x\n", (uint32_t)reg, (uint32_t)reg0));
}

static void Read_V_Debug(HdcpObj_t* hdcpObj, int mode)
{
	SiiInst_t       craInst     = sCraInstGet(hdcpObj);
	uint8_t         reg;
	uint8_t i;

	return;
	reg = SiiDrvCraRdReg8(craInst, REG_ADDR__DDC_MANUAL);
	if (mode) { //read Ri from tx
		reg |= BIT_MSK__DDC_MANUAL__VP_SEL;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Tx V':                 "));
	} else { //read Ri from rx
		reg &= ~BIT_MSK__DDC_MANUAL__VP_SEL;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Rx V':                 "));
	}
	(void)SiiDrvCraWrReg8(craInst,  REG_ADDR__DDC_MANUAL, reg);
	for (i = 0; i < 20; i++) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("reg[0x%x]=%4x\n", (uint32_t)(0x30D8 + i), (uint32_t)SiiDrvCraRdReg8(craInst, REG_ADDR__TXVH0_0 + i)));
	}
}

static void Read_HDCP2x_Debug(HdcpObj_t* hdcpObj)
{
	#if 0
	SiiInst_t       craInst     = sCraInstGet(hdcpObj);
	uint8_t         reg[6];

	reg[0] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_AUTH_STAT);
	reg[1] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_STATE);
	reg[2] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_GEN_STATUS);
	reg[3] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_RPT_DETAIL);
	reg[4] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_RPT_DEPTH);
	reg[5] = SiiDrvCraRdReg8(craInst, REG_ADDR__HDCP2X_RPT_DEVCNT);
	SI_HDMI20_PRINT("\nreg[0x1f57080C]:0x%x, 380D:0x%x, 380E:0x%x, 382d:0x%x, 382f:0x%x, 3830:0x%x\n", \
					reg[0], reg[1], reg[2], reg[3], reg[4], reg[5]);
	#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      Show HDCP query status
//-------------------------------------------------------------------------------------------------

static void sPrintHdcpQueryStatus(HdcpObj_t* hdcpObj, uint8_t query)
{
	SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP Query Status:\n"));
	SII_LIB_LOG_DEBUG1(hdcpObj, ("Link:                 "));

	switch (query &  BIT_MSK__TPI_COPP_DATA1__REG_COPP_LINK_STATUS) {
		case BIT_ENUM__TPI_HDCP_QUERY__STATUS_NORMAL:
			SII_LIB_LOG_DEBUG2(("Normal\n"));
			Read_Ri_Debug(hdcpObj, 1);
			Read_Ri_Debug(hdcpObj, 0);
			break;
		case BIT_ENUM__TPI_HDCP_QUERY__STATUS_LOST:
			SII_LIB_LOG_DEBUG2(("Lost\n"));
			Read_Ri_Debug(hdcpObj, 1);
			Read_Ri_Debug(hdcpObj, 0);
			Read_V_Debug(hdcpObj, 1);
			Read_V_Debug(hdcpObj, 0);
			Read_HDCP2x_Debug(hdcpObj);
			break;
		case BIT_ENUM__TPI_HDCP_QUERY__STATUS_FAILED:
			SII_LIB_LOG_DEBUG2(("Failed\n"));
			Read_Ri_Debug(hdcpObj, 1);
			Read_Ri_Debug(hdcpObj, 0);
			Read_V_Debug(hdcpObj, 1);
			Read_V_Debug(hdcpObj, 0);
			Read_HDCP2x_Debug(hdcpObj);
			#if 0
			{
				int i;
				for (i = 0; i < 9; i++) {
					(void)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | (REG_ADDR__TPI_HW_DBG1 + i));
				}
				for (i = 0; i < 16; i++) {
					(void)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | (REG_ADDR__TPI_COPP_DATA1 + i));
				}
			}
			#endif
			break;
			//case BIT_ENUM__TPI_HDCP_QUERY__STATUS_SUSPENDED:
			//	SII_LIB_LOG_DEBUG2(("Suspended\n"));
			//	break;
	}

	SII_LIB_LOG_DEBUG1(hdcpObj, ("DS HDCP:              "));
	if (query & BIT_MSK__TPI_COPP_DATA1__REG_COPP_PROTYPE) {
		SII_LIB_LOG_DEBUG2(("Available\n"));
	} else {
		SII_LIB_LOG_DEBUG2(("Unavailable\n"));
	}

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Repeater:             "));
	if (query & BIT_MSK__TPI_COPP_DATA1__REG_COPP_HDCP_REP) {
		SII_LIB_LOG_DEBUG2(("Yes\n"));
	} else {
		SII_LIB_LOG_DEBUG2(("No\n"));
	}

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Connected Sink Protection:     "));
	if (query & BIT_MSK__TPI_COPP_DATA1__REG_COPP_LPROT) {
		SII_LIB_LOG_DEBUG2(("Yes\n"));
	} else {
		SII_LIB_LOG_DEBUG2(("No\n"));
	}

	SII_LIB_LOG_DEBUG1(hdcpObj, ("Connected Repeater Protection:     "));
	if (query & BIT_MSK__TPI_COPP_DATA1__REG_COPP_GPROT) {
		SII_LIB_LOG_DEBUG2(("Yes\n"));
	} else {
		SII_LIB_LOG_DEBUG2(("No\n"));
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      HDCP Timer event handler.
//!
//!             This function is to be called periodically. The time past from the last call
//!             should be indicated as a parameter.
//-------------------------------------------------------------------------------------------------

static void sHdcpStateMachineHandler(SiiInst_t inst)
{
	HdcpObj_t*		hdcpObj		= (HdcpObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);

	switch (hdcpObj->authState) {
		case SII_MOD_TX_HDCP_EVENT__OFF:
			if (hdcpObj->isAuthRequested) {
				// Check if Downstrem is capable of HDCP2.2
				if (hdcpObj->pConfig->bHdcp2xEn) {
					//if(!hdcpObj->isDsHdcp_2_2_Cap_Read)
					{
						sSetHwTpiBit(hdcpObj);
						sResetTpiStateMachine(hdcpObj);
						#if GET_HDCP22CAP_INPROGRESS
						// Check DDC 0x50 to read the HDCP2.2 capability
						if (hdcpObj->dsHdcp_2_2_Supported != 0x04) {
							if (SII_DDC_ERROR_CODE_NO_ERROR != sReadDsHdcp22Capability(hdcpObj)) {
								// Second attempt to make sure
								if (SII_DDC_ERROR_CODE_NO_ERROR != sReadDsHdcp22Capability(hdcpObj)) {
									//SiiDrvTxDdcReset();
									SII_LIB_LOG_DEBUG1(hdcpObj, ("Error happened during HDCP2.2 Capability read!!"));
								} else {
									hdcpObj->isDsHdcp_2_2_Cap_Read = true;
									SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP2.2 capability has been read successfully!!"));
								}
							} else {
								hdcpObj->isDsHdcp_2_2_Cap_Read = true;
								SII_LIB_LOG_DEBUG1(hdcpObj, ("HDCP2.2 capability has been read successfully!!"));
							}
						}
						#endif
					}
				}
				if ( DsHdcp22SupportGet(hdcpObj) ) {
					SII_LIB_LOG_PRINT1(hdcpObj, ("Down Stream: HDCP2.2 capable!!"));
					sSetHwTpiBit(hdcpObj);
					sResetTpiStateMachine(hdcpObj);
					sStopHdcpOne(hdcpObj);
					sTpiHdcp2ProtectionEnable(hdcpObj, true);
					Hdcp22AuthStart(hdcpObj);
					//Enable HDCP2x Interrupts
					#if 1
					SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B0 \
									 | BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B1 | BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B2 |
									 BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B3 | BIT_MSK__HDCP2X_INTR0_MASK__INTR0_MASK_B6);
					SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B0 \
									 | BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B2 | BIT_MSK__HDCP2X_INTR1_MASK__INTR1_MASK_B5);
					#else
					SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, 0xff);
					SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, 0xff);
					#endif
					hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE;
				} else {
					sClearHwTpiBit(hdcpObj);
					sResetTpiStateMachine(hdcpObj);
					sStopHdcpOne(hdcpObj);
					SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_TPI_HDCP_PREP_EN);
					hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__WAIT_FOR_START;
				}
			}
			break;

		case SII_MOD_TX_HDCP_EVENT__WAIT_FOR_START:
			// TX chip input is provided and stable
			// (If scaler or/and OSD are installed in the schematic,
			// their output signals are provided and stable as well as their input ones)
			if (sTpiIsDownstreamHdcpAvailable(hdcpObj)) { // DDC ACK check
				SII_LIB_LOG_PRINT1(hdcpObj, ("Down Stream: HDCP1.4 capable!!"));
				//Enable Masks for HDCP1x Interrupts
				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_INTR_EN, BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B3 | BIT_MSK__TPI_INTR_EN__REG_TPI_INTR_MASK_B7);

				// DS device must be ready to start HDCP authentication
				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN);
				SiiLibTimeMilliDelay(5);
				sTpiHdcpProtectionEnable(hdcpObj, false); // just in case
				SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_CANCEL_PROT_EN);
				sTpiHdcpProtectionEnable(hdcpObj, true); // start authentication
				//SiiLibTimeMilliDelay(100);

				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE;
				//SiiLibSeqTimerStart(hdcpObj->instTimerIsrPoll, TIMER_START__TX_HDCP__INTR, TIMER_START__TX_HDCP__INTR_INTVAL);
			} else {
				#define HDCP14_REP_TOUT (3)
				uint8_t ttt = 0;
				for (ttt=0;ttt<HDCP14_REP_TOUT;ttt++) {
					uint8_t sts = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5);
					SII_LIB_LOG_PRINT1(hdcpObj, ("Hdcp14 Rep fail:[67D]=0x%x!!",sts));
					if ( sts != 7 ) {
						break;
					}
					SiiLibTimeMilliDelay(5);
				}
				SII_LIB_LOG_PRINT1(hdcpObj, ("Hdcp14 Rep fail:%d %d!!",ttt,HDCP14_REP_TOUT));
				if ( ttt == HDCP14_REP_TOUT ) {
					SiiDrvCraSetBit8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_REAUTH_CTL);
				}
			}

			break;

		case SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READ_DONE:
			if (sTpiHdcpIsPart2Done(hdcpObj)) {
				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
			}
			break;
		case SII_MOD_TX_HDCP_EVENT__AUTHENTICATED:
			//Enabling HDCP Encryption
			//SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
			break;

		case SII_MOD_TX_HDCP_EVENT__FAILED:
			sHdcpReauth(hdcpObj);
			break;

		default:
			break;
	}

	sPrintHdcpStatus(hdcpObj);
}

static void sRcvIdListGet(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint8_t			/*dsDepth = 0, */dsCount = 0;
	uint16_t fifoByteCounter;

	//dsDepth = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DEPTH);
	dsCount = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DEVCNT);
	sResetKsvFifo(hdcpObj);
	hdcpObj->ksvList.length = dsCount * SII_HDCP2X_RCVID_LENGTH;
	fifoByteCounter = hdcpObj->ksvList.length;

	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0, BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD_START); //rcvid_rd_start=1
	while (fifoByteCounter--) {
		*(hdcpObj->ksvList.pList) = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_RPT_RCVID_OUT);
		hdcpObj->ksvList.pList += 1;
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0, BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD); //rcvid_rd=1
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0, BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD); //rcvid_rd=0
	}
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0, BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_RCVID_RD_START); //rcvid_rd_start=0
}

static bool_t IsDSdeviceHDCP2Repeater(HdcpObj_t *hdcpObj)
{
	uint8_t val;
	// return 1 if HDCP 2 repeater
	val = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__HDCP2X_GEN_STATUS);
	return ( val & BIT_MSK__HDCP2X_GEN_STATUS__RO_HDCP2_REPEATER) ? true : false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      HDCP2x Interrupt handler routine.
//!
//!             More detailed description.
//!
//! @param[in]  Hdcp Object
//-------------------------------------------------------------------------------------------------
static void sHdcp2xVirtualIsrHandler(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint8_t hdcp2AuthStatus = 0;
	uint8_t hdcp2AuthStateStatus = 0;
	bool_t rpt_fail = 0;
	SiiModTxHdcpInternalState_t     authStateLast = hdcpObj->authState;

	if (hdcpObj->hdcp2xIntStat.reg1 & BIT_MSK__HDCP2X_INTR1__INTR1_STAT2) { //Ake sent interrupt received
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_0, 0x00);
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_1, 0x00);
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_M_2, 0x00);
		hdcpObj->hdcp2x_seq_num_m = 0;
		hdcpObj->hdcp2x_repeater_ready = false;

		SII_LIB_LOG_DEBUG1(hdcpObj, ("Ake Init sent:[0x%x,0x%x] [0x%x 0x%x 0x%x]\n",hdcpObj->hdcp2xIntStat.reg0,hdcpObj->hdcp2xIntStat.reg1,SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2)));
		SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_INTR2, SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2));
	}

	if ( hdcpObj->isAuthRequested == false ) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("hdpc2x is disabled...\n"));
		return;
	}

	if (hdcpObj->hdcp2xIntStat.reg0 & BIT_MSK__HDCP2X_INTR0__INTR0_STAT2) { //Repeater Ready Interrupt
		// Check repeater parameters
		if (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RPT_DETAIL) & \
				(BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_DEVS_EXC | BIT_MSK__HDCP2X_RPT_DETAIL__RI_HDCP2RX_RPT_MX_CASC_EXC)) {
			// MAX_DEVS_EXCEEDED or MAX_CASCADE_EXCEEDED
			// if MAX_DEV set or MAX_CASC set re-auth after 1.5 secs
			SiiLibTimeMilliDelay(2000);
			rpt_fail = 1;
		}
		hdcpObj->hdcp2x_repeater_ready = true;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Repeater Ready\n"));
	}
	if (hdcpObj->hdcp2xIntStat.reg1 & BIT_MSK__HDCP2X_INTR1__INTR1_STAT0) { //ReceiverID Changed Interrupt
		// seq_num_v check
		uint32_t seq_num_v;
		seq_num_v = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_V_2);
		seq_num_v = (seq_num_v << 8) | SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_V_1);
		seq_num_v = (seq_num_v << 8) | SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_RX_SEQ_NUM_V_0);
		if (seq_num_v == 0xffffff) {
			//Seq_num_v is neither valid nor equal to expected seq_num_v
			SII_LIB_LOG_DEBUG1(hdcpObj, ("Maximum sequence Number\n"));
		}
		SII_LIB_LOG_DEBUG1(hdcpObj, ("seq_num_v:%x \n", seq_num_v));
		//Read ReceiverID List
		sRcvIdListGet(hdcpObj);
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Receiver ID List::  \n"));
		sPrintKsvList(hdcpObj);
		if ( (hdcpObj->hdcp2x_repeater_ready) && (!rpt_fail)) {
			if ((seq_num_v == 0x0000) && (hdcpObj->authState != SII_MOD_TX_HDCP_EVENT__AUTHENTICATED)) {
				sHdcp2streamManageMessageSet(hdcpObj, hdcpObj->hdcpContentType);
			}
		}
	}
	if (hdcpObj->hdcp2xIntStat.reg0 & BIT_MSK__HDCP2X_INTR0__INTR0_STAT3) { //Hash fail
		/*SII_LIB_LOG_DEBUG1(hdcpObj, ("Hash Fail  "));*/
		sHdcp2streamManageMessageSet(hdcpObj, hdcpObj->hdcpContentType);
	}

	if (hdcpObj->hdcp2xIntStat.reg1 & BIT_MSK__HDCP2X_INTR1__INTR1_STAT5) { //stream manage message xfer done Interrupt
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Stream manage message xfer done\n"));
	}
	if (hdcpObj->hdcp2xIntStat.reg0 & BIT_MSK__HDCP2X_INTR0__INTR0_STAT0) { //Authenctication Done Interrupt
		hdcp2AuthStatus = SiiDrvCraRdReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_AUTH_STAT );
		hdcp2AuthStateStatus = SiiDrvCraRdReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_STATE );

		if ((0x81 == hdcp2AuthStatus) && (0x2B == hdcp2AuthStateStatus)) {
			hdcpObj->authFailCounter = 0;
			// HDCP2 Authentication Successfull
			// Check if downstream is a HDCP2 repeater
			if (IsDSdeviceHDCP2Repeater(hdcpObj)) {
				SII_LIB_LOG_DEBUG2(("Downstream repeater\n"));
				/*SiiLibTimeMilliDelay(100); //If repeater, wait atleast 100ms before enabling encryption*/
				SiiLibTimeMilliDelay(100);
			}
			/*SiiLibTimeMilliDelay(20);*/
			if ( hdcpObj->isAuthRequested ) {
				//Enabling HDCP Encryption
				sAvMuteSetDONE(hdcpObj, false, true);
				SiiLibTimeMilliDelay( 30 );
				SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0, BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN);
				//Clear Intr_Encryption
				SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);

				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
				//sAvMuteSetDONE(hdcpObj, false);
				sPrintHdcpStatus(hdcpObj);
				SII_LIB_LOG_DEBUG1(hdcpObj, ("Auth OK\n"));
				if ( authStateLast != SII_MOD_TX_HDCP_EVENT__AUTHENTICATED ) {
					sNotifyHdcpStatus(hdcpObj);
				}
			}
		}
	}

	if ((hdcpObj->hdcp2xIntStat.reg0 & BIT_MSK__HDCP2X_INTR0__INTR0_STAT1) || rpt_fail) {
		hdcpObj->authFailCounter++;
		SII_LIB_LOG_DEBUG1(hdcpObj, ("Auth fail:%d\n",hdcpObj->authFailCounter));
		if ((hdcpObj->authFailCounter > HDCP2X_RETRY_THRESHOLD || rpt_fail) && 0) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("Auth fail\n"));
			hdcpObj->authFailCounter = 0;
			rpt_fail = 0;
			//Disable HDCP Encryption
			SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0, BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN);
			hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
			sHdcpReauth(hdcpObj);
		}
	}
	if (hdcpObj->hdcp2xIntStat.reg0 & BIT_MSK__HDCP2X_INTR0__INTR0_STAT6) {
		hdcp2AuthStatus = SiiDrvCraRdReg8(craInst,  baseAddr |
										  REG_ADDR__HDCP2X_AUTH_STAT );
		hdcp2AuthStateStatus = SiiDrvCraRdReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_STATE );
		if (hdcp2AuthStatus & 0x40) {/* reauth request status*/
			//SII_LIB_LOG_DEBUG1(hdcpObj, ("Reauth Request:0x%x",hdcp2AuthStateStatus));
			#if 0
			{
				uint32_t i;
				SII_LIB_LOG_DEBUG1(hdcpObj, ("Reauth Request:0x%x 0x%x 0x%x\n",SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2)));
				for (i=0;i<256;i++) {
					SII_LIB_LOG_DEBUG1(hdcpObj, ("Reg[%x]=0x%x\n",(baseAddr | REG_ADDR__HDCP2X_CTRL_0)+i,SiiDrvCraRdReg8(craInst, (baseAddr | REG_ADDR__HDCP2X_CTRL_0)+i)));
				}
			}
			#else
				SII_LIB_LOG_DEBUG1(hdcpObj, ("Reauth Request:0x%x 0x%x 0x%x\n",SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1),SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2)));
			#endif
			SiiDrvCraWrReg8(craInst,  baseAddr | REG_ADDR__HDCP2X_INTR2, SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2));
			if (hdcp2AuthStateStatus != 0x41 && hdcp2AuthStateStatus != 0x0) {
				SII_LIB_LOG_DEBUG1(hdcpObj, ("Reauth Request Really:0x%x\n",hdcp2AuthStateStatus));
				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
				sHdcpReauth(hdcpObj);
			}
		}
	}

}

//-------------------------------------------------------------------------------------------------
//! @brief      HDCP Interrupt handler routine.
//!
//!             More detailed description.
//!
//! @param[in]  Hdcp Object.
//-------------------------------------------------------------------------------------------------

static uint8_t sWaitTxHDCPRepeaterReady(HdcpObj_t* hdcpObj)
{
	uint8_t reg8;
	uint8_t rep = 0;
	uint8_t tm = 20;
	do {
		reg8 = SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5);
		rep = (reg8 & 0xf) < 6;
		if ( rep ) {
			SiiLibTimeMilliDelay(10);
			tm --;
			SII_LIB_LOG_DEBUG1(hdcpObj, ("Tx is not in rep state!!\n"));
		} else {
			//SII_LIB_LOG_PRINT1(hdcpObj, ("rep:0x%x,cnt=%d\n",reg8 & 0xf,tm));
			break;
		}
	} while (rep && tm > 0);
	if ( tm == 0 ) {
		SII_LIB_LOG_PRINT1(hdcpObj, ("Rep Rdy Tout!!\n"));
	}
	return 1;
}

static void sVirtualIsrHandler(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	SiiModTxHdcpInternalState_t     authStateLast = hdcpObj->authState;

	if ( hdcpObj->isAuthRequested == false ) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("hdpc1x is disabled...\n"));
		return;
	}

	if (hdcpObj->hdcp1xIntStat.reg0 & BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST7) {
		uint8_t hdcpStatus = sTpiHdcpStatusGet(hdcpObj);

		sPrintHdcpQueryStatus(hdcpObj, hdcpStatus);

		switch (hdcpStatus & BIT_MSK__TPI_COPP_DATA1__REG_COPP_LINK_STATUS) {
			case BIT_ENUM__TPI_HDCP_QUERY__STATUS_NORMAL:
				if (hdcpObj->authState == SII_MOD_TX_HDCP_EVENT__WAIT_FOR_DONE) {
					if (hdcpStatus & BIT_MSK__TPI_COPP_DATA1__REG_COPP_HDCP_REP) {
						hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READY;
					} else {
						if (!(hdcpStatus & BIT_MSK__TPI_COPP_DATA1__REG_COPP_LPROT)) {
							hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
						} else {
							hdcpObj->ksvList.length  += SII_BKSV_LIST_BYTES;
							sTpiBksvGet(hdcpObj, hdcpObj->ksvList.pList);
							hdcpObj->ksvList.pList += SII_BKSV_LIST_BYTES;

							SII_LIB_LOG_DEBUG1(hdcpObj, ("Sink BKSVs::  "));
							sPrintKsvList(hdcpObj);

							//Enabling HDCP Encryption
							SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
							// Part 1 is done and no downstream Part 2 needs to be done
							hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
							sAvMuteSetDONE(hdcpObj, false, false);
						}
					}
				}
				break;

			case BIT_ENUM__TPI_HDCP_QUERY__STATUS_FAILED:
			case BIT_ENUM__TPI_HDCP_QUERY__STATUS_LOST:
				if ( hdcpObj->isAuthRequested ) {
					if ( ! hdcpObj->bAvMute) {
						sAvMuteSet(hdcpObj, true);
					}
					hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
					sNotifyHdcpStatus(hdcpObj);
				}
				break;
		}

		sPrintHdcpStatus(hdcpObj);
		if(hdcpObj->authState == SII_MOD_TX_HDCP_EVENT__AUTHENTICATED) {
			sNotifyHdcpStatus(hdcpObj);
		}
	}

	if (sTpiHdcpStatusGet(hdcpObj) & BIT_MSK__TPI_COPP_DATA1__REG_COPP_HDCP_REP) {
		if (hdcpObj->authState == SII_MOD_TX_HDCP_EVENT__WAIT_FIFO_READY && sWaitTxHDCPRepeaterReady(hdcpObj)) {
			uint8_t aDsBStatus[2];
			uint8_t dev_exceed = 0, casc_exceed = 0;
			SiiLibTimeMilliDelay(10);
			sTpiBStatusGet(hdcpObj, aDsBStatus);
			#if 0
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\naDsBStatus[0] = 0x%x, aDsBStatus[1] = 0x%x\n", aDsBStatus[0], aDsBStatus[1]));
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\naDbg[679] = 0x%x, Dbg[67d] = 0x%x, Dbg[67e] = 0x%x\n", (uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG1), \
										 (uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5), \
										 (uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG6)));
			#endif

			dev_exceed  = aDsBStatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_EXCEED;
			casc_exceed = aDsBStatus[1] & BIT_MSK__TPI_BSTATUS2__REG_DS_CASC_EXCEED;
			if (dev_exceed || casc_exceed) {
				if (dev_exceed) {
					SII_LIB_LOG_DEBUG1(hdcpObj, ("Max DS Devices Exceeded...\n"));
				}
				if (casc_exceed) {
					SII_LIB_LOG_DEBUG1(hdcpObj, ("Max Cascaded Devices Exceeded...\n"));
				}
				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
				sPrintHdcpStatus(hdcpObj);
				sNotifyHdcpStatus(hdcpObj);
			} else if ((aDsBStatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_CNT)) {
				uint32_t tov = 10;
				WAIT_DEV0toN:
				tov = 10;
				if (hdcpObj->hdcp1xIntStat.reg1 & BIT_MSK__TPI_INTR_ST0__TPI_INTR_ST3) {
					//Read KSV Fifo
					sResetKsvFifo(hdcpObj);
					hdcpObj->authState = sTpiGetKSVList(hdcpObj, aDsBStatus);

					SII_LIB_LOG_DEBUG1(hdcpObj, ("Repeater KSV List::  "));
					sPrintKsvList(hdcpObj);

				WAIT_PART2_DONE:
					if (sTpiHdcpIsPart2Done(hdcpObj)) {
						SII_LIB_LOG_DEBUG1(hdcpObj, ("Part2 Authentication Done\n"));
						//Enabling HDCP Encryption
						SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
						//Part2 Authentication is Done
						hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
						sAvMuteSetDONE(hdcpObj, false, false);
					} else {
						if ( tov-- ) {
							SiiLibTimeMilliDelay(50);
							goto WAIT_PART2_DONE;
						} else {
							SII_LIB_LOG_DEBUG1(hdcpObj, ("Part2 timeout RepSts:0x%x!!!\n",SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5)));
						}
						hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
					}

					sPrintHdcpStatus(hdcpObj);
					sNotifyHdcpStatus(hdcpObj);
				}
			} else if (aDsBStatus[1] && 0) {
				hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
				sPrintHdcpStatus(hdcpObj);
				sNotifyHdcpStatus(hdcpObj);
			} else {
				uint32_t tov1 = 100;
			WAIT_PART2_DONE_DEV0:
				sTpiBStatusGet(hdcpObj, aDsBStatus);
				if ( aDsBStatus[0] & BIT_MSK__TPI_BSTATUS1__REG_DS_DEV_CNT ) {
					SII_LIB_LOG_DEBUG1(hdcpObj, ("Repd0 Part2 timeout:repSts:0x%x, Devn=%d,time=%d(ms)!!!\n",SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5),aDsBStatus[0]&0x7f,(100-tov1)*50));
					goto WAIT_DEV0toN;
				}
				if (sTpiHdcpIsPart2Done(hdcpObj)) {
					SII_LIB_LOG_DEBUG1(hdcpObj, ("Repd0 Part2 Authentication Done\n"));
					//Enabling HDCP Encryption
					SiiDrvCraClrBit8(craInst,  baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
					//Part2 Authentication is Done
					hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
					sAvMuteSetDONE(hdcpObj, false, false);
				} else {
					if ( tov1-- ) {
						SiiLibTimeMilliDelay(50);
						goto WAIT_PART2_DONE_DEV0;
					} else {
						SII_LIB_LOG_DEBUG1(hdcpObj, ("Repd0 Part2 timeout:repSts:0x%x!!!\n",SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5)));
					}
					hdcpObj->authState = SII_MOD_TX_HDCP_EVENT__FAILED;
				}
				sPrintHdcpStatus(hdcpObj);
				sNotifyHdcpStatus(hdcpObj);
			}
		}
	}

	// Clear all pending HDCP interrupts
	if (hdcpObj->hdcp1xIntStat.reg0) {
		//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, hdcpObj->hdcp1xIntStat.reg0);
		//hdcpObj->hdcp1xIntStat.reg0 = 0x00;
	}

	if (hdcpObj->hdcp1xIntStat.reg1) {
		//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__TPI_INTR_ST0, hdcpObj->hdcp1xIntStat.reg1);
		//hdcpObj->hdcp1xIntStat.reg1 = 0x00;
	}
	if (hdcpObj->authState == SII_MOD_TX_HDCP_EVENT__FAILED) {
		SII_LIB_LOG_DEBUG1(hdcpObj, ("\nalsts=%d,Dbg[629] = 0x%x,Dbg[679] = 0x%x, Dbg[67d] = 0x%x, Dbg[67e] = 0x%x\n", authStateLast,(uint32_t)sTpiHdcpStatusGet(hdcpObj),(uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG1), \
									 (uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG5), \
									 (uint32_t)SiiDrvCraRdReg8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_HW_DBG6)));
	}
}

static void sHdcp2xIntrHandler(HdcpObj_t *hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	IntStat_t       intStat		= {0};

	intStat.reg0 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0);
	intStat.reg1 = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1);
	SI_HDMI20_PRINT("\n0x3803:0x%x,0x3804=0x%x,0x3805=0x%x\n", (uint32_t)intStat.reg0, (uint32_t)intStat.reg1, \
					(uint32_t)SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR2));

	if (SII_MEMCMP(&hdcpObj->hdcp2xIntStat, &intStat, sizeof(IntStat_t))) {
		uint8_t intreg0 = 0;
		uint8_t intreg1 = 0;

		// Find any interrupt status bit that changed to '1'
		intreg0 |= ((hdcpObj->hdcp2xIntStat.reg0 ^ intStat.reg0) & intStat.reg0);
		intreg1 |= ((hdcpObj->hdcp2xIntStat.reg1 ^ intStat.reg1) & intStat.reg1);

		hdcpObj->hdcp2xIntStat = intStat;
		if (intreg0 | intreg1) {
			// Call derived interrupt handler
			sHdcp2xVirtualIsrHandler(hdcpObj);
			//Clear the Interrupts
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0, hdcpObj->hdcp2xIntStat.reg0);
			SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1, hdcpObj->hdcp2xIntStat.reg1);
			hdcpObj->hdcp2xIntStat.reg0 = 0;
			hdcpObj->hdcp2xIntStat.reg1 = 0;
		}
	}
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
static void sVirtualIsrHandlerMt(HdcpObj_t *hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint8_t sts;

	sts = hdcpObj->EcmIntStat.reg0;
	// Clear all pending hardware TX interrupts
	if (hdcpObj->EcmIntStat.reg0) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__INTR_STATUS, hdcpObj->EcmIntStat.reg0);
		hdcpObj->EcmIntStat.reg0 = 0x00;
	}

	// set intr mask
	if ( sts ) {
		if (( sts & BIT_MSK_REG__EMP_ERR_CPU_MASK) && (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_MASK) & BIT_MSK_REG__EMP_ERR_CPU_MASK)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nirq emp cpu err:0x%x\n", (uint32_t)sts));
		}
		if (( sts & BIT_MSK_REG__EMP_ERR_HDMI_MASK) && (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_MASK) & BIT_MSK_REG__EMP_ERR_HDMI_MASK)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nirq emp send fail:0x%x\n", (uint32_t)sts));
		}
		if (( sts & BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK) && (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_MASK) & BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nirq emp send success:0x%x\n", (uint32_t)sts));
		}
		if (( sts & BIT_MSK_REG__MTW_FALLING_EDGE_MASK) && (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_MASK) & BIT_MSK_REG__MTW_FALLING_EDGE_MASK)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nirq emp mtw falling edge:0x%x\n", (uint32_t)sts));
		}
		if (( sts & BIT_MSK_REG__DMA_DONE_MASK) && (SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__INTR_MASK) & BIT_MSK_REG__DMA_DONE_MASK)) {
			SII_LIB_LOG_DEBUG1(hdcpObj, ("\nirq emp dma done:0x%x\n", (uint32_t)sts));
		}
	}
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable HDCP2 protection.
//!
//! @param[in]  isEnabled - true, if HDCP protection has to be enabled.
//-------------------------------------------------------------------------------------------------
void sTpiHdcp2ProtectionEnable(HdcpObj_t* hdcpObj, bool_t isEnabled)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);

	if (isEnabled) {
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTRL_0,
						 (BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDCPTX | BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDMIMODE));// enabling HDCP2.2 control reg for Tx
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0, BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN); // enabling HDCP2.2 mode
	} else {
		// Revert to HDCP1 mode
		uint8_t protMask = HDCP_CTRL_MODE;
		SiiDrvCraPutBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, protMask, isEnabled ? SET_BITS : CLEAR_BITS);

		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0	, BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN); // disabling HDCP2.2 mode
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTRL_0, (BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDCPTX | BIT_MSK__HDCP2X_CTRL_0__RI_HDCP2_HDMIMODE));// disabling HDCP2.2 control reg for Tx
	}
}

void Hdcp22AuthStart(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);

	// If 0x50 response with HDCP2.2 capable
	if ( DsHdcp22SupportGet(hdcpObj) ) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0, 0x03); // //Clearing auth done , fail interrupts
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTRL_1, 0xE1); //enabling re-authentication --- one pulse
		SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTRL_1, BIT_MSK__HDCP2X_CTRL_1__RI_HDCP2_REAUTH_SW); // disabling re-authentication
	}
	hdcpObj->hdcp2x_repeater_ready = false;
	hdcpObj->hdcp2x_seq_num_m = 0;
}

void Hdcp22AuthStop(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint8_t intStatus;

	hdcpObj->authFailCounter = 0;
	intStatus = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0, intStatus); // //Clearing auth done , fail interrupts

	intStatus = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1);
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1, intStatus);

	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0_MASK, 0x00);
	SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1_MASK, 0x00);
	//Diasabling HDCP2.2 Encryption
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0, (BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_ENCRYPT_EN | BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN));
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__HDCP2X_TX_CTRL_0, BIT_MSK__HDCP2X_TX_CTRL_0__RI_HDCP2TX_RPT_SMNG_XFER_START);
	SiiLibTimeMilliDelay(50);
	//hdcp2x_core_reset
	SiiDrvCraSetBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_TX_SRST, BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_CRST);
	SiiLibTimeMilliDelay(5);
	SiiDrvCraClrBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_TX_SRST, BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_CRST);
	sHdcp2xCodeUpdate(hdcpObj);
}

/******* Update HDCP2.2 Patch********************/
static void sHdcp2xCodeUpdate(HdcpObj_t* hdcpObj)
{
	uint16_t cupdTimeout		= HDCP2X_CUPD_MAX_TIMEOUT;
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	//SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	hdcpObj->hdcp2xCupdStat		= SII_DRV_HDCP2X_CUPD_CHK__ERROR;

	//Disabling HW Code Update :: Pease verify later
	SiiDrvCraClrBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2, BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_HW);

	//If cupd_start is already set :: Remove this Later
	if (SiiDrvCraRdReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2) & BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_START) {
		SiiDrvCraClrBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2, BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_START);
		//hdcp2x_core_reset
		SiiDrvCraSetBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_TX_SRST, BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_CRST);
		SiiLibTimeMilliDelay(5);
		SiiDrvCraClrBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_TX_SRST, BIT_MSK__HDCP2X_TX_SRST__REG_HDCP2X_CRST);
	}

	SiiDrvCraSetBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2, BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_START);
	SiiDrvCraSetBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2, BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_DONE);
	while (--cupdTimeout && (!(SiiDrvCraRdReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_INTR0) & BIT_MSK__HDCP2X_INTR0__INTR0_STAT4))) {
		SiiLibTimeMilliDelay(1);
	}
	if (!cupdTimeout) {
		hdcpObj->hdcp2xCupdStat = SII_DRV_HDCP2X_CUPD_CHK__ERROR;
	} else {
		if (HDCP2X_CCHK_FAIL == (SiiDrvCraRdReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_AUTH_STAT) & HDCP2X_CCHK_FAIL)) {
			hdcpObj->hdcp2xCupdStat = SII_DRV_HDCP2X_CUPD_CHK__FAIL;
		} else {
			hdcpObj->hdcp2xCupdStat = SII_DRV_HDCP2X_CUPD_CHK__DONE;
		}

		//Clear CCHK_DONE interrupt
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_INTR0, BIT_MSK__HDCP2X_INTR0__INTR0_STAT4);
	}
	SiiDrvCraClrBit8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTRL_2, BIT_MSK__HDCP2X_CTRL_2__RI_HDCP2_CUPD_DONE);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read from downstream DDC device.
//!
//!             The function can be used for downstream EDID or HDCP DDC reading.
//!
//! @param[in]  hdcpObj - hdcp object
//!
//! @return     Error code, @see SiiDdcComErr_t
//-------------------------------------------------------------------------------------------------
static SiiDdcComErr_t sReadDsHdcpRead(HdcpObj_t* hdcpObj, uint8_t regAddr, uint8_t *pBuf, uint16_t length)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	SiiDdcComErr_t		dsDdcError  = SII_DDC_ERROR_CODE_NO_ERROR;
	uint16_t		fifoSize;
	uint16_t		timeOutMs;
	bool_t			loop = false;
	uint8_t			segmentIndex = 0xFF;

	do {
		if ( length == 0 ) {
			break;
		}

		if ( !pBuf ) {
			break;
		}

		if (!sWaitForDdcBus(hdcpObj)) {
			dsDdcError = SII_DDC_ERROR_CODE_BUSY;
			SII_LIB_LOG_DEBUG1(hdcpObj, ("DDC TX BUSY--Try later\n"));
			return dsDdcError;
		}

		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
		if ( SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
			SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
		}
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_ADDR, (0xFF != segmentIndex) ? 0xA0 : 0x74 );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_SEGM, (0xFF != segmentIndex) ? segmentIndex : 0x00);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_OFFSET, regAddr);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT2, 0);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT1, (uint8_t)length);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, segmentIndex ? BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ : BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);

		timeOutMs = length + 3; // timeout is proportional to length

		// wait until the FIFO is filled with several bytes
		SiiLibTimeMilliDelay(2); // also makes time aligning

		do {
			fifoSize = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__DDC_DOUT_CNT) & BIT_MSK__DDC_DOUT_CNT__DDC_DATA_OUT_CNT;

			if ( fifoSize ) {
				// if the FIFO has some bytes
				if ( fifoSize > length ) {
					dsDdcError = SII_DDC_ERROR_CODE_TX_HW;
					break;
				} else if ( fifoSize > LEN_TPI__DDC_FIFO_SIZE ) {
					dsDdcError = SII_DDC_ERROR_CODE_LIM_EXCEED;
					break;
				} else {
					// read fifo_size bytes
					SiiDrvCraFifoRead8(craInst, baseAddr | REG_ADDR__DDC_DATA, pBuf, fifoSize);

					length -= fifoSize;
					pBuf += fifoSize;
				}
			} else {
				SiiLibTimeMilliDelay(1); // note, the time is aligned
				timeOutMs--;
			}
		} while (length && timeOutMs);

		if ( dsDdcError ) {
			break;
		}

		if ( 0 == timeOutMs ) {
			dsDdcError = SII_DDC_ERROR_CODE_TIMEOUT;
			SII_LIB_LOG_DEBUG1(hdcpObj, ("DDC TIMEOUT\n"));
		}
	} while (loop);

	if ( dsDdcError ) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION);
	}

	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION | 0x30);
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);

	return dsDdcError;
}

#if GET_HDCP22CAP_INPROGRESS
//-------------------------------------------------------------------------------------------------
//! @brief      Read from downstream DDC device.
//!
//!             The function can be used for downstream EDID or HDCP DDC reading.
//!
//! @param[in]  hdcpObj - hdcp object
//!
//! @return     Error code, @see SiiDdcComErr_t
//-------------------------------------------------------------------------------------------------
static SiiDdcComErr_t sReadDsHdcp22Capability(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	SiiDdcComErr_t		dsDdcError  = SII_DDC_ERROR_CODE_NO_ERROR;
	uint16_t		fifoSize;
	uint16_t		timeOutMs;
	bool_t			loop = false;
	uint8_t			segmentIndex = 0xFF;
	uint8_t			regAddr = 0x50;
	uint8_t			*pBuf = &hdcpObj->dsHdcp_2_2_Supported;
	uint16_t		length = 1;

	do {
		if ( length == 0 ) {
			break;
		}

		if ( !pBuf ) {
			break;
		}

		if (!sWaitForDdcBus(hdcpObj)) {
			dsDdcError = SII_DDC_ERROR_CODE_BUSY;
			SII_LIB_LOG_DEBUG1(hdcpObj, ("DDC TX BUSY--Try later\n"));
			return dsDdcError;
		}

		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
		if ( SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
			SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
		}
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_ADDR, (0xFF != segmentIndex) ? 0xA0 : 0x74 );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_SEGM, (0xFF != segmentIndex) ? segmentIndex : 0x00);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_OFFSET, regAddr);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT2, 0);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT1, (uint8_t)length);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		//SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, segmentIndex ? BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ : BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);

		timeOutMs = length + 3; // timeout is proportional to length

		// wait until the FIFO is filled with several bytes
		SiiLibTimeMilliDelay(2); // also makes time aligning

		do {
			fifoSize = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__DDC_DOUT_CNT) & BIT_MSK__DDC_DOUT_CNT__DDC_DATA_OUT_CNT;

			if ( fifoSize ) {
				// if the FIFO has some bytes
				if ( fifoSize > length ) {
					dsDdcError = SII_DDC_ERROR_CODE_TX_HW;
					break;
				} else if ( fifoSize > LEN_TPI__DDC_FIFO_SIZE ) {
					dsDdcError = SII_DDC_ERROR_CODE_LIM_EXCEED;
					break;
				} else {
					// read fifo_size bytes
					SiiDrvCraFifoRead8(craInst, baseAddr | REG_ADDR__DDC_DATA, pBuf, fifoSize);

					length -= fifoSize;
					pBuf += fifoSize;
				}
			} else {
				SiiLibTimeMilliDelay(1); // note, the time is aligned
				timeOutMs--;
			}
		} while (length && timeOutMs);

		if ( dsDdcError ) {
			break;
		}

		if ( 0 == timeOutMs ) {
			dsDdcError = SII_DDC_ERROR_CODE_TIMEOUT;
			SII_LIB_LOG_DEBUG1(hdcpObj, ("DDC TIMEOUT\n"));
		}
	} while (loop);

	if ( dsDdcError ) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION);
	}

	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION | 0x30);
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	if ( DsHdcp22SupportGet(hdcpObj) == false ) {
		//SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__DDC_CMD, 0x7F);
	}

	return dsDdcError;
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Down stream hdcp2.2 support
//-------------------------------------------------------------------------------------------------

bool_t DsHdcp22SupportGet(HdcpObj_t* hdcpObj)
{
	return (hdcpObj->dsHdcp_2_2_Supported == 0x04 && hdcpObj->pConfig->bHdcp2xEn);
}

static bool_t sWaitForDdcBus(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr    = sbaseAddrGet(hdcpObj);
	SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint8_t         val			= 0;
	uint8_t         time_out    = LEN_TPI__DDC_FIFO_SIZE + 1;

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

#if TPG_RECOVERY_UNUSED_FUNC
//-------------------------------------------------------------------------------------------------
//! @brief      Set dynamic or static Link Integrity mode.
//!
//!             If downstream repeater is discovered, the dynamic mode shall be enabled.
//!             It forces the chip logic to re-authenticate any time the incoming clock
//!             resolution changes. Dynamic mode works for non-repeater sink. However
//!             the authentication may take a little longer time.
//!
//! @param[in]  isEnabled - true for dynamic, false for static mode.
//-------------------------------------------------------------------------------------------------

static void sTpiHdcpDynamicAuthenticationEnable(HdcpObj_t* hdcpObj, bool_t isEnabled)
{
	SiiDrvCraPutBit8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_SC, BIT_MSK__TPI_SC__REG_TPI_REAUTH_CTL, isEnabled ? SET_BITS : CLEAR_BITS);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable HDCP encryption of HDMI data.
//!
//! @param[in]  isEnabled - true: to enable encryption if authentication succeeded,
//!                         false: to disable encryption (even after successful authentication).
//-------------------------------------------------------------------------------------------------
static void sTpiHdcpEncriptionEnable(HdcpObj_t* hdcpObj, bool_t isEnable)
{
	SiiDrvCraPutBit8(sCraInstGet(hdcpObj), sbaseAddrGet(hdcpObj) | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION, isEnable ? SET_BITS : CLEAR_BITS);
}
#endif

static void sNotifyHdcpStatus(HdcpObj_t* hdcpObj)
{
	if (hdcpObj->cbFunc) {
		hdcpObj->cbFunc(hdcpObj->pConfig->instTx, SII_DRV_TX_EVENT__HDCP_STATE_CHNG);
	}
}

/* Mini Uboot supports HDCP, Lite Uboot NOT support HDCP */
#if !defined(CONFIG_TARGET_SYMPHONY6_LITE)

#if defined(CONFIG_MT_CHIP_ETUDE2)
bool_t SiiModTxHdcpLoadKey(SiiInst_t inst, uint8_t *key, uint32_t len)
{
	uint32_t val;
	uint32_t i;

	//write kram_sel to 1
	//val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_CTRL);
	//val |= BIT_MSK__KRAM_SEL;
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, BIT_MSK__KRAM_SEL);

	//write data to sram
	for (i = 0; i < len; i++) {
		#if 0
		val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_CTRL);
		val |= BIT_MSK__KRAM_SEL;
		val &= ~BIT_MSK__KRAM_ADDR;
		val |= (i << 16) & BIT_MSK__KRAM_ADDR;
		val &= ~BIT_MSK__KRAM_DATA;
		val |= key[i] & BIT_MSK__KRAM_DATA;
		SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, val);

		//kram_wr 0->1->0 (interval >= 500ns)
		val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_W_CTRL);
		val &= ~BIT_MSK__KRAM_WR;
		val |= 0 & BIT_MSK__KRAM_WR;
		SiiLibTimeMilliDelay(1);

		val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_W_CTRL);
		val &= ~BIT_MSK__KRAM_WR;
		val |= 1 & BIT_MSK__KRAM_WR;
		SiiLibTimeMilliDelay(1);

		val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_W_CTRL);
		val &= ~BIT_MSK__KRAM_WR;
		val |= 0 & BIT_MSK__KRAM_WR;
		SiiLibTimeMilliDelay(1);
		#else
		SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, BIT_MSK__KRAM_SEL | (i << 16) | key[i]);
		SiiLibTimeMicroDelay(1);
		SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
		SiiLibTimeMicroDelay(1);
		SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, BIT_MSK__KRAM_WR);
		SiiLibTimeMicroDelay(1);
		SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
		SiiLibTimeMicroDelay(1);
		#endif
	}

	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, BIT_MSK__KRAM_SEL | (510 << 16) | 0x00);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, BIT_MSK__KRAM_WR);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
	SiiLibTimeMicroDelay(1);

	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, BIT_MSK__KRAM_SEL | (511 << 16) | 0xff);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, BIT_MSK__KRAM_WR);
	SiiLibTimeMicroDelay(1);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_W_CTRL, 0);
	SiiLibTimeMicroDelay(1);
	//write kram_sel to 0
	//val = SiiDrvCraRdReg32(inst, REG_ADDR__KRAM_CTRL);
	//val &= ~BIT_MSK__KRAM_SEL;
	//SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, val);
	SiiDrvCraWrReg32(inst, REG_ADDR__KRAM_CTRL, 0);

	/* 2021-09-10 added via Liu Xingdi's suggestion*/
	SiiDrvCraClrBit8(inst, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);//clear this bit first via Xingdi's suggestion
	SiiDrvCraSetBit8(inst, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);
	while (BIT_MSK__EPST__OTP_UNLOCKED == (SiiDrvCraRdReg8(inst, REG_ADDR__EPST) & BIT_MSK__EPST__OTP_UNLOCKED)) {
		SiiLibTimeMicroDelay(1);
	}

	return true;
}
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
//#define HDCP_SRAM_NUM (512)
void SiiModTxHdcpLoadKeyValidate(SiiInst_t inst, uint8_t *p_key, uint32_t len)
{
	bool_t ret = FALSE;
	uint32_t i;
	for (i=1;i<6;i++){
		if (p_key[i]!=0) {
			ret = TRUE;
			break;
		}
	}
	//SII_LIB_LOG_DEBUG2(("Ksv all zero %d\n",ret));
	if (ret == TRUE) {
		for (i=9;i<len-1;i++){
			if (p_key[i]!=0) {
				break;
			}
		}
		if (i>=(len-1)) {
			ret = FALSE;
			//SII_LIB_LOG_DEBUG2(("Key list all zero %d\n",ret));
		}
	}
	if (len >= 292) {
		p_key[0] = 0xFF;
		p_key[6] = 0xFF;
		p_key[7] = 0xFF;
		p_key[8] = 0xFF;
		p_key[289] = 0x00;
		p_key[290] = 0x00;
		p_key[291] = 0x00;
	}
	if ( ret == FALSE ) {
		for (i=1;i<6;i++){
			p_key[i] = 0x01;
		}
		for (i=9;i<288;i++){
			p_key[i] = 0x01;
		}
		//SII_LIB_LOG_DEBUG2(("Fill all 0x01\n"));
	}
}

#if __HDMI_OS_KERNEL__
void SiiModTxHdcpOtpWrite(void)
{
	uint32_t data_w = 0;
	bool_t ret = TRUE;

	ret |= ic_verify_reg_wt(REG_ADDR__READ_EN, 0x16885188UL); // read enable: HDCP_SRAM-->CPU
	ret |= ic_verify_reg_wt(REG_ADDR__HCLK_EN, 0x87654321UL); // Write enable: CPU-->HDCP_SRAM

	ret |= (bool_t)ic_verify_reg_rd(REG_ADDR__HDCP_KRAM_BASE + 0x1fcUL, &data_w);
	data_w &= ~(0xffff << 16);
	data_w |= (0xff00 << 16);
	ret |= ic_verify_reg_wt(REG_ADDR__HDCP_KRAM_BASE + 0x1fcUL, data_w);
	ret |= (bool_t)ic_verify_reg_rd(REG_ADDR__HDCP_KRAM_BASE + 0x1fcUL, &data_w);
	/* 2021-09-10 added via Liu Xingdi's suggestion*/
	SiiDrvCraClrBit8((SiiInst_t)NULL, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);//clear this bit first via Xingdi's suggestion
	SiiDrvCraSetBit8((SiiInst_t)NULL, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);
	while (BIT_MSK__EPST__OTP_UNLOCKED == (SiiDrvCraRdReg8((SiiInst_t)NULL, REG_ADDR__EPST) & BIT_MSK__EPST__OTP_UNLOCKED)) {
		SiiLibTimeMicroDelay(1);
	}
	SII_LIB_LOG_DEBUG2(("SiiModTxHdcpOtpWrite 15:0x%x,data_w£º0x%x", SiiDrvCraRdReg8((SiiInst_t)NULL,0x15),data_w));
}
#endif

bool_t SiiModTxHdcpLoadKey(SiiInst_t inst, uint8_t *p_key, uint32_t len)
{
#define HDMI_PKEY_LEN_MAX (512)
	uint32_t i = 0;
	uint32_t data_w = 0;
	uint32_t line_num = 16;
	uint32_t step = 4;
	bool_t ret = TRUE;
	uint8_t p_key_k[HDCP_SRAM_NUM];

	if ( len > HDCP_SRAM_NUM || len == 0 || len < step) {
		SII_LIB_LOG_DEBUG2(("HDCP key len %d > max len %d OR len not step %d allign", len, HDCP_SRAM_NUM, step));
		len = HDCP_SRAM_NUM;
		ret = FALSE;
		return ret;
	}

	ret |= ic_verify_reg_wt(REG_ADDR__READ_EN, 0x16885188UL); // read enable: HDCP_SRAM-->CPU
	ret |= ic_verify_reg_wt(REG_ADDR__HCLK_EN, 0x87654321UL); // Write enable: CPU-->HDCP_SRAM

	// To get HDCP decrypted key data (289B):
	// Padded byte(1B)
	// KSV (5B)
	// Padded byte(3B)
	// Product keys (40 x 7B = 280B)

	{
		memset(p_key_k, 0, sizeof(p_key_k));
		#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__ || __HDMI_OS_RTOS__)
		memcpy(p_key_k, p_key, len);
		#else
		if (copy_from_user(p_key_k, p_key, len)) {
			for (i=0;i<len;i++){
				p_key_k[i] = p_key[i];
			}
		}
		#endif
		#if 0
		for (i=0;i<len-1;i++){
			SII_LIB_LOG_DEBUG2(("%2x ",p_key_k[i]));
			if (i%10==9){
				SII_LIB_LOG_DEBUG2(("\n"));
			}
		}
		#endif
		SiiModTxHdcpLoadKeyValidate(inst, p_key_k, len);
		for (i = 0; i < len - 1; i += step) {
			data_w = (p_key_k[i] + (p_key_k[i + 1] << 8) + (p_key_k[i + 2] << 16) + (p_key_k[i + 3] << 24));
			ret |= ic_verify_reg_wt(REG_ADDR__HDCP_KRAM_BASE + i, data_w);
		}
	}

	ret |= (bool_t)ic_verify_reg_rd(REG_ADDR__HDCP_KRAM_BASE + 0x1fcUL, &data_w);
	data_w &= ~(0xffff << 16);
	data_w |= (0xff00 << 16);
	ret |= ic_verify_reg_wt(REG_ADDR__HDCP_KRAM_BASE + 0x1fcUL, data_w);

	{
		SII_LIB_LOG_DEBUG2(("Read HDCP key after write:\n"));
		for (i = 0; i < HDCP_SRAM_NUM; i += step) {
			if (i % line_num == 0) {
				SII_LIB_LOG_DEBUG2(("hdcp key 0x%03x0: ", i / line_num));
			}
			ret |= (bool_t)ic_verify_reg_rd(REG_ADDR__HDCP_KRAM_BASE + i, &data_w);
			SII_LIB_LOG_DEBUG2(("0x%08x ", data_w));
			if ( ((i + step) % line_num) == 0) {
				SII_LIB_LOG_DEBUG2(("\n"));
			}
		}
		SII_LIB_LOG_DEBUG2(("\n"));
	}

	/* 2021-09-10 added via Liu Xingdi's suggestion*/
	SiiDrvCraClrBit8(inst, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);//clear this bit first via Xingdi's suggestion
	SiiDrvCraSetBit8(inst, REG_ADDR__EPCM, BIT_MSK__EPCM__REG_LD_KSV);
	while (BIT_MSK__EPST__OTP_UNLOCKED == (SiiDrvCraRdReg8(inst, REG_ADDR__EPST) & BIT_MSK__EPST__OTP_UNLOCKED)) {
		SiiLibTimeMicroDelay(1);
	}

	if (ret == TRUE) {
		SII_LIB_LOG_DEBUG2(("load hdcp key ok!!!\n"));
	} else {
		SII_LIB_LOG_DEBUG2(("load hdcp key fail!!!\n"));
	}
	return ret;
}
#else
bool_t SiiModTxHdcpLoadKey(SiiInst_t inst, uint8_t *key, uint32_t len)
{
	SII_LIB_LOG_DEBUG2(("write g_hdcp2x_pram[%d]:0x%x\n", len));
	return true;
}
#endif

static void sHdcp2xCodeUpdatePatch(HdcpObj_t* hdcpObj)
{
	SiiDrvCraAddr_t baseAddr	= sbaseAddrGet(hdcpObj);
	//SiiInst_t		craInst		= sCraInstGet(hdcpObj);
	uint32_t i;

	SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_RAM_SELECT, 0x00);
	SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CUPD_START_ADDR_LO, 0x00);
	SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CUPD_START_ADDR_HI, 0x00);
	SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTL_2, 0x01);
	for (i = 0; i < (sizeof(g_hdcp2x_pram) / sizeof(g_hdcp2x_pram[0])); i++) {
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2_PRAM_DATA, g_hdcp2x_pram[i]);
		if (i % 1024 == 0 || i == ((sizeof(g_hdcp2x_pram) / sizeof(g_hdcp2x_pram[0])) - 1)) {
			SII_LIB_LOG_DEBUG2(("write g_hdcp2x_pram[%d]:0x%x\n", i, g_hdcp2x_pram[i]));
		}
	}

	//read for debug
	#if 0
	{
		uint8_t reg;
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__BIST31_CTRL, 0x00);
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CUPD_START_ADDR_LO, 0xf6);
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CUPD_START_ADDR_HI, 0x3f);
		SiiDrvCraWrReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTL_2, 0x01);
		for (i = 0; i < 10; i++) {
			reg = SiiDrvCraRdReg8(hdcpObj->pConfig->instTxCra, baseAddr | REG_ADDR__HDCP2_PRAM_DATA);
		}
	}
	#endif
}
#endif

/***** end of file ***********************************************************/
