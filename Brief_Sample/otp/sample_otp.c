/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "mt_unf_otp.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_OTP_DEBUG
#define MT_OTP_PRINT   printf
#else
#define MT_OTP_PRINT
#endif

#define SAMPLE_OTP_FUNCTION_ENTER()       MT_OTP_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_OTP_FUNCTION_EXIT()        MT_OTP_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_OTP_FATAL_PRINT(fmt...)    MT_OTP_PRINT(" [FATAL] " fmt)
#define SAMPLE_OTP_ERR_PRINT(fmt...)      MT_OTP_PRINT(" [ERROR] " fmt)
#define SAMPLE_OTP_WARN_PRINT(fmt...)     MT_OTP_PRINT(" [WARN] "  fmt)
#define SAMPLE_OTP_INFO_PRINT(fmt...)     MT_OTP_PRINT(" [INFO] "  fmt)
#define SAMPLE_OTP_DBG_PRINT(fmt...)      MT_OTP_PRINT(" [DEBUG] " fmt)

#define SAMPLE_OTP_PRINT    printf

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static const MT_CHAR otpProductType[5][32] =
{
    "PRODUCT_DVBS",
    "PRODUCT_DVBC",
    "PRODUCT_DVBT",
    "PRODUCT_MULTI_DEMO",
    "PRODUCT_WITHOUT_DEMO"
};

static const MT_CHAR otpChipFamily[3][32] =
{
    "Symphony",
    "Concerto",
    "Maestro",
};

static const MT_CHAR otpChipGeneration[7][32] =
{
	" ",
	" ",
    "Symphony2",
    " ",
    "Symphony4",
    " ",
    "Symphony6",
};

static const MT_CHAR otpDisplayResolutionType[10][32] =
{
    "DISPLAY_FHD",
    "DISPLAY_UHD_4K_4.5K_DMIPS_GPU",
    "DISPLAY_UHD_4K_7K_DMIPS_GPU",
    "DISPLAY_UHD_4K_15K_DMIPS_AI",
    " ",
    " ",
    " ",
    " ",
    "DISPLAY_UHD_8K_15K_DMIPS_AI",		//8
	"DISPLAY_UHD_8K_30K_DMIPS_AI",		//9
};

static const MT_CHAR otpCaVendorType[15][32] =
{
    "CA_VENDOR_NO",
    "CA_VENDOR_NAGRA_CONAX",
    "CA_VENDOR_CONAX_CRI",
	"CA_VENDOR_CTI",
	"CA_VENDOR_ABV",
	"CA_VENDOR_VO",
	"CA_VENDOR_PANACCESS",
	"CA_VENDOR_VERIMATRIX",
	"CA_VENDOR_IRDETO",
	"CA_VENDOR_GS_DRECRYPTO",
	"CA_VENDOR_DCAS",
	"CA_VENDOR_ICAS",
	"CA_VENDOR_CRYPTOGUARD",
	"CA_VENDOR_NOVEL",
	"CA_VENDOR_SUMA",
};

static const MT_CHAR otpPackageType[11][32] =
{
    "PACKAGE_QFP_1ST_GEN",
    "PACKAGE_QFP_2ND_GEN",
    "PACKAGE_QFP_3RD_GEN",
    " ",
    "PACKAGE_QFN_1ST_GEN",
    "PACKAGE_QFN_2ND_GEN",
    "PACKAGE_QFN_3RD_GEN",
    " ",
    "PACKAGE_BGA_1ST_GEN",
	"PACKAGE_BGA_2ND_GEN",
	"PACKAGE_BGA_3RD_GEN",
};

static const MT_CHAR otpIPLicense[4][32] =
{
    "NONE",
    "DOLBY_AUDIO_DOLBY_VISION",
    "DOLBY_VISION",
	"DOLBY_AUDIO",
};

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_OtpMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_VOID MT_OtpGetAllInfo(MT_VOID)
{
	int     family = 0;
	int     generation = 0;
    MT_U32  hPid = 0;
    MT_U32  lPid = 0;
    MT_U32  hChipid = 0;
    MT_U32  lChipid = 0;
    MT_S32  version = 0;
    MT_S32  Ret = MT_FAILURE;
    otp_product_type_t product = 0;
    otp_ca_vendor_type_t vendor = 0;
    otp_package_info_type_t packageInfo = 0;
    otp_display_resolution_type_t resolution = 0;
    otp_ip_license_type_t iplicense = 0;

    Ret = MT_UNF_OTP_get_chip_family(&family);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_chip_family error\n");
    }

    Ret = MT_UNF_OTP_get_chip_generation(&generation);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_chip_generation error\n");
    }

    Ret = MT_UNF_OTP_get_ip_license(&iplicense);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_ip_license error\n");
    }

    Ret = MT_UNF_OTP_get_pid(&hPid, &lPid);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_pid error\n");
    }

    Ret = MT_UNF_OTP_get_package_info(&packageInfo);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_package_info error\n");
    }

    Ret = MT_UNF_OTP_get_product(&product);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_product error\n");
    }

    Ret = MT_UNF_OTP_get_display_resolution(&resolution);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_display_resolution error\n");
    }

    Ret = MT_UNF_OTP_get_ca_vendor(&vendor);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_ca_vendor error\n");
    }

    Ret = MT_UNF_OTP_get_ca_version(&version);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_ca_version error\n");
    }

    Ret = MT_UNF_OTP_get_chipid(&hChipid, &lChipid);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_chipid error\n");
    }

    SAMPLE_OTP_PRINT("Chip Family:       %2d %s\n", family, otpChipFamily[family]);
    SAMPLE_OTP_PRINT("Chip Gen:          %2d %s\n", generation, otpChipGeneration[generation]);

    SAMPLE_OTP_PRINT("PID:                  %08x %08x\n", hPid, lPid);
    SAMPLE_OTP_PRINT("Chip ID:              %08x %08x\n", hChipid, lChipid);
    SAMPLE_OTP_PRINT("CA Vendor:         %2d %s\n", vendor, otpCaVendorType[vendor]);
    SAMPLE_OTP_PRINT("Version:           %2d\n", version);
    SAMPLE_OTP_PRINT("Product:           %2d %s\n", product, otpProductType[product]);
    SAMPLE_OTP_PRINT("Resolution:        %2d %s\n", resolution, otpDisplayResolutionType[resolution]);
    SAMPLE_OTP_PRINT("Package Type:      %2d %s\n", packageInfo, otpPackageType[packageInfo]);

	SAMPLE_OTP_PRINT("IP License:        %2d %s\n", iplicense, otpIPLicense[iplicense]);
}

static MT_VOID MT_OtpPrintMenu(MT_VOID)
{
    SAMPLE_OTP_PRINT("\ncommond: \n");
    SAMPLE_OTP_PRINT("     d: get the PID\n");
    SAMPLE_OTP_PRINT("     i: get the package type\n");
    SAMPLE_OTP_PRINT("     p: get the product type\n");
    SAMPLE_OTP_PRINT("     r: get the display resolution\n");
    SAMPLE_OTP_PRINT("     v: get the CA vendor information\n");
    SAMPLE_OTP_PRINT("     e: get the CA version\n");
    SAMPLE_OTP_PRINT("     c: get the chip ID\n");
    SAMPLE_OTP_PRINT("     a: get all the information\n");
    SAMPLE_OTP_PRINT("     q: quit \n");
    SAMPLE_OTP_PRINT("     h: help \n");
    SAMPLE_OTP_PRINT("OTP>> ");
}

static MT_VOID MT_OtpCmdTask(MT_VOID)
{
    MT_U32  hPid = 0;
    MT_U32  lPid = 0;
    MT_U32  hChipid = 0;
    MT_U32  lChipid = 0;
    MT_S32  version = 0;
    MT_S32  Ret = MT_FAILURE;
    MT_CHAR inputCmd[32] = { 0 };
    otp_product_type_t product = 0;
    otp_ca_vendor_type_t vendor = 0;
    otp_package_info_type_t packageInfo = 0;
    otp_display_resolution_type_t resolution = 0;

    while(1)
    {
        (MT_VOID)MT_OtpPrintMenu();

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if('d' == inputCmd[0])
        {
            Ret = MT_UNF_OTP_get_pid(&hPid, &lPid);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_pid error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("pid:               %08x %08x\n", hPid, lPid);
        }
        else if('i' == inputCmd[0])
        {
            Ret = MT_UNF_OTP_get_package_info(&packageInfo);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_package_info error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("package info type: %s\n", otpPackageType[packageInfo]);
        }
        else if('p' == inputCmd[0])
        {
            Ret = MT_UNF_OTP_get_product(&product);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_product error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("product:           %s\n", otpProductType[product]);
        }
        else if('r' == inputCmd[0])
        {
             Ret = MT_UNF_OTP_get_display_resolution(&resolution);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_display_resolution error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("resolution:        %s\n", otpDisplayResolutionType[resolution]);
        }
        else if('v' == inputCmd[0])
        {
            Ret = MT_UNF_OTP_get_ca_vendor(&vendor);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_ca_vendor error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("vendor:            %s\n", otpCaVendorType[vendor]);
        }
        else if('e' == inputCmd[0])
        {
            Ret = MT_UNF_OTP_get_ca_version(&version);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_ca_version error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("version:           %d\n", version);
        }
        else if('c' == inputCmd[0])
        {
           Ret = MT_UNF_OTP_get_chipid(&hChipid, &lChipid);
            if(MT_SUCCESS != Ret)
            {
                SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_get_chipid error\n");
                break;
            }
            SAMPLE_OTP_INFO_PRINT("chip id:          %08x %08x\n", hChipid, lChipid);
        }
        else if('a' == inputCmd[0])
        {
            (MT_VOID)MT_OtpGetAllInfo();
        }
        else if('q' == inputCmd[0])
        {
            break;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_OTP_INFO_PRINT("Print help info \n");
        }
    }
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_OtpMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32 Ret = MT_FAILURE;

#ifndef MT_SAMPLE_APP
    Ret = mt_sys_init();
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("mt_sys_init failed, Ret = %d\n", Ret);
        return Ret;
    }
#endif

    Ret = MT_UNF_OTP_Init();
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_OTP_ERR_PRINT("MT_UNF_OTP_Init failed, Ret = %d\n", Ret);
        goto ERR0;
    }

    (MT_VOID)MT_OtpCmdTask();

    (MT_VOID)MT_UNF_OTP_Deinit();
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return Ret;
}

