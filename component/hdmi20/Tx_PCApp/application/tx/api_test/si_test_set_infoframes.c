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
* @file si_test_set_infoframes.c
*
* @brief Infoframes Setting API
*
*****************************************************************************/

/***** #include statements ***************************************************/
#include <conio.h>
#include "si_lib_log_api.h"
#include "si_lib_obj_api.h"
#include "si_drv_tx_api.h"

#define SII_INFOFRAME_MAX_LEN 31

#define SII_INFOFRAME_AVI_MAX_LEN 17
#define SII_INFOFRAME_AUDIO_MAX_LEN 14
#define SII_INFOFRAME_VS_MAX_LEN 31
#define MAX_COUNT 100

typedef struct {
	uint8_t scanInfo;
	uint8_t barInfo;
	uint8_t activeFormatInfo;
	uint8_t clrSpc;
	uint8_t activeAR;
	uint8_t pictureAR;
	uint8_t colorimetry;
	uint8_t scalingInfo;
	uint8_t rgbQR;
	uint8_t extColorimetry;
	uint8_t itContent;
	uint8_t vic;
	uint8_t repetition;
	uint8_t itContentType;
	uint8_t yccQR;
} SiiAviInfo_t;
SiiAviInfo_t aviInfo;
void ManSetAVIF(uint8_t *infoframe);
void ManSetVSIF(SiiInst_t inst);
void SetAVIF(int level, SiiInst_t inst);
void SetAudioIF(SiiInst_t inst);
void SetVSIF( SiiInst_t inst);
void SetHDRIF( SiiInst_t inst);

static void SetScanInfo(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Scan Info :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Data\n	1 - Overscanned\n	2 - Underscanned\n"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '2') {
		SII_LIB_LOG_PRINT2(("\nInvalid Scan Info...returning"));
		return;
	}

	switch (key) {
		case '0':
			aviInfo.scanInfo = 0;
			break;
		case '1':
			aviInfo.scanInfo = 1;
			break;
		case '2':
			aviInfo.scanInfo = 2;
			break;
	}
}

static void SetBarInfo(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Bar Info :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Data valid\n	1 - Vertical valid\n	2 - Horizontal valid\n	3 - Vert. & Horiz. valid"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("\nInvalid Bar Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.barInfo = 0;
			break;
		case '1':
			aviInfo.barInfo = 1;
			break;
		case '2':
			aviInfo.barInfo = 2;
			break;
		case '3':
			aviInfo.barInfo = 3;
			break;
	}
}

static void SetActiveFormatInfo(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Active Format Info :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Active Format Info\n	1 - Active Format Info Present\n"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '1') {
		SII_LIB_LOG_PRINT2(("\nInvalid AcitveFormat Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.activeFormatInfo = 0;
			break;
		case '1':
			aviInfo.activeFormatInfo = 1;
			break;
	}
}

static void SetClrSpc(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Colorspace :"));
	SII_LIB_LOG_PRINT2(("\n	0 - RGB(default)\n	1 - YC422\n	2 - YC444\n	3 - YC420"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("\nInvalid Colorspace Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.clrSpc = 0;
			break;
		case '1':
			aviInfo.clrSpc = 1;
			break;
		case '2':
			aviInfo.clrSpc = 2;
			break;
		case '3':
			aviInfo.clrSpc = 3;
			break;
	}
}

static void SetActiveAR(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Active Portion Aspect Ratio :"));
	SII_LIB_LOG_PRINT2(("\n	0 - Same as Picture\n	1 - 4:3(center)\n	2 - 16:9(center)\n	3 - 14:9(center)\n	4 - box 16:9(top)"));
	SII_LIB_LOG_PRINT2(("\n	5 - box 14:9(top)\n	6 - box > 16:9(center)\n	7 - 4:3(14:9 center)\n	8 - 16:9(14:9 center)\n	9 - 16:9(4:3 center)"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '9') {
		SII_LIB_LOG_PRINT2(("\nInvalid ActiveAR Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.activeAR = 8;
			break;
		case '1':
			aviInfo.activeAR = 9;
			break;
		case '2':
			aviInfo.activeAR = 10;
			break;
		case '3':
			aviInfo.activeAR = 11;
			break;
		case '4':
			aviInfo.activeAR = 2;
			break;
		case '5':
			aviInfo.activeAR = 3;
			break;
		case '6':
			aviInfo.activeAR = 4;
			break;
		case '7':
			aviInfo.activeAR = 13;
			break;
		case '8':
			aviInfo.activeAR = 14;
			break;
		case '9':
			aviInfo.activeAR = 15;
			break;
	}
}

static void SetPictureAR(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Picture Aspect Ratio :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Data\n	1 - 4:3\n	2 - 16:9\n"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '2') {
		SII_LIB_LOG_PRINT2(("\nInvalid PictureAR Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.pictureAR = 0;
			break;
		case '1':
			aviInfo.pictureAR = 1;
			break;
		case '2':
			aviInfo.pictureAR = 2;
			break;
	}
}

static void SetColorimetry(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Colorimetry :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Data(RGB)\n	1 - SMPTE 170M/ITU601\n	2 - ITU709\n	3 - Extended valid"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("\nInvalid Colorimetry Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.colorimetry = 0;
			break;
		case '1':
			aviInfo.colorimetry = 1;
			break;
		case '2':
			aviInfo.colorimetry = 2;
			break;
		case '3':
			aviInfo.colorimetry = 3;
			break;
	}
}

static void SetScaling(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Scaling :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Known Scaling\n	1 - Horizontally Scaled\n	2 - Vertically Scaled\n	3 - Vert. & Horiz. Scaled"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("\nInvalid Scaling Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.scalingInfo = 0;
			break;
		case '1':
			aviInfo.scalingInfo = 1;
			break;
		case '2':
			aviInfo.scalingInfo = 2;
			break;
		case '3':
			aviInfo.scalingInfo = 3;
			break;
	}
}

static void SetRgbQr(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet RGB Quantization Range :"));
	SII_LIB_LOG_PRINT2(("\n	0 - Default\n	1 - Limited Range\n	2 - Full Range\n"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '2') {
		SII_LIB_LOG_PRINT2(("\nInvalid RGB Quantization Range...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.rgbQR = 0;
			break;
		case '1':
			aviInfo.rgbQR = 1;
			break;
		case '2':
			aviInfo.rgbQR = 2;
			break;
	}
}

static void SetExtColorimetry(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet Extended Colorimetry :"));
	SII_LIB_LOG_PRINT2(("\n	0 - xvYCC601\n	1 - xvYCC709\n	2 - sYCC601\n	3 - AdobeYCC601\n	4 - AdobeRGB"));
	SII_LIB_LOG_PRINT2(("\n	5 - ITU-R BT2020 (const luminous)\n	6 - ITU-R BT2020 (non const luminous)"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '5') {
		SII_LIB_LOG_PRINT2(("\nInvalid Extended Colorimetry Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.extColorimetry = 0;
			break;
		case '1':
			aviInfo.extColorimetry = 1;
			break;
		case '2':
			aviInfo.extColorimetry = 2;
			break;
		case '3':
			aviInfo.extColorimetry = 3;
			break;
		case '4':
			aviInfo.extColorimetry = 4;
			break;
		case '5':
			aviInfo.extColorimetry = 5;
			break;
		case '6':
			aviInfo.extColorimetry = 6;
			break;
	}
}

static void SetITContent(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet IT Content :"));
	SII_LIB_LOG_PRINT2(("\n	0 - No Data\n	1 - ITContentType Bits valid\n"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '1') {
		SII_LIB_LOG_PRINT2(("\nInvalid IT Content Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.itContent = 0;
			break;
		case '1':
			aviInfo.itContent = 1;
			break;
	}
}

static void SetVIC(void)
{
	char key[5];
	uint8_t vic;

	SII_LIB_LOG_PRINT2(("\nSet vic :"));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	vic = (uint8_t)atoi(key);
	SII_LIB_LOG_PRINT2(("\nSet vic  done:"));
	if (vic > 110) {
		SII_LIB_LOG_PRINT2(("\nInvalid VIC Info...returning"));
		return;
	}
	//if( (key >= 93 && key <= 95) || key == 98)
	//key = 0;//fill vic in VSIF
	aviInfo.vic = vic;
}

static void SetPixRep(void)
{
	char key[5];
	uint8_t vic;

	SII_LIB_LOG_PRINT2(("\nSet repeat number :"));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	vic = (uint8_t)atoi(key);
	SII_LIB_LOG_PRINT2(("\nSet Repeation done:"));
	if (vic > 15) {
		SII_LIB_LOG_PRINT2(("\nInvalid repeation Info...returning"));
		return;
	}
	aviInfo.repetition = vic;
}

static void SetITContentType(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet IT Content Type :"));
	SII_LIB_LOG_PRINT2(("\n	0 - Graphics\n	1 - Photo\n	2 - Cinema\n	3 - Game"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("\nInvalid ITContentType Info...returning"));
		return;
	}

	aviInfo.itContentType = key - '0';
	switch (key) {
		case '0':
			break;
		case '1':
			break;
		case '2':
			break;
		case '3':
			break;
	}
}

static void SetYccQr(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("\nSet YCC Quantization Range :"));
	SII_LIB_LOG_PRINT2(("\n	0 - Limited Range\n	1 - Full Range"));
	while (!_kbhit()) {;}
	key = (char)_getch();
	if (key < '0' || key > '1') {
		SII_LIB_LOG_PRINT2(("\nInvalid YCC Quantization Range Info...returning"));
		return;
	}
	switch (key) {
		case '0':
			aviInfo.yccQR = 0;
			break;
		case '1':
			aviInfo.yccQR = 1;
			break;
	}
}

void ManSetAVIF(uint8_t *infoframe)
{
	uint8_t i;
	uint8_t cksum = 0;

	SetClrSpc();//[7:5]
	SetActiveFormatInfo();//[4]
	SetBarInfo();//[3:2]
	SetScanInfo();//[1:0]

	SetColorimetry();//[7:6]
	SetPictureAR();//[5:4]
	SetActiveAR();//[3:0]

	SetITContent();//[7]
	SetExtColorimetry();//[6:4]
	SetRgbQr();//[3:2]
	SetScaling();//[1:0]

	SetVIC();//[7:0]

	SetYccQr();//[7:6]
	SetITContentType();//[5:4]
	SetPixRep();//[3:0]

	infoframe[3] = 0;

	infoframe[4] &= ~(0x7 << 5);
	infoframe[4] |= (aviInfo.clrSpc << 5);
	infoframe[4] &= ~(0x1 << 4);
	infoframe[4] |= (aviInfo.activeFormatInfo << 4);
	infoframe[4] &= ~(0x3 << 2);
	infoframe[4] |= (aviInfo.barInfo << 2);
	infoframe[4] &= ~(0x3 << 0);
	infoframe[4] |= (aviInfo.scanInfo << 0);

	infoframe[5] &= ~(0x3 << 6);
	infoframe[5] |= (aviInfo.colorimetry << 6);
	infoframe[5] &= ~(0x3 << 4);
	infoframe[5] |= (aviInfo.pictureAR << 4);
	infoframe[5] &= ~(0xf << 0);
	infoframe[5] |= (aviInfo.activeAR << 0);

	infoframe[6] &= ~(0x1 << 7);
	infoframe[6] |= (aviInfo.itContent << 7);
	infoframe[6] &= ~(0x7 << 4);
	infoframe[6] |= (aviInfo.extColorimetry << 4);
	infoframe[6] &= ~(0x3 << 2);
	infoframe[6] |= (aviInfo.rgbQR << 2);
	infoframe[6] &= ~(0x3 << 0);
	infoframe[6] |= (aviInfo.scalingInfo << 0);

	if ((aviInfo.vic >= 93 && aviInfo.vic <= 95) || aviInfo.vic == 98) {
		infoframe[7] = 0;
	} else {
		infoframe[7] = aviInfo.vic;
	}

	infoframe[8] &= ~(0x3 << 6);
	infoframe[8] |= (aviInfo.yccQR << 6);
	infoframe[8] &= ~(0x3 << 4);
	infoframe[8] |= (aviInfo.itContentType << 4);
	infoframe[8] &= ~(0xf << 0);
	infoframe[8] |= (aviInfo.repetition << 0);

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;
}

void ManSetVSIF(SiiInst_t inst)
{
	uint8_t vic, i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t;
	uint8_t *infoframe;
	char key[5];

	memset(&infoframe, 0, SII_INFOFRAME_MAX_LEN);
	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81;
	infoframe[1] = 0x01;
	infoframe[2] = 0x05;
	infoframe[3] = 0;

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;
	infoframe[7] = 0x20;//[7:5] = 0x1

	SII_LIB_LOG_PRINT2(("\nSet VSIF vic, only 1:3840x2160@30/29.97,2:3840x2160@p25,3:3840x2160@p24/23.976,4:4096x2160@p24 is valid :"));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	vic = (uint8_t)atoi(key);
	if (vic < 93 || (vic > 95 && vic != 98)) {
		SII_LIB_LOG_PRINT2(("\nInvalid VSIF VIC...returning"));
		vic = 0;
		return;
	}
	infoframe[8] = vic;

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(inst, &infoframe_t);
}

void SetAVIF(int level, SiiInst_t inst)
{
	char key[5];
	char buf[MAX_COUNT];
	uint8_t vic, i = 0;
	uint8_t avi[SII_INFOFRAME_AVI_MAX_LEN + 1];
	FILE *fin = NULL;
	SiiInfoFrame_t pInfoFrame;
	//uint8_t man_info = 0;
	//level = level;

	fopen_s(&fin, "infoframes/video_list.txt", "r");
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No Video Timing List File available... returning\n"));
		return;
	}
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Set AVI Infoframes for :\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	while (fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("	%d - %s", i++, buf));
	}
	SII_LIB_LOG_PRINT2(("	: "));
	fclose(fin);

	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	vic = (uint8_t)atoi(key);

	fopen_s(&fin, "infoframes/avi_infoframes.txt", "r");
	//If no file, return
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No AVI Infoframes File available... returning\n"));
		return;
	}

	//Based on the vic input, parse through the file and goto required line
	for (i = 0; i < vic; i++) {
		if (!fgets(buf, MAX_COUNT, fin)) {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
			return;
		}
	}
	//Now read the required infoframes line
	if (!fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
		return;
	}
	fclose(fin);

	for (i = 0; i < SII_INFOFRAME_AVI_MAX_LEN; i++) {
		sscanf_s(buf + i * 3, "%hx", (short unsigned int *)&avi[i]);
	}

	SII_MEMSET(&pInfoFrame, 0, sizeof(pInfoFrame));
	pInfoFrame.ifId = SII_INFO_FRAME_ID__AVI;
	SII_MEMCPY((uint8_t*)&pInfoFrame.b, avi, SII_INFOFRAME_AVI_MAX_LEN);
	SII_LIB_LOG_PRINT2(("\n Decide if manual set avinfoframe : 0-use fileinput only 1-manual set again\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	SII_LIB_LOG_PRINT2(("\n Decide if manual set avinfoframe : 0-use fileinput only 1-manual set again\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	key[0] = (char)_getch();
	//gets_s(key, sizeof(key));
	//man_info = (uint8_t)atoi(key[0]);
	if (key[0] == '1') {
		ManSetAVIF(&(pInfoFrame.b[0]));
		if ((aviInfo.vic >= 93 && aviInfo.vic <= 95) || aviInfo.vic == 98) {
			ManSetVSIF(inst);
		}
	}
	SiiDrvTxInfoframeSet(inst, &pInfoFrame);

	#if 0
	char key;
	switch (level) {
		case 1:
			do {
				SII_LIB_LOG_PRINT2(("\nSet AVI Infoframes :"));
				SII_LIB_LOG_PRINT2(("\n0 - Scan Info\n1 - Bar Info\n2 - Active Format Info\n3 - RGB/YCbCr\n4 - Active Aspect Ratio\n5 - Picture Aspect Ratio"));
				SII_LIB_LOG_PRINT2(("\n6 - Colorimetry\n7 - Scaling Info\n8 - RGB Quantization Range\n9 - More Infoframes\nx- Exit"));

				while (!_kbhit()) {;}
				key = (char)_getch();

				switch (key) {
					case '0':
						SetScanInfo();
						break;
					case '1':
						SetBarInfo();
						break;
					case '2':
						SetActiveFormatInfo();
						break;
					case '3':
						SetClrSpc();
						break;
					case '4':
						SetActiveAR();
						break;
					case '5':
						SetPictureAR();
						break;
					case '6':
						SetColorimetry();
						break;
					case '7':
						SetScaling();
						break;
					case '8':
						SetRgbQr();
						break;
					case '9':
						SetAVIF(2, inst);
						break;
					case 'x':
						return;
					default:
						break;
				}
			} while (1);
			break;
		case 2:
			do {
				SII_LIB_LOG_PRINT2(("\n0 - Extended Colorimetry\n1 - IT Content\n2 - VIC Info\n3 - Repetition\n4 -IT Content Type\n5 - YCC Quantization Range\nx - Exit"));
				while (!_kbhit()) {;}
				key = (char)_getch();

				switch (key) {
					case '0':
						SetExtColorimetry();
						break;
					case '1':
						SetITContent();
						break;
					case '2':
						SetVIC();
						break;
					case '3':
						SetPixRep();
						break;
					case '4':
						SetITContentType();
						break;
					case '5':
						SetYccQr();
						break;
					case 'x':
						return;
					default:
						break;
				}
			} while (1);
			break;
	}
	#endif
}

void SetAudioIF(SiiInst_t inst)
{
	char key[5];
	char buf[100];
	uint8_t i = 0, layout;
	FILE *fin = NULL;
	uint8_t audio[SII_INFOFRAME_AUDIO_MAX_LEN + 1];
	SiiInfoFrame_t pInfoFrame;

	fopen_s(&fin, "infoframes/audio_list.txt", "r");
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No Audio List File available... returning\n"));
		return;
	}
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Set Audio Infoframes for : \n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	while (fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("	%d - %s", i++, buf));
	}
	SII_LIB_LOG_PRINT2(("	: "));
	fclose(fin);

	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	layout = (uint8_t)atoi(key);

	fopen_s(&fin, "infoframes/audio_infoframes.txt", "r");
	//If no file, return
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No Audio Infoframes File available... returning\n"));
		return;
	}
	//Based on the input, parse through the file and goto required line
	for (i = 0; i < layout; i++) {
		if (!fgets(buf, MAX_COUNT, fin)) {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
			return;
		}
	}
	//Now read the required infoframes line
	if (!fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
		return;
	}
	fclose(fin);

	for (i = 0; i < SII_INFOFRAME_AUDIO_MAX_LEN; i++) {
		sscanf_s(buf + i * 3, "%hx", (short unsigned int *)&audio[i]);
	}

	SII_MEMSET(&pInfoFrame, 0, sizeof(pInfoFrame));
	pInfoFrame.ifId = SII_INFO_FRAME_ID__AUDIO;
	SII_MEMCPY((uint8_t*)&pInfoFrame.b, audio, SII_INFOFRAME_AUDIO_MAX_LEN);
	SiiDrvTxInfoframeSet(inst, &pInfoFrame);
}

void SetVSIF( SiiInst_t inst)
{
	char key[5];
	char buf[MAX_COUNT];
	uint8_t vic, i = 0;
	uint8_t vsif[SII_INFOFRAME_VS_MAX_LEN + 1];
	FILE *fin = NULL;
	SiiInfoFrame_t pInfoFrame;

	fopen_s(&fin, "infoframes/vsif_list.txt", "r");
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No Vendor Specific IF List File available... returning\n"));
		return;
	}
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Set VSIF Infoframes for :\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	while (fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("	%d - %s", i++, buf));
	}
	SII_LIB_LOG_PRINT2(("	: "));
	fclose(fin);

	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	vic = (uint8_t)atoi(key);

	fopen_s(&fin, "infoframes/vsif_infoframes.txt", "r");
	//If no file, return
	if (!fin) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("No VSIF Infoframes File available... returning\n"));
		return;
	}

	//Based on the vic input, parse through the file and goto required line
	for (i = 0; i < vic; i++) {
		if (!fgets(buf, MAX_COUNT, fin)) {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
			return;
		}
	}
	//Now read the required infoframes line
	if (!fgets(buf, MAX_COUNT, fin)) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
		return;
	}
	fclose(fin);

	for (i = 0; i < SII_INFOFRAME_VS_MAX_LEN; i++) {
		sscanf_s(buf + i * 3, "%hx", (short unsigned int *)&vsif[i]);
	}

	SII_MEMSET(&pInfoFrame, 0, sizeof(pInfoFrame));
	pInfoFrame.ifId = SII_INFO_FRAME_ID__VS;
	SII_MEMCPY((uint8_t*)&pInfoFrame.b, vsif, SII_INFOFRAME_VS_MAX_LEN);
	SiiDrvTxInfoframeSet(inst, &pInfoFrame);
}

//only for test
void SetHDRIF( SiiInst_t inst)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;
	char key;

	SII_LIB_LOG_PRINT2((" 0:  HDR/HLG OFF, 1: HDR  2:HLG  3:Zero\n"));
	key = (char)_getch();
	key = key - '0';
	infoframe = infoframe_t.b;

	switch ( key ) {
		case 1:
			infoframe[0] = 0x87; //packet type
			infoframe[1] = 0x01; //version
			infoframe[2] = 0x1a; //length
			infoframe[3] = 0x00; //cksum
			infoframe[4] = 0x02; //HDR
			infoframe[5] = 0x00; //static metadata type 1
			break;
		case 2:
			infoframe[0] = 0x87; //packet type
			infoframe[1] = 0x01; //version
			infoframe[2] = 0x1a; //length
			infoframe[3] = 0x00; //cksum
			infoframe[4] = 0x03; //HLG
			infoframe[5] = 0x00; //static metadata type 1
			break;
		case 3:
			infoframe[0] = 0x87; //packet type
			infoframe[1] = 0x01; //version
			infoframe[2] = 0x1a; //length
			infoframe[3] = 0x00; //cksum
			break;
		case 0:
		default:
			break;
	}

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__HDR;
	SiiDrvTxInfoframeSet(inst, &infoframe_t);
}

