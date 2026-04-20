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
* @brief cec
*
*****************************************************************************/

#ifndef __SI_APP_CEC_H__
#define __SI_APP_CEC_H__

#define SUPPORT_CEC_2P		(1)
#define CEC_2P_CTS_4PBORTUNER		(1)
#define SUPPORT_CEC_VENDOR_CMD	(1)

#include "si_lib_obj_api.h"
#include "si_cec_enums.h"
#define MAKE_SRCDEST( src, dest )   ((( ((uint8_t)src) << 4) & 0xF0) | (((uint8_t)dest) & 0x0F))
#define SII_MAX_CMD_SIZE 16 // defines number of operands
typedef struct {
	uint8_t bCount;
	uint8_t bRXNextCount;
	uint8_t bDestOrRXHeader;
	uint8_t bOpcode;
	uint8_t bOperand[ SII_MAX_CMD_SIZE ];
} SiI_CEC_t;

typedef enum _SiiCecPowerstatus_t { // Operands for <Power Status> Opcode
	CEC_POWERSTATUS_ON              = 0x00,
	CEC_POWERSTATUS_STANDBY         = 0x01,
	CEC_POWERSTATUS_STANDBY_TO_ON   = 0x02,
	CEC_POWERSTATUS_ON_TO_STANDBY   = 0x03,
} SiiCecPowerstatus_t;

typedef enum _SiiCecEvent_E {
	SII_CEC_EVENT_TV_SEND_CEC_STANDBY,
	SII_CEC_EVENT_TV_SEND_CEC_OK,
	SII_CEC_EVENT_TV_SEND_CEC_UP,
	SII_CEC_EVENT_TV_SEND_CEC_DOWN,
	SII_CEC_EVENT_TV_SEND_CEC_LEFT,
	SII_CEC_EVENT_TV_SEND_CEC_RIGHT,
	SII_CEC_EVENT_TV_SEND_CEC_VOL_UP,
	SII_CEC_EVENT_TV_SEND_CEC_VOL_DOWN,
	SII_CEC_EVENT_TV_SEND_CEC_MUTE,
	SII_CEC_EVENT_CEC_STATUS_INIT,
	SII_CEC_EVENT_CEC_STATUS_READY,
	SII_CEC_EVENT_CEC_STATUS_POWER_ON,
	SII_CEC_EVENT_CEC_STATUS_PORT_SWTICH_OUT,
	SII_CEC_EVENT_MAX
} SiiCecEvent_E;

typedef void (*CecEventcbFunc)(SiiCecEvent_E);

void SiiCecCreate(SiiInst_t instCra);
void SiiCecDelete(void);
void SiiCecDeviceCreate( SiiInst_t inst, uint16_t cpiBaseAddr );
void SiiCecDeviceDelete(void);
//static void SiiCecCallBack(SiiInst_t inst);
void SiiCecEnumerateDevices ( uint8_t inst, bool_t enumerate, uint8_t *pDeviceList );
bool_t SiiCecEnumerateDeviceLa ( uint8_t *pDeviceList );
void SiiCecUpdatePowerState(SiiCecPowerstatus_t pwrstat);
void SiiCecEnable(void);
void SiiCecUpdatePhysicalAdress(uint16_t devPa);
void SiiCecSetDevicePA ( uint16_t devPa );
void SiiCecSetSourceActive( bool_t isActiveSource );
void SiiCecSendActiveSource ( uint8_t logicalAddr, uint16_t physicalAddr );
bool_t SiiCecOneTouchPlay ( void );
void SiiCecSetDeviceLA ( SiiCecLogicalAddresses_t logicalAddr );
void SiiCecReset( void );
uint16_t SiiCecSendReportPhysicalAddress ( void );
uint16_t SiiCecSendReportFeatures ( void );
SiiInst_t	SiiGetCecInst(void);

bool_t SiiCecSysStandby ( void );
bool_t SiiCecRemotePassPress ( uint8_t val );
bool_t SiiCecRemotePassRelease (void);
bool_t SiiCecVersionGet ( void );
bool_t SiiCecReportCurLatency ( void );
bool_t SiiCecSendMsgAck ( SiI_CEC_t *cec_frame );
void SiiCecRegisterEventNotifyCallBack(SiiInst_t inst, CecEventcbFunc cbFunc);
bool_t SiiCecUpdateOsdName(char *newOsdName);
SiiCecPowerstatus_t SiiCecGetSinkPowerOnStatus(void);
SiiCecPowerstatus_t SiiCecGetPowerState(void);
void SiiCecDisable(void);
bool_t SiiCecEnable_Status(void);
#endif //__SI_APP_CEC__
