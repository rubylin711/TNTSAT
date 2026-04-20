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
* @file si_lib_edid.c
*
* @brief Edid Parser
*
*****************************************************************************/

/***** #include statements ***************************************************/

#include "si_datatypes.h"
#include "si_lib_edid_api.h"
#include "si_lib_obj_api.h"
#if (__HDMI_OS_RTOS__)
#include "../mta_hdmi/mta_hdmi.h"
#include "hdmi.h"
#endif
/***** Register Module name **************************************************/

//SII_LIB_OBJ_MODULE_DEF(lib_edid);

/***** local macro definitions ***********************************************/

#if ((defined(CONFIG_MT_FPGA) && defined (CONFIG_MT_CHIP_ETUDE2)) || (__HDMI_UBOOT__ == 1) )
	#define EDID_PARSE_FULL                       0//print edid info too slowly
#else
	#define EDID_PARSE_FULL                       1
#endif

#if (__HDMI_OS_RTOS__)
uint8_t		g_edid_print_en			= 0;
#else
uint8_t		g_edid_print_en			= 0;
#endif
#if(EDID_PARSE_FULL)
	#define EDID_PRINTF(val)                    { if (g_edid_print_en&0x1) SII_PRINTF(val); }
#else
	#define EDID_PRINTF(val)
#endif

#define EDID_BLOCKS_MAX                       2
#define SII_EDID_BLOCK_SIZE                   128

#define EDID_SPD_SIZE                         3
#define EDID_SAD_SIZE                         3
#define EDID_SVD_SIZE                         1
/***** local type definitions ************************************************/

typedef struct {
	SiiLibEdidRaw_t*      piEdidRaw;
	SiiLibEdidPar_t*      poEdidPar;
	SiiLibEdidErrCode_t   error;
	uint8_t               extensions;
} ParseData_t;

/***** local prototypes ******************************************************/

static uint8_t* sRawPointerGet(ParseData_t* p, uint8_t block, uint8_t addr);
static uint8_t  sExtentionsGet(const SiiLibEdidRaw_t* piEdidRaw);
static bool_t   sCheckSumPassed(const SiiLibEdidRaw_t* piEdidRaw, uint8_t block);
static void     sPrintRawDataBlock(const SiiLibEdidRaw_t* piEdidRaw, uint8_t block);
static void     sParseCheckSum(ParseData_t* p, uint8_t block);
static void     sParseHeader(ParseData_t* p);
static void     sParseProductInfo(ParseData_t* p);
static void     sParseEDIDVersion(ParseData_t* p);
#if SII_LIB_EDID_PAR__DISPLAY_AR
	static void     sParseBasicPars(ParseData_t* p);
#endif
static void     sParseChromaticity(ParseData_t* p);
static void     sParseEstablishedTimings(ParseData_t* p);
static void     sParseStandardTimingIDs(ParseData_t* p);
static void     sFindExtensions(ParseData_t* p);
static void     sParseBasicInfo(ParseData_t* p);
static void     sCheck861B(ParseData_t* p, uint8_t block);
static void     sParseCEAExt3Byte3(ParseData_t* p, uint8_t block);
static void     sParseSpeakerTag(ParseData_t* p, SiiLibEdidSpD_t *pSpD);
static void     sParseSpeakerTagTable(ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len);
static void     sParseSAD(ParseData_t* p, SiiLibEdidSad_t *pSad );
static void     sParseSADTable(ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len);
static void     sParseVsdbHdmi14( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len );
static void     sParseVsdbHdmiForum( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len );
static void     sParseVenderSpecificDataBlock( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len );
static void     sCEADataBlockCollection(ParseData_t* p, uint8_t block);
static void     sResetBasicInfo(ParseData_t* p);
static void     sResetExtentionInfo(ParseData_t* p);
static void     sResetParseInfo(ParseData_t* p);
static void     sParseEdid(ParseData_t* p);
static void     sBin2CecAddr(SiiLibEdidCecAddr_t* pCecAddr, uint8_t* pData);

#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	#include "mt_unf_video.h"
	#include "mt_unf_edid.h"
	#include "drv_global.h"
	#include "mt4si/linux_k/si_lib_edid_mt.h"
#endif

uint32_t g_2sqrt_n_32[32] = {
	1000000, //1.0
	1021897, //1.021897 * 1000000
	1044273, //...
	1067140, //...
	1090507,
	1114386,
	1138788,
	1163724,
	1189207,
	1215247,
	1241857,
	1269050,
	1296839,
	1325236,
	1354255,
	1383909,
	1414213,
	1445180,
	1476826,
	1509164,
	1542210,
	1575980,
	1610490,
	1645755,
	1681792,
	1718619,
	1756252,
	1794709,
	1834008,
	1874167,
	1915206,
	1957144
};

/***** local data objects ****************************************************/

/***** public functions ******************************************************/

uint8_t SiiLibEdidExtentionsGet(const SiiLibEdidRaw_t* piEdidRaw)
{
	uint8_t extentions = 0;

	if ( sCheckSumPassed(piEdidRaw, 0) ) {
		extentions = sExtentionsGet(piEdidRaw);
	}
	return extentions;
}

SiiLibEdidErrCode_t SiiLibEdidParse(SiiLibEdidPar_t* poEdidPar, const SiiLibEdidRaw_t* piEdidRaw)
{
	ParseData_t  ParseData;
	ParseData_t* p = &ParseData;

	p->poEdidPar = poEdidPar;
	p->piEdidRaw = (SiiLibEdidRaw_t*)piEdidRaw;
	sParseEdid(p);
	return p->error;
}

void SiiLibEdidPrintEn( uint8_t en)
{
	g_edid_print_en = en;
}
/***** local functions *******************************************************/

static uint8_t* sRawPointerGet(ParseData_t* p, uint8_t block, uint8_t addr)
{
	uint16_t offset   = ((uint16_t)block * SII_EDID_BLOCK_SIZE) + addr;

	return p->piEdidRaw + offset;
}

static uint8_t sExtentionsGet(const SiiLibEdidRaw_t* piEdidRaw)
{
	uint8_t ext_num = 0;
	uint8_t *bk1 = (uint8_t *)(&(piEdidRaw[SII_EDID_BLOCK_SIZE]));
	ext_num = piEdidRaw[0x7E];
	if ( ext_num ) {
		if ( (bk1[4] >> 5) == 7 && bk1[5] == 0x78 ) {
			ext_num = bk1[6];
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nEDID EEODB ext %d\n", ext_num));
			if ( ext_num > 7 ) {
				ext_num = 7;
			}
		}
	}

	return ext_num;
}

static bool_t sCheckSumPassed(const SiiLibEdidRaw_t* piEdidRaw, uint8_t block)
{
	uint8_t addr;
	uint8_t checkSum = 0;

	piEdidRaw += ((uint16_t)block * 128);

	for ( addr = 0; addr < SII_EDID_BLOCK_SIZE; addr++ ) {
		checkSum += piEdidRaw[addr];
	}

	return (0x00 == checkSum);
}

static void sPrintRawDataBlock(const SiiLibEdidRaw_t* piEdidRaw, uint8_t block)
{
	uint8_t addr;

	piEdidRaw += ((uint16_t)block * 128);

	/* Calculate checkSum */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nBlock %i...\n", block));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s :\n", "Raw data"));
	for ( addr = 0; addr < SII_EDID_BLOCK_SIZE; addr++ ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%02X%c", piEdidRaw[addr], ((addr + 1) % 16) ? ' ' : '\n'));
	}
}

static void sParseCheckSum(ParseData_t* p, uint8_t block)
{
	uint8_t* pData    = NULL;

	pData = sRawPointerGet(p, block, 0);

	sPrintRawDataBlock(p->piEdidRaw, block);

	/* EDID header */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Check Sum"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[127]));

	if ( sCheckSumPassed(p->piEdidRaw, block) ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING " (OK)\n"));
	} else {
		if (!(pData[126] != pData[127] || (pData[126] == pData[127] && pData[126] != 0 && pData[126] != 0xFF))) {
			p->error = SII_LIB_EDID_ERR_CODE__CHECKSUM;
		}
		EDID_PRINTF(( SI_LOG_LEVEL_STRING " (BAD)\n"));
	}
}

static void sParseHeader(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0);
	uint8_t  i;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nEDID Header...\n"));

	/* EDID header */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Header"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[0]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[1]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[2]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[3]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[4]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[5]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[6]));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X", pData[7]));

	if ( 0x00 != pData[0] ) {
		//p->error = SII_LIB_EDID_ERR_CODE__BAD_HEADER; //26249 EDID error
		EDID_PRINTF(( SI_LOG_LEVEL_STRING " (BAD)\n"));
		return;
	}

	for ( i = 1; i < 7; i++ ) {
		if ( pData[i] != 0xFF ) {
			p->error = SII_LIB_EDID_ERR_CODE__BAD_HEADER;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING " (BAD)\n"));
			return;
		}
	}
	EDID_PRINTF(( SI_LOG_LEVEL_STRING " (OK)\n"));
}

static uint16_t sgetWordVal( uint8_t* pData )
{
	return ( ( ((uint16_t)pData[1] << 8) & 0xFF00UL ) |
			 ( ((uint16_t)pData[0] << 0) & 0x00FFUL ) );
}

static uint32_t sgetLongVal( uint8_t* pData )
{
	return ( ((uint32_t)pData[3] << 24) & 0xFF000000UL ) |
		   ( ((uint32_t)pData[2] << 16) & 0x00FF0000UL ) |
		   ( ((uint32_t)pData[1] << 8) & 0x0000FF00UL ) |
		   ( ((uint32_t)pData[0] << 0) & 0x000000FFUL );
}

static void sParseProductInfo(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x08);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nVendor / Product Identification...\n"));

	/* ID Manufacturer Name */
	{
		char     pStr[4];

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "ID Manufacturer Name"));
		pStr[0] = ((pData[0] >> 2) & 0x1F) + 0x40;
		pStr[1] = ((pData[0] << 3) & 0x18) + ((pData[1] >> 5) & 0x07) + 0x40;
		pStr[2] = ((pData[1] >> 0) & 0x1F) + 0x40;
		pStr[3] = 0;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", pStr));
		p->poEdidPar->maninfo.man_name[0] = (uint8_t)(pStr[0]);
		p->poEdidPar->maninfo.man_name[1] = (uint8_t)(pStr[1]);
		p->poEdidPar->maninfo.man_name[2] = (uint8_t)(pStr[2]);
		p->poEdidPar->maninfo.man_name[3] = (uint8_t)(pStr[3]);
#if (__HDMI_OS_RTOS__)
		g_hdmi_edid_result.Manufacturer[2] = pStr[2];
		g_hdmi_edid_result.Manufacturer[1] = pStr[1];
		g_hdmi_edid_result.Manufacturer[0] = pStr[0];
#endif
	}

	/* ID Product Code */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "ID Product Code"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%4X\n", sgetWordVal(&pData[2])));
	p->poEdidPar->maninfo.product_code = (uint32_t)sgetWordVal(&pData[2]);

	/* ID Serial Number */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "ID Serial Number"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%8X\n", sgetLongVal(&pData[4])));
	p->poEdidPar->maninfo.serialnum = (uint32_t)sgetLongVal(&pData[4]);

	/* Week of Manufacture */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Week of Manufacture"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", pData[8]));
	p->poEdidPar->maninfo.week = (uint32_t)(pData[8]);

	/* Year of Manufacture */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Year of Manufacture"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%d\n", 1990 + pData[9]));
	p->poEdidPar->maninfo.year = (uint32_t)(1990 + pData[9]);

#if (__HDMI_OS_RTOS__)
	g_hdmi_edid_result.ProductCode = sgetWordVal(&pData[2]);
	g_hdmi_edid_result.SerialID = sgetLongVal(&pData[4]);
	g_hdmi_edid_result.ProductWeek = pData[8];
	g_hdmi_edid_result.ProductYear = 1990 + pData[9];
#endif
}

#if SII_LIB_EDID_PAR__VERSION
static void sParseEDIDVersion(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x12);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nEDID Version...\n"));

	/* Version no. */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Version no."));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", pData[0]));

	/* Revision no. */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Revision no."));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", pData[1]));

	p->poEdidPar->version = (uint16_t)pData[0] << 8 | pData[1];
#if (__HDMI_OS_RTOS__)
	g_hdmi_edid_result.Version[0] = pData[0];
	g_hdmi_edid_result.Version[1] = pData[1];
#endif
}
#endif

#if SII_LIB_EDID_PAR__DISPLAY_AR
static void sParseBasicPars(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x14);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nBasic Display Parameters/Features...\n"));

	/* Video Input Definition */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s :\n", "Video Input Definition"));

	/* Analog/Digital Signal Level */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Analog/Digital Signal Level"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x80) ? "Digital" : "Analog"));

	if ( pData[0] & 0x80 ) {
		/* DFP 1.x compliant */
		if ( pData[0] & 0x01 ) {
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        DFP 1.x compliant\n"));
		}
	} else {
		/* Signal Level Standard */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    Signal Level Standard"));
		switch ( pData[0] & 0x60 ) {
			case 0x00 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.700, 0.300 (1.000 V p-p)\n"));
				break;
			case 0x20 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.714, 0.286 (1.000 V p-p)\n"));
				break;
			case 0x40 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "1.000, 0.400 (1.400 V p-p)\n"));
				break;
			case 0x60 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.700, 0.000 (0.700 V p-p)\n"));
				break;
		}

		/* Setup */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    Setup"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x10) ? ("TRUE") : ("FALSE")));

		/* separate syncs. supported */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    separate syncs supported"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x08) ? ("TRUE") : ("FALSE")));

		/* composite sync. supported */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    composite sync. supported"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x04) ? ("TRUE") : ("FALSE")));

		/* sync. on green supported */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    sync. on green supported"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x02) ? ("TRUE") : ("FALSE")));

		/* Required Vsync serration */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "    Required Vsync serration"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", (pData[0] & 0x01) ? ("TRUE") : ("FALSE")));
	}

	{
		uint8_t horSize = pData[1];
		uint8_t verSize = pData[2];

		/* Max. Horizontal Image Size */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Max. horizontal Image Size (cm)"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02\n", horSize));

		/* Max. Vertical Image Size */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Max. vertical Image Size (cm)"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", verSize));

		p->poEdidPar->fDisplayAR    = (verSize) ? ((float)horSize / verSize) : (0.0);
	}

	/* Gamma Value */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Gamma Value"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%1.3f\n", (pData[3] + 100) / 100.0));
}
#endif

static void sParseChromaticity(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x19);
	{
		uint32_t fData;
		uint32_t tData;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nColor Characteristics Block...\n"));

		tData = ((pData[2] << 2) + ((pData[0] >> 6) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Red-x"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[3] << 2) + ((pData[0] >> 4) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Red-y"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[4] << 2) + ((pData[0] >> 2) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Green-x"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[5] << 2) + ((pData[0] >> 0) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Green-y"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[6] << 2) + ((pData[1] >> 6) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Blue-x"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[7] << 2) + ((pData[1] >> 4) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Blue-y"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[8] << 2) + ((pData[1] >> 2) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "White-x"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));

		tData = ((pData[9] << 2) + ((pData[1] >> 0) & 0x03)) * MT_FLOAT_FACTOR_1000000;
		fData = MT_FLOAT_DIV(uint64_t, tData, 1024);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "White-y"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0.%06d\n", fData));
	}
}

static void sParseEstablishedTimings(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x23);

	{
		uint8_t  i     = 0;
		uint32_t dat32 = 0;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nEstablished VESA Timings...\n"));

		/* Read Data */
		for ( i = 0; i < 3; i++ ) {
			dat32 |= ((uint32_t)pData[i]) << (8 * (3 - i));
		}

		i = 0;
		while ( dat32 ) {
			if ( dat32 & 0x80000000 ) {
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "    Established Timing "));
				switch ( i ) {
					case  0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "%-21s : ", "(IBM)"));
						break;
					case  1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "%-21s : ", "(IBM, XGA2)"));
						break;
					case  2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "%-21s : ", "(IBM, VGA)"));
						break;
					case  4 :
					case  5 :
					case  6 :
					case  7 :
					case  8 :
					case  9 :
					case 11 :
					case 12 :
					case 13 :
					case 14 :
					case 15 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "%-21s : ", "(VESA)"));
						break;
					case  3 :
					case 10 :
					case 16 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "%-21s : ", "(Apple, Mac II)"));
						break;
				}

				switch ( i ) {
					case  0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x400p,   70Hz, 9:5"));
						break;
					case  1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x400p,   88Hz, 9:5"));
						break;
					case  2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,   60Hz, 4:3"));
						break;
					case  3 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,   67Hz, 4:3"));
						break;
					case  4 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,   72Hz, 4:3"));
						break;
					case  5 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,   75Hz, 4:3"));
						break;
					case  6 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "800x600p,   56Hz, 4:3"));
						break;
					case  7 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "800x600p,   60Hz, 4:3"));
						break;
					case  8 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "800x600p,   72Hz, 4:3"));
						break;
					case  9 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "800x600p,   75Hz, 4:3"));
						break;
					case 10 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "832x624p,   75Hz, 4:3"));
						break;
					case 11 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1024x768i,  87Hz, 4:3"));
						break;
					case 12 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1024x768p,  60Hz, 4:3"));
						break;
					case 13 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1024x768p,  70Hz, 4:3"));
						break;
					case 14 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1024x768p,  75Hz, 4:3"));
						break;
					case 15 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x1024p, 75Hz, 5:4"));
						break;
					case 16 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "1152x870p,  75Hz, 4:3"));
						break;

					default :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
						break;
				}

				EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
			}
			dat32 <<= 1;
			i++;
		}
	}
}

//#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__ || __HDMI_OS_RTOS__)
#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__ )
static void sParseStandardTimingIDs(ParseData_t* p)
{
	uint8_t* pData = sRawPointerGet(p, 0, 0x26);

	{
		uint8_t  i;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nStandard VESA Timing identifications...\n"));

		for ( i = 0; i < 8; i++ ) {
			uint16_t horAW   = (pData[i * 2 + 0] + 31) * 8;  /* Horizontal active pixels */
			float    fAspRat = 0;
			float    fFrmFrq = 0;
			uint16_t verAW;

			fFrmFrq = (float)((pData[i * 2 + 1] & 0x3F) + 60.0); /* Frame rate */

			switch ( (pData[i * 2 + 1] & 0xC0) >> 6 ) {
				case 0 :
					fAspRat = (float)(16.0 / 10.0);
					break;  /* 16:10 */
				case 1 :
					fAspRat =  (float)(4.0 /  3.0);
					break;  /*  4: 3 */
				case 2 :
					fAspRat =  (float)(5.0 /  4.0);
					break;  /*  5: 4 */
				case 3 :
					fAspRat = (float)(16.0 /  9.0);
					break;  /* 16: 9 */
			}

			verAW = (uint16_t)(horAW / fAspRat);

			/* Standard Timing Identification */
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %s %-9d : ", "Standard Timing Identification", i));
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "%dx%dp@%4.2f,+/+,%4.2f\n", horAW, verAW, fFrmFrq, fAspRat));
		}
	}
}
#endif

static void sParseDetailedTimingDescriptor( ParseData_t* p, uint8_t* pData )
{
	uint32_t fVfreq_int = 0;
	uint32_t fVfreq_dec = 0;
	uint32_t pixFrq  = (((uint32_t)pData[1] << 8) + pData[0]) * 10000;
	uint16_t horAW   = (((uint16_t)pData[4] & 0xF0) << 4) + pData[2];
	uint16_t horBln  = (((uint16_t)pData[4] & 0x0F) << 8) + pData[3];
	uint16_t verAW   = (((uint16_t)pData[7] & 0xF0) << 4) + pData[5];
	uint16_t verBln  = (((uint16_t)pData[7] & 0x0F) << 8) + pData[6];
	uint16_t horSO   = (((uint16_t)pData[11] & 0xC0) << 2) + pData[8];
	uint16_t horSW   = (((uint16_t)pData[11] & 0x30) << 4) + pData[9];
	uint16_t verSO   = (((uint16_t)pData[11] & 0x0C) << 2) + (((uint16_t)pData[10] & 0xF0) >> 4);
	uint16_t verSW   = (((uint16_t)pData[11] & 0x04) << 4) + (((uint16_t)pData[10] & 0x0F) >> 0);
	//uint16_t horSiz  = (((uint16_t)pData[14]&0xF0)<<4) + pData[12];
	//uint16_t verSiz  = (((uint16_t)pData[14]&0x0F)<<8) + pData[13];
	bool_t   bIntrl  = (pData[17] & 0x80) ? (true) : (false);
	uint8_t  hVPol   = (pData[17] & 0x06) >> 1;
	uint16_t horTot  = horAW + horBln;
	uint16_t verTot  = verAW + verBln;
	if ( horTot != 0 &&  verTot != 0 ) {
		fVfreq_int  = (pixFrq) / ((uint32_t)horTot * verTot);
		fVfreq_dec = (pixFrq) % ((uint32_t)horTot * verTot);
		fVfreq_dec = (fVfreq_dec * MT_FLOAT_FACTOR_1000) / ((uint32_t)horTot * verTot);
	} else {
		fVfreq_int = pixFrq;
		fVfreq_dec = pixFrq;
		//fVfreq_dec = fVfreq_dec;
	}

	//p = p;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nDetailed Timing Descriptor...\n"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Format"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%dx%d%c@%d.%03d,%c/%c\n", horAW, (bIntrl) ? (verAW * 2) : (verAW), (bIntrl) ? 'i' : 'p', fVfreq_int, fVfreq_dec, (hVPol & 0x2) ? '+' : '-', (hVPol & 0x1) ? '+' : '-'));

	#if(SII_LIB_EDID_PAR__DTDB)
	{
		SiiLibVideoTiming_t vidTim;

		vidTim.hTotal      = horTot;
		vidTim.hActive     = horAW;
		vidTim.hFrontPorch = horSO;
		vidTim.hSync       = horSW;
		vidTim.bHpol       = (hVPol & 0x2) ? (true) : (false);

		vidTim.vTotal      = (bIntrl) ? (2 * verTot + 1) : (verTot);
		vidTim.vActive     = (bIntrl) ? (2 * verAW) : (verAW);
		vidTim.vFrontPorch = (bIntrl) ? (2 * verSO + 1) : (verSO);
		vidTim.vSync       = (bIntrl) ? (2 * verSW) : (verSW);
		vidTim.bVpol       = (hVPol & 0x1) ? (true) : (false);

		vidTim.bInterlaced = (bIntrl) ? (true) : (false);
		vidTim.pixelFreq   = pixFrq;
		vidTim.lineFreq    = (vidTim.hTotal)   ? (pixFrq / vidTim.hTotal)   : (0);
		vidTim.vFreq       = (vidTim.lineFreq) ? (pixFrq / vidTim.lineFreq) : (0);
		//vidTim.pictAr      = (verSiz) ? ((float)horSiz / verSiz) : (0.0);

		if ( SII_LIB_EDID__DTD_MAX > p->poEdidPar->dtdb.size ) {
			p->poEdidPar->dtdb.vidTim[p->poEdidPar->dtdb.size] = vidTim;
			p->poEdidPar->dtdb.size++;
		}
	}
	#endif
	#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	sParseEdidSaveFmtByDTD(pixFrq, horTot, verTot, fVfreq_int, fVfreq_dec);
	ParsePreferredTiming((MT_U8 *)pData);
	#endif
}

static void sParseDescriptorDefinedByManufacturer( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nDescriptor Defined by Manufacturer...\n"));
}

static void sParseStandardTimingIDBlock( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nStandard Timing ID Block...\n"));
}

static void sParseColorPointDataBlock( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nColor Point Data Block...\n"));
}

#if(SII_LIB_EDID_PAR__MONITOR_NAME)
static void sParseFirstMonitorDescriptorBlock( ParseData_t* p, uint8_t* pData )
{
	uint8_t i;
	char    MonitorNameStr[14];

	SII_MEMCPY(MonitorNameStr, &pData[5], 13);

	for ( i = 0; i < 13; i++ ) {
		if ( 0x0A == MonitorNameStr[i] ) {
			MonitorNameStr[i] = 0;
			break;
		}
	}

	MonitorNameStr[13] = 0;//Make sure there's string end
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nFirst Monitor Descriptor Block...\n"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Monitor Name"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%s\n", MonitorNameStr));

	SII_STRCPY(p->poEdidPar->pMonitorNameStr, MonitorNameStr);
#if (__HDMI_OS_RTOS__)
	g_hdmi_edid_result.NameLength = i;
	SII_STRCPY(g_hdmi_edid_result.MonitorNanme, MonitorNameStr);
#endif
}
#endif

static void sParseSecondMonitorDescriptorBlock( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nSecond Monitor Descriptor Block...\n"));
}

#if (__HDMI_OS_RTOS__)
static void sParseBasicDisplayParamBlock( ParseData_t* p)
{
  u8 gamma = 0;
  u8 h_size = 0;
  u8 v_size = 0;
  u8 feature = 0;
  u8 disp_mode = 0;

  EDID_PRINTF(("%s: %d\n", __FUNCTION__, __LINE__));

  h_size = sRawPointerGet(p, 0, 0x15);
  v_size = sRawPointerGet(p, 0, 0x16);
  gamma = sRawPointerGet(p, 0, 0x17);
  feature = sRawPointerGet(p, 0, 0x18);

  g_hdmi_edid_result.H_ImageSize = h_size;
  g_hdmi_edid_result.V_ImageSize = v_size;
  g_hdmi_edid_result.Gamma = gamma;
  g_hdmi_edid_result.FeatureSupportDetail = feature;

  g_hdmi_edid_result.support_standby = ((feature & 0x80) ? 0x1 : 0x0);
  g_hdmi_edid_result.support_suspend = ((feature & 0x40) ? 0x1 : 0x0);
  g_hdmi_edid_result.support_active_off = ((feature & 0x20) ? 0x1 : 0x0);
  g_hdmi_edid_result.support_sRGB = ((feature & 0x04) ? 0x1 : 0x0);
  g_hdmi_edid_result.support_GTF = ((feature & 0x01) ? 0x1 : 0x0);

  disp_mode = (feature & 0x18) >> 3;
  g_hdmi_edid_result.display_type = disp_mode;
  switch(disp_mode)
  {
    case 2:
      g_hdmi_edid_result.support_RGB_disp_mode = 0;
      g_hdmi_edid_result.support_Monochrome_disp = 0;
      g_hdmi_edid_result.support_non_RGB_disp_mode = 1;
      break;
    case 1:
      g_hdmi_edid_result.support_RGB_disp_mode = 1;
      g_hdmi_edid_result.support_Monochrome_disp = 0;
      g_hdmi_edid_result.support_non_RGB_disp_mode = 0;
      break;
    case 0:
      g_hdmi_edid_result.support_RGB_disp_mode = 0;
      g_hdmi_edid_result.support_Monochrome_disp = 1;
      g_hdmi_edid_result.support_non_RGB_disp_mode = 0;
      break;
    default:
      g_hdmi_edid_result.support_RGB_disp_mode = 0;
      g_hdmi_edid_result.support_Monochrome_disp = 0;
      g_hdmi_edid_result.support_non_RGB_disp_mode = 0;
      break;
  }

  EDID_PRINTF(("Image size     :  (H %d : V %d ) cm \n",  g_hdmi_edid_result.H_ImageSize, g_hdmi_edid_result.V_ImageSize));
  EDID_PRINTF(("Gamma          :  %d \n",  g_hdmi_edid_result.Gamma));

  EDID_PRINTF(("FeatureDetail:  %02x \n",  g_hdmi_edid_result.FeatureSupportDetail));
  EDID_PRINTF(("support_standby:  %x \n",  g_hdmi_edid_result.support_standby));
  EDID_PRINTF(("support_suspend:  %x \n",  g_hdmi_edid_result.support_suspend));
  EDID_PRINTF(("support_active_off:  %x \n",  g_hdmi_edid_result.support_active_off));
  EDID_PRINTF(("support_sRGB   :  %x \n",  g_hdmi_edid_result.support_sRGB));

  EDID_PRINTF(("support_display_mode: = %x0;  0: Monochrome, 1: RGB mode, 2: non-RGB mode \n",  disp_mode));

  EDID_PRINTF(("%s: %d end \n", __FUNCTION__, __LINE__));

  return;
}
#endif
static void sASCIIStringBlock( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nASCII String Block...\n"));
}

static void sMonitorSerialNumberBlock( ParseData_t* p, uint8_t* pData )
{
	//p     = p;
	//pData = pData;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nMonitor Serial Number Block...\n"));
}

static void sParseDetailedDescriptorBlock( ParseData_t* p, uint8_t block, uint8_t ddbAddr )
{
	uint8_t* pData = sRawPointerGet(p, block, ddbAddr);

	if ( !pData[0] && !pData[1] && !pData[4] ) {
		if ( 0x01 == pData[3] ) {
			/* Parse First Monitor Descriptor Block */
			sParseDescriptorDefinedByManufacturer(p, pData);
		} else if ( 0x10 == pData[3] ) {
			/* Dummy descriptor Block */
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nDummy descriptor Block\n"));
		} else if ( 0xFA == pData[3] ) {
			/* Parse Standard Timing ID Block */
			sParseStandardTimingIDBlock(p, pData);
		} else if ( 0xFB == pData[3] ) {
			/* Parse Color Point Data Block */
			sParseColorPointDataBlock(p, pData);
		} else if ( 0xFC == pData[3] ) {
			#if(SII_LIB_EDID_PAR__MONITOR_NAME)
			/* Parse First Monitor Descriptor Block */
			sParseFirstMonitorDescriptorBlock(p, pData);
			#endif
		} else if ( 0xFD == pData[3] ) {
			/* Parse Second Monitor Descriptor Block */
			sParseSecondMonitorDescriptorBlock(p, pData);
		} else if ( 0xEE == pData[3] ) {
			/* Parse ASCII String Block */
			sASCIIStringBlock(p, pData);
		} else if ( 0xFF == pData[3] ) {
			/* Parse Monitor Serial Number Block */
			sMonitorSerialNumberBlock(p, pData);
		}
	} else {
		/* Parse Detailed Timing Descriptor Block */
		sParseDetailedTimingDescriptor(p, pData);
	}
}

static void sFindExtensions(ParseData_t* p)
{
	p->extensions = sExtentionsGet(p->piEdidRaw);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nExtension Flag...\n"));

	/* Extension Flag */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Extensions"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d", p->extensions));

	if ( 2 == p->extensions ) {
		//p->error = SII_LIB_EDID_ERR_CODE__NOEXTENSIONS;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING " (BAD)\n"));
		//return;
	}
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
}

static void sParseBasicInfo(ParseData_t* p)
{
	uint8_t i       = 0;
	uint8_t ddbAddr = 0x36;

	/* Check EDID Header */
	sParseHeader(p);
	if ( p->error ) {
		return;
	}

	/* Check Product Information */
	sParseProductInfo(p);
	if ( p->error ) {
		return;
	}

	#if SII_LIB_EDID_PAR__VERSION
	/* Check EDID Version */
	sParseEDIDVersion(p);
	if ( p->error ) {
		return;
	}
	#endif

#if (__HDMI_OS_RTOS__)
    /* Check Basic Dsiaplay Parameters */
    sParseBasicDisplayParamBlock(p);
#endif
	#if SII_LIB_EDID_PAR__DISPLAY_AR
	/* Check Basic Dsiaplay Parameters */
	sParseBasicPars(p);
	if ( p->error ) {
		return;
	}
	#endif

	/* Parse Chromaticity Parameters */
	sParseChromaticity(p);
	if ( p->error ) {
		return;
	}

	/* Established VESA Timings */
	sParseEstablishedTimings(p);

	/* Standard VESA Timing Identification */
	sParseStandardTimingIDs(p);
	if ( p->error ) {
		return;
	}

	/* Check Detailed Decriptor Blocks */
	for ( i = 0; i < 4; i++ ) {
		sParseDetailedDescriptorBlock(p, 0, ddbAddr);
		if ( p->error ) {
			return;
		}

		ddbAddr += 18;
	}

	/* Find number of extensions */
	sFindExtensions(p);
	if ( p->error ) {
		return;
	}
}

static void sCheck861B( ParseData_t* p, uint8_t block )
{
	uint8_t* pData = sRawPointerGet(p, block, 0x00);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nCheck for 861B (block %i)...\n", block));

	/* CEA EDID timing extension tag */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "CEA EDID timing extension tag"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X\n", pData[0]));

#if (__HDMI_OS_RTOS__)
	/* if revision number equal 0x02 or 0x03 ,analysis yuv_support,
	   refrecence EIA-CEA-861-D.pdf  CEA Externsion 2 3 table */
	if(pData[1] == 0x02 || pData[1] == 0x03)
	{
	    //g_hdmi_edid_result.native_format_parsed_flag = 1;
	    if( pData[3] & 0X20 )
	    {
	        g_hdmi_edid_result.ycbcr444_supported = 1;
	    }
	    if( pData[3] & 0X10 )
	    {
	        g_hdmi_edid_result.ycbcr422_supported = 1;
	    }

	}
#endif
	if ( 2 != pData[0] ) {
		p->error = SII_LIB_EDID_ERR_CODE__CEA_TAG;
		return;
	}

	/* CEA 861B version */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "CEA 861B version"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02X\n", pData[1]));
	if ( 3 != pData[1] ) {
		p->error = SII_LIB_EDID_ERR_CODE__NO_861B;
		return;
	}
}

static void sParseCEAExt3Byte3( ParseData_t* p, uint8_t block )
{
	uint8_t* pData = sRawPointerGet(p, block, 0x03);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : %02d\n", "Number of native DTD's", pData[0] & 0x0F));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : %s\n", "Basic Underscan Support", (pData[0] & 0x80) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : %s\n", "Basic Audio Support", (pData[0] & 0x40) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : %s\n", "Basic YCbCr 444 Support", (pData[0] & 0x20) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : %s\n", "Basic YCbCr 422 Support", (pData[0] & 0x10) ? "Yes" : "No"));

	#if SII_LIB_EDID_PAR__AUD_FRM
	p->poEdidPar->bBasicAudio = (pData[0] & 0x40) ? (true) : (false);
	#endif
	p->poEdidPar->bUnderScan = (pData[0] & 0x80) ? (true) : (false);
	p->poEdidPar->Yuv444 = (pData[0] & 0x20) ? (true) : (false);
	p->poEdidPar->Yuv422 = (pData[0] & 0x10) ? (true) : (false);
}

static void sParseSpeakerTag( ParseData_t* p, SiiLibEdidSpD_t *pSpD )
{
	//p = p;

	/* Speaker allocation */
	{
		uint8_t spkCfg = pSpD->p[0];
		uint8_t i      = 0;
		bool_t  bComma = false;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Allocated Speakers"));
		p->poEdidPar->audInfo.speakerformat = spkCfg | ((pSpD->p[1]) << 8);

		while ( spkCfg ) {
			if ( spkCfg & 0x01 ) {
				if ( bComma ) {
					EDID_PRINTF(( SI_LOG_LEVEL_STRING ","));
				} else {
					bComma = true;
				}

				switch ( i ) {
					case 0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FL/FR"));
						break;
					case 1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "LFE"));
						break;
					case 2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FC"));
						break;
					case 3 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "RL/RR"));
						break;
					case 4 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "RC"));
						break;
					case 5 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FLC/FRC"));
						break;
					case 6 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "RLC/RRC"));
						break;
					case 7 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FLW/FRW"));
						break;
				}
			}
			i++;
			spkCfg >>= 1;
		}

		spkCfg = pSpD->p[1];
		while ( spkCfg ) {
			if ( spkCfg & 0x01 ) {
				if ( bComma ) {
					EDID_PRINTF(( SI_LOG_LEVEL_STRING ","));
				} else {
					bComma = true;
				}

				switch ( i ) {
					case 0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FLH/FRH"));
						break;
					case 1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "TC"));
						break;
					case 2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "FCH"));
						break;
				}
			}
			i++;
			spkCfg >>= 1;
		}
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
	}
}

static void sParseSpeakerTagTable( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nSpeaker Alloc Data Block (Length=%02d)...\n", len));

	while ( EDID_SPD_SIZE <= len ) {
		uint8_t*        pData = sRawPointerGet(p, block, addr);
		SiiLibEdidSpD_t SpD;

		/* Read Data */
		SII_MEMCPY((uint8_t*)&SpD, pData, EDID_SPD_SIZE);

		/* Print out Data */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Speaker Alloc Descriptor Block"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%02X, 0x%02X, 0x%02X", SpD.p[0], SpD.p[1], SpD.p[2]));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));

		sParseSpeakerTag(p, &SpD);
#if (__HDMI_OS_RTOS__)
		g_hdmi_edid_result.speakerformat =SpD.p[0];
#endif
		#if SII_LIB_EDID_PAR__SADB
		if ( p->poEdidPar->sadb.size < SII_LIB_EDID__SPD_MAX ) {
			p->poEdidPar->sadb.SpD[p->poEdidPar->sadb.size] = SpD;
			p->poEdidPar->sadb.size++;
		}
		#endif

		addr += EDID_SPD_SIZE;
		len  -= EDID_SPD_SIZE;
	}
}

static void sParseSVDTable( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nVideo Data Block (Length=%02d)...\n", len));

	while ( EDID_SVD_SIZE <= len ) {
		uint8_t*        pData = sRawPointerGet(p, block, addr);
		SiiLibEdidSvd_t SVD;

		/* Read Data */
		SII_MEMCPY((uint8_t*)&SVD, pData, EDID_SVD_SIZE);

		/* Print out Data */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Short Video Descriptor"));

		{
			uint8_t VID      = SVD.p[0] & 0x7F;
			bool_t  bNative  = (SVD.p[0] & 0x80) ? (true) : (false);

			#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
			sStoreFmtInfo((uint32_t)VID, bNative, false);
			#endif

			EDID_PRINTF(( SI_LOG_LEVEL_STRING "%20d-", VID));
			switch ( VID ) {
				case  0 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "No VID available"));
					break;

				case  1 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,    60Hz,  4:3"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_640x480p_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_640x480p_60Hz = bNative;
					#endif
					break;
				case  2 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,    60Hz,  4:3"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_720x480p_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720x480p_60Hz = bNative;
					#endif
					break;
				case  3 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,    60Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
                    g_hdmi_edid_result.supported_720x480p_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720x480p_60Hz = bNative;
					#endif
					break;
				case  4 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,   60Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
                    g_hdmi_edid_result.supported_720p_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720p_60Hz = bNative;
					#endif
					break;
				case  5 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  60Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_1080i_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_1080i_60Hz = bNative;
					#endif
					break;
				case  6 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,    60Hz,  4:3"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_720x480i_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720x480i_60Hz = bNative;
					#endif
					break;
				case  7 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,    60Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_720x480i_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720x480i_60Hz = bNative;
					#endif
					break;
				case  8 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x240p,    60Hz,  4:3"));
					break;
				case  9 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x240p,    60Hz, 16:9"));
					break;
				case 10 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480i,   60Hz,  4:3"));
					break;
				case 11 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480i,   60Hz, 16:9"));
					break;
				case 12 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x240p,   60Hz,  4:3"));
					break;
				case 13 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x240p,   60Hz, 16:9"));
					break;
				case 14 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x480p,   60Hz,  4:3"));
					break;
				case 15 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x480p,   60Hz, 16:9"));
					break;
				case 16 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  60Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_1080p_60Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_1080p_60Hz = bNative;
					#endif
					break;
				case 17 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,    50Hz,  4:3"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_576p_50Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_576p_50Hz = bNative;
					#endif
					break;
				case 18 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,    50Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_576p_50Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_576p_50Hz = bNative;
					#endif
					break;
				case 19 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,   50Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_720p_50Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_720p_50Hz = bNative;
					#endif
					break;
				case 20 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  50Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_1080i_50Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_1080i_50Hz =bNative;
					#endif
					break;
				case 21 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,    50Hz,  4:3"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_576i_50Hz =MT_TRUE;
					g_hdmi_edid_result.is_native_576i_50Hz = bNative;
					#endif
					break;
				case 22 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,    50Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_576i_50Hz =MT_TRUE;
					g_hdmi_edid_result.is_native_576i_50Hz = bNative;
					#endif
					break;
				case 23 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x288p,    50Hz,  4:3"));
					break;
				case 24 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x288p,    50Hz, 16:9"));
					break;
				case 25 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576i,   50Hz,  4:3"));
					break;
				case 26 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576i,   50Hz, 16:9"));
					break;
				case 27 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x288p,   50Hz,  4:3"));
					break;
				case 28 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x288p,   50Hz, 16:9"));
					break;
				case 29 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x576p,   50Hz,  4:3"));
					break;
				case 30 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x576p,   50Hz, 16:9"));
					break;
				case 31 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  50Hz, 16:9"));
					#if ( __HDMI_OS_RTOS__ )
					g_hdmi_edid_result.supported_1080p_50Hz =MT_TRUE;
                    g_hdmi_edid_result.is_native_1080p_50Hz =bNative;
					#endif
					break;
				case 32 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  24Hz, 16:9"));
					break;
				case 33 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  25Hz, 16:9"));
					break;
				case 34 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  30Hz, 16:9"));
					break;
				case 35 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480p,   60Hz,  4:3"));
					break;
				case 36 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480p,   60Hz, 16:9"));
					break;
				case 37 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576p,   50Hz,  4:3"));
					break;
				case 38 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576p,   50Hz, 16:9"));
					break;
				case 39 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  50Hz, 16:9"));
					break;
				case 40 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i, 100Hz, 16:9"));
					break;
				case 41 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  100Hz, 16:9"));
					break;
				case 42 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,   100Hz,  4:3"));
					break;
				case 43 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,   100Hz, 16:9"));
					break;
				case 44 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   100Hz,  4:3"));
					break;
				case 45 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   100Hz, 16:9"));
					break;
				case 46 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i, 120Hz, 16:9"));
					break;
				case 47 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  120Hz, 16:9"));
					break;
				case 48 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   120Hz,  4:3"));
					break;
				case 49 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   120Hz, 16:9"));
					break;
				case 50 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   120Hz,  4:3"));
					break;
				case 51 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   120Hz, 16:9"));
					break;
				case 52 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720X576p,   200Hz,  4:3"));
					break;
				case 53 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720X576p,   200Hz, 16:9"));
					break;
				case 54 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   200Hz,  4:3"));
					break;
				case 55 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   200Hz, 16:9"));
					break;
				case 56 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   240Hz,  4:3"));
					break;
				case 57 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   240Hz, 16:9"));
					break;
				case 58 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   240Hz,  4:3"));
					break;
				case 59 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   240Hz, 16:9"));
					break;
				case 60 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  24Hz,  16:9"));
					break;
				case 61 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  25Hz,  16:9"));
					break;
				case 62 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  30Hz,  16:9"));
					break;
				case 93 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 24Hz,  16:9"));
					break;
				case 94 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 25Hz,  16:9"));
					break;
				case 95 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 30Hz,  16:9"));
					break;
				case 96 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 50Hz,  16:9"));
					break;
				case 97 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 60Hz,  16:9"));
					break;
				case 98 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 24Hz,  256:135"));
					break;
				case 99 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 25Hz,  256:135"));
					break;
				case 100 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 30Hz,  256:135"));
					break;
				case 101 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 50Hz,  256:135"));
					break;
				case 102 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 60Hz,  256:135"));
					break;

				default :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
					break;
			}

			if ( bNative ) {
				EDID_PRINTF(( SI_LOG_LEVEL_STRING " (N)"));
			}

			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
		}

		#if SII_LIB_EDID_PAR__VDB
		if ( p->poEdidPar->vdb.size < SII_LIB_EDID__SVD_MAX ) {
			p->poEdidPar->vdb.svd[p->poEdidPar->vdb.size] = SVD;
			p->poEdidPar->vdb.size++;
		}
		#endif

		addr += EDID_SVD_SIZE;
		len  -= EDID_SVD_SIZE;
	}
}

static void sParseSAD( ParseData_t* p, SiiLibEdidSad_t *pSAD )
{
	uint8_t format_code, u8bit, u8Count;

	//p = p;

	u8Count = 0;
	if (p->poEdidPar->audInfo.audformat_cnt >= HDMI_AUDIO_FORMAT_MAX_NUM_EDID) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "Audio Capability count over max number %d \n", HDMI_AUDIO_FORMAT_MAX_NUM_EDID));
		return;
	}

	/* Max. Number of Channels */
	{
		uint8_t MaxNoCh = 0;
		MaxNoCh = ((pSAD->p[0] & 0x07) + 1);

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Max. Number of Channels"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", MaxNoCh));
		p->poEdidPar->audInfo.audchannel[p->poEdidPar->audInfo.audformat_cnt] = (uint8_t)MaxNoCh;
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audiochannel[p->poEdidPar->audInfo.audformat_cnt] =MaxNoCh;
#endif
	}

	/* Audio Format Code */
	{
		uint8_t FrmCode = ((pSAD->p[0] >> 3) & 0xF);
		format_code = (uint8_t)FrmCode;
		p->poEdidPar->audInfo.audformat[p->poEdidPar->audInfo.audformat_cnt] = format_code;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Audio Format Code"));

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%2d-", FrmCode));
		switch ( FrmCode ) {
			case  0 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
				break;
			case  1 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "Linear PCM"));
				break;
			case  2 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "AC3"));
				break;
			case  3 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "MPEG1 (Layers 1 & 2"));
				break;
			case  4 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "MP3 (MPEG1 Layer 2)"));
				break;
			case  5 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "MPEG2 (multichannel)"));
				break;
			case  6 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "AAC"));
				break;
			case  7 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "DTS"));
				break;
			case  8 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "ATREC"));
				break;
			case  9 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "One Bit Audio"));
				break;
			case 10 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "Dolby Digital+"));
				break;
			case 11 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "DTS-HD"));
				break;
			case 12 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "MAT (MLP)"));
				break;
			case 13 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "DST"));
				break;
			case 14 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "WMA Pro"));
				break;
			case 15 :
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
				break;
		}
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audioformat[p->poEdidPar->audInfo.audformat_cnt] =FrmCode;
#endif
	}

	/* Supported Sample Frequencies */
	{
		uint8_t SmpFrq = pSAD->p[1] & 0x7F;
		uint8_t i      = 0;
		bool_t  bComma = false;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Sample Frequencies"));
		p->poEdidPar->audInfo.audfs[p->poEdidPar->audInfo.audformat_cnt] = SmpFrq;
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audiofs[p->poEdidPar->audInfo.audformat_cnt] =SmpFrq;
#endif
		while ( SmpFrq ) {
			if ( SmpFrq & 0x01 ) {
				if ( bComma ) {
					EDID_PRINTF(( SI_LOG_LEVEL_STRING ","));
				} else {
					bComma = true;
				}

				switch ( i ) {
					case 0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "32"));
						break;
					case 1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "44"));
						break;
					case 2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "48"));
						break;
					case 3 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "88"));
						break;
					case 4 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "96"));
						break;
					case 5 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "176"));
						break;
					case 6 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "192"));
						break;
				}
			}
			i++;
			SmpFrq >>= 1;
		}
		EDID_PRINTF(( SI_LOG_LEVEL_STRING " KHz\n"));
	}

	if (1 == format_code) {
		u8Count = 0;
		u8bit = pSAD->p[2];
		//EDID_INFO("Bit Depth:0x%02x\n",u8bit);
		//EDID_PRINTF(( SI_LOG_LEVEL_STRING "Bit Depth:0x%02x\n",u8bit));
		if (u8bit & 0x01) {
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 16 Bit\n", "Bit Depth"));
			u8Count++;
		}
		if (u8bit & 0x02) {
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 20 Bit\n", "Bit Depth"));
			u8Count++;
		}
		if (u8bit & 0x04) {
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 24 Bit\n", "Bit Depth"));
			u8Count++;
		}
		//EDID_INFO("Bit Depth num:%d\n",u8Count);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %d \n", "Bit Depth num:", u8Count));
		p->poEdidPar->audInfo.audlen[p->poEdidPar->audInfo.audformat_cnt] = u8bit & 0x07;
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audiolength[p->poEdidPar->audInfo.audformat_cnt] = u8bit & 0x07;
#endif

	} else if ((format_code > 1) && (format_code < 9)) {
		p->poEdidPar->audInfo.audlen[p->poEdidPar->audInfo.audformat_cnt] = pSAD->p[2];
		//EDID_INFO("Max Bit Rate:%d\n",pstAudioInfo[cur].u32MaxBitRate);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Maximum Bit Rate"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%d Kbps\n", (mt_u32)(8 * (pSAD->p[2]))));
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audiolength[p->poEdidPar->audInfo.audformat_cnt] = (8 * (pSAD->p[2]));//sym6 should check here need 8* ?
#endif
	} else if (format_code == 12 ) {
		p->poEdidPar->audInfo.audlen[p->poEdidPar->audInfo.audformat_cnt] = pSAD->p[2];
		//EDID_INFO("Max Bit Rate:%d\n",pstAudioInfo[cur].u32MaxBitRate);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "MAT Profile"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%d\n", (mt_u32)pSAD->p[2]));
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.audiolength[p->poEdidPar->audInfo.audformat_cnt] = (mt_u32)pSAD->p[2]));//sym6 should check here need 8* ?
#endif
	} else { //9 -15 reserve
		//g_stEdidInfo.stAudInfo[format_code].u32Reserve = pData[Index*3 + 2];
	}

	p->poEdidPar->audInfo.audformat_cnt++;
}

static void sParseSADTable( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nAudio Data Block (Length=%02d)...\n", len));

	while ( EDID_SAD_SIZE <= len ) {
		uint8_t*         pData = sRawPointerGet(p, block, addr);
		SiiLibEdidSad_t  SAD;

		/* Read Data */
		SII_MEMCPY((uint8_t*)&SAD, pData, EDID_SAD_SIZE);

		/* Print out Data */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Short Audio Descriptor"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%02X, 0x%02X, 0x%02X", SAD.p[0], SAD.p[1], SAD.p[2]));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));

		sParseSAD(p, &SAD);

		#if SII_LIB_EDID_PAR__ADB
		if ( p->poEdidPar->adb.size < SII_LIB_EDID__SAD_MAX ) {
			p->poEdidPar->adb.sad[p->poEdidPar->adb.size] = SAD;
			p->poEdidPar->adb.size++;
		}
		#endif

		addr += EDID_SAD_SIZE;
		len  -= EDID_SAD_SIZE;
	}
#if ( __HDMI_OS_RTOS__ )
	g_hdmi_edid_result.audioformat_cnt = p->poEdidPar->adb.size;
#endif
}

static void sParseVsdbHdmi14( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*             pData;
	SiiLibEdidCecAddr_t  cecAddr = {{0, 0, 0, 0}};
	uint8_t              flags;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    HDMI-LLC Vendor Specific Data Block...\n"));

	#if SII_LIB_EDID_PAR__CEC_ADDR
	p->poEdidPar->cecAddrPtr = block * 128 + addr;
	#endif

	/* Read Data */
	if ( 2 > len ) {
		return;
	}
	pData = sRawPointerGet(p, block, addr);
	len  -= 2;
	addr += 2;

	sBin2CecAddr(&cecAddr, pData);

	/* Print out Data */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "CEC Address"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d.%02d.%02d.%02d\n", cecAddr.sub[0], cecAddr.sub[1], cecAddr.sub[2], cecAddr.sub[3]));

	#if SII_LIB_EDID_PAR__CEC_ADDR
	p->poEdidPar->cecAddr = cecAddr;
#if ( __HDMI_OS_RTOS__ )
	g_hdmi_edid_result.cec_phy_addr = ((cecAddr.sub[0] << 12) | (cecAddr.sub[1] << 8) | (cecAddr.sub[2] << 4) | cecAddr.sub[3]);
#endif
	#endif

	if ( len ) {
		pData = sRawPointerGet(p, block, addr);
		addr++;
		len--;

		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 0x%02X\n", "Color Depth flags", pData[0]));
		#if SII_LIB_EDID_PAR__AUD_FRM
		p->poEdidPar->bSupportsAI = (pData[0] & 0x80) ? (true) : (false);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Support AI", \
					  p->poEdidPar->bSupportsAI ? "Yes" : "No"));
		#endif

		#if SII_LIB_EDID_PAR__CLR_SPACE
		p->poEdidPar->bY444       = (pData[0] & 0x08) ? (true) : (false);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Support DC Y444", \
					  p->poEdidPar->bY444 ? "Yes" : "No"));
		#endif

		#if SII_LIB_EDID_PAR__DEEP_CLR
		p->poEdidPar->b444DC10    = (pData[0] & 0x10) ? (true) : (false);
		p->poEdidPar->b444DC12    = (pData[0] & 0x20) ? (true) : (false);
		p->poEdidPar->b444DC16    = (pData[0] & 0x40) ? (true) : (false);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Support DC 10bit", \
					  p->poEdidPar->b444DC10 ? "Yes" : "No"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Support DC 12bit", \
					  p->poEdidPar->b444DC12 ? "Yes" : "No"));
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Support DC 16bit", \
					  p->poEdidPar->b444DC16 ? "Yes" : "No"));
		#endif
#if ( __HDMI_OS_RTOS__ )
		g_hdmi_edid_result.rgb30bit =p->poEdidPar->b444DC10;
		g_hdmi_edid_result.rgb36bit =p->poEdidPar->b444DC12;
		g_hdmi_edid_result.rgb48bit =p->poEdidPar->b444DC16;
		g_hdmi_edid_result.dc_y444 =p->poEdidPar->bY444;
#endif
	}

	if ( !len ) {
		return;
	}
	pData = sRawPointerGet(p, block, addr);
	addr++;
	len--;

	#if SII_LIB_EDID_PAR__MAX_TMDS
	p->poEdidPar->maxTmds = 5000000 * (uint32_t)pData[0];
	#endif

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "Max TMDS Clock"));
	if ( pData[0] ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "%d MHz\n", (uint16_t)pData[0] * 5));
	} else {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "N.I.\n"));
	}

	if ( !len ) {
		return;
	}
	pData = sRawPointerGet(p, block, addr);
	addr++;
	len--;
	flags = pData[0];

	if ( flags & 0x0F ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        Supported content type:\n"));
	}
	if ( flags & 0x01 ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "            Graphics (text)\n"));
	}
	if ( flags & 0x02 ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "            Photo\n"));
	}
	if ( flags & 0x02 ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "            Cinema\n"));
	}
	if ( flags & 0x04 ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "            Game\n"));
	}

	if ( flags & 0x80 ) {
		if ( !len ) {
			return;
		}
		pData = sRawPointerGet(p, block, addr);
		addr++;
		len--;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 0x%02X\n", "Video Latency", pData[0]));
		#if SII_LIB_EDID_PAR__LIPSYNC
		p->poEdidPar->lipSync.latencyPresent = true;
		p->poEdidPar->lipSync.videoLatency = pData[0];
		#endif
		if ( !len ) {
			return;
		}
		pData = sRawPointerGet(p, block, addr);
		addr++;
		len--;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 0x%02X\n", "Audio Latency", pData[0]));
		#if SII_LIB_EDID_PAR__LIPSYNC
		p->poEdidPar->lipSync.audioLatency = pData[0];
		#endif
	}

	if ( flags & 0x40 ) {
		if ( !len ) {
			return;
		}
		pData = sRawPointerGet(p, block, addr);
		addr++;
		len--;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 0x%02X\n", "Interlaced Video Latency", pData[0]));
		#if SII_LIB_EDID_PAR__LIPSYNC
		p->poEdidPar->lipSync.ILatencyPresent = true;
		p->poEdidPar->lipSync.IVideoLatency = pData[0];
		#endif

		if ( !len ) {
			return;
		}
		pData = sRawPointerGet(p, block, addr);
		addr++;
		len--;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : 0x%02X\n", "Interlaced Audio Latency", pData[0]));
		#if SII_LIB_EDID_PAR__LIPSYNC
		p->poEdidPar->lipSync.IAudioLatency = pData[0];
		#endif

	}

	if ( flags & 0x20 ) {
		#if SII_LIB_EDID_PAR__3D
		/* Copy 3D data Block */
		p->poEdidPar->db3d.size = len;
		if ( len ) {
			pData = sRawPointerGet(p, block, addr);
			SII_MEMCPY(p->poEdidPar->db3d.b, pData, (p->poEdidPar->db3d.size <= SII_LIB_EDID__3DDB_MAX) ? p->poEdidPar->db3d.size : SII_LIB_EDID__3DDB_MAX);
		}
		#endif

		/* Parse 3D data Block */
		{
			uint8_t  VICLen  = 0;
			uint8_t  D3Len   = 0;
			uint8_t  D3Multi = 0;
			uint8_t  No3D    = 1;

			if ( !len ) {
				return;
			}
			pData = sRawPointerGet(p, block, addr);
			addr++;
			len--;
			D3Multi = (pData[0] & 0x60) >> 5;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "3D-Support", (pData[0] & 0x80) ? "yes" : "no"));
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "3D_Multi_present", D3Multi));
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "Image_Size", (pData[0] & 0x18) >> 3));

			#if SII_LIB_EDID_PAR__3D
			p->poEdidPar->d3Info.supported_3d = (pData[0] & 0x80) ? 1 : 0;
			p->poEdidPar->d3Info.supported_3d_multi = (pData[0] >> 5) & 0x3;
			p->poEdidPar->d3Info.Image_Size = (pData[0] >> 3) & 0x3;
			#endif
			if ( !len ) {
				return;
			}
			pData = sRawPointerGet(p, block, addr);
			addr++;
			len--;
			VICLen = (pData[0] & 0xE0) >> 5;
			D3Len  = (pData[0] & 0x1F) >> 0;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "HDMI_VIC_LEN", VICLen));
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "HDMI_3D_LEN", D3Len));

			while ( VICLen ) {
				/* Read Data */
				if ( !len ) {
					return;
				}
				pData = sRawPointerGet(p, block, addr);
				addr++;
				len--;

				/* Print out Data */
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "HDMI_VIC"));
				{
					#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
					sParseVsdbHdmi14SaveFmt(pData);
					#endif
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "%2d-", pData[0]));
					switch ( pData[0] ) {
						case  1 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "4K x 2K, 30Hz"));
							p->poEdidPar->d3Info.supported_4k2k_30 = 1;
#if ( __HDMI_OS_RTOS__ )
							g_hdmi_edid_result.supported_4k2k_30 = true;
#endif
							break;
						case  2 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "4K x 2K, 25Hz"));
							p->poEdidPar->d3Info.supported_4k2k_25 = 1;
#if ( __HDMI_OS_RTOS__ )
							g_hdmi_edid_result.supported_4k2k_25 = true;
#endif
							break;
						case  3 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "4K x 2K, 24Hz"));
							p->poEdidPar->d3Info.supported_4k2k_24 = 1;
#if ( __HDMI_OS_RTOS__ )
							g_hdmi_edid_result.supported_4k2k_24 = true;
#endif
							break;
						case  4 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "4K x 2K, 24Hz, SMPTE"));
							p->poEdidPar->d3Info.supported_4k2k_smpte_24 = 1;
#if ( __HDMI_OS_RTOS__ )
							g_hdmi_edid_result.supported_4k2k_smpte_24 = true;
#endif
							break;

						default :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
							break;
					}
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
				}

				VICLen--;
			}

			if ( 2 <= D3Len ) {
				if ( (0x01 == D3Multi) || (0x02 == D3Multi) ) {
					uint16_t Flag16;

					/* Read 1st Data */
					if ( !len ) {
						return;
					}
					pData = sRawPointerGet(p, block, addr);
					addr++;
					len--;
					D3Len--;

					Flag16 = pData[0] << 8;

					/* Read 2nd Data */
					if ( !len ) {
						return;
					}
					pData = sRawPointerGet(p, block, addr);
					addr++;
					len--;
					D3Len--;

					Flag16 |= pData[0];
					#if SII_LIB_EDID_PAR__3D
					p->poEdidPar->d3Info.Struc_all_3d = Flag16;
					#endif
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "3D_Structure_ALL"));
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%04X, (%s-%s-%s)\n", Flag16, (Flag16 & 0x1) ? "Frame" : " ", (Flag16 & 0x40) ? "TAB" : " ", (Flag16 & 0x100) ? "SBS" : " "));
				}
			}

			if ( 2 <= D3Len ) {
				if ( 0x02 == D3Multi ) {
					uint16_t Flag16;

					/* Read 1st Data */
					if ( !len ) {
						return;
					}
					pData = sRawPointerGet(p, block, addr);
					addr++;
					len--;
					D3Len--;

					Flag16 = pData[0] << 8;

					/* Read 2nd Data */
					if ( !len ) {
						return;
					}
					pData = sRawPointerGet(p, block, addr);
					addr++;
					len--;
					D3Len--;

					Flag16 |= pData[0];
					#if SII_LIB_EDID_PAR__3D
					p->poEdidPar->d3Info.mask_3d = Flag16;
					#endif
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : ", "3D_MASK"));
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "0x%04X\n", Flag16));
				}
			}

			while ( D3Len ) {
				uint8_t struc3d = 0;

				/* Read Data */
				if ( !len ) {
					return;
				}
				pData = sRawPointerGet(p, block, addr);
				addr++;
				len--;
				D3Len--;

				EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %s%-23d : ", "2D_VIC_order_", No3D));
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "%02d\n", (pData[0] & 0xF0) >> 4));

				struc3d = (pData[0] & 0x0F) >> 0;
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %s%-23d : ", "3D_Structure_", No3D));
				#if SII_LIB_EDID_PAR__3D
				p->poEdidPar->d3Info.vic_ord[No3D - 1].vic_order = (pData[0] & 0xF0) >> 4;
				p->poEdidPar->d3Info.vic_ord[No3D - 1].struc_3d = struc3d;
				p->poEdidPar->d3Info.Edid3DVicOrderCnt = No3D;
				#endif
				switch ( struc3d ) {
					case  0x0 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Frame Packing"));
						p->poEdidPar->d3Info.supported_3d_frmpack = 1;
						break;
					case  0x1 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Field alternative"));
						break;
					case  0x2 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Line alternative"));
						break;
					case  0x3 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Side-by-Side (full)"));
						break;
					case  0x4 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "L + depth"));
						break;
					case  0x5 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "L + depth + graph. + graph.-depth"));
						break;
					case  0x6 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Top-and-Bottom"));
						p->poEdidPar->d3Info.supported_3d_tb = 1;
						break;
					case  0x8 :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Side-by-Side (half)"));
						p->poEdidPar->d3Info.supported_3d_sbys = 1;
						break;
					default :
						EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
						break;
				}
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));

				/* Read Data */
				if ( (0x7 < struc3d) && D3Len ) {
					if ( !len ) {
						return;
					}
					pData = sRawPointerGet(p, block, addr);
					addr++;
					len--;
					D3Len--;

					EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %s%-26d : ", "3D_Detail_", No3D));
					switch ( (pData[0] & 0xF0) >> 4 ) {
						case  0x0 :
						case  0x1 :
						case  0x2 :
						case  0x3 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Horizontal sub-sampling"));
							break;
						case  0x4 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Odd/Left Pic., Odd/Right Pic."));
							break;
						case  0x5 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Odd/Left Pic., Even/Right Pic."));
							break;
						case  0x6 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Even/Left Pic., Odd/Right Pic."));
							break;
						case  0x7 :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Even/Left Pic., Even/Right Pic."));
							break;

						default :
							EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
							break;
					}
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
				}

				No3D++;
			}
		}
	}
}

static void sParseVsdbHdmiForum( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t* pData = sRawPointerGet(p, block, addr);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    HDMI Forum Vendor Specific Data Block"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "Version", pData[0]));
	len--;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %d MHz\n", "Max_TMDS_Character_Rate", (uint16_t)pData[1] * 5));
	len--;

	#if SII_LIB_EDID_PAR__SCDC
	p->poEdidPar->scdc.vclk_mb             = (uint32_t)pData[1] * 5;
	p->poEdidPar->scdc.bScdcPresent        = (pData[2] & 0x80) ? (true) : (false);
	p->poEdidPar->scdc.bReadReqCapable     = (pData[2] & 0x40) ? (true) : (false);
	p->poEdidPar->scdc.bLTE340MscsScramble = (pData[2] & 0x08) ? (true) : (false);

	p->poEdidPar->scdc.bIndependentView = (pData[2] & 0x04) ? (true) : (false);
	p->poEdidPar->scdc.bDualView        = (pData[2] & 0x02) ? (true) : (false);
	p->poEdidPar->scdc.b3DOsdDisparity  = (pData[2] & 0x01) ? (true) : (false);

	//	p->poEdidPar->bScdcRr  = (pData[1]*0x40) ? (true) : (false);
	//	p->poEdidPar->bScdc340 = (pData[1]*0x08) ? (true) : (false);
	#endif

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "SCDC_Present", (pData[2] & 0x80) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "RR_Capable", (pData[2] & 0x40) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "LTE_340Mcsc_scramble", (pData[2] & 0x08) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Independent_view", (pData[2] & 0x04) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Dual_View", (pData[2] & 0x02) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "3D_OSD_Disparity", (pData[2] & 0x01) ? "Yes" : "No"));
	len--;

	#if SII_LIB_EDID_PAR__SCDC
	p->poEdidPar->scdc.bUHD_VIC = (pData[3] & 0x08) ? (true) : (false);

	p->poEdidPar->scdc.bDc48bit420  = (pData[3] & 0x04) ? (true) : (false);
	p->poEdidPar->scdc.bDc36bit420  = (pData[3] & 0x02) ? (true) : (false);
	p->poEdidPar->scdc.bDc30bit420  = (pData[3] & 0x01) ? (true) : (false);
	#endif

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "UHD_VIC", (pData[3] & 0x08) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_48bit_420", (pData[3] & 0x04) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_36bit_420", (pData[3] & 0x02) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_30bit_420", (pData[3] & 0x01) ? "Yes" : "No"));

	len--;
}

static void sParseScdbHdmiForum( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t* pData;

	if (len < 6 || len > 30) {
		return;
	}

	p->poEdidPar->bHfScdb = 1;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    HDMI Forum Sink Capability Data Block...\n"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %02d\n", "Version", pData[2]));
	len--;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %d MHz\n", "Max_TMDS_Character_Rate", (uint16_t)pData[3] * 5));
	len--;

	#if SII_LIB_EDID_PAR__SCDC
	p->poEdidPar->scdc.vclk_mb             = (uint32_t)pData[3] * 5;
	p->poEdidPar->scdc.bScdcPresent        = (pData[4] & 0x80) ? (true) : (false);
	p->poEdidPar->scdc.bReadReqCapable     = (pData[4] & 0x40) ? (true) : (false);
	p->poEdidPar->scdc.bLTE340MscsScramble = (pData[4] & 0x08) ? (true) : (false);

	p->poEdidPar->scdc.bIndependentView = (pData[4] & 0x04) ? (true) : (false);
	p->poEdidPar->scdc.bDualView        = (pData[4] & 0x02) ? (true) : (false);
	p->poEdidPar->scdc.b3DOsdDisparity  = (pData[4] & 0x01) ? (true) : (false);

	#endif

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "SCDC_Present", (pData[4] & 0x80) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "RR_Capable", (pData[4] & 0x40) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "LTE_340Mcsc_scramble", (pData[4] & 0x08) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Independent_view", (pData[4] & 0x04) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "Dual_View", (pData[4] & 0x02) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "3D_OSD_Disparity", (pData[4] & 0x01) ? "Yes" : "No"));
	len--;

	#if SII_LIB_EDID_PAR__SCDC
	p->poEdidPar->scdc.bUHD_VIC = (pData[5] & 0x08) ? (true) : (false);

	p->poEdidPar->scdc.bDc48bit420  = (pData[5] & 0x04) ? (true) : (false);
	p->poEdidPar->scdc.bDc36bit420  = (pData[5] & 0x02) ? (true) : (false);
	p->poEdidPar->scdc.bDc30bit420  = (pData[5] & 0x01) ? (true) : (false);
	#endif

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "UHD_VIC", (pData[5] & 0x08) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_48bit_420", (pData[5] & 0x04) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_36bit_420", (pData[5] & 0x02) ? "Yes" : "No"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "        %-36s : %s\n", "DC_30bit_420", (pData[5] & 0x01) ? "Yes" : "No"));
	len--;
}

static void sParseSbtmHdmiForum( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t* pData;
	uint8_t DRDM_ind = 0;
	uint8_t GDRDM_support = 0;
	uint8_t Gamut = 0;
	uint8_t Use_HGIG_DRDM;
	uint16_t Rx, Ry, Gx, Gy, Bx, By, Wx, Wy;
	uint8_t Min_bright, Peak_bright;
	uint8_t offset = 1;

	if (len < 1) {
		return;
	}

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);

	DRDM_ind = (pData[0] >> 7) & 0x1;
	GDRDM_support = (pData[0] >> 5) & 0x3;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    HDMI Forum Source-Based Tone Mapping Data Block...\n"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "DRDM_ind:%d,GRDM_support:%d\n", (int)DRDM_ind, (int)GDRDM_support));
	if ( DRDM_ind ) {
		offset += 1;
		Gamut = (pData[1] & 0xC0) >> 6;
		Use_HGIG_DRDM = pData[1] & 0x10;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "Gamut:%d,MaxRGB:%d,Use_HGID_DRDM:%d,HGIG_cat_DRDM_sel:%d\n", (int)Gamut, (int)(pData[1] & 0x20), (int)Use_HGIG_DRDM, (int)(pData[1] & 0x1)));
		if ( Gamut == 0x0) {
			Rx = pData[2] + (pData[3] << 8);
			Ry = pData[4] + (pData[5] << 8);
			Gx = pData[6] + (pData[7] << 8);
			Gy = pData[8] + (pData[9] << 8);
			Bx = pData[10] + (pData[11] << 8);
			By = pData[12] + (pData[13] << 8);
			Wx = pData[14] + (pData[15] << 8);
			Wy = pData[16] + (pData[17] << 8);
			offset += 16;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "R[%d %d] G[%d %d] B[%d %d] W[%d %d]\n", (uint32_t)Rx, (uint32_t)Ry, (uint32_t)Gx, (uint32_t)Gy, (uint32_t)Bx, (uint32_t)By, (uint32_t)Wx, (uint32_t)Wy));
		}
		if ( Use_HGIG_DRDM == 0 ) {
			Min_bright = pData[offset];
			Peak_bright = pData[offset + 1];
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "Min_bri:%d,Peak_bri:%d\n", (uint32_t)Min_bright, (uint32_t)Peak_bright));
			offset += 2;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "P0[%d %d %d]\n", (uint32_t)((pData[offset] >> 2) & 0x3F), (uint32_t)(pData[offset] & 0x3), pData[offset + 1]));
			offset += 2;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "P1[%d %d %d]\n", (uint32_t)((pData[offset] >> 2) & 0x3F), (uint32_t)(pData[offset] & 0x3), pData[offset + 1]));
			offset += 2;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "P2[%d %d %d]\n", (uint32_t)((pData[offset] >> 2) & 0x3F), (uint32_t)(pData[offset] & 0x3), pData[offset + 1]));
			offset += 2;
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "P3[%d %d %d]\n", (uint32_t)((pData[offset] >> 2) & 0x3F), (uint32_t)(pData[offset] & 0x3), pData[offset + 1]));
		}
	}
}

static void sParseVenderSpecificDataBlock( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*             pData;
	uint32_t             ieee    = 0x000000;
	//SiiLibEdidCecAddr_t  cecAddr = {0,0,0,0};

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nVendor Specific Data Block (Length=%02d)...\n", len));

	/* Read Data */
	if ( 3 > len ) {
		return;
	}
	pData = sRawPointerGet(p, block, addr);
	len  -= 3;
	addr += 3;

	ieee = ((uint32_t)pData[0] << 0) + ((uint32_t)pData[1] << 8) + ((uint32_t)pData[2] << 16);
	//	p->poEdidPar->ieee_id = ieee;

	/* Print out Data */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "IEEE Registration Identifier"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%06X\n", ieee));

	switch ( ieee ) {
		case 0x000C03 :
		case 0x000C00 : //Not official value, for #27755
			sParseVsdbHdmi14(p, block, addr, len);
			p->poEdidPar->ieee_id = ieee;
			break;

		case 0xC45DD8 :
			sParseVsdbHdmiForum(p, block, addr, len);
			p->poEdidPar->ieee_id = ieee;
			break;

		default :
			return;
	}
}

static void sParseColorimetry( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;

	if (len > 2 ) {
		return;
	}

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);

	p->poEdidPar->colorimetry.xvYCC601 = (bool_t)((pData[0] >> 0) & 0x1);
	p->poEdidPar->colorimetry.xvYCC709 = (bool_t)((pData[0] >> 1) & 0x1);
	p->poEdidPar->colorimetry.sYCC601 = (bool_t)((pData[0] >> 2) & 0x1);
	p->poEdidPar->colorimetry.AdobeYCC601 = (bool_t)((pData[0] >> 3) & 0x1);
	p->poEdidPar->colorimetry.AdobeRGB = (bool_t)((pData[0] >> 4) & 0x1);
	p->poEdidPar->colorimetry.BT2020cYCC = (bool_t)((pData[0] >> 5) & 0x1);
	p->poEdidPar->colorimetry.BT2020YCC = (bool_t)((pData[0] >> 6) & 0x1);
	p->poEdidPar->colorimetry.BT2020RGB = (bool_t)((pData[0] >> 7) & 0x1);

	p->poEdidPar->colorimetry.DCI_P3 = (bool_t)((pData[1] >> 7) & 0x1);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n BT2020cYCC:%04d, BT2020YCC:%04d, BT2020RGB:%04d, %04x\n", \
				  (uint32_t)p->poEdidPar->colorimetry.BT2020cYCC, \
				  (uint32_t)p->poEdidPar->colorimetry.BT2020YCC, \
				  (uint32_t)p->poEdidPar->colorimetry.BT2020RGB, (uint32_t)pData[0]));
#if (__HDMI_OS_RTOS__)
    g_hdmi_edid_result.supported_xvycc601 =p->poEdidPar->colorimetry.xvYCC601;
    g_hdmi_edid_result.supported_xvycc709 =p->poEdidPar->colorimetry.xvYCC709;
    g_hdmi_edid_result.supported_bt2020cycc =p->poEdidPar->colorimetry.BT2020cYCC;
    g_hdmi_edid_result.supported_bt2020ycc =p->poEdidPar->colorimetry.BT2020YCC;
    g_hdmi_edid_result.supported_bt2020rgb =p->poEdidPar->colorimetry.BT2020RGB;
#endif
}

static uint32_t sGetHdrLum(uint8_t data)
{
	uint32_t lum = 0;
	lum = (1<<(data>>5))*g_2sqrt_n_32[data&0x1f];
	lum *= 50;
	lum = MT_FLOAT_DIV(uint64_t,lum,1000000);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sGetHdrLum:0x%x,max:%d\n", (uint32_t)data, lum));
	return lum;
}

static uint32_t sGetHdrMinLum(uint8_t data, uint32_t maxlum)
{
	#define HDR_MIN_LUM_MULTI			(100)
	uint32_t lum = 0;
	lum = maxlum * data * data * HDR_MIN_LUM_MULTI;
	lum = MT_FLOAT_DIV(uint64_t,lum,65025);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sGetHdrMinLum:0x%x,max:%d,lum:%d\n", (uint32_t)data, maxlum,lum));
	return lum;
}

static void sParseEOTF( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;

	if (len < 1 ) {
		return;
	}

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);

	p->poEdidPar->hdrInfo.Gamma_SDR = (bool_t)((pData[0] >> 0) & 0x1);
	p->poEdidPar->hdrInfo.Gamma_HDR = (bool_t)((pData[0] >> 1) & 0x1);
	p->poEdidPar->hdrInfo.Smpte2084 = (bool_t)((pData[0] >> 2) & 0x1);     //HDR 10
	p->poEdidPar->hdrInfo.HLG = (bool_t)((pData[0] >> 3) & 0x1);               //HLG

	p->poEdidPar->hdrInfo.SmType = (bool_t)((pData[1] >> 0) & 0x1);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n len:%d HDR:%02d, HLG:%02d\n", (uint32_t)len, (uint32_t)p->poEdidPar->hdrInfo.Smpte2084, \
				  (uint32_t)p->poEdidPar->hdrInfo.HLG));
	if ( len > 2 ) {
		p->poEdidPar->hdrInfo.MaxLum = sGetHdrLum(pData[2]);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Desired Content Max Luminance %d(nit)\n", p->poEdidPar->hdrInfo.MaxLum));
	}
	if ( len > 3 ) {
		p->poEdidPar->hdrInfo.MaxAvgLum = sGetHdrLum(pData[3]);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Desired Content Max Frame-average Luminance %d(nit)\n", p->poEdidPar->hdrInfo.MaxAvgLum));
	}
	if ( len > 4 ) {
		p->poEdidPar->hdrInfo.MinLum = sGetHdrMinLum(pData[4], p->poEdidPar->hdrInfo.MaxLum);
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Desired Content Min Luminance %d(x0.0001nit)\n", p->poEdidPar->hdrInfo.MinLum));
	}
#if (__HDMI_OS_RTOS__)
	g_hdmi_edid_result.supported_hlg =p->poEdidPar->hdrInfo.HLG;
    g_hdmi_edid_result.supported_hdr10 =p->poEdidPar->hdrInfo.Smpte2084;
#endif
}

static void sParse420VDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nYUV420 Video Data Block (Length=%02d)...\n", len));

	while ( EDID_SVD_SIZE <= len ) {
		uint8_t*        pData = sRawPointerGet(p, block, addr);
		SiiLibEdidSvd_t SVD;

		/* Read Data */
		SII_MEMCPY((uint8_t*)&SVD, pData, EDID_SVD_SIZE);

		/* Print out Data */
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Short Video Descriptor"));

		{
			uint8_t VID      = SVD.p[0] & 0x7F;
			bool_t  bNative  = (SVD.p[0] & 0x80) ? (true) : (false);

			#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
			sStoreFmtInfo((uint32_t)VID, bNative, true);
			#endif

			EDID_PRINTF(( SI_LOG_LEVEL_STRING "%20d-", VID));
			switch ( VID ) {
				case  0 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "No VID available"));
					break;

				case  1 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "640x480p,    60Hz,  4:3"));
					break;
				case  2 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,    60Hz,  4:3"));
					break;
				case  3 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,    60Hz, 16:9"));
					break;
				case  4 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,   60Hz, 16:9"));
					break;
				case  5 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  60Hz, 16:9"));
					break;
				case  6 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,    60Hz,  4:3"));
					break;
				case  7 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,    60Hz, 16:9"));
					break;
				case  8 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x240p,    60Hz,  4:3"));
					break;
				case  9 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x240p,    60Hz, 16:9"));
					break;
				case 10 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480i,   60Hz,  4:3"));
					break;
				case 11 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480i,   60Hz, 16:9"));
					break;
				case 12 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x240p,   60Hz,  4:3"));
					break;
				case 13 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x240p,   60Hz, 16:9"));
					break;
				case 14 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x480p,   60Hz,  4:3"));
					break;
				case 15 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x480p,   60Hz, 16:9"));
					break;
				case 16 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  60Hz, 16:9"));
					break;
				case 17 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,    50Hz,  4:3"));
					break;
				case 18 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,    50Hz, 16:9"));
					break;
				case 19 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,   50Hz, 16:9"));
					break;
				case 20 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  50Hz, 16:9"));
					break;
				case 21 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,    50Hz,  4:3"));
					break;
				case 22 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,    50Hz, 16:9"));
					break;
				case 23 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x288p,    50Hz,  4:3"));
					break;
				case 24 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x288p,    50Hz, 16:9"));
					break;
				case 25 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576i,   50Hz,  4:3"));
					break;
				case 26 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576i,   50Hz, 16:9"));
					break;
				case 27 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x288p,   50Hz,  4:3"));
					break;
				case 28 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x288p,   50Hz, 16:9"));
					break;
				case 29 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x576p,   50Hz,  4:3"));
					break;
				case 30 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1440x576p,   50Hz, 16:9"));
					break;
				case 31 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  50Hz, 16:9"));
					break;
				case 32 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  24Hz, 16:9"));
					break;
				case 33 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  25Hz, 16:9"));
					break;
				case 34 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080p,  30Hz, 16:9"));
					break;
				case 35 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480p,   60Hz,  4:3"));
					break;
				case 36 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x480p,   60Hz, 16:9"));
					break;
				case 37 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576p,   50Hz,  4:3"));
					break;
				case 38 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "2880x576p,   50Hz, 16:9"));
					break;
				case 39 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i,  50Hz, 16:9"));
					break;
				case 40 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i, 100Hz, 16:9"));
					break;
				case 41 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  100Hz, 16:9"));
					break;
				case 42 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,   100Hz,  4:3"));
					break;
				case 43 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576p,   100Hz, 16:9"));
					break;
				case 44 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   100Hz,  4:3"));
					break;
				case 45 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   100Hz, 16:9"));
					break;
				case 46 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1920x1080i, 120Hz, 16:9"));
					break;
				case 47 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  120Hz, 16:9"));
					break;
				case 48 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   120Hz,  4:3"));
					break;
				case 49 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   120Hz, 16:9"));
					break;
				case 50 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   120Hz,  4:3"));
					break;
				case 51 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   120Hz, 16:9"));
					break;
				case 52 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720X576p,   200Hz,  4:3"));
					break;
				case 53 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720X576p,   200Hz, 16:9"));
					break;
				case 54 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   200Hz,  4:3"));
					break;
				case 55 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x576i,   200Hz, 16:9"));
					break;
				case 56 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   240Hz,  4:3"));
					break;
				case 57 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480p,   240Hz, 16:9"));
					break;
				case 58 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   240Hz,  4:3"));
					break;
				case 59 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "720x480i,   240Hz, 16:9"));
					break;
				case 60 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  24Hz,  16:9"));
					break;
				case 61 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  25Hz,  16:9"));
					break;
				case 62 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "1280x720p,  30Hz,  16:9"));
					break;
				case 93 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 24Hz,  16:9"));
					break;
				case 94 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 25Hz,  16:9"));
					break;
				case 95 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 30Hz,  16:9"));
					break;
				case 96 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 50Hz,  16:9"));
					break;
				case 97 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "3840x2160p, 60Hz,  16:9"));
					break;
				case 98 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 24Hz,  256:135"));
					break;
				case 99 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 25Hz,  256:135"));
					break;
				case 100 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 30Hz,  256:135"));
					break;
				case 101 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 50Hz,  256:135"));
					break;
				case 102 :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "4096x2160p, 60Hz,  256:135"));
					break;

				default :
					EDID_PRINTF(( SI_LOG_LEVEL_STRING "Reserved"));
					break;
			}

			if ( bNative ) {
				EDID_PRINTF(( SI_LOG_LEVEL_STRING " (N)"));
			}

			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n"));
		}

		if ( p->poEdidPar->yuv420Vdb.size < SII_LIB_EDID__SVD_MAX ) {
			p->poEdidPar->yuv420Vdb.svd[p->poEdidPar->yuv420Vdb.size] = SVD;
			p->poEdidPar->yuv420Vdb.size++;
		}

		addr += EDID_SVD_SIZE;
		len  -= EDID_SVD_SIZE;
	}
}

static void sParse420CMDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	if ( len > SII_LIB_EDID__YUV420CMDB_MAX) {
		len = SII_LIB_EDID__YUV420CMDB_MAX;
	}
	memset(p->poEdidPar->yuv420CMDB, 0, SII_LIB_EDID__YUV420CMDB_MAX);
	for (i = 0; i < len; i++) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n YCbCr420 CMDB[%d]=0x%x\n", (uint32_t)i, (uint32_t)pData[i]));
		p->poEdidPar->yuv420CMDB[i] = pData[i];
	}
}

static void sParseVSADB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint32_t		IEEE_Id = 0;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	IEEE_Id = pData[0] | (pData[1] << 8) | (pData[2] << 16);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n VSADB IEEE:0x%x\n",IEEE_Id));
	switch ( IEEE_Id ) {
		case 0x00D046:
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Dolby Vendor ADB\n"));
			p->poEdidPar->DdVsadb.HdPhonePb = (pData[3]&0x80) >> 7;
			p->poEdidPar->DdVsadb.HeightSpeaker = (pData[3]&0x40) >> 6;
			p->poEdidPar->DdVsadb.SurroundSpeaker = (pData[3]&0x20) >> 5;
			p->poEdidPar->DdVsadb.CenterSpeaker = (pData[3]&0x10) >> 4;
			p->poEdidPar->DdVsadb.DdVsadbVer = pData[3]&0x07;
			p->poEdidPar->DdVsadb.SinkCap = pData[4];
			break;
		default:
			break;
	}
}

static void sParseIFDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;
	uint8_t		ext_len = 0;
	uint8_t		ext_sids_len = 0;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	if ( len != 9) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseIFDB len=0x%x\n", (uint32_t)len));
	}
	//memset(p->poEdidPar->yuv420CMDB, 0, SII_LIB_EDID__YUV420CMDB_MAX);
	ext_len = (pData[0] >> 5) & 0x07;
	if ( ext_len != 0 ) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n IFDB Ext len=0x%x\n", (uint32_t)ext_len));
	}
	{
		p->poEdidPar->vsifs = pData[1] + 1;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Support VSIFs can be received in addition to the first : %d\n", (uint32_t)(pData[1])));
	}
	for (i = 2; i < len;) {
		uint8_t ifcode = 0;
		ext_sids_len = (pData[i] >> 5) & 0x07;
		ifcode = pData[i] & 0x1f;
		/*
		*    CTA-861-H   Infoframe Type Code
		*    0x00: Reserved
		*    0x01: Vendor-Specific(Defined in section 6.1)
		*    0x02: Auxiliary Video Infoframe(Defined in section 6.4)
		*    0x03: Source Product Description(Defined in section 6.5)
		*    0x04: Audio(Defined in section 6.6)
		*    0x05: MPEG Source(Defined in section 6.7)
		*    0x06: NTSC VBI(Defined in section 6.8)
		*    0x07: Dynamic Range and Mastering(Defined in section 6.9)
		*/
		switch ( ifcode ) {
			case 0:
				break;
			case 1:
				//SVSIDs
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Short VS IFD {0x%x 0x%x 0x%x}\n", \
							  (uint32_t)pData[i + 1], (uint32_t)pData[i + 2], (uint32_t)pData[i + 3]));
				i += 1 + 3 + ext_sids_len;
				break;
			default:
				//SIDs
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n Short IFD {0x%x}\n", \
							  (uint32_t)(pData[i + 1])));
				i += 1 + ext_sids_len;
				break;
		}
		//EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseIFDB[%d]=0x%x\n", (uint32_t)i,(uint32_t)pData[i]));
		//p->poEdidPar->yuv420CMDB[i] = pData[i];
	}
}

static void sParseVFPDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	for (i = 0; i < len; i++) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n VFPDB[%d]=0x%x\n", (uint32_t)i, (uint32_t)pData[i]));
	}
}

static void sParseHdrDMDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;
	uint8_t		j;
	uint8_t		k = 0;
	uint8_t		h = 0;
	uint8_t	tlen = 0;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	for (i = 0; i < len;) {
		tlen = pData[i];
		if ( tlen > 2 ) {
			p->poEdidPar->DMDBInfo[k].DMType = pData[i + 1] | (pData[i + 2] << 8);
			p->poEdidPar->DMDBInfo[k].Flags = pData[i + 3];
			if ( p->poEdidPar->DMDBInfo[k].DMType & 0xFF) {
				p->poEdidPar->hdr10p_emp |= 1 << ((p->poEdidPar->DMDBInfo[k].DMType & 0xFF) - 1);
			} else if ( p->poEdidPar->DMDBInfo[k].DMType & 0xFF00 ) {
				p->poEdidPar->hdr10p_emp |= 1 << (((p->poEdidPar->DMDBInfo[k].DMType >> 8) & 0xFF) + 15);
			}
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n HDR DMDB type[%d]:0x%x  ,0x%x\n", k, p->poEdidPar->DMDBInfo[k].DMType, p->poEdidPar->DMDBInfo[k].Flags));
			i += 4;
			h = 0;
			for (j = 0; j < (tlen - 3); j++) {
				p->poEdidPar->DMDBInfo[k].OptFields[h] = pData[i + j];
				EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n HDR DMDB data[%d]=0x%x\n", (uint32_t)i, (uint32_t)pData[i]));
				h++;
			}
			i += h;
		} else {
			EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n HDR DMDB error:%d,%d,%d\n", (uint32_t)i, (uint32_t)k, (uint32_t)tlen));
			break;
		}
		k++;
	}
}

static void sParseEEODB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	for (i = 0; i < len; i++) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n EEODB[%d]=0x%x\n", (uint32_t)i, (uint32_t)pData[i]));
	}
}

static void sParseVSVDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         i;
	uint32_t		IEEE_Id = 0;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	if ( len < 3) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVSVDB len=0x%x error\n", (uint32_t)len));
		return ;
	}
	IEEE_Id = pData[0] | (pData[1] << 8) | (pData[2] << 16);
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVSVDB IEEE OUI: 0x%x\n", (uint32_t)IEEE_Id));
	if ( IEEE_Id == 0x90848B) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVSVDB IEEE OUI: HDR10+ VSIF\n"));
		p->poEdidPar->hdr10p_vsif = 1;
	} else if ( IEEE_Id == 0x047503) {
		p->poEdidPar->hdr10p_vivid.vivid = 1;
		p->poEdidPar->hdr10p_vivid.ststem_start_code = pData[3];
		p->poEdidPar->hdr10p_vivid.version_code = pData[4]>>4;
		p->poEdidPar->hdr10p_vivid.MaxLum = pData[5] | (pData[6] << 8) | (pData[7] << 16) | (pData[8] << 24);
		p->poEdidPar->hdr10p_vivid.MinLum = pData[9] | (pData[10] << 8);
		p->poEdidPar->hdr10p_vivid.monitor_mode_support = (pData[11] & 0x80) != 0;
		p->poEdidPar->hdr10p_vivid.rx_mode_support = (pData[11] & 0x40) != 0;
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVSVDB IEEE OUI: HDR10+ Vivid,[%d, %d], maxLum:%d(*0.0001cd/m2),Minlum:%d(*0.0001cd/m2)[%d %d]\n",\
			p->poEdidPar->hdr10p_vivid.ststem_start_code,p->poEdidPar->hdr10p_vivid.version_code,\
			p->poEdidPar->hdr10p_vivid.MaxLum,p->poEdidPar->hdr10p_vivid.MinLum,\
			p->poEdidPar->hdr10p_vivid.monitor_mode_support,p->poEdidPar->hdr10p_vivid.rx_mode_support));
	}
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	sParseVSVDBSaveHDR10P((MT_BOOL)(p->poEdidPar->hdr10p_vsif), (MT_BOOL)(p->poEdidPar->hdr10p_vivid.vivid));
#endif
	for (i = 0; i < len - 3; i++) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVSVDB data [%d]=0x%x\n", (uint32_t)i, (uint32_t)pData[i + 3]));
	}
}

static void sParseVCDB( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t			QY;
	uint8_t			QS;
	uint8_t			S_PT;
	uint8_t			S_IT;
	uint8_t			S_CE;

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	if ( len < 1) {
		EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n sParseVCDB len=0x%x error\n", (uint32_t)len));
		return ;
	}
	QY = (pData[0] >> 7) & 0x1;
	QS = (pData[0] >> 6) & 0x1;
	S_PT = (pData[0] >> 4) & 0x3;
	S_IT = (pData[0] >> 2) & 0x3;
	S_CE = (pData[0] >> 0) & 0x3;
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n VCDB: QY:%s,QS:%s,Over/Under Scan PT:0x%x,IT:0x%x,CE:0x%x\n", QY ? "Selectable" : "No Data", QS ? "Selectable" : "No Data", S_PT, S_IT, S_CE));
}

static void sParseExtendedTagTable( ParseData_t* p, uint8_t block, uint8_t addr, uint8_t len )
{
	uint8_t*        pData;
	uint8_t         type;
	//SiiLibEdidCecAddr_t  cecAddr = {0,0,0,0};

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\nUse Extended Tag (Length=%02d)...\n", len));

	/* Read Data */
	pData = sRawPointerGet(p, block, addr);
	len--;
	addr ++;

	type = pData[0];

	/* Print out Data */
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "    %-40s : ", "Extended tag"));
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "%06X\n", (uint32_t)type));

	switch ( type ) {
		case 0x0 :
			/* Video Capability Data Block */
			;
			sParseVCDB(p, block, addr, len);
			break;

		case 0x1 :
			/* Vendor-Specific Video Data Block */
			;
			sParseVSVDB(p, block, addr, len);
			break;

		case 0x2 :
			/* VESA Display Device Data Block [100] */
			;
			break;

		case 0x3 :
			/* VESA Video Timing Block Extension */
			;
			break;

		case 0x4 :
			/* Reserved for HDMI Video Data Block */
			;
			break;

		case 0x5 :
			/* Colorimetry Data Block */
			;
			sParseColorimetry(p, block, addr, len);
			break;

		case 0x6 :
			/* HDR Static Metadata Data Block */
			;
			sParseEOTF(p, block, addr, len);
			break;

		case 0x7 :
			/* HDR Dynamic Metadata Data Block */
			;
			sParseHdrDMDB(p, block, addr, len);
			break;

		case 0xD :
			/* Video Format Preference Data Block */
			;
			sParseVFPDB(p, block, addr, len);
			break;

		case 0xE :
			/* YCbCr 4:2:0 Video Data Block */
			;
			sParse420VDB(p, block, addr, len);
			break;

		case 0xF :
			/* YCbCr 4:2:0 Capability Map Data Block */
			;
			sParse420CMDB(p, block, addr, len);
			break;
		case 0x11:
			/* vendor specific audio Data Block */
			sParseVSADB(p, block, addr, len);
			break;

		case 0x20 :
			/* Infoframe Data Block */
			;
			sParseIFDB(p, block, addr, len);
			break;

		case 0x78:
			/* HDMI Forum EEODB(EDID Extension Override Data Block) */
			;
			sParseEEODB(p, block, addr, len);
			break;

		case 0x79:
			/* HDMI Forum SCDB */
			;
			sParseScdbHdmiForum(p, block, addr, len);
			break;
		case 0x7A:
			/* HDMI Forum SBTM - HF-SBTMDB */
			;
			sParseSbtmHdmiForum(p, block, addr, len);
			break;
		default :
			return;
	}
}

static void sCEADataBlockCollection( ParseData_t* p, uint8_t block )
{
	uint8_t  addr      = 4;
	uint8_t  StartDTD  = 4;
	uint8_t* pData;

	sCheck861B(p, block);
	if ( p->error ) {
		return;
	}

	sParseCEAExt3Byte3(p, block);
	if ( p->error ) {
		return;
	}

	/* Read End address */
	pData = sRawPointerGet(p, block, 0x02);
	StartDTD = pData[0];

	while ( StartDTD > addr ) {
		uint8_t Tag = 0;
		uint8_t len = 0;

		/* Read Data */
		pData = sRawPointerGet(p, block, addr);
		Tag = (pData[0] & 0xE0) >> 5;
		len = (pData[0] & 0x1F);

		addr++;
		switch ( Tag ) {
			case 1 :
				/* CEA Short Audio Descriptor Block */
				sParseSADTable(p, block, addr, len);
				break;

			case 2 :
				/* CEA Short Video Descriptor */
				sParseSVDTable(p, block, addr, len);
				break;

			case 3 :
				/* Vendor Specific Data Block */
				sParseVenderSpecificDataBlock(p, block, addr, len);
				break;

			case 4 :
				/* Speaker Allocation data Block */
				sParseSpeakerTagTable(p, block, addr, len);
				break;

			case 5 :
				/* VESA Display Transfer Characteristic data Block */
				break;

			case 7 :
				/* Use Extended Tag */
				sParseExtendedTagTable(p, block, addr, len);
				break;

			default :
				break;
		}

		addr += len;
	}
}

static void sResetBasicInfo(ParseData_t* p)
{
	if ( p->poEdidPar ) {
		#if SII_LIB_EDID_PAR__DISPLAY_AR
		p->poEdidPar->DisplayAR           = 0.0;
		#endif

		#if SII_LIB_EDID_PAR__MONITOR_NAME
		p->poEdidPar->pMonitorNameStr[0]  = 0;
		p->poEdidPar->pMonitorNameStr[13] = 0;
		#endif

		#if SII_LIB_EDID_PAR__DTDB
		p->poEdidPar->dtdb.size           = 0;
		#endif
		memset(&(p->poEdidPar->maninfo), 0, sizeof(SiiLibManInfo_t));
	}
}

static void sResetExtentionInfo(ParseData_t* p)
{
	if ( p->poEdidPar ) {
		#if SII_LIB_EDID_PAR__SADB
		p->poEdidPar->sadb.size           = 0;
		#endif

		#if SII_LIB_EDID_PAR__ADB
		p->poEdidPar->adb.size            = 0;
		#endif

		#if SII_LIB_EDID_PAR__VDB
		p->poEdidPar->vdb.size            = 0;
		#endif

		#if SII_LIB_EDID_PAR__3D
		p->poEdidPar->db3d.size           = 0;
		#endif

		#if SII_LIB_EDID_PAR__CEC_ADDR
		p->poEdidPar->cecAddrPtr          = 0;
		p->poEdidPar->cecAddr.sub[0]      = 0;
		p->poEdidPar->cecAddr.sub[1]      = 0;
		p->poEdidPar->cecAddr.sub[2]      = 0;
		p->poEdidPar->cecAddr.sub[3]      = 0;
		#endif

		#if SII_LIB_EDID_PAR__AUD_FRM
		p->poEdidPar->bBasicAudio         = false;
		p->poEdidPar->bSupportsAI         = false;
		#endif

		#if SII_LIB_EDID_PAR__MAX_TMDS
		p->poEdidPar->maxTmds            = 0;
		#endif

		#if SII_LIB_EDID_PAR__DEEP_CLR
		p->poEdidPar->b444DC10            = false;
		p->poEdidPar->b444DC12            = false;
		p->poEdidPar->b444DC16            = false;
		#endif

		#if SII_LIB_EDID_PAR__CLR_SPACE
		p->poEdidPar->bY444               = false;
		#endif

		#if SII_LIB_EDID_PAR__SCDC
		p->poEdidPar->scdc.b3DOsdDisparity     = false;
		p->poEdidPar->scdc.bDualView           = false;
		p->poEdidPar->scdc.bIndependentView    = false;
		p->poEdidPar->scdc.bLTE340MscsScramble = false;
		p->poEdidPar->scdc.bReadReqCapable     = false;
		p->poEdidPar->scdc.bScdcPresent        = false;
		p->poEdidPar->scdc.bDc30bit420         = false;
		p->poEdidPar->scdc.bDc36bit420         = false;
		p->poEdidPar->scdc.bDc48bit420         = false;
		p->poEdidPar->scdc.bUHD_VIC            = false;
		#endif
		memset(&(p->poEdidPar->colorimetry), 0, sizeof(SiiLibColorimetry_t));
		memset(&(p->poEdidPar->hdrInfo), 0, sizeof(SiiLibHDRInfo_t));
		memset(p->poEdidPar->yuv420CMDB, 0, SII_LIB_EDID__YUV420CMDB_MAX);
		p->poEdidPar->yuv420Vdb.size            = 0;
		p->poEdidPar->bHfScdb = 0;
		p->poEdidPar->bUnderScan	= 0;
		p->poEdidPar->bAudio	= 0;
		p->poEdidPar->Yuv444	= 0;
		p->poEdidPar->Yuv422	= 0;
		p->poEdidPar->hdr10p_vsif = 0;
		p->poEdidPar->hdr10p_emp = 0;
		memset(&(p->poEdidPar->hdr10p_vivid), 0, sizeof(SiiLibVividInfo_t));
		p->poEdidPar->vsifs = 1;
		memset(&(p->poEdidPar->DdVsadb), 0, sizeof(SiiLibDdVsAudInfo_t));
		memset(&(p->poEdidPar->d3Info), 0, sizeof(SiiLibEdid3DInfo_t));
		memset(&(p->poEdidPar->audInfo), 0, sizeof(SiiLibAudInfo_t));
		memset(p->poEdidPar->DMDBInfo, 0, sizeof(SiiLibEdidDmTypeInfo_t)*SII_LIB_EDID__DMDB_MAX);
	}
}

static void sResetParseInfo(ParseData_t* p)
{
	p->error = 0;
	sResetBasicInfo(p);
	sResetExtentionInfo(p);
}

static void sParseEdid(ParseData_t* p)
{
	uint8_t block;
	uint8_t ddbAddr;

	EDID_PRINTF(( SI_LOG_LEVEL_STRING "\n\nFirst Block Parse...\n"));

	/* Clear previous parsed data */
	sResetParseInfo(p);

	/* Check CheckSum first Block */
	sParseCheckSum(p, 0);
	if ( p->error ) {
		return;
	}

	/* Parse first block */
	sParseBasicInfo(p);
	if ( p->error ) {
		return;
	}

	#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	EdidCheckHdmiFlg();
	#endif

	/* Extention Blocks */
	if ( 0 != p->extensions ) {
		uint8_t* pData;

		for ( block = 1; block <= p->extensions; block++ ) {
			/* Check CheckSum other Blocks */
			sParseCheckSum(p, block);
			//if( p->error ) return;
			p->error = SII_LIB_EDID_ERR_CODE__NO_ERROR;
		}

		//block = (1 == p->extensions) ? (1) : (2);

		for (block = 1; block <= p->extensions; block++ ) {
			/* Read d = offset for the byte following the reserved data block */
			pData   = sRawPointerGet(p, block, 0x02);
			ddbAddr = pData[0];

			/* Check for available CEA Data Blocks */
			if ( 4 != ddbAddr ) {
				sCEADataBlockCollection(p, block);
				//if( p->error ) return;
				if ( p->error == SII_LIB_EDID_ERR_CODE__CEA_TAG) {
					p->error = SII_LIB_EDID_ERR_CODE__NO_ERROR;
					continue;
				} else if ( p->error ) {
					return;
				}
			}

			/* Check for available Detailed Decriptor Blocks */
			if ( 0 != ddbAddr ) {
				/* Check Detailed Decriptor Blocks */
				while ( (0x7F - 18) >= ddbAddr ) {
					sParseDetailedDescriptorBlock(p, block, ddbAddr);
					if ( p->error ) {
						return;
					}

					ddbAddr += 18;
				}
			}
		}
	}
	#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	extern void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid);
	DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_0, MT_TRUE);
	#endif
}

static void sBin2CecAddr( SiiLibEdidCecAddr_t* pCecAddr, uint8_t* pData )
{
	pCecAddr->sub[0] = (pData[0] >> 4) & 0xF;
	pCecAddr->sub[1] = (pData[0] >> 0) & 0xF;
	pCecAddr->sub[2] = (pData[1] >> 4) & 0xF;
	pCecAddr->sub[3] = (pData[1] >> 0) & 0xF;
}

void sParseEdidErrInit(SiiLibEdidPar_t* poEdidPar)
{
	if ( poEdidPar->version == 0 ) {
		poEdidPar->version = 0x0103;
	}
	if ( poEdidPar->sadb.size == 0 ) {
		poEdidPar->sadb.size 		= 1;
		poEdidPar->sadb.SpD[0].p[0] = 1;
	}

	if ( poEdidPar->adb.size == 0 ) {
		poEdidPar->adb.size			= 1;
		poEdidPar->adb.sad[0].p[0] = 9; //LPCM 2channel
		poEdidPar->adb.sad[0].p[1] = 4; //48KHz
		poEdidPar->adb.sad[0].p[2] = 7; //16-20-24bits
	}

	if ( poEdidPar->vdb.size == 0 ) {
		poEdidPar->vdb.size			= 5;
		poEdidPar->vdb.svd[0].p[0]			= 1; //640x480@60 4:3
		poEdidPar->vdb.svd[2].p[0]			= 4; //1280x720p@60 16:9
		poEdidPar->vdb.svd[1].p[0]			= 5; //1920x1080i@60 16:9
		poEdidPar->vdb.svd[3].p[0]			= 19; //1280x720p@50 16:9
		poEdidPar->vdb.svd[4].p[0]			= 20; //1920x1080i@50 16:9
		#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
		sParseEdidSaveFmtByVdb(poEdidPar);
		#endif
	}

	if ( poEdidPar->ieee_id == 0 ) {
		poEdidPar->ieee_id = 0x000C03;
	}

	if ( poEdidPar->maxTmds == 0 ) {
		poEdidPar->maxTmds			= 290000000;
	}

	if ( poEdidPar->audInfo.audformat_cnt == 0 ) {
		poEdidPar->audInfo.audformat_cnt = 1;
		poEdidPar->audInfo.audformat[0] = 1;//LPCM
		poEdidPar->audInfo.audfs[0] = 4;//48KHz
		poEdidPar->audInfo.audlen[0] = 7;//16-20-24bits
		poEdidPar->audInfo.audchannel[0] = 2;//2 channels
		poEdidPar->audInfo.speakerformat = 1;//FL/FR
	}
#if ( __HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__ )
	extern void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid);
	DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_0, MT_TRUE);
#endif
}

/** END of File *********************************************************/
