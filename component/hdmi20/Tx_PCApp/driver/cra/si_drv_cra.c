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
* @file si_drv_cra.c
*
* @brief CRA (common register access) driver
*
*****************************************************************************/
//#define SII_DEBUG

/***** #include statements ***************************************************/

#include "si_drv_cra_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_malloc_api.h"
#include "si_lib_time_api.h"
/***** local data objects ****************************************************/
#define CRA_RETRY_COUNT	3
#define SIMG_PLATFORM 0

SII_LIB_OBJ_MODULE_DEF(si_cra);

typedef struct {
	SiiDrvCraConfig_t *pConfig;
} CraObj_t;

/***** public functions ******************************************************/
SiiInst_t SiiDrvCraCreate( SiiDrvCraConfig_t *pCfg)
{
	uint8_t i;
	SiiPlatformStatus_t platStat = SII_PLATFORM_STATUS__FAILED;
	CraObj_t *pObj = (CraObj_t *)SII_LIB_OBJ_CREATE("cra", sizeof(CraObj_t));
	SII_PLATFORM_DEBUG_ASSERT(pObj);

	pObj->pConfig = (SiiDrvCraConfig_t *) SiiLibMallocCreate(sizeof(SiiDrvCraConfig_t));
	SII_PLATFORM_DEBUG_ASSERT(pObj->pConfig);

	if ( pObj->pConfig ) {
		memcpy(pObj->pConfig, pCfg, sizeof(SiiDrvCraConfig_t));
	} else {
		//SII_LIB_LOG_PRINT2(("SiiDrvCraCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		if (pObj->pConfig->pInterfaceInfo->interfaceHardware == SII_PLATFORM_HARDWARE__BB) {
			int j;
			//if all gpios of BB header are not configured as BIDI, GPIO out functionality is not achieved.
			// also pin 3 and pin 5 (SII_PLATFORM_GPIO__1 and SII_PLATFORM_GPIO__2 correspondingly) are working for GPIO ouput.
			for (j = SII_PLATFORM_GPIO__0; j <= SII_PLATFORM_GPIO__7; j++) {
				platStat = SiiPlatformGPIOConfig(pObj->pConfig->pInterfaceInfo, (SiiPlatformGPIO_t)j, SII_PLATFORM_GPIO_DIRECTION__INOUT);
			}
		} else {
			platStat = SiiPlatformGPIOConfig(pObj->pConfig->pInterfaceInfo, pObj->pConfig->intrPin, SII_PLATFORM_GPIO_DIRECTION__INPUT);
			platStat |= SiiPlatformGPIOConfig(pObj->pConfig->pInterfaceInfo, pObj->pConfig->rstPin, SII_PLATFORM_GPIO_DIRECTION__OUTPUT);
		}

		platStat |= SiiPlatformGPIOSet(pObj->pConfig->pInterfaceInfo, pObj->pConfig->rstPin, SII_PLATFORM_GPIO_LEVEL__HIGH);

		if (SII_PLATFORM_STATUS__SUCCESS == platStat) {
			return SII_LIB_OBJ_INST(pObj);
		}
	}
	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
	return (SiiInst_t)NULL;
}

void SiiDrvCraDelete(SiiInst_t craInst)
{
	CraObj_t *pObj = (CraObj_t *)SII_LIB_OBJ_PNTR(craInst);
	SiiLibMallocDelete(pObj->pConfig);
	SII_LIB_OBJ_DELETE(pObj);
}

bool_t SiiDrvCraHardwareReset(SiiInst_t craInst)
{
	#if SIMG_PLATFORM
	SiiPlatformStatus_t platStatus;
	CraObj_t *pObj = (CraObj_t *)SII_LIB_OBJ_PNTR(craInst);
	platStatus = SiiPlatformGPIOSet(pObj->pConfig->pInterfaceInfo, pObj->pConfig->rstPin, SII_PLATFORM_GPIO_LEVEL__LOW);
	if (platStatus) {
		return false;
	}
	SiiLibTimeMilliDelay(100);
	platStatus = SiiPlatformGPIOSet(pObj->pConfig->pInterfaceInfo, pObj->pConfig->rstPin, SII_PLATFORM_GPIO_LEVEL__HIGH);
	if (platStatus) {
		return false;
	}
	return true;
	#else
	return false;
	#endif
}

void SiiDrvCraWrReg8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t val )
{
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);
	SiiPlatformStatus_t platStat;
	uint8_t i;
	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformWrite(pObj->pConfig->pInterfaceInfo, 0x40, addr, &val, 1);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
}

uint8_t SiiDrvCraRdReg8(SiiInst_t inst, SiiDrvCraAddr_t addr )
{
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);
	SiiPlatformStatus_t platStat;
	uint8_t i;
	uint8_t retVal = 0;

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformRead(pObj->pConfig->pInterfaceInfo, 0x40, addr, &retVal, 1);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return retVal;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}

	return 1;
}

void SiiDrvCraSetBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask )
{
	uint8_t val;
	val = SiiDrvCraRdReg8(inst, addr) ;
	val = (val & ((uint8_t)~mask)) | mask;
	SiiDrvCraWrReg8(inst, addr, val);
}

void SiiDrvCraClrBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask )
{
	uint8_t val;
	val = SiiDrvCraRdReg8(inst, addr) ;
	val = (val & ((uint8_t)~mask));
	SiiDrvCraWrReg8(inst, addr, val);
}

void SiiDrvCraPutBit8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t mask, uint8_t val )
{
	uint8_t temp;
	temp = SiiDrvCraRdReg8(inst, addr);
	temp &= (~mask);
	temp |= (mask & val);
	SiiDrvCraWrReg8(inst, addr, temp);
}

void SiiDrvCraWrReg16(SiiInst_t inst, SiiDrvCraAddr_t addr, uint16_t val )
{
	uint8_t writeVal;
	writeVal = val & 0xff;
	SiiDrvCraWrReg8(inst, addr, writeVal);

	writeVal = (val >> 8) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 1), writeVal);
}

void SiiDrvCraWrReg24(SiiInst_t inst, SiiDrvCraAddr_t addr, uint32_t val )
{
	uint8_t writeVal;
	writeVal = val & 0xff;
	SiiDrvCraWrReg8(inst, addr, writeVal);

	writeVal = (val >> 8) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 1), writeVal);

	writeVal = (val >> 16) & 0xff;;
	SiiDrvCraWrReg8(inst, (addr + 2), writeVal);
}

void SiiDrvCraFifoRead8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	uint16_t i;
	for (i = 0; i < size; i++) {
		*pData = SiiDrvCraRdReg8(inst, addr);
		pData++;
	}
}

void SiiDrvCraBlockWrite8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);

	uint16_t i;
	SiiPlatformStatus_t platStat;
	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformWrite(pObj->pConfig->pInterfaceInfo, 0x40, addr, pData, size);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
}

void SiiDrvCraBlockRead8(SiiInst_t inst, SiiDrvCraAddr_t addr, uint8_t* pData, SiiDrvCraSize_t size )
{
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);
	SiiPlatformStatus_t platStat;
	uint8_t i;

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformRead(pObj->pConfig->pInterfaceInfo, 0x40, addr, pData, size);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
}

bool_t SiiDCardRegBlockRead(SiiInst_t inst, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
	CraObj_t *pObj = (CraObj_t *)SII_LIB_OBJ_PNTR(inst);
	uint16_t i;

	SiiPlatformInterface_t *pebbelsInterfaceinfo;
	pebbelsInterfaceinfo = (SiiPlatformInterface_t*)SiiLibMallocCreate(sizeof(SiiPlatformInterface_t));
	if ( pebbelsInterfaceinfo ) {
		memcpy(pebbelsInterfaceinfo, pObj->pConfig->pInterfaceInfo, sizeof(SiiPlatformInterface_t));
	} else {
		//SII_LIB_LOG_PRINT2(("SiiDCardRegBlockRead Malloc info fail\n"));
		return false;
	}
	pebbelsInterfaceinfo->interfaceType = SII_PLATFORM_TYPE__I2C;

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		if ( SII_PLATFORM_STATUS__SUCCESS == SiiPlatformRead(pebbelsInterfaceinfo, deviceId, regAddr + i, &pBuffer[i], count)) {
			SiiLibMallocDelete(pebbelsInterfaceinfo);
			return true;
		}
	}

	SiiLibMallocDelete(pebbelsInterfaceinfo);
	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}

	return false;
}

bool_t SiiDCardRegBlockWrite(SiiInst_t inst, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
	CraObj_t *pObj = (CraObj_t *)SII_LIB_OBJ_PNTR(inst);

	uint16_t i;

	SiiPlatformInterface_t *pebbelsInterfaceinfo;
	pebbelsInterfaceinfo = (SiiPlatformInterface_t*)SiiLibMallocCreate(sizeof(SiiPlatformInterface_t));
	if ( pebbelsInterfaceinfo ) {
		memcpy(pebbelsInterfaceinfo, pObj->pConfig->pInterfaceInfo, sizeof(SiiPlatformInterface_t));
	} else {
		//SII_LIB_LOG_PRINT2(("SiiDCardRegBlockWrite Malloc info fail\n"));
		return false;
	}
	pebbelsInterfaceinfo->interfaceType = SII_PLATFORM_TYPE__I2C;

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		if ( SII_PLATFORM_STATUS__SUCCESS == SiiPlatformWrite(pebbelsInterfaceinfo, deviceId, regAddr, pBuffer, count)) {
			SiiLibMallocDelete(pebbelsInterfaceinfo);
			return true;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}

	SiiLibMallocDelete(pebbelsInterfaceinfo);
	return false;
}

bool_t SiiDrvCraIsInterruptRcvd(SiiInst_t inst)
{
	#if 0 //SIMG_PLATFORM
	SiiPlatformGPIOLevel_t intrPinLevel;
	CraObj_t *pObj = (CraObj_t *) SII_LIB_OBJ_PNTR(inst);
	intrPinLevel = SiiPlatformGPIOStatusGet(pObj->pConfig->pInterfaceInfo, pObj->pConfig->intrPin);
	return !intrPinLevel ? true : false;
	#else
	//inst = inst;
	return true;
	#endif
}

void    SiiDrvCraWrReg32(SiiInst_t inst, SiiDrvCraAddr_t addr, uint32_t val )
{
	#if 0
	writeVal = val & 0xff;
	SiiDrvCraWrReg8(inst, addr, writeVal);

	writeVal = (val >> 8) & 0xff;
	SiiDrvCraWrReg8(inst, (addr + 1), writeVal);

	writeVal = (val >> 16) & 0xff;
	SiiDrvCraWrReg8(inst, (addr + 2), writeVal);

	writeVal = (val >> 24) & 0xff;
	SiiDrvCraWrReg8(inst, (addr + 3), writeVal);
	#else
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);
	SiiPlatformStatus_t platStat;
	uint8_t i;
	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformWrite(pObj->pConfig->pInterfaceInfo, 0x11, addr, (uint8_t *)(&val), 1);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
	#endif
}

uint32_t    SiiDrvCraRdReg32(SiiInst_t inst, SiiDrvCraAddr_t addr )
{
	uint32_t readVal = 0;
	#if 0

	readVal = (uint32_t)SiiDrvCraRdReg8(inst, addr);
	readVal |= ((uint32_t)SiiDrvCraRdReg8(inst, addr + 1)) << 8;
	readVal |= ((uint32_t)SiiDrvCraRdReg8(inst, addr + 2)) << 16;
	readVal |= ((uint32_t)SiiDrvCraRdReg8(inst, addr + 3)) << 24;
	#else
	CraObj_t  *pObj = (CraObj_t  *) SII_LIB_OBJ_PNTR(inst);
	SiiPlatformStatus_t platStat;
	uint8_t i;

	for (i = 0; i < CRA_RETRY_COUNT; i++) {
		platStat = SiiPlatformRead(pObj->pConfig->pInterfaceInfo, 0x11, addr, (uint8_t *)(&readVal), 1);
		if (platStat == SII_PLATFORM_STATUS__SUCCESS) {
			return readVal;
		}
	}

	if (pObj->pConfig->callBack) {
		pObj->pConfig->callBack();
	}
	#endif

	return readVal;
}

/** END of File *********************************************************/
