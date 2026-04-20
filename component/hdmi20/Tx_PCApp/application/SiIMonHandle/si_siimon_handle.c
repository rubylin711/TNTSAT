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
* @brief Si_siimon_handle - SiIMon Command Handling.
*
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "si_lib_obj_api.h"
#include "si_lib_seq_api.h"
#include "si_drv_cra_api.h"
#if (MT_SDK_COMPILE_HDMI20 == 0)
	#include <Windows.h>
#endif
//#include "si_drv_platform_api.h"

SII_LIB_OBJ_MODULE_DEF(siimon_handler);

LPCWSTR comPort;
OVERLAPPED ovRead;
OVERLAPPED ovWrite;
DWORD dwEventMask;
DWORD dwBytesRead;
DWORD timeout;
DCB dcb;
COMMTIMEOUTS cto = { MAXDWORD, 0, 0, 0, 0 };

typedef struct SiIMonHandler {
	MT_HANDLE_T hComm;
	bool_t isSiIMonHandlingEnabled;
	SiiInst_t instSiIMonHandler;
	SiiInst_t instTxCra;
} SiIMonHandler_t;

static void sSiIMonHandler(SiiInst_t inst);

SiiInst_t SiIMonHandleCreate(SiiInst_t inst)
{
	SiIMonHandler_t *pSiimonHandler;
	memset(&ovRead, 0, sizeof(OVERLAPPED));
	memset(&ovWrite, 0, sizeof(OVERLAPPED));
	ovRead.hEvent = CreateEvent( 0, true, 0, 0);
	ovWrite.hEvent = CreateEvent( 0, true, 0, 0);

	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = 19200;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.ByteSize = 8;

	pSiimonHandler = (SiIMonHandler_t*)SII_LIB_OBJ_CREATE("siimonHandle", sizeof(SiIMonHandler_t));
	pSiimonHandler->isSiIMonHandlingEnabled = false;
	pSiimonHandler->instTxCra = inst;

	pSiimonHandler->instSiIMonHandler = SII_LIB_SEQ_TIMER_CREATE("SiIMonHandler", sSiIMonHandler, SII_LIB_OBJ_INST(pSiimonHandler), 1);
	SII_PLATFORM_DEBUG_ASSERT(pSiimonHandler->instSiIMonHandler);

	return SII_LIB_OBJ_INST(pSiimonHandler);
}

void SiIMonHandleDelete(SiiInst_t inst)
{
	SiIMonHandler_t* pSiimonHandler = (SiIMonHandler_t*)SII_LIB_OBJ_PNTR(inst);

	SiiLibSeqTimerDelete(pSiimonHandler->instSiIMonHandler);
	SII_LIB_OBJ_DELETE(pSiimonHandler);
}

bool_t SiIMonHandleStart(SiiInst_t inst, LPCWSTR port)
{
	SiIMonHandler_t *pSiIMonHandler = (SiIMonHandler_t*) SII_LIB_OBJ_PNTR(inst);
	// Open COM port on which SiImon Communicates
	pSiIMonHandler->hComm = CreateFile(port        					,
									   GENERIC_READ | GENERIC_WRITE		, //access ( read and write)
									   0								,	//(share) 0:cannot share the COM port
									   0								,	//security  (None)
									   OPEN_EXISTING					,	// creation : open_existing
									   FILE_FLAG_OVERLAPPED			,	// we want overlapped operation
									   0									// no templates file for COM port...
									  );

	if (pSiIMonHandler->hComm == INVALID_HANDLE_VALUE) {
		// Error with read port, exit application
		printf("Error opening Write port!!\n");
		return false;
	}
	if ( !SetCommTimeouts(pSiIMonHandler->hComm, &cto) ) {
		printf("SetCommTimeouts for Write port Failed\n");
		return false;
	}

	if (!SetCommState(pSiIMonHandler->hComm, &dcb)) {
		printf("SetCommState for Write port Failed\n");
		return false;
	}

	SetCommMask(pSiIMonHandler->hComm, EV_RXCHAR | EV_TXEMPTY);
	WaitCommEvent(pSiIMonHandler->hComm, &dwEventMask, &ovRead);
	WaitCommEvent( pSiIMonHandler->hComm, &dwEventMask, &ovWrite );

	SiiLibSeqTimerStart(pSiIMonHandler->instSiIMonHandler, 1, 20);
	pSiIMonHandler->isSiIMonHandlingEnabled = true;
	return true;
}

bool_t SiIMonHandleStop(SiiInst_t inst)
{
	SiIMonHandler_t *pSiIMonHandler = (SiIMonHandler_t*) SII_LIB_OBJ_PNTR(inst);
	if (pSiIMonHandler->isSiIMonHandlingEnabled) {
		SiiLibSeqTimerStop(pSiIMonHandler->instSiIMonHandler);
		CloseHandle(pSiIMonHandler->hComm);
		return true;
	}
	return false;
}

static void sSiIMonHandler(SiiInst_t inst)
{
	DWORD bytesWritten;
	SiIMonHandler_t *pSiIMonHandler = (SiIMonHandler_t*) SII_LIB_OBJ_PNTR(inst);
	while ( WaitForSingleObject(ovRead.hEvent, timeout) == WAIT_OBJECT_0 ) {
		unsigned short wData[19];
		uint8_t readDataBuf[20];		// Buffer should be of sufficient size to accommodate all the characters being read from serial port
		bool_t isWriteCmd = false;
		bool_t isReadCmd = false;
		uint8_t opCode = 0;;

		do {
			memset(readDataBuf, 0, sizeof(readDataBuf));
			memset(wData, 0, sizeof(wData));
			ReadFile( pSiIMonHandler->hComm, readDataBuf, sizeof(readDataBuf), &dwBytesRead, &ovRead );

			memcpy(&opCode, &readDataBuf[1], 1);
			//printf("no:%d\n", dwBytesRead);
			switch (opCode) {
				case 0x9f: //SiIMon Read Command
					isReadCmd = true;
					break;
				case 0x1f: //SiIMon Write Command
					isWriteCmd = true;
					break;

				default:
					break;
			}// switch

			if (isReadCmd) {
				wData[0] = 0xFF;
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				wData[0] = readDataBuf[1];
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				wData[0] = readDataBuf[6];
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler

				//CraReadBlockI2c(pSiIMonHandler->instTxCra, readDataBuf[3], ((uint16_t)(readDataBuf[4]) << 8) | (((uint16_t) readDataBuf[5]) & 0xff),(uint8_t*) wData, readDataBuf[6]);
				SiiDrvCraBlockRead8(pSiIMonHandler->instTxCra, ((uint16_t)(readDataBuf[4]) << 8) | (((uint16_t) readDataBuf[5]) & 0xff), (uint8_t*) wData, readDataBuf[6]);

				//send data along with header
				WriteFile(pSiIMonHandler->hComm, wData, readDataBuf[6], &bytesWritten, &ovWrite);
				isReadCmd = false;
			}
			if (isWriteCmd) {
				//CraWriteBlockI2c(pSiIMonHandler->instTxCra, readDataBuf[3], ((uint16_t)(readDataBuf[4]) << 8) | (((uint16_t) readDataBuf[5]) & 0xff),(uint8_t*) wData, readDataBuf[6]);
				SiiDrvCraBlockWrite8(pSiIMonHandler->instTxCra, ((uint16_t)(readDataBuf[4]) << 8) | (((uint16_t) readDataBuf[5]) & 0xff), (uint8_t*) &readDataBuf[7], readDataBuf[6]);

				//send ack for write
				wData[0] = 0xFF;
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				wData[0] = readDataBuf[1];
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				wData[0] = 1;
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				wData[0] = readDataBuf[5];
				WriteFile(pSiIMonHandler->hComm, wData, 1, &bytesWritten, &ovWrite);
				ResetEvent(ovWrite.hEvent);							// Reset event handler
				isWriteCmd = false;
			}

			ResetEvent(ovRead.hEvent);							// Reset event handler
			ResetEvent(ovWrite.hEvent);							// Reset event handler
			WaitCommEvent( pSiIMonHandler->hComm, &dwEventMask, &ovWrite );	// Wait for event
			WaitCommEvent( pSiIMonHandler->hComm, &dwEventMask, &ovRead );	// Wait for event

		}//do

		while ( dwBytesRead > 0 ); // Reading the serial port only if data is available

	}//while WaitForSingleObject
}
