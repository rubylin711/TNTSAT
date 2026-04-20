
//#define SII_DEBUG

#include "si_datatypes.h"
#include "si_app_cec.h"
#include "si_drv_cpi_api.h"
#include "si_cec_enums.h"
#include "si_lib_malloc_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_log_api.h"
#include "si_lib_time_api.h"
#include "sii_time.h"
#include "si_lib_seq_api.h"
#if __HDMI_OS_RTOS__
#include "hdmi.h"
#endif

//#include "si_rx_interface_config.h"
//SII_LIB_OBJ_MODULE_DEF(cec_app);

#define NUM_CPI_INSTANCES 1

#define TASK_QUEUE_LENGTH               8   // Number of task queue entries

#define ACTIVE_TASK                     spCecInstData->taskQueue[ spCecInstData->taskQueueOut]
//#define MAKE_SRCDEST( src, dest )   ((( (src) << 4) & 0xF0) | ((dest) & 0x0F))
#define GET_CEC_SRC( srcDest )      (( srcDest >> 4) & 0x0F)
#define GET_CEC_DEST( srcDest )     (( srcDest >> 0) & 0x0F)

static char l_cecTxOsdNameString [15] = "SII TX";

typedef enum _SiiDrvCecError_t {
	RESULT_CEC_SUCCESS,             // Success result code
	RESULT_CEC_FAIL,                // General Failure result code

	RESULT_CEC_INVALID_PARAMETER,

	RESULT_CEC_INVALID_LOGICAL_ADDRESS,
	RESULT_CEC_INVALID_PHYSICAL_ADDRESS,
	RESULT_CEC_INVALID_PORT_INDEX,
	RESULT_CEC_NOT_ADJACENT,
	RESULT_CEC_NO_PA_FOUND,
	RESULT_CEC_TASK_QUEUE_FULL,
	RESULT_CEC_NO_LA_FOUND,

} SiiDrvCecError_t;

typedef enum {
	SiiCEC_POWERSTATE_CHANGE    = 0x0001,
	SiiCEC_SOURCE_LOST          = 0x0002
} SiiCecStatus_t;

typedef enum _SiiCecQueueStates_t {
	SiiCecTaskQueueStateIdle     = 0,
	SiiCecTaskQueueStateQueued,
	SiiCecTaskQueueStateRunning,
} SiiCecQueueStates_t;

/* Task CPI states. */
typedef enum _SiiCpiStates_t {
	CPI_IDLE            = 0,
	CPI_WAIT_ACK,
	CPI_WAIT_RESPONSE,
	CPI_RESPONSE
} SiiCpiStates_t;

typedef enum _SiiCecTasks_t {
	SiiCECTASK__IDLE                 = 0,
	SiiCECTASK__CANCEL,
	SiiCECTASK__ENUMERATE,
	SiiCECTASK__SETLA,
	SiiCECTASK__ONETOUCH,
	SiiCECTASK__SYSSTANDBY,
	SiiCECTASK__REMOTEPASSPRESS,
	SiiCECTASK__REMOTEPASSRELEASE,
	SiiCECTASK__SENDMSG,
	SiiCECTASK__GETCECVERSION,
	SiiCECTASK__RPTCURLATENCY,
	SiiCECTASK__SENDMSGACK_MT,

	SiiCECTASK__COUNT
} SiiCecTasks_t;

typedef struct _SiiCecTaskState_t {
	SiiCecTasks_t       task;           // This CEC task #
	SiiCecQueueStates_t queueState;     // Task running or waiting to be run
	uint8_t             taskState;      // Internal task state
	SiiCpiStates_t      cpiState;       // State of CPI transactions
	uint8_t             destLA;         // Logical address of target device
	uint8_t             taskData1;      // BYTE Data unique to task.
	uint16_t            taskData2;      // WORD Data unique to task.
	uint8_t             *pTaskData3;    // Pointer to BYTE Data array unique to task.
	uint16_t            msgId;          // Helps serialize CPI transactions
	uint32_t            taskTimer;
} SiiCecTaskState_t;

typedef struct _CecLogicalDevice_t {
	int_t   deviceType;     // 0 - Device is a TV.
	// 1 - Device is a Recording device
	// 2 - Device is a reserved device
	// 3 - Device is a Tuner
	// 4 - Device is a Playback device
	// 5 - Device is an Audio System
	uint16_t    devPA;      // CEC Physical address of the device.
	uint16_t    devLA;
	bool_t      selected;
	int8_t      osdName[10];
	uint32_t    vendorId;
	SiiCecPowerstatus_t         sourcePowerStatus;
} CecLogicalDevice_t;

typedef struct _CecInstanceData_t {
	int         lastResultCode;         // Contains the result of the last API function called
	uint16_t    statusFlags;

	SiiCecDeviceTypes_t deviceType;     // type of CEC device

	uint_t      debugDisplayLevel;

	bool_t                      enable;
	uint8_t                     numVirtualDevices;
	SiiCecLogicalAddresses_t    logicalAddr;
	SiiCecPowerstatus_t         powerState;

	uint8_t     paShift;
	uint16_t    paChildMask;
	uint16_t    physicalAddr;

	uint8_t     osdName[14];
	int         osdNameLen;

	uint8_t     lastUserControlPressedSourceLa;     // For User Control Released
	uint8_t     lastUserControlPressedTargetLa;     // For User Control Released

	// RX-only data

	SiiCecPowerstatus_t         sourcePowerStatus;
	SiiCecLogicalAddresses_t    activeSrcLogical;
	uint16_t                    activeSrcPhysical;
	CecLogicalDevice_t          logicalDeviceInfo [16];

	uint8_t             numDevsInNetwork;

	// TX-only data

	bool_t      isActiveSource;

	// Task data

	int_t               taskQueueIn;
	int_t               taskQueueOut;
	SiiCecTasks_t       currentTask;
	SiiCecTaskState_t   taskQueue[TASK_QUEUE_LENGTH];
	bool_t              enumerateComplete;

	SiiCecLogicalAddresses_t logAddrList[16];
	SiiInst_t          cpiInst[NUM_CPI_INSTANCES];
	CecEventcbFunc CecEvtNotifycbFunc;
}	CecInstanceData_t;

CecInstanceData_t *spCecInstData;

SiiInst_t          sCpiInst = 0;
mt_u32            g_menu_status_si = 1;             // 1 is deactivated, 0 is activated

#define ACTIVE_TASK                     spCecInstData->taskQueue[ spCecInstData->taskQueueOut]

uint8_t CecVendorID[3] = {0x00, 0x01, 0x03}; //should be SIMG vendor ID

static uint8_t cecValidate [128] = {
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1,

	//0x0E - 0x0F Reserved
	0, 0,

	1, 1,

	//0x12 - 0x1C Reserved
	0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0,

	1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1,

	//0x2D - 0x2E Reserved
	0, 0,

	1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1,

	//0x39 - 0x3F Reserved
	0, 0, 0,
	0, 0, 0, 0,

	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1,

	//0x4D - 0x4F Reserved
	0, 0, 0,

	1, 1, 1, 1,
	1, 1, 1, 1,

	//0x58 - 0x5F Reserved
	0, 0, 0, 0,
	0, 0, 0, 0,

	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1,

	//0x6E - 0x70 Reserved
	0, 0,
	0,

	1, 1, 1,
	1, 1, 1,

	// 0x77 - 0x7F Reserved
	0,
	0, 0, 0, 0,
	0, 0, 0, 0
};

uint8_t l_devTypes [16] = {
	CEC_DT_TV,
	CEC_DT_RECORDING_DEVICE,
	CEC_DT_RECORDING_DEVICE,
	CEC_DT_TUNER,
	CEC_DT_PLAYBACK,
	CEC_DT_AUDIO_SYSTEM,
	CEC_DT_TUNER,
	CEC_DT_TUNER,
	CEC_DT_PLAYBACK,
	CEC_DT_RECORDING_DEVICE,
	CEC_DT_TUNER,
	CEC_DT_PLAYBACK,
	CEC_DT_RECORDING_DEVICE,
	CEC_DT_RECORDING_DEVICE,
	CEC_DT_TV,
	CEC_DT_TV
};

static bool_t CecAddTask ( SiiCecTaskState_t *pNewTask );
static void SiiCecCallBack(SiiInst_t inst);
void SiiCecSendInactiveSource ( uint8_t logicalAddr, uint16_t physicalAddr );
uint8_t CecTaskOneTouchPlay ( SiiCpiStatus_t *pCecStatus );
void SiiCecSendSysStandby ( uint8_t srclogicalAddr, uint8_t dstlogicalAddr);
uint8_t CecTaskSysStandby ( SiiCpiStatus_t *pCecStatus );
uint8_t CecTaskRemotePass ( SiiCpiStatus_t *pCecStatus );
bool_t SiiCecEnumerateIsComplete ( void );
bool_t SiiCecDeviceLaIsAvailable ( uint8_t deviceLa );
SiiCecLogicalAddresses_t SiiCecGetAvailableLa ( uint8_t *pDeviceLaList );
uint16_t SiiCecGetDevicePA ( void );
uint8_t CecTaskGetCECVerison ( SiiCpiStatus_t *pCecStatus );
uint8_t CecTaskReportCurLatency ( SiiCpiStatus_t *pCecStatus );
uint8_t CecTaskSendMsgAck ( SiiCpiStatus_t *pCecStatus );
void SiiCecDisable(void);
SiiCecLogicalAddresses_t SiiCecGetDeviceLA ( void );

//------------------------------------------------------------------------------
static bool_t CecValidateMessage ( SiiCpiData_t *pMsg )
{
	uint8_t parameterCount = 0;
	bool_t  countOK = true;
	bool_t  isFromUnregistered = false;

	// If message is from Broadcast address, we ignore it except for
	// some specific cases.
	uint8_t i = 0;

	SII_LIB_LOG_DEBUG2(("cec msg: srcDestAddr=%02X  opcode=%02X argCount=%02X txState=%02X msgId=%x\n", (uint32_t)pMsg->srcDestAddr, (uint32_t)pMsg->opcode, (uint32_t)pMsg->argCount, (uint32_t)pMsg->txState, (uint32_t)pMsg->msgId));
	HDMI_DEBUG_TRACE("cec msg.get: srcDestAddr=%02X  opcode=%02X argCount=%02X txState=%02X msgId=%x\n", (uint32_t)pMsg->srcDestAddr, (uint32_t)pMsg->opcode, (uint32_t)pMsg->argCount, (uint32_t)pMsg->txState, (uint32_t)pMsg->msgId);
	for (i = 0; i < pMsg->argCount; i++) {
		SII_LIB_LOG_DEBUG2(("%02X ", pMsg->args[i]));
	}

	SII_LIB_LOG_DEBUG2(("\n"));

	if (( pMsg->srcDestAddr & 0xF0 ) == 0xF0 ) {
		switch ( pMsg->opcode ) {
			case CECOP_STANDBY:
			case CECOP_SYSTEM_AUDIO_MODE_REQUEST:
			case CECOP_ROUTING_CHANGE:
			case CECOP_ROUTING_INFORMATION:
			case CECOP_ACTIVE_SOURCE:
			case CECOP_GIVE_PHYSICAL_ADDRESS:
			case CECOP_REPORT_PHYSICAL_ADDRESS:
			case CECOP_REQUEST_ACTIVE_SOURCE:
			case CECOP_GET_MENU_LANGUAGE:
			case CECOP_SET_STREAM_PATH:
			case CDCOP_HEADER:
				break;
			default:
				isFromUnregistered = true;          // All others should be ignored
				break;
		}
	}

	/* Determine required parameter count   */

	switch ( pMsg->opcode ) {
		case CECOP_IMAGE_VIEW_ON:
		case CECOP_TEXT_VIEW_ON:
		case CECOP_STANDBY:
		case CECOP_GIVE_PHYSICAL_ADDRESS:
		case CECOP_GIVE_DEVICE_VENDOR_ID:
		case CECOP_GIVE_DEVICE_POWER_STATUS:
		case CECOP_GET_MENU_LANGUAGE:
		case CECOP_GET_CEC_VERSION:
		case CECOP_INITIATE_ARC:
		case CECOP_REPORT_ARC_INITIATED:
		case CECOP_REPORT_ARC_TERMINATED:
		case CECOP_REQUEST_ARC_INITIATION:
		case CECOP_REQUEST_ARC_TERMINATION:
		case CECOP_TERMINATE_ARC:
		case CECOP_ABORT:
		case CECOP_MENU_REQUEST:
			#if (SUPPORT_CEC_2P)
		case CECOP_GIVE_FEATURES:
			#endif
			parameterCount = 0;
			break;
		case CECOP_REPORT_POWER_STATUS:         // power status
		case CECOP_CEC_VERSION:                 // cec version
			parameterCount = 1;
			break;
		case CECOP_INACTIVE_SOURCE:             // physical address
		case CECOP_FEATURE_ABORT:               // feature opcode / abort reason
		case CECOP_ACTIVE_SOURCE:               // physical address
			parameterCount = 2;
			break;
		case CECOP_REPORT_PHYSICAL_ADDRESS:     // physical address / device type
		case CECOP_DEVICE_VENDOR_ID:            // vendor id
			parameterCount = 3;
			break;
		case CECOP_USER_CONTROL_PRESSED:        // UI command
		case CECOP_SET_OSD_NAME:                // osd name (1-14 bytes)
			#if (CEC_2P_CTS_4PBORTUNER == 0)
		case CECOP_SET_OSD_STRING:              // 1 + x   display control / osd string (1-13 bytes)
			#endif
			parameterCount = 1;                 // must have a minimum of 1 operands
			break;
		case CECOP_REPORT_CURRENT_LATENCY:
			parameterCount = 4;
			break;

		default:
			break;
	}

	/* Test for correct parameter count.    */

	if (( pMsg->argCount < parameterCount ) || isFromUnregistered ) {
		countOK = false;
	}

	return ( countOK );
}

//------------------------------------------------------------------------------
static bool_t SiiCecValidateKeyCode ( uint8_t keyData )
{
	bool_t  validKey = false;

	// All keys 0x80 - 0xFF are invalid, use the table for the rest
	if (( cecValidate[ keyData & ~SII_BIT7]) != 0 ) {
		validKey = true;
	}

	return ( validKey );
}

//------------------------------------------------------------------------------
uint16_t SiiCecSendReportFeatures ( void )
{
	#if SUPPORT_CEC_2P
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_REPORT_FEATURES;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_UNREGORBC );
	cecFrame.args[0]        = (uint8_t)0x06;           // [CEC version] 0x5: CEC 1.4, 0x6:CEC 2.0
	cecFrame.args[1]        = (uint8_t)0x20;         // [All Device Types] -- only tuner
	cecFrame.args[2]        = (uint8_t)0x40;    // [RC Profile] Source [Primary Device Type]
	cecFrame.args[3]        = (uint8_t)0x00;    // [Device Features] Source [Primary Device Type]
	cecFrame.argCount       = 4;

	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));
	#else
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_CEC_VERSION;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
	cecFrame.args[0]        = (uint8_t)0x05;           // [CEC version] 0x5: CEC 1.4, 0x6:CEC 2.0
	cecFrame.argCount       = 1;

	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));
	#endif
}

//------------------------------------------------------------------------------
uint16_t SiiCecSendReportPhysicalAddress ( void )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_REPORT_PHYSICAL_ADDRESS;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_UNREGORBC );
	cecFrame.args[0]        = (uint8_t)(spCecInstData->physicalAddr >> 8);           // [Physical Address] High
	cecFrame.args[1]        = (uint8_t)spCecInstData->physicalAddr & 0xFF;         // [Physical Address] Low
	cecFrame.args[2]        = l_devTypes[spCecInstData->logicalAddr];    // [Device Type]
	cecFrame.argCount       = 3;

	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));
}

//------------------------------------------------------------------------------
static void SiiCecUpdateLogicalDeviceInfo ( uint8_t newLA, uint16_t newPA, bool_t isActive )
{

	spCecInstData->logicalDeviceInfo[ newLA ].devPA      = newPA;
	spCecInstData->logicalDeviceInfo[ newLA ].devLA      = newLA;
	spCecInstData->logicalDeviceInfo[ newLA ].deviceType = l_devTypes[ newLA ];
	spCecInstData->logicalDeviceInfo[ newLA ].selected = isActive ? true : false;

	spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;
}

//------------------------------------------------------------------------------
static uint16_t SiiCecSendReportPowerStatus ( SiiCecLogicalAddresses_t destLa )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode        = CECOP_REPORT_POWER_STATUS;
	cecFrame.srcDestAddr   = MAKE_SRCDEST( spCecInstData->logicalAddr, destLa );
	cecFrame.args[0]       = spCecInstData->powerState;
	cecFrame.argCount      = 1;

	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));
}

#if (SUPPORT_CEC_VENDOR_CMD == 1)
static uint16_t SiiCecSendVendorId(  SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t *vendorId )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode        = CECOP_DEVICE_VENDOR_ID;
	cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
	cecFrame.args[0]       = vendorId[0];
	cecFrame.args[1]       = vendorId[1];
	cecFrame.args[2]       = vendorId[2];
	cecFrame.argCount      = 3;

	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));

}
#endif

//------------------------------------------------------------------------------
//! @brief  Send the device name string as the OSD name
//------------------------------------------------------------------------------
static void CecSendSetOsdName ( uint8_t destLogicalAddr )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_SET_OSD_NAME;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( spCecInstData->logicalAddr, destLogicalAddr );
	memcpy( &cecFrame.args, spCecInstData->osdName, spCecInstData->osdNameLen );
	cecFrame.argCount       = (uint8_t)spCecInstData->osdNameLen;

	SII_LIB_LOG_DEBUG2(("%s %d \n", __FUNCTION__, __LINE__));
	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//-------------------------------------------------------------------------------------------------
static void SiiCecFeatureAbortSendEx (uint8_t opCode, uint8_t reason, SiiCecLogicalAddresses_t destLa )
{
	SiiCpiData_t cecFrame;

	cecFrame.args[0]        = opCode;
	cecFrame.args[1]        = reason;
	cecFrame.opcode         = CECOP_FEATURE_ABORT;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( spCecInstData->logicalAddr, destLa );
	cecFrame.argCount       = 2;

	SII_LIB_LOG_DEBUG2(("%s %d \n", __FUNCTION__, __LINE__));
	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetSourceActive
// Description: Set the current source as Active in TPG case
//------------------------------------------------------------------------------

void SiiCecSetSourceActive( bool_t isActiveSource )
{
	spCecInstData->isActiveSource = isActiveSource;
	    if(spCecInstData->isActiveSource && spCecInstData->physicalAddr != 0xFFFF)
	{
	SiiCecSendActiveSource(spCecInstData->logicalAddr, spCecInstData->physicalAddr);
	}
}

//------------------------------------------------------------------------------
static bool_t SiiCecSetOsdName ( char *pOsdName )
{
	int nameLen = (int)strlen( pOsdName );

	spCecInstData->lastResultCode = RESULT_CEC_INVALID_PARAMETER;
	if ( nameLen < 14 ) {
		spCecInstData->osdNameLen = nameLen+1;
		memset(spCecInstData->osdName,0,14);
		memcpy( spCecInstData->osdName, pOsdName, spCecInstData->osdNameLen );
		spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;
	}

	return ( spCecInstData->lastResultCode == RESULT_CEC_SUCCESS );
}

//------------------------------------------------------------------------------
//! @brief  Set the device logical address based on the first available entry
//!         in the passed list of device addresses.  Expects that an enumerate
//!         function has been run.
//------------------------------------------------------------------------------
bool_t SiiCecEnumerateDeviceLa ( uint8_t *pDeviceList )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__SETLA;
	newTask.taskState   = 0;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	newTask.pTaskData3  = pDeviceList;
	return ( CecAddTask( &newTask ));
}

#if TPG_RECOVERY_UNUSED_FUNC
//------------------------------------------------------------------------------
static void SiiCecGetLogicalDevice(SiiCecLogicalAddresses_t devLa, CecLogicalDevice_t *pCld)
{
	int i;
	for (i = 0; i < spCecInstData->numDevsInNetwork; i++) {
		if (spCecInstData->logicalDeviceInfo[i].devLA == devLa) {
			*pCld = spCecInstData->logicalDeviceInfo[i];
		}
	}
}
#endif

//------------------------------------------------------------------------------
//! @brief  Send an ACTIVE SOURCE message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going active
//! @param[in]  - physicalAddr  Physical address of device
//------------------------------------------------------------------------------
void SiiCecSendMenuStatus ( uint8_t logicalAddr, uint8_t Dst, uint8_t sts )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_MENU_STATUS;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( logicalAddr, Dst );
	cecFrame.args[0]        = sts;        // [Physical Address] High
	cecFrame.argCount       = 1;

	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//------------------------------------------------------------------------------
static bool_t CecMsgHandlerFirst ( SiiCpiData_t *pMsg )
{
	bool_t  usedMessage         = true;
	bool_t  isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );

	// Don't process unless it is intended for the local logical address (we must check
	// in case another device has been added for virtual device support).
	switch ( pMsg->opcode ) {
		case CECOP_STANDBY:                                             // Direct and Broadcast
			//if ( spCecInstData->powerState != CEC_POWERSTATUS_STANDBY ) {
				// Next time through the main loop, power will be cycled off

				spCecInstData->powerState    = CEC_POWERSTATUS_ON_TO_STANDBY;
				spCecInstData->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;        // Signal upper layer
				spCecInstData->isActiveSource = false;                           // Only impacts TX
				if ( spCecInstData->CecEvtNotifycbFunc ) {
					spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_STANDBY);
				}
			//}
			break;

		case CECOP_GIVE_PHYSICAL_ADDRESS:
			if ( isDirectAddressed) {                  // Ignore as broadcast message
				SiiCecSendReportPhysicalAddress( );
			} else {
				usedMessage = false;
			}
			break;

		case CECOP_REPORT_PHYSICAL_ADDRESS:

			if ( !isDirectAddressed ) {                 // Ignore as direct message
				if (pMsg->args[2] != 0) {
					pMsg->srcDestAddr = (uint8_t)((pMsg->srcDestAddr & 0x0F) | (pMsg->args[2] << 4)) ;
				}

				SiiCecUpdateLogicalDeviceInfo((pMsg->srcDestAddr >> 4) & 0x0F,         // broadcast logical address
											  (uint16_t)((((uint16_t)pMsg->args[0]) << 8) | pMsg->args[1]),   // broadcast physical address
											  false
											 );
				// Let Enumerate task know about it.
				if ( ACTIVE_TASK.task == SiiCECTASK__ENUMERATE ) {
					ACTIVE_TASK.cpiState = CPI_RESPONSE;
				}
			}
			break;

		case CECOP_GIVE_DEVICE_POWER_STATUS:
			if ( isDirectAddressed ) {              // Ignore as broadcast message
				SiiCecSendReportPowerStatus( (SiiCecLogicalAddresses_t)(pMsg->srcDestAddr >> 4) );
			} else {
				usedMessage = false;
			}
			break;

		case CECOP_REPORT_POWER_STATUS:                 // Someone sent us their power state.
			if ( isDirectAddressed ) {                  // Ignore as broadcast message
				//CecLogicalDevice_t cld;  // Doubt
				//SiiCecGetLogicalDevice((SiiCecLogicalAddresses_t)((pMsg->srcDestAddr & 0xF0) >> 4), &cld);
				// unrecognised device not taken care
				spCecInstData->sourcePowerStatus = (SiiCecPowerstatus_t)pMsg->args[0];
			}
			break;

		case CECOP_USER_CONTROL_PRESSED:
			if ( !isDirectAddressed ) {
				break;    // Ignore as broadcast message
			}

			// If not a VALID CEC key, feature abort
			if ( !SiiCecValidateKeyCode( pMsg->args[0])) {
				SiiCecFeatureAbortSendEx( (uint8_t)pMsg->opcode, CECAR_INVALID_OPERAND, (SiiCecLogicalAddresses_t)GET_CEC_SRC( pMsg->srcDestAddr ));
				break;              // Used the message...
			}
			usedMessage = false;    // Didn't use the message, we just validated the key ID
			break;

		case CECOP_GIVE_OSD_NAME:
			SiiCecSetOsdName(l_cecTxOsdNameString);
			usedMessage = false;
			break;

			#if (SUPPORT_CEC_VENDOR_CMD == 1)
		case CECOP_GIVE_DEVICE_VENDOR_ID:
			SiiCecSendVendorId(spCecInstData->logicalAddr, CEC_LOGADDR_UNREGORBC, CecVendorID);
			break;
			#endif

			#if (SUPPORT_CEC_2P)
		case CECOP_GIVE_FEATURES:
			SiiCecSendReportFeatures();
			break;
			#endif

		default:
			usedMessage = false;                        // Didn't use the message
			break;
	}

	return ( usedMessage );
}

//------------------------------------------------------------------------------
// Function:    CecMsgHandlerLast
// Description: This is the last message handler called in the chain, and
//              parses any messages left untouched by the previous handlers.
//
// NOTE:        Messages handled here should be common to all system types.
//
//------------------------------------------------------------------------------

static void CecMsgHandlerLast ( SiiCpiData_t *pMsg )
{
	uint8_t         srcAddr;
	bool_t          isDirectAddressed;
	SiiCpiData_t    cecFrame;
	uint8_t dst_la = 0;
	isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
	srcAddr             = GET_CEC_SRC( pMsg->srcDestAddr );

	switch ( pMsg->opcode ) {
		case CECOP_FEATURE_ABORT:
			if ( isDirectAddressed ) {              // Ignore as broadcast message
				SII_LIB_LOG_DEBUG2(("Feature abort received\n"));
			}
			break;

		// These messages have already been handled for internal purposes
		// by CecRxMsgHandler and passed to the application level
		// and/or were ignored but not consumed by the application level.
		// Ignore them here.

		case CECOP_IMAGE_VIEW_ON:
		case CECOP_TEXT_VIEW_ON:
		case CECOP_GET_MENU_LANGUAGE:
		case CECOP_USER_CONTROL_PRESSED:
			//SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: USER CONTROL PRESSED\n"));
			switch (pMsg->args[0]) {
				case CEC_RC_PLAY:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: PLAY Command Received\n"));
					break;
				case CEC_RC_STOP:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: STOP Command Received\n"));
					break;
				case CEC_RC_PAUSE:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: PAUSE Command Received\n"));
					break;
				case CEC_RC_FAST_FORWARD:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: FAST FORWARD Command Received\n"));
					break;
				case CEC_RC_REWIND:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: REWIND Command Received\n"));
					break;
				case CEC_RC_UP:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: UP ARROW Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_UP);
					}
					break;
				case CEC_RC_DOWN:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: DOWN ARROW Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_DOWN);
					}
					break;
				case CEC_RC_LEFT:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: LEFT ARROW Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_LEFT);
					}
					break;
				case CEC_RC_RIGHT:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: RIGHT ARROW Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_RIGHT);
					}
					break;
				case CEC_RC_SELECT:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: SELECT Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_OK);
					}
					break;
				case CEC_RC_0:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 0  Received\n"));
					break;
				case CEC_RC_1:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 1  Received\n"));
					break;
				case CEC_RC_2 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 2  Received\n"));
					break;
				case CEC_RC_3 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 3  Received\n"));
					break;
				case CEC_RC_4 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 4  Received\n"));
					break;
				case CEC_RC_5 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 5  Received\n"));
					break;
				case CEC_RC_6 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 6  Received\n"));
					break;
				case CEC_RC_7 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 7  Received\n"));
					break;
				case CEC_RC_8 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 8  Received\n"));
					break;
				case CEC_RC_9 :
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Number 9  Received\n"));
					break;
				case CEC_RC_VOLUME_UP:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: VOL+ Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_VOL_UP);
					}
					break;
				case CEC_RC_VOLUME_DOWN:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: VOL- Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_VOL_DOWN);
					}
					break;
				case CEC_RC_MUTE:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: MUTE/UNMUTE Command Received\n"));
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_TV_SEND_CEC_MUTE);
					}
					break;
				default:
					break;
			}
			break;

		case CECOP_USER_CONTROL_RELEASED:
			//SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: User Control Released received\n"));
			break;

		// Handle this here because the app level may have upgraded the version
		// and handled it before it gets here.
		case CECOP_PLAY:
			SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: PLAY Command Received\n"));
			switch (pMsg->args[0]) {
				case CEC_PLAY_FORWARD:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: PLAY FORWARD Command Received\n"));
					break;
				case CEC_PLAY_STILL:
					SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: PLAY STILL Command Received\n"));
					break;
				case CEC_PLAY_REVERSE:
					SII_LIB_LOG_DEBUG2( ("CecMsgHandlerLast():: PLAY REVERSE Command Received\n"));
					break;
				default:
					break;
			}
			break;
		case CECOP_DECK_CONTROL:
			SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: DECK CONTROL Command Received\n"));
			break;
			#if (CEC_2P_CTS_4PBORTUNER == 0)
		case CECOP_VENDOR_REMOTE_BUTTON_DOWN:
			SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: VENDOR REMOTE BUTTON DOWN Command Received\n"));
			break;
			#endif

		case CECOP_GET_CEC_VERSION:
			if ( isDirectAddressed ) {                  // Ignore as broadcast message
				// Respond to this request with the CEC version support.
				cecFrame.srcDestAddr   = MAKE_SRCDEST( spCecInstData->logicalAddr, pMsg->srcDestAddr >> 4 );
				cecFrame.opcode        = CECOP_CEC_VERSION;
				#if SUPPORT_CEC_2P
				cecFrame.args[0]       = 0x06;       // Report CEC2.0
				#else
				cecFrame.args[0]       = 0x05;       // Report CEC1.4
				#endif
				cecFrame.argCount      = 1;
				SiiDrvCpiWrite( sCpiInst, &cecFrame );
			}
			break;

		case CECOP_GIVE_OSD_NAME:
			if ( isDirectAddressed ) {
				CecSendSetOsdName( srcAddr );
			}
			break;

		// Ignore these messages if unrecognized AND broadcast
		// but feature abort them if directly addressed
		case CECOP_GIVE_DEVICE_VENDOR_ID:
			SiiCecSendVendorId(spCecInstData->logicalAddr, CEC_LOGADDR_UNREGORBC, CecVendorID);
			break;
		case CDCOP_HEADER:
			if ( isDirectAddressed ) {
				SiiCecFeatureAbortSendEx( (uint8_t)pMsg->opcode, CECAR_UNRECOG_OPCODE, (SiiCecLogicalAddresses_t)srcAddr );
			}
			break;

		case CECOP_SET_STREAM_PATH:
			SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: Set Stream Path Command Received:%d,0x%x,0x%x,0x%x\n", (uint32_t)isDirectAddressed, (uint32_t)pMsg->args[0], (uint32_t)pMsg->args[1], (uint32_t)spCecInstData->physicalAddr ));
			if ( !isDirectAddressed ) {                 // Ignore as direct message
				//if ( spCecInstData->isActiveSource )
				// This msg means tv power on
				// bug 24043: AOC T1951MD TV send error 0x86 msg: 0x0f 0x86 0x20 0x00 mean physical addr 0x2000
				//			  but the really physical addr from edid is 0x1000
				#if 0
				if ( (spCecInstData->physicalAddr >> 8)  == pMsg->args[0] && \
						(spCecInstData->physicalAddr & 0xFF)  == pMsg->args[1])
				#endif
				if (spCecInstData->physicalAddr != 0) {
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_CEC_STATUS_POWER_ON);
					}
					SiiCecSendActiveSource( spCecInstData->logicalAddr, spCecInstData->physicalAddr );
					spCecInstData->isActiveSource = 1;
				}
			}
			break;

		case CECOP_ROUTING_CHANGE:
			dst_la = pMsg->srcDestAddr & 0x0f;
			//bug 24043: AOC T1951MD TV send error 0x80 mgs :0x0f 0x80 0x00 0x00 0x20 0x00
			//                                          and :0x0f 0x80 0x20 0x00 0x20 0x00
			if (dst_la == 0xf &&  pMsg->argCount >= 4)
			{
				if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_CEC_STATUS_POWER_ON);
					}
			}
			break;

		// Any directly addressed message that gets here is not supported by this
		// device, so feature abort it with unrecognized opcode.
		// This means that the app layer must be sure to properly handle any messages
		// that it should be able to handle.

		case CECOP_CEC_VERSION:
			if ( isDirectAddressed ) {                 // Ignore as direct message
				SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: CECOP_CEC_VERSION Received: %d\n", pMsg->args[0]));
			}
			break;
		case CECOP_REPORT_CURRENT_LATENCY:
			SII_LIB_LOG_DEBUG2(("CecMsgHandlerLast():: CECOP_CEC_Report Latency Received: \n"));
			break;
		case CECOP_MENU_REQUEST:
			{
			mt_u8 request_status = pMsg->args[0];
			SII_LIB_LOG_DEBUG2(("HDMI_CEC_MENU_REQUEST_MSG request_status %d\n", request_status));
			if(request_status == 0 || request_status == 1)//0 ACTIVATED, 1 DEACTIVATED
			{
				g_menu_status_si = request_status;
				SiiCecSendMenuStatus(spCecInstData->logicalAddr, srcAddr, g_menu_status_si);
			}
			else if(request_status == 2)// 2 QUERY
			{
				SiiCecSendMenuStatus(spCecInstData->logicalAddr, srcAddr, g_menu_status_si);
			}
			else {
				SII_LIB_LOG_DEBUG2(("Warning, HDMI_CEC_MENU_REQUEST_MSG request_status %d is wrong!\n", request_status));
			}
			}
			break;
		case CECOP_ABORT:
		default:
			if ((pMsg->opcode != CECOP_ABORT) &&
					((pMsg->opcode == CECOP_ROUTING_CHANGE) || (pMsg->opcode == CECOP_ROUTING_INFORMATION) )) {
				break;
			}
			if ( isDirectAddressed ) {                  // Ignore as broadcast message
				SiiCecFeatureAbortSendEx( (uint8_t)pMsg->opcode, CECAR_UNRECOG_OPCODE, (SiiCecLogicalAddresses_t)srcAddr );
			}
			break;
	}
}

#if TPG_RECOVERY_UNUSED_FUNC
//------------------------------------------------------------------------------
static void CecHandleActiveSource ( SiiCpiData_t *pMsg )
{
	// Extract the logical and physical addresses of the new active source.

	spCecInstData->activeSrcLogical  = (pMsg->srcDestAddr >> 4) & 0x0F;
	spCecInstData->activeSrcPhysical = ((uint16_t)pMsg->args[0] << 8 ) | pMsg->args[1];

	SiiCecUpdateLogicalDeviceInfo( spCecInstData->activeSrcLogical, spCecInstData->activeSrcPhysical, true );

	// Determine the index of the HDMI port that is handling this physical address.

	SII_LIB_LOG_DEBUG2(("\nACTIVE_SOURCE: %02X (%04X) (port %02X)\n",
						(int)spCecInstData->activeSrcLogical, spCecInstData->activeSrcPhysical) );

}

//------------------------------------------------------------------------------
static void CecHandleInactiveSource ( SiiCpiData_t *pMsg )
{
	uint8_t la;

	la = (pMsg->srcDestAddr >> 4) & 0x0F;
	if ( la == spCecInstData->activeSrcLogical ) {  // The active source has deserted us!
		spCecInstData->activeSrcLogical  = CEC_LOGADDR_TV;
		spCecInstData->activeSrcPhysical = 0x0000;
	}
	// Signal upper layer that the active source has been lost

	spCecInstData->statusFlags |= SiiCEC_SOURCE_LOST;
}

//------------------------------------------------------------------------------
static void CecHandleReportPhysicalAddress ( SiiCpiData_t *pMsg )
{
	if (pMsg->args[2] != 0) {
		pMsg->srcDestAddr = (pMsg->srcDestAddr & 0x0F) | (pMsg->args[2] << 4) ;
	}
	SiiCecUpdateLogicalDeviceInfo((pMsg->srcDestAddr >> 4) & 0x0F,         // broadcast logical address
								  (((uint16_t)pMsg->args[0]) << 8) | pMsg->args[1],   // broadcast physical address
								  false
								 );
}
#endif

//---------------- CEC Tx Functions -------------------------------------------

/* One Touch Play Task internal states    */
enum {
	SiiCecTaskOtpIdle           = 0,
	SiiCecTaskOtpStart,
	SiiCecTaskOtpSendImageViewOn,
	SiiCecTaskOtpSentActiveSource,
};

//------------------------------------------------------------------------------
//! @brief  Send an ACTIVE SOURCE message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going active
//! @param[in]  - physicalAddr  Physical address of device
//------------------------------------------------------------------------------
void SiiCecSendActiveSource ( uint8_t logicalAddr, uint16_t physicalAddr )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_ACTIVE_SOURCE;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( logicalAddr, CEC_LOGADDR_UNREGORBC );
	cecFrame.args[0]        = (uint8_t)(physicalAddr >> 8);        // [Physical Address] High
	cecFrame.args[1]        = (uint8_t)(physicalAddr & 0xFF);      // [Physical Address] Low
	cecFrame.argCount       = 2;

	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  Send an INACTIVE SOURCE message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going inactive
//! @param[in]  - physicalAddr  Physical address of device
//------------------------------------------------------------------------------
void SiiCecSendInactiveSource ( uint8_t logicalAddr, uint16_t physicalAddr )
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_INACTIVE_SOURCE;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( logicalAddr, CEC_LOGADDR_UNREGORBC );
	cecFrame.args[0]        = (uint8_t)(physicalAddr >> 8);        // [Physical Address] High
	cecFrame.args[1]        = (uint8_t)(physicalAddr & 0xFF);      // [Physical Address] Low
	cecFrame.argCount       = 2;

	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  Send an SYSTEM STANDBY message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going active
//------------------------------------------------------------------------------
void SiiCecSendSysStandby ( uint8_t srclogicalAddr, uint8_t dstlogicalAddr)
{
	SiiCpiData_t cecFrame;

	cecFrame.opcode         = CECOP_STANDBY;
	cecFrame.srcDestAddr    = MAKE_SRCDEST( srclogicalAddr, dstlogicalAddr );
	cecFrame.argCount       = 0;

	SiiDrvCpiWrite( sCpiInst, &cecFrame );
}

//------------------------------------------------------------------------------
// Function:    CecTaskCpiWaitAck
// Description: Waits for an ACK from the last command sent.
//------------------------------------------------------------------------------

static uint8_t CecTaskCpiWaitAck ( SiiCpiStatus_t *pCecStatus )
{
	uint8_t         newTask = ACTIVE_TASK.task;

	if ( pCecStatus->txState == SiiTX_SENDFAILED ) {
		//DEBUG_PRINT( CEC_MSG_DBG,( "Task:: NoAck received\n" ));
		SII_LIB_LOG_DEBUG2(("CecTaskCpiWaitAck():: Task:: NoAck received\n"));

		/* Abort task */

		ACTIVE_TASK.cpiState    = CPI_IDLE;
		newTask                         = SiiCECTASK__IDLE;
	} else if ( pCecStatus->txState == SiiTX_SENDACKED ) {
		//DEBUG_PRINT( CEC_MSG_DBG,( "Task:: ACK received\n" ));
		SII_LIB_LOG_DEBUG2(("CecTaskCpiWaitAck():: Task:: ACK received\n"));
	}
	SII_LIB_LOG_DEBUG2(("CecTaskCpiWaitAck():: Task:: %d\n", (uint32_t)pCecStatus->txState));
	return ( newTask );
}

//------------------------------------------------------------------------------
// Function:    CecTaskOneTouchPlay
// Description: Implement One Touch Play message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskOneTouchPlay ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame;
	uint8_t         newTask = ACTIVE_TASK.task;

	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Current Task = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecTaskOtpStart:
					// Send an Image View On command to wake up the TV

					cecFrame.opcode             = CECOP_IMAGE_VIEW_ON;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
					cecFrame.argCount           = 0;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState       = SiiCecTaskOtpSendImageViewOn;
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;

				case SiiCecTaskOtpSendImageViewOn:
					// Now send an ACTIVE SOURCE message to tell them who we are
					SiiCecSendActiveSource( spCecInstData->logicalAddr, spCecInstData->physicalAddr );
					//DEBUG_PRINT( CEC_MSG_DBG, ("TX: Sent Active Source\n" ));
					spCecInstData->isActiveSource = true;
					SII_LIB_LOG_DEBUG2( ("CecTaskOneTouchPlay:: TX: Sent Active Source\n") );
					ACTIVE_TASK.taskState       = SiiCecTaskOtpSentActiveSource;
					break;

				case SiiCecTaskOtpSentActiveSource:
					newTask = SiiCECTASK__IDLE;
					spCecInstData->isActiveSource = true;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

/* One Standby internal states    */
enum {
	SiiCecSysStandbyTaskOtpIdle           = 0,
	SiiCecSysStandbyTaskOtpStart,
	SiiCecSysStandbyTaskOtpSendStandby,
};
//------------------------------------------------------------------------------
// Function:    CecTaskSysStandby
// Description: Implement System Standby message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskSysStandby ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame;
	uint8_t         newTask = ACTIVE_TASK.task;

	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Current Task = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecSysStandbyTaskOtpStart:
					// Send an Standby command to the TV

					cecFrame.opcode             = CECOP_STANDBY;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
					cecFrame.argCount           = 0;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState       = SiiCecSysStandbyTaskOtpSendStandby;
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;
				default:
					newTask = SiiCECTASK__IDLE;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

/* One Remote Control Pass Through internal states    */
enum {
	SiiCecRemotePassTaskOtpIdle           = 0,
	SiiCecRemotePassTaskOtpStart,
	SiiCecRemotePassTaskOtpSendRelease,
};
//------------------------------------------------------------------------------
// Function:    CecTaskSysStandby
// Description: Implement System Standby message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskRemotePass ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame;
	uint8_t         newTask = ACTIVE_TASK.task;

	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Current Task = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecRemotePassTaskOtpStart:
					// Send an press command to the TV

					cecFrame.opcode             = CECOP_USER_CONTROL_PRESSED;
					cecFrame.args[0]		= ACTIVE_TASK.taskData1;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
					cecFrame.argCount           = 1;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState       = SiiCecRemotePassTaskOtpIdle;
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;
				case SiiCecRemotePassTaskOtpSendRelease:
					// Send an release command to the TV

					cecFrame.opcode 			= CECOP_USER_CONTROL_RELEASED;
					cecFrame.srcDestAddr		= MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
					cecFrame.argCount			= 0;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState		= SiiCecRemotePassTaskOtpIdle;
					ACTIVE_TASK.cpiState		= CPI_WAIT_ACK;
					break;
				default:
					newTask = SiiCECTASK__IDLE;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

/* Get CEC Verison Task internal states    */
enum {
	SiiCecTaskCECVerOtpIdle           = 0,
	SiiCecTaskCECVerOtpStart,
};

//------------------------------------------------------------------------------
// Function:    CecTaskGetCecVerison
// Description: Implement Get CEC Version message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskGetCECVerison ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame;
	uint8_t         newTask = ACTIVE_TASK.task;

	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Current Task = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecTaskCECVerOtpStart:
					// Send an Get CEC Verison command to the TV

					cecFrame.opcode             = CECOP_GET_CEC_VERSION;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_TV );
					cecFrame.argCount           = 0;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState       = SiiCecTaskCECVerOtpIdle;
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;
				default:
					newTask = SiiCECTASK__IDLE;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

/* Report Current Latency Task internal states    */
enum {
	SiiCecTaskRptLatOtpIdle           = 0,
	SiiCecTaskRptLatOtpStart,
};

//------------------------------------------------------------------------------
// Function:    CecTaskReportCurLatency
// Description: Implement Report Current Latency message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskReportCurLatency ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame;
	uint8_t         newTask = ACTIVE_TASK.task;

	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Current Task = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecTaskRptLatOtpStart:
					// Send an Report Current Latency command to the TV

					cecFrame.opcode             = CECOP_REQUEST_CURRENT_LATENCY;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, CEC_LOGADDR_UNREGORBC );
					cecFrame.args[0]		= (uint8_t)((spCecInstData->physicalAddr) >> 8); 	  // [Physical Address] High
					cecFrame.args[1]		= (uint8_t)((spCecInstData->physicalAddr) & 0xFF);	   // [Physical Address] Low
					cecFrame.argCount           = 2;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState       = SiiCecTaskRptLatOtpIdle;
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;
				default:
					newTask = SiiCECTASK__IDLE;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

/* common msg internal states    */
enum {
	SiiCecTaskSendMsgAckIdle           = 0,
	SiiCecTaskSendMsgAckStart,
	SiiCecTaskSendMsgAckRemoteRelease,
};
//------------------------------------------------------------------------------
// Function:    CecTaskSendMsgAck
// Description: Implement common send message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskSendMsgAck ( SiiCpiStatus_t *pCecStatus )
{
	SiiCpiData_t    cecFrame = {0};
	uint8_t         newTask = ACTIVE_TASK.task;

//	SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Task Queue cpiState = %d, Current State:%d \n", __func__, __LINE__, (uint32_t)ACTIVE_TASK.cpiState, (uint32_t)ACTIVE_TASK.taskState) );
	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			switch ( ACTIVE_TASK.taskState ) {
				case SiiCecTaskSendMsgAckStart:
					// Send an Image View On command to wake up the TV

					cecFrame.opcode             = ACTIVE_TASK.taskData1;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, ACTIVE_TASK.destLA );
					cecFrame.argCount           = ACTIVE_TASK.taskData2;
					if ( cecFrame.argCount ) {
						uint8_t i;
						for (i = 0; i < cecFrame.argCount; i++) {
							cecFrame.args[i] = ACTIVE_TASK.pTaskData3[i];
						}
						{
							SiiLibMallocDelete(ACTIVE_TASK.pTaskData3);
							//SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Release mem:%p \n", __func__,__LINE__,ACTIVE_TASK.pTaskData3 ));
							ACTIVE_TASK.pTaskData3 = NULL;
						}
					}

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					if ( cecFrame.opcode == CECOP_USER_CONTROL_PRESSED ) {
						ACTIVE_TASK.taskState		= SiiCecTaskSendMsgAckRemoteRelease;
					} else {
						ACTIVE_TASK.taskState		= SiiCecTaskSendMsgAckIdle;
					}
					ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
					break;
				case SiiCecTaskSendMsgAckRemoteRelease:
					cecFrame.opcode 			= CECOP_USER_CONTROL_RELEASED;
					cecFrame.srcDestAddr        = MAKE_SRCDEST( spCecInstData->logicalAddr, ACTIVE_TASK.destLA );
					cecFrame.argCount			= 0;

					SiiDrvCpiWrite( sCpiInst, &cecFrame );
					ACTIVE_TASK.taskState		= SiiCecTaskSendMsgAckIdle;
					ACTIVE_TASK.cpiState		= CPI_WAIT_ACK;
					break;
				case SiiCecTaskSendMsgAckIdle:
					newTask = SiiCECTASK__IDLE;
					break;
			}

			break;

		case CPI_WAIT_ACK:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
			break;
		default:
			break;
	}

	return ( newTask );
}

//------------------------------------------------------------------------------
// Function:    SI_CecOneTouchPlay
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecOneTouchPlay ( void )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__ONETOUCH;
	newTask.taskState   = SiiCecTaskOtpStart;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    SI_CecVersion
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecVersionGet ( void )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__GETCECVERSION;
	newTask.taskState   = SiiCecTaskCECVerOtpStart;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    SI_Report Current Latency
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecReportCurLatency ( void )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__RPTCURLATENCY;
	newTask.taskState   = SiiCecTaskRptLatOtpStart;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    SiiCecSysStandby
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecSysStandby ( void )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__SYSSTANDBY;
	newTask.taskState   = SiiCecSysStandbyTaskOtpStart;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    SiiCecRemotePassPress
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecRemotePassPress ( uint8_t val )
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__REMOTEPASSPRESS;
	newTask.taskState   = SiiCecRemotePassTaskOtpStart;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = val;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

bool_t SiiCecRemotePassRelease (void)
{
	SiiCecTaskState_t  newTask;

	newTask.task        = SiiCECTASK__REMOTEPASSRELEASE;
	newTask.taskState   = SiiCecRemotePassTaskOtpSendRelease;
	newTask.destLA      = 0;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = 0;
	newTask.taskData2   = 0;
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    SI_CecOneTouchPlay
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecSendMsgAck ( SiI_CEC_t *cec_frame )
{
	SiiCecTaskState_t  newTask;

	if(CECOP_IMAGE_VIEW_ON == cec_frame->bOpcode){
		spCecInstData->isActiveSource = true;
		//printk(KERN_EMERG "++++active = ture\n");
	}

	newTask.task        = SiiCECTASK__SENDMSGACK_MT;
	newTask.taskState   = SiiCecTaskSendMsgAckStart;
	newTask.destLA      = cec_frame->bDestOrRXHeader;
	newTask.cpiState    = CPI_IDLE;
	newTask.taskData1   = cec_frame->bOpcode;
	newTask.taskData2   = cec_frame->bCount;
	if ( cec_frame->bCount ) {
		newTask.pTaskData3 = SiiLibMallocCreate(cec_frame->bCount);
		//SII_LIB_LOG_DEBUG2( ("[%s_%d]CEC:: Malloc mem:%p \n", __func__,__LINE__,newTask.pTaskData3 ));
		if ( newTask.pTaskData3 ) {
			memcpy(newTask.pTaskData3, cec_frame->bOperand, cec_frame->bCount);
		} else {
			return false;
		}
	} else {
		newTask.pTaskData3 = NULL;
	}
	return ( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------------------
static bool_t CecMsgHandler(SiiCpiData_t *pMsg)
{
	bool_t  isDirectAddressed, usedMessage;

	isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
	usedMessage         = false;

	switch ( pMsg->opcode ) {
		case CECOP_ACTIVE_SOURCE:
			if ( !isDirectAddressed ) {                 // Ignore as direct message
				switch ((pMsg->srcDestAddr & 0xF0) >> 4) {
					case CEC_LOGADDR_PLAYBACK1:
					case CEC_LOGADDR_PLAYBACK2:
					case CEC_LOGADDR_PLAYBACK3:
					case CEC_LOGADDR_TUNER1:
					case CEC_LOGADDR_TUNER2:
					case CEC_LOGADDR_TUNER3:
					case CEC_LOGADDR_TUNER4:
						//              case CEC_LOGADDR_TV:
						spCecInstData->isActiveSource = false;
						//            	DEBUG_PRINT(MSG_ALWAYS,"used Message in ACTIVE SOURCE\n");
						usedMessage = true;
						break;
					default:
						break;
				}
			} else {
				usedMessage = true;
			}
			break;

		case CECOP_REQUEST_ACTIVE_SOURCE:
			if ( !isDirectAddressed ) {                 // Ignore as direct message
				if ( spCecInstData->isActiveSource ) {
					SiiCecSendActiveSource( spCecInstData->logicalAddr, spCecInstData->physicalAddr );
					if ( spCecInstData->CecEvtNotifycbFunc ) {
						spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_CEC_STATUS_POWER_ON);
					}
				}
			}
			usedMessage = true;
			break;
		default:
			break;
	}

	return ( usedMessage );
}

//------------------------------------------------------------------------------

static void UpdateTaskQueueState ( void )
{
	if ( spCecInstData->currentTask == SiiCECTASK__IDLE ) {
		ACTIVE_TASK.queueState = SiiCecTaskQueueStateIdle;
		spCecInstData->taskQueueOut = (spCecInstData->taskQueueOut + 1) % TASK_QUEUE_LENGTH;
	}
}

//------------------------------------------------------------------------------
static uint8_t CecTaskSendMsg ( SiiCpiStatus_t *pCecStatus )
{
	uint8_t newTask = ACTIVE_TASK.task;

	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			ACTIVE_TASK.cpiState = CPI_IDLE;
			newTask = SiiCECTASK__IDLE;
			break;

		case CPI_WAIT_ACK:
			// Make sure this message status is associated with the message we sent.
			if ( pCecStatus->msgId != ACTIVE_TASK.msgId ) {
				break;
			}
			if ( pCecStatus->txState == SiiTX_SENDFAILED ) {
				// Remove LA from active list
				//DEBUG_PRINT( CEC_MSG_DBG, "Remove LA %X from CEC list\n", ACTIVE_TASK.destLA );
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].devPA      = 0xFFFF;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = CEC_DT_COUNT;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected   = false;
			}
			ACTIVE_TASK.cpiState = CPI_IDLE;
			break;
		default:
			ACTIVE_TASK.cpiState = CPI_IDLE;
			break;
	}

	return ( newTask );
}

//------------------------------------------------------------------------------
//! @brief  Return state of current enumeration task.
//------------------------------------------------------------------------------
bool_t SiiCecEnumerateIsComplete ( void )
{
	return ( spCecInstData->enumerateComplete );
}

//------------------------------------------------------------------------------
//! @brief  Return availability of passed logical address.
//------------------------------------------------------------------------------
bool_t SiiCecDeviceLaIsAvailable ( uint8_t deviceLa )
{
	return ( spCecInstData->logicalDeviceInfo[ deviceLa].deviceType == CEC_DT_COUNT );
}

//------------------------------------------------------------------------------
//! @brief      Returns an available logical address from the passed list of
//!             devices.
//! @param[in]  pDeviceList - list of logical addresses to check
//! @return     Available logical address or CEC_LOGADDR_UNREGORBC.
//------------------------------------------------------------------------------
SiiCecLogicalAddresses_t SiiCecGetAvailableLa ( uint8_t *pDeviceLaList )
{
	int     i = 0;
	bool_t  laFound = false;

	// We must have already enumerated
	if ( spCecInstData->enumerateComplete ) {
		for ( i = 0; pDeviceLaList[i] < CEC_LOGADDR_UNREGORBC; i++ ) {
			//CecPrintLogAddr( pDeviceLaList[i] );
			if ( SiiCecDeviceLaIsAvailable( pDeviceLaList[i] )) {
				laFound = true;
				break;
			}
		}

		if ( !laFound ) {
			SII_LIB_LOG_DEBUG2(( "CEC : No LA available from list\n" ));
		}
	} else {
		SII_LIB_LOG_DEBUG2(( "CEC : Must be enumerated to set LA\n" ));
	}

	return ( laFound ? pDeviceLaList[i] : CEC_LOGADDR_UNREGORBC );
}

//----------------------------------------------------------------------------
static void SiiCecTaskTimerSet(void)
{
	ACTIVE_TASK.taskTimer = (uint32_t)SiI_get_global_time();
}

static bool_t SiiCecTaskTimerExpired(uint32_t msTime)
{
	uint32_t processTime = (uint32_t)SiI_get_global_time();
	return ((processTime - ACTIVE_TASK.taskTimer) > msTime) ? true : false;
}

//------------------------------------------------------------------------------
static uint16_t SiiCecSendMessage( uint8_t opCode, uint8_t dest )
{
	SiiCpiData_t cecFrame;

	if ( !spCecInstData->enable ) {
		spCecInstData->lastResultCode = RESULT_CEC_FAIL;
		return ( 0 );
	}
	cecFrame.opcode        = opCode;
	cecFrame.srcDestAddr   = MAKE_SRCDEST( spCecInstData->logicalAddr, dest );
	cecFrame.argCount      = 0;

	SII_LIB_LOG_DEBUG2(("%s %d opCode=02X, dest=%02X \n", __FUNCTION__, __LINE__, opCode, dest));

	spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;
	return ( SiiDrvCpiWrite( sCpiInst, &cecFrame ));
}

//------------------------------------------------------------------------------
static uint8_t CecTaskGetConnectedDevices ( SiiCpiStatus_t *pCecStatus )
{
	uint8_t newTask = ACTIVE_TASK.task;

	switch ( ACTIVE_TASK.cpiState ) {
		case CPI_IDLE:

			// We're done if we've reached the unregistered address
			if ( ACTIVE_TASK.pTaskData3[ ACTIVE_TASK.taskData1] >= CEC_LOGADDR_UNREGORBC ) {
				spCecInstData->enumerateComplete = true;

				ACTIVE_TASK.cpiState = CPI_IDLE;
				newTask = SiiCECTASK__IDLE;
				break;
			}
			ACTIVE_TASK.destLA = ACTIVE_TASK.pTaskData3[ ACTIVE_TASK.taskData1++];
			SiiCecTaskTimerSet();
			ACTIVE_TASK.msgId = SiiDrvCpiSendPing( sCpiInst, ACTIVE_TASK.destLA );
			SII_LIB_LOG_DEBUG2(("CEC: Ping dest LA = %d, msgId=0x%x\n", (uint32_t)ACTIVE_TASK.destLA, (uint32_t)ACTIVE_TASK.msgId) );
			ACTIVE_TASK.cpiState = CPI_WAIT_ACK;
			break;

		case CPI_WAIT_ACK:
			if (SiiCecTaskTimerExpired( 1000 )) {
				SII_LIB_LOG_DEBUG2( ("Tx Enumerate: Timed out waiting for ack\n") );
				pCecStatus->txState = SiiTX_SENDFAILED;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].devPA = 0xFFFF;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = CEC_DT_COUNT;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected = false;

				spCecInstData->enumerateComplete = true;
				ACTIVE_TASK.cpiState = CPI_IDLE;
				newTask = SiiCECTASK__IDLE;

				// Restore Tx State to IDLE to try next address.
				ACTIVE_TASK.cpiState = CPI_IDLE;
				SII_LIB_LOG_DEBUG2(("CEC: Send Failed\n") );
				break;
			}
			// Make sure this message status is associated with the message we sent.
			if ( pCecStatus->msgId != ACTIVE_TASK.msgId ) {
				SII_LIB_LOG_DEBUG2(("CEC: wait ack...[0x%x  0x%x  0x%x ]\n", (uint32_t)pCecStatus->msgId, (uint32_t)ACTIVE_TASK.msgId, (uint32_t)pCecStatus->txState) );
				break;
			}
			if ( pCecStatus->txState == SiiTX_SENDFAILED ) {

				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].devPA = 0xFFFF;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = CEC_DT_COUNT;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected = false;

				spCecInstData->enumerateComplete = true;
				ACTIVE_TASK.cpiState = CPI_IDLE;
				newTask = SiiCECTASK__IDLE;

				// Restore Tx State to IDLE to try next address.
				ACTIVE_TASK.cpiState = CPI_IDLE;
				SII_LIB_LOG_DEBUG2(("CEC: Send Failed\n") );
			} else if ( pCecStatus->txState == SiiTX_SENDACKED ) {

				// Get the physical address from this source and add it to our
				// list if it responds within 2 seconds, otherwise, ignore it.
				ACTIVE_TASK.msgId = SiiCecSendMessage( CECOP_GIVE_PHYSICAL_ADDRESS, ACTIVE_TASK.destLA );
				SiiCecTaskTimerSet();
				ACTIVE_TASK.cpiState = CPI_WAIT_RESPONSE;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].devPA = 0;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = 0;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected = 0;
				SII_LIB_LOG_DEBUG2(("CEC: Send Acked\n") );

			} else {
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].devPA = 0;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = 0;
				spCecInstData->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected = 0;
				SII_LIB_LOG_DEBUG2(("CEC: HOW TO DO!!!\n") );
			}
			break;

		case CPI_WAIT_RESPONSE:
			if ( SiiCecTaskTimerExpired( 1000 )) {
				SII_LIB_LOG_DEBUG2( ("Tx Enumerate: Timed out waiting for response\n") );

				// Ignore this LA and move on to the next.
				ACTIVE_TASK.cpiState = CPI_IDLE;
			} else {
				SII_LIB_LOG_DEBUG2(("CEC: wait response\n") );
			}
			break;

		case CPI_RESPONSE:
			// The CEC Rx Message Handler has updated the child port list,
			// restore Tx State to IDLE to try next address.
			SII_LIB_LOG_DEBUG2(("CEC: received response\n") );
			ACTIVE_TASK.cpiState = CPI_IDLE;
			break;
	}

	return ( newTask );
}

SiiCecLogicalAddresses_t SiiCecGetDeviceLA ( void )
{
	spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;

	return( spCecInstData->logicalAddr);
}


//------------------------------------------------------------------------------
// Function:    SiiCecGetDevicePA
// Description: Return the physical address for this Host device
//------------------------------------------------------------------------------

uint16_t SiiCecGetDevicePA ( void )
{
	spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;

	return ( spCecInstData->physicalAddr );
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetDevicePA
// Description: Set the host device physical address (initiator physical address)
//------------------------------------------------------------------------------

void SiiCecSetDevicePA ( uint16_t devPa )
{
	uint8_t     index;
	uint16_t    mask;

	spCecInstData->physicalAddr = devPa;

	/* We were given the Host, or parent, PA, so we determine   */
	/* the direct child Physical Address field (A.B.C.D).       */

	mask = 0x00F0;
	for ( index = 1; index < 4; index++ ) {
		if (( devPa & mask ) != 0) {
			break;
		}
		mask <<= 4;
	}

	spCecInstData->paShift = (uint8_t)((index - 1) * 4);
	spCecInstData->paChildMask = 0x000F << spCecInstData->paShift;

}

//------------------------------------------------------------------------------
// Function:    SiiCecSetDeviceLA
// Description: Set the CEC logical address for this Host device
//------------------------------------------------------------------------------

void SiiCecSetDeviceLA ( SiiCecLogicalAddresses_t logicalAddr )
{
	spCecInstData->lastResultCode = RESULT_CEC_SUCCESS;
	spCecInstData->logicalAddr = logicalAddr;
	spCecInstData->deviceType = (SiiCecDeviceTypes_t)l_devTypes[logicalAddr];
	SiiDrvCpiSetLogicalAddr(sCpiInst, logicalAddr);
	SII_LIB_LOG_DEBUG2( ("CEC:: Set Device LA = %d for this Host Device\n", logicalAddr) );
}

static void CecTaskServer ( SiiCpiStatus_t *pCecStatus )
{
	uint8_t logicalAddr;

	if (spCecInstData->currentTask > 0) {
		SII_LIB_LOG_DEBUG2( ("CEC:: Current Task = %d, qout=%d \n", (uint32_t)spCecInstData->currentTask, (uint32_t)spCecInstData->taskQueueOut) );
	}
	switch ( spCecInstData->currentTask ) {
		case SiiCECTASK__CANCEL:

			// If a task is in progress, cancel it.
			if ( spCecInstData->taskQueue[ spCecInstData->taskQueueOut].queueState == SiiCecTaskQueueStateRunning ) {
				spCecInstData->taskQueue[ spCecInstData->taskQueueOut].queueState = SiiCecTaskQueueStateIdle;
				spCecInstData->currentTask = SiiCECTASK__IDLE;
			}
			break;

		case SiiCECTASK__IDLE:
			if ( spCecInstData->taskQueue[ spCecInstData->taskQueueOut].queueState == SiiCecTaskQueueStateQueued ) {
				spCecInstData->taskQueue[ spCecInstData->taskQueueOut].queueState = SiiCecTaskQueueStateRunning;
				spCecInstData->currentTask = spCecInstData->taskQueue[ spCecInstData->taskQueueOut].task;
			}
			break;

		case SiiCECTASK__ENUMERATE:
			SII_LIB_LOG_DEBUG2(("CEC: Enum...\n") );
			spCecInstData->currentTask = CecTaskGetConnectedDevices( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__SETLA:
			logicalAddr = SiiCecGetAvailableLa( ACTIVE_TASK.pTaskData3 );
			if ( logicalAddr < CEC_LOGADDR_UNREGORBC ) {
				SiiCecSetDeviceLA( (SiiCecLogicalAddresses_t)logicalAddr );
				SiiCecSendReportFeatures();
				SiiCecSendReportPhysicalAddress( );

				#if __HDMI_OS_RTOS__
				{
				extern void hdmi_cec_status_update(uint8_t status);
				hdmi_cec_status_update(CEC_READY_STATUS);
				}
				if ( spCecInstData->CecEvtNotifycbFunc ) {
					spCecInstData->CecEvtNotifycbFunc(SII_CEC_EVENT_CEC_STATUS_READY);
				}
				#endif
			}
			spCecInstData->currentTask = SiiCECTASK__IDLE;
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__ONETOUCH:
			spCecInstData->currentTask = CecTaskOneTouchPlay( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__SENDMSG:
			spCecInstData->currentTask = CecTaskSendMsg( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__SYSSTANDBY:
			spCecInstData->currentTask = CecTaskSysStandby( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__REMOTEPASSPRESS:
			spCecInstData->currentTask = CecTaskRemotePass( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__REMOTEPASSRELEASE:
			spCecInstData->currentTask = CecTaskRemotePass( pCecStatus );
			UpdateTaskQueueState();
			break;

		case SiiCECTASK__GETCECVERSION:
			spCecInstData->currentTask = CecTaskGetCECVerison( pCecStatus );
			UpdateTaskQueueState();
			break;
		case SiiCECTASK__RPTCURLATENCY:
			spCecInstData->currentTask = CecTaskReportCurLatency( pCecStatus );
			UpdateTaskQueueState();
			break;
		case SiiCECTASK__SENDMSGACK_MT:
			spCecInstData->currentTask = CecTaskSendMsgAck( pCecStatus );
			UpdateTaskQueueState();
			break;

		default:
			break;
	}
}

//------------------------------------------------------------------------------
static void SiiCecCallBack(SiiInst_t inst)
{
	bool_t          processedMsg;
	SiiCpiStatus_t  cecStatus;
	SiiCpiData_t    cecFrame;
	uint_t          frameCount;

	sCpiInst = inst;

	if (spCecInstData->enable) {
		SiiDrvCpiServiceWriteQueue(sCpiInst);               // Send any pending messages
		SiiDrvCpiHwStatusGet( sCpiInst, &cecStatus );         // Get the CEC transceiver status

		if (cecStatus.rxState) {
			for (;;) {
				frameCount = SiiDrvCpiFrameCount(sCpiInst);
				if ( frameCount == 0 ) {
					break;
				}
				if ( !SiiDrvCpiRead( sCpiInst, &cecFrame )) {
					SII_LIB_LOG_DEBUG2(("Error in Rx Fifo\n") );
					break;
				}
				processedMsg = false;
				if ( !CecValidateMessage( &cecFrame )) { // If invalid message, ignore it, but treat it as handled
					processedMsg = true;
				}
				if ( !processedMsg && CecMsgHandlerFirst( &cecFrame )) { // Handle the common system messages
					processedMsg = true;
				}
				if (!processedMsg && CecMsgHandler(&cecFrame)) {
					processedMsg = true;
				}
				if ( !processedMsg ) {
					CecMsgHandlerLast( &cecFrame );   // Let the built-in handler take care of leftovers.
				}
			}

		}

		CecTaskServer(&cecStatus);

	}

}

//------------------------------------------------------------------------------
static bool_t CecAddTask ( SiiCecTaskState_t *pNewTask )
{
	bool_t      success = true;

	// Store the message in the task queue
	if ( spCecInstData->taskQueue[ spCecInstData->taskQueueIn ].queueState == SiiCecTaskQueueStateIdle ) {
		memcpy( &spCecInstData->taskQueue[ spCecInstData->taskQueueIn ], pNewTask, sizeof( SiiCecTaskState_t ));
		spCecInstData->taskQueue[ spCecInstData->taskQueueIn ].queueState = SiiCecTaskQueueStateQueued;

		spCecInstData->taskQueueIn = (spCecInstData->taskQueueIn + 1) % TASK_QUEUE_LENGTH;
	} else {
		success = false;
	}

	spCecInstData->lastResultCode = (success) ? RESULT_CEC_SUCCESS : RESULT_CEC_TASK_QUEUE_FULL;
	return ( success );
}

//------------------------------------------------------------------------------
void SiiCecEnumerateDevices ( uint8_t inst, bool_t enumerate, uint8_t *pDeviceList )
{
	SiiCecTaskState_t  newTask;

	sCpiInst = spCecInstData->cpiInst[inst];

	if (!enumerate) {
		if ( spCecInstData->currentTask == SiiCECTASK__ENUMERATE ) {
			spCecInstData->currentTask = SiiCECTASK__CANCEL;
			CecTaskServer( NULL );
			spCecInstData->enumerateComplete = true;
		}
	} else {
		spCecInstData->enumerateComplete = false;
		newTask.task        = SiiCECTASK__ENUMERATE;
		newTask.taskState   = 0;
		newTask.destLA      = 0;
		newTask.cpiState    = CPI_IDLE;
		newTask.taskData1   = 0;
		newTask.taskData2   = 0;
		newTask.pTaskData3  = pDeviceList;
		CecAddTask( &newTask );
	}
}

//------------------------------------------------------------------------------
void SiiCecUpdatePowerState(SiiCecPowerstatus_t pwrstat)
{
	spCecInstData->powerState = pwrstat;
}

SiiCecPowerstatus_t SiiCecGetPowerState(void)
{
	return spCecInstData->powerState;
}

//------------------------------------------------------------------------------
void SiiCecEnable(void)
{
	uint8_t i;
	for (i = 0; i < NUM_CPI_INSTANCES; i++) {
		SiiDrvCpiEnable(spCecInstData->cpiInst[i]);
	}
	spCecInstData->enable = true;
}

//------------------------------------------------------------------------------
void SiiCecDisable(void)
{
	uint8_t i;
	for (i = 0; i < NUM_CPI_INSTANCES; i++) {
		SiiDrvCpiDisable(spCecInstData->cpiInst[i]);
	}
	spCecInstData->enable = false;
}

bool_t SiiCecEnable_Status(void)
{
	if (spCecInstData) {
		return spCecInstData->enable;
	} else {
		return 0;
	}
}
//------------------------------------------------------------------------------
void SiiCecUpdatePhysicalAdress(uint16_t devPa)
{
	spCecInstData->physicalAddr = devPa;
	SII_LIB_LOG_DEBUG2(("PhysicalAddr:0x%x\n", devPa) );
}

//------------------------------------------------------------------------------
void SiiCecDeviceCreate(SiiInst_t inst, uint16_t cpiBaseAddr )
{
	/*uint16_t i = 0;*/
	spCecInstData = (CecInstanceData_t *) SiiLibMallocCreate(sizeof(CecInstanceData_t));
	if ( spCecInstData == NULL ) {
		SII_LIB_LOG_DEBUG2(("SiiCecDeviceCreate Malloc fail\n") );
		return;
	}

	SiiCecReset ();

	sCpiInst = spCecInstData->cpiInst[0] = SiiDrvCpiCreate(inst, cpiBaseAddr, SiiCecCallBack);
}

void SiiCecDeviceDelete(void)
{
	SiiDrvCpiDelete(sCpiInst);
	SiiLibMallocDelete(spCecInstData);
}

void SiiCecReset( void )
{
	uint16_t i = 0;

	spCecInstData->logicalAddr = CEC_LOGADDR_TUNER1;
	spCecInstData->powerState = CEC_POWERSTATUS_STANDBY;
	spCecInstData->sourcePowerStatus = CEC_POWERSTATUS_STANDBY;

	spCecInstData->lastUserControlPressedSourceLa = CEC_LOGADDR_UNREGORBC;
	spCecInstData->lastUserControlPressedTargetLa = CEC_LOGADDR_UNREGORBC;

	SiiCecSetDevicePA( 0xFFFF );

	for ( i = 0; i <= CEC_LOGADDR_UNREGORBC; i++ ) {
		spCecInstData->logicalDeviceInfo[i].devPA = 0;
		spCecInstData->logicalDeviceInfo[i].devLA = 0;
		spCecInstData->logicalDeviceInfo[i].deviceType = 0;
	}
	// fix: when do more plug-in and then quickly plug-out times, the cec sending message maybe failed
	// because:  cectask add in taskQueueIn++ always(0 1 2...5 0 1 2...5), the first current taskQueueOut msg
	//           may not be the first ping message when plug-in.
	SiiDrvCpiReset(spCecInstData->cpiInst[0]);
	spCecInstData->currentTask = SiiCECTASK__IDLE;
	spCecInstData->enumerateComplete = false;
	spCecInstData->taskQueueOut = 0;
	spCecInstData->taskQueueIn = 0;
	for ( i = 0; i < TASK_QUEUE_LENGTH; i++) {
		spCecInstData->taskQueue[i].queueState = SiiCecTaskQueueStateIdle;
	}
}

void SiiCecCreate(SiiInst_t instCra)
{
	//SiiCecDeviceCreate(instCra, 0x3F00);
	SiiCecDeviceCreate(instCra, 0x0F00);
	SiiCecEnable();
}

void SiiCecDelete(void)
{
	SiiCecDeviceDelete();
	sCpiInst = 0;
}
SiiInst_t	SiiGetCecInst(void)
{
	return sCpiInst;
}

void SiiCecRegisterEventNotifyCallBack(SiiInst_t inst, CecEventcbFunc cbFunc)
{
	spCecInstData->CecEvtNotifycbFunc = cbFunc;
}

bool_t SiiCecUpdateOsdName(char *newOsdName)
{
	if (NULL == newOsdName) {
		return false;
	}

	memset(l_cecTxOsdNameString, '\0', 15);
	strncpy(l_cecTxOsdNameString, newOsdName, 13);
	SII_LIB_LOG_DEBUG2(("CEC:app-> osd name SiiCecUpdateOsdName:%s\n", l_cecTxOsdNameString));
	return true;
}

SiiCecPowerstatus_t SiiCecGetSinkPowerOnStatus(void)
{
	return spCecInstData->sourcePowerStatus;
}

