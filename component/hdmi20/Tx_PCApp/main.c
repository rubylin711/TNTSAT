// Copyright ?2007, Deco, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Deco, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------
//#define SII_DEBUG 1

/***** #include statements ***************************************************/
#include "conio.h"
#include "si_datatypes.h"
#include "si_lib_malloc_api.h"
#include "si_lib_seq_api.h"
#include "platform_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_log_api.h"
#include "si_drv_cra_api.h"
#include "si_app_tx_api.h"

typedef enum {
	TX_REG_ACCESS_TYPE_BB_I2C,
	TX_REG_ACCESS_TYPE_BB_I2C_OVER_PRIF,
	TX_REG_ACCESS_TYPE_AA_I2C,
	TX_REG_ACCESS_TYPE_AA_I2C_OVER_PRIF,
	TX_REG_ACCESS_TYPE_AA_SPI,
	TX_REG_ACCESS_TYPE_CHEETAH_SPI
} SiiTxRegAccessPlatformType;

#define I2C_BITRATE  100

static void SiiPlatformBBI2CInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__I2C;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__BB;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformAardvarkI2CInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__I2C;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformBBI2C2PRIFInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__PRIF_OVER_I2C;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__BB;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformAardvarkI2C2PRIFInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__PRIF_OVER_I2C;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformAardvarkSPIInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__SPI;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformAardvarkGPIOInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__GPIO;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformCheetahSPIInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__SPI;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__CHEETAH;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

static void SiiPlatformCheetahGPIOInit(SiiPlatformInterface_t *interfaceInfo)
{
	interfaceInfo->interfaceType = SII_PLATFORM_TYPE__GPIO;
	interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__CHEETAH;
	if (SII_PLATFORM_STATUS__SUCCESS != SiiPlatformInit(interfaceInfo)) {
		SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
		getchar();
		exit(1);
	}
}

void platform_init(uint8_t pltfrm_type, SiiPlatformInterface_t *interfaceInfo)
{
	switch (pltfrm_type) {
		case TX_REG_ACCESS_TYPE_BB_I2C:
			SiiPlatformBBI2CInit(interfaceInfo);
			break;
		case TX_REG_ACCESS_TYPE_BB_I2C_OVER_PRIF:
			SiiPlatformBBI2C2PRIFInit(interfaceInfo);
			break;
		case TX_REG_ACCESS_TYPE_AA_I2C:
			SiiPlatformAardvarkI2CInit(interfaceInfo);
			break;
		case TX_REG_ACCESS_TYPE_AA_I2C_OVER_PRIF:
			SiiPlatformAardvarkI2C2PRIFInit(interfaceInfo);
			break;
		case TX_REG_ACCESS_TYPE_AA_SPI:
			SiiPlatformAardvarkSPIInit(interfaceInfo);
			break;
		case TX_REG_ACCESS_TYPE_CHEETAH_SPI:
			SiiPlatformCheetahSPIInit(interfaceInfo);
			break;
		default:
			break;
	}
}

#define TX_REG_ACCESS_PLATFORM_TYPE  3
SiiPlatformInterface_t pInterfaceInfo;

/***** Register Module name **************************************************/

SII_LIB_OBJ_MODULE_DEF(app_main);

/***** local prototypes ******************************************************/

/***** public functions ******************************************************/
#define TEST_MACROS 0

#if TEST_MACROS

void BasicMacro(void)
{
	//device "SW_TPI_Page0";

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3046, 0x02); 	// DCAP (Data Capture) Video Input: 12bpp
	SiiDrvCraWrReg8(getTxCraInstance(), 0x300B, 0xF4); 	// DPD
	SiiDrvCraWrReg8(getTxCraInstance(), 0x300C, 0x05);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3008, 0x35);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x30F7, 0x02); 	// Enable HDMI mode for output when
	SiiDrvCraWrReg8(getTxCraInstance(), 0x30a1, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3080, 0x74);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3082, 0xa4);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3083, 0x10);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3084, 0x30);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x303e, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3033, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3048, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3049, 0x00);

	//device "Page 3";

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3330, 0x9C);	// enable HDMI mode
}

void AudioInsertion(void)
{
	//2-ch I2S 48kHz

	//I2S source is from Header H21 with AP as source.
	//Set audio ID using DIP SW2[4:0]=01000 (I2S)

	//string con = "Tx IP Aardvark I2C Direct";

	//device "AIP_Page10";
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A21, 0x02);   	// set channel status fs to 48khz
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A22, 0x0b);   	// set channel status word length to 24 bits

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A01, 0x0a);   	// configure cts generation

	//write ("Page A", 0x03, 0x00); 	// n value
	//write ("Page A", 0x04, 0x60);
	//write ("Page A", 0x05, 0x00);

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A24, 0x0b);   	// set i2s word length to 24 bits
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A1d, 0x60);   	// configure i2s input
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A14, 0x10);   	// enable sd0 input

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2f, 0x00); 	// layout 0
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A13, 0x01); 	// enable audio (set audio ID using DIP SW2)

	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2C, 0x03);      // reset AIP and AFIFO
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2C, 0x00);      // reset AIP and AFIFO

	//device "HW_TPI_Page6";

	SiiDrvCraWrReg8(getTxCraInstance(), 0x36bf, 0x02);	// audio infoframe
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c0, 0x84);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c1, 0x01);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c2, 0x0a);

	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c3, 0x70);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c4, 0x01);	// 2-ch
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c5, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c6, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c7, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c8, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36c9, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36ca, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36cb, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36cc, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36cd, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x36df, 0xc0);	// enable & repeat

	//device "SW_TPI_Page0";

	SiiDrvCraWrReg8(getTxCraInstance(), 0x300e, 0x01);	// enable SW reset
	SiiDrvCraWrReg8(getTxCraInstance(), 0x300e, 0x00);

}

// 0x71: INTR1
#define VID_INPUT_FORMAT_CHANGE 0x01
#define VID_INPUT_FRMRATE_CHANGE 0x02
#define RI_128 0x08
#define PRIFTOUT 0x10
#define MDI_RSEN 0x20
#define MDI_HPD 0x40
#define SYS_CNTR 0x80

// 0x72 INTR2
#define VSYNC 0x01
#define TCLK_STBL_CHANGED 0x02
#define SOFTWARE_INTERRUPT 0x04
#define ENC_EN_CHANGED 0x20
#define BCAP_DONE 0x80

// 0x73 INTR3
#define DDC_EMPTY 0x01
#define DDC_FIFO_FULL 0x02
#define DDC_FIFO_HALF_FULL 0x04
#define DDC_CMD_DONE 0x08
#define RI_ERR_0 0x10
#define RI_ERR_1 0x20
#define RI_ERR_2 0x40
#define RI_ERR_3 0x80

void TestInteruupts()
{
	// clear all Interrups
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3074, 0xFF);
	#if 1
	// enable Interrupts. Mask Registers
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3075, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3076, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3077, 0xFF);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3078, 0xFF);
	#else
	// enable Interrupts. Mask Registers
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3075, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3076, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3077, 0x00);
	SiiDrvCraWrReg8(getTxCraInstance(), 0x3078, 0x00);
	#endif

	while (1) {
		// INTR 1
		if (SiiDrvCraRdReg8(0x3071) & VID_INPUT_FORMAT_CHANGE) {
			SII_PRINTF(("\n Video Input format Changed...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, VID_INPUT_FORMAT_CHANGE);
		}

		if (SiiDrvCraRdReg8(0x3071) & VID_INPUT_FRMRATE_CHANGE) {
			SII_PRINTF(("\n Video Input Frame Rate Changed...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, VID_INPUT_FRMRATE_CHANGE);
		}

		if (SiiDrvCraRdReg8(0x3071) & RI_128) {
			SII_PRINTF(("\n Ri Rolls over after 128 frames...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, RI_128);
		}

		if (SiiDrvCraRdReg8(0x3071) & PRIFTOUT) {
			SII_PRINTF(("\n PRIF timeout due to par_rdy stuck for too long...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, PRIFTOUT);
		}

		if (SiiDrvCraRdReg8(0x3071) & MDI_RSEN) {
			SII_PRINTF(("\n Monitor Detection signal (RSEN) has changed...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, MDI_RSEN);
		}

		if (SiiDrvCraRdReg8(0x3071) & MDI_HPD) {
			SII_PRINTF(("\n Monitor Detection signal (HPD) has changed...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, MDI_HPD);
		}

		if (SiiDrvCraRdReg8(0x3071) & SYS_CNTR) {
			SII_PRINTF(("\n 5000ms(HDCP), 2000ms(CBUS) wait time counter...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3071, SYS_CNTR);
		}

		// INTR 2

		if (SiiDrvCraRdReg8(0x3072) & VSYNC) {
			SII_PRINTF(("\n VSync active edge is recognized...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, VSYNC);
		}

		if (SiiDrvCraRdReg8(0x3072) & TCLK_STBL_CHANGED) {
			SII_PRINTF(("\n TCLK_STABLE has changed state Interrupt. ...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, TCLK_STBL_CHANGED);

			if (SiiDrvCraRdReg8(0x3070) & 0x01) {
				SII_PRINTF(("\n live state of TCLK_STABLE is *1* (SYS_STAT register bit 1) . ...\n"));
			} else {
				SII_PRINTF(("\n live state of TCLK_STABLE is *0* (SYS_STAT register bit 1) . ...\n"));
			}

		}

		if (SiiDrvCraRdReg8(0x3072) & SOFTWARE_INTERRUPT) {
			SII_PRINTF(("\n 1'b1 is written into 0x06F[7]...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, SOFTWARE_INTERRUPT);
		}
		if (SiiDrvCraRdReg8(0x3072) & ENC_EN_CHANGED) {
			SII_PRINTF(("\n ENC_EN changed from 1 to 0...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, ENC_EN_CHANGED);
		}
		if (SiiDrvCraRdReg8(0x3072) & BCAP_DONE) {
			SII_PRINTF(("\n detected that BCAP KSV ready bit is 1...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3072, BCAP_DONE);
		}

		// INTR 3

		if (SiiDrvCraRdReg8(0x3073) & DDC_EMPTY) {
			SII_PRINTF(("\n DDC FIFO is empty...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, DDC_EMPTY);
		}

		if (SiiDrvCraRdReg8(0x3073) & DDC_FIFO_FULL) {
			SII_PRINTF(("\n DDC FIFO is full interrup...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, DDC_FIFO_FULL);
		}

		if (SiiDrvCraRdReg8(0x3073) & DDC_FIFO_HALF_FULL) {
			SII_PRINTF(("\n DDC FIFO is half-full interrupt...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, DDC_FIFO_HALF_FULL);
		}

		if (SiiDrvCraRdReg8(0x3073) & DDC_CMD_DONE) {
			SII_PRINTF(("\n DDC command is done interrupt...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, DDC_CMD_DONE);
		}

		if (SiiDrvCraRdReg8(0x3073) & RI_ERR_0) {
			SII_PRINTF(("\n Ri and Ri?don't match during 1st frame *default during frame #127 = reg. 0x025 -1*...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, RI_ERR_0);
		}

		if (SiiDrvCraRdReg8(0x3073) & RI_ERR_1) {
			SII_PRINTF(("\n Ri and Ri?don't match during 2nd frame *default during frame #0 = reg. 0x025*...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, RI_ERR_1);
		}

		if (SiiDrvCraRdReg8(0x3073) & RI_ERR_2) {
			SII_PRINTF(("\n Ri did not changed between frames #127 and #0 *can happens 1/65000*...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, RI_ERR_2);
		}

		if (SiiDrvCraRdReg8(0x3073) & RI_ERR_3) {
			SII_PRINTF(("\n Ri reading was not done within one frame...\n"));
			SiiDrvCraWrReg8(getTxCraInstance(), 0x3073, RI_ERR_3);
		}

	}
}
#endif
//Todo:: remove this later
#define SI_9777   0
#define SI_9776   0

void SiiDrvPhyInit(SiiInst_t inst)
{
	SiiDrvCraWrReg8(inst, 0x14, 0x00);        // vo0 <- rx0, vo1 <- rx0, vo2 <- rx0

	return;
	/*
	//device "Titan";                   // enable tx0 phy & digital (titan)
	SiiDrvCraWrReg8(inst, 0x300b, 0xf6);      // b1: enable mhl clk
	// b2: enable pclk
	// b5: enable rx tmds 1.2v domain (may not be required)
	// b6: enable tx tmds core
	SiiDrvCraWrReg8(inst, 0x30f7, 0xc2);
	SiiDrvCraWrReg8(inst, 0x32a1, 0x1c);      // enable hdmi mode
	SiiDrvCraWrReg8(inst, 0x3330, 0x9d);      // b4: enable hdmi mode for tx phy
	SiiDrvCraWrReg8(inst, 0x3337, 0x85);      // b4.3: 1x tmds clk
	*/

}

/***** public functions ******************************************************/
void main(void)
{
	char key;
	SII_LIB_LOG_PRINT2(("                                                     \n"));
	SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
	SII_LIB_LOG_PRINT2(("Select I2C connection:\n\n"));
	SII_LIB_LOG_PRINT2(("a	-	Aadvark, PRIF over I2C  [INTR - PIN5 and RESET - PIN9]\n"));
	SII_LIB_LOG_PRINT2(("b	-	BlackBox, PRIF over I2C [INTR - PIN3 and RESET - PIN5]\n\n"));

	while ( !_kbhit() ) { ; }
	key = (char)_getch();

	switch (key) {
		case 'a':
		case 'A':
			platform_init(TX_REG_ACCESS_TYPE_AA_I2C_OVER_PRIF, &pInterfaceInfo);
			break;
		case 'b':
		case 'B':
			platform_init(TX_REG_ACCESS_TYPE_BB_I2C_OVER_PRIF, &pInterfaceInfo);
			break;
		default:
			SII_LIB_LOG_PRINT2(("Invalid selection, Closing Application"));
			exit(1);
	}
	//Configure GPIO later

	//Create Tx Instance
	if (SiiTxCreate(&pInterfaceInfo)) {
		bool_t loop = true;
		//Oscilator calibration to 20MHz
		SiiInst_t craInst = getTxCraInstance();
		SiiDrvCraWrReg8(craInst, 0x0098, 0xB3);

		SiiDrvPhyInit(craInst);

		#if TEST_MACROS
		BasicMacro(getTxCraInstance());
		AudioInsertion(getTxCraInstance());

		while (1);
		//TestInteruupts(getTxCraInstance());
		#endif
		while (loop) {
			SiiLibSeqTask();

			loop = SiiTxReCreate(&pInterfaceInfo);
		}
	}
}

/***** local functions *******************************************************/

/***** end of file ***********************************************************/
