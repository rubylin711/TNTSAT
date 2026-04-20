/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HDMI20_H__
#define __HDMI20_H__

/*=========================================================================
| TYPEDEFS
 ========================================================================*/
#define REG_SYS_BLOCK_RESET      (0xbf50a60c)
#define ETUDE2_TEST_SDHD_VENC (1)

#if ETUDE2_TEST_SDHD_VENC
#include "mt_common.h"
#define SSHH_PRINTF printf
#define INVALID_NUM (0xffffffff)

typedef enum TEST_CASE_ENUM_S {
	TEST_CASE_SD_VENC_START = 3,
	TEST_CASE_SD_VENC_COLOR_BAR_TEST = 3,
	TEST_CASE_SD_VENC_INTR_TEST = 20,
	TEST_CASE_SD_VENC_REG_RW_TEST = 21,
	TEST_CASE_SD_VENC_VID_RAM_TEST = 22,
	TEST_CASE_SD_VENC_RESET_TEST = 23,
	TEST_CASE_SD_VENC_MAX,

	TEST_CASE_HD_VENC_START = 4,
	TEST_CASE_HD_VENC_COLOR_BAR_TEST = 4,
	TEST_CASE_HD_VENC_DAC_SIN_TEST = 5,
	TEST_CASE_HD_VENC_INTR_TEST = 25,
	TEST_CASE_HD_VENC_REG_RW_TEST = 26,
	TEST_CASE_HD_VENC_RESET_TEST = 27,
	TEST_CASE_HD_VENC_MAX,

	TEST_CASE_HDMI_START = 101,
	TEST_CASE_HDMI_IIS_TEST = 101,
	TEST_CASE_HDMI_SPDIF_TEST = 105,
	TEST_CASE_HDMI_TVSYSTEM_TEST = 108,
	TEST_CASE_HDMI_CEC_TEST = 160,
	TEST_CASE_HDMI_COM_STABLE_TEST = 162,
	TEST_CASE_HDMI_HDCP14_TEST = 165,
	TEST_CASE_HDMI_HDCP21_TEST = 166,
	TEST_CASE_HDMI_HDR_TEST = 167,
	TEST_CASE_HDMI_REG_RW_TEST = 168,
	TEST_CASE_HDMI_REG1XXX_RW_TEST = 169,
	TEST_CASE_HDMI_REG2XXX_RW_TEST = 170,
	TEST_CASE_HDMI_REG0XXX_RW_HANGUP_TEST = 171,
	TEST_CASE_HDMI_CTS_TEST = 254,
	TEST_CASE_HDMI_ATE_TEST = 287,

	TEST_CASE_HDMI_RESET_TEST = 800,
	TEST_CASE_HDMI_HDCP_LOAD_KEY  = 801,
	TEST_CASE_HDMI_SET_POWER_ON_TVSYS = 802,
	TEST_CASE_HDMI_SET_ATE = 803,
	TEST_CASE_HDMI_MAX,
} TEST_CASE_ENUM_T;

typedef enum TEST_CASE_FUNC_ENUM_S {
	TEST_CASE_FUNC_IDLE,
	TEST_CASE_FUNC_SD_VENC,
	TEST_CASE_FUNC_VBI,
	TEST_CASE_FUNC_HD_VENC,
	TEST_CASE_FUNC_HDMI,
	TEST_CASE_FUNC_AOUT,
	TEST_CASE_FUNC_MAX,
} TEST_CASE_FUNC_ENUM_T;

typedef union REG_VALUE_S {
	mt_u8   reg8;
	mt_u16 reg16;
	mt_u32 reg32;
} REG_VALUE_T;

typedef struct REG_TEST_S {
	mt_u32 reg_addr;
	//REG_VALUE_T reg_default_value;
	mt_u32 reg_default_value;
	mt_u32 mask;  // dont care the bits be marked with 1 for read or write
	mt_u32 maskw;  // dont care the bits be marked with 1 for write
	mt_u32 maskr;  // dont care the bits be marked with 1 for read
	mt_u32 attr;    // [0]: read only, [1]: write only, 3: read/write, [2]:self-clearing 0, [3]:write to clear to be 0
	mt_u32 width; //register bit width
} REG_TEST_T;

/***** local functions *******************************************************/
mt_u32 misc_mod_test(void);

/***** end of file ***********************************************************/
#endif
#endif /* __HDMI20_H__ */
