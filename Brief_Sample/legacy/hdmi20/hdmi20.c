/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "mt_type.h"
#include <time.h>

#include <dlfcn.h>

#include <sys/select.h>
#include <signal.h>
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
#include "hdmi20.h"
#include "sdvenc_testcase.h"
#include "hdvenc_testcase.h"
#include "hdmi20_testcase.h"
#include "aout_testcase.h"
#include "misc_reg_op.h"
#include "mt_unf_flash.h"
#include "mt_unf_hdmi.h"

typedef enum {
    TX_REG_ACCESS_TYPE_BB_I2C,
    TX_REG_ACCESS_TYPE_BB_I2C_OVER_PRIF,
    TX_REG_ACCESS_TYPE_AA_I2C,
    TX_REG_ACCESS_TYPE_AA_I2C_OVER_PRIF,
    TX_REG_ACCESS_TYPE_AA_SPI,
    TX_REG_ACCESS_TYPE_CHEETAH_SPI
} SiiTxRegAccessPlatformType;

#define I2C_BITRATE  100
#define REG_SYS_BLOCK_RESET      (0xbf50a60c)

unsigned char g_hdcp_key_m2m_hdmi20[292] = {0};
static MT_U32 g_mtddev = 0xff;


void platform_init(uint8_t pltfrm_type, SiiPlatformInterface_t *interfaceInfo);
void SiiDrvPhyInit(SiiInst_t inst);
#if ETUDE2_TEST_SDHD_VENC
    mt_u32 char2int(mt_u8 *num);
    mt_u32 do_case(TEST_CASE_FUNC_ENUM_T func_num, mt_u32 case_num);
    mt_u32 misc_mod_test_4qa(void);
#endif
void hdmi20_load_hdcp_key(SiiInst_t inst);
extern mt_u32 do_case_reg1xxx_rw(void);
extern mt_u32 do_case_reg2xxx_rw(void);
extern mt_u32 do_case_reg0xxx_rw_hangup_test(void);
extern mt_u32 do_case_set_tvsys_cmd(mt_void);
extern bool_t SiiModTxHdcpLoadKey(SiiInst_t inst, uint8_t *p_key, uint32_t len);
extern void hdmi_block_reset(void);
extern mt_u32 do_case_set_tvsys_default(void);
extern mt_u32 do_case_iis_audio_default(void);

static void SiiPlatformBBI2CInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__I2C;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__BB;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
        SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
        getchar();
        exit(1);
    }
}

static void SiiPlatformAardvarkI2CInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__I2C;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
        SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
        getchar();
        exit(1);
    }
}

static void SiiPlatformBBI2C2PRIFInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__PRIF_OVER_I2C;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__BB;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
        SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
        getchar();
        exit(1);
    }
}

static void SiiPlatformAardvarkI2C2PRIFInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__PRIF_OVER_I2C;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
        SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
        getchar();
        exit(1);
    }
}

static void SiiPlatformAardvarkSPIInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__SPI;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__AARDVARK;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
        SII_LIB_LOG_DEBUG2(("platform initialization failed\n\nPress any key to Exit"));
        getchar();
        exit(1);
    }
}

static void SiiPlatformCheetahSPIInit(SiiPlatformInterface_t *interfaceInfo)
{
    interfaceInfo->interfaceType = SII_PLATFORM_TYPE__SPI;
    interfaceInfo->interfaceHardware = SII_PLATFORM_HARDWARE__CHEETAH;
    if (SII_PLATFORM_STATUS__SUCCESS != (SiiPlatformStatus_t)SiiPlatformInit(interfaceInfo)) {
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

//if use SII_LIB_LOG_DEBUG1 or SII_LIB_LOG_PRINT1, should open the following define
//SII_LIB_OBJ_MODULE_DEF_NOCREATE(app_main);

/***** local prototypes ******************************************************/

/***** public functions ******************************************************/
#define TEST_MACROS 0

#if TEST_MACROS

void BasicMacro(void)
{
    //device "SW_TPI_Page0";

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3046, 0x02);  // DCAP (Data Capture) Video Input: 12bpp
    SiiDrvCraWrReg8(getTxCraInstance(), 0x300B, 0xF4);  // DPD
    SiiDrvCraWrReg8(getTxCraInstance(), 0x300C, 0x05);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3008, 0x35);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x30F7, 0x02);  // Enable HDMI mode for output when
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

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3330, 0x9C);  // enable HDMI mode
}

void AudioInsertion(void)
{
    //2-ch I2S 48kHz

    //I2S source is from Header H21 with AP as source.
    //Set audio ID using DIP SW2[4:0]=01000 (I2S)

    //string con = "Tx IP Aardvark I2C Direct";

    //device "AIP_Page10";
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A21, 0x02);      // set channel status fs to 48khz
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A22, 0x0b);      // set channel status word length to 24 bits

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A01, 0x0a);      // configure cts generation

    //write ("Page A", 0x03, 0x00);     // n value
    //write ("Page A", 0x04, 0x60);
    //write ("Page A", 0x05, 0x00);

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A24, 0x0b);      // set i2s word length to 24 bits
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A1d, 0x60);      // configure i2s input
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A14, 0x10);      // enable sd0 input

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2f, 0x00);  // layout 0
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A13, 0x01);  // enable audio (set audio ID using DIP SW2)

    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2C, 0x03);      // reset AIP and AFIFO
    SiiDrvCraWrReg8(getTxCraInstance(), 0x3A2C, 0x00);      // reset AIP and AFIFO

    //device "HW_TPI_Page6";

    SiiDrvCraWrReg8(getTxCraInstance(), 0x36bf, 0x02);  // audio infoframe
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c0, 0x84);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c1, 0x01);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c2, 0x0a);

    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c3, 0x70);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c4, 0x01);  // 2-ch
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c5, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c6, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c7, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c8, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36c9, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36ca, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36cb, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36cc, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36cd, 0x00);
    SiiDrvCraWrReg8(getTxCraInstance(), 0x36df, 0xc0);  // enable & repeat

    //device "SW_TPI_Page0";

    SiiDrvCraWrReg8(getTxCraInstance(), 0x300e, 0x01);  // enable SW reset
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

#if ETUDE2_TEST_SDHD_VENC

mt_u32 char2int(mt_u8 *num)
{
    mt_u32 ret = 1;
    if ((*num) < '0' || (*num) > '9') {
        ret = INVALID_NUM;
    } else {
        ret = (mt_u32)((*num) - '0');
        SSHH_PRINTF("\n%s_%d:%d,%d,%d\n", __func__, __LINE__, (int)(*num), (int)('0'), ret);
    }
    return ret;
}

mt_u32 do_case(TEST_CASE_FUNC_ENUM_T func_num, mt_u32 case_num)
{
    mt_u32 ret = 0;
    switch (func_num) {
        case TEST_CASE_FUNC_SD_VENC:
        case TEST_CASE_FUNC_VBI:
            if (case_num < TEST_CASE_SD_VENC_START || case_num >= TEST_CASE_SD_VENC_MAX) {
                ret = 1;
            }
            break;
        case TEST_CASE_FUNC_HD_VENC:
            if (case_num < TEST_CASE_HD_VENC_START || case_num >= TEST_CASE_HD_VENC_MAX) {
                ret = 1;
            }
            break;
        case TEST_CASE_FUNC_HDMI:
        case TEST_CASE_FUNC_AOUT:
            if (case_num < TEST_CASE_HDMI_START || case_num >= TEST_CASE_HDMI_MAX) {
                ret = 1;
            }
            break;
        default:
            ret = 1;
            break;
    }

    if (ret) {
        SSHH_PRINTF("\n%s_%d: error func%dcase%d:\n", __func__, __LINE__, (mt_u32)func_num, case_num);
        return ret;
    }

    switch (case_num) {
        case TEST_CASE_SD_VENC_COLOR_BAR_TEST:
            ret = do_case_sd_color_bar();
            break;
        case TEST_CASE_HD_VENC_COLOR_BAR_TEST:
            break;
        case TEST_CASE_HD_VENC_DAC_SIN_TEST:
            ret = do_case_hd_dac_sin();
            break;
        case TEST_CASE_SD_VENC_REG_RW_TEST:
            break;
        case TEST_CASE_HD_VENC_REG_RW_TEST:
            break;
        case TEST_CASE_SD_VENC_RESET_TEST:
            ret = do_case_sd_reset();
            break;
        case TEST_CASE_HD_VENC_RESET_TEST:
            ret = do_case_hd_reset();
            break;
        case TEST_CASE_HDMI_RESET_TEST:
            hdmi_block_reset_cmd();
            break;
        case TEST_CASE_HDMI_REG_RW_TEST:
            ret = do_case_reg_rw();
            break;
        case TEST_CASE_HDMI_REG1XXX_RW_TEST:
            ret = do_case_reg1xxx_rw();
            break;
        case TEST_CASE_HDMI_REG2XXX_RW_TEST:
            ret = do_case_reg2xxx_rw();
            break;
        case TEST_CASE_HDMI_REG0XXX_RW_HANGUP_TEST:
            ret = do_case_reg0xxx_rw_hangup_test();
            break;
        case TEST_CASE_HD_VENC_INTR_TEST:
            //20 for hdvenc
            ret = do_case_share_irq(20);
            break;
        case TEST_CASE_HDMI_HDCP_LOAD_KEY:
            ret = do_case_hdcp_load_key();
            break;

        case TEST_CASE_HDMI_IIS_TEST:
        case TEST_CASE_HDMI_SPDIF_TEST:
            ret = do_case_iis_audio();
            break;
        case TEST_CASE_HDMI_SET_POWER_ON_TVSYS:
            ret = do_case_set_tvsys_cmd();
            break;
        case TEST_CASE_HDMI_SET_ATE:
            printf("prbs31 test config:\n");
            {
                char key;
                printf("\nSet whether to do test clock step by step, '0':set 3229[1] enable txbist and 3235[0] prbs31 en, '1':insert prbs error\n");
                key = (char)_getch();
                key = (char)(key - '0');
                switch (key) {
                    case 0:
                        misc_reg_put8(0x3229, 0xff, 0x2);
                        misc_reg_put8(0x3235, 0xff, 0x1);
                        break;
                    case 1:
                        misc_reg_put8(0x3229, 0xff, 0x2);
                        misc_reg_put8(0x3235, 0xff, 0x1);
                        misc_reg_put8(0x3235, 0xff, 0x3d);
                        break;
                    default:
                        break;
                }
                printf("\n key=%d,reg[0x3229]=0x%x, reg[0x3235]=0x%x\n", (uint32_t)key, \
                       (uint32_t)misc_reg_read8_single(0x3229), \
                       (uint32_t)misc_reg_read8_single(0x3235));
            }
            break;
        default:
            break;
    }

    if (ret) {
        SSHH_PRINTF("\n%s_%d: %d test case %d fail\n", __func__, __LINE__, (int)func_num, case_num);
    }
    return ret;
}

mt_u32 misc_mod_test(void)
{
    char key;
    TEST_CASE_FUNC_ENUM_T func_flag = 0;
    mt_u32 case_num = 0;
    mt_u32 ret = 0;
    printf("\n%s_%d: start to test sd/hd venc function\n", __func__, __LINE__);
SSHH_START:
    debug_key_wait(__FILE__, __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: input function num:\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 's''S' for sd venc function\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 'h''H' for hd venc function\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 'm''M' for hdmi function\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 'a''A' for Aout function\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 'e''E' To exit this test\n", __func__, __LINE__);
    SSHH_PRINTF("\n%s_%d: 'q''Q' To exit this App\n", __func__, __LINE__);

    key = (char)_getch();
    switch (key) {
        case 's':
        case 'S':
            func_flag = TEST_CASE_FUNC_SD_VENC;
            break;
        case 'v':
        case 'V':
            func_flag = TEST_CASE_FUNC_VBI;
            break;
        case 'h':
        case 'H':
            func_flag = TEST_CASE_FUNC_HD_VENC;
            break;
        case 'm':
        case 'M':
            func_flag = TEST_CASE_FUNC_HDMI;
            break;
        case 'a':
        case 'A':
            func_flag = TEST_CASE_FUNC_AOUT;
            break;
        case 'e':
        case 'E':
            SSHH_PRINTF("\n%s_%d: End to test sd/hd venc funcion\n", __func__, __LINE__);
            goto SSHH_END;
            break;
        case 'q':
        case 'Q':
            exit(1);
            break;
        default:
            exit(1);
            SII_LIB_LOG_PRINT2(("Err option, End to test sd/hd venc funcion\n"));
            goto SSHH_END;
            break;
    }

    //get case number
    {
        int i = 0;
        int bb = 1;
        SSHH_PRINTF("\n%s_%d: type 3 numbers\n", __func__, __LINE__);
        case_num = 0;
        for (i = 0; i < 3; i++) {
            key = (char)_getch();
            SSHH_PRINTF("\n%c,%d,%d\n", key, (int)sizeof(char), (int)sizeof(mt_u8));
            ret = char2int((mt_u8*)&key);
            SSHH_PRINTF("\n%d\n", ret);
            if (ret != INVALID_NUM) {
                case_num = case_num * bb + ret;
                SSHH_PRINTF("\n%s_%d: %d\n", __func__, __LINE__, case_num);
            } else {
                SSHH_PRINTF("\n%s_%d: input invalid numer...end\n", __func__, __LINE__);
                break;
            }
            bb = 10;
        }
    }

    ret = regfile_mem_init();
    ret = regfile_mem_open_all();
    ret = do_case(func_flag, case_num);
    if (ret) {
        SSHH_PRINTF("\n%s_%d: func%dcase%d test fail\n", __func__, __LINE__, (mt_u32)func_flag, case_num);
    } else {
        SSHH_PRINTF("\n%s_%d: func%dcase%d test success\n", __func__, __LINE__, (mt_u32)func_flag, case_num);
    }
    ret = regfile_mem_close_all();
    ret = regfile_mem_deinit();

    goto SSHH_START;

SSHH_END:
    SII_LIB_LOG_PRINT2(("\n....End to test sd/hd venc funcion\n"));
    return ret;
}

mt_u32 misc_mod_test_4qa(void)
{
    mt_u32 ret = 0;
    printf("\n%s_%d: set default video/audio\n", __func__, __LINE__);

    ret |= regfile_mem_init();
    ret |= regfile_mem_open_all();
    hdmi_block_reset();
    ret |= do_case_set_tvsys_default();
    ret |= do_case_iis_audio_default();
    ret |= regfile_mem_close_all();
    ret |= regfile_mem_deinit();

    return ret;
}

#endif

static mt_s32 read_hdcp_key(MT_UNF_HDMI_ID_E enHDMIId, mt_u8 *p_key, mt_u32 key_len)
{
    MT_U32 mtddev = 16;//DEFAULT_MTDDEV;
    char mtddev_name[64] = {0};
    ulong gmtd_handle = 0;
    mt_s32 ret = MT_FAILURE;

	if ( g_mtddev != 0xff ) {
		mtddev = g_mtddev;
	}
    ret = mt_unf_flash_init();
    if ( ret == MT_SUCCESS )
    {
        sprintf(mtddev_name,"/dev/mtd%d",mtddev);
        ret = mt_unf_flash_open(mtddev_name, &gmtd_handle);
        if (MT_SUCCESS == ret)
        {
            ret = mt_unf_flash_read(gmtd_handle, 0, p_key, key_len);
        }
    }

    return ret;
}

void hdmi20_load_hdcp_key(SiiInst_t inst)
{
    bool_t rret;
    unsigned char hdcp_key_m2m_hdmi20_tmp[304];
    mt_u32 key_len = 0;
	//int i;

    key_len = sizeof(g_hdcp_key_m2m_hdmi20) / sizeof(g_hdcp_key_m2m_hdmi20[0]);
    memset(g_hdcp_key_m2m_hdmi20, 0, key_len);
    (void)read_hdcp_key(MT_UNF_HDMI_ID_0, g_hdcp_key_m2m_hdmi20, key_len);

    memcpy(hdcp_key_m2m_hdmi20_tmp, g_hdcp_key_m2m_hdmi20, key_len);

    rret = SiiModTxHdcpLoadKey(inst, hdcp_key_m2m_hdmi20_tmp, key_len);
    if (rret) {
        return;
    }
}

#define HDMI_TEST_TIMER	(0)
#if HDMI_TEST_TIMER
void test_timer(void)
{
    //return;
    uint32_t cnt = 10000;
    uint32_t i;
    #if 0
    printf("\ntest sleep time %dms start \n", cnt);
    for (i = 0; i < cnt; i++) {
        SiiLibTimeMilliDelay(1);
    }
    printf("\ntest sleep time %dms end \n", cnt);
    #endif
    printf("\ntest sleep time %dms start \n", cnt);
    for (i = 0; i < 10; i++) {
        SiiLibTimeMilliDelay(1000);
    }
    printf("\ntest sleep time %dms end \n", cnt);
}
#endif

/***** public functions ******************************************************/
int main(int argc, char *argv[])
{
    #if ((HDMI20_RELEASE_2_QA == 0 || defined(CONFIG_MT_FPGA)) && defined (CONFIG_MT_CHIP_ETUDE2))
    char key;
    bool_t ret = TRUE;
    #endif
    SII_LIB_LOG_PRINT2(("                                                     \n"));
    SII_LIB_LOG_PRINT2(("----------------------------------------------------\n"));
    SII_LIB_LOG_PRINT2(("b  -   BlackBox, PRIF over I2C [INTR - PIN3 and RESET - PIN5]\n\n"));
    SII_LIB_LOG_PRINT2(("TEST ver: 100.1\n\n"));

	#if HDMI_TEST_TIMER
    test_timer();
	#endif
    {
	    int i;
	    printf("Program name: %s\n", argv[0]);
	    for (i = 1; i < argc; i++) {
	        printf("Argument %d: %s\n", i, argv[i]);
	    }
		if (argc > 1) {
			g_mtddev = atoi(argv[1]);
			printf("g_mtddev %d\n", g_mtddev);
		}
    }

    platform_init(TX_REG_ACCESS_TYPE_BB_I2C_OVER_PRIF, &pInterfaceInfo);

    #if (HDMI20_RELEASE_2_QA && ((!defined(CONFIG_MT_FPGA)) || HDMI20_RELEASE_2_QA_SYM6))
    misc_mod_test_4qa();
    #else
    #if ETUDE2_TEST_SDHD_VENC
    misc_mod_test();
    hdmi_block_reset();
    #endif
    #endif
    //Configure GPIO later

    //
    (mt_void)regfile_mem_init();
    (mt_void)regfile_mem_open_all();

    //Create Tx Instance
    if (SiiTxCreate(&pInterfaceInfo)) {
        bool_t loop = true;

        //Oscilator calibration to 20MHz
        SiiInst_t craInst = getTxCraInstance();
        printf("\n%s_%d\n", __func__, __LINE__);
        hdmi20_load_hdcp_key(craInst);
        printf("\n0x%x_0x%x_0x%x_0x%x\n", (int)SiiDrvCraRdReg8(craInst, 0x3000), \
               (int)SiiDrvCraRdReg8(craInst, 0x3001), \
               (int)SiiDrvCraRdReg8(craInst, 0x3002), \
               (int)SiiDrvCraRdReg8(craInst, 0x3003));
        printf("\n%s_%d\n", __func__, __LINE__);
        #if ((HDMI20_RELEASE_2_QA == 0 || defined(CONFIG_MT_FPGA)) && defined (CONFIG_MT_CHIP_ETUDE2))
        while (1) {
            debug_key_wait(__FILE__, __func__, __LINE__);
            SSHH_PRINTF("\n%s_%d: input function num:\n", __func__, __LINE__);
            SSHH_PRINTF("\n%s_%d: Plz press 's''S' To set parameters in 3s\n", __func__, __LINE__);
            SiiLibTimeMicroDelay(3000);//delay 3s for pressing key
            SiiApiTestInitSetting();
            debug_key_wait(__FILE__, __func__, __LINE__);
            SSHH_PRINTF("\n%s_%d: input function num:\n", __func__, __LINE__);
            SSHH_PRINTF("\n%s_%d: 'q''Q' To exit this test setting to continue, else to setting again\n", __func__, __LINE__);

            key = (char)_getch();
            if ('q' == key || 'Q' == key) {
                break;
            }
        }
        #else
        extern mt_void hdmi_save_avinfoframe(SiiInst_t inst);
        hdmi_save_avinfoframe(getTxInstance());
        #endif
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
    return 0;
}

/***** local functions *******************************************************/

/***** end of file ***********************************************************/

