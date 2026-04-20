/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2023, Montage LZ Technology Co., Ltd.
 *
 * File Name      : mt_otp_object_sym6.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2023/08/02
 * Description    : MT Symphony6 OTP Objects.
 * History        :
 * 1.Date         : 2023/08/02
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_otp.h>
#elif defined(__KERNEL__)
#include "mt_type.h"
#include "mt_drv_otp.h"
#else	/*AVCPU*/
#include "sys_types.h"
#endif

#include "mt_otp_object.h"

#if !defined(CONFIG_TARGET_SYMPHONY6_LITE) && !defined(CONFIG_TARGET_SYMPHONY6_MINI)

#define SZ_RESERVED		"reserved"

/* The first character '0'/'1'/... is for Silk Screen */
/* OTP_SubFeature */
static const char *desc_OTP_SubFeature[] =
{
	/* SD/FHD, reference: */
	/*0*/ "0-H.264 zapper(1-T)",
	/*1*/ "1-H.264 feature(1-T,10/100 eth)",
	/*2*/ "2-H.264 rich(>=2-T,10/100 eth)",
	/*3*/ "3-H.265 zapper(1-T)",
	/*4*/ "4-H.265 feature(1-T,10/100 eth)",
	/*5*/ "5-H.265 rich(>=2-T,10/100 eth)",
	/*6*/ "6-" SZ_RESERVED,
	/*7*/ "7-TSIO",
	/*8*/ "8-" SZ_RESERVED,
	/*9*/ "9-CI/CI+",

	/*10*/ SZ_RESERVED,
	/*11*/ SZ_RESERVED,
	/*12*/ SZ_RESERVED,
	/*13*/ SZ_RESERVED,
	/*14*/ SZ_RESERVED,
	/*15*/ SZ_RESERVED,

	/* UHD, reference: */
	/*16*/ "0-H.265 zapper(1-T)",
	/*17*/ "1-H.265 feature(1-T,10/100 eth)",
	/*18*/ "2-H.265 feature(>=2-T,10/100 eth)",
	/*19*/ "3-H.265 feature(>=2-T,Gbe)",
	/*20*/ "4-H.265 feature(=1-T,eth) + AVx",
	/*21*/ "5-H.265 feature(>=2-T,eth) + AVx",
	/*22*/ "6-H.265 rich(>=2-T,eth) + AVx + VVC",
	/*23*/ "7-TSIO",
	/*24*/ "8-" SZ_RESERVED,
	/*25*/ "9-CI/CI+",

	NULL,
};

/* The first character '0'/'1'/... is for Silk Screen */
/* OTP_DisplayResolution */
static const char *desc_OTP_DisplayResolution[] =
{
	/*0*/ "0-SD",
	/*1*/ "1-FHD >= 1.2K DMIPS",
	/*2*/ "2-FHD >= 2K DMIPS",
	/*3*/ "3-FHD >= 3K DMIPS",
	/*4*/ "4-UHD, 4K > 4.5K DMIPS + GPU",
	/*5*/ "5-UHD, 4K >= 7K DMIPS + GPU",
	/*6*/ "6-UHD, 4K >= 15K DMIPS + AI",
	/*7*/ "7-" SZ_RESERVED,
	/*8*/ "8-UHD, 8K >= 15K DMIPS + AI",
	/*9*/ "9-UHD, 8K >= 30K DMIPS + AI",
	NULL,
};

/* The first character '2'/'4'/... is for Silk Screen */
/* OTP_ChipGeneration */
static const char *desc_OTP_ChipGeneration[] =
{
	/*0*/ "2-Symphony2",
	/*1*/ "4-Symphony4",
	/*2*/ "6-Symphony6",
	NULL,
};

/* The first character '2'/'5'/... is for Silk Screen */
/* OTP_ChipFamily */
static const char *desc_OTP_ChipFamily[] =
{
	/*0*/ "2-Symphony",
	/*1*/ "5-Concerto",
	/*2*/ "8-Maestro",
	NULL,
};

/* The first character 'S'/'C'/... is for Silk Screen */
/* OTP_ProductType */
static const char *desc_OTP_ProductType[] =
{
	/*0*/ "S-DVB-S/S2/S2X + C",
	/*1*/ "C-DVB-C",
	/*2*/ "T-DVB-T/T2 + C",
	/*3*/ "M-S/S2/S2X + T/T2 + C",
	/*4*/ "N-Decoder only without demodulator",
	/*5*/ "B-ABS",
	/*6*/ "A-ATSC",
	/*7*/ "I-ISDB-T",
	NULL,
};

/* The first character 'X'/'B'/... is for Silk Screen */
/* OTP_IPLicense */
static const char *desc_OTP_IPLicense[] =
{
	/*0*/ "X-No limitaion of License",
	/*1*/ "1-" SZ_RESERVED,
	/*2*/ "2-" SZ_RESERVED,
	/*3*/ "B-Dolby Audio + Dolbyvision",
	/*4*/ "4-" SZ_RESERVED,
	/*5*/ "5-" SZ_RESERVED,
	/*6*/ "6-" SZ_RESERVED,
	/*7*/ "D-Dolby Audio",
	/*8*/ "8-" SZ_RESERVED,
	/*9*/ "9-" SZ_RESERVED,
	/*10*/ SZ_RESERVED,
	/*11*/ "A-Dolbyvision",
	/*12*/ SZ_RESERVED,
	/*13*/ SZ_RESERVED,
	/*14*/ SZ_RESERVED,
	/*15*/ "0-No any License",
	NULL,
};

/* The first character '0'/'1'/... is for Silk Screen */
/* OTP_Bin_HWIP_Info */
static const char *desc_OTP_Bin_HWIP_Info[] =
{
	/*0*/ "0-bin1, Normal(1.5GHz)",
	/*1*/ "1-bin2, Middle(1.2GHz)",
	/*2*/ "2-bin3, Low(1.0GHz)",
	/*3*/ "3-bin1 + TSIO(if absent in #6)",
	/*4*/ "4-bin2 + TSIO(if absent in #6)",
	/*5*/ "5-bin3 + TSIO(if absent in #6)",
	/*6*/ SZ_RESERVED,
	/*7*/ SZ_RESERVED,
	/*8*/ SZ_RESERVED,
	/*9*/ SZ_RESERVED,
	/*10*/ "6-bin1 + Nexguard",
	/*11*/ "7-bin2 + Nexguard",
	/*12*/ "8-bin3 + Nexguard",
	NULL,
};

/* The first character '0'/'1'/... is for Silk Screen */
/* OTP_Chipset_Version */
static const char *desc_OTP_Chipset_Version[] =
{
	/*0*/ "0-A version die",
	/*1*/ "1-B version die",
	/*2*/ "2-C version die",
	/*3*/ "3-D version die",
	/*4*/ "4-E version die",
	/*5*/ "5-F version die",
	NULL,
};

/* The first character '0'/'G'/... is for Silk Screen */
/* OTP_CAVendor */
static const char *desc_OTP_CAVendor[] =
{
	/*0*/ "0-No CA",
	/*1*/ "G-Nagra+Conax",
	/*2*/ "C-Conax+CRI",
	/*3*/ "T-CTI",
	/*4*/ "A-ABV",
	/*5*/ "U-VO",
	/*6*/ "P-Panaccess",
	/*7*/ "V-Verimatrix",
	/*8*/ "D-Irdeto",
	/*9*/ "Y-NSTV",
	/*10*/ "Z-DCAS",
	/*11*/ "S-Sumavision",

	/* 'R': Cryptoguard */

	NULL,
};

/* The first character '0'/'1'/... is for Silk Screen */
/* OTP_SiPDRAMSize */
static const char *desc_OTP_SiPDRAMSize[] =
{
	/*0*/ "0-Non-SIP",
	/*1*/ "1-32MB",
	/*2*/ "2-64MB",
	/*3*/ "3-128MB",
	/*4*/ "4-256MB",
	/*5*/ "5-512MB",
	/*6*/ "6-1GB",
	/*7*/ "7-2GB",
	/*8*/ "8-4GB",
	/*9*/ SZ_RESERVED,
	/*10*/ SZ_RESERVED,
	/*11*/ SZ_RESERVED,
	/*12*/ SZ_RESERVED,
	/*13*/ SZ_RESERVED,
	/*14*/ SZ_RESERVED,
	/*15*/ SZ_RESERVED,
	/*16*/ SZ_RESERVED,
	/*17*/ SZ_RESERVED,
	/*18*/ "B-DDR3 64MB",
	/*19*/ "C-DDR3 128MB",
	/*20*/ SZ_RESERVED,
	/*21*/ "D-DDR4 512MB",
	/*22*/ "E-DDR4 1GB",
	NULL,
};

/* The first character 'N'/'B'/... is for Silk Screen */
/* OTP_PackageInfo */
static const char *desc_OTP_PackageInfo[] =
{
	/*0*/ "0-" SZ_RESERVED,
	/*1*/ "1-" SZ_RESERVED,
	/*2*/ "N-QFN",
	/*3*/ "3-" SZ_RESERVED,
	/*4*/ "B-BGA",
	/*5*/ "5-" SZ_RESERVED,
	/*6*/ "6-" SZ_RESERVED,
	/*7*/ "Q-QFP",
	NULL,
};

/* OTP_FMap_SiliconCut */
static const char *desc_OTP_FMap_SiliconCut[] =
{
	/*0*/ "A0",
	/*1*/ "A1",
	/*2*/ "A2",
	NULL,
};

/* OTP_ADAC_PackageType */
static const char *desc_OTP_ADAC_PackageType[] =
{
	/*0*/ "2Vrms, internal buffer",
	/*1*/ "0dbu, external buffer",
	NULL,
};

/* OTP_BootModeSel */
static const char *desc_OTP_BootModeSel[] =
{
	/*0*/ "Strap pin",
	/*1*/ SZ_RESERVED,
	/*2*/ SZ_RESERVED,
	/*3*/ "SPI NOR",
	/*4*/ SZ_RESERVED,
	/*5*/ "Parallel NAND",
	/*6*/ "SPI NAND",
	/*7*/ "eMMC",
	NULL,
};

/* OTP_DDR_SIZE */
static const char *desc_OTP_DDR_SIZE[] =
{
	/*0*/ "non-SIP",
	/*1*/ "SIP-1Gb",
	/*2*/ "SIP-2Gb",
	/*3*/ "SIP-4Gb",
	NULL,
};

//---------------------------------------------------------------------------//

/* Symphony6 OTP Objects */
struct mt_otp_object g_mt_otp_objects_sym6_a0[] =
{
/*   .name                         .bit_addr_flag .offset .shift .width .mask  .help  .description */

/* Bonding Options SW */

	/* bit 15-19 */
	{OTP_SubFeature,               0,             0x3FC0, 15,   5,       0x1F, NULL,  desc_OTP_SubFeature},
	/* bit 20-23 */
	{OTP_DisplayResolution,        0,             0x3FC0, 20,   4,        0xF, NULL,  desc_OTP_DisplayResolution},
	/* bit 24-25 */
	{OTP_ChipGeneration,           0,             0x3FC0, 24,   2,        0x3, NULL,  desc_OTP_ChipGeneration},
	/* bit 26-27 */
	{OTP_ChipFamily,               0,             0x3FC0, 26,   2,        0x3, NULL,  desc_OTP_ChipFamily},
	/* bit 28-31 */
	{OTP_ProductType,              0,             0x3FC0, 28,   4,        0xF, NULL,  desc_OTP_ProductType},

	/* bit 6-9 */
	{OTP_IPLicense,                0,             0x3FC4,  6,   4,        0xF, NULL,  desc_OTP_IPLicense},
	/* bit 8 */
	{"OTP_DolbyVision",            0,             0x3FC4,  8,   1,        0x1, "Dolby Vision",  NULL},
	/* bit 9 */
	{OTP_DolbyEnable,              0,             0x3FC4,  9,   1,        0x1, "Dolby Audio",  NULL},

	/* bit 10-13 */
	{OTP_Bin_HWIP_Info,            0,             0x3FC4, 10,   4,        0xF, NULL,  desc_OTP_Bin_HWIP_Info},
	/* bit 14-17 */
	{OTP_Chipset_Version,          0,             0x3FC4, 14,   4,        0xF, NULL,  desc_OTP_Chipset_Version},
	/* bit 18-22 */
	{OTP_CAVendor,                 0,             0x3FC4, 18,   5,       0x1F, NULL,  desc_OTP_CAVendor},
	/* bit 23-27 */
	{OTP_SiPDRAMSize,              0,             0x3FC4, 23,   5,       0x1F, NULL,  desc_OTP_SiPDRAMSize},
	/* bit 28-31 */
	{OTP_PackageInfo,              0,             0x3FC4, 28,   4,        0xF, NULL,  desc_OTP_PackageInfo},

	/* bit 29 */
	/* 0: enable, 1: disable */
	{OTP_DRAEnable,                0,             0x3ED4, 29,   1,	       0x1, "disbale or enable DRA",	NULL},
	/* bit 30-31 */
	/* 00: enbale, others disable */
	{OTP_DolbyForceDisable,        0,             0x3ED4, 30,   2,	       0x3, "make STB capable of disabling Dolby on field by OTA",	NULL},

/* Bonding Options HW */

	/* bit 2 */
	{"OTP_RVDIS",                  0,             0x3AA0,  2,   1,	       0x1, "disable or enable RealVideo 8/9/10 decode", NULL},
	/* bit 3 */
	{"OTP_HDR10P_Dis",             0,             0x3AA0,  3,   1,	       0x1, "disable or enable support HDR10+", NULL},
	/* bit 5 */
	{OTP_HDMI_EMP_Dis,             0,             0x3AA0,  5,   1,	       0x1, "disable or enable Extended Metadata Packet for HDR", NULL},
	/* bit 6 */
	{OTP_HDMIDis,                  0,             0x3AA0,  6,   1,	       0x1, NULL, NULL},
	/* bit 7 */
	/* CRI Content protection core/CRI Crypto firewall */
	{"OTP_CPCDis",                 0,             0x3AA0,  7,   1,	       0x1, "CRI Content protection core/CRI Crypto firewall", NULL},
	/* bit 8 */
	{"OTP_ARMCore1Dis",            0,             0x3AA0,  8,   1,	       0x1, NULL, NULL},
	/* bit 9 */
	{"OTP_G31_Dis",                0,             0x3AA0,  9,   1,	       0x1, "Mali-G31", NULL},
	/* bit 10 */
	{"OTP_VP9Dis",                 0,             0x3AA0, 10,   1,	       0x1, NULL, NULL},
	/* bit 14 */
	/* USB3.0 function in USB1 port */
	{"OTP_USB1_30",                0,             0x3AA0, 14,   1,	       0x1, "disable or enable USB3.0 function in USB1 port", NULL},
	/* bit 15 */
	{"OTP_EMAC_Dis",               0,             0x3AA0, 15,   1,	       0x1, NULL, NULL},
	/* bit 16 */
	{"OTP_SL_HDR1_Dis",            0,             0x3AA0, 16,   1,	       0x1, "disable or enable SL HDR1", NULL},
	/* bit 17 */
	{"OTP_SMCDis",                 0,             0x3AA0, 17,   1,	       0x1, "disable or enable Smart Card function", NULL},
	/* bit 18 */
	{"OTP_VDEC10B",                0,             0x3AA0, 18,   1,	       0x1, NULL, NULL},
	/* bit 19 */
	{"OTP_HDR10_HLG_Dis",          0,             0x3AA0, 19,   1,	       0x1, "disable or enable HDR10/HLG", NULL},
	/* bit 20 */
	{"OTP_GMAC_Dis",               0,             0x3AA0, 20,   1,	       0x1, NULL, NULL},
	/* bit 21 */
	{"OTP_DVBS_S2",                0,             0x3AA0, 21,   1,	       0x1, NULL, NULL},
	/* bit 22 */
	{"OTP_DVBS2X",                 0,             0x3AA0, 22,   1,	       0x1, NULL, NULL},
	/* bit 23 */
	{"OTP_DVBT",                   0,             0x3AA0, 23,   1,	       0x1, NULL, NULL},
	/* bit 25 */
	{"OTP_DVBC",                   0,             0x3AA0, 25,   1,	       0x1, NULL, NULL},
	/* bit 26 */
	{"OTP_J83B",                   0,             0x3AA0, 26,   1,	       0x1, NULL, NULL},
	/* bit 27 */
	{"OTP_USB1",                   0,             0x3AA0, 27,   1,	       0x1, NULL, NULL},
	/* bit 28 */
	{"OTP_USB0",                   0,             0x3AA0, 28,   1,	       0x1, NULL, NULL},
	/* bit 29 */
	{"OTP_H265",                   0,             0x3AA0, 29,   1,	       0x1, NULL, NULL},
	/* bit 30 */
	{"OTP_Macrovision",            0,             0x3AA0, 30,   1,	       0x1, NULL, NULL},
	/* bit 31 */
	{"OTP_UHD",                    0,             0x3AA0, 31,   1,	       0x1, "disable or enable 4K UHD", NULL},

	/* bit 0 */
	/* 0: allow to probe chip internal states, 1: forbidden */
	{"OTP_SysProbeEn",             0,             0x3AA4,  0,   1,	       0x1, NULL, NULL},
	/* bit 1 */
	/* 1: Boot clock gated forbidden, 0: enable */
	{"OTP_BootGateDis",            0,             0x3AA4,  1,   1,	       0x1, NULL, NULL},
	/* bit 2 */
	/* 1: Previous revision, 0: Updated revision */
	{"OTP_RSACodeRev",             0,             0x3AA4,  2,   1,	       0x1, NULL, NULL},
	/*
	 * bit 3
	 *  0: Allowed the default status of the ARM JTAG pinmuxs to be ARM JTAG;
	 *  1: Disallowed the default status of the ARM JTAG pinmuxs to be ARM JTAG, one register bit is used to control it.
	 */
	{"OTP_JTAGPINDEF_Dis",         0,             0x3AA4,  3,   1,	       0x1, NULL, NULL},
	/* bit 4 */
	{"OTP_ARMVectorBaseSel",       0,             0x3AA4,  4,   1,	       0x1, NULL, NULL},
	/* bit 5 */
	/*
	 * 0: one bsram read operation consumes 1 cycle
	 * 1: one bsram read operation consumes 2 cycle
	 */
	{"OTP_IFCPBsramTiming_Mode",   0,             0x3AA4,  5,   1,	       0x1, NULL, NULL},
	/* bit 6 */
	/* 0: use 2-stage flip-flop synchronizer, 1: use 3-stage flip-flop synchronizer */
	{"OTP_ARM_SYNC_Mode",          0,             0x3AA4,  6,   1,	       0x1, NULL, NULL},
	/* bit 21-25 */
	/* [4]: 0: Use register for default value; 1: Update default value from OTP after OTP preload done
	 * [3]: SMC5V IO parameter: pwr_out_en
	 * [2]: SMC5V IO parameter: mod5v_3vb; 0: 5V, 1: 3V
	 * [1]: SMC5V IO parameter: smc_reg<7>
	 * [0]: SMC5V IO parameter: smc_reg<6>
	 */
	{"OTP_SMC5V_Mode",             0,             0x3AA4, 21,   5,	      0x1F, NULL, NULL},

/* Fuse Map Version */
	/* bit 0-31 */
	{OTP_FuseMapVersion,           0,             0x3A30,  0,  32, 0xFFFFFFFF, NULL,  NULL},
	/* bit 0-7 */
	{"OTP_FMap_Version",           0,             0x3A30,  0,  8,        0xFF, NULL,  NULL},
	/* bit 16-23 */
	{"OTP_FMap_CASID",             0,             0x3A30, 16,  8,        0xFF, NULL,  NULL},
	/* bit 24-27 */
	{"OTP_FMap_SiliconCut",        0,             0x3A30, 24,  4,         0xF, NULL,  desc_OTP_FMap_SiliconCut},
	/* bit 28-31 */
	/*
	 * 0: Mass Production use or E2E test
	 * D: Internal use (restricted only inside Mont)
	 * E: Engineering samples (deliver to CA for certification)
	 * F: Presales samples with test keys (early sample to customers)
	 * Others: reserved
	 */
	{"OTP_FMap_Type",              0,             0x3A30, 28,  4,         0xF, NULL,  NULL},

/* ChipUID */
	/* bit 0-31 */
	{OTP_LotNumber_L,              0,             0x3FC8,  0,  32, 0xFFFFFFFF, "Chip Lot number information by Montage(Low 32bit)",  NULL},
	{OTP_LotNumber_H,              0,             0x3FCC,  0,  32, 0xFFFFFFFF, "Chip Lot number information by Montage(High 32bit)",  NULL},

/* CA Chip ID */
	/* bit 0-31 */
	{OTP_CA_Chip_ID_L,             0,             0x3EAC,  0,  32, 0xFFFFFFFF, NULL,  NULL},
	{OTP_CA_Chip_ID_H,             0,             0x3EB0,  0,  32, 0xFFFFFFFF, NULL,  NULL},

/* STB CA Serial Number */
	{"OTP_STB_CA_SN",              0,             0x3E74,  0,  32, 0xFFFFFFFF, "STB CA Serial Number",  NULL},

/* Boot from backup partition */
/*
 * 0000: Allow boot from backup partition
 * 0101: Only allowed eMMC boot from backup partition
 * others: Forbid boot from backup partition
 */
	{"OTP_Boot_Backup_Ena",        0,             0x3A80,  2,   4,        0xF, "Boot from backup partition",  NULL},

/* APCPU */
	/* bit 0-7 */
	{OTP_APCPUClkCfg,              0,             0x3AEC,  0,   8,       0xFF, "Clock configuration for APCPU: "
																			    "0: No restriction, "
																				"1: 1600MHz, "
																				"3: 1520MHz, "
																				"7: 1280MHz, "
																				"15: 1000MHz(share with EPHY PLL), "
																				"31: 960MHz, "
																				"63: 720MHz",
																			   NULL},

/* Analog calibration */
	/* bit 0-1 */
	{"OTP_DDR_SIZE",               0,             0x3FD4,  0,   2,	      0x3, "ddr-size for ddr parameter select",    desc_OTP_DDR_SIZE},
	/* bit 4 */
	{"OTP_MBIST_HVS_STATUS",       0,             0x3FD4,  4,   1,	      0x1, NULL,    NULL},
	/* bit 5 */
	{"OTP_SCAN_HVS_STATUS",        0,             0x3FD4,  5,   1,	      0x1, NULL,    NULL},
	/* bit 6 */
	/*
	 * 0: not calibration
	 * 1: calibration done
	 */
	{OTP_VDAC_CalibrationStatus,   0,             0x3FD4,  6,   1,	       0x1, NULL,    NULL},
	/* bit 7 */
	{OTP_ADAC_PackageType,         0,             0x3FD4,  7,   1,	       0x1, NULL,    desc_OTP_ADAC_PackageType},
	/* bit 8-15 */
	{OTP_VDAC_V1,                  0,             0x3FD4,  8,   8,	      0xFF, NULL,    NULL},
	/* bit 16-23 */
	{OTP_VDAC_V3,                  0,             0x3FD4, 16,   8,	      0xFF, NULL,    NULL},
	/* bit 24-30 */
	/* (Vmeas - 2000)/10(mV)(0-127 */
	{OTP_ADAC_Avddscr_Voltage,     0,             0x3FD4, 24,   7,	      0x7F, NULL,    NULL},

	/* bit 0-8 */
	/* Vp-Vn-800(mV) 0~511 */
	{"OTP_EPHY_CalibrationVol",    0,             0x3FD8,  0,   9,	     0x1FF, NULL,    NULL},
	/* bit 9 */
	/*
	 * 0: not calibrated
	 * 1: calibration done
	 */
	{"OTP_EPHY_CalibrationStatus", 0,             0x3FD8,  9,   1,	       0x1, NULL,    NULL},

/* CASID */
	{"OTP_CASID_0",                0,             0x3AB0,  0,   8,	      0xFF, NULL,    NULL},
	{"OTP_CASID_1",                0,             0x3AB0,  8,   8,	      0xFF, NULL,    NULL},

/* TEE Enable */
	/* bit 0-3 */
	/*
	 * 0000: TEE Disabled
	 * others: TEE Enabled
	 */
	{OTP_TEE_EN,				   0,			  0x3A3C,  0,	4,		  0xF, NULL,	NULL},
	/* bit 4-15 */
	/*
	 * 0: secure boot is disabled
	 * others: secure boot is enabled
	 */
	{"OTP_SECURE_EN",			   0,			  0x3A3C,  4,  12,		0xFFF, NULL,	NULL},

/* STR */
	/*
	 * 1101: STR function is disabled
	 * 0001: STR function is enabled
	 */
	{"OTP_STRFunc_Dis", 		   0,			  0x3A44, 20,	4,		  0xF, "disable or enable STR function",	NULL},
	/* bit 24-27 */
	{"OTP_BootPrintDis",		   0,			  0x3A44, 24,	4,		  0xF, NULL,	NULL},

/* UART */
	/* bit 24-25 */
	/*
	 * 00: enabled
	 * others: disabled
	 */
	{"OTP_UART1_TX_Dis",		   0,			  0x3A7C, 24,	 2, 	   0x3, NULL,  NULL},
	{"OTP_UART1_RX_Dis",		   0,			  0x3A7C, 26,	 2, 	   0x3, NULL,  NULL},
	{"OTP_UART0_TX_Dis",		   0,			  0x3A7C, 28,	 2, 	   0x3, NULL,  NULL},
	{"OTP_UART0_RX_Dis",		   0,			  0x3A7C, 30,	 2, 	   0x3, NULL,  NULL},

	/* bit 24-31 */
	/*
	 * 0x55: Uart Boot Enabled
	 * others: disable
	 */
	{"OTP_UARTBootEna", 		   0,			  0x3EB4, 24,	8,		 0xFF, NULL,	NULL},

	/* bit 4-6 */
	{"OTP_BootModeSel", 		   0,			  0x3EB4,  4,	3,		  0x7, NULL,	desc_OTP_BootModeSel},

/* Flash/eMMC */
	//TODO:
	{"OTP_FLASH_EMMC_PARAM",       0,             0x3EBC, 0,   30, 0x3FFFFFFF, "Flash/eMMC Boot Parameters",  NULL},

/* JTAG/I2C */
	/* bit 0-8 */
	/*
	 * 000000000: OPEN
	 * 000hhhxxx: Password protected
	 * hhhxxxxxx: CLOSED
	 */
	{"OTP_I2CMode", 			   0,			  0x3EF8,  0,	9,		0x1FF, "I2C debug port states",  NULL},
	/* bit 18-26 */
	/*
	 * 000000000: OPEN
	 * 000hhhxxx: Password protected
	 * hhhxxxxxx: CLOSED
	 */
	{"OTP_REEJTAGMode", 		   0,			  0x3EF8, 18,	9,		0x1FF, "REE JTAG debug port states",  NULL},

	/* bit 4-5 */
	/*
	 * 00: allow coresight as firewall host through private AHB prot.
	 * other: forbid
	 */
	{"OTP_Coresight_FWHost",	   0,			  0x3A54,  4,	2,		  0x3, NULL,  NULL},
	/* bit 6-7 */
	/*
	 * 00: enable AVCPU_JTAG if ARM_REEJTAG_OPEN
	 * other: disable AVCPU_JTAG forever
	 */
	{"OTP_AVCPU_JTAG",			   0,			  0x3A54,  6,	2,		  0x3, NULL,  NULL},

	/*
	 * bit 31
	 *
	 * HDCP Key Ram Read En
	 */
	{OTP_HDCP_Key_RAM_RD_Dis,      0,             0x3A78, 31,   1,         0x1, NULL,    NULL},

	{NULL},
};

#else

/* CONFIG_TARGET_SYMPHONY6_LITE || CONFIG_TARGET_SYMPHONY6_MINI */

struct mt_otp_object g_mt_otp_objects_sym6_a0[] =
{
	/* TODO: */

	/* bit 8-15 */
	{OTP_VDAC_V1,                  0,             0x3FD4,  8,   8,	      0xFF, NULL,    NULL},
	/* bit 16-23 */
	{OTP_VDAC_V3,                  0,             0x3FD4, 16,   8,	      0xFF, NULL,    NULL},
};

#endif

