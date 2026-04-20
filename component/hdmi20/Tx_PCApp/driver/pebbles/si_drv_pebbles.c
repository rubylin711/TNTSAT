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

#include "si_datatypes.h"
#include "si_drv_pebbles_api.h"
//#include "si_drv_platform_api.h"
#include "sii_time.h"
#if (MT_SDK_COMPILE_HDMI20 == 0)
	#include <Windows.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include "si_drv_cra_api.h"

void SiiPebblesInfoframeGet(SiiInst_t inst, SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame)
{
	int i;
	uint8_t checkSum = 0;

	pInfoFrame->ifId = ifId;

	switch (ifId) {
		case SII_INFO_FRAME_ID__AVI    :

			for (i = 0; i < SII_INFO_FRAME_LEN__AVI; i++) {
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__AVI+i), &pInfoFrame->b[i], 1);
				SiiDCardRegBlockRead(inst, DEVICE_ID_PEBBLES, (uint16_t)(SII_INFO_FRAME_OFFSET__AVI + i), &pInfoFrame->b[i], 1);
				if (i != 3) {
					checkSum += pInfoFrame->b[i];
				}

				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			checkSum = 0xff - checkSum;
			checkSum++;
			printf("\nComp Checksum -- %2x", checkSum);

			break;

		case SII_INFO_FRAME_ID__AUDIO  :

			for (i = 0; i < SII_INFO_FRAME_LEN__AUDIO; i++) {
				SiiDCardRegBlockRead(inst, DEVICE_ID_PEBBLES, (uint16_t)(SII_INFO_FRAME_OFFSET__AUDIO + i), &pInfoFrame->b[i], 1);
				if (i == 4) {
					pInfoFrame->b[i] = 0x01;
				}
				if (i == 7) {
					pInfoFrame->b[i] = 0x00;
				}

				if (i != 3) {
					checkSum += pInfoFrame->b[i];
				}

				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}
			checkSum = 0xff - checkSum;
			checkSum++;
			pInfoFrame->b[3] = checkSum;
			printf("\nComp Checksum -- %2x", checkSum);

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

			for (i = 0; i < SII_INFO_FRAME_LEN__VS; i++) {
				pInfoFrame->b[i] = 0; // TODO: remove this and uncomment below line after finding the VSIF correct address
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__VS+i), &pInfoFrame->b[i], 1);
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			break;
		case SII_INFO_FRAME_ID__SPD    :

			for (i = 0; i < SII_INFO_FRAME_LEN__SPD; i++) {
				SiiDCardRegBlockRead(inst, DEVICE_ID_PEBBLES, (uint16_t)(SII_INFO_FRAME_OFFSET__SPD + i), &pInfoFrame->b[i], 1);
				if (i != 3) {
					checkSum += pInfoFrame->b[i];
				}
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}
			checkSum = 0xff - checkSum;
			checkSum++;
			printf("\nComp Checksum -- %2x", checkSum);

			break;
		case SII_INFO_FRAME_ID__GBD    :

			for (i = 0; i < SII_INFO_FRAME_LEN__GBD; i++) {
				pInfoFrame->b[i] = 0; // TODO: remove this and uncomment below line after finding the VSIF correct address
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__GBD+i), &pInfoFrame->b[i], 1);
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			break;
		case SII_INFO_FRAME_ID__MPEG   :

			for (i = 0; i < SII_INFO_FRAME_LEN__MPEG; i++) {
				SiiDCardRegBlockRead(inst, DEVICE_ID_PEBBLES, (uint16_t)(SII_INFO_FRAME_OFFSET__MPEG + i), &pInfoFrame->b[i], 1);
				if (i != 3) {
					checkSum += pInfoFrame->b[i];
				}
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}
			checkSum = 0xff - checkSum;
			checkSum++;
			printf("\nComp Checksum -- %2x", checkSum);

			break;
		case SII_INFO_FRAME_ID__ISRC   :

			for (i = 0; i < SII_INFO_FRAME_LEN__ISRC; i++) {
				pInfoFrame->b[i] = 0; // TODO: remove this and uncomment below line after finding the VSIF correct address
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__ISRC+i), &pInfoFrame->b[i], 1);
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			break;
		case SII_INFO_FRAME_ID__ISRC2  :

			for (i = 0; i < SII_INFO_FRAME_LEN__ISRC2; i++) {
				pInfoFrame->b[i] = 0; // TODO: remove this and uncomment below line after finding the VSIF correct address
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__ISRC2+i), &pInfoFrame->b[i], 1);
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			break;
		case SII_INFO_FRAME_ID__ACP    :

			for (i = 0; i < SII_INFO_FRAME_LEN__ACP; i++) {
				SiiDCardRegBlockRead(inst, DEVICE_ID_PEBBLES, (uint16_t)(SII_INFO_FRAME_OFFSET__ACP + i), &pInfoFrame->b[i], 1);
				if (i != 3) {
					checkSum += pInfoFrame->b[i];
				}
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}
			checkSum = 0xff - checkSum;
			checkSum++;
			printf("\nComp Checksum -- %2x", checkSum);

			break;
		case SII_INFO_FRAME_ID__HDR     :

			for (i = 0; i < SII_INFO_FRAME_LEN__HDR; i++) {
				pInfoFrame->b[i] = 0; // TODO: remove this and uncomment below line after finding the VSIF correct address
				//CraReadBlockI2c(0, DEVICE_ID_PEBBLES, (SII_INFO_FRAME_OFFSET__VS+i), &pInfoFrame->b[i], 1);
				printf("\n%d -- %2x", i, pInfoFrame->b[i]);
			}

			break;
		default :
			break;
	}

}
/*
bool_t SiiDCardRegBlockRead(uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
uint16_t i;
bool_t status = true;

for (i=0; i<count; i++)
{
if( SII_OS_STATUS_SUCCESS != CraReadBlockI2c(0, deviceId, regAddr+i, &pBuffer[i], 1));
{
status = false;
break;
}
}

return status;
}

bool_t SiiDCardRegBlockWrite(uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count)
{
uint16_t i;
bool_t status = true;

for (i=0; i<count; i++)
{
if( SII_OS_STATUS_SUCCESS != CraWriteBlockI2c(0,deviceId, regAddr+i, &pBuffer[i], 1))
{
status = false;
break;
}
}

return status;
}*/
