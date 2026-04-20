/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_UNF_OTP_H__
#define __MT_UNF_OTP_H__

#include "mt_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/* IO structure */
struct otp_transfer_data {
	MT_U32 bit_addr;
	MT_U32 bit_len;
	int    buf_size;
#ifdef __KERNEL__
	mt_user_ulong_t buf;
#else
	void   *buf;
#endif
	int    version;
};

/* IO CMD */
#define OTP_DATA_IN     _IOW('O',0,struct otp_transfer_data)
#define OTP_DATA_OUT    _IOR('O',0,struct otp_transfer_data)
#define OTP_DATA_UNLOCK	_IOR('O',1,struct otp_transfer_data)

#if 0
enum
{
	CHIP_CONCERTO_A0 = 0xa0,
	CHIP_CONCERTO_A1,
	CHIP_CONCERTO_A3,
	CHIP_CONCERTO_B0,
	CHIP_SYMPHONY_A0,
	CHIP_SYMPHONY_A1,
	CHIP_SYMPHONY_A2,
	CHIP_SYMPHONY_MAX,
	CHIP_SYMPHONY3_A0,
	CHIP_SYMPHONY3_MAX,
	CHIP_SYMPHONY2_A0,
	CHIP_SYMPHONY2_A1,
	CHIP_SYMPHONY2_A2,
	CHIP_SYMPHONY2_A3,
	CHIP_SYMPHONY2_A4,
	CHIP_SYMPHONY2_A5,
	CHIP_SYMPHONY2_MAX,
	CHIP_SYMPHONY4_A0,
	CHIP_SYMPHONY4_A1,
	CHIP_UNKNOWN
};
#endif

typedef enum
{
	CMD_GET_PROPERTY_HD_EN = 0,
	CMD_GET_DRIVER_VERSION,
	CMD_GET_DOLBY_EN,
	CMD_GET_CUSTOMER_ID,
	CMD_GET_DRA_EN,
	CMD_GET_DTMB_EN,
	CMD_GET_DEMO_EN,
	CMD_GET_EPHY_EN,
	CMD_GET_H265_EN,
	CMD_GET_PROPERTY_HD_DEC_EN,
	CMD_GET_CW_CFG_EN,				/* 10 */
	CMD_GET_MACROVISION_EN,
	CMD_GET_USB0_EN,
	CMD_GET_USB1_EN,
	CMD_GET_DVBT2_EN,
	CMD_GET_DVBT_EN,
	CMD_GET_DVBS2X_EN,
	CMD_GET_DVBS_S2_S2X_EN,
	CMD_GET_J83B_EN,
	CMD_GET_DVBC_EN,
	CMD_GET_HDR_EN,					/* 20 */
	CMD_GET_VDEC10B_EN,
	CMD_GET_OTP_SMCD_EN,
	CMD_GET_OTP_SPIMEMMAP_EN,
	CMD_GET_OTP_ETHD_EN,
	CMD_GET_OTP_VP9_EN,

	/* SYM6 */
	CMD_GET_RV_EN,				/* RealVideo 8/9/10 */
	CMD_GET_HDMI_EMP_EN,		/* HDMI Extended Metadata Packet for HDR */
	CMD_GET_HDMI_EN,
	CMD_GET_CPC_EN,				/* CRI Content protection core/CRI Crypto firewall */
	CMD_GET_ARM_CORE1_EN,
	CMD_GET_G31_EN,				/* Mali-G31 */
	CMD_GET_USB1_30_EN,			/* USB3.0 function in USB1 port */
	CMD_GET_SL_HDR1_EN,			/* SL_HDR1 */
	CMD_GET_HDR_HLG_EN,			/* HDR10/HLG */
	CMD_GET_GMAC_EN,
	CMD_GET_UHD_EN,				/* 4K UHD */

	CMD_OTP_MAX,
}cmd_otp_property_e_t;

typedef enum
{
	E_AUTH_RW = 0,
	E_AUTH_RO,
	E_AUTH_WO,
	E_AUTH_FBD,
	E_AUTH_MAX,
}otp_ctrl_authority_t;

typedef enum
{
	E_PACKAGE_NULL = 0,
	E_PACKAGE_QFP144,
	E_PACKAGE_QFN68_SIP_512Mb_DDR2,
	E_PACKAGE_QFN68_SIP_1Gb_DDR2,
	E_PACKAGE_QFN68_SIP_1Gb_DDR3,
	E_PACKAGE_BGA_SIP_512Mb_DDR2,
	E_PACKAGE_BGA_SIP_1Gb_DDR2,
	E_PACKAGE_BGA_SIP_1Gb_DDR3,
	E_PACKAGE_QFN68_SIP_512Mb_DDR2_C,
	E_PACKAGE_QFN68_SIP_1Gb_DDR2_C,
	E_PACKAGE_QFN68_SIP_1Gb_DDR3_C,
	E_PACKAGE_BGA_SIP_512Mb_DDR2_C,
	E_PACKAGE_BGA_SIP_1Gb_DDR2_C,
	E_PACKAGE_BGA_SIP_1Gb_DDR3_C,
	E_PACKAGE_QFN88,
	E_PACKAGE_BGA,
	E_PACKAGE_MAX,
}otp_package_type_t;

//typedef enum
//{
//	E_PRODUCT_DVB_NONE_C_S_T = 0,
//	E_PRODUCT_DVB_S_S2_S2X,
//	E_PRODUCT_DVB_C,
//	E_PRODUCT_DVB_S_C,
//	E_PRODUCT_DVB_T_T2,
//	E_PRODUCT_DVB_S_T,
//	E_PRODUCT_DVB_C_T,
//	E_PRODUCT_DVB_S_C_T,
//	E_PRODUCT_MAX,
//}otp_product_type_t;
//note:the define follow the document ,if the document changed ,then zhe code will change
#ifdef CONFIG_MT_CHIP_SYMPHONY6
#if 0
/* A0 */
typedef enum
{
	E_PRODUCT_S_S2_S2X_C = 0,
	E_PRODUCT_C,
	E_PRODUCT_T_T2_C,
	E_PRODUCT_ALL,
	E_PRODUCT_WITHOUT_DEMO,
	E_PRODUCT_ABS,
	E_PRODUCT_ATSC,
	E_PRODUCT_ISDBT,
	E_PRODUCT_MAX,
}otp_product_type_t;
#else
/* A1 */
typedef enum
{
	E_PRODUCT_DVBS = 0,
	E_PRODUCT_DVBC,
	E_PRODUCT_DVBT,
	E_PRODUCT_MULTI_DEMO,
	E_PRODUCT_WITHOUT_DEMO,
	E_PRODUCT_MAX,
}otp_product_type_t;
#endif
#else
typedef enum
{
	E_PRODUCT_ALL = 0,
	E_PRODUCT_S_S2_S2X,
	E_PRODUCT_C,
	E_PRODUCT_T_T2,
	E_PRODUCT_S_S2S2X_T_T2,
	E_PRODUCT_MAX,
}otp_product_type_t;
#endif

//typedef enum		//TODO
//{
//}otp_chip_family_type_t;
//
//typedef enum
//{
//}otp_chip_generation_type_t;

#ifdef CONFIG_MT_CHIP_SYMPHONY6
#if 0
/* A0 */
typedef enum
{
	E_DISPLAY_SD = 0,
	E_DISPLAY_FHD_1P2K_DMIPS,
	E_DISPLAY_FHD_2K_DMIPS,
	E_DISPLAY_FHD_3K_DMIPS,
	E_DISPLAY_UHD_4K_4P5K_DMIPS_GPU,
	E_DISPLAY_UHD_4K_7K_DMIPS_GPU,
	E_DISPLAY_UHD_4K_15K_DMIPS_AI,
	E_DISPLAY_RSV,
	E_DISPLAY_UHD_8K_15K_DMIPS_AI,
	E_DISPLAY_UHD_8K_30K_DMIPS_AI,
	E_DISPLAY_MAX,
}otp_display_resolution_type_t;
#else
/* A1 */
typedef enum
{
	E_DISPLAY_FHD_3K_DMIPS = 0,
	E_DISPLAY_UHD_4K_4D5K_DMIPS_GPU,
	E_DISPLAY_UHD_4K_7K_DMIPS_GPU,
	E_DISPLAY_UHD_4K_15K_DMIPS_AI,

	E_DISPLAY_UHD_8K_15K_DMIPS_AI = 8,
	E_DISPLAY_UHD_8K_30K_DMIPS_AI = 9,
	E_DISPLAY_MAX,
}otp_display_resolution_type_t;
#endif
#else
typedef enum
{
	E_DISPLAY_ALL = 0,
	E_DISPLAY_SD_H264,
	E_DISPLAY_HD_H264,
	E_DISPLAY_HD_HEVC_8BIT,
	E_DISPLAY_HD_HEVC_10BIT,
	E_DISPLAY_HD_HEVC_HDR,
	E_DISPLAY_4K,
	E_DISPLAY_4K_PREMIUM,
	E_DISPLAY_8K,
	E_DISPLAY_MAX,
}otp_display_resolution_type_t;
#endif

typedef enum
{
	E_SECURITY_FTA = 0,
	E_SECURITY_ADVANCED_CA,
	E_SECURITY_ADVANCED_CA_DRM,
	E_SECURITY_SOFTWARE_CA,
	E_SECURITY_GENERAL_DRM,
	E_SECURITY_MAX,
}otp_security_type_t;

/* Symphony 4 */
typedef enum
{
	E_PACKAGE_INFO_LQFP = 0,
	E_PACKAGE_INFO_TQFP,
	E_PACKAGE_INFO_QFN,
	E_PACKAGE_INFO_MQFN,
	E_PACKAGE_INFO_TFBGA,
	E_PACKAGE_INFO_FCCSP,
	E_PACKAGE_INFO_LQFP176,

	/* Symphony 6 A0 */
	E_PACKAGE_SYM6_QFN = 2,
	E_PACKAGE_SYM6_BGA = 4,
	E_PACKAGE_SYM6_QFP = 7,

	/* Symphony 6 A1 */
	E_PACKAGE_SYM6_QFP_1ST_GEN = 0,
	E_PACKAGE_SYM6_QFP_2ND_GEN = 1,
	E_PACKAGE_SYM6_QFP_3RD_GEN = 2,
	E_PACKAGE_SYM6_QFN_1ST_GEN = 4,
	E_PACKAGE_SYM6_QFN_2ND_GEN = 5,
	E_PACKAGE_SYM6_QFN_3RD_GEN = 6,
	E_PACKAGE_SYM6_BGA_1ST_GEN = 8,
	E_PACKAGE_SYM6_BGA_2ND_GEN = 9,
	E_PACKAGE_SYM6_BGA_3RD_GEN = 10,

	E_PACKAGE_INFO_MAX,

}otp_package_info_type_t;

#ifdef CONFIG_MT_CHIP_SYMPHONY6
#if 0
/* A0 */
typedef enum
{
	E_DRAM_SIZE_NON = 0,
	E_DRAM_SIZE_32M,
	E_DRAM_SIZE_64M,
	E_DRAM_SIZE_128M,
	E_DRAM_SIZE_256M,
	E_DRAM_SIZE_512M,
	E_DRAM_SIZE_1G,
	E_DRAM_SIZE_2G,
	E_DRAM_SIZE_4G,

	E_DRAM_SIZE_DDR3_64M  = 18,
	E_DRAM_SIZE_DDR3_128M = 19,
	E_DRAM_SIZE_DDR4_512M = 21,
	E_DRAM_SIZE_DDR4_1G   = 22,

	E_DRAM_SIZE_MAX,
}otp_sip_dram_size_type_t;
#else
/* A1 */
typedef enum
{
	E_DRAM_SIZE_NONSIP = 0,
	E_DRAM_SIZE_32M,			/* 256Mb */

	E_DRAM_SIZE_64M_DDR2,		/* 512Mb */
	E_DRAM_SIZE_64M_DDR3,

	E_DRAM_SIZE_128M_DDR2,		/* 1Gb */
	E_DRAM_SIZE_128M_DDR3,

	E_DRAM_SIZE_256M,			/* 2Gb */

	E_DRAM_SIZE_512M_DDR3,		/* 4Gb */
	E_DRAM_SIZE_512M_DDR4,

	E_DRAM_SIZE_1G_DDR3,		/* 8Gb */
	E_DRAM_SIZE_1G_DDR4,

	E_DRAM_SIZE_2G,				/* 16Gb */

	E_DRAM_SIZE_MAX,
}otp_sip_dram_size_type_t;
#endif
#else
typedef enum
{
	E_DRAM_SIZE_NON = 0,
	E_DRAM_SIZE_256M,
	E_DRAM_SIZE_512M,
	E_DRAM_SIZE_1G,
	E_DRAM_SIZE_2G,
	E_DRAM_SIZE_4G,
	E_DRAM_SIZE_MAX,
}otp_sip_dram_size_type_t;
#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY6
typedef enum
{
	E_CA_VENDOR_NO = 0,
	E_CA_VENDOR_NAGRA_CONAX,
	E_CA_VENDOR_CONAX_CRI,
	E_CA_VENDOR_CTI,
	E_CA_VENDOR_ABV,
	E_CA_VENDOR_VO,				/* Viaccess-ORCA */
	E_CA_VENDOR_PANACCESS,
	E_CA_VENDOR_VERIMATRIX,
	E_CA_VENDOR_IRDETO,
	//E_CA_VENDOR_NSTV,			/* A0 */
	E_CA_VENDOR_GS_DRECRYPTO,	/* A1 */
	E_CA_VENDOR_DCAS,
	//E_CA_VENDOR_SUMAVISION,	/* A0 */
	E_CA_VENDOR_ICAS,			/* A1 */

	/* A1 added */
	E_CA_VENDOR_CRYPTOGUARD,	/* 12 */
	E_CA_VENDOR_NOVEL,
	E_CA_VENDOR_SUMA,

	E_CA_VENDOR_MAX,
}otp_ca_vendor_type_t;
#else
typedef enum
{
	E_CA_VENDOR_NO = 0,
	E_CA_VENDOR_NAGRA,
	E_CA_VENDOR_CONAX,
	E_CA_VENDOR_MAX,
}otp_ca_vendor_type_t;
#endif

typedef enum
{
	E_DRM_INFO_NO = 0,
	E_DRM_INFO_PLAYREADY,
	E_DRM_INFO_WIDEVINE,
	E_DRM_INFO_MARLIN,
	E_DRM_INFO_ALL_SUPPORT,
	E_DRM_INFO_MAX,
}otp_drm_info_type_t;

#ifdef CONFIG_MT_CHIP_SYMPHONY6
#if 0
/* A0 */
typedef enum
{
	E_LICENSE_NO_LIMIT = 0,						/* 0000 */
	E_LICENSE_DOLBY_AUDIO_DOLBY_VISION = 3,		/* 0011 */
	E_LICENSE_DOLBY = 7,						/* 0111 */
	E_LICENSE_DOLBY_VISION = 11,				/* 1011 */
	E_LICENSE_NO = 15,							/* 1111 */

	E_LICENSE_MAX,
}otp_ip_license_type_t;
#else
/* A1 */
typedef enum
{
	E_LICENSE_NO = 0,							/* 0000 */
	E_LICENSE_DOLBY_AUDIO_DOLBY_VISION = 1,		/* 0001 */
	E_LICENSE_DOLBY_VISION = 2,					/* 0010 */
	E_LICENSE_DOLBY_AUDIO = 3,					/* 0011 */

	E_LICENSE_MAX,
}otp_ip_license_type_t;
#endif
#else
typedef enum
{
	E_LICENSE_NO = 0,
	E_LICENSE_DOLBY,
	E_LICENSE_DOLBY_DTS,
	E_LICENSE_DOLBY_MACROVISION,
	E_LICENSE_DTS,
	E_LICENSE_DTS_MACROVISION,
	E_LICENSE_DOLBY_VISION,
	E_LICENSE_DOLBY_VISION_DTS,
	E_LICENSE_DOLBY_VISION_DTS_MACROVISION,
	E_LICENSE_MACROVISION,
	E_LICENSE_MAX,
}otp_ip_license_type_t;
#endif

typedef enum
{
	E_DDR_KGD_A2 = 0,
	E_DDR_KGD_A2B,
	E_DDR_KGD_A2C,
	E_DDR_KGD_MAX,
}otp_ddr_kgd_type_t;

typedef enum
{
	E_DRAM_CAP_NONE = 0,
	E_DRAM_CAP_512Mb_DDR2,
	E_DRAM_CAP_1Gb_DDR2,
	E_DRAM_CAP_1Gb_DDR3,
	E_DRAM_CAP_2Gb_DDR3,
	E_DRAM_CAP_MAX,
}otp_dram_cap_type_t;

MT_S32 MT_UNF_OTP_Init(MT_VOID);
MT_S32 MT_UNF_OTP_Deinit(MT_VOID);
MT_S32 MT_UNF_OTP_read(MT_U32 bitaddr,MT_U32 bitlen,MT_U32 * data);
MT_S32 MT_UNF_OTP_write(MT_U32 bitaddr,MT_U32 bitlen,MT_U32  data);
MT_S32 MT_UNF_OTP_unlock(MT_U32 bitaddr,MT_U32 bitlen,MT_U32  data);

/**
 * @brief get CAS Chip ID
 *
 * @param[out] pid_h High 32bit of CAS Chip ID
 * @param[out] pid_l Low 32bit of CAS Chip ID
 *
 * @return
 *   MT_SUCCESS
 *   MT_FAILURE
 */
MT_S32 MT_UNF_OTP_get_pid(MT_U32 * pid_h ,MT_U32 * pid_l);

MT_S32 MT_UNF_OTP_get_ctrl_authority(MT_U32 ctrl_no, MT_U32 *authority);
MT_S32 MT_UNF_OTP_get_package_type(MT_U32 *package_type);

/**
 * @brief get Chip Production Lot Number
 *
 * @param[out] p_chipid_h High 32bit of Lot Number
 * @param[out] p_chipid_l Low 32bit of Lot Number
 *
 * @return
 *   MT_SUCCESS
 *   MT_FAILURE
 */
MT_U32 MT_UNF_OTP_get_chipid(MT_U32 *p_chipid_h, MT_U32 *p_chipid_l);

MT_U32 MT_UNF_OTP_get_property(MT_U32 cmd, MT_U32 *p_result);
MT_S32 MT_UNF_OTP_get_product(otp_product_type_t * data);
MT_S32 MT_UNF_OTP_get_dram_cap(otp_dram_cap_type_t * data);
MT_S32 MT_UNF_OTP_get_ddr_kgd(otp_ddr_kgd_type_t * data);

/**
 * @brief get Chip Family
 *
 * @param[out] data Chip Family
 *                  0: Symphony
 *                  1: Concerto
 *                  2: Maestro
 *
 * @return
 *   MT_SUCCESS
 *   MT_FAILURE
 */
MT_S32 MT_UNF_OTP_get_chip_family(int * data);

/**
 * @brief get Chip Generation
 *
 * @param[out] data Chip Generation
 *                  for Symphony Chip Family -
 *                   2: Symphony2
 *                   4: Symphony4
 *                   6: Symphony6
 *
 * @return
 *   MT_SUCCESS
 *   MT_FAILURE
 */
MT_S32 MT_UNF_OTP_get_chip_generation(int * data);

MT_S32 MT_UNF_OTP_get_display_resolution(otp_display_resolution_type_t * data);
MT_S32 MT_UNF_OTP_get_secure_type(otp_security_type_t * data);
MT_S32 MT_UNF_OTP_get_package_info(otp_package_info_type_t * data);
MT_S32 MT_UNF_OTP_get_sipdramsize(otp_sip_dram_size_type_t * data);
MT_S32 MT_UNF_OTP_get_ca_vendor(otp_ca_vendor_type_t * data);
MT_S32 MT_UNF_OTP_get_ca_version(int * data);
MT_S32 MT_UNF_OTP_get_drm_info(otp_drm_info_type_t * data);
MT_S32 MT_UNF_OTP_get_ip_license(otp_ip_license_type_t * data);
MT_S32 MT_UNF_OTP_lock_STBM(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif	//__MT_UNF_OTP_H__
