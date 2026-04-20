/*************************************************************************
* si_tx_api_testing.c
*************************************************************************/

/***** #include statements ***************************************************/
#include <conio.h>
#include <pthread.h>
#include "si_datatypes.h"
#include "si_drv_tx_api.h"
#include "si_app_tx_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_seq_api.h"
#include "si_lib_malloc_api.h"
#include "si_lib_log_api.h"
#include "sii_time.h"
#include "si_tx_api_testing_api.h"
#include "si_test_set_infoframes_api.h"
#if (MT_SDK_COMPILE_HDMI20 == 0)
	#include "si_siimon_handle.h"
#endif
#include "si_drv_pebbles_api.h"
#include "si_app_cec.h"
#include "si_mod_tx_scdc_api.h"

#include "mt_hdmi20_testcase.h"
#include "si_drv_tx_regs.h"
/***** Register Module name **************************************************/

SII_LIB_OBJ_MODULE_DEF(api_test);

/***** local type definitions ************************************************/
#define AUDIO_FORMAT_1CH	0x01
#define AUDIO_FORMAT_2CH	0x02
#define AUDIO_FORMAT_3CH	0x03
#define AUDIO_FORMAT_4CH	0x04
#define AUDIO_FORMAT_5CH	0x05
#define AUDIO_FORMAT_6CH	0x06
#define AUDIO_FORMAT_7CH	0x07
#define AUDIO_FORMAT_8CH	0x08

typedef struct {
	uint16_t         emscFloodTestDataLen;
	uint8_t          *pEmscFloodData; //max 255 bytes of data and 1 byte of count (which sahll be added by HW) totalling to 256 bytes
	uint16_t         nextEmscDataIndex;
} emscFloodData_t;

typedef struct {
	TestModulesInstInfo_t *pInfo;
	SiiInst_t        instTimer;
	SiiPlatformInterface_t	*pInterface;
	emscFloodData_t	emscFloodData;
} ApiTestObj_t;

static mt_u32 g_tvsys_thread_sts = 0;

static TVSYSSWITCH_T g_tvsys_data = {0};

/***** global prototypes *****************************************************/
extern bool_t sequencerLoop;
/***** local prototypes ******************************************************/
static void sApiTestSelect(SiiInst_t inst);
static void sPrintEdid(uint8_t *pData, uint16_t len);
static void sPrintKsvList(SiiDrvHdcpKsvList_t ksvList);

static void TestRegisterCallBack(void);
static void TestCreate(void);
static void TestDelete(void);
static void TestEdidGet(void);
static void TestHotPlugGet(void);
static void TestRsenGet(void);
static void TestTmdsModeSet(void);
static void TestTmdsModeStatusGet(void);
static void TestCecPhysicalAddrGet(void);
static void TestHvSyncPolaritySet(void);
static void TestAvMuteSet(void);
static void TestInfoframeSet(void);
static void TestInfoframeOnOffSet(void);
static void TestChannelStatusSet(void);
static void TestAudioFormatGet(void);
static void TestAudioFormatSet(void);
static void TestHdcpProtectionSet(void);
static void TestHdcpStatusGet(void);
static void TestHdcpBksvListGet(void);
static void TestHdcpBksvListApprovalSet(void);
static void TestHdcpTopologyGet(void);
static void TestHdcp2ContentTypeSet(void);
static void TestVidPathInputColorInfoConfig(void);
static void TestVidPathOutputColorSpaceSet(void);
//static void TestVidPathOutputBitDepthSet(void);
static void TestTxOutputBitDepthSet(void);
//static void TestSiIMonRegisterHandlingStart(SiiInst_t inst);
#if (MT_SDK_COMPILE_HDMI20 == 0)
	static void TestSiIMonRegisterHandlingStop(SiiInst_t inst);
#endif
static void TestProgramPebbles(SiiInst_t inst);
static void TestCecOneTouchPlay(void);
void TestTpgEnable(SiiInst_t inst);
void TestTpgFormatSet(SiiInst_t inst);
void TestTpgPatternSet(SiiInst_t inst);
static void ScdcReadRequestTest(void);
static void ScdcScrambleEnable(void);
void MoreApis(SiiInst_t inst, uint8_t level);
void SiiApiTestInitSetting(void);

/***** local data objects ****************************************************/
static ApiTestObj_t *spApiTestObj = NULL;

void SiiApiTestModuleCreate(SiiPlatformInterface_t *pInterface, TestModulesInstInfo_t *pInfo )
{
	spApiTestObj = (ApiTestObj_t *)SII_LIB_OBJ_CREATE("ApiTestModule", sizeof(ApiTestObj_t));
	SII_PLATFORM_DEBUG_ASSERT(spApiTestObj);

	spApiTestObj->pInfo = (TestModulesInstInfo_t*)SiiLibMallocCreate(sizeof(TestModulesInstInfo_t));
	if ( spApiTestObj->pInfo ) {
		SII_MEMCPY(spApiTestObj->pInfo, pInfo, sizeof(TestModulesInstInfo_t));
	} else {
		SII_LIB_LOG_PRINT2(("SiiApiTestModuleCreate Malloc info fail\n"));
		return;
	}

	spApiTestObj->pInterface = pInterface;

	// Allocate timer handler
	spApiTestObj->instTimer = SII_LIB_SEQ_TIMER_CREATE("ApiTestSelect", sApiTestSelect, SII_LIB_OBJ_INST(spApiTestObj), 251);
	SII_PLATFORM_DEBUG_ASSERT(spApiTestObj->instTimer);

	SiiLibSeqTimerStart(spApiTestObj->instTimer, 50 /*Sii_Get_Process_Time()*/, 10);
}

void SiiApiTestModuleDelete( void )
{
	//Delete in the Reverse Order of Creation
	SiiLibSeqTimerDelete(spApiTestObj->instTimer);
	SiiLibMallocDelete(spApiTestObj->pInfo);
	SII_LIB_OBJ_DELETE(spApiTestObj);
}

/***** local functions *******************************************************/
static void sPrintEdid(uint8_t *pData, uint16_t len)
{
	uint8_t i = 0, blkno = 0;
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	while (len--) {
		if ( !((len + 1) % 0x80) ) {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("EDID Block %d :\n", blkno++));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
		}
		SII_LIB_LOG_PRINT2((" %02X", *pData));
		pData++;
		if (++i == 0x10) {
			SII_LIB_LOG_PRINT2(("\n"));
			i = 0;
		}
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

static void sPrintKsvList(SiiDrvHdcpKsvList_t ksvList)
{
	uint8_t		*ptrTemp;
	uint16_t	length;

	length = ksvList.length;
	ptrTemp = ksvList.pListStart;
	while (length--) {
		SII_LIB_LOG_PRINT2((" %02X", *ptrTemp));
		ptrTemp++;
	}

}

static void TestRegisterCallBack(void)
{
	//Define your own callback function and then register.
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	if (spApiTestObj->pInfo->instTx) {
		SII_LIB_LOG_PRINT2(("Already Registered SiiTxCallBack as callback function\n"));
	} else {
		SII_LIB_LOG_PRINT2(("No Tx Instance Created\n"));
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));

}

static void TestCreate(void)
{
	//Tx Object must have already been created...
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	if (spApiTestObj->pInfo->instTx) {
		SII_LIB_LOG_PRINT2(("Done\n"));
	} else {
		SII_LIB_LOG_PRINT2(("Unable to create Tx Instance... Restart the Application\n"));
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

static void TestDelete(void)
{
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	if (spApiTestObj->pInfo->instTx) {
		SiiDrvTxDelete(spApiTestObj->pInfo->instTx);
		spApiTestObj->pInfo->instTx = 0;
		SII_LIB_LOG_PRINT2(("Done... Close the Application\n"));
	} else {
		SII_LIB_LOG_PRINT2(("No Tx Instance to delete... Restart the Application\n"));
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

static void TestEdidGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiEdid_t pEdid;
		uint8_t extensions = 0;
		SII_MEMSET(&pEdid, 0, sizeof(SiiEdid_t));
		SiiDrvTxEdidGet(spApiTestObj->pInfo->instTx, &pEdid);
		extensions = pEdid.b[0x7E];
		sPrintEdid((uint8_t*)&pEdid, (1 + extensions) * SII_EDID_BLOCK_SIZE);
		{
			FILE *pfile;
			mt_u32 wlen;
			char path[100];

			SiiLibEdidPar_t ppEdid = {0};
			SiiDrvTxEdidParseGet(spApiTestObj->pInfo->instTx, &ppEdid);
			for (int i=0;i<7;i++) {
				ppEdid.pMonitorNameStr[7+i] = 0;
			}
			sprintf(path, "%s_%s_%d_%d_%x_%s", "/media/casetest/EDID_", ppEdid.maninfo.man_name, ppEdid.maninfo.year, ppEdid.maninfo.week, ppEdid.maninfo.serialnum, ppEdid.pMonitorNameStr);
			pfile = fopen(path, "wb");
			if (pfile) {
				wlen = (mt_u32)fwrite(pEdid.b, 1, (1 + extensions) * SII_EDID_BLOCK_SIZE, pfile);
				fclose(pfile);
				system("sync");
				SII_LIB_LOG_PRINT2(("Edid len:%s,%d\n", path, wlen));
			}
		}
	}
}

static void TestHotPlugGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		bool_t hotPlug = SiiDrvTxHotPlugStatusGet(spApiTestObj->pInfo->instTx);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("HPD - %s\n", hotPlug ? "ON" : "OFF"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestRsenGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		bool_t rsen = SiiDrvTxRsenStatusGet(spApiTestObj->pInfo->instTx);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("RSENSE - %s\n", rsen ? "HIGH" : "LOW"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}

}

static void ScdcEnableAutoPoll(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {

		char key;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Autp polling Enable or Disable :\n	1 - Enable\n	0- Disable\n"));
		while (!_kbhit())
			;
		key = (char)_getch();
		if (key < '0' || key > '1') {
			SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}
		SiiDrvTxScdcAutopollEnable(spApiTestObj->pInfo->instTx, ( (key == '1') ? 1 : 0));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}
}

static void ScdcScrambleEnable(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		SiiDrvTxScdcSinKCaps_t  scdc;

		memset(&scdc, 0, sizeof(SiiLibScdcSinKCaps_t));
		scdc.bLTE340MscsScramble = 1;
		scdc.bScdcPresent = 1;

		SiiDrvTxScdcScrambleenable(spApiTestObj->pInfo->instTx, &scdc);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}
}

static void ScdcReadRequestTest(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		char msdelay = 5;
		SiiDrvTxScdcReadRequestTest(spApiTestObj->pInfo->instTx, msdelay);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}

}

static void ScdcReadManufacturerRegister(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		SiiDrvTxScdcManufacturerStatus_t peerManufStatus;
		SiiDrvTxScdcManufacturerRegisterstatus(spApiTestObj->pInfo->instTx, &peerManufStatus);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}
}

static void ScdcReadScrambleAndClock(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		SiiDrvTxScdcScrmbleclkStatus_t scrambleclkStatus;
		SiiDrvTxScdcScrambleAndClockstatus(spApiTestObj->pInfo->instTx, &scrambleclkStatus);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}
}

static void ScdcReadStatusRegister(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		SiiDrvTxScdcRegisterStatus_t peerRegisterStatus;
		SiiDrvTxScdcPeerRegisterstatus(spApiTestObj->pInfo->instTx, &peerRegisterStatus);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));

	}
}

static void ScdcResetUpdateRegisters(void)
{
	if (spApiTestObj->pInfo->bScDcEn) {
		//SiiDrvTxScdcPeerUpdateRegisters_t resetupdateregister = 0xff;

		SiiDrvTxScdcResetUpdateRegisters(spApiTestObj->pInfo->instTx);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}

}

static void TestTmdsModeSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Set TMDS Mode to :\n"));
		SII_LIB_LOG_PRINT2(("	0 - NONE\n	1 - DVI\n	2 - HDMI1\n	3 - HDMI2\n	4 - AUTO\n"));
		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '0' || key > '4') {
			SII_LIB_LOG_PRINT2(("Invalid input... returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}

		switch (key) {
			case '0':
				SiiDrvTxTmdsModeSet(spApiTestObj->pInfo->instTx, SII_TMDS_MODE__NONE);
				break;
			case '1':
				SiiDrvTxTmdsModeSet(spApiTestObj->pInfo->instTx, SII_TMDS_MODE__DVI);
				break;
			case '2':
				SiiDrvTxTmdsModeSet(spApiTestObj->pInfo->instTx, SII_TMDS_MODE__HDMI1);
				break;
			case '3':
				SiiDrvTxTmdsModeSet(spApiTestObj->pInfo->instTx, SII_TMDS_MODE__HDMI2);
				break;
			case '4':
				SiiDrvTxTmdsModeSet(spApiTestObj->pInfo->instTx, SII_TMDS_MODE__AUTO);
				break;
		}
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestTmdsModeStatusGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiTmdsMode_t tmdsMode = SiiDrvTxTmdsModeStatusGet(spApiTestObj->pInfo->instTx);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("TMDS Mode : "));
		switch (tmdsMode) {
			case 0:
				SII_LIB_LOG_PRINT2(("TMDS_MODE__NONE\n"));
				break;
			case 1:
				SII_LIB_LOG_PRINT2(("TMDS_MODE__DVI\n"));
				break;
			case 2:
				SII_LIB_LOG_PRINT2(("TMDS_MODE__HDMI1\n"));
				break;
			case 3:
				SII_LIB_LOG_PRINT2(("TMDS_MODE__HDMI2\n"));
				break;
			case 4:
				SII_LIB_LOG_PRINT2(("TMDS_MODE__AUTO\n"));
				break;
		}
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestCecPhysicalAddrGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		uint16_t cecAddr = SiiDrvTxCecPhysicalAddrGet(spApiTestObj->pInfo->instTx);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		if (cecAddr) {
			SII_LIB_LOG_PRINT2(("CEC Physical Address : %d.%d.%d.%d\n", ((cecAddr & 0xF000) >> 12),
								((cecAddr & 0x0F00) >> 8), ((cecAddr & 0x00F0) >> 4), (cecAddr & 0x0F)));
		} else {
			SII_LIB_LOG_PRINT2(("No Downstream Cable Connected\n"));
		}

		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestHvSyncPolaritySet(void)
{
	char key;
	SiiHvSyncPol_t hvSyncPol;
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Set HV_SYNC Polarity:\n	0 - SII_HV_SYNC_POL__HPVP\n	1 - SII_HV_SYNC_POL__HPVN\n	2 - SII_HV_SYNC_POL__HNVP\n	3 - SII_HV_SYNC_POL__HNVN\n"));
	while (!_kbhit())
		;
	key = (char)_getch();
	if (key < '0' || key > '3') {
		SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		return;
	}
	switch (key) {
		case '0':
			hvSyncPol = SII_HV_SYNC_POL__HPVP;
			break;
		case '1':
			hvSyncPol = SII_HV_SYNC_POL__HPVN;
			break;
		case '2':
			hvSyncPol = SII_HV_SYNC_POL__HNVP;
			break;
		case '3':
			hvSyncPol = SII_HV_SYNC_POL__HNVN;
			break;
	}
	SiiDrvTxHvSyncPolaritySet(spApiTestObj->pInfo->instTx, &hvSyncPol);
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

static void TestAvMuteSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Set AVMute to :\n	0 - OFF\n	1- On\n"));
		while (!_kbhit())
			;
		key = (char)_getch();
		if (key < '0' || key > '1') {
			SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}
		SiiDrvTxAvMuteSet(spApiTestObj->pInfo->instTx, (key == '0') ? 0 : 1);
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestInfoframeSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Choose Infoframe to set :\n"));
		SII_LIB_LOG_PRINT2(("	0 - SII_INFO_FRAME_ID__AVI\n"));
		SII_LIB_LOG_PRINT2(("	1 - SII_INFO_FRAME_ID__AUDIO\n"));
		SII_LIB_LOG_PRINT2(("	2 - SII_INFO_FRAME_ID__VS\n"));
		SII_LIB_LOG_PRINT2(("	3 - SII_INFO_FRAME_ID__SPD\n"));
		SII_LIB_LOG_PRINT2(("	4 - SII_INFO_FRAME_ID__GBD\n"));
		SII_LIB_LOG_PRINT2(("	5 - SII_INFO_FRAME_ID__MPEG\n"));
		SII_LIB_LOG_PRINT2(("	6 - SII_INFO_FRAME_ID__ISRC\n"));
		SII_LIB_LOG_PRINT2(("	7 - SII_INFO_FRAME_ID__ISRC2\n"));
		SII_LIB_LOG_PRINT2(("	8 - SII_INFO_FRAME_ID__ACP\n"));
		SII_LIB_LOG_PRINT2((" 9 - SII_INFO_FRAME_ID__HDR\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '0' || key > '9') {
			SII_LIB_LOG_PRINT2(("Invalid Infoframe ID...returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}

		switch (key) {
			case '0':
				SetAVIF(1, spApiTestObj->pInfo->instTx);
				break;
			case '1':
				SetAudioIF(spApiTestObj->pInfo->instTx);
				break;
			case '2':
				SetVSIF(spApiTestObj->pInfo->instTx);
				break;
			case '9':
				SetHDRIF(spApiTestObj->pInfo->instTx);
				break;
			default:
				SII_LIB_LOG_PRINT2(("                                                     \n"));
				SII_LIB_LOG_PRINT2(("Not Yet Implemented for input : %c\n", key));
		}
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestInfoframeOnOffSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		uint8_t ifId = 0, ifOnOff = 0;

		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Choose Infoframe ID :\n"));
		SII_LIB_LOG_PRINT2(("	0 - SII_INFO_FRAME_ID__AVI\n	1 - SII_INFO_FRAME_ID__AUDIO\n	2 - SII_INFO_FRAME_ID__VS\n	3 - SII_INFO_FRAME_ID__SPD\n"));
		SII_LIB_LOG_PRINT2(("	4 - SII_INFO_FRAME_ID__GBD\n	5 - SII_INFO_FRAME_ID__MPEG\n	6 - SII_INFO_FRAME_ID__ISRC\n	7 - SII_INFO_FRAME_ID__ISRC2\n"));
		SII_LIB_LOG_PRINT2(("	8 - SII_INFO_FRAME_ID__GCP\n	9 - SII_INFO_FRAME_ID__ACP\n	a/A - SII_INFO_FRAME_ID__HDR\n"));

		while (!_kbhit())
			;
		key = (char)_getch();
		if (key < '0' || key > '9') {
			if (key != 'a' && key != 'A') {
				SII_LIB_LOG_PRINT2(("                                                     \n"));
				SII_LIB_LOG_PRINT2(("Invalid Infoframe ID... returning\n"));
				SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
				SII_LIB_LOG_PRINT2(("                                                     \n"));
				return;
			}
		}
		if (key == 'a' || key == 'A') {
			//HDR
			ifId = 10;
		} else {
			ifId = (uint8_t)atoi(&key);
		}
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Select\n	0 - OFF\n	1 - On	: "));
		while (!_kbhit())
			;
		key = (char)_getch();
		if (key < '0' || key > '1') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}
		ifOnOff = (uint8_t)atoi(&key);

		SiiDrvTxInfoframeOnOffSet(spApiTestObj->pInfo->instTx, (SiiInfoFrameId_t)ifId, ifOnOff);
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestChannelStatusSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		{
			SiiChannelStatus_t pChannelStatus;
			pChannelStatus.i2s_chst0 = 0x00;
			pChannelStatus.i2s_chst1 = 0x00;
			pChannelStatus.i2s_chst2 = 0x00;
			pChannelStatus.i2s_chst3 = 0x02;
			pChannelStatus.i2s_chst4 = 0x0B;
			pChannelStatus.i2s_chst5 = 0x00;
			pChannelStatus.i2s_chst6 = 0x00;

			SII_LIB_LOG_PRINT2(("Applying default values... Refer Tx AIP Registry Map for more information\n"));
			SiiDrvTxChannelStatusSet(spApiTestObj->pInfo->instTx, &pChannelStatus);

		}
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestAudioFormatGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiAudioFormat_t audioFormat;
		SiiDrvTxAudioFormatStatusGet(spApiTestObj->pInfo->instTx, &audioFormat);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Audio Sampling Frequency : "));
		switch (audioFormat.audioFs) {
			case SII_AUDIO_FS__22_05KHZ:
				SII_LIB_LOG_PRINT2(("22_05KHZ"));
				break;
			case SII_AUDIO_FS__24KHZ:
				SII_LIB_LOG_PRINT2(("24KHZ"));
				break;
			case SII_AUDIO_FS__32KHZ:
				SII_LIB_LOG_PRINT2(("32KHZ"));
				break;
			case SII_AUDIO_FS__44_1KHZ:
				SII_LIB_LOG_PRINT2(("44_1KHZ"));
				break;
			case SII_AUDIO_FS__48KHZ:
				SII_LIB_LOG_PRINT2(("48KHZ"));
				break;
			case SII_AUDIO_FS__88_2KHZ:
				SII_LIB_LOG_PRINT2(("88_2KHZ"));
				break;
			case SII_AUDIO_FS__96KHZ:
				SII_LIB_LOG_PRINT2(("96KHZ"));
				break;
			case SII_AUDIO_FS__176_4KHZ:
				SII_LIB_LOG_PRINT2(("176KHZ"));
				break;
			case SII_AUDIO_FS__192KHZ:
				SII_LIB_LOG_PRINT2(("192KHZ"));
				break;
			case SII_AUDIO_FS__768KHZ:
				SII_LIB_LOG_PRINT2(("768KHZ"));
				break;
			case SII_AUDIO_FS__64KHZ:
				SII_LIB_LOG_PRINT2(("64KHZ"));
				break;
			case SII_AUDIO_FS__128KHZ:
				SII_LIB_LOG_PRINT2(("128KHZ"));
				break;
			default:
				break;
		}
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Audio Channel Status : %d CH\n\n", audioFormat.layout1));
		SII_LIB_LOG_PRINT2(("Down Sampling: %s \n", audioFormat.downSample ? "Enabled" : "Disabled"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestAudioFormatSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		uint8_t audioFs, layout;
		SiiAudioFormat_t audioFormat;
		/*	audioFormat.audioFs = SII_AUDIO_FS__48KHZ;
		audioFormat.layout1 = AUDIO_FORMAT_2CH;
		audioFormat.spdif = true;
		audioFormat.dsd = false;
		audioFormat.hbrA = false;
		*/
		SiiDrvTxAudioFormatStatusGet(spApiTestObj->pInfo->instTx, &audioFormat);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Set Sampling Frequency to : \n"));
		SII_LIB_LOG_PRINT2(("	0 - 22_05KHZ\n	1 - 24KHZ\n	2 - 32KHZ\n	3 - 44_1KHZ\n	4 - 48KHZ\n"	\
							"	5 - 88_2KHZ\n	6 - 96KHZ\n	7 - 176_4KHZ\n	8 - 192KHZ\n	9 - 768KHZ\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '0' || key > '9') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid Sampling Frequency... setting default\n"));
		} else {
			audioFs = (uint8_t)atoi(&key);
			audioFormat.audioFs = (SiiAudioFs_t)audioFs;
		}

		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Set Audio Format to : \n"));
		SII_LIB_LOG_PRINT2(("	0 - SPDIF\n	1 - I2S\n	2 - HBRA\n	3 - DSD\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '0' || key > '3') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid Audio Format... setting default\n"));
		} else {
			audioFs = (uint8_t)atoi(&key);
			switch (audioFs) {
				case 0:
					audioFormat.spdif = (uint8_t)true;
					break;
				case 1:
					audioFormat.i2s = (uint8_t)true;
					break;
				case 2:
					audioFormat.hbrA = (uint8_t)true;
					break;
				case 3:
					audioFormat.dsd = (uint8_t)true;
					break;
				default:
					break;
			}
		}

		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Set Channel Layout to :\n"));
		SII_LIB_LOG_PRINT2(("	1 - 1CH\n	2 - 2CH\n	3 - 3CH\n	4 - 4CH\n	5 - 5CH\n	6 - 6CH\n	7 - 7CH\n	8 - 8CH\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '1' || key > '8') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid Channel Layout... setting default\n"));
		} else {
			layout = (uint8_t)atoi(&key);
			audioFormat.layout1 = layout;
		}

		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Enable/Disable Audio Downsampling  : \n"));
		SII_LIB_LOG_PRINT2(("	0 - Disable\n	1 - Enable\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key < '0' || key > '1') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid key... setting default\n"));
		} else {
			audioFs = (uint8_t)atoi(&key);
			audioFormat.downSample = (uint8_t)audioFs;
		}

		SiiDrvTxAudioFormatSet(spApiTestObj->pInfo->instTx, &audioFormat);
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestHdcpProtectionSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("HDCP Protection Set\n	0- OFF\n	1- ON\n"));
		while (!_kbhit())
			;
		key = (char)_getch();
		if (key < '0' || key > '1') {
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			return;
		}
		SiiDrvTxHdcpProtectionSet(spApiTestObj->pInfo->instTx, (key == '0') ? 0 : 1);
		SII_LIB_LOG_PRINT2(("HDCP Protection :: %s\n", (key == '0') ? "Disabled" : "Enabled"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestHdcpStatusGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiDrvHdcpStatus_t	hdcpStatus;
		SiiDrvTxHdcpStateStatusGet(spApiTestObj->pInfo->instTx, &hdcpStatus);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("HDCP Status : "));
		switch (hdcpStatus) {
			case SII_DRV_HDCP_STATUS__OFF:
				SII_LIB_LOG_PRINT2(("HDCP_STATUS__OFF\n"));
				break;
			case SII_DRV_HDCP_STATUS__SUCCESS_1X:
				SII_LIB_LOG_PRINT2(("HDCP_STATUS__SUCCESS_1X\n"));
				break;
			case SII_DRV_HDCP_STATUS__SUCCESS_22:
				SII_LIB_LOG_PRINT2(("HDCP_STATUS__SUCCESS_22\n"));
				break;
			case SII_DRV_HDCP_STATUS__AUTHENTICATING:
				SII_LIB_LOG_PRINT2(("HDCP_STATUS__AUTHENTICATING\n"));
				break;
			case SII_DRV_HDCP_STATUS__FAILED:
				SII_LIB_LOG_PRINT2(("HDCP_STATUS__FAILED\n"));
				break;
		}
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}

}

static void TestHdcpBksvListGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiDrvHdcpKsvList_t 	bksvList;
		SiiDrvTxHdcpKsvListGet(spApiTestObj->pInfo->instTx, &bksvList);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		if (bksvList.length) {
			SII_LIB_LOG_PRINT2(("Sink BKSVs :: "));
			sPrintKsvList(bksvList);
		} else {
			SII_LIB_LOG_PRINT2(("Error reading BKSV List from downstream device"));
		}
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}

}
static void TestHdcpBksvListApprovalSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		TestHdcpBksvListGet();
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Acknowledge BKSV List :\n	0 - NO\n	1 - YES\n"));
		while (_kbhit());
		key = (char)_getch();

		if (key != '0' && key != '1') {
			SII_LIB_LOG_PRINT2(("Invalid Input... Setting as YES\n"));
		}

		SiiDrvTxHdcpKsvListApprovalSet(spApiTestObj->pInfo->instTx, (key == '0') ? false : true);
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}
static void TestHdcpTopologyGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiDrvHdcpTopology_t pTopology;
		SiiDrvTxHdcpTopologyGet(spApiTestObj->pInfo->instTx,  &pTopology);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("HDCP Topology :\n"));
		SII_LIB_LOG_PRINT2(("	DS Device Count = %d\n	DS Depth = %d\n	Max DS Devices Exceeded : %s\n",
							pTopology.deviceCount, pTopology.depth, pTopology.maxDevsExceeded ? "YES" : "NO"));
		SII_LIB_LOG_PRINT2(("	Max Cascade Exceeded : %s\n	Any HDCP2.0 Repeater : %s\n	Any HDCP1.0 Repeater : %s\n",
							pTopology.maxCascadeExceeded ? "YES" : "NO", pTopology.hdcp20RepeaterDs ? "YES" : "NO", pTopology.hdcp1xRepeaterDs ? "YES" : "NO"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestHdcpReauthSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiDrvTxHdcpReauth(spApiTestObj->pInfo->instTx,  true);
		SII_LIB_LOG_PRINT2(("Manual HDCP Reauth Done                                                     \n"));
	}
}

static void TestHdcp2ContentTypeSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SiiDrvHdcpContentType_t pContentType;
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Set Content Type to :\n"));
		SII_LIB_LOG_PRINT2(("	0 - CONTENT_TYPE_0\n	1 - CONTENT_TYPE_1\n"));
		while (!_kbhit()) {;}
		key = (char)_getch();
		if (key != '0' && key != '1') {
			SII_LIB_LOG_PRINT2(("Invalid Input... returning\n"));
			return;
		}

		if (key == '0') {
			pContentType = SII_DRV_HDCP_CONTENT_TYPE__0;
		} else {
			pContentType = SII_DRV_HDCP_CONTENT_TYPE__1;
		}

		SiiDrvTxHdcp2ContentTypeSet(spApiTestObj->pInfo->instTx, &pContentType);
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestVidPathInputColorInfoConfig(void)
{
	SiiDrvTxColorInfoCfg_t clrInfo;

	char key;
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select Input color space:\n"));
	SII_LIB_LOG_PRINT2(("0 - \n1 - YC444_601\n2 - YC422_601\n3 - YC420_601\n"));
	SII_LIB_LOG_PRINT2(("4 - YC444_709\n5 - YC422_709\n6 - YC420_709\n"));
	SII_LIB_LOG_PRINT2(("7 - XVYCC444_601\n8 - XVYCC422_601\n9 - XVYCC420_601\n"));
	SII_LIB_LOG_PRINT2(("a - XVYCC444_709\nb - XVYCC422_709\nc - XVYCC420_709\n"));
	SII_LIB_LOG_PRINT2(("d - YC444_2020\ne - YC422_2020\nf - YC420_2020\n"));
	SII_LIB_LOG_PRINT2(("g - RGB_FULL\nh - RGB_LIMITED\n"));
	while ( !_kbhit() ) {
		;
	}
	key = (char)_getch();
	switch (key) {
		case '0':
			SII_LIB_LOG_PRINT2(("Invalid Color Space... Returning..\n"));
			return;
		case '1':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC444_601;
			break;
		case '2':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC422_601;
			break;
		case '3':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC420_601;
			break;
		case '4':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC444_709;
			break;
		case '5':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC422_709;
			break;
		case '6':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC420_709;
			break;
		case '7':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC444_601;
			break;
		case '8':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC422_601;
			break;
		case '9':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC420_601;
			break;
		case 'a':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC444_709;
			break;
		case 'b':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC422_709;
			break;
		case 'c':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__XVYCC420_709;
			break;
		case 'd':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC444_2020;
			break;
		case 'e':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC422_2020;
			break;
		case 'f':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__YC420_2020;
			break;
		case 'g':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__RGB_FULL;
			break;
		case 'h':
			clrInfo.inputClrSpc = SII_DRV_CLRSPC__RGB_LIMITED;
			break;
		default:
			return;
	}

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select Input Color Depth:\n"));
	SII_LIB_LOG_PRINT2(("0 - 8 bit\n1 - 10 bit\n2 - 12 bit\n"));

	while (!_kbhit()) {;}
	key = (char)_getch();

	switch (key) {
		case '0':
			clrInfo.inputVidDcDepth = SII_DRV_BIT_DEPTH__8_BIT;
			break;
		case '1':
			clrInfo.inputVidDcDepth = SII_DRV_BIT_DEPTH__10_BIT;
			break;
		case '2':
			clrInfo.inputVidDcDepth = SII_DRV_BIT_DEPTH__12_BIT;
			break;
		case '3':
			clrInfo.inputVidDcDepth = SII_DRV_BIT_DEPTH__16_BIT;
			break;
		default:
			SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
			return;
	}

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select Input Video Convertion Standard:\n"));
	SII_LIB_LOG_PRINT2(("0 - BT 709\n1 - BT 601\n2 - BT 2020 non const luminous\n3 - BT 2020 const luminous\n"));

	while (!_kbhit()) {;}
	key = (char)_getch();

	switch (key) {
		case '0':
			clrInfo.inputClrConvStd = SII_DRV_CONV_STD__BT_709;
			break;
		case '1':
			clrInfo.inputClrConvStd = SII_DRV_CONV_STD__BT_601;
			break;
		case '2':
			clrInfo.inputClrConvStd = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
			break;
		case '3':
			clrInfo.inputClrConvStd = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
			break;
		default:
			SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
			return;
	}

	SiiDrvTxColorInfoConfig(spApiTestObj->pInfo->instTx, &clrInfo);
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Done\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

static void TestVidPathOutputColorSpaceSet(void)
{
	SiiDrvClrSpc_t clrSpc;

	char key;
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select Output color space:\n"));
	SII_LIB_LOG_PRINT2(("0 - PASSTHRU\n1 - YC444_601\n2 - YC422_601\n3 - YC420_601\n"));
	SII_LIB_LOG_PRINT2(("4 - YC444_709\n5 - YC422_709\n6 - YC420_709\n"));
	SII_LIB_LOG_PRINT2(("7 - XVYCC444_601\n8 - XVYCC422_601\n9 - XVYCC420_601\n"));
	SII_LIB_LOG_PRINT2(("a - XVYCC444_709\nb - XVYCC422_709\nc - XVYCC420_709\n"));
	SII_LIB_LOG_PRINT2(("d - YC444_2020\ne - YC422_2020\nf - YC420_2020\n"));
	SII_LIB_LOG_PRINT2(("g - RGB_FULL\nh - RGB_LIMITED\n"));
	while ( !_kbhit() ) {
		;
	}
	key = (char)_getch();
	switch (key) {
		case '0':
			clrSpc = SII_DRV_CLRSPC__PASSTHRU;
			break;
		case '1':
			clrSpc = SII_DRV_CLRSPC__YC444_601;
			break;
		case '2':
			clrSpc = SII_DRV_CLRSPC__YC422_601;
			break;
		case '3':
			clrSpc = SII_DRV_CLRSPC__YC420_601;
			break;
		case '4':
			clrSpc = SII_DRV_CLRSPC__YC444_709;
			break;
		case '5':
			clrSpc = SII_DRV_CLRSPC__YC422_709;
			break;
		case '6':
			clrSpc = SII_DRV_CLRSPC__YC420_709;
			break;
		case '7':
			clrSpc = SII_DRV_CLRSPC__XVYCC444_601;
			break;
		case '8':
			clrSpc = SII_DRV_CLRSPC__XVYCC422_601;
			break;
		case '9':
			clrSpc = SII_DRV_CLRSPC__XVYCC420_601;
			break;
		case 'a':
			clrSpc = SII_DRV_CLRSPC__XVYCC444_709;
			break;
		case 'b':
			clrSpc = SII_DRV_CLRSPC__XVYCC422_709;
			break;
		case 'c':
			clrSpc = SII_DRV_CLRSPC__XVYCC420_709;
			break;
		case 'd':
			clrSpc = SII_DRV_CLRSPC__YC444_2020;
			break;
		case 'e':
			clrSpc = SII_DRV_CLRSPC__YC422_2020;
			break;
		case 'f':
			clrSpc = SII_DRV_CLRSPC__YC420_2020;
			break;
		case 'g':
			clrSpc = SII_DRV_CLRSPC__RGB_FULL;
			break;
		case 'h':
			clrSpc = SII_DRV_CLRSPC__RGB_LIMITED;
			break;
		default:
			return;
	}
	SiiDrvTxOutputColorSpaceSet(spApiTestObj->pInfo->instTx, &clrSpc);
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Done\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));

}

static void TestTxOutputBitDepthSet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		char key;
		SiiDrvBitDepth_t bitDepth;

		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Select Tx Output Color Depth:\n"));
		SII_LIB_LOG_PRINT2(("0 - 8 bit\n1 - 10 bit\n2 - 12 bit\n"));

		while (!_kbhit()) {;}
		key = (char)_getch();

		switch (key) {
			case '0':
				bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
				break;
			case '1':
				bitDepth = SII_DRV_BIT_DEPTH__10_BIT;
				break;
			case '2':
				bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
				break;
			case '3':
				bitDepth = SII_DRV_BIT_DEPTH__16_BIT;
				break;
			default:
				SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
				return;
		}

		SiiDrvTxOutputBitDepthSet(spApiTestObj->pInfo->instTx, bitDepth);
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("Done\n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
}

static void TestLipSyncInfoGet(void)
{
	if (spApiTestObj->pInfo->instTx) {
		SiiLipSyncInfo_t lipSyncInfo;
		SiiDrvTxLipSyncInfoGet(spApiTestObj->pInfo->instTx, &lipSyncInfo);
		SII_LIB_LOG_PRINT2(("LipSync Info...\n"));
		if (lipSyncInfo.latencyPresent) {
			SII_LIB_LOG_PRINT2(("	Latency Present :: Video Latency : %d	Audio Latency : %d\n", \
								lipSyncInfo.videoLatency, lipSyncInfo.audioLatency));
		} else {
			SII_LIB_LOG_PRINT2(("	Latency Not Present\n"));
		}
		if (lipSyncInfo.ILatencyPresent) {
			SII_LIB_LOG_PRINT2(("	Interlaced Latency Present :: Interlaced Video Latency : %d	Interlaced Audio Latency : %d\n", \
								lipSyncInfo.IVideoLatency, lipSyncInfo.IAudioLatency));
		} else {
			SII_LIB_LOG_PRINT2(("	Interlaced Latency Not Present\n"));
		}
	}
}

#if (MT_SDK_COMPILE_HDMI20 == 0)
//--------------------------------------------------------------
static void TestSiIMonRegisterHandlingStart(SiiInst_t inst)
{
	char portInfo[20] = {0};
	char portName[15] = {"\\\\.\\COM"};
	char port[3];
	WCHAR wPort[15];
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Enter COM port number (Ex: for COM17, enter 17):\n"));
	gets_s(portInfo, sizeof(portInfo));
	//sscanf_s(portInfo, "%s", port, sizeof(port));
	sscanf_s(portInfo, "%s", port);
	strcat_s(&portName[7], sizeof(port), port);
	swprintf(wPort, 15, L"%hs", portName);
	SII_LIB_LOG_PRINT2(("                                                     \n"));

	if (SiIMonHandleStart(inst, wPort)) {
		SII_LIB_LOG_PRINT2(("Done\n"));
	} else {
		SII_LIB_LOG_PRINT2(("Failed\n"));
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

//--------------------------------------------------------------
static void TestSiIMonRegisterHandlingStop(SiiInst_t inst)
{
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	if (SiIMonHandleStop(inst)) {
		SII_LIB_LOG_PRINT2(("Done\n"));
	} else {
		SII_LIB_LOG_PRINT2(("Failed\n"));
	}
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}
#endif

static void TestProgramPebbles(SiiInst_t inst)
{
	uint8_t regValue = 0;

	if (!SiiDCardRegBlockWrite(inst, 0x60, 0xB5, &regValue, 1)) {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Error:: Programming Pebbles Reg 0x60B5 \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		return;
	} else {
		SII_LIB_LOG_PRINT2(("                                                     \n"));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("Programmed Pebbles Reg 0x60B5 = %2x\n", regValue));
		SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
		SII_LIB_LOG_PRINT2(("                                                     \n"));
	}

	/*
	if (!SiiDCardRegBlockRead(0x60, 0xB5, &regValueRead, 1))
	{
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Error:: Reading Pebbles Reg 0x60B5 \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	}
	else
	{
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Programmed Pebbles Reg 0x60B5 = %2x\n", regValueRead));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	}*/
}

static void TestCecOneTouchPlay(void)
{
	SiiCecOneTouchPlay();

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent One Touch Play Command \n"));
}

static void TestCecSysStandby(void)
{
	SiiCecSysStandby();

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent System Standby Command \n"));
}

static void TestCecRemotePassPress(void)
{
	char key[5];
	uint8_t val;
	SII_LIB_LOG_PRINT2(("													  \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("VOL Control : \n"));
	SII_LIB_LOG_PRINT2(("	65 - Vol+\n	66 - Vol-\n 67 - mute\n "));

	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	val = (uint8_t)atoi(key);
	if (val != CEC_RC_VOLUME_UP && val != CEC_RC_VOLUME_DOWN && val != CEC_RC_MUTE) {
		SII_LIB_LOG_PRINT2(("	error input key:%d\n ", (uint32_t)val));
		return;
	}
	SiiCecRemotePassPress(val);
	SiiCecRemotePassRelease();
	if ( val == CEC_RC_VOLUME_UP || val ==  CEC_RC_VOLUME_DOWN) {
		SiiCecRemotePassPress(val);
		SiiCecRemotePassRelease();
	}

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent Remote Control Pass Through Command \n"));
}

static void TestCecRemotePassAnyPress(void)
{
	char key[5];
	uint8_t val;
	SII_LIB_LOG_PRINT2(("													  \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("	user key\n "));

	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	val = (uint8_t)atoi(key);
	SII_LIB_LOG_PRINT2(("	error input key:%d\n ", (uint32_t)val));
	SiiCecRemotePassPress(val);
	SiiCecRemotePassRelease();

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent Remote Control Pass Through Command %x Done!!!\n", val));
}

static void TestCecGetVersionCmd(void)
{
	//CecTaskGetCECVerison();
	SiiCecVersionGet();

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent Get CEC Version Command \n"));
}

static void TestCecReportCurrentLatency(void)
{
	SiiCecReportCurLatency();

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Sent Report Current Latency Command \n"));
}

static void TestCecSendMsg(void)
{
	SiI_CEC_t cec_frame = {0};
	char key[10];
	uint8_t i;
	char *endptr = NULL;

	SII_LIB_LOG_PRINT2((" set opcode 0~0xff\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		cec_frame.bOpcode = (uint8_t)strtol(&key[2], &endptr, 16);
	} else {
		cec_frame.bOpcode = (uint8_t)atoi(key);
	}

	SII_LIB_LOG_PRINT2((" set dst addr:0~0xf\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		cec_frame.bDestOrRXHeader = (uint8_t)strtol(&key[2], &endptr, 16);
	} else {
		cec_frame.bDestOrRXHeader = (uint8_t)atoi(key);
	}
	if ( cec_frame.bDestOrRXHeader > 0xf ) {
		cec_frame.bDestOrRXHeader = 0xf;
	}

	SII_LIB_LOG_PRINT2((" set operand count :0~0xf\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		cec_frame.bCount = (uint8_t)strtol(&key[2], &endptr, 16);
	} else {
		cec_frame.bCount = (uint8_t)atoi(key);
	}
	if ( cec_frame.bCount > 0xf ) {
		cec_frame.bCount = 0xf;
	}

	for (i = 0; i < cec_frame.bCount; i++) {
		SII_LIB_LOG_PRINT2((" set operand[%d] :0~0xff per opcode\n", i));
		memset(key, 0, sizeof(key));
		while (!_kbhit()) {;}
		gets_s(key, sizeof(key));
		if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
			cec_frame.bOperand[i] = (uint8_t)strtol(&key[2], &endptr, 16);
		} else {
			cec_frame.bOperand[i] = (uint8_t)atoi(key);
		}
	}
	SiiCecSendMsgAck(&cec_frame);

	if (cec_frame.bCount) {
		SiiLibTimeMilliDelay(3000); //wait msg send out
	}
	SII_LIB_LOG_PRINT2(("Sent cec Command:opcode=0x%x,dst=0x%x,opcnt=%d,op[0]=0x%x,op[1]=0x%x done\n", cec_frame.bOpcode, cec_frame.bDestOrRXHeader, cec_frame.bCount, cec_frame.bOperand[0], cec_frame.bOperand[1]));
}

static mt_void SI_wait_hdcp_off_tc(mt_void)
{
	mt_u8 hdcp1x_ctrl = 0;
	mt_u8 hdcp2x_ctrl0 = 0;
	mt_u8 hdcp2x_ctrl1 = 0;
	mt_s32 cnt = 0;
	do {
		hdcp1x_ctrl = SiiDrvCraRdReg8(spApiTestObj->pInfo->instTxCra, REG_ADDR__TPI_INTR_EN);
		hdcp2x_ctrl0 = SiiDrvCraRdReg8(spApiTestObj->pInfo->instTxCra, REG_ADDR__HDCP2X_INTR0_MASK);
		hdcp2x_ctrl1 = SiiDrvCraRdReg8(spApiTestObj->pInfo->instTxCra, REG_ADDR__HDCP2X_INTR1_MASK);
		SiiLibTimeMilliDelay(10);
		cnt++;
	} while ((hdcp1x_ctrl || hdcp2x_ctrl0 || hdcp2x_ctrl1 || SiiDrvTxHdcpSetProtectionDone(spApiTestObj->pInfo->instTx) != 1) && cnt < 200);
	SII_LIB_LOG_PRINT2(("\nhdcp disabled time :%d(ms), 0x%x,hdcp2x:0x%x,0x%x\n", cnt*10, hdcp1x_ctrl,hdcp2x_ctrl0,hdcp2x_ctrl1));
}

#define GET_SCDC_STATUS_TEST (0)
#if GET_SCDC_STATUS_TEST
static void GetScdcStatus(void* arg)
{
	int i;
	int cnt;
	#define GET_SCDC_STS_STEP (300)
	#define GET_SCDC_STS_TIME (3000)
	SiiDrvTxScdcScrmbleclkStatus_t sts;
	int scra0_cnt = 0;
	int scra1_cnt = 0;
	uint8_t reg;
	SiiTmdsMode_t tmds = SII_TMDS_MODE__HDMI1;

	SiiLibTimeMilliDelay(2000); //wait hdcp Authed
	cnt = (int)I2C_ReadBlock(0, 0x40, 0x3900, &reg, 1);
	if ( 1 || reg & 0x01 ) {
		cnt = GET_SCDC_STS_TIME/GET_SCDC_STS_STEP;
		for (i=0;i<cnt;i++){
			memset(&sts, 0, sizeof(SiiDrvTxScdcScrmbleclkStatus_t));
			SiiDrvTxScdcScrambleAndClockstatus(spApiTestObj->pInfo->instTx,&sts);
			if ( tmds != SII_TMDS_MODE__NONE ) {
				tmds = SiiDrvTxTmdsModeStatusGet(spApiTestObj->pInfo->instTx);
			}
			if ( tmds == SII_TMDS_MODE__NONE ) {
				break;
			}
			if (sts.sink_clk_on_off && sts.sink_scramble_on_off) {
				//printf(("wait time %d (ms)\n",GET_SCDC_STS_STEP*i));
				scra1_cnt++;
				SiiLibTimeMilliDelay(GET_SCDC_STS_STEP);
				//break;
			} else {
				//printf(("wait time %d (ms)\n",GET_SCDC_STS_STEP*i));
				scra0_cnt++;
				SiiLibTimeMilliDelay(GET_SCDC_STS_STEP);
			}
		}
		printf("GetScdcStatus: 0cnt:%d, 1cnt:%d, newest:%d\n",scra0_cnt,scra1_cnt,sts.sink_scramble_on_off);
		if ( sts.sink_scramble_on_off != (reg & 0x01) ) {
			SiiLibTimeMilliDelay(500);
			printf("GetScdcStatus: Scdc Scramble status error, exp:%d, act:%d\n",reg & 0x01, sts.sink_scramble_on_off);
			//tmds = SiiDrvTxTmdsModeStatusGet(spApiTestObj->pInfo->instTx);
			if ( tmds == SII_TMDS_MODE__NONE ) {
				printf("Sink triggered hdmi Unplug, No Signal\n");
			} else {
				exit(1);
			}
		}
	}
}
#endif
static void TvSysSwitchThread(void *tvsys)
{
	TVSYSSWITCH_T tvsys_data = {0};
	mt_u32	hdcpStatus;
	SiiDrvHdcpStatus_t	hdcpStatus_tmp;
	//mt_s32 cnt = 100;
	//mt_u32 up_cnt = 0;
	mt_u32 hdcp_off_on = 1;

	g_tvsys_thread_sts = 1;
	tvsys_data = *((TVSYSSWITCH_T*)tvsys);
	SII_LIB_LOG_PRINT2(("[%s_%d] [%d %d %d]Enter...\n", __func__, __LINE__, \
						tvsys_data.tvsys, tvsys_data.first_set, tvsys_data.cs));
	hdcpStatus = (mt_u32)SiiDrvTxHdcpProtectionGet(spApiTestObj->pInfo->instTx);
	//set tvsys
	if (spApiTestObj->pInfo->instTx && hdcp_off_on) {
		SII_LIB_LOG_PRINT2(("HDCP Protection Set 1- OFF\n"));
		SiiDrvTxHdcpProtectionSet(spApiTestObj->pInfo->instTx, 0);
		SI_wait_hdcp_off_tc();
	}

	tvsys_data.instTx = spApiTestObj->pInfo->instTx;
	{
		SiiDrvTxAvMuteSet(spApiTestObj->pInfo->instTx, 1);
		SiiLibTimeMilliDelay(50);
		SiiDrvTxTmdsMute(spApiTestObj->pInfo->instTx, 1);
		set_tvsys_cmd(tvsys_data);
		SiiLibTimeMilliDelay(50);
		SiiDrvTxTmdsMute(spApiTestObj->pInfo->instTx, 0);
		#if GET_SCDC_STATUS_TEST
		{
			int ret = TRUE;
			const pthread_attr_t attribs;
			pthread_attr_init(&attribs);
			pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
			ret = pthread_create(&(g_tvsys_data.task_id), &attribs, (void *)GetScdcStatus, NULL);
			if ( ret ) {
				SiiLibTimeMilliDelay(50);
			}
		}
		#endif
		SiiLibTimeMilliDelay(200);
	}
	if (spApiTestObj->pInfo->instTx && hdcp_off_on) {
		if (hdcpStatus) {
			SII_LIB_LOG_PRINT2(("HDCP Protection Set 1- ON\n"));
			SiiDrvTxHdcpProtectionSet(spApiTestObj->pInfo->instTx, 1);
			#if 1
			SiiDrvTxHdcpStateStatusGet(spApiTestObj->pInfo->instTx, &hdcpStatus_tmp);
			SII_LIB_LOG_PRINT2(("HDCP Protection status:%d\n", hdcpStatus_tmp));
			#endif
		} else {
			SiiDrvTxAvMuteSet(spApiTestObj->pInfo->instTx, 0);
		}
	} else {
		SiiDrvTxAvMuteSet(spApiTestObj->pInfo->instTx, 0);
	}
	SII_LIB_LOG_PRINT2(("[%s_%d] Exit...\n", __func__, __LINE__));
	g_tvsys_thread_sts = 2;
}

static void TestTvSysSwitch(void)
{
	int ret = TRUE;

	while (1 == g_tvsys_thread_sts) {
		SiiLibTimeMilliDelay(1);
	}
	SiiLibTimeMilliDelay(1);
	g_tvsys_thread_sts = 0;
	test_set_tvsys_cmd(&g_tvsys_data);
	#if 1
	const pthread_attr_t attribs;
	pthread_attr_init(&attribs);
	pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&(g_tvsys_data.task_id), &attribs, (void *)TvSysSwitchThread, (void *)(&g_tvsys_data));
	#else
	TvSysSwitchThread((void *)(&g_tvsys_data));
	ret = 0;
	#endif
	if ( 0 == ret) {
		while (0 == g_tvsys_thread_sts) {
			//wait thread work
			SiiLibTimeMilliDelay(1);
		}
		SII_LIB_LOG_PRINT2(("Exit !!!\n"));
	} else {
		SII_LIB_LOG_PRINT2(("Create tvsys switch thread fail!!!\n"));
	}
}

extern byte I2C_ReadBlock_4dump(BB_HANDLE bbHandle, byte deviceID, word regAddr, byte* pData, word length);
static void test_dump_regs(void)
{
#define HDMI20_REG_NUM (4096)
	uint32_t i;
	uint32_t val = 0;
	uint32_t val_1 = 4096;
	uint8_t reg[HDMI20_REG_NUM];

	#if (__HDMI_UBOOT__==0)
	char key[10];
	char *endptr = NULL;
	SII_LIB_LOG_PRINT2(("													  \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Set dump hdmi start address:\n"));

	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		val = (uint32_t)strtol(&key[2], &endptr, 16);
	} else {
		val = (uint32_t)atoi(key);
	}
	SII_LIB_LOG_PRINT2(("Set dump numbers:\n"));

	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));

	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		val_1 = (uint32_t)strtol(&key[2], &endptr, 16);
	} else {
		val_1 = (uint32_t)atoi(key);
	}

	if ( (val_1 + val) >= 4096 ) {
		SII_LIB_LOG_PRINT2(("Set error register dump range 0x%x~0x%x:\n", val, val + val_1));
		return ;
	}
	#endif

	for (i = val; i < (val + val_1); i++) {
		I2C_ReadBlock_4dump(0, 0x40, 0x3000 + i, reg, 1);
		printf("0x%x:   0x%x \n", (unsigned int)HDMI20_REG_BASE_ADDR + i, (unsigned int)reg[0]);
	}
	printf("dump hdmi registers success!!!\n");
}

static void test_write_regs(SiiInst_t inst)
{
	ApiTestObj_t* testObj = (ApiTestObj_t*)SII_LIB_OBJ_PNTR(inst);
	SiiDrvCraAddr_t reg = 0;
	uint8_t val = 0;
	char key[10];
	char *endptr = NULL;

	SII_LIB_LOG_PRINT2(("Set write address:0x3000~0x3fff\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		reg = (SiiDrvCraAddr_t)strtol(&key[2], &endptr, 16);
	} else {
		reg = (SiiDrvCraAddr_t)atoi(key);
	}
	if ( reg >= 4096 ) {
		SII_LIB_LOG_PRINT2(("reg 0x%x is out of range 0x~0xfff:\n", (uint32_t)reg));
		return ;
	}
	SII_LIB_LOG_PRINT2(("write reg 0x%x \n", (uint32_t)reg));
	SII_LIB_LOG_PRINT2(("Set write value:0x00~0xff\n"));
	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	if (key[0] == '0' && (key[1] == 'x' || key[1] == 'X') ) {
		val = (uint8_t)strtol(&key[2], &endptr, 16);
	} else {
		val = (uint8_t)atoi(key);
	}
	SII_LIB_LOG_PRINT2(("reg:0x%x,val=0x%x\n", (SiiDrvCraAddr_t)reg, (uint32_t)val));
	SiiDrvCraWrReg8(testObj->pInfo->instTxCra, (SiiDrvCraAddr_t)reg | BASE_ADDRESS, val);
	//printf("hdmi write registers success!!!\n");
}

void MoreApis(SiiInst_t inst, uint8_t level)
{
	ApiTestObj_t *testObj = (ApiTestObj_t*)SII_LIB_OBJ_PNTR(inst);

	char key;
	switch (level) {
		case 1:
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("More APIs...\n0 - SiiDrvTxHvSyncPolaritySet\n1 - SiiDrvTxAvMuteSet\n2 - SiiDrvTxInfoframeSet\n3 - SiiDrvTxInfoframeOnOffSet\n"));
			SII_LIB_LOG_PRINT2(("4 - SiiDrvTxChannelStatusSet\n5 - SiiDrvTxAudioFormatStatusGet\n6 - SiiDrvTxAudioFormatSet\n7 - SiiDrvTxHdcpProtectionSet\n"));
			SII_LIB_LOG_PRINT2(("8 - SiiDrvTxHdcpStateStatusGet\n9 - more apis\nx - exit\nq - Close Application\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			while ( !_kbhit() ) {
				;
			}
			key = (char)_getch();
			if (key == 'X') {
				key = 'x';
			}

			switch (key) {
				case '0':
					TestHvSyncPolaritySet();
					break;
				case '1':
					TestAvMuteSet();
					break;
				case '2':
					TestInfoframeSet();
					break;
				case '3':
					TestInfoframeOnOffSet();
					break;
				case '4':
					TestChannelStatusSet();
					break;
				case '5':
					TestAudioFormatGet();
					break;
				case '6':
					TestAudioFormatSet();
					break;
				case '7':
					TestHdcpProtectionSet();
					break;
				case '8':
					TestHdcpStatusGet();
					break;
				case '9':
					MoreApis(inst, 2);
					break;
				case 'x':
					return;
				case 'q':
					exit(1);
				default:
					SII_LIB_LOG_PRINT2(("Invalid Key Enterd.. Exiting.. \n"));
					break;
			}
			break;
		case 2:
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("Some more APIs...\n0 - SiiDrvTxHdcpKsvListGet\n1 - SiiDrvTxHdcpKsvListApprovalSet\n2 - SiiDrvTxHdcpTopologyGet\n3 - SiiDrvTxHdcp2ContentTypeSet\n"));
			SII_LIB_LOG_PRINT2(("4 - SiiDrvTxColorInfoConfig\n5 - SiiDrvTxOutputColorSpaceSet\n6 - SiiDrvTxOutputBitDepthSet\n"));
			if (testObj->pInfo->bProgrammPebbelsEn) {
				SII_LIB_LOG_PRINT2(("8 - ProgramPebbles\n"));
			}
			SII_LIB_LOG_PRINT2(("9 - more apis\nx - exit\nq - Close Application\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			while ( !_kbhit() ) {
				;
			}
			key = (char)_getch();
			if (key == 'X') {
				key = 'x';
			}

			switch (key) {
				case '0':
					TestHdcpBksvListGet();
					break;
				case '1':
					TestHdcpBksvListApprovalSet();
					break;
				case '2':
					TestHdcpTopologyGet();
					break;
				case '3':
					TestHdcp2ContentTypeSet();
					break;
				case '4':
					TestVidPathInputColorInfoConfig();
					break;
				case '5':
					TestVidPathOutputColorSpaceSet();
					break;
				case '6':
					TestTxOutputBitDepthSet();
					break;
				case '8':
					if (testObj->pInfo->bProgrammPebbelsEn) {
						TestProgramPebbles(testObj->pInfo->instTxCra);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '9':
					MoreApis(inst, 3);
					break;
				case 'x':
					return;
				case 'q':
					exit(1);
				default:
					SII_LIB_LOG_PRINT2(("Invalid Key Enterd.. Exiting.. \n"));
					break;
			}
			break;
		default:
			break;
		case 3:
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("Some more APIs...\n"));
			if (testObj->pInfo->bSimmonHandlingEn) {
				SII_LIB_LOG_PRINT2(("0 - EnableSiIMonAccess\n1 - DisableSiIMonAcess\n"));
			}
			if (testObj->pInfo->bCecEn) {
				SII_LIB_LOG_PRINT2(("2 - EnableCECOneTouchPlay\n"));
			}
			if (testObj->pInfo->bTpgEn) {
				SII_LIB_LOG_PRINT2(("3 - Enable/DisableTpg\n4 - ConfigTpgFormat\n5 - SelectTpgPatten\n"));
			}
			SII_LIB_LOG_PRINT2(("6 - GetLipSyncInfo\n\n"));
			if (testObj->pInfo->bScDcEn) {
				SII_LIB_LOG_PRINT2(("8 - ScdcReadScrambleandClockStatus\n\n"));
			}
			SII_LIB_LOG_PRINT2(("9 - more apis\n"));
			SII_LIB_LOG_PRINT2(("x - exit\nq - Close Application\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			while ( !_kbhit() ) {
				;
			}
			key = (char)_getch();
			if (key == 'X') {
				key = 'x';
			}

			switch (key) {
					#if (MT_SDK_COMPILE_HDMI20 == 0)
				case '0':
					if (testObj->pInfo->bSimmonHandlingEn) {
						TestSiIMonRegisterHandlingStart(testObj->pInfo->instSiIMonHandle);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '1':
					if (testObj->pInfo->bSimmonHandlingEn) {
						TestSiIMonRegisterHandlingStop(testObj->pInfo->instSiIMonHandle);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
					#endif
				case '2':
					if (testObj->pInfo->bCecEn) {
						TestCecOneTouchPlay();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '3':
					if (testObj->pInfo->bTpgEn) {
						TestTpgEnable(testObj->pInfo->instTpg);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '4':
					if (testObj->pInfo->bTpgEn) {
						TestTpgFormatSet(testObj->pInfo->instTpg);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '5':
					if (testObj->pInfo->bTpgEn) {
						TestTpgPatternSet(testObj->pInfo->instTpg);
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '6':
					TestLipSyncInfoGet();
					break;
				case '7':
					break;
				case '8':
					if (testObj->pInfo->bScDcEn) {
						ScdcReadScrambleAndClock();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '9':
					MoreApis(inst, 4);
					break;
				case 'x':
					return;
				case 'q':
					exit(1);
				default:
					SII_LIB_LOG_PRINT2(("Invalid Key Enterd.. Exiting.. \n"));
					break;
			}
			break;
		case 4:
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			if (testObj->pInfo->bScDcEn) {
				SII_LIB_LOG_PRINT2(("More APIs...\n0 - ScdcReadRequestTest\n1 - ScdcEnableAutopollingMode\n"));
			}
			SII_LIB_LOG_PRINT2(("2 - ScdcScrambleEnable\n3 - ScdcReadManufacturerRegister\n4 - ScdcReadStatusRegister\n5 - ScdcResetUpdateRegisters\n"));
			if (testObj->pInfo->bCecEn) {
				SII_LIB_LOG_PRINT2(("6 - EnableCECStandby\n"));
			}
			if (testObj->pInfo->bCecEn) {
				SII_LIB_LOG_PRINT2(("7 - EnableCEC Volume control\n"));
			}
			if (testObj->pInfo->bCecEn) {
				SII_LIB_LOG_PRINT2(("8 - Send CEC User Define key\n"));
			}
			SII_LIB_LOG_PRINT2(("x - exit\nq - Close Application\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("                                                     \n"));
			while ( !_kbhit() ) {
				;
			}
			key = (char)_getch();
			if (key == 'X') {
				key = 'x';
			}

			switch (key) {
				case '0':
					if (testObj->pInfo->bScDcEn) {
						ScdcReadRequestTest();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '1':
					if (testObj->pInfo->bScDcEn) {
						ScdcEnableAutoPoll();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '2':
					if (testObj->pInfo->bScDcEn) {
						ScdcScrambleEnable();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '3':
					if (testObj->pInfo->bScDcEn) {
						ScdcReadManufacturerRegister();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '4':
					if (testObj->pInfo->bScDcEn) {
						ScdcReadStatusRegister();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '5':
					if (testObj->pInfo->bScDcEn) {
						ScdcResetUpdateRegisters();
					} else {
						SII_LIB_LOG_PRINT2(("\nPlease check configuration file to enable SCDC\n"));
					}
					break;
				case '6':
					if (testObj->pInfo->bCecEn) {
						TestCecSysStandby();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '7':
					if (testObj->pInfo->bCecEn) {
						TestCecRemotePassPress();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '8':
					if (testObj->pInfo->bCecEn) {
						TestCecRemotePassAnyPress();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '9':
					MoreApis(inst, 5);
					break;
				case 'x':
					return;
				case 'q':
					exit(1);
				default:
					SII_LIB_LOG_PRINT2(("Invalid Key Enterd.. Exiting.. \n"));
					break;
			}
			break;
		case 5:
			SII_LIB_LOG_PRINT2(("													  \n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			if (testObj->pInfo->bCecEn) {
				SII_LIB_LOG_PRINT2(("0 - Get CEC Version\n1 - CEC Report Cur Latency\n"));
				SII_LIB_LOG_PRINT2(("2 - Send CEC Cmd\n"));
			}
			SII_LIB_LOG_PRINT2(("x - exit\nq - Close Application\n"));
			SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
			SII_LIB_LOG_PRINT2(("													  \n"));
			while ( !_kbhit() ) {
				;
			}
			key = (char)_getch();
			if (key == 'X') {
				key = 'x';
			}

			switch (key) {
				case '0':
					if (testObj->pInfo->bCecEn) {
						TestCecGetVersionCmd();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '1':
					if (testObj->pInfo->bCecEn) {
						TestCecReportCurrentLatency();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case '2':
					if (testObj->pInfo->bCecEn) {
						TestCecSendMsg();
					} else {
						SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
					}
					break;
				case 'x':
					return;
				case 'q':
					exit(1);
				default:
					SII_LIB_LOG_PRINT2(("Invalid Key Enterd.. Exiting.. \n"));
					break;
			}
			break;
	}
}

extern mt_u32 hdmi_get_test_aud_open_file_status(mt_void);
extern mt_u32 hdmi_get_test_aud_mode(mt_void);
extern mt_u32 hdmi_get_test_aud_sr(mt_void);
extern mt_u32 hdmi_get_test_aud_ch(mt_void);
extern mt_u32 hdmi_get_test_hbr_mode(mt_void);
static void sApiTestPrintDebugInfo(SiiInst_t inst)
{
	ApiTestObj_t* testObj = (ApiTestObj_t*)SII_LIB_OBJ_PNTR(inst);

	TestHotPlugGet();
	TestRsenGet();
	TestTmdsModeStatusGet();
	if (testObj->pInfo->bCecEn) {
		TestCecPhysicalAddrGet();
	}
	//TestAudioFormatGet();
	TestHdcpStatusGet();
	TestHdcpTopologyGet();
	SII_LIB_LOG_PRINT2(("\nCEC EN=%d\n", ((mt_u32)(SiiGetCecInst())) ? 1 : 0));
	SII_LIB_LOG_PRINT2(("\ntvsys=%d\n", hdmi_get_test_tvsys()));
	SII_LIB_LOG_PRINT2(("aud file open:%s\n", hdmi_get_test_aud_open_file_status() ? "FAIL" : "SUCCESS"));
	switch (hdmi_get_test_aud_mode()) {
		case 0:
			SII_LIB_LOG_PRINT2(("aud PCM & I2S\n"));
			break;
		case 1:
			SII_LIB_LOG_PRINT2(("aud DD & SPIDF\n"));
			break;
		case 2:
			SII_LIB_LOG_PRINT2(("aud PCM & SPDIF\n"));
			break;
		case 3:
			SII_LIB_LOG_PRINT2(("aud DD+ & SPDIF\n"));
			break;
		case 4:
			if ( hdmi_get_test_hbr_mode() ) {
				SII_LIB_LOG_PRINT2(("aud DTS & I2S HBR\n"));
			} else {
				SII_LIB_LOG_PRINT2(("aud DTS & SPDIF\n"));
			}
			break;
		default:
			break;
	}
	switch (hdmi_get_test_aud_sr()) {
		case 0:
			SII_LIB_LOG_PRINT2(("aud sample rate 32KHz\n"));
			break;
		case 1:
			SII_LIB_LOG_PRINT2(("aud sample rate 48KHz\n"));
			break;
		case 2:
			SII_LIB_LOG_PRINT2(("aud sample rate 96KHz\n"));
			break;
		case 3:
			SII_LIB_LOG_PRINT2(("aud sample rate 192KHz\n"));
			break;
		case 4:
			SII_LIB_LOG_PRINT2(("aud sample rate 44.1KHz\n"));
			break;
		case 5:
			SII_LIB_LOG_PRINT2(("aud sample rate 88.2KHz\n"));
			break;
		case 6:
			SII_LIB_LOG_PRINT2(("aud sample rate 176.4KHz\n"));
			break;
		default:
			break;
	}
	SII_LIB_LOG_PRINT2(("aud channel num:%d\n", hdmi_get_test_aud_ch()));
	SII_LIB_LOG_PRINT2(("aud hbr mode:%d\n", hdmi_get_test_hbr_mode()));
}

extern void sSetDelayEDIDRead(uint32_t delay);
static void TestSetEDIDReadDelay(void)
{
	char key[10];
	uint32_t val;
	SII_LIB_LOG_PRINT2(("													  \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Set EDID read delay 0<n<1000 ms\n"));

	memset(key, 0, sizeof(key));
	while (!_kbhit()) {;}
	gets_s(key, sizeof(key));
	val = (uint32_t)atoi(key);
	if (val >= 1000) {
		SII_LIB_LOG_PRINT2(("	Please input a number 0~1000\n "));
		return;
	}
	sSetDelayEDIDRead(val);
	SII_LIB_LOG_PRINT2(("Set EDID read delay val=%d ms\n", val));
}

extern void sSetEDIDPrintEn(uint32_t en);
static void TestSetEDIDPrint(void)
{
	char key = 0;
	SII_LIB_LOG_PRINT2(("													  \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Set Print Enable: 0: Disable(default), [0] = 1: EDID print on, [1] = 1: Log print on\n"));

	while (!_kbhit()) {;}
	key = (char)_getch();
	key -= '0';
	sSetEDIDPrintEn(key);
	SII_LIB_LOG_PRINT2(("Set Print Opt: %s, %s, %d!!!\n", (key & 0x1) ? "EDID Print on" : "EDID Print off", \
						(key & 0x2) ? "Log Print on" : "Log Print off", (uint32_t)key));
}

extern mt_void play_test_audio(void);
extern mt_u32 regfile_mem_close_all (void);
extern mt_u32 regfile_mem_deinit (void);
static void sApiTestSelect(SiiInst_t inst)
{
	ApiTestObj_t* testObj = (ApiTestObj_t*)SII_LIB_OBJ_PNTR(inst);
	char key = 0;

	if ( !_kbhit() ) {
		return;
	}
	key = (char)_getch();
	if (key != 's' && key != 'S') {
		return;
	}
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select an API from below to test:\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("0 - SiiDrvTxRegisterCallBack\n1 - SiiDrvTxCreate\n2 - SiiDrvTxDelete\n3 - SiiDrvTxEdidGet\n4 - SiiDrvTxHotPlugStatusGet\n"));
	SII_LIB_LOG_PRINT2(("5 - SiiDrvTxRsenStatusGet\n6 - SiiDrvTxTmdsModeSet\n7 - SiiDrvTxTmdsModeStatusGet\n"));
	if (testObj->pInfo->bCecEn) {
		SII_LIB_LOG_PRINT2(("8 - SiiDrvTxCecPhysicalAddrGet\n"));
	}
	SII_LIB_LOG_PRINT2(("9 - more apis\nu - Unit Test Cases\n"));
	SII_LIB_LOG_PRINT2(("t - switch tv format\n"));
	SII_LIB_LOG_PRINT2(("a - force load&play audio from buffer\n"));
	SII_LIB_LOG_PRINT2(("l - print test parameters(tv format, hdcp on/off, hdcp 1.4/2.2, ...)\n"));
	SII_LIB_LOG_PRINT2(("x - exit\nq - Close Application\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	while ( !_kbhit() ) {
		;
	}
	key = (char)_getch();
	if (key == 'X') {
		key = 'x';
	}

	switch ( key ) {
		default: // show help
			SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd.. Press 's' or 'S' to enter Test Mode\n"));
			break;
		case '0':
			TestRegisterCallBack();
			break;
		case '1':
			TestCreate();
			break;
		case '2':
			TestDelete();
			break;
		case '3':
			TestEdidGet();
			break;
		case '4':
			TestHotPlugGet();
			break;
		case '5':
			TestRsenGet();
			break;
		case '6':
			TestTmdsModeSet();
			break;
		case '7':
			TestTmdsModeStatusGet();
			break;
		case '8':
			if (testObj->pInfo->bCecEn) {
				TestCecPhysicalAddrGet();
			} else {
				SII_LIB_LOG_PRINT2(("\nInvalid Key Enterd..\n"));
			}
			break;
		case '9':
			MoreApis(inst, 1);
			break;
		case 't':
			TestTvSysSwitch();
			break;
		case 'a':
			//set audio format
			play_test_audio();
			break;
		case 'l':
			sApiTestPrintDebugInfo(inst);
			break;
		case 'd':
			test_dump_regs();
			break;
		case 'w':
			test_write_regs(inst);
			break;
		case 'D':
			TestSetEDIDReadDelay();
			break;
		case 'E':
			TestSetEDIDPrint();
			break;
		case 'r':
			test_set_ar_cmd(testObj->pInfo->instTx);
			break;
		case 'T':
			set_ae_test_en();
			break;
		case 'F':
			printf("Please type in the shell cmd:\n");
			{
				char	cmd[100];
				memset(cmd, 0, sizeof(cmd));
				gets_s(cmd, sizeof(cmd));
				printf("%s:\n", cmd);
				system(cmd);
			}
			break;
		case 'p':
			printf("prbs31 test config:\n");

			{
				//char key;
				printf("\nSet whether to do test clock step by step, '0':set 3229[1] enable txbist and 3235[0] prbs31 en, '1':insert prbs error\n");
				key = (char)_getch();
				key = (char)(key - '0');
				switch (key) {
					case 0:
						SiiDrvCraWrReg8(testObj->pInfo->instTxCra, REG_ADDR__TX_XBIST_CNTL, 0x2);
						SiiDrvCraWrReg8(testObj->pInfo->instTxCra, REG_ADDR__BIST31_CTRL, 0x1);
						break;
					case 1:
						SiiDrvCraWrReg8(testObj->pInfo->instTxCra, REG_ADDR__BIST31_CTRL, 0x3f);
						break;
					default:
						break;
				}
				printf("\n key=%d,reg[0x3229]=0x%x, reg[0x3235]=0x%x\n", (uint32_t)key, \
					   (uint32_t)SiiDrvCraRdReg8(testObj->pInfo->instTxCra, REG_ADDR__TX_XBIST_CNTL), \
					   (uint32_t)SiiDrvCraRdReg8(testObj->pInfo->instTxCra, REG_ADDR__BIST31_CTRL));
			}
			break;
		case 'R':
			TestHdcpReauthSet();
			break;
		case 'x':
			SII_LIB_LOG_PRINT2(("\nExit menu list...\n"));
			SII_LIB_LOG_PRINT2(("\nPress 's' or 'S' to enter again!!!\n"));
			return;
		case 'q':
			(mt_void)regfile_mem_close_all();
			(mt_void)regfile_mem_deinit();
			exit(1);
		case 'u':
		case 'U':
			sequencerLoop = false;
			break;
	}

}

void TestTpgEnable(SiiInst_t inst)
{
	char key;
	uint8_t  tpgEnable;

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("TPG Enable/Disable:\n"));
	SII_LIB_LOG_PRINT2(("0 - TPG Disable\n1 - TPG Enable\n"));

	while (!_kbhit()) {;}
	key = (char)_getch();

	switch (key) {
		case '0':
			tpgEnable = false;
			break;
		case '1':
			tpgEnable = true;
			break;
		default:
			SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
			return;
	}

	SiiTxTpgEnable(inst, tpgEnable);
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Done\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

void TestTpgFormatSet(SiiInst_t inst)
{
	uint8_t  tpgFormat;
	char portInfo[3];
	char port[3];
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("TPG Enable/Disable:\n"));
	SII_LIB_LOG_PRINT2(("0 - vm1_640x480p\n1 - vm2_3_720x480p\n2 - vm4_1280x720p\n3 - vm5_1920x1080i\n4 - vm6_7_720_1440x480i\n5 - vm8_9_720_1440x240p_1\n"));
	SII_LIB_LOG_PRINT2(("6 - vm8_9_720_1440x240p_2\n7 - vm10_11_2880x480i\n8 - vm12_13_2880x240p_1\n9 - vm12_13_2880x240p_2\n10 - vm14_15_1440x480p\n"));
	SII_LIB_LOG_PRINT2(("11 - vm16_1920x1080p\n12 - vm17_18_720x576p\n13 - vm19_1280x720p\n14 - vm20_1920x1080i\n15 - vm21_22_720_1440x576i\n"));
	SII_LIB_LOG_PRINT2(("16 - vm23_24_720_1440x288p_1\n17 - vm23_24_720_1440x288p_2\n18 - vm23_24_720_1440x288p_3\n19 - vm25_26_2880x576i\n20 - vm27_28_2880x288p_1\n"));
	SII_LIB_LOG_PRINT2(("21 - vm27_28_2880x288p_2\n22 - vm27_28_2880x288p_3\n23 - vm29_30_1440x576p\n24 - vm31_1920x1080p\n25 - vm32_1920x1080p\n"));
	SII_LIB_LOG_PRINT2(("26 - vm33_1920x1080p\n27 - vm34_1920x1080p\n28 - vm35_36_2880x480p\n29 - vm37_38_2880x576p\n30 - vm39_1920x1080i_1250_total\n"));
	SII_LIB_LOG_PRINT2(("31 - vm40_1920x1080i\n32 - vm41_1280x720p\n33 - vm42_43_720x576p\n34 - vm44_45_720_1440x576i\n35 - vm46_1920x1080i\n"));
	SII_LIB_LOG_PRINT2(("36 - vm47_1280x720p\n37 - vm48_49_720x480p\n38 - vm50_51_720_1440x480i\n39 - vm52_53_720x576p\n40 - vm54_55_720_1440x576i\n"));
	SII_LIB_LOG_PRINT2(("41 - vm56_57_720x480p\n42 - vm58_59_720_1440x480i\n43 - vm60_1280x720p\n44 - vm61_1280x720p\n45 - vm62_1280x720p\n"));
	SII_LIB_LOG_PRINT2(("46 - vm63_1920x1080p\n47 - vm64_1920x1080p\n48 - vm_3840x2160p\n49 - vm_3840x2160p\n50 - vm_3840x2160p\n"));
	SII_LIB_LOG_PRINT2(("51 - vm_4096x2160p\n52 - VM_PC_VGA72\n53 - VM_PC_VGA75\n54 - VM_PC_VGA85_1\n55 - VM_PC_VGA85_2\n"));
	SII_LIB_LOG_PRINT2(("56 - VM_PC_SVGA56\n57 - VM_PC_SVGA60\n58 - VM_PC_SVGA72\n59 - VM_PC_SVGA75\n60 - VM_PC_SVGA85\n"));
	SII_LIB_LOG_PRINT2(("61 - VM_PC_XGAI87\n62 - VM_PC_XGA60\n63 - VM_PC_XGA70\n64 - VM_PC_XGA75_1\n65 - VM_PC_XGA85\n"));
	SII_LIB_LOG_PRINT2(("66 - VM_PC_XGA75_2\n67 - VM_PC_WXGA60_1\n68 - VM_PC_WXGA60_2\n69 - VM_PC_WXGA75\n70 - VM_PC_WXGA60_800\n"));
	SII_LIB_LOG_PRINT2(("71 - VM_PC_WXGA60_3\n72 - VM_PC_SXGA60\n73 - VM_PC_WSXGA60_1\n74 - VM_PC_WSXGA60_2\n75 - VM_PC_WXGA85_1\n"));
	SII_LIB_LOG_PRINT2(("76 - VM_PC_WXGA85_2\n77 - VM_PC_SXGA75\n78 - VM_PC_SXGA85\n79 - VM_PC_HD60\n80 - VM_PC_WSXGA60\n"));
	SII_LIB_LOG_PRINT2(("81 - VM_PC_WSXGA75\n82 - VM_PC_WSXGA50\n83 - VM_PC_HD50_PLUS\n84 - VM_PC_UXGA60\n85 - VM_PC_WSXGA50_PLUS\n"));
	SII_LIB_LOG_PRINT2(("86 - VM_PC_WUXGA50\n87 - VM_PC_WUXGA60\n88 - VM_720X480i\n89 - VM_720X576i\n90 - VM_960X540\n"));
	SII_LIB_LOG_PRINT2(("91 - vm_3840x2160p\n92 - vm_3840x2160p\n"));

	//	while(!_kbhit()) {;}
	//key = (char)_getch();
	gets_s(portInfo, sizeof(portInfo));
	//sscanf_s(portInfo, "%s", port, sizeof(port));
	sscanf_s(portInfo, "%s", port);

	tpgFormat = (uint8_t)atoi(portInfo);

	if (tpgFormat > 91) {
		SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
		return;
	}

	SiiTxTpgFormatSet(inst, tpgFormat);

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Done\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

void TestTpgPatternSet(SiiInst_t inst)
{
	//	char key;
	uint8_t  tpgPattern;
	char portInfo[3];
	char port[3];

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select TPG Pattern:\n"));
	SII_LIB_LOG_PRINT2(("0 - FULLRED75\n1 - FULLGREEN75\n2 - FULLBLUE75\n3 - FULLCYAN75\n4 - FULLMAGENTA75\n5 - FULLYELLOW75\n"));
	SII_LIB_LOG_PRINT2(("6 - WINDOWIRE0\n7 - FULLWHITE75\n8 - 256GRAYRAMP\n9 - CLR8BARS100\n10 - CHECKERBOARD\n"));
	SII_LIB_LOG_PRINT2(("11 - SIMP92\n12 - FRMGEOM\n"));

	//	while(!_kbhit()) {;}
	//	key = (char)_getch();
	gets_s(portInfo, sizeof(portInfo));
	//sscanf_s(portInfo, "%s", port, sizeof(port));
	sscanf_s(portInfo, "%s", port);

	tpgPattern = (uint8_t)atoi(portInfo);

	if (tpgPattern > 12) {
		SII_LIB_LOG_PRINT2(("\nInvalid Input... returning"));
		return;
	}

	SiiTxTpgPatternSet(inst, tpgPattern);

	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("Done\n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("                                                     \n"));
}

void SiiApiTestInitSetting(void)
{
	sApiTestSelect((SiiInst_t)spApiTestObj);
}

/** END of File *********************************************************/
