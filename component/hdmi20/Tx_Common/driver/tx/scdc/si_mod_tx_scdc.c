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
* file si_drv_tx_scdc.c
*
* brief Tx SCDC driver
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "sii_time.h"
#include "si_lib_seq_api.h"
#include "si_lib_malloc_api.h"
#include "si_drv_cra_api.h"
#include "si_lib_log_api.h"
#include "si_lib_time_api.h"
#include "si_mod_tx_scdc_api.h"
#include "si_drv_tx_regs.h"

/* scramble states */
#define SCRAMBLE_INITIAL    0
#define SCRAMBLE_WAIT       1
#define SCRAMBLE_ENABLED    2
#define SCRAMBLE_NONE       3

#define PAGE_ADDR           0x3000

/***** Register Module name **************************************************/
SII_LIB_OBJ_MODULE_DEF(drv_tx_scdc);

/***** local definitions ************************************************/
#define SII_DRV_TX_SCDC_EVENT__CHAR_ERR_DET     0x00000001 // SCDC status changed
#define SII_DRV_TX_SCDC_EVENT__TEST_RR_ACK      0x00000002 // Returning response from sink upon 'test read request'

#define SCDC_ID                                                 0xA8

#define DDC_OFFSET__SCDC_SINK_VERSION                           0x01
#define DDC_OFFSET__SCDC_SOURCE_VERSION                         0x02

#define DDC_OFFSET__SCDC_UPDATE_0                               0x10
#define BIT_MASK__UPDATE_0__STATUS_UPDATE                       0x01
#define BIT_MASK__UPDATE_0__CED_UPDATE                          0x02

#define BIT_MASK__UPDATE_0__RR_TEST                         0x04

#define DDC_OFFSET__SCDC_UPDATE_1                               0x11

#define DDC_OFFSET__SCDC_TMDS_CONFIG                            0x20
#define BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE                  0x01
#define BIT_MASK__TMDS_CONFIG__BIT_CLOCK_RATIO                  0x02

#define DDC_OFFSET__SCDC_TMDS_STATUS                            0x21
#define BIT_MASK__TMDS_STATUS__SCRAMBLE_STATUS                  0x01

#define DDC_OFFSET__SCDC_CONGIG_0                               0x30
#define BIT_MASK__CONGIG_0__RR_ENABLE                           0x01

#define DDC_OFFSET__SCDC_STATUS_FLAGS_0                         0x40
#define BIT_MASK__STATUS_FLAGS__CLK_DETECTED                    0x01
#define BIT_MASK__STATUS_FLAGS__CH0_LOCKED                      0x02
#define BIT_MASK__STATUS_FLAGS__CH1_LOCKED                      0x04
#define BIT_MASK__STATUS_FLAGS__CH2_LOCKED                      0x08

#define DDC_OFFSET__SCDC_CHANNEL_0_ERROR_COUNT                  0x50
#define DDC_OFFSET__SCDC_CHANNEL_0_ERROR_COUNT_VALID            0x51
#define DDC_OFFSET__SCDC_CHANNEL_1_ERROR_COUNT                  0x52
#define DDC_OFFSET__SCDC_CHANNEL_1_ERROR_COUNT_VALID            0x53
#define DDC_OFFSET__SCDC_CHANNEL_2_ERROR_COUNT                  0x54
#define DDC_OFFSET__SCDC_CHANNEL_2_ERROR_COUNT_VALID            0X55

#define DDC_OFFSET__SCDC_TEST_CONFIG_0                          0xC0
#define BIT_MASK__TEST_CONFIG_0__TEST_READ_REQUEST              0x80
#define BIT_MASK__TEST_CONFIG_0__TRR_DELAY_MS                   0x7F

#define DDC_OFFSET__SCDC_MANF_OUI_3                             0xD0
#define DDC_OFFSET__SCDC_MANF_OUI_2                             0xD1
#define DDC_OFFSET__SCDC_MANF_OUI_1                             0xD2

#define DDC_OFFSET__SCDC_MANF_DEV_ID_STR                        0xD3

#define DDC_OFFSET__SCDC_MANF_DEV_ID_HW_REV                     0xDB
#define BIT_MASK__MANF_DEV_ID_HW_REV__MAJOR                     0xF0
#define BIT_MASK__MANF_DEV_ID_HW_REV__MINOR                     0x0F

#define DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MAJOR_REV               0xDC
#define DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MINOR_REV               0xDD

//Page 0:0xF3 - REG_ADDR__DDC_CMD
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_READ				0x02
#define BIT_ENUM__DDC_CMD__ENHANCED_DDC_READ			0x04
#define BIT_ENUM__DDC_CMD__SEQUENTIAL_WRITE				0x06
#define BIT_ENUM__DDC_CMD__CLEAR_FIFO					0x09
#define BIT_ENUM__DDC_CMD__ABORT_TRANSACTION			0x0F

typedef struct {
	SiiInst_t              parentInst;  // Parent instance. Provided when with call back
	SiiModTxScdcConfig_t   *config;     // Static instance configuration

	SiiInst_t   timerScrambleExpire;    // max wait for scrambling enabled
	SiiInst_t   timerScramblePoll;      // polling for scrambling enabled
	//SiiInst_t   timerRrIntPoll;       // polling for Read Request Interrupt
	SiiInst_t   timerupdateflagscan;    // polling for Auto update flag
	//uint16_t    vclk_mb;              // video clock in MB, submitted by app layer

	bit_fld_t   sc_wait_expired;        // flag( 1 - expired, 0 - not expired )
	uint8_t     sc_state;               // (0 - SCRAMBLE_INITIAL, 1 - SCRAMBLE_WAIT, 2 - SCRAMBLE_ENABLE, 3 - SCRAMBLE_NONE)
	uint8_t     sinkScStatus;           // 0 - disabled , 1 - enabled.
	uint8_t     scdcUpdate0;
	uint8_t     scdcUpdate1;
	uint32_t    scdc_events;
	SiiLibScdcSinKCaps_t sinkCaps;
	SiiDrvTxScdcManufacturerStatus_t peerManfStatus;
	SiiDrvTxScdcRegisterStatus_t scdcregStatus;
	SiiDrvTxScdcScrmbleclkStatus_t scmbleclkStatus;
	bool_t	srcCrAbove340m;
} ScdcObj_t;

static uint8_t updateFlag0_prev = 0;
/***** local prototypes ******************************************************/

static void sScrambleTimeout(SiiInst_t inst);
static void sScdcHandler(SiiInst_t inst);
static void sWriteScdcReg8(ScdcObj_t* pObj, uint8_t addr, uint8_t val);
static uint8_t sReadScdcReg8(ScdcObj_t* pObj, uint8_t addr);
static uint8_t sScdcSinkVersionGet(SiiInst_t inst);
static void sScdcRegReadTest(ScdcObj_t *pObj);
static void sScdcManufacturerRegReadTest(ScdcObj_t *pObj);
static void sScdcScramblingAndClockStatus(ScdcObj_t *pObj);
static void sScdcReadupdateflags(SiiInst_t inst);
static void sScdcAutoPollingmode(ScdcObj_t *pObj);
static void sScdcResetUpdateRegisters(ScdcObj_t *pObj);
static void sScdcScrambleenable(ScdcObj_t *pObj, SiiLibScdcSinKCaps_t* sinkCaps);
static void sScdcScrambleDisable(ScdcObj_t *pObj, SiiLibScdcSinKCaps_t* sinkCaps);
//static void sScdcReadRequestTest(SiiInst_t inst, SiiLibTimeMilli_t msDelay);
static void sScdcReadRequestTest(ScdcObj_t *pObj, SiiLibTimeMilli_t msDelay);
static void sScdcSourceVersionSet(SiiInst_t inst, uint8_t ver);
static void sScdcReadRequestEnable(SiiInst_t inst, uint8_t enable);
static void sScdcReset(SiiInst_t inst);
static void sScdcSinkCapsSet(SiiInst_t inst, SiiLibScdcSinKCaps_t* sinkCaps);
static void sScdcSrcCrSet(SiiInst_t inst, bool_t cr);
static void sScdcRegReadCeds(ScdcObj_t *pObj);
static void sScdcRegReadUpdateFlgs(ScdcObj_t *pObj, uint8_t *updata);

//static void sScdcInterruptHandler(SiiInst_t inst);

/***** local data objects ****************************************************/

/***** public functions ******************************************************/

SiiInst_t SiiModTxScdcCreate(char *pNameStr, SiiInst_t parentInst, SiiModTxScdcConfig_t *pConfig)
{
	ScdcObj_t*  pObj = NULL;
	SiiDrvCraAddr_t baseAddr;
	/* Allocate memory for object */
	pObj = (ScdcObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(ScdcObj_t));
	SII_PLATFORM_DEBUG_ASSERT(pObj);

	pObj->parentInst = parentInst;
	pObj->config = (SiiModTxScdcConfig_t*)SiiLibMallocCreate(sizeof(SiiModTxScdcConfig_t));
	if ( pObj->config ) {
		memcpy(pObj->config, pConfig, sizeof(SiiModTxScdcConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiModTxScdcCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	baseAddr = pObj->config->baseAddr;

	//ENABLE INTERRUPT AND auto reply mode

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY); // enable SCDC auto reply read request from slave for SCDC registers up_flag0 and up_flag1
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK, (BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK2 |
					BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3 |
					BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK4)); // enable interrupt for update flage0 and flag1
	//	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK, 0x3F);

	//Create timers for scrambling control
	pObj->timerScrambleExpire = SII_LIB_SEQ_TIMER_CREATE("SCR_Wait_Expire", sScrambleTimeout, SII_LIB_OBJ_INST(pObj), 189);
	SII_PLATFORM_DEBUG_ASSERT(pObj->timerScrambleExpire);

	pObj->timerScramblePoll = SII_LIB_SEQ_TIMER_CREATE("SCR_Ready_Poll", sScdcHandler, SII_LIB_OBJ_INST(pObj), 190);
	SII_PLATFORM_DEBUG_ASSERT(pObj->timerScramblePoll);

	//Create timers to read SCDC_UP_FLAG0 and SCDC_UP_FLAG1
	pObj->timerupdateflagscan = SII_LIB_SEQ_TIMER_CREATE("SCDC_Read_Updateflags", sScdcReadupdateflags, SII_LIB_OBJ_INST(pObj), 189);
	SII_PLATFORM_DEBUG_ASSERT(pObj->timerupdateflagscan);

	//	pObj->timerRrIntPoll = SII_LIB_SEQ_TIMER_CREATE("SCDC_Int_Poll", sScdcInterruptHandler, SII_LIB_OBJ_INST(pObj), 188);
	//	SII_PLATFORM_DEBUG_ASSERT(pObj->timerRrIntPoll);

	return SII_LIB_OBJ_INST(pObj);
}

void SiiModTxScdcDelete(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	SiiLibSeqTimerStop(pObj->timerupdateflagscan);
	SiiLibSeqTimerDelete(pObj->timerupdateflagscan);

	SiiLibSeqTimerStop(pObj->timerScramblePoll);
	SiiLibSeqTimerDelete(pObj->timerScramblePoll);

	SiiLibSeqTimerStop(pObj->timerScrambleExpire);
	SiiLibSeqTimerDelete(pObj->timerScrambleExpire);

	SiiLibMallocDelete(pObj->config);
	SII_LIB_OBJ_DELETE(pObj);
}

bool_t SiiModTxScdcSet(SiiInst_t inst, SiiModTxScdcOpcode_t opcode, void *inData)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	if (pObj == NULL) {
		return false;
	}

	switch (opcode) {
		case SII_MOD_TX_SCDC_OPCODE__RESET_UPDATE_REGISTER:
			sScdcResetUpdateRegisters(pObj);
			break;
		case SII_MOD_TX_SCDC_OPCODE__ENABLE_AUTOPOLL_MODE:
			sScdcAutoPollingmode(pObj);
			break;
		case SII_MOD_TX_SCDC_OPCODE__ENABLE_READ_REQ_TEST: {
			uint32_t Delay = * (uint32_t *)inData;
			sScdcReadRequestTest(pObj, Delay);
			break;
		}
		case SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_ENABLE: {
			SiiLibScdcSinKCaps_t* sinkCapability = (SiiLibScdcSinKCaps_t*)inData;
			sScdcScrambleenable(pObj, sinkCapability);
			break;
		}
		case SII_MOD_TX_SCDC_OPCODE__SCDC_SCRAMBLE_DISABLE: {
			SiiLibScdcSinKCaps_t* sinkCapability = (SiiLibScdcSinKCaps_t*)inData;
			sScdcScrambleDisable(pObj, sinkCapability);
			break;
		}
		case SII_MOD_TX_SCDC_OPCODE__SCDC_RESET_CAPABILITY: {
			sScdcReset(inst);
			break;
		}
		case SII_MOD_TX_SCDC_OPCODE__SCDC_SINK_CAPABILITY: {
			SiiLibScdcSinKCaps_t* sinkCapability = (SiiLibScdcSinKCaps_t*)inData;
			sScdcSinkCapsSet(inst, sinkCapability);
			break;
		}
		case SII_MOD_TX_SCDC_OPCODE__SCDC_SRC_CR: {
			bool_t cr = *(bool_t*)inData;
			sScdcSrcCrSet(inst, cr);
			break;
		}
		default:
			SII_LIB_LOG_DEBUG1(pObj, ("SCDC set error opcode:%d \n", opcode));
			break;
	}
	return true;
}

bool_t SiiModTxScdcGet(SiiInst_t inst, SiiModTxScdcOpcode_t opcode, void *outData)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	switch (opcode) {
		case SII_MOD_TX_SCDC_OPCODE__PEER_MANF_STATUS:
			sScdcManufacturerRegReadTest(pObj);
			SII_MEMCPY(outData, &pObj->peerManfStatus, sizeof(SiiDrvTxScdcManufacturerStatus_t));
			break;
		case SII_MOD_TX_SCDC_OPCODE__PEER_SCDC_STATUS:
			sScdcRegReadTest(pObj);
			SII_MEMCPY(outData, &pObj->scdcregStatus, sizeof(SiiDrvTxScdcRegisterStatus_t));
			break;
		case SII_MOD_TX_SCDC_OPCODE__SCRAMBLE_CLOCK_STATUS:
			sScdcScramblingAndClockStatus(pObj);
			SII_MEMCPY(outData, &pObj->scmbleclkStatus, sizeof(SiiDrvTxScdcScrmbleclkStatus_t));
			break;
		default:
			SII_LIB_LOG_DEBUG1(pObj, ("SCDC get error opcode:%d \n", opcode));
			break;
	}
	return true;
}
/***** local static functions ******************************************************/

static void sScdcReset(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	SII_MEMSET(&pObj->sinkCaps, 0, sizeof(SiiLibScdcSinKCaps_t));

	pObj->sc_state = SCRAMBLE_INITIAL;

	sScdcHandler(inst);

	pObj->sc_state = SCRAMBLE_INITIAL;
	pObj->srcCrAbove340m = 0;
}

static void sWriteScdcReg8(ScdcObj_t* pObj, uint8_t addr, uint8_t val)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	SiiLibTimeMilli_t ddcGrabTime;

	#ifdef CONFIG_MT_CHIP_ETUDE2
	SiiLibTimeOutMilliSet( &ddcGrabTime, 500 );
	#else
	SiiLibTimeOutMilliSet( &ddcGrabTime, 10 );
	#endif

	#if 1
	do {
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2); // Clear DDC Conflict Interrupt
		SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Clear SCDC DDC selection
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Select SCDC DDC cycle
		if ( SiiLibTimeOutMilliIs(&ddcGrabTime) ) {
			SII_LIB_LOG_DEBUG1(pObj, ("DDC Busy: Scdc Reg Write Fail \n"));
			break;
		}
	} while ( (SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0) & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2) ); // Check for DDC conflict interrupt

	#endif
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	//SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO | 0x30);
	if ( SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	}
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_ADDR, 0xA8);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_SEGM, 0);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_OFFSET, addr);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_DIN_CNT2, 0);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_DIN_CNT1, 1);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO | 0x30);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_DATA, val);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__SEQUENTIAL_WRITE | 0x30);

	SiiLibTimeMilliDelay(5);

	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	#if 1
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION | 0x30);
	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_CMD, 0x7F);
	#endif

}

static SiiDdcComErr_t sReadScdcBytes(ScdcObj_t* pObj, uint8_t addr, uint8_t *pBuf, uint16_t length)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	SiiInst_t		craInst		= pObj->config->instTxCra;
	SiiDdcComErr_t	dsDdcError  = SII_DDC_ERROR_CODE_NO_ERROR;
	uint16_t        fifoSize;
	uint16_t        timeOutMs;
	bool_t			loop = false;
	SiiLibTimeMilli_t ddcGrabTime;

	do {
		if ( length == 0 ) {
			break;
		}

		if ( !pBuf ) {
			break;
		}

		#ifdef CONFIG_MT_CHIP_ETUDE2
		SiiLibTimeOutMilliSet( &ddcGrabTime, 500 );
		#else
		SiiLibTimeOutMilliSet( &ddcGrabTime, 10 );
		#endif

		do {
			SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2); // Clear DDC Conflict Interrupt
			SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Clear SCDC DDC selection
			SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Select SCDC DDC cycle
			if ( SiiLibTimeOutMilliIs(&ddcGrabTime) ) {
				SII_LIB_LOG_DEBUG1(pObj, ("DDC Busy: Scdc Reg Read Fail \n"));
				break;
			}
		} while ( (SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0) & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2) ); // Check for DDC conflict interrupt

		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
		if ( SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
			SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
		}
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_ADDR, 0xA8 );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_SEGM, 0);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_OFFSET, addr);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT2, ((length >> 8) & 0xFF) );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_DIN_CNT1,  (length & 0xFF) );
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO);
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__SEQUENTIAL_READ);

		timeOutMs = length + 3; // timeout is proportional to length

		// wait until the FIFO is filled with several bytes
		SiiLibTimeMilliDelay(2); // also makes time aligning

		do {
			fifoSize = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__DDC_DOUT_CNT) & BIT_MSK__DDC_DOUT_CNT__DDC_DATA_OUT_CNT;

			if ( fifoSize ) {
				// if the FIFO has some bytes
				if ( fifoSize > length ) {
					SII_LIB_LOG_DEBUG1(pObj, ("DDC Error: FIFO Size Exceeded Length\n"));
					dsDdcError = SII_DDC_ERROR_CODE_TX_HW;
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
			SII_LIB_LOG_DEBUG1(pObj, ("DDC Error:: Timedout\n"));
			dsDdcError = SII_DDC_ERROR_CODE_TIMEOUT;
		}
	} while (loop);

	if ( dsDdcError ) {
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION);
	}

	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	//SiiDrvCraClrBit8(craInst, baseAddr | REG_ADDR__DDC_CMD, 0x7F);

	return dsDdcError;
}

static uint8_t sReadScdcReg8(ScdcObj_t* pObj, uint8_t addr)
{
	uint8_t rz = 0;

	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	SiiLibTimeMilli_t ddcGrabTime;

	#ifdef CONFIG_MT_CHIP_ETUDE2
	SiiLibTimeOutMilliSet( &ddcGrabTime, 500 );
	#else
	SiiLibTimeOutMilliSet( &ddcGrabTime, 10 );
	#endif

	do {
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2); // Clear DDC Conflict Interrupt
		SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Clear SCDC DDC selection
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_ACCESS);     // Select SCDC DDC cycle
		if ( SiiLibTimeOutMilliIs(&ddcGrabTime) ) {
			SII_LIB_LOG_DEBUG1(pObj, ("DDC Busy: Scdc Reg Read Fail \n"));
			break;
		}
	} while ( (SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0) & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT2) ); // Check for DDC conflict interrupt

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr |  REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	if ( SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__HDCP2X_CTL_0) & BIT_MSK__HDCP2X_CTL_0__REG_HDCP2X_EN ) {
		SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	}
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_ADDR, 0xA8);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_SEGM, 0);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_OFFSET, addr);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_DIN_CNT2, 0);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_DIN_CNT1, 1);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__CLEAR_FIFO | 0x30);
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__SEQUENTIAL_READ | 0x30);

	SiiLibTimeMilliDelay(2);

	//	while( !(SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0) & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT0) );

	//	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT0);

	rz = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_DATA);

	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_HW_OPT3, BIT_MSK__TPI_HW_OPT3__REG_DDC_DEBUG);
	// Disable DDC Master
	#if 1
	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr |   REG_ADDR__DDC_CMD, BIT_ENUM__DDC_CMD__ABORT_TRANSACTION | 0x30);
	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__TPI_DDC_MASTER_EN, BIT_MSK__TPI_DDC_MASTER_EN__REG_HW_DDC_MASTER);
	SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__DDC_CMD, 0x7F);
	#endif

	return rz;
}

static void sScdcScrambleenable(ScdcObj_t *pObj, SiiLibScdcSinKCaps_t* sinkCaps)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;

	if (sinkCaps) {
		pObj->sinkCaps = *sinkCaps;
	}

	SII_LIB_LOG_DEBUG1(pObj, ("%s_%d:%d,%d\n", __func__, __LINE__, (uint32_t)pObj->sinkCaps.vclk_mb, (uint32_t)pObj->sinkCaps.bScdcPresent));
	if (pObj->sinkCaps.vclk_mb > 340) {
		if (pObj->sinkCaps.bScdcPresent) {
			if (pObj->srcCrAbove340m) {
				//enable 1/4 bit clock in the sink by writing 1 into SCDC reg.
				//enable scrambling in the sink by writing 1 into SCDC reg.
				sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE | BIT_MASK__TMDS_CONFIG__BIT_CLOCK_RATIO);

				//enable scrambling in TX
				SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);
				pObj->sc_state = SCRAMBLE_ENABLED;
				SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON\n"));
			} else {
				if (pObj->sinkCaps.bLTE340MscsScramble) {
					sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE);
					//enable scrambling in TX
					SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);
					pObj->sc_state = SCRAMBLE_ENABLED;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON\n"));
				} else {
					//disable scrambling in the sink by writing 0 into SCDC reg.
					sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);
					//disable scrambling in TX
					SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON);
					pObj->sc_state = SCRAMBLE_NONE;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
				}
			}
		} else {
			//enable scrambling in TX
			SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);

			pObj->sc_state = SCRAMBLE_ENABLED;
			SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON, Sink NOT Support SCDC!!!\n"));
		}
	} else {
		if (pObj->sinkCaps.bScdcPresent) {
			//disable scrambling in the sink by writing 0 into SCDC reg.
			sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);
		}

		//disable scrambling in TX
		SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON);
		pObj->sc_state = SCRAMBLE_NONE;
		SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
	}

}

static void sScdcScrambleDisable(ScdcObj_t *pObj, SiiLibScdcSinKCaps_t* sinkCaps)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;

	if (sinkCaps) {
		pObj->sinkCaps = *sinkCaps;
	}

	SII_LIB_LOG_DEBUG1(pObj, ("%s_%d:%d,%d\n", __func__, __LINE__, (uint32_t)pObj->sinkCaps.vclk_mb, (uint32_t)pObj->sinkCaps.bScdcPresent));
	if (pObj->sinkCaps.vclk_mb > 340) {

		if (pObj->sinkCaps.bScdcPresent) {
			//disable 1/4 bit clock in the sink by writing 1 into SCDC reg.
			//disable scrambling in the sink by writing 1 into SCDC reg.
			sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);

			//disable scrambling in TX
			//SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_HDMI2_ON);
			SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON);

			//pObj->sc_state = SCRAMBLE_WAIT;
			//pObj->sc_wait_expired = 0;

			//set timer for 200MS ( will make sc_wait_expired == 1, when hit )
			//SiiLibSeqTimerStart(pObj->timerScrambleExpire, 200, 0);

			//continue;
		} else {
			//disable scrambling in TX
			//SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_HDMI2_ON);
			SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON);

			pObj->sc_state = SCRAMBLE_NONE;
			SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
		}
	} else {
		if (pObj->sinkCaps.bScdcPresent) {
			//disable scrambling in the sink by writing 0 into SCDC reg.
			sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);
		}

		//disable scrambling in TX
		SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON);
		pObj->sc_state = SCRAMBLE_NONE;
		SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
	}

}

static void sScdcHandler(SiiInst_t inst)
{

	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;

	SII_LIB_LOG_DEBUG1(pObj, ("%s_%d:sc=%d,mMcsc=%d,scdc=%d,LTE340=%d\n", __func__, __LINE__, (uint32_t)pObj->sc_state, (uint32_t)pObj->sinkCaps.vclk_mb, (uint32_t)pObj->sinkCaps.bScdcPresent, \
							  (uint32_t)pObj->sinkCaps.bLTE340MscsScramble));
	for (;;) {
		if (pObj->sc_state == SCRAMBLE_INITIAL) {
			// disable scrambling in TX
			SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);

			if (pObj->sinkCaps.vclk_mb > 340) {
				if (pObj->sinkCaps.bScdcPresent) {
					//enable 1/4 bit clock in the sink by writing 1 into SCDC reg.
					//enable scrambling in the sink by writing 1 into SCDC reg.
					//sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE | BIT_MASK__TMDS_CONFIG__BIT_CLOCK_RATIO);

					//enable scrambling in TX
					SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);
					sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE);

					pObj->sc_state = SCRAMBLE_WAIT;
					pObj->sc_wait_expired = 0;

					//set timer for 200MS ( will make sc_wait_expired == 1, when hit )
					SiiLibSeqTimerStart(pObj->timerScrambleExpire, 200, 0);

					continue;
				} else {
					//enable scrambling in TX
					SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);

					pObj->sc_state = SCRAMBLE_ENABLED;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON\n"));
					break;
				}
			} else {
				if (pObj->sinkCaps.bScdcPresent) {

					if (pObj->sinkCaps.bLTE340MscsScramble) {
						//enable scrambling in the sink by writing 1 into SCDC reg.

						//sWriteScdcReg8(pObj, DDC_OFFSET_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE);
						sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE);

						//enable scrambling in TX
						SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);

						pObj->sc_state = SCRAMBLE_WAIT;
						pObj->sc_wait_expired = 0;
						//set timer for 200MS ( will make sc_wait_expired == 1 when hit )
						SiiLibSeqTimerStart(pObj->timerScrambleExpire, 200, 0);

						continue;
					} else {
						//disable scrambling in the sink by writing 0 into SCDC reg.
						sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);

						//disable scrambling in TX.
						SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);

						pObj->sc_state = SCRAMBLE_NONE;
						SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
						break;
					}
				} else {
					//disable scrambling in the sink by writing 0 into SCDC reg.
					sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);

					//disable scrambling in TX
					SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);
					pObj->sc_state = SCRAMBLE_NONE;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE NONE\n"));
					break;
				}
			}
		}

		if (pObj->sc_state == SCRAMBLE_WAIT) {
			if (pObj->sc_wait_expired) {
				//disable scrambling in the sink by writing 0 into SCDC reg
				sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG, 0);

				if (pObj->sinkCaps.vclk_mb <= 340) {
					/*disable scrambling in TX */
					SiiDrvCraClrBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL, BIT_MSK__SCRCTL__REG_SCR_ON | BIT_MSK__SCRCTL__REG_HDMI2_ON);
					pObj->sc_state = SCRAMBLE_NONE;

					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE OFF\n"));
				} else {
					pObj->sc_state = SCRAMBLE_ENABLED;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON\n"));
				}
				break;
			} else {
				uint8_t scr_status;

				scr_status = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_STATUS);
				if ( scr_status & BIT_MASK__TMDS_STATUS__SCRAMBLE_STATUS ) {
					pObj->sc_state = SCRAMBLE_ENABLED;
					SII_LIB_LOG_DEBUG1(pObj, ("SCRAMBLE ON :: %02x\n", scr_status));
					break;
				} else {
					//stay in the same state
					//continue processing in 10MS
					SiiLibSeqTimerStart(pObj->timerScramblePoll, 10, 0);
					break;
				}
			}
		}
		break;
	}
}

static void sScrambleTimeout(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	pObj->sc_wait_expired = true;
}

static void sScdcReadupdateflags(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	uint8_t updateFlag0;

	updateFlag0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG0);

	if (updateFlag0 != updateFlag0_prev) {
		updateFlag0_prev = updateFlag0;
		SII_LIB_LOG_DEBUG1(pObj, ("\n-----Update Flags:%02X\n", updateFlag0));
	} else {
		SII_LIB_LOG_DEBUG1(pObj, ("\n no achange in update flag registers..\n"));
	}

	//this will not change as all fields are reserved
	//updateFlag1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG1);
	//check with previous values for difference

}

static void sScdcReadRequestTest(ScdcObj_t* pObj, SiiLibTimeMilli_t msDelay)
{
	//uint8_t updateFlag0,updateFlag1,l1_intr_stat_1;
	// ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	//uint8_t scdcInt;
	// uint8_t status0,status1,contrlReg;

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3); // Clear Slave Read Requrest Interrupt

	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TEST_CONFIG_0, (BIT_MASK__TEST_CONFIG_0__TEST_READ_REQUEST | (uint8_t)msDelay));

	// delay for 5 msec.
	SiiLibTimeMilliDelay(msDelay + 2); //swarupa comment :: why added 2 ??
	/*
	do
	{
	scdcInt = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	} while( !(scdcInt & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3) );

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3); // Clear Slave Read Requrest Interrupt

	do
	{
	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);
	} while( !(updateFlag0 & BIT_MASK__UPDATE_0__RR_TEST) );
	*/

	#if 0 //should get interrupt

	contrlReg = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL);
	status0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	status1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK);
	l1_intr_stat_1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__L1_INTR_STAT_0);

	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);
	updateFlag1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG0);
	if (status0 & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3) {
		updateFlag0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG0);

		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test PASSED******\n"));

		//updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);
		//SII_LIB_LOG_DEBUG1(pObj, ("\nJust for update flag write confirmation %02x\n", updateFlag0));
	} else {
		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test FAILED******\n"));
	}

	#endif

	#if 0  //swarupa commented
	contrlReg = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL);
	status0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	status1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK);
	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);

	l1_intr_stat_1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__L1_INTR_STAT_0);

	SiiDrvCraWrReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, 0x00 );
	status0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	l1_intr_stat_1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__L1_INTR_STAT_0);

	if (updateFlag0 & BIT_MASK__UPDATE_0__RR_TEST) {
		sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, BIT_MASK__UPDATE_0__RR_TEST);

		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test PASSED******\n"));

		//updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);
		//SII_LIB_LOG_DEBUG1(pObj, ("\nJust for update flag write confirmation %02x\n", updateFlag0));
	} else {
		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test FAILED******\n"));
	}

	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_TEST_CONFIG_0, CLEAR_BITS);
	#endif
}

uint8_t sScdcSinkVersionGet(SiiInst_t inst)
{
	uint8_t sinkVer;
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	sinkVer = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_SINK_VERSION);

	return (sinkVer);

}

static void sScdcSourceVersionSet(SiiInst_t inst, uint8_t ver)
{

	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_SOURCE_VERSION, ver);
}

static void sScdcReadRequestEnable(SiiInst_t inst, uint8_t enable)
{

	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);

	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_CONGIG_0, enable ? 0x01 : 0x00);

}

void sScdcResetUpdateRegisters(ScdcObj_t *pObj)
{
	uint8_t updateFlag0;

	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);  // no need to read update1 all are reserved
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, updateFlag0); // clear update flages after reading
	SII_LIB_LOG_DEBUG1(pObj, ("\nReset update flag registers %02x\n", updateFlag0));

}

#if 0 //not clear functionality
void SiiDrvTxScdcSetRrAutoResponse(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	uint8_t updateFlag0;

	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);  // no need to read update1 all are reserved
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, updateFlag0); // clear update flages after reading
	SII_LIB_LOG_DEBUG1(pObj, ("\nReset update flag registers %02x\n", updateFlag0));

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY); //Enables automatic response to read request
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK, BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3);//Enables hardware interrupt
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_CONGIG_0, BIT_MASK__CONGIG_0__RR_ENABLE);//Enables read request in sink
}

void SiiDrvTxScdcSetRrStopResponse(SiiInst_t inst)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
	uint8_t updateFlag0;

	updateFlag0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);  // no need to read update1 all are reserved
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, updateFlag0); // clear update flages after reading
	SII_LIB_LOG_DEBUG1(pObj, ("\nReset update flag registers %02x\n", updateFlag0));

	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_REPLY_STOP); //Enables STOP response to read request
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK, BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3);//Enables hardware interrupt
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_CONGIG_0, BIT_MASK__CONGIG_0__RR_ENABLE);//Enables read request in sink
}

#endif

static void sScdcScramblingAndClockStatus(ScdcObj_t *pObj)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;

	bool_t scramble_in_source = false;
	bool_t scramble_in_sink = false;
	bool_t one_to_four_clock_in_source = false;
	bool_t one_to_four_clock_in_sink = false;
	uint8_t rg0, rg1;

	if ((SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCRCTL) & BIT_MSK__SCRCTL__REG_SCR_ON ) == BIT_MSK__SCRCTL__REG_SCR_ON) {
		scramble_in_source = true;
	}
	pObj->scmbleclkStatus.source_scramble_on_off = scramble_in_source;

	if (SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__TXC_DATA_DIV) == 0x02) { //TXC divide by 4 not defined in header file!
		one_to_four_clock_in_source = true;
	}
	pObj->scmbleclkStatus.source_clk_on_off = one_to_four_clock_in_source;

	rg0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG);
	rg1 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_STATUS);

	if ((rg0 & BIT_MASK__TMDS_CONFIG__SCRAMBLE_ENABLE) && (rg1 & BIT_MASK__TMDS_STATUS__SCRAMBLE_STATUS)) {
		scramble_in_sink = true;
	}
	pObj->scmbleclkStatus.sink_scramble_on_off = scramble_in_sink;

	if (rg0 & BIT_MASK__TMDS_CONFIG__BIT_CLOCK_RATIO) {
		one_to_four_clock_in_sink = true;
	}
	pObj->scmbleclkStatus.sink_clk_on_off = one_to_four_clock_in_sink;

	if (scramble_in_source) {
		SII_LIB_LOG_PRINT2(("Scrambling in source: ON\n"));
	} else {
		SII_LIB_LOG_PRINT2(("Scrambling in source: OFF\n"));
	}

	if (one_to_four_clock_in_source) {
		SII_LIB_LOG_PRINT2(("1/4 Clock in source: ON\n"));
	} else {
		SII_LIB_LOG_PRINT2(("1/4 Clock in source: OFF\n"));
	}

	if (scramble_in_sink) {
		SII_LIB_LOG_PRINT2(("Scrambling in sink: ON(%d %d)\n", (uint32_t)rg0, (uint32_t)rg1));
	} else {
		SII_LIB_LOG_PRINT2(("Scrambling in sink: OFF(%d %d)\n", (uint32_t)rg0, (uint32_t)rg1));
	}

	if (one_to_four_clock_in_sink) {
		SII_LIB_LOG_PRINT2(("1/4 Clock in sink: ON(%d %d)", (uint32_t)rg0, (uint32_t)rg1 ));
	} else {
		SII_LIB_LOG_PRINT2(("1/4 Clock in sink: OFF(%d %d)", (uint32_t)rg0, (uint32_t)rg1));
	}

}

void sScdcAutoPollingmode(ScdcObj_t *pObj)
{
	SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;

	/* disable interrupt from SCDC */
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK, 0);

	/* reset update registers */
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, 0xFF);
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_1, 0xFF);

	/* disable read request */
	sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_CONGIG_0, 0);

	/* activate/deactivate auto read of update registers */
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL, BIT_MSK__SCDC_CTL__REG_SCDC_AUTO_POLL); //Enables enables SCDC auto polling
	//set timer for 10MS ( will make sc_wait_expired == 1 when hit )
	SiiLibSeqTimerStart(pObj->timerScrambleExpire, 10, 10);
}

static void sScdcSinkCapsSet(SiiInst_t inst, SiiLibScdcSinKCaps_t* sinkCaps)
{

	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	uint8_t sinkVer = 0;
	uint8_t update[2] = {0};
	SiiLibTimeMilli_t cedtout;

	pObj->sinkCaps = *sinkCaps;

	if (pObj->sinkCaps.bScdcPresent) {
		sinkVer = sScdcSinkVersionGet(inst);

		if ( sinkVer != 1 ) {
			SII_LIB_LOG_DEBUG1(pObj, ("Sink Ver:%d\n", (int)sinkVer));
			return ;
		}
		sScdcSourceVersionSet(inst, pObj->config->srcVersion);
		sScdcRegReadTest(pObj); //need to chk for obj

		if (pObj->sinkCaps.bReadReqCapable) {
			if (pObj->config->bReadReq) {
				sScdcReadRequestEnable(inst, pObj->config->bReadReq);
				SiiLibTimeOutMilliSet( &cedtout, 10);
				do {
					sScdcRegReadUpdateFlgs(pObj, update);
					if ( SiiLibTimeOutMilliIs(&cedtout) ) {
						break;
					}
					SiiLibTimeMilliDelay(1);
				} while ( (update[0] & 0x2) == 0 );
				if ( (update[0] & 0x2) == 0) {
					SII_LIB_LOG_DEBUG1(pObj, ("%s_%d:Update[0]=0x%x,tcnt:%d\n", __func__, __LINE__, update[0]));
				}
				sScdcRegReadCeds(pObj);
				SiiLibTimeMilliDelay(1);
				sScdcReadRequestTest(pObj, 5);  //swarupa commented
			}
		}
	}

	//  SiiLibSeqTimerStart(pObj->timerScramblePoll, 10, 0);
	//SiiLibSeqTimerStart(pObj->timerRrIntPoll, 200, 500);
}

void SiiModTxScdcInterruptHandler(SiiInst_t inst)
{
	#define SCDC_INT_TEST_DEBUG	(0)
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t baseAddr;

	uint8_t scdcInt = 0;
	uint8_t updateFlag0;

	baseAddr = pObj->config->baseAddr;

	scdcInt = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, scdcInt); //clear interrupt status

	//contrlReg = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_CTL);
	//status0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);
	//status1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0_MASK);
	//l1_intr_stat_1 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__L1_INTR_STAT_0);

	updateFlag0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG0);
	#if SCDC_INT_TEST_DEBUG
	SII_LIB_LOG_DEBUG1(pObj, ("%s_%d:0x%x,0x%x\n", __func__, __LINE__, (uint32_t)scdcInt, (uint32_t)updateFlag0));
	#endif

	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK0) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("SCDC slave read request interrupt received\n"));
		#endif
	}
	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK1) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("SCDC slave read request interrupt received\n"));
		#endif
	}
	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK2) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("DDC bus conflicting\n"));
		#endif
	}
	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK3) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("SCDC slave read request interrupt received\n"));
		#endif
		updateFlag0 = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_UP_FLAG0);
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test PASSED******\n"));
		SII_LIB_LOG_DEBUG1(pObj, ("\n updateflag0 value is %02x\n", updateFlag0));
		#endif
		pObj->scdc_events |= SII_DRV_TX_EVENT__SCDCS_READ_REQST;
	} else {
		SII_LIB_LOG_DEBUG1(pObj, ("******Read Request Test FAILED******\n"));
	}
	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK4) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("DDC update flag change\n"));
		#endif
	}
	if (scdcInt & BIT_MSK__SCDC_INTR0_MASK__REG_SCDC_INTR0_MASK5) {
		#if SCDC_INT_TEST_DEBUG
		SII_LIB_LOG_DEBUG1(pObj, ("SCDC DDC stall request acknowledged\n"));
		#endif
	}
	if (pObj->scdc_events && pObj->config->cbFunc) {
		pObj->config->cbFunc(pObj->parentInst, pObj->scdc_events);
	}

}

static void sScdcRegReadCeds(ScdcObj_t *pObj)
{
	uint8_t regRead = 0, regRead_tmp = 0;
	uint8_t ced[7] = {0};

	sReadScdcBytes(pObj, DDC_OFFSET__SCDC_CHANNEL_0_ERROR_COUNT, ced, 7);
	regRead = ced[0];
	regRead_tmp = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x50 : CHAN0_ERROR_COUNT : %2x\n", regRead));

	regRead = ced[1];
	pObj->scdcregStatus.chnl0_error_cnt = ((regRead << 8) | regRead_tmp );
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x51 : CHAN0_ERROR_COUNT_VALID : %2x\n", regRead));

	regRead = ced[2];
	regRead_tmp = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x52 : CHAN1_ERROR_COUNT : %2x\n", regRead));

	regRead = ced[3];
	pObj->scdcregStatus.chnl1_error_cnt = ((regRead << 8) | regRead_tmp );
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x53 : CHAN1_ERROR_COUNT_VALID : %2x\n", regRead));

	regRead = ced[4];
	//pObj->scdcregStatus.chnl1_error_cnt = regRead;
	regRead_tmp = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x54 : CHAN2_ERROR_COUNT : %2x\n", regRead));

	regRead = ced[5];
	pObj->scdcregStatus.chnl2_error_cnt = ((regRead << 8) | regRead_tmp );
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x55 : CHAN2_ERROR_COUNT_VALID : %2x\n", regRead));
}

static void sScdcRegReadUpdateFlgs(ScdcObj_t *pObj, uint8_t *updata)
{
	uint8_t regRead = 0;

	sReadScdcBytes(pObj, DDC_OFFSET__SCDC_UPDATE_0, updata, 2);
	regRead = updata[0];
	pObj->scdcregStatus.update_0 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x10 : UPDATE_0: %2x :: RR_TEST = %d, CED_Update = %d, Status_update = %d\n", regRead, (regRead & 0x04), (regRead & 0x02), (regRead & 0x01)));

	regRead = updata[1];
	pObj->scdcregStatus.update_1 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x11 : DDC_OFFSET__SCDC_UPDATE_1 : %2x\n", regRead));
}

static void sScdcRegReadTest(ScdcObj_t *pObj)
{
	uint8_t regRead = 0;
	uint8_t update[2] = {0};

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_SINK_VERSION);
	pObj->scdcregStatus.sink_ver = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x01 : DDC_OFFSET__SCDC_SINK_VERSION : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_SOURCE_VERSION);
	pObj->scdcregStatus.source_ver = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x02 : DDC_OFFSET__SCDC_SOURCE_VERSION : %2x\n", regRead));

	sScdcRegReadUpdateFlgs(pObj, update);

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_CONFIG);
	pObj->scdcregStatus.tmds_config = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x20 : TMDS_CONFIG : %2x :: TMDS_Bit_Clk_Rat = %d, Scramb_Enable = %d\n", regRead, (regRead & 0x02), (regRead & 0x01)));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TMDS_STATUS);
	pObj->scdcregStatus.tmds_status = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x21 : SCRAMBLER_STATUS : %2x:: Scrambler_Status = %d\n", regRead, (regRead & 0x01)));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_CONGIG_0);
	pObj->scdcregStatus.config_0 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x30 : SCDC_CONGIG_0 : %2x:: RR_Enable =  %d\n", regRead, (regRead & 0x01)));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_STATUS_FLAGS_0);
	pObj->scdcregStatus.status_flag_0 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x40 : STATUS_FLAGS_0 : %2x:: Ch2_Lock = %d, Ch1_Lock = %d, Ch0_Lock = %d, Clock_Det = %d\n", regRead, (regRead & 0x08), (regRead & 0x04), (regRead & 0x02), (regRead & 0x01)));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_TEST_CONFIG_0);
	pObj->scdcregStatus.test_config_0 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0x40 : TEST_CONFIG_0 : %2x\n", regRead));

	sScdcRegReadCeds(pObj);
}

static void sScdcManufacturerRegReadTest(ScdcObj_t *pObj)
{
	uint8_t regRead = 0;

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_OUI_3);
	pObj->peerManfStatus.oui_3 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xD0 : DDC_OFFSET__SCDC_MANF_OUI_3 : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_OUI_2);
	pObj->peerManfStatus.oui_2 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xD1 : DDC_OFFSET__SCDC_MANF_OUI_2 : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_OUI_1);
	pObj->peerManfStatus.oui_1 = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xD2 : DDC_OFFSET__SCDC_MANF_OUI_1 : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_DEV_ID_STR);
	pObj->peerManfStatus.dev_id_str = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xD3 - 0xDA : DDC_OFFSET__SCDC_MANF_DEV_ID_STR : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_DEV_ID_HW_REV);
	pObj->peerManfStatus.dev_id_hw_rev = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xDB : DDC_OFFSET__SCDC_MANF_DEV_ID_HW_REV : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MAJOR_REV);
	pObj->peerManfStatus.dev_id_sw_major_rev = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xDC : DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MAJOR_REV : %2x\n", regRead));

	regRead = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MINOR_REV);
	pObj->peerManfStatus.dev_id_sw_minor_rev = regRead;
	SII_LIB_LOG_DEBUG2(("SCDC REG: 0xDD : DDC_OFFSET__SCDC_MANF_DEV_ID_SW_MINOR_REV : %2x\n", regRead));

}

static void sScdcSrcCrSet(SiiInst_t inst, bool_t cr)
{
	ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
	pObj->srcCrAbove340m = cr;
}

/*
void sScdcInterruptHandler(SiiInst_t inst)
{
ScdcObj_t* pObj = (ScdcObj_t*)SII_LIB_OBJ_PNTR(inst);
SiiDrvCraAddr_t baseAddr = pObj->config->baseAddr;
uint8_t scdcInt = 0;

scdcInt = SiiDrvCraRdReg8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0);

if(scdcInt & BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3)
{
SiiDrvCraSetBit8(pObj->config->instTxCra, baseAddr | REG_ADDR__SCDC_INTR0, BIT_MSK__SCDC_INTR0__REG_SCDC_INTR0_STAT3); // Clear Slave Read Requrest Interrupt

pObj->scdcUpdate0 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0);
pObj->scdcUpdate1 = sReadScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_1);

sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_0, pObj->scdcUpdate0);
sWriteScdcReg8(pObj, DDC_OFFSET__SCDC_UPDATE_1, pObj->scdcUpdate1);
}
}
*/
/***** end of file ***********************************************************/
